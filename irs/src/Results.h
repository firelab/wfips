// Results.h
// Class to store the Initial Attack results for a scenario
// for IRS 12/11

#ifndef RESULTS_H
#define RESULTS_H


#include <iostream>
using namespace std;
#include <string>

#include "Fire.h"
#include "Contain.h"
#include "ContainSim.h"

class CResults	{

public:
#ifdef ENABLE_IRS_TESTING
        CResults(CFire fire):m_Fire(fire){}
#endif
	CResults( CFire fire, double firecost, double fireline, double fireperimeter, double firesize, double firesweep, double firetime, 
		double numrescused, string Status, bool displogicfilled, bool InSeason );
	CResults( const CResults &results );
	~CResults();

	CResults& operator=(const CResults  &rhs);						//assignment operator
	
	//void SetFire( CFire &fire );									// Set the fire for the results
	CFire GetFire();												// Get the fire for the results
	void SetFireCost( double firecost );							// Set the costs for the fire
	double GetFireCost();											// Get the costs for the fire
	void SetFireLine( double fireline );							// Set the length of line for the fire
	double GetFireLine();											// Get the length of line for the fire
	void SetFirePerimeter( double fireperimeter );					// Set the perimeter for the fire
	double GetFirePerimeter();										// Get the perimeter for the fire
	void SetFireSize( double firesize );							// Set the size of the fire
	double GetFireSize();											// Get the size of the fire
	void SetFireSweep( double firesweep );							// Set the sweep of the fire
	double GetFireSweep();											// Get the sweep of the fire
	void SetFireTime( double firetime );							// Set the final fire time
	double GetFireTime();											// Get the final fire time
	void SetNumRescUsed( double rescused );							// Set the number of resources used on the fire
	double GetNumRescUsed();										// Get the number of resources used on the fire
	void SetStatus( string status );								// Set the fire Status
	string GetStatus();												// Get the fire Status
	void SetDispLogicFilled( bool displogicfilled );				// Set the dispatch Logic filled indicator
	bool GetDispLogicFilled();										// Get the dispatch logic filled indicator
	void SetInSeason( bool inseason );								// Set the level 3 resources in season
	bool GetInSeason();												// Get the level 3 resources in season

	void PrintResults();											// Print the results

private:
	
	CFire m_Fire;
	double m_FinalFireCost;
  	double m_FinalFireLine;
  	double m_FinalFirePerimeter;
  	double m_FinalFireSize;
  	double m_FinalFireSweep;
  	double m_FinalFireTime;
  	double m_FinalResourcesUsed;
	string m_Status;
	bool m_DispLogicFilled;
	bool m_InSeason;
	
};	// End Class CResults

#endif	// define RESULTS_H
