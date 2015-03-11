// Airtanker.h
// Defines the class for large airtankers CAirtanker
// for IRS 1/12

#ifndef AIRTANKER_H
#define AIRTANKER_H

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


class CAirtanker : public CAerial	{
	
public:
	// Constructor for Large Airtanker class CAirtanker
	CAirtanker( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly, int volume );	
	CAirtanker( const CAirtanker &airtanker );
	virtual ~CAirtanker();

	CAirtanker& operator=(const CAirtanker &rhs);					//assignment operator

	virtual double DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM );		//abstract - get the fireline production rate for the fire
	virtual int CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates > &VProdRates, std::list< CContainValues > &VContainValues, double Multiplier );	//determine the start time, production rate, and duration values for the resource to input into contain algorithm
	virtual bool IsRescAvailable( CFire fire );						//abstract - determine if the resource is available


};		// End class CAirtanker

#endif	// AIRTANKER_H