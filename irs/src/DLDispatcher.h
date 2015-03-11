// DLDispatcher.h
// Function definitions for class CDLDispatcher for dispatch location dispatchers derived from base class CDispatchBase
// for IRS 11/12

#ifndef DLDispatcher_H
#define DLDispatcher_H

#include <string>											//C++ standard string class
using std::string;
#include <list>
#include <vector>
#include <map>												//map class template definitions
#include <utility>											//for the pair

// include resource information from class CRescBase.h
#include "Rescbase.h"
// Include definition for class CDispatchBase from DispatchBase.h
#include "DispatchBase.h"
// Include definition for class CDispLoc from DispLoc.h
#include "DispLoc.h"

//#include "RunScenario.h"

// Include tree definitions from omffr_tree.h
#include "omffr_tree.h"

enum CalcDistance { TTP, LINEAR, HAVERSINE, STEW, INTERNET };

typedef std::multimap< string, CResource*, std::less< string > > RescMap;
typedef std::pair< OmffrNode< CDispatchBase* >, double > WeightPair;

class CDLDispatcher : public CDispatchBase {

public:
	CDLDispatcher( string dispatcherID, CDispLoc &DispLoc );
	CDLDispatcher( const CDLDispatcher &dldispatcher);		//copy constructor
	virtual ~CDLDispatcher();								//destructor

	virtual bool operator==( CDispatchBase &dispatchbase );	// equivalence operator
		
	// function definitions for set and get functions for class member variables
	
	string ConvertDispType( int DispTypeIdx );				//convert the resource's dispatch type index to a string plus Crew20
	int ConvertDispTypeIdx( string DispType);				//convert the resource map's dispatch type string to an index 0-13
	void SetRescMap( RescMap rescmap );						//set a resource map with another resource map
	RescMap GetRescMap();									//get a resource map
	std::list< CResource* > GetRescList();					//get a list of pointers to all the resources in the dispatch location's resource map
	void InsertInRescMap ( CResource* resource );			//insert a resource into the dispatch location's resource map
	void RemoveFromRescMap ( CResource* resource );			//remove a specific resource from the dispatch location's resource map
	std::list< CResource* >GetByDispType( string DispType );		//get a list of all the resources of a given dispatch type in the dispatch location's resource map
	CResource* GetRandofType( string DispType );			//get a random resource of dispatch type from dispatch location's resource map
	void SetDispLoc( CDispLoc &disploc );					//set the dispatch location
	CDispLoc &GetDispLoc();									//get the dispatch location
	void SetWeightPairs( vector< WeightPair > WeightPair );	//set the weight pairs with a vector of weight pairs
	vector< WeightPair > GetWeightPairs();					//get the vector of weight pairs
	void AddWeightPair( WeightPair weightpair );			//add a weight pair to the vector of weight pairs
	WeightPair GetWeightPairByIndex( int index );			//get a weight pair from the vector by index
	void SetAltHelicopterDLsList( list< CDLDispatcher* > AltHelicopterList );	//Set the alternate helicopter list with a list
	list< CDLDispatcher* > GetAltHelicopterDLsList();			// Get the list of alternate helicopter dispatch location pointers
	bool AddtoAltHelicopterDLsList( CDLDispatcher* DispLocPtr );	// Add a dispatch location pointer to the list of alternate helicopter dispatch locations
	bool RemovefromAltHelicopterDLSList( CDLDispatcher* DispLocPtr );	// Remove a dispatch location pointer from the list of alternate helicopter dispatch locations

	double CalculateDistance( CFire fire, CalcDistance CalcDistMethod );	// calculate the distance to a fire

	// access to static lists
	static list< CResource* > GetHelicopters();				// get the list of available and unattached helicotpers
	static list< CResource* > GetHelitack();				// get the list of available and unattached helitack
	static list< CResource* > GetSmokeJumperAircraft();		// get the list of available and unattached smokejumper aircraft
	static list< CResource* > GetSmokeJumpers();			// get the list of available and unattached smokejumpers
	static list< CResource* > GetGroundResources();			// get the list of ground resources not in the dispatch logic for use as substitutes
	static list< CResource* > GetAerialResources();			// get the list of aerial resources not in the dispatch logic for use as substitutes
	static list< CResource* > GetLateArrivals();			// get the list of resources in the dispatch logic that were not currently available
	static void EmptyHelicopters();							// empty helicopter list
	static void EmptyHelitack();							// empty helitack list
	static void EmptySmokeJumperAircraft();					// empty smokejumper aircraft list
	static void EmptySmokeJumpers();						// empty smokejumper list
	static void EmptyGroundResources();						// empty ground resources list
	static void EmptyAerialResources();						// empty aerial resources list
	static void EmptyLateArrivals();						// empty late arrivals list

	int NumRescIdx( int Index );							// get the number of resources, available and unavailable, that are at the dispatch location

	// functions to dispatch resources
	list< CResource* > DeployResources( vector< int > &RemainingDispLogic, CFire &Fire, list< CResource* > &DropHelicopters );	//add currently available resources to dispatch logic
	bool DeploySmokejumpers( vector< int > &RemainingDispLogic, CFire &Fire, list< CResource* > &LDeployedRescs, double AirMiles );	// Deploy smokejumpers
	bool DeployHelitack( vector< int > &RemainingDispLogic, CFire &Fire, list< CResource* > &LDeployedRescs, list< CResource* > &LResources, list< CResource* > &HelicopterList, double AirMiles );		// Deploy helitack and a helicopter
	bool DeployHelitackNew( vector< int > &RemainingDispLogic, CFire &Fire, list< CResource* > &LDeployedRescs, list< CResource* > &LResources, list< CResource* > &HelicopterList, list< CResource* > &HelitackList, double AirMiles );	// Deploy helitack and a helicopter New version 8/13
	bool HasRescInSeason( int Julian );						//Check if the dispatch location has resources in season on the julian date
	list< CResource* > OrderResources( CFire Fire, list< CResource* > LResources, double AirMiles );	// Order a list of CResources
	void RemoveSunset( list< CResource* > &Resources, CFire &Fire, int Sunset, double AirMiles );		// Remove resources from list that will not arrive before sunset

	virtual void ResetNewScenario( int scenario );			//iterate through the resources at the dispatch location and reset values for resources
	virtual void ResetNewDay( int Julian );					//reset the values for a new day - Daily Minimum resource level

	// functions to get determine the levels
	vector< int > DetermineCurRescLevel();					// Determines the number of resources available at a dispatch location regardless of avialability
	virtual vector< int > DetermineCurRescLevel( int timeYear, vector< CDispatchBase* > VDispatchers );		//determine the current level of resources by type for a dispatch location dispatcher
	vector< int > DetermineCurRescLevelatDeploy( CFire fire, list< CResource* > LDeployedResources );		//determine the current level of resources by type excluding the resources being deployed
	virtual void DetermineBaseRescLevel( vector< CDispatchBase* > VDispatchers );	//determine base resource level for a dispatcher.  For internal nodes need to be sure to calculated values for all subnodes first

	virtual bool CalcDailyUsageLevel(int Julian, vector<CDispatchBase*> VDisaptchers);			// Calculate the daily usage for a day for the dispatch locaton dispatcher

private:
	RescMap m_RescMap;										//multimap of resources at the dispatch location with deploy type key
	CDispLoc &m_DispLoc;									//reference to a dispatch location instance
	vector< WeightPair > m_VWeightPair;						//vector of pairs that consist of a pointer to a dispatcher and a weight
	list< CDLDispatcher* > m_AltHelicopterDLs;				//list of alternate helicopter dispatch locations for helitack at htis location

	static list< CResource* > s_Helicopters;				// list of available and unattached helicotpers for the fire
	static list< CResource* > s_Helitack;					// list of available and unattached helitack for the fire
	static list< CResource* > s_SmokeJumperAircraft;		// list of available and unattached smokejumper aircraft for the fire
	static list< CResource* > s_SmokeJumpers;				// list of available and unattached smokejumpers for the fire for the fire
	static list< CResource* > s_GroundResources;			// list of ground resources not in the dispatch logic for use as substitutes
	static list< CResource* > s_AerialResources;			// list of aerial resources not in the dispatch logic for use as substitutes
	static list< CResource* > s_LateArrivals;				// list of resources in the dispatch logic that were not currently available
	CalcDistance CalcDistMethod;

};		// end of class CDLDispatcher

#endif   // DLDISPATCHER_H