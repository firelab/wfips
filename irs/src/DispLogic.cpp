// DispLogic.cpp
// CDispLogic member function definitions
//	10/11 for IRS

#include <iostream>
using namespace::std;

#include <fstream>											//contains functions for inputting and outputting to a file
using std::ifstream;										//for inputting from a file

#include <string>
using std::string;

#include "DispLogic.h"

// Constructor for class CDispLogic
CDispLogic::CDispLogic( string logicid, string index, int nlevels, int breakpoints[5], int rescnum[13][5] )
{
	m_LogicID = logicid;											//initialize dispatch logic id
	m_Index = index;												//initialize index to be used for the dispatch logic (ERC, BI, ROS)
	m_NumLevels = nlevels;											//initialize number of levels in the dispatch logic

	for ( int i = 0; i < 5; i++ )	{
		m_BreakPoints.push_back( breakpoints[i] );							//initialize the maximum index value for each dispatch level m_BreakPoints[4] = infinity
	}
	
	for ( int i = 0; i < 13; i++ )	{
		for ( int j = 0; j < 5; j++ )	{
			m_RescNums.push_back( rescnum[i][j]);							//initialize the maximum number of each resource type for each dispatch level
	
		}
	}
	
}

// Copy constructor for CDispLogic
CDispLogic::CDispLogic( const CDispLogic &displogic )	
{
	m_LogicID = displogic.m_LogicID;
	m_Index = displogic.m_Index;
	m_NumLevels = displogic.m_NumLevels;
	m_BreakPoints = displogic.m_BreakPoints;
	m_RescNums = displogic.m_RescNums;

}

// Destructor for CDispLogic
CDispLogic::~CDispLogic() {}

// Set the dispatch logic id
void CDispLogic::SetLogicID( string logicid )
{	m_LogicID = logicid;	}

// Get the dispatch logic id
string CDispLogic::GetLogicID()
{	return m_LogicID;	}

// Set the index type for the dispatch logic
void CDispLogic::SetIndex( string index )
{	m_Index = index;	}

// Get the index type for the dispatch logic
string CDispLogic::GetIndex()
{	return m_Index;	}

// Set the number of levels for the dispatch logic
void CDispLogic::SetNumLevels( int nlevels )
{
	// Must be between 3 and 5
	if ( nlevels < 3 || nlevels > 5 )
		nlevels = 5;
	m_NumLevels = nlevels;
}

// Get the number of levels for the dispatch logic
int CDispLogic::GetNumLevels()
{	return m_NumLevels;	}

// Set the breakpoints for the dispatch logic
void CDispLogic::SetBreakPoints( std::vector< int > breakpoints )
{
	m_BreakPoints = breakpoints;
}

// Get a breakpoint for the disptach logic for level i
int CDispLogic::GetBreakPoint( int i)
{ return m_BreakPoints[i];	}

// Set the resource numbers for the dispatch logic
void CDispLogic::SetRescNums( std::vector< int > rescnums )
{	m_RescNums = rescnums;	}

// Get the resource number for resource type i and dispatch level j for dispatch logic
int CDispLogic::GetRescNum( int i, int j)
{	
	int k = i * 5 + j;
	return m_RescNums[k];	}


// Print the dispatch logic information
void CDispLogic::PrintDispLogic()
{
	cout << "Dispatch Logic ID: " << m_LogicID << "\n";
	cout << "	Dispatch Logic Index: " << m_Index << "\n";
	cout << "	Number of Levels: " << m_NumLevels << "\n";
	cout << "	Break Points: ";
	for ( int i = 0; i < m_NumLevels; i++ )
			cout << m_BreakPoints[i] << " ";
	cout << "\n Maximum Number of Resources for each Type: ";
	cout << "\n		ATT: ";
	for ( int j = 0; j < m_NumLevels; j++ )
			cout << m_RescNums[j] << " ";
	cout << "\n		CRW: ";
	for ( int j = 0; j < m_NumLevels; j++ )
			cout << m_RescNums[1*5+j] << " ";
	cout << "\n		DZR: ";
	for ( int j = 0; j < m_NumLevels; j++ )
			cout << m_RescNums[2*5+j] << " ";
	cout << "\n		ENG: ";
	for ( int j = 0; j < m_NumLevels; j++ )
			cout << m_RescNums[3*5+j] << " ";
	cout << "\n		FBDZ: ";
	for ( int j = 0; j < m_NumLevels; j++ )
			cout << m_RescNums[4*5+j] << " ";
	cout << "\n		FRBT: ";
	for ( int j = 0; j < m_NumLevels; j++ )
			cout << m_RescNums[5*5+j] << " ";
	cout << "\n		Helicopter: ";
	for ( int j = 0; j < m_NumLevels; j++ )
			cout << m_RescNums[6*6+j] << " ";
	cout << "\n		HELI: ";
	for ( int j = 0; j < m_NumLevels; j++ )
			cout << m_RescNums[7*5+j] << " ";
	cout << "\n		SCPSEAT: ";
	for ( int j = 0; j < m_NumLevels; j++ )
			cout << m_RescNums[8*5+j] << " ";
	cout << "\n		SJAC: ";
	for ( int j = 0; j < m_NumLevels; j++ )
			cout << m_RescNums[9*5+j] << " ";
	cout << "\n		SMJR: ";
	for ( int j = 0; j < m_NumLevels; j++ )
			cout << m_RescNums[10*5+j] << " ";
	cout << "\n		TP: ";
	for ( int j = 0; j < m_NumLevels; j++ )
			cout << m_RescNums[11*5+j] << " ";
	cout << "\n		WT: ";
	for ( int j = 0; j < m_NumLevels; j++ )
			cout << m_RescNums[12*5+j] << " ";
	cout << "\n";
}





