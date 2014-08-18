/******************************************************************************
 *
 * $Id: fig_api.h 147 2012-08-21 16:04:38Z kyle.shannon $
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Class representing a fire ignition generator
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

#ifndef FIG_API_H_
#define FIG_API_H_

#include "fig.h"

#if defined(WIN32)
  #if defined(fig_EXPORTS)
    #define FIGDLL_EXPORT __declspec(dllexport)
  #else
    #define FIGDLL_EXPORT __declspec(dllimport)
  #endif
#else
  #define FIGDLL_EXPORT
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    typedef void *FIGGeneratorH;
    FIGDLL_EXPORT FIGGeneratorH FIGCreate( const char * const *);
    FIGDLL_EXPORT int FIGGenerateFromFile( FIGGeneratorH hFig, 
                                     const char *pszInputFile,
                                     const char *pszOutputFile,
                                     const char * const *papszOptions = NULL );
    FIGDLL_EXPORT int FIGSetDataPath( FIGGeneratorH hFig, const char *pszPath,
                                const char * const *papszOptions = NULL );
    FIGDLL_EXPORT int FIGSetInputWkt( FIGGeneratorH hFig, const char *pszWkt,
                                const char * const *papszOptions = NULL );
    FIGDLL_EXPORT int FIGSetFormat( FIGGeneratorH hFig, const char *pszFormat,
                                const char * const *papszOptions = NULL );
    FIGDLL_EXPORT int FIGSetSpatialEnv( FIGGeneratorH hFig, double dfMaxX,
                                        double dfMinX, double dfMaxY,
                                        double dfMinY,
                                        const char * const * papszOptions = NULL );
    FIGDLL_EXPORT int FIGSetFpuCode( FIGGeneratorH hFig, const char *pszFpuCode,
                                     const char * const * papszOptions = NULL );
    FIGDLL_EXPORT void FIGShowProgress( FIGGeneratorH hFig, int bShow,
                                        const char * const *papszOptions = NULL );
    FIGDLL_EXPORT int FIGGetLastErrorCode( FIGGeneratorH,
                                     const char * const *papszOptions = NULL );
    FIGDLL_EXPORT const char * FIGGetLastErrorMessage( FIGGeneratorH, 
                                                 const char * const *papszOptions = NULL );
    FIGDLL_EXPORT int FIGDestroy( FIGGeneratorH, const char * const *papszOptions = NULL );
#ifdef __cplusplus
}
#endif
#endif /* FIG_API_H_ */
