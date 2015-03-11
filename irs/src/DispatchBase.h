// DispatchBase.h
// Abstract base class for dispatch levels for preparidness resources
// for IRS 11/12

#ifndef DISPATCHBASE_H
#define DISPATCHBASE_H

#include <string>											//C++ standard string class
using std::string;
#include <list>
using std::list;
#include <vector>
using std::vector;

// include Disptach Location definitions from CDispLoc.h
#include "DispLoc.h"
#include "CRescType.h"
#include "Escape.h"

// include Dispatch Location definitions from DispLoc.h
//#include "DispLoc.h"
// include fire definitions from Fire.h
//#include "Fire.h"
// include production rate definitions from ProdRates.h
//#include "ProdRates.h"
// include definitions for contain values from ContainValues.h
//#include "ContainValues.h"
// include definitions for the times the resource is working during the year from RescWorkYear.h
//#include "RescWorkYear.h"

class CDispatchBase {
	friend std::ostream &operator<<(std::ostream &out, CDispatchBase *dispatchbase );

public:
	CDispatchBase();										//CDispatchBase default constructor
	CDispatchBase( string dispatcherID );
	CDispatchBase( const CDispatchBase &dispatchbase);		//copy constructor
	virtual ~CDispatchBase();								//destructor

	CDispatchBase& operator=(const CDispatchBase &rhs);		// assignment operator
	//virtual bool operator==( CDispatchBase &dispatchbase );	// equal operator

		
	// function definitions for set and get functions for class member variables
	void SetDispatcherID( string dispatcherID );			//set the resource id
	string GetDispatcherID();								//get the resource id
	void SetBaseRescLevel( vector< int > baseresclevel );	//set the base resource level using a vector
	vector< int > GetBaseRescLevel();						//get the base resource level vector
	void SetBaseRescLevelIdx( int index, int level );		//set the base resource level for an individual index
	int GetBaseRescLevelIdx( int index );					//get the base resource level for an individual index
	void SetMinRescLevel( vector< int > minresclevel );		//set the minimum resource level using a vector
	vector< int > GetMinRescLevel();						//get the minimum resource level vector
	void SetMinRescLevelIdx( int index, int level );		//set the minimum resource level for an individual index
	int GetMinRescLevelIdx( int index );					//get the minimum resource level for an individual index
	void SetCurRescLevel( vector< int > curresclevel );		//set the current resource level using a vector
	vector< int > GetCurRescLevel();						//get the current resource level vector
	void SetCurRescLevelIdx( int index, int level );		//set the current resource level for an individual index
	int GetCurRescLevelIdx( int index );					//get the current resource level for an individual index
	void SetDailyMinRescLevel( vector< int > dailyminlevel );	//set the daily minimum resource level using a vector
	vector< int > GetDailyMinRescLevel();					//get the daily minimum resource level vector
	void SetDailyMinRescLevelIdx( int index, int level );	//set the daily minimum resource level for an individual index
	int GetDailyMinRescLevelIdx( int index );				//get the daily minimum resource level for an individual index
	void SetExpectLevelVector( vector< double > expectlevel );	//set the expected level using a vector
	vector< double > GetExpectLevelVector();				//get the vector of expected levels
	void SetExpectLevel( double expectlevel, int Julian );	//set the expected work load level
	double GetExpectLevel( int Julian );						//get the expected work load level
	void AddtoExpectLevel( int Julian) ;					// Add one to the expected level
	void SetPreviousLevelVector( vector< double > previouslevel );	//set the previous levels using a vector
	vector< double > GetPreviousLevelVector();				//get the previous level vector
	void SetPreviousLevel( double previouslevel, int Julian );	//set the previous work load level
	double GetPreviousLevel( int Julian );					//get the previous work load level
	void SetDailyUsageLevel(vector<double> dailyusagelevel);	//set the daily usage level vector with a vector
	vector<double> GetDailyUsageLevel();					//get the daily usage level vector
	void SetDailyUsage(double dailyusage, int Julian);		//set the daily usage level for a julian day
	double GetDailyUsage(int Julian);						//get the daily usage level for a julian day
	void SetDailyDispatchLevelVector(vector<int> dailydispatchlevel);	//set the daily dispatch level vector with a vector
	vector<int> GetDailyDisaptchLevelVector();				//get the daily dispatch level vector
	void SetDailyDispatchLLevel(int dailydispatchlevel, int Julian);	//set a day of the daily dispatch level
	int GetDailyDispatchLevel(int Julian);					//get the daily dispatch level for a given julian day
	void AddDailyDispatch(int Julian, int numberdispatches = 1);	//add dispatches to the daily dispatch level
	void SetEscapeLevelVector( vector< int > escapelevel ); //set the exscape levels vector
	vector< int > GetEscapeLevelVector();					//get the escape level vector
	void SetEscapeLevel( int escapelevel, int Julian );		//set the escape fire level
	int GetEscapeLevel( int Julian );						//get the escape fire level
	void AddToEscapeLevel( int Julian );					//add one to the escape fire level
	void SetDispLogicUnfilledVector( vector< int > dispLogicunfilled ); //set the dispatch logic unfilled vector
	vector< int > GetDispLogicUnfilledVector();				//get the dispatch logic unfilled vector
	void SetDispLogicUnfilled( int displogicunfilled, int Julian );		//set the dispatch logic unfilled
	int GetDispLogicUnfilled( int Julian );					//get the dispatch logic unfilled
	void AddToDispLogicUnfilled( int Julian );				//add one to the dispatch logic unfilled
	void SetSeasonStart( int Julian );						//set the start date for the fire season
	int GetSeasonStart();									//get the start date for the fire season
	void SetSeasonEnd( int Julian );						//set the end date for the fire season
	int GetSeasonEnd();										//get the end date for the fire season
	bool IsInFireSeason ( int Julian );						//is the Julian date within the fire season for the dispatcher

	virtual void ResetNewScenario( int scenario );			//set values for the new scenario
	virtual void ResetNewDay( int Julian );					//reset the values for a new day - Daily Minimum resource level
	void PrintRescLevel( vector< int > VRescLevel);			//print the current resource level requires a vector or 14 integers that represents resource dispatch types
	virtual vector< int > DetermineCurRescLevel( int timeYear, vector< CDispatchBase* > VDispatchers );		//determine the current level of resources by type and a dispatcher
	virtual void DetermineBaseRescLevel( vector< CDispatchBase* > VDispatchers );	//determine base resource level for a dispatcher.  For internal nodes need to be sure to calculated values for all subnodes first
	void ClearEscapes();									// Clear the temporary list of escape fires for the node
	void AddEscape( CEscape Escape );						// Add an escape fire to the list in the proper order
	virtual bool CalcDailyUsageLevel(int Julian, vector<CDispatchBase*> VDispatchers);					// Calcualte the daily usage for the dispatch location dispatcher

private:
	string m_DispatcherID;									//unique id for the dispatcher
	vector< int > m_BaseRescLevel;							//Base level of resources for the dispatcher by dispatch type plus 20 person crews and large helicopters
	vector< int > m_MinRescLevel;							//Minimum number of resources available for each dispatch type plus 20 person crews and large helicopters
	vector< int > m_CurRescLevel;							//Current number of resources available for each dispatch type plus 20 person crews and large helicopters
	vector< int > m_DailyMinRescLevel;						//The minimum number of resources available by each dispatch type that occured during the day
	vector< double > m_VExpectLevel;						//expected work load level for the dispatcher
	vector< double > m_VPreviousLevel;						//previous work load level for the dispatcher
	vector<double> m_VDailyUsage;							//tracks the daily usage of resources (number of dispatches/number of resources)
	vector<int> m_VDailyDispatches;							//vector with the number of dispatches per julian day
	vector< int > m_VEscapeLevel;							//escape fire level for the dispatcher
	vector< int > m_VDispLogicUnfilled;						//level of fires with unfilled dispatch logic
	int m_SeasonStart;										//Start date for the fire season
	int m_SeasonEnd;										//End date for the fire season
	list< CEscape > m_LTempEscapes;							//Temporary list of the escape fires for the node
		
	static int count;										//number of objects instantiated
	

};		// end of class CDispatchBase

#endif   // DISPATCHBASE_H
