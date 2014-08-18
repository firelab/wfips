/******************************************************************************
 *
 * $Id: frisk.h 340 2013-04-10 15:13:13Z kyle.shannon $
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Class to hold Fire Risk information
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

#ifndef FRISK_H_
#define FRISK_H_

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <ctype.h>
#include <iostream>
#include <fstream>

#include "omffr_rand.h"
#include "cpl_error.h"

#define N_DAYS   365
#define N_PERC   100
#define N_MONTHS 12
#define N_DIRS   8
#define N_SPDS   6

#define BUF_SIZE 512

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define TIME_SERIES_ID "time series data"
#define PERCENTILE_ID  "percentiles"
#define WIND_DATA_ID   "wind speed vs dir"

#define FRISK_SIZE 10128

using namespace std;
class FRisk
{

public:
    enum FILE_MODE { ASCII, BINARY };
    FRisk();
    FRisk( const char *pszFilename, FILE_MODE mode );
    FRisk( const FRisk &rhs );
    ~FRisk();

    int Read( const char *pszFilename, FILE_MODE mode );
    int Dump( const char *pszFilename, FILE_MODE mode ) const;
    int WriteBinary(ofstream& fout) const;
    int ReadBinary(ifstream& fin);
    float GetErcAvg( int day ) const;
    void GetErcAvgArr(float * ercavg);  // Retrieve the pointer to the array
    void SetErcAvg(float * ercavg);
    float GetErcStd( int day ) const;
    void SetErcStd(float * ercstddev);
    float GetErcCur( int day ) const;
    void SetErcCur(float * erccurr);

    float GetErcPerc( float erc ) const;
    float GetErcPercP( int perc ) const;
    void SetErcPerc(float * erc_perc_arr);
    float GetOneHour( float erc ) const;
    float GetOneHourP( int perc ) const;
    void SetOneHour( float * onehr);
    float GetTenHour( float erc ) const;
    float GetTenHourP( int perc ) const;
    void SetTenHour(float * tenhr);
    float GetHundredHour( float erc ) const;
    float GetHundredHourP( int perc ) const;
    void SetHundredHour(float * hundhr);

    float GetThousandHour( float erc ) const;
    float GetThousandHourP( int perc ) const;
    void SetThousandHour(float * thoushr);
    float GetHerb( float erc ) const;
    float GetHerbP( int perc ) const;
    void SetHerb(float * herbfm);
    float GetWoody( float erc ) const;
    float GetWoodyP( int perc ) const;
    void SetWoody(float * woodyfm);
    float GetX_Thousand( float erc_perc ) const;
    float GetX_ThousandP( int perc ) const;
    void SetX1000(float * x1000);

    void SetYearCalm( float yc );
    void SetYearProb( float * yp );
    void SetMonthCalm( float * mc );
    void SetMonthProb( float * mp );
    inline float GetYearCalm() const { return year_calm; }
    inline float GetMonthCalm( int month ) const { return month_calm[month]; }
    double GetRandomWindSpeed( const int day );
    int GetRandomWind( const int day, double *spd, double *dir );
    int JulianToMonth( int day );

    /* Printing data.  Wraps Dump*() calls w/stdout */
    inline void PrintTimeSeries() const { DumpTimeSeries( stdout ); }
    inline void PrintWind() const { DumpWind( stdout ); }
    inline void PrintPercentile() const { DumpPercentile( stdout ); }
    inline void Print() const { PrintTimeSeries(); PrintPercentile();
                                PrintWind(); }

    /* Operators */
    FRisk & operator = ( const FRisk &rhs );
    bool operator == ( const FRisk &rhs );
    bool operator != ( const FRisk &rhs );

private:
    Random randomGen;
    /* Month names for Dump() */
    static const char * const MONTHS[];
    static const int DAYS_IN_MONTH[];
    /* Daily data 365 x 1 (ignore leap year) */
    float *erc_avg;
    float *erc_std;
    float *erc_cur;

    /* Percentile based data 100 x 1 */
    float *erc_perc_arr;
    float *one_hour;
    float *ten_hour;
    float *hundred_hour;
    float *thousand_hour;
    float *herb;
    float *woody;
    float *x_thousand;

    /* Wind data */
    int *directions;
    float *speeds;
    float *year_prob;
    float year_calm;
    float *month_prob;
    float *month_calm;

    int ReadAscii( const char *pszFilename );
    int ReadBinary( const char *pszFilename );
    int DumpAscii( const char *pszFilename ) const;
    int DumpBinary( const char *pszFilename ) const;

    void DumpTimeSeries( FILE *fp ) const;
    void DumpPercentile( FILE *fp ) const;
    void DumpWind( FILE *fp ) const;

    char * strip( char *src );
    char * lower( char *src );
    /* int readline( FILE *fp, char *buf ); */
    int FindPercentileIndex( float erc ) const;


public:
    int test_julian();
    int test_wind_prob();
};

#endif /* FRISK_H_ */

