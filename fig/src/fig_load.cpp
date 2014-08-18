/******************************************************************************
 *
 * $Id: fig_load.cpp 414 2013-08-23 20:11:09Z kyle.shannon $
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

#include "fig_api.h"

void Usage()
{
    printf( "Usage: fig_load [-p] [-b buffer] [-d data_path] [-of format]\n" );
    printf( "                [-co KEY=VALUE] [-t path]\n                   " );
    printf( "                input_file output_file\n                      " );
    exit( 1 );
}

/**
 * Read in output from Isaac and populate a sqlite db with the read in values
 * as well as calculated values using erc.  Calculated values include:
 */
int main( int argc, char *argv[] )
{
    GDALAllRegister();
    OGRRegisterAll();
    const char *pszInputfile = NULL;
    const char *pszOutputfile = NULL;
    const char *pszOutputFormat = "CSV";
    char **papszCreateOptions = NULL;
    const char *pszDataPath = NULL;
    const char *pszFpuCode = NULL;
    const char *pszTreatPath = NULL;
    int nLimit = 0;
    int bProgress = FALSE;
    double dfMaxX, dfMinX, dfMaxY, dfMinY;
    int bLimit = FALSE;
    double dfBuffer = 0.0;

    int i = 1;
    while( i < argc )
    {
        if( EQUAL( argv[i], "-p" ) )
        {
            bProgress = TRUE;
        }
        else if( EQUAL( argv[i], "-d" ) )
        {
            pszDataPath = argv[++i];
        }
        else if( EQUAL( argv[i], "-of" ) )
        {
            pszOutputFormat = argv[++i];
        }        
        else if( EQUAL( argv[i], "-co" ) )
        {
            papszCreateOptions = CSLAddString( papszCreateOptions, argv[++i] );
        }
        else if( EQUAL( argv[i], "-sl" ) )
        {
            dfMaxX = atof(argv[++i]);
            dfMinX = atof(argv[++i]);
            dfMaxY = atof(argv[++i]);
            dfMinY = atof(argv[++i]);
            bLimit = TRUE;
        }
        else if( EQUAL( argv[i], "-fpu" ) )
        {
            pszFpuCode = argv[++i];
        }
        else if( EQUAL( argv[i], "-b" ) )
        {
            dfBuffer = atof( argv[++i] );
        }
        else if( EQUAL( argv[i], "-l" ) )
        {
            nLimit = atoi( argv[++i] );
        }
        else if( EQUAL( argv[i], "-t" ) )
        {
            pszTreatPath = argv[++i];
            papszCreateOptions = CSLAddNameValue( papszCreateOptions,
                                                  "TREATMENT_PATH",
                                                  pszTreatPath );
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
            Usage();
        }
        i++;
    }

    if( pszInputfile == NULL )
    {
        fprintf( stderr, "No input file provided\n");
        Usage();
    }
    if( pszOutputfile == NULL )
    {
        fprintf( stderr, "Invalid output selected, use database and table or "
                         "output file\n" );
        Usage();
    }
    if( bLimit && pszFpuCode )
    {
        printf( "Specify one of the options '-fpu' or '-sl', not both\n" );
        Usage();
    }    
    FIGGeneratorH hFig;
    hFig = FIGCreate( NULL );
    FE_Err error;
    if( pszDataPath != NULL )
    {
        FIGSetDataPath( hFig, pszDataPath, NULL );
    }
    if( bProgress )
        FIGShowProgress( hFig, 1, NULL);
    error = FIGSetFormat( hFig, pszOutputFormat );

    if( pszFpuCode != NULL )
    {
        error = FIGSetFpuCode( hFig, pszFpuCode, NULL );
        if( error != 0 )
        {
            printf( "Failed to subset fpu.  Cannot read fpu file or find fpu\n" );
            Usage();
        }
    }

    if( bLimit )
    {
        FIGSetSpatialEnv( hFig, dfMaxX,  dfMinX, dfMaxY, dfMinY,
                          NULL );
    }

    error = FIGGenerateFromFile( hFig, pszInputfile, pszOutputfile, papszCreateOptions );
    if( error != FE_NONE )
    {
        fprintf( stderr, "%s\n", FIGGetLastErrorMessage( hFig, NULL ) );
        FIGDestroy( hFig, NULL );
        exit( error );
    }
    FIGDestroy( hFig, NULL );
    hFig = NULL;
    CSLDestroy( papszCreateOptions );

    return 0;
}

