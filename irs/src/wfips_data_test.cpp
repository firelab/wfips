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

