// CRescType.cpp
// Member functions for class CRescType holding resource type information
// created 10/11 for IRS

#include <iostream>											//contains input and output functions
using namespace std;
#include <string>											//contains functions for operations with strings
using std::string;


// Include Resouce Type class definitions from CRescType.h
#include "CRescType.h"


// CRescType default constructor
CRescType::CRescType()
{
	m_Type = "";
	m_AvgSpeed = 0;
	m_DispatchDelay = 0;
	m_ResponseDelay = 0;
	m_SetupDelay = 0;
}

// CRescType destructor
CRescType::~CRescType() {}

// CRescType constructor 
CRescType::CRescType( string Type, int Avg_Speed, int Disp_Delay, int Resp_Delay, int Setup_Delay)
{
	m_Type = Type;
	m_AvgSpeed = Avg_Speed;;
	m_DispatchDelay = Disp_Delay;
	m_ResponseDelay = Resp_Delay;
	m_SetupDelay = Setup_Delay;
}
/**
 * \brief Copy constructor
 * \author Kyle Shannon <kyle@pobox.com>
 * \date 2012-09-11
 * \param rhs object to copy
 */

CRescType::CRescType( const CRescType &rhs )
{
    m_Type = rhs.m_Type;
    m_AvgSpeed = rhs.m_AvgSpeed;
    m_DispatchDelay = rhs.m_DispatchDelay;
    m_ResponseDelay = rhs.m_ResponseDelay;
    m_SetupDelay = rhs.m_SetupDelay;
}

// Set the resource type
void CRescType::SetRescType( string resctype )
{	m_Type = resctype;	}

// Get the resource type
string CRescType::GetRescType()
{	return m_Type;	}

// Set the average speed for the resource type
void CRescType::SetAvgSpeed( int speed )
{	m_AvgSpeed = speed;	}

// Get the average speed for the resource type
int CRescType::GetAvgSpeed()
{	return m_AvgSpeed;	}

// Set the dispatch delay for the resource type
void CRescType::SetDispatchDelay( int delay )
{	m_DispatchDelay = delay;	}

// Get the dispatch delay for the resource type
int CRescType::GetDispatchDelay()
{	return m_DispatchDelay;	}

// Set the response delay for the resource type
void CRescType::SetResponseDelay( int delay )
{	m_ResponseDelay = delay;	}

// Get the response delay for the resource type
int CRescType::GetResponseDelay()
{	return m_ResponseDelay;	}

// Set the setup delay for the resource type
void CRescType::SetSetupDelay( int delay )
{	m_SetupDelay = delay;	}

// Get the setup delay for the resource type
int CRescType::GetSetupDelay()
{	return m_SetupDelay;	}

// Calculate protion of arrival delay due to dispatch, response, and set-up delays
int CRescType::PreConstructDelay()
{
	return m_DispatchDelay + m_ResponseDelay + m_SetupDelay;
}

// Print information about the resource type
void CRescType::PrintRescType()
{
	cout << "Resource Type: " << m_Type << "\n";
	cout << "	Average Speed: " << m_AvgSpeed << "\n";
	cout << "	Dispatch Delay: " << m_DispatchDelay << "\n";
	cout << "	Resource Response Delay: " << m_ResponseDelay << "\n";
	cout << "	Set-up Delay: " << m_SetupDelay << "\n";

	return;
}
