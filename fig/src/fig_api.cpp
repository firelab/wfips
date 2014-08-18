/******************************************************************************
 *
 * $Id: fig_api.cpp 160 2012-09-04 21:02:30Z kyle.shannon $
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

#include "fig_api.h"

/**
 * \file fig_api.h
 * \brief C API for Fire Ignition Generator
 * \date 2012-07-30
 * \author Kyle Shannon <kyle@pobox.com>
 * \since 1.0
 * \see FIGGenerator
 */

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \brief Create and return an opaque handle to a FIG object.
 *
 * \param papszOptions options for future implementation, not used
 * \return an opaque handle to a FIGGenerator object
 */
FIGDLL_EXPORT FIGGeneratorH FIGCreate( const char * const * papszOptions )
{
    FIGGenerator *poFig = new FIGGenerator( papszOptions );
    return (FIGGeneratorH)poFig;
}
/**
 * \brief Generate the FIG data from a standard output file from the R
 * model
 *
 * \param hFig an opaque handle to a FIGGenerator object
 * \param pszInputFile input text file
 * \param pszOutputFile file to write data in.
 * \param papszOptions options for future implementation, not used
 * \return FE_NONE on success, specific error otherwise
 */
FIGDLL_EXPORT int FIGGenerateFromFile( FIGGeneratorH hFig,
                                       const char *pszInputFile,
                                       const char *pszOutputFile, 
                                       const char * const * papszOptions )
{
    return ((FIGGenerator*)hFig)->GenerateFromFile( (const char*)pszInputFile,
                                                    (const char*)pszOutputFile,
                                                    papszOptions );
}
/**
 * \brief Optionally set the data path for FIG
 *
 * FIG_DATA is checked first.  If it is set, the path is set as the
 * default.  If it is not set, a path must be specified with a call to this
 * function.  In the future, it will check ${INSTALL_PREFIX}/data and most
 * likely the executable directory and the cwd as well.  This is not
 * implemented yet and a call should be made here or FIG_DATA should be
 * set.  Either of those will override and automatic 'finding' in the
 * future.
 *
 * \param hFig an opaque handle to a FIGGenerator object
 * \param pszPath path to the fig data, preferably absolute
 * \param papszOptions options for future implementation, not used
 * \return FE_NONE on success, specific error otherwise
 */
FIGDLL_EXPORT int FIGSetDataPath( FIGGeneratorH hFig, const char *pszPath,
                                  const char * const * papszOptions )
{
    return ((FIGGenerator*)hFig)->SetDataPath( (const char*)pszPath,
                                               papszOptions );
}
/**
 * \brief Override the default wkt for the incoming points
 *
 * \param hFig an opaque handle to a FIGGenerator object
 * \param pszWkt projection information in wkt format
 * \param papszOptions options for future implementation, not used
 * \return FE_NONE on success, specific error otherwise
 */
FIGDLL_EXPORT int FIGSetInputWkt( FIGGeneratorH hFig, const char *pszWkt,
                                  const char * const * papszOptions )
{
    return ((FIGGenerator*)hFig)->SetInputWkt( (const char*)pszWkt, papszOptions );
}
/**
 * \brief Override the default output format for the data
 *
 * \param hFig and opaque handle to a FIGGenerator object
 * \param pszFormat a new OGR supported format
 * \param papszOptions options for future implementation, not used
 * \return FE_NONE on success, specific error otherwise
 */
FIGDLL_EXPORT int FIGSetFormat( FIGGeneratorH hFig, const char *pszFormat,
                                const char * const *papszOptions )
{
    return ((FIGGenerator*)hFig)->SetFormat( pszFormat, papszOptions );
}
/**
 * \brief Set an envelope to limit the output spatially
 *
 * \param hFig an opaque handle to a FIGGenerator object
 * \param n Northern extent of envelope
 * \param e Eastern extent of envelope
 * \param s Southern extent of envelope
 * \param w Western extent of envelope
 * \param papszOptions options for future implementation, not used
 * \return FE_NONE on success
 */
FIGDLL_EXPORT int FIGSetSpatialEnv( FIGGeneratorH hFig, double dfMaxX,
                                    double dfMinX, double dfMaxY,
                                    double dfMinY,
                                    const char * const * papszOptions )
{
    return ((FIGGenerator*)hFig)->SetSpatialEnv( dfMaxX, dfMinX, dfMaxY,
                                                 dfMinY, papszOptions );
}
/**
 * \brief Set an envelope to limit the output spatially based on an fpu
 *
 * \param hFig an opaque handle to a FIGGenerator object
 * \param pszFpuCode the fpu code
 * \param papszOptions options for future implementation, not used
 * \return FE_NONE on success
 */
FIGDLL_EXPORT int FIGSetFpuCode( FIGGeneratorH hFig, const char *pszFpuCode,
                                 const char * const * papszOptions )
{
    return ((FIGGenerator*)hFig)->SetFpuCode( pszFpuCode, papszOptions );
}
  
/**
 * \brief Show progress on the command line
 *
 * \note This should be removed in favor of a call back
 *
 * \param hFig an opaque handle to a FIGGenerator object
 * \param bShow boolean to show the default GDALTermProgress()
 * \param papszOptions options for future implementation, not used
 */
FIGDLL_EXPORT void FIGShowProgress( FIGGeneratorH hFig, int bShow,
                                    const char * const * papszOptions )
{
    ((FIGGenerator*)hFig)->ShowProgress( bShow, papszOptions );
}
/**
 * \brief Fetch the last recorded error
 *
 * This function allows access to the last FE_Err code (as an integer)
 *
 * \param hFig an opaque handle to a FIGGenerator object
 * \param papszOptions options for future implementation, not used
 */
FIGDLL_EXPORT int FIGGetLastErrorCode( FIGGeneratorH hFig,
                                       const char * const * papszOptions )
{
    return ((FIGGenerator*)hFig)->GetLastErrorCode( papszOptions);
}
/**
 * \brief Fetch the last recorded error message
 *
 * This function allows access to the last error message
 *
 * \param hFig an opaque handle to a FIGGenerator object
 * \param papszOptions options for future implementation, not used
 */
FIGDLL_EXPORT const char * FIGGetLastErrorMessage( FIGGeneratorH hFig,
                                               const char * const * papszOptions )
{
    return ((FIGGenerator*)hFig)->GetLastErrorMessage( papszOptions);
}
/**
 * \brief Remove the object from memory and free up space
 *
 * \param hFig an opaque handle to a FIGGenerator object
 * \param papszOptions options for future implementation, not used.  Most
 * likely never will be.
 * \return 0
 */
FIGDLL_EXPORT int FIGDestroy( FIGGeneratorH hFig,
                              const char * const * papszOptions )
{
    delete (FIGGenerator*)hFig;
    return 0;
}
#ifdef __cplusplus
}
#endif
