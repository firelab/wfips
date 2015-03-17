// Crew.cpp
// definitions for class CCrew for handcrews where fireline production rates depend on the number of people in the crew
// for IRS 11/11

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
// Include definition for class CGround from ground.h
#include "Ground.h"
// Include definition for class CCrew from Crew.h
#include "Crew.h"
// Include definition for class CFire from Fire.h
#include "Fire.h"
// Include definition for class CFWA from FWA.h
#include "FWA.h"
// Include definition for class CProdRates from ProdRates.h
#include "ProdRates.h"

// Constructor for Handcrew ground resource
CCrew::CCrew( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly ) 
		: CGround( rescID, resctype, staffing, starttime, endtime, startdayofweek, enddayofweek, startseason, endseason, dispatchloc, 
		percentavail, daily, hourly ) {}

// Copy constructor for CCrew
CCrew::CCrew( const CCrew &crew ) : CGround( crew )
{}

// Destructor for CCrew
CCrew::~CCrew(){}

// Get the fireline production rate for the constant producer
double CCrew::DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM ) 
{		
	// Define all the values that need to be sent to the Compare(...) function for CProdRates
	string RescType = CResource::GetRescType().GetRescType();
	int SlopePct = fire.GetSlopePct();
	int SlopeClass = 0;												//fireline production rates for crew do not depend on slope
	
	//cout << "Resource Type: " << RescType << "\n";
	//cout << " Slope class: " << SlopeClass << "\n";
	
	int Staffing = CResource::GetStaffing();
	int FuelModel = fire.GetFuelModel();
	string SpecificCondition = fire.GetSpecificCondition();

	//cout << "Staffing: " << Staffing << "\n";
	//cout << "Fuel Model: " << FuelModel << "\n";
	//cout << "Specific Condition: " << SpecificCondition << "\n";
	
	double ProdRate = -1.0;
	// Get the fireline production rate for the constant producer
	//cout << "Number of entries in VPRodRates vector: " << VProdRates.size() << "\n";
	int i = 0;
	while ( ProdRate < 0 && i < (int)VProdRates.size() )	{
		ProdRate = VProdRates[i].Compare( RescType, 0, 1, FuelModel, SpecificCondition );
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
int CCrew::CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates >  &VProdRate, std::list< CContainValues > &LContainValues, double Multiplier )
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
