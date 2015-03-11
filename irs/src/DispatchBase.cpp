// DispatchBase.cpp
// Member function definitions for class CDispatchBase
// created 11/12 for IRS

#include <iostream>													//contains functions for input and output
using namespace std;
#include <fstream>													//contains functions for inputting and outputting to a file
using std::ifstream;												//for inputting from a file
#include <sstream>
#include <cctype>													//contains functions for characters, strings, and structures
#include <string>													//contains functions for operations with strings
using std::string;
#include <cstring>													//to convert a string to a const char*
#include <cstdlib>													//to use atof to convert const char* to int
#include <vector>
#include <list>

// Include definition for class CDispatchBase from DispatchBase.h
#include "DispatchBase.h"
// Include definition for class CDispLoc from DispLoc.h
#include "DispLoc.h"

// define and initialize static data member count of dispatch locations
int CDispatchBase::count = 0;

// Default Constructor for CDispatchBase
CDispatchBase::CDispatchBase()
{
	m_DispatcherID = " Default ";
	m_SeasonStart = 1;
	m_SeasonEnd = 365;
	
	for ( int i=0; i<15; i++)	{
		m_BaseRescLevel.push_back(0);
		m_MinRescLevel.push_back(0);
		m_CurRescLevel.push_back(0);
		m_DailyMinRescLevel.push_back(10000);
	}
	
	for ( int i = 0; i < 365; i++ )	{
		m_VExpectLevel.push_back( 0.0 );
		m_VPreviousLevel.push_back( 1.0 );
		m_VEscapeLevel.push_back( 0 );
		m_VDispLogicUnfilled.push_back( 0 );
		m_VDailyDispatches.push_back(0);
		m_VDailyUsage.push_back(0.0);
	}

	count++;
}

// Constructor for CDispatchBase
CDispatchBase::CDispatchBase( string dispatcherID )
{

	m_DispatcherID = dispatcherID;									//unique id for the dispatcher
	m_SeasonStart = 1;
	m_SeasonEnd = 365;
	
	for ( int i=0; i<15; i++)	{
		m_BaseRescLevel.push_back(0);
		m_MinRescLevel.push_back(0);
		m_CurRescLevel.push_back(0);
		m_DailyMinRescLevel.push_back(10000);
	}
	
	for ( int i = 0; i < 365; i++ )	{
		m_VExpectLevel.push_back( 0.0 );
		m_VPreviousLevel.push_back( 1.0 );
		m_VEscapeLevel.push_back( 0 );
		m_VDispLogicUnfilled.push_back( 0 );
		m_VDailyDispatches.push_back(0);
		m_VDailyUsage.push_back(0.0);
	}
	
	count++;														// Increase the resource count
}

// Copy constructor for CDisptchBase
CDispatchBase::CDispatchBase( const CDispatchBase &dispatchbase )
{
	m_DispatcherID = dispatchbase.m_DispatcherID;
	m_SeasonStart = dispatchbase.m_SeasonStart;
	m_SeasonEnd = dispatchbase.m_SeasonEnd;
	m_BaseRescLevel = dispatchbase.m_BaseRescLevel;
	m_MinRescLevel = dispatchbase.m_MinRescLevel;
	m_CurRescLevel = dispatchbase.m_CurRescLevel;
	m_DailyMinRescLevel = dispatchbase.m_DailyMinRescLevel;
	m_VExpectLevel = dispatchbase.m_VExpectLevel;
	m_VPreviousLevel = dispatchbase.m_VPreviousLevel;
	m_VEscapeLevel = dispatchbase.m_VEscapeLevel;
	m_VDispLogicUnfilled = dispatchbase.m_VDispLogicUnfilled;
	m_VDailyDispatches = dispatchbase.m_VDailyDispatches;
	m_VDailyUsage = dispatchbase.m_VDailyUsage;
}

// Destructor for CDispatchBase
CDispatchBase::~CDispatchBase() {}

// Print tree
std::ostream &operator<<(std::ostream &out, CDispatchBase *dispatchbase )
{	
	out << dispatchbase->GetDispatcherID() << "\n";
	return out;
}

// Equal operator for CDispatchBase
bool CDispatchBase::operator==( CDispatchBase &dispatchbase )
{
	bool Equal = true;
	if ( m_DispatcherID != dispatchbase.m_DispatcherID )
		Equal = false;
	if ( m_SeasonStart != dispatchbase.m_SeasonStart )
		Equal = false;
	if ( m_SeasonEnd != dispatchbase.m_SeasonEnd )
		Equal = false;
	if ( m_BaseRescLevel != dispatchbase.m_BaseRescLevel )
		Equal = false;
	if ( m_MinRescLevel != dispatchbase.m_MinRescLevel )
		Equal = false;
	if ( m_CurRescLevel != dispatchbase.m_CurRescLevel )
		Equal = false;
	if ( m_DailyMinRescLevel != dispatchbase.m_DailyMinRescLevel )
		Equal = false;
	if ( m_VExpectLevel != dispatchbase.m_VExpectLevel )
		Equal = false;
	if ( m_VPreviousLevel != dispatchbase.m_VPreviousLevel )
		Equal = false;
	if ( m_VEscapeLevel != dispatchbase.m_VEscapeLevel )
		Equal = false;
	if ( m_VDispLogicUnfilled != dispatchbase.m_VDispLogicUnfilled )
		Equal = false;
	if (m_VDailyDispatches != dispatchbase.m_VDailyDispatches)
		Equal = false;
	if (m_VDailyUsage != dispatchbase.m_VDailyUsage)
		Equal = false;


	return Equal;
}



// set the Dispatcher's ID
void CDispatchBase::SetDispatcherID( string dispatcherID )
{	m_DispatcherID = dispatcherID;	}

// get the Dispatcher's ID
string CDispatchBase::GetDispatcherID()
{	return m_DispatcherID;	}

// set the base resource level for the dispatcher
void CDispatchBase::SetBaseRescLevel( std::vector< int > baseresclevel )
{	m_BaseRescLevel = baseresclevel;	}

// get the base resource level for the dispatcher
std::vector< int > CDispatchBase::GetBaseRescLevel()
{	return m_BaseRescLevel;	}

// set the base resource level for a dispatcher by index
void CDispatchBase::SetBaseRescLevelIdx( int index, int level )
{	m_BaseRescLevel[index] = level;	}

// get the base resource level for a dispatcher by index
int CDispatchBase::GetBaseRescLevelIdx( int index )
{	return m_BaseRescLevel[index];	}

// set the minimum resource level for the dispatcher
void CDispatchBase::SetMinRescLevel( std::vector< int > minresclevel )
{	m_MinRescLevel = minresclevel;	}

// Get the minimum resource level for the dispatcher
std::vector< int > CDispatchBase::GetMinRescLevel()
{	return m_MinRescLevel;	}

// Set the minimum resource level for a dispatcher by index
void CDispatchBase::SetMinRescLevelIdx( int index, int level )
{	m_MinRescLevel[index] = level;	}

// get the minimum resource level for a dispatcher by index
int CDispatchBase::GetMinRescLevelIdx( int index )
{	return m_MinRescLevel[index];	}

// set the current resource level for the dispatcher
void CDispatchBase::SetCurRescLevel( std::vector< int > curresclevel )
{	m_CurRescLevel = curresclevel;	}

// Get the current resource level for the dispatcher
std::vector< int > CDispatchBase::GetCurRescLevel()
{	return m_CurRescLevel;	}

// Set the current resource level for a dispatcher by index
void CDispatchBase::SetCurRescLevelIdx( int index, int level )
{	m_CurRescLevel[index] = level;	}

// get the Current resource level for a dispatcher by index
int CDispatchBase::GetCurRescLevelIdx( int index )
{	return m_CurRescLevel[index];	}

// set the daily minimum resource level for the dispatcher
void CDispatchBase::SetDailyMinRescLevel( std::vector< int > dailyminlevel )
{	m_DailyMinRescLevel = dailyminlevel;	}

// Get the daily minimum resource level for the dispatcher
std::vector< int > CDispatchBase::GetDailyMinRescLevel()
{	return m_DailyMinRescLevel;	}

// Set the daily minimum resource level for a dispatcher by index
void CDispatchBase::SetDailyMinRescLevelIdx( int index, int level )
{	m_DailyMinRescLevel[index] = level;	}

// get the daily minimum resource level for a dispatcher by index
int CDispatchBase::GetDailyMinRescLevelIdx( int index )
{	return m_DailyMinRescLevel[index];	}

// set the expected use level for the dispatcher using a vector
void CDispatchBase::SetExpectLevelVector( vector< double > expectlevel )
{	m_VExpectLevel = expectlevel;	}

// get the expected use level vector for the dispatcher
vector< double > CDispatchBase::GetExpectLevelVector()
{	return m_VExpectLevel;	}

// set the expected use level for the dispatcher
void CDispatchBase::SetExpectLevel( double expectlevel, int Julian )
{	m_VExpectLevel[Julian-1] = expectlevel;	}

// get the expected use level for the dispatcher
double CDispatchBase::GetExpectLevel( int Julian )
{	return m_VExpectLevel[Julian-1];	}

// Add one fire the the expected level
void CDispatchBase::AddtoExpectLevel( int Julian) 
{	m_VExpectLevel[Julian-1] = m_VExpectLevel[Julian-1] + 1;	}

// set the previous use level for the dispatcher using a vector
void CDispatchBase::SetPreviousLevelVector( vector< double > previouslevel )
{	m_VPreviousLevel = previouslevel;	}

// get the expected use level vector for the dispatcher
vector< double > CDispatchBase::GetPreviousLevelVector()
{	return m_VPreviousLevel;	}

// set the previous use level for the dispatcher
void CDispatchBase::SetPreviousLevel( double previouslevel, int Julian )
{	m_VPreviousLevel[Julian-1] = previouslevel;	}

// get the previous use level for the dispatcher
double CDispatchBase::GetPreviousLevel( int Julian )
{
    assert( 0 < Julian < 366 );
    return m_VPreviousLevel[Julian-1];
}

// Set the daily usage level vector with a vector
void CDispatchBase::SetDailyUsageLevel(vector<double> dailyusagelevel)
{	m_VDailyUsage = dailyusagelevel;	}

// Get the daily usage level vector
vector<double> CDispatchBase::GetDailyUsageLevel()
{	return m_VDailyUsage;	}

// Set the daily usage level vector for a julian day
void CDispatchBase::SetDailyUsage(double dailyusage, int Julian)
{	assert( 0 < Julian < 366 );
	m_VDailyUsage[Julian-1] = dailyusage;	}

// Get the daily usage level for a julian day
double CDispatchBase::GetDailyUsage(int Julian)
{	return m_VDailyUsage[Julian-1];	}

// Set the daily dispatch level vector with a vector
void CDispatchBase::SetDailyDispatchLevelVector(vector<int> dailydispatchlevel)
{	m_VDailyDispatches = dailydispatchlevel;	}

// Get the daily dispatch level vector
vector<int> CDispatchBase::GetDailyDisaptchLevelVector()
{	return m_VDailyDispatches;	}

// Set a day of the daily dispatch level vector
void CDispatchBase::SetDailyDispatchLLevel(int dailydispatchlevel, int Julian)
{	m_VDailyDispatches[Julian - 1] = dailydispatchlevel;	}

// Get the daily dispatch level for a given julian day
int CDispatchBase::GetDailyDispatchLevel(int Julian)
{	return m_VDailyDispatches[Julian - 1];	}

// Add dispatches to the daily dispatch level
void CDispatchBase::AddDailyDispatch(int Julian, int numberdispatches)
{	m_VDailyDispatches[Julian - 1] = m_VDailyDispatches[Julian - 1] + numberdispatches;	}

// set the escape fire level for the dispatcher using a vector
void CDispatchBase::SetEscapeLevelVector( vector< int > escapelevel )
{	m_VEscapeLevel = escapelevel;	}

// get the escape fire level vector for the dispatcher
vector< int > CDispatchBase::GetEscapeLevelVector()
{	return m_VEscapeLevel;	}

// set the escape fire level for the dispatcher
void CDispatchBase::SetEscapeLevel( int escapelevel, int Julian )
{	m_VEscapeLevel[Julian-1] = escapelevel;	}

// get the escape fire level for the dispatcher
int CDispatchBase::GetEscapeLevel( int Julian )
{	return m_VEscapeLevel[Julian-1];	}

// add one to the escape fire level for the dispatcher
void CDispatchBase::AddToEscapeLevel( int Julian )
{	m_VEscapeLevel[Julian-1] = m_VEscapeLevel[Julian-1] + 1;	}

// set the dispatch logic unfilled level for the dispatcher using a vector
void CDispatchBase::SetDispLogicUnfilledVector( vector< int > displogicunfilled )
{	m_VDispLogicUnfilled = displogicunfilled;	}

// get the dispatch logic unfilled level vector for the dispatcher
vector< int > CDispatchBase::GetDispLogicUnfilledVector()
{	return m_VDispLogicUnfilled;	}

// set the dispatch logic unfilled level for the dispatcher
void CDispatchBase::SetDispLogicUnfilled( int displogicunfilled, int Julian )
{	m_VDispLogicUnfilled[Julian-1] = displogicunfilled;	}

// get the dispatch logic unfilled level for the dispatcher
int CDispatchBase::GetDispLogicUnfilled( int Julian )
{	return m_VDispLogicUnfilled[Julian-1];	}

// add one to the dispatch logic unfilled level for the dispatcher
void CDispatchBase::AddToDispLogicUnfilled( int Julian )
{	m_VDispLogicUnfilled[Julian-1] = m_VDispLogicUnfilled[Julian-1] + 1;	}

// set the start date for the fire season
void CDispatchBase::SetSeasonStart( int Julian )
{	m_SeasonStart = Julian;	}

// get the start date for the fire season
int CDispatchBase::GetSeasonStart()
{	return m_SeasonStart;	}

// set the end date for the fire season
void CDispatchBase::SetSeasonEnd( int Julian )
{	m_SeasonEnd = Julian;	}

// get the end date for the fire season
int CDispatchBase::GetSeasonEnd()
{	return m_SeasonEnd;	}

// is the julian date in the fire season for the dispatch location
bool CDispatchBase::IsInFireSeason( int Julian )
{
	bool IsIn = true;

	if ( m_SeasonEnd > m_SeasonStart )	{

		if ( Julian > m_SeasonEnd || Julian < m_SeasonStart )
			IsIn = false;

	}

	else	{

		if ( Julian < m_SeasonEnd && Julian > m_SeasonStart )
			IsIn = false;

	}

	return IsIn;

}

// Print the resource dispatch level vector for the resources
void CDispatchBase::PrintRescLevel( vector< int > VRescLevel)
{
	// Check that the size of the vector is appropriate
	int size = VRescLevel.size();
	if ( size != 15 )
		cout << "CDispatchBase::PrintRescLevel - Current Resource Level Vector does not have the appropriate size\n";
	
	vector< string > RescType;
	RescType.push_back( "	ATT - " );
	RescType.push_back( "	CRW - " );
	RescType.push_back( "	DZR - " );
	RescType.push_back( "	ENG - " );
	RescType.push_back( "	FBDZ - " );
	RescType.push_back( "	FRBT - " );
	RescType.push_back( "	Helicopter - " );
	RescType.push_back( "	HELI - " );
	RescType.push_back( "	SCPSEAT - " );
	RescType.push_back( "	SJAC - " );
	RescType.push_back( "	SMJR - " );
	RescType.push_back( "	TP - " );
	RescType.push_back( "	WT - " );
	RescType.push_back( "	CRW20 - " );
	RescType.push_back( "	T1HEL - " );

	// Print the values
	cout << "Resource Level Vector \n";
	for ( int i = 0; i < size; i++ )
		cout << "	" << RescType[i] << VRescLevel[i] << "\n";

}

// Reset values for a new scenario
void CDispatchBase::ResetNewScenario( int scenario )
{
	ofstream outFile( "Levels.dat" , ios::app );
	  
	// Exit program if unable to create file
	if ( !outFile ) { 
		cerr << "Output file could not be opened" << endl;
		//exit( 1 );
	}
	
	// Print out the data for the 365 days of the year
	
	// If the scenario is greater than 0 add to levels.dat file
	if ( scenario > 0 )	{
		string DispatcherID = GetDispatcherID();
		outFile << "Dispatcher ID: " << DispatcherID << "\n";

		for ( int i = 0; i < 365; i++ )
			outFile << scenario << ", " << i << ", " << m_VExpectLevel[i] << ", " << m_VPreviousLevel[i] << ", " << m_VEscapeLevel[i] << ", " << 
			m_VDispLogicUnfilled[i] << "\n";
	}

	
	// Reset the 
	for ( int i=0; i<15; i++)	{
		m_CurRescLevel[i] = 0;
		m_DailyMinRescLevel[i] = 10000;
	}
	
	for ( int i = 0; i < 365; i++ )	{
		//m_VExpectLevel[i] = 0.0;			Expected level is read in with the fire file
		m_VPreviousLevel[i] = 1.0;
		m_VEscapeLevel[i] = 0;
		m_VDispLogicUnfilled[i] = 0;
	}
}

// Determine the current level of resources available at interior nodes in the tree
// Note depends on the levels at the external nodes so be sure to determine those first
vector< int > CDispatchBase::DetermineCurRescLevel( int timeYear, vector< CDispatchBase* > VDispatchers )
{
	vector< int > NumRescs( 15, 0 );

	// The current level of resources available at a node is the sum of the levels at the external nodes further down the tree
	// This is equivalent to summing the current level of resources available at all the Node's childern provided all the nodes current resource values have been calculated
	for ( int i = 0; i < VDispatchers.size(); i++ )	{
		vector< int > AddNumRescs = VDispatchers[i]->GetCurRescLevel();
			
		// Add in the resources for the dispatcher
		for ( int j = 0; j < 15; j++ )
				NumRescs[j] = NumRescs[j] + AddNumRescs[j];
	}

	// Update the current resource levels
	SetCurRescLevel( NumRescs );

	// Update the Daily minimum resource level for each resource type
	for ( int i = 0; i < 15; i++ )	{
		if ( m_DailyMinRescLevel[i] > m_CurRescLevel[i] )
			m_DailyMinRescLevel[i] = m_CurRescLevel[i];
	}

	return NumRescs;
}

// Reset the values for a new day - Daily minimum resource values
void CDispatchBase::ResetNewDay( int Julian )
{
	// Need to calculate the previous use level which will use these values
	// Calculate the Daily minimum number of resources
	int sum = 0;
	for ( int i = 0; i < 15; i++ )	{
		if ( m_DailyMinRescLevel[i] < 10000 )
		sum = sum + m_DailyMinRescLevel[i];
	}

	// Determine the previous level for the day as the proportion of the base level of resources
	int sumbase = 0;
	for ( int i = 0; i < 15; i++ )
		sumbase = sumbase + m_BaseRescLevel[i];

	double sumd = static_cast<double>( sum );
	double sumbased = static_cast<double>( sumbase );

	if ( sumbase > 0 )
		m_VPreviousLevel[ Julian - 1] = sumd / sumbased;

	else
		m_VPreviousLevel[ Julian - 1 ] = 1;

	// Reset the m_DailyMinRescLevel vector
	for ( int i = 0; i < 15; i++ )
		m_DailyMinRescLevel[i] = 10000;

	
}

// Determine the base resource level for dispatcher - for internal nodes need to be sure to calculate values for all subnodes first
void CDispatchBase::DetermineBaseRescLevel( vector< CDispatchBase* > VDispatchers )
{
	// Setup the vector
	vector< int > Levels( 15, 0);

	// Iterate through the children of the node and add up the values for the levels
	for ( int i = 0; i < VDispatchers.size(); i++ )	{

		// Add the values for each resource type
		for ( int j = 0; j < 15; j++ )
			Levels[j] = VDispatchers[i]->GetBaseRescLevelIdx( j ) + Levels[j];

	}

	SetBaseRescLevel( Levels );
}

// Clear the temporary list of escape fires for the dispatcher
void CDispatchBase::ClearEscapes()
{	m_LTempEscapes.clear();	}

// Add an escape fire to the list in the proper order
void CDispatchBase::AddEscape( CEscape Escape )
{
	// If the list is empty
	if ( m_LTempEscapes.empty() )
		m_LTempEscapes.push_back( CEscape( Escape ) );

	else	{

		int ELevel = Escape.GetLevel();
		int ETime = Escape.GetEscapeTimeBY();

		list< CEscape >::iterator It = m_LTempEscapes.begin();

		bool Found = false;

		// iterate through the list of escapes and find the proper placement for the escape
		// Order by level then escape time

		while (  It != m_LTempEscapes.end() && !Found )	{

			int CompLevel = ( *It ).GetLevel();
			int CompTime = ( *It ).GetEscapeTimeBY();

			// Check the Level of the escape fire in the list
			if ( CompLevel > ELevel )	
				Found = true;
				
			if ( CompLevel == ELevel && CompTime > ETime )
				Found = true;

			if ( !Found )
				It++;	

		}

		// place the escape fire
		m_LTempEscapes.push_back( CEscape( Escape ) );

	}

}

// Calcualte the daily usage for the dispatcher as a function of the dispatch location nodes under it
bool CDispatchBase::CalcDailyUsageLevel(int Julian, vector<CDispatchBase*> VDispatchers)
{
	// Get the number of resources at the dispatch location dispatcher 
	vector<int> numResources = GetCurRescLevel();

	int count = 0;
	for (int i = 0; i < numResources.size(); i++)
		count = count + numResources[i];

	

	if (count == 0)	
		SetDailyUsage(0,Julian);
	else	{
		// Get the number of resources deployed
		int deployed = 0;
		for (int i = 0; i < VDispatchers.size(); i++)	{
			int DLDeployed = VDispatchers[i]->GetDailyDispatchLevel(Julian);
			deployed = deployed + DLDeployed;
		}
		double dailyUsage = static_cast<double>(deployed)/count;
		SetDailyUsage(dailyUsage, Julian);
	}

	return true;
}




	


	
