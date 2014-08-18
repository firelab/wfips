
#ifndef MainHeaderFile
#define MainHeaderFile

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <dos.h>
//#include <conio.h>
#include <string.h>

#include "fsxlandt.h"
//#include <windows.h>
//#include "fsxpfront.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
typedef struct
{
	double Point;                           // accel const for points
	double Line;				           		// accel const for lines
	double Limit;			           			// upper perimeter limit for points
} AccelConstants;


struct Acceleration {
	AccelConstants ac[261];
	Acceleration();
};

class Crown {
   double CritRos, CritCFB, Ro;

public:
	double A, Io, SFC, CrownFractionBurned, CrownLoadingBurned;
	double HLCB, FoliarMC, CrownBulkDensity, TreeHeight;
	double FlameLength;
	FlamMap *pFlamMap;
	Crown(FlamMap *_pFlamMap);
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
	FlamMap *pFlamMap;
	double startx, starty;

	APolygon(FlamMap *_pFlamMap);
	~APolygon() {};
	int Overlap(int count);
	double direction(double xpt1, double ypt1);
};


/*class StandardizePolygon        // for reordering perimeter from extreme point
{
public:
	FlamMap *pFlamMap;
	StandardizePolygon(FlamMap *_pFlamMap);
	~StandardizePolygon() {}
	int FindExternalPoint(int CurrentFire, int type);
	void ReorderPerimeter(int CurrentFire, int NewBeginning);
	void FindOuterFirePerimeter(int CurrentFire);
	bool Cross(double xpt1, double ypt1, double xpt2, double ypt2,
			 double xpt1n, double ypt1n, double xpt2n, double ypt2n,
			 double *newx, double *newy, int *dup1, int *dup2);
	void DensityControl(int CurrentFire);
     void RemoveDuplicatePoints(int CurrentFire);
     void RemoveIdenticalPoints(int FireNum);
     void BoundingBox(int CurrentFire);
};
*/

/*class XUtilities : public APolygon
{
	int swapnumalloc;
	size_t nmemb;

public:
	FlamMap *pFlamMap;
	int ExNumPts, OldNumPoints;      // USED BY RASTER AND BURN CLASSES, WITH D-METH2
	double *swapperim;                // pter to swap array for perimeter points

	XUtilities(FlamMap *_pFlamMap);
	~XUtilities();
	void AllocSwap(int NumPoint);
	void FreeSwap();
	void GetSwap(int NumPoint, double *xpt, double *ypt, double *ros, double *fli, double *rct);
	void SetSwap(int NumPoint, double xpt, double ypt, double ros, double fli, double rct);
	void SwapTranz(int writefire, int nump);
	void tranz(int count, int nump);
	void rediscretize(int *count, bool Reorder);
     void RePositionFire(int *firenum);
     void RestoreDeadPoints(int firenum);
};

*/
/*class CompareRect
{
	double Xlo, Xhi, Ylo, Yhi;
	double xlo, xhi, ylo, yhi;

public:
	double XLO, XHI, YLO, YHI;

	CompareRect() {};
	~CompareRect() {};
     void InitRect(int FireNum);
	void WriteHiLo(int FireNum);
	void ExchangeRect(int FireNum);
	bool XOverlap();
	bool YOverlap();
	bool BoundCross(int Fire1, int Fire2);
	void DetermineHiLo(double xpt, double ypt);
	bool MergeInAndOutOK1(int Fire1, int Fire2);
};
*/

/*class CrossThread
{
     int CurrentFire, NextFire;
     int ThreadOrder;
     int SpanAStart, SpanAEnd, SpanBStart, SpanBEnd, Terminate;
     int NumCross, NumAlloc;
     int *intersect;
     double *interpoint;
     unsigned ThreadID;
     HANDLE hXEvent;
     APolygon poly;
	    FlamMap *pFlamMap;
     bool Cross(double x, double y, double xn, double yn, double cx, double cy,
               double cxn, double cyn, double *nx, double *ny, int *dup1, int *dup2);
     bool ReallocCross(int Num);
     bool AllocCross(int Num);
     void SetInterPoint(int Number, double XCoord, double YCoord, double Ros, double Fli, double Rct);
     void SetIntersection(int Number, int XOrder, int YOrder);
     void FreeCross();
     void CrossCompare();
     static unsigned __stdcall RunCrossThread(void *crossthread);

public:
     CrossThread(FlamMap *_pFlamMap);
     ~CrossThread();

     void SetRange(int SpanAStart, int SpanAEnd);
     void StartCrossThread(int threadorder, int currentfire, int nextfire);
     int GetNumCross();
     int* GetIsect();
     double* GetIpoint();
};


class Intersections: public XUtilities, public CompareRect, public StandardizePolygon
{
	int SpanAStart, SpanAEnd, SpanBStart, SpanBEnd;
	int NoCrosses, numcross, noffset1, noffset2, readnum, writenum;
	int *intersect;                          // stores array addresses for intersecting perimeter points
	int *crossout;
	int *NewIsect;
	int *AltIsect;
	double *interpoint;                       // stores actual intersection coordinates, ROS, & FLI
	int crossnumalloc, intersectnumalloc, interpointnumalloc;
     int newisectnumalloc;				  // for newclip
     int NumCrossThreads;
     CrossThread **crossthread;
     size_t nmemb;

     bool AllocCrossThreads();
     void FreeCrossThreads();
	bool MergeInAndOutOK2(int Fire1, int Fire2);
     bool SwapBarriersAndFires();
	void AllocIntersection(int Number);
	void FreeIntersection();
	void AllocInterPoint(int Number);
	void FreeInterPoint();
	void GetIntersection(int Number, int *XOrder, int *YOrder);
	void SetIntersection(int Number, int XOrder, int YOrder);
	void GetInterPointCoord(int Number, double *XCoord, double *YCoord);
	void GetInterPointFChx(int Number, double *Ros, double *Fli, double *Rct);
	void SetInterPoint(int Number, double XCoord, double YCoord, double Ros, double Fli, double Rct);
	void AllocCrossout(int Number);
	void FreeCrossout();
	int GetCrossout(int Number);
	void SetCrossout(int Number, int Value);
	int GetSpan(int Number, int ReadFire);
	int intercode(int offcheck,  int xypt);
	void GetOffcheck(int *offcheck);
	void CheckIllogicalExpansions(int CurrentFire);
	//int GetExtremePair(Extremes *First, int count);
	void FindMergeSpans(int FireNum);
	bool CrossCompare(int *CurrentFire, int NextFire);
     bool EliminateCrossPoints(int CurrentFire);
	bool CrossCompare1(int *CurrentFire, int NextFire);
	void BoundaryBox(int NumPoints);
	void FindFirePerimeter(int CurrentFire, int StartPoint);
	bool MergeFire(int *CurrentFire,  int NextFire);
     void MergeBarrier(int *CurrentFire, int NextFire);
     void MergeBarrierNoCross(int *CurrentFire, int NextFire);
	void MergeWrite(int xend,  int readstart,  int readend,  int *xwrite);
     void CheckEnvelopedFires(int Fire1, int Fire2);
	void OrganizeCrosses(int CurrentFire);
     void OrganizeIntersections(int CurrentFire);
     bool TurningNumberOK(int CurrentFire, int StartPoint);
	bool AllocNewIntersection(int NumCross);
	bool FreeNewIntersection();
	void SetNewIntersection(int Alt, int count, int isect1, int isect2);
	void GetNewIntersection(int Alt, int count, int *isect1, int *isect2);

public:
     PostFrontal post;
	FlamMap *pFlamMap;
	Intersections(FlamMap *_pFlamMap);
	~Intersections();
	double arp(int PerimNum, int count);		// area of fire
     void CrossFires(int check,  int *firenum);	// perform loop-clip and mergers
     void CrossesWithBarriers(int FireNum);		// fires X barriers @in sub-timesteps
	void CleanPerimeter(int CurrentFire);
     void ResetIntersectionArrays();
     void CloseCrossThreads();
};

*/

/*class AreaPerimeter: public APolygon
{
public:
	double area, perimeter, sperimeter;
	double cuumslope[2];

	AreaPerimeter(FlamMap *_pFlamMap);
	~AreaPerimeter();
	void arp( int count);
};*/



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
     void   TransformWindDir(int slope, double aspectf);
	double vectordir(double aspectf);
	double vectorspd(double *VWindSpeed, double aspectf, int FireType);

public:
	FlamMap *pFlamMap;
	double xpt, ypt, midx, midy, xt, yt, xptn, yptn, xptl, yptl, xdiff, ydiff;
	double vecspeed, ivecspeed, vecdir, m_windspd, m_winddir, m_twindspd;
	double ActualSurfaceSpread, HorizSpread, avgros, cros, fros, vecros, RosT1, RosT, R10;
	double fli, FliFinal, ExpansionRate;
	double timerem, react, savx, cosslope, HSpreadRate;
	double CurrentTime, FlameLength, CrownLoadingBurned, CrownFractionBurned;
	double head, back, flank;

	Mechanix(FlamMap *_pFlamMap);
	~Mechanix();
	void limgrow(void);							// limits growth to distance checking
	void grow(double ivecdir);					// Richards (1990) differential equation
	//void distchek(int CurrentFire);				// checks perimeter and updates distance check
	void ellipse(double iros, double wspeed);		// calculates elliptical dimensions
	void scorrect( int slope, double aspectf);		// slope correction
	double spreadrate( int slope, double windspd, int fuel);	// Rothermel spread equation
     void GetEquationTerms(double *phiw, double *phis, double *b, double *part1);
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

int GetFmTolerance();
void SetFmTolerance(int Tol);



class FELocalSite: public LandscapeData
{
	  int 	LW, LH;
   double 	onehour, tenhour, hundhour, thousands, tenthous;
	  double 	twindspd, mwindspd, wwinddir, airtemp, relhumid, solrad;
   double 	XLocation, YLocation;
   celldata		cell;
   crowndata 	crown;
   grounddata 	ground;

    void 	windreduct();

public:
    int		StationNumber;
	   double 	AirTemp, AirHumid, PtTemperature, PtHumidity;           // fuel level temps and humidities at each point
		 	FELocalSite(FlamMap *_pFlamMap);
    void 	GetFireEnvironment ( double SimTime, bool All);
   	int 	GetLandscapeData(double xpt, double ypt, LandscapeStruct &ls);
	   void 	GetEnvironmentData(EnvironmentData *ed2);
};


class MechCalls: public Mechanix
{
public:
	//FlamMap *pFlamMap;
	double A, SubTimeStep;		// acceleration constant
	EnvironmentData gmw, lmw;   	// initialize global and local structures
	LandscapeStruct ld;

	MechCalls(FlamMap *_pFlamMap);
	~MechCalls();
	void GetPoints(int Row,  int Col);
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
// 	FireEnvironment2 *env;
     FELocalSite *fe;

	struct fcoord	{
	  double x, y, xl, yl, xn, yn, e;
	 	int cover;
	};

public:
	typedef struct	{
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
 	int NumEmbers, CarryOverEmbers, NumSpots, SpotFires, CarrySpots;
 	fcoord Fcoord;

 	Embers(FlamMap *_pFlamMap);
 	~Embers();
 	void Loft(double CFlameLength, double CFBurned, double CrownHeight, double LoadingBurned,
 	double ROS, double SubTimeStep, double curtime);
 	void Plume(double CFlameLength, double CFBurned);
 	void Flight(double CurTime, double EndofTimeStep);
 	void Overlap();
	 void EmberReset();
     void SpotReset(int numspots, spotdata* ThisSpot);
     void SetFireEnvironmentCalls ( FELocalSite *Fe);
     double GetMaxSpotDist(double xpt, double ypt);

     emberdata ExtractEmber(int type);    // type=0, regular, type==1, carry
     void AddEmber(emberdata *ember);
};


class BurnThread: public MechCalls
{
	    //CFile *threadSafeFile[17];
	//int File;
	FILE * threadSafeFile[17]; //ALM
	    int 	CurrentFire, CurrentPoint, CuumPoint, TotalPoints, CrownState;
     int 	turn, Begin, End, ColStart, ColEnd, RowStart, RowEnd;
     bool 	FireIsUnderAttack;
     double 	TimeIncrement, CuumTimeIncrement, SIMTIME, SimTimeOffset;
     unsigned 	ThreadID;
     //HANDLE 	hBurnThread;
     Crown 	cf;
     //FireRing *firering;
    	 void CreateThreadSafeFiles();
     void DestroyThreadSafeFiles();
    	//void WriteValToFile(int layerNum, int64_t pos);//ALM
	 void WriteValToFile(int layerNum, int64_t pos);//ALM
	    void BurnTheLandscape();
    	void SurfaceFire();
    	void CrownFire();
    	void SpotFire(int SpotSource);
    	void EmberCoords();
     static unsigned int RunBurnThread(void *burnthread);
    // void PerimeterThread();
     static unsigned int RunSpotThread(void *burnthread);
     static unsigned int RunLandscapeThread(void *bt);
     void SpotThread();
     void SetConstantValues();
     void WriteOutputLayers(int row, int col);
     double GetLastValue(int layer);

public:
     int 		ThreadOrder;
    	bool 		CanStillBurn, StillBurning, Started, DoSpots;
	    double 		TimeMaxRem;               // maximum time remaining
     double 		EventTimeStep;
     //HANDLE		hBurnEvent;

    	Embers 		embers;
    	//AreaPerimeter 	prod;
//     FireEnvironment2 		*env;
     FELocalSite			*fe;

    	BurnThread ( FlamMap *_pFlamMap);
   	~BurnThread();
     //HANDLE StartBurnThread(int ID);
     //ALM HANDLE StartLandscapeThread(int ID);
	 void StartLandscapeThread(int ID);
     //HANDLE GetThreadHandle();
     void SetLandscapeRange(int colstart, int colend, int rowstart, int rowend);
     //void SetPerimeterRange(int CurrentFire, double SimTime, double CuumTimeIncrement, double TimeIncrement,
     //double TimeMaxRem, int Begin, int End, int turn, bool FireIsUnderAttack, FireRing *ring);
};


/**************************************************************/
// class Burn:  public Intersections
class Burn 
{
	  double 	TimeIncrement, EventTimeStep, TimeMaxRem;
   int 	NumPerimThreads;
   //FireRing 		*firering;
   BurnThread 	**burnthread;
   FlamMap *pFlamMap ;
   bool AllocPerimThreads();
   void CloseAllPerimThreads();
   void FreePerimThreads();
   void Factor(int number, int *x, int *y);

public:
    bool			CanStillBurn;
	   bool			StillBurning;
    int			*NumSpots;
    FELocalSite	 		*fe;
   	Burn(FlamMap *_pFlamMap);
  	~Burn();     
	   int StartLandscapeThreads();
    void ResetAllPerimThreads();
};

#endif


