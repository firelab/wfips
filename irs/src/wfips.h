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

#ifndef WFIPS_H_
#define WFIPS_H_


/* Standard library */
#include <assert.h>
#include <stdlib.h>
#include "wfips_data.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void * WfipsH;

WfipsH WfipsCreate( const char *pszDataPath );

int WfipsSetRescDb( WfipsH w, const char *pszPath );

int WfipsWriteRescDb( WfipsH w, const char *pszPath,
                      int *panIds, int *panDispLocIds,
                      int nCount );

int WfipsSetPrepositioning( WfipsH w, double dfEngine,
                            double dfCrews, double dfHelitack );

int WfipsGetAssociatedDispLoc( WfipsH w, const char *pszWkt,
                               int **panDispLocIds, int *pnCount );

int WfipsGetAssociatedResources( WfipsH w, int *panDispLocIds, int nDispLocCount,
                                 WfipsResc **ppsResc, int *pnRescLocCount,
                                 int nAgencyFlags );
void WfipsFreeAssociatedResources( WfipsResc *psResc, int nCount );
void WfipsFree( void *p );

int WfipsGetScenarioIndices( WfipsH w, int **ppanIndices );

#ifdef __cplusplus
}
#endif

#endif /* WFIPS_H_ */

