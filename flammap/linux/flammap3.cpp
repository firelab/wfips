/*----------------------------------------------------------------------------
*  FlamMap version 2.0, Command Line
*  
*       Mark A. Finney, 2004
---------------------------------------------------------------------------*/

#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define BOOL_STR(b) (b?"true":"false")
#define strcmpi strcasecmp
const double PI=acos(-1.0);

#define PATH_MAX 4096

#include "flammap3.h"
#include "fsxsync.h"
#include "FlamMap_DLL.h"
//#include <atlimage.h>
//#include <AtlBase.h>
//#include <AtlConv.h>
//ALM #include "pdflib.hpp"
//#include "Histogram.h"
#include <iostream>
//#include "ogr_srs_api.h"
#include "ogr_spatialref.h" 
#include <string.h>
#include <fstream>
#include <unistd.h>
//using namespace std;

//char **wgs84WKT = "GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.01745329251994328,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4326"]]";
//#include "CImg.h"
//using namespace cimg_library;

#define FSPRO_VERSION false
#define NODESPREAD_VERSION true

#define max(a,b)      (((a) > (b)) ? (a) : (b))

#define min(a,b)      (((a) < (b)) ? (a) : (b))

int shortCompareAscending( const void *arg1, const void *arg2 )
{
	short d1, d2;
	d1 = *(short *)arg1;
	d2 = *(short *)arg2;
	if(d1 > d2)
		return 1;
	else if(d1 < d2)
		return -1;
	return 0;
}

/******************************************************************************
* Name: RunFlamMapThreads
* Desc: Check and run Dead Fuel Moisture Conditioning than start FlamMap
* NOTE: WindNinja was run before coming here, 
*   In: 
*  Ret: 1 ok
*       0 error see below 
******************************************************************************/
int FlamMap::RunFlamMapThreads()
{
int i; 
//printf("in RunFlammapThreads\n");
 	 time( &timeLaunch);

/*..........................................................*/
/* If inputs file had wind/weather, cond period inputs than do Condtioning   True means don't use Fuel moist cond. */
   /*if ( this->ConstFuelMoistures(GETVAL) == false ) {   // if not set to do Constant Fuel Moist 
     printf ("ConstFuelMoistures == false\n");
     runState = Conditioning;
     i = RunConditioning ();  // Load inputs, check & Run Conditioning DLL 
     if ( i <= 0 ) {          // Error, Abort, see FlamMap.cr_CondErrMes 
       return 0; }   
   }*/
   //printf ("ConstFuelMoistures == true\n");
	 TerminateBurn = false;
	// printf("in RunFlamMap TerminateBurn = %s\n", BOOL_STR(TerminateBurn));
  if (!TerminateBurn)	{
	  //printf("!TerminateBurn in RunFlammapthreads\n");
		  StartBurnThread();
		  //WaitForSingleObject(hBurnThread, INFINITE);
		 // CloseHandle(hBurnThread);
  }
	//printf("past !TerminateBurn in RunFlammapThreads\n");
 	time( &timeFinish);
  return 1;
}


/***********************************************************************/
CoarseWoody::CoarseWoody()
{
	 wd=0;
     NumClasses=0;
     Units=-1;
     TotalWeight=0.0;
     Depth=0.0;
}

CanopyCharacteristics::CanopyCharacteristics()
{
 	DefaultHeight=Height=15.0;           // need default for changing Variables in LCP
 	DefaultBase=CrownBase=4.0;
	 DefaultDensity=BulkDensity=0.20;
 	Diameter=20.0;
 	FoliarMC=100;
	 Tolerance=2;
 	Species=1;
}



CFMPRect::CFMPRect()
{
	left = top = right = bottom = 0L;
}

CFMPRect::CFMPRect(int l, int t, int r, int b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

int CFMPRect::Width()
{
	return abs(right - left) + 1;
}

int CFMPRect::Height()
{
	return abs(bottom - top) + 1;
}


/*********************************************************************/
FuelConversions::FuelConversions()
{
	int i;

	for(i=0; i<257; i++)             		// could also read default file here
		Type[i]=i;
	Type[99]=-1;
	Type[98]=-2;
	Type[97]=-3;
	Type[96]=-4;
	Type[95]=-5;
	Type[94]=-6;
	Type[93]=-7;
	Type[92]=-8;
	Type[91]=-9;
}

/****************************************************************************/
FlamMap::FlamMap()
{
  //strcpy (this->cr_CondErrMes,"");
  instanceID = 0;//++eventInstanceID;
  burn=new Burn(this);
	 CrownDensityLinkToCC=false;    // crown bulk density is a function of canopy cover
	 LandFileOpen=false;
	 landscape=0;
	 CantAllocLCP=false;
	 landfile=0;
	 headsize=sizeof(headdata);
	 OldFilePosition=0;
	 NEED_CUST_MODELS=false;	// custom fuel models
	 HAVE_CUST_MODELS=false;
	 NEED_CONV_MODELS=false;     // fuel model conversions
	 HAVE_CONV_MODELS=false;
	 //AtmGrid=0;			// pointer to AtmGrid;
	 LandFName[0] = 0;
	 WeightLossErrorTol=1.0;	// Mg/ha
 	MaxThreads=1;
	
	 FuelModelUnits=0;
	 NumWeatherStations=0;
  ConstFuelMoisture=1;
	 CombineOption=CWD_COMBINE_ABSENT;
 	CondPeriod=false;
 	OutputDirection=RELATIVEDIR;

	 for(int i = 0; i < NUM_STATICOUTPUTS; i++)
		 outlayer[i] = NULL;

	FlameAceBaseName[0] = NULL;
	WindFileName[0] = NULL;
	WeatherFileName[0] = NULL;
	FuelMoistureFileName[0] = NULL;
	FuelModelName[0] = NULL;
	OffsetFromMax=0.0;	// offset in fire spread direction from Rmax
	ConstWindDirection=-1.0;
	ConstWindSpeed=0.0;
	InactiveEnclaves=true;
	DownTime=0.0;
	//1hr
	MoistCalcInterval[0][0] = 60;
	MoistCalcInterval[0][1] = 200;
	MoistCalcInterval[0][2] = 10;
	MoistCalcInterval[0][3] = 45;
	MoistCalcInterval[0][4] = 15;
	//10hr
	MoistCalcInterval[1][0] = 60;
	MoistCalcInterval[1][1] = 200;
	MoistCalcInterval[1][2] = 10;
	MoistCalcInterval[1][3] = 45;
	MoistCalcInterval[1][4] = 15;
	//100hr
	MoistCalcInterval[2][0] = 120;
	MoistCalcInterval[2][1] = 400;
	MoistCalcInterval[2][2] = 10;
	MoistCalcInterval[2][3] = 45;
	MoistCalcInterval[2][4] = 20;
	//1000hr
	MoistCalcInterval[3][0] = 240;
	MoistCalcInterval[3][1] = 400;
	MoistCalcInterval[3][2] = 15;
	MoistCalcInterval[3][3] = 45;
	MoistCalcInterval[3][4] = 30;

	for(int i = 0; i < 4; i++)
		for(int j = 0; j < 5; j++)
			 EnvtChanged[i][j] = false;

 ResetNewFuels();
	analysisLoEast = analysisHiEast = analysisLoNorth = analysisHiNorth = 0L;
 timeLaunch = timeFinish = 0;;
	/*ALM
 for(int t = 0; t < 10; t++)
	 	legends[t] = NULL;
		*/
	ConstBack = false;
	DynamicDistanceResolution=0.0;
	AccelerationState=false;
	CrowningOK = true;
	CrownFireCalculation = 0;
	SpottingOK = true;
	//wind grids
	windDirGrid = windSpeedGrid = NULL;
	nWindRows = nWindCols = 0L;
	windsResolution = 0.0;
	hasGriddedWinds = false;
	for(int i = 0; i < 64; i++)
	 	ThreadProgress[i] = 0.0;
	runState = Stopped;
	conditioningProgress = 0.0;
	critsec=0;
	CreateLandscapeFileCS();
	StartProcessor = 0;
}


/*****************************************************************************/
FlamMap::~FlamMap()
{
int i;

	 CloseLandFile();
  for(i=0; i< NUM_STATICOUTPUTS; i++)
     FreeOutputLayer(i);
  	LandFileOpen=false;

	 for(i=0; i<NUM_STATICOUTPUTS; i++)
     	SelectOutputLayer(i, false);

  if(burn)
    delete burn;

  CloseLandFile();

  //FreeFarsiteEvents(EVENT_BURN);
  //FreeFarsiteEvents(EVENT_MOIST);
  //FreeFarsiteEvents(EVENT_BURN_THREAD);
  //FreeFarsiteEvents(EVENT_MOIST_THREAD);
  /*
	 for(int t = 0; t < 10; t++) {
		   if(legends[t]){
		    	delete legends[t];
			    legends[t] = NULL; }	}
				ALM*/
 	DeleteWindGrids();
 	FreeLandscapeFileCS();
}

/************************************************************************/
bool FlamMap::CreateLandscapeFileCS()
{
	FreeLandscapeFileCS();
	//InitializeCriticalSection(&lcpCS);
	critsec=1;

     return true;
}


void FlamMap::FreeLandscapeFileCS()
{
	//if(critsec)
		//DeleteCriticalSection(&lcpCS);
	critsec=0;
}

void FlamMap::SetInstanceID(int id)
{
	instanceID = id;
}

void FlamMap::DeleteWindGrids()
{
	if(windSpeedGrid)
	{
		for(int i = 0; i < nWindRows; i++)
			delete[] windSpeedGrid[i];
		delete[] windSpeedGrid;
		windSpeedGrid = NULL;
	}
	if(windDirGrid)
	{
		for(int i = 0; i < nWindRows; i++)
			delete[] windDirGrid[i];
		delete[] windDirGrid;
		windDirGrid = NULL;
	}
	windsXllCorner = windsYllCorner = windsResolution = 0.0;
	nWindRows = nWindCols = 0L;
	hasGriddedWinds = false;

}
void FlamMap::AllocWindGrids(int wndRows, int wndCols, double wndRes, double wndXLL, double wndYLL)
{
	DeleteWindGrids();
	if(wndRows == 0 || wndCols == 0)
		return;
	nWindRows = wndRows;
	nWindCols = wndCols;
	windSpeedGrid = new float *[nWindRows];
	windDirGrid = new float *[nWindRows];
	windsXllCorner = wndXLL;
	windsYllCorner = wndYLL;
	windsResolution = wndRes;
	for(int r = 0; r < nWindRows; r++)
	{
		windSpeedGrid[r] = new float[nWindCols];
		windDirGrid[r] = new float[nWindCols];
		for(int c = 0; c < nWindCols; c++)
		{
			windSpeedGrid[r][c] = windDirGrid[r][c] = NODATA_VAL;
		}
	}
	hasGriddedWinds = true;
}
void FlamMap::SetWindGridValues(int wndRow, int wndCol, float wndDir, float wndSpeed)
{
	if(wndRow < 0 || wndRow >= nWindRows || wndCol < 0 || wndCol >= nWindCols)
		return;
	windSpeedGrid[wndRow][wndCol] = wndSpeed;
	windDirGrid[wndRow][wndCol] = wndDir;
}

float FlamMap::GetWindGridSpeed(int wndRow, int wndCol)
{
	if(wndRow < 0 || wndRow >= nWindRows || wndCol < 0 || wndCol >= nWindCols)
		return 0;
	return windSpeedGrid[wndRow][wndCol];
}

float FlamMap::GetWindGridDir(int wndRow, int wndCol)
{
	if(wndRow < 0 || wndRow >= nWindRows || wndCol < 0 || wndCol >= nWindCols)
		return 0;
	return windDirGrid[wndRow][wndCol];
}
float FlamMap::GetWindGridSpeedByCoord(double xCoord, int yCoord)
{
	if(windsResolution <= 0)
		return NODATA_VAL;
	int r = (yCoord - windsYllCorner) / windsResolution;
	int c = (xCoord - windsXllCorner) / windsResolution;
	return GetWindGridSpeed(r, c);
}

float FlamMap::GetWindGridDirByCoord(double xCoord, int yCoord)
{
	if(windsResolution <= 0)
		return NODATA_VAL;
	int r = (yCoord - windsYllCorner) / windsResolution;
	int c = (xCoord - windsXllCorner) / windsResolution;
	return GetWindGridDir(r, c);
}

/************************************************************************/
void FlamMap::ResetThreads()
{
//  burn->env->ResetAllThreads();
 //ALM FreeFarsiteEvents(EVENT_BURN);
  //FreeFarsiteEvents(EVENT_MOIST);

  //ALM FreeFarsiteEvents(EVENT_BURN_THREAD);
  //FreeFarsiteEvents(EVENT_MOIST_THREAD);

	 char eventInstanceStr[64];
	 sprintf(eventInstanceStr, "FlamEventBurn_%ld", instanceID);
  //ALM AllocFarsiteEvents(EVENT_BURN, GetMaxThreads(), eventInstanceStr, false, false);
  //AllocFarsiteEvents(EVENT_BURN, GetMaxThreads(), "FlamEventBurn", false, false);
//	 sprintf(eventInstanceStr, "FlamEventMoist_%ld", instanceID);
  //AllocFarsiteEvents(EVENT_MOIST, GetMaxThreads(), eventInstanceStr, false, false);

	 sprintf(eventInstanceStr, "FlamEventBurnThread_%ld", instanceID);
  //ALM AllocFarsiteEvents(EVENT_BURN_THREAD, GetMaxThreads(), eventInstanceStr, true, false);
  //AllocFarsiteEvents(EVENT_BURN_THREAD, GetMaxThreads(), "FlamEventBurnThread", true, false);
//	 sprintf(eventInstanceStr, "FlamEventMoistThread_%ld", instanceID);
 // AllocFarsiteEvents(EVENT_MOIST_THREAD, GetMaxThreads(), eventInstanceStr, true, false);

	 for(int i = 0; i < 64; i++)
		  ThreadProgress[i] = 0.0;
	 printf("TerminateBurn = False in ResetThreads\n");
	 TerminateBurn = TerminateMoist = false;
	 conditioningProgress = 0.0;
}

/*************************************************************************/
void FlamMap::StartBurnThread()
{
	//printf("in StartBurnThreads\n");
	//if (outFiles[0] != NULL)printf("pFlamMap->outFiles[layerNum] is openc\n");
 	runState = FireBehavior;
  unsigned int BurnThreadID;
  TerminateBurn=false;
  //ALMhBurnThread=(HANDLE) ::_beginthreadex(NULL, 0, &FlamMap::RunBurnThread, this, NULL, &BurnThreadID);
  FlamMap::RunBurnThread(this);//ALM
}

/***************************************************************************/
unsigned int FlamMap::RunBurnThread(void *FarIntFace)
{
	//if (outFiles[0] != NULL)printf("pFlamMap->outFiles[layerNum] is openc\n");
	//printf("in RunBurnThread\n");
	 static_cast <FlamMap *> (FarIntFace)->BurnThread();
   return 1;
}

/*******************************************************************************/
void FlamMap::ResetBurn()
{
	if(burn)
	 	delete burn;
	burn = new Burn(this);
}

/*******************************************************************************/
void FlamMap::BurnThread()
{
	//if (outFiles[0] != NULL)printf("pFlamMap->outFiles[layerNum] is openc\n");
  TerminateBurn=false;
  //printf("in BurnThread\n");
	 burn->StartLandscapeThreads();   // need way to display progress from all threads
  if ( !TerminateBurn )
		   runState = Complete;
	 else
		   runState = Stopped;
	//printf("TerminateBurn = True in BurnThread\n");
	 TerminateBurn=true;
}




/****************************************************************************/
int FlamMap::SetMoistures(int fuelModel, int _fm1, int _fm10, int _fm100,
							 int _fmHerb, int _fmWoody)
{
	if(fuelModel < 1 || fuelModel > 256)
		return 0;
	fm[fuelModel - 1].TL1 = _fm1;
	fm[fuelModel - 1].TL10 = _fm10;
	fm[fuelModel - 1].TL100 = _fm100;
	fm[fuelModel - 1].TLLH = _fmHerb;
	fm[fuelModel - 1].TLLW = _fmWoody;
	if(fm[fuelModel - 1].TL1 > 1 && fm[fuelModel - 1].TL10 > 1)
		fm[fuelModel - 1].FuelMoistureIsHere=true;
	else
		fm[fuelModel - 1].FuelMoistureIsHere=false;

	for(int k=0; k<4; k++)       // only up to 1000 hr fuels [3]
	{    
		for(int j=0; j<5; j++)
			EnvtChanged[k][j]=true;
	}
	return 1;
}

void FlamMap::SetAllMoistures(int _fm1, int _fm10, int _fm100,
		int _fmHerb, int _fmWoody)
{
	int i, j, k;
	for(i = 0; i <= 256; i++)
	{
		fm[i].TL1 = _fm1;
		fm[i].TL10 = _fm10;
		fm[i].TL100 = _fm100;
		fm[i].TLLH = _fmHerb;
		fm[i].TLLW = _fmWoody;
		if(fm[i].TL1>1 && fm[i].TL10>1)
			fm[i].FuelMoistureIsHere=true;
		else
			fm[i].FuelMoistureIsHere=false;

		for(k=0; k<4; k++)       // only up to 1000 hr fuels [3]
		{    
			for(j=0; j<5; j++)
				EnvtChanged[k][j]=true;
		}
	}
}



bool FlamMap::InquireOutputSelections()
{	// need to have some outputs selected
	int i;

     for(i=0; i<NUM_STATICOUTPUTS; i++)
     {    if(GetOutputOption(i))
     		return true;
     }

	return false;
}

bool FlamMap::OpenLandFile()
{
//	SetCanopyChx(15.0, 4.0, 0.2, 30.0, 100, 2, 1);
	if(landfile)
     	fclose(landfile);
     if(landscape)
     {	delete[] landscape;//GlobalFree(landscape);//free(landscape);
          landscape=0;
     	CantAllocLCP=false;
     }
	if((landfile=fopen(LandFName, "rb"))==NULL)
	{    landfile=0;

		return false;
     }

	return true;
}


bool FlamMap::LoadLCPHeader(headdata head)
{
	Header = head;
	return true;
}

bool FlamMap::LoadLandscape(short *landscape2)
{
	landscape = landscape2;
	return true;
}

bool FlamMap::LoadLandscapeFile(char *FileName, CFlamMap *caller)
{
     CloseLandFile();
	SetLandFileName(FileName);
	char *p = strrchr(FileName, '.');
	if(strcmpi(p, ".lcp") != 0)
	{
		if(ReadGeoTiff(FileName) == 1)
			LandFileOpen = true;
		else
			return false;
	}
	else if(ReadGdalLCP(FileName) == 1)
		LandFileOpen =  true;
	else
		return false;
    /*  if(!OpenLandFile())
     {	//::MessageBox(HWindow, FileName,
     	//	"Error Loading LCP", MB_OK);
          //lcp->SetCheck(BF_UNCHECKED);
     	//tlcp->SetText("");

     	return false;
     }*/

     //LandFileOpen=true;
     //ReadHeader();
     SetCustFuelModelID(HaveCustomFuelModels());
     SetConvFuelModelID(HaveFuelConversions());
	//create default legends....
	/*ALM for(int i = 0; i < 10; i++)
	 {
		 legends[i] = new CLegendData(caller, i);
		 legends[i]->LoadDefault();
	 }
	 */
     return true;
}


bool FlamMap::LoadFuelMoistureFile(char *FileName)
{

	int ModNum, F1, F10, F100, FLW, FLH;
     FILE *CurrentFile;

     if((CurrentFile=fopen(FileName, "r"))==NULL)
     	return false;

	while(!feof(CurrentFile))
     {    fscanf(CurrentFile,"%ld", &ModNum);
          if(feof(CurrentFile))
               break;
          fscanf(CurrentFile, "%ld %ld %ld %ld %ld", &F1, &F10, &F100, &FLH, &FLW);
		if(ModNum>256)
		{	//::MessageBox(HWindow, "Correct Fuel Moisture File Before Proceeding",
			//		"Error: Model Number>50", MB_OK);
               //fms->SetCheck(BF_UNCHECKED);
               //tfms->SetText("");

			return false;
		}
		if(F1<=0 || F10<=0 || F100<=0 || FLW<=0 || FLH<=0)
		{	//::MessageBox(HWindow, "Correct Before Proceeding",
			//		"Error: Fuel Moisture <=0", MB_OK);
               //fms->SetCheck(BF_UNCHECKED);
               //tfms->SetText("");

			return false;
		}
		if(ModNum == 0)
			SetAllMoistures(F1, F10, F100, FLH, FLW);
		else
          SetInitialFuelMoistures(ModNum, F1, F10, F100, FLH, FLW);
	}
     fclose(CurrentFile);

     return true;
}

/* 88888888888888888888888888888888888888888888888888888888888888888888888888888888888888 */
int FlamMap::LoadCustomFuelFile(char *FileName)
{
	int ret = 1;
	SetCustFuelModelID(false);
	char *ptr, dynamic[32]="", code[32]="";
     char Line[256]="", head[64]="", ErrMsg[256]="", BackupFile[256]="", comment[256]="";
     bool Metric=false, BadFile=false;
	int num, count, ModNum, FileFormat=0;
     double s1, slh, slw;
     NewFuel newfuel, newfuelm;
     FILE *fout;
     FILE *CurrentFile;

     CurrentFile=fopen(FileName, "rt");
     memset(&newfuel, 0x0, sizeof(NewFuel));
     memset(&newfuelm, 0x0, sizeof(NewFuel));
     do
     {    rewind(CurrentFile);
     	memset(head, 0x0, 64*sizeof(char));
     	memset(Line, 0x0, sizeof(Line));
	     fgets(Line, 255, CurrentFile);
     	sscanf(Line, "%s", head);
	    	AccessFuelModelUnits(0);
		int temp=0;
		  char c;
		  char tempChar[64];
		  while (head[temp])
		  {
			c=head[temp];
			tempChar[temp] = tolower(c);
			temp++;
		  }
		  int temp2 = 0;
		  while (tempChar[temp2])
		  {
			head[temp2] = tempChar[temp2];
			temp2++;
		  }
     	//ALM if(!strcmp(_strlwr(head), "metric"))
		if(!strcmp(head, "metric"))
	     {    Metric=true;
     	     AccessFuelModelUnits(1);
	     }
     	else if(strcmp(head, "english"))   // no header in file
	     {    if(atol(head)>256)
     	     {     //::MessageBox(Client->HWindow, "Bad Header in File", "Custom Fuel Model File Error", MB_OK);
          	     fclose(CurrentFile);

	               return false;
     	     }
               memset(head, 0x0, 64*sizeof(char));
     		rewind(CurrentFile);
	     }
          if(FileFormat>0)
          {    if(FileFormat==1)
          	{  	strcat(BackupFile, FileName);
	               strcat(BackupFile, ".old");
     	          //ALM CopyFile( FileName,  BackupFile, false);
				   const char * tempfilename = (const char *)FileName;
				   std::ifstream ifs(tempfilename, std::ios::binary);
				   std::ofstream ofs(BackupFile, std::ios::binary);
				   ofs << ifs.rdbuf();
          	     fclose(CurrentFile);
               	CurrentFile=fopen(BackupFile, "r");//alm was _access
			     if((access(FileName, 02))==-1)
			     {	//SetFileAttributes( FileName, FILE_ATTRIBUTE_NORMAL);
					 chmod(FileName, 777);
				     //DeleteFile( FileName);
					 remove(FileName);
			     }
                    fout=fopen(FileName, "w");
                    if(Metric)
                    	fprintf(fout, "METRIC\n");
                    else
                    	fprintf(fout, "ENGLISH\n");
               }
               else
               	fout=NULL;
          	break;
          }

	     memset(Line, 0x0, 256*sizeof(char));
	     fgets(Line, 255, CurrentFile);
	     if(feof(CurrentFile))
	         	break;
	     if(strlen(Line)==0 || !strncmp(Line, "\n", 1))
	         	continue;
	     num=sscanf(Line, "%ld %s %lf %lf %lf %lf %lf %s",
	               &ModNum, head, &newfuel.h10, &newfuel.h100,
	               &newfuel.lh, &newfuel.lw, &s1, comment);
          //if(!strcmp(_strlwr(comment), "dynamic"))
          //  	FileFormat=2;
          //else if(!strcmp(_strlwr(comment), "static"))
          //	FileFormat=2;
          //else
		 int temp3=0;
		  char c3;
		  char tempChar2[64];
		  while (comment[temp3])
		  {
			c3=comment[temp3];
			tempChar[temp3] = tolower(c3);
			temp3++;
		  }
		  int temp4 = 0;
		  while (tempChar2[temp4])
		  {
			comment[temp4] = tempChar2[temp4];
			temp4++;
		  }
          if(strstr(comment, "d"))
          	FileFormat=2;
          else if(strstr(comment, "s"))
          	FileFormat=2;
          else
          	FileFormat=1;
     } while(FileFormat>0);

     rewind(CurrentFile);
     memset(head, 0x0, 64*sizeof(char));
     fgets(Line, 255, CurrentFile);
    	sscanf(Line, "%s", head);
		int temp=0;
		  char c;
		  char tempChar[64];
		  while (head[temp])
		  {
			c=head[temp];
			tempChar[temp] = tolower(c);
			temp++;
		  }
		  int temp2 = 0;
		  while (tempChar[temp2])
		  {
			head[temp2] = tempChar[temp2];
			temp2++;
		  }
    	if(strcmp(head, "metric") && strcmp(head, "english"))
     	rewind(CurrentFile);

     count=0;
	while(!feof(CurrentFile))
	{    memset(Line, 0x0, 256*sizeof(char));
     	fgets(Line, 255, CurrentFile);
          if(strlen(Line)==0 || !strncmp(Line, "\n", 1))
          	continue;
          //if(feof(CurrentFile))
          //	break;
          memset(comment, 0x0, 256*sizeof(char));
          if(FileFormat==1)
          {	num=sscanf(Line, "%ld %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %s",
               	&ModNum, &newfuel.h1, &newfuel.h10, &newfuel.h100,
	               &newfuel.lh, &newfuel.lw, &s1, &slh, &slw,
     	          &newfuel.depth, &newfuel.xmext, &newfuel.heatd, &newfuel.heatl, comment);
          	if(num<13)
          		break;
               sprintf(newfuel.code, "FM%ld", ModNum);
               newfuel.dynamic=0;
          }
          else
          {    memset(code, 0x0, 32*sizeof(char));
          	memset(dynamic, 0x0, 32*sizeof(char));
          	num=sscanf(Line, "%ld %s %lf %lf %lf %lf %lf %s %lf %lf %lf %lf %lf %lf %lf %s",
               	&ModNum, code, &newfuel.h1, &newfuel.h10, &newfuel.h100,
	               &newfuel.lh, &newfuel.lw, dynamic, &s1, &slh, &slw,
     	          &newfuel.depth, &newfuel.xmext, &newfuel.heatd, &newfuel.heatl, comment);
          	if(num<15)
          		break;

		  int tempa=0;
		  char ca;
		  char tempChara[64];
		  while (dynamic[tempa])
		  {
			ca=dynamic[tempa];
			tempChara[tempa] = toupper(ca);
			tempa++;
		  }
		  int temp2b = 0;
		  while (tempChara[temp2b])
		  {
			dynamic[temp2b] = tempChara[temp2b];
			temp2b++;
		  }
              //if(!strcmp(_strupr(dynamic), "DYNAMIC"))
		  if(!strcasecmp(dynamic, "DYNAMIC"))
              	newfuel.dynamic=1;
			int temp=0;
		  char c;
		  char tempChar[64];
		  while (dynamic[temp])
		  {
			c=dynamic[temp];
			tempChar[temp] = tolower(c);
			temp++;
		  }
		  int temp2 = 0;
		  while (tempChar[temp2])
		  {
			dynamic[temp2] = tempChar[temp2];
			temp2++;
		  }
          	if(strstr(dynamic, "d"))
               	newfuel.dynamic=1;
               else
               	newfuel.dynamic=0;
               strncpy(newfuel.code, code, 7);
		}

          newfuel.sav1=(int) s1;
          newfuel.savlh=(int) slh;
          newfuel.savlw=(int) slw;
          memset(newfuel.desc, 0x0, 256*sizeof(char));
          if(strlen(comment)>0)
	     {    ptr=strstr(Line, comment);
          	strncpy(newfuel.desc, ptr, 64);
          }
          if(Metric)     // convert to english
          {    memcpy(&newfuelm, &newfuel, sizeof(NewFuel));
          	newfuel.h1/=2.2417;
               newfuel.h10/=2.2417;
               newfuel.h100/=2.2417;
               newfuel.lh/=2.2417;
               newfuel.lw/=2.2417;
               newfuel.sav1=s1*30.480060960;
               newfuel.savlh=slh*30.480060960;
               newfuel.savlw=slw*30.480060960;
               newfuel.depth/=30.480060960;
               newfuel.heatd/=2.324375;
               newfuel.heatl/=2.324375;
          }
          count++;
		if(ModNum<14 || ModNum>256)
		{    sprintf(ErrMsg, "Model Data Error Line => %ld", count);
          	//::MessageBox(Client->HWindow, "Fuel Model Number >256 or <14", ErrMsg, MB_OK);
               BadFile=true;
			   ret = e_EMS_CF_FM;
		}
		else if(newfuel.xmext<=0.0)
		{    sprintf(ErrMsg, "Model Data Error Line => %ld", count);
          	//::MessageBox(Client->HWindow, "Extinction Moisture = 0", ErrMsg, MB_OK);
               BadFile=true;
			   ret = e_EMS_CF_EM;
		}
          else if(newfuel.h1+newfuel.h10+newfuel.h100<=0.0)
          {    sprintf(ErrMsg, "Model Data Error Line => %ld", count);
          	//::MessageBox(Client->HWindow, "Fuel Model Has No Dead Fuel", ErrMsg, MB_OK);
               BadFile=true;
			   ret = e_EMS_CF_DF;
          }
		else if(newfuel.depth<=0.0)
		{    sprintf(ErrMsg, "Model Data Error Line => %ld", count);
          	//::MessageBox(Client->HWindow, "Depth = 0.0", ErrMsg, MB_OK);
               BadFile=true;
			   ret = e_EMS_CF_DEPTH;
		}
		else if(newfuel.heatl<6000)
		{    sprintf(ErrMsg, "Model Data Error Line => %ld", count);
          	//::MessageBox(Client->HWindow, "Live Heat Content Too Low", ErrMsg, MB_OK);
               BadFile=true;
			   ret = e_EMS_CF_LH;
		}
		else if(newfuel.heatd<4000)
		{    sprintf(ErrMsg, "Model Data Error Line => %ld", count);
          	//::MessageBox(Client->HWindow, "Dead Heat Content Too Low", ErrMsg, MB_OK);
               BadFile=true;
			   ret = e_EMS_CF_DH;
		}
		else if(newfuel.sav1>4000 || newfuel.savlh>4000 || newfuel.savlw>4000)
		{    sprintf(ErrMsg, "Model Data Error Line => %ld", count);
          	//::MessageBox(Client->HWindow, "SAV Ratios Out of Range", ErrMsg, MB_OK);
               BadFile=true;
			   ret = e_EMS_CF_SAV;
		}

          if(BadFile)
          {    fclose(CurrentFile);
			return ret;
          }
		newfuel.xmext/=100.0;
          newfuelm.xmext=newfuel.xmext;
          newfuel.number=newfuelm.number=ModNum;
          SetNewFuel(&newfuel);
	     if(FileFormat==1 && fout!=NULL)
          {    if(newfuel.dynamic==0)
               	sprintf(dynamic, "static");
          	else
               	sprintf(dynamic, "dynamic");
          	if(Metric)
	     		fprintf(fout, "%ld %s %lf %lf %lf %lf %lf %s %ld %ld %ld %lf %ld %lf %lf %s\n",
     	          	newfuelm.number, newfuelm.code, newfuelm.h1, newfuelm.h10, newfuelm.h100,
	     	          newfuelm.lh, newfuelm.lw, dynamic, newfuelm.sav1, newfuelm.savlh, newfuelm.savlw,
     	     	     newfuelm.depth, (int) (newfuelm.xmext*100.0), newfuelm.heatd, newfuelm.heatl, newfuelm.desc);
               else
	     		fprintf(fout, "%ld %s %lf %lf %lf %lf %lf %s %ld %ld %ld %lf %ld %lf %lf %s\n",
     	          	newfuel.number, newfuel.code, newfuel.h1, newfuel.h10, newfuel.h100,
	     	          newfuel.lh, newfuel.lw, dynamic, newfuel.sav1, newfuel.savlh, newfuel.savlw,
     	     	     newfuel.depth, (int) (newfuelm.xmext*100.0), newfuel.heatd, newfuel.heatl, newfuel.desc);
          }
	}
	fclose(CurrentFile);
     if(fout)
     {	fclose(fout);
   		//::MessageBox(Client->HWindow, "Existing File Renamed with .OLD extension",
		//	"Custom Fuel Model File Converted to new Format", MB_OK);
     }

	SetCustFuelModelID(true);

	return 1;//success
}

/* 9999999999999999999999999999999999999999999999999999999999999999999999999999999999 */







bool FlamMap::LoadWindFile(char *FileName)
{

#ifdef wowowwow
int count=0;
double wss, month, wwwinddir, day, hhour, cloudcover;
bool Metric=false, BadData=false;
int i, fpos=0;
char UnitsString[256]="", ErMsg[256]="";
FILE *CurrentFile;

     FreeWindData(0);

     if((CurrentFile=fopen(FileName, "r"))==NULL)
     	return false;

     fscanf(CurrentFile, "%s", UnitsString);
     if(!strcmp(_strlwr(UnitsString), "metric"))
     {    Metric=true;
          fpos=ftell(CurrentFile);
     }
     else if(!strcmp(_strlwr(UnitsString), "english"))
     {    Metric=false;
          fpos=ftell(CurrentFile);
     }
     else
          rewind(CurrentFile);

	while(!feof(CurrentFile))
	{    fscanf(CurrentFile, "%lf", &month);
          if(feof(CurrentFile))
               break;
          fscanf(CurrentFile,"%lf %lf %lf %lf %lf", &day, &hhour, &wss, &wwwinddir, &cloudcover);
		count++;
		if(month<1 || month>12)
		{    sprintf(ErMsg, "%s %ld", "Month Out of Range, Line ->", count);
               //::MessageBox(HWindow, ErMsg, " Bad Wind File", MB_OK);
               BadData=true;
          }
          else if(day>31 || day<1)
		{    sprintf(ErMsg, "%s %ld", "Day Out of Range, Line ->", count);
               //::MessageBox(HWindow, ErMsg, " Bad Wind File", MB_OK);
               BadData=true;
          }
          else if(hhour<0 || hhour>2359)
		{    sprintf(ErMsg, "%s %ld", "Hour Out of Range, Line ->", count);
               //::MessageBox(HWindow, ErMsg, " Bad Wind File", MB_OK);
               BadData=true;
          }
          else if(cloudcover>100 || cloudcover<0)
		{    sprintf(ErMsg, "%s %ld", "Cloud Cover Out of Range, Line ->", count);
               //::MessageBox(HWindow, ErMsg, " Bad Wind File", MB_OK);
               BadData=true;
          }

          if(BadData)
          {    fclose(CurrentFile);
          	//wnd->SetCheck(BF_UNCHECKED);

			return false;
		}
	}
	int StationNumber=AllocWindData(0, count);
	if (StationNumber>=0)	{   
   fseek (CurrentFile, fpos, SEEK_SET);
	 	for (i=0; i<count; i++)	{  
      fscanf(CurrentFile,"%lf %lf %lf %lf %lf %lf", &month, &day, &hhour, &wss, &wwwinddir, &cloudcover);            
      if (Metric)
        wss*=0.5402; //(0.62125/1.15);     10m wind kph to 20ft wind mph
			   SetWindData(StationNumber, i, (int) month, (int) day, (int) hhour,
               	 	(int) wss, (int) wwwinddir, (int) cloudcover);
		 }
		 SetWindData(StationNumber, i, 13, (int) day, (int) hhour, (int) wss,
          			 (int) wwwinddir, (int) cloudcover); // signal end of wind data
		 fclose(CurrentFile);
	}

#endif 
  return true;
}





void FlamMap::SelectOutput(int LayerID, bool On)
{
     SelectOutputLayer(LayerID, On);
}


bool FlamMap::LoadOutputFile(char *FileName)
{
     if(FSPRO_VERSION)
     {	
		 SelectOutput(MAXSPREADDIR, true);
	     SelectOutput(ELLIPSEDIM_A, true);
     	SelectOutput(ELLIPSEDIM_B, true);
	     SelectOutput(ELLIPSEDIM_C, true);
     	SelectOutput(MAXSPOT, true);
     }
     else
     {    if(NODESPREAD_VERSION)
		{	SelectOutput(MAXSPREADDIR, true);
			SelectOutput(ELLIPSEDIM_A, true);
     		SelectOutput(ELLIPSEDIM_B, true);
			SelectOutput(ELLIPSEDIM_C, true);
     		SelectOutput(MAXSPOT, true);
			SelectOutput(SPREADRATE, true);
     		SelectOutput(INTENSITY, true);
			SelectOutput(CROWNSTATE, true);
		}
		else
		{	SelectOutput(FLAMELENGTH, true);
     		SelectOutput(SPREADRATE, true);
     		SelectOutput(INTENSITY, true);
			//SelectOutput(HEATAREA, true);
     		SelectOutput(CROWNSTATE, true);
			//SelectOutput(SOLARRADIATION, true);
     		//SelectOutput(FUELMOISTURE1, true);
			//SelectOutput(FUELMOISTURE10, true);
     		//SelectOutput(MIDFLAME, true);
			//SelectOutput(HORIZRATE, true);
		}
     }
     SetOutputBaseName(FileName);

     return true;
}




#ifdef wowowowo
bool FlamMap::LoadDateFile(char *FileName)
{
     int smo, sday, shour, emo, eday, ehour;
     char krap[256]="";
     FILE *fin=fopen(FileName, "r");

     if(fin==0)
     {	///printf("File Not Found %s, Not using conditioning period\n\n");

		return true;
     }
     fscanf(fin, "%s %ld\n", krap, &smo);
     fscanf(fin, "%s %ld\n", krap, &sday);
     fscanf(fin, "%s %ld\n", krap, &shour);
     fscanf(fin, "%s %ld\n", krap, &emo);
     fscanf(fin, "%s %ld\n", krap, &eday);
     fscanf(fin, "%s %ld\n", krap, &ehour);

     fclose(fin);

     return SetDates(smo, sday, shour, emo, eday, ehour);
}
#endif 

void FlamMap::SetCanopyChx(double Height, double CrownBase, double BulkDensity, double Diameter,
			   int FoliarMoisture, int Tolerance, int Species)
{
	CanopyChx.Height=CanopyChx.DefaultHeight=Height;
	CanopyChx.CrownBase=CanopyChx.DefaultBase=CrownBase;
	CanopyChx.BulkDensity=CanopyChx.DefaultDensity=BulkDensity;
	CanopyChx.Diameter=Diameter;
	CanopyChx.FoliarMC=FoliarMoisture;
	CanopyChx.Tolerance=Tolerance;
	CanopyChx.Species=Species;
}

void FlamMap::SetFoliarMoistureContent(int Percent)
{
     if(Percent<1)
     	Percent=100;
     if(Percent>300)
     	Percent=300;
	CanopyChx.FoliarMC=Percent;
}

void FlamMap::GetDefaultCrownChx(double *Height, double *Base, double *Density)
{
	*Height=CanopyChx.DefaultHeight;
	*Base=CanopyChx.DefaultBase;
	*Density=CanopyChx.DefaultDensity;
}


double FlamMap::GetDefaultCrownHeight()
{
	return CanopyChx.Height;
}


double FlamMap::GetDefaultCrownBase()
{
	return CanopyChx.CrownBase;
}


double FlamMap::GetDefaultCrownBD(short cover)
{
	if(CrownDensityLinkToCC)
		return CanopyChx.BulkDensity*((double) cover)/100.0;

	return CanopyChx.BulkDensity;
}


double FlamMap::GetAverageDBH()
{
	return CanopyChx.Diameter;
}


double FlamMap::GetFoliarMC()
{
	return CanopyChx.FoliarMC;
}


int FlamMap::GetTolerance()
{
	return CanopyChx.Tolerance;
}


int FlamMap::GetCanopySpecies()
{
	return CanopyChx.Species;
}

bool FlamMap::LinkDensityWithCrownCover(int TrueFalse)
{
 	if(TrueFalse>=0)
     	CrownDensityLinkToCC=TrueFalse;

     return CrownDensityLinkToCC;
}

celldata FlamMap::CellData(double east, double north, celldata &cell, crowndata &cfuel, grounddata &gfuel, int64_t *posit)
{
     int64_t Position;
	// printf("In CellData1 landscape = %i\n", landscape);
     if(landscape==0)
     {    
		 printf("In CellData2\n");
		 if(CantAllocLCP==false)
     	{	int i;

     		fseek(landfile, headsize, SEEK_SET);
			int64_t rowSize = (int64_t)Header.numeast * (int64_t)NumVals;
			int64_t allOfIt = (int64_t)Header.numnorth * rowSize;
			if((landscape=new short[allOfIt])!=NULL)
          	{    
				//ALM ZeroMemory(landscape, allOfIt*sizeof(short));
				memset(landscape, 0x0, allOfIt*sizeof(short));
          		for(int64_t i64=0; i64<Header.numnorth; i64++)
          			fread(&landscape[i64*rowSize], sizeof(short),
                    		rowSize, landfile);
          		fseek(landfile, headsize, SEEK_SET);
//     	     	OldFilePosition=0;     // thread local
               	CantAllocLCP=false;
          	}
          	else
          		CantAllocLCP=true;
          }
     }

     Position=GetCellPosition(east, north);
	if(!CantAllocLCP)
     {    GetCellDataFromMemory(Position, cell, cfuel, gfuel);
		if(posit!=NULL)
			*posit = Position;
          return cell;
     }
	//file based! Use the Critical Section to allow threads correct access
	//EnterCriticalSection(&lcpCS);
	if(Header.CrownFuels==20)
	{    if(Header.GroundFuels==20)
     	{   	fseek(landfile, (Position-OldFilePosition)*sizeof(celldata), SEEK_CUR);
			fread(&cell, sizeof(celldata), 1, landfile);
          }
          else
     	{   	fseek(landfile, (Position-OldFilePosition)*(sizeof(celldata)+sizeof(grounddata)), SEEK_CUR);
			fread(&cell, sizeof(celldata), 1, landfile);
               fread(&gfuel, sizeof(grounddata), 1, landfile);
          }
	}
	else
	{    if(Header.GroundFuels==20)		// none
	     {	fseek(landfile, (Position-OldFilePosition)*(sizeof(celldata)+sizeof(crowndata)), SEEK_CUR);
			fread(&cell, sizeof(celldata), 1, landfile);
			fread(&cfuel, sizeof(crowndata), 1, landfile);
     	}
     	else
	     {	fseek(landfile, (Position-OldFilePosition)*(sizeof(celldata)+sizeof(crowndata)+
               	sizeof(grounddata)), SEEK_CUR);
			fread(&cell, sizeof(celldata), 1, landfile);
			fread(&cfuel, sizeof(crowndata), 1, landfile);
               fread(&gfuel, sizeof(grounddata), 1, landfile);
     	}
		if(cfuel.h>0)
		{	CanopyChx.Height=(double) cfuel.h/10.0;
			if(Header.HUnits==2)
				CanopyChx.Height/=3.280839;
		}
		else
			CanopyChx.Height=CanopyChx.DefaultHeight;
		if(cfuel.b>0)
		{	CanopyChx.CrownBase=(double) cfuel.b/10.0;
			if(Header.BUnits==2)
				CanopyChx.CrownBase/=3.280839;
		}
		else
			CanopyChx.CrownBase=CanopyChx.DefaultBase;
		if(cfuel.p>0)
		{    if(Header.PUnits==1)
				CanopyChx.BulkDensity=((double) cfuel.p)/100.0;
			else	if(Header.PUnits==2)
				CanopyChx.BulkDensity=((double) cfuel.p/1000.0)*16.01845;
		}
		else
			CanopyChx.BulkDensity=CanopyChx.DefaultDensity;
	}

	OldFilePosition=Position+1;
     if(posit!=NULL)
     	*posit=Position;
	//LeaveCriticalSection(&lcpCS);
	return cell;
}

/*int FlamMap::GetTheme_DistanceUnits()
{
	return Header.GridUnits;
}*/

char *FlamMap::GetTheme_UnitsString(short DataTheme)
{
	if(DataTheme < 0 || DataTheme > 9)
		return "";
	switch(DataTheme)
     {	
		case 0: 
			if(Header.EUnits >= 0 && Header.EUnits <= 1)
				return unitStrings[elevUnits[Header.EUnits]];
			break;
		case 1: //Header.SUnits;
			if(Header.SUnits >= 0 && Header.SUnits <= 1)
				return unitStrings[slopeUnits[Header.SUnits]];
			break;
		case 2: //Header.AUnits;
			if(Header.AUnits >= 0 && Header.AUnits <= 2)
				return unitStrings[aspectUnits[Header.AUnits]];
			break;
		case 3: //Header.FOptions;
			return unitStrings[fuelUnits[0]];
			break;
		case 4:// Header.CUnits;
			if(Header.CUnits >= 0 && Header.CUnits <= 1)
				return unitStrings[coverUnits[Header.CUnits]];
			break;

		case 5:        // Header.HUnits;
		 	if ( Header.HUnits >= 1 && Header.HUnits <= 4)	{
			   	if ( Header.HUnits == 1 || Header.HUnits == 3)
				    	return unitStrings[heightUnits[2]];
				   return unitStrings[heightUnits[3]];
			}
			break;

		case 6: //Header.BUnits;
			if(Header.BUnits >= 1 && Header.BUnits <= 4)
			{
				if(Header.BUnits == 1 || Header.BUnits == 3)
					return unitStrings[baseUnits[2]];
				return unitStrings[baseUnits[3]];
			}
			break;
		case 7: //Header.PUnits;
			if(Header.PUnits >= 1 && Header.PUnits <= 4)
			{
				if(Header.PUnits == 1 || Header.PUnits == 3)
					return unitStrings[bulkUnits[2]];
				return unitStrings[bulkUnits[3]];
			}
			break;
		case 8:// Header.DUnits;
			if(Header.DUnits >= 1 && Header.DUnits <= 2)
				return unitStrings[duffUnits[Header.DUnits - 1]];
			break;
		case 9: //Header.WOptions;
				return unitStrings[woodyUnits[0]];
			break;
     }
	return "Unknown";
}

/*short FlamMap::GetTheme_Units(short DataTheme)
{
	short units;

	switch(DataTheme)
     {	case 0: units=Header.EUnits; break;
		case 1: units=Header.SUnits; break;
		case 2: units=Header.AUnits; break;
		case 3: units=Header.FOptions; break;
		case 4: units=Header.CUnits; break;
		case 5: units=Header.HUnits; break;
		case 6: units=Header.BUnits; break;
		case 7: units=Header.PUnits; break;
		case 8: units=Header.DUnits; break;
		case 9: units=Header.WOptions; break;
     }

     return units;
}*/

short FlamMap::GetTheme_NumCategories(short DataTheme)
{
	int cats;

     switch(DataTheme)
     {	case 0: cats=Header.numelev; break;
     	case 1: cats=Header.numslope; break;
     	case 2: cats=Header.numaspect; break;
     	case 3: cats=Header.numfuel; break;
     	case 4: cats=Header.numcover; break;
     	case 5: cats=Header.numheight; break;
     	case 6: cats=Header.numbase; break;
     	case 7: cats=Header.numdensity; break;
     	case 8: cats=Header.numduff; break;
     	case 9: cats=Header.numwoody; break;
     }

     return (short) cats;
}

int FlamMap::GetTheme_HiValue(short DataTheme)
{
	int hi;

     switch(DataTheme)
     {	case 0: hi=Header.hielev; break;
     	case 1: hi=Header.hislope; break;
     	case 2: hi=Header.hiaspect; break;
     	case 3: hi=Header.hifuel; break;
     	case 4: hi=Header.hicover; break;
     	case 5: hi=Header.hiheight; break;
     	case 6: hi=Header.hibase; break;
     	case 7: hi=Header.hidensity; break;
     	case 8: hi=Header.hiduff; break;
     	case 9: hi=Header.hiwoody; break;
     }

     return hi;
}

int FlamMap::GetTheme_LoValue(short DataTheme)
{
	int lo;

     switch(DataTheme)
     {	case 0: lo=Header.loelev; break;
     	case 1: lo=Header.loslope; break;
     	case 2: lo=Header.loaspect; break;
     	case 3: lo=Header.lofuel; break;
     	case 4: lo=Header.locover; break;
     	case 5: lo=Header.loheight; break;
     	case 6: lo=Header.lobase; break;
     	case 7: lo=Header.lodensity; break;
     	case 8: lo=Header.loduff; break;
     	case 9: lo=Header.lowoody; break;
     }

     return lo;
}

char *FlamMap::GetTheme_FileName(short DataTheme)
{
     //char name[256]="";

	switch(DataTheme)
     {	case 0:	return Header.ElevFile; break;
     	case 1:	return Header.SlopeFile; break;
     	case 2:	return Header.AspectFile; break;
     	case 3:	return Header.FuelFile; break;
     	case 4:	return Header.CoverFile; break;
     	case 5:	return Header.HeightFile; break;
     	case 6:	return Header.BaseFile; break;
     	case 7:	return Header.DensityFile; break;
     	case 8:	return Header.DuffFile; break;
     	case 9:	return Header.WoodyFile; break;
     }

     return NULL;
}

/*int FlamMap::HaveCrownFuels()
{
	return Header.CrownFuels-20;		// subtract 10 to ID file as version 2.x
}

int FlamMap::HaveGroundFuels()
{
	return Header.GroundFuels-20;
}*/

/*double FlamMap::GetCellResolutionX()
{
//	CellResolution=(Header.NorthUtm-Header.SouthUtm)/Header.numnorth;
//	CellResolution=(Header.EastUtm-Header.WestUtm)/Header.numeast;

	return Header.XResol;
}*/


/*double FlamMap::GetCellResolutionY()
{
//	CellResolution=(Header.NorthUtm-Header.SouthUtm)/Header.numnorth;
//	CellResolution=(Header.EastUtm-Header.WestUtm)/Header.numeast;

	return Header.YResol;
}*/


double FlamMap::MetricResolutionConvert()
{
	if(Header.GridUnits==1)
		return 3.280839895;     // metric conversion to meters
	else
		return 1.0;
}


int FlamMap::CheckCellResUnits()
{
	return Header.GridUnits;
}

void FlamMap::ResetFuelConversions()
{
	for(int i=0; i<257; i++)             		// could also read default file here
		fuelconversion.Type[i]=i;
     fuelconversion.Type[0]=-1;
	fuelconversion.Type[99]=-1;
	fuelconversion.Type[98]=-2;
	fuelconversion.Type[97]=-3;
	fuelconversion.Type[96]=-4;
	fuelconversion.Type[95]=-5;
	fuelconversion.Type[94]=-6;
	fuelconversion.Type[93]=-7;
	fuelconversion.Type[92]=-8;
	fuelconversion.Type[91]=-9;
     HAVE_CONV_MODELS=false;
}

int FlamMap::GetFuelConversion(int fuel)
{// retrieve fuel model conversions
	int cnv=-1;

	if(fuel>=0 && fuel<257)    // check fuel for valid array range
	{	cnv=fuelconversion.Type[fuel];   // get new fuel
          if(cnv<1)
          {    if(cnv<-9)
                    cnv=-1;
          }
          else if(!IsNewFuelReserved(cnv))
          {    if(newfuels[cnv].number==0)
                    cnv=-1;
          }
          else if(cnv>256)
               cnv=-1;
     }

     return cnv;
}

int FlamMap::SetFuelConversion(int From, int To)
{// set fuel model conversions
	if(From>=0 && From<257 && To<257 && To>=0)
	{    if(To>90 && To<100)
     		To=To-100;		// make all negative for the 90's, indicate unburnable
     	else if(To==0)
          	To=-1;
          fuelconversion.Type[From]=To;
	}
	else
		return false;

	return true;
}


void FlamMap::ReadHeader2()
{
	fseek(landfile, 0, SEEK_SET);
	fread(&Header2, sizeof(headdata2), 1, landfile);
//	celldata cell;
//	fseek(landfile, sizeof(cell), SEEK_CUR);
//	OldFilePosition=ftell(landfile);           // position of file at end of first record
//	OldFilePosition=0;

	if(Header2.FOptions==1 || Header2.FOptions==3)
		NEED_CUST_MODELS=true;
	else
		NEED_CUST_MODELS=false;
	if(Header2.FOptions==2 || Header2.FOptions==3)
		NEED_CONV_MODELS=true;
	else
		NEED_CONV_MODELS=false;
	HAVE_CUST_MODELS=false;
	HAVE_CONV_MODELS=false;
	// set raster resolution
//	RasterCellResolutionX=(Header2.EastUtm-Header2.WestUtm)/(double) Header2.numeast;
//	RasterCellResolutionY=(Header2.NorthUtm-Header2.SouthUtm)/(double) Header2.numnorth;
	//ViewPortNorth=RasterCellResolutionY*(double) Header2.numnorth+Header2.lonorth;
	//ViewPortSouth=Header2.lonorth;
	//ViewPortEast=RasterCellResolutionX*(double) Header2.numeast+Header2.loeast;
	//ViewPortWest=Header2.loeast;
//	NumViewNorth=(ViewPortNorth-ViewPortSouth)/Header.YResol;
//	NumViewEast=(ViewPortEast-ViewPortWest)/Header.XResol;
	/*double rows, cols;
	rows=(ViewPortNorth-ViewPortSouth)/Header2.YResol;
	NumViewNorth=rows;
	if(modf(rows, &rows)>0.5)
		NumViewNorth++;
	cols=(ViewPortEast-ViewPortWest)/Header2.XResol;
	NumViewEast=cols;
	if(modf(cols, &cols)>0.5)
		NumViewEast++;*/

     if(HaveCrownFuels())
     	NumVals=8;
     else
     	NumVals=5;
     CantAllocLCP=false;


}

void FlamMap::SetCustFuelModelID(bool True_False)
{
	HAVE_CUST_MODELS=True_False;
}


void FlamMap::SetConvFuelModelID(bool True_False)
{
	HAVE_CONV_MODELS=True_False;
}


bool FlamMap::NeedCustFuelModels()
{
	return NEED_CUST_MODELS;
}


bool FlamMap::NeedConvFuelModels()
{
	return NEED_CONV_MODELS;
}


bool FlamMap::HaveCustomFuelModels()
{
	return HAVE_CUST_MODELS;
}

bool FlamMap::HaveFuelConversions()
{
	return HAVE_CONV_MODELS;
}

size_t FlamMap::GetHeadSize()
{
	return headsize;
}

int FlamMap::LoadLandscapeFromGDAL(GDALDataset *poSrcDS)
{
    int nXSize, nYSize;
    nXSize = poSrcDS->GetRasterXSize();
    nYSize = poSrcDS->GetRasterYSize();
	bool bHasCrownFuels = false, bHasGroundFuels = false;
	int64_t nVals = (int64_t)nXSize * (int64_t)nYSize, loc;
    if( NumVals == 8 || NumVals == 10 )
        bHasCrownFuels = true;
    if( NumVals == 7 || NumVals == 10 )
        bHasGroundFuels = true;
	//set optional band data to 0's
	Header.numheight = 0;
	Header.numbase = 0;
	Header.numdensity = 0;
	Header.numduff = 0;
	Header.numwoody = 0;
	for(int i = 0; i < 100; i++)
	{
		Header.heights[i] = Header.bases[i] = Header.densities[i] = Header.duffs[i] = Header.woodies[i] = 0;
	}
	try
	{
		landscape = new short[nVals * (int64_t)NumVals];
	}
	catch (...)//assume meory allocation failure!
	{
		CantAllocLCP = true;
		landscape = 0;
		//GDALClose( poSrcDS );
		return -1;
	}
	if (landscape != NULL)
	{
		GDALRasterBand *lcpBands[10];
		short *panScanlines[10];
		for(int b = 0; b < 10; b++)
		{
			lcpBands[b] = NULL;
			panScanlines[b] = NULL;
		}
		for(int b = 0; b < NumVals; b++)
		{
			lcpBands[b] = poSrcDS->GetRasterBand( b + 1);
			panScanlines[b] = (short*) CPLMalloc( sizeof( short ) * nXSize );
		}

		//ZeroMemory(landscape,Header.numnorth * Header.numeast * NumVals * sizeof(short));
		memset(landscape,0x0, nVals * (int64_t)NumVals * sizeof(short));
		for( int i = 0; i < nYSize; i++ ) 
		{
			for(int b = 0; b < NumVals; b++)
			{
				lcpBands[b]->RasterIO( GF_Read, 0, i, nXSize, 1, panScanlines[b], nXSize,
					1, GDT_Int16, 0, 0 );
				for(int x = 0; x < nXSize; x++)
				{
					loc = ((int64_t)i) * ((int64_t)Header.numeast) * ((int64_t)NumVals) + ((int64_t)x) * ((int64_t)NumVals) + ((int64_t)b);
					landscape[loc] = panScanlines[b][x];
					//landscape[i * Header.numeast * NumVals + x * NumVals + b] = panScanlines[b][x];
				}
			}
		}
		for(int b = 0; b < 10; b++)
		{
			if(panScanlines[b] != NULL)
				CPLFree(panScanlines[b]);
		}
		CantAllocLCP = false;
		//now build Header legending data
		short uniques[10][100];
		for(int b = 0; b < 10; b++)
		{
			uniques[b][0] = NODATA_VAL;
			for(int i = 1; i < 100; i++)
				uniques[b][i] = -1;
		}
		short *pData = new short[nVals], vMin = 0, vMax = 0;
		for(int b = 0; b < NumVals; b++)
		{
			memset(pData,0x0,nVals * sizeof(short));
			for(int64_t v = 0; v < nVals; v++)
			{
				loc = (int64_t) NumVals * v + (int64_t)b; 
				pData[v] = landscape[loc];
			}
			qsort(pData, nVals, sizeof(short), shortCompareAscending);
			//got sorted values, need min, max, and count of uniques (as int as < 100 uniques)
			loc = 1;
			for(int64_t v = 0; v < nVals; v++)
			{
				if(pData[v] >= 0)
				{
					if(loc == 1)
						vMin = pData[v];
					if(pData[v] != uniques[b][loc - 1])
					{
						uniques[b][loc] = pData[v];
						loc++;
						if(loc >= 100)
						{
							loc = -1;
							break;
						}
					}
				}
			}
			if(b == 0)//elev
			{
				Header.loelev = vMin;
				Header.hielev = pData[nVals - 1];
				Header.numelev = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.elevs[i] = uniques[b][i];
			}
			if(b == 1)//slope
			{
				Header.loslope = vMin;
				Header.hislope = pData[nVals - 1];
				Header.numslope = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.slopes[i] = uniques[b][i];
			}
			if(b == 2)//aspect
			{
				Header.loaspect = vMin;
				Header.hiaspect = pData[nVals - 1];
				Header.numaspect = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.aspects[i] = uniques[b][i];
			}
			if(b == 3)//fuel
			{
				Header.lofuel = vMin;
				Header.hifuel = pData[nVals - 1];
				Header.numfuel = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.fuels[i] = uniques[b][i];
			}
			if(b == 4)//canopy cover
			{
				Header.locover = vMin;
				Header.hicover = pData[nVals - 1];
				Header.numcover = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.covers[i] = uniques[b][i];
			}
			if(b == 5 && bHasCrownFuels)//tree height
			{
				Header.loheight = vMin;
				Header.hiheight = pData[nVals - 1];
				Header.numheight = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.heights[i] = uniques[b][i];
			}
			if(b == 6 && bHasCrownFuels)//canopy base height
			{
				Header.lobase = vMin;
				Header.hibase = pData[nVals - 1];
				Header.numbase = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.bases[i] = uniques[b][i];
			}
			if(b == 7 && bHasCrownFuels)//crown bulk density
			{
				Header.lodensity = vMin;
				Header.hidensity = pData[nVals - 1];
				Header.numdensity = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.densities[i] = uniques[b][i];
			}
			if(b == 8 && bHasCrownFuels && bHasGroundFuels || b == 5 && (!bHasCrownFuels && bHasGroundFuels))//crown bulk density
			{
				Header.lowoody = vMin;
				Header.hiwoody = pData[nVals - 1];
				Header.numwoody = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.woodies[i] = uniques[b][i];
			}
			if(b == 9 && bHasCrownFuels && bHasGroundFuels || b == 6 && (!bHasCrownFuels && bHasGroundFuels))//crown bulk density
			{
				Header.loduff = vMin;
				Header.hiduff = pData[nVals - 1];
				Header.numduff = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.duffs[i] = uniques[b][i];
			}
		}
		delete[] pData;
	}
	return 1;
}

int FlamMap::ReadGdalLCP(char *lcpName)
{
    GDALDriver *poTiffDriver;
    GDALDataset *poSrcDS;
    int nXSize, nYSize;
    int nBands;
    double adfGeoTransform[6];
	bool bHasCrownFuels = false, bHasGroundFuels = false;
    //check for good ds
    GDALAllRegister();
    poSrcDS = (GDALDataset*)GDALOpen( lcpName, GA_ReadOnly );
    if( poSrcDS == NULL ) 
	{
        return -1;
    }
	NumVals = nBands = poSrcDS->GetRasterCount();
	Header.latitude = atol(poSrcDS->GetMetadataItem( "LATITUDE" ));
	switch(nBands)
	{
	case 5:
		Header.GroundFuels = Header.CrownFuels = 20;
		break;
	case 7:
		Header.GroundFuels = 21;
		Header.CrownFuels = 20;
		break;
	case 8:
		Header.GroundFuels = 20;
		Header.CrownFuels = 21;
		break;
	case 10:
		Header.GroundFuels = Header.CrownFuels = 21;
		break;
	default:
		//invlaid!
		GDALClose( poSrcDS );
		return -1;
	}
    //get general info

    nXSize = poSrcDS->GetRasterXSize();
    nYSize = poSrcDS->GetRasterYSize();
	int64_t nVals = (int64_t)nXSize * (int64_t)nYSize, loc;
	Header.numeast = nXSize;
	Header.numnorth = nYSize;
    if( poSrcDS->GetGeoTransform( adfGeoTransform ) == CE_None ) 
	{
		Header.XResol = fabs(adfGeoTransform[1]);
		Header.YResol = fabs(adfGeoTransform[5]);
		Header.loeast = Header.WestUtm = adfGeoTransform[0];
		Header.hieast = Header.EastUtm = Header.loeast + nXSize * Header.XResol;
		Header.hinorth = Header.NorthUtm = adfGeoTransform[3];
		Header.lonorth = Header.SouthUtm = Header.hinorth - nYSize * Header.YResol;
	}
	else
	{
		GDALClose( poSrcDS );
		return -1;
	}
	//read the metadata
	char metaData[64];
	strcpy(metaData, poSrcDS->GetMetadataItem("LINEAR_UNIT"));
	if(strcmpi(metaData, "Feet") == 0)
		Header.GridUnits = 1;
	else
		Header.GridUnits = 0;

	GDALRasterBand *lcpBands[10];
		//short *panScanlines[10];
	for(int b = 0; b < 10; b++)
	{
		lcpBands[b] = NULL;
			//panScanlines[b] = NULL;
	}
	//elevation
	lcpBands[0] = poSrcDS->GetRasterBand( 1);
	if(lcpBands[0] != NULL)
	{
		strcpy(metaData, lcpBands[0]->GetMetadataItem("ELEVATION_UNIT"));
		Header.EUnits = atoi(metaData);
		strcpy(metaData, lcpBands[0]->GetMetadataItem("ELEVATION_MIN"));
		Header.loelev = atol(metaData);
		strcpy(metaData, lcpBands[0]->GetMetadataItem("ELEVATION_MAX"));
		Header.hielev = atol(metaData);
	}
	//slope
	lcpBands[1] = poSrcDS->GetRasterBand( 2);
	if(lcpBands[1] != NULL)
	{
		strcpy(metaData, lcpBands[1]->GetMetadataItem("SLOPE_UNIT"));
		Header.SUnits = atoi(metaData);
		strcpy(metaData, lcpBands[1]->GetMetadataItem("SLOPE_MIN"));
		Header.loslope = atol(metaData);
		strcpy(metaData, lcpBands[1]->GetMetadataItem("SLOPE_MAX"));
		Header.hislope = atol(metaData);
	}
	//aspect
	lcpBands[2] = poSrcDS->GetRasterBand( 3);
	if(lcpBands[2] != NULL)
	{
		strcpy(metaData, lcpBands[2]->GetMetadataItem("ASPECT_UNIT"));
		Header.AUnits = atoi(metaData);
		strcpy(metaData, lcpBands[2]->GetMetadataItem("ASPECT_MIN"));
		Header.loaspect = atol(metaData);
		strcpy(metaData, lcpBands[2]->GetMetadataItem("ASPECT_MAX"));
		Header.hiaspect = atol(metaData);
	}
	//fuels
	lcpBands[3] = poSrcDS->GetRasterBand( 4);
	if(lcpBands[3] != NULL)
	{
		strcpy(metaData, lcpBands[3]->GetMetadataItem("FUEL_MODEL_OPTION"));
		Header.FOptions = atoi(metaData);
		strcpy(metaData, lcpBands[3]->GetMetadataItem("FUEL_MODEL_MIN"));
		Header.lofuel = atol(metaData);
		strcpy(metaData, lcpBands[3]->GetMetadataItem("FUEL_MODEL_MAX"));
		Header.hifuel = atol(metaData);
	}
	//canopy cover
	lcpBands[4] = poSrcDS->GetRasterBand( 5);
	if(lcpBands[4] != NULL)
	{
		strcpy(metaData, lcpBands[4]->GetMetadataItem("CANOPY_COV_UNIT"));
		Header.CUnits = atoi(metaData);
		strcpy(metaData, lcpBands[4]->GetMetadataItem("CANOPY_COV_MIN"));
		Header.locover = atol(metaData);
		strcpy(metaData, lcpBands[4]->GetMetadataItem("CANOPY_COV_MAX"));
		Header.hicover = atol(metaData);
	}
	if(Header.CrownFuels > 20)
	{
		//canopy height
		lcpBands[5] = poSrcDS->GetRasterBand( 6);
		if(lcpBands[5] != NULL)
		{
			strcpy(metaData, lcpBands[5]->GetMetadataItem("CANOPY_HT_UNIT"));
			Header.HUnits = atoi(metaData);
			strcpy(metaData, lcpBands[5]->GetMetadataItem("CANOPY_HT_MIN"));
			Header.loheight = atol(metaData);
			strcpy(metaData, lcpBands[5]->GetMetadataItem("CANOPY_HT_MAX"));
			Header.hiheight = atol(metaData);
		}
		//canopy base height
		lcpBands[6] = poSrcDS->GetRasterBand( 7);
		if(lcpBands[6] != NULL)
		{
			strcpy(metaData, lcpBands[6]->GetMetadataItem("CBH_UNIT"));
			Header.BUnits = atoi(metaData);
			strcpy(metaData, lcpBands[6]->GetMetadataItem("CBH_MIN"));
			Header.lobase = atol(metaData);
			strcpy(metaData, lcpBands[6]->GetMetadataItem("CBH_MAX"));
			Header.hibase = atol(metaData);
		}
		//canopy bulk density
		lcpBands[7] = poSrcDS->GetRasterBand( 8);
		if(lcpBands[7] != NULL)
		{
			strcpy(metaData, lcpBands[7]->GetMetadataItem("CBD_UNIT"));
			Header.PUnits = atoi(metaData);
			strcpy(metaData, lcpBands[7]->GetMetadataItem("CBD_MIN"));
			Header.lodensity = atol(metaData);
			strcpy(metaData, lcpBands[7]->GetMetadataItem("CBD_MAX"));
			Header.hidensity = atol(metaData);
		}
	}
	if(Header.GroundFuels > 20)
	{
		int loc = 8;
		if(Header.CrownFuels == 20)
			loc = 5;
		//duff
		lcpBands[loc] = poSrcDS->GetRasterBand( loc + 1);
		if(lcpBands[loc] != NULL)
		{
			strcpy(metaData, lcpBands[loc]->GetMetadataItem("DUFF_UNIT"));
			Header.DUnits = atoi(metaData);
			strcpy(metaData, lcpBands[loc]->GetMetadataItem("DUFF_MIN"));
			Header.loduff = atol(metaData);
			strcpy(metaData, lcpBands[loc]->GetMetadataItem("DUFF_MAX"));
			Header.hiduff = atol(metaData);
		}
		//coarse woody
		loc++;
		lcpBands[loc] = poSrcDS->GetRasterBand( loc + 1);
		if(lcpBands[loc] != NULL)
		{
			strcpy(metaData, lcpBands[loc]->GetMetadataItem("CWD_OPTION"));
			Header.WOptions = atoi(metaData);
			strcpy(metaData, lcpBands[loc]->GetMetadataItem("CWD_MIN"));
			Header.lowoody = atol(metaData);
			strcpy(metaData, lcpBands[loc]->GetMetadataItem("CWD_MAX"));
			Header.hiwoody = atol(metaData);
		}

	}
	int lcpLoaded = LoadLandscapeFromGDAL(poSrcDS);
	/*for(int b = 0; b < NumVals; b++)
	{
		lcpBands[b] = poSrcDS->GetRasterBand( b + 1);
			//panScanlines[b] = (short*) CPLMalloc( sizeof( short ) * nXSize );
		if(lcpBands[b] != NULL)
		{
			strcpy(metaData, lcpBands[b]->GetMetadataItem("ELEVATION_UNIT"));
		}
	}*/
	//read the metadata for each band

	analysisLoEast = Header.loeast;
	//printf("analysisLoEast2 = %f\n", analysisLoEast);
	analysisHiEast = Header.hieast;
	//printf("analysisHiEast2 = %f\n", analysisHiEast);
	analysisLoNorth = Header.lonorth;
	analysisHiNorth = Header.hinorth;
	analysisRect.left = 0;
	analysisRect.right = Header.numeast - 1;
	analysisRect.top = 0;
	analysisRect.bottom = Header.numnorth - 1;
	GDALClose( poSrcDS );
	return lcpLoaded;
}

bool BandHasData(GDALDataset *pData, int bandNum)
{
    int nXSize = pData->GetRasterXSize();
    int nYSize = pData->GetRasterYSize();
	int64_t nVals = (int64_t)nXSize * (int64_t)nYSize;
	GDALRasterBand *pBand = pData->GetRasterBand(bandNum);
	if(pBand == NULL)
		return false;
	int hasData = false;
	short *panScanlines = (short*) CPLMalloc( sizeof( short ) * nXSize );
	for( int i = 0; i < nYSize; i++ ) 
	{
		pBand->RasterIO( GF_Read, 0, i, nXSize, 1, panScanlines, nXSize,
					1, GDT_Int16, 0, 0 );
		for(int x = 0; x < nXSize; x++)
		{
			if(panScanlines[x] > 0)
			{
				hasData = true;
				break;
			}
		}
		if(hasData)
			break;
	}

	CPLFree(panScanlines);
	return hasData;

}
int FlamMap::ReadGeoTiff(char *geoTiffName)
{
    GDALDriver *poTiffDriver;
    GDALDataset *poSrcDS;
    int nXSize, nYSize;
   // int nNoDataValue = -9999;
    int nBands;
    double adfGeoTransform[6];
	bool bHasCrownFuels = false, bHasGroundFuels = false;
    //check for good ds
    GDALAllRegister();
    poSrcDS = (GDALDataset*)GDALOpen( geoTiffName, GA_ReadOnly );
    if( poSrcDS == NULL ) 
	{
        return -1;
    }

    //get general info

    nXSize = poSrcDS->GetRasterXSize();
    nYSize = poSrcDS->GetRasterYSize();
	int64_t nVals = (int64_t)nXSize * (int64_t)nYSize, loc;

    if( poSrcDS->GetGeoTransform( adfGeoTransform ) != CE_None ) 
	{
		GDALClose( poSrcDS );
        return -1;
    }

    NumVals = nBands = poSrcDS->GetRasterCount();
	if(nBands != 10)
		return -1;
	bool hasHeights = BandHasData(poSrcDS, 6), hasBases = BandHasData(poSrcDS, 7), hasBulks = BandHasData(poSrcDS, 8), 
		hasDuffs = BandHasData(poSrcDS, 9), hasWoodies = BandHasData(poSrcDS, 10);
	if(hasHeights && hasBases && hasBulks)
        bHasCrownFuels = true;
    if( hasDuffs && hasWoodies)
        bHasGroundFuels = true;
	if(bHasCrownFuels && bHasGroundFuels)
		NumVals = 10;
	else if(bHasGroundFuels)
		NumVals = 7;
	else if(bHasCrownFuels)
		NumVals = 8;
	else
		NumVals = 5;
	Header.CrownFuels = (bHasCrownFuels == true) ? 21 : 20;
	Header.GroundFuels = (bHasGroundFuels == true) ? 21 : 20;
	Header.numeast = nXSize;
	Header.numnorth = nYSize;
	Header.XResol = fabs(adfGeoTransform[1]);
	Header.YResol = fabs(adfGeoTransform[5]);
	Header.loeast = Header.WestUtm = adfGeoTransform[0];
	Header.hieast = Header.EastUtm = Header.loeast + nXSize * Header.XResol;
	Header.hinorth = Header.NorthUtm = adfGeoTransform[3];
	Header.lonorth = Header.SouthUtm = Header.hinorth - nYSize * Header.YResol;
	Header.GridUnits = 0;
	Header.EUnits = 0;
	Header.SUnits = 0;
	Header.AUnits = 2;
	Header.FOptions = 0;
	Header.CUnits = 1;
	Header.HUnits = 3;
	Header.BUnits = 3;
	Header.PUnits = 3;
	Header.DUnits = 1;
	Header.WOptions = 0;
	Header.latitude = 45;
	NEED_CUST_MODELS = NEED_CONV_MODELS = false;
	HAVE_CUST_MODELS=false;
	HAVE_CONV_MODELS=false;
	
	//safety
	Header.numheight = Header.numbase = Header.numdensity = Header.numduff = Header.numwoody = 0;
	for(int i = 0; i < 100; i++)
	{
		Header.heights[i] = Header.bases[i] = Header.densities[i] = Header.woodies[i] = Header.duffs[i] = 0;
	}
	Header.loheight = Header.lobase = Header.lodensity = Header.lowoody = Header.loduff =
		Header.hiheight = Header.hibase = Header.hidensity = Header.hiwoody = Header.hiduff = 0;
	//need to populatelandscape array AND populate header information for each layer
	//CloseLandFile();
	try
	{
		landscape = new short[nVals * (int64_t)NumVals];
	}
	catch (...)//assume memory allocation failure!
	{
		CantAllocLCP = true;
		landscape = 0;
		GDALClose( poSrcDS );
		return -1;
	}
	if (landscape != NULL)
	{
		GDALRasterBand **lcpBands = new GDALRasterBand*[NumVals];
		short **panScanlines = new short*[NumVals];
		short *bandLocs = new short[NumVals];
		bandLocs[0] = 1;
		bandLocs[1] = 2;
		bandLocs[2] = 3;
		bandLocs[3] = 4;
		bandLocs[4] = 5;
		if(NumVals == 7)
		{
			bandLocs[5] = 9;
			bandLocs[6] = 10;
		}
		else if(NumVals == 8)
		{
			bandLocs[5] = 6;
			bandLocs[6] = 7;
			bandLocs[7] = 8;
		}
		else if(NumVals == 10)
		{
			bandLocs[5] = 6;
			bandLocs[6] = 7;
			bandLocs[7] = 8;
			bandLocs[8] = 9;
			bandLocs[9] = 10;
		}
		for(int b = 0; b < NumVals; b++)
		{
			lcpBands[b] = NULL;
			panScanlines[b] = NULL;
		}
		for(int b = 0; b < NumVals; b++)
		{
			lcpBands[b] = poSrcDS->GetRasterBand( bandLocs[b]);
			panScanlines[b] = (short*) CPLMalloc( sizeof( short ) * nXSize );
		}

		//ZeroMemory(landscape,Header.numnorth * Header.numeast * NumVals * sizeof(short));
		memset(landscape,0x0, nVals * (int64_t)NumVals * sizeof(short));
		for( int i = 0; i < nYSize; i++ ) 
		{
			for(int b = 0; b < NumVals; b++)
			{
				lcpBands[b]->RasterIO( GF_Read, 0, i, nXSize, 1, panScanlines[b], nXSize,
					1, GDT_Int16, 0, 0 );
				for(int x = 0; x < nXSize; x++)
				{
					loc = ((int64_t)i) * ((int64_t)Header.numeast) * ((int64_t)NumVals) + ((int64_t)x) * ((int64_t)NumVals) + ((int64_t)b);
					landscape[loc] = panScanlines[b][x];
					//landscape[i * Header.numeast * NumVals + x * NumVals + b] = panScanlines[b][x];
				}
			}
		}
		for(int b = 0; b < NumVals; b++)
		{
			if(panScanlines[b] != NULL)
				CPLFree(panScanlines[b]);
		}
		delete[] panScanlines;
		delete[] bandLocs;
		double	dfGeoX, dfGeoY;
		dfGeoX = adfGeoTransform[0] + adfGeoTransform[1] * nXSize/2
				+ adfGeoTransform[2] * nYSize/2;
		dfGeoY = adfGeoTransform[3] + adfGeoTransform[4] * nXSize/2
				+ adfGeoTransform[5] * nYSize/2;
		const char *pszWkt = poSrcDS->GetProjectionRef();
		bool calcedLatLon = false;
		if( pszWkt != NULL )
		{
			char **tmp = CSLTokenizeString(pszWkt);
			OGRSpatialReferenceH hSrc = NULL, hDst = NULL;
			hSrc = OSRNewSpatialReference(pszWkt);
			if(hSrc != NULL)
			{
				hDst = OSRCloneGeogCS( hSrc );
				if(hDst)
				{
					OGRCoordinateTransformationH hTransform = OCTNewCoordinateTransformation( hSrc, hDst );
					if(hTransform != NULL)
					{
						OCTTransform(hTransform,1,&dfGeoX,&dfGeoY,NULL);
						Header.latitude = (int) dfGeoY;
						calcedLatLon = true;
						OCTDestroyCoordinateTransformation( hTransform );
					}
				}
			}
			if(hDst != NULL)
				OSRDestroySpatialReference( hDst );
			if(hSrc != NULL)
				OSRDestroySpatialReference( hSrc );
			/*OGRSpatialReference *osrSrc, *osrDst;// = NULL;
			osrSrc = new OGRSpatialReference();
			osrSrc->importFromESRI(tmp);
			osrDst = new OGRSpatialReference();//osrSrc->CloneGeogCS();
			osrDst->SetWellKnownGeogCS("WGS84");
			OGRCoordinateTransformation *pTransform = OGRCreateCoordinateTransformation(osrSrc, osrDst);
			pTransform->Transform(1, &dfGeoX, &dfGeoY, NULL);
			Header.latitude = (int) dfGeoY;
			delete pTransform;
			int sRef = osrSrc->GetReferenceCount();
			int dRef = osrDst->GetReferenceCount();*/

			//OGRSpatialReference::DestroySpatialReference(osrDst);
			//osrDst->Release();
			//osrSrc->DestroySpatialReference(osrDst);
			//osrSrc.Release();
			//OGRSpatialReference::DestroySpatialReference(osrSrc);
			//delete osrSrc;
			//osrSrc.DestroySpatialReference();
			CSLDestroy(tmp);

		}
		if(!calcedLatLon)//guess
		{
			Header.latitude = 35;

		}


		//OCTTransform(hTransform,1,&dfGeoX,&dfGeoY,NULL);
		CantAllocLCP = false;
		GDALClose( poSrcDS );
		delete[] lcpBands;
		//now need to populate Header with min, max, and legend values (up to 100, -1if < 100)
		short uniques[10][100];
		for(int b = 0; b < 10; b++)
		{
			uniques[b][0] = NODATA_VAL;
			for(int i = 1; i < 100; i++)
				uniques[b][i] = -1;
		}
		short *pData = new short[nVals], vMin = 0, vMax = 0;
		for(int b = 0; b < NumVals; b++)
		{
			memset(pData,0x0,nVals * sizeof(short));
			for(int64_t v = 0; v < nVals; v++)
			{
				loc = (int64_t) NumVals * v + (int64_t)b; 
				pData[v] = landscape[loc];
			}
			qsort(pData, nVals, sizeof(short), shortCompareAscending);
			//got sorted values, need min, max, and count of uniques (as int as < 100 uniques)
			loc = 1;
			for(int64_t v = 0; v < nVals; v++)
			{
				if(pData[v] >= 0)
				{
					if(loc == 1)
						vMin = pData[v];
					if(pData[v] != uniques[b][loc - 1])
					{
						uniques[b][loc] = pData[v];
						loc++;
						if(loc >= 100)
						{
							loc = -1;
							break;
						}
					}
				}
			}
			if(b == 0)//elev
			{
				Header.loelev = vMin;
				Header.hielev = pData[nVals - 1];
				Header.numelev = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.elevs[i] = uniques[b][i];
			}
			if(b == 1)//slope
			{
				Header.loslope = vMin;
				Header.hislope = pData[nVals - 1];
				Header.numslope = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.slopes[i] = uniques[b][i];
			}
			if(b == 2)//aspect
			{
				Header.loaspect = vMin;
				Header.hiaspect = pData[nVals - 1];
				Header.numaspect = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.aspects[i] = uniques[b][i];
			}
			if(b == 3)//fuel
			{
				Header.lofuel = vMin;
				Header.hifuel = pData[nVals - 1];
				Header.numfuel = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.fuels[i] = uniques[b][i];
			}
			if(b == 4)//canopy cover
			{
				Header.locover = vMin;
				Header.hicover = pData[nVals - 1];
				Header.numcover = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.covers[i] = uniques[b][i];
			}
			if(b == 5 && bHasCrownFuels)//tree height
			{
				Header.loheight = vMin;
				Header.hiheight = pData[nVals - 1];
				Header.numheight = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.heights[i] = uniques[b][i];
			}
			if(b == 6 && bHasCrownFuels)//canopy base height
			{
				Header.lobase = vMin;
				Header.hibase = pData[nVals - 1];
				Header.numbase = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.bases[i] = uniques[b][i];
			}
			if(b == 7 && bHasCrownFuels)//crown bulk density
			{
				Header.lodensity = vMin;
				Header.hidensity = pData[nVals - 1];
				Header.numdensity = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.densities[i] = uniques[b][i];
			}
			if(b == 8 && bHasCrownFuels && bHasGroundFuels || b == 5 && (!bHasCrownFuels && bHasGroundFuels))//crown bulk density
			{
				Header.lowoody = vMin;
				Header.hiwoody = pData[nVals - 1];
				Header.numwoody = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.woodies[i] = uniques[b][i];
			}
			if(b == 9 && bHasCrownFuels && bHasGroundFuels || b == 6 && (!bHasCrownFuels && bHasGroundFuels))//crown bulk density
			{
				Header.loduff = vMin;
				Header.hiduff = pData[nVals - 1];
				Header.numduff = (loc > 0) ? loc : -1;
				for(int i = 0; i < 100; i++)
					Header.duffs[i] = uniques[b][i];
			}
		}
		delete[] pData;
	}
	else
		CantAllocLCP = true;


	analysisLoEast = Header.loeast;
	analysisHiEast = Header.hieast;
	printf("analysisLoEast3 = %f", analysisLoEast);
	printf("analysisHiEast3 = %f", analysisHiEast);
	analysisLoNorth = Header.lonorth;
	analysisHiNorth = Header.hinorth;
	analysisRect.left = 0;
	analysisRect.right = Header.numeast - 1;
	analysisRect.top = 0;
	analysisRect.bottom = Header.numnorth - 1;
	return 1;
}

void FlamMap::ReadHeader()
{
	fseek(landfile, 0, SEEK_SET);

//     unsigned int sizeh=sizeof(Header);
//	fread(&Header, sizeh, 1, landfile);

	fread(&Header.CrownFuels, sizeof(int), 1, landfile);
	fread(&Header.GroundFuels, sizeof(int), 1, landfile);
	fread(&Header.latitude, sizeof(int), 1, landfile);
	fread(&Header.loeast, sizeof(double), 1, landfile);
	fread(&Header.hieast, sizeof(double), 1, landfile);
	fread(&Header.lonorth, sizeof(double), 1, landfile);
	fread(&Header.hinorth, sizeof(double), 1, landfile);
     fread(&Header.loelev, sizeof(int), 1, landfile);
     fread(&Header.hielev, sizeof(int), 1, landfile);
     fread(&Header.numelev, sizeof(int), 1, landfile);
     fread(Header.elevs, sizeof(int), 100, landfile);
     fread(&Header.loslope, sizeof(int), 1, landfile);
     fread(&Header.hislope, sizeof(int), 1, landfile);
     fread(&Header.numslope, sizeof(int), 1, landfile);
     fread(Header.slopes, sizeof(int), 100, landfile);
     fread(&Header.loaspect, sizeof(int), 1, landfile);
     fread(&Header.hiaspect, sizeof(int), 1, landfile);
     fread(&Header.numaspect, sizeof(int), 1, landfile);
     fread(Header.aspects, sizeof(int), 100, landfile);
     fread(&Header.lofuel, sizeof(int), 1, landfile);
     fread(&Header.hifuel, sizeof(int), 1, landfile);
     fread(&Header.numfuel, sizeof(int), 1, landfile);
     fread(Header.fuels, sizeof(int), 100, landfile);
     fread(&Header.locover, sizeof(int), 1, landfile);
     fread(&Header.hicover, sizeof(int), 1, landfile);
     fread(&Header.numcover, sizeof(int), 1, landfile);
     fread(Header.covers, sizeof(int), 100, landfile);
     fread(&Header.loheight, sizeof(int), 1, landfile);
     fread(&Header.hiheight, sizeof(int), 1, landfile);
     fread(&Header.numheight, sizeof(int), 1, landfile);
     fread(Header.heights, sizeof(int), 100, landfile);
     fread(&Header.lobase, sizeof(int), 1, landfile);
     fread(&Header.hibase, sizeof(int), 1, landfile);
     fread(&Header.numbase, sizeof(int), 1, landfile);
     fread(Header.bases, sizeof(int), 100, landfile);
     fread(&Header.lodensity, sizeof(int), 1, landfile);
     fread(&Header.hidensity, sizeof(int), 1, landfile);
     fread(&Header.numdensity, sizeof(int), 1, landfile);
     fread(Header.densities, sizeof(int), 100, landfile);
     fread(&Header.loduff, sizeof(int), 1, landfile);
     fread(&Header.hiduff, sizeof(int), 1, landfile);
     fread(&Header.numduff, sizeof(int), 1, landfile);
     fread(Header.duffs, sizeof(int), 100, landfile);
     fread(&Header.lowoody, sizeof(int), 1, landfile);
     fread(&Header.hiwoody, sizeof(int), 1, landfile);
     fread(&Header.numwoody, sizeof(int), 1, landfile);
     fread(Header.woodies, sizeof(int), 100, landfile);
     fread(&Header.numeast, sizeof(int), 1, landfile);
     fread(&Header.numnorth, sizeof(int), 1, landfile);
     fread(&Header.EastUtm, sizeof(double), 1, landfile);
     fread(&Header.WestUtm, sizeof(double), 1, landfile);
     fread(&Header.NorthUtm, sizeof(double), 1, landfile);
     fread(&Header.SouthUtm, sizeof(double), 1, landfile);
     fread(&Header.GridUnits, sizeof(int), 1, landfile);
     fread(&Header.XResol, sizeof(double), 1, landfile);
     fread(&Header.YResol, sizeof(double), 1, landfile);
     fread(&Header.EUnits, sizeof(short), 1, landfile);
     fread(&Header.SUnits, sizeof(short), 1, landfile);
     fread(&Header.AUnits, sizeof(short), 1, landfile);
     fread(&Header.FOptions, sizeof(short), 1, landfile);
     fread(&Header.CUnits, sizeof(short), 1, landfile);
     fread(&Header.HUnits, sizeof(short), 1, landfile);
     fread(&Header.BUnits, sizeof(short), 1, landfile);
     fread(&Header.PUnits, sizeof(short), 1, landfile);
     fread(&Header.DUnits, sizeof(short), 1, landfile);
     fread(&Header.WOptions, sizeof(short), 1, landfile);
     fread(Header.ElevFile, sizeof(char), 256, landfile);
     fread(Header.SlopeFile, sizeof(char), 256, landfile);
     fread(Header.AspectFile, sizeof(char), 256, landfile);
     fread(Header.FuelFile, sizeof(char), 256, landfile);
     fread(Header.CoverFile, sizeof(char), 256, landfile);
     fread(Header.HeightFile, sizeof(char), 256, landfile);
     fread(Header.BaseFile, sizeof(char), 256, landfile);
     fread(Header.DensityFile, sizeof(char), 256, landfile);
     fread(Header.DuffFile, sizeof(char), 256, landfile);
     fread(Header.WoodyFile, sizeof(char), 256, landfile);
     fread(Header.Description, sizeof(char), 512, landfile);
	if(Header.FOptions==1 || Header.FOptions==3)
		NEED_CUST_MODELS=true;
	else
		NEED_CUST_MODELS=false;
	if(Header.FOptions==2 || Header.FOptions==3)
		NEED_CONV_MODELS=true;
	else
		NEED_CONV_MODELS=false;
	HAVE_CUST_MODELS=false;
	HAVE_CONV_MODELS=false;
	// set raster resolution
//	RasterCellResolutionX=(Header.EastUtm-Header.WestUtm)/(double) Header.numeast;
//	RasterCellResolutionY=(Header.NorthUtm-Header.SouthUtm)/(double) Header.numnorth;
//	ViewPortNorth=RasterCellResolutionY*(double) Header.numnorth+Header.lonorth;
//	ViewPortSouth=Header.lonorth;
//	ViewPortEast=RasterCellResolutionX*(double) Header.numeast+Header.loeast;
//	ViewPortWest=Header.loeast;
//	NumViewNorth=(ViewPortNorth-ViewPortSouth)/Header.YResol;
//	NumViewEast=(ViewPortEast-ViewPortWest)/Header.XResol;
/*	double rows, cols;
	rows=(ViewPortNorth-ViewPortSouth)/Header.YResol;
	NumViewNorth=rows;
	if(modf(rows, &rows)>0.5)
		NumViewNorth++;
	cols=(ViewPortEast-ViewPortWest)/Header.XResol;
	NumViewEast=cols;
	if(modf(cols, &cols)>0.5)
		NumViewEast++;
*/
     if(HaveCrownFuels())
     {	if(HaveGroundFuels())
     		NumVals=10;
	     else
     		NumVals=8;
     }
     else
     {	if(HaveGroundFuels())
     		NumVals=7;
	     else
     		NumVals=5;
     }
     //CantAllocLCP=false;


     if(landscape==0)
     {    
		 if(CantAllocLCP==false)
     	{	int i;

     		fseek(landfile, headsize, SEEK_SET);
			int64_t rowSize = (int64_t)Header.numeast * (int64_t)NumVals;
			int64_t allOfIt = (int64_t)Header.numnorth * rowSize;
			try 
			{
				if((landscape=new short[allOfIt])!=NULL)
          		{    
					//ALM ZeroMemory(landscape, allOfIt*sizeof(short));
					memset(landscape, 0x0, allOfIt*sizeof(short));
          			for(int64_t i64=0; i64<Header.numnorth; i64++)
          				fread(&landscape[i64*rowSize], sizeof(short),
                    			rowSize, landfile);
          			fseek(landfile, headsize, SEEK_SET);
	//     	     	OldFilePosition=0;     // thread local
               		CantAllocLCP=false;
          		}
          		else
          			CantAllocLCP=true;
			}
			catch(...)///memory allocation failed
			{
				landscape = NULL;
          		CantAllocLCP=true;
			}
          }
     }
	 analysisLoEast = Header.loeast = Header.WestUtm;
	 analysisHiEast = Header.hieast = Header.EastUtm;
	 printf("analysisLoEast4 = %f", analysisLoEast);
	 printf("analysisHiEast4 = %f", analysisHiEast);
	 analysisLoNorth = Header.lonorth = Header.SouthUtm;
	 analysisHiNorth = Header.hinorth = Header.NorthUtm;
	 analysisRect.left = 0;
	 analysisRect.right = Header.numeast - 1;
	 analysisRect.top = 0;
	 analysisRect.bottom = Header.numnorth - 1;
//	int p;
//   CellData(Header.loeast, Header.hinorth, &p);
}
float FlamMap::GetLayerValueByCell(int _layer, int col, int row)
{
	float ret = -1;
	celldata tCell;
	crowndata tCrown;
	grounddata tGround;
	int tCol = col + analysisRect.left, tRow = row + analysisRect.top;
	int northi = tRow;//Header.numnorth - tRow - 1;// - analysisRect.top;
	//int northi = Header.numnorth - row - 1;// - analysisRect.top;
	if(tCol < 0 || tCol >= Header.numeast)
		return -1;
	if(northi < 0 || northi >= Header.numnorth)
		return -1;
	//int posit=(northi*Header.numeast+col);
	int64_t posit=((int64_t)northi*(int64_t)Header.numeast+col);
	if(!CantAllocLCP)
		GetCellDataFromMemory(posit, tCell, tCrown, tGround);
	else
	{
		double e = analysisRect.left + col * GetCellResolutionX();
		double n = analysisRect.bottom + row * GetCellResolutionY();
		CellData(e, n, tCell, tCrown, tGround, &posit);
	}
	switch(_layer)
	{
	case E_DATA:
		ret = tCell.e;
		break;
	case S_DATA:
		ret = tCell.s;
		break;
	case A_DATA:
		ret = tCell.a;
		break;
	case F_DATA:
		ret = tCell.f;
		break;
	case C_DATA:
		ret = tCell.c;
		break;
	case H_DATA:
		ret = tCrown.h;
		break;
	case B_DATA:
		ret = tCrown.b;
		break;
	case P_DATA:
		ret = tCrown.p;
		break;
	case D_DATA:
		ret = tGround.d;
		break;
	case W_DATA:
		ret = tGround.w;
		break;
	}
	return ret;
}

void FlamMap::GetCellDataFromMemory(int64_t posit, celldata &cell, crowndata &cfuel, grounddata &gfuel)
{
	short ldata[10];
	//copymemory
     memcpy(ldata, &landscape[posit*(int64_t)NumVals], NumVals*sizeof(short));
	switch(NumVals)
     {	case 5: 		// only 5 basic themes
			memcpy(&cell, ldata, NumVals*sizeof(short));
               break;
     	case 7:		// 5 basic and duff and woody
			memcpy(&cell, ldata, 5*sizeof(short));
               memcpy(&gfuel, &ldata[5], 2*sizeof(short));
			   cfuel.b = cfuel.h = cfuel.p = 0;
               break;
          case 8:		// 5 basic and crown fuels
			memcpy(&cell, ldata, 5*sizeof(short));
               memcpy(&cfuel, &ldata[5], 3*sizeof(short));
			   gfuel.d = gfuel.w = 0;
               break;
          case 10:		// 5 basic, crown fuels, and duff and woody
			memcpy(&cell, ldata, 5*sizeof(short));
               memcpy(&cfuel, &ldata[5], 3*sizeof(short));
               memcpy(&gfuel, &ldata[8], 2*sizeof(short));
               break;
     }
}

int64_t FlamMap::GetCellPosition(double east, double north)
{
	double xpt=(east-Header.loeast)/GetCellResolutionX();
	double ypt=(north-Header.lonorth)/GetCellResolutionY();
	int64_t easti=((int64_t) xpt);
	int64_t northi=((int64_t) ypt);
	northi=Header.numnorth-northi-1;
	if(northi<0)
		northi=0;
	int64_t posit=(northi*(int64_t)Header.numeast+easti);

	return posit;
}

int FlamMap::SetAnalysisArea(double tEast, double tWest, double tNorth, double tSouth)
{
	if(tEast > Header.EastUtm || tWest < Header.WestUtm || tNorth > Header.NorthUtm
		|| tSouth < Header.SouthUtm)
		return 0;
	analysisHiEast = tEast;
	printf("analysisHiEast6 = %f", analysisHiEast);
	analysisLoEast = tWest;
	printf("analysisLoEast6 = %f", analysisLoEast);
	analysisLoNorth = tSouth;
	analysisHiNorth = tNorth;
	//now figure new FMPRect
	int l, t, r, b;
	double temp;
	l = r = t = b = 0;
	l = (int)((tWest - Header.WestUtm) / Header.XResol);
	l = min(l, Header.numeast - 2);
	l = max(l, 0);
	temp = (tEast - Header.WestUtm) / Header.XResol - 1;
	r = (int)temp;
	if(modf(temp, &temp) > 0)
		r++;
	r = min(r, Header.numeast - 1);
	r = max(r, 0);
	t = (int)((Header.NorthUtm - tNorth) / Header.YResol);
	t = min(t, Header.numnorth - 2);
	t = max(t, 0);
	temp = (Header.NorthUtm - tSouth) / Header.YResol - 1;
	b = (int)temp;
	if(modf(temp, &temp) > 0)
		b++;
	b = min(b, Header.numnorth - 1);
	b = max(b, 0);
	analysisRect.left = l;
	analysisRect.right = r;
	analysisRect.top = t;
	analysisRect.bottom = b;
	return 1;
}

/*int FlamMap::GetNumEast()
{
	return analysisRect.Width();//Header.numeast;
}


int FlamMap::GetNumNorth()
{
	return analysisRect.Height();//Header.numnorth;
}

double FlamMap::GetWestUtm()
{
	return analysisLoEast;//Header.WestUtm;
}

double FlamMap::GetEastUtm()
{
	return analysisHiEast;//Header.EastUtm;
}

double FlamMap::GetSouthUtm()
{
	return analysisLoNorth;//Header.SouthUtm;
}

double FlamMap::GetNorthUtm()
{
	return analysisHiNorth;//Header.NorthUtm;
}

double FlamMap::GetLoEast()
{
	return analysisLoEast;//Header.loeast;
}
double FlamMap::GetHiEast()
{
	return analysisHiEast;//Header.hieast;
}

double FlamMap::GetLoNorth()
{
	return analysisLoNorth;//Header.lonorth;
}

double FlamMap::GetHiNorth()
{
	return analysisHiNorth;//Header.hinorth;
}*/

int FlamMap::GetFuelCats(int ThemeNo, int *cats)
{
     int number;

     switch(ThemeNo)
     {    case 0: number=Header.numelev;
               memcpy(cats, Header.elevs, 100*sizeof(int));
               break;
          case 1: number=Header.numslope;
               memcpy(cats, Header.slopes, 100*sizeof(int));
               break;
          case 2: number=Header.numaspect;
               memcpy(cats, Header.aspects, 100*sizeof(int));
               break;
          case 3: number=Header.numfuel;
               memcpy(cats, Header.fuels, 100*sizeof(int));
               break;
          case 4: number=Header.numcover;
               memcpy(cats, Header.covers, 100*sizeof(int));
               break;
          case 5: number=Header.numheight;
               memcpy(cats, Header.heights, 100*sizeof(int));
               break;
          case 6: number=Header.numbase;
               memcpy(cats, Header.bases, 100*sizeof(int));
               break;
          case 7: number=Header.numdensity;
               memcpy(cats, Header.densities, 100*sizeof(int));
               break;
          case 8: number=Header.numduff;
               memcpy(cats, Header.duffs, 100*sizeof(int));
               break;
          case 9: number=Header.numwoody;
               memcpy(cats, Header.woodies, 100*sizeof(int));
               break;
     }

     return number;
}

double FlamMap::ConvertEastingOffsetToUtm(double input)
{
     double MetersToKm=1.0;
     double ipart;

     if(Header.GridUnits==2)
     	MetersToKm=0.001;

     modf(Header.WestUtm/1000.0, &ipart);

	return (input+ipart*1000.0)*MetersToKm;
}

double FlamMap::ConvertNorthingOffsetToUtm(double input)
{
	double MetersToKm=1.0;
     double ipart;

     if(Header.GridUnits==2)
     	MetersToKm=0.001;

     modf(Header.SouthUtm/1000.0, &ipart);

	return (input+ipart*1000.0)*MetersToKm;
}

double FlamMap::ConvertUtmToEastingOffset(double input)
{
	double KmToMeters=1.0;
     double ipart;

     if(Header.GridUnits==2)
     	KmToMeters=1000.0;

     modf(Header.WestUtm/1000.0, &ipart);

	return input*KmToMeters-ipart*1000.0;
}

double FlamMap::ConvertUtmToNorthingOffset(double input)
{
	double KmToMeters=1.0;
     double ipart;

     if(Header.GridUnits==2)
     	KmToMeters=1000.0;

     modf(Header.SouthUtm/1000.0, &ipart);

	return input*KmToMeters-ipart*1000.0;
}


char *FlamMap::GetHeaderDescription()
{
	return Header.Description;
}

//-----------------------------------------------------------------------------

int FlamMap::GetLoElev()
{
	return Header.loelev;
}

int FlamMap::GetHiElev()
{
	return Header.hielev;
}

int FlamMap::GetLatitude()
{
	return Header.latitude;
}

void FlamMap::CloseLandFile()
{
     if(landscape)
     {	delete[] landscape;//GlobalFree(landscape);//free(landscape);
          landscape=0;
     	CantAllocLCP=false;
    }
	if(landfile)
		fclose(landfile);
     memset(LandFName, 0x0, sizeof LandFName);
	landfile=0;
	CanopyChx.Height=CanopyChx.DefaultHeight;
	CanopyChx.CrownBase=CanopyChx.DefaultBase;
	CanopyChx.BulkDensity=CanopyChx.DefaultDensity;
}

char *FlamMap::GetLandFileName()
{
	return LandFName;
}

void FlamMap::SetLandFileName(char* FileName)
{
	memset(LandFName, 0x0, sizeof LandFName);
	sprintf(LandFName, "%s", FileName);
}
/*
bool FlamMap::SetAtmosphereGrid(int NumGrids)
{
	if(NumGrids==0)
     {    
     	if(AtmGrid)
          	delete AtmGrid;
          AtmGrid=0;
		//NorthGridOffset=0.0;
		//EastGridOffset=0.0;

          return false;
     }
	if(AtmGrid==0)
     	AtmGrid=new AtmosphereGrid(NumGrids, this);
     else
     {	delete AtmGrid;
      	AtmGrid=new AtmosphereGrid(NumGrids, this);
     }

     return true;
}
*/
/*
AtmosphereGrid* FlamMap::GetAtmosphereGrid()
{
	return AtmGrid;
}

int FlamMap::AtmosphereGridExists()
{
	if(AtmGrid)
     {	if(AtmGrid->AtmGridWTR)
     	    	return 2;
          else if(AtmGrid->AtmGridWND)
			return 1;
     }

     return 0;
}
*/
void FlamMap::GetCurrentFuelMoistures(int fuelmod, int woodymod, double *mxin, double *mxout, int NumMx)
{
	bool Combine;
     int i, NumClasses=0;

     memset(mxout, 0x0, 20*sizeof(double));

     switch(WoodyCombineOptions(GETVAL))
     {	case CWD_COMBINE_NEVER: Combine=false; break;
     	case CWD_COMBINE_ALWAYS: Combine=true; break;
     	case CWD_COMBINE_ABSENT:
               if(woodymod<1)
               	Combine=true;
               else	if(coarsewoody[woodymod-1].wd==0)
               	Combine=true;
               else
               	Combine=false;
               break;
     }

	if(Combine)
     {    if(fuelmod<14)
	     {	switch(fuelmod)
     		{   	case 0: 	break;
	     		case 1:	memcpy(mxout, mxin, 3*sizeof(double));
                              NumClasses=3;
                    		break;
	     		case 2:	memcpy(mxout, mxin, 3*sizeof(double));
                    		mxout[3]=mxin[5];
                              NumClasses=4;
               			break;
		     	case 3:	memcpy(mxout, mxin, 3*sizeof(double));
                              NumClasses=3;
                    		break;
	     		case 4:	memcpy(mxout, mxin, 3*sizeof(double));
                    		mxout[3]=mxin[5];
                              NumClasses=4;
                    		break;
	     		case 5:	memcpy(mxout, mxin, 3*sizeof(double));
	               	     mxout[3]=mxin[5];
                              NumClasses=4;
                    		break;
		     	case 6:	memcpy(mxout, mxin, 3*sizeof(double));
						NumClasses=3;
                    		break;
		     	case 7:	memcpy(mxout, mxin, 4*sizeof(double));
						NumClasses=4;
                    		break;
	     		case 8:	memcpy(mxout, mxin, 3*sizeof(double));
						NumClasses=3;
                    		break;
	     		case 9:	memcpy(mxout, mxin, 3*sizeof(double));
						NumClasses=3;
                    		break;
		     	case 10:	memcpy(mxout, mxin, 3*sizeof(double));
						mxout[3]=mxin[5];
                              NumClasses=4;
                    		break;
		     	case 11:	memcpy(mxout, mxin, 3*sizeof(double));
                              NumClasses=3;
                    		break;
	     		case 12:	memcpy(mxout, mxin, 3*sizeof(double));
                              NumClasses=3;
                    		break;
	     		case 13:	memcpy(mxout, mxin, 3*sizeof(double));
                              NumClasses=3;
                    		break;
	          }
     	}
	     else
     	{	//if(FuelMod.TL1[fuelmod-14]>0.0)        // always have 1, 10, 100 hr fuels
     			mxout[NumClasses++]=mxin[0];
	     	//if(FuelMod.TL10[fuelmod-14]>0.0)
	     		mxout[NumClasses++]=mxin[1];
		    	//if(FuelMod.TL100[fuelmod-14]>0.0)
	     		mxout[NumClasses++]=mxin[2];
	     	if(newfuels[fuelmod].lh>0.0)//FuelMod.TLLiveH[fuelmod-14]>0.0)
     			mxout[NumClasses++]=mxin[6];
     		if(newfuels[fuelmod].lw>0.0)//FuelMod.TLLiveW[fuelmod-14]>0.0)
     			mxout[NumClasses++]=mxin[5];
	     }
     }
     if(woodymod>0)
     {	for(i=0; i<coarsewoody[woodymod-1].NumClasses; i++)
     		mxout[NumClasses+i]=coarsewoody[woodymod-1].wd[i].FuelMoisture;
		memcpy(&mxout[NumClasses], mxin, 3*sizeof(double));
          mxout[3]=mxin[5];
     }
}

void FlamMap::FreeOutputLayer(int Layer)
{
	if(Layer >= MAXSPOT_DIR)
		return;
	if(outlayer[Layer])
           delete[] outlayer[Layer];
     outlayer[Layer]=0;
	 //fd = filedesc()
	 //ALM if(outFiles[Layer].m_hFile != CFile::hFileNull && outFiles[Layer].GetFilePath().GetLength() > 0)
	 char temp [PATH_MAX];
	 if(&outFiles[Layer] != NULL && &outFiles[Layer] > 0)
	 {


		 const char * delFileName = (const char *) &outFiles[Layer];
		//		   std::ifstream ifs(delFileName, std::ios::binary);
				   //std::ofstream ofs(BackupFile, std::ios::binary);
				   
          	   //  ifs.init(ifs.rdbuf());




		 //ALM CString delFileName = outFiles[Layer].GetFilePath();
		 
		// close(this.&outFiles[Layer]));
		 //CFile::Remove(delFileName);
		 //try to remove it! might beopen by someone else!
		 close(*delFileName);
		 try
		{
			remove(delFileName);
		}
		catch (...)
		{
			printf("File %20s cannot be removed\n", delFileName);
			 //pEx->Delete();
		}

	 }
}


#ifdef wowowwo
void FlamMap::FreeWindData(int StationNumber)
{
	if(wddt[StationNumber])
	{    delete[] wddt[StationNumber];//free(wddt[StationNumber]);
		NumWindStations--;
          MaxWindObs[StationNumber]=0;
	}
	wddt[StationNumber]=0;
     FirstMonth[StationNumber].wnd=0;
	LastMonth[StationNumber].wnd=0;
	FirstDay[StationNumber].wnd=0;
	LastDay[StationNumber].wnd=0;
	FirstHour[StationNumber].wnd=0;
	LastHour[StationNumber].wnd=0;
}
#endif 



bool FlamMap::SelectOutputLayer(int Layer, bool truefalse)
{
	//printf("FlamMap::SelectoutputLayer1\n");
	int selLayer = Layer;
	if(selLayer == MAXSPOT_DX || selLayer == MAXSPOT_DIR)
		selLayer = MAXSPOT;
	if(selLayer == WINDSPEEDGRID || selLayer == WINDDIRGRID)
		return true;
	//printf("FlamMap::SelectoutputLayer2\n");
	FreeOutputLayer(selLayer);
	//printf("FlamMap::SelectoutputLayer3\n");
	if(truefalse)
     {	
		 //printf("FlamMap::SelectoutputLayer4\n");
		 //if((outlayer[Layer]=(float *) calloc(GetNumNorth()*GetNumEast(), sizeof(float)))==NULL)
		 int64_t sz = (int64_t)GetNumNorth() * (int64_t)GetNumEast();
		 //printf("sz = %i  useMemOutputs = %i\n", sz, this->icf.i_useMemoryOutputs);
		 if(this->icf.i_useMemoryOutputs)
		 {
			 //printf("FlamMap::SelectoutputLayer5\n");
			 try
			 {
				 //printf("FlamMap::SelectoutputLayer6\n");
				 outlayer[selLayer]=new float[sz];
				if(outlayer[selLayer])
				{
					//printf("FlamMap::SelectoutputLayer7\n");
					int i;
          			for(i=0; i<sz; i++)
               		outlayer[selLayer][i]=-1.0;
				}
			}
			catch(...)
			{//memory allocation failed, use output file instead
			}
		}
		 //printf("FlamMap::SelectoutputLayer8\n");
 		if(!outlayer[selLayer])
		{
			//printf("FlamMap::SelectoutputLayer9\n");
			FreeOutputLayer(selLayer);//safety
			//printf("FlamMap::SelectoutputLayer10\n");
			char bName[512];
			GetNextBinFileName(bName);
			//printf("FlamMap::SelectoutputLayer11  %c\n", bName);
			//CA2T szr(bName);
				//ALM CFile tFile;
			outFiles[selLayer] = fopen(bName, "w+");//ALM
			char buffer[(sz * sizeof(float))];
			//outFiles[selLayer] is an array of file pointers, szr is the file name, CA2T does some wierd stream conversion and buffer thing.
			//outFiles[selLayer] = fread((long)sz, tFile);
				//if(outFiles[selLayer].Open(szr, FILE::modeCreate | FILE::modeReadWrite | FILE::typeBinary | FILE::shareDenyNone) == 0)//error Opening file...
			if (outFiles[selLayer] == NULL)//ALM
			{
				printf("FlamMap::SelectoutputLayer12\n");
				return false;
			}
			//outFiles[selLayer].SetLength(sz * sizeof(float));

			//printf("FlamMap::SelectoutputLayer13\n");
			//&outFiles[selLayer] = sz * sizeof(double);
			//printf("%i   %i   %i   %i\n", sizeof(long), sizeof(int), sizeof(float), sizeof(double));
			//printf("%i   %i   %i   %i  %i  %i   %i   %i   %i\n", sz * sizeof(float), sizeof(&outFiles[selLayer]), sizeof(outFiles[selLayer]), selLayer, sizeof(int), sizeof(float), sizeof(long), sizeof(double), sizeof(int64_t));
			//long asdf = ftell(outFiles[selLayer]);
			setbuf(outFiles[selLayer], buffer);
			//printf("size of outFiles = %i  size of buffer = %i\n", buffer);
			fseek(outFiles[selLayer], 0, SEEK_SET);
			//printf("FlamMap::SelectoutputLayer14\n");
			//outFiles[selLayer].SeekToBegin();
			//float f = NODATA_VAL;
			//for(int i = 0; i < sz; i++)
			//	outFiles[Layer].Write(&f, sizeof(float));
		}
    	/*if((outlayer[selLayer]=new float[GetNumNorth()*GetNumEast()])==NULL)//(float *) GlobalAlloc(GMEM_FIXED, GetNumNorth()*GetNumEast()*sizeof(float)))==NULL)
               return false;
     	else
          {	int i, j;
          	j=GetNumNorth()*GetNumEast();
          	for(i=0; i<j; i++)
               	outlayer[selLayer][i]=-1.0;
          }*/
     }
	if (outFiles[10] != NULL)printf("pFlamMap->outFiles[layerNum] is openq\n");
	//printf("FlamMap::SelectoutputLayer15\n");
     return true;
}

/*int FlamMap::GetMoistCalcInterval(int FM_SIZECLASS, int CATEGORY)
{
	return MoistCalcInterval[FM_SIZECLASS][CATEGORY];
}*/

void FlamMap::SetMoistCalcInterval(int FM_SIZECLASS, int CATEGORY, int Val)
{
	switch(CATEGORY)
     {	case FM_INTERVAL_TIME:
     			switch(FM_SIZECLASS)
                    {	case 1:
                              if(Val<60)
                              	Val=60;
                              if(Val>240)
                              	Val=240;
                    		break;
                    	case 2:
                              if(Val<60)
                              	Val=60;
                              if(Val>240)
                              	Val=240;
                    		break;
                         case 3:
                              if(Val<60)
                              	Val=60;
                              if(Val>240)
                              	Val=240;
                    		break;
                    }
          	break;
		case FM_INTERVAL_ELEV:
				if(Val<10)
                    	Val=10;
          	break;
		case FM_INTERVAL_SLOPE:
				if(Val<5)
                    	Val=5;
          	break;
		case FM_INTERVAL_ASP:
				if(Val<5)
                    	Val=5;
          	break;
		case FM_INTERVAL_COV:
				if(Val<5)
                    	Val=5;
          	break;
     }
	MoistCalcInterval[FM_SIZECLASS][CATEGORY]=Val;
}

/**********************************************************
* Name: ConstFuelMoistures
* Desc: Check or Set the Constant Fuel Moisture Switch 
*       true = Yes we are using Constant Fuel Moisture
*       false = NO we're not using Constant Fuel Moisture
*               meaning we are using Fuel Moist Conditioning
**********************************************************/
int FlamMap::ConstFuelMoistures(int Const)
{
   if ( Const >= 0 )
     	ConstFuelMoisture = (bool) Const;
   //return ConstFuelMoisture;
   return true;//ALM
}

/******************************************************/
int FlamMap::GetJulianDays(int Month)
{
     int days = 0;

	switch(Month)
	{	case 1: days=0; break;			// cumulative days to begin of month
		case 2: days=31; break;           // except ignores leapyear, but who cares anyway,
		case 3: days=59; break;
		case 4: days=90; break;
		case 5: days=120; break;
		case 6: days=151; break;
		case 7: days=181; break;
		case 8: days=212; break;
		case 9: days=243; break;
		case 10: days=273; break;
		case 11: days=304; break;
		case 12: days=334; break;
	}

     return days;
}


int FlamMap::SetStartProcessor(int _procNum)
{
//SYSTEM_INFO sysInfo;
	  //GetSystemInfo(&sysInfo);
	  int numCPU = sysconf( _SC_NPROCESSORS_ONLN );
	  if (_procNum >= 0 && _procNum < numCPU)
	   	StartProcessor = _procNum;
	  return StartProcessor;
}

int FlamMap::GetStartProcessor()
{
	return StartProcessor;
}

int FlamMap::GetMaxThreads()
{
	return MaxThreads;
}

void FlamMap::SetMaxThreads(int numthreads)
{
	if (numthreads > 0 && numthreads < 65)
		MaxThreads = numthreads;
}


void FlamMap::WriteFriskToDisk()
{
     //int i, j;
	FILE *outfile;
     char FileName[256]="";
     int nodata=-1;

     strcat(FileName, FlameAceBaseName);
     strcat(FileName, ".FSK");
     outfile=fopen(FileName, "wb");
	int wid, hgt;
	wid = analysisRect.Width();
	hgt = analysisRect.Height();
	double w, s;
	w = analysisLoEast;
	s = analysisLoNorth;
     /*fwrite(&Header.numeast, sizeof(int), 1, outfile);
     fwrite(&Header.numnorth, sizeof(int), 1, outfile);
     fwrite(&Header.WestUtm, sizeof(double), 1, outfile);
     fwrite(&Header.SouthUtm, sizeof(double), 1, outfile);*/
	 fwrite(&wid, sizeof(int), 1, outfile);
     fwrite(&hgt, sizeof(int), 1, outfile);
     fwrite(&Header.WestUtm, sizeof(double), 1, outfile);
     fwrite(&Header.SouthUtm, sizeof(double), 1, outfile);
     fwrite(&Header.XResol, sizeof(double), 1, outfile);
     fwrite(&nodata, sizeof(int), 1, outfile);

     fwrite(outlayer[MAXSPREADDIR], sizeof(float), wid*hgt, outfile);
     fwrite(outlayer[ELLIPSEDIM_A], sizeof(float), wid*hgt, outfile);
     fwrite(outlayer[ELLIPSEDIM_B], sizeof(float), wid*hgt, outfile);
     fwrite(outlayer[ELLIPSEDIM_C], sizeof(float), wid*hgt, outfile);
     fwrite(outlayer[MAXSPOT], sizeof(float), wid*hgt, outfile);
     fclose(outfile);
}
bool FlamMap::GetOutputOption(int Layer)
{
	if(Layer == MAXSPOT_DX || Layer == MAXSPOT_DIR)
	{
		if(&outlayer[MAXSPOT]!= NULL && &outFiles[MAXSPOT] > 0)
			return true;
		return false;
	}
	if(Layer == WINDSPEEDGRID || Layer == WINDDIRGRID)
	{
		return false;
	}
//ALM	if(outlayer[Layer] ||  (outFiles[Layer].m_hFile!= CFile::hFileNull && outFiles[Layer].GetFilePath().GetLength() > 0))

	if(&outlayer[Layer]!= NULL && &outFiles[Layer] > 0)
     	return true;

     return false;
}

int FlamMap::WriteOutputLayerToDisk(int Layer)
{
	int ret = 1;//success
     int i, j;
	FILE *outfile;
     char FileName[256]="";

     strcat(FileName, FlameAceBaseName);
     switch(Layer)
     {	case FLAMELENGTH:
               strcat(FileName, ".FML");
     		break;
     	case SPREADRATE:
               strcat(FileName, ".ROS");
     		break;
          case INTENSITY:
               strcat(FileName, ".FLI");
               break;
          case HEATAREA:
               strcat(FileName, ".HPA");
               break;
          case CROWNSTATE:
               strcat(FileName, ".CFR");
               break;
          case SOLARRADIATION:
               strcat(FileName, ".SLR");
               break;
          case FUELMOISTURE1:
               strcat(FileName, ".FMO");
               break;
          case FUELMOISTURE10:
               strcat(FileName, ".FMT");
               break;
          case MIDFLAME:
               strcat(FileName, ".MFW");
               break;
          case HORIZRATE:
               strcat(FileName, ".RHM");
               break;
          case MAXSPREADDIR:
               strcat(FileName, ".MDR");
               break;
          case ELLIPSEDIM_A:
               strcat(FileName, ".EDA");
               break;
          case ELLIPSEDIM_B:
               strcat(FileName, ".EDB");
               break;
          case ELLIPSEDIM_C:
               strcat(FileName, ".EDC");
               break;
          case MAXSPOT:
               strcat(FileName, ".SPT");
               break;
          case MAXSPOT_DIR:
               strcat(FileName, ".SDR");
               break;
          case MAXSPOT_DX:
               strcat(FileName, ".SDX");
               break;
     }
     outfile=fopen(FileName, "w");
     fprintf(outfile, "%s %ld\n", "NCOLS", GetNumEast());
     fprintf(outfile, "%s %ld\n", "NROWS", GetNumNorth());
     fprintf(outfile, "%s %lf\n", "XLLCORNER", GetWestUtm());
     fprintf(outfile, "%s %lf\n", "YLLCORNER", GetSouthUtm());
     fprintf(outfile, "%s %lf\n", "CELLSIZE", GetCellResolutionX());
     fprintf(outfile, "%s %ld\n", "NODATA_VALUE", -1);
	 for(i=0; i<GetNumNorth(); i++)
	 {  	
		 for(j=0; j<GetNumEast(); j++)
		 {
			if(Layer == MAXSPOT_DIR)
			{
				double tmp = outlayer[MAXSPOT][i*GetNumEast()+j];
				double dist;
				double wdir = 0.0;
				if(tmp != 0.0)
					wdir = modf(tmp, &dist);
				wdir *= 10.0;
          		fprintf(outfile, "%f ", wdir);
			}
			else if(Layer == MAXSPOT_DX)
			{
				double tmp = outlayer[MAXSPOT][i*GetNumEast()+j];
				double dist = 0.0;
				double wdir = 0.0;
				if(tmp != 0.0)
					wdir = modf(tmp, &dist);
          		fprintf(outfile, "%f ", dist);
			}
			else
				fprintf(outfile, "%f ", outlayer[Layer][i*GetNumEast()+j]);
		 }
		 fprintf(outfile, "\n");
	 }
     fclose(outfile);
	 return ret;
}

char * FlamMap::GetOutputLayerName32char(int layer, char *textBuf)
{
	char buf[33];
	for(int i = 0; i < 32; i++)
		buf[i] = ' ';
	buf[32] = 0;
	switch(layer)
	{
	case FLAMELENGTH:
		sprintf(buf, "%-32.32s", "FLAMELENGTH");
		break;
	case SPREADRATE:
		sprintf(buf, "%-32.32s", "SPREADRATE");
		break;
	case INTENSITY:
		sprintf(buf, "%-32.32s", "INTENSITY");
		break;
	case HEATAREA:
		sprintf(buf, "%-32.32s", "HEATAREA");
		break;
	case CROWNSTATE:
		sprintf(buf, "%-32.32s", "CROWNSTATE");
		break;
	case SOLARRADIATION:
		sprintf(buf, "%-32.32s", "SOLARRADIATION");
		break;
	case FUELMOISTURE1:
		sprintf(buf, "%-32.32s", "FUELMOISTURE1");
		break;
	case FUELMOISTURE10:
		sprintf(buf, "%-32.32s", "FUELMOISTURE10");
		break;
	case MIDFLAME:
		sprintf(buf, "%-32.32s", "MIDFLAME");
		break;
	case HORIZRATE:
		sprintf(buf, "%-32.32s", "HORIZRATE");
		break;
	case MAXSPREADDIR:
		sprintf(buf, "%-32.32s", "MAXSPREADDIR");
		break;
	case ELLIPSEDIM_A:
		sprintf(buf, "%-32.32s", "ELLIPSEDIM_A");
		break;
	case ELLIPSEDIM_B:
		sprintf(buf, "%-32.32s", "ELLIPSEDIM_B");
		break;
	case ELLIPSEDIM_C:
		sprintf(buf, "%-32.32s", "ELLIPSEDIM_C");
		break;
	case MAXSPOT:
		sprintf(buf, "%-32.32s", "MAXSPOT");
		break;
	case FUELMOISTURE100:
		sprintf(buf, "%-32.32s", "FUELMOISTURE100");
		break;
	case FUELMOISTURE1000:
		sprintf(buf, "%-32.32s", "FUELMOISTURE1000");
		break;
	case MAXSPOT_DIR:
		sprintf(buf, "%-32.32s", "MAXSPOT_DIR");
		break;
	case MAXSPOT_DX:
		sprintf(buf, "%-32.32s", "MAXSPOT_DX");
		break;
	case WINDDIRGRID:
		sprintf(buf, "%-32.32s", "WINDDIRGRID");
		break;
	case WINDSPEEDGRID:
		sprintf(buf, "%-32.32s", "WINDSPEEDGRID");
		break;
	default:
		sprintf(buf, "%-32.32s", "UNKNOWN");
		break;
	}
	strcpy(textBuf, buf);
	return textBuf;
}

int CloseAndReturn(FILE *stream, int returnCode)
{
	int retCode = returnCode;
	if(fclose(stream) != 0)
	{
		if(returnCode != 1)
			retCode = e_EMS_FILE_CLOSE_ERROR;
	}
	stream = NULL;
	return retCode;
}

int  FlamMap::WriteOutputLayerBinary(int Layer, char *_fName)
{
	if(Layer == WINDDIRGRID || Layer == WINDSPEEDGRID)
	{
		return WriteWindGridBinary(Layer, _fName);
	}
	int trgLayer = Layer;
	if(Layer == MAXSPOT_DIR || Layer == MAXSPOT_DX)
		trgLayer = MAXSPOT;
	if ( outlayer[trgLayer] == 0 
		&& (&outFiles[trgLayer] == NULL || &outFiles[trgLayer] <= 0))
		return e_EMS_OUTPUT_DOES_NOT_EXIST; 

	int numNorth = GetNumNorth(), numEast = GetNumEast(), layerType = Layer;
	float cellResX = GetCellResolutionX(), cellResY = GetCellResolutionY(), westUTM = GetWestUtm(), 
		southUTM = GetSouthUtm();
	float nullVal = NODATA_VAL;
	FILE *stream = fopen(_fName, "wb");
	if(!stream)
		return e_EMS_FILE_OPEN_ERROR;
	char textBuf[64];
	GetOutputLayerName32char(Layer, textBuf);
	if(fwrite(textBuf, sizeof(char), 32, stream) != 32)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	if(fwrite(&layerType, sizeof(layerType), 1, stream) != 1)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	if(fwrite(&numEast, sizeof(numEast), 1, stream) != 1)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	if(fwrite(&numNorth, sizeof(numNorth), 1, stream) != 1)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	if(fwrite(&westUTM, sizeof(westUTM), 1, stream) != 1)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	if(fwrite(&southUTM, sizeof(southUTM), 1, stream) != 1)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	if(fwrite(&cellResX, sizeof(cellResX), 1, stream) != 1)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	if(fwrite(&nullVal, sizeof(nullVal), 1, stream) != 1)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	/*char tmp[256];
	strcpy(tmp, title);
	fwrite(&tmp, sizeof(char), title.GetLength() + 1, stream);
	fwrite(&dataUnits, sizeof(dataUnits), 1, stream);*/
	int64_t offset;
	switch(Layer)
	{
	case MAXSPOT_DIR:
	case MAXSPOT_DX:
		{
			double outputVal, spotVal;
			double dist;
			double wdir = 0.0;
			for(int64_t i = 0; i < numNorth; i++)
			{
				for(int64_t j = 0; j < numEast; j++)
				{
					offset = i * (int64_t)GetNumEast() + j;
					spotVal = GetOutputLayerVal(MAXSPOT, offset);//outlayer[MAXSPOT][i*GetNumEast()+j];
					if(spotVal != 0.0)
						wdir = modf(spotVal, &dist);
					else
					{
						wdir = 0.0;
						dist = 0.0;
					}
					wdir *= 10.0;
					if(Layer == MAXSPOT_DIR)
					{
						wdir *= 180.0/PI;
						if(fwrite(&wdir, sizeof(float), 1, stream) != 1)
							return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
					}
					else//distance
						if(fwrite(&dist, sizeof(float), 1, stream) != 1)
							return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
				}
			}
		}
		break;
	default:
		if(Layer < NUM_STATICOUTPUTS)
		{
			if(outlayer[Layer])
				fwrite(outlayer[Layer], sizeof(float), numEast*numNorth, stream);
			else
			{
				for(int64_t i = 0; i < numNorth; i++)
				{
					for(int64_t j = 0; j < numEast; j++)
					{
						offset = i * (int64_t)GetNumEast() + j;
						float tmpVal = GetOutputLayerVal(Layer, offset);
						if(fwrite(&tmpVal, sizeof(float), 1, stream) != 1)
							return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);

						//fwrite(outlayer[Layer][i*GetNumEast()+j], sizeof(float), 1, stream);
					}
					//fprintf(outfile, "%f ", outlayer[Layer][i*GetNumEast()+j]);
				}
			}
		}
	}
	/*if(outlayer[Layer])
	{
		for(int i = 0; i < numNorth; i++)
		{
			fwrite(vals[i], sizeof(float), numEast, stream);
		}
	}
	else
	{
		for(int i = 0; i < numNorth; i++)
		{
			//fwrite(vals[i], sizeof(float), numEast, stream);
			for(int j = 0; j < numEast; j++)
			{
				float tVal = GetVal(j, i);
				fwrite(&tVal, sizeof(float), 1, stream);
			}
		}
	}*/
	return CloseAndReturn(stream, 1);
}

int FlamMap::WriteWindGridAscii(int Layer, char *_fName)
{
	if(Layer == WINDDIRGRID && !windDirGrid)
		return e_EMS_OUTPUT_DOES_NOT_EXIST; 
	if(Layer == WINDSPEEDGRID && !windSpeedGrid)
		return e_EMS_OUTPUT_DOES_NOT_EXIST; 
	char FileName[512]="";
	strcpy(FileName, _fName);
     FILE *outfile=fopen(FileName, "wt");
	 if(!outfile)
		return e_EMS_FILE_OPEN_ERROR;

		 if(fprintf(outfile, "%s %i\n", "NCOLS", nWindCols) < 0)
			return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
		 if(fprintf(outfile, "%s %i\n", "NROWS", nWindRows) < 0)
			return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
		 if(fprintf(outfile, "%s %lf\n", "XLLCORNER", windsXllCorner) < 0)
			return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
		 if(fprintf(outfile, "%s %lf\n", "YLLCORNER", windsYllCorner) < 0)
			return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
		 if(fprintf(outfile, "%s %lf\n", "CELLSIZE", windsResolution) < 0)
			return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
		 if(fprintf(outfile, "%s %i\n", "NODATA_VALUE", -1) < 0)
			return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
		 //for(int i=0; i<nWindRows; i++)
		 for(int i=nWindRows - 1; i>= 0; i--)
		 {  	
			 for(int j=0; j<nWindCols; j++)
			 {
				if(Layer == WINDDIRGRID)
				{
          			if(fprintf(outfile, "%.2f ", windDirGrid[i][j]) < 0)
						return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
		 
				}
				else if(Layer == WINDSPEEDGRID)
				{
          			if(fprintf(outfile, "%.2f ", windSpeedGrid[i][j]) < 0)
						return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
				}
			 }
			 if(fprintf(outfile, "\n") < 0)
				return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
		 }
		return CloseAndReturn(outfile, 1);

}

int FlamMap::WriteWindGridBinary(int Layer, char *_fName)
{
	if(Layer == WINDDIRGRID && !windDirGrid)
		return e_EMS_OUTPUT_DOES_NOT_EXIST; 
	if(Layer == WINDSPEEDGRID && !windSpeedGrid)
		return e_EMS_OUTPUT_DOES_NOT_EXIST; 
	int numNorth = nWindRows, numEast = nWindCols, layerType = Layer;
	float cellResX = windsResolution, westUTM = windsXllCorner, 
		southUTM = windsYllCorner;
	float nullVal = NODATA_VAL;
	FILE *stream = fopen(_fName, "wb");
	 if(!stream)
		return e_EMS_FILE_OPEN_ERROR;
	char textBuf[64];
	GetOutputLayerName32char(Layer, textBuf);
	if(fwrite(textBuf, sizeof(char), 32, stream) != 32)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	if(fwrite(&layerType, sizeof(layerType), 1, stream) != 1)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	if(fwrite(&numEast, sizeof(numEast), 1, stream) != 1)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	if(fwrite(&numNorth, sizeof(numNorth), 1, stream) != 1)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	if(fwrite(&westUTM, sizeof(westUTM), 1, stream) != 1)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	if(fwrite(&southUTM, sizeof(southUTM), 1, stream) != 1)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	if(fwrite(&cellResX, sizeof(cellResX), 1, stream) != 1)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	if(fwrite(&nullVal, sizeof(nullVal), 1, stream) != 1)
		return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
	float wgVal;
	//for(int i = 0; i < numNorth; i++)
	for(int i = numNorth - 1; i >= 0; i--)
	{
		for(int j = 0; j < numEast; j++)
		{
			if(Layer == WINDDIRGRID)
				wgVal = windDirGrid[i][j];
			else//assume speed
				wgVal = windSpeedGrid[i][j];
			if(fwrite(&wgVal, sizeof(float), 1, stream) != 1)
				return CloseAndReturn(stream, e_EMS_FILE_WRITE_ERROR);
		}
	}
	return CloseAndReturn(stream, 1);
}

int FlamMap::WriteOutputLayerToDisk(int Layer, char *_fName)
{
     int64_t i, j;
	FILE *outfile;
     char FileName[512]="";
	if(Layer == WINDDIRGRID || Layer == WINDSPEEDGRID)
	{
		return WriteWindGridAscii(Layer, _fName);
	}
/* Check - was Layer previously selected & memory allocated, else will crash...........*/
	 if ( Layer == MAXSPOT_DIR || Layer == MAXSPOT_DX )
		 i = MAXSPOT;
	 else
		 i = Layer; 
	 if ( outlayer[i] == 0 && (&outFiles[i] == NULL || &outFiles[i] <= 0))
		return e_EMS_OUTPUT_DOES_NOT_EXIST; 

	 strcpy(FileName, _fName);

	 int64_t offset;
     outfile=fopen(FileName, "w");
	 if(!outfile)
		 return e_EMS_FILE_OPEN_ERROR;
		 if(fprintf(outfile, "%s %ld\n", "NCOLS", GetNumEast()) < 0)
				return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
		 if(fprintf(outfile, "%s %ld\n", "NROWS", GetNumNorth()) < 0)
				return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
		 if(fprintf(outfile, "%s %lf\n", "XLLCORNER", GetWestUtm()) < 0)
				return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
		 if(fprintf(outfile, "%s %lf\n", "YLLCORNER", GetSouthUtm()) < 0)
				return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
		 if(fprintf(outfile, "%s %lf\n", "CELLSIZE", GetCellResolutionX()) < 0)
				return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
		 if(fprintf(outfile, "%s %i\n", "NODATA_VALUE", -1) < 0)
				return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
		 for(i=0; i<GetNumNorth(); i++)
		 {  	
			 for(j=0; j<GetNumEast(); j++)
			 {
				offset = i * (int64_t)GetNumEast() + j;
				/*ALM - I used this for testif (Layer == INTENSITY){
					double tmp = GetOutputLayerVal(INTENSITY, offset);
					printf("In WriteOutputlayertodisk iNTENSITY = %f", tmp);
				}*/
				if(Layer == MAXSPOT_DIR)
				{
					double tmp = GetOutputLayerVal(MAXSPOT, offset);//Getoutlayer[MAXSPOT][i*GetNumEast()+j];
					double dist;
					double wdir = 0.0;
					if(tmp != 0.0)
						wdir = modf(tmp, &dist);
					wdir *= 10.0;
					wdir *= 180.0/PI;
          			if(fprintf(outfile, "%f ", wdir) < 0)
						return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
				}
				else if(Layer == MAXSPOT_DX)
				{
					double tmp = GetOutputLayerVal(MAXSPOT, offset);//outlayer[MAXSPOT][i*GetNumEast()+j];
					double dist = 0.0;
					double wdir = 0.0;
					if(tmp != 0.0)
						double(wdir) = modf(double(tmp), &dist);
          			if(fprintf(outfile, "%f ", dist) < 0)
						return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
				}
				//else if(Layer == 
				else{
					/*if (Layer == INTENSITY){
						double tmp = GetOutputLayerVal(INTENSITY, offset);
						printf("In WriteOutputlayertodisk iNTENSITY = %f , %s", tmp, FileName);
					}*/
					if(fprintf(outfile, "%f ", GetOutputLayerVal(Layer, offset)) < 0){
					//printf("in the right spot\n");
						return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);

					}
					//else printf("in the right place\n");
				}
			 }
			 if(fprintf(outfile, "\n") < 0)
				return CloseAndReturn(outfile, e_EMS_FILE_WRITE_ERROR);
		 }
		return CloseAndReturn(outfile, 1);
}


void FlamMap::SetOutputBaseName(char *name)
{
     memset(FlameAceBaseName, 0x0, sizeof(FlameAceBaseName));
	strcpy(FlameAceBaseName, name);
}

bool FlamMap::SetInitialFuelMoistures(int Model, int t1, int t10, int t100, int tlh, int tlw)
{
     if(Model>256 || Model<1)
     	return false;

	fm[Model-1].TL1=t1;
	fm[Model-1].TL10=t10;
	fm[Model-1].TL100=t100;
	fm[Model-1].TLLH=tlh;
	fm[Model-1].TLLW=tlw;

     if(t1>1 && t10>1)
	     fm[Model-1].FuelMoistureIsHere=true;
     else
	     fm[Model-1].FuelMoistureIsHere=false;

     int i, j;
     for(i=0; i<4; i++)       // only up to 1000 hr fuels [3]
     {    for(j=0; j<5; j++)
               EnvtChanged[i][j]=true;
     }

     return fm[Model-1].FuelMoistureIsHere;
}

bool FlamMap::GetInitialFuelMoistures(int Model, int *t1, int *t10, int *t100, int *tlh, int *tlw)
{
     if(Model>256 || Model<1)
     	return false;

	if(fm[Model-1].FuelMoistureIsHere)
     {	*t1=fm[Model-1].TL1;
		*t10=fm[Model-1].TL10;
		*t100=fm[Model-1].TL100;
		*tlh=fm[Model-1].TLLH;
		*tlw=fm[Model-1].TLLW;

          return true;
     }

     return false;
}

int FlamMap::GetInitialFuelMoisture(int Model, int FuelClass)
{
     if(Model>256 || Model<1)
     	return 2;

	int mx;

	switch(FuelClass)
     {	case 0: mx=fm[Model-1].TL1; break;
     	case 1: mx=fm[Model-1].TL10; break;
          case 2: mx=fm[Model-1].TL100; break;
          case 3: mx=fm[Model-1].TLLH; break;
          case 4: mx=fm[Model-1].TLLW; break;
		 // case 5: mx = fm[Model-1].
     }

     return mx;
}

bool FlamMap::InitialFuelMoistureIsHere(int Model)
{
     if(Model>256 || Model<1)
     	return false;

	return fm[Model-1].FuelMoistureIsHere;
}

int FlamMap::AccessFuelModelUnits(int Val)
{
	if(Val>-1)
     	FuelModelUnits=Val;

	return FuelModelUnits;
}

void FlamMap::ResetNewFuels()
{
    	memset(newfuels, 0x0, 257*sizeof(NewFuel));

     InitializeNewFuel();

     int i;
     for(i=0; i<257; i++)
     {	if(newfuels[i].number)
          	newfuels[i].number*=-1;	// indicate permanence
     }

    	HAVE_CUST_MODELS=false;
}


bool FlamMap::SetNewFuel(NewFuel *newfuel)
{
	int FuelNum;//i, 

     if(newfuel==0)
     	return false;

     FuelNum=newfuel->number;
     if(FuelNum>256 || FuelNum<0)
     	return false;

     newfuel->number=0;
	if(newfuel->h1>0.0)
     	newfuel->number=FuelNum;
     else if(newfuel->h10>0.0)
     	newfuel->number=FuelNum;
     else if(newfuel->h100>0.0)
     	newfuel->number=FuelNum;
     else if(newfuel->lh>0.0)
     	newfuel->number=FuelNum;
     else if(newfuel->lw>0.0)
     	newfuel->number=FuelNum;

     memcpy(&newfuels[FuelNum], newfuel, sizeof(NewFuel));

	return true;
}


bool FlamMap::GetNewFuel(int number, NewFuel *newfuel)
{
	if(number<0)
     	return false;

	if(newfuels[number].number==0)
     	return false;

     if(newfuel!=0)
	{    memcpy(newfuel, &newfuels[number], sizeof(NewFuel));
	     newfuel->number=labs(newfuel->number);	// return absolute value of number
     }

     return true;
}


bool FlamMap::IsNewFuelReserved(int number)
{
	if(number<0)
     	return false;

	if(newfuels[number].number<0)
     	return true;

     return false;
}

void FlamMap::InitializeNewFuel()
{
     newfuels[1].number=1;
     strcpy(newfuels[1].code, "FM1");
     newfuels[1].h1=0.740000;
     newfuels[1].h10=0.000000;
     newfuels[1].h100=0.000000;
     newfuels[1].lh=0.000000;
     newfuels[1].lw=0.000000;
     newfuels[1].dynamic=0;
     newfuels[1].sav1=3500;
     newfuels[1].savlh=1800;
     newfuels[1].savlw=1500;
     newfuels[1].depth=1.000000;
     newfuels[1].xmext=0.120000;
     newfuels[1].heatd=8000.000000;
     newfuels[1].heatl=8000.000000;
     strcpy(newfuels[1].desc, "Short Grass");

     newfuels[2].number=2;
     strcpy(newfuels[2].code, "FM2");
     newfuels[2].h1=2.000000;
     newfuels[2].h10=1.000000;
     newfuels[2].h100=0.500000;
     newfuels[2].lh=0.000000;
     newfuels[2].lw=0.500000;
     newfuels[2].dynamic=0;
     newfuels[2].sav1=3000;
     newfuels[2].savlh=1800;
     newfuels[2].savlw=1500;
     newfuels[2].depth=1.000000;
     newfuels[2].xmext=0.150000;
     newfuels[2].heatd=8000.000000;
     newfuels[2].heatl=8000.000000;
     strcpy(newfuels[2].desc, "Timber Grass/Understory");

     newfuels[3].number=3;
     strcpy(newfuels[3].code, "FM3");
     newfuels[3].h1=3.010000;
     newfuels[3].h10=0.000000;
     newfuels[3].h100=0.000000;
     newfuels[3].lh=0.000000;
     newfuels[3].lw=0.000000;
     newfuels[3].dynamic=0;
     newfuels[3].sav1=1500;
     newfuels[3].savlh=1800;
     newfuels[3].savlw=1500;
     newfuels[3].depth=2.500000;
     newfuels[3].xmext=0.250000;
     newfuels[3].heatd=8000.000000;
     newfuels[3].heatl=8000.000000;
     strcpy(newfuels[3].desc, "Tall Grass");

     newfuels[4].number=4;
     strcpy(newfuels[4].code, "FM4");
     newfuels[4].h1=5.010000;
     newfuels[4].h10=4.010000;
     newfuels[4].h100=2.000000;
     newfuels[4].lh=0.000000;
     newfuels[4].lw=5.010000;
     newfuels[4].dynamic=0;
     newfuels[4].sav1=2000;
     newfuels[4].savlh=1800;
     newfuels[4].savlw=1500;
     newfuels[4].depth=6.000000;
     newfuels[4].xmext=0.200000;
     newfuels[4].heatd=8000.000000;
     newfuels[4].heatl=8000.000000;
     strcpy(newfuels[4].desc, "Chaparral");

     newfuels[5].number=5;
     strcpy(newfuels[5].code, "FM5");
     newfuels[5].h1=1.000000;
     newfuels[5].h10=0.500000;
     newfuels[5].h100=0.000000;
     newfuels[5].lh=0.000000;
     newfuels[5].lw=2.000000;
     newfuels[5].dynamic=0;
     newfuels[5].sav1=2000;
     newfuels[5].savlh=1800;
     newfuels[5].savlw=1500;
     newfuels[5].depth=2.000000;
     newfuels[5].xmext=0.200000;
     newfuels[5].heatd=8000.000000;
     newfuels[5].heatl=8000.000000;
     strcpy(newfuels[5].desc, "Short Brush");

     newfuels[6].number=6;
     strcpy(newfuels[6].code, "FM6");
     newfuels[6].h1=1.500000;
     newfuels[6].h10=2.500000;
     newfuels[6].h100=2.000000;
     newfuels[6].lh=0.000000;
     newfuels[6].lw=0.000000;
     newfuels[6].dynamic=0;
     newfuels[6].sav1=1750;
     newfuels[6].savlh=1800;
     newfuels[6].savlw=1500;
     newfuels[6].depth=2.500000;
     newfuels[6].xmext=0.250000;
     newfuels[6].heatd=8000.000000;
     newfuels[6].heatl=8000.000000;
     strcpy(newfuels[6].desc, "Dormant Brush");

     newfuels[7].number=7;
     strcpy(newfuels[7].code, "FM7");
     newfuels[7].h1=1.130000;
     newfuels[7].h10=1.870000;
     newfuels[7].h100=1.500000;
     newfuels[7].lh=0.000000;
     newfuels[7].lw=0.370000;
     newfuels[7].dynamic=0;
     newfuels[7].sav1=1550;
     newfuels[7].savlh=1800;
     newfuels[7].savlw=1500;
     newfuels[7].depth=2.500000;
     newfuels[7].xmext=0.400000;
     newfuels[7].heatd=8000.000000;
     newfuels[7].heatl=8000.000000;
     strcpy(newfuels[7].desc, "Southern Rough");

     newfuels[8].number=8;
     strcpy(newfuels[8].code, "FM8");
     newfuels[8].h1=1.500000;
     newfuels[8].h10=1.000000;
     newfuels[8].h100=2.500000;
     newfuels[8].lh=0.000000;
     newfuels[8].lw=0.000000;
     newfuels[8].dynamic=0;
     newfuels[8].sav1=2000;
     newfuels[8].savlh=1800;
     newfuels[8].savlw=1500;
     newfuels[8].depth=0.200000;
     newfuels[8].xmext=0.300000;
     newfuels[8].heatd=8000.000000;
     newfuels[8].heatl=8000.000000;
     strcpy(newfuels[8].desc, "Closed Timber Litter");

     newfuels[9].number=9;
     strcpy(newfuels[9].code, "FM9");
     newfuels[9].h1=2.920000;
     newfuels[9].h10=0.410000;
     newfuels[9].h100=0.150000;
     newfuels[9].lh=0.000000;
     newfuels[9].lw=0.000000;
     newfuels[9].dynamic=0;
     newfuels[9].sav1=2500;
     newfuels[9].savlh=1800;
     newfuels[9].savlw=1500;
     newfuels[9].depth=0.200000;
     newfuels[9].xmext=0.250000;
     newfuels[9].heatd=8000.000000;
     newfuels[9].heatl=8000.000000;
     strcpy(newfuels[9].desc, "Hardwood Litter");

     newfuels[10].number=10;
     strcpy(newfuels[10].code, "FM10");
     newfuels[10].h1=3.010000;
     newfuels[10].h10=2.000000;
     newfuels[10].h100=5.010000;
     newfuels[10].lh=0.000000;
     newfuels[10].lw=2.000000;
     newfuels[10].dynamic=0;
     newfuels[10].sav1=2000;
     newfuels[10].savlh=1800;
     newfuels[10].savlw=1500;
     newfuels[10].depth=1.000000;
     newfuels[10].xmext=0.250000;
     newfuels[10].heatd=8000.000000;
     newfuels[10].heatl=8000.000000;
     strcpy(newfuels[10].desc, "Timber Litter/Understory");

     newfuels[11].number=11;
     strcpy(newfuels[11].code, "FM11");
     newfuels[11].h1=1.500000;
     newfuels[11].h10=4.510000;
     newfuels[11].h100=5.510000;
     newfuels[11].lh=0.000000;
     newfuels[11].lw=0.000000;
     newfuels[11].dynamic=0;
     newfuels[11].sav1=1500;
     newfuels[11].savlh=1800;
     newfuels[11].savlw=1500;
     newfuels[11].depth=1.000000;
     newfuels[11].xmext=0.150000;
     newfuels[11].heatd=8000.000000;
     newfuels[11].heatl=8000.000000;
     strcpy(newfuels[11].desc, "Light Slash");

     newfuels[12].number=12;
     strcpy(newfuels[12].code, "FM12");
     newfuels[12].h1=4.010000;
     newfuels[12].h10=14.030000;
     newfuels[12].h100=16.530000;
     newfuels[12].lh=0.000000;
     newfuels[12].lw=0.000000;
     newfuels[12].dynamic=0;
     newfuels[12].sav1=1500;
     newfuels[12].savlh=1800;
     newfuels[12].savlw=1500;
     newfuels[12].depth=2.300000;
     newfuels[12].xmext=0.200000;
     newfuels[12].heatd=8000.000000;
     newfuels[12].heatl=8000.000000;
     strcpy(newfuels[12].desc, "Medium Slash");

     newfuels[13].number=13;
     strcpy(newfuels[13].code, "FM13");
     newfuels[13].h1=7.010000;
     newfuels[13].h10=23.040000;
     newfuels[13].h100=28.050000;
     newfuels[13].lh=0.000000;
     newfuels[13].lw=0.000000;
     newfuels[13].dynamic=0;
     newfuels[13].sav1=1500;
     newfuels[13].savlh=1800;
     newfuels[13].savlw=1500;
     newfuels[13].depth=3.000000;
     newfuels[13].xmext=0.250000;
     newfuels[13].heatd=8000.000000;
     newfuels[13].heatl=8000.000000;
     strcpy(newfuels[13].desc, "Heavy Slash");

	newfuels[99].number=99;
     strcpy(newfuels[99].code, "NB9");
     strcpy(newfuels[99].desc, "Barren");

	newfuels[98].number=98;
     strcpy(newfuels[98].code, "NB8");
     strcpy(newfuels[98].desc, "Water");

	newfuels[93].number=93;
     strcpy(newfuels[93].code, "NB3");
     strcpy(newfuels[93].desc, "Snow or Ice");

	newfuels[92].number=92;
     strcpy(newfuels[92].code, "NB2");
     strcpy(newfuels[92].desc, "Agricultural or Cropland");

	newfuels[91].number=91;
     strcpy(newfuels[91].code, "NB1");
     strcpy(newfuels[91].desc, "Urban or Developed");

     newfuels[101].number=101;
     strcpy(newfuels[101].code, "GR1");
     newfuels[101].h1=0.100000;
     newfuels[101].h10=0.000000;
     newfuels[101].h100=0.000000;
     newfuels[101].lh=0.300000;
     newfuels[101].lw=0.000000;
     newfuels[101].dynamic=1;
     newfuels[101].sav1=2200;
     newfuels[101].savlh=2000;
     newfuels[101].savlw=1500;
     newfuels[101].depth=0.400000;
     newfuels[101].xmext=0.150000;
     newfuels[101].heatd=8000.000000;
     newfuels[101].heatl=8000.000000;
     strcpy(newfuels[101].desc, "Short, sparse, dry climate grass");

     newfuels[102].number=102;
     strcpy(newfuels[102].code, "GR2");
     newfuels[102].h1=0.100000;
     newfuels[102].h10=0.000000;
     newfuels[102].h100=0.000000;
     newfuels[102].lh=1.000000;
     newfuels[102].lw=0.000000;
     newfuels[102].dynamic=1;
     newfuels[102].sav1=2000;
     newfuels[102].savlh=1800;
     newfuels[102].savlw=1500;
     newfuels[102].depth=1.000000;
     newfuels[102].xmext=0.150000;
     newfuels[102].heatd=8000.000000;
     newfuels[102].heatl=8000.000000;
     strcpy(newfuels[102].desc, "Low load, dry climate grass");

     newfuels[103].number=103;
     strcpy(newfuels[103].code, "GR3");
     newfuels[103].h1=0.100000;
     newfuels[103].h10=0.400000;
     newfuels[103].h100=0.000000;
     newfuels[103].lh=1.500000;
     newfuels[103].lw=0.000000;
     newfuels[103].dynamic=1;
     newfuels[103].sav1=1500;
     newfuels[103].savlh=1300;
     newfuels[103].savlw=1500;
     newfuels[103].depth=2.000000;
     newfuels[103].xmext=0.300000;
     newfuels[103].heatd=8000.000000;
     newfuels[103].heatl=8000.000000;
     strcpy(newfuels[103].desc, "Low load, very coarse, humid climate grass");

     newfuels[104].number=104;
     strcpy(newfuels[104].code, "GR4");
     newfuels[104].h1=0.250000;
     newfuels[104].h10=0.000000;
     newfuels[104].h100=0.000000;
     newfuels[104].lh=1.900000;
     newfuels[104].lw=0.000000;
     newfuels[104].dynamic=1;
     newfuels[104].sav1=2000;
     newfuels[104].savlh=1800;
     newfuels[104].savlw=1500;
     newfuels[104].depth=2.000000;
     newfuels[104].xmext=0.150000;
     newfuels[104].heatd=8000.000000;
     newfuels[104].heatl=8000.000000;
     strcpy(newfuels[104].desc, "Moderate load, dry climate grass");

     newfuels[105].number=105;
     strcpy(newfuels[105].code, "GR5");
     newfuels[105].h1=0.400000;
     newfuels[105].h10=0.000000;
     newfuels[105].h100=0.000000;
     newfuels[105].lh=2.500000;
     newfuels[105].lw=0.000000;
     newfuels[105].dynamic=1;
     newfuels[105].sav1=1800;
     newfuels[105].savlh=1600;
     newfuels[105].savlw=1500;
     newfuels[105].depth=1.500000;
     newfuels[105].xmext=0.400000;
     newfuels[105].heatd=8000.000000;
     newfuels[105].heatl=8000.000000;
     strcpy(newfuels[105].desc, "Low load, humid climate grass");

     newfuels[106].number=106;
     strcpy(newfuels[106].code, "GR6");
     newfuels[106].h1=0.100000;
     newfuels[106].h10=0.000000;
     newfuels[106].h100=0.000000;
     newfuels[106].lh=3.400000;
     newfuels[106].lw=0.000000;
     newfuels[106].dynamic=1;
     newfuels[106].sav1=2200;
     newfuels[106].savlh=2000;
     newfuels[106].savlw=1500;
     newfuels[106].depth=1.500000;
     newfuels[106].xmext=0.400000;
     newfuels[106].heatd=9000.000000;
     newfuels[106].heatl=9000.000000;
     strcpy(newfuels[106].desc, "Moderate load, humid climate grass");

     newfuels[107].number=107;
     strcpy(newfuels[107].code, "GR7");
     newfuels[107].h1=1.000000;
     newfuels[107].h10=0.000000;
     newfuels[107].h100=0.000000;
     newfuels[107].lh=5.400000;
     newfuels[107].lw=0.000000;
     newfuels[107].dynamic=1;
     newfuels[107].sav1=2000;
     newfuels[107].savlh=1800;
     newfuels[107].savlw=1500;
     newfuels[107].depth=3.000000;
     newfuels[107].xmext=0.150000;
     newfuels[107].heatd=8000.000000;
     newfuels[107].heatl=8000.000000;
     strcpy(newfuels[107].desc, "High load, dry climate grass");

     newfuels[108].number=108;
     strcpy(newfuels[108].code, "GR8");
     newfuels[108].h1=0.500000;
     newfuels[108].h10=1.000000;
     newfuels[108].h100=0.000000;
     newfuels[108].lh=7.300000;
     newfuels[108].lw=0.000000;
     newfuels[108].dynamic=1;
     newfuels[108].sav1=1500;
     newfuels[108].savlh=1300;
     newfuels[108].savlw=1500;
     newfuels[108].depth=4.000000;
     newfuels[108].xmext=0.300000;
     newfuels[108].heatd=8000.000000;
     newfuels[108].heatl=8000.000000;
     strcpy(newfuels[108].desc, "High load, very coarse, humid climate grass");

     newfuels[109].number=109;
     strcpy(newfuels[109].code, "GR9");
     newfuels[109].h1=1.000000;
     newfuels[109].h10=1.000000;
     newfuels[109].h100=0.000000;
     newfuels[109].lh=9.000000;
     newfuels[109].lw=0.000000;
     newfuels[109].dynamic=1;
     newfuels[109].sav1=1800;
     newfuels[109].savlh=1600;
     newfuels[109].savlw=1500;
     newfuels[109].depth=5.000000;
     newfuels[109].xmext=0.400000;
     newfuels[109].heatd=8000.000000;
     newfuels[109].heatl=8000.000000;
     strcpy(newfuels[109].desc, "Very high load, humid climate grass");

     newfuels[121].number=121;
     strcpy(newfuels[121].code, "GS1");
     newfuels[121].h1=0.200000;
     newfuels[121].h10=0.000000;
     newfuels[121].h100=0.000000;
     newfuels[121].lh=0.500000;
     newfuels[121].lw=0.650000;
     newfuels[121].dynamic=1;
     newfuels[121].sav1=2000;
     newfuels[121].savlh=1800;
     newfuels[121].savlw=1800;
     newfuels[121].depth=0.900000;
     newfuels[121].xmext=0.150000;
     newfuels[121].heatd=8000.000000;
     newfuels[121].heatl=8000.000000;
     strcpy(newfuels[121].desc, "Low load, dry climate grass-shrub");

     newfuels[122].number=122;
     strcpy(newfuels[122].code, "GS2");
     newfuels[122].h1=0.500000;
     newfuels[122].h10=0.500000;
     newfuels[122].h100=0.000000;
     newfuels[122].lh=0.600000;
     newfuels[122].lw=1.000000;
     newfuels[122].dynamic=1;
     newfuels[122].sav1=2000;
     newfuels[122].savlh=1800;
     newfuels[122].savlw=1800;
     newfuels[122].depth=1.500000;
     newfuels[122].xmext=0.150000;
     newfuels[122].heatd=8000.000000;
     newfuels[122].heatl=8000.000000;
     strcpy(newfuels[122].desc, "Moderate load, dry climate grass-shrub");

     newfuels[123].number=123;
     strcpy(newfuels[123].code, "GS3");
     newfuels[123].h1=0.300000;
     newfuels[123].h10=0.250000;
     newfuels[123].h100=0.000000;
     newfuels[123].lh=1.450000;
     newfuels[123].lw=1.250000;
     newfuels[123].dynamic=1;
     newfuels[123].sav1=1800;
     newfuels[123].savlh=1600;
     newfuels[123].savlw=1600;
     newfuels[123].depth=1.800000;
     newfuels[123].xmext=0.400000;
     newfuels[123].heatd=8000.000000;
     newfuels[123].heatl=8000.000000;
     strcpy(newfuels[123].desc, "Moderate load, humid climate grass-shrub");

     newfuels[124].number=124;
     strcpy(newfuels[124].code, "GS4");
     newfuels[124].h1=1.900000;
     newfuels[124].h10=0.300000;
     newfuels[124].h100=0.100000;
     newfuels[124].lh=3.400000;
     newfuels[124].lw=7.100000;
     newfuels[124].dynamic=1;
     newfuels[124].sav1=1800;
     newfuels[124].savlh=1600;
     newfuels[124].savlw=1600;
     newfuels[124].depth=2.100000;
     newfuels[124].xmext=0.400000;
     newfuels[124].heatd=8000.000000;
     newfuels[124].heatl=8000.000000;
     strcpy(newfuels[124].desc, "High load, humid climate grass-shrub");

     newfuels[141].number=141;
     strcpy(newfuels[141].code, "SH1");
     newfuels[141].h1=0.250000;
     newfuels[141].h10=0.250000;
     newfuels[141].h100=0.000000;
     newfuels[141].lh=0.150000;
     newfuels[141].lw=1.300000;
     newfuels[141].dynamic=1;
     newfuels[141].sav1=2000;
     newfuels[141].savlh=1800;
     newfuels[141].savlw=1600;
     newfuels[141].depth=1.000000;
     newfuels[141].xmext=0.150000;
     newfuels[141].heatd=8000.000000;
     newfuels[141].heatl=8000.000000;
     strcpy(newfuels[141].desc, "Low load, dry climate shrub");

     newfuels[142].number=142;
     strcpy(newfuels[142].code, "SH2");
     newfuels[142].h1=1.350000;
     newfuels[142].h10=2.400000;
     newfuels[142].h100=0.750000;
     newfuels[142].lh=0.000000;
     newfuels[142].lw=3.850000;
     newfuels[142].dynamic=0;
     newfuels[142].sav1=2000;
     newfuels[142].savlh=1800;
     newfuels[142].savlw=1600;
     newfuels[142].depth=1.000000;
     newfuels[142].xmext=0.150000;
     newfuels[142].heatd=8000.000000;
     newfuels[142].heatl=8000.000000;
     strcpy(newfuels[142].desc, "Moderate load, dry climate shrub");

     newfuels[143].number=143;
     strcpy(newfuels[143].code, "SH3");
     newfuels[143].h1=0.450000;
     newfuels[143].h10=3.000000;
     newfuels[143].h100=0.000000;
     newfuels[143].lh=0.000000;
     newfuels[143].lw=6.200000;
     newfuels[143].dynamic=0;
     newfuels[143].sav1=1600;
     newfuels[143].savlh=1800;
     newfuels[143].savlw=1400;
     newfuels[143].depth=2.400000;
     newfuels[143].xmext=0.400000;
     newfuels[143].heatd=8000.000000;
     newfuels[143].heatl=8000.000000;
     strcpy(newfuels[143].desc, "Moderate load, humid climate shrub");

     newfuels[144].number=144;
     strcpy(newfuels[144].code, "SH4");
     newfuels[144].h1=0.850000;
     newfuels[144].h10=1.150000;
     newfuels[144].h100=0.200000;
     newfuels[144].lh=0.000000;
     newfuels[144].lw=2.550000;
     newfuels[144].dynamic=0;
     newfuels[144].sav1=2000;
     newfuels[144].savlh=1800;
     newfuels[144].savlw=1600;
     newfuels[144].depth=3.000000;
     newfuels[144].xmext=0.300000;
     newfuels[144].heatd=8000.000000;
     newfuels[144].heatl=8000.000000;
     strcpy(newfuels[144].desc, "Low load, humid climate timber-shrub");

     newfuels[145].number=145;
     strcpy(newfuels[145].code, "SH5");
     newfuels[145].h1=3.600000;
     newfuels[145].h10=2.100000;
     newfuels[145].h100=0.000000;
     newfuels[145].lh=0.000000;
     newfuels[145].lw=2.900000;
     newfuels[145].dynamic=0;
     newfuels[145].sav1=750;
     newfuels[145].savlh=1800;
     newfuels[145].savlw=1600;
     newfuels[145].depth=6.000000;
     newfuels[145].xmext=0.150000;
     newfuels[145].heatd=8000.000000;
     newfuels[145].heatl=8000.000000;
     strcpy(newfuels[145].desc, "High load, dry climate shrub");

     newfuels[146].number=146;
     strcpy(newfuels[146].code, "SH6");
     newfuels[146].h1=2.900000;
     newfuels[146].h10=1.450000;
     newfuels[146].h100=0.000000;
     newfuels[146].lh=0.000000;
     newfuels[146].lw=1.400000;
     newfuels[146].dynamic=0;
     newfuels[146].sav1=750;
     newfuels[146].savlh=1800;
     newfuels[146].savlw=1600;
     newfuels[146].depth=2.000000;
     newfuels[146].xmext=0.300000;
     newfuels[146].heatd=8000.000000;
     newfuels[146].heatl=8000.000000;
     strcpy(newfuels[146].desc, "Low load, humid climate shrub");

     newfuels[147].number=147;
     strcpy(newfuels[147].code, "SH7");
     newfuels[147].h1=3.500000;
     newfuels[147].h10=5.300000;
     newfuels[147].h100=2.200000;
     newfuels[147].lh=0.000000;
     newfuels[147].lw=3.400000;
     newfuels[147].dynamic=0;
     newfuels[147].sav1=750;
     newfuels[147].savlh=1800;
     newfuels[147].savlw=1600;
     newfuels[147].depth=6.000000;
     newfuels[147].xmext=0.150000;
     newfuels[147].heatd=8000.000000;
     newfuels[147].heatl=8000.000000;
     strcpy(newfuels[147].desc, "Very high load, dry climate shrub");

     newfuels[148].number=148;
     strcpy(newfuels[148].code, "SH8");
     newfuels[148].h1=2.050000;
     newfuels[148].h10=3.400000;
     newfuels[148].h100=0.850000;
     newfuels[148].lh=0.000000;
     newfuels[148].lw=4.350000;
     newfuels[148].dynamic=0;
     newfuels[148].sav1=750;
     newfuels[148].savlh=1800;
     newfuels[148].savlw=1600;
     newfuels[148].depth=3.000000;
     newfuels[148].xmext=0.400000;
     newfuels[148].heatd=8000.000000;
     newfuels[148].heatl=8000.000000;
     strcpy(newfuels[148].desc, "High load, humid climate shrub");

     newfuels[149].number=149;
     strcpy(newfuels[149].code, "SH9");
     newfuels[149].h1=4.500000;
     newfuels[149].h10=2.450000;
     newfuels[149].h100=0.000000;
     newfuels[149].lh=1.550000;
     newfuels[149].lw=7.000000;
     newfuels[149].dynamic=1;
     newfuels[149].sav1=750;
     newfuels[149].savlh=1800;
     newfuels[149].savlw=1500;
     newfuels[149].depth=4.400000;
     newfuels[149].xmext=0.400000;
     newfuels[149].heatd=8000.000000;
     newfuels[149].heatl=8000.000000;
     strcpy(newfuels[149].desc, "Very high load, humid climate shrub");

     newfuels[161].number=161;
     strcpy(newfuels[161].code, "TU1");
     newfuels[161].h1=0.200000;
     newfuels[161].h10=0.900000;
     newfuels[161].h100=1.500000;
     newfuels[161].lh=0.200000;
     newfuels[161].lw=0.900000;
     newfuels[161].dynamic=1;
     newfuels[161].sav1=2000;
     newfuels[161].savlh=1800;
     newfuels[161].savlw=1600;
     newfuels[161].depth=0.600000;
     newfuels[161].xmext=0.200000;
     newfuels[161].heatd=8000.000000;
     newfuels[161].heatl=8000.000000;
     strcpy(newfuels[161].desc, "Light load, dry climate timber-grass-shrub");

     newfuels[162].number=162;
     strcpy(newfuels[162].code, "TU2");
     newfuels[162].h1=0.950000;
     newfuels[162].h10=1.800000;
     newfuels[162].h100=1.250000;
     newfuels[162].lh=0.000000;
     newfuels[162].lw=0.200000;
     newfuels[162].dynamic=0;
     newfuels[162].sav1=2000;
     newfuels[162].savlh=1800;
     newfuels[162].savlw=1600;
     newfuels[162].depth=1.000000;
     newfuels[162].xmext=0.300000;
     newfuels[162].heatd=8000.000000;
     newfuels[162].heatl=8000.000000;
     strcpy(newfuels[162].desc, "Moderate load, humid climate timber-shrub");

     newfuels[163].number=163;
     strcpy(newfuels[163].code, "TU3");
     newfuels[163].h1=1.100000;
     newfuels[163].h10=0.150000;
     newfuels[163].h100=0.250000;
     newfuels[163].lh=0.650000;
     newfuels[163].lw=1.100000;
     newfuels[163].dynamic=1;
     newfuels[163].sav1=1800;
     newfuels[163].savlh=1600;
     newfuels[163].savlw=1400;
     newfuels[163].depth=1.300000;
     newfuels[163].xmext=0.300000;
     newfuels[163].heatd=8000.000000;
     newfuels[163].heatl=8000.000000;
     strcpy(newfuels[163].desc, "Moderate load, humid climate timber-grass-shrub");

     newfuels[164].number=164;
     strcpy(newfuels[164].code, "TU4");
     newfuels[164].h1=4.500000;
     newfuels[164].h10=0.000000;
     newfuels[164].h100=0.000000;
     newfuels[164].lh=0.000000;
     newfuels[164].lw=2.000000;
     newfuels[164].dynamic=0;
     newfuels[164].sav1=2300;
     newfuels[164].savlh=1800;
     newfuels[164].savlw=2000;
     newfuels[164].depth=0.500000;
     newfuels[164].xmext=0.120000;
     newfuels[164].heatd=8000.000000;
     newfuels[164].heatl=8000.000000;
     strcpy(newfuels[164].desc, "Dwarf conifer with understory");

     newfuels[165].number=165;
     strcpy(newfuels[165].code, "TU5");
     newfuels[165].h1=4.000000;
     newfuels[165].h10=4.000000;
     newfuels[165].h100=3.000000;
     newfuels[165].lh=0.000000;
     newfuels[165].lw=3.000000;
     newfuels[165].dynamic=0;
     newfuels[165].sav1=1500;
     newfuels[165].savlh=1800;
     newfuels[165].savlw=750;
     newfuels[165].depth=1.000000;
     newfuels[165].xmext=0.250000;
     newfuels[165].heatd=8000.000000;
     newfuels[165].heatl=8000.000000;
     strcpy(newfuels[165].desc, "Very high load, dry climate timber-shrub");

     newfuels[181].number=181;
     strcpy(newfuels[181].code, "TL1");
     newfuels[181].h1=1.000000;
     newfuels[181].h10=2.200000;
     newfuels[181].h100=3.600000;
     newfuels[181].lh=0.000000;
     newfuels[181].lw=0.000000;
     newfuels[181].dynamic=0;
     newfuels[181].sav1=2000;
     newfuels[181].savlh=1800;
     newfuels[181].savlw=1600;
     newfuels[181].depth=0.200000;
     newfuels[181].xmext=0.300000;
     newfuels[181].heatd=8000.000000;
     newfuels[181].heatl=8000.000000;
     strcpy(newfuels[181].desc, "Low load, compact conifer litter");

     newfuels[182].number=182;
     strcpy(newfuels[182].code, "TL2");
     newfuels[182].h1=1.400000;
     newfuels[182].h10=2.300000;
     newfuels[182].h100=2.200000;
     newfuels[182].lh=0.000000;
     newfuels[182].lw=0.000000;
     newfuels[182].dynamic=0;
     newfuels[182].sav1=2000;
     newfuels[182].savlh=1800;
     newfuels[182].savlw=1600;
     newfuels[182].depth=0.200000;
     newfuels[182].xmext=0.250000;
     newfuels[182].heatd=8000.000000;
     newfuels[182].heatl=8000.000000;
     strcpy(newfuels[182].desc, "Low load broadleaf litter");

     newfuels[183].number=183;
     strcpy(newfuels[183].code, "TL3");
     newfuels[183].h1=0.500000;
     newfuels[183].h10=2.200000;
     newfuels[183].h100=2.800000;
     newfuels[183].lh=0.000000;
     newfuels[183].lw=0.000000;
     newfuels[183].dynamic=0;
     newfuels[183].sav1=2000;
     newfuels[183].savlh=1800;
     newfuels[183].savlw=1600;
     newfuels[183].depth=0.300000;
     newfuels[183].xmext=0.200000;
     newfuels[183].heatd=8000.000000;
     newfuels[183].heatl=8000.000000;
     strcpy(newfuels[183].desc, "Moderate load confier litter");

     newfuels[184].number=184;
     strcpy(newfuels[184].code, "TL4");
     newfuels[184].h1=0.500000;
     newfuels[184].h10=1.500000;
     newfuels[184].h100=4.200000;
     newfuels[184].lh=0.000000;
     newfuels[184].lw=0.000000;
     newfuels[184].dynamic=0;
     newfuels[184].sav1=2000;
     newfuels[184].savlh=1800;
     newfuels[184].savlw=1600;
     newfuels[184].depth=0.400000;
     newfuels[184].xmext=0.250000;
     newfuels[184].heatd=8000.000000;
     newfuels[184].heatl=8000.000000;
     strcpy(newfuels[184].desc, "Small downed logs");

     newfuels[185].number=185;
     strcpy(newfuels[185].code, "TL5");
     newfuels[185].h1=1.150000;
     newfuels[185].h10=2.500000;
     newfuels[185].h100=4.400000;
     newfuels[185].lh=0.000000;
     newfuels[185].lw=0.000000;
     newfuels[185].dynamic=0;
     newfuels[185].sav1=2000;
     newfuels[185].savlh=1800;
     newfuels[185].savlw=1600;
     newfuels[185].depth=0.600000;
     newfuels[185].xmext=0.250000;
     newfuels[185].heatd=8000.000000;
     newfuels[185].heatl=8000.000000;
     strcpy(newfuels[185].desc, "High load conifer litter");

     newfuels[186].number=186;
     strcpy(newfuels[186].code, "TL6");
     newfuels[186].h1=2.400000;
     newfuels[186].h10=1.200000;
     newfuels[186].h100=1.200000;
     newfuels[186].lh=0.000000;
     newfuels[186].lw=0.000000;
     newfuels[186].dynamic=0;
     newfuels[186].sav1=2000;
     newfuels[186].savlh=1800;
     newfuels[186].savlw=1600;
     newfuels[186].depth=0.300000;
     newfuels[186].xmext=0.250000;
     newfuels[186].heatd=8000.000000;
     newfuels[186].heatl=8000.000000;
     strcpy(newfuels[186].desc, "High load broadleaf litter");

     newfuels[187].number=187;
     strcpy(newfuels[187].code, "TL7");
     newfuels[187].h1=0.300000;
     newfuels[187].h10=1.400000;
     newfuels[187].h100=8.100000;
     newfuels[187].lh=0.000000;
     newfuels[187].lw=0.000000;
     newfuels[187].dynamic=0;
     newfuels[187].sav1=2000;
     newfuels[187].savlh=1800;
     newfuels[187].savlw=1600;
     newfuels[187].depth=0.400000;
     newfuels[187].xmext=0.250000;
     newfuels[187].heatd=8000.000000;
     newfuels[187].heatl=8000.000000;
     strcpy(newfuels[187].desc, "Large downed logs");

     newfuels[188].number=188;
     strcpy(newfuels[188].code, "TL8");
     newfuels[188].h1=5.800000;
     newfuels[188].h10=1.400000;
     newfuels[188].h100=1.100000;
     newfuels[188].lh=0.000000;
     newfuels[188].lw=0.000000;
     newfuels[188].dynamic=0;
     newfuels[188].sav1=1800;
     newfuels[188].savlh=1800;
     newfuels[188].savlw=1600;
     newfuels[188].depth=0.300000;
     newfuels[188].xmext=0.350000;
     newfuels[188].heatd=8000.000000;
     newfuels[188].heatl=8000.000000;
     strcpy(newfuels[188].desc, "Long-needle litter");

     newfuels[189].number=189;
     strcpy(newfuels[189].code, "TL9");
     newfuels[189].h1=6.650000;
     newfuels[189].h10=3.300000;
     newfuels[189].h100=4.150000;
     newfuels[189].lh=0.000000;
     newfuels[189].lw=0.000000;
     newfuels[189].dynamic=0;
     newfuels[189].sav1=1800;
     newfuels[189].savlh=1800;
     newfuels[189].savlw=1600;
     newfuels[189].depth=0.600000;
     newfuels[189].xmext=0.350000;
     newfuels[189].heatd=8000.000000;
     newfuels[189].heatl=8000.000000;
     strcpy(newfuels[189].desc, "Very high load broadleaf litter");

     newfuels[201].number=201;
     strcpy(newfuels[201].code, "SB1");
     newfuels[201].h1=1.500000;
     newfuels[201].h10=3.000000;
     newfuels[201].h100=11.000000;
     newfuels[201].lh=0.000000;
     newfuels[201].lw=0.000000;
     newfuels[201].dynamic=0;
     newfuels[201].sav1=2000;
     newfuels[201].savlh=1800;
     newfuels[201].savlw=1600;
     newfuels[201].depth=1.000000;
     newfuels[201].xmext=0.250000;
     newfuels[201].heatd=8000.000000;
     newfuels[201].heatl=8000.000000;
     strcpy(newfuels[201].desc, "Low load activity fuel");

     newfuels[202].number=202;
     strcpy(newfuels[202].code, "SB2");
     newfuels[202].h1=4.500000;
     newfuels[202].h10=4.250000;
     newfuels[202].h100=4.000000;
     newfuels[202].lh=0.000000;
     newfuels[202].lw=0.000000;
     newfuels[202].dynamic=0;
     newfuels[202].sav1=2000;
     newfuels[202].savlh=1800;
     newfuels[202].savlw=1600;
     newfuels[202].depth=1.000000;
     newfuels[202].xmext=0.250000;
     newfuels[202].heatd=8000.000000;
     newfuels[202].heatl=8000.000000;
     strcpy(newfuels[202].desc, "Moderate load activity or low load blowdown");

     newfuels[203].number=203;
     strcpy(newfuels[203].code, "SB3");
     newfuels[203].h1=5.500000;
     newfuels[203].h10=2.750000;
     newfuels[203].h100=3.000000;
     newfuels[203].lh=0.000000;
     newfuels[203].lw=0.000000;
     newfuels[203].dynamic=0;
     newfuels[203].sav1=2000;
     newfuels[203].savlh=1800;
     newfuels[203].savlw=1600;
     newfuels[203].depth=1.200000;
     newfuels[203].xmext=0.250000;
     newfuels[203].heatd=8000.000000;
     newfuels[203].heatl=8000.000000;
     strcpy(newfuels[203].desc, "High load activity fuel or moderate load blowdown");

     newfuels[204].number=204;
     strcpy(newfuels[204].code, "SB4");
     newfuels[204].h1=5.250000;
     newfuels[204].h10=3.500000;
     newfuels[204].h100=5.250000;
     newfuels[204].lh=0.000000;
     newfuels[204].lw=0.000000;
     newfuels[204].dynamic=0;
     newfuels[204].sav1=2000;
     newfuels[204].savlh=1800;
     newfuels[204].savlw=1600;
     newfuels[204].depth=2.700000;
     newfuels[204].xmext=0.250000;
     newfuels[204].heatd=8000.000000;
     newfuels[204].heatl=8000.000000;
     strcpy(newfuels[204].desc, "High load blowdown");

}


#ifdef wowerwer

int FlamMap::GetOpenWeatherStation()
{
	int i;

	for(i=0; i<5; i++)
	{	if(!wtrdt[i])
		break;
	}

	return i;
}



int FlamMap::GetNumStations()
{
	if(NumWeatherStations>NumWindStations)
		return NumWindStations;

	return NumWeatherStations;
}




int FlamMap::AllocWindData(int StatNum, int NumObs)
{
	int StationNumber=StatNum;

     if(wddt[StationNumber])
	{	delete[] wddt[StationNumber];
		wddt[StationNumber]=0;
          MaxWindObs[StatNum]=0;
	}
	else
	{	StationNumber=GetOpenWindStation();
		if(StationNumber<5)
			NumWindStations=StationNumber+1;
	}
	if(StationNumber<5 && NumObs>0)
	{	size_t nmemb=MaxWindObs[StationNumber]=NumObs*2;			// alloc 2* number needed
		if((wddt[StationNumber]=new WindData[nmemb])==NULL)
			StationNumber=-1;
	}
	else
		StationNumber=-1;

	return StationNumber;
}
int FlamMap::GetOpenWindStation()
{
     int i;

	for(i=0; i<5; i++)
	{	if(!wddt[i])
		break;
	}

	return i;
}



int FlamMap::SetWindData(int StationNumber, int NumObs, int month, int day,
			  int hour, double windspd, int winddir, int cloudy)
{
	if(NumObs<MaxWindObs[StationNumber])
	{	wddt[StationNumber][NumObs].mo=month;
		wddt[StationNumber][NumObs].dy=day;
		wddt[StationNumber][NumObs].hr=hour;
		wddt[StationNumber][NumObs].ws=windspd;
		wddt[StationNumber][NumObs].wd=winddir;
		wddt[StationNumber][NumObs].cl=cloudy;

		if(month==13)
		{    FirstMonth[StationNumber].wnd=wddt[StationNumber][0].mo;
			LastMonth[StationNumber].wnd=wddt[StationNumber][NumObs-1].mo;
			FirstDay[StationNumber].wnd=wddt[StationNumber][0].dy;
			LastDay[StationNumber].wnd=wddt[StationNumber][NumObs-1].dy;
			FirstHour[StationNumber].wnd=wddt[StationNumber][0].hr;
			LastHour[StationNumber].wnd=wddt[StationNumber][NumObs-1].hr;
		}

		return 1;
	}

	return 0;
}


int FlamMap::GetWindMonth(int StationNumber, int NumObs)
{
     if(NumObs>MaxWindObs[StationNumber]-1)
     	NumObs=MaxWindObs[StationNumber]-1;

	return wddt[StationNumber][NumObs].mo;
}

int FlamMap::GetWindDay(int StationNumber, int NumObs)
{

     if(NumObs>MaxWindObs[StationNumber]-1)
     	NumObs=MaxWindObs[StationNumber]-1;

	return wddt[StationNumber][NumObs].dy;
}

int FlamMap::GetWindHour(int StationNumber, int NumObs)
{
     if(NumObs>MaxWindObs[StationNumber]-1)
     	NumObs=MaxWindObs[StationNumber]-1;

	return wddt[StationNumber][NumObs].hr;
}


int FlamMap::GetMaxWindObs(int StationNumber)
{
	if(StationNumber>4)
     	return -1;

     return MaxWindObs[StationNumber]-1;
}

 

bool FlamMap::IndividualFuelMoisturesSet(void)
{
	bool moisturesOK = true;
	for(int f = 0; f <= Header.numfuel; f++)
	{
		if(Header.fuels[f] > 0 && Header.fuels[f] <= 256 
			&& (Header.fuels[f] < 90 || Header.fuels[f] > 99)
			&& !fm[Header.fuels[f]].FuelMoistureIsHere)
		{
			printf("Error: Moistures not set for fuel %d\n", Header.fuels[f]);
			moisturesOK = false;
		}
	}
	return moisturesOK;
}
#endif


int FlamMap::WoodyCombineOptions(int Option)
{
	if(Option>0)
     	CombineOption=Option;

     return CombineOption;
}









bool FlamMap::UseConditioningPeriod(int YesNo)
{
	if(YesNo>-1)
          CondPeriod=(bool) YesNo;

	return CondPeriod;
}




double FlamMap::GetWoodyFuelMoisture(int ModelNumber, int SizeClass)
{
     if(ModelNumber>MAXNUM_COARSEWOODY_MODELS)
     	return 0.0;

	if(coarsewoody[ModelNumber-1].NumClasses<SizeClass)
     	return 0.0;

     return coarsewoody[ModelNumber-1].wd[SizeClass].FuelMoisture;
}

void FlamMap::GetWoodyData(int WoodyModelNumber, int SurfModelNumber, int *NumClasses, WoodyData *woody, double *depth, double *load)
{
     bool Combine=false;

     *NumClasses=0;
     *load=0.0;


     switch(WoodyCombineOptions(GETVAL))
     {	case CWD_COMBINE_NEVER: Combine=false; break;
     	case CWD_COMBINE_ALWAYS: Combine=true; break;
     	case CWD_COMBINE_ABSENT:
               if(WoodyModelNumber<1)
               	Combine=true;
               else if(coarsewoody[WoodyModelNumber-1].wd==0)
               	Combine=true;
               else
               	Combine=false;
               break;
     }

     if(Combine)
     {
	// gather surface fuel model data
	if(SurfModelNumber>0 && SurfModelNumber<14)                                                         // alloc the max
	{    *NumClasses=NFFLWoody[SurfModelNumber-1].NumClasses;//tempwoody.NumClasses=
     	*depth=NFFLWoody[SurfModelNumber-1].Depth; //tempwoody.Depth=
          *load=NFFLWoody[SurfModelNumber-1].TotalWeight;//tempwoody.TotalWeight=
     	memcpy(woody, NFFLWoody[SurfModelNumber-1].wd, NFFLWoody[SurfModelNumber-1].NumClasses*sizeof(WoodyData));
     }
	else if(SurfModelNumber>0)
     {	//double t1, t10, t100, tLH, tLW, s1, sLH, sLW, hd, hl, d, xm;
     	int i=0, j;
          NewFuel nf;
	     memset(&nf, 0x0, sizeof(NewFuel));

		GetNewFuel(SurfModelNumber, &nf);
     	*depth=nf.depth;
          *load=0.0;
          if(nf.h1>0.0)
          {	woody[i].Load=nf.h1;//*0.224169061;
          	woody[i].SurfaceAreaToVolume=nf.sav1;//*3.280839895;
               woody[i++].HeatContent=nf.heatd;//*2.32599;
          }
          if(nf.h10>0.0)
          {	woody[i].Load=nf.h10;
          	woody[i].SurfaceAreaToVolume=109.0;//*3.280839895;
               woody[i++].HeatContent=nf.heatd;//*2.32599;
          }
          if(nf.h100>0.0)
          {	woody[i].Load=nf.h100;
          	woody[i].SurfaceAreaToVolume=30;//*3.280839895;
               woody[i++].HeatContent=nf.heatd;//*2.32599;
          }
          if(nf.lw>0.0)
          {	woody[i].Load=nf.lw;
          	woody[i].SurfaceAreaToVolume=nf.savlw;//*3.280839895;
               woody[i++].HeatContent=nf.heatl;//*2.32599;
          }
          if(nf.lh>0.0)
          {	woody[i].Load=nf.lh;
          	woody[i].SurfaceAreaToVolume=nf.savlh;//*3.280839895;
               woody[i++].HeatContent=nf.heatl;//*2.32599;
          }
          for(j=0; j<i; j++)
          {    woody[j].Density=513.0;
               *load+=woody[j].Load;
          }
          *NumClasses=tempwoody.NumClasses=i;
     }
     }
// patch into coarsewoody model data if present
	if(WoodyModelNumber>0)
     {    if(coarsewoody[WoodyModelNumber-1].wd)
	     {    memcpy(&woody[*NumClasses], coarsewoody[WoodyModelNumber-1].wd, coarsewoody[WoodyModelNumber-1].NumClasses*sizeof(WoodyData));
			*NumClasses+=coarsewoody[WoodyModelNumber-1].NumClasses;
	     	*depth=coarsewoody[WoodyModelNumber-1].Depth;
		     *load+=coarsewoody[WoodyModelNumber-1].TotalWeight;
     	}
     }
}

bool FlamMap::AllocCoarseWoody(int ModelNumber, int NumClasses)
{
     if(coarsewoody[ModelNumber-1].wd)
     	FreeCoarseWoody(ModelNumber);
	if((coarsewoody[ModelNumber-1].wd=new WoodyData[NumClasses])==NULL) //(WoodyData *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, NumClasses*sizeof(WoodyData)))==NULL)
     	return false;
	memset(coarsewoody[ModelNumber-1].wd,0x0, NumClasses*sizeof(WoodyData));
				//	memset(landscape, 0x0, allOfIt*sizeof(short));
	coarsewoody[ModelNumber-1].NumClasses=NumClasses;
     coarsewoody[ModelNumber-1].TotalWeight=0.0;
     memset(coarsewoody[ModelNumber-1].Description,0x0, 64*sizeof(char));

	return true;
}


void FlamMap::FreeCoarseWoody(int ModelNumber)
{
	if(coarsewoody[ModelNumber-1].wd)
     	delete[] coarsewoody[ModelNumber-1].wd;//GlobalFree(coarsewoody[ModelNumber-1].wd);//free(coarsewoody[ModelNumber-1].wd);
    	coarsewoody[ModelNumber-1].NumClasses=0;
     coarsewoody[ModelNumber-1].wd=0;
     coarsewoody[ModelNumber-1].Units=-1;
     coarsewoody[ModelNumber-1].TotalWeight=0.0;
     coarsewoody[ModelNumber-1].Depth=0.0;
}

void FlamMap::FreeAllCoarseWoody()
{
	int i;

     for(i=1; i<MAXNUM_COARSEWOODY_MODELS+1; i++)
     	FreeCoarseWoody(i);
     for(i=0; i<13; i++)
     {    if(NFFLWoody[i].wd)
	     	delete[] NFFLWoody[i].wd;//GlobalFree(NFFLWoody[i].wd);//free(NFFLWoody[i].wd);
          NFFLWoody[i].NumClasses=0;
     	NFFLWoody[i].wd=0;
     }
     if(tempwoody.wd)
     	delete[] tempwoody.wd;//GlobalFree(tempwoody.wd);//free(tempwoody.wd);
}

bool FlamMap::SetWoodyDataDepth(int ModelNumber, double depth, char *Description)
{
     if(ModelNumber>MAXNUM_COARSEWOODY_MODELS-1)
     	return false;

     coarsewoody[ModelNumber-1].Depth=depth;
     memcpy(coarsewoody[ModelNumber-1].Description, Description, 64*sizeof(char));

	return true;
}

double FlamMap::WeightLossErrorTolerance(double value)
{
	if(value>0.0)
	    WeightLossErrorTol=value;

    	return WeightLossErrorTol;
}


int FlamMap::GetOutputDirection()
{
	return OutputDirection;
}

void FlamMap::SetOutputDirection(int dir)
{
	OutputDirection=dir;
}

/***********************************************************
* Name: SetSpreadDirection
* Desc: set the spread direction from inputs file.
*       value comes in from input file switch 
*       SPREAD_DIRECTION_FROM_ (MAX or NORTH)
*       Inputs should have been validate when read in. 
*   In: North >= 0 set Absolute. of if Max >= 0 set Relative
*       Set both params to negative to do nothing and set to
*       defualt - see below         
***********************************************************/
void FlamMap::SetSpreadDirection (double North, double Max)
{
double d;

  if ( North >= 0 ) {   
    this->SetOutputDirection ( ABSOLUTEDIR ); 
    d = North;
    if ( d == 0 )
      d = 360.0;
    this->SetOffsetFromMax(d); }
  
  else if ( Max >= 0 ) {
    this->SetOutputDirection (RELATIVEDIR );
    this->SetOffsetFromMax(Max); }
 
 else {
    this->SetOutputDirection (RELATIVEDIR );   /* Default */
    this->SetOffsetFromMax(0); } 

}



/********************************************************************************/
void FlamMap::SetThreadProgress(int ThreadNum, double Fract)
{
   	if ( ThreadNum < 64 )
     	ThreadProgress[ThreadNum] = Fract;
}


double *FlamMap::GetThreadProgress()
{
	return ThreadProgress;
}

void FlamMap::ResetProgress()
{
	for (int i = 0; i < 64; i++)
	  ThreadProgress[i] = 0.0;
}

double FlamMap::GetFuelDepth(int Number)
{
	if(Number<0)
     	return 0.0;

	if(newfuels[Number].number==0)
     	return 0.0;

	return newfuels[Number].depth;
}


void FlamMap::SetConstWind(double Speed, double Dir)
{
	ConstWindSpeed=Speed;
     ConstWindDirection=Dir;
}

double FlamMap::GetConstWindSpeed()
{
 	return ConstWindSpeed;
}

double FlamMap::GetConstWindDir()
{
	return ConstWindDirection;
}

bool FlamMap::IsConstWinds()
{
     if(ConstWindSpeed<0.0)
     	return false;

	return true;
}
void FlamMap::SetOffsetFromMax(double degrees)
{
	OffsetFromMax=degrees;
}


double FlamMap::GetOffsetFromMax()
{
	return OffsetFromMax;
}

void	FlamMap::SetWeatherFileName(char *name)
{
	strcpy(WeatherFileName, name);
}

void	FlamMap::SetFuelMoistureFileName(char *name)
{
	strcpy(FuelMoistureFileName, name);
}

void FlamMap::SetCustFuelModelName(char *name)
{
	strcpy(FuelModelName, name);
}

char *FlamMap::GetCustFuelModelName()
{
	return FuelModelName;
}

void FlamMap::SetWindFileName(char *name)
{
	strcpy(WindFileName, name);
}

char *FlamMap::GetWeatherFileName()
{
	return WeatherFileName;
}

char *FlamMap::GetFuelMoistureFileName()
{
	return FuelMoistureFileName;
}

char *FlamMap::GetWindFileName()
{
	return WindFileName;
}

bool FlamMap::PreserveInactiveEnclaves(int YesNo)
{
     if(YesNo>=0)
          InactiveEnclaves=(bool) YesNo;

     return InactiveEnclaves;
}

double FlamMap::GetDownTime()
{
	return DownTime;
}
/*ALM class CrossTab
{
public:
	CrossTab(int _nBins, int *binMins, int *binMaxs);
	~CrossTab();
	void AddObs(short obsVal);
	double Average();
	double OverallAverage();
	int nBins;
	double sigma;
	int nObs;
	int nValidObs;
	int *pMins;
	int *pMaxs;
	int *freqs;
};
class IndexItem
{
public:
	int freq;
	int loc;
};*/
/*
CrossTab::CrossTab(int _nBins, int *binMins, int *binMaxs)
{
	nBins = _nBins + 1;
	pMins = binMins;
	pMaxs = binMaxs;
	nValidObs = nObs = 0;
	sigma = 0.0;
	freqs = NULL;
	freqs = new int[nBins];
	for(int i = 0; i < nBins; i++)
		freqs[i] = 0;
}

CrossTab::~CrossTab()
{
	if(freqs)
		delete[] freqs;
}

void CrossTab::AddObs(short obsVal)
{
	nObs++;
	if(obsVal < 0)
		freqs[0]++;
	else
	{
		for(int b = 1; b < nBins; b++)
		{
			if((pMins[b] == pMaxs[b] && (obsVal == pMins[b]))// && obsVal <= pMaxs[b]))
				|| (pMins[b] != pMaxs[b] && (obsVal >= pMins[b] && obsVal < pMaxs[b]))
				|| (pMins[b] != pMaxs[b] && b == nBins - 1 ))
			{//right bin
				freqs[b]++;
				sigma += obsVal;
				nValidObs++;
				break;
			}
		}
	}
}

double CrossTab::Average()
{
	if(nValidObs <= 0)
		return 0.0;
	return sigma / nValidObs;
}

double CrossTab::OverallAverage()
{
	if(nObs <= 0)
		return 0.0;
	return sigma / nObs;
}
*/
int IndexCompare( const void *arg1, const void *arg2 )
{
	IndexItem pv1, pv2;
	pv1 = *(IndexItem *)arg1;
	pv2 = *(IndexItem *)arg2;
	if(pv1.freq > pv2.freq)
		return -1;
	else if(pv1.freq < pv2.freq)
		return 1;
	return 0;
}

int FlamMap::CritiqueHTML(char *FileName)
{
	return 0;
}

/*
float FlamMap::GetDisplayVal(float val, CLegendData *pLegend)
{
	int displayUnits = pLegend->GetUnits();
	float tVal = val;
	int dataUnits = 0;
	if(val != NODATA_VAL )//&& dataUnits)// != displayUnits)
	{
		switch(pLegend->GetThemeType())
		{
		case E_DATA:
			dataUnits = Header.EUnits;
			break;
		case S_DATA:
			dataUnits = Header.SUnits;
			break;
		case A_DATA:
			dataUnits = Header.AUnits;
			break;
		case F_DATA:
			dataUnits = 0;//Header.FUnits;
			break;
		case C_DATA:
			dataUnits = Header.CUnits;
			break;
		case H_DATA://always divide by 10...
			dataUnits = (Header.HUnits == 1 || Header.HUnits == 3) ? 2 : 3;
  			tVal /= 10.0;
			break;
		case B_DATA:
			dataUnits = (Header.BUnits == 1 || Header.BUnits == 3) ? 2 : 3;
			tVal /= 10.0;
			break;
		case P_DATA:
			dataUnits = (Header.PUnits == 1 || Header.PUnits == 3) ? 2 : 3;
			tVal /= 100.0;
			break;
		case D_DATA:
			dataUnits = Header.DUnits;
			//tVal /= 10.0;
			break;
		case W_DATA:
			dataUnits = 0;//Header.WUnits;
			break;
		default:
			break;
		}
		tVal = ConvertUnits(tVal, dataUnits, displayUnits, pLegend->GetThemeType());
	}
	return tVal;
}

float FlamMap::GetDataUnitsVal(float val, CLegendData *pLegend)
{
	int displayUnits = pLegend->GetUnits();
	float tVal = val;
	if(val != NODATA_VAL)//&& dataUnits)// != displayUnits)
	{
		switch(pLegend->GetThemeType())
		{
		case E_DATA:
			if(Header.EUnits == 0 && displayUnits == 1)
				tVal /= 3.2808;
			else if(Header.EUnits == 1 && displayUnits == 0)
				tVal *= 3.2808;
			break;
		case S_DATA:
			if(Header.SUnits == 1 && displayUnits == 0)
			
			{//convert to percent
				tVal = tan((PI * tVal)/ 180.0) * 100.0;
			}
			else if(Header.SUnits == 0 && displayUnits == 1)
			{//convert percent to degrees
				double fraction, ipart;
				double slopef;

				tVal = slopef = atan((double) tVal / 100.0) / PI * 180.0;
				fraction = modf(slopef, &ipart);
				if(fraction>=0.5)
					tVal++;
			}
			break;
		case A_DATA://always convert from degrees if necessary
			if(Header.AUnits == 0)// grass 1-25 counterclockwise from east to degrees
			{
				if(tVal != 25.0)
				{
					tVal = (tVal - 1.0) * 15.0 - 90;
					if(tVal <= 0)
						tVal = fabs(tVal);
					else
						tVal = 360 - tVal;
				}
			}
			else if(Header.AUnits == 1)// degrees 0 to 360 counterclockwise from east to degrees
			{
				tVal = tVal - 90;
				if(tVal <= 0)
					tVal = fabs(tVal);
				else
					tVal = 360 - tVal;
			}
			//otherwise already in degrees
			break;
		case F_DATA://no conversions
			break;
		case C_DATA://no conversions...can be in classes or percent
			break;
		case H_DATA://mult divide by 10...
		case B_DATA:
			{
				int dataUnits = (pLegend->GetThemeType() == H_DATA) ? Header.HUnits : Header.BUnits;
				if((dataUnits == 1 || dataUnits == 3) && displayUnits == 1)
				{//convert feet to meters 
					tVal /= 3.2808;
				}
				else if((dataUnits == 2 || dataUnits == 4) && displayUnits == 0)
				{//convert meters to feet
					tVal *= 3.2808;
				}
				tVal *= 10.0;
			}
			break;
		case P_DATA:
			if(Header.PUnits == 1 || Header.PUnits == 3)
			{
				if(displayUnits == 1)
					tVal /= 100.0;
				else if(displayUnits == 2 || displayUnits == 3)
				{
					tVal *= 16.0185;
					if(displayUnits == 3)
						tVal /= 1000.0;
				}
			}
			/*else if(dataUnits == 3)
			{
				if(displayUnits == 0)
					tVal *= 100.0;
				else if(displayUnits == 2 || displayUnits == 3)
				{
					//tVal 
					tVal *= 16.0185;
					if(displayUnits == 3)
						tVal /= 1000.0;
					tVal *= 100.0;
				}
			}*
			else if(Header.PUnits == 2 || Header.PUnits == 4)//lb//ft3 destuntis
			{
				if(displayUnits == 3)
					tVal /= 1000.0;
				else if(displayUnits == 0 || displayUnits == 1)
				{
					tVal /= 16.0185;
					if(displayUnits == 1)
						tVal /= 100.0;
				}
			}
			/*else if(dataUnits == 4)//lb/f3*1000 destunits
			{
				if(displayUnits == 2)
					tVal *= 1000.0;
				else if(displayUnits == 0 || displayUnits == 1)
				{
					tVal *= 16.0185;
					if(displayUnits == 3)
						tVal /= 100.0;
					tVal *= 1000.0;
				}
			}*
			/*if((dataUnits == 1 || dataUnits == 3) && displayUnits == 1)
			{//convert kg/m3 to 10lb/ft3
				tVal *=1.61845;;
			}
			else if((dataUnits == 2 || dataUnits == 4) && displayUnits == 0)
			{//convert 10lb/ft3 to kg/m3
				tVal /=1.61845;
			}*
			tVal *= 100.0;
			break;
		case D_DATA:
			if(Header.DUnits == 1 && displayUnits == 1)
			{//convert Mg/Ha to Tons/Acre
				tVal *= 2.2417088978002777;
			}
			else if(Header.DUnits == 2 && displayUnits == 0)
			{//convert Tons/Acre to Mg/Ha
				tVal /= 2.2417088978002777;
			}
			tVal *= 10.0;
			break;
		case W_DATA://classes, don't change
			break;
	/*	case GENERIC_GRID://generic, don't change
			break;
		case TT_FLAMELEN://meters or feet
			if(dataUnits == 0 && displayUnits == 1)
			{//meters to feet
				tVal /= 3.2808;
			}
			else if(dataUnits == 1 && displayUnits == 0)
			{//feet to meters
				tVal *= 3.2808;
			}
			break;
		case TT_INTENSITY:// kW/m	/=3.4613 BTU/ft/sec
			if(dataUnits == 0 && displayUnits == 1)
			{//
				tVal *= 3.4613;
			}
			else if(dataUnits == 1 && displayUnits == 0)
			{
				tVal /= 3.4613;
			}
			break;
		case TT_HEATAREA:// kJ/m2    *=0.08806  BTU/ft2  
			if(dataUnits == 0 && displayUnits == 1)
			{//
				tVal /= 0.08806;
			}
			else if(dataUnits == 1 && displayUnits == 0)
			{
				tVal *= 0.08806;
			}
			break;
		case TT_CROWNSTATE://classes leave alone
			break;
		case TT_SOLRRADIATION:// W/m2, leave alone
			break;
		case TT_FUELMOISTURE1://percentage, leave alone
			break;
		case TT_FUELMOISTURE10://percentage, leave alone
			break;
		case TT_MIDFLAME:// mph    /=0.62125 kph
			if(dataUnits == 0 && displayUnits == 1)
			{//
				tVal *= 0.62125;
			}
			else if(dataUnits == 1 && displayUnits == 0)
			{
				tVal /= 0.62125;
			}
			break;
		case TT_ELLIPSEDIMC:// m/min	*=3.2808 ft/min
		case TT_ELLIPSEDIMB:// m/min	*=3.2808 ft/min
		case TT_ELLIPSEDIMA:// m/min	*=3.2808 ft/min
		case TT_HORIZRATE:// m/min   *=3.2808 ft/min
		case TT_SPREADRATE://m/min or ft/min
			if(dataUnits == 0 && displayUnits == 1)
			{//
				tVal /= 3.2808;
			}
			else if(dataUnits == 1 && displayUnits == 0)
			{
				tVal *= 3.2808;
			}
			else if(dataUnits == 0 && displayUnits == 2)//chains/hr
			{
				tVal *= 66.0;
				tVal /= 60.0;
				tVal /= 3.2808;
			}
			break;
		case TT_MAXSPREADDIR://radians  value/PI*180.0 degrees
			if(dataUnits == 1 && displayUnits == 0)
			
			{//
				tVal = tVal * 180 / PI;
			}
			else if(dataUnits == 0 && displayUnits == 1)
			{
				tVal = tVal * PI / 180;
			}
			break;*
		}
	}
	return tVal;
}
*/
float FlamMap::ConvertUnits(float val, int srcUnits, int destUnits, int themeType)
{
	float tVal = val;
	if(val != NODATA_VAL)//&& dataUnits)// != destUnits)
	{
		switch(themeType)
		{
		case E_DATA:
			if(srcUnits == 0 && destUnits == 1)
				tVal *= 3.2808;
			else if(srcUnits == 1 && destUnits == 0)
				tVal /= 3.2808;
			break;
		case S_DATA:
			if(srcUnits == 0 && destUnits == 1)
			{//convert to percent
				tVal = tan((PI * tVal)/ 180.0) * 100.0;
			}
			else if(srcUnits == 1 && destUnits == 0)
			{//convert percent to degrees
				double fraction, ipart;
				double slopef;

				tVal = slopef = atan((double) tVal / 100.0) / PI * 180.0;
				fraction = modf(slopef, &ipart);
				if(fraction>=0.5)
					tVal++;
			}
			break;
		case A_DATA://always convert to degrees if necessary
			if(srcUnits == 0)// grass 1-25 counterclockwise from east to degrees
			{
				if(tVal != 25.0)
				{
					tVal = (tVal - 1.0) * 15.0 - 90;
					if(tVal <= 0)
						tVal = fabs(tVal);
					else
						tVal = 360 - tVal;
				}
			}
			else if(srcUnits == 1)// degrees 0 to 360 counterclockwise from east to degrees
			{
				tVal = tVal - 90;
				if(tVal <= 0)
					tVal = fabs(tVal);
				else
					tVal = 360 - tVal;
			}
			//otherwise already in degrees
			break;
		case F_DATA://no conversions
			break;
		case C_DATA://can be in classes or percent
			if(srcUnits == 0 && destUnits == 1)//convert class to percent
			{
				if(val == 99.0) 
					tVal=0; 
				else if(val == 1.0)  
					tVal=10; 
				else if(val == 2.0)  
					tVal=30; 
				else if(val == 3.0)  
					tVal=60; 
				else if(val == 4.0)  
					tVal=75; 
				else 
					tVal=0; 

			}
			else if(srcUnits == 1 && destUnits == 0)//convert percent to class
			{
				if(val >= 70)
					tVal = 4;
				else if(val >= 45)
					tVal = 3;
				else if(val >= 20)
					tVal = 2;
				else if(val > 0)
					tVal = 1;
				else
					tVal = 0;
			}
			break;
		case H_DATA://always divide by 10...
  			//tVal /= 10.0;
			if((srcUnits == 0 || srcUnits == 2) && destUnits == 1)
			{//convert meters to feet
				tVal *= 3.2808;
			}
			else if((srcUnits == 1 || srcUnits == 3) && destUnits == 0)
			{//convert feet to meters
				tVal /= 3.2808;
			}
			break;
		case B_DATA:
			//tVal /= 10.0;
			if((srcUnits == 0 || srcUnits == 2) && destUnits == 1)
			{//convert meters to feet
				tVal *= 3.2808;
			}
			else if((srcUnits == 1 || srcUnits == 3) && destUnits == 0)
			{//convert feet to meters
				tVal /= 3.2808;
			}
			break;
		case P_DATA:
			//tVal /= 100.0;
			if(srcUnits == 0 || srcUnits == 2)//kg/m3
			{
				if(destUnits == 1)
					tVal *= 100.0;
				else if(destUnits == 2 || destUnits == 3)
				{
					tVal /= 16.0185;
					if(destUnits == 3)
						tVal *= 1000.0;
				}
			}
			/*else if(srcUnits == 3)//kg/m3 * 100
			{
				if(destUnits == 1)
					tVal *= 100.0;
				else if(destUnits == 2 || destUnits == 3)
				if(destUnits == 0)
					tVal /= 100.0;
				else if(destUnits == 2 || destUnits == 3)
				{
					tVal /= 100.0;
					tVal /= 16.0185;
					if(destUnits == 3)
						tVal *= 1000.0;
				}
			}*/
			else if(srcUnits == 1 || srcUnits == 3)
			{
				if(destUnits == 3)
					tVal *= 1000.0;
				else if(destUnits == 0 || destUnits == 1)
				{
					tVal *= 16.0185;
					if(destUnits == 1)
						tVal *= 100.0;
				}
			}
			/*else if(srcUnits == 4)
			{
				if(destUnits == 2)
					tVal /= 1000.0;
				else if(destUnits == 0 || destUnits == 1)
				{
					tVal /= 1000.0;
					tVal *= 16.0185;
					if(destUnits == 1)
						tVal *= 100.0;
				}
			}*/
			/*if((srcUnits == 1 || srcUnits == 3) && destUnits == 1)
			{//convert kg/m3 to 10lb/ft3
				//tVal /=1.61845;;
				tVal /=1.60185;
			}
			else if((srcUnits == 2 || srcUnits == 4) && destUnits == 0)
			{//convert 10lb/ft3 to kg/m3
				//tVal *=1.61845;
				tVal *=1.60185;
			}*/
			break;
		case D_DATA:
			//tVal /= 10.0;
			if(srcUnits == 1 && destUnits == 1)
			{//convert Mg/Ha to Tons/Acre
				tVal /= 2.2417088978002777;
			}
			else if(srcUnits == 2 && destUnits == 0)
			{//convert Tons/Acre to Mg/Ha
				tVal *= 2.2417088978002777;
			}
			break;
		case W_DATA://classes, don't change
			break;
		/*case GENERIC_GRID://generic, don't change
			break;
		case TT_FLAMELEN://meters or feet
			if(srcUnits == 0 && destUnits == 1)
			{//meters to feet
				tVal *= 3.2808;
			}
			else if(srcUnits == 1 && destUnits == 0)
			{//feet to meters
				tVal /= 3.2808;
			}
			break;
		case TT_INTENSITY:// kW/m	/=3.4613 BTU/ft/sec
		case TT_FLIMAP:
			if(srcUnits == 0 && destUnits == 1)
			{//
				tVal /= 3.4613;
			}
			else if(srcUnits == 1 && destUnits == 0)
			{
				tVal *= 3.4613;
			}
			break;
		case TT_HEATAREA:// kJ/m2    *=0.08806  BTU/ft2  
			if(srcUnits == 0 && destUnits == 1)
			{//
				tVal *= 0.08806;
			}
			else if(srcUnits == 1 && destUnits == 0)
			{
				tVal /= 0.08806;
			}
			break;
		case TT_CROWNSTATE://classes leave alone
			break;
		case TT_SOLRRADIATION:// W/m2, leave alone
			break;
		case TT_FUELMOISTURE1://percentage, leave alone
			break;
		case TT_FUELMOISTURE10://percentage, leave alone
			break;
		case TT_MIDFLAME:// mph    /=0.62125 kph
			if(srcUnits == 0 && destUnits == 1)
			{//
				tVal /= 0.62125;
			}
			else if(srcUnits == 1 && destUnits == 0)
			{
				tVal *= 0.62125;
			}
			break;
		case TT_ELLIPSEDIMC:// m/min	*=3.2808 ft/min
		case TT_ELLIPSEDIMB:// m/min	*=3.2808 ft/min
		case TT_ELLIPSEDIMA:// m/min	*=3.2808 ft/min
		case TT_HORIZRATE:// m/min   *=3.2808 ft/min
		case TT_SPREADRATE://m/min or ft/min
		case TT_ROS:
			if(srcUnits == 0 && destUnits == 1)
			{//
				tVal *= 3.2808;
			}
			else if(srcUnits == 1 && destUnits == 0)
			{
				tVal /= 3.2808;
			}
			else if(srcUnits == 0 && destUnits == 2)//chains/hr
			{
				tVal *= 3.2808;
				tVal *= 60.0;
				tVal /= 66.0;
			}
			break;
		case TT_MAXSPREADDIR://radians  value/PI*180.0 degrees
			if(srcUnits == 0 && destUnits == 1)
			{//
				tVal = tVal * 180 / PI;
			}
			else if(srcUnits == 1 && destUnits == 0)
			{
				tVal = tVal * PI / 180;
			}
			break;*/
		}
	}
	return tVal;

}
/*ALM
int FlamMap::GenerateLegendImage(int _layer, char *_fileName)
{
	int ret = 0;
	CImage image;
	if(!legends[_layer])
		return ret;
	
	if(FAILED(image.Create(100, 100, 24)))
	//image.Attach(hBitmap);//(Header.numeast, Header.numnorth, 24)))
	{
		return ret;
	}
	if(image.IsNull())
		return 0;
	CDC *pDC = CDC::FromHandle(image.GetDC());
	CFont labelFont;
	labelFont.CreatePointFont(80, _T("Arial"), pDC);
	CFont *restoreFont = pDC->SelectObject(&labelFont);
	CSize sz = pDC->GetTextExtent(_T("MMMMMMMM"));
	int cellH = sz.cy + 6;
	int stepH = cellH + 4;
	int lCell = 5, rCell = sz.cx + lCell, lTxt = rCell + 10, rTxt = lTxt + sz.cx;
	int hgt = stepH * (legends[_layer]->numEntries + 3);
	int wid = rTxt + 15;
	pDC->SelectObject(restoreFont);
	labelFont.DeleteObject();
	image.ReleaseDC();
	image.Destroy();
	if(FAILED(image.Create(wid, hgt, 24)))
	//image.Attach(hBitmap);//(Header.numeast, Header.numnorth, 24)))
	{
		return ret;
	}
	if(image.IsNull())
		return 0;
	pDC = CDC::FromHandle(image.GetDC());
	labelFont.CreatePointFont(120, _T("Arial"), pDC);
	pDC->FillSolidRect(0, 0, wid, hgt, RGB(200, 200, 200));
	CBrush brush(RGB(200, 200, 200)), redBrush(RGB(200, 0, 0)), *pOldBrush = NULL;
	pOldBrush = pDC->SelectObject(&redBrush);
	int t = 3 * stepH;
	char tmpFmt[16];
	pDC->SetTextAlign(TA_LEFT | TA_TOP);
	sprintf(tmpFmt,"%%.%df ", legends[_layer]->decimalPlaces);
	//CA2W pszFmt(tmpFmt);
	for(int c = 0; c < legends[_layer]->numEntries; c++)
	{
		CString tStr;
		if(c == 0)
			tStr = "No Data";
		else
			tStr.Format(tmpFmt, GetDisplayVal(legends[_layer]->entries[c]->GetVal(), legends[_layer]));
			//tStr.Format(pszFmt, GetDisplayVal(legends[_layer]->entries[c]->GetVal(), legends[_layer]));
			//CRect tRect(lLabels, t, lLabels + wLabels, t + minSize.cy);
		CRect cRect(lCell, t, rCell, t + cellH);
		pDC->TextOut(lTxt, t, tStr);
		CBrush brush;
		brush.CreateSolidBrush(legends[_layer]->entries[c]->GetColor());
		CBrush *oldBrush = pDC->SelectObject(&brush);
		pDC->Rectangle(cRect);
		t += stepH;
		pDC->SelectObject(oldBrush);
	}
	pDC->TextOut(lCell, cellH, legends[_layer]->title);
	char uStr[32];
	sprintf(uStr, "(%s)", legends[_layer]->GetUnitsString());
	//uStr.Format(_T("(%s)"), legends[_layer]->GetUnitsString());
	pDC->TextOut(lCell, cellH * 2, uStr);//"(" + legends[_layer]->GetUnitsString() + ")");
	//pDC->Rectangle(lCell, t, rCell, t + cellH);
	//pDC->TextOutW(lTxt, t, _T("Value"));
	pDC->SelectObject(restoreFont);
	pDC->SelectObject(pOldBrush);
	image.ReleaseDC();
	//CA2W pszA(_fileName);
	//image.Save(pszA);
	image.Save(_fileName);
	return 1;
}
*/
/*int FlamMap::GenerateLegendImage(int _layer, char *_fileName)
{
	int ret = 0;

	if(!legends[_layer])
		return ret;


	CImg<unsigned char> text;
	const unsigned char gray[] = { 125,125,125 };
	const unsigned char black[] = {0,0,0};
	const unsigned char red[] = {153,153,153};
	const unsigned char white[] = {255,255,255};
    text.assign().draw_text(0,0,"MMMMMMMMMMM",white,black,1,13);
	int cellH = text.height();
   
	
	int stepH = cellH + 4;
	int lCell = 5, rCell = 40 + lCell, lTxt = rCell + 10, rTxt = lTxt + text.width();
	int hgt = stepH * (legends[_layer]->numEntries + 4);
	int wid = rTxt + 10;

	// dimensions for 3-column legend
	if (legends[_layer]->numEntries > 40)
	{
         wid = wid * 3 + 10;
		 hgt = stepH * (legends[_layer]->numEntries/3 + 4);
	} else // dimensions for 3-column legend
	if (legends[_layer]->numEntries > 20)
	{
         wid = wid * 2 + 10;
		 hgt = stepH * (legends[_layer]->numEntries/2 + 4);
	}

	
	
    CImg<unsigned char> img(wid,hgt,1,3);
	
	img.fill(200,200,200);

	//draw axis

	int t = 2 * stepH;
	char tmpFmt[16];
	
	
	img.draw_text(lTxt, t,"Value     Freq",black,0,1,13);
	
	t += stepH ;

	sprintf(tmpFmt,"%%.%df     %%6d", legends[_layer]->decimalPlaces);
	int offset = 0;
	//CA2W pszFmt(tmpFmt);
	for(int c = 0; c < legends[_layer]->numEntries; c++)
	{
		  // 3-columns
		if (legends[_layer]->numEntries > 40)
		{
			if (c == legends[_layer]->numEntries / 3)
			{
				offset = rTxt + 10;
                t = 2 * stepH;
				img.draw_text(lTxt + offset, t,"Value     Freq",black,0,1,13);
				t += stepH;
			} else
			if (c == legends[_layer]->numEntries * 2 / 3)
			{
				offset = offset * 2;
                t = 2 * stepH;
				img.draw_text(lTxt + offset, t,"Value     Freq",black,0,1,13);
				t += stepH;
			}
	    }
		else
	    // 2 columns
		if (legends[_layer]->numEntries > 20)
			if (c == legends[_layer]->numEntries / 2)
			{
				offset = rTxt + 10;
                t = 2 * stepH;
				img.draw_text(lTxt + offset, t,"Value     Freq",black,0,1,13);
				t += stepH;
			}

		char tmpStr[200];
		if(c == 0)
		{
            sprintf(tmpStr,"No Data %6d",legends[_layer]->entries[c]->GetFrequency() );
			
		}
		else
		{
			
			sprintf(tmpStr,tmpFmt,GetDisplayVal(legends[_layer]->entries[c]->GetVal(), legends[_layer]),legends[_layer]->entries[c]->GetFrequency());
		}
			
	
		img.draw_text(lTxt + offset, t+2,tmpStr,black,0,1,13);
		
		// get color and convert to RGB
		COLORREF lcolor = legends[_layer]->entries[c]->GetColor();

		unsigned char bR,bG,bB;
        bB = GetBValue(lcolor);
		bG = GetGValue(lcolor);
		bR = GetRValue(lcolor);
		

		const unsigned char tColor[] = {bR,bG,bB};
		img.draw_rectangle(lCell + offset,t,rCell + offset,t + cellH,tColor,1);
		t += stepH;
	}
    
	text.assign().draw_text(0,0,"%s",white,0,1,17,legends[_layer]->title);

    img.draw_text(wid/2 - text.width()/2, cellH - 12,"%s",black,0,1,17, legends[_layer]->title);	

	text.assign().draw_text(0,0,"(%s)",white,0,1,13,legends[_layer]->GetUnitsString());
    img.draw_text(wid/2 - text.width()/2, cellH * 2 - 8, "(%s)",black,0,1,13,legends[_layer]->GetUnitsString());


	img.save_jpeg(_fileName);
	return 1;
}

int FlamMap::GenerateImage(int _layer, char *_fileName)
{
	int ret = 0;
	
	CImg<unsigned char> text;
	const unsigned char gray[] = { 125,125,125 };
	const unsigned char black[] = {0,0,0};
	const unsigned char red[] = {153,153,153};
	const unsigned char white[] = {255,255,255};
    CImg<unsigned char> img(this->analysisRect.Width(),this->analysisRect.Height(),1,3);
	
	img.fill(255,255,255);


	if(!legends[_layer])
		return ret;
	
	celldata tCell;
	crowndata tCrown;
	grounddata tGround;
	
	float lVal;
	for(int y = 0; y < GetNumNorth(); y++)
	{
		for(int x = 0; x < GetNumEast(); x++)
		{
			//int posit=(y*Header.numeast+x);
			  int64_t p = ((int64_t)y* (int64_t)Header.numeast+x);
			  if(!CantAllocLCP)
			  {
				 GetCellDataFromMemory(p, tCell, tCrown, tGround);
                 //GetCellDataFromMemory(posit, tCell, tCrown, tGround);
			
				 switch(_layer)
			     {
			     case E_DATA:
				     lVal = tCell.e;
				     break;
			     case S_DATA:
				     lVal = tCell.s;
				     break;
			     case A_DATA:
				     lVal = tCell.a;
				     break;
			     case F_DATA:
				     lVal = tCell.f;
				     break;
			     case C_DATA:
				     lVal = tCell.c;
				     break;
			     case H_DATA:
				     lVal = tCrown.h;
				     break;
			     case B_DATA:
				     lVal = tCrown.b;
				     break;
			     case P_DATA:
				     lVal = tCrown.p;
				     break;
			     case D_DATA:
				     lVal = tGround.d;
				     break;
			     case W_DATA:
				     lVal = tGround.w;
				     break;
			     } 
			
			  }
			  else//gotta go to disk...
			  {
				  lVal = this->GetLayerValueByCell(_layer, x, y);
			  }

			COLORREF c = legends[_layer]->MapColor(lVal);
			unsigned char bR, bB, bG;
			bB = GetBValue(c);
			bG = GetGValue(c);
			bR = GetRValue(c);

			unsigned char tColor[] = {bR,bG,bB};
            

			img.draw_point(x - analysisRect.left, y - analysisRect.top,tColor,1);

		
		}
	}
	
	img.save_jpeg(_fileName);
	return 1;
}*/
/*
int FlamMap::GenerateImage(int _layer, char *_fileName)
{
	

	int ret = 0;
	/*HDC hdcMain = GetDC(NULL);

    BITMAPINFO bmInfo; 
    bmInfo.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
	bmInfo.bmiHeader.biPlanes        = 1;
	bmInfo.bmiHeader.biBitCount      = 24;
	bmInfo.bmiHeader.biCompression   = BI_RGB;
	bmInfo.bmiHeader.biXPelsPerMeter = 0;
	bmInfo.bmiHeader.biYPelsPerMeter = 0;
	bmInfo.bmiHeader.biClrUsed       = 0;
	bmInfo.bmiHeader.biClrImportant  = 0;
	bmInfo.bmiHeader.biWidth         = Header.numeast;
	bmInfo.bmiHeader.biHeight        = Header.numnorth;
	bmInfo.bmiHeader.biSizeImage     = 0;

	PVOID pBits;
	HBITMAP hBitmap = CreateDIBSection(hdcMain, &bmInfo, DIB_RGB_COLORS, &pBits, NULL, 0);*

	CImage image;
	if(!legends[_layer])
		return ret;
	if(FAILED(image.Create(this->analysisRect.Width(), this->analysisRect.Height(), 24)))
	//image.Attach(hBitmap);//(Header.numeast, Header.numnorth, 24)))
	{
		return ret;
	}
	celldata tCell;
	crowndata tCrown;
	grounddata tGround;
	if(image.IsNull())
		return 0;
	//CDC *pDC = CDC::FromHandle(image.GetDC());
	//pDC->FillSolidRect(0, 0, Header.numeast, Header.numnorth, GetSysColor(COLOR_WINDOW));
	//for(int y = 0; y < Header.numnorth; y++)
	//{
	//	for(int x = 0; x < Header.numeast; x++)
	//	{
	//for(int y = analysisRect.top; y <= analysisRect.bottom; y++)
	//{
		//for(int x = analysisRect.left; x <= analysisRect.right; x++)
		//{
	float lVal;
	for(int y = 0; y < GetNumNorth(); y++)
	{
		for(int x = 0; x < GetNumEast(); x++)
		{
			//int posit=(y*Header.numeast+x);
			  int64_t p = ((int64_t)y* (int64_t)Header.numeast+x);
			  if(!CantAllocLCP)
			  {
				GetCellDataFromMemory(p, tCell, tCrown, tGround);

			//GetCellDataFromMemory(posit, tCell, tCrown, tGround);
			switch(_layer)
			{
			case E_DATA:
				lVal = tCell.e;
				break;
			case S_DATA:
				lVal = tCell.s;
				break;
			case A_DATA:
				lVal = tCell.a;
				break;
			case F_DATA:
				lVal = tCell.f;
				break;
			case C_DATA:
				lVal = tCell.c;
				break;
			case H_DATA:
				lVal = tCrown.h;
				break;
			case B_DATA:
				lVal = tCrown.b;
				break;
			case P_DATA:
				lVal = tCrown.p;
				break;
			case D_DATA:
				lVal = tGround.d;
				break;
			case W_DATA:
				lVal = tGround.w;
				break;
			}
			  }
			  else//gotta go to disk...
			  {
				  lVal = this->GetLayerValueByCell(_layer, x, y);
			  }

			//ALM COLORREF c = legends[_layer]->MapColor(lVal);
			if(image.IsDIBSection())
			{
				BYTE *p = (BYTE*)image.GetPixelAddress(x - analysisRect.left, y - analysisRect.top);
				if(!p)
					return 0;
				*p++ = GetBValue(c);
    			*p++ = GetGValue(c);
    			*p = GetRValue(c);
			}
			else//slow but safe
			{
				image.SetPixel(x - analysisRect.left, y - analysisRect.top, c);
			}
		}
	}
	//CA2W pszA(_fileName);
	//image.Save(pszA);
	image.Save(_fileName);
	return 1;
}

int FlamMap::TrimLandscape()
{
	if(CantAllocLCP)
		return 0;
	int colMin = this->GetNumEast(), colMax = -1, rowMin = this->GetNumNorth(), rowMax = -1, r, c;
	int fuel;
	for(c = 0; c < GetNumEast(); c++)
	{
		for(r = 0; r < GetNumNorth(); r++)
		{
			fuel = GetLayerValueByCell(FUEL, c, r);
			if((fuel > 0 && fuel < 90) || (fuel > 99 && fuel < 256))//burnable fuel
			{
				colMin = c;
				break;
			}
		}
		if(colMin < GetNumEast())
			break;
	}
	if(colMin >= GetNumEast())
		return 0;
	for(int c = GetNumEast() - 1; c >= 0; c--)
	{
		for(int r = 0; r < GetNumNorth(); r++)
		{
			fuel = GetLayerValueByCell(FUEL, c, r);
			if((fuel > 0 && fuel < 90) || (fuel > 99 && fuel < 256))//burnable fuel
			{
				colMax = c;
				break;
			}
		}
		if(colMax >= 0)
			break;
	}
	for(r = 0; r < GetNumNorth(); r++)
	{
		for(c = 0; c < GetNumEast(); c++)
		{
			fuel = GetLayerValueByCell(FUEL, c, r);
			if((fuel > 0 && fuel < 90) || (fuel > 99 && fuel < 256))//burnable fuel
			{
				rowMin = r;
				break;
			}
		}
		if(rowMin < GetNumNorth())
			break;
	}
	for(r = GetNumNorth() - 1; r >= 0; r--)
	{
		for(c = 0; c < GetNumEast(); c++)
		{
			fuel = GetLayerValueByCell(FUEL, c, r);
			if((fuel > 0 && fuel < 90) || (fuel > 99 && fuel < 256))//burnable fuel
			{
				rowMax = r;
				break;
			}
		}
		if(rowMax >= 0)
			break;
	}
	double newEast, newWest, newNorth, newSouth;
	newEast = GetEastUtm() - ((GetNumEast() - colMax - 1) * GetCellResolutionX());
	newWest = GetWestUtm() + (colMin *  GetCellResolutionX());
	newSouth = GetSouthUtm() + ((GetNumNorth() - rowMax - 1) * GetCellResolutionY());
	newNorth = GetNorthUtm() - (rowMin * GetCellResolutionY());
	return SetAnalysisArea(newEast, newWest, newNorth, newSouth);
	//return 1;
}
*/

int FlamMap::ResampleLandscape(double NewRes)
{
	if(CantAllocLCP)
		return 0;
	int i, j, xloc, yloc, NewNumNorth, NewNumEast;//, NumVals;
	double xpos, ypos;
	short *newlcp, *temp;

	i=-1;
	do
	{	i++;
		ypos=Header.NorthUtm-((double) i*NewRes)-NewRes/2.0;
	}while(ypos>Header.SouthUtm);
	NewNumNorth=i;

	i=-1;
	do
	{	i++;
		xpos=Header.WestUtm+((double) i*NewRes)+NewRes/2.0;
	}while(xpos<Header.EastUtm);
	NewNumEast=i;

	if(HaveCrownFuels())
     {	if(HaveGroundFuels())
     		NumVals=10;
	     else
     		NumVals=8;
     }
     else
     {	if(HaveGroundFuels())
     		NumVals=7;
	     else
     		NumVals=5;
     }

	newlcp=new short[NewNumNorth*NewNumEast*NumVals];
	memset(newlcp, 0x0, NewNumNorth*NewNumEast*NumVals*sizeof(short));

	for(i=0; i<NewNumNorth; i++)
	{	ypos=Header.NorthUtm-((double) i*NewRes)-NewRes/2.0;
		if(ypos<=Header.SouthUtm)
			break;
		yloc=(int) ((Header.NorthUtm-ypos)/Header.YResol);	// old position
		for(j=0; j<NewNumEast; j++)
		{	xpos=Header.WestUtm+((double) j*NewRes)+NewRes/2.0;	
			if(xpos>=Header.EastUtm)
				break;
			xloc=(int) ((xpos-Header.WestUtm)/Header.XResol);	// old position
			memcpy(&newlcp[((int64_t)i*(int64_t)NewNumEast+j)*(int64_t)NumVals], 
				&landscape[((int64_t)yloc*(int64_t)Header.numeast+xloc)*(int64_t)NumVals], 
				NumVals*sizeof(short));
		}
	}

	//temp=landscape;
	delete[] landscape;
	landscape=newlcp;
	//newlcp=temp;

	Header.numnorth=NewNumNorth;
	Header.numeast=NewNumEast;
	Header.SouthUtm=Header.NorthUtm-(double) (NewNumNorth)*NewRes;
	Header.EastUtm=Header.WestUtm+(double) (NewNumEast)*NewRes;
	Header.XResol=Header.YResol=NewRes;

	 analysisLoEast = Header.loeast = Header.WestUtm;
	 analysisHiEast = Header.hieast = Header.EastUtm;
	 printf("analysisHiEast6 = %f", analysisHiEast);
	 analysisLoNorth = Header.lonorth = Header.SouthUtm;
	 analysisHiNorth = Header.hinorth = Header.NorthUtm;
	 analysisRect.left = 0;
	 analysisRect.right = Header.numeast - 1;
	 analysisRect.top = 0;
	 analysisRect.bottom = Header.numnorth - 1;
	return 1;
}

/*
bool FlamMap::AllocFarsiteEvents(int EventNum, int numevents, char *basename, bool ManReset, bool InitState)
{
	bool ret;

     switch(EventNum)
     {	case 1: ret=hBurnEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 2: ret=hMoistEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 3: ret=hBurnupEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 4: ret=hIntegEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 5: ret=hBurnThreadEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 6: ret=hMoistThreadEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 7: ret=hBurnupThreadEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 8: ret=hIntegThreadEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 9: ret=hCrossEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 10: ret=hCrossThreadEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     }

     return ret;
}

bool	FlamMap::FreeFarsiteEvents(int EventNum)
{
	bool ret;

     switch(EventNum)
     {	case 1: ret=hBurnEvent.FreeEvents(); break;
     	case 2: ret=hMoistEvent.FreeEvents(); break;
     	case 3: ret=hBurnupEvent.FreeEvents(); break;
     	case 4: ret=hIntegEvent.FreeEvents(); break;
     	case 5: ret=hBurnThreadEvent.FreeEvents(); break;
     	case 6: ret=hMoistThreadEvent.FreeEvents(); break;
     	case 7: ret=hBurnupThreadEvent.FreeEvents(); break;
     	case 8: ret=hIntegThreadEvent.FreeEvents(); break;
     	case 9: ret=hCrossEvent.FreeEvents(); break;
     	case 10: ret=hCrossThreadEvent.FreeEvents(); break;
     }

     return ret;
}


bool FlamMap::SetFarsiteEvent(int EventNum, int ThreadNum)
{
	bool ret;

     switch(EventNum)
     {	case 1: ret=hBurnEvent.SetEvent(ThreadNum); break;
     	case 2: ret=hMoistEvent.SetEvent(ThreadNum); break;
     	case 3: ret=hBurnupEvent.SetEvent(ThreadNum); break;
     	case 4: ret=hIntegEvent.SetEvent(ThreadNum); break;
     	case 5: ret=hBurnThreadEvent.SetEvent(ThreadNum); break;
     	case 6: ret=hMoistThreadEvent.SetEvent(ThreadNum); break;
     	case 7: ret=hBurnupThreadEvent.SetEvent(ThreadNum); break;
     	case 8: ret=hIntegThreadEvent.SetEvent(ThreadNum); break;
     	case 9: ret=hCrossEvent.SetEvent(ThreadNum); break;
     	case 10: ret=hCrossThreadEvent.SetEvent(ThreadNum); break;
     }

     return ret;
}

bool FlamMap::ResetFarsiteEvent(int EventNum, int ThreadNum)
{
	bool ret;

     switch(EventNum)
     {	case 1: ret=hBurnEvent.ResetEvent(ThreadNum); break;
     	case 2: ret=hMoistEvent.ResetEvent(ThreadNum); break;
     	case 3: ret=hBurnupEvent.ResetEvent(ThreadNum); break;
     	case 4: ret=hIntegEvent.ResetEvent(ThreadNum); break;
     	case 5: ret=hBurnThreadEvent.ResetEvent(ThreadNum); break;
     	case 6: ret=hMoistThreadEvent.ResetEvent(ThreadNum); break;
     	case 7: ret=hBurnupThreadEvent.ResetEvent(ThreadNum); break;
     	case 8: ret=hIntegThreadEvent.ResetEvent(ThreadNum); break;
     	case 9: ret=hCrossEvent.ResetEvent(ThreadNum); break;
     	case 10: ret=hCrossThreadEvent.ResetEvent(ThreadNum); break;
     }

     return ret;
}

bool FlamMap::WaitForFarsiteEvents(int EventNum, int numevents, bool All, unsigned int Wait)
{
	bool ret;

     switch(EventNum)
     {	case 1: ret=hBurnEvent.WaitForEvents(numevents, All, Wait); break;
     	case 2: ret=hMoistEvent.WaitForEvents(numevents, All, Wait); break;
     	case 3: ret=hBurnupEvent.WaitForEvents(numevents, All, Wait); break;
     	case 4: ret=hIntegEvent.WaitForEvents(numevents, All, Wait); break;
     	case 5: ret=hBurnThreadEvent.WaitForEvents(numevents, All, Wait); break;
     	case 6: ret=hMoistThreadEvent.WaitForEvents(numevents, All, Wait); break;
     	case 7: ret=hBurnupThreadEvent.WaitForEvents(numevents, All, Wait); break;
     	case 8: ret=hIntegThreadEvent.WaitForEvents(numevents, All, Wait); break;
     	case 9: ret=hCrossEvent.WaitForEvents(numevents, All, Wait); break;
     	case 10: ret=hCrossThreadEvent.WaitForEvents(numevents, All, Wait); break;
     }

     return ret;
}


bool FlamMap::WaitForOneFarsiteEvent(int EventNum, int ThreadNum, unsigned int Wait)
{
	bool ret;

     switch(EventNum)
     {	case 1: ret=hBurnEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 2: ret=hMoistEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 3: ret=hBurnupEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 4: ret=hIntegEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 5: ret=hBurnThreadEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 6: ret=hMoistThreadEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 7: ret=hBurnupThreadEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 8: ret=hIntegThreadEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 9: ret=hCrossEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 10: ret=hCrossThreadEvent.WaitForOneEvent(ThreadNum, Wait); break;
     }

     return ret;
}
*/
/*
HANDLE FlamMap::GetFarsiteEvent(int EventNum, int ThreadNum)
{
     HANDLE ret;

     switch(EventNum)
     {	case 1: ret=hBurnEvent.GetEvent(ThreadNum); break;
     	case 2: ret=hMoistEvent.GetEvent(ThreadNum); break;
     	case 3: ret=hBurnupEvent.GetEvent(ThreadNum); break;
     	case 4: ret=hIntegEvent.GetEvent(ThreadNum); break;
     	case 5: ret=hBurnThreadEvent.GetEvent(ThreadNum); break;
     	case 6: ret=hMoistThreadEvent.GetEvent(ThreadNum); break;
     	case 7: ret=hBurnupThreadEvent.GetEvent(ThreadNum); break;
     	case 8: ret=hIntegThreadEvent.GetEvent(ThreadNum); break;
     	case 9: ret=hCrossEvent.GetEvent(ThreadNum); break;
     	case 10: ret=hCrossThreadEvent.GetEvent(ThreadNum); break;
     }

     return ret;
}
*/
/*void FlamMap::MassageMoistures(void)
{
	int numNorth = GetNumNorth(), numEast = GetNumEast();//, layerType = Layer;
	const float MINMOIST = 0.02;
	if(outlayer[FUELMOISTURE1])
	{
		for(int p = 0; p < numNorth *numEast; p++)
		{
			if(outlayer[FUELMOISTURE1][p] > 0.0 && outlayer[FUELMOISTURE1][p] < MINMOIST)
				outlayer[FUELMOISTURE1][p] = MINMOIST;
		}
	}
	if(outlayer[FUELMOISTURE10])
	{
		for(int p = 0; p < numNorth *numEast; p++)
		{
			if(outlayer[FUELMOISTURE10][p] > 0.0 && outlayer[FUELMOISTURE10][p] < MINMOIST)
				outlayer[FUELMOISTURE10][p] = MINMOIST;
		}
	}
	if(outlayer[FUELMOISTURE100])
	{
		for(int p = 0; p < numNorth *numEast; p++)
		{
			if(outlayer[FUELMOISTURE100][p] > 0.0 && outlayer[FUELMOISTURE100][p] < MINMOIST)
				outlayer[FUELMOISTURE100][p] = MINMOIST;
		}
	}
	if(outlayer[FUELMOISTURE1000])
	{
		for(int p = 0; p < numNorth *numEast; p++)
		{
			if(outlayer[FUELMOISTURE1000][p] > 0.0 && outlayer[FUELMOISTURE1000][p] < MINMOIST)
				outlayer[FUELMOISTURE1000][p] = MINMOIST;
		}
	}
}*/

/*double FlamMap::MassageMoisture(double inMoist)
{
	if(inMoist > 0.0 && inMoist < 0.02)
		return 0.02;
	return inMoist;
}*/

bool FlamMap::IsCellBurnable(int row, int col)
{
	int testFuel = GetLayerValueByCell(F_DATA, col, row);
	if(testFuel > 0 && testFuel <= 256 && (testFuel < 90 || testFuel > 99))
		return true;
	return false;
}

void FlamMap::GetNextBinFileName(char trg[])
{
	int num = 1;
	if(strlen(icf.cr_storagePath) > 0 && icf.cr_storagePath[strlen(icf.cr_storagePath) - 1] != '\\')
		strcat(icf.cr_storagePath, "\\");
	//CString ret;// = "1.fmb";

	//CFileFind ff;
	if(strlen(icf.cr_storagePath) > 0)
		sprintf(trg, "%s%ld.fmb", icf.cr_storagePath, num);
	else
		sprintf(trg, "%ld.fmb", num);
	//CA2T szr(trg);  
	while(access(trg, 0) == 0)
	//while(ff.FindFile(szr))
	{
		num++;
		if(strlen(icf.cr_storagePath) > 0)
			sprintf(trg, "%s%ld.fmb", icf.cr_storagePath, num);
		else
			sprintf(trg, "%ld.fmb", num);
	}
	//strcpy(trg, szr);
}

void FlamMap::WriteThreadLayerVal(int Layer, int row, int col, float val)
{
	int64_t nE = this->GetNumEast();//workArea->GetFMPRect().Width() + 1;
	int64_t offset = GetNumEast() * (int64_t)row + (int64_t)col;
	if(outlayer[Layer])
		outlayer[Layer][offset] = val;
	else if((&outFiles[Layer] != NULL && &outFiles[Layer] > 0))//file based
	{

  // fseek (CurrentFile, fpos, SEEK_SET);
		fseek(outFiles[Layer],offset * sizeof(float), SEEK_SET);
	
			//	fwrite(outlayer[Layer], sizeof(float), numEast*numNorth, stream);
fwrite(&val, sizeof(float), 1, outFiles[Layer]);	
//outFiles[Layer].Write(&val, sizeof(float));
	}
}

float FlamMap::GetOutputLayerVal(int Layer, int64_t offset)
{
	//if (Layer == 1 && offset<= 25)
		//printf("return outlayer = %f", outlayer[Layer][offset]);
	if(outlayer[Layer])
		return outlayer[Layer][offset];
	if((outFiles[Layer] != NULL && &outFiles[Layer] > 0))//file based
	{
		//printf("Returning outfile val in Getoutputlayerval\n");
		float val;
		fseek(outFiles[Layer], offset*sizeof(float), SEEK_SET);
		fread(&val, sizeof(float), 1, outFiles[Layer]);		
//outFiles[Layer].Seek(offset * sizeof(float), CFile::begin);
		//outFiles[Layer].Read(&val, sizeof(float));
		return val;
	}
	return NODATA_VAL;
}

/******************************************************************************************/
/******************************************************************************************/
/******************************************************************************************/

/****************************************************************************
* Name: RunConditioning
* Desc: Run the fuel moisture conditioning using the DLL or the original 
*       FlamMap code. This is determined by the switch (or lack of) in 
*       the inputs command file.  
* NOTE: the DLL needs to be initialize with the cfmc.Init() but the cfmc.Delete()
*        is used to cleanup if and error occurs, otherwise the cfmc.Delete()
*        will not get used until after FlamMaps has finishing getting the
*        moisture values back out of the DLL.
*  Ret: 1 ok, 
*       0 error see FlamMap.cr_ErrMes
*      -1 Conditioning run was aborted by user.
*****************************************************************************/
/*int FlamMap::RunConditioning () 
{
int i;
char cr_EM[1000];

  strcpy (this->cr_CondErrMes,""); 

//-------------------------------------------------------------------------
// use DLL to run conditioning 
   i = cfmc.Init ();              // Init the fuel moist condit DLL class obj 
   if ( !FMC_LoadInputs (this, &this->icf, &this->Header, &cfmc, cr_EM)) {
      sprintf (this->cr_CondErrMes, "ERROR Loading Condition DLL Inputs\n --> %s\n", cr_EM); 
      cfmc.Delete();               // Cleanup any alloc'd memory 
      return 0; }

   if ( !cfmc.CheckInputs ()) {
      cfmc.GetErrMes(cr_EM);
      sprintf (this->cr_CondErrMes,"ERROR Invalid/Missing Conditioning DLL Inputs\n --> %s\n",cr_EM);
      cfmc.Delete();                /* cleanup alloc'd memory 
      return 0; }
     
    i = cfmc.Run();  // use the MaxTime in FlamMap obj 
    if ( i == 0 ) {   // run was aborted by user 
      strcpy (this->cr_CondErrMes,"User Requested Termination of Fuel Conditioning");
      return -1; }

   return 1 ; 
}
*/

/*********************************************************************
* Name: FMC_LoadInputs
* Desc: Fuel Moisture Conditioning - Load inputs into the CFMC class
*        object.
*  
*   In: fm....FlamMap object - various input data is in here see below
*       icf...Input Command File object - data from FlamMap inputs file
*       lcp...HeadData - data from input LCP file
*  Out: cfmc..Class Fuel Moisture Conditioning - class for DLL
*  Ret: 1 ok
*       0 error
**********************************************************************/
/*ALM int FlamMap::FMC_LoadInputs ( FlamMap *fm, ICF *icf, headdata *lcp, CFMC *cfmc, char cr_ErrMes[])
{
int i,j,s,StartDate, i_StrYr; 
double d;

   i_StrYr = -1; 

// For now always 1 weather & 1 wind station
   cfmc->SetNumStations (1, 1);     

// NOTE - I'm wondering if this woody stuff ever even gets used ? ? 
   if ( !cfmc->Set_Woody (lcp->woodies, lcp->numwoody, cr_ErrMes) ) 
     return 0;

// FuelModel categories 
   if ( !cfmc->Set_FuelModel (lcp->fuels, lcp->numfuel, cr_ErrMes))
     return 0; 

/*........................................................................
/* Load Fuel Moisture Models from the input command file settings struct 
   for ( i = 0; i < icf->iN_FMD; i++ ) {  /* Find defualt Fuel Model 
     if ( icf->a_FMD[i].i_Model == e_DefFulMod )
       break; }
       
/* Set the default fuel moist model 
   cfmc->Set_DefFuelMoistModel (icf->a_FMD[i].i_TL1,   icf->a_FMD[i].i_TL10,
                                icf->a_FMD[i].i_TL100, icf->a_FMD[i].i_TLLH,
                                icf->a_FMD[i].i_TLLW);  
/* Now set all the fuel moist model 
   for ( i = 0; i < icf->iN_FMD; i++ ) {          /* for each Model             
     if ( icf->a_FMD[i].i_Model == e_DefFulMod )  /* Skip the Default Model     
       continue;     
     cfmc->Set_FuelMoistModel (icf->a_FMD[i].i_Model, icf->a_FMD[i].i_TL1, 
                               icf->a_FMD[i].i_TL10, icf->a_FMD[i].i_TL100, 
                               icf->a_FMD[i].i_TLLH, icf->a_FMD[i].i_TLLW); }
/*..................................................................

   cfmc->Set_Elev(lcp->hielev, lcp->loelev, lcp->EUnits);    /* Unit 1 == feet, 0 = meter 
   cfmc->Set_Slope(lcp->hislope, lcp->loslope, lcp->SUnits); /* Unit 0 = percent, 1 = degrees 
   cfmc->Set_Cover(lcp->hicover, lcp->locover); 
  cfmc->Set_ThreadsProcessor (fm->MaxThreads,fm->GetStartProcessor());  /* # of threads, and starting processor # 

	 	cfmc->ResetThreads();
   cfmc->SetInstanceID(1);  /* make unique for each use of DLL 
   cfmc->SetLatitude(lcp->latitude); 

/*----------------------------------------------------------------------
   if ( this->icf.a_RAWS != NULL ) {   /* Do we have RAWS hourly Weather Stream Data 
     i = FlamMap::Load_RAWS(icf->i_MthStart, icf->i_DayStart,&i_StrYr);         /* Go load it 
     if ( i == 0 ){ 
       strcpy (cr_ErrMes,"Error Allocating Weather Stream memory in Cond DLL"); 
       return 0; }  
//     goto CondDate;
  }                 /* hav Wthr Strm so no Wind/Wthr  
     
/*------------------------------------------------------------------------
/* Load the Wind Data 
  int N = icf->iN_Wnd;   
  if ( N > 0 ) {
    if ( !cfmc->AllocWindData_Sta0 (N) ) {
      strcpy (cr_ErrMes, "Error Allocating Wind Data memory in Cond DLL"); 
      return 0; } }
 
  d_Wnd *w; 
  for ( j = 0; j < N; j++ ) {
//    cfmc->SetWindData_Sta0 (j, fm->wddt[0][j].mo, fm->wddt[0][j].dy, fm->wddt[0][j].hr,fm->wddt[0][j].cl);
//    printf ("%d, %d, %d, %d, %d \n", j, fm->wddt[0][j].mo, fm->wddt[0][j].dy, fm->wddt[0][j].hr,fm->wddt[0][j].cl);

    w = &icf->a_Wnd[j];
    cfmc->SetWindData_Sta0 (j, w->i_Year, w->i_Mth, w->i_Day, w->i_Hr, w->i_CloCov); 
//    printf ("%d, %d, %d, %d, %d \n", j, w->i_Mth, w->i_Day, w->i_Hr, w->i_CloCov); 
 }

/*......................................................................./
/* Load Weather Data                                                  
int obs; 
  obs = icf->iN_Wtr;
  if ( obs > 0 ) {
    if ( !cfmc->AllocWeatherData_Sta0 (obs) ) {  /* allocate space in DLL 
      strcpy (cr_ErrMes, "Error Allocating Weather Data memory in Cond DLL"); 
      return 0; } }
 
  d_Wtr *a;

  for ( int j = 0; j < obs; j++ ) {
     a = &icf->a_Wtr[j];
     if ( a->i_Mth == icf->i_MthStart && a->i_Day == icf->i_DayStart)
       i_StrYr = a->i_Year;

    cfmc->SetWeatherData_Sta0(
//   printf ("%3d, %2d %2d %6.2f %4d %3d %6.2f %6.2f %3d %3d %6.2f %3d %3d\n",
                               j,a->i_Mth,a->i_Day,
                               a->f_Per,
                               a->i_mTH,  /* Min Time  hour 
                               a->i_xTH,  /* Max Time  hour 
                               a->f_mT,   /* min Temp 
                               a->f_xT,   /* max Temp 
                               a->i_xH,   /* max Humid                          
                               a->i_mH,   /* min Humid
                      (float)  a->i_Elv,  /* Elev 
                               a->i_PST,  /* Percip Start Time 
                               a->i_PET); /* Percip End time 
   }                     
                               
/*--------------------------------------------------------------------
/* Conditioning start dates, startdate = julian date 
CondDate:

   if ( i_StrYr == -1 ) {
     strcpy (cr_ErrMes, "Specific starting date not found in weather data");  
     return 0; }
     
   StartDate = GetJulianDays(icf->i_MthStart);
   StartDate += icf->i_DayStart;
   cfmc->Set_DateStart (i_StrYr, icf->i_MthStart, icf->i_DayStart, icf->i_HourStart);   

   d = fm->GetMaxTime (); /* Mintues to run conditioning, start to finish 
   if ( d <= 0 ) {        /* but dates should have been check previoulsly 
      strcpy (cr_ErrMes, "Invalid Conditioning start and/or ending dates");  
      return 0; }
    cfmc->Set_RunTime(d); 

/* Simulation time interval is based on a Fuel Size Class, 1, 10, 100, 1000 hr 
/* the 0 used below will cause the default to get set    
   cfmc->Set_MoistCalcHourInterval (0);
   
/*-------------------------------------------------------------------
/* Set the conditioning underlying Nelson Fuel Moisture Stick Model 
/* If none set than the default model is used - see FMC DLL code 
     cfmc->FuelStickModel_Nelson_070();  /* This is the Orginal one used   
     //cfmc->FuelStickModel_Nelson_100();  /* Nelson 1.0.0 added in Oct 09 

   return 1; 
}
ALM- end commenting out*/
/***************************************************************************
* Name: Load_RAWS   Remote Automate Weather Station 
* Desc: Load the RAWS weather stream data into the Cond DLL
*       
*  Ret: 1 ok, 0 can't allocate mem in DLL
***************************************************************************/
/*ALM int FlamMap::Load_RAWS(int i_MthStr, int i_DayStr, int *ai_Yr)
{
int i; 
d_RAWS *a;
 
   if ( !this->cfmc.RAWS_Allocate(this->icf.iN_RAWS))
     return 0; 

   for ( i = 0; i < this->icf.iN_RAWS; i++ ) {  
      a = &this->icf.a_RAWS[i];  
      if ( i_MthStr == a->i_Mth && i_DayStr == a->i_Day )
         *ai_Yr = a->i_Yr;  
      this->cfmc.RAWS_LoadDate(i,a->i_Yr,a->i_Mth,a->i_Day,a->i_Time);
      this->cfmc.RAWS_LoadInfo(i,a->f_Temp,a->f_Humidity,a->f_PerHou,a->f_CloCov);  }
//  this->cfmc.WSD_Display ();

  this->cfmc.RAWS_SetElev(icf.i_RAWSElev); 
 
  return 1; 
}
ALM */
/*************************************************************************
* Name: GetMaxTime
* Desc: Get the difference in minutes between the Conditioning Start
*        and Ending Date/Time.
*       date/time should be validated before calling this function
**************************************************************************/
/*ALM double FlamMap::GetMaxTime ()
{   
int date,min,StrMin,EndMin, MaxMin; 
double RunTime;

   date = icf.GetMCDate (icf.i_MthStart, icf.i_DayStart, icf.i_YearStart);
   min = icf.MilToMin (icf.i_HourStart); /* Military time to minutes 
   StrMin = (date * 1440) + min;  /* to total minutes  

   date = icf.GetMCDate (icf.i_MthEnd, icf.i_DayEnd, icf.i_YearEnd);
   min = icf.MilToMin (icf.i_HourEnd); 
   EndMin = (date * 1440) + min;  /* to total minutes 

   MaxMin = EndMin - StrMin; 
   RunTime = (double) MaxMin; 

   return RunTime; 
}
ALM */
/************************************************************************
* Name: GetCondErrMes
* Desc: Get any error message that was created when trying to load or
*        check the inputs for the Conditioning DLL, or if the simulation
*        was aborted by user. 
*  Ret: NULL = no error
***********************************************************************/
/*ALM char *FlamMap::Get_CondDLLErrMes ()
{
  return this->cr_CondErrMes; 
}
 */

/******************************************************************************/


int FlamMap::GetStartMonth()
{
   return icf.i_MthStart; //	return startmonth;
}

int FlamMap::GetStartDay()
{
  return icf.i_DayStart; //	return startday;
}

int FlamMap::GetStartHour()
{
  return icf.i_HourStart; //	return starthour;
}

int FlamMap::GetStartMin()
{
  return icf.i_HourStart % 100; //	return startmin;
}

int FlamMap::GetStartDate()
{
int l; 
    l = this->GetJulianDate (icf.i_MthStart,icf.i_DayStart);
    return l; //	return startdate;
}



int FlamMap::GetEndMonth()
{
  return icf.i_MthEnd; // 	return endmonth;
}

int FlamMap::GetEndDay()
{
  return icf.i_DayEnd; //	return endday;
}
/* 0 -> 2359 */ 
int FlamMap::GetEndHour()
{
  return icf.i_HourEnd; //	return endhour;
}

int FlamMap::GetEndDate()
{
int l;
  l = this->GetJulianDate(icf.i_MthEnd, icf.i_DayEnd);  
  return l; //	return enddate;
}

int FlamMap::GetJulianDate(int mth, int day)
{
int l; 
   l = this->GetJulianDays (mth) + day;
   return l;
}

int FlamMap::GetEndMin()
{
int l;
   l = icf.i_HourEnd % 100;    
   return l; //	return endmin;
}


void FlamMap::SetStartMonth(int input)
{
   icf.i_MthStart = input; //	startmonth=input;
}

void FlamMap::SetStartDay(int input)
{
  icf.i_DayStart = input; //	startday=input;
}

/*********************************************************
* 0->2359  
*********************************************************/
void FlamMap::SetStartHour(int input)
{
  icf.i_HourStart = input; // 	starthour=input;
}

/************************************************************/
void FlamMap::SetStartMin(int input)
{
// Current we don't save minutes becuase it doesn't get used  
//	startmin=input;
}

void FlamMap::SetStartDate(int input)
{
// we don't store the date (julian) because FlamMap/Conditiong
//  gets it from the start month and day 
  //	startdate=input;
}


void FlamMap::SetEndMonth(int input)
{
  icf.i_MthEnd = input; //	endmonth=input;
}

void FlamMap::SetEndDay(int input)
{
  icf.i_DayEnd; //	endday=input;
}

/*****************************************************************
* hour = 0 -> 2359
****************************************************************/
void FlamMap::SetEndHour(int input)
{
  icf.i_HourEnd = input; //	endhour=input;
}

void FlamMap::SetEndMin(int input)
{
// Not using minutes 
//	endmin=input;
}

void FlamMap::SetEndDate(int input)
{
// We don't use Juilian date we get it from
//  the month and day when we need it 
//	enddate=input;
}


