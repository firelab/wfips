// RescType.h
// Header file for resource type information
// created 10/11 for IRS

//#include <iostream>									//contains input and output functions
#include <string>										//contains functions of operations on strings
using std::string;

#ifndef RESCTYPE_H
#define RESCTYPE_H

class CRescType {
	
public:
	CRescType();									//default constructor
	CRescType( string Type, int Avg_Speed, int Disp_Delay, int Resp_Delay, int Setup_Delay);		//constructor for resource type
	CRescType( const CRescType &rhs );
	~CRescType();									//destructor

	void SetRescType( string resctype );			//set the resource type
	string GetRescType();							//get the resource type
	void SetAvgSpeed( int speed );					//set the average speed for the resource
	int GetAvgSpeed();								//get the average speed for the resource
	void SetDispatchDelay( int delay );				//set the dispatch delay
	int GetDispatchDelay();							//get the dispatch delay
	void SetResponseDelay( int delay );				//set the response delay
	int GetResponseDelay();							//get the response delay
	void SetSetupDelay( int delay );				//set the setup delay
	int GetSetupDelay();							//get the setupdelay


	void PrintRescType();						//print information about resource type
	int PreConstructDelay();					//calculate portion of delay for Dispatch, Response, and Set-up delays

	
private:
	string m_Type;									//resource type
	int m_AvgSpeed;								//average travel speed for resource type
	int m_DispatchDelay;								//dispatch delay for resource type
	int m_ResponseDelay;								//response delay for resource type
	int m_SetupDelay;								//set-up delay for resource type

};		//end class CRescType

#endif
