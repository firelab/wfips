/******************************************************************************
 *
 * Project:  Wildland Fire Investment Planning System
 * Purpose:  Main interface class
 * Author:   Kyle Shannon <kyle at pobox dot com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION ( RMRS )
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

#include "wfips.h"

/*
** C API for WFIPS.  This defines the public interface for the entire program.
*/

/**
 ** \brief Create a WFIPS object for data access and simulation running.
 **
 ** Wfips relies on several SQLite databases to run simulations.  A valid path
 ** to these databases must be provided.
 **
 ** \param pszDataPath a path to the required SQLite database used in WFIPS.
 ** \return a handle to a Wfips object, or NULL if creation fails
 */
WfipsH WfipsCreate( const char *pszDataPath )
{
    WfipsData *p;
    assert( pszDataPath );
    p = new WfipsData( pszDataPath );
    if( !p->Valid() )
    {
        delete p;
    }
    return NULL;
}

/**
 ** \brief Set a user defined resource database for simulation.
 **
 ** WFIPS can use alternative resource sets, or sets modified by the user.
 ** This database must have the same schema as the original database in the
 ** WFIPS data path.
 **
 ** \param w an opaque handle to a Wfips object
 ** \param pszPath path to a valid resource database
 ** \return 0 on success, non-zero otherwise
 */
int WfipsSetRescDb( WfipsH w, const char *pszPath )
{
    assert( w );
    return ((WfipsData*)w)->SetRescDb( pszPath );
}

int WfipsWriteRescDb( WfipsH w,
                      const char *pszPath,
                      int *panIds,
                      int *panDispLocIds,
                      int nCount )
{
    assert( w );
    return ((WfipsData*)w)->WriteRescDb( pszPath, panIds,
                                         panDispLocIds, nCount );
}

int WfipsSetPrepositioning( WfipsH w, double dfEngine,
                            double dfCrews, double dfHelitack )
{
    assert( w );
    return ((WfipsData*)w)->SetPrepositioning( dfEngine, dfCrews, dfHelitack );
}

int WfipsGetAssociatedDispLoc( WfipsH w, const char *pszWkt,
                               int **panDispLocIds, int *pnCount )
{
    assert( w );
    return ((WfipsData*)w)->GetAssociatedDispLoc( pszWkt, panDispLocIds, pnCount );
}

int WfipsGetAssociatedResources( WfipsH w, int *panDispLocIds, int nDispLocCount,
                                 WfipsResc **ppsResc, int *pnRescLocCount,
                                 int nAgencyFlags )
{
    assert( w );
    return ((WfipsData*)w)->GetAssociatedResources( panDispLocIds, nDispLocCount,
                                                    ppsResc, pnRescLocCount,
                                                    nAgencyFlags );
}

int WfipsGetScenarioIndices( WfipsH w, int **ppanIndices )
{
    assert( w );
    return ((WfipsData*)w)->GetScenarioIndices( ppanIndices );
}

void WfipsFreeAssociatedResources( WfipsResc *psResc, int nCount )
{
    WfipsData::FreeAssociatedResources( psResc, nCount );
}

void WfipsFree( void *p )
{
    WfipsData::Free( p );
}


