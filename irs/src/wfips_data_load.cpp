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
    sqlite3_prepare_v2( db, "SELECT * FROM resc_type", -1, &stmt,
                        NULL );

    const char *pszName;
    int nSpeed, nDispDelay, nRespDelay, nSetupTime;
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszName = (const char*)sqlite3_column_text( stmt, 0 );
        nSpeed = sqlite3_column_int( stmt, 1 );
        nDispDelay = sqlite3_column_int( stmt, 2 );
        nRespDelay = sqlite3_column_int( stmt, 3 );
        nSetupTime = sqlite3_column_int( stmt, 4 );
        poScenario->m_VRescType.push_back( CRescType( pszName, nSpeed,
                                                      nDispDelay, nRespDelay,
                                                      nSetupTime ) );
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
    rc = sqlite3_prepare_v2( db, "SELECT * FROM prod_rate", -1, &stmt,
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
        poScenario->m_VProdRates.push_back( CProdRates( pszName, nSlope, 
                                                        nStaff, nFuel,
                                                        pszSpecCond, dfRate ) );
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
    int rc, i, j, n;
    void *pGeom;
    if( pszAnalysisAreaWkt )
    {
        n = CompileGeometry( pszAnalysisAreaWkt, &pGeom );
        if( n > 0 )
        {
            rc = sqlite3_prepare_v2( db, "SELECT displog.name,indice,num_lev,bp_1," \
                                         "bp_2,bp_3,bp_4 FROM fwa JOIN displog " \
                                         "ON fwa.displogic_name=displog.name " \
                                         "JOIN brk_point ON " \
                                         "displog.name=brk_point.name " \
                                         "WHERE ST_Intersects(@geom, geometry) " \
                                         "AND fwa.ROWID IN " \
                                         "(SELECT pkid FROM " \
                                         "idx_fwa_geometry WHERE " \
                                         "xmin <= MbrMaxX(@geom) AND " \
                                         "xmax >= MbrMinX(@geom) AND " \
                                         "ymin <= MbrMaxY(@geom) AND " \
                                         "ymax >= MbrMinY(@geom)) " \
                                         "group by displog.name",
                                     -1, &stmt, NULL );

            rc = sqlite3_bind_blob( stmt, sqlite3_bind_parameter_index( stmt, "@geom" ),
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
        rc = sqlite3_prepare_v2( db, "SELECT name,indice,num_lev,bp_1,bp_2, " \
                                     "bp_3,bp_4 FROM " \
                                     "displog JOIN brk_point USING(name)",
                                 -1, &stmt, NULL );
    }
    rc = sqlite3_prepare_v2( db, "SELECT * FROM  num_resc WHERE name=?", -1,
                             &rstmt, NULL );

    const char *pszName, *pszIndice;
    int nLevels, anBps[4];
    int nBp, anRescCount[13][5];
    while( sqlite3_step( stmt ) == SQLITE_ROW )
    {
        pszName = (const char*)sqlite3_column_text( stmt, 0 );
        pszIndice = (const char*)sqlite3_column_text( stmt, 1 );
        nLevels = sqlite3_column_int( stmt, 2 );
        memset( anBps, 0, sizeof( int ) * 4 );
        for( i = 0; i < nLevels; i++ )
            anBps[i] = sqlite3_column_int( stmt, 3 + i );
        sqlite3_bind_text( rstmt, 1, pszName, -1, NULL );
        i = 0;
        memset( anRescCount, 0, sizeof( int ) * 13 * 5 );
        while( sqlite3_step( rstmt ) == SQLITE_ROW )
        {
            for( j = 0; j < 13; j++ )
            {
                anRescCount[j][i] = sqlite3_column_int( stmt, j+2 );
            }
            i++;
        }
        poScenario->m_VDispLogic.push_back( CDispLogic( std::string( pszName ),
                                            std::string( pszIndice ), nLevels,
                                            anBps, anRescCount ) );
        sqlite3_reset( rstmt );
    }
    sqlite3_finalize( stmt );
    sqlite3_finalize( rstmt );
    return 0;
}
int
WfipsData::LoadFwas()
{
    return 0;
}
int
WfipsData::LoadDispatchLocations()
{
    return 0;
}
int
WfipsData::LoadTankerBases()
{
    return 0;
}
int
WfipsData::LoadResources()
{
    return 0;
}
int
WfipsData::CreateLargeAirTankers()
{
    return 0;
}

int
WfipsData::LoadIrsStructs( const char *pszAnalysisAreaWkt )
{
    SetAnalysisAreaMask( pszAnalysisAreaWkt );
    poScenario = new CRunScenario();
    LoadRescTypes();
    LoadProdRates();
    LoadDispatchLogic();
    LoadFwas();
    LoadDispatchLocations();
    LoadTankerBases();
    LoadResources();
    CreateLargeAirTankers();
    return 0;
}

