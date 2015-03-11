// Helicopter.cpp
// function definitions for class CHelicopter derived from CAerial derived from CResource
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
#include "Helicopter.h"
// Include definition for class CFire from Fire.h
#include "Fire.h"
// Include definition for class CFWA from FWA.h
#include "FWA.h"
// include contain values definitions from ContainValues.h
#include "ContainValues.h"


// Constructor for constant producing ground resource
CHelicopter::CHelicopter( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly, int volume ) 
		: CAerial( rescID, resctype, staffing, starttime, endtime, startdayofweek, enddayofweek, startseason, endseason, dispatchloc, 
		percentavail, daily, hourly, volume ) 
{
	SetWorkshiftLength( 14*60 );										// Set the workshift length for airtankers to 14 hours

	// Fill the vector with crew and volume carrying capacity for the helicopter
	// 0: 1-4999, 1: 5000-5999, 2: 6000-6999, 3: 7000-7999, 4: 8000-8999, 5:9000-9999, 6: 10000+
	
	m_HeliType = 0;														// Initially set to 0 for type I helicopter
	m_IsCarrier = false;												// Helicopter not a carrier ( yet )
	m_HelitackEquipped = false;											// The helicopter is not equipped to deliver helitack						
		
	if ( resctype.GetRescType() == "HEL1" )								// Type 1 helicopter
	{
		m_HeliType = 0;

		m_NumCrew.push_back(18);										// 0 - 4999 ft
		m_NumCrew.push_back(18);										// 5000-5999 ft
		m_NumCrew.push_back(16);										// 6000-6999 ft
		m_NumCrew.push_back(16);										// 7000-7999 ft
		m_NumCrew.push_back(14);										// 8000-8999 ft
		m_NumCrew.push_back(14);										// 9000-9999 ft
		m_NumCrew.push_back(12);										// 10000+ ft

		m_Gallons.push_back(2012);										// 0 - 4999 ft
		m_Gallons.push_back(1384);										// 5000-5999 ft
		m_Gallons.push_back(1207);										// 6000-6999 ft
		m_Gallons.push_back(1142);										// 7000-7999 ft
		m_Gallons.push_back(1000);										// 8000-8999 ft
		m_Gallons.push_back(898);										// 9000-9999 ft
		m_Gallons.push_back(707);										// 10000+ ft
	}
	
	if ( resctype.GetRescType() == "HEL2" )								// Type 2 helicopter
	{
		m_HeliType = 1;
		m_HelitackEquipped = true;	
		
		m_NumCrew.push_back(9);											// 0 - 4999 ft
		m_NumCrew.push_back(8);											// 5000-5999 ft
		m_NumCrew.push_back(7);											// 6000-6999 ft
		m_NumCrew.push_back(7);											// 7000-7999 ft
		m_NumCrew.push_back(6);											// 8000-8999 ft
		m_NumCrew.push_back(6);											// 9000-9999 ft
		m_NumCrew.push_back(4);											// 10000+ ft

		m_Gallons.push_back(404);										// 0 - 4999 ft
		m_Gallons.push_back(299);										// 5000-5999 ft
		m_Gallons.push_back(254);										// 6000-6999 ft
		m_Gallons.push_back(251);										// 7000-7999 ft
		m_Gallons.push_back(211);										// 8000-8999 ft
		m_Gallons.push_back(202);										// 9000-9999 ft
		m_Gallons.push_back(151);										// 10000+ ft

	}

	if ( resctype.GetRescType() == "HEL3" )								// Type 3 helicopter
	{
		m_HeliType = 2;
		m_HelitackEquipped = true;	

		m_NumCrew.push_back(4);											// 0 - 4999 ft
		m_NumCrew.push_back(4);											// 5000-5999 ft
		m_NumCrew.push_back(4);											// 6000-6999 ft
		m_NumCrew.push_back(4);											// 7000-7999 ft
		m_NumCrew.push_back(3);											// 8000-8999 ft
		m_NumCrew.push_back(3);											// 9000-9999 ft
		m_NumCrew.push_back(2);											// 10000+ ft

		m_Gallons.push_back(247);										// 0 - 4999 ft
		m_Gallons.push_back(154);										// 5000-5999 ft
		m_Gallons.push_back(131);										// 6000-6999 ft
		m_Gallons.push_back(123);										// 7000-7999 ft
		m_Gallons.push_back(111);										// 8000-8999 ft
		m_Gallons.push_back(102);										// 9000-9999 ft
		m_Gallons.push_back(81);										// 10000+ ft
	}

}

// Copy constructor for CHelicopter
CHelicopter::CHelicopter( const CHelicopter &helicopter ) : CAerial( helicopter )
{
	m_HeliType = helicopter.m_HeliType;
	m_NumCrew = helicopter.m_NumCrew;
	m_Gallons = helicopter.m_Gallons;
	m_IsCarrier = helicopter.m_IsCarrier;
	m_HelitackEquipped = helicopter.m_HelitackEquipped;
	m_AttachedCrews = helicopter.m_AttachedCrews;				// Note these are pointers and this needs to be changed for threading

}


// Destructor for CConstProd
CHelicopter::~CHelicopter() {}

// Assignment Operator
/*CHelicopter& CHelicopter::operator=(const CHelicopter &rhs)
{
	if (&rhs != this)	{
		CAerial::operator=(rhs);

		m_HeliType = rhs.m_HeliType;
		m_NumCrew = rhs.m_NumCrew;
		m_Gallons = rhs.m_Gallons;
		m_IsCarrier = rhs.m_IsCarrier;
		m_HelitackEquipped = rhs.m_HelitackEquipped;
		m_AttachedCrews = rhs.m_AttachedCrews;
	}

	return *this;
}*/

// Get number of crew helicopter can carry at elevation of fire
int CHelicopter::GetNumCrew( int elevation )
{	if ( elevation < 5000 )
		return m_NumCrew[0];
	if ( elevation > 4999 && elevation < 6000 )
		return m_NumCrew[1];
	if ( elevation > 5999 && elevation < 7000 )
		return m_NumCrew[2];
	if ( elevation > 6999 && elevation < 8000 )
		return m_NumCrew[3];
	if ( elevation > 7999 && elevation < 9000 )
		return m_NumCrew[4];
	if ( elevation > 8999 && elevation < 10000 )
		return m_NumCrew[5];
	return m_NumCrew[6];
}

// Get volume helicopter can carry at elevation of fire
int CHelicopter::GetGallons( int elevation )
{	if ( elevation < 5000 )
		return m_Gallons[0];
	if ( elevation > 4999 && elevation < 6000 )
		return m_Gallons[1];
	if ( elevation > 5999 && elevation < 7000 )
		return m_Gallons[2];
	if ( elevation > 6999 && elevation < 8000 )
		return m_Gallons[3];
	if ( elevation > 7999 && elevation < 9000 )
		return m_Gallons[4];
	if ( elevation > 8999 && elevation < 10000 )
		return m_Gallons[5];
	return m_Gallons[6];
}

// Set whether or not the helicopter is a carrier
void CHelicopter::SetIsCarrier( bool iscarrier )
{	m_IsCarrier = iscarrier;	}

// Get whether or not the helicotper is a carrier
bool CHelicopter::GetIsCarrier()
{	return m_IsCarrier;	}

// Set whether the helicopter can deliver helitack
void CHelicopter::SetHelitackEquipped ( bool helitackEquipped )
{	m_HelitackEquipped = helitackEquipped;	}

// Get whether the helicopter can deliver helitack
bool CHelicopter::GetHelitackEquipped()
{	return m_HelitackEquipped;	}

// Set the attached crews for the helicopter with a list
void CHelicopter::SetAttachedCrewsList ( std::list< CResource* > attachedCrews )
{	m_AttachedCrews = attachedCrews;

	// Be sure m_HelitackEquipped is set to true
	m_HelitackEquipped = true;
}

// Get a list of the attached crews for the helicopter
std::list< CResource* > CHelicopter::GetAttachedCrewsList()
{	return m_AttachedCrews;	}

// Add a helitack crew to the list of attached crew
void CHelicopter::AddAttachedCrew( CResource* helitackCrew )
{	m_AttachedCrews.push_back( helitackCrew );

	// Be sure m_HelitackEquipped is set to true
	m_HelitackEquipped = true;
}

// Remove a helitack crew from the list of attached crews
bool CHelicopter::RemoveAttachedCrew( CResource* helitackCrew )
{	int size = m_AttachedCrews.size();
	
	m_AttachedCrews.remove( helitackCrew );

	if ( size != m_AttachedCrews.size() )
		return true;

	else
		return false;

}

	

// Determine which is the best helicopter to deliver helitack/rappel crews from the list of available helicotpers
int CHelicopter:: BestCarrier( std::list< int > &Carrier, std::vector< CResource * > &Resource, int crewstaffing, int crewtravel, CFire fire, CDispLoc Crewdisploc ) 
{
	if ( Carrier.empty() )	{
		cout << "CHelicopter's BestCarrier function recieved an empty Carrier vector \n";
		return -10;														// There are no carriers in the list
	}
	else	{
		int bestcarrier = -5;											// holds index for the best carrier
		int Carrierarrival = 0;											// Arrival time for the current carrier
		CDispLoc displocCarrier = Crewdisploc;							// Dispatch location for the current carrier - will be reset
		int Mincarrierarrival = 10000;									// holds the arrival time for the best carrier
		int bestcapacity = 0;											// Carrying capacity for the best helicopter
		int comparecapacity = 0;										// capacity for comparison helicopter
		bool notime = true;												// Flag indicating that there are no helicopters yet that can deliver the crews before they reach their workshift length
		string CrewdisplocID = Crewdisploc.GetDispLocID();
		double Crewdistance = fire.GetFWA().GetDistance( CrewdisplocID );		// distance between crew dispatch location and FWA TTP
		int elevation = fire.GetElevation();							// Get elevation for the fire
		
		CAerial *Helicopter = 0;										// pointer to an aerial instance
		CHelicopter *Helicopter2 = 0;									// pointer to a helicopter instance

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
			Helicopter = dynamic_cast< CAerial * >( Resource[ *ItCarrier ] );
			if ( Helicopter!= 0 )	{
				NextLoadArrival = Helicopter->GetNextLoadArrival();
				CrewDispLocId = Helicopter->GetCrewDispLoc();

				// If the NextLoadArrival is 0 then this is would be the first load for the helicopter
				if ( NextLoadArrival > 0 )	{
					Carrierarrival = NextLoadArrival;
				
					// If the dispatch location for the new crew is different than the dispatch location for the crews the helicopter has deployed cannot use this helicopter
					if ( Crewdisploc.GetDispLocID() == CrewDispLocId )	{
						// If the crew will arrive later than the carrier, due to competition or other delays, use their arrival time
						if( crewtravel > Carrierarrival )		
							Carrierarrival = crewtravel;
												
						// If the helicopter cannot deliver the crew and return to it's dispatch location before the end of it's shift cannot be used
						int ShiftRemaining = Resource[ *ItCarrier ]->CalcRemainingWorkshift( fire, Carrierarrival );
						// Get the time for the carrier to return to its dispatch location
						string DispLocID = Resource[ *ItCarrier ]->GetDispLoc().GetDispLocID();
						double ReturnDistance = fire.GetFWA().GetDistance( DispLocID );
						int Speed = Resource[ *ItCarrier ]->GetRescType().GetAvgSpeed();
						int ReturnTime = static_cast< int >( ReturnDistance / Speed + 0.5 );

						if ( (ShiftRemaining - ReturnTime) >= 0 )	{
						
							// If the carrier arrival time is the same as the minimum carrier arrival time deploy this one if the staffing is better
							if ( Carrierarrival == Mincarrierarrival )	{
								// Get the capacity for the helicopter
								Helicopter2 = dynamic_cast< CHelicopter * >( Resource[ *ItCarrier ] );
								if ( Helicopter2!= 0 )
									comparecapacity = Helicopter2->GetNumCrew( elevation );
								else	{
									// Bad pointer for the helicoptr
									cout << "Bad pointer for the helicopter to get helicopter capacity in BestCarrier \n";
									return -1;
								}
								if ( ( crewstaffing > bestcapacity && comparecapacity > bestcapacity ) || ( crewstaffing < bestcapacity && comparecapacity < bestcapacity ) )	{
									bestcarrier = *ItCarrier;
									Mincarrierarrival = Carrierarrival;
									bestcapacity = comparecapacity;
									notime = false;
								}
								
							}
							// If the minimum arrival time is strictly less than the minimum carrier arrival time
							if ( Carrierarrival < Mincarrierarrival )	{
								bestcarrier = *ItCarrier;
								Mincarrierarrival = Carrierarrival;
								// Get the capacity for the helicopter
								Helicopter2 = dynamic_cast< CHelicopter * >( Resource[ *ItCarrier ] );
								if ( Helicopter2!= 0 )
									bestcapacity = Helicopter2->GetNumCrew( elevation );
								else	{
									// Bad pointer for the helicoptr
									cout << "Bad pointer for the helicopter to get helicopter capacity in BestCarrier \n";
									return -1;
								}
								notime = false;
							}
						}
					}
				}

				else	{	// The helicopter has not been used to deliver any crews
					// If the crew and carrier are not at the same dispatch location recalculate the arrival time for the carrier with the time for the carrier to pick up the crew
					if ( displocCarrier.GetDispLocID() != Crewdisploc.GetDispLocID() )	{
						double distance = fire.GetFWA().GetDistance( displocCarrier.GetDispLocID() );
						Carrierarrival = Helicopter->CalcInitArrival( fire, Crewdistance, distance );
						//cout << "Helicopter arrival time with " << Crewdistance << " distance to get crew: " << Carrierarrival << "\n";
					}

					// If the crew will arrive later than the carrier, due to competition or other delays, use their arrival time
					if( crewtravel > Carrierarrival )	
						Carrierarrival = crewtravel;
					
					// If the helicopter cannot deliver the crew and return to it's dispatch location before the end of it's shift cannot be used
					int ShiftRemaining = Resource[ *ItCarrier ]->CalcRemainingWorkshift( fire, Carrierarrival );
					// Get the time for the carrier to return to its dispatch location
					string DispLocID = Resource[ *ItCarrier ]->GetDispLoc().GetDispLocID();
					double ReturnDistance = fire.GetFWA().GetDistance( DispLocID );
					int Speed = Resource[ *ItCarrier ]->GetRescType().GetAvgSpeed();
					int ReturnTime = static_cast< int >( ReturnDistance / Speed + 0.5 );
			
					if ( (ShiftRemaining - ReturnTime) >= 0 )	{
				
						// If the carrier arrival time is the same as the minimum carrier arrival time deploy this one if the staffing is better
						if ( Carrierarrival == Mincarrierarrival )	{
							// Get the capacity for the helicopter
							Helicopter2 = dynamic_cast< CHelicopter * >( Resource[ *ItCarrier ] );
							if ( Helicopter2!= 0 )
								comparecapacity = Helicopter2->GetNumCrew( elevation );
							else	{
								// Bad pointer for the helicoptr
								cout << "Bad pointer for the helicopter to get helicopter capacity in BestCarrier \n";
								return -1;
							}
							if ( ( crewstaffing > bestcapacity && comparecapacity > bestcapacity ) || ( crewstaffing < bestcapacity && comparecapacity < bestcapacity ) )	{
								bestcarrier = *ItCarrier;
								Mincarrierarrival = Carrierarrival;
								bestcapacity = comparecapacity;
								notime = false;
							}
						}
						// If the carrier arrival is strictly less than the minimum carrier arrival time
						if ( Carrierarrival < Mincarrierarrival )	{
							bestcarrier = *ItCarrier;
							Mincarrierarrival = Carrierarrival;
							// Get the capacity for the helicopter
							Helicopter2 = dynamic_cast< CHelicopter * >( Resource[ *ItCarrier ] );
							if ( Helicopter2!= 0 )
								bestcapacity = Helicopter2->GetNumCrew( elevation );
							else	{
								// Bad pointer for the helicoptr
								cout << "Bad pointer for the helicopter to get helicopter capacity in BestCarrier \n";
								return -1;
							}
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
			//cout << "The best helicopter is: " << bestcarrier << " with initial arrival time: " << Mincarrierarrival << "\n";
			return bestcarrier;											//	bestcarrier holds the index for the carrier that's best for this load
		}
	}		// end Carrier list not empty

}		// end function BestCarrier

// Determine the production rate for bucket work with the helicotper
double CHelicopter::DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM )
{
	// Determine the volume that the helicopter can carry at the altitude of the fire
	int elevation = fire.GetElevation();
	int volume = GetGallons( elevation );

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
	case 165:
	case 184:
		factor = 0.5;
		break;

	case 164:
		if (SpecificCondition == "Black Spruce" )
			factor = 0.5;
		else 
			factor = 0.3;
		break;

	case 144:
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

	double volumedb = static_cast< double >(volume);
	double ProdRate = volumedb / 100 * factor;
	//cout << "Volume: " << volume << "\n";
	//cout << "Factor: " << factor << "\n";
	//cout << "Production Rate per hour: " << ProdRate << "\n";

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
int CHelicopter::CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates >  &VProdRate, std::list< CContainValues > &LContainValues, double Multiplier )
{
	// Do not create contain values if water drops are not allowed on the fire
	if ( fire.GetWaterDropsAllowed() )	{
		// If the helicopter has been used as a carrier need to use the Next load arrival time instead of the initial arrival time
		int segstart = 0;
		int NextLoadArrival = GetNextLoadArrival();
		if ( NextLoadArrival > 0 )
			segstart = NextLoadArrival + GetRescType().GetSetupDelay() + fire.GetFWA().GetReloadDelay(3);
		else
			segstart = GetInitArrivalTime() + GetRescType().GetSetupDelay() + fire.GetFWA().GetReloadDelay(3);
		// Adjust the initial arrival time if it is less than the first arrival time that includes first resource delay
		if ( segstart < firstarrival )
			segstart = firstarrival;
		SetInitArrivalTime( segstart );
		//cout << "Segment start time: " << segstart << "\n";
		int timeMSM = segstart + fire.FireStartTime();						// Minutes since midnight for segment start time
		double prodrate = DetermineProdRate( fire, VProdRate, timeMSM ) * Multiplier;	// Production rate for the first drop
		int duration = CalcRemainingWorkshift( fire );						// Drops have a duration of 1 minute.  This duration used to stop dropping. Includes the sunset.
		if ( duration < 1 )		// There is not enough time to make a drop
			return -10;
		string description = GetRescID();
		double daily = GetDailyCost();
		double hourly = GetHourlyCost();
		int sunsetMSM = fire.SunsetMSM();									// Get the sunset time in minutes since midnight

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
		int reloaddelay = fire.GetFWA().GetReloadDelay(3);					// Get reload delay to nearest base

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

	}	//	end water drops allowed

	return 0;

}

// Calculate the initial arrival time for helicotper (does not include first setup delay)
int CHelicopter::CalcInitArrival( CFire fire, double distance )
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

	//cout << "Difference due to Sunrise: " << diffsunrise << " Difference due to competition: " << diffcompetition << "\n";
	//cout << " Difference due to callback: " << callback << " Difference: " << difference << "\n";
	
	initarrivaldelay = initarrivaldelay + difference;					//Add the difference to the initial arrival delay
	//cout << "Helicopter initial arrival time: " << initarrivaldelay << "\n";
	SetInitArrivalTime( initarrivaldelay );								//save the initial arrival time to the fire
	return initarrivaldelay;
}

// Determine if the helicopter is available - differs from other aerial resources because helicopters are used to carry crews even when water drops are not allowed
bool CHelicopter::IsRescAvailable( CFire fire )
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
	//CDispLoc& disploc = GetDispLoc();
	//int FWAindex = fwa.GetIndex();
	//bool isassociated = disploc.DeterIsAssociated( FWAindex );		//determine if the resource's dispatch location is associated with the fire's fwa
	//if ( !isassociated )
		//isavailable = false;

	//cout << "Helicopter availability: " << isavailable << "\n";
	return isavailable;
}

// Get the next available time for the helicopter and all it's attached crews
int CHelicopter::GetHelitackCrewAvailableTime()
{
	int NextAvailableTime = GetAvailableTime();

	for ( list< CResource* >::iterator It = m_AttachedCrews.begin(); It != m_AttachedCrews.end(); It++ )	{

		int CrewAvailableTime = ( *It )->GetAvailableTime();

		if ( CrewAvailableTime > NextAvailableTime )
			NextAvailableTime = CrewAvailableTime;

	}

	return NextAvailableTime;

}

//Get the number of attached crews for the helicopter
int CHelicopter::AttachedCrewsListSize()
{	return m_AttachedCrews.size();	}