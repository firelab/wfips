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

/*
** Test various combinations of resources to create a set.
*/

int WfipsData::TestBuildAgencySet1()
{
    const char* pszSet = BuildAgencySet( USFS );
    return strncmp( pszSet, "'FS'", MAX_PATH );
}

int WfipsData::TestBuildAgencySet2()
{
    const char* pszSet = BuildAgencySet( DOI_ALL );
    return strncmp( pszSet, "'BIA','BLM','FWS','NPS'", MAX_PATH );
}

int WfipsData::TestBuildAgencySet3()
{
    const char* pszSet = BuildAgencySet( FED_ALL );
    return strncmp( pszSet, "'FS','BIA','BLM','FWS','NPS'", MAX_PATH );
}

int WfipsData::TestBuildAgencySet4()
{
    const char* pszSet = BuildAgencySet( AGENCY_ALL );
    return strncmp( pszSet, "'FS','BIA','BLM','FWS','NPS','STATE/LOCAL'", MAX_PATH );
}

int WfipsData::TestBuildAgencySet5()
{
    const char* pszSet = BuildAgencySet( AGENCY_OTHER );
    return strncmp( pszSet, "'STATE/LOCAL'", MAX_PATH );
}

int WfipsData::TestFidSet1()
{
    int anRescSet[] = {1,2,3,4,5};
    char *pszRescSet = BuildFidSet( anRescSet, 5 );
    int rc;
    if( !pszRescSet )
        rc = 1;
    else
        rc = strcmp( pszRescSet, "1,2,3,4,5" );
    WfipsData::Free( pszRescSet );
    return rc;
}

int WfipsData::TestRescTypeLoad1()
{
    poScenario = new CRunScenario();
    LoadRescTypes();
    int rc = 0;
    if( poScenario->m_VRescType.size() == 0 )
        rc = 1;
    delete poScenario;
    return rc;
}

int WfipsData::TestProdLoad1()
{
    poScenario = new CRunScenario();
    LoadProdRates();
    int rc = 0;
    if( poScenario->m_VProdRates.size() == 0 )
        rc = 1;
    delete poScenario;
    return rc;
}

int WfipsData::TestDispLogLoad1()
{
    poScenario = new CRunScenario();
    LoadDispatchLogic();
    int rc = 0;
    if( poScenario->m_VDispLogic.size() == 0 )
        rc = 1;
    delete poScenario;
    return rc;
}

int WfipsData::TestDispLogLoad2()
{
    poScenario = new CRunScenario();
    LoadDispatchLogic();
    int rc = 0;
    int a = poScenario->m_VDispLogic.size();
    poScenario->m_VDispLogic.clear();
    SetAnalysisAreaMask( "POLYGON((-114 47, -113 47, -113 46, -114 46, -114 47))" );
    LoadDispatchLogic();
    int b = poScenario->m_VDispLogic.size();
    delete poScenario;
    if( a <= b || a == 0 || b == 0 )
        rc = 1;
    return rc;
}

int WfipsData::TestFwaLoad1()
{
    poScenario = new CRunScenario();
    LoadDispatchLogic();
    LoadFwas();
    int rc = 0;
    if( poScenario->m_VFWA.size() == 0 )
        rc = 1;
    delete poScenario;
    return rc;
}

int WfipsData::TestFwaLoad2()
{
    poScenario = new CRunScenario();
    LoadDispatchLogic();
    LoadFwas();
    int rc = 0;
    int a = poScenario->m_VFWA.size();
    poScenario->m_VFWA.clear();
    SetAnalysisAreaMask( "POLYGON((-114 47, -113 47, -113 46, -114 46, -114 47))" );
    LoadFwas();
    int b = poScenario->m_VFWA.size();
    delete poScenario;
    if( a <= b || a == 0 || b == 0 )
        rc = 1;
    return rc;
}

int WfipsData::TestDispLocLoad1()
{
    poScenario = new CRunScenario();
    LoadDispatchLogic();
    LoadFwas();
    LoadDispatchLocations();
    int rc = 0;
    if( poScenario->m_VDispLoc.size() == 0 )
        rc = 1;
    delete poScenario;
    return rc;
}

int WfipsData::TestDispLocLoad2()
{
    poScenario = new CRunScenario();
    SetAnalysisAreaMask( "POLYGON((-114 47, -113 47, -113 46, -114 46, -114 47))" );
    LoadFwas();
    LoadDispatchLocations();
    int rc = 0;
    if( poScenario->m_VDispLoc.size() > 1000 )
        rc = 1;
    delete poScenario;
    return rc;
}

int WfipsData::TestResourceLoad1()
{
    poScenario = new CRunScenario();
    LoadProdRates();
    LoadRescTypes();
    LoadFwas();
    LoadDispatchLocations();
    LoadResources();
    int rc = 0;
    if( poScenario->m_VResource.size() == 0 )
        rc = 1;
    delete poScenario;
    return rc;
}

int WfipsData::TestResourceLoad2()
{
    poScenario = new CRunScenario();
    LoadProdRates();
    LoadRescTypes();
    SetAnalysisAreaMask( "POLYGON((-114 47, -113 47, -113 46, -114 46, -114 47))" );
    LoadFwas();
    LoadDispatchLocations();
    LoadResources();
    int rc = 0;
    if( poScenario->m_VResource.size() > 1000 )
        rc = 1;
    delete poScenario;
    return rc;
}

int WfipsData::TestScenLoad1()
{
    int rc;
    poScenario = new CRunScenario();
    LoadFwas();
    rc = LoadScenario( 5, NULL, 1.0, 0 );
    delete poScenario;
    return rc;
}

int WfipsData::TestScenLoad2()
{
    int rc;
    poScenario = new CRunScenario();
    LoadFwas();
    rc = LoadScenario( 1, NULL, 1.0, 0 );
    delete poScenario;
    return poScenario->m_VFire.size();
}

int WfipsData::TestScenLoad3()
{
    int rc;
    poScenario = new CRunScenario();
    SetAnalysisAreaMask( "POLYGON((-114 47, -113 47, -113 46, -114 46, -114 47))" );
    LoadFwas();
    rc = LoadScenario( 5, NULL, 1.0, 0 );
    if( poScenario->m_VFire.size() == 0 )
        rc = 1;
    delete poScenario;
    return rc;
}

int WfipsData::TestScenLoad4()
{
    int rc, a, b;
    rc = 0;
    poScenario = new CRunScenario();
    LoadFwas();
    LoadScenario( 5, NULL, 1.0, 0 );
    a = poScenario->m_VFire.size();
    SetAnalysisAreaMask( "POLYGON((-114 47, -113 47, -113 46, -114 46, -114 47))" );
    LoadScenario( 5, NULL, 1.0, 0 );
    b = poScenario->m_VFire.size();
    if( a == 0 || b == 0 || a < b )
        rc = 1;
    delete poScenario;
    return rc;
}

int WfipsData::TestScenLoad5()
{
    int rc, a, b;
    rc = 0;
    poScenario = new CRunScenario();
    LoadFwas();
    LoadScenario( 5, NULL, 1.0, AGENCY_ALL );
    a = poScenario->m_VFire.size();
    b = LoadScenario( 5, NULL, 1.0, DOI_BLM );
    b = poScenario->m_VFire.size();
    if( a == 0 || b == 0 || a < b )
        rc = 1;
    delete poScenario;
    return rc;
}

int WfipsData::TestScenLoad6()
{
    int rc, i;
    poScenario = new CRunScenario();
    const char *pszWkt = "POLYGON((-114 47, -113 47, -113 46, -114 46, -114 47))";
    SetAnalysisAreaMask( pszWkt );
    LoadFwas();
    rc = LoadScenario( 5, NULL, 1.0, AGENCY_ALL );
    for( i = 0; i < poScenario->m_VFire.size(); i++ )
    {
        if( poScenario->m_VFire[i].GetTreated() != 0 )
        {
            rc = 1;
            break;
        }
    }
    delete poScenario;
    return rc;
}

int WfipsData::TestScenLoad7()
{
    int rc, i;
    poScenario = new CRunScenario();
    const char *pszWkt = "POLYGON((-114 47, -113 47, -113 46, -114 46, -114 47))";
    SetAnalysisAreaMask( pszWkt );
    LoadFwas();
    rc = LoadScenario( 5, pszWkt, 1.0, AGENCY_ALL );
    for( i = 0; i < poScenario->m_VFire.size(); i++ )
    {
        if( poScenario->m_VFire[i].GetTreated() != 1 )
        {
            rc = 1;
            break;
        }
    }
    delete poScenario;
    return rc;
}


