// DispLoc.h
// Header file for dispatch location information
// created 10/11 for IRS

#ifndef DISPLOC_H
#define DISPLOC_H

#include <vector>
#include <list>
#include "FWA.h"

class CDispLoc {
	
public:
	
	CDispLoc( string ID, int callbackdelay, vector< CFWA* > AssocFWAs, string FPU = "" , double Latitude = 0, double Longitude = 0  );		//constructor for resource type
	CDispLoc( string ID, int callbackdelay, string FPU = "", double Latitude = 0, double Longitude = 0 );	// constructor for using new FWA/Dispatcher Associations
	CDispLoc( const CDispLoc &rhs );
	~CDispLoc();									//destructor

	void SetDispLocID( string disploc );			//set the dispatch location id
	string GetDispLocID();							//get the dispatch location id
	void SetCallbackDelay( int callbackdelay );		//set the callback delay
	int GetCallbackDelay();							//get the callback delay
	void SetAssocFWAs( vector< CFWA* > AssocFWAs );	//set all the associated FWAs
	void AddAssocFWA( CFWA* FWA );					//add an associated FWA to the list
	bool RemoveAssocFWA( CFWA* FWA );				//remove an associated FWA from the list
	vector< CFWA* > GetAssocFWAs();					//get a list with the associated FWA 
	string GetAssocFWAID( int i );					//get the name for an associated FWA
	int GetAssocFWAsSize();							//get the size of the associated FWAs vector
	void SetFPU( string FPU );						//set the FPU for the dispatch location
	string GetFPU();								// get the FPU for the dispatch location
	void SetLatitude( double latitude );			//set the latitude for the Dispatch Location
	double GetLatitude();							//get the latitude for the Dispatch Location
	void SetLongitude( double longitude );			//set the longitude for the Dispatch Location
	double GetLongitude();							//get the longitude for the Dispatch Location


	bool DeterIsAssociated( CFWA* FWA );				//determine if the dispatch location is associated with the fwa
	void PrintDispLoc() const;						//print information about resource type
	bool operator==( const CDispLoc &DispLoc );		//determine if 2 dispatch location are equal
	CDispLoc& operator=( const CDispLoc &rhs );		//determine if 2 dispatch location are equal

private:
	string m_ID;									//dispatch location id
	int m_CallbackDelay;							//callback delay for dispatch location
	vector< CFWA* > m_AssocFWAs;					//list of associated FWAs
	string m_FPU;									//FPU, GACC, or national id for the dispatcher controlling the dispatch location
	double m_Latitude;								//Latitude for the dispatch location
	double m_Longitude;								//Longitude for the dispatch location
	
};		//end class CDispLoc

#endif
