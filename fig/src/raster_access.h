/******************************************************************************
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Raster Data Access
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

#ifndef RASTER_ACCESS_H_
#define RASTER_ACCESS_H_

/**
 * \file raster_access.h
 *
 * \brief Allow point data query to a raster, loading data on demand and
 * freeing data as well.
 *
 * \author Kyle Shannon
 * \date 2012-08-15
 * \sinde 1.0.0
 */

#include "gdal.h"
#include "cpl_conv.h"
#include "ogr_spatialref.h"

typedef int RA_Err;
static const int RAE_NONE       = 0;
static const int RAE_IO         = 1;
static const int RAE_BOUNDS     = 2;
static const int RAE_WARP       = 3;
static const int RAE_MEMORY     = 4;
static const int RAE_GDAL       = 5;
static const int RAE_UNINIT     = 6;

/**
 * \class RasterAccess
 *
 * \brief Allow raster data query given a point in some projection.
 *
 * Current implementation assumes raster is traversed by scanline.  Block
 * access should be implemented as well.  Lines are loaded into memory and kept
 * until pushed out
 *
 * \author Kyle Shannon
 * \date 2012-08-15
 * \sinde 1.0.0
 */
class RasterAccess
{
public:
    RasterAccess();
    RasterAccess( const char *pszFilename, int nBand=1, int nLines=0 );
    ~RasterAccess();

    RA_Err SetNumLines( int nLines );
    RA_Err PixelIndex( double dfX, double dfY, int *nPixel, int *nLine,
                       const char *pszWkt=NULL );
    RA_Err PixelValue( double dfX, double dfY, double *dfValue, 
                       const char *pszWkt=NULL );
    RA_Err PixelOrigin( int nPixel, int nLine, double *dfX, double *dfY,
                        const char *pszWkt=NULL );

private:
    GDALDatasetH hDS;
    GDALRasterBandH hBand;
    double *padfData;
    int nLines;
    int *panLines;
    int nIndex;
    int nXSize, nYSize;

    RA_Err LoadLine( int nNewLine, int *nNewIndex );
    RA_Err FetchValue( int nPixel, int nLine, double *dfValue );

    RA_Err ReprojectPoint( double *dfX, double *dfY, const char *pszSrcWkt, 
                           const char *pszDstWkt=NULL );
};
#endif /* RASTER_ACCESS_H_ */

