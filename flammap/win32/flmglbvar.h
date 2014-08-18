
//------------------------------------------------------------------------------
//	FSGLBVAR.H	Header File for accessing global data for FARSITE interface
//
//
//
//
//                  Copyright 1994, 1995
//   			Mark A. Finney, Systems for Environmental Management
//-----------------------------------------------------------------------------


#ifndef FARSITEGLOBVARS
#define FARSITEGLOBVARS

#define NULLLOCATION		0
#define IGNITIONLOCATION 	1
#define BARRIERLOCATION 		2
#define FUELLOCATION     	3
#define WEATHERLOCATION  	4
#define ZOOMLOCATION     	5
#define EDITLOCATION     	6
#define DIRECTLOCATION   	7
#define INDIRECTLOCATION 	8
#define PARALLELLOCATION 	9
#define AERIALLOCATION   	10
#define RELOCATEDIRECT   	11
#define RELOCATEINDIRECT 	12
#define RELOCATEPARALLEL 	13
#define RELOCATEAIRATTACK 	14
#define ADDGROUNDATTACK		15
#define FIREINFORMATION 		16
#define MOVEPERIMPOINTS		17
#define PANLANDSCAPE		18
#define STOPLOCATION		19
#define MEASURELANDSCAPE		20

#define WW_UNITS_METRIC		1
#define WW_UNITS_ENGLISH		0
#define WW_UNITS_UNDEFINED	-1

#define RAST_ARRIVALTIME      20
#define RAST_FIREINTENSITY    21
#define RAST_SPREADRATE       22
#define RAST_FLAMELENGTH      23
#define RAST_HEATPERAREA      24
#define RAST_CROWNFIRE        25
#define RAST_FIREDIRECTION    26
#define RAST_REACTIONINTENSITY 27

#define E_DATA			0
#define S_DATA			1
#define A_DATA			2
#define F_DATA			3
#define C_DATA			4
#define H_DATA			5
#define B_DATA			6
#define P_DATA			7
#define D_DATA			8
#define W_DATA			9

#define FM_INTERVAL_TIME  	0
#define FM_INTERVAL_ELEV		1
#define FM_INTERVAL_SLOPE	2
#define FM_INTERVAL_ASP		3
#define FM_INTERVAL_COV		4

#include "flm4.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include "fsxwatm.h"
#include "fsxw.hpp"
//#include "fsw3d2.h"

void 	CheckMessageLoop();						// peekmessage loop
double 	pow2(double base);               			// returns square of base
void 	GetRatios(double *xratio, double *yratio);   // get window ratios
void 	SetRatios(double xratio, double yratio);	// set landscape window ratios
void 	GetMaxDims(double *maxx, double *maxy);		// get max window dimensions
void 	SetMaxDims(double maxx, double maxy);		// set max window dimensions
void 	SetXORArrows(bool TorF);					// enable XOR wind arrows
bool 	GetXORArrows();						// check if XOR enabled
//TWindow * GetFarsiteWindow();					// retrieve landscape window ptr
//void 	SetFarsiteWindow(TWindow* FWindow);		// set landscape window ptr
//TFarsiteWindow *GetFarsiteWindow();				// retrieve landscape window ptr
//TThreeDWindow *Get3dWindow();
//void 	SetFarsiteWindow(TFarsiteWindow* FWindow);		// set landscape window ptr
//void 	Set3dWindow(TThreeDWindow* DWindow);
HBITMAP 	GetBitmap(short Number);			// get 1of3 bitmaps for landscape window
void 	SetBitmap(short Number, HBITMAP);			// set 1 of 3 landscape window bitmaps
void 	ResetBitmap(short Number);				// reset 1 of 3 landscape window bitmaps
int 		GetFuelsAltered();						// retrieve flag for altered fuel map
void 	SetFuelsAltered(int YesNo);				// set flag for altered fuel map
void 	GetDefaultCrownChx(double *Height, double *Base, double *Density);
long		GetTheme_DistanceUnits();
short	GetTheme_Units(short DataTheme);
short	GetTheme_NumCategories(short DataTheme);
long		GetTheme_HiValue(short DataTheme);
long		GetTheme_LoValue(short DataTheme);
char *	GetTheme_FileName(short DataTheme);
bool		LinkDensityWithCrownCover(long TrueFalse);   // returns true or false
int 		CheckCellResUnits();					// returns 0 for metric 1 for English
double 	GetCellResolutionX();                      	// return landscape X cell dimension
double 	GetCellResolutionY();                      	// return landscape Y cell dimension
double 	MetricResolutionConvert();                 	// 3.28.. for english, 1.0 for metric
long		CanSetRasterResolution(long YesNo);		// flag to allow setting of rast res
long		SetRasterExtentToViewport(long YesNo);		// flag to reduce raster extent for viewport
void 	SetRastRes(double XResol, double YResol);	// set raster output resolution
void 	GetRastRes(double *XResol, double *YResol);  // get raster output resolution
long 	GridInterval(long Interval);				// get & set Grid Interval
COLORREF	GridColor(long Colr);					// get & set Grid Color
long 	TopoShadeDirection(long Direction);		// get & set Light Source Direction
void 	SetTopoShading(bool YesNo);
bool 	TopoShading();
long 	GetInputMode();                              // gets input mode to the landscape window
void 	SetInputMode(long ModeCode, bool Pending);	// sets input model to the landscape window
bool 	ChangeInputMode(long YesNo);                 // set & retrieve input mode changes
void      SetZoomReplayDelay(long MilliSeconds);
long      GetZoomReplayDelay();

void 	SetRastFormat(long Type);                    // raster output information
long 	GetRastFormat();                             //
void 	SetVectFormat(long Type);                    // vector output information
long 	GetVectFormat();                             //
void 	SetRastMake(bool YesNo);                     // make/don't make raster files
bool 	GetRastMake();                               //
void 	SetVectMake(bool YesNo);                     // make/don't make vector files
bool 	GetVectMake();                               //
long 	ExcludeBarriersFromVectorFiles(long YesNo);	// include barriers or not
void 	SetVectVisOnly(bool YesNo);                  // otp visible perim only/not
bool 	GetVectVisOnly();
void		SetShapeMake(bool YesNo);
bool		GetShapeMake();
long      AccessDisplayUnits(long val);
long 	AccessOutputUnits(long val);

double 	MinFireChx(double);						// max display fire chx
double 	MaxFireChx(double);						// max display fire chx
long 	NumFireInterval(long Num);				// num of fire intervals displayed
long 	OutputFireParam(long Param);				// select fire chx for display

void 	SetCanopyChx(double Height, double CrownBase, double BulkDensity, double Diameter,
			   long FoliarMoisture, long Tolerance, long Species);
double 	GetDefaultCrownHeight();
double 	GetDefaultCrownBase();
double 	GetDefaultCrownBD(short cover);
double 	GetAverageDBH();
double 	GetFoliarMC();
long 	GetTolerance();
long 	GetCanopySpecies();
void 	SetFoliarMoistureContent(long Percent);

//double 	GetDuffLoad();
//long 	GetWoodyModel();

void 	ResetFuelConversions();
void 	ResetCustomFuelModels();
int 		GetFuelConversion(int fuel);
int 		SetFuelConversion(int fueltype, int fuelmodel);
double	GetFuelDepth(int Number);
bool		IsCustomFuelModelHere(long Number);
void 	GetFuel(int Number, double *t1, double *t10, double *t100, double *tLH, double *tLW, double *s1,
					double *sL, double *sH, double *hd, double *hl, double *d, double *xm, char *comment);
void 	SetFuel(int Number, double t1, double t10, double t100, double tLH,
                    double tLW, long s1, long sH, long sW,
			   	long hd, long hl, double d, double xm, char *comment);
long 	AccessFuelModelUnits(long Val);
char *	GetCustomModelDescription(long Number);

COLORREF  GetThemeColor(long VisTheme, long Attrib);
void 	SetThemeColor(long VisTheme, long Attrib, COLORREF colr);
COLORREF 	GetFuelColor(int Number);
COLORREF 	GetChangedColor(int Number);
void 	SetFuelColor(int Number, COLORREF colr);
void 	SetChangedColor(int Number, COLORREF colr);
void 	SetOrderToFuel(int Number, int Order);
int 		GetOrderToFuel(int Number);
int 		GetNumFuelChanges();
void 	SetNumFuelChanges(int Num);
void 	IncNumFuelChanges();
void 	SetFullTint(double tint);
double 	GetFullTint();
void 	SetTint(double tint);
double 	GetTint();
void 	SetMaxWindowDim(long Dim);
long 	GetMaxWindowDim();
bool		ViewPortStatus(long TrueFalse);          // see if view port has changed
bool 	ViewPortChanged(long TrueFalse);
void 	SetLandscapeViewPort(double west, double east, double north, double south);
void 	GetLandscapeViewPort(double *west, double *east, double *north, double *south);
double	GetViewNorth();
double 	GetViewSouth();
double 	GetViewEast();
double 	GetViewWest();
long		GetNumViewEast();
long 	GetNumViewNorth();
char *    GetCurrentPath();
bool      CanUseCurrentPath(long YesNo);

long      AdjustIgnitionSpreadRates(long YesNoValue);
bool      PreserveInactiveEnclaves(long YesNo);

void 	SetRasterFileName(char *FileName);
char 	*GetRasterFileName(long Type);
void 	SetVectorFileName(char *FileName);
char 	*GetVectorFileName();
void 	SetVisPerimPoint(long xpt, long ypt, long firenum);
void 	ResetVisPerimFile();
void 	CopyOutVisPerimFile(char *NewVisName);
void 	CopyInVisPerimFile(char *NewVisName);
long 	GetVisPerimFile(long *xpt, long *ypt);
void 	SetVisPerimFile(long xpt, long ypt);
long  	OpenVisPerimFile(int rewindit);
void 	OpenVisPerimFileAtPosition(long FilePosition);
void 	OpenCopyVisPerimFile(char *FileName);
long 	CloseVisPerimFile(int writeend, long firenum);
bool		CreateVisperimSemaphore();
void		CloseVisperimSemaphore();

void 	SetFileOutputOptions(long FileType, bool YesNo);
bool		GetFileOutputOptions(long FileType);

void 	SetViewRasterThemeInfo(char *filename, long hi, long lo);
char*	GetViewRasterThemeInfo(long *hi, long *lo);
bool 	Draw3DVectorFile(long YesNo);
void 	SetViewVectorThemeInfo(char *filename, COLORREF colr, long FileType);
char*	GetViewVectorThemeInfo(COLORREF *colr);
long		GetViewVectorFileType();

void		SetShapeFileChx(char *ShapeFileName, long VisOnly, long Polygons, long BarriersSep);
char*	GetShapeFileChx(long *VisOnly, long *Polygons, long *BarriersSep);

static long	CrownFireCalculation=0;			// 0=Finney (1998), 1=Scott&Reinhardt (2001)



typedef struct
{// structure for holding weather/wind stream data max and min values
	int 	wtr;
	int 	wnd;
	int 	all;
}streamdata;


typedef struct
{// wind data structure
	long 	mo;
	long 	dy;
	long 	hr;
	double 	ws;		// windspeed mph
	long 	wd;		// winddirection azimuth
	long 	cl;		// cloudiness
}WindData;

bool 	CalcFirstLastStreamData();		// compute 1st and last valid dates for simulation
void		SetCorrectStreamNumbers();		// for Atm Grid to fool stream data
long 	GetOpenWindStation();
long 	AllocWindData(long StatNum, long NumObs);
void 	FreeWindData(long StationNumber);
long 	SetWindData(long StationNumber, long NumObs, long month, long day,
				  long hour, double windspd, long winddir, long cloudy);
long 	GetWindMonth(long StationNumber, long NumObs);
long 	GetWindDay(long StationNumber, long NumObs);
long 	GetWindHour(long StationNumber, long NumObs);
double 	GetWindSpeed(long StationNumber, long NumObs);
long 	GetWindDir(long StationNumber, long NumObs);
long 	GetWindCloud(long StationNumber, long NumObs);
long 	GetMaxWindObs(long StationNumber);
void		SetWindUnits(long StationNumber, long Units);
void 	SetWeatherUnits(long StationNumber, long Units);
long		GetWindUnits(long StationNumber);
long 	GetWeatherUnits(long StationNumber);

typedef struct
{
	long 	mo;
	long 	dy;
	double 	rn;
	long 	t1;
	long 	t2;
	double 	T1;
	double 	T2;
	long 	H1;
	long 	H2;
	double 	el;
     long 	tr1;
     long		tr2;
} WeatherData;

long 	GetOpenWeatherStation();
long 	AllocWeatherData(long StatNum, long NumObs);
void 	FreeWeatherData(long StationNumber);
long 	SetWeatherData(long StationNumber, long NumObs, long month, long day,
					double rain, long time1, long time2, double temp1, double temp2,
					long humid1, long humid2, double elevation, long tr1, long tr2);
long 	GetWeatherMonth(long StationNumber, long NumObs);
long 	GetWeatherDay(long StationNumber, long NumObs);
double 	GetWeatherRain(long StationNumber, long NumObs);
long 	GetWeatherTime1(long StationNumber, long NumObs);
long 	GetWeatherTime2(long StationNumber, long NumObs);
double 	GetWeatherTemp1(long StationNumber, long NumObs);
double 	GetWeatherTemp2(long StationNumber, long NumObs);
long 	GetWeatherHumid1(long StationNumber, long NumObs);
long 	GetWeatherHumid2(long StationNumber, long NumObs);
double 	GetWeatherElev(long StationNumber, long NumObs);
void		GetWeatherRainTimes(long StationNumber, long NumObs, long *tr1, long *tr2);
long 	GetMaxWeatherObs(long StationNumber);

struct StationGrid
{
	long 	XDim;                 // number of Easting cells
	long 	YDim;                 // number of Northing cells
	double 	Width;              // width of grid cell
	double 	Height;             // height of grid cell
	long 	*Grid;                // holds weather/wind station numbers
	StationGrid();
	~StationGrid();
};

void 	AllocStationGrid(long XDim, long YDim);	 // allocate memory for weather stations
long 	FreeStationGrid();                       // free memory for weather stations
long 	GetStationNumber(double xpt, double ypt);// find station number for coordinate
long 	SetStationNumber(long XPos, long YPos, long StationNumber);
void 	SetGridEastDimension(long XDim);
void 	SetGridNorthDimension(long YDim);
long 	GetGridEastDimension();
long 	GetGridNorthDimension();
long 	GridInitialized();
long 	GetNumStations();		  			// retrieve number of weather/wind stations
void 	SetGridNorthOffset(double offset);		// set offset for Atm Grid purposes
void 	SetGridEastOffset(double offset);
double 	GetGridNorthOffset();
double 	GetGridEastOffset();


/*
struct CanopyCharacteristics
{	// contains average values, landscape wide, temporary until themes are used
	double DefaultHeight;
	double DefaultBase;
	double DefaultDensity;
	double Height;
	double CrownBase;
	double BulkDensity;
	double Diameter;
	double FoliarMC;
	long Tolerance;
	long Species;

//	CanopyCharacteristics()
//	{	DefaultHeight=Height=20.0;           // need default for changing Variables in LCP
//		DefaultBase=CrownBase=4.0;
//		DefaultDensity=BulkDensity=0.20;
//		Diameter=20.0;
//		FoliarMC=100;
//		Tolerance=2;
//		Species=1;
//	}

};
*/


struct FuelConversions
{
	int Type[257];            // each fuel type contains a fuel model corresponding
	FuelConversions();		 // load defaults
};


struct FuelModels
{// custom fuel model provided by the user
	bool ModelIsHere[37];
	double TL1[37];
	double TL10[37];
	double TL100[37];
	double TLLiveH[37];
	double TLLiveW[37];
	long S1[37];
	long SLiveH[37];
	long SLiveW[37];
	double Depth[37];
	double XMext[37];
     long HD[37];
	long HL[37];
	COLORREF FCOLOR[52];   // hold color for all models
	COLORREF CCOLOR[9];	   // holds changed palette colors except white & black
	long OrderToFuel[9];   // holds the index to fuel changes
	long NumChanges;
     char Comment[37][256]; // holds description of fuel model

	FuelModels();
};


struct ThemeColors
{
	long ENum, SNum, ANum;
	long CNum, HNum, BNum, DNum;
	COLORREF* Elevation;
	COLORREF* Slope;
	COLORREF* Aspect;
	COLORREF* Cover;
	COLORREF* Height;
	COLORREF* Base;
	COLORREF* Density;

	ThemeColors();
	~ThemeColors();

private:
	double MaxVal, MinVal, IntValue;

	COLORREF ColorFromSpectrum(long Attribute);
};


typedef struct
{// structure for holding basic cell information
	long x;				// easting
	long y;                  // northing
	short e;                 // elevation
	short s;                 // slope
	short a;                 // aspect
	short f;                 // fuel models
	short c;                 // canopy cover
}oldcelldata;

// Moved to FSXLANDT.H
/*
typedef struct
{// structure for holding basic cell information
	short e;                 // elevation
	short s;                 // slope
	short a;                 // aspect
	short f;                 // fuel models
	short c;                 // canopy cover
}celldata;


typedef struct
{// structure for holding optional crown fuel information
	short h;				// canopy height
	short b;				// crown base
	short p;				// bulk density
}crowndata;


typedef struct
{
     short d;				// duff model
	short w;				// coarse woody model
}grounddata;
*/

typedef struct
{// header for landscape file
	long CrownFuels;
	long latitude;
	long loeast;
	long hieast;
	long lonorth;
	long hinorth;
	long loelev;
	long hielev;
	long numeast;
	long numnorth;
	double EastUtm;
	double WestUtm;
	double NorthUtm;
	double SouthUtm;
	long GridUnits;        // 0 for metric, 1 for English
	double XResol;
	double YResol;
	short EUnits;
	short SUnits;
	short AUnits;
	short FOptions;
	short CUnits;
	short HUnits;
	short BUnits;
	short PUnits;
}oldheaddata;


typedef struct
{// header for landscape file
	long CrownFuels;       // 10 if no crown fuels, 11 if crown fuels exist
	long latitude;
	double loeast;
	double hieast;
	double lonorth;
	double hinorth;
	long loelev;
	long hielev;
	long numeast;
	long numnorth;
	double EastUtm;
	double WestUtm;
	double NorthUtm;
	double SouthUtm;
	long GridUnits;        // 0 for metric, 1 for English
	double XResol;
	double YResol;
	short EUnits;
	short SUnits;
	short AUnits;
	short FOptions;
	short CUnits;
	short HUnits;
	short BUnits;
	short PUnits;
}headdata2;


typedef struct
{// header for landscape file
	long CrownFuels;         // 20 if no crown fuels, 21 if crown fuels exist
     long GroundFuels;		// 20 if no ground fuels, 21 if ground fuels exist
	long latitude;
	double loeast;
	double hieast;
	double lonorth;
	double hinorth;
	long loelev;
	long hielev;
	long numelev; 			//-1 if more than 100 categories
     long elevs[100];
     long loslope;
     long hislope;
     long numslope;			//-1 if more than 100 categories
     long slopes[100];
     long loaspect;
     long hiaspect;
     long numaspect;		//-1 if more than 100 categories
     long aspects[100];
     long lofuel;
     long hifuel;
     long numfuel;			//-1 if more than 100 categories
     long fuels[100];
     long locover;
     long hicover;
     long numcover;			//-1 if more than 100 categories
     long covers[100];
     long loheight;
     long hiheight;
     long numheight;		//-1 if more than 100 categories
     long heights[100];
     long lobase;
     long hibase;
     long numbase;			//-1 if more than 100 categories
     long bases[100];
     long lodensity;
     long hidensity;
     long numdensity;		//-1 if more than 100 categories
     long densities[100];
     long loduff;
     long hiduff;
     long numduff;			//-1 if more than 100 categories
     long duffs[100];
     long lowoody;
     long hiwoody;
     long numwoody;			//-1 if more than 100 categories
     long woodies[100];
	long numeast;
	long numnorth;
	double EastUtm;
	double WestUtm;
	double NorthUtm;
	double SouthUtm;
	long GridUnits;        // 0 for metric, 1 for English
	double XResol;
	double YResol;
	short EUnits;
	short SUnits;
	short AUnits;
	short FOptions;
	short CUnits;
	short HUnits;
	short BUnits;
	short PUnits;
     short DUnits;
     short WOptions;
     char ElevFile[256];
     char SlopeFile[256];
     char AspectFile[256];
     char FuelFile[256];
     char CoverFile[256];
     char HeightFile[256];
     char BaseFile[256];
     char DensityFile[256];
     char DuffFile[256];
     char WoodyFile[256];
     char Description[512];
}headdata;


bool		TestForLCPVersion1();
bool 	TestForLCPVersion2();
headdata  GetHeaderInformation();					// retrieve header information
void		SetHeader(headdata *data);
void 	ReadHeader2();
void 	ReadHeader();	// only for version 4
bool		ResampleLandscape(double NewRes);
long 	HaveCrownFuels();
long 	HaveGroundFuels();
size_t 	GetHeadSize();
celldata 	CellData(double east, double north, celldata &cell, crowndata &cfuel, grounddata &gfuel, long *posit);
void 	GetCellDataFromMemory(long posit, celldata &cell, crowndata &cfuel, grounddata &gfuel);
bool 	SetCellData(double xpt, double ypt, celldata *cell, crowndata *crown, grounddata *ground);
void 	SetBinaryCellData(unsigned long row, unsigned long col, unsigned long num, short *data);
bool 	GetBinaryCellData(unsigned long row, unsigned long col, unsigned long num, short *data);
long		GetCellPosition(double east, double north);
void 	SetFilePosition(long position);
bool 	HaveCustomFuelModels();
bool 	HaveFuelConversions();
bool 	NeedCustFuelModels();
bool 	NeedConvFuelModels();
void 	SetCustFuelModelID(bool True_False);
void 	SetConvFuelModelID(bool True_False);
long 	DurationResetAtRestart(long YesNo);
long 	IgnitionResetAtRestart(long YesNo);
long 	RotateSensitiveIgnitions(long YesNo);
long      ShowFiresAsGrown(long YesNo);
void 	SetVisibleTheme(long ThemeNumber);
long 	GetVisibleTheme();

FILE * 	GetLandFile();
bool 	OpenLandFile();
bool 	ReOpenLandFile();
bool 	OpenEditLandFile();
void 	CloseLandFile();
void 	CloseEditLandFile();
void 	SetLandFileName(char* FileName);
char *	GetLandFileName();
void 	SetLCPFileNameJustMade(char *FileName);
char *	GetLCPFileNameJustMade();
bool		SaveLCPMemoryToFile(char *NewName);
bool		WriteAsciiLCPTheme(long ThemeNum, long Type, char *Name);

double 	GetWestUtm();
double 	GetEastUtm();
double 	GetSouthUtm();
double 	GetNorthUtm();
double  	GetLoEast();
double 	GetHiEast();
double 	GetLoNorth();
double	GetHiNorth();
long 	GetLoElev();
long 	GetHiElev();
long 	GetNumEast();
long 	GetNumNorth();
long      GetFuelCats(long ThemeNo, long *cats);
double 	ConvertEastingOffsetToUtm(double input);
double 	ConvertNorthingOffsetToUtm(double input);
double 	ConvertUtmToEastingOffset(double input);
double 	ConvertUtmToNorthingOffset(double input);
char*	GetHeaderDescription();

long		GetConditMonth();
long		GetConditDay();
void 	SetConditMonth(long input);
void 	SetConditDay(long input);
long		GetConditMinDeficit();
long		GetLatitude();
long		GetStartMonth();
long		GetStartDay();
long		GetStartHour();
long		GetStartMin();
long		GetStartDate();
void 	SetStartMonth(long input);
void 	SetStartDay(long input);
void 	SetStartHour(long input);
void 	SetStartMin(long input);
void 	SetStartDate(long input);
long		GetEndMonth();
long		GetEndDay();
long		GetEndHour();
long		GetEndDate();
long		GetEndMin();
void 	SetEndMonth(long input);
void 	SetEndDay(long input);
void 	SetEndHour(long input);
void		SetEndMin(long input);
void 	SetEndDate(long input);
long		GetMaxMonth();
long		GetMaxDay();
long		GetMaxHour();
long		GetMinMonth();
long		GetMinDay();
long		GetMinHour();
long		GetJulianDays(long Month);
double 	ConvertActualTimeToSimtime(long mo, long dy, long hr, long mn, bool FromCondit);
void 	ConvertSimtimeToActualTime(double SimTime, long *mo, long *dy, long *hr, long *mn, bool FromCondit);
void 	SetSimulationDuration(double minutes);
double	GetSimulationDuration();
bool		UseConditioningPeriod(long YesNo);
bool		EnvironmentChanged(long Changed, long StationNumber, long FuelSize);
long 	GetMoistCalcInterval(long FM_SIZECLASS, long CATEGORY);
void 	SetMoistCalcInterval(long FM_SIZECLASS, long CATEGORY, long Val);
bool		PreCalcMoistures(long YesNo);

char 	*GetStoredColorFile();
void 	GetStoredColorFile(char *Filename);
bool 	InitializePalette();
LOGPALETTE* GetPalette();
void 	DestroyPalette();
bool 	AccessHDCPalette(HDC hDC, int ONOFF);
//bool 	AccessTDCPalette(TDC& hDC, int ONOFF);
bool 	HavePalette();
void 	SetPaletteColor(int Entry, BYTE Red, BYTE Green, BYTE Blue);
COLORREF 	GetPaletteColor(int Red, int Green, int Blue);



bool SetAtmosphereGrid(long NumGrids);
AtmosphereGrid* GetAtmosphereGrid();
long AtmosphereGridExists();

//----------------------------------------------------------
//
//   Burn Period functions
//
//----------------------------------------------------------

typedef struct
{
	long Month;
     long Day;
     long Start;
     long End;
} AbsoluteBurnPeriod;


typedef struct
{
	double Start;
     double End;
} RelativeBurnPeriod;

void AddDownTime(double time);
double GetDownTime();
bool AllocBurnPeriod(long num);
long GetNumBurnPeriods();
void FreeBurnPeriod();
void SetBurnPeriod(long num, long mo, long dy, long start, long end);
bool GetBurnPeriod(long num, long *mo, long *dy, long *start, long *end);
bool InquireInBurnPeriod(double SimTime);
void ConvertAbsoluteToRelativeBurnPeriod();


//----------------------------------------------------------
//
//   Coarse Woody Data Structures and Access Functions
//
//----------------------------------------------------------

typedef struct
{// initial fuel moistures by fuel type
	bool FuelMoistureIsHere;
	long TL1;
	long TL10;
	long TL100;
	long TLLH;
	long TLLW;
} InitialFuelMoisture;


bool SetInitialFuelMoistures(long model, long t1, long t10, long t100, long tlh, long tlw);
bool GetInitialFuelMoistures(long model, long *t1, long *t10, long *t100, long *tlh, long *tlw);
long GetInitialFuelMoisture(long Model, long FuelClass);
bool InitialFuelMoistureIsHere(long Model);


//----------------------------------------------------------
//
//   Coarse Woody Data Structures and Access Functions
//
//----------------------------------------------------------

#define MAXNUM_COARSEWOODY_MODELS 100
#define CWD_COMBINE_NEVER		1
#define CWD_COMBINE_ALWAYS		2
#define CWD_COMBINE_ABSENT		3


typedef struct
{
     double SurfaceAreaToVolume;
     double Load;
     double HeatContent;
     double Density;
     double FuelMoisture;
} WoodyData;


struct CoarseWoody
{
	long Units;
     double Depth;
     double TotalWeight;
	long NumClasses;
     char Description[64];
     WoodyData *wd;

     CoarseWoody();
};


bool AllocCoarseWoody(long ModelNumber, long NumClasses);
void FreeCoarseWoody(long ModelNumber);
void FreeAllCoarseWoody();
bool SetWoodyData(long ModelNumber, long ClassNumber, WoodyData *wd, long Units);
bool SetWoodyDataDepth(long ModelNumber, double depth, char *Description);
bool SetNFFLWoody();
void GetWoodyData(long WoodyModelNumber, long SurfModelNumber, long *NumClasses, WoodyData *woody, double *Depth, double *TotalLoad);
long GetWoodyDataUnits(long ModelNumber, char *Description);
double GetWoodyFuelMoisture(long ModelNumber, long SizeClass);
//double *GetAllCurrentMoistures(long *NumMx);
void GetCurrentFuelMoistures(long fuelmod, long woodymod, double *MxIn, double *MxOut, long NumMx);
double WeightLossErrorTolerance(double value);
long WoodyCombineOptions(long Options);

//---------------------------------------------------------------
//	Thread Functions
//---------------------------------------------------------------

long GetMaxThreads();
void SetMaxThreads(long numthreads);
bool	CanModifyInputs(long YesNo);


//------------------------------------------------------------------------------
//	Stuff for FlameMap
//------------------------------------------------------------------------------

#define FLAMELENGTH 	0
#define SPREADRATE 		1
#define INTENSITY 		2
#define HEATAREA		3
#define CROWNSTATE 		4
#define SOLARRADIATION 	5
#define FUELMOISTURE1	6
#define FUELMOISTURE10	7
#define MIDFLAME		8
#define HORIZRATE		9
#define MAXSPREADDIR	10
#define ELLIPSEDIM_A	11
#define ELLIPSEDIM_B	12
#define ELLIPSEDIM_C	13
#define MAXSPOT          14
#define RELATIVEDIR		20
#define ABSOLUTEDIR		21

#define NUMOUTPUTS		15

float * 	GetThreadLayer(long Layer, long row);
void 	SetOffsetFromMax(double degrees);
double 	GetOffsetFromMax();
bool 	SelectOutputLayer(long Layer, bool truefalse);
bool 	GetOutputOption(long Layer);
void 	WriteMapCell(long Layer, long row, long col, double value);
void 	WriteFriskToDisk();
void 	WriteOutputLayerToDisk(long Layer);
void 	FreeOutputLayer(long Layer);
void 	SetOutputBaseName(char *name);
char *	GetOutputBaseName();
void 	SetConstWind(double Speed, double Dir);
bool 	IsConstWinds();
double 	GetConstWindSpeed();
double 	GetConstWindDir();
long 	ConstFuelMoistures(long val);
void		SetWeatherFileName(char *name);
void		SetFuelMoistureFileName(char *name);
void 	SetWindFileName(char *name);
void 	SetCustFuelModelName(char *name);
char *	GetCustFuelModelName();
char *	GetWeatherFileName();
char *	GetFuelMoistureFileName();
char * 	GetWindFileName();
long		GetOutputDirection();
void 	SetOutputDirection(long dir);
long		GetThreadOpts();
void 	SetThreadOpts(long num);
void 	SetThreadProgress(long ThreadNum, double Fract);
double 	*GetThreadProgress();

//-----------------------------------------------------------------
//
//   new fuel model support
//
//------------------------------------------------------------------

typedef struct
{
     long number;
     char code[8];
     double h1;
     double h10;
     double h100;
     double lh;
     double lw;
     long dynamic;
     long sav1;
     long savlh;
     long savlw;
     double depth;
     double xmext;
     double heatd;
     double heatl;
     char desc[256];
}NewFuel;

void ResetNewFuels();
bool SetNewFuel(NewFuel *newfuel);
bool GetNewFuel(long number, NewFuel *newfuel);
void InitializeNewFuel();
bool IsNewFuelReserved(long number);


#endif //FARSITEGLOBVARS

