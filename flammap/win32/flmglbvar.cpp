
//******************************************************************************
//	FSGLBVAR.CPP		Global Variable and Functions for FARSITE
//
//
//
//					Copyright 1994, 1995
//					Mark A. Finney, Systems for Environmental Management
//******************************************************************************

#include "flmglbvar.h"
#include <io.h>
#include <math.h>
#include <direct.h>

//static TWindow * FarsiteWindow=0;	// globally declared pointer to the FarsiteWindow
//static TFarsiteWindow * FarsiteWindow=0;// globally declared pointer to the FarsiteWindow
//static TThreeDWindow *ThreeDWindow=0;
static bool InputChanged=true;          // records new input mode changes
static HBITMAP hBitmap1=0;              // bitmaps for landscape window
static HBITMAP hBitmap2=0;			// 1 is unchanged for resetting, 2 is workmap
static HBITMAP hBitmap3=0;			// 3 is temp fuel changes && barriers
static double xrat, yrat;			// ratio of original window to current
static double MaxX, MaxY;			// max coords of farsite display window
static double tint;					// smallest interval of x and y extent of visible landscape
static double FullTint;				// smallest interval of x & y for whole landscape
static long MaxWindowDim=8000;		// maximum farsite window dimension
static bool ViewPort_Changed=false;	//
static bool ViewPort_Status=false;
static double ViewPortNorth=0;
static double ViewPortSouth=0;
static double ViewPortEast=0;
static double ViewPortWest=0;
static long NumViewNorth;
static long NumViewEast;
static long AdjustIgSpreadRates=0;       // can use initial spread rate dialog for ignitions
static int FuelsAltered=0;
static FuelConversions fuelconversion;
static bool NEED_CUST_MODELS=false;	// custom fuel models
static bool HAVE_CUST_MODELS=false;
static bool NEED_CONV_MODELS=false;     // fuel model conversions
static bool HAVE_CONV_MODELS=false;
static FuelModels FuelMod;
static long CanSetRastRes=true;		// flag to allow setting raster resolution
static long RasterVPExtent=false;
static double RasterCellResolutionX;
static double RasterCellResolutionY;
static double SimulationDuration=0.0;
static bool CondPeriod=false;
static bool EnvtChanged[4][5]={{false, false, false, false, false},
						 {false, false, false, false, false},
						 {false, false, false, false, false},
						 {false, false, false, false, false}};

static char RasterArrivalTime[256];
static char RasterFireIntensity[256];
static char RasterFlameLength[256];
static char RasterSpreadRate[256];
static char RasterHeatPerArea[256];
static char RasterCrownFire[256];
static char RasterFireDirection[256];
static char RasterReactionIntensity[256];

static bool rast_arrivaltime=false;
static bool rast_fireintensity=false;
static bool rast_spreadrate=false;
static bool rast_flamelength=false;
static bool rast_heatperarea=false;
static bool rast_crownfire=false;
static bool rast_firedirection=false;
static bool rast_reactionintensity=false;
static long ZoomReplayDelay=0;

static char VectFName[256];
static char RastFName[256];
static char ShapeFileName[256]="";
static long ShapeVisibleStepsOnly=1;
static long ShapePolygonsNotLines=1;
static long ShapeBarriersSeparately=0;
static char ThreeDRasterFile[256];
static long ThreeDRasterHi=0;
static long ThreeDRasterLo=0;
static bool View3DVectorFile=false;
static char ThreeDVectorFile[256];
static COLORREF VectorColor=RGB(255, 255, 255);
static long VectorFileType=0;           // 0=vis, 1=GRASS, 2-4=ARC
static long VisibleThemeNumber=3;		// fuels default visible theme
static long LandscapeInputMode=NULLLOCATION;
static bool CrownDensityLinkToCC=false;    // crown bulk density is a function of canopy cover

static bool RastMake=false;
static bool VectMake=false;
static bool ShapeMake=false;
static bool VISONLY=false;        // visible time step only for vector file
static bool TOPOSHADING=true;	    // topographic shading from display dialog
static long RastFormat=1;
static long VectFormat=0;
static char VisPerim[256]="";//"c:\\visperim.vsp";
static FILE *VP=0;
static WindData *wddt[5]={0,0,0,0,0};
static WeatherData *wtrdt[5]={0,0,0,0,0};
static long MaxWeatherObs[5]={0,0,0,0,0};
static long MaxWindObs[5]={0,0,0,0,0};
static long DurationReset=0;			// force DurationReset at ReStart
static long IgnitionReset=0;
static long RotateIgnitions=0;
static long ShowVectsAsGrown=1;
static bool InactiveEnclaves=true;
static char CurrentPath[256]="";
static bool UseCurrentPath=false;

static long LightSourceDirection=225;
static long OverlayGridInterval=0;
static long OverlayGridColor=RGB(255, 255,255);

//------------------ Display Fire Characteristics on 2-D and 3-D Landscapes
static long FireOutputParam=4;
static double FireMaxChx=1000.0;
static double FireMinChx=0.0;
static long FireNumInterval=16;
//------------------------------------------------------------------------------

static streamdata FirstMonth[5];
static streamdata FirstDay[5];
static streamdata FirstHour[5];
static streamdata LastMonth[5];
static streamdata LastDay[5];
static streamdata LastHour[5];

bool PreCalcFuelMoistures=true;
long MoistCalcInterval[4][5]={{60, 200, 10, 45, 15},           // 1hr
						{60, 200, 10, 45, 15},          // 10hr
						{120, 400, 10, 45, 20},          // 100hr
                              {240, 400, 15, 45, 30}};		   // 1000hr

/*
static int FirstMonth[5]={0,0,0,0,0};
static int FirstDay[5]={0,0,0,0,0};
static int FirstHour[5]={-1,-1,-1,-1,-1};
static int LastMonth[5]={0,0,0,0,0};
static int LastDay[5]={0,0,0,0,0};
static int LastHour[5]={-1,-1,-1,-1,-1};
*/

static long conditmonth=0, conditday=0;
static long startmonth=0, startday=0, starthour=0, startmin=0, startdate=0;		//INITIAL CONDITIONS AT STARTPOINT
static long endmonth=0, endday=0, endhour=0, endmin=0, enddate=0;
static long OldFilePosition=0;	//__thread 
static headdata2 Header2;
static headdata Header;
static oldheaddata OldHeader;
//static celldata __thread cell;
//static crowndata __thread cfuel;
//static grounddata __thread gfuel;
static size_t headsize=sizeof(Header);
static FILE *landfile=0;
static char LandFName[256]="";
static char LCPJustMade[256]="";
static StationGrid grid;
static long NumWeatherStations=0;
static long NumWindStations=0;
static double NorthGridOffset=0.0;
static double EastGridOffset=0.0;

static short *landscape=0;

static LOGPALETTE* FarPal=0;
static HPALETTE hLogPal=0, hMemPal=0, PrevPalette1=0, PrevPalette2=0;
static bool ValidPalette=false;
static bool XORArrows=true;                  // draw XORArrows for wind vetors

static CanopyCharacteristics CanopyChx;//={15.0, 4.0, 0.20, 20.0, 100, 2, 1}; //__thread
//static double __thread DuffLoad=0.0;
//static double __thread WoodyModel=0;

//static VectorStorage StoreVect[5];
static long NumVectorThemes=0;

NewFuel newfuels[257];									// custom and standard fuel models

static AtmosphereGrid *AtmGrid=0;			// pointer to AtmGrid;

FuelModels::FuelModels()					// constructor for fuel models
{
	FCOLOR[0]=RGB(50,50,50);
	FCOLOR[1]=RGB(0,180,0);
	FCOLOR[2]=RGB(80,80,0);
	FCOLOR[3]=RGB(140,140,0);
	FCOLOR[4]=RGB(200,0,0);
	FCOLOR[5]=RGB(160,0,0);
	FCOLOR[6]=RGB(120,0,0);
	FCOLOR[7]=RGB(80,0,0);
	FCOLOR[8]=RGB(0,0,80);
	FCOLOR[9]=RGB(0,0,120);
	FCOLOR[10]=RGB(0,0,160);
	FCOLOR[11]=RGB(80,0,80);
	FCOLOR[12]=RGB(120,0,120);
	FCOLOR[13]=RGB(180,180,180);
	FCOLOR[51]=RGB(0,0,255);
	for(NumChanges=0; NumChanges<9; NumChanges++)
		OrderToFuel[NumChanges]=0;
	for(NumChanges=0; NumChanges<37; NumChanges++)
		FuelMod.ModelIsHere[NumChanges]=false;
	NumChanges=0;
}


void ResetFuelConversions()
{
	for(long i=0; i<257; i++)             		// could also read default file here
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


void ResetCustomFuelModels()
{
	long Changes;
	for(Changes=0; Changes<9; Changes++)
		FuelMod.OrderToFuel[Changes]=0;
	for(Changes=0; Changes<37; Changes++)
	{	SetFuel(Changes, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "");
		FuelMod.ModelIsHere[Changes]=false;
	}
	FuelMod.NumChanges=0;
	HAVE_CUST_MODELS=false;
}


void CheckMessageLoop()
{// catches and interprets incoming WINDOWS messages
	MSG msg;

	if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{    TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


long GridInterval(long Interval)
{// get & set Grid Interval
	if(Interval>=0)
		OverlayGridInterval=Interval;

	return OverlayGridInterval;
}


COLORREF GridColor(long Colr)
{// get & set Grid Color
	/*if(Colr>=0)
	{	switch(Colr)
		{	case 0: OverlayGridColor=GetPaletteColor(255, 255, 255); break;
			case 1: OverlayGridColor=GetPaletteColor(192, 192, 192); break;
			case 2: OverlayGridColor=GetPaletteColor(128, 128, 128); break;
			case 3: OverlayGridColor=GetPaletteColor(0, 0, 0); break;
			default:OverlayGridColor=GetPaletteColor(255, 255, 255); break;
		}
	}

	return OverlayGridColor;
	*/

	return 1;
}


long TopoShadeDirection(long Direction)
{// get & set Light Source Direction
	if(Direction>=0)
		LightSourceDirection=Direction;

	return LightSourceDirection;
}


double pow2(double base)
{// replaces pow(X,Y) where Y==2
	return base*base;
}

void SetFullTint(double tint)
{
	FullTint=tint;
}

double GetFullTint()
{
	return FullTint;
}

void SetTint(double Tint)
{// sets minimum display units for landcape window
	tint=Tint;
}


double GetTint()
{// retrieves minimum display unit for landscappe window
	return tint;
}


void SetMaxWindowDim(long Dim)
{// sets dimension of landscape window
	MaxWindowDim=Dim;
}


long GetMaxWindowDim()
{// gets dimension of landscape window
	return MaxWindowDim;
}

bool ViewPortChanged(long TrueFalse)
{
	if(TrueFalse>=0)
		ViewPort_Changed=TrueFalse;

	return ViewPort_Changed;
}

bool ViewPortStatus(long TrueFalse)
{
	if(TrueFalse>=0)
		ViewPort_Status=TrueFalse;

	return ViewPort_Status;
}

void SetLandscapeViewPort(double north, double south, double east, double west)
{
	double cols, rows;
	ViewPortNorth=north;
	ViewPortSouth=south;
	ViewPortEast=east;
	ViewPortWest=west;
	rows=(ViewPortNorth-ViewPortSouth)/Header.YResol;
	NumViewNorth=rows;
	if(modf(rows, &rows)>0.5)
		NumViewNorth++;
	cols=(ViewPortEast-ViewPortWest)/Header.XResol;
	NumViewEast=cols;
	if(modf(cols, &cols)>0.5)
		NumViewEast++;
}

void GetLandscapeViewPort(double *north, double *south, double *east, double *west)
{
	*north=ViewPortNorth;
	*south=ViewPortSouth;
	*east=ViewPortEast;
	*west=ViewPortWest;
}


double GetViewNorth()
{
	return ViewPortNorth;
}

double GetViewSouth()
{
	return ViewPortSouth;
}

double GetViewEast()
{
	return ViewPortEast;
}

double GetViewWest()
{
	return ViewPortWest;
}

long GetNumViewEast()
{
	return NumViewEast;
}

long GetNumViewNorth()
{
	return NumViewNorth;
}


char* GetCurrentPath()
{
     if(!UseCurrentPath)
          memset(CurrentPath, 0x0, 256*sizeof(char));
     else
          GetCurrentDirectory(256, reinterpret_cast<LPSTR>(CurrentPath));

     return CurrentPath;
}


bool CanUseCurrentPath(long YesNo)
{
     if(YesNo>-1)
          UseCurrentPath=(bool) YesNo;

     return UseCurrentPath;
}


//------------------------------------------------------------------------------
//
//
//
//------------------------------------------------------------------------------


long AdjustIgnitionSpreadRates(long YesNoValue)
{
     if(YesNoValue>-1)
          AdjustIgSpreadRates=YesNoValue;

     return AdjustIgSpreadRates;
}


//------------------------------------------------------------------------------
//
//
//
//------------------------------------------------------------------------------


CanopyCharacteristics::CanopyCharacteristics()
{	DefaultHeight=Height=15.0;           // need default for changing Variables in LCP
	DefaultBase=CrownBase=4.0;
	DefaultDensity=BulkDensity=0.20;
	Diameter=20.0;
	FoliarMC=100;
	Tolerance=2;
	Species=1;
}


void SetCanopyChx(double Height, double CrownBase, double BulkDensity, double Diameter,
			   long FoliarMoisture, long Tolerance, long Species)
{
	CanopyChx.Height=CanopyChx.DefaultHeight=Height;
	CanopyChx.CrownBase=CanopyChx.DefaultBase=CrownBase;
	CanopyChx.BulkDensity=CanopyChx.DefaultDensity=BulkDensity;
	CanopyChx.Diameter=Diameter;
	CanopyChx.FoliarMC=FoliarMoisture;
	CanopyChx.Tolerance=Tolerance;
	CanopyChx.Species=Species;
}


void SetFoliarMoistureContent(long Percent)
{
     if(Percent<1)
     	Percent=100;
     if(Percent>300)
     	Percent=300;
	CanopyChx.FoliarMC=Percent;
}


void GetDefaultCrownChx(double *Height, double *Base, double *Density)
{
	*Height=CanopyChx.DefaultHeight;
	*Base=CanopyChx.DefaultBase;
	*Density=CanopyChx.DefaultDensity;
}


double GetDefaultCrownHeight()
{
	return CanopyChx.Height;
}


double GetDefaultCrownBase()
{
	return CanopyChx.CrownBase;
}


double GetDefaultCrownBD(short cover)
{
	if(CrownDensityLinkToCC)
		return CanopyChx.BulkDensity*((double) cover)/100.0;

	return CanopyChx.BulkDensity;
}


double GetAverageDBH()
{
	return CanopyChx.Diameter;
}


double GetFoliarMC()
{
	return CanopyChx.FoliarMC;
}


long GetTolerance()
{
	return CanopyChx.Tolerance;
}


long GetCanopySpecies()
{
	return CanopyChx.Species;
}


int GetNumFuelChanges()
{
	return FuelMod.NumChanges;
}


void SetNumFuelChanges(int Num)
{
	if(Num==0)                         // reset all changes if Num is 0
	{	for(Num=0; Num<9; Num++)
			FuelMod.OrderToFuel[Num]=0;
		Num=0;
	}
	FuelMod.NumChanges=Num;
}


void IncNumFuelChanges()
{
	FuelMod.NumChanges++;
}


//double GetFuelDepth(int Number)
//{
//	return FuelMod.Depth[Number];
//}

long FuelModelUnits=0;

long AccessFuelModelUnits(long Val)
{
	if(Val>-1)
     	FuelModelUnits=Val;

	return FuelModelUnits;
}


void GetFuel(int Number, double *t1, double *t10, double *t100, double *tLH, double *tLW,
                         double *s1, double *sLH, double *sLW, double *hd, double *hl,
					double *d, double *xm, char *comment)
{
	*t1=FuelMod.TL1[Number];
	*t10=FuelMod.TL10[Number];
	*t100=FuelMod.TL100[Number];
	*tLH=FuelMod.TLLiveH[Number];
	*tLW=FuelMod.TLLiveW[Number];
	*s1=((double) FuelMod.S1[Number]);
	*sLH=((double) FuelMod.SLiveH[Number]);
	*sLW=((double) FuelMod.SLiveW[Number]);
	*hd=((double) FuelMod.HD[Number]);
	*hl=((double) FuelMod.HL[Number]);
	*d=FuelMod.Depth[Number];
	*xm=FuelMod.XMext[Number];
     if(comment!=NULL)
	     strcpy(comment, FuelMod.Comment[Number]);
}


void SetFuel(int Number, double t1, double t10, double t100, double tLH,
					double tLW, long s1, long sLH, long sLW, long hd,
                         long hl, double d, double xm, char *comment)
{
	FuelMod.ModelIsHere[Number]=true;
	FuelMod.TL1[Number]=t1;
	FuelMod.TL10[Number]=t10;
	FuelMod.TL100[Number]=t100;
	FuelMod.TLLiveH[Number]=tLH;
	FuelMod.TLLiveW[Number]=tLW;
	FuelMod.S1[Number]=s1;
	FuelMod.SLiveH[Number]=sLH;
	FuelMod.SLiveW[Number]=sLW;
	FuelMod.HD[Number]=hd;
	FuelMod.HL[Number]=hl;
	FuelMod.Depth[Number]=d;
	FuelMod.XMext[Number]=xm;
     if(comment!=NULL)
	     strcpy(FuelMod.Comment[Number], comment);
}


bool	IsCustomFuelModelHere(long Number)
{
	return FuelMod.ModelIsHere[Number];
}


char *GetCustomModelDescription(long Number)
{
	if(FuelMod.ModelIsHere[Number]==false)
     	return NULL;

     return FuelMod.Comment[Number];
}


void SetTopoShading(bool YesNo)
{
	TOPOSHADING=YesNo;
}


bool TopoShading()
{
	return TOPOSHADING;
}


long GetInputMode()
{// gets input mode to the landscape window

	return LandscapeInputMode;
}


void SetInputMode(long ModeCode, bool Pending)
{// sets input model to the landscape window
	LandscapeInputMode=ModeCode;
     if(Pending)
     	LandscapeInputMode+=100;
}


COLORREF GetFuelColor(int Number)
{
	if(Number==98 || Number==-2)
		Number=51;
	else if(Number==99 || Number==-1)
		Number=0;
	if(Number<52 && Number>-1)
	{    COLORREF colr=FuelMod.FCOLOR[Number];
		if(ValidPalette)
			return colr ;//GetPaletteColor(GetRValue(colr),GetGValue(colr),GetBValue(colr));

		return colr;
	}

	return RGB(0, 0, 0);
}


COLORREF GetChangedColor(int Number)
{
	COLORREF colr=FuelMod.CCOLOR[Number];
	if(ValidPalette)
		return 1;//GetPaletteColor(GetRValue(colr),GetGValue(colr),GetBValue(colr));
	else
		return colr;
}


void SetFuelColor(int Number, COLORREF colr)
{
	if(Number>51 || Number==-2)
		Number=51;
	if(Number==-1)
		Number=0;
	FuelMod.FCOLOR[Number]=colr;
}


void SetChangedColor(int Number, COLORREF colr)
{
	FuelMod.CCOLOR[Number]=colr;
}


void SetOrderToFuel(int Number, int Order)
{
	FuelMod.OrderToFuel[Number]=Order;
}


int GetOrderToFuel(int Number)
{
	return FuelMod.OrderToFuel[Number];
}


void SetXORArrows(bool TorF)
{
	XORArrows=TorF;
}


bool GetXORArrows()
{
	return XORArrows;
}


void GetMaxDims(double *maxx, double *maxy)
{
	*maxx=MaxX;
	*maxy=MaxY;
}


void SetMaxDims(double maxx, double maxy)
{
	MaxX=maxx;
	MaxY=maxy;
}


void GetRatios(double *xratio, double *yratio)
{
	*xratio=xrat;
	*yratio=yrat;
}


void SetRatios(double xratio, double yratio)
{
	xrat=xratio;
	yrat=yratio;
}


//TWindow * GetFarsiteWindow()
/*
TFarsiteWindow * GetFarsiteWindow()
{
	return FarsiteWindow;
}


TThreeDWindow *Get3dWindow()
{
	return ThreeDWindow;
}


//void SetFarsiteWindow(TWindow* FWindow)
void SetFarsiteWindow(TFarsiteWindow* FWindow)
{
	FarsiteWindow=FWindow;
}


void Set3dWindow(TThreeDWindow *window)
{
	ThreeDWindow=window;
}
*/

HBITMAP GetBitmap(short Number)
{
	HBITMAP bm=0;

	switch(Number)
	{	case 1: bm=hBitmap1; break;
		case 2: bm=hBitmap2; break;
		case 3: bm=hBitmap3; break;
	}

	return bm;
}


void ResetBitmap(short Number)
{
	switch(Number)
	{	case 1: if(hBitmap1)
				DeleteObject(hBitmap1);
				hBitmap1=0;
				break;
		case 2: if(hBitmap2)
				DeleteObject(hBitmap2);
				hBitmap2=0;
				break;
		case 3: if(hBitmap3)
				DeleteObject(hBitmap3);
				hBitmap3=0;
				break;
	}
}


void SetBitmap(short Number, HBITMAP hBitmap)
{
	switch(Number)
	{	case 1: hBitmap1=hBitmap; break;
		case 2: hBitmap2=hBitmap; break;
		case 3: hBitmap3=hBitmap; break;
	}
}

int GetFuelsAltered()
{
	return FuelsAltered;
}

void SetFuelsAltered(int YesNo)
{
	FuelsAltered=YesNo;
}


FuelConversions::FuelConversions()
{
	long i;

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


int GetFuelConversion(int fuel)
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


int SetFuelConversion(int From, int To)
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


bool ChangeInputMode(long YesNo)
{// true if new change, false if not, -1 returns existing value
	if(YesNo>=0)
		InputChanged=YesNo;

	return InputChanged;
}


void SetZoomReplayDelay(long MilliSeconds)
{
     if(MilliSeconds>1000)
          MilliSeconds=1000;
     if(MilliSeconds<0)
          MilliSeconds=0;
     ZoomReplayDelay=MilliSeconds;
}


long GetZoomReplayDelay()
{
     return ZoomReplayDelay;
}

long GetTheme_DistanceUnits()
{
	return Header.GridUnits;
}


short GetTheme_Units(short DataTheme)
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
}

short GetTheme_NumCategories(short DataTheme)
{
	long cats;

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

long GetTheme_HiValue(short DataTheme)
{
	long hi;

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

long GetTheme_LoValue(short DataTheme)
{
	long lo;

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


char *GetTheme_FileName(short DataTheme)
{
     char name[256]="";

	switch(DataTheme)
     {	case 0:	strcpy(name, Header.ElevFile); break;
     	case 1:	strcpy(name, Header.SlopeFile); break;
     	case 2:	strcpy(name, Header.AspectFile); break;
     	case 3:	strcpy(name, Header.FuelFile); break;
     	case 4:	strcpy(name, Header.CoverFile); break;
     	case 5:	strcpy(name, Header.HeightFile); break;
     	case 6:	strcpy(name, Header.BaseFile); break;
     	case 7:	strcpy(name, Header.DensityFile); break;
     	case 8:	strcpy(name, Header.DuffFile); break;
     	case 9:	strcpy(name, Header.WoodyFile); break;
     }

     return &name[0];
}


bool	LinkDensityWithCrownCover(long TrueFalse)
{
 	if(TrueFalse>=0)
     	CrownDensityLinkToCC=TrueFalse;

     return CrownDensityLinkToCC;
};


long HaveCrownFuels()
{
	return Header.CrownFuels-20;		// subtract 10 to ID file as version 2.x
}

long HaveGroundFuels()
{
	return Header.GroundFuels-20;
}


double GetCellResolutionX()
{
//	CellResolution=(Header.NorthUtm-Header.SouthUtm)/Header.numnorth;
//	CellResolution=(Header.EastUtm-Header.WestUtm)/Header.numeast;

	return Header.XResol;
}


double GetCellResolutionY()
{
//	CellResolution=(Header.NorthUtm-Header.SouthUtm)/Header.numnorth;
//	CellResolution=(Header.EastUtm-Header.WestUtm)/Header.numeast;

	return Header.YResol;
}


double MetricResolutionConvert()
{
	if(Header.GridUnits==1)
		return 3.280839895;     // metric conversion to meters
	else
		return 1.0;
}


int CheckCellResUnits()
{
	return Header.GridUnits;
}


void SetCustFuelModelID(bool True_False)
{
	HAVE_CUST_MODELS=True_False;
}


void SetConvFuelModelID(bool True_False)
{
	HAVE_CONV_MODELS=True_False;
}


bool NeedCustFuelModels()
{
	return NEED_CUST_MODELS;
}


bool NeedConvFuelModels()
{
	return NEED_CONV_MODELS;
}


bool HaveCustomFuelModels()
{
	return HAVE_CUST_MODELS;
}


bool HaveFuelConversions()
{
	return HAVE_CONV_MODELS;
}


void SetRastFormat(long Type)
{
	RastFormat=Type;
}


long GetRastFormat()
{
	return RastFormat;
}


void SetVectFormat(long Type)
{
	VectFormat=Type;
}


long GetVectFormat()
{
	return VectFormat;
}


long CanSetRasterResolution(long YesNo)
{
	if(YesNo>-1)
		CanSetRastRes=YesNo;

	return CanSetRastRes;
}


long SetRasterExtentToViewport(long YesNo)
{
	if(YesNo>-1)
		RasterVPExtent=YesNo;

	return RasterVPExtent;
}


void SetRastRes(double XResol, double YResol)
{// raster output resolution
	RasterCellResolutionX=XResol;
	RasterCellResolutionY=YResol;
}


void GetRastRes(double *XResol, double *YResol)
{// raster output resolution
	*XResol=RasterCellResolutionX;
	*YResol=RasterCellResolutionY;
}


void SetVectorFileName(char *FileName)
{
	memset(VectFName, 0x0, sizeof(VectFName));
	sprintf(VectFName, "%s", FileName);
	FILE *newfile;

     newfile=fopen(VectFName, "w");
     if(newfile==NULL)
     {    SetFileAttributes(reinterpret_cast<LPCSTR>(VectFName), FILE_ATTRIBUTE_NORMAL);
          newfile=fopen(VectFName, "w");
     }
     fclose(newfile);
}


char *GetVectorFileName()
{
	return VectFName;
}


void SetRasterFileName(char *FileName)
{
	memset(RasterArrivalTime, 0x0, sizeof(RasterArrivalTime));
	memset(RasterFireIntensity, 0x0, sizeof(RasterFireIntensity));
	memset(RasterFlameLength, 0x0, sizeof(RasterFlameLength));
	memset(RasterSpreadRate, 0x0, sizeof(RasterSpreadRate));
	memset(RasterHeatPerArea, 0x0, sizeof(RasterHeatPerArea));
	memset(RasterCrownFire, 0x0, sizeof(RasterCrownFire));
	memset(RasterFireDirection, 0x0, sizeof(RasterFireDirection));
	memset(RasterReactionIntensity, 0x0, sizeof(RasterReactionIntensity));
	memset(RastFName, 0x0, sizeof(RastFName));
	sprintf(RasterArrivalTime, "%s%s", FileName, ".toa");
	sprintf(RasterFireIntensity, "%s%s", FileName, ".fli");
	sprintf(RasterFlameLength, "%s%s", FileName, ".fml");
	sprintf(RasterSpreadRate, "%s%s", FileName, ".ros");
	sprintf(RasterHeatPerArea, "%s%s", FileName, ".hpa");
	sprintf(RasterCrownFire, "%s%s", FileName, ".cfr");
	sprintf(RasterFireDirection, "%s%s", FileName, ".sdr");
	sprintf(RasterReactionIntensity, "%s%s", FileName, ".rci");
	sprintf(RastFName, "%s%s", FileName, ".rst");
}


char *GetRasterFileName(long Type)
{
	if(Type==0)
		return RastFName;
	if(Type==RAST_ARRIVALTIME)
		return RasterArrivalTime;
	if(Type==RAST_FIREINTENSITY)
		return RasterFireIntensity;
	if(Type==RAST_FLAMELENGTH)
		return RasterFlameLength;
	if(Type==RAST_SPREADRATE)
		return RasterSpreadRate;
	if(Type==RAST_HEATPERAREA)
		return RasterHeatPerArea;
     if(Type==RAST_CROWNFIRE)
          return RasterCrownFire;
     if(Type==RAST_FIREDIRECTION)
          return RasterFireDirection;
     if(Type==RAST_REACTIONINTENSITY)
          return RasterReactionIntensity;

	return NULL;
}


void SetFileOutputOptions(long FileType, bool YesNo)
{
     switch(FileType)
     {    case RAST_ARRIVALTIME:
                    rast_arrivaltime=YesNo;
                    break;
          case RAST_FIREINTENSITY:
                    rast_fireintensity=YesNo;
                    break;
          case RAST_SPREADRATE:
                    rast_spreadrate=YesNo;
                    break;
          case RAST_FLAMELENGTH:
                    rast_flamelength=YesNo;
                    break;
          case RAST_HEATPERAREA:
                    rast_heatperarea=YesNo;
                    break;
          case RAST_CROWNFIRE:
                    rast_crownfire=YesNo;
                    break;
          case RAST_FIREDIRECTION:
                    rast_firedirection=YesNo;
                    break;
          case RAST_REACTIONINTENSITY:
                    rast_reactionintensity=YesNo;
                    break;
     }
}


bool GetFileOutputOptions(long FileType)
{
     bool YesNo=false;

     switch(FileType)
     {    case RAST_ARRIVALTIME:
                    YesNo=rast_arrivaltime;
                    break;
          case RAST_FIREINTENSITY:
                    YesNo=rast_fireintensity;
                    break;
          case RAST_SPREADRATE:
                    YesNo=rast_spreadrate;
                    break;
          case RAST_FLAMELENGTH:
                    YesNo=rast_flamelength;
                    break;
          case RAST_HEATPERAREA:
                    YesNo=rast_heatperarea;
                    break;
          case RAST_CROWNFIRE:
                    YesNo=rast_crownfire;
                    break;
          case RAST_FIREDIRECTION:
                    YesNo=rast_firedirection;
                    break;
          case RAST_REACTIONINTENSITY:
                    YesNo=rast_reactionintensity;
                    break;
     }

     return YesNo;
}


void SetRastMake(bool YesNo)
{
	RastMake=YesNo;
}


bool GetRastMake()
{
	return RastMake;
}

static long DisplayUnits=0;
static long OutputUnits=0;

long AccessDisplayUnits(long val)
{
	if(val>=0)
     	DisplayUnits=val;

     return DisplayUnits;
}


long AccessOutputUnits(long val)
{
	if(val>=0)
     	OutputUnits=val;

    	return OutputUnits;
}




void SetVectMake(bool YesNo)
{
	VectMake=YesNo;
}


bool GetVectMake()
{
	return VectMake;
}

void	SetShapeMake(bool YesNo)
{
	ShapeMake=YesNo;
}


bool	GetShapeMake()
{
	return ShapeMake;
}

long ExcludeBarriersFromVectorFiles(long YesNo)
{
	if(YesNo>=0)
		ShapeBarriersSeparately=YesNo;

     return ShapeBarriersSeparately;
}


void SetVectVisOnly(bool YesNo)
{
	VISONLY=YesNo;
}


bool GetVectVisOnly()
{
	return VISONLY;
}


void	SetShapeFileChx(char *ShapeName, long VisOnly, long Polygons, long BarriersSep)
{
	memset(ShapeFileName, 0x0, sizeof(ShapeFileName));
     strcpy(ShapeFileName, ShapeName);
	ShapeVisibleStepsOnly=VisOnly;
     ShapePolygonsNotLines=Polygons;
     ShapeBarriersSeparately=BarriersSep;
}


char* GetShapeFileChx(long *VisOnly, long *Polygons, long *BarriersSep)
{
	*VisOnly=ShapeVisibleStepsOnly;
     *Polygons=ShapePolygonsNotLines;
     *BarriersSep=ShapeBarriersSeparately;

     return ShapeFileName;
}

/*
//------------------------------------------------------------------------------
//  Raster Overlay Stuff
//------------------------------------------------------------------------------

static double rW, rE, rN, rS, rCellSizeX, rCellSizeY, rMaxVal, rMinVal;
static long   rRows, rCols;
static double *RasterOverlay=0;

bool	CreateRasterOverlay(char *FileName, double *max, double *min)
{
	long i, j;
     char TestRead[30];
     double value;
     double rMaxVal, rMinVal;
     double rN, rS, rE, rW, rCellSizeX, rCellSizeY;
     FILE *Rast;

     if((Rast=fopen(FileName, "r"))==NULL)
     	return false;

     if(RasterOverlay)
     	free(RasterOverlay);
     RasterOverlay=0;

	rMaxVal=1e-300;
     rMinVal=1e300;
     fscanf(Rast, "%s", TestRead);
     if(!strcmp(TestRead, "north:")) // GRASS File
	{    fscanf(Rast, "%lf", &rN);
		fscanf(Rast, "%s", TestRead);
		fscanf(Rast, "%lf", &rS);
		fscanf(Rast, "%s", TestRead);
		fscanf(Rast, "%lf", &rE);
		fscanf(Rast, "%s", TestRead);
		fscanf(Rast, "%lf", &rW);
		fscanf(Rast, "%s", TestRead);
		fscanf(Rast, "%ld", &rRows);
		fscanf(Rast, "%s", TestRead);
		fscanf(Rast, "%ld", &rCols);
		rCellSizeY=(rN-rS)/(double) rRows;
		rCellSizeX=(rE-rW)/(double) rCols;
	}
	else if(!strcmp(strupr(TestRead), "NCOLS"))      // ARC grid file
	{    fscanf(Rast, "%ld", &rCols);
		fscanf(Rast, "%s", TestRead);
		fscanf(Rast, "%ld", &rRows);
		fscanf(Rast, "%s", TestRead);
		fscanf(Rast, "%lf", &rW);
		fscanf(Rast, "%s", TestRead);
		fscanf(Rast, "%lf", &rS);
		fscanf(Rast, "%s", TestRead);
		fscanf(Rast, "%lf", &rCellSizeX);
		fscanf(Rast, "%s", TestRead);
		fscanf(Rast, "%s", TestRead);
		rN=rS+rCellSizeX*(double) rRows;
		rE=rW+rCellSizeX*(double) rCols;
		rCellSizeY=rCellSizeX;
	}

     if((RasterOverlay=(double *) calloc(rRows*rCols, sizeof(double)))==NULL)
     	return false;

	for(i=0; i<rRows; i++)
     {	for(j=0; j<rCols; j++)
     	{	fscanf(Rast, "%lf", &value);
               RasterOverlay[i*rCols+j]=value;
               if(value>=0.0)
               {    if(value<rMinVal)
               		rMinVal=value;
               	if(value>rMaxVal)
               		rMaxVal=value;
               }
          }
     }
     fclose(Rast);

     *max=rMaxVal;
     *min=rMinVal;

     rW=ConvertUtmToEastingOffset(rW);
     rE=ConvertUtmToEastingOffset(rE);
     rN=ConvertUtmToNorthingOffset(rN);
     rS=ConvertUtmToNorthingOffset(rS);

     return true;
}


void DeleteRasterOverlay()
{
     if(RasterOverlay)
     	free(RasterOverlay);
     RasterOverlay=0;
}

double QueryRasterOverlay(double xcoord, double ycoord)
{
     if(RasterOverlay==0)
     	return -2;
     if(xcoord<rW || xcoord>rE)
     	return -3;
     if(ycoord<rS || ycoord>rN)
     	return -4;

     double xpt, ypt;
     long easti, northi, posit;

	xpt=(xcoord-rW)/rCellSizeX;
	ypt=(ycoord-rS)/rCellSizeY;
	easti=((long) xpt);
	northi=((long) ypt);
	northi=rRows-northi-1;
	if(northi<0)
		northi=0;
	posit=(northi*rCols+easti);

	return RasterOverlay[posit];
}


bool	CheckRasterOverlay()
{
 	if(RasterOverlay)
     	return true;

     return false;
}


*/
//------------------------------------------------------------------------------
//  Vis Perim stuff
//------------------------------------------------------------------------------

static const long MaxCount=500;
static long i, MemCount=0;
static long MemPerim[MaxCount*2];
static HANDLE hVisperimSemaphore=0;
static long prevct;


void CopyOutVisPerimFile(char *NewVisFile)
{
	CopyFile(reinterpret_cast<LPCSTR>(VisPerim), reinterpret_cast<LPCSTR>(NewVisFile), false);
}


void CopyInVisPerimFile(char *NewVisFile)
{
     WaitForSingleObject(hVisperimSemaphore, INFINITE);
	if(strlen(VisPerim)>0)
     {	if((access(VisPerim, 00))!=-1)
		{	SetFileAttributes(reinterpret_cast<LPCSTR>(VisPerim), FILE_ATTRIBUTE_NORMAL);
		    	DeleteFile(reinterpret_cast<LPCSTR>(VisPerim));
			memset(VisPerim, 0x0, sizeof(VisPerim));
	     }
     }
	getcwd(VisPerim, 255);
//	strcat(VisPerim, "\\visperim.vsp");
     char RNum[16]="";
     sprintf(RNum, "%d", rand() %1000);
	strcat(VisPerim, "\\visperim");
     strcat(VisPerim, RNum);
     strcat(VisPerim, ".vsp");
     if((access(NewVisFile, 00))!=-1)
	{	SetFileAttributes(reinterpret_cast<LPCSTR>(NewVisFile), FILE_ATTRIBUTE_NORMAL);
     	CopyFile(reinterpret_cast<LPCSTR>(NewVisFile), reinterpret_cast<LPCSTR>(VisPerim), false);
     }
     MemCount=0;
     ReleaseSemaphore(hVisperimSemaphore, 1, &prevct);
}


void ResetVisPerimFile()
{
//	system("del c:\visperim.vsp");
     WaitForSingleObject(hVisperimSemaphore, INFINITE);
     if((access(VisPerim, 00))!=-1)
     {	SetFileAttributes(reinterpret_cast<LPCSTR>(VisPerim), FILE_ATTRIBUTE_NORMAL);
	     DeleteFile(reinterpret_cast<LPCSTR>(VisPerim));
     }
	memset(VisPerim, 0x0, sizeof VisPerim);
	getcwd(VisPerim, 255);
//	strcat(VisPerim, "\\visperim.vsp");
     char RNum[16]="";
     sprintf(RNum, "%d", rand() %1000);
	strcat(VisPerim, "\\visperim");
     strcat(VisPerim, RNum);
     strcat(VisPerim, ".vsp");
     if((access(VisPerim, 00))!=-1)
     {	SetFileAttributes(reinterpret_cast<LPCSTR>(VisPerim), FILE_ATTRIBUTE_NORMAL);
	     DeleteFile(reinterpret_cast<LPCSTR>(VisPerim));
     }
	MemCount=0;
     ReleaseSemaphore(hVisperimSemaphore, 1, &prevct);
//	VP=fopen(VisPerim, "w");
//	fclose(VP);
//	DeleteFile(VisPerim);
}


long GetVisPerimFile(long *xpt, long *ypt)
{
     if(VP)
     {	if(fscanf(VP, "%ld %ld", xpt, ypt)==EOF)    // end of file
			return -1;
	     else
		{	if(*xpt<0)
				return 0;					   // end of fire polygon
			else
				return 1;                        // successful read
		}
     }

     return -1;
}


void SetVisPerimFile(long xpt, long ypt)
{
     if(!VP)
     	return;

     MemPerim[MemCount*2]=xpt;
     MemPerim[MemCount*2+1]=ypt;
     MemCount++;
	if(MemCount>MaxCount-1)
     {    for(i=0; i<MemCount; i++)
          	fprintf(VP, "%ld %ld\n", MemPerim[i*2], MemPerim[i*2+1]);
     	MemCount=0;
     }
	//if(VP)
   	//	fprintf(VP, "%ld %ld\n", xpt, ypt);
}


void SetVisPerimPoint(long xpt, long ypt, long firenum)
{	// Writes "point" to file nad ends fire
     if(!VP)
     	return;

     if(MemCount<MaxCount-2)
     {	MemPerim[MemCount*2]=xpt;
     	MemPerim[MemCount*2+1]=ypt;
     	MemCount++;
      	MemPerim[MemCount*2]=xpt+1;
     	MemPerim[MemCount*2+1]=ypt+1;
     	MemCount++;
      	MemPerim[MemCount*2]=-firenum;
     	MemPerim[MemCount*2+1]=-1;
     	MemCount++;
     }
     else
     {    for(i=0; i<MemCount; i++)
          	fprintf(VP, "%ld %ld\n", MemPerim[i*2], MemPerim[i*2+1]);
     	MemCount=0;
          fprintf(VP, "%ld %ld\n", xpt, ypt);
		fprintf(VP, "%ld %ld\n", xpt+1, ypt+1);
		fprintf(VP, "%ld %ld\n", -firenum, -1);
     }
}


long OpenVisPerimFile(int readfromstart)
{
	long FilePos=0;

     WaitForSingleObject(hVisperimSemaphore, INFINITE);
	if(readfromstart)
	{	if((VP=fopen(VisPerim, "r"))!=NULL)
			fseek(VP, 0, SEEK_SET);
	}
	else
	{	if((VP=fopen(VisPerim, "r"))!=NULL)
		{	fseek(VP, 0, SEEK_END);
			FilePos=ftell(VP);//fgetpos(VP, &FilePos);
			fclose(VP);
		}
		VP=fopen(VisPerim, "a");
          if(VP==NULL)
          {    SetFileAttributes(reinterpret_cast<LPCSTR>(VisPerim), FILE_ATTRIBUTE_NORMAL);
     		VP=fopen(VisPerim, "a");
          }
	}

	return FilePos;
}


void OpenVisPerimFileAtPosition(long FilePosition)
{
     WaitForSingleObject(hVisperimSemaphore, INFINITE);
	if((VP=fopen(VisPerim, "r"))!=NULL)
		fseek(VP, FilePosition, SEEK_SET);
	else
   		VP=0;
}


void OpenCopyVisPerimFile(char *FileName)
{
     WaitForSingleObject(hVisperimSemaphore, INFINITE);
	if((VP=fopen(FileName, "r"))!=NULL)
		rewind(VP);
	else
   		VP=0;
}


long CloseVisPerimFile(int writeend, long firenum)
{
    	long FSize=0;

	if(VP)
   	{    for(i=0; i<MemCount; i++)
          	fprintf(VP, "%ld %ld\n", MemPerim[i*2], MemPerim[i*2+1]);
     	MemCount=0;
     	if(writeend)
		{	fprintf(VP, "%ld %ld\n", -firenum, -1);
			FSize=1;
		}
		fclose(VP);
     }
     ReleaseSemaphore(hVisperimSemaphore, 1, &prevct);

	return FSize;
}


bool CreateVisperimSemaphore()
{
     CloseVisperimSemaphore();
	hVisperimSemaphore=CreateSemaphore(NULL, 1, 1, reinterpret_cast<LPCSTR>("VPSEMAPHORE"));
     if(hVisperimSemaphore==0)
     	return false;

     return true;
}

void CloseVisperimSemaphore()
{
     if(hVisperimSemaphore)
		CloseHandle(hVisperimSemaphore);
     hVisperimSemaphore=0;
}


//--------------------------------------------------------------------------------
//
//	view 3-D RASTER & VECTOR file information
//
//--------------------------------------------------------------------------------


void SetViewRasterThemeInfo(char *filename, long hi, long lo)
{
	strcpy(ThreeDRasterFile, filename);
	ThreeDRasterHi=hi;
	ThreeDRasterLo=lo;
}

char* GetViewRasterThemeInfo(long *hi, long *lo)
{
	*hi=ThreeDRasterHi;
	*lo=ThreeDRasterLo;

	return ThreeDRasterFile;
}

bool Draw3DVectorFile(long YesNo)
{
	if(YesNo>=0)
		View3DVectorFile=YesNo;

	return View3DVectorFile;
}


void SetViewVectorThemeInfo(char *filename, COLORREF colr, long FileType)
{
	strcpy(ThreeDVectorFile, filename);
	VectorColor=colr;
	VectorFileType=FileType;
}


char* GetViewVectorThemeInfo(COLORREF *colr)
{
	*colr=VectorColor;

	return ThreeDVectorFile;
}


long GetViewVectorFileType()
{
	return VectorFileType;
}


//--------------------------------------------------------------------------------
//
//	manage weather/wind stream data
//
//--------------------------------------------------------------------------------

bool CalcFirstLastStreamData()
{
     bool           BadRange=false;
     long           sn;
     unsigned long  FirstWtr, FirstWnd, LastWtr, LastWnd;
     double         ipart, fract;

	for(sn=0; sn<GetNumStations(); sn++)
	{    if(AtmosphereGridExists()<2)
     	{  	if(!wtrdt[sn] || (!wddt[sn] && AtmosphereGridExists()==0))
	          {
		               continue;
          	}
          }

          FirstWtr=(GetJulianDays(FirstMonth[sn].wtr)+FirstDay[sn].wtr)*1440.0;
          fract=modf((double) FirstHour[sn].wnd/100.0, &ipart);
          FirstWnd=(GetJulianDays(FirstMonth[sn].wnd)+FirstDay[sn].wnd)*1440.0+
                    ipart*60.0+fract;
          LastWtr=(GetJulianDays(LastMonth[sn].wtr)+LastDay[sn].wtr)*1440.0;
          fract=modf((double) LastHour[sn].wnd/100.0, &ipart);
          LastWnd=(GetJulianDays(LastMonth[sn].wnd)+LastDay[sn].wnd)*1440.0+
                    ipart*60.0+fract;

          if(FirstWtr>LastWtr)
               LastWtr+=365.0*1440.0;
          if(FirstWnd>LastWnd)
               LastWnd+=365.0*1440.0;

          if(FirstWnd<=FirstWtr)
          {    if(LastWnd<=FirstWtr)
                    BadRange=true;
          }
          else if(FirstWnd>FirstWtr)
          {    if(FirstWnd>LastWtr)
                    BadRange=true;
          }
          if(BadRange)
          {    FirstMonth[sn].all=0;
               FirstDay[sn].all=0;
               FirstHour[sn].all=0;

               return false;
          }

          if(FirstMonth[sn].wtr>FirstMonth[sn].wnd)
		{	FirstMonth[sn].all=FirstMonth[sn].wtr;
			FirstDay[sn].all=FirstDay[sn].wtr;
			FirstHour[sn].all=FirstHour[sn].wtr;
		}
		else if(FirstMonth[sn].wtr<FirstMonth[sn].wnd)
		{	FirstMonth[sn].all=FirstMonth[sn].wnd;
			FirstDay[sn].all=FirstDay[sn].wnd;
			FirstHour[sn].all=FirstHour[sn].wnd;
		}
		else
		{	FirstMonth[sn].all=FirstMonth[sn].wtr;
			if(FirstDay[sn].wtr>FirstDay[sn].wnd)
			{	FirstDay[sn].all=FirstDay[sn].wtr;
				FirstHour[sn].all=FirstHour[sn].wtr;
			}
			else if(FirstDay[sn].wtr<FirstDay[sn].wnd)
			{	FirstDay[sn].all=FirstDay[sn].wnd;
				FirstHour[sn].all=FirstHour[sn].wnd;
			}
			else
			{    FirstDay[sn].all=FirstDay[sn].wnd;
				if(FirstHour[sn].wtr>FirstHour[sn].wnd)
					FirstHour[sn].all=FirstHour[sn].wtr;
				else
					FirstHour[sn].all=FirstHour[sn].wnd;
			}
		}
		if(LastMonth[sn].wtr<LastMonth[sn].wnd)
		{	LastMonth[sn].all=LastMonth[sn].wtr;
			LastDay[sn].all=LastDay[sn].wtr;
			LastHour[sn].all=LastHour[sn].wtr;
		}
		else if(LastMonth[sn].wtr>LastMonth[sn].wnd)
		{	LastMonth[sn].all=LastMonth[sn].wnd;
			LastDay[sn].all=LastDay[sn].wnd;
			LastHour[sn].all=LastHour[sn].wnd;
		}
		else
		{	LastMonth[sn].all=LastMonth[sn].wtr;
			if(LastDay[sn].wtr<LastDay[sn].wnd)
			{	LastDay[sn].all=LastDay[sn].wtr;
				LastHour[sn].all=LastHour[sn].wtr;
			}
			else if(LastDay[sn].wtr>LastDay[sn].wnd)
			{	LastDay[sn].all=LastDay[sn].wnd;
				LastHour[sn].all=LastHour[sn].wnd;
			}
			else
			{    LastDay[sn].all=LastDay[sn].wnd;
				if(LastHour[sn].wtr<LastHour[sn].wnd)
					LastHour[sn].all=LastHour[sn].wtr;
				else
					LastHour[sn].all=LastHour[sn].wnd;
			}
		}
	}

     return true;
}



//--------------------------------------------------------------------------------
//
//	Manage Atmosphere Grid with regular Weather/Wind Stream Data
//
//--------------------------------------------------------------------------------


void SetCorrectStreamNumbers()
{
	long i;

	if(AtmosphereGridExists()==1)		// if wind grid only
     {	for(i=0; i<5; i++)
     		FreeWindData(i);
     	if(NumWeatherStations>0)
	          NumWindStations=NumWeatherStations;
          else
          	NumWindStations=1;
          for(i=0; i<NumWindStations; i++)
          {	FirstMonth[i].wnd=AtmGrid->GetAtmMonth(0);
               LastMonth[i].wnd=AtmGrid->GetAtmMonth(AtmGrid->GetTimeIntervals()-1);
			FirstDay[i].wnd=AtmGrid->GetAtmDay(0);
               LastDay[i].wnd=AtmGrid->GetAtmDay(AtmGrid->GetTimeIntervals()-1);
			FirstHour[i].wnd=AtmGrid->GetAtmHour(0);
               LastHour[i].wnd=AtmGrid->GetAtmHour(AtmGrid->GetTimeIntervals()-1);
          }
     }
     else if(AtmosphereGridExists()==2)	// if weather and wind grids
     {    for(i=0; i<5; i++)
     	{	FreeWindData(i);
           	FreeWeatherData(i);
          }
          i=0;
          NumWindStations=1;
          NumWeatherStations=1;
          FirstMonth[i].wnd=AtmGrid->GetAtmMonth(0);
          LastMonth[i].wnd=AtmGrid->GetAtmMonth(AtmGrid->GetTimeIntervals()-1);
		FirstDay[i].wnd=AtmGrid->GetAtmDay(0);
          LastDay[i].wnd=AtmGrid->GetAtmDay(AtmGrid->GetTimeIntervals()-1);
		FirstHour[i].wnd=AtmGrid->GetAtmHour(0);
          LastHour[i].wnd=AtmGrid->GetAtmHour(AtmGrid->GetTimeIntervals()-1);
          FirstMonth[i].wtr=AtmGrid->GetAtmMonth(0);
          LastMonth[i].wtr=AtmGrid->GetAtmMonth(AtmGrid->GetTimeIntervals()-1);
		FirstDay[i].wtr=AtmGrid->GetAtmDay(0);
          LastDay[i].wtr=AtmGrid->GetAtmDay(AtmGrid->GetTimeIntervals()-1);
		FirstHour[i].wtr=AtmGrid->GetAtmHour(0);
          LastHour[i].wtr=AtmGrid->GetAtmHour(AtmGrid->GetTimeIntervals()-1);
     }
}



//--------------------------------------------------------------------------------
//
//	wind data structures
//
//--------------------------------------------------------------------------------

long GetOpenWindStation()
{
     long i;

	for(i=0; i<5; i++)
	{	if(!wddt[i])
		break;
	}

	return i;
}


long AllocWindData(long StatNum, long NumObs)
{
	long StationNumber=StatNum;

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


void FreeWindData(long StationNumber)
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


long SetWindData(long StationNumber, long NumObs, long month, long day,
			  long hour, double windspd, long winddir, long cloudy)
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

long GetWindMonth(long StationNumber, long NumObs)
{
     if(NumObs>MaxWindObs[StationNumber]-1)
     	NumObs=MaxWindObs[StationNumber]-1;

	return wddt[StationNumber][NumObs].mo;
}

long GetWindDay(long StationNumber, long NumObs)
{

     if(NumObs>MaxWindObs[StationNumber]-1)
     	NumObs=MaxWindObs[StationNumber]-1;

	return wddt[StationNumber][NumObs].dy;
}

long GetWindHour(long StationNumber, long NumObs)
{
     if(NumObs>MaxWindObs[StationNumber]-1)
     	NumObs=MaxWindObs[StationNumber]-1;

	return wddt[StationNumber][NumObs].hr;
}

double GetWindSpeed(long StationNumber, long NumObs)
{
     if(NumObs>MaxWindObs[StationNumber]-1)
     	NumObs=MaxWindObs[StationNumber]-1;

	return wddt[StationNumber][NumObs].ws;
}

long GetWindDir(long StationNumber, long NumObs)
{
     if(NumObs>MaxWindObs[StationNumber]-1)
     	NumObs=MaxWindObs[StationNumber]-1;

	return wddt[StationNumber][NumObs].wd;
}

long GetWindCloud(long StationNumber, long NumObs)
{
     if(NumObs>MaxWindObs[StationNumber]-1)
     	NumObs=MaxWindObs[StationNumber]-1;

	return wddt[StationNumber][NumObs].cl;
}

long GetMaxWindObs(long StationNumber)
{
	if(StationNumber>4)
     	return -1;

     return MaxWindObs[StationNumber]-1;
}

//--------------------------------------------------------------------------------
//
//	weather data structures
//
//--------------------------------------------------------------------------------

long GetOpenWeatherStation()
{
	long i;

	for(i=0; i<5; i++)
	{	if(!wtrdt[i])
		break;
	}

	return i;
}


long GetNumStations()
{
	if(NumWeatherStations>NumWindStations)
		return NumWindStations;

	return NumWeatherStations;
}


long AllocWeatherData(long StatNum, long NumObs)
{
	long StationNumber=StatNum;

	if(wtrdt[StationNumber])
	{    delete[] wtrdt[StationNumber];//free(wtrdt[StationNumber]);
		wtrdt[StationNumber]=0;
          MaxWeatherObs[StatNum]=0;
	}
	else
	{	StationNumber=GetOpenWeatherStation();
		if(StationNumber<5)
			NumWeatherStations=StationNumber+1;
	}
	if(StationNumber<5 && NumObs>0)
	{	size_t nmemb=MaxWeatherObs[StationNumber]=NumObs+20;		// alloc 20 more than needed
		if((wtrdt[StationNumber] = new WeatherData[nmemb])==NULL)
			StationNumber=-1;
	}
	else
		StationNumber=-1;

	return StationNumber;
}


void FreeWeatherData(long StationNumber)
{
	if(wtrdt[StationNumber])
	{    delete[] wtrdt[StationNumber];
          MaxWeatherObs[StationNumber]=0;
		NumWeatherStations--;
	}
	wtrdt[StationNumber]=0;
     FirstMonth[StationNumber].wtr=0;
	LastMonth[StationNumber].wtr=0;
	FirstDay[StationNumber].wtr=0;
	LastDay[StationNumber].wtr=0;
	FirstHour[StationNumber].wtr=0;
	LastHour[StationNumber].wtr=0;
}


long SetWeatherData(long StationNumber, long NumObs, long month, long day, double rain, long time1, long time2,
				double temp1, double temp2, long humid1, long humid2, double elevation, long tr1, long tr2)
{
	if(NumObs<MaxWeatherObs[StationNumber])
	{	wtrdt[StationNumber][NumObs].mo=month;
		wtrdt[StationNumber][NumObs].dy=day;
		wtrdt[StationNumber][NumObs].rn=rain;
		wtrdt[StationNumber][NumObs].t1=time1;
		wtrdt[StationNumber][NumObs].t2=time2;
		wtrdt[StationNumber][NumObs].T1=temp1;
		wtrdt[StationNumber][NumObs].T2=temp2;
		wtrdt[StationNumber][NumObs].H1=humid1;
		wtrdt[StationNumber][NumObs].H2=humid2;
		wtrdt[StationNumber][NumObs].el=elevation;
		wtrdt[StationNumber][NumObs].tr1=tr1;
		wtrdt[StationNumber][NumObs].tr2=tr2;

          if(month==13)
		{	FirstMonth[StationNumber].wtr=wtrdt[StationNumber][0].mo;
			LastMonth[StationNumber].wtr=wtrdt[StationNumber][NumObs-1].mo;
			FirstDay[StationNumber].wtr=wtrdt[StationNumber][0].dy;
			LastDay[StationNumber].wtr=wtrdt[StationNumber][NumObs-1].dy;
			FirstHour[StationNumber].wtr=wtrdt[StationNumber][0].t1;
			LastHour[StationNumber].wtr=wtrdt[StationNumber][NumObs-1].t2;
		}

          EnvtChanged[0][StationNumber]=true;   // 1hr fuels
          EnvtChanged[1][StationNumber]=true;   // 10hr fuels
          EnvtChanged[2][StationNumber]=true;   // 100hr fuels
          EnvtChanged[3][StationNumber]=true;   // 1000hr fuels
		return 1;
	}

	return 0;
}


long GetWeatherMonth(long StationNumber, long NumObs)
{
     if(NumObs>MaxWeatherObs[StationNumber]-1)
     	NumObs=MaxWeatherObs[StationNumber]-1;

	return wtrdt[StationNumber][NumObs].mo;
}

long GetWeatherDay(long StationNumber, long NumObs)
{
     if(NumObs>MaxWeatherObs[StationNumber]-1)
     	NumObs=MaxWeatherObs[StationNumber]-1;

	return wtrdt[StationNumber][NumObs].dy;
}

double GetWeatherRain(long StationNumber, long NumObs)
{
     if(NumObs>MaxWeatherObs[StationNumber]-1)
     	NumObs=MaxWeatherObs[StationNumber]-1;

	return wtrdt[StationNumber][NumObs].rn;
}

long GetWeatherTime1(long StationNumber, long NumObs)
{
     if(NumObs>MaxWeatherObs[StationNumber]-1)
     	NumObs=MaxWeatherObs[StationNumber]-1;

	return wtrdt[StationNumber][NumObs].t1;
}

long GetWeatherTime2(long StationNumber, long NumObs)
{
     if(NumObs>MaxWeatherObs[StationNumber]-1)
     	NumObs=MaxWeatherObs[StationNumber]-1;

	return wtrdt[StationNumber][NumObs].t2;
}

double GetWeatherTemp1(long StationNumber, long NumObs)
{
     if(NumObs>MaxWeatherObs[StationNumber]-1)
     	NumObs=MaxWeatherObs[StationNumber]-1;

	return wtrdt[StationNumber][NumObs].T1;
}

double GetWeatherTemp2(long StationNumber, long NumObs)
{
     if(NumObs>MaxWeatherObs[StationNumber]-1)
     	NumObs=MaxWeatherObs[StationNumber]-1;

	return wtrdt[StationNumber][NumObs].T2;
}

long GetWeatherHumid1(long StationNumber, long NumObs)
{
     if(NumObs>MaxWeatherObs[StationNumber]-1)
     	NumObs=MaxWeatherObs[StationNumber]-1;

	return wtrdt[StationNumber][NumObs].H1;
}

long GetWeatherHumid2(long StationNumber, long NumObs)
{
     if(NumObs>MaxWeatherObs[StationNumber]-1)
     	NumObs=MaxWeatherObs[StationNumber]-1;

	return wtrdt[StationNumber][NumObs].H2;
}

double GetWeatherElev(long StationNumber, long NumObs)
{
     if(NumObs>MaxWeatherObs[StationNumber]-1)
     	NumObs=MaxWeatherObs[StationNumber]-1;

	return wtrdt[StationNumber][NumObs].el;
}


void GetWeatherRainTimes(long StationNumber, long NumObs, long *tr1, long *tr2)
{
     if(NumObs>MaxWeatherObs[StationNumber]-1)
     	NumObs=MaxWeatherObs[StationNumber]-1;

     *tr1=wtrdt[StationNumber][NumObs].tr1;
     *tr2=wtrdt[StationNumber][NumObs].tr2;
}


long GetMaxWeatherObs(long StationNumber)
{
	if(StationNumber>4)
     	return -1;

     return MaxWeatherObs[StationNumber]-1;
}


bool PreCalcMoistures(long YesNo)
{
	if(YesNo>-1)
     	PreCalcFuelMoistures=(bool) YesNo;

     return PreCalcFuelMoistures;
}


long GetMoistCalcInterval(long FM_SIZECLASS, long CATEGORY)
{
	return MoistCalcInterval[FM_SIZECLASS][CATEGORY];
}


void SetMoistCalcInterval(long FM_SIZECLASS, long CATEGORY, long Val)
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


//------------------------------------------------------------------------------
//
//  Weather/Wind units
//
//------------------------------------------------------------------------------

long WeatherUnits[5]={-1, -1, -1, -1, -1};
long WindUnits[5]={-1, -1, -1, -1, -1};

void	SetWindUnits(long StationNumber, long Units)
{
	WindUnits[StationNumber]=Units;
}

void SetWeatherUnits(long StationNumber, long Units)
{
	WeatherUnits[StationNumber]=Units;
}

long	GetWindUnits(long StationNumber)
{
	return WindUnits[StationNumber];
}

long GetWeatherUnits(long StationNumber)
{
	return WeatherUnits[StationNumber];
}




//------------------------------------------------------------------------------
//
//  Weather/Wind station grid functions                            s
//
//------------------------------------------------------------------------------

StationGrid::StationGrid()
{
	Grid=0;
}


StationGrid::~StationGrid()
{
	if(Grid)
     	delete[] Grid;
}


void AllocStationGrid(long XDim, long YDim)
{
	long x, y, Num;

	FreeStationGrid();
	size_t nmemb=2*XDim*YDim;
	grid.XDim=XDim;
	grid.YDim=YDim;
	grid.Width=((double) (GetHiEast()-GetLoEast()))/((double) XDim);
	grid.Height=((double) (GetHiNorth()-GetLoNorth()))/((double) YDim);
	grid.Grid=new long[nmemb];
	for(y=0; y<YDim; y++)
	{	for(x=0; x<XDim; x++)  // initialize grid to station 1
		{    Num=x+y*grid.XDim;
			grid.Grid[Num]=1;
		}
	}
}


long FreeStationGrid()
{
	if(grid.Grid)
	{	delete[] grid.Grid;
		grid.Grid=0;

		return 1;
	}
	grid.Grid=0;

	return 0;
}


long GetStationNumber(double xpt, double ypt)
{
	long XCell=(xpt-GetLoEast())/grid.Width;
	long YCell=(ypt-GetLoNorth())/grid.Height;
	long CellNum=XCell+YCell*grid.XDim;

	if(grid.Grid[CellNum]>NumWeatherStations)   // check to see if data exist
		return 0;             // for a given weather station
	if(grid.Grid[CellNum]>NumWindStations)
		return 0;

	return grid.Grid[CellNum];
}


long SetStationNumber(long XPos, long YPos, long StationNumber)
{
	long CellNumber=XPos+YPos*grid.XDim;
	if(StationNumber<=NumWeatherStations && StationNumber<=NumWindStations &&
		StationNumber>0)
		grid.Grid[CellNumber]=StationNumber;
	else
		return 0;

	return 1;
}


long GridInitialized()
{
	if(grid.Grid)
		return 1;

	return 0;
}


void SetGridEastDimension(long XDim)
{
	grid.XDim=XDim;
}


void SetGridNorthDimension(long YDim)
{
	grid.YDim=YDim;
}

long GetGridEastDimension()
{
	return grid.XDim;
}


long GetGridNorthDimension()
{
	return grid.YDim;
}


void SetGridNorthOffset(double offset)
{
	NorthGridOffset=offset;
}

void SetGridEastOffset(double offset)
{
	EastGridOffset=offset;
}

double GetGridNorthOffset()
{
	return NorthGridOffset;
}

double GetGridEastOffset()
{
	return EastGridOffset;
}




//------------------------------------------------------------------------------
//
//	Landscape header and cell positioning functions
//
//------------------------------------------------------------------------------

static long NumVals;
static bool CantAllocLCP=false;

void ReadHeader2()
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
	RasterCellResolutionX=(Header2.EastUtm-Header2.WestUtm)/(double) Header2.numeast;
	RasterCellResolutionY=(Header2.NorthUtm-Header2.SouthUtm)/(double) Header2.numnorth;
	ViewPortNorth=RasterCellResolutionY*(double) Header2.numnorth+Header2.lonorth;
	ViewPortSouth=Header2.lonorth;
	ViewPortEast=RasterCellResolutionX*(double) Header2.numeast+Header2.loeast;
	ViewPortWest=Header2.loeast;
//	NumViewNorth=(ViewPortNorth-ViewPortSouth)/Header.YResol;
//	NumViewEast=(ViewPortEast-ViewPortWest)/Header.XResol;
	double rows, cols;
	rows=(ViewPortNorth-ViewPortSouth)/Header2.YResol;
	NumViewNorth=rows;
	if(modf(rows, &rows)>0.5)
		NumViewNorth++;
	cols=(ViewPortEast-ViewPortWest)/Header2.XResol;
	NumViewEast=cols;
	if(modf(cols, &cols)>0.5)
		NumViewEast++;

     if(HaveCrownFuels())
     	NumVals=8;
     else
     	NumVals=5;
     CantAllocLCP=false;


}


void ReadHeader()
{
	fseek(landfile, 0, SEEK_SET);

//     unsigned int sizeh=sizeof(Header);
//	fread(&Header, sizeh, 1, landfile);

	fread(&Header.CrownFuels, sizeof(long), 1, landfile);
	fread(&Header.GroundFuels, sizeof(long), 1, landfile);
	fread(&Header.latitude, sizeof(long), 1, landfile);
	fread(&Header.loeast, sizeof(double), 1, landfile);
	fread(&Header.hieast, sizeof(double), 1, landfile);
	fread(&Header.lonorth, sizeof(double), 1, landfile);
	fread(&Header.hinorth, sizeof(double), 1, landfile);
     fread(&Header.loelev, sizeof(long), 1, landfile);
     fread(&Header.hielev, sizeof(long), 1, landfile);
     fread(&Header.numelev, sizeof(long), 1, landfile);
     fread(Header.elevs, sizeof(long), 100, landfile);
     fread(&Header.loslope, sizeof(long), 1, landfile);
     fread(&Header.hislope, sizeof(long), 1, landfile);
     fread(&Header.numslope, sizeof(long), 1, landfile);
     fread(Header.slopes, sizeof(long), 100, landfile);
     fread(&Header.loaspect, sizeof(long), 1, landfile);
     fread(&Header.hiaspect, sizeof(long), 1, landfile);
     fread(&Header.numaspect, sizeof(long), 1, landfile);
     fread(Header.aspects, sizeof(long), 100, landfile);
     fread(&Header.lofuel, sizeof(long), 1, landfile);
     fread(&Header.hifuel, sizeof(long), 1, landfile);
     fread(&Header.numfuel, sizeof(long), 1, landfile);
     fread(Header.fuels, sizeof(long), 100, landfile);
     fread(&Header.locover, sizeof(long), 1, landfile);
     fread(&Header.hicover, sizeof(long), 1, landfile);
     fread(&Header.numcover, sizeof(long), 1, landfile);
     fread(Header.covers, sizeof(long), 100, landfile);
     fread(&Header.loheight, sizeof(long), 1, landfile);
     fread(&Header.hiheight, sizeof(long), 1, landfile);
     fread(&Header.numheight, sizeof(long), 1, landfile);
     fread(Header.heights, sizeof(long), 100, landfile);
     fread(&Header.lobase, sizeof(long), 1, landfile);
     fread(&Header.hibase, sizeof(long), 1, landfile);
     fread(&Header.numbase, sizeof(long), 1, landfile);
     fread(Header.bases, sizeof(long), 100, landfile);
     fread(&Header.lodensity, sizeof(long), 1, landfile);
     fread(&Header.hidensity, sizeof(long), 1, landfile);
     fread(&Header.numdensity, sizeof(long), 1, landfile);
     fread(Header.densities, sizeof(long), 100, landfile);
     fread(&Header.loduff, sizeof(long), 1, landfile);
     fread(&Header.hiduff, sizeof(long), 1, landfile);
     fread(&Header.numduff, sizeof(long), 1, landfile);
     fread(Header.duffs, sizeof(long), 100, landfile);
     fread(&Header.lowoody, sizeof(long), 1, landfile);
     fread(&Header.hiwoody, sizeof(long), 1, landfile);
     fread(&Header.numwoody, sizeof(long), 1, landfile);
     fread(Header.woodies, sizeof(long), 100, landfile);
     fread(&Header.numeast, sizeof(long), 1, landfile);
     fread(&Header.numnorth, sizeof(long), 1, landfile);
     fread(&Header.EastUtm, sizeof(double), 1, landfile);
     fread(&Header.WestUtm, sizeof(double), 1, landfile);
     fread(&Header.NorthUtm, sizeof(double), 1, landfile);
     fread(&Header.SouthUtm, sizeof(double), 1, landfile);
     fread(&Header.GridUnits, sizeof(long), 1, landfile);
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
	RasterCellResolutionX=(Header.EastUtm-Header.WestUtm)/(double) Header.numeast;
	RasterCellResolutionY=(Header.NorthUtm-Header.SouthUtm)/(double) Header.numnorth;
	ViewPortNorth=RasterCellResolutionY*(double) Header.numnorth+Header.lonorth;
	ViewPortSouth=Header.lonorth;
	ViewPortEast=RasterCellResolutionX*(double) Header.numeast+Header.loeast;
	ViewPortWest=Header.loeast;
//	NumViewNorth=(ViewPortNorth-ViewPortSouth)/Header.YResol;
//	NumViewEast=(ViewPortEast-ViewPortWest)/Header.XResol;
	double rows, cols;
	rows=(ViewPortNorth-ViewPortSouth)/Header.YResol;
	NumViewNorth=rows;
	if(modf(rows, &rows)>0.5)
		NumViewNorth++;
	cols=(ViewPortEast-ViewPortWest)/Header.XResol;
	NumViewEast=cols;
	if(modf(cols, &cols)>0.5)
		NumViewEast++;

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
     CantAllocLCP=false;


     if(landscape==0)
     {    if(CantAllocLCP==false)
     	{	LONG64 i;
			LONG64 datasize;

     		fseek(landfile, headsize, SEEK_SET);
//     		if((landscape=(short *) calloc(Header.numnorth*Header.numeast, NumVals*sizeof(short)))!=NULL)
			//printf("got this far...");
			datasize=(LONG64) Header.numnorth*(LONG64) Header.numeast*(LONG64) NumVals;
			if((landscape=new short[datasize])!=NULL)
			{    memset(landscape, 0x0, datasize*sizeof(short));
          		for(i=0; i<Header.numnorth; i++)
          			fread(&landscape[i*(LONG64) NumVals*(LONG64) Header.numeast], sizeof(short),
                    		(LONG64) NumVals*(LONG64) Header.numeast, landfile);
          		fseek(landfile, headsize, SEEK_SET);
//     	     	OldFilePosition=0;     // thread local
               	CantAllocLCP=false;
          	}
          	else
			{	printf("can't allocate memory for landscape... aborting\n\n");          	
				CantAllocLCP=true;
			}
          }
     }
//	long p;
//   CellData(Header.loeast, Header.hinorth, &p);
}


bool ResampleLandscape(double NewRes)
{
	LONG64 i, j, xloc, yloc, NewNumNorth, NewNumEast, NumVals;
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
	memset(newlcp, 0x0, NewNumNorth*NewNumEast*NumVals*(LONG64) sizeof(short));

	for(i=0; i<NewNumNorth; i++)
	{	ypos=Header.NorthUtm-((double) i*NewRes)-NewRes/2.0;
		if(ypos<=Header.SouthUtm)
			break;
		yloc=(long) ((Header.NorthUtm-ypos)/Header.YResol);	// old position
		for(j=0; j<NewNumEast; j++)
		{	xpos=Header.WestUtm+((double) j*NewRes)+NewRes/2.0;	
			if(xpos>=Header.EastUtm)
				break;
			xloc=(long) ((xpos-Header.WestUtm)/Header.XResol);	// old position
			memcpy(&newlcp[(i*NewNumEast+j)*NumVals], 
				&landscape[(yloc*Header.numeast+xloc)*NumVals], 
				NumVals*sizeof(short));
		}
	}

	temp=landscape;
	delete[] landscape;
	landscape=newlcp;
	newlcp=temp;

	Header.numnorth=NewNumNorth;
	Header.numeast=NewNumEast;
	Header.SouthUtm=Header.NorthUtm-(double) (NewNumNorth)*NewRes;
	Header.EastUtm=Header.WestUtm+(double) (NewNumEast)*NewRes;
	Header.XResol=Header.YResol=NewRes;

	return true;
}



size_t GetHeadSize()
{
	return headsize;
}


void SetFilePosition(long position)
{
	OldFilePosition=position;
}


bool SetCellData(double x, double y, celldata *cell, crowndata *crown, grounddata *ground)
{
	short ldata[10];
     long posit;

     posit=GetCellPosition(x, y);

     if(!CantAllocLCP)
     {    CopyMemory(ldata, &landscape[posit*NumVals], NumVals*sizeof(short));

	     if(cell!=NULL)
          	CopyMemory(ldata, cell, 5*sizeof(short));
          if(crown!=NULL)
          	CopyMemory(&ldata[5], crown, 3*sizeof(short));
		if(ground!=NULL)
          	CopyMemory(&ldata[8], ground, 2*sizeof(short));

		CopyMemory(&landscape[posit*NumVals], ldata, NumVals*sizeof(short));
     }
     else
     {    celldata tcell;

        	CloseEditLandFile();
		OpenEditLandFile();
		fseek(GetLandFile(), GetHeadSize(), SEEK_SET);
		SetFilePosition(0);

     	if(Header.CrownFuels==20)
		{    if(Header.GroundFuels==20)
     		{   	fseek(landfile, (posit-OldFilePosition)*sizeof(celldata), SEEK_CUR);
                    if(cell!=NULL)
				{	fseek(landfile, -(posit-OldFilePosition)*sizeof(celldata), SEEK_CUR);
                         fwrite(cell, sizeof(celldata), 1, landfile);
               	}
	          }
     	     else
     		{   	fseek(landfile, (posit-OldFilePosition)*(sizeof(celldata)+sizeof(grounddata)), SEEK_CUR);
                    if(cell!=NULL && ground!=NULL)
                    {    fwrite(cell, sizeof(celldata), 1, landfile);
                         fwrite(ground, sizeof(grounddata), 1, landfile);
                    }
                    else if(cell!=NULL)
                         fwrite(cell, sizeof(celldata), 1, landfile);
                    else if(ground!=NULL)
                    {	fread(&tcell, sizeof(celldata), 1, landfile);
                         fwrite(ground, sizeof(grounddata), 1, landfile);
                    }
     	     }
		}
		else
		{    if(Header.GroundFuels==20)		// none
		     {	fseek(landfile, (posit-OldFilePosition)*(sizeof(celldata)+sizeof(crowndata)), SEEK_CUR);
                    if(cell!=NULL && crown!=NULL)
                    {    fwrite(cell, sizeof(celldata), 1, landfile);
                         fwrite(crown, sizeof(crowndata), 1, landfile);
                    }
                    else if(cell!=NULL)
                         fwrite(cell, sizeof(celldata), 1, landfile);
                    else if(crown!=NULL)
                    {	fread(&tcell, sizeof(celldata), 1, landfile);
                         fwrite(crown, sizeof(crowndata), 1, landfile);
                    }
     		}
	     	else
		     {	fseek(landfile, (posit-OldFilePosition)*(sizeof(celldata)+sizeof(crowndata)+
          	     	sizeof(grounddata)), SEEK_CUR);
                    if(cell!=NULL && crown!=NULL && ground!=NULL)
                    {    fwrite(cell, sizeof(celldata), 1, landfile);
                         fwrite(crown, sizeof(crowndata), 1, landfile);
                         fwrite(ground, sizeof(grounddata), 1, landfile);
                    }
                    else if(cell!=NULL && crown!=NULL)
                    {    fwrite(cell, sizeof(celldata), 1, landfile);
                         fwrite(crown, sizeof(crowndata), 1, landfile);
                    }
                    else if(cell!=NULL && ground!=NULL)
                    {    fwrite(cell, sizeof(celldata), 1, landfile);
                         fread(&tcell, sizeof(crowndata), 1, landfile);
                         fwrite(ground, sizeof(grounddata), 1, landfile);
                    }
                    else if(crown!=NULL && ground!=NULL)
                    {   	fread(&tcell, sizeof(celldata), 1, landfile);
                         fwrite(crown, sizeof(crowndata), 1, landfile);
                         fwrite(ground, sizeof(grounddata), 1, landfile);
                    }
                    else if(cell!=NULL)
                        fwrite(cell, sizeof(celldata), 1, landfile);
                    else if(crown!=NULL)
                    {   	fread(&tcell, sizeof(celldata), 1, landfile);
                         fwrite(crown, sizeof(crowndata), 1, landfile);
                    }
                    else if(ground!=NULL)
                    {   	fread(&tcell, sizeof(celldata), 1, landfile);
                         fread(&tcell, sizeof(crowndata), 1, landfile);
                         fwrite(ground, sizeof(grounddata), 1, landfile);
                    }
	     	}
          }

          CloseEditLandFile();
		ReOpenLandFile();    // doesent alloc memory
	}

     return true;
}


void GetCellDataFromMemory(long posit, celldata &cell, crowndata &cfuel, grounddata &gfuel)
{
	short ldata[10];

     CopyMemory(ldata, &landscape[posit*NumVals], NumVals*sizeof(short));
	switch(NumVals)
     {	case 5: 		// only 5 basic themes
			CopyMemory(&cell, ldata, NumVals*sizeof(short));
               break;
     	case 7:		// 5 basic and duff and woody
			CopyMemory(&cell, ldata, 5*sizeof(short));
               CopyMemory(&gfuel, &ldata[5], 2*sizeof(short));
               break;
          case 8:		// 5 basic and crown fuels
			CopyMemory(&cell, ldata, 5*sizeof(short));
               CopyMemory(&cfuel, &ldata[5], 3*sizeof(short));
               break;
          case 10:		// 5 basic, crown fuels, and duff and woody
			CopyMemory(&cell, ldata, 5*sizeof(short));
               CopyMemory(&cfuel, &ldata[5], 3*sizeof(short));
               CopyMemory(&gfuel, &ldata[8], 2*sizeof(short));
               break;
     }
}


void SetBinaryCellData(unsigned long row, unsigned long col, unsigned long num, short *data)
{
     long Posit;

     Posit=(row*Header.numeast+col);
     if(Posit+num>=Header.numeast*Header.numnorth)
     	num=Header.numeast*Header.numnorth-Posit-1;
     Posit*=NumVals;
     CopyMemory(&landscape[Posit], data, NumVals*num*sizeof(short));
}


bool GetBinaryCellData(unsigned long row, unsigned long col, unsigned long num, short *data)
{
     if(landscape==0)
     {    if(CantAllocLCP==false)
     	{	long i;

     		fseek(landfile, headsize, SEEK_SET);
     		if((landscape=new short[Header.numnorth*Header.numeast*NumVals])!=NULL)
          	{    ZeroMemory(landscape, Header.numnorth*Header.numeast*NumVals*sizeof(short));
               	for(i=0; i<Header.numnorth; i++)
          			fread(&landscape[i*NumVals*Header.numeast], sizeof(short),
                    		NumVals*Header.numeast, landfile);
          		fseek(landfile, headsize, SEEK_SET);
     	     	OldFilePosition=0;
               	CantAllocLCP=false;
          	}
          	else
          		CantAllocLCP=true;
          }
     }
     if(CantAllocLCP)
     	return false;

	long Posit;

     if(row<0)
     	row=0;
     else if(row>Header.numnorth-1)
     	row=Header.numnorth-1;
     if(col<0)
     	col=0;
     else if(col>Header.numeast-1)
     	col=Header.numeast-1;

     Posit=(row*Header.numeast+col);
     if(Posit+num>=Header.numeast*Header.numnorth)
     	num=Header.numeast*Header.numnorth-Posit-1;
     Posit*=NumVals;
     CopyMemory(data, &landscape[Posit], NumVals*num*sizeof(short));

     return true;
}


bool	SaveLCPMemoryToFile(char *NewName)
{
     FILE *otpfile;
     long i, j, posit;

     otpfile=fopen(NewName, "wb");
     fwrite(&Header, sizeof(headdata), 1, otpfile);
     for(i=0; i<Header.numnorth; i++)
     {	for(j=0; j<Header.numeast; j++)
     	{	posit=(i*Header.numeast+j)*NumVals;
		     fwrite(&landscape[posit], NumVals*sizeof(short), 1, otpfile);
          }
     }
     fclose(otpfile);

	return true;
}


bool	WriteAsciiLCPTheme(long ThemeNum, long Type, char *Name)
{
	FILE *otpfile;
     double MetersToKm=1.0, mult=1.0;
     long i, j, numcells, ThemeLoc=ThemeNum;
     unsigned long loc;

     if(CheckCellResUnits()==2)
     	MetersToKm=0.001;

     otpfile=fopen(Name, "w");
     switch(Type)
     {	case 1:	// grid
			fprintf(otpfile, "%s %ld\n", "NCOLS", Header.numeast);
			fprintf(otpfile, "%s %ld\n", "NROWS", Header.numnorth);
			fprintf(otpfile, "%s %lf\n", "XLLCORNER", Header.WestUtm*MetersToKm);
			fprintf(otpfile, "%s %lf\n", "YLLCORNER", Header.SouthUtm*MetersToKm);
			fprintf(otpfile, "%s %lf\n", "CELLSIZE", Header.XResol*MetersToKm);
			fprintf(otpfile, "%s %ld\n", "NODATA_VALUE", -1);
               break;
     	case 2:	// grass
               fprintf(otpfile, "%s    %lf\n", "north:", Header.NorthUtm*MetersToKm);
               fprintf(otpfile, "%s    %lf\n", "south:", Header.SouthUtm*MetersToKm);
			fprintf(otpfile, "%s     %lf\n", "east:", Header.EastUtm*MetersToKm);
               fprintf(otpfile, "%s     %lf\n", "west:", Header.WestUtm*MetersToKm);
               fprintf(otpfile, "%s     %ld\n", "rows:", Header.numnorth);
               fprintf(otpfile, "%s     %ld\n", "cols:", Header.numeast);
               break;
     }
	if(NumVals==7)
		ThemeLoc-=3;

	if(ThemeNum==5 || ThemeNum==6)	// crown height and base height
     {    switch(ThemeNum)
     	{	case 5: if(Header.HUnits==3 || Header.HUnits==4)
          			mult=10.0;
          		   break;
          	case 6: if(Header.BUnits==3 || Header.HUnits==4)
               		mult=10.0;
               	   break;

          }

     	for(i=0; i<Header.numnorth; i++)
	     {	for(j=0; j<Header.numeast; j++)
     		{    loc=(i*Header.numeast+j)*NumVals+ThemeLoc;
          		fprintf(otpfile, "%4.1lf ", landscape[loc]/mult);
	          }
     	     fprintf(otpfile, "\n");
	     }
     	fclose(otpfile);
     }
     else if(ThemeNum==7)	// bulk density
     {    if(Header.PUnits==3)
          	mult=100.0;
     	else if(Header.PUnits==4)
             	mult=1000.0;
     	for(i=0; i<Header.numnorth; i++)
	     {	for(j=0; j<Header.numeast; j++)
     		{    loc=(i*Header.numeast+j)*NumVals+ThemeLoc;
          		fprintf(otpfile, "%1.4lf ", landscape[loc]/mult);
	          }
     	     fprintf(otpfile, "\n");
	     }
     	fclose(otpfile);
     }
     else
     {	for(i=0; i<Header.numnorth; i++)
	     {	for(j=0; j<Header.numeast; j++)
     		{    loc=(i*Header.numeast+j)*NumVals+ThemeLoc;
          		fprintf(otpfile, "%d ", landscape[loc]);
	          }
     	     fprintf(otpfile, "\n");
	     }
     	fclose(otpfile);
     }

	return true;
}


celldata CellData(double east, double north, celldata &cell, crowndata &cfuel, grounddata &gfuel, long *posit)
{
     long Position;

     if(landscape==0)
     {    if(CantAllocLCP==false)
     	{	long i;

     		fseek(landfile, headsize, SEEK_SET);
//     		if((landscape=(short *) calloc(Header.numnorth*Header.numeast, NumVals*sizeof(short)))!=NULL)
			if((landscape=new short[Header.numnorth*Header.numeast*NumVals])!=NULL)
          	{    ZeroMemory(landscape, Header.numnorth*Header.numeast*NumVals*sizeof(short));
			    	for(i=0; i<Header.numnorth; i++)
          			fread(&landscape[i*NumVals*Header.numeast], sizeof(short),
                    		NumVals*Header.numeast, landfile);
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
			*posit=Position;

          return cell;
     }

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

	return cell;
}


long GetCellPosition(double east, double north)
{
	double xpt=(east-Header.loeast)/GetCellResolutionX();
	double ypt=(north-Header.lonorth)/GetCellResolutionY();
	long easti=((long) xpt);
	long northi=((long) ypt);
	northi=Header.numnorth-northi-1;
	if(northi<0)
		northi=0;
	long posit=(northi*Header.numeast+easti);

	return posit;
}


void SetVisibleTheme(long ThemeNumber)
{
	VisibleThemeNumber=ThemeNumber;
}

long GetVisibleTheme()
{
	if(!HaveCrownFuels() && VisibleThemeNumber>4)
     	VisibleThemeNumber=3;

	return VisibleThemeNumber;
}


//--------------------------------------------------------

headdata GetHeaderInformation()
{
	return Header;
}


void SetHeader(headdata *data)
{
	CopyMemory(&Header, data, sizeof(headdata));
}


long GetNumEast()
{
	return Header.numeast;
}


long GetNumNorth()
{
	return Header.numnorth;
}


double GetWestUtm()
{
	return Header.WestUtm;
}

double GetEastUtm()
{
	return Header.EastUtm;
}

double GetSouthUtm()
{
	return Header.SouthUtm;
}

double GetNorthUtm()
{
	return Header.NorthUtm;
}

double GetLoEast()
{
	return Header.loeast;
}
double GetHiEast()
{
	return Header.hieast;
}

double GetLoNorth()
{
	return Header.lonorth;
}

double GetHiNorth()
{
	return Header.hinorth;
}


long GetFuelCats(long ThemeNo, long *cats)
{
     long i, number;

     switch(ThemeNo)
     {    case 0: number=Header.numelev;
               memcpy(cats, Header.elevs, 100*sizeof(long));
               break;
          case 1: number=Header.numslope;
               memcpy(cats, Header.slopes, 100*sizeof(long));
               break;
          case 2: number=Header.numaspect;
               memcpy(cats, Header.aspects, 100*sizeof(long));
               break;
          case 3: number=Header.numfuel;
               memcpy(cats, Header.fuels, 100*sizeof(long));
               break;
          case 4: number=Header.numcover;
               memcpy(cats, Header.covers, 100*sizeof(long));
               break;
          case 5: number=Header.numheight;
               memcpy(cats, Header.heights, 100*sizeof(long));
               break;
          case 6: number=Header.numbase;
               memcpy(cats, Header.bases, 100*sizeof(long));
               break;
          case 7: number=Header.numdensity;
               memcpy(cats, Header.densities, 100*sizeof(long));
               break;
          case 8: number=Header.numduff;
               memcpy(cats, Header.duffs, 100*sizeof(long));
               break;
          case 9: number=Header.numwoody;
               memcpy(cats, Header.woodies, 100*sizeof(long));
               break;
     }

     return number;
}


double ConvertEastingOffsetToUtm(double input)
{
     double MetersToKm=1.0;
     double ipart;

     if(Header.GridUnits==2)
     	MetersToKm=0.001;

     modf(Header.WestUtm/1000.0, &ipart);

	return (input+ipart*1000.0)*MetersToKm;
}

double ConvertNorthingOffsetToUtm(double input)
{
	double MetersToKm=1.0;
     double ipart;

     if(Header.GridUnits==2)
     	MetersToKm=0.001;

     modf(Header.SouthUtm/1000.0, &ipart);

	return (input+ipart*1000.0)*MetersToKm;
}

double ConvertUtmToEastingOffset(double input)
{
	double KmToMeters=1.0;
     double ipart;

     if(Header.GridUnits==2)
     	KmToMeters=1000.0;

     modf(Header.WestUtm/1000.0, &ipart);

	return input*KmToMeters-ipart*1000.0;
}

double ConvertUtmToNorthingOffset(double input)
{
	double KmToMeters=1.0;
     double ipart;

     if(Header.GridUnits==2)
     	KmToMeters=1000.0;

     modf(Header.SouthUtm/1000.0, &ipart);

	return input*KmToMeters-ipart*1000.0;
}


char *GetHeaderDescription()
{
	return Header.Description;
}

//-----------------------------------------------------------------------------

long GetLoElev()
{
	return Header.loelev;
}

long GetHiElev()
{
	return Header.hielev;
}


long	GetConditMonth()
{
	return conditmonth;
}


long	GetConditDay()
{
	return conditday;
}


long GetConditMinDeficit()
{
     if(CondPeriod==false)
     	return 0;

	long CondMin;
     long StartMin;

     CondMin=(GetJulianDays(conditmonth)+conditday)*1440;// 00:01 on this day
     StartMin=(GetJulianDays(startmonth)+startday)*1440+(starthour/100*60)+startmin;
     if(StartMin<CondMin)
     	StartMin+=365*1440;

     return StartMin-CondMin;
}


void SetConditMonth(long input)
{
	conditmonth=input;
}


void SetConditDay(long input)
{
	conditday=input;
}


long GetLatitude()
{
	return Header.latitude;
}

long GetStartMonth()
{
	return startmonth;
}

long GetStartDay()
{
	return startday;
}

long GetStartHour()
{
	return starthour;
}


long GetStartMin()
{
	return startmin;
}

long GetStartDate()
{
	return startdate;
}


long GetMaxMonth()
{
	long maxmonth=LastMonth[0].all;
	long numstations=GetNumStations();

	for(long i=0; i<numstations; i++)
	{	if(LastMonth[i].all<maxmonth)
			maxmonth=LastMonth[i].all;
	}

	return maxmonth;
}

long GetMaxDay()
{
     long i;
	long maxday=LastDay[0].all;
	long numstations=GetNumStations();
	long maxmonth=GetMaxMonth();

	for(i=0; i<numstations; i++)
	{    if(LastMonth[i].all==maxmonth && maxday>LastDay[i].all)
			maxday=LastDay[i].all;
	}

	return maxday;
}

long GetMaxHour()
{
     long i;
	long maxhour=LastHour[0].all;
	long maxmonth=GetMaxMonth();
	long maxday=GetMaxDay();
	long numstations=GetNumStations();

	for(i=0; i<numstations; i++)
	{	if(LastMonth[i].all==maxmonth && LastDay[i].all==maxday)
		{	if(LastHour[i].all<maxhour)
				maxhour=LastHour[i].all;
		}
	}

	return maxhour;
}


long GetMinMonth()
{
     long i;
	long minmonth=FirstMonth[0].all;
	long NStat=GetNumStations();

	for(i=0; i<NStat; i++)
	{    if(FirstMonth[i].all>minmonth)
			minmonth=FirstMonth[i].all;
	}

	return minmonth;
}


long GetMinDay()
{
     long i;
	long minday=FirstDay[0].all;
	long NStat=GetNumStations();
	long minmonth=GetMinMonth();

	for(i=0; i<NStat; i++)
	{    if(FirstMonth[i].all==minmonth)
		{	if(FirstDay[i].all>minday)
				minday=FirstDay[i].all;
		}
	}

	return minday;
}


long GetMinHour()
{
     long i;
	long minhour=FirstHour[0].all;
	long NStat=GetNumStations();
	long minmonth=GetMinMonth();
	long minday=GetMinDay();

	for(i=0; i<NStat; i++)
	{    if(LastMonth[i].all==minmonth && LastDay[i].all==minday)
		{	if(FirstHour[i].all>minhour)
				minhour=FirstHour[i].all;
		}
	}

	return minhour;
}


//-------

long IgnitionResetAtRestart(long YesNo)
{
	if(YesNo>=0)
		IgnitionReset=YesNo;

	return IgnitionReset;
}

long RotateSensitiveIgnitions(long YesNo)
{
	if(YesNo>=0)
		RotateIgnitions=YesNo;

	return RotateIgnitions;
}


long ShowFiresAsGrown(long YesNo)
{
     if(YesNo>=0)
          ShowVectsAsGrown=YesNo;

     return ShowVectsAsGrown;
}


long DurationResetAtRestart(long YesNo)
{
	if(YesNo>=0)
		DurationReset=YesNo;

	return DurationReset;
}


bool PreserveInactiveEnclaves(long YesNo)
{
     if(YesNo>=0)
          InactiveEnclaves=(bool) YesNo;

     return InactiveEnclaves;
}


void SetStartMonth(long input)
{
	startmonth=input;
}

void SetStartDay(long input)
{
	startday=input;
}

void SetStartHour(long input)
{
	starthour=input;
}

void SetStartMin(long input)
{
	startmin=input;
}

void SetStartDate(long input)
{
	startdate=input;
}

long GetEndMonth()
{
	return endmonth;
}

long GetEndDay()
{
	return endday;
}

long GetEndHour()
{
	return endhour;
}

long GetEndDate()
{
	return enddate;
}

long GetEndMin()
{
	return endmin;
}

void SetEndMonth(long input)
{
	endmonth=input;
}

void SetEndDay(long input)
{
	endday=input;
}

void SetEndHour(long input)
{
	endhour=input;
}

void	SetEndMin(long input)
{
	endmin=input;
}

void SetEndDate(long input)
{
	enddate=input;
}

double GetSimulationDuration()
{
	return SimulationDuration;
}

void SetSimulationDuration(double simdur)
{
	SimulationDuration=simdur;
}


bool	UseConditioningPeriod(long YesNo)
{
	if(YesNo>-1)
          CondPeriod=(bool) YesNo;

	return CondPeriod;
}


bool EnvironmentChanged(long YesNo, long StationNumber, long FuelSize)
{
	if(FuelSize>3)
     	FuelSize=3;
	if(YesNo>-1)
     	EnvtChanged[FuelSize][StationNumber]=(bool) YesNo;

     return EnvtChanged[FuelSize][StationNumber];
}


FILE *GetLandFile()
{
	return landfile;
}


bool OpenLandFile()
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


bool ReOpenLandFile()
{
	if(landfile)
     	fclose(landfile);
     //if(landscape)
     //{	GlobalFree(landscape);//free(landscape);
     //     landscape=0;
     //	CantAllocLCP=false;
     //}
	if((landfile=fopen(LandFName, "rb"))==NULL)
	{    landfile=0;

		return false;
     }
	fseek(landfile, headsize, SEEK_SET);
     OldFilePosition=0;

	return true;
}


bool OpenEditLandFile()
{
	if(landfile)
     	fclose(landfile);
	//if(landscape)
     //{	GlobalFree(landscape);//free(landscape);
     //     landscape=0;
     //}
    	CantAllocLCP=true;
	if((landfile=fopen(LandFName, "r+b"))!=NULL)
		return true;
     else
     {    SetFileAttributes(reinterpret_cast<LPCSTR>(LandFName), FILE_ATTRIBUTE_NORMAL);
          landfile=fopen(LandFName, "r+b");
     }

	return true;
}


void CloseLandFile()
{
     if(landscape)
     {	delete[] landscape;//GlobalFree(landscape);//free(landscape);
          landscape=0;
     }
    	CantAllocLCP=false;
	if(landfile)
		fclose(landfile);
     memset(LandFName, 0x0, sizeof LandFName);
	landfile=0;
	CanopyChx.Height=CanopyChx.DefaultHeight;
	CanopyChx.CrownBase=CanopyChx.DefaultBase;
	CanopyChx.BulkDensity=CanopyChx.DefaultDensity;
}

void CloseEditLandFile()
{
	if(landfile)
     	fclose(landfile);
    	CantAllocLCP=false;
     landfile=0;
     //lcptheme->ReadStats();
}


bool TestForLCPVersion1()
{
	long TestCrownFuelValue;

     rewind(landfile);
	fread(&TestCrownFuelValue, sizeof(long), 1, landfile);
	if(TestCrownFuelValue<10)     // v1 LCP file had crown fuel indicator as 0 or 1
		return true;			// v2 LCP file has crown fuel indicator as 10 or 11

	return false;
}


bool TestForLCPVersion2()
{
	long TestCrownFuelValue;

     rewind(landfile);
	fread(&TestCrownFuelValue, sizeof(long), 1, landfile);
	if(TestCrownFuelValue>9 && TestCrownFuelValue<20)     // v1 LCP file had crown fuel indicator as 0 or 1
		return true;							    // v2 LCP file has crown fuel indicator as 10 or 11

	return false;
}



char *GetLandFileName()
{
	return LandFName;
}

void SetLandFileName(char* FileName)
{
	memset(LandFName, 0x0, sizeof LandFName);
	sprintf(LandFName, "%s", FileName);
}

void SetLCPFileNameJustMade(char *FileName)
{
     memset(LCPJustMade, 0x0, sizeof(LCPJustMade));
	sprintf(LCPJustMade, "%s", FileName);
}

char *GetLCPFileNameJustMade()
{
	return LCPJustMade;
}



//-------------------------------------------------------------------------------
//		 Access to Fire Display Functions
//-------------------------------------------------------------------------------

double MinFireChx(double Type)
{// max display fire chx
	if(Type>=0.0)
		FireMinChx=Type;

	return FireMinChx;
}


double MaxFireChx(double Type)
{// max display fire chx
	if(Type>=0.0)
		FireMaxChx=Type;

	return FireMaxChx;
}


long NumFireInterval(long Num)
{// num of fire intervals displayed
	if(Num>0)
		FireNumInterval=Num;

	return FireNumInterval;
}


long OutputFireParam(long Param)
{// select fire chx for display
	if(Param>=0)
		FireOutputParam=Param;

	return FireOutputParam;
}




//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//
//	Atmosphere Grid Global Functions
//
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------


bool SetAtmosphereGrid(long NumGrids)
{
	if(NumGrids==0)
     {    /*
     	if(AtmGrid->AtmGridWTR)
     	{	NumWeatherStations=0;    // reset number of weather stations
          	NumWindStations=0;       // reset number of wind stations
          }
          else if(AtmGrid->AtmGridWND)
          	NumWindStations=0;       // reset number of wind stations
          */
     	if(AtmGrid)
          	delete AtmGrid;
          AtmGrid=0;
		NorthGridOffset=0.0;
		EastGridOffset=0.0;

          return false;
     }
	if(AtmGrid==0)
     	AtmGrid=new AtmosphereGrid(NumGrids);
     else
     {	delete AtmGrid;
      	AtmGrid=new AtmosphereGrid(NumGrids);
     }

     return true;
}


AtmosphereGrid* GetAtmosphereGrid()
{
	return AtmGrid;
}

long AtmosphereGridExists()
{
	if(AtmGrid)
     {	if(AtmGrid->AtmGridWTR)
     	    	return 2;
          else if(AtmGrid->AtmGridWND)
			return 1;
     }

     return 0;
}


//-------------------------------------------------------------------------------
//	BurnPeriod Data and Functions
//-------------------------------------------------------------------------------

static double DownTime=0.0;
static long NumAbsoluteData=0;
static long NumRelativeData=0;
static long LastAccess=-1;
static AbsoluteBurnPeriod *abp=0;
static RelativeBurnPeriod *rbp=0;


bool AllocBurnPeriod(long Num)
{
	FreeBurnPeriod();

     long i, FirstDay, StartDay, EndDay, NumDays, LastDay;
     long mo, dy, maxdy;

     FirstDay=GetJulianDays(GetMinMonth());
     FirstDay+=GetMinDay();
     LastDay=GetJulianDays(GetMaxMonth());
     LastDay+=GetMaxDay();
     NumDays=LastDay-FirstDay;
     if(Num<NumDays)
          Num=NumDays;

     abp=new AbsoluteBurnPeriod[Num];
	if(abp==NULL)
     	return false;
     memset(abp, 0x0, Num*sizeof(AbsoluteBurnPeriod));
     NumAbsoluteData=Num;

     mo=GetMinMonth();
     dy=GetMinDay();
     for(i=0; i<NumDays; i++)
     {    abp[i].Month=mo;
          abp[i].Day=dy;
          abp[i].Start=0;
          abp[i].End=2400;
          dy++;
          if(mo<12)
               maxdy=GetJulianDays(mo+1)-GetJulianDays(mo);
          else
               maxdy=31;
          if(dy>maxdy)
          {    mo++;
               if(mo>12)
                    mo=1;
               dy=1;
          }
     }

	return true;
}


void FreeBurnPeriod()
{
	NumRelativeData=0;
     NumAbsoluteData=0;
     if(abp)
     	delete[] abp;	//free(abp);
     abp=0;
     if(rbp)
     	delete[] rbp;	//free(rbp);
     rbp=0;
     DownTime=0.0;
     LastAccess=-1;
}


void SetBurnPeriod(long num, long mo, long dy, long start, long end)
{
	if(abp)
     {    for(i=0; i<NumAbsoluteData; i++)
          {    num=i;
               if(abp[i].Month==mo && abp[i].Day==dy)
                    break;
          }
          abp[num].Month=mo;
     	abp[num].Day=dy;
          abp[num].Start=start;
          abp[num].End=end;
     }
}


bool GetBurnPeriod(long num, long *mo, long *dy, long *start, long *end)
{
	if(abp)
     {	*mo=abp[num].Month;
     	*dy=abp[num].Day;
          *start=abp[num].Start;
          *end=abp[num].End;
     }
     else
	{     *mo=*dy=*start=*end=0;

          return false;
     }

     return true;
}


long GetNumBurnPeriods()
{
	return NumAbsoluteData;
}


bool InquireInBurnPeriod(double SimTime)
{
     if(!rbp)
     	return true;

	for(i=LastAccess+1; i<NumRelativeData; i++)
     {	if(SimTime>=rbp[i].Start)
     	{	if(SimTime<=rbp[i].End)
          	{	LastAccess=i-1;

               	return true;
               }
          }
          else
          	break;
     }

     return false;
}

long GetJulianDays(long Month)
{
     long days;

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


double ConvertActualTimeToSimtime(long mo, long dy, long hr, long mn, bool FromCondit)
{
	double SimTime, RefStart;

	if(!FromCondit)
     {    if(GetStartDate()==0)
     	{	SimTime=-1.0;
           	return SimTime;
          }
		RefStart=(GetJulianDays(startmonth)+startday)*1440.0+starthour/100.0*60.0+startmin;
     }
     else
     {    if(conditmonth==0 && conditday==0)
     	{	SimTime=-1.0;
           	return SimTime;
          }
		RefStart=(GetJulianDays(conditmonth)+conditday)*1440.0;
     }

	if(!FromCondit)
     {  	if(mo>=startmonth)
			SimTime=(GetJulianDays(mo)+dy)*1440.0+hr/100.0*60.0+mn;
         	else
          	SimTime=(GetJulianDays(mo)+dy+365.0)*1440.0+hr/100.0*60.0+mn;
     }
     else
     {  	if(mo>=conditmonth)
			SimTime=(GetJulianDays(mo)+dy)*1440.0+hr/100.0*60.0+mn;
         	else
          	SimTime=(GetJulianDays(mo)+dy+365.0)*1440.0+hr/100.0*60.0+mn;
     }
     SimTime-=RefStart;

     return SimTime;
}


void ConvertSimtimeToActualTime(double SimTime, long *mo, long *dy, long *hr, long *mn, bool FromCondit)
{
	long months, days, hours, mins;
     double fday, fhour;

     fday=(SimTime/1440.0);	// days from minutes
     days=(long) fday;
     fhour=(fday-days)*24.0;  // hours from days
     hours=(long) fhour;
     mins=(fhour-hours)*60;	// minutes from hours

     if(!FromCondit)
	{    hours+=GetStartHour()/100;
          mins+=GetStartMin();
     }
     if(mins>60)
     {    mins-=60;
          hours++;
     }
     if(hours>=24)
     {	hours-=24;
     	days++;
     }
     hours*=100;
	if(!FromCondit)
	{    if(GetStartDate()+days>365)
	     {    days=-365;
     		for(i=1; i<12; i++)
          	{	if(days<GetJulianDays(i))
          			break;
	          }
     	     days-=GetJulianDays(i);
          	months=i;
	     }
     	else
	     {    for(i=1; i<12; i++)
		     {	if(days+GetStartDate()<GetJulianDays(GetStartMonth()+i))
          	   		break;
	          }
     	     days-=(GetJulianDays(GetStartMonth()+i-1)-GetStartDate());
          	         months=GetStartMonth()+(i-1);
		}
     }
     else
     {    long ConDate=GetJulianDays(conditmonth)+conditday;

     	if(ConDate+days>365)
	     {    days=-365;
     		for(i=1; i<12; i++)
          	{	if(days<GetJulianDays(i))
          			break;
	          }
     	     days-=GetJulianDays(i);
          	months=i;
	     }
     	else
	     {    for(i=1; i<12; i++)
		     {	if(days+ConDate<GetJulianDays(conditmonth+i))
          	   		break;
	          }
     	     days-=(GetJulianDays(conditmonth+i-1)-ConDate);
          	         months=conditmonth+(i-1);
		}
     }

     *mo=months;
     *dy=days;
     *hr=hours;
     *mn=mins;
}


void ConvertAbsoluteToRelativeBurnPeriod()
{
     long i, j, days, Day, Month, NumAlloc;
     double Add, Begin, End, Start, hrs, mins;
     RelativeBurnPeriod *trbp;

	if(abp==0)
     	return;
     if(rbp)
     	delete[] rbp;//GlobalFree(rbp);//free(rbp);

     Start=GetJulianDays(startmonth);
     Start+=startday-1;
     End=GetJulianDays(endmonth);
     End+=endday;
     if(End<Start)
     	NumRelativeData=End+365-Start;
     else
	  	NumRelativeData=End-Start;

     NumAlloc=NumAbsoluteData;
     if(NumRelativeData>NumAbsoluteData)
          NumAlloc=NumRelativeData;

     rbp=new RelativeBurnPeriod[NumAlloc];
	memset(rbp, 0x0, NumAlloc*sizeof(RelativeBurnPeriod));
     trbp= new RelativeBurnPeriod[NumAlloc];
	memset(trbp, 0x0, NumAlloc*sizeof(RelativeBurnPeriod));

      if(NumRelativeData>NumAbsoluteData)
     {    AbsoluteBurnPeriod *tabp=new AbsoluteBurnPeriod[NumRelativeData];
          CopyMemory(tabp, abp, NumAbsoluteData*sizeof(AbsoluteBurnPeriod));
          delete[] abp;
          abp=new AbsoluteBurnPeriod[NumRelativeData];
          CopyMemory(abp, tabp, NumAbsoluteData*sizeof(AbsoluteBurnPeriod));
          delete[] tabp;

          Month=abp[NumAbsoluteData-1].Month;
          Day=abp[NumAbsoluteData-1].Day;
          for(i=NumAbsoluteData; i<NumRelativeData; i++)
          {    Day++;
               if(Month<12)
               {    if(Day>GetJulianDays(Month+1)-GetJulianDays(Month))
                    {    Day=1;
                         Month++;
                    }
               }
               else
               {    if(Day>31)
                    {    Day=1;
                         Month=1;
                    }
               }
               abp[i].Month=Month;
               abp[i].Day=Day;
               abp[i].Start=0;
               abp[i].End=2400;
          }
          NumAbsoluteData=NumRelativeData;
     }

     rbp[0].Start=0.0;//starthour*0.6;
     rbp[0].End=1440-starthour*0.6;
     for(i=1; i<NumRelativeData; i++)            // initialize all data to minutes
     {	rbp[i].Start=rbp[i-1].End;
     	rbp[i].End=1440*(i+1)-starthour*0.6;
     }

     days=GetJulianDays(abp[0].Month);
     days+=abp[0].Day-1;
     End=Start=days*1440;
     mins=modf(abp[0].Start/100.0, &hrs);
     Start+=(hrs*60.0+mins);
     mins=modf(abp[0].End/100.0, &hrs);
     End+=(hrs*60.0+mins);
     End-=Start;

     trbp[0].Start=0;
     trbp[0].End=End;

     for(i=1; i<NumAbsoluteData; i++)
     {    days=GetJulianDays(abp[i].Month);
          days+=abp[i].Day-1;
     	End=Begin=days*1440;
     	mins=modf(abp[i].Start/100.0, &hrs);
     	Begin+=(hrs*60.0+mins);
     	mins=modf(abp[i].End/100.0, &hrs);
     	End+=(hrs*60.0+mins);
          Begin-=Start;
          End-=Start;

     	trbp[i].Start=Begin;
     	trbp[i].End=End;
     }

 	days=GetJulianDays(startmonth);
     days+=startday-1;
     Begin=days*1440;
     mins=modf(starthour/100.0, &hrs);
     Begin+=(hrs*60.0+mins);            // beginning of simulation time

     if(trbp[NumAbsoluteData-1].End+Start<Begin)	  // if after new year
     	Add=Begin-(Start+(365*1440)); // start of data af
     else
     	Add=Start-Begin;   // start of data earlier than begin of simulation

     for(i=0; i<NumAbsoluteData; i++)
     {	trbp[i].Start+=Add;
     	trbp[i].End+=Add;
     }

     long FirstDay, StartDay, EndDay, NumDays;// LastDay;

     FirstDay=GetJulianDays(GetMinMonth());
     FirstDay+=GetMinDay();
//     LastDay=GetJulianDays(GetMaxMonth());
//     LastDay+=GetMaxDay();

     StartDay=GetJulianDays(GetStartMonth());
	StartDay+=GetStartDay();
     EndDay=GetJulianDays(GetEndMonth());
	EndDay+=GetEndDay();

     NumDays=EndDay-StartDay;

     for(i=0; i<=NumDays; i++)
     {	j=i+StartDay-FirstDay;
     	if(i<NumAlloc && j<NumAlloc)
	     	rbp[i]=trbp[j];
     }

     /*
     for(i=0; i<NumAbsoluteData; i++)
     {	for(j=0; j<NumRelativeData; j++)
     	{    if(trbp[i].Start>=rbp[j].Start && trbp[i].End<=rbp[j].End)
          		rbp[j]=trbp[i];
          }
     }
     */

     LastAccess=-1;
//   DownTime=0.0;
     delete[] trbp;//GlobalFree(trbp);//free(trbp);
}

void AddDownTime(double time)
{
	if(time<0)
     	DownTime=0.0;
     else
     	DownTime+=time;
}

double GetDownTime()
{
	return DownTime;
}


//-------------------------------------------------------------------------------
// Fuel Moisture Data and Functions
//-------------------------------------------------------------------------------

static InitialFuelMoisture fm[257];

bool SetInitialFuelMoistures(long Model, long t1, long t10, long t100, long tlh, long tlw)
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

     long i, j;
     for(i=0; i<3; i++)       // only up to 100 hr fuels [3]
     {    for(j=0; j<5; j++)
               EnvtChanged[i][j]=true;
     }

     return fm[Model-1].FuelMoistureIsHere;
}

bool GetInitialFuelMoistures(long Model, long *t1, long *t10, long *t100, long *tlh, long *tlw)
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

long GetInitialFuelMoisture(long Model, long FuelClass)
{
     if(Model>256 || Model<1)
     	return 2;

	long mx;

	switch(FuelClass)
     {	case 0: mx=fm[Model-1].TL1; break;
     	case 1: mx=fm[Model-1].TL10; break;
          case 2: mx=fm[Model-1].TL100; break;
          case 3: mx=fm[Model-1].TLLH; break;
          case 4: mx=fm[Model-1].TLLW; break;
     }

     return mx;
}

bool InitialFuelMoistureIsHere(long Model)
{
     if(Model>256 || Model<1)
     	return false;

	return fm[Model-1].FuelMoistureIsHere;
}


//-------------------------------------------------------------------------------
//	Coarse Woody Data and Functions
//-------------------------------------------------------------------------------

CoarseWoody coarsewoody[MAXNUM_COARSEWOODY_MODELS];
static double WeightLossErrorTol=1.0;	// Mg/ha
CoarseWoody NFFLWoody[13];
CoarseWoody tempwoody;
static long CombineOption=CWD_COMBINE_ABSENT;


CoarseWoody::CoarseWoody()
{
	wd=0;
     NumClasses=0;
     Units=-1;
     TotalWeight=0.0;
     Depth=0.0;
}


bool AllocCoarseWoody(long ModelNumber, long NumClasses)
{
     if(coarsewoody[ModelNumber-1].wd)
     	FreeCoarseWoody(ModelNumber);
	if((coarsewoody[ModelNumber-1].wd=new WoodyData[NumClasses])==NULL) //(WoodyData *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, NumClasses*sizeof(WoodyData)))==NULL)
     	return false;
	ZeroMemory(coarsewoody[ModelNumber-1].wd, NumClasses*sizeof(WoodyData));
	coarsewoody[ModelNumber-1].NumClasses=NumClasses;
     coarsewoody[ModelNumber-1].TotalWeight=0.0;
     ZeroMemory(coarsewoody[ModelNumber-1].Description, 64*sizeof(char));

	return true;
}


void FreeCoarseWoody(long ModelNumber)
{
	if(coarsewoody[ModelNumber-1].wd)
     	delete[] coarsewoody[ModelNumber-1].wd;//GlobalFree(coarsewoody[ModelNumber-1].wd);//free(coarsewoody[ModelNumber-1].wd);
    	coarsewoody[ModelNumber-1].NumClasses=0;
     coarsewoody[ModelNumber-1].wd=0;
     coarsewoody[ModelNumber-1].Units=-1;
     coarsewoody[ModelNumber-1].TotalWeight=0.0;
     coarsewoody[ModelNumber-1].Depth=0.0;
}

void FreeAllCoarseWoody()
{
	long i;

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

bool SetWoodyData(long ModelNumber, long ClassNumber, WoodyData *wd, long units)
{
     if(ModelNumber>MAXNUM_COARSEWOODY_MODELS-1)
     	return false;

	if(coarsewoody[ModelNumber-1].NumClasses<=ClassNumber)
          return false;

     //long i;
     double Total=0.0;

     //coarsewoody[ModelNumber-1].TotalWeight=0.0;
     CopyMemory(&coarsewoody[ModelNumber-1].wd[ClassNumber], wd, sizeof(WoodyData));
     coarsewoody[ModelNumber-1].Units=units;
     //for(i=0; i<=ClassNumber; i++)
    	Total=coarsewoody[ModelNumber-1].TotalWeight+wd[0].Load;//coarsewoody[ModelNumber-1].wd[i];
     coarsewoody[ModelNumber-1].TotalWeight=Total;
     SetNFFLWoody();
	EnvtChanged[3][0]=true;
	EnvtChanged[3][1]=true;
	EnvtChanged[3][2]=true;
	EnvtChanged[3][3]=true;
	EnvtChanged[3][4]=true;

     return true;
}

bool SetWoodyDataDepth(long ModelNumber, double depth, char *Description)
{
     if(ModelNumber>MAXNUM_COARSEWOODY_MODELS-1)
     	return false;

     coarsewoody[ModelNumber-1].Depth=depth;
     CopyMemory(coarsewoody[ModelNumber-1].Description, Description, 64*sizeof(char));

	return true;
}


bool SetNFFLWoody()
{
	long i, j, k;

	if(NFFLWoody[0].wd)
     	return true;

     for(i=0; i<256; i++)
     {    if(NFFLWoody[i].wd)
	     	delete[] NFFLWoody[i].wd;
          NFFLWoody[i].NumClasses=0;
     	NFFLWoody[i].wd=0;
     }

	for(i=0; i<256; i++)
     {    if(newfuels[i+1].number>0)
     	{   	NFFLWoody[i].NumClasses=0;
          	if(newfuels[i+1].h1>0.0)
	              	NFFLWoody[i].NumClasses++;
			if(newfuels[i+1].h10>0.0)
	              	NFFLWoody[i].NumClasses++;
			if(newfuels[i+1].h100>0.0)
	              	NFFLWoody[i].NumClasses++;
			if(newfuels[i+1].lh>0.0)
	              	NFFLWoody[i].NumClasses++;
			if(newfuels[i+1].lw>0.0)
	              	NFFLWoody[i].NumClasses++;
		     if((NFFLWoody[i].wd=new WoodyData[NFFLWoody[i].NumClasses])==NULL)
     			return false;
               ZeroMemory(NFFLWoody[i].wd, NFFLWoody[i].NumClasses*sizeof(WoodyData));
     		k=0;
	          if(newfuels[i+1].h1>0.0)
	          {   	NFFLWoody[i].wd[k].Load=newfuels[i+1].h1;
	              	NFFLWoody[i].wd[k++].Load=newfuels[i+1].sav1;
               }
			if(newfuels[i+1].h10>0.0)
	          {   	NFFLWoody[i].wd[k].Load=newfuels[i+1].h10;
	              	NFFLWoody[i].wd[k++].Load=109.0;
               }
			if(newfuels[i+1].h100>0.0)
	          {   	NFFLWoody[i].wd[k].Load=newfuels[i+1].h100;
	              	NFFLWoody[i].wd[k++].Load=30.0;
               }
			if(newfuels[i+1].lh>0.0)
	          {   	NFFLWoody[i].wd[k].Load=newfuels[i+1].lh;
	              	NFFLWoody[i].wd[k++].Load=newfuels[i+1].savlh;
               }
			if(newfuels[i+1].lw>0.0)
	          {   	NFFLWoody[i].wd[k].Load=newfuels[i+1].lw;
	              	NFFLWoody[i].wd[k++].Load=newfuels[i+1].savlw;
               }
          }
		else
          {	NFFLWoody[i].NumClasses=0;
	          NFFLWoody[i].Depth=0.0;
     	     NFFLWoody[i].TotalWeight=0.0;
			NFFLWoody[i].Units=1;
	          NFFLWoody[i].wd=0;
          }
     }


//	if((tempwoody.wd=(WoodyData *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, 20*sizeof(WoodyData)))==NULL)
//     	return false;

/*
// fuel model 1
	NFFLWoody[i].NumClasses=3;
     NFFLWoody[i].Depth=1.0;
     NFFLWoody[i].Units=1;
//     if((NFFLWoody[i].wd=(WoodyData *) calloc(NFFLWoody[i].NumClasses, sizeof(WoodyData)))==NULL)
     if((NFFLWoody[i].wd=new WoodyData[NFFLWoody[i].NumClasses])==NULL)
     	return false;
     ZeroMemory(NFFLWoody[i].wd, NFFLWoody[i].NumClasses*sizeof(WoodyData));
     NFFLWoody[i].wd[0].SurfaceAreaToVolume=3500.0;
     NFFLWoody[i].wd[0].Load=0.74;
     NFFLWoody[i].wd[1].SurfaceAreaToVolume=109.0;
     NFFLWoody[i].wd[1].Load=0.0;
     NFFLWoody[i].wd[2].SurfaceAreaToVolume=30.0;
     NFFLWoody[i++].wd[2].Load=0.0;

// fuel model 2
	NFFLWoody[i].NumClasses=4;
     NFFLWoody[i].Depth=1.0;
     NFFLWoody[i].Units=1;
//     if((NFFLWoody[1].wd=(WoodyData *) calloc(NFFLWoody[i].NumClasses, sizeof(WoodyData)))==NULL)
     if((NFFLWoody[1].wd=new WoodyData[NFFLWoody[i].NumClasses])==NULL)
     	return false;
//     if((NFFLWoody[1].wd=(WoodyData *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, NFFLWoody[i].NumClasses*sizeof(WoodyData)))==NULL)
     ZeroMemory(NFFLWoody[1].wd, NFFLWoody[i].NumClasses*sizeof(WoodyData));
     NFFLWoody[i].wd[0].SurfaceAreaToVolume=3000.0;
     NFFLWoody[i].wd[0].Load=2.00;
     NFFLWoody[i].wd[1].SurfaceAreaToVolume=109.0;
     NFFLWoody[i].wd[1].Load=1.00;
     NFFLWoody[i].wd[2].SurfaceAreaToVolume=30.0;
     NFFLWoody[i].wd[2].Load=0.50;
     NFFLWoody[i].wd[3].SurfaceAreaToVolume=1500.0;
     NFFLWoody[i++].wd[3].Load=0.50;

// fuel model 3
	NFFLWoody[i].NumClasses=3;
     NFFLWoody[i].Depth=2.5;
     NFFLWoody[i].Units=1;
//     if((NFFLWoody[2].wd=(WoodyData *) calloc(NFFLWoody[i].NumClasses, sizeof(WoodyData)))==NULL)
//     if((NFFLWoody[2].wd=(WoodyData *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, NFFLWoody[i].NumClasses*sizeof(WoodyData)))==NULL)
     if((NFFLWoody[2].wd=new WoodyData[NFFLWoody[i].NumClasses])==NULL)
     	return false;
     ZeroMemory(NFFLWoody[2].wd, NFFLWoody[i].NumClasses*sizeof(WoodyData));
     NFFLWoody[i].wd[0].SurfaceAreaToVolume=1500.0;
     NFFLWoody[i].wd[0].Load=3.01;
     NFFLWoody[i].wd[1].SurfaceAreaToVolume=109.0;
     NFFLWoody[i].wd[1].Load=0.0;
     NFFLWoody[i].wd[2].SurfaceAreaToVolume=30.0;
     NFFLWoody[i++].wd[2].Load=0.0;

// fuel model 4
	NFFLWoody[i].NumClasses=4;
     NFFLWoody[i].Depth=6.0;
     NFFLWoody[i].Units=1;
//     if((NFFLWoody[3].wd=(WoodyData *) calloc(NFFLWoody[i].NumClasses, sizeof(WoodyData)))==NULL)
//     if((NFFLWoody[3].wd=(WoodyData *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, NFFLWoody[i].NumClasses*sizeof(WoodyData)))==NULL)
//     	return false;
     if((NFFLWoody[3].wd=new WoodyData[NFFLWoody[i].NumClasses])==NULL)
     	return false;
     ZeroMemory(NFFLWoody[3].wd, NFFLWoody[i].NumClasses*sizeof(WoodyData));
     NFFLWoody[i].wd[0].SurfaceAreaToVolume=2000.0;
     NFFLWoody[i].wd[0].Load=5.01;
     NFFLWoody[i].wd[1].SurfaceAreaToVolume=109.0;
     NFFLWoody[i].wd[1].Load=4.01;
     NFFLWoody[i].wd[2].SurfaceAreaToVolume=30.0;
     NFFLWoody[i].wd[2].Load=2.00;
     NFFLWoody[i].wd[3].SurfaceAreaToVolume=1500.0;
     NFFLWoody[i++].wd[3].Load=5.01;

// fuel model 5
	NFFLWoody[i].NumClasses=4;
     NFFLWoody[i].Depth=2.0;
     NFFLWoody[i].Units=1;
//     if((NFFLWoody[4].wd=(WoodyData *) calloc(NFFLWoody[i].NumClasses, sizeof(WoodyData)))==NULL)
//     if((NFFLWoody[4].wd=(WoodyData *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, NFFLWoody[i].NumClasses*sizeof(WoodyData)))==NULL)
//     	return false;
     if((NFFLWoody[4].wd=new WoodyData[NFFLWoody[i].NumClasses])==NULL)
     	return false;
     ZeroMemory(NFFLWoody[4].wd, NFFLWoody[i].NumClasses*sizeof(WoodyData));
     NFFLWoody[i].wd[0].SurfaceAreaToVolume=2000.0;
     NFFLWoody[i].wd[0].Load=1.00;
     NFFLWoody[i].wd[1].SurfaceAreaToVolume=109.0;
     NFFLWoody[i].wd[1].Load=0.5;
     NFFLWoody[i].wd[2].SurfaceAreaToVolume=30.0;
     NFFLWoody[i].wd[2].Load=0.00;
     NFFLWoody[i].wd[3].SurfaceAreaToVolume=1500.0;
     NFFLWoody[i++].wd[3].Load=2.00;

// fuel model 6
	NFFLWoody[i].NumClasses=3;
     NFFLWoody[i].Depth=2.5;
     NFFLWoody[i].Units=1;
//     if((NFFLWoody[5].wd=(WoodyData *) calloc(NFFLWoody[i].NumClasses, sizeof(WoodyData)))==NULL)
//     if((NFFLWoody[5].wd=(WoodyData *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, NFFLWoody[i].NumClasses*sizeof(WoodyData)))==NULL)
//     	return false;
     if((NFFLWoody[5].wd=new WoodyData[NFFLWoody[i].NumClasses])==NULL)
     	return false;
     ZeroMemory(NFFLWoody[5].wd, NFFLWoody[i].NumClasses*sizeof(WoodyData));
     NFFLWoody[i].wd[0].SurfaceAreaToVolume=1750.0;
     NFFLWoody[i].wd[0].Load=1.50;
     NFFLWoody[i].wd[1].SurfaceAreaToVolume=109.0;
     NFFLWoody[i].wd[1].Load=2.5;
     NFFLWoody[i].wd[2].SurfaceAreaToVolume=30.0;
     NFFLWoody[i++].wd[2].Load=2.0;

// fuel model 7
	NFFLWoody[i].NumClasses=4;
     NFFLWoody[i].Depth=2.5;
     NFFLWoody[i].Units=1;
//     if((NFFLWoody[6].wd=(WoodyData *) calloc(NFFLWoody[i].NumClasses, sizeof(WoodyData)))==NULL)
//     if((NFFLWoody[6].wd=(WoodyData *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, NFFLWoody[i].NumClasses*sizeof(WoodyData)))==NULL)
//     	return false;
     if((NFFLWoody[6].wd=new WoodyData[NFFLWoody[i].NumClasses])==NULL)
     	return false;
     ZeroMemory(NFFLWoody[6].wd, NFFLWoody[i].NumClasses*sizeof(WoodyData));
     NFFLWoody[i].wd[0].SurfaceAreaToVolume=1750.0;
     NFFLWoody[i].wd[0].Load=1.13;
     NFFLWoody[i].wd[1].SurfaceAreaToVolume=109.0;
     NFFLWoody[i].wd[1].Load=1.87;
     NFFLWoody[i].wd[2].SurfaceAreaToVolume=30.0;
     NFFLWoody[i].wd[2].Load=1.5;
     NFFLWoody[i].wd[3].SurfaceAreaToVolume=1500.0;
     NFFLWoody[i++].wd[3].Load=0.37;

// fuel model 8
	NFFLWoody[i].NumClasses=3;
     NFFLWoody[i].Depth=0.2;
     NFFLWoody[i].Units=1;
//     if((NFFLWoody[7].wd=(WoodyData *) calloc(NFFLWoody[i].NumClasses, sizeof(WoodyData)))==NULL)
//     if((NFFLWoody[7].wd=(WoodyData *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, NFFLWoody[i].NumClasses*sizeof(WoodyData)))==NULL)
//     	return false;
     if((NFFLWoody[7].wd=new WoodyData[NFFLWoody[i].NumClasses])==NULL)
     	return false;
     ZeroMemory(NFFLWoody[7].wd, NFFLWoody[i].NumClasses*sizeof(WoodyData));
     NFFLWoody[i].wd[0].SurfaceAreaToVolume=2000.0;
     NFFLWoody[i].wd[0].Load=1.50;
     NFFLWoody[i].wd[1].SurfaceAreaToVolume=109.0;
     NFFLWoody[i].wd[1].Load=1.00;
     NFFLWoody[i].wd[2].SurfaceAreaToVolume=30.0;
     NFFLWoody[i++].wd[2].Load=2.5;

// fuel model 9
	NFFLWoody[i].NumClasses=3;
     NFFLWoody[i].Depth=0.2;
     NFFLWoody[i].Units=1;
//     if((NFFLWoody[8].wd=(WoodyData *) calloc(NFFLWoody[i].NumClasses, sizeof(WoodyData)))==NULL)
//     if((NFFLWoody[8].wd=(WoodyData *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, NFFLWoody[i].NumClasses*sizeof(WoodyData)))==NULL)
//     	return false;
     if((NFFLWoody[8].wd=new WoodyData[NFFLWoody[i].NumClasses])==NULL)
     	return false;
     ZeroMemory(NFFLWoody[8].wd, NFFLWoody[i].NumClasses*sizeof(WoodyData));
     NFFLWoody[i].wd[0].SurfaceAreaToVolume=2500.0;
     NFFLWoody[i].wd[0].Load=2.92;
     NFFLWoody[i].wd[1].SurfaceAreaToVolume=109.0;
     NFFLWoody[i].wd[1].Load=0.41;
     NFFLWoody[i].wd[2].SurfaceAreaToVolume=30.0;
     NFFLWoody[i++].wd[2].Load=0.15;

// fuel model 10
	NFFLWoody[i].NumClasses=4;
     NFFLWoody[i].Depth=1.0;
     NFFLWoody[i].Units=1;
//     if((NFFLWoody[9].wd=(WoodyData *) calloc(NFFLWoody[i].NumClasses, sizeof(WoodyData)))==NULL)
//     if((NFFLWoody[9].wd=(WoodyData *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, NFFLWoody[i].NumClasses*sizeof(WoodyData)))==NULL)
//     	return false;
     if((NFFLWoody[9].wd=new WoodyData[NFFLWoody[i].NumClasses])==NULL)
     	return false;
     ZeroMemory(NFFLWoody[9].wd, NFFLWoody[i].NumClasses*sizeof(WoodyData));
     NFFLWoody[i].wd[0].SurfaceAreaToVolume=2000.0;
     NFFLWoody[i].wd[0].Load=3.01;
     NFFLWoody[i].wd[1].SurfaceAreaToVolume=109.0;
     NFFLWoody[i].wd[1].Load=2.0;
     NFFLWoody[i].wd[2].SurfaceAreaToVolume=30.0;
     NFFLWoody[i].wd[2].Load=5.01;
     NFFLWoody[i].wd[3].SurfaceAreaToVolume=1500.0;
     NFFLWoody[i++].wd[3].Load=2.00;

// fuel model 11
	NFFLWoody[i].NumClasses=3;
     NFFLWoody[i].Depth=1.0;
     NFFLWoody[i].Units=1;
//     if((NFFLWoody[10].wd=(WoodyData *) calloc(NFFLWoody[i].NumClasses, sizeof(WoodyData)))==NULL)
//     if((NFFLWoody[10].wd=(WoodyData *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, NFFLWoody[i].NumClasses*sizeof(WoodyData)))==NULL)
//     	return false;
     if((NFFLWoody[10].wd=new WoodyData[NFFLWoody[i].NumClasses])==NULL)
     	return false;
     ZeroMemory(NFFLWoody[10].wd, NFFLWoody[i].NumClasses*sizeof(WoodyData));
     NFFLWoody[i].wd[0].SurfaceAreaToVolume=1500.0;
     NFFLWoody[i].wd[0].Load=1.50;
     NFFLWoody[i].wd[1].SurfaceAreaToVolume=109.0;
     NFFLWoody[i].wd[1].Load=4.51;
     NFFLWoody[i].wd[2].SurfaceAreaToVolume=30.0;
     NFFLWoody[i++].wd[2].Load=5.51;

// fuel model 12
	NFFLWoody[i].NumClasses=3;
     NFFLWoody[i].Depth=2.3;
     NFFLWoody[i].Units=1;
//     if((NFFLWoody[11].wd=(WoodyData *) calloc(NFFLWoody[i].NumClasses, sizeof(WoodyData)))==NULL)
//     if((NFFLWoody[11].wd=(WoodyData *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, NFFLWoody[i].NumClasses*sizeof(WoodyData)))==NULL)
//     	return false;
     if((NFFLWoody[11].wd=new WoodyData[NFFLWoody[i].NumClasses])==NULL)
     	return false;
     ZeroMemory(NFFLWoody[11].wd, NFFLWoody[i].NumClasses*sizeof(WoodyData));
     NFFLWoody[i].wd[0].SurfaceAreaToVolume=1500.0;
     NFFLWoody[i].wd[0].Load=4.01;
     NFFLWoody[i].wd[1].SurfaceAreaToVolume=109.0;
     NFFLWoody[i].wd[1].Load=14.03;
     NFFLWoody[i].wd[2].SurfaceAreaToVolume=30.0;
     NFFLWoody[i++].wd[2].Load=16.53;

// fuel model 13
	NFFLWoody[i].NumClasses=3;
     NFFLWoody[i].Depth=3.0;
     NFFLWoody[i].Units=1;
//     if((NFFLWoody[12].wd=(WoodyData *) calloc(NFFLWoody[i].NumClasses, sizeof(WoodyData)))==NULL)
//     if((NFFLWoody[12].wd=(WoodyData *) GlobalAlloc(GMEM_FIXED |  GMEM_ZEROINIT, NFFLWoody[i].NumClasses*sizeof(WoodyData)))==NULL)
//     	return false;
     if((NFFLWoody[12].wd=new WoodyData[NFFLWoody[i].NumClasses])==NULL)
     	return false;
     ZeroMemory(NFFLWoody[12].wd, NFFLWoody[i].NumClasses*sizeof(WoodyData));
     NFFLWoody[i].wd[0].SurfaceAreaToVolume=1500.0;
     NFFLWoody[i].wd[0].Load=7.01;
     NFFLWoody[i].wd[1].SurfaceAreaToVolume=109.0;
     NFFLWoody[i].wd[1].Load=23.04;
     NFFLWoody[i].wd[2].SurfaceAreaToVolume=30.0;
     NFFLWoody[i].wd[2].Load=28.05;

     long j;

     // convert all to metric
     for(i=0; i<13; i++)
     {	NFFLWoody[i].Depth/=3.280839895;
     	for(j=0; j<NFFLWoody[i].NumClasses; j++)
          {	NFFLWoody[i].wd[j].SurfaceAreaToVolume*=3.280839895;
          	NFFLWoody[i].wd[j].Load*=0.224169061;
               NFFLWoody[i].wd[j].HeatContent=8000.0*2.32599;
               NFFLWoody[i].wd[j].Density=513.0;
               NFFLWoody[i].TotalWeight+=NFFLWoody[i].wd[j].Load;
          }
     }
*/
     return true;
}


double GetWoodyFuelMoisture(long ModelNumber, long SizeClass)
{
     if(ModelNumber>MAXNUM_COARSEWOODY_MODELS)
     	return 0.0;

	if(coarsewoody[ModelNumber-1].NumClasses<SizeClass)
     	return 0.0;

     return coarsewoody[ModelNumber-1].wd[SizeClass].FuelMoisture;
}


long GetWoodyDataUnits(long ModelNumber, char *Description)
{
	CopyMemory(Description, coarsewoody[ModelNumber-1].Description, 64*sizeof(char));

	return coarsewoody[ModelNumber-1].Units;
}


void GetWoodyData(long WoodyModelNumber, long SurfModelNumber, long *NumClasses, WoodyData *woody, double *depth, double *load)
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
     	CopyMemory(woody, NFFLWoody[SurfModelNumber-1].wd, NFFLWoody[SurfModelNumber-1].NumClasses*sizeof(WoodyData));
     }
	else if(SurfModelNumber>0)
     {	//double t1, t10, t100, tLH, tLW, s1, sLH, sLW, hd, hl, d, xm;
     	long i=0, j;
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
	     {    CopyMemory(&woody[*NumClasses], coarsewoody[WoodyModelNumber-1].wd, coarsewoody[WoodyModelNumber-1].NumClasses*sizeof(WoodyData));
			*NumClasses+=coarsewoody[WoodyModelNumber-1].NumClasses;
	     	*depth=coarsewoody[WoodyModelNumber-1].Depth;
		     *load+=coarsewoody[WoodyModelNumber-1].TotalWeight;
     	}
     }
}


void GetCurrentFuelMoistures(long fuelmod, long woodymod, double *mxin, double *mxout, long NumMx)
{
	bool Combine;
     long i, NumClasses=0;

     ZeroMemory(mxout, 20*sizeof(double));

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
	     		case 1:	CopyMemory(mxout, mxin, 3*sizeof(double));
                              NumClasses=3;
                    		break;
	     		case 2:	CopyMemory(mxout, mxin, 3*sizeof(double));
                    		mxout[3]=mxin[5];
                              NumClasses=4;
               			break;
		     	case 3:	CopyMemory(mxout, mxin, 3*sizeof(double));
                              NumClasses=3;
                    		break;
	     		case 4:	CopyMemory(mxout, mxin, 3*sizeof(double));
                    		mxout[3]=mxin[5];
                              NumClasses=4;
                    		break;
	     		case 5:	CopyMemory(mxout, mxin, 3*sizeof(double));
	               	     mxout[3]=mxin[5];
                              NumClasses=4;
                    		break;
		     	case 6:	CopyMemory(mxout, mxin, 3*sizeof(double));
						NumClasses=3;
                    		break;
		     	case 7:	CopyMemory(mxout, mxin, 4*sizeof(double));
						NumClasses=4;
                    		break;
	     		case 8:	CopyMemory(mxout, mxin, 3*sizeof(double));
						NumClasses=3;
                    		break;
	     		case 9:	CopyMemory(mxout, mxin, 3*sizeof(double));
						NumClasses=3;
                    		break;
		     	case 10:	CopyMemory(mxout, mxin, 3*sizeof(double));
						mxout[3]=mxin[5];
                              NumClasses=4;
                    		break;
		     	case 11:	CopyMemory(mxout, mxin, 3*sizeof(double));
                              NumClasses=3;
                    		break;
	     		case 12:	CopyMemory(mxout, mxin, 3*sizeof(double));
                              NumClasses=3;
                    		break;
	     		case 13:	CopyMemory(mxout, mxin, 3*sizeof(double));
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
		CopyMemory(&mxout[NumClasses], mxin, 3*sizeof(double));
          mxout[3]=mxin[5];
     }
}


long WoodyCombineOptions(long Option)
{
	if(Option>0)
     	CombineOption=Option;

     return CombineOption;
}


double WeightLossErrorTolerance(double value)
{
	if(value>0.0)
	    WeightLossErrorTol=value;

    	return WeightLossErrorTol;
}

//----------------------------------------------------------------
//
//----------------------------------------------------------------

static long MaxThreads=1;
static bool CanModify=true;

long GetMaxThreads()
{
	return MaxThreads;
}


void SetMaxThreads(long numthreads)
{
	if(numthreads>0 && numthreads<65)
		MaxThreads=numthreads;
}


bool	CanModifyInputs(long YesNo)
{
	if(YesNo>=0)
     	CanModify=(bool) YesNo;

	return CanModify;
}



//------------------------------------------------------------------------------
//	Stuff for FlameMap
//------------------------------------------------------------------------------

static float*  ThreadLayer;
static long 	ThreadOpts=0;
static long    OutputDirection=RELATIVEDIR;
static double 	ConstWindDirection=-1.0;
static double 	ConstWindSpeed=0.0;
static long 	ConstFuelMoisture=1;
static float 	*outlayer[15]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static char 	FlameAceBaseName[256]="";
static char 	WindFileName[256]="";
static char	WeatherFileName[256]="";
static char 	FuelMoistureFileName[256]="";
static char    FuelModelName[256]="";
static double	OffsetFromMax=0.0;	// offset in fire spread direction from Rmax
static double ThreadProgress[64];


void SetOffsetFromMax(double degrees)
{
	OffsetFromMax=degrees;
}


double GetOffsetFromMax()
{
	return OffsetFromMax;
}


bool SelectOutputLayer(long Layer, bool truefalse)
{
	FreeOutputLayer(Layer);
	if(truefalse)
     {	//if((outlayer[Layer]=(float *) calloc(GetNumNorth()*GetNumEast(), sizeof(float)))==NULL)
     	if((outlayer[Layer]=new float[GetNumNorth()*GetNumEast()])==NULL)//(float *) GlobalAlloc(GMEM_FIXED, GetNumNorth()*GetNumEast()*sizeof(float)))==NULL)
               return false;
     	else
          {	long i, j;
          	j=GetNumNorth()*GetNumEast();
          	for(i=0; i<j; i++)
               	outlayer[Layer][i]=-1.0;
          }
     }

     return true;
}


bool GetOutputOption(long Layer)
{
	if(outlayer[Layer])
     	return true;

     return false;
}


float *GetThreadLayer(long Layer, long row)
{
	return ThreadLayer=&outlayer[Layer][row*GetNumEast()];
}


void WriteMapCell(long Layer, long row, long col, double value)
{// writes value to memory
	outlayer[Layer][row*GetNumEast()+col]=(float) value;
}

void WriteFriskToDisk()
{
     long i, j;
	FILE *outfile;
     char FileName[256]="";
     long nodata=-1;

     strcat(FileName, FlameAceBaseName);
     strcat(FileName, ".FSK");
     outfile=fopen(FileName, "wb");

     fwrite(&Header.numeast, sizeof(long), 1, outfile);
     fwrite(&Header.numnorth, sizeof(long), 1, outfile);
     fwrite(&Header.WestUtm, sizeof(double), 1, outfile);
     fwrite(&Header.SouthUtm, sizeof(double), 1, outfile);
     fwrite(&Header.XResol, sizeof(double), 1, outfile);
     fwrite(&nodata, sizeof(long), 1, outfile);

     fwrite(outlayer[MAXSPREADDIR], sizeof(float), Header.numeast*Header.numnorth, outfile);
     fwrite(outlayer[ELLIPSEDIM_A], sizeof(float), Header.numeast*Header.numnorth, outfile);
     fwrite(outlayer[ELLIPSEDIM_B], sizeof(float), Header.numeast*Header.numnorth, outfile);
     fwrite(outlayer[ELLIPSEDIM_C], sizeof(float), Header.numeast*Header.numnorth, outfile);
     fwrite(outlayer[MAXSPOT], sizeof(float), Header.numeast*Header.numnorth, outfile);
	fwrite(outlayer[INTENSITY], sizeof(float), Header.numeast*Header.numnorth, outfile);
     fclose(outfile);

	//printf("WestUtm %lf\n", Header.WestUtm);
	//printf("SouthUtm %lf\n", Header.SouthUtm);
	//if(access("slope.asc", 00)==-1)
		WriteAsciiLCPTheme(1, 1, "slope.asc");
	//if(access("aspect.asc", 00)==-1)
		WriteAsciiLCPTheme(2, 1, "aspect.asc");
}


void WriteOutputLayerToDisk(long Layer)
{
     long i, j;
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
     }
     outfile=fopen(FileName, "w");
     fprintf(outfile, "%s %ld\n", "NCOLS", GetNumEast());
     fprintf(outfile, "%s %ld\n", "NROWS", GetNumNorth());
     fprintf(outfile, "%s %lf\n", "XLLCORNER", GetWestUtm());
     fprintf(outfile, "%s %lf\n", "YLLCORNER", GetSouthUtm());
     fprintf(outfile, "%s %lf\n", "CELLSIZE", GetCellResolutionX());
     fprintf(outfile, "%s %ld\n", "NODATA_VALUE", -1);
     for(i=0; i<GetNumNorth(); i++)
     {  	for(j=0; j<GetNumEast(); j++)
          	fprintf(outfile, "%f ", outlayer[Layer][i*GetNumEast()+j]);
     	fprintf(outfile, "\n");
     }
     fclose(outfile);
}


void FreeOutputLayer(long Layer)
{
	if(outlayer[Layer])
     	//free(outlayer[Layer]);
          delete[] outlayer[Layer];//GlobalFree(outlayer[Layer]);
     outlayer[Layer]=0;
	//memset(FlameAceBaseName, 0x0, sizeof(FlameAceBaseName));
}


void SetOutputBaseName(char *name)
{
     memset(FlameAceBaseName, 0x0, sizeof(FlameAceBaseName));
	strcpy(FlameAceBaseName, name);
}

char *GetOutputBaseName()
{
	return FlameAceBaseName;
}

void SetConstWind(double Speed, double Dir)
{
	ConstWindSpeed=Speed;
     ConstWindDirection=Dir;
}

double GetConstWindSpeed()
{
 	return ConstWindSpeed;
}

double GetConstWindDir()
{
	return ConstWindDirection;
}

bool IsConstWinds()
{
     if(ConstWindSpeed<0.0)
     	return false;

	return true;
}

long ConstFuelMoistures(long Const)
{
     if(Const>=0)
     	ConstFuelMoisture=(bool) Const;

    	return ConstFuelMoisture;
}


void	SetWeatherFileName(char *name)
{
	strcpy(WeatherFileName, name);
}

void	SetFuelMoistureFileName(char *name)
{
	strcpy(FuelMoistureFileName, name);
}

void SetCustFuelModelName(char *name)
{
	strcpy(FuelModelName, name);
}

char *GetCustFuelModelName()
{
	return FuelModelName;
}

void SetWindFileName(char *name)
{
	strcpy(WindFileName, name);
}

char *GetWeatherFileName()
{
	return WeatherFileName;
}

char *GetFuelMoistureFileName()
{
	return FuelMoistureFileName;
}

char *GetWindFileName()
{
	return WindFileName;
}

long	GetOutputDirection()
{
	return OutputDirection;
}

void SetOutputDirection(long dir)
{
	OutputDirection=dir;
}

long	GetThreadOpts()
{
	return ThreadOpts;
}

void SetThreadOpts(long num)
{
	ThreadOpts=num;
}


void SetThreadProgress(long ThreadNum, double Fract)
{
	if(ThreadNum<64)
     	ThreadProgress[ThreadNum]=Fract;
}


double *GetThreadProgress()
{
	return ThreadProgress;
}


//------------------------------------------------------------------------
//
//
//
//------------------------------------------------------------------------


void ResetNewFuels()
{
    	memset(newfuels, 0x0, 257*sizeof(NewFuel));

     InitializeNewFuel();

     long i;
     for(i=0; i<257; i++)
     {	if(newfuels[i].number)
          	newfuels[i].number*=-1;	// indicate permanence
     }

    	HAVE_CUST_MODELS=false;
}


bool SetNewFuel(NewFuel *newfuel)
{
	long i, FuelNum;

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


bool GetNewFuel(long number, NewFuel *newfuel)
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


bool IsNewFuelReserved(long number)
{
	if(number<0)
     	return false;

	if(newfuels[number].number<0)
     	return true;

     return false;
}


double GetFuelDepth(int Number)
{
	if(Number<0)
     	return 0.0;

	if(newfuels[Number].number==0)
     	return 0.0;

	return newfuels[Number].depth;
}


void InitializeNewFuel()
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














