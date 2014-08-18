// AirDelivered.h
// Class definition for ground resources that are delivered by aircraft i.e. smokejumpers and helitack/rappel crews derived from CResource
// for IRS 1/12

#ifndef AIRDELIVERED_H
#define AIRDELIVERED_H

#include <string>													//C++ standard string class
using std::string;

#include "Rescbase.h"
// include Resource Type definitions from CRescType.h
#include "CRescType.h"
// include Dispatch Location definitions from DispLoc.h
#include "DispLoc.h"
// include fire definitions from Fire.h
#include "Fire.h"
// include production rate definitions from ProdRates.h
#include "ProdRates.h"

class CAirDelivered : public CResource {
	
public:
	// Constructor for crews that are delivered by aircraft
	CAirDelivered( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly );
	CAirDelivered( const CAirDelivered &airdelivered );				
	virtual ~CAirDelivered();


	virtual void SetCarrierIndex( int carrierindex );					// Set the index for the carrier aircraft
	virtual int GetCarrierIndex();										// Get the index for the carrier aircraft

	virtual int CalcInitArrival( CFire fire, double distance );							// abstract - Calculate the initial arrival time for the aerial delivered crews
	virtual bool IsRescAvailable( CFire fire);							// abstract - Determine if the resource is available
	virtual double DetermineProdRate( CFire fire, std::vector< CProdRates > &VProdRates, int timeMSM );		//abstract - get the fireline production rate for the fire
	virtual int CreateContainValues( CFire fire, int firstarrival, std::vector< CProdRates > &VProdRates, std::list< CContainValues > &VContainValues, double Multiplier );	//determine the start time, production rate, and duration values for the resource to input into contain algorithm
	virtual bool DeploySelectEqual( CResource *resc2 );					//determine if two resources are equivalent for deploying by comparing m_InitArrival, m_DispLoc, and m_Duration 
	virtual bool DeploySelectCompare( CResource *resc2 );				//determine if resource 1 is better to deploy than resource 2 based on m_InitArrival, m_DispLoc, and m_Duration
	virtual int AdjustWorkshiftStartTime( CFire fire );					// Reset the Workshift start time for the aerial delivered resources deployed to the fire	
	virtual int CalcRemainingWorkshift( CFire fire );					//calculate the time remaining in the workshift to use on current fire after resources arrive

private:

	int m_CarrierIndex;													// Resource index for the carrier aircraft used to deploy crews to the fire ( default -1 )


};	// definition of class CAirDelivered

#endif  // AIRDELIVERED_H