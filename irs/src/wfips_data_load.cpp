/****************************************************************************t*
 *
 * Project:  Wildland Fire Investment Planning System
 * Purpose:  Access on disk data
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

#include "wfips_data.h"

int
WfipsData::LoadRescTypes()
{
    sqlite3_stmt *stmt;
    poScenario->m_VRescType.clear();
    sqlite3_prepare_v2( db, "SELECT * FROM resc_type", -1, &stmt,
                        NULL );
    poScenario->m_VRescType.clear();
    const char *pszName;
    int nSpeed, nDispDelay, nRespDelay, nSetupTime;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszName = (const char*)sqlite3_column_text( stmt, 0 );
        assert( pszName );
        nSpeed = sqlite3_column_int( stmt, 1 );
        assert( nSpeed >= 0 );
        nDispDelay = sqlite3_column_int( stmt, 2 );
        assert( nDispDelay >= 0 );
        nRespDelay = sqlite3_column_int( stmt, 3 );
        assert( nRespDelay >= 0 );
        nSetupTime = sqlite3_column_int( stmt, 4 );
        assert( nSetupTime >= 0 );
        poScenario->m_VRescType.push_back( CRescType( std::string( pszName ),
                                                      nSpeed, nDispDelay,
                                                      nRespDelay, nSetupTime ) );
    }

    sqlite3_finalize( stmt );
    stmt = NULL;
    return 0;
}
int
WfipsData::LoadProdRates()
{
    int rc;
    sqlite3_stmt *stmt;
    poScenario->m_VProdRates.clear();
    rc = sqlite3_prepare_v2( db, "SELECT * FROM prod_rate", -1, &stmt,
                             NULL );

    const char *pszName, *pszSpecCond;
    int nSlope, nStaff, nFuel;
    double dfRate;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszName = (const char*)sqlite3_column_text( stmt, 0 );
        assert( pszName );
        nSlope = sqlite3_column_int( stmt, 1 );
        assert( nSlope >= 0 );
        nStaff = sqlite3_column_int( stmt, 2 );
        assert( nStaff >= 0 );
        nFuel = sqlite3_column_int( stmt, 3 );
        assert( nFuel > 0 );
        pszSpecCond = (const char*)sqlite3_column_text( stmt, 4 );
        /* No assertion ? */
        dfRate = sqlite3_column_double( stmt, 5 );
        assert( dfRate >= 0 );
        poScenario->m_VProdRates.push_back( CProdRates( std::string( pszName ),
                                                        nSlope, nStaff, nFuel,
                                                        std::string( pszSpecCond ),
                                                        dfRate ) );
    }
    sqlite3_finalize( stmt );
    stmt = NULL;
    return 0;
}

int
WfipsData::LoadDispatchLogic()
{
    sqlite3_stmt *stmt;
    sqlite3_stmt *rstmt;
    int rc, i, j, n, iLogic;
    void *pGeom;

    /* Clear our lookup */
    DispLogIndexMap.clear();

    poScenario->m_VDispLogic.clear();

    if( pszAnalysisAreaWkt )
    {
        n = CompileGeometry( pszAnalysisAreaWkt, &pGeom );
        if( n > 0 )
        {
            rc = sqlite3_prepare_v2( db, "SELECT displog.name,indice,num_lev,bp_1,"
                                         "bp_2,bp_3,bp_4 FROM fwa JOIN displog "
                                         "ON fwa.displogic_name=displog.name "
                                         "JOIN brk_point ON "
                                         "displog.name=brk_point.name "
                                         "WHERE ST_Intersects(@geom, geometry) "
                                         "AND fwa.name NOT LIKE '%unassign%' "
                                         "AND substr(fwa.name, 0, 3) NOT IN "
                                         "('EA','SA','AK') "
                                         "AND fwa.ROWID IN "
                                         "(SELECT pkid FROM "
                                         "idx_fwa_geometry WHERE "
                                         "xmin <= MbrMaxX(@geom) AND "
                                         "xmax >= MbrMinX(@geom) AND "
                                         "ymin <= MbrMaxY(@geom) AND "
                                         "ymax >= MbrMinY(@geom)) "
                                         "group by displog.name",
                                     -1, &stmt, NULL );

            rc = sqlite3_bind_blob( stmt,
                                    sqlite3_bind_parameter_index( stmt, "@geom" ),
                                    pGeom, n, sqlite3_free );
        }
        else
        {
            /* Geometry is bad */
            return SQLITE_ERROR;
        }
    }
    else
    {
        rc = sqlite3_prepare_v2( db, "SELECT displog.name,indice,"
                                     "num_lev,bp_1,bp_2,bp_3,bp_4 FROM "
                                     "fwa JOIN displog ON "
                                     "fwa.displogic_name=displog.name "
                                     "JOIN brk_point ON "
                                     "displog.name=brk_point.name "
                                     "WHERE fwa.name NOT LIKE '%unassign%' "
                                     "AND substr(fwa.name, 0, 3) NOT IN "
                                     "('EA','SA','AK') "
                                     "GROUP BY displog.name",
                                 -1, &stmt, NULL );
    }
    rc = sqlite3_prepare_v2( db, "SELECT * FROM  num_resc WHERE name=?", -1,
                             &rstmt, NULL );

    const char *pszName, *pszIndice;
    int nLevels, anBps[4];
    int nBp, anRescCount[13][5];
    iLogic = 0;

    /* Push a default displogic on so we avoid errors */
    poScenario->m_VDispLogic.push_back( CDispLogic() );
    DispLogIndexMap.insert( std::pair<std::string, int>( poScenario->m_VDispLogic[0].GetLogicID(), iLogic++ ) );

    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszName = (const char*)sqlite3_column_text( stmt, 0 );
        assert( pszName );
        pszIndice = (const char*)sqlite3_column_text( stmt, 1 );
        /* No assert currently, but do we assume BI? */
        nLevels = sqlite3_column_int( stmt, 2 );
        assert( nLevels >= 3 && nLevels <= 5 );
        memset( anBps, 0, sizeof( int ) * 4 );
        for( i = 0; i < nLevels - 1; i++ )
        {
            anBps[i] = sqlite3_column_int( stmt, 3 + i );
        }
        sqlite3_bind_text( rstmt, 1, pszName, -1, NULL );
        i = 0;
        memset( anRescCount, 0, sizeof( int ) * 13 * 5 );
        while( sqlite3_step( rstmt ) == SQLITE_ROW )
        {
            for( j = 0; j < 13; j++ )
            {
                anRescCount[j][i] = sqlite3_column_int( rstmt, j+2 );
                assert( anRescCount[j][i] >= 0 );
                assert( i < 5 );
            }
            i++;
        }
        poScenario->m_VDispLogic.push_back( CDispLogic( std::string( pszName ),
                                            std::string( pszIndice ), nLevels,
                                            anBps, anRescCount ) );
        DispLogIndexMap.insert( std::pair<std::string, int>( pszName, iLogic++ ) );
        sqlite3_reset( rstmt );
    }
    assert( DispLogIndexMap.size() == poScenario->m_VDispLogic.size() );
    sqlite3_finalize( stmt );
    sqlite3_finalize( rstmt );
    return 0;
}

int
WfipsData::LoadFwas()
{
    //assert( poScenario->m_VDispLogic.size() > 0 );
    sqlite3_stmt *stmt;
    int rc, n, i;
    void *pGeom;

    /* Clear our lookup */
    FwaIndexMap.clear();

    poScenario->m_VFWA.clear();

    if( pszAnalysisAreaWkt )
    {
        n = CompileGeometry( pszAnalysisAreaWkt, &pGeom );
        if( n > 0 )
        {
            rc = sqlite3_prepare_v2( db, "SELECT * FROM fwa JOIN reload ON "
                                         "fwa.name=reload.fwa_name JOIN "
                                         "walk_in ON "
                                         "reload.fwa_name=walk_in.fwa_name "
                                         "WHERE ST_Intersects(@geom, geometry) "
                                         "AND name NOT LIKE '%unassigned%' "
                                         "AND substr(fwa.name, 0, 3) NOT IN "
                                         "('EA','SA','AK') "
                                         "AND fwa.ROWID IN "
                                         "(SELECT pkid FROM "
                                         "idx_fwa_geometry WHERE "
                                         "xmin <= MbrMaxX(@geom) AND "
                                         "xmax >= MbrMinX(@geom) AND "
                                         "ymin <= MbrMaxY(@geom) AND "
                                         "ymax >= MbrMinY(@geom))",
                                     -1, &stmt, NULL );

            rc = sqlite3_bind_blob( stmt,
                                    sqlite3_bind_parameter_index( stmt, "@geom" ),
                                    pGeom, n, sqlite3_free );
        }
        else
        {
            return SQLITE_ERROR;
        }
    }
    else
    {
        rc = sqlite3_prepare_v2( db, "SELECT * FROM fwa JOIN reload ON "
                                     "fwa.name=reload.fwa_name JOIN "
                                     "walk_in ON "
                                     "reload.fwa_name=walk_in.fwa_name "
                                     "WHERE fwa.name NOT LIKE '%unassigned%' "
                                     "AND substr(fwa.name, 0, 3) NOT IN "
                                     "('EA','SA','AK')",
                                 -1, &stmt, NULL );
    }
    const char *pszName, *pszFpu;
    int nWalkIn, nPumpRoll, nHead, nTail, nPara;
    double dfAttDist;
    int bWaterDrops, bExcluded;
    double dfDiscSize, dfEslSize, dfEslTime, dfAirGrnd;
    int nFirstDelay;
    const char *pszLogic;
    int anReload[5];
    int anWalkIn[6];
    int iFwa = 0;
    std::map<std::string, int>::iterator it;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszName = (const char *)sqlite3_column_text( stmt, 1 );
        assert( pszName );
        pszFpu = (const char *)sqlite3_column_text( stmt, 2 );
        assert( pszFpu );
        // FMG 3
        nWalkIn = sqlite3_column_int( stmt, 4 );
        assert( nWalkIn >= 0 || nWalkIn <= 100 );
        nPumpRoll = sqlite3_column_int( stmt, 5 );
        assert( nPumpRoll >= 0 || nPumpRoll <= 100 );
        nHead = sqlite3_column_int( stmt, 6 );
        assert( nHead >= 0 || nHead <= 100 );
        nTail = sqlite3_column_int( stmt, 7 );
        assert( nTail >= 0 || nTail <= 100 );
        nPara = sqlite3_column_int( stmt, 8 );
        assert( nPara >= 0 || nPara <= 100 );
        assert( nPara + nTail + nHead < 101 );
        dfAttDist = sqlite3_column_double( stmt, 9 );
        assert( dfAttDist >= 0.0 );
        bWaterDrops = sqlite3_column_int( stmt, 10 );
        assert( bWaterDrops == 0 || bWaterDrops == 1 );
        bExcluded = sqlite3_column_int( stmt, 11 );
        assert( bExcluded == 0 || bExcluded == 1 );
        dfDiscSize = sqlite3_column_double( stmt, 12 );
        assert( dfDiscSize >= 0 );
        dfEslTime = sqlite3_column_double( stmt, 13 );
        assert( dfEslTime >= 0 );
        dfEslSize = sqlite3_column_double( stmt, 14 );
        assert( dfEslSize >= 0 );
        dfAirGrnd = sqlite3_column_double( stmt, 15 );
        assert( dfAirGrnd >= 0 );
        nFirstDelay = sqlite3_column_int( stmt, 16 );
        assert( nFirstDelay >= 0 );
        pszLogic = (const char *)sqlite3_column_text( stmt, 17 );
        assert( pszLogic );
        memset( anReload, 0, sizeof( int ) * 5 );
        for( i = 0; i < 5; i++ )
        {
            anReload[i] = sqlite3_column_int( stmt, 18+i );
            assert( anReload[i] >= 0 );
        }
        memset( anWalkIn, 0, sizeof( int ) * 6 );
        for( i = 0; i < 6; i++ )
        {
            anWalkIn[i] = sqlite3_column_int( stmt, 18+5+i );
            assert( anWalkIn[i] >= 0 );
        }
        /*
        ** We use defaults now for some delays: From FPA:
        **
        ** Post Escape Delay 20 minutes for all but smokejumpers, smokejumpers 
        ** 120 minutes.
        ** Post Unused Delay 10 minutes for all.
        ** Post Used Delay 30 minutes for all but boats and smokejumpers, boats
        ** and smokejumpers 120 minutes.
        **
        ** Columns:
        ** tracked,boat,crew,engine,helitack,smkjmp
        **/
        int anPostEscape[6] = {20,20, 20,20,20,120};
        int anPostUnused[6] = {10,10, 10,10,10,10};
        int anPostUsed[6]   = {30,120,30,30,30,120};
        /* Fixed, no assert */


        /*
        ** These values have also been defaulted.
        */
        std::string aoRos[10];
        double adfRosCoeff[10];
        double adfDiurn[24];
        for( i = 0; i < 10; i++ )
        {
            aoRos[i] = std::string( "NA" );
            adfRosCoeff[i] = 1.0;
        }
        /* Fixed, no assert */
        for( i = 0; i < 24; i++ )
        {
            adfDiurn[i] = 1.;
        }
        /* Fixed, no assert */

        /*
        ** The value of this is up for discussion...
        */
        it = DispLogIndexMap.find( pszLogic );
        if( it == DispLogIndexMap.end() )
        {
            i = 0;
            printf("Failed to load logic: %s for fwa: %s\n", pszLogic, pszName);
        }
        else
        {
            i = it->second;
        }
        poScenario->m_VFWA.push_back( CFWA( std::string( pszName ),
                                            std::string( "" ), nWalkIn,
                                            nPumpRoll, nHead, nTail, nPara,
                                            dfAttDist, bWaterDrops, bExcluded,
                                            dfDiscSize, dfEslTime, dfEslSize,
                                            dfAirGrnd, anWalkIn, anPostUsed,
                                            anPostUnused, anPostEscape, anReload,
                                            nFirstDelay, adfDiurn, aoRos,
                                            adfRosCoeff, iFwa,
                                            poScenario->m_VDispLogic[i],
                                            std::string( pszFpu ) ) );
        FwaIndexMap.insert( std::pair<std::string, int>( std::string( pszName ),
                                                         iFwa ) );
        iFwa++;
    }
    assert( FwaIndexMap.size() == poScenario->m_VFWA.size() );
    sqlite3_finalize( stmt );
    return 0;
}

int
WfipsData::LoadDispatchLocations()
{
    sqlite3_stmt *stmt, *astmt, *estmt;
    int rc, n, i;
    void *pGeom = NULL;
    int nFwaCount;

    poScenario->m_VDispLoc.clear();

    if( pszAnalysisAreaWkt )
    {
        n = CompileGeometry( pszAnalysisAreaWkt, &pGeom );
        if( n > 0 )
        {
            rc = sqlite3_prepare_v2( db, "SELECT disploc.name,fpu_code,"
                                         "callback,X(disploc.geometry),"
                                         "Y(disploc.geometry) FROM "
                                         "disploc JOIN assoc ON "
                                         "name=disploc_name "
                                         "WHERE fwa_name NOT LIKE '%unassign%' "
                                         "AND substr(fwa_name, 0, 3) NOT IN "
                                         "('EA','SA','AK') "
                                         "AND fwa_name IN "
                                         "(SELECT name FROM fwa WHERE "
                                         "ST_Intersects(@geom, fwa.geometry) AND "
                                         "fwa.ROWID IN(SELECT pkid FROM "
                                         "idx_fwa_geometry WHERE "
                                         "xmin <= MbrMaxX(@geom) AND "
                                         "xmax >= MbrMinX(@geom) AND "
                                         "ymin <= MbrMaxY(@geom) AND "
                                         "ymax >= MbrMinY(@geom))) GROUP BY "
                                         "disploc.name",
                                     -1, &stmt, NULL );

            rc = sqlite3_bind_blob( stmt,
                                    sqlite3_bind_parameter_index( stmt, "@geom" ),
                                    pGeom, n, sqlite3_free );
        }
        else
        {
            return SQLITE_ERROR;
        }
    }
    else
    {
        rc = sqlite3_prepare_v2( db, "SELECT name,fpu_code,callback,"
                                     "X(geometry), Y(geometry) FROM disploc "
                                     "JOIN assoc ON name=disploc_name WHERE "
                                     "fwa_name NOT LIKE '%unassign%' "
                                     "AND substr(fwa_name, 0, 3) NOT IN "
                                     "('EA','SA','AK') "
                                     "GROUP BY disploc.name",
                                 -1, &stmt, NULL );
    }
    rc = sqlite3_prepare_v2( db, "SELECT fwa_name, distance FROM assoc WHERE "
                                 "disploc_name=?",
                             -1, &astmt, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT COUNT(*) FROM resource WHERE disploc=?",
                             -1, &estmt, NULL );
    nFwaCount = poScenario->m_VFWA.size();

    const char *pszName, *pszFpu;
    const char *pszFwa;
    int nCallBack;
    double dfX, dfY;
    double dfDist;
    std::map<std::string, int>::iterator it;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszName = (const char*)sqlite3_column_text( stmt, 0 );
        assert( pszName );
        rc = sqlite3_bind_text( estmt, 1, pszName, -1, NULL );
        if( sqlite3_step( estmt ) != SQLITE_ROW ||
            sqlite3_column_int( estmt, 0 ) < 1 )
        {
            sqlite3_reset( estmt );
            continue;
        }
        pszFpu = (const char*)sqlite3_column_text( stmt, 1 );
        assert( pszFpu );
        nCallBack = sqlite3_column_int( stmt, 2 );
        assert( nCallBack >= 0 );
        dfX = sqlite3_column_double( stmt, 3 );
        assert( dfX > -180 && dfX < 360 );
        dfY = sqlite3_column_double( stmt, 4 );
        assert( dfY > -180 && dfY < 360 );
        CDispLoc oDispLoc( std::string( pszName ), nCallBack,
                           std::string( pszFpu ), dfY, dfX );

        rc = sqlite3_bind_text( astmt, 1, pszName, -1, NULL );
        while( sqlite3_step( astmt ) == SQLITE_ROW )
        {
            pszFwa = (const char*)sqlite3_column_text( astmt, 0 );
            assert( pszFwa );
            dfDist = sqlite3_column_double( astmt, 1 );
            assert( dfDist >= 0 );
            it = FwaIndexMap.find( std::string( pszFwa ) );
            if( it != FwaIndexMap.end() )
            {
                i = it->second;
                poScenario->m_VFWA[i].AddAssociation( std::string( pszName ), dfDist );
                oDispLoc.AddAssocFWA( &(poScenario->m_VFWA[i]) );
            }
        }
        sqlite3_reset( astmt );
        sqlite3_reset( estmt );
        poScenario->m_VDispLoc.push_back( oDispLoc );
    }

    sqlite3_finalize( stmt );
    sqlite3_finalize( astmt );
    sqlite3_finalize( estmt );

	poScenario->CreateRescTypeVectors("C:/wfips/data/");

    return 0;
}
int
WfipsData::LoadTankerBases()
{
    sqlite3_stmt *stmt;
    int rc;
    return 0;
}
/*
** Load resources from disk.  If we have edited the resources and saved them,
** does that mean we just get to load all of the resources?
*/
int
WfipsData::LoadResources()
{
    sqlite3_stmt *stmt;
    int rc, i, j, n;
    const char *pszName, *pszType;
    int nStaffing;
    const char *pszStartTime, *pszEndTime, *pszStartDay, *pszEndDay;
    int nStartSeason, nEndSeason;
    const char *pszDispLocName;
    int nPercAvail = 100;
    double dfDayCost, dfHourCost;
    dfDayCost = dfHourCost = 0;
    int nVolume, bSeverity;
    const char *pszVersion;
    pszVersion = "A";
    double dfFte = 0;
    double dfNumPositions = 0;
    double dfAnnualCost = 0;
    double dfVehicleCost = 0;
    std::string osRescDispLoc;

    std::multimap<std::string, CResource*>resc_map;

    /* Fix query for explicit columns */
    rc = sqlite3_prepare_v2( db, "SELECT * FROM resource WHERE disploc=?", -1,
                             &stmt, NULL );

    poScenario->m_VResource.clear();

    for( i = 0; i < poScenario->m_VDispLoc.size(); i++ )
    {
        osRescDispLoc = poScenario->m_VDispLoc[i].GetDispLocID();
        rc = sqlite3_bind_text( stmt, 1, osRescDispLoc.c_str(), -1, SQLITE_TRANSIENT );
        while( sqlite3_step( stmt ) == SQLITE_ROW )
        {
            pszName = (const char*)sqlite3_column_text( stmt, 0 );
            assert( pszName );
            pszType = (const char*)sqlite3_column_text( stmt, 1 );
            assert( pszType );
            nStaffing = sqlite3_column_int( stmt, 2 );
            /* Maybe should be > 0 */
            assert( nStaffing >= 0 );
            pszStartTime = (const char*)sqlite3_column_text( stmt, 3 );
            assert( pszStartTime && atoi( pszStartTime ) >= 0 && atoi( pszStartTime ) < 2400 );
            pszEndTime = (const char*)sqlite3_column_text( stmt, 4 );
            assert( pszEndTime && atoi( pszEndTime ) >= 0 && atoi( pszEndTime ) < 2400 );
            /*
            ** XXX Add better checks.
            */
            pszStartDay = apszWfipsDayOfWeek[sqlite3_column_int( stmt, 5 )];
            pszEndDay = apszWfipsDayOfWeek[sqlite3_column_int( stmt, 6 )];
            nStartSeason = sqlite3_column_int( stmt, 7 );
            assert( nStartSeason > 0 && nStartSeason <= 365 );
            nEndSeason = sqlite3_column_int( stmt, 8 );
            assert( nEndSeason > 0 && nEndSeason <= 365 );
            nVolume = sqlite3_column_int( stmt, 10 );
            assert( nVolume >= 0 );
            bSeverity = sqlite3_column_int( stmt, 11 );
            assert( bSeverity == 0 || bSeverity == 1 );

            /* We need a resc type object for the resource constructor */
            j = 0;
            while( j < poScenario->m_VRescType.size() )
            {
                if( std::string( pszType ) == poScenario->m_VRescType[j].GetRescType() )
                    break;
                j++;
            }
            if( j == poScenario->m_VRescType.size() )
            {
                printf( "Could not find resource type!\n" );
                continue;
            }

            // Construct CResource object of the appropriate type
            if( EQUALN( pszType, "DZR", 3 ) ||
                EQUALN( pszType, "TP", 2 ) )
            {
                poScenario->m_VResource.push_back( new CConstProd( pszName,
                                                   poScenario->m_VRescType[j],
                                                   nStaffing, pszStartTime,
                                                   pszEndTime, pszStartDay, 
                                                   pszEndDay, nStartSeason,
                                                   nEndSeason, poScenario->m_VDispLoc[i],
                                                   nPercAvail, dfDayCost,
                                                   dfHourCost ) );

            }
            else if( EQUAL( pszType, "CRW" ) )
            {
                poScenario->m_VResource.push_back( new CCrew( pszName,
                                                   poScenario->m_VRescType[j], nStaffing,
                                                   pszStartTime, pszEndTime, pszStartDay,
                                                   pszEndDay, nStartSeason,
                                                   nEndSeason, poScenario->m_VDispLoc[i], nPercAvail,
                                                   dfDayCost, dfHourCost ) );
            }
            else if( EQUALN( pszType, "EN", 2 ) )
            {
                assert( nVolume > 0 );
                assert( nStaffing < 6 );
                poScenario->m_VResource.push_back( new CEngine( pszName,
                                                   poScenario->m_VRescType[j], nStaffing,
                                                   pszStartTime, pszEndTime, 
                                                   pszStartDay, pszEndDay, 
                                                   nStartSeason, nEndSeason, 
                                                   poScenario->m_VDispLoc[i], nPercAvail, dfDayCost,
                                                   dfHourCost, nVolume ) );
            }
            else if( EQUAL( pszType, "WT" ) )
            {
                 poScenario->m_VResource.push_back( new CWaterTender( pszName,
                                                    poScenario->m_VRescType[j],
                                                    nStaffing, pszStartTime,
                                                    pszEndTime, pszStartDay,
                                                    pszEndDay, nStartSeason,
                                                    nEndSeason, poScenario->m_VDispLoc[i],
                                                    nPercAvail, dfDayCost,
                                                    dfHourCost ) );
            }
            else if( EQUAL( pszType, "ATT" ) )
            {
                poScenario->m_VResource.push_back( new CAirtanker( pszName,
                                                   poScenario->m_VRescType[j],
                                                   nStaffing, pszStartTime,
                                                   pszEndTime, pszStartDay,
                                                   pszEndDay, nStartSeason,
                                                   nEndSeason, poScenario->m_VDispLoc[i],
                                                   nPercAvail, dfDayCost,
                                                   dfHourCost, nVolume ) );
            }
            else if( EQUAL( pszType, "SEAT" ) || EQUAL( pszType, "SCP" ) )
            {
                poScenario->m_VResource.push_back( new CSmallAT( pszName,
                                                   poScenario->m_VRescType[j], 
                                                   nStaffing, pszStartTime,
                                                   pszEndTime, pszStartDay, pszEndDay,
                                                   nStartSeason, nEndSeason,
                                                   poScenario->m_VDispLoc[i], nPercAvail, dfDayCost,
                                                   dfHourCost, nVolume ) );
            }
            else if( EQUAL( pszType, "SMJR" ) )
            {
                poScenario->m_VResource.push_back( new CSmokejumper( pszName,
                                                   poScenario->m_VRescType[j],
                                                   nStaffing, pszStartTime,
                                                   pszEndTime, pszStartDay,
                                                   pszEndDay, nStartSeason,
                                                   nEndSeason, poScenario->m_VDispLoc[i],
                                                   nPercAvail, dfDayCost,
                                                   dfHourCost ) ); 
            }
            else if( EQUAL( pszType, "SJAC" ) )
            {
                 poScenario->m_VResource.push_back( new CSMJAircraft( pszName,
                                                    poScenario->m_VRescType[j],
                                                    nStaffing, pszStartTime,
                                                    pszEndTime, pszStartDay,
                                                    pszEndDay, nStartSeason,
                                                    nEndSeason, poScenario->m_VDispLoc[i],
                                                    nPercAvail, dfDayCost,
                                                    dfHourCost, nVolume ) );
            }
            else if( EQUAL( pszType, "HELI" ) )
            {
                poScenario->m_VResource.push_back( new CHelitack( pszName,
                                                 poScenario->m_VRescType[j],
                                                 nStaffing, pszStartTime,
                                                 pszEndTime, pszStartDay,
                                                 pszEndDay, nStartSeason,
                                                 nEndSeason, poScenario->m_VDispLoc[i],
                                                 nPercAvail, dfDayCost,
                                                 dfHourCost ) );
            }
            else if( EQUALN( pszType, "HEL", 3 ) )
            {
                poScenario->m_VResource.push_back( new CHelicopter( pszName,
                                                   poScenario->m_VRescType[j],
                                                   nStaffing, pszStartTime,
                                                   pszEndTime, pszStartDay,
                                                   pszEndDay, nStartSeason,
                                                   nEndSeason, poScenario->m_VDispLoc[i],
                                                   nPercAvail, dfDayCost,
                                                   dfHourCost, nVolume ) );
                resc_map.insert( std::pair<string, CResource*>( pszName,
                                                                poScenario->m_VResource[poScenario->m_VResource.size()-1] ) );
             }
            //poScenario->m_VResource[resc.size()-1]->SetDbId( nRescDbId );
        }
        sqlite3_reset( stmt );
    }
    rc = AssociateHelitack( resc_map );
    sqlite3_finalize( stmt );

    return 0;
}

int
WfipsData::AssociateHelitack( std::multimap<std::string, CResource*>&resc_map )
{
    sqlite3_stmt *stmt;
    int rc;
    rc = sqlite3_prepare_v2( db, "SELECT heli FROM heli_assign WHERE hel=?",
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

int
WfipsData::CreateLargeAirTankers()
{
    /* Implement later... */
    return 0;
}

#define WFIPS_CHECK if(rc) goto error

int
WfipsData::LoadIrsData( const char *pszAnalysisAreaWkt )
{
    int rc = 0;
    rc = SetAnalysisAreaMask( pszAnalysisAreaWkt );
    WFIPS_CHECK;
    rc = LoadIrsData();
error:
    return rc;
}

int
WfipsData::LoadIrsData()
{
    int rc = 0;
    poScenario = new CRunScenario();
    WFIPS_CHECK;
    rc = LoadRescTypes();
    WFIPS_CHECK;
    rc = LoadProdRates();
    WFIPS_CHECK;
    rc = LoadDispatchLogic();
    WFIPS_CHECK;
    rc = LoadFwas();
    WFIPS_CHECK;
    rc = LoadDispatchLocations();
    WFIPS_CHECK;
    rc = LoadTankerBases();
    WFIPS_CHECK;
    rc = LoadResources();
    WFIPS_CHECK;
    rc = CreateLargeAirTankers();
    WFIPS_CHECK;
    poScenario->m_NumFWA = poScenario->m_VFWA.size();
    poScenario->m_NumRescType = poScenario->m_VRescType.size();
    poScenario->m_NumDispLoc = poScenario->m_VDispLoc.size();
    poScenario->m_NumProdRates = poScenario->m_VProdRates.size();
    poScenario->m_NumResource = poScenario->m_VResource.size();
    rc = !poScenario->CreateDispTree();
    WFIPS_CHECK;
    rc = !poScenario->ResourcesToDispatchers();
    WFIPS_CHECK;
    rc = !poScenario->FWAsFindClosestAirtankerBases();
    WFIPS_CHECK;
error:
    return rc;
}

