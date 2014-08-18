//******************************************************************************
//	FSXWBURN.CPP	Burn model functions for FARSITE
//
//
//				Copyright 1994, 1995, 1996
//				Mark A. Finney, Systems for Environemntal Management
//******************************************************************************


#include "flm4.hpp"
//#include "fsxwattk.h"
//#include "fsairatk.h"
//#include "fsxpfront.h"
#include "flmglbvar.h"
#include "fsxsync.h"
#include <process.h>

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

BurnThread::BurnThread(FireEnvironment2 *Env)
{
	FireIsUnderAttack=false;
     ThreadID=0;
     ColStart=RowStart=ColEnd=RowEnd=0;
     turn=0;
     hBurnThread=0;
     fe=new FELocalSite();
     env=Env;
     //firering=0;
     embers.SetFireEnvironmentCalls(env, fe);
     Started=false;
     DoSpots=false;
     TotalPoints=CuumPoint=0;
}


BurnThread::~BurnThread()
{
//     if(hBurnEvent)
//     	CloseHandle(hBurnEvent);
    if(fe)
        delete fe;
}


void BurnThread::SetPerimeterRange(long currentfire, double SimTime, double cuumtimeincrement,
					double timeincrement, double timemaxrem,
                         long begin, long end, long Turn, bool attack, FireRing *Ring)
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
}


void BurnThread::SetLandscapeRange(long colstart, long colend, long rowstart, long rowend)
{
     ColStart=colstart;
     ColEnd=colend;
     RowStart=rowstart;
     RowEnd=rowend;
     SetConstantValues();
}


void BurnThread::SetConstantValues()
{     //stuff from FARSITE
     double start, end;

     SetActualTimeStep(0.0);
     SetDistRes(50.0);
     SetDynamicDistRes(50.0);
     SetPerimRes(50.0);
     SetVisibleTimeStep(30.0);
     InitializeRosRed();
     timerem=0.0;

     if(!ConstFuelMoistures(GETVAL))
     {	start=GetJulianDays(GetStartMonth());
          start+=GetStartDay();
          start*=1440.0;
          start+=(GetStartHour()/100.0)*60.0;
     	end=GetJulianDays(GetEndMonth());
          end+=GetEndDay();
          end*=1440.0;
          end+=(GetEndHour()/100.0)*60.0;
          SIMTIME=end-start;
     }
     else
     	SIMTIME=0.0;

}


HANDLE BurnThread::StartBurnThread(long ID)
{
	if(Started==false)//hBurnThread==0)
	{    ThreadOrder=ID;
          hBurnEvent=GetFarsiteEvent(EVENT_BURN_THREAD, ThreadOrder);
        	hBurnThread=(HANDLE) _beginthreadex(NULL, 0, &BurnThread::RunBurnThread, this, CREATE_SUSPENDED, &ThreadID);
          ResumeThread(hBurnThread);
          CloseHandle(hBurnThread);
          hBurnThread=0;
     }
     else
         SetEvent(hBurnEvent);//SetFarsiteEvent(EVENT_BURN_THREAD, ThreadOrder);

     return hBurnThread;
}


HANDLE BurnThread::StartLandscapeThread(long ID)
{
     //BurnTheLandscape();    // non-threaded version

	if(Started==false)
	{    ThreadOrder=ID;
          hBurnEvent=GetFarsiteEvent(EVENT_BURN_THREAD, ThreadOrder);
        	hBurnThread=(HANDLE) _beginthreadex(NULL, 0, &BurnThread::RunLandscapeThread, this, CREATE_SUSPENDED, &ThreadID);
          ResumeThread(hBurnThread);
          CloseHandle(hBurnThread);
          hBurnThread=0;
     }
     else
         SetEvent(hBurnEvent);//SetFarsiteEvent(EVENT_BURN_THREAD, ThreadOrder);
    
     return hBurnThread;
}


HANDLE BurnThread::GetThreadHandle()
{
	return hBurnThread;
}


unsigned __stdcall BurnThread::RunBurnThread(void *bt)
{
	static_cast <BurnThread *> (bt)->PerimeterThread();

     return 1;
}

unsigned __stdcall BurnThread::RunLandscapeThread(void *bt)
{
	static_cast <BurnThread *> (bt)->BurnTheLandscape();

     return 1;
}


void BurnThread::BurnTheLandscape()
{
	long j, k;
     double MaxSpotDist, SpotSpreadRate, BSR;

     for(k=RowStart; k<RowEnd; k++)
     {    if(k>GetNumNorth()-1)
     	{	k=GetNumNorth()-1;

          }
     	for(j=ColStart; j<ColEnd; j++)
          {    if(j>GetNumEast()-1)
                    k=GetNumEast()-1;
          //--------------------------------------------------------------------
          // initialize output variables in case of unburnable stuff
          //--------------------------------------------------------------------
          	RosT=RosT1=FliFinal=HSpreadRate=0.0;
               vecdir=head=flank=back=0.0;
               CrownState=0;
			embers.MaxSpot=0.0;
			embers.NumEmbers=0;

          //--------------------------------------------------------------------
          	GetPoints(k, j);					// reconstruct orientations
               fe->GetLandscapeData(xpt, ypt, ld);     // Burn::GetLandscapeData();
               fe->GetFireEnvironment(env, SIMTIME, true);
               EmberCoords();                     // compute ember source coordinates
			SurfaceFire();
               if(ld.fuel>0 && ld.cover>0 && ld.height>0.0)	 	// if there IS a tree canopy
          	{    CrownFire();

		          if(embers.NumEmbers>0)
				{  	embers.Flight(SIMTIME, SIMTIME+120.0);      // curent time to Flight is iter*actual, but really (iter+1)*actual
				     MaxSpotDist=embers.GetMaxSpotDist(embers.Fcoord.x, embers.Fcoord.y);

                    //SpotSpreadRate=MaxSpotDist/10.0;
                    //if(SpotSpreadRate>RosT1)
                    //{    RosT1=SpotSpreadRate;
                    //     BSR=2.0*head-(head+back);
                    //     head=SpotSpreadRate/2.0;
                    //     back=head-BSR;
                    //}
	               }
               }

               WriteOutputLayers(k, j);
          }
          // update progress every row, but could do it every iteration
          //SetThreadProgress(ThreadOrder, (double) (k-RowStart)/(double) (RowEnd-RowStart));
          if(ThreadOrder==0)
               printf("row finished %ld of %ld\n", k*GetMaxThreads(), RowEnd*GetMaxThreads());
     }
     printf("Rows finished, Thread %ld/%ld \n", ThreadOrder+1, GetMaxThreads());
     SetFarsiteEvent(EVENT_BURN, ThreadOrder);
}

#ifdef FLMP_ENABLE_OMFFR
void BurnThread::BurnTheLandscape(LandscapeStruct lcp_data, double one, double ten, double hund, OutputFBStruct &output)
{
    long j, k;
    double MaxSpotDist, SpotSpreadRate, BSR;
    SetConstantValues();
    //--------------------------------------------------------------------
    // initialize output variables in case of unburnable stuff
    //--------------------------------------------------------------------
    RosT=RosT1=FliFinal=HSpreadRate=0.0;
    MaxSpotDist = 0.0;
    vecdir=head=flank=back=0.0;
    CrownState=0;
    embers.MaxSpot=0.0;
    embers.NumEmbers=0;

    //--------------------------------------------------------------------
	GetPoints(0, 0);					// reconstruct orientations
    //fe->GetLandscapeData(xpt, ypt, ld);     // Burn::GetLandscapeData();
    CopyLandscapeStruct(lcp_data, &ld);
    CopyLandscapeStruct(lcp_data, &fe->ld);
    fe->GetFireEnvironment(env, SIMTIME, true);
    //fe->OverrideFuels(one/100.0, ten/100.0, hund/100.0);
	/* we don't have an lcp, no 'space' dimension */
    EmberCoords();                     // compute ember source coordinates
    embers.NumEmbers = 0;
    SurfaceFire(lcp_data);
    if(ld.fuel>0 && ld.cover>0 && ld.height>0.0)	 	// if there IS a tree canopy
    {
        CrownFire();
        if(embers.NumEmbers>0)
        {
            embers.Flight(SIMTIME, SIMTIME+120.0);      // curent time to Flight is iter*actual, but really (iter+1)*actual
            MaxSpotDist=embers.GetMaxSpotDist(embers.Fcoord.x, embers.Fcoord.y);

            //SpotSpreadRate=MaxSpotDist/10.0;
            //if(SpotSpreadRate>RosT1)
            //{    RosT1=SpotSpreadRate;
            //     BSR=2.0*head-(head+back);
            //     head=SpotSpreadRate/2.0;
            //     back=head-BSR;
            //}
        }
    }

    /* Get the output using GetLastValue() */
    /* Don't need ros_t */
	/* take abs of FliFinal to get flame length */
	FliFinal = fabs(FliFinal);
    output.ros = GetLastValue( SPREADRATE );
    output.fli = fabs(GetLastValue( INTENSITY ) );
    output.flame_length = GetLastValue( FLAMELENGTH );
    output.h_ros = GetLastValue( HORIZRATE );
    output.v_dir = GetLastValue( MAXSPREADDIR );
    output.head = GetLastValue( ELLIPSEDIM_B );
    output.flank = GetLastValue( ELLIPSEDIM_A );
    output.back = GetLastValue( ELLIPSEDIM_B );
    output.crown_state = GetLastValue( CROWNSTATE );
    output.max_spot = GetLastValue( MAXSPOT );

    //WriteOutputLayers(k, j);
    // update progress every row, but could do it every iteration
    //SetThreadProgress(ThreadOrder, (double) (k-RowStart)/(double) (RowEnd-RowStart));
    SetFarsiteEvent(EVENT_BURN, ThreadOrder);
}

void BurnThread::CopyLandscapeStruct(LandscapeStruct src, LandscapeStruct *dst)
{
    (*dst).elev = src.elev;
    (*dst).slope = src.slope;
    (*dst).aspect = src.aspect;
    (*dst).fuel = src.fuel;
    (*dst).cover = src.cover;               // READ OR DERIVED FROM LANDSCAPE DATA
    (*dst).aspectf = src.aspectf;
    (*dst).height = src.height;
    (*dst).base = src.base;
    (*dst).density = src.density;
    (*dst).duff = src.duff;
    (*dst).woody = src.woody;
}
#endif /* FLMP_ENABLE_OMFFR */


void BurnThread::PerimeterThread()
{
     long 	begin, end;
     long		i, TestPointL, TestPointN;
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

                         fe->GetFireEnvironment(env, SIMTIME+SimTimeOffset, false);
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
		          {	GetCurrentFuelMoistures(ld.fuel, ld.woody, (double*) &gmw, mx, 7);
		          	AddToCurrentFireRing(firering, CurrentPoint, ld.fuel, ld.woody, ld.duff, mx, cf.CrownLoadingBurned*1000.0);
		          }
			}
		}
		SetFarsiteEvent(EVENT_BURN, ThreadOrder);		//hSimEvent[ThreadOrder]
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

     SetFarsiteEvent(EVENT_BURN, ThreadOrder);
}


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
          CalculateFireOrientation(GetOffsetFromMax());
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

#ifdef FLMP_ENABLE_OMFFR
void BurnThread::SurfaceFire(LandscapeStruct lcp_data)
{// does surface fire calculations of spread rate and intensity
    CrownState=0;
    LoadGlobalFEData(fe);
    //CopyLandscapeStruct(lcp_data, &ld);
    if(ld.fuel<=0)
        return;
    fros=spreadrate(ld.slope, m_windspd, ld.fuel);
    if(fros>0.0)                                    // if rate of spread is >0
    {
        GetAccelConst();              // get acceleration constants
        VecSurf();                              // vector wind with slope
        CalculateFireOrientation(GetOffsetFromMax());
        NormalizeDirectionWithLocalSlope();
        ellipse(vecros, ivecspeed);     // compute elliptical dimensions
        grow(vecdir);                   // compute time derivatives xt and yt for perimeter point
        AccelSurf1();                 // compute new equilibrium ROS and Avg. ROS in remaining timestep
        SlopeCorrect(1);
        SubTimeStep=timerem;
        //timerem=0.0;
        limgrow();                              // limits HORIZONTAL growth to mindist
        AccelSurf2();                           // calcl ros & fli for SubTimeStep Kw/m from BTU/ft/s*/
        CrownState=1;
        //if(SSpotOK && cover==99)
        //{
        //    cf.FlameLength=0;          // may want to set other crown parameters to 0 here
        //    SpotFire(3);                    // spotting from  winddriven surface fires
        //}
    }
    else
    {
        timerem=0;
        RosT=0;
        fli=0;
        FliFinal=0;
    }
}
#endif

void BurnThread::CrownFire(void)
{// does crown fire calculations of spread rate and intensity (Van Wagner's)
	//double FabsFli=fabs(fli);
     double FabsFli=3.4613*(384.0*(react/0.189275)*(ExpansionRate/0.30480060960)/(60.0*savx));	// ending forward fli in timestep
     double ExpRate=ExpansionRate;
     double h, f, b, d;

	if(EnableCrowning(GETVAL))			   		   // global flag for disabling crowning
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
          	if(GetCrownFireCalculation()==0)
			{	cf.CrownBurn(ExpRate, FabsFli, A);
				R10=spreadrate(ld.slope, m_twindspd*0.4, 10);
               }
     		else
				R10=cf.CrownBurn2(ExpRate, FabsFli, A, this);
               if(R10==0.0)
				R10=avgros;
               VecCrown();                 // get vectored crown spread rate
			CalculateFireOrientation(GetOffsetFromMax());
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
				if(EnableSpotting(GETVAL) && FliFinal>0.0)    // no spotting from dead perimeter
					SpotFire(0);      // spotting from active crown fire
			}
			else                        // if passive crown fire
               {    cf.CrownIntensity(avgros, &fli);    // calc flamelength and FLI for passive crown fire
                    // restore elliptical dimensions and direction if not crown fire
                    head=h;
                    back=b;
                    flank=f;
                    vecdir=d;
                    if(EnableSpotting(GETVAL) && FliFinal>0.0)    // no spotting from dead perimeter
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
	
	embers.MaxSpot=0;
	embers.NumEmbers=0;
	embers.NumSpots=0;
	if(fli>0.0)                                    // if check method 2 and second pass
	{	if(turn==1)
			CurrentTime=CuumTimeIncrement+SIMTIME;   	// timerem will be ~zero with method 2
		else
			CurrentTime=(GetActualTimeStep()-timerem)+SIMTIME;    	// timerem will represent time remaining in actual TS
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



double BurnThread::GetLastValue(long Layer)
{
     double value;

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
          case MIDFLAME:
          		value=gmw.windspd;//gmw.windspd;
          	break;
          case HORIZRATE:
          		value=HSpreadRate;//cosslope*RosT1;
          	break;
          case MAXSPREADDIR:
          		value=PI+vecdir;
                    if(value>PI*2.0)
                    	value-=PI*2.0;
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
				embers.MaxSpot=0.0;
               break;
     }

     return value;
}


void BurnThread::WriteOutputLayers(long row, long col)

{
	if(ld.fuel==-3)
     	return;

	float *layer;

	if(ld.slope>35)
		ld.slope*=1;

	if(GetOutputOption(FLAMELENGTH))
     {	layer=GetThreadLayer(FLAMELENGTH, row);
     	layer[col]=GetLastValue(FLAMELENGTH);
     }
	if(GetOutputOption(SPREADRATE))
     {	layer=GetThreadLayer(SPREADRATE, row);
     	layer[col]=GetLastValue(SPREADRATE);
     }
	if(GetOutputOption(INTENSITY))
     {	layer=GetThreadLayer(INTENSITY, row);
     	layer[col]=GetLastValue(INTENSITY);
     }
	if(GetOutputOption(HEATAREA))
     {	layer=GetThreadLayer(HEATAREA, row);
     	layer[col]=GetLastValue(HEATAREA);
     }
	if(GetOutputOption(CROWNSTATE))
     {	layer=GetThreadLayer(CROWNSTATE, row);
     	layer[col]=GetLastValue(CROWNSTATE);
     }
	if(GetOutputOption(SOLARRADIATION))
     {	layer=GetThreadLayer(SOLARRADIATION, row);
     	layer[col]=GetLastValue(SOLARRADIATION);
     }
	if(GetOutputOption(FUELMOISTURE1))
     {	layer=GetThreadLayer(FUELMOISTURE1, row);
     	layer[col]=GetLastValue(FUELMOISTURE1);
     }
	if(GetOutputOption(FUELMOISTURE10))
     {	layer=GetThreadLayer(FUELMOISTURE10, row);
     	layer[col]=GetLastValue(FUELMOISTURE10);
     }
	if(GetOutputOption(MIDFLAME))
     {	layer=GetThreadLayer(MIDFLAME, row);
     	layer[col]=GetLastValue(MIDFLAME);
     }
	if(GetOutputOption(HORIZRATE))
     {	layer=GetThreadLayer(HORIZRATE, row);
     	layer[col]=GetLastValue(HORIZRATE);
     }
	if(GetOutputOption(MAXSPREADDIR))
     {	layer=GetThreadLayer(MAXSPREADDIR, row);
     	layer[col]=GetLastValue(MAXSPREADDIR);
     }
	if(GetOutputOption(ELLIPSEDIM_A))
     {	layer=GetThreadLayer(ELLIPSEDIM_A, row);
     	layer[col]=GetLastValue(ELLIPSEDIM_A);
     }
	if(GetOutputOption(ELLIPSEDIM_B))
     {	layer=GetThreadLayer(ELLIPSEDIM_B, row);
     	layer[col]=GetLastValue(ELLIPSEDIM_B);
     }
	if(GetOutputOption(ELLIPSEDIM_C))
     {	layer=GetThreadLayer(ELLIPSEDIM_C, row);
     	layer[col]=GetLastValue(ELLIPSEDIM_C);
     }
	if(GetOutputOption(MAXSPOT))
     {	layer=GetThreadLayer(MAXSPOT, row);
     	layer[col]=GetLastValue(MAXSPOT);
     }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//	Burn:: Functions
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


Burn::Burn ()
{
     //InitialAllocFirePerims();
     CreateSpotSemaphore();
     SIMTIME=0;
     CuumTimeIncrement=0.0;
     TotalPoints=0;
     burnthread=0;
     NumPerimThreads=0;
     fe=new FELocalSite();
     env=new FireEnvironment2();
     //embers.SetFireEnvironmentCalls(env, fe);
     NumSpots=0;
     ExNumPts=0;
}


Burn::~Burn()
{
	ResetAllPerimThreads();
     CloseSpotSemaphore();
     FreeAllFirePerims();
     FreeElev();
     delete fe;
     delete env;
#ifdef FLMP_ENABLE_OMFFR
     delete pointBurnThread;
#endif
}


void Burn::BurnIt(long count)
{// controls access to different burn methods and output options
	CurrentFire=count;		  // local copy of currentfire
	if(DistanceCheckMethod(GETVAL))
		BurnMethod1();        // distance checking on individual fire basis
	else
		BurnMethod2();        // distance checking on Simulation basis
}


void Burn::ResetAllPerimThreads()
{
	CloseAllPerimThreads();
}


bool Burn::AllocPerimThreads()
{
	if(NumPerimThreads==GetMaxThreads())
     	return true;

	CloseAllPerimThreads();
	burnthread=(BurnThread **) new BurnThread*[GetMaxThreads()];
#ifdef FLMP_ENABLE_OMFFR
        pointBurnThread = new BurnThread(env);
#endif

     long i;
     for(i=0; i<64; i++)
          SetThreadProgress(i, 0.0);

     if(burnthread)
     {    NumPerimThreads=GetMaxThreads();
	     NumSpots=new long[NumPerimThreads];
     	for(i=0; i<NumPerimThreads; i++)
          	burnthread[i]=new BurnThread(env);
		return true;
     }

     return false;
}


void Burn::CloseAllPerimThreads()
{
	long m;

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
     long i;

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


void Burn::ResumeSpotThreads(long threadct)
{
     long i;

     env->CheckMoistureTimes(SIMTIME); //must do here because ember flight uses: GetFireEnvt()
     for(i=0; i<threadct; i++)
     {	burnthread[i]->SetPerimeterRange(0, SIMTIME, CuumTimeIncrement, TimeIncrement,
          	TimeMaxRem, 0, 0, 0, FireIsUnderAttack, firering);
	     if(!burnthread[i]->Started)
	     {	burnthread[i]->StartBurnThread(i);
               WaitForOneFarsiteEvent(EVENT_BURN, i, INFINITE);
          }
     }
     for(i=0; i<threadct; i++)
     {	burnthread[i]->DoSpots=true;
          burnthread[i]->StartBurnThread(i);
     }
     Sleep(1);
	WaitForFarsiteEvents(EVENT_BURN, threadct, TRUE, INFINITE);
}


void Burn::ResumePerimeterThreads(long threadct)
{
     long i;

     for(i=0; i<threadct; i++)
        	burnthread[i]->SetPerimeterRange(CurrentFire, SIMTIME, CuumTimeIncrement, TimeIncrement,
          					TimeMaxRem, -1, 0, turn, FireIsUnderAttack, firering);
     for(i=0; i<threadct; i++)
     	burnthread[i]->StartBurnThread(i);
	WaitForFarsiteEvents(EVENT_BURN, threadct, TRUE, INFINITE);
}


long Burn::StartPerimeterThreads_Equal()
{
     long i;
     long begin, end, threadct, range;
     double fract, interval, ipart;

	AllocPerimThreads();

     interval=((double) GetNumPoints(CurrentFire))/((double) NumPerimThreads);
     fract=modf(interval, &ipart);
     range=(long) interval;
     if(fract>0.0)
     	range++;
	//if(range<6)
     //	range=6;
     //if(range>GetNumPoints(CurrentFire))
     //	range=GetNumPoints(CurrentFire);

     begin=threadct=0;

     for(i=0; i<NumPerimThreads; i++)
     {	end=begin+range;
    	     if(begin>=GetNumPoints(CurrentFire))
          {	//begin=0;
          	//end=0;
          	continue;
          }
	     if(end>GetNumPoints(CurrentFire))
     	    	end=GetNumPoints(CurrentFire);
         	burnthread[i]->SetPerimeterRange(CurrentFire, SIMTIME, CuumTimeIncrement, TimeIncrement,
          					TimeMaxRem, begin, end, turn, FireIsUnderAttack, firering);
	     threadct++;
          begin=end;
     }

     for(i=0; i<threadct; i++)
     	burnthread[i]->StartBurnThread(i);
	WaitForFarsiteEvents(EVENT_BURN, threadct, true, INFINITE);
//	WaitForMultipleObjects(threadct, hSimEvent, TRUE, INFINITE);

     return threadct;
}



long Burn::StartPerimeterThreads_ActiveOnly()
{
     long i, NumLive;
     long begin, end, liverange, threadct;//, range;
     double fli;

	AllocPerimThreads();
	NumLive=0;
     if(NumPerimThreads>1)
     {    for(CurrentPoint=0; CurrentPoint<GetNumPoints(CurrentFire); CurrentPoint++)
		{	fli=GetPerimeter2Value(CurrentPoint, FLIVAL);
    		     if(fli>=0.0)
         			NumLive++;
	     }
     	liverange=NumLive/NumPerimThreads+1;
	     if(liverange>GetNumPoints(CurrentFire))
     		liverange=GetNumPoints(CurrentFire);
     }
     begin=threadct=0;
     for(i=0; i<NumPerimThreads; i++)
     {    if(NumPerimThreads>1)
     	{   	NumLive=0;
	     	for(CurrentPoint=begin; CurrentPoint<GetNumPoints(CurrentFire); CurrentPoint++)
			{	fli=GetPerimeter2Value(CurrentPoint, FLIVAL);
    	     		if(fli>=0.0)
         				NumLive++;
	               if(NumLive>liverange)
     	          	break;
     		}
	          end=CurrentPoint;
     	     if(begin>=GetNumPoints(CurrentFire))
          		continue;
	          if(end>GetNumPoints(CurrentFire))
     	     	end=GetNumPoints(CurrentFire);
          }
          else
          {	begin=0;
          	end=GetNumPoints(CurrentFire);
          }
         	burnthread[i]->SetPerimeterRange(CurrentFire, SIMTIME, CuumTimeIncrement, TimeIncrement,
          					TimeMaxRem, begin, end, turn, FireIsUnderAttack, firering);
	     threadct++;
          begin=end;
     }
     for(i=0; i<threadct; i++)
     	burnthread[i]->StartBurnThread(i);
	WaitForFarsiteEvents(EVENT_BURN, threadct, true, INFINITE);

     return threadct;
}


void Burn::Factor(long number, long *x, long *y)
{    // finds largest factors of integer number

     long X, Y;
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


long Burn::StartLandscapeThreads()
{
     long i, j, NumX, NumY, Ret;
     long threadct, XRange, YRange, Xprocs, Yprocs;
     long ColStart, ColEnd, RowStart, RowEnd;

	AllocPerimThreads();

     // find number of threads along x-axis and y-axis
     Factor(NumPerimThreads, &NumX, &NumY);
     XRange=ceil((double) GetNumEast()/(double) NumX);
     YRange=ceil((double) GetNumNorth()/(double) NumY);

     //XRange=ceil((double) GetNumEast()/ceil((double) NumPerimThreads/(double) NumX));
     //YRange=ceil((double) GetNumNorth()/ceil((double) NumPerimThreads/(double) NumY));

     RowStart=RowEnd=threadct=0;

     for(i=0; i<NumY; i++)
     {    RowEnd+=YRange;
          if(RowEnd>GetNumNorth()-1)
               RowEnd=GetNumNorth();
          ColStart=ColEnd=0;
          for(j=0; j<NumX; j++)
          {    ColEnd+=XRange;
               if(ColEnd>GetNumEast()-1)
                    ColEnd=GetNumEast();
              	burnthread[threadct++]->SetLandscapeRange(ColStart, ColEnd, RowStart, RowEnd);
               ColStart=ColEnd;
          }
          RowStart=RowEnd;
     }                                        


     for(j=0; j<threadct; j++)
     	burnthread[j]->StartLandscapeThread(j);

     // post message periodically here for combined progress from threads

     WaitForFarsiteEvents(EVENT_BURN, threadct, true, INFINITE);

     return threadct;
}


void Burn::WriteLayersToDisk()
{
	if(GetOutputOption(FLAMELENGTH))
     	WriteOutputLayerToDisk(FLAMELENGTH);
	if(GetOutputOption(SPREADRATE))
     	WriteOutputLayerToDisk(SPREADRATE);
	if(GetOutputOption(INTENSITY))
     	WriteOutputLayerToDisk(INTENSITY);
	if(GetOutputOption(HEATAREA))
     	WriteOutputLayerToDisk(HEATAREA);
	if(GetOutputOption(CROWNSTATE))
     	WriteOutputLayerToDisk(CROWNSTATE);
	if(GetOutputOption(SOLARRADIATION))
     	WriteOutputLayerToDisk(SOLARRADIATION);
	if(GetOutputOption(FUELMOISTURE1))
     	WriteOutputLayerToDisk(FUELMOISTURE1);
	if(GetOutputOption(FUELMOISTURE10))
     	WriteOutputLayerToDisk(FUELMOISTURE10);
	if(GetOutputOption(MIDFLAME))
     	WriteOutputLayerToDisk(MIDFLAME);
	if(GetOutputOption(HORIZRATE))
     	WriteOutputLayerToDisk(HORIZRATE);
	if(GetOutputOption(MAXSPREADDIR))
     	WriteOutputLayerToDisk(MAXSPREADDIR);
	if(GetOutputOption(ELLIPSEDIM_A))
     	WriteOutputLayerToDisk(ELLIPSEDIM_A);
	if(GetOutputOption(ELLIPSEDIM_B))
     	WriteOutputLayerToDisk(ELLIPSEDIM_B);
	if(GetOutputOption(ELLIPSEDIM_C))
     	WriteOutputLayerToDisk(ELLIPSEDIM_C);
}



void Burn::BurnMethod1()
{// In-TimeStep growth of new inward fires 12/27/1994
	bool 	INFIRE=false;
     long		i;
	long 	NewFires;
    	long 	NewPts;
	long 	NewInFires;
     long 	ThisInFire;
     long 	InFiresBurned=0;              // Number of new inward burning fires
	long 	FireType=GetInout(CurrentFire);    // save fire type in case inward fire burns out in this timestep
//     long 	NumStartAttack;
     long		NumLastAttack;
     bool		GroupDirectAttack;	//=false;
     double 	DownTime;
     double	xpt, ypt;//, *perim;

     //GroupAirAttack *gaat;
	//AttackData* atk;                        // Pointer to AttackData struct
     //Attack Atk;						// Instance of Attack

	NewInFires=NewFires=GetNewFires();	// copy of new fire number
	NumInFires=0;						// reset BURN::NumInFires;
	turn=0;
	StillBurning=false;                         // start each fire out as burning (changed)
     CanStillBurn=false;                         // must test to see if fire is still active
	InitRect(CurrentFire);				// resets hi's and lo's for bounding rectangle
	TimeIncrement=EventTimeStep=GetActualTimeStep();
	CuumTimeIncrement=0.0;
	AllocElev(CurrentFire);				// alloc space for elevations
	tranz(CurrentFire,0);				// transfer points to perimeter2 array for next turn
     TimeMaxRem=0.0;               		// maximum time remaining
	//if((atk=GetAttackForFireNumber(CurrentFire, 0, &NumLastAttack))!=0)
     //	FireIsUnderAttack=true;
     //else
     //	FireIsUnderAttack=false;
     //if((GetGroupAttackForFireNumber(CurrentFire, 0, &NumLastAttack))!=0)
     //{	GroupDirectAttack=true;
     //     gaat=new GroupAirAttack();
     //}
     //else
     //{	GroupDirectAttack=false;
     //     gaat=0;
     //}
   	do
	{    ExNumPts=0;
     	if(GetInout(CurrentFire)==3)
     	{	CrossFires(0, &CurrentFire);
          	CanStillBurn=true;
               TimeIncrement=0.0;
               CuumTimeIncrement=GetActualTimeStep();
               break;
	     }
          // multithreading stuff
     //--------------------------------------------------------------------------
     //--------------------------------------------------------------------------
     //--------------------------------------------------------------------------
//		NumStartAttack=0;
		if(turn==0)
		{    env->CheckMoistureTimes(SIMTIME+CuumTimeIncrement);
               ThreadCount=StartPerimeterThreads_Equal();
          }
          else
          {	ResumePerimeterThreads(ThreadCount);

			CanStillBurn=burnthread[0]->CanStillBurn;
			StillBurning=burnthread[0]->StillBurning;
               prod.cuumslope[0]+=burnthread[0]->prod.cuumslope[0];
               prod.cuumslope[1]+=burnthread[0]->prod.cuumslope[1];
	          for(i=1; i<ThreadCount; i++)
     	    	{	if(burnthread[i]->CanStillBurn)
          	         	CanStillBurn=true;
               	if(burnthread[i]->StillBurning)
                   		StillBurning=true;
	               prod.cuumslope[0]+=burnthread[i]->prod.cuumslope[0];
     	          prod.cuumslope[1]+=burnthread[i]->prod.cuumslope[1];
	          }
          }
     //--------------------------------------------------------------------------
     //--------------------------------------------------------------------------
     //--------------------------------------------------------------------------
		if(turn)
		{    for(i=0; i<GetNumPoints(CurrentFire); i++)
          	{	xpt=GetPerimeter1Value(CurrentFire, i, XCOORD);
          		ypt=GetPerimeter1Value(CurrentFire, i, YCOORD);
               	DetermineHiLo(xpt, ypt);
               }

          	if(CuumTimeIncrement==0.0 && INFIRE==false)	// should only happen first time through
			{	prod.arp(CurrentFire);				// calc area of previous fire perimeter
                	DownTime=GetDownTime();				// time spent out of active burn period
               }
               else
               	DownTime=0.0;
               NewFires=GetNewFires();
               /*while((atk=GetAttackForFireNumber(CurrentFire, NumStartAttack, &NumLastAttack))!=0)
          	{	NumStartAttack=NumLastAttack+1;
                    if(atk->Indirect==2)
                    {    if(!Atk.ParallelAttack(atk, TimeIncrement+DownTime))
                         	CancelAttack(atk->AttackNumber);
                    }
                    else if(!Atk.DirectAttack(atk, TimeIncrement+DownTime))  // perform attack on this fire in timeincrement
                    	CancelAttack(atk->AttackNumber);
               }
               if(GroupDirectAttack)
               {    NumStartAttack=0;
               	while((GetGroupAttackForFireNumber(CurrentFire, NumStartAttack, &NumLastAttack))!=0)
                    {    NumStartAttack=NumLastAttack+1;
                         gaat->GetCurrentGroup();
				     if(gaat->CheckSuspendState(GETVAL))
                             	continue;
    					gaat->ExecuteAttacks(TimeIncrement+DownTime); //for each aircraft,
                    }                                         //execute, but if not==TimeIncrement, just dec waittimes
               }*/
			WriteHiLo(CurrentFire);
               if(FireIsUnderAttack)
	          {    RestoreDeadPoints(CurrentFire);
          		BoundingBox(CurrentFire);
               }
			if(GetInout(CurrentFire)!=0 && StillBurning)	// if inward fire not eliminated && still burning
			{    //if(rastmake)  						// comput raster information
				//	rast.rasterinit(CurrentFire, 0, SIMTIME, TimeIncrement, CuumTimeIncrement+TimeIncrement);
                    if(FireIsUnderAttack)
                    {	while(GetNewFires()>NewFires)
	               	{    if(GetInout(NewFires)!=1)
                         	{	NewFires++;
                              	continue;
                              }
                         	ReorderPerimeter(NewFires, FindExternalPoint(NewFires, 0));
						FindOuterFirePerimeter(NewFires);
						NewPts=GetNumPoints(NewFires);
						FreePerimeter1(NewFires);
                    		if(NewPts>0)
						{	AllocPerimeter1(NewFires, NewPts+1);
							tranz(NewFires, NewPts);
			          	}
			          	else
		     	     	{	SetNumPoints(NewFires, 0);
		               		SetInout(NewFires, 0);
		              			IncSkipFires(1);
	          			}
                              NewFires++;
                         }
	               }
                    CrossFires(0, &CurrentFire);   		// clip cross-overs and rediscretize
				if(GetInout(CurrentFire)==2)            // if inward burning fire
				{	if(arp(1, CurrentFire)>=0.0)   	// eliminates inward fire if area>0
                         	EliminateFire(CurrentFire);
				}
                    CrossesWithBarriers(CurrentFire);		// merge with barriers if present
				while(GetNewFires()>NewInFires)
				{    ThisInFire=NewInFires++;           // copy before incrementing newinfires
					if(GetInout(ThisInFire)==2)
					{	tranz(ThisInFire,0);          // transfer points to perimeter2 in case it is removed
						CrossFires(0, &ThisInFire);  	   		// clip cross-overs and rediscretize
						if(GetInout(ThisInFire)==2)
						{	if(arp(1, ThisInFire)>=0.0)   	// eliminates inward fire if area>0
								EliminateFire(ThisInFire);
							else if((GetActualTimeStep()-CuumTimeIncrement+TimeIncrement)>0.0)
								AllocNewInFire(ThisInFire, GetActualTimeStep()-CuumTimeIncrement-TimeIncrement);
						}
					}
				}
			}

               CuumTimeIncrement+=TimeIncrement;	// accumulate timeincrements

			if(GetInout(CurrentFire)==0 || !StillBurning)// if inward fire has now been eliminated
			{    if(FireType==2 && StillBurning) 		// inward fire was extinguished
				{	//if(rastmake)                       // make rasterizing accurate to the nearest time step
					//	rast.rasterinit(CurrentFire, ExNumPts, SIMTIME, CuumTimeIncrement, GetActualTimeStep());
												// compute raster information
				}
				TimeIncrement=0.0;					// zero time left in timestep
			}
			else
			{	TimeIncrement=GetActualTimeStep()-CuumTimeIncrement;
				EventTimeStep=TimeIncrement;       // reset Event drivent Time Step
				tranz(CurrentFire, 0);    		// transfer points to perimeter2 array for next turn
				turn=0;
			}
		}
		else
		{    TimeMaxRem=burnthread[0]->TimeMaxRem;
     	     EventTimeStep=burnthread[0]->EventTimeStep;
   	          for(i=1; i<ThreadCount; i++)
          	{	if(burnthread[i]->TimeMaxRem>TimeMaxRem)
     	     		TimeMaxRem=burnthread[i]->TimeMaxRem;
               	if(burnthread[i]->EventTimeStep<EventTimeStep)
     	     		EventTimeStep=burnthread[i]->EventTimeStep;
	          }
          	TimeIncrement-=TimeMaxRem;         // Calculate distance limited TS
          	/*if(GroupDirectAttack)
               {	while((GetGroupAttackForFireNumber(CurrentFire, NumStartAttack, &NumLastAttack))!=0)
               	{	NumStartAttack=NumLastAttack+1;
                    	gaat->GetCurrentGroup();   // get current group attack
                         if(gaat->CheckSuspendState(GETVAL))
                         	continue;
                         EventTimeStep=gaat->GetNextAttackTime(EventTimeStep);
                    }
               }*/
			if(EventTimeStep<TimeIncrement && EventTimeStep>0.01)    // Check to see if event limited TS < distance limited TS
				TimeIncrement=EventTimeStep;
			turn=1;
		}
		TimeMaxRem=0.0;
		if(TimeIncrement==0.0 && NumInFires>InFiresBurned)
		{    do
			{    if(InFiresBurned<NumInFires)
				{	GetNewInFire(InFiresBurned);
					InFiresBurned++;
				}
				else
					TimeIncrement=-1.0;
				if(TimeIncrement>0.0)
				{    CuumTimeIncrement=GetActualTimeStep()-TimeIncrement;
					EventTimeStep=TimeIncrement; 		// reset EventTimeStep
					AllocElev(CurrentFire);	  // alloc space for elevations
					tranz(CurrentFire,0);	  // transfer points to perimeter2 array for next turn
					InitRect(CurrentFire);	  // resets hi's and lo's for bounding rectangle
					CanStillBurn=false;		  // reset ability to burn as false
					FireType=2;                // set local var. FireType to inward fire
					turn=0;                    // set turn to 1st time through
                         INFIRE=true;
				}
			} while(TimeIncrement==0.0);
		}
     } while(TimeIncrement>0.0);			  // while time remaining in time step
	if(!StillBurning && !CanStillBurn && GetInout(CurrentFire)==2)	// eliminates fire perimeter around rock or lake islands
	{    if(PreserveInactiveEnclaves(GETVAL)==false)                 // but not in fuel
          	EliminateFire(CurrentFire);
     }
	FreeNewInFires();
     //if(gaat)
     //	delete gaat;      // group air attack
}


void Burn::DetermineSimLevelTimeStep()
{
     long i;

     TimeMaxRem=0.0;
     TotalPoints=0;
    	TimeIncrement=EventTimeStep=GetActualTimeStep()-CuumTimeIncrement;
     for(i=0; i<GetNumFires(); i++)
     	TotalPoints+=GetNumPoints(i);
     CuumPoint=0;
	for(CurrentFire=0; CurrentFire<GetNumFires(); CurrentFire++)
     {    if(GetInout(CurrentFire)==3 || GetInout(CurrentFire)==0)
     		continue;
          PreBurn();
     }
     TimeIncrement-=TimeMaxRem;
	if(EventTimeStep<TimeIncrement && EventTimeStep>0.01)    // Check to see if event limited TS < distance limited TS
		TimeIncrement=EventTimeStep;
     SetTemporaryTimeStep(TimeIncrement);
     CuumPoint=0;
}


void Burn::PreBurn()
{// just finds out what is the fastest spreading point among all fires ==> sets timestep
     bool		GroupDirectAttack; //ComputeSpread,
     long		i, NumLastAttack; //i, TestPointL, TestPointN, NumStartAttack=0,
     //GroupAirAttack *gaat;

	AllocElev(CurrentFire);				// alloc space for elevations
	tranz(CurrentFire,0);				// transfer points to perimeter2 array for next turn
	/*if((GetAttackForFireNumber(CurrentFire, 0, &NumLastAttack))!=0)
     	FireIsUnderAttack=true;
     else
     	FireIsUnderAttack=false;
     if((GetGroupAttackForFireNumber(CurrentFire, 0, &NumLastAttack))!=0)
     {	GroupDirectAttack=true;
          gaat=new GroupAirAttack();
     }
     else
     {	GroupDirectAttack=false;
          gaat=0;
     }
     */
     //--------------------------------------------------------------------------
     //--------------------------------------------------------------------------
     //--------------------------------------------------------------------------
     turn=0;
     env->CheckMoistureTimes(SIMTIME+CuumTimeIncrement);
     ThreadCount=StartPerimeterThreads_Equal();

	TimeMaxRem=burnthread[0]->TimeMaxRem;
     EventTimeStep=burnthread[0]->EventTimeStep;
   	for(i=1; i<ThreadCount; i++)
     {	if(burnthread[i]->TimeMaxRem>TimeMaxRem)
	    		TimeMaxRem=burnthread[i]->TimeMaxRem;
         	if(burnthread[i]->EventTimeStep<EventTimeStep)
     		EventTimeStep=burnthread[i]->EventTimeStep;
     }
     //--------------------------------------------------------------------------
     //--------------------------------------------------------------------------
     //--------------------------------------------------------------------------
     /*
     if(GroupDirectAttack)
     {	while((GetGroupAttackForFireNumber(CurrentFire, NumStartAttack, &NumLastAttack))!=0)
         	{	NumStartAttack=NumLastAttack+1;
               gaat->GetCurrentGroup();   // get current group attack
               if(gaat->CheckSuspendState(GETVAL))
                   	continue;
               EventTimeStep=gaat->GetNextAttackTime(EventTimeStep);
          }
     }
     */
}


void Burn::BurnMethod2()
{// Simulation-level process control, uses time step determind by PreBurn()
	long 	NewFires;
    	long 	NewPts;
	long 	FireType=GetInout(CurrentFire);    // save fire type in case inward fire burns out in this timestep
//     long 	NumStartAttack;
     long		NumLastAttack;
     bool 	FireIsUnderAttack=false;			//=false;
     bool		GroupDirectAttack;			//=false;
     long		i;
     double	xpt, ypt;
     //GroupAirAttack *gaat;
	//AttackData* atk;                        // Pointer to AttackData struct
     //Attack Atk;						// Instance of Attack
									// for rasterizing inside of extinct fire
	turn=1; 	                              // always is 'turn' because timestep already determined
	StillBurning=1;                         // start each fire out as burning (changed)
     CanStillBurn=false;                         // must test to see if fire is still active
	InitRect(CurrentFire);				// resets hi's and lo's for bounding rectangle
	TimeIncrement=GetTemporaryTimeStep();
	tranz(CurrentFire,0);				// transfer points to perimeter2 array for next turn
	//if((atk=GetAttackForFireNumber(CurrentFire, 0, &NumLastAttack))!=0)
     //	FireIsUnderAttack=true;
     //else
     //	FireIsUnderAttack=false;
     //if((GetGroupAttackForFireNumber(CurrentFire, 0, &NumLastAttack))!=0)
     //{	GroupDirectAttack=true;
     //     gaat=new GroupAirAttack();
     //}
     //else
     //{	GroupDirectAttack=false;
     //     gaat=0;
     //}

     if(GetInout(CurrentFire)==3)
    	{	CrossFires(0, &CurrentFire);
         	CanStillBurn=true;
          TimeIncrement=0.0;

          return;
     }
	AllocElev(CurrentFire);				// alloc space for elevations
     if(CheckPostFrontal(GETVAL))          	// store current fire perim in ring
		firering=post.SetupFireRing(CurrentFire, SIMTIME+CuumTimeIncrement, SIMTIME+CuumTimeIncrement+TimeIncrement);

     //--------------------------------------------------------------------------
     //--------------------------------------------------------------------------
     //--------------------------------------------------------------------------
//	NumStartAttack=0;

     env->CheckMoistureTimes(SIMTIME+CuumTimeIncrement);
     ThreadCount=StartPerimeterThreads_Equal();

	CanStillBurn=burnthread[0]->CanStillBurn;
	StillBurning=burnthread[0]->StillBurning;
     prod.cuumslope[0]+=burnthread[0]->prod.cuumslope[0];
     prod.cuumslope[1]+=burnthread[0]->prod.cuumslope[1];
     for(i=1; i<ThreadCount; i++)
   	{	if(burnthread[i]->CanStillBurn)
              	CanStillBurn=true;
          if(burnthread[i]->StillBurning)
              	StillBurning=true;
          prod.cuumslope[0]+=burnthread[i]->prod.cuumslope[0];
          prod.cuumslope[1]+=burnthread[i]->prod.cuumslope[1];
     }

     for(i=0; i<GetNumPoints(CurrentFire); i++)
   	{	xpt=GetPerimeter1Value(CurrentFire, i, XCOORD);
   		ypt=GetPerimeter1Value(CurrentFire, i, YCOORD);
        	DetermineHiLo(xpt, ypt);
     }
     //--------------------------------------------------------------------------
     //--------------------------------------------------------------------------
     //--------------------------------------------------------------------------

     ExNumPts=0;
	prod.arp(CurrentFire);				// calc area of previous fire perimeter
     NewFires=GetNewFires();
     /*while((atk=GetAttackForFireNumber(CurrentFire, NumStartAttack, &NumLastAttack))!=0)
    	{	NumStartAttack=NumLastAttack+1;
          if(atk->Indirect==2)
          {    if(!Atk.ParallelAttack(atk, TimeIncrement+GetDownTime()))
                   	CancelAttack(atk->AttackNumber);
          }
          else if(!Atk.DirectAttack(atk, TimeIncrement+GetDownTime()))  // perform attack on this fire in timeincrement
              	CancelAttack(atk->AttackNumber);
     }
     if(GroupDirectAttack)
     {    NumStartAttack=0;
        	while((GetGroupAttackForFireNumber(CurrentFire, NumStartAttack, &NumLastAttack))!=0)
          {    NumStartAttack=NumLastAttack+1;
               gaat->GetCurrentGroup();
		     if(gaat->CheckSuspendState(GETVAL))
                   	continue;
			gaat->ExecuteAttacks(TimeIncrement+GetDownTime()); //for each aircraft,
          }                                         //execute, but if not==TimeIncrement, just dec waittimes
     }*/
	WriteHiLo(CurrentFire);
     if(FireIsUnderAttack)
     {    RestoreDeadPoints(CurrentFire);
     	BoundingBox(CurrentFire);
     }

     if(FireIsUnderAttack && CheckPostFrontal(GETVAL))
          post.UpdateAttackPoints(firering, CurrentFire);

	if(GetInout(CurrentFire)!=0 && StillBurning)	// if inward fire not eliminated && still burning
	{    //if(rastmake)  						// comput raster information
		//	rast.rasterinit(CurrentFire, ExNumPts, SIMTIME, TimeIncrement, CuumTimeIncrement+TimeIncrement);
          if(FireIsUnderAttack)
          {	while(GetNewFires()>NewFires)
              	{    if(GetInout(NewFires)!=1)
                   	{	NewFires++;
                        	continue;
                    }
                   	ReorderPerimeter(NewFires, FindExternalPoint(NewFires, 0));
				FindOuterFirePerimeter(NewFires);
				NewPts=GetNumPoints(NewFires);
				FreePerimeter1(NewFires);
              		if(NewPts>0)
				{	AllocPerimeter1(NewFires, NewPts+1);
					tranz(NewFires, NewPts);
	          	}
	          	else
	    	     	{	//FreePerimeter2();
	        	    		SetNumPoints(NewFires, 0);
	              		SetInout(NewFires, 0);
	         			IncSkipFires(1);
         			}
                    NewFires++;
               }
          }
          CrossFires(0, &CurrentFire);   		// clip cross-overs and rediscretize
		if(GetInout(CurrentFire)==2)            // if inward burning fire
		{	if(arp(1, CurrentFire)>=0.0)   	// eliminates inward fire if area>0
				EliminateFire(CurrentFire);
		}
          CrossesWithBarriers(CurrentFire);		// merge with barriers if present
	}

	if(GetInout(CurrentFire)==0 || !StillBurning)// if inward fire has now been eliminated
	{    if(FireType==2 && StillBurning) 		// inward fire was extinguished
		{	//if(rastmake)                       // make rasterizing accurate to the nearest time step
			//	rast.rasterinit(CurrentFire, ExNumPts, SIMTIME, CuumTimeIncrement, CuumTimeIncrement+TimeIncrement);//GetActualTimeStep());
		}
		TimeIncrement=0.0;					// zero time left in timestep
	}

	if(!StillBurning && !CanStillBurn && GetInout(CurrentFire)==2)	// eliminates fire perimeter around rock or lake islands
	{    if(PreserveInactiveEnclaves(GETVAL)==false)                 // but not in fuel
          	EliminateFire(CurrentFire);                   		// but not in fuel
     }
}


void Burn::AllocNewInFire(long NewNum, double TimeInc)
{// allocates linked list of new inward fire structures
	if(NumInFires==0)		// BURN::NumInFires data member
	{	FirstInFire=(newinfire *) new newinfire;//GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, 1*sizeof(newinfire));
		FirstInFire->next=(newinfire *) new newinfire;
		TempInNext=(newinfire *) FirstInFire->next;//GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, 1*sizeof(newinfire));
		TempInFire=FirstInFire;
	}
	TempInFire->FireNum=NewNum;
	TempInFire->TimeInc=TimeInc;
	TempInFire=TempInNext;
	TempInFire->next=(newinfire *) new newinfire;
	TempInNext=(newinfire*) TempInFire->next;
	NumInFires++;
}


void Burn::GetNewInFire(long InFire)
{// retrieves data from linked list of new inward fires
	if(InFire==0)
	{	CurInFire=FirstInFire;
		NextInFire=(newinfire *) FirstInFire->next;
	}
	CurrentFire=CurInFire->FireNum;       // BURN::CurrentFire
	TimeIncrement=CurInFire->TimeInc;	   // BURN::TimeIncrement
	CurInFire=NextInFire;
	NextInFire=(newinfire *) CurInFire->next;
}


void Burn::FreeNewInFires()
{// frees all linked list for new inward fires
	long i;

	if(NumInFires)
	{    CurInFire=FirstInFire;
		NextInFire=(newinfire *) CurInFire->next;
		for(i=0; i<NumInFires; i++)
		{	delete CurInFire;//GlobalFree(CurInFire);
			CurInFire=NextInFire;
			NextInFire=(newinfire *) CurInFire->next;
		}
		delete CurInFire; //GlobalFree(CurInFire);
		delete NextInFire; //GlobalFree(NextInFire);
          NumInFires=0;
	}
}


void Burn::EliminateFire(long FireNum)
{// removes a fire, and cleans up memory for it
	SetNumPoints(FireNum, 0);  		// reset number of points
	SetInout(FireNum, 0);     	 	// reset inward/outward indicator
	FreePerimeter1(FireNum);			// free perimeter array
	IncSkipFires(1);	          	// increment number of extinquished fires
}


void Burn::SetSpotLocation(long loc)
{
     long i;
	if(loc<0)
     {	for(i=0; i<NumPerimThreads; i++)
          {	if(loc<-1)                       // remove all of them
          	{	burnthread[i]->embers.CarrySpots=0;
                    burnthread[i]->embers.NextSpot=burnthread[i]->embers.FirstSpot;
                    if(loc<-2)
                         burnthread[i]->embers.EmberReset();
          	}
               else if(burnthread[i]->embers.CarrySpots>0)
               	burnthread[i]->embers.NextSpot=(Embers::spotdata *) burnthread[i]->embers.CarrySpot->next;
               else
				burnthread[i]->embers.NextSpot=burnthread[i]->embers.FirstSpot;
          	if(burnthread[i]->embers.NumSpots>0)
			     burnthread[i]->embers.SpotReset(burnthread[i]->embers.NumSpots,
                    	burnthread[i]->embers.FirstSpot);
		     else if(burnthread[i]->embers.SpotFires>0)
			     burnthread[i]->embers.SpotReset(burnthread[i]->embers.SpotFires-
                    						  burnthread[i]->embers.CarrySpots,
					                      	  burnthread[i]->embers.NextSpot);
          }
          SpotCount=0;

          return;
     }
                                     
     CurThread=-1;
     if(EnableSpotFireGrowth(GETVAL))
     {  	for(i=0; i<NumPerimThreads; i++)
	     {	if(CurThread==-1 && NumSpots[i]>0)
			{	CurThread=i;
               	burnthread[CurThread]->embers.CurSpot=burnthread[CurThread]->embers.FirstSpot;
                    burnthread[CurThread]->embers.CarrySpot=burnthread[CurThread]->embers.FirstSpot;
                    break;
               }
	     }
     }
     else
     {	for(i=0; i<NumPerimThreads; i++)
	     {	if(CurThread==-1 && NumSpots[i]>0)
			{	CurThread=i;
               	burnthread[CurThread]->embers.CurSpot=burnthread[CurThread]->embers.FirstSpot;
                    burnthread[CurThread]->embers.CarrySpot=burnthread[CurThread]->embers.FirstSpot;
                    break;
               }
	     }
     }
     SpotCount=0;
}


Embers::spotdata *Burn::GetSpotData(double CurrentTimeStep)
{	// will add CurrentTimeStep to CurSpot->TimeRem and return 0 if delay is longer than current TimeStep
     if(CurThread>=NumPerimThreads)
     	return NULL;

	if(SpotCount>=NumSpots[CurThread])
    	{    SpotCount=0;
		do
     	{   	CurThread++;
          	if(CurThread>=NumPerimThreads)
               {	SpotCount=0;

          		return NULL;
               }
			burnthread[CurThread]->embers.CurSpot=burnthread[CurThread]->embers.FirstSpot;
          } while(SpotCount>=NumSpots[CurThread]);
     }
     else if(SpotCount>0 && SpotCount<NumSpots[CurThread])
     {    burnthread[CurThread]->embers.NextSpot=(Embers::spotdata *) burnthread[CurThread]->embers.CurSpot->next;
         	burnthread[CurThread]->embers.CurSpot=burnthread[CurThread]->embers.NextSpot;
	}
     SpotCount++;
     // looks at timeremaining for spot fire and adds time to it for delay if less than 0.0
     // then stores it at the beginning of the linked list
     if(CurrentTimeStep>0.0)
     {	if(burnthread[CurThread]->embers.CurSpot->TimeRem<=0.0)
     	{    if(burnthread[CurThread]->embers.CarrySpots==0)
          		burnthread[CurThread]->embers.CarrySpot=burnthread[CurThread]->embers.FirstSpot;
          	else
			{	burnthread[CurThread]->embers.NextSpot=(Embers::spotdata *) burnthread[CurThread]->embers.CarrySpot->next;
		          burnthread[CurThread]->embers.CarrySpot=burnthread[CurThread]->embers.NextSpot;
               }
          	burnthread[CurThread]->embers.CurSpot->TimeRem+=CurrentTimeStep;
               CopyMemory(burnthread[CurThread]->embers.CarrySpot, burnthread[CurThread]->embers.CurSpot, 3*sizeof(double));
               if(burnthread[CurThread]->embers.CarrySpots>=NumSpots[CurThread])
	               burnthread[CurThread]->embers.CarrySpots+=1;
               else
	               burnthread[CurThread]->embers.CarrySpots++;

	          return NULL;
          }
     }

     return burnthread[CurThread]->embers.CurSpot;
}


void Burn::BurnSpotThreads()
{
	long TotalEmbers, dest, *amount, num, excess;
     long i, j, k, m, range;
     double fract, interval, ipart;


     TotalEmbers=0;
     for(i=0; i<NumPerimThreads; i++)	// for each thread
     	TotalEmbers+=burnthread[i]->embers.NumEmbers;

     if(TotalEmbers==0)
     {	TotalSpots=SpotFires=0;

     	return;
     }


	interval=((double) (TotalEmbers)/(double) NumPerimThreads);
     fract=modf(interval, &ipart);
     range=(long) interval;
     if(fract>0.0)
     	range++;

	// this section equalizes the numbers of embers in each thread for flight calculation
     amount=new long[NumPerimThreads];

     for(i=0; i<NumPerimThreads; i++)
     {    for(k=0; k<NumPerimThreads; k++)//NumPerimThreads; i++)
     		amount[k]=-1;
          // find where embers are short
          if(burnthread[i]->embers.NumEmbers<range)
     	{    num=range-burnthread[i]->embers.NumEmbers;
              	dest=i;
	          // find the embers from among all burnthreads[]
          	for(j=0; j<NumPerimThreads; j++)	// for each thread
		     {  	if(j==dest)
          			continue;
	          	excess=burnthread[j]->embers.NumEmbers-range;
     	         	if(excess>0)
     		     {	if(excess>=num)
	          	     {	amount[j]=num;

	          	     	break;
	                    }
     	               else
	     	          {	amount[j]=excess;
               	     	num-=excess;
                    	}
		          }
     	     }
          	for(k=0; k<NumPerimThreads; k++)
     	     {	if(amount[k]>0)
          	    	{  	for(m=0; m<amount[k]; m++)
		     			burnthread[dest]->embers.AddEmber(&(burnthread[k]->embers.ExtractEmber(0)));
          	     }
	          }
          }
     }
     delete[] amount;

	//This section starts the spot threads and synchronizes their completion before returning
     SpotCount=0;
	ResumeSpotThreads(NumPerimThreads);

     TotalSpots=SpotFires=0;
     CurThread=-1;
     if(EnableSpotFireGrowth(GETVAL))
     {  	for(i=0; i<NumPerimThreads; i++)
	     {	NumSpots[i]=burnthread[i]->embers.SpotFires;
     		TotalSpots+=NumSpots[i];
               if(CurThread==-1 && NumSpots[i]>0)
			{	CurThread=i;
               	burnthread[CurThread]->embers.CurSpot=burnthread[CurThread]->embers.FirstSpot;
               }
			burnthread[i]->embers.CarrySpots=0;
	     }
          SpotFires=TotalSpots;
     }
     else
     {	for(i=0; i<NumPerimThreads; i++)
	     {	NumSpots[i]=burnthread[i]->embers.NumSpots;
     		TotalSpots+=NumSpots[i];
               if(CurThread==-1 && NumSpots[i]>0)
			{	CurThread=i;
               	burnthread[CurThread]->embers.CurSpot=burnthread[CurThread]->embers.FirstSpot;
               }
			burnthread[i]->embers.CarrySpots=0;
	     }
          SpotFires=0;
     }
}



