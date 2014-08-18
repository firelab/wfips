
//------------------------------------------------------------------------------
// 	FSXW.H 		Header File contains functions for accessing global data
// 				associated with the fire growth model
//
//
//
//                  Copyright 1994, 1995
//   			Mark A. Finney, Systems for Environmental Management
//------------------------------------------------------------------------------


#ifndef GlobalFarsiteModelFunctions
#define GlobalFarsiteModelFunctions

#include <stdlib.h>
//#include <owl\window.h>

#define GETVAL -1		// definitions used for getting and storing data
#define XCOORD 0		// in fire perimeter1 arrays:
#define YCOORD 1
#define ROSVAL 2
#define FLIVAL 3
#define RCXVAL 4

#define NUMDATA 5

#define MAXNUM_STOPLOCATIONS 10

#define DISTCHECK_SIMLEVEL  	0
#define DISTCHECK_FIRELEVEL  1

//typedef unsigned BYTE;

//----------------- Model Parameter Access Functions --------------------------

double 	GetDistRes();
void 	SetDistRes(double input);
void 	SetDynamicDistRes(double input);
double 	GetDynamicDistRes();
double 	GetPerimRes();
void   	SetPerimRes(double input);
bool 	AccelerationON();
void 	SetAccelerationON(bool State);
double 	GetRosRed(int fuel);
void 	SetRosRed(int fuel, double rosred);
void 	InitializeRosRed();
double 	GetActualTimeStep();
double 	GetVisibleTimeStep();
double	GetTemporaryTimeStep();
void 	SetActualTimeStep(double input);
void 	SetVisibleTimeStep(double input);
void		SetTemporaryTimeStep(double input);
void 	SetSecondaryVisibleTimeStep(double input);
double 	GetSecondaryVisibleTimeStep();
double 	EventMinimumTimeStep(double time);       	// event driven time step
bool 	CheckExpansions(long YesNo);              	// check illogical perimeter expansions
bool 	CheckPostFrontal(long YesNo);
long 	DistanceCheckMethod(long Method);

//----------------- Fire Data Access Functions --------------------------------

long 	GetInout(long FireNumber);
long 	GetNumPoints(long FireNumber);
void 	SetInout(long FireNumber, int Inout);
void 	SetNumPoints(long FireNumber, long NumPoints);
void 	GetPerimeter2(long coord, double *xpt, double *ypt, double *ros, double *fli, double *rct);
double 	GetPerimeter2Value(long coord, long value);
void 	SetPerimeter2(long coord, double xpt, double ypt, double ros, double fli, double rct);
double* 	AllocPerimeter2(long NumPoints);
void 	FreePerimeter2();
double* 	AllocPerimeter1(long NumFire, long NumPoints);
void 	FreePerimeter1(long NumFire);
double 	GetPerimeter1Value(long NumFire, long NumPoint, int coord);
void 	SetPerimeter1(long NumFire, long NumPoint, double xpt, double ypt);
void 	SetFireChx(long NumFire, long NumPoint, double ros, double fli);
void		SetReact(long NumFire, long NumPoint, double ReactionIntensity);
double*	GetPerimeter1Address(long NumFire, long NumPoint);
long      SwapFirePerims(long NumFire1, long NumFire2);
long 	GetNumPerimAlloc();
bool 	AllocFirePerims(long num);
bool 	ReAllocFirePerims();
void 	FreeAllFirePerims();


//----------------- Fire Accounting Functions ---------------------------------

bool 	CreateSpotSemaphore();
void 	CloseSpotSemaphore();
void  	GetNumSpots(long *NumSpots, bool inc);
void 	SetNumSpots(long input);
void 	IncNumSpots(long increment);
long 	GetNumFires();
void 	SetNumFires(long input);
void 	IncNumFires(long increment);
long 	GetNewFires();
void 	SetNewFires(long input);
void 	IncNewFires(long increment);
long 	GetSkipFires();
void 	SetSkipFires(long newvalue);
void 	IncSkipFires(long increment);
double 	PercentIgnition(double);
double 	IgnitionDelay(double delay);
bool		EnableCrowning(long);
bool		EnableSpotting(long);
bool		EnableSpotFireGrowth(long);
bool		ConstantBackingSpreadRate(long);
long		GetCrownFireCalculation();
long		SetCrownFireCalculation(long Type);

// -----------------Elevation Functions ---------------------------------------

void 	AllocElev(long CurrentFire);
void 	SetElev(long Num, long elev);
long 	GetElev(long Num);
long* 	GetElevAddress(long Num);
void 	FreeElev();

// -----------------Stop-location Functions ---------------------------------------

long 	SetStopLocation(double xcoord, double ycoord);
bool		GetStopLocation(long StopNum, double *xcoord, double *ycoord);
void 	ResetStopLocation(long StopNum);
bool		EnableStopLocation(long StopNum, long Action);
void 	ResetAllStopLocations();
long		GetNumStopLocations();

#endif    //GlobalFarsiteModelFunctions

