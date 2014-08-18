//******************************************************************************
//	FSXWBURN.CPP	Burn model functions for FARSITE
//
//
//				Copyright 1994, 1995, 1996
//				Mark A. Finney, Systems for Environemntal Management
//******************************************************************************
#define BOOL_STR(b) (b?"true":"false")
#include "stdafx.h"

#include "flm4.hpp"
//#include "fsxwattk.h"
//#include "fsairatk.h"
//#include "fsxpfront.h"
#include "flammap3.h"
//#include "fsxsync.h"
//#include <process.h>
#include "FlamMap_DLL.h"
#include <string.h>
#define BURN_THREAD 0
#define SPOT_THREAD 1
static const double PI=acos(-1.0);


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//	BurnThread:: Functions
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

BurnThread::BurnThread ( FlamMap *_pFlamMap)
: MechCalls(_pFlamMap), cf(_pFlamMap), /*prod(_pFlamMap),*/ embers(_pFlamMap)
{
	pFlamMap = _pFlamMap;
	memset(threadSafeFile, 0, NUM_STATICOUTPUTS * sizeof(FILE *));
	FireIsUnderAttack=false;
     ThreadID=0;
     ColStart=RowStart=ColEnd=RowEnd=0;
     turn=0;
     //hBurnThread=0;
     fe=new FELocalSite(pFlamMap);
//     env=Env;
     //firering=0;
     embers.SetFireEnvironmentCalls( fe);
     Started=false;
     DoSpots=false;
     TotalPoints=CuumPoint=0;
}


BurnThread::~BurnThread()
{
//     if(hBurnEvent)
//     	CloseHandle(hBurnEvent);
	DestroyThreadSafeFiles();
	delete fe;
}


/*void BurnThread::SetPerimeterRange(int currentfire, double SimTime, double cuumtimeincrement,
					double timeincrement, double timemaxrem,
                         int begin, int end, int Turn, bool attack, FireRing *Ring)
{
     if(begin>-1)
     {	Begin=begin;
	     End=end;
     }
     CurrentFire=currentfire;
     SIMTIME=SimTime;
     CuumTimeIncrement=cuumtimeincrement;
     TimeMaxRem=timemaxrem;
     TimeIncrement=EventTimeStep=timeincrement;
     turn=Turn;
     FireIsUnderAttack=attack;
     firering=Ring;
     CanStillBurn=StillBurning=false;
     SimTimeOffset=CuumTimeIncrement;
     //if(DistanceCheckMethod(GETVAL)==0)
	//     SimTimeOffset-=GetTemporaryTimeStep();
}*/


void BurnThread::SetLandscapeRange(int colstart, int colend, int rowstart, int rowend)
{
     ColStart=colstart;
     ColEnd=colend;
	 //printf("rowstart = %i  rowend = %i\n", rowstart, rowend);
     RowStart=rowstart;
     RowEnd=rowend;
     SetConstantValues();
}


void BurnThread::SetConstantValues()
{     //stuff from FARSITE
double start, end;

     //SetActualTimeStep(0.0);
     //SetDistRes(50.0);
     //SetDynamicDistRes(50.0);
    // SetPerimRes(50.0);
    // SetVisibleTimeStep(30.0);
     //InitializeRosRed();
     timerem=0.0;

     if (!pFlamMap->ConstFuelMoistures(GETVAL)) {
		 printf("Error:  You are attempting to run fuel moisture conditioning instead running Constant Fuel Moisture\n");  //ALM
		 SIMTIME=0.0;
		 /*
	       start = pFlamMap->GetJulianDays (pFlamMap->GetStartMonth());
        start += pFlamMap->GetStartDay();
        start *= 1440.0;
        start += (pFlamMap->GetStartHour()/100.0)*60.0;
     	  end = pFlamMap->GetJulianDays (pFlamMap->GetEndMonth());
        end += pFlamMap->GetEndDay();
        end *= 1440.0;
        end += (pFlamMap->GetEndHour() / 100.0) * 60.0;
        SIMTIME=end-start; ALM */}
     else
      	SIMTIME=0.0;

}


/*HANDLE BurnThread::StartBurnThread(int ID)
{
	if(Started==false)//hBurnThread==0)
	{    ThreadOrder=ID;
          hBurnEvent=pFlamMap->GetFarsiteEvent(EVENT_BURN_THREAD, ThreadOrder);
        	hBurnThread=(HANDLE) _beginthreadex(NULL, 0, &BurnThread::RunBurnThread, this, CREATE_SUSPENDED, &ThreadID);
          ResumeThread(hBurnThread);
          CloseHandle(hBurnThread);
          hBurnThread=0;
     }
     else
         SetEvent(hBurnEvent);//SetFarsiteEvent(EVENT_BURN_THREAD, ThreadOrder);

     return hBurnThread;
}

*/
//ALM HANDLE BurnThread::StartLandscapeThread(int ID)
void BurnThread::StartLandscapeThread(int ID)
{
	//printf("in StartLandscapeThread\n");
     BurnTheLandscape();    // non-threaded version
	//printf("in StartLandscapeThread2\n");
	/*if(Started==false)
	{    
		printf("in StartLandscapeThread3\n");
		CreateThreadSafeFiles();}
		/*
		ThreadOrder=ID;
          hBurnEvent=pFlamMap->GetFarsiteEvent(EVENT_BURN_THREAD, ThreadOrder);
          SYSTEM_INFO sysinf;
          GetSystemInfo(&sysinf);
          int NumTimes=(int) (ThreadOrder/sysinf.dwNumberOfProcessors);
          //int ProcNum=ThreadOrder-NumTimes*sysinf.dwNumberOfProcessors;
		  int ProcNum = pFlamMap->GetStartProcessor() + ThreadOrder;
		  while(ProcNum >= sysinf.dwNumberOfProcessors)
			  ProcNum -= sysinf.dwNumberOfProcessors;
        	hBurnThread=(HANDLE) _beginthreadex(NULL, 0, &BurnThread::RunLandscapeThread, this, CREATE_SUSPENDED, &ThreadID);
		//SetThreadIdealProcessor(hBurnThread, ProcNum);
			unsigned int Affinity = pow(2.0, (int)ProcNum);
			SetThreadAffinityMask(hBurnThread, Affinity);
          ResumeThread(hBurnThread);
          CloseHandle(hBurnThread);
          hBurnThread=0;
     }
     else
         SetEvent(hBurnEvent);//SetFarsiteEvent(EVENT_BURN_THREAD, ThreadOrder);
    
     return hBurnThread;
	 */
}

/*
HANDLE BurnThread::GetThreadHandle()
{
	return hBurnThread;
}
*/

/*unsigned __stdcall BurnThread::RunBurnThread(void *bt)
{
	static_cast <BurnThread *> (bt)->PerimeterThread();

     return 1;
}*/

unsigned int BurnThread::RunLandscapeThread(void *bt)
{
	//printf("in RunLandscapeThread");
	static_cast <BurnThread *> (bt)->BurnTheLandscape();

     return 1;
}


void BurnThread::BurnTheLandscape()
{
	//if (pFlamMap->outFiles[0] != NULL)printf("pFlamMap->outFiles[layerNum] is open\n");
	//printf("in BurntheLandscape\n");
	int j, k;
     double MaxSpotDist;//, SpotSpreadRate, BSR;
	 //printf("In BurnTheLandscape TerminateBurn = %s\n", BOOL_STR(pFlamMap->TerminateBurn));
     for(k=RowStart; k<RowEnd && !pFlamMap->TerminateBurn; k++)
     {
		 printf(".");
		//printf("k = %i  rowend = %i\n", k, RowEnd);
		if(k>pFlamMap->GetNumNorth()-1)
     	{	k=pFlamMap->GetNumNorth()-1;
		//printf("In BurnTheLandscape TerminateBurn1\n");
          }
     	for(j=ColStart; j<ColEnd && !pFlamMap->TerminateBurn; j++)
          {    
			  //printf("In BurnTheLandscape TerminateBurn2\n");
			  if(j>pFlamMap->GetNumEast()-1)
                   // k=pFlamMap->GetNumEast()-1;//changed 2009/04/23 SB
                    j=pFlamMap->GetNumEast()-1;
          //--------------------------------------------------------------------
          // initialize output variables in case of unburnable stuff
          //--------------------------------------------------------------------
			 //printf("In BurnTheLandscape TerminateBurn3\n");
          	RosT=RosT1=FliFinal=HSpreadRate=0.0;
               vecdir=head=flank=back=0.0;
               CrownState=0;
			   embers.MaxSpot = 0.0;
			   embers.NumEmbers = 0;
			   HorizSpread = 0.0;
			   vecdir = NODATA_VAL;
			   //embers.n
			   //printf("In BurnTheLandscape TerminateBurn4\n");
          //--------------------------------------------------------------------
          	GetPoints(k, j);					// reconstruct orientations
			//printf("In BurnTheLandscape TerminateBurn5 x = %f y = %f k = %i j = %i\n", xpt, ypt, k, j);
               fe->GetLandscapeData(xpt, ypt, ld);     // Burn::GetLandscapeData();
			  // printf("In BurnTheLandscape TerminateBurn6\n");
               fe->GetFireEnvironment( SIMTIME, true);
			  // printf("In BurnTheLandscape TerminateBurn7\n");
 				//double tempX = xpt, tempY = ypt;
				EmberCoords();                     // compute ember source coordinates
				//printf("In BurnTheLandscape TerminateBurn8\n");
              SurfaceFire();
			  //printf("In BurnTheLandscape TerminateBurn9 fuel = %i, cover = %i  height = %f\n", ld.fuel,ld.cover,ld.height);
               if(ld.fuel>0 && ld.cover>0 && ld.height>0.0)	 	// if there IS a tree canopy
          	{
				//EmberCoords();                     // compute ember source coordinates
				//printf("In BurnTheLandscape TerminateBurn10\n");
          		CrownFire();
				//printf("In BurnTheLandscape TerminateBurn11\n");

				   if(embers.NumEmbers>0)
				{  	
					//printf("In BurnTheLandscape TerminateBurn12\n");
					embers.Flight(SIMTIME, SIMTIME+120.0);      // curent time to Flight is iter*actual, but really (iter+1)*actual
					//printf("In BurnTheLandscape TerminateBurn13\n");
						MaxSpotDist=embers.GetMaxSpotDist(xpt, ypt);
						//printf("In BurnTheLandscape TerminateBurn14\n");
						//MaxSpotDist=embers.GetMaxSpotDist(tempX, tempY);

						//SpotSpreadRate=MaxSpotDist/10.0;
						//if(SpotSpreadRate>RosT1)
						//{    RosT1=SpotSpreadRate;
						//     BSR=2.0*head-(head+back);
						//     head=SpotSpreadRate/2.0;
						//     back=head-BSR;
						//}
				   }
               }
			   //printf("In BurnTheLandscape TerminateBurn15 k = %i  j = %i\n", k, j);
               WriteOutputLayers(k, j);
			  // printf("In BurnTheLandscape TerminateBurn16\n");
          }
          // update progress every row, but could do it every iteration
         //ALM pFlamMap->SetThreadProgress(ThreadOrder, (double) (k-RowStart)/(double) (RowEnd-RowStart-1));
         // if(ThreadOrder==0)
               //printf("row finished %ld of %ld\n", k*GetMaxThreads(), RowEnd*GetMaxThreads());
     }
     //printf("Rows finished, Thread %ld/%ld \n", ThreadOrder+1, GetMaxThreads());
     //pFlamMap->SetFarsiteEvent(EVENT_BURN, ThreadOrder);
	//DestroyThreadSafeFiles();
	// printf("In BurnTheLandscape TerminateBurn17\n");
	 printf("\nBurn Complete\n");
}


/*void BurnThread::PerimeterThread()
{
     int 	begin, end;
     int		i, TestPointL, TestPointN;
     bool 	ComputeSpread;				//=true;
	double 	oldfli;
     double	FliL, FliN;
     double 	mx[20];			// fuel moistures

     begin=Begin;
     end=End;
     Started=true;	// means that the thread has been started
	do
     {    if(End<0)
     		break;
     	if(CurrentFire>=0)
          {	if(turn==0)
     			distchek(CurrentFire);
			else if(DistanceCheckMethod(GETVAL)==0)
     			distchek(CurrentFire);
          }
     	for(CurrentPoint=begin; CurrentPoint<end; CurrentPoint++)	// FOR ALL POINTS ON EACH FIRE
		{    timerem=TimeIncrement;
			GetPoints(CurrentFire, CurrentPoint);	   	// Burn::GetPoints();
			fe->GetLandscapeData(xpt, ypt, ld);      		    	// Burn::GetLandscapeData();
			if(ld.fuel>0)// && fli>=0.0)				// if not a rock or lake etc.
			{    fli=FliFinal=oldfli=GetPerimeter2Value(CurrentPoint, FLIVAL);
               	if(fli<0.0)
               	{    StillBurning=true;
                         if(FireIsUnderAttack)
                    	{    if(turn)
                              {    ComputeSpread=false;
                         	     for(i=1; i<3; i++)
                         	     {	TestPointL=CurrentPoint-i;
							     TestPointN=CurrentPoint+i;
                                        if(TestPointL<0)
                                   	     TestPointL+=GetNumPoints(CurrentFire);
                                        if(TestPointN>=GetNumPoints(CurrentFire))
                                   	     TestPointN-=GetNumPoints(CurrentFire);
	                                   FliL=GetPerimeter2Value(TestPointL, FLIVAL);
     	                              FliN=GetPerimeter2Value(TestPointN, FLIVAL);
          	                         if(FliL>0.0 || FliN>0.0)
	          	                    {	ComputeSpread=true;
                                             break;
                                        }
                                   }
                              }
                              else
                                   ComputeSpread=true;
                         }
                    	else
                         	ComputeSpread=false;
                    }
                    else
                    	ComputeSpread=true;
				if(ComputeSpread)
                    {	EmberCoords();                     // compute ember source coordinates
					CanStillBurn=true;				// has fuel, can still burn

                         fe->GetFireEnvironment( SIMTIME+SimTimeOffset, false);
					if(EventMinimumTimeStep(GETVAL)<EventTimeStep && EventMinimumTimeStep(GETVAL)>0.0)
						EventTimeStep=EventMinimumTimeStep(GETVAL);

     	               NormalizeDirectionWithLocalSlope();     // mechcalls function to transfrm orientation of point with slope
					RosT=GetPerimeter2Value(CurrentPoint, ROSVAL);    // get forward ROS from last timestep or last substep
					SurfaceFire();
                         if(oldfli<0.0)
                         {    fli*=-1.0;
                              FliFinal*=-1.0;
                              react=0.0;
                         }
					if(ld.cover>0 && ld.height>0)	 	// if there IS a tree canopy
						CrownFire();    				// if turn==1 then start try spot fires, else no
                         else
                         	cf.CrownLoadingBurned=0.0;
					if(oldfli>=0.0 && timerem>0.001 && timerem>TimeMaxRem)	// tolerance for simulation 1/1000 min
						TimeMaxRem=timerem;				// LARGEST TIME REMAINING, MEANS FASTEST SPREAD RATE
				}
                    else
                    	RosT1=RosT=GetPerimeter2Value(CurrentPoint, ROSVAL);
               }
			else //if(fuel==0)
			{	//timerem=0.0;
				RosT=RosT1=0.0;
				fli=FliFinal=0.0;
                    react=0.0;
			}
			if(turn==1)
			{    if(FireIsUnderAttack && !ComputeSpread)
                         FliFinal*=-1.0;
               	prod.cuumslope[0]+=(double) ld.slope;     	// CUMULATIVE SLOPE ANGLES
				prod.cuumslope[1]+=1;				// CUMULATIVE NUMBER OF PERIMETER POINTS
				SetPerimeter1(CurrentFire, CurrentPoint, xpt, ypt);
				SetFireChx(CurrentFire, CurrentPoint, RosT1, FliFinal);
				SetReact(CurrentFire, CurrentPoint, react);

				if(CuumTimeIncrement==0.0)   // beginning of timestep
	                    SetElev(CurrentPoint, ld.elev);

			     if(DistanceCheckMethod(GETVAL)==0 && CheckPostFrontal(GETVAL))          	// store current fire perim in ring
		          {	pFlamMap->GetCurrentFuelMoistures(ld.fuel, ld.woody, (double*) &gmw, mx, 7);
		          	AddToCurrentFireRing(firering, CurrentPoint, ld.fuel, ld.woody, ld.duff, mx, cf.CrownLoadingBurned*1000.0);
		          }
			}
		}
		pFlamMap->SetFarsiteEvent(EVENT_BURN, ThreadOrder);		//hSimEvent[ThreadOrder]
		WaitForSingleObject(hBurnEvent, INFINITE);
          ResetEvent(hBurnEvent);
          if(End<0)
     		break;

          prod.cuumslope[0]=0.0;	// reset after they have been read by burn::
          prod.cuumslope[1]=0;
          begin=Begin;	// restore local copies from Class data
          end=End;
          if(DoSpots)
          {  	CurrentFire=-1;
               if(embers.NumEmbers>0)
			{  	embers.Flight(SIMTIME, SIMTIME+GetActualTimeStep());      // curent time to Flight is iter*actual, but really (iter+1)*actual
				if(embers.NumSpots>0 && EnableSpotFireGrowth(GETVAL))
					embers.Overlap();		   // check ember resting positions for overlap with burned areas
			}
               begin=0;	// go back to the begining
               end=0;
               DoSpots=false;
          }
     } while(End>-1);

     pFlamMap->SetFarsiteEvent(EVENT_BURN, ThreadOrder);
}
*/

void BurnThread::SurfaceFire(void)
{// does surface fire calculations of spread rate and intensity
     CrownState=0;
	LoadGlobalFEData(fe);
     if(ld.fuel<=0)
          return;
	fros=spreadrate(ld.slope, m_windspd, ld.fuel);
	if(fros>0.0)				    	// if rate of spread is >0
	{	GetAccelConst();              // get acceleration constants
		VecSurf();		 	    	// vector wind with slope
          CalculateFireOrientation(pFlamMap->GetOffsetFromMax());
		NormalizeDirectionWithLocalSlope();
		ellipse(vecros, ivecspeed);  	// compute elliptical dimensions
		grow(vecdir);  		    	// compute time derivatives xt and yt for perimeter point
		AccelSurf1();                 // compute new equilibrium ROS and Avg. ROS in remaining timestep
		SlopeCorrect(1);
		SubTimeStep=timerem;
		limgrow();   			    	// limits HORIZONTAL growth to mindist
		AccelSurf2();			    	// calcl ros & fli for SubTimeStep Kw/m from BTU/ft/s*/
          CrownState=1;
	//	if(SSpotOK && cover==99)
	//	{    cf.FlameLength=0;		// may want to set other crown parameters to 0 here
	//		SpotFire(3);	   	 	// spotting from  winddriven surface fires
	//	}
	}
	else
	{    timerem=0;
		RosT=0;
		fli=0;
		FliFinal=0;
	}
}


void BurnThread::CrownFire(void)
{// does crown fire calculations of spread rate and intensity (Van Wagner's)
	//double FabsFli=fabs(fli);
    //printf("in CrownFire\n");
	double FabsFli=3.4613*(384.0*(react/0.189275)*(ExpansionRate/0.30480060960)/(60.0*savx));	// ending forward fli in timestep
     double ExpRate=ExpansionRate;
     double h, f, b, d;

	if(pFlamMap->EnableCrowning(GETVAL))			   		   // global flag for disabling crowning
	{	if(FabsFli>0.0)                        // use average fli
			cf.CrownIgnite(ld.height, ld.base, ld.density);           // calculate critical intensity from van Wagner
		else
			cf.Io=1;
		if(FabsFli>=cf.Io)			   // if sufficient energy to ignite crown
		{	h=head;
               b=back;
               f=flank;
               d=vecdir;
			CrownState+=1;
          	if(pFlamMap->GetCrownFireCalculation()==0)
			{	cf.CrownBurn(ExpRate, FabsFli, A);
				R10=spreadrate(ld.slope, m_twindspd*0.4, 10);
               }
     		else
				R10=cf.CrownBurn2(ExpRate, FabsFli, A, this);
               if(R10==0.0)
				R10=avgros;
               VecCrown();                 // get vectored crown spread rate
			CalculateFireOrientation(pFlamMap->GetOffsetFromMax());
     		NormalizeDirectionWithLocalSlope();
               ellipse(vecros, ivecspeed); // determine elliptical dims with vectored winds not m_twindspd
			grow(vecdir);    	   	   // compute time derivatives xt & yt for perimeter pt.
			SpreadCorrect();   	   	   // correct vecros for directional spread
               cros=cf.CrownSpread(ExpRate, ExpansionRate);	// wind-driven crown fire rate of spread Rothermel 1991
               if(cros>0.0)			   // if active crown fire
			{	if(cros>=3.0/ld.density)
					CrownState+=1;
               	timerem=SubTimeStep;   // reset timerem again to before surface spread began in last substep
				A=cf.A;                // use crown fire acceleration rate from Crown::CrownBurn()
				AccelCrown1();
				SlopeCorrect(0);
				limgrow();   		   // limits HORIZONTAL growth to mindist
				AccelCrown2();
				SubTimeStep=SubTimeStep-timerem;  // actual subtimestep after crowning
				cf.FlameLength=0;
				cf.CrownIntensity(cros, &fli);    // calc flamelength and FLI from crownfire
				//	AVGFlameLength=AVGFlameLength+FlameLength/SubTimeStep;  // fl/unittime
				//   CrownDuration=CrownDuration+SubTimeStep;                // total time spent crowning
				if(pFlamMap->EnableSpotting(GETVAL) && FliFinal>0.0)    // no spotting from dead perimeter
					SpotFire(0);      // spotting from active crown fire
			}
			else                        // if passive crown fire
               {    cf.CrownIntensity(avgros, &fli);    // calc flamelength and FLI for passive crown fire
                    // restore elliptical dimensions and direction if not crown fire
                    head=h;
                    back=b;
                    flank=f;
                    vecdir=d;
                    if(pFlamMap->EnableSpotting(GETVAL) && FliFinal>0.0)    // no spotting from dead perimeter
     			{    cf.FlameLength=0;
	     			SpotFire(1);      // spotting from torching trees
                    }
			}
               if(fli>fabs(FliFinal))
			{    if(FliFinal<0.0)
					FliFinal=fli*-1.0;
				else
					FliFinal=fli;
			}
		}
          else
          	cf.CrownLoadingBurned=0.0;	// need to set because of post-frontal
	}
     else
         	cf.CrownLoadingBurned=0.0;	// need to set because of post-frontal
}



void BurnThread::SpotFire(int SpotSource)
{// calls spot fire functions for lofting embers

	if(fli>0.0)                                    // if check method 2 and second pass
	{	if(turn==1)
			CurrentTime=CuumTimeIncrement+SIMTIME;   	// timerem will be ~zero with method 2
		else
			CurrentTime=(0.0-timerem)+SIMTIME;    	// timerem will represent time remaining in actual TS
			//CurrentTime=(GetActualTimeStep()-timerem)+SIMTIME;    	// timerem will represent time remaining in actual TS
		embers.SpotSource=SpotSource;
		embers.Loft(cf.FlameLength, cf.CrownFractionBurned, ld.height, cf.CrownLoadingBurned, HorizSpread, SubTimeStep, CurrentTime);
	}
}


void BurnThread::EmberCoords()
{// copies coordinates for perimeter segment that generates embers
	embers.Fcoord.x=xpt;
	embers.Fcoord.y=ypt;			// STORE PERIMETER SEGMENT FOR SPOTTING
	embers.Fcoord.xl=xptl;
	embers.Fcoord.xn=xptn;
	embers.Fcoord.yl=yptl;
	embers.Fcoord.yn=yptn;			// natural units (english or metric grid units)
	embers.Fcoord.e=(double) ld.elev;     // meters
	embers.Fcoord.cover=ld.cover;         // transfer cover to
}



double BurnThread::GetLastValue(int Layer)
{
	//printf ("in GetLastValue %i\n", Layer);
     double value;
	 //printf("in getlastvalue1 layer = %i", Layer);
	switch(Layer)
     {	case FLAMELENGTH:
                    if(FliFinal>0.0)
                    {    if(CrownState<2)
					     value=0.0775*pow(FliFinal, 0.46);			// flamelength and hpua
                         else
                              value=((0.2*pow(FliFinal/3.4613,2.0/3.0))/3.2808);
                    }
                    else
                    	value=0.0;
     		break;
     	case SPREADRATE:
              		value=RosT1;
          	break;
          case INTENSITY:
               	value=FliFinal;
				//printf("in GestLastValue Intensity = %f\n", value);
          	break;
          case HEATAREA:
                    if(RosT1>0.0)
			   		value=(60.0*FliFinal)/RosT1;
                    else
                    	value=0.0;
          	break;
          case CROWNSTATE:
          		value=(double) CrownState;
          	break;
          case SOLARRADIATION:
          		value=gmw.solrad;//SolarRadiation;
          	break;
          case FUELMOISTURE1:
          		value=gmw.ones;//gmw.ones;
          	break;
          case FUELMOISTURE10:
          		value=gmw.tens;//gmw.tens;
          	break;
          case FUELMOISTURE100:
          		value=gmw.hundreds;//gmw.ones;
          	break;
          case FUELMOISTURE1000:
          		value=gmw.thousands;//gmw.tens;
          	break;
          case MIDFLAME:
          		value=gmw.windspd;//gmw.windspd;
          	break;
          case HORIZRATE:
          		value=HorizSpread;//HSpreadRate;//cosslope*RosT1;
				if(value < 0.0)
					value = 0.0;
          	break;
		  case MAXSPREADDIR:
			  if(vecdir == NODATA_VAL)
				  value = vecdir;
			  else
			  {
				  value=PI+vecdir;
				  if(value>PI*2.0)
					  value-=PI*2.0;
			  }
			  break;
          case ELLIPSEDIM_A:
          		value=flank;
          	break;
          case ELLIPSEDIM_B:
          		value=head;
          	break;
          case ELLIPSEDIM_C:
          		value=back;//cosslope*RosT1;
          	break;
          case MAXSPOT:
                    value=embers.MaxSpot;
					embers.MaxSpot = 0.0;
               break;
     }
	//printf("returning layer = %i value = %f\n", Layer, value);
     return value;
}

void BurnThread::CreateThreadSafeFiles()
{
	printf("in CreateThreadSafeFiles1");
	FILE * threadSafeFile[NUM_STATICOUTPUTS];
	printf("in CreateThreadSafeFiles2");
	for(int i = 0; i < NUM_STATICOUTPUTS; i++)
	{
		if(!pFlamMap->outlayer[i] && &pFlamMap->outFiles[i] > 0 && &pFlamMap->outFiles[i] != NULL )
		{
			printf("in CreateThreadSafeFiles3");
			threadSafeFile[i] = fopen((const char *) pFlamMap->outFiles[i], "w+");
//CFile::modeReadWrite | CFile::typeBinary | CFile::shareDenyNone);
			//threadSafeFile[i]->SetStatus(;
		}
	}
	printf("in CreateThreadSafeFiles4");
}

void BurnThread::DestroyThreadSafeFiles()
{
	for(int i = 0; i < NUM_STATICOUTPUTS; i++)
	{
		if(threadSafeFile[i])
		{
			if(threadSafeFile[i] != NULL)
				fclose(threadSafeFile[i]);
			remove((const char *)threadSafeFile[i]);
			threadSafeFile[i] = NULL;
		}
	}
}

void BurnThread::WriteValToFile(int layerNum, int64_t pos)
{
	//if(threadSafeFile[layerNum])
	printf("writevaltofilepre %i  %i\n", pFlamMap->outFiles[layerNum], layerNum);
	if(pFlamMap->outFiles[layerNum])
	{
		//printf("writevaltofilepreprea\n");
		float f = GetLastValue(layerNum);
		//printf("writevaltofileprepreb\n");
		if(layerNum == FUELMOISTURE1
			|| layerNum == FUELMOISTURE10
			|| layerNum == FUELMOISTURE100
			|| layerNum == FUELMOISTURE1000)
		{
			//printf("writevaltofilepreprec\n");
			f = pFlamMap->MassageMoisture(f);
			//printf("writevaltofileprepred\n");
		}
		//EnterCriticalSection(&pFlamMap->outputCS);
		
		//pFlamMap->outFiles[layerNum].Seek(pos * sizeof(float), CFile::begin);
		//pFlamMap->outFiles[layerNum].Write(&f, sizeof(float));
		//pFlamMap->outFiles[layerNum].Flush();
		//ALM
		//printf("writevaltofile1 %i  %i\n",sizeof(threadSafeFile[layerNum]), pos * sizeof(float));
		//if (pFlamMap->outFiles[layerNum] != NULL)printf("pFlamMap->outFiles[layerNum] is opena\n");
		//fseek(threadSafeFile[layerNum], pos * sizeof(float), SEEK_SET);
		fseek(pFlamMap->outFiles[layerNum], pos * sizeof(float), SEEK_SET);
		//fseek(pFlamMap->outlayer[layerNum], pos * sizeof(float), SEEK_SET);
		//pFlamMap->outFiles[layerNum] = NULL;
		//printf("writevaltofile2\n");
		//threadSafeFile[layerNum]->Seek(pos * sizeof(float), CFile::begin);
		//fwrite(&f, sizeof(float), 1, threadSafeFile[layerNum]);
		//fputs(&f, pFlamMap->outFiles[layerNum]);
		//printf("writing file layerNum = %i", layerNum);
		fwrite(&f, sizeof(float), 1, pFlamMap->outFiles[layerNum]);
		//printf("writevaltofile3\n");
		//threadSafeFile[layerNum]->Write(&f, sizeof(float));
		//LeaveCriticalSection(&pFlamMap->outputCS);
		//threadSafeFile[layerNum]->Flush();
	}
}

void BurnThread::WriteOutputLayers(int row, int col)

{
	if (pFlamMap->outFiles[0] != NULL)printf("pFlamMap->outFiles[layerNum] is open\n");
	//printf("in writeoutputlayers1\n");
	if(ld.fuel==-3)
     	return;
	//printf("in writeoutputlayers2\n");
	float *layer;

	if(ld.slope>35)
		ld.slope*=1;
	//printf("in writeoutputlayers3\n");
	int nE = pFlamMap->analysisRect.Width(), loc;
	//printf("in writeoutputlayers4 geteast  = %i  row = %i  col = %i\n", pFlamMap->GetNumEast(), row, col);
	//int nE = pRun->workArea->GetFMPRect().Width() + 1, loc;
	int64_t offset = (int64_t)pFlamMap->GetNumEast() * (int64_t)row + (int64_t)col;
	loc = row*nE + col;
	//printf("in writeoutputlayers5\n");
	for(int i = FLAMELENGTH; i < NUM_STATICOUTPUTS; i++)
	{
		//printf("in writeoutputlayers6\n");
		if(pFlamMap->GetOutputOption(i))
		{	
			//printf("in writeoutputlayers7\n");
			//pFlamMap->WriteThreadLayerVal(i, row, col, GetLastValue(i))
			layer=pFlamMap->outlayer[i];
			if(layer)
			{
				//printf("in writeoutputlayers8\n");
				layer[offset]=GetLastValue(i);
				//printf("in writeoutputlayers9 layer[offset] = %f pFlamMap->outlayer[i] = %f\n", layer[offset], pFlamMap->outlayer[i]);
				if(i == FUELMOISTURE1
					|| i == FUELMOISTURE10
					|| i == FUELMOISTURE100
					|| i == FUELMOISTURE1000)
				{
					//printf("in writeoutputlayers10\n");
					layer[offset] = pFlamMap->MassageMoisture(layer[offset]);
				}
			}
			else{
				//printf("in writeoutputlayers11 offset = %i\n", offset);
				WriteValToFile(i, offset);
				//printf("in writeoutputlayers12\n");
			}
		}
	}
	//printf("in writeoutputlayers13\n");
/*	if(pFlamMap->GetOutputOption(FLAMELENGTH))
	{	
		layer=pFlamMap->GetThreadLayer(FLAMELENGTH, row);
		if(layer)
		{
			layer[col]=GetLastValue(FLAMELENGTH);
			}
		else
		{
			WriteValToFile(FLAMELENGTH, loc);
		}

	}
	if(pFlamMap->GetOutputOption(SPREADRATE))
     {	layer=pFlamMap->GetThreadLayer(SPREADRATE, row);
     	layer[col]=GetLastValue(SPREADRATE);
     }
	if(pFlamMap->GetOutputOption(INTENSITY))
     {	layer=pFlamMap->GetThreadLayer(INTENSITY, row);
     	layer[col]=GetLastValue(INTENSITY);
     }
	if(pFlamMap->GetOutputOption(HEATAREA))
     {	layer=pFlamMap->GetThreadLayer(HEATAREA, row);
     	layer[col]=GetLastValue(HEATAREA);
     }
	if(pFlamMap->GetOutputOption(CROWNSTATE))
     {	layer=pFlamMap->GetThreadLayer(CROWNSTATE, row);
     	layer[col]=GetLastValue(CROWNSTATE);
     }
	if(pFlamMap->GetOutputOption(SOLARRADIATION))
     {	layer=pFlamMap->GetThreadLayer(SOLARRADIATION, row);
     	layer[col]=GetLastValue(SOLARRADIATION);
     }
	if(pFlamMap->GetOutputOption(FUELMOISTURE1))
     {	layer=pFlamMap->GetThreadLayer(FUELMOISTURE1, row);
     	layer[col]=pFlamMap->MassageMoisture(GetLastValue(FUELMOISTURE1));
     }
	if(pFlamMap->GetOutputOption(FUELMOISTURE10))
     {	layer=pFlamMap->GetThreadLayer(FUELMOISTURE10, row);
     	layer[col]=pFlamMap->MassageMoisture(GetLastValue(FUELMOISTURE10));
     }
	if(pFlamMap->GetOutputOption(FUELMOISTURE100))
     {	layer=pFlamMap->GetThreadLayer(FUELMOISTURE100, row);
     	layer[col]=pFlamMap->MassageMoisture(GetLastValue(FUELMOISTURE100));
     }
	if(pFlamMap->GetOutputOption(FUELMOISTURE1000))
     {	layer=pFlamMap->GetThreadLayer(FUELMOISTURE1000, row);
     	layer[col]=pFlamMap->MassageMoisture(GetLastValue(FUELMOISTURE1000));
     }
	if(pFlamMap->GetOutputOption(MIDFLAME))
     {	layer=pFlamMap->GetThreadLayer(MIDFLAME, row);
     	layer[col]=GetLastValue(MIDFLAME);
     }
	if(pFlamMap->GetOutputOption(HORIZRATE))
     {	layer=pFlamMap->GetThreadLayer(HORIZRATE, row);
     	layer[col]=GetLastValue(HORIZRATE);
     }
	if(pFlamMap->GetOutputOption(MAXSPREADDIR))
     {	layer=pFlamMap->GetThreadLayer(MAXSPREADDIR, row);
     	layer[col]=GetLastValue(MAXSPREADDIR);
     }
	if(pFlamMap->GetOutputOption(ELLIPSEDIM_A))
     {	layer=pFlamMap->GetThreadLayer(ELLIPSEDIM_A, row);
     	layer[col]=GetLastValue(ELLIPSEDIM_A);
     }
	if(pFlamMap->GetOutputOption(ELLIPSEDIM_B))
     {	layer=pFlamMap->GetThreadLayer(ELLIPSEDIM_B, row);
     	layer[col]=GetLastValue(ELLIPSEDIM_B);
     }
	if(pFlamMap->GetOutputOption(ELLIPSEDIM_C))
     {	layer=pFlamMap->GetThreadLayer(ELLIPSEDIM_C, row);
     	layer[col]=GetLastValue(ELLIPSEDIM_C);
     }
	if(pFlamMap->GetOutputOption(MAXSPOT))
     {	layer=pFlamMap->GetThreadLayer(MAXSPOT, row);
     	layer[col]=GetLastValue(MAXSPOT);
     }*/
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//	Burn:: Functions
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


Burn::Burn (FlamMap *_pFlamMap) // : Intersections(_pFlamMap)  // , prod(_pFlamMap)
{
     //InitialAllocFirePerims();
     //CreateSpotSemaphore();
//     SIMTIME=0;
//     CuumTimeIncrement=0.0;
//     TotalPoints=0;

// Shit - I added this 
     pFlamMap = _pFlamMap;

     burnthread=0;


     NumPerimThreads=0;
     fe=new FELocalSite(_pFlamMap);
//      env=new FireEnvironment2(_pFlamMap);
     //embers.SetFireEnvironmentCalls(env, fe);
     NumSpots=0;
 
// Shit this was in the Intersection that I took out of Burn Class 
//    ExNumPts=0;
}


Burn::~Burn()
{
	ResetAllPerimThreads();
    // CloseSpotSemaphore();
     //FreeAllFirePerims();
     //FreeElev();
     delete fe;

/* Add this line to make sure pointer isn't null when the */
/* Moisture Conditioning stuff was added to FSPRo */
//     if ( env != NULL ) 
//        delete env;
}




void Burn::ResetAllPerimThreads()
{
	CloseAllPerimThreads();
}

/*********************************************************/
bool Burn::AllocPerimThreads()
{
	 if (NumPerimThreads==pFlamMap->GetMaxThreads())
     	return true;

	 CloseAllPerimThreads();
	 burnthread=(BurnThread **) new BurnThread*[pFlamMap->GetMaxThreads()];

  int i;
  for (i=0; i<64; i++)
     pFlamMap->SetThreadProgress(i, 0.0);

  if (burnthread) {  
    NumPerimThreads=pFlamMap->GetMaxThreads();
	   NumSpots=new int[NumPerimThreads];
    for (i=0; i<NumPerimThreads; i++)
       burnthread[i]=new BurnThread( pFlamMap);
		  return true;
  }

  return false;
}


void Burn::CloseAllPerimThreads()
{
	int m;

     if(NumPerimThreads==0)
     	return;

	for(m=0; m<NumPerimThreads; m++)
          burnthread[m]->SetLandscapeRange(0, 0, 0, 0);
          //burnthread[m]->SetPerimeterRange(0, 0.0, 0.0, 0.0, 0.0, 0, -1, 0, false, 0);
//   for(m=0; m<NumPerimThreads; m++)
//	     	burnthread[m]->StartBurnThread(m);
//	Sleep(50);
//     WaitForFarsiteEvents(EVENT_BURN, NumPerimThreads, true, INFINITE);//2000);
     FreePerimThreads();
}

void Burn::FreePerimThreads()
{
     int i;

	if(burnthread)
     {	for(i=0; i<NumPerimThreads; i++)
	    		delete burnthread[i];
     	delete[] burnthread;//GlobalFree(burnthread);
          if(NumSpots)
	          delete[] NumSpots;
     }

     burnthread=0;
     NumPerimThreads=0;
     NumSpots=0;
}

void Burn::Factor(int number, int *x, int *y)
{    // finds largest factors of integer number

     int X, Y;
     double mid, fract, ipart;

     mid=sqrt(((double) number));

     X=0;
     do
     {    fract=modf(((double) number)/((double) (++X)), &ipart);
          if(fract==0.0)
               Y=ipart;
     }while((double) X<mid);
     X=number/Y;

     *x=X;
     *y=Y;
}

/***********************************************************/
int Burn::StartLandscapeThreads()
{
int i, j, NumX, NumY;//, Ret;
int threadct, XRange, YRange;//, Xprocs, Yprocs;
int ColStart, ColEnd, RowStart, RowEnd;
	//printf("in StartLandscapeThreads\n");
	//if (pFlamMap->outFiles[0] != NULL)printf("pFlamMap->outFiles[layerNum] is openb\n");
	 AllocPerimThreads();

// find number of threads aint x-axis and y-axis
  Factor(NumPerimThreads, &NumX, &NumY);
  XRange=ceil((double) pFlamMap->GetNumEast()/(double) NumX);
  YRange=ceil((double) pFlamMap->GetNumNorth()/(double) NumY);
 
  RowStart=RowEnd=threadct=0;

  for (i=0; i<NumY; i++) { 
	  //printf("in StartLandscapeThreads  RowEnd = %i  pFlamMap->GetNumNorth() = %i\n", RowEnd, pFlamMap->GetNumNorth()-1);
     RowEnd+=YRange;
     if ( RowEnd > pFlamMap->GetNumNorth()-1)
        RowEnd = pFlamMap->GetNumNorth();
     ColStart = ColEnd = 0;
     for ( j=0; j < NumX; j++) { 
        ColEnd += XRange;
        if ( ColEnd > pFlamMap->GetNumEast()-1)
           ColEnd = pFlamMap->GetNumEast();
		//printf("In StartLandScapeThread RowStart = %i  RowEnd = %i\n", RowStart, RowEnd);
           burnthread[threadct++]->SetLandscapeRange (ColStart, ColEnd, RowStart, RowEnd);
           ColStart = ColEnd;
     }
     RowStart=RowEnd;
  }                                        

  for (j=0; j<threadct; j++)
    burnthread[j]->StartLandscapeThread(j);
 // printf("In startlandscapethread after for");
// post message periodically here for combined progress from threads
   //ALM pFlamMap->WaitForFarsiteEvents(EVENT_BURN, threadct, true, INFINITE);

   return threadct;
}

















