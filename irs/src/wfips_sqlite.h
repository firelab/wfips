/******************************************************************************
 *
 * Project:  Wildland Fire Investment Planning System
 * Purpose:  Generic SQLite operations
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

#ifndef WFIPS_SQLITE_H_
#define WFIPS_SQLITE_H_

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "sqlite3.h"

#ifdef WIN32
#define SPATIALITE_EXT "spatialite.dll"
#else
#define SPATIALITE_EXT "libspatialite.so"
#endif

#define ASSOC_DB            "assoc.db"
#define COST_DB             "cost.db"
/* Moved to fwa db */
//#define DELAY_DB            "delay.db"
#define DISPLOC_DB          "disploc.db"
#define DISPLOG_DB          "kyledisplog.db"
#define DISTRICT_DB         "district.db"
#define FIG_DB              "fakefig.db"
#define FOREST_DB           "forest.db"
#define FPU_DB              "fpu.db"
#define FWA_DB              "kylefwa.db"
#define GACC_DB             "gacc.db"
//#define LF_DB               "kylelargefire.db"
#define LF_DB               "largefire.db"
#define STATIC_DB           "static.db"
#define RESC_DB             "resc.db"
#define COUNTY_DB           "us_county.db"
#define STATE_DB            "us_state.db"
#define WFIPS_DB_COUNT      15

/* String comparison macros */
#ifndef EQUAL
#define EQUAL(a,b) strcmp((a),(b))==0
#endif

#ifndef EQUALN
#define EQUALN(a,b,n) strncmp((a),(b),(n))==0
#endif

/*
** Agencies
*/
#define USFS                  (1 << 1)
#define DOI_BIA               (1 << 2)
#define DOI_BLM               (1 << 3)
#define DOI_FWS               (1 << 4)
#define DOI_NPS               (1 << 5)
#define STATE_LOCAL           (1 << 6)
//#define REGIONAL              (1 << 7)
#define DOI_ALL               (DOI_BIA | DOI_BLM | DOI_FWS | DOI_NPS)
#define FED_ALL               (USFS | DOI_ALL)
#define AGENCY_ALL            (FED_ALL | STATE_LOCAL)
#define AGENCY_OTHER          (AGENCY_ALL &~ FED_ALL)

/* Fire outcomes */
#define NO_RESC_SENT          0x0001
#define TIME_LIMIT_EXCEED     0x0002
#define SIZE_LIMIT_EXCEED     0x0004
#define EXHAUSTED             0x0008
#define CONTAINED             0x0010
#define ALL_ESCAPES           (NO_RESC_SENT | TIME_LIMIT_EXCEED | \
                               SIZE_LIMIT_EXCEED | EXHAUSTED)

#define NO_RESC_SENT_STR      "No Resources Sent"
#define TIME_LIMIT_EXCEED_STR "TimeLimitExceeded"
#define SIZE_LIMIT_EXCEED_STR "SizeLimitExceeded"
#define EXHAUSTED_STR         "Exhausted"
#define CONTAINED_STR         "Contained"
#define MONITOR_STR           "Monitor"

static const char *apszStatusStrings[] = { NO_RESC_SENT_STR,
                                           TIME_LIMIT_EXCEED_STR,
                                           SIZE_LIMIT_EXCEED_STR,
                                           EXHAUSTED_STR,
                                           NULL };

#define WFIPS_CHECK_SQLITE if(rc)goto error

int WfipsAttachDb( sqlite3 *db, const char *pszPath, const char *pszName );

double WfipsRandom();

int WfipsCompileGeometry( sqlite3 *db, const char *pszWkt, void **pGeometry );

class WfipsSqlite
{

public:

    char *pszPath;
    char *pszDataPath;

protected:
    int Attach(){ return 0; }
};

#endif /* WFIPS_SQLITE_H_ */

