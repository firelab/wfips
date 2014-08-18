// Fire.cpp
// CFire member function definitions for individual fires
// 10/11 for IRS

#include <iostream>											//contains functions for input and output
using namespace std;
#include <string>											//contains functions for operations with strings
using std::string;
#include <cstring>											//to convert a string to a const char*
#include <cstdlib>											//to use atof to convert const char* to int

#include "Fire.h"											//definition file for CFire class
#include "FWA.h"											//definition file for CFWA class


// define and initialize static data member
int CFire::count = 0;

// CFire constructor
CFire::CFire( int scenario, int firenum, int julianday, string dayofweek, string discoverytime, int bi, double ros, int fuelmodel,
		string spcondition, int slopepct, bool iswalkin, string tactic, double distance, int elevation, double ltowratio, int minnumsteps,
		int maxnumsteps, string sunrise, string sunset, bool waterdrops, bool pumpnroll, CFWA &fwa, double lat, double lon ) : m_FWA( fwa )
{
	m_Scenario = scenario;											//initialize the fire's scenario
	m_FireNumber = firenum;											//initialize the fire number
	m_JulianDay = julianday;										//initialze the julian day of the fire discovery
	SetDayofWeek( dayofweek );										//initialize the day of week for the fire
	SetDiscoveryTime( discoverytime );								//initialize the discoverytime for the fire
	m_BI = bi;														//initialize the burning index for the fire
	m_ROS = ros;													//initialize the rate of spread for the fire
	m_FuelModel = fuelmodel;										//initialize the fuel model for the fire
	m_SpecificCondition = spcondition;								//initialize the specific condition for the fire
	m_SlopePct = slopepct;											//initialize the slope percent for the fire
	m_IsWalkIn = iswalkin;											//initialize whether or not the fire is walkin
	SetTactic( tactic );											//initialize the tactic: HEAD, TAIL, or PARALLEL
	m_Distance = distance;											//initialize the distance the fireline is built from the fire perimeter
	m_Elevation = elevation;										//initialize the elevation of the fire
	SetLtoWRatio( ltowratio );										//initialize the length to width ratio
	SetMinNumSteps( minnumsteps );									//initialize the minimum number of steps
	SetMaxNumSteps( maxnumsteps );									//initialize the maximum number of steps
	m_SunRise = sunrise;											//initialize the time of sunrise
	m_SunSet = sunset;												//initialize the time of sunset
	m_WaterDrops = waterdrops;										//initialize whether or not water drops are allowed
	m_IsPumpNRoll = pumpnroll;										//initialize whether engines on fire use pump and roll methods

        m_lat = lat;
        m_lon = lon;

	m_WTArrival = 10000;												//arrival time for the first water tender to the fire

	count++;														//increase the count for the number of fires
}

// Copy Constructor
CFire::CFire( const CFire&thatfire ) : m_FWA( thatfire.m_FWA )
{
	SetMemVars( thatfire );

	count++;
}

// Destructor for CFire
CFire::~CFire() {}

// Assignment operator for CFire
CFire& CFire::operator=( const CFire &thatfire )
{
	SetFWA( thatfire.m_FWA );										//set fire fwa
	// Set the member variables 
	SetMemVars( thatfire );

	return *this;
}

// Set the member variables
CFire& CFire::SetMemVars( const CFire &thatfire )
{
	m_Scenario = thatfire.m_Scenario;								//set the fire scenario
	m_FireNumber = thatfire.m_FireNumber;							//set the fire number
	m_JulianDay = thatfire.m_JulianDay;								//set the julian day of the fire discovery
	SetDayofWeek( thatfire.m_DayofWeek );							//set the day of week for the fire
	SetDiscoveryTime( thatfire.m_DiscoveryTime );					//set the discoverytime for the fire
	m_BI = thatfire.m_BI;											//set the burning index for the fire
	m_ROS = thatfire.m_ROS;											//set the rate of spread for the fire
	m_FuelModel = thatfire.m_FuelModel;								//set the fuel model for the fire
	m_SpecificCondition = thatfire.m_SpecificCondition;				//set the specific condition for the fire
	m_SlopePct = thatfire.m_SlopePct;								//set the slope percent for the fire
	m_IsWalkIn = thatfire.m_IsWalkIn;								//set whether or not the fire is walkin
	SetTactic( thatfire.m_Tactic );									//set the tactic: HEAD, TAIL, or PARALLEL
	m_Distance = thatfire.m_Distance;								//set the distance the fireline is built from the fire perimeter
	m_Elevation = thatfire.m_Elevation;								//set the elevation of the fire
	SetLtoWRatio( thatfire.m_LtoWRatio );							//set the length to width ratio
	SetMinNumSteps( thatfire.m_MinNumSteps );						//set the minimum number of steps
	SetMaxNumSteps( thatfire.m_MaxNumSteps );						//set the maximum number of steps
	m_SunRise = thatfire.m_SunRise;									//set the time of sunrise
	m_SunSet = thatfire.m_SunSet;									//set the time of sunset
	m_WaterDrops = thatfire.m_WaterDrops;							//set whether or not water drops are allowed
	m_IsPumpNRoll = thatfire.m_IsPumpNRoll;							//set whether engines on fire use pump and roll methods
	m_WTArrival = thatfire.m_WTArrival;								//arrival time for the first water tender to the fire
        m_lat = thatfire.m_lat;
        m_lon = thatfire.m_lon;
        m_SimulateContain = thatfire.m_SimulateContain;
        m_Treated = thatfire.m_Treated;

	return *this;
}

// Set the fire's scenario
void CFire::SetScenario( int scenario )
{	m_Scenario = scenario;	}

// Get the fire's scenario
int CFire::GetScenario()
{	return m_Scenario;	}

// Set fire number
void CFire::SetFireNumber( int firenum )
{	m_FireNumber = firenum;	}

// Get fire number
int CFire::GetFireNumber()
{	return m_FireNumber;	}

// Set the julian day of the fire discovery
void CFire::SetJulianDay( int julianday )
{	m_JulianDay = julianday;	}

// Get the julian day of the fire discovery
int CFire::GetJulianDay()
{	return m_JulianDay;	}

// Set the day of week for the fire
void CFire::SetDayofWeek( string dayofweek)
{	
	// Check if valid end day for workweek
	if (dayofweek != "Monday" && dayofweek != "Tuesday" && dayofweek != "Wednesday" && dayofweek != "Thursday" &&
		dayofweek != "Friday" && dayofweek != "Saturday" && dayofweek != "Sunday") dayofweek = "Monday";

	m_DayofWeek = dayofweek;	
}

// Get the day of week for the fire
string CFire::GetDayofWeek()
{	return m_DayofWeek;	}

// Set the discovery time for the fire
void CFire::SetDiscoveryTime( string discoverytime )
{
	// Pad the front of the discovery time if there are not 4 digets
	int len = discoverytime.length();
	if (len != 4)	{
		int pad = 4 - len;
		discoverytime.insert( 0, pad, '0' );
	}

	string hr = discoverytime.substr(0,2);							//get the hour portion of the string
	const char* hrcstr = hr.c_str();								//cast to const char 
	double hrnum = atof( hrcstr );									//so can use atof to convert to integer
	if (hrnum > 24.0 || hrnum < 0.0) discoverytime = "1200";		//if hour not between 00 and 24 invalid, set to 1200
	
	string min = discoverytime.substr(2,2);							//get the minute portion of the string
	const char* mincstr = min.c_str();								//cast to const char
	double minnum = atof( mincstr );								//so can use atof to convert to integer
	if (minnum > 60.0 || minnum < 0.0) discoverytime = "1200";		//if minute not between 00 and 60 invalid, set to 1200
	
	m_DiscoveryTime = discoverytime;
}

// Get the discovery time for the fire
string CFire::GetDiscoveryTime()
{	return m_DiscoveryTime;	}

// Set the BI for the fire
void CFire::SetBI( int bi )
{	m_BI = bi;	}

// Get the BI for the fire
int CFire::GetBI()
{	return m_BI;	}

// Set the ROS for the fire
void CFire::SetROS( double ros)
{	m_ROS = ros;	}

// Get the ROS for the fire
double CFire::GetROS()
{	return m_ROS;	}

// Set the fuel model at the fire
void CFire::SetFuelModel( int fuelmodel )
{	m_FuelModel = fuelmodel;	}

//Get the fuel model at the fire
int CFire::GetFuelModel()
{	return m_FuelModel;	}

// Set the specific condition for the fuel model
void CFire::SetSpecificCondition( string spcondition )
{	m_SpecificCondition = spcondition;	}

// Get the specific condition for the fuel model
string CFire::GetSpecificCondition()
{	return m_SpecificCondition;	}

// Set the percent slope at the fire
void CFire::SetSlopePct( int slopepct)
{	m_SlopePct = slopepct;	}

// Get the percent slope at the fire
int CFire::GetSlopePct()
{	return m_SlopePct;	}

// Set FWA pointer for fire
void CFire::SetFWA( CFWA &fwa )
{	m_FWA = fwa;	}

// Get pointer to FWA that fire is in
CFWA& CFire::GetFWA()
{	return m_FWA;	}

// Set if fire is Walk-in
void CFire::SetIsWalkIn( bool walkin )
{	m_IsWalkIn = walkin;	}

// Get if fire is walk-in
bool CFire::GetIsWalkIn()
{	return m_IsWalkIn;	}

// Set Tactic - HEAD, TAIL, or PARALLEL
void CFire::SetTactic( string tactic )
{	if ( tactic != "HEAD" && tactic != "TAIL" && tactic != "PARALLEL" )
		tactic = "HEAD";
		m_Tactic = tactic;	}

// Get Tactic - HEAD, TAIL, or PARALLEL
string CFire::GetTactic()
{	return m_Tactic;	}

// Set Distance fireline is built from fire's perimeter
void CFire::SetDistance( double distance )
{	if ( distance < 0 )
		distance = 0;
	m_Distance = distance;	}

// Get Distance fireline is built from fire's perimeter
double CFire::GetDistance()
{	return m_Distance;	}

// Set Elevation of fire
void CFire::SetElevation( int elevation )
{	if ( elevation < 0 )
		elevation = 0;
	m_Elevation = elevation;	}

// Get Elevation of fire
int CFire::GetElevation ()
{	return m_Elevation;	}

// Set the Length to Width Ratio
void CFire::SetLtoWRatio( double ltowratio )
{	m_LtoWRatio = ltowratio;	}

// Get the Length to Width Ratio
double CFire::GetLtoWRatio()
{	return m_LtoWRatio;	}

// Set the Minimum number of steps to use in the containment algorithm
void CFire::SetMinNumSteps( int minnumsteps )
{	if ( minnumsteps < 250 )
		minnumsteps = 250;
	m_MinNumSteps = minnumsteps;	}

// Get the Minimum number of steps to use in the containment algorithm
int CFire::GetMinNumSteps()
{	return m_MinNumSteps;	}

// Set the Maximum number of steps to use in the containment algorithm
void CFire::SetMaxNumSteps( int maxnumsteps )
{	int min = GetMinNumSteps();
	if ( maxnumsteps < min + 1 )
		maxnumsteps = 25000;
	m_MaxNumSteps = maxnumsteps;	}

// Get the maximum number of steps to use in the containment algorithm
int CFire::GetMaxNumSteps()
{	return m_MaxNumSteps;	}

// Set the time for sunrise
void CFire::SetSunRise( string sunrise )
{	m_SunRise = sunrise;	}

// Get the time for sunrise
string CFire::GetSunRise()
{	return m_SunRise;	}

// Set the time for sunset
void CFire::SetSunSet( string sunset )
{	m_SunSet = sunset;	}

// Get the time for sunset
string CFire::GetSunSet()
{	return m_SunSet;	}

// Set whether or not water drops are allowed on the fire
void CFire::SetWaterDropsAllowed( bool waterdrops )
{	m_WaterDrops = waterdrops;	}

// Get whether or not water drops are allowed on the fire
bool CFire::GetWaterDropsAllowed()
{	return m_WaterDrops;	}

// Set whether or not the fire is pump and roll
void CFire::SetIsPumpNRoll( bool pumpnroll )
{	m_IsPumpNRoll = pumpnroll;	}

// Get whether or not the fire is pump and roll
bool CFire::GetIsPumpNRoll()
{	return m_IsPumpNRoll;	}

// Set the latitude
void CFire::SetLatitude( double latitude )
{	m_lat = latitude;	}

// Get the latitude
double CFire::GetLatitude()
{	return m_lat;	}

// Set longitude
void CFire::SetLongitude( double longitude )
{	m_lon = longitude;	}

// Get longitude
double CFire::GetLongitude()
{	return m_lon;	}

// Set the time for the first water tender arrival to the fire
void CFire::SetWTArrivalTime( int arrival )
{	m_WTArrival = arrival;	}

// Get the time for the first water tender arrival to the fire
int CFire::GetWTArrivalTime()
{ return m_WTArrival;	}

// Calculate the time, 4 chr, to minutes since midnight
int CFire::ConvertTimeMSM( string time )
{	
	// If length is not 4 prepad with 0s
	if ( time.length() < 4 )	{
		int NumZeros = 4 - time.length();

		for ( int i = 0; i < NumZeros; i++ )	
			time.insert( 0, "0" );
	}
	string hr = time.substr(0,2);									//get the hour portion of the string
	const char* hrcstr = hr.c_str();								//cast to const char 
	double hrnum = atof( hrcstr );									//so can use atof to convert to integer			

	string min = time.substr(2,2);									//get the minute portion of the string
	const char* mincstr = min.c_str();								//cast to const char
	double minnum = atof( mincstr );								//so can use atof to convert to integer
	
	double timeMSM = hrnum * 60 + minnum;							//minutes since midnight for fire's discoverytime
	return static_cast< int>( timeMSM );
}

// Convert discovery time to minutes since midnight
int CFire::FireStartTime()
{	return ConvertTimeMSM( GetDiscoveryTime());	}

// Convert sunrise to minutes since midnight
int CFire::SunriseMSM()
{	return ConvertTimeMSM( GetSunRise());	}

// Convert sunset to minutes since midnight
int CFire::SunsetMSM()
{	return ConvertTimeMSM( GetSunSet());	}

// Get the count for the number of resources instantiated
int CFire::GetCount()
{	return count;	}

// Print information about fire
void CFire::PrintFireInfo()
{
	cout << "Fire Number: " << m_FireNumber << "\n";
	cout << "Julian Day:" << m_JulianDay << "\n";
	cout << "Fire Day of Week: " << m_DayofWeek << "\n";
	cout << "Fire Discovery Time: " << m_DiscoveryTime << "\n";
	cout << "Burning Index: "	<< m_BI << "\n";
	cout << "Rate of Spread: " << m_ROS << "\n";
	cout << "Fuel Model: " << m_FuelModel << "\n";
	cout << "Specific Condition: " << m_SpecificCondition << "\n";
	cout << "Slope Percent: " << m_SlopePct << "\n";
	cout << "Is Walk In? " << m_IsWalkIn << "\n";
	cout << "Tactic: " << m_Tactic << "\n";
	cout << "Attack Distance: " << m_Distance << "\n";
	cout << "Elevation: " << m_Elevation << "\n";
	cout << "Length to Width Ratio: " << m_LtoWRatio << "\n";
	cout << "Minimum number of Steps: " << m_MinNumSteps << "\n";
	cout << "Maximum number of Steps: " << m_MaxNumSteps << "\n";
	cout << "Sun Rise: " << m_SunRise << "\n";
	cout << "Sun Set: " << m_SunSet << "\n";
	cout << "Water Drops Allowed: " << m_WaterDrops << "\n";
	cout << "Pump and Roll Tactic: " << m_IsPumpNRoll << "\n";
	
	m_FWA.PrintFWAInfo();
}

// Calculate the size of a fire at a given time since the fire's discovery time without any containment
double CFire::CalcFBFireSize( int time )
{
	int starttime = FireStartTime();								// Fire start time in minutes since midnight
	// If time is less than 0, return 0;
	if ( time < 0 )
		return 0;
	
	const double PI = 3.141592;
	double size;													// The size of the fire in acres at a given time since the fire started
	double head;													// The head distance of the fire
	double A;														// length of major axis of ellipse
	double C;														// distance from center to focus of ellipse
	double B;														// length of minor axis of ellipse
	double LtoWRatio = GetLtoWRatio();								// length to width ratio for the ellipse

	// Calcualate the fire head distance
	// Calculate the length of the fire head at the fire's discovery
	double chsq = 10 * GetFWA().GetDiscoverySize();					// Discovery Size of the fire in square chains
	A = sqrt( chsq * LtoWRatio /  PI );	
	B = A / LtoWRatio;
	C = sqrt ( (A * A) - (B * B) );
	head = A + C;													// Length of head at discovery time
	//cout << "A: " << A << " B: " << B << " C: " << C << " Head: " << head << " Time: " << time << "\n";


	// Add the distance the head travels until the specified time 
        if(starttime == 1440)
            starttime--;
	int hour = starttime / 60;
        assert( hour < 24 );
	double diurnalcoeff = GetFWA().GetDiurnalCoefficient( hour );
	double firesROS = GetROS();										// Original ROS for the fire
	CFWA& FWA = GetFWA();											// Get the FWA the fire is in
	int fuelmodel = GetFuelModel();									// Get the fuel model for the fire
	firesROS = FWA.FuelModelAdjROS( fuelmodel, firesROS );			// Adjust for ROS for the FWA fuel model adjustment factor
	double hoursROS = firesROS * diurnalcoeff;						// ROS for the fire at discovery time in chains per hour
	int min = 60 - starttime % 60;
	//cout << "Hour: " << hour << " Minutes: " << min << "\n";
	// Increment the hours until there is no longer a whole hour left in the time
	while	( min <= time )	{
		head = head + ( static_cast< double >( min ) / 60.0 ) * hoursROS;	// Calculate the ROS growth for the head for the rest of this hour
		time = time - min;											// Subtract the time accounted for by the growth
		min = 60;													// From now on the minutes in the hour are 60
		hour++;									
		if ( hour >= 24 )											// If next day start diurnal coefficients over again
			hour = 0;
                assert( hour < 24 );
		diurnalcoeff = GetFWA().GetDiurnalCoefficient( hour );
		hoursROS = firesROS * diurnalcoeff;

		//cout << "Time: " << time << " Head: " << head << "\n";
	}

	if ( time > 0 )
		head = head + ( static_cast< double >( time ) / 60.0 ) * hoursROS;						// Add the head growth for the final hour

	// Calculate the size of the fire in acres based on the fire's head
	double factor = sqrt ( 1 - ( 1 / ( LtoWRatio * LtoWRatio) ) );
	factor = factor + 1;
	A = head / factor;
	B = A / LtoWRatio;

	size = PI * A * B;												// Area of fire in square chains
	size = size / 10.0;												// Area of fire in acres

	//cout << "A: " << A << " B: " << B << " C: " << C << "\n";
	//cout << "Final Head: " << head << " Size: " << size << "\n";
		
	return size;
}
