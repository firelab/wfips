// RescWorkYear.cpp
// Function definitions for class CRescWorkYear that form an entry for a log of a resources activities on fires during the year
// for IRS 12/11

#include <iostream>
using namespace std;
#include <string>
using std::string;

#include "RescWorkYear.h"

// Constructor for class CRescWorkYear
CRescWorkYear::CRescWorkYear( int scenario, int firenum, int julian, int starttime, int endtime, string effort, bool flag )
{
	m_Scenario = scenario;
	m_FireNum = firenum;
	m_Julian = julian;
	m_StartTime = starttime;
	m_EndTime = endtime;
	m_Effort = effort;
	m_Flag = flag;
}

// Copy Constructor for class CRescWorkYear
CRescWorkYear::CRescWorkYear( const CRescWorkYear& that )
{
	m_Scenario = that.m_Scenario;
	m_FireNum = that.m_FireNum;
	m_Julian = that.m_Julian;
	m_StartTime = that.m_StartTime;
	m_EndTime = that.m_EndTime;
	m_Effort = that.m_Effort;
	m_Flag = that.m_Flag;
}

// Destructor for class CRescWorkYear
CRescWorkYear::~CRescWorkYear() {}

// Assignment operator for class CRescWorkYear
CRescWorkYear& CRescWorkYear::operator=( const CRescWorkYear& that)
{
	m_Scenario = that.m_Scenario;
	m_FireNum = that.m_FireNum;
	m_Julian = that.m_Julian;
	m_StartTime = that.m_StartTime;
	m_EndTime = that.m_EndTime;
	m_Effort = that.m_Effort;
	m_Flag = that.m_Flag;
	
	return *this;
}

// Set the Scenario for the entry
void CRescWorkYear::SetScenario( int scenario )
{	m_Scenario = scenario;	}

// Get the Scenario for the entry
int CRescWorkYear::GetScenario()
{	return m_Scenario;	}

// Set the fire number for the entry
void CRescWorkYear::SetFireNum( int firenum )
{	m_FireNum = firenum;	}

// Get the fire number for the entry
int CRescWorkYear::GetFireNum()
{	return m_FireNum;	}

// Set Julian day for the fire's discovery
void CRescWorkYear::SetJulianDay( int julian)
{	m_Julian = julian;	}

// Get the Julian day for the fire's descovery
int CRescWorkYear::GetJulianDay()
{	return m_Julian;	}

// Set Start time
void CRescWorkYear::SetStartTime( int starttime )
{	m_StartTime = starttime;	}

// Get start time
int CRescWorkYear::GetStartTime()
{	return m_StartTime;	}

// Set End Time
void CRescWorkYear::SetEndTime( int endtime )
{	m_EndTime = endtime;	}

// Get End Time
int CRescWorkYear::GetEndTime()
{	return m_EndTime;	}

// Set Effort type: fire contained resource used, fire contained resource unused, fire escape
void CRescWorkYear::SetEffort( string effort )
{ m_Effort = effort;	}

// Get Effort Type: fire contained resource used, fire contained resource unused, fire escape
string CRescWorkYear::GetEffort()
{	return m_Effort;	}

// Set the flag indicating if the resources achieved an adequate work to rest break between this fire and the previous ones
void CRescWorkYear::SetFlag( bool flag )
{	m_Flag = flag;	}

// Get the flag indicating if the resources achieved and adequate work to rest break between this fire and the previous ones
bool CRescWorkYear::GetFlag()
{	return m_Flag;	}

void CRescWorkYear::PrintEntry()
{
	cout << "	Julian Day: " << m_Julian;
	cout << "	Start Time: " << m_StartTime;
	cout << "	End Time: " << m_EndTime;
	cout << "	Effort: " << m_Effort;
	cout << "	Flag: "	<< m_Flag;
}




