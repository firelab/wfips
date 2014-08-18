// ContainValues.h
// class definitions for the resource fireline production values that are input into the contain algorithm
// for IRS 11/11

#ifndef CONTAINVALUES_H
#define CONTAINVALUES_H

class CContainValues	{

public:
	CContainValues( int starttime, double prodrate, int duration, string description, double daily, double hourly );		// constructor for the class CContainValues
	CContainValues( const CContainValues &containvalues );
	~CContainValues();
	
	void SetStartTime( int starttime );								// set the start time for the contain segment
	int GetStartTime();												// get the start time for the segment
	void SetProdRate( double prodrate );							// set the production rate for the segment
	double GetProdRate();											// get the production rate for the segment
	void SetDuration( int duration );								// set the duration for the segment
	int GetDuration ();												// get the duration for the segment
	void SetDescription( string description );						// set the description for the resource
	string GetDescription();										// get the description for the resource
	void SetDailyCost( double dailycost );							// set the daily cost for the resource
	double GetDailyCost();											// get the daily cost for the resource
	void SetHourlyCost( double hourlycost );						// set the hourly cost for the resoruce
	double GetHourlyCost();											// get the hourly cost for the resource

	void PrintValues();												// print the contain values


private:
	int m_StartTime;												// start time for the segement of fireline production
	double m_ProdRate;												// production rate for the segment of fireline production
	int m_Duration;													// duration time for the segment of fireline production
	string m_Description;											// description of the resource
	double m_DailyCost;												// daily cost for the resource
	double m_HourlyCost;												// hourly cost for the resource

};	// End class CContainValues

#endif /* CONTAINVALUES_H */
