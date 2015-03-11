// SmallAT.h
// Defines the class for small airtankers CSmallAT specifically Seats and Scoopers
// for IRS 1/12

#ifndef SmallAT_H
#define SmallAT_H

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


class CSmallAT : public CAerial	{
	
public:
	// Constructor for constant producing ground resources DZR1, DZR2, DZR3, FBDZ, TP12, TP23, TP56 and FRBT
	CSmallAT( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly, int volume );	
	CSmallAT( const CSmallAT &smallat );
	virtual ~CSmallAT();

	CSmallAT& operator=(const CSmallAT &rhs);					//assignment operator

	virtual double DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM );		//abstract - get the fireline production rate for the fire
	virtual int CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates > &VProdRates, std::list< CContainValues > &VContainValues, double Multiplier );	//determine the start time, production rate, and duration values for the resource to input into contain algorithm
	


};		// End class CSmallAT

#endif	// SmallAT_H