/******************************************************************************
 *
 * Project:  Wildland Fire Investment Planning System
 * Purpose:  Access on disk data
 * Author:   Kyle Shannon <kyle at pobox dot com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION ( RMRS )
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

#ifndef WFIPS_DATA_H_
#define WFIPS_DATA_H_
/* Standard library */
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* SQLite/Spatialite */
#include <sqlite3.h>
#include "wfips_sqlite.h"

/* IRS */
#include "RunScenario.h"
#include "DispLoc.h"
#include "Rescbase.h"
#include "Airtanker.h"
#include "Engine.h"
#include "SmallAT.h"
#include "SMJAircraft.h"
#include "Smokejumper.h"
#include "WaterTender.h"
#include "Helicopter.h"
#include "Helitack.h"
#include "Fire.h"

#include "wfips_result.h"

#ifndef MAX_PATH
#define MAX_PATH 8192
#endif

#ifndef WFIPS_DATA_TEST_PATH
 #ifdef WIN32
  #define WFIPS_DATA_TEST_PATH "c:/wfips/data/"
 #else
  #define WFIPS_DATA_TEST_PATH "/home/kyle/src/wfips/build/"
 #endif
 #define WFIPS_TEST_OUTPUT_DB  "KYLE_WFIPS_TEST.DB"
#endif

#define WFIPS_SCRAP_BUFFER_SIZE 10

/* Map day indices to names.  NULL padded for 1-based */
static const char *apszWfipsDayOfWeek[] = { NULL,
                                           "Monday",
                                           "Tuesday",
                                           "Wednesday",
                                           "Thursday",
                                           "Friday",
                                           "Saturday",
                                           "Sunday" };

/* Greg Dillon's WFP values */
#define WFP_PRIORITY_1        (1 << 1)
#define WFP_PRIORITY_2        (1 << 2)
#define WFP_PRIORITY_3        (1 << 3)
#define WFP_PRIORITY_4        (1 << 4)

#define WFP_PRIORITY_COUNT    4

static double WFP_NO_TREAT  [WFP_PRIORITY_COUNT] = {0.,0.,0.,0.};
static double WFP_FULL_TREAT[WFP_PRIORITY_COUNT] = {1.,1.,1.,1.};

/* Matt and Crystals Strategic Response */
#define STR_RESP_1            (1 << 1)
#define STR_RESP_2A           (1 << 2)
#define STR_RESP_2B           (1 << 3)
#define STR_RESP_3A           (1 << 4)
#define STR_RESP_3B           (1 << 5)
#define STR_RESP_4            (1 << 6)
#define STR_RESP_5            (1 << 7)
#define STR_RESP_6            (1 << 8)

#define STR_RESP_COUNT        8

/*
** Identifiers for resources in sql. NULL padded to align with shifts above.
*/
static const char *aszAgencyNames[] = { NULL,
                                        "'FS','USFS'",
                                        "'BIA'",
                                        "'BLM'",
                                        "'FWS'",
                                        "'NPS'",
                                        "'STATE/LOCAL'" };
typedef struct WfipsResc WfipsResc;
struct WfipsResc
{
    int nId;
    char *pszDispLoc;
    char *pszName;
    char *pszType;
};

/* Database connections are limited to 10 on the normal (default) SQLite */
static const char *apszDbFiles[] = {ASSOC_DB,
                                    COST_DB,
                                    DISPLOC_DB,
                                    DISPLOG_DB,
                                    //DISTRICT_DB,
                                    FIG_DB,
                                    //FOREST_DB,
                                    //FPU_DB,
                                    FWA_DB,
                                    //GACC_DB,
                                    LF_DB,
                                    STATIC_DB,
                                    RESC_DB,
                                    //COUNTY_DB,
                                    //STATE_DB,
                                    NULL};

/*
** Class for handling reading supporting data and input data for WFIPS.
** Heavily reliant (required) on sqlite3.  Use sqlite3_malloc/sqlite3_free and
** kin for mem management here.
*/
class WfipsData
{

public:
    static WfipsData * Create( const char *pszPath = NULL );
    WfipsData();
    WfipsData( const char *pszPath );
    ~WfipsData();


    int Open();
    int Open( const char *pszPath );
    int Close();

    int ExecuteSql( const char *pszSql );
    int GetAssociatedDispLoc( const char *pszWkt,
                              int **panDispLocIds,
                              int *pnCount );


    int GetAssociatedResources( int *panDispLocIds, int nDispLocCount,
                                WfipsResc **ppsResc, int *pnRescLocCount,
                                int nAgencyFlags );

    int Valid() { return bValid; }
    int SpatialiteEnabled() { return bSpatialiteEnabled; }

    int SetRescDb( const char *pszPath );
    int WriteRescDb( const char *pszPath,
                     int *panIds,
                     int *panDispLocIds,
                     int nCount );

    static void FreeAssociatedResources( WfipsResc *psResc, int nCount );
    static void Free( void *p );

    int GetScenarioIndices( int **ppanIndices );

    /* Move to private ??? */
    int LoadScenario( int nYearIdx, const char *pszTreatWkt,
                      double dfTreatProb, int nWfpTreatMask,
                      double *padfWfpTreatProb, double dfStratProb,
                      int nAgencyFilter );

    int SetAnalysisAreaMask( const char *pszWkt );
    int SetResultPath( const char *pszPath );
    int WriteResults();
    WfipsResult * GetResults() { return poResult; }
    /* XXX TO BE IMPLEMENTED XXX */
    int RunScenario( int iYearIndex );
    /* Not implemented */
    int SetPrepositioning( double, double, double ){return 0;}
    /* int SetDrawdown(){return 0;} */
    int LoadIrsData( const char *pszAnalysisAreaWkt );
    int LoadIrsData();
    int SimulateLargeFire( int nJulStart, int nJulEnd, double dfNoRescProb,
                           double dfTimeLimitProb, double dfSizeLimitProb,
                           double dfExhaustProb );
    int Reset();

    /* XXX TO BE IMPLEMENTED XXX */

    /* Test private fx */
    int TestBuildAgencySet1();
    int TestBuildAgencySet2();
    int TestBuildAgencySet3();
    int TestBuildAgencySet4();
    int TestBuildAgencySet5();
    int TestFidSet1();
    int TestRescTypeLoad1();
    int TestProdLoad1();
    int TestDispLogLoad1();
    int TestDispLogLoad2();
    int TestFwaLoad1();
    int TestFwaLoad2();
    int TestDispLocLoad1();
    int TestDispLocLoad2();
    int TestResourceLoad1();
    int TestResourceLoad2();
    int TestScenLoad1();
    int TestScenLoad2();
    int TestScenLoad3();
    int TestScenLoad4();
    int TestScenLoad5();
    int TestScenLoad6();
    int TestScenLoad7();
    int TestScenLoad8();
    int TestScenLoad9();
    int TestScenLoad10();
    //int TestScenLoad11();
    //int TestScenLoad12();
    //int TestScenLoad13();
    //int TestScenLoad14();

private:
    void Init();

    /* File path and db helpers */
    const char* FormFileName( const char *pszPath, const char *pszDb );
    const char* BaseName( const char *pszPath );
    int Attach( const char *pszPath );

    char* pszPath;
    char* pszRescPath;
    int bValid;
    int bSpatialiteEnabled;
    sqlite3 *db;

    /* String helpers */
    const char * BuildAgencySet( int nAgencyFlags );
    char * BuildFidSet( int *panFids, int nCount );

    const char *pszAnalysisAreaWkt;

    /* scratch strings */
    char* GetScrapBuffer();
    char szScrap[WFIPS_SCRAP_BUFFER_SIZE][MAX_PATH];
    int iScrap;

    /* SQLite/Spatialite convenience */
    int CompileGeometry( const char *pszWkt, void **pCompiled );
    double Random();

    /* Results stuff */
    char* pszResultPath;
    WfipsResult *poResult;

    /* Diane's structs */
    CRunScenario *poScenario;
    int LoadRescTypes();
    int LoadProdRates();
    int LoadDispatchLogic();
    int LoadFwas();
    int LoadDispatchLocations();
    int LoadTankerBases();
    int LoadResources();
    int AssociateHelitack( std::multimap<std::string, CResource*>&resc_map );
    int CreateLargeAirTankers();

    /* Helper for Diane's structs */
    std::map<std::string, int> FwaIndexMap;
    std::map<std::string, int> DispLogIndexMap;
};

#endif /* WFIPS_DATA_H_ */

