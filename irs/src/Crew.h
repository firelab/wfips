// Crew.h
// definitions for class CCrew for handcrews where fireline production rates depend on the number of people in the crew
// for IRS 11/11

#ifndef CREW_H
#define CREW_H

#include <string>													//C++ standard string class
using std::string;

#include "Ground.h"
#include "Rescbase.h"
// include Resource Type definitions from CRescType.h
#include "CRescType.h"
// include Dispatch Location definitions from DispLoc.h
#include "DispLoc.h"
// include fire definitions from Fire.h
#include "Fire.h"
// include production rate definitions from ProdRates.h
#include "ProdRates.h"

class CCrew : public CGround {
	
public:
	// Constructor for handcrews which are ground resources CRW
	CCrew( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly );
	CCrew( const CCrew &crew );
	virtual ~CCrew();

	CCrew& operator=(const CCrew &rhs);						//assignment operator

	virtual double DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM );		//abstract - get the fireline production rate for the fire
	virtual int CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates > &VProdRates, std::list< CContainValues > &VContainValues, double Multiplier );	//determine the start time, production rate, and duration values for the resource to input into contain algorithm


};	// definition of class CCrew

#endif  // CREW_H
