/******************************************************************************
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  IRS command line interface for examples
 * Author:   Kyle Shannon <kyle at pobox dot com>
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

#include "irs.h"

void Usage()
{
    printf("irs\n");
}

int i = 0;
const int nReduceCount = 500;
//const int nReduceCount = 20;
char pszOut[8192];
char pszOutLast[8192];
char pszShapeOut[8192];

int irs_cplusplus( int argc, char *argv[] )
{
    IRSSuite *poSuite;
    poSuite = (IRSSuite*)IRSSuite::Create( "/home/kyle/src/omffr/trunk/data/omffr.sqlite", 0 );
    poSuite->SetQuiet( FALSE );
    sprintf( pszOut, "red%d.db", i );
    sprintf( pszOutLast, "red%d.db", i-1 );
    sprintf( pszShapeOut, "%s.shp", pszOut );
    poSuite->SetOutputPath( "lffix.db" );
    poSuite->SetOgrOutputPath( pszShapeOut );
    //poSuite->SetPreviousResultPath( "/home/kyle/src/TEST.db" );
    //poSuite->SetFpuFilter( "GB" );
    poSuite->SetFpuFilter( "GB_ID_002" );
    //poSuite->SetFpuFilter( "CA_CA_010" );
    //poSuite->SetSpatialFilter("POLYGON((-116 47, -114 47, -114 45, -116 45, " \
    //                                   "-116 47))");
    //poSuite->SetTreatmentProb( 0.7 );
    //if( i == 0 )
    //    poSuite->ReduceResources( 0, 0, 0.0, 0, NULL );
    //else
    //    poSuite->ReduceResources( 0, 0, nReduceCount, RESC_REDUCE_RAND, pszOutLast );
    //poSuite->LoadData( NULL );
    //poSuite->RunAllScenarios( 6, NULL );
   // poSuite->RunScenarios( 2, 1, NULL );
    //poSuite->RunScenario( 1 );
    IRSMultiRunData sData;
    sData.dfTreatPercStart = 0.3;
    sData.dfTreatPercStop = 0.3;
    sData.nTreatPercSteps = 1;
    sData.dfRescRedStart = 500;
    sData.dfRescRedStop = 2000;
    sData.nRescRedSteps = 4;
    sData.nRescTypeReduction = CRW;
    sData.nRegionReduction = REGION_ALL;
    sData.nReductionMethod = RESC_REDUCE_RAND;
    sData.nYearCount = 25;
    sData.dfLargeFirePerc = 1.0;
    sData.nLargeFireMask = ALL_ESCAPES;
    sData.nLargeFireMinSize = 300;
    poSuite->SetMultiRun( &sData, 4, 6, IRSDefaultProgress );
    /*
    poSuite->PostProcessLargeFire( 0, 1.0, 10, NULL );
    poSuite->SummarizeByFpu();
    poSuite->ExportFpuSummary();
    char **papszOpts = NULL;
    papszOpts = CSLAddString( papszOpts, "SPATIALITE=YES" );
    papszOpts = CSLAddString( papszOpts, "OVERWRITE=YES" );
    poSuite->ExportOgrSummary("SQLITE", "testout.db", papszOpts );
    poSuite->ExportFpuSummaryCsv("testout.csv" );
    */
    i++;
    IRSSuite::Destroy( poSuite );
    return 0;
}

int irs_c( int argc, char *argv[] )
{
    IRSSuiteH hSuite;
    //hSuite = IRSCreate( "/home/kyle/src/omffr/trunk/data/omffr.sqlite", 0 );
    return 0;
}

int irs_text( int argc, char *argv[] )
{
    /*
    IRSSuite *poSuite;
    poSuite = new IRSSuite( "/Cprogramming/Dispatcher/Dispatcher/",0);
    poSuite->setQuiet( FALSE );
    poSuite->poScenario->ReadFiles( "/Cprogramming/Dispatcher/Dispatcher/" );
    poSuite->poScenario->OpenLevelsFile();
    char FireFile[512];
    for ( int Scenario = 1; Scenario < 2; Scenario++ );
    {
        sprintf( FireFile,  "/Cprogramming/Dispatcher/Dispatcher/Scenario%d.dat", Scenario );
        poSuite->poScenario->ReadFireFile( FireFile );
        poSuite->poScenario->RunScenario(0, Scenario);
        poSuite->poScenario->LastScenarioDay();
        poSuite->poScenario->SaveTypeTwoIARescWorkYear();
        poSuite->poScenario->SaveMovementVectorsAT();
    }

    poSuite->poScenario->Output();
    */

    return 0;
}

int main( int argc, char *argv[] )
{
    OGRRegisterAll();
    for( int j = 0; j < 1; j++ )
    {
        printf("RUN:%d\n", j);
        irs_cplusplus( argc, argv );
    }
    return 0;
}
