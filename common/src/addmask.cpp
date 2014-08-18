
#include "gdal.h"
#include "gdal_priv.h"
#include "cpl_conv.h"

void Usage()
{
    printf("addmask [-p] [-mask_nodata] [-co NAME=VALUE] [-h] input_file\n");
    printf("        output_file maskvalues\n"                              );
    printf("\n"                                                            );
    printf("-p           -> show progress\n"                               );
    printf("-mask_nodata -> mask the no data value as well as any other\n" );
    printf("                supplied value\n"                              );
    printf("-of          -> output format\n"                               );
    printf("-co          -> pass a creation option to the output driver\n" );
    printf("-h           -> Show this message, then exit\n"                );
    printf("input_file   -> input file\n"                                  );
    printf("output_file  -> output file\n"                                 );
    printf("maskvalues   -> values to identify and set mask to 0, other\n" );
    printf("                values are set to 255\n"                       );
    exit(1);
}

int main(int argc, char *argv[])
{
    const char *pszInputfile = NULL;
    const char *pszOutputfile = NULL;
    const char *pszFormat = "GTiff";
    int bProgress = FALSE;
    int bMaskNoData = FALSE;
    GDALDatasetH hSrcDS;
    GDALDatasetH hMaskDS;
    GDALDriverH hDriver;
    double adfGeoTransform[6];
    const char *pszWkt;
    int nXSize, nYSize;
    int nMaskCount = 0;
    int nPixelsMasked = 0;
    double adfMaskValues[1024];
    int anMaskFound[1024];
    int bMaskProvided = FALSE;
    double dfNoData;
    int i, j, k;
    GDALRasterBandH hSrcBand, hMaskBand;
    double *padfScanline;
    unsigned char *pabyScanline;
    int nHasNoData;
    char** papszCreationOptions = NULL;

    i = 1;
    while( i < argc )
    {
        if( EQUAL( argv[i], "-p" ) )
        {
            bProgress = TRUE;
        }
        else if( EQUAL( argv[i], "-mask_nodata" ) )
        {
            bMaskNoData = TRUE;
            bMaskProvided = TRUE;
        }
        else if( EQUAL( argv[i], "-of" ) )
        {
            pszFormat = argv[++i];
        }
        else if( EQUAL( argv[i], "-co" ) )
        {
            papszCreationOptions = CSLAddString(papszCreationOptions,
                                                argv[++i]);
        }
        else if( EQUAL( argv[i], "-h" ) )
        {
            Usage();
        }
        else if( pszInputfile == NULL )
        {
            pszInputfile = argv[i];
        }
        else if( pszOutputfile == NULL )
        {
            pszOutputfile = argv[i];
        }
        else
        {
            adfMaskValues[nMaskCount++] = atof(argv[i]);
            bMaskProvided = TRUE;
        }
        //else
        //{
        //    Usage();
        //}
        i++;
    }

    if( pszInputfile == NULL )
    {
        fprintf( stderr, "No input file provided\n");
        Usage();
    }
    if( pszOutputfile == NULL )
    {
        fprintf( stderr, "No output file provided\n" );
        Usage();
    }
    GDALAllRegister();

    hSrcDS = GDALOpen(pszInputfile, GA_ReadOnly);
    if(hSrcDS == NULL)
    {
        CPLError(CE_Fatal, CPLE_OpenFailed, "Failed to open source dataset");
    }

    hDriver = GDALGetDriverByName(pszFormat);
    if(hDriver == NULL)
    {
        CPLError(CE_Fatal, CPLE_NotSupported, "Failed to load output driver");
    }

    nXSize = GDALGetRasterXSize(hSrcDS);
    nYSize = GDALGetRasterYSize(hSrcDS);
    hMaskDS = GDALCreate(hDriver, pszOutputfile, nXSize, nYSize, 1, GDT_Byte,
                         papszCreationOptions);
    pszWkt = GDALGetProjectionRef(hSrcDS);
    if(pszWkt != NULL)
    {
        GDALSetProjection(hMaskDS, pszWkt);
    }
    if(GDALGetGeoTransform(hSrcDS, adfGeoTransform) == CE_None)
    {
        GDALSetGeoTransform(hMaskDS, adfGeoTransform);
    }
    hSrcBand = GDALGetRasterBand(hSrcDS, 1);
    hMaskBand = GDALGetRasterBand(hMaskDS, 1);

    nHasNoData = FALSE;
    if(bMaskNoData)
    {
        dfNoData = GDALGetRasterNoDataValue(hSrcBand, &nHasNoData);
        if(nHasNoData)
        {
            adfMaskValues[nMaskCount++] = dfNoData;
        }
    }
    for(i = 0;i < nMaskCount;i++)
    {
        anMaskFound[i] = 0;
    }

    padfScanline = (double*)CPLMalloc(nXSize * sizeof(double));
    pabyScanline = (unsigned char*)CPLMalloc(nXSize * sizeof(unsigned char));

    if(bProgress)
    {
        GDALTermProgress(0.0, NULL, NULL);
    }

    for(i = 0;i < nYSize;i++)
    {
        GDALRasterIO(hSrcBand, GF_Read, 0, i, nXSize, 1, padfScanline, nXSize,
                     1, GDT_Float64, 0, 0);
        for(j = 0; j < nXSize;j++)
        {
            pabyScanline[j] = 255;
            for(k = 0; k < nMaskCount; k++)
            {
                if(CPLIsEqual(padfScanline[j], adfMaskValues[k]))
                {
                    pabyScanline[j] = 0;
                    anMaskFound[k]++;
                    nPixelsMasked++;
                    break;
                }
            }
        }
        GDALRasterIO(hMaskBand, GF_Write, 0, i, nXSize, 1, pabyScanline, 
                     nXSize, 1, GDT_Byte, 0, 0);
        if(bProgress)
        {
            GDALTermProgress((double)i / (double)nYSize, NULL, NULL);
        }
    }
    if(bProgress)
    {
        GDALTermProgress(1.0, NULL, NULL);
    }

    for(i = 0; i < nMaskCount;i++)
    {
        CPLDebug("addmask", "%d pixels were masked for value: %lf",
                 anMaskFound[i], adfMaskValues[i]);
    }
    CPLDebug("addmask", "%d pixels were masked total", nPixelsMasked);

    CPLFree(padfScanline);
    CPLFree(pabyScanline);

    GDALClose(hSrcDS);
    GDALClose(hMaskDS);
}

