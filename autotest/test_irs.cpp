/******************************************************************************
 *
 * $Id$ 
 *
 * Project:  WindNinja
 * Purpose:  Test IRS
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
#include "omffr_conv.h"

#define _IRS_TEST_EPSILON 0.0000001

/*
** IRS Tests
** Tests: irs/
*/


struct IrsTestData
{
    IrsTestData()
    {
        pszInputDb = CPLGetConfigOption( "OMFFR_DATA", NULL );
        BOOST_REQUIRE( pszInputDb );
        pszInputDb = CPLFormFilename( pszInputDb, "omffr", ".sqlite" );
        poDA = IRSDataAccess::Create( 0, pszInputDb );
        BOOST_REQUIRE( poDA );
    }
    ~IrsTestData()
    {
        delete poDA;
    }
    const char *pszInputDb;
    IRSDataAccess *poDA;
    CRunScenario Runner;
};

BOOST_FIXTURE_TEST_SUITE( irs, IrsTestData )

BOOST_AUTO_TEST_CASE( avg_fires_1 )
{
    std::vector<int> fires;
    fires = poDA->GetAvgNumFires( "GB_ID_002" );
    BOOST_CHECK_EQUAL( fires.size(), 365 );
    /*
    ** 20131122 -> Visual inspection found 19 fires in GB_ID_002 and current
    ** fig has 26 years.
    */
    int j = 19 / 26;
    for( int i = 0; i < 365; i++ )
    {
        BOOST_CHECK( fires[i] >= 0 );
        BOOST_CHECK( fires[99] == j );
    }
}

BOOST_AUTO_TEST_CASE( db_fpu_query )
{
    std::string expected;
    expected = poDA->PointQuery( "fpu_bndry", "fpu_code", -115.67, 43.61 );
    BOOST_CHECK_EQUAL( expected, "GB_ID_002" );
}

BOOST_AUTO_TEST_CASE( smooth_1 )
{
    std::vector<int> raw;
    for( int i = 0; i < 365; i++ )
    {
        raw.push_back( i );
    }
    std::vector<int>smooth = poDA->SmoothNumFires( raw, 3 );
    BOOST_CHECK_EQUAL( smooth[0], 1 );
    BOOST_CHECK_EQUAL( smooth[1], 2 );
    BOOST_CHECK_EQUAL( smooth[364], 364 );
}

BOOST_AUTO_TEST_CASE( smooth_2 )
{
    std::vector<int> raw;
    for( int i = 0; i < 365; i++ )
        raw.push_back( i * 2 );
    std::vector<int>smooth = poDA->SmoothNumFires( raw, 3 );
    BOOST_CHECK_EQUAL( smooth[0], 2 );
    BOOST_CHECK_EQUAL( smooth[1], 4 );
    BOOST_CHECK_EQUAL( smooth[364], 364 * 2 );
}

BOOST_AUTO_TEST_CASE( smooth_3 )
{
    std::vector<int> raw;
    for( int i = 0; i < 365; i++ )
        raw.push_back( i * 2 );
    std::vector<int>smooth = poDA->SmoothNumFires( raw, 3 );
    BOOST_CHECK_EQUAL( smooth.size(), 365 );
}

BOOST_AUTO_TEST_CASE( scenario_order )
{
   poDA->SetSpatialFilter( "POLYGON(( -116.390 44.214, -115.836 44.214, -115.836 43.668, -116.390 43.668, -116.390 44.214 ))" );
   std::vector<int>years = poDA->GetYearIndexes();

   poDA->LoadRescType( Runner.m_VRescType, NULL );
   poDA->LoadProdRates( Runner.m_VProdRates, NULL );
   poDA->LoadDispatchLogic( Runner.m_VDispLogic, NULL, NULL );
   poDA->LoadFwa( Runner.m_VDispLogic, Runner.m_VFWA, NULL, NULL );
   poDA->LoadDispatchLocation( Runner.m_VFWA, Runner.m_VDispLoc, NULL );
   poDA->LoadResource( &Runner, Runner.m_VRescType, Runner.m_VDispLoc, Runner.m_VResource,
                       NULL, NULL, NULL, NULL, NULL );
   Runner.m_NumDispLogic = Runner.m_VDispLogic.size();
   Runner.m_NumFWA = Runner.m_VFWA.size();
   Runner.m_NumRescType = Runner.m_VRescType.size();
   Runner.m_NumDispLoc = Runner.m_VDispLoc.size();
   Runner.m_NumProdRates = Runner.m_VProdRates.size();
   Runner.m_NumResource = Runner.m_VResource.size();

   poDA->LoadScenario( Runner.m_VFWA, Runner.m_VFire, years[0], 0.0, NULL );
   Runner.m_NumFire = Runner.m_VFire.size();

   int j, t;
   j = 0;
   t = -1;
   for( int i = 0; i < Runner.m_VFire.size(); i++ )
   {
       BOOST_CHECK( j <= Runner.m_VFire[i].GetJulianDay() );
       if( j != Runner.m_VFire[i].GetJulianDay() )
           t = -1;
       j = Runner.m_VFire[i].GetJulianDay();
       BOOST_CHECK( t <= atoi( Runner.m_VFire[i].GetDiscoveryTime().c_str() ) );
       t = atoi( Runner.m_VFire[i].GetDiscoveryTime().c_str() );
   }
}

BOOST_AUTO_TEST_CASE( resource_dist_1 )
{
    double dfDist;
    BOOST_CHECK_EQUAL( poDA->ResourceDist( "GB", "EN34", "Shoshone", "BLM",
                                           &dfDist ) , 0 );
    /* We should get 4 / 119 ~ 0.03361344537815126 */
    BOOST_CHECK_CLOSE( dfDist, 0.03361344537815126, _IRS_TEST_EPSILON );
    BOOST_CHECK_EQUAL( poDA->ResourceDist( "GB", "EN34", "Shoshone",
                                            NULL, &dfDist ), 0 );
    /* We should get 4 / 187 ~ 0.0213903743315508 */
    BOOST_CHECK_CLOSE( dfDist, 0.0213903743315508, _IRS_TEST_EPSILON );
}

BOOST_AUTO_TEST_CASE( point_query_1 )
{
    std::string value;
    value = poDA->PointQuery( "fpu_bndry", "fpu_code", -115.67, 43.61 );
    BOOST_CHECK( value == "GB_ID_002" );
    value = poDA->PointQuery( "gacc_bndry", "ga_abbr", -115.67, 43.61 );
    BOOST_CHECK( value == "GB" );
}

BOOST_AUTO_TEST_CASE( load_resc_type )
{
    std::set<std::string>names;
    names.insert( "ATT" );
    names.insert( "SEAT" );
    names.insert( "SCP" );
    names.insert( "FBDZ" );
    names.insert( "FRBT" );
    names.insert( "CRW" );
    names.insert( "HELI" );
    names.insert( "HEL1" );
    names.insert( "HEL2" );
    names.insert( "HEL3" );
    names.insert( "SMJR" );
    names.insert( "SJAC" );
    names.insert( "DZR1" );
    names.insert( "DZR2" );
    names.insert( "DZR3" );
    names.insert( "EN12" );
    names.insert( "EN34" );
    names.insert( "EN56" );
    names.insert( "EN70" );
    names.insert( "TP12" );
    names.insert( "TP34" );
    names.insert( "TP56" );
    names.insert( "WT" );
    names.insert( "CRW20" );
    std::vector<CRescType>types;
    poDA->LoadRescType( types, NULL );
    BOOST_REQUIRE( types.size() == names.size() );
    int bFound;
    for( int i = 0; i < types.size(); i++ )
    {
        BOOST_CHECK( names.count( types[i].GetRescType() ) > 0 );
    }
}

BOOST_AUTO_TEST_CASE( general_resc_1 )
{
   sqlite3 *db;
   poDA->CreateGeneralResourceDb(":memory:", &db);
   sqlite3_stmt *stmt;
   sqlite3_prepare_v2(db, "SELECT * FROM general_resc WHERE gacc='NR' AND "
                          "type='HANDCRW'", -1, &stmt, NULL);
   int rc = 0;
   BOOST_CHECK( sqlite3_step( stmt ) == SQLITE_ROW );
   BOOST_CHECK( EQUAL( (const char*)sqlite3_column_text( stmt, 0 ), "NR" ) );
   BOOST_CHECK( EQUAL( (const char*)sqlite3_column_text( stmt, 1 ), "HANDCRW" ) );
   BOOST_CHECK_EQUAL( sqlite3_column_int( stmt, 2 ), 31 );
   BOOST_CHECK_CLOSE( sqlite3_column_double( stmt, 3 ), 72.48, 0.0000000001 );
   BOOST_CHECK_EQUAL( sqlite3_column_int( stmt, 4 ), 192 );
   sqlite3_finalize(stmt);
   stmt = NULL;
   sqlite3_close(db);
}

BOOST_AUTO_TEST_CASE( load_airtankers )
{
    poDA->SetSpatialFilter( "POLYGON((-120.74 48.33, -112.52 48.33, -112.52 43.06, -120.74 43.06, -120.74 48.33))" );
    poDA->LoadRescType( Runner.m_VRescType, NULL );
    poDA->LoadProdRates( Runner.m_VProdRates, NULL );
    poDA->LoadDispatchLogic( Runner.m_VDispLogic, NULL, NULL );
    poDA->LoadFwa( Runner.m_VDispLogic, Runner.m_VFWA, NULL, NULL );
    poDA->LoadTankerBases( &Runner, Runner.m_VFWA, Runner.m_VDispLoc, -1, NULL );
    std::set<std::string>oNames;
    oNames.insert( "Delta_Junction_KYLE" );
    oNames.insert( "Fort_Wainwright_(FBK)_KYLE" );
    oNames.insert( "Kenai_(ENA)_KYLE" );
    oNames.insert( "McGrath_(MCG)_KYLE" );
    oNames.insert( "Palmer_(PAQ)_KYLE" );
    oNames.insert( "Tanacross_(TSG)_KYLE" );
    oNames.insert( "Bishop_KYLE" );
    oNames.insert( "Chester_AAB_KYLE" );
    oNames.insert( "Chico_AAB_KYLE" );
    oNames.insert( "Fresno_AAB_KYLE" );
    oNames.insert( "Hemet_KYLE" );
    oNames.insert( "Lancaster_KYLE" );
    oNames.insert( "Paso_Robles_KYLE" );
    oNames.insert( "Porterville_KYLE" );
    oNames.insert( "Redding_KYLE" );
    oNames.insert( "San_Bernandino_KYLE" );
    oNames.insert( "Santa_Maria_KYLE" );
    oNames.insert( "Siskiyou_County_KYLE" );
    oNames.insert( "Sonoma_County_KYLE" );
    oNames.insert( "Stockton_Air_Base_KYLE" );
    oNames.insert( "Bemidji_KYLE" );
    oNames.insert( "Brainard_KYLE" );
    oNames.insert( "Ely_KYLE" );
    oNames.insert( "Hibbing_KYLE" );
    oNames.insert( "Battle_Mountain_KYLE" );
    oNames.insert( "Boise_KYLE" );
    oNames.insert( "Cedar_City_KYLE" );
    oNames.insert( "Hill_AFB_KYLE" );
    oNames.insert( "McCall_KYLE" );
    oNames.insert( "Pocatello_KYLE" );
    oNames.insert( "Stead/Reno_KYLE" );
    oNames.insert( "Twin_Falls_KYLE" );
    oNames.insert( "Billings_KYLE" );
    oNames.insert( "Coeur_d'_Alene_KYLE" );
    oNames.insert( "Helena_KYLE" );
    oNames.insert( "Missoula_KYLE" );
    oNames.insert( "West_Yellowstone_KYLE" );
    oNames.insert( "Kingsley_ATB_KYLE" );
    oNames.insert( "La_Grande_ATB_KYLE" );
    oNames.insert( "Medford_ATB_KYLE" );
    oNames.insert( "Moses_Lake_ATB_KYLE" );
    oNames.insert( "Redmond_ATB_KYLE" );
    oNames.insert( "Troutdale_ATB_KYLE" );
    oNames.insert( "Durango_KYLE" );
    oNames.insert( "Grand_Junction_KYLE" );
    oNames.insert( "JEFCO_KYLE" );
    oNames.insert( "Rapid_City_KYLE" );
    oNames.insert( "Chattanooga_KYLE" );
    oNames.insert( "Fayetteville_KYLE" );
    oNames.insert( "Fort_Smith_KYLE" );
    oNames.insert( "Lake_City_ATB_KYLE" );
    oNames.insert( "Tallahassee_KYLE" );
    oNames.insert( "Alamogordo_KYLE" );
    oNames.insert( "Albuquerque_KYLE" );
    oNames.insert( "Fort_Huachuca_KYLE" );
    oNames.insert( "Phoenix_Mesa_KYLE" );
    oNames.insert( "Prescott_KYLE" );
    oNames.insert( "Roswell_KYLE" );
    oNames.insert( "Silver_City_KYLE" );
    oNames.insert( "Williams/Gateway_KYLE" );
    oNames.insert( "Winslow_KYLE" );

    BOOST_CHECK_EQUAL( Runner.m_VDispLoc.size(), oNames.size() );
    for(  int i = 0; i < Runner.m_VDispLoc.size(); i++  )
    {
        BOOST_CHECK( oNames.count( Runner.m_VDispLoc[i].GetDispLocID() ) > 0 );
    }
}

BOOST_AUTO_TEST_CASE( load_disploc_fpu )
{
    std::string oFpu;
    oFpu = poDA->GetDispLocFPU( "Boise/BLM" );
    std::cout << oFpu << std::endl;
    BOOST_CHECK( oFpu == "GB_ID_002" );
}

BOOST_AUTO_TEST_CASE( get_fpu_name )
{
    std::string oFpuName;
    oFpuName = poDA->GetFpuName( -115.67, 43.61 );
    if( oFpuName != "GB_ID_002" )
    BOOST_CHECK( oFpuName == "GB_ID_002" );
}

BOOST_AUTO_TEST_CASE( fpu_filter )
{
    std::vector<int>years = poDA->GetYearIndexes();

    poDA->AddFpuFilter( "GB_ID_002" );
    poDA->LoadRescType( Runner.m_VRescType, NULL );
    poDA->LoadProdRates( Runner.m_VProdRates, NULL );
    poDA->LoadDispatchLogic( Runner.m_VDispLogic, NULL, NULL );
    poDA->LoadFwa( Runner.m_VDispLogic, Runner.m_VFWA, NULL, NULL );
    poDA->LoadDispatchLocation( Runner.m_VFWA, Runner.m_VDispLoc, NULL );
    poDA->LoadResource( &Runner, Runner.m_VRescType, Runner.m_VDispLoc, Runner.m_VResource,
                        NULL, NULL, NULL, NULL, NULL );
    Runner.m_NumDispLogic = Runner.m_VDispLogic.size();
    Runner.m_NumFWA = Runner.m_VFWA.size();
    Runner.m_NumRescType = Runner.m_VRescType.size();
    Runner.m_NumDispLoc = Runner.m_VDispLoc.size();
    Runner.m_NumProdRates = Runner.m_VProdRates.size();
    Runner.m_NumResource = Runner.m_VResource.size();

    poDA->LoadScenario( Runner.m_VFWA, Runner.m_VFire, years[0], 0.0, NULL );
    BOOST_CHECK( Runner.m_VFire.size() > 0 && Runner.m_VFire.size() < 1000 );
}

BOOST_AUTO_TEST_CASE( gacc_filter )
{
    std::vector<int>years = poDA->GetYearIndexes();

    poDA->AddFpuFilter( "GB" );
    poDA->LoadRescType( Runner.m_VRescType, NULL );
    poDA->LoadProdRates( Runner.m_VProdRates, NULL );
    poDA->LoadDispatchLogic( Runner.m_VDispLogic, NULL, NULL );
    poDA->LoadFwa( Runner.m_VDispLogic, Runner.m_VFWA, NULL, NULL );
    poDA->LoadDispatchLocation( Runner.m_VFWA, Runner.m_VDispLoc, NULL );
    poDA->LoadResource( &Runner, Runner.m_VRescType, Runner.m_VDispLoc, Runner.m_VResource,
                        NULL, NULL, NULL, NULL, NULL );
    Runner.m_NumDispLogic = Runner.m_VDispLogic.size();
    Runner.m_NumFWA = Runner.m_VFWA.size();
    Runner.m_NumRescType = Runner.m_VRescType.size();
    Runner.m_NumDispLoc = Runner.m_VDispLoc.size();
    Runner.m_NumProdRates = Runner.m_VProdRates.size();
    Runner.m_NumResource = Runner.m_VResource.size();

    poDA->LoadScenario( Runner.m_VFWA, Runner.m_VFire, years[0], 0.0, NULL );
    BOOST_CHECK( Runner.m_VFire.size() > 1000 && Runner.m_VFire.size() < 10000 );
}

BOOST_AUTO_TEST_CASE( state_filter )
{
    std::vector<int>years = poDA->GetYearIndexes();

    poDA->AddFpuFilter( "ID" );
    poDA->LoadRescType( Runner.m_VRescType, NULL );
    poDA->LoadProdRates( Runner.m_VProdRates, NULL );
    poDA->LoadDispatchLogic( Runner.m_VDispLogic, NULL, NULL );
    poDA->LoadFwa( Runner.m_VDispLogic, Runner.m_VFWA, NULL, NULL );
    poDA->LoadDispatchLocation( Runner.m_VFWA, Runner.m_VDispLoc, NULL );
    poDA->LoadResource( &Runner, Runner.m_VRescType, Runner.m_VDispLoc, Runner.m_VResource,
                        NULL, NULL, NULL, NULL, NULL );
    Runner.m_NumDispLogic = Runner.m_VDispLogic.size();
    Runner.m_NumFWA = Runner.m_VFWA.size();
    Runner.m_NumRescType = Runner.m_VRescType.size();
    Runner.m_NumDispLoc = Runner.m_VDispLoc.size();
    Runner.m_NumProdRates = Runner.m_VProdRates.size();
    Runner.m_NumResource = Runner.m_VResource.size();

    poDA->LoadScenario( Runner.m_VFWA, Runner.m_VFire, years[0], 0.0, NULL );
    BOOST_CHECK( Runner.m_VFire.size() > 500 && Runner.m_VFire.size() < 5000 );
}

BOOST_AUTO_TEST_CASE( bad_filter )
{
    std::vector<int>years = poDA->GetYearIndexes();

    poDA->AddFpuFilter( "KYLE" );
    poDA->LoadRescType( Runner.m_VRescType, NULL );
    poDA->LoadProdRates( Runner.m_VProdRates, NULL );
    poDA->LoadDispatchLogic( Runner.m_VDispLogic, NULL, NULL );
    poDA->LoadFwa( Runner.m_VDispLogic, Runner.m_VFWA, NULL, NULL );
    poDA->LoadDispatchLocation( Runner.m_VFWA, Runner.m_VDispLoc, NULL );
    poDA->LoadResource( &Runner, Runner.m_VRescType, Runner.m_VDispLoc, Runner.m_VResource,
                        NULL, NULL, NULL, NULL, NULL );
    Runner.m_NumDispLogic = Runner.m_VDispLogic.size();
    Runner.m_NumFWA = Runner.m_VFWA.size();
    Runner.m_NumRescType = Runner.m_VRescType.size();
    Runner.m_NumDispLoc = Runner.m_VDispLoc.size();
    Runner.m_NumProdRates = Runner.m_VProdRates.size();
    Runner.m_NumResource = Runner.m_VResource.size();

    BOOST_CHECK( poDA->LoadScenario( Runner.m_VFWA, Runner.m_VFire, years[0], 0.0, NULL ) != 0 );
}

BOOST_AUTO_TEST_CASE( resc_copy_1 )
{
    std::vector<CResource*> aoOrig, aoCopy;
    CRescType oType = CRescType( "CRW", 1, 2, 3, 4 );
    CDispLoc oLoc = CDispLoc( "disp_loc", 1, "GB_ID_002", 45, -117 );
    aoOrig.push_back( new CCrew( "test_1", oType, 1, "1000", "1200", "Monday",
                                 "Tuesday", 100, 200, oLoc, 100, 0, 0 ) );
    aoOrig.push_back( new CCrew( "test_1", oType, 1, "1000", "1200", "Monday",
                                 "Tuesday", 100, 200, oLoc, 100, 0, 0 ) );

    int rc = poDA->CopyResources( aoOrig, aoCopy );
    BOOST_CHECK( rc == 0);
    BOOST_CHECK( aoOrig.size() == aoCopy.size() );
    for( int i = 0; i < aoOrig.size(); i++ )
    {
        BOOST_CHECK( aoOrig[i]->GetRescID() == aoCopy[i]->GetRescID() );
        BOOST_CHECK( &(aoOrig[i]) != &(aoCopy[i]) );
    }
}

BOOST_AUTO_TEST_CASE( resc_remove_set_1 )
{
    char **papszR;
    int nCount;
    int nHowMany = 100;
    papszR = poDA->GetRescRemovalSet( USFS | DOI_BLM, REGION_ALL, RESC_ALL, nHowMany,
                                      &nCount, NULL );
    BOOST_CHECK( papszR );
    BOOST_CHECK( nCount == nHowMany );
    int i = 0;
    while( papszR[i] != NULL )
        BOOST_CHECK( atoi( papszR[i++] ) > 0 );
    poDA->DestroyStringList( papszR );
}

BOOST_AUTO_TEST_CASE( resc_remove_set_2 )
{
    char **papszR;
    int nCount;
    int nHowMany = 100;
    papszR = poDA->GetRescRemovalSet( USFS | DOI_BLM, REGION_ALL, ENG, nHowMany, &nCount,
                                      NULL );
    BOOST_CHECK( papszR );
    BOOST_CHECK( nCount == nHowMany );
    int i = 0;
    while( papszR[i] != NULL )
        BOOST_CHECK( atoi( papszR[i++] ) > 0 );
    poDA->DestroyStringList( papszR );
}

BOOST_AUTO_TEST_CASE( resc_remove_set_3 )
{
    char **papszR;
    int nCount;
    int nHowMany = 100;
    papszR = poDA->GetRescRemovalSet( AGENCY_ALL, REGION_ALL, ENG, nHowMany, &nCount,
                                      NULL );
    BOOST_CHECK( papszR );
    BOOST_CHECK( nCount == nHowMany );
    int i = 0;
    while( papszR[i] != NULL )
        BOOST_CHECK( atoi( papszR[i++] ) > 0 );
    poDA->DestroyStringList( papszR );
}

BOOST_AUTO_TEST_CASE( resc_remove_set_4 )
{
    char **papszR;
    int nCount;
    int nHowMany = 10;
    /*
    ** Generate a ficticious ignore field.  We should only get back 201 and
    ** 202.  6044 is the max id, 6 is for commas/spaces/parentheses, 512 is
    ** sluff.
    */
    char szIgnore[6044 * 6 + 512];
    szIgnore[0] = '\0';
    int i = 0;
    for( i = 203; i < 6044; i++ )
    {
        sprintf( szIgnore, "%s%d,", szIgnore, i );
    }
    sprintf( szIgnore, "%s%d", szIgnore, i );
    papszR = poDA->GetRescRemovalSet( AGENCY_ALL, REGION_ALL, RESC_ALL, nHowMany,
                                      &nCount, szIgnore );
    BOOST_REQUIRE( papszR );
    BOOST_CHECK( nCount == 2 );
    i = 0;
    int nFound = 0;
    while( papszR[i] != NULL )
    {
        if( strcmp( papszR[i], "201" ) == 0 ||
            strcmp( papszR[i], "202" ) == 0 )
        {
            nFound++;
        }
        i++;
    }
    BOOST_CHECK_EQUAL( nFound, 2 );
    BOOST_CHECK_EQUAL( i, 2 );
    poDA->DestroyStringList( papszR );
}

BOOST_AUTO_TEST_CASE( resc_remove_set_5 )
{
    char **papszR;
    int nCount;
    int nHowMany = 6044;

    papszR = poDA->GetRescRemovalSet( AGENCY_ALL, REGION_ALL, ENG, nHowMany, &nCount,
                                      NULL );
    BOOST_REQUIRE( papszR );
    int rc;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    rc = sqlite3_open_v2( pszInputDb, &db, SQLITE_OPEN_READONLY, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT resc_type FROM resources " \
                                 "WHERE id=?", -1, &stmt, NULL );
    int nFound = 0;
    const char *pszType;
    int i = 0;
    for( i = 0; i < nCount; i++ )
    {
        sqlite3_bind_text( stmt, 1, papszR[i], -1, NULL );
        rc = sqlite3_step( stmt );
        pszType = (const char*)sqlite3_column_text( stmt, 0 );
        BOOST_CHECK( strncmp( pszType, "EN", 2 ) == 0 );
        rc = sqlite3_reset( stmt );
    }
    poDA->DestroyStringList( papszR );
    rc = sqlite3_finalize( stmt ); stmt = NULL;
}

BOOST_AUTO_TEST_CASE( resc_remove_set_6 )
{
    char **papszR;
    int nCount;
    int nHowMany = 6044;

    papszR = poDA->GetRescRemovalSet( 0, 0, 0, nHowMany, &nCount,
                                      NULL );
    BOOST_REQUIRE( papszR );
    BOOST_CHECK( nCount == 0 );
    BOOST_CHECK( papszR[0] == NULL );
    poDA->DestroyStringList( papszR );
}

BOOST_AUTO_TEST_CASE( resc_remove_set_7 )
{
    char **papszR;
    int nCount;
    papszR = poDA->GetRescRemovalSet( 0, 0, 0, 0, &nCount,
                                      NULL );
    BOOST_REQUIRE( !papszR );
    poDA->DestroyStringList( papszR );
}

BOOST_AUTO_TEST_CASE( resc_remove_set_8 )
{
    char **papszR;
    int nCount;
    papszR = poDA->GetRescRemovalSet( 0, 0, RESC_ALL, 100, &nCount,
                                      NULL );
    BOOST_REQUIRE( papszR );
    BOOST_CHECK_EQUAL( nCount, 0 );
    poDA->DestroyStringList( papszR );
}

BOOST_AUTO_TEST_CASE( resc_remove_set_9 )
{
    char **papszR;
    int nCount;
    papszR = poDA->GetRescRemovalSet( AGENCY_ALL, 0, 0, 100, &nCount,
                                      NULL );
    BOOST_REQUIRE( papszR );
    BOOST_CHECK_EQUAL( nCount, 0 );
    poDA->DestroyStringList( papszR );
}

BOOST_AUTO_TEST_CASE( resc_remove_set_10 )
{
    int rc;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    rc = sqlite3_open_v2( pszInputDb, &db, SQLITE_OPEN_READONLY, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM resources "
                                 "WHERE resc_type != 'ATT'",
                             -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    int nTotalCount = sqlite3_column_int( stmt, 0 );
    rc = sqlite3_finalize( stmt ); stmt = NULL;
    int nResult = poDA->GetRescCount( AGENCY_ALL, REGION_ALL, RESC_ALL, NULL );
    BOOST_CHECK_EQUAL( nResult, nTotalCount );
}

BOOST_AUTO_TEST_CASE( resc_remove_set_11 )
{
    int rc;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    rc = sqlite3_open_v2( pszInputDb, &db, SQLITE_OPEN_READONLY, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM resources " \
                                 "WHERE agency='FS' AND resc_type LIKE 'EN%' ",
                             -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    int nTotalCount = sqlite3_column_int( stmt, 0 );
    rc = sqlite3_finalize( stmt ); stmt = NULL;
    int nResult = poDA->GetRescCount( USFS, REGION_ALL, ENG, NULL );
    BOOST_CHECK_EQUAL( nResult, nTotalCount );
}

BOOST_AUTO_TEST_CASE( resc_remove_set_12 )
{
    int rc;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    rc = sqlite3_open_v2( pszInputDb, &db, SQLITE_OPEN_READONLY, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM resources " \
                                 "LEFT JOIN fpu on fpu_id=fpu.id " \
                                 "WHERE resc_type LIKE 'EN%' " \
                                 "AND fpu.name LIKE 'GB%'",
                             -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    int nTotalCount = sqlite3_column_int( stmt, 0 );
    rc = sqlite3_finalize( stmt ); stmt = NULL;
    int nResult = poDA->GetRescCount( AGENCY_ALL, GREAT_BASIN, ENG, NULL );
    BOOST_CHECK_EQUAL( nResult, nTotalCount );
}

BOOST_AUTO_TEST_CASE( resc_remove_set_13 )
{
    int rc;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    rc = sqlite3_open_v2( pszInputDb, &db, SQLITE_OPEN_READONLY, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM resources " \
                                 "LEFT JOIN fpu on fpu_id=fpu.id " \
                                 "WHERE resc_type LIKE 'EN%' " \
                                 "AND substr(fpu.name, 0, 3) IN " \
                                 "('GB','NR')",
                             -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    int nTotalCount = sqlite3_column_int( stmt, 0 );
    rc = sqlite3_finalize( stmt ); stmt = NULL;
    int nResult = poDA->GetRescCount( AGENCY_ALL, 
                                      GREAT_BASIN | NORTHERN_ROCKIES,
                                      ENG, NULL );
    BOOST_CHECK_EQUAL( nResult, nTotalCount );
}

BOOST_AUTO_TEST_SUITE_END() /* irs */

