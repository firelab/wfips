// RescWorkYear.h 
// class description for elements of a vector that store the times when a resource is deployed to a fire CRescWorkYear
// for IRS 12/11

#ifndef RESCWORKYEAR_H
#define RESCWORKYEAR_H


// There are 5 elements to this class
//	julian day
//	start time for suppression effort.  This is the time when the resource is deployed to the fire
//	end time for suppression effort.  This is the time when the resource is available for the next fire.
//	effort type.	Resource used on a contained fire
//					Resource not used on a contained fire
//					Resource on a escape fire
//  flag. Indicating if the resource had a 2 to 1 break between the previous fire of the day
//			and this fire.  0 - false, 1 - true

// The available time for the next day's fires is calculated by summing the total time  the resource is on fires or on workshift and 
//	requiring that the resource is not available on the next day until it has at least half these hours in a break.  If the resource has 
//	an adequate break between fires on the fire day, those fires are not included in the calculation.  The same calculations are used 
//	when the workshift length length is reached.

#include <string>
using std::string;

class CRescWorkYear		{

public:
	CRescWorkYear( int scenario, int firenum, int julian, int starttime, int endtime, string effort, bool flag );	// Constructor
	CRescWorkYear( const CRescWorkYear& that);
	~CRescWorkYear();

	CRescWorkYear& operator=( const CRescWorkYear& that);

	void SetScenario( int scenario );
	int GetScenario();
	void SetFireNum( int firenum );
	int GetFireNum();
	void SetJulianDay( int julian );
	int GetJulianDay();
	void SetStartTime( int starttime );
	int GetStartTime();
	void SetEndTime( int endtime );
	int GetEndTime();
	void SetEffort( string effort );
	string GetEffort();
	void SetFlag( bool flag );
	bool GetFlag();

private:
	int m_Scenario;													// Scenario Number
	int m_FireNum;													// Fire Number
	int m_Julian;													// Julian day for the fire's discovery time
	int m_StartTime;												// Time when the resource is deployed to the fire in minutes since start of year
	int m_EndTime;													// Time when the resource is available for the next fire in minutes since start of year
	string m_Effort;												// Type of effort 0 - contained fire resource used, 1- contained fire resource unused, 2- escape fire
	bool m_Flag;													// Indicates if the resource achieved the 2 to 1 work rest limit before being deployed to this fire

	void PrintEntry();												// Print the values for this entry


};		// End of class CRescWorkYear

#endif // RESCWORKYEAR_H




