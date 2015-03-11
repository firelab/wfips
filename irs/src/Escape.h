// Header file for escape fires Escape.h
// IRS remastered 5/13

#ifndef ESCAPE_H
#define ESCAPE_H

#include <string>
#include <iostream>
#include <cstdlib>  

#include "Fire.h"

class CEscape	{

public:
	CEscape( int EscapeDate, int EscapeTime, CFire &Fire, int Level = 0, int Duration = 0, int Size = 0, int WUILevel = 0, 
	int HVRLevel = 0, bool WFU = false );
		
	~CEscape();
		
	CEscape( const CEscape &Escape );
	CEscape &operator=( const CEscape &Escape );
	bool operator==(const CEscape &Escape);

	void SetEscapeDate( int Julian );								// Set the date the fire escaped
	int GetEscapeDate();											// Get the date the fire escaped
	void SetEscapeTimeMSM( int Time );								// Set the escape time (minutes since midnight)
	int GetEscapeTimeMSM();											// Get the escape time (minutes since midnight)
	void SetEscapeTimeBY( int Time );								// Set the escape time (minutes since beginning of the year)
	int GetEscapeTimeBY();											// Get the escape time (minutes since beginning of the year)
	void SetFire( CFire &Fire );									// Set the fire reference
	CFire &GetFire();												// Get the fire reference
	void SetLevel( int Level );										// Set the level for the fire
	int GetLevel();													// Get the level for the fire
	void SetDuration( int Duration );								// Set the duration for the fire (minutes)
	int GetDuration();												// Get the duration for the fire (minutes)
	void SetSize( int Size );										// Set the size for the fire (acres)
	int GetSize();													// Get the size for the fire (acres)
	void SetWUILevel( int WUILevel );								// Set the WUI Level for the fire
	int GetWUILevel();												// Get the WUI Level for the fire
	void SetHVRLevel( int HVRLevel );								// Set the HVR Level for the fire
	int GetHVRLevel();												// Get the HVR Level for the fire
	void SetWFU( bool WFU );										// Set if the fire is WFU
	bool GetWFU();													// Get if the fire is WFU
	void SetReqRescVector( vector< int > ReqResc );					// Set the requested resources vector with a vector
	void SetReqRescIdx( int idx, int NumResc );						// Set a value for the requested resources vector by index
	vector< int > GetReqRescVector();								// Get the requested resources vector
	int GetReqRescIdx( int idx );									// Get a value from the requested resources vector by index
	
	void GenerateEscapeLevel();										// Randomly generate the level for the fire - before large fire
	
private:
	int m_EscapeDate;												// Julian Date for the fire's escape
	int m_EscapeTimeMSM;											// Time the fire escaped
	CFire& m_Fire;													// Reference to the fire that escaped
	int m_Level;													// Response Level for the fire
	int m_Duration;													// Final duration for the escape fire (minutes)
	int m_Size;														// Final size in acres for the escape fire
	int m_WUILevel;													// Level of structures involved				
	int m_HVRLevel;													// Level of high valued resources
	bool m_IsWFU;													// Is the fire WFU?
	vector< int > m_VReqRescs;										// As Dispatch logic vector of resources requested for the fire
};

#endif 
