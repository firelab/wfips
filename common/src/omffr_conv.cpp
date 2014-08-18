/******************************************************************************
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  general disclaimer
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
#include "omffr_conv.h"

/**
 * \brief Get a textual display of the day of the week
 *
 * \param nDay zero based index of the day of the week
 * \return a textual representation of the day, ie "Tuesday"
 */
const char* DayOfWeek( int nDay )
{
    //return DAYS_OF_WEEK[nDay];
    switch( nDay )
    {
        case 0:
            return "Sunday";
            break;
        case 1:
            return "Monday";
            break;

        case 2:
            return "Tuesday";
            break;

        case 3:
            return "Wednesday";
            break;

        case 4:
            return "Thursday";
            break;

        case 5:
            return "Friday";
            break;

        case 6:
            return "Saturday";
            break;
        default:
            return "Sunday";
    }
}

/**
 * \brief find the day of the month given a julian day
 *
 * \note ignore leap years
 *
 * \param nDay julian day
 * \return day of the month, ie julian day 32-> 1 for February 1
 */
int DayOfMonth( int nDay )
{
    int nMonth = MonthOfJulian( nDay );
    int nDaysInMonth = DAYS_IN_MONTH[nMonth];
    int nPrevDays = 0;
    for( int i = 0; i < nMonth; i++ )
    {
        nPrevDays += DAYS_IN_MONTH[i];
    }
    return nDay - nPrevDays;
}

/**
 * \brief Find the month that a julian day resides in.
 *
 * \param nDay julian day
 * \return index for month of the year, zero based
 */
int MonthOfJulian( int nDay )
{
    int i = DAYS_IN_MONTH[0];
    int month = 0;
    while( i < nDay )
        i += DAYS_IN_MONTH[month++ + 1];
    return month;
}

/**
 * \brief Convert hours from midnight to packed hours minutes
 *
 * Packed hours minutes in format HHMM
 *
 * \todo Add tests
 *
 * \param dfHour hours from midnight
 * \return packed hours/minutes
 */
int HourToClock( double dfHour )
{
    int nHour = (int)dfHour;
    int nMinute = (int)( ( dfHour - nHour ) * 60.0 );
    if( nHour >= 24 )
        nHour -= 24;
    return nHour * 100 + nMinute;
}

/**
 * \brief Get the sunrise time for a point
 *
 * \param nDay julian day of the year
 * \param nYear year, overridden to 2012 currently
 * \param dfX X coordinate
 * \param dfY Y coordinate
 * \param pszWkt wkt representation of the point
 * \return integer representation of time, ie: 1859 for 6:59 PM in UTC
 */
int GetSunRise( int nDay, int nYear, double dfX, double dfY,
                   const char *pszWkt )
{
    nYear = 2012;
    ReprojectPoint( &dfX, &dfY, pszWkt );
    if( nDay < 365 )
        nDay = nDay % 365;
    int nMonth = MonthOfJulian( nDay );
    nDay = DayOfMonth( nDay );
    double dfSunRise, dfSunSet;
    sun_rise_set( nYear, nMonth, nDay, dfX, dfY, &dfSunRise, &dfSunSet );
    return HourToClock( dfSunRise );
}

/**
 * \brief Get the sunset time for a point
 *
 * \param nDay julian day of the year
 * \param nYear year, overridden to 2012 currently
 * \param dfX X coordinate
 * \param dfY Y coordinate
 * \param pszWkt wkt representation of the point
 * \return integer representation of time, ie: 1859 for 6:59 PM in UTC
 */
int GetSunSet( int nDay, int nYear, double dfX, double dfY,
               const char *pszWkt )
{
    nYear = 2012;
    ReprojectPoint( &dfX, &dfY, pszWkt );
    if( nDay < 365 )
        nDay = nDay % 365;
    int nMonth = MonthOfJulian( nDay );
    nDay = DayOfMonth( nDay );
    double dfSunRise, dfSunSet;
    sun_rise_set( nYear, nMonth, nDay, dfX, dfY, &dfSunRise, &dfSunSet );
    return HourToClock( dfSunSet );
}

/**
 * \brief Fetch string representation of attack method
 *
 * \param nMethod method index
 * \return string description of the method
 */
const char * AttackMethod( int nMethod )
{
    if( nMethod == 0 )
        return "HEAD";
    else if( nMethod == 1 )
        return "TAIL";
    else
        return "PARALLEL";
}

/**
 * \brief Fetch the pixel/line coordinates for a given projected coordinate
 *
 * \param hDS handle to a dataset
 * \param dfGeoX x coordinate
 * \param dfGeoY y coordinate
 * \param pszWkt projection reference string in wkt, if NULL assume coords
 *               are in native srs
 * \param [out] nPixel pixel index to be set
 * \param [out] nLine line index to be set
 * \return 0 on success, non zero otherwise
 */

int PixelIndex( GDALDatasetH hDS, double dfGeoX, double dfGeoY,
                const char *pszWkt, int *nPixel, int *nLine )
{
    const char *pszDstWkt;
    double dfNewX, dfNewY, dfDummy;
    dfNewX = (double)dfGeoX;
    dfNewY = (double)dfGeoY;

    pszDstWkt = GDALGetProjectionRef( hDS );
    int err = ReprojectPoint( &dfNewX, &dfNewY, pszWkt, pszDstWkt );
    if( err != 0 )
    {
        fprintf( stderr, "Could not reproject point\n" );
        return -1;
    }

    double adfGeoTransform[6], adfInvGeoTransform[6];

    if( GDALGetGeoTransform( hDS, adfGeoTransform ) != CE_None )
    {
        fprintf( stderr, "Cannot fetch Geotransform" );
        return -4;
    }

    int rc = GDALInvGeoTransform( adfGeoTransform, adfInvGeoTransform );

    *nPixel = (int)( adfInvGeoTransform[0] + adfInvGeoTransform[1] *
                     dfNewX + adfInvGeoTransform[2] * dfNewY );
    *nLine = (int)( adfInvGeoTransform[3] + adfInvGeoTransform[4] *
                    dfNewX + adfInvGeoTransform[5] * dfNewY );
    return rc;
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
int PixelValue ( GDALDatasetH hDS, int band, double dfGeoX, double dfGeoY,
                 double *dfValue, const char *pszWkt )
{
    GDALRasterBandH hBand;
    int e;
    int nPixel, nLine;
    e = PixelIndex( hDS, dfGeoX, dfGeoY, pszWkt, &nPixel, &nLine );
    if( nPixel < 0 || nLine < 0 || nPixel > GDALGetRasterXSize( hDS ) ||
        nLine > GDALGetRasterYSize( hDS ) )
    {
        return 1;
    }
    hBand = GDALGetRasterBand( hDS, band );
    GDALRasterIO( hBand, GF_Read, nPixel, nLine, 1, 1, dfValue, 1, 1, 
                  GDT_Float64, 0, 0 );
    return 0;
}

/**
 * \brief find the upper left corner of a pixel given a pixel and line.
 *
 * \param hDS GDAL dataset to interrogate
 * \param nPixel pixel offset
 * \param nLine line offset
 * \param [out] dfX X coordinate to be set
 * \param [out] dfY Y coordinate to be set
 * \pszDstWkt wkt representation if the point is to be reprojected, NULL for
 *            native srs
 * \return 0 on success, non-zero otherwise
 */
int PixelOrigin( GDALDatasetH hDS, int nPixel, int nLine, double *dfX,
                 double *dfY, const char *pszDstWkt )
{
    double adfGeoTransform[6];
    int err = GDALGetGeoTransform( hDS, adfGeoTransform );
    if( err != CE_None )
    {
        return -1;
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
    return 0;
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
int ReprojectPoint( double *dfX, double *dfY, const char *pszSrcWkt,
                    const char *pszDstWkt )
{
    double dfDummy;
    if(pszDstWkt == NULL)
    {
        fprintf( stderr, "Invalid projection reference" );
        return -1;
    }
    if( pszSrcWkt != NULL && !EQUAL( pszSrcWkt, pszDstWkt ) )
    {
        OGRSpatialReference oSrcSRS, oDstSRS;
        OGRCoordinateTransformation *poCT;
        oSrcSRS.importFromWkt( (char**)&pszSrcWkt );
        oSrcSRS.morphFromESRI();
        if( EQUAL( pszDstWkt, "WGS84" ) )
        {
            oDstSRS.SetWellKnownGeogCS( pszDstWkt );
        }
        else
        {
            oDstSRS.importFromWkt( (char**)&pszDstWkt );
            oDstSRS.morphFromESRI();
        }
        poCT = OGRCreateCoordinateTransformation( &oSrcSRS, &oDstSRS );
        if( poCT == NULL )
        {
            OGRCoordinateTransformation::DestroyCT( poCT );
            return -3;
        }
        poCT->Transform( 1, dfX, dfY, &dfDummy );
        OGRCoordinateTransformation::DestroyCT( poCT );
    }
    return 0;
}

/**
 * \brief return the best UTM zone in WGS-84
 *
 * Return thhe epsg utm designation for a point.  The point may come in any
 * valid projection.
 *
 * \param dfX xcoordinate
 * \param dfY yCoordinate
 * \param pszSrcWkt the wkt designation of the incoming point
 * \return integer representation of the epsg code, ie: 32612 for EPSG:32612
 */
int BestUtmZone( double dfX, double dfY, const char *pszSrcWkt )
{
    ReprojectPoint( &dfX, &dfY, pszSrcWkt, "WGS84");

    int wkid;
    int baseValue;

    // Southern hemisphere if latitude is less than 0
    if ( dfY < 0 )
    {
        baseValue = 32700;
    }
    // Otherwise, Northern hemisphere
    else
    {
        baseValue = 32600;
    }

    // Perform standard calculation lat/long --> UTM Zone WKID calculation
    // and adjust it later for special cases.
    wkid = baseValue + (int)floor( ( dfX + 186 ) / 6 );

    // Make sure longitude 180 is in zone 60
    if ( dfX == 180 )
    {
        wkid = baseValue + 60;
    }
    // Special zone for Norway
    else if ( dfY >= 56.0 && dfY < 64.0
             && dfX >= 3.0 && dfX < 12.0 )
    {
        wkid = baseValue + 32;
    }
    // Special zones for Svalbard
    else if ( dfY >= 72.0 && dfY < 84.0 )
    {
        if ( dfX >= 0.0 && dfX < 9.0 )
        {
            wkid = baseValue + 31;
        }
        else if ( dfX >= 9.0 && dfX < 21.0 )
        {
            wkid = baseValue + 33;
        }
        else if ( dfX >= 21.0 && dfX < 33.0 )
        {
            wkid = baseValue + 35;
        }
        else if ( dfX >= 33.0 && dfX < 42.0 )
        {
            wkid = baseValue + 37;
        }
    }

    return wkid;
}

#ifndef MAX_PATH
#define MAX_PATH 8192
#endif

const char * GetFullPath( const char *pszFile )
{
#ifdef WIN32
    char *pszFp;
    pszFp = _fullpath( NULL, pszFile, MAX_PATH );
    return strdup( pszFp );
#else
    char szFp[MAX_PATH]; 
    realpath( pszFile, szFp ); 
    return strdup( szFp );
#endif
}
/**
 * \brief Join a list of strings.
 *
 * Join a null terminated list of strings into one string with a delimiter.
 *
 * \param papszStrings null terminated list of strings
 * \param pszDelimiter delimiter used to join.  ',' if NULL
 * \return a null terminated string to be freed with OmffrFree
 */
const char * JoinStringList( const char * const *papszStrings,
                             const char *pszDelimiter )
{
    if( papszStrings == NULL )
        return NULL;
    if( pszDelimiter == NULL )
        pszDelimiter = ",";
    int nSize = 0;
    int i = 0;
    while( papszStrings[i] != NULL )
    {
        nSize += strlen( papszStrings[i++] );
    }
    if(nSize == 0)
        return strdup( "" );
    nSize = nSize + ((i - 1) * strlen( pszDelimiter ) );
    char *pszJoined = (char*)OmffrMalloc( sizeof( char ) * nSize + 1 );
    pszJoined[0] = '\0';
    i = 0;
    sprintf( pszJoined, "%s", papszStrings[i++] );
    char *pszTmp = pszJoined;
    while( papszStrings[i] != NULL )
    {
        pszTmp = strcat( pszTmp, pszDelimiter );
        pszTmp = strcat( pszTmp, papszStrings[i++] );
        //sprintf( pszJoined, "%s%s%s", pszJoined, pszDelimiter, papszStrings[i++] );
    }
#ifndef NDEBUG
    int rc = strlen( pszJoined );
    assert( rc == nSize );
    assert( pszJoined[nSize] == '\0' );
#endif
    return pszJoined;
}

char ** AddString( char ** papszList, const char *pszNew )
{
    if( pszNew == NULL )
        return papszList;
    int nCount = CountStrings( papszList );
    papszList = (char**)OmffrRealloc( papszList, sizeof( char * ) * (nCount + 2) );
    papszList[nCount] = strdup( pszNew );
    papszList[nCount+1] = NULL;
    return papszList;
}

int CountStrings( const char * const *papszList )
{
    if( papszList == NULL )
        return 0;
    int i = 0;
    while( papszList[i] != NULL )
        i++;
    return i;
}

void FreeStringList( char ** papszList )
{
    int i = 0;
    while( papszList[i] != NULL )
        OmffrFree( papszList[i++] );
    OmffrFree( papszList );
}

void * OmffrMalloc( size_t n )
{
    return malloc( n );
}

void * OmffrRealloc( void *p, size_t n )
{
    return realloc( p, n );
}

void * OmffrCalloc( size_t n, size_t c )
{
    return calloc( n, c );
}

void OmffrFree( void *p )
{
    free( p );
}

