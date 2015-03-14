/****************************************************************************t*
 *
 * Project:  Wildland Fire Investment Planning System
 * Purpose:  Access on disk data
 * Author:   Kyle Shannon <kyle at pobox dot com>
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

#include "wfips_data.h"

WfipsData * WfipsData::Create( const char *pszPath )
{
    WfipsData *p;
    if( pszPath )
        p = new WfipsData();
    else
        p = new WfipsData( pszPath );
    return p;
}

WfipsData::WfipsData()
{
    Init();
}

WfipsData::WfipsData( const char *pszPath )
{
    Init();
    if( pszPath )
        this->pszPath = sqlite3_mprintf( "%s", pszPath );
}

void
WfipsData::Init()
{
    pszPath = NULL;
    pszRescPath = NULL;
    db = NULL;
    bValid = 0;
    bSpatialiteEnabled = 0;
    iScrap = 0;
    pszAnalysisAreaWkt = NULL;
    poScenario = NULL;
    pszRescPath = NULL;
    poResult = NULL;
}

WfipsData::~WfipsData()
{
    sqlite3_free( pszPath );
    sqlite3_free( pszRescPath );
    sqlite3_free( pszResultPath );
    delete poResult;
    sqlite3_close( db );
    sqlite3_free( (void*)pszAnalysisAreaWkt );
}

int
WfipsData::Open()
{
    if( pszPath == NULL )
    {
        return SQLITE_ERROR;
    }
    return Open( pszPath );
}

char*
WfipsData::GetScrapBuffer()
{
    iScrap++;
    iScrap = iScrap >= WFIPS_SCRAP_BUFFER_SIZE ?
             iScrap % WFIPS_SCRAP_BUFFER_SIZE :
             iScrap;
    memset( szScrap[iScrap], '\0', MAX_PATH );
    return szScrap[iScrap];
}

const char*
WfipsData::FormFileName( const char *pszPath,
                         const char *pszDb )
{
    int nPathLength = strlen( pszPath );
    int nDbLength = strlen( pszDb );
    char cTrail = *(pszPath + nPathLength - 1);
    char *pszScrap = GetScrapBuffer();
    strncat( pszScrap, pszPath, MAX_PATH );
    if( cTrail != '/' && cTrail != '\\' )
    {
        strncat( pszScrap, "/", MAX_PATH - nPathLength );
    }
    strncat( pszScrap, pszDb, MAX_PATH - nPathLength - 1 );
    return pszScrap;
}

const char*
WfipsData::BaseName( const char *pszPath )
{
    char *pszScrap = GetScrapBuffer();
    const char *p, *q;
    p = strrchr( pszPath, '/' ) ? strrchr( pszPath, '/' ) : strrchr( pszPath, '\\' );
    if( p == NULL )
    {
        p = pszPath;
    }
    else if( p + 1 < pszPath + strlen( pszPath ) - 1 )
    {
        p++;
    }
    q = strrchr( p, '.' );
    if( q == NULL )
        q = p + strlen( p ) - 1;
    strncpy( pszScrap, p, q - p );
    return pszScrap;
}

/*
** Attach a database file as it's basename, ie:
**
** /home/kyle/src/somedb.db -> ATTACH /home/kyle/src/somedb.db AS somedb
*/
int
WfipsData::Attach( const char *pszPath )
{
    return WfipsAttachDb( db, pszPath, BaseName( pszPath ) );
    char *pszSql;
    int rc;
    if( db == NULL )
    {
        return SQLITE_ERROR;
    }
    pszSql = sqlite3_mprintf( "ATTACH %Q AS %s", pszPath, BaseName( pszPath ) );
    rc = sqlite3_exec( db, pszSql, NULL, NULL, NULL );
    sqlite3_free( pszSql );
    return rc;
}

#define WFIPS_CHECK_STATUS if(rc != SQLITE_OK) goto error

int
WfipsData::Open( const char *pszPath )
{
    if( pszPath == NULL )
    {
        return SQLITE_ERROR;
    }
    int rc;
    int i;

    /* Open one db, attach the rest */
    rc = sqlite3_open_v2( FormFileName( pszPath, apszDbFiles[0] ), &db,
                          SQLITE_OPEN_READWRITE, NULL );
    WFIPS_CHECK_STATUS;
    rc = sqlite3_limit( db, SQLITE_LIMIT_ATTACHED, WFIPS_DB_COUNT + 1 );
    i = 1;
    while( apszDbFiles[i] != NULL )
    {
        rc = Attach( FormFileName( pszPath, apszDbFiles[i++] ) );
        WFIPS_CHECK_STATUS;
    }
    rc = sqlite3_enable_load_extension( db, 1 );
    if( rc != SQLITE_OK )
    {
        bSpatialiteEnabled = 0;
        rc = SQLITE_OK;
    }
    else
    {
        rc = sqlite3_load_extension( db, SPATIALITE_EXT, NULL, NULL );
        if( rc != SQLITE_OK )
        {
            bSpatialiteEnabled = 0;
            printf( "Warning, spatialite could not be loaded!\n" );
            rc = SQLITE_OK;
        }
        else
        {
            bSpatialiteEnabled = 1;
        }
    }
    bValid = 1;
    return rc;

error:
    Close();
    return rc;
}

int
WfipsData::ExecuteSql( const char *pszSql )
{
    sqlite3_stmt *stmt;
    int rc;
    rc = sqlite3_prepare_v2( db, pszSql, -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    sqlite3_finalize( stmt );
    stmt = NULL;
    rc = rc == SQLITE_ROW || rc == SQLITE_DONE ? 0 : 1;
    return rc;
}

double
WfipsData::Random()
{
    double d;
    int n;
    sqlite3_randomness( sizeof( int ), &n );
    d = (double)abs(n)/(double)INT_MAX;
    assert( d >= 0.0  && d <= 1.0 );
    return d;
}

/*
** Compile a geometry into spatialite binary.  To be free'd by the caller using
** sqlite3_free()
*/
int
WfipsData::CompileGeometry( const char *pszWkt, void **pCompiled )
{
    const void *p;
    int n, rc;
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, "SELECT GeomFromText(?)", -1, &stmt, NULL );
    WFIPS_CHECK_STATUS;
    rc = sqlite3_bind_text( stmt, 1, pszWkt, -1, NULL );
    WFIPS_CHECK_STATUS;
    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW )
        goto error;
    n = sqlite3_column_bytes( stmt, 0 );
    p = sqlite3_column_blob( stmt, 0 );
    *pCompiled = sqlite3_malloc( n );
    memcpy( *pCompiled, p, n );
    sqlite3_finalize( stmt );
    return n;
error:
    sqlite3_finalize( stmt );
    return 0;
}

/*
** Find all dispatch locations associated with any fwa intersecting the polygon
** defined by pszWkt.  Note that FPU boundaries and FWA boundaries were derived
** separately, and intersect frequently when they appear coincident.
** [in]  pszWkt           -> boundary to intersect with fwa geometry.
** [out] panDispLocIds    -> array of integers to be filled.
** [out] nCount           -> size of array filled.
** FIXME: [in]  pfnProgress -> optional progress function (to be implemented)
*
* Resulting int array should be freed with the static member WfipsData::Free(),
* which is analagous to sqlite3_free().
*
* This is primarily used for display, not loading data for the simulation.  The
* GUI will need to know what locations are available for editing.
*/
int
WfipsData::GetAssociatedDispLoc( const char *pszWkt,
                                 int **panDispLocIds,
                                 int *pnCount /*,
                                 IRSProgress pfnProgress*/ )
{
    assert( pnCount );
    if( bSpatialiteEnabled == 0 )
    {
        *pnCount = 0;
        return SQLITE_ERROR;
    }
    int rc;
    int i, n, nGeomSize;
    sqlite3_stmt *stmt;
    const void *p;
    void *pAnalysisGeometry;
    /*
    ** Just allocate for all locations, then realloc when we know how many
    ** there are. We don't want to run spatial queries twice to get a real
    ** count.
    */
    rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM disploc", -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    assert( rc == SQLITE_ROW );
    n = sqlite3_column_int( stmt, 0 );
    *panDispLocIds = (int*)sqlite3_malloc( sizeof( int ) * n );
    sqlite3_finalize( stmt );
    nGeomSize = CompileGeometry( pszWkt, &pAnalysisGeometry );
    if( nGeomSize == 0 )
    {
        *pnCount = 0;
        return SQLITE_ERROR;
    }

    /* Find the associated locations */
    rc = sqlite3_prepare_v2( db, "SELECT DISTINCT(disploc.ROWID) FROM " \
                                 "disploc JOIN assoc ON name=disploc_name " \
                                 "WHERE fwa_name IN " \
                                 "(SELECT name FROM fwa WHERE " \
                                 "ST_Intersects(?1, fwa.geometry) AND " \
                                 "fwa.ROWID IN(SELECT pkid FROM "
                                 "idx_fwa_geometry WHERE "
                                 "xmin <= MbrMaxX(?1) AND "
                                 "xmax >= MbrMinX(?1) AND "
                                 "ymin <= MbrMaxY(?1) AND "
                                 "ymax >= MbrMinY(?1)))",
                             -1, &stmt, NULL );

    rc = sqlite3_bind_blob( stmt, 1, pAnalysisGeometry, nGeomSize, NULL );
    n = i = 0;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        (*panDispLocIds)[i++] = sqlite3_column_int( stmt, 0 );
    }
    *panDispLocIds = (int*)sqlite3_realloc( (*panDispLocIds), sizeof( i ) * i );
    *pnCount = i;
    sqlite3_finalize( stmt );
    sqlite3_free( pAnalysisGeometry );
    return SQLITE_OK;
}

char *
WfipsData::BuildFidSet( int *panFids, int nCount )
{
    sqlite3_stmt *stmt;
    int rc;
    int nOrder;
    rc = sqlite3_prepare_v2( db, "SELECT MAX(ROWID) FROM resource",
                             -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW )
    {
        sqlite3_finalize( stmt );
        return NULL;
    }

    nOrder = strlen( (const char *)sqlite3_column_text( stmt, 0 ) );

    sqlite3_finalize( stmt );

    int nSize = (nCount * (nOrder + 1)) + 1;
    char *pszSet = (char*)sqlite3_malloc( nSize );
    if( pszSet == NULL )
    {
        return NULL;
    }
    *pszSet = '\0';
    int i = 0;
    int n = 0;
    char *pszId = (char*)sqlite3_malloc( nOrder + 1 );
    for( ; i < nCount; i++ )
    {
        *pszId = '\0';
        sprintf( pszId, "%d", panFids[i] );
        assert( strlen( pszId ) + strlen( pszSet ) < nSize - 1 );
        pszSet = strcat( pszSet, pszId );
        if( i < nCount - 1 )
            pszSet = strcat( pszSet, "," );
    }
    sqlite3_free( pszId );
    pszSet = (char*)sqlite3_realloc( pszSet, strlen( pszSet ) + 1 );
    /* To be free'd by the caller */
    return pszSet;
}

const char *
WfipsData::BuildAgencySet( int nAgencyFlags )
{
    if( nAgencyFlags == 0 )
    {
        nAgencyFlags = AGENCY_ALL;
    }
    char *pszSet = GetScrapBuffer();
    /* Count the flags for our commas */
    int i, n;
    n = 0;
    for( i = 1; i < 7; i++ )
    {
        if( nAgencyFlags & 1 << i )
            n++;
    }

    for( i = 1; i < 7; i++ )
    {
        if( nAgencyFlags & 1 << i )
        {
            strncat( pszSet, aszAgencyNames[i], MAX_PATH );
            n--;
            if( n )
            {
                strncat( pszSet, ",", MAX_PATH );
            }
        }
    }
    return pszSet;
}

int
WfipsData::GetAssociatedResources( int *panDispLocIds, int nDispLocCount,
                                   WfipsResc **ppsResc, int *pnRescCount,
                                   int nAgencyFlags )
{
    assert( pnRescCount );
    const char *pszAgencySet = BuildAgencySet( nAgencyFlags );
    sqlite3_stmt *stmt;
    char *pszSql;
    int nCount, n, i, rc;
    rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM resource",
                             -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    nCount = sqlite3_column_int( stmt, 0 );
    *ppsResc = (WfipsResc*)sqlite3_malloc( sizeof( WfipsResc ) * nCount );

    rc = sqlite3_finalize( stmt );

    pszSql = sqlite3_mprintf( "SELECT disploc.name, resource.ROWID, " \
                              "resource.name, resource.resc_type " \
                              "FROM resource LEFT JOIN disploc ON " \
                              "resource.disploc=disploc.name " \
                              "WHERE disploc.ROWID=? AND " \
                              "resource.agency IN(%s)", pszAgencySet );

    rc = sqlite3_prepare_v2( db, pszSql, -1, &stmt, NULL );
    sqlite3_free( pszSql );

    n = 0;
    for( i = 0; i < nDispLocCount; i++ )
    {
        rc = sqlite3_bind_int( stmt, 1, panDispLocIds[i] );
        while( sqlite3_step( stmt ) == SQLITE_ROW )
        {
            (*ppsResc)[n].pszDispLoc = sqlite3_mprintf( "%s", sqlite3_column_text( stmt, 0 ) );
            (*ppsResc)[n].nId = sqlite3_column_int( stmt, 1 );
            (*ppsResc)[n].pszName = sqlite3_mprintf( "%s", sqlite3_column_text( stmt, 2 ) );
            (*ppsResc)[n].pszType = sqlite3_mprintf( "%s", sqlite3_column_text( stmt, 3 ) );
            n++;
        }
        sqlite3_reset( stmt );
    }
    rc = sqlite3_finalize( stmt );

    *pnRescCount = n;
    return 0;
}

void
WfipsData::FreeAssociatedResources( WfipsResc *psResc, int nCount )
{
    int i;
    if( !psResc )
        return;
    for( i = 0; i < nCount; i++ )
    {
        sqlite3_free( psResc[i].pszDispLoc );
        sqlite3_free( psResc[i].pszName );
        sqlite3_free( psResc[i].pszType );
    }
    sqlite3_free( psResc );
    return ;
}

void
WfipsData::Free( void *p )
{
    sqlite3_free( p );
}

int
WfipsData::Close()
{
    int rc = SQLITE_OK;
    rc = sqlite3_close( db );
    db = NULL;
    bValid = 0;
    return rc;
}

int
WfipsData::SetRescDb( const char *pszPath )
{
    int rc;
    char *pszSql = NULL;
    sqlite3_free( pszRescPath );
    if( pszPath )
        pszRescPath = sqlite3_mprintf( "%s", pszPath );
    else
    {
        pszRescPath = NULL;
        return SQLITE_ERROR;
    }
    rc = sqlite3_exec( db, "DETACH resc", NULL, NULL, NULL );
    pszSql = sqlite3_mprintf( "ATTACH %Q AS resc", pszPath );
    rc = sqlite3_exec( db, pszSql, NULL, NULL, NULL );
    sqlite3_free( pszSql );
    return rc;
}

int
WfipsData::WriteRescDb( const char *pszPath, int *panIds, int *panDispLocIds,
                        int nCount )
{
    int i, n, rc;
    sqlite3 *brdb;
    sqlite3 *rdb;
    sqlite3_stmt *stmt;
    char *pszSchema;
    char szRescId[128];
    char *pszRescSet;

    if( nCount < 1 || panIds == NULL )
    {
        return SQLITE_ERROR;
    }

    int bUseExtResc = pszRescPath ? 1 : 0;

    rc = sqlite3_open_v2( pszPath, &rdb,
                          SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE,
                          NULL );
    if( rc != SQLITE_OK )
    {
        return rc;
    }
    if( bUseExtResc )
    {
        rc = sqlite3_open_v2( pszRescPath, &brdb, SQLITE_OPEN_READONLY, NULL );
        if( rc != SQLITE_OK )
        {
            sqlite3_close( rdb );
            return rc;
        }
        rc = sqlite3_prepare_v2( brdb, "SELECT sql FROM sqlite_master " \
                                       "WHERE type='table' AND name='resource'",
                                 -1, &stmt, NULL );
    }
    else
    {
        brdb = db;
        rc = sqlite3_prepare_v2( brdb, "SELECT sql FROM resc.sqlite_master " \
                                       "WHERE type='table' AND name='resource'",
                                 -1, &stmt, NULL );
    }
    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW )
    {
        sqlite3_finalize( stmt );
        sqlite3_close( db );
        sqlite3_close( rdb );
        return rc;
    }
    pszSchema = sqlite3_mprintf( "%s", (char*)sqlite3_column_text( stmt, 0 ) );
    sqlite3_finalize( stmt );
    rc = sqlite3_exec( rdb, pszSchema, NULL, NULL, NULL );
    assert( rc == SQLITE_OK );
    sqlite3_free( pszSchema );

    pszRescSet = BuildFidSet( panIds, nCount );

    const char *pszBaseRescPath;
    if( bUseExtResc )
    {
        pszBaseRescPath = this->pszRescPath;
    }
    else
    {
        pszBaseRescPath = FormFileName( this->pszPath, RESC_DB );
    }
    char *pszSql = sqlite3_mprintf( "ATTACH %Q AS baseresc", pszBaseRescPath );
    rc = sqlite3_exec( rdb, pszSql, NULL, NULL, NULL );
    sqlite3_free( pszSql );

    rc = sqlite3_exec( rdb, "BEGIN", NULL, NULL, NULL );
    pszSql = sqlite3_mprintf( "INSERT INTO resource SELECT * FROM " \
                              "baseresc.resource WHERE ROWID " \
                              "IN (%s)", pszRescSet );
    sqlite3_free( pszRescSet );
    rc = sqlite3_exec( rdb, pszSql, NULL, NULL, NULL );
    sqlite3_free( pszSql );
    rc = sqlite3_exec( rdb, "COMMIT", NULL, NULL, NULL );
    sqlite3_exec( rdb, "DETACH baseresc", NULL, NULL, NULL );
    /*
    ** If the user has changed locations, run through and update them. -1 means
    ** no change, > 0 means change.
    */
    if( panDispLocIds != NULL )
    {
        const char *pszDispPath = FormFileName( this->pszPath, DISPLOC_DB );
        const char *pszDispName;
        pszSql = sqlite3_mprintf( "ATTACH %Q AS disploc", pszDispPath );
        rc = sqlite3_exec( rdb, pszSql, NULL, NULL, NULL );
        sqlite3_free( pszSql );
        sqlite3_stmt *ustmt, *sstmt;
        rc = sqlite3_prepare_v2( rdb, "SELECT name FROM disploc WHERE ROWID=?",
                                 -1, &sstmt, NULL );
        rc = sqlite3_prepare_v2( rdb, "UPDATE resource SET disploc=?" \
                                      "WHERE ROWID=?",
                                 -1, &ustmt, NULL );
        rc = sqlite3_exec( rdb, "BEGIN", NULL, NULL, NULL );
        for( i = 0; i < nCount; i++ )
        {
            if( panDispLocIds[i] < 1 )
                continue;
            rc = sqlite3_bind_int( sstmt, 1, panDispLocIds[i] );
            rc = sqlite3_step( sstmt );
            pszDispName = (const char *)sqlite3_column_text( sstmt, 0 );
            rc = sqlite3_bind_text( ustmt, 1, pszDispName, -1, NULL );
            rc = sqlite3_bind_int( ustmt, 2, panIds[i] );
            rc = sqlite3_step( ustmt );
            sqlite3_reset( sstmt );
            sqlite3_reset( ustmt );
        }
        rc = sqlite3_exec( rdb, "COMMIT", NULL, NULL, NULL );
        rc = sqlite3_exec( rdb, "DETACH disploc", NULL, NULL, NULL );
        rc = sqlite3_finalize( sstmt );
        rc = sqlite3_finalize( ustmt );
    }
    if( bUseExtResc )
    {
        sqlite3_close( brdb );
    }
    sqlite3_close( rdb );
    return rc;
}

int
WfipsData::LoadScenario( int nYearIdx, const char *pszTreatWkt,
                         double dfTreatProb, int nWfpTreatMask,
                         double *padfWfpTreatProb, double dfStratProb,
                         int nAgencyFilter )
{
    sqlite3_stmt *stmt;
    sqlite3_stmt *gstmt = NULL;
    int i, n, rc;
    char *pszSql, *pszAnalysisAreaSql, *pszOwnerSql;
    int nAnalysisGeomSize;
    void *pAnalysisGeom = NULL;
    int nTreatSize;
    void *pTreatGeom = NULL;
    poScenario->m_VFire.clear();
    if( pszAnalysisAreaWkt != NULL )
    {
        pszAnalysisAreaSql =
            sqlite3_mprintf( "AND ST_Contains(@geom, geometry) AND " \
                             "fig.ROWID IN(SELECT pkid FROM " \
                             "idx_fig_geometry WHERE " \
                             "xmin <= MbrMaxX(@geom) AND "
                             "xmax >= MbrMinX(@geom) AND "
                             "ymin <= MbrMaxY(@geom) AND "
                             "ymax >= MbrMinY(@geom))" );
    }
    else
    {
        pszAnalysisAreaSql = sqlite3_mprintf( "%s", "" );
    }
    if( nAgencyFilter != AGENCY_ALL && nAgencyFilter != 0 )
    {
        const char *pszAgencySet = BuildAgencySet( nAgencyFilter );
        pszOwnerSql = sqlite3_mprintf( "AND owner IN(%s)", pszAgencySet );
    }
    else
    {
        pszOwnerSql = sqlite3_mprintf( "%s", "" );
    }

    pszSql = sqlite3_mprintf( "SELECT *, X(geometry), Y(geometry) FROM " \
                              "fig WHERE year=@yidx AND fwa_name NOT " \
                              "LIKE '%%unassign%%' %s %s " \
                              "ORDER BY jul_day, disc_time",
                              pszAnalysisAreaSql, pszOwnerSql );

    rc = sqlite3_prepare_v2( db, pszSql, -1, &stmt, NULL );
    rc = sqlite3_bind_int( stmt, sqlite3_bind_parameter_index( stmt, "@yidx" ),
                           nYearIdx );
    if( pszAnalysisAreaWkt != NULL )
    {
        nAnalysisGeomSize = CompileGeometry( pszAnalysisAreaWkt, &pAnalysisGeom );
        if( nAnalysisGeomSize > 0 )
        {
            rc = sqlite3_bind_blob( stmt,
                                    sqlite3_bind_parameter_index( stmt, "@geom" ),
                                    pAnalysisGeom, nAnalysisGeomSize, sqlite3_free );
        }
        else
        {
            /* Panic? */
        }
    }

    if( pszTreatWkt != NULL && dfTreatProb > 0. )
    {
        rc = sqlite3_prepare_v2( db, "SELECT 1 WHERE " \
                                     "ST_Contains(@treat, @fig) AND " \
                                     "X(@fig) <= MbrMaxX(@treat) AND " \
                                     "X(@fig) >= MbrMinX(@treat) AND " \
                                     "Y(@fig) <= MbrMaxY(@treat) AND " \
                                     "Y(@fig) >= MbrMinY(@treat)",
                                 -1, &gstmt, NULL );
        nTreatSize = CompileGeometry( pszTreatWkt, &pTreatGeom );
        if( nTreatSize > 0 )
        {
            rc = sqlite3_bind_blob( gstmt,
                                    sqlite3_bind_parameter_index( gstmt, "@treat" ),
                                    pTreatGeom, nTreatSize, sqlite3_free );
        }
        else
        {
            /* Panic */
        }
    }

    int nYear, nFire, nJulDay;
    const char *pszWeekDay, *pszDiscTime;
    int nBi;
    double dfRos;
    int nElev, nFbfm;
    const char *pszSc;
    int nSlope, bWalkIn;
    const char *pszTactic;
    double dfAttDist, dfRatio;
    const char *pszSunrise, *pszSunset;
    int bWaterDrops, bPumpRoll;
    const char *pszFwa;
    double dfGmtOffset;
    int nTreatBi;
    double dfTreatRos;
    int nTreatFbfm;
    const char *pszTreatSc;
    double dfTreatRatio;
    const char *pszOwner;
    int nWfpTpa;
    double dfManObj;
    void *pFigGeom;
    double dfX, dfY;

    int bTreated;
    int nFigGeomSize;
    int nMinSteps = 250;
    int nMaxSteps = 10000;

    int iFire = 0;
    std::map<std::string, int>::iterator it;
    int nInvalid = 0;
    int nProb;
    double dfProb, dfUserProb;

    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        nYear = sqlite3_column_int( stmt, 0 );
        nFire = sqlite3_column_int( stmt, 1 );
        nJulDay = sqlite3_column_int( stmt, 2 );
        pszWeekDay = apszWfipsDayOfWeek[sqlite3_column_int( stmt, 3 )+1];
        pszDiscTime = (const char*)sqlite3_column_text( stmt, 4 );
        nBi = sqlite3_column_int( stmt, 5 );
        dfRos = sqlite3_column_double( stmt, 6 );
        nElev = sqlite3_column_int( stmt, 7 );
        nFbfm = sqlite3_column_int( stmt, 8 );
        pszSc = (const char*)sqlite3_column_text( stmt, 9 );
        nSlope = sqlite3_column_int( stmt, 10 );
        bWalkIn = sqlite3_column_int( stmt, 11 );
        pszTactic = (const char*)sqlite3_column_text( stmt, 12 );
        dfAttDist = sqlite3_column_double( stmt, 13 );
        dfRatio = sqlite3_column_double( stmt, 14 );
        pszSunrise = (const char*)sqlite3_column_text( stmt, 15 );
        pszSunset = (const char*)sqlite3_column_text( stmt, 16 );
        bWaterDrops = sqlite3_column_int( stmt, 17 );
        bPumpRoll = sqlite3_column_int( stmt, 18 );
        pszFwa = (const char*)sqlite3_column_text( stmt, 19 );
        dfGmtOffset = sqlite3_column_double( stmt, 20 );
        nTreatBi = sqlite3_column_int( stmt, 21 );
        dfTreatRos = sqlite3_column_double( stmt, 22 );
        nTreatFbfm = sqlite3_column_int( stmt, 23 );
        pszTreatSc = (const char*)sqlite3_column_text( stmt, 24 );
        dfTreatRatio = sqlite3_column_double( stmt, 25 );
        n = sqlite3_column_bytes( stmt, 26 );
        pFigGeom = (void*)sqlite3_column_blob( stmt, 26 );
        pszOwner = (const char*)sqlite3_column_text( stmt, 27 );
        nWfpTpa = sqlite3_column_int( stmt, 28 );
        dfManObj = sqlite3_column_double( stmt, 29 );

        /* X and Y from spatialite */
        dfX = sqlite3_column_double( stmt, 30 );
        dfY = sqlite3_column_double( stmt, 31 );

        bTreated = 0;

        if( dfTreatProb > 0. )
        {
            dfUserProb = dfTreatProb;
        }
        else if( nWfpTreatMask && nWfpTpa > 0 )
        {
            dfUserProb = padfWfpTreatProb[nWfpTpa-1];
        }
        else
        {
            dfUserProb = 1.;
        }
        dfProb = Random();

        if( pszTreatWkt != NULL && dfTreatProb > 0. )
        {
            rc = sqlite3_bind_blob( gstmt,
                                    sqlite3_bind_parameter_index( gstmt, "@fig" ),
                                    pFigGeom, n, NULL );
            rc = sqlite3_step( gstmt );
            if( rc == SQLITE_ROW && sqlite3_column_int( gstmt, 0 ) == 1 )
            {
                bTreated = 1;
            }
            sqlite3_reset( gstmt );
        }
        else if( nWfpTreatMask )
        {
            bTreated = nWfpTreatMask & (1 << nWfpTpa);
        }
        else if( dfTreatProb > 0. )
        {
            bTreated = 1;
        }
        if( bTreated )
        {
            if( dfProb < dfUserProb )
            {
                /* Set FB params, etc. */
                nBi = nTreatBi;
                dfRos = dfTreatRos;
                nFbfm = nTreatFbfm;
                pszSc = pszTreatSc;
                dfRatio = dfTreatRatio;
            }
            else
            {
                bTreated = 0;
            }
        }
        it = FwaIndexMap.find( pszFwa );
        if( it == FwaIndexMap.end() )
        {
            nInvalid++;
            //printf("Invalid fwa name: %s\n", pszFwa);
            continue;
        }
        i = it->second;
        poScenario->m_VFire.push_back( CFire( nYear, nFire, nJulDay,
                                              std::string( pszWeekDay ),
                                              std::string( pszDiscTime ), nBi,
                                              dfRos, nFbfm,
                                              std::string( pszSc ),
                                              nSlope, (bool)bWalkIn,
                                              std::string( pszTactic ),
                                              dfAttDist, nElev, dfRatio,
                                              nMinSteps, nMaxSteps,
                                              std::string( pszSunrise ),
                                              std::string( pszSunset ),
                                              (bool)bWaterDrops, (bool)bPumpRoll,
                                              (CFWA&)(poScenario->m_VFWA[i]),
                                              dfY, dfX ) );
        poScenario->m_VFire[iFire].SetTreated( bTreated );
        poScenario->m_VFire[iFire].SetManageObjective( dfManObj );
        /* XXX: Set up properly (3A or 3B) */
        if( Random() < dfStratProb  && dfManObj >= 2.9 && dfManObj < 4.0 )
            poScenario->m_VFire[iFire].SetUseStrategy( 1 );
        else
            poScenario->m_VFire[iFire].SetUseStrategy( 0 );

        iFire++;
    }
    poScenario->m_NumFire = poScenario->m_VFire.size();
    //printf( "Warning. %d fires failed to load due to invalid fwa names\n", nInvalid );

    sqlite3_finalize( stmt );
    sqlite3_finalize( gstmt );
    sqlite3_free( pszAnalysisAreaSql );
    sqlite3_free( pszOwnerSql );
    sqlite3_free( pszSql );

    return 0;
}

int
WfipsData::GetScenarioIndices( int **ppanIndices )
{
    int rc, i, n;
    assert( ppanIndices != NULL );
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, "SELECT COUNT(DISTINCT(year)) FROM fig", -1,
                             &stmt, NULL );
    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW )
        return 0;
    n = sqlite3_column_int( stmt, 0 );
    sqlite3_finalize( stmt );
    *ppanIndices = (int*)sqlite3_malloc( sizeof( int ) * n );
    rc = sqlite3_prepare_v2( db, "SELECT DISTINCT(year) FROM fig", -1, &stmt,
                             NULL );
    i = 0;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        assert( i < n );
        (*ppanIndices)[i++] = sqlite3_column_int( stmt, 0 );
    }
    sqlite3_finalize( stmt );
    return n;
}

int
WfipsData::SetAnalysisAreaMask( const char *pszMaskWkt )
{
    sqlite3_free( (void*)pszAnalysisAreaWkt );
    if( pszMaskWkt )
    {
        pszAnalysisAreaWkt = sqlite3_mprintf( "%s", pszMaskWkt );
    }
    else
    {
        pszAnalysisAreaWkt = NULL;
    }
    return SQLITE_OK;
}

int
WfipsData::SetResultPath( const char *pszPath )
{
    if( pszPath == NULL )
    {
        return SQLITE_ERROR;
    }
    pszResultPath = sqlite3_mprintf( "%s", pszPath );
    poResult = new WfipsResult( pszResultPath, this->pszPath );
    if( !poResult->Valid() )
    {
        delete poResult;
        poResult = NULL;
        sqlite3_free( pszResultPath );
        pszResultPath = NULL;
        return SQLITE_ERROR;
    }
    return SQLITE_OK;
}

int
WfipsData::WriteResults()
{
    int i, rc;
    if( poResult == NULL )
        return SQLITE_ERROR;
    poResult->StartTransaction();
    for( i = 0; i < poScenario->m_VResults.size(); i++ )
    {
        poResult->WriteRecord( poScenario->m_VResults[i] );
    }
    poResult->Commit();
    return 0;
}

int
WfipsData::RunScenario( int iYearIdx )
{
    int rc;
    rc = poScenario->RunScenario( 0, iYearIdx, NULL );
    poScenario->Output();
    WriteResults();
    /* XXX: DO THESE GO HERE?! XXX */
    poScenario->Reset();
    poScenario->m_VResults.clear();

    return rc;
}

