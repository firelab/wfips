// Aerial.h 
// Function definitions for class CAerial from base class CResource for aerial resources and resources delivered by aerial resources
// for IRS 1/12

#ifndef AERIAL_H
#define AERIAL_H

#include "Rescbase.h"											//include the base class for CGround, CResource class definition file
#include "Fire.h"												//include the class CFire containing fires

class CAerial : public CResource	{

public:
	
	CAerial( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly, int volume );		//constructor for ground resources
	CAerial( const CAerial &Aerial );							// Copy Constructor
	virtual ~CAerial();											//destructor

	CAerial& operator=(const CAerial &rhs);						//assignment operator
	
	// Get and set functions for member variables
	void SetVolume( int volume );								//set volume of aircraft - helicopter volume depends on altitude so set to 0 initially
	int GetVolume();											//get volume of aircraft - helicopter volume depends on altitude so set to 0 initially
	void SetNextLoadArrival( int nextloadarrival );				// set the time when the carrier would deliver the next load to the fire
	int GetNextLoadArrival();									// get the time when the carrier would deliver the next load to the fire
	void SetCrewDispLoc( string disploc );						// set the id for the dispatch location where the crews are being delivered from
	string GetCrewDispLoc();									// get the id for the dispatch location where the crews are being delivered from

	virtual int CalcInitArrival( CFire fire, double distance );					//calculate the initial arrival time for the resource
	virtual int CalcInitArrival( CFire fire, double adddistance, double distance );		// Calculate initial arrival time if smokejumper aircraft or helicopter has to pick up crews from a different base
	virtual bool IsRescAvailable( CFire fire );					//is the resource available?
	virtual int CalcRemainingWorkshift( CFire fire );			//calculate the time remaining in the workshift to use on current fire after resources arrive
	virtual int AdjustWorkshiftStartTime( CFire fire );			// Reset the Workshift start time for the aerial resources deployed to the fire	

private:
	int m_Volume;												//volume of drop
	int m_NextLoadArrival;										// the time when the next load of crew would be delivered to the fire, -1 first load
	string m_CrewDispLoc;										// id for the dispatch location the carrier is delivering crew from
}; //	end  class CAerial

#endif //	AERIAL_H