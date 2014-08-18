// ConstProd.h
// Defines the class for ground resources that are constant producers
// for IRS 11/11

#ifndef CONSTPROD_H
#define CONSTPROD_H

#include <string>										//C++ standard string class
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
// include contain values definitions from ContainValues.h
#include "ContainValues.h"


class CConstProd : public CGround	{
	
public:
	// Constructor for constant producing ground resources DZR1, DZR2, DZR3, FBDZ, TP12, TP23, TP56 and FRBT
	CConstProd( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly );	
	CConstProd( const CConstProd &constprod );
	virtual ~CConstProd();

	virtual double DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM );		//abstract - get the fireline production rate for the fire
	virtual int CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates > &VProdRates, std::list< CContainValues > &VContainValues, double Multiplier );	//determine the start time, production rate, and duration values for the resource to input into contain algorithm

};		// End class CConstProd

#endif	// CONSTPROD_H
