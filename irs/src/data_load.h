/******************************************************************************
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Load data from a given datasource
 * Author:   Kyle Shannon <kyle@pobox.com>
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

#ifndef DATA_LOAD_H_
#define DATA_LOAD_H_

typedef int IRS_Err;

#include <stdlib.h>

#include <assert.h>
#include <set>
#include <utility>
#include <iostream>
#include <string.h>
#ifdef _NOT_DEFINED
#include "omffr_conv.h"
#endif
#include "omffr_rand.h"

#include "RunScenario.h"
#include "DispLoc.h"
#include "Rescbase.h"
#include "Airtanker.h"
#include "Engine.h"
#include "SmallAT.h"
#include "SMJAircraft.h"
#include "Smokejumper.h"
#include "WaterTender.h"
#include "Helicopter.h"
#include "Helitack.h"
#include "DataMaster.h"

#ifdef _NOT_DEFINED
#include "cpl_conv.h"
#include "cpl_port.h"
#include "cpl_string.h"
#endif

#include "irs_const.h"
#include "irs_error.h"
#include "irs_progress.h"

#ifndef EQUAL
#define EQUAL(a,b) strcmp(a,b)==0
#endif
#ifndef EQUALN
#define EQUALN(a,b,n) strncmp(a,b,n)==0
#endif

#ifdef NAD83_EPSG
#undef NAD83_EPSG
#define NAD83_EPSG 4269
#endif

typedef struct _IRSLargeFire
{
    int nAcres;
    int nCost;
    int nWuiAcres;
} IRSLargeFire;

#define IRS_HAVE_SPATIALITE
#ifdef IRS_HAVE_SPATIALITE
#include <sqlite3.h>
#endif

typedef struct _GaccMapping
{
    const char *pszCode;
    const char *pszName;
    int nRegion;
} GaccMapping;

static const GaccMapping GaccMap[] = { { "NR", "Northern Rockies", 1 },
                                       { "RM", "Rocky Mountains", 2 },
                                       { "SW", "Southwest", 3 },
                                       { "GB", "Great Basin", 4 },
                                       { "CA", "California", 5 },
                                       { "NW", "Northwest", 6 },
                                       /* 7 is unused */
                                       { "SA", "Southern", 8 },
                                       { "EA", "Eastern", 9 },
                                       { "AK", "Alaska", 10 },
                                       { NULL, NULL, 0 } };
static const char *IRSDayOfWeek[] = { "Sunday", "Monday",
                                      "Tuesday", "Wednesday",
                                      "Thursday", "Friday",
                                      "Saturday", "Sunday" };
#undef OMFFR_GUI
#ifdef OMFFR_GUI
#include <QObject>
#endif

/**
 * \class IRSDataAccess
 * \brief Define an interface for the data access class.  All access must
 * define the entire interface
 *
 * There are 4 possible candidates(* -> implemented):
 * - Text
 * - SQLite
 * - SQLite/Spatialite*
 * - GDAL/OGR
 *
 * The optimum and most likely access will be using GDAL/OGR.  It requires far
 * less data to be stored on disk (less redundancy).  It will also allow for
 * every *other* type via drivers if the type can be identified by OGR (via
 * CSV and SQLite drivers, along with any other supported format).
 */
class IRSDataAccess
{
#ifdef OMFFR_GUI
    Q_OBJECT
#endif
public:
    static IRSDataAccess * Create( int type, const char *pszSource );
    static IRS_Err * Destroy( IRSDataAccess *poDA );
    IRSDataAccess();
    IRSDataAccess( const char *pszSource );
    virtual ~IRSDataAccess();
    int bSpatialEnabled;
    virtual IRS_Err LoadAllData( CDataMaster *poScenario,
                                 const char *pszExternalDb ) = 0;
    virtual IRS_Err SetSource( const char *pszSource,
                               const char * const *papszOptions );
    virtual IRS_Err LoadDispatchLogic( std::vector<CDispLogic>&logic, 
                                       IRSProgress pfnProgress,
                                       const char * const *papszOptions ) = 0;
    virtual IRS_Err LoadDispatchLocation( std::vector<CFWA>&fwas,
                                          std::vector<CDispLoc>&loc, 
                                          const char * const *papszOptions ) = 0;
     virtual IRS_Err LoadTankerBases( CDataMaster *poScenario,
                                      std::vector<CFWA>&fwas,
                                      std::vector<CDispLoc>&loc, 
                                      double dfDist,
                                      const char * const *papszOptions ) = 0;
    virtual IRS_Err LoadFwa( std::vector<CDispLogic>&logic,
                             std::vector<CFWA>&fwas,
                             IRSProgress pfnProgress,
                             const char * const *papszOptions ) = 0;
    virtual IRS_Err LoadRescType( std::vector<CRescType>&types,
                                  const char * const *papszOptions ) = 0;
    virtual IRS_Err LoadProdRates( std::vector<CProdRates>&rates,
                                   const char * const *papszOptions ) = 0;
    virtual IRS_Err LoadResource( CDataMaster*,
                                  std::vector<CRescType>&types,
                                  std::vector<CDispLoc>&loc,
                                  std::vector<CResource*>&resc,
                                  const char *pszExternalDb,
                                  const char *pszExcludeAgency,
                                  const char *pszOmitResourceDb,
                                  const char *pszRescOmit,
                                  const char * const *papszOptions ) = 0;
    virtual IRS_Err AssociateHelitack( std::multimap<std::string, CResource*>&resc_map ) = 0;
    virtual IRS_Err LoadAlternateHelibases( std::map<std::string, OmffrNode<CDispatchBase*>*>&DLDispMap,
                                            OmffrTree<CDispatchBase*>&DispTree ) = 0;
    virtual IRS_Err CreateLargeAirtankers( std::vector<CRescType>&types,
                                           std::vector<CDispLoc>&loc,
                                           std::vector<CResource*>&resc,
                                           int nATTCount,
                                           const char * const *papszOptions ) = 0;

    virtual IRS_Err LoadScenario( const std::vector<CFWA>&fwas,
                                  std::vector<CFire>&fires,
                                  int year,
                                  double dfApplyTreatment,
                                  const char * const *papszOptions ) = 0;
    virtual IRS_Err LoadSpatial( std::vector<CFWA>&fwa,
                                 std::vector<CDispLoc>&disp_loc,
                                 std::vector<CDispLogic>&logic,
                                 std::vector<CResource*>&resc,
                                 const char * const *papszOptions ) = 0;
    virtual IRS_Err SetSpatialFilter( const char *pszWktGeometry ) = 0;
    virtual const char * GetSpatialFilter() = 0;
    virtual IRS_Err AddFpuFilter( const char *pszFpuName ) = 0;
    virtual const char * GetFpuFilter() = 0;
    virtual std::vector<int>GetYearIndexes() = 0;
    virtual std::vector<int> GetAvgNumFires( const char *pszFpuName ) = 0;
    IRS_Err CleanUpResources( std::vector<CResource*>&resc );
    virtual std::string GetFpuName( double x, double y ) = 0;
    virtual std::map<std::string, double>GetFwaAssocDistance( const char *pszFwaName ) = 0;
    virtual std::string GetDispLocFPU( const char *pszDispName ) = 0;
    virtual std::string PointQuery( const char *pszTableName,
                                    const char *pszAttrName, double dfX,
                                    double dfY ) = 0;
    static std::vector<int> SmoothNumFires( const std::vector <int>daily_fires,
                                            int nDays );
    virtual std::pair<double, double> GetTankerPercent( const char *pszTanker,
                                                        const char *pszFpuName ) = 0;
    virtual std::vector<int> GetNumFires( const char *pszFpuName, int nYear ) = 0;
    virtual IRS_Err LoadGeneralResources( sqlite3 *rsc_db, 
                                          const char * const *papszOptions ) = 0;
    virtual IRS_Err CreateGeneralResourceDb( const char *pszFilename, 
                                             sqlite3 **newdb ) = 0;
    virtual IRS_Err PopulateGeneralResources( sqlite3 *res_db, std::vector<CDispLoc>&oLoc,
                                              std::vector<CResource*>&oResc, 
                                              std::vector<CRescType>&oRescType,
                                              int (*pfnProgress)(double, const char*, void*) ) = 0;
    virtual IRS_Err ResourceDist( const char *pszGacc, const char *pszType,
                                  const char *pszDispLoc, const char *pszAgency,
                                  double *pdfDist ) = 0;
    virtual CResource * CreateGenericResource( const char *pszType, CRescType &oType,
                                               CDispLoc &oLoc ) = 0;
    virtual sqlite3 * ExportResults( const char* pszOutputFile, int nYear,
                                     std::vector<CResults>oResults,
                                     double dfPercentKeep ) = 0;
    virtual std::map< std::string, std::vector< int > >
    SummarizeFwa( std::vector<CResults>oResults,
                  const char *pszOutputFile, double dfKeep,
                  int (*pfnProgress)(double, const char*, void*)) = 0;
    virtual std::vector<std::string> GetFwasFromRegion( const char* pszRegion ) = 0;
    virtual std::vector<std::string> GetFwasFromFpu( const char* pszFpu ) = 0;
    virtual std::vector< std::map<std::string, std::string> >
    GetAttributes( const char *pszTable, const char *pszQueryField,
                   const char *pszQueryValue ) = 0;
    virtual std::vector<int>GetFireCount(int nYear, const char *pszGeometry) = 0;
    virtual std::vector<int>GetFireStatsMin( const char *pszGeometry,
                                             int (*pfnProgress)(double, const char*, void*)) = 0;
    virtual std::vector<int>GetFireStatsMax( const char *pszGeometry, 
                                             int (*pfnProgress)(double, const char*, void*)) = 0;
    virtual IRS_Err SetTimes() = 0;
    int GetIndex( double *probs, double find, int size );
    virtual std::vector<double> GetFireStatsMean( const char *pszGeometry, int (*pfnProgress)(double, const char*, void*) ) = 0;
    virtual sqlite3* ExportFullResults( const char* pszOutputFile, int nYear,
                                        std::vector<CResults>oResults,
                                        std::vector<int>oRescUsage ) = 0;
    virtual IRS_Err WriteCartFile( const char *pszResultDb,
                                   const char *pszOutputCsv ) = 0;

    IRS_Err CopyResources( const std::vector<CResource*> aoResourcesIn,
                           std::vector<CResource*> &aoResourcesCopy );
    virtual IRS_Err GetLargeFireValues( double dfX, double dfY,
                                        double dfRadius, int nMethod,
                                        IRSLargeFire *psLargeFire ) = 0;

    virtual char ** GetRescRemovalSet( int nAgencies, int nRegions, int nTypes,
                                       int nHowMany, int *nCount,
                                       const char *pszIgnoreIds ) = 0;

    virtual int GetRescCount( int nAgencies, int nRegions, int nTypes,
                              const char *pszIgnoreIds ) = 0;

    virtual void DestroyStringList( char ** );
protected:
    char * LaunderFwa( const char *pszIn );
    const char * LaunderType( const char *pszIn );
    const char *pszSource;
#ifdef OMFFR_GUI
signals:
    void SendProgress(double, const char *pszMessage, void *hUnused);
#endif

};


#ifdef IRS_HAVE_SPATIALITE
#include <sqlite3.h>
class SpatialiteDataAccess : public IRSDataAccess
{
public:
    SpatialiteDataAccess();
    SpatialiteDataAccess( const char *pszSource );
    ~SpatialiteDataAccess();
    virtual IRS_Err LoadAllData( CDataMaster *poScenario,
                                 const char *pszExternalDb );
    virtual IRS_Err LoadDispatchLogic( std::vector<CDispLogic>&logic, 
                                       IRSProgress pfnProgress,
                                       const char * const *papszOptions );
    virtual IRS_Err LoadDispatchLocation( std::vector<CFWA>&fwas,
                                          std::vector<CDispLoc>&loc,
                                          const char * const *papszOptions );
    virtual IRS_Err LoadTankerBases( CDataMaster *poScenario,
                                     std::vector<CFWA>&fwas,
                                     std::vector<CDispLoc>&loc, 
                                     double dfDist,
                                     const char * const *papszOptions );
    virtual IRS_Err LoadFwa( std::vector<CDispLogic>&logic,
                             std::vector<CFWA>&fwas,
                             IRSProgress pfnProgress,
                             const char * const *papszOptions );
    virtual IRS_Err LoadRescType( std::vector<CRescType>&types,
                                  const char * const *papszOptions );
    virtual IRS_Err LoadProdRates( std::vector<CProdRates>&rates,
                                   const char * const *papszOptions );
    virtual IRS_Err AssociateHelitack( std::multimap<std::string, CResource*>&resc_map );
    virtual IRS_Err LoadAlternateHelibases( std::map<std::string, OmffrNode<CDispatchBase*>*>&DLDispMap,
                                            OmffrTree<CDispatchBase*>&DispTree );
    virtual IRS_Err CreateLargeAirtankers( std::vector<CRescType>&types,
                                           std::vector<CDispLoc>&loc,
                                           std::vector<CResource*>&resc,
                                           int nATTCount,
                                           const char * const *papszOptions );
    virtual IRS_Err LoadResource( CDataMaster*,
                                  std::vector<CRescType>&types,
                                  std::vector<CDispLoc>&loc,
                                  std::vector<CResource*>&resc,
                                  const char *pszExternalDb,
                                  const char *pszExcludeAgency,
                                  const char *pszOmitResourceDb,
                                  const char *pszRescOmit,
                                  const char * const *papszOptions );
    virtual IRS_Err LoadScenario( const std::vector<CFWA>&fwas,
                                  std::vector<CFire>&fires,
                                  int year,
                                  double dfApplyTreatment,
                                  const char * const *papszOptions );
    virtual IRS_Err LoadSpatial( std::vector<CFWA>&fwa,
                                 std::vector<CDispLoc>&disp_loc,
                                 std::vector<CDispLogic>&logic,
                                 std::vector<CResource*>&resc,
                                 const char * const *papszOptions );
    virtual std::vector<int>GetYearIndexes();
    CFWA* LoadFwaByName( const char *pszName );
    IRS_Err SetSpatialFilter( const char *pszWktGeometry );
    const char * GetSpatialFilter();
    IRS_Err AddFpuFilter( const char *pszFpuName );
    virtual const char * GetFpuFilter();
    std::string GetFpuName( double x, double y );
    int GetFwaId( const char *pszFwaName );
    std::map<std::string, double>GetFwaAssocDistance( const char *pszFwaName );
    std::string GetDispLocFPU( const char *pszDispName );
    std::string PointQuery( const char *pszTableName, const char *pszAttrName,
                            double dfX, double dfY );
    std::vector<int> GetAvgNumFires( const char *pszFpuName );
    virtual std::vector<int> GetNumFires( const char *pszFpuName, int nYear );
    virtual std::pair<double, double> GetTankerPercent( const char *pszTanker,
                                                        const char *pszFpuName );
    virtual IRS_Err LoadGeneralResources( sqlite3 *rsc_db, 
                                          const char * const *papszOptions );
    virtual IRS_Err CreateGeneralResourceDb( const char *pszFilename,
                                             sqlite3 **new_db );
    virtual IRS_Err PopulateGeneralResources( sqlite3 *res_db, std::vector<CDispLoc>&oLoc,
                                              std::vector<CResource*>&oResc,
                                              std::vector<CRescType>&oRescType,
                                              int (*pfnProgress)(double, const char*, void*));
    virtual IRS_Err ResourceDist( const char *pszGacc, const char *pszType,
                                  const char *pszDispLoc, const char *pszAgency,
                                  double *pdfDist );
    virtual CResource * CreateGenericResource( const char *pszType, CRescType &oType,
                                               CDispLoc &oLoc );
    virtual sqlite3 * ExportResults( const char* pszOutputFile, int nYear,
                                     std::vector<CResults>oResults,
                                     double dfPercentKeep );
    virtual std::map< std::string, std::vector< int > >
    SummarizeFwa( std::vector<CResults>oResults,
                  const char *pszOutputFile, double dfKeep,
                  int (*pfnProgress)(double, const char*, void*));
    virtual std::vector<std::string> GetFwasFromRegion( const char* pszRegion );
    virtual std::vector<std::string> GetFwasFromFpu( const char* pszFpu );
    virtual std::vector< std::map<std::string, std::string> >
    GetAttributes( const char *pszTable, const char *pszQueryField,
                   const char *pszQueryValue );
    virtual std::vector<int>GetFireCount( int nYear, const char *pszGeometry );
    virtual std::vector<int>GetFireStatsMin( const char *pszGeometry,
                                             int (*pfnProgress)(double, const char*, void*));
    virtual std::vector<int>GetFireStatsMax( const char *pszGeometry, 
                                             int (*pfnProgress)(double, const char*, void*));
    virtual IRS_Err SetTimes();
    virtual std::vector<double> GetFireStatsMean( const char *pszGeometry, int (*pfnProgress)(double, const char*, void*) );
    virtual sqlite3* ExportFullResults( const char* pszOutputFile, int nYear,
                                        std::vector<CResults>oResults,
                                        std::vector<int>oRescUsage );
    virtual IRS_Err WriteCartFile( const char *pszResultDb,
                                   const char *pszOutputCsv );
    virtual IRS_Err GetLargeFireValues( double dfX, double dfY,
                                        double dfRadius, int nMethod,
                                        IRSLargeFire *psLargeFire );
    virtual char ** GetRescRemovalSet( int nAgencies, int nRegions, int nTypes,
                                       int nHowMany, int *nCount,
                                       const char *pszIgnoreIds );
    virtual int GetRescCount( int nAgencies, int nRegions, int nTypes,
                              const char *pszIgnoreIds );


private:
    int GetUnionedGeometry( const char *, char **, int * );
    sqlite3 *db;
    const char *pszFilter;
    const char *pszFpuFilter;
};
#endif /* IRS_HAVE_SPATIALITE */

#endif /* DATA_LOAD_H_ */
