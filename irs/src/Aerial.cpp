// Aerial.cpp
// Function definitions for the class of aerial resources including resources delivered by aerial resources
// for IRS 1/12

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
// Include definition for class CAerialfrom Aerial.h
#include "Aerial.h"
// Include definition for class CFire from Fire.h
#include "Fire.h"
// Include definition for class CFWA from FWA.h
#include "FWA.h"


// Constructor for Ground Resources
CAerial::CAerial( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly, int volume ) 
		: CResource( rescID, resctype, staffing, starttime, endtime, startdayofweek, enddayofweek, startseason, endseason, dispatchloc, 
		percentavail, daily, hourly ) 
{
	m_Volume = volume;
	m_NextLoadArrival = -1;
	m_CrewDispLoc = "";
}	

// Copy Constructor for CAerial
CAerial::CAerial( const CAerial &aerial ) : CResource( aerial )
{
	m_Volume = aerial.m_Volume;
	m_NextLoadArrival = aerial.m_NextLoadArrival;
	m_CrewDispLoc = aerial.m_CrewDispLoc;

}

// Destructor for CGround
CAerial::~CAerial() {}

// Set drop volume
void CAerial::SetVolume( int volume )
{	m_Volume = volume;	}

// Get drop volume
int CAerial::GetVolume()
{	return m_Volume;	}

// Set next load arrivial time for crew delivery
void CAerial::SetNextLoadArrival ( int nextloadarrival )
{	m_NextLoadArrival = nextloadarrival;
	//cout << "Set next load arrival: " << nextloadarrival << "\n";
}

// Get next load arrival time for aircraft delivering crews
int CAerial::GetNextLoadArrival()
{	return m_NextLoadArrival;	}

// Set Dispatch Location where aircraft is delivering crews from
void CAerial::SetCrewDispLoc( string disploc )
{	m_CrewDispLoc = disploc;	}

// Get Dispatch Location where aircraft is delivering crews from
string CAerial::GetCrewDispLoc()
{	return m_CrewDispLoc;	}

// Calculate the initial arrival time for the resource (does not include first responder delay)
int CAerial::CalcInitArrival( CFire fire, double distance )
{
	int initarrivaldelay = 0;											//initial arrival delay time
	
	// Get delay due to resource type: dispatch delay, response delay and set-up delay
	CRescType resctype = GetRescType();
	initarrivaldelay = resctype.PreConstructDelay();
	//cout << "Preconstruction Delay: " << initarrivaldelay << "\n";

	// Calculate travel time delay
	CFWA &fwa = fire.GetFWA();											//get the FWA for the fire
	CDispLoc disploc = GetDispLoc();									//get the dispatch location for the resource
	// Get the air distance between the dispatch location and the fire's FWA
	//int index = fwa.GetIndex();
	//double distance = disploc.GetFWAAssoc( index );
	double traveltime = 0.0;
	int avgspeed = resctype.GetAvgSpeed();								//get the average speed for the resource
	traveltime = (distance * 60) / avgspeed;							//calculate the travel time delay
	int travelint = static_cast< int > ( traveltime + 0.5 );
	
	initarrivaldelay = initarrivaldelay + travelint;					//add to initial delay
	//cout << " Travel Time: " << traveltime << "\n";

	// The next part assumes that the aviation resources do not begin preparing to fly until sunrise
	// Avaition resources can't start until sunrise
	int sunrise = fire.SunriseMSM();									//get the sunrise time in minutes since midnight
	int firestart = fire.FireStartTime();								//get the fire start time
	int diffsunrise = sunrise - firestart;								//calculate the difference due to sunrise
	int difference = 0;
	if ( diffsunrise > 0 )
		difference = diffsunrise;										//difference will hold the greater of the difference due to sunrise, the callback delay, and the difference due to competition delay

	// Add Callback delay if appropriate ( discovery time not in workshift, resource is not already on a fire on the julian date )
	int callback = 0;
	if ( !IsInWorkshift( fire ) )	
		callback = disploc.GetCallbackDelay();
	
	// If the callback delay is different than the difference due to sunrise use the callback delay
	if ( callback > difference )
		difference = callback;

	// Determine the difference due to competition delay
	int available = GetAvailableTime();									//get the time the resource is next avialable time (minutes since beginning of year)
	int julian = fire.GetJulianDay();									//get the julian day of the fire
	int availminmidnight = available - (julian-1) * 1440;				//minutes since midnight for resource's next available time
	int diffcompetition = availminmidnight - firestart;					//calculate the difference between the discovery time and available time (competition delay)
	if ( diffcompetition > difference )	
		difference = diffcompetition;									//difference due to competition is greater
	
	initarrivaldelay = initarrivaldelay + difference;					//Add the difference to the initial arrival delay
	
	SetInitArrivalTime( initarrivaldelay );								//save the initial arrival time to the fire
	return initarrivaldelay;
}

// Determine if the resource is available
bool CAerial::IsRescAvailable( CFire fire )
{
	bool isavailable = true;
		
	// Is the julian day for the fire within the resource's season
	int fireday = fire.GetJulianDay();
	if ( GetStartSeason() <= GetEndSeason() )	{
		if ( fireday < GetStartSeason() || fireday > GetEndSeason() )
			isavailable = false;
	}
	else	{
		if ( fireday < GetStartSeason() && fireday > GetEndSeason() )
			isavailable = false;
	}

	//cout << "Fire Discovery Day: " << fireday << " Is fire in resouce's season: " << isavailable << "\n";

	// Is the resource available due to percent availability
	int random = rand() % 101;										//get a random number between 1 and 100
	if ( random > GetPctAvail() )
		isavailable = false;										//if random draw is greater than the percent availablity the resource is not available
	
	//cout << "Random draw for availablity: " << random << "\n";

	// Determine if the resource is in the dispatch logic
	int type = DetermineDispType();									//get the dispatch type for the resource to use with dispatch array
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

	// Aerial resources can only work where drops are allowed
	if ( !fire.GetWaterDropsAllowed() )	
		isavailable = false;
		

	return isavailable;
}


// Calculate the remaining workshift for aerial resources
int CAerial::CalcRemainingWorkshift( CFire fire )
{
	// Calculate the time remaining in the workshift
	int FireStartTime = fire.FireStartTime();
	int SunriseStart = fire.SunriseMSM();
	// If the fire is not in the workshift subtract the callback delay
	if ( !IsInWorkshift( fire ) )
		SunriseStart = SunriseStart - GetDispLoc().GetCallbackDelay();
	int Julian = fire.GetJulianDay();
	int PrevFireTime = 0;
	int PrevRestTime = 0;

	int ShiftUsed = 0;
	if ( GetWorkshiftStartTime() >= 0 && !WRGuidelinesMet( FireStartTime, Julian, PrevFireTime, PrevRestTime) )		
		ShiftUsed = (FireStartTime + GetInitArrivalTime()) - GetWorkshiftStartTime();

	else	{	
		ShiftUsed = GetInitArrivalTime();

		// If the fire starts before sunrise, the shift for aerial resources starts at sunrise, so use sunrise instead of the fire start time
		if ( FireStartTime < SunriseStart - GetDispLoc().GetCallbackDelay() )
			ShiftUsed = GetInitArrivalTime() - SunriseStart + FireStartTime;
	}

	//cout << "Workshift Start time: " << GetWorkshiftStartTime() << "\n";
	//cout << "Shift used: " << ShiftUsed << "\n";
	
	int ShiftRemaining = GetWorkshiftLength() - ShiftUsed;

	// Determine when the workshift will end - minutes since midnight
	int ShiftEndMSM = FireStartTime + GetInitArrivalTime() + ShiftRemaining;
	//cout << "Shift end in Minutes since midnight: " << ShiftEndMSM << "\n";

	// Aviation resources can not make drops after sunset
	int Sunset = fire.SunsetMSM();
	//cout << "Sunset in Minutes since midnight: " << Sunset << "\n";

	if ( ShiftEndMSM > Sunset )
		ShiftRemaining = ShiftRemaining - ( ShiftEndMSM - Sunset ) - 1;
		//cout << "Shift remaining: " << ShiftRemaining << "\n";


	SetDuration( ShiftRemaining );									//save the duration the resource can work the fire
	return ShiftRemaining;
}

// Calculate the initial arrival time for the resource (does not include first responder delay)
int CAerial::CalcInitArrival( CFire fire, double adddistance, double distance )
{
	int initarrivaldelay = 0;											//initial arrival delay time
	
	// Get delay due to resource type: dispatch delay, response delay and set-up delay
	CRescType resctype = GetRescType();
	initarrivaldelay = resctype.PreConstructDelay() - resctype.GetSetupDelay();
	//cout << "Preconstruction Delay: " << initarrivaldelay << "\n";

	// Calculate travel time delay
	CFWA &fwa = fire.GetFWA();											//get the FWA for the fire
	CDispLoc disploc = GetDispLoc();									//get the dispatch location for the resource
	// Get the air distance between the dispatch location and the fire's FWA
	//int index = fwa.GetIndex();
	//double distance = disploc.GetFWAAssoc( index );
	// Add in distance to get smokejumpers
	distance = distance + adddistance;		
	double traveltime = 0.0;
	int avgspeed = resctype.GetAvgSpeed();								//get the average speed for the resource
	traveltime = (distance * 60) / avgspeed;							//calculate the travel time delay
	int travelint = static_cast< int > ( traveltime + 0.5 );
	
	initarrivaldelay = initarrivaldelay + travelint;					//add to initial delay
	//cout << " Travel Time: " << traveltime << "\n";

	// The next part assumes that the aviation resources do not begin preparing to fly until sunrise
	// Avaition resources can't start until sunrise
	int sunrise = fire.SunriseMSM();									//get the sunrise time in minutes since midnight
	int firestart = fire.FireStartTime();								//get the fire start time
	int diffsunrise = sunrise - firestart;								//calculate the difference due to sunrise
	int difference = 0;
	if ( diffsunrise > 0 )
		difference = diffsunrise;										//difference will hold the greater of the difference due to sunrise, the callback delay, and the difference due to competition delay

	// Add Callback delay if appropriate ( discovery time not in workshift, resource is not already on a fire on the julian date )
	int callback = 0;
	if ( !IsInWorkshift( fire ) )	
		callback = disploc.GetCallbackDelay();
	
	// If the callback delay is different than the difference due to sunrise use the callback delay
	if ( callback > difference && GetWorkshiftStartTime() < 0 )
		difference = callback;

	// Determine the difference due to competition delay
	int available = GetAvailableTime();									//get the time the resource is next avialable time (minutes since beginning of year)
	int julian = fire.GetJulianDay();									//get the julian day of the fire
	int availminmidnight = available - (julian-1) * 1440;				//minutes since midnight for resource's next available time
	int diffcompetition = availminmidnight - firestart;					//calculate the difference between the discovery time and available time (competition delay)
	if ( diffcompetition > difference )	
		difference = diffcompetition;									//difference due to competition is greater
	
	initarrivaldelay = initarrivaldelay + difference;					//Add the difference to the initial arrival delay
		
	// SetInitArrivalTime( initarrivaldelay );								//save the initial arrival time to the fire
	return initarrivaldelay;
}

// Reset the Workshift start time for the resources deployed to the fire
int CAerial::AdjustWorkshiftStartTime( CFire fire )
{
	int FireStartTime = fire.FireStartTime();
	int Sunrise = fire.SunriseMSM();
	// If the resource is not in it's workshift adjust the workshift start time to account for the callback delay
	if ( !IsInWorkshift( fire ) )
		Sunrise = Sunrise - GetDispLoc().GetCallbackDelay();
	int Julian = fire.GetJulianDay();
	int wsstarttime = GetWorkshiftStartTime();							// calculated work shift start time - default needs to be changed
	// If the workshift hasn't started for the fire
	//cout << "Workshift Start Time: " << wsstarttime << "\n";
	if ( wsstarttime < 0 )	{
		wsstarttime = FireStartTime;

		// If the fire starts before sunrise make the workshift start time sunrise for aerial resources
		if ( wsstarttime < Sunrise )
			wsstarttime = Sunrise;

		// Is there a competition delay from the last day
		int AvailableTimeDay = GetAvailableTime() - ((Julian-1) * 24 * 60);		// Minutes left since the start of the day
		if ( AvailableTimeDay > wsstarttime )
			wsstarttime = AvailableTimeDay;
	}

	else	{				// The workshift has started so need to determine if the work/rest criteria was met before this fire started
		// Check if the work rest guidelines have been met since the last deployment for the day
		int prevfiretime = 0;
		int prevresttime = 0;
		bool metWR = WRGuidelinesMet( FireStartTime, Julian, prevfiretime, prevresttime );
		if ( metWR )	{
			wsstarttime = FireStartTime;

			// Is there a callback delay?  if so exclude this from the initial arrival time ?????????????????
			//if ( !IsInWorkshift( fire ) )	
				//wsstarttime = wsstarttime + GetDispLoc().GetCallbackDelay();
		}
	}
	
	SetWorkshiftStartTime( wsstarttime );
	return wsstarttime;
}