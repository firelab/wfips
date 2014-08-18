//------------------------------------------------------------------------------
//  Burnup, Albini and Reinhardt
//
//
//------------------------------------------------------------------------------
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>                      
#include <string.h>
#include <windows.h>
#include "burnupcw.h"
//#include "nonlin.h"

static const double ch2o=4186.0;
static const double tpdry=353.0;
static const double smallx=1.e-06;
static const double big= 1.e+06;
//#define min(a,b) ((a<b) ? a : b)
//#define max(a,b) ((a>b) ? a : b)

double BurnUp::pow2(double input)
{
	return input*input;
}

void BurnUp::ResetEmissionsData()
{
	long i;

     for(i=0; i<MAXNO; i++)
     {    Smoldering[i]=0.0;
          Flaming[i]=0.0;
     }
     Smoldering[MAXNO]=0.0;
}


BurnUp::BurnUp()
{
	long i;

     ntimes=0;
     number=0;
	fi	=0.0;
     ti	=0.0;
     u	=0.0;
     d	=0.0;
     tamb	=0.0;
     ak	=0.0;
     r0	=0.0;
     dr	=0.0;
     dt	=0.0;
     wdf	=0.0;
     dfm	=2.0;

     for(i=0; i<MAXNO; i++)
     {	wdry[i]		=0.0;
		ash[i]		=0.0;
          htval[i]		=0.0;
		fmois[i]		=0.0;
          dendry[i]		=0.0;
          sigma[i]		=0.0;
		cheat[i]		=0.0;
          condry[i]		=0.0;
          alfa[i]		=0.0;
		tpig[i]		=0.0;
          tchar[i]		=0.0;
		flit[i]		=0.0;
          fout[i]		=0.0;
          work[i]		=0.0;
		alone[i]		=0.0;
		area[i]		=0.0;
          fint[i]		=0.0;
          Smoldering[i]=0.0;
          Flaming[i]=0.0;
     }
     Smoldering[MAXNO]=0.0;
     //memset(Message, 0x0, sizeof(Message));
     ZeroMemory(Message, sizeof(Message));
     fistart=-1.0;
     NumAllocRegrData=0;
     x=y=w=0;	// arrays for regression
     ux=vx=0;
     sig=0;
     FintSwitch=15.0;
     bs=0;
}


BurnUp::~BurnUp()
{
	FreeBurnStruct();
     FreeRegressionData(NumAllocRegrData);
}


bool BurnUp::CheckData()
{
	long i;

	const double ash1=0.0001, ash2=0.1;
     const double htv1=1.0e07, htv2=3.0e7;
     const double fms1=0.01, fms2=3.0;
     const double den1=200.0, den2=1000.0;
     const double sig1=4.0, sig2=1.0e4;
     const double cht1=1000.0, cht2=4000.0;
     const double con1=0.025, con2=0.25;
     const double tig1=200.0, tig2=400.0;
     const double tch1=250.0, tch2=500.0;
     const double fir1=0.1, fir2=1.0e5;
     const double ti1=1.0, ti2=200.0;
     const double u1=0.0, u2=5.0;
     const double d1=0.1, d2=5.0;
     const double tam1=-40.0, tam2=40.0;
     const double wdf1=0.1, wdf2=30.0;
     const double dfm1=0.1, dfm2=1.972;

     for(i=0; i<number; i++)
     {	sprintf(Message, "%s %ld", "Line Number", i);
     	if(wdry[i]<=smallx || wdry[i]>=big)
      	{	strcat(Message, " dry loading out of range (kg/m2)");
           	break;
          }
          if(ash[i]<=ash1 || ash[i]>=ash2)
      	{	strcat(Message, " ash content out of range (fraction)");
           	break;
          }
		if(htval[i]<=htv1 || htval[i]>=htv2)
      	{	strcat(Message, " heat content out of range (J/kg)");
           	break;
          }
          if(fmois[i]<=fms1 || fmois[i]>=fms2)
          {	strcat(Message, " fuel moisture out of range (fraction)");
          	break;
          }
          if(dendry[i]<=den1 || dendry[i]>=den2)
          {	strcat(Message, " dry mass density out of range (kg/m3)");
          	break;
          }
          if(sigma[i]<=sig1 || sigma[i]>=sig2)
          {	strcat(Message, " SAV out of range (1/m)");
          	break;
          }
          if(cheat[i]<=cht1 || cheat[i]>=cht2)
          {	strcat(Message, " heat capacity out of range (J/kg/K");
          	break;
          }
          if(condry[i]<=con1 || condry[i]>=con2)
          {	strcat(Message, " thermal conductivity out of range (W/m/K)");
          	break;
          }
          if(tpig[i]<=tig1 || tpig[i]>=tig2)
          {	strcat(Message, " ignition temperature out of range (C)");
          	break;
          }
          if(tchar[i]<=tch1 || tchar[i]>=tch2)
          {	strcat(Message, " char end pyrolisis temperature out of range (C)");
          	break;
          }
     }
     if(i<number)
     	return false;
     //memset(Message, 0x0, sizeof(Message));
	ZeroMemory(Message, sizeof(Message));
     
	if(ti<ti1)
     {	double rat, tempf, tempt;

     	rat=fistart/ti;
		tempf=fir1;
          tempt=(fistart-fir1)/rat;
          ti+=tempt;
          fistart=tempf;
     }

     if(fistart<fir1 || fistart>fir2)
     	strcat(Message, " igniting fire intensity out of range (kW/m2)");
     else if(ti<ti1)// || ti>ti2 )
     	strcat(Message, " igniting surface fire res. time out of range (s)");
     else if(u<u1 || u>u2)
     	strcat(Message, " windspeed at top of fuelbed out of range (m/s)");
     else if(d<d1 || d>d2)
     	strcat(Message, " depth of fuel bed out of range (m)");
     else if(tamb-273<tam1 || tamb-273>tam2)
     	strcat(Message, " ambient temperature out of range (C)");
     //else if(wdf<wdf1 || wdf>wdf2)
     //	strcat(Message, " duff dry weight loading out of range (kg/m2)");
     else if(dfm<dfm1 || dfm>dfm2)
     	strcat(Message, " duff moisture out of range (fraction)");

     if(strlen(Message)>0)
     	return false;

	AllocBurnStruct();	// allocate for ntimes of output data

     return true;
}


bool BurnUp::GetDatFile(char *InFile, long Number)
{
     long i;
     double drywt, ashes, hots, fms, dryd, sigs, cpd, cond, tigi, tchi;

	FILE *infile;

    	number=Number;				// class copy of Number of fuel classes
     if(number>MAXNO)
     	return false;

	if((infile=fopen(InFile, "r"))==NULL)
     	return false;

	for(i=0; i<number; i++)
     {	fscanf(infile, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
     		&drywt, &ashes, &hots, &fms, &dryd, &sigs, &cpd, &cond, &tigi, &tchi);
     	wdry[i]=drywt;
          ash[i]=ashes;
          htval[i]=hots;
		fmois[i]=fms;
          dendry[i]=dryd;
          sigma[i]=sigs;
		cheat[i]=cpd;
          condry[i]=cond;
		tpig[i]=tigi+273.0;
          tchar[i]=tchi+273.0;
     }

     fscanf(infile, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %ld %lf %lf",
     	&fi, &ti, &u, &d, &tamb, &ak, &r0, &dr, &dt, &ntimes, &wdf, &dfm);
	tamb+=273;

     fclose(infile);

     if(!CheckData())
     	return false;

	return true;
}


bool BurnUp::SetFuelStruct(long NumParts, FuelStruct *fs)
{
	long i;

	number=NumParts;
     if(number>MAXNO)
    	{	sprintf(Message, "%s %ld", "Number of fuel partitions exceeds max:", MAXNO);
     	return false;
     }

	for(i=0; i<number; i++)
     {  	wdry[i]=fs[i].wdry;
          ash[i]=fs[i].ash;
          htval[i]=fs[i].htval*1000.0;
		fmois[i]=fs[i].fmois;
          dendry[i]=fs[i].dendry;
          sigma[i]=fs[i].sigma;
		cheat[i]=fs[i].cheat;
          condry[i]=fs[i].condry;
		tpig[i]=fs[i].tpig+273.0;
          tchar[i]=fs[i].tchar+273.0;
     }

     return true;
}


bool BurnUp::SetFuelDat(long NumParts, double *drywt, double *ashes, double *hots,
							double *fms, double *dryd, double *sigs,
                                   double *cpd, double *cond, double *tigi,
                                   double *tchi)
{
	long i;

	number=NumParts;
     if(number>MAXNO)
    	{	sprintf(Message, "%s %ld", "Number of fuel partitions exceeds max:", MAXNO);
     	return false;
     }

	for(i=0; i<number; i++)
     {  	wdry[i]=drywt[i];
          ash[i]=ashes[i];
          htval[i]=hots[i];
		fmois[i]=fms[i];
          dendry[i]=dryd[i];
          sigma[i]=sigs[i];
		cheat[i]=cpd[i];
          condry[i]=cond[i];
		tpig[i]=tigi[i]+273.0;
          tchar[i]=tchi[i]+273.0;
     }

     return true;
}


bool BurnUp::SetFuelInfo(long NumParts, double *datastruct)
{
	long i;

	number=NumParts;
     if(number>MAXNO)
    	{	sprintf(Message, "%s %ld", "Number of fuel partitions exceeds max:", MAXNO);
     	return false;
     }

	for(i=0; i<number; i++)
     {	sigma[i]=datastruct[i*5];
     	wdry[i]=datastruct[i*5+1];
          htval[i]=datastruct[i*5+2]*1000.0;
          dendry[i]=datastruct[i*5+3];
          fmois[i]=datastruct[i*5+4];
          ash[i]=	0.05;
          cheat[i]=	2750.0;
          condry[i]=0.133;
          tpig[i]=	327;// deg C+273;
          tchar[i]=	377;// deg C+273;
     }

     return true;
}


bool BurnUp::SetFireDat(long NumIter, double Fi, double Ti, double U, double D, double Tamb,
					double R0, double Dr, double Dt, double Wdf, double Dfm)
{
	ntimes=NumIter;

     if(ntimes<=0)
     {	sprintf(Message, "%s", "Number of Iterations too smallx (<=0");

		return false;
     }
    	fistart=Fi;
    	ti=Ti;
    	u=U;
    	d=D;
    	tamb=Tamb+273.0;
    	r0=R0;
    	dr=Dr;
    	dt=Dt;
    	wdf=Wdf;
    	dfm=Dfm;

	return true;
}


bool BurnUp::Burnup()
{
     char HistFile[]="burn_hist.txt";
     char SnapFile[]="burn_snap.txt";
	long nruns = 0;
	long now, nohist=0;
	double fimin = 0.1;
     double tis, dfi, tdf, fid;

     if(ntimes==0 || number==0)
		return false;

     ResetEmissionsData();
     fi=fistart;
     Arrays();
     now=1;
     tis=ti;
     DuffBurn(wdf, dfm, &dfi, &tdf);

     Start(tis, now, &nruns);
     if(tis<tdf)
     	fid=dfi;
     else
         	fid=0.0;
     if(!nohist)
     	Stash(HistFile, SnapFile, tis, now);
	fi=FireIntensity();

     if(fi>fimin)
     {  	do
          {  	Step(dt, tis, fid, &nruns);
			now++;
     		tis+=dt;
     		if(tis<tdf)
     			fid=dfi;
			else
     			fid=0.0;
          	fi=FireIntensity();
      		if(fi<=fimin)
				break;
	      	if(!nohist)
				Stash(HistFile, SnapFile, tis, now);
		} while(now<=ntimes);
	}
     Summary(HistFile);

     return true;
}


bool	BurnUp::StartLoop()
{
	fimin = 0.1;
	fi=fistart;
     if(ntimes==0 || number==0)
		return false;

     ResetEmissionsData();
     Arrays();
     now=1;
     tis=ti;
     DuffBurn(wdf, dfm, &dfi, &tdf);

     if(Start(tis, now, &nruns))
     {	if(tis<tdf)
     		fid=dfi;
	     else
     	    	fid=0.0;
	    	SetBurnStruct(0.0, now);
          fi=FireIntensity();
	    	SetBurnStruct(tis, now);
	     nruns=0;

	     if(fi<=fimin)
     		return false;
     }
     else
     	return false;

     return true;
}


bool	BurnUp::BurnLoop()
{
  	Step(dt, tis, fid, &nruns);
	now++;
	tis+=dt;
	if(tis<tdf)
		fid=dfi;
	else
     	fid=0.0;
     fi=FireIntensity();

     if(fi<=fimin)
     {	fi=fistart;
          while(tis<tdf && now<ntimes)
          {    fi=fid;
               SetBurnStruct(tis, now++);
               tis+=dt;
          };
     	return false;
     }
     SetBurnStruct(tis, now);
	if(now>ntimes)
     	return false;

     return true;
}



long BurnUp::loc(long k, long l)
{


	return k*(k+1.0)/2.0+l-1;

     //return k*((long) ((k+1)/2))+l-1;

     /*
     double HalfK;

	HalfK=(double) (k+1.0)/2.0;
     return (long) ((double) k*HalfK+l)-1;
     */
}

long BurnUp::Nint(double input)
{
	long Input;

     Input=(long) input;
     if(input-(double) Input>=0.5)
     	Input+=1;

     return Input;
}




//------------------------------------------------------------------------------
//
//	Tignit
//
//------------------------------------------------------------------------------

//      subroutine TIGNIT( tpam , tpdr , tpig , tpfi , cond ,
//     +                   chtd , fmof , dend , hbar , tmig )

//c   tpam = ambient temperature , K
//c   tpdr = fuel temperature at start of drying , K
//c   tpig = fuel surface temperature at ignition , K
//c   tpfi = fire environment temperature , K
//c   cond = fuel ovendry thermal conductivity , W / m K
//c   chtd = fuel ovendry specific heat capacity , J / kg K
//c   fmof = fuel moisture content , fraction dry weight
//c   dend = fuel ovendry density , kg / cu m
//c   hbar = effective film heat transfer coefficient [< HEATX] W / sq m K
//c   tmig = predicted time to piloted ignition , s


double BurnUp::ff(double x, double tpfi, double tpig)
{
      const double a03=-1.3371565;
      const double a13=0.4653628;
      const double a23=-0.1282064;
      double b03;

      b03=a03*(tpfi-tpig)/(tpfi-tamb);

 	 return b03+x*(a13+x*(a23+x));
}


double BurnUp::TIgnite(double tpdr, double tpig, double tpfi,
			double cond, double chtd, double fmof, double dend,
               double hbar)
{
      const double pinv=2.125534;
      const double hvap=2.177e+06;
      const double cpm=4186.0;
      const double conc=4.27e-04;
      double xlo, xhi, xav, fav, beta, conw, dtb, dti, ratio, rhoc, tmig;


// MOVED TO function FF(...)
//------------------------------------------------------------------------------
//c   approximate function of beta to be solved is ff( x ) where
//c   x  =  1 / ( 1 + p * beta )  { Hastings, Approximations for
//c   digital computers } and we employ      pinv  =  1 / p
//      ff( x ) = b03 + x * ( a13 + x * ( a23 + x ) )

//c   radiant heating equivalent form gives end condition fixes beta
//      b03 = a03 * ( tpfi - tpig ) / ( tpfi - tpam )
//------------------------------------------------------------------------------


//c   find x that solves ff( x ) = 0 ;  method is binary search

     xlo = 0.0;
     xhi = 1.0;

     do
     {	xav = 0.5 * ( xlo + xhi );
	     fav = ff(xav, tpfi, tpig);
     	if(fabs(fav)>smallx)
	     {	if(fav<0.0)
     			xlo = xav;
      		if(fav>0.0)
          		xhi = xav;
	     }
     } while(fabs(fav)>smallx);

     beta = pinv*(1.0-xav)/xav;
     conw = cond+conc*dend*fmof;
     dtb 	= tpdr-tamb;
     dti 	= tpig-tamb;
     ratio = (hvap+cpm*dtb)/(chtd*dti);
     rhoc = dend*chtd*(1.0+fmof*ratio);
     tmig = pow2(beta/hbar)*conw*rhoc;

     return tmig;
}


//------------------------------------------------------------------------------
//
//	DuffBurn
//
//------------------------------------------------------------------------------


void BurnUp::DuffBurn(double wdf, double dfm, double *dfi, double *tdf)
{
//c     Duff burning rate (ergo, intensity) and duration

	double ff;

     *dfi = 0.0;
     *tdf = 0.0;
     if((wdf<=0.0) || (dfm >=1.96))
     	return;
     *dfi=11.25-4.05*dfm;
     ff=0.837-0.426*dfm;
     *tdf=1.e+04*ff*wdf/(7.5-2.7*dfm);

     Smoldering[MAXNO]+=((ff*wdf)/(*tdf));
}


//------------------------------------------------------------------------------
//
//	Arrays
//
//------------------------------------------------------------------------------



void BurnUp::Arrays()
{

//     subroutine ARRAYS( maxno , number , wdry , ash , dendry , fmois ,
//     +                   sigma , htval , cheat , condry , tpig , tchar ,
//     +                   diam , key , work , ak , elam , alone , xmat ,
//     +                   wo , maxkl )

//c  Orders the fuel description arrays according to the paradigm described in
//c  subroutine SORTER and computes the interaction matrix xmat from the array
//c  elam and the list alone returned from subroutine OVLAPS.  Parameters in
//c  arrays are defined in terms of initial values as:

//c       wdry            ovendry mass loading , kg / sq m
//c       ash             mineral content , fraction dry mass
//c       dendry          ovendry mass density , kg / cu m
//c       fmois           moisture content , fraction dry mass
//c       sigma           surface to volume ratio , 1 / m
//c       htval           low heat of combustion , J / kg
//c       cheat           specific heat capacity , ( J / K ) / kg dry mass
//c       condry          thermal conductivity , W / m  K , ovendry
//c       tpig            ignition temperature , K
//c       tchar           char temperature , K
//c       diam            initial diameter , m [ by interaction pairs ]
//c       key             ordered index list
//c       work            workspace array
//c       elam            interaction matrix from OVLAPS
//c       alone           noninteraction fraction list from OVLAPS
//c       xmat            consolidated interaction matrix
//c       wo              initial dry loading by interaction pairs


//      real*4 wdry( maxno ) , ash( maxno ) , dendry( maxno )
//      real*4 fmois( maxno ) , sigma( maxno ) , htval( maxno )
//      real*4 cheat( maxno ) , condry( maxno ) , tpig( maxno )
//      real*4 tchar( maxno ) , work( maxno )
//      real*4 elam( maxno , maxno ) , alone( maxno )
//      real*4 xmat( maxkl ) , diam( maxkl ) , wo( maxkl )
//      integer key( maxno )

     double diak, wtk;
     long j, k, kl, kj;



	Sorter();	//  call SORTER( maxno , number , sigma , fmois , dendry , key )


     for(j=0; j<number; j++)
     {	k = key[j];
          work[j]= wdry[k];
     }
     for(j=0; j<number; j++)
          wdry[j] = work[j];
     for(j=0; j<number; j++)
     {    k = key[j];
          work[j]= ash[k];
     }
     for(j=0; j<number; j++)
          ash[j] = work[j];
     for(j=0; j<number; j++)
     {    k = key[j];
          work[j]= htval[k];
     }
     for(j=0; j<number; j++)
          htval[j]= work[j];
     for(j=0; j<number; j++)
     {    k = key[j];
          work[j]= cheat[k];
     }
     for(j=0; j<number; j++)
          cheat[j] = work[j];
     for(j=0; j<number; j++)
     {    k = key[j];
          work[j]= condry[k];
     }
     for(j=0; j<number; j++)
          condry[j] = work[j];
     for(j=0; j<number; j++)
     {    k = key[j];
          work[j]= tpig[k];
     }
     for(j=0; j<number; j++)
          tpig[j] = work[j];
     for(j=0; j<number; j++)
     {    k = key[j];
          work[j]= tchar[k];
     }
     for(j=0; j<number; j++)
          tchar[j] = work[j];

     OverLaps();   //   call OVLAPS( wdry , sigma , dendry , ak , number , maxno , maxkl , xmat , elam , alone )

     for(k=1; k<=number; k++)	//do k = 1 , number
     {	diak = 4.0/sigma[k-1];
         	wtk = wdry[k-1];
         	kl = loc(k, 0);
         	diam[kl] = diak;
         	xmat[kl] = alone[k-1];
         	wo[kl] = wtk * xmat[kl];
         	for(j=1; j<=k; j++)	//do j = 1 , k
          {	kj = loc(k, j);
            	diam[kj] = diak;
            	xmat[kj] = elam[k-1][j-1];
            	wo[kj] = wtk * xmat[kj];
          }
     }
}


//------------------------------------------------------------------------------
//
//	TempF
//
//------------------------------------------------------------------------------

double BurnUp::TempF(double q, double r)
{
//      function TEMPF( q , r , tamb )

//c  Returns a fire environment temperature , TEMPF , given the fire intensity
//c  q in kW / square meter , the ambient temperature tamb in Kelvins, and the
//c  dimensionless mixing parameter r.

     const double err=1.0e-04;
     const double aa=20.0;

     double term, rlast, den, rnext, test, tempf;

     term=r/(aa*q);
     rlast=r;
     do
     {    den=1.0+term*(rlast+1.0)*(rlast*rlast+1.0);
      	rnext=0.5*(rlast+1.0+r/den);
          test=fabs(rnext-rlast);
      	if(test<err)
          {  	tempf=rnext*tamb;

          	break;
          }
 	     rlast=rnext;
     } while(test>=err);

     return tempf;
}



//----------------------------------------------------------------------------
//
//	Step
//
//----------------------------------------------------------------------------

/*

      subroutine STEP( dt , MXSTEP , now , maxno , number , wo , alfa ,
     +                 dendry , fmois , cheat , condry , diam , tpig ,
     +                 tchar , xmat , tambb , tpdry , fi , flit , fout ,
     +                 tdry , tign , tout , qcum , tcum , acum , qdot ,
     +                 ddot , wodot , work , u , d , r0 , dr , ch2o ,
     +                 ncalls , maxkl , tin , fint , fid )

c  Updates status of all fuel component pairs and returns a snapshot
c
c  Input parameters:
c
c       tin =           start of current time step
c       dt =            time step , sec
c       MXSTEP =        max dimension of historical sequences
c       now =           index marks end of time step
c       maxno =         max number of fuel components
c       number =        actual number of fuel components
c       wo =            current ovendry loading for the larger of
c                       each component pair, kg / sq m
c       alfa =          dry thermal diffusivity of component , sq m / s
c       dendry =        ovendry density of component , kg / cu m
c       fmois =         moisture fraction of component
c       cheat =         specific heat capacity of component , J / kg K
c       condry =        ovendry thermal conductivity , w / sq m K
c       diam =          current diameter of the larger of each
c                       fuel component pair , m
c       tpig =          ignition temperature ( K ) , by component
c       tchar =         end - pyrolysis temperature ( K ) , by component
c       xmat =          table of influence fractions between components
c       tambb =         ambient temperature ( K )
c       tpdry =         temperature ( all components ) start drying ( K )
c       fi =            current fire intensity ( site avg ) , kW / sq m
c       work( k ) =     factor of heat transfer rate hbar * (Tfire - Tchar)
c                       that yields ddot( k )
c       fint( k ) =     correction to fi to compute local intensity
c                       that may be different due to k burning
c       fid =           fire intensity due to duff burning ... this is
c                       used to up the fire intensity for fuel pieces
c                       that are burning without interacting with others
c       plus the following constants and bookkeeping parameters
c       u , d , r0 , dr , ch20 , ncalls , maxkl
c
c  Parameters updated [input and output]
c
c       ncalls =        counter of calls to this routine ...
c                               = 0 on first call or reset
c                               cumulates after first call
c       flit =          fraction of each component currently alight
c       fout =          fraction of each component currently gone out
c       tdry =          time of drying start of the larger of each
c                       fuel component pair
c       tign =          ignition time for the larger of each
c                       fuel component pair
c       tout =          burnout time of larger component of pairs
c       qcum =          cumulative heat input to larger of pair , J / sq m
c       tcum =          cumulative temp integral for qcum ( drying )
c       acum =          heat pulse area for historical rate averaging
c       qdot =          history ( post ignite ) of heat transfer rate
c                       to the larger of component pair , W / sq m
c       ddot =          diameter reduction rate , larger of pair , m / s
c       wodot =         dry loading loss rate for larger of pair
c
c  Constant parameters
c
c       u =             mean horizontal windspeed at top of fuelbed
c       d =             fuelbed depth
c       r0 =            minimum value of mixing parameter
c       dr =            max - min value of mixing parameter
c       ch2o =          specific heat capacity of water , J / kg K
c       hvap =          heat of vaporization of water , J / kg
*/

      //real*4 wo( maxkl ) , alfa( maxno ) , dendry( maxno )
      //real*4 fmois( maxno ) , cheat( maxno )
      //real*4 condry( maxno ) , diam( maxkl ) , tpig( maxno )
      //real*4 tchar( maxno ) , xmat( maxkl )
      //real*4 tdry( maxkl ) , tign( maxkl )
      //real*4 tout( maxkl ) , qcum( maxkl )
      //real*4 tcum( maxkl ) , acum( maxkl )
      //real*4 qdot( maxkl , MXSTEP ) , ddot( maxkl )
      //real*4 wodot( maxkl ) , flit( maxno ) , fout( maxno )
      //real*4 work( maxno ) , fint( maxno )


void BurnUp::Step(double dt, double tin, double fid, long *ncalls)
{
	int nspan;
	long k, l, kl, next, now, mu, index;
     double c, rindef=1.e+30;
     double tnext, tnow, tgo, tdun, tifi;
     double aint, tav1, tav2, tav3, tavg;
     double qdavg, qdsum, tspan, deltim;
     double tlit, ts, r, gi, tf, dia, hf, hb;
     double e, qqq, tst, dnext, wnext, rate, ddt, dryt, dtemp, dqdt;
     double dteff, heff, tfe, dtlite, qd, delt, factor;
     double conwet, dtcum, he, dtef, thd, biot, cpwet, tbar, fac;
     bool flag;

     *ncalls+=1;
     now=*ncalls;
     tnow = tin;
     tnext = tnow + dt;
//c  tifi = time when fire ignition phase ended ( at now = 1 )
     tifi = tnow-((double)(now-1))*dt;
     next = now+1;

     for(k=1; k<=number; k++)	//do k = 1 , number
     {	c = condry[k-1];
          for(l=0; l<=k; l++)	//do l = 0 , k
	     {    kl = loc(k, l);
     	     tdun = tout[kl];

//c  See if k of ( k , l ) pair burned out

          	if(tnow>=tdun)
               {	ddot[kl]=0.0;
	               wodot[kl]=0.0;

                    continue;	   //       goto 10
     		}
            	if(tnext>=tdun)
               {	tgo = tdun-tnow;
               	ddot[kl] = diam[kl]/tgo;
               	wodot[kl] = wo[kl]/tgo;
               	wo[kl] = 0.0;
               	diam[kl]= 0.0;

                    continue;
               }

//c  k has not yet burned out ... see if k of ( k , l ) pair is ignited

            	tlit = tign[kl];
            	if(tnow>=tlit)
               {	ts = tchar[k-1];
               	if(l==0)
                   	{	r = r0 + 0.5 * dr;
                   		gi = fi + fid;
                    }
               	else if(l==k)
	               {    r =r0+0.5*(1.0+flit[k-1])*dr;
     	               gi =fi+flit[k-1]*fint[k-1];
                    }
               	else//(l!=0 && l!=k)
                    {	r = r0+0.5*(1.0+flit[l-1])*dr;
                   		gi =fi+fint[k-1]+flit[l-1]*fint[l-1];
                    }
               	tf = TempF(gi, r);
               	dia = diam[kl];
               	HeatExchange(dia, tf, ts, &hf, &hb, c, &e);
               	qqq = hb * max(tf-ts, 0.0);
               	tst = max(tlit, tifi);
               	nspan = max((long) 1, Nint((tnext-tst)/dt));	//nint((tnext-tst)/dt));
               	if(nspan<=MXSTEP)
                    	qdot[kl][nspan-1]=qqq;
               	else if(nspan>MXSTEP)
  				{	for(mu=2; mu<=MXSTEP; mu++)	//do mu = 2 , MXSTEP
	                    	qdot[kl][mu-2]=qdot[kl][mu-1];
	     	     	qdot[kl][MXSTEP-1]=qqq;
                    }
               	aint = pow2(c/hb);
               	acum[kl]+=(aint*dt);
               	tav1 = tnext-tlit;
               	tav2 = acum[kl]/alfa[k-1];
               	tav3 = pow2(dia/4.0)/alfa[k-1];
                    tavg=tav1;
                    if(tav2<tavg)
                    	tavg=tav2;
                    if(tav3<tavg)
                    	tavg=tav3;
               	index=min(nspan, MXSTEP); //index = 1+min(nspan, MXSTEP);
               	qdsum=0.0;
               	tspan=0.0;
               	deltim=dt;

                    do
                    {	index-=1;
               		if(index==0)	//==1
                    		deltim=tnext-tspan-tlit;
	               	if((tspan+deltim)>=tavg)
     	               	deltim=tavg-tspan;
          	     	qdsum+=(qdot[kl][index]*deltim);
               		tspan+=deltim;
                         if(tspan>=tavg)
                         	break;
                    } while(index>0);

	               qdavg = max(qdsum/tspan, 0.0);
     	          ddot[kl] =qdavg*work[k-1];
          	     dnext = max(0.0, dia-dt*ddot[kl]);
               	wnext = wo[kl]*pow2(dnext/dia);
               	if((dnext==0.0) && (ddot[kl]>0.0))
                    	tout[kl]=tnow+dia/ddot[kl];
               	else if((dnext>0.0) && (dnext<dia))
	               {	rate=dia/(dia-dnext);
                  		tout[kl]=tnow+rate*dt;
                    }
               	if(qdavg<=(double) MXSTEP)       // <=20.0 in Albini's code
                    	tout[kl]=0.5*(tnow+tnext);
               	ddt = min(dt, (tout[kl]-tnow));
               	wodot[kl]=(wo[kl]-wnext)/ddt;
               	diam[kl]=dnext;
               	wo[kl]=wnext;

               	continue;	//goto 10
			}

//c  See if k of ( k , l ) has reached outer surface drying stage yet

            	dryt = tdry[kl];
            	if(tnow>=dryt && tnow<tlit)
               {	if(l==0)
                   	{	r=r0;
                   		gi=fi+fid;
                    }
               	else if(l==k)
                    {	r=r0;
                   		gi=fi;
                    }
               	else// if(l!=0 && l!=k)
                    {	r=r0+0.5*flit[l-1]*dr;
                    	gi=fi+flit[l-1]*fint[l-1];
                    }
	          	tf = TempF(gi, r);
               	ts = tamb;
               	dia = diam[kl];
                    HeatExchange(dia, tf, ts, &hf, &hb, c, &e);
               	//call heatx( u , d , dia , tf , ts , hf , hb , c , e )
               	dtemp=max(0.0, tf-ts);
               	dqdt=hb*dtemp;
               	qcum[kl]+=(dqdt*dt);
               	tcum[kl]+=(dtemp*dt);
               	dteff=tcum[kl]/(tnext-dryt);
               	heff=qcum[kl]/tcum[kl];
               	tfe=ts+dteff;
               	dtlite=rindef;
               	if(tfe>(tpig[k-1]+10.0))
                    	dtlite=TIgnite(tpdry, tpig[k-1], tfe,
               					condry[k-1], cheat[k-1], fmois[k-1], dendry[k-1], heff);
				tign[kl]=0.5*(dryt+dtlite);

//c  If k will ignite before time step over , must interpolate

               	if(tnext>tign[kl])
                    {	ts = tchar[k-1];
                    	HeatExchange(dia, tf, ts, &hf, &hb, c, &e);
                  		qdot[kl][0]=hb*max(tf-ts, 0.0);
                  		qd=qdot[kl][0];
                  		ddot[kl]=qd*work[k-1];
                  		delt=tnext-tign[kl];
                  		dnext=max(0.0, dia-delt*ddot[kl]);
                  		wnext=wo[kl]*pow2(dnext/dia);
                  		if(dnext==0.0)
                         	tout[kl]=tnow+dia/ddot[kl];
                  		else if((dnext>0.0) && dnext<dia)
                         {	rate=dia/(dia-dnext);
		                    tout[kl]=tnow+rate*dt;
                         }
                  		if(tout[kl]>tnow)
                     	{	ddt=min(dt, (tout[kl]-tnow));
                     		wodot[kl]=(wo[kl]-wnext)/ddt;
                         }
                  		else
                     		wodot[kl]=0.0;
                  		diam[kl]=dnext;
                  		wo[kl]=wnext;
                    }

                    continue;         // goto 10
			}

//c  If k of ( k , l ) still coming up to drying temperature , accumulate
//c  heat input and driving temperature difference , predict drying start

            	if(tnow<dryt)
            	{	factor=fmois[k-1]*dendry[k-1];
               	conwet=condry[k-1]+4.27e-04*factor;
               	if(l==0)
                    {	r=r0;
                   		gi=fi+fid;
                    }
               	else if(l==k)
                   	{	r=r0;
                   		gi=fi;
                    }
               	else if((l!=0) && (l!=k))
                    {	r=r0+0.5*flit[l-1]*dr;
                   		gi=fi+flit[l-1]*fint[l-1];
                    }
               	tf = TempF(gi, r);
               	if(tf<=(tpdry+10.0))
                    	continue;	// goto 10
               	dia=diam[kl];
               	ts=0.5*(tamb+tpdry);
                    HeatExchange(dia, tf, ts, &hf, &hb, c, &e);
               	dtcum = max((tf-ts)*dt, 0.0);
               	tcum[kl]+=dtcum;
               	qcum[kl]+=(hb*dtcum);
               	he=qcum[kl]/tcum[kl];
               	dtef=tcum[kl]/tnext;
               	thd=(tpdry-tamb)/dtef;
               	if(thd>0.9)
                    	continue;
               	biot=he*dia/conwet;

               	dryt=DryTime(biot, thd);
               	cpwet=cheat[k-1]+ch2o*fmois[k-1];
               	fac=pow2(0.5*dia)/conwet;
               	fac=fac*cpwet*dendry[k-1];
               	tdry[kl]=fac*dryt;
               	if(tdry[kl]<tnext)
                    {	ts=tpdry;
                  		HeatExchange(dia, tf, ts, &hf, &hb, c, &e);
                  		dqdt=hb*(tf-ts);
                  		delt=tnext-tdry[kl];
                  		qcum[kl]=dqdt*delt;
                  		tcum[kl]=(tf-ts)*delt;
                  		tbar=0.5*(tpdry+tpig[k-1]);

//c  See if ignition to occur before time step complete

                  		if(tf<=(tpig[k-1]+10.0))
                         	continue;
                         dtlite=TIgnite(tpdry, tpig[k-1], tf, condry[k-1],
                         			cheat[k-1] , fmois[k-1] , dendry[k-1], hb);
                  		tign[kl]=0.5*(tdry[kl]+dtlite);
                  		if(tnext>tign[kl])
                    	{	ts=tchar[k-1];
                    		qdot[kl][0]=hb*max(tf-ts, 0.0);
                         }
				}
			}
		}
	}

//c  Update fractions ignited and burned out , to apply at next step start

     for(k=1; k<=number; k++)	// do k = 1 , number
     {	flit[k-1]=0.0;
         	fout[k-1]=0.0;
         	for(l=0; l<=k; l++)	//do l = 0 , k
          {  	kl=loc(k, l);
            	if(tnext>=tign[kl])
            		flag=true;
            	else
            		flag=false;
            	if(flag && tnext<=tout[kl])
            		flit[k-1]+=xmat[kl];
            	if(tnext>tout[kl])
               	fout[k-1]+=xmat[kl];
          }
     }
}


//------------------------------------------------------------------------------
//
//	Start
//
//------------------------------------------------------------------------------


//      subroutine START( dt , MXSTEP , now , maxno , number , wo , alfa ,
//     +                 dendry , fmois , cheat , condry , diam , tpig ,
//     +                 tchar , xmat , tambb , tpdry , fi , flit , fout ,
//     +                 tdry , tign , tout , qcum , tcum , acum , qdot ,
//     +                 ddot , wodot , work , u , d , r0 , dr , ch2o ,
//     +                 ncalls , maxkl )

//c  This routine initializes variables prior to starting sequence of calls
//c  to subroutine STEP.  On input here, fi is area intensity of spreading
//c  fire , dt is the residence time for the spreading fire.  Only physical
//c  parameters specified are the fuel array descriptors.  To call STEP ,
//c  one must initialize the following variables.
//c
//c  Input parameters:
//c
//c       dt =            spreading fire residence time , sec
//c       MXSTEP =        max dimension of historical sequences
//c       now =           index marks end of time step
//c       maxno =         max number of fuel components
//c       number =        actual number of fuel components
//c       wo =            current ovendry loading for the larger of
//c                       each component pair, kg / sq m
//c       alfa =          dry thermal diffusivity of component , sq m / s
//c       dendry =        ovendry density of component , kg / cu m
//c       fmois =         moisture fraction of component
//c       cheat =         specific heat capacity of component , J / kg K
//c       condry =        ovendry thermal conductivity , W / sq m K
//c       diam =          current diameter of the larger of each
//c                       fuel component pair , m
//c       tpig =          ignition temperature ( K ) , by component
//c       tchar =         end - pyrolysis temperature ( K ) , by component
//c       xmat =          table of influence fractions between components
//c       tambb =         ambient temperature ( K )
//c       fi =            current fire intensity ( site avg ) , kW / sq m
//c
//c  Parameters updated [input and output]
//c
//c       ncalls =        counter of calls to this routine ...
//c                               = 0 on first call or reset
//c                               cumulates after first call
//c       flit =          fraction of each component currently alight
//c       fout =          fraction of each component currently gone out
//c       tdry =          time of drying start of the larger of each
//c                       fuel component pair
//c       tign =          ignition time for the larger of each
//c                       fuel component pair
//c       tout =          burnout time of larger component of pairs
//c       qcum =          cumulative heat input to larger of pair , J / sq m
//c       tcum =          cumulative temp integral for qcum ( drying )
//c       acum =          heat pulse area for historical rate averaging
//c       qdot =          history ( post ignite ) of heat transfer rate
//c                       to the larger of each component pair
//c       ddot =          diameter reduction rate , larger of pair , m / s
//c       wodot =         dry loading loss rate for larger of pair
//c
//c  Constant parameters
//c
//c       u =             mean horizontal windspeed at top of fuelbed
//c       d =             fuelbed depth
//c       r0 =            minimum value of mixing parameter
//c       dr =            max - min value of mixing parameter
//c       ch2o =          specific heat capacity of water , J / kg K
//c       hvap =          heat of vaporization of water J / kg
//c       tpdry =         temperature ( all components ) start drying ( K )

//      real*4 wo( maxkl ) , alfa( maxno ) , dendry( maxno )
//      real*4 fmois( maxno ) , cheat( maxno )
//      real*4 condry( maxno ) , diam( maxkl ) , tpig( maxno )
//      real*4 tchar( maxno ) , xmat( maxkl )
//      real*4 tdry( maxkl ) , tign( maxkl )
//      real*4 tout( maxkl ) , qcum( maxkl )
//      real*4 tcum( maxkl ) , acum( maxkl )
//      real*4 qdot( maxkl , MXSTEP ) , ddot( maxkl )
//      real*4 wodot( maxkl ) , flit( maxno ) , fout( maxno )
//      real*4 work( maxno )



bool BurnUp::Start(double dt, long now, long *ncalls)
{
      long k, l, kl, nlit;
      double delm, heatk, r, tf, ts, thd, tx, factor;
      double conwet, dia, hb, en, cpwet, fac;
      double hf, dryt, tsd, c, tigk, e, dtign, trt;
      double aint, ddt, dnext, wnext, df;
      const double rindef=1.e+30;

//c  Initialize time varying quantities and set up work( k )
//c  The diameter reduction rate of fuel component k is given
//c  by the product of the rate of heat transfer to it, per
//c  unit surface area, and the quantity work( k )

     for(k=1; k<=number; k++)	//do k = 1 , number
     {	fout[k-1]=0.0;
         	flit[k-1]=0.0;
         	alfa[k-1]=condry[k-1]/(dendry[k-1]*cheat[k-1]);
//c        	effect of moisture content on burning rate (scale factor)
         	delm=1.67* fmois[k-1];
//c        	effect of component mass density (empirical)
         	heatk=dendry[k-1] / 446.0;
//c        	empirical burn rate factor, J / cu m - K
         	heatk=heatk*2.01e+06*(1.0+delm);
//c        	normalize out driving temperature difference (Tfire - Tchar)
//c        	to average value of lab experiments used to find above constants
          work[k-1]=1.0/(255.0*heatk);
          for(l=0; l<=k; l++)	//do l = 0 , k
          {	kl=loc(k, l);
            	tout[kl]=rindef;
            	tign[kl]=rindef;
            	tdry[kl]=rindef;
            	tcum[kl]=0.0;
            	qcum[kl]=0.0;
          }
	}

//c  Make first estimate of drying start times for all components
//c  These times are usually brief and make little or no difference

      r=r0+0.25*dr;
      tf=TempF(fi, r);
      ts=tamb;
      if(tf<=(tpdry+10.0))
      {	strcat(Message, " STOP: Igniting fire cannot dry fuel");

      	return false;
      }
      thd=(tpdry-ts)/(tf-ts);
      tx=0.5*(ts+tpdry);
      //tpamb=tamb;

      for(k=1; k<=number; k++)	//do k = 1 , number
      {	factor=dendry[k-1]*fmois[k-1];
         	conwet=condry[k-1]+4.27e-04*factor;
         	for(l=0; l<=k; l++) //do l = 0 , k
          {	kl=loc(k, l);
            	dia=diam[kl];
            	HeatExchange(dia, tf, tx, &hf, &hb, conwet, &en);
            	dryt=DryTime(en, thd);
            	cpwet=cheat[k-1]+fmois[k-1]*ch2o;
            	fac=pow2(0.5*dia)/conwet;
            	fac=fac*dendry[k-1]*cpwet;
            	dryt=fac*dryt;
            	tdry[kl]=dryt;
          }
      }

//c  Next , determine which components are alight in spreading fire

     tsd=tpdry;

     for(k=1; k<=number; k++)	//do k = 1 , number
     {	c=condry[k-1];
         	tigk=tpig[k-1];
         	for(l=0; l<=k; l++)	//do l = 0 , k
          {   	kl=loc(k, l);
              	dryt=tdry[kl];
              	if(dryt>=dt)
               	continue;
              	dia=diam[kl];
              	ts=0.5*(tsd+tigk);
              	HeatExchange(dia, tf, ts, &hf, &hb, c, &e);
              	tcum[kl]=max((tf-ts)*(dt-dryt), 0.0);
              	qcum[kl]=hb * tcum[kl];
              	if(tf<=(tigk+10.0))
               	continue;
               dtign=TIgnite(tpdry, tpig[k-1], tf, condry[k-1], cheat[k-1], fmois[k-1], dendry[k-1], hb);
              	trt=dryt+dtign;
              	tign[kl]=0.5*trt;
              	if(dt>trt)
               	flit[k-1]+=xmat[kl];
  		}
  	}
     nlit = 0;
     trt = rindef;

//c  Determine minimum ignition time and verify ignition exists

     for(k=1; k<=number; k++)	//do k = 1 , number
     {	if(flit[k-1]>0.0)
     		nlit+=1;
         	for(l=0; l<=k; l++)		//do l = 0 , k
          {	kl=loc(k, l);
            	trt=min(trt, tign[kl]);
          }
     }
	if(nlit==0)
     {	strcat(Message, " STOP: No Fuel Ignited");

     	return false;
     }
//c  Deduct trt from all time estimates , resetting time origin

     for(k=1; k<=number; k++)	//do k = 1 , number
     {	for(l=0; l<=k; l++)//	do l = 0 , k
          {	kl = loc(k, l);
            	if(tdry[kl]<rindef)
               	tdry[kl]-=trt;
            	if(tign[kl]<rindef)
	               tign[kl]-=trt;
          }
     }

//c  Now go through all component pairs and establish burning rates
//c  for all the components that are ignited; extrapolate to end time dt

	for(k=1; k<=number; k++)	//do k = 1 , number
     {	if(flit[k-1]==0.0)
     	{	for(l=0; l<=k; l++)//     do l = 0 , k
          	{	kl=loc(k, l);
               	ddot[kl]=0.0;
               	tout[kl]=rindef;
               	wodot[kl]=0.0;
               }
          }
         	else
          {	ts=tchar[k-1];
            	c=condry[k-1];
            	for(l=0; l<=k; l++)	//do l = 0 , k
               {	kl=loc(k, l);
               	dia=diam[kl];
               	HeatExchange(dia, tf, ts, &hf, &hb, c, &e);

               	qdot[kl][now-1]=hb*max((tf-ts), 0.0);
               	aint=pow2(c/hb);
               	ddt=dt-tign[kl];
               	acum[kl]=aint * ddt;
               	ddot[kl]=qdot[kl][now-1]*work[k-1];
               	tout[kl]=dia/ddot[kl];
               	dnext=max(0.0, (dia-ddt*ddot[kl]));
               	wnext=wo[kl]*pow2(dnext/dia);
               	wodot[kl]=(wo[kl]-wnext)/ddt;
               	diam[kl]=dnext;
               	wo[kl]=wnext;
               	df=0.0;
               	if(dnext<=0.0)
                  	{	df=xmat[kl];
                  		wodot[kl]=0.0;
                  		ddot[kl]=0.0;
                    }
               	flit[k-1]-=df;
               	fout[k-1]+=df;
               }
          }
     }
     *ncalls = 0;

     return true;
}


//------------------------------------------------------------------------------
//
//	Sorter
//
//------------------------------------------------------------------------------



//      subroutine SORTER( maxno , number , sigma , fmois , dendry , key )
//
//c  Sorts fuel element list in order of increasing size (decreasing sigma)
//c  For elements with same size, order determined on increasing moisture
//c  content (fmois).  If items have same size and moisture content, order
//c  on the basis of increasing mass density (dendry).  "number" elements are
//c  included in the list, which has a maximum length of "maxno".  The integer
//c  list:  key( j ) , j = 1 , number holds the indices in order, so other
//c  fuel parameters can be ordered and associated as necessary.

//      real*4 sigma( maxno ) , fmois( maxno ) , dendry( maxno )
//      integer key( maxno )
void BurnUp::Sorter()
{
	long i, j, keep;
     double s, fm, de, usi;
     bool diam, mois, dens, tied, earlyout;

     for(j=0; j<MAXNO; j++) 	//do j=1 , maxno
     	key[j]=j;

//c  Replacement sort: order on increasing size , moisture , density

     for(j=2; j<=number; j++)		//do j = 2 , number
     {	s=1.0/sigma[j-1];
         	fm = fmois[j-1];
         	de = dendry[j-1];
         	keep = key[j-1];
          for(i=(j-2); i>=0; i--)	//do i = ( j - 1 ) , 1 , -1
          {    earlyout=true;
          	usi = 1.0/sigma[i];
               if(usi<s)
               	diam=true;
               else
               	diam=false;
            	if(diam)
               	break;// goto 10
               if(usi==s)
               	tied=true;
               else
               	tied=false;
            	if(tied) 			//goto 05
               {    if(fmois[i]<fm)
               		mois=true;
               	else
                    	mois=false;
            		if( mois )
                    	break;	//goto 10
                    if(fmois[i]==fm)
               		tied=true;
               	else
                    	tied=false;
            		if(tied)
            		{    if(dendry[i]<=de)
                    		dens=true;
                    	else
                         	dens=false;
            			if(dens)
                    		break;// goto 10
                    }
               }
          	sigma[i+1]=sigma[i];
            	fmois[i+1]=fmois[i];
            	dendry[i+1]=dendry[i];
           	key[i+1]=key[i];
               earlyout=false;
  		}
          if(!earlyout)
	         	i=0;
     	sigma[i+1]=1.0/s;
         	fmois[i+1]=fm;
         	dendry[i+1]=de;
         	key[i+1]=keep;
  	}
}



//------------------------------------------------------------------------------
//
//	OvLaps
//
//------------------------------------------------------------------------------


void BurnUp::OverLaps()
{
// wdry , sigma , dendry , ak , number , maxno , maxkl , beta , elam , alone , area )

//  Computes the interaction matrix elam( j , k ) which apportions the
//  influence of smallxer and equal size pieces on each size class for the
//  purpose of establishing the rates at which the elements burn out.
//  Input quantities are:  wdry , the ovendry mass per unit area of each
//  element available for burning after the passage of the igniting surface
//  fire;  sigma , the particle's surface / volume ratio , and dendry , the
//  ovendry mass density of the particle; ak a dimensionless parameter that
//  scales the planform area of a particle to its area of influence. There
//  are "number" separate particle classes, of a maximum number = maxno.
//  It is assumed that the lists are ordered on size class (nonincreasing
//  surface / volume ratio). List "alone" gives the fraction of each loading
//  that is not influenced by any other category.

      //double wdry[MAXNO] , sigma[MAXNO] , dendry[MAXNO];
      //double beta[MAXKL] , elam[MAXNO][MAXNO] , alone[MAXNO];
      //double area[MAXNO];

	long j, k, l, kj, kl;
     double a, bb, pi, siga, frac;


     pi = fabs(acos(-1.0));
     for(j=1; j<=number; j++)	//do j = 1 , number
     {	alone[j-1]=0.0;
          for(k=1; k<=j; k++)//    	do k = 1 , j
          {	kj = loc(j, k);
               xmat[kj]=0.0;
          }
         	for(k=1; k<=number; k++) //do k = 1 , number
          	elam[j-1][k-1] = 0.0;
	}
     for(k=1; k<=number; k++)	// do k = 1 , number
     {    for(l=1; l<=k; l++)	//do l = 1 , k
          {	ak=3.25*exp(-20.0*pow2(fmois[l-1]));
		    	siga = ak * sigma[k-1]/pi;
          	kl = loc(k, l);
            	a = siga*wdry[l-1]/dendry[l-1];
            	if(k==l)
	          {    bb = 1.0 - exp(-a);
               	if(bb<1e-30)
                    	bb=1e-30;
     	          area[k-1]=bb;
               }
               else //if(k!=1)
               	bb = min(1.0, a);
	          xmat[kl]=bb;
          }
     }
     if(number==1)
     {	elam[0][0]=xmat[1];
         	alone[0]=1.0- elam[0][0];

         	return;
	}
     for(k=1; k<=number; k++)	//   do k = 1 , number
     {	frac=0.0;
         	for(l=1; l<=k; l++)	//	do l = 1 , k
          {	kl=loc(k, l);
            	frac+=xmat[kl];
          }
         	if(frac>1.0)
          {	for(l=1; l<=k; l++)	//do l = 1 , k
          	{	kl=loc(k, l);
               	elam[k-1][l-1]=xmat[kl]/frac;
               }
	          alone[k-1]=0.0;
          }
         	else
          {	for(l=1; l<=k; l++)	//  do l = 1 , k
          	{	kl=loc(k, l);
               	elam[k-1][l-1]=xmat[kl];
	          }
     	     alone[k-1]=1.0-frac;
	     }
     }
}


//------------------------------------------------------------------------------
//
//      subroutine HEATX()
//
//------------------------------------------------------------------------------



void BurnUp::HeatExchange(double dia , double tf , double ts ,
				double *hfm , double *hbar , double cond, double *en)// returns en
{

//  Given horizontal windspeed u at height d [top of fuelbed], cylindrical
//  fuel particle diameter dia, fire environment temperature tf, and mean
//  surface temperature, ts, subroutine returns film heat transfer coefficient
//  hfm and an "effective" film heat transfer coefficient including radiation
//  heat transfer, hbar.  Using the wood's thermal conductivity, cond, the
//  modified Nusselt number [ en ] used to estimate onset of surface drying
//  is returned as well.

      double v, re, enuair, conair, fac, hfmin;

      const double g = 9.8;
      const double vis = 7.5e-05;
      const double a = 8.75e-03;
      const double b = 5.75e-05;
      const double rad = 5.67e-08;
      const double fmfac = 0.382;
      const double hradf = 0.5;
      double hrad;

      *hfm = 0.0;
      if(dia>b)
      {	v		=sqrt(u*u+0.53*g*d);
      	re		=v*dia/vis;
	     enuair	=0.344*pow(re, 0.56);
	     conair	=a+b*tf;
	     fac 		=sqrt(fabs(tf-ts)/dia);
	     hfmin 	=fmfac*sqrt(fac);
     	*hfm		=max((enuair*conair/dia), hfmin);
	}
     hrad=hradf*rad*(tf+ts)*(tf*tf+ts*ts);
     *hbar=*hfm+hrad;
     *en=*hbar*dia/cond;
}


//------------------------------------------------------------------------------
//
//		FIRINT.CPP
//
//------------------------------------------------------------------------------


double BurnUp::FireIntensity()
{
 	//wodot , ash , htval , maxno , number , maxkl , area , fint , fi )

//  Computes fi = site avg fire intensity given the burning rates of all
// interacting pairs of fuel components [ wodot ] , the mineral ash content
// of each component [ ash ] , the heat of combustion value [ htval ] for
// each , and the number of fuel components [ number ] , where max = maxno.
// fi is in kW / sq m , while htval is in J / kg.

// fint( k ) is the correction to fi to adjust
// the intensity level to be the local value where size k is burning.

//      real*4 ash( maxno ) , htval( maxno )
//      real*4 wodot( maxkl ) , area( maxno ) , fint( maxno )
//      data smallx / 1.e-06 /

	 long k, l, kl, k0;
      double sum, wdotk, ark, term;
      double wnoduff, wnoduffsum, noduffsum, test, noduffterm;//, fintnoduff[MAXNO];
      double fracf;

	 sum=noduffsum=wnoduffsum=0.0;
      for(k=1; k<=number; k++)
      {	wdotk=wnoduff=0.0;
          for(l=0; l<=k; l++)
          {	kl = loc(k, l);
            	wdotk+=wodot[kl];
               //if(l>0)
               //	wnoduff+=wodot[kl];
               //else
   		     // 	Smoldering[k-1]+=wodot[kl];
          }
          term=(1.0-ash[k-1])*htval[k-1]*wdotk*1.e-03;
          ark=area[k-1];
		if(ark>smallx)
         	   	fint[k-1]=term/ark-term;
         	else
             	fint[k-1] = 0.0;
          k0=loc(k, 0);
          Smoldering[k-1]=wodot[k0];
          wnoduff=wdotk-Smoldering[k-1];
          noduffterm=(1.0-ash[k-1])*htval[k-1]*wnoduff*1.e-03;
          if(wnoduff>0.0)
		{	fracf=wnoduff/wdotk;
	      	test=fracf*fint[k-1];
          }
          else
          	test=0.0;

//--------------------------------------
//	flaming and smoldering decision here
//--------------------------------------
          if(test>FintSwitch/ark-FintSwitch)
	         	Flaming[k-1]+=wnoduff;		//wdotk;
     	else
          	Smoldering[k-1]+=wnoduff;	//wdotk;
//--------------------------------------
//--------------------------------------
         	sum+=term;
          noduffsum+=noduffterm;
          wnoduffsum+=wnoduff;
     }

     return sum;
}


void	BurnUp::SetFintSwitch(double fint)
{
	FintSwitch=fint;
}


long BurnUp::GetFintSwitch()
{
	return FintSwitch;
}


//------------------------------------------------------------------------------
//
//    subroutine DRYTIM( enu , theta , tau )
//
//------------------------------------------------------------------------------


double BurnUp::func(double h, double theta)
{
	const double a = 0.7478556;
     const double b = 0.4653628;
     const double c = 0.1282064;

	return h*(b-h*(c-h))-(1.0-theta)/a;
}


double BurnUp::DryTime(double enu, double theta)
{

//  Given a Nusselt number ( enu , actually Biot number = h D / k )
//  and the dimensionless temperature rise required for the start
//  of surface drying ( theta ), returns the dimensionless time ( tau )
//  needed to achieve it.  The time is given multiplied by thermal
//  diffusivity and divided by radius squared.  Solution by binary search.

	long n;
     double tau;
     double x, xl, xh, xm;
     const double p = 0.47047;

     xl=0.0;
     xh=1.0;
     for(n=0; n<15; n++)
     {	xm=0.5*(xl+xh);
     	if(func(xm, theta)<0.0)
          	xl = xm;
         	else
	         	xh = xm;
     }
     x=(1.0/xm-1.0)/p;
     tau=pow2(0.5*x/enu);

     return tau;
}




//------------------------------------------------------------------------------
//
//	Stash
//
//------------------------------------------------------------------------------



//      subroutine STASH( time , now , maxno , number , outfil , fi ,
//     +                  flit , fout , wo , wodot , diam , ddot ,
//     +                  tdry , tign , tout , fmois , maxkl , nun )

//c  This routine stashes output from the BURNUP model package on a snapshot
//c  basis.  Every time it is called, it "dumps" a picture of the status of
//c  each component of the fuel complex, as a table of interacting pairs.

//      real*4 wo( maxkl ) , wodot( maxkl )
//      real*4 diam( maxkl ) , ddot( maxkl )
//      real*4 flit( maxno ) , fout( maxno )
//      real*4 tdry( maxkl ) , tign( maxkl )
//      real*4 tout( maxkl ) , fmois( maxno )
//      character*12 outfil , histry
//      logical snaps


void BurnUp::Stash(char *HistFile, char *SnapFile, double time, long now)
{
	long m, mn, n;
     double wd, wg, wgm;
     double wdm, wgf, wdf, fmm;
     FILE *histfile, *snapfile;

     if(now==1)//	if( now .EQ. 1 ) then
     {    histfile=fopen(HistFile, "w");
     	snapfile=fopen(SnapFile, "w");

     	wd = 0.0;
         	wg = 0.0;
         	for(m=1; m<=number; m++)	//do m = 1 , number
          {	fmm = fmois[m-1];
	          wdm = 0.0;
            	for(n=0; n<=m; n++)	//do n = 0 , m
               {    mn = loc(m, n);
               	wdm = wdm + wo[mn];
               }
            	wgm=wdm*(1.0+fmm);
            	wd+=wdm;
            	wg+=wgm;
          }
         	wd0=wd;
         	wg0=wg;
	}
     else
     {    histfile=fopen(HistFile, "a");
     	snapfile=fopen(SnapFile, "a");
     }
     fprintf(snapfile, "\n%lf %lf\n", time, fi);
	wd=0.0;
   	wg=0.0;
   	for(m=1; m<=number; m++)	//do m = 1 , number
    	{	fmm=fmois[m-1];
         	wdm=0.0;
          fprintf(snapfile, "%lf %lf %lf\n", time, flit[m-1], fout[m-1]);
   		for(n=0; n<=m; n++)	//do n = 0 , m
        	{	mn=loc(m, n);
            	wdm+=wo[mn];//(wo[mn]*wodot[mn]*dt);
               fprintf(snapfile, "     %lf %lf %lf %lf %lf %lf %lf %lf\n",
               	time, wo[mn], wodot[mn], diam[mn], ddot[mn], tdry[mn], tign[mn], tout[mn]);
		}
          fprintf(snapfile, "\n");
          wgm=wdm*(1.0+fmm);
    	     wd+=wdm;
    		wg+=wgm;
	}
     wgf = wg/wg0;
     wdf = wd/wd0;
     fprintf(histfile, "%lf %lf %lf %lf %lf %lf\n", time, wg, wd, wgf, wdf, fi);

     fclose(histfile);
     fclose(snapfile);
}


bool	BurnUp::AllocBurnStruct()
{
     FreeBurnStruct();

     bs=new BurnStruct[ntimes];
     if(bs==NULL)
     	return false;

     ZeroMemory(bs, ntimes*sizeof(BurnStruct));

	return true;
}


void	BurnUp::FreeBurnStruct()
{
	if(bs)
     	delete[] bs;
     bs=0;
}


void BurnUp::SetBurnStruct(double time, long now)
{
	long i, m, mn, n;
     double wd;
     double wdm, wdf;
     double wt[2]={0.0, 0.0};

     if(now==1)//	if( now .EQ. 1 ) then
     {	wd = 0.0;
         	for(m=1; m<=number; m++)	//do m = 1 , number
          {    wdm = 0.0;
            	for(n=0; n<=m; n++)	//do n = 0 , m
               {    mn = loc(m, n);
               	wdm = wdm + wo[mn];
               }
            	wd+=wdm;
          }
         	wd0=wd;

	}
	wd=0.0;
   	for(m=1; m<=number; m++)	//do m = 1 , number
    	{   	wdm=0.0;
   		for(n=0; n<=m; n++)	//do n = 0 , m
        	{	mn=loc(m, n);
            	wdm+=wo[mn];//(wo[mn]*wodot[mn]*dt);
		}
    	     wd+=wdm;
	}
     wdf = wd/wd0;

     for(i=0; i<MAXNO; i++)
	{    wt[0]+=Flaming[i];
          wt[1]+=Smoldering[i];
          Flaming[i]=Smoldering[i]=0.0;
     }
     wt[1]+=Smoldering[MAXNO];

     if(now>ntimes)	// don't overflow the buffer
     	return;

     bs[now-1].time=time;
     bs[now-1].wdf=wdf;
     if(wt[0]+wt[1]>0.0)
     	bs[now-1].ff=wt[0]/(wt[0]+wt[1]);
     else
     	bs[now-1].ff=0.0;
}


bool BurnUp::Summary(char *OutFile)
{
	long m, n, mn;
     double fr, ti, ts, to, tf, wd, rem, di;
	FILE *outfile;


     if((outfile=fopen(OutFile, "w"))==NULL)
     	return false;

     fprintf(outfile, "%s\t %s\t %s\t %s\t %s\t %s\t %s\t %s\n", "#", "wt", "fmst", "~D", "ts", "tf", "rem", "%rem");
     for(m=1; m<=number; m++)
     {	fprintf(outfile, "%3ld\t %3.2lf\t %3.2lf\t %3.2lf\t", m, wdry[m-1], fmois[m-1], 4.0/sigma[m-1]);
		rem=0.0;
          ts=1.0e31;
          tf=0.0;
     	for(n=0; n<=m; n++)
          {   	mn = loc(m, n);
              	fr = xmat[mn];
              	ti = tign[mn];
              	ts = min(ti, ts);
              	to = tout[mn];
              	tf = max(to, tf);
              	wd = wo[mn];
              	rem = rem + wd;
              	di = diam[mn];
          //     fprintf(outfile, "\n     %ld %lf %lf %lf %lf", n, fr, ti, to, wd, di);
          }
          fprintf(outfile, " %8.2lf\t %10.2lf\t %3.4lf\t %3.2lf\n", ts, tf, rem, rem/wdry[m-1]);
     }
     fclose(outfile);

     return true;

}


//------------------------------------------------------------------------------
//
//	CurveFit Stuff
//
//------------------------------------------------------------------------------
/*
long BurnUp::GetRegressionCoefficients(double **coefs, long *NumCalcs)
{
     *coefs=aa;
     *NumCalcs=now;

	return ma;
}
*/
bool BurnUp::AllocRegressionData(long ndata)
{
	if(ndata>=NumAllocRegrData)
     {	FreeRegressionData(ndata);

	     x=new double[ndata];
	     y=new double[ndata];
          if(x==NULL || y==NULL)
          {	FreeRegressionData(ndata);

          	return false;
          }

          NumAllocRegrData=ndata;
     }
     ZeroMemory(x, ndata*sizeof(double));
     ZeroMemory(y, ndata*sizeof(double));

     return true;
}

void BurnUp::FreeRegressionData(long /*ndata*/)
{
	if(x)
	     delete[] x;
	if(y)
	     delete[] y;
     x=y=0;
     NumAllocRegrData=0;
}


long BurnUp::GetSamplePoints(long Flaming, long NumberOfPts)//, double **xs, double **ys, long *numcalcs)
{
	long i, j;
     long NewPointPos, *pts, NewPoint, NumInSample, MaxNum;
     double NewX, NewY;
	double interval, xtest, ytest, ymid, ydiff, ymax;

     ma=NumberOfPts;
     if(ma<4)
     	ma=4;

	AllocRegressionData(ma);
     pts=new long[ma];

     NumInSample=3;

	if(Flaming)	// find end of flaming period
     {	for(i=0; i<now; i++)
     	{	if(bs[i].ff>0.0)
          		MaxNum=i;
          }
          MaxNum++;
     }
     else
     	MaxNum=now;

     MaxNum--;
     while(bs[MaxNum].time==0.0)
     {	MaxNum--;
	 	if(MaxNum<0)
              	break;
     }
     interval=(double) (MaxNum)/(double) NumInSample;
		// fill out the initial array
     if(interval==0)
     {	if(pts)
	     	delete[] pts;
          return NumInSample;
     }
     else if(interval<1.0)
     	interval=1.0;

	NumInSample++;
  	for(i=0; i<NumInSample; i++)
     {	j=(long)((double) i * interval);
	     //GetBurnStruct(j, &bpx);
          pts[i]=j;
    	    	x[i]=bs[j].time/60.0;
          if(!Flaming)
        		y[i]=bs[j].wdf;  // for nonlinear stuff and .wdf
          else
               y[i]=bs[j].ff;
     }
     NewPointPos=0;
     NewPoint=1;
     NewX=x[0];
     do
     {	// check error for individual spans
          ymax=0.0;
          for(i=1; i<NumInSample; i++)
          {    interval=pts[i]-pts[i-1];
          	j=(long) ((double) pts[i-1] + interval/2.0);
               if(j<1)
               	continue;
          	//GetBurnStruct(j, &bpx);
			xtest=bs[j].time/60.0;//bpx.time/3600.0;
               if(!Flaming)
               	ytest=bs[j].wdf;//bpx.wdf;  // for nonlinear stuff and .wdf
               else
		          ytest=bs[j].ff;//bpx.fi;
               if(fabs(x[i-1]-x[i])>1e-6)
               {	ymid=y[i-1]-(y[i-1]-y[i])*(x[i-1]-xtest)/(x[i-1]-x[i]);
               	ydiff=fabs(ymid-ytest);
               	if(ydiff>ymax)
               	{	ymax=ydiff;
                    	NewX=xtest;
                    	NewY=ytest;
               		NewPointPos=i;
                    	NewPoint=j;
                    }
               }
          }
          if(NewX!=x[NewPointPos])
          {    MoveMemory(&x[NewPointPos+1], &x[NewPointPos], (NumInSample-NewPointPos)*sizeof(double));
          	MoveMemory(&y[NewPointPos+1], &y[NewPointPos], (NumInSample-NewPointPos)*sizeof(double));
          	MoveMemory(&pts[NewPointPos+1], &pts[NewPointPos], (NumInSample-NewPointPos)*sizeof(long));
          	x[NewPointPos]=NewX;
          	y[NewPointPos]=NewY;
          	pts[NewPointPos]=NewPoint;
          	NumInSample++;
          }
          else
          	ma--;
     } while(NumInSample<ma);
     if(pts)
	     delete[] pts;

     //*xs=x;
     //*ys=y;
     //*numcalcs=MaxNum;

     return ma;
}


bool BurnUp::GetSample(long num, double *xpt, double *ypt)
{
	if(num<NumAllocRegrData)
     {	*xpt=x[num];
     	*ypt=y[num];

          return true;
     }

     return false;
}

bool BurnUp::GetBurnStruct(long Num, BurnStruct *Bs)
{
	if(Num>now-1)
     	return false;

	*Bs=bs[Num];

     return true;
}


long BurnUp::GetNumOutputData()
{
	return now;
}
/*
void polynom(double x, double a[], long n);

void polynom(double x, double a[], long n)
{
	long i;

     a[1]=1.0;
     for(i=2; i<=n; i++)
     	a[i]=a[i-1]*x;
}



long BurnUp::RunPolynomialRegression(long Flaming)
{
     long ndata;
   	long i, j, maxnum;
   	double maxval;
     double oldchisq, chisq;
     double tempaa[20]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

     maxval=0.0;
     maxnum=0;
     for(i=0; i<now; i++)
     {    if(bs[i].wdf>maxval)		// really just using weight loss %
          {	maxval=bs[i].wdf;
		     maxnum=i;
          }
     }
     ndata=now-maxnum;

     if(AllocRegressionData(ndata)==false)
     	return 0;

     j=-1;
     if(Flaming)
     {	for(i=maxnum; i<now; i++)
	     {    j++;
	         	x[j]=bs[i].time/60.0;
	          y[j]=bs[i].ff;  // for nonlinear stuff and .wdf
	     }
     }
     else
     {	for(i=maxnum; i<now; i++)
	     {    j++;
	         	x[j]=bs[i].time/60.0;
	          y[j]=bs[i].wdf;  // for nonlinear stuff and .wdf
	     }
     }
     for(i=0; i<ndata; i++)
     	sig[i]=1;///=maxval+0.00001;
	if(!Flaming)
	{    sig[ndata-1]=0.01;
		sig[0]=0.01;		// no weighted regression with intensity data
     }

     // find regression with minimum chi-squared, start with 4 parameters
     ma=4;
     LinearModel(ma, ndata, &chisq);
	memcpy(tempaa, aa, 4*sizeof(double));
   	oldchisq=chisq;
	for(i=5; i<20; i++)
     {	LinearModel(i, ndata, &chisq);
     	if(oldchisq-chisq>0.01)
          {	oldchisq=chisq;
			memcpy(tempaa, aa, i*sizeof(double));
               ma=i;
          }
          else
          	break;
     }
     memcpy(aa, tempaa, 20*sizeof(double));

	return ma;
}

long BurnUp::LinearModel(long ma, long ndata, double *Chisq)
{
     double chisq;

     vx=dmatrix(1, ma, 1, ma);
    	svdfit(x-1, y-1, sig-1, ndata, aa-1, ma, ux, vx, w, &chisq, (*polynom));
     *Chisq=chisq;
	if(vx)
	     GlobalFree_dmatrix(vx, 1, ma, 1, ma);
     vx=0;

     return ndata;
}

*/




