// FWA.h
// class definitions for CFWA, information about individual FWAs
// 10/11 for IRS

#ifndef FWA_H
#define FWA_H

#include <string>												//C++ standard string class
using std::string;
#include <vector>												//vector class
#include<map>

#include "DispLogic.h"											//definitions for CDispLogic for dispatch logic

typedef std::multimap< double, string > AssocMap;

class CFWA {

public:
#ifdef ENABLE_IRS_TESTING
        CFWA(CDispLogic &logic):m_DispLogic(logic){}
#endif
	CFWA( string FWAid, string FMG, int walkinpct, int pumpnroll, int head, int tail, int parallel, int attackdist,
		bool waterdrops, bool excluded, double discsize, int esltime, int eslsize, double airtoground, int walkindelay[6],
		int postcontainused[6], int postcontainunused[6], int postescape[6], int reloaddelay[4], int firstunitdelay, 
		double diurnalcoeffs[24], string ROSadjfuel[10], double ROSadjustment[10], int index, CDispLogic &displogic, string FPU,
		double Latitude = 0, double Longitude = 0 );			//constructor
	CFWA( const CFWA &fwa );									//copy constructor for CFWA

	~CFWA();													//destructor

	CFWA &operator=( const CFWA &fwa);							//Assignment operator

	// Set and get functions for FWA variables
	void SetFWAID( string id );									//set the FWA ID
	string GetFWAID() const;									//get the FWA ID
	void SetFMG( string fmg );									//set the FMG the fire is in
	string GetFMG();											//get the FMG the fire is in
	void SetWalkInPct( int walkinpct );							//set the percent walk-in fires for the FWA
	int GetWalkInPct();											//get the percent walk-in fires for the FWA
	void SetPumpnRoll( int pumpnroll );							//set the percent pump and roll fires for the FWA
	int GetPumpnRoll();											//get the percent pump and roll fires for the FWA
	void SetFireAttack( int head, int tail, int parallel );		//set the percent head, tail, and parallel fires
	int GetHead();												//get the percent head fires
	int GetTail();												//get the percent tail fires
	int GetParallel();											//get the percent parallel fires
	void SetAttackDist( double attackdist );					//set the distance for parallel attack on fires in FWA
	double GetAttackDist();										//get the distance for parallel attack on fires in FWA
	void SetWaterDrops( bool waterdrops );						//set whether water drops are allowed or not
	bool GetWaterDrops();										//get whether water drops are allowed or not
	void SetExcluded( bool excluded );							//set whether the FWA is excluded from the analysis
	bool GetExcluded();											//get whether the FWA is excluded from the analysis
	void SetDiscoverySize( double discsize );					//set the discovery size for fires in the FWA
	double GetDiscoverySize();									//get the discovery size for fires in the FWA
	void SetESLTime( int esltime );								//set the escape time for fires in the FWA
	int GetESLTime();											//get the escape time for fires in the FWA
	void SetESLSize( int eslsize );								//set the escape size for fires in the FWA
	int GetESLSize();											//get the escape size for fires in the FWA
	void SetAirtoGround( double airtoground );					//set the travel distance ratio for ground resouces vs air resources in FWA
	double GetAirtoGround();									//get the travel distance ratio for ground resources vs air resources in FWA
	void SetWalkInDelays( int walkindelays[] );					//set all the walkin delays for the FWA
	void SetWalkInDelay( int i, int walkindelay );				//set a value in the walkindelay vector for the FWA 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
	int GetWalkInDelay( int i );								//get a value in the walkindelay vector for the FWA 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
	void SetPostContUsedDelays( int postdelays[] );				//set all the post contain used delays for the FWA
	void SetPostContUsedDelay( int i, int Postdelay );			//set a value in the post contain used delay vector for the FWA 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
	int GetPostContUsedDelay( int i );							//get a value in the post contain used delay vector for the FWA 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
	void SetPostContUnusedDelays( int postdelays[] );			//set all the post contain unused delays for the FWA
	void SetPostContUnusedDelay( int i, int Postdelay );		//set a value in the post contain unused delay vector for the FWA 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
	int GetPostContUnusedDelay( int i );						//get a value in the post contain unused delay vector for the FWA 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
	void SetEscapeDelays( int postdelays[] );					//set all the post escape delays for the FWA
	void SetEscapeDelay( int i, int walkindelay );				//set a value in the post escape delay vector for the FWA 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
	int GetEscapeDelay( int i );								//get a value in the post escape delay vector for the FWA 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
	void SetReloadDelays( int reloaddelays[] );					//set all the reload delays for the FWA
	void SetReloadDelay( int i, int reloaddelay );				//set a value in the reload delay vector for the FWA 0-scooper, 1-SEAT, 2-engine, 3-helicopter
	int GetReloadDelay( int i );								//get a value in the reload delay vector for the FWA 0-scooper, 1-SEAT, 2-engine, 3-helicopter
	void SetFirstUnitDelay( int firstunitdelay);				//set the first unit delay for the FWA
	int GetFirstUnitDelay();									//get the first unit delay for the FWA
	void SetDiurnalCoefficients( double diurnalcoeffs[24] );	//set all the diurnal coeffients for the FWA
	void SetDiurnalCoefficient( int i, double diurnalcoeff );	//set a diurnal coefficient for the FWA
	std::vector< double > GetDiurnalCoefficients();				//get all the diurnal coefficients for the FWA
	double GetDiurnalCoefficient( int i );						//get a diurnal coefficient for the FWA
	void SetROSFuelAdj( string fuelmodel, double adjustment );	//set a new ROS adjustment by fuel model for the FWA
	std::vector< string >GetAdjFuels();							//get a fuel models for the ROS adjustment by fuel model for the FWA
	string GetROSFuelAdj( int i );								//get the fuel model for ROS adjustment by fuel model for the FWA
	double GetAdjustment( int i );								//get the rate of spread adjustment for a fuel model by index value for the FWA 
	void SetDispLogic( CDispLogic &displogic );					//set a reference to the dispatch logic for the FWA using the DispatchLogic ID
	CDispLogic &GetDispLogic();									//get a reference to the dispatch logic for the FWA
	int GetIndex();												//get the index for the fwa to use in the dispatch logic distance array
	void SetFPU( string FPU	);									//Set the FPU the FWA resides in
	string GetFPU();											//get the FPU the FWA resides in
	void SetLatitude( double latitude );						//set the latitude for the FWA
	double GetLatitude();										//get the latitude for the FWA
	void SetLongitude( double longitude );						//set the longitude for the FWA
	double GetLongitude();										//get the longitude for the FWA
	void SetClosestATBase( string );							//set the name of the closest airtanker base
	string GetClosestATBase();									//get the name of the closest airtanker base

	static int GetCount();										//get the number of fires instantiated
	void PrintFWAInfo();										//print information about fire
	double FuelModelAdjROS( int fuelmodel, double ROS );		//Return the ROS for the fire adjusted by the FWA's Fuel Model adjustment factor
	
	void SetAssociations( AssocMap map );						// Set the association map using an association map
	void AddAssociation( string DispLoc, double distance );		// Add an association to the associated dispatch location map
	AssocMap GetAssociationMap();								// Get the Association Map
	bool IsAssociated( string DispLocID );						// Determine if a Dispatch Location is associated with the FWA
	double GetDistance( string DispLocID );						// Calculate the distance to the dispatch location using the travel time point

private:
	string m_FWAID;												//FWA id
	string m_FMG;												//Name of FMG  the FWA belongs to
	int m_WalkInPct;											//percent walk-in fires in FWA
	int m_PumpNRollPct;											//percent pump and roll fires in FWA
	int m_Head;													//percent head attack on fires in FWA
	int m_Tail;													//percent tail attack on fires in FWA
	int m_Parallel;												//Percent parallel attack on fires in FWA
	double m_AttackDist;										//Attack distance for parallel attack fires ( and head fires where attack distance <> 0)
	bool m_WaterDrops;											//Are water drops allowed in the FWA?
	bool m_Excluded;											//Is FWA excluded from the analysis?
	double m_DiscoverySize;										//Discovery size for fires in the FWA
	int m_ESLTime;												//Escape time for fires in the FWA
	int m_ESLSize;												//Escape size for fires in the FWA
	double m_AirtoGround;										//travel distance ratio for ground travel vs air travel
	std::vector< int > m_WalkInDelays;							//vector of walkin delays for 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
	std::vector< int > m_PostContainedUsed;						//vector of post contain used delays for 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
	std::vector< int > m_PostContainedUnused;					//vector of post contain unused delays for 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
	std::vector< int > m_PostEscape;							//vector of post escape delays for 0-tracked, 1-boat, 2-crew, 3-engine, 4-Helitack, and 5-Smokejumpers
	std::vector< int > m_ReloadDelay;							//vector of reload dealys for 0-scooper, 1-SEAT, 2-engine, 3-helicopter, 4-airtanker
	int m_FirstUnitDelay;										//first unit delay
	std::vector< double > m_DiurnalCoeffs;						//vector of diurnal coefficients for hours of the day 0-23
	std::vector< string > m_ROSAdjFuel;							//vector of fuel model names for rate of spread adjustments (up to 10) for FWA
	std::vector< double > m_ROSAdjustment;						//vector of adjustment factors for rate of spread adjustments (up to 10) for FWA that correspond to m_ROSAdjFuel models with same index
	string m_DispLogicID;										//id for dispatch logic used by the FWA
	CDispLogic &m_DispLogic;									//dispatch logic for the FWA
	int m_Index;												//index number in Dispatch Logic array for travel distances
	string m_FPU;												//name of FPU the FWA resides in
	double m_Latitude;
	double m_Longitude;
	string m_ClosestATBase;										//stores the name of the airtanker base dispatch location dispatcher that is closest to the FWA's TTP
	AssocMap m_AssocMap;										//stores the distance and id of the associated dispatch locations
	
	static int count;											//number of objects instantiated

	
};	//end class CFWA

#endif	// FWA_H
