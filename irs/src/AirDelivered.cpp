// AirDelivered.cpp
// Function definitions for class CAirDelivered derived from CResource
// for IRS 1/12

#include <iostream>												//contains functions for input and output
using namespace std;
#include <string>												//contains functions for operations with strings
using std::string;
#include <vector>
#include <cstring>												//to convert a string to a const char*
#include <cstdlib>												//to use atof to convert const char* to int and random numbers
//#include <cstdlib>											//contains function prototype for rand

// Include definition for class CResource from Rescbase.h
#include "Rescbase.h"
// Include definition for class CRescType from CRescType.h
#include "CRescType.h"
// Include definition for class CDispLoc from DispLoc.h
#include "DispLoc.h"
// Include definition for class CFire from Fire.h
#include "Fire.h"
// Include definition for class CFWA from FWA.h
#include "FWA.h"
// Include definition for class CProdRates from ProdRates.h
#include "ProdRates.h"
// Include definition class for AirDelivered resources
#include "AirDelivered.h"

// Constructor for Aerially delivered crews
CAirDelivered::CAirDelivered( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly ) 
		: CResource( rescID, resctype, staffing, starttime, endtime, startdayofweek, enddayofweek, startseason, endseason, dispatchloc, 
		percentavail, daily, hourly ) 
{
	SetCarrierIndex( -1);											// Set number for resource index for deploying carrier aircraft to -1
}

// Copy Constructor for CAirDelivered
CAirDelivered::CAirDelivered( const CAirDelivered &airdelivered ) : CResource( airdelivered )
{
	m_CarrierIndex = airdelivered.m_CarrierIndex;

}

// Destructor for Aerially delivered crews
CAirDelivered::~CAirDelivered(){}

// Set the index for the smokejumper aircraft
void CAirDelivered::SetCarrierIndex( int carrierindex )
{	m_CarrierIndex = carrierindex;	}

// Get the index for the carrier aircraft
int CAirDelivered::GetCarrierIndex()
{	return m_CarrierIndex;	}

// Calculate the initial arrival time for the resource (does not include first responder delay)
int CAirDelivered::CalcInitArrival( CFire fire, double distance )
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
	//double distance = disploc.GetFWAAssoc( index );
	double traveltime = 0.0;
	int avgspeed = resctype.GetAvgSpeed();						//get the average speed for the resource
	traveltime = (distance * 60) / avgspeed;					//calculate the travel time delay
	int travelint = static_cast< int > (traveltime + 0.5 );
	
	initarrivaldelay = initarrivaldelay + travelint;			//add to initial delay
	//cout << " Travel Time: " << traveltime;

	// Add Callback delay if appropriate ( discovery time not in workshift, resource is not already on a fire on the julian date
	bool InWorkshift = IsInWorkshift( fire );					//determine if the discovery time for the fire is within the work week and shift for the resource
	int firestart = fire.FireStartTime();						//fire start time in minutes since midnight
	int availabletime = GetAvailableTime() - (24 * 60 * (fire.GetJulianDay() -1 ));						
	int callbackdelay = disploc.GetCallbackDelay();
	int diff = availabletime - firestart;
	if ( !InWorkshift )		{
		if ( GetWorkshiftStartTime() < 0 || diff < 0 )	{
			initarrivaldelay = initarrivaldelay + callbackdelay;			//if not in work shift add callback delay
			//cout << " Callback Delay: " << callbackdelay;
		}
	}

	// Add Walkin delay - Aerially delivered crews always use the walk in delay
	int i = DetermineDelayType();							//determine the delay type index for the resource type and fire FWA
	initarrivaldelay = initarrivaldelay + fwa.GetWalkInDelay( i );				//add in walkin delay if random number is greater than percent fires walkin for FWA
	//cout << "Fire is Walk in - delay: " << fwa.GetWalkInDelay( i ) << "\n";
	
	// Add Competition delay if appropriate
	// Convert discovery time and resource's next available time to minutes since midnight
	int available = GetAvailableTime();								//get the time the resource is next avialable time (minutes since beginning of year)
	if ( available > 0 )	{										//resource is the first fire of the year
		int julian = fire.GetJulianDay();							//get the julian day of the fire

		double availminmidnight = available - (julian-1) * 1440;	//minutes since midnight for resource's next available time

		double firestarttime = fire.FireStartTime();				// fire start time in minutes since midnight

		double difference = availminmidnight - firestarttime;		//calculate the difference between the discovery time and available time (competition delay)
		if ( difference > 0 )
			initarrivaldelay = static_cast<int>(difference) + initarrivaldelay;		//add the competition delay if the difference is greater than 0
		//cout << "minutes since midnight for resource: " << availminmidnight << "\n";
		//cout << " Competition Delay: " << difference << "\n";
	} 

	SetInitArrivalTime( initarrivaldelay );							//save the initial arrival time to the fire
	return initarrivaldelay;
}

// Determine if the resource is available
bool CAirDelivered::IsRescAvailable( CFire fire)
{
	bool isavailable = true;
		
	// Is the julian day for the fire within the resource's season
	int fireday = fire.GetJulianDay();
	if ( GetStartSeason() <= GetEndSeason() )	{
		if ( fireday <  GetStartSeason() || fireday >  GetEndSeason() )
			isavailable = false;
	}
	else	{
		if ( fireday <  GetStartSeason() && fireday >  GetEndSeason() )
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

	return isavailable;
}


// Get the fireline production rate for the constant producer
double CAirDelivered::DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM ) 
{		
	// Define all the values that need to be sent to the Compare(...) function for CProdRates
	string RescType = CResource::GetRescType().GetRescType();
	
	//cout << "Resource Type: " << RescType << "\n";
		
	int Staffing = CResource::GetStaffing();						// Needs to equal 1
	int FuelModel = fire.GetFuelModel();
	string SpecificCondition = fire.GetSpecificCondition();

	//cout << "Staffing: " << Staffing << "\n";
	//cout << "Fuel Model: " << FuelModel << "\n";
	//cout << "Specific Condition: " << SpecificCondition << "\n";
	
	double ProdRate = -1.0;
	// Get the fireline production rate for the single crew producer
	//cout << "Number of entries in VPRodRates vector: " << VProdRates.size() << "\n";
	int i = 0;
	while ( ProdRate < 0 )	{
		ProdRate = VProdRates[i].Compare( "CREW", 0, 1, FuelModel, SpecificCondition );
		i++;
		if ( i >= static_cast< int >( VProdRates.size() ) )
			cout << "There is no entry in the Production Rate table for this combination \n";
	}
	
	//cout << "Fireline Production Rate for single producer: " << ProdRate << "\n";

	ProdRate = ProdRate * Staffing;
	
	//cout << "Fireline Production Rate for Crew: " << ProdRate << "\n";

	SetProdRate( ProdRate );
	return ProdRate;
}

// Calculate the contain values for the deployed resources
int CAirDelivered::CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates >  &VProdRate, std::list< CContainValues > &LContainValues, double Multiplier )
{
	// Adjust the initial arrival time if it is less than the first arrival time that includes first resource delay
	if ( GetInitArrivalTime() < firstarrival )
		SetInitArrivalTime( firstarrival );

	int segstart = GetInitArrivalTime();
	double prodrate = DetermineProdRate( fire, VProdRate, GetInitArrivalTime() ) * Multiplier;
	int duration = CalcRemainingWorkshift( fire );
	string description = GetRescID();
	double daily = GetDailyCost();
	double hourly = GetHourlyCost();

	// Determine placement for new set of contain values in list VContainValues
	std::list< CContainValues >::iterator Iterator;

	if ( LContainValues.empty() )	
		LContainValues.push_back( CContainValues( segstart, prodrate, duration, description, daily, hourly));
	else	{
		// Find the placement for the new contain values object
		Iterator = LContainValues.begin();
		bool isbetter = false;
		while ( !isbetter && Iterator != LContainValues.end() )	{
			if ( segstart > (Iterator)->GetStartTime()  )	
				Iterator++;
			else
				isbetter = true;
		}

		// Insert the new CContainValues object into the proper position in the list
		LContainValues.insert( Iterator, CContainValues( segstart, prodrate, duration, description, daily, hourly ) );

	}

	return 1;

}

// Determine if two resources are equal as good to deploy based on the initial arrival time, Dispatch Location, and duration of time they can be on the fire
bool CAirDelivered::DeploySelectEqual( CResource *resc2 )
{	bool equal = false;
	// Are the initial arrival times, durations and production rates the same for the resources?
	if ( this->GetInitArrivalTime() == resc2->GetInitArrivalTime() && this->GetDuration() == resc2->GetDuration() && 
		this->GetDispLoc().GetDispLocID() == resc2->GetDispLoc().GetDispLocID() )
		equal = true;

	return equal;
}

// Determine if it is better to deploy the first resource rather than the second resource based on initial arrival time, dispatch location, and duration
bool CAirDelivered::DeploySelectCompare( CResource *resc2 )
{	bool thisbetter = false;
	// Is the initial arrival time for the first resource shorter than the second resource?
	if ( this->GetInitArrivalTime() < resc2->GetInitArrivalTime() )
		thisbetter = true;
	else	{
		if ( this->GetInitArrivalTime() == resc2->GetInitArrivalTime() )	{	
			if ( this->GetDispLoc().GetDispLocID() == resc2->GetDispLoc().GetDispLocID() )	{
				if ( this->GetDuration() < resc2->GetDuration() )	
					thisbetter = true;	
			}
		}
	}
	
	return thisbetter;
}

// Reset the Workshift start time for the resources deployed to the fire
int CAirDelivered::AdjustWorkshiftStartTime( CFire fire )
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

// Calculate the remaining workshift for aerial delivered resources
int CAirDelivered::CalcRemainingWorkshift( CFire fire )
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

	SetDuration( ShiftRemaining );									//save the duration the resource can work the fire
	return ShiftRemaining;
}