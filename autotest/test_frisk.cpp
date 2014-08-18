/******************************************************************************
 *
 * $Id$ 
 *
 * Project:  WindNinja
 * Purpose:  Test fig
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

#include "frisk.h"
#include "cpl_conv.h"
#include "cpl_vsi.h"

#define _FRISK_TEST_EPSILON 0.0001

/*
** FRisk Tests
** Tests: frisk/
*/

struct FRiskTestData
{
    FRiskTestData()
    {
        pszInputPath = CPLGetConfigOption( "OMFFR_DATA", NULL );
        BOOST_REQUIRE( pszInputPath );
        pszInputBinary = CPLFormFilename( pszInputPath, "narr/narr_32km_frisk",
                                          ".flt" );
        pszInputAscii = CPLFormFilename( pszInputPath, "frisk/firerisk",
                                         ".txt" );
        fin.open( pszInputAscii );
        BOOST_REQUIRE( !fin.fail() );
        fin.close();
        fin.open( pszInputBinary, ios::binary );
        BOOST_REQUIRE( !fin.fail() );
        frisk = new FRisk();
        pszTmpFile = CPLGenerateTempFilename( NULL );
    }
    ~FRiskTestData()
    {
        delete frisk;
        fin.close();
        if( CPLCheckForFile( (char*)pszTmpFile, NULL ) )
        {
            VSIUnlink( (char*)pszTmpFile );
        }
    }
    const char *pszInputAscii;
    const char *pszInputBinary;
    const char *pszInputPath;
    const char *pszTmpFile;
    FRisk *frisk;
    ifstream fin;
};

BOOST_FIXTURE_TEST_SUITE( frisk, FRiskTestData )

/* assert() kills test in debug mode */
BOOST_AUTO_TEST_CASE_EXPECTED_FAILURES( julian_1, 1 )

BOOST_AUTO_TEST_CASE( wind_prob_1 )
{
    int nPixel = 225;
    int nLine = 200;
    long offset = nPixel + nLine * 349;
    fin.seekg(offset * FRISK_SIZE, ios::beg);
    frisk->ReadBinary( fin );
    BOOST_CHECK_EQUAL( frisk->test_wind_prob(), 0 );
}

BOOST_AUTO_TEST_CASE( ascii_read_1 )
{
    BOOST_REQUIRE_EQUAL( frisk->Read( pszInputAscii, FRisk::ASCII ), 0 );
    BOOST_CHECK_CLOSE( frisk->GetErcAvg( 1 ), 27.5, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetErcStd( 1 ), 8.6, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetErcCur( 1 ), 31, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetErcAvg( 2 ), 23.7, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetErcStd( 2 ), 12.9, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetErcCur( 2 ), 42, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetErcAvg( 365 ), 24.0, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetErcStd( 365 ), 9.7, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetErcCur( 365 ), -1, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetErcPerc( 1 ), 8.0, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetOneHour( 1 ), 34.78, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetTenHour( 1 ), 35.04, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetHundredHour( 1 ), 21.16, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetThousandHour( 1 ), 19.13, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetHerb( 1 ), 35.90, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetWoody( 1 ), 80.00, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetX_Thousand( 1 ), 19.13, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetErcPerc( 100 ), 95.0, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetOneHour( 100 ), 2.57, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetTenHour( 100 ), 3.26, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetHundredHour( 100 ), 3.79, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetThousandHour( 100 ), 5.68, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetHerb( 100 ), 2.57, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetWoody( 100 ), 60.00, _FRISK_TEST_EPSILON );
    BOOST_CHECK_CLOSE( frisk->GetX_Thousand( 100 ), 5.68, _FRISK_TEST_EPSILON );
}

BOOST_AUTO_TEST_CASE( ascii_read_2 )
{
    BOOST_REQUIRE_EQUAL( frisk->Read( pszInputAscii, FRisk::ASCII ), 0 );
    double s, d;
    BOOST_CHECK_EQUAL( frisk->GetRandomWind( 100, &s, &d ), 0 );
    BOOST_CHECK( s >= 0.0 && d >= 0.0 && d <= 360.0);
}
BOOST_AUTO_TEST_CASE( equal_op )
{
    FRisk oOther;
    BOOST_REQUIRE_EQUAL( frisk->Read( pszInputAscii, FRisk::ASCII ), 0 );
    BOOST_REQUIRE_EQUAL( oOther.Read( pszInputAscii, FRisk::ASCII ), 0 );
    BOOST_CHECK( *frisk == oOther );
    BOOST_CHECK( !( *frisk != oOther ) );
}

BOOST_AUTO_TEST_CASE( ascii_write )
{
    FRisk oOther;
    BOOST_REQUIRE_EQUAL( frisk->Read( pszInputAscii, FRisk::ASCII ), 0 );
    frisk->Dump( pszTmpFile, FRisk::ASCII );
    BOOST_REQUIRE_EQUAL( oOther.Read( pszTmpFile, FRisk::ASCII ), 0 );
    BOOST_CHECK( *frisk == oOther );
}

BOOST_AUTO_TEST_CASE( binary_write )
{
    FRisk oOther;
    ifstream fin2;
    BOOST_REQUIRE_EQUAL( frisk->Read( pszInputAscii, FRisk::ASCII ), 0 );
    frisk->Dump( pszTmpFile, FRisk::BINARY );
    fin2.open( pszTmpFile, ios::binary );
    BOOST_REQUIRE_EQUAL( oOther.ReadBinary( fin2 ), 0 );
    BOOST_CHECK( *frisk == oOther );
}

BOOST_AUTO_TEST_CASE( julian_1 )
{
    BOOST_CHECK_EQUAL( frisk->test_julian(), 0 );
}

BOOST_AUTO_TEST_CASE( memcheck_1 )
{
    int nPixel = 225;
    int nLine = 200;
    long offset = nPixel + nLine * 349;
    fin.seekg(offset * FRISK_SIZE, ios::beg);
    for(int i = 0;i < 100;i++)
    {
        BOOST_CHECK_EQUAL( frisk->ReadBinary( fin ), 0 );
    }
}

BOOST_AUTO_TEST_SUITE_END() /* frisk */

