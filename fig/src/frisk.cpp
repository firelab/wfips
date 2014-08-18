/******************************************************************************
 *
 * $Id: frisk.cpp 497 2014-01-15 21:40:09Z kyle.shannon $
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Structure to store fire risk summary data
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

#include "frisk.h"

const char * const FRisk::MONTHS [] = { "january", "february", "march", "april", 
                                        "may", "june", "july", "august",
                                        "september", "october", "november",
                                        "december" };

const int FRisk::DAYS_IN_MONTH [] = { 31, 28, 31, 30,
                                      31, 30, 31, 31,
                                      30, 31, 30, 31 };

FRisk::FRisk()
{
    erc_avg = new float[N_DAYS];
    erc_std = new float[N_DAYS];
    erc_cur = new float[N_DAYS];

    erc_perc_arr = new float[N_PERC];
    one_hour = new float[N_PERC];
    ten_hour = new float[N_PERC];
    hundred_hour = new float[N_PERC];
    thousand_hour = new float[N_PERC];
    herb = new float[N_PERC];
    woody = new float[N_PERC];
    x_thousand = new float[N_PERC];

    directions = new int[N_DIRS];
    speeds = new float[N_SPDS];
    year_prob = new float[N_DIRS * N_SPDS];
    year_calm = 0.0;
    month_prob = new float[N_MONTHS * N_DIRS * N_SPDS];
    month_calm = new float[N_MONTHS];

    // Load the direction and speed arrays (only used for label outptut matrices)
    float DirDivs = 360 / N_DIRS;
    float SpdDivs = 30 / N_SPDS;

    /* This is not how the original data was stored.  Have to fix */
    //for(int i=0;i < N_DIRS;i++){ directions[i] = 0 + i * DirDivs;}
    for(int i=0;i < N_DIRS;i++){ directions[i] = 45 + i * DirDivs;}
    for(int i=0;i < N_SPDS;i++){ speeds[i] = 5 + i * SpdDivs;}

}

FRisk::FRisk( const char *pszFilename, FILE_MODE mode )
{
    erc_avg = new float[N_DAYS];
    erc_std = new float[N_DAYS];
    erc_cur = new float[N_DAYS];

    erc_perc_arr = new float[N_PERC];
    one_hour = new float[N_PERC];
    ten_hour = new float[N_PERC];
    hundred_hour = new float[N_PERC];
    thousand_hour = new float[N_PERC];
    herb = new float[N_PERC];
    woody = new float[N_PERC];
    x_thousand = new float[N_PERC];

    directions = new int[N_DIRS];
    speeds = new float[N_SPDS];
    year_prob = new float[N_DIRS * N_SPDS];
    year_calm = 0.0;
    month_prob = new float[N_MONTHS * N_DIRS * N_SPDS];
    month_calm = new float[N_MONTHS];

    Read( pszFilename, mode );
}

FRisk::FRisk( const FRisk &rhs )
{
    erc_avg = new float[N_DAYS];
    memcpy( erc_avg, rhs.erc_avg, sizeof(float) * N_DAYS );
    erc_std = new float[N_DAYS];
    memcpy( erc_std, rhs.erc_std, sizeof(float) * N_DAYS );
    erc_cur = new float[N_DAYS];
    memcpy( erc_cur, rhs.erc_cur, sizeof(float) * N_DAYS );

    erc_perc_arr = new float[N_PERC];
    memcpy( erc_perc_arr, rhs.erc_perc_arr, sizeof(float) * N_PERC );
    one_hour = new float[N_PERC];
    memcpy( one_hour, rhs.one_hour, sizeof(float) * N_PERC );
    ten_hour = new float[N_PERC];
    memcpy( ten_hour, rhs.ten_hour, sizeof(float) * N_PERC );
    hundred_hour = new float[N_PERC];
    memcpy( hundred_hour, rhs.hundred_hour, sizeof(float) * N_PERC );
    thousand_hour = new float[N_PERC];
    memcpy( thousand_hour, rhs.thousand_hour, sizeof(float) * N_PERC );
    herb = new float[N_PERC];
    memcpy( herb, rhs.herb, sizeof(float) * N_PERC );
    woody = new float[N_PERC];
    memcpy( woody, rhs.woody, sizeof(float) * N_PERC );
    x_thousand = new float[N_PERC];
    memcpy( x_thousand, rhs.x_thousand, sizeof(float) * N_PERC );

    directions = new int[N_DIRS];
    memcpy( directions, rhs.directions, sizeof(float) * N_DIRS );
    speeds = new float[N_SPDS];
    memcpy( speeds, rhs.speeds, sizeof(float) * N_SPDS );
    year_prob = new float[N_DIRS * N_SPDS];
    memcpy( year_prob, rhs.year_prob, sizeof(float) * N_DIRS * N_SPDS );
    year_calm = rhs.year_calm;
    month_prob = new float[N_MONTHS * N_DIRS * N_SPDS];
    memcpy( month_prob, rhs.month_prob, sizeof(float) * N_MONTHS * N_DIRS * N_SPDS );
    month_calm = new float[N_MONTHS];
    memcpy( month_calm, rhs.month_calm, sizeof(float) * N_MONTHS );
}

FRisk::~FRisk()
{
    delete [] erc_avg;
    delete [] erc_std;
    delete [] erc_cur;
    delete [] erc_perc_arr;
    delete [] one_hour;
    delete [] ten_hour;
    delete [] hundred_hour;
    delete [] thousand_hour;
    delete [] herb;
    delete [] woody;
    delete [] x_thousand;
    delete [] directions;
    delete [] speeds;
    delete [] year_prob;
    delete [] month_prob;
    delete [] month_calm;
}

FRisk & FRisk::operator = ( const FRisk &rhs )
{
    if( &rhs != this )
    {
        //erc_avg = new float[N_DAYS];
        memcpy( erc_avg, rhs.erc_avg, sizeof(float) * N_DAYS );
        //erc_std = new float[N_DAYS];
        memcpy( erc_std, rhs.erc_std, sizeof(float) * N_DAYS );
        //erc_cur = new float[N_DAYS];
        memcpy( erc_cur, rhs.erc_cur, sizeof(float) * N_DAYS );

        //erc_perc_arr = new float[N_PERC];
        memcpy( erc_perc_arr, rhs.erc_perc_arr, sizeof(float) * N_PERC );
        //one_hour = new float[N_PERC];
        memcpy( one_hour, rhs.one_hour, sizeof(float) * N_PERC );
        //ten_hour = new float[N_PERC];
        memcpy( ten_hour, rhs.ten_hour, sizeof(float) * N_PERC );
        //hundred_hour = new float[N_PERC];
        memcpy( hundred_hour, rhs.hundred_hour, sizeof(float) * N_PERC );
        //thousand_hour = new float[N_PERC];
        memcpy( thousand_hour, rhs.thousand_hour, sizeof(float) * N_PERC );
        //herb = new float[N_PERC];
        memcpy( herb, rhs.herb, sizeof(float) * N_PERC );
        //woody = new float[N_PERC];
        memcpy( woody, rhs.woody, sizeof(float) * N_PERC );
        //x_thousand = new float[N_PERC];
        memcpy( x_thousand, rhs.x_thousand, sizeof(float) * N_PERC );

        //directions = new int[N_DIRS];
        memcpy( directions, rhs.directions, sizeof(float) * N_DIRS );
        //speeds = new float[N_SPDS];
        memcpy( speeds, rhs.speeds, sizeof(float) * N_SPDS );
        //year_prob = new float[N_DIRS * N_SPDS];
        memcpy( year_prob, rhs.year_prob, sizeof(float) * N_DIRS * N_SPDS );
        year_calm = rhs.year_calm;
        //month_prob = new float[N_MONTHS * N_DIRS * N_SPDS];
        memcpy( month_prob, rhs.month_prob, sizeof(float) * N_MONTHS * N_DIRS * N_SPDS );
        //month_calm = new float[N_MONTHS];
        memcpy( month_calm, rhs.month_calm, sizeof(float) * N_MONTHS );
    }
    return *this;
}

bool FRisk::operator == ( const FRisk &rhs )
{
    for( int i = 0; i < N_DAYS; i++ )
    {
        if( this->erc_avg[i] != rhs.erc_avg[i] ||
            this->erc_std[i] != rhs.erc_std[i] ||
            this->erc_cur[i] != rhs.erc_cur[i] )
            return false;
    }
    for( int i = 0; i < N_PERC; i++ )
    {
        if( this->erc_perc_arr[i] != rhs.erc_perc_arr[i] ||
            this->one_hour[i] != rhs.one_hour[i] ||
            this->ten_hour[i] != rhs.ten_hour[i] ||
            this->hundred_hour[i] != rhs.hundred_hour[i] ||
            this->herb[i] != rhs.herb[i] ||
            this->woody[i] != rhs.woody[i] ||
            this->x_thousand[i] != rhs.x_thousand[i] )
            return false;
    }
    for( int i = 0; i < N_DIRS; i++ )
    {
        if( this->directions[i] != rhs.directions[i] )
            return false;
    }
    for( int i = 0; i < N_SPDS; i++ )
    {
        if( this->speeds[i] != rhs.speeds[i] )
            return false;
    }
    for( int i = 0; i < N_DIRS * N_SPDS; i++ )
    {
        if( this->year_prob[i] != rhs.year_prob[i] )
            return false;
    }
    if( this->year_calm != rhs.year_calm )
        return false;

    for( int i = 0; i < N_MONTHS * N_DIRS * N_SPDS; i++ )
    {
        if( this->month_prob[i] != rhs.month_prob[i] )
            return false;
    }
    for( int i = 0; i < N_MONTHS; i++ )
    {
        if( this->month_calm[i] != rhs.month_calm[i] )
            return false;
    }
    return true;
}

bool FRisk::operator != ( const FRisk &rhs )
{
    return !( operator == ( rhs ) );
}

int FRisk::Read( const char *pszFilename, FILE_MODE mode )
{
    if( mode == ASCII )
        return ReadAscii( pszFilename );
    else if( mode == BINARY )
        return ReadBinary( pszFilename );
    else
        return -1;
}

int FRisk::Dump( const char *pszFilename, FILE_MODE mode ) const
{
    if( mode == ASCII )
        return DumpAscii( pszFilename );
    else if( mode == BINARY )
        return DumpBinary( pszFilename );
    else
        return -1;
}

int FRisk::ReadAscii( const char *pszFilename )
{
    FILE *fin;
    fin = fopen( pszFilename, "r" );
    if( fin == NULL )
    {
        fprintf( stderr, "Failed to open %s for reading.\n", pszFilename );
        return -1;
    }
    char buffer[BUF_SIZE];
    char tmp[BUF_SIZE];
    /*
     * Read one line in and check if it's 'Time Series Data'.  If not, 
     * keep going.
     */
    if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
    {
        fclose( fin );
        fprintf( stderr, "Could not find time series data\n" );
        return -1;
    }
    strip( buffer );
    while( !feof( fin ) )
    {
        if( strcmp( lower( buffer ), TIME_SERIES_ID ) == 0 )
            break;
        if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
        {
            fclose( fin );
            fprintf( stderr, "Could not find time series data\n" );
            return -1;
        }
        strip( buffer );
    }
    /* Check and see if we actually found it. */
    if( strcmp( lower( buffer ), TIME_SERIES_ID ) != 0 )
    {
        fclose( fin );
        fprintf( stderr, "Could not find time series data\n" );
        return -1;
    }

    /* Header for time series data */
    if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
    {
        fclose( fin );
        fprintf( stderr, "Could not read time series data\n" );
        return -1;
    }

    /* Start reading time series data */
    for( int i = 0; i < N_DAYS; i++ )
    {
        if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
        {
            fclose( fin );
            fprintf( stderr, "Could not read time series data\n" );
            return -1;
        }
        strip( buffer );
        sscanf( buffer, "%f %f %f %s", &erc_avg[i], &erc_std[i], &erc_cur[i],
                tmp );
    }

    /* Percentile data */
    while( !feof( fin ) )
    {
        if( strcmp( lower( buffer ), PERCENTILE_ID ) == 0 )
            break;
        if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
        {
            fclose( fin );
            fprintf( stderr, "Could not read percentile data\n" );
            return -1;
        }
        strip( buffer );
    }

    /* Make sure we found the percentile data */
    if( strcmp( lower( buffer ), PERCENTILE_ID ) != 0 )
    {
        fclose( fin );
        fprintf( stderr, "Could not find percentile data\n" );
        return -1;
    }

    /* Header data for percentile section */
    if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
    {
        fclose( fin );
        fprintf( stderr, "Could not read percentile data\n" );
        return -1;
    }
    /* Read the percentile data */
    for( int i = 0; i < 100; i++ )
    {
        if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
        {
            fclose( fin );
            fprintf( stderr, "Could not read percentile data\n" );
            return -1;
        }
        strip( buffer );
        int nVals;
        nVals = sscanf( buffer, "%f %f %f %f %f %f %f %f",
                        &erc_perc_arr[i],
                        &one_hour[i],
                        &ten_hour[i],
                        &hundred_hour[i],
                        &thousand_hour[i],
                        &herb[i],
                        &woody[i],
                        &x_thousand[i]);
    }

    /* Find the wind data */
    while( !feof( fin ) )
    {
        if( strcmp( lower( buffer ), WIND_DATA_ID ) == 0 )
            break;
        if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
        {
            fclose( fin );
            fprintf( stderr, "Could not read wind data\n" );
            return -1;
        }
        strip( buffer );
    }

    /* Make sure we found the wind data */
    if( strcmp( lower( buffer ), WIND_DATA_ID ) != 0 )
    {
        fclose( fin );
        fprintf( stderr, "Could not find wind data\n" );
        return -1;
    }

    /* Header data for wind section */
    if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
    {
        fclose( fin );
        fprintf( stderr, "Could not read wind data\n" );
        return -1;
    }

    /* Read the directions */
    if( fgets( buffer, BUF_SIZE -1, fin ) == NULL )
    {
        fclose( fin );
        fprintf( stderr, "Could not read direction data\n" );
        return -1;
    }
    strip( buffer );
    sscanf( buffer, "%s %d %d %d %d %d %d %d %d", tmp,
                                                  &directions[0],
                                                  &directions[1],
                                                  &directions[2],
                                                  &directions[3],
                                                  &directions[4],
                                                  &directions[5],
                                                  &directions[6],
                                                  &directions[7] );

    /* Read the summary wind data.  8 directions, 6 speeds */
    for( int i = 0; i < N_SPDS; i++ )
    {
        if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
        {
            fclose( fin );
            fprintf( stderr, "Could not read year wind data\n" );
            return -1;
        }
        strip( buffer );
        sscanf( buffer, "%f %f %f %f %f %f %f %f %f", &speeds[i],
                                                      &year_prob[i*N_DIRS+0],
                                                      &year_prob[i*N_DIRS+1],
                                                      &year_prob[i*N_DIRS+2],
                                                      &year_prob[i*N_DIRS+3],
                                                      &year_prob[i*N_DIRS+4],
                                                      &year_prob[i*N_DIRS+5],
                                                      &year_prob[i*N_DIRS+6],
                                                      &year_prob[i*N_DIRS+7] );
    }

    /* Get the calm threshold */
    if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
    {
        fclose( fin );
        fprintf( stderr, "Could not read year wind data\n" );
        return -1;
    }
    strip( buffer );
    sscanf( buffer, "%f", &year_calm );

    /* Read the wind data.  12 months, 8 directions, 6 speeds */
    for( int i = 0; i < N_MONTHS; i++ )
    {
        /* Header for month data (month name) */
        if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
        {
            fclose( fin );
            fprintf( stderr, "Could not read wind data\n" );
            return -1;
        }

        /* Read the directions */
        if( fgets( buffer, BUF_SIZE -1, fin ) == NULL )
        {
            fclose( fin );
            fprintf( stderr, "Could not read direction data\n" );
            return -1;
        }
        strip( buffer );
        /*
         * We don't need this, we already have it, but could check 
         * and make sure they are all the same, I am not sure if the values
         * have to be.
         */
        //sscanf( buffer, "%s %d %d %d %d %d %d %d %d", tmp,
        //                                          &directions[0],
        //                                          &directions[1],
        //                                          &directions[2],
        //                                          &directions[3],
        //                                          &directions[4],
        //                                          &directions[5],
        //                                          &directions[6],
        //                                          &directions[7] );

        for( int j = 0; j < N_SPDS; j++ )
        {
            if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
            {
                fclose( fin );
                fprintf( stderr, "Could not read percentile data\n" );
                return -1;
            }
            strip( buffer );
            /* We shouldn't need speed either, dummy */
            float pafTmp;
            sscanf( buffer, "%f %f %f %f %f %f %f %f %f",
                    &pafTmp,
                    &month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+0)],
                    &month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+1)],
                    &month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+2)],
                    &month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+3)],
                    &month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+4)],
                    &month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+5)],
                    &month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+6)],
                    &month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+7)] );
        }
        if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
        {
            fclose( fin );
            fprintf( stderr, "Could not read year wind data\n" );
            return -1;
        }
        strip( buffer );
        sscanf( buffer, "%f", &month_calm[i] );
    }
    fclose( fin );
    return 0;
}

char * FRisk::strip( char *src )
{
    for( unsigned int i = 0;i < strlen( src );i++ )
    {
        if( src[i] == '\r' || src[i] == '\n' )
        {
            src[i] = '\0';
            break;
        }
    }
    return src;
}

char * FRisk::lower( char *src )
{
    for( unsigned int i = 0; i < strlen( src ); i++ )
        src[i] = tolower( src[i] );
    return src;
}

/*
int FRisk::readline( FILE *fp, char *buf )
{
    if( fgets( buf, BUF_SIZE - 1, fp ) == NULL )
    {
        fclose( fp );
        fprintf( stderr, "Could not read percentile data\n" );
        return FALSE;
    }
    strip( buf );
    return TRUE;
}
*/

int FRisk::ReadBinary( const char *pszFilename )
{
    return 1;
}

int FRisk::DumpAscii( const char *pszFilename ) const
{
    FILE *fout = fopen( pszFilename, "w" );
    if( fout == NULL )
    {
        fprintf( stderr, "Could not open %s for writing\n", pszFilename );
        return -1;
    }
    DumpTimeSeries( fout );
    DumpPercentile( fout );
    DumpWind( fout );
    fclose( fout );
    return 0;
}

int FRisk::DumpBinary( const char *pszFilename ) const
{
    ofstream fout;
    fout.open( pszFilename, ios::binary | ios::out );
    if( fout.fail() )
    {
        fprintf( stderr, "Could not open %s for writing\n", pszFilename );
        return -1;
    }
    return WriteBinary( fout );
}

int FRisk::WriteBinary(ofstream& fout) const
{
    fout.write((char *) erc_avg, sizeof(float) * N_DAYS);
    fout.write((char *) erc_std, sizeof(float) * N_DAYS);
    fout.write((char *) erc_cur, sizeof(float) * N_DAYS);
    fout.write((char *) erc_perc_arr, sizeof(float) * N_PERC);
    fout.write((char *) one_hour,sizeof(float) * N_PERC);
    fout.write((char *) ten_hour,sizeof(float) * N_PERC);
    fout.write((char *) hundred_hour,sizeof(float) * N_PERC);
    fout.write((char *) thousand_hour,sizeof(float) * N_PERC);
    fout.write((char *) herb,sizeof(float) * N_PERC);
    fout.write((char *) woody, sizeof(float) * N_PERC);
    fout.write((char *) x_thousand,sizeof(float) * N_PERC);
    fout.write((char *) year_prob,sizeof(float) * N_DIRS * N_SPDS);
    fout.write((char *) &year_calm,sizeof(float));
    fout.write((char *) month_prob,sizeof(float) * N_MONTHS * N_DIRS * N_SPDS);
    fout.write((char *) month_calm,sizeof(float) * N_MONTHS);
    return 0;
}
int FRisk::ReadBinary(ifstream& fin)
{
    fin.read((char *) erc_avg, sizeof(float) * N_DAYS);
    fin.read((char *) erc_std, sizeof(float) * N_DAYS);
    fin.read((char *) erc_cur, sizeof(float) * N_DAYS);
    fin.read((char *) erc_perc_arr, sizeof(float) * N_PERC);
    fin.read((char *) one_hour,sizeof(float) * N_PERC);
    fin.read((char *) ten_hour,sizeof(float) * N_PERC);
    fin.read((char *) hundred_hour,sizeof(float) * N_PERC);
    fin.read((char *) thousand_hour,sizeof(float) * N_PERC);
    fin.read((char *) herb, sizeof(float) * N_PERC);
    fin.read((char *) woody,sizeof(float) * N_PERC);
    fin.read((char *) x_thousand,sizeof(float) * N_PERC);
    fin.read((char *) year_prob,sizeof(float) * N_DIRS * N_SPDS);
    fin.read((char *) &year_calm,sizeof(float));
    fin.read((char *) month_prob,sizeof(float) * N_MONTHS * N_DIRS * N_SPDS);
    fin.read((char *) month_calm,sizeof(float) * N_MONTHS);
    //FixWind();
    return 0;
}

void FRisk::DumpTimeSeries( FILE *fp ) const
{
    if( fp == NULL )
        fp = stdout;
    fprintf( fp, "Time Series Data\n" );
    fprintf( fp, "%d %d ERC_AVG ERC_STDDEV ERC_CURR\n", N_DAYS, 26 );
    for( int i = 1; i < N_DAYS+1; i++ )
    {
        fprintf( fp, "%-5.2f %-5.2f %-5.0f\n", GetErcAvg( i ), GetErcStd( i ),
                 GetErcCur( i ) );
    }
}

void FRisk::DumpPercentile( FILE *fp ) const
{
    fprintf( fp, "Percentiles\n" );
    fprintf( fp, "ERC FM1 FM10 FM100 FM1000 HERB WOODY X1000\n" );
    float erc;
    for( int i = 0; i < N_PERC; i++ )
    {
        fprintf( fp, "%-5.2f %-5.2f %-5.2f %-5.2f %-5.2f %-5.2f %-5.2f %-5.2f\n", 
                 erc_perc_arr[i], one_hour[i], ten_hour[i], hundred_hour[i],
                 thousand_hour[i], herb[i], woody[i], x_thousand[i] );
    }
}

void FRisk::DumpWind( FILE *fp ) const
{
    if( fp == NULL )
        fp = stdout;
    fprintf( fp, "Wind Speed vs Dir\n" );
    fprintf( fp, "%d %d\n", N_SPDS, N_DIRS );

    /* Year Summary */
    fprintf( fp, "speed %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d\n",
             directions[0], directions[1], directions[2], directions[3],
             directions[4], directions[5], directions[6], directions[7] );
    for( int i = 0; i < N_SPDS; i++ )
    {
        fprintf( fp, "%-5.2f %-5.2f %-5.2f %-5.2f %-5.2f %-5.2f %-5.2f " \
                     "%-5.2f %-5.2f\n",
                 speeds[i], year_prob[i*N_DIRS+0], year_prob[i*N_DIRS+1],
                 year_prob[i*N_DIRS+2], year_prob[i*N_DIRS+3],
                 year_prob[i*N_DIRS+4], year_prob[i*N_DIRS+5],
                 year_prob[i*N_DIRS+6], year_prob[i*N_DIRS+7] );
    }
    fprintf( fp, "%.2f\n", year_calm );

    /* Print the monthly data */
    for( int i = 0; i < N_MONTHS; i++ )
    {
        fprintf( fp, "%s\n", MONTHS[i] );
        fprintf( fp, "speed %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d\n",
                 directions[0], directions[1], directions[2],
                 directions[3], directions[4], directions[5],
                 directions[6], directions[7] );

        for( int j = 0; j < N_SPDS; j++ )
        {
            fprintf( fp, "%-5.2f %-5.2f %-5.2f %-5.2f %-5.2f %-5.2f %-5.2f " \
                     "%-5.2f %-5.2f\n",
                     speeds[j],
                     month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+0)],
                     month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+1)],
                     month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+2)],
                     month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+3)],
                     month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+4)],
                     month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+5)],
                     month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+6)],
                     month_prob[i*N_DIRS*N_SPDS+(j*N_DIRS+7)] );
        }
        fprintf( fp, "%-5.2f\n", month_calm[i] );
    }
}

float FRisk::GetErcAvg( int day ) const
{
    assert( day > 0 && day <= 365 );
    return erc_avg[day - 1];
}
void FRisk::GetErcAvgArr(float * ercavg)
{
    erc_avg = ercavg;
}
void FRisk::SetErcAvg( float * ercavg)
{
    memcpy( erc_avg, ercavg, sizeof(float) * N_DAYS );
}
float FRisk::GetErcStd( int day ) const
{
    assert( day > 0 && day <= 365 );
    return erc_std[day - 1];
}
void FRisk::SetErcStd( float * ercstddev)
{
    memcpy( erc_std, ercstddev, sizeof(float) * N_DAYS );
}

float FRisk::GetErcCur( int day ) const
{
    return erc_cur[day - 1];
}

void FRisk::SetErcCur( float * erccurr)
{
    memcpy( erc_cur, erccurr, sizeof(float) * N_DAYS );
}

int FRisk::FindPercentileIndex( float erc ) const
{
    /*
     * If the percentile asked for is greater than the max
     * return the max.
     */
    if( erc >= erc_perc_arr[99] )
        return 99;
    int i = 0;
    while( erc_perc_arr[i] <= ceil( erc ) )
    {
        if( i == 99 )
            break;
        i++;
    }
    /* If we are at the bounds, don't decrement */
    if( i != 99 && i != 0 )
        i--;
    assert( i >= 0 && i < 100 );
    return i;
}

float FRisk::GetErcPerc( float erc ) const
{
    int i = FindPercentileIndex( erc );
    return erc_perc_arr[i];
}

float FRisk::GetErcPercP( int perc ) const
{
    assert( perc > 0 && perc <= 100 );
    return erc_perc_arr[perc-1];
}

void FRisk::SetErcPerc(float * ercpercarr)
{
    memcpy(erc_perc_arr,ercpercarr,sizeof(float) * N_PERC);
}

float FRisk::GetOneHour( float erc ) const
{
    int i = FindPercentileIndex( erc );
    return one_hour[i];
}

float FRisk::GetOneHourP( int perc ) const
{
    assert( perc > 0 && perc <= 100 );
    return one_hour[perc-1];
}
void FRisk::SetOneHour( float * onehr)
{
    memcpy( one_hour, onehr, sizeof(float) * N_PERC );
}

float FRisk::GetTenHour( float erc ) const
{
    int i = FindPercentileIndex( erc );
    return ten_hour[i];
}

float FRisk::GetTenHourP( int perc ) const
{
    assert( perc > 0 && perc <= 100 );
    return ten_hour[perc-1];
}

void FRisk::SetTenHour( float * tenhr)
{
    memcpy( ten_hour, tenhr, sizeof(float) * N_PERC );
}

float FRisk::GetHundredHour( float erc ) const
{
    int i = FindPercentileIndex( erc );
    return hundred_hour[i];
}

float FRisk::GetHundredHourP( int perc ) const
{
    assert( perc > 0 && perc <= 100 );
    return hundred_hour[perc-1];
}

void FRisk::SetHundredHour( float * hundhr)
{
    memcpy( hundred_hour, hundhr, sizeof(float) * N_PERC );
}

float FRisk::GetThousandHour( float erc ) const
{
    int i = FindPercentileIndex( erc );
    return thousand_hour[i];
}

float FRisk::GetThousandHourP( int perc ) const
{
    assert( perc > 0 && perc <= 100 );
    return thousand_hour[perc-1];
}

void FRisk::SetThousandHour( float * thoushr)
{
    memcpy( thousand_hour, thoushr, sizeof(float) * N_PERC );
}
float FRisk::GetHerb( float erc ) const
{
    int i = FindPercentileIndex( erc );
    return herb[i];
}

float FRisk::GetHerbP( int perc ) const
{
    assert( perc > 0 && perc <= 100 );
    return herb[perc-1];
}

void FRisk::SetHerb( float * herbfm)
{
    memcpy( herb, herbfm, sizeof(float) * N_PERC );
}
float FRisk::GetWoody( float erc ) const
{
    int i = FindPercentileIndex( erc );
    return woody[i];
}

float FRisk::GetWoodyP( int perc ) const
{
    assert( perc > 0 && perc <= 100 );
    return woody[perc-1];
}

void FRisk::SetWoody( float * woodyfm)
{
    memcpy( woody, woodyfm, sizeof(float) * N_PERC );
}

float FRisk::GetX_Thousand( float erc ) const
{
    int i = FindPercentileIndex( erc );
    return x_thousand[i];
}

float FRisk::GetX_ThousandP( int perc ) const
{
    assert( perc > 0 && perc <= 100 );
    return x_thousand[perc-1];
}

void FRisk::SetX1000( float * x1000)
{
    memcpy( x_thousand, x1000, sizeof(float) * N_PERC );
}

void FRisk::SetYearCalm( float yc )
{
    year_calm = yc;
}

void FRisk::SetYearProb(float * yp)
{
    memcpy(year_prob,yp,sizeof(float) * N_DIRS * N_SPDS);

}

void FRisk::SetMonthCalm(float * mc)
{
    memcpy(month_calm,mc,sizeof(float) * N_MONTHS);
}

void FRisk::SetMonthProb(float * mp)
{
    memcpy(month_prob,mp,sizeof(float) * N_MONTHS * N_DIRS * N_SPDS);

}

/**
 * Randomly select a wind speed for a day.
 *
 * Convenience wrapper for GetRandomWind()
 *
 * @see GetRandomWind
 *
 * @param day julian day
 * @return a randomly chosen wind speed.
 */
double FRisk::GetRandomWindSpeed( const int day )
{
    double s, d;
    GetRandomWind( day, &s, &d );
    return s;
}

/**
 * Select a random wind speed and direction for a given day.
 *
 * A random number is generated and compared against the 
 * cumulative sum of the probability table.  The wind speed
 * and direction is then extracted from the 'bin' the probability 
 * falls in.
 *
 * @param day julian day to map to a month
 * @param spd windspeed to be assigned
 * @param dir wind direction to be assigned
 * @return 0 on success
 */
int FRisk::GetRandomWind( const int day, double *spd, double *dir )
{
    assert( day > 0 && day <= 365 );
    double prob = randomGen.rand3();
    int month = JulianToMonth( day );
    double sum = GetMonthCalm( month );

    int i = 0;
    while( sum < prob * 100 && i < N_SPDS * N_DIRS )
    {
        sum += month_prob[(N_SPDS * N_DIRS * month) + i++];
    }
    /* Just stayed calm, didn't loop at all */
    if( i == 0 )
    {
        *spd = 0.0;
        *dir = 45.0;
    }
    else
    {
        /* We went one passed our bin, 'rewind' one iteration */
        i--;
        *spd = (double)speeds[i / N_DIRS];
        if( i < N_DIRS )
            *dir = (double)directions[i];
        else
            *dir = (double)directions[i % N_DIRS];
    }
    //CPLDebug( "FRisk::GetRandomWind", 
    //          "speed:%f,dir:%f,prob:%lf,index:%d,month:%d", 
    //          *spd, *dir, prob, i, month );
    return 0;
}

/**
 * Return month index for a given julian day, zero based. Ignores leap year.
 *
 * @param day julian day
 * @return month of the year 0-11
 */
int FRisk::JulianToMonth( const int day )
{
    //assert( day > 0 && day <= 365 );
    if( day > 365 || day < 1 )
    {
        fprintf( stderr, "Julian day %d is not valid\n", day );
        return -1;
    }
    int i = DAYS_IN_MONTH[0];
    int month = 0;
    while( i < day )
        i += DAYS_IN_MONTH[month++ + 1];
    return month;
}

/**
 * Run the self tests
 */

/**
 * Simple test for julian to month conversion, ignore leap year
 */
int FRisk::test_julian()
{
    int i;
    /* First day of months */
    i = JulianToMonth( 1 );
    assert( i == 0 );
    i = JulianToMonth( 32 );
    assert( i == 1 );
    i = JulianToMonth( 60 );
    assert( i == 2 );
    i = JulianToMonth( 91 );
    assert( i == 3 );
    i = JulianToMonth( 121 );
    assert( i == 4 );
    i = JulianToMonth( 152 );
    assert( i == 5 );
    i = JulianToMonth( 182 );
    assert( i == 6 );
    i = JulianToMonth( 213 );
    assert( i == 7 );
    i = JulianToMonth( 244 );
    assert( i == 8 );
    i = JulianToMonth( 274 );
    assert( i == 9 );
    i = JulianToMonth( 305 );
    assert( i == 10 );
    i = JulianToMonth( 335 );
    assert( i == 11 );
    /* Last day of months */
    i = JulianToMonth( 31 );
    assert( i == 0 );
    i = JulianToMonth( 59 );
    assert( i == 1 );
    i = JulianToMonth( 90 );
    assert( i == 2 );
    i = JulianToMonth( 120 );
    assert( i == 3 );
    i = JulianToMonth( 151 );
    assert( i == 4 );
    i = JulianToMonth( 181 );
    assert( i == 5 );
    i = JulianToMonth( 212 );
    assert( i == 6 );
    i = JulianToMonth( 243 );
    assert( i == 7 );
    i = JulianToMonth( 273 );
    assert( i == 8 );
    i = JulianToMonth( 304 );
    assert( i == 9 );
    i = JulianToMonth( 334 );
    assert( i == 10 );
    i = JulianToMonth( 365 );
    assert( i == 11 );
    /* Invalid */
    i = JulianToMonth( 366 );
    assert( i == -1 );
    i = JulianToMonth( 0 );
    assert( i == -1 );
    i = JulianToMonth( -1 );
    assert( i == -1 );
    return 0;
}
/**
 * Test to see if binary data is read in correctly and the cumulative sum
 * of the wind probablilities approaches 100.
 */
int FRisk::test_wind_prob()
{
    double sum = 0;
    for( int i = 0; i < N_SPDS * N_DIRS; i++ )
    {
        sum += year_prob[i];
    }

    float delta = fabs(( sum + 0.5 ) - 100);
    return   delta < 2.0 ? 0 : 1;
}

