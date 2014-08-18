// SMJAircraft.h
// Class definitions for smokejumper aircraft class CSMJAircraft 
// for IRS 1/12

#ifndef SMJAIRCRAFT_H
#define SMJAIRCRAFT_H

#include <string>										//C++ standard string class
using std::string;

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


class CSMJAircraft : public CAerial	{
	
public:
	// Constructor for smokejumper aircraft class CSMJAircraft
	CSMJAircraft( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly, int volume );	
	CSMJAircraft( const CSMJAircraft &smjaircraft );
	virtual ~CSMJAircraft();

	virtual double DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM );		//abstract - get the fireline production rate for the fire
	virtual int CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates > &VProdRates, std::list< CContainValues > &VContainValues, double Multiplier );	//determine the start time, production rate, and duration values for the resource to input into contain algorithm
	int BestCarrier( std::list< int > &Carrier, std::vector< CResource * > &Resource, int crewstaffing, int crewtravel, CFire fire, CDispLoc Crewdisploc);	// Determine which smokejumper aircraft is best for deploying the crews to the fire

	bool IsRescAvailable( CFire fire );


};		// End class CSMJAircraft

#endif	// SMJAIRCRAFT_H
