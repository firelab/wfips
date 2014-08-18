// DispLogic.h
// class that holds the different dispatch logics for the FWAs
// 10/11 for IRS


#ifndef DISPLOGIC_H
#define DISPLOGIC_H

#include <string>													//class containing definitions for string objects
using std::string;
								
#include <vector>													//needed so have access to = operator for arrays


class CDispLogic	{

public:
#ifdef ENABLE_IRS_TESTING
        CDispLogic(){}
#endif
	CDispLogic( string logicid, string index, int nlevels, int breakpoints[5], int rescnums[13][5] );		//constructor
	CDispLogic( const CDispLogic &displogic );
	~CDispLogic();													//destructor

	void SetLogicID( string logicid );								//set the dispatch logic id
	string GetLogicID();											//get the dispatch logic id
	void SetIndex( string index );									//set the index for the dispatch logic
	string GetIndex();												//get the index for the dispatch logic
	void SetNumLevels( int levels );								//set the number of dispatch levels
	int GetNumLevels();												//get the number of dispatch levels
	void SetBreakPoints( std::vector< int > breakpt );				//set the breakpoints for the dispatch levels
	int GetBreakPoint( int i );										//get the breakpoints for the dispatch levels
	void SetRescNums( std::vector< int > rescnums );				//set the maximum number of resources for each dispatch level
	int GetRescNum( int i, int j );									//get the maximum number of resources for each dispatch level

	void PrintDispLogic();											//print the dispatch logic
	

	
private:
	string m_LogicID;												//dispatch logic id
	string m_Index;													//index to be used for the dispatch logic (ERC, BI, ROS)
	int m_NumLevels;												//number of levels in the dispatch logic
	//int m_BreakPoints[5];											//maximum index value for each dispatch level m_BreakPoints[4] = infinity
	//int m_RescNums[13][5];										//maximum number of each resource type for each dispatch level
	std::vector< int > m_BreakPoints;								//array written as vector so can use operator= (copy constructor)
	std::vector< int > m_RescNums;									//array written as vector so can use operator= (copy constructor)

};	// end class CDispLogic

#endif		// DISPLOGIC_H
