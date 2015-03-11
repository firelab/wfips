// RunScenario.cpp
// CRunScenario member function definitions
// for IRS 11/11

#include <iostream>											//contains functions for inputting and outputting
#include <fstream>											//contains functions for inputting and outputting to a file
using std::ifstream;										//for inputting from a file
#include <sstream>
#include <cctype>											//contains functions for characters, strings, and structures

using namespace std;

#include <cstring>											//contains string functions
#include <cstdlib>											//contains function prototype for rand
#include <ctime>											//contains functions for manipulating time and date
#include <vector>											//contains vector functions
#include <list>												//contains list functions
#include <map>												//contains map functions
#include <set>												//contains set functions
#include <map>
//#include <unordered_map>								// contains functions to locate items using a key

#include "Contain.h"
#include "ContainResource.h"
#include "ContainSim.h"
#include "ContainForce.h"
#include <math.h>
//#include <conio.h>

// Include tree definitions from omffr_tree.h
#include "omffr_tree.h"
// Include tree definitions for the CDispatcher class DispatchBase.h, DLDispatcher.h
#include "DispatchBase.h"
#include "NatDispatcher.h"
#include "GACCDispatcher.h"
#include "LocalDispatcher.h"
#include "DLDispatcher.h"
#include "TypeTwoIACrew.h"


static const double PI=acos(-1.0);


// Include definition for class CRescType from CRescType.h
#include "CRescType.h"
// Include definition for class CDispLoc from DispLoc.h
#include "DispLoc.h"
// Include definition for class CResource from CRescbase.h
#include "Rescbase.h"
// Include definiton for class CGround from ground.h
#include "Ground.h"
// Include definition for class CFWA in FWA.h
#include "FWA.h"
// Include definition for class CFire in Fire.h
#include "Fire.h"
// Include definition for class CDispLogic in DispLogic.h
#include "DispLogic.h"
// Include definition for class CProdRates in ProdRates.h
#include "ProdRates.h"
// Include definition for class CConstProd in ConstProd.h
#include "ConstProd.h"
// Include definition for class CEngine in Engine.h
#include "Engine.h"
// Include definition for class CCrew in Crew.h
#include "Crew.h"
// Include definition for class CWaterTender in WaterTender.h
#include "WaterTender.h"
// Include definition for class CAirtanker in Airtanker.h
#include "Airtanker.h"
// Include definition for class CSmallAT in SmallAT.h
#include "SmallAT.h"
// Include definition for class CSmokejumper in Smokejumper.h
#include "Smokejumper.h"
// Include definiton for class CSMJAircraft in SMJAircraft.h
#include "SMJAircraft.h"
// Include definition for class CHelicopter in Helicopter.h
#include "Helicopter.h"
// Include definition for class CHelitack in Helitack.h
#include "Helitack.h"
// Include definition for class CRunScenario in RunScenario.h
#include "RunScenario.h"
// Include definitions for class CContainValuse in ContainValues.h
#include "ContainValues.h"
// Include definitions for class CResults in Results.h

/* Silence warnings that aren't our fault */
#if defined(OMFFR) && defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wwrite-strings"
#pragma GCC push
#endif /* defined(OMFFR) && defined(__GNUC__) */
PrepositionStruct::PrepositionStruct()
{
    rescType = "";
    level = 1.0;
    outOfSeason = false;
}

PrepositionStruct & PrepositionStruct::operator = ( const PrepositionStruct &rhs )
{
    if( &rhs != this )
    {
        rescType = rhs.rescType;
        level = rhs.level;
        outOfSeason = rhs.outOfSeason;
    }
    return *this;
}

PrepositionStruct::PrepositionStruct( const PrepositionStruct &rhs )
{
    rescType = rhs.rescType;
    level = rhs.level;
    outOfSeason = rhs.outOfSeason;
}

// Default constructor for CRunScenario
CRunScenario::CRunScenario( ) : m_VDispLogic(), m_VFWA(), m_VRescType(), m_VDispLoc(), m_VProdRates(), m_VResource(), m_VFire(), m_VResults(), m_DispTree(), m_DispMap()  
{
	m_NumDispLogic = 0;
	m_NumFWA = 0;
	m_NumRescType = 0;
	m_NumDispLoc = 0;
	m_NumProdRates = 0;
	m_NumResource = 0;
	m_NumFire = 0;
	m_NumResults = 0;
	m_EscapeIdx = -1;

        /* Preposition stuff */
	ATTpp = PrepositionStruct();
	RCRWpp = PrepositionStruct();
	RHelpp = PrepositionStruct();
	FSCRWpp = PrepositionStruct();
	DOICRWpp = PrepositionStruct();
	FSENGpp = PrepositionStruct();
	DOIENGpp = PrepositionStruct();

	FSCRWdd = PrepositionStruct();
	DOICRWdd = PrepositionStruct();
	FSDZRdd = PrepositionStruct();
	DOIDZRdd = PrepositionStruct();
	FSENGdd = PrepositionStruct();
	DOIENGdd = PrepositionStruct();
	FSSEATdd = PrepositionStruct();
	DOISEATdd = PrepositionStruct();


}

// Destructor for CRunScenario
CRunScenario::~CRunScenario()
{
	// m_VResource is the only vector of pointers so need to delete all the CResource objects
	if ( m_VResource.size() > 0 )	{
		for ( int i = 0; i < m_VResource.size(); i++ )
			delete m_VResource[i];
	}

	//***** m_DispTree also needs to have the dispatchers deleted from it.
	if( m_DispTree.Size() > 0 && m_DispTree.Root() != NULL)
    {
       std::vector<OmffrNode< CDispatchBase* >*> nodes = m_DispTree.Preorder(m_DispTree.Root());
       for( int i = 0; i < nodes.size(); i++ )	{
		   //Is the node a GACC Dispatcher?
		   CGACCDispatcher *GACCDispatch = dynamic_cast< CGACCDispatcher* >( m_DispTree.Element( nodes[i] ) );

		   if ( GACCDispatch !=0 )	{		// Is a GACC Dispatcher
			   // Get the vector of Type II IA Crews and iterate through each
			   vector< CTypeTwoIACrew > TypeTwoCrewVector = GACCDispatch->GetTypeTwoCrewVector();
			   for ( int k = 0; k < TypeTwoCrewVector.size(); k++ )	{
				   // Need to delete the IA crews  do it in RunScenario
				   vector< CResource* > VIACrews = TypeTwoCrewVector[k].GetIACrewsVector();
				   for ( int j = 0; j < VIACrews.size(); j++ )
					delete VIACrews[j];
			   }
		   }

           delete m_DispTree.Element(nodes[i]);

		}
	}
}

CRunScenario::CRunScenario(const CRunScenario &rhs)
{
    m_VDispLogic = std::vector<CDispLogic>(rhs.m_VDispLogic);
    m_VFWA = std::vector<CFWA>(rhs.m_VFWA);
    m_VRescType = std::vector<CRescType>(rhs.m_VRescType);
    m_VDispLoc = std::vector<CDispLoc>(rhs.m_VDispLoc);
    m_VProdRates = std::vector<CProdRates>(rhs.m_VProdRates);
    m_VResource = std::vector<CResource*>(rhs.m_VResource);
    m_VFire = std::vector<CFire>(rhs.m_VFire);
    m_VResults = std::vector<CResults>(rhs.m_VResults);
    //Diane's comment
    //std::vector< CFPU > m_VFPU;

	m_NumDispLogic = rhs.m_NumDispLogic;
    m_NumFWA = rhs.m_NumFWA;
    m_NumRescType = rhs.m_NumRescType;
    m_NumDispLoc = rhs.m_NumDispLoc;
    m_NumProdRates = rhs.m_NumProdRates;
    m_NumResource = rhs.m_NumResource;
    m_NumFire = rhs.m_NumFire;
    m_NumResults = rhs.m_NumResults;

    m_DispTree = rhs.m_DispTree;
    m_DispMap = rhs.m_DispMap;
    m_InMap = rhs.m_InMap;
	m_FWAMap = rhs.m_FWAMap;
	
    m_NumDispLogic = rhs.m_NumDispLogic;
    m_NumFWA = rhs.m_NumFWA;
    m_NumRescType = rhs.m_NumRescType;
    m_NumDispLoc = rhs.m_NumDispLoc;
    m_NumProdRates = rhs.m_NumProdRates;
    m_NumResource = rhs.m_NumResource;
    m_NumFire = rhs.m_NumFire;
    m_NumResults = rhs.m_NumResults;

    m_RegionalCrewDLs = std::vector<string>(rhs.m_RegionalCrewDLs);
    m_RegionalHelicopterDLs = std::vector<string>(rhs.m_RegionalHelicopterDLs);
    m_SmokejumperDLs = std::vector<string>(rhs.m_SmokejumperDLs);
    m_AirtankerDLs = std::vector<string>(rhs.m_AirtankerDLs);
	m_FSCRWDLs = std::vector<string>(rhs.m_FSCRWDLs);
	m_DOICRWDLs = std::vector<string>(rhs.m_DOICRWDLs);
	m_FSENGDLs = std::vector<string>(rhs.m_FSENGDLs);
	m_DOIENGDLs = std::vector<string>(rhs.m_DOIENGDLs);
	m_DOIDZRDLs = rhs.m_DOIDZRDLs;
	m_DOIHelDLs = rhs.m_DOIHelDLs;
	m_DOIHELIDLs = rhs.m_DOIHELIDLs;
	m_DOISEATDLs = rhs.m_DOISEATDLs;
	m_FSDZRDLs = rhs.m_FSDZRDLs;
	m_FSHelDLs = rhs.m_FSHelDLs;
	m_FSHELIDLs = rhs.m_FSHELIDLs;
	m_FSSEATDLs = rhs.m_FSSEATDLs;

    m_BorrowedResources = rhs.m_BorrowedResources;
    m_VEscapes = std::vector<CEscape>(rhs.m_VEscapes);
    int m_EscapeIdx;

	ATTpp = rhs.ATTpp;
	RCRWpp = rhs.RCRWpp;
	RHelpp = rhs.RHelpp;
	FSCRWpp = rhs.FSCRWpp;
	DOICRWpp = rhs.DOICRWpp;
	FSENGpp = rhs.FSENGpp;
	DOIENGpp = rhs.DOIENGpp;

	FSCRWdd = rhs.FSCRWdd;
	DOICRWdd = rhs.DOICRWdd;
	FSDZRdd = rhs.FSDZRdd;
	DOIDZRdd = rhs.DOIDZRdd;
	FSENGdd = rhs.FSENGdd;
	DOIENGdd = rhs.DOIENGdd;
	FSSEATdd = rhs.FSSEATdd;
	DOISEATdd = rhs.DOISEATdd;

}

// Generate random number between 0 and 100
int CRunScenario::GetRand()
{
	int i = rand() % 100 + 1;

	return i;
}                                                                                               

//Read in dispatch logic data from file DispLogic.dat
void CRunScenario::ReadDispLogicFile( std::string oFilename )
{
	//if constructor opens a file
	ifstream inDispLogicFile( oFilename.c_str(), ios::in );

	//exit program if ifstream could not open file
	if ( !inDispLogicFile ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}

	// initialize dispatch logic
	string displogicid( "Default" );
	string index( "BI" );
	int nlevels = 5;
	int breakpts[5] = { 20, 40, 60 , 80, 100 };
	int numresc [13][5]= { 0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0, 
					       0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0, 
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0 };

	// read in each disptach logic entry from file
	while ( inDispLogicFile >> displogicid >> index >> nlevels )	{
		for ( int i = 0; i < 5; i++ )
			inDispLogicFile >> breakpts[i];
		for ( int i = 0; i < 13; i++ )	{
			for ( int j = 0; j < 5; j++ )
				inDispLogicFile >> numresc[i][j];
		}
		
		// Construct a CDisplogic object
		m_VDispLogic.push_back( CDispLogic( displogicid, index, nlevels, breakpts, numresc ));

	}
}

// Read in FWA data from file FWA.dat
void CRunScenario::ReadFWAFile( string oFilename )
{
	//if constructor opens a file
	ifstream inFWAFile( oFilename.c_str(), ios::in );

	//exit program if ifstream could not open file
	if ( !inFWAFile ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}
	// Enter elements for first FWA 
	string FWA( "Roadless_Stevensville" );
	string FMG( "Roadless" );
	int WalkInPct = 98;
	int PumpNRoll = 0;
	int Head = 100;
	int Tail = 0;
	int Parallel = 0;
	int AttDist = 0;
	bool WaterDrops = true;
	bool Excluded = false;
	double DiscSize = 0.01;
	int ESLTime = 1440;
	int ESLSize = 300;
	double AirtoGround = 1.6;
	int WalkInDelay[6] = { 0, 0, 30, 30, 15, 15 };
	int PostContUsed[6] = { 0, 0, 20, 20, 20, 20 };
	int PostContUnused[6] = { 0, 0, 0, 0, 0, 0, };
	int PostEscape[6] = { 0, 0, 20, 20, 20, 20 };
	int ReloadDelay[5] = { 0, 20, 20, 5, 25 };
	int FirstUnitDelay = 5;
	double Diurnal[24] = {0.10, 0.10, 0.10, 0.10, 0.10, 0.10,
						  0.10, 0.10, 0.10, 0.10, 0.20, 0.40, 
						  0.70, 1.00, 1.00, 1.00, 1.00, 1.00, 
						  1.00, 1.00, 0.70, 0.50, 0.30, 0.20 };
	string ROSAdjFuel[10] = { "x", "x", "x", "x", "x", "x", "x", "x", "x", "x" };
	double ROSAdjustment[10] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	int Index = 0;
	string DispLogicId( "FMU12" );
	double Latitude = 0;
	double Longitude = 0;
	string FPU = "NR_MT_002";

	// Read in each FWA entry from file
	while ( inFWAFile >> FWA >> FMG >> WalkInPct >> PumpNRoll >> Head >> Tail >> Parallel >> AttDist >> WaterDrops >> Excluded 
			>> DiscSize >> ESLTime >> ESLSize >> AirtoGround )	{
		for ( int i = 0; i < 6; i++ )
			inFWAFile >> WalkInDelay[i];
		for ( int i = 0; i < 6; i++ )
			inFWAFile >> PostContUsed[i];
		for ( int i = 0; i < 6; i++ )
			inFWAFile >> PostContUnused[i];
		for ( int i = 0; i < 6; i++ )
			inFWAFile >> PostEscape[i];
		for ( int i = 0; i < 5; i++ )
			inFWAFile >> ReloadDelay[i];
			inFWAFile >> FirstUnitDelay;
		for ( int i = 0; i < 24; i++ )
			inFWAFile >> Diurnal[i];
		for ( int i = 0; i < 10; i++ )
			inFWAFile >> ROSAdjFuel[i];
		for ( int i = 0; i < 10; i++ )
			inFWAFile >> ROSAdjustment[i];
		inFWAFile >> Index >> DispLogicId >> Latitude >> Longitude >> FPU;
		

		// Find the vector element for the dispatch logic
		if ( m_VDispLogic.size() == 0 ) {
			cerr << "No Dispatch Logic objects read into program" << endl;
			exit( 1 );
		}

		int j = 0;														//index for dispatch logic
		string VDLid = m_VDispLogic[j].GetLogicID();
		while ( VDLid != DispLogicId )	{
			j++;
			if ( j > m_VDispLogic.size()-1 )
				cout << "Dispatch logic not found!!!!!\n";
			VDLid = m_VDispLogic[j].GetLogicID();	}

		// Determine the index for the FWA
		int index = m_VFWA.size();

		// Construct CFWA object with reference in VFWA vector
		m_VFWA.push_back( CFWA( FWA, FMG, WalkInPct, PumpNRoll, Head, Tail, Parallel, AttDist, WaterDrops, Excluded, DiscSize,
			ESLTime, ESLSize, AirtoGround, WalkInDelay, PostContUsed, PostContUnused, PostEscape, ReloadDelay, FirstUnitDelay, 
		    Diurnal, ROSAdjFuel, ROSAdjustment, index, m_VDispLogic[j], FPU, Latitude, Longitude ));

		// Insert the FWAId, reference to the FWA into the map
		m_FWAMap.insert( std::pair< string, int >( FWA, index ) );
			
	}	
}

//Read in resource type information from file
void CRunScenario::ReadRescTypeFile( std::string oFilename )
{
	//if constructor opens a file
	ifstream inRescTypeFile( oFilename.c_str(), ios::in );

	//exit program if ifstream could not open file
	if ( !inRescTypeFile ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}
	// Enter elements for first FWA 
	string Type( "Default" );
	int AvgSpeed = 0;
	int DispDelay = 0;
	int RespDelay = 0 ;
	int SetupDelay = 0;

	// Read in each Resource Type entry from file
	while ( inRescTypeFile >> Type >> AvgSpeed >> DispDelay >> RespDelay >> SetupDelay )	{
		// Construct a CDisplogic object
		m_VRescType.push_back( CRescType( Type, AvgSpeed, DispDelay, RespDelay, SetupDelay ));	
	}
		
}


// Read in information about dispatch locations from file DispLoc.dat 
void CRunScenario::ReadDispLocFile( const int numFWAs, std::string oFilename )
{
	//if constructor opens a file
	ifstream inDispLocFile( oFilename.c_str(), ios::in );

	//exit program if ifstream could not open file
	if ( !inDispLocFile ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}
	// Enter elements for first Dispatch Location
	string DispLocID( "Default" );
	double Latitude;
	double Longitude;
	int CallbackDelay = 0;
	/*std::vector< double > FWAAssocs;
	for ( int i = 0; i < numFWAs; i++ )
		FWAAssocs.push_back( 0 );*/
	string FPU = "Default";

	// Read in each Dispatch Location entry from file
	while ( inDispLocFile >> DispLocID >> Latitude >> Longitude >> CallbackDelay )	{
		/*for ( int i = 0 ; i < numFWAs; i++ )
			inDispLocFile >> FWAAssocs[i];*/
		inDispLocFile >> FPU;
		// Construct a CDisploc object
		m_VDispLoc.push_back( CDispLoc( DispLocID, CallbackDelay, FPU, Latitude, Longitude ));
	}
}

// Read in the FWA / Dispatch Location assiciation information
void CRunScenario::ReadAssociationsFile( std::string oFilename )
{
	//if constructor opens a file
	ifstream inAssociationsFile( oFilename.c_str(), ios::in );

	//exit program if ifstream could not open file
	if ( !inAssociationsFile ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}
	// Enter elements for first Dispatch Location
	string FWAID( "Default" );
	string DispLocID( "Default" );
	double Distance;

	// While not end of file, read in the association information
	while( inAssociationsFile >> FWAID >> DispLocID >> Distance )	{

		// Find the FWA
		bool foundFWA = false;
		int i = 0;

		std::map< string, int >::iterator It = m_FWAMap.find( FWAID );

		if ( It != m_FWAMap.end() )	{

			foundFWA = true;
			i = ( *It ).second;

		}

		if ( !foundFWA )
			cout << "Could not find the FWA to record the Association \n";

		// Find the DispLoc
		bool foundDL = false;
		int j = 0;

		while ( !foundDL &&  j < m_VDispLoc.size() )	{

			if ( DispLocID == m_VDispLoc[j].GetDispLocID() )	
				foundDL = true;

			else
				j++;

		}

		if ( !foundDL )
			cout << "Could not find the Dispatch Location to record the Association \n";

		if ( foundFWA && foundDL )	{

			// Record association in both CDispLoc and CFWA
			m_VFWA[i].AddAssociation( DispLocID, Distance );
			m_VDispLoc[j].AddAssocFWA( &m_VFWA[i] );

		}
		
	}

}

//Read in production rate information from file
void CRunScenario::ReadProdRatesFile( std::string oFilename )
{
	//if constructor opens a file
	ifstream inProdRatesFile( oFilename.c_str(), ios::in );

	//exit program if ifstream could not open file
	if ( !inProdRatesFile ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}
	// Enter elements for first FWA 
	string RescType( "Default" );
	int SlopeClass = 0;
	int Staffing = 0;
	int FuelModel = 0 ;
	string SpCondition( "x" );
	double ProdRate = 0;

	// Read in each Resource Type entry from file
	while ( inProdRatesFile >> RescType >> SlopeClass >> Staffing >> FuelModel >> SpCondition >> ProdRate )	{
		// Construct a CDisplogic object
		m_VProdRates.push_back( CProdRates( RescType, SlopeClass, Staffing, FuelModel, SpCondition, ProdRate ));	
	}
		
}

// Read in Resource data from file Resource.dat
void CRunScenario::ReadResourceFile( std::string oFilename )
{
	// Map containing the resource id and their pointer value created to aid in assigning helitack a to helicopter
	//std::unordered_multimap< string, CResource* > ResourceMap;
	std::multimap< string, CResource* >ResourceMap;
	
	//if constructor opens a file
	ifstream inResourceFile( oFilename.c_str(), ios::in );

	//exit program if ifstream could not open file
	if ( !inResourceFile ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}
	// Enter elements for first FWA 
	string RescID( "Default" );
	string RescType( "CRW" );
	int Staffing = 1;
	string StartTime = ( "0000" );
	string EndTime = ( "2400" );
	string StartDay = ( "Monday" );
	string EndDay = ( "Sunday" );
	int StartSeason = 1;
	int EndSeason = 365;
	string DispLoc = ( "Darby_Ranger_Station" );
	int PctAvailable = 100;
	double DailyCost = 0.0;
	double HourlyCost = 0.0;
	int Volume = 0;
	int Severity = 0;
	string Version = ( "a");
	double TotalFTEs = 0.0;
	int TotalPositions = 0;
	int AnnualCost = 0;
	int VehicleCapCost = 0;

	int n = -1;

	// Read in each Resource entry from file
	while ( inResourceFile >> RescID >> RescType >> Staffing >> StartTime >> EndTime >> StartDay >> EndDay >> StartSeason >> EndSeason
		>> DispLoc >> PctAvailable >> DailyCost >> HourlyCost >> Volume >> Severity >> Version >> TotalFTEs >> TotalPositions >> AnnualCost >> 
		VehicleCapCost )	{
		
		n++;
		
		// Find the vector element for the Resource Type
		if ( m_VRescType.size() == 0 ) {
			cerr << "No Resource Type objects read into program" << endl;
			//exit( 1 );
		}

		int j = 0;														//index for dispatch logic
		string VRescTypeId = m_VRescType[j].GetRescType();
		while ( VRescTypeId != RescType )	{
			j++;
			if ( j >= static_cast< int> ( m_VRescType.size() ))	{
				cerr << "No Resource Type match" << endl;
				//exit( 1 );
			}
			VRescTypeId = m_VRescType[j].GetRescType();	}

		// Find the vector element for the Dispatch Location
		if ( m_VDispLoc.size() == 0 ) {
			cerr << "No Dispatch Location objects read into program" << endl;
			//exit( 1 );
		}

		//cout << DispLoc;
		int k = 0;														//index for dispatch logic
		bool found = false;

		while ( !found && k < m_VDispLoc.size() )	{
						
			if ( DispLoc == m_VDispLoc[k].GetDispLocID())	{
				found = true;
			}

			else
				k++;

		}

		if ( !found )
			cout << "Dispatch Location for Resource not found \n";

		else	{
					
			// Construct CResource object of the appropriate type
			if ( RescType == "DZR1" || RescType == "DZR2" || RescType == "DZR3" || RescType == "FBDZ" || RescType == "FRBT" || 
				RescType == "TP12" || RescType== "TP34" || RescType == "TP56" )
				m_VResource.push_back( new CConstProd( RescID, m_VRescType[j], Staffing, StartTime, EndTime, StartDay, EndDay, StartSeason, EndSeason, 
				 m_VDispLoc[k], PctAvailable, DailyCost, HourlyCost ));
			else if ( RescType == "CRW" )
				m_VResource.push_back( new CCrew( RescID, m_VRescType[j], Staffing, StartTime, EndTime, StartDay, EndDay, StartSeason, EndSeason, 
				 m_VDispLoc[k], PctAvailable, DailyCost, HourlyCost ));
			else if ( RescType == "EN12" || RescType == "EN34" || RescType == "EN56" || RescType == "EN70" )
				m_VResource.push_back( new CEngine( RescID, m_VRescType[j], Staffing, StartTime, EndTime, StartDay, EndDay, StartSeason, EndSeason, 
				 m_VDispLoc[k], PctAvailable, DailyCost, HourlyCost, Volume ));
			else if ( RescType == "WT" )
				m_VResource.push_back( new CWaterTender( RescID, m_VRescType[j], Staffing, StartTime, EndTime, StartDay, EndDay, StartSeason, EndSeason, 
				 m_VDispLoc[k], PctAvailable, DailyCost, HourlyCost ));
			else if ( RescType == "ATT" )
				m_VResource.push_back( new CAirtanker( RescID, m_VRescType[j], Staffing, StartTime, EndTime, StartDay, EndDay, StartSeason, EndSeason,
				m_VDispLoc[k], PctAvailable, DailyCost, HourlyCost, Volume ));
			else if ( RescType == "SEAT" || RescType == "SCP" )
				m_VResource.push_back( new CSmallAT( RescID, m_VRescType[j], Staffing, StartTime, EndTime, StartDay, EndDay, StartSeason, EndSeason,
				 m_VDispLoc[k], PctAvailable, DailyCost, HourlyCost, Volume ));
			else if ( RescType == "SMJR" )
				m_VResource.push_back( new CSmokejumper( RescID, m_VRescType[j], Staffing, StartTime, EndTime, StartDay, EndDay, StartSeason, EndSeason,
				 m_VDispLoc[k], PctAvailable, DailyCost, HourlyCost ));
			else if ( RescType == "SJAC" )
				m_VResource.push_back( new CSMJAircraft( RescID, m_VRescType[j], Staffing, StartTime, EndTime, StartDay, EndDay, StartSeason, EndSeason,
				 m_VDispLoc[k], PctAvailable, DailyCost, HourlyCost, Volume ));
			else if ( RescType == "HEL1" || RescType == "HEL2" || RescType == "HEL3" )		{
				m_VResource.push_back( new CHelicopter( RescID, m_VRescType[j], Staffing, StartTime, EndTime, StartDay, EndDay, StartSeason, EndSeason,
				m_VDispLoc[k], PctAvailable, DailyCost, HourlyCost, Volume ));

				// Place the key and pointer in the the unordered multimap ResourceMap
				int size = m_VResource.size();
				std::pair< string, CResource* > Pair( RescID, m_VResource[size-1] );
				ResourceMap.insert( Pair );

			}

			else if ( RescType == "HELI" )	{
				m_VResource.push_back( new CHelitack( RescID, m_VRescType[j], Staffing, StartTime, EndTime, StartDay, EndDay, StartSeason, EndSeason, 
				 m_VDispLoc[k], PctAvailable, DailyCost, HourlyCost ));

				// Place the key and pointer in the the unordered multimap ResourceMap
				int size = m_VResource.size();
				std::pair< string, CResource* > Pair( RescID, m_VResource[size-1] );
				ResourceMap.insert( Pair );

			}

		}	

	}

	// Assign the helitack crews to their primary helicopter
	AssignHelitack(ResourceMap);

}

// Read in Fire data from file Fire.dat
void CRunScenario::ReadFireFile( const char* Scenario )
{
	cout << "Entering Read Fires \n";
	cout << "Fire vector size " << m_VFire.size() << "\n";
	if ( m_VFire.size() > 0 )
		m_VFire.clear();
	//if constructor opens a file
	ifstream inFireFile( Scenario, ios::in );

	//exit program if ifstream could not open file
	if ( !inFireFile ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}
	// Enter elements for first FWA 
	int ScenarioNum = 1;
	int FireNum = 1;
	int JulianDay = 98;
	string DayofWeek = "Monday";
	string DiscoveryTime = "0000";
	int BI = 0;
	double ROS = 0;
	int FuelModel = 121;
	string SpecificCondition = "x";
	int SlopePct = 0;
	bool IsWalkIn = false;
	string Tactic = "HEAD";
	double Distance = 0;
	int Elevation = 1000;
	double LtoWRatio = 1.5;
	int MinNumSteps = 250;
	int MaxNumSteps = 10000;
	string Sunrise = "0600";
	string Sunset = "1000";
	bool WaterDrops = true;
	bool IsPumpNRoll = false;
	string FWAId = "Wilderness_Darby";
	double Latitude = 0.0;
	double Longitude = 0.0;
	

	// Read in each FWA entry from file
	while ( inFireFile >> ScenarioNum >> FireNum >> JulianDay >> DayofWeek >> DiscoveryTime >> BI >> ROS >> FuelModel >> SpecificCondition >> SlopePct >>
		IsWalkIn >> Tactic >> Distance >> Elevation >> LtoWRatio >> MinNumSteps >> MaxNumSteps >> Sunrise >> Sunset >> WaterDrops >> 
			IsPumpNRoll >> FWAId >> Latitude >> Longitude )	{
		
		// Find the vector element for the FWA
		if ( m_VFWA.size() == 0 ) {
			cerr << "No Dispatch Logic objects read into program" << endl;
			//exit( 1 );
		}

		// Find the FWA
		bool foundFWA = false;
		int j = 0;

		std::map< string, int >::iterator It = m_FWAMap.find( FWAId );

		if ( It != m_FWAMap.end() )	{

			foundFWA = true;
			j = ( *It ).second;

		}

		if ( !foundFWA )	{

			cout << "No valid FWA for fire entry: " << FWAId << "\n";
			break;

		}

		// Construct CFire object with reference in VFire vector
		m_VFire.push_back( CFire( ScenarioNum, FireNum, JulianDay, DayofWeek, DiscoveryTime, BI, ROS, FuelModel, SpecificCondition, 
			//SlopePct, IsWalkIn, Tactic, Distance, Elevation, LtoWRatio, MinNumSteps, MaxNumSteps, Sunrise - 6, Sunset - 6, WaterDrops, 
			SlopePct, IsWalkIn, Tactic, Distance, Elevation, LtoWRatio, MinNumSteps, MaxNumSteps, Sunrise, Sunset, WaterDrops, 
			IsPumpNRoll, m_VFWA[j], Latitude, Longitude ));
			
	}
	cout << "Number of Fires Read: " << m_VFire.size() << "\n";
	m_NumFire = m_VFire.size();
	inFireFile.close();
}

//Create a Dispatcher tree from vectors in this class
bool CRunScenario::CreateDispTree()
{
	
	// Create a Dispatcher node pointer to contain returned values
	OmffrNode< CDispatchBase* > *Node;

	// Create a vector of tree nodes to contain returned values
	std::vector< OmffrNode< CDispatchBase* >* > VNodes;

	// Create the National Node for the Dispatcher Tree
	Node = m_DispTree.AddChild( NULL, new CNatDispatcher( "National" ));
	
	// Go through the Dispatch Location list and create nodes in the tree as needed
	for ( int j = 0; j < m_VDispLoc.size(); j++ )
	{
		// Get the FPU id for the dispatch location
		string FPUId = m_VDispLoc[j].GetFPU();
		
		// If the FPU id is not "NA_US" the dispatch location national dispatcher
		if ( FPUId == "NA_US" )	{

			// Get the National Node
			Node = m_DispTree.Root();

			// Get a pointer to the National Dispatcher so can add airtanker bases to it's vector
			CNatDispatcher *NatPtr = dynamic_cast< CNatDispatcher* >( m_DispTree.Element( Node ) );

			if (NatPtr != 0)    {
                // Does the Dispatch Location already exist in the tree?
			    VNodes = m_DispTree.Children( Node );

			    // If the node does not exist create it
			    bool Found = false;
			    for ( int i = 0; i < VNodes.size(); i++ )	{
				    // Get a pointer to the Dispatch Location Dispatcher
				    CDLDispatcher *DLDispatcher = dynamic_cast < CDLDispatcher * > ( m_DispTree.Element(VNodes[i]));
				    if ( DLDispatcher != 0 )	{

					    if ( DLDispatcher->GetDispLoc() == m_VDispLoc[j] )
						    Found = true;
				    }
			    }

			    if ( !Found )	{
				    string DLID = m_VDispLoc[j].GetDispLocID();
				    Node = m_DispTree.AddChild( Node, new CDLDispatcher( DLID, m_VDispLoc[j] ) );

				    // If the dispatch location is an airtanker base need to add it's pointer to the vector of airtanker bases at the National Node
				    string DispLocID = m_VDispLoc[j].GetDispLocID();
				    int Length = DispLocID.size();
				    if ( Length > 4 )	{
					    string Key = DispLocID.substr( Length-4, 4 );

					    if ( Key == "KYLE" && NatPtr != 0 )	{
						    CDLDispatcher *DLPtr = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( Node ) );
						    NatPtr->AddATDispatcher( DLPtr );
						
					    }

				    }
				
			    }
            }
		}

		else	{
			// Parse the FPU Id to determine the GACC Id 
			string GACCId = FPUId.substr(0,2);

			// Does the GACC node already exist in the tree
			Node = m_DispTree.Root();										// Get pointer to the root (National) Node
			VNodes = m_DispTree.Children( Node );							// Get a vector of the current GACC Nodes in the tree
			
			// If the node exists move to it, if not create it			
			bool Found = false;

			int NodeI = 0;
			for ( int i = 0; i < VNodes.size(); i++ )	{
				if ( m_DispTree.Element(VNodes[i])->GetDispatcherID() == GACCId )
				{
					Found = true;
					NodeI = i;
					break;
				}
					
			}

			if ( Found )
				Node = VNodes[NodeI];
			else
				Node = m_DispTree.AddChild( Node, new CGACCDispatcher( GACCId ) );

			// Is the Dispatch Location located at the GACC?
			if ( GACCId == FPUId )	{

				// Does the Dispatch Location already exist in the tree?
				VNodes = m_DispTree.Children( Node );

				// If the node does not exist create it
				Found = false;
				for ( int i = 0; i < VNodes.size(); i++ )	{
					// Get a pointer to the Dispatch Location Dispatcher
					CDLDispatcher *DLDispatcher = dynamic_cast < CDLDispatcher * > ( m_DispTree.Element(VNodes[i]));
					
                    if ( DLDispatcher != 0 )	{

						if ( DLDispatcher->GetDispLoc() == m_VDispLoc[j] )
						{
							Found = true;
							NodeI = i;
							break;
						}
					}
				}

				if ( !Found )	{
					string DLID = m_VDispLoc[j].GetDispLocID();
					Node = m_DispTree.AddChild( Node, new CDLDispatcher( DLID, m_VDispLoc[j] ) );
				}

			}

			else	{

				// Does the FPU node already exist in the tree?
				VNodes = m_DispTree.Children( Node );								// Get a vector of the current FPU Nodes in the tree

				// If the node exists move to it, if not create it
				Found = false;
				for ( int i = 0; i < VNodes.size(); i++ )	{

					if ( m_DispTree.Element(VNodes[i])->GetDispatcherID() == FPUId )
					{
						Found = true;
						NodeI = i;
						break;
					}
				}

				if ( Found )
					Node = VNodes[NodeI];
				else
					Node = m_DispTree.AddChild( Node, new CLocalDispatcher( FPUId ) );

				// Does the Dispatch Location already exist in the tree?
				VNodes = m_DispTree.Children( Node );

				// If the node does not exist create it
				Found = false;
				for ( int i = 0; i < VNodes.size(); i++ )	{
					// Get a pointer to the Dispatch Location Dispatcher
					CDLDispatcher *DLDispatcher = dynamic_cast < CDLDispatcher * > ( m_DispTree.Element(VNodes[i]));
					if ( DLDispatcher != 0 )	{

						if ( DLDispatcher->GetDispLoc() == m_VDispLoc[j] )
						{
							Found = true;
							NodeI = i;
							break;
						}
					}
				}

				if ( !Found )	{
					string DLID = m_VDispLoc[j].GetDispLocID();
					Node = m_DispTree.AddChild( Node, new CDLDispatcher( DLID, m_VDispLoc[j] ) );
				}
			}
		}
	}

	// Get the indexed map of the dispatch locations so can go directly to the desired node 
	// the index string is the dispatch location id
	m_DispMap = BuildDispatchIndex( &m_DispTree );
	m_InMap = BuildInternalIndex( &m_DispTree );

	// Call Function to add alternate dispatch locations for helicopters to deploy helitack
	AltHelicDLs( m_DispMap );

	return true;
}

// Put pointers to the resources in the resource vector into the resource maps for the appropriate dispatch location dispatcher
bool CRunScenario::ResourcesToDispatchers()
{
	// Get a vector of all the dispatch locations (external nodes) in the tree
	std::vector< OmffrNode< CDispatchBase* >* > VDispatchLocations = m_DispTree.GetExternalNodes( m_DispTree.Root(), -1 );
	
	// Iterate through the resources in the resource vector m_VResource
	for ( int i = 0; i < m_VResource.size(); i++ )
	{
		// If the resource is a large airtanker - resource type ATT add it to the list of airtankers at the national level
		if ( m_VResource[i]->GetRescType().GetRescType() == "ATT" )	{
			OmffrNode< CDispatchBase* > *Node = m_DispTree.Root();
			CNatDispatcher* NatDisp = dynamic_cast< CNatDispatcher* >( m_DispTree.Element( Node ) );
			if ( NatDisp != 0 )
				NatDisp->AddAirtankerToVector( m_VResource[i] );
		}
		
		// Get the dispatch location for the resource
		CDispLoc DispLoc = m_VResource[i]->GetDispLoc();

		// Iterate through the vector for dispatch location dispatchers to find the dispatch location dispatcher the resource belongs in
		bool found = false;
		int j = 0;
		while ( !found && j < VDispatchLocations.size() )	{
			CDLDispatcher *DLDispLoc = dynamic_cast< CDLDispatcher * >( m_DispTree.Element( VDispatchLocations[j] ) ); 
			if ( DLDispLoc != 0 )	{
				if( DispLoc == DLDispLoc->GetDispLoc() )	{
					found = true;

					// Is this Dispatch Location Dispatcher at the GACC Level?
					OmffrNode< CDispatchBase* > *Node;
					Node = m_DispTree.Parent( VDispatchLocations[j] );
					CGACCDispatcher *GACCDisp = dynamic_cast< CGACCDispatcher* >( m_DispTree.Element( Node ) );
					if ( GACCDisp != 0 )	{

						// Check if resource is a Type II IA Crew 
						if ( m_VResource[i]->GetRescType().GetRescType() == "CRW" && m_VResource[i]->GetStaffing() == 20 )	{

							// create the CTypeTwoIACrew instance and put the 4-5 person IA Crews at the dispatch location
							GACCDisp->AddNewTypeTwoCrew( m_VResource[i], DLDispLoc  );

						}

						else	
							DLDispLoc->InsertInRescMap( m_VResource[i] );
					}

					else
						DLDispLoc->InsertInRescMap( m_VResource[i] );

				}
			}

			else
				cout << "Have an external node in the dispatcher tree that is not a dispatch location dispatcher\n";

			j++;
		}

		if ( !found )
			cout << "The dispatch location dispatcher for the resource is not found in the tree\n";
	}

	// Determine the base resource level for each node by resource type
	// Get a list of the dispatch location dispatchers all external nodes
	vector< OmffrNode< CDispatchBase* >* > DLNodes = m_DispTree.GetExternalNodes( m_DispTree.Root(), -1 );

	// Iterate through the dispatch location dispatchers and update the base resources level 
	for ( int i = 0; i < DLNodes.size(); i++ )	{
		vector< CDispatchBase* > BlankVector;
		m_DispTree.Element( DLNodes[i] )->DetermineBaseRescLevel( BlankVector );
	}

	// Get a list of the internal node dispatchers and update their base resources level
	vector< OmffrNode< CDispatchBase* >* > VNodes = GetInternalNodes( m_DispTree.Root() );
	VNodes = OrderNodesRootLast( VNodes );

	// Iterate through the internal nodes and update the base resources level
	for ( int i = 0; i < VNodes.size(); i++ )	{
		// Get the external nodes to the node
		vector< OmffrNode< CDispatchBase* >* > ExNodes = m_DispTree.GetExternalNodes( VNodes[i], -1 );
		vector < CDispatchBase* > ExDispatchers;
		for ( int j = 0; j < ExNodes.size(); j++ )
			ExDispatchers.push_back( m_DispTree.Element( ExNodes[j] ) );

		m_DispTree.Element( VNodes[i] )->DetermineBaseRescLevel( ExDispatchers );
	}

	// Print out the resources that are in each node of the tree along with their resource type
	for ( int j = 0; j < VDispatchLocations.size(); j++ )	{
		//cout << "Dispatch Location: " << m_DispTree.Element( VDispatchLocations[j] )->GetDispatcherID() << "\n";

		// Get the resource map for the dispatch location dispatcher 
		CDLDispatcher *DLDispLoc = dynamic_cast< CDLDispatcher * >( m_DispTree.Element( VDispatchLocations[j] ) ); 
			if ( DLDispLoc != 0 )	{
				RescMap DLRescMap = DLDispLoc->GetRescMap();

				// Iterate through it printing out the resource id and resource type
				for ( RescMap::iterator Iter= DLRescMap.begin(); Iter != DLRescMap.end(); Iter++ )	{
                                    (void)j;
					//cout << "	Resource ID: " <<( *Iter ).second->GetRescID() << " Type: " << ( *Iter ).first << "\n";
				}

			}

			else
				cout << "Have an extrenal node in the dispatcher tree that is not a dispatch location disaptcher (print) \n";

	}

	
	return true;
}

// Find the name of the closest airtanker base for each FWA in the analysis
bool CRunScenario::FWAsFindClosestAirtankerBases()
{
	// Get the Airtanker Base Dispatch Location Dispatchers
	vector< CDLDispatcher* > AirTankerBases;
	vector< OmffrNode< CDispatchBase* >* > VNodes = m_DispTree.Children( m_DispTree.Root() );
	vector< OmffrNode< CDispatchBase* >* >::iterator Iter;

	// Iterate throught the nodes and find the airtanker bases
	for ( Iter = VNodes.begin(); Iter != VNodes.end(); Iter++ )	{
		// Get the Dispatch Location pointer, it it's not a dispatch location it's not an airtanker base
		CDLDispatcher* DispLocDispatcher = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( *Iter ) );

		if ( DispLocDispatcher != 0 )	{

			// For now need to use the name to determine if it is an airtanker base.  WILL NEED TO CHANGE
			CDispLoc DispLoc = DispLocDispatcher->GetDispLoc();
			string DispLocID = DispLoc.GetDispLocID();

			int length = DispLocID.length();

			string key = "no";
			if ( length > 4 )
				key = DispLocID.substr( length-4, 4 );
			

			// If this is an airtanker base add to the airtanker base vector
			if ( key == "KYLE" )
				AirTankerBases.push_back( DispLocDispatcher );

		}		

	}		// End iterate through children of national node
			
	// Iterate through each FWA and find the closest airtanker base
	for ( int i = 0; i < m_VFWA.size(); i++ )	{

		int ShortestDist = 1000000;
		string NameShortest = "";
		
		// Iterate throught all the Airtanker bases to find the closest airtanker base
		for ( int a = 0; a < AirTankerBases.size(); a++ )	{

			
			int Distance = m_VFWA[i].GetDistance( AirTankerBases[a]->GetDispLoc().GetDispLocID() );

			if ( Distance < ShortestDist && Distance > 0 )	{

				// Save the information for the airtanker base
				NameShortest = AirTankerBases[a]->GetDispLoc().GetDispLocID();
				ShortestDist = Distance;

			}

		}	// End Search through Airtanker bases

		if ( ShortestDist < 1000000 )	
			m_VFWA[i].SetClosestATBase( NameShortest );

		else
                    (void)ShortestDist;
			//cout << "Could not find a closest airtanker base for the FWA. \n";

	}	// End iterate through the FWAs

	return true;
}


// Read in all the data files needed of the run
void CRunScenario::ReadFiles( std::string oPath )	
{
	ReadDispLogicFile( oPath + "DispLogic.dat" );
	m_NumDispLogic = m_VDispLogic.size();
	cout << "Number of Dispatch Logics read: " << m_NumDispLogic << "\n";
	ReadFWAFile( oPath + "FWA.dat" );
	m_NumFWA = m_VFWA.size();
	cout << "Number of FWAs read: " << m_NumFWA << "\n";
	ReadRescTypeFile(oPath + "RescType.dat");
	m_NumRescType = m_VRescType.size();
	cout << "Number of Resource Types read: " << m_NumRescType << "\n";
	ReadDispLocFile( m_NumFWA, oPath + "DispLoc.dat" );
	m_NumDispLoc = m_VDispLoc.size();
	cout << "Number of Dispatch Locations read: " << m_NumDispLoc << "\n";
	ReadAssociationsFile( oPath + "Associations.dat" );
	cout << "Read Associations \n";
	ReadProdRatesFile( oPath + "ProdRates.dat");
	m_NumProdRates = m_VProdRates.size();
	cout << "Number of Production rates read: " << m_NumProdRates << "\n";
	ReadResourceFile( oPath + "Resource.dat" );
	m_NumResource = m_VResource.size();
	cout << "Number of Resources read: " << m_NumResource << "\n";

	// Create a Dispatcher tree from the vector data
	bool Succeed = CreateDispTree();
	if (Succeed)
		cout << "Tree successfully created\n";

	// Create the resource maps for the dispatch location dispatchers
	Succeed = ResourcesToDispatchers();
	if (Succeed)
		cout << "Resources to Map successfully\n";

	//m_DispTree.Print( m_DispTree.Root());

	// Find the closest airtanker base for each dispatch location
	FWAsFindClosestAirtankerBases();
	CreateRescTypeVectors();
	ReadFireSeason();

}

// Determine which resources to deploy to a fire and calculate their start time, production rate, duration vectors
// Debugging = 0 no debugging, Debugging = 1 debugging
bool CRunScenario::DeployResources( int Debugging, int f, int scenario )
{
	// Presets for options
	// Indicate the method used to dispatch resources 0 = original method, 1 uses dispatcher tree
	int DispMethod = 1;
	
	// Set values for the new options
	bool CrewDrawDown = false;										// Apply Type II IA Crew draw down to analysis?
	int CDDEscapes = 5;												// Number of escape fires to trigger a drawdown
	int CDDDuration = 2880;											// Duration of the drawdown, added to the next available time
	int CDDLimit = 20160;											// Maximum level of drawdown from midnight of past fire day that can be applied to any Type II IA Crew
	
	bool MoveAT = false;											// Move airtankers after working on a fire to the nearest base 
	bool PrepositionResources = true;								// Preposition large airtankers at the beginning of each day
	
	bool HelicoptersStayOnFire = false;								// After the helicopters deliver helitack they remain tied to the fire
	bool HelicopterDrops = false;									// Simulate drops for helicopters delivering helitack
	double HelitackProdRateMultiplier = 1.2;							// Used to increase helitack production rate to account for helicotper drops. 1 = no increase	
	double WaterDropMultiplier = 0.5;								// Water drops produce half the line retardant drops do	

	// Print the fire information to file if debuggins is set on
	if ( Debugging > 0 )
		PrintFireInfo( f );

	// Create a variable to indicate if the dispatch logic is filled
	bool DispLogicFilled = true;
	bool InSeason = false;

	// Create a list of the resources that are deployed to the fire
	std::list< CResource* > LDeployedResources;
	
	// Get the information for the fire
	CFWA& FWA = m_VFire[f].GetFWA();

	// If using the resource vector to dispatch resources rather than the dispatch tree (original method)
	if ( DispMethod == 0 )
		LDeployedResources = DeployFromRescVector( f, Debugging );

	else	{	// use the dispatcher tree to dispatch resources
		
		// Determine if this fire is in a new scenario
		if ( f == 0 )	{
		
			//Iterate through all external dispatch nodes in the tree
			//The Dispatcher function will interate through the resources and reset the values
			vector< OmffrNode< CDispatchBase* >* > VNodes = m_DispTree.Preorder( m_DispTree.Root() );
			for ( int i = 0; i < VNodes.size(); i++ )
				m_DispTree.Element(VNodes[i])->ResetNewScenario( scenario-1 );

			string RescType = "Airtanker";
                        double MoveLevel =	1.5 - ATTpp.level;
                        bool OutSeason =  ATTpp.outOfSeason;
                        if ( MoveLevel > 0 )
                            PrepositionResourceIA( RescType, MoveLevel, OutSeason, m_VFire[f].GetJulianDay() );
                }

		else	{		// Determine if the fire is on a new day and reset some of the values accordingly
			if ( m_VFire[f].GetJulianDay() > m_VFire[f-1].GetJulianDay() )	{

				// Get the Julian date for the last day that had fires to store the previous use level information
				int JulianPast = m_VFire[f-1].GetJulianDay();

				// Reset the Daily minimum resource levels for each dispatcher
				vector< OmffrNode< CDispatchBase* >* > VNodes = m_DispTree.Preorder( m_DispTree.Root() );
				for ( int i = 0; i < VNodes.size(); i++ )
					m_DispTree.Element(VNodes[i])->ResetNewDay( JulianPast );

				// Calculate the resource usage values for the last fire day
				// Get a list of the dispatch location dispatchers all external nodes
				vector< OmffrNode< CDispatchBase* >* > DLNodes = m_DispTree.GetExternalNodes( m_DispTree.Root(), -1 );

				// Create Null vector of tree nodes to send to dispatch location dispatcher's function
				vector< CDispatchBase* > NullNodes;

				for (int i = 0; i < DLNodes.size(); i++)	{
					// Calculate the daily usage level for the dispatch location based on the levels at 12:01
					m_DispTree.Element(DLNodes[i])->CalcDailyUsageLevel(JulianPast, NullNodes);
				}

				// Get a list of the internal node dispatchers and update their current resources available level
				VNodes = GetInternalNodes( m_DispTree.Root() );
				VNodes = OrderNodesRootLast( VNodes );

				// Iterate through the internal nodes and update the current resources available level
				for ( int i = 0; i < VNodes.size(); i++ )	{
					// Get the external nodes to the node
					vector< OmffrNode< CDispatchBase* >* > ExNodes = m_DispTree.GetExternalNodes( VNodes[i], -1 );
					vector < CDispatchBase* > ExDispatchers;
					for ( int j = 0; j < ExNodes.size(); j++ )
						ExDispatchers.push_back( m_DispTree.Element( ExNodes[j] ) );

					// Determine the resource usage, the number of deploys divided by the number of resources for the dispatch locations below the node
					m_DispTree.Element(VNodes[i])->CalcDailyUsageLevel(JulianPast, ExDispatchers);
				}

				// If the fire day is not the day after the previous fire day, Calculate the resource availability for the days in between
				JulianPast++;

				while ( m_VFire[f].GetJulianDay() > JulianPast )	{

					// Use time 0001 to determine highest level of unavailable resources
					int timeYear = ( JulianPast - 1 ) * 1440 + 1;

					// Iterate through the dispatch location dispatchers and update the current resources available level without the resources in the deployed resources list
					for ( int i = 0; i < DLNodes.size(); i++ )	{
						// Calcualte the Current Resource Level at the dispatch locations at 12:01 for the day
						m_DispTree.Element( DLNodes[i] )->DetermineCurRescLevel( timeYear, NullNodes );

						// Calculate the daily usage level for the dispatch location based on the levels at 12:01
						m_DispTree.Element(DLNodes[i])->CalcDailyUsageLevel(JulianPast, NullNodes);
					}
						
					// Iterate through the internal nodes and update the current resources available level
					for ( int i = 0; i < VNodes.size(); i++ )	{
						// Get the external nodes to the node
						vector< OmffrNode< CDispatchBase* >* > ExNodes = m_DispTree.GetExternalNodes( VNodes[i], -1 );
						vector < CDispatchBase* > ExDispatchers;
						for ( int j = 0; j < ExNodes.size(); j++ )
							ExDispatchers.push_back( m_DispTree.Element( ExNodes[j] ) );

						// Determine the number of resources, by type, where the level is the sum of the resources at the dispatch locations below the node
						m_DispTree.Element( VNodes[i] )->DetermineCurRescLevel( timeYear, ExDispatchers );

						// Determine the resource usage, the number of deploys divided by the number of resources for the dispatch locations below the node
						m_DispTree.Element(VNodes[i])->CalcDailyUsageLevel(JulianPast, ExDispatchers);
					}

					// Reset the Daily minimum resource levels for each dispatcher
					VNodes = m_DispTree.Preorder( m_DispTree.Root() );
					for ( int i = 0; i < VNodes.size(); i++ )
						m_DispTree.Element(VNodes[i])->ResetNewDay( JulianPast );

					JulianPast++;
				}

				// Return borrowed resources if their time has expired
				ReturnResources( m_VFire[f].GetJulianDay() );

				// Draw Down Resources for large fires
				list< int > LOrderedEscapeFires = OrderEscapeFires();

				if ( LOrderedEscapeFires.size() > 0 )	{

					DailyDrawDown( LOrderedEscapeFires );

					// Reset the index to reflect the fires that have been dealt with
					m_EscapeIdx = m_VEscapes.size() -1;

				}
				
				// Draw down the Type II IA Crews at each GACC based on the escape fire level for the last fire day
				if ( CrewDrawDown )	
					int NumCrews = TypeIICrewDrawDown( f, CDDEscapes, CDDDuration, CDDLimit );

				// Preposition the Resources
				if ( PrepositionResources )	{

					int Julian = m_VFire[f].GetJulianDay();
					PrepositionAllResc( ATTpp, RCRWpp, RHelpp, FSCRWpp, DOICRWpp, FSENGpp, DOIENGpp, FSHELIpp, DOIHELIpp, Julian );

				}
			}
		}

		// Reset all the lists for deploying resources in the CDLDispatcher class 
		CDLDispatcher::EmptyHelicopters();							// empty helicopter list
		CDLDispatcher::EmptyHelitack();								// empty helitack list
		CDLDispatcher::EmptySmokeJumperAircraft();					// empty smokejumper aircraft list
		CDLDispatcher::EmptySmokeJumpers();							// empty smokejumper list
		CDLDispatcher::EmptyGroundResources();						// empty ground resources list
		CDLDispatcher::EmptyAerialResources();						// empty aerial resources list
		CDLDispatcher::EmptyLateArrivals();							// empty late arrivals list

		// Get the information for the fire
		int j = 0;
		while ( m_VFWA[j].GetFWAID() != FWA.GetFWAID() && j < static_cast< int >( m_VFWA.size() ) )	
			j++;
		if ( j > static_cast< int >( m_VFWA.size() ))	{
			cout << "Fire's specified FWA not in analysis :";
			return false;	}

		CDispLogic& DispLogic = m_VFWA[j].GetDispLogic();
	
		string Value = DispLogic.GetIndex();
		int LogicValue = 0;
		if ( Value == "BI" )
			LogicValue = m_VFire[f].GetBI();
		if ( Value == "ROS" )
			LogicValue = m_VFire[f].GetROS();
		
		// Determine the dispatch level to use
		int level = 0;
			for ( int m = 0; m < DispLogic.GetNumLevels()-1; m++ )	{
			if (  LogicValue > DispLogic.GetBreakPoint(m) )
				level++;	}
	
		vector< int > RemainingDispLogic(15);
		for ( int t = 0; t < 13; t++ )
			RemainingDispLogic[t] = DispLogic.GetRescNum( t, level );
		
		// Currently don't have a value for the 20 person crew or helicopters for dropping water
		RemainingDispLogic[13] = 0;
		RemainingDispLogic[14] = 0;

		// Is the dispatch logic filled?
		bool filled = false;
		list< CResource* > DropHelicopters;						// List of helicopters that can be used for dropping water
		
		// Iterate through the different levels of the tree in reverse order to fill dispatch logic
		for ( int Level = 3; Level > 0; Level-- )	{

			// If dispatch logic is not filled
			if ( !filled )	{
		
				// Using the FWA get a list of the associated dispatch locations dispatcher nodes ordered by distance from the fire (FWA TTP)
				// Get an ordered list of the associated dispatch location's names for the lowest level
				std::list< string > FWAsDispLocList = GetOrderedDispLocsbyLevel( FWA, Level );

				// Do until the dispatch logic is filled or run out of associated dispatch locations
				std::list< string >::iterator AssIt = FWAsDispLocList.begin();

				// Create a Dispatcher node pointer to contain returned values
				OmffrNode< CDispatchBase* > *Node ;

				// Create an iterator to the dispatch map to contain returned values
				std::map<std::string, OmffrNode<CDispatchBase*>*>::iterator DispMapIt;

				// If the dispatchers are at level 3 determine if there any resources in season
				if ( Level == 3 )	{
				
					// Iterate through the Dispatch Location nodes and determine if there are any in season resources at level 3
					for ( AssIt = FWAsDispLocList.begin(); AssIt != FWAsDispLocList.end(); AssIt++ )	{ 
						// find the pointer to the dispatch location node using the dispatch location map
						DispMapIt = m_DispMap.find( *AssIt );
			
						// if the dispatch location cannot be found in the map the DispMapIt = FWAsDispLocList.end()
						if( DispMapIt != m_DispMap.end() )	{

							Node =  DispMapIt->second ;
	
							CDLDispatcher *DLDispLoc = dynamic_cast< CDLDispatcher * >( m_DispTree.Element( Node ) ); 
							if ( DLDispLoc != 0 )	{

								// Are there any resources in season at this dispatch location and the dispatch location is at level 3
								int Julian = m_VFire[f].GetJulianDay();
								bool thisInSeason = DLDispLoc->HasRescInSeason( Julian ) ;

								// Change InSeason if any level three dispatch locations have resources in season
								if ( thisInSeason )
									InSeason = true;
							}
						}
					}
				}

				AssIt = FWAsDispLocList.begin();
				while ( !filled && AssIt != FWAsDispLocList.end() )	{
					// find the pointer to the dispatch location node using the dispatch location map
					DispMapIt = m_DispMap.find( *AssIt );
					list< CResource* > ThisDeployed;

					// if the dispatch location cannot be found in the map the DispMapIt = FWAsDispLocList.end()
					if( DispMapIt != m_DispMap.end() )	{

						Node =  DispMapIt->second ;

						CDLDispatcher *DLDispLoc = dynamic_cast< CDLDispatcher * >( m_DispTree.Element( Node ) ); 
						if ( DLDispLoc != 0 )	{
							// Deploy resources from this dispatch location
							ThisDeployed = DLDispLoc->DeployResources( RemainingDispLogic, m_VFire[f], DropHelicopters );

							
						}
				
						// Add the recently deployed resources to the deployed resource list
						LDeployedResources.splice( LDeployedResources.end(), ThisDeployed );
					}
										
					// Is the Dispatch Logic filled
					filled = true;

					for ( int i = 0; i < 15; i++ )	{
						if ( RemainingDispLogic[i] > 0 )
							filled = false;
					}

					AssIt++;

				}

			}

		}

		// Check if there are additional helicopters in the remaining dispatch logic to deploy helicopters for dropping water
		if ( RemainingDispLogic[6] > 0 )	{

			// Find the number of helicopters to be deployed
			int NumHelicopters = RemainingDispLogic[6];

			if ( NumHelicopters > DropHelicopters.size() )
				NumHelicopters = DropHelicopters.size();

			// Deploy the helicopters
				list< CResource* >::iterator ItDHel = DropHelicopters.begin();

				for ( int i = 0; i < NumHelicopters; i++ )	{

				if ( ItDHel == DropHelicopters.end() )
					cout << "The size of DropHelicopters is too small\n";

				LDeployedResources.push_back( *ItDHel );
					ItDHel++;

			}

		}
		
		if ( !filled )
			DispLogicFilled = false;

	}

	std::list< CResource* >::iterator Iterator;

	// Create a vector of resources to be removed from the analysis
	std::list< CResource* > Remove;
	
	// Determine if there are any engines deployed to the fire, if not do not deploy water tenders
	bool EngsDeployed = false;

	for ( Iterator = LDeployedResources.begin(); Iterator!= LDeployedResources.end(); Iterator++ )	{
		// If the resource is an engine set the engines deployed value to true
		if ( (*Iterator)->GetDispatchType() == 3 )
			EngsDeployed = true;
	}
	
	// Determine the arrival time for the water tender(s) if any are deployed to the fire.  If not arrival time = 10000
	int WTArrival = 10000;	

	for ( Iterator = LDeployedResources.begin(); Iterator!= LDeployedResources.end(); Iterator++ )	{
		// If the resource is a water tender update the first arrival time for water tenders
		if ( (*Iterator)->GetDispatchType() == 12 )
		{
			if ( !EngsDeployed )
				Remove.push_back( *Iterator );
			else	{
				// Update the water tender first arrival time
				if ( (*Iterator)->GetInitArrivalTime() < WTArrival )
					WTArrival = (*Iterator)->GetInitArrivalTime() ;
			}
		}
	}

	// Remove any Water Tenders from the remove list
	if ( Remove.size() > 0 )
		RemoveDeployedResc( LDeployedResources, Remove );
	
	int firstarrival = 10000;
	
	for ( Iterator = LDeployedResources.begin(); Iterator != LDeployedResources.end(); Iterator++ )	{

		// Do not use airtankers or helicopters to determine first arrival time
		//cout << "Resource: " << *Iterator << " Dispatch Type: " << m_VResource[ *Iterator ]->GetDispatchType() << "\n";
		if ( ( *Iterator )->GetDispatchType() != 0 && ( *Iterator )->GetDispatchType() != 6 && 
			( *Iterator )->GetDispatchType() != 8 && ( *Iterator )->GetDispatchType() != 9 )	{
			//cout << "Resource Number: " << *Iterator <<  " Initial Arrival Time: " << m_VResource[ *Iterator ]->GetInitArrivalTime() << "\n";
			if ( firstarrival > ( *Iterator )->GetInitArrivalTime() )
				firstarrival = ( *Iterator )->GetInitArrivalTime();
		}
	}

	// No resources can arrive any sooner than first arrival
	firstarrival = firstarrival + FWA.GetFirstUnitDelay();

	for ( Iterator = LDeployedResources.begin(); Iterator != LDeployedResources.end(); Iterator++ )	{
		int rescarrival = ( *Iterator )->GetInitArrivalTime();
		if ( rescarrival < firstarrival )
			( *Iterator )->SetInitArrivalTime( firstarrival );
		int duration = ( *Iterator )->CalcRemainingWorkshift( m_VFire[ f ]);
		if ( duration <= 0 )	{
			// Make a vector, Remove, of the values that need to be removed from the list
			Remove.push_back( *Iterator );
			
		}
	}

	// Remove any resources that have no duration once the first arrival time is added
	if ( Remove.size() > 0 )
		RemoveDeployedResc( LDeployedResources, Remove );

	// Calculate the first arrival time for the final set of deployed resoruces
	firstarrival = 10000;
	
	for ( Iterator = LDeployedResources.begin(); Iterator != LDeployedResources.end(); Iterator++ )	{

		// Do not use airtankers or helicopters to determine first arrival time
		//cout << "Resource: " << *Iterator << " Dispatch Type: " << m_VResource[ *Iterator ]->GetDispatchType() << "\n";
		if ( ( *Iterator )->GetDispatchType() != 0 && ( *Iterator )->GetDispatchType() != 6 && 
			( *Iterator )->GetDispatchType() != 8 && ( *Iterator )->GetDispatchType() != 9 )	{
			//cout << "Resource Number: " << *Iterator <<  " Initial Arrival Time: " << m_VResource[ *Iterator ]->GetInitArrivalTime() << "\n";
			if ( firstarrival > ( *Iterator )->GetInitArrivalTime() )
				firstarrival = ( *Iterator )->GetInitArrivalTime();
				
		}
	}

	// No resources can arrive any sooner than first arrival
	firstarrival = firstarrival + FWA.GetFirstUnitDelay();

	// If aviation resource do not deploy if drop can't be made before sunset
	int sunset = m_VFire[ f ].SunsetMSM();
	int firestarttime = m_VFire[ f ].FireStartTime();
	
	for ( Iterator = LDeployedResources.begin(); Iterator != LDeployedResources.end(); Iterator++ )	{
		//cout << *Iterator << "\n";
		// Determine if deployed resource is Airtanker, seat, or scooper
		if ( ( *Iterator )->GetDispatchType() == 0 || ( *Iterator )->GetDispatchType() == 8 )	{
			// If sunset is before the resource can arrive and make a drop remove the resource from the deploy list
			int rescarrival = ( *Iterator )->GetInitArrivalTime();
			if ( rescarrival < firstarrival )
				rescarrival = firstarrival;
			if ( sunset < firestarttime + rescarrival + 1 )	{
				// Make a vector, VRemove, of the values that need to be removed from the list
				Remove.push_back( *Iterator );
			}
		}

		// Determine if deployed resource is a helicopter and not used as the carrier for helitack/rappel
		if ( ( *Iterator )->GetDispatchType() == 6 )	{
			// Get a CHelicopter pointer to the helicopter
			CHelicopter *Helicopter = dynamic_cast< CHelicopter* >( ( *Iterator ) );
			if ( Helicopter != 0 ) {
				bool iscarrier = Helicopter->GetIsCarrier();
				// If the helicopter is not a carrier
				if ( !iscarrier )	{
					// If sunset is before the resource can arrive and make a drop remove the resource from the deploy list
					int rescarrival = ( *Iterator )->GetInitArrivalTime() + ( *Iterator )->GetRescType().GetSetupDelay() + FWA.GetReloadDelay(3);
					if ( rescarrival < firstarrival )
					rescarrival = firstarrival;
					if ( sunset < firestarttime + rescarrival + 1 )	{
						// Make a vector, VRemove, of the values that need to be removed from the list
						Remove.push_back( *Iterator );
					}
				}
			}
			else	{
				// bad pointer to carrier
				cout << "Bad pointer to carrier to initialize carrier's values \n";
				return false;
			}
		}
		
	}

	// Remove the values from the LDeployedResources in Remove
	if ( Remove.size() > 0 )
		RemoveDeployedResc( LDeployedResources, Remove );

	// !!!! The Deployed Resources list is complete at this point
	// Calculate the current resources available level for all the nodes in the dispatcher tree
	// Include the resources in the dispatched resources list as unavailable

	// Get a list of the dispatch location dispatchers all external nodes
	vector< OmffrNode< CDispatchBase* >* > DLNodes = m_DispTree.GetExternalNodes( m_DispTree.Root(), -1 );

	// Iterate through the dispatch location dispatchers and update the current resources available level without the resources in the deployed resources list
	for ( int i = 0; i < DLNodes.size(); i++ )	{
		CDLDispatcher* DLDispatcher = dynamic_cast< CDLDispatcher* >(m_DispTree.Element( DLNodes[i] ));

		if ( DLDispatcher != 0 )	{
			DLDispatcher->DetermineCurRescLevelatDeploy( m_VFire[f], LDeployedResources );
		}
	}

	// Get a list of the internal node dispatchers and update their current resources available level
	vector< OmffrNode< CDispatchBase* >* > VNodes = GetInternalNodes( m_DispTree.Root() );
	VNodes = OrderNodesRootLast( VNodes );

	// Determine the discovery time for the fire in minutes since beginning of the year
	int timeYear = ( m_VFire[f].GetJulianDay() - 1 ) * 1440 + m_VFire[f].FireStartTime();

	// Iterate through the internal nodes and update the current resources available level
	for ( int i = 0; i < VNodes.size(); i++ )	{
		// Get the external nodes to the node
		vector< OmffrNode< CDispatchBase* >* > ExNodes = m_DispTree.GetExternalNodes( VNodes[i], -1 );
		vector < CDispatchBase* > ExDispatchers;
		for ( int j = 0; j < ExNodes.size(); j++ )
			ExDispatchers.push_back( m_DispTree.Element( ExNodes[j] ) );

		m_DispTree.Element( VNodes[i] )->DetermineCurRescLevel( timeYear, ExDispatchers );
	}

	
	// calculate the size of the fire when the first resource arrives - for now save and output this in CResults as FinalFireSweep
	double size = m_VFire[f].CalcFBFireSize( firstarrival );
	//cout << "Size at first arrival: " << size << "\n";

	// Determine the start time, prod rate, duration values for the deployed resources
	std::list< CContainValues > LContainValues;

	// If there are no ground resources firstarrival will = 10000 and should not try to calculate contain values
	if ( firstarrival < 1140 )	{
		for ( Iterator = LDeployedResources.begin(); Iterator != LDeployedResources.end(); Iterator++ )	{

			// If the resource is a helicopter determine if it is a carrier then use HelicopterDrops to determine if it makes drops
			if ( ( *Iterator )->GetDispatchType() == 6 )	{

				CHelicopter* Helic = dynamic_cast< CHelicopter* >( *Iterator );

				if ( Helic != 0 )	{

					if ( Helic->GetIsCarrier() == false || HelicopterDrops == true )
						( *Iterator )->CreateContainValues( m_VFire[ f ], firstarrival, m_VProdRates, LContainValues, WaterDropMultiplier );

				}

			}

			// If the resource is a helitack crew and HelitackProdRateMultiplier != 1 then adjust the production rates
			else if ( ( *Iterator )->GetDispatchType() == 7 )
				( *Iterator )->CreateContainValues( m_VFire[ f ], firstarrival, m_VProdRates, LContainValues, HelitackProdRateMultiplier );
						
			else
				( *Iterator )->CreateContainValues( m_VFire[ f ], firstarrival, m_VProdRates, LContainValues, 1.0 );

		}
	}

	// Print out the CContainValues in LContainValues
	//std::list< CContainValues * >::iterator It;

	//for ( It = LContainValues.begin(); It != LContainValues.end(); It++ )	
		//cout << "Start Time: " << (*It)->GetStartTime() << " Production Rate: " << (*It)->GetProdRate() << " Duration: " << 
			//(*It)->GetDuration() << " Description: " << (*It)->GetDescription() << "\n";

		
	// Print the contain input information to file if debuggings is set on
	if ( Debugging > 0 )
		PrintContainInput( m_VFire[f], LContainValues, firstarrival );

		// If there are no resources to deploy or there are only aerial resources deployed ( i.e. firstarrival = 10000 )

	if ( LContainValues.empty() || firstarrival > FWA.GetESLTime() || size > FWA.GetESLSize() )	{
		
		string SimulationStatus = "No Resources Sent";
		double ffc=0.0;
  		double ffl=0.0;
  		double ffp=0.0;
  		double ffsz=0.0;
  		double ffsw=size;
  		double fft=firstarrival;
  		double fru=0.0;

		// If there are no resources available to send
		if ( LContainValues.empty() )	{
			ffsw = 0.0;
			fft = 0.0;
		}
		
		// Save the results to CResults
		m_VResults.push_back( CResults( m_VFire[ f ], ffc, ffl, ffp, ffsz, ffsw, fft, fru, SimulationStatus, DispLogicFilled, InSeason ));
		m_NumResults++;

		// Print the final fire information to file if debuggins is set on
		if ( Debugging > 0 )
			m_VResults[ m_VResults.size()-1 ].PrintResults();

		// Add the fire to the escape fire list
		AddEscapeFire( m_VFire[f].GetJulianDay(), m_VFire[f].FireStartTime(), m_VFire[f] );
                m_VFire[f].SetSimulateContain( false );


	}		// end no resources sent
	
	else	{

                string Status;
                bool bRunContain = true;
#ifdef IRS_ALLOW_SKIP_CONTAIN
                bRunContain = SimulateContain( firstarrival, size, m_VFire[f] );
#endif
                m_VFire[f].SetSimulateContain( bRunContain );
                if( bRunContain )
                {
                    Status = RunContain( Debugging, f, LContainValues, size, firstarrival, DispLogicFilled, InSeason );
                }
                else
                {
                    Status = "Contained";
                    m_VResults.push_back( CResults( m_VFire[ f ], firstarrival, 0.0, 0.0, 0.0, size, 60, 0.0, Status, DispLogicFilled, InSeason ));
                }

				// If Status is not equal to contain add to escapeFire list
				int Index = m_VResults.size() - 1;
				if ( Index >= 0 )	{

					if ( m_VResults[Index].GetStatus() != "Contained"	)	{

						// Get the time the fire escaped in minutes since Midnight
						int FireTime = m_VResults[Index].GetFireTime();
						FireTime = FireTime + m_VFire[f].FireStartTime();
		
						AddEscapeFire( m_VFire[f].GetJulianDay(), FireTime, m_VFire[f] );

					}

				}

		// Change the workshift variables for the resources that were deployed: CResource m_AvailableTime and m_WorkshiftStartTime
		double FireTime = m_VFire[ f ].FireStartTime() + m_VResults[ m_VResults.size()-1 ].GetFireTime();
		// Round the end time ( in minutes since midnight ) for the fire
		int FireEndTime = static_cast< int >(FireTime);
		if ( FireTime - FireEndTime > 0.5)
			FireEndTime = FireEndTime + 1;
		
		for ( Iterator = LDeployedResources.begin(); Iterator != LDeployedResources.end(); Iterator++ )	{
			// If the fire end time is before sunrise do not adjust aerial or aerial delivered resources
			int SunriseMSM = m_VFire[f].SunriseMSM();
			CGround * IsGround = dynamic_cast< CGround * >( ( *Iterator ));
			if ( FireEndTime > SunriseMSM || IsGround != 0 )	{
				( *Iterator )->AdjustWorkshiftStartTime( m_VFire[ f ] );
		
				// Determine the time when the resource will be available to deploy to another fire
				int PostFireDelay = 0;
				if ( m_VResults[ m_VResults.size()-1 ].GetStatus() == "Contained" )	{
			
					if (FireEndTime < ( *Iterator )->GetInitArrivalTime() )	// Resource is deployed but not used on fire
						PostFireDelay = m_VFire[ f ].GetFWA().GetPostContUnusedDelay( ( *Iterator )->GetDelayType() );
					else
						PostFireDelay = m_VFire[ f ].GetFWA().GetPostContUsedDelay( ( *Iterator )->GetDelayType() );
				}
				else
					PostFireDelay = m_VFire[ f ].GetFWA().GetEscapeDelay( ( *Iterator )->GetDelayType() );
	
				// Determine the time when the resource is next available
				int julian = m_VFire[ f ].GetJulianDay();
			
				// Determine the resources start time for this fire (competition delay)
				int starttime = m_VFire[f].FireStartTime();					// Minutes since midnight for fire start time
				int sunriseMSM = m_VFire[f].SunriseMSM();					// Get Sunrise minutes since midnight
				int initarrivaltime = ( *Iterator )->GetInitArrivalTime();
				int rescavailable = ( *Iterator )->GetAvailableTime();	// Get the time when the resource was available for this fire in minutes since beginning of year
				rescavailable = rescavailable - (24 * 60 * (julian-1));				// Convert to minutes since midnight
			
				// If the resource is aviation can not start until sunrise
				int DispType = ( *Iterator )->GetDispatchType();
				if ( DispType == 0 || DispType == 6 || DispType == 7 || DispType == 8 || DispType == 9 || DispType == 10 )	
					if ( rescavailable < sunriseMSM )
						rescavailable = sunriseMSM;
					
				if ( rescavailable > starttime )
					starttime = rescavailable;							// If the resource was not available at the fire discovery time use when resource was available
			
				// Determine the resource's end time for this fire (workshift end time)
				int endtime = FireEndTime + PostFireDelay;				// Minutes since midnight
				int workshiftstart = ( *Iterator )->GetWorkshiftStartTime();		// Workshift start time in minutes since midnight
				int workshiftlength = ( *Iterator )->GetWorkshiftLength();		// Workshift length in minutes since midnight
				int workshiftend = workshiftstart + workshiftlength;	// Workshift end time in minutes since midnight
				int workshiftendyear = workshiftend + (24 * 60 * (julian-1));	// Workshift end time in minutes since beginning of year
				if ( workshiftend < endtime )
					endtime = workshiftend;								// If the workshift end is reached before the fire containment effort ends

				// Determine endtime for aerial resources
				int endaerial = -1;
				// Determine workshift end time for smokejumper aircraft
				if ( ( *Iterator )->GetDispatchType() == 9	)	{
					CAerial *Aerial = dynamic_cast< CAerial * >( ( *Iterator ));
					if ( Aerial != 0 )	{
						int NextCrewArrival = Aerial->GetNextLoadArrival();		// This is the time the last crew arrived at the fire
						//cout << "Next Crew Arrival Time: " << NextCrewArrival << "\n";
						string displocID = Aerial->GetDispLoc().GetDispLocID();
						double distance = FWA.GetDistance( displocID );
						int Returntime = static_cast< int >( ( (distance * 60 ) / ( *Iterator )->GetRescType().GetAvgSpeed() ) + 0.5 );
						endaerial = NextCrewArrival + Returntime + m_VFire[f].FireStartTime();
						//cout << "End Time: " << endaerial << "\n";
					}
					else	{
						// bad pointer
						cout << "Bad pointer for smokejumper aircraft for resource work year end time \n";
					}
				}

				// Determine workshift end time for helicopter 
				if ( ( *Iterator )->GetDispatchType() == 6	)	{

					CHelicopter* Helic = dynamic_cast< CHelicopter* >( *Iterator );

					bool IsCarrier = false;

					if ( Helic != 0 )
						IsCarrier = Helic->GetIsCarrier();

					// Carrier Helicopter available after delivering Helitack
					if ( !m_VFire[f].GetWaterDropsAllowed() || ( IsCarrier && !HelicoptersStayOnFire ) )	{
						CAerial *Aerial = dynamic_cast< CAerial * >( ( *Iterator ) );
						if ( Aerial != 0 )	{
							int NextCrewArrival = Aerial->GetNextLoadArrival();		// This is the time the last crew arrived at the fire
							string displocID = Aerial->GetDispLoc().GetDispLocID();
							double distance = FWA.GetDistance( displocID );
							int Returntime = static_cast< int >( ( (distance * 60 ) / ( *Iterator )->GetRescType().GetAvgSpeed() ) + 0.5 );
							endaerial = NextCrewArrival + Returntime + m_VFire[f].FireStartTime();
						}
						else	{
						// bad pointer
						cout << "Bad pointer for helicopter for resource work year end time \n";
						}
					}

					else	{	// Helicopter used for water drops, determine end time based on sunset
						CAerial *Aerial = dynamic_cast< CAerial * >( ( *Iterator ));
						if ( Aerial != 0 )	{
							int DropsBegin = Aerial->GetNextLoadArrival();		// This is the time the last crew arrived at the fire
							//cout << "Next Load Arrival Time: " << DropsBegin << "\n";
							if ( DropsBegin < 1 )
								DropsBegin = ( *Iterator )->GetInitArrivalTime();
							DropsBegin = DropsBegin + ( *Iterator )->GetRescType().GetSetupDelay() + FWA.GetReloadDelay(3);
							//cout << "Time Drops Begin: " << DropsBegin << "\n";
							if ( DropsBegin < firstarrival ) 
								DropsBegin = firstarrival;
							//cout << "Time Drops Begin adjusted for ground personnel: " << DropsBegin << "\n";
							int DropsBeginMSM = m_VFire[f].FireStartTime() + DropsBegin;
							//cout << "Time in MSM the drops begin: " << DropsBeginMSM << "\n";
							int DropDuration = m_VFire[f].SunsetMSM() - DropsBeginMSM;
							//cout << "Duration of time the helicopter can drop: " << DropDuration << "\n";
							int ReturnTime = FWA.GetReloadDelay(3) + 1;
							int LoadsDelivered = static_cast< int >((DropDuration / ReturnTime) + 0.5 );
							int DropTime = LoadsDelivered * ReturnTime;
							//cout << "Time Helicopter spends dropping: " << DropTime << "\n";
							string displocID = Aerial->GetDispLoc().GetDispLocID();
							double distance = FWA.GetDistance( displocID );
							int Returntime = static_cast< int >( ( (distance * 60 ) / ( *Iterator )->GetRescType().GetAvgSpeed() ) + 0.5 );
							//cout << "Return time for helicotper: " << Returntime << "\n";
							endaerial = DropsBeginMSM + DropTime + Returntime + 1;
							if ( DropDuration < 0 )	{		//  There is not enough time after delivering crews to make any drops
								endaerial = Aerial->GetNextLoadArrival();
								if ( endaerial < 1 )
									endaerial = ( *Iterator )->GetInitArrivalTime();
								endaerial = endaerial + m_VFire[f].FireStartTime() + Returntime + 1;
							}
							//cout << "Aerial Drop time for Helicotper: " << endaerial << "\n";
						}
						else	{
						// bad pointer
						cout << "Bad pointer for helicopter for resource work year end time \n";
						}
					}
				}
			
				// Determine workshift end time for other aerial resources 
				if ( ( *Iterator )->GetDispatchType() == 0 || ( *Iterator )->GetDispatchType() == 8	)	{
					// Aircraft used for drops, determine end time based on sunset
						int DropsBegin = ( *Iterator )->GetInitArrivalTime();
					if ( DropsBegin < firstarrival )
						DropsBegin = firstarrival;
					int DropsBeginMSM = m_VFire[f].FireStartTime() + DropsBegin;
					//cout << "Airtanker drops begin: " << DropsBeginMSM << "\n";
					int DropDuration = m_VFire[f].SunsetMSM() - DropsBeginMSM;
					string displocID = ( *Iterator )->GetDispLoc().GetDispLocID();
					double distance = FWA.GetDistance( displocID );
					int Returntime = static_cast< int >( ( ( distance * 60 ) / ( *Iterator )->GetRescType().GetAvgSpeed() ) + 0.5 );
					int ReturnTime = 1;
					if ( ( *Iterator )->GetRescType().GetRescType() == "SCP" )
						ReturnTime = ReturnTime + FWA.GetReloadDelay(0);
					if ( ( *Iterator )->GetRescType().GetRescType() == "SEAT" )
						ReturnTime = ReturnTime + FWA.GetReloadDelay(1);
					if ( ( *Iterator )->GetRescType().GetRescType() == "ATT" )
						ReturnTime = ReturnTime + FWA.GetReloadDelay(4);
					int LoadsDelivered = static_cast< int >(DropDuration / ReturnTime + 0.5);
					int DropTime = LoadsDelivered * ReturnTime;
					//cout << "Airtanker Drop Time: " << DropTime << " Loads delivered: " << LoadsDelivered << " Return Time: " << ReturnTime << "\n";
					endaerial = DropsBeginMSM + DropTime + Returntime + 1;
					//cout << "Airtanker end time: " << endaerial << "\n";
				}

				// If the fire endtime is less than the time calculated for the aerial resources use the fire endtime
				if ( endaerial < endtime && endaerial > 0 )
					endtime = endaerial;


				string effort = Status;								// Final Fire Status
				int firenum = m_VFire[f].GetFireNumber();
				if ( starttime < endtime )	{	// Don't store the work rest if the resource doesn't start toward the fire until after the fire effort is complete
					int AvailTime = ( *Iterator )->DetermineWorkRest( scenario, firenum, julian, starttime, endtime, effort);
			
					//cout << "Resource ID: " << m_VResource[ *Iterator ]->GetRescID() << " Stored Available Time: " << m_VResource[ *Iterator ]->GetAvailableTime()
					//<< " New Available Time: " << AvailTime << "\n";
					( *Iterator )->SetAvailableTime( AvailTime );

					if ( DispMethod == 1 && ( *Iterator )->GetDispatchType() == 0 && MoveAT )	{
					// If the resource is a large airtanker change the base to the closest airtanker base to the FWA
					string NewBase = m_VFire[f].GetFWA().GetClosestATBase();

					// Create an iterator to the dispatch map to contain returned values
					std::map<std::string, OmffrNode<CDispatchBase*>*>::iterator DispMapIt;

					// Find the dispatch location in the DispMap
					DispMapIt = m_DispMap.find( NewBase );
					
					// Get a pointer to the CDLDispatcher for this dispatch location
					OmffrNode< CDispatchBase* > *NewNode = DispMapIt->second;

					// Get the dispatch location where the airtanker came from
					string OldBase = ( *Iterator )->GetCurrentLocation();

					// Find the dispatch location in the DispMap
					DispMapIt = m_DispMap.find( OldBase );

					// Get a pointer to the CDLDispatcher for this dispatch location
					OmffrNode< CDispatchBase* > *OldNode = DispMapIt->second;

					// If the old base and new base are different move the airtanker
					if ( OldBase != NewBase )	{
					
						// Remove the airtanker from the resource map of the old node
						CDLDispatcher* DLDispatcherPtr = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( OldNode ) );
						if ( DLDispatcherPtr != 0 )
							DLDispatcherPtr->RemoveFromRescMap( *Iterator );
						else
							cout << "Bad Pointer trying to remove an airtanker from the resource map of the old dispatch location \n";

						// Add the airtanker to the resource map of the new node
						DLDispatcherPtr = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( NewNode ) ) ;
						if ( DLDispatcherPtr != 0 )
							DLDispatcherPtr->InsertInRescMap( *Iterator );
						else
							cout << "Bad Pointer trying to insert an airtanker into the resource map of the new dispatch location \n";

						// Add the airtankers movement to the movement vector for the airtanker
						int TimeMBY = endtime + ( m_VFire[f].GetJulianDay() - 1 ) * 1440;
						( *Iterator )->RecordMove( TimeMBY, NewBase );

						//cout << "Moved Airtanker from: " << OldBase << " to: " << NewBase << "\n";
					}

					}

				}

			}		// End fire end time is greater than sunrise or is ground resource
			
			// Update the count of the number of dispatches for the day for the DLDispatcher the resource is located at
			string dispLocId = (*Iterator)->GetCurrentLocation();
			int julian = m_VFire[f].GetJulianDay();
			UpdateDLDispatcherDispatchCount(dispLocId, julian);
		}
		// Print the resource work year information to file if debuggings is set on
	if ( Debugging > 0 )
		PrintRescWorkYear( LDeployedResources, m_VFire[f], FireEndTime );
		//cout << "Printed resource work year information \n";
	}

	// If using the dispatchers to deploy resources
	if ( DispMethod == 1 )
		UpdateDispatcherLevels( m_VFire[f], m_VResults.size()-1, DispLogicFilled );
		
	return true;
}

// Run the Contain Algorithm
string CRunScenario::RunContain( int Debugging, int f, std::list< CContainValues > LContainValues, double size, int firstarrival, bool DispLogicFilled, bool InSeason )
{
	CFire fire = m_VFire[f];
	CFWA FWA = fire.GetFWA();
	
	int i;
	int StartTimeMins = fire.FireStartTime();
  	double feReportSize = FWA.GetDiscoverySize();
	// Adjust ROS using FWA's fuel model ROS adjustment factor if appropriate
	double ROS = fire.GetROS();
	int FuelModel = fire.GetFuelModel();
	ROS = FWA.FuelModelAdjROS( FuelModel, ROS );
	double feReportRate = ROS;  
	if ( feReportRate < 0.00001 )
		feReportRate = 0.00001;											// Contain algorithm can not deal with ROS = 0
  	double feLWRatio = fire.GetLtoWRatio();
  	/*
        ** Setting to 0 due to time issues in ignitions.
        */
        //double feAttackDistance = fire.GetDistance();
        double feAttackDistance = 0;
  	bool simRetry=true;
  	int simMinSteps = 250; /* fire.GetMinNumSteps(); */
  	int simMaxSteps = 10000; /* fire.GetMaxNumSteps(); */
    int fireESL = FWA.GetESLSize(); 
    int maxFireTime = FWA.GetESLTime();
  	//cout << "runsim 2\n";

	double DiurnalROS[24];

	for(i=0; i<24; i++)
		DiurnalROS[i] = FWA.GetDiurnalCoefficient(i) * ROS;
	
  	// Enter the Contain Values
	std::list< Sem::ContainResource * > LContainResources;

	double SegStart = 0.0;
	double ProdRate = 0.0;
	double Duration = 0.0;

	char* Description = "Resource";
	double Daily = 0.0;
	double Hourly = 0.0;
	int RescCount = 0;													// Count of the resources in LContainResources

	std::list< CContainValues >::iterator Iterator;
	for ( Iterator = LContainValues.begin(); Iterator != LContainValues.end(); Iterator++ )	{
		SegStart = static_cast< double >(( Iterator )->GetStartTime());
		ProdRate = ( Iterator )->GetProdRate();
		Duration = static_cast< double >(( Iterator )->GetDuration());
		string Desc = ( Iterator )->GetDescription();
		Description = &Desc[0];
		Daily = ( Iterator )->GetDailyCost();
		Hourly = ( Iterator )->GetHourlyCost();
		
		LContainResources.push_back( new Sem::ContainResource( SegStart, ProdRate, Duration, Sem::LeftFlank, Description, Daily, Hourly ));
	}

	string SimulationStatus = "";
try{
	// Add the resources to the containforce
	std::list< Sem::ContainResource * >::iterator It;
	Sem::ContainForce *cf =  new Sem::ContainForce; 
	
	for ( It = LContainResources.begin(); It != LContainResources.end(); It++ )
		cf->addResource(*It);       
      
	Sem::Contain::ContainTactic ct =  Sem::Contain::RearAttack;
	if ( fire.GetTactic() == "HEAD" )		
  		ct = Sem::Contain::HeadAttack;

  	Sem::ContainSim sim(feReportSize,feReportRate, DiurnalROS, StartTimeMins, feLWRatio, cf, ct, feAttackDistance, simRetry, simMinSteps, simMaxSteps, fireESL, maxFireTime );
	sim.run();

	// Delete LContainResources and cf members - this is handled in the destructor for cf
	delete cf;
	
  	
  	Sem::Contain::ContainStatus status = sim.status();

	switch (status)	{
		case 0:	
			SimulationStatus = "Unreported";
			break;
		case 1:
			SimulationStatus = "Reported";
			break;
		case 2:
			SimulationStatus = "Attacked";
			break;
		case 3:
			SimulationStatus = "Contained";
			break;
		case 4:
			SimulationStatus = "Overrun";
			break;
		case 5:
			SimulationStatus = "Exhausted";
			break;
		case 6:
			SimulationStatus = "Overflow";
			break;
		case 7:
			SimulationStatus = "SizeLimitExceeded";
			break;
		case 8:
			SimulationStatus = "TimeLimitExceeded";
			break;
		default:
			SimulationStatus = "Oops Something went wrong";
			break;
	}
			

				 
  	double ffc=sim.finalFireCost();
  	double ffl=sim.finalFireLine();
  	double ffp=sim.finalFirePerimeter();
	double ffsz=sim.finalFireSize();
	double ffsw=sim.finalFireSweep();
  	double fft=sim.finalFireTime();
  	double fru=sim.finalResourcesUsed();

	// Count the resources that were used for the containment of this fire
	double rcount = 0;
	std::vector< string >VRescUsed;
	std::vector< string >::iterator IT;
	for ( Iterator = LContainValues.begin(); Iterator != LContainValues.end(); Iterator++ )	{
		if ( (*Iterator).GetStartTime() <= fft )	{
			string Resource = (*Iterator).GetDescription(); 
			bool found = false;
			
			for ( IT = VRescUsed.begin(); IT != VRescUsed.end(); IT++ )	{
				if ( *IT == Resource )
					found = true;
			}

			if ( !found )
				VRescUsed.push_back( Resource );

		}
	}

	// Set fru the number of resources used to the number of entries in VRescUsed
	fru = static_cast< double >(VRescUsed.size());

	// Save the results to CResults
	double FirstArrival = static_cast< double >(firstarrival);
	m_VResults.push_back( CResults( fire, FirstArrival, ffl, ffp, ffsz, size, fft, fru, SimulationStatus, DispLogicFilled, InSeason ));
	m_NumResults++;

	// Print the fire results information to file if debugging is set on
	if ( Debugging > 0 )
		m_VResults[ m_VResults.size()-1 ].PrintResults();
	
	// Print the fire containment information to file if debugging is set on
	if ( Debugging > 0 )
		PrintContainInfo( f, ffc, ffl, ffp, ffsz, ffsw, fft, fru, SimulationStatus );
	

	}catch(...){
		cout << "runsim failed\n";
	}	

	//_getch();

	
	return SimulationStatus;
  }

  // Run the scenario
  bool CRunScenario::RunScenario( int Debugging, int Scenario,
          int (*pfnProgress)(double dfProgress, const char *pszMessage, void *pArg ) )
  {
	 cout << "Running Scenario \n";
	 //cout << "Number of Fires: " << m_NumFire << "\n";
	 
	  if ( m_VFire.empty() )	{
		  cout << "*******************No fires to run****************\n";
		  return 0;
	  }
	  else	{
              if(pfnProgress)
                  pfnProgress(-1, "Running Scenarios...", NULL);
		// iterate through the fires and run each one

                LoadExpectedLevels( Scenario, 2 );
		for ( int j = 0; j < m_VFire.size(); j++ )
		{
			// Run each individual fire
			int Julian = m_VFire[j].GetJulianDay();
			assert( Julian > 0 && Julian < 366 );
			DeployResources( Debugging, j, Scenario );
			//cout << "Run Fire: " << j << "\n";
                        if(pfnProgress)
                        {
                            pfnProgress((double)j / m_VFire.size(), "Running Scenario...", NULL);
                        }
		}
	  }
	  return 1;
  }

  void CRunScenario::PrintFireInfo( int f )
  {
	  CFire fire = m_VFire[f];
	  CFWA& fwa = fire.GetFWA();
	  CDispLogic& displogic = fwa.GetDispLogic();

	  // Print Information to debug the program to a file
	  // Open a file to print to
	  ofstream outFile( "DebugInfo.dat" , ios::out );
	  
	  // Exit program if unable to create file
	  if ( !outFile ) { 
		  cerr << "Output file could not be opened" << endl;
			  //exit( 1 );
	  }

	  // Print Fire Information 
	  outFile << "Fire Number: " << fire.GetFireNumber() << "\n";
	  outFile << "Julian Day:" << fire.GetJulianDay() << "\n";
	  outFile << "Fire Day of Week: " << fire.GetDayofWeek() << "\n";
	  outFile << "Fire Discovery Time: " << fire.GetDiscoveryTime() << "\n";
	  outFile << "Burning Index: "	<< fire.GetBI() << "\n";
	  outFile << "Rate of Spread: " << fire.GetROS() << "\n";
	  outFile << "Fuel Model: " << fire.GetFuelModel() << "\n";
	  outFile << "Specific Condition: " << fire.GetSpecificCondition() << "\n";
	  outFile << "Slope Percent: " << fire.GetSlopePct() << "\n";
	  outFile << "Is Walk In? " << fire.GetIsWalkIn() << "\n";
	  outFile << "Tactic: " << fire.GetTactic() << "\n";
	  outFile << "Attack Distance: " << fire.GetDistance() << "\n";
	  outFile << "Elevation: " << fire.GetElevation() << "\n";
	  outFile << "Length to Width Ratio: " << fire.GetLtoWRatio() << "\n";
	  outFile << "Minimum number of Steps: " << fire.GetMinNumSteps() << "\n";
	  outFile << "Maximum number of Steps: " << fire.GetMaxNumSteps() << "\n";
	  outFile << "Sun Rise: " << fire.GetSunRise() << "\n";
	  outFile << "Sun Set: " << fire.GetSunSet() << "\n";
	  outFile << "Water Drops Allowed: " << fire.GetWaterDropsAllowed() << "\n";
	  outFile << "Pump and Roll Tactic: " << fire.GetIsPumpNRoll() << "\n";
	  
	  // Print FWA Information
	  outFile << "FWA ID: " << fwa.GetFWAID() << "\n";
	  outFile << "	Index: " << fwa.GetIndex() << "\n";
	  outFile << "	FMG: " << fwa.GetFMG() << "\n";
	  outFile << "	Percent Walk-in fires: " << fwa.GetWalkInPct() << "\n";
	  outFile << "	Percent Pump and Roll fires: " << fwa.GetPumpnRoll() << "\n";
	  outFile << "	Percent Head fires: " << fwa.GetHead() << "\n";
	  outFile << "	Percent Tail fires: " << fwa.GetTail() << "\n";
	  outFile << "	Percent Parallel fires: " << fwa.GetParallel() << "\n";
	  outFile << "	Attack Distance: " << fwa.GetAttackDist() << "\n";
	  outFile << "	Water Drops allowed? " << fwa.GetWaterDrops() << "\n";
	  outFile << "	FWA excluded from analysis? " << fwa.GetExcluded() << "\n";
	  outFile << "	Discovery size: " << fwa.GetDiscoverySize() << "\n";
	  outFile << "	Escape Time: " << fwa.GetESLTime() << "\n";
	  outFile << "	Escape Size: " << fwa.GetESLSize() << "\n";
	  outFile << "	Air to ground ratio: " << fwa.GetAirtoGround() << "\n";
	  outFile << "	Walk-in delays: Tracked: " << fwa.GetWalkInDelay(0) << " Boat: " << fwa.GetWalkInDelay(1) << " Crew: " << fwa.GetWalkInDelay(2)
		<< " Engine: " << fwa.GetWalkInDelay(3) << " Helitack: " << fwa.GetWalkInDelay(4) << " Smokejumpers: " << fwa.GetWalkInDelay(5) << "\n";
	  outFile << "	Post Contained Delay Used: Tracked: "<< fwa.GetPostContUsedDelay(0) << " Boat: " << fwa.GetPostContUsedDelay(1) << " Crew: " << fwa.GetPostContUsedDelay(2)
		<< " Engine: " << fwa.GetPostContUsedDelay(3) << " Helitack: " << fwa.GetPostContUsedDelay(4) << " Smokejumpers: " << fwa.GetPostContUsedDelay(5) << "\n";
	  outFile << "	Post Contained Delay Unused: Tracked: "<< fwa.GetPostContUsedDelay(0) << " Boat: " << fwa.GetPostContUsedDelay(1) << " Crew: " << fwa.GetPostContUsedDelay(2)
		<< " Engine: " << fwa.GetPostContUsedDelay(3) << " Helitack: " << fwa.GetPostContUsedDelay(4) << " Smokejumpers: " << fwa.GetPostContUsedDelay(5) << "\n";
	  outFile << "	Post Escaped Delay: Tracked: "<< fwa.GetEscapeDelay(0) << " Boat: " << fwa.GetEscapeDelay(1) << " Crew: " << fwa.GetEscapeDelay(2)
		<< " Engine: " << fwa.GetEscapeDelay(3) << " Helitack: " << fwa.GetEscapeDelay(4) << " Smokejumpers: " << fwa.GetEscapeDelay(5) << "\n";
	  outFile << "	Reload Delay: Scooper: " << fwa.GetReloadDelay(0) << " SEAT: " << fwa.GetReloadDelay(1) << " Engine: " << fwa.GetReloadDelay(2) << " Helicopter: " << fwa.GetReloadDelay(3) << " Airtanker: " << fwa.GetReloadDelay(4) << "\n";
	  outFile << "	First unit Delay: " << fwa.GetFirstUnitDelay() << "\n";
	  outFile << "	Diurnal Coefficients: 0000: " << fwa.GetDiurnalCoefficient(0) << " 0100: " << fwa.GetDiurnalCoefficient(1) << " 0200: " << fwa.GetDiurnalCoefficient(2)
		<< " 0300: " << fwa.GetDiurnalCoefficient(3) << " 0400: " << fwa.GetDiurnalCoefficient(4) << " 0500: " << fwa.GetDiurnalCoefficient(5) << " 0600: " << fwa.GetDiurnalCoefficient(6)
		<< " 0700: " << fwa.GetDiurnalCoefficient(7) << " 0800: " << fwa.GetDiurnalCoefficient(8) << " 0900: " << fwa.GetDiurnalCoefficient(9) << " 1000: " << fwa.GetDiurnalCoefficient(10)
		<< " 1100: " << fwa.GetDiurnalCoefficient(11) << " 1200: " << fwa.GetDiurnalCoefficient(12) << "\n"
		<< " 1300: " << fwa.GetDiurnalCoefficient(13) << " 1400: " << fwa.GetDiurnalCoefficient(14)
		<< " 1500: " << fwa.GetDiurnalCoefficient(15) << " 1600: " << fwa.GetDiurnalCoefficient(16) << " 1700: " << fwa.GetDiurnalCoefficient(17) << " 1800: " << fwa.GetDiurnalCoefficient(18)
		<< " 1900: " << fwa.GetDiurnalCoefficient(19) << " 2000: " << fwa.GetDiurnalCoefficient(20) << " 2100: " << fwa.GetDiurnalCoefficient(21) << " 2200: " << fwa.GetDiurnalCoefficient(22)
		<< " 2300: " << fwa.GetDiurnalCoefficient(23) << " \n";
	  outFile << "	ROS adjustment by fuel model: " << "\n";
	  int i = 0;
	  while (fwa.GetROSFuelAdj(i) != "x" )	{
		outFile << " Fuel Model: " << fwa.GetROSFuelAdj(i) << " ROS Adjustment: " << fwa.GetAdjustment(i) << "\n";
		i++;
	  }

	  // Print Dispatch logic for this fire
	  int NumLevels = displogic.GetNumLevels();
	  outFile << "Dispatch Logic ID: " << displogic.GetLogicID() << "\n";
	  outFile << "	Dispatch Logic Index: " << displogic.GetIndex() << "\n";
	  outFile << "	Number of Levels: " << displogic.GetNumLevels() << "\n";
	  outFile << "	Break Points: ";
	  for ( int i = 0; i < NumLevels; i++ )
			outFile << displogic.GetBreakPoint(i) << " ";
	  outFile << "\n Maximum Number of Resources for each Type: ";
	  outFile << "\n		ATT: ";
	  for ( int j = 0; j < NumLevels; j++ )
			outFile << displogic.GetRescNum(0, j) << " ";
	  outFile << "\n		CRW: ";
	  for ( int j = 0; j < NumLevels; j++ )
			outFile << displogic.GetRescNum(1, j) << " ";
	  outFile << "\n		DZR: ";
	  for ( int j = 0; j < NumLevels; j++ )
			outFile << displogic.GetRescNum(2, j) << " ";
	  outFile << "\n		ENG: ";
	  for ( int j = 0; j < NumLevels; j++ )
			outFile << displogic.GetRescNum(3, j) << " ";
	  outFile << "\n		FBDZ: ";
	  for ( int j = 0; j < NumLevels; j++ )
			outFile << displogic.GetRescNum(4, j) << " ";
	  outFile << "\n		FRBT: ";
	  for ( int j = 0; j < NumLevels; j++ )
			outFile << displogic.GetRescNum(5, j) << " ";
	  outFile << "\n		Helicopter: ";
	  for ( int j = 0; j < NumLevels; j++ )
			outFile << displogic.GetRescNum(6, j) << " ";
	  outFile << "\n		HELI: ";
	  for ( int j = 0; j < NumLevels; j++ )
			outFile << displogic.GetRescNum(7, j) << " ";
	  outFile << "\n		SCPSEAT: ";
	  for ( int j = 0; j < NumLevels; j++ )
			outFile << displogic.GetRescNum(8, j) << " ";
	  outFile << "\n		SJAC: ";
	  for ( int j = 0; j < NumLevels; j++ )
			outFile << displogic.GetRescNum(9, j) << " ";
	  outFile << "\n		SMJR: ";
	  for ( int j = 0; j < NumLevels; j++ )
			outFile <<displogic.GetRescNum(10, j) << " ";
	  outFile << "\n		TP: ";
	  for ( int j = 0; j < NumLevels; j++ )
			outFile << displogic.GetRescNum(11, j) << " ";
	  outFile << "\n		WT: ";
	  for ( int j = 0; j < NumLevels; j++ )
			outFile << displogic.GetRescNum(12, j) << " ";
	  outFile << "\n";

	  // Print information for each resource
	  if ( m_VResource.empty() )
		  outFile << "No Resources read in" << "\n";
	  else {
		CResource* resc = m_VResource[0];
		  for (int r = 0; r < m_VResource.size(); r++ )	{
			  resc = m_VResource[r];
			  outFile << "Resource ID: " << resc->GetRescID() << "\n";
			  // Print out resource type information
			  outFile << "	Resource Dispatch Type: " << resc->GetDispatchType() << "\n";
			  outFile << "	Resource Delay Type: " << resc->GetDelayType() << "\n";
			  outFile << "	Resource Type: " << resc->GetRescType().GetRescType() << "\n";
			  outFile << "	Average Speed: " << resc->GetRescType().GetAvgSpeed() << "\n";
			  outFile << "	Dispatch Delay: " << resc->GetRescType().GetDispatchDelay() << "\n";
			  outFile << "	Resource Response Delay: " << resc->GetRescType().GetResponseDelay() << "\n";
			  outFile << "	Set-up Delay: " << resc->GetRescType().GetSetupDelay() << "\n";
			  // Print out the dispatch location information
			  outFile << "Dispatch Location ID: " << resc->GetDispLoc().GetDispLocID() << "\n";
			  outFile << "	Callback Delay for dispatch location: " << resc->GetDispLoc().GetCallbackDelay() << "\n";
			  outFile << "	Travel Time for:\n";
			 // for ( int i = 0; i < static_cast< int > ( resc->GetDispLoc().GetAssocFWAsSize() ); i++ )	{
				//outFile << "	" << i << " " << resc->GetDispLoc().GetAssocFWAID(i) << "\n";
			  //}
			 outFile << "Staffing: " << resc->GetStaffing() << "\n";
			 outFile << "Shift Start Time: " << resc->GetStartTime() << "\n";
			 outFile << "Shift End Time: " << resc->GetEndTime() << "\n";
			 outFile << "Start Day of Week for Workshift: " << resc->GetStartDayofWeek() << "\n";
			 outFile << "End Day of Week for Workshift: " << resc->GetEndDayofWeek() << "\n";
			 outFile << "Julian Date for Start of Season: " << resc->GetStartSeason() << "\n";
			 outFile << "Julian Date for End of Season: " << resc->GetEndSeason() << "\n";
			 outFile << "Percent Availability: " << resc->GetPctAvail() << "\n";
			 // Determine if the resource is an engine if it is print the volume
			 CEngine *engineptr = dynamic_cast< CEngine * >( resc );
			 if ( engineptr != 0 )
				outFile << "Volume: " << engineptr->GetVolume() << "\n";
			 CAerial *aerialptr = dynamic_cast< CAerial * >( resc );
			 if ( aerialptr != 0 )
				 outFile << "Volume: " << aerialptr->GetVolume() << "\n";
			 outFile << "Daily Cost: " << resc->GetDailyCost() << "\n";
			 outFile << "Hourly Cost: " << resc->GetHourlyCost() << "\n";
			 outFile << "Initial Arrival Time: " << resc->GetInitArrivalTime()  << "\n";
			 outFile << "Workshift Start Time: " << resc->GetWorkshiftStartTime() << "\n";
			 outFile << "Next Available Time for Resource: " << resc->GetAvailableTime() << "\n";
			 outFile << "Resources Work so far this year \n";
			 for ( int i=0; i < resc->WorkYearSize(); i++ )	{
				CRescWorkYear workyearptr = resc->GetWorkYearEntry(i);
				outFile << "Entry Number: " << i << " Julian Day: " << workyearptr.GetJulianDay() << " Start Time: " 
					<< workyearptr.GetStartTime() << " End Time: " << workyearptr.GetEndTime() << " Effort: " 
					<< workyearptr.GetEffort() << " Met Work/Rest Flag: " << workyearptr.GetFlag() << "\n";
			 }
			 outFile << "\n";
			 
		  }  // end for each resource
	  }    // end else
		
}  // end print file information

// Print information used to deploy resources to a fire
void CRunScenario::PrintDeployInfo( std::list< int > ATT, std::list< int > CREW, std::list< int > DZR, std::list< int > ENG, 
	std::list< int > FBDZ, std::list< int > FRBT, std::list< int > Helicopter, std::list< int > HELI, std::list< int > SCPSEAT, 
	std::list< int > SJAC, std::list< int > SMJR, std::list< int > TP, std::list< int > WT, std::list< int > LDeployedResources, 
	int WTArrival )
{
	// Open a file to print to
	ofstream outFile( "DebugInfo.dat" , ios::app );
	  
	// Exit program if unable to create file
	if ( !outFile ) { 
	  cerr << "Output file could not be opened" << endl;
		  //exit( 1 );
	}
	
	// Print out available resource lists
	if ( m_VResource.empty() )
		outFile << "No resources entered for this FPU";
	else	{
		CResource* resc = m_VResource[0];
		std::list< int >:: iterator it;

		if ( !ATT.empty() )	{
			for ( it = ATT.begin(); it != ATT.end(); it++ )	{
				resc = m_VResource[*it];
				outFile << "Airtanker index: " << *it << " Arrival Time: " << resc->GetInitArrivalTime() << " Production Rate: " << resc->GetProdRate()
					<< " Duration: " << resc->GetDuration() << "\n";
			}
		}
		if ( !CREW.empty() )	{
			for ( it = CREW.begin(); it != CREW.end(); it++ )	{
				resc = m_VResource[*it];
				outFile << "Crew index: " << *it << " Arrival Time: " << resc->GetInitArrivalTime() << " Production Rate: " << resc->GetProdRate()
					<< " Duration: " << resc->GetDuration() << "\n";
			}
		}
		if ( !DZR.empty() )	{
			for ( it = DZR.begin(); it != DZR.end(); it++ )	{
				resc = m_VResource[*it];
				outFile << "Dozer index: " << *it << " Arrival Time: " << resc->GetInitArrivalTime() << " Production Rate: " << resc->GetProdRate()
					<< " Duration: " << resc->GetDuration() << "\n";
			}
		}
		if ( !ENG.empty() )	{
			for ( it = ENG.begin(); it != ENG.end(); it++ )	{
				resc = m_VResource[*it];
				outFile << "Engine index: " << *it << " Arrival Time: " << resc->GetInitArrivalTime() << " Production Rate: " << resc->GetProdRate()
					<< " Duration: " << resc->GetDuration() << "\n";
			}
		}
		if ( !FBDZ.empty() )	{
			for ( it = FBDZ.begin(); it != FBDZ.end(); it++ )	{
				resc = m_VResource[*it];
				outFile << "Fire Boat Dozer index: " << *it << " Arrival Time: " << resc->GetInitArrivalTime() << " Production Rate: " << resc->GetProdRate()
					<< " Duration: " << resc->GetDuration() << "\n";
			}
		}
		if ( !FRBT.empty() )	{
			for ( it = FRBT.begin(); it != FRBT.end(); it++ )	{
				resc = m_VResource[*it];
				outFile << "Fire Boat index: " << *it << " Arrival Time: " << resc->GetInitArrivalTime() << " Production Rate: " << resc->GetProdRate()
					<< " Duration: " << resc->GetDuration() << "\n";
			}
		}
		if ( !Helicopter.empty() )	{
			for ( it = Helicopter.begin(); it != Helicopter.end(); it++ )	{
				resc = m_VResource[*it];
				outFile << "Helicopter index: " << *it << " Arrival Time: " << resc->GetInitArrivalTime() << " Production Rate: " << resc->GetProdRate()
					<< " Duration: " << resc->GetDuration() << "\n";
			}
		}
		if ( !HELI.empty() )	{
			for ( it = HELI.begin(); it != HELI.end(); it++ )	{
				resc = m_VResource[*it];
				outFile << "Helitack/Rappel index: " << *it << " Arrival Time: " << resc->GetInitArrivalTime() << " Production Rate: " << resc->GetProdRate()
					<< " Duration: " << resc->GetDuration() << "\n";
			}
		}
		if ( !SCPSEAT.empty() )	{
			for ( it = SCPSEAT.begin(); it != SCPSEAT.end(); it++ )	{
				resc = m_VResource[*it];
				outFile << "Scooper/Seat index: " << *it << " Arrival Time: " << resc->GetInitArrivalTime() << " Production Rate: " << resc->GetProdRate()
					<< " Duration: " << resc->GetDuration() << "\n";
			}
		}
		if ( !SJAC.empty() )	{
			for ( it = SJAC.begin(); it != SJAC.end(); it++ )	{
				resc = m_VResource[*it];
				outFile << "Smokejumper Aircraft index: " << *it << " Arrival Time: " << resc->GetInitArrivalTime() << " Production Rate: " << resc->GetProdRate()
					<< " Duration: " << resc->GetDuration() << "\n";
			}
		}
		if ( !SMJR.empty() )	{
			for ( it = SMJR.begin(); it != SMJR.end(); it++ )	{
				resc = m_VResource[*it];
				outFile << "Smokejumpers index: " << *it << " Arrival Time: " << resc->GetInitArrivalTime() << " Production Rate: " << resc->GetProdRate()
					<< " Duration: " << resc->GetDuration() << "\n";
			}
		}
		if ( !TP.empty() )	{
			for ( it = TP.begin(); it != TP.end(); it++ )	{
				resc = m_VResource[*it];
				outFile << "Tractor Plow index: " << *it << " Arrival Time: " << resc->GetInitArrivalTime() << " Production Rate: " << resc->GetProdRate()
					<< " Duration: " << resc->GetDuration() << "\n";
			}
		}
		if ( !WT.empty() )	{
			for ( it = WT.begin(); it != WT.end(); it++ )	{
				resc = m_VResource[*it];
				outFile << "Water Tender index: " << *it << " Arrival Time: " << resc->GetInitArrivalTime() << " Production Rate: " << resc->GetProdRate()
					<< " Duration: " << resc->GetDuration() << "\n";
			}
		}
	}
	
		// Print the list of deployed resources
		if ( LDeployedResources.empty() )
			outFile << " \n No Resources Deployed \n";
		else	{
			CResource* resc = m_VResource[0];
			std::list< int >:: iterator it;

			outFile << " \n Deployed Resources \n";
			for ( it = LDeployedResources.begin(); it != LDeployedResources.end(); it++ )	{
				resc = m_VResource[*it];
				outFile << "Resource index: " << *it << " Resource Id: " << resc->GetRescID() << " Resource Type: " << resc->GetRescType().GetRescType() << "\n";
			}
		}	// end else

		// Print out water tender arrival time
		outFile << "Arrival Time for first Water Tender: " << WTArrival << "\n";

}	// end print deploy information

// Print information from contain algorithm
void CRunScenario::PrintContainInfo( int f, double FireCost, double FireLine, double FirePerimeter, double FireSize, double FireSweep,
	double FireTime, double ResourcesUsed, string Status )
{
	// Open a file to print to
	ofstream outFile( "DebugInfo.dat" , ios::app );
	  
	// Exit program if unable to create file
	if ( !outFile ) { 
	  cerr << "Output file could not be opened" << endl;
		  //exit( 1 );
	}

	int firenum = m_VFire[ f ].GetFireNumber();
	outFile << "Results for fire number: " << firenum << "\n";
	outFile << "	Fire Costs: " << FireCost << "\n";
	outFile << "	Fire Line: " << FireLine << "\n";
	outFile << "	Fire Perimeter: " << FirePerimeter << "\n";
	outFile << "	Fire Size: " << FireSize << "\n";
	outFile << "	Fire Sweep: " << FireSweep << "\n";
	outFile << "	Fire Time: " << FireTime << "\n";
	outFile << "	Number of Resources Used: " << ResourcesUsed << "\n";
	outFile << "	Status: " << Status << "\n";
}

// Print the resource values that are being input into the contain algorithm
void CRunScenario::PrintContainInput( CFire fire, std::list< CContainValues > LContainValues, int firstarrivaltime )
{
	// Open a file to print to
	ofstream outFile( "DebugInfo.dat" , ios::app );
	  
	// Exit program if unable to create file
	if ( !outFile ) { 
	  cerr << "Output file could not be opened" << endl;
		  //exit( 1 );
	}

	CFWA FWA = fire.GetFWA();
	
	outFile << "Fire Start Time: " << fire.FireStartTime() << "\n";
  	outFile << "Fire Discovery Size: " << FWA.GetDiscoverySize() << "\n";;
	// Adjust ROS using FWA's fuel model ROS adjustment factor if appropriate
	double ROS = fire.GetROS();
	outFile << "Original Rate of Spread: " << ROS << "\n";
	int FuelModel = fire.GetFuelModel();
	outFile << "Fuel Model: " << FuelModel << "\n";
	outFile << "Adjusted Rate of Spread: " << FWA.FuelModelAdjROS( FuelModel, ROS ) << "\n";
	outFile << "Length to Width Ratio: " << fire.GetLtoWRatio() << "\n";
  	outFile << "Attack Distance: " << fire.GetDistance() << "\n";
  	outFile << "simRetry: true \n";
  	outFile << "Minimum Number of Steps: " << fire.GetMinNumSteps() << "\n";
  	outFile << "Maximum Number of Steps: " << fire.GetMaxNumSteps() << "\n";
    outFile << "Escape Fire Size: " << FWA.GetESLSize() << "\n"; 
    outFile << "Escape Fire Time: " << FWA.GetESLTime() << "\n";
  	
	double DiurnalROS[24];

	for(int i=0; i<24; i++)	{
		DiurnalROS[i] = FWA.GetDiurnalCoefficient(i) * ROS;
		outFile << "Hour " << i << " Diurnal ROS: " << DiurnalROS[i] << "\n";
	}

	outFile << "\n";
	outFile << "Arrival time for the first resource: " << firstarrivaltime << "\n";
	outFile << "\n";

	// Print the production values for the resources in the deployed resources
	std::list< CContainValues >::iterator Iterator;
	outFile << "CONTAIN ALGORITHM INPUT VALUES \n";

	for ( Iterator = LContainValues.begin(); Iterator != LContainValues.end(); Iterator++ )	{	
		outFile << "Start Time: " << (*Iterator).GetStartTime() << " Production Rate: " << (*Iterator).GetProdRate() << " Duration: " << (*Iterator).GetDuration() << "\n";
	}
}	//	print resource input to contain algorithm

// Print the resources work year information
void CRunScenario::PrintRescWorkYear( std::list< CResource* > DeployedResc, CFire fire, int FireEndTime )
{
	std::list< CResource* >::iterator It;									// Iterator for the deployed resources

	// Open a file to print to
	ofstream outFile( "DebugInfo.dat" , ios::app );
	  
	// Exit program if unable to create file
	if ( !outFile ) { 
	  cerr << "Output file could not be opened" << endl;
		  //exit( 1 );
	}

	// Print out fire number, fire start time and fire end time
	outFile << "\nFire Number: " << fire.GetFireNumber() << "\n";
	outFile << "Fire Discovery Time: " << fire.GetDiscoveryTime() << " Fire End Time: " << FireEndTime << "\n";
	
	// Iterate through the deployed resources and print out the last 5 entries in the resource work year vector
	for ( It = DeployedResc.begin(); It != DeployedResc.end(); It++ )
	{
		// Print the resource information
		outFile << "	Resource Index: " << *It << " Resource ID: " << ( *It )->GetRescID() << "\n";
		outFile << "RESOURCE WORK YEAR INFORMATION: \n";
		int WorkYearSize = ( *It )->WorkYearSize();
		int Start = WorkYearSize - 5;
		if ( Start < 0 )
			Start = 0;
		for ( int i = Start; i < WorkYearSize; i++ )
		{ // Get the entry for the work year information
			outFile << " Entry Number: " << i << " ";
			CRescWorkYear WorkYear = ( *It )->GetWorkYearEntry( i );
			outFile << "Julian: " << WorkYear.GetJulianDay() << " Start: " << WorkYear.GetStartTime() 
				<< " End: " << WorkYear.GetEndTime() << " Effort: " << WorkYear.GetEffort() << " Flag: " << WorkYear.GetFlag() << "\n";
		}
	}
}


// Recalculate Crews initial arrival time using carrier information
std::list< int > CRunScenario::FindCrewWorking( CFire fire, std::list< int > &Carrier, std::list< int > &Crew )
{
	// Recalculate the arrivial times for the crew based on available aircraft
	// If there is a problem an empty Crew working list is returned
	std::list< int > Crew_working;									// Make a working list for the crew to reorder by initial arrival time
	std::list< int >::iterator ItCrew;								// Iterator for crew list
	std::list< int >::iterator ItCarrier;							// Iterator for aircraft carrier list
	std::list< int >::iterator ItCreww;								// Iterator for crew working list

	int FWAindex = fire.GetFWA().GetIndex();						// Get the index for the fire's FWA
	// Iterate through the crew list and recalculate the arrival times and put new ordered list in Crew_working
	if ( Crew.empty() )	{
		cout << "There are no crews in the crew list \n";
		return Crew_working;
	}

	else	{
		// There are crews in the crew list
		for ( ItCrew = Crew.begin(); ItCrew != Crew.end(); ItCrew++ )	{

			CDispLoc displocCrew = m_VResource[ *ItCrew ]->GetDispLoc();	// Get the dispatch location for the crew
			string DispLocCrewID = displocCrew.GetDispLocID();
			double Crewdistance = fire.GetFWA().GetDistance( DispLocCrewID );		// Get the travel distance to the fire for the crew
			int Crewdelay = m_VResource[ *ItCrew ]->GetRescType().GetSetupDelay();	// Get crew setup delay
			int CrewDelayType = m_VResource[ *ItCrew ]->DetermineDelayType();		// Walk-in index for crew
			Crewdelay = Crewdelay + fire.GetFWA().GetWalkInDelay( CrewDelayType );	// Add the walkin delay
			int Carriernum = -1;											// Number of deploying aircraft
			int Crewarrival = m_VResource[ *ItCrew ]->GetInitArrivalTime();			// Calculated initial arrival time
			int newarrival = 0;											// New arrival time for crew based on current aircraft
			int minarrival = 10000;										// Calculated minimum arrival time
			int minaircraft = -1;										// Saves the aircraft index for the minumum arrival time

			// Iterate through the carrier list and determine the aircraft that provides the quickest arrival time for the crew
			if ( Carrier.empty() )	{
				cout << "There are not carriers in the carrier list \n";
				return Crew_working;
			}
			else	{
				
				int Carrierarrival = 0;									// Arrival time for the smokejumper aircraft
				// There are carriers in the carrier list
				for ( ItCarrier = Carrier.begin(); ItCarrier != Carrier.end(); ItCarrier++ )	{
					
					// Need to get the carrier pointer in order to use the modified initial arrival function for the aircraft
					CAerial *Air = dynamic_cast< CAerial * >( m_VResource[ *ItCarrier ] );
					if ( Air != 0 )		{
						// Get the dispatch location for the crews that the carrier is delivering
						string displocCarrier = Air->GetCrewDispLoc();

						// If the dispatch location is "x" then the carrier is not assigned to a crew/dispatch location
						if ( displocCarrier == "x" )	{
							// If the carrier aircraft does not have the same dispatch location need to recalculate the initial arrival time for the aircraft with the additional distance
							displocCarrier = m_VResource[ *ItCarrier ]->GetDispLoc().GetDispLocID();	// Get the dispatch location for the smokejumper aircraft
					
							// If the carrier is not working from a disptach location and the carrier's and crew's dispatch location are different
							if ( displocCarrier != displocCrew.GetDispLocID() )	{	// Carrier and crew are not at the same disptach location

								// Recalculate carrier initial arrival time with distance added to pick up crews
								double distance = fire.GetFWA().GetDistance( displocCarrier );
			  					Carrierarrival = Air->CalcInitArrival( fire, Crewdistance, distance );
							}

							else	{
								// If carrier and crews have the same dispatch location
								Carrierarrival = m_VResource[ *ItCarrier ]->GetInitArrivalTime();
							}
						}

						else	{	// Carrier is already working out of dispatch location so it can only deliver crews from that dispatch location
							// if the crew and carrier are at different dispatch locations the carrier can't be used to deploy the crew
							if ( displocCarrier != displocCrew.GetDispLocID() )
								Carrierarrival = -1;
							else
								Carrierarrival = Air->GetNextLoadArrival();
						}
					}

					else	{
						// Did not get the appropriate pointer
						cout << "Bad dynamic_cast for carrier \n";
						Carrierarrival = -1;
					}

					
					// If the crew and carrier have different arrival times use the latest one
					newarrival = Crewarrival;
					if( (Crewarrival - Crewdelay) < Carrierarrival )	
						newarrival = Carrierarrival + Crewdelay;
					
					// Is the new arrival time less than the arrival time using any other aircraft
					if ( newarrival < minarrival && newarrival > Crewdelay )	{
						minarrival = newarrival;						// Save the minimum arrival time
						minaircraft = *ItCarrier;						// Save the minimum arrival time aircraft
					}
				}
			}		// Iterate through carriers

			// Save crews modified initial arrival time
			m_VResource[ *ItCrew ]->SetInitArrivalTime( minarrival );

			// Set the carrier index for the crew, if there are no aircraft that can deliver this crew the carrier index will be set to -1
			SetCarrierIndex( *ItCrew, minaircraft );
			
			// Place Crew in working crew list that orders by arrival times that include carrier information, dispatch location, and duration
			// Keep crews with the same initial arrival time and dispatch location together
			if ( minaircraft != -1 )	{
				if ( Crew_working.empty() )
					Crew_working.push_back( *ItCrew );
				else	{
					// Iterate through the list to find placement for current resource and insert there
					int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
					std::list< int >::iterator Iterator;
					Iterator = Crew_working.begin();
					bool better = false;
					while ( !better && Iterator != Crew_working.end() )	{
						better = m_VResource[ *ItCrew ]->DeploySelectCompare(m_VResource[ *Iterator ]);		// Is this resource better than resource in present position of list?
						if ( !better )
							Iterator++;	}
						
					// If the resource isn't the best so far
					if ( Iterator != Crew_working.begin() )	{
						
						bool equal = true;
						while ( equal && Iterator != Crew_working.begin() )	{
							Iterator--;
							equal = m_VResource[ *ItCrew ]->DeploySelectEqual( m_VResource[ *Iterator ] );		// Is this resource equal to the resource in present position of list?
							//cout << " equal? " << equal << "\n";
							if ( equal )	
								k++;
							}
						Iterator++;

						// Place the resource in the appropriate spot
						if ( k > 0 )	{	
							int random = rand() % k + 1;
							//cout << "Random " << random << "\n";
							for ( int j = 1; j < random ; j++ )
								Iterator++;	}
					}

					// Insert index for resource into list
					Crew_working.insert( Iterator, *ItCrew );

				}	
			}		// end there is a carrier available for this crew so don't add to working crew list	
		}		// end for this crew
	}		// end for all crews

	return Crew_working;
}

// Define the crew and carrier for a load
int CRunScenario::FindBestCarrier( CFire fire, int numcrews, int numresc, int numresca, std::list< int > &Carrier, std::list< int > &Crew_working )
{
	int Crewloadarrival = 0;										// Initial arrival time for each load of crews
	int crewtravel = 0;												// Travel time for crew without delays once crews arrive
	int CrewDelayType = -1;											// Returns index for the crew delays in CFWA
	int crewdelay = 0;												// Delay time once the crew is on the ground
	int Crewcomparearrival = 0;										// Initial arrival time for next crew for comparison
	string displocused = "";										// Dispatch Location crew are coming from
	string disploccompare = "";										// Dispatch Location for next crew for comparison
	//int numcrews = 0;												// Number of crews deployed
	int numcarriers = 0;											// Number of carriers deployed
	int staffing = 0;												// Number of lineproducers in the next crew
	bool groupend = false;											// Flag to indicate that the group has ended based on differece in resource arrival times and dispatch location, or dispatch logic
	std::list< int > DeployedCrews;									// List of crews that are deployed
	std::list< int >::iterator ItCrew;								// Iterator for the crew working list
		
	// Iterate through the crew working list while the initial arrival times and dispatch location are the same to define a group
	// Stop if the number of crews to be deployed is reached before the group is filled
	// This is to determine the staffing level that can be deployed from the first arriving crews so the best sized helicopter can be used
	// Note: This routine assumes that each crew has at least 2 members so don't need to worry about deploying less than 2
	ItCrew = Crew_working.begin(); 

	// Get information for the first crew in the group
	Crewloadarrival = m_VResource[ *ItCrew ]->GetInitArrivalTime();
	CDispLoc Crewdisploc = m_VResource[ *ItCrew ]->GetDispLoc();
	displocused = Crewdisploc.GetDispLocID();
		

	// if need resource (crew) add to deployed list
	if ( numcrews < numresc )	{
		DeployedCrews.push_back( *ItCrew );
		staffing = m_VResource[ *ItCrew ]->GetStaffing();
		numcrews = 1;
	}

	// while the end of the group has not been found
	while ( !groupend )	{
			
		// get information for next crew in working crew list
		ItCrew++;
		if ( ItCrew != Crew_working.end() )	{
			Crewcomparearrival = m_VResource[ *ItCrew ]->GetInitArrivalTime();		// Note for helitack/rappel crews the fastest helicopter type is used to calculate the initial arrival time,  When the carrier is selected the arrival time will be adjusted
			disploccompare = m_VResource[ *ItCrew ]->GetDispLoc().GetDispLocID();

			// Is this crew in the same group?
			if ( Crewcomparearrival == Crewloadarrival && disploccompare == displocused )	{
				// if need resource (crew) add to deployed list
				if ( numcrews <= numresc )	{
					DeployedCrews.push_back( *ItCrew );
					staffing = staffing + m_VResource[ *ItCrew ]->GetStaffing();
					//cout << "Staffing for first load of aerially delivered crews: " << staffing << "\n";
					numcrews++;
					
				}		// If there are now enough crews deployed
				else 
					groupend = true;
				
			}		// If crew from same dispatch location with same arrival time
			else
				groupend = true;									// Have reached the end of the group
		}		// If not at end of working crew list
		else
			groupend = true;
	}		// Have a group of crews from the same dispatch location with the same initial arrival times

	// Print the content of the deployed crew list
	//for ( ItCrew = DeployedCrews.begin(); ItCrew != DeployedCrews.end(); ItCrew++ )	
		//cout << "Deployed crew: " << *ItCrew << "\n";
	
	ItCrew = DeployedCrews.begin();									// ItCrew holds the index for the first crew in the deployed list
			
	// Need to determine appropriate aircraft to deploy resources
	// The number of crew members to deploy is in staffing
	int Carrierused = -1;											// Index number of the carrier used
	//cout << "Number of crews deployed: " << numcrews << "\n";
	int Carriercompare = -1;										// Index of carrier for  comparison
	//std::list< int > DeployedCarriers;							// List containing the deployed carriers
	//std::list< int >::iterator ItCarrier;							// Iterator for the crew working list

	// Get the information about the aircraft that was used to determine the inital arrival time for the first crew in the group

	Carrierused = GetCarrierIndex( *ItCrew );
		
	// If this is a helitack/rappel crew, which helicopter is the best to deliver them
	int CarryCapacity = -1;											// Carrying capacity for best carrier
		
	CrewDelayType = m_VResource[ *ItCrew ]->DetermineDelayType();
	crewdelay = m_VResource[ *ItCrew ]->GetRescType().GetSetupDelay() + fire.GetFWA().GetWalkInDelay( CrewDelayType );	// Delays once crews arrive
	crewtravel = Crewloadarrival - crewdelay;						// Travel time for the crew

	CHelicopter *Helicopter = dynamic_cast < CHelicopter * > (m_VResource[ *Carrier.begin() ]);
	if ( Helicopter != 0 )	{
		Carrierused = Helicopter->BestCarrier( Carrier, m_VResource, staffing, crewtravel, fire, Crewdisploc );		// index for the resource that is the best carrier
		
		if ( Carrierused == -1 )	{
			// The BestCarrier function in CHelicopter returned a bad value
			cout << "The BestCarrier function in CHelicopter returned a bad value \n";
			return Carrierused;
		}

		if ( Carrierused == -10 )	{
			// There is no carrier available that can deliver the load and return to base before the end of their workshift
			//cout << "The BestCarrier function in CHelicopter returned that there is no carrier available to deliver the load before the end of their workshift";
			return Carrierused;
		}
	}
	
	CSMJAircraft *SJAC = dynamic_cast < CSMJAircraft * > (m_VResource[ *Carrier.begin() ]);
	if ( SJAC != 0 )	{
		Carrierused = SJAC->BestCarrier( Carrier, m_VResource, staffing, crewtravel, fire, Crewdisploc );		// index for the resource that is the best carrier
		
		if ( Carrierused == -1 )	{
			// The BestCarrier function in CSMJAircraft returned a bad value
			cout << "The BestCarrier function in CSMJAircraft returned a bad value \n";
			return Carrierused;
		}

		if ( Carrierused == -10 )	{
			// There is no carrier available that can deliver the load and return to base before the end of their workshift
			//cout << "The BestCarrier function in CSMJAIrcraft returned that there is no carrier available to deliver the load before the end of their workshift";
			return Carrierused;
		}
	}
	// Return the index for the best carrier to use for this crew
	return Carrierused;
}

// Deploy the crew and carrier
std::list< int > CRunScenario::DeployCrewAndCarrier( CFire fire, int Carrierused, int numcrews, int numrescc, std::list< int > &Crew_working, std::list< int > &LDeployedResources )
{
	std::list<int> CrewRemoveList;										// List of crews that have been deployed so they can be deleted
	// Have the first carrier and group of crews
	int Carrierarrival = m_VResource[ Carrierused ]->GetInitArrivalTime();
	std::list< int >::iterator ItCrewD;
	ItCrewD = Crew_working.begin();
	// Determine the "once on the ground" delay for the crew
	int CrewDelayType = m_VResource[ *ItCrewD ]->DetermineDelayType();
	int crewdelay = m_VResource[ *ItCrewD ]->GetRescType().GetSetupDelay() + fire.GetFWA().GetWalkInDelay( CrewDelayType );
	int crewarrival = m_VResource[ *ItCrewD ]->GetInitArrivalTime();
	int Crewtravel = crewarrival - crewdelay;
	//cout << "Carrier arrival time: " << Carrierarrival << "\n";
	//cout << "Crew travel time: " << Crewtravel << "\n";

	//if ( Carrierarrival < Crewtravel )	{
	//		cout << "Oops!!!  the carrier's initial arrival time is less than the crew's initial arrival time \n";
	//		return CrewRemoveList;
	//}

	// Can the crew be delivered before sunset?
	if ( fire.SunsetMSM() > fire.FireStartTime() + Carrierarrival )	{	 // Crews can be delivered before sunset make deploy list
			
		// Deploy crews and carrier
		int CarrierCapacity = 0;
		
		// The arrival times for each load of crews
		int CarrierLoadArrival = Carrierarrival;
		int loadsize = 0;											// number of individuals on the load
		bool loadend = false;										// Indicates that the aircraft load is full

		// Get pointer to helicopter
		CHelicopter *Helicopter = dynamic_cast< CHelicopter * >( m_VResource[ Carrierused ]);
		if ( Helicopter != 0 )	{
			int elevation = fire.GetElevation();
			CarrierCapacity = Helicopter->GetNumCrew( elevation );
		}
		else
			CarrierCapacity = 8;									// If not helicopter is smokejumper aircraft with carrying capacity of 8
			
		// Values to compare crews in list and determine if the crews are at same dispatch location and have initial arrival times such that then can be deployed with this load
		string Displocused = m_VResource[ *ItCrewD ]->GetDispLoc().GetDispLocID();
		string Disploccompare = Displocused;
		int Crewtravelcompare = Crewtravel;
		
		while ( ItCrewD != Crew_working.end() && !loadend )	{
			// Is this crew at the same dispatch location and does it have a travel time such that it can be included in this load?
			if ( Displocused == Disploccompare && Crewtravel <= Carrierarrival )	{	
				// Add crew to load size
				int crewstaffing = m_VResource[ *ItCrewD ]->GetStaffing();
				
				// If the crew cannot arrive at the fire before the end of their workshift, do not deploy
				if ( m_VResource[ *ItCrewD ]->CalcRemainingWorkshift( fire ) < 0 )	
					CrewRemoveList.push_back( *ItCrewD );				// eliminate this crew from the available crew list
					
				else	{
					// If the entire crew can be added to the load do so
					if ( loadsize + crewstaffing <= CarrierCapacity )		{
						loadsize = loadsize + crewstaffing;
						numcrews++;
						//cout << "Load size: " << loadsize << "\n";
						// Deploy the crew					
						LDeployedResources.push_back( *ItCrewD );			// Add resource to deployed resource list
						CrewRemoveList.push_back( *ItCrewD );				// Add crew to list of crews to be removed from the available crew list
						// Record the carrier that was used to deliver the crew in their class
						SetCarrierIndex( *ItCrewD, Carrierused );
						// Adjust the initial arrival time for the crew to that of the carrier + ground delays
						m_VResource[ *ItCrewD ]->SetInitArrivalTime( Carrierarrival + crewdelay );

						if ( loadsize == CarrierCapacity || numcrews >= numrescc )	{
							loadend = true;									// this load is full
							//cout << "Loadsize = carriercapacity or numcrews = numrescc " << loadsize << "\n";
						}
						
					}	// Crew can be included in this load
								
					// The aircraft cannot fit the entire crew so it will have to wait for another load
					else	{
						loadend = true;
						//cout << "Addition of the next crew overfills the aircraft \n";
					}	// load full can't include this crew
				}	// Crew will reach the fire before the end of their workshift

				ItCrewD++;
				if ( ItCrewD != Crew_working.end() )	{
					Disploccompare = m_VResource[ *ItCrewD ]->GetDispLoc().GetDispLocID();
					Crewtravelcompare = m_VResource[ *ItCrewD ]->GetInitArrivalTime() - crewdelay;
				}

			}	// End crew can be included in this same load due to dispatch location and arrival time
			
			else	//  have reached the end of the crews that can be included in this load
				loadend = true;
									
		}		// Deployed load of Crews 

	}
	return CrewRemoveList;											// if not empty the load has been successfully deployed
}

// Routine to control all the parts of the crew and carrier deployment
int CRunScenario::CrewAndCarrier( CFire fire, std::list< int > &Carrier, std::list< int > &Crew, int numrescc, int numresca, std::list< int > &LDeployedResources, std::list< int > &UsedCarriers )
{
	 // Iterate throught the carriers in the list and set their Dispatch Location to "x" and their next load arrival time to -1
	 int numcrews = 0;												// Number of crews that have been deployed
	 bool quitflag = false;											// Flag to indicate that cannot deploy more crews due to workshift length or sunset 
	 std::list< int > Crew_working;

	 while ( numcrews < numrescc && !quitflag )
	 {
		// Develop the working crew list with arrival times adjusted for available carriers
		Crew_working.clear();	
		Crew_working = FindCrewWorking( fire, Carrier, Crew );
		// *** Print Results in crew working
		//if ( Crew_working.empty() )
			//cout << " Crew_working is empty \n";
		//std::list< int >::iterator ITCW;
		//for ( ITCW = Crew_working.begin(); ITCW != Crew_working.end(); ITCW++ )	{
			//CAirDelivered *ADCrew = dynamic_cast< CAirDelivered * >(m_VResource[ *ITCW ]);
			//if ( ADCrew != 0 )
			 //cout << "Resource Index: " << *ITCW << " Initial Arrival Time: " << m_VResource[ *ITCW ]->GetInitArrivalTime() << " Carrier ID: " << ADCrew->GetCarrierIndex() << "\n";
		//}

		// Find the best carrier to deliver the first arriving crews in the working crew list to the fire
		std::list< int >::iterator ItCrew;
		ItCrew = Crew_working.begin();
		int Carrierused = GetCarrierIndex( *ItCrew );

		// If this crew is a helitack/rappel crew find the helicopter with the best arrival time and most appropriate carrying capacity to deliver the crew
		Carrierused = FindBestCarrier( fire,  numcrews, numrescc,  numresca,  Carrier,  Crew_working );
	 
		// Deploy the crew and carrier
		if ( Carrierused > -1 )	{									// There is a carrier available to carry this crew
			std::list< int > RemoveCrews;
			double sizecompare = LDeployedResources.size();
			RemoveCrews = DeployCrewAndCarrier( fire, Carrierused, numcrews, numrescc, Crew_working, LDeployedResources );
			double newsize = LDeployedResources.size();

			if ( newsize > sizecompare )		{						// The crew was successfully deployed
			
				// Increase the number of crews deployed 
				numcrews = numcrews + static_cast< int >( newsize - sizecompare );

				// Calculate time to reload 2* travel distance + carrier response delay to set the time the next crew would arrive
				CDispLoc CrewDispLoc = m_VResource[ *ItCrew ]->GetDispLoc();
				string CrewDispLocID = CrewDispLoc.GetDispLocID();
				int NextCrewArrival = static_cast< int >( 2* fire.GetFWA().GetDistance( CrewDispLocID ) * 60 / m_VResource[ Carrierused ]->GetRescType().GetAvgSpeed() + 0.5 );
				NextCrewArrival = NextCrewArrival + m_VResource[ Carrierused ]->GetRescType().GetResponseDelay();
				//cout << "Next crew arrival time added: " << NextCrewArrival << "\n";
				NextCrewArrival = NextCrewArrival + m_VResource[ Carrierused ]->GetInitArrivalTime();
				SetNextLoadArrival( Carrierused, NextCrewArrival );
								
				// Add carrier to dispatch list if necessary and store time for next load and crew's dispatch location for additional loads
				bool IsIn = IsInList( LDeployedResources, Carrierused );
				if ( !IsIn )	{
					LDeployedResources.push_back( Carrierused );
					UsedCarriers.push_back( Carrierused );

					// Set the flag for the helicopters to indicate that the helicopter is used as a carrier and dispatch location carrier is working out of
					CHelicopter *Helicopter = dynamic_cast< CHelicopter * >( m_VResource[ Carrierused ]);
					if ( Helicopter != 0 )	{
						Helicopter->SetIsCarrier( true );
						string CrewDispLocId = CrewDispLoc.GetDispLocID();
						Helicopter->SetCrewDispLoc( CrewDispLocId );
					}
				}

				// If the number of carriers deployed is equal to the number of carriers in the dispatch logic, can use only these carriers
				if ( UsedCarriers.size() == numresca )
					Carrier = UsedCarriers;
			
				// Remove deployed crews from crews list
				for ( ItCrew = RemoveCrews.begin(); ItCrew != RemoveCrews.end(); ItCrew++ )	{
					Crew.remove( *ItCrew );
				}

			}		// End bookkeeping for deploying the crew

			else	{	// There were no crews deployed	because either the crews will not arrive at the fire before the end of their shift, or there is not enough space on the aircraft for an entire crew
				
				// Remove deployed crews from crews list that were added because they couldn't get to the fire before the end of their shift
				for ( ItCrew = RemoveCrews.begin(); ItCrew != RemoveCrews.end(); ItCrew++ )	
					Crew.remove( *ItCrew );

				quitflag  = true;

			}
			// If there are no crews left in the list this routine is done
			if ( Crew.empty() )
				quitflag = true;

		 }		// End deploy crew

		else	{	// There are no carriers available to deploy this crew due to either The carrier not being able to deploy the crews before sunset orno carriers available to deploy resources from this dispatch location
			// Remove all crews from the first crew's dispatch location from the list
			std::list< int > Remove;
			ItCrew = Crew_working.begin();
			string displocused = m_VResource[ *ItCrew ]->GetDispLoc().GetDispLocID();
			for ( ItCrew = Crew.begin(); ItCrew != Crew.end(); ItCrew++ )	{
				if ( m_VResource[ *ItCrew ]->GetDispLoc().GetDispLocID() == displocused )
					Remove.push_back( *ItCrew );
			}

			// Now remove these crews from the Crew list
			for ( ItCrew = Remove.begin(); ItCrew != Remove.end(); ItCrew++ )
				Crew.remove( *ItCrew);

			if ( Crew.empty() )
				quitflag = true;
		}
	 }	// End while there are more crews needed

	 // Before returning set the carriers in the carriers used lists Next Load Arrival Time to the time the last crew they deployed to the fire 
	 std::list< int >::iterator ItCarrier;
	 for ( ItCarrier = UsedCarriers.begin(); ItCarrier != UsedCarriers.end(); ItCarrier++ )	{
		CAerial *Aerial = dynamic_cast< CAerial * >( m_VResource[ *ItCarrier ]);
		if ( Aerial != 0 )	{
			int NextCrewArrival = Aerial->GetNextLoadArrival();
			CDispLoc disploc = Aerial->GetDispLoc();
			string DispLocID = disploc.GetDispLocID();

			// Calculate time to reload 2* travel distance + carrier response delay to set the time the next crew would arrive
			// So it can be removed from the nextloadarrival time and make it the last load arrival time
			int CrewReload = static_cast< int >( 2* fire.GetFWA().GetDistance( DispLocID ) * 60 / m_VResource[ *ItCarrier ]->GetRescType().GetAvgSpeed() + 0.5 );
			CrewReload = CrewReload + m_VResource[ *ItCarrier ]->GetRescType().GetResponseDelay();
			NextCrewArrival = NextCrewArrival - CrewReload;
			//cout << "Next crew arrival time removed: " << NextCrewArrival << "\n";
			//cout << "Reset the next load arrival for bucket work \n";
			Aerial->SetNextLoadArrival( NextCrewArrival );

			// So now m_NextCrewArrival for the deployed carriers contains the time the final crew they delivered arrived at the fire
		}
		else	{
			// bad pointer
			cout << "Bad CAerial pointer for adjusting m_NextCrewArrival to the time when the last crew delivered to the fire arrived \n";
			return -1;
		}

		
	 }		// end for adjusting the deployed carrier's next crew arrival time



	 return 1;														// Have successfully deployed the crews and carriers
}
// Have made the assumptions in the proceeding routine that:
//	- Once a carrier begins deploying crews from one dispatch location it will only deploy crews from that dispatch location
//	- A crew will not be split between multiple carrier loads

// Set the carrier index for crews that are delivered to the fire by aircraft
int CRunScenario::SetCarrierIndex( int crewindex, int carrierindex )
{
	CAirDelivered *ADCrew = dynamic_cast < CAirDelivered * > ( m_VResource[ crewindex ] );
	if ( ADCrew != 0 )
		ADCrew->SetCarrierIndex( carrierindex );
	else	{
		// Did not get the appropriate pointer
		cout << "Bad dynamic_cast for Aerially delivered crew, to set carrier index. \n";
		return -1;
	}
	return 1;
}

// Get the carrier index for crews that are delivered to the fire by aircraft
int CRunScenario::GetCarrierIndex( int crewindex)
{
	CAirDelivered *ADCrew = dynamic_cast < CAirDelivered * > ( m_VResource[ crewindex ] );
	if ( ADCrew != 0 )	{
		int carrierindex = ADCrew->GetCarrierIndex();
		return carrierindex;
	}
	else	{
		// Did not get the appropriate pointer
		cout << "Bad dynamic_cast for Aerially delivered crew, to get carrier index. \n";
		return -1;
	}
}

// Set the next load arrival time for a carrier
int CRunScenario::SetNextLoadArrival( int carrierindex, int nextloadarrival )
{
	CAerial *Aerial = dynamic_cast < CAerial * > ( m_VResource[ carrierindex ] );
	if ( Aerial != 0 )	{
		Aerial->SetNextLoadArrival( nextloadarrival );
		return nextloadarrival;
	}
	else	{
		// Did not get the appropriate pointer
		cout << "Bad dynamic_cast for Carrier to set next load arrival time. \n";
		return -1;
	}
}

// Get the next load arrival time for a carrier
int CRunScenario::GetNextLoadArrival( int carrierindex )
{
	CAerial *Aerial = dynamic_cast < CAerial * > ( m_VResource[ carrierindex ] );
	if ( Aerial != 0 )	{
		int nextloadarrival = Aerial->GetNextLoadArrival();
		return nextloadarrival;
	}
	else	{
		// Did not get the appropriate pointer
		cout << "Bad dynamic_cast for Carrier to set next load arrival time. \n";
		return -1;
	}
}

// Is the integer in the list?  To determine if a resource in a resource list
bool CRunScenario::IsInList( std::list< int > Thelist, int item )
{
	bool IsIn = false;
	
	std::list< int >::iterator It;
	for ( It = Thelist.begin(); It!= Thelist.end(); It++ )	{
		if ( *It == item )
			IsIn = true;
	}

	return IsIn;
}
/**
 * \brief Free up resources
 *
 */
void CRunScenario::Reset()
{
    //Iterate through all external dispatch nodes in the tree
    //The Dispatcher function will interate through the resources and reset the values
    vector< OmffrNode< CDispatchBase* >* > VNodes = m_DispTree.Preorder( m_DispTree.Root() );
    for ( int i = 0; i < VNodes.size(); i++ )
        m_DispTree.Element(VNodes[i])->ResetNewScenario( 100 );
}
        
// Output the desired information for the run
void CRunScenario::Output()
{
			
	// Open a file to print to
	ofstream outFile( "Results.dat" , ios::out );
	  
	// Exit program if unable to create file
	if ( !outFile ) { 
		cerr << "Output file could not be opened" << endl;
		//exit( 1 );
	}

	outFile << "Fire Number" << ", " << "Julian Day" << ", " << "Discovery Time" << ", " << "BI" << ", " << "ROS" << ", " << "Fire Cost"
			<< ", " << "Fire Line" << ", " << "Fire Perimeter" << ", " << "Fire Size" << ", " << "Fire Sweep" << ", " << "Fire Time"
			<< ", " << "Resources Used" << ", " << "Status" << ", " << "Dispatch Logic Filled?" << ", " << "In Season? " << "\n";
	
	// For now just output the information for the fires
	//std::vector< CResults >::iterator It;

	int NumResults = static_cast< int >(m_VResults.size());

        int nEscapes = 0;
        int nNoResSent = 0;
	for ( int j=0 ; j < m_VResults.size(); j++ )
	{
		// Print the desired fire and result information to a file
		CFire fire =  m_VResults[j].GetFire();
                int y = fire.GetScenario();
                std::string fwa = fire.GetFWA().GetFWAID();
		int FireNum = fire.GetFireNumber();
		int JulianDay = fire.GetJulianDay();
		string DiscoveryTime = fire.GetDiscoveryTime();
		int BI = fire.GetBI();
		double ROS = fire.GetROS();
		double FireCost = m_VResults[j].GetFireCost();
  		double FireLine = m_VResults[j].GetFireLine();
		double FirePerimeter = m_VResults[j].GetFirePerimeter();
		double FireSize = m_VResults[j].GetFireSize();
  		double FireSweep = m_VResults[j].GetFireSweep();
  		double FireTime = m_VResults[j].GetFireTime();
  		double ResourcesUsed = m_VResults[j].GetNumRescUsed();
		string Status = m_VResults[j].GetStatus();
		bool DispLogicFilled = m_VResults[j].GetDispLogicFilled();
		bool InSeason = m_VResults[j].GetInSeason();

                if( Status != "Contained" )
                    nEscapes++;
                if( Status == "No Resources Sent" )
                    nNoResSent++;

		outFile << FireNum << ", " << JulianDay << ", " << DiscoveryTime << ", " << BI << ", " << ROS << ", " << FireCost
			<< ", " << FireLine << ", " << FirePerimeter << ", " << FireSize << ", " << FireSweep << ", " << FireTime
			<< ", " << ResourcesUsed << ", " << Status << ", " << DispLogicFilled << ", " << InSeason << ", " << y << ", " << fwa << ", " << fire.m_lat << ", " << fire.m_lon << "\n";

	}

	outFile.close();
        /* Print summary info */
        printf( "Summary:\n"
                "Total Fires: %lu\n"
                "Total Escapes: %d\n"
                "Total Fires w/no resources dispatched: %d\n",
                m_VResults.size(), nEscapes, nNoResSent );
        nEscapes = nNoResSent = 0;
}

/**
 * \brief Build an 'index' for accessing the dispatch locations directly,
 * rather than iterating through elements.  The name/id of the dispatch
 * location will be stored along with pointer to the node that it is stored at.
 * This will allow access to the locations and optionally the tree from the
 * external nodes, rather than the root.  This should only be called once after
 * the tree is built.
 *
 * \note I don't know the class name yet, I am just using int's as place
 *       holders.
 *
 * \param oTree tree to build an index on.
 * \return a map of id and pointers to Dispatch objects
 */
std::map<std::string, OmffrNode<CDispatchBase*>*>
CRunScenario::BuildDispatchIndex( OmffrTree<CDispatchBase*> *oTree )
{
    std::map<std::string, OmffrNode<CDispatchBase*>*> map;
    std::vector<OmffrNode<CDispatchBase*>*> external;
    external = (*oTree).GetExternalNodes( (*oTree).Root(), -1 );
    for( int i = 0; i < external.size(); i++ )
    {
        map.insert( std::pair<std::string,
                    OmffrNode<CDispatchBase*>*>( (*oTree).Element(external[i])->GetDispatcherID(), 
                                                 external[i] ) );
    }
    return map;
}

// Build a map with the ids for the interior nodes and their node for accessing the internal nodes directly 
std::map<std::string, OmffrNode< CDispatchBase* >* > CRunScenario::BuildInternalIndex( OmffrTree< CDispatchBase* > *oTree )
{
	// Get a vector with the internal nodes
	vector< OmffrNode< CDispatchBase* >* > InNodes = GetInternalNodes( (*oTree).Root() );

	// Create the map
	std::map<std::string, OmffrNode<CDispatchBase*>*> map;
	for ( int i = 0; i < InNodes.size(); i++ )	{

		map.insert( std::pair< std::string, OmffrNode< CDispatchBase* >* >( (*oTree).Element( InNodes[i] )->GetDispatcherID(), InNodes[i] ) );

	}

	return map;

}


// Deploy resources from the resource vector not using the dispatcher tree (original method)
std::list< CResource* > CRunScenario::DeployFromRescVector( int f, int Debugging )
{	// Create 13 lists, 1 for each dispatch type.  Order the resources in these lists according to their dispatch ordering, initial arrival time, duration, and production rate
	std::list< int > ATT;												// dispatch type 0
	std::list< int > CREW;												// dispatch type 1
	std::list< int > DZR;												// dispatch type 2
	std::list< int > ENG;												// dispatch type 3
	std::list< int > FBDZ;												// dispatch type 4
	std::list< int > FRBT;												// dispatch type 5
	std::list< int > Helicopter;										// dispatch type 6
	std::list< int > HELI;												// dispatch type 7
	std::list< int > SCPSEAT;											// dispatch type 8
	std::list< int > SJAC;												// dispatch type 9
	std::list< int > SMJR;												// dispatch type 10
	std::list< int > TP;												// dispatch type 11
	std::list< int > WT;												// dispatch type 12
	
	// Iterate through the vector of resources - reset the workshift start time for the resource to -1 if this fire is on a new day, 
	// calculate initial arrival time, duration of time available for fire, and production rate
	// Order the resources by dispatch type by arrival time, (then duration and production rate) in the order for deploying
	// Select the best in each dispatch type to meet the dispatch logic to deploy

	// stop at the second year so can add break points
	//if ( f == 0 && m_VFire[f].GetScenario() == 2)	{
	//	int input;
	//	cin >> input;
	//}


	// Determine if this fire is on a new julian day
	bool newday = true;
	int PreviousDay = 0;
	if ( f > 0 ) {
		PreviousDay = m_VFire[ f-1 ].GetJulianDay();
		if ( m_VFire[ f ].GetJulianDay() == PreviousDay )
			newday = false;
		//cout << " Is new day? " << newday << "\n";
	}

	// Determine if this fire is in a new scenario
	if ( f == 0 )	{
		for ( int i = 0; i < m_VResource.size(); i++ )	{
			m_VResource[i]->SetWorkshiftStartTime( -1);
			m_VResource[i]->ResetAvailableTime();
			m_VResource[i]->ClearRescWorkYear();
			m_VResource[i]->SetAvailableFlag( false );
		}
	}

	// Iterage through the vector of resources
	for ( int i = 0; i < m_VResource.size(); i++ )	{
		// If this fire is on a new day reset the workshift start time for all the resources to -1
		if ( newday == true && f != 0 )	{
			m_VResource[i]->SetWorkshiftStartTime( -1);

			// Add rest time to resources if necessary

			m_VResource[i]->AdjustAvailableTime( );
			//cout << "Resource Number: " << i << " Workshift Start Time: " << m_VResource[i]->GetWorkshiftStartTime() << "\n";

			// Determine if the fire's day is beyond the end of the resource's season and set the available time for the resource beyond the end of the year
			int Julian = m_VFire[f].GetJulianDay();
			m_VResource[i]->EndSeasonAvailableTime( Julian );
		}

		// If the resource is a helicopter set the flag to indicate that it is not used as a carrier
		CHelicopter *Helic = dynamic_cast< CHelicopter * >( m_VResource[i]);
		if ( Helic != 0 )	{
			Helic->SetIsCarrier( false );
		}

		//cout << m_VResource[i]->GetRescID() << "\n";

		if (!m_VResource[i]->IsRescAvailable( m_VFire[f] ))	{			// Resource not available so set values to -1
			//cout << "Resource not available \n";
			m_VResource[i]->SetInitArrivalTime( -1 );
			m_VResource[i]->SetDuration( -1 );
			m_VResource[i]->SetProdRate( -1 );	}
		else	{
			// Calculate the values to determine placement in dispatch type lists
			//cout << "Resource " << i << " Get Dispatch Type: " << m_VResource[i]->GetDispatchType() << "\n";
			double distance = m_VFire[f].GetFWA().GetDistance( m_VResource[i]->GetDispLoc().GetDispLocID() );
			int initarrival = m_VResource[i]->CalcInitArrival( m_VFire[f], distance );		// Calculate the initial arrival time for the resource
			int duration = m_VResource[i]->CalcRemainingWorkshift( m_VFire[f] );	// Calculate the duration of time the resource can work fire
			int timeMSM = m_VResource[i]->GetInitArrivalTime() + m_VFire[f].FireStartTime();
			double prodrate = m_VResource[i]->DetermineProdRate( m_VFire[f], m_VProdRates, timeMSM );	// Get the production rate for the resource
			//cout << "Resource: " << i << " Inital Arrival = " << m_VResource[i]->GetInitArrivalTime() << " Duration = " << m_VResource[i]->GetDuration()
			//	<< " Production Rate = " << m_VResource[i]->GetProdRate() << "\n";

			// If resource can arrive at the fire before it reaches its workshift length
			if ( duration > 0 )	{
				// Place the index for the resource in the appropriate dispatch type list in dispatch order
				//cout << " Resource ID: " << m_VResource[i]->GetRescID() << " Resource's dispatch type: " << m_VResource[i]->GetDispatchType() << "\n";
				switch ( m_VResource[i]->GetDispatchType() )	{
					case 0:                   
						if ( ATT.empty() )
							ATT.push_back( i );
						else	{
							// Iterate through the list to find placement for current resource and insert there
							int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
							std::list< int >::iterator Iterator;
							Iterator = ATT.begin();
							bool better = false;
							while ( !better && Iterator != ATT.end() )	{
								better = m_VResource[ i ]->DeploySelectCompare(m_VResource[ *Iterator ]);		// Is this resource better than resource in present position of list?
								if ( !better )
									Iterator++;	}
							
							// If the resource isn't the best so far
							if ( Iterator != ATT.begin() )	{
						
								bool equal = true;
								while ( equal && Iterator != ATT.begin() )	{
									Iterator--;
									equal = m_VResource[ i ]->DeploySelectEqual( m_VResource[ *Iterator ] );		// Is this resource equal to the resource in present position of list?
									//cout << " equal? " << equal << "\n";
									if ( equal )	
										k++;
									}
								Iterator++;
									
								// Place the resource in the appropriate spot
								if ( k > 0 )	{	
									int random = rand() % k + 1;
									//cout << "Random " << random << "\n";
									for ( int j = 1; j < random ; j++ )
										Iterator++;	}
								
							}
							// Insert index for resource into list
							ATT.insert( Iterator, i );
						}

						break;

					case 1:                   
						if ( CREW.empty() )
							CREW.push_back( i );
						else	{
							// Iterate through the list to find placement for current resource and insert there
							int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
							std::list< int >::iterator Iterator;
							Iterator = CREW.begin();
							bool better = false;
							while ( !better && Iterator != CREW.end() )	{
								better = m_VResource[ i ]->DeploySelectCompare(m_VResource[ *Iterator ]);		// Is this resource better than resource in present position of list?
								if ( !better )
									Iterator++;	}
							
							// If the resource isn't the best so far
							if ( Iterator != CREW.begin() )	{
							
								bool equal = true;
								while ( equal && Iterator != CREW.begin() )	{
									Iterator--;
									equal = m_VResource[ i ]->DeploySelectEqual( m_VResource[ *Iterator ] );		// Is this resource equal to the resource in present position of list?
									//cout << " equal? " << equal << "\n";
									if ( equal )	
										k++;
									}
								Iterator++;

								// Place the resource in the appropriate spot
								if ( k > 0 )	{	
									int random = rand() % k + 1;
									//cout << "Random " << random << "\n";
									for ( int j = 1; j < random ; j++ )
										Iterator++;	}
	
							}
								CREW.insert( Iterator, i );
						}
	
						break;
	
					case 2:                   
						if ( DZR.empty() )
							DZR.push_back( i );
						else	{
							// Iterate through the list to find placement for current resource and insert there
							int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
							std::list< int >::iterator Iterator;
							Iterator = DZR.begin();
							bool better = false;
							while ( !better && Iterator != DZR.end() )	{
								better = m_VResource[ i ]->DeploySelectCompare(m_VResource[ *Iterator ]);		// Is this resource better than resource in present position of list?
								if ( !better )
									Iterator++;	}
						
							// If the resource isn't the best so far
							if ( Iterator != DZR.begin() )	{
						
								bool equal = true;
								while ( equal && Iterator != DZR.begin() )	{
									Iterator--;
									equal = m_VResource[ i ]->DeploySelectEqual( m_VResource[ *Iterator ] );		// Is this resource equal to the resource in present position of list?
									//cout << " equal? " << equal << "\n";
									if ( equal )	
										k++;
									}
								Iterator++;

								// Place the resource in the appropriate spot
								if ( k > 0 )	{	
									int random = rand() % k + 1;
									//cout << "Random " << random << "\n";
									for ( int j = 1; j < random ; j++ )
										Iterator++;	}

							}
								DZR.insert( Iterator, i );
						}
	
						break;

					case 3:                   
						if ( ENG.empty() )
							ENG.push_back( i );
						else	{
							// Iterate through the list to find placement for current resource and insert there
							int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
							std::list< int >::iterator Iterator;
							Iterator = ENG.begin();
							bool better = false;
							while ( !better && Iterator != ENG.end() )	{
								better = m_VResource[ i ]->DeploySelectCompare(m_VResource[ *Iterator ]);		// Is this resource better than resource in present position of list?
								if ( !better )
									Iterator++;	}
						
							// If the resource isn't the best so far
							if ( Iterator != ENG.begin() )	{
						
								bool equal = true;
								while ( equal && Iterator != ENG.begin() )	{
									Iterator--;
									equal = m_VResource[ i ]->DeploySelectEqual( m_VResource[ *Iterator ] );		// Is this resource equal to the resource in present position of list?
									//cout << " equal? " << equal << "\n";
									if ( equal )	
										k++;
									}
								Iterator++;

								// Place the resource in the appropriate spot
								if ( k > 0 )	{	
									int random = rand() % k + 1;
									//cout << "Random " << random << "\n";
									for ( int j = 1; j < random ; j++ )
										Iterator++;	}

							}

							// Insert index for resource into list
							ENG.insert( Iterator, i );
						}
							
						break;

					case 4:                   
						if ( FBDZ.empty() )
							FBDZ.push_back( i );
						else	{
							// Iterate through the list to find placement for current resource and insert there
							int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
							std::list< int >::iterator Iterator;
							Iterator = FBDZ.begin();
							bool better = false;
							while ( !better && Iterator != FBDZ.end() )	{
								better = m_VResource[ i ]->DeploySelectCompare(m_VResource[ *Iterator ]);		// Is this resource better than resource in present position of list?
								if ( !better )
									Iterator++;	}
							
							// If the resource isn't the best so far
							if ( Iterator != FBDZ.begin() )	{
						
								bool equal = true;
								while ( equal && Iterator != FBDZ.begin() )	{
									Iterator--;
									equal = m_VResource[ i ]->DeploySelectEqual( m_VResource[ *Iterator ] );		// Is this resource equal to the resource in present position of list?
									//cout << " equal? " << equal << "\n";
									if ( equal )	
										k++;
									}
								Iterator++;

								// Place the resource in the appropriate spot
								if ( k > 0 )	{	
									int random = rand() % k + 1;
									//cout << "Random " << random << "\n";
									for ( int j = 1; j < random ; j++ )
										Iterator++;	}

							}

							// Insert index for resource into list
							FBDZ.insert( Iterator, i );
						}

						break;

					case 5:                   
						if ( FRBT.empty() )
							FRBT.push_back( i );
						else	{
							// Iterate through the list to find placement for current resource and insert there
							int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
							std::list< int >::iterator Iterator;
							Iterator = FRBT.begin();
							bool better = false;
							while ( !better && Iterator != FRBT.end() )	{
								better = m_VResource[ i ]->DeploySelectCompare(m_VResource[ *Iterator ]);		// Is this resource better than resource in present position of list?
								if ( !better )
									Iterator++;	}
						
							// If the resource isn't the best so far
							if ( Iterator != FRBT.begin() )	{
						
								bool equal = true;
								while ( equal && Iterator != FRBT.begin() )	{
									Iterator--;
									equal = m_VResource[ i ]->DeploySelectEqual( m_VResource[ *Iterator ] );		// Is this resource equal to the resource in present position of list?
									//cout << " equal? " << equal << "\n";
									if ( equal )	
										k++;
									}
								Iterator++;

								// Place the resource in the appropriate spot
								if ( k > 0 )	{	
									int random = rand() % k + 1;
									//cout << "Random " << random << "\n";
									for ( int j = 1; j < random ; j++ )
										Iterator++;	}

							}

							// Insert index for resource into list
							FRBT.insert( Iterator, i );
						}

						break;

					case 6:                   
						if ( Helicopter.empty() )
							Helicopter.push_back( i );
						else	{
							// Iterate through the list to find placement for current resource and insert there
							int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
							std::list< int >::iterator Iterator;
							Iterator = Helicopter.begin();
							bool better = false;
							while ( !better && Iterator != Helicopter.end() )	{
								better = m_VResource[ i ]->DeploySelectCompare(m_VResource[ *Iterator ]);		// Is this resource better than resource in present position of list?
								if ( !better )
									Iterator++;	}
						
							// If the resource isn't the best so far
							if ( Iterator != Helicopter.begin() )	{
							
								bool equal = true;
								while ( equal && Iterator != Helicopter.begin() )	{
									Iterator--;
									equal = m_VResource[ i ]->DeploySelectEqual( m_VResource[ *Iterator ] );		// Is this resource equal to the resource in present position of list?
									//cout << " equal? " << equal << "\n";
									if ( equal )	
										k++;
									}
								Iterator++;

								// Place the resource in the appropriate spot
								if ( k > 0 )	{	
									int random = rand() % k + 1;
									//cout << "Random " << random << "\n";
									for ( int j = 1; j < random ; j++ )
										Iterator++;	}

							}

							// Insert index for resource into list
							Helicopter.insert( Iterator, i );
						}

						break;

					case 7:                   
						if ( HELI.empty() )
							HELI.push_back( i );
						else	{
							// Iterate through the list to find placement for current resource and insert there
							int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
							std::list< int >::iterator Iterator;
							Iterator = HELI.begin();
							bool better = false;
							while ( !better && Iterator != HELI.end() )	{
								better = m_VResource[ i ]->DeploySelectCompare(m_VResource[ *Iterator ]);		// Is this resource better than resource in present position of list?
								if ( !better )
									Iterator++;	}
						
							// If the resource isn't the best so far
							if ( Iterator != HELI.begin() )	{
						
								bool equal = true;
								while ( equal && Iterator != HELI.begin() )	{
									Iterator--;
									equal = m_VResource[ i ]->DeploySelectEqual( m_VResource[ *Iterator ] );		// Is this resource equal to the resource in present position of list?
									//cout << " equal? " << equal << "\n";
									if ( equal )	
										k++;
									}
								Iterator++;

								// Place the resource in the appropriate spot
								if ( k > 0 )	{	
									int random = rand() % k + 1;
									//cout << "Random " << random << "\n";
									for ( int j = 1; j < random ; j++ )
										Iterator++;	}

							}

							// Insert index for resource into list
							HELI.insert( Iterator, i );
						}

						break;

					case 8:                   
						if ( SCPSEAT.empty() )
							SCPSEAT.push_back( i );
						else	{
							// Iterate through the list to find placement for current resource and insert there
							int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
							std::list< int >::iterator Iterator;
							Iterator = SCPSEAT.begin();
							bool better = false;
							while ( !better && Iterator != SCPSEAT.end() )	{
								better = m_VResource[ i ]->DeploySelectCompare(m_VResource[ *Iterator ]);		// Is this resource better than resource in present position of list?
								if ( !better )
									Iterator++;	}
						
							// If the resource isn't the best so far
							if ( Iterator != SCPSEAT.begin() )	{
						
								bool equal = true;
								while ( equal && Iterator != SCPSEAT.begin() )	{
									Iterator--;
									equal = m_VResource[ i ]->DeploySelectEqual( m_VResource[ *Iterator ] );		// Is this resource equal to the resource in present position of list?
									//cout << " equal? " << equal << "\n";
									if ( equal )	
										k++;
									}
								Iterator++;

								// Place the resource in the appropriate spot
								if ( k > 0 )	{	
									int random = rand() % k + 1;
									//cout << "Random " << random << "\n";
									for ( int j = 1; j < random ; j++ )
										Iterator++;	}

							}

							// Insert index for resource into list
							SCPSEAT.insert( Iterator, i );
						}

						break;

						case 9:                   
						if ( SJAC.empty() )
							SJAC.push_back( i );
						else	{
							// Iterate through the list to find placement for current resource and insert there
							int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
							std::list< int >::iterator Iterator;
							Iterator = SJAC.begin();
							bool better = false;
							while ( !better && Iterator != SJAC.end() )	{
								better = m_VResource[ i ]->DeploySelectCompare(m_VResource[ *Iterator ]);		// Is this resource better than resource in present position of list?
								if ( !better )
									Iterator++;	}
						
							// If the resource isn't the best so far
							if ( Iterator != SJAC.begin() )	{
						
								bool equal = true;
								while ( equal && Iterator != SJAC.begin() )	{
									Iterator--;
									equal = m_VResource[ i ]->DeploySelectEqual( m_VResource[ *Iterator ] );		// Is this resource equal to the resource in present position of list?
									//cout << " equal? " << equal << "\n";
									if ( equal )	
										k++;
									}
								Iterator++;

								// Place the resource in the appropriate spot
								if ( k > 0 )	{	
									int random = rand() % k + 1;
									//cout << "Random " << random << "\n";
									for ( int j = 1; j < random ; j++ )
										Iterator++;	}

							}

							// Insert index for resource into list
							SJAC.insert( Iterator, i );
						}

						break;

					case 10:                   
						if ( SMJR.empty() )
							SMJR.push_back( i );
						else	{
							// Iterate through the list to find placement for current resource and insert there
							int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
							std::list< int >::iterator Iterator;
							Iterator = SMJR.begin();
							bool better = false;
							while ( !better && Iterator != SMJR.end() )	{
								better = m_VResource[ i ]->DeploySelectCompare(m_VResource[ *Iterator ]);		// Is this resource better than resource in present position of list?
								if ( !better )
									Iterator++;	}
						
							// If the resource isn't the best so far
							if ( Iterator != SMJR.begin() )	{
						
								bool equal = true;
								while ( equal && Iterator != SMJR.begin() )	{
									Iterator--;
									equal = m_VResource[ i ]->DeploySelectEqual( m_VResource[ *Iterator ] );		// Is this resource equal to the resource in present position of list?
									//cout << " equal? " << equal << "\n";
									if ( equal )	
										k++;
									}
								Iterator++;

								// Place the resource in the appropriate spot
								if ( k > 0 )	{	
									int random = rand() % k + 1;
									//cout << "Random " << random << "\n";
									for ( int j = 1; j < random ; j++ )
										Iterator++;	}
	
							}

							// Insert index for resource into list
							SMJR.insert( Iterator, i );
						}

						break;

					case 11:                   
						if ( TP.empty() )
							TP.push_back( i );
						else	{
							// Iterate through the list to find placement for current resource and insert there
							int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
							std::list< int >::iterator Iterator;
							Iterator = TP.begin();
							bool better = false;
							while ( !better && Iterator != TP.end() )	{
								better = m_VResource[ i ]->DeploySelectCompare(m_VResource[ *Iterator ]);		// Is this resource better than resource in present position of list?
								if ( !better )
									Iterator++;	}
							
							// If the resource isn't the best so far
							if ( Iterator != TP.begin() )	{
						
								bool equal = true;
								while ( equal && Iterator != TP.begin() )	{
									Iterator--;
									equal = m_VResource[ i ]->DeploySelectEqual( m_VResource[ *Iterator ] );		// Is this resource equal to the resource in present position of list?
									//cout << " equal? " << equal << "\n";
									if ( equal )	
										k++;
									}
								Iterator++;

								// Place the resource in the appropriate spot
								if ( k > 0 )	{	
									int random = rand() % k + 1;
									//cout << "Random " << random << "\n";
									for ( int j = 1; j < random ; j++ )
										Iterator++;	}

							}

							// Insert index for resource into list
							TP.insert( Iterator, i );
						}

						break;

					case 12:                   
						if ( WT.empty() )
							WT.push_back( i );
						else	{
							// Iterate through the list to find placement for current resource and insert there
							int k = 0;										// Count of number of resources with the same initial arrival, duration and production rate values.
							std::list< int >::iterator Iterator;
							Iterator = WT.begin();
							bool better = false;
							while ( !better && Iterator != WT.end() )	{
								better = m_VResource[ i ]->DeploySelectCompare(m_VResource[ *Iterator ]);		// Is this resource better than resource in present position of list?
								if ( !better )
									Iterator++;	}
						
							// If the resource isn't the best so far
							if ( Iterator != WT.begin() )	{
							
								bool equal = true;
								while ( equal && Iterator != WT.begin() )	{
									Iterator--;
									equal = m_VResource[ i ]->DeploySelectEqual( m_VResource[ *Iterator ] );		// Is this resource equal to the resource in present position of list?
									//cout << " equal? " << equal << "\n";
									if ( equal )	
										k++;
									}
								Iterator++;

								// Place the resource in the appropriate spot
								if ( k > 0 )	{	
									int random = rand() % k + 1;
									//cout << "Random " << random << "\n";
									for ( int j = 1; j < random ; j++ )
										Iterator++;	}

							}

							// Insert index for resource into list
							WT.insert( Iterator, i );
						}

						break;

					default:
							cout << "Incorrect Dispatch Type for the resource.";
							break;

				}		// End of switch

			}		// End of enough workshift left
			
		}		// Resource is available	

	}		// End iterating through resources

	// Create a list of the resources that are deployed to the fire
	std::list< int > LDeployedResources;
	CFWA& FWA = m_VFire[f].GetFWA();
	int j = 0;
	while ( m_VFWA[j].GetFWAID() != FWA.GetFWAID() && j < static_cast< int >( m_VFWA.size() ) )	
		j++;
	if ( j > static_cast< int >( m_VFWA.size() ))	{
		cout << "Fire's specified FWA not in analysis :";
			}

	CDispLogic& DispLogic = m_VFWA[j].GetDispLogic();

	
	string Value = DispLogic.GetIndex();
	int LogicValue = 0;
	if ( Value == "BI" )
		LogicValue = m_VFire[f].GetBI();
	if ( Value == "ROS" )
		LogicValue = m_VFire[f].GetROS();
		
	// Determine the dispatch level to use
	int level = 0;
	for ( int m = 0; m < DispLogic.GetNumLevels()-1; m++ )	{
		if (  LogicValue > DispLogic.GetBreakPoint(m) )
			level++;	}
	
	// Get the number of resources of each type for the dispatch logic at the appropriate dispatch level
	std::list< int >::iterator Iterator;

	// Determine the arrival time for the water tender(s) if any are deployed to the fire.  If not arrival time = 10000
	int WTArrival = 10000;	

	// Determine if there are any engines deployed to the fire, if not do not deploy water tenders
	bool EngsDeployed = false;
	
	int NumResc = 0;												// Number of resources of the type in the dispatch logic
	// Deploy Type 0 - ATT
	Iterator = ATT.begin();											// Set iterator to the beginning of the list
	NumResc = DispLogic.GetRescNum( 0, level );
	if ( NumResc > 0 && !ATT.empty() )	{
		if ( NumResc > static_cast< int >(ATT.size() )	)			// The minimum of the number of resources from the dispatch logic or the number in the type vector
			NumResc = static_cast< int >(ATT.size());
		for ( int l= 0; l < NumResc; l++ )	{
			LDeployedResources.push_back( *Iterator );
			Iterator++;	} 	}
	// Deploy Type 1 - CREW
	Iterator = CREW.begin();										// Set iterator to the beginning of the list
	NumResc = DispLogic.GetRescNum( 1, level );
	if ( NumResc > 0 && !CREW.empty() )	{
		if ( NumResc > static_cast< int >(CREW.size() )	)			// The minimum of the number of resources from the dispatch logic or the number in the type vector
			NumResc = static_cast< int >(CREW.size());
		for ( int l= 0; l < NumResc; l++ )	{
			LDeployedResources.push_back( *Iterator );
			Iterator++;	}	}
	// Deploy Type 2 - DZR
	Iterator = DZR.begin();
	NumResc = DispLogic.GetRescNum( 2, level );
	if ( NumResc > 0 && !DZR.empty() )	{
		if ( NumResc > static_cast< int >(DZR.size() )	)			// The minimum of the number of resources from the dispatch logic or the number in the type vector
			NumResc = static_cast< int >(DZR.size());
		for ( int l= 0; l < NumResc; l++ )	{
			LDeployedResources.push_back( *Iterator );
			Iterator++;	}	}
	// Deploy Type 3 - ENG
	Iterator = ENG.begin();
	NumResc = DispLogic.GetRescNum( 3, level );
	if ( NumResc > 0 && !ENG.empty() )	{
		if ( NumResc > static_cast< int >(ENG.size() ) )			// The minimum of the number of resources from the dispatch logic or the number in the type vector
			NumResc = static_cast< int >(ENG.size());
		for ( int l= 0; l < NumResc; l++ )	{
			LDeployedResources.push_back( *Iterator );
			EngsDeployed = true;
			Iterator++;	}	}
	// Deploy Type 4 - FBDZ
	Iterator = FBDZ.begin();
	NumResc = DispLogic.GetRescNum( 4, level );
	if ( NumResc > 0 && !FBDZ.empty() )	{
		if ( NumResc > static_cast< int >(FBDZ.size() )	)			// The minimum of the number of resources from the dispatch logic or the number in the type vector
			NumResc = static_cast< int >(FBDZ.size());
		for ( int l= 0; l < NumResc; l++ )	{
			LDeployedResources.push_back( *Iterator );
			Iterator++;	}	}
	// Deploy Type 5 - FRBT
	Iterator = FRBT.begin();
	NumResc = DispLogic.GetRescNum( 5, level );
	if ( NumResc > 0 && !FRBT.empty() )	{
		if ( NumResc > static_cast< int >(FRBT.size() )	)			// The minimum of the number of resources from the dispatch logic or the number in the type vector
			NumResc = static_cast< int >(FRBT.size());
		for ( int l= 0; l < NumResc; l++ )	{
			LDeployedResources.push_back( *Iterator );
			Iterator++;	}	}
	// Deploy Type 6 - Helicopter
	// Iterate throught the helicopters in the list and set their Dispatch Location to "x" and their next load arrival time to -1
	// Need to do this so that will know if any of the helicopters have been used as a carrier which will adjust their start time for bucket work 
	
	std::list< int >::iterator ItCarrier;

	 for ( ItCarrier = Helicopter.begin(); ItCarrier != Helicopter.end(); ItCarrier++ )
	 {
		CAerial *Aerial = dynamic_cast< CAerial * >( m_VResource[ *ItCarrier ]);
		if ( Aerial != 0 ) {
			Aerial->SetNextLoadArrival( 0 );
			Aerial->SetCrewDispLoc( "x" );
			//cout << "Initializing carrier values \n";
		}
		else	{
			// bad pointer to carrier
			cout << "Bad pointer to carrier to initialize carrier's values \n";
			
		}
	 }

	// *****Print information to ensure information going into the helitack/helicopter deployment is ok
	//std::list< int >::iterator It;
	//cout << " Helicopter List -- " << Helicopter.size() << " items \n";
	//for ( It = Helicopter.begin(); It != Helicopter.end(); It++ )
	//cout << "	Resource Index: " << *It << " Resource Id: " << m_VResource[ *It ]->GetRescID() << "\n";
	//cout << " Helitack/Rappel List -- " << HELI.size() << "items \n";
	//for ( It = HELI.begin(); It != HELI.end(); It++ )
		//cout <<	"	Resource Index: " << *It << " Resource Id: " << m_VResource[ *It ]->GetRescID() << "\n";

	// Deploy helitack/rappel (Type 7) and helicopters (Type 6)
	int NumRescA = DispLogic.GetRescNum( 6, level );
	int NumRescC = DispLogic.GetRescNum( 7, level );
	if ( NumRescA > 0 && !Helicopter.empty() )	{
		std::list< int > CarriersUsed;
		if ( NumRescC > 0 && !HELI.empty() )	{
			std::list< int > HELIbk = HELI;
			int deployedHelitack = CrewAndCarrier( m_VFire[f], Helicopter, HELIbk, NumRescC, NumRescA, LDeployedResources, CarriersUsed );
			if ( deployedHelitack < 0 )	{
				// Something didn't work
				cout << "Failure deploying helitack and helicopters \n";
			}
		}
		// Deploy helicopters for bucket work
		if ( m_VFire[f].GetWaterDropsAllowed() )	{
			int NumCarriers = CarriersUsed.size();
			if ( NumCarriers < NumRescA )	{
				// Remove the carriers that are already deployed from the list
				std::list< int > Helicopterbk = Helicopter;
				std::list< int >::iterator ItCarrier;
				for ( ItCarrier = CarriersUsed.begin(); ItCarrier != CarriersUsed.end(); ItCarrier++ )
					Helicopterbk.remove( *ItCarrier );
				if ( NumRescA - NumCarriers > 0 && !Helicopterbk.empty() )	{
					Iterator = Helicopterbk.begin();
					NumResc = NumRescA- NumCarriers;
					while ( NumResc != 0 && Iterator != Helicopterbk.end() )	{
						LDeployedResources.push_back( *Iterator );
						NumResc--;
						Iterator++;
					}
				}
			}
		}
	}
				
	// Deploy Type 8 - SCPSEAT
	Iterator = SCPSEAT.begin();
	NumResc = DispLogic.GetRescNum( 8, level );
	if ( NumResc > 0 && !SCPSEAT.empty() )	{
		if ( NumResc > static_cast< int >(SCPSEAT.size() )	)		// The minimum of the number of resources from the dispatch logic or the number in the type vector
			NumResc = static_cast< int >(SCPSEAT.size());
		for ( int l= 0; l < NumResc; l++ )	{
			LDeployedResources.push_back( *Iterator );
			Iterator++;	}	}
	// Deploy smokejumpers (Type 10) and smokejumper aircraft (Type 9)
	// Iterate throught the helicopters in the list and set their Dispatch Location to "x" and their next load arrival time to -1
	// Need to do this so that will know if any of the helicopters have been used as a carrier which will adjust their start time for bucket work 
	 for ( ItCarrier = SJAC.begin(); ItCarrier != SJAC.end(); ItCarrier++ )
	 {
		CAerial *Aerial = dynamic_cast< CAerial * >( m_VResource[ *ItCarrier ]);
		if ( Aerial != 0 ) {
			Aerial->SetNextLoadArrival( 0 );
			Aerial->SetCrewDispLoc( "x" );
			//cout << "Initializing carrier values \n";
		}
		else	{
			// bad pointer to carrier
			cout << "Bad pointer to carrier to initialize carrier's values \n";
			
		}
	 }

	 // *****Print information to ensure information going into the smokejumper/smokejumper iarcraft deployment is ok
	//std::list< int >::iterator It;
	//cout << " Smokejumper Aircraft List -- " << SJAC.size() << " items \n";
	//for ( It = SJAC.begin(); It != SJAC.end(); It++ )
		//cout << "	Resource Index: " << *It << " Resource Id: " << m_VResource[ *It ]->GetRescID() << "\n";
	//cout << " Smokejumper List -- " << SMJR.size() << "items \n";
	//for ( It = SMJR.begin(); It != SMJR.end(); It++ )
		//cout <<	"	Resource Index: " << *It << " Resource Id: " << m_VResource[ *It ]->GetRescID() << "\n";

	NumRescA = DispLogic.GetRescNum( 9, level );
	NumRescC = DispLogic.GetRescNum( 10, level );

	if ( NumRescA > 0 && !SJAC.empty() )	{
		std::list< int > CarriersUsed;
		if ( NumRescC > 0 && !SMJR.empty() )	{
			std::list< int > SMJRbk = SMJR;
			int deployedSmokejumper = CrewAndCarrier( m_VFire[f], SJAC, SMJRbk, NumRescC, NumRescA, LDeployedResources, CarriersUsed );
			if ( deployedSmokejumper < 0 )	{
				// Something didn't work
				cout << "Failure deploying smokejumpers and smokejumper aircraft \n";
			}
		}
	}

	// Deploy Type 11 - TP
	Iterator = TP.begin();
	NumResc = DispLogic.GetRescNum( 11, level );
	if ( NumResc > 0 && !TP.empty() )	{
		if ( NumResc > static_cast< int >(TP.size() ) )				// The minimum of the number of resources from the dispatch logic or the number in the type vector
			NumResc = static_cast< int >(TP.size());
		for ( int l= 0; l < NumResc; l++ )	{
			LDeployedResources.push_back( *Iterator );
			Iterator++;	}	}
	// Deploy Type 12 - WT
	Iterator = WT.begin();
	NumResc = DispLogic.GetRescNum( 12, level );
	if ( NumResc > 0 && !WT.empty() && EngsDeployed )	{			// EngsDeployed indicates that there are engines deployed.  If not don't sent water tenders
		if ( NumResc > static_cast< int >(WT.size() ) )				// The minimum of the number of resources from the dispatch logic or the number in the type vector
			NumResc = static_cast< int >(WT.size());
		for ( int l= 0; l < NumResc; l++ )	{
			LDeployedResources.push_back( *Iterator );
			
			// Determine the time for the first water tender arrival to the fire
			int arrival = m_VResource[ *Iterator ]->GetInitArrivalTime();
			if ( arrival < WTArrival )	{
				WTArrival = arrival;
				m_VFire[ f ].SetWTArrivalTime( arrival );	}

			Iterator++;	
		}	
	}

	// Iterate through the deployed resource list and create a list with pointers to the CResources instead
	std::list< CResource* > LDeployedRescsPTR;
	
	for ( Iterator = LDeployedResources.begin(); Iterator != LDeployedResources.end(); Iterator++ )	
		LDeployedRescsPTR.push_back( m_VResource[ *Iterator ] );

	return LDeployedRescsPTR;
}

// Create an ordered list of the dispatch location names that are associated with the fire's FWA ordered by the distance from the fire
// Don't really need this function can use GetOrderedDispLocsLevel( CFWA FWA, Level = -1 )
std::list< string > CRunScenario::GetOrderedDispLocs( CFWA FWA )
{
	/*std::list< CDispLoc > OrderedDispLocs;
	std::list< CDispLoc >::iterator It;

	// Get the FWAs index for the associations
	int FWAindex = FWA.GetIndex();

	// Iterate through the dispatch locations vector and place the dispatch location names in the order of their distance from the fire
	for ( int i = 0; i < m_VDispLoc.size(); i++ )	{

		// Is the dispatch location associated?
		// Get the distance to the dispatch location (air miles )
		double DLDistance = m_VDispLoc[i].GetFWAAssoc( FWAindex );

		if (  DLDistance > 0.0 )	{

			// Place the dispatch location's name in the appropriate position
			if ( OrderedDispLocs.size() == 0 )
				OrderedDispLocs.push_back( m_VDispLoc[i] ) ;
			
			else	{

				It = OrderedDispLocs.begin();
				bool found = false;

				while ( !found && It != OrderedDispLocs.end() )	{

					// Is the Dispatch Location in the List's distance greater than the incoming Dispatch Location's distance?
					if ( DLDistance <  ( *It ).GetFWAAssoc( FWAindex ))
						found = true;
					else
						It++;
				}

				OrderedDispLocs.insert( It, m_VDispLoc[i] );

			}

		}

	}

	// Create another list that contains the dispatch location Ids
	std::list< string > OrderedDispLocsID;

	for ( It = OrderedDispLocs.begin(); It != OrderedDispLocs.end(); It++ )
		OrderedDispLocsID.push_back( ( *It ).GetDispLocID() );*/


	list< string > OrderedDispLocsID;

	// Reworked on 9/9/2013 to reflect modified FWA / Dispatch Location Association data
	std::multimap< double, string > Associations = FWA.GetAssociationMap();

	// The Multimap contains the dispatch location names ordered by distance from the FWA travel time point
	std::multimap< double, string >::iterator It = Associations.begin();

	while ( It != Associations.end() )
		OrderedDispLocsID.push_back( ( *It ).second );

	return OrderedDispLocsID;

}

// Remove resource pointers from a list of resources pointers
void CRunScenario::RemoveDeployedResc( std::list< CResource* > &LDeployedResources, std::list< CResource* > Remove )
{
	std::list< CResource* >::iterator It;

	// Iterate through the list of resource pointers to be removed and remove from the deployed resource list
	for ( It = Remove.begin(); It != Remove.end(); It++ )
		LDeployedResources.remove( *It );

}

// Create an ordered list of the dispatch location names that are associated with the fire's FWA for a given level of the tree.
// Dispatch Location level = 3.  All dispatch locations in tree level = -1
std::list< string > CRunScenario::GetOrderedDispLocsbyLevel( CFWA FWA, int Level )
{	
	/*// Create lists for each level of the tree
	std::list< CDispLoc > OrderedDispLocs;						// List for the dispatch locations at the desired level of the tree
	std::list< CDispLoc >::iterator It;

	// Create a Dispatcher node pointer to contain returned values
	OmffrNode< CDispatchBase* > *Node;

	// Create a vector of tree nodes to contain returned values
	std::vector< OmffrNode< CDispatchBase* >* > VNodes;
	
	// Get the FWAs index for the associations
	int FWAindex = FWA.GetIndex();

	// Get all the external nodes (dispatch locations) for the tree
	Node = m_DispTree.Root();
	VNodes = m_DispTree.GetExternalNodes( Node, -1 );

	// Iterate through the nodes vector and place the dispatch location names in the order of their distance from the fire
		bool found = false;
		double DLDistance = 0;
		for ( int j = 0; j < VNodes.size(); j++ )	{

			found = false;

			CDLDispatcher* DLDisp = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( VNodes[j]) );
			if ( DLDisp != 0 )	{
				// Are there any resources at the dispatch location
				list< CResource* > Rescs = DLDisp->GetRescList();

				// Get the Distance to determine if associated
				DLDistance = DLDisp->GetDispLoc().GetFWAAssoc( FWAindex );

				// Get the level for the node
				int NodeLevel = m_DispTree.Depth( VNodes[j] );

				// Get the number of resources at the dispatch location
				int NumResc = Rescs.size();

				// If there are add the dispatch location to the list
				if ( NumResc > 0 && DLDistance > 0.0 && NodeLevel == Level )
					found = true;
				
				if ( found )	{
					// Place the dispatch location's name in the appropriate position
					if ( OrderedDispLocs.size() == 0 )	
						OrderedDispLocs.push_back( DLDisp->GetDispLoc() ) ;
			
					else	{
						It = OrderedDispLocs.begin();
						bool Ofound = false;

						while ( !Ofound && It != OrderedDispLocs.end() )	{

							// Is the Dispatch Location in the List's distance greater than the incoming Dispatch Location's distance?
							if ( DLDistance <  ( *It ).GetFWAAssoc( FWAindex ))
								Ofound = true;
							else
								It++;
						}

						OrderedDispLocs.insert( It, DLDisp->GetDispLoc() );

					}

				}

			}

		}

	// Create another list that contains the dispatch location Ids
	std::list< string > OrderedDispLocsID;

	for ( It = OrderedDispLocs.begin(); It != OrderedDispLocs.end(); It++ )
		OrderedDispLocsID.push_back( ( *It ).GetDispLocID() );*/

	// Reworked to reflect changes to how FWA / Dispatch Location Associations are handeled in the program

	list< string > OrderedDispLocsID;

	// Reworked on 9/9/2013 to reflect modified FWA / Dispatch Location Association data
	std::multimap< double, string > Associations = FWA.GetAssociationMap();

	// The Multimap contains the dispatch location names ordered by distance from the FWA travel time point
	std::multimap< double, string >::iterator It = Associations.begin();

	while ( It != Associations.end() )	{

		string DispLocID = ( *It ).second;

		std::map< std::string, OmffrNode< CDispatchBase* >* >::iterator ItNode = m_DispMap.find( DispLocID );
		int DLLevel = m_DispTree.Depth( ( *ItNode ).second );

		if ( DLLevel == Level )
			OrderedDispLocsID.push_back( ( *It ).second );

		It++;

	}

	return OrderedDispLocsID;
}

// Update the dispatchers levels following containment effort on a fire
bool CRunScenario::UpdateDispatcherLevels( CFire fire, int ResultsIndex, bool DispLogicFilled )
{
	// Update the Escape fire levels for the dispatchers
	if ( m_VResults[ m_VResults.size()-1 ].GetStatus() != "Contained" )	{

		// Get the FPU the fire was in
		CFWA FWA  = fire.GetFWA();
		string FPU = FWA.GetFPU();
		string GACC = FPU.substr(0, 2);

		// Get the Julian day the fire was on
		int Julian = fire.GetJulianDay();

		// Find the GACC in the Dispatcher tree
		OmffrNode< CDispatchBase* > *Node = m_DispTree.Root();
		OmffrNode< CDispatchBase* > *FPUNode;
		vector< OmffrNode< CDispatchBase* >* > VNodes = m_DispTree.Children( Node );
		
		int i = 0;
		string NodeID;
		bool Found = false;

		while ( i < VNodes.size() )	{
			// Get the id for the dispatcher
			NodeID = m_DispTree.Element( VNodes[i] )->GetDispatcherID();

			// Is the dispatcher the FPU
			if ( NodeID == GACC )	{
				// Save the Node because it's the one the fire is in
				Node = VNodes[i];
				i = VNodes.size();
				Found = true;
			}
		
			else
				i++;
		}

		if ( !Found )	{
			cout << " The GACC Node for the fire's FWA is not found in the tree \n";
			return false;
		}

		// Find the FPU node in the dispatcher tree
		VNodes = m_DispTree.Children( Node );
		
		i = 0;
		NodeID;
		Found = false;

		while ( i < VNodes.size() )	{
			// Get the id for the dispatcher
			NodeID = m_DispTree.Element( VNodes[i] )->GetDispatcherID() ;

			// Is the dispatcher the FPU
			if ( NodeID == FPU )	{
				// Save the Node because it's the one the fire is in
				FPUNode = VNodes[i];
				i = VNodes.size();
				Found = true;
			}
		
			else
				i++;
		}

		if ( !Found )	{
			cout << " The FPU Node for the fire's FWA is not found in the tree \n";
			return false;
		}


		// Increase the Escape Level in the fire's FPU dispatcher by 1
		m_DispTree.Element( FPUNode )->AddToEscapeLevel( Julian );

		// Get the GACC node and increase the Escape Level for the GACC dispatcher by 1
		Node = m_DispTree.Parent( FPUNode );
		m_DispTree.Element( Node )->AddToEscapeLevel( Julian );

		// Get the national node and increase the Escape Level for the National level by 1
		Node = m_DispTree.Parent( Node );
		m_DispTree.Element( Node )->AddToEscapeLevel( Julian );

		// Get all the Dispatch Location Dispatchers associated with the fire's FWA and increase their Escape Level by 1
		// Get a list of the disptach location dispatcher Ids that are associated with the fire's FWA
		list< string > LDLIds = GetOrderedDispLocsbyLevel( FWA, -1 );
		list< string >::iterator It;

		// Get a vector of all the dispatch location dispatcher nodes in the tree
		VNodes = m_DispTree.GetExternalNodes( Node, -1 );

		// Iterate through the nodes and if the dispatch location dispatcher is associated increase the Escape Level by 1
		for ( int i = 0; i < VNodes.size(); i++ )	{
			string DLDispatcherID = m_DispTree.Element( VNodes[i] )->GetDispatcherID();

			Found = false;

			for ( It = LDLIds.begin(); It!= LDLIds.end(); It++ )	{
				if ( *It == DLDispatcherID )	
					Found = true;
			}

			if ( Found )
				m_DispTree.Element( VNodes[i] )->AddToEscapeLevel( Julian );


		}
	}


	// Update the Dispatch Logic Unfilled levels for the dispatchers
	if ( !DispLogicFilled )	{

		// Get the FPU the fire was in
		CFWA FWA  = fire.GetFWA();
		string FPU = FWA.GetFPU();
		string GACC = FPU.substr(0, 2);

		// Get the Julian day the fire was on
		int Julian = fire.GetJulianDay();

		// Find the GACC in the Dispatcher tree
		OmffrNode< CDispatchBase* > *Node = m_DispTree.Root();
		OmffrNode< CDispatchBase* > *FPUNode;
		vector< OmffrNode< CDispatchBase* >* > VNodes = m_DispTree.Children( Node );
		
		int i = 0;
		string NodeID;
		bool Found = false;

		while ( i < VNodes.size() )	{
			// Get the id for the dispatcher
			NodeID = m_DispTree.Element( VNodes[i] )->GetDispatcherID();

			// Is the dispatcher the FPU
			if ( NodeID == GACC )	{
				// Save the Node because it's the one the fire is in
				Node = VNodes[i];
				i = VNodes.size();
				Found = true;
			}
		
			else
				i++;
		}

		if ( !Found )	{
			cout << " The GACC Node for the fire's FWA is not found in the tree \n";
			return false;
		}

		// Find the FPU node in the dispatcher tree
		VNodes = m_DispTree.Children( Node );
		
		i = 0;
		NodeID;
		Found = false;

		while ( i < VNodes.size() )	{
			// Get the id for the dispatcher
			NodeID = m_DispTree.Element( VNodes[i] )->GetDispatcherID();

			// Is the dispatcher the FPU
			if ( NodeID == FPU )	{
				// Save the Node because it's the one the fire is in
				FPUNode = VNodes[i];
				i = VNodes.size();
				Found = true;
			}
		
			else
				i++;
		}

		if ( !Found )	{
			cout << " The FPU Node for the fire's FWA is not found in the tree \n";
			return false;
		}

		// Increase the Dispatch Logic Unfilled Level in the fire's FPU dispatcher by 1
		m_DispTree.Element( FPUNode )->AddToDispLogicUnfilled( Julian );

		// Get the GACC node and increase the Dispatch Logic Unfilled Level for the GACC dispatcher by 1
		Node = m_DispTree.Parent( FPUNode );
		m_DispTree.Element( Node )->AddToDispLogicUnfilled( Julian );

		// Get the national node and increase the Dispatch Logic Unfilled Level for the National level by 1
		Node = m_DispTree.Parent( Node );
		m_DispTree.Element( Node )->AddToDispLogicUnfilled( Julian );

		// Get all the Dispatch Location Dispatchers associated with the fire's FWA and increase their Dispatch Logic Unfilled Level by 1
		// Get a list of the disptach location dispatcher Ids that are associated with the fire's FWA
		list< string > LDLIds = GetOrderedDispLocsbyLevel( FWA, -1 );
		list< string >::iterator It;

		// Get a vector of all the dispatch location dispatcher nodes in the tree
		VNodes = m_DispTree.GetExternalNodes( Node, -1 );

		// Iterate through the nodes and if the dispatch location dispatcher is associated increase the Escape Level by 1
		for ( int i = 0; i < VNodes.size(); i++ )	{
			string DLDispatcherID = m_DispTree.Element( VNodes[i] )->GetDispatcherID();

			Found = false;

			for ( It = LDLIds.begin(); It!= LDLIds.end(); It++ )	{
				if ( *It == DLDispatcherID )	
					Found = true;
			}

			if ( Found )
				m_DispTree.Element( VNodes[i] )->AddToDispLogicUnfilled( Julian );


		}
	}

	return true;
}

// Open Levels file and set header
void CRunScenario::OpenLevelsFile()
{
    /* Don't write this */
    return;
	ofstream outFile( "Levels.dat" , ios::out );

	// Exit program if unable to create file
	if ( !outFile ) { 
		cerr << "Output file could not be opened" << endl;
		//exit( 1 );
	}

	outFile << "Scenario, " << "Day, " << "Expected Level, " << "Previous Level, " << "Escape Level, " << "Dispatch Logic Unfilled\n";
	outFile.close();
}

// Get the internal nodes from Node down on the tree excluding Node
vector< OmffrNode< CDispatchBase* >* > CRunScenario::GetInternalNodes( OmffrNode< CDispatchBase* >* Node )
{
	// Get all the nodes past Node
	vector< OmffrNode< CDispatchBase* >* > AllNodes = m_DispTree.Postorder( Node );
	vector< OmffrNode< CDispatchBase* >* > InNodes;

	// If AllNodes is empty
	if ( AllNodes.empty() )
		return AllNodes;

	// Remove the external nodes from the AllNodes vector
	for ( int i = 0; i < AllNodes.size(); i++ )	{
		if ( m_DispTree.Internal( AllNodes[i] ) )
			InNodes.push_back( AllNodes[i] );
	}

	return InNodes;
}

// Order the tree nodes in a vector so that the ones furthest from the root are first
vector< OmffrNode< CDispatchBase* >* > CRunScenario::OrderNodesRootLast( vector< OmffrNode< CDispatchBase* >* > Nodes )
{
	// Return the original vector if there are 0 or 1 nodes in the vector
	if ( Nodes.size() < 2 )
		return Nodes;
	
	// Use a list to order the nodes
	list< OmffrNode< CDispatchBase* >* > LNodes;
	list< OmffrNode< CDispatchBase* >* >::iterator It;

	LNodes.push_back( Nodes[0] );

	for ( int i = 1; i < Nodes.size(); i++ )	{
		
		// Get the level for the node being placed in the list  (root depth = 0, Gacc depth = 1, FPU depth = 2 )
		int Level = m_DispTree.Depth( Nodes[i] );

		// Place the node in the list ordered by the depth in decreasing order
		
		It = LNodes.begin();
		bool greater = false;
		while ( !greater && It!= LNodes.end() )	{
					
			int CompLevel = m_DispTree.Depth( *It );
			if ( Level > CompLevel )
				greater = true;
			else
				It++;
		}

		// insert the Node before the list node pointed to by the iterator, or at the end
		LNodes.insert( It, Nodes[i] );
	}

	// Convert the list to a vector
	vector< OmffrNode< CDispatchBase* >* > OrderedNodes;

	for ( It = LNodes.begin(); It != LNodes.end(); It++ )
		OrderedNodes.push_back( *It );

	// Return the ordered vector
	return OrderedNodes;
}

// Determine the Expected fire level for each dispatcher in the tree
void CRunScenario::DetermineExpectedLevel()
{
	// Get a vector of the internal nodes in the tree ordered so the FPU nodes are first
		vector< OmffrNode< CDispatchBase* >* > InNodes = GetInternalNodes( m_DispTree.Root() );
		InNodes = OrderNodesRootLast( InNodes );
	
	// Iterate through the fires in the scenario and increment the count for the Dispatch Location dispatchers and FPU nodes
	for ( int i = 0; i < m_VFire.size(); i++ )	{
		// Get the FWA for the fire
		CFWA FWA = m_VFire[i].GetFWA();

		// Get the FPU dispatcher id for the fire
		string FPUId = FWA.GetFPU();

		// Find the FPU node in the tree
		OmffrNode< CDispatchBase* >* TheNode = InNodes[0];
		bool Found = false;
		int j = 0;

		while ( !Found && j < InNodes.size() )	{
			// Is node j the FPU's node
			string NodeId = m_DispTree.Element( InNodes[j] )->GetDispatcherID();

			if ( NodeId == FPUId )	{
				Found = true;
				TheNode = InNodes[j];
			}
                        j++;
		}

		// Increment the Expected Level for the day for the FPU Node
		int Julian = m_VFire[i].GetJulianDay();

		m_DispTree.Element( TheNode )->AddtoExpectLevel( Julian );

		// Get the GACC Node and increment the Expected Level
		TheNode = m_DispTree.Parent( TheNode );
		m_DispTree.Element( TheNode) ->AddtoExpectLevel( Julian );

		// Get the National Node and increment the Expected Leve
		TheNode = m_DispTree.Parent( TheNode );
		m_DispTree.Element( TheNode) ->AddtoExpectLevel( Julian );


		// Get a vector of Dispatch Location Dispatcher Ids for the associated Dispatch Location Dispatchers
		list< string > DLDispatcherIds = GetOrderedDispLocs( FWA );
		list< string >::iterator It;

		// Get a node pointer for each Dispatch Location Dispatcher and increment the Expected Value
		for ( list< string >::iterator It = DLDispatcherIds.begin(); It != DLDispatcherIds.end(); It++ )	{
			TheNode = m_DispMap.find( *It )->second;
			m_DispTree.Element( TheNode) ->AddtoExpectLevel( Julian );
		}
			
	}

	// Read in the airtanker base expected levels from a file
	AirtankerBasesExpectedLevels();
}

// Read in the Expected values for the Airtanker bases
// Data file contains the airtanker base name followed by 365 double values between 0 and 1
void CRunScenario::AirtankerBasesExpectedLevels()	
{
	OmffrNode< CDispatchBase* > *Node = m_DispTree.Root();
	CNatDispatcher *NatDisp = dynamic_cast< CNatDispatcher* >( m_DispTree.Element( Node ) );

	vector< CDLDispatcher* > ATBases = NatDisp->GetAtDispatchersVector();
	
	// Open Base Expected Values file for the scenario
	ifstream inFile( "ATBaseExpectLevel.dat", ios::in );

	//exit program if ifstream could not open file
	if ( !inFile ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}

	string Base = "";
	double ExpectedValue[365];

	while ( inFile >> Base  )	{

		for ( int i = 0; i < 365; i++ )
			inFile >> ExpectedValue[i];
		
		// Find the pointer to the airtanker base's dispatch location dispatcher
		vector< CDLDispatcher* >::iterator It = ATBases.begin();
		bool Found = false;

		while ( !Found && It != ATBases.end() )	{

			string CompBaseID = ( *It )->GetDispLoc().GetDispLocID();

			if ( CompBaseID == Base )	
				Found = true;

			else
				It++;

		}

		if ( !Found )
			cout << "Did not find the Airtanker base while trying to save the expected values from the file \n";

		else	{
			
			// Save the values in the from the file into the expected levels for the base
			for ( int i = 0; i < 365; i++ )	
				( *It )->SetExpectLevel( ExpectedValue[i], i + 1);
		}
	}

	inFile.close();

}

// Get a vector of the Dispatch Locations associated with an FWA 
vector< OmffrNode< CDispatchBase* >* >CRunScenario::GetAssociatedDispLocs( CFWA FWA )
{
	vector< OmffrNode< CDispatchBase* >* > AssDLDispatchers; 
	vector< OmffrNode< CDispatchBase* >* > ExtNodes = m_DispTree.GetExternalNodes( m_DispTree.Root(), -1 );

	// Iterate through the dispatch locations dispatcher vector and place the dispatch location names in the order of their distance from the fire
	for ( int i = 0; i < ExtNodes.size(); i++ )	{

		// Is the dispatch location associated?
		// Get the distance to the dispatch location (air miles )
		CDLDispatcher* DLExtNode = dynamic_cast< CDLDispatcher* >(m_DispTree.Element( ExtNodes[i] ));

		if ( DLExtNode != 0 )	{

			string DispLocID = DLExtNode->GetDispLoc().GetDispLocID();
			double DLDistance = FWA.GetDistance( DispLocID );

			if (  DLDistance > 0.0 )	
				AssDLDispatchers.push_back( ExtNodes[i] );
			
		}

	}

	return AssDLDispatchers;

}

// Get a vector of the Dispatch Locations associated with an FWA that have resource at them for dispatching resources
vector< OmffrNode< CDispatchBase* >* >CRunScenario::GetAssociatedDispLocsWithResc( CFWA FWA )
{
	vector< OmffrNode< CDispatchBase* >* > AssDLDispatchers; 
	vector< OmffrNode< CDispatchBase* >* > ExtNodes = m_DispTree.GetExternalNodes( m_DispTree.Root(), -1 );

	// Iterate through the dispatch locations dispatcher vector and place the dispatch location names in the order of their distance from the fire
	for ( int i = 0; i < ExtNodes.size(); i++ )	{

		// Is the dispatch location associated?
		// Get the distance to the dispatch location (air miles )
		CDLDispatcher* DLExtNode = dynamic_cast< CDLDispatcher* >(m_DispTree.Element( ExtNodes[i] ));

		if ( DLExtNode != 0 )	{
	
			string DispLocID = DLExtNode->GetDispLoc().GetDispLocID();
			double DLDistance = FWA.GetDistance( DispLocID );

			if (  DLDistance > 0.0 )	{

				// Are there resources at the dispatch location, if so add to vector
				list< CResource* > Rescs = DLExtNode->GetRescList();

				if ( Rescs.size() > 0 )
					AssDLDispatchers.push_back( ExtNodes[i] );
			}
			
		}

	}

	return AssDLDispatchers;

}
		
// Get the FWAs that are associated with a dispatch location dispatcher
vector< CFWA* >CRunScenario::GetAssociatedFWAs( CDLDispatcher* DLDispatcher )
{
	// Get the Dispatch Location that is associated with the DL dispatcher
	CDispLoc DispLoc = DLDispatcher->GetDispLoc();

	// Get the list of the FWA pointers that are associated with the DL Dispatcher
	vector< CFWA* > AssFWAs = DispLoc.GetAssocFWAs();
	
	return AssFWAs;
}

// Process the last day of the scenario to determine use previous use level for the last fire day of the scenario
void CRunScenario::LastScenarioDay()
{
	// Get the Julian date for the last fire in the scenario
	int JulianPast = m_VFire[m_VFire.size()-1].GetJulianDay();

	// Reset the Daily minimum resource levels for each dispatcher
	vector< OmffrNode< CDispatchBase* >* > VNodes = m_DispTree.Preorder( m_DispTree.Root() );
	for ( int i = 0; i < VNodes.size(); i++ )
		m_DispTree.Element(VNodes[i])->ResetNewDay( JulianPast );
}

// Save Dispatch Logic File To DispLogicOut.dat
void CRunScenario::SaveDispLogicFile( )
{
	//if constructor opens a file
	ofstream OutDispLogicFile( "DispLogicOut.dat", ios::out );

	//exit program if ifstream could not open file
	if ( !OutDispLogicFile ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}

	// Iterate throught the dispatch logic and save the data
	for ( int i = 0; i < m_VDispLogic.size(); i++ )	{
		string displogicid = m_VDispLogic[i].GetLogicID();
		string index = m_VDispLogic[i].GetIndex();
		int nlevels = m_VDispLogic[i].GetNumLevels();
		int breakpts[5] = { 0, 0, 0 , 0, 0 };
		int numresc [13][5]= { 0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0, 
					       0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0, 
						   0, 0, 0, 0, 0,
						   0, 0, 0, 0, 0 };
		for ( int j = 0; j < 5; j++ )	{
			breakpts[j] = m_VDispLogic[i].GetBreakPoint(j);
			for ( int k = 0; k < 13; k ++ )
				numresc[k][j] = m_VDispLogic[i].GetRescNum(k,j);
		}

		
		// save each disptach logic entry to the file
		OutDispLogicFile << displogicid << " " << index << " "<< nlevels << " "; 
		for ( int i = 0; i < 5; i++ )
			OutDispLogicFile << breakpts[i] << " ";
		for ( int i = 0; i < 13; i++ )	{
			for ( int j = 0; j < 5; j++ )
				OutDispLogicFile << numresc[i][j] << " ";
		}

		OutDispLogicFile<< "\n";
	}

	OutDispLogicFile.close();
}

// Save FWA data to file FWAOut.dat
void CRunScenario::SaveFWAFile( )
{
	//if constructor opens a file
	ofstream OutFWAFile( "FWAOut.dat", ios::out  );

	//exit program if ifstream could not open file
	if ( !OutFWAFile ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}
	// iterate through the FWAs and save the data to a file
	for ( int i = 0; i < m_VFWA.size(); i++ )	{
		string FWA = m_VFWA[i].GetFWAID();
		string FMG = m_VFWA[i].GetFMG();
		int WalkInPct = m_VFWA[i].GetWalkInPct();
		int PumpNRoll = m_VFWA[i].GetPumpnRoll();
		int Head = m_VFWA[i].GetHead();
		int Tail = m_VFWA[i].GetTail();
		int Parallel = m_VFWA[i].GetParallel();
		int AttDist = m_VFWA[i].GetAttackDist();
		bool WaterDrops = m_VFWA[i].GetWaterDrops();
		bool Excluded = m_VFWA[i].GetExcluded();
		double DiscSize = m_VFWA[i].GetDiscoverySize();
		int ESLTime = m_VFWA[i].GetESLTime();
		int ESLSize = m_VFWA[i].GetESLSize();
		double AirtoGround = m_VFWA[i].GetAirtoGround();
		int WalkInDelay[6] = { 0, 0, 0, 0, 0, 0 };
		int PostContUsed[6] = { 0, 0, 0, 0, 0, 0 };
		int PostContUnused[6] = { 0, 0, 0, 0, 0, 0, };
		int PostEscape[6] = { 0, 0, 0, 0, 0, 0 };
		int ReloadDelay[5] = { 0, 0, 0, 0, 0 };
		for ( int j = 0; j < 6; j++ )	{
			WalkInDelay[j] = m_VFWA[i].GetWalkInDelay( j );
			PostContUsed[j] = m_VFWA[i].GetPostContUsedDelay( j );
			PostContUnused[j] = m_VFWA[i].GetPostContUnusedDelay(j);
			PostEscape[j] = m_VFWA[i].GetEscapeDelay( j );
			if ( j < 5 )
				ReloadDelay[j] = m_VFWA[i].GetReloadDelay( j );
		}

		int FirstUnitDelay = m_VFWA[i].GetFirstUnitDelay();
		double Diurnal[24] = {0.10, 0.10, 0.10, 0.10, 0.10, 0.10,
							  0.10, 0.10, 0.10, 0.10, 0.20, 0.40, 
							  0.70, 1.00, 1.00, 1.00, 1.00, 1.00, 
							 1.00, 1.00, 0.70, 0.50, 0.30, 0.20 };
		for ( int j = 0; j < 24; j++ )	
			Diurnal[j] = m_VFWA[i].GetDiurnalCoefficient( j );

		string ROSAdjFuel[10] = { "x", "x", "x", "x", "x", "x", "x", "x", "x", "x" };
		double ROSAdjustment[10] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
		for ( int j = 0; j < 10; j ++ )	{
			ROSAdjFuel[j] = m_VFWA[i].GetROSFuelAdj( j );
			ROSAdjustment[j] = m_VFWA[i].GetAdjustment( j );
		}

		int Index = m_VFWA[i].GetIndex();
		string DispLogicId = m_VFWA[i].GetDispLogic().GetLogicID();
		double Latitude = m_VFWA[i].GetLatitude();
		double Longitude = m_VFWA[i].GetLongitude();
		string FPU = m_VFWA[i].GetFPU();


		// Save each FWA entry to the file
		OutFWAFile << FWA << " " << FMG << " " << WalkInPct << " " << PumpNRoll << " " << Head << " " << Tail << " " << Parallel << " " <<
			AttDist << " " << WaterDrops << " " << Excluded << " " << DiscSize << " " << ESLTime << " " << ESLSize << " " << AirtoGround << " "; 
		for ( int j = 0; j < 6; j++ )
			OutFWAFile << WalkInDelay[j] << " ";
		for ( int j = 0; j < 6; j++ )
			OutFWAFile << PostContUsed[j] << " ";
		for ( int j = 0; j < 6; j++ )
			OutFWAFile << PostContUnused[j] << " ";
		for ( int j = 0; j < 6; j++ )
			OutFWAFile << PostEscape[j] << " ";
		for ( int j = 0; j < 5; j++ )
			OutFWAFile << ReloadDelay[j] << " ";
			OutFWAFile << FirstUnitDelay << " ";
		for ( int j = 0; j < 24; j++ )
			OutFWAFile << Diurnal[j] << " ";
		for ( int j = 0; j < 10; j++ )
			OutFWAFile << ROSAdjFuel[j] << " ";
		for ( int j = 0; j < 10; j++ )
			OutFWAFile << ROSAdjustment[j] << " ";
		OutFWAFile << Index << " " << DispLogicId << " " << Latitude << " " << Longitude << " " << FPU << "\n";
	
	}

	OutFWAFile.close();
}

// Save Dispatch Location information in file DispLocOut.dat
void CRunScenario::SaveDispLocFile()
{
	//if constructor opens a file
	ofstream OutDispLocFile( "DispLocOut.dat" , ios::out );

	//exit program if ifstream could not open file
	if ( !OutDispLocFile ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}

	// Save Dispatch Location information
	for ( int i = 0; i < m_VDispLoc.size(); i++ )	{
		string DispLocID = m_VDispLoc[i].GetDispLocID();
		double Latitude = m_VDispLoc[i].GetLatitude();
		double Longitude = m_VDispLoc[i].GetLongitude();
		int CallbackDelay = m_VDispLoc[i].GetCallbackDelay();
		string FPU = m_VDispLoc[i].GetFPU();
	
	// Save each Dispatch Location entry in file
	 OutDispLocFile << DispLocID << " " << Latitude << " " << Longitude << " " << CallbackDelay << " " ;
		OutDispLocFile <<  FPU << "\n";
	
	}

	OutDispLocFile.close();
}

// Save Resource data from file ResourceOut.dat
void CRunScenario::SaveResourceFile( )
{
	//if constructor opens a file
	ofstream OutResourceFile( "ResourceOut.dat", ios::out );

	//exit program if ifstream could not open file
	if ( !OutResourceFile ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}
	// Enter elements for FWA
	for ( int i = 0; i < m_VResource.size(); i++ )	{
		string RescID = m_VResource[i]->GetRescID();
		string RescType = m_VResource[i]->GetRescType().GetRescType();
		int Staffing =m_VResource[i]->GetStaffing();
		string StartTime = m_VResource[i]->GetStartTime();
		string EndTime = m_VResource[i]->GetEndTime();
		string StartDay = m_VResource[i]->GetStartDayofWeek();
		string EndDay = m_VResource[i]->GetEndDayofWeek();
		int StartSeason = m_VResource[i]->GetStartSeason();
		int EndSeason = m_VResource[i]->GetEndSeason();
		string DispLoc = m_VResource[i]->GetDispLoc().GetDispLocID();
		int PctAvailable = m_VResource[i]->GetPctAvail();
		double DailyCost = m_VResource[i]->GetDailyCost();
		double HourlyCost = m_VResource[i]->GetHourlyCost();
		int Volume = 0;
		CEngine* ENG = dynamic_cast< CEngine* >( m_VResource[i] );
		if ( ENG != 0 )
			Volume = ENG->GetVolume();
		CAerial* Aerial = dynamic_cast< CAerial* >( m_VResource[i] );
		if ( Aerial != 0 )
			Volume = Aerial->GetVolume();
		int Severity = 0;
		string Version = ( "a");
		double TotalFTEs = 0.0;
		int TotalPositions = 0;
		int AnnualCost = 0;
		int VehicleCapCost = 0;

		// Save each Resource entry in file
		OutResourceFile << RescID << " " << RescType << " " << Staffing << " " << StartTime << " " << EndTime << " " << StartDay << " " <<
			EndDay << " " << StartSeason << " " << EndSeason << " " << DispLoc << " " << PctAvailable << " " << DailyCost << " " <<
			HourlyCost << " " << Volume << " " << Severity << " " << Version << " " << TotalFTEs << " " << TotalPositions << " " <<
			AnnualCost << " " << VehicleCapCost << "\n";
				
	}	

	OutResourceFile.close();
}

// Save Fire data in file ScenarioxOut.dat
void CRunScenario::SaveFireFile( int Scenario )
{
	char FireFile[512];
	sprintf( FireFile, "Scenario%dOut.dat", Scenario + 1 );
	ofstream  OutFireFile( FireFile, ios::out );

	//exit program if ifstream could not open file
	if ( !OutFireFile ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}
	// Save Fire information to file 
	for ( int i = 0; i < m_VFire.size(); i++ )	{
		int ScenarioNum = m_VFire[i].GetScenario();
		int FireNum = m_VFire[i].GetFireNumber();
		int JulianDay = m_VFire[i].GetJulianDay();
		string DayofWeek = m_VFire[i].GetDayofWeek();
		string DiscoveryTime = m_VFire[i].GetDiscoveryTime();
		int BI = m_VFire[i].GetBI();
		double ROS = m_VFire[i].GetROS();
		int FuelModel = m_VFire[i].GetFuelModel();
		string SpecificCondition = m_VFire[i].GetSpecificCondition();
		int SlopePct = m_VFire[i].GetSlopePct();
		bool IsWalkIn = m_VFire[i].GetIsWalkIn();
		string Tactic = m_VFire[i].GetTactic();
		double Distance = m_VFire[i].GetDistance();
		int Elevation = m_VFire[i].GetElevation();
		double LtoWRatio = m_VFire[i].GetLtoWRatio();
		int MinNumSteps = m_VFire[i].GetMinNumSteps();
		int MaxNumSteps = m_VFire[i].GetMaxNumSteps();
		string Sunrise = m_VFire[i].GetSunRise();
		string Sunset = m_VFire[i].GetSunSet();
		bool WaterDrops = m_VFire[i].GetWaterDropsAllowed();
		bool IsPumpNRoll = m_VFire[i].GetIsPumpNRoll();
		string FWAId = m_VFire[i].GetFWA().GetFWAID();
		double Latitude = 0.0;
		double Longitude = 0.0;
	

	// Read in each FWA entry from file
	OutFireFile << ScenarioNum << " " << FireNum << " " << JulianDay << " " << DayofWeek << " " << DiscoveryTime << " " << BI << " " <<
		ROS << " " << FuelModel << " " << SpecificCondition << " " << SlopePct << " " << IsWalkIn << " " << Tactic << " " << Distance << " " <<
		Elevation << " " << LtoWRatio << " " << MinNumSteps << " " << MaxNumSteps << " " << Sunrise << " " << Sunset << " " << WaterDrops
		<< " " << IsPumpNRoll << " " << FWAId << " " << Latitude << " " << Longitude << "\n";
		
	}

	OutFireFile.close();
}

// Save the files needed to redo this run
void CRunScenario::SaveFiles( )
{	
	SaveDispLogicFile();
	SaveFWAFile();
	SaveDispLocFile();
	SaveResourceFile();
}

// Apply Type II IA Crew Draw Down to the GACC Nodes
int CRunScenario::TypeIICrewDrawDown( int fire, int CDDEscapes, int CDDDuration, int CDDLimit )
{
	// Get the GACC Nodes
	// Get the Internal Nodes in the tree
	vector< OmffrNode< CDispatchBase* >* > InNodes = GetInternalNodes( m_DispTree.Root() );

	// Get the GACC Nodes
	vector< OmffrNode< CDispatchBase* >* > GACCNodes;

	for ( int i = 0; i < InNodes.size(); i++ )	{
		if ( InNodes[i]->parent == m_DispTree.Root() )
			GACCNodes.push_back( InNodes[i] );
	}
	
	// Get the previous fire day
	int JulianPast = m_VFire[fire-1].GetJulianDay();
	int CrewsApplied = 0;

	// Iterate through the GACC nodes
	for ( int i = 0; i < GACCNodes.size(); i++ )	{
	
		// Were there any escapes at the GACC?
		int EscapeLevel = m_DispTree.Element( GACCNodes[i] )->GetEscapeLevel( JulianPast );

		// If the escape level is greater than the Escape Level trigger then draw down the Type II IA Crews for the GACC
		while  ( EscapeLevel > CDDEscapes )	{

			// Get a pointer to the GACC dispatcher
			CGACCDispatcher* GACC = dynamic_cast< CGACCDispatcher* >( m_DispTree.Element( GACCNodes[i] ) );

			if ( GACC != 0 )	{

				// Get the number of Type II IA Crews at the GACC
				int NumberCrews = GACC->NumberTypeTwoCrews();

				// If there are some Type II Crews
				if ( NumberCrews > 0 )	{

					// Determine which crew will be available first
					int TheCrewIndex = GACC->FirstTypeTwoCrewAvailable();

					// Get the time when the entire crew is next available
					int AvailableTime = GACC->CrewAvailableTime( TheCrewIndex );

					// Determine what time to set the Next available time to
					// What is the maximum time limit - Midnight for the previous fire day plus the limit for the draw down time
					int Limit = JulianPast * 1440 + CDDLimit;

					// If the next available time is past the limit then the draw is not applied to any more crews
					if ( AvailableTime > Limit )
						EscapeLevel = 0;

					else	{		// Apply the draw down to the crew

						// Time the crew would be next available
						int Time = 0;
						if ( AvailableTime > JulianPast * 1440 )
							Time = AvailableTime + CDDDuration;
						else
							Time = JulianPast * 1440 + CDDDuration;

						// If the duration would take the crew beyond the limit use the limit time instead
						if ( Limit < Time )
							Time = Limit;

						// Change the next avialable time for the Type II IA Crew and it's parts
						GACC->SetTypeTwoCrewNextAvailableTime( TheCrewIndex, Time );
		
						// Increment the number of crews the draw down was applied to
						CrewsApplied++;

						// Subtract the escape fires that have been accounted for
						EscapeLevel = EscapeLevel - CDDEscapes;

					}		// End of apply drawdown

				}  

				else EscapeLevel = 0;							// There were no Type II IA Crews so can't apply draw down

			}		// End of if have a good pointer for the for the CGACCDispatcher

			else	 {							// Do not have a good pointer so can't apply draw down

				EscapeLevel = 0;
				cout << "Did not get a good pointer to the GACC Dispatcher so can't apply draw down to the Type II IA Crews \n";
			}

		}		// Now try again if need be

	}

	return CrewsApplied;
			
}

// Save the Resource work year files for the IA Crews of the Type II IA Crew
void CRunScenario::SaveTypeTwoIARescWorkYear()
{
	// Get the GACC Nodes
	// Get the Internal Nodes in the tree
	vector< OmffrNode< CDispatchBase* >* > InNodes = GetInternalNodes( m_DispTree.Root() );

	// Get the GACC Nodes
	vector< OmffrNode< CDispatchBase* >* > GACCNodes;

	for ( int i = 0; i < InNodes.size(); i++ )	{
		if ( InNodes[i]->parent == m_DispTree.Root() )
			GACCNodes.push_back( InNodes[i] );
	}

	// Open the output file
	ofstream  OutCrews( "TypeTwoIACrews.dat", ios::out );

	//exit program if ifstream could not open file
	if ( !OutCrews ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}
	
	// Iterate through the GACC nodes and print the resource work year entries for the IA crews of the Type II IA Crews for the GACC
	for ( int i = 0; i < GACCNodes.size(); i++ )	{

		// Get the vector of Type II IA Crews
		CGACCDispatcher* GACC = dynamic_cast< CGACCDispatcher* >( m_DispTree.Element( GACCNodes[i] ) );

		if ( GACC != 0 )	{

			OutCrews << "GACC: " << GACC->GetDispatcherID() << "\n";

			vector< CTypeTwoIACrew > Crews(GACC->GetTypeTwoCrewVector());

			// Iterate through the Type II IA Crews at the GACC
			for ( int j = 0; j < Crews.size(); j++ )	{

				// Get pointers to the IACrews and print their resource work year information
				vector< CResource* > IACrews = Crews[j].GetIACrewsVector();

				for ( int k = 0; k < IACrews.size(); k++ )	{

					// Print the Type Two IA Crew ID and value of k
					OutCrews << "Crew ID: " << IACrews[k]->GetRescID() << " IA Crew: " << k << "\n";

					// Resource work year for the IA Crew
					OutCrews << "Resources Work for the year \n";
					for ( int l = 0; l < IACrews[k]->WorkYearSize(); l++ )	{
							CRescWorkYear workyearptr = IACrews[k]->GetWorkYearEntry(l);
							OutCrews << "Entry Number: " << l << " Julian Day: " << workyearptr.GetJulianDay() << " Start Time: " 
								<< workyearptr.GetStartTime() << " End Time: " << workyearptr.GetEndTime() << " Effort: " 
								<< workyearptr.GetEffort() << " Met Work/Rest Flag: " << workyearptr.GetFlag() << "\n";
					 }
					 OutCrews << "\n";

				}
			}
		}

	}

	OutCrews.close();

}

// Save the movement vectors for the airtankers
void CRunScenario::SaveMovementVectors()
{
	// Open the file
	ofstream  Out( "Movement.dat", ios::out );

	//exit program if ifstream could not open file
	if ( !Out ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}
	
	// Iterate through all the resources and find the airtankers
	for ( int i = 0; i < m_VResource.size(); i++ )	{

		// Is the resource an airtanker
		if ( m_VResource[i]->GetRescType().GetRescType() == "ATT" )	{

			vector< std::pair< int, string > > Movement = m_VResource[i]->GetMovement();

			Out << m_VResource[i]->GetRescID() << "\n";

			for ( int j = 0; j < Movement.size(); j++ )	{

				// Get the pair
				std::pair< int, string > pair = Movement[j];

				// Output the pairs
				Out << pair.first << " " << pair.second << "\n";

			}

		}

	}

	// Iterate through all the resources and find the SmokeJumpers
	for ( int i = 0; i < m_VResource.size(); i++ )	{

		// Is the resource an Smokejumpers or Smokejumper aircraft
		if ( m_VResource[i]->GetRescType().GetRescType() == "SMJR" || m_VResource[i]->GetRescType().GetRescType() == "SJAC" )	{

			vector< std::pair< int, string > > Movement = m_VResource[i]->GetMovement();

			Out << m_VResource[i]->GetRescID() << "\n";

			for ( int j = 0; j < Movement.size(); j++ )	{

				// Get the pair
				std::pair< int, string > pair = Movement[j];

				// Output the pairs
				Out << pair.first << " " << pair.second << "\n";

			}

		}

	}

	// Iterate through all the resources and find the Type II IA Crews
	for ( int i = 0; i < m_VResource.size(); i++ )	{

		// Is the resource an 209 person crews
		if ( m_VResource[i]->GetRescType().GetRescType() == "CRW"  && m_VResource[i]->GetStaffing() == 20 )	{

			vector< std::pair< int, string > > Movement = m_VResource[i]->GetMovement();

			Out << m_VResource[i]->GetRescID() << "\n";

			for ( int j = 0; j < Movement.size(); j++ )	{

				// Get the pair
				std::pair< int, string > pair = Movement[j];

				// Output the pairs
				Out << pair.first << " " << pair.second << "\n";

			}

		}

	}

	// Iterate through all the resources and find the helicopters
	for ( int i = 0; i < m_VResource.size(); i++ )	{

		// Is the resource an helicopters
		if ( m_VResource[i]->GetRescType().GetRescType() == "Helicopter" )	{

			vector< std::pair< int, string > > Movement = m_VResource[i]->GetMovement();

			Out << m_VResource[i]->GetRescID() << "\n";

			for ( int j = 0; j < Movement.size(); j++ )	{

				// Get the pair
				std::pair< int, string > pair = Movement[j];

				// Output the pairs
				Out << pair.first << " " << pair.second << "\n";

			}

		}

	}


	Out.close();

}

// Preposition Large airtankers at the beginning of each day
int CRunScenario::PrepositionResourceIA( string RescType, double MoveLevel, bool OutSeason, int Julian )
{
	int NumResc = 0;				// Record the number of resources moved

	// Move Level set to not move this resource type
	if ( MoveLevel == 0.0 )			
		return 0;

	// Level of use that curtails resource movement out of a dispatch location and number of days to check
	double UseLevel = 0.5;		// Just a guess
	int NumUseDays = 2;			// Another guess

	// Time Limit for determining if the resource will be available soon enough to preposition
	int TimeLimit = 1440;

	// Get a vector of all the Dispatch Location node pointers for the resource type
	vector< string > DispLocs;
	string DispType = "";
	bool IsRegionalCrew = false;
	bool IsHelitackCrew = false;
	int RescTypeIdx = -1;
	int TravelTime = 0;
	int ReturnTime = 12 * 1440;

	if ( RescType == "RegionalCrew" )	{

		DispLocs = m_RegionalCrewDLs;
		DispType = "CRW";
		IsRegionalCrew = true;
		RescTypeIdx = 1;
		TravelTime = 2880;
		
	}

	if ( RescType == "RegionalHelicopter" )	{

		DispLocs = m_RegionalHelicopterDLs;
		DispType = "Helicopter";
		RescTypeIdx = 6;
		TimeLimit = 600;
		TravelTime = 1440;
		ReturnTime = 5 * 1440;

	}

	if ( RescType == "Smokejumper" )	{

		DispLocs = m_SmokejumperDLs;
		DispType = "SMJR";
		RescTypeIdx = 10;
		TravelTime = 1440;

	}

	if ( RescType == "Airtanker" )	{

		DispLocs = m_AirtankerDLs;
		DispType = "ATT";
		RescTypeIdx = 0;
		TimeLimit = 600;


	}

	if ( RescType == "FSLocalCRW" )	{

		DispLocs = m_FSCRWDLs;
		DispType = "CRW";
		RescTypeIdx = 1;
		TimeLimit = 1;
		TravelTime = 2880;

	}

	if ( RescType == "DOILocalCRW" )	{

		DispLocs = m_DOICRWDLs;
		DispType = "CRW";
		RescTypeIdx = 1;
		TimeLimit = 1;
		TravelTime = 2880;

	}

	if ( RescType == "FSLocalENG" )	{

		DispLocs = m_FSENGDLs;
		DispType = "ENG";
		RescTypeIdx = 3;
		TimeLimit = 1;
		TravelTime = 2880;

	}

	if ( RescType == "DOILocalENG" )	{

		DispLocs = m_DOIENGDLs;
		DispType = "ENG";
		RescTypeIdx = 3;
		TimeLimit = 1;
		TravelTime = 2880;

	}

	if ( RescType == "FSLocalHELI" )	{

		DispLocs = m_FSHELIDLs;
		DispType = "Helicopter";
		IsHelitackCrew = true;
		RescTypeIdx = 6;
		TimeLimit = 1;
		TravelTime = 1440;

	}

	if ( RescType == "DOILocalHELI" )	{

		DispLocs = m_DOIHELIDLs;
		DispType = "Helicopter";
		IsHelitackCrew = true;
		RescTypeIdx = 6;
		TimeLimit = 1;
		TravelTime = 1440;

	}

	if ( DispLocs.size() == 0 )
		return 0;
	
	// Get the node pointers for the dispatch locations where the resources are located or can be located
	vector< OmffrNode< CDispatchBase* >* > DLDispPtrs;
	OmffrNode< CDispatchBase* > *Node;
	map< string, OmffrNode< CDispatchBase* >* >::iterator It;

	for ( int i = 0; i < DispLocs.size(); i++ )	{

		It = m_DispMap.find( DispLocs[i] );

		if ( It != m_DispMap.end() )	{

			Node = It->second;
			DLDispPtrs.push_back( Node );

		}

	}

	// Iterate through the Dispatch Locations and determine which will need resources, order by need
	list< CDLDispatcher* > OrderedDLs;
	// Make an list of resource pointers ordered by dispatch location's expected level ( low to high )
	std::pair< CResource*, CDLDispatcher* > RescPair;
	list< std::pair< CResource*, CDLDispatcher* > > OrderedRescs;

	for ( int i = 0; i < DLDispPtrs.size(); i++ )	{

		CDLDispatcher *DLDisp = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( DLDispPtrs[i] ) );

		if ( DLDisp != 0 )	{
			
			double DLExpectLevel = m_DispTree.Element( DLDispPtrs[i] )->GetExpectLevel( Julian );

			if ( DLExpectLevel >= MoveLevel )	{

				// Are there already extra resources at the dispatch location?
				int BaseLevel = m_DispTree.Element( DLDispPtrs[i] )->GetBaseRescLevelIdx( RescTypeIdx );
				int CurNumResc = DLDisp->NumRescIdx( RescTypeIdx );		
			
				// If ther are not extra resources already at the dispatch location
				if ( CurNumResc <= BaseLevel )	{

					// Place pointer in ordered list
					if ( OrderedDLs.size() == 0 )
						OrderedDLs.push_back( DLDisp );

					else	{

						// Iterate through the pointers in the ordered list and determine where the new value belongs
						list< CDLDispatcher* >::iterator It = OrderedDLs.begin(); 
						bool Found = false;

						while ( It != OrderedDLs.end() && !Found )	{

							// Get the Expected Level for the comparison dispatch location
							double CompExpectLevel = ( *It )->GetExpectLevel( Julian );

							if ( CompExpectLevel < DLExpectLevel )	
								Found = true;

							else 
								It++;
						}

						// Insert the base before the Iterator
						OrderedDLs.insert( It, DLDisp );

					}	// End else need to find ordered location where the dispatch location belongs

				}

			}		// End if dispatch location needs to borrow resources

			// Is the dispatch location's expected level low enough that it can lend resources
			if ( DLExpectLevel < 1 - MoveLevel )	{

				// Are there any resources of the appropriate resource type at the dispatch location?
				list< CResource* > RescAtDL =  DLDisp->GetByDispType( DispType );

				// Get the Minimum resource level for the dispatch location
				int MinLevel = m_DispTree.Element( DLDispPtrs[i] )->GetMinRescLevelIdx( RescTypeIdx );
				int CurNumResc = DLDisp->NumRescIdx( RescTypeIdx );
				int NumCanLend = CurNumResc - MinLevel;
				if ( RescAtDL.size() > 0 && NumCanLend > 0 )	{

					// If OutSeason == true then is the julian date outside the dispatch location's fire season
					if ( !OutSeason || DLDisp->IsInFireSeason( Julian ) )	{

						// If the Resources at this dispatch location have not been used much lately ( UseLevel, NumUseDays )
						double MinPreviousLevel = 1;
						for ( int d = 0; d < NumUseDays; d++ )	{

							double PreviousLevel = 1;
							
							if ( Julian - d > 0 )
								PreviousLevel = DLDisp->GetPreviousLevel( Julian - d - 1 );

							if ( PreviousLevel < MinPreviousLevel )
								MinPreviousLevel = PreviousLevel;

						}

						if ( MinPreviousLevel > UseLevel )	{

							// A list of the resource id's at the dispatch location - use to identify subcrews
							list< CResource* > RescIDs;
							list< CResource* >::iterator ItID;
						
							// Iterate through the resources
							for ( list< CResource* >::iterator It = RescAtDL.begin(); It != RescAtDL.end(); It++ )	{

								string ThisRescID = ( *It )->GetRescID();
								bool found = false;

								// If the resource type is not RegionalCrew or the resource id is not in the list
								if ( IsRegionalCrew )	{

									for ( ItID = RescIDs.begin(); ItID != RescIDs.end(); ItID++ )	{

										if ( ( *ItID )->GetRescID() == ThisRescID )
											found = true;

									}

								}

								// If the resource type is Helitack crew and the helicopter does not have an attached crew then the helicopter is not of the right type
								if ( IsHelitackCrew )	{

									CHelicopter* HelicPtr = dynamic_cast< CHelicopter* >( *It );

									if ( HelicPtr != 0 )	{

										if ( HelicPtr->AttachedCrewsListSize() == 0 )
											found = true;

									}

								}

								
								if ( !found )	{

									// Is the resource in season
									bool RescInSeason = ( *It )->IsInSeason( Julian );

									if ( RescInSeason )	{

										// Get Next Available time, if too far away exclude resource
										int NextAvailableTime = ( *It )->GetAvailableTime();

										// If the resource type is regional crew get the next available time for the whole crew
										if ( IsRegionalCrew )	{

											Node = m_DispTree.Parent( DLDispPtrs[i] );
											CGACCDispatcher* GACC = dynamic_cast< CGACCDispatcher* >( m_DispTree.Element( Node ));
                                            
                                            vector< CTypeTwoIACrew > Crews;
                                            if (GACC != 0)  
											    Crews = GACC->GetTypeTwoCrewVector();
                                           
											bool CrewFound = false;
											int c = 0;

											 while ( !CrewFound && c < Crews.size() )	{

												 string CrewID = Crews[c].GetCrewID();

												 if ( CrewID == ThisRescID )
													 CrewFound = true;

												 else 
													 c++;

											 }

											if ( !CrewFound )
												 cout << "Could not find the Type II IA Crew for the subcrew ";

											 else
												 NextAvailableTime = Crews[c].NextAvailableTime();

										}

fail:

										// If the resource is a helitack crew and helicopter
										if ( IsHelitackCrew )	{

											CHelicopter* Helicptr = dynamic_cast< CHelicopter* >( *It );

											if ( Helicptr != 0 )
												NextAvailableTime = Helicptr->GetHelitackCrewAvailableTime();

										}

										int CurrentTime = ( Julian - 1 ) * 1440;

										if ( NextAvailableTime - CurrentTime < TimeLimit )	{

											// Add the resource to a list resources for this dispatch location ordered by Next Available Time
											if ( RescIDs.size() == 0 )
												RescIDs.push_back( *It );

											else	{
											
												ItID = RescIDs.begin(); 
												bool AvFound = false;
												string ListRescID;

												while ( !AvFound && ItID != RescIDs.end() )	{

													ListRescID = ( *ItID )->GetRescID();
													int CompNextAvailableTime = ( *ItID )->GetAvailableTime();

													// Get the Next Available Time for each resource
													if ( IsRegionalCrew )	{

														Node = m_DispTree.Parent( DLDispPtrs[i] );
														CGACCDispatcher* GACC = dynamic_cast< CGACCDispatcher* >( m_DispTree.Element(Node));
                                                                                                                if( !GACC )
                                                                                                                    goto fail2;

														vector< CTypeTwoIACrew > Crews;
                                                        if (GACC != 0)
                                                            Crews = GACC->GetTypeTwoCrewVector();
														bool CrewFound = false;
														int c = 0;

														while ( !CrewFound && c < Crews.size() )	{

															string CrewID = Crews[c].GetCrewID();

															if ( CrewID == ListRescID )
																 CrewFound = true;

															else 
																c++;

														 }

														if ( !CrewFound )
															cout << "Could not find the Type II IA Crew for the subcrew ";

														else
															CompNextAvailableTime = Crews[c].NextAvailableTime();

													}
fail2:
													// Compare the Next Available Times
													if ( NextAvailableTime < CompNextAvailableTime )
														AvFound = true;

													else
														ItID++;

												}

												RescIDs.push_back( *It );

											}

										}		// End resource will be available within specified time limit

									}		// End resource is in season

								}		// End resource type is not a regional crew or is the first instance of one

							}		// End iterate through the resources at the dispatch location, have a list ordered by arrival time

							// Add NumCanLend of these resources to the Ordered Resources list
							ItID = RescIDs.begin();
						 
							for ( int n = 0; n < NumCanLend; n++ )	{

								if ( ItID != RescIDs.end() )	{
																		
									// Finally can add the resource to the ordered list based on the expected level
									if ( OrderedRescs.size() == 0 )	{

										RescPair = std::make_pair( *ItID, DLDisp );
										OrderedRescs.push_back( RescPair );

									}

									else	{
										bool IsFound = false;

										list< std::pair< CResource*, CDLDispatcher* > >::iterator ItPair = OrderedRescs.begin();
										while ( !IsFound  && ItPair != OrderedRescs.end()  )	{

											double CompEL = ( *ItPair ).second->GetExpectLevel( Julian );

											if ( CompEL > DLExpectLevel )
												IsFound = true;

											else 
												ItPair++;

										}

										RescPair = std::make_pair( *ItID, DLDisp );
										OrderedRescs.push_back( RescPair );

									}		// End else ordered resource list not empty

									ItID++;

								}		// End not at end of resouce id list

							}		// Added NumCanLend resources to the ordered resources list			

						}		// End resources at this dispatch location have not been used lately

					}		// Either out of season or can be lent in season

				}		// There are resources of the correct type at the dispatch location and there are enough that some can be lent out

			}		// End if can lend resources

		}		// End of got a good pointer for the dispatch location dispatcher

	}		// End of iterate through dispatch locations with the resource type

	// Now have a list of the dispatch locations needing resources and one of the resources that can be lent ordered by the Expected level at their dispatch location
	// Need to move the resoruces

	// If more dispatch locations that need resources than there are resources available, the dispatch locations with the lowest expected level will not get any
		if ( OrderedDLs.size() > OrderedRescs.size() )
			OrderedDLs.resize( OrderedRescs.size() );

	// Need to move the resources
	// Iterate through the dispatch locations that need resourcess
	list< std::pair< CResource*, CDLDispatcher* > >::iterator ItResc = OrderedRescs.begin();

	for ( list< CDLDispatcher* >::iterator It = OrderedDLs.begin(); It!= OrderedDLs.end(); It++ )	{

		if ( RescType == "RegionalCrew" )	{

			// Get GACC Dispatcher pointer for the dispatch location the crew is moving from
			map< string, OmffrNode< CDispatchBase* >* >::iterator MapIt;
			string DLID = ( *It )->GetDispatcherID();
			
			MapIt = m_DispMap.find( DLID );

			OmffrNode< CDispatchBase* >* Node = m_DispTree.Parent( ( *MapIt ).second );

			CGACCDispatcher* NewGACC = dynamic_cast< CGACCDispatcher* >( m_DispTree.Element( Node ) );

			// Get GACC Dispatcher pointer for the dispatch location the crew is moving to
			DLID = ( *ItResc ).second->GetDispatcherID();

			MapIt = m_DispMap.find( DLID );

			Node = m_DispTree.Parent( ( *MapIt ).second );

			CGACCDispatcher* OldGACC = dynamic_cast< CGACCDispatcher* >( m_DispTree.Element( Node ) );

			// Get the Type II IA Crew reference from the old GACC
			string RescID = ( *ItResc ).first->GetRescID();

            vector< CTypeTwoIACrew > Crews;
            if (OldGACC != 0)
			    Crews = OldGACC->GetTypeTwoCrewVector();

			int i = 0;
			bool found = false;

			while ( !found && i < Crews.size() )	{

				string CompID = Crews[i].GetCrewID();

				if ( CompID == RescID )
					found = true;

				else
					i++;

			}

			
			if ( found )	{

				// Relocate the Regional crew
                if (NewGACC != 0)   {
				    NewGACC->MoveTypeTwoCrew( Crews[i], *It, OldGACC );
				
				    // Adjust Next available time and record values so resource can be returned
				    int AvailableTime = ( Julian - 1 ) * 1440 + TravelTime;
				    Crews[i].SetNextAvailableTime( AvailableTime );
				
				    CResource* RescPtr = Crews[i].GetResourcePtr();
				    RescPtr->SetIsBorrowed( true );
				    int Return = ( Julian - 1 ) * 1440 + ReturnTime;
				    RescPtr->SetReturnTime( Return );
				    int timeMBY = ( Julian - 1 ) * 1440 + 1;
				    string DispLocID = ( *It )->GetDispLoc().GetDispLocID();
				    RescPtr->RecordMove( timeMBY, DispLocID );

				    m_BorrowedResources.push_back( RescPtr );
                }

			}

			
		}

		else if	( IsHelitackCrew )	{

			int AvailableTime = ( Julian - 1 ) * 1440 + TravelTime;
			int Return = ( Julian - 1 ) *1440 + ReturnTime;
			int timeMBY = ( Julian - 1 ) * 1440 + 1;

			// Add the helicopter and all the attached crews to the resource map for the dispatch location where they are being moved
			( *It )->InsertInRescMap( ( *ItResc).first );

			// Remove from the resource map where the resource was located
			( *ItResc ).second->RemoveFromRescMap( ( *ItResc ).first );

			// Adjust Next available time and record values so resource can be returned
			( *ItResc ).first->SetAvailableTime( AvailableTime );

			( *ItResc ).first->SetIsBorrowed( true );
			( *ItResc ).first->SetReturnTime( Return );
			
			m_BorrowedResources.push_back( ( *ItResc ).first );

			// Record move in the resources movement history
			string DispLocID = ( *It )->GetDispLoc().GetDispLocID();

			( *ItResc ).first->RecordMove( timeMBY, DispLocID );	

			// Now for the attached crews
			list< CResource* > AttachedCrews;

			CHelicopter* HelicPtr = dynamic_cast< CHelicopter* >( ( *ItResc).first );

			if ( HelicPtr != 0 )	
				AttachedCrews = HelicPtr->GetAttachedCrewsList();

			else
				cout << "Bad cast to helicopter to get attached crews list";

			for ( list< CResource* >::iterator ItCrews = AttachedCrews.begin(); ItCrews != AttachedCrews.end(); ItCrews++ )	{
				
				// Get the current location for the attached crew
				string DispLocID = ( *ItCrews )->GetCurrentLocation();

				map< string, OmffrNode< CDispatchBase* >* >::iterator MapIt;
				MapIt = m_DispMap.find( DispLocID );

				CDLDispatcher* DLDispPtr = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( ( *MapIt ).second ) );

				// Insert the crew into the resource map
				( *It )->InsertInRescMap( *ItCrews );

				// Remove the crew from the old dispatch location's resource map
				DLDispPtr->RemoveFromRescMap( *ItCrews );

				// Adjust Next available time and record values so resource can be returned
				( *ItCrews )->SetAvailableTime( AvailableTime );

				( *ItCrews )->SetIsBorrowed( true );
				( *ItCrews )->SetReturnTime( Return );

				m_BorrowedResources.push_back( *ItCrews );
				
				// Record move in the resources movement history
				string DLID = ( *It )->GetDispLoc().GetDispLocID();
				( *ItCrews )->RecordMove( timeMBY, DLID );	

			}

		}
		
		else	{
			// Add the resource to the resource map of the dispatch location where it is being moved
			( *It )->InsertInRescMap( ( *ItResc ).first );

			// Remove from the resource map where the resource was located
			( *ItResc ).second->RemoveFromRescMap( ( *ItResc ).first );

			// Adjust Next available time and record values so resource can be returned
			int AvailableTime = ( Julian - 1 ) * 1440 + TravelTime;
			( *ItResc ).first->SetAvailableTime( AvailableTime );

			( *ItResc ).first->SetIsBorrowed( true );
			int Return = ( Julian - 1 ) *1440 + ReturnTime;
			( *ItResc ).first->SetReturnTime( Return );
			
			m_BorrowedResources.push_back( ( *ItResc ).first );

			// Record move in the resources movement history
			int timeMBY = ( Julian - 1 ) * 1440 + 1;
			string DispLocID = ( *It )->GetDispLoc().GetDispLocID();

			( *ItResc ).first->RecordMove( timeMBY, DispLocID );	

		}

		// Move to the next resource in the ordered list
		ItResc++;

		NumResc++;

	}

	return NumResc;
}

// Preposition all the resources as indicated in the GUI interface
void CRunScenario::PrepositionAllResc( PrepositionStruct ATTpp, PrepositionStruct RCRWpp, PrepositionStruct RHelpp, 
	PrepositionStruct FSCRWpp, PrepositionStruct DOICRWpp, PrepositionStruct FSENGpp, PrepositionStruct DOIENGpp, 
	PrepositionStruct FSHELIpp, PrepositionStruct DOIHELIpp, int Julian )
{
	// For each resource Type send the appropriate information to preposition the resources
	string RescType = "Airtanker";
	double MoveLevel =	1.5 - ATTpp.level;
	bool OutSeason =  ATTpp.outOfSeason;
	if ( MoveLevel > 0 )
		PrepositionResourceIA( RescType, MoveLevel, OutSeason, Julian );

	RescType = "RegionalCrew";
	MoveLevel = 1.5 - RCRWpp.level;
	OutSeason = RCRWpp.outOfSeason;
	if ( MoveLevel > 0 )
		PrepositionResourceIA( RescType, MoveLevel, OutSeason, Julian );

	RescType = "RegionalHelicopter";
	MoveLevel = 1.5 - RHelpp.level;
	OutSeason = RHelpp.outOfSeason;
	if ( MoveLevel > 0 )
		PrepositionResourceIA( RescType, MoveLevel, OutSeason, Julian );

	/*RescType = "Smokejumper";
	MoveLevel = SMJRpp.level / 100;
	OutSeason = SMJRpp.outOfSeason;
	if ( MoveLevel > 0 )
		PrepositionResourceIA( RescType, MoveLevel, OutSeason, Julian ); */

	RescType = "FSLocalCRW";
	MoveLevel = 1.5 - FSCRWpp.level;
	OutSeason = FSCRWpp.outOfSeason;
	if ( MoveLevel > 0 )
		PrepositionResourceIA( RescType, MoveLevel, OutSeason, Julian );

	RescType = "DOILocalCRW";
	MoveLevel = 1.5 - DOICRWpp.level;
	OutSeason = DOICRWpp.outOfSeason;
	if ( MoveLevel > 0 )
		PrepositionResourceIA( RescType, MoveLevel, OutSeason, Julian );

	RescType = "FSLocalENG";
	MoveLevel = 1.5 - FSENGpp.level;
	OutSeason = FSENGpp.outOfSeason;
	if ( MoveLevel > 0 )
		PrepositionResourceIA( RescType, MoveLevel, OutSeason, Julian );

	RescType = "DOILocalENG";
	MoveLevel = 1.5 - DOIENGpp.level;
	OutSeason = DOIENGpp.outOfSeason;
	if ( MoveLevel > 0 )
		PrepositionResourceIA( RescType, MoveLevel, OutSeason, Julian );

	RescType = "FSLocalHELI";
	MoveLevel = 1.5 - FSHELIpp.level;
	OutSeason = FSHELIpp.outOfSeason;
	if ( MoveLevel > 0 )
		PrepositionResourceIA( RescType, MoveLevel, OutSeason, Julian );

	RescType = "DOILocalHELI";
	MoveLevel = 1.5 - DOIHELIpp.level;
	OutSeason = DOIHELIpp.outOfSeason;
	if ( MoveLevel > 0 )
		PrepositionResourceIA( RescType, MoveLevel, OutSeason, Julian );


}

// Read in the FPU data
/*
void CRunScenario::ReadFPUs()
{
	string FileNameBase = "/Cprogramming/Fires/";
	vector< string > FPUNames;
	
	// Read in the FPUNames
	ifstream inFPUNameFile( "/Cprogramming/Fires/FPUNames.dat", ios::in );

	if ( !inFPUNameFile )	{
		cerr << "FPU Name file could not be opened" << endl;
	}

	string FPU( "Default" );
	// Read in each FWA entry from file
	while ( inFPUNameFile >> FPU )	{		
		FPUNames.push_back( FPU );
	}

	inFPUNameFile.close();

	for ( int i = 0; i < FPUNames.size(); i++ )	{

		// Construct the name for the FPU file
		string FileName = FileNameBase + FPUNames[i] + ".txt";

		//if constructor opens a file
		ifstream inFPUFile( FileName.c_str(), ios::in );

		//exit program if ifstream could not open file
		if ( !inFPUFile ) {
			cerr << "File could not be opened" << endl;
			//exit( 1 );
		}
		// Enter elements for first FWA 
		string FPUID( "Default" );
		double Day[365];
		double Average[365];
		double Max[365];
		double FausMax[365];
	
		// Read in the FPU entry from file
		for ( int j = 0; j < 365; j++ )	
			inFPUFile >> Day[j] >> Average[j] >> Max[j] >> FausMax[j];
		

		// Construct CFPU object 
		m_VFPU.push_back( CFPU( FPUNames[i], Average, Max, FausMax ) );

		inFPUFile.close();

	}

	// Set the FPU's reliance on large airtankers
	
	// Open the file with the FPU airtanker Reliance values 
	ifstream inRelianceFile( "/Cprogramming/Fires/TankerReliance.dat", ios::in );

	if ( !inRelianceFile )	{
		cerr << "Reliance file could not be opened" << endl;
	}

	double Reliance = 0;
	// Read in each Reliance set from the file
	while ( inRelianceFile >> FPU >> Reliance )	{	
		
		// Find the FPU
		for ( int i = 0; i < m_VFPU.size(); i++ )	{

			string FPUID = m_VFPU[i].GetFPUID();

			if ( FPUID == FPU )
				m_VFPU[i].SetReliance ( Reliance/100 );
		}

	}

	inRelianceFile.close();

}

// Calculate the Expected Level for the FPUs for a scenario
void CRunScenario::CalcFPUExpectLevel()
{
	// Preset for expected level
	int TopUsed = 0;												// Use which value for the top # Fires, 0 = Max
	// For each FPU determine the number of fires for each day
	for ( int f = 0; f < m_VFire.size(); f++ )	{

		int Julian = m_VFire[f].GetJulianDay();
		string FPU = m_VFire[f].GetFWA().GetFPU();

		// Find the FPU the fire is in
		for ( int i = 0; i < m_VFPU.size(); i++ )	{

			string FPUID = m_VFPU[i].GetFPUID();

			// If is the FPU add to the count
			if ( FPUID == FPU )
				m_VFPU[i].AddFire( Julian );

		}

	}
	// Get the Number of Fires for each FPU from the Scenario.dat files in the FPU's folder
	// Open the file with the FPU airtanker Reliance values 
	ifstream inFPUsFile( "/Cprogramming/Data/FPUDone.dat", ios::in );

	if ( !inFPUsFile )	{
		cerr << "The file with the IDs for the completed FPUs could not be opened" << endl;
	}

	string FPU = "";
	vector< string > FPUNames;

	while ( inFPUsFile >> FPU )	{
		FPUNames.push_back( FPU );
	}

	inFPUsFile.close();

	// Enter elements for the fire
	int ScenarioNum = 1;
	int FireNum = 1;
	int JulianDay = 98;
	string DayofWeek = "Monday";
	string DiscoveryTime = "0000";
	int BI = 0;
	double ROS = 0;
	int FuelModel = 121;
	string SpecificCondition = "x";
	int SlopePct = 0;
	bool IsWalkIn = false;
	string Tactic = "HEAD";
	double Distance = 0;
	int Elevation = 1000;
	double LtoWRatio = 1.5;
	int MinNumSteps = 250;
	int MaxNumSteps = 10000;
	string Sunrise = "0600";
	string Sunset = "1000";
	bool WaterDrops = true;
	bool IsPumpNRoll = false;
	string FWAId = "Wilderness_Darby";
	double Latitude = 0.0;
	double Longitude = 0.0;

	// Iterate through the FPU names and get the Scenario1 files then count the fires for each day
	for ( int i = 0; i < FPUNames.size(); i++ )	{

		// Create the name to read the fires from
		string FileName = "/Cprogramming/Data/" + FPUNames[i] + "/Scenario1.dat";

		ifstream inScenarioFile( FileName.c_str(), ios::in );

		if ( !inScenarioFile )	{
		cerr << "The Scenario1.dat file for the FPU: "<< FPUNames[i] << " could not be opened" << endl;
		}

		// Create a vector for the number of fires for each day in the scenario fill it with 365 0s
		vector< int > NumFires;
		for ( int j = 0; j < 365; j++ )
			NumFires.push_back( 0 );

		// Read in each Fire entry from file
		while ( inScenarioFile >> ScenarioNum >> FireNum >> JulianDay >> DayofWeek >> DiscoveryTime >> BI >> ROS >> FuelModel >> SpecificCondition >> SlopePct >>
			IsWalkIn >> Tactic >> Distance >> Elevation >> LtoWRatio >> MinNumSteps >> MaxNumSteps >> Sunrise >> Sunset >> WaterDrops >> 
			IsPumpNRoll >> FWAId >> Latitude >> Longitude )	{

			// Add the fire to the appropriate day
			NumFires[ JulianDay - 1 ] = NumFires[ JulianDay - 1 ] + 1;

		}

		// Find the FPU class
		int k = 0;
		bool Found = false;

		while ( !Found && k < m_VFPU.size() )	{

			string FPUID = m_VFPU[k].GetFPUID();

			if ( FPUNames[i] == FPUID )
				Found = true;
			else
				k++;
		}

		if ( !Found )
			cout << "The FPU Class for the Fire count could not be found \n";

		else	{

			m_VFPU[k].SetNumFiresVector( NumFires );
		}
	}

	// Calculate the Expected Level for Each FPU
	for ( int i = 0; i < m_VFPU.size(); i++ )	{

		// Calculate the Expected Level for each day
		for ( int j = 0; j < 365; j++ )	{

			double Average = m_VFPU[i].GetAverageIdx( j );
			if ( Average == 0 )
				Average = 0.000001;
			int NumFires = m_VFPU[i].GetNumFires( j + 1 );		// Function written for Julian day not index

			double Top = 100;
			if ( TopUsed = 0 )	{
				Top = m_VFPU[i].GetFausMaxIdx( j );
				if ( Top < Average )
					Top = Average;
			}
			else	{
				Top = m_VFPU[i].GetMaxIdx(j);
				if ( Top < Average )
					Top = Average;
			}

			double ExpectLevel = 0;

			// If the number of fires is less than the average for the day
			if ( NumFires < Average )
				ExpectLevel = NumFires * 0.5 / Average;

			// If the number of fires is less than the average for the day
			else if	( NumFires > Average )	{

				// if the number of fires is greater than the Maximum value being used the expected value equals 1
				if ( NumFires < Top )	
					ExpectLevel = ( static_cast< double >( NumFires ) - Average ) * 0.5 / ( Top - Average ) + 0.5;

				else
					ExpectLevel = 1;

			}

			else	// NumFires = Average
				ExpectLevel = 0.5;

			m_VFPU[i].SetExpectLevelIdx( ExpectLevel, j );

		}

	}

}

// Calculate and save the Expected Levels for the airtanker bases
void CRunScenario::CalcATExpectLevel()
{
	// Open the Area files for the airtanker bases
	// Tanker Base Name, FPU id, Area ordered by airtanker base name

	ifstream inAreaFile( "/CProgramming/Fires/TankersSort.dat", ios::in );

	if ( !inAreaFile )	{
		cerr << "Area file for large airtankers could not be opened" << endl;
	}

	// Open the output file
	ofstream  OutATBase( "/CProgramming/Dispatcher/Dispatcher/ATBaseExpectLevel.dat", ios::out );

	//exit program if ifstream could not open file
	if ( !OutATBase ) {
		cerr << "File could not be opened the output file for the airtanker base expected levels" << endl;
	}

	string FPU( "Default" );
	string Base( "Default" );
	string BaseOld( "Default" );
	double Area = 0;
	double ExpectLevel[365];
	for ( int i = 0; i < 365; i++ )
		ExpectLevel[i] = 0.0;

	// Read each entry
	while ( inAreaFile >> Base >> FPU >> Area )	{		
		
		// Is this a new base
		if ( Base != BaseOld )	{
			
			// Save the previous bases information - write to a file
			if ( BaseOld != "Default" )	{
				OutATBase << BaseOld;
				for ( int i = 0; i < 365; i++ ) {
					if ( ExpectLevel[i] < 0.0 )
						ExpectLevel[i] = 0.0;
				
					OutATBase << " " << ExpectLevel[i];
				}

				OutATBase << "\n";
			}

			BaseOld = Base;
			for ( int i = 0; i < 365; i++ )
				ExpectLevel[i] = 0.0;

		}

		// Find the FPU 
		int i = 0;
		bool Found = false;

		while ( !Found && i < m_VFPU.size()	)	{

			string FPUID = m_VFPU[i].GetFPUID();

			if ( FPUID == FPU )
				Found = true;

			else 
				i++;

		}

		if ( !Found )
			cout << "The FPU could not be found in the vector of FPUs for the calculation of the AT Bases Expected Levels \n";

		else	{

			double FPUReliance = m_VFPU[i].GetReliance();

			for ( int j = 0; j < 365; j++ )	
				ExpectLevel[j] = ExpectLevel[j] + ( m_VFPU[i].GetExpectLevelIdx( j ) * FPUReliance * Area );

		}

	}		// end while still entries in the Tankers Sorted file

	// Save the last base's informtaion - write to file
	OutATBase << Base;
	for ( int i = 0; i < 365; i++ )
		OutATBase << " " << ExpectLevel[i];

	OutATBase << "\n";


	// Close the files
	inAreaFile.close();
	OutATBase.close();

}
// Print the information about the FPU
void CRunScenario::PrintFPUInfo()
{
	// Open the output file
	ofstream  OutFPU( "/CProgramming/Dispatcher/Dispatcher/Airtankers/FPUInfo.dat", ios::out );

	//exit program if ifstream could not open file
	if ( !OutFPU ) {
		cerr << "File could not be opened the output file for the fPUs" << endl;
	}

	// Print headers 
	OutFPU << "Number of Fires: Average Number of Fires: Maximum Number of Fires: 9th Ranked Number of Fires ( out of 10): Expected Level:\n";

	// Iterate through the FPUs and Print their information
	for ( int i = 0; i < m_VFPU.size(); i++ )	{

		string FPUID = m_VFPU[i].GetFPUID();

		OutFPU << FPUID << "\n";
		for ( int j = 0; j < 365; j++ )	{

			int NumFires = m_VFPU[i].GetNumFires( j+1 );
			double Average = m_VFPU[i].GetAverageIdx( j );
			double Max = m_VFPU[i].GetMaxIdx( j );
			double FausMax = m_VFPU[i].GetFausMaxIdx( j );
			double ExpectLevel = m_VFPU[i].GetExpectLevelIdx( j );

			OutFPU << NumFires << " " << Average << " " << Max << " " << FausMax << " " << ExpectLevel << "\n";

		}

	}

	OutFPU.close();

}
*/

// Create a file that contains the FPU weights for upper level dispatch locations to calculate the Expected Levels for the DLDispatchers
void CRunScenario::DLDispatcherExpectedWeights()
{
	// Get the DLDispatchers in the analysis that are attached to the GACC or National nodes
	vector< OmffrNode< CDispatchBase* >* > NatNodes = m_DispTree.GetExternalNodes( m_DispTree.Root(), 1 );
	vector< OmffrNode< CDispatchBase* >* > GACCNodes = m_DispTree.GetExternalNodes( m_DispTree.Root(), 2 );

	// Merge the two vectors
	NatNodes.insert( NatNodes.begin(), GACCNodes.begin(), GACCNodes.end() );

	// Open the output file
	ofstream OutFile( "DispLocWeights" , ios::out );

	if ( !OutFile )
		cout << "Could not open output file for dispatch location weights";

	else	{

		// Iterate through the dispatchers
		for ( int i = 0; i < NatNodes.size(); i++ )	{

			// If the disptacher is an airtanker base ( ends in KYLE ) skip, because used a different method for determining weights
			string DispID = m_DispTree.Element( NatNodes[i] )->GetDispatcherID();

			string EndID = "";
			if ( DispID.length() > 4 )
				EndID = DispID.substr( DispID.length() - 4, 4 );

			if ( EndID != "KYLE" )	{

				CDLDispatcher* DLDisp = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( NatNodes[i] ) );

				if ( DLDisp != 0 )	{
	
					// Get pointers to the FWAs associated with the dispatch location
					vector< CFWA* > FWAs = GetAssociatedFWAs( DLDisp );

					// Get the FPU for each of these FWAs
					list< string > FPUID;
		
					for ( int j = 0; j < FWAs.size(); j++ )	{

						string FPU = FWAs[j]->GetFPU();
					
						FPUID.push_back( FPU );

					}

					// Sort the list so all the FPUs are located together
					FPUID.sort();

					// Iterate through the list and determine the percent for each FPU
					list< string >::iterator It;
					int Count = 0;
					string Previous = "New";
					double Total = static_cast< double >( FPUID.size() );

					for ( It = FPUID.begin(); It != FPUID.end(); It++ )	{

						// Get the FPUId, is it new?
						string FPU = ( *It );

						if ( FPU != Previous )	{

							if ( Previous != "New" )	{

								// Store the data to a file
								double Countd = static_cast< double >( Count );
								double Weight = Countd / Total;
								OutFile << DispID << "	" << Previous << "	" << Weight << endl;
							
							}

							Previous = FPU;
							Count = 1;

						}

						else
							Count++;
					}

					// Store last data to a file
					double Countd = static_cast< double >( Count );
					double Weight = Countd / Total;
					OutFile << DispID << "	" << Previous << "	" << Weight << endl;

				}	// Got a good pointer to the CDLDispatcher

			}	// Not Airtanker Base

		}	// Next Node

		OutFile.close();

	}	// Could open output file

}

// Read in the files with the names of the Airtanker bases, and dispatch locations with 20 person crews, Type I, II and III helicopter and smokejumpers
// Create vectors in CRunScenario with this information for moving these types of resources
void CRunScenario::CreateRescTypeVectors()
{
	string BaseID;

	// Open the Airtanker File
	ifstream ATTFile( "Airtankers.dat", ios::in );

	if ( !ATTFile )
		cout << "Cannot open file with IDs for airtanker bases";

	else	{

		while ( ATTFile >> BaseID )	{

			m_AirtankerDLs.push_back( BaseID );

		}

	}

	ATTFile.close();

	// Open the Regions Crew File
	ifstream CrewFile( "Crew20.dat", ios::in );

	if ( !CrewFile )
		cout << "Cannot open file with IDs for Regional Crew bases";

	else	{

		while ( CrewFile >> BaseID )	{

			m_RegionalCrewDLs.push_back( BaseID );

		}

	}

	CrewFile.close();

	// Open the Regional Helicopter File
	ifstream HELFile( "RegionalHeli.dat", ios::in );

	if ( !HELFile )
		cout << "Cannot open file with IDs for Regional Helicopter bases";

	else	{

		while ( HELFile >> BaseID )	{

			m_RegionalHelicopterDLs.push_back( BaseID );

		}

	}

	// Open the Smokejumper File
	ifstream SMJRFile( "SMJR.dat", ios::in );

	if ( !SMJRFile )
		cout << "Cannot open file with IDs for Smokejumper bases";

	else	{

		while ( SMJRFile >> BaseID )	{

			m_SmokejumperDLs.push_back( BaseID );

		}

	}

	SMJRFile.close();

	// Open the DOI Crew File
	ifstream DOICRWFile( "DOICRW.dat", ios::in );

	if ( !DOICRWFile )
		cout << "Cannot open file with IDs for DOI Crew bases";

	else	{

		while ( DOICRWFile >> BaseID )	{

			m_DOICRWDLs.push_back( BaseID );

		}

	}

	DOICRWFile.close();

	// Open the DOI Dozer File
	ifstream DOIDZRFile( "DOIDZR.dat", ios::in );

	if ( !DOIDZRFile )
		cout << "Cannot open file with IDs for DOI Dozer bases";

	else	{

		while ( DOIDZRFile >> BaseID )	{

			m_DOIDZRDLs.push_back( BaseID );

		}

	}

	DOIDZRFile.close();

	// Open the DOI Engine File
	ifstream DOIENGFile( "DOIENG.dat", ios::in );

	if ( !DOIENGFile )
		cout << "Cannot open file with IDs for DOI Engine bases";

	else	{

		while ( DOIENGFile >> BaseID )	{

			m_DOIENGDLs.push_back( BaseID );

		}

	}

	DOIENGFile.close();

	// Open the DOI Helicopter File
	ifstream DOIHelFile( "DOIHel.dat", ios::in );

	if ( !DOIHelFile )
		cout << "Cannot open file with IDs for DOI Helicopter bases";

	else	{

		while ( DOIHelFile >> BaseID )	{

			m_DOIHelDLs.push_back( BaseID );

		}

	}

	DOIHelFile.close();

	// Open the DOI Helitack File
	ifstream DOIHELIFile( "DOIHELI.dat", ios::in );

	if ( !DOIHELIFile )
		cout << "Cannot open file with IDs for DOI Helitack bases";

	else	{

		while ( DOIHELIFile >> BaseID )	{

			m_DOIHELIDLs.push_back( BaseID );

		}

	}

	DOIHELIFile.close();

	// Open the DOI SEAT File
	ifstream DOISEATFile( "DOISEAT.dat", ios::in );

	if ( !DOISEATFile )
		cout << "Cannot open file with IDs for DOI SEAT bases";

	else	{

		while ( DOISEATFile >> BaseID )	{

			m_DOISEATDLs.push_back( BaseID );

		}

	}

	DOISEATFile.close();

	// Open the FS Crew File
	ifstream FSCRWFile( "FSCRW.dat", ios::in );

	if ( !FSCRWFile )
		cout << "Cannot open file with IDs for FS Crew bases";

	else	{

		while ( FSCRWFile >> BaseID )	{

			m_FSCRWDLs.push_back( BaseID );

		}

	}

	FSCRWFile.close();

	// Open the FS Dozer File
	ifstream FSDZRFile( "FSDZR.dat", ios::in );

	if ( !FSDZRFile )
		cout << "Cannot open file with IDs for FS Dozer bases";

	else	{

		while ( FSDZRFile >> BaseID )	{

			m_FSDZRDLs.push_back( BaseID );

		}

	}

	FSDZRFile.close();

	// Open the FS EngineFile
	ifstream FSENGFile( "FSENG.dat", ios::in );

	if ( !FSENGFile )
		cout << "Cannot open file with IDs for FS Engine bases";

	else	{

		while ( FSENGFile >> BaseID )	{

			m_FSENGDLs.push_back( BaseID );

		}

	}

	FSENGFile.close();

	// Open the FS Helicopter File
	ifstream FSHelFile( "FSHel.dat", ios::in );

	if ( !FSHelFile )
		cout << "Cannot open file with IDs for FS Helicopter bases";

	else	{

		while ( FSHelFile >> BaseID )	{

			m_FSHelDLs.push_back( BaseID );

		}

	}

	FSHelFile.close();

	// Open the FS Helitack File
	ifstream FSHELIFile( "FSHELI.dat", ios::in );

	if ( !FSHELIFile )
		cout << "Cannot open file with IDs for FS Helitack bases";

	else	{

		while ( FSHELIFile >> BaseID )	{

			m_FSHELIDLs.push_back( BaseID );

		}

	}

	FSHELIFile.close();

	// Open the FS SEAT File
	ifstream FSSEATFile( "FSSEAT.dat", ios::in );

	if ( !FSSEATFile )
		cout << "Cannot open file with IDs for FS SEAT bases";

	else	{

		while ( FSSEATFile >> BaseID )	{

			m_FSSEATDLs.push_back( BaseID );

		}

	}

	FSSEATFile.close();

}
	
// Load the expected levels for the year into all the nodes in the tree - the BaseForcast indicates the number of days for the forcast for the internal and level 3 dispatchers
void CRunScenario::LoadExpectedLevels( int Scenario, int BaseForcast )
{
	// Load the expected levels for the the Internal Nodes
	char ExpectedFile[512];
#ifdef WIN32
	sprintf( ExpectedFile, "/Cprogramming/Expected/Expected/Expected/DataFiles/Year%dExpected%d.dat", Scenario, BaseForcast );
#else
        BaseForcast = 2;
	sprintf( ExpectedFile, "Year%dExpected%d.txt", Scenario, BaseForcast );
#endif

	ifstream InExp( ExpectedFile, ios::in );

	if ( !InExp )
		cout << "Could not open the Expected Level files for the interior nodes.";

	else	{

		// Read each file entry and put the expected level values in the appropriate node
		string NodeID = "Default";
		vector< double > ELs;
		for ( int i = 0; i < 365; i++ )	
			ELs.push_back( 0.0 );

		while ( InExp >> NodeID )	{

			for ( int i = 0; i < 365; i++ )
				InExp >> ELs[i];

			map< string, OmffrNode< CDispatchBase* >* >::iterator It;
			OmffrNode< CDispatchBase* > *Node; 

			// See if the node is a dispatch location dispatcher
			It = m_DispMap.find( NodeID );

			if ( It != m_DispMap.end() )	{
				Node = It->second;

				// Set the values for the internal node
				m_DispTree.Element( Node )->SetExpectLevelVector( ELs );

			}

			else	{
				
				// Is the node an interior node?
				It = m_InMap.find( NodeID );

				if ( It != m_InMap.end() )	{
					Node = It->second;

					// Set the values for the internal node
					m_DispTree.Element( Node )->SetExpectLevelVector( ELs );

				}

			}

		}

	}

	InExp.close();

	// Load the expected levels for the the AirtankerBases
#ifdef WIN32
	sprintf( ExpectedFile, "/Cprogramming/Expected/Expected/Expected/DataFiles/Year%dATExpected2.dat", Scenario );
#else
	sprintf( ExpectedFile, "Year%dATExpected2.txt", Scenario );
#endif 

	ifstream InAT( ExpectedFile, ios::in );

	if ( !InAT )
		cout << "Could not open the Expected Level files for the airtanker bases.";

	else	{

		// Read each file entry and put the expected level values in the appropriate node
		string NodeID = "Default";
		vector< double > ELs;
		for ( int i = 0; i < 365; i++ )	
			ELs.push_back( 0.0 );

		while ( InAT >> NodeID )	{

			for ( int i = 0; i < 365; i++ )
				InAT >> ELs[i];

			map< string, OmffrNode< CDispatchBase* >* >::iterator It = m_DispMap.find( NodeID );

			OmffrNode< CDispatchBase* > *Node;

			if ( It != m_DispMap.end() )	{
				Node = It->second;

				// Set the values for the internal node
				m_DispTree.Element( Node )->SetExpectLevelVector( ELs );

			}

		}

	}

	InAT.close();

}

bool CRunScenario::SimulateContain( int firstArrival, double size, CFire &fire )
{
    double ros = fire.GetROS();
    if( ros < 14.8 )
    {
        if( firstArrival >= 7 && firstArrival < 278 )
        {
            if( size < 68 )
            {
                return false;
            }
        }
    }
    return true;
}

// Add an escaped fire to the escape fire vector
void CRunScenario::AddEscapeFire( int EscapeDate, int EscapeTime, CFire &Fire )
{
	m_VEscapes.push_back( CEscape( EscapeDate, EscapeTime, Fire )	);

	// Randomly generate the rest of the escape fire information
	int Index = m_VEscapes.size() - 1;
	m_VEscapes[Index].GenerateEscapeLevel();

}

// Order the escape fires for the day for each internal node according to importance for getting support from IA resources
// Order by Level then escape time.  since want to use local resources for Local and regional fires order 1-3 level
list< int > CRunScenario::OrderEscapeFires()
{
	// Vector for ordered escape fires
	list< int > LOrderedEscapes;
	
	// Determine if there are any escape fires since the last fire day
	int size = m_VEscapes.size();

	if ( m_EscapeIdx >= size - 1 )
		return LOrderedEscapes;

	// There are escape fires since the last fire day
	else	{

		// Iterate through the escape fires since the last fire day and order
		for ( int i = m_EscapeIdx + 1; i < m_VEscapes.size(); i++ )	{

			list< int >::iterator It = LOrderedEscapes.begin();
			
			if ( LOrderedEscapes.size() == 0 )	
				LOrderedEscapes.insert( It, i );

			else	{

				int Level = m_VEscapes[i].GetLevel();
				int EscapeTime = m_VEscapes[i].GetEscapeTimeBY();

				// Iterate through the escape fires in the ordered vector and find the placement for this fire
				bool found = false;
				
				while ( !found && It != LOrderedEscapes.end() )	{

					int CompLevel = m_VEscapes[ *It ].GetLevel();
					int CompEscapeTime = m_VEscapes[ *It ].GetEscapeTimeBY();

					if ( Level < CompLevel )
						found = true;

					else	{

						if ( Level == CompLevel )	{

							// Compare escape fire times
							if ( EscapeTime < CompEscapeTime )
								found = true;

							else
								It++;

						}

						else
							It++;

					}

				}

				LOrderedEscapes.insert( It, i );

			}

		}

	}

	return LOrderedEscapes;
}

// Determine the draw down of resources except Type II IA crews for the day
void CRunScenario::DailyDrawDown( list< int > LOrderedEscapeFires )
{
	// Is the Escape fire list empty?
	if ( LOrderedEscapeFires.size() == 0 )
		return;

	list< CDLDispatcher* > OrderedCRW;
	list< CDLDispatcher* > OrderedDZR;
	list< CDLDispatcher* > OrderedENG;
	list< CDLDispatcher* > OrderedHel;
	list< CDLDispatcher* > OrderedHELI;
	list< CDLDispatcher* > OrderedSEAT;
	
	int OldJulian = 0;
	
	// Iterate through the escape fires in the list and deploy resources
	list< int >::iterator It;
	for ( It = LOrderedEscapeFires.begin(); It != LOrderedEscapeFires.end(); It++ )	{

		int Level = m_VEscapes[ *It ].GetLevel();
		vector< int > ReqRescs = m_VEscapes[ *It ].GetReqRescVector();
		int EscapeTime = m_VEscapes[ *It ].GetEscapeTimeBY();
		int Duration = m_VEscapes[ *It ].GetDuration();
		int FireNum = m_VEscapes[ *It ].GetFire().GetFireNumber();
		int Julian = m_VEscapes[ *It ].GetEscapeDate();
			
		// Are there any IA resources requested for this fire
		int sum = 0;
		for ( int i = 0; i < ReqRescs.size(); i++ )	
			sum = sum + ReqRescs[i];

		if ( sum != 0 )	{
			
			// If the level is 1 only use local resources on the fire ( dispatch locations associated with the Fire's FWA )
			if ( Level == 1 )	{

				CFWA &FWA = m_VEscapes[ *It ].GetFire().GetFWA();

				// Get the dispatch location associated with this Fire's FWA
				list< string > DispLocs = GetOrderedDispLocsbyLevel( FWA, 3 );

				std::map<std::string, OmffrNode<CDispatchBase*>*>::iterator MapIt;
				list< string >::iterator DLIt;

				for ( DLIt= DispLocs.begin(); DLIt != DispLocs.end(); DLIt++ )	{

					if ( sum > 0 )	{

						// Get the node for the Dispatch location dispatcher
						MapIt = m_DispMap.find( *DLIt );

						if ( MapIt != m_DispMap.end() )	{

							OmffrNode< CDispatchBase* >* Node = ( *MapIt ).second;

							// This escape fire is local level not GACC so do not use resources from dispatch locations attached to the GACC
							int NodeLevel = m_DispTree.Depth( Node );

							if ( NodeLevel == 3 )	{

								CDLDispatcher* DLDisp = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( Node ) );

								if ( DLDisp != 0 )	{

									// Assuming that the expected fire level for this dispatcher does not come into play for level 1 escape fires

									// Iterate through the requested resources and see if any are available at the fire's escape time from this dispatch location
									for ( int i = 0; i < 13; i++ )	{

										if ( ReqRescs[i] > 0  )	{

											// Get the post escape time for the resource type at the fwa
											int EscapeDelay = 0;
											if ( i == 2 || i == 11 )
												EscapeDelay = FWA.GetEscapeDelay(0);
											if ( i == 4 || i == 5 )
												EscapeDelay = FWA.GetEscapeDelay(1);
											if ( i == 1 )
												EscapeDelay = FWA.GetEscapeDelay(2);
											if ( i == 3 )
												EscapeDelay = FWA.GetEscapeDelay(3);
											if ( i == 7 )
												EscapeDelay = FWA.GetEscapeDelay(4);
											if ( i == 10 )
												EscapeDelay = FWA.GetEscapeDelay(5);

											int EscapeTime2 = EscapeDelay + EscapeTime;

											// Get the resources at this dispatch location and determine if they are the right type and are available
											string DispType = DLDisp->ConvertDispType( i );						// Convert the dispatch type to a string
											list< CResource* > LResources = DLDisp->GetByDispType( DispType );	// Get the resources of this type at the dispatch location

											if ( LResources.size() > 0 )								// Were there any resources at the dispatch location
											{
												// Iterate through the list to determine if resources are currently available
												list< CResource* >::iterator Iter = LResources.begin();
												bool found = false;

												while ( !found && Iter != LResources.end() )	{
					
													int AvailableTime = (*Iter)->GetAvailableTime();

													if ( AvailableTime <= EscapeTime2 )
														found = true;

													else
														Iter++;

												}

												// Found a resource to apply the draw down to
												if ( found )	{

													// Apply Draw down
													int Time = Duration * 1440 + EscapeTime2;
													( *Iter )->SetAvailableTime( Time );

													// add to resources draw down vector
													( *Iter )->RecordDrawDown( EscapeTime, FireNum );
											
													// reduce required resources
													ReqRescs[i] = ReqRescs[i] - 1;
													sum = sum - 1;

												}

											}	// end iterate through resources at the dispatch location

										}	// end iterate through the requested resources vector

									}	// end there are resources of this dispatch type needed

								}	// end successful dynamic cast to dispatch location dispatcher

							}	// Node at the local level

						}	// end found node

					}	// end more resources needed

				}	// end iterate through dispatch locations

			}	// end if level 1

			if ( Level == 2 )	{

				// Reset the time for the resources to arrive to the morning after the fire
				EscapeTime = Julian * 1440;

				// Get the Gacc for the fire
				string FPUID = m_VEscapes[ *It ].GetFire().GetFWA().GetFPU();
				string GACCID = FPUID.substr( 0, 2 );

				// Get the node for the Dispatch location dispatcher
				std::map<std::string, OmffrNode<CDispatchBase*>*>::iterator MapIt;
				MapIt = m_InMap.find( GACCID );

				if ( MapIt != m_InMap.end() )	{

					OmffrNode< CDispatchBase* > *GACCNode = ( *MapIt ).second;

					// Does the requested resources set include Type II IA Crews?
					if ( ReqRescs[13] > 0 )	{

						// Fill the Type II IA Crews with crews from the parent GACC
						// Get a pointer to the GACC dispatcher
						CGACCDispatcher* GACC = dynamic_cast< CGACCDispatcher* >( m_DispTree.Element( GACCNode ) );

						if ( GACC != 0 )	{

							// Get the number of Type II IA Crews at the GACC
							int NumberCrews = GACC->NumberTypeTwoCrews();

							// Determine the time when the crew will be released after draw down
							int Time = Duration * 1440 + EscapeTime;

							// If there are some Type II Crews
							if ( NumberCrews > 0 )	{

								// Order the crews according to thier next available time
								list< int > OrderedCrews;
								list< int >::iterator CIt = OrderedCrews.begin();

								for ( int c = 0; c < NumberCrews; c++ )	{

									// Get the next available time for the crew
									int Available = GACC->GetTypeTwoCrew( c ).NextAvailableTime();

									if ( Available < EscapeTime )	{		// Place the crew in the ordered crew list

										if ( OrderedCrews.empty() )
											OrderedCrews.push_back( c );

										else	{

											bool found = false;
										
											while ( !found && CIt != OrderedCrews.end() )	{
	
												int CompAvailable = GACC->GetTypeTwoCrew( *CIt ).NextAvailableTime();

												if ( Available < CompAvailable )
													found = true;

												else
													CIt++;

											}

											OrderedCrews.insert( CIt, c );

										}

									}

								}

								// If there are any crews in the ordered list they can be used for draw down for this fire
								int Size = OrderedCrews.size();

								if ( Size > ReqRescs[13] )
									Size = ReqRescs[13];

								// Deploy these resources
								int c = 0;
								CIt = OrderedCrews.begin();

								while ( c < Size )	{

									// Change the next avialable time for the Type II IA Crew and it's parts
									GACC->SetTypeTwoCrewNextAvailableTime( *CIt, Time );
									GACC->AddtoDrawDownVector( *CIt, EscapeTime, FireNum );

									ReqRescs[13] = ReqRescs[13] - 1;

									CIt++;
									c++;
		
								}		// End of apply drawdown

							}  

						}

					}	// end fill Type II IA crew requests

					if ( ReqRescs[14] > 0 )	{

						// Fill the Large Helicopters from the parent GACC
						// Get a vector of all the children to the GACC node
						vector< OmffrNode< CDispatchBase* >* > Nodes = m_DispTree.Children( GACCNode );

						// Iterate through all the nodes, find the dispatch locations and determine if there are helicopter at them
						// Order the helicopters by available time
						list< CResource* > OrderedRescs;
						list< CResource* >::iterator RIt;
						int Time = Duration * 1440 + EscapeTime;

						for ( int n = 0; n < Nodes.size(); n++ )	{

							CDLDispatcher* DLDisp = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( Nodes[n] ) );
							if ( DLDisp != 0 )	{

								// Get the resources at this dispatch location and place in list according to next available time
								string DispType = DLDisp->ConvertDispType( 6 );						// Convert the dispatch type to a string
								list< CResource* > LResources = DLDisp->GetByDispType( DispType );	// Get the resources of this type at the dispatch location

								if ( LResources.size() > 0 )								// Were there any resources at the dispatch location
								{
									// Iterate through the list and determine available time
									list< CResource* >::iterator Iter;

									for ( Iter = LResources.begin(); Iter!= LResources.end(); Iter++ )	{

										int Available = ( *Iter )->GetAvailableTime();

										// If the resource is available before the fire is contained place in the ordered list
										if ( Available < Time )	{

											// If the resource is available before the fire is contained
											if ( Available < EscapeTime )
												Available = EscapeTime;

											if ( OrderedRescs.size() == 0 )	
												OrderedRescs.push_back( *Iter );

											bool found = false;
											RIt = OrderedRescs.begin();

											while ( !found && RIt != OrderedRescs.end() )	{
					
												int CompAvailable = (*RIt )->GetAvailableTime();
												if ( CompAvailable < EscapeTime )
													CompAvailable = EscapeTime;

												if ( Available < CompAvailable )
													found = true;

												else
													RIt++;

											}

											// Insert in list
											OrderedRescs.insert( RIt, *Iter );

										}

									}

								}

							}

						}

						// Have an ordered list of GACC level helicopters to draw down from
						int Size = OrderedRescs.size();

						if ( Size > ReqRescs[14] )	
							Size = ReqRescs[14];

						// Apply the draw down
						RIt = OrderedRescs.begin();

						for ( int n = 0; n < Size; n++ )	{
						
							( *RIt )->SetAvailableTime( Time );

							// add to resources draw down vector
							( *RIt )->RecordDrawDown( EscapeTime, FireNum );
											
							// reduce required resources
							ReqRescs[14] = ReqRescs[14] - 1;
							RIt++;

						}

					}	// end fill Type I Helicopter requests

					// Are there requests for airtankers
					if ( ReqRescs[0] > 0 )	{

						// Iterate through the airtanker bases and find the airtankers that are available and at the lowest Expected Level
						OmffrNode< CDispatchBase* >* Node = m_DispTree.Root();

						CNatDispatcher* NatNode = dynamic_cast< CNatDispatcher* >( m_DispTree.Element( Node ) );

						if ( NatNode != 0 )	{

							// Get the vector of pointers to the airtanker bases
							vector< CDLDispatcher* > VAirtankerBases = NatNode->GetAtDispatchersVector();

							// Iterate through the vector of airtanker bases and find the airtankers that are available and at the base with the lowest expected level
							list< CDLDispatcher* > OrderedAtBases;
							list< CDLDispatcher* >::iterator DLIt;

							for ( int d = 0; d < VAirtankerBases.size(); d++ )	{

								// Is there an airtanker here
								string DispType = VAirtankerBases[d]->ConvertDispType( 1 );						// Convert the dispatch type to a string
								list< CResource* > LResources = VAirtankerBases[d]->GetByDispType( DispType );	// Get the resources of this type at the dispatch location

								if ( LResources.size() > 0 )								// Were there any resources at the dispatch location
								{
									// Order the dispatch location dispatchers with resources by their expected values
									double ExpectLevel = VAirtankerBases[d]->GetExpectLevel( Julian );

									if ( OrderedAtBases.size() == 0 )	
										OrderedAtBases.push_back( VAirtankerBases[d] );

									else	{

										bool found = false;
										DLIt = OrderedAtBases.begin();

										while ( !found && DLIt != OrderedAtBases.end() )	{
					
											double CompExpectLevel = (*DLIt )->GetExpectLevel( Julian );

											if ( ExpectLevel < CompExpectLevel )
												found = true;

											else
												DLIt++;

										}

										// Insert in list
										OrderedAtBases.insert( DLIt, VAirtankerBases[d] );

									}

								}

							}

							// Have a list of airtanker bases, with airtankers, ordered by the expected level for the escape day
							int Size = OrderedAtBases.size();

							if ( Size > 0 )	{	
								
								// Move the airtankers at the base to the base nearest the escape fire

								CFWA FWA = m_VEscapes[ *It ].GetFire().GetFWA();
								string ClosestATBase = FWA.GetClosestATBase();

								//std::map<std::string, OmffrNode<CDispatchBase*>*>::iterator MapIt;
								MapIt = m_DispMap.find( ClosestATBase );

								OmffrNode< CDispatchBase* > *Node = ( *MapIt ).second;

								CDLDispatcher *DLDisp = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( Node ) );

								if ( DLDisp != 0 )	{

									DLIt = OrderedAtBases.begin();
							
									while ( ReqRescs[0] > 0 && DLIt != OrderedAtBases.end() )	{
						
										// Go to the airtanker base and get the airtankers located there
										list< CResource* > LResources = ( *DLIt )->GetByDispType( "ATT" );	// Get the resources of this type at the dispatch location
										list< CResource* >::iterator RIt;

										for ( RIt = LResources.begin(); RIt != LResources.end(); RIt++ )	{

											// Add the resource to the resource map of the dispatch location where it is being moved
											DLDisp->InsertInRescMap( *RIt );

											// Remove from the resource map where the resource was located
											( *DLIt )->RemoveFromRescMap( *RIt );

											// Record move in the resources movement history
											int AvailableTime = ( *RIt )->GetAvailableTime();
											if ( AvailableTime < EscapeTime )
												AvailableTime = EscapeTime;

											( *RIt )->RecordMove( AvailableTime, ClosestATBase );	
											
											// reduce required resources
											ReqRescs[0] = ReqRescs[0] - 1;

										}

										DLIt++;

									}

								}

							}

						}	// End national node casting success

					}

					// Fill the requests for other resource types from the local dispatchers in the GACC
					sum = 0;
					for ( int i = 1; i < 13; i++ )	
						sum = sum + ReqRescs[i];

					if ( sum > 0 )	{

						// Get a list of the FPU Dispatchers that are in the GACC ordered by their expected level
						vector< OmffrNode< CDispatchBase* >* > Nodes = m_DispTree.Children( GACCNode );
	
						// Iterate through all the nodes, determine if the node is an FPU, order the FPUs by the expected levels
						list< OmffrNode< CDispatchBase* >* > OrderedFPUs;
						list< OmffrNode< CDispatchBase* >* >::iterator DIt;
						int Time = Duration * 1440 + EscapeTime;

						for ( int n = 0; n < Nodes.size(); n++ )	{

							// If an internal node insert in the ordered FPU list
							if ( m_DispTree.Internal( Nodes[n] ) )	{

								if ( OrderedFPUs.size() == 0 )
									OrderedFPUs.push_back( Nodes[n] );

								else	{
									bool found = false;
									DIt = OrderedFPUs.begin();

									double ExpectLevel = m_DispTree.Element( Nodes[n] )->GetExpectLevel( Julian );

									while ( !found && DIt != OrderedFPUs.end() )	{

										double CompExpectLevel = m_DispTree.Element( *DIt )->GetExpectLevel( Julian );

										if ( ExpectLevel < CompExpectLevel )
											found = true;

										else
											DIt++;

									}

									OrderedFPUs.insert( DIt, Nodes[n] );

								}

							}

						}

						// Have a list of FPU nodes that are ordered by the expected level
						// Iterate through the dispatch location attached to these nodes and fill the requested resources
						DIt = OrderedFPUs.begin(); 

						while ( sum > 0 && DIt != OrderedFPUs.end() )	{

							// Get the Dispatch location dispatchers that are children of the FPU
							vector < OmffrNode < CDispatchBase* >* > DLNodes = m_DispTree.Children( *DIt );
							int d = 0;

							// Iterate through the Dispatch locations
							while ( sum > 0 && d < DLNodes.size() )	{

								CDLDispatcher *DLDisp = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( DLNodes[d] ) );

								if ( DLDisp != 0 )	{

									// Iterate through the different resource types and fill the requests
									for ( int r = 1; r < 13; r++ )	{

										string DispType = DLDisp->ConvertDispType( r );						// Convert the dispatch type to a string
										list< CResource* > LResources = DLDisp->GetByDispType( DispType );	// Get the resources of this type at the dispatch location

										if ( LResources.size() > 0 )	{	
			
											// Iterate throuth the resources, check the available time and deploy to escape fire
											list< CResource* >::iterator RIt= LResources.begin();

											while ( ReqRescs[r] > 0 && RIt != LResources.end() )	{

												// if is helicopter check if helitack are also deployed
												if ( r == 6 )	{

													CHelicopter* Helic = dynamic_cast< CHelicopter* >( * RIt );

													if ( Helic != 0 )	{

														// If helitack are also deployed find helicopter with helitack attached
														if ( ReqRescs[7] > 0 )	{

															if ( Helic->AttachedCrewsListSize() > 0 )	{

																int Available = Helic->GetHelitackCrewAvailableTime();

																// Send Helitack Crews and Helicopter if available
																if ( Available < Time )	{

																	list< CResource* > AttachedCrews = Helic->GetAttachedCrewsList();

																	// Reset available time and record drawdown for helitack
																	for ( list< CResource* >::iterator ACIt = AttachedCrews.begin(); ACIt != AttachedCrews.end(); ACIt++ )	{

																		( *ACIt )->SetAvailableTime( Time );

																		( *ACIt )->RecordDrawDown( EscapeTime, FireNum );

																		ReqRescs[7] = ReqRescs[7] - 1;

																	}

																	// Reset available time and record drawdown for helicopter
																	( *RIt )->SetAvailableTime( Time );

																	// add to resources draw down vector
																	( *RIt )->RecordDrawDown( EscapeTime, FireNum );
										
																	// reduce required resources
																	ReqRescs[6] = ReqRescs[6] - 1; 

																}

															}

															else	{	//no helitack, use helicopter without attached crews

																int Available = ( *RIt )->GetAvailableTime();

																if ( Available < Time )	{

																	( *RIt )->SetAvailableTime( Time );

																	// add to resources draw down vector
																	( *RIt )->RecordDrawDown( EscapeTime, FireNum );
										
																	// reduce required resources
																	ReqRescs[r] = ReqRescs[r] - 1;

																}

															}

														}

													}

												}

												else if ( r != 7 )	{  // not helicopter or helitack

													int Available = ( *RIt )->GetAvailableTime();

													if ( Available < Time )	{

														( *RIt )->SetAvailableTime( Time );

														// add to resources draw down vector
														( *RIt )->RecordDrawDown( EscapeTime, FireNum );
										
														// reduce required resources
														ReqRescs[r] = ReqRescs[r] - 1;

													}

												}

												RIt++ ;

											}

										}

									}	// end iterate through other resource types

								}

								// Adjust ReqRescs for helitack, if no helicopters or more crews sent than requested
								if ( ReqRescs[6] < 0 || ReqRescs[7] < 0 )
									ReqRescs[7] = 0;

								sum = 0;
								for ( int r = 0; r < 13; r++ )	{

									if ( ReqRescs[r] > 0 )
										sum = sum + ReqRescs[r];

								}

								d++;

							}		// iterate through dispatch locates in FPU

							DIt++;

						}	

					}	// end fill requests for local resources

				}	// end found GACC node


			}	// end if level 2

			if ( Level == 3 )	{

				// Reset the time for the resources to arrive to the morning after the fire
				EscapeTime = Julian * 1440;
				
				// Does the requested resources set include Type II IA Crews?
				if ( ReqRescs[13] > 0  || ReqRescs[14] > 0 )	{

					// Order the GACC Nodes by their Expected Level
					list< CGACCDispatcher* > OrderedGACCs;
					list< CGACCDispatcher* >::iterator GIt;

					vector< OmffrNode< CDispatchBase* >* > InNodes = GetInternalNodes( m_DispTree.Root() );

					for ( int g = 0; g < InNodes.size(); g++ )	{

						CGACCDispatcher* GACC = dynamic_cast< CGACCDispatcher* >( m_DispTree.Element( InNodes[g] ) );

						if ( GACC != 0 )	{

							if ( OrderedGACCs.empty() )
								OrderedGACCs.push_back( GACC );

							else	{

								double ExpectLevel = GACC->GetExpectLevel( Julian );

								GIt = OrderedGACCs.begin();
								bool found = false;

								while ( !found && GIt != OrderedGACCs.end() )	{

									double CompExpectLevel = ( *GIt )->GetExpectLevel( Julian );

									if ( ExpectLevel < CompExpectLevel )
										found = true;

									else
										GIt++;

								}

								OrderedGACCs.insert( GIt, GACC );

							}

						}

					}

					if ( ReqRescs[13] > 0 )	{

						// Have a list of ordered GACCs, fill Type II IA Crews from this list
						GIt = OrderedGACCs.begin();

						while ( ReqRescs[13] > 0 && GIt != OrderedGACCs.end() )	{

							// Get the number of Type II IA Crews at the GACC
							int NumberCrews = ( *GIt )->NumberTypeTwoCrews();

							// Determine the time when the crew will be released after draw down
							int Time = Duration * 1440 + EscapeTime;

							// If there are some Type II Crews
							if ( NumberCrews > 0 )	{

								// Order the crews according to thier next available time
								list< int > OrderedCrews;
								list< int >::iterator CIt = OrderedCrews.begin();

								for ( int c = 0; c < NumberCrews; c++ )	{

									// Get the next available time for the crew
									int Available = ( *GIt )->GetTypeTwoCrew( c ).NextAvailableTime();
									if ( Available < EscapeTime )
										Available = EscapeTime;

									if ( Available < Time )	{		// Place the crew in the ordered crew list

										if ( OrderedCrews.empty() )
											OrderedCrews.push_back( c );

										else	{

											bool found = false;
										
											while ( !found && CIt != OrderedCrews.end() )	{
	
												int CompAvailable = ( *GIt )->GetTypeTwoCrew( *CIt ).NextAvailableTime();
												if ( CompAvailable < EscapeTime )
													CompAvailable = EscapeTime;
	
												if ( Available < CompAvailable )
													found = true;

												else
													CIt++;

											}

											OrderedCrews.insert( CIt, c );

										}

									}
	
								}

								// If there are any crews in the ordered list they can be used for draw down for this fire
								int Size = OrderedCrews.size();

								if ( Size > ReqRescs[13] )
									Size = ReqRescs[13];

								// Deploy these resources
								int c = 0;
								CIt = OrderedCrews.begin();

								while ( c < Size )	{

									// Change the next avialable time for the Type II IA Crew and it's parts
									( *GIt )->SetTypeTwoCrewNextAvailableTime( *CIt, Time );
									( *GIt )->AddtoDrawDownVector( *CIt, EscapeTime, FireNum );

									ReqRescs[13] = ReqRescs[13] - 1;

									CIt++;
									c++;
		
								}		// End of apply drawdown

							} 

							GIt++;

						}

					}	// end fill Type II IA crew requests

					// Uisng the list of ordered GACCs
					if ( ReqRescs[14] > 0 )	{

						GIt = OrderedGACCs.begin();

						while ( ReqRescs[14] > 0 && GIt != OrderedGACCs.end() )	{

							// Get the node for the GACC
							std::map<std::string, OmffrNode<CDispatchBase*>*>::iterator MapIt;
							MapIt = m_InMap.find( ( *GIt )->GetDispatcherID() );

							// Get a vector of all the children to the GACC node
							vector< OmffrNode< CDispatchBase* >* > Nodes = m_DispTree.Children( ( *MapIt ).second );

							// Iterate through all the nodes, find the dispatch locations and determine if there are helicopter at them
							// Order the helicopters by available time
							list< CResource* > OrderedRescs;
							list< CResource* >::iterator RIt;
							int Time = Duration * 1440 + EscapeTime;

							for ( int n = 0; n < Nodes.size(); n++ )	{

								CDLDispatcher* DLDisp = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( Nodes[n] ) );
								if ( DLDisp != 0 )	{

									// Get the resources at this dispatch location and place in list according to next available time
									string DispType = DLDisp->ConvertDispType( 6 );						// Convert the dispatch type to a string
									list< CResource* > LResources = DLDisp->GetByDispType( DispType );	// Get the resources of this type at the dispatch location

									if ( LResources.size() > 0 )								// Were there any resources at the dispatch location
									{
										// Iterate through the list and determine available time
										list< CResource* >::iterator Iter;

										for ( Iter = LResources.begin(); Iter!= LResources.end(); Iter++ )	{

											int Available = ( *Iter )->GetAvailableTime();

											// If the resource is available before the fire is contained place in the ordered list
											if ( Available < Time )	{

												// If the resource is available before the fire escapes
												if ( Available < EscapeTime )
													Available = EscapeTime;

												if ( OrderedRescs.size() == 0 )	
													OrderedRescs.push_back( *Iter );

												bool found = false;
												RIt = OrderedRescs.begin();

												while ( !found && RIt != OrderedRescs.end() )	{
					
													int CompAvailable = (*RIt )->GetAvailableTime();
													if ( CompAvailable < EscapeTime )
														CompAvailable = EscapeTime;

													if ( Available < CompAvailable )
														found = true;

													else
														RIt++;

												}

												// Insert in list
												OrderedRescs.insert( RIt, *Iter );

											}

										}

									}

								}

							}

							// Have an ordered list of GACC level helicopters to draw down from
							int Size = OrderedRescs.size();

							if ( Size > ReqRescs[14] )	
								Size = ReqRescs[14];

							// Apply the draw down
							RIt = OrderedRescs.begin();

							for ( int n = 0; n < Size; n++ )	{
						
								( *RIt )->SetAvailableTime( Time );

								// add to resources draw down vector
								( *RIt )->RecordDrawDown( EscapeTime, FireNum );
											
								// reduce required resources
								ReqRescs[14] = ReqRescs[14] - 1;
								RIt++;

							}

							GIt++;

						}

					}	// end fill Type I Helicopter requests

					// Are there requests for airtankers
					if ( ReqRescs[0] > 0 )	{

						// Iterate through the airtanker bases and find the airtankers that are available and at the lowest Expected Level
						OmffrNode< CDispatchBase* >* Node = m_DispTree.Root();

						CNatDispatcher* NatNode = dynamic_cast< CNatDispatcher* >( m_DispTree.Element( Node ) );

						if ( NatNode != 0 )	{

							// Get the vector of pointers to the airtanker bases
							vector< CDLDispatcher* > VAirtankerBases = NatNode->GetAtDispatchersVector();

							// Iterate through the vector of airtanker bases and find the airtankers that are available and at the base with the lowest expected level
							list< CDLDispatcher* > OrderedAtBases;
							list< CDLDispatcher* >::iterator DLIt;

							for ( int d = 0; d < VAirtankerBases.size(); d++ )	{

								// Is there an airtanker here
								string DispType = VAirtankerBases[d]->ConvertDispType( 0 );						// Convert the dispatch type to a string
								list< CResource* > LResources = VAirtankerBases[d]->GetByDispType( DispType );	// Get the resources of this type at the dispatch location

								if ( LResources.size() > 0 )								// Were there any resources at the dispatch location
								{
									// Order the dispatch location dispatchers with resources by their expected values
									double ExpectLevel = VAirtankerBases[d]->GetExpectLevel( Julian );

									if ( OrderedAtBases.size() == 0 )	
										OrderedAtBases.push_back( VAirtankerBases[d] );

									else	{

										bool found = false;
										DLIt = OrderedAtBases.begin();

										while ( !found && DLIt != OrderedAtBases.end() )	{
					
											double CompExpectLevel = (*DLIt )->GetExpectLevel( Julian );

											if ( ExpectLevel < CompExpectLevel )
												found = true;

											else
												DLIt++;

										}

										// Insert in list
										OrderedAtBases.insert( DLIt, VAirtankerBases[d] );

									}

								}

							}

							// Have a list of airtanker bases, with airtankers, ordered by the expected level for the escape day
							int Size = OrderedAtBases.size();

							if ( Size > 0 )	{	
								
								// Move the airtankers at the base to the base nearest the escape fire

								CFWA FWA = m_VEscapes[ *It ].GetFire().GetFWA();
								string ClosestATBase = FWA.GetClosestATBase();

								std::map<std::string, OmffrNode<CDispatchBase*>*>::iterator MapIt;
								MapIt = m_DispMap.find( ClosestATBase );

								OmffrNode< CDispatchBase* > *Node = ( *MapIt ).second;

								CDLDispatcher *DLDisp = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( Node ) );

								if ( DLDisp != 0 )	{

									DLIt = OrderedAtBases.begin();
							
									while ( ReqRescs[0] > 0 && DLIt != OrderedAtBases.end() )	{
						
										// Go to the airtanker base and get the airtankers located there
										list< CResource* > LResources = ( *DLIt )->GetByDispType( "ATT" );	// Get the resources of this type at the dispatch location
										list< CResource* >::iterator RIt;

										for ( RIt = LResources.begin(); RIt != LResources.end(); RIt++ )	{

											// Add the resource to the resource map of the dispatch location where it is being moved
											DLDisp->InsertInRescMap( *RIt );

											// Remove from the resource map where the resource was located
											( *DLIt )->RemoveFromRescMap( *RIt );

											// Record move in the resources movement history
											int AvailableTime = ( *RIt )->GetAvailableTime();
											if ( AvailableTime < EscapeTime )
												AvailableTime = EscapeTime;

											( *RIt )->RecordMove( AvailableTime, ClosestATBase );	
											
											// reduce required resources
											ReqRescs[0] = ReqRescs[0] - 1;

										}

										DLIt++;

									}

								}

							}

						}	// End national node casting success

						// Fill the requests for other resource types from the local dispatchers in the GACC
						sum = 0;
						for ( int i = 1; i < 13; i++ )	
							sum = sum + ReqRescs[i];

						if ( sum > 0 )	{

							// Determine if need to create the ordered dispatch location lists for the resources
							if ( Julian != OldJulian )	{	

								bool Result = OrderLocalDispatchLocations( Julian, OrderedCRW, OrderedDZR, OrderedENG, OrderedHel, OrderedHELI, OrderedSEAT, FSCRWdd, DOICRWdd, FSDZRdd, DOIDZRdd, FSENGdd, DOIENGdd, FSSEATdd, DOISEATdd );
								OldJulian = Julian;
							}

							list< CDLDispatcher* >::iterator DLIt = OrderedCRW.begin();
							int Time = Duration * 1440 + EscapeTime;
							
							// Draw down crews if applicable
							while ( ReqRescs[1] > 0 && DLIt != OrderedCRW.end() )	{

								string DispType = ( *DLIt )->ConvertDispType( 1 );						// Convert the dispatch type to a string
								list< CResource* > LResources = ( *DLIt )->GetByDispType( DispType );	// Get the resources of this type at the dispatch location

								if ( LResources.size() > 0 )	{			

									// Iterate throuth the resources, check the available time and deploy to escape fire
									list< CResource* >::iterator RIt= LResources.begin();

									while ( ReqRescs[1] > 0 && RIt != LResources.end() )	{

										int Available = ( *RIt )->GetAvailableTime();

										if ( Available <= EscapeTime )	{

											( *RIt )->SetAvailableTime( Time );

											// add to resources draw down vector
											( *RIt )->RecordDrawDown( EscapeTime, FireNum );
										
											// reduce required resources
											ReqRescs[1] = ReqRescs[1] - 1;

										}

										RIt++;

									}

								}

								DLIt++;

							}

							// Draw down dozers if applicable
							DLIt = OrderedDZR.begin();

							while ( ReqRescs[2] > 0 && DLIt != OrderedDZR.end() )	{

								string DispType = ( *DLIt )->ConvertDispType( 2 );						// Convert the dispatch type to a string
								list< CResource* > LResources = ( *DLIt )->GetByDispType( DispType );	// Get the resources of this type at the dispatch location

								if ( LResources.size() > 0 )	{			

									// Iterate throuth the resources, check the available time and deploy to escape fire
									list< CResource* >::iterator RIt= LResources.begin();

									while ( ReqRescs[2] > 0 && RIt != LResources.end() )	{

										int Available = ( *RIt )->GetAvailableTime();

										if ( Available <= EscapeTime )	{

											( *RIt )->SetAvailableTime( Time );

											// add to resources draw down vector
											( *RIt )->RecordDrawDown( EscapeTime, FireNum );
											
											// reduce required resources
											ReqRescs[2] = ReqRescs[2] - 1;

										}

										RIt++;

									}

								}

								DLIt++;

							}

							// Draw down engines if applicable
							DLIt = OrderedENG.begin();

							while ( ReqRescs[3] > 0 && DLIt != OrderedENG.end() )	{

								string DispType = ( *DLIt )->ConvertDispType( 3 );						// Convert the dispatch type to a string
								list< CResource* > LResources = ( *DLIt )->GetByDispType( DispType );	// Get the resources of this type at the dispatch location

								if ( LResources.size() > 0 )	{			

									// Iterate throuth the resources, check the available time and deploy to escape fire
									list< CResource* >::iterator RIt= LResources.begin();

									while ( ReqRescs[3] > 0 && RIt != LResources.end() )	{

										int Available = ( *RIt )->GetAvailableTime();

										if ( Available <= EscapeTime )	{

											( *RIt )->SetAvailableTime( Time );

											// add to resources draw down vector
											( *RIt )->RecordDrawDown( EscapeTime, FireNum );
											
											// reduce required resources
											ReqRescs[3] = ReqRescs[3] - 1;

										}

										RIt++;

									}

								}

								DLIt++;

							}

							// Draw down helicopters if applicable
							DLIt = OrderedHel.begin();

							while ( ReqRescs[6] > 0 && DLIt != OrderedHel.end() )	{

								string DispType = ( *DLIt )->ConvertDispType( 6 );						// Convert the dispatch type to a string
								list< CResource* > LResources = ( *DLIt )->GetByDispType( DispType );	// Get the resources of this type at the dispatch location

								if ( LResources.size() > 0 )	{			

									// Iterate throuth the resources, check the available time and deploy to escape fire
									list< CResource* >::iterator RIt= LResources.begin();

									while ( ReqRescs[6] > 0 && RIt != LResources.end() )	{

										CHelicopter* Helic = dynamic_cast< CHelicopter* >( *RIt );
										
										if ( Helic != 0 )	{

											// If helitack are also deployed find helicopter with helitack attached
											if ( ReqRescs[7] > 0 )	{

												if ( Helic->AttachedCrewsListSize() > 0 )	{

													int Available = Helic->GetHelitackCrewAvailableTime();

													if ( Available <= EscapeTime )	{

														list< CResource* > AttachedCrews = Helic->GetAttachedCrewsList();

														// Reset available time and record drawdown for helitack
														for ( list< CResource* >::iterator ACIt = AttachedCrews.begin(); ACIt != AttachedCrews.end(); ACIt++ )	{

															( *ACIt )->SetAvailableTime( Time );

															( *ACIt )->RecordDrawDown( EscapeTime, FireNum );

															if ( ReqRescs[7] > 0 )
																ReqRescs[7] = ReqRescs[7] - 1;

														}

														// Reset the available time and record drawdown for the helicotper
														( *RIt )->SetAvailableTime( Time );

														// add to resources draw down vector
														( *RIt )->RecordDrawDown( EscapeTime, FireNum );
											
														// reduce required resources
														ReqRescs[6] = ReqRescs[6] - 1;

													}

												}


											}
											
											else	{	//no helitack, use helicopter without attached crews
												
												// Reset the available time and record drawdown for the helicotper
												( *RIt )->SetAvailableTime( Time );

												// add to resources draw down vector
												( *RIt )->RecordDrawDown( EscapeTime, FireNum );
											
												// reduce required resources
												ReqRescs[6] = ReqRescs[6] - 1;

											}

										}

										RIt++;

									}

								}

								DLIt++;

							}
							
							// Draw down SEAT if applicable
							DLIt = OrderedSEAT.begin();

							while ( ReqRescs[8] > 0 && DLIt != OrderedSEAT.end() )	{

								string DispType = ( *DLIt )->ConvertDispType( 8 );						// Convert the dispatch type to a string
								list< CResource* > LResources = ( *DLIt )->GetByDispType( DispType );	// Get the resources of this type at the dispatch location

								if ( LResources.size() > 0 )	{			

									// Iterate throuth the resources, check the available time and deploy to escape fire
									list< CResource* >::iterator RIt= LResources.begin();

									while ( ReqRescs[8] > 0 && RIt != LResources.end() )	{

										int Available = ( *RIt )->GetAvailableTime();

										if ( Available <= EscapeTime )	{

											( *RIt )->SetAvailableTime( Time );

											// add to resources draw down vector
											( *RIt )->RecordDrawDown( EscapeTime, FireNum );
											
											// reduce required resources
											ReqRescs[8] = ReqRescs[8] - 1;

										}

										RIt++;

									}

								}

								DLIt++;

							}

						}	// end fill requests for local resources

					}	// end found GACC node

				}	

			}	// end if level 3

		} // end there are requested resources
	}
}

// Fill the ordered list of dispatch locations 
bool CRunScenario::OrderLocalDispatchLocations( int Julian, list< CDLDispatcher* > &OrderedCRW, list< CDLDispatcher* > &OrderedDZR, 
	list< CDLDispatcher* > &OrderedENG, list< CDLDispatcher* > &OrderedHel, list< CDLDispatcher* > &OrderedHELI, list< CDLDispatcher* > OrderedSEAT, 
	PrepositionStruct FSCRWdd, PrepositionStruct DOICRWdd, PrepositionStruct FSDZRdd, PrepositionStruct DOIDZRdd, 
	PrepositionStruct FSENGdd, PrepositionStruct DOIENGdd, PrepositionStruct FSSEATdd, PrepositionStruct DOISEATdd )

{
	// Values that can change or input from the GUI
	double Cutoff = 0.5;			// The Adjusted Expected Level cut off value for allowing resources to be moved from base
	
	OrderedCRW.clear();
	OrderedDZR.clear();
	OrderedENG.clear();
	OrderedHel.clear();
	OrderedHELI.clear();
	OrderedSEAT.clear();

	list< CDLDispatcher* >::iterator DLIt;

	// Order the FS and DOI Crew Dispatch Locations
	if ( FSCRWdd.level > 0 )
		AddRescTypeDLs( OrderedCRW, m_FSCRWDLs, FSCRWdd.outOfSeason, FSCRWdd.level, Julian, 1 ); 
	if ( DOICRWdd.level > 0 )
		AddRescTypeDLs( OrderedCRW, m_DOICRWDLs, DOICRWdd.outOfSeason, DOICRWdd.level, Julian, 1 );

	// Order the FS and DOI Dozer Dispatch Locations
	if ( FSDZRdd.level > 0 )
		AddRescTypeDLs( OrderedDZR, m_FSDZRDLs, FSDZRdd.outOfSeason, FSDZRdd.level, Julian, 2 ); 
	if ( DOIDZRdd.level > 0 )
		AddRescTypeDLs( OrderedDZR, m_DOIDZRDLs, DOIDZRdd.outOfSeason, DOIDZRdd.level, Julian, 2 );

	// Order the FS and DOI Engine Dispatch Locations
	if ( FSENGdd.level > 0 )
		AddRescTypeDLs( OrderedENG, m_FSENGDLs, FSENGdd.outOfSeason, FSENGdd.level, Julian, 3 ); 
	if ( DOIENGdd.level > 0 )
		AddRescTypeDLs( OrderedENG, m_DOIENGDLs, DOIENGdd.outOfSeason, DOIENGdd.level, Julian, 3 );

	// Order the FS and DOI Local Helicopter Dispatch Locations
	//if ( FSHeldd.level > 0 )
	//	AddRescTypeDLs( OrderedHel, m_FSHelDLs, FSHeldd.outOfSeason, FSHeldd.level, Julian, 6 ); 
	//if ( DOIHeldd.level > 0 )
	//	AddRescTypeDLs( OrderedHel, m_DOIHelDLs, DOIHeldd.outOfSeason, DOIHeldd.level, Julian, 6 );

	// Order the FS and DOI Helitack Dispatch Locations
	//if ( FSHELIdd.level > 0 )
	//	AddRescTypeDLs( OrderedHELI, m_FSHELIDLs, FSHELIdd.outOfSeason, FSHELIdd.level, Julian, 7 ); 
	//if ( DOIHELIdd.level > 0 )
	//	AddRescTypeDLs( OrderedHELI, m_DOIHELIDLs, DOIHELIdd.outOfSeason, DOIHELIdd.level, Julian, 7 );

	// Order the FS and DOI SEAT Dispatch Locations
	if ( FSSEATdd.level > 0 )
		AddRescTypeDLs( OrderedSEAT, m_FSSEATDLs, FSSEATdd.outOfSeason, FSSEATdd.level, Julian, 8 ); 
	if ( DOISEATdd.level > 0 )
		AddRescTypeDLs( OrderedSEAT, m_DOISEATDLs, DOISEATdd.outOfSeason, DOISEATdd.level, Julian, 8 );

	return true;
	
}

// Add the resourcetype's and agency's dispatch locations to the ordered lists 
void CRunScenario::AddRescTypeDLs( list< CDLDispatcher* > &OrderedDLs, vector< string > &DLIDVector, bool OutSeason, double Movability, int Julian, int DispTypeIdx ) 
{
	double Cutoff = 0.5;			// The Adjusted Expected Level cut off value for allowing resources to be moved from base
	
	std::map<std::string, OmffrNode<CDispatchBase*>*>::iterator MapIt;
	list< CDLDispatcher* >::iterator DLIt;
	
	if ( Movability > 0 )	{

		// Iterate through the FS Crew dispatch locations
		for ( int i = 0; i < DLIDVector.size(); i++ )	{

			// Get the pointer to the dispatch location dispatcher
			MapIt = m_DispMap.find( DLIDVector[i] );
			
			if ( MapIt != m_DispMap.end() )	{

				CDLDispatcher* DLDisp = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( ( *MapIt ).second ) );

				if ( DLDisp != 0 )	{

					// Is the OutSeason flag true, if so check the if the dispatch location is out of season
					bool Include = OutSeason && !DLDisp->IsInFireSeason( Julian );
					if ( !OutSeason || Include )	{

						// Is there a resource of the correct type available?
						string DispType = DLDisp->ConvertDispType( DispTypeIdx );
						list< CResource* > LResource = DLDisp->GetByDispType( DispType );
						list< CResource* >::iterator RIt;
						bool Add = false;

						if ( LResource.size() > 0 )	{

							// Iterate through the resources and determine if any are available at 8:00 am
							int Time = ( Julian - 1 ) * 1440;

							for (  RIt = LResource.begin(); RIt != LResource.end(); RIt++ )	{

								int Available = ( *RIt )->GetAvailableTime();

								if ( Available <= Time )	
									Add = true;

							}

						}

						// If the adjusted Expected level is greater than or equal to the cutoff level do not add
						double AdjEL = DLDisp->GetExpectLevel( Julian ) / Movability;

						if ( AdjEL > Cutoff )
							Add = false;

						// if Add then add dispatch location to the ordered DL list
						if ( Add == true )	{

							if ( OrderedDLs.empty() )
								OrderedDLs.push_back( DLDisp );

							else	{

								// Iterate through the Ordered Resource vector and place the dispatch location dispatcher according to the Adjusted Expected Level
								DLIt = OrderedDLs.begin();
								bool found = false;
								bool IDfound = false;

								double AdjEL = DLDisp->GetExpectLevel( Julian ) / Movability;
								string DLID = DLDisp->GetDispatcherID();

								while ( !found && DLIt != OrderedDLs.end() )	{

									double CompAdjEL = ( *DLIt )->GetExpectLevel( Julian ) / Movability;
									string CompDLID = ( *DLIt )->GetDispatcherID();

									if ( DLID == CompDLID )
										IDfound = true;

									if ( AdjEL <= CompAdjEL )
										found = true;

									else
										DLIt++;

								}

								if ( !IDfound )
									OrderedDLs.insert( DLIt, DLDisp );

							}

						}	// end if Add is true

					}

				}

			}
			
		}

	}

}

// Read in the start and end of the fire season the internal nodes
void CRunScenario::ReadFireSeason()
{
	string oFilename = "FireSeason.dat";
	
	//if constructor opens a file
	ifstream inFireSeasonFile( oFilename.c_str(), ios::in );

	//exit program if ifstream could not open file
	if ( !inFireSeasonFile ) {
		cerr << "File could not be opened" << endl;
		//exit( 1 );
	}

	// Enter elements for first FWA 
	string Dispatcher( "Default" );
	int SeasonStart = 1;
	int SeasonEnd = 365;

	std::map<std::string, OmffrNode<CDispatchBase*>*>::iterator MapIt;

	while ( inFireSeasonFile >> Dispatcher >> SeasonStart >> SeasonEnd )	{
		
		// get a pointer to the dispatcher
		MapIt = m_InMap.find( Dispatcher );

		if ( MapIt != m_InMap.end() )	{

			// Set the start and end of the fire season
			OmffrNode< CDispatchBase* >* Node = ( *MapIt ).second;

			m_DispTree.Element( Node )->SetSeasonStart( SeasonStart );
			m_DispTree.Element( Node )->SetSeasonEnd( SeasonEnd );

		}
		
	}

	// Apply these dates to all the dispatch location dispatchers
	for ( int i = 0; i < m_VDispLoc.size(); i++ )	{
	
		string DispLocID = m_VDispLoc[i].GetDispLocID();

		MapIt = m_DispMap.find( DispLocID );

		if ( MapIt != m_DispMap.end() )	{

			OmffrNode< CDispatchBase* >* Node = ( *MapIt ).second;
			OmffrNode< CDispatchBase* >* ParentNode = m_DispTree.Parent( Node );

			// Save the parent's start and end dates for fire season to the dispatch location's dispatcher
			m_DispTree.Element( Node )->SetSeasonStart( m_DispTree.Element( ParentNode )->GetSeasonStart() );
			m_DispTree.Element( Node )->SetSeasonEnd( m_DispTree.Element( ParentNode )->GetSeasonEnd() );

		}

	}

} 

// Return the borrowed resources to their original bases
void CRunScenario::ReturnResources( int Julian )
{
	int GroundTime = 1440;		// Travel time for ground resources excludes the 24 hours for this day
	int AirTime = 0;			// Travel time for aerial resources excludes the 24 hours for this day

	std::set< int > RemovedResources;
	std::map<std::string, OmffrNode<CDispatchBase*>*>::iterator MapIt;
	
	// What is the time since the beginning of the year for the end of the julian day
	int Time = Julian * 1440;

	// Iterate through the borrowed resources and determine if any need to be returned today
	for ( int i = 0; i < m_BorrowedResources.size(); i++ )	{

		int Return = m_BorrowedResources[i]->GetReturnTime();

		if ( Return <= Time )	{

			// Determine the resource type
			int DispType = m_BorrowedResources[i]->GetDispatchType();

			// Determine the dispatch location where the resource is currently located
			string DispLoc = m_BorrowedResources[i]->GetCurrentLocation();
			MapIt = m_DispMap.find( DispLoc );

			if ( MapIt != m_DispMap.end() )	{

				OmffrNode< CDispatchBase* >* Node = ( *MapIt ).second;
				
				CDLDispatcher *DLDisp = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( Node ) );

				if ( DLDisp != 0 )	{

					// Get the home dispatch location for the dispatcher
					string DispBase = m_BorrowedResources[i]->GetDispLoc().GetDispLocID();
					MapIt = m_DispMap.find( DispBase );

					if ( MapIt != m_DispMap.end() )	{

						OmffrNode< CDispatchBase* >* BaseNode = ( *MapIt ).second;
				
						CDLDispatcher *DLDispBase = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( BaseNode ) );

						if ( DLDispBase != 0 )	{

							// Determine if the resource is a regional crew
							if ( DispType == 1 )	{

								OmffrNode< CDispatchBase* >* ParentNode = m_DispTree.Parent( Node );
								int Level = m_DispTree.Depth( ParentNode );

								if ( Level == 1 )	{	// Is a regional crew

									// Get the GACC index for the regional crew
									CGACCDispatcher* GACCDisp = dynamic_cast< CGACCDispatcher* >( m_DispTree.Element( ParentNode ) );

									if ( GACCDisp != 0 )	{

										string CrewID = m_BorrowedResources[i]->GetRescID();
										int Index = GACCDisp->LocateCrewIndex( CrewID );

										if ( Index < 0 )
											cout << "The Type Two Crew: " << CrewID << "could not be located at the borrowing GACC!";

										else	{

											CTypeTwoIACrew TypeTwoCrew(GACCDisp->GetTypeTwoCrew( Index ));

											// Get the GACCDispatcher for the Home Base Node
											OmffrNode< CDispatchBase* >* BaseParentNode = m_DispTree.Parent( BaseNode );
											int Level = m_DispTree.Depth( BaseParentNode );

											if ( Level == 1 )	{

												CGACCDispatcher *ParentBaseGACC = dynamic_cast< CGACCDispatcher* >(m_DispTree.Element( BaseParentNode ) );

												if ( ParentBaseGACC != 0 )	{

													ParentBaseGACC->MoveTypeTwoCrew( TypeTwoCrew, DLDispBase, GACCDisp );
													int AvailableTime = Return + GroundTime;
													Index = ParentBaseGACC->LocateCrewIndex( CrewID );
													ParentBaseGACC->SetTypeTwoCrewNextAvailableTime( Index, AvailableTime );

													m_BorrowedResources[i]->SetIsBorrowed( false );
													m_BorrowedResources[i]->SetReturnTime( -1 );
													m_BorrowedResources[i]->RecordMove( Return, DispBase );

													RemovedResources.insert( i );

												}

											}

										}

									}

								}	// End is Regional Crew

								else	{

								// Have a crew but it is not a regional crew
								DLDispBase->InsertInRescMap( m_BorrowedResources[i] );
								DLDisp->RemoveFromRescMap( m_BorrowedResources[i] );

								int AvailableTime = Return + GroundTime;
								m_BorrowedResources[i]->SetAvailableTime( AvailableTime );
								m_BorrowedResources[i]->SetIsBorrowed( false );
								m_BorrowedResources[i]->SetReturnTime( -1 );
								m_BorrowedResources[i]->RecordMove( Return, DispBase );

								RemovedResources.insert( i );

								}	// End is Local Crew

							}	// End is crew

							if ( DispType == 3 )	{	// Is an engine

								DLDispBase->InsertInRescMap( m_BorrowedResources[i] );
								DLDisp->RemoveFromRescMap( m_BorrowedResources[i] );

								int AvailableTime = Return + GroundTime;
								m_BorrowedResources[i]->SetAvailableTime( AvailableTime );
								m_BorrowedResources[i]->SetIsBorrowed( false );
								m_BorrowedResources[i]->SetReturnTime( -1 );
								m_BorrowedResources[i]->RecordMove( Return, DispBase );

								RemovedResources.insert( i );

							}

							if ( DispType == 6 || DispType == 7 )	{	// Is a helicopter

								DLDispBase->InsertInRescMap( m_BorrowedResources[i] );
								DLDisp->RemoveFromRescMap( m_BorrowedResources[i] );

								int AvailableTime = Return + AirTime;
								m_BorrowedResources[i]->SetAvailableTime( AvailableTime );
								m_BorrowedResources[i]->SetIsBorrowed( false );
								m_BorrowedResources[i]->SetReturnTime( -1 );
								m_BorrowedResources[i]->RecordMove( Return, DispBase );

								RemovedResources.insert( i );

							}

						}

					}

				}

			}

		}	

	}	// End of iterating through the borrowed resources

	if ( RemovedResources.size() > 0 )	{
		vector< CResource* > NewList;
		std::set< int >::iterator it;
		// Need to remove the returned resources from the list
		for ( int i = 0; i < m_BorrowedResources.size(); i++ )	{

			bool found = false;
			it = RemovedResources.find(i);

			if ( it == RemovedResources.end() )
				NewList.push_back( m_BorrowedResources[i] );

		}

		m_BorrowedResources.clear();
		m_BorrowedResources = NewList;
	
	}

}

// Set the prepositioning and drawdown values with out the GUI interface
void CRunScenario::SetPreposition( PrepositionStruct &SATTpp, PrepositionStruct &SRCRWpp, PrepositionStruct &SRHelpp, PrepositionStruct &SFSCRWpp,
		PrepositionStruct &SDOICRWpp, PrepositionStruct &SFSENGpp, PrepositionStruct &SDOIENGpp, PrepositionStruct &SFSCRWdd, 
		PrepositionStruct &SDOICRWdd, PrepositionStruct &SFSDZRdd, PrepositionStruct &SDOIDZRdd, PrepositionStruct &SFSENGdd,
		PrepositionStruct &SDOIENGdd, PrepositionStruct &SFSSEATdd, PrepositionStruct &SDOISEATdd, PrepositionStruct &SFSHELIpp,
		PrepositionStruct &SDOIHELIpp )
{
	ATTpp = SATTpp;
	RCRWpp = SRCRWpp;
	RHelpp = SRHelpp;
	FSCRWpp = SFSCRWpp;
	DOICRWpp = SDOICRWpp;
	FSENGpp = SFSENGpp;
	DOIENGpp = SDOIENGpp;
	FSCRWdd = SFSCRWdd;
	DOICRWdd = SDOICRWdd;
	FSDZRdd = SFSDZRdd;
	DOIDZRdd = SDOIDZRdd;
	FSENGdd = SFSENGdd;
	DOIENGdd = SDOIENGdd;
	FSSEATdd = SFSSEATdd;
	DOISEATdd = SDOISEATdd;
	FSHELIpp = SFSHELIpp;
	DOIHELIpp = SDOIHELIpp;
}

//Run the contain algorithm for a fire with values read from a file
void CRunScenario::RunContainAlgorithmFromFile( string Filename )
{
	// Open the file
	ifstream inFile( Filename.c_str(), ios::in );

	if ( !inFile )
		cout << "Could not open the file with the fire data for the contain algorithm \n";

	else	{
		// Read the information to run the contain algorithm	
		int i;
		string Lable;
		int StartTimeMins = 0;
  		double feReportSize = 0;
		double feReportRate = 0;  
		if ( feReportRate < 0.00001 )
			feReportRate = 0.00001;											// Contain algorithm can not deal with ROS = 0
		double feLWRatio = 0;
		string Tactic = "HEAD";
  		double feAttackDistance = 0;
  		bool simRetry=true;
  		int simMinSteps = 250;
  		int simMaxSteps = 10000;
		int fireESL = 0; 
		int maxFireTime = 0;
  	
		double DiurnalCoeff[24];
		double DiurnalROS[24];

		for(i=0; i<24; i++)	{
			DiurnalCoeff[i] = 0;
			DiurnalROS[i] = 0;
		}

		inFile >> Lable >> StartTimeMins;
		inFile >> Lable >> feReportSize;
		inFile >> Lable >> feReportRate;
		inFile >> Lable >> feLWRatio;
		inFile >> Lable >> Tactic;
		inFile >> Lable >> feAttackDistance;
		inFile >> Lable >> fireESL;
		inFile >> Lable >> maxFireTime;
		inFile >> Lable >> DiurnalCoeff[0];

		DiurnalROS[0] = feReportRate * DiurnalCoeff[0];

		for ( i = 1; i < 24; i++ )	{
			inFile >> DiurnalCoeff[i];
			DiurnalROS[i] = feReportRate * DiurnalCoeff[i];
		}

		inFile >> Lable;
		inFile >> Lable;
		inFile >> Lable;
		inFile >> Lable;
		inFile >> Lable;

		// Enter the Contain Values
		std::list< Sem::ContainResource * > LContainResources;

		double SegStart = 0.0;
		double ProdRate = 0.0;
		double Duration = 0.0;
		char* Description = "Resource";
		string Descript;
				
		while ( inFile >> SegStart >> ProdRate >> Duration >> Descript )	{

			// Make the contain resource
			LContainResources.push_back( new Sem::ContainResource( SegStart, ProdRate, Duration, Sem::LeftFlank, Description, 0.0, 0.0 ));
		}

		inFile.close();

		string SimulationStatus = "";
	
		// Add the resources to the containforce
		std::list< Sem::ContainResource * >::iterator It;
		Sem::ContainForce *cf =  new Sem::ContainForce; 
	
		for ( It = LContainResources.begin(); It != LContainResources.end(); It++ )
			cf->addResource(*It);       
      
		Sem::Contain::ContainTactic ct =  Sem::Contain::RearAttack;
		if ( Tactic == "HEAD" )		
  		ct = Sem::Contain::HeadAttack;

		Sem::ContainSim sim(feReportSize,feReportRate, DiurnalROS, StartTimeMins, feLWRatio, cf, ct, feAttackDistance, simRetry, simMinSteps, simMaxSteps, fireESL, maxFireTime );
		sim.run();

		// Delete LContainResources and cf members - this is handled in the destructor for cf
		delete cf;
	
  	
		Sem::Contain::ContainStatus status = sim.status();

		switch (status)	{
			case 0:	
				SimulationStatus = "Unreported";
				break;
			case 1:
				SimulationStatus = "Reported";
				break;
			case 2:
				SimulationStatus = "Attacked";
				break;
			case 3:
				SimulationStatus = "Contained";
				break;
			case 4:
				SimulationStatus = "Overrun";
				break;
			case 5:
				SimulationStatus = "Exhausted";
				break;
			case 6:
				SimulationStatus = "Overflow";
				break;
			case 7:
				SimulationStatus = "SizeLimitExceeded";
				break;
			case 8:
				SimulationStatus = "TimeLimitExceeded";
				break;
			default:
				SimulationStatus = "Oops Something went wrong";
				break;
		}
			

				 
  		double ffc=sim.finalFireCost();
  		double ffl=sim.finalFireLine();
  		double ffp=sim.finalFirePerimeter();
		double ffsz=sim.finalFireSize();
		double ffsw=sim.finalFireSweep();
		double fft=sim.finalFireTime();
  		double fru=sim.finalResourcesUsed();

		// Output the Results
		ofstream outFile( "/Cprogramming/Helitack/ContainRun.dat", ios::out );

		if ( !outFile )
			cout << "The output file for the contain run did not open \n";

		else	{ 

			outFile << "Output For: " << Filename << endl;
			outFile << "Final Cost: " << ffc << endl; 
			outFile << "Final Line: " << ffl << endl;
			outFile << "Final Perimeter: " << ffp << endl;
			outFile << "Final Size: " << ffsz << endl;
			outFile << "Final Sweep: " << ffsw << endl;
			outFile << "Final Time: " << fft << endl;
			outFile << "Final Resources: " << fru << endl;
			outFile << SimulationStatus << endl;

			outFile.close();

		}

	}

}

// Assign helitack crews to a helicopter
//bool CRunScenario::AssignHelitack( std::unordered_multimap< string, CResource* > ResourceMap )
bool CRunScenario::AssignHelitack( std::multimap< string, CResource* > ResourceMap )
{
	// Open the file containing the assignments
	ifstream inFile( "/Cprogramming/Dispatcher/Dispatcher/AttachedHelic.dat", ios::in );

	if ( !inFile )	{
		cout << "Could not open the file attaching the helitack crews to a helicopter \n";
		return false;
	}

	else	{

		// Iterators to the helicopter and helitack resources
		//std::unordered_multimap< string, CResource* >::iterator ItHelic;
		//std::unordered_multimap< string, CResource* >::iterator ItHeli;
		std::multimap< string, CResource* >::iterator ItHelic;
		std::multimap< string, CResource* >::iterator ItHeli;

		// Read in the helicopter ID,  helitack crew ID pairs	
		string HelicopterID = "";
		string HelitackID = "";

		while 	( inFile >> HelicopterID >> HelitackID )	{
			
			// Check if both are good
			bool Good = true;

			ItHelic = ResourceMap.find( HelicopterID );

			if ( ItHelic == ResourceMap.end() )
				Good = false;

			if ( ResourceMap.count( HelicopterID ) > 1 ) 
				cout << "There are more than one helicotpers with the ID \n";

			ItHeli = ResourceMap.find( HelitackID );

			if ( ItHeli == ResourceMap.end() )
				Good = false;

			if ( ResourceMap.count( HelitackID ) > 1 )
				cout << "There are more than one helitack crew with the ID \n";

			// If both values are good then assign the helitack crew to the helicotper
			if ( Good )	{

				CHelicopter* HelicPtr = dynamic_cast< CHelicopter* >( ItHelic->second );

				if ( HelicPtr != 0 )	
					HelicPtr->AddAttachedCrew( ItHeli->second );

			}

		}

	}

	inFile.close();
	return true;

}

// Assign alternate dispatch locations for helicopters to the nodes for dispatch location dispatchers
bool CRunScenario::AltHelicDLs( std::map<std::string, OmffrNode<CDispatchBase*>*> DLDispMap )
{
	// Use the dispatch map to assign alternate dispatch locations for helicopters to a dispatch location
	// Open the file containing the assignments
	ifstream inFile( "HeliBases.dat", ios::in );

	if ( !inFile )	{
		cout << "Could not open the file containing alternate helibases for helicopters to deploy helitack \n";
		return false;
	}

	else	{

		// Iterators to the Dispatch Location and Alternate dispatch location
		std::map< string, OmffrNode< CDispatchBase* >* >::iterator ItBase;
		std::map< string, OmffrNode< CDispatchBase* >* >::iterator ItAlt;

		// Read in the helicopter ID,  helitack crew ID pairs	
		string BaseID = "";
		string AltID = "";

		while 	( inFile >> BaseID >> AltID )	{
			
			// Check if both are good
			bool Good = true;

			ItBase = DLDispMap.find( BaseID );

			if ( ItBase == DLDispMap.end() )
				Good = false;

			ItAlt = DLDispMap.find( AltID );

			if ( ItAlt == DLDispMap.end() )
				Good = false;

			// If both values are good then assign the helitack crew to the helicotper
			if ( Good )	{

				CDLDispatcher* BasePtr = dynamic_cast< CDLDispatcher* >(  m_DispTree.Element( ItBase->second ) );
				CDLDispatcher* AltPtr = dynamic_cast< CDLDispatcher* >( m_DispTree.Element( ItAlt->second ) );

				if ( BasePtr != 0 && AltPtr != 0 )	
					BasePtr->AddtoAltHelicopterDLsList( AltPtr );

				else
					cout << "Did not get good DL pointers for adding alternate helicopter bases\n";

			}

			else
                            (void)Good;
				//cout << "Did not find dispatch locations to add alternate helicopter bases\n";

		}

	}

	inFile.close();
	return true;

}


/**
 * \brief Get the total usage of each resource by type for the year.
 *
 * \return a vector with 13 values, the count in minutes for each type of
 *         resource in the dispatch logic.
 */
std::vector<int> CRunScenario::GetResourceUsage()
{
    std::vector<CResource*>::iterator it;
    it = m_VResource.begin();
    std::vector<int> usage(13);
    int minutes;
    for( int i = 0; i < 13; i++ )
    {
        usage[i] = 0;
    }
    int j = 0;
    int size = 0;
    for( ; it < m_VResource.end(); it++ )
    {
        minutes = 0;
        size = (*it)->WorkYearSize();
        for(j = 0;j < size;j++)
        {
            minutes += (*it)->GetWorkYearEntry(j).GetEndTime() - (*it)->GetWorkYearEntry(j).GetStartTime();
        }
        usage[(*it)->GetDispatchType()] += minutes;
    }
    return usage;
}

/**
 * \brief Get the usage of all resources.
 *
 * \return a map with one entry per resource by id, and minutes worked.
 */

std::map<std::string, double> CRunScenario::GetSingleResourceUsage()
{
    std::vector<CResource*>::iterator it;
    it = m_VResource.begin();
    std::map<std::string, double> usage;
    int minutes;
    int j = 0;
    int size = 0;
    int nStartDay, nEndDay;
    int nDays;
    for( ; it < m_VResource.end(); it++ )
    {
        usage[(*it)->GetRescID()] = 0;
        minutes = 0;
        size = (*it)->WorkYearSize();
        /*for(j = 0;j < size;j++)
        {
            minutes += (*it)->GetWorkYearEntry(j).GetEndTime() - (*it)->GetWorkYearEntry(j).GetStartTime();
        }*/
		minutes = (*it)->SumRescWorkYear();
        nStartDay = (*it)->GetStartSeason();
        nEndDay = (*it)->GetEndSeason();
        nDays = nEndDay - nStartDay;
        if( nDays < 0 )
            nDays = 365 - nStartDay + nEndDay;
        usage[(*it)->GetRescID()] += minutes / nDays;
    }
    return usage;
}

// Update the count for the resources deployed from the DLDispatcher for the day
bool CRunScenario::UpdateDLDispatcherDispatchCount(string dispLocId, int Julian)
{
	// Get the DLdispatcher for the dispatch location
	map< string, OmffrNode< CDispatchBase* >* >::iterator It = m_DispMap.find( dispLocId );

	OmffrNode< CDispatchBase* > *Node;

	if ( It != m_DispMap.end() )	{
		Node = It->second;

		// Set the values for the internal node
		m_DispTree.Element( Node )->AddDailyDispatch(Julian);

		return true;
	}

	else
		return false;
}

#if defined(OMFFR) && defined(__GNUC__)
#pragma GCC pop
#endif /* defined(OMFFR) && defined(__GNUC__) */

