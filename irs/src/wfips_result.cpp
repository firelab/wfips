/******************************************************************************
 *
 * Project:  Wildland Fire Investment Planning System
 * Purpose:  Save results to disk
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

#include "wfips_result.h"

WfipsResult::WfipsResult( const char *pszPath, const char *pszDataPath )
{
    Init();
    if( pszPath )
        this->pszPath = sqlite3_mprintf( "%s", pszPath );
    if( pszDataPath )
        this->pszDataPath = sqlite3_mprintf( "%s", pszDataPath );
    Open();
}

void WfipsResult::Init()
{
    pszPath = NULL;
    pszDataPath = NULL;
    bValid = 0;
}

int WfipsResult::Open()
{
    if( pszPath == NULL )
    {
        bValid = 0;
        return SQLITE_ERROR;
    }
    int rc = sqlite3_open_v2( pszPath, &db,
                              SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                              NULL );
    if( rc != SQLITE_OK )
    {
        rc = sqlite3_close( db );
        db = NULL;
        return SQLITE_ERROR;
    }

    rc = sqlite3_enable_load_extension( db, 1 );
    WFIPS_CHECK_SQLITE;
    rc = sqlite3_load_extension( db, SPATIALITE_EXT, NULL, NULL );
    WFIPS_CHECK_SQLITE;
    rc = sqlite3_exec( db, "CREATE TABLE IF NOT EXISTS "
                           "fire_result(year integer, "
                                       "jul_day integer, "
                                       "fire_num integer, "
                                       "arrtime integer,"
                                       "arrsize real,"
                                       "finalsize real,"
                                       "finaltime real,"
                                       "run_contain integer,"
                                       "treated integer,"
                                       "status text)",
                       NULL, NULL, NULL );
    WFIPS_CHECK_SQLITE;

    rc = sqlite3_prepare_v2( db, "INSERT INTO fire_result(year, jul_day, "
                                 "fire_num, arrtime, arrsize, finalsize, "
                                 "finaltime, run_contain, treated, status) "
                                 "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1,
                             &istmt, NULL );

error:
    if( rc != SQLITE_OK )
    {
        sqlite3_close( db );
        bValid = 0;
    }
    return rc;
}

int WfipsResult::StartTransaction()
{
    return sqlite3_exec( db, "BEGIN", NULL, NULL, NULL );
}
int WfipsResult::Commit()
{
    return sqlite3_exec( db, "COMMIT", NULL, NULL, NULL );
}

WfipsResult::~WfipsResult()
{
    Close();
}

int WfipsResult::Close()
{
    sqlite3_free( pszPath );
    sqlite3_free( pszDataPath );
    return sqlite3_close( db );
}

int WfipsResult::WriteRecord( CResults &oResults )
{
    int rc;
    int nYear, nFireNum, nJulDay, nArrTime;
    std::string oStatus;
    double dfArrSize;
    double dfFinalSize, dfFinalTime;
    int bTreated, bRunContain;
    int nFullYear = oResults.GetFire().GetScenario();
    nYear = oResults.GetFire().GetScenario();
    nJulDay = oResults.GetFire().GetJulianDay();
    nFireNum = oResults.GetFire().GetFireNumber();
    nArrTime = (int)oResults.GetFireCost();
    dfArrSize = oResults.GetFireSweep();
    dfFinalSize = oResults.GetFireSize();
    dfFinalTime = oResults.GetFireTime();
    bTreated = oResults.GetFire().GetTreated();
    bRunContain = oResults.GetFire().GetSimulateContain();
    oStatus = oResults.GetStatus();
    rc = sqlite3_bind_int( istmt, 1, nYear );
    rc = sqlite3_bind_int( istmt, 2, nJulDay );
    rc = sqlite3_bind_int( istmt, 3, nFireNum );
    rc = sqlite3_bind_int( istmt, 4, nArrTime );
    rc = sqlite3_bind_double( istmt, 5, dfArrSize );
    rc = sqlite3_bind_double( istmt, 6, dfFinalSize );
    rc = sqlite3_bind_double( istmt, 7, dfFinalTime );
    rc = sqlite3_bind_int( istmt, 8, bRunContain );
    rc = sqlite3_bind_int( istmt, 9, bTreated );
    rc = sqlite3_bind_text( istmt, 10, oStatus.c_str(), -1, SQLITE_TRANSIENT );
    rc = sqlite3_step( istmt );
    rc = sqlite3_reset( istmt );
    return SQLITE_OK;
}

WfipsResult::WfipsResult() {}
WfipsResult::WfipsResult( const WfipsResult &rhs ){}

