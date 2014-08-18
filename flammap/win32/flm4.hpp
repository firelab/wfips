//#include <owl\owlpch.h>
#ifndef MainHeaderFile
#define MainHeaderFile

//#include <owl\owlpch.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dos.h>
#include <conio.h>
#include <string.h>
//#include <windef.h>
//#include <alloc.h>

#include "newfms.h"
#include "fsxlandt.h"
#include <windows.h>
#include "fsxpfront.h"
#include "cdtlib.h"
//typedef unsigned char BYTE;

typedef struct
{
	double Point;                           // accel const for points
	double Line;						// accel const for lines
	double Limit;						// upper perimeter limit for points
} AccelConstants;


struct Acceleration
{
	AccelConstants ac[261];

	Acceleration();
};


class Crown
{
     double CritRos, CritCFB, Ro;

public:
	double A, Io, SFC, CrownFractionBurned, CrownLoadingBurned;
	double HLCB, FoliarMC, CrownBulkDensity, TreeHeight;
	double FlameLength;

	Crown() { };
	~Crown() { };
	void CrownIgnite(double height, double base, double density);
	double CrownSpread(double avgros, double R10);
	void CrownIntensity(double cros, double *fli);
	void CrownBurn(double avgros, double fli, double AccelConstant);
     double CrownBurn2(double avgros, double fli, double AccelConstant, void *bt);
};


class APolygon
{
public:
	double startx, starty;

	APolygon() {};
	~APolygon() {};
	long Overlap(long count);
	double direction(double xpt1, double ypt1);
};


class StandardizePolygon        // for reordering perimeter from extreme point
{
public:
	StandardizePolygon() {}
	~StandardizePolygon() {}
	long FindExternalPoint(long CurrentFire, long type);
	void ReorderPerimeter(long CurrentFire, long NewBeginning);
	void FindOuterFirePerimeter(long CurrentFire);
	bool Cross(double xpt1, double ypt1, double xpt2, double ypt2,
			 double xpt1n, double ypt1n, double xpt2n, double ypt2n,
			 double *newx, double *newy, long *dup1, long *dup2);
	void DensityControl(long CurrentFire);
     void RemoveDuplicatePoints(long CurrentFire);
     void RemoveIdenticalPoints(long FireNum);
     void BoundingBox(long CurrentFire);
};


class IgnitionCorrect: public APolygon, public StandardizePolygon
{
public:
	IgnitionCorrect();
	~IgnitionCorrect();

	void ReversePoints(long type);
	//void BoundingBox();
	double arp();
};


class XUtilities : public APolygon
{
	long swapnumalloc;
	size_t nmemb;

public:
	long ExNumPts, OldNumPoints;      // USED BY RASTER AND BURN CLASSES, WITH D-METH2
	double *swapperim;                // pter to swap array for perimeter points

	XUtilities();
	~XUtilities();
	void AllocSwap(long NumPoint);
	void FreeSwap();
	void GetSwap(long NumPoint, double *xpt, double *ypt, double *ros, double *fli, double *rct);
	void SetSwap(long NumPoint, double xpt, double ypt, double ros, double fli, double rct);
	void SwapTranz(long writefire, long nump);
	void tranz(long count, long nump);
	void rediscretize(long *count, bool Reorder);
     void RePositionFire(long *firenum);
     void RestoreDeadPoints(long firenum);
};


class CompareRect
{
	double Xlo, Xhi, Ylo, Yhi;
	double xlo, xhi, ylo, yhi;

public:
	double XLO, XHI, YLO, YHI;

	CompareRect() {};
	~CompareRect() {};
     void InitRect(long FireNum);
	void WriteHiLo(long FireNum);
	void ExchangeRect(long FireNum);
	bool XOverlap();
	bool YOverlap();
	bool BoundCross(long Fire1, long Fire2);
	void DetermineHiLo(double xpt, double ypt);
	bool MergeInAndOutOK1(long Fire1, long Fire2);
};


class CrossThread
{
     long CurrentFire, NextFire;
     long ThreadOrder;
     long SpanAStart, SpanAEnd, SpanBStart, SpanBEnd, Terminate;
     long NumCross, NumAlloc;
     long *intersect;
     double *interpoint;
     unsigned ThreadID;
     HANDLE hXEvent;
     APolygon poly;

     bool Cross(double x, double y, double xn, double yn, double cx, double cy,
               double cxn, double cyn, double *nx, double *ny, long *dup1, long *dup2);
     bool ReallocCross(long Num);
     bool AllocCross(long Num);
     void SetInterPoint(long Number, double XCoord, double YCoord, double Ros, double Fli, double Rct);
     void SetIntersection(long Number, long XOrder, long YOrder);
     void FreeCross();
     void CrossCompare();
     static unsigned __stdcall RunCrossThread(void *crossthread);

public:
     CrossThread();
     ~CrossThread();

     void SetRange(long SpanAStart, long SpanAEnd);
     void StartCrossThread(long threadorder, long currentfire, long nextfire);
     long GetNumCross();
     long* GetIsect();
     double* GetIpoint();
};


class Intersections: public XUtilities, public CompareRect, public StandardizePolygon
{
	long SpanAStart, SpanAEnd, SpanBStart, SpanBEnd;
	long NoCrosses, numcross, noffset1, noffset2, readnum, writenum;
	long *intersect;                          // stores array addresses for intersecting perimeter points
	long *crossout;
	long *NewIsect;
	long *AltIsect;
	double *interpoint;                       // stores actual intersection coordinates, ROS, & FLI
	long crossnumalloc, intersectnumalloc, interpointnumalloc;
     long newisectnumalloc;				  // for newclip
     long NumCrossThreads;
     CrossThread *crossthread;
     size_t nmemb;

	//typedef struct
	//{    long number;
	//	double extreme;
	//	void *next;
	//} Extremes;

     bool AllocCrossThreads();
     void FreeCrossThreads();
	bool MergeInAndOutOK2(long Fire1, long Fire2);
     bool SwapBarriersAndFires();
	void AllocIntersection(long Number);
	void FreeIntersection();
	void AllocInterPoint(long Number);
	void FreeInterPoint();
	void GetIntersection(long Number, long *XOrder, long *YOrder);
	void SetIntersection(long Number, long XOrder, long YOrder);
	void GetInterPointCoord(long Number, double *XCoord, double *YCoord);
	void GetInterPointFChx(long Number, double *Ros, double *Fli, double *Rct);
	void SetInterPoint(long Number, double XCoord, double YCoord, double Ros, double Fli, double Rct);
	void AllocCrossout(long Number);
	void FreeCrossout();
	long GetCrossout(long Number);
	void SetCrossout(long Number, long Value);
	long GetSpan(long Number, long ReadFire);
	long intercode(long offcheck,  long xypt);
	void GetOffcheck(long *offcheck);
	void CheckIllogicalExpansions(long CurrentFire);
	//long GetExtremePair(Extremes *First, long count);
	void FindMergeSpans(long FireNum);
	bool CrossCompare(long *CurrentFire, long NextFire);
     bool EliminateCrossPoints(long CurrentFire);
	bool CrossCompare1(long *CurrentFire, long NextFire);
	void BoundaryBox(long NumPoints);
	void FindFirePerimeter(long CurrentFire, long StartPoint);
	bool MergeFire(long *CurrentFire,  long NextFire);
     void MergeBarrier(long *CurrentFire, long NextFire);
     void MergeBarrierNoCross(long *CurrentFire, long NextFire);
	void MergeWrite(long xend,  long readstart,  long readend,  long *xwrite);
     void CheckEnvelopedFires(long Fire1, long Fire2);
	void OrganizeCrosses(long CurrentFire);
     void OrganizeIntersections(long CurrentFire);
     bool TurningNumberOK(long CurrentFire, long StartPoint);
	bool AllocNewIntersection(long NumCross);
	bool FreeNewIntersection();
	void SetNewIntersection(long Alt, long count, long isect1, long isect2);
	void GetNewIntersection(long Alt, long count, long *isect1, long *isect2);

public:
     PostFrontal post;

	Intersections();
	~Intersections();
	double arp(int PerimNum, long count);		// area of fire
     void CrossFires(int check,  long *firenum);	// perform loop-clip and mergers
     void CrossesWithBarriers(long FireNum);		// fires X barriers @in sub-timesteps
	void CleanPerimeter(long CurrentFire);
     void ResetIntersectionArrays();
     void CloseCrossThreads();
};

/*
class OutputFile
{
	LandscapeData ld;

protected:
	FILE *otpfile;
	double x, y, t, f, l, r, h, rx;
     double North, South, East, West;
	long c, d;
	double convf1, convf2, convf3;
	long filepos;                   // record filepositions in
	long FileOutput;
	long numrows, numcols;
     long NumRastAlloc;
     long NumRastData;

     typedef struct
     {    double 	xpt;
     	double	ypt;
     	double 	Time;
     	double 	Fli;
          double    Ros;
          double	Rcx;
          long 	Dir;
          bool 	Write;
     } RastData;
     RastData *rd;

     bool AllocRastData(long Num);
     bool ReallocRastData(long Num);
     void FreeRastData();
     bool SetRastData(double xpt, double ypt, double time, double fli, double ros, double rcx, long dir);
     long GetRelativeRasterLocation(long Type);
     void WriteOptionalFile();

public:
	OutputFile();
	~OutputFile();

	void SelectOutputs(long Type);
	void WriteRastFiles();
	bool RastFile(long Type);
	void SelectMemOutputs(long Type);
	void WriteRastMemFiles();
	bool RastMemFile(long Type);
	void OptionalOutput(bool FromMemory);
	void Calcs(int TYPE);
	void ConvF();
	void InitRasterFiles(long HeaderType);
	void FirstFile(long HeaderType, long Type);
	long GetRasterFileLocation(long Type);
     void GetRasterExtent();
};


class Vectorize: public OutputFile
{
	long CurFire, i, I;
	FILE *vectfile;

public:
	Vectorize();
	~Vectorize();
	void VectData(long CurrentFire, double SimTime);
	void ArcFileFormat();
	void OptionalFileFormat();
	void GetXY();
	void GetRF();
};


class Rasterize : public APolygon, public OutputFile
{
	long count, ExNumPts;
	long parallel;
	long num;
	double W1, W2, W3, W4, Wr1, Wr2, Wr3, Wr4, Wf1, Wf2, Wf3, Wf4;
	double WT, WrT, WfT, WxT, WRT;
	double diffx, diffy, xmid, ymid;
     double PFWest, PFSouth, PFRes;
     double StartX, StartY;
	double box[4][7];
     double TempBox[2];
	double LastTime, CurTime;
     bool SamePoint;
	FILE *rastfile;

	void raster();
	long Overlap();
	bool Interpolate(bool WriteFile);
     void AllDistanceWeighting();
	void TriangleArea();
	void DistWt();
	void TimeWt();
	void AreaWt();
	//void Calcs(int TYPE);
	void FMaxMin(int Coord, double *ptmax, double *ptmin);
	double findmax(double pt1, double pt1n, double pt2, double pt2n);
	double findmin(double pt1, double pt1n, double pt2, double pt2n);
	void rastdata(double east, double north, double *eastf, double *northf, long MaxMIn);
	void PFrastdata(double east, double north, double *eastf, double *northf, long MaxMIn);
	bool Cross(double xpt1, double ypt1, double xpt2, double ypt2,
			 double xpt1n, double ypt1n, double xpt2n, double ypt2n,
			 double *newx, double *newy);

public:

	Rasterize();
	~Rasterize();
	void rasterinit(long CurrentFire, long ExNumPts, double SIMTIME, double TimeIncrement, double CuumTimeIncrement);
     void RasterizePostFrontal(double Resolution, char *FileName, bool ViewPort, long ThemeNumber);
	void RasterReset();
};
*/

class AreaPerimeter: public APolygon
{
public:
	double area, perimeter, sperimeter;
	double cuumslope[2];

	AreaPerimeter();
	~AreaPerimeter();
	void arp( long count);
};



class Mechanix
{
	double lb_ratio, hb_ratio, rateo;		     // rateo is ROS w/o slope or wind
     double b, part1;                             // parameters for phiw
//	void fuelmod(int fuel, double *depth);
	double headback(void);
     double CalcEffectiveWindSpeed();

protected:
	double m_ones, m_tens, m_hundreds, m_livew, m_liveh;				// local copies of FE data for use in spreadrate
	double phiw, phis, phiew, LocalWindDir;
	double FirePerimeterDist, slopespd;

	double accel(double RosE, double RosT, double A, double *avgros, double *cosslope);
     void   TransformWindDir(long slope, double aspectf);
	double vectordir(double aspectf);
	double vectorspd(double *VWindSpeed, double aspectf, long FireType);

public:
	double xpt, ypt, midx, midy, xt, yt, xptn, yptn, xptl, yptl, xdiff, ydiff;
	double vecspeed, ivecspeed, vecdir, m_windspd, m_winddir, m_twindspd;
	double ActualSurfaceSpread, HorizSpread, avgros, cros, fros, vecros, RosT1, RosT, R10;
	double fli, FliFinal, ExpansionRate;
	double timerem, react, savx, cosslope, HSpreadRate;
	double CurrentTime, FlameLength, CrownLoadingBurned, CrownFractionBurned;
	double head, back, flank;

	Mechanix();
	~Mechanix();
	void limgrow(void);							// limits growth to distance checking
	void grow(double ivecdir);					// Richards (1990) differential equation
	void distchek(long CurrentFire);				// checks perimeter and updates distance check
	void ellipse(double iros, double wspeed);		// calculates elliptical dimensions
	void scorrect( long slope, double aspectf);		// slope correction
	double spreadrate( long slope, double windspd, int fuel);	// Rothermel spread equation
     void GetEquationTerms(double *phiw, double *phis, double *b, double *part1);
};

/*
class LandscapeData
{
public:
	int elev, slope, aspect, fuel, cover;               // READ OR DERIVED FROM LANDSCAPE DATA
	double HLCB, CrownBulkDensity, TreeHeight; // READ OR DERIVED FROM LANDSCAPE DATA
	double aspectf;

	void AspectConvert();
	void CoverConvert();
	void ElevConvert();
	void SlopeConvert();
	void FuelConvert();
};
*/

class FireEnvironment: public LandscapeData
{
	double hours;

	int chrono(double SIMTIME, int *hour);
	void sfmc(double rain, double humid, double temp);
	void hffmc(double humid, double temp);
	double emc(double humid, double temp);
	void daylength(int date, double *sunrise, double *sunset);
	void humtemp(int date, int hour, double *tempref, double *humref,
			   long *elevref, double *rain, double *humidmx,
			   double *humidmn, double *tempmx, double *tempmn);
	void sitespec( long elevref,  long elev, double tempref, double *temp,
				double humref, double *humid);
	double irradiance( long cloud,  int date,  int hour, double *shade);
	void adjust(double *humid, double *temp, double I);
	void windadj(int date, int hour, long *cloud);
	void windreduct();
     void AtmWindAdjustments(int curdate, int hour, long *cloud);
     void AtmHumTemp(int date, int hour, double *temp, double *humid,
			   double *tempmin, double *humidmin, double *rain, long *cloud);

protected:
	long LW, LH;
	double ohmc, thmc, hhmc, mwindspd, twindspd, wwinddir, FuelTemp;

	void envt(double SIMTIME);
//	void winload();
//	void wtrload();

public:
	double FoliarMC, AirTemp, AirHumid, PtTemperature, PtHumidity;           // fuel level temps and humidities at each point
	//long northi, easti;
     double XLocation, YLocation;
	long StationNumber, posit;

	FireEnvironment();
	~FireEnvironment();
};


typedef struct
{
	double ones;
	double tens;
	double hundreds;
     double thousands;
     double tenthous;
	double livew;
	double liveh;
	double windspd;
	double winddir;
	double tws;
     double temp;
     double humid;
     double solrad;
} EnvironmentData;


class FECalls: public FireEnvironment
{
public:
	EnvironmentData gmw, lmw, *FireMW;   	// initialize global and local structures

	FECalls();
	~FECalls();

	long GetLandscapeData(double xpt, double ypt, LandscapeStruct& ls);
	void GetFireEnvironment(double SIMTIME, int type, long ReadNewData, long PointNum);
//	void LoadWtrData();
//	void LoadWndData();
	void globaldata(long ReadNewData, long NumPoints);
	void localdata();
	long AllocFireMW(long NumPoints);
	void FreeFireMW();
	void NoMoistureData(int type);
	double PIgnite();
	void ResetData();
};


//------------------------------------------------------------------------------
//
// NEW MOISTURE MODEL AND
//
//------------------------------------------------------------------------------

#define FM_SLOPPY 	3
#define FM_LIBERAL 	2
#define FM_MODERATE 1
#define FM_HARDASS 	0

#define MAXNUM_FUEL_MODELS 256
#define NUM_FUEL_SIZES 4
#define MAX_NUM_STATIONS 5

#define SIZECLASS_1HR			0
#define SIZECLASS_10HR			1
#define SIZECLASS_100HR			2
#define SIZECLASS_1000HR			3

long GetFmTolerance();
void SetFmTolerance(long Tol);

//------------------------------------------------------------------------------
//           Moisture History Stuff
//------------------------------------------------------------------------------

// DeadMoisture History is storage for old moisture calculations
typedef struct
{
     double 	LastTime;
     double	SimTime;
     long		Elevation;			// feet
	double 	AirTemperature;		// fahrenheit
     double 	RelHumidity;             // percentage
     long		CloudCover;			// percentage
     double 	Rainfall;                // cm of rain
	double 	*Moisture;	        	// array of 10hr moisture contents at SimTime
	void 	*next;			   	// pointer to next DeadMoistureHistory Struct
} DeadMoistureHistory;


// DeadMoistureDescription is summary of old moisture calculations
struct DeadMoistureDescription
{
     unsigned long		NumAlloc[MAX_NUM_STATIONS][NUM_FUEL_SIZES];				// Number of moistures in each DeadMoistureHistory
     long 	NumStations;
     long  	NumElevs[MAX_NUM_STATIONS][NUM_FUEL_SIZES];
     long		NumSlopes[MAX_NUM_STATIONS][NUM_FUEL_SIZES];
     long		NumAspects[MAX_NUM_STATIONS][NUM_FUEL_SIZES];
     long		NumCovers[MAX_NUM_STATIONS][NUM_FUEL_SIZES];
     long  	NumFuels[MAX_NUM_STATIONS][NUM_FUEL_SIZES];			// for each fuel type
     long  	FuelKey[MAX_NUM_STATIONS][NUM_FUEL_SIZES][256];
     long		NumHist[MAX_NUM_STATIONS][NUM_FUEL_SIZES];
     double	EndTime[MAX_NUM_STATIONS][NUM_FUEL_SIZES];			
     Fms		**fms;

     DeadMoistureDescription();
     void ResetDescription(long Station, long FuelSize);
     ~DeadMoistureDescription();
};


//------------------------------------------------------------------------------
//           NEW Nelson Calculations for 10hr fuels
//------------------------------------------------------------------------------


struct FMS_Cover
{
	Fms 		**fms;            // Array of Nelsons 10hr Fms Sticks
     long 	FuelSize;
     long 	LoVal;              // Low value of radiation (Millivolts)
     long 	HiVal;			// hi value of radiation	 (Millivolts)
     double	SolRad;           	// solar radiation, W/m2
     double 	Aspectf;			// radians
     long 	NumFms;			// number of sticks (radiation categories)
     long 	Aspect, Elev, Fuel, Slope;	// Elevation and Fuel type for site
	double 	*LastMx;			// Array of last moistures (fractions)
     double 	*NextMx;			// Array of current moistures
     double	*LastEq;			// Array of equilibrium moisture contents
     double	*NextEq;			// Array of equilibrium moisture contents

     FMS_Cover(long FuelSize);
     bool AllocCover();
     void FreeCover();
};


struct FMS_Aspects
{
	long Slope, Elev, Fuel;
	long NumAspects, FuelSize;
     long LoVal, HiVal;    		// degrees
	FMS_Cover **Fms_Aspect;

     FMS_Aspects(long FuelSize);
     bool AllocAspects();
     void FreeAspects();
};


struct FMS_Slopes
{
     long Elev, Fuel;
	long NumSlopes, FuelSize;
	long LoVal, HiVal;
	FMS_Aspects **Fms_Slope;

     FMS_Slopes(long FuelSize);
     bool AllocSlopes();
     void FreeSlopes();
};


struct FMS_Elevations
{
     long 			NumElevs;    		// Number of elevational categories
     long 			Fuel, FuelSize; 	// fuel type for this site, and size class for this elev
     double 			FirstTime;		// starting time for moisture
     double 			LastTime;			// ending time for moisture
     FMS_Slopes 		**Fms_Elev;	// Array of elevations each containing an array of radiations

     FMS_Elevations(long FuelSize);
     void SetFuel(long Fuel);
     bool AllocElevations();
     void FreeElevations();
};



//------------------------------------------------------------------------------
//           Complete Fuel Moisture Map for all fuel type
//------------------------------------------------------------------------------

struct FuelMoistureMap
{
     long 				NumFuels[4];
     long 				FuelKey[4][256];
     long 				FmTolerance[4][3];
     double				CuumRain;           // centimeters
	FMS_Elevations 		**FMS[4];

     FuelMoistureMap();
     ~FuelMoistureMap();
     void SearchCondenseFuels(long FuelSize);
     bool AllocFuels(long FuelSize);
     void FreeFuels(long FuelSize);
     bool ReAllocFuels(long FuelSize);
};

// global function to convert SimTime to hour and date
long      Chrono(double SIMTIME, long *hour, double *hours, bool RelCondit);

class FmsThread
{
	bool 	FirstTime;
     unsigned 	ThreadID;
	long 	StationNumber, FuelType, FuelSize;
     long 	Begin, End, Date, Hour;
	double 	SimTime, *temp;
     FuelMoistureMap	*Stations;
     DeadMoistureHistory **CurHist;
     DeadMoistureDescription *MxDesc;

     static unsigned  __stdcall RunFmsThread(void *fmsthread);
     void  	UpdateMapMoisture();

public:
     long ThreadOrder, LastTime;
     HANDLE hFmsEvent;

	FmsThread();
     ~FmsThread();

     void 	StartFmsThread(long ID, long SizeClass, DeadMoistureDescription *mxdesc, bool FirstTime);
     void 	SetRange(double SimTime, long date, long hour, long StationNumber,
     					long FuelType,	FuelMoistureMap *map, DeadMoistureHistory **hist,
                              long Begin, long End);
     void 	SiteSpecific(long ElevDiff, double tempref, double *temp,
     					double humref, double *humid);
     double 	SimpleRadiation(long date, double hour, long cloud, long elev,
     					long slope, long aspect, long cover);
     void 	UpdateMoistures();
};


class FireEnvironment2
{
     long		HistoryCount, NumFmsThreads;
     long 	LastDate, LastCount, CloudCount;
     long 	NumStations;
     double 	SimStart;
     FuelMoistureMap 		*Stations;
     DeadMoistureDescription	MxDesc;
     DeadMoistureHistory		*FirstHist[5][4];    // four types of fuels, 1hr 10hr, 100hr, 1000hr X 5 streams,
     DeadMoistureHistory		*CurHist[5][4];
     DeadMoistureHistory		*NextHist[5][4];

	bool 	AllocData(double lo, double hi, long interval, long elev, long fuel, long StationNumber);
     void 	HumTemp(long date, long hour, double *tempref, double *humref, long *elevref, double *rain,
     				   double *humidmx, double *humidmn, double *tempmx, double *tempmn);
     void 	RunFmsThreads(double SimTime, long StationNumber, long FuelType, long FuelSize);
     bool		CheckMoistureHistory(long Station, long FuelSize, double SimTime);
     void		CopyMoistureHistory(long Station, long FuelSize);
     bool 	AllocHistory(long Station, long FuelSize);
     void 	FreeHistory(long Station, long FuelSize);
     void		RefreshHistoryDescription(long Station, long FuelSize);
     void 	CopyFmsData(long Station, long FuelSize, bool ToHistory);
     long 	GetClouds(long date, double hour);
     bool		AllocFmsThreads();
     void 	CloseFmsThreads();
     void 	FreeFmsThreads();

protected:

public:
     long		StationNumber;
	long		elevref;
     double 	tempref, humref, rain;
     double 	humidmx, humidmn, tempmx, tempmn;
     FmsThread	*fmsthread;

	FireEnvironment2();
     ~FireEnvironment2();
     void 	ResetData(long FuelSize);
     void 	ResetAllThreads();
     void 	TerminateHistory();            // storage for moisture history
     bool 	AllocStations(long Num);
     void 	FreeStations();
     double 	GetMx(double Time, long fuel, long elev, long slope,
     					double aspectf, long cover, double *equil, double *solrad, long FuelSize);
     bool 	CalcMapFuelMoistures(double SimTime);
     bool 	ExportMoistureData(char *FileName, char *Desc);
     bool		ImportMoistureData(char *FileName, char *Desc);
     bool		CheckMoistureTimes(double SimTime);
     void 	NormalizeMoistureStartTime();
};


class FELocalSite: public LandscapeData
{
	long 	LW, LH;
     double 	onehour, tenhour, hundhour, thousands, tenthous;
	double 	twindspd, mwindspd, wwinddir, airtemp, relhumid, solrad;
     double 	XLocation, YLocation;
     celldata		cell;
     crowndata 	crown;
     grounddata 	ground;

     void 	windreduct();
     void 	windadj(long date, double hours, long *cloud);
     void 	AtmWindAdjustments(long date, double hours, long *cloud);

public:
     long		StationNumber;
	double 	AirTemp, AirHumid, PtTemperature, PtHumidity;           // fuel level temps and humidities at each point

			FELocalSite();
     void 	GetFireEnvironment(FireEnvironment2 *env, double SimTime, bool All);
	long 	GetLandscapeData(double xpt, double ypt, LandscapeStruct &ls);
	long 	GetLandscapeData(double xpt, double ypt);
	void 	GetEnvironmentData(EnvironmentData *ed2);
#ifdef FLMP_ENABLE_OMFFR
	void OverrideFuels(double one, double ten, double hund);
#endif
};


//------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------


class MechCalls: public Mechanix
{
public:
	double A, SubTimeStep;		// acceleration constant
	EnvironmentData gmw, lmw;   	// initialize global and local structures
	LandscapeStruct ld;

	MechCalls();
	~MechCalls();
	void GetPoints(long Row,  long Col);
     void CalculateFireOrientation(double DegreesFromUphill);
     bool NormalizeDirectionWithLocalSlope();  // modify point orient'n with slope
	void VecSurf();                           // computes vectored surface spread rate
	void VecCrown();					  // computes vectored crown spread rate
	void SpreadCorrect();     			  // corrects crown spread rate from directional spread
	void GetAccelConst();              	  // retrieves acceleration constants
	void AccelSurf1();					  // performs 1st accel for surface fire
	void AccelSurf2();                        // performs 2nd accel for surface fire
	void AccelCrown1();					  // performs 1st accel for crown fire
	void AccelCrown2();                       // performs 2nd accel for crown fire
	void SlopeCorrect(int spreadkind);        // corrects spread for slope
	void LoadGlobalFEData(FELocalSite *fe);
	void LoadLocalFEData(FELocalSite *fe);
};


typedef struct
{
	double x;
	double y;
	double PartDiam;
	double ZHeight;
	double StartElev;
	double CurrentTime;
	double ElapsedTime;
	void *next;				// pointer to next ember in sequence
} emberdata;


class Embers : public APolygon        // define Embers object, with ember and spot data
{

	emberdata TempEmber;
	emberdata *FirstEmber;
	emberdata *CurEmber;
	emberdata *NextEmber;
	emberdata *CarryEmber;
	emberdata *NextCarryEmber;

	double xdiffl, ydiffl, xdiffn, ydiffn, FrontDist;	    // sstep is the spot time step, in minutes
	void fuelcoefs(int fuel, double *coef, double *expon);
	double torchheight(double partdiam, double zo);
	double pileheight(double partdiam);
	double VertWindSpeed(double HeightAboveGround, double TreeHt);
	double partcalc(double vowf, double z);
	MechCalls mech;
	FireEnvironment2 *env;
     FELocalSite *fe;

	struct fcoord
	{	double x, y, xl, yl, xn, yn, e;
		long cover;
	};

public:
	typedef struct
	{
		double x;
		double y;
		double TimeRem;
		void *next;
	} spotdata;

	spotdata *FirstSpot;
	spotdata *CurSpot;
	spotdata *NextSpot;
	spotdata *CarrySpot;

	int SpotSource;
	double SteadyHeight, Duration, SourceRadius, SourcePower; 	// flame parameters for lofting
     double MaxSpot;
	long NumEmbers, CarryOverEmbers, NumSpots, SpotFires, CarrySpots;
	fcoord Fcoord;

	Embers();
	~Embers();
	void Loft(double CFlameLength, double CFBurned, double CrownHeight, double LoadingBurned,
			double ROS, double SubTimeStep, double curtime);
	void Plume(double CFlameLength, double CFBurned);
	void Flight(double CurTime, double EndofTimeStep);
	void Overlap();
	void EmberReset();
     void SpotReset(long numspots, spotdata* ThisSpot);
     void SetFireEnvironmentCalls(FireEnvironment2 *Env, FELocalSite *Fe);
     double GetMaxSpotDist(double xpt, double ypt);

     emberdata ExtractEmber(long type);    // type=0, regular, type==1, carry
     void AddEmber(emberdata *ember);
};


#ifdef FLMP_ENABLE_OMFFR
typedef struct
{
    double ros;
    double fli;
    double flame_length;
    double h_ros;
    double v_dir;
    double head;
    double flank;
    double back;
    int crown_state;
    double max_spot;
}OutputFBStruct;
#endif

class BurnThread: public MechCalls
{
	long 	CurrentFire, CurrentPoint, CuumPoint, TotalPoints, CrownState;
     long 	turn, Begin, End, ColStart, ColEnd, RowStart, RowEnd;
     bool 	FireIsUnderAttack;
     double 	TimeIncrement, CuumTimeIncrement, SIMTIME, SimTimeOffset;
     unsigned 	ThreadID;
     HANDLE 	hBurnThread;
     Crown 	cf;
     FireRing *firering;

	void BurnTheLandscape();
	void SurfaceFire();
#ifdef FLMP_ENABLE_OMFFR
    void SurfaceFire(LandscapeStruct lcp_data);
#endif
	void CrownFire();
	void SpotFire(int SpotSource);
	void EmberCoords();
     static unsigned __stdcall RunBurnThread(void *burnthread);
     void PerimeterThread();
     static unsigned __stdcall RunSpotThread(void *burnthread);
     static unsigned __stdcall BurnThread::RunLandscapeThread(void *bt);
     void SpotThread();
     void SetConstantValues();
     void WriteOutputLayers(long row, long col);
     double GetLastValue(long layer);

public:
     long 		ThreadOrder;
	bool 		CanStillBurn, StillBurning, Started, DoSpots;
	double 		TimeMaxRem;               // maximum time remaining
     double 		EventTimeStep;
     HANDLE		hBurnEvent;

	Embers 		embers;
	AreaPerimeter 	prod;
     FireEnvironment2 		*env;
     FELocalSite			*fe;

	BurnThread(FireEnvironment2 *env);
	~BurnThread();
     HANDLE StartBurnThread(long ID);
     HANDLE StartLandscapeThread(long ID);
     HANDLE GetThreadHandle();
     void SetLandscapeRange(long colstart, long colend, long rowstart, long rowend);
     void SetPerimeterRange(long CurrentFire, double SimTime, double CuumTimeIncrement, double TimeIncrement,
     	double TimeMaxRem, long Begin, long End, long turn, bool FireIsUnderAttack, FireRing *ring);
#ifdef FLMP_ENABLE_OMFFR
    void CopyLandscapeStruct(LandscapeStruct src, LandscapeStruct *dst);
    void BurnTheLandscape(LandscapeStruct, double one, double ten, double hund, OutputFBStruct&);
#endif
};



class Burn: public Intersections
{
	long 	turn;
	long 	CurrentFire, CurrentPoint, NumInFires, CuumPoint, TotalPoints;
	double 	TimeIncrement, EventTimeStep, TimeMaxRem;
     long 	NumPerimThreads;
     bool 	FireIsUnderAttack;
     long 	SpotCount, CurThread, ThreadCount;

     FireRing 		*firering;
     BurnThread 	**burnthread;

	typedef struct
	{
		long FireNum;
		double TimeInc;
		void *next;
	} newinfire;
	newinfire *FirstInFire;	     // pointer to first inward fire
	newinfire *CurInFire;         // current inward fire
	newinfire *NextInFire;
	newinfire *TempInFire;		// temporary fire for allocation
	newinfire *TempInNext;

	void AllocNewInFire(long NewNum, double TimeInc);
	void GetNewInFire(long InFireNum);
	void FreeNewInFires();

	void PreBurn();
     void BurnMethod1();
	void BurnMethod2();
     bool AllocPerimThreads();
     void CloseAllPerimThreads();
     void FreePerimThreads();
     long StartPerimeterThreads_Equal();
     long StartPerimeterThreads_ActiveOnly();
     void ResumePerimeterThreads(long threadct);
     void ResumeSpotThreads(long threadct);
     void Factor(long number, long *x, long *y);

public:
     bool			CanStillBurn;
	bool			StillBurning;
	long 		DistMethod, rastmake;
	double 		SIMTIME, CuumTimeIncrement;
     long			*NumSpots;
     long			TotalSpots, SpotFires;
	//Embers		embers;
	//Rasterize 	rast;
	AreaPerimeter 	prod;
     FireEnvironment2 		*env;
     FELocalSite	 		*fe;

#ifdef FLMP_ENABLE_OMFFR
    BurnThread *pointBurnThread;
#endif
	Burn();
	~Burn();
     void BurnIt(long CurrentFire);
	long StartLandscapeThreads();
     void EliminateFire(long FireNum);
	void DetermineSimLevelTimeStep();
     void ResetAllPerimThreads();
     void BurnSpotThreads();
     void SetSpotLocation(long Loc);
     Embers::spotdata *GetSpotData(double CurrentTimeStep);
     void WriteLayersToDisk();
};

#endif


