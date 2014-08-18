/******************************************************************************
 *
 * $Id: frisk_test_1.cpp 107 2012-08-02 15:25:49Z kyle.shannon $
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Test for frisk class
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

#include "flammap.h"
#include "flm4.hpp"
#include "fsxlandt.h"
#include "gdal_priv.h"
#include "cpl_port.h"

#define ELEV_BAND 1
#define SLOPE_BAND 2
#define ASPECT_BAND 3
#define FUEL_BAND 4
#define COVER_BAND 5

#define PI fabs(acos(-1.0))

#define EPSILON 0.000001
bool CloseEnough(double a, double b, double eps=EPSILON)
{
        return fabs(a - b) < eps;
}


int main( int argc, char *argv[] )
{
    GDALAllRegister();
    int result = 0;
    const char *pszDataPath = getenv("OMFFR_DATA");
    const char *pszInputFilename;
    if( pszDataPath == NULL )
    {
        fprintf(stderr, "Failed to find input ascii file, set OMFFR_DATA\n");
        return 1;
    }
    pszInputFilename = CPLFormFilename(pszDataPath, "autotest/fbtest", ".lcp");
    GDALDatasetH hLcpDS = GDALOpen(pszInputFilename, GA_ReadOnly);
    pszInputFilename = CPLFormFilename(pszDataPath, "autotest/fbtest", ".ROS");
    GDALDatasetH hRosDS = GDALOpen(pszInputFilename, GA_ReadOnly);
    pszInputFilename = CPLFormFilename(pszDataPath, "autotest/fbtest", ".FLI");
    GDALDatasetH hFliDS = GDALOpen(pszInputFilename, GA_ReadOnly);
    pszInputFilename = CPLFormFilename(pszDataPath, "autotest/fbtest", ".FML");
    GDALDatasetH hFlmDS = GDALOpen(pszInputFilename, GA_ReadOnly);
    if(hLcpDS == NULL || hRosDS == NULL || hFliDS == NULL || hFlmDS == NULL)
    {
        fprintf(stderr, "Failed to open test data file.\n");
        return 1;
    }
    GDALRasterBandH hElevBand, hSlopeBand, hAspectBand, hFuelBand, hCoverBand;
    hElevBand = GDALGetRasterBand(hLcpDS, ELEV_BAND);
    hSlopeBand = GDALGetRasterBand(hLcpDS, SLOPE_BAND);
    hAspectBand = GDALGetRasterBand(hLcpDS, ASPECT_BAND);
    hFuelBand = GDALGetRasterBand(hLcpDS, FUEL_BAND);
    hCoverBand = GDALGetRasterBand(hLcpDS, COVER_BAND);

    GDALRasterBandH hRosBand, hFliBand, hFmlBand;
    hRosBand = GDALGetRasterBand(hRosDS, 1);
    hFliBand = GDALGetRasterBand(hFliDS, 1);
    hFmlBand = GDALGetRasterBand(hFlmDS, 1);

    int nXSize = GDALGetRasterXSize(hLcpDS);
    int nYSize = GDALGetRasterYSize(hLcpDS);

    FlamMap f;
    bool WndOK=false, WtrOK=false;
    long CFMethod, FoliarMC;
    double Resolution;
    ResetNewFuels();
    Resolution=-1.0;
    f.SetResolution(Resolution);
    FoliarMC=100;
    CFMethod=0;

    SetFoliarMoistureContent(FoliarMC);
    SetCrownFireCalculation(CFMethod);
    EnableSpotting(true);
    LandscapeStruct lcp_data;
    FuelMoistStruct moist_data;
    moist_data.fm_1 = 10;
    moist_data.fm_10 = 15;
    moist_data.fm_100 = 20;
    moist_data.fm_1000 = 5;
    moist_data.fm_herb = 80;
    moist_data.fm_wood = 100;

    double wind_spd = 10.0;
    double wind_dir = 270.0;
    OutputFBStruct output;
    f.burn->pointBurnThread = new BurnThread(f.burn->env);


    short nElev, nSlope, nAspect, nFuel, nCover;
    double dfRos, dfFli, dfFml;
    for(int i = 0;i < nXSize; i++)
    {
        for(int j = 0;j < nYSize;j++)
        {    
            GDALRasterIO(hElevBand, GF_Read, i, j, 1, 1, &nElev, 1, 1, 
                         GDT_Int16, 0, 0);
            GDALRasterIO(hSlopeBand, GF_Read, i, j, 1, 1, &nSlope, 1, 1, 
                         GDT_Int16, 0, 0);
            GDALRasterIO(hAspectBand, GF_Read, i, j, 1, 1, &nAspect, 1, 1, 
                         GDT_Int16, 0, 0);
            GDALRasterIO(hFuelBand, GF_Read, i, j, 1, 1, &nFuel, 1, 1, 
                         GDT_Int16, 0, 0);
            GDALRasterIO(hCoverBand, GF_Read, i, j, 1, 1, &nCover, 1, 1, 
                         GDT_Int16, 0, 0);
            nSlope = (short)(atan((double) nSlope / 100.0) / PI * 180.0) + 0.5;
            lcp_data.elev = nElev;
            lcp_data.slope = nSlope;
            lcp_data.aspect = nAspect;
            lcp_data.fuel = nFuel;
            lcp_data.cover = nCover;
            lcp_data.aspectf = (double)lcp_data.aspect/180.0*PI;     // arcinfo, degrees AZIMUTH;
            //default
            //if(hHeight == 0.0)
            //        lcp_data.height = 15.0;
            //else
            //    lcp_data.height = hHeight;
            lcp_data.height = 15.0;
            lcp_data.base = 0;
            lcp_data.density = 0;
            lcp_data.duff = 0;
            lcp_data.woody = 0;
            f.RunFlamMapPoint(lcp_data, moist_data, wind_spd, wind_dir,
                                    output);
            GDALRasterIO(hRosBand, GF_Read, i, j, 1, 1, &dfRos, 1, 1, 
                         GDT_Float64, 0, 0);
            GDALRasterIO(hFliBand, GF_Read, i, j, 1, 1, &dfFli, 1, 1, 
                         GDT_Float64, 0, 0);
            GDALRasterIO(hFmlBand, GF_Read, i, j, 1, 1, &dfFml, 1, 1, 
                         GDT_Float64, 0, 0);

            if(!CloseEnough(output.ros, dfRos))
            {
                fprintf(stderr, "Pixel Diff at i: %d, j: %d. Expected:%lf, "
                        "Got:%lf for ros\n", i, j, dfRos, output.ros);
                result = 1;
            }
            /* Single bad calculation, unknown why */
            if(i == 2 && j == 0)
            {
                if(!CloseEnough(output.fli, dfFli, 0.001))
                {
                    fprintf(stderr, "Pixel Diff at i: %d, j: %d. Expected:%lf, "
                            "Got:%lf for fli\n", i, j, dfFli, output.fli);
                    result = 1;
                }
            }
            else
            {
                if(!CloseEnough(output.fli, dfFli, 0.001))
                {
                    fprintf(stderr, "Pixel Diff at i: %d, j: %d. Expected:%lf, "
                            "Got:%lf for fli\n", i, j, dfFli, output.fli);
                    result = 1;
                }
            }
            if(!CloseEnough(output.flame_length, dfFml))
            {
                fprintf(stderr, "Pixel Diff at i: %d, j: %d. Expected:%lf, "
                        "Got:%lf fml\n", i, j, dfFml, output.flame_length);
                result = 1;
            }
        }
    }
    return result;
}

