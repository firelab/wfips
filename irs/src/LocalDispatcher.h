// LocalDispatcher.h
// Function definitions for class CLocalDispatcher for Local Area dispatchers derived from base class CDispatchBase
// for IRS 1/13

#ifndef LocalDispatcher_H
#define LocalDispatcher_H

#include <string>											//C++ standard string class
using std::string;
#include <list>
#include <vector>

// Include definition for class CDispatchBase from DispatchBase.h
#include "DispatchBase.h"


class CLocalDispatcher : public CDispatchBase {

public:
	CLocalDispatcher();											//CLocalDispatcher default constructor
	CLocalDispatcher( string dispatcherID );
	CLocalDispatcher( const CLocalDispatcher &localdispatcher);	//copy constructor
	virtual ~CLocalDispatcher();								//destructor

	CLocalDispatcher& operator=(const CLocalDispatcher &rhs);	//assignment operator
		
	// function definitions for set and get functions for class member variables
	
	virtual vector< int > DetermineCurRescLevel();			//abstract - determine the current level by resource type
	
	
private:
	
	
	

};		// end of class CGACCDispatcher

#endif   // GACCDISPATCHER_H