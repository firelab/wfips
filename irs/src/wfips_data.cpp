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

static WfipsData * Create( const char *pszPath )
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
}

WfipsData::~WfipsData()
{
    sqlite3_free( pszPath );
    sqlite3_free( pszRescPath );
    sqlite3_close( db );
}

int
WfipsData::Open()
{
    if( pszPath == NULL )
    {
        return 1;
    }
    return Open( pszPath );
}

char*
WfipsData::GetScrapBuffer()
{
    iScrap++;
    if( iScrap >= 10 )
        iScrap = iScrap % 10;
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
        return 1;
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
        return 1;
    int rc;
    int i;

    rc = sqlite3_open_v2( FormFileName( pszPath, COST_DB ), &db,
                          SQLITE_OPEN_READWRITE, NULL );
    WFIPS_CHECK_STATUS;
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
            rc = SQLITE_OK;
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
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        printf("SQLITE_ROW\n");
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return rc;
}

int
WfipsData::GetAssociatedDispLoc( const char *pszWkt,
                                 int **panDispLocIds,
                                 int *nCount )
{
    assert( nCount );
    if( bSpatialiteEnabled == 0 )
    {
        if( nCount )
        {
            *nCount = 0;
        }
        return 1;
    }
    int rc;
    int n;
    sqlite3_stmt *stmt;
    const void *p;
    void *pGeometry;
    rc = sqlite3_prepare_v2( db, "SELECT AsText(?)", -1, &stmt, NULL );
    rc = sqlite3_bind_text( stmt, 1, pszWkt, -1, NULL );
    rc = sqlite3_step( stmt );
    n = sqlite3_column_bytes( stmt, 0 );
    p = sqlite3_column_blob( stmt, 0 );
    pGeometry = sqlite3_malloc( n );
    memcpy( pGeometry, p, n );
    sqlite3_finalize( stmt );

    return 0;
}

int WfipsData::Close()
{
    int rc = SQLITE_OK;
    rc = sqlite3_close( db );
    db = NULL;
    bValid = 0;
    return rc;
}

int WfipsData::SetRescDb( const char *pszPath )
{
    if( pszRescPath )
        sqlite3_free( pszRescPath );
    if( pszPath )
        pszRescPath = sqlite3_mprintf( "%s", pszPath );
    else
        pszRescPath = NULL;
    return 0;
}

int
WfipsData::WriteRescDb( const char *pszPath, int *panIds, int nCount )
{
    int i, n, rc;
    sqlite3 *brdb;
    sqlite3 *rdb;
    sqlite3_stmt *stmt;
    char *pszSchema;
    char szRescId[128];
    char *pszRescSet;

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
        return rc;
    }
    pszSchema = sqlite3_mprintf( "%s", (char*)sqlite3_column_text( stmt, 0 ) );
    sqlite3_finalize( stmt );
    rc = sqlite3_exec( rdb, pszSchema, NULL, NULL, NULL );
    assert( rc == SQLITE_OK );
    sqlite3_free( pszSchema );

    pszRescSet = sqlite3_mprintf( "%d", panIds[0] );
    for( i = 1; i < nCount; i++ )
    {
        sprintf( szRescId, "%d", panIds[i] );
        n = strlen( szRescId ) + 1;
        pszRescSet = (char*)sqlite3_realloc( pszRescSet, strlen( pszRescSet ) + n + 1 );
        sprintf( pszRescSet, "%s,%d", pszRescSet, panIds[i] );
    }

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

    pszSql = sqlite3_mprintf( "INSERT INTO resource SELECT * FROM " \
                              "baseresc.resource WHERE ROWID " \
                              "IN (%s)", pszRescSet );
    sqlite3_free( pszRescSet );
    rc = sqlite3_exec( rdb, pszSql, NULL, NULL, NULL );
    sqlite3_free( pszSql );
    sqlite3_exec( rdb, "DETACH baseresc", NULL, NULL, NULL );
    if( bUseExtResc )
    {
        sqlite3_close( brdb );
    }
    sqlite3_close( rdb );
    return rc;
}

