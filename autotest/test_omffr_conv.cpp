/******************************************************************************
 *
 * $Id$ 
 *
 * Project:  WindNinja
 * Purpose:  Test common source and utilities
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

#include "omffr_conv.h"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( conv )

/* Fails, but shouldn't */
BOOST_AUTO_TEST_CASE_EXPECTED_FAILURES( path_1, 1 )

BOOST_AUTO_TEST_CASE( path_1 )
{
    const char *pszFullPath;
    const char *pszRealPath;
    pszFullPath = GetFullPath( "CMakeCache.txt" );
#ifdef WIN32
    pszRealPath = "c:/src/omffr/build_x86/autotest/CMakeCache.txt";
#else
    pszRealPath = "/home/kyle/src/omffr/build/CMakeCache.txt";
#endif
    int rc = strcmp( pszFullPath, pszRealPath );
    printf("rc = %d\n", rc);
    BOOST_CHECK_EQUAL( rc, 0 );
    OmffrFree( (void*)pszFullPath );
}

BOOST_AUTO_TEST_CASE( join_1 )
{
    int rc;
    const char *papszIn [] = { "My", "name", "is", "Kyle", NULL };
    const char *pszOut;
    pszOut = JoinStringList( papszIn, " " );
    rc = strcmp( pszOut, "My name is Kyle" );
    BOOST_CHECK_EQUAL( rc, 0 );
    OmffrFree( (void*)pszOut );
}

BOOST_AUTO_TEST_CASE( join_2 )
{
    int rc;
    const char *papszIn [] = { "My", "name", "is", "Kyle", NULL };
    const char *pszOut;
    pszOut = JoinStringList( papszIn, NULL );
    rc = strcmp( pszOut, "My,name,is,Kyle" );
    BOOST_CHECK_EQUAL( rc, 0 );
    OmffrFree( (void*)pszOut );
}

BOOST_AUTO_TEST_CASE( join_3 )
{
    int rc;
    const char *papszIn [] = { "My", "name", "is", "Kyle", NULL };
    const char *pszOut;
    pszOut = JoinStringList( papszIn, "" );
    rc = strcmp( pszOut, "MynameisKyle" );
    BOOST_CHECK_EQUAL( rc, 0 );
    OmffrFree( (void*)pszOut );
}

BOOST_AUTO_TEST_CASE( join_4 )
{
    int rc;
    const char *papszIn [] = { "Mars", NULL };
    const char *pszOut;
    pszOut = JoinStringList( papszIn, " " );
    rc = strcmp( pszOut, "Mars" );
    BOOST_CHECK_EQUAL( rc, 0 );
    OmffrFree( (void*)pszOut );
}

BOOST_AUTO_TEST_CASE( join_5 )
{
    const char *pszOut;
    pszOut = JoinStringList( NULL, "," );
    BOOST_CHECK( pszOut == NULL );
    OmffrFree( (void*)pszOut );
}

BOOST_AUTO_TEST_CASE( count_1 )
{
    int rc;
    rc = CountStrings( NULL );
    BOOST_CHECK_EQUAL( rc, 0 );
}

BOOST_AUTO_TEST_CASE( count_2 )
{
    int rc;
    const char * papszIn[] = { "Mars", "Kyle", NULL };
    rc = CountStrings( papszIn );
    BOOST_CHECK_EQUAL( rc, 2 );
}

BOOST_AUTO_TEST_CASE( count_3 )
{
    int rc;
    const char * papszIn[] = { NULL };
    rc = CountStrings( papszIn );
    BOOST_CHECK_EQUAL( rc, 0 );
}

BOOST_AUTO_TEST_CASE( count_4 )
{
    int rc;
    char **papszIn = (char**)OmffrMalloc( 3 * sizeof( char * ) );
    papszIn[0] = strdup( "KYLE" );
    papszIn[1] = strdup( "" );
    papszIn[2] = NULL;
    rc = CountStrings( papszIn );
    BOOST_CHECK_EQUAL( rc, 2 );
    OmffrFree( papszIn[0] );
    OmffrFree( papszIn[1] );
    OmffrFree( papszIn );
}

BOOST_AUTO_TEST_CASE( add_string_1 )
{
    int rc;
    char **papszIn = NULL;
    papszIn = AddString( papszIn, "KYLE" );
    BOOST_CHECK_EQUAL( CountStrings( papszIn ), 1 );
    rc = strcmp( papszIn[0], "KYLE" );
    BOOST_CHECK_EQUAL( rc, 0 );
    FreeStringList( papszIn );
}

BOOST_AUTO_TEST_CASE( add_string_2 )
{
    int rc;
    char **papszIn = NULL;
    papszIn = AddString( papszIn, "KYLE" );
    papszIn = AddString( papszIn, NULL );
    BOOST_CHECK_EQUAL( CountStrings( papszIn ), 1 );
    rc = strcmp( papszIn[0], "KYLE" );
    BOOST_CHECK_EQUAL( rc, 0 );
    FreeStringList( papszIn );
}

BOOST_AUTO_TEST_SUITE_END() /* omffr_tree */

