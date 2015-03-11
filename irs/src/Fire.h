// Fire.h
// class definitions for CFire, information about individual fires
//10/11 for IRS

#ifndef FIRE_H
#define FIRE_H

#include <assert.h>
#include <string>												//C++ standard string class
using std::string;

#include "FWA.h"												//definition file for CFWA class
#include "math.h"

class CFire {

public:
#ifdef ENABLE_IRS_TESTING
        CFire(CFWA &fwa):m_FWA(fwa){}
#endif
	CFire( int scenario, int firenum, int julianday, string DayofWeek, string discoverytime, int BI, double ROS, int fuelmodel,
		string spcondition, int slopepct, bool iswalkin, string tactic, double distance, int elevation, double ltowratio,
		int minnumsteps, int maxnumsteps, string sunrise, string sunset, bool waterdrops, bool pumpnroll, CFWA &fwa, double lat,
                double lon);			//constructor
	CFire( const CFire &thatfire );								//copy constructor for CFire
	~CFire();													//destructor

	CFire& operator=( const CFire &fire);						//assignment operator for fire
	bool operator==(const CFire &fire);							//equals operator
	bool operator!=(const CFire &fire);							//notequals operator
	
	// Set and get functions for fire variables
	void SetFWA( CFWA &fwa );									//set the FWA for the fire - problem with = operator in CFWA
	CFWA& GetFWA();												//get the FWA for the fire
	void SetScenario( int scenario );							//set scenario
	int GetScenario();											//get scenario
	void SetFireNumber( int firenum );							//set the fire number
	int GetFireNumber();										//get the fire number
	void SetJulianDay( int julianday );							//set the julian day of the fire discovery
	int GetJulianDay();											//Set the julian day of the fire discovery
	void SetDayofWeek( string dayofweek );						//set the day of week for the fire
	string GetDayofWeek();										//get the day of week for the fire
	void SetDiscoveryTime( string discoverytime );				//set the discovery time for the fire
	string GetDiscoveryTime();									//get the discovery time for the fire
	void SetBI( int bi );										//set the burning index for the fire
	int GetBI();												//get the burning index for the fire
	void SetROS( double ros );									//set the ros for the fire
	double GetROS();											//get the ros for the fire
	void SetFuelModel( int fuelmodel );							//set the fuel model
	int GetFuelModel();											//get the fuel model
	void SetSpecificCondition( string spcondition );			//set the specific condition 
	string GetSpecificCondition();								//get the specific condition
	void SetSlopePct( int slopepct );							//set the percent slope at the fire
	int GetSlopePct();											//get the percent slope at the fire
	void SetIsWalkIn( bool walkin );							//set is fire walk-in?
	bool GetIsWalkIn();											//get is fire walk-in?
	void SetTactic( string tactic );							//set fire tactic
	string GetTactic();											//get fire tactic
	void SetDistance( double distance );							//set distance to build fireline from perimeter
	double GetDistance();											//get distance to build fireline from perimeter
	void SetElevation( int elevation);							//set elevation of fire
	int GetElevation();											//get elevation of fire
	void SetLtoWRatio( double ltowratio );						//set length to width ratio
	double GetLtoWRatio();										//get length to width ratio
	void SetMinNumSteps( int minnumsteps );						//set minimum number of steps to use in contain algorithm
	int GetMinNumSteps();										//get minimum number of steps to use in contain algorithm
	void SetMaxNumSteps( int maxnumsteps );						//set the maximum number of steps to use in the contain algorithm
	int GetMaxNumSteps();										//get the maximum number of steps to use in the contain algorithm
	void SetSunRise( string sunrise );							//set the time of sunrise
	string GetSunRise();										//get the time of sunrise
	void SetSunSet( string sunset );							//set the time of sunset
	string GetSunSet();											//get the time of sunset
	void SetWaterDropsAllowed( bool waterdrops );				//set if water drops are allowed
	bool GetWaterDropsAllowed();								//get if water drops are allowed
	void SetIsPumpNRoll( bool pumpnroll );						//set is the fire is pump and roll
	bool GetIsPumpNRoll();										//get if the fire is pump and roll
	void SetLatitude( double latitude );
	double GetLatitude();
	void SetLongitude( double longitude );
	double GetLongitude();

	static int GetCount();										//get the number of fires instantiated
	void PrintFireInfo();										//print information about fire
	int FireStartTime();										//calculate the start time for the fire in minutes since midnight
	int SunriseMSM();											//calculate the sunrise in minutes since midnight
	int SunsetMSM();											//calculate the sunset in minutes since midnight
	int ConvertTimeMSM( string time );							//convert string time (4 chr) to minutes since midnight

	void SetWTArrivalTime( int arrival );						//set the time for the first water tender arrival to the fire
	int GetWTArrivalTime();										//get the time for the first water tender arrival to the fire
        bool GetSimulateContain() {return m_SimulateContain ? 1 : 0;}
        void SetSimulateContain(bool set) {m_SimulateContain = set;}
        bool GetTreated() {return m_Treated;}
        void SetTreated(bool bTreated) {m_Treated = bTreated;}

	CFire& SetMemVars( const CFire &Fire);						//function to set the membervariables excluding m_FWA for copy constructor and assignment operator
	
	double CalcFBFireSize( int timeMSM );							//calculate the size of a fire at a given time without any containment

        double m_lat;
        double m_lon;

private:
	int m_Scenario;												//fire scenario
	int m_FireNumber;											//fire number
	int m_JulianDay;											//julian day for fire discovery
	string m_DayofWeek;											//day of week for the fire
	string m_DiscoveryTime;										//discovery time for the fire
	int m_BI;													//burning index for the fire
	double m_ROS;												//rate of spread for the fire
	int m_FuelModel;											//fuel model for the fire
	string m_SpecificCondition;									//specific condition for the fuel model
	int m_SlopePct;												//slope percent
	bool m_IsWalkIn;											//is the fire walk-in?
	string m_Tactic;											//is the tactic used on the fire head, tail, or parallel
	double m_Distance;											//distance the fireline is built from the fire's edge in chains
	int m_Elevation;											//elevation at the fire
	double m_LtoWRatio;											//length to width ratio of the fire's growth
	int m_MinNumSteps;											//minimum number of steps used in the contain algorithm
	int m_MaxNumSteps;											//maximum number of steps used in the contain algorithm
	string m_SunRise;											//time of sunrise
	string m_SunSet;											//time of sunset
	bool m_WaterDrops;											//are water drops allowed on the fire
	bool m_IsPumpNRoll;											//Is the fire a pump and roll fire?
	CFWA &m_FWA;												//FWA the fire is located in

	int m_WTArrival;												//Time the first water tender arrives at the fire
        bool m_SimulateContain;
        bool m_Treated;
	
	static int count;											//number of objects instantiated

};	//end class CFire

#endif	// FIRE_H
