// DataMaster.h
// for WFIPs 2/15

#ifndef DATAMASTER_H
#define DATAMASTER_H

#include <cstring>                                                                                      //contains string functions
#include <cstdlib>                                                                                      //contains function prototype for rand
#include <vector>
#include <list>
#include <map>

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
#include "Airtanker.h"
// Include definition for class CAirtanker in Airtanker.h
#include "SmallAT.h"
// Include definition for class CSmallAT in SmallAT.h
#include "Smokejumper.h"
// Include definition for class CSmokejumper in Smokejumper.h
#include "SMJAircraft.h"
// Include definition for class CSMJAircraft in SMJAircraft.h
#include "Helitack.h"
// Include definition for class CHelitack in Helitack.h
#include "Helicopter.h"
// Include definition for class CHelicopter in Helicopter.h
#include "omffr_tree.h"
// Include tree definitions for the CDispatcher class DispatchBase.h, DLDispatcher.h
#include "DispatchBase.h"
#include "NatDispatcher.h"
#include "GACCDispatcher.h"
#include "LocalDispatcher.h"
#include "DLDispatcher.h"
#include "FPU.h"
#include "RunScenario.h"


/*class PrepositionStruct
{
public:
    PrepositionStruct();
    PrepositionStruct( const PrepositionStruct &rhs );
    PrepositionStruct & operator = ( const PrepositionStruct &rhs );

    std::string rescType;
    double level;
    bool outOfSeason;
};
typedef PrepositionStruct DrawdownStruct;*/

class CDataMaster      {
public:
		CDataMaster();
		CDataMaster(const CDataMaster &rhs);
	    ~CDataMaster();                                                                                 //destructor for CRunScenario
        
		CDataMaster& operator=(const CDataMaster &rhs);													//assignment operator
		
        //int GetRand();                                                                                  //get a random integer between 0 and 100
        //void ReadDispLogicFile( std::string oFilename );                                                //read in dispatch logic from file
        //void ReadFWAFile( std::string oFilename );                                                      //read in FWA from file
        //void ReadRescTypeFile( std::string oFilename );                                                 //read in resource type information from file
        //void ReadDispLocFile( const int numFWAs, std::string oFilename );                               //read in dispatch location information from file
        //void ReadAssociationsFile( std::string OFilename );												//read in the FWA / Dispatch Location Associations
		//void ReadProdRatesFile( std::string oFilename );                                                //read in production rates information from file
        //void ReadResourceFile( std::string oFilename );                                                 //read in resource information from file
        //void ReadFireFile( const char *Scenario );                                                      //read in fire information from file
		//void ReadFireSeason();																			////read the fire season start and end date from the file for internal nodes
		
		/*void SetPreposition( PrepositionStruct &ATTpp, PrepositionStruct &RCRWpp, PrepositionStruct &RHelpp, PrepositionStruct &FSCRWpp,
		PrepositionStruct &DOICRWpp, PrepositionStruct &FSENGpp, PrepositionStruct &DOIENGpp, PrepositionStruct &FSCRWdd, 
		PrepositionStruct &DOICRWdd, PrepositionStruct &FSDZRdd, PrepositionStruct &DOIDZRdd, PrepositionStruct &FSENGdd,
		PrepositionStruct &DOIENGdd, PrepositionStruct &FSSEATdd, PrepositionStruct &DOISEATdd, PrepositionStruct &FSHELIpp,
		PrepositionStruct &DOIHELIpp );*/
		
		// Functions added for dispatcher class and tree
		
		bool AltHelicDLs(std::map<std::string, OmffrNode<CDispatchBase*>*>DLDispMap );					// Add alternate dispatch locations for helicopters to deploy helitack to DLDispatcher list
		bool CreateDispTree();																			// Create a tree from the vectors in this class
		bool ResourcesToDispatchers();																	// Create the resource maps for the dispatch location dispatchers
		bool FWAsFindClosestAirtankerBases();															// Find the closest airtanker base for each FWA, store as FWA member
		//***data_load***void CreateRescTypeVectors();																	// Fill the vectors defining the dispatch location where each type of resource can be prepositioned
		bool CreateFWAMap();																			// Create a map with the FWA id and it's index in the m_VFWA array

		//void OpenLevelsFile();																			// Open the dispatcher levels file and set the header
		vector< OmffrNode<CDispatchBase*>*>GetInternalNodes(OmffrNode<CDispatchBase*>* Node );			// Get the internal nodes from Node down on the dispatcher tree
		//vector< OmffrNode< CDispatchBase*>*>OrderNodesRootLast(vector<OmffrNode< CDispatchBase*>*> Nodex );	// Order the nodes in a vector so the ones furthest from the root are first
		//void DetermineExpectedLevel();																	// ** Not Used **Determine the Expected Fire Level for each dispatcher in the tree
		//void AirtankerBasesExpectedLevels();															// ** Not Used **Determine the Expected fire Level for the airtanker bases
		//vector<OmffrNode<CDispatchBase*>*>GetAssociatedDispLocs(CFWA FWA);								// Get a vector of the Dispatch Locations associated with an FWA
		//vector<OmffrNode<CDispatchBase*>*>GetAssociatedDispLocsWithResc(CFWA FWA);						// Get a vector of the Dispatch Locations associated with an FWA that have resources at them
		//vector<CFWA*>GetAssociatedFWAs(CDLDispatcher* DLDispatcher);									// Get a vector of pointers to FWAs associated with a Dispatch Location Dispatcher
		//int TypeIICrewDrawDown(int fire, int CDDEscapes, int CDDDuration, int CDDLimit);				// Apply Type II IA Crew Draw Down
		//void SaveTypeTwoIARescWorkYear();
		
		//void LoadExpectedLevels(int Scenario, int BaseForcast);											// Load the expected levels into the nodes for all the years
		//bool OrderLocalDispatchLocations( int Julian, list< CDLDispatcher* > &OrderedCRW, list< CDLDispatcher* > &OrderedDZR, 
			//list< CDLDispatcher* > &OrderedENG, list< CDLDispatcher* > &OrderedHel, list< CDLDispatcher* > &OrderedHELI, list< CDLDispatcher* > OrderedSEAT, 
			//PrepositionStruct FSCRWdd, PrepositionStruct DOICRWdd, PrepositionStruct FSDZRdd, PrepositionStruct DOIDZRdd, 
			//PrepositionStruct FSENGdd, PrepositionStruct DOIENGdd, PrepositionStruct FSSEATdd, PrepositionStruct DOISEATdd );				// Fill the ordered lists of dispatch locations by agency and resource type		
		//void AddRescTypeDLs( list< CDLDispatcher* > &OrderedDLs, vector< string > &DLIDVector, bool OutSeason, double Movability, int Julian, int DispTypeIdx ); 
		
        //void ReadFiles(std::string oPath="");															//read in all the data files
        
        //bool IsInList( std::list< int > Thelist, int item );                    // is the integer in the list?

		bool CopyDataSet(std::vector<CResource*> &resourceVector, OmffrTree<CDispatchBase*> &dispatchTree, std::map<string, OmffrNode<CDispatchBase*>*> &newDispMap, 
			std::map<string, OmffrNode<CDispatchBase*>*> &newInMap);	// Make a copy of the data for a new thread
		bool CopyResourceVector(std::vector<CResource*> &newResourceVector, std::map<CResource*, CResource*> &rescMap, vector<CResource*> &ATTPtrs);		// Make a copy of the resource vector
		bool CopyAttachedHelitackCrews(std::map<CResource*,CResource*> rescMap, std::vector<CResource*> helicopters);		// Copy the pointers for the attached helitack for each helicoptere
		bool CopyDispatchTree(OmffrTree<CDispatchBase*> &newdispatchTree,std::map<string, OmffrNode<CDispatchBase*>*> &newDispMap, 
			std::map<string, OmffrNode<CDispatchBase*>*> &newInMap, std::map<CResource*, CResource*> &rescMap, std::vector<CResource*> &ATTptrs);		// Make a copy of the dispatch tree
		string DetermineNodeType(OmffrNode<CDispatchBase*>* node);						// Determine the node type, National, GACC, Local, Dispatch Location
		bool AddTreeNode(OmffrTree<CDispatchBase*> &newTree, OmffrNode<CDispatchBase*>* oldNode, OmffrNode<CDispatchBase*>* parentNode,
			std::map<string, OmffrNode<CDispatchBase*>*> &newDispMap, std::map<string, OmffrNode<CDispatchBase*>*> &newInMap, 
			std::map<CResource*, CResource*> &rescMap, vector<CResource*> &ATTPtrs, std::vector<std::pair<CGACCDispatcher*, CResource*> > pairsGACCTypeTwoCrews);	// Add a node and the tree below it to the to a copy of the tree
	

		
		inline std::vector<CResource*>& GetResourceRef() {return m_VResource;}
        inline std::vector<CDispLoc>& GetDispLocRef() {return m_VDispLoc;}
        inline std::vector<CRescType>& GetRescTypeRef() {return m_VRescType;}

		//Just here for debugging
		void Reset(){};
		std::vector<int> GetResourceUsage(){};
		std::map<std::string, double> GetSingleResourceUsage(){};

	//private:
        std::vector<CDispLogic> m_VDispLogic;
        std::vector<CFWA> m_VFWA;
        std::vector<CRescType> m_VRescType;
        std::vector<CDispLoc> m_VDispLoc;
        std::vector<CProdRates> m_VProdRates;
        std::vector<CResource*> m_VResource;
        std::vector<CFire> m_VFire;
 		//std::vector<CFPU> m_VFPU;

		OmffrTree< CDispatchBase* > m_DispTree;
		std::map<std::string, OmffrNode<CDispatchBase*>*> m_DispMap;
		std::map<std::string, OmffrNode<CDispatchBase*>*> m_InMap;
		std::map<std::string, int> m_FWAMap;
		
        std::map<std::string, OmffrNode<CDispatchBase*>*> BuildDispatchIndex( OmffrTree<CDispatchBase*> *oTree );
		std::map<std::string, OmffrNode< CDispatchBase*>*> BuildInternalIndex( OmffrTree< CDispatchBase* > *oTree );

		std::vector<string> m_RegionalCrewDLs;
		std::vector<string> m_RegionalHelicopterDLs;
		std::vector<string> m_SmokejumperDLs;
		std::vector<string> m_AirtankerDLs;
		std::vector<string> m_DOICRWDLs;
		std::vector<string> m_DOIDZRDLs;
		std::vector<string> m_DOIENGDLs;
		std::vector<string> m_DOIHelDLs;
		std::vector<string> m_DOIHELIDLs;
		std::vector<string> m_DOISEATDLs;
		std::vector<string> m_FSCRWDLs;
		std::vector<string> m_FSDZRDLs;
		std::vector<string> m_FSENGDLs;
		std::vector<string> m_FSHelDLs;
		std::vector<string> m_FSHELIDLs;
		std::vector<string> m_FSSEATDLs;

		/*PrepositionStruct ATTpp;
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
		PrepositionStruct DOISEATdd;*/

		//just here for debugging
		std::vector< CResource* > m_BorrowedResources;				// pointers for the resources that have been borrowed
		vector< CEscape > m_VEscapes;								// vector with the escape fires
		int m_EscapeIdx;											// index for the last escape fire that was dealt with
		vector<CResults> m_VResults;
		int m_NumFWA;
		int m_NumRescType; 
		int m_NumDispLoc;
		int m_NumProdRates;
		int m_NumResource;
		int m_NumDispLogic;
		int m_NumFire;
		int m_NumResults;

};              // End class CDataMaster

#endif /* DATAMASTER_H */
