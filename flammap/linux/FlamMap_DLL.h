#define _POSIX_SOURCE 1
//FlamMap DLL header file
//Exported routines for FlamMap.dll
#pragma once

#define openMPOff
#ifdef BUILD_FLAMMAPDLL 
#define FLAMMAPDLL_EXPORT __declspec(dllexport) 
#else 
#define FLAMMAPDLL_EXPORT __declspec(dllimport) 
#endif 

//#include <windows.h>
//#include <windef.h>
#ifndef BOOL
typedef int                 BOOL;
#endif

#ifndef COLORREF
typedef unsigned int   COLORREF;
#endif

const int MAX_FMP_PATH = 512;
#define NODATA_VAL -9999.0
#define MAX_CATS 100
//native FLamMap Layers
typedef enum LcpLayers{ELEV = 0, SLOPE = 1, ASPECT = 2, FUEL = 3, COVER = 4, HEIGHT = 5, BASE_HEIGHT = 6, BULK_DENSITY = 7, 
	DUFF = 8, WOODY = 9} LCP_LAYER;
typedef enum FlamMapRunState{Stopped = 0, Loading=1, WindNinja = 2, Conditioning = 3, FireBehavior = 4, Complete = 5} FLAMMAPRUNSTATE;

//output layers available
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
#define MAXSPOT         14
#define FUELMOISTURE100 15
#define FUELMOISTURE1000 16
#define MAXSPOT_DIR     17
#define MAXSPOT_DX      18
#define WINDDIRGRID      19
#define WINDSPEEDGRID    20

//forward declaration
class FlamMap;
//ALM class CLegendData;

typedef struct
{// header for landscape file
	int CrownFuels;         // 20 if no crown fuels, 21 if crown fuels exist
     int GroundFuels;		// 20 if no ground fuels, 21 if ground fuels exist
	int latitude;
	double loeast;
	double hieast;
	double lonorth;
	double hinorth;
	int loelev;
	int hielev;
	int numelev; 			//-1 if more than 100 categories
     int elevs[100];
     int loslope;
     int hislope;
     int numslope;			//-1 if more than 100 categories
     int slopes[100];
     int loaspect;
     int hiaspect;
     int numaspect;		//-1 if more than 100 categories
     int aspects[100];
     int lofuel;
     int hifuel;
     int numfuel;			//-1 if more than 100 categories
     int fuels[100];
     int locover;
     int hicover;
     int numcover;			//-1 if more than 100 categories
     int covers[100];
     int loheight;
     int hiheight;
     int numheight;		//-1 if more than 100 categories
     int heights[100];
     int lobase;
     int hibase;
     int numbase;			//-1 if more than 100 categories
     int bases[100];
     int lodensity;
     int hidensity;
     int numdensity;		//-1 if more than 100 categories
     int densities[100];
     int loduff;
     int hiduff;
     int numduff;			//-1 if more than 100 categories
     int duffs[100];
     int lowoody;
     int hiwoody;
     int numwoody;			//-1 if more than 100 categories
     int woodies[100];
	int numeast;
	int numnorth;
	double EastUtm;
	double WestUtm;
	double NorthUtm;
	double SouthUtm;
	int GridUnits;        // 0 for metric, 1 for English
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


//class FLAMMAPDLL_EXPORT CFlamMap
class CFlamMap
{
public:
	CFlamMap();
	//alm ~CFlamMap();

/* FuelCondDLL <---- here to identify and search on any code changes */
/* for Conditioning DLL  */
/* Functions gets error message back when using the DLL */
   //char * Get_CondDLLErrMes();


/*.....................................................*/
/* MCR - Moisture Conditioning Reuse and related stuff */
 // void MCR_Copy(CFlamMap *from);
  void  MCR_Init (); 
 // int   MCR_Limit (); 
 // bool MCR_RunSavOne (int MC );
 // bool  MCR_ifConditioning();
 // void  MCR_SetReuse(int l); 
 // void  MCR_SetSave (int l); 
 // void  MCR_Delete();

 
 int FSPRo_ConditionInputs (char cr_PthFN[]); 

 // bool   WindNinjaRun ( int *ai_Err); 
 // char * WindNinjaError ( int i_Err); 


/* WNS WindNinja Save ......................... */
  float **Get_windDirGrid (); 
  float **Get_windSpdGrid ();    
  bool has_GriddedWinds (); 
  bool GridWindRowCol (int *Rows, int *Cols); 
 
   int CommandFileLoad(char cr_PthFN[]);
   char *CommandFileError(int i_ErrNum); 
	char *GetErrorMessage(int errNum);

	
  void GetWindCorners (double *X, double *Y);
  void AllocWindGrids(int wndRows, int wndCols, double wndRes, double wndXLL, double wndYLL);


	//initialization, associate with a landscape file
	int SetLandscapeFile(char *_lcpFileName);
	int LoadLCPHeader(headdata head);
	int LoadLandscape(short landscape);
	char *GetLandscapeFileName();
	int SetUseDiskForLCP(int set);
	int GetNumCols();
	int GetNumRows();
	double GetWest();
	double GetSouth();
	double GetEast();
	double GetNorth();
	double GetCellSize();
	float GetLayerValue(int _layer, double east, double north);
	float GetLayerValueByCell(int _layer, int col, int row);
	int GetUnits(int _layer);
	//resample landscape at greater resolution for FSPro
	int ResampleLandscape(double NewRes);
	//associate with a subset of the landscape file, uses cell indices (zero based)
	//int SetAnalysisArea(int _cellNumLeft, int _cellNumRight, 
	//	int _cellNumTop, int _cellNumBottom);
	//int GetAnalysisArea(int _cellNumLeft, int _cellNumRight, 
	//	int _cellNumTop, int _cellNumBottom);
	//associate with a subset of the landscape file, uses native landscape 
	//coordinates
	int SetAnalysisArea(double tEast, double tWest, double tNorth, double tSouth);

	//int TrimLandscape();
	//calculation resolution
	double SetCalculationResolution(double _resolution);
	double GetCalculationResolution();

	int SetStartProcessor(int _procNum);
	int GetStartProcessor();
	//number of threads
	int SetNumberThreads(int _numThreads);
	int GetNumberThreads();
	int GetNumberMTTThreads();//get MTT Threads from Inputs file...
	double GetMTT_SpotProbability();//get MTT Spot Probability from Inputs file...
	//fuel moistures
	int SetFuelMoistureFile(char *_fuelMoistureFileName);
	char *GetFuelMoistureFile();
	int SetMoistures(int fuelModel, int _fm1, int _fm10, int _fm100,
		int _fmHerb, int _fmWoody);
	int SetAllMoistures(int _fm1, int _fm10, int _fm100,
		int _fmHerb, int _fmWoody);
	//custom fuels
	//int SetUseCustomFuels(int _trueFalse);
	//int GetUseCustomFuels();
	//custom fuels file name/url
	int SetCustomFuelsFile(char *_customFuelsFileName);
	char *GetCustomFuelsFile();

	//wind inputs
	int SetWindType(int _windType);
	int GetWindType();
	int SetConstWind(double _windSpeed, double _windDir);
	int SetWindDirection(double _windDir);
	double GetWindDirection();
	int SetWindSpeed(double _windSpeed);
	double GetWindSpeed();
	int SetWindVectorThemes(char *_speedFileName, char *_dirFileName);
	char *GetWindVectorSpeedFileName();
	char *GetWindVectorDirectionFileName();

	//constant canopy height
	int SetCanopyHeight(double _height);
	double GetCanopyHeight();

	//constant canopy base height
	int SetCanopyBaseHeight(double _baseHeight);
	double GetCanopyBaseHeight();

	//constant bulk density
	int SetBulkDensity(double _bulkDensity);
	double GetBulkDensity();

	//foliar moisture content
	int SetFoliarMoistureContent(int _moisturePercent);
	double GetFoliarMoistureContent();

	//fuel moisture conditioning
	int SetUseFixedFuelMoistures(int _useFixedFuels);
	int GetUseFixedFuelMoistures();
	int SetWeatherFile(char *_wtrFileName);
	char *GetWeatherFile();
	int SetWindsFile(char *_wndFileName);
	char *GetWindsFile();
	int SetConditioningPeriod (int _startMonth, int _startDay, 
	                          	int _startHour, int _startMinute,
	                          	int _endMonth, int _endDay, 
	                          	int _endHour, int _endMinute);


	int GetConditioningStartMonth();
	int GetConditioningStartDay();
	int GetConditioningStartHour();
	int GetConditioningStartMinute();
	int GetConditioningEndMonth();
	int GetConditioningEndDay();
	int GetConditioningEndHour();
	int GetConditioningEndMinute();

	//crown fire calc type (default = Finney)
	int SetUseScottReinhardt(int _select);
	int GetUseScottReinhardt();

	//spread direction adjustments
	int SetSpreadDirectionFromNorth(int _select, double _degrees);
	int GetUseSpreadDirectionFromNorth();
	double GetSpreadDirectionFromNorth();

	//output memory or file based
	int SelectOutputReturnType(int _returnType); //0 = memory, 1 = files

	//outputs to calculate
	int SelectOutputLayer(int _layer, int _select);

	/*
	0 = flameLength,
	1 = rateOfSpread,
	2 = firelineIntensity, 
	3 = heatPerUnitArea,
	4 = crownFireActivity,
	5 = solarRadiation,
	6 = 1HrFuelMoisture,
	7 = 10HrFuelMoisture,
	8 = midflameWindspeed,
	9 = horizintalMovementRate,
	10 = spreadDirection,
	11 = ellipticalDimA,
	12 = ellipticalDimB,
	13 = ellipticalDimC,
	14 = Spotting Prob
	*/
	int GetTheme_DistanceUnits();

	//output retrieval
	float *GetOutputLayer(int _layer);
	char *GetTempOutputFileName(int _layer, char buf[], int sz);
	char *GetOutputGridFileName(int _layer, char *buf, int bufLen);
	int WriteOutputLayerToDisk(int _layer, char *name);
	int WriteOutputLayerBinary(int _layer, char *name);

	//Process control
	int CanLaunchFlamMap();
	int LaunchFlamMap(int _runAsync);
	double QueryFlamMapProgress();
	double GetConditioningProgress();
	double GetWindNinjaProgress();
	double GetFireBehaviorProgress();
	bool ResetFlamMapProgress();
	int CancelFlamMap();
	void ResetBurn();
	FLAMMAPRUNSTATE GetRunState();
	int GetTotalRunTime();
	int CritiqueTXT(char *FileName);
	//ALM int CritiquePDF(char *FileName);

	//almint GenerateImage(int _layer, char *_fileName);
	//ALMint GenerateLegendImage(int _layer, char *_fileName);
	int LoadFlamMapTextSettingsFile(char *FileName);
	int LoadFuelConversionFile(char *_fileName);  // returns the number of fuels models converted

	//MTT input helpers (Loaded from a FlamMap inputs file
	double GetMttResolution();
	int GetMttSimTime();
	int GetMttPathInterval();
	char *GetMttIgnitionFile();
	char *GetMttBarriersFile();
	int GetMttFillBarriers();

	//legending helpers
  //ALM float GetDataUnitsVal(float val, CLegendData *pLegend);
  //ALM float GetDisplayVal(float val, CLegendData *pLegend);

	//Header Access helpers....
	int GetLayerHi(int layer);
	int GetLayerLo(int layer);
	int GetLayerHeaderValCount(int layer);
	int GetLayerHeaderVal(int layer, int loc);
	short GetLayerUnits(int layer);
	int GetNumWindRows();
	int GetNumWindCols();
	int GetWindsResolution();

void  GWN_Set_SpeedDir (int WindSpeed, int WindDir);
void  GWN_Set_NoSw (); 
void  GWN_Set_YesSw (); 


void   GWN_Set_Yes (
             float f_Resolution, 
             float f_WindHeight, 
             int l_procs, 
             bool Diurnal,  
             int i_Month, int i_Day, int i_Year,
             int i_Second, int i_Minute, int i_Hour, int i_TimeZone,      
             float f_Temp, float f_CloudCover, 
             float f_Longitude);

 


private:
	char m_strLCPName[MAX_FMP_PATH];
	char m_strCustFuelFileName[MAX_FMP_PATH];
	int m_outputsType;//0 = memory, 1 = files
	FlamMap *pFlamMap;
	bool m_runningFlamMap;
	bool m_moisturesSet;
	float *maxSpotDirLayer;
	float *maxSpotDxLayer;
	float *windNinjaDirLayer;
	float *windNinjaSpeedLayer;
};
