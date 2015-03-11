// FWA.cpp
// CFWA member function definitions for individual FWAs
// 10/11 for IRS

#include <assert.h>

#include <iostream>											//contains functions for input and output
using namespace std;
#include <string>											//contains functions for operations with strings
using std::string;
#include <cstring>											//to convert a string to a const char*
#include <cstdlib>											//to use atof to convert const char* to int
#include <vector>

//#include "Fire.h"											//definition file for CFire class
#include "FWA.h"											//definition file for CFWA class
#include "DispLogic.h"										//definition file for CDispLogic class


// define and initialize static data member
int CFWA::count = 0;

// Default constructor
///CFWA::CFWA()
//{
//	m_FWAID = "";
//	m_Distance = 0;	
//	m_WalkInDelay = 0;
//	m_WalkInPct = 0;
//}

// CFWA constructor
CFWA::CFWA( string FWAid, string FMG, int walkinpct, int pumpnroll, int head, int tail, int parallel, int attackdist,
		bool waterdrops, bool excluded, double discsize, int esltime, int eslsize, double airtoground, int walkindelay[6],
		int postcontainused[6], int postcontainunused[6], int postescape[6], int reloaddelay[5], int firstunitdelay, 
		double diurnalcoeffs[24], string ROSadjfuel[10], double ROSadjustment[10], int index,  CDispLogic &displogic, string FPU,
		double Latitude, double Longitude ) : m_DispLogic( displogic )
{
	m_FWAID = FWAid;
	m_FMG = FMG;
	m_WalkInPct = walkinpct;
	m_PumpNRollPct = pumpnroll;
	m_Head = head;
	m_Tail = tail;
	m_Parallel = parallel;
	m_AttackDist = attackdist;
	m_WaterDrops = waterdrops;
	m_Excluded = excluded;
	m_DiscoverySize = discsize;
	m_ESLTime = esltime;
	m_ESLSize = eslsize;
	m_AirtoGround = airtoground;
	for ( int i=0; i<6; i++ )
		m_WalkInDelays.push_back( walkindelay[i] );
	for ( int i=0; i<6; i++ )
		m_PostContainedUsed.push_back( postcontainused[i] );
	for ( int i=0; i<6; i++ )
		m_PostContainedUnused.push_back( postcontainunused[i] );
	for ( int i=0; i<6; i++ )
		m_PostEscape.push_back( postescape[i] );
	for ( int i=0; i<5; i++ )
		m_ReloadDelay.push_back( reloaddelay[i] );
	m_FirstUnitDelay = firstunitdelay;
	for ( int i=0; i<24; i++ )
		m_DiurnalCoeffs.push_back( diurnalcoeffs[i] );
	for ( int i=0; i<10; i++ )
		m_ROSAdjFuel.push_back( ROSadjfuel[i] );
	for ( int i=0; i<10; i++ )
		m_ROSAdjustment.push_back( ROSadjustment[i] );
	m_Index = index;
	m_FPU = FPU;
	m_Latitude = Latitude;
	m_Longitude = Longitude;
	m_ClosestATBase = "";
	
	count++;
}

// Destructor for CFWA
CFWA::~CFWA() {}

// CFWA copy constructor
CFWA::CFWA( const CFWA &fwa )  : m_DispLogic( fwa.m_DispLogic )
{
	m_FWAID = fwa.m_FWAID;
	m_FMG = fwa.m_FMG;
	m_WalkInPct = fwa.m_WalkInPct;
	m_PumpNRollPct = fwa.m_PumpNRollPct;
	m_Head = fwa.m_Head;
	m_Tail = fwa.m_Tail;
	m_Parallel = fwa.m_Parallel;
	m_AttackDist = fwa.m_AttackDist;
	m_WaterDrops = fwa.m_WaterDrops;
	m_Excluded = fwa.m_Excluded;
	m_DiscoverySize = fwa.m_DiscoverySize;
	m_ESLTime = fwa.m_ESLTime;
	m_ESLSize = fwa.m_ESLSize;
	m_AirtoGround = fwa.m_AirtoGround;
	m_WalkInDelays = fwa.m_WalkInDelays;
	m_PostContainedUsed = fwa.m_PostContainedUsed;
	m_PostContainedUnused = fwa.m_PostContainedUnused;
	m_PostEscape = fwa.m_PostEscape;
	m_ReloadDelay = fwa.m_ReloadDelay;
	m_FirstUnitDelay = fwa.m_FirstUnitDelay;
	m_DiurnalCoeffs = fwa.m_DiurnalCoeffs;
	m_ROSAdjFuel = fwa.m_ROSAdjFuel;
	m_ROSAdjustment = fwa.m_ROSAdjustment;
	m_Index = fwa.m_Index;
	m_FPU = fwa.m_FPU;
	m_Latitude = fwa.m_Latitude;
	m_Longitude = fwa.m_Longitude;
	m_ClosestATBase = fwa.m_ClosestATBase;
	m_AssocMap = fwa.m_AssocMap;

}

// Define assignment operator (=) for CFWA
CFWA &CFWA::operator=( const CFWA &fwa )
{
	if ( &fwa != this )		{
		m_FWAID = fwa.m_FWAID;
		m_FMG = fwa.m_FMG;
		m_WalkInPct = fwa.m_WalkInPct;
		m_PumpNRollPct = fwa.m_PumpNRollPct;
		m_Head = fwa.m_Head;
		m_Tail = fwa.m_Tail;
		m_Parallel = fwa.m_Parallel;
		m_AttackDist = fwa.m_AttackDist;
		m_WaterDrops = fwa.m_WaterDrops;
		m_Excluded = fwa.m_Excluded;
		m_DiscoverySize = fwa.m_DiscoverySize;
		m_ESLTime = fwa.m_ESLTime;
		m_ESLSize = fwa.m_ESLSize;
		m_AirtoGround = fwa.m_AirtoGround;
		m_WalkInDelays = fwa.m_WalkInDelays;
		m_PostContainedUsed = fwa.m_PostContainedUsed;
		m_PostContainedUnused = fwa.m_PostContainedUnused;
		m_PostEscape = fwa.m_PostEscape;
		m_ReloadDelay = fwa.m_ReloadDelay;
		m_FirstUnitDelay = fwa.m_FirstUnitDelay;
		m_DiurnalCoeffs = fwa.m_DiurnalCoeffs;
		m_ROSAdjFuel = fwa.m_ROSAdjFuel;
		m_ROSAdjustment = fwa.m_ROSAdjustment;
		m_DispLogic = fwa.m_DispLogic;
		m_Index = fwa.m_Index;
		m_FPU = fwa.m_FPU;
		m_Latitude = fwa.m_Latitude;
		m_Longitude = fwa.m_Longitude;
		m_ClosestATBase = fwa.m_ClosestATBase;
		m_AssocMap = fwa.m_AssocMap;
	}

	return *this;	// enables x=y=z, for example
}

// Set FWA id
void CFWA::SetFWAID( string fwaid )
{	m_FWAID = fwaid;	}

// Get FWA Id
string CFWA::GetFWAID() const
{	return m_FWAID;	}

// Set the FMG that contains the FWA
void CFWA::SetFMG( string fmg )
{	m_FMG = fmg;	}

// Get the FMG that contains the FWA
string CFWA::GetFMG()
{	return m_FMG;	}

// Set percent walkin fires for FWA
void CFWA::SetWalkInPct( int walkinpct )
{	
	if ( walkinpct < 0 )
		walkinpct = 0;
	if ( walkinpct > 100 )
		walkinpct = 100;
	m_WalkInPct = walkinpct;
}

// Get percent walkin fires for FWA
int CFWA::GetWalkInPct()
{	return m_WalkInPct;	}

// Set the percent pump and roll fires in the FWA
void CFWA::SetPumpnRoll( int pumpnroll) 
{
	if ( pumpnroll < 0 )
		pumpnroll = 0;
	if ( pumpnroll > 100 )
		pumpnroll = 100;
	m_PumpNRollPct = pumpnroll;
}

// Get the percent pump and roll fires in the FWA
int CFWA::GetPumpnRoll()
{	return m_PumpNRollPct;	}

// Set the percentages for the types of fire attack Head, Tail, and Parallel
void CFWA::SetFireAttack( int head, int tail, int parallel )
{
	if ( head > 100 )	
		head = 100;
	
	if ( head + tail > 100 )	
		tail = 100 - head;
	
	if ( head + tail + parallel != 100 )
		parallel = 100 - head - tail;
			
	m_Head = head;
	m_Tail = tail;
	m_Parallel = parallel;
}

// Get the percent of head fires
int CFWA::GetHead()
{	return m_Head;	}

// Get the percent tail fires
int CFWA::GetTail()
{	return m_Tail;	}

// Get the percent parallel fires
int CFWA::GetParallel()
{	return m_Parallel;	}

// Set the attack distance from the fire for the FWA
void CFWA::SetAttackDist( double attackdist )
{	if ( attackdist < 0 )
		attackdist = 0;
	m_AttackDist = attackdist;	}

// Get the attack distance from the fire for the FWA
double CFWA::GetAttackDist()
{	return m_AttackDist;	}

// Set whether water or retardant drops are allowed in the FWA
void CFWA::SetWaterDrops( bool waterdrops )
{	m_WaterDrops = waterdrops;	}

// Get whether water or retardant drops are allowed in the FWA
bool CFWA::GetWaterDrops()
{ return m_WaterDrops;	}

// Set whether the FWA is excluded from the analysis
void CFWA::SetExcluded( bool excluded )
{	m_Excluded = excluded;	}

// Get whether the FWA is excluded from the analysis
bool CFWA::GetExcluded()
{	return m_Excluded;	}

// Set the discovery size for fires in the FWA
void CFWA::SetDiscoverySize( double discsize )
{	if ( discsize < .01 )
		discsize = .01;
	m_DiscoverySize = discsize;	}

// Get the discovery size for fires in the FWA
double CFWA::GetDiscoverySize()
{	return m_DiscoverySize;	}

// Set the escape time limit for fires in the FWA
void CFWA::SetESLTime( int esltime )
{	if ( esltime < 0 )
		esltime = 0;
	m_ESLTime = esltime;	}

// Get the escape time limit for fires in the FWA
int CFWA::GetESLTime()
{	return m_ESLTime;	}

// Set the escape size limit for fires in the FWA
void CFWA::SetESLSize( int eslsize )
{	if ( eslsize < 0 )
		eslsize = 0;
	m_ESLSize = eslsize;	}

// Get the escape size limit for fires in the FWA
int CFWA::GetESLSize()
{	return m_ESLSize;	}

// Set the travel distance ratio for ground vs air travel in the FWA
void CFWA::SetAirtoGround( double airtoground )
{	
	if ( airtoground < 0 )
		airtoground = 1;
	m_AirtoGround = airtoground;
}

// Get the travel distance ratio for ground vs air travel in the FWA
double CFWA::GetAirtoGround()
{	return m_AirtoGround;	}

// Set all the walk-in delays for the FWA in the vector 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
void CFWA::SetWalkInDelays( int walkindelays[6] )
{	for ( int i = 0; i < 6; i++ )	{
		if ( walkindelays[i] < 0 )
			walkindelays[i] = 0;	
		m_WalkInDelays.push_back( walkindelays[i] );
	}
}

// Set a walk-in delay in the vector of walk-in delays for the FWA
void CFWA::SetWalkInDelay( int i, int walkindelay )
{	if ( i >= 0 && i < 6 )
		m_WalkInDelays[i] = walkindelay;	}

// Get a walk-in delay from the vector of walk-in delays for the FWA
int CFWA::GetWalkInDelay( int i )
{	
	// Some resources do not have a delay Airtankers, seats, scoopers, helicopters and watertenders.  Their delay index is -1
	if ( i > 0  && i < 6 )
		return m_WalkInDelays[i];	
	else
		return 0;
}

// Set all the post contain used delays for the FWA in the vector 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
void CFWA::SetPostContUsedDelays( int postdelay[6] )
{	for ( int i = 0; i < 6; i++ )	{
		if ( postdelay[i] < 0 )
			postdelay[i] = 0;	
		m_PostContainedUsed.push_back( postdelay[i] );
	}
}

// Set a post contain used delay in the vector of post contain used delays for the FWA
void CFWA::SetPostContUsedDelay( int i, int postdelay )
{	if ( i >= 0 && i < 6 )
		m_PostContainedUsed[i] = postdelay;	}

// Get a post contain used delay from the vector of post contain used delays for the FWA
int CFWA::GetPostContUsedDelay( int i )
{	
	// Some resources do not have a delay Airtankers, seats, scoopers, helicopters and watertenders.  Their delay index is -1
	if ( i > 0  && i < 6 )
		return m_PostContainedUsed[i];
	else
		return 0;
}

// Set all the post contain unused delays for the FWA in the vector 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
void CFWA::SetPostContUnusedDelays( int postdelay[6] )
{	for ( int i = 0; i < 6; i++ )	{
		if ( postdelay[i] < 0 )
			postdelay[i] = 0;	
		m_PostContainedUnused.push_back( postdelay[i] );
	}
}

// Set a post contain unused delay in the vector of post contain unused delays for the FWA
void CFWA::SetPostContUnusedDelay( int i, int postdelay )
{	if ( i >= 0 && i < 6 )
		m_PostContainedUnused[i] = postdelay;	}

// Get a post contain unused delay from the vector of post contain unused delays for the FWA
int CFWA::GetPostContUnusedDelay( int i )
{	
	// Some resources do not have a delay Airtankers, seats, scoopers, helicopters and watertenders.  Their delay index is -1
	if ( i > 0  && i < 6 )
		return m_PostContainedUnused[i];
	else
		return 0;
}

// Set all the post escape delays for the FWA in the vector 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
void CFWA::SetEscapeDelays( int postdelay[6] )
{	for ( int i = 0; i < 6; i++ )	{
		if ( postdelay[i] < 0 )
			postdelay[i] = 0;	
		m_PostEscape.push_back( postdelay[i] );
	}
}

// Set a post escape delay in the vector of post escape delays for the FWA
void CFWA::SetEscapeDelay( int i, int postdelay )
{	if ( i >= 0 && i < 6 )
		m_PostEscape[i] = postdelay;	}

// Get a post escape delay from the vector of escape used delays for the FWA
int CFWA::GetEscapeDelay( int i )
{	
	// Some resources do not have a delay Airtankers, seats, scoopers, helicopters and watertenders.  Their delay index is -1
	if ( i > 0  && i < 6 )
		return m_PostEscape[i];	
	else
		return 0;
}


// Set all reload delays for the FWA 0-scooper, 1-SEAT, 2-engine, 3-helicopter, 4-airtanker
void CFWA::SetReloadDelays( int reloaddelays[5] )
{	for ( int i = 0; i < 5; i++ )	{
		if ( reloaddelays[i] < 0 )
			reloaddelays[i] = 0;
		m_ReloadDelay.push_back( reloaddelays[i] );	}
}

// Set a reload delay for the FWA vector 0-scooper, 1-SEAT, 2-engine, 3-helicopter, 4-airtanker
void CFWA::SetReloadDelay( int i, int reloaddelay )
{	if ( i >= 0 && i < 5 )	{ 
		if ( reloaddelay < 0 )
			reloaddelay = 0;
		m_ReloadDelay[i] = reloaddelay;	}
}

// Get a reload delay from the FWA vector 0-scooper, 1-SEAT, 2-engine, 3-helicopter, 4-airtanker
int CFWA::GetReloadDelay( int i )
{	return m_ReloadDelay[i];	}

// Set the delay first unit to respond delay for fires in the FWA
void CFWA::SetFirstUnitDelay( int firstunitdelay )
{	if ( firstunitdelay < 0 )
		firstunitdelay = 0;
	m_FirstUnitDelay = firstunitdelay;	}

// Get the delay for the first unit to respond for fires in the FWA
int CFWA::GetFirstUnitDelay()
{	return m_FirstUnitDelay;	}

// Set all diurnal coefficients for the FWA
void CFWA::SetDiurnalCoefficients( double diurnalcoeffs[23] )
{	for ( int i = 0; i < 23; i++ )	{
		if ( diurnalcoeffs[i] < 0.01 || diurnalcoeffs[i] > 2.0 )
			diurnalcoeffs[i] = 1.0;
		m_DiurnalCoeffs.push_back( diurnalcoeffs[i] );	}
}

// Set a diurnal coefficient for the FWA
void CFWA::SetDiurnalCoefficient( int i, double diurnalcoeff )
{	if ( diurnalcoeff < 0.01 || diurnalcoeff > 2.0 )
		diurnalcoeff = 1.0;
		m_DiurnalCoeffs[i] = diurnalcoeff;	}

// Get all diurnal coefficients for FWA
std::vector< double > CFWA::GetDiurnalCoefficients()
{	return m_DiurnalCoeffs;	}

// Get a diruanl coefficient for the FWA
double CFWA::GetDiurnalCoefficient( int i )
{
    assert( i >= 0 && i < 24 );
    return m_DiurnalCoeffs[i];
}

// Set a ROS Fuel Model adjustment for the FWA
void CFWA::SetROSFuelAdj( string fuelmodel, double adjustment)
{	m_ROSAdjFuel.push_back( fuelmodel );
	if ( adjustment < 0.01 || adjustment > 2.0 )
		adjustment = 1.0;
	m_ROSAdjustment.push_back( adjustment );	}

// Get the ith Fuel model for ROS fuel model adjustment for the FWA
string CFWA::GetROSFuelAdj( int i )
{	
	if ( i > -1 || i < 10 )	
		return m_ROSAdjFuel[i];	
	else
		return "Index out of bounds";
}

// Get the ith ROS adjustment for the ROS fuel model adjustment for the FWA
double CFWA::GetAdjustment( int i )
{
	if (i > -1 || i < 10 )
		return m_ROSAdjustment[i];
	else
		return 1.0;
}
// Set the dispatch logic for FWA
void CFWA::SetDispLogic( CDispLogic &displogic )
{	m_DispLogic = displogic;	}

// Get the dispatch logic for FWA
CDispLogic &CFWA::GetDispLogic()
{	return m_DispLogic;	}

// Get the index for the FWA to obtain air travel distances in dispatch location array
int CFWA::GetIndex()
{	return m_Index;	}

// Set the FPU the FWA resides in
void CFWA::SetFPU( string fpu )
{	m_FPU = fpu;	}

// Get the FPU the FWA resides in
string CFWA::GetFPU()
{	return m_FPU;	}

// Set the FWAs Latitude
void CFWA::SetLatitude( double latitude )
{	m_Latitude = latitude;	}

// Get the FWAs Latitude
double CFWA::GetLatitude()
{	return m_Latitude;	}

void CFWA::SetLongitude( double longitude )	
{	m_Longitude = longitude;	}

double CFWA::GetLongitude()
{	return m_Longitude;		}

// Get the fuel model adjustment factor for a numerical fuel model
double CFWA::FuelModelAdjROS( int fuelmodel, double ROS )
{	
	// Get the string value for the numerical fuel model
	string EqFuelModel;
	switch ( fuelmodel )
	{
		case 101:
			EqFuelModel = "GR1";
			break;
		case 102:
			EqFuelModel = "GR2";
			break;
		case 103:
			EqFuelModel = "GR3";
			break;
		case 104:
			EqFuelModel = "GR4";
			break;
		case 105:
			EqFuelModel = "GR5";
			break;
		case 106:
			EqFuelModel = "GR6";
			break;
		case 107:
			EqFuelModel = "GR7";
			break;
		case 108:
			EqFuelModel = "GR8";
			break;
		case 109:
			EqFuelModel = "GR9";
			break;
		case 121:
			EqFuelModel = "GS1";
			break;
		case 122:
			EqFuelModel = "GS2";
			break;
		case 123:
			EqFuelModel = "GS3";
			break;
		case 124:
			EqFuelModel = "GS4";
			break;
		case 141:
			EqFuelModel = "SH1";
			break;
		case 142:
			EqFuelModel = "SH2";
			break;
		case 143:
			EqFuelModel = "SH3";
			break;
		case 144:
			EqFuelModel = "SH4";
			break;
		case 145:
			EqFuelModel = "SH5";
			break;
		case 146:
			EqFuelModel = "SH6";
			break;
		case 147:
			EqFuelModel = "SH7";
			break;
		case 148:
			EqFuelModel = "SH8";
			break;
		case 149:
			EqFuelModel = "Sh9";
			break;
		case 161:
			EqFuelModel = "TU1";
			break;
		case 162:
			EqFuelModel = "TU2";
			break;
		case 163:
			EqFuelModel = "TU3";
			break;
		case 164:
			EqFuelModel = "TU4";
			break;
		case 165:
			EqFuelModel = "TU5";
			break;
		case 181:
			EqFuelModel = "TL1";
			break;
		case 182:
			EqFuelModel = "TL2";
			break;
		case 183:
			EqFuelModel = "TL3";
			break;
		case 184:
			EqFuelModel = "TL4";
			break;
		case 185:
			EqFuelModel = "TL5";
			break;
		case 186:
			EqFuelModel = "TL6";
			break;
		case 187:
			EqFuelModel = "TL7";
			break;
		case 188:
			EqFuelModel = "TL8";
			break;
		case 189:
			EqFuelModel = "TL9";
			break;
		case 201:
			EqFuelModel = "SB1";
			break;
		case 202:
			EqFuelModel = "SB2";
			break;
		case 203:
			EqFuelModel = "SB3";
			break;
		case 204:
			EqFuelModel = "SB4";
			break;
		default:
			EqFuelModel = "none";
			break;
	}

	// Check the fuel model ROS adjustments for the FWA and determine if one exists for the fire's fuel model
	int adj = -1;
	for (int i = 0; i < 10; i++ )	
		if ( m_ROSAdjFuel[ i ] == EqFuelModel )
			adj = i;
	if ( adj != -1 )
		ROS = ROS * m_ROSAdjustment[ adj ];

	return ROS;
	
}

// Set the name of the Closest Airtanker Base
void CFWA::SetClosestATBase( string ATBase )
{	m_ClosestATBase = ATBase;	}

// Get the name of the Closest AirtankerBase
string CFWA::GetClosestATBase()
{	return m_ClosestATBase;	}

// Get the count for the number of resources instantiated
int CFWA::GetCount()
{	return count;	}

// Print Information about the FWA
void CFWA::PrintFWAInfo()
{
	cout << "FWA ID: " << m_FWAID << "\n";
	cout << "	Index: " << m_Index << "\n";
	cout << "	FMG: " << m_FMG << "\n";
	cout << "	Percent Walk-in fires: " << m_WalkInPct << "\n";
	cout << "	Percent Pump and Roll fires: " << m_PumpNRollPct << "\n";
	cout << "	Percent Head fires: " << m_Head << "\n";
	cout << "	Percent Tail fires: " << m_Tail << "\n";
	cout << "	Percent Parallel fires: " << m_Parallel << "\n";
	cout << "	Attack Distance: " << m_AttackDist << "\n";
	cout << "	Water Drops allowed? " << m_WaterDrops << "\n";
	cout << "	FWA excluded from analysis? " << m_Excluded << "\n";
	cout << "	Discovery size: " << m_DiscoverySize << "\n";
	cout << "	Escape Time: " << m_ESLTime << "\n";
	cout << "	Escape Size: " << m_ESLSize << "\n";
	cout << "	Air to ground ratio: " << m_AirtoGround << "\n";
	cout << "	Walk-in delays: Tracked: " << m_WalkInDelays[0] << " Boat: " << m_WalkInDelays[1] << " Crew: " << m_WalkInDelays[2]
		<< " Engine: " << m_WalkInDelays[3] << " Helitack: " << m_WalkInDelays[4] << " Smokejumpers: " << m_WalkInDelays[5] << "\n";
	cout << "	Post Contained Delay Used: Tracked: "<< m_PostContainedUsed[0] << " Boat: " << m_PostContainedUsed[1] << " Crew: " << m_PostContainedUsed[2]
		<< " Engine: " << m_PostContainedUsed[3] << " Helitack: " << m_PostContainedUsed[4] << " Smokejumpers: " << m_PostContainedUsed[5] << "\n";
	cout << "	Post Contained Delay Unused: Tracked: "<< m_PostContainedUnused[0] << " Boat: " << m_PostContainedUnused[1] << " Crew: " << m_PostContainedUnused[2]
		<< " Engine: " << m_PostContainedUnused[3] << " Helitack: " << m_PostContainedUnused[4] << " Smokejumpers: " << m_PostContainedUnused[5] << "\n";
	cout << "	Post Escaped Delay: Tracked: "<< m_PostEscape[0] << " Boat: " << m_PostEscape[1] << " Crew: " << m_PostEscape[2]
		<< " Engine: " << m_PostEscape[3] << " Helitack: " << m_PostEscape[4] << " Smokejumpers: " << m_PostEscape[5] << "\n";
	cout << "	Reload Delay: Scooper: " << m_ReloadDelay[0] << " SEAT: " << m_ReloadDelay[1] << " Engine: " << m_ReloadDelay[2] << " Helicopter: " << m_ReloadDelay[3] << "\n";
	cout << "	First unit Delay: " << m_FirstUnitDelay << "\n";
	cout << "	Diurnal Coefficients: 0000: " << m_DiurnalCoeffs[0] << " 0100: " << m_DiurnalCoeffs[1] << " 0200: " << m_DiurnalCoeffs[2]
		<< " 0300: " << m_DiurnalCoeffs[3] << " 0400: " << m_DiurnalCoeffs[4] << " 0500: " << m_DiurnalCoeffs[6] << " 0600: " << m_DiurnalCoeffs[6]
		<< " 0700: " << m_DiurnalCoeffs[7] << " 0800: " << m_DiurnalCoeffs[8] << " 0900: " << m_DiurnalCoeffs[9] << " 1000: " << m_DiurnalCoeffs[10]
		<< " 1100: " << m_DiurnalCoeffs[11] << " 1200: " << m_DiurnalCoeffs[12] << "\n"
		<< " 1300: " << m_DiurnalCoeffs[13] << " 1400: " << m_DiurnalCoeffs[14]
		<< " 1500: " << m_DiurnalCoeffs[15] << " 1600: " << m_DiurnalCoeffs[16] << " 1700: " << m_DiurnalCoeffs[17] << " 1800: " << m_DiurnalCoeffs[18]
		<< " 1900: " << m_DiurnalCoeffs[19] << " 2000: " << m_DiurnalCoeffs[20] << " 2100: " << m_DiurnalCoeffs[21] << " 2200: " << m_DiurnalCoeffs[22]
		<< " 2300: " << m_DiurnalCoeffs[23] << " \n";
	cout << "	ROS adjustment by fuel model: " << "\n";
	int i = 0;
	while (m_ROSAdjFuel[i] != "x" )	{
		cout << " Fuel Model: " << m_ROSAdjFuel[i] << " ROS Adjustment: " << m_ROSAdjustment[i] << "\n";
		i++;
	}
	m_DispLogic.PrintDispLogic();
	
}

// Set the FWAs Associated Dispatch Location Map with a map
void CFWA::SetAssociations( AssocMap map )
{	m_AssocMap = map;	}

// Add a Dispatch location / Distance pair to the Association map
void CFWA::AddAssociation( string DispLoc, double distance )
{	m_AssocMap.insert( std::pair< double, string >( distance, DispLoc ) );	}

// Get the Associations map for the FWA
AssocMap CFWA::GetAssociationMap()
{	return m_AssocMap;	}

// Determine if a Dispatch Location is associated with the FWA
bool CFWA::IsAssociated( string DispLocID )
{
	std::multimap< double, string >::iterator It = m_AssocMap.begin();
	
	bool found = false;

	while ( !found && It != m_AssocMap.end() )	{

		string AssocDL = ( *It ).second;

		if ( AssocDL == DispLocID )
			found = true;

		else
			It++;

	}

	return found;

}

// Get the Distance from the associations map for a Dispatch Location
double CFWA::GetDistance( string DispLocID )
{
	std::multimap< double, string >::iterator It = m_AssocMap.begin();
	
	bool found = false;

	while ( !found && It != m_AssocMap.end() )	{

		string AssocDL = ( *It ).second;

		if ( AssocDL == DispLocID )
			found = true;

		else
			It++;

	}

	if ( found )
		return ( *It ).first;

	else 
		return -1;

}
