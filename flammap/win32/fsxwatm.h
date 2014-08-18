#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
//#include <iostream.h>

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
     long   CellX;
     long   CellY;
     short*  Value;

public:
     long   XNumber;
     long   YNumber;
     long   NumCellsPerGrid;

     Atmosphere();
     ~Atmosphere();
     void SetHeaderInfo(double north, double south, double east, double west,
     			    double resolutionx, double resolutiony);
     bool GetHeaderInfo(double *north, double *south, double *east, double *west,
     				double *resolutionx, double *resolutiony);
     void GetResolution(double *resolutionx, double *resolutiony);
     bool CompareHeader(double north, double south, double east, double west,
     			    double resolutionx, double resolutiony);
	bool AllocAtmGrid(long timeintervals);
     bool FreeAtmGrid();
     bool GetAtmValue(double xpt, double ypt, long time, short *value);
     bool SetAtmValue(long number, long time, short value);
};


class AtmosphereGrid
{// base class for access and loading of atmospheric variables
	FILE *InputTable;
     FILE *ThisFile;
     long month, day, hour;
     long* Month;		// array of pointers to longs that store dates
     long* Day;
     long* Hour;
     long NumGrids;
     long StartGrid;
	long TimeIntervals;
     long Metric;
     Atmosphere atmosphere[6];

public:
     bool AtmGridWTR;
     bool AtmGridWND;
	char ErrMsg[256];

	AtmosphereGrid(long numgrids);      // will default to 6 if all themes included,
	~AtmosphereGrid();                  // and 3 if only wind spd dir & cloud %
     bool ReadInputTable(char * InputFileName);
     bool ReadHeaders(long FileNumber);
     bool CompareHeader(long FileNumber);
     bool SetAtmosphereValues(long timeinterval, long filenumber);
     bool GetAtmosphereValue(long FileNumber, double xpt, double ypt, long time, short *value);
     void GetResolution(long FileNumber, double *resolutionx, double *resolutiony);
     long GetAtmMonth(long count);
     long GetAtmDay(long count);
     long GetAtmHour(long count);
     long GetTimeIntervals();
     void FreeAtmData();
};

