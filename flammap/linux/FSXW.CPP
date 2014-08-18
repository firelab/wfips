//#include "fsxw.hpp"
//#include "fsglbvar.h"
#include "stdafx.h"
#include "flammap3.h"
//#include <mem.h>
#include <math.h>
#include <windows.h>

#define SIZE_FIREPERIM_BLOCK 10000

//static double 	PerimeterResolution=0.0;			// maximum tangential perimeter resolution
//static double 	DistanceResolution=0.0;        	// minimum radial spread distance resolution
//static double 	DynamicDistanceResolution=0.0;	// minimum run-time radial resolution
//static double 	actual=0.0;					// actual time step
//static double 	visible=0.0;                   	// visible time step
//static double 	TemporaryTimestep=0.0;			// temp timestep for sim-level proc. control
//static double 	secondaryvisible=-1.0;         	// seconary visible timestep
//static double 	EventMinTimeStep=-1.0;			// event driven minimum timestep
//static bool 	checkexpansions=false;
//static bool 	checkpostfrontal=false;
//static long 	DistanceMethod=1;				// method for distance checking
//static bool 	AccelerationState=false;			// Flag for using acceleration constants

static long 	NumPerimAlloc=0;				// Current number of perimeter arrays allocated with new
static long* 	inout=0;						// fire doesn't exist (0), burning out(2), in(2)
static long* 	numpts=0;                   		// number of points in each fire
static double** perimeter1=0;            		// pointers to arrays with perimeter points
static double*	 perimeter2=0;					// swap array

static double 	redros[257];              		// rate of spread reduction factors
static size_t 	nmemb;
static double 	PercentageOfEmberIgnitions=5.0;  	// % embers that start fires
static double 	SpotIgnitionDelay=0.0;			// delay (minutes) for ignition of spot fires
static bool	CrowningOK=true;                   // enable crowning
static bool	SpottingOK=true;                   // enable spotting (just ember generation and flight)
static bool	SpotGrowthOK=false;                // enable spot fire growth
static bool 	ConstBack=false;                   // use constant backing spread rate (no wind no slope)
static long	CrownFireCalculation=0;			// 0=Finney (1998), 1=Scott&Reinhardt (2001)

static long 	numfires=0;					// number of fires
static long 	newfires=0;                      	// number of new fires
static long 	numspots=0;                      	// number of spot fires
static long 	skipfire=0;                      	// number of extinguished fires
static long 	p2numalloc=0;                    	// allocated space in perimeter2 array
static long*	GroundElev=0;					// stores elevation of points
static long 	numelev=0;

static long 	prevct;
static HANDLE 	hNewSpotSemaphore=0;
static HANDLE 	hNewPerimSemaphore=0;

static long	NumStopLocations=0;
static double	StopLocation[MAXNUM_STOPLOCATIONS*2];
static bool  	StopEnabled[MAXNUM_STOPLOCATIONS];

bool AccelerationON()
{
	return AccelerationState;
}

void SetAccelerationON(bool State)
{
	AccelerationState=State;
}


void InitializeRosRed()
{
	for(long i=0; i<257; i++)
		redros[i]=1.0;
}


double GetRosRed(int fuel)
{
	if(redros[fuel-1]>0.0)
		return redros[fuel-1];
	else
		return 1.0;
}

void SetRosRed(int fuel, double rosred)
{
	redros[fuel-1]=fabs(rosred);
}

long GetInout(long FireNumber)
{
	return inout[FireNumber];
}

void SetInout(long FireNumber, int Inout)
{
	inout[FireNumber]=Inout;
}

long GetNumPoints(long FireNumber)
{
	return numpts[FireNumber];
}

void SetNumPoints(long FireNumber, long NumPoints)
{
	numpts[FireNumber]=NumPoints;
}

double PercentIgnition(double percent)
{
	if(percent>=0.01 && percent<=100.0)
		PercentageOfEmberIgnitions=percent;
	else if(percent>=0.0 && percent<0.01)
		PercentageOfEmberIgnitions=0.01;
	else if(percent>100.0)
		PercentageOfEmberIgnitions=100.0;

	return PercentageOfEmberIgnitions;
}


double IgnitionDelay(double delay)
{
     if(delay>=0.0)
     	SpotIgnitionDelay=delay;

 	return SpotIgnitionDelay;
}


bool	EnableCrowning(long Crowning)
{
	if(Crowning>=0)
     	CrowningOK=(bool) Crowning;

	return CrowningOK;
}

long	GetCrownFireCalculation()
{
	return CrownFireCalculation;
}


long	SetCrownFireCalculation(long Type)
{
	CrownFireCalculation=Type;

	return CrownFireCalculation;
}


bool	EnableSpotting(long Spotting)
{
	if(Spotting>=0)
     	SpottingOK=(bool) Spotting;

	return SpottingOK;
}

bool	EnableSpotFireGrowth(long Growth)
{
	if(Growth>=0)
     	SpotGrowthOK=(bool) Growth;

	return SpotGrowthOK;
}

bool	ConstantBackingSpreadRate(long Back)
{
	if(Back>=0)
     	ConstBack=(bool) Back;

	return ConstBack;
}


bool CreateSpotSemaphore()
{
    	CloseSpotSemaphore();
     hNewSpotSemaphore=CreateSemaphore(NULL, 1, 1, "SPOTSEMAPHORE");
     hNewPerimSemaphore=CreateSemaphore(NULL, 1, 1, "NEWP1SEMAPHORE");

     if(hNewSpotSemaphore==NULL)
     	return false;
     if(hNewPerimSemaphore==NULL)
     	return false;

     return true;
}

void CloseSpotSemaphore()
{
	if(hNewSpotSemaphore!=NULL)
		CloseHandle(hNewSpotSemaphore);
     hNewSpotSemaphore=0;
	if(hNewPerimSemaphore!=NULL)
		CloseHandle(hNewPerimSemaphore);
     hNewPerimSemaphore=0;
}

long GetNumFires()
{
	return numfires;
}

void SetNumFires(long input)
{
	numfires=input;
}

void IncNumFires(long MoreFires)
{
	numfires+=MoreFires;
}

long GetNewFires()
{
	return newfires;
}

void SetNewFires(long input)
{
	WaitForSingleObject(hNewSpotSemaphore, INFINITE);
	newfires=input;
     numspots=input;
	ReleaseSemaphore(hNewSpotSemaphore, 1, &prevct);
}

void IncNewFires(long increment)
{
	WaitForSingleObject(hNewSpotSemaphore, INFINITE);
	newfires+=increment;
     numspots=newfires;
	ReleaseSemaphore(hNewSpotSemaphore, 1, &prevct);
}

void GetNumSpots(long *num, bool inc)
{
	WaitForSingleObject(hNewSpotSemaphore, INFINITE);
     *num=numspots;
     if(inc)
     {	numspots++;
          newfires=numspots;
     }
	ReleaseSemaphore(hNewSpotSemaphore, 1, &prevct);
}

void SetNumSpots(long input)
{
	WaitForSingleObject(hNewSpotSemaphore, INFINITE);
	numspots=input;
	ReleaseSemaphore(hNewSpotSemaphore, 1, &prevct);
}

long GetSkipFires()
{
	return skipfire;
}

void SetSkipFires(long newvalue)
{
	WaitForSingleObject(hNewPerimSemaphore, INFINITE);
	skipfire=newvalue;
	ReleaseSemaphore(hNewPerimSemaphore, 1, &prevct);
}

void IncSkipFires(long increment)
{
	WaitForSingleObject(hNewPerimSemaphore, INFINITE);
	skipfire+=increment;
	ReleaseSemaphore(hNewPerimSemaphore, 1, &prevct);
}

double GetPerimRes()
{
	return PerimeterResolution;
}

void SetPerimRes(double input)
{
	PerimeterResolution=input;
}


double GetDynamicDistRes()
{
	return DynamicDistanceResolution;
}


void SetDynamicDistRes(double input)
{
	DynamicDistanceResolution=input;
}


double GetDistRes()
{
	return DistanceResolution;
}

void SetDistRes(double input)
{
	DistanceResolution=DynamicDistanceResolution=input;      // set both default and dynamic min dist
}


double GetTemporaryTimeStep()
{
	return TemporaryTimestep;
}


void SetTemporaryTimeStep(double value)
{
	TemporaryTimestep=value;
}


double GetActualTimeStep()
{
	return actual;
}

void SetActualTimeStep(double input)
{
	actual=input;
}

double GetVisibleTimeStep()
{
	return visible;
}

void SetVisibleTimeStep(double input)
{
	long nuvis=input/actual;
	if(nuvis<1)
		nuvis=1;
	visible=nuvis*actual;
}


void SetSecondaryVisibleTimeStep(double input)
{
	secondaryvisible=input;
}


double GetSecondaryVisibleTimeStep()
{
	return secondaryvisible;
}


double EventMinimumTimeStep(double time)
{// returns EventMinTimeStep if "time" is negative
	if(time>=0)
		EventMinTimeStep=time;

	return EventMinTimeStep;
}


bool CheckExpansions(long YesNo)
{
	if(YesNo>-1)
		checkexpansions=YesNo;

	return checkexpansions;
}


bool CheckPostFrontal(long YesNo)
{
	if(YesNo>-1)
     	checkpostfrontal=YesNo;

     return checkpostfrontal;
}


long DistanceCheckMethod(long Method)
{
	if(Method>=0)
		DistanceMethod=Method;

	return DistanceMethod;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//	Fire perimeter2, swap space for fire growth calculations
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void GetPerimeter2(long coord, double *xpt, double *ypt, double *ros, double *fli, double *rct)
{
	if(coord<p2numalloc)
	{	coord*=NUMDATA;
		*xpt=perimeter2[coord];
		*ypt=perimeter2[++coord];
		*ros=perimeter2[++coord];
		*fli=perimeter2[++coord];
          *rct=perimeter2[++coord];
	}
}

double GetPerimeter2Value(long coord, long value)
{
	if(coord<0 || value<0)
		return (double) p2numalloc;
	else if(perimeter2 && coord<p2numalloc)
		return perimeter2[coord*NUMDATA+value];

	return 0.0; //(double) p2numalloc
}

void SetPerimeter2(long coord, double xpt, double ypt, double ros, double fli, double rct)
{
	if(coord<p2numalloc)
	{	coord*=NUMDATA;
		perimeter2[coord]=xpt;
		perimeter2[++coord]=ypt;
		perimeter2[++coord]=ros;
		perimeter2[++coord]=fli;
          perimeter2[++coord]=rct;
	}
}


double *AllocPerimeter2(long NumPoints)
{
	if(NumPoints)
	{    if(NumPoints>=p2numalloc)
     	{	FreePerimeter2();
	     	nmemb=NumPoints*NUMDATA;
               perimeter2=new double[nmemb];
               if(perimeter2!=NULL)
				p2numalloc=NumPoints;
     	}
	}
     else
	     return NULL;

     return perimeter2;
}


void FreePerimeter2()
{
	if(perimeter2)
		delete[] perimeter2;//GlobalFree(perimeter2);//free(perimeter2);
	perimeter2=0;
	p2numalloc=0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//	Must Call at begining of the program
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


void FreeAllFirePerims()
{
	if(perimeter1)
     	delete[] perimeter1;
     if(numpts)
     	delete[] numpts;
     if(inout)
     	delete[] inout;
     perimeter1=0;
	numpts=0;
     inout=0;
     NumPerimAlloc=0;
}


long GetNumPerimAlloc()
{
	return NumPerimAlloc;
}


bool AllocFirePerims(long num)
{
	FreeAllFirePerims();
     perimeter1 	= new double*[num];
     if(perimeter1==NULL)
		return false;
     numpts		= new long[num];
     if(numpts==NULL)
     	return false;
     inout		= new long[num];
     if(inout==NULL)
     	return false;
	NumPerimAlloc=num;
     ZeroMemory(perimeter1, num*sizeof(double *));
     ZeroMemory(numpts, num*sizeof(long));
     ZeroMemory(inout, num*sizeof(long));

     return true;
}


bool ReAllocFirePerims()
{
     long 	i, OldNumAlloc;
     long 	*newinout, *newnumpts;
     double 	*temp1;
	double 	**newperim1;

     newperim1=perimeter1;
     newinout=inout;
     newnumpts=numpts;

     perimeter1=0;
     inout=0;
     numpts=0;
     OldNumAlloc=NumPerimAlloc;

     if(!AllocFirePerims(NumPerimAlloc+SIZE_FIREPERIM_BLOCK))
     	return false;

	if(newinout)
	{    CopyMemory(inout, newinout, OldNumAlloc*sizeof(long));
		delete[] newinout;
     }
     if(newnumpts)
	{    CopyMemory(numpts, newnumpts, OldNumAlloc*sizeof(long));
		delete[] newnumpts;
     }

     if(newperim1)
     {    for(i=0; i<OldNumAlloc; i++)
		{	temp1=perimeter1[i];
     	   	perimeter1[i]=newperim1[i];
			if(numpts[i]>0)
	          	delete[] temp1;
	     }
     	delete[] newperim1;
     }


	return true;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//	Fire Perimeter1, main perimeter storage and retrieval functions
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

double *AllocPerimeter1(long NumFire, long NumPoints)
{
     WaitForSingleObject(hNewPerimSemaphore, INFINITE);
	if(NumPoints)
	{	if(NumFire>=NumPerimAlloc)
     	{    if(ReAllocFirePerims()==false)
			{	ReleaseSemaphore(hNewPerimSemaphore, 1, &prevct);

                    return NULL;
          	}
     	}
		nmemb=(NumPoints)*NUMDATA;			// add 1 to make room for bounding rectangle
          if(perimeter1[NumFire]>0)
          	FreePerimeter1(NumFire);
          perimeter1[NumFire]= new double[nmemb];

          if(perimeter1[NumFire]==NULL)
		{	NumFire=-1;		// debugging
			perimeter1[NumFire]=0;
               ReleaseSemaphore(hNewPerimSemaphore, 1, &prevct);

               return NULL;
          }
     }
	ReleaseSemaphore(hNewPerimSemaphore, 1, &prevct);

     return perimeter1[NumFire];
}

void FreePerimeter1(long NumFire)
{
	if(perimeter1[NumFire])
	{    delete[] perimeter1[NumFire];
		perimeter1[NumFire]=0;
	}
}

double GetPerimeter1Value(long NumFire, long NumPoint, int coord)
{
	if(perimeter1[NumFire])
		return perimeter1[NumFire][NumPoint*NUMDATA+coord];

	return 0.0;
}


double*	GetPerimeter1Address(long NumFire, long NumPoint)
{
	return &perimeter1[NumFire][NumPoint*NUMDATA];
}


void SetPerimeter1(long NumFire, long NumPoint, double xpt, double ypt)
{
//	WaitForSingleObject(hNewPerimSemaphore, INFINITE);
     NumPoint*=NUMDATA;
	perimeter1[NumFire][NumPoint]=xpt;
	perimeter1[NumFire][NumPoint+1]=ypt;
//	ReleaseSemaphore(hNewPerimSemaphore, 1, &prevct);
}

void SetFireChx(long NumFire, long NumPoint, double ros, double fli)
{
//	WaitForSingleObject(hNewPerimSemaphore, INFINITE);
     NumPoint*=NUMDATA;
	perimeter1[NumFire][NumPoint+2]=ros;
	perimeter1[NumFire][NumPoint+3]=fli;
//	ReleaseSemaphore(hNewPerimSemaphore, 1, &prevct);
}

void SetReact(long NumFire, long NumPoint, double react)
{
//	WaitForSingleObject(hNewPerimSemaphore, INFINITE);
	perimeter1[NumFire][NumPoint*NUMDATA+4]=react;
//	ReleaseSemaphore(hNewPerimSemaphore, 1, &prevct);
}

long SwapFirePerims(long NumFire1, long NumFire2)
{
     double *TempFire;
     long TempInout, TempNum;

     if(NumFire1>=0 && NumFire2>=0)         // two fires in perim1
     {    TempFire=perimeter1[NumFire2];
          perimeter1[NumFire2]=perimeter1[NumFire1];
          perimeter1[NumFire1]=TempFire;
          TempInout=inout[NumFire2];
          inout[NumFire2]=inout[NumFire1];
          inout[NumFire1]=TempInout;
          TempNum=numpts[NumFire2];
          numpts[NumFire2]=numpts[NumFire1];
          numpts[NumFire1]=TempNum;

          return 1;
     }
     else if(NumFire1<0 && NumFire2>=0)
     {    AllocPerimeter2(numpts[NumFire2]);
		if(MoveMemory(perimeter2, perimeter1[NumFire2], numpts[NumFire2]*NUMDATA*sizeof(double)))
               return 1;
          else
               return 0;
     }
     else if(NumFire1>=0 && NumFire2<0)
     {    if(perimeter1[NumFire1])
          {    if(MoveMemory(perimeter1[NumFire1], perimeter2, (NumFire2*-1)*NUMDATA*sizeof(double)))
                   return 1;
          }
          return 0;
     }

     return 0;
}


void AllocElev(long CurrentFire)
{
	nmemb=numpts[CurrentFire];
	if(nmemb>=(unsigned long) numelev)
	{	FreeElev();
        	GroundElev=new long[nmemb];//(long *) GlobalAlloc(GMEM_FIXED, nmemb*sizeof(long));
          numelev=nmemb;
     }
}


void SetElev(long Num, long elev)
{
	GroundElev[Num]=elev;
}

long GetElev(long Num)
{
     if(!GroundElev)
          return (long) NULL;

	return GroundElev[Num];
}

long *GetElevAddress(long Num)
{
     if(!GroundElev)
          return (long) NULL;

	return &GroundElev[Num];
}

void FreeElev()
{
	if(GroundElev)
		delete[] GroundElev;//GlobalFree(GroundElev);//free(GroundElev);
	GroundElev=0;
     numelev=0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//	Stop Location Functions
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


long SetStopLocation(double xcoord, double ycoord)
{
	if(NumStopLocations<MAXNUM_STOPLOCATIONS)
     {	StopLocation[NumStopLocations*2]=xcoord;
     	StopLocation[NumStopLocations*2+1]=ycoord;
          StopEnabled[NumStopLocations]=true;
     }
     else
     	return 0;

     return ++NumStopLocations;
}

bool	GetStopLocation(long StopNum, double *xcoord, double *ycoord)
{
	if(StopNum<MAXNUM_STOPLOCATIONS)
     {	*xcoord=StopLocation[StopNum*2];
     	*ycoord=StopLocation[StopNum*2+1];
     }
     if(StopEnabled[StopNum])
     	return true;

     return false;
}


void ResetStopLocation(long StopNum)
{
	//long i;

	if(StopNum<NumStopLocations)
     {	CopyMemory(&StopLocation[StopNum*2], &StopLocation[(StopNum+1)*2],
     		(NumStopLocations-StopNum-1)*sizeof(double)*2);
     	CopyMemory(&StopEnabled[StopNum], &StopEnabled[StopNum+1],
     		(NumStopLocations-StopNum-1)*sizeof(bool));
     	NumStopLocations--;
     }
}


bool	EnableStopLocation(long StopNum, long Action)
{
	if(Action>=0 && StopNum<NumStopLocations)
     	StopEnabled[StopNum]=(bool) Action;

     return (bool) StopEnabled[StopNum];
}


void ResetAllStopLocations()
{
	NumStopLocations=0;
     memset(StopEnabled, 0x0, MAXNUM_STOPLOCATIONS*sizeof(bool));
}

long	GetNumStopLocations()
{
	return NumStopLocations;
}



