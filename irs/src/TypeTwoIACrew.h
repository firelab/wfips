// TypeTwoIACrew.h
// definitions for class CTypeTwoIACrew for 20 person type II Initial Attack crews that can be broken down to 4 5-person handcrews for initiak attack
// for IRS 1/13

#ifndef TYPETWOIACREW_H
#define TYPETWOIACREW_H

#include <string>													//C++ standard string class
using std::string;
#include <vector>


#include "Rescbase.h"
#include "Crew.h"

#include "CRescType.h"
#include "DLDispatcher.h"


class CTypeTwoIACrew {
	
public:
	// Constructor for Type Two IA crews
	CTypeTwoIACrew( string crewID, CResource* resource, CDLDispatcher* DLdispatcher );	
	CTypeTwoIACrew( const CTypeTwoIACrew &rhs );

	virtual ~CTypeTwoIACrew();
	
	bool operator==( const CTypeTwoIACrew &rhs );

	void SetCrewID( string CrewID );
	string GetCrewID();			
	void SetResourcePtr( CResource* resource );
	CResource *GetResourcePtr();
	void SetIACrewsVector( vector< CResource* > VIACrews );
	vector< CResource* > GetIACrewsVector();
	CResource* GetIACrewIdx( int idx );
	void SetDLDispatcher( CDLDispatcher* DLdispatcher );
	CDLDispatcher *GetDLDispatcher();

	void CreateCrews();												// Create 4 - 5 person IA crews from a Type II IA Crew and put pointers to these crews in the memery variable vector
	int NextAvailableTime();										// Determines the time when the entire Type II IA Crew is next available in minutes since the start of the year
	bool SetNextAvailableTime( int Time );							// Set the time when the Type II IA Crew will next be available - Time must be later than the latest next available for the IA Crews or returns false
	bool AddtoDrawDownVector( int Time, int FireNum );
	virtual int SumRescWorkYear();									// Get the resource work year sum for the max of the 4 person crews

private:

	string m_CrewID;												// Id for the crew same as CResource instance m_RescID
	CResource* m_ResourcePtr;										// pointer to the CResource / CCrew instance
	vector< CResource* > m_VIACrews;								// vector of pointers to the 5 person IA crews created from the 20 person Type II IA crew
	CDLDispatcher* m_DLDispatcherPtr;								// reference to the dispatch location dispatcher where the 5 person IA crews are located



};	// definition of class CTypeTwoIACrew

#endif  // TYPETWOIACREW_H