#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "flammap3.h"
//#include <iostream.h>
class FlamMap;

#define ATMTEMP  0
#define ATMHUMID 1
#define ATMRAIN  2
#define ATMWSPD  3
#define ATMWDIR  4
#define ATMCLOUD 5

struct Atmosphere
{
private:
	double North;
     double South;
     double East;
     double West;
     double ResolutionX;
     double ResolutionY;
     int   CellX;
     int   CellY;
     short*  Value;

public:
     int   XNumber;
     int   YNumber;
     int   NumCellsPerGrid;
	FlamMap *pFlamMap;
     Atmosphere();
     ~Atmosphere();
     void SetHeaderInfo(double north, double south, double east, double west,
     			    double resolutionx, double resolutiony);
     bool GetHeaderInfo(double *north, double *south, double *east, double *west,
     				double *resolutionx, double *resolutiony);
     void GetResolution(double *resolutionx, double *resolutiony);
     bool CompareHeader(double north, double south, double east, double west,
     			    double resolutionx, double resolutiony);
	bool AllocAtmGrid(int timeintervals);
     bool FreeAtmGrid();
     bool GetAtmValue(double xpt, double ypt, int time, short *value);
     bool SetAtmValue(int number, int time, short value);
};


class AtmosphereGrid
{// base class for access and loading of atmospheric variables
	FILE *InputTable;
     FILE *ThisFile;
     int month, day, hour;
     int* Month;		// array of pointers to ints that store dates
     int* Day;
     int* Hour;
     int NumGrids;
     int StartGrid;
	int TimeIntervals;
     int Metric;
     Atmosphere atmosphere[6];

public:
     bool AtmGridWTR;
     bool AtmGridWND;
	char ErrMsg[256];
	FlamMap *pFlamMap;
	AtmosphereGrid(int numgrids, FlamMap *_pFlamMap);      // will default to 6 if all themes included,
	~AtmosphereGrid();                  // and 3 if only wind spd dir & cloud %
     bool ReadInputTable(char * InputFileName);
     bool ReadHeaders(int FileNumber);
     bool CompareHeader(int FileNumber);
     bool SetAtmosphereValues(int timeinterval, int filenumber);
     bool GetAtmosphereValue(int FileNumber, double xpt, double ypt, int time, short *value);
     void GetResolution(int FileNumber, double *resolutionx, double *resolutiony);
     int GetAtmMonth(int count);
     int GetAtmDay(int count);
     int GetAtmHour(int count);
     int GetTimeIntervals();
     void FreeAtmData();
};

