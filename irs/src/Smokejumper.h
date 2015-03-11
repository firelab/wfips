// Smokejumper.h
// Class description for somkejumper class CSmokejumper derived from class CAirDelivered derived from class CResource
// for IRS 1/12

#ifndef SMOKEJUMPER_H
#define SMOKEJUMPER_H

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


class CSmokejumper : public CAirDelivered	{
	
public:
	// Constructor for smokejumpers
	CSmokejumper( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly );	
	CSmokejumper( const CSmokejumper &smokejumper );
	virtual ~CSmokejumper();

	CSmokejumper& operator=(const CSmokejumper &rhs);					// assignment operator

	virtual int CalcInitArrival( CFire fire, double distance );			// abstract - Calculate the initial arrival time for the smokejumper
	virtual bool IsRescAvailable( CFire fire);							// abstract - Determine if the resource is available
	virtual double DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM );		//abstract - get the fireline production rate for the fire
	virtual int CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates > &VProdRates, std::list< CContainValues > &VContainValues, double Multiplier );	//determine the start time, production rate, and duration values for the resource to input into contain algorithm
	

};		// End class CSmokejumper

#endif	// SMOKEJUMPER_H
