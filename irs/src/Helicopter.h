// Helicopter.h
// Class defining functions for the class CHelicopter derived from base class CAviation, which is derived from class CResource
// for IRS 1/12

#ifndef HELICOPTER_H
#define HELICOPTER_H

#include <string>														//C++ standard string class
using std::string;
#include <vector>														// For the vectors storing the crew and volume capacity

#include "Aerial.h"
#include "Rescbase.h"
// include Resource Type definitions from CRescType.h
#include "CRescType.h"
// include Dispatch Location definitions from DispLoc.h
#include "DispLoc.h"
// include fire definitions from Fire.h
#include "Fire.h"
// include production rate definitions from ProdRates.h
#include "ProdRates.h"
// include contain values definitions from ContainValues.h
#include "ContainValues.h"
// include Aerial resource type definitions from CAerial.h
#include "Aerial.h"


class CHelicopter : public CAerial	{

public:
	CHelicopter( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly, int volume );
	CHelicopter( const CHelicopter &helicopter );
	virtual ~CHelicopter();

	int GetNumCrew( int elevation );								// Get the number of crew that can be carried based on elevation at fire
	int GetGallons( int elevation );								// Get the volume for drops based on elevation at fire
	void SetIsCarrier( bool iscarrier );							// Set whether or not the helicopter is a carrier
	bool GetIsCarrier();											// Get whether or not the helicotper is a carrier
	void SetHelitackEquipped ( bool helitackEquipped );				// Set whether the helicopter can deliver helitack
	bool GetHelitackEquipped ();									// Get whether the helicopter can deliver helitack
	void SetAttachedCrewsList ( std::list< CResource* > attachedCrews );	// Set the attached crews for the helicopter with a list
	std::list< CResource* > GetAttachedCrewsList();					// Get a list of the attached crews for the helicopter
	void AddAttachedCrew( CResource* helitackCrew );				// Add a helitack crew to the list of attached crew
	bool RemoveAttachedCrew( CResource* helitackCrew );				// Remove a helitack crew from the list of attached crews
	int GetHelitackCrewAvailableTime();								// Get the next available time for the helicopter and attached crews
	int AttachedCrewsListSize();									// 

	virtual bool IsRescAvailable( CFire fire);						// abstract - Determine if the resource is available
	virtual int CalcInitArrival( CFire fire, double distance );		// Calculate the initial arrival time for helicopters does not include setup delay which is used to deploy the bucket
	virtual double DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM );		//abstract - get the fireline production rate for the fire
	virtual int CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates > &VProdRates, std::list< CContainValues > &VContainValues, double Multiplier );	//determine the start time, production rate, and duration values for the resource to input into contain algorithm
	int BestCarrier( std::list< int > &Carrier, std::vector< CResource * > &Resource, int crewstaffing, int crewtravel, CFire fire, CDispLoc Crewdisploc);	// Determine which helicopter is the best for carrying helicack/rappel crews to a fire

private: 
	int m_HeliType;													// Helicopter Type I: 1, II: 2, or III: 3
	std::vector< int >m_NumCrew;									// Number of crew that can be carried at different altitudes 0 - 0-4999, 1 - 5000-5999, 2 - 6000-6999
																	// 3 - 7000-7999, 4 - 8000-8999, 5 - 9000-9999, 6 - 1000+ for the helicopter type
	std::vector< int >m_Gallons;									// Volume that can be carried at different altitudes ( as above ) for the helicopter type
	bool m_IsCarrier;												// Indicates whether or not the helicopter is being used as a carrier

	std::list<CResource*> m_AttachedCrews;							// List of crews that are attached to the helicotper for prepositioning and drawdown
	bool m_HelitackEquipped;										// Indicates whether the helicopter is equipped to deliver helitack crews

	CHelicopter& operator=(const CHelicopter &rhs){};				// assignment operator private because pointer to attached crews needs to be updated for threading

};
#endif	// end HELICOPTER.H
