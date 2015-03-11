// GACCDispatcher.cpp
// Member function definitions for class CGACCDispatcher Geographic Area dispatcher from CDispatchBase
// created 1/13 for IRS

#include <iostream>													//contains functions for input and output
using namespace std;
#include <string>													//contains functions for operations with strings
using std::string;
#include <cstring>													//to convert a string to a const char*
#include <cstdlib>													//to use atof to convert const char* to int
#include <vector>
#include <list>
#include <map>

// Include definition for class CNatDispatcher from NatDispatcher.h
#include "GACCDispatcher.h"
// Include definition for class CDispatchBase from DispatchBase.h
#include "DispatchBase.h"
// Include definition for class CTypeTwoIACrew from TyepTwoIACrew.h
#include "TypeTwoIACrew.h"

#include "CRescType.h"
#include "DispLoc.h"


// Default Constructor for CGACCDispatcher
CGACCDispatcher::CGACCDispatcher() : CDispatchBase(), m_VTypeTwoCrews()
{
}

// Constructor for CGACCDispatcher
CGACCDispatcher::CGACCDispatcher( string dispatcherID ) : CDispatchBase( dispatcherID ), m_VTypeTwoCrews() 
{
}

// Copy constructor for CGACCDispatcher
CGACCDispatcher::CGACCDispatcher( const CGACCDispatcher &gaccdispatcher ) : CDispatchBase ( gaccdispatcher )
{
    SetTypeTwoCrewVector(gaccdispatcher.m_VTypeTwoCrews);
}

// Destructor for CGACCDispatcher
CGACCDispatcher::~CGACCDispatcher()
{}

// Add a Type II Crew that doesn't exist to the GACC Dispatcher - This includes placing the IA Crews at a dispatch location
void CGACCDispatcher::AddNewTypeTwoCrew( CResource* Resource, CDLDispatcher *DLdispatcher )
{	// Create a new Type II IA crew at this dispatch location
	string RescID = Resource->GetRescID();
	
	// Add the type II Crew to this GACC
	m_VTypeTwoCrews.push_back( CTypeTwoIACrew( RescID, Resource, DLdispatcher ) );

	int Crew = m_VTypeTwoCrews.size() - 1;

	// Create the crews and get a vector of the newly created crews
	m_VTypeTwoCrews[Crew].CreateCrews();

	vector< CResource* > IACrew = m_VTypeTwoCrews[Crew].GetIACrewsVector();

	// Put the 4 -5 person IA crews at the dispatch location
	for ( int i = 0; i < 4; i++ )	
		DLdispatcher->InsertInRescMap( IACrew[i] );

}

// Add a Type II Crew that already exists to the GACC Dispatcher - This includes placing the IA Crews at a dispatch location
// and deleting the Type II Crew from the old GACCDispatcher
void CGACCDispatcher::MoveTypeTwoCrew( CTypeTwoIACrew &typetwocrew, CDLDispatcher *DLdispatcher, CGACCDispatcher *oldGACCdispatcher )
{	// Create a new Type II IA crew at this dispatch location
	string RescID = typetwocrew.GetCrewID();
	CResource* RescPtr = typetwocrew.GetResourcePtr();
	CDLDispatcher* DLDispPtr = typetwocrew.GetDLDispatcher();
	vector< CResource* > IACrew = typetwocrew.GetIACrewsVector();

	// Iterate through the vector of Type II IA Crews and determine if the crew already exists at the new GACC
	bool Found = false;

	for ( int i = 0; i < m_VTypeTwoCrews.size(); i++ )	{

		CTypeTwoIACrew CompCrew(m_VTypeTwoCrews[i]);

		if ( CompCrew == typetwocrew )	{
			Found = true;

			// Change the dispatch location pointer for the crew
			m_VTypeTwoCrews[i].SetDLDispatcher( DLdispatcher );

		}

	}

	if ( !Found )	{				// Add the type II Crew to this GACC

		m_VTypeTwoCrews.push_back( CTypeTwoIACrew( typetwocrew ) );

		// Change the dispatch location pointer for the crew
		m_VTypeTwoCrews[ m_VTypeTwoCrews.size() - 1 ].SetDLDispatcher( DLdispatcher );

	}

	//else
		//cout << "The Type II crew was found in the vector for the new GACC";

	// Put the 4 -5 person IA crews at the dispatch location
	for ( int i = 0; i < 4; i++ )	
		DLdispatcher->InsertInRescMap( IACrew[i] );

	// Remove the 4 - 5 person IA crews at the old dispatch location
	for ( int i = 0; i < 4; i++ )
		DLDispPtr->RemoveFromRescMap( IACrew[i] );

	if ( !Found )	{	// The crew was not found in the new GACC so new GACC != oldGACC
	
		// Iterate through the vector of Type II IA Crews and determine if the crew already exists at the old GACC
		bool Foundb = false;
		vector< CTypeTwoIACrew > OldTypeTwoCrews = oldGACCdispatcher->GetTypeTwoCrewVector();
		int i = 0;
	
		// Remove the type two crew from the old GACC dispatcher list
		while ( !Foundb &&  i < OldTypeTwoCrews.size() )	{

			CTypeTwoIACrew CompCrew(OldTypeTwoCrews[i]);

			if ( CompCrew.GetCrewID() == typetwocrew.GetCrewID() )
				Foundb = true;
		
			else
				i++;

		}

		if ( Foundb )	{		// Remove the type II Crew to this GACC

			OldTypeTwoCrews.erase( OldTypeTwoCrews.begin() + i );
			oldGACCdispatcher->SetTypeTwoCrewVector( OldTypeTwoCrews );

		}


		else
			cout << "The Type II crew was not found in the vector for the GACC";

	}
	
}

// Remove a Type Two IA Crew from a GACC - This include removing the IA crews from the dispatch location
bool CGACCDispatcher::RemoveTypeTwoCrew( CTypeTwoIACrew &typetwocrew )
{
	// find the Type II Crew
	int CrewFound = -1;
	for ( int i = 0; i < m_VTypeTwoCrews.size(); i++ )	{
		if ( m_VTypeTwoCrews[i] == typetwocrew )
			CrewFound = i;
	}
	
	if ( CrewFound >= 0 )	{
		// Get a pointer to the dispatch location where the IACrews reside and get a vector with the IACrews
		CDLDispatcher *DLDispPtr = typetwocrew.GetDLDispatcher();
		vector< CResource* > IACrew = typetwocrew.GetIACrewsVector();

		// Remove the 4 - 5 person IA crews at the old dispatch location
		for ( int i = 0; i < 4; i++ )
			DLDispPtr->RemoveFromRescMap( IACrew[i] );

		// Remove the crew from the GACC's vector which also deletes the Type II IA Crew
		m_VTypeTwoCrews.erase( m_VTypeTwoCrews.begin() + CrewFound );

		return true;
	}

	else
		return false;
}

// Set the type II IA Crew vector using a vector
void CGACCDispatcher::SetTypeTwoCrewVector( vector< CTypeTwoIACrew > VTypeTwoCrews )
{	m_VTypeTwoCrews = VTypeTwoCrews;	}
								
// Get a vector of references to Type II IA Crews located at the GACC
vector< CTypeTwoIACrew > CGACCDispatcher::GetTypeTwoCrewVector()
{	return m_VTypeTwoCrews;	}

// Get a reference to a Type II IA Crew by vector index
CTypeTwoIACrew CGACCDispatcher::GetTypeTwoCrew( int i )
{	return m_VTypeTwoCrews[i];	}
	
// Determine the current resource level
vector< int > CGACCDispatcher::DetermineCurRescLevel()
{
	std::vector< int > CurRescLevel;

	return CurRescLevel;

}

// Get the number of Type II IA Crews at the GACC
int CGACCDispatcher::NumberTypeTwoCrews()
{	return m_VTypeTwoCrews.size();	}

// Find the/a Type II IA Crew that has the soonest next available time
int CGACCDispatcher::FirstTypeTwoCrewAvailable()
{
	int TheCrew = -1;
	int SoonestTime = 1000000;

	// Vector to save ties then decide randomly
	vector< int > Crews;

	// Iterate through the crews and determine which one has the soonest next available time
	for ( int i = 0; i < m_VTypeTwoCrews.size(); i++ )	{

		// Determine if this crew can arrive sooner than the best so far
		int CrewTime = m_VTypeTwoCrews[i].NextAvailableTime();
		if ( CrewTime < SoonestTime )	{
			SoonestTime = CrewTime;
			TheCrew = i;
		}

	}

	// Look for any tied crews
	for ( int i = 0; i < m_VTypeTwoCrews.size(); i++ )	{
		int CrewTime = m_VTypeTwoCrews[i].NextAvailableTime();
		if( CrewTime == SoonestTime )	
			Crews.push_back( i );
	}

	// If there is a tie decide randomly
	if ( Crews.size() > 1 )	{
		int index = rand() % Crews.size();
		TheCrew = Crews[index];
	}


	return TheCrew;
}

// The time when the entire Type II IA Crew is available
int CGACCDispatcher::CrewAvailableTime( int index )
{	return m_VTypeTwoCrews[index].NextAvailableTime();	}

// Set The next available time for a Type Two crew by index to a new time - return -1 if the time is befor the crew is next available
bool CGACCDispatcher::SetTypeTwoCrewNextAvailableTime( int Index, int Time )
{
	// Is this a valid crew index?
	if ( Index > m_VTypeTwoCrews.size() - 1 )
		return false;

	else	{		// Set the crew to the appropriate Time - return false if the time is before all the IA crews are available
		bool Results = m_VTypeTwoCrews[Index].SetNextAvailableTime( Time );

		if ( Results )
			return true;
		else
			return false;
	}
}

// Add to the Type Two IA Crews draw down vector
bool CGACCDispatcher::AddtoDrawDownVector( int Index, int Time, int FireNum )
{
	// Is this a valid crew index?
	if ( Index > m_VTypeTwoCrews.size() - 1 )
		return false;

	else	{		// Set the crew to the appropriate Time - return false if the time is before all the IA crews are available
		bool Results = m_VTypeTwoCrews[Index].AddtoDrawDownVector( Time, FireNum );

		if ( Results )
			return true;
		else
			return false;
	}
}

// Determine if a type Two IA Crew by Crew ID exists at the GACC and return the index for it or negative number if not there
int CGACCDispatcher::LocateCrewIndex( string CrewID )
{
	int Index = -1;
	bool found = false;

	for ( int i = 0; i < m_VTypeTwoCrews.size(); i++ )	{

		string ThisCrewID = m_VTypeTwoCrews[i].GetCrewID();

		if ( CrewID == ThisCrewID )	{

			found = true;
			Index = i;

		}

	}

	return Index;

}
