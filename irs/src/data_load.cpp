/****************************************************************************t*
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Load data from a given datasource
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

#include "data_load.h"
static const char * AttackMethod( int nMethod )
{
    if( nMethod == 0 )
        return "HEAD";
    else if( nMethod == 1 )
        return "TAIL";
    else
        return "PARALLEL";
}

static const char* DayOfWeek( int nDay )
{
    //return DAYS_OF_WEEK[nDay];
    switch( nDay )
    {
        case 0:
            return "Sunday";
            break;
        case 1:
            return "Monday";
            break;

        case 2:
            return "Tuesday";
            break;

        case 3:
            return "Wednesday";
            break;

        case 4:
            return "Thursday";
            break;

        case 5:
            return "Friday";
            break;

        case 6:
            return "Saturday";
            break;
        default:
            return "Sunday";
    }
}


IRSDataAccess * IRSDataAccess::Create( int type,
                                       const char *pszSource )
{
    if( type == 0 )
    {
        IRSDataAccess *poDA = new SpatialiteDataAccess( pszSource );
        if( !poDA->bSpatialEnabled )
            return NULL;
        return poDA;
    }
    return NULL;
}

IRS_Err * IRSDataAccess::Destroy( IRSDataAccess *poDA )
{
    delete poDA;
	return 0;
}

/**
 * \brief Create a default access point
 *
 */
IRSDataAccess::IRSDataAccess()
{
    pszSource = NULL;
}

IRSDataAccess::IRSDataAccess( const char *pszSource )
{
    SetSource( pszSource, NULL );
}

IRSDataAccess::~IRSDataAccess()
{
    //if( pszSource )
    //    free( (void*)pszSource );
}

IRS_Err IRSDataAccess::SetSource( const char *pszSource,
                                  const char * const *papszOptions )
{
    assert( pszSource != NULL );
    this->pszSource = strdup( pszSource );
    return 0;
}

/**
 * \brief Replace spaces with underscores in FWA name
 *
 * \param pszIn name to launder
 * \return new name with spaces changed to '_' to be freed by caller
 */

char * IRSDataAccess::LaunderFwa( const char *pszIn )
{
    if( pszIn == NULL )
        return NULL;
    char * pszOut = strdup( pszIn );
    for( int i = 0;i < strlen( pszOut );i++ )
    {
        if( pszOut[i] == ' ' )
            pszOut[i] = '_';
    }
    //CPLDebug( "IRS", "Laundering fwa name: %s -> %s", pszIn, pszOut );
    return pszOut;
}

/**
 * \brief Copy a vector of resources
 *
 * This will be used for threading.  The resources vector is the only mutable
 * vector in the simulation, each thread needs a copy.
 *
 * \param aoResourcesIn the vector to copy
 * \return a new vector with the same resources
 */
IRS_Err
IRSDataAccess::CopyResources( const std::vector<CResource*> aoResourcesIn,
                              std::vector<CResource*> &aoResourcesCopy)
{
    if( aoResourcesIn.size() < 1 )
        return 0;
    aoResourcesCopy.clear();
    aoResourcesCopy.reserve( aoResourcesIn.size() );
    for( int i = 0; i < aoResourcesIn.size(); i++ )
    {
        std::string osType = aoResourcesIn[i]->GetRescType().GetRescType();
        if( EQUALN( osType.c_str(), "DZR", 3 ) ||
            EQUALN( osType.c_str(), "TP", 2 ) )
        {
            aoResourcesCopy.push_back( new CConstProd( *((CConstProd*)aoResourcesIn[i]) ) );
        }
        else if( EQUAL( osType.c_str(), "CRW" ) )
        {
            aoResourcesCopy.push_back( new CCrew( *((CCrew*)aoResourcesIn[i]) ) );
        }
        else if( EQUALN( osType.c_str(), "EN", 2 ) )
        {
            aoResourcesCopy.push_back( new CEngine( *((CEngine*)aoResourcesIn[i]) ) );
        }
        else if( EQUAL( osType.c_str(), "WT" ) )
        {
             aoResourcesCopy.push_back( new CWaterTender( *((CWaterTender*)aoResourcesIn[i]) ) );
        }
        else if( EQUAL( osType.c_str(), "ATT" ) )
        {
            aoResourcesCopy.push_back( new CAirtanker( *((CAirtanker*)aoResourcesIn[i]) ) );
        }
        else if( EQUAL( osType.c_str(), "SEAT" ) ||
                 EQUAL( osType.c_str(), "SCP" ) )
        {
            aoResourcesCopy.push_back( new CSmallAT( *((CSmallAT*)aoResourcesIn[i]) ) );
        }
        else if( EQUAL( osType.c_str(), "SMJR" ) )
        {
            aoResourcesCopy.push_back( new CSmokejumper( *((CSmokejumper*)aoResourcesIn[i]) ) );
        }
        else if( EQUAL( osType.c_str(), "SJAC" ) )
        {
             aoResourcesCopy.push_back( new CSMJAircraft( *((CSMJAircraft*)aoResourcesIn[i]) ) );
        }
        else if( EQUAL( osType.c_str(), "HELI" ) )
        {
            aoResourcesCopy.push_back( new CHelitack( *((CHelitack*)aoResourcesIn[i]) ) );
        }
        else if( EQUALN( osType.c_str(), "HEL", 3 ) )
        {
            aoResourcesCopy.push_back( new CHelicopter( *((CHelicopter*)aoResourcesIn[i]) ) );
        }
        else
        {
            return 1;
        }
    }
    return 0;
}

IRS_Err IRSDataAccess::CleanUpResources( std::vector<CResource*>&resc )
{
    for( int i = 0;i < resc.size();i++ )
        delete resc[i];
    return 0;
}

std::vector<int>
IRSDataAccess::SmoothNumFires( const std::vector<int>daily_fires, int nDays )
{
    assert( daily_fires.size() == 365 );
    int i = 0;
    int j = 0;
    double fires = 0;
    int count = daily_fires.size();
    std::vector<int>smooth( count );
    for( i = 0; i < count - nDays; i++ )
    {
        fires = 0;
        for( j = 0; j < nDays; j++ )
        {
            fires += daily_fires[i+j];
        }
        smooth[i] = (int)( ( fires + 0.5 ) / nDays );
    }
    for( i = count - nDays; i < count; i ++ )
    {
        fires = 0;
        for( j = i; j < count; j++ )
        {
            fires += daily_fires[j];
        }
        smooth[i] = (int)( ( fires + 0.5 ) / ( count - i ) );
    }
    return smooth;
}

void IRSDataAccess::DestroyStringList( char **papszList )
{
    if( !papszList )
        return;
    int i = 0;
    while( papszList[i] != NULL )
        sqlite3_free( (void*)papszList[i++] );
}

#ifdef IRS_HAVE_SPATIALITE
SpatialiteDataAccess::SpatialiteDataAccess() : IRSDataAccess()
{
    pszSource = NULL;
    sqlite3_initialize();
}

SpatialiteDataAccess::SpatialiteDataAccess( const char *pszSource )
                     : IRSDataAccess( pszSource )
{
    pszFilter = NULL;
    pszFpuFilter = NULL;
    int rc;
    //spatialite_init( 0 );
    rc = sqlite3_initialize();
    /* Can't return an error, check on first call */
    rc = sqlite3_open_v2( pszSource, &db, SQLITE_OPEN_READONLY, NULL);
    rc = sqlite3_enable_load_extension( db, 1 );
    char *pszErr;
    rc = sqlite3_load_extension( db, SPATIALITE_EXTENSION,
                                 SPATIALITE_INIT, &pszErr );
    if(rc)
    {
        //CPLError(CE_Warning, CPLE_AppDefined, "====\n\n"
        //         "Could not load libspatialite. "
        //         "sqlite return code:%d\n\n====", rc);
        fprintf(stderr,  "====\n\nCould not load libspatialite. "
                         "sqlite return code:%d\n\n====", rc);

        bSpatialEnabled = FALSE;
    }
    else
        bSpatialEnabled = TRUE;
    //rc = sqlite3_exec( db, "PRAGMA cache_size = 4096", NULL, NULL, &pszErr );
    /* Not really useful because we don't do inserts */
    //rc = sqlite3_exec( db, "PRAGMA synchronous = OFF", NULL, NULL, &pszErr );
    //rc = sqlite3_exec( db, "PRAGMA journal_mode = WAL", NULL, NULL, &pszErr );
}

SpatialiteDataAccess::~SpatialiteDataAccess() 
{
    sqlite3_close( db );
    db = NULL;
    sqlite3_shutdown();
    if( pszSource )
        free( (void*)pszSource );
    if( pszFilter )
        free( (void*)pszFilter );
    if( pszFpuFilter )
        free( (void*)pszFpuFilter );
}

/**
 * \brief Load all the spatially dependent data.
 *
 * This is everything based on FWA data.  Any FWAs intersecting the area of
 * interest will be used.  If no options are provided, the spatial filter is
 * checked.  If it isn't set, use all available data.  Some options (to be
 * implemented) are:
 * FPA_FILTER=FPA_NAME
 * GACC_FILTER=GACC_NAME
 * DIST_CALC=FIRE/DIST_CALC=TRAVEL_POINT
 *
 * This is quite a large function and will probably be broken up a little bit.
 * Tough to tell how it's gonna go due to lack of experience in databases for
 * kyle.
 *
 * \note The geometry for the Spatialite reader must be in WKT, and must create
 * a valid geometry using GeomFromText(), ie GeomFromText(POLYGON((x1 y1, x2
 * y2, x3 y3, x4 y4, x1 y1))
 *
 * \param [out] fwa vector of FWAs to fill
 * \param [out] disp_loc vector of dispatch locations to fill
 * \param [out] logic vector of dispatch logics to fill
 * \param papszOptions name/value options
 * \return zero on success, non-zero otherwise
 */
IRS_Err SpatialiteDataAccess::LoadSpatial( std::vector<CFWA>&fwa,
                                           std::vector<CDispLoc>&disp_loc,
                                           std::vector<CDispLogic>&logic,
                                           std::vector<CResource*>&resc,
                                           const char * const * papszOptions )
{
    return 0;
}

IRS_Err SpatialiteDataAccess::LoadAllData( CDataMaster *poScenario,
                                           const char *pszExternalResourceDb )
{
    this->LoadRescType( poScenario->m_VRescType, NULL );
    this->LoadProdRates( poScenario->m_VProdRates, NULL );
    this->LoadDispatchLogic( poScenario->m_VDispLogic, NULL, NULL );
    this->LoadFwa( poScenario->m_VDispLogic, poScenario->m_VFWA, NULL, NULL );
    this->LoadDispatchLocation( poScenario->m_VFWA, poScenario->m_VDispLoc, NULL );
    this->LoadTankerBases( poScenario, poScenario->m_VFWA, poScenario->m_VDispLoc, -1, NULL );
    this->LoadResource( poScenario, poScenario->m_VRescType, poScenario->m_VDispLoc,
                        poScenario->m_VResource, NULL, NULL, NULL, NULL, NULL );

    poScenario->m_NumFWA = poScenario->m_VFWA.size();
    poScenario->m_NumRescType = poScenario->m_VRescType.size();
    poScenario->m_NumDispLoc = poScenario->m_VDispLoc.size();
    poScenario->m_NumProdRates = poScenario->m_VProdRates.size();
    poScenario->m_NumResource = poScenario->m_VResource.size();

    poScenario->CreateDispTree();
    poScenario->ResourcesToDispatchers();
    //poScenario->OpenLevelsFile();
	return 0;
}

static int AddGmtOffset( char *pszTime, double dfOffset )
{
    int nMinute, nHour;
    nMinute =  atoi( &pszTime[2] );
    pszTime[2] = '\0';
    nHour = atoi( pszTime );
    nHour = nHour + dfOffset;
    if( nHour < 0 )
        nHour = 24 + nHour;
    sprintf( pszTime, "%02d%02d", (int)nHour, (int)nMinute );
    return 0;
}

/*
** Get a merged geometry for administrative boundaries.  Combine fpu geometries
** to get regions or states, the filter is either a region, eg GB, a state
** abbrev, eg ID, or an fpu, GB_ID_002. Results stored in pszGeometry (null on
** failure), and nSize (0 if it fails).  Return non-zero on failure.
** pszGeometry gets allocated in here, and should be freed by the caller.
*/

int SpatialiteDataAccess::GetUnionedGeometry( const char *pszFilter, char **pszGeometry,
                                              int *nSize )
{
    int rc;
    int rc2 = 0;
    sqlite3_stmt *stmt;
    const char *pabyGeometry;
    rc = sqlite3_prepare_v2( db, "SELECT ST_Union(fpu_bndry.geometry) "
                                 "FROM fpu_bndry WHERE fpu_code LIKE "
                                 "? || '%' ", -1, &stmt, NULL );
    rc = sqlite3_bind_text( stmt, 1, pszFilter, -1, NULL );
    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW )
    {
        fprintf( stderr, "Invalid FPU/GACC filter\n" );
        sqlite3_finalize( stmt );
        return 1;
    }
    int nGeomSize = sqlite3_column_bytes( stmt, 0 );
    if( !nGeomSize )
    {
        /*
        ** Try for the state filter.  It's possible we could just use this
        ** to start, and get whatever we get.  There may be some strange
        ** artifacts though.
        */
        rc = sqlite3_reset( stmt );
        rc = sqlite3_prepare_v2( db, "SELECT ST_Union(fpu_bndry.geometry) "
                                     "FROM fpu_bndry WHERE fpu_code LIKE "
                                     "'%' || ? || '%' ", -1, &stmt, NULL );
        rc = sqlite3_bind_text( stmt, 1, pszFilter, -1, NULL );
        rc = sqlite3_step( stmt );
        if( rc != SQLITE_ROW )
        {
            fprintf( stderr, "Invalid FPU/GACC filter\n" );
            rc2 = 1;
        }
    }
    nGeomSize = sqlite3_column_bytes( stmt, 0 );
    pabyGeometry = (const char *)sqlite3_column_blob( stmt, 0 );
    if( !pabyGeometry || !nGeomSize )
    {
        fprintf( stderr, "Invalid FPU/GACC filter\n" );
        rc2 = 1;
    }
    if( nSize )
    {
        if( rc2 )
        {
            *nSize = 0;
        }
        else
        {
            *nSize = nGeomSize;
        }
    }
    if( rc2 )
    {
        *pszGeometry = NULL;
    }
    else
    {
        (*pszGeometry) = (char*)malloc( nGeomSize );
        memcpy( (*pszGeometry), pabyGeometry, nGeomSize );
    }
    sqlite3_finalize( stmt );
    return rc2;
}

IRS_Err SpatialiteDataAccess::LoadScenario( const std::vector<CFWA>&fwas,
                                            std::vector<CFire>&fires,
                                            int year,
                                            double dfApplyTreatment,
                                            const char * const *papszOptions )
{

    /* Get the scenario information, then fwa info to make an fwa class */
    sqlite3_stmt *stmt;
    sqlite3_stmt *fwa_stmt;
    sqlite3_stmt *spc_stmt;
    sqlite3_stmt *fpu_stmt;
    sqlite3_stmt *union_stmt = NULL;
    int rc;
    char *pabyGeometry;
    int nGeomSize = 0;
    rc = sqlite3_exec( db, "BEGIN TRANSACTION", NULL, NULL, NULL );
    /* Filter */
    if( pszFilter )
    {
        rc = sqlite3_prepare_v2( db, "SELECT * FROM scenario WHERE "
                                     "ST_Contains(GeomFromText(?1), scenario.geometry) "
                                     "AND year=?2 AND region < 7 AND "
                                     "region IS not NULL AND "
                                     "fwa_id NOT LIKE '%other%' AND "
                                     "fwa_id NOT LIKE '%unassign%' AND "
                                     "fwa_id NOT LIKE '%water%' AND "
                                     "scenario.ROWID IN(SELECT pkid FROM "
                                     "idx_scenario_geometry WHERE "
                                     "xmin <= MbrMaxX(GeomFromText(?1)) AND "
                                     "xmax >= MbrMinX(GeomFromText(?1)) AND "
                                     "ymin <= MbrMaxY(GeomFromText(?1)) AND "
                                     "ymax >= MbrMinY(GeomFromText(?1))) "
                                     "ORDER BY jul_day, disc_time",
                                 -1, &stmt, NULL );
        rc = sqlite3_bind_text( stmt, 1, pszFilter, -1, NULL );
    }
    else if( pszFpuFilter )
    {
        if( strlen( pszFpuFilter ) == 0 )
        {
            fprintf( stderr, "Invalid FPU/GACC filter\n" );
            sqlite3_finalize( union_stmt );
            return 1;
        }
        if( strlen( pszFpuFilter ) >= 2 )
        {
            rc = GetUnionedGeometry( pszFpuFilter, &pabyGeometry, &nGeomSize );
            if( !pabyGeometry || rc || !nGeomSize )
            {
                fprintf( stderr, "Invalid FPU/GACC filter\n" );
                sqlite3_finalize( union_stmt );
                return 1;
            }
            rc = sqlite3_prepare_v2( db, "SELECT * FROM scenario WHERE "
                                         "MBRContains(?1, scenario.geometry) "
                                         "AND ST_Contains(?1, scenario.geometry) "
                                         "AND year=?2 AND region < 7 "
                                         "AND region IS NOT NULL AND "
                                         "fwa_id NOT LIKE '%other%' AND "
                                         "fwa_id NOT LIKE '%unassign%' AND "
                                         "fwa_id NOT LIKE '%water%' AND "
                                         "scenario.ROWID IN(SELECT pkid FROM "
                                         "idx_scenario_geometry WHERE "
                                         "xmin <= MbrMaxX(?1) AND "
                                         "xmax >= MbrMinX(?1) AND "
                                         "ymin <= MbrMaxY(?1) AND "
                                         "ymax >= MbrMinY(?1)) "
                                         "ORDER BY jul_day, disc_time",
                                     -1, &stmt, NULL );
            rc = sqlite3_bind_blob( stmt, 1, (void*)pabyGeometry, nGeomSize, SQLITE_STATIC );
        }
        else if( strlen( pszFpuFilter ) == 9 )
        {
            rc = sqlite3_prepare_v2( db, "SELECT scenario.* FROM fpu_bndry JOIN scenario " \
                                         "ON ST_Contains(fpu_bndry.geometry, scenario.geometry) " \
                                         //"WHERE MBRContains(fpu_bndry.geometry, scenario.geometry) "
                                         "WHERE fpu_bndry.fpu_code=?1 AND region < 7 "
                                         "AND year=?2 AND region < 7 "
                                         "AND region IS NOT NULL AND "
                                         "fwa_id NOT LIKE '%other%' AND "
                                         "fwa_id NOT LIKE '%unassign%' AND "
                                         "fwa_id NOT LIKE '%water%' AND "
                                         "scenario.ROWID IN(SELECT pkid FROM "
                                         "idx_scenario_geometry WHERE "
                                         "xmin <= MbrMaxX(fpu_bndry.geometry) AND "
                                         "xmax >= MbrMinX(fpu_bndry.geometry) AND "
                                         "ymin <= MbrMaxY(fpu_bndry.geometry) AND "
                                         "ymax >= MbrMinY(fpu_bndry.geometry)) "
                                         "ORDER BY jul_day, disc_time",
                                     -1, &stmt, NULL );
            rc = sqlite3_bind_text( stmt, 1, pszFpuFilter, -1, NULL );
        }
    }
    else
    {
        rc = sqlite3_prepare_v2( db, "SELECT * FROM scenario WHERE year =?2 AND "
                                "region < 7 AND region IS NOT NULL AND "
                                "fwa_id NOT LIKE '%other%' AND "
                                "fwa_id NOT LIKE '%unassign%' AND "
                                "fwa_id NOT LIKE '%water%' "
                                "ORDER BY jul_day, disc_time",
                            -1, &stmt, NULL );
    }
    /* bind the year to any of the above statements */
    rc = sqlite3_bind_int( stmt, 2, year );

    rc = sqlite3_prepare_v2( db, "SELECT name FROM fwa WHERE id=?", -1,
                             &fwa_stmt, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT name FROM fpu where id=?", -1,
                             &fpu_stmt, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT fuel_model, spec_cond FROM spec_cond "
                                 "where fpu_id=? and fuel_model=?", -1,
                             &spc_stmt, NULL );

    int nYear, nNum, nDay;
    const char *pszDayOfWeek, *pszDiscTime;
    int nBI;
    double dfROS;
    int nFuelModel; 
    const char *pszSpecCond;
    int nSlopePerc, bWalkIn;
    const char *pszTactic;
    double dfDistance;
    int nElev;
    double dfLenWidth;
    int nMinSteps, nMaxSteps;
    const char *pszSunrise, *pszSunset;
    int bWaterDrops, bPumpRoll, nFwaId;
    double dfLongitude, dfLatitude;
    const char *pszFwaName;
    char abyTimeBuf[128];
    int nTime;
    int nSunRise, nSunSet;
    Random rand;
    int bTreated;
    double dfSample;
    int index = 0;
    double dfGmtOffset;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        nYear = sqlite3_column_int( stmt, 0 );
        nNum = sqlite3_column_int( stmt, 1 );
        nDay = sqlite3_column_int( stmt, 2 );
        pszDayOfWeek = DayOfWeek( sqlite3_column_int( stmt, 3 ) );
        nTime = sqlite3_column_int( stmt, 4 );
        sprintf( abyTimeBuf, "%04d", nTime );
        pszDiscTime = strdup( abyTimeBuf );
        nElev = sqlite3_column_int( stmt, 7 );
        nSlopePerc = sqlite3_column_int( stmt, 10 );
        bWalkIn = sqlite3_column_int( stmt, 11 );
        pszTactic = AttackMethod( sqlite3_column_int( stmt, 12 ) );
        dfDistance = sqlite3_column_double( stmt, 13 );
        nMinSteps = sqlite3_column_int( stmt, 15 );
        nMaxSteps = sqlite3_column_int( stmt, 16 );
        dfGmtOffset = sqlite3_column_double( stmt, 26 );
        nTime = sqlite3_column_int( stmt, 17 );
        sprintf( abyTimeBuf, "%04d", nTime );
        AddGmtOffset( abyTimeBuf, dfGmtOffset );
        pszSunrise = strdup( abyTimeBuf );
        nTime = sqlite3_column_int( stmt, 18 );
        sprintf( abyTimeBuf, "%04d", nTime );
        AddGmtOffset( abyTimeBuf, dfGmtOffset );
        pszSunset = strdup( abyTimeBuf );
        /* Sunrise tomorrow is 21 */
        bWaterDrops = sqlite3_column_int( stmt, 20 );
        bPumpRoll = sqlite3_column_int( stmt, 21 );
        /*
         * XXX: Currently fwa is stored as a name in scenario, I imagine that
         * will change, have to fix here, but just getting the name for now.
         */
        pszFwaName = (const char*)sqlite3_column_text( stmt, 22 );
        dfLongitude = sqlite3_column_double( stmt, 23 );
        dfLatitude = sqlite3_column_double( stmt, 24 );
        assert(sqlite3_column_int( stmt, 5 ) == sqlite3_column_int( stmt, 28 ) );
        bTreated = FALSE;
        if( dfApplyTreatment > 0.0 )
        {
            dfSample = rand.rand3();
            if( dfSample < dfApplyTreatment )
            {
                nBI = sqlite3_column_int( stmt, 28 );
                dfROS = sqlite3_column_double( stmt, 29 );
                nFuelModel = sqlite3_column_int( stmt, 30 );
                pszSpecCond = (const char*)sqlite3_column_text( stmt, 31 );
                dfLenWidth = sqlite3_column_double( stmt, 32 );
                bTreated = TRUE;
            }
            else
            {
                nBI = sqlite3_column_int( stmt, 5 );
                dfROS = sqlite3_column_double( stmt, 6 );
                nFuelModel = sqlite3_column_int( stmt, 8 );
                pszSpecCond = (const char*)sqlite3_column_text( stmt, 9 );
                dfLenWidth = sqlite3_column_double( stmt, 14 );
            }
        }
        else
        {
            nBI = sqlite3_column_int( stmt, 5 );
            dfROS = sqlite3_column_double( stmt, 6 );
            nFuelModel = sqlite3_column_int( stmt, 8 );
            pszSpecCond = (const char*)sqlite3_column_text( stmt, 9 );
            dfLenWidth = sqlite3_column_double( stmt, 14 );
        }
        //nFwaId = sqlite3_column_int( stmt, 21 );
        //sqlite3_bind_int( fwa_stmt, 1, nFwaId );
        //sqlite3_step( fwa_stmt );
        //pszFwaName = (const char*)sqlite3_column_text( fwa_stmt, 1 );
        int i = 0;
        while( i < fwas.size() )
        {
            if( fwas[i].GetFWAID() == std::string( pszFwaName ) )
            {
                break;
            }
            i++;
        }

        if( i == fwas.size() && fwas[i-1].GetFWAID() != std::string( pszFwaName ) )
        {
            //CPLError( CE_Failure, CPLE_AppDefined, "Could not find fwa in "
            //    "database, skipping scenario" );
            fprintf(stderr, "Could not find fwa in database, skipping scenario" );
            continue;
        }
        /* Make a fwa object */

        fires.push_back( CFire( nYear, nNum, nDay, std::string( pszDayOfWeek ),
                                std::string( pszDiscTime ), nBI, dfROS,
                                nFuelModel, std::string( pszSpecCond ),
                                nSlopePerc, (bool)bWalkIn, 
                                std::string( pszTactic ), dfDistance,
                                nElev, dfLenWidth, nMinSteps,
                                nMaxSteps, std::string( pszSunrise ),
                                std::string( pszSunset ), bWaterDrops,
                                (bool)bPumpRoll, (CFWA&)fwas[i], dfLatitude,
                                dfLongitude ));
        fires[index].SetTreated( bTreated );
        index++;
        free( (void*)pszDiscTime );
        free( (void*)pszSunrise );
        free( (void*)pszSunset );
    }
    sqlite3_finalize( stmt );
    sqlite3_finalize( fwa_stmt );
    sqlite3_finalize( spc_stmt );
    sqlite3_finalize( fpu_stmt );
    sqlite3_finalize( union_stmt );
    stmt = fwa_stmt = spc_stmt = fpu_stmt = union_stmt = NULL;
    sqlite3_exec( db, "END TRANSACTION", NULL, NULL, NULL );
    return 0;
}

std::vector<int> SpatialiteDataAccess::GetYearIndexes()
{
    sqlite3_stmt *stmt;
    int rc;
    rc = sqlite3_prepare_v2( db, "SELECT DISTINCT(year) from scenario;", -1, 
                             &stmt, NULL );
    std::vector<int>years;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        years.push_back(sqlite3_column_int( stmt, 0 ) );
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return years;
}

IRS_Err SpatialiteDataAccess::LoadFwa( std::vector<CDispLogic>&logic,
                                       std::vector<CFWA>&fwas,
                                       IRSProgress pfnProgress,
                                       const char * const *papszOptions )
{
    int rc;
    sqlite3_stmt *fwa_stmt = NULL;
    sqlite3_stmt *fpu_stmt = NULL;
    sqlite3_stmt *walk_stmt = NULL;
    sqlite3_stmt *used_stmt = NULL;
    sqlite3_stmt *unused_stmt = NULL;
    sqlite3_stmt *escape_stmt = NULL;
    sqlite3_stmt *reload_stmt = NULL;
    sqlite3_stmt *diurnal_stmt = NULL;
    sqlite3_stmt *rosadj_stmt = NULL;
    sqlite3_stmt *rosfuel_stmt = NULL;
    sqlite3_stmt *dlg_stmt = NULL;
    sqlite3_exec( db, "BEGIN TRANSACTION", NULL, NULL, NULL );
    /* Prepare */
    /* Filter */
    if( pszFilter != NULL )
    {
        sqlite3_prepare_v2( db, "SELECT fwa.* FROM fwa LEFT JOIN fwa_bndry "
                                "ON (fwa.fwa_gis_id=fwa_bndry.fwa_gis_id) WHERE "
                                "fwa.name NOT LIKE '%other%' AND "
                                "fwa.name NOT LIKE '%unassign%' AND "
                                "fwa.name NOT LIKE '%water%' AND "
                                "ST_Intersects(fwa_bndry.geometry, "
                                "GeomFromText(?1)) AND "
                                "fwa_bndry.ROWID IN (SELECT pkid FROM "
                                "idx_fwa_bndry_geometry WHERE "
                                "xmin <= MbrMaxX(GeomFromText(?1)) AND "
                                "xmax >= MbrMinX(GeomFromText(?1)) AND "
                                "ymin <= MbrMaxY(GeomFromText(?1)) AND "
                                "ymax >= MbrMinY(GeomFromText(?1)))",
                            -1, &fwa_stmt, NULL );
        sqlite3_bind_text( fwa_stmt, 1, pszFilter, -1, NULL );
    }
    else if( pszFpuFilter != NULL )
    {
        /* FIXME: add filtering */
        rc = sqlite3_prepare_v2( db, "SELECT * FROM fwa LEFT JOIN fpu "
                                     "ON fwa.fpu_id=fpu.id "
                                     "WHERE fpu.name LIKE '%' || ? || '%'",
                                 -1, &fwa_stmt, NULL );
        rc = sqlite3_bind_text( fwa_stmt, 1, pszFpuFilter, -1, NULL );
    }
    else
    {
        sqlite3_prepare_v2( db, "SELECT * FROM fwa", -1, &fwa_stmt, NULL );
    }

    sqlite3_prepare_v2( db, "SELECT name FROM fpu WHERE id=?", -1, 
                        &fpu_stmt, NULL );

    sqlite3_prepare_v2( db, "SELECT * FROM walk_delay WHERE fwa_id=?", -1, 
            &walk_stmt, NULL );
    /* Post-used delay */
    sqlite3_prepare_v2( db, "SELECT * FROM post_used WHERE fwa_id=?", -1, 
            &used_stmt, NULL );
    /* Post-unused delay */
    sqlite3_prepare_v2( db, "SELECT * FROM post_unused WHERE fwa_id=?", -1, 
            &unused_stmt, NULL );
    /* Escape delay */
    sqlite3_prepare_v2( db, "SELECT * FROM post_escape WHERE fwa_id=?", -1, 
            &escape_stmt, NULL );
    /* Escape delay */
    sqlite3_prepare_v2( db, "SELECT * FROM reload_delay WHERE fwa_id=?", -1, 
            &reload_stmt, NULL );
    /* Diurnal Coefficients */
    sqlite3_prepare_v2( db, "SELECT * FROM diurn_coeff WHERE fwa_id=?", -1, 
            &diurnal_stmt, NULL );
    /* Rate of spread adjustments */
    sqlite3_prepare_v2( db, "SELECT * FROM  ros_coeff WHERE fwa_id=?", -1, 
            &rosadj_stmt, NULL );

    /* Dispatch location data */
    /* Associations/travel times */
    /* Dispatch logic data */
    sqlite3_prepare_v2( db, "SELECT name FROM disp_logic WHERE id=?", -1,
            &dlg_stmt, NULL );

    int nCounter;

    int nFwaIndex = 0;
    int nFwaId;
    int nFpuId;
    const char *pszFwaName;
    const char *pszFmgName;
    int nWalkIn, nPumpRoll, nHead, nTail, nParallel, nAttDist;
    int bWaterDrops, bExcluded;
    double dfDiscSize;
    int nEslTime, nEslSize;
    double dfAirGround;
    int nFirstDelay, nDispLogId;
    const char *pszDispLogName;
    const char *pszFpuName;

    while( sqlite3_step( fwa_stmt ) == SQLITE_ROW )
    {
        bExcluded = sqlite3_column_int( fwa_stmt, 11 );
        if( bExcluded )
        {
            continue;
        }
        nFwaId = sqlite3_column_int( fwa_stmt, 0 );
        pszFwaName = (const char*)sqlite3_column_text( fwa_stmt, 1 );
        nFpuId = sqlite3_column_int( fwa_stmt, 2 );
        pszFmgName = (const char*)sqlite3_column_text( fwa_stmt, 3 );
        nWalkIn = sqlite3_column_int( fwa_stmt, 4 );
        nPumpRoll = sqlite3_column_int( fwa_stmt, 5 );
        nHead = sqlite3_column_int( fwa_stmt, 6 );
        nTail = sqlite3_column_int( fwa_stmt, 7 );
        nParallel = sqlite3_column_int( fwa_stmt, 8 );
        nAttDist = sqlite3_column_int( fwa_stmt, 9 );
        bWaterDrops = sqlite3_column_int( fwa_stmt, 10 );
        dfDiscSize = sqlite3_column_double( fwa_stmt, 12 );
        nEslTime = sqlite3_column_int( fwa_stmt, 13 );
        nEslSize = sqlite3_column_int( fwa_stmt, 14 );
        dfAirGround = sqlite3_column_double( fwa_stmt, 15 );
        nFirstDelay = sqlite3_column_int( fwa_stmt, 16 );
        nDispLogId = sqlite3_column_int( fwa_stmt, 17 );

        sqlite3_bind_int( fpu_stmt, 1, nFpuId );
        if( sqlite3_step( fpu_stmt ) != SQLITE_ROW )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not locate "
            //        "fpu name for fwa id:%d in database, improper id",
            //        nFwaId );
            fprintf(stderr, "Could not locate "
                "fpu name for fwa id:%d in database, improper id",
                nFwaId );

            sqlite3_reset( fwa_stmt );
            continue;
        }
        pszFpuName = (const char*)sqlite3_column_text( fpu_stmt, 0 );

        sqlite3_bind_int( walk_stmt, 1, nFwaId );
        int anWalkDelay[6];
        if( sqlite3_step( walk_stmt ) != SQLITE_ROW )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not locate "
            //        "walkin delay for fwa id:%d in database, improper id",
            //        nFwaId );
            fprintf( stderr, "Could not locate "
                    "walkin delay for fwa id:%d in database, improper id",
                    nFwaId );

            sqlite3_reset( fwa_stmt );
            sqlite3_reset( walk_stmt );
            continue;
        }
        for( nCounter = 0; nCounter < 6; nCounter++ )
        {
            anWalkDelay[nCounter] = sqlite3_column_int( walk_stmt, nCounter+1 );
        }
        sqlite3_reset( walk_stmt );

        /* Get the Post-used data */
        sqlite3_bind_int( used_stmt, 1, nFwaId );
        int anPostUsed[6];
        if( sqlite3_step( used_stmt ) != SQLITE_ROW )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not locate "
                    //"Post-used data for fwa id:%d in database, improper id",
                    //nFwaId );
            fprintf( stderr, "Could not locate "
                    "Post-used data for fwa id:%d in database, improper id",
                    nFwaId );

            continue;
        }
        for( nCounter = 0; nCounter < 6; nCounter++ )
        {
            anPostUsed[nCounter] = sqlite3_column_int( used_stmt, nCounter+1 );
        }
        sqlite3_reset( used_stmt );

        /* Get the Post-unused data */
        sqlite3_bind_int( unused_stmt, 1, nFwaId );
        int anPostUnused[6];
        if( sqlite3_step( unused_stmt ) != SQLITE_ROW )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not locate "
                    //"Post-used data for fwa id:%d in database, improper id",
                    //nFwaId );
            fprintf( stderr, "Could not locate "
                    "Post-used data for fwa id:%d in database, improper id",
                    nFwaId );

            continue;
        }
        for( nCounter = 0; nCounter < 6; nCounter++ )
        {
            anPostUnused[nCounter] = sqlite3_column_int( unused_stmt, nCounter+1 );
        }
        sqlite3_reset( unused_stmt );

        /* Get the Post-escape data */
        sqlite3_bind_int( escape_stmt, 1, nFwaId );
        int anPostEscape[6];
        if( sqlite3_step( escape_stmt ) != SQLITE_ROW )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not locate "
                    //"Post-escape data for fwa id:%d in database, improper id",
                    //nFwaId );
            fprintf( stderr, "Could not locate "
                    "Post-escape data for fwa id:%d in database, improper id",
                    nFwaId );

            continue;
        }
        for( nCounter = 0; nCounter < 6; nCounter++ )
        {
            anPostEscape[nCounter] = sqlite3_column_int( escape_stmt, nCounter+1 );
        }
        sqlite3_reset( escape_stmt );

        /* Get the reload data */
        sqlite3_bind_int( reload_stmt, 1, nFwaId );
        int anReload[5];
        if( sqlite3_step( reload_stmt ) != SQLITE_ROW )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not locate "
                    //"reload data for fwa id:%d in database, improper id",
                    //nFwaId );
            fprintf( stderr, "Could not locate "
                    "reload data for fwa id:%d in database, improper id",
                    nFwaId );

            continue;
        }
        for( nCounter = 0; nCounter < 5; nCounter++ )
        {
            anReload[nCounter] = sqlite3_column_int( reload_stmt, nCounter+1 );
        }
        sqlite3_reset( reload_stmt );

        /* Get the diurnal coefficient data */
        sqlite3_bind_int( diurnal_stmt, 1, nFwaId );
        double adfDiurnal[24];
        if( sqlite3_step( diurnal_stmt ) != SQLITE_ROW )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not locate "
                    //"diurnal data for fwa id:%d in database, improper id",
                    //nFwaId );
            fprintf( stderr, "Could not locate "
                    "diurnal data for fwa id:%d in database, improper id",
                    nFwaId );
            continue;
        }
        for( nCounter = 0; nCounter < 24; nCounter++ )
        {
            adfDiurnal[nCounter] = sqlite3_column_double( diurnal_stmt, nCounter+1 );
        }
        sqlite3_reset( diurnal_stmt );

        /*
         * XXX Skipping for now, this was a 'calibration' system for old fpa,
         * XXX ie to 'cheat'. Plus it's wrong. -kss
         */

        /* Get the rate of spread coefficient data */
        std::string aoRos[10];
        double adfRosCoeff[10];
        /*
         * XXX sqlite3_bind_int( rosadj_stmt, 1, nFwaId );
         * XXX nCounter = 0;
         * XXX while( sqlite3_step( rosadj_stmt ) == SQLITE_ROW )
         * XXX {
         * XXX    papszRosAdj = aoRos[nCounter] =
         * XXX    std::string( (const char*)sqlite3_column_text( rosadj_stmt, 1 ) );
         * XXX   adfDiurnal[nCounter++] = sqlite3_column_double( rosadj_stmt, 2 );
         * XXX}
         */

        /* Fill in default values */
        for( int i = 0; i < 10; i++ )
        {
            aoRos[i] = std::string( "NA" );
            adfRosCoeff[i] = 1.0;
        }

        /* Find the dispatch logic name */
        sqlite3_bind_int( dlg_stmt, 1, nDispLogId );
        if( sqlite3_step( dlg_stmt ) != SQLITE_ROW )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not find dispatch "
                    //"logic name in database for fwa id: %d", nFwaId );
            fprintf( stderr, "Could not find dispatch "
                    "logic name in database for fwa id: %d", nFwaId );

            continue;
        }
        pszDispLogName = (const char*)sqlite3_column_text( dlg_stmt, 0 );

        nCounter = 0;
        while( nCounter < logic.size() )
        {
            if( logic[nCounter].GetLogicID() == std::string( pszDispLogName )
                || nCounter + 1 == logic.size() )
                break;
            nCounter++;
        }
        if( logic[nCounter].GetLogicID() != std::string( pszDispLogName ) )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not locate "
                    //"dispatch logic data for fwa id:%d in database, "
                    //"improper id",
                    //nFwaId );
            fprintf( stderr, "Could not locate "
                    "dispatch logic data for fwa id:%d in database, "
                    "improper id",
                    nFwaId );
            continue;
        }
        sqlite3_reset( dlg_stmt );

        fwas.push_back( CFWA( std::string( pszFwaName ),
                              std::string( pszFmgName ), nWalkIn, nPumpRoll,
                              nHead, nTail, nParallel, nAttDist, bWaterDrops,
                              bExcluded, dfDiscSize, nEslTime, nEslSize,
                              dfAirGround, anWalkDelay, anPostUsed,
                              anPostUnused, anPostEscape, anReload,
                              nFirstDelay, adfDiurnal, aoRos, adfRosCoeff,
                              nFwaIndex, logic[nCounter],
                              std::string( pszFpuName ) ) );
        nFwaIndex++;
        sqlite3_reset( fpu_stmt );
        sqlite3_reset( rosadj_stmt );
        if( pfnProgress )
        {
            pfnProgress( -1, "Loading Dispatch Logic...", NULL );
        }
    }
    sqlite3_finalize( fwa_stmt );
    sqlite3_finalize( fpu_stmt );
    sqlite3_finalize( walk_stmt );
    sqlite3_finalize( used_stmt );
    sqlite3_finalize( unused_stmt );
    sqlite3_finalize( escape_stmt );
    sqlite3_finalize( reload_stmt );
    sqlite3_finalize( diurnal_stmt );
    sqlite3_finalize( rosadj_stmt );
    sqlite3_finalize( rosfuel_stmt );
    sqlite3_finalize( dlg_stmt );
    fwa_stmt = fpu_stmt = walk_stmt = used_stmt = unused_stmt = NULL;
    escape_stmt = reload_stmt = diurnal_stmt = rosadj_stmt = NULL;
    rosfuel_stmt = dlg_stmt = NULL;
    sqlite3_exec( db, "END TRANSACTION", NULL, NULL, NULL );
    return 0;
}

IRS_Err SpatialiteDataAccess::LoadDispatchLogic( std::vector<CDispLogic>&logic,
                                                 IRSProgress pfnProgress,
                                                 const char * const *papszOptions )
{
    /* Declare all sql stmts */
    sqlite3_stmt *fwa_stmt;
    sqlite3_stmt *dlg_stmt;
    sqlite3_stmt *ind_stmt;
    sqlite3_stmt *bkp_stmt;
    sqlite3_stmt *res_stmt;
    sqlite3_exec( db, "BEGIN TRANSACTION", NULL, NULL, NULL );
    /* Prepare */
    /* Depends on our 'filter' */
    if( pszFilter != NULL )
    {
        sqlite3_prepare_v2( db, "SELECT DISTINCT(displog_id) FROM "
                                "fwa LEFT JOIN fwa_bndry ON "
                                "(fwa.name=fwa_bndry.fwa_lndr_name) "
                                "WHERE ST_Intersects(fwa_bndry.geometry, "
                                "GeomFromText(?1)) AND "
                                "fwa_bndry.ROWID IN (SELECT pkid FROM "
                                "idx_fwa_bndry_geometry WHERE "
                                "xmin <= MbrMaxX(GeomFromText(?1)) AND "
                                "xmax >= MbrMinX(GeomFromText(?1)) AND "
                                "ymin <= MbrMaxY(GeomFromText(?1)) AND "
                                "ymax >= MbrMinY(GeomFromText(?1)))",
                            -1, &fwa_stmt, NULL );
        sqlite3_bind_text( fwa_stmt, 1, pszFilter, -1, NULL );
    }
    else if( pszFpuFilter != NULL )
    {
        sqlite3_prepare_v2( db, "SELECT DISTINCT displog_id FROM fwa "
                                "LEFT JOIN fpu ON fwa.fpu_id=fpu.id "
                                "WHERE fpu.name LIKE '%' || ? || '%'",
                            -1, &fwa_stmt, NULL );
        sqlite3_bind_text( fwa_stmt, 1, pszFpuFilter, -1, NULL );
    }
    else
    {
        sqlite3_prepare_v2( db, "SELECT DISTINCT displog_id from fwa",
                            -1, &fwa_stmt, NULL );
    }

    sqlite3_prepare_v2( db, "SELECT * FROM disp_logic WHERE id=?", -1,
            &dlg_stmt, NULL );
    /* Dispatch logic indice */
    sqlite3_prepare_v2( db, "SELECT name FROM indice WHERE value=?", -1,
            &ind_stmt, NULL );
    /* Dispatch logic break points */
    sqlite3_prepare_v2( db, "SELECT * FROM  brk_point WHERE displog_id=?", -1, 
            &bkp_stmt, NULL );
    /* Dispatch logic break points */
    sqlite3_prepare_v2( db, "SELECT * FROM  num_resc WHERE displog_id=?", -1,
            &res_stmt, NULL );
    /* Bind and go through all unique dispatch logics */
    int nCounter;
    int nDispLogId;
    const char *pszDlgName;
    int nIndice, nNumLevels;
    const char *pszIndice;
    int anPoints[5];
    int anResourceCount[13][5];
    while( sqlite3_step( fwa_stmt ) == SQLITE_ROW )
    {
        nDispLogId = sqlite3_column_int( fwa_stmt, 0 );

        /* Get the Dispatch logic data */
        sqlite3_bind_int( dlg_stmt, 1, nDispLogId );
        if( sqlite3_step( dlg_stmt ) != SQLITE_ROW )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not locate "
                //"dispatch logic id:%d in database, improper id",
                //nDispLogId );
            fprintf( stderr, "Could not locate "
                "dispatch logic id:%d in database, improper id",
                nDispLogId );
            continue;
        }
        pszDlgName = (const char*)sqlite3_column_text( dlg_stmt, 1 );
        /* index 2 is fpu id */
        nIndice = sqlite3_column_int( dlg_stmt, 3 );
        sqlite3_bind_int( ind_stmt, 1, nIndice );
        if( sqlite3_step( ind_stmt ) != SQLITE_ROW )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not locate indice "
                    //"for dispatch logic id:%d", nDispLogId );
            fprintf( stderr, "Could not locate indice "
                    "for dispatch logic id:%d", nDispLogId );
            continue;
        }
        pszIndice = (const char*)sqlite3_column_text( ind_stmt, 0 );
        nNumLevels = sqlite3_column_int( dlg_stmt, 4 );

        /* Get the Break point logic data */
        sqlite3_bind_int( bkp_stmt, 1, nDispLogId );
        if( sqlite3_step( bkp_stmt ) != SQLITE_ROW )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not locate "
                    //"break point info for dl id:%d in database, improper id",
                    //nDispLogId );
            fprintf( stderr, "Could not locate "
                    "break point info for dl id:%d in database, improper id",
                    nDispLogId );
             sqlite3_reset( bkp_stmt );
            continue;
        }
        for( nCounter = 0; nCounter < 5; nCounter++ )
        {
            anPoints[nCounter] = sqlite3_column_int( bkp_stmt, nCounter+1 );
            /* Get Number of resources for that point */
            sqlite3_bind_int( res_stmt, 1, nDispLogId );
            /* Step through breaks, 1-5 */
            int i = 0;
            while( sqlite3_step( res_stmt ) == SQLITE_ROW )
            {
                /* Don't forget to offset indexes */
                for( int nRescCount = 0; nRescCount < 13; nRescCount++ )
                {
                    anResourceCount[nRescCount][i] =
                        sqlite3_column_int( res_stmt, nRescCount + 2 );
                }
                i++;
            }
            sqlite3_reset( res_stmt );
        }
        logic.push_back( CDispLogic( std::string( pszDlgName ),
                                     std::string( pszIndice ), nNumLevels,
                                     anPoints, anResourceCount ) );
        sqlite3_reset(dlg_stmt);
        sqlite3_reset(ind_stmt);
        sqlite3_reset(bkp_stmt);
        if( pfnProgress )
        {
            pfnProgress( -1, "Loading Dispatch Logic...", NULL );
        }
    }
    sqlite3_finalize(fwa_stmt);
    sqlite3_finalize(dlg_stmt);
    sqlite3_finalize(ind_stmt);
    sqlite3_finalize(bkp_stmt);
    sqlite3_finalize(res_stmt);
    fwa_stmt = dlg_stmt = ind_stmt = bkp_stmt = res_stmt = NULL;
    sqlite3_exec( db, "END TRANSACTION", NULL, NULL, NULL );
    return 0;
}
CFWA* SpatialiteDataAccess::LoadFwaByName( const char *pszFwaName )
{
    int nFwaId = GetFwaId( pszFwaName );

    return NULL;
}
IRS_Err SpatialiteDataAccess::LoadRescType( std::vector<CRescType>&types,
                                            const char * const *papszOptions )
{
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2( db, "SELECT * FROM resc_type", -1, &stmt,
                        NULL );

    const char *pszName;
    int nSpeed, nDispDelay, nRespDelay, nSetupTime;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        /* idx 0 is pk id */
        pszName = (const char*)sqlite3_column_text( stmt, 1 );
        nSpeed = sqlite3_column_int( stmt, 2 );
        nDispDelay = sqlite3_column_int( stmt, 3 );
        nRespDelay = sqlite3_column_int( stmt, 4 );
        nSetupTime = sqlite3_column_int( stmt, 5 );
        types.push_back( CRescType( pszName, nSpeed, nDispDelay, nRespDelay,
                                    nSetupTime ) );
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return 0;
}

IRS_Err SpatialiteDataAccess::LoadProdRates( std::vector<CProdRates>&rates,
                                             const char * const *papszOptions )
{
    int rc;
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, "SELECT * FROM prod_rates", -1, &stmt,
                             NULL );

    const char *pszName, *pszSpecCond;
    int nSlope, nStaff, nFuel;
    double dfRate;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszName = (const char*)sqlite3_column_text( stmt, 0 );
        nSlope = sqlite3_column_int( stmt, 1 );
        nStaff = sqlite3_column_int( stmt, 2 );
        nFuel = sqlite3_column_int( stmt, 3 );
        pszSpecCond = (const char*)sqlite3_column_text( stmt, 4 );
        dfRate = sqlite3_column_double( stmt, 5 );
        rates.push_back( CProdRates( pszName, nSlope, nStaff, nFuel,
                         pszSpecCond, dfRate ) );
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return 0;
}
IRS_Err SpatialiteDataAccess::LoadDispatchLocation( std::vector<CFWA>&fwas,
                                                    std::vector<CDispLoc>&loc,
                                                    const char * const *papszOptions )
{
    int nFwaCount = fwas.size();
    int rc;

    sqlite3_stmt *fwa_stmt;
    sqlite3_stmt *ass_stmt;
    sqlite3_stmt *loc_stmt;
    sqlite3_stmt *dist_stmt;
    sqlite3_stmt *resc_stmt;
    const char *pszErr = sqlite3_errmsg(db);
    rc = sqlite3_exec( db, "BEGIN TRANSACTION", NULL, NULL, NULL );

    rc = sqlite3_prepare_v2( db, "SELECT id from fwa where name=?", -1, &fwa_stmt,
                        NULL );
    rc = sqlite3_prepare_v2( db, "SELECT * FROM disploc WHERE id=?1", -1,
                             &loc_stmt, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT distance from disploc_fwa where fwa_id=? "
                                 "AND disploc_id=?",
                             -1, &dist_stmt, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM resources WHERE "
                                 "disp_loc_id=?",
                             -1, &resc_stmt, NULL );

    /* Build a set of fwa ids to query against */
    int *panFwaIds = (int*)malloc( nFwaCount * sizeof( int ) );
    char *abyFwaSet = (char*)malloc( nFwaCount * 10 );
    abyFwaSet[0] = '\0';
    int nFwaId;
    for( int i = 0; i < nFwaCount; i++ )
    {
        std::string oFwaName = fwas[i].GetFWAID();
        rc = sqlite3_bind_text( fwa_stmt, 1, oFwaName.c_str(), -1, SQLITE_TRANSIENT );
        rc = sqlite3_step( fwa_stmt );
        if( rc != SQLITE_ROW )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not load fwa id "
                        //"for: %s", oFwaName.c_str() );
            fprintf( stderr, "Could not load fwa id "
                        "for: %s", oFwaName.c_str() );
            sqlite3_reset( fwa_stmt );
            continue;
        }
        else
        {
            panFwaIds[i] = sqlite3_column_int( fwa_stmt, 0 );
            sprintf( abyFwaSet + strlen( abyFwaSet ), " %d", panFwaIds[i] );
            if( i != nFwaCount - 1 )
                sprintf( abyFwaSet + strlen( abyFwaSet ), "," );
            sqlite3_reset( fwa_stmt );
        }
    }
    char *abySql = (char*)malloc( ( nFwaCount * 10 ) +
            strlen("SELECT DISTINCT(disploc_id) FROM disploc_fwa WHERE fwa_id IN " ) );
    sprintf( abySql, "SELECT DISTINCT(disploc_id) FROM disploc_fwa "
                     "WHERE fwa_id IN (%s)", abyFwaSet );
    rc = sqlite3_prepare_v2( db, abySql, -1, &ass_stmt, NULL );
    int nDispLocId;
    const char *pszDispLocName;
    int nCallback;
    int nDispType;
    std::string oDispLocName;
    double dfDist;
    int skipped = 0;
    while( sqlite3_step( ass_stmt ) == SQLITE_ROW )
    {
        nDispLocId = sqlite3_column_int( ass_stmt, 0 );
        rc = sqlite3_bind_int( resc_stmt, 1, nDispLocId );
        rc = sqlite3_step( resc_stmt );
        if( rc != SQLITE_ROW || sqlite3_column_int( resc_stmt, 0 ) < 1)
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "No resources in dispatch "
                    //"location: %d, skipping.", nDispLocId );
            if(!skipped)
            {
                fprintf( stderr, "Some dispatch locations are skipped. One " \
                                 "or more contain no resources.\n" );
            }
            skipped++;
            sqlite3_reset( resc_stmt );
            sqlite3_reset( loc_stmt );
            continue;
        }
        rc = sqlite3_bind_int( loc_stmt, 1, nDispLocId );
        if( sqlite3_step( loc_stmt ) != SQLITE_ROW )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not find dispatch id "
            //        "from associations table for %d", nDispLocId );
            sqlite3_reset( resc_stmt );
            sqlite3_reset( loc_stmt );
            continue;
        }
        oDispLocName = std::string((const char*)sqlite3_column_text( loc_stmt, 1 ) );
        nCallback = sqlite3_column_int( loc_stmt, 3 );
        double dfLon, dfLat;
        dfLon = sqlite3_column_double( loc_stmt, 5 );
        dfLat = sqlite3_column_double( loc_stmt, 6 );
        nDispType = sqlite3_column_int( loc_stmt, 7 );
        std::string oFpu = GetDispLocFPU( oDispLocName.c_str() );
        if( oFpu == "" )
        {
            if( oDispLocName != "Large_Airtankers" )
            {
                //CPLError( CE_Warning, CPLE_AppDefined, "Could not find fpu for "
                   //"dispatch location: %s", oDispLocName.c_str() );
                //fprintf( stderr, "Could not find fpu for "
                //   "dispatch location: %s", oDispLocName.c_str() );
            }
            sqlite3_reset( resc_stmt );
            sqlite3_reset( loc_stmt );
            continue;
        }

        /* This is hard coded for now, should be handled better */
        if( nDispType == 1 )
        {
            oFpu = "NA_US";
        }
        else if( nDispType == 2 )
        {
            oFpu = oFpu.substr(0, 2);
        }

        CDispLoc oDispLoc( oDispLocName, nCallback, oFpu, dfLat, dfLon );
        for( int i = 0;i < nFwaCount; i++ )
        {
            rc = sqlite3_bind_int( dist_stmt, 1, panFwaIds[i] );
            rc = sqlite3_bind_int( dist_stmt, 2, nDispLocId );
            rc = sqlite3_step( dist_stmt );
            if( rc != SQLITE_ROW )
            {
                sqlite3_reset( dist_stmt );
                continue;
            }
            else
            {
                dfDist = sqlite3_column_double( dist_stmt, 0 );
            }
            fwas[i].AddAssociation( oDispLocName, dfDist );
            oDispLoc.AddAssocFWA( &fwas[i] );
            sqlite3_reset( dist_stmt );
        }

        loc.push_back( oDispLoc );

        sqlite3_reset( loc_stmt );
        sqlite3_reset( resc_stmt );
    }
    fprintf( stderr, "%d dispatch locations skipped\n", skipped );
    sqlite3_reset( ass_stmt );

    free( panFwaIds );
    free( abyFwaSet );
    free( abySql );
    sqlite3_finalize( fwa_stmt );
    sqlite3_finalize( ass_stmt );
    sqlite3_finalize( loc_stmt );
    sqlite3_finalize( dist_stmt );
    sqlite3_finalize( resc_stmt );
    fwa_stmt = ass_stmt = loc_stmt = dist_stmt = NULL;
    sqlite3_exec( db, "END", NULL, NULL, NULL );
    return 0;
}

/**
 * \brief Load all tanker bases within a given distance of the fwas
 *
 * If no distance is supplied, load all large air tanker bases.
 *
 * \param fwas List of fwas in the simulation
 * \param loc locations to append air tankers to.
 * \param dfDistance buffer distance to limit loading bases.  A negative value
 *        causes all to be loaded.
 * \param papszOptions name/value options
 * \return 0 on success, non-zero otherwise
 */
IRS_Err SpatialiteDataAccess::LoadTankerBases( CDataMaster *poScenario,
                                               std::vector<CFWA>&fwas,
                                               std::vector<CDispLoc>&loc,
                                               double dfMaxDistance,
                                               const char * const *papszOptions )
 {
    int nFwaCount =  fwas.size();
    int rc;
    sqlite3_stmt *fwa_stmt;
    sqlite3_stmt *att_stmt;
    sqlite3_stmt *ass_stmt;
    sqlite3_prepare_v2( db, "SELECT id FROM fwa WHERE name=?", -1,
                        &fwa_stmt, NULL);
    sqlite3_prepare_v2( db, "SELECT OGC_FID, name, X(geometry), Y(geometry) "
                            "FROM tanker_base", -1, &att_stmt, NULL );
    sqlite3_prepare_v2( db, "SELECT CvtToMi(distance) FROM att_assoc WHERE "
                            "base_ogc_id=? AND fwa_id=?", -1,
                            &ass_stmt, NULL );
    const char *pszBaseName;
    int nBaseId;
    double dfX, dfY;
    int nFwaId;
    double dfDist;
    vector<double> oDist;
    const char *pszFwaName;
    sqlite3_exec( db, "BEGIN TRANSACTION", NULL, NULL, NULL );
    while( sqlite3_step( att_stmt ) == SQLITE_ROW )
    {
        nBaseId = sqlite3_column_int( att_stmt, 0 );
        pszBaseName = (const char*)sqlite3_column_text( att_stmt, 1 );
        dfX = sqlite3_column_double( att_stmt, 2 );
        dfY = sqlite3_column_double( att_stmt, 3 );
        oDist.clear();
        CDispLoc oLoc = CDispLoc( pszBaseName, 0, "NA_US", dfY, dfX );
        for( int i = 0;i < nFwaCount; i++ )
        {
            pszFwaName = strdup( fwas[i].GetFWAID().c_str() );
            sqlite3_bind_text( fwa_stmt, 1, pszFwaName, -1,
                                NULL );
            if( sqlite3_step( fwa_stmt ) != SQLITE_ROW )
            {
                //CPLError( CE_Warning, CPLE_AppDefined, "Could not find fwa "
                        //"for dispatch location: %s", pszBaseName );
                fprintf( stderr, "Could not find fwa "
                        "for dispatch location: %s", pszBaseName );
                sqlite3_reset( fwa_stmt );
                free( (void*)pszFwaName );
                continue;
            }
            nFwaId = sqlite3_column_int( fwa_stmt, 0 );
            sqlite3_bind_int( ass_stmt, 1, nBaseId );
            sqlite3_bind_int( ass_stmt, 2, nFwaId );
            rc = sqlite3_step( ass_stmt );
            if( rc != SQLITE_ROW )
            {
                //CPLError( CE_Warning, CPLE_AppDefined, "Could not find "
                //          "distance association for dispatch location: %s "
                //          "and fwa: %s", pszBaseName,
                //          fwas[i].GetFWAID().c_str() );
                free( (void*)pszFwaName );
                sqlite3_reset( fwa_stmt );
                sqlite3_reset( ass_stmt );
                continue;
            }
            dfDist = sqlite3_column_double( ass_stmt, 0 );
            fwas[i].AddAssociation( pszBaseName, dfDist );
            oLoc.AddAssocFWA( &fwas[i] );
            free( (void*)pszFwaName );
            sqlite3_reset( fwa_stmt );
            sqlite3_reset( ass_stmt );
        }
        loc.push_back( oLoc );
        poScenario->m_AirtankerDLs.push_back( pszBaseName );
    }
    sqlite3_finalize( fwa_stmt );
    sqlite3_finalize( ass_stmt );
    sqlite3_finalize( att_stmt );
    fwa_stmt = ass_stmt = att_stmt = NULL;
    sqlite3_exec( db, "END TRANSACTION", NULL, NULL, NULL );

    return 0;
}


std::string SpatialiteDataAccess::GetDispLocFPU( const char *pszName )
{
    int rc;
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, "SELECT fpu_code FROM fpu_bndry, dispatch_location "
                                 "WHERE dispatch_location.name=?"
                                 "AND ST_Contains(fpu_bndry.geometry, "
                                 "                dispatch_location.geometry)", -1,
                             &stmt, NULL );

    rc = sqlite3_bind_text( stmt, 1, pszName, -1, NULL );
    if( sqlite3_step( stmt ) != SQLITE_ROW )
    {
        //CPLError( CE_Warning, CPLE_AppDefined, "Could not find fpu for "
                //"dispatch location: %s", pszName );
        fprintf( stderr, "Could not find fpu for "
                "dispatch location: %s", pszName );
        return std::string();
    }
    std::string oFpu;
    oFpu = (const char*)sqlite3_column_text( stmt, 0 );
    sqlite3_finalize( stmt );
    stmt = NULL;
    return oFpu;
}

IRS_Err SpatialiteDataAccess::LoadResource( CDataMaster* scenario,
                                            std::vector<CRescType>&types,
                                            std::vector<CDispLoc>&loc,
                                            std::vector<CResource*>&resc,
                                            const char *pszExternalDb,
                                            const char *pszExcludeAgency,
                                            const char *pszOmitResourceDb,
                                            const char *pszOmitSet,
                                            const char * const *papszOptions )//,
                                            //int(*pfnProgress)(double, const char*, void))
{
    int rc;
    sqlite3_stmt *disp_stmt;
    sqlite3_stmt *omit_stmt;
    sqlite3_stmt *resc_stmt;
    sqlite3_stmt *count_stmt;
    sqlite3_stmt *heli_ass_stmt;
    sqlite3 *resc_db = NULL;
    char *pszSql = NULL;
    if( pszExternalDb != NULL )
    {
        rc = sqlite3_open_v2 ( pszExternalDb, &resc_db, SQLITE_OPEN_READONLY,
                               NULL);
        if( rc != SQLITE_OK )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not open external " \
                                                   //"resource database, using " \
                                                   //"internal resources" );
            fprintf( stderr, "Could not open external " \
                                                   "resource database, using " \
                                                   "internal resources" );
            sqlite3_close( resc_db );
            resc_db = db;
        }
    }
    else
    {
        resc_db = db;
    }
    int bReadOmit = FALSE;
    if( pszOmitResourceDb != NULL )
    {
        sqlite3_stmt *omit_stmt;
        rc = sqlite3_prepare_v2( db, "ATTACH ? AS omit", -1, &omit_stmt,
                                 NULL );
        rc = sqlite3_bind_text( omit_stmt, 1, pszOmitResourceDb, -1, NULL );
        rc = sqlite3_step( omit_stmt );
        if( rc == SQLITE_ROW || rc == SQLITE_DONE || rc == SQLITE_OK )
            bReadOmit = TRUE;
        sqlite3_finalize( omit_stmt );
        rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM omit.sqlite_master " \
                                     "WHERE name='resc_remove' AND type='table'",
                                 -1, &omit_stmt, NULL );
        rc = sqlite3_step( omit_stmt );
        if( rc != SQLITE_ROW )
            bReadOmit = FALSE;
        else
        {
            rc = sqlite3_column_int( omit_stmt, 0 );
            if( rc != 1 )
                bReadOmit = FALSE;
        }
    }

    rc = sqlite3_prepare_v2( resc_db, "SELECT id FROM disploc WHERE name=?", -1,
                             &disp_stmt, NULL );

    if( bReadOmit )
    {
        rc = sqlite3_prepare_v2( resc_db, "SELECT * FROM resources " \
                                          "WHERE disp_loc_id=? AND " \
                                          "resources.id NOT IN " \
                                          "(SELECT resc_id FROM " \
                                          "omit.resc_remove)", -1,
                                 &resc_stmt, NULL );
    }
    else if( pszOmitSet )
    {
        pszSql = sqlite3_mprintf( "SELECT * FROM resources " \
                                  "WHERE disp_loc_id=? AND " \
                                  "resources.id NOT IN (%s)",
                                  pszOmitSet);
        rc = sqlite3_prepare_v2( resc_db, pszSql, -1, &resc_stmt, NULL );
    }
    else
    {
        rc = sqlite3_prepare_v2( resc_db, "SELECT * FROM resources " \
                                          "WHERE disp_loc_id=?", -1,
                                 &resc_stmt, NULL );
    }

    int nDispLocId;
    int nRescDbId;
    const char *pszRescName;
    const char *pszRescType;
    int nStaffing;
    const char *pszStartTime;
    const char *pszEndTime;
    const char *pszStartDay;
    const char *pszEndDay;
    int nStartSeason;
    int nEndSeason;
    const char *pszDispLocName;
    int nPercAvail;
    double dfDayCost;
    double dfHourCost;
    int nVolume;
    int bSeverity;
    const char *pszVersion;
    double dfFte;
    double dfNumPositions;
    double dfAnnualCost;
    double dfVehicleCost;
    /*
    ** XXX: FIXME: Not sure about this little guy.
    */
    std::multimap<std::string, CResource*>resc_map;

    int nDone = 0;

    for( int i = 0;i < loc.size();i++ )
    {
        pszDispLocName = strdup(loc[i].GetDispLocID().c_str());
        rc = sqlite3_bind_text( disp_stmt, 1, pszDispLocName, -1,
                                NULL );
        rc = sqlite3_step( disp_stmt );
        nDispLocId = sqlite3_column_int( disp_stmt, 0 );

        rc = sqlite3_bind_int( resc_stmt, 1, nDispLocId );
        while( sqlite3_step( resc_stmt ) == SQLITE_ROW )
        {
            nRescDbId = sqlite3_column_int( resc_stmt, 0 );
#ifdef IRS_USE_DBID_AS_RESC_NAME
            pszRescName = (const char*)sqlite3_column_text( resc_stmt, 0 );
#else
            pszRescName = (const char*)sqlite3_column_text( resc_stmt, 1 );
#endif
            pszRescType = (const char*)sqlite3_column_text( resc_stmt, 2 );
            nStaffing = sqlite3_column_int( resc_stmt, 3 );
            pszStartTime = (const char*)sqlite3_column_text( resc_stmt, 4 );
            pszEndTime = (const char*)sqlite3_column_text( resc_stmt, 5 );
            pszStartDay = DayOfWeek( sqlite3_column_int( resc_stmt, 6 ) );
            pszEndDay = DayOfWeek( sqlite3_column_int( resc_stmt, 7 ) );
            nStartSeason = sqlite3_column_int( resc_stmt, 8 );
            nEndSeason = sqlite3_column_int( resc_stmt, 9 );
            nPercAvail = sqlite3_column_int( resc_stmt, 11 );
            dfDayCost = sqlite3_column_double( resc_stmt, 12 );
            dfHourCost = sqlite3_column_double( resc_stmt, 13 );
            nVolume = sqlite3_column_int( resc_stmt, 14 );
            bSeverity = sqlite3_column_int( resc_stmt, 15 );
            pszVersion = (const char*)sqlite3_column_text( resc_stmt, 16 );
            dfFte = sqlite3_column_double( resc_stmt, 17 );
            dfNumPositions = sqlite3_column_double( resc_stmt, 18 );
            dfAnnualCost = sqlite3_column_double( resc_stmt, 19 );
            dfVehicleCost = sqlite3_column_double( resc_stmt, 20 );

            /* We need a resc type object for the resource constructor */
            int j = 0;
            while( j < types.size() )
            {
                if( std::string( pszRescType ) == types[j].GetRescType() )
                    break;
                j++;
            }
            if( j == types.size() )
            {
                //CPLError( CE_Warning, CPLE_AppDefined, "Could not associate "
                        //"resource with a type" );
                fprintf( stderr, "Could not associate "
                        "resource with a type" );
                sqlite3_reset( resc_stmt );
                sqlite3_reset( disp_stmt );
                free((void*)pszDispLocName);
                continue;
            }

            /* We don't need to do this, we know our dispatch location, i. */
            /*
            int k = 0;
            while( k < loc.size() )
            {
                if( std::string( pszDispLocName ) == loc[k].GetDispLocID() )
                    break;
                k++;
            }

            if( k == loc.size() )
            {
                CPLError( CE_Warning, CPLE_AppDefined, "Could not associate "
                        "resource with a dispatch location" );
                continue;
            }
            */

            // Construct CResource object of the appropriate type
            if( EQUALN( pszRescType, "DZR", 3 ) ||
                EQUALN( pszRescType, "TP", 2 ) )
            {
                resc.push_back( new CConstProd( pszRescName,
                                                types[j],
                                                nStaffing, pszStartTime,
                                                pszEndTime, pszStartDay, 
                                                pszEndDay, nStartSeason,
                                                nEndSeason, loc[i],
                                                nPercAvail, dfDayCost,
                                                dfHourCost ) );

            }
            else if( EQUAL( pszRescType, "CRW" ) )
            {
                resc.push_back( new CCrew( pszRescName,
                                           types[j], nStaffing,
                                           pszStartTime, pszEndTime, pszStartDay,
                                           pszEndDay, nStartSeason,
                                           nEndSeason, loc[i], nPercAvail,
                                           dfDayCost, dfHourCost ) );
            }
            else if( EQUALN( pszRescType, "EN", 2 ) )
            {
                resc.push_back( new CEngine( pszRescName,
                                             types[j], nStaffing,
                                             pszStartTime, pszEndTime, 
                                             pszStartDay, pszEndDay, 
                                             nStartSeason, nEndSeason, 
                                             loc[i], nPercAvail, dfDayCost,
                                             dfHourCost, nVolume ) );
            }
            else if( EQUAL( pszRescType, "WT" ) )
            {
                 resc.push_back( new CWaterTender( pszRescName,
                                                   types[j],
                                                   nStaffing, pszStartTime,
                                                   pszEndTime, pszStartDay,
                                                   pszEndDay, nStartSeason,
                                                   nEndSeason, loc[i],
                                                   nPercAvail, dfDayCost,
                                                   dfHourCost ) );
            }
            else if( EQUAL( pszRescType, "ATT" ) )
            {
                resc.push_back( new CAirtanker( pszRescName,
                                                types[j],
                                                nStaffing, pszStartTime,
                                                pszEndTime, pszStartDay,
                                                pszEndDay, nStartSeason,
                                                nEndSeason, loc[i],
                                                nPercAvail, dfDayCost,
                                                dfHourCost, nVolume ) );
            }
            else if( EQUAL( pszRescType, "SEAT" ) || EQUAL( pszRescType, "SCP" ) )
            {
                resc.push_back( new CSmallAT( pszRescName,
                                              types[j], 
                                              nStaffing, pszStartTime,
                                              pszEndTime, pszStartDay, pszEndDay,
                                              nStartSeason, nEndSeason,
                                              loc[i], nPercAvail, dfDayCost,
                                              dfHourCost, nVolume ) );
            }
            else if( EQUAL( pszRescType, "SMJR" ) )
            {
                resc.push_back( new CSmokejumper( pszRescName,
                                                  types[j],
                                                  nStaffing, pszStartTime,
                                                  pszEndTime, pszStartDay,
                                                  pszEndDay, nStartSeason,
                                                  nEndSeason, loc[i],
                                                  nPercAvail, dfDayCost,
                                                  dfHourCost ) ); 
            }
            else if( EQUAL( pszRescType, "SJAC" ) )
            {
                 resc.push_back( new CSMJAircraft( pszRescName,
                                                   types[j],
                                                   nStaffing, pszStartTime,
                                                   pszEndTime, pszStartDay,
                                                   pszEndDay, nStartSeason,
                                                   nEndSeason, loc[i],
                                                   nPercAvail, dfDayCost,
                                                   dfHourCost, nVolume ) );
            }
            else if( EQUAL( pszRescType, "HELI" ) )
            {
                resc.push_back( new CHelitack( pszRescName,
                                               types[j],
                                               nStaffing, pszStartTime,
                                               pszEndTime, pszStartDay,
                                               pszEndDay, nStartSeason,
                                               nEndSeason, loc[i],
                                               nPercAvail, dfDayCost,
                                               dfHourCost ) );
            }
            else if( EQUALN( pszRescType, "HEL", 3 ) )
            {
                resc.push_back( new CHelicopter( pszRescName,
                                                 types[j],
                                                 nStaffing, pszStartTime,
                                                 pszEndTime, pszStartDay,
                                                 pszEndDay, nStartSeason,
                                                 nEndSeason, loc[i],
                                                 nPercAvail, dfDayCost,
                                                 dfHourCost, nVolume ) );
                resc_map.insert( std::pair<string, CResource*>( pszRescName,
                                                                resc[resc.size()-1] ) );
             }
            resc[resc.size()-1]->SetDbId( nRescDbId );
        }
        sqlite3_reset( resc_stmt );
        sqlite3_reset( disp_stmt );
        free((void*)pszDispLocName);
    }
    AssociateHelitack( resc_map );
    /*if( bReadOmit )
    {
        sqlite3_exec( resc_db, "DETACH omit", NULL, NULL, NULL );
    }*/
    sqlite3_finalize( disp_stmt );
    sqlite3_finalize( resc_stmt );
    disp_stmt = resc_stmt = NULL;

    //Load for resource type/agency dispatch location vectors
    sqlite3_stmt *stmt = NULL;

    rc = sqlite3_prepare_v2( resc_db, "SELECT DISTINCT disploc.name FROM resources " \
                                          "JOIN disploc ON resources.disp_loc_id = " \
                                          "disploc.id WHERE agency IN (?1, ?2, ?3, ?4) AND resc_type IN (?5, ?6, ?7, ?8)", -1,
                                          &stmt, NULL );

    if (rc != SQLITE_OK) return 1;

#ifndef IRS_USE_DIANE_FILES
    std::string dl;

    sqlite3_bind_text(stmt, 1, "BIA", -1, NULL);
    sqlite3_bind_text(stmt, 2, "BLM", -1, NULL);
    sqlite3_bind_text(stmt, 3, "FWS", -1, NULL);
    sqlite3_bind_text(stmt, 4, "NPS", -1, NULL);
    sqlite3_bind_text(stmt, 5, "CRW", -1, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW)    {
        dl = (const char*)sqlite3_column_text(stmt, 0);
        for (int i = 0; i < loc.size(); i++)    {
            if (dl == loc[i].GetDispLocID())
                scenario->m_DOICRWDLs.push_back(dl);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);

    sqlite3_bind_text(stmt, 1, "BIA", -1, NULL);
    sqlite3_bind_text(stmt, 2, "BLM", -1, NULL);
    sqlite3_bind_text(stmt, 3, "FWS", -1, NULL);
    sqlite3_bind_text(stmt, 4, "NPS", -1, NULL);
    sqlite3_bind_text(stmt, 5, "DZR1", -1, NULL);
    sqlite3_bind_text(stmt, 6, "DZR2", -1, NULL);
    sqlite3_bind_text(stmt, 7, "DZR3", -1, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW)    {
        dl = (const char*)sqlite3_column_text(stmt, 0);
        for (int i = 0; i < loc.size(); i++)    {
            if (dl == loc[i].GetDispLocID())
                scenario->m_DOIDZRDLs.push_back(dl);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);

    sqlite3_bind_text(stmt, 1, "BIA", -1, NULL);
    sqlite3_bind_text(stmt, 2, "BLM", -1, NULL);
    sqlite3_bind_text(stmt, 3, "FWS", -1, NULL);
    sqlite3_bind_text(stmt, 4, "NPS", -1, NULL);
    sqlite3_bind_text(stmt, 5, "EN12", -1, NULL);
    sqlite3_bind_text(stmt, 6, "EN34", -1, NULL);
    sqlite3_bind_text(stmt, 7, "EN56", -1, NULL);
    sqlite3_bind_text(stmt, 8, "EN70", -1, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW)    {
        dl = (const char*)sqlite3_column_text(stmt, 0);
        for (int i = 0; i < loc.size(); i++)    {
            if (dl == loc[i].GetDispLocID())
                scenario->m_DOIENGDLs.push_back(dl);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);

    sqlite3_bind_text(stmt, 1, "BIA", -1, NULL);
    sqlite3_bind_text(stmt, 2, "BLM", -1, NULL);
    sqlite3_bind_text(stmt, 3, "FWS", -1, NULL);
    sqlite3_bind_text(stmt, 4, "NPS", -1, NULL);
    sqlite3_bind_text(stmt, 5, "HEL2", -1, NULL);
    sqlite3_bind_text(stmt, 6, "HEL3", -1, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW)    {
        dl = (const char*)sqlite3_column_text(stmt, 0);
        for (int i = 0; i < loc.size(); i++)    {
            if (dl == loc[i].GetDispLocID())
                scenario->m_DOIHelDLs.push_back(dl);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);

    sqlite3_bind_text(stmt, 1, "BIA", -1, NULL);
    sqlite3_bind_text(stmt, 2, "BLM", -1, NULL);
    sqlite3_bind_text(stmt, 3, "FWS", -1, NULL);
    sqlite3_bind_text(stmt, 4, "NPS", -1, NULL);
    sqlite3_bind_text(stmt, 5, "HELI", -1, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW)    {
        dl = (const char*)sqlite3_column_text(stmt, 0);
        for (int i = 0; i < loc.size(); i++)    {
            if (dl == loc[i].GetDispLocID())
                scenario->m_DOIHELIDLs.push_back(dl);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);

    sqlite3_bind_text(stmt, 1, "BIA", -1, NULL);
    sqlite3_bind_text(stmt, 2, "BLM", -1, NULL);
    sqlite3_bind_text(stmt, 3, "FWS", -1, NULL);
    sqlite3_bind_text(stmt, 4, "NPS", -1, NULL);
    sqlite3_bind_text(stmt, 5, "SEAT", -1, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW)    {
        dl = (const char*)sqlite3_column_text(stmt, 0);
        for (int i = 0; i < loc.size(); i++)    {
            if (dl == loc[i].GetDispLocID())
                scenario->m_DOISEATDLs.push_back(dl);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);

    sqlite3_bind_text(stmt, 1, "FS", -1, NULL);
    sqlite3_bind_text(stmt, 5, "CRW", -1, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW)    {
        dl = (const char*)sqlite3_column_text(stmt, 0);
        for (int i = 0; i < loc.size(); i++)    {
            if (dl == loc[i].GetDispLocID())
                scenario->m_FSCRWDLs.push_back(dl);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);

    sqlite3_bind_text(stmt, 1, "FS", -1, NULL);
    sqlite3_bind_text(stmt, 5, "DZR1", -1, NULL);
    sqlite3_bind_text(stmt, 6, "DZR2", -1, NULL);
    sqlite3_bind_text(stmt, 7, "DZR3", -1, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW)    {
        dl = (const char*)sqlite3_column_text(stmt, 0);
        for (int i = 0; i < loc.size(); i++)    {
            if (dl == loc[i].GetDispLocID())
                scenario->m_FSDZRDLs.push_back(dl);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);

    sqlite3_bind_text(stmt, 1, "FS", -1, NULL);
    sqlite3_bind_text(stmt, 5, "EN12", -1, NULL);
    sqlite3_bind_text(stmt, 6, "EN34", -1, NULL);
    sqlite3_bind_text(stmt, 7, "EN56", -1, NULL);
    sqlite3_bind_text(stmt, 8, "EN70", -1, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW)    {
        dl = (const char*)sqlite3_column_text(stmt, 0);
        for (int i = 0; i < loc.size(); i++)    {
            if (dl == loc[i].GetDispLocID())
                scenario->m_FSENGDLs.push_back(dl);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);

    sqlite3_bind_text(stmt, 1, "FS", -1, NULL);
    sqlite3_bind_text(stmt, 5, "HEL2", -1, NULL);
    sqlite3_bind_text(stmt, 6, "HEL3", -1, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW)    {
        dl = (const char*)sqlite3_column_text(stmt, 0);
        for (int i = 0; i < loc.size(); i++)    {
            if (dl == loc[i].GetDispLocID())
                scenario->m_FSHelDLs.push_back(dl);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);

    sqlite3_bind_text(stmt, 1, "FS", -1, NULL);
    sqlite3_bind_text(stmt, 5, "HELI", -1, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW)    {
        dl = (const char*)sqlite3_column_text(stmt, 0);
        for (int i = 0; i < loc.size(); i++)    {
            if (dl == loc[i].GetDispLocID())
                scenario->m_FSHELIDLs.push_back(dl);
        }
    }
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);

    sqlite3_bind_text(stmt, 1, "FS", -1, NULL);
    sqlite3_bind_text(stmt, 5, "SEAT", -1, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW)    {
        dl = (const char*)sqlite3_column_text(stmt, 0);
        for (int i = 0; i < loc.size(); i++)    {
            if (dl == loc[i].GetDispLocID())
                scenario->m_FSSEATDLs.push_back(dl);
        }
    }
    sqlite3_finalize(stmt);

    rc = sqlite3_prepare_v2( resc_db, "SELECT DISTINCT disploc.name FROM resources " \
                                          "JOIN disploc ON resources.disp_loc_id = " \
                                          "disploc.id WHERE resc_type = 'HEL1'", -1,
                                          &stmt, NULL );
    if (rc != SQLITE_OK) return 1;
     while (sqlite3_step(stmt) == SQLITE_ROW)    {
        dl = (const char*)sqlite3_column_text(stmt, 0);
        for (int i = 0; i < loc.size(); i++)    {
            if (dl == loc[i].GetDispLocID())
                scenario->m_RegionalHelicopterDLs.push_back(dl);
        }
    }
    sqlite3_finalize(stmt);

    rc = sqlite3_prepare_v2( resc_db, "SELECT DISTINCT disploc.name FROM resources " \
                                          "JOIN disploc ON resources.disp_loc_id = " \
                                          "disploc.id WHERE resc_type = 'SMJR'", -1,
                                          &stmt, NULL );
    if (rc != SQLITE_OK) return 1;
     while (sqlite3_step(stmt) == SQLITE_ROW)    {
        dl = (const char*)sqlite3_column_text(stmt, 0);
        for (int i = 0; i < loc.size(); i++)    {
            if (dl == loc[i].GetDispLocID())
                scenario->m_SmokejumperDLs.push_back(dl);
        }
    }
    sqlite3_finalize(stmt);

    rc = sqlite3_prepare_v2( resc_db, "SELECT DISTINCT disploc.name FROM resources " \
                                          "JOIN disploc ON resources.disp_loc_id = " \
                                          "disploc.id WHERE resc_type = 'CRW' AND staffing = 20 "   \
                                          "AND disploc.fpu_id = 'NA_US'", -1,
                                          &stmt, NULL );
    if (rc != SQLITE_OK) return 1;
     while (sqlite3_step(stmt) == SQLITE_ROW)    {
        dl = (const char*)sqlite3_column_text(stmt, 0);
        for (int i = 0; i < loc.size(); i++)    {
            if (dl == loc[i].GetDispLocID())
                scenario->m_RegionalCrewDLs.push_back(dl);
        }
    }
    sqlite3_finalize(stmt);
#endif /* IRS_USE_DIANE_FILES */

    if( bReadOmit )
    {
        sqlite3_exec( resc_db, "DETACH omit", NULL, NULL, NULL );
    }
    sqlite3_free( pszSql );

    return 0;
}

IRS_Err
SpatialiteDataAccess::AssociateHelitack( std::multimap<std::string, CResource*>&resc_map )
{
    sqlite3_stmt *stmt;
    int rc;
    rc = sqlite3_prepare_v2( db, "SELECT helitack FROM heli_assign WHERE hel=?",
                             -1, &stmt, NULL );
    std::string key;
    std::multimap<string, CResource*>::iterator it = resc_map.begin();
    std::multimap<string, CResource*>::iterator find;
    for( ; it != resc_map.end(); it++ )
    {
        if( EQUAL( it->second->GetRescType().GetRescType().c_str(), "HELI" ) )
        {
            continue;
        }

        sqlite3_bind_text( stmt, 1, it->first.c_str(), -1, SQLITE_TRANSIENT );
        while( sqlite3_step( stmt ) == SQLITE_ROW )
        {
            key = (const char*)sqlite3_column_text( stmt, 0 );
            find = resc_map.find( key );
            if( find == resc_map.end() )
            {
                continue;
            }
            CHelicopter* poHelic = dynamic_cast<CHelicopter*>( it->second );
            if( poHelic )
            {
                poHelic->AddAttachedCrew( find->second );
            }
        }
        sqlite3_reset( stmt );
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return 0;
}

IRS_Err SpatialiteDataAccess::LoadAlternateHelibases( std::map<std::string, OmffrNode<CDispatchBase*>*>&DLDispMap,
                                                      OmffrTree<CDispatchBase*>&DispTree )
{

    sqlite3_stmt *stmt;
    int rc;
    rc = sqlite3_prepare_v2( db, "SELECT assoc_base FROM hel_base WHERE base=?",
                             -1, &stmt, NULL );
    std::string key;
    std::map<string, OmffrNode<CDispatchBase*>*>::iterator it, find;
    it = DLDispMap.begin();
    for( ; it != DLDispMap.end(); it++ )
    {
        rc = sqlite3_bind_text( stmt, 1, it->first.c_str(), -1, SQLITE_TRANSIENT );
        while( sqlite3_step( stmt ) == SQLITE_ROW )
        {
            key = (const char*)sqlite3_column_text( stmt, 0 );
            find = DLDispMap.find( key );
            if( find == DLDispMap.end() )
            {
                continue;
            }
            CDLDispatcher* poBase = dynamic_cast<CDLDispatcher*>( DispTree.Element( it->second ) );
            CDLDispatcher* poAltBase = dynamic_cast<CDLDispatcher*>( DispTree.Element( find->second ) );
            if( poBase && poAltBase )
            {
                poBase->AddtoAltHelicopterDLsList( poAltBase );
            }
        }
        sqlite3_reset( stmt );
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return 0;
}

IRS_Err SpatialiteDataAccess::SetSpatialFilter( const char *pszWktGeometry )
{
    if( pszWktGeometry )
        pszFilter = strdup( pszWktGeometry );
    return 0;
}

const char * SpatialiteDataAccess::GetSpatialFilter()
{
    return pszFilter;
}

/**
 * \brief Get the fpu name for a point.
 *
 * \param x coordinate in NAD83, longitude
 * \param y coordinate in NAD83, latitude
 * \return empty string on failure, the fpu code on success
 */
std::string SpatialiteDataAccess::GetFpuName( double x, double y )
{
    sqlite3_stmt *stmt;
    int rc;
    rc = sqlite3_prepare_v2( db, "SELECT fpu_code from fpu_bndry WHERE "
                                 "ST_Contains(geometry, MakePoint(?1, ?2, 4269)) AND "
                                 "fpu_bndry.ROWID IN (SELECT pkid FROM "
                                 "idx_fpu_bndry_geometry WHERE "
                                 "?1 <= MbrMaxX(geometry) AND "
                                 "?1 >= MbrMinX(geometry) AND "
                                 "?2 <= MbrMaxY(geometry) AND "
                                 "?2 >= MbrMinY(geometry))",
                             -1, &stmt, NULL );
    rc = sqlite3_bind_double( stmt, 1, x );
    rc = sqlite3_bind_double( stmt, 2, y );
    rc = sqlite3_step( stmt );
    std::string oFpuName;

    if( rc != SQLITE_ROW )
        oFpuName = std::string();
    else
        oFpuName = ( (const char*)sqlite3_column_text( stmt, 0 ) );
    sqlite3_finalize( stmt );
    stmt = NULL;
    return oFpuName;
}

int SpatialiteDataAccess::GetFwaId( const char *pszFwaName )
{
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2( db, "SELECT id FROM fwa WHERE name=?",
                        -1, &stmt, NULL );
    sqlite3_bind_text( stmt, 1, pszFwaName, strlen(pszFwaName), NULL );
    int nFwaId;
    if( sqlite3_step( stmt ) != SQLITE_ROW )
        return nFwaId = 0;
    else
        nFwaId = sqlite3_column_int( stmt, 0 );
    sqlite3_finalize( stmt );
    stmt = NULL;
    return nFwaId;
}

std::map<std::string, double>SpatialiteDataAccess::GetFwaAssocDistance( const char *pszFwaName )
{
    int rc = 0;
    int nFwaId = GetFwaId( pszFwaName );
    sqlite3_stmt *stmt;
    std::map<std::string, double> map;
    rc = sqlite3_prepare_v2( db, "SELECT disploc.name, disploc_fwa.distance "
                                 "FROM disploc_fwa LEFT JOIN disploc "
                                 "ON disploc.id=disploc_fwa.disploc_id "
                                 "WHERE disploc_fwa.fwa_id=? "
                                 "ORDER BY disploc_fwa.distance", -1,
                             &stmt, NULL );
    rc = sqlite3_bind_int( stmt, 1, nFwaId );
    const char *pszDispName;
    double dfDistance;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszDispName = (const char *)sqlite3_column_text( stmt, 0 );
        if( pszDispName == NULL )
        {
            continue;
        }
        dfDistance = sqlite3_column_double( stmt, 1 );
        map.insert(std::pair<std::string, double>( std::string( pszDispName ),
                                                   dfDistance ) );
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return map;
}

IRS_Err SpatialiteDataAccess::AddFpuFilter( const char *pszFpuName )
{
    if( pszFpuName != NULL )
    {
        pszFpuFilter = strdup( pszFpuName );
        return 0;
    }
    else
        return 1;
}

const char * SpatialiteDataAccess::GetFpuFilter()
{
    return pszFpuFilter;
}

std::string SpatialiteDataAccess::PointQuery( const char *pszTableName,
                                              const char *pszAttrName,
                                              double dfX, double dfY )
{
    std::string oValue;
    char pszSql[8192];
    pszSql[0] = '\0';
    sprintf( pszSql, "SELECT %s FROM %s WHERE "
                     "ST_Contains(geometry, MakePoint(?1,?2,4269)) AND "
                     "%s.ROWID IN (SELECT pkid FROM "
                     "idx_%s_geometry WHERE "
                     "?1 <= MbrMaxX(geometry) AND "
                     "?1 >= MbrMinX(geometry) AND "
                     "?2 <= MbrMaxY(geometry) AND "
                     "?2 >= MbrMinY(geometry))",
             pszAttrName, pszTableName, pszTableName, pszTableName );
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2( db, pszSql, -1, &stmt, NULL );
    rc = sqlite3_bind_double( stmt, 1, dfX );
    rc = sqlite3_bind_double( stmt, 2, dfY );
    if( sqlite3_step( stmt ) != SQLITE_ROW )
    {
        //CPLError( CE_Warning, CPLE_AppDefined, "Could not locate attribute:%s "
                //"in table:%s using geometry query", pszAttrName, pszTableName );
        fprintf( stderr, "Could not locate attribute:%s "
                "in table:%s using geometry query", pszAttrName, pszTableName );
        oValue = "";
    }
    else
    {
        oValue = (const char*)sqlite3_column_text( stmt, 0 );
    }
    sqlite3_reset( stmt );
    sqlite3_finalize( stmt );
    stmt = NULL;
    return oValue;
}

std::vector<int>
SpatialiteDataAccess::GetAvgNumFires( const char *pszFpuName )
{
    int rc;
    sqlite3_stmt *year_stmt;
    sqlite3_stmt *fire_stmt;
    rc = sqlite3_prepare_v2( db, "SELECT COUNT(DISTINCT(year)) FROM scenario",
                             -1, &year_stmt, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT jul_day, COUNT(*) FROM scenario, "
                                 "fpu_bndry WHERE fpu_bndry.fpu_code=?1 AND "
                                 "ST_Contains(fpu_bndry.geometry, "
                                 "scenario.geometry) AND "
                                 "scenario.ROWID IN (SELECT pkid FROM "
                                 "idx_scenario_geometry, fpu_bndry WHERE "
                                 "fpu_bndry.fpu_code=?1 AND "
                                 "X(scenario.geometry) <= "
                                 "MBRMaxX(fpu_bndry.geometry) AND "
                                 "X(scenario.geometry) >= "
                                 "MBRMinX(fpu_bndry.geometry) AND "
                                 "Y(scenario.geometry) <= "
                                 "MBRMaxY(fpu_bndry.geometry) AND "
                                 "Y(scenario.geometry) >= "
                                 "MBRMinY(fpu_bndry.geometry)) "
                                 "GROUP BY jul_day",
                             -1, &fire_stmt, NULL );
    /*
    rc = sqlite3_prepare_v2( db, "SELECT jul_day, COUNT(*) FROM scenario, "
                                 "fpu_bndry WHERE fpu_bndry.fpu_code=? "
                                 "AND ST_Contains(fpu_bndry.geometry, "
                                 "scenario.geometry) GROUP BY jul_day "
                                 "ORDER BY jul_day",
                             -1, &fire_stmt, NULL );
    */
    rc = sqlite3_step( year_stmt );
    int nYears;
    double dfFires;
    std::vector<int>fire_count( 365 );
    fire_count.assign( 365, 0 );

    nYears = sqlite3_column_int( year_stmt, 0 );

    rc = sqlite3_bind_text( fire_stmt, 1, pszFpuName, -1, NULL );
    int nDay;
    while( sqlite3_step( fire_stmt ) == SQLITE_ROW )
    {
        nDay = sqlite3_column_int( fire_stmt, 0 );
        dfFires = sqlite3_column_int( fire_stmt, 1 );
        dfFires = ( dfFires + 0.5 ) / nYears;
        fire_count[nDay] = (int)dfFires;
    }
    sqlite3_reset( fire_stmt );
    sqlite3_finalize( year_stmt );
    sqlite3_finalize( fire_stmt );

    return fire_count;
}

std::pair<double, double>
SpatialiteDataAccess::GetTankerPercent( const char *pszTanker,
                                        const char *pszFpuName )
{
    int rc;
    sqlite3_stmt *att_stmt;
    sqlite3_stmt *fpu_stmt;
    sqlite3_stmt *att_perc_stmt;
    sqlite3_stmt *fpu_perc_stmt;
    rc = sqlite3_prepare_v2( db, "SELECT id FROM fpu WHERE name=?", -1, 
                             &fpu_stmt, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT ogc_fid FROM tanker_base "
                                 "WHERE name=?", -1, &fpu_stmt, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT att_use_perc from att_use WHERE "
                                 "id=?", -1, &fpu_perc_stmt, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT area_perc from base_area WHERE "
                                 "base_ogc_id=? and fpu_id=?", -1, 
                                 &att_perc_stmt, NULL );
    int nFpuId, nBaseId;
    double dfArea, dfUse;
    rc = sqlite3_bind_text( fpu_stmt, 1, pszFpuName, -1, NULL );
    if( sqlite3_step( fpu_stmt ) != SQLITE_ROW )
    {
        //CPLError( CE_Warning, CPLE_AppDefined, "Could not find fpu" );
        fprintf( stderr, "Could not find fpu" );
        return std::make_pair( 0, 0 );
    }
    nFpuId = sqlite3_column_int( fpu_stmt, 0 );
    rc = sqlite3_bind_text( att_stmt, 1, pszFpuName, -1, NULL );
    if( sqlite3_step( att_stmt ) != SQLITE_ROW )
    {
        //CPLError( CE_Warning, CPLE_AppDefined, "Could not find att base" );
        fprintf( stderr, "Could not find att base" );
        return std::make_pair( 0, 0 );
    }
    nBaseId = sqlite3_column_int( att_stmt, 0 );
    sqlite3_bind_int( fpu_perc_stmt, 1, nFpuId );
    if( sqlite3_step( fpu_stmt ) != SQLITE_ROW )
    {
        //CPLError( CE_Warning, CPLE_AppDefined, "Could not find percentage" );
        fprintf( stderr, "Could not find percentage" );
        return std::make_pair( 0, 0 );
    }
    dfUse = sqlite3_column_double( fpu_perc_stmt, 0 );

    sqlite3_bind_int( att_perc_stmt, 1, nBaseId );
    sqlite3_bind_int( att_perc_stmt, 2, nFpuId );
    if( sqlite3_step( att_stmt ) != SQLITE_ROW )
    {
        //CPLError( CE_Warning, CPLE_AppDefined, "Could not find percentage" );
        fprintf( stderr, "Could not find percentage" );
        return std::make_pair( 0, 0 );
    }
    dfArea = sqlite3_column_double( att_perc_stmt, 0 );
    sqlite3_finalize( att_stmt );
    sqlite3_finalize( att_perc_stmt );
    sqlite3_finalize( fpu_stmt );
    sqlite3_finalize( fpu_perc_stmt );
    return std::make_pair( dfUse, dfArea );
}

std::vector<int> SpatialiteDataAccess::GetNumFires( const char *pszFpuName,
                                                    int nYear )
{
    int rc;
    sqlite3_stmt *year_stmt;
    sqlite3_stmt *fire_stmt;
    /*
    ** FIXME: Add RTree
    */
    rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM scenario, fpu_bndry "
                                 "WHERE scenario.jul_day=? "
                                 "AND year=? "
                                 "AND fpu_bndry.fpu_code=? AND "
                                 "MBRContains(fpu_bndry.geometry, "
                                 "scenario.geometry) AND "
                                 "ST_Contains(fpu_bndry.geometry, "
                                 "scenario.geometry)",
                             -1, &fire_stmt, NULL );
    int nFires;
    std::vector<int>fire_count(365);
    for( int i = 0;i < 365; i++ )
    {
        rc = sqlite3_bind_int( fire_stmt, 1, i );
        rc = sqlite3_bind_int( fire_stmt, 2, nYear );
        rc = sqlite3_bind_text( fire_stmt, 3, pszFpuName, -1, NULL );
        if( sqlite3_step( fire_stmt ) != SQLITE_ROW )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not find fire count" );
            fprintf( stderr, "Could not find fire count" );
            sqlite3_reset( fire_stmt );
            continue;
        }
        nFires = sqlite3_column_int( fire_stmt, 0 );
        fire_count[i] = nFires;
        sqlite3_reset( fire_stmt );
    }
    sqlite3_finalize( year_stmt );
    sqlite3_finalize( fire_stmt );

    return fire_count;
}

IRS_Err
SpatialiteDataAccess::LoadGeneralResources( sqlite3 *rsc_db,
                                            const char * const *papszOptions )
{

    return 0;
}

/**
 * \brief Create and load a generic resource table.
 *
 * \param pszFilename Filename to write data to.
 * \param new_db sqlite database handle.
 * \return zero on success.
 */
IRS_Err
SpatialiteDataAccess::CreateGeneralResourceDb( const char *pszFilename,
                                               sqlite3 **new_db )
{
    int rc = 0;
    sqlite3_open_v2( pszFilename, new_db, 
                     SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL );

    sqlite3_stmt *create_stmt;
    sqlite3_stmt *select_stmt;
    sqlite3_stmt *insert_stmt;
    rc = sqlite3_exec(*new_db, "DROP TABLE IF EXISTS general_resc", NULL, NULL, NULL);
    rc = sqlite3_prepare_v2( *new_db, "CREATE TABLE general_resc(gacc text,"
                                                           "type text,"
                                                           "count integer,"
                                                           "fte real,"
                                                           "personnel integer)",
                        -1, &create_stmt, NULL );
    sqlite3_step( create_stmt );
    rc = sqlite3_prepare_v2( db, "SELECT * FROM general_resc", -1, &select_stmt,
                        NULL );
    rc = sqlite3_prepare_v2( *new_db, "INSERT INTO general_resc VALUES(?, ?, ?, ?, ?)", 
                        -1, &insert_stmt, NULL );
    rc = sqlite3_finalize( create_stmt );
    const char *pszRegion, *pszType;
    int nCount;
    double dfFte;
    int nPeople;
    sqlite3_exec(*new_db, "BEGIN TRANSACTION", NULL, NULL, NULL);
    while( sqlite3_step( select_stmt ) == SQLITE_ROW )
    {
        pszRegion = (const char*)sqlite3_column_text( select_stmt, 0 );
        pszType = (const char*)sqlite3_column_text( select_stmt, 1 );
        nCount = sqlite3_column_int( select_stmt, 2 );
        dfFte = sqlite3_column_double( select_stmt, 3 );
        nPeople = sqlite3_column_int( select_stmt, 4 );
        rc = sqlite3_bind_text( insert_stmt, 1, pszRegion, -1, NULL );
        rc = sqlite3_bind_text( insert_stmt, 2, pszType, -1, NULL );
        rc = sqlite3_bind_int( insert_stmt, 3, nCount );
        rc = sqlite3_bind_double( insert_stmt, 4, dfFte );
        rc = sqlite3_bind_int( insert_stmt, 5, nPeople );
        rc = sqlite3_step( insert_stmt );
        rc = sqlite3_reset( insert_stmt );
    }
    sqlite3_finalize( select_stmt );
    sqlite3_finalize( insert_stmt );
    sqlite3_exec(*new_db, "END TRANSACTION", NULL, NULL, NULL);
    select_stmt = insert_stmt = NULL;
    return 0;
}

/**
 * \brief Launder Lichtenstein's type to a fpa type.
 * \param pszIn Type to be laundered
 * \return laundered type
 */
const char * IRSDataAccess::LaunderType( const char *pszIn )
{
    if( EQUALN( pszIn, "ZEN", 3 ) || EQUALN( pszIn, "EN", 2 ) )
        return "EN34";
    else if( EQUAL( pszIn, "HANDCRW" ) )
        return "CRW";
    else if( EQUAL( pszIn, "REGHOT" ) || EQUAL( pszIn, "NATHOT" ) )
        return "CRW20";
    else if( EQUAL( pszIn, "RAPPEL" ) )
        return "HELI";
    else if( EQUAL( pszIn, "HEL2E" ) )
        return "HEL2";
    else if( EQUAL( pszIn, "HEAVY" ) || EQUAL( pszIn, "DZR" ) )
        return "DZR2";
    else if( EQUAL( pszIn, "TP" ) )
        return "TP34";
    else
        return pszIn;
}

/**
 * \brief Given a region, type of resource and number of those resources,
 * populate dispatch locations with generic resources. Should be used when
 * using macro-scale simulations.
 *
 * \param General resource database
 * \param oLoc a vector  idx_scenario_geometry WHERE  X(scenario.geometry) <= MBRMaxX(fpu_bndry.geometry) AND  X(scenario.geometry) >= MBRMinX(fpu_bndry.geometry) AND  Y(scenario.geometry) <= MBRMaxY(fpu_bndry.geometry) AND  Y(scenario.geometry) <= MBRMinY(fpu_bndry.geometry)) 
of dispatch locations
 * \param [out] oResc a vector of resources to fill
 * \return zero on success, non-zero otherwise
 */
IRS_Err
SpatialiteDataAccess::PopulateGeneralResources( sqlite3 *res_db,
                                                std::vector<CDispLoc>&oLoc,
                                                std::vector<CResource*>&oResc,
                                                std::vector<CRescType>&oRescType,
                                                int (*pfnProgress)(double, const char*, void*))
{
    if( res_db == NULL )
        return 1;
    int rc = 0;
    sqlite3_stmt *fpu_stmt;
    sqlite3_stmt *gres_stmt;

    rc = sqlite3_prepare_v2( db, "SELECT fpu.name from disploc JOIN fpu ON " \
                                 "fpu.id=disploc.fpu_id WHERE disploc.name=?",
                             -1, &fpu_stmt, NULL );
    rc = sqlite3_prepare_v2( res_db, "SELECT type, count from general_resc "
                                     "WHERE gacc=?", -1, &gres_stmt, NULL);

    int i = 0;
    const char *pszFpu;
    char *pszGacc;
    int nRegion = 0;
    double dfRescDist = 0.0;
    int nTotalRescCount = 0;
    const char *pszType;
    double dfRemainder;
    CResource *poResource;
    int nDone = 0;
    for( int i = 0; i < oLoc.size(); i++ )
    {
        nDone++;
        rc = sqlite3_bind_text( fpu_stmt, 1, oLoc[i].GetDispLocID().c_str(), -1,
                                SQLITE_TRANSIENT );
        if( sqlite3_step( fpu_stmt ) != SQLITE_ROW )
        {
            //CPLError( CE_Warning, CPLE_AppDefined, "Could not locate " \
            //                                       "dispatch location." );
            fprintf( stderr, "Could not locate dispatch location." );
            sqlite3_reset( fpu_stmt );
            continue;
        }
        // Tease out the region from the fpu name
        pszFpu = (const char*)sqlite3_column_text( fpu_stmt, 0 );
        assert( strlen( pszFpu ) > 3 );
        pszGacc = strdup( pszFpu );
        pszGacc[2] = '\0';
        rc = sqlite3_bind_text( gres_stmt, 1, pszGacc, -1, NULL );
        while( sqlite3_step( gres_stmt ) == SQLITE_ROW )
        {
            pszType = LaunderType((const char*)sqlite3_column_text( gres_stmt, 0 ) );
            nTotalRescCount = sqlite3_column_int( gres_stmt, 1 );
            rc = ResourceDist( pszGacc, pszType, oLoc[i].GetDispLocID().c_str(),
                               NULL, &dfRescDist );
            if( rc != 0 )
            {
                //CPLError( CE_Warning, CPLE_AppDefined, "Could not load resource"
                                                       //"distribution" );
                fprintf( stderr, "Could not load resource distribution" );

                continue;
            }
            else if( dfRescDist == 0.0 )
            {
                //CPLDebug( "DataAccess", "No resource distribution here" );
                continue;
            }
            else
            {
                for( int j = 0;j < (int)(dfRescDist * nTotalRescCount); j++ )
                {
                    int k = 0;
                    while( k < oRescType.size() )
                    {
                        if( std::string( pszType ) == oRescType[k].GetRescType() )
                            break;
                        k++;
                    }
                    if( k == oRescType.size() )
                    {
                        //CPLError( CE_Warning, CPLE_AppDefined, "Could not associate "
                                  //"resource with a type" );
                        fprintf( stderr, "Could not associate "
                                  "resource with a type" );
                        continue;
                    }
                    poResource = CreateGenericResource( pszType, oRescType[k], oLoc[i] );
                    if( poResource )
                    {
                        //CPLDebug( "DataAccess", "Loading default resource: "
                                //"%s for location:%s",
                                //pszType, oLoc[i].GetDispLocID().c_str());
                        oResc.push_back( poResource );
                    }
                }
            }
        }
        //free( pszGacc );
        sqlite3_reset( gres_stmt );
        sqlite3_reset( fpu_stmt );
    }
    sqlite3_finalize( gres_stmt );
    sqlite3_finalize( fpu_stmt );
    gres_stmt = fpu_stmt = NULL;
    return 0;
}

CResource *
SpatialiteDataAccess::CreateGenericResource( const char *pszType, CRescType &oType,
                                             CDispLoc &oLoc )
{
    sqlite3_stmt *stmt;
    sqlite3_stmt *dl_stmt;
    sqlite3_prepare_v2( db, "SELECT resc_default.*, resc_type.* "
                            "FROM resc_default JOIN resc_type ON " \
                            "resc_default.resc_type=resc_type.id WHERE " \
                            "resc_type.name=?", -1, &stmt, NULL );

    sqlite3_bind_text( stmt, 1, pszType, -1, NULL );
    if( sqlite3_step( stmt ) != SQLITE_ROW )
    {
        //CPLError( CE_Warning, CPLE_AppDefined, "Could not load generic " \
                                               //"resource" );
        fprintf( stderr, "Could not load generic " \
                                               "resource" );
        sqlite3_reset( stmt );
        sqlite3_finalize( stmt );
        return NULL;
    }

    /* Don't need name, or type, or fpu */
    char pszRescName[512];
    sprintf( pszRescName, "Generic_%s", pszType );
    int nStaffing;
    const char *pszStartTime;
    const char *pszEndTime;
    const char *pszStartDay;
    const char *pszEndDay;
    int nStartSeason;
    int nEndSeason;
    int nPercAvail;
    double dfDayCost;
    double dfHourCost;
    int nVolume;
    int bSeverity;
    const char *pszVersion;
    double dfFte;
    double dfNumPositions;
    double dfAnnualCost;
    double dfVehicleCost;

    int nSpeed;
    int nDispDelay;
    int nResponseDelay;
    int nSetupTime;

    nStaffing = sqlite3_column_int( stmt, 2 );
    pszStartTime = (const char*)sqlite3_column_text( stmt, 3 );
    pszEndTime = (const char*)sqlite3_column_text( stmt, 4 );
    pszStartDay = DayOfWeek( sqlite3_column_int( stmt, 5 ) );
    pszEndDay = DayOfWeek( sqlite3_column_int( stmt, 6 ) );
    nStartSeason = sqlite3_column_int( stmt, 7 );
    nEndSeason = sqlite3_column_int( stmt, 8 );
    nPercAvail = sqlite3_column_int( stmt, 10 );
    dfDayCost = sqlite3_column_double( stmt, 11 );
    dfHourCost = sqlite3_column_double( stmt, 12 );
    nVolume = sqlite3_column_int( stmt, 13 );
    bSeverity = sqlite3_column_int( stmt, 14 );
    pszVersion = (const char*)sqlite3_column_text( stmt, 15 );
    dfFte = sqlite3_column_double( stmt, 16 );
    dfNumPositions = sqlite3_column_double( stmt, 17 );
    dfAnnualCost = sqlite3_column_double( stmt, 18 );
    dfVehicleCost = sqlite3_column_double( stmt, 19 );

    nSpeed = sqlite3_column_int( stmt, 24 );
    nDispDelay = sqlite3_column_int( stmt, 25 );
    nResponseDelay = sqlite3_column_int( stmt, 26 );
    nSetupTime = sqlite3_column_int( stmt, 27 );

    CResource *poResource = NULL;

    if( EQUALN( pszType, "DZR", 3 ) ||
        EQUALN( pszType, "TP", 2 ) )
    {
        poResource = new CConstProd( pszRescName,
                                     oType,
                                     nStaffing, pszStartTime,
                                     pszEndTime, pszStartDay, 
                                     pszEndDay, nStartSeason,
                                     nEndSeason, oLoc,
                                     nPercAvail, dfDayCost,
                                     dfHourCost );
    }
    else if( EQUAL( pszType, "CRW" ) )
    {
        poResource = new CCrew( pszRescName,
                                oType, nStaffing,
                                pszStartTime, pszEndTime, pszStartDay,
                                pszEndDay, nStartSeason,
                                nEndSeason, oLoc, nPercAvail,
                                dfDayCost, dfHourCost );
    }
    else if( EQUALN( pszType, "EN", 2 ) )
    {
        poResource = new CEngine( pszRescName,
                                 oType, nStaffing,
                                 pszStartTime, pszEndTime, 
                                 pszStartDay, pszEndDay, 
                                 nStartSeason, nEndSeason, 
                                 oLoc, nPercAvail, dfDayCost,
                                 dfHourCost, nVolume );
    }
    else if( EQUAL( pszType, "WT" ) )
    {
         poResource = new CWaterTender( pszRescName,
                                        oType,
                                        nStaffing, pszStartTime,
                                        pszEndTime, pszStartDay,
                                        pszEndDay, nStartSeason,
                                        nEndSeason, oLoc,
                                        nPercAvail, dfDayCost,
                                        dfHourCost );
    }
    else if( EQUAL( pszType, "ATT" ) )
    {
        poResource = new CAirtanker( pszRescName,
                                     oType,
                                     nStaffing, pszStartTime,
                                     pszEndTime, pszStartDay,
                                     pszEndDay, nStartSeason,
                                     nEndSeason, oLoc,
                                     nPercAvail, dfDayCost,
                                     dfHourCost, nVolume );
    }
    else if( EQUAL( pszType, "SEAT" ) || EQUAL( pszType, "SCP" ) )
    {
        poResource = new CSmallAT( pszRescName,
                                   oType, 
                                   nStaffing, pszStartTime,
                                   pszEndTime, pszStartDay, pszEndDay,
                                   nStartSeason, nEndSeason,
                                   oLoc, nPercAvail, dfDayCost,
                                   dfHourCost, nVolume );
    }
    else if( EQUAL( pszType, "SMJR" ) )
    {
        poResource = new CSmokejumper( pszRescName,
                                       oType,
                                       nStaffing, pszStartTime,
                                       pszEndTime, pszStartDay,
                                       pszEndDay, nStartSeason,
                                       nEndSeason, oLoc,
                                       nPercAvail, dfDayCost,
                                       dfHourCost ); 
    }
    else if( EQUAL( pszType, "SJAC" ) )
    {
         poResource = new CSMJAircraft( pszRescName,
                                        oType,
                                        nStaffing, pszStartTime,
                                        pszEndTime, pszStartDay,
                                        pszEndDay, nStartSeason,
                                        nEndSeason, oLoc,
                                        nPercAvail, dfDayCost,
                                        dfHourCost, nVolume );
    }
    else if( EQUAL( pszType, "HELI" ) )
    {
        poResource = new CHelitack( pszRescName,
                                    oType,
                                    nStaffing, pszStartTime,
                                    pszEndTime, pszStartDay,
                                    pszEndDay, nStartSeason,
                                    nEndSeason, oLoc,
                                    nPercAvail, dfDayCost,
                                    dfHourCost );
    }
    else if( EQUALN( pszType, "HEL", 3 ) )
    {
        poResource = new CHelicopter( pszRescName,
                                      oType,
                                      nStaffing, pszStartTime,
                                      pszEndTime, pszStartDay,
                                      pszEndDay, nStartSeason,
                                      nEndSeason, oLoc,
                                      nPercAvail, dfDayCost,
                                      dfHourCost, nVolume );
    }
    sqlite3_finalize( stmt );
    return poResource;
}


/**
 * \brief Return the percent a given dispatch location is responsible for in a
 * gacc for any given resource.
 */
IRS_Err
SpatialiteDataAccess::ResourceDist( const char *pszGacc, const char *pszType,
                                    const char *pszDispLoc,
                                    const char *pszAgency, double *pdfDist )
{
    int nErr = 0;
    int nCount, nSum;
    int rc;
    const char *pszAgencyLaunder;
    sqlite3_stmt *sum_stmt = NULL;
    sqlite3_stmt *perc_stmt = NULL;
    if( pszAgency == NULL )
    {
        pszAgencyLaunder = strdup( "%" );
    }
    else
    {
        pszAgencyLaunder = strdup( pszAgency );
    }

    rc = sqlite3_prepare_v2( db, "SELECT sum(count) FROM resc_dist WHERE "
                                 "ga_abbr=? AND resc_type=? AND agency LIKE ?",
                             -1, &sum_stmt, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT count FROM resc_dist WHERE "
                                 "ga_abbr=? AND resc_type=? "
                                 "AND displocname=? AND agency LIKE ?",
                             -1, &perc_stmt, NULL );
    rc = sqlite3_bind_text( sum_stmt, 1, pszGacc, -1, NULL );
    rc = sqlite3_bind_text( sum_stmt, 2, pszType, -1, NULL );
    rc = sqlite3_bind_text( sum_stmt, 3, pszAgencyLaunder, -1, NULL );
    rc = sqlite3_bind_text( perc_stmt, 1, pszGacc, -1, NULL );
    rc = sqlite3_bind_text( perc_stmt, 2, pszType, -1, NULL );
    rc = sqlite3_bind_text( perc_stmt, 3, pszDispLoc, -1, NULL );
    rc = sqlite3_bind_text( perc_stmt, 4, pszAgencyLaunder, -1, NULL );
    /* iffy here.  if there is no count, no rows are returned. */
    if( sqlite3_step( sum_stmt ) != SQLITE_ROW )
    {
        *pdfDist = 0.0;
    }
    else if( sqlite3_step( perc_stmt ) != SQLITE_ROW )
    {
        *pdfDist = 0.0;
    }
    else
    {
        nSum = sqlite3_column_int( sum_stmt, 0 );
        if(nSum <= 0)
        {
            //CPLError( CE_Warning, CPLE_AppDefined,
            //          "Could not complete Resource Count query" );
            fprintf( stderr, 
                      "Could not complete Resource Count query" );
            nErr = 1;
        }
        else
        {
            nCount = sqlite3_column_int( perc_stmt, 0 );
            *pdfDist = (double)nCount / (double)nSum;
        }
    }
    sqlite3_finalize( sum_stmt );
    sqlite3_finalize( perc_stmt );
    free( (void*)pszAgencyLaunder );
    sum_stmt = perc_stmt = NULL;
    return nErr;
}

sqlite3* SpatialiteDataAccess::ExportFullResults( const char* pszOutputFile, int nYear,
                                                  std::vector<CResults>oResults,
                                                  std::vector<int>oRescUsage )
{
    sqlite3 *result_db;
    sqlite3_stmt *stmt;
    sqlite3_stmt *usg_stmt;
    int rc = 0;
    rc = sqlite3_open_v2( pszOutputFile, &result_db,
                          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL );
    if( rc != SQLITE_OK || result_db == NULL )
    {
        return NULL;
    }

    //Year,Julian Day,Fire #,# Fires/Day,Day of Week,Discovery Time,BI,ROS,FM,IsWalkIn,Distance,L to W Ratio,FWA,Arrival Time,Arrival Size, Status
    rc = sqlite3_exec( result_db, "CREATE TABLE IF NOT EXISTS "
                                  "full_results(year integer, "
                                               "jul_day integer, "
                                               "fire_num integer, "
                                               "arrtime integer,"
                                               "arrsize real,"
                                               "finalsize real,"
                                               "finaltime real,"
                                               "run_contain integer,"
                                               "treated integer,"
                                               "status text)",
                       NULL, NULL, NULL );
    rc = sqlite3_exec( result_db, "CREATE TABLE IF NOT EXISTS "
                                  "resc_usage(year integer, "
                                             "att integer,"
                                             "crw integer,"
                                             "dzr integer,"
                                             "eng integer,"
                                             "fbdz integer,"
                                             "frbt integer,"
                                             "helicopter integer,"
                                             "heli integer,"
                                             "scpseat integer,"
                                             "sjac integer,"
                                             "smjr integer,"
                                             "tp integer,"
                                             "wt integer)",
                       NULL, NULL, NULL );
    rc = sqlite3_exec( result_db, "CREATE INDEX IF NOT EXISTS idx_day_num_year ON full_results(year, jul_day, fire_num)",
                       NULL, NULL, NULL );
    rc = sqlite3_prepare_v2( result_db, "INSERT INTO full_results(year, jul_day, "
                                        "fire_num, arrtime, arrsize, finalsize, "
                                        "finaltime, treated, run_contain, status) "
                                        "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1,
                             &stmt, NULL );
    rc = sqlite3_prepare_v2( result_db, "INSERT INTO resc_usage(year, att, crw, dzr, " \
                                        "eng, fbdz, frbt, helicopter, heli, " \
                                        "scpseat, sjac, smjr, tp, wt) VALUES "\
                                        "(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                             -1, &usg_stmt, NULL );
    rc = sqlite3_exec( result_db, "PRAGMA SYNCHRONOUS=OFF", NULL, NULL, NULL );
    rc = sqlite3_exec( result_db, "BEGIN TRANSACTION", NULL, NULL, NULL );
    int nFireNum, nJulDay, nArrTime;
    std::string oStatus;
    double dfArrSize;
    double dfFinalSize, dfFinalTime;
    int bTreated, bRunContain;
    int nFullYear = oResults[0].GetFire().GetScenario();
    for( int i = 0;i < oResults.size(); i++ )
    {
        //nYear = oResults[i].GetFire().GetScenario();
        nJulDay = oResults[i].GetFire().GetJulianDay();
        nFireNum = oResults[i].GetFire().GetFireNumber();
        nArrTime = (int)oResults[i].GetFireCost();
        dfArrSize = oResults[i].GetFireSweep();
        dfFinalSize = oResults[i].GetFireSize();
        dfFinalTime = oResults[i].GetFireTime();
        bTreated = oResults[i].GetFire().GetTreated();
        bRunContain = oResults[i].GetFire().GetSimulateContain();
        oStatus = oResults[i].GetStatus();
        rc = sqlite3_bind_int( stmt, 1, nYear );
        rc = sqlite3_bind_int( stmt, 2, nJulDay );
        rc = sqlite3_bind_int( stmt, 3, nFireNum );
        rc = sqlite3_bind_int( stmt, 4, nArrTime );
        rc = sqlite3_bind_double( stmt, 5, dfArrSize );
        rc = sqlite3_bind_double( stmt, 6, dfFinalSize );
        rc = sqlite3_bind_double( stmt, 7, dfFinalTime );
        rc = sqlite3_bind_int( stmt, 8, bTreated );
        rc = sqlite3_bind_int( stmt, 9, bRunContain );
        rc = sqlite3_bind_text( stmt, 10, oStatus.c_str(), -1, SQLITE_TRANSIENT );
        rc = sqlite3_step( stmt );
        rc = sqlite3_reset( stmt );
    }
    rc = sqlite3_bind_int( usg_stmt, 1, nYear );
    for( int i = 0; i < oRescUsage.size(); i++ )
    {
        rc = sqlite3_bind_int( usg_stmt, i + 2, oRescUsage[i] );
    }
    rc = sqlite3_step( usg_stmt );
    rc = sqlite3_reset( usg_stmt );
    rc = sqlite3_exec( result_db, "END TRANSACTION", NULL, NULL, NULL );
    rc = sqlite3_exec( result_db, "PRAGMA SYNCHRONOUS=ON", NULL, NULL, NULL );
    rc = sqlite3_finalize( stmt );
    rc = sqlite3_finalize( usg_stmt );
    stmt = usg_stmt = NULL;
    return result_db;
}

/**
 * \brief Export results to a spatialite database.
 *
 * Create a file in memory or on disk using sqlite semantics (ie ":memory:",
 * "", etc.).  Enable spatial features for the fire and write data to the db.
 * This will allow for spatial queries and summary statistics.
 *
 * \param pszOutputFile file to write data to
 * \param oResults a vector of results classes
 * \param dfSample the percentage of fires to keep in the outputs, 0.0 - 1.0
 * \return a handle to a sqlite3 db, NULL otherwise.
 */
sqlite3* SpatialiteDataAccess::ExportResults( const char* pszOutputFile, int nYear,
                                              std::vector<CResults>oResults,
                                              double dfPercentKeep )
{
    int rc = 0;
    sqlite3 *result_db;
    sqlite3_stmt *stmt;
    rc = sqlite3_open_v2( pszOutputFile, &result_db,
                          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL );
    if( rc || result_db == NULL )
    {
        return NULL;
    }
    rc = sqlite3_exec( result_db, "CREATE TABLE IF NOT EXISTS "
                                  "results(fwa text, "
                                          "year integer, "
                                          "contained integer, "
                                          "escapes integer, "
                                          "norescs integer)",
                       NULL, NULL, NULL );

    rc = sqlite3_exec( result_db, "CREATE INDEX IF NOT EXISTS idx_fwa_year ON results(fwa, year)",
                       NULL, NULL, NULL );

    rc = sqlite3_prepare_v2( result_db, "INSERT INTO results(fwa, year, contained, "
                                        "escapes, norescs) VALUES(?, ?, ?, ?, ?)", -1,
                             &stmt, NULL );
    std::map< std::string, std::vector<int> > oNewMap;
    std::map< std::string, std::vector<int> >::iterator oIterator;
    std::pair<std::map< std::string, std::vector<int> >::iterator, bool> oReturnPair;
    std::string oFwa, oStatus;
    for( int i = 0;i < oResults.size(); i++ )
    {
        oFwa = oResults[i].GetFire().GetFWA().GetFWAID();
        oStatus = oResults[i].GetStatus();
        std::pair<std::string, std::vector<int> > oPair = std::pair<std::string, std::vector<int> >( oFwa, std::vector<int>(3) );
        oReturnPair = oNewMap.insert( oPair );
        oIterator = oReturnPair.first;
        if( oStatus == "Contained" )
        {
            oIterator->second[0]++;
        }
        else if( oStatus == "No Resources Sent" )
        {
            oIterator->second[2]++;
        }
        else
        {
            oIterator->second[1]++;
        }
    }
    rc = sqlite3_exec( result_db, "PRAGMA synchronous = OFF", NULL, NULL, NULL );
    rc = sqlite3_exec( result_db, "BEGIN TRANSACTION", NULL, NULL, NULL );
    oIterator = oNewMap.begin();
    int nContain, nEscape, nNoResc;
    for( ;oIterator != oNewMap.end();oIterator++ )
    {
        oFwa = oIterator->first;
        nContain = oIterator->second[0];
        nEscape = oIterator->second[1];
        nNoResc = oIterator->second[2];

        rc = sqlite3_bind_text( stmt, 1, oFwa.c_str(), -1, SQLITE_TRANSIENT );
        rc = sqlite3_bind_int( stmt, 2, nYear );
        rc = sqlite3_bind_int( stmt, 3, nContain );
        rc = sqlite3_bind_int( stmt, 4, nEscape );
        rc = sqlite3_bind_int( stmt, 5, nNoResc );
        rc = sqlite3_step( stmt );
        rc = sqlite3_reset( stmt );
    }
    rc = sqlite3_exec( result_db, "END TRANSACTION", NULL, NULL, NULL );
    rc = sqlite3_finalize( stmt );
    stmt = NULL;
    return result_db;
}

std::map< std::string, std::vector< int > >
SpatialiteDataAccess::SummarizeFwa( std::vector<CResults>oResults,
                                    const char *pszOutputFile, double dfKeep,
                                    int (*pfnProgress)(double, const char*, void*))
{
    int nTotalSize = 0;
    std::string oFwa;
    std::string oStatus;
    std::map< std::string, std::vector< int > > oNewMap;
    std::map< std::string, std::vector<int> >::iterator oIterator;
    std::pair<std::map< std::string, std::vector<int> >::iterator, bool> oReturnPair;
    for( int i = 0;i < oResults.size(); i++ )
    {
        //std::random_shuffle( oResults.begin[i](), oResults[i].end() )
        oFwa = oResults[i].GetFire().GetFWA().GetFWAID();
        oStatus = oResults[i].GetStatus();
        std::pair<std::string, std::vector<int> > oPair = std::pair<std::string, std::vector<int> >( oFwa, std::vector<int>(3) );
        oReturnPair = oNewMap.insert( oPair );
        oIterator = oReturnPair.first;
        if( oStatus == "Contained" )
        {
            oIterator->second[0]++;
        }
        else if( oStatus == "No Resources Sent" )
        {
            oIterator->second[2]++;
        }
        else
        {
            oIterator->second[1]++;
        }
    }
    return oNewMap;
}

std::vector<std::string>
SpatialiteDataAccess::GetFwasFromRegion( const char* pszRegion )
{
    int rc = 0;
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, "SELECT fwa.name FROM fpu join fwa "
                                 "ON fwa.fpu_id=fpu.id WHERE "
                                 "fpu.name LIKE ? || '%'", -1,
                             &stmt, NULL );
    sqlite3_bind_text( stmt, 1, pszRegion, -1, NULL );
    std::string oFwa;
    std::vector<std::string>oFwas;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        oFwa = (const char*)sqlite3_column_text( stmt, 0 );
        oFwas.push_back( oFwa );
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return oFwas;
}

std::vector<std::string>
SpatialiteDataAccess::GetFwasFromFpu( const char* pszFpu )
{
    int rc = 0;
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, "SELECT fwa.name FROM fpu join fwa "
                                 "ON fwa.fpu_id=fpu.id WHERE fpu.name=?", -1,
                             &stmt, NULL );
    sqlite3_bind_text( stmt, 1, pszFpu, -1, NULL );
    std::string oFwa;
    std::vector<std::string>oFwas;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        oFwa = (const char*)sqlite3_column_text( stmt, 0 );
        oFwas.push_back( oFwa );
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return oFwas;
}

std::vector< std::map<std::string, std::string> >
SpatialiteDataAccess::GetAttributes( const char *pszTable,
                                     const char *pszQueryField,
                                     const char *pszQueryValue )
{
    int i = 0;
    int rc = 0;
    char sql[1024];
    std::vector< std::map<std::string, std::string> > oRows;
    std::map<std::string, std::string>oMap;
    sprintf( sql, "SELECT * FROM %s WHERE %s=?", pszTable, pszQueryField );
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, sql, -1, &stmt, NULL );
    rc = sqlite3_bind_text( stmt, 1, pszQueryValue, -1, NULL );
    const char *pszName, *pszValue;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        for( i = 0;i < sqlite3_data_count( stmt );i++ )
        {
            pszName = (const char*)sqlite3_column_name( stmt, i );
            pszValue = (const char*)sqlite3_column_text( stmt, i );
            oMap[pszName] = pszValue;
        }
        oRows.push_back(oMap);
    }
    sqlite3_finalize( stmt );
    return oRows;
}

std::vector<int>
SpatialiteDataAccess::GetFireCount( int nYear, const char *pszGeometry )
{
    sqlite3_stmt *stmt;
    if( pszGeometry == NULL )
    {
        sqlite3_prepare_v2( db, "SELECT jul_day, count(*) FROM scenario "
                                "WHERE year=? AND region < 7 "
                                "GROUP BY jul_day", -1, &stmt,
                            NULL );
    }
    else
    {
        /*
        ** FIXME: Add RTree
        */
        sqlite3_prepare_v2( db, "SELECT jul_day, count(*) FROM scenario "
                                "WHERE year=? AND region < 7 "
                                "AND MBRContains(GeomFromText(?), geometry) "
                                "GROUP BY jul_day",
                            -1, &stmt, NULL );
    }
    sqlite3_bind_int( stmt, 1, nYear );
    if( pszGeometry != NULL )
    {
        sqlite3_bind_text( stmt, 2, pszGeometry, -1, NULL );
    }
    std::vector<int>count(365);
    int i = 0;
    int nDay, nCount;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        assert(i < 365);
        nDay = sqlite3_column_int( stmt, 0 );
        nCount = sqlite3_column_int( stmt, 1 );
        count[nDay-1] = nCount;
        i++;
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return count;
}

std::vector<double>
SpatialiteDataAccess::GetFireStatsMean( const char *pszGeometry,
                                        int (*pfnProgress)(double, const char*, void*) )
{
    sqlite3_stmt *stmt;
    sqlite3_stmt *count_stmt;
    sqlite3_prepare_v2( db, "SELECT COUNT(DISTINCT(year)) FROM scenario ",
                        -1, &count_stmt, NULL );
    int rc = sqlite3_step( count_stmt );
    int nYears = sqlite3_column_int( count_stmt, 0 );
    rc = sqlite3_finalize( count_stmt );
    if( pszGeometry == NULL )
    {
        rc = sqlite3_prepare_v2( db, "SELECT jul_day, COUNT(*) FROM "
                                     "scenario where region < 7 "
                                     "GROUP BY jul_day", -1, &stmt, NULL );
    }
    else
    {
        rc = sqlite3_prepare_v2( db, "SELECT jul_day, COUNT(*) FROM "
                                     "scenario WHERE region < 7 AND "
                                     "scenario.ROWID IN (SELECT pkid FROM "
                                     "idx_scenario_geometry WHERE "
                                     "xmin <= MbrMaxX(GeomFromText(?1)) AND "
                                     "xmax >= MbrMinX(GeomFromText(?1)) AND "
                                     "ymin <= MbrMaxY(GeomFromText(?1)) AND "
                                     "ymax >= MbrMinY(GeomFromText(?1)))"
                                     "GROUP BY jul_day",
                                     -1, &stmt, NULL );
    }
    if( pszGeometry != NULL )
    {
        sqlite3_bind_text( stmt, 1, pszGeometry, -1, NULL );
    }
    std::vector<double>count(365);
    for( int i = 0;i < 365;i++ )
    {
        count[i] = 0;
    }
    int i = 0;
    int nDay, nCount;
    double dfDone;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        assert(i < 365);
        nDay = sqlite3_column_int( stmt, 0 );
        nCount = sqlite3_column_int( stmt, 1 );
        count[nDay-1] = (double)nCount / nYears;
        i++;
        dfDone = (float)i / 365.0;
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return count;
}


/**
 * \brief Get minimum statistics of the fire ignitions
 *
 * \param pszGeometry Geometry subset
 * \return vector of minimum fires of length 365
 */
std::vector <int>
SpatialiteDataAccess::GetFireStatsMin( const char *pszGeometry,
                                       int (*pfnProgress)(double, const char*, void*))
{
    sqlite3_stmt *stmt;
    char sql[8192];
    int rc;
    if( pszGeometry == NULL )
    {
        sqlite3_prepare_v2( db, "SELECT jul_day, MIN(cnt) FROM "
                                "(SELECT jul_day, COUNT(*) as cnt FROM "
                                "scenario where region < 7 "
                                "GROUP BY year, jul_day) "
                                "GROUP BY jul_day", -1, &stmt, NULL );
    }
    else
    {
        rc = sqlite3_prepare_v2( db, "SELECT jul_day, MIN(cnt) FROM "
                                     "(SELECT jul_day, COUNT(*) as cnt FROM "
                                     "scenario WHERE region < 7 AND "
                                     "scenario.ROWID IN (SELECT pkid FROM "
                                     "idx_scenario_geometry WHERE "
                                     "xmin <= MBRMaxX(GeomFromText(?1)) AND "
                                     "xmax >= MBRMinX(GeomFromText(?1)) AND "
                                     "ymin <= MBRMaxY(GeomFromText(?1)) AND "
                                     "ymax >= MBRMinY(GeomFromText(?1))) "
                                     "GROUP BY year, jul_day) "
                                     "GROUP BY jul_day",
                                     -1, &stmt, NULL );

    }
    if( pszGeometry != NULL )
    {
        sqlite3_bind_text( stmt, 1, pszGeometry, -1, NULL );
    }
    std::vector<int>count(365);
    for( int i = 0;i < 365;i++ )
    {
        count[i] = 0;
    }
    int i = 0;
    int nDay, nCount;
    double dfDone;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        assert(i < 365);
        nDay = sqlite3_column_int( stmt, 0 );
        nCount = sqlite3_column_int( stmt, 1 );
        count[nDay-1] = nCount;
        i++;
        dfDone = (float)i / 365.0;
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return count;
}

/**
 * \brief Sample the large fire simulator.
 *
 * Assign certain values to an escaped fire given a location.
 *
 * \param dfX X coordinate
 * \param dfY Y coordinate
 * \param dfRadius maximum search radius
 * \param nMethod search method
 * \param [out] psLargeFire Large fire information to extract
 *
 * \return 0 on success.
 *
 * \todo TODO: Figure out sampling algorithm.  This currently just takes a
 * radius, it should probably be thought over.  Limit by julian day, fuel
 * model, etc.  Space/time sampling.  Also it might be a good idea to group the
 * queries so as to only prepare the statement once, for example, JOIN the
 * escapes with the large fire table and select the closest one with a day
 * close to julian day of the fire, and perhaps a similar fuel model.  The
 * query (in that case) has to be deterministic, no random perturbations can be
 * involved.
 */
int
SpatialiteDataAccess::GetLargeFireValues( double dfX, double dfY,
                                          double dfRadius, int nMethod,
                                          IRSLargeFire *psLargeFire )
{
    sqlite3_stmt *stmt;
    int rc;
    sqlite3_prepare_v2( db, "SELECT * FROM large_fire WHERE " \
                            "ST_Intersects(" \
                            "ST_Buffer(MakePoint(?1, ?2, 4269), ?3), geometry) " \
                            "AND large_fire.ROWID IN(" \
                            "SELECT pkid FROM idx_large_fire WHERE " \
                            "xmin <= MbrMaxX( " \
                            "ST_Buffer(MakePoint(?1, ?2, 4269), ?3)) AND " \
                            "xmax >= MbrMinX(" \
                            "ST_Buffer(MakePoint(?1, ?2, 4269), ?3)) AND " \
                            "ymin <= MbrMaxY(" \
                            "ST_Buffer(MakePoint(?1, ?2, 4269), ?3)) AND " \
                            "ymax >= MbrMinY(" \
                            "ST_Buffer(MakePoint(?1, ?2, 4269), ?3)))",
                            -1, &stmt, NULL );

    sqlite3_bind_double( stmt, 1, dfX );
    sqlite3_bind_double( stmt, 2, dfY );
    sqlite3_bind_double( stmt, 3, dfRadius );

    sqlite3_reset( stmt );
    sqlite3_finalize( stmt );
    stmt = NULL;
    return 0;
}


/**
 * \brief Get maximum statistics of the fire ignitions
 *
 * \param pszGeometry Geometry subset
 * \return vector of minimum fires of length 365
 */
std::vector <int>
SpatialiteDataAccess::GetFireStatsMax( const char *pszGeometry,
                                       int (*pfnProgress)(double, const char*, void*))
{
    sqlite3_stmt *stmt;
    char sql[8192];
    int rc;
    if( pszGeometry == NULL )
    {
        sqlite3_prepare_v2( db, "SELECT jul_day, MAX(cnt) FROM " \
                                "(SELECT jul_day, COUNT(*) as cnt FROM " \
                                "scenario WHERE region < 7 " \
                                "GROUP BY year, jul_day) " \
                                "GROUP BY jul_day", -1, &stmt, NULL );
    }
    else
    {
        rc = sqlite3_prepare_v2( db, "SELECT jul_day, MAX(cnt) FROM "
                                     "(SELECT jul_day, COUNT(*) as cnt FROM "
                                     "scenario WHERE region < 7 AND "
                                     "scenario.ROWID IN (SELECT pkid FROM "
                                     "idx_scenario_geometry WHERE "
                                     "xmin <= MBRMaxX(GeomFromText(?1)) AND "
                                     "xmax >= MBRMinX(GeomFromText(?1)) AND "
                                     "ymin <= MBRMaxY(GeomFromText(?1)) AND "
                                     "ymax >= MBRMinY(GeomFromText(?1))) "
                                     "GROUP BY year, jul_day) "
                                     "GROUP BY jul_day",
                                     -1, &stmt, NULL );
    }
    if( pszGeometry != NULL )
    {
        sqlite3_bind_text( stmt, 1, pszGeometry, -1, NULL );
    }
    std::vector<int>count(365);
    for( int i = 0;i < 365;i++ )
    {
        count[i] = 0;
    }
    int i = 0;
    int nDay, nCount;
    double dfDone;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        assert(i < 365);
        nDay = sqlite3_column_int( stmt, 0 );
        nCount = sqlite3_column_int( stmt, 1 );
        count[nDay-1] = nCount;
        i++;
        dfDone = (float)i / 365.0;
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return count;
}


/**
 * \brief Get the index of an array where a value resides
 *
 * \param probs array of values to search
 * \param find value to find
 * \param size size of probs
 * \return index of value
 */
int IRSDataAccess::GetIndex( double *probs, double find, int size )
{
    if(find > probs[size-1] || find < probs[0])
    {
        return 0;
    }
    int mid = size / 2;
    if(size == 1)
        return 0;
    if(find < probs[mid])
        return 0 + GetIndex(probs, find, mid);
    else
        return mid + GetIndex(probs + mid, find, size - mid);
}

IRS_Err SpatialiteDataAccess::SetTimes()
{
    FILE *fin = fopen("/home/kyle/src/omffr/trunk/data/discovery-times.txt", "r");
    int time;
    double prob;
    double probs[1440];
    int times[1440];
    int i = 0;
    while(fscanf(fin, "%d %lf", &time, &prob) != EOF)
    {
        times[i] = time;
        if(i == 0)
            probs[i] = prob;
        else
            probs[i] = prob + probs[i-1];
        i++;
    }

    int rc;
    sqlite3_stmt *sel_stmt;
    sqlite3_stmt *upd_stmt;
    rc = sqlite3_prepare_v2(db, "SELECT year, fire_num from scenario WHERE "
                                "disc_time=0 OR disc_time IS NULL",
                                -1, &sel_stmt, NULL);
    rc = sqlite3_prepare_v2(db, "UPDATE scenario SET disc_time=? WHERE year=? "
                                "AND fire_num=?", -1, &upd_stmt, NULL);
    Random random;
    int year, fire_num;
    double prb;
    int ign_time;
    int count = 0;
    rc = sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
    while(sqlite3_step(sel_stmt) == SQLITE_ROW)
    {
        year = sqlite3_column_int(sel_stmt, 0);
        fire_num = sqlite3_column_int(sel_stmt, 1);
        prb = random.rand3();
        ign_time = times[GetIndex(probs, prb, 1440)];
        rc = sqlite3_bind_int(upd_stmt, 1, ign_time);
        rc = sqlite3_bind_int(upd_stmt, 2, year);
        rc = sqlite3_bind_int(upd_stmt, 3, fire_num);
        rc = sqlite3_step(upd_stmt);
        sqlite3_reset(upd_stmt);
        count++;
        if(count % 1000 == 0)
        {
            sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, NULL);
        }
    }
    rc = sqlite3_exec(db, "COMMIT TRANSACTION", NULL, NULL, NULL);
    sqlite3_finalize(sel_stmt);
    sqlite3_finalize(upd_stmt);
    sel_stmt = upd_stmt = NULL;

    return 0;
}

IRS_Err SpatialiteDataAccess::WriteCartFile( const char *pszResultDb,
                                             const char *pszOutputCsv )
{
    sqlite3_stmt *stmt;
    sqlite3_stmt *cnt_stmt;
    int rc = 0;
    char sql[1024];
    sprintf( sql, "ATTACH '%s' AS results", pszResultDb );

    rc = sqlite3_exec( db, sql, NULL, NULL, NULL );

    sqlite3_prepare_v2( db, "SELECT scenario.year, scenario.jul_day, "
                            "scenario.fire_num, scenario.week_day, "
                            "scenario.disc_time, scenario.bi, scenario.ros, "
                            "scenario.fuel_model, scenario.walk_in, "
                            "scenario.attack_dist, scenario.ratio, scenario.fwa_id, "
                            "results.full_results.arrtime, "
                            "results.full_results.arrsize, "
                            "results.full_results.status FROM results.full_results "
                            "LEFT JOIN scenario ON "
                            "results.full_results.year=scenario.year AND "
                            "results.full_results.fire_num=scenario.fire_num "
                            "ORDER BY results.full_results.year, "
                            "results.full_results.jul_day",
                        -1, &stmt, NULL );
    sqlite3_prepare_v2( db, "SELECT count(*) FROM results.full_results "
                            "WHERE year=? AND jul_day=?",
                        -1, &cnt_stmt, NULL );

//Year,Julian Day,Fire #,# Fires/Day,Day of Week,Discovery Time,BI,ROS,FM,IsWalkIn
//,Distance,L to W Ratio,FWA,Arrival Time,Arrival Size, Status

    FILE *fout = fopen( pszOutputCsv, "w" );
    if( fout == NULL )
    {}
    int nYear, nDay, nFireNum, nFireCount;
    int nWeekDay;
    const char *pszDiscTime;
    int nBi;
    double dfRos;
    int nFuel, nWalkIn;
    double dfAttDist, dfRatio;
    const char *pszFwa;
    int nArrTime;
    double dfArrSize;
    const char *pszStatus;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        nYear = sqlite3_column_int( stmt, 0 );
        nDay = sqlite3_column_int( stmt, 1 );
        nFireNum = sqlite3_column_int( stmt, 2 );
        nWeekDay = sqlite3_column_int( stmt, 3 );
        pszDiscTime = (const char*)sqlite3_column_text( stmt, 4 );
        nBi = sqlite3_column_int( stmt, 5 );
        dfRos = sqlite3_column_double( stmt, 6 );
        nFuel = sqlite3_column_int( stmt, 7 );
        nWalkIn = sqlite3_column_int( stmt, 8 );
        dfAttDist = sqlite3_column_double( stmt, 9 );
        dfRatio = sqlite3_column_double( stmt, 10 );
        pszFwa = (const char*)sqlite3_column_text( stmt, 11 );
        nArrTime = sqlite3_column_int( stmt, 12 );
        dfArrSize = sqlite3_column_double( stmt , 13 );
        pszStatus = (const char*)sqlite3_column_text( stmt, 14 );

        sqlite3_bind_int( cnt_stmt, 1, nYear );
        sqlite3_bind_int( cnt_stmt, 2, nDay );
        sqlite3_step( cnt_stmt );
        nFireCount = sqlite3_column_int( cnt_stmt, 0 );
        sqlite3_reset( cnt_stmt );

        fprintf( fout, "%d,%d,%d,%d,%s,%s,%d,%lf,%d,%d,%lf,%lf,%s,%d,%lf,%s\n",
                 nYear, nDay, nFireNum, nFireCount, IRSDayOfWeek[nWeekDay],
                 pszDiscTime, nBi, dfRos, nFuel, nWalkIn, dfAttDist, dfRatio,
                 pszFwa, nArrTime, dfArrSize, pszStatus );
    }
    sqlite3_finalize( stmt );
    sqlite3_finalize( cnt_stmt );
    stmt = cnt_stmt = NULL;
    fclose( fout );

    sprintf( sql, "DETACH %s", pszResultDb );
    rc = sqlite3_exec( db, sql, NULL, NULL, NULL );

    return 0;
}

#define RESC_REM_BUF 8192
static void AddValueToSet( char *pBuf, const char *pszToAdd, int bAddComma )
{
    if( bAddComma )
        sprintf( pBuf, "%s,'%s'", pBuf, pszToAdd );
    else
        sprintf( pBuf, "%s'%s'", pBuf, pszToAdd );
}

static void SetRescRemovalMasks( int nAgencies, int nRegions, int nTypes,
                                 char *szAgencySet, char *szRegionSet,
                                 char *szTypeSet )
{
    int bComma = FALSE;
    int bUseAgencies = nAgencies ? TRUE : FALSE;
    int bUseRegions = nRegions ? TRUE : FALSE;
    int bUseTypes = nTypes ? TRUE : FALSE;

    if( bUseAgencies )
    {
        if( nAgencies & USFS )
        {
            AddValueToSet( szAgencySet, "FS", bComma );
            bComma = TRUE;
        }
        if( nAgencies & DOI_BLM )
        {
            AddValueToSet( szAgencySet, "BLM", bComma );
            bComma = TRUE;
        }
        if( nAgencies & DOI_NPS )
        {
            AddValueToSet( szAgencySet, "NPS", bComma );
            bComma = TRUE;
        }
        if( nAgencies & DOI_BIA )
        {
            AddValueToSet( szAgencySet, "BIA", bComma );
            bComma = TRUE;
        }
        if( nAgencies & DOI_FWS )
        {
            AddValueToSet( szAgencySet, "FWS", bComma );
            bComma = TRUE;
        }
        if( nAgencies & STATE_LOCAL )
        {
            AddValueToSet( szAgencySet, "STATE/LOCAL", bComma );
            bComma = TRUE;
        }
    }
    if( bUseRegions )
    {
        bComma = FALSE;
        if( nRegions & NORTHERN_ROCKIES )
        {
            AddValueToSet( szRegionSet, R1, bComma );
            bComma = TRUE;
        }
        if( nRegions & ROCKY_MOUNTAINS )
        {
            AddValueToSet( szRegionSet, R2, bComma );
            bComma = TRUE;
        }
        if( nRegions & SOUTHWEST )
        {
            AddValueToSet( szRegionSet, R3, bComma );
            bComma = TRUE;
        }
        if( nRegions & GREAT_BASIN )
        {
            AddValueToSet( szRegionSet, R4, bComma );
            bComma = TRUE;
        }
        if( nRegions & CALIFORNIA )
        {
            AddValueToSet( szRegionSet, R5, bComma );
            bComma = TRUE;
        }
        if( nRegions & NORTHWEST )
        {
            AddValueToSet( szRegionSet, R6, bComma );
            bComma = TRUE;
        }
        if( nRegions & SOUTHERN )
        {
            AddValueToSet( szRegionSet, R8, bComma );
            bComma = TRUE;
        }
        if( nRegions & EASTERN )
        {
            AddValueToSet( szRegionSet, R9, bComma );
            bComma = TRUE;
        }
        if( nRegions & ALASKA )
        {
            AddValueToSet( szRegionSet, R10, bComma );
            bComma = TRUE;
        }
        if( nRegions & NATIONAL )
        {
            AddValueToSet( szRegionSet, NA, bComma );
            bComma = TRUE;
        }
    }
    if( bUseTypes )
    {
        bComma = FALSE;
        if( nTypes & ATT )
        {
            AddValueToSet( szTypeSet, "ATT", bComma );
            bComma = TRUE;
        }
        if( nTypes & CRW )
        {
            AddValueToSet( szTypeSet, "CRW", bComma );
            bComma = TRUE;
        }
        if( nTypes & DZR )
        {
            AddValueToSet( szTypeSet, "DZR1','DZR2','DZR3','TP12','TP34", bComma );
            bComma = TRUE;
        }
        if( nTypes & ENG )
        {
            AddValueToSet( szTypeSet, "EN12','EN34','EN56','EN70", bComma );
            bComma = TRUE;
        }
        if( nTypes & HEL )
        {
            AddValueToSet( szTypeSet, "HEL1','HEL2','HEL3", bComma );
            bComma = TRUE;
        }
        if( nTypes & HELI )
        {
            AddValueToSet( szTypeSet, "HELI", bComma );
            bComma = TRUE;
        }
        if( nTypes & SMJR )
        {
            AddValueToSet( szTypeSet, "SMJR", bComma );
            bComma = TRUE;
        }
        if( nTypes & SJAC )
        {
            AddValueToSet( szTypeSet, "SJAC", bComma );
            bComma = TRUE;
        }
        if( nTypes & WT )
        {
            AddValueToSet( szTypeSet, "WT", bComma );
            bComma = TRUE;
        }
        if( nTypes & FRBT )
        {
            AddValueToSet( szTypeSet, "FRBT", bComma );
            bComma = TRUE;
        }
        if( nTypes & SEAT || nTypes & AIR_ATTACK )
        {
            AddValueToSet( szTypeSet, "SEAT','SCP", bComma );
            bComma = TRUE;
        }
    }
}

/**
 * \brief exract a set of resource ids to be excluded from a run.  This can be
 * limited by agency and type and region.
 *
 * \param
 * \param
 * \param
 * \param
 * \param
 * \return
 */
char ** SpatialiteDataAccess::GetRescRemovalSet( int nAgencies, int nRegions,
                                                 int nTypes, int nHowMany,
                                                 int *nCount,
                                                 const char *pszIgnoreIds )
{
    if( nHowMany < 1 )
        return NULL;
    int rc;
    sqlite3_stmt *stmt = NULL;

    char szAgencySet[RESC_REM_BUF];
    char szRegionSet[RESC_REM_BUF];
    char szTypeSet[RESC_REM_BUF];
    char *pszIgnoreSet;

    szAgencySet[0] = '\0';
    szRegionSet[0] = '\0';
    szTypeSet[0] = '\0';

    SetRescRemovalMasks( nAgencies, nRegions, nTypes, szAgencySet,
                         szRegionSet, szTypeSet );
    /*
    ** Account for resources already removed.
    */
    if( pszIgnoreIds != NULL )
    {
        pszIgnoreSet = sqlite3_mprintf( "%s", pszIgnoreIds );
    }
    else
    {
        pszIgnoreSet = sqlite3_mprintf( "%s", "" );
    }
    const char *pszSql;
    pszSql = sqlite3_mprintf( "SELECT resources.id FROM resources " \
                              "LEFT JOIN fpu ON fpu_id=fpu.id " \
                              "WHERE agency IN(%s) " \
                              "AND resc_type IN (%s) " \
                              "AND substr(fpu.name, 0, 3) IN (%s) " \
                              "AND resources.id NOT IN(%s) " \
                              "ORDER BY RANDOM() " \
                              "LIMIT %d", szAgencySet, szTypeSet,
                              szRegionSet, pszIgnoreSet, nHowMany );
    rc = sqlite3_prepare_v2( db, pszSql, -1, &stmt, NULL );

    char ** papszResc;
    papszResc = (char**)calloc( sizeof( char *), nHowMany + 1 );
    int i = 0;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        papszResc[i++] = sqlite3_mprintf( "%s", sqlite3_column_text( stmt, 0 ) );
    }
    assert( i <= nHowMany );
    papszResc[i] = NULL;
    if( nCount )
    {
        *nCount = i;
    }

    sqlite3_free( (void*)pszIgnoreSet );
    sqlite3_free( (void*)pszSql );
    rc = sqlite3_finalize( stmt ); stmt = NULL;

    return papszResc;
}

int SpatialiteDataAccess::GetRescCount( int nAgencies, int nRegions,
                                        int nTypes, const char *pszIgnoreIds )
{
    int rc;
    sqlite3_stmt *stmt = NULL;

    char szAgencySet[RESC_REM_BUF];
    char szRegionSet[RESC_REM_BUF];
    char szTypeSet[RESC_REM_BUF];
    char *pszIgnoreSet;

    szAgencySet[0] = '\0';
    szRegionSet[0] = '\0';
    szTypeSet[0] = '\0';
    SetRescRemovalMasks( nAgencies, nRegions, nTypes, szAgencySet, szRegionSet, 
                         szTypeSet );
    /*
    ** Account for resources already removed.
    */
    if( pszIgnoreIds != NULL )
    {
        pszIgnoreSet = sqlite3_mprintf( "%s", pszIgnoreIds );
    }
    else
    {
        pszIgnoreSet = sqlite3_mprintf( "%s", "" );
    }
    const char *pszSql;
    pszSql = sqlite3_mprintf( "SELECT count(*) FROM resources " \
                              "LEFT JOIN fpu ON fpu_id=fpu.id " \
                              "WHERE agency IN(%s) " \
                              "AND resc_type IN (%s) " \
                              "AND substr(fpu.name, 0, 3) IN (%s) " \
                              "AND resources.id NOT IN(%s)",
                              szAgencySet, szTypeSet, szRegionSet,
                              pszIgnoreSet );
    rc = sqlite3_prepare_v2( db, pszSql, -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW )
    {
        sqlite3_free( (void*)pszSql );
        sqlite3_finalize( stmt ); stmt = NULL;
        return 0;
    }
    int nC = sqlite3_column_int( stmt, 0 );
    sqlite3_free( (void*)pszSql );
    rc = sqlite3_finalize( stmt ); stmt = NULL;
    return nC;
}

IRS_Err
SpatialiteDataAccess::CreateLargeAirtankers( std::vector<CRescType>&types,
                                             std::vector<CDispLoc>&loc,
                                             std::vector<CResource*>&resc,
                                             int nATTCount,
                                             const char * const *papszOptions )
{
    int i;
    int nType;
    int nBase;
    char szName[32];
    int bFound = FALSE;
    for( i = 0; i < types.size(); i++ )
    {
        if( types[i].GetRescType() == "ATT" )
        {
            nType = i;
            bFound = TRUE;
            break;
        }
    }
    if( !bFound )
    {
        return IRS_INVALID_INPUT;
    }
    bFound = FALSE;
    for( i = 0; i < loc.size(); i++ )
    {
        if( strstr( loc[i].GetDispLocID().c_str(), "Alamogordo_KYLE" ) )
        {
            nBase = i;
            bFound = TRUE;
            break;
        }
    }
    if( !bFound )
    {
        return IRS_INVALID_INPUT;
    }
    CResource *poR;
    for( int i = 0; i < nATTCount; i ++ )
    {

        sprintf( szName, "LAT_%d", i );

        poR = new CAirtanker( szName, types[nType], 1, "930", "2000",
                              DayOfWeek( i % 7 ), DayOfWeek( (i+5) % 7 ),
                              100, 200, loc[nBase], 100, 0, 0, 2600 );
        resc.push_back( poR );
    }
    return IRS_OK;
}


#endif /* IRS_HAVE_SPATIALITE */

