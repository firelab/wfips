// WaterTender.cpp
// definitions for class CWaterTender for water tenders with no fireline production rate
// for IRS 11/11

#include <iostream>												//contains functions for input and output
using namespace std;
#include <string>												//contains functions for operations with strings
using std::string;
#include <vector>
#include <cstring>												//to convert a string to a const char*
#include <cstdlib>												//to use atof to convert const char* to int and random numbers
//#include <cstdlib>											//contains function prototype for rand

// Include definition for class CResource from Rescbase.h
#include "Rescbase.h"
// Include definition for class CRescType from CRescType.h
#include "CRescType.h"
// Include definition for class CDispLoc from DispLoc.h
#include "DispLoc.h"
// Include definition for class CGround from ground.h
#include "Ground.h"
// Include definition for class CWaterTender from WaterTender.h
#include "WaterTender.h"
// Include definition for class CFire from Fire.h
#include "Fire.h"
// Include definition for class CFWA from FWA.h
#include "FWA.h"
// Include definition for class CProdRates from ProdRates.h
#include "ProdRates.h"

// Constructor for water tenders
CWaterTender::CWaterTender( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly ) 
		: CGround( rescID, resctype, staffing, starttime, endtime, startdayofweek, enddayofweek, startseason, endseason, dispatchloc, 
		percentavail, daily, hourly ) {}	

// Copy constructor for CWaterTender
CWaterTender::CWaterTender( const CWaterTender &watertender ) : CGround( watertender )
{}

// Destructor for class CWaterTender
CWaterTender::~CWaterTender() {}

// Determine production rate for water tender = 0
double CWaterTender::DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM )
{	SetProdRate( 0.0 );
	return 0.0;	}

// Calculate the contain values for the deployed resources
int CWaterTender::CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates >  &VProdRate, std::list< CContainValues > &LContainValues, double Multiplier )
{
	// WaterTenders do not build fireline so no contain values are needed

	return 1;
}