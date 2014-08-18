// NatDispatcher.cpp
// Member function definitions for class CNatDispatcher National dispatcher from CDispatchBase
// created 1/13 for IRS

#include <iostream>													//contains functions for input and output
using namespace std;
#include <string>													//contains functions for operations with strings
using std::string;
#include <cstring>													//to convert a string to a const char*
#include <cstdlib>													//to use atof to convert const char* to int
#include <vector>
#include <list>
#include <map>

// Include definition for class CNatDispatcher from NatDispatcher.h
#include "NatDispatcher.h"
// Include definition for class CDispatchBase from DispatchBase.h
#include "DispatchBase.h"


// Default Constructor for CNatDispatcher
CNatDispatcher::CNatDispatcher() : CDispatchBase()
{
	
}

// Constructor for CNatDispatcher
CNatDispatcher::CNatDispatcher( string dispatcherID ) : CDispatchBase( dispatcherID )
{

}

// Copy constructor for CNatDispatcher
CNatDispatcher::CNatDispatcher( const CNatDispatcher &natdispatcher ) : CDispatchBase ( natdispatcher )
{
	
}

// Destructor for CDLDispatcher
CNatDispatcher::~CNatDispatcher()
{}

// Set the vector of airtankers with a vector of airtankers
void CNatDispatcher::SetAirtankerVector( vector< CResource* > VAirtankers )
{	m_VAirtankers = VAirtankers; }

// Get the vectors of airtankers
vector< CResource* > CNatDispatcher::GetAirtankerVector()
{	return m_VAirtankers;	}

// Add a CResource pointer for an airtanker to the airtanker vector
void CNatDispatcher::AddAirtankerToVector( CResource* Airtanker )
{	m_VAirtankers.push_back( Airtanker );	}

// Get a CResource pointer for an airtanker by the vector index
CResource* CNatDispatcher::GetAirtankerByIndex( int index )
{	return m_VAirtankers[index];	}

// Set the vector of airtanker bases with a vector of dispatch location dispatchers
void CNatDispatcher::SetATDispatchersVector( vector< CDLDispatcher* > VATDispatcherPtrs )
{	m_VATDispatcherPtrs = VATDispatcherPtrs;	}

// Get the vector of pointers to airtanker bases
vector< CDLDispatcher* > CNatDispatcher::GetAtDispatchersVector()
{	return m_VATDispatcherPtrs;	}

// Add a CDLDispatcher pointer for an airtanker base
void CNatDispatcher::AddATDispatcher( CDLDispatcher* Base )
{	m_VATDispatcherPtrs.push_back( Base );	}

// Get a CDLDispatcher Pointer for an airtanker base by index
CDLDispatcher* CNatDispatcher::GetATDispatcherByIndex( int index )
{	return m_VATDispatcherPtrs[index];	}
	
// Determine the current resource level
vector< int > CNatDispatcher::DetermineCurRescLevel()
{
	std::vector< int > CurRescLevel;

	return CurRescLevel;

}