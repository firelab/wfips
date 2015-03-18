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
    char c;
    if( pszPath )
    {
        this->pszPath = sqlite3_mprintf( "%s", pszPath );
    }
    if( pszDataPath )
    {
        c = pszDataPath[strlen(pszDataPath) -1];
        if( c != '/' || c != '\\' )
            this->pszDataPath = sqlite3_mprintf( "%s/", pszDataPath );
        else
            this->pszDataPath = sqlite3_mprintf( "%s", pszDataPath );
    }
    Open();
}

void WfipsResult::Init()
{
    pszPath = NULL;
    pszDataPath = NULL;
    bValid = 0;
    db = NULL;
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

    rc = sqlite3_exec( db, "SELECT InitSpatialMetadata(1)", NULL, NULL, NULL );

    char *pszTmpPath;
    pszTmpPath = sqlite3_mprintf( "%s%s", pszDataPath, FIG_DB );
    WfipsAttachDb( db, pszTmpPath, "fig" );
    sqlite3_free( pszTmpPath );
    pszTmpPath = sqlite3_mprintf( "%s%s", pszDataPath, LF_DB );
    WfipsAttachDb( db, pszTmpPath, "largefire" );
    sqlite3_free( pszTmpPath );

error:
    if( rc != SQLITE_OK )
    {
        sqlite3_close( db );
        bValid = 0;
    }
    else
    {
        bValid = 1;
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

int WfipsResult::EnableVolatile( int bVolatile )
{
    int rc;
    if( bVolatile )
    {
        rc = sqlite3_exec( db, "PRAGMA synchronous=off", NULL, NULL, NULL );
        rc = sqlite3_exec( db, "PRAGMA journal_mode=off", NULL, NULL, NULL );
    }
    else
    {
        rc = sqlite3_exec( db, "PRAGMA synchronous=on", NULL, NULL, NULL );
        rc = sqlite3_exec( db, "PRAGMA journal_mode=delete", NULL, NULL, NULL );
    }
    return rc;
}

WfipsResult::~WfipsResult()
{
}

int WfipsResult::Close()
{
    sqlite3_free( pszPath );
    sqlite3_free( pszDataPath );
    int rc = sqlite3_close( db );
    db = NULL;
    pszPath = pszDataPath = NULL;
    return rc;
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
int
WfipsResult::SimulateLargeFire( int nJulStart, int nJulEnd, double dfNoRescProb,
                              double dfTimeLimitProb, double dfSizeLimitProb,
                              double dfExhaustProb )
{
    int rc, i;
    sqlite3_stmt *stmt, *lfstmt;
    double adfProbs[4] = { dfNoRescProb, dfTimeLimitProb, dfSizeLimitProb, dfExhaustProb };
    rc = sqlite3_exec( db, "CREATE TABLE IF NOT EXISTS " \
                           "large_fire_result(year,fire_num,cost_acre,acres,pop)",
                       NULL, NULL, NULL );

    double dfRadius = 0.085;
    int nLimit = 20;
    int nDayDelta = 7;

    rc = sqlite3_prepare_v2( db, "SELECT fig.year,fig.fire_num,fig.jul_day," \
                                 "fig.geometry FROM fire_result LEFT JOIN fig " \
                                 "USING(year,fire_num) WHERE status=?1 "
                                 "AND fig.jul_day BETWEEN ?2 AND ?3",
                             -1, &stmt, NULL );

    rc = sqlite3_bind_int( stmt, 2, nJulStart );
    rc = sqlite3_bind_int( stmt, 3, nJulEnd );

    rc = sqlite3_prepare_v2( db, "SELECT acres, pop, cost " \
                                 "FROM largefire WHERE ST_Intersects(" \
                                 "ST_Buffer(?1, ?2), largefire.geometry) " \
                                 "AND largefire.jday BETWEEN ?3 AND ?4 " \
                                 "AND (ABS(RANDOM())/9223372036854775807.) > ?5 " \
                                 "AND largefire.ROWID IN (SELECT " \
                                 "pkid FROM idx_largefire_geometry WHERE " \
                                 "xmin <= MbrMaxX(ST_Buffer(?1, ?2)) AND " \
                                 "xmax >= MbrMinX(ST_Buffer(?1, ?2)) AND " \
                                 "ymin <= MbrMaxY(ST_Buffer(?1, ?2)) AND " \
                                 "ymax >= MbrMinY(ST_Buffer(?1, ?2))) " \
                                 "ORDER BY RANDOM() LIMIT ?6",
                             -1, &lfstmt, NULL );

    int nYear, nFire, nJulDay;
    int nSize;
    const void *pGeom;
    int nAcres, nCost, nPop;
    for( i = 0; i < 4; i++ )
    {
        rc = sqlite3_bind_text( stmt, 1, apszEscapes[i], -1, NULL );
        while( sqlite3_step( stmt ) == SQLITE_ROW )
        {
            nYear = sqlite3_column_int( stmt, 0 );
            nFire = sqlite3_column_int( stmt, 1 );
            nJulDay = sqlite3_column_int( stmt, 2 );
            nSize = sqlite3_column_bytes( stmt, 3 );
            pGeom = sqlite3_column_blob( stmt, 3 );
            rc = sqlite3_bind_blob( lfstmt, 1, pGeom, nSize, NULL );
            rc = sqlite3_bind_double( lfstmt, 2, dfRadius );
            rc = sqlite3_bind_int( lfstmt, 3, nJulDay - nDayDelta );
            rc = sqlite3_bind_int( lfstmt, 4, nJulDay + nDayDelta );
            rc = sqlite3_bind_double( lfstmt, 5, adfProbs[i] );
            rc = sqlite3_bind_int( lfstmt, 6, nLimit );
            while( sqlite3_step( lfstmt ) == SQLITE_ROW )
            {
            }
            rc = sqlite3_reset( lfstmt );
        }
        rc = sqlite3_reset( stmt );
    }
    sqlite3_finalize( stmt );
    sqlite3_finalize( lfstmt );
    return 0;
}

int
WfipsResult::SpatialSummary( const char *pszKey )
{
    pszKey = "fpu";
    sqlite3_stmt *stmt, *sstmt;
    int rc;
    char szPath[8192];
    char szFile[8192];
    szPath[0] = '\0';
    sprintf( szPath, "%s%s.db", pszDataPath, pszKey );
    rc = WfipsAttachDb( db, szPath, pszKey );


    if( rc != SQLITE_OK )
        return rc;

    /*
    ** FIXME NEED TO ADD YEARS!
    */

    char *pszSql = sqlite3_mprintf( "SELECT fpu_code, %s.geometry, status, " \
                                    "COUNT(status) FROM fire_result LEFT JOIN " \
                                    "fig.fig USING(year, fire_num) JOIN %s ON " \
                                    "ST_INTERSECTS(fig.geometry, %s.geometry) " \
                                    "GROUP BY fpu_code, status", pszKey, pszKey,
                                    pszKey );

    StartTransaction();
    EnableVolatile( 1 );
    rc = sqlite3_prepare_v2( db, pszSql, -1, &stmt, NULL );
    rc = sqlite3_exec( db, "CREATE TABLE spatial_result(name,noresc,tlimit," \
                           "slimit,exhaust,contain,monitor,contratio)", NULL,
                       NULL, NULL );
    rc = sqlite3_exec( db, "SELECT AddGeometryColumn('spatial_result','geometry', " \
                           "4269,'MULTIPOLYGON','XY')", NULL, NULL, NULL );

    rc = sqlite3_prepare_v2( db, "INSERT INTO spatial_result "
                                 "VALUES(?,?,?,?,?,?,?,?,?)",
                            -1, &sstmt, NULL );
    char *pszName = NULL;
    int nSize;
    void *pGeom = NULL;
    const char *pszStatus;
    int nCount;
    int nNoResc, nTimeLimit, nSizeLimit, nExhaust, nContain, nMonitor;
    double dfRatio;
    double dfSum;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        if( pszName == NULL || !EQUAL( pszName, (const char*)sqlite3_column_text( stmt, 0 ) ) )
        {
            if( pszName != NULL )
            {
                rc = sqlite3_bind_text( sstmt, 1, pszName, -1, NULL );
                rc = sqlite3_bind_int( sstmt, 2, nNoResc );
                rc = sqlite3_bind_int( sstmt, 3, nTimeLimit );
                rc = sqlite3_bind_int( sstmt, 4, nSizeLimit );
                rc = sqlite3_bind_int( sstmt, 5, nExhaust );
                rc = sqlite3_bind_int( sstmt, 6, nContain );
                dfSum = nNoResc + nTimeLimit + nSizeLimit + nExhaust + nContain;
                dfRatio = (double)(nContain + nMonitor) / dfSum;
                rc = sqlite3_bind_double( sstmt, 7, dfRatio );
                rc = sqlite3_bind_blob( sstmt, 8, pGeom, nSize, NULL );
                rc = sqlite3_step( sstmt );
                rc = sqlite3_reset( sstmt );
            }
            sqlite3_free( pszName );
            pszName = sqlite3_mprintf("%s", (const char*)sqlite3_column_text( stmt, 0 ) );
            nSize = sqlite3_column_bytes( stmt, 1 );
            sqlite3_free( pGeom );
            pGeom = sqlite3_malloc( nSize );
            memcpy( pGeom, sqlite3_column_blob( stmt, 1 ), nSize );
            nNoResc = nTimeLimit = nSizeLimit = nNoResc = 0;
            nExhaust = nMonitor = nContain = 0;
        }
        pszStatus = (const char *)sqlite3_column_text( stmt, 2 );
        nCount = sqlite3_column_int( stmt, 3 );
        if( EQUAL( pszStatus, "Contained" ) )
            nContain += nCount;
        else if( EQUAL( pszStatus, "Monitor" ) )
            nMonitor += nCount;
        else if( EQUAL( pszStatus, "No Resources Sent" ) )
            nNoResc += nCount;
        else if( EQUAL( pszStatus, "TimeLimitExceeded" ) )
            nTimeLimit += nCount;
        else if( EQUAL( pszStatus, "SizeLimitExceeded" ) )
            nSizeLimit += nCount;
    }
    rc = sqlite3_bind_text( sstmt, 1, pszName, -1, NULL );
    rc = sqlite3_bind_int( sstmt, 2, nNoResc );
    rc = sqlite3_bind_int( sstmt, 3, nTimeLimit );
    rc = sqlite3_bind_int( sstmt, 4, nSizeLimit );
    rc = sqlite3_bind_int( sstmt, 5, nExhaust );
    rc = sqlite3_bind_int( sstmt, 6, nContain );
    rc = sqlite3_bind_int( sstmt, 7, nMonitor );
    dfSum = nNoResc + nTimeLimit + nSizeLimit + nExhaust + nContain;
    dfRatio = (double)nContain / dfSum;
    rc = sqlite3_bind_double( sstmt, 7, dfRatio );
    rc = sqlite3_bind_blob( sstmt, 8, pGeom, nSize, NULL );
    rc = sqlite3_step( sstmt );

    rc = sqlite3_reset( stmt );
    rc = sqlite3_reset( sstmt );
    rc = sqlite3_finalize( stmt );
    rc = sqlite3_finalize( sstmt );
    Commit();
    EnableVolatile( 0 );
    sqlite3_free( pszName );
    sqlite3_free( pGeom );
    sqlite3_free( pszSql );
    return 0;
}

WfipsResult::WfipsResult() {}
WfipsResult::WfipsResult( const WfipsResult &rhs ){}

