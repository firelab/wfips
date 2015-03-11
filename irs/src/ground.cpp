// ground.cpp
// CGround member function file
//10/11 ofr IRS

#include <iostream>												//contains functions for input and output
using namespace std;
#include <string>												//contains functions for operations with strings
using std::string;
#include <cstring>												//to convert a string to a const char*
#include <cstdlib>												//to use atof to convert const char* to int and random numbers


// Include definition for class CResource from Rescbase.h
#include "Rescbase.h"
// Include definition for class CRescType from CRescType.h
#include "CRescType.h"
// Include definition for class CDispLoc from DispLoc.h
#include "DispLoc.h"
// Include definition for class CGround from ground.h
#include "Ground.h"
// Include definition for class CFire from Fire.h
#include "Fire.h"
// Include definition for class CFWA from FWA.h
#include "FWA.h"


// Constructor for Ground Resources
CGround::CGround( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly ) 
		: CResource( rescID, resctype, staffing, starttime, endtime, startdayofweek, enddayofweek, startseason, endseason, dispatchloc, 
		percentavail, daily, hourly ) {}	

// Copy constructor for CGround
CGround::CGround( const CGround &ground ) : CResource( ground )
{}

// Destructor for CGround
CGround::~CGround() {}

// Assignment Operator
CGround& CGround::operator=(const CGround &rhs)
{
	if (&rhs != this)	
		CResource::operator=(rhs);

	return *this;
}

// Calculate the initial arrival time for the resource (does not include first responder delay)
int CGround::CalcInitArrival( CFire fire,  double distance )
{
	int initarrivaldelay = 0;								//initial arrival delay time
	
	// Get delay due to resource type: dispatch delay, response delay and set-up delay
	CRescType resctype = GetRescType();
	initarrivaldelay = resctype.PreConstructDelay();
	//cout << "Preconstruction Delay: " << initarrivaldelay;

	// Calculate travel time delay
	CFWA &fwa = fire.GetFWA();									//get the FWA for the fire
	CDispLoc disploc = GetDispLoc();							//get the dispatch location for the resource
	// Get the ground distance between the dispatch location and the fire's FWA
	//int index = fwa.GetIndex();
	distance = distance * fwa.GetAirtoGround();
	int avgspeed = resctype.GetAvgSpeed();						//get the average speed for the resource
	double traveltime = (distance * 60) / avgspeed;					//calculate the travel time delay
	int travelint = static_cast< int > (traveltime + 0.5);
	
	initarrivaldelay = initarrivaldelay + travelint;			//add to initial delay
	//cout << " Travel Time: " << traveltime;

	// Add Callback delay if appropriate ( discovery time not in workshift, resource is not already on a fire on the julian date
	bool InWorkshift = IsInWorkshift( fire );					//determine if the discovery time for the fire is within the work week and shift for the resource
	int firestart = fire.FireStartTime();						//fire start time in minutes since midnight
	int availabletime = GetAvailableTime() - (24 * 60 * (fire.GetJulianDay() -1 ));						
	int callbackdelay = disploc.GetCallbackDelay();
	int diff = availabletime - firestart;
	if ( !InWorkshift )		{
		if ( GetWorkshiftStartTime() < 0 || diff < 0 )
			initarrivaldelay = initarrivaldelay + callbackdelay;			//if not in work shift add callback delay
		
		//cout << " Callback Delay: " << disploc.GetCallbackDelay();
	}

	// Add Walkin delay if appropriate
	if ( fire.GetIsWalkIn() )	{
		int i = CResource::DetermineDelayType();				//determine the delay type index for the resource type and fire FWA
		initarrivaldelay = initarrivaldelay + fwa.GetWalkInDelay( i );				//add in walkin delay if random number is greater than percent fires walkin for FWA
		//cout << "Fire is Walk in - delay: " << fwa.GetWalkInDelay( i ) << "\n";
	}
	
	// Add Competition delay if appropriate
	// Convert discovery time and resource's next available time to minutes since midnight
	int available = GetAvailableTime();								//get the time the resource is next avialable time (minutes since beginning of year)
	if ( available > 0 )	{										//resource is the first fire of the year
		int julian = fire.GetJulianDay();							//get the julian day of the fire

		double availminmidnight = available - (julian-1) * 1440;	//minutes since midnight for resource's next available time
		double discminmidnight = fire.FireStartTime();				//get fire's discovery time

		double difference = availminmidnight - discminmidnight;		//calculate the difference between the discovery time and available time (competition delay)
		if ( difference > 0 )
			initarrivaldelay = static_cast<int>(difference) + initarrivaldelay;		//add the competition delay if the difference is greater than 0
		//cout << "minutes since midnight for resource: " << availminmidnight << " discovery time minutes since midnight: " << discminmidnight << " difference: " << difference << "\n";
		//cout << " Competition Delay: " << difference << "\n";
	} 

	SetInitArrivalTime( initarrivaldelay );							//save the initial arrival time to the fire
	return initarrivaldelay;
}

// Determine if the resource is available
bool CGround::IsRescAvailable( CFire fire)
{
	bool isavailable = true;
		
	// Is the julian day for the fire within the resource's season
	int fireday = fire.GetJulianDay();
	if ( CResource::GetStartSeason() <= CResource::GetEndSeason() )	{
		if ( fireday < CResource::GetStartSeason() || fireday > CResource::GetEndSeason() )
			isavailable = false;
	}
	else	{
		if ( fireday < CResource::GetStartSeason() && fireday > CResource::GetEndSeason() )
			isavailable = false;
	}

	//cout << "Fire Discovery Day: " << fireday << " Is fire in resouce's season: " << isavailable << "\n";

	// Is the resource available due to percent availability
	int random = rand() % 101;										//get a random number between 1 and 100
	if ( random > CResource::GetPctAvail() )
		isavailable = false;										//if random draw is greater than the percent availablity the resource is not available
	
	//cout << "Random draw for availablity: " << random << "\n";

	// Determine if the resource is in the dispatch logic
	int type = CResource::DetermineDispType();						//get the dispatch type for the resource to use with dispatch array
	CFWA &fwa = fire.GetFWA();										//get the FWA for the fire
	CDispLogic &displogic = fwa.GetDispLogic();						//get the dispatch logic for the FWA
	string index = displogic.GetIndex();							//get the index type for the dispatch logic

	int displevel = 0;
	if ( index == "BI" )	{										//determine the dispatch level to use for this fire
		int firebi = fire.GetBI();
		int numdisplevels = displogic.GetNumLevels();
		for ( int i = 0; i < numdisplevels - 1; i++ )	{
			if ( firebi > displogic.GetBreakPoint(i) )
				displevel++;
		}
		//cout << "Dispatch Level Index is BI = " << firebi << " Number of Disptach Levels: " << numdisplevels << "\n";
	}

	if ( index == "ROS" )	{
		double fireros = fire.GetROS();
		int numdisplevels = displogic.GetNumLevels();
		for ( int i = 0; i < numdisplevels - 1; i++ )	{
			if ( fireros > static_cast<double>(displogic.GetBreakPoint(i)) )
				displevel ++;
		}
		//cout << "Dispatch Level Index is ROS = " << fireros << " Number of Disptach Levels: " << numdisplevels << "\n";
	}

	if ( displogic.GetRescNum( type, displevel ) == 0 )
		isavailable = false;
	//cout << "Number of resource in disptach logic: " << displogic.GetRescNum( type, displevel ) << "\n";
	//cout << "Resources type: " << type << " Dispatch Level: " << displevel+1 << " Is available? " << isavailable << "\n";

	// If the resource has timed out for the day, m_AvailableFlag = true and resource is not available
	if ( GetAvailableFlag() )
		isavailable = false;										//resource not available until next day due to previous fires
	
	// Is the resource's dispatch location associated with the fire's FWA association
	CDispLoc& disploc = GetDispLoc();
	int FWAindex = fwa.GetIndex();
	bool isassociated = disploc.DeterIsAssociated( &fwa );		//determine if the resource's dispatch location is associated with the fire's fwa
	if ( !isassociated )
		isavailable = false;

	// If fire is walk in and resource is water tender then resource is not deployed
	if ( fire.GetIsWalkIn() && GetRescType().GetRescType() == "WT" )
		isavailable = false;

	return isavailable;
}
