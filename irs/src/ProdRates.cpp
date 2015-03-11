// ProdRates.cpp
// function definitions for the class CProdRates that define the production rates for a resource/fire combination
// for IRS 11/11

#include <string>
using std::string;
#include <iostream>
using namespace std;


#include "ProdRates.h"


// Constructor for CProdRates class
CProdRates::CProdRates( string resctype, int slopecat, int staffing, int fuelmodel, string spcondition, double prodrate )
{
	m_RescType = resctype;
	m_SlopeCat = slopecat;
	m_Staffing = staffing;
	m_FuelModel = fuelmodel;
	m_SpecificCondition = spcondition;
	m_ProdRate = prodrate;
}

// Copy constructor for CProdRates
CProdRates::CProdRates( const CProdRates &prodrates )
{
	m_RescType = prodrates.m_RescType;
	m_SlopeCat = prodrates.m_SlopeCat;
	m_Staffing = prodrates.m_Staffing;
	m_FuelModel = prodrates.m_FuelModel;
	m_SpecificCondition = prodrates.m_SpecificCondition;
	m_ProdRate = prodrates.m_ProdRate;

}

// Destructor for CProdRates
CProdRates::~CProdRates() {}

CProdRates& CProdRates::operator=(const CProdRates &rhs)
{
	if (&rhs != this)	{
		m_RescType = rhs.m_RescType;
		m_SlopeCat = rhs.m_SlopeCat;
		m_Staffing = rhs.m_Staffing;
		m_FuelModel = rhs.m_FuelModel;
		m_SpecificCondition = rhs.m_SpecificCondition;
		m_ProdRate = rhs.m_ProdRate;
	}

	return *this;
}

// Compare with instance of class and return production rate if match else -1
double CProdRates::Compare( string resctype, int slopecat, int staffing, int fuelmodel, string spcondition )
{
	if ( m_RescType == resctype && m_Staffing == staffing && m_FuelModel == fuelmodel && m_SpecificCondition == spcondition ) {
		if ( m_SlopeCat == 0 || m_SlopeCat == slopecat )
			return m_ProdRate;	}
		
	return -1.0;
}



// Print an instance in the class CProdRate
void CProdRates::PrintProdRate()
{
	cout << " Resource Type: " << m_RescType << "\n";
	cout << "	Slope Category: " << m_SlopeCat << "\n";
	cout << "	Staffing: " << m_Staffing << "\n";
	cout << "	Fuel Model: " << m_FuelModel << "\n";
	cout << "	Specific Condition: ";
	if ( m_SpecificCondition != "x" )
		cout << m_SpecificCondition;
	cout << "\n";
	cout << "	Production Rate: " << m_ProdRate << "\n";
}