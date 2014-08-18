// Escape.cpp
// Member functions definition file for class CEscape for escape fire

#include <string>
using std::string;
#include <iostream>
using namespace std;
#include <cstdlib>													// for Rand function

#include "Escape.h"
#include "Fire.h"

// Constructor for an Escape Fire
CEscape::CEscape( int EscapeDate, int EscapeTime, CFire &Fire, int Level, int Duration, int Size, int WUILevel, 
	int HVRLevel, bool WFU ) : m_Fire( Fire )
{	
	m_EscapeDate = EscapeDate;
	m_EscapeTimeMSM = EscapeTime;
	m_Level = Level;													
	m_Duration = Duration;	
	m_Size = Size;		
	m_WUILevel = WUILevel;			
	m_HVRLevel = HVRLevel;	
	m_IsWFU = WFU;	

	for ( int i = 0; i < 15; i++ )
		m_VReqRescs.push_back( 0 );

}

// Destructor for an Escape Fire
CEscape::~CEscape() {}

// Copy constructor for an Escape Fire
CEscape::CEscape( const CEscape &Escape ) : m_Fire( Escape.m_Fire )
{
	m_EscapeDate = Escape.m_EscapeDate;
	m_EscapeTimeMSM = Escape.m_EscapeTimeMSM;

	// If the fire escaped on the next day
	if ( m_EscapeTimeMSM > 1440 )	{

		m_EscapeTimeMSM = m_EscapeTimeMSM - 1440;
		m_EscapeDate = m_EscapeDate + 1;

	}

	m_Level = Escape.m_Level;
	m_Duration = Escape.m_Duration;
	m_Size = Escape.m_Size;
	m_WUILevel = Escape.m_WUILevel;
	m_HVRLevel = Escape.m_HVRLevel;
	m_IsWFU = Escape.m_IsWFU;
	m_VReqRescs = Escape.m_VReqRescs;

}

// Assignment operator (=) for CEscape
const CEscape &CEscape::operator=( const CEscape &Escape )
{
	if ( &Escape != this )	{
		m_EscapeDate = Escape.m_EscapeDate;
		m_EscapeTimeMSM = Escape.m_EscapeTimeMSM;
		m_Fire = Escape.m_Fire;
		m_Level = Escape.m_Level;
		m_Duration = Escape.m_Duration;
		m_Size = Escape.m_Size;
		m_WUILevel = Escape.m_WUILevel;
		m_HVRLevel = Escape.m_HVRLevel;
		m_IsWFU = Escape.m_IsWFU;
		m_VReqRescs = Escape.m_VReqRescs;

	}

	return *this;

}

// Set the escape date
void CEscape::SetEscapeDate( int Julian )
{	m_EscapeDate = Julian;	}

// Get the escape date
int CEscape::GetEscapeDate()
{	return m_EscapeDate;	}

// Set the escape time in MSM
void CEscape::SetEscapeTimeMSM( int Time )
{	m_EscapeTimeMSM = Time;

	// If the fire escaped on the next day
	if ( m_EscapeTimeMSM > 1440 )	{

		m_EscapeTimeMSM = m_EscapeTimeMSM - 1440;
		m_EscapeDate = m_EscapeDate + 1;

	}

}

// Get the escape time in MSM
int CEscape::GetEscapeTimeMSM()
{	return m_EscapeTimeMSM;	}

// Set the escape time using time since begining of the year
void CEscape::SetEscapeTimeBY( int Time )
{	
	int Julian = m_EscapeDate;
	
	// Check if the Julian date for the fire has been set
	if ( Julian == 0 )	{

		// the escape date has not been set so use the Julian date for the fire
		Julian = m_Fire.GetJulianDay();

	}

	int DayTime = ( Julian - 1 ) * 1440;
	int TimeMSM = Time - DayTime;

	// If the time MSM is greater than 0 then reset the member value
	if ( TimeMSM >= 0 )
		m_EscapeTimeMSM = TimeMSM;

	else
		cout << "ERROR - The time for the fire is less than 0";
	
}

// Get the escape time in time since beginning of the year
int CEscape::GetEscapeTimeBY()
{	
	int Julian = m_EscapeDate;
	
	// Check if the Julian date for the fire has been set
	if ( Julian == 0 )	{

		// the escape date has not been set so use the Julian date for the fire
		Julian = m_Fire.GetJulianDay();

	}

	int DayTime = ( Julian - 1 ) * 1440;
	int TimeMSM = DayTime + m_EscapeTimeMSM;

	return TimeMSM;
	
}

// Set the fire
void CEscape::SetFire( CFire &Fire )
{	m_Fire = Fire;	}

// Get a reference to the fire
CFire &CEscape::GetFire()
{	return m_Fire;	}

// Set the level of the fire
void CEscape::SetLevel( int Level )
{	m_Level = Level;	}

// Get the level of the fire
int CEscape::GetLevel()
{	return m_Level;		}

// Set the duration of the fire
void CEscape::SetDuration( int Duration )
{	m_Duration = Duration;	}

// Get the duration of the fire
int CEscape::GetDuration()
{	return m_Duration;	}

// Set the size of the fire
void CEscape::SetSize( int Size )
{	m_Size = Size;	}

// Get the size of the fire
int CEscape::GetSize()
{	return m_Size;	}

// Set the WUILevel for the fire
void CEscape::SetWUILevel( int WUILevel )
{	m_WUILevel = WUILevel;	}

// Get the WUILevel for the fire
int CEscape::GetWUILevel()
{	return m_WUILevel;	}

// Set the HVRLevel for the fire
void CEscape::SetHVRLevel( int HVRLevel )
{	m_HVRLevel = HVRLevel;	}

// Get the HVRLevel for the fire
int CEscape::GetHVRLevel()
{	return m_HVRLevel;	}

// Set if the fire is WFU
void CEscape::SetWFU( bool WFU )
{	m_IsWFU = WFU;	}

// Get if the fire is WFU
bool CEscape::GetWFU()
{	return m_IsWFU;	}

// Set the requested resources vector with a vector
void CEscape::SetReqRescVector( vector< int > ReqResc )
{	m_VReqRescs = ReqResc;	}

// Set a value in the requested resources vector by index
void CEscape::SetReqRescIdx( int idx, int NumResc )
{	
	// Check if the index is valid
	if ( idx < 0 || idx >= m_VReqRescs.size() )
		cout << "The index to set a requested resource value for the escape fire is invalid";

	else
		m_VReqRescs[idx] = NumResc;

}

// Get the requested resources vector
vector< int > CEscape::GetReqRescVector()
{	return m_VReqRescs;	}

// Get a value from the requested resources vector by index
int CEscape::GetReqRescIdx( int idx )
{
	// Check if the index is valid
	if ( idx < 0 || idx >= m_VReqRescs.size() )	{

		cout << "The index to get a requested resource value for the escape fire is invalid";
		return -1;

	}

	else
		return m_VReqRescs[idx];

}

// Create an escape fire and randomly generate it's level
void CEscape::GenerateEscapeLevel()
{
	// Randomly generate the escape level
	// Current Levels
	//	- Local ( 1 ) - Resources on the fire stay until Post Escape Time
	//	- Regional ( 2 ) - Resources on the fire stay until Post Escape Time and Regional Crews and Helicopters are ordered
	//	- National ( 3 ) - Resources on the fire stay until Post Escape Time, airtankers are ordered, Regional Crews and helicopters are
	//				 ordered, and resources are moved from outside the region to assist on the fire

	int Random = 1 + rand() % 10;
	int Level = 1;

	if ( Random > 5 )
		Level = 2;

	if ( Random > 8 )
		Level = 3;
	
	SetLevel( Level );

	switch ( Level )	{

	case 1:
		SetDuration( 0 );
		SetSize( 1000 );
		SetWUILevel( 0 );
		SetHVRLevel( 0 );
		SetWFU( false );

		break;

	case 2:
		SetDuration( 3 );
		SetSize( 5000 );
		SetWUILevel( 0 );

		// Use a random draw to determine if the fire is in the WUI and needs engines
		Random = 1 + rand() % 10;
		if ( Random > 7 )	{

			SetWUILevel( 1 );
			m_VReqRescs[3] = 2;

		}
		
		SetHVRLevel( 0 );
		SetWFU( false );
		m_VReqRescs[13] = 1;
		m_VReqRescs[14] = 1;
		break;

	case 3:
		SetDuration( 14 );
		SetSize( 100000 );
		SetWUILevel( 0 );

		// Use a random draw to determine if the fire is in the WUI and needs engines
		Random = 1 + rand() % 10;
		if ( Random > 7 )	{

			SetWUILevel( 1 );
			m_VReqRescs[3] = 5;

		}

		SetHVRLevel( 0 );
		SetWFU( 0 );
		m_VReqRescs[0] = 0;
		m_VReqRescs[1] = 2;
		m_VReqRescs[13] = 2;
		m_VReqRescs[14] = 1;
		break;

	default:
		cout << "Escape fire not assigned a defined level" ;

	}

}
