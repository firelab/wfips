#include "stdafx.h"
//#include "fswindow.h"
#include "flammap3.h"

const double PI=acos(-1.0);

LandscapeData::LandscapeData(FlamMap *_pFlamMap)
{
	pFlamMap = _pFlamMap;
}

/*FECalls::FECalls()
{
	FireMW=0;
}

                                   
FECalls::~FECalls()
{
	FreeFireMW();
}


void FECalls::ResetData()
{
	gmw.ones=0;
	gmw.tens=0;
	gmw.hundreds=0;
	gmw.livew=0;
	gmw.liveh=0;
	gmw.windspd=0;
	gmw.winddir=0;
	gmw.tws=0;
	lmw.ones=0;
	lmw.tens=0;
	lmw.hundreds=0;
	lmw.livew=0;
	lmw.liveh=0;
	lmw.windspd=0;
	lmw.winddir=0;
	lmw.tws=0;
}

int FECalls::GetLandscapeData(double xpt, double ypt, LandscapeStruct& ls)
{
     if(xpt<GetLoEast() || xpt>GetHiEast() || ypt<GetLoNorth() || ypt>GetHiNorth())
     	return 0;
     XLocation=xpt;		// copy coordinates to FireEnvironment::Variables for use with gridded data
     YLocation=ypt;

	celldata cell;
     crowndata crown;
     grounddata ground;

	CellData(xpt, ypt, cell, crown, ground, &posit);
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
	StationNumber=GetStationNumber(xpt, ypt)-1;
	ls=ld;

	return posit;
}*/


void LandscapeData::FuelConvert(short fl)
{
	ld.fuel=pFlamMap->GetFuelConversion(fl);
}


void LandscapeData::ElevConvert(short el)
{
     ld.elev=el;
	if(ld.elev==-9999)
     	return;
          
	switch(pFlamMap->GetTheme_Units(E_DATA))
	{    case 0:	// meters default
			break;
		case 1:	// feet
			ld.elev/=3.2804;
			break;
	}
}

void LandscapeData::SlopeConvert(short sl)
{
	ld.slope=sl;
     if(ld.slope==-9999)
     	return;

	switch(pFlamMap->GetTheme_Units(S_DATA))
	{	case 0:  // degrees default
			break;
		case 1:  // percent
               double fraction, ipart;
               double slopef;

			ld.slope=slopef=atan((double) ld.slope/100.0)/PI*180.0;
               fraction=modf(slopef, &ipart);
               if(fraction>=0.5)
                    ld.slope++;
			break;
	}
}


void LandscapeData::AspectConvert(short as)
{
     if(as==-9999)
     {	ld.aspect=0;
     	ld.aspectf=0.0;
     	return;
     }
	ld.aspect=as;
	ld.aspectf=(double) as;
	switch(pFlamMap->GetTheme_Units(A_DATA))
	{	case 0:		// grass 1-25 counterclockwise from east
			if(ld.aspect!=25)
			    ld.aspectf=(2.0*PI-(ld.aspectf/12.0*PI))+(7.0*PI/12.0);  /* aspect from GRASS, east=1, north=7, west=13, south=19 */
			else
			{	ld.aspectf=25.0;
				ld.slope=0;
			}
			break;
		case 1:		// degrees 0 to 360 counterclockwise from east
			ld.aspectf=(2.0-ld.aspectf/180.0)*PI+PI/2.0;
			break;
		case 2:
			ld.aspectf=ld.aspectf/180.0*PI;     // arcinfo, degrees AZIMUTH
			break;
	}
	if(ld.aspectf>(2.0*PI))
		ld.aspectf-=(2.0*PI);
}



void LandscapeData::CoverConvert(short cov)
{
	ld.cover=cov;
	if(pFlamMap->GetTheme_Units(C_DATA)==0)
	{	switch(ld.cover)
		{	case 99: ld.cover=0; break;
			case 1:  ld.cover=10; break;
			case 2:  ld.cover=30; break;
			case 3:  ld.cover=60; break;
			case 4:  ld.cover=75; break;
			default: ld.cover=0; break;
		}
	}
}


void LandscapeData::HeightConvert(short height)
{
	//printf("in HeightConvert height = %i\n", height);
     if(pFlamMap->HaveCrownFuels())
     {	if(height>=0)
		{    short units=pFlamMap->GetTheme_Units(H_DATA);

          	ld.height=(double) height/10.0;
           	if(units==2 || units==4)
               	ld.height/=3.280839;
			if(ld.height>100.0)
	               ld.height/=10.0;	// probably got wrong units
     	}
     	else
          	ld.height=pFlamMap->GetDefaultCrownHeight();
     }
    	else
         	ld.height=pFlamMap->GetDefaultCrownHeight();
}

void LandscapeData::BaseConvert(short base)
{
     if(pFlamMap->HaveCrownFuels())
     {	if(base>=0)
		{    short units=pFlamMap->GetTheme_Units(B_DATA);
          	ld.base=(double) base/10.0;
           	if(units==2 || units==4)
               	ld.base/=3.280839;
			if(ld.base>100)
               	ld.base/=10.0;	// probably got wrong units
     	}
     	else
          	ld.base=pFlamMap->GetDefaultCrownBase();
     }
    	else
         	ld.base=pFlamMap->GetDefaultCrownBase();
}

void LandscapeData::DensityConvert(short density)
{
     if(pFlamMap->HaveCrownFuels())
     {	if(density>=0)
		{    short units=pFlamMap->GetTheme_Units(P_DATA);
          	ld.density=((double) density)/100.0;
           	if(units==2 || units==4)
               	ld.density*=1.61845;	// convert 10lb/ft3 to kg/m3
			if(ld.density>1.0)
               	ld.density/=100.0;	// probably got wrong units
               if(pFlamMap->LinkDensityWithCrownCover(GETVAL))
               	ld.density*=((double) ld.cover)/100.0;
     	}
     	else
          	ld.density=pFlamMap->GetDefaultCrownBD(ld.cover);
     }
    	else
         	ld.density=pFlamMap->GetDefaultCrownBD(ld.cover);
}

void LandscapeData::DuffConvert(short duff)
{
	if(pFlamMap->HaveGroundFuels())
     {	if(duff>=0)
     	{    ld.duff=(double) duff/10.0;
          	if(pFlamMap->GetTheme_Units(D_DATA)==2)
	              	ld.duff*=2.2417088978002777;
          }
          else
          	ld.duff=0.0;
     }
     else
     	ld.duff=0.0;
}

void LandscapeData::WoodyConvert(short woody)
{
	if(pFlamMap->HaveGroundFuels())
		ld.woody=woody;
     else
     	ld.woody=0;
}


/*void FECalls::GetFireEnvironment(double SIMTIME, int type, int ReadNewData, int PointNum)
{
     int oldfuel=ld.fuel;

     if(ld.fuel<0)
     {	ld.fuel=51;
      	StationNumber=0;
     }
	if(InitialFuelMoistureIsHere(ld.fuel))
     {	if(ReadNewData)
			envt(SIMTIME);
		if(type)
			globaldata(ReadNewData, PointNum);
		else
			localdata();
	}
	else
		NoMoistureData(type);
	ld.fuel=oldfuel;
}


void FECalls::globaldata(int ReadNewData, int PointNum)
{
	if(ReadNewData)
	{	gmw.ones=ohmc/100;
		gmw.tens=thmc/100;
		gmw.hundreds=hhmc/100;
		gmw.livew=(double) GetInitialFuelMoisture(ld.fuel, 4)/100.0;
		gmw.liveh=(double) GetInitialFuelMoisture(ld.fuel, 3)/100.0;
		gmw.windspd=mwindspd;
		gmw.tws=twindspd; //WStat[StationNumber].tws=twindspd;
		gmw.winddir=wwinddir/180.0*PI; //WStat[StationNumber].winddir=wwinddir/180*PI;

		if(PointNum>=0)
		{	if(FireMW==0)
				AllocFireMW(ReadNewData);
			FireMW[PointNum]=gmw;
		}
	}
	else
		gmw=FireMW[PointNum];
}


int FECalls::AllocFireMW(int NumPoints)
{
	if(!FireMW)
	{	if((FireMW=(EnvironmentData *) GlobalAlloc(GMEM_FIXED, NumPoints*sizeof(EnvironmentData)))!=NULL)
			return 1;
	}

	return 0;
}


void FECalls::FreeFireMW()
{
	if(FireMW)
		GlobalFree(FireMW);
	FireMW=0;
}



void FECalls::localdata(void)
{
	lmw.ones=ohmc/100;
	lmw.tens=thmc/100;
	lmw.hundreds=hhmc/100;
	lmw.livew=(double) GetInitialFuelMoisture(ld.fuel, 4)/100.0;
	lmw.liveh=(double) GetInitialFuelMoisture(ld.fuel, 3)/100.0;
	lmw.windspd=mwindspd;
	lmw.tws=twindspd;
	lmw.winddir=wwinddir/180*PI;
}


void FECalls::NoMoistureData(int type)
{
	if(type)				// local data
	{	gmw.ones=0.5;
		gmw.tens=0.5;
		gmw.hundreds=0.5;
		gmw.livew=1.5;
		gmw.liveh=1.5;
		gmw.windspd=0.0;
		gmw.tws=0.0;
		gmw.winddir=0.0;
	}
	else
	{	lmw.ones=0.5;
		lmw.tens=0.5;
		lmw.hundreds=0.5;
		lmw.livew=1.5;
		lmw.liveh=1.5;
		lmw.windspd=0.0;
		lmw.tws=0.0;
		lmw.winddir=0.0;
	}
}


double FECalls::PIgnite(void)
{
	double mf=ohmc/100, Qig, ProbIgnite, FT=(5.0/9.0)*(FuelTemp-32.0);  // convert to deg C

	Qig=144.51-.266*FT-.00058*pow2(FT)+(mf/100)*(100-FT)+18.54*(1.0-exp(-15.1*mf))+640*mf;
	ProbIgnite=.000932*pow((344-Qig)/10,3.66)/50.0;

	return ProbIgnite;
}*/


/*FireEnvironment::FireEnvironment(FlamMap *_pFlamMap) : LandscapeData(_pFlamMap)
{
	StationNumber=1;
}


FireEnvironment::~FireEnvironment()
{

}


void FireEnvironment::envt(double SIMTIME)
{
	int thisdate, curdate, curhour, date, hour, nearesthour, lastdate;
	double xemc, D, Pd=0, elevation;
	double humid, temp, I, shade=100, rain=0;
	double sunrise, sunset, tempref=60, humref=50;
	double emcmax, emcmin, humidmx, humidmn, tempmx, tempmn, dayhours, nitehours;
	int elevref,  cloud=0;
     int AtmGridExists=pFlamMap->AtmosphereGridExists();
     double LagRate;

     FILE *radout;
     radout=fopen("Radout.txt", "a");

	elevation=(double) ld.elev*3.28;		// m to ft. conversion
	date=chrono(SIMTIME, &hour);	// determines date and hour of current iteration
	if(hour>=1400)
		lastdate=date;
	else
		lastdate=date-1;
	curhour=1400;
	thmc=pFlamMap->GetInitialFuelMoisture(ld.fuel, 1);//FuelMoisture.TL10[fuel-1];
	hhmc=pFlamMap->GetInitialFuelMoisture(ld.fuel, 2);//FuelMoisture.TL100[fuel-1];
	for(curdate=pFlamMap->GetStartDate(); curdate<=lastdate; curdate++)
	{    thisdate=curdate;
     	if(thisdate>365)
          	thisdate-=365;
     	daylength(thisdate, &sunrise, &sunset);
     	if(AtmGridExists==2)
          {	AtmHumTemp(thisdate, curhour, &temp, &humid, &tempmn, &humidmn, &rain, &cloud);
	          humidmx=humid;
	          tempmx=temp;
          }
     	else	// interpolate humidity and temperature from hi/lo
          {   	humtemp(thisdate, curhour, &tempref, &humref, &elevref, &rain, &humidmx, &humidmn, &tempmx, &tempmn);
//			rain/=100.0;
			sitespec(elevref, elevation, tempref, &temp, humref, &humid);	// adjusts to site specific temp and relhum at a given pixel
          }
		AirTemp=temp; AirHumid=humid;
          if(AtmGridExists==0)
			windadj(thisdate, curhour, &cloud);			// retrieves wind information and adjusts to fuel-level conditions
          else
          	AtmWindAdjustments(thisdate, curhour, &cloud);
		I=irradiance(cloud, thisdate, curhour, &shade);
		if(shade<70)	// adjust humidity and temperature at fuel level only for shade <70%
			adjust(&humid, &temp, I);
		sfmc(rain, humid, temp);
          if(AtmGridExists<2)
          {	sitespec(elevref, elevation, tempmn, &tempmn, humidmx, &humidmx);
			sitespec(elevref, elevation, tempmx, &tempmx, humidmn, &humidmn);
		}
		if(shade<70)
			adjust(&humidmn, &tempmx, I);
		emcmax=emc(humidmx, tempmx);
		emcmin=emc(humidmn, tempmn);
		dayhours=sunset-sunrise;      // using NFDRS 1978 equations 
		nitehours=24-dayhours;
		xemc=(nitehours*emcmax + dayhours*emcmin)/24;
		if(rain>0)
			Pd=(rain+.02)/.05;
		D=((24.0-Pd)*xemc+(.5*Pd+41.0)*Pd)/24.0;
		hhmc=hhmc+(D-hhmc)*(1.0-.87*exp(-24.0/100.0));    // could put thousand hour moisture calculation in here
	}
	curdate--;
     if(curdate<1)
     	curdate=365;
     thisdate=curdate;
     if(thisdate>365)
         	thisdate-=365;
     nearesthour=(hour/100)*100;
	if(nearesthour!=1400)
	{ 	while(curhour!=nearesthour)	// calcs thmc for all hours since last day
		{	curhour+=100;
			if(curhour>=2400)
			{	curhour-=2400;
				curdate=date;
                    thisdate=curdate;
     			if(thisdate>365)
          			thisdate-=365;
			}
			daylength(thisdate, &sunrise, &sunset);			// interpolates humidity and temperature from hi/lo
               if(AtmGridExists==2)
	     		AtmHumTemp(thisdate, curhour, &temp, &humid, &tempmn, &humidmn, &rain, &cloud);
               else
               {	humtemp(thisdate, curhour, &tempref, &humref, &elevref, &rain, &humidmx, &humidmn, &tempmx, &tempmn);
				sitespec(elevref, elevation, tempref, &temp, humref, &humid);	// adjusts to site specific temp and relhum at a given pixel
               }
			AirTemp=temp; AirHumid=humid;
	          if(AtmGridExists==0)
				windadj(thisdate, curhour, &cloud);			// retrieves wind information and adjusts to fuel-level conditions
	     	else
     	     	AtmWindAdjustments(thisdate, curhour, &cloud);
			if(curhour/100<sunset && curhour/100>sunrise)
			{    I=irradiance(cloud, thisdate, curhour, &shade);
				fprintf(radout, "%d %lf\n", curhour, I);
				if(shade<70)	 // adjust humidity and temperature at fuel level only for shade <70%
					adjust(&humid, &temp, I);
			}
			hffmc(humid, temp);  // tenhour fuel moisture content
		}
	}
	if(nearesthour!=hour)
	{	if(AtmGridExists==0)
	     	windadj(thisdate, hour, &cloud);
     	else
          	AtmWindAdjustments(thisdate, hour, &cloud);
    }
	xemc=emc(humid, temp);
	ohmc=(4.0*xemc+thmc)/5.0;     // one hour fuel mc 
     if(SIMTIME<241.0)
     {    LagRate=1.0-1.0/exp(SIMTIME/60.0);
          //ohmc=FuelMoisture.TL1[fuel-1]-LagRate*(FuelMoisture.TL1[fuel-1]-ohmc);
          ohmc=pFlamMap->GetInitialFuelMoisture(ld.fuel, 1)-LagRate*(pFlamMap->GetInitialFuelMoisture(ld.fuel, 1)-ohmc);
     }
	FuelTemp=temp;
//	PtTemperature=temp;
//	PtHumidity=humid;
	PtTemperature=AirTemp;
	PtHumidity=AirHumid;

     fclose(radout);
}


void FireEnvironment::daylength(int date, double *sunrise, double *sunset)
// COMPUTES DAY LENGTH (WITHOUT LEAP YEARS 
{
	double delta, flatitude, hour, temp;

	flatitude=(double (pFlamMap->GetLatitude()))/180.0*PI;
	delta=23.45*sin((0.9863*(284.0+date))/180.0*PI);
	delta=delta/180.0*PI;
     temp=(-sin(flatitude)*sin(delta))/(cos(flatitude)*cos(delta));
     if(fabs(temp)>1.0)
     	temp=1.0;
     hour=asin(temp);

	*sunrise=(hour/PI*180)/15+6;
	*sunset=((1-hour/PI)*180)/15+6;
}


void FireEnvironment::AtmHumTemp(int date, int hour, double *temp, double *humid,
						   double *tempmin, double *humidmin, double *rain, int *cloud)
{
	int count=-1;
     int ddate, month, day, hhour, xmonth, xday;
     short humidx, tempx, ppt, twspd, wndir, cloud_s;
     short humidmin1, humidmin2, tempmin1, tempmin2;
	double MinTimeStep;

     do
     {    count++;
     	month=pFlamMap->GetAtmosphereGrid()->GetAtmMonth(count);
          if(month==-1)					// hit end of ATMDATA
          	break;
          day=pFlamMap->GetAtmosphereGrid()->GetAtmDay(count);
          ddate=day+pFlamMap->GetJulianDays(month);
	} while(ddate!=date);
     if(month!=-1)						// if hit end of ATMDATA data
	{	tempmin1=-1;
          humidmin1=-1;
          do
		{    xmonth=pFlamMap->GetAtmosphereGrid()->GetAtmMonth(count);
			xday=pFlamMap->GetAtmosphereGrid()->GetAtmDay(count);
			hhour=pFlamMap->GetAtmosphereGrid()->GetAtmHour(count);
			if(xday>day || xmonth>month)
				break;
               if(hour==1400)	// use 1400 obs as max
               {	pFlamMap->GetAtmosphereGrid()->GetAtmosphereValue(ATMTEMP, XLocation, YLocation, count, &tempmin2);
	     	     pFlamMap->GetAtmosphereGrid()->GetAtmosphereValue(ATMHUMID, XLocation, YLocation, count, &humidmin2);
                    if(tempmin1==-1)
                    {	tempmin1=tempmin2;
                    	humidmin1=humidmin2;
                    }
                    else
                    {	if(tempmin2<tempmin1)
                    		tempmin1=tempmin2;
                    	if(humidmin2<humidmin1)
                         	humidmin1=humidmin2;
                    }
               }
               count++;
		} while(hour>=hhour);
          --count;
          pFlamMap->GetAtmosphereGrid()->GetAtmosphereValue(ATMTEMP, XLocation, YLocation, count, &tempx);
          pFlamMap->GetAtmosphereGrid()->GetAtmosphereValue(ATMHUMID, XLocation, YLocation, count, &humidx);
          pFlamMap->GetAtmosphereGrid()->GetAtmosphereValue(ATMRAIN, XLocation, YLocation, count, &ppt);
          *temp=(double) tempx;
          *humid=(double) humidx;
          *rain=((double) ppt)/100.0;
		*tempmin=(double) tempmin1;
          *humidmin=(double) humidmin1;
          pFlamMap->GetAtmosphereGrid()->GetAtmosphereValue(ATMWSPD, XLocation, YLocation, count, &twspd);
          pFlamMap->GetAtmosphereGrid()->GetAtmosphereValue(ATMWDIR, XLocation, YLocation, count, &wndir);
          pFlamMap->GetAtmosphereGrid()->GetAtmosphereValue(ATMCLOUD, XLocation, YLocation, count, &cloud_s);
          *cloud=cloud_s;
          twindspd=(double) twspd;
          wwinddir=(double) wndir;
          windreduct();
	}
	if(hour!=1400 && hours<=hhour)
	{	MinTimeStep=(double) hhour-hours;
		EventMinimumTimeStep(MinTimeStep);         // load into global variable
	}
}


void FireEnvironment::AtmWindAdjustments(int curdate, int hour, int *cloud)
{
	int count=-1, ddate, month, day, hhour, xmonth, xday;
     short twspd, wndir, cloud_s;
	double MinTimeStep;

     do
     {    count++;
     	month=pFlamMap->GetAtmosphereGrid()->GetAtmMonth(count);
          if(month==-1)					// hit end of ATMDATA
          	break;
          day=pFlamMap->GetAtmosphereGrid()->GetAtmDay(count);
          ddate=day+pFlamMap->GetJulianDays(month);
	} while(ddate!=curdate);
     if(month!=-1)						// if hit end of ATMDATA data
	{	do
		{    xmonth=pFlamMap->GetAtmosphereGrid()->GetAtmMonth(count);
			xday=pFlamMap->GetAtmosphereGrid()->GetAtmDay(count);
			hhour=pFlamMap->GetAtmosphereGrid()->GetAtmHour(count);
			if(xday>day || xmonth>month)
				break;
               count++;
		} while(hour>=hhour);
          --count;
          pFlamMap->GetAtmosphereGrid()->GetAtmosphereValue(ATMWSPD, XLocation, YLocation, count, &twspd);
          pFlamMap->GetAtmosphereGrid()->GetAtmosphereValue(ATMWDIR, XLocation, YLocation, count, &wndir);
          pFlamMap->GetAtmosphereGrid()->GetAtmosphereValue(ATMCLOUD, XLocation, YLocation, count, &cloud_s);
          *cloud=cloud_s;
          twindspd=(double) twspd;
          wwinddir=(double) wndir;
          windreduct();
	}
	if(hours<=hhour)
	{	MinTimeStep=(double) hhour-hours;
		EventMinimumTimeStep(MinTimeStep);         // load into global variable
	}
}



void FireEnvironment::humtemp(int date, int hour, double *tempref, double *humref, int *elevref, double *rain, double *humidmx, double *humidmn, double *tempmx, double *tempmn)
// FINDS AND INTERPOLATES HUMIDTY AND TEMPERATURE FOR CURRENT HOUR 
{
	int count=-1, ddate=-1, month, day, hmorn, haft, Tmin, Tmax, Hmax, Hmin;
	int elref, hx, garbage, ppt;
	double h1, h2, dtprime, dtmxmn, humid, temp, tempf, humf, sign;
//	double dewptref, dewpt;

	while(ddate!=date)
	{    count++;
		month=pFlamMap->GetWeatherMonth(StationNumber, count);
		day=pFlamMap->GetWeatherDay(StationNumber, count);
          ddate=day+pFlamMap->GetJulianDays(month);
	}
	hmorn=pFlamMap->GetWeatherTime1(StationNumber, count);
	haft=pFlamMap->GetWeatherTime2(StationNumber, count);
	ppt=pFlamMap->GetWeatherRain(StationNumber, count);
	Tmin=pFlamMap->GetWeatherTemp1(StationNumber, count);
	Tmax=pFlamMap->GetWeatherTemp2(StationNumber, count);
	Hmax=pFlamMap->GetWeatherHumid1(StationNumber, count);
	Hmin=pFlamMap->GetWeatherHumid2(StationNumber, count);
	elref=pFlamMap->GetWeatherElev(StationNumber, count);
	if(hour>haft)
	{	count++;
		garbage=pFlamMap->GetWeatherElev(StationNumber, count);
		hmorn=pFlamMap->GetWeatherTime1(StationNumber, count);
		Tmin=pFlamMap->GetWeatherTemp1(StationNumber, count);
		Hmax=pFlamMap->GetWeatherHumid1(StationNumber, count);
		if(garbage!=elref)
		{    tempf=(double) Tmin;
			humf=(double) Hmax;
			sitespec(elref, garbage, tempf, &temp, humf, &humid);	// adjusts to site specific temp and relhum at a given pixel 
			Tmin=(int) temp;
			Hmax=(int) humid;
		}
	}
	else
	{	if(hour<hmorn)
		{    count--;
			if(count<0)
				count=0;
			garbage=pFlamMap->GetWeatherElev(StationNumber, count);
			ppt=pFlamMap->GetWeatherRain(StationNumber, count);
			haft=pFlamMap->GetWeatherTime2(StationNumber, count);
			Tmax=pFlamMap->GetWeatherTemp2(StationNumber, count);
			Hmin=pFlamMap->GetWeatherHumid2(StationNumber, count);
			if(garbage!=elref)
			{    tempf=Tmax;
				humf=Hmin;
				sitespec(elref, garbage, tempf, &temp, humf, &humid);	// adjusts to site specific temp and relhum at a given pixel 
				Tmax=(int) temp;
				Hmin=(int) humid;
			}
		}
	}
	hx=hmorn/100;
	hx*=100;
	if(hx<hmorn)
		h1=(double) hx+(10.0*(hmorn-hx)/6.0);
	else
		h1=(double) hmorn;
	hx=haft/100;
	hx*=100;
	if(hx<haft)
		h2=(double) hx+(10.0*(haft-hx)/6.0);
	else
		h2=(double) haft;
	dtmxmn=(2400-h2)+h1;		// this section interpolates temperature 
	if(hour>=h1 && hour<=h2)		// and humidity from high and low obs 
	{    if(hour==h2)
			garbage=elref;
		dtprime=double (hour)-h1;	// and their time of observation 
		dtmxmn=2400-dtmxmn;
		sign=1.0;
	}
	else
	{	if(hour>h2)
			dtprime=double (hour)-h2;   // time since the maximum
		else
			dtprime=(2400-h2)+hour;    // time since the maximum
		sign=-1.0;
	}
	double Thalf=(double(Tmax)- double(Tmin))/2.0*sign;
	double Hhalf=(double(Hmin)-double(Hmax))/2.0*sign;

	*tempref=double (Tmax+Tmin)/2.0+(Thalf*sin(PI*(dtprime/dtmxmn-.5)));
	*humref=double (Hmax+Hmin)/2.0+(Hhalf*sin(PI*(dtprime/dtmxmn-.5)));
	*elevref=elref;
	*tempmx=(double)Tmax;
	*tempmn=(double)Tmin;
	*humidmx=(double)Hmax;
	*humidmn=(double)Hmin;
	*rain=((double) ppt)/100.0;
}




int FireEnvironment::chrono(double SIMTIME, int *hour)
// Determines actual date and time from SIMTIME variable
{
	int date, month, day, min;
	double mins, days;  //hours,

	hours=SIMTIME/60.0;
	*hour = (int) hours;                    // truncates time to nearest hour
	min=(int) ((hours-*hour)*60);
	mins=(hours-*hour)*60.0;
	hours=((double) *hour)*100.0 + (double) pFlamMap->GetStartHour();		     // elapsed time in hours
	days=hours/2400.0;
	day=(int) days;
	//if(RelCondit && UseConditioningPeriod(GETVAL))
    //{    
	//	date = day+GetConditDay();
    // 	month = GetJulianDays(GetConditMonth());
    //}
    //else
    //{	
	//	date=day+GetStartDay();
	//    month=GetJulianDays(GetStartMonth());
   // }
	date=day+pFlamMap->GetStartDay();
     month=pFlamMap->GetJulianDays(pFlamMap->GetStartMonth());
	date=date+month;
	if(date > 365)
		date -= 365;
	days=day;
	*hour=int (hours-days*2400)+min;         // integer hour
	hours=(hours-days*2400.0)+mins;          // double precision hours

	return date;
}




void FireEnvironment::sfmc(double rain, double humid, double temp)
//CALCULATES STANDARD DAILY FINE FUEL MOISTURE CONTENT 
{
	double fr, F, m, mr, Ed, Ew, X, f, delta, fo=thmc;

	fo=101-fo;

	if(rain>1.5)
		rain=1.5;
	if(rain>.02)
	{	if(rain<.055)
			F=-56-55.6*log(rain+.04);
		else
		{	if(rain<.225)
				F=-1-18.2*log(rain-.04);
			else
			{	if(rain>.225)
					F=14-8.25*log(rain-.075);
			}
		}

		fr=(F*fo/100)+1-8.73*exp(-.1117*fo);
		if(fr<0)
			fr=0;
	}
	else
		fr=fo;

	mr=101.0-fr;
	Ed=(.942*pow(fabs(humid),0.679))+11*exp(humid/10.0-10);
	Ew=(.597*pow(fabs(humid),0.768))+14*exp(humid/8.0-12.5);
	if(mr<Ed)
		m=Ew+(mr-Ew)/1.9953;
	else
	{	if(mr>Ed)
		{	X=.424*(1-pow((humid/100.0),1.7)+.088*sqrt(fabs(twindspd))*(1.0-pow(humid/100.0,8)));
			m=Ed+(mr-Ed)/(pow(10.0,X));
		}
		else
			m=mr;
	}

	if(fo<99)
	{	delta=(temp-70.0)*(.63-.0065*fr);
		if(delta<-16)
			delta=-16;
	}
	else
		delta=0;
	f=(101-m+delta);
	if(f>99)
		f=99;
	if(f<0)
		f=0;
	thmc=101-f;
}



void FireEnvironment::hffmc(double humid, double temp)
// CALCULATES HOURLY FINE FUEL MOISTURE INDEX FROM CANADIAN FBPS, ROTHERMEL ET AL. 1986 
{
	double Ed, Ew, m, mo, ka, kd, kb, kw, Tc=(5.0/9.0)*(temp-32.0);		// Tc=temperature deg C
	double twspd=twindspd*1.6096;		// convert mph to kph 

	mo=m=thmc;
	Ed=.942*pow(fabs(humid),0.679)+11*exp((humid-100.0)/10.0)+.18*(21.1-Tc)*(1.0-exp(-0.15*humid));
	if(mo>Ed)
	{	ka=.424*(1-pow(fabs(humid/100),0.17))+.0694*sqrt(fabs(twspd))*(1-pow((humid/100),8));
		kd=.0579*ka*exp(.0365*Tc);
		m=Ed+(mo-Ed)*exp(-2.303*kd);
	}
	else
	{	if(mo<Ed)
		{	Ew=.618*pow(fabs(humid),0.753)+10*exp((humid-100.0)/10.0)+.18*(21.1-Tc)*(1-exp(-0.15*humid));
			if(mo<Ew)
			{    kb=.424*(pow(1.0-((100.0-humid)/100.0),1.7))+.0694*sqrt(fabs(twspd))*(1-pow(((100.0-humid)/100.0),8));
				kw=.0579*kb*exp(.0365*temp);
				m=Ew-(Ew-mo)*exp(-2.303*kw);
			}
		}
		else
			m=mo;
	}
	thmc=m;
}




double FireEnvironment::emc(double humid, double temp)
// FINDS EQUILIBRIUM MOISUTER CONTENT OF TEN-HOUR FUELS 
{
	double xemc;

	if(humid<11.0)
		xemc=.03299+.281073*humid-.000578*humid*temp;
	else
	{	if(humid<51)
			xemc=2.22749+.160107*humid-.01478*temp;
		else
			xemc=21.06060+.005565*pow2(humid)-.00035*humid*temp-.483199*humid;
	}

	return xemc;
}




void FireEnvironment::sitespec(int elevref, int elevx, double tempref, double *temp, double humref, double *humid)
//FROM ROTHERMEL, WILSON, MORRIS, & SACKETT  1986 
{
	double dewptref, dewpt;
	double er=elevref;
	double ex=elevx;

	dewptref=-398-7469/(log(humref/100)-7469/(tempref+398));

	*temp=tempref+(er-ex)/1000.0*5.5;       // Stephenson 1988 found summer adiabat at 3.07 F/1000ft
	dewpt=dewptref+(er-ex)/1000.0*1.1; 	// new humidity, new dewpt, and new humidity
	*humid=7469*(1.0/(*temp+398.0)-1.0/(dewpt+398.0));
	*humid=exp(*humid)*100.0;			// convert from ln units
	if(*humid>99.0)
		*humid=99.0;
}




void FireEnvironment::windadj(int date, int hour, int *cloud)
// FINDS MOST CURRENT WINDSPD AND CALCULATES MIDFLAME WINDSPDS FROM 20-FT WS (MPH) 
{
	int count=-1, month, xmonth, xday, day, hhour=0, xdate;
	double MinTimeStep;

	do
	{    count++;
		month=pFlamMap->GetWindMonth(StationNumber, count);
		day=pFlamMap->GetWindDay(StationNumber, count);
          if(month<13)
          	xmonth=pFlamMap->GetJulianDays(month);
          else
          {	day=0;
               xmonth=date;
          }
		xdate=xmonth+day;
	}	while(xdate!=date);
	if(month!=13)						// if hit end of windspeed data
	{	xday=day;
		hhour=pFlamMap->GetWindHour(StationNumber, count);
		while(hour>=hhour)
		{    count++;
			day=pFlamMap->GetWindDay(StationNumber, count);
			xmonth=pFlamMap->GetWindMonth(StationNumber, count);
			hhour=pFlamMap->GetWindHour(StationNumber, count);
			if(day>xday || xmonth>month)
				break;
		}
	}
	if(hours<=hhour)
	{	MinTimeStep=(double) hhour-hours;
		EventMinimumTimeStep(MinTimeStep);         // load into global variable
	}
	count--;
	twindspd=pFlamMap->GetWindSpeed(StationNumber, count);
	wwinddir=pFlamMap->GetWindDir(StationNumber, count);
	*cloud=pFlamMap->GetWindCloud(StationNumber, count);
	//twindspd=(double) wss;
	windreduct();			// GET MIDFLAME WINDSPEED
}



void FireEnvironment::windreduct(void)
// FUNCTION TO REDUCE WINDSPEED (MI/HR) TO MIDFLAME OR VEGETATION HEIGHT 
{
	double ffactor, htfuel, htflame, m1, m2;
	double canopyht=ld.height*3.2808;//GetDefaultCrownHeight()*3.28;     // convert to feet

	if(ld.cover==0 || canopyht==0) // cover==99
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
		htflame=2*htfuel;		// from Baughman and Albini 6th conf. FFM 1980
		m1=(1+.36*(htfuel/htflame))/(log((20+.36*htfuel)/(0.13*htfuel)));
		m2=log(((htflame/htfuel+.36)/.13))-1;
		mwindspd=m1*m2*twindspd;
	}
	else
	{    ffactor=((double) ld.cover/100.0)*0.33333; // volume ratio of cone to cylinder
          ffactor*=PI/4.0;                        // area ratio of circle to square

		m1=.555/(sqrt(canopyht*ffactor)*log((20+.36*canopyht)/(.13*canopyht)));


//-------------------------------------------------------------------------------
//	Topographic Sheltering, Test only, Not Implemented
//-------------------------------------------------------------------------------
          //double wdir=((double) wwinddir)/180.0*PI;
          //double DiffAngle=aspectf-wdir;
          //if(DiffAngle>PI)
          	//DiffAngle=2.0*PI-aspectf+wdir;
          //m2=sqrt(pow2(1.25*cos(DiffAngle)+.75)+pow2(.75*sin(DiffAngle)));

		//mwindspd=m1*m2*twindspd;
		
		mwindspd=m1*twindspd;
	}
}



void FireEnvironment::adjust(double *humid, double *temp, double I)
// ADJUSTS HUMIDTY AND TEMPERATURE FOR FUEL LEVEL
{
	double tempdiff;

	tempdiff=I/(0.015*mwindspd+.026);
	*humid=*humid*exp(-.033*tempdiff);
	*temp=*temp+tempdiff;
}



double FireEnvironment::irradiance(int cloud, int date, int hour, double *shade)
// calculates irradiance from ROTHERMEL ET AL. 1986 
{
	double sangle, I, Im, M, atm, row, delta, ztan, zcos, zeta, N1;
	double tc, Ab, Ah, As, A, X, Z, flatitude, fslope, fhour;
	double tt, tn, Ia, Id;
//	int month;
	double D, xcover, FTol;    // D is assumed crown diameter in feet
	double L=fabs(ld.height-ld.base)*3.2808;//fabs(GetDefaultCrownHeight()-GetDefaultCrownBase())*3.28;      // feet
     double CrownHeight=ld.height*3.2808;

	xcover=(double) ld.cover/100.0;

	if(L<0.0)
		L=1.0;
     D=L/2.5;             // assume L/w ratio of crowns to be 2.5
     if(D>40.0)
          D=40.0;

     if(L==0.0 || D==0.0)
          xcover=0.0;

	int Tolerance=pFlamMap->GetTolerance();

	switch(Tolerance)
	{	case 1: FTol=.13; break;
		case 2: FTol=.16; break;
		case 3: FTol=.20; break;
	}
//	switch(cover)
//	{	case 1: xcover=.05; break;
//		case 2: xcover=.30; break;
//		case 3: xcover=.75; break;
//		case 4: xcover=.95; break;
//		case 99: xcover=0.0; break;
//	}
	flatitude=double (pFlamMap->GetLatitude())/180.0*PI;
	fslope=double (ld.slope)/180.0 * PI;

	delta=23.45*sin((.9863*(284+date))/180*PI);
	delta=delta/180*PI;
	 //	r2=.999847+.001406*delta;
	fhour=(PI/12)*(hour/100-6);  // hour angle from 6 AM
	sangle=sin(fhour)*cos(delta)*cos(flatitude)+sin(delta)*sin(flatitude);  // sine of solar angle 
	if(sangle>0.0)
	{	A=asin(sangle);
     	ztan=(sin(fhour)*cos(delta)*sin(flatitude)-sin(delta)*cos(flatitude))/(cos(fhour)*cos(delta));
          zcos=cos(fhour)*(cos(delta)/cos(A));
          Z=atan(ztan);
          if(zcos>=0.0)
      		Z+=PI/2.0;
          else
      		Z+=3.0*PI/2.0;
          if(Z>2.0*PI)
               Z-=2.0*PI;

		if(ld.slope>0)
		{    zeta=atan(tan(fslope)*cos(Z-ld.aspectf))*-1.0;
               sangle=sin(A-zeta)*(cos(fslope)/cos(zeta));
		}
		if(sangle>0.0)
		{    atm=1000.0*exp(-.0000448*ld.elev);		// atmospheric pressure at elev 
			M=atm/1000.0*(1.0/sangle);
			//if(elev<5000)		// determines atmospheric transparency 
			//	row=.6;        // assumed dense haze below 5000 ft, fairly typical of southern Sierra Nevada in summer 
			//else
			//{	if(elev<7000)
			//		row=.7;    // .7 is for moderate haze 
			//	else
					row=.8;   // very clear atmosphere 
			//}
			if(M<200.0)					// don't underflow Im
				Im=1.98*pow(row,M);			// cal/cm2/min 
			else
				Im=0;
			tc=((double) (100.0-cloud)/100.0);
               if(xcover>0)
               {	if(tan(A)>=(2.0*L/D))
     				Ah=PI*pow2(D)/4.0;
	     		else
		     	{    double G=acos(tan(A)/(2.0*L/D));
			     	Ah=(PI-G)*pow2(D)/4.0+D*L*(1/tan(A))*sin(G);
     			}
	     		X=PI*pow2(D)*L/(12.0*Ah*sin(A));
		     	Ab=0.0093*pow2(CrownHeight)*(1.0/tan(A));
			     Ab=Ab+Ah*(1-exp(-FTol*X));              // optical attenuation of light
     			if(ld.slope>0.0)						// through tree crown
	     		{	if((A+zeta)>.01)
		     			As=Ab*(cos(zeta)*sin(A))/sin(A+zeta);
			     	else
				     	As=0.0;
     			}
	     		else
		     		As=Ab;
     			if(As>0.0)
	     		{    N1=(As/(PI*pow2(D)/4.0))*xcover;   //  shade projection X cover divided by horiz proj
     		     	tt=exp(-N1);
     			}
	     		else
		     		tt=0.0;
               }
               else
                    tt=1.0;
			*shade=(1.0-tt)*100;
			if(*shade<0.0)
				*shade=100;
			tn=tc*tt;
			Ia=Im*tn;
			Id=Ia*sangle;
			if(Id<0.001)
				I=0.0;
			else
				I=(double) Id;
		}
		else
		{    I=0.0;
			*shade=100;
		}
	}
	else
	{	I=0.0;
		*shade=100;
	}

	return I;
}

*/