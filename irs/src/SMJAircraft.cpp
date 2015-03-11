// SMJAircraft.cpp
// function definitions for smokejumper aircraft class CSMJAircraft derived from CAerial derived from CResource
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
#include "SMJAircraft.h"
// Include definition for class CFire from Fire.h
#include "Fire.h"
// Include definition for class CFWA from FWA.h
#include "FWA.h"
// include contain values definitions from ContainValues.h
#include "ContainValues.h"


// Constructor for smokejumper aircraft
CSMJAircraft::CSMJAircraft( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly, int volume ) 
		: CAerial( rescID, resctype, staffing, starttime, endtime, startdayofweek, enddayofweek, startseason, endseason, dispatchloc, 
		percentavail, daily, hourly, volume ) 
{
	SetWorkshiftLength( 14*60 );											// Set the workshift length for airtankers to 14 hours
}

// Copy constructor for CSMJAircraft
CSMJAircraft::CSMJAircraft( const CSMJAircraft &smjaircraft ) : CAerial( smjaircraft )
{}

// Destructor for CSMJAircraft
CSMJAircraft::~CSMJAircraft() {}


// Assignment Operator
CSMJAircraft& CSMJAircraft::operator=(const CSMJAircraft &rhs)
{
	if (&rhs != this)
		CAerial::operator=(rhs);
	return *this;
}

// Get the fireline production rate for the constant producer
double CSMJAircraft::DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM ) 
{		
	// Smokejumper aircraft do not produce fireline
	return 0;
}

// Calculate the contain values for the deployed resources
int CSMJAircraft::CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates >  &VProdRate, std::list< CContainValues > &LContainValues, double Multiplier )
{
	// Smokejumper aircraft do not build fireline so no contain values are needed

	// Determine the end time for the smokejumper aircraft's activities which are delivering smokejumpers and returning to the base
	CFWA& fwa = fire.GetFWA();
	string DispLocID = GetDispLoc().GetDispLocID();
	double onewaytravel = fwa.GetDistance( DispLocID );
	int onewaytraveltime = static_cast< int>( onewaytravel * 60 / GetRescType().GetAvgSpeed() + 0.5 );
	int endtime = fire.FireStartTime() + GetInitArrivalTime() + onewaytraveltime;		// time airtanker finishes last drop
	
	// Return the time the resource ends work on the fire
	return endtime;

}

// Determine which smokejumper aircraft is the best for delivering the smokejumpers to the fire
int CSMJAircraft:: BestCarrier( std::list< int > &Carrier, std::vector< CResource * > &Resource, int crewstaffing, int crewtravel, CFire fire, CDispLoc Crewdisploc ) 
{
	if ( Carrier.empty() )	{
		cout << "CSMJAircraft's BestCarrier function recieved an empty Carrier vector \n";
		return -10;														// There are no carriers in the list
	}
	else	{
		int bestcarrier = -5;											// holds index for the best carrier
		int Carrierarrival = 0;											// Arrival time for the current carrier
		CDispLoc displocCarrier = Crewdisploc;							// Dispatch location for the current carrier - will be reset
		int Mincarrierarrival = 10000;									// holds the arrival time for the best carrier
		bool notime = true;												// Flag indicating that there are no helicopters yet that can deliver the crews before they reach their workshift length
		string DispLocID = displocCarrier.GetDispLocID();
		double Crewdistance = fire.GetFWA().GetDistance( DispLocID );		// distance between crew dispatch location and FWA TTP
				
		CAerial *SJAC = 0;												// pointer to an aerial instance
		CSMJAircraft *SJAC2 = 0;										// pointer to a helicopter instance

		int NextLoadArrival = 0;
		string CrewDispLocId = "";

		// Get the first carrier in the list
		std::list< int >::iterator ItCarrier;							// Iterator for the carrier list
		ItCarrier = Carrier.begin();	

		// Iterate through the carriers and find the best carrier for the  crew
		while ( ItCarrier != Carrier.end() )	{

			Carrierarrival = Resource[ *ItCarrier ]->GetInitArrivalTime();	// get arrival time for the current carrier
			displocCarrier = Resource[ *ItCarrier ]->GetDispLoc();			// Get the dispatch location for the current carrier

			// Get next load arrival and crew dispatch location for the carrier.
			SJAC = dynamic_cast< CAerial * >( Resource[ *ItCarrier ] );
			if ( SJAC!= 0 )	{
				NextLoadArrival = SJAC->GetNextLoadArrival();
				CrewDispLocId = SJAC->GetCrewDispLoc();

				// If the NextLoadArrival is 0 then this is would be the first load for the smokejumper aircraft
				if ( NextLoadArrival > 0 )	{
					Carrierarrival = NextLoadArrival;
				
					// If the dispatch location for the new crew is different than the dispatch location for the crews the smokejumper aircraft has deployed cannot use this smokejumper aircraft
					if ( Crewdisploc.GetDispLocID() == CrewDispLocId )	{
						// If the crew will arrive later than the carrier, due to competition or other delays, use their arrival time
						if( crewtravel > Carrierarrival )		
							Carrierarrival = crewtravel;
												
						// If the smokejumper aircraft cannot deliver the crew and return to it's dispatch location before the end of it's shift cannot be used
						int ShiftRemaining = Resource[ *ItCarrier ]->CalcRemainingWorkshift( fire, Carrierarrival );
						// Get the time for the carrier to return to its dispatch location
						string DispLocID = Resource[ *ItCarrier ]->GetDispLoc().GetDispLocID();
						double ReturnDistance = fire.GetFWA().GetDistance( DispLocID );
						int Speed = Resource[ *ItCarrier ]->GetRescType().GetAvgSpeed();
						int ReturnTime = static_cast< int >( ReturnDistance / Speed + 0.5 );

						if ( (ShiftRemaining - ReturnTime) >= 0 )	{
						
							// If the minimum arrival time is strictly less than the minimum carrier arrival time
							if ( Carrierarrival < Mincarrierarrival )	{
								bestcarrier = *ItCarrier;
								Mincarrierarrival = Carrierarrival;
								notime = false;
							}
						}
					}
				}

				else	{	// The smokejumper aircraft has not been used to deliver any crews
					// If the crew and carrier are not at the same dispatch location recalculate the arrival time for the carrier with the time for the carrier to pick up the crew
					if ( displocCarrier.GetDispLocID() != Crewdisploc.GetDispLocID() )	{
						double distance = fire.GetFWA().GetDistance( displocCarrier.GetDispLocID() );
						Carrierarrival = SJAC->CalcInitArrival( fire, Crewdistance, distance );
						//cout << "Smokejumper aircraft arrival time with " << Crewdistance << " distance to get crew: " << Carrierarrival << "\n";
					}

					// If the crew will arrive later than the carrier, due to competition or other delays, use their arrival time
					if( crewtravel > Carrierarrival )	
						Carrierarrival = crewtravel;
					
					// If the smokejumper aircraft cannot deliver the crew and return to it's dispatch location before the end of it's shift cannot be used
					int ShiftRemaining = Resource[ *ItCarrier ]->CalcRemainingWorkshift( fire, Carrierarrival );
					// Get the time for the carrier to return to its dispatch location
					string DispLocID = Resource[ *ItCarrier ]->GetDispLoc().GetDispLocID();
					double ReturnDistance = fire.GetFWA().GetDistance( DispLocID );
					int Speed = Resource[ *ItCarrier ]->GetRescType().GetAvgSpeed();
					int ReturnTime = static_cast< int >( ReturnDistance / Speed + 0.5 );
			
					if ( (ShiftRemaining - ReturnTime) >= 0 )	{
				
						// If the carrier arrival is strictly less than the minimum carrier arrival time
						if ( Carrierarrival < Mincarrierarrival )	{
							bestcarrier = *ItCarrier;
							Mincarrierarrival = Carrierarrival;
							notime = false;
						}
					}
				}
			}

			else	{	
				// Did not get the appropriate pointer
				cout << "Bad dynamic_cast for aerial to get next load arrival time in CHelicopter's BestCarrier \n";
				return -1;
			}
			// Get the next carrier
			ItCarrier++;
		}		// End of while not end of carrier list

		// Either have a best carrier or notime = true in which case there is no best carrier
		
		// If notime = true then there are no carriers available which can deliver the crew and return to base before they reach their workshift length
		// return -10 if this is the case

		if ( notime )
			return -10;
		else	{
			// adjust bestcarrier's initial arrival time
			Resource[ bestcarrier ]->SetInitArrivalTime( Mincarrierarrival );
			//cout << "The best smokejumper aircraft is: " << bestcarrier << " with initial arrival time: " << Mincarrierarrival << "\n";
			return bestcarrier;											//	bestcarrier holds the index for the carrier that's best for this load
		}
	}		// end Carrier list not empty

}		// end function BestCarrier

/*
// Determine if the resource is available
bool CSMJAircraft::IsRescAvailable( CFire fire )
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
	bool isassociated = disploc.DeterIsAssociated( FWAindex );		//determine if the resource's dispatch location is associated with the fire's fwa
	if ( !isassociated )
		isavailable = false;

	return isavailable;
}
*/

bool CSMJAircraft::IsRescAvailable( CFire fire )
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

	return isavailable;
}



