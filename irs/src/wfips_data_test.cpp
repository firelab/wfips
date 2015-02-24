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
#define USFS                  (1 << 1)
#define DOI_BIA               (1 << 2)
#define DOI_BLM               (1 << 3)
#define DOI_FWS               (1 << 4)
#define DOI_NPS               (1 << 5)
#define STATE_LOCAL           (1 << 6)
#define REGIONAL              (1 << 7)
#define DOI_ALL               (DOI_BIA | DOI_BLM | DOI_FWS | DOI_NPS)
#define FED_ALL               (USFS | DOI_ALL)
#define AGENCY_ALL            (FED_ALL | STATE_LOCAL)
#define AGENCY_OTHER          (AGENCY_ALL &~ FED_ALL)

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


