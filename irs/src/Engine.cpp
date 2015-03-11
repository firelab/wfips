// Engine.cpp
// Functions for class CEngine
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
// Include definition for class CResource from Rescbase.h
#include "Rescbase.h"
// Include definition for class CGround from ground.h
#include "Ground.h"
// Include definition for class CEngine from Engine.h
#include "Engine.h"
// Include definition for class CFire from Fire.h
#include "Fire.h"
// Include definition for class CFWA from FWA.h
#include "FWA.h"
// Include definition for class CProdRates from ProdRates.h
#include "ProdRates.h"

// Constructor for Handcrew ground resource
CEngine::CEngine( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly,
		int volume ) : CGround( rescID, resctype, staffing, starttime, endtime, startdayofweek, enddayofweek, startseason, endseason, 
		dispatchloc, percentavail, daily, hourly ) 
{	m_Volume = volume;	}

// Copy constructor for CEngine
CEngine::CEngine( const CEngine &engine ) : CGround( engine )
{
	m_Volume = engine.m_Volume;	

}

// Destructor for CEngine
CEngine::~CEngine() {}

// Assignment Operator
CEngine& CEngine::operator=(const CEngine &rhs)
{
	if (&rhs != this)	{
		CGround::operator=(rhs);

		m_Volume = rhs.m_Volume;
		m_CrewProdRate = rhs.m_CrewProdRate;
	}

	return *this;
}

// Set volume for the engine
void CEngine::SetVolume( int volume )
{	m_Volume = volume;	}

// Get the volume for the engine
int CEngine::GetVolume()
{	return m_Volume;	}

// Set the production rate for the crew without the engine
void CEngine::SetCrewProdRate( double prodrate )
{	m_CrewProdRate = prodrate;	}

// Get the production rate for the crew without the engine
double CEngine::GetCrewProdRate()
{	return m_CrewProdRate;	}

// Get the fireline production rate for the constant producer
double CEngine::DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM ) 
{		
	// Define all the values that need to be sent to the Compare(...) function for CProdRates
	string RescType = CResource::GetRescType().GetRescType();
	if ( RescType == "EN12" || RescType == "EN34" || RescType == "EN56" || RescType == "EN70" )
		RescType = "EN";
		if ( fire.GetIsPumpNRoll() )
			RescType = "ENP";
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
	while ( ProdRate < 0 )	{
		ProdRate = VProdRates[i].Compare( RescType, 0, Staffing, FuelModel, SpecificCondition );
		i++;
		if ( i >= static_cast< int >( VProdRates.size() ) )
			cout << "There is no entry in the Production Rate table for this combination \n";
	}
	
	//cout << "Fireline Production Rate for engine: " << ProdRate << "\n";

	SetProdRate( ProdRate );
	return ProdRate;
}

// Get the fireline production rate for the engine crew when the engine is reloading
double CEngine::DetermineProdRateCrew( CFire fire, std::vector< CProdRates > &VProdRates )
{
	// Define all the values that need to be sent to the Compare(...) function for CProdRates	
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
	while ( ProdRate < 0 )	{
		ProdRate = VProdRates[i].Compare( "CRW", 0, 1, FuelModel, SpecificCondition );
		i++;
		if ( i >= static_cast< int >( VProdRates.size() ) )
			cout << "There is no entry in the Production Rate table for this combination \n";
	}
	
	//cout << "Fireline Production Rate for single producer: " << ProdRate << "\n";

	ProdRate = ProdRate * ( Staffing - 1 );
	
	//cout << "Fireline Production Rate for Crew: " << ProdRate << "\n";

	SetCrewProdRate( ProdRate );
	return ProdRate;
}

// Calculate the number of engines and engine can pump for before it has to reload = volume / 6 gal/min
int CEngine::CalcEngProdTime()
{
	double dvolume = static_cast< double >( m_Volume );
	//cout << "Mintues of Pumping Time: " << dvolume / 6.0 << "\n";
	int minutes = static_cast< int >( dvolume / 6.0 );
	int remainder = static_cast< int >(dvolume) % 6;
	if ( remainder > 3 )
		minutes = minutes + 1;

	return minutes;
}

// Calculate the contain values for the deployed resources
int CEngine::CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates >  &VProdRate, std::list< CContainValues > &LContainValues, double Multiplier )
{
	// Adjust the initial arrival time if it is less than the first arrival time that includes first resource delay
	if ( GetInitArrivalTime() < firstarrival )
		SetInitArrivalTime( firstarrival );

	int segstart = GetInitArrivalTime();
	double prodrate = DetermineProdRate( fire, VProdRate, GetInitArrivalTime() ) * Multiplier;
	double crewprodrate = DetermineProdRateCrew( fire, VProdRate ) * Multiplier;
	int staffing = GetStaffing();
	double walkinprodrate = crewprodrate * staffing / ( staffing - 1 ) * Multiplier;
	int duration = CalcRemainingWorkshift( fire );
	int pumptime = CalcEngProdTime();
	CFWA FWA = fire.GetFWA();
	int engreloadtime = FWA.GetReloadDelay( 2 );
	string description = GetRescID();
	double daily = GetDailyCost();
	double hourly = GetHourlyCost();


	// If fire is walk-in use crew production rate for the duration of the workshift
	if ( fire.GetIsWalkIn() )	{
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
			LContainValues.insert( Iterator, CContainValues( segstart, walkinprodrate, duration, description, daily, hourly ) );
		}
	}
	else	{
		// Is there a water tender in the analysis?  if so when does it arrive? 
		int WTArrival = fire.GetWTArrivalTime();			// WTArrival = 10000 indicates no water tenders on the fire

		// Keep track of where in the reload cycle for the engine the program is at with IsReload
		bool IsReload = false;

		int cycletime = 0;									// duration time for the pump - reload cycle
		double cycleprodrate = 0.0;							// production rate for the cycle
		
		// Fire is not walk in.  While there is time left in the workshift length
		int remainingtime = duration;
		while ( remainingtime > 0 )	{
			if ( !IsReload )	{
			
				// Pumping cycle - use engine production rate
				if ( remainingtime < pumptime )
					cycletime = remainingtime;
				else
					cycletime = pumptime;

				cycleprodrate = prodrate;							// Set the production rate to the pumping rate
				IsReload = true;									// Set the next cycle to reload
			
			}
			else	{
				// Reload cycle: if water tender has not arrived during the pump cycle the engine needs to reload
				// else set IsReload to true and continue
				if ( segstart > WTArrival )	{
					// Engine doesn't need to reload and can pump to end of workshift
					cycletime = remainingtime;							// time for this cycle is the remaining time for the engine because the water tender has arrived
					cycleprodrate = prodrate;							// production rate for this cycle is the pumping production rate
					IsReload = false;									// set the next cycle ( there shouldn't be one ) to pumping
				}
				else	{
					// Engine needs to reload
					cycletime = engreloadtime;							// time for this cycle is the engine reload time 
					if ( engreloadtime > remainingtime )
						cycletime = remainingtime;						// use remaining time in workshift if not enough time to reload
					
					cycleprodrate = crewprodrate;						// production rate for this cycle is the production rate for the crew without the engine
					IsReload = false;									// set the next cycle pumping
				}
			}

			// Determine placement for new set of contain values in list VContainValues
			std::list< CContainValues >::iterator Iterator;

			// Print values
			//cout << "Segment Start: " << segstart << " Production Rate: " << cycleprodrate << " Duration: " << cycletime << " Description:" << description << "\n";

			if ( LContainValues.empty() )	
				LContainValues.push_back( CContainValues( segstart, cycleprodrate , cycletime, description, daily, hourly ));
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
			LContainValues.insert( Iterator, CContainValues( segstart, cycleprodrate, cycletime, description, daily, hourly ) );
				
			}
			
			remainingtime = remainingtime - cycletime;				// find time remaining after pump cycle
			segstart = segstart + cycletime;						// determine the time for the next segment to start

		}		// End While remaining time > 0
	}

	return 1;
}

// Print information about the engine - over rides CResource's PrintRescInfo
void CEngine::PrintRescInfo()
{
	cout << "Resource ID: " << GetRescID() << "\n";
	GetRescType().PrintRescType();
	GetDispLoc().PrintDispLoc();
	cout << "Staffing: " << GetStaffing()<< "\n";
	cout << "Shift Start Time: " << GetStartTime() << "\n";
	cout << "Shift End Time: " << GetEndTime() << "\n";
	cout << "Start Day of Week for Workshift: " <<GetStartDayofWeek() << "\n";
	cout << "End Day of Week for Workshift: " << GetEndDayofWeek() << "\n";
	cout << "Julian Date for Start of Season: " << GetStartSeason() << "\n";
	cout << "Julian Date for End of Season: " << GetEndSeason() << "\n";
	cout << "Percent Availability: " << GetPctAvail() << "\n";
	cout << "Engine Volume: " << m_Volume << "\n";
}