/******************************************************************************
 *
 * $Id: RunScenario.h 586 2014-05-10 22:29:23Z kyle.shannon $
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Run a suite of years through IRS
 * Author:   Diane Tretheway <dtretheway@fs.fed.us>
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
/**
 * \file RunScenario.h
 * \brief Definition file for class that runs a fire scenario
 * \author Diane Tretheway <dtretheway@fs.fed.us>
 * \date 2012-09-04
 * \since 1.0
 */

// RunScenario.h
// for IRS 11/11

#ifndef RUNSCENARIO_H
#define RUNSCENARIO_H

#include <iostream>                                                                                     //contains functions for inputting and outputting
#include <fstream>                                                                                      //contains functions for inputting and outputting to a file
using std::ifstream;                                                                            //for inputting from a file
#include <cctype>                                                                                       //contains functions for characters, strings, and structures

using namespace std;

#include <cstring>                                                                                      //contains string functions
#include <cstdlib>                                                                                      //contains function prototype for rand
#include <ctime>                                                                                        //contains functions for manipulating time and date
#include <vector>
#include <list>
//#include <unordered_map>

// Include definition for class CRescType from CRescType.h
#include "CRescType.h"
// Include definition for class CDispLoc from DispLoc.h
#include "DispLoc.h"
// Include definition for class CResource from CRescbase.h
#include "Rescbase.h"
// Include definiton for class CGround from ground.h
#include "Ground.h"
// Include definition for class CFWA in FWA.h
#include "FWA.h"
// Include definition for class CFire in Fire.h
#include "Fire.h"
// Include definition for class CDispLogic in DispLogic.h
#include "DispLogic.h"
// Include definition for class CProdRates in ProdRates.h
#include "ProdRates.h"
// Include definition for class CConstProd in ConstProd.h
#include "ConstProd.h"
// Include definition for class CEngine in Engine.h
#include "Engine.h"
// Include definition for class CCrew in Crew.h
#include "Crew.h"
// Include definition for class CWaterTender in WaterTender.h
#include "WaterTender.h"
// Include definition for class CResults in Results.h
#include "Results.h"
// Include tree definitions from omffr_tree.h
#include "omffr_tree.h"
// Include tree definitions for the CDispatcher class DispatchBase.h, DLDispatcher.h
#include "DispatchBase.h"
#include "NatDispatcher.h"
#include "GACCDispatcher.h"
#include "LocalDispatcher.h"
#include "DLDispatcher.h"
#include "Escape.h"

#include "FPU.h"

//#include "cpl_progress.h"

class PrepositionStruct
{
public:
    PrepositionStruct();
    PrepositionStruct( std::string, double, bool );
    PrepositionStruct( const PrepositionStruct &rhs );
    PrepositionStruct & operator = ( const PrepositionStruct &rhs );

    std::string rescType;
    double level;
    bool outOfSeason;
};
typedef PrepositionStruct DrawdownStruct;

class CRunScenario      {
public:
		CRunScenario();     
	    ~CRunScenario();                                                                                //destructor for CRunScenario
        CRunScenario(const CRunScenario &rhs);

        int GetRand();                                                                                  //get a random integer between 0 and 100
        void ReadDispLogicFile( std::string oFilename );                                                //read in dispatch logic from file
        void ReadFWAFile( std::string oFilename );                                                      //read in FWA from file
        void ReadRescTypeFile( std::string oFilename );                                                 //read in resource type information from file
        void ReadDispLocFile( const int numFWAs, std::string oFilename );                               //read in dispatch location information from file
        void ReadAssociationsFile( std::string OFilename );								//read in the FWA / Dispatch Location Associations
		void ReadProdRatesFile( std::string oFilename );                                                //read in production rates information from file
        void ReadResourceFile( std::string oFilename );                                                 //read in resource information from file
        void ReadFireFile( const char *Scenario );                                                      //read in fire information from file
		void ReadFireSeason();																			////read the fire season start and end date from the file for internal nodes
		
		void SetPreposition( PrepositionStruct &ATTpp, PrepositionStruct &RCRWpp, PrepositionStruct &RHelpp, PrepositionStruct &FSCRWpp,
		PrepositionStruct &DOICRWpp, PrepositionStruct &FSENGpp, PrepositionStruct &DOIENGpp, PrepositionStruct &FSCRWdd, 
		PrepositionStruct &DOICRWdd, PrepositionStruct &FSDZRdd, PrepositionStruct &DOIDZRdd, PrepositionStruct &FSENGdd,
		PrepositionStruct &DOIENGdd, PrepositionStruct &FSSEATdd, PrepositionStruct &DOISEATdd, PrepositionStruct &FSHELIpp,
		PrepositionStruct &DOIHELIpp );
		
		// Functions added for dispatcher class and tree
		//bool AssignHelitack( std::unordered_multimap< string, CResource* > ResourceMap );				// Assign helitack to a helicopter
		bool AssignHelitack( std::multimap< string, CResource* > ResourceMap );
		bool AltHelicDLs( std::map<std::string, OmffrNode<CDispatchBase*>*> DLDispMap );				// Add alternate dispatch locations for helicopters to deploy helitack to DLDispatcher list
		bool CreateDispTree();																			// Create a tree from the vectors in this class
		bool ResourcesToDispatchers();																	// Create the resource maps for the dispatch location dispatchers
		bool FWAsFindClosestAirtankerBases();															// Find the closest airtanker base for each FWA, store as FWA member
		std::list< CResource* > DeployFromRescVector( int f, int Debugging );							// Deploy resource using the resource vectors ( original method )
		std::list< string > GetOrderedDispLocs( CFWA FWA );												// Make a list of the names Dispatch Locations associated with the fire's FWA and order them by distance from the fire
		std::list< string > GetOrderedDispLocsbyLevel( CFWA FWA, int Level );							// Make a list of the names Dispatch Locations associated with the fire's FWA for a given level of the tree ordered by the dispatch location's distance from the fire -- Don't really need
		void RemoveDeployedResc( std::list< CResource* > &LDeployedResources, std::list< CResource* > Remove );	// Remove resources from the deployed resources list that are in the Remove list
		bool UpdateDispatcherLevels( CFire fire, int ResultsIndex, bool DispLogicFilled );				// Following the Containment effort on a fire update the levels for the dispatchers
		void OpenLevelsFile();																			// Open the dispatcher levels file and set the header
		vector< OmffrNode< CDispatchBase* >* >GetInternalNodes( OmffrNode< CDispatchBase* >* Node );		// Get the internal nodes from Node down on the dispatcher tree
		vector< OmffrNode< CDispatchBase* >* >OrderNodesRootLast( vector< OmffrNode< CDispatchBase* >* > Nodex );	// Order the nodes in a vector so the ones furthest from the root are first
		void DetermineExpectedLevel();																	// ** Not Used **Determine the Expected Fire Level for each dispatcher in the tree
		void AirtankerBasesExpectedLevels();															// ** Not Used **Determine the Expected fire Level for the airtanker bases
		vector< OmffrNode< CDispatchBase* >* >GetAssociatedDispLocs( CFWA FWA);							// Get a vector of the Dispatch Locations associated with an FWA
		vector< OmffrNode< CDispatchBase* >* >GetAssociatedDispLocsWithResc( CFWA FWA );				// Get a vector of the Dispatch Locations associated with an FWA that have resources at them
		vector< CFWA* >GetAssociatedFWAs( CDLDispatcher* DLDispatcher );								// Get a vector of pointers to FWAs associated with a Dispatch Location Dispatcher
		int TypeIICrewDrawDown( int fire, int CDDEscapes, int CDDDuration, int CDDLimit );				// Apply Type II IA Crew Draw Down
		void SaveTypeTwoIARescWorkYear();
		
		void PrepositionAllResc( PrepositionStruct ATTpp, PrepositionStruct RCRWpp, PrepositionStruct RHelpp, 
				PrepositionStruct FSCRWpp, PrepositionStruct DOICRWpp, PrepositionStruct FSENGpp, PrepositionStruct DOIENGpp,
				PrepositionStruct FSHELIpp, PrepositionStruct DOIHELIpp, int Julian );				// Preposition resources according to GUI
		void ReturnResources( int Julian );								// Return the resource that were borrowed
		void SaveMovementVectors();										// Save the movement vector for resources
		int PrepositionAirtankersIA( double MoveLevel, int Julian );		// preposition airtankers
		int PrepositionResourceIA( string RescType, double MoveLevel, bool OutSeason, int Julian );		// Preposition large airtankers at the beginning of each day
		void DLDispatcherExpectedWeights();								// Create a file that contains the FPU weights for the DLDispatchers to be used for calculating the IA Expected Levels
		void CreateRescTypeVectors();									// Create vectors with the ids for the dispatch locations that house the types
		void LoadExpectedLevels( int Scenario, int BaseForcast );		// Load the expected levels into the nodes for all the years
		void  DailyDrawDown( list< int > VOrderedEscapeFires );		// Daily Draw Down for escape fires
		list< int > OrderEscapeFires();								// Order the escape fires for each internal node of the tree
		void AddEscapeFire( int EscapeDate, int EscapeTime, CFire &Fire );	// Add an escape fire to the list
		bool OrderLocalDispatchLocations( int Julian, list< CDLDispatcher* > &OrderedCRW, list< CDLDispatcher* > &OrderedDZR, 
			list< CDLDispatcher* > &OrderedENG, list< CDLDispatcher* > &OrderedHel, list< CDLDispatcher* > &OrderedHELI, list< CDLDispatcher* > OrderedSEAT, 
			PrepositionStruct FSCRWdd, PrepositionStruct DOICRWdd, PrepositionStruct FSDZRdd, PrepositionStruct DOIDZRdd, 
			PrepositionStruct FSENGdd, PrepositionStruct DOIENGdd, PrepositionStruct FSSEATdd, PrepositionStruct DOISEATdd );				// Fill the ordered lists of dispatch locations by agency and resource type		
		void AddRescTypeDLs( list< CDLDispatcher* > &OrderedDLs, vector< string > &DLIDVector, bool OutSeason, double Movability, int Julian, int DispTypeIdx ); 
		
		bool SimulateContain( int firstArrival, double size, CFire &fire );

		// Functions to calculate the expected values for airtanker bases
		//void ReadFPUs();
		//void CalcFPUExpectLevel();
		//void CalcATExpectLevel();
		//void PrintFPUInfo();

        void ReadFiles( std::string oPath="" );                                                         //read in all the data files
        bool DeployResources( int Debugging, int f, int scenario );                                     //determine which resources to deploy to the fire and calculate their start time, production rate, duration vectors
        int FirstArrivalTime( std::list< int > deployed );                                              //determine the time when the first resource arrives
        string RunContain( int Debugging, int f, std::list< CContainValues > LContainValues, double size, int firstarrival, bool DispLogicFilled, bool InSeason );           //run the contain program
        bool RunScenario( int Debugging, int Scenario, int (*pfnProgress)(double dfProgress, const char *pszMessage, void *pArg ) = NULL );

        void PrintFireInfo( int f );                                                                    //print informtaion about a fire and the resources to a file
        void PrintDeployInfo( std::list< int > ATT, std::list< int > CREW, std::list< int > DZR, std::list< int > ENG, 
        std::list< int > FBDZ, std::list< int > FRBT, std::list< int > Helicopter, std::list< int > HELI, std::list< int > SCPSEAT, 
        std::list< int > SJAC, std::list< int > SMJR, std::list< int > TP, std::list< int > WT, std::list< int > LDeployedResources, 
        int WTArrival);                                                                                                 //print the deploy information about the resources to a file 
        void PrintContainInput( CFire fire, std::list< CContainValues > LContainValues, int firstarrivaltime ); // Print the resource values that are input into the contain algorithm
        void PrintContainInfo( int f, double FireCost, double FireLine, double FirePerimeter, double FireSize, double FireSweep,
        double FireTime, double ResourcesUsed, string Status );                 //print the fire's contain information
        void PrintRescWorkYear( std::list< CResource* > DeployedResc, CFire fire, int FireEndTime );           //print the information in the resource work year file for the deployed resources
        //void PrintRescWorkYear();																		// Print all the resource work year information for a list of resources
		void Reset();                                                                                  // Output the desired information for the run
		void Output();                                                                                  // Output the desired information for the run
		void LastScenarioDay();																			// Process the previous level information for the last day of the scenario

        std::list<int> FindCrewWorking( CFire fire, std::list< int > &Carrier, std::list< int > &Crew );        // Order the crews in the crew list taking into account the carriers
        int FindBestCarrier( CFire fire, int numcrews, int numresc, int numresca, std::list< int > &Carrier, std::list< int > &Crew_working );
        std::list<int> DeployCrewAndCarrier( CFire fire, int carrier, int numcrews, int numrescc, std::list< int > &Crew_working, std::list< int > &LDeployedResources );       // Deploy the crews and their carriers
        int CrewAndCarrier( CFire fire, std::list< int > &Carrier, std::list< int > &Crew, int numrescC, int numresca, std::list< int > &LDeployedResources, std::list< int > &CarriersUsed );

        int SetCarrierIndex( int crewindex, int carrierindex );                 // set the carrier index for crews that are delivered to the fire by aircraft
        int GetCarrierIndex( int crewindex );                                                   // get the carrier index for crews that are delivered to the fire by aircraft
        int SetNextLoadArrival( int carrierindex, int nextloadarrival );        // set the next load arrival time for a carrier
        int GetNextLoadArrival(int carrierindex );                                              // get the next load arrival time for a carrier
        bool IsInList( std::list< int > Thelist, int item );                    // is the integer in the list?

		bool UpdateDLDispatcherDispatchCount(string dispLocId, int Julian);		// Increase by one the deployed resources count for the day for the dispatch location

        inline std::vector<CResource*>& GetResourceRef() {return m_VResource;}
        inline std::vector<CDispLoc>& GetDispLocRef() {return m_VDispLoc;}
        inline std::vector<CRescType>& GetRescTypeRef() {return m_VRescType;}

		void SaveDispLogicFile( );                                                //save dispatch logic from file
        void SaveFWAFile( );                                                      //save FWA from file
        void SaveDispLocFile( );												  //save dispatch location information from file
        void SaveResourceFile( );                                                 //save resource information from file
        void SaveFireFile( int Scenario );                                        //save fire information from file
		void SaveFiles();														  //save all the files used in this run

		void RunContainAlgorithmFromFile( string Filename );					  // run the contain algorithm with values read from a file
		


        std::vector<int> GetResourceUsage();
        std::map<std::string, double> GetSingleResourceUsage();


//private:
        std::vector< CDispLogic > m_VDispLogic;
        std::vector< CFWA > m_VFWA;
        std::vector< CRescType > m_VRescType;
        std::vector< CDispLoc > m_VDispLoc;
        std::vector< CProdRates > m_VProdRates;
        std::vector< CResource * > m_VResource;
        std::vector< CFire > m_VFire;
        std::vector< CResults > m_VResults;
		//std::vector< CFPU > m_VFPU;

		OmffrTree< CDispatchBase* > m_DispTree;
		std::map<std::string, OmffrNode<CDispatchBase*>*> m_DispMap;
		std::map<std::string, OmffrNode<CDispatchBase*>*> m_InMap;
		std::map<std::string, int > m_FWAMap;
		
        int m_NumDispLogic;
        int m_NumFWA;
        int m_NumRescType;
        int m_NumDispLoc;
        int m_NumProdRates;
        int m_NumResource;
        int m_NumFire;
        int m_NumResults;

		std::map<std::string, OmffrNode<CDispatchBase*>*> BuildDispatchIndex( OmffrTree<CDispatchBase*> *oTree );
		std::map<std::string, OmffrNode< CDispatchBase* >* > BuildInternalIndex( OmffrTree< CDispatchBase* > *oTree );

		std::vector< string > m_RegionalCrewDLs;
		//std::vector< string > m_TypeIHelicopterDLs;
		//std::vector< string > m_TypeIIHelicopterDLs;
		//std::vector< string > m_TypeIIIHelicopterDLs;
		std::vector< string > m_RegionalHelicopterDLs;
		std::vector< string > m_SmokejumperDLs;
		std::vector< string > m_AirtankerDLs;
		std::vector< string > m_DOICRWDLs;
		std::vector< string > m_DOIDZRDLs;
		std::vector< string > m_DOIENGDLs;
		std::vector< string > m_DOIHelDLs;
		std::vector< string > m_DOIHELIDLs;
		std::vector< string > m_DOISEATDLs;
		std::vector< string > m_FSCRWDLs;
		std::vector< string > m_FSDZRDLs;
		std::vector< string > m_FSENGDLs;
		std::vector< string > m_FSHelDLs;
		std::vector< string > m_FSHELIDLs;
		std::vector< string > m_FSSEATDLs;

		std::vector< CResource* > m_BorrowedResources;				// pointers for the resources that have been borrowed
		vector< CEscape > m_VEscapes;								// vector with the escape fires
		int m_EscapeIdx;											// index for the last escape fire that was dealt with

		PrepositionStruct ATTpp;
		PrepositionStruct RCRWpp;
		PrepositionStruct RHelpp;
		PrepositionStruct FSCRWpp;
		PrepositionStruct DOICRWpp;
		PrepositionStruct FSENGpp;
		PrepositionStruct DOIENGpp;
		PrepositionStruct FSHELIpp;
		PrepositionStruct DOIHELIpp;

		PrepositionStruct FSCRWdd;
		PrepositionStruct DOICRWdd;
		PrepositionStruct FSDZRdd;
		PrepositionStruct DOIDZRdd;
		PrepositionStruct FSENGdd;
		PrepositionStruct DOIENGdd;
		PrepositionStruct FSSEATdd;
		PrepositionStruct DOISEATdd;

	};              // End class CRunScenario

#endif /* RUNSCENARIO_H */
