// LocalDispatcher.cpp
// Member function definitions for class CLocalDispatcher Local Area dispatcher from CDispatchBase
// created 1/13 for IRS

#include <iostream>													//contains functions for input and output
using namespace std;
#include <string>													//contains functions for operations with strings
using std::string;
#include <cstring>													//to convert a string to a const char*
#include <cstdlib>													//to use atof to convert const char* to int
#include <vector>
#include <list>

// Include definition for class CNatDispatcher from NatDispatcher.h
#include "LocalDispatcher.h"
// Include definition for class CDispatchBase from DispatchBase.h
#include "DispatchBase.h"


// Default Constructor for CLocalDispatcher
CLocalDispatcher::CLocalDispatcher() : CDispatchBase()
{
	
}

// Constructor for CNatDispatcher
CLocalDispatcher::CLocalDispatcher( string dispatcherID ) : CDispatchBase( dispatcherID )
{

}

// Copy constructor for CNatDispatcher
CLocalDispatcher::CLocalDispatcher( const CLocalDispatcher &localdispatcher ) : CDispatchBase ( localdispatcher )
{
	
}

// Destructor for CDLDispatcher
CLocalDispatcher::~CLocalDispatcher()
{}

// Assignment Operator
CLocalDispatcher& CLocalDispatcher::operator=(const CLocalDispatcher &rhs)
{
	if (&rhs != this)
		CDispatchBase::operator=(rhs);

	return *this;
}

// Determine the current resource level
vector< int > CLocalDispatcher::DetermineCurRescLevel()
{
	std::vector< int > CurRescLevel;

	return CurRescLevel;

}