/******************************************************************************
 *
 * $Id: fig.h 414 2013-08-23 20:11:09Z kyle.shannon $
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

#ifndef FIG_H_
#define FIG_H_

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#include <iostream>
using std::ifstream;

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#ifndef PI
#define PI fabs(acos(-1.0))
#endif

#include "gdal_priv.h"
#include "ogr_api.h"
#include "ogr_srs_api.h"

#include "frisk.h"
#include "nfdr32.h"
#include "omffr_conv.h"
#include "flammap.h"
#include "raster_access.h"

/**
 * \class FIGGenerator fig.h
 *
 * \brief Generate a fire ignition table based on an input text file or weather
 *
 * This class provides the capability to read in a stream of generated
 * ignitions and set values needed for the Initial Response Simulator (IRS).
 * These values include fire behavior values (flame length and spread rate) and
 * fire danger rating values (burning index, spread component).  The values are
 * extracted from a binary Fire Risk (frisk) grid.  This grid is a packed
 * 32-km 'grid' of the US.  An associated mask is checked for valid data.  If
 * the mask is false, the values at that point are undefined and should be
 * considered garbage.  Fuel moistures and wind values are extracted from the
 * frisk grid and used to calculate the nfdrs and fire behavior values.  The
 * values are written to a space delimited file.  This class relies on GDAL
 * datasets and the FRisk class that stores and accesses frisk data.
 *
 * \author Kyle Shannon
 * \date 2012-07-24
 */

#define BUF_SIZE 512
#define OUTPUT_BUF_SIZE 8192
#define FUEL_MODEL_G 71

/* Default file names and constants */
#define ASPECT_FILE "lcp/small_aspect.tif"
#define BASE_FILE "lcp/chuck/base.tif"
#define COVER_FILE "lcp/chuck/cover.tif"
#define DENSITY_FILE "lcp/chuck/density.tif"
#define ELEVATION_FILE "lcp/small_elevation.tif"
#define FUEL_FILE "lcp/chuck/fuel40.tif"
#define HEIGHT_FILE "lcp/chuck/height.tif"
#define SLOPE_FILE "lcp/small_slope.tif"
#define MASK_FILE "narr/narr_mask_byte.tif"
#define FRISK_BINARY "narr/narr_32km_frisk.flt"
#define FWA_SHAPE "admin/FWA_20120702.zip/FWA_20120702.shp"
#define FPU_SHAPE "admin/FPA_FPU_20120228.zip/National_FPU_Updates_2013AY_20120228.shp"
#define FWA_RASTER "admin/fwa.tif"
#define FWA_FTR_MASK "ign/fwa_mask.tif"
#define IGN_MASK "ign/ign_prob2.tif"
#define FUEL_PERC "ign/fuel_perc_mask.tif"

#define ALBERS_WKT "PROJCS[\"USA_Contiguous_Albers_Equal_Area_Conic_USGS_version\"," "GEOGCS[\"GCS_North_American_1983\",DATUM[\"D_North_American_1983\"," "SPHEROID[\"GRS_1980\",6378137.0,298.257222101]]," "PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]]," "PROJECTION[\"Albers\"],PARAMETER[\"False_Easting\",0.0]," "PARAMETER[\"False_Northing\",0.0]," "PARAMETER[\"Central_Meridian\",-96.0]," "PARAMETER[\"Standard_Parallel_1\",29.5]," "PARAMETER[\"Standard_Parallel_2\",45.5]," "PARAMETER[\"Latitude_Of_Origin\",23.0],UNIT[\"Meter\",1.0]]"
#define NAD83_WKT "GEOGCS[\"NAD83\", DATUM[\"North_American_Datum_1983\", SPHEROID[\"GRS 1980\",6378137,298.257222101, AUTHORITY[\"EPSG\",\"7019\"]], TOWGS84[0,0,0,0,0,0,0], AUTHORITY[\"EPSG\",\"6269\"]], PRIMEM[\"Greenwich\",0, AUTHORITY[\"EPSG\",\"8901\"]], UNIT[\"degree\",0.0174532925199433, AUTHORITY[\"EPSG\",\"9108\"]], AUTHORITY[\"EPSG\",\"4269\"]]"
#define NARR_WKT "PROJCS[\"NARR Lambert\", GEOGCS[\"Sphere\", DATUM[\"Sphere\", SPHEROID[\"Sphere\",6371000,0]], PRIMEM[\"Greenwich\",0], UNIT[\"degree\",0.0174532925199433]], PROJECTION[\"Lambert_Conformal_Conic_2SP\"], PARAMETER[\"standard_parallel_1\",50], PARAMETER[\"standard_parallel_2\",50], PARAMETER[\"latitude_of_origin\",50], PARAMETER[\"central_meridian\",-107], PARAMETER[\"false_easting\",0], PARAMETER[\"false_northing\",0], UNIT[\"metre\",1, AUTHORITY[\"EPSG\",\"9001\"]]]" 
#define IGN_EPSILON 0.00000001
#define NO_IGN_EPSILON 0.01
#define MAX_PLACEMENT_TRIES 1000;

typedef int FE_Err;
static const int FE_NONE = 0;
static const int FE_FILE_IO = 1;
static const int FE_MEMORY = 2;
static const int FE_OBJECT_NULL = 3;
static const int FE_GDAL_ERR = 4;
static const int FE_INVALID_INPUT = 5;
static const int FE_NO_FUEL = 6;

class FIGGenerator
{
public:
    FIGGenerator();
    FIGGenerator( const char * const * papszOptions = NULL );
    FIGGenerator( FIGGenerator &rhs );
    ~FIGGenerator();

    FE_Err GenerateFromFile( const char *pszInputFilename,
                                const char *pszOutputFilename,
                                const char * const * papszOptions = NULL );
    FE_Err GenerateFromWeather( const char *pszInputPath,
                                const char *pszOutputFilename,
                                const char * const * papszOptions = NULL );
    FE_Err SetDataPath( const char *pszPath,
                        const char * const * papszOptions = NULL );
    FE_Err SetInputWkt( const char *pszWkt,
                        const char * const * papszOptions = NULL );
    FE_Err SetFormat( const char *pszFormat,
                      const char * const * papszOptions = NULL );
    FE_Err SetSpatialEnv( double n, double s, double e, double w,
                          const char * const * papszOptions = NULL );
    FE_Err SetFpuCode( const char *pszFpuCode, 
                       const char * const * papszOptions = NULL );
    void ShowProgress( int bShowProgress,
                       const char * const * papszOptions = NULL );

    FE_Err GetLastErrorCode( const char * const * papszOptions = NULL );
    const char *GetLastErrorMessage( const char * const * papszOptions = NULL );

private:
    /* Options */
    enum PointInPolygonSearch
    {
        PIP_RASTER,
        PIP_HYBRID,
        PIP_REAL
    };
    PointInPolygonSearch ePointSearch;
    int bReadInZip;
    const char * pszFormat;
    int nSqliteTransactions;

    int bAddTreated;

    /* End of options */
    Random random;

    FRisk frisk;
    NFDRCalc nfdr;
    FlamMap flammap;
    RasterAccess *poAspect, *poBase, *poCover, *poDensity, *poElev, *poFuel,
                 *poHeight, *poSlope, *poNarrMask, *poFuelPercMask;
    GDALDatasetH hAspectDS, hBaseDS, hCoverDS, hDensityDS,
                 hElevDS, hFuelDS, hHeightDS, hSlopeDS, hMaskDS, hFuelPercMaskDS;

    GDALDatasetH hBaseTreatDS, hCoverTreatDS, hDensityTreatDS, hFuelTreatDS,
                 hHeightTreatDS;

    RasterAccess *poFwaRaster, *poFwaMask;
    RasterAccess *poIgnMask;
    GDALDatasetH hFwaRasterDS, hFwaFeatureMask;
    GDALDatasetH hIgnMaskDS;

    OGRDataSourceH hFwaDS;
    OGRLayerH hFwaLayer;
    OGRFeatureDefnH hFwaFeatureDefn;
    OGRFeatureH hFwaFeature;
    OGRGeometryH hFwaGeometry;
    OGRGeometryH hPoint;
    FILE *fin;
    ifstream friskStream;
    OGRSFDriverH hOutputDriver;
    OGRDataSourceH hOutputDS;
    OGRLayerH hOutputLayer;
    OGRFieldDefnH hOutputFieldDefn;
    OGRFeatureH hOutputFeature;
    OGRGeometryH hOutputPoint;

    FILE *fout;

    const char *pszInputFilename;
    const char *pszOutputFilename;
    const char *pszAspectFilename;
    const char *pszBaseFilename;
    const char *pszCoverFilename;
    const char *pszDensityFilename;
    const char *pszElevFilename;
    const char *pszFuelFilename;
    const char *pszHeightFilename;
    const char *pszSlopeFilename;
    const char *pszNarrMaskFilename;
    const char *pszFriskBinaryFilename;
    const char *pszFwaShpFilename;
    const char *pszFpuShpFilename;
    const char *pszFwaRasterFilename;
    const char *pszFwaMaskFilename;
    const char *pszIgnProbFilename;
    const char *pszFuelPercFilename;
    const char *pszFpuCode;

    const char *pszInputWkt;
    const char *pszDataPath;
    int bOverwrite;
    int bShowProgress;

    double dfMaxX, dfMinX, dfMaxY, dfMinY;
    int bEnvSet;

    int nMaxPlaceAttempts;
    int nMaxRetryAttempts;
    double dfDefaultProb;

    /* Input/Output */
    double dfX;
    double dfY;
    int nCause;
    double dfErc;
    int nErc;
    FuelMoistStruct oFuelMoisture;
    double dfWindSpeed;
    double dfWindDir;
    LandscapeStruct oLcpData;
    OutputFBStruct oOutputFB;

    int nYear;
    int nFireNum;
    int nJulDay;
    int nDayOfWeek;
    int nDiscTime;
    int nNfdrBurnIndex;
    int nNfdrSpreadComp;
    double dfFlmpSpreadRate;
    int nElev;
    int nFuelModel;
    const char *pszSpecCond;
    int nSlopePercent;
    double dfFlmpLenToWid;
    int nSunRise;
    int nSunSet;
    int nFwaId;
    const char *pszFwaName;
    double lon;
    double lat;
    int nFlmpFlameLength;

    char abyOutputBuffer[OUTPUT_BUF_SIZE];

    void Initialize();

    FE_Err InitializeFiles();
	FE_Err InitializeTreated( const char *pszPath );
    FE_Err CreateRasterAccess();
    FE_Err CloseFiles();
	FE_Err CloseTreated();
    FE_Err WriteToOGR();
    FE_Err WriteTextToFile();
    FE_Err PlaceIgnition( double x, double y, const char *pszWkt );
    FE_Err SlopeClass( int slope, int *slopeClass );
    FE_Err FetchLandscapeData( double x, double y, const char *pszWkt );
    FE_Err FetchTreatedData( double x, double y, const char *pszWkt );
    FE_Err FetchFriskData( double x, double y, const char *pszWkt );
    FE_Err FetchFwaId( double x, double y, const char *pszWkt );
    FE_Err RunNfdr();
    FE_Err RunFlamMap();
    FE_Err GetSpecCond( int nFuel, int nFwa=0 );
    int GetDayOfWeek( int y, int d );

    const char *pszLastErrorMessage;
    FE_Err nLastErrorCode;
    FE_Err ReportError( FE_Err eCode, const char *pszMessage );
};

#endif /* FIG_H_ */
