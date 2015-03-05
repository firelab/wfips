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
WfipsData::LoadDipatchLogic()
{
   /* Declare all sql stmts */
    sqlite3_stmt *fwa_stmt;
    sqlite3_stmt *dlg_stmt;
    sqlite3_stmt *ind_stmt;
    sqlite3_stmt *bkp_stmt;
    sqlite3_stmt *res_stmt;
    sqlite3_exec( db, "BEGIN TRANSACTION", NULL, NULL, NULL );
    /* Prepare */

    /*
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
    sqlite3_prepare_v2( db, "SELECT name FROM indice WHERE value=?", -1,
            &ind_stmt, NULL );
    sqlite3_prepare_v2( db, "SELECT * FROM  brk_point WHERE displog_id=?", -1, 
            &bkp_stmt, NULL );
    sqlite3_prepare_v2( db, "SELECT * FROM  num_resc WHERE displog_id=?", -1,
            &res_stmt, NULL );
    */

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
    LoadDipatchLogic();
    LoadFwas();
    LoadDispatchLocations();
    LoadTankerBases();
    LoadResources();
    CreateLargeAirTankers();
    return 0;
}

