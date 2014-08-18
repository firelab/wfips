// WaterTender.h
// definitions for the class CWaterTender describing the water tender suppression resources
// for IRS 11/11

#ifndef WATERTENDER_H
#define WATERTENDER_H

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

class CWaterTender : public CGround {
	
public:
	// Constructor for handcrews which are ground resources CRW
	CWaterTender( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly );	
	CWaterTender( const CWaterTender &watertender );
	virtual ~CWaterTender();												// destructor for class CWaterTender
	
	virtual double DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM );		//abstract - get the fireline production rate for the fire
	virtual int CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates > &VProdRates, std::list< CContainValues > &VContainValues, double Multiplier );	//determine the start time, production rate, and duration values for the resource to input into contain algorithm

	
};	// definition of class CCrew

#endif  // WATERTENDER_H
