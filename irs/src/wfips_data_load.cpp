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

int WfipsData::LoadRescTypes()
{
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2( db, "SELECT * FROM resc_type", -1, &stmt,
                        NULL );

    const char *pszName;
    int nSpeed, nDispDelay, nRespDelay, nSetupTime;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszName = (const char*)sqlite3_column_text( stmt, 0 );
        nSpeed = sqlite3_column_int( stmt, 1 );
        nDispDelay = sqlite3_column_int( stmt, 2 );
        nRespDelay = sqlite3_column_int( stmt, 3 );
        nSetupTime = sqlite3_column_int( stmt, 4 );
        poScenario->m_VRescType.push_back( CRescType( pszName, nSpeed,
                                                      nDispDelay, nRespDelay,
                                                      nSetupTime ) );
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return 0;
}
int WfipsData::LoadProdRates()
{
    int rc;
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, "SELECT * FROM prod_rate", -1, &stmt,
                             NULL );

    const char *pszName, *pszSpecCond;
    int nSlope, nStaff, nFuel;
    double dfRate;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszName = (const char*)sqlite3_column_text( stmt, 0 );
        nSlope = sqlite3_column_int( stmt, 1 );
        nStaff = sqlite3_column_int( stmt, 2 );
        nFuel = sqlite3_column_int( stmt, 3 );
        pszSpecCond = (const char*)sqlite3_column_text( stmt, 4 );
        dfRate = sqlite3_column_double( stmt, 5 );
        poScenario->m_VProdRates.push_back( CProdRates( pszName, nSlope, 
                                                        nStaff, nFuel,
                                                        pszSpecCond, dfRate ) );
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return 0;
}

int
WfipsData::LoadIrsStructs()
{
    poScenario = new CRunScenario();
    /*
    pD->LoadRescType( pS->m_VRescType, NULL );
    if( pRd->pfnProgress )
        pRd->pfnProgress( 0.1, "Loading Production Rates...", NULL );
    pD->LoadProdRates( pS->m_VProdRates, NULL );
    if( pRd->pfnProgress )
        pRd->pfnProgress( 0.3, "Loading Dispatch Logic...", NULL );
    pD->LoadDispatchLogic( pS->m_VDispLogic, NULL, NULL );
    if( pRd->pfnProgress )
        pRd->pfnProgress( 0.5, "Loading FWAs...", NULL );
    pD->LoadFwa( pS->m_VDispLogic, pS->m_VFWA, NULL, NULL );
    if( pRd->pfnProgress )
        pRd->pfnProgress( 0.7, "Loading Dispatch Locations...", NULL );
    pD->LoadDispatchLocation( pS->m_VFWA, pS->m_VDispLoc, NULL );
    if( pRd->pfnProgress )
        pRd->pfnProgress( 0.8, "Loading Tanker bases...", NULL );
    pD->LoadTankerBases( pS, pS->m_VFWA, pS->m_VDispLoc, -1, NULL );
    if( pRd->pfnProgress )
        pRd->pfnProgress( 0.9, "Loading Resources...", NULL );
    pD->LoadResource( pS, pS->m_VRescType, pS->m_VDispLoc, pS->m_VResource,
                      NULL, NULL, NULL, pRd->pszOmitSet, NULL );
                      //pszExternalResourceDb, NULL, NULL );
    pD->CreateLargeAirtankers( pS->m_VRescType, pS->m_VDispLoc,
                               pS->m_VResource, pRd->nTankerCount, NULL );

                               */
    return 0;
}

