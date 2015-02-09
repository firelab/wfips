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
    delete poData;
}

BOOST_AUTO_TEST_SUITE_END() /* irs */

