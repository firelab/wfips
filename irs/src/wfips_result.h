/******************************************************************************
 *
 * Project:  Wildland Fire Investment Planning System
 * Purpose:  Save results to disk
 * Author:   Kyle Shannon <kyle at pobox dot com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION ( RMRS )
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

#ifndef WFIPS_RESULT_H_
#define WFIPS_RESULT_H_

#include <stdio.h>
#include <stdlib.h>

/* SQLite/Spatialite */
#include <sqlite3.h>

#include "wfips_sqlite.h"

#include "ogr_api.h"

#include "omffr_conv.h"

#include "Results.h"

#define NO_RESC_SENT_STR      "No Resources Sent"
#define TIME_LIMIT_EXCEED_STR "TimeLimitExceeded"
#define SIZE_LIMIT_EXCEED_STR "SizeLimitExceeded"
#define EXHAUSTED_STR         "Exhausted"
#define CONTAINED_STR         "Contained"

static const char *apszEscapes[] = { NO_RESC_SENT_STR,
                                   TIME_LIMIT_EXCEED_STR,
                                   SIZE_LIMIT_EXCEED_STR,
                                   EXHAUSTED_STR,
                                   NULL };

class WfipsResult
{

public:
    WfipsResult( const char *pszPath, const char *pszDataPath );
    ~WfipsResult();

    int Open();
    int Close();

    int StartTransaction();
    int Commit();

    int Valid() {return bValid;}

    int WriteRecord( CResults &oResult );

    int CreateIndices();

    int SimulateLargeFire( int nJulStart, int nJulEnd, double dfNoRescProb,
                           double dfTimeLimitProb, double dfSizeLimitProb,
                           double dfExhaustProb, const char *pszTreatWkt,
                           double dfTreatProb );

    int SpatialSummary( const char *pszKey );

    int EnableVolatile( int bVolatile );

    int ExportFires( const char *pszFile, const char *pszDriver );

private:
    WfipsResult();
    WfipsResult( const WfipsResult &rhs );

    void Init();

    char *pszPath;
    char *pszDataPath;

    int bValid;

    sqlite3 *db;
    sqlite3_stmt *istmt;

    GDALDatasetH hTreatDS;

};

#endif /* WFIPS_RESULT_H_ */

