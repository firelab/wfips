#include "irs.h"

void Usage()
{
    printf( "irs_export_result [-f ogr_frmt] result_db output\n" );
    exit( 1 );
}

int main( int argc, char *argv[] )
{
    OGRRegisterAll();
    const char *pszDb = NULL;
    const char *pszOut = NULL;

    int i = 1;
    if( argc < 3 )
    {
        fprintf( stderr, "Invalid arguments\n" );
        Usage();
    }
    while( i < argc )
    {
        if( EQUAL( argv[i], "-f" ) )
        {
            fprintf( stderr, "Not supported\n" );
            Usage();
        }
        else if( pszDb == NULL )
        {
            pszDb = argv[i];
        }
        else if( pszOut == NULL )
        {
            pszOut = argv[i];
        }
        else
        {
            Usage();
        }
        i++;
    }

    IRSResult oResult( pszDb, "/home/kyle/src/omffr/trunk/data/omffr.sqlite",
                       1, TRUE, FALSE, NULL );
    int rc;
    //rc = oResult.PostProcessLargeFire( 0, 0.0, 10 );
    //rc = oResult.ExportToOgr( FPU_GEOMETRY, LARGE_FIRE |
    //                          FIRE_OUTCOME | RESC_USAGE, 
    //                          "ESRI Shapefile", NULL, NULL );
    //rc = oResult.ExportToOgr( FPU_GEOMETRY, FIRE_OUTCOME | RESC_USAGE,
    //                          "ESRI Shapefile", NULL, NULL );
    rc = oResult.ExportOgrSummary( "ESRI Shapefile", argv[2], NULL );
    return 0;
}

