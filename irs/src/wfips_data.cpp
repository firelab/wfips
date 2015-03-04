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
    //pszAnalysisAreaWkt = NULL;
    nIgnOwnMask = AGENCY_ALL;
    pszFuelTreatMaskWkt = NULL;
    dfTreatProb = 1.0;
}

WfipsData::~WfipsData()
{
    sqlite3_free( pszPath );
    sqlite3_free( pszRescPath );
    sqlite3_close( db );
    //sqlite3_free( (void*)pszAnalysisAreaWkt );
    sqlite3_free( (void*)pszFuelTreatMaskWkt );
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
WfipsData::LoadScenario( int nYearIdx, const char *pszAnalysisAreaWkt,
                         const char *pszTreatWkt, double dfTreatProb,
                         int nAgencyFilter )
{
    sqlite3_stmt *stmt;
    int n, rc;
    char *pszSql, *pszAnalysisAreaSql, *pszOwnerSql;
    int nAnalysisGeomSize;
    void *pAnalysisGeom = NULL;
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

    pszSql = sqlite3_mprintf( "SELECT * FROM fig WHERE year=@yidx %s %s",
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
                                    pAnalysisGeom, nAnalysisGeomSize, NULL );
        }
        else
        {
            /* Panic? */
        }
    }

    n = 0;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        n++;
    }

    sqlite3_finalize( stmt );
    sqlite3_free( pszAnalysisAreaSql );
    sqlite3_free( pszOwnerSql );
    sqlite3_free( pszSql );
    sqlite3_free( pAnalysisGeom );

    return n;
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
    return 0;
    //sqlite3_free( (void*)pszAnalysisAreaWkt );
    //if( pszMaskWkt )
        //pszAnalysisAreaWkt = sqlite3_mprintf( "%s", pszMaskWkt );
    //else
        //pszAnalysisAreaWkt = NULL;
    //return SQLITE_OK;
}

int
WfipsData::SetFuelTreatmentMask( const char *pszMaskWkt, double dfProb )
{
    assert( dfProb >= 0. && dfProb <= 1. );
    sqlite3_free( (void*)pszFuelTreatMaskWkt );
    if( pszMaskWkt )
        pszFuelTreatMaskWkt = sqlite3_mprintf( "%s", pszMaskWkt );
    else
        pszFuelTreatMaskWkt = NULL;
    dfTreatProb = dfProb;
    return SQLITE_OK;
}
int
WfipsData::SetLargeFireParams( int nJulStart, int nJulEnd, double dfNoRescProb,
                               double dfTimeLimitProb, double dfSizeLimitProb,
                               double dfExhaustProb )
{
    assert( nJulStart < nJulEnd );
    assert( nJulStart < 366 && nJulStart > 0 );
    assert( nJulEnd < 366 && nJulEnd > 0 );
    assert( dfNoRescProb <= 1.0 && dfNoRescProb >= 0.0 );
    assert( dfTimeLimitProb <= 1.0 && dfTimeLimitProb >= 0.0 );
    assert( dfSizeLimitProb <= 1.0 && dfSizeLimitProb >= 0.0 );
    assert( dfExhaustProb  <= 1.0 && dfExhaustProb  >= 0.0 );

    nLfJulStart = nJulStart;
    nLfJulEnd = nJulEnd;
    dfLfNoRescProb = dfNoRescProb;
    dfLfTimeLimitProb = dfTimeLimitProb;
    dfLfSizeLimitProb = dfSizeLimitProb;
    dfLfExhaustProb = dfExhaustProb;
}

