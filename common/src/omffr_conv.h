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
#ifndef OMFFR_CONV_H
#define OMFFR_CONV_H

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <map>
#include <string>
#include <vector>

#include "sunriset.h"

#include "gdal_priv.h"
#include "ogr_spatialref.h"

#include "omffr_tree.h"

const int DAYS_IN_MONTH [] = { 31, 28, 31, 30,
                               31, 30, 31, 31,
                               30, 31, 30, 31 };

//const char *DAYS_OF_WEEK [] = { "sunday", "monday", "tuesday", "wednesday",
//                                "thursday", "friday", "saturday" };

const char* DayOfWeek( int nDay );
int DayOfMonth( int nDay );
int MonthOfJulian( int nDay );
int HourToClock( double dfHour );
int GetSunRise( int nDay, int nYear, double dfX, double dfY,
                const char *pszWkt=NULL );
int GetSunSet( int nDay, int nYear, double dfX, double dfY,
               const char *pszWkt=NULL );
const char * AttackMethod( int nMethod );
int PixelIndex( GDALDatasetH hDS, double dfGeoX, double dfGeoY,
                const char *pszWkt, int *nPixel, int *nLine );
int PixelValue( GDALDatasetH hDS, int band, double dfGeoX, double dfGeoY,
                double *dfValue, const char *pszWkt );
int PixelOrigin( GDALDatasetH hDS, int nPixel, int nLine, double *dfX,
                 double *dfY, const char *pszDstWkt=NULL );
int ReprojectPoint( double *dfX, double *dfY, const char *pszSrcWkt,
                    const char *pszDstWkt="WGS84" );
int BestUtmZone( double dfX, double dfY, const char *pszSrcWkt );

const char * GetFullPath( const char *pszFile );

int CountStrings( const char * const *papszList );

const char * JoinStringList( const char * const *papszStrings,
                             const char *pszDelimiter );
char ** AddString( char **papszList, const char *pszNew );
void FreeStringList( char **papszList );
void * OmffrMalloc( size_t n );
void * OmffrRealloc( void *p, size_t n );
void * OmffrCalloc( size_t n, size_t c );
void OmffrFree( void * );

#endif /* OMFFR_CONV_H */

