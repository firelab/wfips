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
    istmt = NULL;
    hTreatDS = NULL;
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
    //rc = sqlite3_exec( db, "PRAGMA threads=4", NULL, NULL, NULL );
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

    rc = sqlite3_exec( db, "SELECT InitSpatialMetadata(1)", NULL, NULL, NULL );
    WFIPS_CHECK_SQLITE;
    rc = sqlite3_exec( db, "SELECT AddGeometryColumn('fire_result', 'geometry',"
                           "4269, 'POINT', 'XY')",
                       NULL, NULL, NULL );
    WFIPS_CHECK_SQLITE;
    rc = sqlite3_prepare_v2( db, "INSERT INTO fire_result(year, jul_day, "
                                 "fire_num, arrtime, arrsize, finalsize, "
                                 "finaltime, run_contain, treated, status,geometry) "
                                 "VALUES(?,?,?,?,?,?,?,?,?,?,MakePoint(?,?,4269))",
                             -1, &istmt, NULL );

    WFIPS_CHECK_SQLITE;

    char *pszTmpPath;
    pszTmpPath = sqlite3_mprintf( "%s%s", pszDataPath, FIG_DB );
    WfipsAttachDb( db, pszTmpPath, "fig" );
    sqlite3_free( pszTmpPath );
    pszTmpPath = sqlite3_mprintf( "%s%s", pszDataPath, LF_DB );
    WfipsAttachDb( db, pszTmpPath, "largefire" );
    sqlite3_free( pszTmpPath );
    pszTmpPath = sqlite3_mprintf( "%s%s", pszDataPath, "treat_perc.tif" );
    hTreatDS = GDALOpen( pszTmpPath, GA_ReadOnly );
    sqlite3_free( pszTmpPath );
    if( hTreatDS == NULL )
    {
        rc = SQLITE_ERROR;
    }
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
    sqlite3_finalize( istmt );
    int rc = sqlite3_close( db );
    db = NULL;
    GDALClose( hTreatDS );
    hTreatDS = NULL;
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
    double dfX, dfY;
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
    dfX = oResults.GetFire().GetLongitude();
    dfY = oResults.GetFire().GetLatitude();
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
    rc = sqlite3_bind_double( istmt, 11, dfX );
    rc = sqlite3_bind_double( istmt, 12, dfY );
    rc = sqlite3_step( istmt );
    rc = sqlite3_reset( istmt );
    return SQLITE_OK;
}

int WfipsResult::CreateIndices()
{
    int rc;
    if( !WfipsHasTable( db, "idx_fire_result_year_fire_num" ) )
    {
        rc = sqlite3_exec( db, "CREATE INDEX idx_fire_result_year_fire_num "
                               "ON fire_result(year,fire_num)",
                           NULL, NULL, NULL );
    }
    if( !WfipsHasTable( db, "idx_fire_result_geometry" ) )
    {
        rc = sqlite3_exec( db, "SELECT CreateSpatialIndex('fire_result', 'geometry')",
                           NULL, NULL, NULL );
    }
    if( WfipsHasTable( db, "large_fire_result" ) )
    {
        rc = sqlite3_exec( db, "CREATE INDEX idx_large_fire_result_year_fire_num "
                               "ON large_fire_result(year,fire_num)",
                           NULL, NULL, NULL );
    }
    return rc;
}

typedef struct LargeFireData LargeFireData;
struct LargeFireData
{
    double dfCost;
    double dfSize;
    double dfPop;
    int bExcluded;
};

static int CompareLargeFireSize( const void *a, const void *b )
{
    assert( a && b );
    if( ((LargeFireData*)a)->dfSize > ((LargeFireData*)b)->dfSize )
        return -1;
    else if( ((LargeFireData*)a)->dfSize < ((LargeFireData*)b)->dfSize )
        return 1;
    else
        return 0;
}

static int AvgLargeFireSize( LargeFireData *pasFires, int nSize,
                             double *dfAvgSize, double *dfSumSize )
{
    assert( pasFires && dfAvgSize && dfSumSize );
    *dfAvgSize = 0;
    *dfSumSize = 0;
    int i;
    for( i = 0; i < nSize; i++ )
    {
        if( pasFires[i].bExcluded )
        {
            continue;
        }
        *dfAvgSize += pasFires[i].dfSize;
    }
    *dfSumSize = *dfAvgSize;
    *dfAvgSize /= nSize;
    return SQLITE_OK;
}

static const double LargeFireTreatPerc[] = { 0.0,
                                             0.1,
                                             0.2,
                                             0.3,
                                             0.4,
                                             0.5,
                                             0.6,
                                             0.7,
                                             0.8,
                                             0.9,
                                             1.0, };
static const double LargeFireTreatRed[] =  { 1.00,
                                             0.92,
                                             0.74,
                                             0.52,
                                             0.37,
                                             0.30,
                                             0.25,
                                             0.22,
                                             0.21,
                                             0.20,
                                             0.20 };

static double LargeFireTargetAverage( double dfTreatPerc )
{
    assert( dfTreatPerc >= 0.0 && dfTreatPerc <= 1.0 );
    int nIndex = (int)(dfTreatPerc * 10);
    if( nIndex == 10 )
        return 0.20;
    assert( nIndex < 10 );
    double dX, dY, dfVal;
    dX = dfTreatPerc - LargeFireTreatPerc[nIndex];
    dY = (LargeFireTreatRed[nIndex] - LargeFireTreatRed[nIndex+1]) * dX;
    dfVal = LargeFireTreatRed[nIndex] - dY;
    return dfVal;
}

/*
 * Decrease the size of the large fire sampling due to the affect of
 * treatments.  I am not sure about the post condition here.  Do we *have* to
 * meat the target size?  Or do we just try and if we don't hit it due to
 * randomness, we don't hit it.  The initial size also influences the ability
 * to meet the target correctly.
 *
 * Using the following table:
 * % treated    % average fire size
 *   0.0          1.00
 *   0.1          0.92
 *   0.2          0.74
 *   0.3          0.52
 *   0.4          0.37
 *   0.5          0.30
 *   0.6          0.25
 *   0.7          0.22
 *   0.8          0.21
 *   0.9          0.20
 *   1.0          0.20
 */

static int DecreaseLargeFireSize( LargeFireData *pasFires, int nSize,
                                  double dfPercTreat )
{
    assert( pasFires && nSize > 0 && dfPercTreat >= 0.0 && dfPercTreat <= 1.0 );
    int i = 0;
    int nRemoved = 0;
    int nTries;
    double dfAvgSize, dfSumSize;
    if( AvgLargeFireSize( pasFires, nSize, &dfAvgSize, &dfSumSize ) != 0 )
        return SQLITE_ERROR;
    double dfAvgRed = LargeFireTargetAverage( dfPercTreat );
    double dfTargetAvg = dfAvgSize * dfAvgRed;
    qsort( pasFires, nSize, sizeof( LargeFireData ), CompareLargeFireSize );
    double dfProb;
    double r;

    nTries = 0;
    while( dfAvgSize > dfTargetAvg && nTries < 5 )
    {
        for( i = 0; i < nSize; i++ )
        {
            if( pasFires[i].bExcluded )
                continue;
            /*
            ** FIXME: Not sure about calculation of the prob for removal.  Needs to
            ** be revisited.  Right now it's relative to the max size * 1.1, which
            ** is arbitrary.  It used to be relative to the sum of the sizes, which
            ** was artificially low.
            */
            //dfProb = pasFires[i].dfSize / (pasFires[0].dfSize * 1.1);
            dfProb = pasFires[i].dfSize / dfSumSize;

            r = WfipsRandom();
            if( r * dfPercTreat < dfProb && !pasFires[i].bExcluded )
            {
                pasFires[i].bExcluded = TRUE;
                nRemoved++;
            }
            if( nRemoved == nSize )
            {
                return SQLITE_ERROR;
            }
            if( AvgLargeFireSize( pasFires, nSize, &dfAvgSize, &dfSumSize ) != SQLITE_OK )
                return SQLITE_ERROR;
            if( dfAvgSize < dfTargetAvg )
            {
                break;
            }
        }
        nTries++;
    }
    return SQLITE_OK;
}

int
WfipsResult::SimulateLargeFire( int nJulStart, int nJulEnd, double dfNoRescProb,
                                double dfTimeLimitProb, double dfSizeLimitProb,
                                double dfExhaustProb, const char *pszTreatWkt,
                                double dfTreatProb )
{
    int rc, n, i, j, k;
    sqlite3_stmt *stmt, *lfstmt, *lfistmt, *gstmt;
    double adfProbs[4] = { dfNoRescProb, dfTimeLimitProb, dfSizeLimitProb, dfExhaustProb };
    /*
    ** FIXME: Check inputs
    */
    /* All 0 probabilities is a no-op */
    if( dfNoRescProb == 0. && dfTimeLimitProb == 0. &&
        dfSizeLimitProb == 0. && dfExhaustProb == 0. )
    {
        return SQLITE_OK;
    }
    rc = sqlite3_exec( db, "CREATE TABLE IF NOT EXISTS " \
                           "large_fire_result(year,fire_num,acres,pop,cost)",
                       NULL, NULL, NULL );
    rc = sqlite3_prepare_v2( db, "INSERT INTO large_fire_result VALUES(?,?,?,?,?)",
                             -1, &lfistmt, NULL );

    double dfRadius = 0.1;
    int nLimit = 40;
    int nDayDelta = 7;

    double dfX, dfY;

    LargeFireData *pasFires = (LargeFireData*)sqlite3_malloc( sizeof( LargeFireData ) * nLimit );

    rc = sqlite3_prepare_v2( db, "SELECT fig.year,fig.fire_num,fig.jul_day," \
                                 "fig.geometry, X(fig.geometry), Y(fig.geometry) " \
                                 "FROM fire_result LEFT JOIN fig " \
                                 "USING(year,fire_num) WHERE status=?1 "
                                 "AND fig.jul_day BETWEEN ?2 AND ?3",
                             -1, &stmt, NULL );

    rc = sqlite3_bind_int( stmt, 2, nJulStart );
    rc = sqlite3_bind_int( stmt, 3, nJulEnd );

    rc = sqlite3_prepare_v2( db, "SELECT acres, pop, cost " \
                                 "FROM largefire WHERE ST_Intersects(" \
                                 "ST_Buffer(?1, ?2), largefire.geometry) " \
                                 "AND largefire.jday BETWEEN ?3 AND ?4 " \
                                 "AND largefire.ROWID IN (SELECT " \
                                 "pkid FROM idx_largefire_geometry WHERE " \
                                 "xmin <= MbrMaxX(ST_Buffer(?1, ?2)) AND " \
                                 "xmax >= MbrMinX(ST_Buffer(?1, ?2)) AND " \
                                 "ymin <= MbrMaxY(ST_Buffer(?1, ?2)) AND " \
                                 "ymax >= MbrMinY(ST_Buffer(?1, ?2))) " \
                                 "ORDER BY RANDOM() LIMIT ?5",
                             -1, &lfstmt, NULL );

    rc = sqlite3_prepare_v2( db, "SELECT 1 WHERE "
                                 "ST_Contains(@treat,"
                                 "MakePoint(@figX,@figY,4269)) AND "
                                 "@figX <= MbrMaxX(@treat) AND "
                                 "@figX >= MbrMinX(@treat) AND "
                                 "@figY <= MbrMaxY(@treat) AND "
                                 "@figY >= MbrMinY(@treat)", -1,
                             &gstmt, NULL );

    void *pTreatGeom;
    int nTreatGeomSize = WfipsCompileGeometry( db, pszTreatWkt, &pTreatGeom );
    int bTreated;

    int nYear, nFire, nJulDay;
    int nSize;
    const void *pGeom;
    int nAcres, nCost, nPop;
    double dfTreated;
    int nValidFires;
    int nActualCount;
    StartTransaction();
    for( i = 0; i < 4; i++ )
    {
        rc = sqlite3_bind_text( stmt, 1, apszEscapes[i], -1, NULL );
        rc = sqlite3_bind_int( stmt, 2, nJulStart );
        rc = sqlite3_bind_int( stmt, 3, nJulEnd );
        while( sqlite3_step( stmt ) == SQLITE_ROW )
        {
            if( WfipsRandom() > adfProbs[i] )
            {
                continue;
            }
            nYear = sqlite3_column_int( stmt, 0 );
            nFire = sqlite3_column_int( stmt, 1 );
            nJulDay = sqlite3_column_int( stmt, 2 );
            nSize = sqlite3_column_bytes( stmt, 3 );
            pGeom = sqlite3_column_blob( stmt, 3 );
            dfX = sqlite3_column_double( stmt, 4 );
            dfY = sqlite3_column_double( stmt, 5 );
            rc = PixelValue( hTreatDS, 1, dfX, dfY, &dfTreated, NULL );
            rc = sqlite3_bind_blob( lfstmt, 1, pGeom, nSize, NULL );
            rc = sqlite3_bind_double( lfstmt, 2, dfRadius );
            rc = sqlite3_bind_int( lfstmt, 3, nJulDay - nDayDelta );
            rc = sqlite3_bind_int( lfstmt, 4, nJulDay + nDayDelta );
            rc = sqlite3_bind_int( lfstmt, 5, nLimit );
            /* Populate the large fire structures */
            j = 0;
            while( sqlite3_step( lfstmt ) == SQLITE_ROW )
            {
                pasFires[j].dfSize = sqlite3_column_double( lfstmt, 0 );
                pasFires[j].dfPop = sqlite3_column_double( lfstmt, 1 );
                pasFires[j].dfCost = sqlite3_column_double( lfstmt, 2 );
                pasFires[j].bExcluded = 0;
                j++;
            }
            nActualCount = j;
            rc = sqlite3_reset( lfstmt );
            if( nActualCount < 1 )
            {
                continue;
            }
            /*
            ** If we have a mask or a prob, we need to apply treatments.  A
            ** NULL mask and a prob means 'gloabal' treatments
            */
            /* Valid mask */
            if( (pszTreatWkt && nTreatGeomSize) )
            {
                rc = sqlite3_bind_blob( gstmt,
                                        sqlite3_bind_parameter_index( gstmt, "@treat" ),
                                        pTreatGeom, nTreatGeomSize, NULL );
                rc = sqlite3_bind_double( gstmt,
                                          sqlite3_bind_parameter_index( gstmt, "@figX" ),
                                          dfX );
                rc = sqlite3_bind_double( gstmt,
                                          sqlite3_bind_parameter_index( gstmt, "@figY" ),
                                          dfY );
                rc = sqlite3_step( gstmt );
                if( rc == SQLITE_ROW && sqlite3_column_int( gstmt, 0 ) )
                {
                    bTreated = 1;
                }
                else
                {
                    bTreated = 0;
                }
                sqlite3_reset( gstmt );
                rc = SQLITE_OK;
            }
            /* No mask, but treatment probability */
            else if( dfTreatProb > 0 )
            {
                /* for sqlite3_free() */
                pTreatGeom = NULL;
                bTreated = 1;
                rc = SQLITE_OK;
            }
            /* No treatment */
            else
            {
                /* for sqlite3_free() */
                pTreatGeom = NULL;
                bTreated = 0;
                rc = SQLITE_OK;
            }
            if( bTreated )
            {
                rc = DecreaseLargeFireSize( pasFires, nActualCount,
                                            dfTreated / 100. * dfTreatProb );
            }
            if( rc == SQLITE_OK )
            {
                nValidFires = 0;
                for( j = 0; j < nActualCount; j++ )
                {
                    if( !pasFires[j].bExcluded )
                        nValidFires++;
                }
                n = (int)(WfipsRandom() * nValidFires);
                assert( n < nValidFires && n >= 0 );
                k = j = 0;
                while( k < n )
                {
                    if( !pasFires[j].bExcluded )
                    {
                        k++;
                    }
                    j++;
                }
                rc = sqlite3_bind_int( lfistmt, 1, nYear );
                rc = sqlite3_bind_int( lfistmt, 2, nFire );
                rc = sqlite3_bind_int( lfistmt, 3, pasFires[k].dfSize );
                rc = sqlite3_bind_int( lfistmt, 4, pasFires[k].dfPop );
                rc = sqlite3_bind_double( lfistmt, 5, pasFires[k].dfCost );
                rc = sqlite3_step( lfistmt );
                sqlite3_reset( lfistmt );
            }
        }
        rc = sqlite3_reset( stmt );
    }
    sqlite3_finalize( stmt );
    sqlite3_finalize( lfstmt );
    sqlite3_finalize( lfistmt );
    sqlite3_finalize( gstmt );
    sqlite3_free( pTreatGeom );
    sqlite3_free( pasFires );
    Commit();
    return 0;
}

int
WfipsResult::SpatialSummary( const char *pszKey )
{
    sqlite3_stmt *stmt, *sstmt, *lfstmt;
    int rc;
    char szPath[8192];
    char szFile[8192];
    szPath[0] = '\0';
    sprintf( szPath, "%s%s", pszDataPath, FPU_DB );
    rc = WfipsAttachDb( db, szPath, "fpu" );
    sprintf( szPath, "%s%s", pszDataPath, FWA_DB );
    rc = WfipsAttachDb( db, szPath, "fwa" );

    assert( EQUAL( pszKey, "fwa" ) || EQUAL( pszKey, "fpu" ) );

    if( rc != SQLITE_OK )
        return rc;

    int bWriteLargeFire = WfipsHasTable( db, "large_fire_result" );
    /*
    ** FIXME NEED TO ADD YEARS!
    */
    char *pszSql;
    if( EQUAL( pszKey, "fpu" ) )
    {

        pszSql = sqlite3_mprintf( "%s", "SELECT fpu_code, fpu.geometry, status, " \
                                        "COUNT(status) FROM fire_result, fpu WHERE "
                                        "ST_Intersects(fire_result.geometry,"
                                        "fpu.geometry) AND "
                                        "fire_result.ROWID IN "
                                        "(SELECT pkid FROM idx_fire_result_geometry WHERE "
                                        "xmin <= MbrMaxX(fpu.geometry) AND "
                                        "xmax >= MbrMinX(fpu.geometry) AND "
                                        "ymin <= MbrMaxY(fpu.geometry) AND "
                                        "ymax >= MbrMinY(fpu.geometry)) "
                                        "GROUP BY fpu_code, status" );

        if( bWriteLargeFire )
        {
            rc = sqlite3_prepare_v2( db, "SELECT fpu.fpu_code, SUM(acres), SUM(pop), " \
                                         "SUM(cost) FROM large_fire_result " \
                                         "LEFT JOIN fig.fig USING(year,fire_num) " \
                                         "LEFT JOIN fpu.fpu ON "
                                         "substr(fig.fwa_name,0,10)=fpu.fpu_code " \
                                         "WHERE fpu.fpu_code=?",
                                     -1, &lfstmt, NULL );
        }
    }
    else if( EQUAL( pszKey, "fwa" ) )
    {
        pszSql = sqlite3_mprintf( "%s", "SELECT fwa.name, fwa.geometry, status, " \
                                        "COUNT(status) FROM fire_result, fwa " \
                                        "WHERE ST_Intersects(fire_result.geometry,"
                                        "fwa.geometry) AND fire_result.ROWID IN "
                                        "(SELECT pkid FROM idx_fire_result_geometry WHERE "
                                        "xmin <= MbrMaxX(fwa.geometry) AND "
                                        "xmax >= MbrMinX(fwa.geometry) AND "
                                        "ymin <= MbrMaxY(fwa.geometry) AND "
                                        "ymax >= MbrMinY(fwa.geometry)) "
                                        "GROUP BY fwa.name, status" );

        if( bWriteLargeFire )
        {
            rc = sqlite3_prepare_v2( db, "SELECT fwa.name, SUM(acres), SUM(pop), " \
                                         "SUM(cost) FROM large_fire_result " \
                                         "LEFT JOIN fig.fig USING(year,fire_num) " \
                                         "LEFT JOIN fwa.fwa ON "
                                         "fig.fwa_name=fwa.name " \
                                         "WHERE fwa.name=?",
                                     -1, &lfstmt, NULL );
        }
    }
    rc = sqlite3_prepare_v2( db, pszSql, -1, &stmt, NULL );
    rc = sqlite3_exec( db, "CREATE TABLE spatial_result(name,noresc,tlimit," \
                           "slimit,exhaust,contain,monitor,contratio," \
                           "lfacre,lfpop,lfcost)",
                       NULL, NULL, NULL );
    rc = sqlite3_exec( db, "SELECT AddGeometryColumn('spatial_result','geometry'," \
                           "4269,'MULTIPOLYGON','XY')", NULL, NULL, NULL );

    rc = sqlite3_prepare_v2( db, "INSERT INTO spatial_result "
                                 "VALUES(?,?,?,?,?,?,?,?,?,?,?,?)",
                             -1, &sstmt, NULL );

    if( !bWriteLargeFire )
    {
        lfstmt = NULL;
    }
    char *pszName = NULL;
    int nSize;
    void *pGeom = NULL;
    const char *pszStatus;
    int nCount;
    int nNoResc, nTimeLimit, nSizeLimit, nExhaust, nContain, nMonitor;
    nNoResc = nTimeLimit = nSizeLimit = nNoResc = 0;
    nExhaust = nMonitor = nContain = 0;
    double dfRatio;
    double dfSum;
    int nLfAcre;
    int nLfPop;
    double dfLfCost;
    StartTransaction();
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
                rc = sqlite3_bind_int( sstmt, 7, nMonitor );
                dfSum = nNoResc + nTimeLimit + nSizeLimit + nExhaust + nContain + nMonitor;
                dfRatio = (double)(nContain + nMonitor) / dfSum;
                rc = sqlite3_bind_double( sstmt, 8, dfRatio );
                /* large fire */
                nLfAcre = nLfPop = dfLfCost = 0.;
                if( bWriteLargeFire )
                {
                    rc = sqlite3_bind_text( lfstmt, 1, pszName, -1, NULL );

                    rc = sqlite3_step( lfstmt );
                    if( rc == SQLITE_ROW )
                    {
                        nLfAcre = sqlite3_column_int( lfstmt, 1 );
                        nLfPop = sqlite3_column_int( lfstmt, 2 );
                        dfLfCost = sqlite3_column_double( lfstmt, 3 );
                    }
                    sqlite3_reset( lfstmt );
                }
                rc = sqlite3_bind_int( sstmt, 9, nLfAcre );
                rc = sqlite3_bind_int( sstmt, 10, nLfPop );
                rc = sqlite3_bind_int( sstmt, 11, dfLfCost );
                rc = sqlite3_bind_blob( sstmt, 12, pGeom, nSize, NULL );
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
        if( EQUAL( pszStatus, CONTAINED_STR ) )
            nContain += nCount;
        else if( EQUAL( pszStatus, MONITOR_STR ) )
            nMonitor += nCount;
        else if( EQUAL( pszStatus, NO_RESC_SENT_STR ) )
            nNoResc += nCount;
        else if( EQUAL( pszStatus, TIME_LIMIT_EXCEED_STR ) )
            nTimeLimit += nCount;
        else if( EQUAL( pszStatus, SIZE_LIMIT_EXCEED_STR ) )
            nSizeLimit += nCount;
        else if( EQUAL( pszStatus, EXHAUSTED_STR ) )
            nExhaust += nCount;
    }
    rc = sqlite3_bind_text( sstmt, 1, pszName, -1, NULL );
    rc = sqlite3_bind_int( sstmt, 2, nNoResc );
    rc = sqlite3_bind_int( sstmt, 3, nTimeLimit );
    rc = sqlite3_bind_int( sstmt, 4, nSizeLimit );
    rc = sqlite3_bind_int( sstmt, 5, nExhaust );
    rc = sqlite3_bind_int( sstmt, 6, nContain );
    rc = sqlite3_bind_int( sstmt, 7, nMonitor );
    dfSum = nNoResc + nTimeLimit + nSizeLimit + nExhaust + nContain + nMonitor;
    dfRatio = (double)nContain / dfSum;
    rc = sqlite3_bind_double( sstmt, 8, dfRatio );
    /* large fire */
    nLfAcre = nLfPop = dfLfCost = 0.;
    if( bWriteLargeFire )
    {
        rc = sqlite3_bind_text( lfstmt, 1, pszName, -1, NULL );
        rc = sqlite3_step( lfstmt );
        if( rc == SQLITE_ROW )
        {
            nLfAcre = sqlite3_column_int( lfstmt, 1 );
            nLfPop = sqlite3_column_int( lfstmt, 2 );
            dfLfCost = sqlite3_column_double( lfstmt, 3 );
        }
        sqlite3_reset( lfstmt );
    }
    rc = sqlite3_bind_int( sstmt, 9, nLfAcre );
    rc = sqlite3_bind_int( sstmt, 10, nLfPop );
    rc = sqlite3_bind_int( sstmt, 11, dfLfCost );
    rc = sqlite3_bind_blob( sstmt, 12, pGeom, nSize, NULL );

    rc = sqlite3_step( sstmt );

    rc = sqlite3_reset( stmt );
    rc = sqlite3_reset( sstmt );
    rc = sqlite3_finalize( stmt );
    rc = sqlite3_finalize( sstmt );
    rc = sqlite3_finalize( lfstmt );
    Commit();
    sqlite3_free( pszName );
    sqlite3_free( pGeom );
    sqlite3_free( pszSql );
    return 0;
}

int WfipsResult::ExportFires( const char *pszFile, const char *pszDriver )
{
    if( pszFile == NULL || pszDriver == NULL )
    {
        return SQLITE_ERROR;
    }
    OGRSFDriverH hDrv;
    OGRDataSourceH hDS;
    OGRLayerH hLayer;
    OGRFeatureH hFeature;
    OGRFeatureDefnH hFeatDefn;
    OGRFieldDefnH hFieldDefn;
    OGRGeometryH hGeom;

    OGRSpatialReferenceH hSRS;

    /* Make sure we have fires */
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( db, "SELECT COUNT() FROM fire_result", -1,
                                 &stmt, NULL );
    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW )
    {
        sqlite3_finalize( stmt );
        return SQLITE_ERROR;
    }
    rc = sqlite3_column_int( stmt, 0 );
    if( rc < 1 )
    {
        sqlite3_finalize( stmt );
        return SQLITE_ERROR;
    }
    rc = sqlite3_finalize( stmt );
    hDrv = OGRGetDriverByName( pszDriver );
    if( hDrv == NULL )
    {
        return SQLITE_ERROR;
    }
    hDS = OGR_Dr_CreateDataSource( hDrv, pszFile, NULL );
    if( hDS == NULL )
    {
        return SQLITE_ERROR;
    }
    hSRS = OSRNewSpatialReference( NULL );
    OSRImportFromEPSG( hSRS, 4269 );
    hLayer = OGR_DS_CreateLayer( hDS, "fire_result", hSRS, wkbPoint, NULL );
    OSRDestroySpatialReference( hSRS );
    if( hLayer == NULL )
    {
        OGR_DS_Destroy( hDS );
        return SQLITE_ERROR;
    }
    rc = sqlite3_prepare_v2( db, "PRAGMA table_info(fire_result)", -1, &stmt,
                             NULL );

    const char *pszColName, *pszDataType;
    OGRFieldType eFieldType;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszColName = (const char*)sqlite3_column_text( stmt, 1 );
        pszDataType = (const char*)sqlite3_column_text( stmt, 2 );
        /* We only have int, real, text right now */
        if( EQUAL( pszDataType, "integer" ) )
            eFieldType = OFTInteger;
        else if( EQUAL( pszDataType, "real" ) )
            eFieldType = OFTReal;
        else if( EQUAL( pszDataType, "text" ) )
            eFieldType = OFTString;
        else
            eFieldType = OFTString;
        hFieldDefn = OGR_Fld_Create( pszColName, eFieldType );
        OGR_L_CreateField( hLayer, hFieldDefn, TRUE );
        OGR_Fld_Destroy( hFieldDefn );
    }
    rc = sqlite3_finalize( stmt );
    rc = sqlite3_prepare_v2( db, "SELECT X(geometry), Y(geometry), fire_result.* "
                                 "FROM fire_result",
                             -1, &stmt, NULL );
    hFeatDefn = OGR_L_GetLayerDefn( hLayer );
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        hFeature = OGR_F_Create( hFeatDefn );
        hGeom = OGR_G_CreateGeometry( wkbPoint );
        OGR_G_SetPoint_2D( hGeom, 0, sqlite3_column_double( stmt, 0 ),
                           sqlite3_column_double( stmt, 1 ) );
        OGR_F_SetGeometry( hFeature, hGeom );
        for( int i = 0; i < OGR_FD_GetFieldCount( hFeatDefn ); i++ )
        {
            hFieldDefn = OGR_FD_GetFieldDefn( hFeatDefn, i );
            if( OGR_Fld_GetType( hFieldDefn ) == OFTInteger )
                OGR_F_SetFieldInteger( hFeature, i, sqlite3_column_int( stmt, i + 2 ) );
            else if( OGR_Fld_GetType( hFieldDefn ) == OFTReal )
                OGR_F_SetFieldDouble( hFeature, i, sqlite3_column_double( stmt, i + 2 ) );
            else if( OGR_Fld_GetType( hFieldDefn ) == OFTString )
                OGR_F_SetFieldString( hFeature, i, (const char*)sqlite3_column_text( stmt, i + 2 ) );
        }
        OGR_L_CreateFeature( hLayer, hFeature );
        OGR_G_DestroyGeometry( hGeom );
        OGR_F_Destroy( hFeature );
    }
    OGR_DS_Destroy( hDS );
    return 0;
}

/*
** Export the data for each year by an area.  This is highly related to the
** spat-sum branch.
*/
int
WfipsResult::ExportSummary( const char *pszKey, const char *pszOutFile )
{
    sqlite3_stmt *stmt, *lfstmt, *lfustmt;
    int rc, i, nYear;
    const char *pszPath, *pszSql;
    pszPath = CPLSPrintf( "%s%s", pszDataPath, FPU_DB );
    rc = WfipsAttachDb( db, pszPath, "fpu" );
    pszPath = CPLSPrintf( "%s%s", pszDataPath, FWA_DB );
    rc = WfipsAttachDb( db, pszPath, "fwa" );

    assert( EQUAL( pszKey, "fwa" ) || EQUAL( pszKey, "fpu" ) );

    if( rc != SQLITE_OK )
        return rc;

    int bWriteLargeFire = WfipsHasTable( db, "large_fire_result" );

    const char *pszName, *pszTable;
    if( EQUAL( pszKey, "fpu" ) )
    {
        pszName = "fpu_code";
        pszTable = "fpu";
    }
    else if( EQUAL( pszKey, "fwa" ) )
    {
        pszName = "name";
        pszTable = "fwa";
    }
    else
    {
        assert( 0 );
    }

    /* Make sure we have our indices, no-op otherwise */
    CreateIndices();

    EnableVolatile( 1 );
    StartTransaction();

    /*
    ** This is how we can do this???
    **
    ** sqlite> pragma table_info(t1);
    ** cid         name        type        notnull     dflt_value  pk
    ** ----------  ----------  ----------  ----------  ----------  ----------
    ** 0           c1                      0                       0
    ** 1           c2                      0                       0
    **
    ** sqlite> select c1,c2,count(c2) as val from t1 group by c1,c2;
    ** c1          c2          val
    ** ----------  ----------  ----------
    ** ada         a           7
    ** ada         b           8
    ** ada         c           4
    ** ada         d           1
    ** blaine      a           1
    ** blaine      b           2
    ** blaine      c           2
    ** custer      a           4
    ** custer      d           4
    **
    ** sqlite> select c1, sum(case when c2='a' then val else 0 end) as a, 
    ** sum(case when c2='b' then val else 0 end) as b, sum(case when c2='c'
    ** then val else 0 end) as c, sum(case when c2='d' then val else 0 end)
    ** as d from (select c1,c2,count(c2) as val from t1 group by c1,c2)
    ** group by c1;
    ** c1          a           b           c           d
    ** ----------  ----------  ----------  ----------  ----------
    ** ada         7           8           4           1
    ** blaine      1           2           2           0
    ** custer      4           0           0           4
    **
    ** And with the years:
    **
    ** select c1,c2 as year,sum(case when c3='a' then val else 0 end) as a,
    ** sum(case when c3='b' then val else 0 end) as b, sum(case when c3='c'
    ** then val else 0 end) as c, sum(case when c3='d' then val else 0 end)
    ** as d from (select c1,c2,c3,count(c3) as val from t2 group by c1,c2,c3)
    ** group by c1,c2;
    */
    rc = sqlite3_exec( db, "DROP TABLE IF EXISTS temp_spatial_output", NULL,
                       NULL, NULL );
    pszSql = sqlite3_mprintf( "CREATE TABLE temp_spatial_output AS "
                              "SELECT %s as name, year, "
                              "SUM(CASE WHEN status='"NO_RESC_SENT_STR"'"
                              "    THEN c ELSE 0 END) as noresc,"
                              "SUM(CASE WHEN status='"TIME_LIMIT_EXCEED_STR"'"
                              "    THEN c ELSE 0 END) as tlimit,"
                              "SUM(CASE WHEN status='"SIZE_LIMIT_EXCEED_STR"'"
                              "    THEN c ELSE 0 END) as slimit,"
                              "SUM(CASE WHEN status='"EXHAUSTED_STR"'"
                              "    THEN c ELSE 0 END) as exhaust,"
                              "SUM(CASE WHEN status='"CONTAINED_STR"'"
                              "    THEN c ELSE 0 END) as contain,"
                              "SUM(CASE WHEN status='"MONITOR_STR"'"
                              "    THEN c ELSE 0 END) as monitor, "
                              "0 as lf_acres, 0 as lf_pop, 0 as lf_cost "
                              "FROM(SELECT %s,year,status,COUNT(status) as c "
                              "     FROM fire_result,%s WHERE "
                              "    ST_Intersects(fire_result.geometry,"
                              "    %s.geometry) AND fire_result.ROWID IN "
                              "        (SELECT pkid FROM idx_fire_result_geometry WHERE "
                              "        xmin <= MbrMaxX(%s.geometry) AND "
                              "        xmax >= MbrMinX(%s.geometry) AND "
                              "        ymin <= MbrMaxY(%s.geometry) AND "
                              "        ymax >= MbrMinY(%s.geometry)) "
                              "    GROUP BY %s, year, status) "
                              " GROUP BY name,year", pszName, pszName,
                              pszTable, pszTable, pszTable, pszTable, pszTable,
                              pszTable, pszName );
    rc = sqlite3_exec( db, pszSql, NULL, NULL, NULL );
    sqlite3_free( (void*)pszSql );

    Commit();

    if( bWriteLargeFire )
    {
        const char *pszPlace;
        int nYear, nAcres, nPop, nCost;
        pszSql = sqlite3_mprintf( "SELECT %s,large_fire_result.year,"
                                  "SUM(acres),SUM(pop),SUM(cost) FROM "
                                  "large_fire_result,%s "
                                  "WHERE ST_Intersects(large_fire_result.geometry,"
                                  "    %s.geometry) AND large_fire_result.ROWID IN "
                                  "        (SELECT pkid FROM idx_large_fire_result_geometry WHERE "
                                  "        xmin <= MbrMaxX(%s.geometry) AND "
                                  "        xmax >= MbrMinX(%s.geometry) AND "
                                  "        ymin <= MbrMaxY(%s.geometry) AND "
                                  "        ymax >= MbrMinY(%s.geometry)) "
                                  "    GROUP BY %s,large_fire_result.year",
                                  pszName, pszTable, pszTable, pszTable, pszTable,
                                  pszTable, pszTable, pszName );
        rc = sqlite3_prepare_v2( db, pszSql, -1, &lfstmt, NULL );
        rc = sqlite3_prepare_v2( db, "UPDATE temp_spatial_output SET lf_acres=?,"
                                     "lf_pop=?,lf_cost=? WHERE name=? AND year=?",
                                 -1, &lfustmt, NULL );
        std::vector<std::string>aoPlaces;
        std::string osPlace;
        std::vector<int>anYears;
        std::vector<int>anAcres;
        std::vector<int>anPop;
        std::vector<int>anCost;

        StartTransaction();

        while( sqlite3_step( lfstmt ) == SQLITE_ROW )
        {
            pszPlace = (const char*)sqlite3_column_text( lfstmt, 0 );
            nYear = sqlite3_column_int( lfstmt, 1 );
            nAcres = sqlite3_column_int( lfstmt, 2 );
            nPop = sqlite3_column_int( lfstmt, 3 );
            nCost = sqlite3_column_int( lfstmt, 4 );
            aoPlaces.push_back( std::string( pszPlace ) );
            anYears.push_back( nYear );
            anAcres.push_back( nAcres );
            anPop.push_back( nPop );
            anCost.push_back( nCost );
        }
        rc = sqlite3_reset( lfstmt );
        rc = sqlite3_finalize( lfstmt );

        rc = sqlite3_prepare_v2( db, "UPDATE temp_spatial_output SET lf_acres=?,"
                                     "lf_pop=?,lf_cost=? WHERE name=? AND year=?",
                                 -1, &lfustmt, NULL );
        for( i = 0; i < aoPlaces.size(); i++ )
        {
            osPlace = aoPlaces[i];
            nYear = anYears[i];
            nAcres = anAcres[i];
            nPop = anPop[i];
            nCost = anCost[i];
            rc = sqlite3_bind_int( lfustmt, 1, nAcres );
            rc = sqlite3_bind_int( lfustmt, 2, nPop );
            rc = sqlite3_bind_int( lfustmt, 3, nCost );
            rc = sqlite3_bind_text( lfustmt, 4, osPlace.c_str(), -1, SQLITE_TRANSIENT );
            rc = sqlite3_bind_int( lfustmt, 5, nYear );
            rc = sqlite3_step( lfustmt );
            rc = sqlite3_reset( lfustmt );
        }
        sqlite3_finalize( lfustmt );
        Commit();
    }
    /* Export */
    FILE *fout;
    fout = fopen( pszOutFile, "wb" );
    if( !fout )
    {
        return 1;
    }
    rc = sqlite3_prepare_v2( db, "PRAGMA table_info(temp_spatial_output)", -1,
                             &stmt, NULL );

    int n = 0;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        fprintf( fout, "%s,", sqlite3_column_text( stmt, 1 ) );
        n++;
    }
    /* eat the last comma */
    fseek( fout, -1, SEEK_CUR );
    fprintf( fout, "%s", "\n" );

    rc = sqlite3_finalize( stmt );
    rc = sqlite3_prepare_v2( db, "SELECT * FROM temp_spatial_output", -1, &stmt,
                             NULL );
    const char *pszToken;
    int nType;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        i = 0;
        while( i < n )
        {
            nType = sqlite3_column_type( stmt, i );
            switch( nType )
            {
                case SQLITE_INTEGER:
                    fprintf( fout, "%d,", sqlite3_column_int( stmt, i ) );
                    break;
                case SQLITE_FLOAT:
                    fprintf( fout, "%lf,", sqlite3_column_double( stmt, i ) );
                    break;
                case SQLITE_TEXT:
                    fprintf( fout, "\"%s\",", sqlite3_column_text( stmt, i ) );
                    break;
                case SQLITE_BLOB:
                case SQLITE_NULL:
                    fprintf( fout, ",", sqlite3_column_int( stmt, i ) );
                    break;
            }
            i++;
        }
        fseek( fout, -1, SEEK_CUR );
        fprintf( fout, "%s", "\n" );
    }
    rc = sqlite3_finalize( stmt );
    fclose( fout );
    return 0;
}

WfipsResult::WfipsResult() {}
WfipsResult::WfipsResult( const WfipsResult &rhs ){}

