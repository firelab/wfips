/******************************************************************************
 *
 * $Id$ 
 *
 * Project:  WindNinja
 * Purpose:  Test IRS Results class
 * Author:   Kyle Shannon <kyle@pobox.com>
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
#include "irs.h"

/*
** IRS Tests
** Tests: irs_results/
**                    single_resc_usage
*/

struct IrsResultTestData
{
    IrsResultTestData()
    {
        /* In-memory named, so we can check with sqlite */
        //poResults = new IRSResult( "file::memory:?cache=shared", "", 1, NULL );
        poResults = new IRSResult( "tmp.db", "", 0, 1, 1, NULL );
        BOOST_REQUIRE( poResults );
    }
    ~IrsResultTestData()
    {
        delete poResults;
        sqlite3_close( db );
        unlink( "tmp.db" );
    }
    const char *pszInputDb;
    IRSResult *poResults;
    sqlite3 *db;
    sqlite3_stmt *stmt;
};

BOOST_FIXTURE_TEST_SUITE( irs_result, IrsResultTestData )

BOOST_AUTO_TEST_CASE( single_resc_usage )
{
    std::map<std::string, double> oMap;
    int rc;
    oMap["kyle"] = 0.1;
    oMap["diane"] = 0.2;

    poResults->AddSingleResourceUsage( oMap );

    //rc = sqlite3_open_v2( "file::memory:?cache=shared", &db,
    //                      SQLITE_OPEN_READONLY, NULL );
    rc = sqlite3_open_v2( "tmp.db", &db,
                         SQLITE_OPEN_READONLY, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT * FROM resource_usage", -1, &stmt, 
                             NULL );
    const char *pszName;
    double dfValue;
    int bHaveData = 0;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        bHaveData = 1;
        pszName = (const char *)sqlite3_column_text( stmt, 0 );
        dfValue = sqlite3_column_double( stmt, 1 );

        BOOST_CHECK( oMap[std::string(pszName)] == dfValue );
    }
    BOOST_CHECK( bHaveData );
    sqlite3_reset( stmt );

    oMap["diane"] = 0.2;
    oMap["jason"] = 0.5;
    poResults->AddSingleResourceUsage( oMap );
    oMap["diane"] = 0.2 + 0.2;
    oMap["kyle"] = 0.1 + 0.1;
    bHaveData = 0;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        bHaveData = 1;
        pszName = (const char *)sqlite3_column_text( stmt, 0 );
        dfValue = sqlite3_column_double( stmt, 1 );

        BOOST_CHECK( oMap[std::string(pszName)] == dfValue );
    }
    BOOST_CHECK( bHaveData );
    sqlite3_reset( stmt );
    sqlite3_finalize( stmt );
}

BOOST_AUTO_TEST_CASE( remove_resc_1 )
{
    BOOST_CHECK( 1 );
}

BOOST_AUTO_TEST_SUITE_END() /* irs */

BOOST_AUTO_TEST_CASE( result_attributes_1 )
{
    /*
    ** Disable to reduce data size, fix later...
    */
    /*
    IRSResult *poAttResult;
    poAttResult = new IRSResult( "/home/kyle/src/omffr/trunk/autotest/data/att_1.db",
                                 "", 0, 1, 1, NULL );
    char **papszAtts = poAttResult->GetResultAttributes();
    BOOST_CHECK( papszAtts );
    int i = 0;
    while( papszAtts[i++] != NULL );
    BOOST_CHECK( i == 8 );
    delete poAttResult;
    */
}

