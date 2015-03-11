// NatDispatcher.h
// Function definitions for class CNatDispatcher for National dispatchers derived from base class CDispatchBase
// for IRS 1/13

#ifndef NatDispatcher_H
#define NatDispatcher_H

#include <string>											//C++ standard string class
using std::string;
#include <list>
#include <vector>
#include <map>												//map class template definitions

// include resource information from class CRescBase.h
#include "Rescbase.h"
// Include definition for class CDispatchBase from DispatchBase.h
#include "DispatchBase.h"
#include "DLDispatcher.h"


class CNatDispatcher : public CDispatchBase {

public:
	CNatDispatcher();										//CNatDispatcher default constructor
	CNatDispatcher( string dispatcherID );
	CNatDispatcher( const CNatDispatcher &rhs);	//copy constructor
	virtual ~CNatDispatcher();								//destructor

	// function definitions for set and get functions for class member variables
	void SetAirtankerVector( vector< CResource* > VAirtankers );	//set the vector of airtankers with a vector of airtanker pointers
	vector< CResource* > GetAirtankerVector();				//get the vector of airtanker pointers
	void AddAirtankerToVector( CResource* Airtanker );		//add a CResource pointer for an airtanker to the vector
	CResource* GetAirtankerByIndex( int index );			//get a CResource pointer for an airtanker by the vector index
	void SetATDispatchersVector( vector< CDLDispatcher* > VATDispatcherPtrs );	// set the vector of airtanker bases with a vector of dispatch location dispatcher pointers
	vector< CDLDispatcher* > GetAtDispatchersVector();		//get the vector of pointers to airtanker bases
	void AddATDispatcher( CDLDispatcher* Base );			//add a CDLDispatcher pointer for an airtanker base
	CDLDispatcher* GetATDispatcherByIndex( int index );		//get a CDLDispatcher pointer for an airtanker base by index
	
	virtual vector< int > DetermineCurRescLevel();			//abstract - determine the current level by resource type
	
	
private:
	
	vector< CResource* > m_VAirtankers;						//vector of the airtankers in the analysis
	vector< CDLDispatcher* > m_VATDispatcherPtrs;			//vector of pointers to the airtanker bases

	CNatDispatcher& operator=(const CNatDispatcher &rhs){};	//assignment operator in private because of vectors of airtankers and airtanker dispatch locations
	

};		// end of class CNatDispatcher

#endif   // NatDISPATCHER_H