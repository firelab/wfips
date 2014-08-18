// TypeTwoIACrew.cpp
// definitions for class CTypeTwoIACrew for 20 person Type II IA crews that can be used as 4 - 5 person IA handcrews
// for IRS 11/11

#include <iostream>												//contains functions for input and output
using namespace std;
#include <string>												//contains functions for operations with strings
using std::string;
#include <vector>
#include <map>
#include <cstring>												//to convert a string to a const char*
#include <cstdlib>												//to use atof to convert const char* to int and random numbers

// Include definition for class CTypeTwoIACrew from TypeTwoIACrew.h
#include "TypeTwoIACrew.h"
// Include definition for class CResource from Rescbase.h
#include "Rescbase.h"
// Include definition for class CCrew from Crew.h
#include "Crew.h"
// Include definition for class CRescType from CRescType.h
#include "CRescType.h"
// Include definition for class CDispLoc from DispLoc.h
#include "DLDispatcher.h"
#include "DispLoc.h"
#include "GACCDispatcher.h"


// Constructor for Type II IA Crew
CTypeTwoIACrew::CTypeTwoIACrew( string crewID, CResource* resource, CDLDispatcher* DLdispatcher ) 
	/*: m_ResourcePtr( resource ), m_VIACrews(), m_DLDispatcherPtr( DLdispatcher ) */
{	
	m_ResourcePtr = resource;
	m_DLDispatcherPtr = DLdispatcher;
	m_CrewID = crewID;
}	

// Copy constructor for Type II IA Crew
CTypeTwoIACrew::CTypeTwoIACrew( const CTypeTwoIACrew &rhs )/*: m_ResourcePtr( rhs.m_ResourcePtr ), /* m_VIACrews(), m_DLDispatcherPtr( rhs.m_DLDispatcherPtr)*/
{
	m_ResourcePtr = rhs.m_ResourcePtr;
	m_DLDispatcherPtr = rhs.m_DLDispatcherPtr;
	m_CrewID = rhs.m_CrewID;
	m_VIACrews = rhs.m_VIACrews;
}

// Destructor for CCrew
CTypeTwoIACrew::~CTypeTwoIACrew()
{
	// Need to delete the IA crews  do it in RunScenario
	//for ( int i = 0; i < m_VIACrews.size(); i++ )
	//	delete m_VIACrews[i];
}

// Equivalence operator for CTypeTwoIACrew
bool CTypeTwoIACrew::operator==( const CTypeTwoIACrew &rhs )
{
	bool equal = true;
	if ( m_CrewID != rhs.m_CrewID)
		equal = false;
	if ( m_ResourcePtr != rhs.m_ResourcePtr )
		equal = false;
	if ( m_VIACrews != rhs.m_VIACrews )
		equal = false;
	if ( m_DLDispatcherPtr != rhs.m_DLDispatcherPtr )
		equal = false;

	return equal;
}

// Set the Type II IA Crew ID
void CTypeTwoIACrew::SetCrewID( string CrewID )
{	m_CrewID = CrewID;	}

// Get the Type II IA Crew ID
string CTypeTwoIACrew::GetCrewID()
{	return m_CrewID;	}

// Set the pointer to the original Type II IA Crew 
void CTypeTwoIACrew::SetResourcePtr( CResource* resource )
{	m_ResourcePtr = resource;	}

// Get the pointer to the original Type II IA Crew
CResource *CTypeTwoIACrew::GetResourcePtr()
{	return m_ResourcePtr;	}

// Set the IA Crews vector using another vector
void CTypeTwoIACrew::SetIACrewsVector( vector< CResource* > VIACrews )
{	m_VIACrews = VIACrews;	}

// Get the IA Crews vector
vector< CResource* > CTypeTwoIACrew::GetIACrewsVector()
{	return m_VIACrews;	}

// Get a IA Crew by index to the vector
CResource* CTypeTwoIACrew::GetIACrewIdx( int idx )
{	return m_VIACrews[idx];	}

// Set the dispatch location dispatcher where the IA crews are located
void CTypeTwoIACrew::SetDLDispatcher( CDLDispatcher* DLdispatcher )
{	m_DLDispatcherPtr = DLdispatcher;	}

// Get the dispatch location dispatcher where the IA crews are located
CDLDispatcher *CTypeTwoIACrew::GetDLDispatcher()
{	return m_DLDispatcherPtr;	}

// Create 4 - 5 person IA crews from a Type II IA Crew and put pointers to these crews in the memory variable vector
void CTypeTwoIACrew::CreateCrews()
{
	// Get the information for the Type II IA Crew
	string RescID = m_ResourcePtr->GetRescID( );
	CRescType &RescType = m_ResourcePtr->GetRescType();
	int Staffing = m_ResourcePtr->GetStaffing();
	string StartTime = m_ResourcePtr->GetStartTime();
	string EndTime = m_ResourcePtr->GetEndTime();
	string StartDay = m_ResourcePtr->GetStartDayofWeek();
	string EndDay = m_ResourcePtr->GetEndDayofWeek();
	int StartSeason = m_ResourcePtr->GetStartSeason();
	int EndSeason = m_ResourcePtr->GetEndSeason();
	CDispLoc &DispLoc = m_ResourcePtr->GetDispLoc();
	int PctAvailable = m_ResourcePtr->GetPctAvail();
	double DailyCost = m_ResourcePtr->GetDailyCost();
	double HourlyCost = m_ResourcePtr->GetHourlyCost();
	

	for ( int i = 0; i < 4; i++ )	{
		
		// Create the IA Crews and store in the Type II IA Crew's IACrew vector
		m_VIACrews.push_back( new CCrew( RescID, RescType, Staffing, StartTime, EndTime, StartDay, EndDay, StartSeason, EndSeason, 
			 DispLoc, PctAvailable, DailyCost, HourlyCost ));
	}
}

// Determine the time when the entire Type II IA Crew is next avialable in minutes since the beginning of the year
int CTypeTwoIACrew::NextAvailableTime()
{
	// Iterate through the 4 - 5 person IA Crews to determine the latest time available
	int LatestTime = -1;
	for (int i = 0; i < 4; i++ )	{
		int CrewTime = m_VIACrews[i]->GetAvailableTime();
		if ( LatestTime < CrewTime )
			LatestTime = CrewTime;
	}

	return LatestTime;
}

// Set the next available time for the crew to a time
bool CTypeTwoIACrew::SetNextAvailableTime( int Time )
{
	// Determine the latest time when the IA Crews were available
	int LatestTime = NextAvailableTime();

	if ( LatestTime <= Time )	{
		// Reset the Available time for all the IA Crews
		for ( int i = 0; i < 4; i++ )
			m_VIACrews[i]->SetAvailableTime( Time );
	
		return true;
	}

	else
		return false;

}

bool CTypeTwoIACrew::AddtoDrawDownVector( int Time, int FireNum )
{
	CResource* ResourcePtr = GetResourcePtr();

	ResourcePtr->RecordDrawDown( Time, FireNum );

	return true;

}

int CTypeTwoIACrew::SumRescWorkYear()
{
	int sum = 0;
	for (int i = 0; i < m_VIACrews.size(); i++)	{
		int CrewSum = m_VIACrews[i]->SumRescWorkYear();
		if (CrewSum > sum)
			sum = CrewSum;
	}

	return sum;


}