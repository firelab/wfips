
#include "omffr_port.h"

#include "flm4.hpp"
#include "flmglbvar.h"

typedef struct
{
    float fm_1;
    float fm_10;
    float fm_100;
    float fm_1000;
    float fm_herb;
    float fm_wood;
} FuelMoistStruct;

class FlamMap
{
     //Burn *burn;
     HANDLE hMoistThread;
     HANDLE hBurnThread;
     bool TerminateMoist, TerminateBurn;
     double MaxTime, MoistSimTime, Resolution;

     long RunFlamMapThreads();
     bool RunFuelMoistureCalculations();
     void ThreadOptions();
     void ResetThreads();

     static unsigned __stdcall RunMoistThread(void *);
     void MoistThread();
     void StartMoistThread();

     static unsigned __stdcall RunBurnThread(void *);
     void StartBurnThread();
     void BurnThread();

public:
     FlamMap();
     ~FlamMap();

     Burn *burn;
     bool LoadLandscapeFile(char *FileName);
     bool LoadFuelMoistureFile(char *FileName);
     bool LoadCustomFuelFile(char *FileName);
     bool LoadWeatherFile(char *FileName);
     bool LoadWindFile(char *FileName);
     bool LoadOutputFile(char *FileName);
     bool SetDates(long StartMonth, long StartDay, long StartHour,
                         long EndMonth, long EndDay, long EndHour);
     void SelectOutput(long LayerID, bool On);
     bool LoadDateFile(char *FileName);
     bool InquireOutputSelections();

     void ResetFlamMap();
     bool LoadLCP();
     void SelectOutputs();
     void RunFlamMap();
#ifdef FLMP_ENABLE_OMFFR
    void RunFlamMapPoint(LandscapeStruct lcp_data, FuelMoistStruct moist_data,
                         double wind_spd, double wind_dir, OutputFBStruct &output);
#endif

    void SetResolution(double Res);
};
