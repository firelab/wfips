/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: flammap4.cpp
* Desc: Functions to load the command file data into the actual variables
*        that FlamMap will use.
*
* Date: 1-4-08
*
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{**/

#include   "stdafx.h"
#include  <stdio.h>
#include  <string.h>
#include  <strings.h>

//From WindNinja
#define e_Rough_h   1
#define e_Rough_d   2
#define e_Roughness 4
#define e_Albedo    5
#define e_Bowen     6
#define e_Cg        7
#define e_Anthropog 8
#define e_Elev      9


#include  "FlamMap_DLL.h"
#include  "flammap3.h"
#include  "fsxw.hpp"

//#include "..\WindNinja2\CWindNinja2.h" 

#include "NN_Def.h" 

#define max(a,b)      (((a) > (b)) ? (a) : (b))

#define min(a,b)      (((a) < (b)) ? (a) : (b))
#define e_FtToMt 0.3048   /* Converts feet to meter = ( feet * e_FtToMt )  */

void  _WSConver (d_Wtr *aWS, char cr[]);
void  _WDSConver (d_Wnd *aWDS, char cr[]);
int    _ChkSetRes (float f_CmdFilRes, double d_lcpRes, int lcpRows, int lcpCols, double *ad, char cr_ErrMes[]);
int   _FuelModelMiss (float f);
//int _PutNinjaFile (CWindNinja2 *wn2, char FN[], char Type[]);
float GetRough (int r, int c, int i_Type, FlamMap *FM);
float  computeSurfPropForCell ( int i_Type, double canopyHeight, int canopyCover,
                              int fuelModel, double fuelBedDepth);

int LCD(int x,int y)//computs lowest common denominator
{
	int minVal = min(x, y);
	int d;
	for(d = 2; d < minVal; d++) 
	{
		if(((x%d)==0) && ((x%d)==0)) 
			break;
	}
	if(d == minVal) //no common denominators
	{
		return 1;
	}
	return d;
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: LoadInputFile
* Desc: Read in all switch from command file.
*       Check switches and their arguments, check for mandotory and
*        switch combinations. Check all embedded data in the command file
*        like fuel moist models, wind stream, weather stream
*       Load all data/info to the proper locations for FlamMap to run
*   In: cr_PthFN.........path and/or file name of input command file
*  Ret: 1 = OK, else the error number
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{**/
int FlamMap::LoadInputFile (char cr_PthFN[])
{
 	runState = Loading;
	//For reuse, clear critical data that may exist from previous FlamMap object runs...
	DeleteWindGrids();
int i;

   i = icf.InputFlamMap(cr_PthFN);          /* Load info from commnad file       */
   if ( i != 1 )                        /*  some basic checking done         */
    return i;                           /* Error - return error number       */

   i = icf.ValidateFlamMap();            /* Validate inputs, detailed err chk */
   if ( i != 1 )                        /* Return err num                    */
     return i;

/*............................................................................*/
/* now load all inputs and any default values for things not found in cmd fil */

/* NOTE: Wind/Weather Conditioning Data and WindNinja related data will get */
/*       get loaded later when FlamMap starts running                       */

   i = FlamMap::LoadCustomFuelsData();
   if ( i != 1 )                        /* Return err num                    */
     return i;

/* Set Spread Direction to North or Max - see function */
   FlamMap::SetSpreadDirection((double)icf.i_SDFN, (double)icf.i_SDFM);
   
   FlamMap::LoadFuelMoist();        /* Fuel Moist, File or Embeded data  */
  
   FlamMap::SetConstWind (icf.f_WinSpe, icf.f_WinDir); /* Wind Speed - Direction */

   FlamMap::LoadFoliarMoist(icf.i_FolMoi);          /* Foliar Moisture      */

   FlamMap::LoadCrownFireMethod(icf.cr_CroFirMet);  /* Crown Fire Method, Finney,Reih*/

   FlamMap::LoadNumProcessors (icf.i_NumPro);       /* Number of processores */

   if (icf.b_eastSet && icf.b_westSet && icf.b_northSet && icf.b_southSet) 
	     SetAnalysisArea(icf.f_analysisEast, icf.f_analysisWest, icf.f_analysisNorth, icf.f_analysisSouth);

/* Set Constant Moisture if we didn't have any weather input data, or weather stream data */
   if ( this->icf.iN_Wtr > 0 || this->icf.iN_RAWS > 0){
     printf("Error:  You are attempting to run fuel moisture conditioning instead running Constant Fuel Moisture\n");  //ALM
     this->ConstFuelMoistures(1);//ALM
   }
   else  
     this->ConstFuelMoistures(1);  /* Using Constant Moisture */
 
/*-----------------------------------------------------------------------------*/
   if(icf.i_useMemoryLCP == 0) {
	     CantAllocLCP = true;}
   if(icf.i_useMemoryOutputs == 0) {	   }

/*.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
/* Set Output Grid Layer                                                     */
   if ( icf.i_FL  ) FlamMap::SelectOutputLayer (FLAMELENGTH   , true);
   if ( icf.i_SP  ) FlamMap::SelectOutputLayer (SPREADRATE    , true);
   if ( icf.i_IN  ) FlamMap::SelectOutputLayer (INTENSITY     , true);
   if ( icf.i_HE  ) FlamMap::SelectOutputLayer (HEATAREA      , true);
   if ( icf.i_CR  ) FlamMap::SelectOutputLayer (CROWNSTATE    , true);
   if ( icf.i_SO  ) FlamMap::SelectOutputLayer (SOLARRADIATION, true);
   if ( icf.i_FU1 ) FlamMap::SelectOutputLayer (FUELMOISTURE1 , true);
   if ( icf.i_FU10) FlamMap::SelectOutputLayer (FUELMOISTURE10, true);
   if ( icf.i_MI  ) FlamMap::SelectOutputLayer (MIDFLAME      , true);
   if ( icf.i_HO  ) FlamMap::SelectOutputLayer (HORIZRATE     , true);
   if ( icf.i_MAD ) FlamMap::SelectOutputLayer (MAXSPREADDIR  , true);
   if ( icf.i_ELA ) FlamMap::SelectOutputLayer (ELLIPSEDIM_A  , true);
   if ( icf.i_ELB ) FlamMap::SelectOutputLayer (ELLIPSEDIM_B  , true);
   if ( icf.i_ELC ) FlamMap::SelectOutputLayer (ELLIPSEDIM_C  , true);
   if ( icf.i_MAS ) FlamMap::SelectOutputLayer (MAXSPOT       , true);
   if ( icf.i_MASDir)FlamMap::SelectOutputLayer(MAXSPOT  , true);
   if ( icf.i_MASDx )FlamMap::SelectOutputLayer(MAXSPOT  , true);
   if ( icf.i_FU100 )FlamMap::SelectOutputLayer(FUELMOISTURE100 , true);
   if ( icf.i_FU1k  )FlamMap::SelectOutputLayer(FUELMOISTURE1000, true);
   if ( icf.i_WDG   )FlamMap::SelectOutputLayer(WINDDIRGRID     , true);
   if ( icf.i_WSG   )FlamMap::SelectOutputLayer(WINDSPEEDGRID   , true);

   return 1;

}


/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: _ChekSetRes
* Desc: Check the requested Gridded Wind Resolution from the cmd file 
*       against the native lcp elevation resolution and assign the resolution
*       we need to send into WindNinja
*       We don't allow the requested cmd file res to be less than the lcp
*        because of how memory is allocate in the WindNinja class for pass
*        array values in and out. 
* Ret: 1 = ok,  0 err - see error message
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{**/
int  _ChkSetRes (float f_CmdFilRes, double d_lcpRes, int lcpRows, int lcpCols, double *ad, char cr_ErrMes[])
{
	double d; 
	strcpy (cr_ErrMes,""); 

	if ( d_lcpRes > e_GWR_Max )
	{  /* prevent this, else screws up our logic  */ 
		sprintf (cr_ErrMes,"Native LCP resolution %6.1f exceeds Gridded Wind Resolution Limit %.1f",d_lcpRes,e_GWR_Max);
		return 0; 
	}

	if ( f_CmdFilRes != ef_ICFInit )
	{        /* a res was in cmd file        */
		if ( (double)f_CmdFilRes < d_lcpRes )
		{
			sprintf (cr_ErrMes,"Specified Gridded Winds Resolution %4.1f is below native lcp %.1f", f_CmdFilRes, d_lcpRes);
			return 0; 
		}
		if ( f_CmdFilRes > e_GWR_Max )
		{
			sprintf (cr_ErrMes,"Resolution %4.1f exceeds limit of %.1f", f_CmdFilRes,e_GWR_Max);
			return 0; 
		}    
		//Now make adjustments to output resolution so that: 
		//1) output resolution is a multiple of the native lvp resolution
		// and has same extent as the lcp
		//2) Minimum of 3 output rows and output cols, else WindNinja fails
		/*int lcd = LCD(lcpRows, lcpCols);
		double tRes = d_lcpRes;
		if(lcd > 1)
		{
			while((tRes * lcd) <= f_CmdFilRes)
				tRes *= lcd;
		}
		//now make sure at least 3 rows and columns, else set to native lcp resolution as default!
		if((lcpRows * d_lcpRes) / tRes < 3.0 || (lcpCols * d_lcpRes) / tRes < 3.0)
		{
			tRes = d_lcpRes;
		}
		*ad = tRes; */
		*ad = f_CmdFilRes;
		return 1; 
	}

	if ( f_CmdFilRes == ef_ICFInit )    /* if no Resol set in cmd file        */  
		d = e_GWR_Def;                    /*  use default                       */
	if ( d < d_lcpRes )                 /* make sure not less than Elev lcp res */
		d = d_lcpRes; 
	*ad = d;
	return 1;    
}

/*-------------------------------------------------------------------------*/
/* Only used for test */
/*void  _ShowArray (CWindNinja *aWN, char cr_FN[], char cr[] )
{
int i, j, k, l_Val,c,row,col;
FILE *fh;

   k = aWN->GetOutncols() * aWN->GetOutnrows();
   fh = fopen (cr_FN,"w");
   if ( fh == NULL )
     return ;
     
   i = aWN->GetOutncols();
   fprintf (fh,"NCOLS %d \n", i );

   fprintf (fh,"NROWS %ld \n", aWN->GetOutnrows() );
   fprintf (fh,"XLLCORNER %15.6f \n", aWN->GetOutxllcorner() );
   fprintf (fh,"YLLCORNER %15.6f \n", aWN->GetOutyllcorner() );
   fprintf (fh,"CELLSIZE   %6.2f \n", aWN->GetOutdeltaxy() );
   fprintf (fh,"NODATA_VALUE %6.2f \n", aWN->GetOutnodata() );
   j = 0; 
			 c = 0; 

			for ( row = aWN->GetOutnrows()-1; row >= 0; row-- ) {
				for ( col = 0; col < aWN->GetOutncols(); col++ ) {
       if ( !_stricmp(cr,"ang"))
					  			l_Val = aWN->GetOutAng(row*aWN->GetOutncols()+col);
							else
          l_Val = aWN->GetOutSpd(row*aWN->GetOutncols()+col);
       fprintf (fh, "%4d ", l_Val );
      c++;
						if ( c == aWN->GetOutncols() ) {
        fprintf (fh,"\n");
        c = 0; }
     }
			}
   fclose (fh);
}
*/


/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: LoadNumProcessors
* Desc: Set the number of processors that FlamMap will use based on the
*        on the switch setting in the command file
*   In: i_NumPro....number of processors specifed in cmd file or 'init' value
*                    if no cmd file switch found 
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{**/
void FlamMap::LoadNumProcessors(int i_NumPro)
{
int l,SysPro;
//SYSTEM_INFO sysinf;
//
  //GetSystemInfo(&sysinf);               /* Get system info    */
  //SysPro = sysinf.dwNumberOfProcessors; /* number of processors on system  */
  SysPro = (int) sysconf( _SC_NPROCESSORS_ONLN );
  l = i_NumPro;                        /* from cmd file                  */
  if ( i_NumPro == ei_ICFInit )        /* If no switch was found in cmd file */
    l = SysPro;
  else if ( i_NumPro <= 0 || i_NumPro > SysPro ) 
    l = SysPro;
  
  FlamMap::SetMaxThreads(l); 

}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: LoadFoliarMoist
* Desc: Load the Foliar Moisture found in the command file, if none
*       then set the default value.
*   In: 
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{**/
void FlamMap::LoadFoliarMoist(int i_FolMoi)
{
  if ( i_FolMoi == ei_ICFInit )         /* none was set in cmd file */
    FlamMap::SetFoliarMoistureContent (e_FM_Def);
  else 
    FlamMap::SetFoliarMoistureContent (i_FolMoi);
}

bool FlamMap::AccelerationON()
{
	return AccelerationState;
}

void FlamMap::InitializeRosRed()
{
	for(int i=0; i<257; i++)
		redros[i]=1.0;
}


double FlamMap::GetRosRed(int fuel)
{
	if(redros[fuel-1]>0.0)
		return redros[fuel-1];
	else
		return 1.0;
}

void FlamMap::SetRosRed(int fuel, double rosred)
{
	redros[fuel-1]=fabs(rosred);
}

bool	FlamMap::EnableCrowning(int Crowning)
{
	if(Crowning>=0)
     	CrowningOK=(bool) Crowning;

	return CrowningOK;
}

bool FlamMap::EnableSpotting(int Spotting)
{
	if(Spotting>=0)
     	SpottingOK=(bool) Spotting;

	return SpottingOK;
}

int FlamMap::GetCrownFireCalculation()
{
	return CrownFireCalculation;
}


int FlamMap::SetCrownFireCalculation(int Type)
{
	CrownFireCalculation=Type;

	return CrownFireCalculation;
}

double FlamMap::GetDynamicDistRes()
{
	return DynamicDistanceResolution;
}


void FlamMap::SetDynamicDistRes(double input)
{
	DynamicDistanceResolution=input;
}

bool FlamMap::ConstantBackingSpreadRate(int Back)
{
	if(Back>=0)
     	ConstBack=(bool) Back;

	return ConstBack;
}


/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: LoadCrownFireMethod - Load Crown Fire Method
* Desc: This can be Finney or Reinhardt, 
*       Finney is used as default.
*   In: cr_CroFirMet....text argument as set in the command file
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{**/
void FlamMap::LoadCrownFireMethod (char cr_CroFirMet[])
{	
/* check if Reinhardt was set in cmd file, else we'll assume Finney was set or  */
/* the no switch was found and so Finney is used as default                     */
   if ( !strcasecmp (cr_CroFirMet, e_CFM_ScotRein) )
	  SetCrownFireCalculation ( 1 );   /* if scott reinhardt           */
   else
      SetCrownFireCalculation ( 0 );   /* else assume finney           */
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: LoadFuelMoist
* Desc: Fuel Mositures are 1, 10, 100 hour, Herb and Woody
*       Load from only one source:
*        1. Moisture File - use file name from switch in cmd file.
*        2. Embedded Fuel Model data in cmd file
* NOTE: The data and cmd file switchs should have all been previously
*        check see --> ICF::ChkFueMoi()
*       SO the only error that should happen here is not being able to
*        open in Moisture file if it's even used.
*  Ret: 1 OK, else error number, 
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{**/
void FlamMap::LoadFuelMoist ()
{
int i;

/* Load Fuel Moist File if we have a file name                           */
   if ( strcmp (icf.cr_FMF,"" )) {
     FlamMap::LoadFuelMoistureFile (icf.cr_FMF);
     return;  }

/* Look for the Default Model and load it to all Models .................... */
   for ( i = 0; i < icf.iN_FMD; i++ ) {
     if ( icf.a_FMD[i].i_Model != e_DefFulMod )
       continue;
     FlamMap::SetAllMoistures(icf.a_FMD[i].i_TL1,icf.a_FMD[i].i_TL10,
           icf.a_FMD[i].i_TL100, icf.a_FMD[i].i_TLLH,icf.a_FMD[i].i_TLLW);
   }

/* Load all other Models                                                     */
   for ( i = 0; i < icf.iN_FMD; i++ ) {
     if ( icf.a_FMD[i].i_Model == e_DefFulMod )  /* Skip the Default Model            */
       continue;
     FlamMap::SetInitialFuelMoistures(icf.a_FMD[i].i_Model,
                icf.a_FMD[i].i_TL1,icf.a_FMD[i].i_TL10,
                icf.a_FMD[i].i_TL100, icf.a_FMD[i].i_TLLH,
                icf.a_FMD[i].i_TLLW);
   }
}


#ifdef wwweere
/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name:
* Desc:
*   In:
*  Ret:
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{**/
int  FlamMap::LoadWindStream (d_ICF *a_ICF)
{
int i, i_Mth, iX;
int  StationNumber;

d_Wnd *aWDS;

  if ( icf.iN_Wnd <= 0 )                /* No Wind Data switches found    */
    return 0;                           /*  That's ok                        */

  FlamMap::FreeWindData(0);

  StationNumber = AllocWindData (0, icf.iN_Wnd);
  if ( StationNumber < 0 )
    return e_EMS_Aloc;

  aWDS = icf.a_Wnd;
  iX = -1;

/* Load data for each observation PLUS and then load an extra 13 mth using   */
/*  the last observation's data                                              */
  for ( i = 0; i <= icf.iN_Wnd; i++ ) {
    if ( i == icf.iN_Wnd )              /* done with obsrvation data         */
      i_Mth = 13;                       /* Set month to 13                   */
    else {
      iX++;                             /* each stored struct of obsvrd data */
      i_Mth = aWDS[iX].i_Mth;
     _WDSConver(&aWDS[iX],icf.cr_WiDU);}/* Do any needed unit conversion     */

    FlamMap::SetWindData(               /* Load into FlamMap class           */
               StationNumber,           /*        int StationNumber         */
               i,                       /*        int NumObs,               */
               i_Mth,                   /* month, int month,                */
               aWDS[iX].i_Day,          /*  (int) day,                      */
               aWDS[iX].i_Hr,           /*  (int) hhour,                    */
               aWDS[iX].f_Spd,          /*  (int) wss,                      */
               aWDS[iX].i_Dir,          /*  (int) wwwinddir,                */
               aWDS[iX].i_CloCov);      /*  (int) cloudcover)               */

  } /* for i */

  return 0;
}
#endif 


/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: _WDSConver
* Desc: do wind speed conversion if data comes in as metric
*
*   In:
*  Ret:
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{**/
void  _WDSConver (d_Wnd *aWDS, char cr[])
{
  if ( !strcasecmp (cr,"Metric")) /* if Metric                                  */
    aWDS->f_Spd *= 0.5402;     /* (0.62125/1.15) 10m wind kph to 20ft wind mph*/
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: LoadInputError
* Desc: Get the Command file error message.
*   In: i_Num.....error number that was returned when cmd file was loaded
*  Ret: address of the error message string
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{**/
char  *FlamMap::LoadInputError (int i_Num)
{
  return icf.ErrorMessage(i_Num);
}

/* *********************************************************************
* Name: LoadGridWindsWN2
* Desc: Load the gridded winds using WindNinja 2. 
*       This version of WindNinja now uses diurnal info which is now
*        in the FlamMaps inputs file. 
* Out: ai...error code
* Ret: 1 = OK, 0 = error, see ai for error code 
********************************************************************* */
int FlamMap::LoadGridWindsWN2 (int *ai)
{/*ALM
bool b; 
int  i,j,c,r, l_NumNorthRows,l_NumEastCols, wnRows, wnCols, lN_EMiss, xl ; 
float f, gwRes; 
double d, d_Elev, yExtent, d_ElevRes, d_resol, unitsConv, minValue, maxValue; 
double   wnyExtent, xExtent, d_OutRes, *dr_Elev, *dr_Rough, wnxExtent;  
c_NN NN;           /* Nearest Neighbor, for filling in grids 
// CWindNinja2 WN2;

   *ai = 0;                                /* Returning Error Code, set to none 
   if (_stricmp(this->icf.cr_GGW,e_GW_Yes)) /* Chk Grid Wind Switch in cmd file
      return 1;                            /*  if not yes then beat it        

// Stu - check this out, is this what we want to do 
/* Need diurnal and/or windspeed to run Windninja gridded winds 
   if ( !_stricmp (this->icf.s_WDI.cr_YesNo,e_GW_No)){ /* No Diurnal data 
     if (icf.WindSpeed() < 0)  {                    /* No Wind speed 
	      strcpy(icf.cr_GGW, e_GW_No); 
       return 1; }}

  this->runState = WindNinja;   /* Let FlamMap know we're doing ninja 

/*----------------------------------------------------------------------
   unitsConv = 1.0;
   if ( FlamMap::GetTheme_Units (E_DATA) != 0)
      unitsConv = 0.3048; //convert feet to meters
   d_ElevRes = FlamMap::GetCellResolutionX(); 
   
   gwRes = FlamMap::icf.GridWindResolution();
   yExtent = d_ElevRes * FlamMap::GetNumNorth();
   l_NumNorthRows = GetNumNorth(); 
   wnRows = yExtent / gwRes;
   wnyExtent = wnRows * gwRes;
 
   while(wnyExtent < yExtent) {
		   wnRows++;
		   wnyExtent = wnRows * gwRes; }
  
   l_NumNorthRows = wnyExtent / d_ElevRes;
   while ( l_NumNorthRows * d_ElevRes < wnyExtent)
	     l_NumNorthRows++;
       
   xExtent = d_ElevRes * FlamMap::GetNumEast();
   wnCols = xExtent / gwRes;
   wnxExtent = wnCols * gwRes;
   while ( wnxExtent < xExtent ){
	    	wnCols++;
	    	wnxExtent = wnCols * gwRes;  }
  
   l_NumEastCols = wnxExtent / d_ElevRes;
   while ( l_NumEastCols * d_ElevRes < wnxExtent )
	     l_NumEastCols++;

/* Setup WindNinja elev & rough height grids 
   WN2.SetHeadData( l_NumEastCols, l_NumNorthRows,  /* size of grid 
                    FlamMap::GetWestUtm(), (FlamMap::GetNorthUtm() - wnyExtent),
                    FlamMap::GetCellResolutionX(), 
                    NODATA_VAL,                     /* Miss data value 
                    NODATA_VAL);                    /* Init grid with this 
 
  dr_Elev = new double [l_NumNorthRows * l_NumEastCols]; 
  dr_Rough = new double [l_NumNorthRows * l_NumEastCols]; 
    
  minValue = 99999.9;  /* set for finding min and max 
  maxValue = -1.0;    
  lN_EMiss = 0;      
  xl = 0; 

/* Get elevations from FlamMap lcp 
  for ( r = 0;  r < l_NumNorthRows; r++ ) {
    for ( c = 0; c < l_NumEastCols; c++ ) {
       d_Elev = FlamMap::GetLayerValueByCell(E_DATA, c, r);	/* returns 1- if out of grid 
       if ( d_Elev == (double) NODATA_VAL || d_Elev < 0.0 ) {    
         dr_Elev[xl++] = NODATA_VAL; 
         lN_EMiss++; 
         continue; }             
 
       d_Elev = d_Elev * unitsConv; /* want meters 
       dr_Elev[xl++] = d_Elev;       
 
       if ( d_Elev < minValue )  /* Find Min Max elevs 
         minValue = d_Elev;
       if ( d_Elev > maxValue )
         maxValue = d_Elev;  }}

  WN2.Set_ElevMinMax (minValue,maxValue); 
 
/* -------------------------------------------------------------------
/* 7 rough height arrarys need to be load, checked for missing values 
/*  and then loaded to windninja, here we do array at a time 
  int ir[] = { e_Rough_h, e_Rough_d, e_Roughness, e_Albedo, e_Bowen, e_Cg, e_Anthropog, -1 }; 

  for ( i = 0; i < 100; i++ ) 
  { /* Do each of the rough types, see above 
	  if ( ir[i] == -1 )          /* All done 
		  break; 
	  xl = -1;
	  for ( r = 0;  r < l_NumNorthRows; r++ ) 
	  {
		  for ( c = 0; c < l_NumEastCols; c++ ) 
		  {
			  xl++;
			  //if ( dr_Elev[xl] == NODATA_VAL )  /* look at elev for corspnd cell 
				//  dr_Rough[xl] = NODATA_VAL;      /* if miss also set rough to mis 
			  //else                              /* go determin which kind rough  
				dr_Rough[xl] = GetRough (r,c,ir[i],this);  /* and return it 
		  }
	  } 

	  /* Check and fill in any missing values 
	  NN.Init(l_NumNorthRows, l_NumEastCols,dr_Rough,NODATA_VAL); 
	  /*if ( !NN.Fill_Mngr())
	  {
		  sprintf (this->icf.cr_ErrExt, "%s - Error while filling missing elevations", NN.cr_Message);
		  return e_EMS_GWWN;
	  }*/
	  //char roughFile[256];
	  //sprintf(roughFile, "F:\\WFDSS\\STFB\\282960\\wnRough_%ld.asc", i);
	  //NN.DumpTest(this->GetWestUtm(), this->GetSouthUtm(), this->GetCellResolutionX(), roughFile);
	  /* Give data to Windninja 
	  WN2.LoadGrid (dr_Rough,ir[i],l_NumNorthRows, l_NumEastCols); 

  } /* for i - do each type of rough array 


/* After all Rough arrays are done, fill elevation and give to windninja 
	  NN.Init(l_NumNorthRows, l_NumEastCols,dr_Elev,NODATA_VAL); 
   if ( !NN.Fill_Mngr()){
	     sprintf (this->icf.cr_ErrExt, "%s - Error while filling missing elevations", NN.cr_Message);
      return e_EMS_GWWN; }
	 //NN.DumpTest(this->GetWestUtm(), this->GetSouthUtm(), this->GetCellResolutionX(), "F:\\WFDSS\\STFB\\282960\\wnElev.asc");

   WN2.LoadGrid (dr_Elev,e_Elev,l_NumNorthRows, l_NumEastCols); 
  
   delete[] dr_Elev;                   /* won't need these anymore 
   delete[] dr_Rough; 

   WN2.Set_SpeedDirection(icf.WindSpeed(),icf.WindDirection());

/* Input/Output Wind Heights............................
   f = FlamMap::icf.GridWindHeight(); 
   if ( f == ef_ICFInit )                /* Not set in cmd file           
     f = e_GWH_Def;                      /* defualt gridded wind height    
   f = f * e_FtToMt; 		
   WN2.Set_InOutWindHeight (f,f);        /* set both the same 

   d_WDI *w = &this->icf.s_WDI; /* to verbose, s_WDI has inputs file diurnal data 
   if ( !_stricmp (w->cr_YesNo,e_GW_Yes) ){   /* if have diurnal data to use 
     WN2.Diurnal(true);
	    WN2.Diurnal_Date (w->i_Day, w->i_Mth, w->i_Year);	// (d, m, y)
	    WN2.Diurnal_Time (w->i_Sec, w->i_Min, w->i_Hour, w->i_TimeZone);	// (s, m, h, timezone)
	    WN2.Diurnal_TempCloud (w->f_AirTemp,w->f_CloudCov);
     WN2.Diurnal_LatLong ((double)GetLatitude(), (double) w->f_Longitude); }
   else
     WN2.Diurnal(false); 
   
// STU - Check out _ChkSetRes(), see if that looks correct to you, I think
//  you set that up for the last version of windninja 
   f = FlamMap::icf.GridWindResolution();  
   if ( !_ChkSetRes (f, d_ElevRes, l_NumNorthRows, l_NumEastCols, 
                     &d_OutRes, this->icf.cr_ErrExt)) {
      *ai = e_EMS_GWR;    /* Grid Wind Resol error code number 
      return 0; }
	  WN2.Set_MeshResolution (d_OutRes);
	  
   WN2.Set_NumVertLayers(20);  /* ninja team said to use 20 
   
   i = FlamMap::GetMaxThreads(); 
   WN2.Set_NumberCPUs(i);

   WN2.Set_OutFiles (false);    /* Turn off Windninja's output 
   
   WN2.Compute_Domain_Height();   

   if ( !WN2.Check_Errors (this->icf.cr_ErrExt) ){ /* chk inputs bfor run 
	     *ai = e_EMS_GWWN; 
	     return 0; }
 
/* Run Wind Simulation 
   if ( !WN2.Simulate_Wind(this->icf.cr_ErrExt)){
      *ai = e_EMS_GWWN; 
	     return 0; }

/*..................................................... 
/* Windninja loaded 2 grid classes - angle and velocity  
double A,V;
int Rows, Cols; 

  Rows = WN2.Get_AngleGrid_Row();
  Cols = WN2.Get_AngleGrid_Col(); 

// Test dumps output grids windninja created.
// These calls use windninja grid functions to write output file
//   WN2.WriteAngleFile    ("c:\\LarryF\\WN2-Test\\wn-ang.asc");
//   WN2.WriteVelocityFile ("c:\\LarryF\\Vel-out.txt");

// Test dump windninja grid pulling values from it. 
//   _PutNinjaFile ( &WN2, "c:\\LarryF\\ang.asc", "Angle" );


/* in FlamMap class set grid info and allocate memory for WN outputs 
	 AllocWindGrids (Rows, Cols, 
                  WN2.Get_AngleGrid_CellSize(), 
			               WN2.Get_AngleGrid_xllCorner(), 
                  WN2.Get_AngleGrid_yllCorner());

/* Get WN wind angle & velocity into FlamMap class 
	// int wnRow = Rows;        /* ninja grids keep 1st row last 
	 for (r = 0; r < Rows; r++)
	 { 
		// wnRow--; 
		 for (c = 0; c < Cols; c++)
		 {
			 A = WN2.Get_Angle (r,c);
			 V = WN2.Get_Velocity (r,c);
			 SetWindGridValues(r, c, (float)A, (float)V); 
		 }
	 }    

   for ( i = 0; i < 64; i++)
     SetThreadProgress(i, 0.0);
*/
  return 1; 
}
/* -------------------------------------------------------------
* Name: _PutNinjaFile
* Desc: Used for testing.  
*       Dump out the angle or velocity grid that comes 
*       from windninja. 
*   In: FN...file name
*       Type..."Angle" or "Velocity"
* ---------------------------------------------------------------*/
//int _PutNinjaFile (CWindNinja2 *wn2, char FN[], char Type[])
/*{ALM
FILE *fh;
int l_row, l_col,r,c; 
double d,d_xll, d_yll, d_CS;  
   fh = fopen (FN,"w");
   if ( fh == NULL )
     return 0; 

/* Can use these for angle or vel, both are same sizes 
    l_row = wn2->Get_AngleGrid_Row();
    l_col = wn2->Get_AngleGrid_Col(); 
 
		  	for(r = 0; r < l_row; r++ ){ 
       fprintf (fh,"Row: %d \n", r); 
		     for (c = 0; c < l_col; c++){
		       if ( !_stricmp (Type, "Angle" ))
           d = wn2->Get_Angle (r,c);       /* Do one or the other 
         else 
           d = wn2->Get_Velocity (r,c);
        fprintf (fh, "%5.1f ", d);
      }
      fprintf (fh,"\n");  
   }    
   fclose (fh); 
   return 1; 
}*/
 	
/* ***********************************************************************
* Name: GetRough
* Desc: Set WN roughness height values  
*  Ret: 1 ok set,
*       0 = all need FlamMap landscape values were missing 
*********************************************************************** */
float GetRough (int r, int c, int i_Type, FlamMap *FM)
{
int i,j; 
bool b; 
float f_Conv, f_CanHgt, f_CanCov, f_FueMod, f_FueBed; 
float f;

/* Canopy Height..........................................................*/
  i = FM->GetTheme_Units (H_DATA);      // Canopy Height units ?       //
/*  if ( i == 1 || i == 3 )               // if Canopy Height is in meters  //
				f_Conv =  e_FtToMt / 10.0 ;          // See Note-1 above   //
  else                                
    f_Conv = 1.0 / 10.0; */ 

   if ( i == 1 || i == 3 )               // if Canopy Height is in meters  //
    f_Conv = 1.0 / 10.0;  
  else                                
				f_Conv =  e_FtToMt / 10.0 ;          // See Note-1 above   //
 
  f_CanHgt = FM->GetLayerValueByCell(H_DATA, c, r);   // Canopy Height      //
  if ( f_CanHgt != NODATA_VAL )
    f_CanHgt = f_CanHgt * f_Conv;    /* Get to Meters */

/* Canopy Cover ........................................................*/
   f_CanCov = FM->GetLayerValueByCell(C_DATA, c, r);   /* % Canopy Cover    */
  	if ( f_CanCov != NODATA_VAL ) {
     if ( !FM->GetTheme_Units (C_DATA) )      /*if Canopy Cover is in Classes   */
				   f_CanCov = FM->ConvertUnits(f_CanCov, 0, 1, C_DATA); /* Class --> percent */
			}
	 
/* Fuel Model .......................................................*/
   f_FueMod = FM->GetLayerValueByCell(F_DATA, c, r);   /* Fuel Model        */

   //if ( f_CanHgt == NODATA_VAL && f_CanCov == NODATA_VAL && f_FueMod == NODATA_VAL )
   //  return 1.0; 
				
/* Fuel Bed Depth ..................................................*/
    f_FueBed = FM->GetFuelDepth((int)f_FueMod) * e_FtToMt;   /* Always comes back as feet, so convert */
     
/* Call Jason's WN to set all rough values into WN Grids,  */
/*  would only get an error back if send in a bad unit     */
   f = computeSurfPropForCell( i_Type,              /* Rough Value to get */
                             (double) f_CanHgt,    /* meters */ 
                             (int) f_CanCov,       /* percent 1 -> 100 */     
                             (int) f_FueMod, 
                             (double) f_FueBed);  /* meters */
   return f; 
}

/* **************************************************************************
*
* Name: computeSurfPropForCell
*
* This function computes surface properties for WindNinja, based on information
* available in a FARSITE .lcp file.
* If any of the input parameters (canopyHeight, canopyCover, fuelModel, fuelBedDepth)
* are not available (NO_DATA), enter a value less than 0.
* If sufficient data is not passed to this function to determine the surface properties,
* default values (rangeland) are used
*
* In:
*    canopyHeight = meters
*    canopyCover  = percent 0 -> 100 
*    fuelModel
*    fuelBedDepth = meters
*
*    i_Type: e_Rough_h, e_Rough_d, e_Roughness, e_Albedo
*            e_Bowen, e_Cg, e_Anthropog
*
* Ret: return the requested i_Type
*
*************************************************************************** */
float  computeSurfPropForCell ( int i_Type, double canopyHeight, int canopyCover,
                              int fuelModel, double fuelBedDepth)                              
{
double f = NODATA_VAL; 
double Rough_h, Rough_d, Roughness, Albedo;
double Bowen, Cg, Anthropog  ;

  if ( canopyHeight < 0 && canopyCover < 0 &&
        fuelModel    < 0 && fuelBedDepth < 0 )
     return NODATA_VAL;

// Go through logic of determining surface properties, depending on what data is available at this cell

  if (canopyCover >= 5 && canopyHeight > 0 )  {
    Rough_h   = canopyHeight;
    Rough_d   = canopyHeight * 0.63;
    Roughness = canopyHeight * 0.13;
    Albedo    = 0.1;   //assuming forest land cover for heat transfer parameters
    Bowen     = 1.0;
    Cg        = 0.15;
    Anthropog     = 0.0; }
			
// See if it's an unburnable Fuel Model
  else if ( fuelModel==90 || fuelModel==91 || fuelModel==92 ||
       fuelModel==93 || fuelModel==98)   {
     switch ( fuelModel )   {
       case 90:    // Barren
         Rough_h   = 0.00230769;
         Rough_d   = 0.00230769 * 0.63;
         Roughness   = 0.00230769 * 0.13;
         Albedo   = 0.3;
         Bowen   = 1.0;
         Cg   = 0.15;
         Anthropog     = 0.0;
         break;
       case 91:    // Urban Roughness
         Rough_h   = 5.0;
         Rough_d   = 5.0 * 0.63;
         Roughness   = 5.0 * 0.13;
         Albedo   = 0.18;
         Bowen   = 1.5;
         Cg   = 0.25;
         Anthropog     = 0.0;
         break;
       case 92:        // Snow Ice
         Rough_h   = 0.00076923;
         Rough_d   = 0.00076923 * 0.63;
         Roughness   = 0.00076923 * 0.13;
         Albedo   = 0.7;
         Bowen   = 0.5;
         Cg   = 0.15;
         Anthropog     = 0.0;
         break;
       case 93:        // Agriculture
         Rough_h   = 1.0;
         Rough_d   = 1.0 * 0.63;
         Roughness   = 1.0 * 0.13;
         Albedo   = 0.15;
         Bowen   = 1.0;
         Cg   = 0.15;
         Anthropog     = 0.0;
         break;
       case 98:        // Water
         Rough_h   = 0.00153846;
         Rough_d   = 0.00153846 * 0.63;
         Roughness   = 0.00153846 * 0.13;
         Albedo   = 0.1;
         Bowen   = 0.0;
         Cg   = 1.0;
         Anthropog     = 0.0;
         break;
      } /* switch */
    } /* if */

// just use fuel bed depth
  else if ( fuelBedDepth > 0.0 )  {
     Rough_h   = fuelBedDepth;
     Rough_d   = fuelBedDepth * 0.63;
     Roughness   = fuelBedDepth * 0.13;
     Albedo   = 0.25;  //use rangeland values for heat flux parameters
     Bowen   = 1.0;
     Cg   = 0.15;
     Anthropog     = 0.0;  }

// if there is a canopy height (no fuel model though)
  else if ( canopyHeight > 0.0) {
     Rough_h   = canopyHeight;
     Rough_d   = canopyHeight * 0.63;
     Roughness   = canopyHeight * 0.13;
     Albedo   = 0.1;      // assume forest land for heat flux parameters
     Bowen   = 1.0;
     Cg   = 0.15;
     Anthropog     = 0.0;  }

// If we make it to here, we'll just choose parameters based on rangeland...
   else {
     Rough_h   = 0.384615;
     Rough_d   = 0.384615 * 0.63;
     Roughness   = 0.384615 * 0.13;
     Albedo   = 0.25;
     Bowen   = 1.0;
     Cg   = 0.15;
     Anthropog     = 0.0; }

   if      ( i_Type == e_Rough_h   ) f =  Rough_h;
   else if ( i_Type ==  e_Rough_d   ) f =  Rough_d;
   else if ( i_Type ==  e_Roughness ) f =  Roughness;
   else if ( i_Type ==  e_Albedo    ) f =  Albedo;
   else if ( i_Type ==  e_Bowen     ) f =  Bowen;
   else if ( i_Type ==  e_Cg        ) f =  Cg;
   else if ( i_Type ==  e_Anthropog ) f =  Anthropog;
   else f = NODATA_VAL; /* shouldn't happen */
   return f;
}

/* .-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
float **FlamMap::Get_windDirGrid () { return windDirGrid; } 
float **FlamMap::Get_windSpdGrid () { return windSpeedGrid; } 
bool  FlamMap::has_GriddedWind () { return hasGriddedWinds; }
bool  FlamMap::GridWindRowCol (int *Rows, int *Cols) 
{
  *Rows = nWindRows;
  *Cols = nWindCols; 
  return hasGriddedWinds;  /* true - false if there are gridded winds in use */ 
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: FSPRo_ConditionInputs
* Desc: This function is used by FSPRo to load Moisture Conditioning
*        inputs from an FSPRo inputs file. 
*       This is done to take advantage of existing FlamMap functions.
*   In: cr_PthFN.........path and/or file name of input command file
*  Ret: 1 = OK, else the error number
*       < 0  error number 
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{**/
int FlamMap::FSPRo_ConditionInputs (char cr_PthFN[])
{
int i;  
   i = icf.FSPRO_ConditionInputs (cr_PthFN); /* Read data from file into the icf object  */
   if ( i == 0 )              /* No Conditioning inputs found or */
     return 0;                /* Switch set to NO for no conditioning */
   if ( i < 0 )               /* error occured reading in data */
     return i;                /* return error code */

//   FlamMap::LoadWeatherStream (&this->icf);    /* Move from ICF condition data into FlamMap */
//   FlamMap::LoadWindStream (&this->icf); 
//   FlamMap::LoadCondition(&this->icf); 
   return 1;                   /* Cond data found and load into FlamMap OK */
}

/* *********************************************************************
* Name: LoadGridWindsFromFiles
* Desc: Load the gridded winds from ASCII grids. 
* Out: ai...error code
* Ret: 1 = OK, 0 = error, see ai for error code 
********************************************************************* */
int    FlamMap::LoadGridWindsFromFiles (int *ai)
{
	*ai = 0;
	FILE *dirFile = fopen(icf.cr_GWDF, "rt");
	if(!dirFile)
	{
		*ai = e_EMS_GWDF;
	}
	FILE *spdFile = fopen(icf.cr_GWSF, "rt");
	if(!spdFile)
	{
		*ai = e_EMS_GWSF;
	}
	int dcols = 0, drows = 0, scols = 0, srows = 0;
	double dllx = 0.0, dlly = 0.0, dres = 0.0, dNodata, 
		sllx = 0.0, slly = 0.0, sres = 0.0, sNodata;
	char krap[64];
	if(*ai == 0 && fscanf(dirFile, "%s %ld", krap, &dcols) != 2)
		*ai = e_EMS_GWDFREAD;
	if(*ai == 0 && fscanf(dirFile, "%s %ld", krap, &drows) != 2)
		*ai = e_EMS_GWDFREAD;
	if(*ai == 0 && fscanf(dirFile, "%s %lf", krap, &dllx) != 2)
		*ai = e_EMS_GWDFREAD;
	if(*ai == 0 && fscanf(dirFile, "%s %lf", krap, &dlly) != 2)
		*ai = e_EMS_GWDFREAD;
	if(*ai == 0 && fscanf(dirFile, "%s %lf", krap, &dres) != 2)
		*ai = e_EMS_GWDFREAD;
	if(*ai == 0 && fscanf(dirFile, "%s %lf", krap, &dNodata) != 2)
		*ai = e_EMS_GWDFREAD;
	if(*ai == 0 && fscanf(spdFile, "%s %ld", krap, &scols) != 2)
		*ai = e_EMS_GWSFREAD;
	if(*ai == 0 && fscanf(spdFile, "%s %ld", krap, &srows) != 2)
		*ai = e_EMS_GWSFREAD;
	if(*ai == 0 && fscanf(spdFile, "%s %lf", krap, &sllx) != 2)
		*ai = e_EMS_GWSFREAD;
	if(*ai == 0 && fscanf(spdFile, "%s %lf", krap, &slly) != 2)
		*ai = e_EMS_GWSFREAD;
	if(*ai == 0 && fscanf(spdFile, "%s %lf", krap, &sres) != 2)
		*ai = e_EMS_GWSFREAD;
	if(*ai == 0 && fscanf(spdFile, "%s %lf", krap, &sNodata) != 2)
		*ai = e_EMS_GWSFREAD;
	if(*ai == 0)
	{//check for same extents (must match exactly)
		if(dcols != scols || drows != srows || dllx != sllx || dlly != slly || dres != sres)
			*ai = e_EMS_GWEXTENT;
	}
	if(*ai == 0)
	{//check for complete landscape overlap
		double tol = this->GetCellResolutionX() / 2.0;
		if(dllx > this->GetWestUtm() + tol || dllx + dcols * dres < this->GetEastUtm() - tol
			|| dlly > this->GetSouthUtm() + tol || dlly + drows * dres < this->GetNorthUtm() - tol)
			*ai = e_EMS_GWLCP;
	}
	if(*ai == 0)
	{//everything looks ok, let's create storage and read the data
		AllocWindGrids (drows, dcols, dres, dllx, dlly);

		 /* Get  wind angle & velocity into FlamMap class */
		// int wnRow = Rows;        /* ninja grids keep 1st row last */
		double d, s;
		 for (int r = 0; r < drows; r++)
		 {
			 for (int c = 0; c < dcols; c++)
			 {
				 if(*ai == 0 && fscanf(dirFile, "%lf ", &d) != 1)
					 *ai = e_EMS_GWDFREAD;
				 if(*ai == 0 && fscanf(spdFile, "%lf ", &s) != 1)
					 *ai = e_EMS_GWSFREAD;
				 SetWindGridValues(r, c, (float)d, (float)s); 
			 }
		 }  
	}
	if(dirFile)
		fclose(dirFile);
	if(spdFile)
		fclose(spdFile);
	if(*ai == 0)
		return 1;
	return *ai;
}

/****************************************************************************************/
int FlamMap::LoadCustomFuelsData()
{
/* Load Fuel Moist File if we have a file name                           */
   if ( strcmp (icf.cr_CFF,"" )) 
   {
	   return FlamMap::LoadCustomFuelFile(icf.cr_CFF);
   }
   //maybe it was embedded in inputs file...
   if(icf.iN_CustomFuels > 0 && icf.a_customFuels != NULL)
   {
	   for(int f = 0; f < icf.iN_CustomFuels; f++)
	   {
		   NewFuel newfuel;
		   newfuel.number = icf.a_customFuels[f].i_Model;
		   newfuel.h1 = icf.a_customFuels[f].f_h1;
		   newfuel.h10 = icf.a_customFuels[f].f_h10;
		   newfuel.h100 = icf.a_customFuels[f].f_h100;
		   newfuel.lh = icf.a_customFuels[f].f_lh;
		   newfuel.lw = icf.a_customFuels[f].f_lw;
		   newfuel.depth = icf.a_customFuels[f].f_depth;
		   newfuel.xmext = icf.a_customFuels[f].f_xmext;
		   newfuel.heatd = icf.a_customFuels[f].f_heatd;
		   newfuel.heatl = icf.a_customFuels[f].f_heatl;
		   newfuel.dynamic = 1;
		   if(icf.a_customFuels[f].dynamic[0] != 'd' || icf.a_customFuels[f].dynamic[0] != 'D' )
			   newfuel.dynamic = 0;
		   strncpy(newfuel.code, icf.a_customFuels[f].cr_code, 7);
		   newfuel.sav1 = icf.a_customFuels[f].f_sl;
		   newfuel.savlh = icf.a_customFuels[f].f_slh;
		   newfuel.savlw = icf.a_customFuels[f].f_slw;
		   strncpy(newfuel.desc, icf.a_customFuels[f].cr_comment, 255);
		   /*if(icf.cr_CustFuelUnits[0] == 'M' || icf.cr_CustFuelUnits[0] == 'm')//inputs were metric
		   {
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
		   }*/
          SetNewFuel(&newfuel);

	   }
   }
   return 1;
}






