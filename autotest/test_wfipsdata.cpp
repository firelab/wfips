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
        poData = new WfipsData("/home/kyle/src/wfips/build");
        poData->Open();
    }
    ~WfipsDataMock()
    {
        poData->Close();
        delete poData;
    }
    WfipsData *poData;
};

BOOST_FIXTURE_TEST_SUITE( wfipsdata, WfipsDataMock )

BOOST_AUTO_TEST_CASE( create_1 )
{
    WfipsData *poData2 = new WfipsData();
    BOOST_CHECK( poData2->Valid() == 0 );
    delete poData2;
}

BOOST_AUTO_TEST_CASE( create_2 )
{
    WfipsData *poData2 = new WfipsData("/home/kyle/src/wfips/build");
    int rc = poData2->Open();
    BOOST_CHECK( poData2->Valid() == 1 );
    BOOST_CHECK( rc == 0 );
    rc = poData2->Close();
    BOOST_CHECK( rc == 0 );
    delete poData2;
}

BOOST_AUTO_TEST_CASE( sql_1 )
{
    int rc = poData->ExecuteSql( "SELECT COUNT(*) FROM resource" );
    BOOST_CHECK( rc == 0 );
}

BOOST_AUTO_TEST_CASE( resc_copy_1 )
{
    int anRescIds[10] = {1,2,3,4,5,6,7,8,9,10};
    int rc = poData->WriteRescDb( "resc_copy_1.db", anRescIds, NULL, 10 );
    BOOST_CHECK( rc == 0 );
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
    sqlite3_finalize( stmt );
    sqlite3_close( db );
    remove( "resc_copy_1.db" );
}

BOOST_AUTO_TEST_CASE( resc_copy_2 )
{
    int anRescIds[10] = {1,2,3,4,5,6,7,8,9,10};
    int rc = poData->WriteRescDb( "resc_copy_2.db", anRescIds, NULL, 10 );
    BOOST_CHECK( rc == 0 );

    poData->SetRescDb( "resc_copy_2.db" );
    int anRescIds2[2] = {1,2};
    rc = poData->WriteRescDb( "resc_copy_2_.db", anRescIds2, NULL, 2 );

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
    sqlite3_finalize( stmt );
    sqlite3_close( db );
    remove( "resc_copy_2.db" );
    remove( "resc_copy_2_.db" );
}

BOOST_AUTO_TEST_CASE( resc_copy_3 )
{
    int anRescIds[10] = {1,2,3,4,5,6,7,8,9,10};
    int anDispLocIds[10] = {1,1,1,1,1,1,1,1,1,1};
    int rc = poData->WriteRescDb( "resc_copy_3.db", anRescIds, anDispLocIds, 10 );
    BOOST_CHECK( rc == 0 );

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
    sqlite3_finalize( stmt );
    sqlite3_close( db );
    remove( "resc_copy_3.db" );
}

BOOST_AUTO_TEST_CASE( assoc_disploc_1 )
{
    const char *pszWkt = "POLYGON((-114 47, -113 47, -113 46, -114 46, -114 47))";
    int *panIds, nCount;
    poData->GetAssociatedDispLoc( pszWkt, &panIds, &nCount );
    int rc = panIds[0];
    WfipsData::Free( (void*)panIds );
    BOOST_REQUIRE( nCount > 0 );
    BOOST_CHECK( rc > 0 );
}

BOOST_AUTO_TEST_CASE( build_set_1 )
{
    BOOST_CHECK_EQUAL( poData->TestBuildAgencySet1(), 0 );
}

BOOST_AUTO_TEST_CASE( build_set_2 )
{
    BOOST_CHECK_EQUAL( poData->TestBuildAgencySet2(), 0 );
}

BOOST_AUTO_TEST_CASE( build_set_3 )
{
    BOOST_CHECK_EQUAL( poData->TestBuildAgencySet3(), 0 );
}

BOOST_AUTO_TEST_CASE( build_set_4 )
{
    BOOST_CHECK_EQUAL( poData->TestBuildAgencySet4(), 0 );
}

BOOST_AUTO_TEST_CASE( build_set_5 )
{
    BOOST_CHECK_EQUAL( poData->TestBuildAgencySet5(), 0 );
}

BOOST_AUTO_TEST_CASE( build_fid_set_1 )
{
    BOOST_CHECK_EQUAL( poData->TestFidSet1(), 0 );
}

BOOST_AUTO_TEST_CASE( assoc_resource_1 )
{
    int anIds[] = {1,2,3,4,5,6,7,8,9,10};
    WfipsResc *psResc;
    int nCount;
    int rc = poData->GetAssociatedResources( anIds, 10, &psResc, &nCount, AGENCY_ALL );
    WfipsData::FreeAssociatedResources( psResc, nCount );
    BOOST_REQUIRE( nCount > 0 );
    BOOST_CHECK( rc == 0 );
}

BOOST_AUTO_TEST_CASE( assoc_resource_2 )
{
    int anIds[] = {1,2,3,4,5,6,7,8,9,10};
    WfipsResc *psResc;
    int nCount;
    int rc = poData->GetAssociatedResources( anIds, 10, &psResc, &nCount, USFS );
    WfipsData::FreeAssociatedResources( psResc, nCount );
    BOOST_REQUIRE( nCount == 0 );
    BOOST_CHECK( rc == 0 );
}

BOOST_AUTO_TEST_CASE( assoc_resource_3 )
{
    int anIds[] = {1,2,3,4,5,6,7,8,9,10};
    WfipsResc *psResc;
    int nCount;
    int rc = poData->GetAssociatedResources( anIds, 10, &psResc, &nCount, DOI_BLM );
    WfipsData::FreeAssociatedResources( psResc, nCount );
    BOOST_REQUIRE( nCount > 0 );
    BOOST_CHECK( rc == 0 );
}

BOOST_AUTO_TEST_CASE( load_scen_1 )
{
    int rc;
    rc = poData->LoadScenario( 5, NULL, NULL, 1.0, 0 );
    BOOST_CHECK( rc > 0 );
    return;
}

BOOST_AUTO_TEST_CASE( load_scen_2 )
{
    int rc;
    rc = poData->LoadScenario( 1, NULL, NULL, 1.0, 0 );
    BOOST_CHECK( rc == 0 );
    return;
}

BOOST_AUTO_TEST_CASE( load_scen_3 )
{
    int rc;
    const char *pszWkt = "POLYGON((-114 47, -113 47, -113 46, -114 46, -114 47))";
    rc = poData->LoadScenario( 5, pszWkt, NULL, 1.0, 0 );
    BOOST_CHECK( rc > 0 );
    return;
}

BOOST_AUTO_TEST_CASE( load_scen_4 )
{
    int a, b;
    const char *pszWkt = "POLYGON((-114 47, -113 47, -113 46, -114 46, -114 47))";
    a = poData->LoadScenario( 5, NULL, NULL, 1.0, 0 );
    b = poData->LoadScenario( 5, pszWkt, NULL, 1.0, 0 );
    BOOST_CHECK( a != 0 && b != 0 );
    BOOST_CHECK( a > b );
    return;
}

BOOST_AUTO_TEST_CASE( load_scen_5 )
{
    int a, b;
    a = poData->LoadScenario( 5, NULL, NULL, 1.0, AGENCY_ALL );
    b = poData->LoadScenario( 5, NULL, NULL, 1.0, DOI_BLM );
    BOOST_CHECK( a != 0 && b != 0 );
    BOOST_CHECK( a > b );
    return;
}

BOOST_AUTO_TEST_CASE( set_resource_db_1 )
{
    int anIds[] = {1,2,3,4,5,6,7,8,9,10};
    WfipsResc *psResc;
    int nCount;
    poData->SetRescDb( "/home/kyle/src/wfips/build/resc.db" );
    int rc = poData->GetAssociatedResources( anIds, 10, &psResc, &nCount, AGENCY_ALL );
    WfipsData::FreeAssociatedResources( psResc, nCount );
    BOOST_REQUIRE( nCount > 0 );
    BOOST_CHECK( rc == 0 );
}

BOOST_AUTO_TEST_CASE( scen_count_1 )
{
    int *panIndices;
    int rc;
    rc = poData->GetScenarioIndices( &panIndices );
    /* Will change */
    BOOST_CHECK( rc == 1 );
    BOOST_CHECK( panIndices[0] == 5 );
    WfipsData::Free( panIndices );
}

BOOST_AUTO_TEST_CASE( load_resc_types_1 )
{
    BOOST_CHECK( poData->TestProdLoad1() == 0 );
}

BOOST_AUTO_TEST_CASE( load_prod_rates_1 )
{
    BOOST_CHECK( poData->TestProdLoad1() == 0 );
}


BOOST_AUTO_TEST_SUITE_END() /* irs */

