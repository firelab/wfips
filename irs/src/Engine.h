// Engine.h
// definitions for class CEngine for engine resources
// for IRS 11/11

#ifndef CENGINE_H
#define CENGINE_H

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

class CEngine : public CGround {
	
public:
	// Constructor for engines which are ground resources EN12, EN34, EN56, EN70
	CEngine( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly, int volume );	
	CEngine( const CEngine &engine );
	virtual ~CEngine();												//Destructor

	CEngine& operator=(const CEngine &rhs);							//Assignment Operator
	
	void SetVolume( int volume );									//Set the volume for the engine
	int GetVolume();												//Get the volume for the engine
	void SetCrewProdRate( double prodrate );						//Set the production rate for the crew without the engine
	double GetCrewProdRate();										//Get the production rate for the crew without the engine
	
	virtual double DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM );		//abstract - get the fireline production rate for the fire
	double DetermineProdRateCrew( CFire fire, std::vector< CProdRates > &VProdRates );	//get the fireline production rate for the crew when the engine is reloading = CRW prod rate * (staffing - 1)
	int CalcEngProdTime();										//Calculate the time an engine operates before needing to reload
	virtual int CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates > &VProdRates, std::list< CContainValues > &VContainValues, double Multiplier );	//determine the start time, production rate, and duration values for the resource to input into contain algorithm

	virtual void PrintRescInfo();									//print information about resource

private:
	int m_Volume;													//Volume for the engine
	double m_CrewProdRate;											//Production rate for the crew without the engine

};	// definition of class CEngine

#endif  // ENGINE_H
