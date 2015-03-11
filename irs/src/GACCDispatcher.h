// GACCDispatcher.h
// Function definitions for class CGACCDispatcher for Geographic Area dispatchers derived from base class CDispatchBase
// for IRS 1/13

#ifndef GACCDispatcher_H
#define GACCDispatcher_H

#include <string>													//C++ standard string class
using std::string;
#include <list>
#include <vector>
using namespace std;

// Include definition for class CDispatchBase from DispatchBase.h
#include "DispatchBase.h"
#include "TypeTwoIACrew.h"
#include "RunScenario.h"

class CGACCDispatcher : public CDispatchBase {

public:
	CGACCDispatcher();												//CGACCDispatcher default constructor
	CGACCDispatcher( string dispatcherID );
	CGACCDispatcher( const CGACCDispatcher &gaccdispatcher);		//copy constructor
	virtual ~CGACCDispatcher();										//destructor

	// function definitions for set and get functions for class member variables
	void AddNewTypeTwoCrew( CResource* Resource, CDLDispatcher *DLdispatcher );		// Add a new Type II IA Crew to a GACC Dispatcher
	void MoveTypeTwoCrew( CTypeTwoIACrew &typetwocrew, CDLDispatcher *DLdispatcher, CGACCDispatcher *oldGACCdispatcher );	//Move a Type II IA Crew that already exists to the GACC and create 4 - 5 person IA crews at the dispatch location dispatcher
	bool RemoveTypeTwoCrew( CTypeTwoIACrew &typetwocrew );			//remove a Type II IA Crew from a GACC
	int FirstTypeTwoCrewAvailable();								//find the Type II IA Crew at a GACC that will be the first Available - return the index to the vector - return a -1 if the vector is empty
	bool SetTypeTwoCrewNextAvailableTime( int Index, int Time );	//set the next available time for a Type II IA Crew by index to a later timef
	int NumberTypeTwoCrews();										//returns the number of Type II IA crews at the GACC
	int CrewAvailableTime( int index );								//returns the time the entire type II IA Crew is available
	bool AddtoDrawDownVector( int Index, int Time, int FireNum );	//add to the type II IA Crews draw down vector
	int LocateCrewIndex( string CrewID );							//determine if a type II IA Crew is located at the GACC and return the index for it.  If it is not there return negative number

	void SetTypeTwoCrewVector( vector< CTypeTwoIACrew > VTypeTwoCrew );		//set the type II Crews using a vector
	vector< CTypeTwoIACrew > GetTypeTwoCrewVector();				//get a vector of the Type II Crews at the GACC
	CTypeTwoIACrew GetTypeTwoCrew( int i );							//get a Type II Crew by index
	
	virtual vector< int > DetermineCurRescLevel();					//abstract - determine the current level by resource type
	
	
private:
	vector<CTypeTwoIACrew> m_VTypeTwoCrews;							// vector of type two crews at the GACC dispatcher 
	CGACCDispatcher& operator=(const CGACCDispatcher &rhs){};		// assignment operator private because CTypeTwoIACrew has pointers to DLDispatcher
		

};		// end of class CGACCDispatcher

#endif   // GACCDISPATCHER_H