/******************************************************************************
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  IRS main interface
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

#include "irs.h"

static void *hIRSMutex;

/**
 * \brief default progress (console) display
 *
 * \see http://gdal.org/cpl__progress_8h_source.html*
 *
 * \param dfProgress fraction done, 0.0 =< dfProgress <= 1.0
 * \param Optional message, typically NULL
 * \param pProgressArg unused currently.
 * \return TRUE on success
 */

int IRSDefaultProgress( double dfProgress, const char *pszMessage,
                        void *pProgressArg )
{
    printf( "%d%% complete\n", (int)dfProgress * 100 );
    return TRUE;
}

/**
 * \brief Find the index of a string in a string list
 *
 * \param papszList a string list (NULL terminated) to search
 * \param pszKey the value to find
 * \return the integer index of the key in the list.
 */
static int FindStringIndex( const char **papszList, const char * pszKey )
{
    if( papszList == NULL || pszKey == NULL )
        return -1;
    int bFound = FALSE;
    int i;
    while( papszList[i] != NULL )
    {
        if( strcmp( papszList[i], pszKey ) == 0 )
        {
            bFound = TRUE;
            break;
        }
        i++;
    }
    return bFound ? i : -1;
}

/**
 * \brief Get the length of a string list.
 *
 * \param papszList list to count
 * \return number of strings in a list
 */

static int CountStrings( const char **papszList )
{
    int i = 0;
    if( papszList == NULL )
        return i;
    while( papszList[i] != NULL )
        i++;
    return i;
}

/**
 * \brief Create an empty IRS Runner.  Set all appropriate members to NULL.
 * The object is invalid as is.  It's better to use the proper constructor.
 */
IRSSuite::IRSSuite()
{
    Initialize();
}

/**
 * \brief Create a (hopefully) valid IRS Runner.
 *
 * \param pszDataPath A path to a sqlite database with proper tables.
 * \param papszOptions a list of key/value pairs, unused.
 */
IRSSuite::IRSSuite( const char *pszDataPath, char **papszOptions )
{
    Initialize();
    this->pszDataPath = strdup( pszDataPath );
    poDataAccess = IRSDataAccess::Create( 0, pszDataPath );
    poScenario = new CRunScenario();
}

IRSSuiteH IRSSuite::Create( const char *pszDataPath, char **papszOptions )
{
    IRSSuite *pNew;
    try
    {
        pNew = new IRSSuite( pszDataPath, papszOptions );
    }
    catch( ... )
    {
        return NULL;
    }
    return pNew;
}

int IRSSuite::Initialize()
{
    bDataLoaded = FALSE;
    bDataDirty = TRUE;
    pszDataPath = NULL;
    pszExternalResourceDb = NULL;
    poScenario = NULL;
    pszReducePrevPath = NULL;
    pszExternalResourceDb = NULL;
    pszOmitResourceDb = NULL;
    bQuiet = TRUE;

    InitializePrepositionDrawdown();

    pszTreatmentDatasource = NULL;
    pszTreatmentLayer = NULL;
    pszTreatmentWhere = NULL;
    dfTreatment = 0.0;
    dfReduceFraction = 0.0;

    poResult = NULL;
    pszOutputPath = NULL;
    pszOgrOutPath = NULL;
    return IRS_OK;
}

/**
 * \brief Clean up.
 *
 * XXX: Mind the mixed free/deletes here.
 */
IRSSuite::~IRSSuite()
{
    if( pszDataPath )
        free( (void*)pszDataPath );
    if( pszExternalResourceDb )
        free( (void*)pszExternalResourceDb );
    if( pszTreatmentDatasource )
        free( (void*)pszTreatmentDatasource );
    if( pszTreatmentLayer )
        free( (void*)pszTreatmentLayer );
    if( pszTreatmentWhere )
        free( (void*)pszTreatmentWhere );
    if( pszOutputPath )
        free( (void*)pszOutputPath );
    if( pszOgrOutPath )
        free( (void*)pszOgrOutPath );
    if( pszReducePrevPath )
        free( (void*) pszReducePrevPath );
    if( pszOmitResourceDb )
        free( (void*) pszOmitResourceDb );
    if( pszExternalResourceDb )
        free( (void*) pszExternalResourceDb );

    if( poScenario )
        delete( poScenario );
    if( poDataAccess )
        delete( poDataAccess );
    if( poResult )
        delete poResult;
}

void IRSSuite::Destroy( IRSSuiteH hSuite )
{
    delete (IRSSuite*)hSuite;
}

/**
 * \brief Set verbosity
 *
 * \param bQuiet whether to shutup or not
 * \return 0
 */
int IRSSuite::SetQuiet( int bQuiet )
{
    this->bQuiet = bQuiet;
    return IRS_OK;
}

/**
 * \brief Set an arbitrary spatial filter for the simulation.
 *
 * Use some geometry represented as well-known text to filter the simulation.
 * This is for arbitrary geometries, bounding boxes, etc.  If you want to do a
 * simulation on an administrative boundary, use SetFpuFilter().
 *
 * \see SetFpuFilter
 *
 * \param pszWktGeometry a valid geometry in well-known text.
 * \return 0 on success
 */
int IRSSuite::SetSpatialFilter( const char *pszWktGeometry )
{
    if( bDataLoaded )
        bDataDirty = TRUE;
     return poDataAccess->SetSpatialFilter( pszWktGeometry );
}

/**
 * \brief Limit a simulation to an FPU or by proxy, a region.
 *
 * This allows limiting a simulation to a FPU boundary.  It can only run one
 * FPU.  Pass the FPA code for the FPU to limit the simulation, ie "GB_ID_002"
 * for Great Basin, Idaho, FPU 2.  To run a simulation on a GACC/Region, pass
 * the two character representation of the region, ie "GB" for Great Basin.
 *
 * \param pszFpu String representation of a FPU.
 * \return 0 on success, non-zero otherwise.
 */
int IRSSuite::SetFpuFilter( const char *pszFpu )
{
    if( bDataLoaded )
        bDataDirty = TRUE;
    return poDataAccess->AddFpuFilter( pszFpu );
}

/**
 * \brief Set an external database to read resources from.
 *
 * The table must follow a specified schema.  Use CreateExternalResourceDb() to
 * make one if you are unsure of the schema.  Foreign key restraints apply to
 * the main database used for the simulation, ie dispatch locations.
 *
 * \param pszExternalResourceDb Path to a sqlite db with a resource table.
 *
 * \return 0 on success, non-zero otherwise.
 */
int IRSSuite::SetExternalResourceDb( const char *pszExternalResourceDb )
{
    if( !pszExternalResourceDb )
        return IRS_OK;
    if( bDataLoaded )
        bDataDirty = TRUE;
    this->pszExternalResourceDb = strdup( pszExternalResourceDb );
    assert( poResult );
    poResult->SetMetadataItem( "external_resource_db", pszExternalResourceDb );
    return 0;
}

/*
** Keeep track of which resources to remove.
*/

void IRSSuite::LoadDataConcurrent( void *pData )
{
#ifdef IRS_SERIAL_DATA_ACCESS
    CPLCreateOrAcquireMutex( &hIRSMutex, 1000.0 );
#endif
    IRSRunData *pRd = (IRSRunData*)pData;
    CRunScenario *pS = pRd->poScen;
    IRSDataAccess *pD = pRd->poDA;
    if( pRd->pfnProgress )
        pRd->pfnProgress( 0.05, "Loading Resource Types...", NULL );
    pD->LoadRescType( pS->m_VRescType, NULL );
    if( pRd->pfnProgress )
        pRd->pfnProgress( 0.1, "Loading Production Rates...", NULL );
    pD->LoadProdRates( pS->m_VProdRates, NULL );
    if( pRd->pfnProgress )
        pRd->pfnProgress( 0.3, "Loading Dispatch Logic...", NULL );
    pD->LoadDispatchLogic( pS->m_VDispLogic, NULL, NULL );
    if( pRd->pfnProgress )
        pRd->pfnProgress( 0.5, "Loading FWAs...", NULL );
    pD->LoadFwa( pS->m_VDispLogic, pS->m_VFWA, NULL, NULL );
    if( pRd->pfnProgress )
        pRd->pfnProgress( 0.7, "Loading Dispatch Locations...", NULL );
    pD->LoadDispatchLocation( pS->m_VFWA, pS->m_VDispLoc, NULL );
    if( pRd->pfnProgress )
        pRd->pfnProgress( 0.8, "Loading Tanker bases...", NULL );
    pD->LoadTankerBases( pS, pS->m_VFWA, pS->m_VDispLoc, -1, NULL );
    if( pRd->pfnProgress )
        pRd->pfnProgress( 0.9, "Loading Resources...", NULL );
    pD->LoadResource( pS, pS->m_VRescType, pS->m_VDispLoc, pS->m_VResource,
                      NULL, NULL, NULL, pRd->pszOmitSet, NULL );
                      //pszExternalResourceDb, NULL, NULL );
    pD->CreateLargeAirtankers( pS->m_VRescType, pS->m_VDispLoc,
                               pS->m_VResource, pRd->nTankerCount, NULL );

#ifdef IRS_SERIAL_DATA_ACCESS
    CPLReleaseMutex( hIRSMutex );
#endif
    pS->m_NumFWA = pS->m_VFWA.size();
    pS->m_NumRescType = pS->m_VRescType.size();
    pS->m_NumDispLoc = pS->m_VDispLoc.size();
    pS->m_NumProdRates = pS->m_VProdRates.size();
    pS->m_NumResource = pS->m_VResource.size();

    /* TODO: Check signature vs struct array length */
    //pS->SetPreposition( pasPP[0], pasPP[1], pasPP[2], pasPP[3], pasPP[4],
    //                    pasPP[5], pasPP[6], pasDD[0], pasDD[1], pasDD[2],
    //                    pasDD[3], pasDD[4], pasDD[5], pasDD[6], pasDD[7],
    //                    pasPP[7], pasPP[8] );
    pS->CreateDispTree();
    /* Put resources in tree */
    pS->ResourcesToDispatchers();
    pS->FWAsFindClosestAirtankerBases();

    /*
    ** FIXME: doesn't work on windowed simulation?
    */
    //poDataAccess->LoadAlternateHelibases( poScenario->m_DispMap,
    //                                      poScenario->m_DispTree );
    /*
    ** TODO: Implement in DataAccess
    */
    //poScenario->OpenLevelsFile();
    //poScenario->LoadExpectedLevels();
    if( pRd->pfnProgress )
        pRd->pfnProgress( 1.0, "Data loaded.", NULL );

    return;
}

int *panIndexes;
int nRemoveCount;
/**
 * \brief run a Scenario on a thread.
 *
 */
void IRSSuite::RunScenarioConcurrent( void *pData )
{
    CRunScenario *pS = ((IRSRunData*)pData)->poScen;
    IRSDataAccess *pD = ((IRSRunData*)pData)->poDA;
    IRSProgress pfnProgress = ((IRSRunData*)pData)->pfnProgress;
    std::vector<int> anYears = pD->GetYearIndexes();
    int nY = anYears[((IRSRunData*)(pData))->nYearIndex];
    double dfT = ((IRSRunData*)pData)->dfTreat;
    pS->m_VFire.clear();

#ifdef IRS_SERIAL_DATA_ACCESS
    CPLCreateOrAcquireMutex( &hIRSMutex, 1000.0 );
#endif
    pD->LoadScenario( pS->m_VFWA, pS->m_VFire, nY, dfT, NULL );
#ifdef IRS_SERIAL_DATA_ACCESS
    CPLReleaseMutex( hIRSMutex );
#endif
    pS->m_NumFire = pS->m_VFire.size();
    pS->Reset();
    pS->m_VResults.clear();

    /*
    for( int i = 0; i < nRemoveCount; i++ )
    {
        pS->m_VResource[panIndexes[i]]->SetAvailableTime( 525601 * 10 );
    }
    */

    ((IRSRunData*)pData)->nErr = pS->RunScenario( 0, nY, pfnProgress );

    return;
}
/**
 * This should be used to run all simulations.  The other functions are simply
 * syntactic sugar.
 *
 */
int IRSSuite::RunScenarios( unsigned int nYearStart, unsigned nYearCount, 
                            unsigned int nThreads, IRSProgress pfnProgress )
{
    std::vector<int>anYears = poDataAccess->GetYearIndexes();
    int nNumYears = anYears.size();
    int nNumJobs = nYearCount;
    assert( nYearStart < nNumYears );
    assert( nYearCount <= nNumYears );
    assert( nYearStart + nYearCount <= nNumYears );

    if(pfnProgress)
        pfnProgress( 0.0, "Starting simulation...", NULL );

#ifdef IRS_ENABLE_THREADING
    nThreads = nThreads > CPLGetNumCPUs() ? CPLGetNumCPUs() : nThreads ;
    CRunScenario **papoScenarios = new CRunScenario*[nThreads];
    IRSDataAccess **papoDataAccess = new IRSDataAccess*[nThreads];
    IRSRunData *pasRunData = (IRSRunData*)CPLMalloc( sizeof( IRSRunData ) * nThreads );
    unsigned int i;
    unsigned int j;
    IRSThread *pasThreads = (IRSThread*)CPLMalloc( sizeof( IRSThread ) * nThreads );
    for( i = 0; i < nThreads; i++ )
    {
        pasRunData[i].pfnProgress = NULL;
        pasRunData[i].pszOmitSet = NULL;
        papoScenarios[i] = new CRunScenario();
        papoDataAccess[i] = IRSDataAccess::Create( 0, pszDataPath );
        pasRunData[i].poScen = papoScenarios[i];
        pasRunData[i].poDA = papoDataAccess[i];
        pasRunData[i].dfTreat = dfTreatment;
        pasRunData[i].nTankerCount = GetTankerCount();
        if( poDataAccess->GetSpatialFilter() )
        {
            pasRunData[i].poDA->SetSpatialFilter( poDataAccess->GetSpatialFilter() ) ;
        }
        else if( poDataAccess->GetFpuFilter() )
        {
            pasRunData[i].poDA->AddFpuFilter( poDataAccess->GetFpuFilter() ) ;
        }
        pasRunData[i].poScen->SetPreposition( pasPP[0], pasPP[1], pasPP[2], pasPP[3],
                                              pasPP[4], pasPP[5], pasPP[6],
                                              pasDD[0], pasDD[1], pasDD[2], pasDD[3],
                                              pasDD[4], pasDD[5], pasDD[6], pasDD[7],
                                              pasPP[7], pasPP[8] );
    }
    if(pfnProgress)
        pfnProgress( 0.0, "Creating Threads...", NULL );

    /*
    ** XXX: Attempt to handle resource removal.  This is experimental, because
    ** I have to present to OMB in one week.  I might as well wing it now.
    ** OMB? OMG! MARS.
    */
#define OMG_ITS_OMB_EXPERIMENTAL
#ifdef OMG_ITS_OMB_EXPERIMENTAL
    char * pszOmitMe = NULL;
    char *pszOmitAll = NULL;
    int nHowMany;
    int RemoveCount = 0;
    char **papszIds;
    int nPrevRemoved = 0;
    char szMd[IRS_MAX_METADATA_SIZE];
    if( dfReduceFraction > 0 )
    {
        if( pszReducePrevPath != NULL )
        {
            IRSResult oResult( pszReducePrevPath, NULL, 1, TRUE, TRUE, NULL );
            nRemoveCount = oResult.GetRemovedResourceCount();
            if( nRemoveCount > 0 )
            {
                papszIds = oResult.GetRemovedResourceIndexesAsStrings();
                pszOmitMe = (char*)JoinStringList( papszIds, "," );
                poResult->SetRemovedResourcesStrings( papszIds );
                poResult->SetMetadataItem( IRS_RESC_RED_CNT_KEY, 
                                           oResult.GetMetadataItem( IRS_RESC_RED_CNT_KEY ) );
                nPrevRemoved = atoi( oResult.GetMetadataItem( IRS_RESC_RED_CNT_KEY ) );
                oResult.DestroyUnderPerformersList( papszIds );
            }
        }
        /*
        ** If we didn't remove any before, this is the first time with a
        ** reductions.
        */
        if( nPrevRemoved > 1 )
        {
            nHowMany = nPrevRemoved;
        }
        else
        {
            nHowMany = poDataAccess->GetRescCount( nReduceAgencies, nReduceRegions,
                                                   nReduceTypes, pszOmitMe );
            nHowMany = nHowMany * dfReduceFraction;
            sprintf( szMd, "%d", nHowMany );
            poResult->SetMetadataItem( IRS_RESC_RED_CNT_KEY, szMd );
        }
        int nOmitMeTooCount;
        char **papszOmitMeToo;
        if( nReduceMethod == RESC_REDUCE_PERFORM && pszReducePrevPath != NULL)
        {
            IRSResult oResult( pszReducePrevPath, NULL, 1, TRUE, TRUE, NULL );
            papszOmitMeToo = oResult.GetUnderPerformers( nHowMany, FALSE );
        }
        else
        {
            papszOmitMeToo = poDataAccess->GetRescRemovalSet( nReduceAgencies,
                                                              nReduceRegions,
                                                              nReduceTypes,
                                                              nHowMany,
                                                              &nOmitMeTooCount,
                                                              pszOmitMe );
        }
        if( papszOmitMeToo )
        {
            pszOmitAll = (char*)JoinStringList( papszOmitMeToo, "," );
        }
        else
        {
            pszOmitAll = strdup( "" );
        }
        /*
        ** XXX: both plus a ','
        */
        if( pszOmitMe )
        {
            pszOmitAll = (char*)OmffrRealloc( pszOmitAll, strlen( pszOmitAll ) +
                                              strlen( pszOmitMe ) + 2 );
            sprintf( pszOmitAll, "%s,%s", pszOmitAll, pszOmitMe );
        }
        /* XXX: Omit the aholes */
        for( i = 0; i < nThreads; i++ )
        {
            pasRunData[i].pszOmitSet = pszOmitAll;
        }
        if( papszOmitMeToo )
        {
            poResult->SetRemovedResourcesStrings( papszOmitMeToo );
            poDataAccess->DestroyStringList( papszOmitMeToo );
        }
    }
#endif /* OMG_ITS_OMB_EXPERIMENTAL */

    /* Use one thread as a proxy for progress */
    pasRunData[0].pfnProgress = pfnProgress;
    for( i = 0; i < nThreads; i++ )
    {
#ifdef IRS_LOAD_DATA_CONCURRENT
        pasThreads[i].hThread =
            CPLCreateJoinableThread( IRSSuite::LoadDataConcurrent, (void*)&pasRunData[i] );
#else
        LoadDataConcurrent( (void*)&pasRunData[i] );
#endif
    }
#ifdef IRS_LOAD_DATA_CONCURRENT
    for( i = 0; i < nThreads; i++ )
    {
        CPLJoinThread( pasThreads[i].hThread );
    }
#endif

#ifndef OMG_ITS_OMB_EXPERIMENTAL
    if( pszReducePrevPath )
    {
        IRSResult oResult( pszReducePrevPath, NULL, 1, TRUE, TRUE, NULL );
        nRemoveCount = oResult.GetRemovedResourceCount();
        if( nRemoveCount > 0 )
        {
            panIndexes = (int*)calloc( nRemoveCount, sizeof( int ) );
            oResult.GetRemovedResourceIndexes( panIndexes );
        }
    }
    int nToAdd = 0;
    int nResourceCount = pasRunData[0].poScen->m_VResource.size();
    if( dfReduceFraction < 1.0 )
    {
        nToAdd = nResourceCount * dfReduceFraction;
    }
    else
    {
        nToAdd = dfReduceFraction;
    }
    if( nToAdd > 0 )
    {
        panIndexes = (int*)realloc( (void*)panIndexes,
                                    sizeof( int ) * ( nRemoveCount + nToAdd ) );
        nToAdd = SelectRandomIndexes( panIndexes + nRemoveCount,
                                      nResourceCount, nToAdd );
        nRemoveCount = nRemoveCount + nToAdd;
    }
    else
    {
        panIndexes = NULL;
        nToAdd = 0;
    }
#ifdef IRS_SERIAL_DATA_ACCESS
    CPLCreateOrAcquireMutex( &hIRSMutex, 1000.0 );
#endif
    poResult->SetRemovedResources( poScenario->m_VResource, panIndexes,
                                   nRemoveCount );
#endif /* OMG_ITS_OMB_EXPERIMENTAL */
    if( pfnProgress )
        pfnProgress( 0.0, "Running Scenarios...", NULL );
#ifdef IRS_SERIAL_DATA_ACCESS
    CPLReleaseMutex( hIRSMutex );
#endif

    int nJobsPerThread = nNumJobs / nThreads;
    int nLeftOverJobs = nNumJobs % nThreads;
    int nRunThreads = nThreads;
    int nYearToRun = 0;
    if( nLeftOverJobs )
    {
        nJobsPerThread++;
    }
    int nJob = 0;
    if( pfnProgress )
    {
        pfnProgress( 0.0, "Starting simulation...", NULL );
    }
    for( i = 0; i < nJobsPerThread; i++ )
    {
        if( i == nJobsPerThread - 1 && nLeftOverJobs )
        {
            nRunThreads = nLeftOverJobs;
        }
        else
        {
            nRunThreads = nThreads;
        }
        for( j = 0; j < nRunThreads; j++ )
        {
            nYearToRun = nYearStart + nJob;
            pasRunData[j].nYearIndex = nYearToRun;
            printf( "Thread PID %u running %d index\n", j, nYearToRun );
#ifdef IRS_RUN_SCEN_CONCURRENT
            pasThreads[j].hThread =
                CPLCreateJoinableThread( IRSSuite::RunScenarioConcurrent, &pasRunData[j] );
#else
            RunScenarioConcurrent( &pasRunData[j] );
#endif
            nJob++;
            if( pfnProgress )
            {
                pfnProgress( (double)nJob / (double)nNumJobs,
                             "Running simulation...", NULL );
            }

        }
#ifdef IRS_RUN_SCEN_CONCURRENT
            for( unsigned int join = 0;join < nRunThreads; join++ )
            {
                CPLJoinThread( pasThreads[join].hThread );
            }
#endif
        for( unsigned int write = 0; write < nRunThreads; write++ )
        {
#ifdef IRS_SERIAL_DATA_ACCESS
            CPLCreateOrAcquireMutex( &hIRSMutex, 1000.0 );
#endif
            poResult->ExportFullFireResults( anYears[pasRunData[write].nYearIndex],
                                             pasRunData[write].poScen->m_VResults,
                                             pasRunData[write].poScen->GetResourceUsage() );
            poResult->AddSingleResourceUsage( pasRunData[write].poScen->GetSingleResourceUsage() );
#ifdef IRS_SERIAL_DATA_ACCESS
            CPLReleaseMutex( hIRSMutex );
#endif
            pasRunData[write].poScen->Reset();
            pasRunData[write].poScen->m_VResults.clear();
            pasRunData[write].poScen->m_VEscapes.clear();
            pasRunData[write].poScen->m_EscapeIdx = 0;
        }
    }
    //ExportOgr( "ESRI Shapefile", pszOgrOutPath );

#else /* IRS_ENABLE_THREADING */
    for( unsigned int i = 0; i < nYearCount; i++ )
    {
        poScenario->m_VFire.clear();
        poDataAccess->LoadScenario( poScenario->m_VFWA, poScenario->m_VFire,
                                    anYears[nYearStart + i], dfTreatment, NULL );
        poScenario->m_NumFire = poScenario->m_VFire.size();
        poScenario->RunScenario( 0, i, pfnProgress );
        poScenario->Output();
        poResult->ExportFullFireResults( anYears[i], poScenario->m_VResults,
                                         poScenario->GetResourceUsage());
        poResult->AddSingleResourceUsage( poScenario->GetSingleResourceUsage() );
        poScenario->Reset();
        poScenario->m_VResults.clear();
        poScenario->m_VEscapes.clear();
        poScenario->m_EscapeIdx = 0;
        if( pfnProgress )
        {
            pfnProgress( (double)i / (double)nYearCount,
                         "Running simulation...", NULL );
        }
    }

    //ExportOgr( "ESRI Shapefile", pszOgrOutPath );
#endif /* IRS_ENABLE_THREADING */

    if( pfnProgress )
    {
        pfnProgress( 1.0, "Finished simulation.", NULL );
    }
    return IRS_OK;
}


/**
 * \brief Run a single scenario (year).
 *
 * Run one year of the simulation.  The year index provided is not necessarily
 * the scenario number.  It is the nth year in the year array.  In future
 * versions of the fig, nYearIndex will likely by scenario number - 1.
 *
 * This gives external applications a finer-grain control over how to run the
 * simulations.  A good use would be thread handling.
 *
 * \param nYearIndex array index for scenario to run
 * \return zero on success
 */

int IRSSuite::RunScenario( unsigned int nYearIndex )
{
    return RunScenarios( nYearIndex, 1, 1, NULL );
}

/**
 * \brief Run n years of simulations.
 *
 * \param nYearCount number of years to run
 * \param nThreads number of threads to use
 * \param pfnProgress progress function.
 * \return zero on success
 */
int IRSSuite::RunScenarios( unsigned int nYearCount, unsigned int nThreads,
                            IRSProgress pfnProgress )
{
    return RunScenarios( 0, nYearCount, nThreads, pfnProgress );
}

/**
 * \brief Run all available scenarios.
 *
 * \param nThreads number of threads to use
 * \param pfnProgress progress function
 * \return zero on success
 */
int IRSSuite::RunAllScenarios( unsigned int nThreads, IRSProgress pfnProgress )
{
    return RunScenarios( 0, GetScenarioCount(), nThreads, pfnProgress );
}

/**
 * \brief Extract data from SQLite database and load it into memory for
 *        simulations.
 *
 * \param pfnProgress progress function
 * \return zero on success
 */
int IRSSuite::LoadData( IRSProgress pfnProgress )
{
    if( pszDataPath == NULL )
    {
        return IRS_INVALID_INPUT;
    }
    if( bDataLoaded && !bDataDirty )
    {
        return IRS_OK;
    }
    if( pfnProgress )
        pfnProgress( 0.05, "Loading Resource Types...", NULL );
    poDataAccess->LoadRescType( poScenario->m_VRescType, NULL );
    if( pfnProgress )
        pfnProgress( 0.1, "Loading Production Rates...", NULL );
    poDataAccess->LoadProdRates( poScenario->m_VProdRates, NULL );
    if( pfnProgress )
        pfnProgress( 0.3, "Loading Dispatch Logic...", NULL );
    poDataAccess->LoadDispatchLogic( poScenario->m_VDispLogic, NULL, NULL );
    if( pfnProgress )
        pfnProgress( 0.5, "Loading FWAs...", NULL );
    poDataAccess->LoadFwa( poScenario->m_VDispLogic, poScenario->m_VFWA,
                           pfnProgress, NULL );
    if( pfnProgress )
        pfnProgress( 0.7, "Loading Dispatch Locations...", NULL );
    poDataAccess->LoadDispatchLocation( poScenario->m_VFWA,
                                        poScenario->m_VDispLoc, NULL );
    if( pfnProgress )
        pfnProgress( 0.8, "Loading Tanker bases...", NULL );
    poDataAccess->LoadTankerBases( poScenario, poScenario->m_VFWA,
                                   poScenario->m_VDispLoc, -1, NULL );
    if( pfnProgress )
        pfnProgress( 0.9, "Loading Resources...", NULL );
    const char *pszToRem = NULL;
    poDataAccess->LoadResource( poScenario,
                                poScenario->m_VRescType,
                                poScenario->m_VDispLoc,
                                poScenario->m_VResource,
                                pszExternalResourceDb, NULL, pszReducePrevPath,
                                NULL, NULL );
    poDataAccess->CreateLargeAirtankers( poScenario->m_VRescType,
                                         poScenario->m_VDispLoc,
                                         poScenario->m_VResource,
                                         nTankerCount, NULL );
    if( dfReduceFraction != 0.0 )
    {
        IReduceResources( nReduceAgencies, nReduceTypes, dfReduceFraction,
                          nReduceMethod, pszReducePrevPath );
    }
    poScenario->m_NumDispLogic = poScenario->m_VDispLogic.size();
    poScenario->m_NumFWA = poScenario->m_VFWA.size();
    poScenario->m_NumRescType = poScenario->m_VRescType.size();
    poScenario->m_NumDispLoc = poScenario->m_VDispLoc.size();
    poScenario->m_NumProdRates = poScenario->m_VProdRates.size();
    poScenario->m_NumResource = poScenario->m_VResource.size();

    /* TODO: Check signature vs struct array length */
    poScenario->SetPreposition( pasPP[0], pasPP[1], pasPP[2], pasPP[3],
                                pasPP[4], pasPP[5], pasPP[6],
                                pasDD[0], pasDD[1], pasDD[2], pasDD[3],
                                pasDD[4], pasDD[5], pasDD[6], pasDD[7],
                                pasPP[7], pasPP[8] );
    poScenario->CreateDispTree();
    /* Put resources in tree */
    poScenario->ResourcesToDispatchers();
    poScenario->FWAsFindClosestAirtankerBases();
    /*
    ** TODO add to DataAccess.
    */

    /*
    ** FIXME: doesn't work on windowed simulation?
    */
    //poDataAccess->LoadAlternateHelibases( poScenario->m_DispMap,
    //                                      poScenario->m_DispTree );
    /*
    ** TODO: Implement in DataAccess
    */
    //poScenario->OpenLevelsFile();
    //poScenario->LoadExpectedLevels();
    if( pfnProgress )
        pfnProgress( 1.0, "Data loaded.", NULL );
    bDataLoaded = TRUE;
    bDataDirty = FALSE;
    return IRS_OK;
}

/**
 * \brief Free up resources for data access.
 *
 * \return zero on success.
 */
int IRSSuite::UnloadData()
{
    delete poScenario;
    poScenario = NULL;
    delete poDataAccess;
    poDataAccess = NULL;
    bDataLoaded = FALSE;
    return IRS_OK;
}

/**
 * \brief get the number of years in a scenario table.
 *
 * \return the number of distinct years in the db for fires.
 */

unsigned int IRSSuite::GetScenarioCount()
{
    return poDataAccess->GetYearIndexes().size();
}

/**
 * \brief Set the likelihood holding a resource associated with large fires.
 *        It may be a subset of the entire resource pool by agency and/or type.
 *        It may be different depending whether or not the resource's local
 *        unit is in or out of season.
 *
 * \param nAgencies agency code(s) (see above).
 * \param nRescTypes resource code(s) (see above).
 * \param bOutOfSeason if TRUE, only move resources if they are not current
 *                     working in season locally (XXX:check on docs).
 * \param dfDrawDown value between 0.5(never) and 1.0(always) draw down
 * \return zero on success
 */


/**
 * \brief Initialize all preposition and drawdown structs.
 *
 * \return zero on success, non-zero otherwise
 */
int IRSSuite::InitializePrepositionDrawdown()
{
    assert( CountStrings( papszPrePosKeys ) == PREPOS_COUNT );
    int i;
    for( i = 0; i < PREPOS_COUNT; i++ )
    {
        pasPP[i].rescType = papszPrePosKeys[i];
        pasPP[i].level = 1.0;
        pasPP[i].outOfSeason = false;
    }
    assert( CountStrings( papszDrawDownKeys ) == DRAWDOWN_COUNT );
    for( i = 0; i < DRAWDOWN_COUNT; i++ )
    {
        pasDD[i].rescType = papszDrawDownKeys[i];
        pasDD[i].level = 1.0;
        pasDD[i].outOfSeason = false;
    }
    return IRS_OK;
}

int IRSSuite::SetPreposition( const char *pszKey, double dfLevel,
                              int bOutOfSeason )
{
    if( !pszKey || dfLevel > 1.0 || dfLevel < 0.0 )
        return IRS_INVALID_INPUT;
    int i;
    for( i = 0; i < PREPOS_COUNT; i++ )
    {
        if( EQUAL( pszKey, pasPP[i].rescType.c_str() ) )
        {
            pasPP[i].level = dfLevel;
            pasPP[i].outOfSeason = bOutOfSeason;
            return IRS_OK;
        }
    }
    return IRS_INVALID_INPUT;
}
int IRSSuite::SetDrawdown( const char *pszKey, double dfLevel,
                              int bOutOfSeason )
{
    if( !pszKey || dfLevel > 1.0 || dfLevel < 0.0 )
        return IRS_INVALID_INPUT;
    int i;
    for( i = 0; i < DRAWDOWN_COUNT; i++ )
    {
        if( EQUAL( pszKey, pasDD[i].rescType.c_str() ) )
        {
            pasDD[i].level = dfLevel;
            pasDD[i].outOfSeason = bOutOfSeason;
            return IRS_OK;
        }
    }
    return IRS_INVALID_INPUT;
}

/*
** qsort integer comparator
*/
static int IntCompare( const void *nOne, const void *nTwo )
{
    int n = *( (int*)nOne );
    int m = *( (int*)nTwo );
    if( n > m )
        return  1;
    if( n < m )
        return -1;
    return 0;
}

/**
 * \brief Remove resource from the simulation by essentially makeing them
 * unavailable until the end of the year. 
 *
 */

int IRSSuite::RemoveResourceByIndexes( int *panIndexes, int nToRemove )
{
#define UNDEFINED
#ifdef UNDEFINED
    int nIndex, nCount;
    for( int i = 0; i < nToRemove; i++ )
    {
        nCount = poScenario->m_VResource.size();
        nIndex = panIndexes[i] - i;
        assert( nIndex >= 0 && nIndex < nCount );
        poScenario->m_VResource.erase( poScenario->m_VResource.begin() + nIndex );
    }
#else
    for( int i = 0; i < nToRemove; i++ )
    {
        poScenario->m_VResource[panIndexes[i]]->SetAvailableTime( 525601 * 10 );
    }
#endif
    return IRS_OK;
}

/**
 * \brief Reduce a given subset of resources arbitrarily.
 *
 * This forces certain resource to *not* be loaded into the simualtion.
 * Various scenarios could be simulated using this API call:
 *
 * TODO: add good examples
 * - Reduce Airtankers by setting the ATT bit in resource type
 * - Remove all DOI by just setting the agency bit for DOI_ALL
 * - etc.
 *
 * \warning Only random removal (no constraints) is available now.
 *
 * \param nAgencies agency code(s) (see above).
 * \param nRescTypes resource code(s) (see above).
 * \param nMethod possible future use for removing resources also based on 
 *                performance metrics from previous runs.  Currently unused.
 *                FIXME.
 * param dfReduction fraction of resources to reduce if 0.0 <= dfReduction <=
 *                   1.0, or number to reduce by if dfReduction > 1.0
 * \param pszPreviousPath path to a previous run's output.  If not NULL, open
 *                        the db, and *remove the resources that were already
 *                        removed from the previous run.*  This allows
 *                        'chaining' of runs for comparions as resources are
 *                        reduced with the concept: once removed, always
 *                        removed.
 * \return zero on success
 */

int IRSSuite::ReduceResources( int nAgencies, int nRescTypes, double dfReduction,
                               int nMethod, const char *pszPreviousPath )
{
    nReduceAgencies = nAgencies;
    nReduceTypes = nRescTypes;
    dfReduceFraction = dfReduction;
    nReduceMethod = nMethod;
    if( pszPreviousPath )
        pszReducePrevPath = strdup( pszPreviousPath );
    bDataDirty = TRUE;
    return 0;
}

/**
 * \brief Randomly select a number of indexes from an array
 *
 * The function may leave early, leaving one unfilled space.  This is just
 * random slop.  The return value will always be <= nCount.
 *
 * \param [out] panIndexes array to fill.
 * \param nCount size of the full array
 * \param nToRemove number of indexes to select
 * \return number of indexes actually removed
 */

static int SelectRandomIndexes( int *panIndexes, int nCount, int nToRemove )
{
    Random r;
    int i;
    int j;
    double p;
    double dfReduction = (double)nToRemove / (double)nCount;
    for( i = 0, j = 0; i < nCount && j < nToRemove; i++ )
    {
        p = r.rand3();
        if( p < dfReduction )
            panIndexes[j++] = i;
        dfReduction = (double)( nToRemove - j ) / (double)(nCount - i);
    }
    /* Did we get close ? */
    assert( abs( nToRemove - j ) < nToRemove * 0.1 );
    /* Make sure we don't overrun... */
    if( nToRemove > j )
        nToRemove = j;
    return nToRemove;
}

/**
 * \brief Remove resources from a previous run.
 *
 * \param pszPreviousPath the path to the data
 * \return zero on success, non-zero otherwise
 */
int IRSSuite::RemovePreviousResources( const char *pszPreviousPath )
{
    if( !pszPreviousPath )
        return IRS_OK;
    int *panIndexes;
    if( pszPreviousPath )
    {
        IRSResult oResult( pszPreviousPath, NULL, 1, TRUE, TRUE, NULL );
        int nSize = oResult.GetRemovedResourceCount();
        if( nSize > 0 )
        {
            panIndexes = (int*)calloc( nSize, sizeof( int ) );
            if( !oResult.GetRemovedResourceIndexes( panIndexes ) )
            {
                poResult->SetRemovedResources( poScenario->m_VResource,
                                               panIndexes, nSize );
            }
            free( (void*)panIndexes );
        }
    }
    return IRS_OK;
}

/**
 * \brief Remove poorly performing resources
 *
 * \param pszPreviousPath path to last run's output
 * \param dfPerfReduction fraction (<1.0) *or* integer (>1.0) to reduce
 *        workforce.
 * \return zero on success, non-zero otherwise
 */
int IRSSuite::RemovePreviousResourcesPerform( const char *pszPreviousPath,
                                              double dfPerfReduction, 
                                              int bRemoveUnused )
{
    if( !pszPreviousPath )
    {
        return IRS_INVALID_INPUT;
    }
    if( dfPerfReduction < 0.0 )
    {
        return IRS_INVALID_INPUT;
    }
    IRSResult oResult( pszPreviousPath, NULL, 1, TRUE, TRUE, NULL );
    char **papszResources;
    papszResources = oResult.GetUnderPerformers( dfPerfReduction,
                                                 bRemoveUnused );
    int nCount = CountStrings( (const char **)papszResources );
    if( nCount < 1 )
    {
        return IRS_OK;
    }
    /*
    ** Yuck.  Inefficient traversal.
    */
    int *panIndexes = (int*)malloc( sizeof( int ) * nCount );
    int i, j, k;
    k = 0;
    for( i = 0; i < poScenario->m_VResource.size(); i++ )
    {
        for( j = 0; j < nCount; j++ )
        {
            if( EQUAL( poScenario->m_VResource[i]->GetRescID().c_str(),
                       papszResources[j] ) )
            {
                panIndexes[k++] = i;
                if( k == nCount )
                {
                    k--;
                    goto res_perf_done;
                }
            }
        }
    }
res_perf_done:
    if( k + 1 < nCount )
        nCount = k + 1;
    assert( poResult );
    poResult->SetRemovedResources( poScenario->m_VResource, panIndexes,
                                   nCount );
    RemoveResourceByIndexes( panIndexes, nCount );
    oResult.DestroyUnderPerformersList( papszResources );
    free( (void*)panIndexes );
    return IRS_OK;
}

/**
 * \brief Reduce resources by query.
 *
 */
const char * IRSSuite::IReduceResources2( int nAgencies, int nRescTypes,
                                          double dfReduction, int nMethod,
                                          const char *pszPreviousPath )
{
    int i;
    int nRemCount, nRemoved;
    char * pszRemoved = NULL;
    char **papszIgnoreId;
    IRSResult oResult( pszPreviousPath, NULL, 1, TRUE, TRUE, NULL );
    int nSize = oResult.GetRemovedResourceCount();
    if( nSize > 0 )
    {
        panIndexes = (int*)calloc( nSize, sizeof( int ) );
        pszRemoved = (char*)calloc( sizeof( char ) * nSize, 10 );
        if( !oResult.GetRemovedResourceIndexes( panIndexes ) )
        {
            for( i = 0; i < nSize - 1; i++ )
            {
                sprintf( pszRemoved, "%s%d,", pszRemoved, panIndexes[i] );
            }
            sprintf( pszRemoved, "%s%d", pszRemoved, panIndexes[i] );
            poResult->SetRemovedResources( poScenario->m_VResource,
                                           panIndexes, nSize );
        }
        free( (void*) panIndexes );
    }
    int nTotalCount = poDataAccess->GetRescCount( nAgencies, 0, nRescTypes,
                                                  pszRemoved );

    papszIgnoreId = poDataAccess->GetRescRemovalSet( nAgencies, 0, nRescTypes,
                                                     nRemCount, &nRemoved,
                                                     pszRemoved );
    panIndexes = (int*)calloc( sizeof( int ), nRemoved );
    for( i = 0; i < nRemoved; i++ )
    {
        panIndexes[i] = atoi( papszIgnoreId[i] );
        assert( panIndexes[i] > 0 );
    }
    poResult->SetRemovedResourcesIds( panIndexes, nRemoved );
    return IRS_OK;
}

/**
 * \brief Really reduce resources
 *
 * \see ReduceResources
 */
int IRSSuite::IReduceResources( int nAgencies, int nRescTypes, double dfReduction,
                                int nMethod, const char *pszPreviousPath )
{
    /*
    ** We currently don't support agency or type subsets.  Just a random
    ** reduction of resources across the loaded resource set.
    */
    if( nAgencies || nRescTypes )
    {
        return IRS_UNSUPPORTED;
    }
    assert( dfReduction >= 0.0 );
    int nCount = poScenario->m_VResource.size();
    if( nCount == 0 )
    {
        return IRS_INVALID_INPUT;
    }

    /*
    ** Handle the previous path argument, and quickly remove resources that
    ** were already removed.
    */
    if( pszPreviousPath )
    {
        poResult->SetRemovedResources( pszPreviousPath );
    }

    /*
    ** We can be done here if the number to remove is effectively 0
    */
    if( dfReduction == 0.0 )
    {
        return IRS_OK;
    }

    if( nMethod == RESC_REDUCE_PERFORM && pszPreviousPath )
    {
        return RemovePreviousResourcesPerform( pszPreviousPath, dfReduction, FALSE );
    }

    nCount = poScenario->m_VResource.size();
    int nToRemove;
    if( dfReduction > 1.0 )
    {
        nToRemove = (int)dfReduction;
        dfReduction = (double)nToRemove / (double)nCount;
    }
    else
    {
        nToRemove = dfReduction * nCount;
    }
    if( nToRemove > nCount )
    {
        poScenario->m_VResource.clear();
        return IRS_OK;
    }
    int *panIndexes = (int*)calloc( nToRemove, sizeof( int ) );
    assert( panIndexes );
    int nRemoved;
    if( nMethod == RESC_REDUCE_RAND )
    {
        nRemoved = SelectRandomIndexes( panIndexes, nCount, nToRemove );
    }
    assert( poResult );
    /* Log removal in the results db */
    poResult->SetRemovedResources( poScenario->m_VResource, panIndexes,
                                   nRemoved );

    /* Remove the resources from the simulation */
    RemoveResourceByIndexes( panIndexes, nRemoved );
    free( (void*)panIndexes );
    return IRS_OK;
}

/**
 * \brief Use an ogr datasource to mask treatments
 *
 * TODO: all of it.
 *
 * \param pszDatasource path to an OGR datasource
 * \param pszLayer Name of the layer in the datasource to use as a mask
 * \param pszWhere Where clause for the layer (ie BURNED=1)
 * \return zero on success
 */

int IRSSuite::SetTreatmentMask( const char *pszDatasource, const char *pszLayer,
                                const char *pszWhere )
{
    if( bDataLoaded )
        bDataDirty = TRUE;
    return IRS_OK;
}

/**
 * \brief Set the probability of using the treated fire data.
 *
 * TODO: all of it.
 *
 * If a mask is not set, then the probability is used across the landscape, if
 * a treatment mask is set, it only applies within the mask.
 *
 * \param dfTreatment probability a treated fire will be used (0.0 <=
 *                    dfTreatment <= 1.0
 * \return zero on success
 */

int IRSSuite::SetTreatmentProb( double dfTreatment )
{
    if( bDataLoaded )
        bDataDirty = TRUE;
    this->dfTreatment = dfTreatment;
    return IRS_OK;
}

int IRSSuite::SetOutputPath( const char *pszOutputPath )
{
    if( !pszOutputPath )
        return IRS_INVALID_INPUT;
    if( poResult )
    {
        if( EQUAL( pszOutputPath, this->pszOutputPath ) )
            poResult->Reset();
        delete poResult;
    }
    if( this->pszOutputPath )
        free( (void*)this->pszOutputPath );
    this->pszOutputPath = strdup( pszOutputPath );
    poResult = new IRSResult( pszOutputPath, pszDataPath, 1, FALSE, TRUE,
                              NULL );
    /*
    ** FIXME: Check for valid empty db.  We have to clean up if that's the
    ** case.
    */
    //poResult->Reset();
    return IRS_OK;
}

int IRSSuite::SetPreviousResultPath( const char *pszPath )
{
    assert( poResult );
    if( pszPath )
    {
        poResult->SetMetadataItem( IRS_PREV_RUN_KEY, pszPath );
        if( pszOutputPath )
        {
            const char *pszFullPath = GetFullPath( pszOutputPath );
            IRSResult oResult( pszPath, NULL, 1, FALSE, FALSE, NULL );
            oResult.SetMetadataItem( IRS_NEXT_RUN_KEY, pszFullPath );
        }
    }
    return IRS_OK;
}

int IRSSuite::WriteSingleResourceUsage()
{
    assert( poResult );
    return poResult->AddSingleResourceUsage( poScenario->GetSingleResourceUsage() );
}

int IRSSuite::SetOgrOutputPath( const char *pszPath )
{
    if( pszPath )
    {
        pszOgrOutPath = strdup( pszPath );
        return IRS_OK;
    }
    return IRS_INVALID_INPUT;
}

int IRSSuite::ExportOgr( const char *pszFormat, const char *pszFilename )
{
    assert( poResult );
    return poResult->ExportToOgr( FPU_GEOMETRY, FIRE_OUTCOME | LARGE_FIRE,
                                  pszFormat, pszFilename, NULL );
}

int IRSSuite::ExportFpuSummary( IRSProgress pfnProgress )
{
    assert( poResult );
    return poResult->ExportFpuSummary( pfnProgress );
}

/**
 * \brief Post process large fire simulator data.
 *
 * Run all escapes (or a subset) through a sampling procedure stored in a table
 * in the main db.  Some number of large fires are sampled, then averages are
 * set.
 *
 * \see IRSResult::PostProcessLargeFire
 * \param bEscapeTypes bit mask to specify a subset of escape types
 * \param dfSubSample a fraction of escapes to subset.  All escapes are not
 *                    escapes in normal terms, so we may need to artificially
 *                    limit the outcomes.
 * \param nSampleSize number of large fires to sample
 * \return zero on success, non-zero otherwise.
 */
int IRSSuite::PostProcessLargeFire( int bEscapeTypes, double dfSubSample,
                                    unsigned int nSampleSize, int nMinSize,
                                    IRSProgress pfnProgress )
{
    return poResult->PostProcessLargeFire( bEscapeTypes, dfSubSample,
                                           nSampleSize, dfTreatment, 
                                           0.075, 7, nMinSize, pfnProgress );
}

char ** IRSSuite::GetResultAttributes()
{
    return poResult->GetResultAttributes();
}

int IRSSuite::FreeResultAttributes( char ** papszAttributes )
{
    return poResult->FreeResultAttributes( papszAttributes );
}

int IRSSuite::SummarizeByFpu( IRSProgress pfnProgress )
{
    return poResult->SummarizeByFpu( pfnProgress );
}

std::list<double> IRSSuite::NationalDistribution( std::string Attribute )
{
    return poResult->NationalDistribution( Attribute );
}

std::vector<std::pair<std::string, std::list<double> > > IRSSuite::GACCDistributions( std::string Attribute )
{
    return poResult->GACCDistributions( Attribute );
}

/**
 * \brief Run multiple simulations, likely changing resources and/or treatment
 * options.
 *
 * The only things that can change during a run is resource reduction (fixed
 * agencies), and treatments.  Cannot change drawdown/preposition, threading,
 * etc.
 *
 * \param psRunData Information for each run (in the proper 'order' for
 *                  cumulative resource removal).
 * \param nRunCount number of runs
 * \return zero on success, non-zero otherwise
 */
int IRSSuite::SetMultiRun( IRSMultiRunData *psRunData, size_t nRunCount,
                           size_t nThreads, IRSProgress pfnProgress )
{
    if( nRunCount == 0 )
        return IRS_INVALID_INPUT;

    int i;
    int rc;
    /*
    ** Cursory checks.  We can only limit resources *or* fuels.
    */
    char szOutPath[IRS_MAX_PATH];
    char szPrevPath[IRS_MAX_PATH];
    char szNextPath[IRS_MAX_PATH];
    char szOgrPath[IRS_MAX_PATH];
    char szCsvPath[IRS_MAX_PATH];

    double dfStart, dfStop;
    double dfTP;
    double dfRR;
    char szMetadataValue[IRS_MAX_METADATA_SIZE];
    char *pszTmp, *pszExt;
    char *pszRootPath = (char*)GetFullPath( pszOutputPath );
    pszTmp = strrchr( pszRootPath, '.' );
    if( pszTmp == NULL )
    {
        pszExt = strdup( ".db" );
    }
    else
    {
        pszExt = strdup( pszTmp );
        /* Cut off the extension from the base */
        *pszTmp = '\0';
    }

    IRSSuite *poLocalSuite;
    /* We need better control over results, so do it locally */
    for( i = 0; i < nRunCount; i++ )
    {
        /*
        ** This is for safety, in case we have 'leaks' in our clean up.  Start
        ** fresh each round.
        */
#ifdef IRS_BE_OVERLY_SAFE
        if( poScenario )
        {
            IRSSuite::Destroy( poScenario );
        }
        poScenario = new CRunScenario();
#endif /* IRS_BE_OVERLY_SAFE */

        /*
        ** Set up the outputs.
        */
        poLocalSuite = (IRSSuite*)IRSSuite::Create( pszDataPath, NULL );
        poLocalSuite->SetSpatialFilter( poDataAccess->GetSpatialFilter() );
        poLocalSuite->SetFpuFilter( poDataAccess->GetFpuFilter() );
        snprintf( szOutPath, IRS_MAX_PATH, "%s_%d%s", pszRootPath, i, pszExt );
        poLocalSuite->SetOutputPath( szOutPath );
        const char *pszFullPath;
        if( i > 0)
        {
            snprintf( szPrevPath, IRS_MAX_PATH, "%s_%d%s", pszRootPath, i - 1, pszExt );
            pszFullPath = GetFullPath( szPrevPath );
            poLocalSuite->SetPreviousResultPath( pszFullPath );
            poLocalSuite->pszReducePrevPath = strdup( pszFullPath );
        }
        else if( i < nRunCount - 1 && nRunCount >= 2)
        {
            snprintf( szNextPath, IRS_MAX_PATH, "%s_%d%s", pszRootPath, i + 1, pszExt );
        }

        dfStart = psRunData->dfTreatPercStart;
        dfStop = psRunData->dfTreatPercStop;
        dfTP = dfStart + ((dfStop - dfStart) * ((double)i / nRunCount));

        dfStart = psRunData->dfRescRedStart;
        dfStop = psRunData->dfRescRedStop;
        dfRR = dfStart + ((dfStop - dfStart) * ((double)i / nRunCount));

        poLocalSuite->SetTreatmentProb( dfTP );
        poLocalSuite->dfReduceFraction = dfRR;
        snprintf( szMetadataValue, IRS_MAX_METADATA_SIZE, "%lf", dfTP );
        poLocalSuite->GetResultHandle()->SetMetadataItem( IRS_PERC_TREAT_KEY,
                                                          szMetadataValue );
        snprintf( szMetadataValue, IRS_MAX_METADATA_SIZE, "%lf", dfRR );
        poLocalSuite->GetResultHandle()->SetMetadataItem( IRS_RESC_RED_FRC_KEY,
                                                          szMetadataValue );
        snprintf( szMetadataValue, IRS_MAX_METADATA_SIZE, "random" );
        poLocalSuite->GetResultHandle()->SetMetadataItem( IRS_RESC_RED_MTD_KEY,
                                                          szMetadataValue );
        poLocalSuite->nReduceAgencies = psRunData->nAgencyReduction;
        poLocalSuite->nReduceTypes = psRunData->nRescTypeReduction;
        poLocalSuite->nReduceMethod = psRunData->nReductionMethod;
        poLocalSuite->nReduceRegions = psRunData->nRegionReduction;
        poLocalSuite->nTankerCount = GetTankerCount();
#ifndef IRS_ENABLE_THREADING
        poLocalSuite->LoadData( NULL );
#endif /* IRS_ENABLE_THREADING */
        int nYears = psRunData->nYearCount;
        rc = poLocalSuite->RunScenarios( nYears, nThreads, pfnProgress );
        //rc = poLocalSuite->RunAllScenarios( nThreads, NULL );
        if( rc != IRS_OK )
            return rc;
        poLocalSuite->PostProcessLargeFire( psRunData->nLargeFireMask,
                                            psRunData->dfLargeFirePerc,
                                            50, psRunData->nLargeFireMinSize,
                                            pfnProgress );
        poLocalSuite->SummarizeByFpu( pfnProgress );
        poLocalSuite->ExportFpuSummary( pfnProgress );
        if( pfnProgress )
            pfnProgress( 1.0, "Done.", NULL );
        IRSSuite::Destroy( (IRSSuiteH)poLocalSuite );
        //poScenario->Reset();
    }
    free( (void*)pszRootPath );
    free( (void*)pszExt );
    return IRS_OK;
}

int IRSSuite::ExportOgrSummary( const char *pszFormat, const char *pszOutPath,
                                char **papszOptions )
{
    assert( poResult );
    return poResult->ExportOgrSummary( pszFormat, pszOutPath, NULL );
}

int IRSSuite::ExportFpuSummaryCsv( const char *pszOutPath )
{
    assert( poResult );
    return poResult->ExportFpuSummaryCsv( pszOutPath );
}

