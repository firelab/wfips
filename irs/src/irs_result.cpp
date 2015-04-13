/******************************************************************************
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  IRS output file handler
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

#include "irs_result.h"

IRSResult::IRSResult()
{
    db = NULL;
    conn = NULL;
    nConnectIndex = 0;
    nConnectUsed = 0;
    nConnectMax = 0;
    bVerbose = FALSE;
    bWriteFwaGeometry = FALSE;
}

/**
 * \brief Create an IRSResult object and initialize N connections.
 *
 * While we are creating several connections, likely for parallel threads, it
 * is wise to use a critical section to write to disk.  This is because we
 * don't currently handle locking, busy etc.
 *
 * \param pszFilePath path to file to write to.
 * \par|0.0am pszOmffrData path to original db
 * \param nMaxConnects number of connections.
 * \param papszOptions key/value optional options, unused.
 * \return zero on success
 */

IRSResult::IRSResult( const char *pszFilePath, const char *pszOmffrData,
                      int nMaxConnects, int bReadOnly, int bVolatile,
                      char **papszOptions )
{
    int rc;
    pszPath = sqlite3_mprintf( "%s", pszFilePath );
    pszOmffrPath = sqlite3_mprintf( "%s", pszOmffrData );
    rc = sqlite3_open_v2( pszFilePath, &db, SQLITE_OPEN_READWRITE |
                                            SQLITE_OPEN_CREATE, NULL );
    rc = sqlite3_enable_load_extension(db, TRUE);
    char *pszErr;
    rc = sqlite3_load_extension( db, SPATIALITE_EXTENSION,
                                 SPATIALITE_INIT, &pszErr );
    if( rc != SQLITE_OK )
    {
        fprintf( stderr, "Could not load libspatialite\n" );
        if( pszErr )
        {
            fprintf( stderr, "sqlite3 error:%s\n", pszErr );
            sqlite3_free( pszErr );
        }
        //throw();
    }

    this->bVolatile = bVolatile;
    this->nConnectMax = nMaxConnects;
    if( bVolatile )
    {
        sqlite3_exec( db, "PRAGMA synchronous=off", NULL, NULL, NULL );
        sqlite3_exec( db, "PRAGMA journal_mode=off", NULL, NULL, NULL );
    }
    else if( !bReadOnly )
    {
        sqlite3_exec( db, "PRAGMA journal_mode=WAL", NULL, NULL, NULL );
    }
    rc = CreateTables();
}

/**
 * \brief create all output tables.
 *
 */
int IRSResult::CreateTables()
{
    if( CheckForTables() == 0 )
        return IRS_OK;
    int rc;
    sqlite3_stmt *stmt;
    rc = sqlite3_enable_load_extension(db, TRUE);
    rc = sqlite3_load_extension( db, SPATIALITE_EXTENSION, SPATIALITE_INIT, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM sqlite_master WHERE "
                                 "name='spatial_ref_sys'", -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW || sqlite3_column_int( stmt , 0 ) < 1)
    {
        sqlite3_exec( db, "SELECT InitSpatialMetadata()", NULL, NULL, NULL );
    }
    sqlite3_finalize( stmt );
    stmt = NULL;


    rc = sqlite3_exec( db, "CREATE TABLE IF NOT EXISTS " \
                           "irs_metadata(key TEXT PRIMARY KEY " \
                           "NOT NULL, value TEXT)", NULL, NULL, NULL );

    rc = sqlite3_exec( db, "CREATE TABLE IF NOT EXISTS resource_usage" \
                           "(resc_id TEXT PRIMARY KEY, usage REAL)",
                       NULL, NULL, NULL );

    rc = sqlite3_exec( db, "CREATE TABLE IF NOT EXISTS resc_remove"
                           "(resc_id int, idx int, unq int)",
                       NULL, NULL, NULL );

    rc = sqlite3_exec( db, "CREATE TABLE IF NOT EXISTS "
                           "fire_result(year integer, "
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
    rc = sqlite3_exec( db, "CREATE TABLE IF NOT EXISTS "
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

    rc = sqlite3_exec( db, "CREATE TABLE IF NOT EXISTS fpu_sum(fpu_code TEXT NOT NULL, " \
                                                 CONTAIN_COLUMN " INT" \
                           "                     DEFAULT 0, " \
                                                 NO_RESC_COLUMN " INT" \
                           "                     DEFAULT 0, " \
                                                 TIME_LIMIT_COLUMN " INT" \
                           "                     DEFAULT 0, " \
                                                 SIZE_LIMIT_COLUMN " INT" \
                           "                     DEFAULT 0, " \
                                                 EXHAUST_COLUMN " INT" \
                           "                     DEFAULT 0, " \
                                                 PERC_CONT_COLUMN " REAL" \
                           "                     DEFAULT 0, " \
                                                 LF_COST_COLUMN " REAL" \
                           "                     DEFAULT 0, " \
                                                 LF_ACRE_COLUMN " REAL" \
                           "                     DEFAULT 0, " \
                                                 LF_POP_COLUMN " REAL" \
                           "                     DEFAULT 0, " \
                                                 RESC_USAGE_COLUMN " REAL" \
                           "                     DEFAULT 0) ",
                       NULL, NULL, NULL );

    rc = sqlite3_exec( db, "SELECT AddGeometryColumn('fpu_sum', 'geometry', " \
                           "                         4269, " \
                           "                         'MULTIPOLYGON', 'XY')",
                           NULL, NULL, NULL );

    rc = sqlite3_exec( db, "CREATE TABLE IF NOT EXISTS gacc_sum(gacc_code TEXT NOT NULL, " \
                                                  CONTAIN_COLUMN " INT" \
                           "                      DEFAULT 0, " \
                                                  NO_RESC_COLUMN " INT" \
                           "                      DEFAULT 0, " \
                                                  TIME_LIMIT_COLUMN " INT" \
                           "                      DEFAULT 0, " \
                                                  SIZE_LIMIT_COLUMN " INT" \
                           "                      DEFAULT 0, " \
                                                  EXHAUST_COLUMN " INT" \
                           "                      DEFAULT 0, " \
                                                  PERC_CONT_COLUMN " REAL" \
                           "                      DEFAULT 0, " \
                                                  LF_COST_COLUMN " REAL" \
                           "                      DEFAULT 0, " \
                                                  LF_ACRE_COLUMN " REAL" \
                           "                      DEFAULT 0, " \
                                                  LF_POP_COLUMN " REAL" \
                           "                      DEFAULT 0, " \
                                                  RESC_USAGE_COLUMN " REAL" \
                           "                      DEFAULT 0) ",
                       NULL, NULL, NULL );

    rc = sqlite3_exec( db, "CREATE TABLE IF NOT EXISTS " \
                           "large_fire_result(year INTEGER NOT NULL, " \
                           "                  fire_num INTEGER NOT NULL, " \
                           "                  cost_acre REAL, " \
                           "                  acres REAL, " \
                           "                  pop REAL)",
                           NULL, NULL, NULL );

    rc = sqlite3_exec( db, "CREATE TABLE IF NOT EXISTS fpu_output(fpu_code TEXT, " \
                           "                        year INTEGER, " \
                                                    CONTAIN_COLUMN " INT" \
                           "                        DEFAULT 0, " \
                                                    NO_RESC_COLUMN " INT" \
                           "                        DEFAULT 0, " \
                                                    TIME_LIMIT_COLUMN " INT" \
                           "                        DEFAULT 0, " \
                                                    SIZE_LIMIT_COLUMN " INT" \
                           "                        DEFAULT 0, " \
                                                    EXHAUST_COLUMN " INT" \
                           "                        DEFAULT 0, " \
                                                    LF_COST_COLUMN " REAL" \
                           "                        DEFAULT 0, " \
                                                    LF_ACRE_COLUMN " REAL" \
                           "                        DEFAULT 0, " \
                                                    LF_POP_COLUMN " REAL" \
                           "                        DEFAULT 0) ",
                           NULL, NULL, NULL );

    SetMetadataItem( "omffr_db", pszOmffrPath );
    return IRS_OK;
}

int IRSResult::CheckForTables()
{
    int rc;
    sqlite3_stmt *stmt;
    rc  =  sqlite3_prepare_v2( db, "SELECT name from sqlite_master " \
                                   "WHERE type='table'", -1, &stmt, NULL );
    const char *papszKey[] = { "irs_metadata", "spatial_ref_sys", "fpu_output",
                               "fpu_sum", NULL };
    int anFound[4];
    const char * pszKey;
    memset( anFound, 0, 4 * sizeof( int ) );
    int i;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszKey = (const char*)sqlite3_column_text( stmt, 0 );
        i = 0;
        while( papszKey[i] != NULL )
        {
            if( strcmp( pszKey, papszKey[i] ) == 0 )
                anFound[i] = 1;
            i++;
        }
    }
    rc = sqlite3_finalize( stmt ) ;
    int anExpected[4] = { 1, 1, 1, 1 };
    return memcmp( anFound, anExpected, 4 * sizeof( int ) );
}



int IRSResult::InitializeNewDb()
{
    int rc;
    /*
    ** TODO: Should we create an index?  This is likely unnecessary until we
    ** have several values (at least hundreds), which is unlikely.
    */
    SetMetadataItem( "omffr_db", pszOmffrPath );
    return 0;
}

/**
 * \brief Get a value for a given key in the metadata table.
 * Valid keys:
 * - external_resource_db
 * - original_db
 * \param pszKey key to search for in the database table
 * \return the associated value with the key if it exists, NULL otherwise
 */
const char * IRSResult::GetMetadataItem( const char *pszKey )
{
    int rc;
    const char *pszValue;
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, "SELECT value FROM irs_metadata WHERE key=?",
                             -1, &stmt, NULL );
    rc = sqlite3_bind_text( stmt, 1, pszKey, -1, NULL );
    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW )
        return NULL;
    pszValue = strdup( (const char*)sqlite3_column_text( stmt, 0 ) );
    sqlite3_finalize( stmt );
    return pszValue;
}

int IRSResult::SetMetadataItem( const char *pszKey, const char *pszValue )
{
    int rc;
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, "INSERT OR REPLACE INTO irs_metadata "
                                 "VALUES(?, ?)", -1, &stmt, NULL );
    rc = sqlite3_bind_text( stmt, 1, pszKey, -1, NULL );
    rc = sqlite3_bind_text( stmt, 2, pszValue, -1, NULL );
    rc = sqlite3_step( stmt );
    sqlite3_finalize( stmt );
    return 0;
}

IRSResult::~IRSResult()
{
    int rc;
    rc = sqlite3_close( db );
    sqlite3_free( (void*)pszPath );
    sqlite3_free( (void*)pszOmffrPath );
}

int IRSResult::AddScenarioResults( std::vector<CResults>aoResults )
{
    return 0;
}

int IRSResult::AddResourceWorkLoad( std::vector<CRescWorkYear>aoWorkYear )
{
    return 0;
}

int IRSResult::AddSingleResourceUsage( std::map<std::string, double> oRescUsage )
{
    int rc;
    sqlite3_stmt *ustmt;
    sqlite3_stmt *istmt;
    rc = sqlite3_exec( db, "BEGIN", NULL, NULL, NULL );
    rc = sqlite3_prepare_v2( db, "UPDATE resource_usage SET usage = usage + ?2 "
                                 "WHERE resc_id=?1",
                             -1, &ustmt, NULL );
    rc = sqlite3_prepare_v2( db, "INSERT INTO resource_usage " \
                                 "VALUES(?1, ?2)", -1, &istmt, NULL );
    std::map<std::string, double>::iterator it = oRescUsage.begin();
    for( ; it != oRescUsage.end(); it++ )
    {
        rc = sqlite3_bind_text( ustmt, 1, it->first.c_str(), -1, SQLITE_TRANSIENT );
        rc = sqlite3_bind_double( ustmt, 2, it->second );
        rc = sqlite3_step( ustmt );
        rc = sqlite3_changes( db );
        if( !rc )
        {
            rc = sqlite3_bind_text( istmt, 1, it->first.c_str(), -1, SQLITE_TRANSIENT );
            rc = sqlite3_bind_double( istmt, 2, it->second );
            rc = sqlite3_step( istmt );
            if( rc != SQLITE_ROW || rc != SQLITE_OK )
            {
            /*
            ** if(rc)...
            */
            }
        }
        rc = sqlite3_reset( ustmt );
        rc = sqlite3_reset( istmt );
    }
    sqlite3_finalize( ustmt );
    sqlite3_finalize( istmt );
    rc = sqlite3_exec( db, "END", NULL, NULL, NULL );
    return 0;
}

int IRSResult::SetRemovedResourcesIds( int *panIds, int nCount )
{
    int rc;
    sqlite3_stmt *istmt;
    rc = sqlite3_prepare_v2( db, "INSERT INTO resc_remove(resc_id) "
                                 "VALUES(?)", -1, &istmt, NULL );
    rc = sqlite3_exec( db, "BEGIN", NULL, NULL, NULL );
    int i;
    int nId;
    for( i = 0; i < nCount; i++ )
    {
        rc = sqlite3_bind_int( istmt, 1, panIds[i] );
        rc = sqlite3_step( istmt );
        rc = sqlite3_reset( istmt );
    }
    rc = sqlite3_exec( db, "END", NULL, NULL, NULL );
    rc = sqlite3_finalize( istmt ); istmt = NULL;
    return 0;
}

/**
 * \brief Log the resources removed during simulation.
 *
 * This allows us to track which resources were removed before the simulation
 * was run, but we also know the original resource table.  Essentially, it's a
 * diff of the resource list.
 *
 * \param oResources vector of resources
 * \param panIndexes indexes of resources that have been removed
 * \param nCount size of panIndexes
 * \return zero on success, non-zero otherwise.
 */
int IRSResult::SetRemovedResources( std::vector<CResource*>&oResources,
                                    int  *panIndexes, int nCount )
{
    int rc;
    sqlite3_stmt *istmt;
    sqlite3 *odb;
    rc = sqlite3_exec( db, "BEGIN", NULL, NULL, NULL );
    rc = sqlite3_open_v2( pszOmffrPath, &odb, SQLITE_OPEN_READONLY, NULL );
    rc = sqlite3_prepare_v2( db, "INSERT INTO resc_remove VALUES(?, ?, ?)", -1,
                             &istmt, NULL );
    int i;
    int nId, nIndex, nUniq;
    for( i = 0; i < nCount; i++ )
    {
        /* Assumes unique id or IRS_USE_DBID_AS_RESC_NAME */
        nId = atoi( oResources[panIndexes[i]]->GetRescID().c_str() );
        rc = sqlite3_bind_int( istmt, 1, nId );
        rc = sqlite3_bind_int( istmt, 2, panIndexes[i] );
        rc = sqlite3_bind_int( istmt, 3, 0 );
        rc = sqlite3_step( istmt );
        rc = sqlite3_reset( istmt );
    }
    rc = sqlite3_exec( db, "END", NULL, NULL, NULL );
    rc = sqlite3_close( odb );
    rc = sqlite3_finalize( istmt );
    return 0;
}


int IRSResult::SetRemovedResourcesStrings( const char * const *papszIds )
{
    int rc;
    if( !papszIds )
        return IRS_OK;
    sqlite3_stmt *istmt;
    rc = sqlite3_exec( db, "BEGIN", NULL, NULL, NULL );
    rc = sqlite3_prepare_v2( db, "INSERT INTO resc_remove(resc_id) VALUES(?)", -1,
                             &istmt, NULL );
    int i = 0;
    while( papszIds[i] != NULL )
    {
        /* Assumes unique id or IRS_USE_DBID_AS_RESC_NAME */
        sqlite3_bind_text( istmt, 1, papszIds[i++], -1, NULL );
        rc = sqlite3_step( istmt );
        rc = sqlite3_reset( istmt );
    }
    rc = sqlite3_exec( db, "END", NULL, NULL, NULL );
    rc = sqlite3_finalize( istmt );
    return 0;
}

/*
** Copy last table.
*/
int IRSResult::SetRemovedResources( const char *pszLastRun )
{
    if( !pszLastRun )
        return IRS_INVALID_INPUT;
    int rc;
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, "ATTACH ? AS last", -1, &stmt, NULL );
    if( rc )
    {
        sqlite3_finalize( stmt );
        return IRS_INVALID_INPUT;
    }
    rc = sqlite3_bind_text( stmt, 1, pszLastRun, -1, NULL );
    rc = sqlite3_step( stmt );
    //if( rc != SQLITE_DONE )
    //{
        //sqlite3_finalize( stmt );
        //return IRS_INVALID_INPUT;
    //}
    sqlite3_finalize( stmt );
    sqlite3_exec( db, "INSERT INTO resc_remove(resc_id) " \
                      "SELECT resc_id FROM last.resc_remove",
                  NULL, NULL, NULL );
    sqlite3_exec( db, "DETACH last", NULL, NULL, NULL );
    return IRS_OK;
}

/**
 * \brief Get the number of resources removed for the run.
 *
 * \return number of resources removed for the run.
 */
int IRSResult::GetRemovedResourceCount()
{
    int rc;
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, "SELECT count(*) FROM resc_remove",
                            -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW )
    {
        return 0;
    }
    int nCount = sqlite3_column_int( stmt, 0 );
    rc = sqlite3_finalize( stmt );
    return nCount;
}

/**
 * \brief Get the resource indexes removed for this run.
 *
 * \param [out] panIndexes Array of indexes, in ascending order
 * \param [out] pnCount size of the array
 * \return zero on success
 */
int IRSResult::GetRemovedResourceIndexes( int *panIndexes )
{
    assert( panIndexes );
    int rc;
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, "SELECT resc_id FROM resc_remove",
                             -1, &stmt, NULL );
    int i = 0;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        panIndexes[i++] = sqlite3_column_int( stmt, 0 );
    }
    sqlite3_finalize( stmt );
    /* No results */
    if( i == 0 )
        return 1;
    return 0;
}

char ** IRSResult::GetRemovedResourceIndexesAsStrings()
{
    int rc;
    int nCount = GetRemovedResourceCount();
    if( nCount == 0 )
        return NULL;
    int *panIndexes = (int*)sqlite3_malloc( nCount * sizeof( int ) );
    rc = GetRemovedResourceIndexes( panIndexes ) ;
    if( rc )
        return NULL;
    char ** papszIds = (char**)sqlite3_malloc( nCount * sizeof( char *) + 1 );
    int i = 0;
    for( i = 0; i < nCount; i++ )
        papszIds[i] = sqlite3_mprintf( "%d", panIndexes[i] );
    papszIds[i] = NULL;
    return papszIds;
}

/**
 * \brief Get a list of the 'lowest' performing resources from the last run
 *
 * \note Result must be freed with DestroyUnderPerformersList()
 *
 * Get n of the lowest performing resources as calculated by:
 *
 * \f$\frac{minutes worked}{julian start - julian end}\f$
 *
 * \param dfRemoveCount fraction (if < 1.0) or number (if > 1.0) to remove.
 * \param bRemoveUnused if true, remove all resources that were not dispatched.
 *                      dfRemoveCount is ignored if this is set.
 * \return a list of resource ids, to be freed by DestroyUnderPerformersList()
 */
char ** IRSResult::GetUnderPerformers( double dfRemoveCount, int bRemoveUnused )
{
    assert( dfRemoveCount >= 0 );
    int rc;
    sqlite3_stmt *stmt;
    int nCount;
    if( dfRemoveCount < 1.0 )
    {
        rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM resource_usage",
                                 -1, &stmt, NULL );
        rc = sqlite3_step( stmt );
        if( rc != SQLITE_ROW )
        {
            sqlite3_finalize( stmt );
            return NULL;
        }
        nCount = sqlite3_column_int( stmt, 0 );
        nCount = dfRemoveCount * nCount;
        sqlite3_finalize( stmt );
    }
    else
    {
        nCount = dfRemoveCount;
    }
    if( bRemoveUnused )
    {
        rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM resource_usage " \
                                     "WHERE usage=0.0", -1, &stmt, NULL );
        rc = sqlite3_step( stmt );
        if( rc != SQLITE_ROW )
        {
            sqlite3_finalize( stmt );
            return NULL;
        }
        nCount = sqlite3_column_int( stmt, 0 );
        sqlite3_finalize( stmt );
        rc = sqlite3_prepare_v2( db, "SELECT resc_id FROM resource_usage " \
                                     "WHERE usage=0.0", -1, &stmt, NULL );
    }
    else
    {
        rc = sqlite3_prepare_v2( db, "SELECT resc_id FROM resource_usage " \
                                     "WHERE resc_id NOT IN (SELECT resc_id " \
                                     "FROM resc_remove) " \
                                     "ORDER BY usage LIMIT ?", -1, &stmt,
                                 NULL );
        rc = sqlite3_bind_int( stmt, 1, nCount );
    }
    int nStepCount = 0;
    const char *pszResourceId;
    char **papszResourceId = (char**)sqlite3_malloc( sizeof( char * ) * nCount + 1 );
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszResourceId = (const char *)sqlite3_column_text( stmt, 0 );
        papszResourceId[nStepCount++] = sqlite3_mprintf( "%s", pszResourceId );
        assert( nStepCount < nCount + 1 );
    }
    papszResourceId[nStepCount] = NULL;
    sqlite3_finalize( stmt );
    return papszResourceId;
}

/**
 * \brief Free results of GetUnderPerformers()
 *
 * \param pszList list to be freed
 * \return zero on success, non-zero otherwise
 */
int IRSResult::DestroyUnderPerformersList( char **pszList )
{
    return FreeStringList( pszList );
}

int IRSResult::ExportFullFireResults( int nYear, std::vector<CResults>oResults,
                                      std::vector<int>oRescUsage )
{
    sqlite3_stmt *stmt = NULL;
    sqlite3_stmt *usg_stmt = NULL;
    int rc = 0;

    //Year,Julian Day,Fire #,# Fires/Day,Day of Week,Discovery Time,BI,ROS,FM,IsWalkIn,Distance,L to W Ratio,FWA,Arrival Time,Arrival Size, Status
    //rc = sqlite3_exec( db, "CREATE INDEX IF NOT EXISTS idx_day_num_year ON fire_result(year, jul_day, fire_num)",
    //                   NULL, NULL, NULL );
    rc = sqlite3_prepare_v2( db, "INSERT INTO fire_result(year, jul_day, "
                                 "fire_num, arrtime, arrsize, finalsize, "
                                 "finaltime, run_contain, treated, status) "
                                 "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", -1,
                             &stmt, NULL );
    rc = sqlite3_prepare_v2( db, "INSERT INTO resc_usage(year, att, crw, dzr, " \
                                        "eng, fbdz, frbt, helicopter, heli, " \
                                        "scpseat, sjac, smjr, tp, wt) VALUES "\
                                        "(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
                             -1, &usg_stmt, NULL );

    rc = sqlite3_exec( db, "BEGIN", NULL, NULL, NULL );

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
        rc = sqlite3_bind_int( stmt, 8, bRunContain );
        rc = sqlite3_bind_int( stmt, 9, bTreated );
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
    rc = sqlite3_exec( db, "END", NULL, NULL, NULL );
    rc = sqlite3_finalize( stmt );
    rc = sqlite3_finalize( usg_stmt );
    stmt = usg_stmt = NULL;
    return 0;
}

/**
 * \brief Add geometries for admin boundaries to the output for display
 *
 * TODO: ALL
 *
 * return zero on success, non-zero otherwise
 */
int IRSResult::AddGeometries( int nGeomMask )
{
    assert( db );
    if( !nGeomMask & FWA_GEOMETRY &&
        !nGeomMask & FPU_GEOMETRY &&
        !nGeomMask & GACC_GEOMETRY )
    {
        return IRS_INVALID_INPUT;
    }
    int rc;
    sqlite3_stmt *stmt;
    assert( db );
    rc = sqlite3_prepare_v2( db, "ATTACH ? AS omffr", -1, &stmt, NULL );
    rc = sqlite3_bind_text( stmt, 1, pszOmffrPath, -1, NULL );
    rc = sqlite3_step( stmt );
    sqlite3_reset( stmt );
    sqlite3_finalize( stmt );
    rc = sqlite3_enable_load_extension( db, TRUE );
    if( nGeomMask & FPU_GEOMETRY )
    {
        rc = sqlite3_exec( db, "CREATE TABLE fpu(fpu_code TEXT PRIMARY KEY)",
                           NULL, NULL, NULL );
        rc = sqlite3_exec( db, "SELECT AddGeometryColumn('fpu', 'geometry', 4269, " \
                                                   "'MULTIPOLYGON', 'XY')",
                           NULL, NULL, NULL );
        rc = sqlite3_prepare_v2( db, "INSERT INTO fpu SELECT fpu_code, geometry " \
                                     "FROM fpu_bndry", -1, &stmt, NULL );
        rc = sqlite3_step( stmt );
    }
    sqlite3_finalize( stmt );
    rc = sqlite3_prepare_v2( db, "DETACH omffr", -1, &stmt, NULL );
    return 0;
}

/**
 * \brief Export summary data to and assign geometries for mapping.
 *
 * Use the fpu_output table.
 *
 */
int IRSResult::ExportFpuSummary( IRSProgress pfnProgress )
{
    int rc;
    sqlite3_stmt *stmt = NULL;
    sqlite3_stmt *istmt = NULL;

    rc = sqlite3_enable_load_extension( db, TRUE );
    rc = sqlite3_prepare_v2( db, "ATTACH ? AS omffr", -1, &stmt, NULL );
    rc = sqlite3_bind_text( stmt, 1, pszOmffrPath, -1, NULL );
    rc = sqlite3_step( stmt );
    sqlite3_finalize( stmt );
    stmt = NULL;
    rc = sqlite3_exec( db, "PRAGMA journal_mode=OFF", NULL, NULL, NULL );
    rc = sqlite3_exec( db, "BEGIN", NULL, NULL, NULL );

    rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM sqlite_master WHERE "
                                 "name='spatial_ref_sys'", -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW || sqlite3_column_int( stmt , 0 ) < 1)
    {
        sqlite3_exec( db, "SELECT InitSpatialMetadata()", NULL, NULL, NULL );
    }
    sqlite3_finalize( stmt );
    stmt = NULL;

    rc = sqlite3_exec( db, "END", NULL, NULL, NULL );
    rc = sqlite3_exec( db, "PRAGMA journal_mode=DELETE", NULL, NULL, NULL );

    /*
    ** Check for fpu_output, if it isn't there, create it.
    */
    rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM sqlite_master WHERE "
                                 "name='fpu_output'", -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW || sqlite3_column_int( stmt , 0 ) < 1)
    {
        rc = SummarizeByFpu( pfnProgress );
    }
    sqlite3_finalize( stmt );
    stmt = NULL;

    rc = sqlite3_prepare_v2( db, "INSERT INTO fpu_sum(fpu_code, " \
                                                      CONTAIN_COLUMN"," \
                                                      NO_RESC_COLUMN"," \
                                                      TIME_LIMIT_COLUMN"," \
                                                      SIZE_LIMIT_COLUMN"," \
                                                      EXHAUST_COLUMN"," \
                                                      PERC_CONT_COLUMN"," \
                                                      LF_COST_COLUMN"," \
                                                      LF_ACRE_COLUMN"," \
                                                      LF_POP_COLUMN"," \
                                                      "geometry)" \
                                 "SELECT fpu_code, " \
                                 "       SUM("CONTAIN_COLUMN")," \
                                 "       SUM("NO_RESC_COLUMN")," \
                                 "       SUM("TIME_LIMIT_COLUMN")," \
                                 "       SUM("SIZE_LIMIT_COLUMN")," \
                                 "       SUM("EXHAUST_COLUMN")," \
                                 "       SUM("CONTAIN_COLUMN") / " \
                                 "       CAST(SUM("CONTAIN_COLUMN") + " \
                                 "       SUM("NO_RESC_COLUMN") + " \
                                 "       SUM("TIME_LIMIT_COLUMN") + " \
                                 "       SUM("SIZE_LIMIT_COLUMN") + " \
                                 "       SUM("EXHAUST_COLUMN") AS REAL) AS " \
                                         PERC_CONT_COLUMN", " \
                                 "       SUM("LF_COST_COLUMN")," \
                                 "       SUM("LF_ACRE_COLUMN")," \
                                 "       SUM("LF_POP_COLUMN")," \
                                 "       geometry FROM fpu_output LEFT JOIN " \
                                 "fpu_bndry USING(fpu_code) GROUP BY " \
                                 "fpu_code",
                             -1, &stmt, NULL );

    rc = sqlite3_exec( db, "BEGIN", NULL, NULL, NULL );
    rc = sqlite3_step( stmt );
    rc = sqlite3_exec( db, "END", NULL, NULL, NULL );
    sqlite3_finalize( stmt ); stmt = NULL;

    /*
    ** Resource Usage is separate, run through and update the table with a
    ** query.
    */
    rc = sqlite3_prepare_v2( db, "SELECT fpu.name, SUM(usage * " \
                                 "(end_season - start_season)) " \
                                 "FROM resource_usage JOIN resources " \
                                 "ON resource_usage.resc_id=resources.id " \
                                 "JOIN fpu ON resources.fpu_id=fpu.id " \
                                 "GROUP BY fpu.name", -1, &stmt, NULL );
    rc = sqlite3_prepare_v2( db, "UPDATE fpu_sum SET " \
                                 RESC_USAGE_COLUMN"=? WHERE fpu_code=?", -1,
                             &istmt, NULL );
    double dfUsage;
    const char *pszFpu;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszFpu = (const char*)sqlite3_column_text( stmt, 0 );
        dfUsage = sqlite3_column_double( stmt, 1 );
        rc = sqlite3_bind_double( istmt, 1, dfUsage );
        rc = sqlite3_bind_text( istmt, 2, pszFpu, -1, NULL );
        rc = sqlite3_step( istmt );
        rc = sqlite3_reset( istmt );
    }
    rc = sqlite3_finalize( stmt ); stmt = NULL;
    rc = sqlite3_finalize( istmt ); istmt = NULL;

    rc = sqlite3_exec( db, "SELECT CreateSpatialIndex('fpu_sum', 'geometry')",
                       NULL, NULL, NULL );

    rc = ExportGaccSummary();

    rc = sqlite3_exec( db, "DETACH omffr", NULL, NULL, NULL );
    return IRS_OK;
}

/**
 * \brief Create a summary for the GACC level
 *
 */
int IRSResult::ExportGaccSummary()
{
    /*
    ** XXX: We don't attach or load spatialite, because this is called
    ** XXX: internally.
    */
    int rc;
    sqlite3_stmt *stmt;

    rc = sqlite3_exec( db, "SELECT AddGeometryColumn('gacc_sum', 'geometry', " \
                           "                         4269, " \
                           "                         'MULTIPOLYGON', 'XY')",
                           NULL, NULL, NULL );

    rc = sqlite3_prepare_v2( db, "INSERT INTO gacc_sum(gacc_code, " \
                                                       CONTAIN_COLUMN"," \
                                                       NO_RESC_COLUMN"," \
                                                       TIME_LIMIT_COLUMN"," \
                                                       SIZE_LIMIT_COLUMN"," \
                                                       EXHAUST_COLUMN"," \
                                                       PERC_CONT_COLUMN"," \
                                                       LF_COST_COLUMN"," \
                                                       LF_ACRE_COLUMN"," \
                                                       LF_POP_COLUMN"," \
                                                       RESC_USAGE_COLUMN"," \
                                                       "geometry)" \
                                 "SELECT distinct(substr(fpu_code, 0, 3)) " \
                                 "       AS gacc_code, " \
                                 "       SUM("CONTAIN_COLUMN")," \
                                 "       SUM("NO_RESC_COLUMN")," \
                                 "       SUM("TIME_LIMIT_COLUMN")," \
                                 "       SUM("SIZE_LIMIT_COLUMN")," \
                                 "       SUM("EXHAUST_COLUMN")," \
                                 "       SUM("CONTAIN_COLUMN") / " \
                                 "       CAST(SUM("CONTAIN_COLUMN") + " \
                                 "       SUM("NO_RESC_COLUMN") + " \
                                 "       SUM("TIME_LIMIT_COLUMN") + " \
                                 "       SUM("SIZE_LIMIT_COLUMN") + " \
                                 "       SUM("EXHAUST_COLUMN") AS REAL) AS " \
                                         PERC_CONT_COLUMN", " \
                                 "       SUM("LF_COST_COLUMN")," \
                                 "       SUM("LF_ACRE_COLUMN")," \
                                 "       SUM("LF_POP_COLUMN")," \
                                 "       SUM("RESC_USAGE_COLUMN")," \
                                 "       CastToMultiPolygon( " \
                                 "       ST_Union(gacc_bndry.geometry)) " \
                                 "       FROM fpu_sum LEFT JOIN gacc_bndry " \
                                 "       ON gacc_code=ga_abbr " \
                                 "       GROUP BY gacc_code",
                             -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    rc = sqlite3_finalize( stmt ); stmt = NULL;

    /*
    ** FIXME: We have to update the table because our union is wrong.  This
    ** should be done in the original sql though.
    */
    rc = sqlite3_prepare_v2( db, "UPDATE gacc_sum SET " \
                             CONTAIN_COLUMN"="CONTAIN_COLUMN \
                             " / NumGeometries(geometry), " \
                             NO_RESC_COLUMN"="NO_RESC_COLUMN \
                             " / NumGeometries(geometry), " \
                             TIME_LIMIT_COLUMN"="TIME_LIMIT_COLUMN \
                             " / NumGeometries(geometry), " \
                             SIZE_LIMIT_COLUMN"="SIZE_LIMIT_COLUMN \
                             " / NumGeometries(geometry), " \
                             EXHAUST_COLUMN"="EXHAUST_COLUMN
                             " / NumGeometries(geometry), " \
                             CONTAIN_COLUMN"="CONTAIN_COLUMN  \
                             " / NumGeometries(geometry), " \
                             LF_COST_COLUMN"="LF_COST_COLUMN \
                             " / NumGeometries(geometry), " \
                             LF_ACRE_COLUMN"="LF_ACRE_COLUMN \
                             " / NumGeometries(geometry), " \
                             LF_POP_COLUMN"="LF_POP_COLUMN \
                             " / NumGeometries(geometry), " \
                             RESC_USAGE_COLUMN"="RESC_USAGE_COLUMN \
                             " / NumGeometries(geometry)",
                       -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    sqlite3_finalize( stmt ); stmt = NULL;
    rc = sqlite3_exec( db, "SELECT CreateSpatialIndex('gacc_sum', 'geometry')",
                       NULL, NULL, NULL );

    /*
    ** Test our query and make sure we got the right large fire results.
    */
#ifndef NDEBUG
    sqlite3_stmt *fstmt;
    sqlite3_stmt *gstmt;
    rc = sqlite3_prepare_v2( db, "SELECT substr(fpu_code, 0, 3) as gacc, " \
                                 "SUM(lf_cost) FROM fpu_sum " \
                                 "GROUP BY gacc ORDER BY gacc",
                             -1, &fstmt, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT gacc_code, lf_cost FROM gacc_sum " \
                                 "GROUP BY gacc_code ORDER BY gacc_code",
                             -1, &gstmt, NULL );
    const char *pszFpu, *pszGacc;
    int nFpu, nGacc;
    while( sqlite3_step( fstmt ) == SQLITE_ROW &&
           sqlite3_step( gstmt ) == SQLITE_ROW )
    {
        pszFpu = (const char*)sqlite3_column_text( fstmt, 0 );
        pszGacc = (const char*)sqlite3_column_text( gstmt, 0 );
        nFpu = sqlite3_column_int( fstmt, 1 );
        nGacc = sqlite3_column_int( gstmt, 1 );
        printf( "FPU: %s == %d\n", pszFpu, nFpu );
        printf( "GACC: %s == %d\n", pszGacc, nGacc );
        assert( EQUAL( pszFpu, pszGacc ) );
        assert( abs( nFpu - nGacc ) <= 1 );
    }
    sqlite3_finalize( fstmt ); fstmt = NULL;
    sqlite3_finalize( gstmt ); gstmt = NULL;
#endif /* NDEBUG */

    return IRS_OK;
}


/**
 * \brief Export summary data using OGR
 *
 * Export simulation result data to a valid OGR format.  Data may be grouped by
 * region, fpu or fwa.  Currently, use only one geometry, although the use of a
 * bitmask implies many may be used.  This function uses OGR as much as
 * possible to handle data.
 *
 * \param nGeomMask geometry to assign attributes to
 * \param nAttributes attributes to write
 * \param pszFormat Supported OGR format, NULL -> 'Esri Shapefile'
 * \param pszFilename Output file path
 */
int IRSResult::ExportToOgr( int nGeomMask, int nAttributes,
                            const char *pszFormat, const char *pszFilename,
                            IRSProgress pfnProgress )
{
    if( !nGeomMask || !nAttributes )
    {
        return IRS_INVALID_INPUT;
    }
    /* Don't support yet */
    if( nGeomMask & GACC_GEOMETRY || nGeomMask & FWA_GEOMETRY )
    {
        return IRS_INVALID_INPUT;
    }

    int bWriteToSelf = !pszFilename;
    int bWriteLargeFire = nAttributes & LARGE_FIRE;

    int rc;
    sqlite3_stmt *stmt = NULL;
    sqlite3_stmt *rstmt = NULL;
    sqlite3_stmt *lfstmt = NULL;

    rc = sqlite3_enable_load_extension( db, TRUE );
    rc = sqlite3_exec( db, "PRAGMA journal_mode=OFF", NULL, NULL, NULL );
    rc = sqlite3_exec( db, "BEGIN", NULL, NULL, NULL );

    rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM sqlite_master WHERE "
                                 "name='spatial_ref_sys'", -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW || sqlite3_column_int( stmt , 0 ) < 1)
        sqlite3_exec( db, "SELECT InitSpatialMetadata()", NULL, NULL, NULL );
    sqlite3_finalize( stmt );
    stmt = NULL;
    rc = sqlite3_exec( db, "END", NULL, NULL, NULL );
    rc = sqlite3_exec( db, "PRAGMA journal_mode=DELETE", NULL, NULL, NULL );



    OGRSFDriverH hDriver;
    OGRDataSourceH hDS;
    OGRLayerH hLayer;

    if( !bWriteToSelf )
    {
        hDriver = OGRGetDriverByName( pszFormat );
        if( hDriver == NULL )
        {
            return IRS_INVALID_INPUT;
        }

        hDS = OGR_Dr_CreateDataSource( hDriver, pszFilename, NULL );
        if( hDS == NULL )
        {
            return IRS_INVALID_INPUT;
        }
    }
    else
    {
        hDS = OGROpen( pszPath, TRUE, NULL );
        CPLSetConfigOption( "OGR_SQLITE_PRAGMA", "journal_mode=WAL" );
    }
    /*
    ** We use WAL mode because we are reading and writing to the same db.  See
    ** CPLSetConfigOption above as well.
    */
    /* FIXME: Use OGR for everything. */
    rc = sqlite3_exec( db, "PRAGMA journal_mode=WAL", NULL, NULL, NULL );
    rc = sqlite3_exec( db, "CREATE INDEX IF NOT EXISTS idx_fire_num ON " \
                           "fire_result(year, fire_num)", NULL, NULL , NULL );

    rc = sqlite3_prepare_v2( db, "ATTACH ? AS omffr", -1, &stmt, NULL );
    rc = sqlite3_bind_text( stmt, 1, pszOmffrPath, -1, NULL );
    rc = sqlite3_step( stmt );
    sqlite3_reset( stmt );
    /*
    ** Create our output layer with MULTIPOLYGON geometry and a set of
    ** attributes.
    */
    OGRSpatialReferenceH hSRS = NULL;
    hSRS = OSRNewSpatialReference( NULL );
    OSRImportFromEPSG( hSRS, 4269 );
    char **papszOptions = NULL;
    papszOptions = CSLAddString( papszOptions, "OVERWRITE=YES" );
    hLayer = OGR_DS_CreateLayer( hDS, "irs_output", hSRS, wkbMultiPolygon,
                                 papszOptions );

    if( !hLayer )
    {
        return 1;
    }

    OGRFieldDefnH hFieldDefn;
    hFieldDefn = OGR_Fld_Create( "fpu_code", OFTString );
    OGR_Fld_SetWidth( hFieldDefn, 10 );
    OGR_L_CreateField( hLayer, hFieldDefn, TRUE );
    OGR_Fld_Destroy( hFieldDefn );
    if( nAttributes & FIRE_OUTCOME )
    {
        hFieldDefn = OGR_Fld_Create( "size_lim", OFTInteger );
        OGR_L_CreateField( hLayer, hFieldDefn, TRUE );
        OGR_Fld_Destroy( hFieldDefn );
        hFieldDefn = OGR_Fld_Create( "time_lim", OFTInteger );
        OGR_L_CreateField( hLayer, hFieldDefn, TRUE );
        OGR_Fld_Destroy( hFieldDefn );
        hFieldDefn = OGR_Fld_Create( "contain", OFTInteger );
        OGR_L_CreateField( hLayer, hFieldDefn, TRUE );
        OGR_Fld_Destroy( hFieldDefn );
        hFieldDefn = OGR_Fld_Create( "exhaust", OFTInteger );
        OGR_L_CreateField( hLayer, hFieldDefn, TRUE );
        OGR_Fld_Destroy( hFieldDefn );
        hFieldDefn = OGR_Fld_Create( "no_resc", OFTInteger );
        OGR_L_CreateField( hLayer, hFieldDefn, TRUE );
        OGR_Fld_Destroy( hFieldDefn );
        hFieldDefn = OGR_Fld_Create( "perc_esc", OFTReal );
        OGR_L_CreateField( hLayer, hFieldDefn, TRUE );
        OGR_Fld_Destroy( hFieldDefn );
    }
    if( nAttributes & RESC_USAGE )
    {
        hFieldDefn = OGR_Fld_Create( "usage", OFTInteger );
        OGR_L_CreateField( hLayer, hFieldDefn, TRUE );
        OGR_Fld_Destroy( hFieldDefn );
        rc = sqlite3_prepare_v2( db, "SELECT SUM(usage * " \
                                     "(end_season - start_season)) " \
                                     "FROM resource_usage JOIN resources " \
                                     "ON resource_usage.resc_id=resources.id " \
                                     "JOIN fpu ON resources.fpu_id=fpu.id " \
                                     "WHERE fpu.name=?", -1, &rstmt, NULL );
    }

    if( bWriteLargeFire )
    {
        rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM sqlite_master " \
                                     "WHERE name='large_fire'", -1, &lfstmt, 
                                     NULL );
        if( sqlite3_step( lfstmt ) != SQLITE_ROW )
        {
            bWriteLargeFire = FALSE;
            sqlite3_finalize( lfstmt );
            lfstmt = NULL;
        }
        sqlite3_reset( lfstmt );
        if( bWriteLargeFire )
        {
            hFieldDefn = OGR_Fld_Create( "lf_cost_total", OFTReal );
            OGR_L_CreateField( hLayer, hFieldDefn, TRUE );
            OGR_Fld_Destroy( hFieldDefn );
            hFieldDefn = OGR_Fld_Create( "lf_acres", OFTReal );
            OGR_L_CreateField( hLayer, hFieldDefn, TRUE );
            OGR_Fld_Destroy( hFieldDefn );
            hFieldDefn = OGR_Fld_Create( "lf_pop", OFTReal );
            OGR_L_CreateField( hLayer, hFieldDefn, TRUE );
            OGR_Fld_Destroy( hFieldDefn );

#ifndef IRS_RESULT_USE_SPATIAL
            rc = sqlite3_prepare_v2( db, "SELECT SUM(cost_acre*acres), " \
                                         "SUM(pop) FROM large_fire_result JOIN " \
                                         "scenario USING(year, fire_num) " \
                                         "JOIN fwa ON scenario.fwa_id=fwa.name " \
                                         "JOIN fpu on fwa.fpu_id=fpu.id " \
                                         "JOIN fire_season ON " \
                                         "fire_season.fpu_id=fpu.id " \
                                         "WHERE fpu.name=? " \
                                         "AND scenario.jul_day > " \
                                         "fire_season.start_day " \
                                         "AND scenario.jul_day < " \
                                         "fire_season.end_day " \
                                         "GROUP BY fpu.name",
                                    -1, &lfstmt, NULL );
#else
            rc = sqlite3_prepare_v2( db, "SELECT SUM(cost_acre*acres), " \
                                         "SUM(pop) FROM large_fire_result JOIN " \
                                         "scenario USING(year, fire_num) " \
                                         "JOIN fpu_bndry ON " \
                                         "ST_Contains(fpu_bndry.geometry, scenario.geometry) " \
                                         "JOIN fpu ON fpu.name=fpu_bndry.fpu_code "
                                         "JOIN fire_season on fire_season.fpu_id=fpu.id " \
                                         "WHERE fire_result.jul_day > fire_season.start_day " \
                                         "AND fire_result.jul_day < fire_season.end_day " \
                                         "AND scenario.ROWID IN(SELECT pkid FROM " \
                                         "idx_scenario_geometry WHERE " \
                                         "xmin <= MbrMaxX(fpu_bndry.geometry) AND " \
                                         "xmax >= MbrMinX(fpu_bndry.geometry) AND " \
                                         "ymin <= MbrMaxY(fpu_bndry.geometry) AND " \
                                         "ymax >= MbrMinY(fpu_bndry.geometry))" \
                                         "GROUP BY fpu_code", -1, &lfstmt, NULL );
#endif /* IRS_RESULT_USE_SPATIAL */
        }
    }

    sqlite3_stmt *gstmt;
    rc = sqlite3_prepare_v2( db, "SELECT AsText(geometry) FROM fpu WHERE name=?",
                             -1, &gstmt, NULL );
#ifndef IRS_RESULT_USE_SPATIAL
    rc = sqlite3_prepare_v2( db, "SELECT fpu.name, " \
                                 "status, COUNT(*) FROM fire_result " \
                                 "JOIN scenario USING(year, fire_num) " \
                                 "JOIN fwa ON scenario.fwa_id=fwa.name " \
                                 "JOIN fpu on fwa.fpu_id=fpu.id " \
                                 "JOIN fire_season on fire_season.fpu_id=fpu.id " \
                                 "WHERE fire_result.jul_day > fire_season.start_day " \
                                 "AND fire_result.jul_day < fire_season.end_day " \
                                 "GROUP BY fpu.name, status",
                             -1, &stmt, NULL );
#else
    rc = sqlite3_prepare_v2( db, "SELECT fpu_code, " \
                                 "status, COUNT(*) FROM fire_result " \
                                 "JOIN scenario USING(year, fire_num) " \
                                 "JOIN fpu_bndry ON " \
                                 "ST_Contains(fpu_bndry.geometry, scenario.geometry) " \
                                 "JOIN fpu ON fpu.name=fpu_bndry.fpu_code "
                                 "JOIN fire_season on fire_season.fpu_id=fpu.id " \
                                 "WHERE fire_result.jul_day > fire_season.start_day " \
                                 "AND fire_result.jul_day < fire_season.end_day " \
                                 "AND scenario.ROWID IN(SELECT pkid FROM " \
                                 "idx_scenario_geometry WHERE " \
                                 "xmin <= MbrMaxX(fpu_bndry.geometry) AND " \
                                 "xmax >= MbrMinX(fpu_bndry.geometry) AND " \
                                 "ymin <= MbrMaxY(fpu_bndry.geometry) AND " \
                                 "ymax >= MbrMinY(fpu_bndry.geometry))" \
                                 "GROUP BY fpu_code, status",
                             -1, &stmt, NULL );
#endif /* IRS_RESULT_USE_SPATIAL */
    const char *pszFpu = NULL;
    char *pszGeometry = NULL;
    char szTemp[128];
    sprintf( szTemp, "KYLE" );
    const char *pszStatus = NULL;
    int nEscape, nSizeLimit, nTimeLimit, nContain, nNoResc, nExhaust;
    int nUsage;
    int nValue;
    OGRFeatureH hFeature = NULL;
    OGRGeometryH hGeometry;
    double dfPerc;
    double dfAcres, dfCostAcre, dfPop;
    int bFirstTime = TRUE;
    int nAtLeastOne = 0;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszFpu = (const char*)sqlite3_column_text( stmt, 0 );
        if( ( !EQUAL( pszFpu, szTemp ) || bFirstTime ) || !nAtLeastOne )
        {
            if( hFeature )
            {
                OGR_F_SetFieldInteger( hFeature,
                                       OGR_F_GetFieldIndex( hFeature,
                                                            "size_lim" ),
                                       nSizeLimit );
                OGR_F_SetFieldInteger( hFeature,
                                       OGR_F_GetFieldIndex( hFeature,
                                                            "time_lim" ),
                                       nTimeLimit );
                OGR_F_SetFieldInteger( hFeature,
                                       OGR_F_GetFieldIndex( hFeature,
                                                            "contain" ),
                                       nContain );
                OGR_F_SetFieldInteger( hFeature,
                                       OGR_F_GetFieldIndex( hFeature,
                                                            "exhaust" ),
                                       nExhaust );
                OGR_F_SetFieldInteger( hFeature,
                                       OGR_F_GetFieldIndex( hFeature,
                                                            "no_resc" ),
                                       nNoResc );
                dfPerc = nSizeLimit + nTimeLimit + nExhaust + nNoResc;
                dfPerc = dfPerc / ( dfPerc + nContain );
                OGR_F_SetFieldDouble( hFeature,
                                      OGR_F_GetFieldIndex( hFeature,
                                                           "perc_esc" ),
                                       dfPerc );
                OGR_L_CreateFeature( hLayer, hFeature );
                OGR_F_Destroy( hFeature );
                nAtLeastOne = 1;
            }
            /*
            ** Reset the feature crap
            */
            nEscape = 0;
            nSizeLimit = 0;
            nTimeLimit = 0;
            nContain = 0;
            nNoResc = 0;
            nExhaust = 0;

            rc = sqlite3_bind_text( gstmt, 1, pszFpu, -1, NULL );
            rc = sqlite3_step( gstmt );
            pszGeometry = strdup((char*)sqlite3_column_text( gstmt, 0 ));
            hFeature = OGR_F_Create( OGR_L_GetLayerDefn( hLayer ) );
            OGR_F_SetFieldString( hFeature,
                                  OGR_F_GetFieldIndex( hFeature, "fpu_code" ),
                                  pszFpu );
            if( nAttributes & RESC_USAGE )
            {
                sqlite3_bind_text( rstmt, 1, pszFpu, -1, NULL );
                rc = sqlite3_step( rstmt );
                nUsage = sqlite3_column_int( rstmt, 0 );
                OGR_F_SetFieldInteger( hFeature,
                                       OGR_F_GetFieldIndex( hFeature,
                                                            "usage" ),
                                       nUsage );
                sqlite3_reset( rstmt );
            }
            if( bWriteLargeFire )
            {
                rc = sqlite3_bind_text( lfstmt, 1, pszFpu, -1, NULL );
                rc = sqlite3_step( lfstmt );
                if( rc == SQLITE_ROW )
                {
                    dfCostAcre = sqlite3_column_double( lfstmt, 0 );
                    dfAcres = sqlite3_column_double( lfstmt, 1 );
                    dfPop = sqlite3_column_double( lfstmt, 2 );
                    OGR_F_SetFieldDouble( hFeature,
                                          OGR_F_GetFieldIndex( hFeature,
                                                               "lf_cost_acre" ),
                                          dfCostAcre );
                    OGR_F_SetFieldDouble( hFeature,
                                          OGR_F_GetFieldIndex( hFeature,
                                                               "lf_acres" ),
                                          dfAcres );
                    OGR_F_SetFieldDouble( hFeature,
                                          OGR_F_GetFieldIndex( hFeature,
                                                               "lf_pop" ),
                                          dfPop );
                }
                sqlite3_reset( lfstmt );
            }
            rc = OGR_G_CreateFromWkt( &pszGeometry, hSRS, &hGeometry );
            rc = OGR_F_SetGeometry( hFeature, hGeometry );
            OGR_G_DestroyGeometry( hGeometry );
            sqlite3_reset( gstmt );
            bFirstTime = FALSE;
        }
        sprintf( szTemp, "%s", pszFpu );
        pszStatus = (const char*)sqlite3_column_text( stmt, 1 );
        int nValue = sqlite3_column_int( stmt, 2 );
        if(EQUAL(pszStatus, "Contained")) \
            nContain+=nValue;
        else if(EQUAL(pszStatus, "Exhausted")) \
            nExhaust+=nValue;
        else if(EQUAL(pszStatus, "No Resources Sent")) \
            nNoResc+=nValue;
        else if(EQUAL(pszStatus, "SizeLimitExceeded")) \
            nSizeLimit+=nValue;
        else if(EQUAL(pszStatus, "TimeLimitExceeded")) \
            nTimeLimit+=nValue;
    }

    /* Aggregate GACC data */
    rc = sqlite3_exec( db, "CREATE TABLE gacc_result(gacc text, " \
                           "contained integer, escaped integer, " \
                           "perc_escape real, lf_cost real, " \
                           "lf_acre real, lf_pop real)", NULL, NULL, NULL );
    rc = sqlite3_exec( db, "SELECT AddGeometryColumn('gacc_result', " \
                           "'geometry', 4269, 'MULTIPOLYGON', 'XY')", NULL, 
                       NULL, NULL );
    rc = sqlite3_exec( db, "INSERT INTO gacc_result(gacc, contained, escaped, " \
                           "geometry) SELECT substr(fpu_code, 0, 3) as gacc, " \
                           "SUM(contain) as contained, SUM(size_lim) + " \
                           "SUM(time_lim) + SUM(exhaust) + SUM(no_resc) " \
                           "as escaped, CastToMultiPolygon( " \
                           "ST_Union(irs_output.geometry)) as geometry " \
                           "FROM irs_output WHERE gacc=gacc " \
                           "GROUP BY gacc", NULL, NULL, NULL );

    sqlite3_finalize( stmt );
    sqlite3_finalize( gstmt );
    sqlite3_finalize( rstmt );
    sqlite3_exec( db, "DETACH omffr", NULL, NULL , NULL );

    OGR_DS_Destroy( hDS );
    /*
    ** Return to non-WAL.
    */
    sqlite3_exec( db, "PRAGMA journal_mode=DELETE", NULL, NULL, NULL );
    CPLSetConfigOption( "OGR_SQLITE_PRAGMA", "journal_mode=DELETE" );

    return IRS_OK;
}


typedef struct _LargeFireData
{
    double dfCostAcre;
    double dfSize;
    double dfPop;
    int bExcluded;
} LargeFireData;

static int CompareLargeFireSize( const void *a, const void *b )
{
    if( ((LargeFireData*)a)->dfSize > ((LargeFireData*)b)->dfSize )
        return -1;
    else if( ((LargeFireData*)a)->dfSize < ((LargeFireData*)b)->dfSize )
        return 1;
    else
        return 0;
}

static double AvgLargeFireSize( LargeFireData *pasFires, int nSize, 
                                double *dfAvgSize, double *dfSumSize )
{
    if( pasFires == NULL || dfAvgSize == NULL || dfSumSize == NULL )
    {
        return IRS_INVALID_DATA;
    }

    *dfAvgSize = 0;
    *dfSumSize = 0;
    int i;
    for( i = 0; i < nSize; i++ )
    {
        if( pasFires[i].bExcluded )
        {
            continue;
        }
        *dfAvgSize += pasFires[i].dfSize;
    }
    *dfSumSize = *dfAvgSize;
    *dfAvgSize /= nSize;
    return IRS_OK;
}

static const double LargeFireTreatPerc[] = { 0.0,
                                             0.1,
                                             0.2,
                                             0.3,
                                             0.4,
                                             0.5,
                                             0.6,
                                             0.7,
                                             0.8,
                                             0.9,
                                             1.0, };
static const double LargeFireTreatRed[] =  { 1.00,
                                             0.92,
                                             0.74,
                                             0.52,
                                             0.37,
                                             0.30,
                                             0.25,
                                             0.22,
                                             0.21,
                                             0.20,
                                             0.20 };


static double LargeFireTargetAverage( double dfTreatPerc )
{
    if( dfTreatPerc < 0 || dfTreatPerc > 1.0 )
        return IRS_INVALID_DATA;
    int nIndex = (int)(dfTreatPerc * 10);
    assert( nIndex < 10 );
    double dX, dY, dfVal;
    dX = dfTreatPerc - LargeFireTreatPerc[nIndex];
    dY = (LargeFireTreatRed[nIndex] - LargeFireTreatRed[nIndex+1]) * dX;
    dfVal = LargeFireTreatRed[nIndex] - dY;
    return dfVal;
}

/**
 * \brief Decrease the average size of the sample of large fire
 *
 * Using the following table:
 * % treated    % average fire size
 *   0.0          1.00
 *   0.1          0.92
 *   0.2          0.74
 *   0.3          0.52
 *   0.4          0.37
 *   0.5          0.30
 *   0.6          0.25
 *   0.7          0.22
 *   0.8          0.21
 *   0.9          0.20
 *   1.0          0.20
 */

static int DecreaseLargeFireSize( LargeFireData *pasFires, int nSize,
                                  double dfPercTreat )
{
    if( dfPercTreat == 0.0 || nSize < 2 )
        return IRS_OK;
    Random rdm;
    int i = 0;
    int nRemoved = 0;
    double dfAvgSize, dfSumSize;
    if( AvgLargeFireSize( pasFires, nSize, &dfAvgSize, &dfSumSize ) != 0 )
        return IRS_UNKNOWN_FAIL;
    double dfAvgRed = LargeFireTargetAverage( dfPercTreat );
    double dfTargetAvg = dfAvgSize * dfAvgRed;
    /*
    ** Sort our fires by size.
    */
    qsort( pasFires, nSize, sizeof( LargeFireData ), CompareLargeFireSize );
    double dfProb;
    double r;
    i;
    for( i = 0; i < nSize; i++ )
    {
        if( pasFires[i].bExcluded )
            continue;
        dfProb = pasFires[i].dfSize / dfSumSize;

        r = rdm.rand3();
        if( r * dfPercTreat < dfProb && !pasFires[i].bExcluded )
        {
            /* remove */
            pasFires[i].bExcluded = TRUE;
            nRemoved++;
        }
        if( nRemoved == nSize )
        {
            return IRS_INVALID_DATA;
        }
        if( AvgLargeFireSize( pasFires, nSize, &dfAvgSize, &dfSumSize ) != IRS_OK )
            return IRS_UNKNOWN_FAIL;
        if( dfAvgSize < dfTargetAvg )
        {
            break;
        }
    }
    if( dfAvgSize > dfTargetAvg )
        return IRS_UNKNOWN_FAIL;
    return IRS_OK;
}

int IRSResult::PostProcessLargeFire( int nEscapeTypes, double dfSubSample,
                                     unsigned int nSampleSize,
                                     double dfTreated, double dfBuffer,
                                     int nDaySpan, int nMinSize,
                                     IRSProgress pfnProgress)
{
    int rc;
    int i = 0, j = 0, k = 0, v = 0;;
    Random rdm;
    sqlite3_stmt *stmt;
    sqlite3_stmt *estmt;
    sqlite3_stmt *tstmt;
    sqlite3_stmt *lfstmt;
    sqlite3_stmt *istmt;
    rc = sqlite3_prepare_v2( db, "INSERT INTO large_fire_result(year, " \
                                 "fire_num, cost_acre, acres, pop) " \
                                 "VALUES(?, ?, ?, ?, ?)", -1, &istmt,
                             NULL );
    rc = sqlite3_exec( db, "CREATE INDEX IF NOT EXISTS idx_fire_num ON " \
                           "fire_result(year, fire_num)", NULL, NULL , NULL );

    rc = sqlite3_prepare_v2( db, "ATTACH ? AS omffr", -1, &stmt, NULL );
    rc = sqlite3_bind_text( stmt, 1, pszOmffrPath, -1, NULL );
    rc = sqlite3_step( stmt );
    rc = sqlite3_enable_load_extension( db, TRUE );

    /* Clamp minimum size */
    nMinSize = nMinSize < 0 ? 0 : nMinSize;

    sqlite3_finalize( stmt );
    stmt = NULL;
    int nCount = 0;
    char **papszEscapes = NULL;
    char *pszEscapes;
    const char *pszJoined;
    if( !nEscapeTypes )
        nEscapeTypes = ALL_ESCAPES;
    if( nEscapeTypes )
    {
        while( apszStatusStrings[i] != NULL )
        {
            if( nEscapeTypes & 1 << i )
            {
                papszEscapes = AddString( papszEscapes, apszStatusStrings[i] );
            }
            i++;
        }
    }
    pszJoined = JoinStringList( papszEscapes, "','" );
    pszEscapes = sqlite3_mprintf( "'%s'", pszJoined );
    OmffrFree( (void*)pszJoined );

    const char *pszSql = NULL;
    if( pfnProgress )
    {
        pszSql = sqlite3_mprintf( "SELECT COUNT(*) FROM fire_result " \
                                  "WHERE status IN (%s)", pszEscapes );
        rc = sqlite3_prepare_v2( db, pszSql, -1, &stmt, NULL );
        rc = sqlite3_step( stmt );
        nCount = rc != SQLITE_ROW ? 0 : sqlite3_column_int( stmt, 0 );
        sqlite3_reset( stmt );
        sqlite3_finalize( stmt );
        stmt = NULL;
        sqlite3_free( (void*)pszSql );
    }
     pszSql = sqlite3_mprintf( "SELECT fire_result.year, " \
                               "fire_result.fire_num, fire_result.jul_day, " \
                               "geometry, ros - tr_ros FROM " \
                               "fire_result JOIN scenario " \
                               "USING(year, fire_num) WHERE " \
                               "status IN (%s) AND finalsize > " \
                               "%d", pszEscapes, nMinSize );
    rc = sqlite3_prepare_v2( db, pszSql, -1, &estmt, NULL );
    /*
    ** Calculate % actually treated over all years.  This is brute force, and
    ** should be changed to something reasonably faster.
    */
    rc = sqlite3_prepare_v2( db, "SELECT ros, tr_ros FROM scenario WHERE " \
                                 "ST_Contains(ST_Buffer(?1, ?2), " \
                                 "            scenario.geometry) " \
                                 "AND scenario.ROWID IN(SELECT " \
                                 "pkid FROM idx_scenario_geometry WHERE " \
                                 "xmin <= MbrMaxX(ST_Buffer(?1, ?2)) AND " \
                                 "xmax >= MbrMinX(ST_Buffer(?1, ?2)) AND " \
                                 "ymin <= MbrMaxY(ST_Buffer(?1, ?2)) AND " \
                                 "ymax >= MbrMinY(ST_Buffer(?1, ?2)))", -1,
                             &tstmt, NULL );

    rc = sqlite3_prepare_v2( db, "SELECT cost_acre, acres, pop " \
                                 "FROM large_fire WHERE ST_Contains(" \
                                 "ST_Buffer(?1, ?2), large_fire.geometry) " \
                                 "AND large_fire.jul_day BETWEEN ?3 AND ?4 " \
                                 "AND large_fire.ROWID IN (SELECT " \
                                 "pkid FROM idx_large_fire_geometry WHERE " \
                                 "xmin <= MbrMaxX(ST_Buffer(?1, ?2)) AND " \
                                 "xmax >= MbrMinX(ST_Buffer(?1, ?2)) AND " \
                                 "ymin <= MbrMaxY(ST_Buffer(?1, ?2)) AND " \
                                 "ymax >= MbrMinY(ST_Buffer(?1, ?2))) " \
                                 "ORDER BY RANDOM() LIMIT ?5",
                             -1, &lfstmt, NULL );
    int nTotal = 0;
    int nGeomSize;
    const void *hGeometry;
    int nJulDay;
    int nChosen;
    int nYear, nFireNum;
    double dfR;
    LargeFireData *pasLargeFires;
    pasLargeFires = (LargeFireData*)sqlite3_malloc( sizeof( LargeFireData ) *
                                                    nSampleSize );
    double dfCost, dfAcre, dfPop;
    sqlite3_exec( db, "BEGIN", NULL, NULL, NULL );
    /*
    ** We can bind the buffer and limit just once.
    */
    rc = sqlite3_bind_double( lfstmt, 2, dfBuffer );
    rc = sqlite3_bind_int( lfstmt, 5, nSampleSize );
    rc = sqlite3_bind_double( tstmt, 2, dfBuffer );
    int nTreated, nNotTreated;
    int nValid;
    double dfTreatPerc;
    if( pfnProgress )
        pfnProgress( 0.0, "Post Processing large fire...", NULL );
    while( sqlite3_step( estmt ) == SQLITE_ROW )
    {
        dfR = rdm.rand3();
        if( dfR > dfSubSample )
            continue;
        nYear = sqlite3_column_int( estmt, 0 );
        nFireNum = sqlite3_column_int( estmt, 1 );
        nJulDay = sqlite3_column_int( estmt, 2 );
        nGeomSize = sqlite3_column_bytes( estmt, 3 );
        hGeometry = sqlite3_column_blob( estmt, 3 );
        rc = sqlite3_bind_int( lfstmt, 3, nJulDay - nDaySpan / 2 );
        rc = sqlite3_bind_int( lfstmt, 4, nJulDay + nDaySpan / 2 );
        rc = sqlite3_bind_blob( lfstmt, 1, hGeometry, nGeomSize, NULL );

        /*
        ** Treated stuff
        */
        rc = sqlite3_bind_blob( tstmt, 1, hGeometry, nGeomSize, NULL );
        nTreated = nNotTreated = 0;
        while( sqlite3_step( tstmt ) == SQLITE_ROW )
        {
            if( sqlite3_column_double( tstmt, 0 ) !=
                sqlite3_column_double( tstmt, 1 ) )
            {
                nTreated++;
            }
            else
            {
                nNotTreated++;
            }
        }
        i = 0;
        while( sqlite3_step( lfstmt ) == SQLITE_ROW )
        {
            dfCost = sqlite3_column_double( lfstmt, 0 );
            dfAcre = sqlite3_column_double( lfstmt, 1 );
            dfPop = sqlite3_column_double( lfstmt, 2 );
            pasLargeFires[i].dfCostAcre = dfCost;
            pasLargeFires[i].dfSize = dfAcre;
            pasLargeFires[i].dfPop = dfPop;
            pasLargeFires[i].bExcluded = FALSE;
            i++;
        }
        if( !i )
        {
            sqlite3_reset( lfstmt );
            sqlite3_reset( tstmt );
            nTotal += 1;
            continue;
        }
        dfTreatPerc = (double)nTreated / ((double)nTreated + (double)nNotTreated );
        dfTreatPerc *= dfTreated;
        rc = DecreaseLargeFireSize( pasLargeFires, i, dfTreatPerc );
        nValid = 0;
        for( v = 0; v < i; v++ )
        {
            if( !(pasLargeFires[v].bExcluded) )
                nValid++;
        }
        /* Sample, bind, insert a single large file */
        dfR = rdm.rand3();
        nChosen = dfR * nValid;
        j = 0;
        k = 0;
        while( j < i && k < nChosen )
        {
            if( !pasLargeFires[j].bExcluded )
                k++;
            j++;
        }
        if( j == i )
            j--;
        sqlite3_bind_int( istmt, 1, nYear );
        sqlite3_bind_int( istmt, 2, nFireNum );
        sqlite3_bind_double( istmt, 3, pasLargeFires[j].dfCostAcre );
        sqlite3_bind_double( istmt, 4, pasLargeFires[j].dfSize );
        sqlite3_bind_double( istmt, 5, pasLargeFires[j].dfPop );
        rc = sqlite3_step( istmt );

        sqlite3_reset( istmt );
        sqlite3_reset( lfstmt );
        sqlite3_reset( tstmt );
        nTotal += 1;
        if( pfnProgress )
        {
            pfnProgress( (double)nTotal / (double)nCount,
                         "Post Processing large fire...", NULL );
        }
    }

    sqlite3_exec( db, "END", NULL, NULL, NULL );
    sqlite3_reset( estmt );
    sqlite3_finalize( istmt );
    sqlite3_finalize( stmt );
    sqlite3_finalize( estmt );
    sqlite3_finalize( lfstmt );
    sqlite3_free( (void*)pszEscapes );
    sqlite3_free( (void*)pszSql );
    sqlite3_exec( db, "DETACH omffr", NULL, NULL, NULL );
    return IRS_OK;
}

#define IRS_RESULT_ATT_SKIP 2

char ** IRSResult::GetResultAttributes()
{
    assert( db );
    int rc;
    int i;
    char ** papszAttributes = NULL;
    sqlite3_stmt *stmt = NULL;
    rc = sqlite3_prepare_v2( db, "PRAGMA table_info(fpu_output)", -1, &stmt,
                             NULL );
    /*
    ** Burn through twice to count the allocation size and set values.
    */
    int nColCount = 0;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        nColCount++;
    }
    /*
    ** Number of columns minus the ones we discard, plus one for null.
    ** See below and IRS_RESULT_ATT_SKIP to see how many we skip
    */
    papszAttributes = (char **)sqlite3_malloc( sizeof( char *) *
                                               (nColCount -
                                                IRS_RESULT_ATT_SKIP + 1 ) );
    sqlite3_reset( stmt );
    i = 0;
    /* Skip OGC_FID, geometry and fpu */
    while( sqlite3_step( stmt ) && i++ < IRS_RESULT_ATT_SKIP ) ;
    i = 0;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        papszAttributes[i++] =
            sqlite3_mprintf( "%s", sqlite3_column_text( stmt, 1 ) );
    }
    papszAttributes[i] = NULL;
    sqlite3_finalize( stmt );
    return papszAttributes;
}

int IRSResult::FreeResultAttributes( char ** papszAttributes )
{
    return FreeStringList( papszAttributes );
}

int IRSResult::FreeStringList( char **pszList )
{
    if( !pszList )
        return IRS_OK;
    int i = 0;
    while( pszList[i] != NULL )
    {
        sqlite3_free( pszList[i++] );
    }
    return IRS_OK;
}

typedef struct _FpuSummary
{
    const char *pszFpuCode;
    int nYear;
    int anStatus[5];
} FpuSummary;

/**
 * \brief Group results by fpu
 *
 * This summarizes and stores fire results by fpu and year.  Other tables may
 * use this to generate summary data and other outputs (graphs, maps, etc.).
 */
int IRSResult::SummarizeByFpu( IRSProgress pfnProgress )
{
    int rc;
    sqlite3_stmt *stmt, *lfstmt, *istmt, *ulfstmt;
    rc = sqlite3_enable_load_extension( db, TRUE );
    rc = sqlite3_exec( db, "PRAGMA journal_mode=OFF", NULL, NULL, NULL );
    rc = sqlite3_exec( db, "BEGIN", NULL, NULL, NULL );

    rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM sqlite_master WHERE "
                                 "name='spatial_ref_sys'", -1, &stmt, NULL );

    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW || sqlite3_column_int( stmt , 0 ) < 1)
    {
        sqlite3_exec( db, "SELECT InitSpatialMetadata()", NULL, NULL, NULL );
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    rc = sqlite3_exec( db, "END", NULL, NULL, NULL );

    rc = sqlite3_exec( db, "PRAGMA journal_mode=WAL", NULL, NULL, NULL );
    rc = sqlite3_exec( db, "CREATE INDEX IF NOT EXISTS idx_fire_num ON " \
                           "fire_result(year, fire_num)", NULL, NULL , NULL );

    rc = sqlite3_prepare_v2( db, "ATTACH ? AS omffr", -1, &stmt, NULL );
    rc = sqlite3_bind_text( stmt, 1, pszOmffrPath, -1, NULL );
    rc = sqlite3_step( stmt );
    sqlite3_reset( stmt );
    sqlite3_finalize( stmt );
    stmt = NULL;
#ifndef IRS_RESULT_USE_SPATIAL
    /* Fire results */
    rc = sqlite3_prepare_v2( db, "SELECT fpu.name, year," \
                                 "status, COUNT(*) FROM fire_result " \
                                 "JOIN scenario USING(year, fire_num) " \
                                 "JOIN fwa ON scenario.fwa_id=fwa.name " \
                                 "JOIN fpu on fwa.fpu_id=fpu.id " \
                                 "JOIN fire_season on " \
                                 "fire_season.fpu_id=fpu.id WHERE " \
                                 "fire_result.jul_day > fire_season.start_day " \
                                 "AND fire_result.jul_day < fire_season.end_day " \
                                 "GROUP BY fpu.name, year, status " \
                                 "ORDER BY fpu.name, year, status", -1, &stmt,
                             NULL );
    /* Large fire */
    rc = sqlite3_prepare_v2( db, "SELECT fpu.name, large_fire_result.year, " \
                                 "SUM(cost_acre * acres), SUM(acres), " \
                                 "SUM(pop) FROM large_fire_result " \
                                 "LEFT JOIN scenario USING(year, fire_num) " \
                                 "JOIN fwa ON scenario.fwa_id=fwa.name " \
                                 "JOIN fpu on fwa.fpu_id=fpu.id " \
                                 "JOIN fire_season on " \
                                 "fire_season.fpu_id=fpu.id WHERE " \
                                 "scenario.jul_day > " \
                                 "fire_season.start_day AND " \
                                 "scenario.jul_day < " \
                                 "fire_season.end_day " \
                                 "GROUP BY fpu.name, large_fire_result.year ",
                             -1, &lfstmt, NULL );
    /* Resource usage */
#else
    rc = sqlite3_prepare_v2( db, "SELECT fpu_code, year," \
                                 "status, COUNT(*) FROM fire_result " \
                                 "JOIN scenario USING(year, fire_num) " \
                                 "JOIN fpu_bndry ON " \
                                 "ST_Contains(fpu_bndry.geometry, scenario.geometry) " \
                                 "JOIN fpu ON fpu.name=fpu_bndry.fpu_code "
                                 "JOIN fire_season on " \
                                 "fire_season.fpu_id=fpu.id WHERE " \
                                 "fire_result.jul_day > fire_season.start_day " \
                                 "AND fire_result.jul_day < fire_season.end_day " \
                                 "AND scenario.ROWID IN (SELECT pkid FROM " \
                                 "idx_scenario_geometry WHERE " \
                                 "xmin <= MbrMaxX(fpu_bndry.geometry) AND " \
                                 "xmax >= MbrMinX(fpu_bndry.geometry) AND " \
                                 "ymin <= MbrMaxY(fpu_bndry.geometry) AND " \
                                 "ymax >= MbrMinY(fpu_bndry.geometry))" 
                                 "GROUP BY fpu.name, year, status " \
                                 "ORDER BY fpu.name, year, status", -1, &stmt,
                             NULL );
    rc = sqlite3_prepare_v2( db, "SELECT fpu_code, large_fire_result.year, " \
                                 "SUM(cost_acre * acres), SUM(acres), " \
                                 "SUM(pop) FROM large_fire_result JOIN " \
                                 "scenario USING(year, fire_num)" \
                                 "JOIN fpu_bndry ON " \
                                 "ST_Contains(fpu_bndry.geometry, scenario.geometry) " \
                                 "JOIN fpu ON fpu.name=fpu_bndry.fpu_code "
                                 "JOIN fire_season on " \
                                 "fire_season.fpu_id=fpu.id WHERE " \
                                 "scenario.jul_day > fire_season.start_day " \
                                 "AND scenario.jul_day < fire_season.end_day " \
                                 "AND scenario.ROWID IN (SELECT pkid FROM " \
                                 "idx_scenario_geometry WHERE " \
                                 "xmin <= MbrMaxX(fpu_bndry.geometry) AND " \
                                 "xmax >= MbrMinX(fpu_bndry.geometry) AND " \
                                 "ymin <= MbrMaxY(fpu_bndry.geometry) AND " \
                                 "ymax >= MbrMinY(fpu_bndry.geometry))" 
                                 "GROUP BY fpu.name, large_fire_result.year",
                             -1, &lfstmt, NULL );

#endif /* IRS_RESULT_USE_SPATIAL */
    rc = sqlite3_prepare_v2( db, "INSERT INTO fpu_output(fpu_code, year, " \
                                 CONTAIN_COLUMN"," NO_RESC_COLUMN", "  \
                                 TIME_LIMIT_COLUMN", " \
                                 SIZE_LIMIT_COLUMN", "EXHAUST_COLUMN") " \
                                 "VALUES(?, ?, ?, ?, ?, ?, ?)", -1, &istmt,
                             NULL );
    rc = sqlite3_prepare_v2( db, "UPDATE fpu_output SET " \
                                  LF_COST_COLUMN "=?, " \
                                  LF_ACRE_COLUMN "=?, " \
                                  LF_POP_COLUMN  "=? WHERE fpu_code=? " \
                                  "AND year=?", -1, &ulfstmt, NULL );
    const char *pszFpuCode;
    const char *pszStatus;
    int nYear;
    int i, j, k;

    /*
    ** This is a little tricky.  Stepping through the fpu and getting the
    ** data.  I'm sure the sql could be better here, now I get:
    ** fpu | year | status | count
    */
    i = 0;
    int nDone = 0;
    FpuSummary sSummary;
    int nStatCount;
    int bFirstTime = TRUE;
    if( pfnProgress )
    {
        pfnProgress( 0.0, "Summarizing output by fpu...", NULL );
    }
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszFpuCode = (const char*)sqlite3_column_text( stmt, 0 );
        nYear = sqlite3_column_int( stmt, 1 );
        if( bFirstTime )
        {
            sSummary.pszFpuCode = sqlite3_mprintf( "%s", pszFpuCode );
            sSummary.nYear = nYear;
            for( i = 0; i < 5; i++ )
                sSummary.anStatus[i] = 0;
            bFirstTime = FALSE;
        }
        /*
        ** We are done, write to db
        */
        if( ( !EQUAL( sSummary.pszFpuCode, pszFpuCode ) ||
              nYear != sSummary.nYear ) )
        {
            sqlite3_bind_text( istmt, 1, sSummary.pszFpuCode, -1, NULL );
            sqlite3_bind_int( istmt, 2, sSummary.nYear );
            for( i = 0; i < 5; i++ )
            {
                sqlite3_bind_int( istmt, i + 3, sSummary.anStatus[i] );
                sSummary.anStatus[i] = 0;
            }
            rc = sqlite3_step( istmt );
            rc = sqlite3_reset( istmt );
            sqlite3_free( (void*)sSummary.pszFpuCode );
            sSummary.pszFpuCode = sqlite3_mprintf( "%s", pszFpuCode );
            sSummary.nYear = nYear;
            for( i = 0; i < 5; i++ )
                sSummary.anStatus[i] = 0;
        }
        pszStatus = (const char*)sqlite3_column_text( stmt, 2 );
        nStatCount = sqlite3_column_int( stmt, 3 );
        if(EQUAL(pszStatus, "Contained")) 
            sSummary.anStatus[0] += nStatCount;
        else if(EQUAL(pszStatus, "No Resources Sent"))
            sSummary.anStatus[1] += nStatCount;
        else if(EQUAL(pszStatus, "TimeLimitExceeded"))
            sSummary.anStatus[2] += nStatCount;
        else if(EQUAL(pszStatus, "SizeLimitExceeded"))
            sSummary.anStatus[3] += nStatCount;
        else if(EQUAL(pszStatus, "Exhausted")) 
            sSummary.anStatus[4] += nStatCount;
        if( pfnProgress )
        {
            pfnProgress( (double) nDone++ / 132.,
                         "Summarizing output by fpu...", NULL );
        }

    }
    if( pfnProgress )
    {
        pfnProgress( 1.0, "Done", NULL );
    }

    sqlite3_bind_text( istmt, 1, sSummary.pszFpuCode, -1, NULL );
    sqlite3_bind_int( istmt, 2, sSummary.nYear );
    for( i = 0; i < 5; i++ )
    {
        sqlite3_bind_int( istmt, i + 3, sSummary.anStatus[i] );
        sSummary.anStatus[i] = 0;
    }
    rc = sqlite3_step( istmt );
    rc = sqlite3_reset( istmt );
    sqlite3_free( (void*)sSummary.pszFpuCode );

    sqlite3_exec( db, "END", NULL, NULL, NULL );
    sqlite3_finalize( stmt ); stmt = NULL;
    sqlite3_finalize( istmt ); istmt = NULL;
    /*
    ** Large fire result query/update
    */

    double dfCost, dfSize, dfPop;
    if( pfnProgress )
    {
        pfnProgress( 0.0, "Summarizing large fire output by fpu...", NULL );
    }
    sqlite3_exec( db, "BEGIN", NULL, NULL, NULL );
    nDone = 0;
    while( sqlite3_step( lfstmt ) == SQLITE_ROW )
    {
        pszFpuCode = (const char *)sqlite3_column_text( lfstmt, 0 );
        nYear = sqlite3_column_int( lfstmt, 1 );
        dfCost= sqlite3_column_double( lfstmt, 2 );
        dfSize = sqlite3_column_double( lfstmt, 3 );
        dfPop = sqlite3_column_double( lfstmt, 4 );
        rc = sqlite3_bind_double( ulfstmt, 1, dfCost );
        rc = sqlite3_bind_int( ulfstmt, 2, dfSize );
        rc = sqlite3_bind_int( ulfstmt, 3, dfPop );
        rc = sqlite3_bind_text( ulfstmt, 4, pszFpuCode, -1, NULL );
        rc = sqlite3_bind_int( ulfstmt, 5, nYear );
        rc = sqlite3_step( ulfstmt );
        rc = sqlite3_reset( ulfstmt );
        if( pfnProgress )
        {
            pfnProgress( (double)nDone / 132.,
                         "Summarizing large fire output by fpu...", NULL );
        }
    }
    if( pfnProgress )
    {
        pfnProgress( 0.0, "Summarizing large fire output by fpu...", NULL );
    }

    sqlite3_exec( db, "END", NULL, NULL, NULL );
    sqlite3_finalize( lfstmt ); lfstmt = NULL;
    sqlite3_finalize( ulfstmt ); ulfstmt = NULL;
    sqlite3_exec( db, "DETACH omffr", NULL, NULL, NULL );
    rc = sqlite3_exec( db, "PRAGMA journal_mode=DELETE", NULL, NULL, NULL );
    return IRS_OK;
}

/**
 * \brief Clear all of our tables, and init a new db
 *
 * Ideally the user would've just deleted and old and our connections would
 * die, but this is safe.  We can disconnect and unlink, reopen or drop tables.
 * Not sure which one is best, both implemented.  USE IRS_RESULT_UNLINK to
 * enable in cmake.
 *
 */
int IRSResult::Reset()
{
    /*
    ** Do we have a db?
    */
    if( !db )
        return IRS_OK;
    int rc;
#ifdef IRS_RESULT_UNLINK_RESET
    rc = sqlite3_close( db );
    VSIUnlink( pszPath );
    rc = sqlite3_open_v2( pszPath, &db, SQLITE_OPEN_READWRITE |
                                        SQLITE_OPEN_CREATE, NULL );
#else
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, "SELECT type, name FROM sqlite_master", -1,
                             &stmt, NULL );
    const char *pszName;
    const char *pszType;
    const char *pszSql;
    rc = sqlite3_exec( db, "PRAGMA journal_mode=off", NULL, NULL, NULL );
    rc = sqlite3_exec( db, "BEGIN", NULL, NULL, NULL );
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszName = (const char*)sqlite3_column_text( stmt, 0 );
        pszType = (const char*)sqlite3_column_text( stmt, 1 );
        pszSql = sqlite3_mprintf( "DROP %s %s", pszType, pszName );
        sqlite3_exec( db, pszSql, NULL, NULL, NULL );
        sqlite3_free( (void*) pszSql );
    }
    rc = sqlite3_exec( db, "END", NULL, NULL, NULL );
    rc = sqlite3_exec( db, "PRAGMA journal_mode=delete", NULL, NULL, NULL );
#endif /* IRS_RESULT_UNLINK_RESET */
    return IRS_OK;
}

/**
 * \brief Self validation checks.
 *
 */
int IRSResult::IValidate()
{
    int bStatus = IRS_NULL_DB | IRS_MISSING_TABLES | IRS_MISSING_SPLITE;
    if( !db )
        return bStatus;
    else
        bStatus | IRS_NULL_DB;
    int rc;
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2( db, "SELECT * FROM sqlite_master WHERE "
                                 "type='table'", -1, &stmt,
                             NULL );
    const char *pszTable;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszTable = (const char*)sqlite3_column_text( stmt, 0 );
        if( EQUAL( pszTable, "irs_metadata" ) )
            bStatus = bStatus | IRS_MISSING_TABLES;
        else if( EQUAL( pszTable, "spatial_ref_sys" ) )
            bStatus = bStatus | IRS_MISSING_SPLITE;
    }
    sqlite3_finalize( stmt ); stmt = NULL;
    return IRS_OK;
}

std::list<double> IRSResult::NationalDistribution( std::string Attribute )
{
        // Create the std::list for each year's value of the attribute
        std::list<double> Values;

        int rc = 0;
        int rcc = 0;
        double v = 0.0;

        sqlite3_stmt *stmt = NULL;
        sqlite3_stmt *stmtc = NULL;

        if (Attribute == "Percent Contained")   {

                rc = sqlite3_prepare_v2(db, "SELECT year, SUM("CONTAIN_COLUMN") " \
                                                              "FROM fpu_output " \
                                                              "GROUP BY year ORDER BY year", \
															  -1, &stmt, NULL );

                rcc = sqlite3_prepare_v2(db, "SELECT year, " \
                                                                                                              "SUM("SIZE_LIMIT_COLUMN") + SUM("TIME_LIMIT_COLUMN") + " \
                                                              "SUM("CONTAIN_COLUMN") + SUM("EXHAUST_COLUMN") + " \
                                                              "SUM("NO_RESC_COLUMN") FROM fpu_output " \
                                                              "GROUP BY year ORDER BY year", \
															  -1, &stmtc, NULL );
        }

        else if (Attribute == "Large Fire Cost")        {

                rc = sqlite3_prepare_v2(db, "SELECT year, SUM("LF_COST_COLUMN") " \
                                   "FROM fpu_output " \
                                   "GROUP BY year ORDER BY year", \
								   -1, &stmt, NULL );
        }

        else if (Attribute == "Large Fire Acres")       {

                rc = sqlite3_prepare_v2(db, "SELECT year, SUM("LF_ACRE_COLUMN") " \
                                   "FROM fpu_output " \
                                   "GROUP BY year ORDER BY year", \
								   -1, &stmt, NULL );
        }

        else if (Attribute == "Large Fire Population")  {

                rc = sqlite3_prepare_v2(db, "SELECT year, SUM("LF_POP_COLUMN") " \
                                   "FROM fpu_output " \
                                   "GROUP BY year ORDER BY year" \
								   , -1, &stmt, NULL );
        }

        else if (Attribute == "Resource Usage")  {

                rc = sqlite3_prepare_v2(db, "SELECT year, SUM("RESC_USAGE_COLUMN") " \
                                   "FROM fpu_output " \
                                   "GROUP BY year ORDER BY year" \
								   , -1, &stmt, NULL );
        }

        if(rc != SQLITE_OK) 
                return Values;
        if (rcc != 0 && rcc != SQLITE_OK)
                return Values;


        while (sqlite3_step(stmt) == SQLITE_ROW)        {
                if (sqlite3_column_type(stmt, 1) == SQLITE_FLOAT)       
                        v = sqlite3_column_double(stmt, 1);
                else if (sqlite3_column_type(stmt, 1) == SQLITE_INTEGER)
                        v = (double)sqlite3_column_int(stmt, 1);
                Values.push_back(v);
                
        }

        if (Attribute == "Percent Contained")   {
                std::list<double>::iterator it = Values.begin();

                while (sqlite3_step(stmtc) == SQLITE_ROW)       {
                        if (sqlite3_column_type(stmtc, 1) == SQLITE_INTEGER)
                                v = (double)sqlite3_column_int(stmtc, 1);

                        if (it != Values.end()) {
                                (*it) = (*it) / v;
                                it++;
                        }

                        else    {
                                return Values;
                        }
                }
        }


        sqlite3_finalize(stmt);
        if (Attribute == "Percent Contained")
                sqlite3_finalize(stmtc);

        return Values;
        
}

std::vector<std::pair<std::string, std::list<double> > > IRSResult::GACCDistributions( std::string Attribute )
{
        // Create the std::list for each year's value of the attribute
        std::vector<std::pair<std::string, std::list<double> > > GACClists;
        std::vector<std::string> GACCs;
        std::list<double> Values;

        int rc = 0;
        int rcc = 0;
        double v = 0.0;

        GACCs.push_back("AK");
        GACCs.push_back("CA");
        GACCs.push_back("EA");
        GACCs.push_back("GB");
        GACCs.push_back("NR");
        GACCs.push_back("NW");
        GACCs.push_back("RM");
        GACCs.push_back("SA");
        GACCs.push_back("SW");

        sqlite3_stmt *stmt = NULL;
        sqlite3_stmt *stmtc = NULL;
    
        if (Attribute == "Percent Contained")   {

                rc = sqlite3_prepare_v2(db, "SELECT year, SUM("CONTAIN_COLUMN") " \
                                                              "FROM fpu_output " \
                                                              "WHERE fpu_code LIKE ?1 " \
                                                              "GROUP BY year", -1, &stmt, NULL );

                rcc = sqlite3_prepare_v2(db, "SELECT year, " \
                                                              "SUM("SIZE_LIMIT_COLUMN") + SUM("TIME_LIMIT_COLUMN") + " \
                                                              "SUM("CONTAIN_COLUMN") + SUM("EXHAUST_COLUMN") + " \
                                                              "SUM("NO_RESC_COLUMN") FROM fpu_output " \
                                                              "WHERE fpu_code LIKE ?1 " \
                                                              "GROUP BY year", -1, &stmtc, NULL );
        }

        else if (Attribute == "Large Fire Cost")        {
                
                rc = sqlite3_prepare_v2(db, "SELECT year, SUM("LF_COST_COLUMN") " \
                                                               "FROM fpu_output " \
                                                               "WHERE fpu_code LIKE ?1 " \
                                                               "GROUP BY year", -1, &stmt, NULL );
        }

        else if (Attribute == "Large Fire Acres")       {

                rc = sqlite3_prepare_v2(db, "SELECT year, SUM("LF_ACRE_COLUMN") " \
                                                               "FROM fpu_output " \
                                                               "WHERE fpu_code LIKE ?1 " \
                                                               "GROUP BY year", -1, &stmt, NULL );
        }

        else if (Attribute == "Large Fire Population")  {

                rc = sqlite3_prepare_v2(db, "SELECT year, SUM("LF_POP_COLUMN") " \
                                                                "FROM fpu_output " \
                                                                "WHERE fpu_code LIKE ?1 " \
                                                                "GROUP BY year", -1, &stmt, NULL );
        }

         else if (Attribute == "Resource Usage")  {

                rc = sqlite3_prepare_v2(db, "SELECT year, SUM("RESC_USAGE_COLUMN") " \
                                                                "FROM fpu_output " \
                                                                "WHERE fpu_code LIKE ?1 " \
                                                                "GROUP BY year", -1, &stmt, NULL );
        }

        if (rc != SQLITE_OK) 
                return GACClists;
        if (rcc != 0 && rcc != SQLITE_OK)
                return GACClists;

        for (int i = 0; i < GACCs.size(); i++)    {

                Values.clear();
                char likeGACC[10];
                int n = sprintf(likeGACC, "%s%s", GACCs[i].c_str(), "%");
                                
                rc = sqlite3_bind_text(stmt, 1, likeGACC, -1, NULL );
                if (Attribute == "Percent Contained")
                        rcc = sqlite3_bind_text(stmtc, 1, likeGACC, -1, NULL);

                while (sqlite3_step(stmt) == SQLITE_ROW)        {
                        if (sqlite3_column_type(stmt, 1) == SQLITE_FLOAT)       
                                v = sqlite3_column_double(stmt, 1);
                        else if (sqlite3_column_type(stmt, 1) == SQLITE_INTEGER)
                                v = (double)sqlite3_column_int(stmt, 1);
                        
                        Values.push_back(v);
                }

                if (Attribute == "Percent Contained")   {
                        std::list<double>::iterator it = Values.begin();

                        while (sqlite3_step(stmtc) == SQLITE_ROW)       {
                                if (sqlite3_column_type(stmtc, 1) == SQLITE_INTEGER)
                                        v = (double)sqlite3_column_int(stmtc, 1);

                                if (it != Values.end()) {
                                        (*it) = (*it) / v;
                                        it++;
                                }

                                else    {
                                        GACClists.clear();
                                        return GACClists;
                                }
                        }

                        sqlite3_reset(stmtc);
                }

                sqlite3_reset(stmt);
                std::pair<std::string, std::list<double> > GACCpair = std::make_pair(GACCs[i], Values);
                GACClists.push_back(GACCpair);

        }

        sqlite3_finalize(stmt);
        if (Attribute == "Percent Contained")
                sqlite3_finalize(stmtc);

        return GACClists;
}

IRSResult * IRSResult::GetSiblingResult( int nDirection )
{
    if( nDirection != IRS_PREV_RUN_DIR && nDirection != IRS_NEXT_RUN_DIR )
        return NULL;
    IRSResult *poR = NULL;
    const char *pszSibPath = NULL;
    if( nDirection == IRS_PREV_RUN_DIR )
        pszSibPath = GetMetadataItem( IRS_PREV_RUN_KEY );
    else
        pszSibPath = GetMetadataItem( IRS_NEXT_RUN_KEY );
    if( pszSibPath != NULL )
    {
        poR = new IRSResult( pszSibPath, pszOmffrPath, 1, FALSE, FALSE, NULL );
        free( (void*)pszSibPath );
    }
    return poR;
}

std::vector<IRSResult*> IRSResult::GetResultArray( int *nResultCount,
                                                   int nDirection )
{
    std::vector<IRSResult*> apResults;
    int i = 0;
    IRSResult *poR;
    apResults.push_back( this );
    poR = GetSiblingResult( nDirection );
    while( poR != NULL )
    {
        apResults.push_back( poR );
        poR = poR->GetSiblingResult( nDirection );
    }
    if( nResultCount )
    {
        *nResultCount = apResults.size();
    }
    return apResults;
}

int IRSResult::DestroySiblingResult( IRSResult * poResult )
{
    delete poResult;
    return IRS_OK;
}

/*
** The first one is this, don't blow it up.
*/
int IRSResult::DestroySiblingResults( std::vector<IRSResult*> apResults )
{
    int i;
    for( i = 1 ;i < apResults.size();i++ )
        delete apResults[i];
    return IRS_OK;
}

pair<double, double> IRSResult::MinMaxValues( std::string Attribute )
{
        double min, max;
        int rc = 0;
 
        sqlite3_stmt *stmt = NULL;
        sqlite3_stmt *stmtt = NULL;

        if (Attribute == "Percent Contained")   {

		rc = sqlite3_prepare_v2(db, "SELECT MIN("PERC_CONT_COLUMN") " \
                                                "FROM gacc_sum", \
                                                -1, &stmt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);

		rc = sqlite3_prepare_v2(db, "SELECT MAX("PERC_CONT_COLUMN") " \
                                                   "FROM gacc_sum", \
                                                    -1, &stmtt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);
        }

    else if (Attribute == "Large Fire Cost")   {

        rc = sqlite3_prepare_v2(db, "SELECT MIN("LF_COST_COLUMN") " \
                                           "FROM gacc_sum", \
                                            -1, &stmt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);

                rc = sqlite3_prepare_v2(db, "SELECT MAX("LF_COST_COLUMN") " \
                                            "FROM gacc_sum", \
                                            -1, &stmtt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);
        }
                        
        else if (Attribute == "Large Fire Acres") {     
        rc = sqlite3_prepare_v2(db, "SELECT MIN("LF_ACRE_COLUMN") " \
                                            "FROM gacc_sum", \
                                            -1, &stmt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);

                rc = sqlite3_prepare_v2(db, "SELECT MAX("LF_ACRE_COLUMN") " \
                                            "FROM gacc_sum", \
                                            -1, &stmtt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);
        }

        else if (Attribute == "Large Fire Population")  {     
        rc = sqlite3_prepare_v2(db, "SELECT MIN("LF_POP_COLUMN") " \
                                            "FROM gacc_sum", \
                                            -1, &stmt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);

                rc = sqlite3_prepare_v2(db, "SELECT MAX("LF_POP_COLUMN") " \
                                            "FROM gacc_sum", \
                                            -1, &stmtt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW && rc != SQLITE_DONE) return std::make_pair(0.0, 0.0);
        min = sqlite3_column_double(stmt,0);

        rc = sqlite3_step(stmtt);
        if (rc != SQLITE_ROW && rc != SQLITE_DONE) return std::make_pair(0.0, 0.0);
        max = sqlite3_column_double(stmtt,0);

        return std::make_pair(min, max);
}

pair<double, double> IRSResult::FPUMinMaxValues( std::string Attribute )
{
        double min, max;
        int rc = 0;
 
        sqlite3_stmt *stmt = NULL;
        sqlite3_stmt *stmtt = NULL;

        if (Attribute == "Percent Contained")   {

		rc = sqlite3_prepare_v2(db, "SELECT MIN("PERC_CONT_COLUMN") " \
                                                "FROM fpu_sum", \
                                                -1, &stmt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);

		rc = sqlite3_prepare_v2(db, "SELECT MAX("PERC_CONT_COLUMN") " \
                                                   "FROM fpu_sum", \
                                                    -1, &stmtt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);
        }

    else if (Attribute == "Large Fire Cost")   {

        rc = sqlite3_prepare_v2(db, "SELECT MIN("LF_COST_COLUMN") " \
                                           "FROM fpu_sum", \
                                            -1, &stmt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);

                rc = sqlite3_prepare_v2(db, "SELECT MAX("LF_COST_COLUMN") " \
                                            "FROM fpu_sum", \
                                            -1, &stmtt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);
        }
                        
        else if (Attribute == "Large Fire Acres") {     
        rc = sqlite3_prepare_v2(db, "SELECT MIN("LF_ACRE_COLUMN") " \
                                            "FROM fpu_sum", \
                                            -1, &stmt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);

                rc = sqlite3_prepare_v2(db, "SELECT MAX("LF_ACRE_COLUMN") " \
                                            "FROM fpu_sum", \
                                            -1, &stmtt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);
        }

        else if (Attribute == "Large Fire Population")  {     
        rc = sqlite3_prepare_v2(db, "SELECT MIN("LF_POP_COLUMN") " \
                                            "FROM fpu_sum", \
                                            -1, &stmt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);

                rc = sqlite3_prepare_v2(db, "SELECT MAX("LF_POP_COLUMN") " \
                                            "FROM fpu_sum", \
                                            -1, &stmtt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);
        }

        else if (Attribute == "Resource Usage")  {     
        rc = sqlite3_prepare_v2(db, "SELECT MIN("RESC_USAGE_COLUMN") " \
                                            "FROM fpu_sum", \
                                            -1, &stmt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);

                rc = sqlite3_prepare_v2(db, "SELECT MAX("RESC_USAGE_COLUMN") " \
                                            "FROM fpu_sum", \
                                            -1, &stmtt, NULL );
                if (rc != SQLITE_OK) return std::make_pair(0.0, 0.0);
        }

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW && rc != SQLITE_DONE) return std::make_pair(0.0, 0.0);
        min = sqlite3_column_double(stmt,0);

        rc = sqlite3_step(stmtt);
        if (rc != SQLITE_ROW && rc != SQLITE_DONE) return std::make_pair(0.0, 0.0);
        max = sqlite3_column_double(stmtt,0);

        return std::make_pair(min, max);
}

std::vector<std::pair<std::string, double> >IRSResult::GACCSeriesAvg( std::string Attribute )
{
    // Create the std::list for each year's value of the attribute
    std::vector<std::pair<std::string, double> > GACCValues;
    std::vector<std::string> GACCs;
    std::list<double> Values;

    int rc = 0;
    int numYears = 0;
    double v = 0.0;

    sqlite3_stmt *stmty = NULL;
    
	rc = sqlite3_prepare_v2(db, "SELECT count() FROM (SELECT DISTINCT year FROM fpu_output)", -1, &stmty, NULL);

	if (rc != SQLITE_OK) return GACCValues;
	
	rc = sqlite3_step(stmty);
	if (rc != SQLITE_ROW &&rc != SQLITE_DONE) return GACCValues;
	if (sqlite3_column_type(stmty, 0) == SQLITE_INTEGER)
		numYears = sqlite3_column_int(stmty, 0);

	sqlite3_finalize(stmty);

	sqlite3_stmt *stmt = NULL;
	    
    if (Attribute == "Percent Contained")   
		rc = sqlite3_prepare_v2(db, "SELECT gacc_code, "PERC_CONT_COLUMN" " \
										"FROM gacc_sum", -1, &stmt, NULL );

    else if (Attribute == "Large Fire Cost")        
        rc = sqlite3_prepare_v2(db, "SELECT gacc_code, "LF_COST_COLUMN" " \
										"FROM gacc_sum", -1, &stmt, NULL );
        
	else if (Attribute == "Large Fire Acres")       
		rc = sqlite3_prepare_v2(db, "SELECT gacc_code, "LF_ACRE_COLUMN" " \
                                         "FROM gacc_sum", -1, &stmt, NULL );

    else if (Attribute == "Large Fire Population")  
		rc = sqlite3_prepare_v2(db, "SELECT gacc_code, "LF_POP_COLUMN" " \
                                          "FROM gacc_sum", -1, &stmt, NULL );

	else if (Attribute == "Resource Usage")  
		rc = sqlite3_prepare_v2(db, "SELECT gacc_code, "RESC_USAGE_COLUMN" " \
                                          "FROM gacc_sum", -1, &stmt, NULL );

    if (rc != SQLITE_OK) 
        return GACCValues;
    

    while (sqlite3_step(stmt) == SQLITE_ROW)    {

        std::string strGACC;
		double value;

		if (sqlite3_column_type(stmt, 0) == SQLITE_TEXT)	
			strGACC = (const char*)sqlite3_column_text(stmt, 0);
		if (sqlite3_column_type(stmt, 1) == SQLITE_FLOAT)
			value = sqlite3_column_double(stmt, 1);

		GACCValues.push_back(std::make_pair(strGACC, value));
    }

    sqlite3_finalize(stmt);
        
    return GACCValues;

}
int IRSResult::ExportOgrSummary( const char *pszFormat, const char *pszOutPath,
                                 char **papszOptions )
{
    OGRSFDriverH hDriver;
    OGRDataSourceH hSrcDS, hDS;
    OGRLayerH hSrcLayer, hLayer;
    OGRFeatureDefnH hSrcDefn, hDefn;
    OGRSpatialReferenceH hSRS;
    OGRFieldDefnH hFieldDefn;
    OGRFeatureH hSrcFeature, hFeature;

    hDriver = OGRGetDriverByName( pszFormat );
    if( hDriver == NULL )
    {
        return IRS_INVALID_INPUT;
    }

    hDS = OGR_Dr_CreateDataSource( hDriver, pszOutPath, NULL );
    if( hDS == NULL )
    {
        return IRS_INVALID_INPUT;
    }
    hSrcDS = OGROpen( pszPath, FALSE, NULL );
    hSrcLayer = OGR_DS_GetLayerByName( hSrcDS, "fpu_sum" );
    hLayer = OGR_DS_CopyLayer( hDS, hSrcLayer, "fpu_sum", papszOptions );
    hSrcLayer = OGR_DS_GetLayerByName( hSrcDS, "gacc_sum" );
    hLayer = OGR_DS_CopyLayer( hDS, hSrcLayer, "gacc_sum", papszOptions );
    OGR_DS_Destroy( hSrcDS );
    OGR_DS_Destroy( hDS );

    return IRS_OK;
}

int IRSResult::ExportFpuSummaryCsv( const char *pszOutPath )
{
    int rc;
    sqlite3_stmt *stmt = NULL;
    VSILFILE *fout;
    fout = VSIFOpenL( pszOutPath, "w" );
    if( !fout )
        return IRS_INVALID_INPUT;

    rc = sqlite3_prepare_v2( db, "PRAGMA table_info(fpu_output)", -1, &stmt,
                             NULL );

    rc = sqlite3_step( stmt );
    if( rc != SQLITE_ROW )
    {
        sqlite3_finalize( stmt );
        return IRS_INVALID_DATA;
    }
    sqlite3_reset( stmt );
    /*
    ** Write a header
    */
    int i = 0;
    int j = 0;
    char szFrmt[512];
    szFrmt[0] = '\0';
    int nColCount = 0;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        VSIFPrintfL( fout, "%s,", sqlite3_column_text( stmt , i++ ) );
        snprintf( szFrmt, 512, "%s,%s", szFrmt, "%s" );
        nColCount++;
    }
    VSIFPrintfL( fout, "\n" );
    rc = sqlite3_finalize( stmt ); stmt = NULL;
    rc = sqlite3_prepare_v2( db, "SELECT * FROM fpu_output", -1, &stmt, NULL );
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        i = 0;
        while( i < nColCount )
        {
            VSIFPrintfL( fout, "%s,", sqlite3_column_text( stmt , i++ ) );
            j++;
        }
        VSIFPrintfL( fout, "\n" );
        if( j++ % nColCount * 100 == 0 )
        {
            VSIFFlushL( fout );
        }
    }
    VSIFCloseL( fout );
	return IRS_OK;
}

const char *IRSResult::GetResultPath()
{
	return pszPath;
}

int IRSResult::GetNumYears()
{
    int numYears = 0;

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, "SELECT count(DISTINCT year) FROM fpu_output", -1, &stmt, NULL);
    if (rc != SQLITE_OK) return 0;

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_ROW && rc != SQLITE_DONE) return 0;

    if (sqlite3_column_type(stmt, 0) == SQLITE_INTEGER)
        numYears = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    stmt = NULL;

    return numYears;
}

int IRSResult::GetNumFPUs()
{
    int numFPUs = 0;

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, "SELECT count(DISTINCT fpu_code) FROM fpu_output", -1, &stmt, NULL);
    if (rc != SQLITE_OK) return 0;

    rc = sqlite3_step(stmt);
    if(rc != SQLITE_ROW && rc != SQLITE_DONE) return 0;

    if (sqlite3_column_type(stmt, 0) == SQLITE_INTEGER)
        numFPUs = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);
    stmt = NULL;

    return numFPUs;
}

std::vector<std::pair<std::string, std::list<double> > > IRSResult::FPUDistributions( std::string Attribute )
{
        // Create the std::list for each year's value of the attribute
        std::vector<std::pair<std::string, std::list<double> > > FPUlists;
        std::vector<std::string> FPUs;
        const char *FPU;
        std::list<double> Values;

        int rc = 0;
        int rcc = 0;
        double v = 0.0;

        sqlite3_stmt *stmt = NULL;

        rc = sqlite3_prepare_v2(db, "SELECT DISTINCT fpu_code FROM fpu_output", -1, &stmt, NULL);
        if (rc != SQLITE_OK) return FPUlists;

        while (sqlite3_step(stmt) == SQLITE_ROW)    {   
            if (sqlite3_column_type(stmt, 0) == SQLITE_TEXT)  {
                FPU = (const char*)sqlite3_column_text(stmt, 0);    
                FPUs.push_back(FPU);
            }
        }

        sqlite3_finalize(stmt);
        stmt = NULL;
        sqlite3_stmt *stmtc = NULL;
    
        if (Attribute == "Percent Contained")   {

                rc = sqlite3_prepare_v2(db, "SELECT year, "CONTAIN_COLUMN" " \
                                                              "FROM fpu_output " \
                                                              "WHERE fpu_code = ?1 ", -1, &stmt, NULL );

                rcc = sqlite3_prepare_v2(db, "SELECT year, " \
                                                              SIZE_LIMIT_COLUMN" + "TIME_LIMIT_COLUMN" + " \
                                                              CONTAIN_COLUMN" + "EXHAUST_COLUMN" + " \
                                                              NO_RESC_COLUMN" FROM fpu_output " \
                                                              "WHERE fpu_code = ?1", -1, &stmtc, NULL );
        }

        else if (Attribute == "Large Fire Cost")        {
                
                rc = sqlite3_prepare_v2(db, "SELECT year, "LF_COST_COLUMN" " \
                                                               "FROM fpu_output " \
                                                               "WHERE fpu_code LIKE ?1 ", -1, &stmt, NULL );
        }

        else if (Attribute == "Large Fire Acres")       {

                rc = sqlite3_prepare_v2(db, "SELECT year, "LF_ACRE_COLUMN" " \
                                                               "FROM fpu_output " \
                                                               "WHERE fpu_code LIKE ?1 ", -1, &stmt, NULL );
        }

        else if (Attribute == "Large Fire Population")  {

                rc = sqlite3_prepare_v2(db, "SELECT year, "LF_POP_COLUMN" " \
                                                                "FROM fpu_output " \
                                                                "WHERE fpu_code LIKE ?1 ", -1, &stmt, NULL );
        }

         else if (Attribute == "Resource Usage")  {

                rc = sqlite3_prepare_v2(db, "SELECT year, "RESC_USAGE_COLUMN" " \
                                                                "FROM fpu_output " \
                                                                "WHERE fpu_code LIKE ?1 ", -1, &stmt, NULL );
        }

        if (rc != SQLITE_OK) 
                return FPUlists;
        if (rcc != 0 && rcc != SQLITE_OK)
                return FPUlists;

        for (int i = 0; i < FPUs.size(); i++)    {

                Values.clear();
                rc = sqlite3_bind_text(stmt, 1, FPUs[i].c_str(), -1, NULL );
                if (Attribute == "Percent Contained")
                        rcc = sqlite3_bind_text(stmtc, 1, FPUs[i].c_str(), -1, NULL);

                while (sqlite3_step(stmt) == SQLITE_ROW)        {
                        if (sqlite3_column_type(stmt, 1) == SQLITE_FLOAT)       
                                v = sqlite3_column_double(stmt, 1);
                        else if (sqlite3_column_type(stmt, 1) == SQLITE_INTEGER)
                                v = (double)sqlite3_column_int(stmt, 1);
                        
                        Values.push_back(v);
                }

                if (Attribute == "Percent Contained")   {
                        std::list<double>::iterator it = Values.begin();

                        while (sqlite3_step(stmtc) == SQLITE_ROW)       {
                                if (sqlite3_column_type(stmtc, 1) == SQLITE_INTEGER)
                                        v = (double)sqlite3_column_int(stmtc, 1);

                                if (it != Values.end()) {
                                        (*it) = (*it) / v;
                                        it++;
                                }

                                else    {
                                        FPUlists.clear();
                                        return FPUlists;
                                }
                        }

                        sqlite3_reset(stmtc);
                }

                sqlite3_reset(stmt);
                std::pair<std::string, std::list<double> > FPUpair = std::make_pair(std::string(FPUs[i]), Values);
                FPUlists.push_back(FPUpair);

        }

        sqlite3_finalize(stmt);
        if (Attribute == "Percent Contained")
                sqlite3_finalize(stmtc);

        return FPUlists;
}

std::vector<std::pair<std::string, double> >IRSResult::FPUSeriesAvg( std::string Attribute )
{
    // Create the std::list for each year's value of the attribute
    std::vector<std::pair<std::string, double> > FPUValues;
    std::vector<std::string> FPUs;
    std::list<double> Values;

    int rc = 0;
    int numYears = 0;
    double v = 0.0;

    sqlite3_stmt *stmty = NULL;
    
	rc = sqlite3_prepare_v2(db, "SELECT count() FROM (SELECT DISTINCT year FROM fpu_output)", -1, &stmty, NULL);

	if (rc != SQLITE_OK) return FPUValues;
	
	rc = sqlite3_step(stmty);
	if (rc != SQLITE_ROW &&rc != SQLITE_DONE) return FPUValues;
	if (sqlite3_column_type(stmty, 0) == SQLITE_INTEGER)
		numYears = sqlite3_column_int(stmty, 0);

	sqlite3_finalize(stmty);

	sqlite3_stmt *stmt = NULL;
	    
    if (Attribute == "Percent Contained")   
		rc = sqlite3_prepare_v2(db, "SELECT fpu_code, "PERC_CONT_COLUMN" " \
										"FROM fpu_sum", -1, &stmt, NULL );

    else if (Attribute == "Large Fire Cost")        
        rc = sqlite3_prepare_v2(db, "SELECT fpu_code, "LF_COST_COLUMN" " \
										"FROM fpu_sum", -1, &stmt, NULL );
        
	else if (Attribute == "Large Fire Acres")       
		rc = sqlite3_prepare_v2(db, "SELECT fpu_code, "LF_ACRE_COLUMN" " \
                                         "FROM fpu_sum", -1, &stmt, NULL );

    else if (Attribute == "Large Fire Population")  
		rc = sqlite3_prepare_v2(db, "SELECT fpu_code, "LF_POP_COLUMN" " \
                                          "FROM fpu_sum", -1, &stmt, NULL );

	else if (Attribute == "Resource Usage")  
		rc = sqlite3_prepare_v2(db, "SELECT fpu_code, "RESC_USAGE_COLUMN" " \
                                          "FROM fpu_sum", -1, &stmt, NULL );

    if (rc != SQLITE_OK) 
        return FPUValues;
    

    while (sqlite3_step(stmt) == SQLITE_ROW)    {

        std::string strFPU;
		double value;

		if (sqlite3_column_type(stmt, 0) == SQLITE_TEXT)	
			strFPU = (const char*)sqlite3_column_text(stmt, 0);
		if (sqlite3_column_type(stmt, 1) == SQLITE_FLOAT)
			value = sqlite3_column_double(stmt, 1);

		FPUValues.push_back(std::make_pair(strFPU, value));
    }

    sqlite3_finalize(stmt);
        
    return FPUValues;

}

std::vector<std::string>IRSResult::ResultGACCs() 
{
    std::vector<std::string> GACCs;

    sqlite3_stmt *stmt = NULL;

    int rc = sqlite3_prepare_v2(db, "SELECT DISTINCT gacc_code FROM gacc_sum", -1, &stmt, NULL);
    if (rc != SQLITE_OK) return GACCs;
    
    while (sqlite3_step(stmt) == SQLITE_ROW)    {
        if (sqlite3_column_type(stmt, 0) == SQLITE_TEXT)    {
            const char* gacc = (const char*)sqlite3_column_text(stmt, 0);
            GACCs.push_back(gacc);
        }
    }

    return GACCs;
}

const char * IRSResult::GetRemovedAsSet()
{
    int rc;
    int nCount;
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2( db, "SELECT COUNT() FROM resc_remove", -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    nCount = sqlite3_column_int( stmt, 0 );
    sqlite3_finalize( stmt ); stmt = NULL;
    sqlite3_prepare_v2( db, "SELECT resc_id FROM resc_remove", -1, &stmt, NULL );
    char *pszResc = (char*)sqlite3_malloc( nCount * 10 + 512 );
    pszResc[0] = '\0';
    int i = 0;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        strcat( pszResc, (const char*)sqlite3_column_text( stmt, 0 ) );
        i++;
    }
    sqlite3_finalize( stmt ); stmt = NULL;
    pszResc[i] = '\0';
    return pszResc;
}

int IRSResult::GetRunId()
{
    return nRunId;
}

void IRSResult::SetRunId( int nNewId )
{
    assert( nNewId > 0 );
    nRunId = nNewId;
}

