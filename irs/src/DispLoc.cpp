// CDispLoc.cpp
// Member functions for class CDispLoc holding dispatch location type information
// created 10/11 for IRS

#include <iostream>											//contains input and output functions
using namespace std;
#include <string>											//contains functions for operations with strings
using std::string;
#include <vector>
#include<list>



// Include Resouce Type class definitions from CRescType.h
#include "DispLoc.h"

// Constructor
CDispLoc::CDispLoc( string ID, int Callback_Delay, vector< CFWA* > AssocFWAs, string FPU, double Latitude, double Longitude )
{
	m_ID = ID;
	m_CallbackDelay = Callback_Delay;
	m_Latitude = Latitude;
	m_AssocFWAs = AssocFWAs;
	m_Longitude = Longitude;
	m_FPU = FPU;
}

// Constructor for modified FWA/Dispatch Location Associations
CDispLoc::CDispLoc( string ID, int Callback_Delay, string FPU, double Latitude, double Longitude )
{
	m_ID = ID;
	m_CallbackDelay = Callback_Delay;
	m_Latitude = Latitude;
	m_Longitude = Longitude;
	m_FPU = FPU;
}

/**
 * \brief Copy constructor
 * \author Kyle Shannon <kyle@pobox.com>
 * \date 2012-09-11
 * \param rhs object to copy
 */
CDispLoc::CDispLoc( const CDispLoc &rhs )
{
    m_ID = rhs.m_ID;
    m_CallbackDelay = rhs.m_CallbackDelay;
    m_AssocFWAs = rhs.m_AssocFWAs;
    m_Latitude = rhs.m_Latitude;
    m_Longitude = rhs.m_Longitude;
    m_FPU = rhs.m_FPU;
}

/**
 * \brief equals operator
 * \author Kyle Shannon <kyle@pobox.com>
 * \date 2013-05-07
 * \param object to copy
 * \return copied object
 */
CDispLoc & CDispLoc::operator=( const CDispLoc &rhs )
{
    if( &rhs != this )
    {
        m_ID = rhs.m_ID;
        m_CallbackDelay = rhs.m_CallbackDelay;
        m_AssocFWAs = rhs.m_AssocFWAs;
        m_Latitude = rhs.m_Latitude;
        m_Longitude = rhs.m_Longitude;
        m_FPU = rhs.m_FPU;
    }
    return *this;
}

// Equivalence operator for CDispLoc
bool CDispLoc::operator==( const CDispLoc &DispLoc )
{
	if ( m_ID != DispLoc.m_ID )
		return false;
	if ( m_CallbackDelay != DispLoc.m_CallbackDelay )
		return false;
	if (m_AssocFWAs != DispLoc.m_AssocFWAs )
		return false;
	if ( m_Latitude != DispLoc.m_Latitude )
		return false;
	if ( m_Longitude != DispLoc.m_Longitude )
		return false;
	if (m_FPU != DispLoc.m_FPU )		
		return false;					

	return true;
}

// Destructor for CCispLoc
CDispLoc::~CDispLoc() {}

// Set Dispatch location ID
void CDispLoc::SetDispLocID( string disploc )
{	m_ID = disploc;	}

// Get Dispatch location ID
string CDispLoc::GetDispLocID()
{	return m_ID;	}

// Set the callback delay
void CDispLoc::SetCallbackDelay( int callbackdelay)
{	m_CallbackDelay = callbackdelay;	}

// Get the callback delay
int CDispLoc::GetCallbackDelay()
{	return m_CallbackDelay;	}

//	Set all the associated FWAs
void CDispLoc::SetAssocFWAs( vector< CFWA* > AssocFWAs )
{	m_AssocFWAs = AssocFWAs;	}

// Add an associated FWA to the list
void CDispLoc::AddAssocFWA( CFWA* FWA )
{	m_AssocFWAs.push_back( FWA );	}

// Remove and Associated FWA from the list
bool CDispLoc::RemoveAssocFWA( CFWA* FWA )
{	
	// Determine if the FWA is in the list
	bool found = false;
	int pos = -1;

	for ( int i = 0; i < m_AssocFWAs.size(); i++ )	{
		
		if ( FWA == m_AssocFWAs[i] )	{
			found = true;
			pos = i;
		}

	}

	if ( found )	{

		m_AssocFWAs.erase( m_AssocFWAs.begin() + pos );
		return true;

	}

	else
		return false;

}

// Get a list of the associated FWA
vector< CFWA* > CDispLoc::GetAssocFWAs()
{	return m_AssocFWAs;	}

// Get the name for an associated FWA
string CDispLoc::GetAssocFWAID( int i )
{	return m_AssocFWAs[i]->GetFWAID();	}

// Get the size of the associated FWAs vector
int CDispLoc::GetAssocFWAsSize()
{	return m_AssocFWAs.size();	}

// Set the FPU for the Dispatch Location
void CDispLoc::SetFPU( string FPU )
{	m_FPU = FPU;	}

// Get the FPU for the Dispatch Location
string CDispLoc::GetFPU()
{	return m_FPU;	}

// Set the Dispatch Location's Latitude
void CDispLoc::SetLatitude( double Latitude )
{	m_Latitude = Latitude;	}

// Get the Dispatch Location's Latitude
double CDispLoc::GetLatitude()
{	return m_Latitude;	}

// Set the Dispatch Location's Longitude
void CDispLoc::SetLongitude( double Longitude )	
{	m_Longitude = Longitude;	}

// Get the Dispatch Location's Longitude
double CDispLoc::GetLongitude()
{	return m_Longitude;	}


// Print information about the dispatch location
void CDispLoc::PrintDispLoc() const
{
	cout << "Dispatch Location ID: " << m_ID << "\n";
	cout << "	Callback Delay for dispatch location: " << m_CallbackDelay << "\n";
	//cout << "	Air Distance for: ";
	//for ( int i = 0; i < static_cast< int > (m_AssocFWAs.size()); i++ )	{
		//cout << "	FWA " << i << " " << m_FWAAssocs[i] << "\n";
	//}

	cout << "	Latitude: " << m_Latitude << "\n";
	cout << "	Longitude: " << m_Longitude << "\n";
	cout << "	FPU: " << m_FPU << "\n";
	cout << "	Associated FWAs\n";
	for ( int i = 0; i < m_AssocFWAs.size(); i++ )
		cout << "	" << m_AssocFWAs[i]->GetFWAID() << "\n";


	return;

}

bool CDispLoc::DeterIsAssociated( CFWA* FWA )
{
	// Is the FWA pointer in the associated vector
	bool found = false;

    for(int i = 0; i < m_AssocFWAs.size(); i++)
    {
		if ( FWA == m_AssocFWAs[i] )
			found = true;

    }
    return found;
}
