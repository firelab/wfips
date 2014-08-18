/******************************************************************************
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  IRS output file handler
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

#ifndef IRS_RESULT_H_
#define IRS_RESULT_H_

#include <vector>

#include <sqlite3.h>

#include "ogr_api.h"
#include "ogr_srs_api.h"
#include "cpl_conv.h"
#include "cpl_string.h"

#include "irs_const.h"
#include "irs_error.h"
#include "irs_progress.h"

#include "omffr_conv.h"
#include "omffr_rand.h"

#include "Results.h"
#include "RescWorkYear.h"
#include "Rescbase.h"


class IRSResult
{

public:
    IRSResult();
    IRSResult( const char *pszFilePath, const char *pszOmffrData,
               int nMaxConnects, int bReadOnly, int bVolatile,
               char **papszOptions );
    ~IRSResult();

    int CheckForTables();
    int CreateTables();

    inline void SetVerbose( int bVerbose ) { this->bVerbose = bVerbose; }
    inline void SetWriteGeom( int bWriteGeom ) { bWriteFwaGeometry = bWriteGeom; }

    int AddScenarioResults( std::vector<CResults>aoResults );

    int ExportFullFireResults( int nYear, std::vector<CResults>oResults,
                               std::vector<int>oRescUsage );

    int AddResourceWorkLoad( std::vector<CRescWorkYear>aoWorkYear );
    int AddSingleResourceUsage( std::map<std::string, double> oRescUsage );

    int SetRemovedResources( std::vector<CResource*>&oResources,
                             int  *panIndexes,
                             int nCount );
    int SetRemovedResourcesStrings( const char * const *papszIds );
    int SetRemovedResources( const char *pszLastRun );
    int SetRemovedResourcesIds( int *panIds, int nCount );

    int GetRemovedResourceCount();
    int GetRemovedResourceIndexes( int *panIndexes );
    char ** GetRemovedResourceIndexesAsStrings();

    const char * GetMetadataItem( const char *pszKey );
    int SetMetadataItem( const char *pszKey, const char *pszValue );
    char ** GetUnderPerformers( double dfRemoveCount, int bRemoveUnused );
    int DestroyUnderPerformersList( char **pszList );

    int ExportToOgr( int nGeomMask, int nAttributes, const char *pszFormat,
                     const char *pszFilename, IRSProgress pfnProgress );

    int PostProcessLargeFire( int bEscapeTypes, double dfSubSample,
                              unsigned int nSampleSize, double dfTreated,
                              double dfBuffer, int nDaySpan, int nMinSize,
                              IRSProgress pfnProgress );

    char ** GetResultAttributes();
    int FreeResultAttributes( char ** papszAttributes );

    int SummarizeByFpu( IRSProgress pfnProgress );
    int ExportFpuSummary( IRSProgress pfnProgress );
    int ExportGaccSummary();

    int Reset();
    std::list<double> NationalDistribution( std::string Attribute );
    std::vector<std::pair<std::string, std::list<double> > > GACCDistributions( std::string Attribute );
    std::vector<std::pair<std::string, std::list<double> > > FPUDistributions( std::string Attribute );
    pair<double, double> MinMaxValues( std::string Attribute );
    pair<double, double> FPUMinMaxValues( std::string Attribute );
	std::vector<std::pair<std::string, double> >GACCSeriesAvg( std::string Attribute );
    std::vector<std::pair<std::string, double> >FPUSeriesAvg( std::string Attribute );
	const char *GetResultPath();
    int GetNumYears();
    int GetNumFPUs();
    std::vector<std::string> ResultGACCs();

    IRSResult * GetSiblingResult( int nDirection );
    std::vector<IRSResult*> GetResultArray( int *nResultCount, int nDirection );
    int DestroySiblingResult( IRSResult * poResult );
    int DestroySiblingResults( std::vector<IRSResult*> papoResults );

    int ExportOgrSummary( const char *pszFormat, const char *pszOutPath,
                          char **papszOptions );
    int ExportFpuSummaryCsv( const char *pszOutPath );
    const char * GetRemovedAsSet();

    int GetRunId();
    void SetRunId( int nNewId );

private:
    const char *pszPath;
    const char *pszOmffrPath;
    sqlite3 *db;
    sqlite3 **conn;
    int nConnectIndex;
    int nConnectUsed;
    int nConnectMax;
    int bVerbose;
    int bVolatile;
    int bWriteFwaGeometry;

    unsigned int nRunId;

    int InitializeNewDb();
    int AddGeometries( int nGeomMask );
    int FreeStringList( char **papszList );

    int IValidate();

};

#endif /* IRS_RESULT_H_ */

