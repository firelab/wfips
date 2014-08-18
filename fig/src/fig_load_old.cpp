/******************************************************************************
 *
 * $Id: fig_load_old.cpp 79 2012-07-26 15:36:43Z kyle.shannon $
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Load a sqlite table with output from Isaac
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "sqlite3.h"

#include "gdal_priv.h"

#include "omffr_conv.h"
#include "frisk.h"
#include "nfdr32.h"
#include "omffr_conv.h"

#define BUF_SIZE 512

#define FUEL_MODEL_G 71

void Usage()
{
    printf( "Usage: fig_load [-d] [-p] [-db database] [-t table]\n" );
    printf( "                [-f out_text_file] input_file\n"       );
    printf( "\n"                                                    );
    printf( "                *Use text file for speed/debug*\n"     );
    exit( 1 );
}

/**
 * Read in output from Isaac and populate a sqlite db with the read in values
 * as well as calculated values using erc.  Calculated values include:
 */
int main( int argc, char *argv[] )
{
    GDALAllRegister();
    const char *pszInputfile = NULL;
    const char *pszOutputDB = NULL;
    const char *pszTableName = NULL;
    const char *pszOutputfile = NULL;
    int bDrop = FALSE;
    int bProgress = TRUE;

    int i = 1;
    while( i < argc )
    {
        if( EQUAL( argv[i], "-d" ) )
        {
            bDrop = TRUE;
        }
        else if( EQUAL( argv[i], "-p" ) )
        {
            bProgress = TRUE;
        }
        else if( EQUAL( argv[i], "-db" ) )
        {
            pszOutputDB = argv[++i];
        }
        else if( EQUAL( argv[i], "-p" ) )
        {
            pszTableName = argv[++i];
        }
        else if( EQUAL( argv[i], "-f" ) )
        {
            pszOutputfile = argv[++i];
        }
        else if( EQUAL( argv[i], "-h" ) )
        {
            Usage();
        }
        else if( pszInputfile == NULL )
        {
            pszInputfile = argv[i];
        }
        else
        {
            Usage();
        }
        i++;
    }

    if( pszInputfile == NULL )
    {
        fprintf( stderr, "No input file provided\n");
        Usage();
    }
    if( pszOutputfile == NULL && 
        ( pszOutputDB == NULL || pszTableName == NULL ) )
    {
        fprintf( stderr, "Invalid output selected, use database and table or "
                         "output file\n" );
    }
    // Setup outputs
    int bDbOut = FALSE;
    int bTextOut = FALSE;
    if( pszOutputfile != NULL )
    {
        bTextOut = TRUE;
    }
    if( pszOutputDB != NULL && pszTableName != NULL )
    {
        bDbOut = TRUE;
    }

    sqlite3 *db;
    char *zErr;
    int rc;
    char sql[BUF_SIZE];

    if( bDbOut )
    {
        rc = sqlite3_open( pszOutputDB, &db );

        if ( rc )
        {
            fprintf( stderr, "Can't open database: %s\n", sqlite3_errmsg( db ) );
            sqlite3_close( db );
            exit( 1 );
        }

        if( bDrop )
        {
            sprintf( sql, "drop table %s", pszTableName );

            rc = sqlite3_exec( db, sql, NULL, NULL, &zErr );

            if ( rc != SQLITE_OK )
            {
                if ( zErr != NULL )
                {
                    fprintf( stderr, "SQL error: %s\n", zErr );
                    sqlite3_free( zErr );
                }
            }
        }

        sprintf( sql, "create table %s ( day integer, year integer, x float, "
                      "y float, cause int, time int, erc float, bi float, ros float, "
                      "fuel int)", pszTableName );

        rc = sqlite3_exec( db, sql, NULL, NULL, &zErr );

        if ( rc != SQLITE_OK )
        {
            if ( zErr != NULL )
            {
                fprintf( stderr, "SQL error: %s\n", zErr );
                sqlite3_free( zErr );
            }
            exit( 1 );
        }
    }
    FILE *fout;
    if( bTextOut )
    {
        fout = fopen( pszOutputfile, "w" );
    }
    if( fout == NULL )
    {
        fprintf( stderr, "Could not open %s for writing\n", pszOutputfile );
        exit( 1 );
    }

    FILE *fin;
    char buffer[BUF_SIZE];
    fin = fopen( pszInputfile, "r" );
    if( fin == NULL )
    {
        fprintf( stderr, "Could not open %s for reading\n", pszInputfile );
        exit( 1 );
    }
    int nRecords = 0;
    if( bProgress )
    {
        while( fgets( buffer, BUF_SIZE - 1, fin ) )
            nRecords++;
        fseek( fin, 0, SEEK_SET );
    }
    /*
     * Isaac's fields:
     * day year lat long cause time erc
     */
    /* Skip header */
    if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
    {
        fprintf( stderr, "Could not read data from file" );
        exit( 1 );
    }

        //int iNFDRSVersion = 78,iClimateClass = 1,iJulianLookFreeze = 365,iJulianGreenUp = 1,iLat = (int) 45,iHerbAnnual=0,iWoodyDeciduous=0,iOneIsTen=0,iStartKBDI=800;
    //double fStart100 = 20.0,fStart1000=20.0,fAvgPrecip = 45.0;
    //double fMC1 = 3,fMC10 = 4,fMC100 = 5,fMC1000 = 10,fMCWood = 120,fMCHerb = 120;
    //int iRainEvent = 0, iKBDI = 800,iGreenWoody = 20,iGreenHerb = 20,iSeason = 2,iSOW = 0;

    // Initialize the NFDRS calculator
    //nfdrs.iInitialize(iNFDRSVersion,iClimateClass,iJulianLookFreeze,
    //                  iJulianGreenUp,iLat,iHerbAnnual,
    //                  iWoodyDeciduous,iOneIsTen,
    //                  fStart100,fStart1000, 
    //                  iStartKBDI,fAvgPrecip);
    //double fROS=0,fERC=0,fFL=0;
    //int iSC=0,iFIL=0,iBI=0,iWS=0,iSlopeCls=1;

    NFDRCalc nfdr;
    nfdr.iSetFuelModel( FUEL_MODEL_G, 0 );
    nfdr.SetVersion( 78 );

    double one_hour, ten_hour, hundred_hour, thousand_hour, woody, herb;
    /* What we actually get */
    double nfdr_erc;
    int nfdr_bi, nfdr_sc;;
    double dfDummy;
    int nDummy;

    GDALDatasetH hFuelDS;
    hFuelDS = GDALOpen( "/home/kyle/src/omfrr/trunk/data/"
                        "small_fuel40.tif", GA_ReadOnly );
    if( hFuelDS == NULL )
    {
        fprintf( stderr, "Cannot open fuels data for query.\n" );
        exit( 1 );
    }
    GDALDatasetH hSlopeDS;
    hSlopeDS = GDALOpen( "/home/kyle/src/omfrr/trunk/data/"
                         "small_slope.tif", GA_ReadOnly );
    if( hSlopeDS == NULL )
    {
        fprintf( stderr, "Cannot open slope data for query.\n" );
        exit( 1 );
    }
    GDALDatasetH hNarrMaskDS;
    hNarrMaskDS = GDALOpen( "/home/kyle/src/omfrr/trunk/data/"
                            "narr_mask_byte.tif", GA_ReadOnly );
    if( hNarrMaskDS == NULL )
    {
        fprintf( stderr, "Cannot open NARR mask for query.\n" );
        exit( 1 );
    }
    int nMaskXSize = GDALGetRasterXSize( hNarrMaskDS );
    int nMaskYSize = GDALGetRasterYSize( hNarrMaskDS );

    const char *pszAlbersWkt =
        "PROJCS[\"USA_Contiguous_Albers_Equal_Area_Conic_USGS_version\","
        "GEOGCS[\"GCS_North_American_1983\",DATUM[\"D_North_American_1983\","
        "SPHEROID[\"GRS_1980\",6378137.0,298.257222101]],"
        "PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],"
        "PROJECTION[\"Albers\"],PARAMETER[\"False_Easting\",0.0],"
        "PARAMETER[\"False_Northing\",0.0],"
        "PARAMETER[\"Central_Meridian\",-96.0],"
        "PARAMETER[\"Standard_Parallel_1\",29.5],"
        "PARAMETER[\"Standard_Parallel_2\",45.5],"
        "PARAMETER[\"Latitude_Of_Origin\",23.0],UNIT[\"Meter\",1.0]]";
    /* Assume same srs */
    //const char *pszAlbersWkt = NULL;

    char out_buf[2048];
    int day, year, cause, time;
    double x, y, erc;
    int fuel, mask, slope, slope_class;
    ifstream frisk_stream( "/home/kyle/src/omfrr/trunk/data/"
                           "narr_32km_frisk.flt", ios::binary );

    FRisk oFRisk;
    int skip = 0;
    int done = 0;
    int wind_speed;
    long offset;
    int nPixel;
    int nLine;
    if( bProgress )
    {
        GDALTermProgress( 0.0, NULL, NULL );
    }
    while( fgets( buffer, BUF_SIZE - 1, fin ) ) 
    {
        //frisk_stream.seekg( 0, ios::beg );
        done++;
        /* Note that it *appears* that isaac writes x,y and not lat,lon */
        sscanf( buffer, "%d %d %lf %lf %d %d %lf", &day, &year, &x, &y,
                &cause, &time, &erc );

        /* Calculate fire behavior indices based on fire risk data.
         * Load all into the sqlite db.  We need:
         * | BI | ROS | ERC | Flame Length | Fuel Model |
         * ROS and Flame length need to be calculated with rothermel.
         * BI, ERC and SC are calculated with nfdrs and fuel model G
         */

        /* Get the slope and find the class */
        slope = (int)PixelValue( hSlopeDS, 1, x, y, pszAlbersWkt );
        if( slope <= 25 )
            slope_class = 1;
        else if( slope <= 40 )
            slope_class = 2;
        else if( slope <= 55 )
            slope_class = 3;
        else if( slope <= 75 )
            slope_class = 4;
        else
            slope_class = 5;

        /* Check the mask, if we don't have data, skip it and emit a
         * warning
         */

        mask = (int)PixelValue( hNarrMaskDS, 1, x, y, pszAlbersWkt );
        if( !mask )
        {
            fprintf( stderr, "NARR data not available, skipping record\n" );
            skip++;
            continue;
        }
        /* NFDRS Stuff */

        PixelIndex(hNarrMaskDS, x, y, pszAlbersWkt, &nPixel, &nLine);
        if( nPixel >= nMaskXSize || nLine >= nMaskYSize )
        {
            CPLDebug( "fig_load", "Invalid pixel x:%d y:%d", nPixel, nLine );
            continue;
        }

        /* We should always have data for our query */
        assert( mask );
        assert( nPixel < nMaskXSize );
        assert( nLine < nMaskYSize );

        offset = nPixel + nLine * nMaskXSize;
        frisk_stream.seekg( offset * FRISK_SIZE, ios::beg );
        oFRisk.ReadBinary( frisk_stream );

        wind_speed = (int)oFRisk.GetRandomWindSpeed( day );
        one_hour = (double)oFRisk.GetOneHour( erc );
        ten_hour = (double)oFRisk.GetTenHour( erc );
        hundred_hour = (double)oFRisk.GetHundredHour( erc );
        thousand_hour = (double)oFRisk.GetThousandHour( erc );
        woody = (double)oFRisk.GetWoody( erc );
        herb = (double)oFRisk.GetHerb( erc );

        nfdr.iSetMoistures( one_hour, ten_hour, hundred_hour, thousand_hour,
                            woody, herb, nDummy, nDummy, nDummy, nDummy,
                            nDummy, nDummy );

        nfdr_erc = nfdr_bi = nfdr_sc = 0.0;
        nfdr.iCalcIndexes( wind_speed, slope_class, &dfDummy, &nfdr_sc, &nfdr_erc,
                           &dfDummy, &nDummy, &nfdr_bi );

        /* Look up fuel model for fb calculations, *not* nfdr */
        fuel = (int)PixelValue( hFuelDS, 1, x, y, pszAlbersWkt );

        if( bDbOut )
        {
            sprintf( sql, "insert into %s values(%d, %d, %lf, %lf, %d, %d,"
                          "%lf, %d, %lf, %d)",
                     pszTableName, day, year, x, y, cause, time, erc, nfdr_bi,
                     erc, fuel);

            rc = sqlite3_exec( db, sql, NULL, NULL, &zErr );

            if ( rc != SQLITE_OK )
            {
                if ( zErr != NULL )
                {
                    fprintf( stderr, "SQL error: %s\n", zErr );
                    sqlite3_free( zErr );
                    break;
                }
            }
        }
        if( bTextOut )
        {
            sprintf( out_buf, "%d %d %lf %lf %d %d %lf %d %lf %d\n",
                     day, year, x, y, cause, time, erc, nfdr_bi, erc, fuel);
            fputs( out_buf, fout );
        }

        if( bProgress )
        {
            double percent = (float)done / (float) nRecords;
            GDALTermProgress( percent, NULL, NULL );
        }
    }
    if( bProgress )
    {
        GDALTermProgress( 1.0, NULL, NULL );
    }

    if( skip )
    {
        fprintf( stderr, "Skipped %d records\n", skip );
    }

    fclose( fin );
    if( bDbOut )
    {
        sqlite3_close( db );
    }
    if( bTextOut )
    {
        fclose( fout );
    }

    return 0;
}

