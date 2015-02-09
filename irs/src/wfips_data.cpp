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

WfipsData::WfipsData()
{
    Init();
}

WfipsData::WfipsData( const char *pszPath )
{
    Init();
    this->pszPath = sqlite3_mprintf( "%s", pszPath );
}

void WfipsData::Init()
{
    pszPath = NULL;
    pszRescPath = NULL;
    db = NULL;
    bValid = 0;
    iScrap = 0;
}

WfipsData::~WfipsData()
{
    sqlite3_free( pszPath );
    sqlite3_free( pszRescPath );
    sqlite3_close( db );
}

int WfipsData::Open()
{
    if( pszPath == NULL )
    {
        return 1;
    }
    return Open( pszPath );
}

char *WfipsData::GetScrapBuffer()
{
    iScrap++;
    if( iScrap >= 10 )
        iScrap = iScrap % 10;
    memset( szScrap[iScrap], '\0', MAX_PATH );
    return szScrap[iScrap];
}

const char * WfipsData::FormFileName( const char *pszPath, const char *pszDb )
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

char* WfipsData::BaseName( const char *pszPath )
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
int WfipsData::Attach( const char *pszPath )
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

#define IRS_CHECK_STATUS if(rc != SQLITE_OK) goto error

int WfipsData::Open( const char *pszPath )
{
    if( pszPath == NULL )
        return 1;
    int rc;
    int i;

    rc = sqlite3_open_v2( FormFileName( pszPath, COST_DB ), &db,
                          SQLITE_OPEN_READWRITE, NULL );
    IRS_CHECK_STATUS;
    i = 1;
    while( apszDbFiles[i] != NULL )
    {
        rc = Attach( FormFileName( pszPath, apszDbFiles[i++] ) );
        IRS_CHECK_STATUS;
    }
    bValid = 1;
    return rc;

error:
    Close();
    return rc;
}

int WfipsData::ExecuteSql( const char *pszSql )
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

int WfipsData::Close()
{
    int rc = SQLITE_OK;
    rc = sqlite3_close( db );
    db = NULL;
    bValid = 0;
    return rc;
}

