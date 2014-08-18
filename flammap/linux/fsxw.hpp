
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
/*
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
bool 	CheckExpansions(int YesNo);              	// check illogical perimeter expansions
bool 	CheckPostFrontal(int YesNo);
int 	DistanceCheckMethod(int Method);

//----------------- Fire Data Access Functions --------------------------------

int 	GetInout(int FireNumber);
int 	GetNumPoints(int FireNumber);
void 	SetInout(int FireNumber, int Inout);
void 	SetNumPoints(int FireNumber, int NumPoints);
void 	GetPerimeter2(int coord, double *xpt, double *ypt, double *ros, double *fli, double *rct);
double 	GetPerimeter2Value(int coord, int value);
void 	SetPerimeter2(int coord, double xpt, double ypt, double ros, double fli, double rct);
double* 	AllocPerimeter2(int NumPoints);
void 	FreePerimeter2();
double* 	AllocPerimeter1(int NumFire, int NumPoints);
void 	FreePerimeter1(int NumFire);
double 	GetPerimeter1Value(int NumFire, int NumPoint, int coord);
void 	SetPerimeter1(int NumFire, int NumPoint, double xpt, double ypt);
void 	SetFireChx(int NumFire, int NumPoint, double ros, double fli);
void		SetReact(int NumFire, int NumPoint, double ReactionIntensity);
double*	GetPerimeter1Address(int NumFire, int NumPoint);
int      SwapFirePerims(int NumFire1, int NumFire2);
int 	GetNumPerimAlloc();
bool 	AllocFirePerims(int num);
bool 	ReAllocFirePerims();
void 	FreeAllFirePerims();


//----------------- Fire Accounting Functions ---------------------------------

bool 	CreateSpotSemaphore();
void 	CloseSpotSemaphore();
void  	GetNumSpots(int *NumSpots, bool inc);
void 	SetNumSpots(int input);
void 	IncNumSpots(int increment);
int 	GetNumFires();
void 	SetNumFires(int input);
void 	IncNumFires(int increment);
int 	GetNewFires();
void 	SetNewFires(int input);
void 	IncNewFires(int increment);
int 	GetSkipFires();
void 	SetSkipFires(int newvalue);
void 	IncSkipFires(int increment);
double 	PercentIgnition(double);
double 	IgnitionDelay(double delay);
bool		EnableCrowning(int);
bool		EnableSpotting(int);
bool		EnableSpotFireGrowth(int);
bool		ConstantBackingSpreadRate(int);
int		GetCrownFireCalculation();
int		SetCrownFireCalculation(int Type);

// -----------------Elevation Functions ---------------------------------------

void 	AllocElev(int CurrentFire);
void 	SetElev(int Num, int elev);
int 	GetElev(int Num);
int* 	GetElevAddress(int Num);
void 	FreeElev();

// -----------------Stop-location Functions ---------------------------------------

int 	SetStopLocation(double xcoord, double ycoord);
bool		GetStopLocation(int StopNum, double *xcoord, double *ycoord);
void 	ResetStopLocation(int StopNum);
bool		EnableStopLocation(int StopNum, int Action);
void 	ResetAllStopLocations();
int		GetNumStopLocations();
*/
#endif    //GlobalFarsiteModelFunctions

