/******************************************************************************
 *
 * Project:  Wildland Fire Investment Planning System
 * Purpose:  Graphical User Interface
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

#include "wfipsguiutil.h"

/*
** Helper to bounce between QStrings and C style strings.  Please free the
** results with free(), as the return value is owned by you, the caller.
*/
char * QStringToCString( const QString &s )
{
    int n = s.size() + 1;
    char *p = (char*)malloc( sizeof( char ) * n );
    strncpy( p, s.toLocal8Bit().data(), n );
    return p;
}

QStringList WfipsGetRescTypes( QString path )
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    char zDb[8192];
    *zDb = '\0';

    char *zDataPath = QStringToCString( path );
    sqlite3_snprintf( 8192, zDb, "%s/resc.db", zDataPath );
    free( zDataPath );
    rc = sqlite3_open_v2( zDb, &db, SQLITE_OPEN_READONLY, NULL );
    if( rc != SQLITE_OK )
    {
        qDebug() << "Failed to open resource db to extract types.";
        return QStringList();
    }
    rc = sqlite3_prepare_v2( db, "SELECT DISTINCT(resc_type) FROM resource " \
                                 "ORDER BY resc_type",
                             -1, &stmt, NULL );
    if( rc != SQLITE_OK )
    {
        qDebug() << "Failed to run query on resource db for types.";
        return QStringList();
    }
    QStringList types;

    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        types << (const char*)sqlite3_column_text( stmt, 0 );
    }
    sqlite3_finalize( stmt );
    sqlite3_close( db );
    return types;
}

