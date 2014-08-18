#include "FlamMap_DLL.h"

#include "flm4.hpp"
//#include "fsxwatm.h"
#include "fsxw.hpp"
//#include <sys/timeb.h>
#include <time.h>
#include "../Common/icf_def.h" 
//#include "fsxsync.h"
#include "gdal_priv.h"
#include <inttypes.h>
#include <string.h>


//#include "flammap_dll.h"
//#ifdef BUILD_FLAMMAPDLL 
//#define FLAMMAPDLL_EXPORT __declspec(dllexport) 
//#else 
//#define FLAMMAPDLL_EXPORT __declspec(dllimport) 
//#endif 

#define MAXNUM_COARSEWOODY_MODELS 100
#define NUMOUTPUTS		21
#define NUM_STATICOUTPUTS		17
#define FM_INTERVAL_TIME  	0
#define FM_INTERVAL_ELEV		1
#define FM_INTERVAL_SLOPE	2
#define FM_INTERVAL_ASP		3
#define FM_INTERVAL_COV		4
#define RELATIVEDIR		20
#define ABSOLUTEDIR		21

class CFlamMap;

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


#define CWD_COMBINE_NEVER		1
#define CWD_COMBINE_ALWAYS		2
#define CWD_COMBINE_ABSENT		3


static char* unitStrings[] = 
{
	"Meters",			//0
	"Feet",
	"Degrees",
	"Percent",
	"Class",			//4
	"kg/m^3",
	"Mg/Ha",
	"Tons/Acre",
	"lb/ft^3",
	"kg/m^3*100",
	"lb/ft^3*1000",
	"CC from East", //11
	"Meters*10",			//12
	"Feet*10",				//13
};

static short elevUnits[] = {0, 1, -1};
static short slopeUnits[] = {2, 3, -1};
static short aspectUnits[] = {5, 11, 2, -1};
static short fuelUnits[] = {4, -1};
static short coverUnits[] = {4, 3, -1};
static short heightUnits[] = {0, 1, 12, 13, -1};
static short baseUnits[] = {0, 1, 12, 13, -1};
static short bulkUnits[] = {5, 8, 9, 10, -1};
static short duffUnits[] = {6, 7, -1};
static short woodyUnits[] = {4, -1};
static short genericUnits[] = {4, -1};

static short *themeUnits[11] = 
{
	elevUnits,
	slopeUnits,
	aspectUnits,
	fuelUnits,
	coverUnits,
	heightUnits,
	baseUnits,
	bulkUnits,
	duffUnits,
	woodyUnits,
	genericUnits,
};
/*ALM
class CLegendEntry
{
public:
	CLegendEntry(float _val = 0, COLORREF _color = 0, int _frequency = 0);
	~CLegendEntry();
	COLORREF GetColor();
	float GetVal();
	void SetColor(COLORREF _color);
	int GetFrequency();
	void SetFrequency(int _frequency);
	void SetVal(float _val);
private:
	COLORREF color;
	float val;
	int frequency;
};
*/
/*
class CColorRamp
{
public:
	CColorRamp();
	~CColorRamp();
	void Create(int _nColors, COLORREF *_colors);
	int nColors;
	COLORREF *colors;
};
*/
/*ALM
class CLegendData  
{
	friend class CFlamMap;
	//ALM friend class CPredefinedLegend;
public:
	CLegendData(CFlamMap *_pFlamMap, int _themeType);
	virtual ~CLegendData();
	//ALM void ApplyColorRamp(CColorRamp *ramp);
	void ChangeUnits(int destUnits);
	int numEntries;
	int decimalPlaces;
	BOOL GetCategorical();
	BOOL SetCategorical(BOOL isCat);
	BOOL categorical;
	void RampColors(COLORREF top, COLORREF bottom, int nColors, COLORREF *colors);
	COLORREF MapColor(double val);
	void DeleteEntries();
	void AddEntry(CLegendEntry *entry);
	double GetLo();
	void SetLo(double _lo);
	double GetHi();
	void SetHi(double _hi);
	short GetUnits();
	void SetUnits(short _units);
	char *GetUnitsString();
	CLegendEntry **entries;
	int GetThemeType();
	int SetThemeType(int _tt);
	void LoadDefault();
	char title[64];
	//CTheme *theme;
private:
	short units;
	double lo;
	double hi;
	int themeType;
	CFlamMap *pFlamMap;

//ALM public:
	// Applies a predefined legend to an existing legend already constructed
	//ALM void ApplyPredefinedLegend(CPredefinedLegend * preLegend);
};
/*ALM
class CPredefinedLegend : public CLegendData
{
public:
	CPredefinedLegend();
	CPredefinedLegend(CLegendData *lData);
	char legendName[128];
	int themeType;
};
*/
inline double 	pow2(double base) {return base*base;}              			// returns square of base

struct FuelConversions
{
	int Type[257];            // each fuel type contains a fuel model corresponding
	FuelConversions();		 // load defaults
};

typedef struct
{// initial fuel moistures by fuel type
	bool FuelMoistureIsHere;
	int TL1;
	int TL10;
	int TL100;
	int TLLH;
	int TLLW;
} InitialFuelMoisture;

typedef struct
{// structure for holding weather/wind stream data max and min values
	int 	wtr;
	int 	wnd;
	int 	all;
}streamdata;

#ifdef werwerwre
typedef struct
{
	int 	mo;
	int 	dy;
	double 	rn;
	int 	t1;
	int 	t2;
	double 	T1;
	double 	T2;
	int 	H1;
	int 	H2;
	double 	el;
     int 	tr1;
     int		tr2;
} WeatherData;

typedef struct
{// wind data structure
	int 	mo;
	int 	dy;
	int 	hr;
	double 	ws;		// windspeed mph
	int 	wd;		// winddirection azimuth
	int 	cl;		// cloudiness
}WindData;
#endif 

typedef struct
{// header for landscape file
	int CrownFuels;       // 10 if no crown fuels, 11 if crown fuels exist
	int latitude;
	double loeast;
	double hieast;
	double lonorth;
	double hinorth;
	int loelev;
	int hielev;
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
}headdata2;


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
	int Units;
     double Depth;
     double TotalWeight;
	int NumClasses;
     char Description[64];
     WoodyData *wd;

     CoarseWoody();
};

typedef struct
{
     int number;
     char code[8];
     double h1;
     double h10;
     double h100;
     double lh;
     double lw;
     int dynamic;
     int sav1;
     int savlh;
     int savlw;
     double depth;
     double xmext;
     double heatd;
     double heatl;
     char desc[256];
}NewFuel;

class CFMPRect
{
public:
	CFMPRect();
	CFMPRect(int l, int t, int r, int b);
	int Width();
	int Height();
	int left;
	int top;
	int right;
	int bottom;
};


/* .-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
int const iC_MCR = 50; /* Size of array ar_MCR[] */

class FlamMap
{

	    int instanceID;
     Burn *burn;
//     HANDLE hMoistThread;
     //ALM HANDLE hBurnThread;
//     double MaxTime, MoistSimTime;

     //void ThreadOptions();

//     static unsigned __stdcall RunMoistThread(void *);
//     void MoistThread();
//     void StartMoistThread();

     static unsigned int RunBurnThread(void *);
     void BurnThread();
	    int critsec;
	    bool CreateLandscapeFileCS();
	    void FreeLandscapeFileCS();

public:

/* Conditioning Fuel Moisture related */ //ALM-everything commented out is from me
//   CFMC cfmc;
//   int RunConditioning ();
//   int FMC_LoadInputs ( FlamMap *fm, ICF *icf, headdata *lcp, CFMC *cfmc, char cr_ErrMes[]);
//   double GetMaxTime ();
//   char *Get_CondDLLErrMes (); 
//   char cr_CondErrMes[1000]; /* error with Cond DLL inputs, or terminate simulation */
//   int Load_RAWS(int i_MthStr, int i_DayStr, int *ai_Year);
    
    FlamMap();
   ~FlamMap();

/*..........................................................*/    
/* Moisture Conditioning Reuse Array */
//   FireEnvironment2  *ar_MCR[iC_MCR];

//   inline int MCR_Limit() { return iC_MCR; }
//   static const int e_MCRInit = -1; /* used in lx_MCR */
//   static const int e_MCRSave = -2; 
//   int lx_MCR; 

//#define MCR_Reuse "Reuse"
//#define MCR_Save  "Save" 
//   char cr_MCR[50];
  
//   bool MCR_SaveCondMaps (int iX_MCR);

//   void  CloaseFmsThreads ();  

   int FSPRo_ConditionInputs (char cr_PthFN[]);
//   int LoadCondition(d_ICF *a_ICF);


//   CWindNinja2 WN2;

 	// CRITICAL_SECTION lcpCS;
	  void SetInstanceID(int id);
/*..........................................................*/
   ICF icf;          /* input command file class  */
 
    float **Get_windDirGrid ();  
    float **Get_windSpdGrid (); 
    bool has_GriddedWind ();
    bool GridWindRowCol (int *Rows, int *Cols) ;  
/*...........................................................*/
                                                   
  int    LoadInputFile  (char cr_PthFN[]) ;
  char  *LoadInputError (int i_Num) ;
  void    LoadFuelMoist ();
//  int    LoadWindData ();
//  int    LoadWeatherData ();
	bool AccelerationState;
	bool AccelerationON();

	double 	redros[257]; 
	void InitializeRosRed();
	double GetRosRed(int fuel);
	void SetRosRed(int fuel, double rosred);

  bool CrowningOK;
  bool	EnableCrowning(int Crowning);

  bool SpottingOK;
  bool EnableSpotting(int Spotting);

	int CrownFireCalculation;
	int		GetCrownFireCalculation();
	int		SetCrownFireCalculation(int Type);

	double 	DynamicDistanceResolution;
	double GetDynamicDistRes();
	void SetDynamicDistRes(double input);

	bool ConstBack;
	bool	ConstantBackingSpreadRate(int Back);

  void   LoadCrownFireMethod (char cr_CroFirMet[]);
  void   LoadFoliarMoist(int i_FolMoi);
  void   LoadNumProcessors (int i_NumPro); 
//  int    LoadWeatherStream (d_ICF *a_ICF);
//  int    LoadWindStream(d_ICF *a_ICF);
 // int    LoadGridWinds (int *ai); 
	int LoadGridWindsWN2 (int *ai);
 int LoadGridWindsFromFiles (int *ai);

  int LoadCustomFuelsData(); 
/*...........................................................*/

	 int SetAnalysisArea(double tEast, double tWest, double tNorth, double tSouth);
	 int CritiqueTXT(char *FileName);
	 int CritiqueHTML(char *FileName);
	 //ALM int CritiquePDF(char *FileName);
	void ResetBurn();
 	CFMPRect analysisRect;
     void ResetThreads();
    bool LoadLandscapeFile(char *FileName, CFlamMap *caller);
	bool LoadLCPHeader(headdata head);
	bool LoadLandscape(short *landscape);
     bool LoadFuelMoistureFile(char *FileName);
     int LoadCustomFuelFile(char *FileName);
     bool LoadWeatherFile(char *FileName);
     bool LoadWindFile(char *FileName);
     bool LoadOutputFile(char *FileName);
     bool SetDates(int StartMonth, int StartDay, int StartHour,
                         int EndMonth, int EndDay, int EndHour);
     void SelectOutput(int LayerID, bool On);
     bool LoadDateFile(char *FileName);
     bool InquireOutputSelections();
	int SetMoistures(int fuelModel, int _fm1, int _fm10, int _fm100,
		int _fmHerb, int _fmWoody);
	void SetAllMoistures(int _fm1, int _fm10, int _fm100,
		int _fmHerb, int _fmWoody);
//     void ResetFlamMap();
//     bool LoadLCP();
     void SelectOutputs();
    void RunFlamMap ();
  
	void 	SetFoliarMoistureContent(int Percent);
	void GetDefaultCrownChx(double *Height, double *Base, double *Density);
	double GetDefaultCrownHeight();
	double GetDefaultCrownBase();
	double GetDefaultCrownBD(short cover);
	double GetAverageDBH();
	double GetFoliarMC();
	int GetTolerance();
	int GetCanopySpecies();
	bool LinkDensityWithCrownCover(int TrueFalse);
	celldata CellData(double east, double north, celldata &cell, crowndata &cfuel, grounddata &gfuel, int64_t *posit);
	inline int GetTheme_DistanceUnits(){return Header.GridUnits;}
	char *GetTheme_UnitsString(short DataTheme);
	inline short	GetTheme_Units(short DataTheme)
	{

		switch(DataTheme)
		 {	case 0: return Header.EUnits;
			case 1: return Header.SUnits;
			case 2: return Header.AUnits;
			case 3: return Header.FOptions;
			case 4: return Header.CUnits;
			case 5: return Header.HUnits;
			case 6: return Header.BUnits;
			case 7: return Header.PUnits;
			case 8: return Header.DUnits;
			case 9: return Header.WOptions;
		 }

		 return 0;
	}

	short GetTheme_NumCategories(short DataTheme);
	int GetTheme_HiValue(short DataTheme);
	int GetTheme_LoValue(short DataTheme);
	char *GetTheme_FileName(short DataTheme);
	inline int 	HaveCrownFuels(){return Header.CrownFuels-20;}
	inline int 	HaveGroundFuels(){return Header.GroundFuels-20;}
	int 	CheckCellResUnits();					// returns 0 for metric 1 for English
	inline double 	GetCellResolutionX(){return Header.XResol;}                      	// return landscape X cell dimension
	inline double 	GetCellResolutionY(){return Header.YResol;}                              	// return landscape Y cell dimension
	double 	MetricResolutionConvert();                 	// 3.28.. for english, 1.0 for metric
	//void FlamMap::ReadHeader2();
	void    ReadHeader2();
	bool 	NeedCustFuelModels();
	bool 	NeedConvFuelModels();
	void 	SetCustFuelModelID(bool True_False);
	void 	SetConvFuelModelID(bool True_False);
	bool 	HaveCustomFuelModels();
	bool 	HaveFuelConversions();
	inline int 	GetNumEast(){return analysisRect.Width();}
	inline int 	GetNumNorth(){return analysisRect.Height();}
	inline double 	GetWestUtm(){return analysisLoEast;}
	inline double 	GetEastUtm(){return analysisHiEast;}
	inline double 	GetSouthUtm(){return analysisLoNorth;}
	inline double 	GetNorthUtm(){return analysisHiNorth;}
	inline double  GetLoEast(){return analysisLoEast;}
	inline double 	GetHiEast(){return analysisHiEast;}
	inline double 	GetLoNorth(){return analysisLoNorth;}
	inline double	GetHiNorth(){return analysisHiNorth;}
	int GetLatitude();
	void 	SetLandFileName(char* FileName);
	char *	GetLandFileName();
// 	int Chrono(double SIMTIME, int *hour, double *hours, bool RelCondit);

//private:
	void SetCanopyChx(double Height, double CrownBase, double BulkDensity, double Diameter,
			   int FoliarMoisture, int Tolerance, int Species);
	CanopyCharacteristics CanopyChx;
	bool CrownDensityLinkToCC;    // crown bulk density is a function of canopy cover
	short *landscape;
	bool CantAllocLCP;
	FILE *landfile;
	size_t headsize;
	headdata Header;
	headdata2 Header2;
	int NumVals;
	int64_t OldFilePosition;	//__thread 
	bool NEED_CUST_MODELS;	// custom fuel models
	bool HAVE_CUST_MODELS;
	bool NEED_CONV_MODELS;     // fuel model conversions
	bool HAVE_CONV_MODELS;
	void ResetFuelConversions();
	FuelConversions fuelconversion;
	int GetFuelConversion(int fuel);
	int SetFuelConversion(int From, int To);
	void ReadHeader();
	int ReadGeoTiff(char *geoTiffName);
	int ReadGdalLCP(char *lcpName);
	int LoadLandscapeFromGDAL(GDALDataset *poSrcDS);
	size_t GetHeadSize();
	void GetCellDataFromMemory(int64_t posit, celldata &cell, crowndata &cfuel, grounddata &gfuel);

	float GetLayerValueByCell(int _layer, int col, int row);

	int64_t GetCellPosition(double east, double north);
	int    GetFuelCats(int ThemeNo, int *cats);
	//bool SetAtmosphereGrid(int NumGrids);
	//AtmosphereGrid* GetAtmosphereGrid();
	//int AtmosphereGridExists();
	//AtmosphereGrid *AtmGrid;			// pointer to AtmGrid;
	double 	ConvertEastingOffsetToUtm(double input);
	double 	ConvertNorthingOffsetToUtm(double input);
	double 	ConvertUtmToEastingOffset(double input);
	double 	ConvertUtmToNorthingOffset(double input);
	char*	GetHeaderDescription();
	int 	GetLoElev();
	int 	GetHiElev();
	bool 	OpenLandFile();
	char LandFName[256];
	void 	CloseLandFile();
	void GetCurrentFuelMoistures(int fuelmod, int woodymod, double *MxIn, double *MxOut, int NumMx);
	int ResampleLandscape(double NewRes);
	//int TrimLandscape();

	NewFuel newfuels[257];		
	CoarseWoody coarsewoody[MAXNUM_COARSEWOODY_MODELS];
	double WeightLossErrorTol;	// Mg/ha
	void 	FreeOutputLayer(int Layer);
//	void 	FreeWeatherData(int StationNumber);
	void 	FreeWindData(int StationNumber);
	bool 	SelectOutputLayer(int Layer, bool truefalse);
	inline int 	GetMoistCalcInterval(int FM_SIZECLASS, int CATEGORY){return MoistCalcInterval[FM_SIZECLASS][CATEGORY];}

	  
 void 	SetMoistCalcInterval(int FM_SIZECLASS, int CATEGORY, int Val);
	int 	ConstFuelMoistures(int val);
	int		GetJulianDays(int Month);
 int  GetJulianDate (int Month, int Day); 
	int		GetStartMonth();
	int		GetStartDay();
	int		GetStartHour();
	int		GetStartMin();
	int		GetStartDate();
	int		GetEndMonth();
	int		GetEndDay();
	int		GetEndHour();
	int		GetEndDate();
 int		GetEndMin();
	int SetStartProcessor(int _procNum);
	int GetStartProcessor();
	int StartProcessor;
	int GetMaxThreads();
	void SetMaxThreads(int numthreads);
	int MaxThreads;

		int GetThreadOpts();
	int 	ThreadOpts;



	void 	WriteFriskToDisk();
	bool 	GetOutputOption(int Layer);
	int 	WriteOutputLayerToDisk(int Layer);
	int  WriteOutputLayerToDisk(int Layer, char *_fName);
	int WriteWindGridAscii(int Layer, char *_fName);
	int WriteWindGridBinary(int Layer, char *_fName);

	char *GetOutputLayerName32char(int layer, char *textBuf);
	int  WriteOutputLayerBinary(int Layer, char *_fName);
	void 	SetOutputBaseName(char *name);
	InitialFuelMoisture fm[257];
	bool SetInitialFuelMoistures(int model, int t1, int t10, int t100, int tlh, int tlw);
	bool GetInitialFuelMoistures(int model, int *t1, int *t10, int *t100, int *tlh, int *tlw);
	int GetInitialFuelMoisture(int Model, int FuelClass);
	bool InitialFuelMoistureIsHere(int Model);
	int 	AccessFuelModelUnits(int Val);
	int FuelModelUnits;
	void ResetNewFuels();
	bool SetNewFuel(NewFuel *newfuel);
	bool GetNewFuel(int number, NewFuel *newfuel);
	void InitializeNewFuel();
	bool IsNewFuelReserved(int number);
//	int 	AllocWeatherData(int StatNum, int NumObs);
	int 	GetOpenWeatherStation();
	int 	SetWeatherData(int StationNumber, int NumObs, int month, int day,
						double rain, int time1, int time2, double temp1, double temp2,
						int humid1, int humid2, double elevation, int tr1, int tr2);
//	int 	GetWeatherMonth(int StationNumber, int NumObs);
//	int 	GetWeatherDay(int StationNumber, int NumObs);

//	int 	GetNumStations();		  			// retrieve number of weather/wind stations


//	int 	AllocWindData(int StatNum, int NumObs);
//	int 	GetOpenWindStation();
	int 	SetWindData(int StationNumber, int NumObs, int month, int day,
					  int hour, double windspd, int winddir, int cloudy);
	int 	GetWindMonth(int StationNumber, int NumObs);
	int 	GetWindDay(int StationNumber, int NumObs);
//	int 	GetWindHour(int StationNumber, int NumObs);
//	double 	GetWindSpeed(int StationNumber, int NumObs);
//	int 	GetWindDir(int StationNumber, int NumObs);
//	int 	GetWindCloud(int StationNumber, int NumObs);
//	int 	GetMaxWindObs(int StationNumber);
//	void		SetWindUnits(int StationNumber, int Units);
//	void 	SetWeatherUnits(int StationNumber, int Units);
//	int		GetWindUnits(int StationNumber);
//	int 	GetWeatherUnits(int StationNumber);
	void 	SetStartMonth(int input);
	void 	SetStartDay(int input);
	void 	SetStartHour(int input);
	void 	SetStartMin(int input);
	void 	SetStartDate(int input);
	void 	SetEndMonth(int input);
	void 	SetEndDay(int input);
	void 	SetEndHour(int input);
	void	SetEndMin(int input);
	void 	SetEndDate(int input);
//	bool	EnvironmentChanged(int Changed, int StationNumber, int FuelSize);
	int WoodyCombineOptions(int Options);
//	bool IndividualFuelMoisturesSet(void);
	void WriteThreadLayerVal(int Layer, int row, int col, float val);

	float 	*outlayer[NUM_STATICOUTPUTS];//={0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//CFile	outFiles[NUM_STATICOUTPUTS];
	FILE * outFiles[NUM_STATICOUTPUTS];
	double * moutFiles[NUM_STATICOUTPUTS];
	int fd;
	int result;
//	WeatherData *wtrdt[5];//={0,0,0,0,0};
//	int MaxWeatherObs[5];//={0,0,0,0,0};
	int NumWeatherStations;
//	streamdata FirstMonth[5];
//	streamdata FirstDay[5];
//	streamdata FirstHour[5];
//	streamdata LastMonth[5];
//	streamdata LastDay[5];
//	streamdata LastHour[5];
// 	WindData *wddt[5];//={0,0,0,0,0};
//	int NumWindStations;
//	int MaxWindObs[5];//={0,0,0,0,0};
	int 	ConstFuelMoisture;
//	int startmonth;
//	int startday;
//	int starthour;
//	int startmin;
//	int startdate;
//	int endmonth;
//	int endday;
//	int endhour;
//	int endmin;
//	int enddate;
	char 	FlameAceBaseName[256];
	int CombineOption;
	int		GetConditMonth();
//	int		GetConditDay();
	void 	SetConditMonth(int input);
	void 	SetConditDay(int input);
//	int		GetConditMinDeficit();
//	int conditmonth;
//	int conditday;
	bool CondPeriod;


	bool	UseConditioningPeriod(int YesNo);
	CoarseWoody NFFLWoody[13];
	CoarseWoody tempwoody;
	void GetWoodyData(int WoodyModelNumber, int SurfModelNumber, int *NumClasses, WoodyData *woody, double *Depth, double *TotalLoad);
	double GetWoodyFuelMoisture(int ModelNumber, int SizeClass);
	bool AllocCoarseWoody(int ModelNumber, int NumClasses);
	void FreeCoarseWoody(int ModelNumber);
	void FreeAllCoarseWoody();
	//bool SetWoodyData(int ModelNumber, int ClassNumber, WoodyData *wd, int Units);
	bool SetWoodyDataDepth(int ModelNumber, double depth, char *Description);
	double WeightLossErrorTolerance(double value);
	int    OutputDirection;
	int		GetOutputDirection();
	void 	SetOutputDirection(int dir);

 void SetSpreadDirection(double North, double Max);

	//float * GetThreadLayer(int Layer, int row);
	void 	SetThreadOpts(int num);
	void 	SetThreadProgress(int ThreadNum, double Fract);
	double 	*GetThreadProgress();
	void ResetProgress();
	double ThreadProgress[64];
	double conditioningProgress;
//	double windNinjaProgress; looks like we'll get prgress thru the CWindNinj2 class
	double GetFuelDepth(int Number);
	void 	SetConstWind(double Speed, double Dir);
	bool 	IsConstWinds();
	double 	GetConstWindSpeed();
	double 	GetConstWindDir();
	double 	ConstWindDirection;
	double 	ConstWindSpeed;
	char 	WindFileName[256];
	char	WeatherFileName[256];
	char 	FuelMoistureFileName[256];
	char    FuelModelName[256];
	double	OffsetFromMax;	// offset in fire spread direction from Rmax
	void 	SetOffsetFromMax(double degrees);
	double 	GetOffsetFromMax();
	void		SetWeatherFileName(char *name);
	void		SetFuelMoistureFileName(char *name);
	void 	SetWindFileName(char *name);
	void 	SetCustFuelModelName(char *name);
	char *	GetCustFuelModelName();
	char *	GetWeatherFileName();
	char *	GetFuelMoistureFileName();
	char * 	GetWindFileName();
	bool      PreserveInactiveEnclaves(int YesNo);
	bool InactiveEnclaves;
	double GetDownTime();
	double DownTime;
	bool LandFileOpen;//=false;
	Acceleration AccelConst;

	void DeleteWindGrids();
	void AllocWindGrids(int wndRows, int wndCols, double wndRes, double wndXLL, double wndYLL);
	void SetWindGridValues(int wndRow, int wndCol, float wndDir, float wndSpeed);
	float GetWindGridSpeed(int wndRow, int wndCol);
	float GetWindGridDir(int wndRow, int wndCol);
	float GetWindGridSpeedByCoord (double xCoord, int yCoord);
	float GetWindGridDirByCoord (double xCoord, int yCoord);

	void GetNextBinFileName(char trg[]);

	int nWindRows;
	int nWindCols;
	double windsResolution;
	double windsXllCorner;
	double windsYllCorner;
	float **windDirGrid;
	float  **windSpeedGrid;
	bool hasGriddedWinds;
	FLAMMAPRUNSTATE runState;
	//FLAMMAPRUNSTATE GetRunState();
	int MoistCalcInterval[4][5];//={{60, 200, 10, 45, 15},           // 1hr
						//{120, 200, 10, 45, 15},          // 10hr
						//{180, 400, 10, 45, 20},          // 100hr
                             // {240, 400, 15, 45, 30}};		   // 1000hr
	bool EnvtChanged[4][5];//={{false, false, false, false, false},
						// {false, false, false, false, false},
						// {false, false, false, false, false},
						// {false, false, false, false, false}};
	//StationGrid grid;

     bool TerminateMoist, TerminateBurn;
     int RunFlamMapThreads();
//     bool RunFuelMoistureCalculations();
     void StartBurnThread();
	double analysisLoEast, analysisHiEast, analysisLoNorth, analysisHiNorth;

	//timing information
   time_t timeLaunch;
   time_t timeFinish;

   //legending support
  // float GetDataUnitsVal(float val, CLegendData *pLegend);
   //float GetDisplayVal(float val, CLegendData *pLegend);
   float ConvertUnits(float val, int srcUnits, int destUnits, int themeType);

   	//CLegendData *legends[10];
	//ALM int GenerateImage(int _layer, char *_fileName);
	//ALM int GenerateLegendImage(int _layer, char *_fileName);

	//fsxsync functions
	bool 	AllocFarsiteEvents(int EventNum, int numevents, char *basename, bool ManReset, bool InitState);
	bool		FreeFarsiteEvents(int EventNum);
	bool 	SetFarsiteEvent(int EventNum, int ThreadNum);
	bool 	ResetFarsiteEvent(int EventNum, int ThreadNum);
	bool 	WaitForFarsiteEvents(int EventNum, int NumEvents, bool All, unsigned int Wait);
	bool 	WaitForOneFarsiteEvent(int EventNum, int ThreadNum, unsigned int Wait);
	//ALM-all this is commented out
	/*
	HANDLE 	GetFarsiteEvent(int EventNum, int ThreadNum);
	//fsxsync Events
	FarsiteEvent 	hBurnEvent;
	FarsiteEvent 	hMoistEvent;
	FarsiteEvent   hBurnupEvent;
	FarsiteEvent	hIntegEvent;
	FarsiteEvent   hCrossEvent;
	FarsiteEvent 	hBurnThreadEvent;
	FarsiteEvent 	hMoistThreadEvent;
	FarsiteEvent   hBurnupThreadEvent;
	FarsiteEvent	hIntegThreadEvent;
	FarsiteEvent	hCrossThreadEvent;
	*/
	//Function to limit output moistures to 2% or greater
	//void MassageMoistures(void);
	inline double MassageMoisture(double inMoist)
	{
		if(inMoist > 0.0 && inMoist < 0.02)
			return 0.02;
		return inMoist;
	}

	bool IsCellBurnable(int row, int col);
	float GetOutputLayerVal(int Layer, int64_t offset);
};
/*ALM
class CrossTab
{
public:
	CrossTab(int _nBins, int *binMins, int *binMaxs);
	~CrossTab();
	void AddObs(short obsVal);
	double Average();
	double OverallAverage();
	int nBins;
	double sigma;
	int nObs;
	int nValidObs;
	int *pMins;
	int *pMaxs;
	int *freqs;
};*/
class IndexItem
{
public:
	int freq;
	int loc;
};
int IndexCompare( const void *arg1, const void *arg2 );
