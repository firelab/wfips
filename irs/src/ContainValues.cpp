// ContainValues.cpp
// Function defintions for the class CContainValues 
// for IRS 11/11

#include <iostream>
using namespace std;
#include <string>

#include "ContainValues.h"


// Constructor for the contain values
CContainValues::CContainValues( int starttime, double prodrate, int duration, string description, double daily, double hourly )
{
	m_StartTime = starttime;
	m_ProdRate = prodrate;
	m_Duration = duration;
	m_Description = description;
	m_DailyCost = daily;
	m_HourlyCost = hourly;
}

// Copy constructor for CContainValues
CContainValues::CContainValues( const CContainValues &containvalues )
{
	m_StartTime = containvalues.m_StartTime;
	m_ProdRate = containvalues.m_ProdRate;
	m_Duration = containvalues.m_Duration;
	m_Description = containvalues.m_Description;
	m_DailyCost = containvalues.m_DailyCost;
	m_HourlyCost = containvalues.m_HourlyCost;

}

// Destructor for CContainValues
CContainValues::~CContainValues(){}

// Set the start time for the segment
void CContainValues::SetStartTime( int starttime )
{	m_StartTime = starttime;	}

// Get the start time for the segment
int CContainValues::GetStartTime()
{	return m_StartTime;	}

// Set the production rate for the segment
void CContainValues::SetProdRate( double prodrate )
{	m_ProdRate = prodrate;	}

// Get the production rate for the segment
double CContainValues::GetProdRate()
{	return m_ProdRate;	}

// Set the duration for the segment
void CContainValues::SetDuration( int duration)
{	m_Duration = duration;	}

// Get the duration for the segment
int CContainValues::GetDuration()
{	return m_Duration;	}

// Set the description for the segment
void CContainValues::SetDescription( string description )
{	m_Description = description;	}

// Get the description for the segment
string CContainValues::GetDescription()
{	return m_Description;	}

// Set daily cost for the resource
void CContainValues::SetDailyCost( double dailycost )
{	m_DailyCost = dailycost;	}

// Get daily cost for the resource
double CContainValues::GetDailyCost()
{	return m_DailyCost;	}

// Set the hourly cost for the resource
void CContainValues::SetHourlyCost( double hourlycost )
{	m_HourlyCost = hourlycost;	}

// Get the hourly cost for the resource
double CContainValues::GetHourlyCost()
{	return m_HourlyCost;	}


// print the contain values for the segment
void CContainValues::PrintValues()
{
	cout << "Contain Values for the resource: Start Time: " << m_StartTime << " Production Rate: " << m_ProdRate << " Duration: " 
		<< m_Duration << " Description: " << m_Description <<  "\n";
}