/******************************************************************************
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  general disclaimer
 * Author:   Kyle Shannon <kyle@pobox.com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT 
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105 
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT 
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES 
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER 
 * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY, 
 * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef IRS_SUITE_H_
#define IRS_SUITE_H_

#include "data_load.h"
#include "irs_const.h"
#include "omffr_conv.h"
#include "irs_progress.h"
#include "irs_result.h"
#include "irs_error.h"

#include "cpl_conv.h"
#include "cpl_multiproc.h"
#include "ogr_api.h"

/* FIXME: Don't rely on #define counts, use NULL */
static const char * papszPrePosKeys [] = { "ATT", "Regional Crew",
                                           "Regional Helicotper", "FS Crew",
                                           "DOI Crew","FS Engine",
                                           "DOI Engine", "FS HELI",
                                           "DOI HELI", NULL };

/* Size not counting NULL */
#define PREPOS_COUNT 9

static const char * papszDrawDownKeys [] = { "FS Crew", "DOI Crew",
                                             "FS Dozer", "DOI Dozer",
                                             "FS Engine", "DOI Engine",
                                             "FS SEAT", "DOI SEAT",
                                             NULL };

/* Size not counting NULL */
#define DRAWDOWN_COUNT 8

typedef struct _IRSMultiRunData
{
    double dfRescRedStart;
    double dfRescRedStop;
    int nRescRedSteps;
    double dfTreatPercStart;
    double dfTreatPercStop;
    int nTreatPercSteps;
    int nAgencyReduction;
    int nRegionReduction;
    int nRescTypeReduction;
    int nReductionMethod;
    int nYearCount;
    double dfLargeFirePerc;
    int nLargeFireMask;
    int nLargeFireMinSize;
} IRSMultiRunData;

IRS_C_START
static int SelectRandomIndexes( int *panIndexes, int nCount, int nToRemove );

typedef void* IRSSuiteH;
/*
IRS_API IRSSuiteH IRSCreate( char *pszDataPath, char **papszOptions );
IRS_API int IRSSetSpatialFilter( IRSSuiteH hSuite, const char *pszWktGeometry );
IRS_API int SetFpuFilter( IRSSuiteH hSuite, const char *pszFpuCode );
IRS_API int SetExternalResourceDb( IRSSuiteH hSuite, const char *pszPath );
IRS_API int IRSRunScenario( IRSSuiteH hSuite, int nYearIndex );
IRS_API int IRSRunScenarios( IRSSuiteH hSuite, int nYearCount );
IRS_API int IRSRunAllScenarios( IRSSuiteH hSuite );
IRS_API int IRSDestroy( IRSSuiteH hSuite );

*/

struct _IRSRunData
{
    CRunScenario *poScen;
    IRSDataAccess *poDA;
    double dfTreat;
    double dfRescCutCount;
    const char *pszLastRunData;
    int nYearIndex;
    int nErr;
    const char *pszOmitSet;
    int nTankerCount;
    IRSProgress pfnProgress;
};
typedef struct _IRSRunData IRSRunData;

struct _IRSThread
{
    void *hThread;
};
typedef struct _IRSThread IRSThread;

int IRSDefaultProgress( double dfProgress, const char *pszMessage,
                        void *pProgressArg );

IRS_C_END

class IRSSuite
{
public:
    IRSSuite();
    IRSSuite( const char *pszDataPath, char **papszOptions );
    static IRSSuiteH Create( const char *pszDataPath, char **papszOptions );
    ~IRSSuite();
    static void Destroy( IRSSuiteH hSuite );

    static void RunScenarioConcurrent( void *pData );
    static void LoadDataConcurrent( void *pData );

    /* Space, filters, and load */
    int SetSpatialFilter( const char *pszWktGeometry );
    int SetFpuFilter( const char *pszFpuCode );
    int LoadData( IRSProgress );
    int UnloadData();

    /* Resource related */
    int SetExternalResourceDb( const char *pszPath );

    int ReduceResources( int nAgencies, int nRescTypes, double dfReduction,
                         int nMethod, const char *pszPreviousPath );

    /* Treatment related */
    int SetTreatmentMask( const char *pszDatasource, const char *pszLayer,
                          const char *pszWhere );
    int SetTreatmentProb( double dfTreatment );

    /* Simulation related */
    unsigned int GetScenarioCount();
    int RunScenario( unsigned int nYearIndex );
    int RunScenarios( unsigned int nYearCount, unsigned int nThreads,
                      IRSProgress );
    int RunAllScenarios( unsigned int nThreads, IRSProgress );

    /* Canonical version */
    int RunScenarios( unsigned int nYearStart, unsigned nYearCount, 
                      unsigned int nThreads, IRSProgress );

    int SetOutputPath( const char *pszOutputPath );
    int SetOgrOutputPath( const char *pszOutputPath );
    int SetPreviousResultPath( const char *pszPath );
    int WriteSingleResourceUsage();

    int ExportOgr( const char *pszFormat, const char *pszFilename );
    int ExportFpuSummary( IRSProgress pfnProgress );

    int PostProcessLargeFire( int bEscapeTypes, double dfSubSample,
                              unsigned int nSampleSize, int nMinSize, 
                              IRSProgress pfnProgress );

    char ** GetResultAttributes();
    int FreeResultAttributes( char ** papszAttributes );

    int SummarizeByFpu( IRSProgress pfnProgress );
    int ExportOgrSummary( const char *pszFormat, const char *pszOutPath,
                          char **papszOptions );
    int ExportFpuSummaryCsv( const char *pszOutPath );

    std::list<double> NationalDistribution( std::string Attribute );
    std::vector<std::pair<std::string, std::list<double> > > GACCDistributions( std::string Attribute );

    int SetQuiet( int bQuiet );

    int SetMultiRun( IRSMultiRunData *psRunData, size_t nRunCount,
                     size_t nThreads, IRSProgress );

    IRSResult * GetResultHandle() { return poResult; }

    int SetPreposition( const char *pszKey, double dfLevel, int bOutOfSeason );
    int SetDrawdown( const char *pszKey, double dfLevel, int bOutOfSeason );

    void SetTankerCount( int n ) { nTankerCount = n; }
    int GetTankerCount() { return nTankerCount; }

private:
    int bQuiet;
    Random r;

    int Initialize();

    /* IRS model */
    CRunScenario *poScenario;

    int nTankerCount;

    /* Preposition and Drawdown */
    PrepositionStruct pasPP[PREPOS_COUNT];
    DrawdownStruct pasDD[DRAWDOWN_COUNT];

    /* Data access */
    IRSDataAccess *poDataAccess;
    const char *pszDataPath;
    const char *pszExternalResourceDb;
    const char *pszOmitResourceDb;
    int bDataLoaded;
    int bDataDirty;

    /* Resource reduction */
    int nReduceAgencies, nReduceTypes, nReduceMethod, nReduceRegions;
    double dfReduceFraction;
    const char *pszReducePrevPath;
    int IReduceResources( int nAgencies, int nRescTypes, double dfReduction,
                          int nMethod, const char *pszPreviousPath );
    const char * IReduceResources2( int nAgencies, int nRescTypes,
                                    double dfReduction, int nMethod,
                                    const char *pszPreviousPath );

    int RemoveResourceByIndexes( int *panIndexes, int nToRemove );
    int RemovePreviousResources( const char *pszPreviousPath );
    int RemovePreviousResourcesPerform( const char *pszPreviousPath,
                                        double dfPerfReduction,
                                        int bRemoveUnused );

    /* Treatment mask */
    const char *pszTreatmentDatasource;
    const char *pszTreatmentLayer;
    const char *pszTreatmentWhere;
    double dfTreatment;

    /* Outputs */
    IRSResult *poResult;
    const char *pszOutputPath;
    const char *pszOgrOutPath;

    /* Initialization */
    int InitializePrepositionDrawdown();
};

#endif /* IRS_SUITE_H_ */

