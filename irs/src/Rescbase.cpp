// Rescbase.cpp
// Member function definitions for class CResource
// created 10/11 for IRS

#include <iostream>											//contains functions for input and output
using namespace std;
#include <string>											//contains functions for operations with strings
using std::string;
#include <cstring>											//to convert a string to a const char*
#include <cstdlib>											//to use atof to convert const char* to int
#include <vector>
#include <utility>

// Include definition for class CResource from Rescbase.h
#include "Rescbase.h"
// Include definition for class CRescType from CRescType.h
#include "CRescType.h"
// Include definition for class CDispLoc from DispLoc.h
#include "DispLoc.h"
// Include difinition for class CFire from Fire.h
#include "Fire.h"
// Include definitions for class CRescWorkYear form RescWorkYear.h
#include "RescWorkYear.h"


// define and initialize static data member
int CResource::count = 0;

// Constructor for CResource
CResource::CResource( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly)
		: m_RescType( resctype ), m_DispLoc( dispatchloc )
{

	m_RescID = rescID;									//unique id for the resource
	m_Staffing = staffing;								//staffing for the resource
	m_StartTime = starttime;							//start time for the resource's workday
	m_EndTime = endtime;								//end time for the resource's workday
	SetStartDayofWeek( startdayofweek );				//start day for the resource's workweek
	SetEndDayofWeek( enddayofweek );					//end day for the resource's workweek
	m_StartSeason = startseason;						//julian date for the start of the resource's season
	m_EndSeason = endseason;							//julian date for the end of the resource's season
	m_PctAvail = percentavail;							//percent availability for the resource
	m_WorkshiftLength = 16*60;							//workshift length for the resource --default 18 hours--
	//m_AvailableTime = -1;								//Time when resource will be available due to previous fires in minutes since start of year
	m_WorkshiftStartTime = -1;							//Time in minutes since midnight that workshift started - not started yet = -1
	m_InitArrival = -1;									//resources initial arrival time to a fire (default -1 indicates resource not on fire)
	m_Duration = -1;									//duration of workshift left for resource to work fire (default -1 indicates resource not on fire)
	m_ProdRate = -1;									//production rate for resource on fire (default -1 indicates resource not on fire)
	m_DailyCost = 0.0;									//daily cost for the resource
	m_HourlyCost = 0.0;									//hourly cost for the resource
	m_AvailableFlag = false;							//indicates if the resource has timed out for the day ( 0 - not timed out, 1 - timed out)
	m_IsBorrowed = false;								//indicates if the resource has been borrowed by another dispatch location
	m_ReturnTime = -1;									//the time the resource has to leave the dispatch location that borrowed it

	// Set the dispatch type from the resource type
	m_DispatchType = DetermineDispType();							// Determine the dispatch type for the resource
	m_DelayType = DetermineDelayType();								// Determine the delay type for the resource 

	// Set the first entry in the resource movement vector to be the arrival at the original dispatch location
	int StartTime = ( m_StartSeason - 1 ) * 1440 + 1;
	m_AvailableTime = StartTime;

	if ( m_StartSeason > m_EndSeason )
		m_AvailableTime = 0;

	string StartLocation = m_DispLoc.GetDispLocID();

	m_VMovement.push_back( std::make_pair( m_AvailableTime, StartLocation ) );

	count++;														// Increase the resource count
}

// Copy Constructor for CResource
CResource::CResource( const CResource &resource ) : m_RescType( resource.m_RescType ), m_DispLoc( resource.m_DispLoc )
{
	m_RescID = resource.m_RescID;									
	m_Staffing = resource.m_Staffing;								
	m_StartTime = resource.m_StartTime;							
	m_EndTime = resource.m_EndTime;								
	m_StartDayofWeek = resource.m_StartDayofWeek;				
	m_EndDayofWeek = resource.m_EndDayofWeek;					
	m_StartSeason = resource.m_StartSeason;						
	m_EndSeason = resource.m_EndSeason;							
	m_PctAvail = resource.m_PctAvail;
	m_DailyCost = resource.m_DailyCost;									
	m_HourlyCost = resource.m_HourlyCost;	
	m_WorkshiftLength = resource.m_WorkshiftLength;							
	m_InitArrival = resource.m_InitArrival;									
	m_Duration = resource.m_Duration;									
	m_ProdRate = resource.m_ProdRate;									
	m_AvailableTime = resource.m_AvailableTime;	
	m_AvailableFlag = resource.m_AvailableFlag;	
	m_DispatchType = resource.m_DispatchType;							
	m_DelayType = resource.m_DelayType;	
	m_WorkshiftStartTime = resource.m_WorkshiftStartTime;
	m_VRescWorkYear = resource.m_VRescWorkYear;
	m_NumWorkEntries = resource.m_NumWorkEntries;
	m_IsBorrowed = resource.m_IsBorrowed;								
	m_ReturnTime = resource.m_ReturnTime;									
	m_VMovement = resource.m_VMovement;
	m_VDrawDown = resource.m_VDrawDown;
	
}

// Destructor for CResource
CResource::~CResource() {}

// Set the Resource ID
void CResource::SetRescID( string rescid )
{	m_RescID = rescid;	}

// Get the Resource ID
string CResource::GetRescID()
{	return m_RescID;	}

// Set the Resource Type
void CResource::SetRescType( CRescType &resctype )
{	
	// Need to check if a valid resource type is entered
	m_RescType = resctype;
	m_DispatchType = DetermineDispType();							// Determine the dispatch type for the resource
	m_DelayType = DetermineDelayType();								// Determine the dealy type for the resource
}

// Get the dispatch type for the resource
int CResource::GetDispatchType()
{	return m_DispatchType;	}

// Get the delay type for the resource
int CResource::GetDelayType()
{	return m_DelayType;	}

// Get the Resource Type
CRescType &CResource::GetRescType()
{	return m_RescType;	}

// Set the Resource staffing
void CResource::SetStaffing( int staffing )
{	m_Staffing = staffing;	}

// Get the Resource staffing
int CResource::GetStaffing()
{	return m_Staffing;	}

// Set the Start Time for the Resource workshift
void CResource::SetStartTime( string starttime)
{
	int len = starttime.length();
	if (len != 4) starttime = "0000";								//is the length of the start time 4? if not, invalid, set to 0000

	string hr = starttime.substr(0,2);								//get the hour portion of the string
	const char* hrcstr = hr.c_str();								//cast to const char 
	double hrnum = atof( hrcstr );									//so can use atof to convert to integer
	if (hrnum > 24.0 || hrnum < 0.0) starttime = "0000";			//if hour not between 00 and 24 invalid, set to 0000

	string min = starttime.substr(2,2);								//get the minute portion of the string
	const char* mincstr = min.c_str();								//cast to const char
	double minnum = atof( mincstr );								//so can use atof to convert to integer
	if (minnum > 60.0 || minnum < 0.0) starttime = "0000";			//if minute not between 00 and 60 invalid, set to 0000

	m_StartTime = starttime;
}

// Get the Start Time for the Resource workshift
string CResource::GetStartTime()
{	return m_StartTime;	}


// Set the End Time for the Resource workshift
void CResource::SetEndTime( string endtime)
{
	int len = endtime.length();
	if (len != 4) endtime = "2400";									//is the length of the start time 4? if not, invalid, set to 2400
	string hr = endtime.substr(0,2);								//get the hour portion of the string
	const char* hrcstr = hr.c_str();								//cast to const char 
	double hrnum = atof( hrcstr );									//so can use atof to convert to integer
	if (hrnum > 24.0 || hrnum < 0.0) endtime = "2400";				//if hour not between 00 and 24 invalid, set to 2400

	string min = endtime.substr(2,2);								//get the minute portion of the string
	const char* mincstr = min.c_str();								//cast to const char
	double minnum = atof( mincstr );								//so can use atof to convert to integer
	if (minnum > 60.0 || minnum < 0.0) endtime = "2400";			//if minute not between 00 and 60 invalid, set to 2400

	m_EndTime = endtime;
}

// Get the End Time for the Resource workshift
string CResource::GetEndTime()
{	return m_EndTime;	}

// Set the Start Day of Week for the Resource workweek
void CResource::SetStartDayofWeek( string startday )
{	m_StartDayofWeek = startday;	}

// Get the Start Day of Week for the Resource workweek
string CResource::GetStartDayofWeek()
{	return m_StartDayofWeek;	}


// Set the End Day of Week for the Resource workweek
void CResource::SetEndDayofWeek( string endday )
{	m_EndDayofWeek = endday;	}

// Get the End Day of Week for the Resource workweek
string CResource::GetEndDayofWeek()
{	return m_EndDayofWeek;	}

// Set the Julian Date for the Start of the Resource's season
void CResource::SetStartSeason( int startseason )
{
	// Check if valid julian date
	if (startseason < 0 || startseason > 365 ) startseason = 0;

	m_StartSeason = startseason;
}

// Get the Julian Date for the Start of the Resource's season
int CResource::GetStartSeason()
{	return m_StartSeason;	}

// Set the Julian Date for the End of the Resource's season
void CResource::SetEndSeason( int endseason )
{
	// Check if valid julian date
	if (endseason < 0 || endseason > 365 ) endseason = 365;

	m_EndSeason = endseason;
}

// Get the Julian Date for the End of the Resource's season
int CResource::GetEndSeason()
{	return m_EndSeason;	}

// Set the dispatch location for the resource
void CResource::SetDispLoc( CDispLoc &disploc)
{	m_DispLoc = disploc;	}

// Get the dispatch location for the resource
CDispLoc &CResource::GetDispLoc()
{	return m_DispLoc;	}

// Set the percent availability for the resource
void CResource::SetPctAvail( int pctavail )
{
	// Check if the percent is an integer between 0 and 100
	if (pctavail < 0 || pctavail > 100) pctavail = 100;

	m_PctAvail = pctavail;
}

// Get the percent availability for the resource
int CResource::GetPctAvail()
{	return m_PctAvail;	}

// Set the daily cost for the resource
void CResource::SetDailyCost( double dailycost )
{	m_DailyCost = dailycost;	}

// Get the daily cost for the resource
double CResource::GetDailyCost()
{	return m_DailyCost;	}

// Set the hourly cost for the resource
void CResource::SetHourlyCost( double hourlycost )
{	m_HourlyCost = hourlycost;	}

// Get the hourly cost for the resource
double CResource::GetHourlyCost()
{	return m_HourlyCost;	}


// Set the time the resource is next available
void CResource::SetAvailableTime( int availabletime )
{	m_AvailableTime = availabletime;	}

// Get the time the resource is next available
int CResource::GetAvailableTime()
{	
	if ( m_AvailableTime == -1 )	{

		// Set the available time to the start time for the resource's season
		m_AvailableTime = ( m_StartSeason - 1 ) * 1440 + 1;

		if ( m_StartSeason > m_EndSeason )
			m_AvailableTime = 0;

	}

	return m_AvailableTime;

}

// Set the time in minutes since midnight that the resource's workshift started - -1 hasn't started
void CResource::SetWorkshiftStartTime( int time )
{	m_WorkshiftStartTime = time;	}

// Get the time in minutes since midnight that the resource's workshift started - -1 hasn't started
int CResource::GetWorkshiftStartTime()
{	return m_WorkshiftStartTime;	}

// Set the workshift length
void CResource::SetWorkshiftLength( int time )
{	m_WorkshiftLength = time;	}

// Get the workshift length
int CResource::GetWorkshiftLength()
{	return m_WorkshiftLength;	}

// Set the Initial Arrival time to the fire
void CResource::SetInitArrivalTime( int time )
{	m_InitArrival = time;	}

// Get the Initial Arrival time to the fire
int CResource::GetInitArrivalTime()
{	return m_InitArrival;	}

// Set the Duration of time the resource is available for the fire
void CResource::SetDuration( int duration )
{	m_Duration = duration;	}

// Get the Duration of the the resource is available for the fire
int CResource::GetDuration()
{	return m_Duration;	}

// Set the Production rate for the resource at the fire
void CResource::SetProdRate( double prodrate )
{	m_ProdRate = prodrate;	}

// Get the Production rate for the resource at the fire
double CResource::GetProdRate()
{	return m_ProdRate;	}

// Set the available flag which indicates if the resource has timed out for the day ( 0 - not timed out, 1 - timed out )
void CResource::SetAvailableFlag( bool flag )
{	m_AvailableFlag = flag;	}

// Get the available flag which indicates if the resource has timed out for the day ( 0 - not timed out, 1 - timed out )
bool CResource::GetAvailableFlag()
{	return m_AvailableFlag;	}

// Set the value of the is borrowed flag
void CResource::SetIsBorrowed( bool flag )
{	m_IsBorrowed = flag;	}

// Get the value of the is borrowed flag
bool CResource::GetIsBorrowed()
{	return m_IsBorrowed;	}

// Set the time the resource needs to leave the borrowing dispatch location - Minutes since beginning of year
void CResource::SetReturnTime( int Time )
{	m_ReturnTime = Time;	}

// Get the time the resource needs to leave the borrowing dispatch location - Minutes since beginning of year
int CResource::GetReturnTime()
{	return m_ReturnTime;	}

// Record the movement of the resource from one dispatch location to another
void CResource::RecordMove( int timeMBY, string DispLoc )
{
	m_VMovement.push_back( std::make_pair( timeMBY, DispLoc ) );
}

// Get the vector of the resource's movement
vector< std::pair< int, string > > CResource::GetMovement()
{	return m_VMovement;	}

// Get a pair of time since beginning of the year and dispatch location from the m_VMovement vector
std::pair< int, string > CResource::GetMovementPair( int i )
{	
	// Is i a valid index for the vector?
	if ( i > -1 && i < m_VMovement.size() )
		return m_VMovement[i];	

	else 
		return std::make_pair( -1, "" );

}

// Get the current dispatch location for the resource
string CResource::GetCurrentLocation()
{
	// Get the last pair for the movement vector
	int Last = m_VMovement.size() - 1;
	std::pair< int, string > Pair = GetMovementPair( Last );

	return Pair.second;
}

// Record the time when a resource was used on an escape fire
void CResource::RecordDrawDown( int timeMBY, int FireNum )
{
	m_VDrawDown.push_back( std::make_pair( timeMBY, FireNum ) );

}

// Get the vector of resource use on escape fires
vector< std::pair< int, int > > CResource::GetDrawDown()
{	return m_VDrawDown;	}

// Get a pair of time and fire number values from the draw down vector for the resource
std::pair< int, int > CResource::GetDrawDownPair( int i )
{
	// Is i a valid index for the vector?
	if ( i > -1 && i < m_VDrawDown.size() )
		return m_VDrawDown[i];

	else
		return std::make_pair( -1, -1 );

}

// Determine the dispatch resource type
int CResource::DetermineDispType()
{
	int type = -1;													//stores the dispatch logic index for the m_RescNums array
	string resctype = m_RescType.GetRescType();
	if ( resctype == "ATT" ) type = 0;
        else if ( resctype == "CRW" ) type = 1;
        else if ( resctype == "DZR1" || resctype == "DZR2" || resctype == "DZR3" ) type = 2;
        else if ( resctype == "EN12" || resctype == "EN34" || resctype == "EN56"  || resctype == "EN70" ) type = 3;
        else if ( resctype == "FBDZ" ) type = 4;
        else if ( resctype == "FRBT" ) type = 5;
        else if ( resctype == "HEL1" || resctype == "HEL2" || resctype == "HEL3" ) type = 6;
        else if ( resctype == "HELI" ) type = 7;
        else if ( resctype == "SEAT" || resctype == "SCP" ) type = 8;
        else if ( resctype == "SJAC" ) type = 9;
        else if ( resctype == "SMJR" ) type = 10;
        else if ( resctype == "TP12" || resctype == "TP34" || resctype == "TP56" ) type = 11;
        else if ( resctype == "WT" ) type = 12;
        else
            cout << "ERROR incorrect Resource Type \n";

	m_DispatchType = type;											//store the dispatch logic index for the m_RescNums array
	return type;
}

// Determine the FWA delay resource type
int CResource::DetermineDelayType()
{
	int type = -1;													//stores the FWA delay index for the various delay vectors
	string resctype = m_RescType.GetRescType();
	if ( resctype == "DZR1" || resctype == "DZR2" || resctype == "DZR3" ) type = 0;
	if ( resctype == "TP12" || resctype == "TP34" || resctype == "TP56" ) type = 0;
	if ( resctype == "FBDZ" ) type = 1;
	if ( resctype == "FRBT" ) type = 1;
	if ( resctype == "CRW" ) type = 2;
	if ( resctype == "EN12" || resctype == "EN34" || resctype == "EN56"  || resctype == "EN70" ) type = 3;
	if ( resctype == "HELI" ) type = 4;
	if ( resctype == "SMJR" ) type = 5;
	
	m_DelayType = type;												//store the delay type for use with the FWA information
	return type;
}

// Convert day of week to number of day in week 0 = sunday - 6 = saturday
int CResource::ConvertDay( string day )
{
	if ( day == "Sunday" )
		return 0;
	if ( day == "Monday" )
		return 1;
	if ( day == "Tuesday" )
		return 2;
	if ( day == "Wednesday" )
		return 3;
	if ( day == "Thursday" )
		return 4;
	if ( day == "Friday" )
		return 5;
	if ( day == "Saturday" )
		return 6;
	// or else
	return -1;
}


// Convert time in four didget string to hours since midnight
int CResource::ConvertTime( string time )
{
        string hr;
	string min;
        if(time.size() == 4)
        {
            hr = time.substr(0,2);									//get the hour portion of the string
            min = time.substr(2,2);									//get the minute portion of the string
        }
        else if (time.size() == 3)
        {
            hr = time.substr(0,1);
            min = time.substr(1,2);									//get the minute portion of the string
        }
        else /* bad mojo, emit error */ ;
	const char* hrcstr = hr.c_str();								//cast to const char 
	double hrnum = atof( hrcstr );									//so can use atof to convert to integer			

	const char* mincstr = min.c_str();								//cast to const char
	double minnum = atof( mincstr );								//so can use atof to convert to integer
	
	double timeminmidnight = hrnum * 60 + minnum;					//minutes since midnight for fire's discoverytime
	return static_cast< int>( timeminmidnight );
}

// Get the count for the number of resources instantiated
int CResource::GetCount()
{	return count;	}

// Print information about the resource
void CResource::PrintRescInfo()
{
	cout << "Resource ID: " << m_RescID << "\n";
	m_RescType.PrintRescType();
	m_DispLoc.PrintDispLoc();
	cout << "Staffing: " << m_Staffing << "\n";
	cout << "Shift Start Time: " << m_StartTime << "\n";
	cout << "Shift End Time: " << m_EndTime << "\n";
	cout << "Start Day of Week for Workshift: " << m_StartDayofWeek << "\n";
	cout << "End Day of Week for Workshift: " << m_EndDayofWeek << "\n";
	cout << "Julian Date for Start of Season: " << m_StartSeason << "\n";
	cout << "Julian Date for End of Season: " << m_EndSeason << "\n";
	cout << "Percent Availability: " << m_PctAvail << "\n";
}
	
// Is the fire start time in the resource's normal workshift?  Is Callback delay needed?
bool CResource::IsInWorkshift( CFire fire )
{
	bool isin = false;
	
	// Is the day of week within the work week?
	string firedayofweek = fire.GetDayofWeek();
	int discoveryday = ConvertDay( firedayofweek );
	int startday = ConvertDay( m_StartDayofWeek );
	int endday = ConvertDay( m_EndDayofWeek );

	// if the start day of week is less than the end day of week
	if ( startday < endday ) {
		if ( discoveryday >= startday && discoveryday <= endday )
			isin = true;
		//cout << "Resource ID: " << m_RescID << "  Start Day < End Day  ";
		}
	else	{	// if the start day of week is greater than the end day of week
		if ( discoveryday <= endday || discoveryday >= startday )
			isin = true;
		//cout << "Discovery Day: " << discoveryday << " Start Day: " << startday << " End Day: " << endday << " Resource ID: " << m_RescID << "   Start Day > End Day  ";
	}
	//cout << "IS IN: " << isin << "\n";
	
	// Is the fire discovery time within the workshift hours?
	// Fire discovery time in minutes since midnight
	int discoverytime = fire.FireStartTime();
	
	// Convert resource start and end times to integers
	int starttime = ConvertTime( m_StartTime );
	int endtime = ConvertTime( m_EndTime );

	if ( starttime > discoverytime || endtime < discoverytime )
		isin = false;

	
	return isin;
}

// Is the Julian date in the season for the resource
bool CResource::IsInSeason ( int Julian )
{
	bool IsIn = true;

	if ( m_EndSeason > m_StartSeason )	{

		if ( Julian > m_EndSeason || Julian < m_StartSeason )
			IsIn = false;

	}

	else	{

		if ( Julian < m_EndSeason && Julian > m_StartSeason )
			IsIn = false;

	}

	return IsIn;

}

// Calculate the time remaining in the workshift after resources arrive on fire
int CResource::CalcRemainingWorkshift( CFire fire )
{	

	int FireStartTime = fire.FireStartTime();
	int Julian = fire.GetJulianDay();
	int ShiftUsed = 0;
	
	if ( m_WorkshiftStartTime >= 0 )	{
		// Check if the work rest guidelines have been met since the last deployment for the day
		int prevfiretime = 0;
		int prevresttime = 0;

		//cout << "Previous fire time: " << prevfiretime << " Previous rest time: " << prevresttime << "\n";
		bool metWR = WRGuidelinesMet( FireStartTime, Julian, prevfiretime, prevresttime );
		//cout << "Previous fire time: " << prevfiretime << " Previous rest time: " << prevresttime << "\n";

		if ( metWR )	{
			ShiftUsed = GetInitArrivalTime();

			// Is there a callback delay?  if so exclude this from the initial arrival time ?????????????????
			//if ( !IsInWorkshift( fire ) )	
				//ShiftUsed = ShiftUsed - GetDispLoc().GetCallbackDelay();
		}					

		else
			ShiftUsed = ( FireStartTime + GetInitArrivalTime() ) - GetWorkshiftStartTime();
	}
	else	{
		ShiftUsed = GetInitArrivalTime();
		
		// Is there a callback delay?  if so exclude this from the initial arrival time ????????????????
		//if ( !IsInWorkshift( fire ) )	
			//ShiftUsed = ShiftUsed - GetDispLoc().GetCallbackDelay();
	}					

	int ShiftRemaining = m_WorkshiftLength - ShiftUsed;

	SetDuration( ShiftRemaining );									//save the duration the resource can work the fire
	return ShiftRemaining;
}

// Calculate the time remaining in the workshift after resources arrive on fire using a different arrival time than the one saved in m_InitArrival
int CResource::CalcRemainingWorkshift( CFire fire, int arrival )
{	
	int FireStartTime = fire.FireStartTime();
	int Julian = fire.GetJulianDay();
	int ShiftUsed = 0;

	if ( m_WorkshiftStartTime >= 0 )	{
		// Check if the work rest guidelines have been met since the last deployment for the day
		int prevfiretime = 0;
		int prevresttime = 0;
		bool metWR = WRGuidelinesMet( FireStartTime, Julian, prevfiretime, prevresttime );
		if ( metWR )	{
			ShiftUsed = arrival;

			// Is there a callback delay?  if so exclude this from the initial arrival time ?????????????????
			//if ( !IsInWorkshift( fire ) )	
				//ShiftUsed = ShiftUsed - GetDispLoc().GetCallbackDelay();
		}					

		else
			ShiftUsed = ( FireStartTime + arrival ) - GetWorkshiftStartTime();
	}
	else	{
		ShiftUsed = arrival;
		
		// Is there a callback delay?  if so exclude this from the initial arrival time
		//if ( !IsInWorkshift( fire ) )	
			//ShiftUsed = ShiftUsed - GetDispLoc().GetCallbackDelay();
		
	}					

	int ShiftRemaining = m_WorkshiftLength - ShiftUsed;

	return ShiftRemaining;
}

// Reset the Workshift start time for the resources deployed to the fire
int CResource::AdjustWorkshiftStartTime( CFire fire )
{
	int FireStartTime = fire.FireStartTime();
	int Julian = fire.GetJulianDay();
	int wsstarttime = GetWorkshiftStartTime();							// calculated work shift start time - default needs to be changed
	// If the workshift hasn't started for the fire
	//cout << "Workshift Start Time: " << wsstarttime << "\n";
	if ( wsstarttime < 0 )	{
		wsstarttime = FireStartTime;

		// Is there a competition delay from the last day
		int AvailableTimeDay = GetAvailableTime() - ((Julian-1) * 24 * 60);		// Minutes left since the start of the day
		if ( AvailableTimeDay > FireStartTime )
			wsstarttime = AvailableTimeDay;

		// Is there a callback delay?  if so exclude this from the initial arrival time ???????????????????
		//if ( !IsInWorkshift( fire ) )	
			//wsstarttime = wsstarttime + GetDispLoc().GetCallbackDelay();
	}

	else	{				// The workshift has started so need to determine if the work/rest criteria was met before this fire started
		// Check if the work rest guidelines have been met since the last deployment for the day
		int prevfiretime = 0;
		int prevresttime = 0;
		bool metWR = WRGuidelinesMet( FireStartTime, Julian, prevfiretime, prevresttime );
		if ( metWR )	{
			wsstarttime = FireStartTime;

			// Is there a callback delay?  if so exclude this from the initial arrival time ?????????????????
			//if ( !IsInWorkshift( fire ) )	
				//wsstarttime = wsstarttime + GetDispLoc().GetCallbackDelay();
		}
	}
	
	SetWorkshiftStartTime( wsstarttime );
	return wsstarttime;
}

// Adjust the next available time to account for the work/rest break because the next fire is on a new day
void CResource::AdjustAvailableTime()
{
	// Determine the work time and rest time since from work history
	if ( m_NumWorkEntries > 0 )	{
							
		int j = m_NumWorkEntries-1;
		int Julian = m_VRescWorkYear[j].GetJulianDay();
		int CurStartTimeMSM = m_VRescWorkYear[j].GetEndTime() - (Julian-1) * 24 * 60;
		
		// To accumulate work and rest times
		int prevfiretime = 0;										// fire time for previous fires since last break
		int prevresttime = 0;										// rest time for previous fires since last break
		
		// Determine the previous fire time and previous rest time for the day
		bool result = WRGuidelinesMet( CurStartTimeMSM, Julian, prevfiretime, prevresttime );
			
		// Calculate the next available time to include the needed rest break (add half the fire time minus the rest time beyond the fire's ending time)
		int nextavailabletime = m_VRescWorkYear[ m_NumWorkEntries-1 ].GetEndTime() + static_cast< int >(.5 * prevfiretime + 0.5 ) - prevresttime;
		
		// If the set available time is later than the one calculated from the resource work year use the later time
		if ( nextavailabletime < GetAvailableTime() )
			nextavailabletime = GetAvailableTime();

		SetAvailableFlag( false );									// Indicates that the resource has not timed out yet for this new day
		SetAvailableTime( nextavailabletime );						// Next available time includes rest time
		//cout << "Resource ID: " << m_RescID << " New Day Adjustment: " << m_AvailableTime << "\n";
	}
}

// Reset the next available time at the beginning of the scenario, reset it to the start of the first day the resource is in season
void CResource::ResetAvailableTime()
{	// Get the first day of the resources season
    /*
    ** If we have a huge next available time, we are excluding the resource
    ** from the simulation, leave it.  We want to exclude it for all scenarios.
    */
    if( m_AvailableTime > 525600 * 10 )
        return;

	int StartDay = m_StartSeason;

	// Determine the time in minutes since the beginning of the year for this day
	int StartTime = (StartDay - 1) * 1440 + 1;
	m_AvailableTime = StartTime;

	if ( m_StartSeason > m_EndSeason )
		m_AvailableTime = 0;
}	

// Determine if the resource is beyond the end of it's season and adjust it's available time to beyond the end of the year
void CResource::EndSeasonAvailableTime( int Julian )
{	
	// If this day is before the Julian day sent to this function adjust the Available Time to beyond the end of the year
	if ( m_StartSeason < m_EndSeason )	{

		if ( m_EndSeason < Julian )
			m_AvailableTime = 365 * 1440 + 1;

	}

	if ( m_StartSeason > m_EndSeason )	{

		if ( m_EndSeason < Julian )
			m_AvailableTime = ( m_StartSeason - 1 ) * 1440 + 1;

	}
}

// Determine if two resources are equal as good to deploy based on the initial arrival time, duration of time they can be on the fire and production rate
bool CResource::DeploySelectEqual( CResource *resc2 )
{	bool equal = false;
	// Are the initial arrival times, durations and production rates the same for the resources?
	if ( this->GetInitArrivalTime() == resc2->GetInitArrivalTime() && this->GetDuration() == resc2->GetDuration() && 
		this->GetProdRate() == resc2->GetProdRate() )
		equal = true;

	return equal;
}

// Determine if it is better to deploy the first resource rather than the second resource
bool CResource::DeploySelectCompare( CResource *resc2 )
{	bool thisbetter = false;
	// Is the initial arrival time for the first resource shorter than the second resource?
	if ( this->GetInitArrivalTime() < resc2->GetInitArrivalTime() )
		thisbetter = true;
	else	{
		if ( this->GetInitArrivalTime() == resc2->GetInitArrivalTime() )	{	
			if ( this->GetDuration() > resc2->GetDuration() )
				thisbetter = true;	
			else	{
				if ( this->GetDuration() == resc2->GetDuration() )	{
					if ( this->GetProdRate() > resc2->GetProdRate() )
						thisbetter = true;	}
			}
		}
	}
	
	return thisbetter;
}

// Store the work information for the resource and determine the next time when the resource will next be available based on work/rest ratio
int CResource::DetermineWorkRest( int scenario, int firenum, int julian, int daystarttime, int dayendtime, string effort )
{
	// Determine start time and end time since the beginning of the year
	int starttime = ( julian - 1 ) * 1440 + daystarttime;
	int endtime = ( julian - 1 ) * 1440 + dayendtime;
	int earlieststarttime = starttime;								// Earliest start time this day
	int timeworked = endtime - starttime;							// Time worked on this fire
	bool flag = false;												// Set the work/rest flag to false
		
	// Get previous suppression effort if one exists
	if ( m_VRescWorkYear.empty() )	{
		// Is first fire of the year
		bool flag = true;
		m_VRescWorkYear.push_back( CRescWorkYear( scenario, firenum, julian, starttime, endtime, effort, flag ));
		m_NumWorkEntries  = 1;

		// Determine the next time the resource is available
		// Did this first fire of the year take up the whole workshift?
		if ( timeworked < m_WorkshiftLength )
			return endtime;											
		else	{
			SetAvailableFlag( true );								// Set the available flag to indicate that the resource has timed out for the day 1 indicates the resource has timed out
			return ( static_cast< int >( endtime + 0.5 * (endtime - starttime) + 0.5));		// Include the rest break in the time next available time calculation
		}
	}

	else	{
		// Is not the first fire of the year
		// Was the work/rest guideline of 2 to 1 met between this fire and the previous ones?
		bool CurFlag = false;										// Initial setting for the flag for this entry
		int CurStartTime = starttime;								// CurStartTime is the start time for the j+1 fire
		
		// To store values for previous entries ( since not first entry will need to go back at least one entry)
		int j = m_NumWorkEntries-1;									// Index for previous entry
		int PrevJulian = 0;
		int PrevStartTime = 0;
		int PrevEndTime = 0;
		bool PrevFlag = false;

		// To accumulate work and rest times
		int prevfiretime = 0;										// fire time for previous fires since last break
		int prevresttime = 0;										// rest time for previous fires since last break
		
		// Go back through fires until fire where work/rest guidelines were met before the start of the fire
		while (	!CurFlag )	{										// While the work/rest guideline has not been met (assume it hasn't for the first previous fire)
			PrevJulian = m_VRescWorkYear[ j ].GetJulianDay();
			PrevStartTime = m_VRescWorkYear[ j ].GetStartTime();
			PrevEndTime = m_VRescWorkYear[ j ].GetEndTime();
			PrevFlag = m_VRescWorkYear[ j ].GetFlag();
	
			// Accumulate work and rest times
			prevfiretime = prevfiretime + ( PrevEndTime - PrevStartTime );
			prevresttime = prevresttime + ( CurStartTime - PrevEndTime );
			
			// If the previous fire is on the same day as the current one, set the earliest start time for fires on the day
			if ( julian == PrevJulian )
				earlieststarttime = PrevStartTime;					// earliest start time for the resource following last work/rest break
			j--;

			CurFlag = PrevFlag;
			CurStartTime = PrevStartTime;
		}

		// Compare work time and rest time to see if work/rest guidelines have been met
		//cout << "Resource ID: " << m_RescID << " Previous Work Time: " << prevfiretime << " Previous Rest Time: " << prevresttime << "\n";
		if ( static_cast< double >(prevresttime) >= 0.5 * static_cast< double >( prevfiretime ))	{
			flag = true;
			earlieststarttime = starttime;					// reset the earliest start time for the resource following last work/rest break
		}
		
		// Store the fire suppression effort 
		m_VRescWorkYear.push_back( CRescWorkYear( scenario, firenum, julian, starttime, endtime, effort, flag ) );
		m_NumWorkEntries++;

		// Determine next available time for the resource
		// Is the time since the last work/rest break for the day greater than the workshift length?
		timeworked = endtime - earlieststarttime;
		if ( timeworked < m_WorkshiftLength )
			return endtime;											
		else	{
			SetAvailableFlag( true );								// Set the available flag to indicate that the resource has timed out for the day 1 indicates that the resource has timed out
			
			prevfiretime = prevfiretime + endtime - starttime;		// Add the latest piece of fire suppression effort to the time

			int nextavailtime = static_cast< int >( endtime + 0.5 * prevfiretime + 0.5 );
			//cout << "Resource ID: " << GetRescID() << " Available Time after reaching workshift length: " << nextavailtime << "\n";
			return nextavailtime;									// Include the rest break in the time next available time calculation
			
		}
	}
}

// Print the information about a resources work so far this year
void CResource::PrintRescWorkInfo()
{
	for ( int i=0; i < static_cast< int >(m_VRescWorkYear.size()); i++ )	{
		cout << "Entry Number: " << i << " Julian Day: " << m_VRescWorkYear[i].GetJulianDay() << " Start Time: " 
			<< m_VRescWorkYear[i].GetStartTime() << " End Time: " << m_VRescWorkYear[i].GetEndTime() << " Effort: " 
			<< m_VRescWorkYear[i].GetEffort() << " Met Work/Rest Flag: " << m_VRescWorkYear[i].GetFlag() << "\n";
	}
}

// Get the size of the work year vector
int CResource::WorkYearSize()
{	return m_VRescWorkYear.size();	}

// Get a the ith element of the resources work year vector
CRescWorkYear CResource::GetWorkYearEntry( int i )
{	return m_VRescWorkYear[i];	}

// Determine if the work rest guidelines have been met between the end of the last fire and the start time for this fire
bool CResource::WRGuidelinesMet( int firestarttimeMSM, int julian, int &prevfiretime, int &prevresttime )
{
	// If there are no entries in the workrest vector 
	if ( m_VRescWorkYear.size() == 0 )
		return false;
	
	// Was the work/rest guideline of 2 to 1 met between this fire and the previous ones?
	bool CurFlag = false;											// Initial setting for the flag for this entry
	int CurStartTime = firestarttimeMSM + (julian-1) * 24 * 60;		// CurStartTime is the start time for the j+1 fire
		
	// To store values for previous entries ( since not first entry will need to go back at least one entry)
	int j = m_NumWorkEntries-1;										// Index for previous entry
	int PrevJulian = 0;
	int PrevStartTime = 0;
	int PrevEndTime = 0;
	bool PrevFlag = false;
	
	// Go back through fires until fire where work/rest guidelines were met before the start of the fire
	while (	!CurFlag )	{											// While the work/rest guideline has not been met (assume it hasn't for the first previous fire)
		PrevJulian = m_VRescWorkYear[ j ].GetJulianDay();
		PrevStartTime = m_VRescWorkYear[ j ].GetStartTime();
		PrevEndTime = m_VRescWorkYear[ j ].GetEndTime();
		PrevFlag = m_VRescWorkYear[ j ].GetFlag();
	
		// Accumulate work and rest times
		prevfiretime = prevfiretime + ( PrevEndTime - PrevStartTime );
		prevresttime = prevresttime + ( CurStartTime - PrevEndTime );
			
		j--;

		CurFlag = PrevFlag;
		if ( j < 0 )
			CurFlag = true;											// do not allow the vector index to get below 0
		
		CurStartTime = PrevStartTime;
	}
	
	// Compare work time and rest time to see if work/rest guidelines have been met
	//cout << "Resource ID: " << m_RescID << " Previous Work Time: " << prevfiretime << " Previous Rest Time: " << prevresttime << "\n";
	if ( static_cast< double >(prevresttime) >= 0.5 * static_cast< double >( prevfiretime ))	
		return true;
	else
		return false;
}

// Clear the information in the resource work year vector
void CResource::ClearRescWorkYear()
{
	m_VRescWorkYear.clear();
	m_NumWorkEntries = m_VRescWorkYear.size();
	//cout << "Number of WorkEntries: " << m_NumWorkEntries << "\n";
}

int CResource::SumRescWorkYear()
{
	int sum = 0;
	for (int i = 0; i < m_VRescWorkYear.size(); i++ )
		sum += m_VRescWorkYear[i].GetEndTime() - m_VRescWorkYear[i].GetStartTime();

	return sum;
}

void CResource::SetDbId( int nDbId )
{
    this->nDbId = nDbId;
#ifdef IRS_USE_DBID_AS_RESC_NAME
    char pszDb[512];
    sprintf( pszDb, "%d", nDbId );
    SetRescID( pszDb );
#endif
}

