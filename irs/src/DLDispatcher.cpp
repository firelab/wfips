// DLDispatcher.cpp
// Member function definitions for class CDLDispatcher dispatch location dispatcher from CDispatchBase
// created 11/12 for IRS

#include <iostream>													//contains functions for input and output
using namespace std;
#include <fstream>													//contains functions for inputting and outputting to a file
using std::ifstream;												//for inputting from a file
#include <sstream>
#include <cctype>													//contains functions for characters, strings, and structures
#include <string>													//contains functions for operations with strings
using std::string;
#include <cstring>													//to convert a string to a const char*
#include <cstdlib>													//to use atof to convert const char* to int
#include <vector>
#include <list>
#include <map>

// Include definition for class CDLDispatcher from DLDispatcher.h
#include "DLDispatcher.h"
// Include definition for class CDispatchBase from DispatchBase.h
#include "DispatchBase.h"
// Include definition for class CDispLoc from DispLoc.h
#include "DispLoc.h"
// Include definition for class CResource from Rescbase.h
#include "Rescbase.h"
#include "CRescType.h"
#include "DispLoc.h"
#include "Helicopter.h"
#include "Helitack.h"
#include "SMJAircraft.h"
//#include "RunScenario.h"

// define and initialize to empty lists the static member variables 
list< CResource* > CDLDispatcher::s_Helicopters;				// list of available and unattached helicotpers for the fire
list< CResource* > CDLDispatcher::s_Helitack;					// list of available and unattached helitack for the fire
list< CResource* > CDLDispatcher::s_SmokeJumperAircraft;		// list of available and unattached smokejumper aircraft for the fire
list< CResource* > CDLDispatcher::s_SmokeJumpers;				// list of available and unattached smokejumpers for the fire for the fire
list< CResource* > CDLDispatcher::s_GroundResources;			// list of ground resources not in the dispatch logic for use as substitutes
list< CResource* > CDLDispatcher::s_AerialResources;			// list of aerial resources not in the dispatch logic for use as substitutes
list< CResource* > CDLDispatcher::s_LateArrivals;				// list of resources in the dispatch logic that were not currently available

// get the list of available and unattached helicotpers
list< CResource* > CDLDispatcher::GetHelicopters()
{	return s_Helicopters;	}

// get the list of available and unattached helitack
list< CResource* > CDLDispatcher::GetHelitack()
{	return s_Helitack;	}

// get the list of available and unattached smokejumper aircraft
list< CResource* > CDLDispatcher::GetSmokeJumperAircraft()
{	return s_SmokeJumperAircraft;	}

// get the list of available and unattached smokejumpers
list< CResource* > CDLDispatcher::GetSmokeJumpers()
{	return s_SmokeJumpers;	}

// get the list of ground resources not in the dispatch logic for use as substitutes
list< CResource* > CDLDispatcher::GetGroundResources()
{	return s_GroundResources;	}

// get the list of aerial resources not in the dispatch logic for use as substitutes
list< CResource* > CDLDispatcher::GetAerialResources()
{	return s_AerialResources;	}

// get the list of resources in the dispatch logic that were not currently available
list< CResource* > CDLDispatcher::GetLateArrivals()
{	return s_LateArrivals;	}

// empty helicopter list
void CDLDispatcher::EmptyHelicopters()
{	s_Helicopters.empty();	}

// empty helitack list
void CDLDispatcher::EmptyHelitack()
{	s_Helitack.empty();	}

// empty smokejumper aircraft list
void CDLDispatcher::EmptySmokeJumperAircraft()
{	s_SmokeJumperAircraft.empty();	}

// empty smokejumper list
void CDLDispatcher::EmptySmokeJumpers()
{	s_SmokeJumpers.empty();	}

// empty ground resources list
void CDLDispatcher::EmptyGroundResources()
{	s_GroundResources.empty();	}

// empty aerial resources list
void CDLDispatcher::EmptyAerialResources()
{	s_AerialResources.empty();	}

// empty late arrivals list
void CDLDispatcher::EmptyLateArrivals()
{	s_LateArrivals.empty();	}

// Constructor for CDLDispatcher
CDLDispatcher::CDLDispatcher( string dispatcherID, CDispLoc &DispLoc ) : CDispatchBase( dispatcherID ), m_DispLoc( DispLoc )
{
	CalcDistMethod = TTP;
}

// Copy constructor for CDLDispatcher
CDLDispatcher::CDLDispatcher( const CDLDispatcher &dldispatcher ) : CDispatchBase ( dldispatcher ), m_DispLoc( dldispatcher.m_DispLoc )
{
	m_RescMap = dldispatcher.m_RescMap;
	m_VWeightPair = dldispatcher.m_VWeightPair;						
	m_AltHelicopterDLs = dldispatcher.m_AltHelicopterDLs;	
	CalcDistMethod = dldispatcher.CalcDistMethod;
}

// Destructor for CDLDispatcher
CDLDispatcher::~CDLDispatcher()
{}

//  Equivalence operator for CDLDispatcher
bool CDLDispatcher::operator==( CDispatchBase &dispatchbase )
{
	bool Equal = false;

	// Get a pointer to the CDLDispatcher value for dispatchbase
	CDLDispatcher * dldispatcher = dynamic_cast < CDLDispatcher * > ( &dispatchbase );
	if (dldispatcher != 0 )	{
		Equal = true;
	
		if ( GetDispatcherID() != dldispatcher->GetDispatcherID() )
			Equal = false;
		if ( GetBaseRescLevel() != dldispatcher->GetBaseRescLevel() )
			Equal = false;
		if ( GetMinRescLevel() != dldispatcher->GetMinRescLevel() )
			Equal = false;
		if ( GetCurRescLevel() != dldispatcher->GetCurRescLevel() )
			Equal = false;
		if ( GetDailyMinRescLevel() != dldispatcher->GetDailyMinRescLevel() )
			Equal = false;
		if ( GetExpectLevelVector() != dldispatcher->GetExpectLevelVector() )
			Equal = false;
		if ( GetPreviousLevelVector() != dldispatcher->GetPreviousLevelVector() )
			Equal = false;
		if ( GetEscapeLevelVector() != dldispatcher->GetEscapeLevelVector() )
			Equal = false;
		if ( GetDispLogicUnfilledVector() != dldispatcher->GetDispLogicUnfilledVector() )
			Equal = false;

		if ( m_RescMap != dldispatcher->m_RescMap )
			Equal = false;
		if ( CalcDistMethod != dldispatcher->CalcDistMethod )
			Equal = false;
		if ( m_AltHelicopterDLs != dldispatcher->m_AltHelicopterDLs )
			Equal = false;

		if ( m_DispLoc == dldispatcher->m_DispLoc )
			Equal = true;
		else 
			Equal = false;
	}

	return Equal;
}

// Convert the resources dispatch type index to a string
string CDLDispatcher::ConvertDispType( int DispTypeIdx )
{	
	string DispType = "";
	
	switch (DispTypeIdx)	{

		case 0:
			DispType = "ATT";
			break;
		case 1:
			DispType = "CRW";
			break;
		case 2:
			DispType = "DZR";
			break;
		case 3:
			DispType = "ENG";
			break;
		case 4:
			DispType = "FBDZ";
			break;
		case 5:
			DispType = "FRBT";
			break;
		case 6:
			DispType = "Helicopter";
			break;
		case 7:
			DispType = "HELI";
			break;
		case 8:
			DispType = "SCPSEAT";
			break;
		case 9:
			DispType = "SJAC";
			break;
		case 10:
			DispType = "SMJR";
			break;
		case 11:
			DispType = "TP";
			break;
		case 12:
			DispType = "WT";
			break;
		case 13:
			DispType = "CRW20";
			break;
		case 14:
			DispType = "T1HEL";
			break;
		default:
			DispType = " ";
			cout << "No proper dispatch type index found";
			break;
	}

	return DispType;
}

// Convert the resources dispatch type index to a string
int CDLDispatcher::ConvertDispTypeIdx( string DispType)
{	
	int DispTypeIdx = -1;
	
	if ( DispType == "ATT" )
		DispTypeIdx = 0;
		
	if ( DispType == "CRW" )
		DispTypeIdx = 1;
		
	if ( DispType == "DZR" )
		DispTypeIdx = 2;
	
	if ( DispType == "ENG" )
		DispTypeIdx = 3;
	
	if ( DispType == "FBDZ" )
		DispTypeIdx = 4;
	
	if ( DispType == "FRBT" )
		DispTypeIdx = 5;

	if ( DispType == "Helicopter" )
		DispTypeIdx = 6;

	if ( DispType == "HELI" )
		DispTypeIdx = 7;

	if ( DispType == "SCPSEAT" )
		DispTypeIdx = 8;

	if ( DispType == "SJAC" )
		DispTypeIdx = 9;

	if ( DispType == "SMJR" )
		DispTypeIdx = 10;
	
	if ( DispType == "TP" )
		DispTypeIdx = 11;
	
	if ( DispType == "WT" )
		DispTypeIdx = 12;

	if ( DispType == "CRW20" )
		DispTypeIdx = 13;

	if ( DispType == "T1HEL" )
		DispTypeIdx = 14;

	if ( DispTypeIdx == -1 )
		cout << "No proper disptach type found \n";
	
	return DispTypeIdx;
}

// Set the dispatch location's resource map with another resource map
void CDLDispatcher::SetRescMap( RescMap rescmap )
{
	m_RescMap = rescmap;
}

// Return the dispatch location's resource map
RescMap CDLDispatcher::GetRescMap()
{
	return m_RescMap;
}

// Return a list of pointers to all the resources in the dispatch list
std::list< CResource* > CDLDispatcher::GetRescList()
{
	std::list< CResource* > LResourceList;
	
	// Iterate throught the resource map and return a list of the resources
	for ( RescMap::iterator Iter = m_RescMap.begin(); Iter != m_RescMap.end(); Iter++ )	{
		LResourceList.push_back( (*Iter).second );

	}

	return LResourceList;
}

// Insert a new resource index into the dispatch location's resource map
void CDLDispatcher::InsertInRescMap( CResource* resource )
{
	// Get the resource's dispatch type
	int DispTypeIdx = resource->GetDispatchType();
	string DispType = ConvertDispType( DispTypeIdx );

	// Check if the resource already exists in the map
	bool Found = false;
	RescMap::iterator It = m_RescMap.begin();

	while ( !Found && It!= m_RescMap.end() )	{

		if ( ( *It ).second == resource )
			Found = true;

		else
			It++;

	}

	if ( !Found )			// Add the resource to the map
		m_RescMap.insert( RescMap::value_type(DispType, resource ) );

	return;
}

// Remove a resource index from the dispatch location's resource map
void CDLDispatcher::RemoveFromRescMap( CResource* resource )
{	
	RescMap::iterator Iter = m_RescMap.begin();
	CResource* compresc = Iter->second;

	while ( compresc != resource && Iter != m_RescMap.end() )
	{
		Iter++;
		compresc = Iter->second;
	}

	// If the resource pointer is found remove the resource
	if ( compresc == resource )
		m_RescMap.erase( Iter );

	return;
}

// Get a list of the resources of a given dispatch type for the dispatch location
std::list< CResource* > CDLDispatcher::GetByDispType( string DispType )
{
	// Get the pair of iterators that define the beginning and end for the range of resources at the dispatch location of a given type	
	RescMap::iterator Iter;
	pair<multimap< string, CResource* >::iterator,multimap< string, CResource* >::iterator> range;
	range = m_RescMap.equal_range( DispType );

	// Create a list for the resource indexes
	std::list< CResource* > Resources;

	// Iterate through the map and put resources of desired type in list
	for ( Iter = range.first; Iter != range.second; Iter++ )
		Resources.push_back( ( *Iter ).second );

	return Resources;
}

// Get a random resource of the dispatch type from the dispatch location's resource map
CResource* CDLDispatcher::GetRandofType( string DispType )
{
	// Get a list of the resources of type
	std::list< CResource* > Resources = GetByDispType( DispType );

	// Get the size of the list
	int size = Resources.size();

	// Get a random number between 1 and size if size is greater than 1
	if ( size > 0 )		{

		int random = 1 + rand() % size;										//get a random number between 1 and size
		
		// Iterate through the resources of the type to select the random resource indicated above
		std::list< CResource* >::iterator Iter = Resources.begin();
		for ( int I = 1; I < random; I++ )	
			Iter++;
	
		return *Iter;
	}

	cout << "No Resources for the Dispatch Type at Dispatch Location\n";
	return 0;
}

// Set the dispatch location for the dispatcher
void CDLDispatcher::SetDispLoc( CDispLoc &disploc )
{	m_DispLoc = disploc;	}

// Get the dispatch location for the dispatcher
CDispLoc &CDLDispatcher::GetDispLoc()
{	return m_DispLoc;	}

// Determine the number of resources at the dispatch location by type
vector< int > CDLDispatcher::DetermineCurRescLevel()
{	// Create a vector of 15 integers and fill with 0s
	vector< int > VNumRescByType(15,0);

	// Iterate through the Multimap for the dispatch location and count the number of each resource type at the Dispatch Location
	RescMap::iterator Iter;
	for ( Iter = m_RescMap.begin(); Iter != m_RescMap.end(); Iter++ )	{

		// Get the string value for the dispatch type for the resource
		string DispType = (*Iter).first;

		int DispTypeIdx = ConvertDispTypeIdx( DispType );
				
		// Increase the count in the vector according to the dispatch type index value 
		VNumRescByType[DispTypeIdx] = VNumRescByType[DispTypeIdx] + 1;
		
	}

	return VNumRescByType;
}

//Set the weight pairs with a vector of weight pairs
void CDLDispatcher::SetWeightPairs( vector< WeightPair > WeightPair )
{	m_VWeightPair = WeightPair;	}

// Get the vector of weight pairs
vector< WeightPair > CDLDispatcher::GetWeightPairs()
{	return m_VWeightPair;	}

// Add a weight pair to the vector of weight pairs
void CDLDispatcher::AddWeightPair( WeightPair weightpair )
{	m_VWeightPair.push_back( weightpair );	}

// Get a weight pair from the vector by index
WeightPair CDLDispatcher::GetWeightPairByIndex( int index )
{	return m_VWeightPair[index];	}

//Set the alternate helicopter list with a list
void CDLDispatcher::SetAltHelicopterDLsList( list< CDLDispatcher* > AltHelicopterList )
{	m_AltHelicopterDLs = AltHelicopterList;	}

// Get the list of alternate helicopter dispatch location pointers
list< CDLDispatcher* > CDLDispatcher::GetAltHelicopterDLsList()
{	return m_AltHelicopterDLs;	}

// Add a dispatch location pointer to the list of alternate helicopter dispatch locations
bool CDLDispatcher::AddtoAltHelicopterDLsList( CDLDispatcher* DispLocPtr )
{
	// Check to see if the dispatch location pointer is already in the list
	list< CDLDispatcher* >::iterator It = m_AltHelicopterDLs.begin();
	bool found = false;

	while ( !found && It != m_AltHelicopterDLs.end() )	{

		if ( *It == DispLocPtr )
			found = true;
		else
			It++;

	}

	if ( !found )	{
		m_AltHelicopterDLs.push_back(DispLocPtr);
		return true;
	}

		return false;

}

// Remove a dispatch location pointer from the list of alternate helicopter dispatch locations
bool CDLDispatcher::RemovefromAltHelicopterDLSList( CDLDispatcher* DispLocPtr )
{	
	// find the dispatch location pointer in the list
	list< CDLDispatcher* >::iterator It = m_AltHelicopterDLs.begin();
	bool found = false;

	while ( !found && It != m_AltHelicopterDLs.end() )	{

		if ( *It  == DispLocPtr )
			found = true;
		else
			It++;

	}

	if ( found )	{
		m_AltHelicopterDLs.erase( It );
		return true;
	}

		return false;

}

list< CResource* > CDLDispatcher::DeployResources( vector< int > &RemainingDispLogic, CFire &Fire, list< CResource* > &DropHelicopters )
{	//Goes through the resources at the dispatch location and returns a list of those that are needed in the dispatch logic
	// and are currently available.  Reduces the dispatch logic by the deployed resources.  Aerial Delivered resources only deployed if
	// the crews and carrier are both available.  If the crews do not fill the dispatch logic the carrier value is not reduced in the 
	// dispatch logic.

	// Presets for options
	// Values that can be modified to adjust performance			
	int BufferTime = 0;												// Buffer time for defining currently available.  0 means that the resource is available at fire discovery
	int ATTTime = 30;												// Buffer time for Airtankers applies only to large airtankers	
	int ATTMaxDistance = 1000;										// Maximum distance from FWA TTP airtankers will be deployed from
	int ATTMaxTime = 120;											// Maximum time in minutes for the airtanker to arrive
	int HeliTime = 30;												// Buffer time for helicopters deploying helitack
	int SJACTime = 30;												// Buffer time for SJAC deploying smokejumpers


	list< CResource* > HelicopterList;								// List of helicopters available at the dispatch location to transport helitack
	list< CResource* > HelitackList;								// List of helitack that don't have a helicopter available ordered by distance
	list< CResource* > SJACList;									// List of SJAC available at the dispatch location to transport smokejumpers
	
	CFWA FWA = Fire.GetFWA();
	CDispLoc DispLoc = GetDispLoc();
	string DispLocID = DispLoc.GetDispLocID();

	//int FWAindex = FWA.GetIndex();
	//int AirMiles = DispLoc.GetFWAAssoc( FWAindex );
	double AirMiles = CalculateDistance( Fire, CalcDistMethod );
	double GroundMiles = FWA.GetAirtoGround() * AirMiles;
	int FirstUnitDelay = FWA.GetFirstUnitDelay();

	// list of deployed resources
	list< CResource* > LDeployedRescs;

	// Iterate through the dispatch logic and if resources are needed see what's currently available
	for ( int i = 0; i < 15; i++ )	{
		if ( RemainingDispLogic[i] > 0 )	{							// There are resources of this type needed
			
			string DispType = ConvertDispType( i );						// Convert the dispatch type to a string

			list< CResource* > LResources = GetByDispType(DispType );	// Get the resources of this type at the dispatch location

			if ( LResources.size() > 0 )								// Were there any resources at the dispatch location
			{
				// Iterate through the list to determine if resources are currently available
				list< CResource* >::iterator Iter = LResources.begin();
				while ( Iter != LResources.end() )	{
					
					bool IsAvailable = (*Iter)->IsRescAvailable( Fire );
					int AvailableTime = (*Iter)->GetAvailableTime();
					int FireStartTimeYr = Fire.FireStartTime() + ( (Fire.GetJulianDay() - 1 ) * 1440 );
					int TimeLimit = FireStartTimeYr + BufferTime;
					int InitArrivalTime = (*Iter)->CalcInitArrival( Fire, AirMiles );
					int Duration = (*Iter)->CalcRemainingWorkshift( Fire );
					string RescId = (*Iter)->GetRescID();
					
					// Using if and instead of Switch for resource type
					if ( i == 0 )		{
						TimeLimit = FireStartTimeYr + ATTTime;

						// If the large airtanker is not currently available and cannot get to the fire and begin work remove it from the list
						if ( InitArrivalTime > ATTMaxTime || AirMiles > ATTMaxDistance || AvailableTime > TimeLimit  
							|| Duration < FirstUnitDelay || !IsAvailable )
							Iter = LResources.erase( Iter );

						else 
							Iter++;

					}

					else if ( i == 6 )	{

						TimeLimit = FireStartTimeYr + HeliTime;
						CHelicopter* Helic = dynamic_cast< CHelicopter* >( *Iter );
						bool IsHelitackEquipped = false;
						if ( Helic )
							IsHelitackEquipped = Helic->GetHelitackEquipped();

						if ( !IsHelitackEquipped && AvailableTime <= TimeLimit && Duration > FirstUnitDelay && IsAvailable )
							DropHelicopters.push_back( *Iter );

						// If the helicopter is not currently available and cannot get to the fire and begin work remove it from the list
						if ( AvailableTime > TimeLimit	|| Duration < FirstUnitDelay || !IsAvailable || !IsHelitackEquipped )
							Iter = LResources.erase( Iter );

						else 
							Iter++;
												
					}

					else	{
						if ( i == 9 )	
						TimeLimit = FireStartTimeYr + SJACTime;						

						// If the resource is not currently available and cannot get to the fire and begin work remove it from the list
						if ( AvailableTime > TimeLimit  || Duration < FirstUnitDelay || !IsAvailable )
							Iter = LResources.erase( Iter );

						else
							Iter++;

					}

				}

			}

			// Get helicopters from dispatch locations in the alternate helicopter DLs list and add them to LResources so they can be used to deploy the helitack at this dispatch location
			if ( i == 6 )	{			
				list< CDLDispatcher* >::iterator ItDLs;

				for ( ItDLs = m_AltHelicopterDLs.begin(); ItDLs != m_AltHelicopterDLs.end(); ItDLs++ )	{

					// Get a list of the helicopters at this alternate dispatch location
					list< CResource* > AltHelicList = ( *ItDLs )->GetByDispType( "Helicopter" );

					for( list< CResource* >::iterator HeliIt = AltHelicList.begin(); HeliIt != AltHelicList.end(); HeliIt++ )	{
						
						bool IsAvailable = (*HeliIt)->IsRescAvailable( Fire );
						int AvailableTime = (*HeliIt)->GetAvailableTime();
						int FireStartTimeYr = Fire.FireStartTime() + ( (Fire.GetJulianDay() - 1 ) * 1440 );
						int TimeLimit = FireStartTimeYr + BufferTime;
						int InitArrivalTime = (*HeliIt )->CalcInitArrival( Fire, AirMiles );
						int Duration = (*HeliIt)->CalcRemainingWorkshift( Fire );
						string RescId = (*HeliIt)->GetRescID();

						TimeLimit = FireStartTimeYr + HeliTime;
						CHelicopter* Helic = dynamic_cast< CHelicopter* >( *HeliIt );
						bool IsHelitackEquipped = false;
						if ( Helic )
							IsHelitackEquipped = Helic->GetHelitackEquipped();

						// If the helicopter is not currently available and cannot get to the fire and begin work remove it from the list
						if ( AvailableTime <= TimeLimit	&& Duration > FirstUnitDelay && IsAvailable && IsHelitackEquipped )
							LResources.push_back( *HeliIt );

					}

				}

			}

			// Deploy from the resources left in the list if there are any.  Reduce the dispatch logic by the resources deployed
			if ( LResources.size() > 0 )	{
				bool DeploySJAC = false;
				bool DeployHeli = false;
				// Use i and else if to different methods for deploying resources
				if ( i == 6 )		// Is the resource type being deployed helicopter?
					HelicopterList = LResources;
				
				
				else if ( i == 7 )	{		// Is the resource type being deployed helitack?

					// Deploy the helitack
					DeployHeli = DeployHelitackNew( RemainingDispLogic, Fire, LDeployedRescs, LResources, HelicopterList, HelitackList, AirMiles );

					// Put Remaining Helitack and Helicotpers in the lists
					if ( !HelicopterList.empty() )
						s_Helicopters.splice( s_Helicopters.end(), HelicopterList );

					if ( !LResources.empty() )
						s_Helitack.splice( s_Helitack.end(), LResources );

				}

				else if ( i == 9 )	{		// Is the resource type being deployed smokejumper aircraft?

					// Add the SmokeJumper aircraft to the SJAC list so they can be used to deploy smokejumpers at the dispatch location
					SJACList = LResources;

					// Determine how to deploy smokejumper aircraft and smokejumpers
					DeploySJAC = DeploySmokejumpers( RemainingDispLogic, Fire, LDeployedRescs, AirMiles );

					if ( DeploySJAC )	{ // Deploy the smokejumper aircraft
						// Calculate The initial arrival time, duration for the resource
						list< CResource* >::iterator Iter = LResources.begin();
						int InitArrival = (*Iter)->CalcInitArrival( Fire, AirMiles );		// Calculate the initial arrival time for the resource
						(*Iter)->CalcRemainingWorkshift( Fire );		// Calculate the duration of time the resource can work fire
							
						// Need to set the Next Available time for the smokejumper aircraft to the initial arrival time so the end of the smokejumper aircraft's work is calculated correctly
						CSMJAircraft* SJACptr = dynamic_cast< CSMJAircraft* >( *Iter );

						if ( SJACptr != 0 )
							SJACptr->SetNextLoadArrival( InitArrival );
						
						LDeployedRescs.push_back( (*Iter) );		// Add a SJAC to the Deployed Resources List
						RemainingDispLogic[i] = RemainingDispLogic[i] - 1;
						
					}

					i++;

				}

				else if ( i != 10 )	{	// Is the resource type being deployed is not a smokejumper

					// If there are fewer resources in the list than needed, deploy all of them
					if ( LResources.size() <= RemainingDispLogic[i] )	{
								
						RemainingDispLogic[i] = RemainingDispLogic[i] - LResources.size();
						LDeployedRescs.splice( LDeployedRescs.end(), LResources );		// Add all the resources in LResources to the Deployed Resources List
					
							
					}

					// If there are more resources available than needed
					else	{
							
					LResources = OrderResources( Fire, LResources, AirMiles );

						// Deploy the desired number of resources from the ordered resource list
						list< CResource*>::iterator Iterator = LResources.begin();
						for ( int j = 0; j < RemainingDispLogic[i]; j++ )	{
							LDeployedRescs.push_back( *Iterator );
							Iterator++;
						}
	
						// Reduce the RemainingDispLogic value to 0
						RemainingDispLogic[i] = 0;

					}		// End of more resources available at dispatch location than in remaining dispatch logic list

				}		// End if, else if for resource type deployed

			}		// There are resources at dispatch location to deploy
			
		}		// End resource type is in remaining dispatch logic list
			
	}		// End of resource type iteration through dispatch logic

	return LDeployedRescs;
}


// Deploy Helicopters and helitack if they reside at the same base set the helicopter to a carrier so that it does not drop water.
// If the Helicopters are to be deployed to drop water they should reside at the GACC level and will have a different dispatch indicator
bool CDLDispatcher::DeployHelitack( vector< int > &RemainingDispLogic, CFire &Fire, list< CResource* > &LDeployedRescs, list< CResource* > &LResources, list< CResource*> &HelicopterList, double AirMiles )
{
	// Remove any helicopters or helitack that cannot arrive at the fire before sunset
	int sunset = Fire.ConvertTimeMSM(Fire.GetSunSet());
	list< CResource* >::iterator Iter;

	Iter = LResources.begin();
	while ( Iter != LResources.end() )	{
		int InitArrival = ( *Iter )->CalcInitArrival( Fire, AirMiles );
		int Time = Fire.FireStartTime() + InitArrival;
		if ( Time > sunset )
			Iter = LResources.erase( Iter );
		else
			Iter++;
	}

	if ( LResources.empty() )
		return false;

	Iter = HelicopterList.begin();
	while ( Iter != HelicopterList.end() )	{
		int InitArrival = ( *Iter )->CalcInitArrival( Fire, AirMiles );
		int Time = Fire.FireStartTime() + InitArrival;
		if ( Time > sunset )
			Iter = HelicopterList.erase( Iter );
		else
			Iter++;
	}
	
	// Are there any helicopters at the dispatch location
	if ( !HelicopterList.empty() )	{

		int FireNumber = Fire.GetFireNumber();

		// Determine the number of crews to be delivered to the fire
		int NumHeliNeeded = RemainingDispLogic[7];

		if ( NumHeliNeeded < LResources.size() )
			LResources = OrderResources( Fire, LResources, AirMiles );

		// Determine the number of crew members that need to be transported
		int NumCrewMembers = 0;
		if ( NumHeliNeeded > LResources.size() )
			NumHeliNeeded = LResources.size();

		Iter = LResources.begin();
		for ( int i = 0; i < NumHeliNeeded; i++ )	{
			NumCrewMembers = NumCrewMembers + ( *Iter )->GetStaffing();
			Iter++;
		}

		// Find the best helicopter in the list to deliver the helitack
		int Elevation = Fire.GetElevation();

		// If there is only one helicopter it's the one, otherwise order the list
		if ( HelicopterList.size() > 1 )	{
			list< CResource* > OrderedHelicopters;

			for ( Iter = HelicopterList.begin(); Iter != HelicopterList.end(); Iter++ )	{

				if ( OrderedHelicopters.empty() )
					OrderedHelicopters.push_back( *Iter );

				else	{

					CHelicopter* HelicopterPtr = dynamic_cast< CHelicopter* >( *Iter );
					if ( HelicopterPtr != 0 )	{
						int Capacity = HelicopterPtr->GetNumCrew( Elevation );
						int InitArrival = HelicopterPtr->CalcInitArrival( Fire, AirMiles );
						int Duration = HelicopterPtr->CalcRemainingWorkshift( Fire );

						// Iterate through the list to find placement for current resource and insert there
						list< CResource* >::iterator Iter2;
						Iter2 = OrderedHelicopters.begin();
						bool better = false;
						while ( !better && Iter2 != OrderedHelicopters.end() )	{
							// Get information for helicopter in the ordered list
							CHelicopter* HelicopterPtr2 = dynamic_cast< CHelicopter* >( *Iter2 );
							if ( HelicopterPtr2 != 0 )	{
								int Capacity2 = HelicopterPtr2->GetNumCrew( Elevation );
								int InitArrival2 = HelicopterPtr2->CalcInitArrival( Fire, AirMiles );
								int Duration2 = HelicopterPtr2->CalcRemainingWorkshift( Fire );

								// Compare the helicopters, if the new one is better set better equal to true
								if ( (Capacity > Capacity2 && Capacity2 < NumCrewMembers) || (Capacity >= NumCrewMembers && Capacity < Capacity2) )
									better = true;
							
								else if ( Capacity == Capacity2 && Duration > Duration2 )
									better = true;
								else
									Iter2++;

							}

							else
								cout << "Bad dynamic_cast for deploying helitack (inner)\n";

						}

						// Insert the helicopter in the ordered list
						OrderedHelicopters.insert( Iter2, *Iter );

					}

					else
						cout << "Bad dynamic_cast for deploying helitack (outter) \n";

				}
					
			}

			HelicopterList = OrderedHelicopters;

		}

		// Deploy helicopters and helitack 
		list < CResource* > TempDeploy;

		Iter = HelicopterList.begin();
		CResource* FirstHelicopter = ( *Iter );
		int CrewCount = 0;
		int CrewCountTotal = 0;
		int HelicopterCount = 0;
		int NumHelicoptersNeeded = RemainingDispLogic[6];
		
		while ( Iter != HelicopterList.end() && NumHeliNeeded > 0 && NumHelicoptersNeeded > 0 && LResources.size() > 0 )	{

			//cout << "Fire: " << Fire.GetFireNumber() << " Dispatch Location: " << GetDispatcherID() << "\n";
						
			TempDeploy.push_back( *Iter );
			HelicopterCount++;

			CHelicopter* HelicopterPtr = dynamic_cast< CHelicopter* >( *Iter );

			if ( HelicopterPtr != 0 )	{

				// Calculate time to reload 2* travel distance + carrier response delay to set the time the next crew would arrive
				int FWAIndex = Fire.GetFWA().GetIndex();
				CDispLoc CrewDispLoc = HelicopterPtr->GetDispLoc();
				string CrewDispLocID = CrewDispLoc.GetDispLocID();
				int NextCrewArrival = static_cast< int >( 2* Fire.GetFWA().GetDistance( CrewDispLocID ) * 60 /HelicopterPtr->GetRescType().GetAvgSpeed() + 0.5 );
				NextCrewArrival = NextCrewArrival + HelicopterPtr->GetRescType().GetResponseDelay();
				NextCrewArrival = NextCrewArrival + HelicopterPtr->GetInitArrivalTime();
				HelicopterPtr->SetNextLoadArrival( NextCrewArrival );
		
				// Set Carrier flag for the helicopter
				HelicopterPtr->SetIsCarrier( true );

				// Get the capacity for the first helicotper
				int Capacity = HelicopterPtr->GetNumCrew( Elevation );

				// Get the Crews
				list< CResource* >::iterator CrewIter = LResources.begin();
				int NumCrewMembers = ( *CrewIter )->GetStaffing();

				if ( NumCrewMembers > Capacity )		// there is not enough capacity in the helicotper to deploy the crews
					return false;

				// Get the first Load of crews for the helicopter
				while ( NumCrewMembers <= Capacity && CrewIter != LResources.end() && CrewCount < NumHeliNeeded )	{
					TempDeploy.push_back( *CrewIter );
					CrewCount++;

					CrewIter = LResources.erase( CrewIter );
					if ( CrewIter != LResources.end() )
						NumCrewMembers = NumCrewMembers + ( *CrewIter )->GetStaffing();

				}

			}

			else
				cout << "Bad helicopter pointer while attempting to deploy helicopters and helitack.\n";

			Iter = HelicopterList.erase( Iter );
			NumHeliNeeded = NumHeliNeeded - CrewCount;
			NumHelicoptersNeeded = NumHelicoptersNeeded - 1;

			CrewCountTotal = CrewCountTotal + CrewCount;
			CrewCount = 0;


		}

		// If there are still needed at the base and crews available at the base, take additional loads with the helicopter(s)
		if ( NumHeliNeeded > 0 && LResources.size() > 0 )	{
			// The first helicopter is the best
			CHelicopter* HelicopterPtr = dynamic_cast< CHelicopter* >( FirstHelicopter );

			if ( HelicopterPtr != 0 )	{

				// Get the arrival time for the next load of crews
				int NextLoadArrival = HelicopterPtr->GetNextLoadArrival();
					
				// Calculate time to reload 2* travel distance + carrier response delay to set the time the next crew would arrive
				//int FWAIndex = Fire.GetFWA().GetIndex();
				//CDispLoc CrewDispLoc = HelicopterPtr->GetDispLoc();
				//int NextCrewArrival = static_cast< int >( 2* CrewDispLoc.GetFWAAssoc( FWAIndex ) * 60 /HelicopterPtr->GetRescType().GetAvgSpeed() + 0.5 );
				//NextCrewArrival = NextCrewArrival + HelicopterPtr->GetRescType().GetResponseDelay();
				//NextCrewArrival = NextCrewArrival + HelicopterPtr->GetInitArrivalTime();
				//HelicopterPtr->SetNextLoadArrival( NextCrewArrival );

				// Get the capacity for the first helicotper
				int Capacity = HelicopterPtr->GetNumCrew( Elevation );

				// Get the Crews
				list< CResource* >::iterator CrewIter = LResources.begin();
				int NumCrewMembers = ( *CrewIter )->GetStaffing();

				// Get the first Load of crews for the helicopter
				while ( NumCrewMembers <= Capacity && CrewIter != LResources.end() && CrewCount < NumHeliNeeded )	{
					TempDeploy.push_back( *CrewIter );
					CrewCount++;

					// Reset the helitack crew's initial arrivial time to the next load arrival time
					( *CrewIter )->SetInitArrivalTime( NextLoadArrival );

					CrewIter = LResources.erase( CrewIter );
					if ( CrewIter != LResources.end() )
						NumCrewMembers = NumCrewMembers + ( *CrewIter )->GetStaffing();

				}

				// Reset the helicopter's next load arrival time to the time the last crew was delivered.  This will be used to release helicotper
				//HelicopterPtr->SetNextLoadArrival( NextLoadArrival);

				NumHeliNeeded = NumHeliNeeded - CrewCount;
				CrewCountTotal = CrewCountTotal + CrewCount;
				CrewCount = 0;

			}

			else
				cout << "Bad helicopter pointer while attempting to deploy helicopters and helitack.\n";

		}

		// Place the helicopters and helitack in the deployed resources list
		LDeployedRescs.splice( LDeployedRescs.end(), TempDeploy );

		// Adjust the remaining dispatch logic
		RemainingDispLogic[6] = RemainingDispLogic[6] - HelicopterCount;
		RemainingDispLogic[7] = RemainingDispLogic[7] - CrewCountTotal;

		if ( RemainingDispLogic[6] < 0 || RemainingDispLogic[7] < 0 )
			cout << "Something went wrong dispatching helitack!!!\n";

		return true;
	}

	return false;
}

// Revised version of deploy helitack 8/13
bool CDLDispatcher::DeployHelitackNew( vector< int > &RemainingDispLogic, CFire &Fire, list< CResource* > &LDeployedRescs, list< CResource* > &LResources, list< CResource* > &HelicopterList, list< CResource* > &HelitackList, double AirMiles  )
{
	// Remove any helicopters or helitack that cannot arrive at the fire before sunset
	int sunset = Fire.ConvertTimeMSM(Fire.GetSunSet());
	RemoveSunset( LResources, Fire, sunset, AirMiles );

	// Are there any helitack left at the Dispatch Location
	if ( LResources.empty() )
		return false;

	// If there is no helicopter at the base that is available within the time limit, add the available helitack to the helitack list
	if ( HelicopterList.empty() )	{
		HelitackList.splice( HelitackList.end(), LResources );
		return false;
	}

	RemoveSunset( HelicopterList, Fire, sunset, AirMiles );

	// Are there any helicopters at the dispatch location
	if ( !HelicopterList.empty() )	{

		int FireNumber = Fire.GetFireNumber();

		// Determine the number of crews to be delivered to the fire
		int NumHeliNeeded = RemainingDispLogic[7];

		if ( NumHeliNeeded < LResources.size() )
			LResources = OrderResources( Fire, LResources, AirMiles );

		// Determine the number of crew members that need to be transported
		int NumCrewMembers = 0;
		if ( NumHeliNeeded > LResources.size() )
			NumHeliNeeded = LResources.size();

		list< CResource* >::iterator Iter = LResources.begin();
		for ( int i = 0; i < NumHeliNeeded; i++ )	{
			NumCrewMembers = NumCrewMembers + ( *Iter )->GetStaffing();
			Iter++;
		}

		// Find the best helicopter in the list to deliver the helitack
		int Elevation = Fire.GetElevation();

		// If there is only one helicopter it's the one, otherwise order the list
		if ( HelicopterList.size() > 1 )	{
			list< CResource* > OrderedHelicopters;
			Iter = LResources.begin();
			string DispLocIDHeli = ( *Iter )->GetDispLoc().GetDispLocID();

			for ( Iter = HelicopterList.begin(); Iter != HelicopterList.end(); Iter++ )	{

				if ( OrderedHelicopters.empty() )
					OrderedHelicopters.push_back( *Iter );

				else	{

					CHelicopter* HelicopterPtr = dynamic_cast< CHelicopter* >( *Iter );
					if ( HelicopterPtr != 0 )	{
						int Capacity = HelicopterPtr->GetNumCrew( Elevation );
						int InitArrival = HelicopterPtr->CalcInitArrival( Fire, AirMiles );
						int Duration = HelicopterPtr->CalcRemainingWorkshift( Fire );
						bool SameDL = true;
						string HelDL = HelicopterPtr->GetDispLoc().GetDispLocID();
						if ( DispLocIDHeli != HelDL )
							SameDL = false;

						// Iterate through the list to find placement for current resource and insert there
						list< CResource* >::iterator Iter2;
						Iter2 = OrderedHelicopters.begin();
						bool better = false;
						while ( !better && Iter2 != OrderedHelicopters.end() )	{
							// Get information for helicopter in the ordered list
							CHelicopter* HelicopterPtr2 = dynamic_cast< CHelicopter* >( *Iter2 );
							if ( HelicopterPtr2 != 0 )	{
								int Capacity2 = HelicopterPtr2->GetNumCrew( Elevation );
								int InitArrival2 = HelicopterPtr2->CalcInitArrival( Fire, AirMiles );
								int Duration2 = HelicopterPtr2->CalcRemainingWorkshift( Fire );
								bool SameDL2 = true;
								string HelDL2 = HelicopterPtr2->GetDispLoc().GetDispLocID();
								if ( DispLocIDHeli != HelDL2 )
									SameDL2 = false;

								// Compare the helicopters, if the new one is better set better equal to true
								if ( SameDL && !SameDL2 )
									better = true;
								
								if ( (Capacity > Capacity2 && Capacity2 < NumCrewMembers) || (Capacity >= NumCrewMembers && Capacity < Capacity2) )
									better = true;
							
								else if ( Capacity == Capacity2 && Duration > Duration2 )
									better = true;

								if ( !SameDL && SameDL2 )
									better = false;

								if ( better == false )
									Iter2++;

							}

							else
								cout << "Bad dynamic_cast for deploying helitack (inner)\n";

						}

						// Insert the helicopter in the ordered list
						OrderedHelicopters.insert( Iter2, *Iter );

					}

					else
						cout << "Bad dynamic_cast for deploying helitack (outer) \n";

				}
					
			}

			HelicopterList = OrderedHelicopters;

		}

		// Deploy helicopters and helitack 
		list < CResource* > TempDeploy;

		Iter = HelicopterList.begin();
		CResource* FirstHelicopter = ( *Iter );
		int CrewCount = 0;
		int CrewCountTotal = 0;
		int HelicopterCount = 0;
		int NumHelicoptersNeeded = RemainingDispLogic[6];
		
		while ( Iter != HelicopterList.end() && NumHeliNeeded > 0 && NumHelicoptersNeeded > 0 && LResources.size() > 0 )	{

			//cout << "Fire: " << Fire.GetFireNumber() << " Dispatch Location: " << GetDispatcherID() << "\n";
						
			TempDeploy.push_back( *Iter );
			HelicopterCount++;

			CHelicopter* HelicopterPtr = dynamic_cast< CHelicopter* >( *Iter );

			if ( HelicopterPtr != 0 )	{

				// Calculate time to reload 2* travel distance + carrier response delay to set the time the next crew would arrive
				CDispLoc CrewDispLoc = HelicopterPtr->GetDispLoc();
				string CrewDispLocID = CrewDispLoc.GetDispLocID();
				int NextCrewArrival = static_cast< int >( 2* Fire.GetFWA().GetDistance( CrewDispLocID ) * 60 /HelicopterPtr->GetRescType().GetAvgSpeed() + 0.5 );
				NextCrewArrival = NextCrewArrival + HelicopterPtr->GetRescType().GetResponseDelay();
				NextCrewArrival = NextCrewArrival + HelicopterPtr->GetInitArrivalTime();
				HelicopterPtr->SetNextLoadArrival( NextCrewArrival );
		
				// Set Carrier flag for the helicopter
				HelicopterPtr->SetIsCarrier( true );

				// Get the capacity for the first helicotper
				int Capacity = HelicopterPtr->GetNumCrew( Elevation );

				// Get the Crews
				list< CResource* >::iterator CrewIter = LResources.begin();
				int NumCrewMembers = ( *CrewIter )->GetStaffing();

				if ( NumCrewMembers > Capacity )		// there is not enough capacity in the helicotper to deploy the crews
					return false;

				// Get the first Load of crews for the helicopter
				while ( NumCrewMembers <= Capacity && CrewIter != LResources.end() && CrewCount < NumHeliNeeded )	{
					TempDeploy.push_back( *CrewIter );
					CrewCount++;

					CrewIter = LResources.erase( CrewIter );
					if ( CrewIter != LResources.end() )
						NumCrewMembers = NumCrewMembers + ( *CrewIter )->GetStaffing();

				}

			}

			else
				cout << "Bad helicopter pointer while attempting to deploy helicopters and helitack.\n";

			Iter = HelicopterList.erase( Iter );
			NumHeliNeeded = NumHeliNeeded - CrewCount;
			NumHelicoptersNeeded = NumHelicoptersNeeded - 1;

			CrewCountTotal = CrewCountTotal + CrewCount;
			CrewCount = 0;


		}

                bool AllBigCrewFlag = false;
                bool BigCrewCheck = false;
		// If there are still Helitack needed at the fire and crews available at the base, take additional loads with the helicopter(s)
		while ( NumHeliNeeded > 0 && LResources.size() > 0 && !AllBigCrewFlag )	{
			// The first helicopter is the best
			CHelicopter* HelicopterPtr = dynamic_cast< CHelicopter* >( FirstHelicopter );

                        BigCrewCheck = false;

			if ( HelicopterPtr != 0 )	{

				// Get the arrival time for the next load of crews
				int NextLoadArrival = HelicopterPtr->GetNextLoadArrival();
					
				// Calculate time to reload 2* travel distance + carrier response delay to set the time the next crew would arrive
				CDispLoc CrewDispLoc = HelicopterPtr->GetDispLoc();
				string CrewDispLocID = CrewDispLoc.GetDispLocID();
				int NextCrewArrival = static_cast< int >( 2* Fire.GetFWA().GetDistance( CrewDispLocID ) * 60 /HelicopterPtr->GetRescType().GetAvgSpeed() + 0.5 );
				NextCrewArrival = NextCrewArrival + HelicopterPtr->GetRescType().GetResponseDelay();
				NextCrewArrival = NextCrewArrival + HelicopterPtr->GetInitArrivalTime();
				HelicopterPtr->SetNextLoadArrival( NextCrewArrival );

				// Get the capacity for the first helicotper
				int Capacity = HelicopterPtr->GetNumCrew( Elevation );

				// Get the Crews
				list< CResource* >::iterator CrewIter = LResources.begin();
				int NumCrewMembers = ( *CrewIter )->GetStaffing();
                                if( NumCrewMembers < NumHeliNeeded )
                                    BigCrewCheck = true;

				// Get the first Load of crews for the helicopter
				while ( NumCrewMembers <= Capacity && CrewIter != LResources.end() && CrewCount < NumHeliNeeded )	{
					TempDeploy.push_back( *CrewIter );
					CrewCount++;

					// Reset the helitack crew's initial arrivial time to the next load arrival time
					( *CrewIter )->SetInitArrivalTime( NextLoadArrival );

					CrewIter = LResources.erase( CrewIter );
					if ( CrewIter != LResources.end() )
						NumCrewMembers = NumCrewMembers + ( *CrewIter )->GetStaffing();

				}

				// Reset the helicopter's next load arrival time to the time the last crew was delivered.  This will be used to release helicotper
				HelicopterPtr->SetNextLoadArrival( NextLoadArrival);

				NumHeliNeeded = NumHeliNeeded - CrewCount;
				CrewCountTotal = CrewCountTotal + CrewCount;
				CrewCount = 0;

			}

			else
				cout << "Bad helicopter pointer while attempting to deploy helicopters and helitack.\n";

                        if( !BigCrewCheck )
                            AllBigCrewFlag = true;
		}

		// Place the helicopters and helitack in the deployed resources list
		LDeployedRescs.splice( LDeployedRescs.end(), TempDeploy );

		// Adjust the remaining dispatch logic
		RemainingDispLogic[6] = RemainingDispLogic[6] - HelicopterCount;
		RemainingDispLogic[7] = RemainingDispLogic[7] - CrewCountTotal;

		if ( RemainingDispLogic[6] < 0 || RemainingDispLogic[7] < 0 )
			cout << "Something went wrong dispatching helitack!!!\n";

		return true;
	}

	return false;
}

// Deploy smokejumpers and smokejumper aircraft  -- know there are smokejumper aircraft available at the base
// In this version if there is a smokejumper aircraft at the base it will deploy the smokejumpers that are there.
bool CDLDispatcher::DeploySmokejumpers( vector< int > &RemainingDispLogic, CFire &Fire, list< CResource* > &LDeployedRescs, double AirMiles )
{	
	bool DeploySJAC = false;
	int i = 9;

	// Are there any smokejumpers currently available at the dispatch location
	if ( RemainingDispLogic[i+1] > 0 )	{								// There are resources of this type needed
			
		string DispType = ConvertDispType( i+1 );						// Convert the dispatch type to a string

		list< CResource* > LResources = GetByDispType( DispType );		// Get the resources of this type at the dispatch location

		if ( LResources.size() > 0 )									// Were there any resources at the dispatch location
		{
			// Iterate through the list to determine if resources are currently available
			list< CResource* >::iterator Iter = LResources.begin();
			while ( Iter != LResources.end() )	{
				// Determine if this fire is on a new day since the resource last worked
				int entries = (*Iter)->WorkYearSize();				// Get the size of the Resource Work Year vector for the resource

				// If there are values in the resource's Work Year 
				if ( entries > 0 )	{
					CRescWorkYear WorkYearLast = (*Iter)->GetWorkYearEntry( entries-1 );	// Get the last entry
					int LastDay = WorkYearLast.GetJulianDay();
					int FireDay = Fire.GetJulianDay();

					// If new day reset the resource values
					if ( LastDay != FireDay )	{

						(*Iter)->SetWorkshiftStartTime( -1);
						// Add rest time to resources if necessary
						(*Iter)->AdjustAvailableTime( );
					
					}

				}

				bool IsAvailable = (*Iter)->IsRescAvailable( Fire );
				int AvailableTime = (*Iter)->GetAvailableTime();
				int FireStartTimeYr = Fire.FireStartTime() + ( (Fire.GetJulianDay() - 1 ) * 1440 );
				(*Iter)->CalcInitArrival( Fire, AirMiles );
				(*Iter)->CalcRemainingWorkshift( Fire );
					
				// If the resource is not currently available and cannot get to the fire and begin work remove it from the list
				if ( AvailableTime > FireStartTimeYr  || (*Iter)->GetDuration() < Fire.GetFWA().GetFirstUnitDelay() || 
					!IsAvailable )
					Iter = LResources.erase( Iter );

				else
					Iter++;
			}
		}

		// Now have a list of the currently available smokejumpers
		// If there are some deploy the smokejumper aircraft and the smokejumpers
		if ( LResources.size() > 0 )
		{	DeploySJAC = true;
			
			if ( LResources.size() <= RemainingDispLogic[i+1] )	{
				for ( list< CResource* >::iterator Iter = LResources.begin(); Iter != LResources.end(); Iter ++ )	{
					// Calculate The initial arrival time, duration and production rate for the resource
					(*Iter)->CalcInitArrival( Fire, AirMiles );				// Calculate the initial arrival time for the resource
					(*Iter)->CalcRemainingWorkshift( Fire );		// Calculate the duration of time the resource can work fire
				}
					
				RemainingDispLogic[i+1] = RemainingDispLogic[i+1] - LResources.size();	
				LDeployedRescs.splice( LDeployedRescs.end(), LResources );		// Add all the resources in LResources to the Deployed Resources List
					
									
			}

			// If there are more resources available than needed
			else	{

				// Determine the duration for the resources and order accordingly
				list< CResource* >::iterator Iter;

				//Create an ordered list of the resources
				list< CResource* > LOrderedResc;


				for ( Iter= LResources.begin(); Iter != LResources.end(); Iter++ )	{
					// Determine the initial arrival time, duration and 
					(*Iter)->CalcInitArrival( Fire, AirMiles );			// Calculate the initial arrival time for the resource
					(*Iter)->CalcRemainingWorkshift( Fire );		// Calculate the duration of time the resource can work fire
												
					
					if ( LOrderedResc.empty() )
						LOrderedResc.push_back( *Iter );
					else	{
						// Iterate through the list to find placement for current resource and insert there
						int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
						list< CResource* >::iterator Iterator;
						Iterator = LOrderedResc.begin();
						bool better = false;
						while ( !better && Iterator != LOrderedResc.end() )	{
							better = (*Iter)->DeploySelectCompare( *Iterator );		// Is this resource better than resource in present position of list?
							if ( !better )
								Iterator++;	}

						// If the resource isn't the best so far
						if ( Iterator != LOrderedResc.begin() )	{
							
							bool equal = true;
							while ( equal && Iterator != LOrderedResc.begin() )	{
								Iterator--;
								equal = (*Iter)->DeploySelectEqual( *Iterator );		// Is this resource equal to the resource in present position of list?
								//cout << " equal? " << equal << "\n";
								if ( equal )	
									k++;
							}
							Iterator++;

							// Place the resource in the appropriate spot
							if ( k > 0 )	{	
								int random = rand() % k + 1;
								//cout << "Random " << random << "\n";
								for ( int j = 1; j < random ; j++ )
									Iterator++;	}

						}

						// Insert pointer to resource into list
						LOrderedResc.insert( Iterator, *Iter );
					}

					
				}

				LResources = LOrderedResc;


				// Deploy the desired number of resources from the ordered resource list
				list< CResource*>::iterator Iterator = LResources.begin();
				for ( int j = 0; j < RemainingDispLogic[i+1]; j++ )	{
					LDeployedRescs.push_back( *Iterator );
					Iterator++;
				}

				// Reduce the RemainingDispLogic value to 0
				RemainingDispLogic[i+1] = 0;

			}
		}
	}

	return DeploySJAC;
}

// Remove resources from list that will not arrive before sunset
void CDLDispatcher::RemoveSunset( list< CResource* > &Resources, CFire &Fire, int Sunset, double AirMiles )
{
	list< CResource* >::iterator Iter;

	Iter = Resources.begin();
	while ( Iter != Resources.end() )	{
		int InitArrival = ( *Iter )->CalcInitArrival( Fire, AirMiles );
		int Time = Fire.FireStartTime() + InitArrival;
		if ( Time > Sunset )
			Iter = Resources.erase( Iter );
		else
			Iter++;
	}
}

// Determine the current level of resources available at a dispatch location dispatcher 
vector< int > CDLDispatcher::DetermineCurRescLevel( int timeYear, vector< CDispatchBase* > VDispatchers )
{
	vector< int > RescNums( 15, 0 );									// Create the vector and fill with 0s
	
	RescMap ResourceMap = GetRescMap();
	RescMap::iterator It = ResourceMap.begin();

	for ( It = ResourceMap.begin(); It != ResourceMap.end(); It++ )	{
		
		// Check the next available time
		int AvailableTimeYear = (*It).second->GetAvailableTime();

		// If the resource is available
		if ( AvailableTimeYear <= timeYear )	{
		
			// Get the dispatch type
			int DispatchType = (*It).second->GetDispatchType();

			// Increment Resource Type
			RescNums[DispatchType] = RescNums[DispatchType] + 1;
		}
	}

	// Set Current resource numbers
	SetCurRescLevel( RescNums );

	// Reset the Daily Minimum numbers
	for ( int i = 0; i < 15; i++ )	{
		if ( GetDailyMinRescLevelIdx( i ) > RescNums[i] )
			SetDailyMinRescLevelIdx( i, RescNums[i] );
	}

	return RescNums;
}

// Determine the current level of resources available at a dispatch location disptacher - excluding resources in the deployed resources list
vector< int > CDLDispatcher::DetermineCurRescLevelatDeploy( CFire fire, list< CResource* > LDeployedResources )
{
	// Determine the time of the fire start in minutes since the beginning of the year
	int timeYear = ( fire.GetJulianDay() - 1 ) * 1440 + fire.FireStartTime();

	vector< int > RescNums( 15, 0 );									// Create the vector and fill with 0s
	
	RescMap ResourceMap = GetRescMap();
	RescMap::iterator It = ResourceMap.begin();

	for ( It = ResourceMap.begin(); It != ResourceMap.end(); It++ )	{

		// Check the next available time
		int AvailableTimeYear = (*It).second->GetAvailableTime();

		// Check if the resource is in the deployed resources list
		string RescId = (*It).second->GetRescID();
		bool Deployed = false;
		for ( list< CResource* >::iterator Iter = LDeployedResources.begin(); Iter != LDeployedResources.end(); Iter++ )	{
			
			string DeployedRescID = (*Iter)->GetRescID();
			if ( DeployedRescID == RescId )
				Deployed = true;
		}

		// If the resource is available
		if ( AvailableTimeYear <= timeYear && !Deployed )	{
		
			// Get the dispatch type
			int DispatchType = (*It).second->GetDispatchType();

			// Increment Resource Type
			RescNums[DispatchType] = RescNums[DispatchType] + 1;
		}
	}

	// Update the current resource levels
	SetCurRescLevel( RescNums );

	// Update the Daily minimum resource level for each resource type
	for ( int i = 0; i < 15; i++ )	{
		if ( GetDailyMinRescLevelIdx( i ) > GetCurRescLevelIdx( i ) )
			SetDailyMinRescLevelIdx( i, GetCurRescLevelIdx( i ) );
	}

	return RescNums;
}

// Determine the base resource levels for dispatch location - the vector sent to the function is not used
void CDLDispatcher::DetermineBaseRescLevel( vector< CDispatchBase* > VDispatchers )
{
	// Count the number of resources by type entered into the resource map for this dispatch location
	vector< int > RescNums( 15, 0 );									// Create the vector and fill with 0s
	
	RescMap ResourceMap = GetRescMap();
	RescMap::iterator It;

	for ( It = ResourceMap.begin(); It != ResourceMap.end(); It++ )	{

		// Get the dispatch type
		int DispatchType = (*It).second->GetDispatchType();
		
		// Increment Resource Type
		RescNums[DispatchType] = RescNums[DispatchType] + 1;
		
	}

	// Set the base resource level for the dispatcher
	SetBaseRescLevel( RescNums );

}

// Iterate through resources at the dispatch location and reset the WorkShiftStartTime, AvailableTime, AvailableFlag, and RescWorkYear
void CDLDispatcher::ResetNewScenario( int scenario )	
{
	ofstream outFile( "Levels.dat" , ios::app );
	  
	// Exit program if unable to create file
	if ( !outFile ) { 
		cerr << "Output file could not be opened" << endl;
		//exit( 1 );
	}
	
	// Print out the data for the 365 days of the year
	if ( scenario > 0 )	{
		string DispatcherID = GetDispatcherID();
		outFile << "Dispatcher ID: " << DispatcherID << "\n";

		// Add the levels information to the levels.dat file
		for ( int i = 0; i < 365; i++ )
			outFile << scenario << ", " << i << ", " << GetExpectLevel(i+1) << ", " << GetPreviousLevel(i+1) << ", " << GetEscapeLevel(i+1) << ", " <<
			GetDispLogicUnfilled(i+1) << "\n";
	}

	for ( int i=0; i<15; i++)	{
		SetCurRescLevelIdx( i, 0 );
		SetDailyMinRescLevelIdx( i, 10000 );
	}
	
	for ( int i = 0; i < 365; i++ )	{
		//SetExpectLevel( 0.0, i+1 );		Expected Level is loaded when the fire file is read in
		SetPreviousLevel( 1.0, i+1 );
		SetEscapeLevel( 0, i+1 );
		SetDispLogicUnfilled( 0, i+1 );
	}
	
	for ( RescMap::iterator Iter= m_RescMap.begin(); Iter != m_RescMap.end(); Iter++ )	{
			( *Iter ).second->SetWorkshiftStartTime( -1);
			( *Iter ).second->ResetAvailableTime();
			( *Iter ).second->ClearRescWorkYear();
			( *Iter ).second->SetAvailableFlag( false );
		

			// If the resource is a helicopter set the flag to indicate that it is not used as a carrier
			CHelicopter *Helic = dynamic_cast< CHelicopter * >(( *Iter ).second);
			if ( Helic != 0 )	{
				Helic->SetIsCarrier( false );
		}
	}

}

// Reset all the resource values for a new day
void CDLDispatcher::ResetNewDay( int Julian )
{
	// Reset all the daily values for the resources including next available time etc.
	list< CResource* > LResources = GetRescList();					// Get the resourcesat the dispatch location

	if ( LResources.size() > 0 )							// Were there any resources at the dispatch location
	{
		// Iterate through the list and reset the resource values
		list< CResource* >::iterator Iter = LResources.begin();
		while ( Iter != LResources.end() )	{
			(*Iter)->SetWorkshiftStartTime( -1);

			// Add rest time to resources if necessary
			(*Iter)->AdjustAvailableTime( ); 
			
			//Determine if the resource is out of season and set the Available Time beyond the end of the year
			(*Iter)->EndSeasonAvailableTime( Julian );

			// If the resource is a helicopter set the flag to indicate that it is not used as a carrier
			CHelicopter *Helic = dynamic_cast< CHelicopter * >( *Iter );
			if ( Helic != 0 )	{
				Helic->SetIsCarrier( false );
			}

			Iter++;

		}
	}

	// Need to calculate the previous use level which will use these values
	// Calculate the Daily minimum number of resources
	int sum = 0;
	for ( int i = 0; i < 15; i++ )	{
		if ( GetDailyMinRescLevelIdx(i) < 10000 )
		sum = sum + GetDailyMinRescLevelIdx(i);
	}

	// Determine the previous level for the day as the proportion of the base level of resources
	int sumbase = 0;
	for ( int i = 0; i < 15; i++ )
		sumbase = sumbase + GetBaseRescLevelIdx(i);

	double sumd = static_cast<double>( sum );
	double sumbased = static_cast<double>( sumbase );

	if ( sumbase > 0 )	{
		double value  = sumd / sumbased; 
		SetPreviousLevel( value, Julian ); 
	}
	else
		SetPreviousLevel( 1, Julian );

	// Reset the m_DailyMinRescLevel vector
	for ( int i = 0; i < 15; i++ )
		SetDailyMinRescLevelIdx( i, 1000 );
}

// Check if the dispatch location has resources in season on the julian date
bool CDLDispatcher::HasRescInSeason( int Julian )	
{
	bool InSeason = false;
	list< CResource* > RescList = GetRescList();

	for ( list< CResource* >::iterator It = RescList.begin(); It != RescList.end(); It++ )	{
		// If the resource is in season
		int SeasonBegin = (*It)->GetStartSeason();
		int SeasonEnd = (*It)->GetEndSeason();

		if ( Julian >= SeasonBegin && Julian <= SeasonEnd )
			InSeason = true;
	}

	return InSeason;

}

// Order a list of CResources
list< CResource* > CDLDispatcher::OrderResources( CFire Fire, list< CResource* > LResources, double AirMiles )
{
	//Create an ordered list of the resources
	list< CResource* > LOrderedResc;

	// Determine the duration for the resources and order accordingly
	list< CResource* >::iterator Iter;
	for ( Iter = LResources.begin(); Iter != LResources.end(); Iter++ )	{
		// Determine the initial arrival time, duration and 
		int InitArrivalTime = (*Iter)->CalcInitArrival( Fire, AirMiles );		// Calculate the initial arrival time for the resource
		int Duration = (*Iter)->CalcRemainingWorkshift( Fire );		// Calculate the duration of time the resource can work fire
												
		if ( LOrderedResc.empty() )
			LOrderedResc.push_back( *Iter );

		else	{
			// Iterate through the list to find placement for current resource and insert there
			int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
			list< CResource* >::iterator Iterator;
			Iterator = LOrderedResc.begin();
			bool better = false;
			while ( !better && Iterator != LOrderedResc.end() )	{
				better = (*Iter)->DeploySelectCompare( *Iterator );		// Is this resource better than resource in present position of list?
				if ( !better )
					Iterator++;	
			}

			// If the resource isn't the best so far
			if ( Iterator != LOrderedResc.begin() )	{

				bool equal = true;
				while ( equal && Iterator != LOrderedResc.begin() )	{
					Iterator--;
					equal = (*Iter)->DeploySelectEqual( *Iterator );		// Is this resource equal to the resource in present position of list?
					
					if ( equal )	
						k++;
				}

				Iterator++;

				// Place the resource in the appropriate spot
				if ( k > 0 )	{	
					int random = rand() % k + 1;
					//cout << "Random " << random << "\n";
					for ( int j = 1; j < random ; j++ )
						Iterator++;	
				}

			}
			
			// Insert pointer to resource into list
			LOrderedResc.insert( Iterator, *Iter );
		}

							
	}

	return LOrderedResc;

}

// Number of resources, available and unavailable at the dispatcher
int CDLDispatcher::NumRescIdx( int Index )
{	
	RescMap ResourceMap = GetRescMap();
	RescMap::iterator It = ResourceMap.begin();
	vector< int > RescNums;
	for ( int i = 0; i < 15; i++ )
		RescNums.push_back( 0 );

	for ( It = ResourceMap.begin(); It != ResourceMap.end(); It++ )	{

		// Get the dispatch type
		int DispatchType = (*It).second->GetDispatchType();

		// Increment Resource Type
		RescNums[DispatchType] = RescNums[DispatchType] + 1;
		
	}

	return RescNums[ Index ];

}

// Calculate the distance from the Dispatch Location and the Fire
double CDLDispatcher::CalculateDistance( CFire fire, CalcDistance CalcDistMethod )
{
	// To convert degrees to radians
	double conversion = 3.14159265 / 180;

	double distance = 1000;
		
	if ( CalcDistMethod == TTP )	{

		string DispLocID = GetDispLoc().GetDispLocID();

		CFWA FWA = fire.GetFWA();
		
		distance = FWA.GetDistance( DispLocID );

	}

	if ( CalcDistMethod == LINEAR )	{

		double DLLat = GetDispLoc().GetLatitude();
		double DLLong = GetDispLoc().GetLongitude();

		double FireLat = fire.GetLatitude();
		double FireLong = fire.GetLongitude();

		distance = 111.12 *  sqrt( ( DLLat - FireLat ) * ( DLLat - FireLat ) + ( DLLong - FireLong ) * ( DLLong - FireLong ) ) * 0.621;

	}

	if ( CalcDistMethod == HAVERSINE )	{

		

	}

	if ( CalcDistMethod == STEW )	{

		double FromLat = GetDispLoc().GetLatitude() * conversion;
		double FromLong = GetDispLoc().GetLongitude() * conversion;
		double ToLat = fire.GetLatitude() * conversion;
		double ToLong = fire.GetLongitude() * conversion;

		// Calculate the change in latitude and longitude
		double DeltaLat = ( ToLat - FromLat );
		double DeltaLong = ( ToLong - FromLong );

		// Calculate distance based on curvature of the earth
		double temp = pow( sin( DeltaLat/2.0 ), 2 ) + cos( FromLat ) * cos ( ToLat ) * pow( sin( DeltaLong / 2.0 ), 2 );
		distance = 3956 * 2 * atan2( sqrt( temp ), sqrt( 1-temp ));

	}

	if ( CalcDistMethod == INTERNET )	{

		double FromLat = GetDispLoc().GetLatitude() * conversion;
		double FromLong = GetDispLoc().GetLongitude() * conversion;
		double ToLat = fire.GetLatitude() * conversion;
		double ToLong = fire.GetLongitude() * conversion;

		// Calculate distance 
		double temp = ( sin( ToLat ) * sin( FromLat ) ) + ( cos( ToLat ) * cos( FromLat) * cos( ToLong - FromLong ) );
		distance = 3693 * atan2( sqrt( 1 - (temp * temp) ), temp );

	}

	if ( distance < 0.0 )
		cout << "Problem with the distance calculation!\n";

	return distance;

}


bool CDLDispatcher::CalcDailyUsageLevel(int Julian, vector<CDispatchBase*> VDisaptchers)
{
	// Get the number of resources at the dispatch location dispatcher 
	vector<int> numResources = GetCurRescLevel();

	int count = 0;
	for (int i = 0; i < numResources.size(); i++)
		count = count + numResources[i];

	// Get the number of resources deployed
	int deployed = GetDailyDispatchLevel(Julian);

	if (count == 0 || deployed == 0)
		SetDailyUsage(0,Julian);
	else	{
		double dailyUsage = static_cast<double>(deployed)/count;
		SetDailyUsage(dailyUsage, Julian);
	}

	return true;
}
