//------------------------------------------------------------------------------
//
//      FSXPFRONT.H		post frontal combustion module
//
//------------------------------------------------------------------------------
#ifndef POSTFRONTAL
#define POSTFRONTAL

#define CO2 	0
#define CO	1
#define PM10	2
#define PM20	3
#define HEAT	4

#define RINGS_PER_STRUCT 100
#define MAX_FUEL_FRACTIONS 5

#define FLAMING 0
#define TRANSITION 1
#define SMOLDERING 2

#define LO_PRECISION 6
#define MED_PRECISION 12
#define HI_PRECISION 18

//#include "fsglbvar.h"
#include <windows.h>
#include "burnupcw.h"
//#include <process.h>


#define PF_FUELWEIGHT  0
#define PF_TOTALHEAT   1
#define PF_PM25	   2
#define PF_PM10	   3
#define PF_CH4		   4
#define PF_CO		   5
#define PF_CO2		   6

#define PF_FLAMING     0
#define PF_SMOLDERING  1
#define PF_TOTAL	   2


typedef struct
{
     double Time;
	double Flaming;
     double Smoldering;
} PFrontStruct;


class PFrontData
{
	long numarrays;
     PFrontStruct *pf1, *pf2;
     double pm25f, pm25s, pm10s, pm10f, ch4s, ch4f, coF, coS, co2f, co2s;
     double MaxSmolder, MaxFlaming, MaxTotal;
     double FractFlameAtMax;

     double GetFlameMult(long Species);
     double GetSmolderMult(long Species);

public:
	long number;
     double MaxTime;

     PFrontData();
     ~PFrontData();

     void SetData(double Time, double Flame, double Smolder);
	double GetPostFrontalProducts(long Species, long Phase, long Num);
     double GetTime(long Num);
     void ResetData();
     double GetMax(long Species, long Phase);
};


typedef struct
{
     short Status;
     long FireID;
	long VertexID;
     double x, y;
} MergePoints;


typedef struct
{
     short DuffLoad;		  	// duff fuel load kg/m2*100
     unsigned char WoodyFuelType;	// index to woody fuel mod
     unsigned char SurfaceFuelModel;	// index to surface fuel type
     unsigned char Moistures[20];	// array of moisture contents *100;
     unsigned char FlamePolyNum;	// number of regr. coefs
     unsigned char WeightPolyNum;	// number of regr. coefs
     float CrownLoadingBurned;	// crownloading consumed in crown fire Mg/ha
	float FlameCoefX[18];		// coefficients for polynomial interplation
	float FlameCoefY[18];		// coefficients for polynomial interplation
     float WeightCoefX[18];		// coefficients for polynomial interplation
     float WeightCoefY[18];		// coefficients for polynomial interplation
     float FlameTime;			// (min)
     float TotalTime;			// (min)
     double LastIntegTime;		// (min)
     float TotalWeight;			// kg/m2
     float ReactionIntensity[2];	// (kW/m2, reaction Intensity, Rothermel Equation)
     float FlameResidenceTime[2];	// (sec, Rothermel Equation)
     float LastWtRemoved;		// total running fuel weight remaining at current time
     float CurWtRemoved;           // Current total wt lost
     float FlameWtRemoved;         // Current wt lost in flaming
} BurnHistory;


typedef struct
{
     short Status;                  //
	double x1, x2, y1, y2;         // coords of quadrangle
     double SizeMult;               // restriction on area of quad. if it overlaps
     double Area;				 // area of influence of this trajectory 1-2
     double l1, l2, h;			 // dimensions of trapeziod
     BurnHistory hist;              // BurnHistory struct
} PerimPoints;


typedef struct
{
     long OriginalFireNumber;
	long NumFires;
     long *NumPoints;
	double StartTime;
     double ElapsedTime;
     long NumMergePoints[2];
     MergePoints *mergepoints;
	PerimPoints *perimpoints;
} FireRing;


typedef struct
{
     long StructNum;
     long NumFireRings;
	FireRing firering[RINGS_PER_STRUCT];
     void *next;
} RingStruct;


class RingBurn
{
     long 	ThreadOrder, FireNum, Begin, End;
	double 	x1, y1, x2, y2, rt, wf;
     double 	xn1, yn1, xn2, yn2, rt2, wf2; // next points for spatl. integration
     double 	xm1, ym1, xm2, ym2;			// midpoints of quadrangles used for spatl integration
     double 	xi, yi, xmi, ymi;			// time-fraction weighted distance for spatl integration
     double 	xip, yip, xmip, ymip;		// time-fraction weighted distance for spatl integration
     double 	a2, b2, c2, d2, p2, q2;       // dimensions of quadrangle
     unsigned  ThreadID;
     HANDLE 	hThread;
     HANDLE 	hRingEvent;
     FireRing 	*ring;
     BurnUp 	*burnup;
	RingStruct *NextOldRing;
	RingStruct *CurOldRing;

     static unsigned __stdcall RunBurnThread(void *ringburn);
	void 	GetWholePolygon(FireRing *ring, long LastNext, bool Reverse);
     void 	GetSubPolygon(double end, bool Reverse);
     void 	CalculateArea(FireRing *ring, long pt, long ptl, long ptn);
     void 	BurnThread();
     bool 	CombustionHistoryExists(long CurIndex);
     FireRing *GetOldRing(long RingNum);

public:
	bool 	ThreadStarted;

     RingBurn();
     ~RingBurn();
     void 	SetRange(FireRing *ring, long FireNum, long begin, long end);
     HANDLE 	StartBurnThread(long ID);
};


class BurnupFireRings
{
	RingBurn *ringburn;
	long NumRingBurn;

	bool 	AllocRingBurn();
	void 	FreeRingBurn();
	void 	CloseBurnupFireRings();

public:
	BurnupFireRings();
	~BurnupFireRings();
     void 	BurnFireRings(long StartRingNum, long EndRingNum);
     void 	ResetAllThreads();
};




//------------------------------------------------------------------------------
//
//	Global access functions for post frontal
//
//------------------------------------------------------------------------------

FireRing* 	AllocFireRing(long NumPoints, double start, double end);
void 		FreeAllFireRings();
void 		FreeFireRing(long RingNum);
FireRing* 	GetRing(long RingNum);
FireRing* 	GetSpecificRing(long FireNumber, double StartTime);
void 		GetLastRingStruct();
long 		GetNumRings();
void 		SetNumRings(long NewNumRings);
long 		GetNumRingStructs();
void 		CondenseRings(long RingNum);
void 		SetNewFireNumber(long OldNum, long NewNum, long RefNum);
bool			AddToCurrentFireRing(FireRing *firering, long PointNum, long SurfFuelType,
				long WoodyModel, double DuffLoad, double *Moistures, double CrownLoadingBurned);

//------------------------------------------------------------------------------
//
//	PostFrontal class declaration
//
//------------------------------------------------------------------------------

class PFIntegration
{
     long 	ThreadOrder, Begin, End, FireNum;
     unsigned  ThreadID;
     double 	StartTime, ElapsedTime;
     double 	CurrentTime;

     HANDLE 	hThread;
     HANDLE    hIntegEvent;
     FireRing 	*ring;

     double 	CalcWeightLoss(long j, double Fract1, double Fract2,
     							double Time1, double Time2);
     void 	FirstIntegration(FireRing *ring, long j, double TimeInc, long NumCalcs,
								double *TotWeight, double *FlameWt);
	void 	SecondaryIntegration(FireRing *ring, long j, double CurTempTime, double TempTimeInc,
     							long NumSpatCalcs, double *Weight, double *Flame);
     void 	CalculateSpatialIntegrationSteps(FireRing *, long PointNum, double CurrentTime,
				     			long *NumCalcs);
     double 	CalculateTemporalIntegrationSteps(FireRing *, long PointNum, double *CurrentTime,
     							long *NumCalcs, double *BaseFract);
     double 	Interpolate(FireRing *ring, long j, double CurrentTime, long type);

     static unsigned __stdcall RunIntegThread(void *pfintegration);
     void 	IntegThread();

public:
     bool 	ThreadStarted;
	double 	FlameWeightConsumed, SmolderWeightConsumed;

	PFIntegration();
     ~PFIntegration();
     bool 	Integrate();//FireRing *, double CurrentTime, double *FlameWt, double *SmolderWt);
     void 	SetRange(FireRing *ring, long firenum, double CurrentTime, long begin, long end);
     HANDLE 	StartIntegThread(long ID);
};


class PostFrontal
{
     long ReferenceFireRingNum, MergeReferenceRingNum;
	double x1, y1, x2, y2, rt, wf;
     double 	xn1, yn1, xn2, yn2, rt2, wf2; // next points for spatl. integration
     double 	xm1, ym1, xm2, ym2;		// midpoints of quadrangles used for spatl integration
     //double 	xi, yi, xmi, ymi;			// time-fraction weighted distance for spatl integration
     //double 	xip, yip, xmip, ymip;		// time-fraction weighted distance for spatl integration
     //double 	a2, b2, c2, d2, p2, q2;
     //double 	StartTime, ElapsedTime;
     double 	StartX, StartY;
     double 	*Verts;
     double 	SimLevel_FlameWeightConsumed, SimLevel_SmolderWeightConsumed;

     PFIntegration *pfi;	// array of pointers to integration objects for multithreading
     long 	NumPFI;

	void 	InterpolateOverlaps(PerimPoints *perimpoints, long numverts);
     double 	GetPerimVal(long PerimLocation, long NumFire, long PointNum, long TYPE);
     void 	GetRingPoint(FireRing *ring, long PointNum);
	bool 	Cross(double xpt1, double ypt1, double xpt2, double ypt2, bool OnSpan1,
		         double xpt1n, double ypt1n, double xpt2n, double ypt2n, bool OnSpan2,
                   double *newx, double *newy);
     void 	AreaPerim(long NumPoints, double *verts, double *area, double *perim);
     double 	Direction(double xpt, double ypt);
     long 	Overlap(long NumPoints, double *verts);
     void		FillOuterRing(FireRing *firering);
	void 	UpdatePointOrder(FireRing *firering, long firenum);
	void 	UpdateFireRing(FireRing *firering, long firenum, long NewPoints);
	void 	UpdateMergeOrder(FireRing *firering, long firenum);
	void 	FillMergeArray(FireRing *firering);
     void 	PartitionMergeRing(FireRing *firering);
	FireRing *SpawnFireRing(long NumNewFire, long NumAlloc, double Start, double Elapsed);
     void 	RemoveRingEnclaves(FireRing *ring);
     void		RemoveDuplicatePoints(FireRing *ring);

     bool 	AllocPFI();
     void 	CloseAllThreads();
     void      FreePFI();

public:
     BurnupFireRings bup;

	PostFrontal();
	~PostFrontal();

     double 	ComputeSmoke(double CurrentTime, long Species);
     double 	ComputeHeat(double CurrentTime);
     long 	AccessReferenceRingNum(long LoopOrMerge, long RingNum);
     void 	ComputePostFrontal(double CurrentTime, double *smolder, double *flaming);
	//long 	SetFireRing(long NumFire, double Start, double End);
     FireRing* SetupFireRing(long NumFire, double Start, double End);
	//bool	AddToCurrentFireRing(FireRing *firering, long PointNum, long SurfFuelType);
	void 	CorrectFireRing(long NumIsects, long *isects, double *ipoints,
     					long *fires, long NumPerims, long PriorNumPoints);
	void 	MergeFireRings(long *fires, long NumPerims, long *isects,
                               double *ipoints, long NumIsects, long CurNumPts);
	long 	BurnupPrecision(long LoHi);
     void      UpdateAttackPoints(FireRing *firering, long firenum);
     void 	ResetAllThreads();
};


#endif


