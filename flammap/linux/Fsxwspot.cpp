//******************************************************************************
//	FSXWSPOT.CPP	Spot model functions for FARSITE
//
//
//				Copyright 1994, 1995, 1996
//				Mark A. Finney, Systems for Environemntal Management
//******************************************************************************

#include "stdafx.h"
                                    
#include "flm4.hpp"
#include "flammap3.h"
#include <time.h>
//#include <sys\timeb.h>
#include "../Common/rand3.h"

const double PI=acos(-1.0);
//extern const double PI;//=acos(-1.0);

//extern Acceleration AccelConst;

double A[9][4]=
{	 { 15.7, .451, 12.6, .256},
	 { 15.7, .451, 10.7, .278},
	 { 15.7, .451, 6.3, .249},
	 { 12.9, .453, 12.6, .256},
	 { 12.9, .453, 10.7, .278},
	 { 16.5, .515, 10.7, .278},
	 { 2.71, 1.0, 11.9, .389},
	 { 2.71, 1.0, 7.91, .344},
	 { 2.71, 1.0, 13.5, .544},
};


double B[4][2]=
{	 {4.24,.332},
	 {3.64,.391},
	 {2.78,.418},
	 {4.7,0.0},
};


Embers::Embers(FlamMap *_pFlamMap)
: APolygon(_pFlamMap), mech(_pFlamMap)
{
	MaxSpot = 0.0;
	NumEmbers=0;
     CarryOverEmbers=CarrySpots=0;
     NumSpots=0;
     SpotFires=0;
     fe=0;
 //    env=0;
     FirstEmber =new emberdata[16];
     FirstSpot=new spotdata[16];


	 //fCoordDump = fopen("EmbersCoords.txt", "wt");
	//FirstEmber=(emberdata *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, 1*sizeof(emberdata));		// initialize first ember
     //CurEmber=FirstEmber;
	//CurEmber->next=0;
}


Embers::~Embers(void)
{
	//if(fCoordDump)
	//	fclose(fCoordDump);
	mech.MechCalls::~MechCalls();
     /*if(NumSpots>0)
     	SpotReset(NumSpots, FirstSpot);
     if(SpotFires>0)
     	SpotReset(SpotFires, FirstSpot);
     if(NumEmbers>0)
          EmberReset();*/
	if(FirstEmber)
	{
		delete[] FirstEmber;
		FirstEmber = NULL;
	}
	if(FirstSpot)
	{
		delete[] FirstSpot;
		FirstSpot = NULL;
	}
}


void Embers::SetFireEnvironmentCalls( FELocalSite *Fe)
{
//	env=Env;
	fe=Fe;
}


void Embers::fuelcoefs(int fuel, double *coef, double *expon)
{
	switch(fuel)
	{	case 1: *coef=545; *expon=-1.21; break;
		case 2: *coef=709; *expon=-1.32; break;
		case 3: *coef=429; *expon=-1.19; break;
		case 4: *coef=301; *expon=-1.05; break;
		case 5: *coef=235; *expon=-0.92; break;
		case 6: *coef=242; *expon=-0.94; break;
		case 7: *coef=199; *expon=-0.89; break;
		case 8: *coef=262; *expon=-0.97; break;
		case 9: *coef=1121; *expon=-1.51; break;
		case 10: *coef=224; *expon=-0.92; break;
		case 11: *coef=179; *expon=-0.81; break;
		case 12: *coef=163; *expon=-0.78; break;
		case 13: *coef=170; *expon=-0.79; break;
	};
}




void Embers::Loft(double CFlameLength, double CFBurned, double CrownHeight, double /*LoadingBurned*/,
			   double /*ROS*/, double /*SubTimeStep*/, double curtime)
{// This version of Loft attempts to loft 16 embers between .005 and .08 ft diameter
     int count;
	double Diameter;         // ember diameter in feet
	double zo;			// original height ember was lofted
	double ZF;			// height of lofted ember in feet

	Plume(CFlameLength, CFBurned); // compute plume characteristics from torching trees, and temporarily crowning (TEMPORARY!)
	CrownHeight*=3.2808;
	zo=CrownHeight;//GetCrownHeight()*3.28;  	 // initial ember ht at tree top
	NumSpots = 0;
	NumEmbers = 0;
	MaxSpot = 0.0;
	for(count=1; count<17; count++)
	{	Diameter=.005*count;
		switch(SpotSource)
		{    case 0:
				ZF=torchheight(Diameter, zo);    // allow for variable ember starting height
				if(ZF>0)
					ZF+=CrownHeight/2.0;//GetCrownHeight()*1.64;  // 3.28m/2 feet, base of flames at 1/2 tree height, so add to z for total particle height + elevation of ground for altitude
				break;
			case 1:
				ZF=torchheight(Diameter, zo);
				if(ZF>0)
					ZF+=CrownHeight/2.0;//GetCrownHeight()*1.64;  // 3.28m/2 feet, base of flames at 1/2 tree height, so add to z for total particle height + elevation of ground for altitude
				break;
//			case 2:
//				ZF=pileheight(Diameter);
//				if(ZF>0)
//					ZF+=3;                	// ember originate from top of 3m tall pile
//				break;
//			case 3:
//				ZF=                        	// spotting from line fire
//				lineloft(.....);
//				break;
		}
		if(ZF>0)                             	// if ember is lofted by flame
		{
               /*
               if(NumEmbers>0)
          	{	NextEmber=(emberdata *) CurEmber->next=(emberdata *) new emberdata;// GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, 1*sizeof(emberdata));
	               CurEmber=NextEmber;
               }
               else
               {	FirstEmber=(emberdata *) new emberdata;//GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, 1*sizeof(emberdata));		// initialize first ember
			     CurEmber=FirstEmber;
	          }
               */
               CurEmber=&(FirstEmber[NumEmbers]);
               CurEmber->x=Fcoord.x;           	// store ember coordinates
			CurEmber->y=Fcoord.y;           	// UTM meters
			CurEmber->PartDiam=Diameter;    	// ember diameters inches
			CurEmber->ZHeight=ZF/3.2808;      	// ember heights in m
			CurEmber->StartElev=Fcoord.e;	  	// original surface elevation of land from which ember was located
			CurEmber->CurrentTime=curtime;  	// time of ember origination in simulation
			CurEmber->ElapsedTime=0.0;		// time elapsed since ember attained max height
			NumEmbers++;					// calculate total number of embers;
 		}
		else
			count=16;		// force exit from for-loop
	}
}


void Embers::Plume(double CFlameLength, double CFBurned)
{
	double part1, part2, part3, part4;

	if(SpotSource<=1)			// active crown fire or torching trees
	{	int treespecies=pFlamMap->GetCanopySpecies();
		int tnum=1;     				//tnum= # trees simultaneously torching in group
		double DBH=pFlamMap->GetAverageDBH()/2.54;   // cm to inches

		xdiffl=(Fcoord.x-Fcoord.xl)/2.0/pFlamMap->MetricResolutionConvert();   // required to be in meters
		ydiffl=(Fcoord.y-Fcoord.yl)/2.0/pFlamMap->MetricResolutionConvert();
		xdiffn=(Fcoord.x-Fcoord.xn)/2.0/pFlamMap->MetricResolutionConvert();
		ydiffn=(Fcoord.y-Fcoord.yn)/2.0/pFlamMap->MetricResolutionConvert();
		FrontDist=sqrt(pow2(xdiffl)+pow2(ydiffl))+sqrt(pow2(xdiffn)+pow2(ydiffn));	// linear frontal fire distance surrounding x,y coord
		if(SpotSource==0)              		// if active crown fire,
		{	SteadyHeight=CFlameLength*3.28;  	// then steady flameheight = crownfire flame length
			tnum=(int) (FrontDist/5.0); 		// assume 5m mean crown diameter for # trees on firefront
			if(tnum<1) tnum=1;                 // minimum number of trees is 1
			part1=A[treespecies][0];           // INCORRECT, USE DIFFERENT LOFTING MECH FOR ACTIVE CROWN FIRE !!
			part2=A[treespecies][1];           //   ""              ""
			SteadyHeight=part1*pow(DBH,part2)*pow(tnum,0.4);		// steady flame height
		}
		else								// IF TORCHING TREES, THE CALCULATE FLAMEHT FROM ALBINI 1979 AND CHASE 1981
		{    if(CFBurned>0.5 && Fcoord.cover>50)   // cover>2
				tnum++;                       // DEFAULT IS 1-TREE TORCHING INDEPENDENTLY, BUT MORE CROWN BURNED
			if(CFBurned>0.8)				// INCREASES THE NUMBER OF TREES TORCHING TOGETHER with CFB and COVER
			{	tnum++;
				if(Fcoord.cover>50)			// cover>2
					tnum=6;
				if(Fcoord.cover>80)			// cover>3
					tnum=10;
			}
			part1=A[treespecies][0];
			part2=A[treespecies][1];
			SteadyHeight=part1*pow(DBH,part2)*pow(tnum,0.4);	// steady flame height
		}
		part3=A[treespecies][2];
		part4=A[treespecies][3];
		Duration=part3*pow(DBH,-part4)*pow(tnum,-0.2);    	// steady flame Duration
	}
	else
	{	if(SpotSource==2)        						// burning piles or in this case homes or buildings
			SteadyHeight=CFlameLength;

	    /*	else				// wind driven surface fires
		{

		}
		*/
	}
}


void Embers::Flight(double CurTime, double EndCurTimeStep)
{// Iterates ember flight down through logarithmic wind profiles
	double seast=0, snorth=0, tseast=0, tsnorth=0, Diameter=0;
	double sstep=.25, SubSStep=0, curtime, elapsedtime;    					// sstep is the spot time step, in minutes
	//double part1=0, part2=0, part3=0, part4=0, IgChance=0, SmokeDispersionAngle=0;					// H=tree ht in feet,
	double MAXZ=0, Z, mZt=0, Xt=0, mXt=0, Xtot=0;				// mZt is the particle ht at time t, zo is the original height of the particle in tree canopy
	double Z1AboveGround, Z2AboveGround, ZAvgAboveGround;
	double zfuel1, zfuel2, Z1HtFromStart, Z2HtFromStart;
     double ztest1, ztest2, zt1, zt2;
	double voo, tao, DZRate, UH, rwinddir;	// rwinddir=randomwinddirection
	double dxdt, HtLimit, eZelev, Zelev=0;
	int posit, positlast=0;
	int count=0, PassA, PassB;
     int idum;

	//--------------------------------------------------------
	//------Probability of ignition---------------------------

	int IgProb=100.0 * 100;//PercentIgnition(GETVAL)*100;		// in FSXW.CPP
//	int ProbSpot;

	//srand(GetTickCount());
	srand(time(0));
     idum=-((rand()%1000)+1);		// initialize random num gen
	//--------------------------------------------------------

	double eH;
	double TempAccel=mech.A;

	mech.cosslope=99.0;
	mech.RosT=0.0;
	EndCurTimeStep=EndCurTimeStep;//+GetActualTimeStep();     // update Current Time to end of current simulation time step

     if(CarrySpots>0)
     {    CurSpot=CarrySpot;
     	NumSpots=CarrySpots;
     }
     else
     	NumSpots=0;

	for(count=0; count<NumEmbers; count++)		  // for all particles
	{    CurEmber=&(FirstEmber[count]);
		seast=CurEmber->x;
		snorth=CurEmber->y;
		Diameter=CurEmber->PartDiam;			// original particle diameter, stays constant
		Z=CurEmber->ZHeight;				// max height of ember m, stays constant
		Zelev=CurEmber->StartElev;    		// elevation m of land producing ember, stays constant, and zfuel not constant
		curtime=CurEmber->CurrentTime;          // updated current time of ember
		elapsedtime=CurEmber->ElapsedTime; 	// updated time elapsed since ember was at max height
		//CurEmber=NextEmber;
		//if(Diameter>SuccessfulEmberDiameter)
		//	continue;
		//--------------------------------------------------------
		//ProbSpot=rand2(&idum)*10000;//rand()%1000;
		//if(ProbSpot>IgProb)
		//	continue;
		//--------------------------------------------------------
		if(curtime==EndCurTimeStep)			  // >=EndCurTimeStep-actual, wait 2 time steps to see if ember falls with burned zone
		{    /*
               CarryEmber->x=seast;
			CarryEmber->y=snorth;                		// resave ember coordinates
			CarryEmber->PartDiam=Diameter;			// original particle diameter, stays constant
			CarryEmber->ZHeight=Z;					// max height of ember m, stays constant
			CarryEmber->StartElev=Zelev;    			// elevation m of land producing ember, stays constant
			CarryEmber->CurrentTime=curtime;
			CarryEmber->ElapsedTime=elapsedtime;		// copy elapsed time from CurEmber
			NextCarryEmber=(emberdata *) CarryEmber->next;    // use next ember structure
			LastCarryEmber=CarryEmber;
			CarryEmber=NextCarryEmber;
			CarryOverEmbers++;                           // increment number of carryover embers
               */
			continue;								// go to next ember
		}
		else
		{	posit=fe->GetLandscapeData(seast, snorth, mech.ld);
			eH=mech.ld.height*3.2808;//GetCrownHeight()*3.28;  		// metric to feet
			if(eH<=0.0)
				eH=1.0;
			fe->GetFireEnvironment( CurTime, false);//, 0, 1, -1);
               mech.LoadLocalFEData(fe);
			//if(mech.ld.fuel>50)				  // if fuel is pile or building
			//	zfuel1=zfuel2=mech.ld.elev+3;      // 3m high fuel pile
			//else
				zfuel1=zfuel2=mech.ld.elev;
			positlast=posit;
		}
		if(elapsedtime==0)
			Zelev=zfuel1;				// sometimes Zfuel<>zfuel1 becasue of spotoffset aint perimeter in LOFT
		MAXZ=39000.0*Diameter;               // maximum height (ft) particle can fall and still be burning
		//voo=pow((1910.087*Diameter)/0.18,0.5);   // terminal velocity of particle, ft/s;   g=32 ft/sec2, partdens=19lb/ft3, plumedensity=.028 lb/ft3, drag coef=1.2
		voo=sqrt((1910.087*Diameter)/0.18);   // terminal velocity of particle, ft/s;   g=32 ft/sec2, partdens=19lb/ft3, plumedensity=.028 lb/ft3, drag coef=1.2
		tao=(4.8*voo)/(0.0064*PI*32.0);	// gravity= 32 ft/sec2
		Xtot=0;  						// reset total horizontal distance traveled
		Z*=3.28;						// convert to feet above orignial land
		eZelev=Zelev*3.28;				// convert Zelev to feet of original land surface

          zt1=0.0;
		rwinddir=mech.lmw.winddir;
		if(rwinddir < 0.0)
		{
			if(rwinddir == -1.0)
				rwinddir = mech.ld.aspectf;
			else
				rwinddir = PI - mech.ld.aspectf;
		}
		do
		{	ztest1=voo*tao*((elapsedtime*60.0)/tao-.5*pow2((elapsedtime*60.0)/tao));// compute test vertical drop of ember over sstep (0.25 min)
			ztest2=voo*tao*(((sstep+elapsedtime)*60.0)/tao-.5*pow2(((sstep+elapsedtime)*60.0)/tao));// compute test vertical drop of ember over sstep (0.25 min)
			if(ztest1<ztest2)					  // particle burned out before contacting ground
               	DZRate=(ztest2-ztest1)/sstep;                  	// average rate of vertical drop (ft per minute)
			else
			{
				tseast = seast;
				tsnorth = snorth;
				break;
			}
			for(PassA=0; PassA<3; PassA++)
			{    positlast=posit;							// file position of raster
				HtLimit=50.0;								// 50 FT HEIGHT INCREMENT
				for(PassB=0; PassB<2; PassB++)
				{ 	SubSStep=HtLimit/DZRate;					// Sub time step in minutes (ft/(ft/min))
					if(curtime+SubSStep>EndCurTimeStep)		// restrict substep not to exceed current time step
					{	SubSStep=EndCurTimeStep-curtime;
						if(SubSStep<0.0)
						{	SubSStep=0.0;
							curtime=EndCurTimeStep;
						}
						HtLimit=SubSStep*DZRate;				// recalc ht limit in terms of new time step
					}
					zt2=zt1+HtLimit;
					if(zt1<0)
						Z1HtFromStart=eZelev-(Z-zt1);			// EMBER DROPPED BELOW ORIGINAL LAND HEIGHT
					else
						Z1HtFromStart=eZelev+(Z-zt1);
					if(zt2<0)
						Z2HtFromStart=eZelev-(Z-zt2);			// EMBER DROPPED BELOW ORIGINAL LAND HEIGHT
					else
						Z2HtFromStart=eZelev+(Z-zt2);
					Z1AboveGround=Z1HtFromStart-zfuel1*3.28;
					Z2AboveGround=Z2HtFromStart-zfuel2*3.28;
					mZt=Z2AboveGround/3.28;
					if(mZt>1.0)								// 1.0 meter tolerance
					{	UH=VertWindSpeed(Z1AboveGround, eH)/2.0;	// half of UeH in ft/s
						UH+=VertWindSpeed(Z2AboveGround, eH)/2.0;	// average UH in ft/s
						if(Z1AboveGround>0)
						{	if(Z2AboveGround>0)
								ZAvgAboveGround=(Z1AboveGround+Z2AboveGround)/2.0;
							else
								ZAvgAboveGround=Z1AboveGround/2.0;
							if(ZAvgAboveGround>0.1313*eH)
							{	dxdt=UH/2.03*log(ZAvgAboveGround/(.1313*eH));		// dxdt is ft/s
								Xt=dxdt*60.0*SubSStep;      	 //Albini 1979 spotting model, ft/min
								mXt=Xt/3.28;		  		 // incremental distance traveled m/min
							}
							else
								mXt=0.0;
							if(PassB<1 && mXt>(pFlamMap->GetCellResolutionX()/pFlamMap->MetricResolutionConvert()-0.5))
								HtLimit/=2.0;			 	//	SubSStep=(95.12)/(dxdt*60.0);
							else
								PassB+=2;
						}
					}
					else
					{    if(mZt<-1.0)
						{    HtLimit=Z1AboveGround+(3.28*(zfuel1-zfuel2));
								PassB--;
							if(HtLimit<0)				// could happen if ember at zt1 is way below zfuel2 at zt2
							{    mXt=0.0;
								if(!elapsedtime)		// happens when ember is lofted to height below adjacent cell
									PassA=10;
								else
									PassA=6;
								break;
							}
						}
						else
						{	PassA=6;
							break;
						}
					}
				}                       // LOOP WAS while(mXt>29.0);, BUT ONLY WANT 2 PASSES THROUGH THIS LOOP
				if(PassA>7)
					break;
				tseast=seast-mXt*pFlamMap->MetricResolutionConvert()*sin(rwinddir);		// temporary position change, will be permanent if Z iterations ok
				tsnorth=snorth-mXt*pFlamMap->MetricResolutionConvert()*cos(rwinddir);
				if(tseast>=pFlamMap->GetHiEast() || tseast<=pFlamMap->GetLoEast() || tsnorth>=pFlamMap->GetHiNorth() || tsnorth<=pFlamMap->GetLoNorth())
				{	PassA=10;           	  	// EMBER IS OFF THE MAP SURFACE, OUT OF SIMULATION SPACE
					break;
				}
				posit=fe->GetLandscapeData(tseast, tsnorth, mech.ld);
               	//CellData(tseast, tsnorth, &posit);	// retrieve data from nearest raster, including new tree height
				eH=mech.ld.height*3.2808;//GetCrownHeight()*3.28;
				if(eH<=0.0)
					eH=1.0;
				if(posit!=positlast)
				{	posit=fe->GetLandscapeData(tseast, tsnorth, mech.ld);
					eH=mech.ld.height*3.28;//GetCrownHeight()*3.28;
					if(eH<=0.0)
						eH=1.0;
					zfuel1=zfuel2;
					//if(mech.ld.fuel>50)			// if fuel is a building or pile
					//	zfuel2=mech.ld.elev+3;   	// 3m high fuel pile
					//else
						zfuel2=mech.ld.elev;   	// height of structure will be the fuel height at those locations
					fe->GetFireEnvironment ( CurTime, false);//, 0, 1, -1);
                         mech.LoadLocalFEData(fe);
		               rwinddir=mech.lmw.winddir;
					if(rwinddir < 0.0)
					{
						if(rwinddir == -1.0)
							rwinddir = mech.ld.aspectf;
						else
							rwinddir = PI - mech.ld.aspectf;
					}
//					rwinddir=(part4*part1)*SmokeDispersionAngle+mech.lmw.winddir; // >99% of rwinddir will be <= SmoDispAngl
				}
				else
				{	if(zt2>MAXZ)        	// test if ember drop distance has exceeded burnout time
					{	PassA=10;
						break;
					}
					else
						PassA+=2; 		// terminate this loop early
				}
			}
			seast=tseast;
			snorth=tsnorth;
			elapsedtime+=SubSStep;
			Xtot+=Xt;
			zt1=zt2;
			if(PassA==10)
			{    curtime=-1;
				mZt=2.0;					// cause failure of ember
				break;
			}
			else
			{	if(PassA==9)
					mZt=0.0;
				if(curtime<EndCurTimeStep)
					curtime+=SubSStep;
				else
					break;
			}

		/************************************

			if(count<=9000)
			{	filez=fopen(fln1, "a");
					fprintf(filez,"%d %f %f %f %f %f %f \n",count, Diameter, elapsedtime, Xtot, Z1HtFromStart, Z2HtFromStart, 3.28*zfuel2);
					if(mZt<1.0)
					fprintf(filez, "\n");
				fclose(filez);
			}
		//************************************/

		} while(mZt>1.0);  			// DISTANCE TOLERANCE IS 2m ABOVE TARGET IGNITION SURFACE

		if(mZt>=-1.0 && mZt<=1.0 && curtime<EndCurTimeStep)// && NumSpots<19000)  // NumSpot restriction is TEMPORARY
		{	if(mech.ld.fuel>0)						// if not a rock or lake etc.
			{    CurSpot=&(FirstSpot[NumSpots]);
				CurSpot->x = tseast;                       	// store ember coordinates
				CurSpot->y =tsnorth;                      	// UTM meters
				CurSpot->TimeRem=EndCurTimeStep-curtime;
				NumSpots++;
			}
		}
	}
	CarryOverEmbers=NumEmbers=0;
	mech.A=TempAccel;//AccelConst.Line;		// reset surface fire acceleration constant
}


void Embers::EmberReset()
{// resets linked list if simulation interrupted when embers in air (carryover embers)
	if(NumEmbers>0)
	{   // int i;

          delete[] FirstEmber;
		NumEmbers=0;
	}
}


void Embers::SpotReset(int numspots, spotdata* ThisSpot)
{// resets linked list if simulation interrupted or end of timestep
	if(numspots>0)
	{  //  int i;

          delete[] FirstSpot;
		NumSpots=SpotFires=0;
	}
}


double Embers::GetMaxSpotDist(double xpt, double ypt)
{
     int i;
     double MaxDist, Dist, Direction, dx, dy;

     if(NumSpots==0)
	 {
		 MaxSpot = 0.0;
          return 0.0;
	 }

     MaxDist=-1.0;
     for(i=0; i<NumSpots; i++)
     {    CurSpot=&(FirstSpot[i]);
          Dist=pow2(CurSpot->x-xpt)+pow2(CurSpot->y-ypt);
		 // if(fCoordDump)
		 // {
		//	  fprintf(fCoordDump, "xpt = %f, ypt = %f, spotIndex = %ld, CurSpot->x = %f, CurSpot->y = %f, Dist = %f\n",
		//		  xpt, ypt, i, CurSpot->x, CurSpot->y, Dist);
		//  }
          if(MaxDist<Dist)
          {     MaxDist=Dist;
                dx=xpt-CurSpot->x;
                dy=ypt-CurSpot->y;
          }
     }
     if(NumSpots>0)
     {    if(dx==0.0 && dy==0.0)
               Direction=0.0;
          else
          {    Direction=atan2(dx, dy);
               if(Direction<0.0)
                    Direction+=2.0*PI;
          }
          Direction-=PI;                 // convert to direction spots are going TO
          if(Direction<0.0)
               Direction+=2.0*PI;
     }
     NumSpots=SpotFires=0;
     MaxDist=sqrt(MaxDist);
     MaxSpot=(int) MaxDist;
     MaxSpot+=Direction/10.0;      // produce coded distance and direction

     return MaxDist;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

emberdata Embers::ExtractEmber(int type)
{
     memcpy(&TempEmber, FirstEmber, sizeof(emberdata));
	CurEmber=FirstEmber;
     NextEmber=(emberdata *) CurEmber->next;
     delete FirstEmber;//GlobalFree(FirstEmber);
     FirstEmber=NextEmber;
     NumEmbers--;

	return TempEmber;
}

void Embers::AddEmber(emberdata *ember)
{
	if(NumEmbers>0)
     {	NextEmber=(emberdata *) new emberdata;
		CurEmber->next=(emberdata *) NextEmber;
     	CurEmber=NextEmber;
     }
     else
     {	 FirstEmber=(emberdata *) new emberdata;//GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, 1*sizeof(emberdata));		// initialize first ember
	     CurEmber=FirstEmber;
     }
     memcpy(CurEmber, ember, sizeof(emberdata));
     CurEmber->next=0;
     NumEmbers++;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


/*void Embers::Overlap(void)
{// Determines if particle lands inside existing fires or off the map
	int 	i, j, inside;
	int 	newspots, NumVertex;
	double 	Xlo, Xhi, Ylo, Yhi;

     if(CarrySpots==0)
	{	CurSpot=CarrySpot=FirstSpot;			// Set all spot pointers to the first one
		SpotFires=0;						// reinitialize to zero;
          CarrySpots=0;
     }
     else
     {	CurSpot=(spotdata *) CarrySpot->next;
     	CarrySpot=CurSpot;
          SpotFires=CarrySpots;
     }
	for(i=CarrySpots; i<NumSpots; i++)   			// for each ember that landed
	{    startx=CurSpot->x;				// spots array for coords of ember rest position and potential spots
		starty=CurSpot->y;
		inside=0;
		GetNumSpots(&newspots, false);
		for(j=0; j<newspots; j++)		// for each fire
		{    if(GetInout(j)==2)            // search only inward burning fires
			{    NumVertex=GetNumPoints(j);               // get bounding rect
				Xlo=GetPerimeter1Value(j, NumVertex, 0);
				Xhi=GetPerimeter1Value(j, NumVertex, 1);
				Ylo=GetPerimeter1Value(j, NumVertex, 2);
				Yhi=GetPerimeter1Value(j, NumVertex, 3);
				if(startx>Xlo && startx<Xhi)
				{	if(starty>Ylo && starty<Yhi)
					{	if(NumVertex>6)
						{	inside=APolygon::Overlap(j);
							if(inside)
								break;    // ember landed inside inward fire
						}
						else
						{	inside=1;
							break;
						}
					}
				}
			}
		}
		if(!inside)
		{	for(j=0; j<newspots; j++)				// for each fire
			{    if(GetInout(j)==0 || GetNumPoints(j)==0)
               		continue;
               	if(GetInout(j)!=2)            		// search only outward burning fires and barriers
				{    NumVertex=GetNumPoints(j);               // get bounding rect
					Xlo=GetPerimeter1Value(j, NumVertex, 0);
					Xhi=GetPerimeter1Value(j, NumVertex, 1);
					Ylo=GetPerimeter1Value(j, NumVertex, 2);
					Yhi=GetPerimeter1Value(j, NumVertex, 3);
					if(startx>Xlo && startx<Xhi)
					{	if(starty>Ylo && starty<Yhi)
						{	if(NumVertex>6)          // Not within bounds of new spots
							{	inside=APolygon::Overlap(j);
								if(inside)
									break;  		// ember landed inside existing outward fire
							}
							else
							{	inside=1;
								break;
							}
						}
					}
				}
			}
		}
		else
			inside=0;
		if(!inside)                         	// j to newfires forces this function to search for overlap
		{   	CarrySpot->x=startx;	   // save x coordinate to newspots position
		 	CarrySpot->y=starty;        // in Scoord[][] array
		 	CarrySpot->TimeRem=CurSpot->TimeRem-IgnitionDelay(GETVAL);
		 	CarrySpot=(spotdata *) CarrySpot->next;
               SpotFires++;
		}
          NextSpot=(spotdata *) CurSpot->next;
		CurSpot=NextSpot;
	}
     CurSpot=CarrySpot;
     for(i=SpotFires; i<NumSpots; i++)
     {    NextSpot=(spotdata *) CurSpot->next;
          delete CurSpot;
          CurSpot=NextSpot;
     }
     CarrySpots=NumSpots=0;
}
*/

double Embers::torchheight(double partdiam, double zo)
// Iteratively calculates particle height z as a function of part diameter D, flame duration, and flame height
// for torching trees only (ALBINI 1979)
{
	double tf, tt, tT, aT, inc=1.0, inc2=1.0, vowf=-1.0, z=-1.0, zo1;
     double Temp;

	//vowf=40.0*pow(partdiam/SteadyHeight,.5);
     vowf=40.0*sqrt(partdiam/SteadyHeight);
	if(vowf<1.0)
	{	zo1=sqrt(zo/SteadyHeight);
		if(zo1>vowf)
		{    Temp=(1.0-vowf)/(sqrt(zo/SteadyHeight)-vowf);
               if(Temp<0.0)
                    Temp*=-1.0;
               tf=1.0-sqrt(zo/SteadyHeight)+vowf*log(Temp);
			tt=.2*vowf*(1.0+vowf*log(1.0+1.0/(1.0-vowf)));
			aT=Duration+1.2-tf-tt;
			if(aT>=0.0)
			{	//z=SteadyHeight;
				z=++inc*SteadyHeight;
				tT=partcalc(vowf, z);
				while(fabs(tT-aT)>.01)   		// tolerance for iteration is set to .01
				{    if(tT==aT)
						break;
					else
					{    if(tT-aT<0)
							inc+=inc2;
						else
						{	if(tT-aT>0)
							{    inc-=inc2;
								inc2/=2.0;
								inc+=inc2;
							}
						}
					}
					z=inc*SteadyHeight;
					tT=partcalc(vowf, z);
				}
			}
		}
	}
	return z;
}



double Embers::partcalc(double vowf, double z)
// Calculates tT for iterating max particle height z as a function of particle diameter and flameheight (SteadyHeight)
// for torching trees only, function used by torcheight
{
	double r, tT, tp, a=5.963, b=4.563;
     double Temp, r2, pt8vowf;

	r2=(b+z/SteadyHeight)/a;
	r=sqrt(r2);
	//tT=a/3.0*(pow((b+z/SteadyHeight)/a,1.5)-1.0);
     tT=a/3.0*(r*r2-1.0);                  // optimized
     pt8vowf=0.8*vowf;                      // optimized
     Temp=(1.0-pt8vowf)/(1.0-pt8vowf*r);
     if(Temp<0.0)
          Temp*=-1.0;
	//tp=a/pow(.8*vowf,3.0)*(log(Temp)-(.8*vowf)*(r-1.0)-.5*pow(.8*vowf,2.0)*(pow(r,2.0)-1.0));
     tp=a/(pow2(pt8vowf)*pt8vowf)*(log(Temp)-pt8vowf*(r-1.0)-.5*pow2(pt8vowf)*(r2-1.0));
	tT=tp-tT;

	return tT;
}


double Embers::pileheight(double partdiam)
// Iteratively calculates particle height z as a function of particle diameter D and flame height
// for piles only (ALBINI 1981)
{
	double ZoZf, rhs, OldRhs, inc, z=0;
	double lhs=12.428*partdiam;
	double MaxRhs=(.0078*SteadyHeight)*(8.0/3.0)*(1.0-0.625*pow(0.625, 5.0/3.0))*pow(0.625, 2.0/3.0);
	if(lhs<=MaxRhs) 	// if particle can be lifted by max buoyant
	{    ZoZf=40.0;
		inc=ZoZf/2.0-1.6;		// z=1.6 Zf (or 8/5) is about the maximum buoyancy
		OldRhs=rhs=(.0078*SteadyHeight)*(8.0/3.0)*(1.0-0.625*pow(1.0/ZoZf, 5.0/3.0))*pow(1.0/ZoZf,2.0/3.0);
		while(fabs(rhs-lhs)>.01)		//TOLERANCE SET FOR 1/100 FT
		{	if(rhs==lhs)
				break;
			else
			{    if(rhs<lhs)
				{	ZoZf-=inc;
					if(OldRhs>rhs)			// reduce increment if Oldrhs is < new rhs
					{	ZoZf+=inc;
						inc/=2.0;
						ZoZf-=inc;
					}
					while(ZoZf<1.60)          // reduce incremnet if Zf/Z is <.625
					{	ZoZf+=inc;
						inc/=2.0;
						ZoZf-=inc;
					}
				}
				else
				{    ZoZf+=inc;
					if(OldRhs<rhs)			// reduce increment if Oldrhs is < new rhs
					{	ZoZf-=inc;
						inc/=2.0;
						ZoZf+=inc;
					}
				}
				OldRhs=rhs;
				rhs=(.0078*SteadyHeight)*(8.0/3.0)*(1.0-0.625*pow(1.0/ZoZf, 5.0/3.0))*pow(1.0/ZoZf,2.0/3.0);
			}

		}
		z=SteadyHeight*ZoZf;
	}

	return z;
}


/*
double Embers::LineLoft(double fli, int fuel, double partdiam, double *seast, double *snorth, double *SubTimeStep, double *curtime)
// LOFTS EMBERS FROM LINEFIRE AND COMPUTES NEW XY COORDS FROM DRIFT, RETURNS HEIGHT
{
	// compute energy from line fire

	// compute size of thermal from density defect and energy

	// iterate for slipdistance and compute height at which ember exits thermal

	// compute the time required to exit thermal, add to current time and subt from timestep

	return z;
}
*/


double Embers::VertWindSpeed(double HeightAboveGround, double TreeHt)
{// computes windspeed as a function of vertical height above ground
	double UH;

	if(HeightAboveGround<TreeHt)
		UH=mech.lmw.windspd*1.4667;                      			 	// for firebrands at zt<H use mid-flame windspd converted to ft/s
	else                                                         		// Baughman and Albini 1980 (6th conference)
	{  	UH=(mech.lmw.tws*1.4667)/(log((20.0+.36*TreeHt)/(.1313*TreeHt)));   // twindspd converted to feet/s from mph
		//if(HeightAboveGround>TreeHt)                                 	// Albini & Baughman 1979  Res. Pap. INT-221
		//	UH=.493*UH*log(HeightAboveGround/(.1313*TreeHt));		 	// windspd at ember height in ft/s
	}

	return UH;
}





/*

double calc1(int fuel, double fli, double covht, double twindspd)
// CALCULATES MAX PARTICLE HEIGHT USING ALBINI 1979, 1981, 1983, CHASE 1981, MORRIS 1987
{
	double F=0, Y=0, energy=0, SteadyHeight=0, Duration=0, SteadyHeight=10;  // SteadyHeight is in feet
	double part1=0, part2=0, part3=0, part4=0;    // sstep is the spot time step, in seconds
	double DBH=25.0, tnum=1, J=0, coef=0, expon=0, Z=0;
	int SpotSource=0, treespecies=3;        // Xt is the particle distance

	double partheight(double Duration, double SteadyHeight, double partdiam, double zo);

	if(fuel!=50 && covht>0)
		SpotSource=1;	    // torching tree
	else
	{	if(fuel==50)       // house code can be any value 
			SpotSource==2;
		else               // winddriven surface fire
			SpotSource==3;
	}

	if(SpotSource==1)
	{	part1=A[treespecies][0];
		part2=A[treespecies][1];
		part3=A[treespecies][2];
		part4=A[treespecies][3];
		SteadyHeight=part1*pow(DBH,part2)*pow(tnum,.4);
		Duration=part3*pow(DBH,-part4)*pow(tnum,-0.2);
    
		Y=covht/SteadyHeight;
 		if(Y>=1.0)
			J=0;
		else
		{	if(Y>=0.5 && Y<1.0)
				J=1;
			else
			{	if(Y<.5)
				{	if(Duration<3.5)
						J=2;
					else
						J=3;
				}
			}
		}
		part1=B[J][0];
		part2=B[J][1];
		Z=SteadyHeight*part1*pow(Duration,part2)+covht/2;
	}
	else
	{    if(SpotSource==2)	      	// piles or houses in this case
			Z=SteadyHeight*12.2;		// need continuous flame height from somewhere 
		else
		{    fuelcoefs(fuel, &coef, &expon);
			F=coef*pow(.474*twindspd,expon);	 // wind driven surface fires
			energy=fli*F;
			Z=1.055*sqrt(energy);
		}
	}
	return Z;
}



double calc2(double Z, double covht, double twindspd, int fuel);
// CALCULATES MAXIMUM FLAT TERRAIN SPOTTING DISTANCE
{
	double htcrit=0, htused=0, drift=0, distf=0;

	htcrit=2.2*pow(Z,.337)-4;
	if(covht>htcrit)
		htused=covht;
	else
		htused=htcrit;

	if(fuel!=50 && covht==0)					// winddriven surface fire
		drift=.000278*twindspd*pow(Z,.643);
	else
		drift=0;
	distf=.000718*twindspd*sqrt(htused)*(.362+sqrt(Z/htused)/2.0*log(Z/htused))+drift;

	return distf;
}


 */





