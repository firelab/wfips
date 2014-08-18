// ground.h
// abstract class for ground resources that inherets from CResource
// 10/11 for IRS

#ifndef GROUND_H
#define GROUND_H

#include "Rescbase.h"											//include the base class for CGround, CResource class definition file
#include "Fire.h"												//include the class CFire containing fires

class CGround : public CResource	{

public:
	
	CGround( string rescID, CRescType &resctype, int staffing, string starttime, string endtime, string startdayofweek, 
		string enddayofweek, int startseason, int endseason, CDispLoc &dispatchloc, int percentavail, double daily, double hourly );		//constructor for ground resources
	CGround( const CGround &ground );
	virtual ~CGround();													//destructor
	
	virtual int CalcInitArrival( CFire fire, double distance );					//calculate the initial arrival time for the resource
	virtual bool IsRescAvailable( CFire fire );					//is the resource available?
	

}; //	end  class CGround

#endif //	GROUND_H