// Helitack.h
// Class description for helitack/rappel class CHelitack derived from class CAirDelivered derived from class CResource
// for IRS 2/12

#ifndef HELITACK_H
#define HELITACK_H

#include <string>										//C++ standard string class
using std::string;

#include "Rescbase.h"
// include class definition for aerial delivered crews
#include "AirDelivered.h"
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

class CHelitack : public CAirDelivered	{
	
public:
	// Constructor for helitack/rappel crews
	CHelitack( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly );	
	CHelitack( const CHelitack &helitack );
	virtual ~CHelitack();

	CHelitack& operator=(const CHelitack &rhs);						// assignment operator
	
	virtual int CalcInitArrival( CFire fire, double distance );		// abstract - Calculate the initial arrival time for the helitack/rappel crews
	virtual bool IsRescAvailable( CFire fire);						// abstract - Determine if the resource is available
	virtual double DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM );		//abstract - get the fireline production rate for the fire
	virtual int CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates > &VProdRates, std::list< CContainValues > &VContainValues, double Multiplier );	//determine the start time, production rate, and duration values for the resource to input into contain algorithm
	
};		// End class CHelitack
#endif	// HELITACK_H
