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

#include "raster_access.h"

RasterAccess::RasterAccess()
{
    hDS = NULL;
    padfData = NULL;
    nLines = 0;
    panLines = NULL;
    nIndex = 0;
}

RasterAccess::RasterAccess( const char *pszFilename, int nBand, int nLines )
{
    hDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hDS == NULL )
    {
        hDS = NULL;
        padfData = NULL;
        nLines = 0;
        panLines = NULL;
    }
    else
    {
        hBand = GDALGetRasterBand( hDS, nBand );
        nXSize = GDALGetRasterXSize( hDS );
        nYSize = GDALGetRasterYSize( hDS );
        if( nLines > 0 )
        {
            this->nLines = nLines;
        }
        else
        {
            this->nLines = nYSize;
        }
        padfData = (double*)CPLMalloc( nXSize * this->nLines * sizeof( double ) );
        panLines = (int*)CPLMalloc( nLines * sizeof( int ) );
        if( nLines < 1 )
        {
            GDALRasterIO( hBand, GF_Read, 0, 0, nXSize, nYSize,
                          padfData, nXSize, nYSize, GDT_Float64, 0, 0 );
        }
    }
    //nIndex = 0;
}

RasterAccess::~RasterAccess()
{
    GDALClose( hDS );
    if( padfData != NULL )
        CPLFree( padfData );
    if( panLines != NULL )
        CPLFree( panLines );
}

RA_Err RasterAccess::SetNumLines( int nLines )
{
    return RAE_NONE;
}

/**
 * \brief Fetch the pixel/line coordinates for a given projected coordinate
 *
 * \param dfGeoX x coordinate
 * \param dfGeoY y coordinate
 * \param[out] nPixel pixel index to be set
 * \param[out] nLine line index to be set
 * \param pszWkt projection reference string in wkt, if NULL assume coords
 *               are in native srs
 * \return RAE_NONE on success
 */

RA_Err RasterAccess::PixelIndex( double dfX, double dfY, int *nPixel,
                                 int *nLine, const char *pszWkt )
{
    double dfNewX, dfNewY, dfDummy;
    dfNewX = (double)dfX;
    dfNewY = (double)dfY;

    if( pszWkt != NULL )
    {
        const char *pszDstWkt;
        pszDstWkt = GDALGetProjectionRef( hDS );
        int err = ReprojectPoint( &dfNewX, &dfNewY, pszWkt, pszDstWkt );
        if( err != 0 )
        {
            fprintf( stderr, "Could not reproject point\n" );
            return RAE_WARP;
        }
    }

    double adfGeoTransform[6], adfInvGeoTransform[6];

    if( GDALGetGeoTransform( hDS, adfGeoTransform ) != CE_None )
    {
        fprintf( stderr, "Cannot fetch Geotransform" );
        return -RAE_IO;
    }

    GDALInvGeoTransform( adfGeoTransform, adfInvGeoTransform );

    *nPixel = (int) floor( adfInvGeoTransform[0] + adfInvGeoTransform[1] *
                           dfNewX + adfInvGeoTransform[2] * dfNewY );
    *nLine = (int) floor( adfInvGeoTransform[3] + adfInvGeoTransform[4] *
                          dfNewX + adfInvGeoTransform[5] * dfNewY );
    return RAE_NONE;
}

/**
 * \brief Directly fetch a pixel/line value from a GDAL Dataset
 *
 * \warning no error checking
 *
 * \param hDS Valid dataset
 * \param band band to fetch value from
 * \param dfGeoX x coordinate
 * \param dfGeoY y coordinate
 * \param [out] dfValue double to be filled
 * \param pszWkt srs in wkt for coordinates
 * \return 0 on success
 */
RA_Err RasterAccess::PixelValue( double dfX, double dfY, double *dfValue,
                                 const char *pszWkt )
{
    int e;
    int nPixel, nLine;
    e = PixelIndex( dfX, dfY, &nPixel, &nLine, pszWkt );
    if( nPixel < 0 || nLine < 0 )
    {
        return RAE_BOUNDS;
    }
    if( e > 0 )
    {
        return e;
    }
    FetchValue( nPixel, nLine, dfValue );
    return RAE_NONE;
}

/**
 * \brief Find the origin of a pixel
 *
 * Find the origin of a pixel given an pixel/line combination.  Warp the point
 * to a desired SRS if needed.
 *
 * \param nPixel pixel x coordinate
 * \param nLine pixel y coordinate
 * \param[out] dfX geolocated x coordinate
 * \param[out] dfY geolocated y coordinate
 * \param pszDstWkt target SRS in wkt, if null use native coordinates
 * \return RAE_NONE on success
 */
RA_Err RasterAccess::PixelOrigin( int nPixel, int nLine, double *dfX, 
                                  double *dfY, const char *pszDstWkt )
{
    double adfGeoTransform[6];
    int err = GDALGetGeoTransform( hDS, adfGeoTransform );
    if( err != CE_None )
    {
        return RAE_GDAL;
    }
    *dfX = adfGeoTransform[0] + adfGeoTransform[1] * nPixel;
    *dfY = adfGeoTransform[3] + adfGeoTransform[5] * nLine;
    if( pszDstWkt != NULL )
    {
        const char *pszSrcWkt = GDALGetProjectionRef( hDS );
        if( pszSrcWkt != NULL )
        {
            ReprojectPoint( dfX, dfY, pszSrcWkt, pszDstWkt );
        }
    }
    return RAE_NONE;
}

/**
 * \brief reproject a point in place
 *
 * If no destination wkt is given, WGS84 geographic is used
 *
 * \param dfX address of x coordinate
 * \param dfY address of y coordinate
 * \param pszSrcWkt the wkt of the point
 * \param pszDstWkt the wkt of the desired projection
 * \return 0 on success, nonzero otherwise.
 */
RA_Err RasterAccess::ReprojectPoint( double *dfX, double *dfY, const char *pszSrcWkt, 
                       const char *pszDstWkt )
{
    double dfDummy;
    if(pszDstWkt == NULL)
    {
        fprintf( stderr, "Invalid projection reference" );
        return RAE_WARP;
    }
    if( pszSrcWkt != NULL && !EQUAL( pszSrcWkt, pszDstWkt ) )
    {
        OGRSpatialReference oSrcSRS, oDstSRS;
        OGRCoordinateTransformation *poCT;
        oSrcSRS.importFromWkt( (char**)&pszSrcWkt );
        oSrcSRS.morphFromESRI();
        oDstSRS.importFromWkt( (char**)&pszDstWkt );
        oDstSRS.morphFromESRI();
        poCT = OGRCreateCoordinateTransformation( &oSrcSRS, &oDstSRS );
        if( poCT == NULL )
            return RAE_WARP;
        poCT->Transform( 1, dfX, dfY, &dfDummy );
        OGRCoordinateTransformation::DestroyCT( poCT );
    }
    return RAE_NONE;
}

RA_Err RasterAccess::LoadLine( int nNewLine, int *nNewIndex )
{
    /*
    if( hDS == NULL || padfData == NULL || panLines == NULL )
    {
        return RAE_UNINIT;
    }
    for( int i = 0;i < nLines;i++ )
    {
        if( panLines[i] == nNewLine )
        {
            *nNewIndex = i;
            return RAE_NONE;
        }
    }
    nIndex++;
    if( nIndex + 1 > nLines )
    {
        nIndex = nIndex % nLines;
    }
    panLines[nIndex] = nNewLine;
    GDALRasterIO( hBand, GF_Read, 0, nNewLine, nXSize, 1,
                  &padfData[nIndex * nXSize], nXSize, 1, GDT_Float64, 0, 0 );
    *nNewIndex = nIndex;
    */
    return RAE_NONE;
}

RA_Err RasterAccess::FetchValue( int nPixel, int nLine, double *dfValue )
{
    //LoadLine( nLine, &i );
    *dfValue =  padfData[nLine*nXSize+nPixel];
    return RAE_NONE;
}

