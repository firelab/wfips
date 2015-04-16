// ConstProd.cpp
// function definitions for class CConstProd derived from class CGround derived from class CResource
// defines functions for ground resources that have a constant production rate once they arrive at the fire
// DZR1, DZR2, DZR3, FBDZ, TP12, TP34, and TP56
// for IRS 11/11

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
// Include definition for class CGround from ground.h
#include "Ground.h"
// Include definition for class CConstProd from ConstProd.h
#include "ConstProd.h"
// Include definition for class CFire from Fire.h
#include "Fire.h"
// Include definition for class CFWA from FWA.h
#include "FWA.h"
// Include definition for class CProdRates from ProdRates.h
#include "ProdRates.h"
// include contain values definitions from ContainValues.h
#include "ContainValues.h"


// Constructor for constant producing ground resource
CConstProd::CConstProd( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly ) 
		: CGround( rescID, resctype, staffing, starttime, endtime, startdayofweek, enddayofweek, startseason, endseason, dispatchloc, 
		percentavail, daily, hourly ) {}

CConstProd::CConstProd( const CConstProd &constprod ) : CGround( constprod ) {}

// Destructor for CConstProd
CConstProd::~CConstProd() {}

CConstProd& CConstProd::operator=(const CConstProd &rhs)
{
	if (&rhs != this)
		CGround::operator=(rhs);
	return *this;
}


// Get the fireline production rate for the constant producer
double CConstProd::DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM ) 
{		
	// Define all the values that need to be sent to the Compare(...) function for CProdRates
	string RescType = CResource::GetRescType().GetRescType();
	int SlopePct = fire.GetSlopePct();
	int SlopeClass = 0;
	//if ( SlopePct > 55 )
		//SlopeClass = 4;	
	if ( SlopePct > 40 )		//&& SlopePct < 56 )
		SlopeClass = 3;	
	if ( SlopePct > 25 && SlopePct < 41 )
		SlopeClass = 2;
	if ( SlopePct < 26 )
		SlopeClass = 1;
	//cout << "Resource Type: " << RescType << "\n";
	//cout << " Slope class: " << SlopeClass << "\n";
	int Staffing = CResource::GetStaffing();
	// Be sure that staffing is 1 for dozers and tractor plows
	if ( RescType == "DZR1" || RescType == "DZR2" || RescType == "DZR3" || RescType == "TP12" || RescType == "TP34" || RescType == "TP56" )
		Staffing = 1;
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
		ProdRate = VProdRates[i].Compare( RescType, SlopeClass, Staffing, FuelModel, SpecificCondition );
		i++;
		if ( i >= static_cast< int >( VProdRates.size() ) )
			cout << "There is no entry in the Production Rate table for this combination \n";
	}
	
	//cout << "Production Rate: " << ProdRate << "\n";
	SetProdRate( ProdRate );
	return ProdRate;
}

// Calculate the contain values for the deployed resources
int CConstProd::CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates >  &VProdRate, std::list< CContainValues > &LContainValues, double Multiplier )
{
	// Adjust the initial arrival time if it is less than the first arrival time that includes first resource delay
	if ( GetInitArrivalTime() < firstarrival )
		SetInitArrivalTime( firstarrival );

	int segstart = GetInitArrivalTime();
	double prodrate = DetermineProdRate( fire, VProdRate, GetInitArrivalTime() )  * Multiplier;
	int duration = CalcRemainingWorkshift( fire );
	string description = GetRescID();
	double daily = GetDailyCost();
	double hourly = GetHourlyCost();

	// Determine placement for new set of contain values in list VContainValues
	std::list< CContainValues >::iterator Iterator;

	if ( LContainValues.empty() )	
		LContainValues.push_back( CContainValues( segstart, prodrate, duration, description, daily, hourly ));
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
		LContainValues.insert( Iterator, CContainValues( segstart, prodrate * Multiplier, duration, description, daily, hourly ) );

	}

	return 1;

}
