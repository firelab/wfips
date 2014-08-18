// Rescbase.h
// Abstract base class for preparedness resources.
// for IRS 10/11

#ifndef RESCBASE_H
#define RESCBASE_H

#include <stdio.h>
#include <string>										//C++ standard string class
using std::string;
#include <list>
#include <vector>
using std::vector;
#include <utility>

// include Resource Type definitions from CRescType.h
#include "CRescType.h"
// include Dispatch Location definitions from DispLoc.h
#include "DispLoc.h"
// include fire definitions from Fire.h
#include "Fire.h"
// include production rate definitions from ProdRates.h
#include "ProdRates.h"
// include definitions for contain values from ContainValues.h
#include "ContainValues.h"
// include definitions for the times the resource is working during the year from RescWorkYear.h
#include "RescWorkYear.h"

class CResource {

public:
	CResource();										//CResource default constructor
	CResource( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly);
	CResource( const CResource &resource );				// Copy Constructor
	virtual ~CResource();								//destructor
		
		
	// function definitions for set and get functions for class member variables
	void SetRescID( string rescID );					//set the resource id
	string GetRescID();									//get the resource id
	void SetRescType( CRescType &resctype );			//set the resource type
	CRescType &GetRescType();							//get the resource type
	int GetDispatchType();								//get the dispatch type 
	int GetDelayType();									//get the delay type
	void SetStaffing( int staffing );					//set the resource staffing
	int GetStaffing();									//get the resource staffing
	void SetStartTime( string starttime );				//set the resource start time
	string GetStartTime();								//get the resource start time
	void SetEndTime( string endtime );					//set the resource end time
	string GetEndTime();								//get the resource end time
	void SetStartDayofWeek( string startdayofweek );	//set the resource start day of week
	string GetStartDayofWeek();							//get the resource start day of week
	void SetEndDayofWeek ( string enddayofweek );		//set the resource end day of week
	string GetEndDayofWeek();							//get the resource end day of week
	void SetStartSeason( int startseason );				//set the resource start season date
	int GetStartSeason();								//get the resource start season date
	void SetEndSeason( int endseason );					//set the resource end season date
	int GetEndSeason();									//get the resource end season date
	void SetDispLoc( CDispLoc &dispatchloc );			//set the resource dispatch location
	CDispLoc &GetDispLoc();								//get the resource dispatch location
	void SetPctAvail( int percentavail );				//set the resource percent availability
	int GetPctAvail();									//get the resource percent availability
	void SetAvailableTime(int avialabletime );			//set the time the resource is next available in minutes since start of year
	int GetAvailableTime();								//get the time the resource is next available in minutes since start of year
	void SetWorkshiftStartTime( int time );				//set the start time for the workshift
	int GetWorkshiftStartTime();						//get the workshift start time
	void SetDailyCost( double dailycost );				//set the daily cost for the resource
	double GetDailyCost();								//get the daily cost for the resource
	void SetHourlyCost( double hourlycost );			//set the hourly cost for the resource
	double GetHourlyCost();								//get the hourly cost for the resource

	void SetWorkshiftLength( int time );				//set workshift length
	bool IsInSeason( int Julian );						//is the resource in season
	int GetWorkshiftLength();							//get workshift length
	void SetInitArrivalTime( int time );				//set the initial arrival time to the fire
	int GetInitArrivalTime();							//get the initial arrival time to the fire
	void SetDuration( int duration );					//set the duration of time the resource is available for the fire
	int GetDuration();									//get the duration of time the resource is available for the fire
	void SetProdRate( double prodrate );				//set the production rate at the fire
	double GetProdRate();								//get the production rate at the fire
	void SetAvailableFlag ( bool flag );				//set the available flag
	bool GetAvailableFlag();							//get the availableflag
	void SetIsBorrowed( bool flag );					//set the value of the is borrowed flag
	bool GetIsBorrowed();								//get the is borrowed flag
	void SetReturnTime( int Time );						//set the time the resource needs to leave the borrowing dispatch location - Minutes since beginning of year
	int GetReturnTime();								//get the time the resource needs to leave the borrowing dispatch location - Minutes since beginning of year

	void RecordMove( int timeMBY, string DispLoc );		//record the movememt of a resource by adding the time since beginning of the year and the dispatch location 
	vector< std::pair< int, string > > GetMovement();	//get the vector of resource movement
	std::pair< int, string > GetMovementPair( int i );	//get a pair from the movement vector for the resource
	string GetCurrentLocation();						//get the current location of the resource from the last entry in m_VMovement
	void RecordDrawDown( int timeMBY, int FireNum );	//record the times when a resource was used for escape fires
	vector< std::pair< int, int > > GetDrawDown();		// get the vector of resource draw down use
	std::pair< int, int > GetDrawDownPair( int i );		// get a pair from the drawdown vector for the resource


	static int GetCount();								//get the number of resources instantiated
	virtual void PrintRescInfo();						//print information about resource
	virtual void PrintRescWorkInfo();					//print the information about a resources work this year

	bool IsInWorkshift( CFire fire );					//is the fire start time in the resource's workshift (callback delay)
	virtual int CalcInitArrival( CFire fire, double distance ) = 0;		//abstract - calculate the initial arrival time for the resource
	virtual bool IsRescAvailable( CFire fire ) = 0;		//abstract - determine if the resource is available
	int DetermineDispType();							//determine the dispatch resource type
	int DetermineDelayType();							//determine the type of resource for FWA delays ( 0-tracked, 1-boat, 2-crew, 3-engine, 4 heli, 5-smjr)
	int ConvertDay( string day );						//convert day of week to number of day in week 0 = sunday - 6 = saturday
	int ConvertTime( string time );						//convert the time of day - four didget string - to hours since midnight
	virtual double DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int TimeMSM ) = 0;		//abstract - get the fireline production rate for the fire for ground resources
	virtual int CalcRemainingWorkshift( CFire fire );	//calculate the time remaining in the workshift to use on current fire after resources arrive
	virtual int CalcRemainingWorkshift( CFire fire, int arrival );	//calculate the time remaining in the workshift to use on current fire after resources arrive using a different arrival time than that saved in the resource's class
	virtual int AdjustWorkshiftStartTime( CFire fire );		// Reset the Workshift start time for the resources deployed to the fire	
	void AdjustAvailableTime();							//adjust the next available time to account for work/rest break because fire is on a new day
	void ResetAvailableTime();							//use when new scenario, reset the available time to -1
	void EndSeasonAvailableTime( int Julian );			//determine if the resource is past the end of season and adjust the available time to beyond the end of the year
	virtual bool DeploySelectEqual( CResource *resc2 );		//determine if two resources are equivalent for deploying by comparing m_InitArrival, m_Duration, m_ProdRate
	virtual bool DeploySelectCompare( CResource *resc2 );	//determine if resource 1 is better to deploy than resource 2 based on m_InitArrival, m_Duration, and m_ProdRate
	virtual int CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates > &VProdRates, std::list< CContainValues > &LContainValues, double Multiplier ) = 0;	//determine the start time, production rate, and duration values for the resource to input into contain algorithm
	virtual int DetermineWorkRest( int scenario, int firenum, int julian, int starttime, int endtime, string effort );		// determine the next available time the resource is available based on the times the resource is working
	int WorkYearSize();									//return the size of the work year vector
	CRescWorkYear GetWorkYearEntry( int i );			//return the ith element in the resources work year vector
	bool WRGuidelinesMet( int firestarttimeMSM, int julian, int &prevfiretime, int &prevresttime );	 //determine if the work rest guideline has been met before this firestarts
	void ClearRescWorkYear();							//clear the resource work year information for the resource
	virtual int SumRescWorkYear();						//get the total number of minutes in the resource work year
        void SetDbId(int nId);
        int GetDbId() {return nDbId;}

private:
	string m_RescID;									//unique id for the resource
	CRescType &m_RescType;								//type of resource needs to match an instance in CRescType
	int m_Staffing;										//staffing for the resource
	string m_StartTime;									//start time for the resource's workday
	string m_EndTime;									//end time for the resource's workday
	string m_StartDayofWeek;								//start day for the resource's workweek
	string m_EndDayofWeek;									//end day for the resource's workweek
	int m_StartSeason;									//julian date for the start of the resource's season
	int m_EndSeason;									//julian date for the end of the resource's season
	CDispLoc &m_DispLoc;								//dispatch location for the resource needs to match an instance in CDispLoc
	int m_PctAvail;										//percent availability for the resource
	double m_DailyCost;									//daily cost for the resource
	double m_HourlyCost;								//hourly cost for the resource

	int m_WorkshiftLength;								//workshift length in minutes
	int m_InitArrival;									//initial arrival time to a fire
	int m_Duration;										//duration of workshift left for resource to work fire
	double m_ProdRate;									//production rate for fire
	int m_AvailableTime;								//time when resource will next be available due to previous fires in minutes since start of year
	bool m_AvailableFlag;								//indicates if the resource has timed out for the day ( 0 - not timed out, 1 - timed out)
	int m_DispatchType;									//type of resource in dispatch logic by number in dispatch logic m_Nums array, derived when instance created
	int m_DelayType;									//type of resource for determining delays from FWA information
	int m_WorkshiftStartTime;							//time in minutes since midnight when the workshift started - not started = -1
	std::vector< CRescWorkYear > m_VRescWorkYear;		//vector containing information about the times during the year when the resource was working
	int m_NumWorkEntries;								//number of objects int the m_VRescWorkYear vector
	bool m_IsBorrowed;									//indicates that the resource is borrowed and needs to be returned 
	int m_ReturnTime;									//indicates the time the resource needs to leave the dispatch location it was borrowed from - Minutes since beginning of year
	
	vector< std::pair< int, string > > m_VMovement;		//vector containing pairs describing the movements of resources pair is time since beginning of the year and dispatch location id
	vector< std::pair< int, int > > m_VDrawDown;		//vector containing pairs describing the times the resource was used on an escape fire, time since beginning of year and fire number
	
	static int count;									//number of objects instantiated
	int nDbId;

};		// end of class CResource

#endif   // RESCBASE_H
