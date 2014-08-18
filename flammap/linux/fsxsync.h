//------------------------------------------------------------------------------
//
//  FarsiteEvents
//
//------------------------------------------------------------------------------

#ifndef  FARSITE_SYNCEVENTS
#define  FARSITE_SYNCEVENTS

//#include <process.h>
//#include <windows.h>

#define EVENT_BURN 		1
#define EVENT_MOIST 	2
#define EVENT_BURNUP	3
#define EVENT_INTEG		4
#define EVENT_BURN_THREAD	5
#define EVENT_MOIST_THREAD 	6
#define EVENT_BURNUP_THREAD	7
#define EVENT_INTEG_THREAD	8
#define EVENT_CROSS           9
#define EVENT_CROSS_THREAD    10

//int instanceID = 0;

class FarsiteEvent
{
	//HANDLE *hEvent;
     unsigned int NumEvents;

public:
     FarsiteEvent();
     ~FarsiteEvent();
     bool AllocEvents(int numevents, char *basename, bool ManReset, bool InitState);
     bool FreeEvents();

	//HANDLE GetEvent(int ThreadNum);
     bool SetEvent(int ThreadNum);
     bool ResetEvent(int ThreadNum);
     bool WaitForEvents(int numevents, bool All, unsigned int Wait);
     bool WaitForOneEvent(int ThreadNum, unsigned int Wait);
};


/*bool 	AllocFarsiteEvents(int EventNum, int numevents, char *basename, bool ManReset, bool InitState);
bool		FreeFarsiteEvents(int EventNum);
bool 	SetFarsiteEvent(int EventNum, int ThreadNum);
bool 	ResetFarsiteEvent(int EventNum, int ThreadNum);
bool 	WaitForFarsiteEvents(int EventNum, int NumEvents, bool All, unsigned int Wait);
bool 	WaitForOneFarsiteEvent(int EventNum, int ThreadNum, unsigned int Wait);
HANDLE 	GetFarsiteEvent(int EventNum, int ThreadNum);

HANDLE	GetLandscapeSemaphore();
bool 	CreateLandscapeSemaphore();
void 	CloseLandscapeSemaphore();*/



#endif // FARSITE_SYNCEVENTS
