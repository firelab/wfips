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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* SQLite/Spatialite */
#include <sqlite3.h>

/* IRS */
/*
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
*/

#ifndef MAX_PATH
#define MAX_PATH 8192
#endif

#define WFIPS_SCRAP_BUFFER_SIZE 10

#define ASSOC_DB            "assoc.db"
#define COST_DB             "cost.db"
#define DISPLOC_DB          "disploc.db"
#define DISTRICT_DB         "district.db"
#define FOREST_DB           "forest.db"
#define FPU_DB              "fpu.db"
#define FWA_DB              "fwa.db"
#define GACC_DB             "gacc.db"
#define RESC_DB             "resc.db"
#define COUNTY_DB           "us_county.db"
#define STATE_DB            "us_state.db"

#ifdef WIN32
#define SPATIALITE_EXT "spatialite.dll"
#else
#define SPATIALITE_EXT "libspatialite.so"
#endif

static const char *apszDbFiles[] = {COST_DB,
                                    DISPLOC_DB,
                                    DISTRICT_DB,
                                    FOREST_DB,
                                    FPU_DB,
                                    FWA_DB,
                                    GACC_DB,
                                    RESC_DB,
                                    COUNTY_DB,
                                    STATE_DB,
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

    int SetRescDb( const char *pszPath );

    int Open();
    int Open( const char *pszPath );
    int Close();

    int ExecuteSql( const char *pszSql );
    int GetAssociatedDispLoc( const char *pszWkt,
                              int **panDispLocIds,
                              int *nCount );

    int Valid() { return bValid; }
    int SpatialiteEnabled() { return bSpatialiteEnabled; }

    int WriteRescDb( const char *pszPath,
                     int *panIds,
                     int *panDispLocIds,
                     int nCount );

    static void Free( void *p );

private:
    void Init();

    const char* FormFileName( const char *pszPath, const char *pszDb );
    const char* BaseName( const char *pszPath );
    int Attach( const char *pszPath );

    char* pszPath;
    char* pszRescPath;
    int bValid;
    int bSpatialiteEnabled;
    sqlite3 *db;

    /* scratch strings */
    char* GetScrapBuffer();
    char szScrap[WFIPS_SCRAP_BUFFER_SIZE][MAX_PATH];
    int iScrap;

    /* Diane's structs */
    //CRunScenario *poScen;
};

#endif /* WFIPS_DATA_H_ */

