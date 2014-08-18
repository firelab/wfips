
#include "stdafx.h"
//------------------------------------------------------------------------------
//
//   fsxwfms2.cpp
//   FARSITE 4.0
//   10/20/2000
//   Mark A. Finney (USDA For. Serv., Fire Sciences Laboratory)
//
//------------------------------------------------------------------------------

#include "flammap3.h"
#include "fsxsync.h"
//#include <windows.h>
//#include <process.h>
#include <math.h>
 
//extern const double PI;
static const double PI=acos(-1.0);
static int WindLoc;


/********************************************************************************/
FELocalSite::FELocalSite(FlamMap *_pFlamMap) : LandscapeData(_pFlamMap)
{
	StationNumber=-1;
     WindLoc=0;
//	SetCanopyChx(15.0, 4.0, 0.2, 30.0, 100, 2, 1); // for each thread
}

/*************************************************************************************/
int FELocalSite::GetLandscapeData(double xpt, double ypt, LandscapeStruct &ls)
{
     if(xpt<pFlamMap->GetLoEast() || xpt>pFlamMap->GetHiEast() || ypt<pFlamMap->GetLoNorth() || ypt>pFlamMap->GetHiNorth())
     	return 0;
     int64_t posit;
	celldata cell;
     crowndata crown;
     grounddata ground;

	pFlamMap->CellData(xpt, ypt, cell, crown, ground, &posit);
	ElevConvert(cell.e);				// need to convert units and codes
	SlopeConvert(cell.s);
	AspectConvert(cell.a);
	FuelConvert(cell.f);
	CoverConvert(cell.c);
     HeightConvert(crown.h);
     BaseConvert(crown.b);
     DensityConvert(crown.p);
    	DuffConvert(ground.d);
	WoodyConvert(ground.w);
	StationNumber=0;//GetStationNumber(xpt, ypt)-1;
     XLocation=xpt;
     YLocation=ypt;
     ls=ld;

	return posit;
}

/***********************************************************************************************/
void FELocalSite::GetEnvironmentData(EnvironmentData *mw)
{
	(*mw).ones=onehour;
	(*mw).tens=tenhour;
	(*mw).hundreds=hundhour;
	(*mw).liveh=(double) pFlamMap->GetInitialFuelMoisture(ld.fuel, 3)/100.0;
	(*mw).livew=(double) pFlamMap->GetInitialFuelMoisture(ld.fuel, 4)/100.0;
	(*mw).windspd=mwindspd;
	(*mw).tws=twindspd;
     if(wwinddir>=0.0)
		(*mw).winddir=wwinddir/180.0*PI;
     else
		(*mw).winddir=wwinddir;
     (*mw).thousands=thousands;
     (*mw).tenthous=tenthous;
     (*mw).temp=airtemp;
     (*mw).humid=relhumid;
     (*mw).solrad=solrad;
}

/*************************************************************************************/
void FELocalSite::GetFireEnvironment( double SimTime, bool All)
{
 //int    date, hour;
int  l_Elev; 
// double hours; 
double d, Radiate=0.0, EquilMx;

  tenhour=hundhour=onehour=thousands=solrad=mwindspd=0.0;
	 if ( ld.elev==-9999 )
     	return;

  twindspd=pFlamMap->GetConstWindSpeed();
  wwinddir=pFlamMap->GetConstWindDir();
	 if (wwinddir == -1) { //uphill winds
	  	if (ld.aspect < 0) {
		   	twindspd = 0;
			   wwinddir = 0; }
	  	else
		   	wwinddir = ld.aspect;
 	}
 	else if(wwinddir == -2) { //downhill winds
		  if ( ld.aspect < 0) {
		   	twindspd = 0;
			   wwinddir = 0; }
	  else	{
			  wwinddir = ld.aspect - 180;
			  if (wwinddir < 0)
				   wwinddir += 360;}
	 }
	 else if (pFlamMap->hasGriddedWinds)	{
		  wwinddir = pFlamMap->GetWindGridDirByCoord(XLocation, YLocation);
		  twindspd = pFlamMap->GetWindGridSpeedByCoord(XLocation, YLocation);
 	}
  windreduct();
  if (pFlamMap->ConstFuelMoistures(GETVAL)) {  
    onehour = pFlamMap->GetInitialFuelMoisture(ld.fuel, SIZECLASS_1HR)/100.0;
    tenhour = pFlamMap->GetInitialFuelMoisture(ld.fuel, SIZECLASS_10HR)/100.0;
    hundhour = thousands=pFlamMap->GetInitialFuelMoisture(ld.fuel, SIZECLASS_100HR)/100.0;
    thousands=pFlamMap->GetInitialFuelMoisture(ld.fuel, SIZECLASS_100HR)/100.0;
    return; }

//	 date = pFlamMap->Chrono(SimTime, &hour, &hours, false);				// get hours days etc.
 
	 if (ld.fuel<1)
    tenhour=hundhour=onehour=thousands=0.0;
	 else {	
    if (All)
      Radiate=1;

//ALM    tenhour = pFlamMap->cfmc.GetMx(SimTime, ld.fuel, ld.elev, ld.slope, ld.aspectf, ld.cover, &EquilMx, &Radiate, SIZECLASS_10HR);	// Radiate is not returned if set to 0
// old   tenhour=env->GetMx(SimTime, ld.fuel, ld.elev, ld.slope, ld.aspectf, ld.cover, &EquilMx, &Radiate, SIZECLASS_10HR);	// Radiate is not returned if set to 0
 
		  onehour=(4.0*EquilMx+tenhour)/5.0;
    solrad=Radiate;
    Radiate=0.0;

//ALM    hundhour = pFlamMap->cfmc.GetMx(SimTime, ld.fuel, ld.elev, ld.slope, ld.aspectf, ld.cover, &EquilMx, &Radiate, SIZECLASS_100HR);	// Radiate is not returned if set to 0
//ALM    thousands= pFlamMap->cfmc.GetMx(SimTime, ld.fuel, ld.elev, ld.slope, ld.aspectf, ld.cover, &EquilMx, &Radiate, SIZECLASS_1000HR);	// Radiate is not returned if set to 0
// old   hundhour=env->GetMx(SimTime, ld.fuel, ld.elev, ld.slope, ld.aspectf, ld.cover, &EquilMx, &Radiate, SIZECLASS_100HR);	// Radiate is not returned if set to 0
// old  thousands=env->GetMx(SimTime, ld.fuel, ld.elev, ld.slope, ld.aspectf, ld.cover, &EquilMx, &Radiate, SIZECLASS_1000HR);	// Radiate is not returned if set to 0
 
    tenthous=0.30;
  } /* else */

/* ld.elev was previously converted to meters, now we need elev in feet */
  if (All) { 
     l_Elev = ( (double)ld.elev * 3.2808 ) + 0.5;       
//ALM     pFlamMap->cfmc.SiteSpecific (l_Elev, &airtemp, &relhumid); 
// old   env->fmsthread[0].SiteSpecific(env->elevref-ld.elev*3.2808, env->tempref, &airtemp, env->humref, &relhumid);
// old   env->fmsthread[0].SiteSpecific (env->elevref - (l_Elev), env->tempref, &airtemp, env->humref, &relhumid); 
  } /* if (All) */
}

/************************************************************************************/
void FELocalSite::windreduct()
{// FUNCTION TO REDUCE WINDSPEED (MI/HR) TO MIDFLAME OR VEGETATION HEIGHT

	double ffactor, htfuel, htflame, m1, m2;
	double canopyht=ld.height*3.2808;//GetDefaultCrownHeight()*3.28;     // convert to feet

	if(ld.cover<=0 || canopyht<=0.0) // cover==99
	{    if(ld.fuel>0)
		{	switch(ld.fuel)
			{	case 1: htfuel=1.0; break;
				case 2: htfuel=1.0; break;
				case 3: htfuel=2.5; break;
				case 4: htfuel=6.0; break;
				case 5: htfuel=2.0; break;
				case 6: htfuel=2.5; break;
				case 7: htfuel=2.5; break;
				case 8: htfuel=0.2; break;
				case 9: htfuel=0.2; break;
				case 10: htfuel=1.0; break;
				case 11: htfuel=1.0; break;
				case 12: htfuel=2.3; break;
				case 13: htfuel=3.0; break;
				default: htfuel=pFlamMap->GetFuelDepth(ld.fuel);  // retrieve from cust models
					if(htfuel==0.0)
						htfuel=0.01;
					break;
			}
		}
		else
			htfuel=0.01;		// no fuel so height is essentially zero
		htflame=htfuel;		// from Baughman and Albini 6th conf. FFM 1980
		m1=(1+.36*(htfuel/htflame))/(log((20+.36*htfuel)/(0.13*htfuel)));
		m2=log(((htflame/htfuel+.36)/.13))-1;
		mwindspd=m1*m2*twindspd;
	}
	else
	{    ffactor=((double) ld.cover/100.0)*0.33333; // volume ratio of cone to cylinder
          ffactor*=PI/4.0;                        // area ratio of circle to square

		m1=.555/(sqrt(canopyht*ffactor)*log((20+.36*canopyht)/(.13*canopyht)));
		mwindspd=m1*twindspd;
	}
}




