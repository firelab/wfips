/******************************************************************************
 *
 * $Id: fig.cpp 593 2014-05-12 21:45:58Z kyle.shannon $
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

#include "fig.h"
/**
 * \file fig.h
 * \brief C++ class for Fire Ignition Generator
 * \date 2012-07-30
 * \author Kyle Shannon <kyle@pobox.com>
 * \since 1.0
 */


/**
 * \brief Create and set default values for the FIG
 */
FIGGenerator::FIGGenerator()
{
    Initialize();
}

/**
 * \brief Constructor that may take some options.
 *
 * \param papszOptions list of name value options, not used currently
 */
FIGGenerator::FIGGenerator( const char * const * papszOptions )
{
    Initialize();
}

/**
 * \brief Copy Constructor
 *
 */
FIGGenerator::FIGGenerator( FIGGenerator &rhs )
{
}

/**
 * FIXME: Need to delete our crap, handle all new and malloc?
 */
FIGGenerator::~FIGGenerator()
{
    /*
    if( pszInputWkt )
        CPLFree( (void*)pszInputWkt );
    if( hPoint )
        OGR_G_DestroyGeometry( hPoint );
    */
}

/**
 * \brief Initialize a FIG object, used to minimize duplicate code
 *
 */
void FIGGenerator::Initialize()
{
    pszAspectFilename = ASPECT_FILE;
    pszBaseFilename = BASE_FILE;
    pszCoverFilename = COVER_FILE;
    pszDensityFilename = DENSITY_FILE;
    pszElevFilename = ELEVATION_FILE;
    pszFuelFilename = FUEL_FILE;
    pszHeightFilename = HEIGHT_FILE;
    pszSlopeFilename = SLOPE_FILE;
    pszNarrMaskFilename = MASK_FILE;
    pszFriskBinaryFilename = FRISK_BINARY;
    pszFwaShpFilename = FWA_SHAPE;
    pszFpuShpFilename = FPU_SHAPE;
    pszFwaRasterFilename = FWA_RASTER;
    pszFwaMaskFilename = FWA_FTR_MASK;
    pszIgnProbFilename = IGN_MASK;
    pszFuelPercFilename = FUEL_PERC;
    //pszInputWkt = ALBERS_WKT;
    //pszInputWkt = NAD83_WKT;
    pszInputWkt = NARR_WKT;

    pszDataPath = getenv( "OMFFR_DATA" );
    bOverwrite = FALSE;
    bShowProgress = FALSE;
    pszFpuCode = NULL;

    int nMaxPlaceAttempts;
    int nMaxRetryAttempts;
    double dfDefaultProb;

    pszLastErrorMessage = "";
    nLastErrorCode = FE_NONE;

    nfdr.iSetFuelModel( FUEL_MODEL_G, 0 );
    nfdr.SetVersion( 78 );

    ResetNewFuels(); /* ??? */
    flammap.SetResolution(-1.0);
    SetFoliarMoistureContent(100);
    SetCrownFireCalculation(0);
    EnableSpotting(true);
    flammap.burn->pointBurnThread = new BurnThread(flammap.burn->env);

    ePointSearch = PIP_RASTER;
    bReadInZip = TRUE;

    pszFormat = "ESRI Shapefile";
    //pszFormat = "csv";
    bEnvSet = FALSE;

    nSqliteTransactions = 1024;
}

/**
 * \brief Read in a text file, calculate fb and nfdr values, write a text file.
 *
 * \param pszInputFilename input file name
 * \param pszOutputFilename output file name
 * \param papszOptions list of name value pairs, currently not used
 * \return FE_NONE on success, error otherwise
 */

FE_Err
FIGGenerator::GenerateFromFile( const char *pszInputFilename,
                                const char *pszOutputFilename,
                                const char * const * papszOptions )
{
    /* Parse options */
    const char *pszTreatedPath;
    if( papszOptions != NULL )
    {
        pszTreatedPath = CSLFetchNameValue( (char**)papszOptions, "TREATMENT_PATH" );
        if( pszTreatedPath == NULL )
        {
            bAddTreated = FALSE;
        }
        else
            bAddTreated = TRUE;
    }
    this->pszInputFilename = pszInputFilename;
    this->pszOutputFilename = pszOutputFilename;

    CPLSetConfigOption( "OGR_SQLITE_CACHE", "512" );

    FE_Err err = FE_NONE;
    err = InitializeFiles();
    if( err != FE_NONE )
        return err;
    FILE *treated_out;
    if( bAddTreated )
    {
        err = InitializeTreated( pszTreatedPath );
        const char *pszTmp = CPLSPrintf( "%s-treated.txt", pszOutputFilename);
        treated_out = fopen( pszTmp, "w" );
        if( treated_out == NULL )
            bAddTreated = FALSE;
    }

    if( err != FE_NONE )
        return err;

    //CreateRasterAccess();

    char buffer[BUF_SIZE];
    int nRecords = 0;
    if( bShowProgress )
    {
        while( fgets( buffer, BUF_SIZE - 1, fin ) )
            nRecords++;
        fseek( fin, 0, SEEK_SET );
    }
    /*
    ** Skip header.  We now have two header lines (02/03/2014).
    */
    if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
    {
        CPLError( CE_Failure, CPLE_OpenFailed,
                "Could not read data from file" );
        exit( 1 );
    }
    if( fgets( buffer, BUF_SIZE - 1, fin ) == NULL )
    {
        CPLError( CE_Failure, CPLE_OpenFailed,
                "Could not read data from file" );
        exit( 1 );
    }
    int skip = 0;
    int done = 0;
    if( bShowProgress )
    {
        GDALTermProgress( 0.0, NULL, NULL );
    }
    nFireNum = 0;
    if( nSqliteTransactions )
        OGR_L_StartTransaction( hOutputLayer );
    /* Loop over the records */
    double dfTempX, dfTempY;
    int rc = 0;
	double dfTreatX, dfTreatY;
    char abyCrap[512];
    while( fgets( buffer, BUF_SIZE - 1, fin ) ) 
    {
        if( buffer == NULL || buffer[0] == NULL )
        {
            continue;
        }
        /* Isaac's: year, day, x, y, cause, time, erc, erc percentile */
        /* Note that it *appears* that isaac writes x,y and not lat,lon */
        /* Isaac changed (20130320) the output, it is now:
           year, day, x, y, time, cause, erc percentile */
        /* old
        sscanf( buffer, "%d %d %lf %lf %d %d %lf", &nYear, &nJulDay, &dfX,
                &dfY, &nCause, &nDiscTime, &dfErc );
        */
        /*
        ** Isaac changed again, added fire id(in quotes) to the file.  As of 20140203,
        ** the format is now: id, year, day, x, y, time, cause, erc percentile.
        */
        /* older, but not oldest
        rc = sscanf( buffer, "%d %d %lf %lf %d %d %d", &nYear, &nJulDay, &dfX,
                     &dfY, &nDiscTime, &nCause, &nErc );
        */
        rc = sscanf( buffer, "%s %d %d %lf %lf %d %d %d", &abyCrap, &nYear,
                     &nJulDay, &dfX, &dfY, &nDiscTime, &nCause, &nErc );
        if( rc == 0 )
        {
            continue;
        }
        /* Check for envelope filter */
        if( bEnvSet )
        {
            dfTempX = dfX;
            dfTempY = dfY;
            //ReprojectPoint( &dfTempX, &dfTempY, pszInputWkt, NAD83_WKT );
            if( dfTempX > dfMaxX || dfTempX < dfMinX ||
                dfTempY > dfMaxY || dfTempY < dfMinY )
            {
                done++;
                continue;
            }
        }

        /* Calculate fire behavior indices based on fire risk data.
         * Load all into the sqlite db.  We need:
         * | BI | ROS | ERC | Flame Length | Fuel Model |
         * ROS and Flame length need to be calculated with rothermel.
         * BI, ERC and SC are calculated with nfdrs and fuel model G
         */

        err = PlaceIgnition( dfX, dfY, pszInputWkt );
        if( err > FE_NONE )
        {
            skip++;
            continue;
        }
		dfTreatX = dfX;
		dfTreatY = dfY;
        FetchLandscapeData(dfX, dfY, pszInputWkt);

        err = FetchFriskData( dfX, dfY, pszInputWkt );
        if( err != FE_NONE )
        {
            skip++;
            continue;
        }

        //err = FetchFwaId( dfX, dfY, pszInputWkt );

        err = RunNfdr();
        /* FlamMap */
        err = RunFlamMap();

        nSunRise = GetSunRise( nJulDay, nYear, dfX, dfY, pszInputWkt );
        nSunSet = GetSunSet( nJulDay, nYear, dfX, dfY, pszInputWkt );

        nFireNum++;
        err = WriteToOGR();
        if( bAddTreated )
        {
            FetchTreatedData( dfTreatX, dfTreatY, pszInputWkt );
            RunFlamMap();
            /* fire_num, bi, ros, fuel, spec_cond, ratio */
            GetSpecCond( nFuelModel );
            fprintf( treated_out, "%d,%d,%lf,%d,%s,%lf\n", nFireNum,
                     nNfdrBurnIndex, dfFlmpSpreadRate, nFuelModel,
                     pszSpecCond, dfFlmpLenToWid );
        }

        //err = WriteTextToFile();
        if( bShowProgress )
        {
            double percent = (float)done / (float) nRecords;
            GDALTermProgress( percent, NULL, NULL );
        }
        done++;
        if( done % nSqliteTransactions == 0 )
        {
            OGR_L_CommitTransaction( hOutputLayer );
            OGR_L_StartTransaction( hOutputLayer );
        }

    }
    OGR_L_CommitTransaction( hOutputLayer );
    if( bShowProgress )
    {
        GDALTermProgress( 1.0, NULL, NULL );
    }
    CloseFiles();
    if( bAddTreated )
    {
        fclose( treated_out );
        CloseTreated();
    }
    if( bShowProgress )
    {
        fprintf( stderr, "Skipped %d records due to lack of NARR data.\n", skip );
    }

    return FE_NONE;
}

/**
 * \brief Generate FIG data from a raw weather stream
 *
 * Use raw input from NARR data in Matt Jolly's format (unknown right now, will
 * document).  This will generate the weather stream and then get pumped into
 * the R script or C/C++ wrapper and then run the caclulations for fb and nfdr.
 * \note Good candidate for threading
 * \param pszInputPath path to the raw narr data
 * \param pszOutputFilename output text file
 * \param papszOptions not used now, for future options
 * \return FE_NONE on success, error otherwise
 */
FE_Err
FIGGenerator::GenerateFromWeather( const char *pszInputPath,
                                   const char *pszOutputFilename,
                                   const char * const * papszOptions )
{

    fprintf( stderr, "GenerateFromWeather() not implemented\n" );
    return FE_NONE;
}
/**
 * \brief Override the data path (if any)
 *
 * The data path is dictated by the environment variable OMFFR_DATA which 
 * should point to a path where all the necessary data is located.  In the 
 * source tree, it is trunk/data.  No installation candidate exists currently.
 * If the data is in a known place, a call to this function *before* calling
 * one of the Generate() functions is necessary.
 *
 * \param pszPath new path
 * \param papszOptions unused key/value pair list
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::SetDataPath( const char *pszPath,
                           const char * const * papszOptions )
{
    pszDataPath = CPLStrdup( pszPath );
    return FE_NONE;
}

/**
 * \brief Override the default wkt for the input data
 *
 * The input data is known to be NAD83 or an Albers projection.  If the 
 * input data changes, a call to this function will override the default wkt
 * for projections
 *
 * \param pszWkt the new wkt
 * \param papszOptions a set of key/value pairs, unused.
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::SetInputWkt( const char *pszWkt,
                           const char * const * papszOptions )
{
    pszInputWkt = CPLStrdup( pszWkt );
    return FE_NONE;
}
/**
 * \brief Override the default output format.
 *
 *
 * \param pszWkt the new wkt
 * \param papszOptions a set of key/value pairs, unused.
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::SetFormat( const char *pszFormat,
                         const char * const * papszOptions )
{
    this->pszFormat = CPLStrdup( pszFormat );
    return FE_NONE;
}
/**
 * \brief Aquire the necessary file handles for input and output
 *
 * Several files are opened for input, such as fuels, frisk data, masks.  These
 * are all opened at the beginning of the program.  If threading is added,
 * mutexs will need to be assigned to the input text stream and the output
 * stream.  GDALOpen will also become GDALOpenShared().
 *
 * \return FE_NONE on success, FE_FILE_IO on error
 */
FE_Err
FIGGenerator::InitializeFiles()
{
    const char *pszFilename;
    const char *pszReadInZipDataPath;
    if( bReadInZip )
        pszReadInZipDataPath = CPLStrdup( CPLFormFilename( "/vsizip/", pszDataPath, NULL ) );
    else
        pszReadInZipDataPath = CPLStrdup(pszDataPath);

    fin = fopen( pszInputFilename, "r" );
    if( fin == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open input file." );
        return FE_FILE_IO;
    }
    //fout = fopen( pszOutputFilename, "w" );
    //if( fout == NULL )
    //{
        //ReportError( FE_FILE_IO, "Cannot open input file." );
        //return FE_FILE_IO;
    //}

    pszFilename = CPLFormFilename( pszDataPath, pszFriskBinaryFilename, NULL );
    friskStream.open( pszFilename, ios::binary );
    if( friskStream.fail() )
    {
        ReportError( FE_FILE_IO, "Cannot open frisk data for query." );
        return FE_FILE_IO;
    }

    /* Open and get handles to all the gdal datasets: aspect, base, cover,
     * density, elevation, fuel, height, slope
     */
    pszFilename = CPLFormFilename( pszDataPath, pszAspectFilename, NULL );
    hAspectDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hAspectDS == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open aspect data for query." );
        return FE_FILE_IO;
    }

    pszFilename = CPLFormFilename( pszDataPath, pszBaseFilename, NULL );
    hBaseDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hBaseDS == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open base data for query." );
        return FE_FILE_IO;
    }

    pszFilename = CPLFormFilename( pszDataPath, pszCoverFilename, NULL );
    hCoverDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hCoverDS == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open cover data for query." );
        return FE_FILE_IO;
    }

    pszFilename = CPLFormFilename( pszDataPath, pszDensityFilename, NULL );
    hDensityDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hDensityDS == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open density data for query." );
        return FE_FILE_IO;
    }

    pszFilename = CPLFormFilename( pszDataPath, pszElevFilename, NULL );
    hElevDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hElevDS == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open elevation data for query." );
        return FE_FILE_IO;
    }

    pszFilename = CPLFormFilename( pszDataPath, pszFuelFilename, NULL );
    hFuelDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hFuelDS == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open fuels data for query." );
        return FE_FILE_IO;
    }

    pszFilename = CPLFormFilename( pszDataPath, pszHeightFilename, NULL );
    hHeightDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hHeightDS == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open height data for query." );
        return FE_FILE_IO;
    }

    pszFilename = CPLFormFilename( pszDataPath, pszSlopeFilename, NULL );
    hSlopeDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hSlopeDS == NULL )
    {
        ReportError(FE_FILE_IO, "Cannot open slope data for query." );
        return FE_FILE_IO;
    }

    pszFilename = CPLFormFilename( pszDataPath, pszNarrMaskFilename, NULL );
    hMaskDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hMaskDS == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open NARR mask for query." );
        return FE_FILE_IO;
    }

    pszFilename = CPLFormFilename( pszDataPath, pszIgnProbFilename, NULL );
    hIgnMaskDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hIgnMaskDS == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open Ignition Prob mask for query." );
        return FE_FILE_IO;
    }

    pszFilename = CPLFormFilename( pszDataPath, pszFuelPercFilename, NULL );
    hFuelPercMaskDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hFuelPercMaskDS == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open fuel existance prob mask for query." );
        return FE_FILE_IO;
    }

    pszFilename = CPLFormFilename( pszDataPath, pszFwaRasterFilename, NULL );
    hFwaRasterDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hFwaRasterDS == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open fuel existance prob mask for query." );
        return FE_FILE_IO;
    }


    pszFilename = CPLFormFilename( pszReadInZipDataPath, pszFwaShpFilename,
                                   NULL );
    hFwaDS = OGROpen( pszFilename, FALSE, NULL );
    if( hFwaDS == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open FWA file for query." );
        return FE_FILE_IO;
    }
    hFwaLayer = OGR_DS_GetLayerByName( hFwaDS, "FWA_20120702" );
    if( hFwaLayer == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open FWA file for query." );
        return FE_FILE_IO;
    }
    hFwaFeatureDefn = OGR_L_GetLayerDefn( hFwaLayer );
    if( hFwaFeatureDefn == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open FWA file for query." );
        return FE_FILE_IO;
    }
    hPoint = OGR_G_CreateGeometry( wkbPoint );

    hOutputDriver = OGRGetDriverByName( pszFormat );
    if( hOutputDriver == NULL )
    {
        ReportError( FE_FILE_IO, "Output format not supported for writing." );
        return FE_FILE_IO;
    }
    hOutputDS = OGR_Dr_CreateDataSource( hOutputDriver, pszOutputFilename, 
                                         NULL );
    if( hOutputDS == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open output file for writing." );
        return FE_FILE_IO;
    }
    
    OGRSpatialReferenceH hSRS;
    hSRS = OSRNewSpatialReference( NAD83_WKT );
    const char *pszLayername = CPLGetBasename( pszOutputFilename );
    char **papszCreationOptions = NULL;
    if( EQUAL( pszFormat, "ESRI Shapefile" ) )
        papszCreationOptions = CSLAddString( papszCreationOptions, "SHPT=POINT" );
    else if( EQUAL( pszFormat, "CSV" ) )
        papszCreationOptions = CSLAddString( papszCreationOptions, "GEOMETRY=AS_XY" );
    hOutputLayer = OGR_DS_CreateLayer( hOutputDS,
                                       pszLayername, hSRS, wkbPoint,
                                       papszCreationOptions );
    OSRDestroySpatialReference( hSRS );
    if( hOutputLayer == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot create layer in output dataset." );
        return FE_FILE_IO;
    }
    hOutputFieldDefn = OGR_Fld_Create( "fire_num", OFTInteger );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }
    hOutputFieldDefn = OGR_Fld_Create( "year", OFTInteger );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }
    hOutputFieldDefn = OGR_Fld_Create( "day", OFTInteger );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }
    //hOutputFieldDefn = OGR_Fld_Create( "week_day", OFTInteger );
    hOutputFieldDefn = OGR_Fld_Create( "week_day", OFTString );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }
    //hOutputFieldDefn = OGR_Fld_Create( "disc_time", OFTInteger );
    hOutputFieldDefn = OGR_Fld_Create( "disc_time", OFTString );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }
    hOutputFieldDefn = OGR_Fld_Create( "bi", OFTInteger );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }
    hOutputFieldDefn = OGR_Fld_Create( "ros", OFTReal );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }
    hOutputFieldDefn = OGR_Fld_Create( "erc", OFTReal );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }
    hOutputFieldDefn = OGR_Fld_Create( "fuel", OFTInteger );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }
    hOutputFieldDefn = OGR_Fld_Create( "spec_cond", OFTString );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }
    hOutputFieldDefn = OGR_Fld_Create( "slope_perc", OFTInteger );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }
    hOutputFieldDefn = OGR_Fld_Create( "elev", OFTInteger );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }
    hOutputFieldDefn = OGR_Fld_Create( "ratio", OFTReal );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }
    //hOutputFieldDefn = OGR_Fld_Create( "sunrise", OFTInteger );
    hOutputFieldDefn = OGR_Fld_Create( "sunrise", OFTString );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }
    //hOutputFieldDefn = OGR_Fld_Create( "sunset", OFTInteger );
    hOutputFieldDefn = OGR_Fld_Create( "sunset", OFTString );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }
    //hOutputFieldDefn = OGR_Fld_Create( "fwa_id", OFTInteger );
    hOutputFieldDefn = OGR_Fld_Create( "fwa_name", OFTString );
    if( OGR_L_CreateField( hOutputLayer, hOutputFieldDefn, TRUE ) != OGRERR_NONE )
    {
        ReportError( FE_FILE_IO, "Cannot create field in output dataset." );
        return FE_FILE_IO;
    }

    OGR_Fld_Destroy( hOutputFieldDefn );

    /* Why is this here? I would think OGR_G_DestroyGeometry(hPoint); */
    //hPoint = OGR_G_CreateGeometry( wkbPoint );
    OGR_G_DestroyGeometry( hPoint );

    return FE_NONE;
}

FE_Err FIGGenerator::InitializeTreated( const char *pszPath )
{
    const char *pszFilename;
    pszFilename = CPLFormFilename( pszPath, "base.tif", NULL );
    hBaseTreatDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hBaseTreatDS == NULL )
    {
        bAddTreated = FALSE;
        ReportError( FE_FILE_IO, "Cannot open treated base data for query." );
        return FE_FILE_IO;
    }
    pszFilename = CPLFormFilename( pszPath, "cover.tif", NULL );
    hCoverTreatDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hCoverTreatDS == NULL )
    {
        bAddTreated = FALSE;
        ReportError( FE_FILE_IO, "Cannot open cover data for query." );
        return FE_FILE_IO;
    }

    pszFilename = CPLFormFilename( pszPath, "density.tif", NULL );
    hDensityTreatDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hDensityTreatDS == NULL )
    {
        bAddTreated = FALSE;
        ReportError( FE_FILE_IO, "Cannot open density data for query." );
        return FE_FILE_IO;
    }

    pszFilename = CPLFormFilename( pszPath, "fuel40.tif", NULL );
    hFuelTreatDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hFuelTreatDS == NULL )
    {
        bAddTreated = FALSE;
        ReportError( FE_FILE_IO, "Cannot open fuel data for query." );
        return FE_FILE_IO;
    }

    pszFilename = CPLFormFilename( pszPath, "height.tif", NULL );
    hHeightTreatDS = GDALOpen( pszFilename, GA_ReadOnly );
    if( hHeightTreatDS == NULL )
    {
        bAddTreated = FALSE;
        ReportError( FE_FILE_IO, "Cannot open height data for query." );
        return FE_FILE_IO;
    }
    return FE_NONE;
}

FE_Err FIGGenerator::CreateRasterAccess()
{
    const char *pszFilename;
    const char *pszReadInZipDataPath;
    if( bReadInZip )
        pszReadInZipDataPath = CPLStrdup( CPLFormFilename( "/vsizip/", pszDataPath, NULL ) );
    else
        pszReadInZipDataPath = CPLStrdup(pszDataPath);

    pszFilename = CPLFormFilename( pszDataPath, pszAspectFilename, NULL );
    poAspect = new RasterAccess( pszFilename );
    pszFilename = CPLFormFilename( pszDataPath, pszBaseFilename, NULL );
    poBase = new RasterAccess( pszFilename );
    pszFilename = CPLFormFilename( pszDataPath, pszCoverFilename, NULL );
    poCover = new RasterAccess( pszFilename );
    pszFilename = CPLFormFilename( pszDataPath, pszDensityFilename, NULL );
    poDensity = new RasterAccess( pszFilename );
    pszFilename = CPLFormFilename( pszDataPath, pszElevFilename, NULL );
    poElev = new RasterAccess( pszFilename );
    pszFilename = CPLFormFilename( pszDataPath, pszFuelFilename, NULL );
    poFuel = new RasterAccess( pszFilename );
    pszFilename = CPLFormFilename( pszDataPath, pszHeightFilename, NULL );
    poHeight = new RasterAccess( pszFilename );
    pszFilename = CPLFormFilename( pszDataPath, pszSlopeFilename, NULL );
    poSlope = new RasterAccess( pszFilename );
    pszFilename = CPLFormFilename( pszDataPath, pszNarrMaskFilename, NULL );
    poNarrMask = new RasterAccess( pszFilename );
    pszFilename = CPLFormFilename( pszDataPath, pszIgnProbFilename, NULL );
    poIgnMask = new RasterAccess( pszFilename );

    pszFilename = CPLFormFilename( pszDataPath, pszFwaRasterFilename, NULL );
    poFwaRaster = new RasterAccess( pszFilename );
    pszFilename = CPLFormFilename( pszDataPath, pszFwaMaskFilename, NULL );
    poFwaMask = new RasterAccess( pszFilename );

    pszFilename = CPLFormFilename( pszDataPath, pszFuelPercFilename, NULL );
    poFuelPercMask = new RasterAccess( pszFilename );

    return FE_NONE;

}

/**
 * \brief Close the file handles created by InitializeFiles()
 *
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::CloseFiles()
{
    fclose( fin );
    //fclose( fout );
    friskStream.close();
    GDALClose( hAspectDS );
    GDALClose( hBaseDS );
    GDALClose( hCoverDS );
    GDALClose( hDensityDS );
    GDALClose( hElevDS );
    GDALClose( hFuelDS );
    GDALClose( hHeightDS );
    GDALClose( hSlopeDS );
    GDALClose( hMaskDS );
    OGR_DS_Destroy( hFwaDS );
    GDALClose( hIgnMaskDS );
    GDALClose( hFuelPercMaskDS );
    OGR_DS_Destroy( hOutputDS );
    return FE_NONE;
}

FE_Err FIGGenerator::CloseTreated()
{
    GDALClose( hBaseTreatDS );
    GDALClose( hCoverTreatDS );
    GDALClose( hDensityTreatDS );
    GDALClose( hFuelTreatDS );
    GDALClose( hHeightTreatDS );
	return FE_NONE;
}

/**
 * \brief Write a record to disk as text
 *
 * Write data to disk after the calculations are done.
 * 
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::WriteToOGR()
{
    int nDow = GetDayOfWeek( nYear, nJulDay );
    /* longitude, latitude, scenario, fire number, julian day, day of week, discovery time,
     * bi, ros, erc, fuel model, spec cond, slope %, length/width ratio, 
     * sunrise, sunset, fwa id 
     */

    /* Output in NAD83 to match original ignition data */
    ReprojectPoint( &dfX, &dfY, pszInputWkt, NAD83_WKT ); 
    hOutputFeature = OGR_F_Create( OGR_L_GetLayerDefn( hOutputLayer ) );
    hOutputPoint = OGR_G_CreateGeometry( wkbPoint );
    OGR_G_SetPoint_2D( hOutputPoint, 0, dfX, dfY );
    OGR_F_SetGeometry( hOutputFeature, hOutputPoint );
    char abyTemp[BUF_SIZE];

    OGR_F_SetFieldInteger( hOutputFeature,
                           OGR_F_GetFieldIndex( hOutputFeature, "fire_num" ),
                           nFireNum );
    OGR_F_SetFieldInteger( hOutputFeature,
                           OGR_F_GetFieldIndex( hOutputFeature, "year" ),
                           nYear );

    OGR_F_SetFieldInteger( hOutputFeature,
                           OGR_F_GetFieldIndex( hOutputFeature, "day" ),
                           nJulDay );

    OGR_F_SetFieldString( hOutputFeature,
                           OGR_F_GetFieldIndex( hOutputFeature, "week_day" ),
                           DayOfWeek( nDow ) );
                           //nDow );

    sprintf( abyTemp, "%04d", nDiscTime );
    OGR_F_SetFieldString( hOutputFeature,
                           OGR_F_GetFieldIndex( hOutputFeature, "disc_time" ),
                           abyTemp );

    OGR_F_SetFieldInteger( hOutputFeature,
                           OGR_F_GetFieldIndex( hOutputFeature, "bi" ),
                           nNfdrBurnIndex );

    OGR_F_SetFieldDouble( hOutputFeature,
                          OGR_F_GetFieldIndex( hOutputFeature, "ros" ),
                          dfFlmpSpreadRate );

    OGR_F_SetFieldDouble( hOutputFeature,
                          OGR_F_GetFieldIndex( hOutputFeature, "erc" ),
                          dfErc );

    OGR_F_SetFieldInteger( hOutputFeature,
                           OGR_F_GetFieldIndex( hOutputFeature, "fuel" ),
                           nFuelModel );

    GetSpecCond( nFuelModel );
    OGR_F_SetFieldString( hOutputFeature,
                          OGR_F_GetFieldIndex( hOutputFeature, "spec_cond" ),
                          pszSpecCond );

    OGR_F_SetFieldInteger( hOutputFeature,
                           OGR_F_GetFieldIndex( hOutputFeature, "slope_perc" ),
                           nSlopePercent );

    OGR_F_SetFieldInteger( hOutputFeature,
                           OGR_F_GetFieldIndex( hOutputFeature, "elev" ),
                           nElev );

    OGR_F_SetFieldDouble( hOutputFeature,
                          OGR_F_GetFieldIndex( hOutputFeature, "ratio" ),
                          dfFlmpLenToWid );

    int nMdt = nSunRise - 600;
    if( nMdt < 0 )
        nMdt = 2400 - nMdt;
    //sprintf( abyTemp, "%04d", nMdt );
    sprintf( abyTemp, "%04d", nSunRise );
    OGR_F_SetFieldString( hOutputFeature,
                           OGR_F_GetFieldIndex( hOutputFeature, "sunrise" ),
                           abyTemp );
                           //nSunRise );

    nMdt = nSunSet - 600;
    if( nMdt < 0 )
        nMdt = 2400 + nMdt;
    //sprintf( abyTemp, "%04d", nMdt );
    sprintf( abyTemp, "%04d", nSunSet );
    OGR_F_SetFieldString( hOutputFeature,
                           OGR_F_GetFieldIndex( hOutputFeature, "sunset" ),
                           abyTemp );
                           //nSunSet );

    /*sprintf( abyTemp, "%d", nFwaId );
    OGR_F_SetFieldString( hOutputFeature,
                           OGR_F_GetFieldIndex( hOutputFeature, "fwa_name" ),
                           pszFwaName );
                           //abyTemp );
    */

    OGR_G_DestroyGeometry( hOutputPoint );
    OGR_L_CreateFeature( hOutputLayer, hOutputFeature );
    //OGR_L_SetFeature( hOutputLayer, hOutputFeature );
    OGR_F_Destroy( hOutputFeature );

    return FE_NONE;
}

/**
 * \brief Write a record to disk as text
 *
 * Write data to disk after the calculations are done.
 * 
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::WriteTextToFile()
{
    int nDoy = GetDayOfWeek(nYear, nJulDay);
    double dfLenToWid = oOutputFB.head / oOutputFB.flank;
    /* longitude, latitude, scenario, fire number, julian day, day of week, discovery time,
     * bi, ros, erc, fuel model, spec cond, slope %, length/width ratio, 
     * sunrise, sunset, fwa id 
     */
    sprintf( abyOutputBuffer, "%lf %lf %d %d %d %d %d "
                              "%d %lf %lf %d %d %d %lf "
                              "%d %d %d \n",
                              dfX, dfY, nYear, 0, nJulDay, nDoy, nDiscTime,
                              nNfdrBurnIndex, oOutputFB.ros, dfErc, nFuelModel,
                              1, nSlopePercent, dfLenToWid,
                              0, 0, 0);
    fputs( abyOutputBuffer, fout );
    return FE_NONE;
}
/**
 * \brief Write a record to disk in sqlite db
 *
 * Write data to disk after the calculations are done.
 * 
 * \return FE_NONE on success
 */
/*FE_Err
FIGGenerator::WriteTextToFile()
{
    int nDoy = GetDayOfWeek(nYear, nJulDay);
    double dfLenToWid = oOutputFB.head / oOutputFB.flank;
    /* longitude, latitude, scenario, fire number, julian day, day of week, discovery time,
     * bi, ros, erc, fuel model, spec cond, slope %, length/width ratio, 
     * sunrise, sunset, fwa id 
     */
/*
    sprintf( abyOutputBuffer, "%lf %lf %d %d %d %d %d "
                              "%d %lf %lf %d %d %d %lf "
                              "%d %d %d \n",
                              dfX, dfY, nYear, 0, nJulDay, nDoy, nDiscTime,
                              nNfdrBurnIndex, oOutputFB.ros, dfErc, nFuelModel,
                              1, nSlopePercent, dfLenToWid,
                              0, 0, 0);

    return FE_NONE;
}
*/
/**
 * \brief Report an error
 *
 * \param eCode the error code
 * \param pszMessage a message to be printed
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::ReportError( FE_Err eCode, const char *pszMessage )
{
    nLastErrorCode = eCode;
    pszLastErrorMessage = pszMessage;
    fprintf( stderr, "%s\n", pszMessage );
    return FE_NONE;
}

/**
 * \brief Fetch the last error code set
 *
 * \param papszOptions a list of key/value pairs, unused
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::GetLastErrorCode( const char * const * papszOptions )
{
    return nLastErrorCode;
}

/**
 * \brief Fetch the last error message set
 *
 * \warning this string shouldn't be freed or manipulated.
 * \param papszOptions a list of key/value pairs, unused
 * \return FE_NONE on success
 */
const char *
FIGGenerator::GetLastErrorMessage( const char * const * papszOptions )
{
    return pszLastErrorMessage;
}

/**
 * \brief Assign an nfdrs slope class.
 *
 * NFDRS uses slope classes 1-4 depending on slope in percent.
 *
 * \param slope slope in percent
 * \param[out] slope_class slope class output
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::SlopeClass( int slope, int *slope_class )
{
    if(slope < 0 )
        return FE_INVALID_INPUT;
    if( slope <= 25 )
        *slope_class = 1;
    else if( slope <= 40 )
        *slope_class = 2;
    else if( slope <= 55 )
        *slope_class = 3;
    else if( slope <= 75 )
        *slope_class = 4;
    else
        *slope_class = 5;
    return FE_NONE;
}

/**
 * \brief Load the local LandscapeStruct from disk
 *
 * \param x x coordinate
 * \param y y coordinate
 * \param pszWkt spatial reference of the coordinate pair
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::FetchLandscapeData( double x, double y, const char *pszWkt )
{
    double dfValue;
    //poElev->PixelValue( x, y, &dfValue, pszWkt );
    PixelValue( hElevDS, 1, x, y, &dfValue, pszWkt );
    oLcpData.elev = (int)dfValue;
    nElev = (int)dfValue;
    //poSlope->PixelValue( x, y, &dfValue, pszWkt );
    PixelValue( hSlopeDS, 1, x, y, &dfValue, pszWkt );
    nSlopePercent = (int)dfValue;
    //poAspect->PixelValue( x, y, &dfValue, pszWkt );
    PixelValue( hAspectDS, 1, x, y, &dfValue, pszWkt );
    oLcpData.aspect = (int)dfValue;
    //poFuel->PixelValue( x, y, &dfValue, pszWkt );
    PixelValue( hFuelDS, 1, x, y, &dfValue, pszWkt );
    nFuelModel = oLcpData.fuel = (int)dfValue;
    //poCover->PixelValue( x, y, &dfValue, pszWkt );
    PixelValue( hCoverDS, 1, x, y, &dfValue, pszWkt );
    oLcpData.cover = (int)dfValue;
    //poHeight->PixelValue( x, y, &dfValue, pszWkt );
    PixelValue( hHeightDS, 1, x, y, &dfValue, pszWkt );
    oLcpData.height = (int)dfValue;
    //poBase->PixelValue( x, y, &dfValue, pszWkt );
    PixelValue( hBaseDS, 1, x, y, &dfValue, pszWkt );
    oLcpData.base = (int)dfValue;
    //poDensity->PixelValue( x, y, &dfValue, pszWkt );
    PixelValue( hDensityDS, 1, x, y, &dfValue, pszWkt );
    oLcpData.density = (int)dfValue;
    /* Fix slope, keep percent value too */
    oLcpData.slope = (short)( atan( (double) nSlopePercent / 100.0 ) / PI * 180.0) + 0.5;
    /* Change aspectf to radians */
    oLcpData.aspectf = (double)oLcpData.aspect/180.0*PI;
    /* Check for height, if 0.0, set to 15.0 */
    if( oLcpData.height == 0 )
    {
        oLcpData.height = 15.0;
    }
    oLcpData.duff = 0;
    oLcpData.woody = 0;

    return FE_NONE;
}

/**
 * \brief Load the local LandscapeStruct from treated lcp
 *
 * \param x x coordinate
 * \param y y coordinate
 * \param pszWkt spatial reference of the coordinate pair
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::FetchTreatedData( double x, double y, const char *pszWkt )
{
    double dfValue;
    PixelValue( hFuelTreatDS, 1, x, y, &dfValue, pszWkt );
    nFuelModel = oLcpData.fuel = (int)dfValue;
    PixelValue( hCoverTreatDS, 1, x, y, &dfValue, pszWkt );
    oLcpData.cover = (int)dfValue;
    PixelValue( hHeightTreatDS, 1, x, y, &dfValue, pszWkt );
    oLcpData.height = (int)dfValue;
    PixelValue( hBaseTreatDS, 1, x, y, &dfValue, pszWkt );
    oLcpData.base = (int)dfValue;
    PixelValue( hDensityTreatDS, 1, x, y, &dfValue, pszWkt );
    oLcpData.density = (int)dfValue;
    /* Check for height, if 0.0, set to 15.0 */
    if( oLcpData.height == 0 )
    {
        oLcpData.height = 15.0;
    }
    oLcpData.duff = 0;
    oLcpData.woody = 0;

    return FE_NONE;
}


/**
 * \brief Fetch the frisk data for a point
 *
 * The frisk is a large binary file that stores all the erc and weather
 * summaries for the area of interest.  This function seeks to the correct
 * location in the file based on x/y coordinate and sets the appropriate data.
 *
 * \warning Due to issues with the mask along the edges of the lower 48, it
 * appears that some valid points will not find a valid mask value.  Currently
 * a warning is emitted and the record is skipped.  Another alternative would
 * be to use a neighbor for the data.
 *
 * \todo Fix valid point, but bad mask, see warning.
 *
 * \param x x coordinate
 * \param y y coordinate
 * \param pszWkt wkt for the point
 * \return FE_NONE on success, FE_INVALID_INPUT on error
 */
FE_Err
FIGGenerator::FetchFriskData( double x, double y, const char *pszWkt )
{
    int nMaskXSize = GDALGetRasterXSize( hMaskDS );
    int nMaskYSize = GDALGetRasterYSize( hMaskDS );
    double dfValue;
    /* Check the mask, if we don't have data, skip it and emit a
     * warning
     */
    //poNarrMask->PixelValue( x, y, &dfValue, pszWkt );
    int err = PixelValue( hMaskDS, 1, x, y, &dfValue, pszWkt );
    int mask = (int)dfValue;
    //assert( mask );
    /* Search the pixels around the bad mask for any good data.  Set a max
     * search distance, and also use averaging or first point, configurable
     */
    if( !mask )
    {
        fprintf( stderr, "NARR data not available, skipping record\n" );
        return FE_INVALID_INPUT;
    }
    /* NFDRS Stuff */

    int nPixel, nLine;
    //poNarrMask->PixelIndex( x, y, &nPixel, &nLine, pszInputWkt );
    PixelIndex(hMaskDS, x, y, pszInputWkt, &nPixel, &nLine);

    /* We should always have data for our query */
    assert( nPixel < nMaskXSize );
    assert( nLine < nMaskYSize );

    if( nPixel >= nMaskXSize || nLine >= nMaskYSize ||
        nPixel < 0 ||nLine < 0 )
    {
        CPLDebug( "fig_load", "Invalid pixel x:%d y:%d", nPixel, nLine );
        return FE_INVALID_INPUT;
    }

    long offset = nPixel + nLine * nMaskXSize;
    friskStream.seekg( offset * FRISK_SIZE, ios::beg );
    frisk.ReadBinary( friskStream );

    frisk.GetRandomWind( nJulDay, &dfWindSpeed, &dfWindDir );
    /*
    oFuelMoisture.fm_1 = (double)frisk.GetOneHour( dfErc );
    oFuelMoisture.fm_10 = (double)frisk.GetTenHour( dfErc );
    oFuelMoisture.fm_100 = (double)frisk.GetHundredHour( dfErc );
    oFuelMoisture.fm_1000 = (double)frisk.GetThousandHour( dfErc );
    oFuelMoisture.fm_wood = (double)frisk.GetWoody( dfErc );
    oFuelMoisture.fm_herb = (double)frisk.GetHerb( dfErc );
    */
    oFuelMoisture.fm_1 = (double)frisk.GetOneHourP( nErc );
    oFuelMoisture.fm_10 = (double)frisk.GetTenHourP( nErc );
    oFuelMoisture.fm_100 = (double)frisk.GetHundredHourP( nErc );
    oFuelMoisture.fm_1000 = (double)frisk.GetThousandHourP( nErc );
    oFuelMoisture.fm_wood = (double)frisk.GetWoodyP( nErc );
    oFuelMoisture.fm_herb = (double)frisk.GetHerbP( nErc );
    dfErc = (double)frisk.GetErcPercP( nErc );
    return FE_NONE;
}

/**
 * \brief Set a simple spatial filter based on a bounding box
 *
 * In order to 'unset' the envelope, send 4 equal values
 *
 * \param n Northern limit in input SRS
 * \param e Eastern limit in input SRS
 * \param s Southern limit in input SRS
 * \param w Western limit in input SRS
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::SetSpatialEnv( double dfMaxX, double dfMinX,
                             double dfMaxY, double dfMinY,
                             const char * const * papszOptions )
{
    if( dfMaxX == dfMinX == dfMaxY == dfMinY )
        bEnvSet = FALSE;
    else
    {
        this->dfMaxX = dfMaxX;
        this->dfMinX = dfMinX;
        this->dfMaxY = dfMaxY;
        this->dfMinY = dfMinY;
        ReprojectPoint( &this->dfMaxX, &this->dfMinY, NAD83_WKT, pszInputWkt );
        ReprojectPoint( &this->dfMinX, &this->dfMaxY, NAD83_WKT, pszInputWkt );
        bEnvSet = TRUE;
    }
    return FE_NONE;
}

/**
 * \brief Set the spatial envelope given an fpu id

 * Open the FPU shapefile and fetch the envelope from the selected fpu.
 *
 * \param pszFpuCode the fpu id, ie CA_CA_001
 * \param papszOptions a list of key/value options, not used
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::SetFpuCode( const char *pszFpuCode,
                          const char * const * papszOptions )
{
    this->pszFpuCode = CPLStrdup( pszFpuCode );
    const char *pszFilename;
    pszFilename = CPLFormFilename( "/vsizip/", pszDataPath, NULL );
    pszFilename = CPLFormFilename( pszFilename, pszFpuShpFilename, NULL );
    OGRDataSourceH hDS = OGROpen( pszFilename, FALSE, NULL );
    if( hDS == NULL )
    {
        fprintf( stderr, "Cannot open fpu shapefile\n" );
        return FE_FILE_IO;
    }
    OGRLayerH hLayer;
    OGRFieldDefnH hFieldDefn;
    OGRFeatureH hFeature;
    OGRGeometryH hGeometry;
    OGREnvelope psEnvelope;

    hLayer = OGR_DS_GetLayerByName( hDS, CPLGetBasename( pszFpuShpFilename ) );
    if( hLayer == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open FPU file for query." );
        return FE_FILE_IO;
    }
    OGRSpatialReferenceH hSRS = OGR_L_GetSpatialRef( hLayer );
    const char *pszFpuWkt;
    OSRExportToWkt( hSRS, (char**)&pszFpuWkt );
    hFieldDefn = OGR_L_GetLayerDefn( hLayer );
    if( hFieldDefn == NULL )
    {
        ReportError( FE_FILE_IO, "Cannot open FPU file for query." );
        return FE_FILE_IO;
    }
    char pszWhere[BUF_SIZE];
    sprintf( pszWhere, "FPU_CODE='%s'", pszFpuCode );
    OGR_L_SetAttributeFilter( hLayer, pszWhere );
    hFeature = OGR_L_GetNextFeature( hLayer );
    if( hFeature == NULL )
    {
        ReportError( FE_FILE_IO, "Query resulted in no FPU." );
        return FE_FILE_IO;
    }

    hGeometry = OGR_F_GetGeometryRef( hFeature );
    OGR_G_GetEnvelope( hGeometry, &psEnvelope );
    dfMaxX = psEnvelope.MaxX;
    dfMinX = psEnvelope.MinX;
    dfMaxY = psEnvelope.MaxY;
    dfMinY = psEnvelope.MinY;

    /* Performance hit if we reproject incoming points, this just uses input
     * srs as bounds
     */
    ReprojectPoint( &dfMaxX, &dfMinY, pszFpuWkt, pszInputWkt );
    ReprojectPoint( &dfMinX, &dfMaxY, pszFpuWkt, pszInputWkt );

    bEnvSet = TRUE;
    //OGR_F_Destroy( hFeature );
    //OGR_G_DestroyGeometry( hGeometry );
    OGR_DS_Destroy( hDS );
    return FE_NONE;
}

/**
 * \brief Stub for specific condition
 *
 * \warning Only works with GB_ID_002
 */
FE_Err
FIGGenerator::GetSpecCond( int nFuel, int nFwa )
{
    switch( nFuel )
    {
        case 1:
            pszSpecCond = "Grass";
            break;
        case 4:
            pszSpecCond = "Chaparral";
            break;
        case 6:
            pszSpecCond = "BlackSpruce";
            break;
        case 8:
            pszSpecCond = "Conifers";
            break;
        case 9:
            pszSpecCond = "Conifers";
            break;
        case 101:
            pszSpecCond = "Grass";
            break;
        case 102:
            pszSpecCond = "Grass";
            break;
        case 161:
            pszSpecCond = "Conifers";
            break;
        case 164:
            pszSpecCond = "Others";
            break;
        default:
            pszSpecCond = "x";
    }
    return FE_NONE;
}

/**
 * \brief calculate the NFDR values
 *
 * After the input data is loaded from file, the nfdr values are calculated
 * here.
 *
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::RunNfdr()
{

    int nDummy = 0;
    double dfDummy = 0.0;
    int nSlopeClass;
    nfdr.iSetMoistures( oFuelMoisture.fm_1, oFuelMoisture.fm_10, 
                        oFuelMoisture.fm_100, oFuelMoisture.fm_1000,
                        oFuelMoisture.fm_wood, oFuelMoisture.fm_herb,
                        nDummy, nDummy, nDummy, nDummy, nDummy, nDummy );

    nNfdrBurnIndex = nNfdrSpreadComp = 0;
    SlopeClass( nSlopePercent, &nSlopeClass );
    nfdr.iCalcIndexes( dfWindSpeed, nSlopeClass, &dfDummy, &nNfdrSpreadComp,
                       &dfDummy, &dfDummy, &nDummy, &nNfdrBurnIndex );
    return FE_NONE;
}

/**
 * \brief calculate the fire behavior values
 *
 * After the input data is loaded from file, the fire behavior values are        
 * calculated here.
 *
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::RunFlamMap()
{
    /* Not sure if we have to call this each time */
    //flammap.burn->pointBurnThread = new BurnThread(flammap.burn->env);
    flammap.RunFlamMapPoint(oLcpData, oFuelMoisture, dfWindSpeed, dfWindDir,
                            oOutputFB);
    dfFlmpSpreadRate = oOutputFB.ros;
    /* FIXME Need to set l to w if flank == 0 || head == 0 */
    if( oOutputFB.flank > 0.0 )
        dfFlmpLenToWid = oOutputFB.head / oOutputFB.flank;
    else
        dfFlmpLenToWid = 1.0;
    nFlmpFlameLength = oOutputFB.flame_length;
    return FE_NONE;
}

/**
 * \brief Figure out the day of the week given julian day and year.
 * 
 * Year is ignored and set to 2000 for now.  Should be set to 1900 + year
 * \param y year of date
 * \param d julian day
 * \return index of day of week 0-6
 */
int FIGGenerator::GetDayOfWeek( int y, int d )
{
    int days = 0;
    int i = 0;
    int dom = d - days;
    if(d <= DAYS_IN_MONTH[0])
    {
        dom = d;
    }
    else
    {
        while(days < d)
        {
            days += DAYS_IN_MONTH[i];
            i++;
        }
        days -= DAYS_IN_MONTH[i-1];
        dom = d - days;
    }
    time_t t;
    struct tm *t_info;
    time( &t );
    t_info = localtime( &t );
    t_info->tm_year = 100;
    //t_info->tm_yday = d;
    t_info->tm_mon = i;
    t_info->tm_mday = dom;
    mktime( t_info );
    return t_info->tm_wday;
}

/**
 * \brief Fetch the FWA ID from file
 *
 * The FWA data is a large (~200MB) shape file. This is slow. 
 *
 * \param x x coordinate
 * \param y y coordinate
 * \param pszWkt wkt for the coordinate
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::FetchFwaId( double x, double y, const char *pszWkt )
{
    int err;
    int nFeatures;

    double dfValue;
    if( ePointSearch == PIP_RASTER )
    {
        //poFwaRaster->PixelValue( x, y, &dfValue, pszWkt );
        PixelValue( hFwaRasterDS, 1, x, y, &dfValue, pszWkt );
        nFwaId = (int)dfValue;
        int success = FALSE;
        if( nFwaId == GDALGetRasterNoDataValue( GDALGetRasterBand( hFwaRasterDS, 1 ), &success ) )
        {
            ReportError( FE_FILE_IO, "Could not locate FWU Name." );
            pszFwaName = "Unknown";
            return FE_FILE_IO;
        }
        char pszWhere[BUF_SIZE];
        sprintf( pszWhere, "FWA_GIS_ID='%d'", nFwaId ); 
        OGR_L_SetAttributeFilter( hFwaLayer, pszWhere );
        OGR_L_ResetReading( hFwaLayer );
        hFwaFeature = OGR_L_GetNextFeature( hFwaLayer );
        if( hFwaFeature == NULL )
        {
            ReportError( FE_FILE_IO, "Could not locate FWU Name." );
            pszFwaName = "Unknown";
            return FE_FILE_IO;
        }

        pszFwaName = CPLStrdup( OGR_F_GetFieldAsString( hFwaFeature, 
                                                    OGR_FD_GetFieldIndex( hFwaFeatureDefn,
                                                                          "FWA_NAME" ) ) ); 
    }
    else if ( ePointSearch == PIP_HYBRID )
    {
        //poFwaMask->PixelValue( x, y, &dfValue, pszWkt );
        PixelValue( hFwaFeatureMask, 1, x, y, &dfValue, pszWkt );
        nFeatures = (int)dfValue;
        if( nFeatures == 1 )
        {
            //poFwaRaster->PixelValue( x, y, &dfValue, pszWkt );
            PixelValue( hFwaRasterDS, 1, x, y, &dfValue, pszWkt );
            nFwaId = (int)dfValue;
            return FE_NONE;
        }
    }
    else if( ePointSearch == PIP_REAL )
    {
        OGR_G_SetPoint_2D( hPoint, 0, x, y );
        OGR_L_SetSpatialFilter( hFwaLayer, hPoint );
        while( ( hFwaFeature = OGR_L_GetNextFeature( hFwaLayer ) ) != NULL )
        {
            hFwaGeometry = OGR_F_GetGeometryRef( hFwaFeature );
            if( OGR_G_Contains( hFwaGeometry, hPoint ) )
            {
                printf("Found one...\n");
            }
        }
    }
    if(hFwaFeature)
        OGR_F_Destroy( hFwaFeature );

    return FE_NONE;
}

/**
 * \brief Place an ignition on the landscape
 * FIXME: Redocument after last changes.
 * Place an ignition give an x and y that represents the _center_ of a pixel on
 * the NARR weather data.  The working coordinate is moved 1/2 pixel in x and y
 * from the origin of the NARR pixel and placed on the ignition probability
 * raster.  The working coordinate then adds all the probability values that
 * overlap with the NARR pixel.  The looping is done again and the sum of the
 * probabilities is compared to a random number.  The working coordinate is
 * fixed when the sum is greater than the random number.  The working
 * coordinate is then moved to the origin of the pixel it falls in with respect
 * to the ignition probability raster, then randomly placed inside that pixel.
 * The fuel data is queried, and non burnable fuel results in a probability of
 * 0.
 *
 * \f$ W_{i} = \log{(\alpha + \epsilon)} \f$
 *
 * \f$ W = \sum_{i=1}^n W_{i} \f$
 *
 * \f$ p_{i} = \frac{W_{i}}{W} \f$
 *
 * \param x x coordinate of the center of the NARR pixel from the ignition
 *          table
 * \param y y coordinate of the center of the NARR pixel from the ignition
 *          table
 * \param pszWkt the wkt of the point (x,y)
 * \return FE_NONE on success
 */
FE_Err
FIGGenerator::PlaceIgnition( double x, double y, const char *pszWkt )
{
    int nPixel, nLine;
    double dfStartX, dfStartY;
    double dfEndX, dfEndY;
    double dfValue;
    
    //assert( nPixel >= 0 && nLine >= 0 );
    const char *pszNarrWkt = GDALGetProjectionRef( hMaskDS );
    const char *pszIgnWkt = GDALGetProjectionRef( hIgnMaskDS );
    //poNarrMask->PixelIndex( x, y, &nPixel, &nLine, pszWkt );
    PixelIndex( hMaskDS, x, y, pszWkt, &nPixel, &nLine );
    //poNarrMask->PixelValue( x, y, &dfValue, pszWkt );
    PixelValue( hMaskDS, 1, x, y, &dfValue, pszWkt );

    //poNarrMask->PixelOrigin( nPixel, nLine, &dfStartX, &dfStartY, pszIgnWkt );
    PixelOrigin( hMaskDS, nPixel, nLine, &dfStartX, &dfStartY, pszIgnWkt );
    //poNarrMask->PixelOrigin( nPixel+1, nLine+1, &dfStartX, &dfStartY,
                             //pszIgnWkt );
    PixelOrigin( hMaskDS, nPixel+1, nLine+1, &dfEndX, &dfEndY, pszIgnWkt );

    double dfW = 0.0;
    double dfDeltaX;
    double dfDeltaY;
    double adfGeoTransform[6];
    GDALGetGeoTransform( hIgnMaskDS, adfGeoTransform );
    dfDeltaX = adfGeoTransform[1];
    dfDeltaY = adfGeoTransform[5];
    /*
    ** It is unclear why this was introduced.  It seems it would bias
    ** Towards the center all the time.
    */
    /* Pseudo cell center start */
    //dfStartX += ( dfDeltaX / 2 );
    //dfStartY += ( dfDeltaY / 2 );
    /* Random start */
    dfStartX += dfDeltaX * random.rand3();
    dfStartY += dfDeltaY * random.rand3();

    double dfNewX = dfStartX;
    double dfNewY = dfStartY;

    int err;

    while( dfNewY > dfEndY )
    {
        while( dfNewX < dfEndX )
        {
            //err = poIgnMask->PixelValue( dfNewX, dfNewY, &dfValue, pszIgnWkt );
            err = PixelValue( hIgnMaskDS, 1, dfNewX, dfNewY, &dfValue, pszIgnWkt );
            dfNewX += dfDeltaX;
            if( err != 0 )
            {
                continue;
            }
            if( dfValue <= 0.0 )
            {
                dfValue = NO_IGN_EPSILON;
                //dfValue = 0.0;
                /* no probability */
                //continue;
                dfW += dfValue;
            }
            else
            {
                dfW += log( dfValue + IGN_EPSILON );
            }
        }
        dfNewX = dfStartX;
        dfNewY += dfDeltaY;
    }
    /* No chance of ignition in the cell, don't put it down */
    if( dfW == 0.0 )
    {
        fprintf( stderr, "Could not place ignition in NARR cell\n" );
        return FE_NO_FUEL;
    }
    
    double dfProb = random.rand3();

    dfNewX = dfStartX;
    dfNewY = dfStartY;

    double dfSum = 0.0;
    while( dfNewY > dfEndY )
    {
        while( dfNewX < dfEndX )
        {
            //err = poIgnMask->PixelValue( dfNewX, dfNewY, &dfValue, pszIgnWkt );
            err = PixelValue( hIgnMaskDS, 1, dfNewX, dfNewY, &dfValue, pszIgnWkt );
            dfNewX += dfDeltaX;
            if( err != 0 )
            {
                continue;
            }
            if( dfValue <= 0.0 )
            {
                dfValue = NO_IGN_EPSILON;
                //dfValue = 0.0;
                /* no probability */
                //continue;
                dfSum += dfValue;
            }
            else
            {
                dfSum += log( dfValue + IGN_EPSILON ) / dfW;
            }
            if( dfSum > dfProb )
            {
                /* Don't judge, it actually makes sense here, I think */
                /* The done label is within site, see line 1512       */
                goto done;
            }
        }
        dfNewX = dfStartX;
        dfNewY += dfDeltaY;
    }
done:
    if( dfNewX != dfStartX )
    {
        dfNewX -= dfDeltaX;
    }
    double dfNewOriginX, dfNewOriginY;
    //poIgnMask->PixelIndex( dfNewX, dfNewY, &nPixel, &nLine, pszIgnWkt );
    PixelIndex( hIgnMaskDS, dfNewX, dfNewY, pszIgnWkt, &nPixel, &nLine );
    //poIgnMask->PixelOrigin( nPixel, nLine, &dfNewX, &dfNewY );
    PixelOrigin( hIgnMaskDS, nPixel, nLine, &dfNewOriginX, &dfNewOriginY );
    
    double dfRX, dfRY;
    int bFoundFuel = FALSE;
    int nTries = MAX_PLACEMENT_TRIES;
    int nTried = 0;
    int nFuel;
    PixelValue( hFuelPercMaskDS, 1, dfNewOriginX+dfDeltaX, dfNewOriginY+dfDeltaY, &dfValue, pszWkt );
    /* This was above the line above, wrong? */
    int nFuelProb = (int)dfValue;
    if( nFuelProb < 5 )
    {
        if( nFuelProb == 0 )
            nTries = 0;
        else
            nTries = 100;
    }

    while( !bFoundFuel && nTried < nTries )
    {
        dfNewX = dfNewOriginX;
        dfNewY = dfNewOriginY;
        dfRX = random.rand3();
        dfRY = random.rand3();
        dfNewX += dfRX * dfDeltaX;
        dfNewY += dfRY * dfDeltaY;
        //err = poFuel->PixelValue( dfNewX, dfNewY, &dfValue, pszIgnWkt );
        err = PixelValue( hFuelDS, 1, dfNewX, dfNewY, &dfValue, pszIgnWkt );
        nFuel = (int)dfValue; 
        if( (nFuel > 0 && nFuel < 90 ) || nFuel > 99 )
        {
            bFoundFuel = TRUE;
        }
        nTried++;
    }
    if( bFoundFuel == FALSE )
    {
        fprintf( stderr, "Could not locate sufficient fuel to place ignition\n" );
        return FE_NO_FUEL;
    }
    ReprojectPoint( &dfNewX, &dfNewY, pszIgnWkt, pszWkt );

    /* set the x and y coordinates */
    dfX = dfNewX;
    dfY = dfNewY;

    return 0;
}

/**
 * \brief Allow the default progress to be shown
 *
 * \param bShowProgress switch to see progress
 * \param papszOptions list of key value pairs, unused
 */
void
FIGGenerator::ShowProgress( int bShowProgress,
                            const char * const * papszOptions  )
{
    this->bShowProgress = bShowProgress;
}

