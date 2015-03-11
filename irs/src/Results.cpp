// Results.cpp
// function definitions for the class CResults that stores the contain algorithm results
// for IRS 12/11

#include <iostream>
using namespace std;
#include <string>

#include "Fire.h"
#include "Results.h"



// Constructor for the class CResults
CResults::CResults( CFire fire, double firecost, double fireline, double fireperimeter, double firesize, double firesweep, 
	double firetime, double numrescused, string status, bool displogicfilled, bool inseason ) : m_Fire( fire )
{
	
	m_FinalFireCost = firecost;
  	m_FinalFireLine = fireline;
  	m_FinalFirePerimeter = fireperimeter;
  	m_FinalFireSize = firesize;
  	m_FinalFireSweep = firesweep;
  	m_FinalFireTime = firetime;
  	m_FinalResourcesUsed = numrescused;
	m_Status = status;
	m_DispLogicFilled = displogicfilled;
	m_InSeason = inseason;

}

CResults::CResults( const CResults &results ) : m_Fire( results.m_Fire )
{
	m_FinalFireCost = results.m_FinalFireCost;
	m_FinalFireLine = results.m_FinalFireLine;
	m_FinalFirePerimeter = results.m_FinalFirePerimeter;
	m_FinalFireSize = results.m_FinalFireSize;
	m_FinalFireSweep = results.m_FinalFireSweep;
	m_FinalFireTime = results.m_FinalFireTime;
	m_FinalResourcesUsed = results.m_FinalResourcesUsed;
	m_Status = results.m_Status;
	m_DispLogicFilled = results.m_DispLogicFilled;
	m_InSeason = results.m_InSeason;

}

// Destructor for class CResults
CResults::~CResults() {}

CResults& CResults::operator=(const CResults  &rhs)
{
	if(&rhs != this)	{
		m_Fire = rhs.m_Fire;
		m_FinalFireCost = rhs.m_FinalFireCost;
		m_FinalFireLine = rhs.m_FinalFireLine;
		m_FinalFirePerimeter = rhs.m_FinalFirePerimeter;
		m_FinalFireSize = rhs.m_FinalFireSize;
		m_FinalFireSweep = rhs.m_FinalFireSweep;
		m_FinalFireTime = rhs.m_FinalFireTime;
		m_FinalResourcesUsed = rhs.m_FinalResourcesUsed;
		m_Status = rhs.m_Status;
		m_DispLogicFilled = rhs.m_DispLogicFilled;
		m_InSeason = rhs.m_InSeason;
	}

	return *this;
}

// Set pointer to fire
//void CResults::SetFire( CFire &fire )		// Need to create a copy constructor in order to do this
//{	m_Fire = fire;	}

// Get point to fire
CFire CResults::GetFire()
{	return m_Fire;	}

// Set final fire costs
void CResults::SetFireCost( double firecost )
{	m_FinalFireCost = firecost;	}

// Get final fire costs
double CResults::GetFireCost()
{	return m_FinalFireCost;	}

// Set final length of fireline
void CResults::SetFireLine( double fireline )
{	m_FinalFireLine = fireline;	}

// Get final length of fireline
double CResults::GetFireLine()
{	return m_FinalFireLine;	}

// Set final fire perimeter
void CResults::SetFirePerimeter( double fireperimeter )
{	m_FinalFirePerimeter = fireperimeter;	}

// Get final fire perimeter
double CResults::GetFirePerimeter()
{	return m_FinalFirePerimeter;	}

// Set final fire size
void CResults::SetFireSize( double firesize )
{	m_FinalFireSize = firesize;	}

// Get final fire size
double CResults::GetFireSize()
{	return m_FinalFireSize;	}

// Set final fire sweep
void CResults::SetFireSweep( double firesweep )
{	m_FinalFireSweep = firesweep;	}

// Get final fire sweep
double CResults::GetFireSweep()
{	return m_FinalFireSweep;	}

// Set final fire time
void CResults::SetFireTime( double firetime )
{	m_FinalFireTime = firetime;	}

// Get final fire time
double CResults::GetFireTime()
{	return m_FinalFireTime;	}

// Set number of resources used
void CResults::SetNumRescUsed( double numrescused )
{	m_FinalResourcesUsed = numrescused;	}

// Get number of resources used
double CResults::GetNumRescUsed()
{	return m_FinalResourcesUsed;	}

// Set the fire status
void CResults::SetStatus( string status )
{	m_Status = status;	}

// Get the fire status
string CResults::GetStatus()
{	return m_Status;	}

// Set the dispatch logic filled indicator
void CResults::SetDispLogicFilled( bool displogicfilled )
{	m_DispLogicFilled = displogicfilled;	}

// Get the dispatch logic filled indicator
bool CResults::GetDispLogicFilled()
{	return m_DispLogicFilled;	}

// Set the level 3 resources in season flag
void CResults::SetInSeason( bool inseason )
{	m_InSeason = inseason;	}

// Get the level 3 in resources in season flag
bool CResults::GetInSeason()
{	return m_InSeason;	}

// Print results
void CResults::PrintResults()
{
	int firenum = m_Fire.GetFireNumber();
	cout << "Results for fire number: " << firenum << "\n";
	cout << "	Fire Costs: " << m_FinalFireCost << "\n";
	cout << "	Fire Line: " << m_FinalFireLine << "\n";
	cout << "	Fire Perimeter: " << m_FinalFirePerimeter << "\n";
	cout << "	Fire Size: " << m_FinalFireSize << "\n";
	cout << "	Fire Sweep: " << m_FinalFireSweep << "\n";
	cout << "	Fire Time: " << m_FinalFireTime << "\n";
	cout << "	Number of Resources Used: " << m_FinalResourcesUsed << "\n";
	cout << "	Status: " << m_Status << "\n";
	cout << "	Dispatch Logic Filled? " << m_DispLogicFilled << "\n";
	cout << "	Level 3 Resources in season? " << m_InSeason << "\n";
}
