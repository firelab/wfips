// ProdRates.h
// This class contains the production rates for the different resource types in the different fuel models
// for IRS 11/11

#ifndef PRODRATES_H
#define PRODRATES_H

#include <string>
using std::string;

class CProdRates	{

public:
	CProdRates( string resctype, int slopecat, int staffing, int fuelmodel, string spcondition, double prodrate );
	CProdRates( const CProdRates &prodrates );
	~CProdRates();													//destructor

	double GetProdRate();											//get the production rate for the entry
	
	// Compare with CProdRates instance to determine if match then return production rate
	double Compare( string resctype, int slopecat, int staffing, int fuelmodel, string spcondition );
	void PrintProdRate();											//print and object in the production rate class

private:
	string m_RescType;												//resource type for entry
	int m_SlopeCat;													//slope category for fire
	int m_Staffing;													//staffing for resource
	int m_FuelModel;												//fuel model for the fire
	string m_SpecificCondition;										//specific condition for the fuel model, x if none necessary
	double m_ProdRate;												//production rate for all the resource/fire conditions listed above

};	// End class CProdRates

#endif // end PRODRATES_H


