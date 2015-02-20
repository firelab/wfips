/******************************************************************************
 *
 * Project:  Wildland Fire Investment Planning System
 * Purpose:  Test IRS Data management
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

#include <boost/test/unit_test.hpp>

#include "wfips_data.h"

struct WfipsDataMock
{
    WfipsDataMock()
    {
    }
    ~WfipsDataMock()
    {
    }
};

BOOST_FIXTURE_TEST_SUITE( wfipsdata, WfipsDataMock )

BOOST_AUTO_TEST_CASE( create_1 )
{
    WfipsData *poData = new WfipsData();
    BOOST_CHECK( poData->Valid() == 0 );
    delete poData;
}

BOOST_AUTO_TEST_CASE( create_2 )
{
    WfipsData *poData = new WfipsData("/home/kyle/src/wfips/build");
    int rc = poData->Open();
    BOOST_CHECK( poData->Valid() == 1 );
    BOOST_CHECK( rc == 0 );
    rc = poData->Close();
    BOOST_CHECK( rc == 0 );
    poData->Close();
    delete poData;
}

BOOST_AUTO_TEST_CASE( sql_1 )
{
    WfipsData *poData = new WfipsData("/home/kyle/src/wfips/build");
    int rc = poData->Open();
    BOOST_CHECK( poData->Valid() == 1 );
    BOOST_CHECK( rc == 0 );
    rc = poData->ExecuteSql( "SELECT COUNT(*) FROM resource" );
    BOOST_CHECK( rc == 0 );
    rc = poData->Close();
    BOOST_CHECK( rc == 0 );
    poData->Close();
    delete poData;
}

BOOST_AUTO_TEST_CASE( resc_copy_1 )
{
    WfipsData *poData = new WfipsData("/home/kyle/src/wfips/build");
    int rc = poData->Open();
    BOOST_CHECK( poData->Valid() == 1 );
    BOOST_CHECK( rc == 0 );
    int anRescIds[10] = {1,2,3,4,5,6,7,8,9,10};
    rc = poData->WriteRescDb( "resc_copy_1.db", anRescIds, NULL, 10 );
    BOOST_CHECK( rc == 0 );
    poData->Close();
    delete poData;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    rc = sqlite3_open_v2( "resc_copy_1.db", &db, SQLITE_OPEN_READONLY, NULL );
    BOOST_CHECK( rc == SQLITE_OK );
    rc = sqlite3_prepare_v2( db, "SELECT COUNT() FROM resource", -1, &stmt, NULL );
    BOOST_CHECK( rc == SQLITE_OK );
    rc = sqlite3_step( stmt );
    BOOST_CHECK( rc == SQLITE_ROW );
    rc = sqlite3_column_int( stmt, 0 );
    BOOST_CHECK( rc == 10 );
    remove( "resc_copy_1.db" );
    sqlite3_finalize( stmt );
    sqlite3_close( db );
}

BOOST_AUTO_TEST_CASE( resc_copy_2 )
{
    WfipsData *poData = new WfipsData("/home/kyle/src/wfips/build");
    int rc = poData->Open();
    BOOST_CHECK( poData->Valid() == 1 );
    BOOST_CHECK( rc == 0 );
    int anRescIds[10] = {1,2,3,4,5,6,7,8,9,10};
    rc = poData->WriteRescDb( "resc_copy_2.db", anRescIds, NULL, 10 );
    BOOST_CHECK( rc == 0 );

    poData->SetRescDb( "resc_copy_2.db" );
    int anRescIds2[2] = {1,2};
    rc = poData->WriteRescDb( "resc_copy_2_.db", anRescIds2, NULL, 2 );
    poData->Close();
    delete poData;

    sqlite3 *db;
    sqlite3_stmt *stmt;
    rc = sqlite3_open_v2( "resc_copy_2_.db", &db, SQLITE_OPEN_READONLY, NULL );
    BOOST_CHECK( rc == SQLITE_OK );
    rc = sqlite3_prepare_v2( db, "SELECT COUNT() FROM resource", -1, &stmt, NULL );
    BOOST_CHECK( rc == SQLITE_OK );
    rc = sqlite3_step( stmt );
    BOOST_CHECK( rc == SQLITE_ROW );
    rc = sqlite3_column_int( stmt, 0 );
    BOOST_CHECK( rc == 2 );
    remove( "resc_copy_2.db" );
    remove( "resc_copy_2_.db" );
    sqlite3_finalize( stmt );
    sqlite3_close( db );
}

BOOST_AUTO_TEST_CASE( resc_copy_3 )
{
    WfipsData *poData = new WfipsData("/home/kyle/src/wfips/build");
    int rc = poData->Open();
    BOOST_CHECK( poData->Valid() == 1 );
    BOOST_CHECK( rc == 0 );
    int anRescIds[10] = {1,2,3,4,5,6,7,8,9,10};
    int anDispLocIds[10] = {1,1,1,1,1,1,1,1,1,1};
    rc = poData->WriteRescDb( "resc_copy_3.db", anRescIds, anDispLocIds, 10 );
    BOOST_CHECK( rc == 0 );
    poData->Close();
    delete poData;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    rc = sqlite3_open_v2( "resc_copy_3.db", &db, SQLITE_OPEN_READONLY, NULL );
    BOOST_CHECK( rc == SQLITE_OK );
    rc = sqlite3_prepare_v2( db, "SELECT COUNT(DISTINCT disploc) FROM resource", -1, &stmt, NULL );
    BOOST_CHECK( rc == SQLITE_OK );
    rc = sqlite3_step( stmt );
    BOOST_CHECK( rc == SQLITE_ROW );
    rc = sqlite3_column_int( stmt, 0 );
    BOOST_CHECK( rc == 1 );
    remove( "resc_copy_3.db" );
    sqlite3_finalize( stmt );
    sqlite3_close( db );
}

BOOST_AUTO_TEST_CASE( assoc_disploc_1 )
{
    WfipsData *poData = new WfipsData("/home/kyle/src/wfips/build");
    int rc = poData->Open();
    BOOST_CHECK( poData->Valid() == 1 );
    BOOST_CHECK( rc == 0 );
    const char *pszWkt = "POLYGON((-114 47, -113 47, -113 46, -114 46, -114 47))";
    int *panIds, nCount;
    poData->GetAssociatedDispLoc( pszWkt, &panIds, &nCount );
    poData->Close();
    delete poData;
    rc = panIds[0];
    WfipsData::Free( (void*)panIds );
    BOOST_REQUIRE( nCount > 0 );
    BOOST_CHECK( rc > 0 );
}

BOOST_AUTO_TEST_SUITE_END() /* irs */

