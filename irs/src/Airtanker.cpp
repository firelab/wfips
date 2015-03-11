// Airtanker.cpp
// function definitions for class CAirtanker derived from class CAerial derived from class CResource
// defines functions for large airtankers
// for IRS 1/12

#include <iostream>												//contains functions for input and output
using namespace std;
#include <string>												//contains functions for operations with strings
using std::string;
#include <vector>
#include <list>
#include <cstring>												//to convert a string to a const char*
#include <cstdlib>												//to use atof to convert const char* to int and random numbers
//#include <cstdlib>											//contains function prototype for rand

// Include definition for class CResource from Rescbase.h
#include "Rescbase.h"
// Include definition for class CRescType from CRescType.h
#include "CRescType.h"
// Include definition for class CDispLoc from DispLoc.h
#include "DispLoc.h"
// Include definition for class CAerial from Aerial.h
#include "Aerial.h"
// Include definition for class CAirtanker from Airtanker.h
#include "Airtanker.h"
// Include definition for class CFire from Fire.h
#include "Fire.h"
// Include definition for class CFWA from FWA.h
#include "FWA.h"
// include contain values definitions from ContainValues.h
#include "ContainValues.h"


// Constructor for constant producing ground resource
CAirtanker::CAirtanker( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly, int volume ) 
		: CAerial( rescID, resctype, staffing, starttime, endtime, startdayofweek, enddayofweek, startseason, endseason, dispatchloc, 
		percentavail, daily, hourly, volume ) 
{
	SetWorkshiftLength( 14*60 );											// Set the workshift length for airtankers to 14 hours
}

// Copy Constructor for CAirtanker
CAirtanker::CAirtanker( const CAirtanker &airtanker ) : CAerial( airtanker )
{}

// Destructor for CAirtanker
CAirtanker::~CAirtanker() {}

// Assignment Operator
CAirtanker& CAirtanker::operator=(const CAirtanker &rhs)
{
	if (&rhs != this)	
		CAerial::operator=(rhs);
	
	return *this;
}

// Get the fireline production rate for the constant producer
double CAirtanker::DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM ) 
{		
	// Get fuel model information to determine production factor for drops
	int FuelModel = fire.GetFuelModel();
	string SpecificCondition = fire.GetSpecificCondition();
	double factor = 1;

	switch (FuelModel)	{

	case 103:
	case 104:
	case 105:
	case 106:
	case 121:
	case 122:
	case 123:
	case 141:
	case 142:
	case 146:
	case 161:
	case 181:
	case 182:
	case 183:
	case 186:
	case 188:
	case 189:
		factor = 0.7;
		break;

	case 107:
	case 108:
	case 109:
	case 124:
	case 143:
	case 201:
		factor = 0.6;
		break;

	case 162:
	case 163:
	case 184:
		factor = 0.5;
		break;

	case 164:
		if (SpecificCondition == "Black Spruce" )
			factor = 0.5;
		else 
			factor = 0.3;
		break;

	case 165:
		factor = 0.3;
		break;

	case 185:
	case 187:
	case 205:
	case 145:
	case 147:
	case 148:
	case 149:
		factor = 0.2;
		break;

	default:
		break;

	}		// end switch

	double ProdRate = GetVolume() / 100 * factor;

	// Adjust production rate for fire's rate of spread
	double ROS = fire.GetROS();

	// Adjust fire's rate of spread for fuel model if necessary
	ROS = fire.GetFWA().FuelModelAdjROS( FuelModel, ROS );
	
	// Adjust fire's rate of spread for the diurnal coefficients
	int hour = timeMSM/60;
	hour = hour % 24;
	ROS = ROS * fire.GetFWA().GetDiurnalCoefficient( hour );
	//cout << "TimeMSM: " << timeMSM << " Hour: " << hour << " Rate of Spread: " << ROS << "\n";
	switch (FuelModel)	{

	case 101:
	case 102:
		// Production rate is reduced linearly from ROS=1 to ROS=80 where the value is 0
		if ( ROS > 1 )	{
			if (ROS < 80 )
				ProdRate = ProdRate / 79 * ( 80 - ROS );
			else
				ProdRate = 0;
		}
		break;

	default:
		// Production rate is reduced linearly from ROS=1 to ROS = 40 where the value is 0
		if ( ROS > 1 )	{
			if (ROS < 40 )
				ProdRate = ProdRate / 39 * ( 40 - ROS );
			else
				ProdRate = 0;
		}
		break;
	}

	// Production rate is in chains per drop, need chains per hour to input to the contain algrithm
	ProdRate = ProdRate*60;

	//cout << "Fuel Model: " << FuelModel << "\n";
	//cout << "Specific Condition: " << SpecificCondition << "\n";
	//cout << "Production Rate: " << ProdRate << "\n";

	SetProdRate( ProdRate );
	return ProdRate;
}

// Calculate the contain values for the deployed resources
int CAirtanker::CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates >  &VProdRate, std::list< CContainValues > &LContainValues, double Multiplier )
{
	// Adjust the initial arrival time if it is less than the first arrival time that includes first resource delay
	if ( GetInitArrivalTime() < firstarrival )
		SetInitArrivalTime( firstarrival );

	int segstart = GetInitArrivalTime();
	int timeMSM = GetInitArrivalTime() + fire.FireStartTime();
	double prodrate = DetermineProdRate( fire, VProdRate, timeMSM ) * Multiplier;	// Production rate for the first drop
	int duration = CalcRemainingWorkshift( fire );						// Drops have a duration of 1 minute.  This duration used to stop dropping
	string description = GetRescID();
	double daily = GetDailyCost();
	double hourly = GetHourlyCost();

	// Determine placement for new set of contain values in list VContainValues
	std::list< CContainValues >::iterator Iterator;
	//cout << "Segment Start: " << segstart << " Production Rate: " << prodrate << "\n";
	if ( LContainValues.empty() )	
		LContainValues.push_back( CContainValues( segstart, prodrate, 1, description, daily, hourly ));
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
		LContainValues.insert( Iterator, CContainValues( segstart, prodrate, 1, description, daily, hourly ) );

	}

	// Get reload time
	int reloaddelay = fire.GetFWA().GetReloadDelay(4);					// Get reload delay to nearest base

	// Duration of time left after drop and reload
	duration = duration - reloaddelay - 1;

	// While the duration of the work time is left continue making drops ( workshift of 14 hours or sunset )
	while (duration > 0 )
	{
		// Values for next segment
		segstart = segstart + reloaddelay + 1;
		timeMSM = segstart + fire.FireStartTime();
		prodrate = DetermineProdRate( fire, VProdRate, timeMSM ) * Multiplier;		// Get the production rate for this drop

		//cout << "Segment Start: " << segstart << " Production Rate: " << prodrate << "\n";
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
		LContainValues.insert( Iterator, CContainValues( segstart, prodrate, 1, description, daily, hourly ) );

		// Adjust duration for next drop
		duration = duration - reloaddelay - 1;

	}

	// Determine the end time for the airtanker's activities
	duration = duration + reloaddelay;									// remaining duration after final drop
	int timeworked = GetDuration() - duration;							// time worked once airtanker arrives at the fire
	int endtime = fire.FireStartTime() + GetInitArrivalTime() + timeworked;		// time airtanker finishes last drop
	
	double returntime = (reloaddelay - GetRescType().GetResponseDelay()) / 2;	// oneway travel time to nearest base

	endtime = endtime + static_cast< int >( returntime );

	// Return the time the resource ends work on the fire
	return endtime;

}

// Virtual function to determine if the resource is available, modified for large airtankers because they are not available on their day off
bool CAirtanker::IsRescAvailable( CFire fire )
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

	// Is in work week? Airtankers are not available on their day off
	string firedayofweek = fire.GetDayofWeek();
	int discoveryday = ConvertDay( firedayofweek );
	int startday = ConvertDay( GetStartDayofWeek() );
	int endday = ConvertDay( GetEndDayofWeek() );

	// if the start day of week is less than the end day of week
	if ( startday < endday ) {
		if ( discoveryday < startday || discoveryday > endday )
			isavailable = false;
		
	}
	else	{	// if the start day of week is greater than the end day of week
		if ( discoveryday > endday && discoveryday < startday )
			isavailable = false;
		
	}
		

	return isavailable;
}