//******************************************************************************
//	FSXWMECH.CPP	Mechanix model functions for FARSITE
//                  Grows the Fire, Acceleration, Fire Spread Rate, Elliptical
//
//				Copyright 1994, 1995, 1996
//				Mark A. Finney, Systems for Environemntal Management
//******************************************************************************
#include "stdafx.h"

#include "flm4.hpp"
#include "flammap3.h"
//#include <excpt.h>
//#include <iostream.h>

//extern Acceleration AccelConst;			 // acceleration constants
//extern const double PI;
static const double PI = acos(-1.0);

//------------------------------------------------------------------------------
//
//	Acceleration Constant Functions
//
//------------------------------------------------------------------------------


Acceleration::Acceleration()
{
     int i;

	for(i=0; i<261; i++)
	{	ac[i].Point=.115;
		ac[i].Line=.3;
		ac[i].Limit=20;
	}
}


//------------------------------------------------------------------------------
//
//	Mechanix and MechCalls Functions
//
//------------------------------------------------------------------------------


MechCalls::MechCalls(FlamMap *_pFlamMap) : Mechanix(_pFlamMap)
{
	FirePerimeterDist = 0.0;
	A = 0;
}

MechCalls::~MechCalls()
{

}

void MechCalls::LoadGlobalFEData(FELocalSite *fe)
{// load fire envt data back up to mechanix class functions


	fe->GetEnvironmentData(&gmw);

	m_ones=pFlamMap->MassageMoisture(gmw.ones);
	m_tens=pFlamMap->MassageMoisture(gmw.tens);
	m_hundreds=pFlamMap->MassageMoisture(gmw.hundreds);
	m_livew=pFlamMap->MassageMoisture(gmw.livew);
	m_liveh=pFlamMap->MassageMoisture(gmw.liveh);
	m_windspd=gmw.windspd;
	m_twindspd=gmw.tws;
	m_winddir=gmw.winddir;
     ld=fe->ld;

     if(m_winddir==-1.0)
          m_winddir=ld.aspectf;
     else if(m_winddir<-1.0)
          m_winddir=(PI-ld.aspectf);

     /*
	m_ones=gmw.ones=0.05;
	m_tens=gmw.tens=0.10;
	m_hundreds=gmw.hundreds=0.15;
	m_livew=gmw.livew=0.95;
	m_liveh=gmw.liveh=0.95;
	m_windspd=gmw.windspd=1.0;
	m_twindspd=gmw.tws=6;
	m_winddir=gmw.winddir=PI/2.0;
     */
}



void MechCalls::LoadLocalFEData(FELocalSite *fe)
{
	fe->GetEnvironmentData(&lmw);

	m_ones=pFlamMap->MassageMoisture(lmw.ones);
	m_tens=pFlamMap->MassageMoisture(lmw.tens);
	m_hundreds=pFlamMap->MassageMoisture(lmw.hundreds);
	m_livew=pFlamMap->MassageMoisture(lmw.livew);
	m_liveh=pFlamMap->MassageMoisture(lmw.liveh);
	m_windspd=lmw.windspd;
	m_twindspd=lmw.tws;
	m_winddir=lmw.winddir;
     ld=fe->ld;

     if(m_winddir==-1.0)
          m_winddir=ld.aspectf;
     else if(m_winddir<-1.0)
          m_winddir=(PI-ld.aspectf);
}





void MechCalls::GetPoints(int row, int col)
{// need to reconstruct the points and orientations of each slope facet
	//xdiff=GetCellResolutionX();

     xpt=col*pFlamMap->GetCellResolutionX()+pFlamMap->GetLoEast()+pFlamMap->GetCellResolutionX()/2.0;
     ypt=pFlamMap->GetHiNorth()-row*pFlamMap->GetCellResolutionY()-pFlamMap->GetCellResolutionY()/2.0;
}

void MechCalls::CalculateFireOrientation(double DegreesFromMaxSpread)
{
	//double tangent;
     double tangentf;


     //tangent=aspect-DegreesFromUphill-90.0;
     //tangentf=aspectf-DegreesFromUphill/180.0*dPI-dPI/2.0;

     // must calculate vecdir first!! by calling vecsurf or veccrown
     //tangent=vecdir-DegreesFromMaxSpread-90.0;
     if(pFlamMap->GetOutputDirection()==RELATIVEDIR)
     	tangentf=vecdir-DegreesFromMaxSpread/180.0*PI;//-PI/2.0;
     else
     	tangentf=DegreesFromMaxSpread/180.0*PI-PI;///2.0;

     xdiff=cos(tangentf)*-30.0;
     ydiff=sin(tangentf)*30.0;

     xptl=xpt+xdiff/2.0;
     xptn=xpt-xdiff/2.0;
     yptl=ypt+ydiff/2.0;
     yptn=ypt-ydiff/2.0;
}


bool MechCalls::NormalizeDirectionWithLocalSlope()
{
     double Theta;
     double Angle;
     double hx, hy, ds;
     double XSign=1.0, YSign=1.0;
     double Mx=-1.0, My=-1.0;
     double slopef=((double) ld.slope/180.0)*PI;
//     double PI=3.14159265358979324;
     double xdiffln, ydiffln, dist;

     if(ld.slope==0)
          return false;

     xdiffln=xptl-xptn;
     ydiffln=yptl-yptn;
     dist=sqrt(pow2(xdiffln)+pow2(ydiffln));

     if(dist==0.0)
          return false;

     // calculate angles on horizontal coordinate system
     Angle=atan2(xdiffln, ydiffln);
     if(Angle<0.0)
          Angle+=2.0*PI;
     Theta=ld.aspectf-Angle;

     // calculate angles on local surface coordinate system
     Theta=-atan2(cos(Theta)/cos(slopef), sin(Theta))+PI/2.0;
     //Angle=ld.aspectf-Theta;
     //if(Angle<0.0)
     //     Angle+=2.0*PI;

     ds=dist*cos(Theta)*(1.0-cos(slopef));

     // if(quadrants 1 and 3
     if((ld.aspectf<=PI/2.0) || (ld.aspectf>PI && ld.aspectf<=3.0*PI/2.0))
     {    if(cos(Angle)!=0.0 || Angle==PI)
          {    if(tan(Angle)<0.0)
               {    if(tan(Angle)<tan(ld.aspectf-PI/2.0))
                         My=1.0;
                    else
                         Mx=1.0;
               }
          }
     }
     else // if quadrants 2 and 3
     {    if(cos(Angle)!=0.0)
          {    if(tan(Angle)>0.0 && Angle!=PI)
               {    if(tan(Angle)>tan(ld.aspectf-PI/2.0))
                         My=1.0;
                    else
                         Mx=1.0;
               }
          }
     }

     // transform x and y components to local surface coordinates
     hx=fabs(xdiffln)-Mx*fabs(ds*sin(ld.aspectf));
     hy=fabs(ydiffln)-My*fabs(ds*cos(ld.aspectf));

     //determine signs of horiz x and y components
     if(xdiff<0.0)
          XSign=-1.0;
     if(ydiff<0.0)
          YSign=-1.0;

     // substitute surface for horizontal components
     xdiff=hx*XSign;
     ydiff=hy*YSign;

     return true;
}


void MechCalls::GetAccelConst()
{// copies acceleration constants from global data structure
	//if(GetNumPoints(CurrentFire)*GetPerimRes()>AccelConst.ac[fuel-1].Limit)
	if(FirePerimeterDist>pFlamMap->AccelConst.ac[ld.fuel-1].Limit)
		A=pFlamMap->AccelConst.ac[ld.fuel-1].Line; 	    	// acceleration constant for lines
	else
		A=pFlamMap->AccelConst.ac[ld.fuel-1].Point;	    	// accleration constant for points
}


void MechCalls::AccelSurf1()
{// does first acceleration of fire, computes Ros at T+1 given time step

	if(RosT<0.0)    	// if negative ros (initial fraction of equilibrium ROS at input only)
		RosT*=(-vecros);
     if(xt==0.0 && yt==0.0)
		vecros=RosT;
	else
     	SpreadCorrect();	// vecros=equilibrium spread rate in vectored direction

	if(RosT<vecros && pFlamMap->AccelerationON()) 	// if RosT is < equilibrium, then accelerate
	{    cosslope=99.0;			// RosT1 is directional ROS with accelleration
		RosT1=accel(vecros, RosT, A, &avgros, &cosslope);
		double SpreadRatio=avgros/vecros;  // ratio of accel to equilibrium
		xt*=SpreadRatio;			     // in spread direction
		yt*=SpreadRatio;
	}					 // avgros is actual ROS averaged over time-step and spread distance
	else					 // else if new ROSEq is < RosT then reset new Ros to current equilibrium
		RosT1=avgros=vecros; // ROS after current time step is at steady state
}


void MechCalls::AccelSurf2()
{// does second acceleration of fire, computes Ros at T+1 given distance traveled
	if(timerem>0.0)                   // if time remaining, e.g. no limits to growth from distcheck (limgrow())
	{	if(RosT<vecros && pFlamMap->AccelerationON() && ExpansionRate>0.0)  // if not at steady state ROS
		{    timerem=SubTimeStep;  // reset timerem-- don't use limgrow() to decrement timestep, use accel()
			RosT1=accel(vecros, RosT, A, &avgros, &cosslope);
		}	     // computes foreward ROS RosT1 with accelleration within sub-timestep
		else
		{	RosT1=vecros;
			avgros=vecros;//ActualSurfaceSpread; //vecros;
		}
	}
	//else
    	//	avgros=vecros; //avgros=ActualSurfaceSpread; // necessary to calculate surface fli

	fli=3.4613*(384.0*(react/0.189275)*(avgros/.30480060960)/(60.0*savx));		// average fli in actual direction
	FliFinal=3.4613*(384.0*(react/0.189275)*(RosT1/.30480060960)/(60.0*savx));	// ending forward fli in timestep
	if(timerem<0.0)
	{	fli*=-1.0;
		FliFinal*=-1.0;
		timerem=0.0;
	}
}


void MechCalls::AccelCrown1()
{// does first acceleration of fire, computes Ros at T+1 given time step
	if(RosT<cros && pFlamMap->AccelerationON())			   	// if RosT is < equilibrium, then accelerate
	{    cosslope=99.0;           // RosT1 is forward ROS with accelleration
		RosT1=accel(cros, RosT, A, &avgros, &cosslope);
		double SpreadRatio=avgros/cros;  // ratio of accel to equilibrium
		xt*=SpreadRatio;			  // in spread direction
		yt*=SpreadRatio;

	}	// avgros is FORWARD ROS averaged over time-step and spread distance, timerem is unchanged here
	else	// else if new ROSEq is < RosT then reset new Ros to current equilibrium
		RosT1=avgros=cros;     // ROS after current time step is at steady state

}


void MechCalls::AccelCrown2()
{// does second acceleration of fire, computes Ros at T+1 given distance traveled
	if(timerem>0.0)                  	 // if time remaining, e.g. no limits to growth from distcheck (limgrow())
	{	if(RosT<cros && pFlamMap->AccelerationON() && ExpansionRate>0.0)				 // if not at steady state ROS
		{    timerem=SubTimeStep;      // reset timerem-- don't use limgrow() to decrement timestep, use accel()
			RosT1=accel(cros, RosT, A, &avgros, &cosslope);
		}	     				 // computes directional ROS RosT1 with accelleration within sub-timestep
		else
		{	RosT1=cros;
			avgros=cros;//ActualSurfaceSpread;
		}
	}
	//else                       		 // this is optional, use if avg ros is a desired output
	//	avgros=ActualSurfaceSpread;
}



void MechCalls::SlopeCorrect(int SurfaceSpread)
{// calls slope correction for spread rate
	double RosReduct=1.0;         // spread rate reduction only for surface fire

	if(SurfaceSpread)  			// if surface fire spread, subst. midx,y for x,ypts
	{	midx=xpt;				// not for crownfire spread
		midy=ypt;
		RosReduct=pFlamMap->GetRosRed(ld.fuel);
	}

	ActualSurfaceSpread=pow2(xt)+pow2(yt);
	if(ActualSurfaceSpread>0.0)// Expansion Rate
	{    ActualSurfaceSpread=sqrt(ActualSurfaceSpread);
     	if(ld.slope>0)
		{	scorrect(ld.slope, ld.aspectf);   				// CORRECTS ROS FOR SLOPE to get horizontal ROS
			HorizSpread=sqrt(pow2(xt)+pow2(yt));   	// gets Slope and Aspectf from FireEnvironment data members
			cosslope=HorizSpread/ActualSurfaceSpread;    // cosine of slope in spread direction
		}
		else
			cosslope=1.0;		// slope is 0 then cosine is 1.0
		xpt=midx-(timerem*xt*RosReduct*pFlamMap->MetricResolutionConvert()); // computes new horizontal xpt
		ypt=midy-(timerem*yt*RosReduct*pFlamMap->MetricResolutionConvert()); // and ypt coordinates
	}
}


void MechCalls::VecSurf()
{// calculates wind/slope vector for surface fire
	if(ld.slope>0)
	{    TransformWindDir(ld.slope, ld.aspectf);  // transform horiz to slope coords
          vecspeed=vectorspd(&ivecspeed, ld.aspectf, 0);
		vecdir=vectordir(ld.aspectf);      // spread vector direction
		if(vecspeed>phiew)
			vecspeed=phiew;            // vecspeed is still dimensionless
		vecros=(fros*(1.0+vecspeed));   // wind/slope vectored spread rate
	}
	else
	{	vecdir=m_winddir;
		ivecspeed=m_windspd;
		if(phiw>phiew)
			phiw=phiew;
		vecspeed=phiw;
		vecros=(fros*(1+phiw));	// winddriven spread rate in wind directn
	}
}



void MechCalls::VecCrown()
{// calculates wind/slope vector for crown fire
	if(ld.slope>0)
	{    // vectored wind speed with open wind*0.4 for surface fire
          vecspeed=vectorspd(&ivecspeed, ld.aspectf, 1);
		vecdir=vectordir(ld.aspectf);    // compute direction
		if(vecspeed>phiew)
			vecspeed=phiew;          // vecspeed is still dimensionless
		vecros=R10*(1.0+vecspeed);    // no ros reduction factor for crown spread
								// because this was an empirical relationship
								// from Rothermel 1991
	}
	else
	{    vecdir=m_winddir;
		ivecspeed=m_twindspd*0.5;
		if(phiw>phiew)
			phiw=phiew;
		vecspeed=phiw;
		vecros=R10*(1.0+phiw);	 	// eliminate ros reduction factor for crown fire spread
	}
	vecros*=3.34;				// R10 x 3.34 to get cros ... Rothermel 1991 p9
}


void MechCalls::SpreadCorrect()
{// calculates fire spread rate (Catchpole et al. 1982 && Richards 1995)
	//vecros=ExpansionRate=sqrt(pow2(xt)+pow2(yt));

     double Normal, cosn;

	if(xdiff==0.0 && ydiff==0.0)
	{	vecros=RosT; // should be 0.0 in this case
      	ExpansionRate=0.0;
     }
     else
     {    ExpansionRate=sqrt(pow2(xt)+pow2(yt));
          Normal=(atan2(ydiff, xdiff)+vecdir)-PI;
     	cosn=cos(Normal);
		vecros=back*cosn+sqrt(pow2(head)*pow2(cosn)+pow2(flank)*pow2(sin(Normal)));
          //vecros*=GetRosRed(ld.fuel); // only for producing rasters
     }
}


Mechanix::Mechanix(FlamMap *_pFlamMap)
{
	pFlamMap = _pFlamMap;
}


Mechanix::~Mechanix()
{

}


/*void Mechanix::distchek(int CurrentFire)
{// calculates distances around fires and resets distance checking
	double xpt, ypt, xptn, yptn, fli, flin;
     double Dist, ActivePerimDist=0, NewMinDist;
	int i, NumPtTotal=0, NumPtActive=0;

	SetDynamicDistRes(GetDistRes());  // set Dynamic min dist to default
     FirePerimeterDist=0.0;
	if((NumPtTotal=GetNumPoints(CurrentFire))>0)
	{	xpt=GetPerimeter2Value(0, XCOORD);
		ypt=GetPerimeter2Value(0, YCOORD);
		fli=GetPerimeter2Value(0, FLIVAL);
		for(i=1; i<=NumPtTotal; i++)
		{	if(i<NumPtTotal)
			{	xptn=GetPerimeter2Value(i, XCOORD);
				yptn=GetPerimeter2Value(i, YCOORD);
                    flin=GetPerimeter2Value(i, FLIVAL);
			}
			else
			{	xptn=GetPerimeter2Value(0, XCOORD);
				yptn=GetPerimeter2Value(0, YCOORD);
                    flin=GetPerimeter2Value(0, FLIVAL);
			}
			Dist=sqrt(pow2(xpt-xptn)+pow2(ypt-yptn));
               FirePerimeterDist+=Dist;
               if(fli>=0.0 || flin>=0.0)
               {    ActivePerimDist+=Dist;
                    ++NumPtActive;
               }
			xpt=xptn;
			ypt=yptn;
               fli=flin;
		}
          if(NumPtActive>1)
     		NewMinDist=(ActivePerimDist/(double) NumPtActive)/pFlamMap->MetricResolutionConvert();		// convert to meters
          else
               NewMinDist=-1.0;
		if(GetInout(CurrentFire)==2)
			NewMinDist/=1.4;			    // min dist less for inward fires
		if(NewMinDist<GetDistRes() && NewMinDist>0.0)
		{    if(NewMinDist<1.0)
                    NewMinDist=1.0;              // minimum distance resolution is 1 meter
          	SetDynamicDistRes(NewMinDist);    // set Dynamic min dist to new dist
          }
	}
	else
		NumPtTotal=-1;		// for debugging
}*/



void Mechanix::grow(double ivecdir)
{// grows fire perimeter points using Richards' equation
	double part1, part2, part3, part4, part5, part6;
	double f2, h2;

	if(xdiff==0 && ydiff==0)
	{	xt=0.0;
		yt=0.0;
	}
	else
	{	f2 = pow2(flank);
		h2 = pow2(head);
		part1 = f2*cos(ivecdir)*(xdiff*sin(ivecdir)+ydiff*cos(ivecdir));
		part2 = h2*sin(ivecdir)*(xdiff*cos(ivecdir)-ydiff*sin(ivecdir));
		part3 = h2*pow2((xdiff*cos(ivecdir)-ydiff*sin(ivecdir)));
		part4 = f2*pow2((xdiff*sin(ivecdir)+ydiff*cos(ivecdir)));
		part5 = f2*sin(ivecdir)*(xdiff*sin(ivecdir)+ydiff*cos(ivecdir));
		part6 = h2*cos(ivecdir)*(xdiff*cos(ivecdir)-ydiff*sin(ivecdir));
		xt = ((part1 - part2)/sqrt((part3 + part4)))+back*sin(ivecdir);
		yt = ((-part5 - part6)/sqrt((part3 + part4)))+back*cos(ivecdir);
	}
}



void Mechanix::limgrow()
{// limits distance growth to distance check and decrements remaining time step
	double fdist, dist, distx, disty, MINDIST, step;

	MINDIST=pFlamMap->GetDynamicDistRes()*pFlamMap->MetricResolutionConvert();
	step=timerem;
	distx=midx-xpt;
	disty=midy-ypt;
	dist=sqrt(pow2(distx)+pow2(disty));  	// computes horizontal distance traveled
	if(dist>MINDIST)
	{    fdist=MINDIST/dist;
		xpt=midx-distx*fdist;	 		// new xpt and new ypt computed
		ypt=midy-disty*fdist;     		// from mindist distance aint spread direction
		timerem=step-(MINDIST/(dist/step));// remainder of time step, distance traveled/ros ONLY IF CONSTANT ROS
		HorizSpread=pFlamMap->GetDynamicDistRes();	// horizontal spread now equals the distance check
	}
	else
		timerem=0.0;					// no time in timestep remaining
	if(xpt<=pFlamMap->GetLoEast())    				// keeps fire on visible landscape
	{	xpt=midx;                          // keep old point to prevent coincident points on edge of landscape
		timerem=-1.0;
	}
	else if(xpt>=pFlamMap->GetHiEast())
	{	xpt=midx;
		timerem=-1.0;
	}
	if(ypt<=pFlamMap->GetLoNorth())
	{	ypt=midy;
		timerem=-1.0;
	}
	else	if(ypt>=pFlamMap->GetHiNorth())
	{	ypt=midy;
		timerem=-1.0;
	}
}


double Mechanix::accel(double RosE, double RosT, double A, double *avgros, double *cosslope)
{// ROS acceleration from point source,VanWagner's (McAlpine and Wakimoto 1991)
	double RosT1, Tt, Dt, Dt1;

     Tt=0.0;
     Dt=0.0;
	if(RosT!=0.0)
	{	Tt=log(1.0-RosT/RosE)/(-A);   	 // time to reach current RosT given the equilibrium rate RosE
		Dt=RosE*(Tt+exp(-A*Tt)/A-1.0/A);	 // distance traveled to achieve current ROS,
	}								 // Alexander et al. 1992 eq. 71
	if(*cosslope==99.0)                              // forward distance traveled to eventual forward ROS timestep
	{	Dt1=RosE*(timerem+Tt+exp(-A*(timerem+Tt))/A-1.0/A);
		*avgros=(Dt1-Dt)/(timerem);			    // average rate of spread over forward distance Dt1-Dt
		RosT1=RosE*(1.0-exp(-A*(timerem+Tt)));      // eventual forward ROS at end of timestep
		*cosslope=0.0;
	}
	else
	{    double t, f1, f2, B=RosE/A;
		double LimSpread=pFlamMap->GetDynamicDistRes()/(*cosslope);	// convert horizontal distance check to slope distance equivalent
		//double LimForwardSpread=(head*timerem+back*timerem)*
		//					LimSpread/(ActualSurfaceSpread*timerem);
									// LimForwardSpread is the max spread in forward direction proportional to
          if(vecros>0.0 && ExpansionRate>0.0)
          	LimSpread*=vecros/ExpansionRate;

		if(LimSpread<B) 		   		// the Limited spread in actual direction based on elliptical dimensions
			t=sqrt((2.0*LimSpread)/B); 	// if LimSpread is "small"
		else
			t=LimSpread/RosE;		    	// if LimSpread is "large"
		do	                              // travel time to end of distance check
		{	B=exp(-A*t);                  // iterate using Newton's Method
			f1=RosE*(t+B/A-1.0/A)-(LimSpread+Dt); // total forward distance traveled to reach RosE
			f2=RosE*(1.0-B);
			t-=f1/f2;
		} while(fabs(f1)>1e-4);   // tolerance for convergence of iteration
		*avgros=LimSpread/(t-Tt);	// average surface ROS over distance in subtimestep in spread direction (not forward)
		RosT1=RosE*(1.0-exp(-A*t));	// eventual directional ROS at end of subtimestep
		timerem=timerem-(t-Tt);		// decrement time remaining in timestep
		if(timerem<0.0)
			timerem=0.0;
		*cosslope=0.0;				// change cosslope to 0
	}

	return fabs(RosT1);
}


void Mechanix::scorrect(int slope, double aspectf)
{
     double Theta;
     double Angle;
     double hxt, hyt, ds;
     double XSign=1.0, YSign=1.0;
     double Mx=-1.0, My=-1.0;
     double slopef=((double) slope/180.0)*PI;
     //double PI=3.14159265358979324;

     // compute angles of spread direction in surface plane
     Angle=atan2(-xt, -yt);
     if(Angle<0.0)
          Angle+=2.0*PI;
	Theta=aspectf-Angle;
     if(fabs(Theta)<0.0000000001)     // precision problem
          Theta=0;

     // compute horizontal spread differential
     ds=ActualSurfaceSpread*cos(Theta)*(1.0-cos(slopef));

     // if quadrants 1 and 3
     if((aspectf<=PI/2.0) || (aspectf>PI && aspectf<=3.0*PI/2.0))
     {    if(cos(Angle)!=0.0 || Angle==PI)
          {    if(tan(Angle)<0)
               {    if(tan(Angle)<tan(aspectf-PI/2.0))
                         My=1.0;
                    else
                         Mx=1.0;
               }
          }
     }
     else  // if quadrants 2 and 4
     {    if(cos(Angle)!=0.0)
          {    if(tan(Angle)>0 && Angle!=PI)
               {    if(tan(Angle)>tan(aspectf-PI/2.0))
                         My=1.0;
                    else
                         Mx=1.0;
               }
          }
     }

     // transfrom spread components in surface coords to horizontal coords
     hxt=fabs(xt)+Mx*fabs(ds*sin(aspectf));
     hyt=fabs(yt)+My*fabs(ds*cos(aspectf));

     //determine signs of horiz x and y components
     if(xt<0.0)
          XSign=-1.0;
     if(yt<0.0)
          YSign=-1.0;

     xt=hxt*XSign;
     yt=hyt*YSign;
}

double Mechanix::spreadrate(int slope, double windspd, int fuel)
{// Rothermel spread equation based on BEHAVE source code,
 //  support for dynamic fuel models added 10/13/2004
	int i, j, ndead=0, nlive=0;
	double depth;
	double seff[4][2]={{0.01,0.01},{0.01,0.01},{0.01,0},{0.01,0.0}};	     //mineral content
	double wtfact, fined=0.0, finel=0.0, wmfd=0.0, fdmois=0.0, w=0.0, wo=0.0, beta;
	double rm, sigma=0.0, rhob, sum3=0.0, betaop, rat, aa, gammax=0.0, gamma=0, wind=0, rm2;
	double xir, rbqig=0, xi=0, c, e, slopex=0;
	double ewind=0, wlim, sum1=0, sum2=0;

	double mois[4][2]=		// fraction of oven-dry weight
	{	{m_ones, m_liveh},        // use Mechanix Class copies of FE data
		{m_tens, m_livew},
		{m_hundreds, 0.0},
		{m_ones, 0.0}
	};
	double gx[5]={0.0, 0.0, 0.0, 0.0, 0.0};
	double wn[4][2]={{0,0},{0,0},{0,0},{0,0}};
	double qig[4][2]={{0,0},{0,0},{0,0},{0,0}};
	double a[4][2]={{0,0},{0,0},{0,0},{0,0}};
	double f[4][2]={{0,0},{0,0},{0,0},{0,0}};
	double g[4][2]={{0,0},{0,0},{0,0},{0,0}};
	double ai[2]={0,0};
	double fi[2]={0,0};
	double hi[2]={0,0};
	double se[2]={0,0};
	double xmf[2]={0,0};
	double si[2]={0,0};
	double wni[2]={0,0};
	double etam[2]={0,0};
	double etas[2]={0,0};
	double rir[2]={0,0};

     NewFuel newfuel;

     memset(&newfuel, 0x0, sizeof(NewFuel));
     if(!pFlamMap->GetNewFuel(fuel, &newfuel)) // will get all models, 13, 40, and custom
     {	rateo=0.0;

          return rateo;
     }

     // count number of fuels
     if(newfuel.h1) ndead++;
	if(newfuel.h10) ndead++;
	if(newfuel.h100) ndead++;
	if(newfuel.lh) nlive++;
	if(newfuel.lw) nlive++;

     if(nlive==0)
     	newfuel.dynamic=0;

     if(nlive>0)
          nlive=2;                      // boost to max number
     if(ndead>0)
          ndead=4;

     double nclas[2]={ndead,nlive};  // # of dead & live fuel classes
    	double xmext[2]={newfuel.xmext, 0};

    	double load[4][2]=			     // tons per acre, later converted to lb/ft2
	{	{newfuel.h1, newfuel.lh},
		{newfuel.h10, newfuel.lw},
		{newfuel.h100, 0.0},
		{0.0, 0.0},
	};

     depth=newfuel.depth;

     //-------------------------------------------------------------------------
     // do the dynamic load transfer
     //-------------------------------------------------------------------------
     if(newfuel.dynamic)
     {    if(mois[0][1]<0.30) // if live herbaceous is less than 30.0
          {    load[3][0]=load[0][1];
               load[0][1]=0.0;
          }
          else if(mois[0][1]<1.20)
          {    load[3][0]=load[0][1]*(1.20-mois[0][1])/0.9;
               load[0][1]-=load[3][0];
          }
     }
     //-------------------------------------------------------------------------

	double sav[4][2]=			     // 1/ft
	{	{newfuel.sav1, newfuel.savlh},
		{109.0, newfuel.savlw},
		{30.0, 0.0},
		{newfuel.savlh, 0.0},
	};

	double heat[4][2]=
     {    {newfuel.heatd, newfuel.heatl},
          {newfuel.heatd, newfuel.heatl},
          {newfuel.heatd, 0.0},
          {newfuel.heatd, 0.0},
     };

	wind=windspd*88.0;                      // ft/minute
	slopex=tan((double) slope/180.0*PI);  	// convert from degrees to tan

	// fuel weighting factors
	for(i=0; i<2; i++)
	{	for(j=0; j<nclas[i]; j++)
		{    a[j][i]=load[j][i]*sav[j][i]/32.0;
			ai[i]=ai[i]+a[j][i];
			wo=wo+0.04591*load[j][i];
		}
		if(nclas[i]!=0)
		{	for(j=0; j<nclas[i]; j++)
			{    if(ai[i]>0.0)
                    	f[j][i]=a[j][i]/ai[i];
                    else
                         f[j][i]=0.0;
               }
               memset(gx, 0x0, 5*sizeof(double));
               for(j=0; j<nclas[i]; j++)
			{    if(sav[j][i]>=1200.0)
	              		gx[0]+=f[j][i];
				else if(sav[j][i]>=192.0)
               		gx[1]+=f[j][i];
				else if(sav[j][i]>=96.0)
               		gx[2]+=f[j][i];
				else if(sav[j][i]>=48.0)
               		gx[3]+=f[j][i];
				else if(sav[j][i]>=16.0)
               		gx[4]+=f[j][i];
			}
               for(j=0; j<nclas[i]; j++)
			{    if(sav[j][i]>=1200.0)
	              		g[j][i]=gx[0];
				else if(sav[j][i]>=192.0)
	              		g[j][i]=gx[1];
				else if(sav[j][i]>=96.0)
	              		g[j][i]=gx[2];
				else if(sav[j][i]>=48.0)
	              		g[j][i]=gx[3];
				else if(sav[j][i]>=16.0)
	              		g[j][i]=gx[4];
                    else
                    	g[j][i]=0.0;
			}

		}
	}
	fi[0]=ai[0]/(ai[0]+ai[1]);
	fi[1]=1.0-fi[0];

	/* no need for this, because extinction moistures are assigned */
	/* as on last page of Burgan and Rothermel 1984 */
	/*	rhob=(wo/depth);
		beta=rhob/32;
		xmext[0]=.12+4.*beta;
	*/

	//moisture of extinction
	if(nclas[1]!=0)
	{	for(j=0; j<nclas[0]; j++)
		{    wtfact=load[j][0]*exp(-138.0/sav[j][0]);
			fined=fined+wtfact;
			wmfd=wmfd+wtfact*mois[j][0];
		}
		fdmois=wmfd/fined;
		for(j=0; j<nclas[1]; j++)
		{    if(sav[j][1]<1e-6)
              		continue;
			finel=finel+load[j][1]*exp(-500.0/sav[j][1]);
          }
		w=fined/finel;
		xmext[1]=2.9*w*(1.0-fdmois/xmext[0])-0.226;
		if(xmext[1]<xmext[0])
			xmext[1]=xmext[0];
	}

	// intermediate calculations, summing parameters by fuel component
	for(i=0; i<2; i++)
	{	if(nclas[i]!=0)
		{	for(j=0; j<nclas[i]; j++)
			{    if(sav[j][i]<1e-6)
               		continue;
               	wn[j][i]=0.04591*load[j][i]*(1.0-0.0555);
				qig[j][i]=250.0+1116.0*mois[j][i];
				hi[i]=hi[i]+f[j][i]*heat[j][i];
				se[i]=se[i]+f[j][i]*seff[j][i];
				xmf[i]=xmf[i]+f[j][i]*mois[j][i];
				si[i]=si[i]+f[j][i]*sav[j][i];
				sum1=sum1+0.04591*load[j][i];
				sum2=sum2+0.04591*load[j][i]/32.0;
				sum3=sum3+fi[i]*f[j][i]*qig[j][i]*exp(-138.0/sav[j][i]);
			}
			for(j=0; j<nclas[i]; j++)
				//wni[i]=wni[i]+f[j][i]*wn[j][i];
				wni[i]=wni[i]+g[j][i]*wn[j][i];  /* g[j][i] should be subst for f[j][i] in the wni[i] equation */
										   /* if the above g-factors are calculated */
			rm=xmf[i]/xmext[i];
			//etam[i]=1.0-2.59*rm+5.11*pow2(rm)-3.52*pow(rm,3.0);
			//optimization to get rid of pow() call
			rm2 = pow2(rm);
			etam[i]=1.0-2.59*rm+5.11*rm2-3.52 * rm2 *rm;
			if(xmf[i] >= xmext[i])
				etam[i]=0;
			etas[i]=0.174/(pow(se[i],0.19));
			if(etas[i]>1.0)
				etas[i]=1.0;
			sigma=sigma+fi[i]*si[i];
			rir[i]=wni[i]*hi[i]*etas[i]*etam[i];
		}
	}

	/* final calculations */
	rhob=sum1/depth;
	beta=sum2/depth;
	betaop=3.348/pow(sigma,0.8189);
	rat=beta/betaop;
	aa=133.0/pow(sigma,0.7913);
	//gammax=pow(sigma,1.5)/(495.0+0.0594*pow(sigma,1.5));
     gammax=(sigma*sqrt(sigma))/(495.0+0.0594*sigma*sqrt(sigma));
	gamma=gammax*pow(rat,aa)*exp(aa*(1.0-rat));
	xir=gamma*(rir[0]+rir[1]);
	rbqig=rhob*sum3;
	xi=exp((0.792+0.681*sqrt(sigma))*(beta+0.1))/(192.0+0.2595*sigma);
/*	flux=xi*xir;*/
	rateo=xir*xi/rbqig;   /* this is in English units */

	phis=5.275*pow(beta,-0.3)*pow2(slopex);
		c=7.47*exp(-0.133*pow(sigma,0.55));
		b=0.02526*pow(sigma,0.54);
		e=0.715*exp(-0.000359*sigma);
		part1=c*pow(rat,-e);
	phiw=pow(wind,b)*part1;

	wlim=0.9*xir;

	if(phis>0.0)
     {	if(phis>wlim)             // can't have inifinite windspeed
     		phis=wlim;
		slopespd=pow((phis/part1),1.0/b)/88.0; 	// converts phis to windspd in mph
     }
    	else
		slopespd=0.0;

/*   rate=(rateo*(1+phiw+phis));    */
/*   *byram=384*xir*rate/(60*sigma);*/
/*	flame=.45*pow(byram,.46);      */
/*	hpua=xir*384/sigma;            */

/*   maximum windspeed effect on ros*/
	phiew=phiw+phis;
	ewind=pow(((phiew*pow(rat,e))/c),1.0/b);

	if(ewind>wlim)
	{	ewind=wlim;
		phiew=c*pow(wlim,b)*pow(rat,-e);
/*		rate=rateo*(*phiew+1);	     */
/*		byram=384*xir*rate/(60*sigma);*/
/*		flame=.45*pow(byram,.46);     */
	}
	savx=sigma;					// SAVX, REACT IN MECHANIX.PUBLIC:
	react=xir*0.189275;  		    	// convert btu/f2/min to kW/m2
	rateo=rateo*0.30480060960;				// convert from f/min to m/min

     return rateo;
}


void Mechanix::GetEquationTerms(double *pphiw, double *pphis, double *bb, double *ppart1)
{
	*pphiw=phiw;
     *pphis=phis;
     *bb=b;
     *ppart1=part1;
}


/*
double Mechanix::spreadrate(int slope, double windspd, int fuel)
{// Rothermel spread equation based on BEHAVE source code
	int i, j, ndead=0, nlive=0;
	double depth=0, lone=0, lten=0.0, lhun=0, llivew=0, lliveh=0;
     double s1=0, slivew=1500, sliveh=1800, xm1=0;
	double hd=0, hl=0;		//heat content variables
	double heat[3][2]={{8000,8000},{8000,8000},{8000,0}};	// BTU/lb
	double seff[3][2]={{.01,.01},{.01,.01},{.01,0}};	     //mineral content
	double wtfact, fined=0.0, finel=0.0, wmfd=0.0, fdmois=0.0, w=0.0, wo=0.0, beta;
	double rm, sigma=0.0, rhob, sum3=0.0, betaop, rat, aa, gammax=0.0, gamma=0, wind=0;
	double xir, rbqig=0, xi=0, c, e, slopex=0;
	double ewind=0, wlim, sum1=0, sum2=0;
	//double g1, g2, g3, g4, g5, s, h1, h2, h3, h4, byram=0, hpua=0, flame=0, flux=0;
	double mois[3][2]=		// fraction of oven-dry weight
	{	{m_ones, m_liveh},        // use Mechanix Class copies of FE data
		{m_tens, m_livew},
		{m_hundreds, 0}
	};
	double wn[3][2]={{0,0},{0,0},{0,0}};
	double qig[3][2]={{0,0},{0,0},{0,0}};
	double a[3][2]={{0,0},{0,0},{0,0}};
	double f[3][2]={{0,0},{0,0},{0,0}};
	// double g[3][2]={0,0,0,0,0,0,};
	double ai[2]={0,0};
	double fi[2]={0,0};
	double hi[2]={0,0};
	double se[2]={0,0};
	double xmf[2]={0,0};
	double si[2]={0,0};
	double wni[2]={0,0};
	double etam[2]={0,0};
	double etas[2]={0,0};
	double rir[2]={0,0};
	double xmext[]={0,0};

if(fuel>13)
{	depth=GetFuelDepth(fuel-14);
	if(depth==0.0)
		fuel=0;
}

if(fuel>0) 			  /* rock, wet meadow, or water
{	if(fuel<14)
	{	switch(fuel)
		{	case 1: lone=0.74; lten=0.0; lhun=0.0; llivew=0.0;s1=3500;slivew=1500;depth=1.0;xmext[0]=.12;break;
			case 2: lone=2.0; lten=1.0; lhun=0.5; llivew=.5;s1=3000.0;slivew=1500.0;depth=1.0;xmext[0]=.15;break;
			case 3: lone=3.01; lten=0.0; lhun=0.0; llivew=0.0;s1=1500.0;slivew=1500.0;depth=2.5;xmext[0]=.25;break;
			case 4: lone=5.01; lten=4.01; lhun=2.0; llivew=5.01;s1=2000.0;slivew=1500.0;depth=6.0;xmext[0]=.20;break;
			case 5: lone=1.0; lten=0.5; lhun=0.0; llivew=2.0;s1=2000.0;slivew=1500.0;depth=2.0;xmext[0]=.20;break;
			case 6: lone=1.5; lten=2.5; lhun=2.0; llivew=0.0;s1=1750.0;slivew=1500.0;depth=2.5;xmext[0]=.25;break;
			case 7: lone=1.13; lten=1.87; lhun=1.5; llivew=0.37;slivew=1550.0;s1=1750.0;depth=2.5;xmext[0]=.40;break;
			case 8: lone=1.5; lten=1.0; lhun=2.5; llivew=0.0;s1=2000.0;slivew=1500.0;depth=0.2;xmext[0]=.30;break;
			case 9: lone=2.92; lten=0.41; lhun=0.15; llivew=0.0;s1=2500.0;slivew=1500.0;depth=0.2;xmext[0]=.25;break;
			case 10: lone=3.01; lten=2.0; lhun=5.01; llivew=2.0;s1=2000.0;slivew=1500.0;depth=1.0;xmext[0]=.25;break;
			case 11: lone=1.5; lten=4.51; lhun=5.51; llivew=0.0;s1=1500.0;slivew=1500.0;depth=1.0;xmext[0]=.15;break;
			case 12: lone=4.01; lten=14.03; lhun=16.53; llivew=0.0;s1=1500.0;slivew=1500.0;depth=2.3;xmext[0]=.20;break;
			case 13: lone=7.01; lten=23.04; lhun=28.05; llivew=0.0;s1=1500.0;slivew=1500.0;depth=3.0;xmext[0]=.25;break;
		}
	}
	else                         		// get loadings, surfs, and depth from array of custom models
	{    int Nfuel=fuel-14;
		GetFuel(Nfuel, &lone, &lten, &lhun, &lliveh, &llivew, &s1, &sliveh, &slivew, &hd, &hl, &depth, &xm1, 0);
		xmext[0]=xm1;
//		lone=FuelMod.TL1[Nfuel];    		// array address is -14 to start at 0
//		lten=FuelMod.TL10[Nfuel];
//		lhun=FuelMod.TL100[Nfuel];
//		llive=FuelMod.TLLive[Nfuel];
//		s1=FuelMod.S1[Nfuel];             // can input custom s10 && s100 also
//		slive=FuelMod.SLive[Nfuel];
//		depth=FuelMod.Depth[Nfuel];
//		xmext[0]=FuelMod.XMext[Nfuel];
 //		HeatDead=FuelMod.HD[Nfuel];       // can input h1, h10, && h100 also
//		HeatLive=FuelMod.HL[Nfuel];
//		for(i=0; i<3; i++) 			    // substitute custom heat conts
//			heat[i][0]=HeatDead;
//		heat[0][1]=HeatLive;
	}
	if(lone) ndead++;
	if(lten) ndead++;
	if(lhun) ndead++;
	if(lliveh) nlive++;
	if(llivew) nlive++;

     if(nlive>0)
          nlive=2;                 // boost to max number
     if(ndead>0)
          ndead=3;

    	double load[3][2]=			// tons per acre, later converted to lb/ft2
	{	{lone,lliveh},
		{lten,llivew},
		{lhun,0},
	};
	double sav[3][2]=			 // 1/ft
	{	{s1,sliveh},
		{109,slivew},
		{30,0},
	};
	double nclas[2]={ndead,nlive};  // # of dead & live fuel classes

	wind=windspd*88.0;                      // ft/minute
	slopex=tan((double) slope/180.0*PI);  	// convert from degrees to tan

	// fuel weighting factors
	for(i=0; i<2; i++)
	{	for(j=0; j<nclas[i]; j++)
		{    a[j][i]=load[j][i]*sav[j][i]/32.0;
			ai[i]=ai[i]+a[j][i];
			wo=wo+0.04591*load[j][i];
		}
		if(nclas[i]!=0)
		{	for(j=0;j<nclas[i];j++)
			{    if(ai[i]>0.0)
                    	f[j][i]=a[j][i]/ai[i];
                    else
                         f[j][i]=0.0;
               }
		}
	}
	fi[0]=ai[0]/(ai[0]+ai[1]);
	fi[1]=1.0-fi[0];

	// no need for this, because extinction moistures are assigned
	// as on last page of Burgan and Rothermel 1984
	//	rhob=(wo/depth);
	//	beta=rhob/32;
	//	xmext[0]=.12+4.*beta;


	//moisture of extinction
	if(nclas[1]!=0)
	{	for(j=0; j<nclas[0]; j++)
		{    wtfact=load[j][0]*exp(-138.0/sav[j][0]);
			fined=fined+wtfact;
			wmfd=wmfd+wtfact*mois[j][0];
		}
		fdmois=wmfd/fined;
		for(j=0; j<nclas[1]; j++)
			finel=finel+load[j][1]*exp(-500.0/sav[j][1]);
		w=fined/finel;
		xmext[1]=2.9*w*(1.0-fdmois/xmext[0])-0.226;
		if(xmext[1]<xmext[0])
			xmext[1]=xmext[0];
	}

	// intermediate calculations, summing parameters by fuel component
	for(i=0; i<2; i++)
	{	if(nclas[i]!=0)
		{	for(j=0; j<nclas[i]; j++)
			{    if(sav[j][i]<1e-6)
                         continue;
                    wn[j][i]=0.04591*load[j][i]*(1-0.0555);
				qig[j][i]=250.0+1116.0*mois[j][i];
				hi[i]=hi[i]+f[j][i]*heat[j][i];
				se[i]=se[i]+f[j][i]*seff[j][i];
				xmf[i]=xmf[i]+f[j][i]*mois[j][i];
				si[i]=si[i]+f[j][i]*sav[j][i];
				sum1=sum1+0.04591*load[j][i];
				sum2=sum2+0.04591*load[j][i]/32.0;
				sum3=sum3+fi[i]*f[j][i]*qig[j][i]*exp(-138.0/sav[j][i]);
			}
			for(j=0; j<nclas[i]; j++)
				wni[i]=wni[i]+f[j][i]*wn[j][i];  // g[j][i] should be subst for f[j][i] in the wni[i] equation
										   // if the above g-factors are calculated
			rm=xmf[i]/xmext[i];
			etam[i]=1.0-2.59*rm+5.11*pow2(rm)-(3.52*rm*rm*rm);//pow(rm,3.0);
			if(xmf[i] >= xmext[i])
				etam[i]=0;
			etas[i]=0.174/(pow(se[i],0.19));
			if(etas[i]>1.0)
				etas[i]=1.0;
			sigma=sigma+fi[i]*si[i];
			rir[i]=wni[i]*hi[i]*etas[i]*etam[i];
		}
	}

	// final calculations
	rhob=sum1/depth;
	beta=sum2/depth;
	betaop=3.348/pow(sigma,0.8189);
	rat=beta/betaop;
	aa=133.0/pow(sigma,0.7913);
	//gammax=pow(sigma,1.5)/(495.0+0.0594*pow(sigma,1.5));
     gammax=(sigma*sqrt(sigma))/(495.0+0.0594*sigma*sqrt(sigma));
	gamma=gammax*pow(rat,aa)*exp(aa*(1.0-rat));
	xir=gamma*(rir[0]+rir[1]);
	rbqig=rhob*sum3;
	xi=exp((0.792+0.681*sqrt(sigma))*(beta+0.1))/(192.0+0.2595*sigma);
//	flux=xi*xir;
	rateo=xir*xi/rbqig;   // this is in English units

	phis=5.275*pow(beta,-0.3)*pow2(slopex);
		c=7.47*exp(-0.133*pow(sigma,0.55));
		b=0.02526*pow(sigma,0.54);
		e=0.715*exp(-0.000359*sigma);
		part1=c*pow(rat,-e);
	phiw=pow(wind,b)*part1;

	wlim=0.9*xir;

	if(phis>0.0)
     {	if(phis>wlim)             // can't have inifinite windspeed
     		phis=wlim;
		slopespd=pow((phis/part1),1.0/b)/88.0; 	// converts phis to windspd in mph
     }
    	else
		slopespd=0.0;

//  rate=(rateo*(1+phiw+phis));
//  *byram=384*xir*rate/(60*sigma);
//	flame=.45*pow(byram,.46);
//	hpua=xir*384/sigma;

//  maximum windspeed effect on ros
	phiew=phiw+phis;
	ewind=pow(((phiew*pow(rat,e))/c),1.0/b);

	if(ewind>wlim)
	{	ewind=wlim;
		phiew=c*pow(wlim,b)*pow(rat,-e);
//		rate=rateo*(*phiew+1);
//		byram=384*xir*rate/(60*sigma);
//		flame=.45*pow(byram,.46);
	}
	savx=sigma;					// SAVX, REACT IN MECHANIX.PUBLIC:
	react=xir*0.189275;  		    	// convert btu/f2/min to kW/m2
	rateo=rateo*0.30480060960;				// convert from f/min to m/min
}
else
{	rateo=0.0;
	// react[count1]=0
}
return rateo;
}
*/

double Mechanix::headback()
{// calculates Alexanders head/backing ratio assuming fire origin at focus equation [16]
	double hb_ratio, part;

	part=sqrt(pow2(lb_ratio)-1);
	hb_ratio=(lb_ratio+part)/(lb_ratio-part);

	return hb_ratio;
}


void Mechanix::ellipse(double iros, double wspeed)
{// calculates spread rate parameters for ellipse
	 // ivecspeed=ivecspeed*1.6096;  /* convert to km/hr for ALEXANDER'S EQUATION*/
	 // then multiply ivecspeed for 20 ft ws by 1.15 to get 10-m open ws for Alexander's equations
	 // need ivecspeed boosted to effective 10-m windspeed
	 // this would require passing cover and fuel type to this function
	 // lb_ratio=.5+.5*exp(.05039*ivecspeed); // ALEXANDER'S EQUATION [36]
							 // ANDERSON 1983 FOR MIDFLAME WINDSPEED
	lb_ratio=.936*exp(.1147*wspeed)+.461*exp(-.0692*wspeed)-.397;

	if(lb_ratio>8.0)              // maximum eccentricity
		lb_ratio=8.0;
	hb_ratio=headback();		 //	subt 0.397 to make circle at 0 windspd (my own addition)
	head=iros;
	if(pFlamMap->ConstantBackingSpreadRate(GETVAL))
		back=rateo;			// backing ros should be ros with 0 slope & wind
	else
		back=head/hb_ratio;		// backspread from hb_ratio from Alexander 1985
	flank=((head+back)/lb_ratio)/2.0;
	head=(head+back)/2.0;
	back=head-back;			// modifies rates of spread for elliptical dimensions
}

void Mechanix::TransformWindDir(int slope, double aspect)
{// horizontal wind direction to local slope coordinates

     double Theta, slopef;
     double X, Y, Z, S;//, P;

     if(m_twindspd==0.0)
     {    LocalWindDir=m_winddir;

          return;
     }

     slopef=((double) slope/180.0)*PI;
     Theta=aspect-m_winddir;
	X=cos(Theta);
     Y=sin(Theta);
     Z=X*tan(slopef);
	S=sqrt(X*X+Z*Z);
     //P=sqrt(pow2(S)+pow2(Y));
	//*Ratio=P;

     if(X>0.0)
          S=-S;
     LocalWindDir=atan2(Y, S)+aspect-PI;
     if(LocalWindDir<0.0)
     	LocalWindDir+=2.0*PI;
}

/*
void Mechanix::TransformWindDir(int slope, double aspectf)
{// horizontal wind direction to local slope coordinates
     if(m_twindspd==0.0)
     {    LocalWindDir=m_winddir;

          return;
     }

     double Theta;//, PI=3.14159265358979324;
     double slopef=((double) slope/180.0)*PI;

     Theta=aspectf-m_winddir;

     // calculate angles on local surface coordinate system
     Theta=-atan2(cos(Theta)/cos(slopef), sin(Theta))+PI/2.0;
     LocalWindDir=aspectf-Theta;

     if(LocalWindDir<0.0)
          LocalWindDir+=2.0*PI;
}
*/

double Mechanix::CalcEffectiveWindSpeed()
{
     return pow((vecspeed/part1),1.0/b)/88.0;
}


double Mechanix::vectorspd(double *ivecspeed, double aspectf, int CrownWinds)
{// determines the direction and strength of windXslope vector
	double angle, vecspeed;

	//angle = m_winddir - ld.aspectf;  //-PI);  correct for slope and uphill spread
	angle=fabs(LocalWindDir-aspectf);
	if(angle!=PI)		    		// dimensionless coefs
	{	vecspeed = sqrt(pow2(phiw)+pow2(phis)+(2.0*phiw*phis*cos(angle)));
          if(CrownWinds==0)
     		*ivecspeed = sqrt(pow2(m_windspd)+pow2(slopespd)+(2.0*m_windspd*slopespd*cos(angle)));
          //   *ivecspeed=CalcEffectiveWindSpeed();
          else
          	*ivecspeed=sqrt(pow2(m_twindspd*0.5)+pow2(slopespd)+(2.0*m_twindspd*0.5*slopespd*cos(angle)));
	}				    		// actual windspeed vector, not dimensionless coefs
	else
	{	vecspeed = fabs(phis-phiw);
          if(CrownWinds==0)
     		*ivecspeed = fabs(slopespd-m_windspd);
          //   *ivecspeed=CalcEffectiveWindSpeed();
          else
     		*ivecspeed=fabs(slopespd-m_twindspd*0.5);
	}

	return vecspeed;
}



double Mechanix::vectordir(double aspectf)
{// calculates direction for resultant wind-slope vector
	double angle, angleabs, vangle, vecdir, aside, bside, cside;

	angle = LocalWindDir-aspectf;//m_winddir - ld.aspectf;
	angleabs=fabs(angle);

	if(phis>=phiw)
	{	aside=phiw;
		bside=phis;
		cside=vecspeed;
	}
	else
	{	aside=phis;
		bside=phiw;
		cside=vecspeed;
	}
	if(bside!=0.0 && cside!=0.0)
	{	vangle=(pow2(aside)-pow2(bside)-pow2(cside))/(-2.0*bside*cside);
		if(vangle>1.0)
			vangle=1.0;
		//else
		//{
          if(vangle<0.0)
			vangle=PI;
		else
		{	vangle=(acos(vangle));
			vangle=fabs(vangle);
		}
		//}
	}
	else
		vangle=0.0;
	if(angleabs<PI)
	{	if(angle>0.0)
		{	if(phiw>=phis)
				vecdir=LocalWindDir-vangle;//m_winddir-vangle;
			else
				vecdir=aspectf+vangle;
		}
		else
		{	if(phiw>=phis)
				vecdir=LocalWindDir+vangle;//m_winddir+vangle;
			else
				vecdir=aspectf-vangle;
		}
	}
	else
	{	if(angle>0.0)
		{	if(phiw>=phis)
				vecdir=LocalWindDir+vangle;//m_winddir+vangle;
			else
				vecdir=aspectf-vangle;
		}
		else
		{	if(phiw>=phis)
				vecdir=LocalWindDir-vangle;//m_winddir-vangle;
			else
				vecdir=aspectf+vangle;
		}
	}
	if(vecdir<0.0)
		vecdir=2.0*PI+vecdir;

	return vecdir;
}

