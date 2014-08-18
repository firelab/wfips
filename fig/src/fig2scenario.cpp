/******************************************************************************
 *
 * $Id: fig_load.cpp 147 2012-08-21 16:04:38Z kyle.shannon $
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Write an output for IRS from FIG
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

//#include "fig.h"
#include "FWA.h"
#include "data_load.h"
#include "ogr_api.h"
#include "omffr_rand.h"
#include "omffr_conv.h"

void Usage()
{
    printf( "Usage: fig2scenario input_file output_file\n"                                );
    exit( 1 );
}

std::vector<CFWA>LoadFwas( const char *pszFwaFile )
{
    std::vector<CFWA>fwas;
    //if constructor opens a file
    ifstream inFWAFile( pszFwaFile, ios::in );

    //exit program if ifstream could not open file
    if ( !inFWAFile ) {
        cerr << "File could not be opened" << endl;
    //    exit( 1 );
    }
    // Enter elements for first FWA 
    string FWA( "Roadless_Stevensville" );
    string FMG( "Roadless" );
    int WalkInPct = 98;
    int PumpNRoll = 0;
    int Head = 100;
    int Tail = 0;
    int Parallel = 0;
    int AttDist = 0;
    bool WaterDrops = true;
    bool Excluded = false;
    double DiscSize = 0.01;
    int ESLTime = 1440;
    int ESLSize = 300;
    double AirtoGround = 1.6;
    int WalkInDelay[6] = { 0, 0, 30, 30, 15, 15 };
    int PostContUsed[6] = { 0, 0, 20, 20, 20, 20 };
    int PostContUnused[6] = { 0, 0, 0, 0, 0, 0, };
    int PostEscape[6] = { 0, 0, 20, 20, 20, 20 };
    int ReloadDelay[5] = { 0, 20, 20, 5, 25 };
    int FirstUnitDelay = 5;
    double Diurnal[24] = {0.10, 0.10, 0.10, 0.10, 0.10, 0.10,
                          0.10, 0.10, 0.10, 0.10, 0.20, 0.40, 
                          0.70, 1.00, 1.00, 1.00, 1.00, 1.00, 
                          1.00, 1.00, 0.70, 0.50, 0.30, 0.20 };
    string ROSAdjFuel[10] = { "x", "x", "x", "x", "x", "x", "x", "x", "x", "x" };
    double ROSAdjustment[10] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    int Index = 0;
    string DispLogicId( "FMU12" );
    double Latitude = 0;
    double Longitude = 0;

    // Read in each FWA entry from file
    while ( inFWAFile >> FWA >> FMG >> WalkInPct >> PumpNRoll >> Head >> Tail >> Parallel >> AttDist >> WaterDrops >> Excluded 
            >> DiscSize >> ESLTime >> ESLSize >> AirtoGround )    {
        for ( int i = 0; i < 6; i++ )
            inFWAFile >> WalkInDelay[i];
        for ( int i = 0; i < 6; i++ )
            inFWAFile >> PostContUsed[i];
        for ( int i = 0; i < 6; i++ )
            inFWAFile >> PostContUnused[i];
        for ( int i = 0; i < 6; i++ )
            inFWAFile >> PostEscape[i];
        for ( int i = 0; i < 5; i++ )
            inFWAFile >> ReloadDelay[i];
            inFWAFile >> FirstUnitDelay;
        for ( int i = 0; i < 24; i++ )
            inFWAFile >> Diurnal[i];
        for ( int i = 0; i < 10; i++ )
            inFWAFile >> ROSAdjFuel[i];
        for ( int i = 0; i < 10; i++ )
            inFWAFile >> ROSAdjustment[i];
        inFWAFile >> Index >> DispLogicId >> Latitude >> Longitude;

        int bp[5] = { 0,0,0,0,0 };
        int rn[13][5] = { {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1},
                          {1,1,1,1,1} };

        CDispLogic logic("", "", 3, bp, rn );
		string FPU = "GB_ID_002";

        fwas.push_back( CFWA( FWA, FMG, WalkInPct, PumpNRoll, Head, Tail, Parallel, AttDist, WaterDrops, Excluded, DiscSize,
            ESLTime, ESLSize, AirtoGround, WalkInDelay, PostContUsed, PostContUnused, PostEscape, ReloadDelay, FirstUnitDelay, 
            Diurnal, ROSAdjFuel, ROSAdjustment, 0, logic, FPU ));
            
    }    
    return fwas;
}

CFWA * FindFwa( std::vector<CFWA>fwas, const char *pszFwaName )
{
    std::string oFwaName( pszFwaName );
    for( int i = 0; i < fwas.size(); i++ )
    {
        if( fwas[i].GetFWAID() == oFwaName )
            return new CFWA( fwas[i] );
    }
    return NULL;
}

void LaunderFwaName( char *abyName )
{
    for( int i = 0; i < strlen( abyName ); i++ )
    {
        if( abyName[i] == ' ' )
            abyName[i] = '_';
    }
}

/* What we need:  specific condition, is walk in , tactic, distance elevation
 * minsteps, maxsteps, waterdrops, pump/roll, fwa id.
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
    int nLimit = 0;
    int bProgress = TRUE;
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
        CPLError( CE_Failure, CPLE_OpenFailed, "No input file provided\n");
        Usage();
    }
    if( pszOutputfile == NULL )
    {
        CPLError( CE_Failure, CPLE_OpenFailed, "Invalid output selected, "
                "use database and table or output file\n" );
        Usage();
    }

    pszDataPath = CPLGetConfigOption( "OMFFR_DATA", NULL );

    OGRDataSourceH hInputDS = OGROpen( pszInputfile, FALSE, NULL );
    if( hInputDS == NULL )
    {
        CPLError( CE_Failure, CPLE_OpenFailed, "Cannot open input file\n" );
        Usage();
    }

    int year, num, day;
    const char *dow, *disc_time;
    int bi;
    double ros;
    int fuel;
    const char *spec_cond;
    int slope, walkin;
    const char *tactic;
    double dist;
    int elev;
    double ltow;
    int minsteps = 250;
    int maxsteps = 10000;
    const char *sunrise, *sunset;
    int waterdrops, pumproll;
    char *abyFwa;
    const char *fwaid;
    double lon, lat;

    OGRLayerH hInputLayer;
    hInputLayer = OGR_DS_GetLayerByName( hInputDS, CPLGetBasename( pszInputfile ) );
    OGRFeatureDefnH hInputFeatureDefn;

    OGRFeatureH hInputFeature;

    OGRGeometryH hGeometry;

    hInputFeatureDefn = OGR_L_GetLayerDefn( hInputLayer );

    const char *pszTmpFilename =CPLFormFilename( pszDataPath, "irs/FWA", ".dat" );
    std::vector<CFWA>fwas = LoadFwas( pszTmpFilename );
    
    int nFeatures = OGR_L_GetFeatureCount( hInputLayer, TRUE );
    FILE *fout = fopen( pszOutputfile, "w" );

    //CFWA *fwa;

    Random random;

    char pszDb[8192];
    sprintf( pszDb, "%s/omffr.sqlite", pszDataPath );

    IRSDataAccess *poDA = IRSDataAccess::Create( 0, pszDb );
    int rc;
    sqlite3 *db;
    rc = sqlite3_open_v2( pszDb, &db, SQLITE_OPEN_READONLY, NULL );
    rc = sqlite3_enable_load_extension( db, 1 );
    rc = sqlite3_load_extension( db, "/usr/local/lib/libspatialite.so", 0, NULL );
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, "SELECT * from fwa join fwa_bndry USING(fwa_gis_id) " \
                                 "WHERE ST_Contains(fwa_bndry.geometry, MakePoint(?, ?, 4269))",
                                 -1, &stmt, NULL );

    if(rc)
    {
        CPLError( CE_Failure, CPLE_AppDefined, "Could not open DB");
    }

    GDALTermProgress( 0.0, NULL, NULL );
    OGR_L_ResetReading( hInputLayer );
    const char *pszFwaName;
    int nDone = 0;
    while( ( hInputFeature = OGR_L_GetNextFeature( hInputLayer ) ) != NULL )
    {
        /*
        fwaid = OGR_F_GetFieldAsString( hInputFeature,
                                        OGR_FD_GetFieldIndex( hInputFeatureDefn,
                                                              "fwa_name" ) );
        abyFwa = CPLStrdup( fwaid );
        LaunderFwaName( abyFwa );
        fwa = FindFwa( fwas, abyFwa );
        if( fwa == NULL )
        {
            CPLError( CE_Warning, CPLE_FileIO,
                    "Could not load fwa (%s)from file, missing\n", abyFwa );
            continue;
        }
        */
        /* Get fwa by point */
        hGeometry = OGR_F_GetGeometryRef( hInputFeature );
        /* Try to handle non-geometry types (csv) */
        if( hGeometry != NULL )
        {
            lat = OGR_G_GetY( hGeometry, 0 );
            lon = OGR_G_GetX( hGeometry, 0 );
        }
        else
        {
            lat = OGR_F_GetFieldAsDouble( hInputFeature, OGR_FD_GetFieldIndex( hInputFeatureDefn, 
                                                            "Y") );
            lon = OGR_F_GetFieldAsDouble( hInputFeature, OGR_FD_GetFieldIndex( hInputFeatureDefn, 
                                                            "X") );
        }

        std::string oFwaName = poDA->PointQuery( "fwa_bndry", "fwa_lndr_name",
                                                 lon, lat );
        rc = sqlite3_bind_double( stmt, 1, lon );
        rc = sqlite3_bind_double( stmt, 2, lat );
        //sqlite3_bind_text( stmt, 1, oFwaName.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step( stmt );
        if( rc != SQLITE_ROW && rc != SQLITE_DONE )
        {
            CPLError( CE_Warning, CPLE_FileIO,
                    "Could not load fwa (%s)from file, missing\n", oFwaName.c_str() );
            sqlite3_reset(stmt);
            continue;
        }

        int nFwaWalkIn, nFwaHead, nFwaTail, nFwaPara, nFwaAttackD;
        int nFwaWaterDrop, nFwaPumpRoll;
        nFwaWalkIn = sqlite3_column_int( stmt, 4 );
        nFwaHead = sqlite3_column_int( stmt, 6 );
        nFwaTail = sqlite3_column_int( stmt, 7 );
        nFwaPara = sqlite3_column_int( stmt, 8 );
        nFwaAttackD = sqlite3_column_int( stmt, 9 );
        nFwaWaterDrop = sqlite3_column_int( stmt, 10 );
        nFwaPumpRoll = sqlite3_column_int( stmt, 5 );

        year = OGR_F_GetFieldAsInteger( hInputFeature,
                                        OGR_FD_GetFieldIndex( hInputFeatureDefn,
                                                              "year" ) );
        num = OGR_F_GetFieldAsInteger( hInputFeature, OGR_FD_GetFieldIndex( hInputFeatureDefn,
                                                           "fire_num" ) );
        day = OGR_F_GetFieldAsInteger( hInputFeature, OGR_FD_GetFieldIndex( hInputFeatureDefn,
                                                           "day" ) );
        dow = OGR_F_GetFieldAsString( hInputFeature, OGR_FD_GetFieldIndex( hInputFeatureDefn,
                                                           "week_day" ) );
        disc_time = OGR_F_GetFieldAsString( hInputFeature, OGR_FD_GetFieldIndex( hInputFeatureDefn,
                                                                "disc_time" ) );
        bi = OGR_F_GetFieldAsInteger( hInputFeature, OGR_FD_GetFieldIndex( hInputFeatureDefn,
                                                          "bi" ) );
        ros = OGR_F_GetFieldAsDouble( hInputFeature, OGR_FD_GetFieldIndex( hInputFeatureDefn,
                                                           "ros" ) );
        fuel = OGR_F_GetFieldAsInteger( hInputFeature, OGR_FD_GetFieldIndex( hInputFeatureDefn,
                                                            "fuel" ) );
        spec_cond = OGR_F_GetFieldAsString( hInputFeature, OGR_FD_GetFieldIndex( hInputFeatureDefn,
                                                                "spec_cond" ) );
        slope = OGR_F_GetFieldAsInteger( hInputFeature, OGR_FD_GetFieldIndex( hInputFeatureDefn,
                                                             "slope_perc" ) );
        //if( random.rand3() * 100 > fwa->GetWalkInPct() )
        if( random.rand3() * 100 > nFwaWalkIn )
            walkin = 0;
        else
            walkin = 1;
        //if( fwa->GetHead() == 100 )
        if( nFwaHead == 100 )
            tactic = "HEAD\0";
        //else if( fwa->GetTail() == 100 )
        else if( nFwaTail == 100 )
            tactic = "TAIL\0";
        //else if( fwa->GetParallel() == 100 )
        else if( nFwaTail == 100 )
            tactic = "PARALLEL\0";
        else
        {
            int r = (int)(random.rand3() * 100 );
            int total = 0;
            if( r < nFwaHead )
                tactic = "HEAD\0";
            else if( r < nFwaTail + nFwaTail )
                tactic = "TAIL\0";
            else
                tactic = "PARALLEL\0";
        }
        //dist = fwa->GetAttackDist();
        dist = nFwaAttackD;
        elev = OGR_F_GetFieldAsInteger( hInputFeature, OGR_FD_GetFieldIndex( hInputFeatureDefn,
                                                            "elev" ) );
        ltow = OGR_F_GetFieldAsDouble( hInputFeature, OGR_FD_GetFieldIndex( hInputFeatureDefn,
                                                           "ratio" ) );
        sunrise = OGR_F_GetFieldAsString( hInputFeature, OGR_FD_GetFieldIndex( hInputFeatureDefn,
                                                             "sunrise" ) );
        sunset = OGR_F_GetFieldAsString( hInputFeature, OGR_FD_GetFieldIndex( hInputFeatureDefn,
                                                             "sunset" ) );
        //if( fwa->GetWaterDrops() )
        if( nFwaWaterDrop )
            waterdrops = TRUE;
        else
            waterdrops = FALSE;
        //if( fwa->GetPumpnRoll() )
        if( nFwaPumpRoll )
            pumproll = TRUE;
        else
            pumproll = FALSE;

        fprintf( fout, "%d %d %d %s %s "
                       "%d %lf %d %s %d "
                       "%d %s %lf %d %lf "
                       "%d %d %s %s %d "
                       "%d %s %lf %lf\n",
                 year, num, day, dow, disc_time,
                 bi, ros, fuel, spec_cond, slope, 
                 walkin, tactic, dist, elev, ltow, 
                 minsteps, maxsteps, sunrise, sunset, waterdrops, 
                 pumproll, /* abyFwa */ oFwaName.c_str(), lat, lon );

        sqlite3_reset(stmt);
        nDone++;
        GDALTermProgress( (float)nDone / (float)nFeatures, NULL, NULL );
    }
    GDALTermProgress( 1.0, NULL, NULL );
    fclose( fout );
    OGR_DS_Destroy( hInputDS );

    return 0;
}

