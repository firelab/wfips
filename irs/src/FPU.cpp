// FPU.cpp
// CFPU member function definitions for individual FPUs
// 3/13 for IRS

#include <iostream>											//contains functions for input and output
using namespace std;
#include <string>											//contains functions for operations with strings
using std::string;
#include <cstring>											//to convert a string to a const char*
#include <cstdlib>											//to use atof to convert const char* to int
#include <vector>

#include "FPU.h"

// Default Constructor
CFPU::CFPU()
{
	m_FPUID = "Default";
	for ( int i = 0; i < 365; i++ )	{
		m_Average.push_back( 0.0 );
		m_Max.push_back( 0.0 );
		m_FausMax.push_back( 0.0 );
		m_ExpectLevel.push_back( 0.0 );
		m_NumFires.push_back( 0.0 );
		m_ATReliance = 0.0;
	}

}

// Constructor
CFPU::CFPU( string FPUid, double Average[365], double Max[365], double FausMax[365] )
{
	m_FPUID = FPUid;
	
	for ( int i = 0; i < 365; i++ )	{
		m_Average.push_back( Average[i] );
		m_Max.push_back( Max[i] );
		m_FausMax.push_back( FausMax[i] );
		m_ExpectLevel.push_back( 0.0 );
		m_NumFires.push_back( 0.0 );
		m_ATReliance = 0.0;
	}

}

// Destructor
CFPU::~CFPU(){}													

// Copy Constructor
CFPU::CFPU( const CFPU &fpu ) 
{
	m_FPUID = fpu.m_FPUID;
	m_Average = fpu.m_Average;
	m_Max = fpu.m_Max;
	m_FausMax = fpu.m_FausMax;
	m_ExpectLevel = fpu.m_ExpectLevel;
	m_NumFires = fpu.m_NumFires;
	m_ATReliance = fpu.m_ATReliance;
}

// Assignment operator
const CFPU &CFPU::operator=( const CFPU &fpu)
{
	m_FPUID = fpu.m_FPUID;
	m_Average = fpu.m_Average;
	m_Max = fpu.m_Max;
	m_FausMax = fpu.m_FausMax;
	m_ExpectLevel = fpu.m_ExpectLevel;
	m_NumFires = fpu.m_NumFires;
	m_ATReliance = fpu.m_ATReliance;

	return *this;
}

// Set FPU ID

void CFPU::SetFPUID( string id )
{	m_FPUID = id;	}

// Get the FPU ID
string CFPU::GetFPUID()
{	return m_FPUID;	}

// Set the Average Number of Fires vector with a vector
void CFPU::SetAverageVector( vector< double > Average )
{	m_Average = Average;	}

// Get the Average Number of Fires vector
vector< double > CFPU::GetAverageVector()
{	return m_Average;	}

// Set an average value using an index
void CFPU::SetAverageIdx( double Value, int Index )
{	m_Average[ Index ] = Value;	}

// Get an average value from the vector
double CFPU::GetAverageIdx( int Index )
{	return m_Average[ Index ];	}

// Set the maximum vector with a vector
void CFPU::SetMaxVector( vector< double > Max )
{	m_Max = Max;	}

// Get the maximum vector
vector< double > CFPU::GetMaxVector()
{	return m_Max;	}

// Set a maximum value
void CFPU::SetMaxIdx( double Value, int Index )
{	m_Max[ Index ] = Value;	}

// Get a maximum value using an index
double CFPU::GetMaxIdx( int Index )
{	return m_Max[ Index ];	}

// Set the Faus Max Vector
void CFPU::SetFausMaxVector( vector< double > FausMax )
{	m_FausMax = FausMax;	}

// Get the Faus Max Vector
vector< double > CFPU::GetFausMaxVector()
{	return m_FausMax;	}

// Set a Faus Max value
void CFPU::SetFausMaxIdx( double Value, int Index )
{	m_FausMax[ Index ] = Value;	}

// Get a Faus Max Value
double CFPU::GetFausMaxIdx( int Index )
{	return m_FausMax[ Index ];	}

// Set the expected level vector with a vector
void CFPU::SetExpectLevelVector( vector< double > expect )
{	m_ExpectLevel = expect;	}

// Get the expected level vector
vector< double > CFPU::GetExpectLevelVector()
{	return m_ExpectLevel;	}

// Set an expected level value
void CFPU::SetExpectLevelIdx( double Value, int Index )
{	m_ExpectLevel[ Index ] = Value;	}

// Get an expected level value
double CFPU::GetExpectLevelIdx( int Index )
{	return m_ExpectLevel[ Index ];	}

// Set the number of fires for a scenario with a vector
void CFPU::SetNumFiresVector( vector< int > numfires )
{	m_NumFires = numfires;	}

// Get the vector with the number of fires
vector< int > CFPU::GetNumFiresVector()
	{	return m_NumFires;	}

// Get the number of fires on a julian day
void CFPU::SetNumFires( int Value, int Julian )
{	m_NumFires[ Julian - 1 ] = Value;	}

// Get the number of fires for a Julian day
int CFPU::GetNumFires( int Julian )
{	return m_NumFires[ Julian - 1 ];	}

// Add to the number of fires on a Julian day
void CFPU::AddFire( int Julian )
{	m_NumFires[ Julian - 1 ] = m_NumFires[ Julian - 1 ] + 1;	}

// Set the FPU's reliance on large airtankers
void CFPU::SetReliance( double reliance )
{	m_ATReliance = reliance;	}

// Get the FPU's reliance on large airtankers
double CFPU::GetReliance()
{	return m_ATReliance;	}



