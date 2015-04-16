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
 ** \param [in] pszDataPath a path to the required SQLite database used in WFIPS.
 ** \return a handle to a Wfips object, or NULL if creation fails.
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
 ** \param [in] w an opaque handle to a Wfips object.
 ** \param [in] pszPath path to a valid resource database.
 ** \return 0 on success, non-zero otherwise.
 */
int WfipsSetRescDb( WfipsH w, const char *pszPath )
{
    assert( w );
    return ((WfipsData*)w)->SetRescDb( pszPath );
}

/**
 ** \brief Write a resource subset to disk as a database.
 **
 ** This allows users to subset the resource data from the WFIPS data path, or
 ** whichever resource database is loaded using WfipsSetRescDb().
 **
 ** Optionally, a dispatch location array may be passed allowing resources to
 ** effectively move.  The size of this array must be the same size as the
 ** resource array.  The elements contain the ROWIDs of the dispatch location
 ** the resource is moved to.  -1 indicates no movement, and >= 0 indicates
 ** the resource should be moved.  If the array is NULL, then no resource is
 ** moved from it's current location.
 **
 ** \param [in] w an opaque handle to a Wfips object.
 ** \param [in] pszPath path to write the database to.
 ** \param [in] panIds resource ROWIDs from the currently loaded resource database.
 ** \param [in] panDispLocIds dispatch locations the resources shall be located at,
 **                      may be NULL if no resources move.
 ** \param nCount the size of the resource id array, and if not NULL, the
 **               dispatch location array.
 ** \return 0 on success, non-zero otherwise.
 */
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

/**
 ** \brief Set the resource prepositioning likelihood.
 **
 ** All values must meet the condition 0.0 >= value >= 1.0
 **
 ** \param [in] w an opaque handle to a Wfips object.
 ** \param [in] dfEngine engine prepositioning.
 ** \param [in] dfCrews crew prepositioning.
 ** \param [in] dfHelitack helitack  prepositioning.
 ** \return 0 on success, non-zero otherwise.
 */
int WfipsSetPrepositioning( WfipsH w, double dfEngine,
                            double dfCrews, double dfHelitack )
{
    assert( w );
    return ((WfipsData*)w)->SetPrepositioning( dfEngine, dfCrews, dfHelitack );
}

/**
 ** \brief Get the locations associated with an analysis area.
 **
 ** This function fetches an array of ROWIDs/FIDs for any dispatch location
 ** involved in an area defined as WKT.  This means any dispatch location
 ** associated with any FWA that intersects the geometry specified in the WKT.
 **
 ** panDispLocIds must be free'd with WfipsFree().
 **
 ** \param [in] w an opaque handle to a Wfips object.
 ** \param [in] pszWkt OGC Well-known text representation of a geometry.
 **                    Presumably a polygon or multipolygon, although any
 **                    geometry will work.
 ** \param [out] panDispLocIds array to be filled, and allocated.
 ** \param [out] pnCount output array size.
 ** \return 0 on success, non-zero otherwise.
 */

/*
int WfipsGetAssociatedDispLoc( WfipsH w, const char *pszWkt,
                               int **panDispLocIds, int *pnCount )
{
    assert( w );
    return ((WfipsData*)w)->GetAssociatedDispLoc( pszWkt, panDispLocIds, pnCount );
}
*/

/**
 ** \brief Get all resources for a set of dispatch locations.
 **
 ** ppsResc must be free'd with WfipsFreeAssociatedResources().
 **
 ** \param [in] w an opaque handle to a Wfips object.
 ** \param [in] panDispLocIds array of ROWIDs for dispatch locations.
 ** \param [in] nDispLocCount number of elements in the array.
 ** \param [out] ppsResc array of resources to be filled.
 ** \param [out] pnRescLocCount output array size.
 ** \param [in] nAgencyFlags filter for including specific agencies.
 ** \return 0 on success, non-zero otherwise.
 */
/*
int WfipsGetAssociatedResources( WfipsH w, int *panDispLocIds, int nDispLocCount,
                                 WfipsResc **ppsResc, int *pnRescLocCount,
                                 int nAgencyFlags )
{
    assert( w );
    return ((WfipsData*)w)->GetAssociatedResources( panDispLocIds, nDispLocCount,
                                                    ppsResc, pnRescLocCount,
                                                    nAgencyFlags );
}
*/

/**
 ** \brief Get the unique year identifiers for the fig.
 **
 ** \param [in] w an opaque handle to a Wfips object.
 ** \param [out] ppanIndices array of years to fill.
 ** \return the size of the array of years.
 */
/*
int WfipsGetScenarioIndices( WfipsH w, int **ppanIndices )
{
    assert( w );
    return ((WfipsData*)w)->GetScenarioIndices( ppanIndices );
}
*/

