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

#include "wfips_sqlite.h"

int WfipsAttachDb( sqlite3 *db, const char *pszPath, const char *pszName )
{
    char *pszSql;
    int rc;
    if( db == NULL || pszPath == NULL || pszName == NULL )
    {
        return SQLITE_ERROR;
    }
    pszSql = sqlite3_mprintf( "ATTACH %Q AS %s", pszPath, pszName );
    rc = sqlite3_exec( db, pszSql, NULL, NULL, NULL );
    sqlite3_free( pszSql );
    return rc;
}

double WfipsRandom()
{
    double d;
    int n;
    sqlite3_randomness( sizeof( int ), &n );
    d = (double)abs(n)/(double)INT_MAX;
    assert( d >= 0.0  && d <= 1.0 );
    return d;
}

/**
 ** Compile a well-known text geometry into one we can use with spatialite.
 **
 ** \note if db is null, an in memory one is used
 */
int WfipsCompileGeometry( sqlite3 *db, const char *pszWkt, void **pCompiled )
{
    const void *p;
    int n, rc;
    sqlite3 *db2 = NULL;
    sqlite3_stmt *stmt;
    if( !db )
    {
        rc = sqlite3_open_v2( ":memory:", &db2,
                              SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE,
                              NULL );
        rc = sqlite3_enable_load_extension( db2, 1 );
        rc = sqlite3_load_extension( db2, SPATIALITE_EXT, NULL, NULL );
        db = db2;
    }
    rc = sqlite3_prepare_v2( db, "SELECT GeomFromText(?)", -1, &stmt, NULL );
    if( rc != SQLITE_OK )
    {
        sqlite3_finalize( stmt );
        sqlite3_close( db2 );
        return 0;
    }
    rc = sqlite3_bind_text( stmt, 1, pszWkt, -1, NULL );
    if( rc != SQLITE_OK )
    {
        sqlite3_finalize( stmt );
        sqlite3_close( db2 );
        return 0;
    }
    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW )
    {
        sqlite3_finalize( stmt );
        sqlite3_close( db2 );
        return 0;
    }
    n = sqlite3_column_bytes( stmt, 0 );
    p = sqlite3_column_blob( stmt, 0 );
    *pCompiled = sqlite3_malloc( n );
    memcpy( *pCompiled, p, n );
    sqlite3_finalize( stmt );
    sqlite3_close( db2 );
    return n;
}

int WfipsHasTable( sqlite3 *db, const char *pszName )
{
    if( db == NULL || pszName == NULL )
    {
        return 0;
    }
    sqlite3_stmt *stmt;
    int rc;
    rc = sqlite3_prepare_v2( db, "SELECT COUNT() FROM sqlite_master WHERE "
                                 "type='table' AND name=?",
                             -1, &stmt, NULL );
    if( rc != SQLITE_OK )
    {
        sqlite3_finalize( stmt );
        return 0;
    }
    rc = sqlite3_bind_text( stmt, 1, pszName, -1, NULL );
    if(rc != SQLITE_OK )
    {
        sqlite3_finalize( stmt );
        return 0;
    }
    rc = sqlite3_step( stmt );
    if( rc == SQLITE_ROW && sqlite3_column_int( stmt, 0 ) > 0 )
        rc = 1;
    else
        rc = 0;
    sqlite3_finalize( stmt );
    return rc;
}

