//------------------------------------------------------------------------------
//
//  FarsiteEvents
//
//------------------------------------------------------------------------------

#ifndef  FARSITE_SYNCEVENTS
#define  FARSITE_SYNCEVENTS

#include <process.h>
#include <windows.h>

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


class FarsiteEvent
{
	HANDLE *hEvent;
     unsigned long NumEvents;

public:
     FarsiteEvent();
     ~FarsiteEvent();
     bool AllocEvents(long numevents, char *basename, bool ManReset, bool InitState);
     bool FreeEvents();

	HANDLE GetEvent(long ThreadNum);
     bool SetEvent(long ThreadNum);
     bool ResetEvent(long ThreadNum);
     bool WaitForEvents(long numevents, bool All, unsigned long Wait);
     bool WaitForOneEvent(long ThreadNum, unsigned long Wait);
};


bool 	AllocFarsiteEvents(long EventNum, long numevents, char *basename, bool ManReset, bool InitState);
bool		FreeFarsiteEvents(long EventNum);
bool 	SetFarsiteEvent(long EventNum, long ThreadNum);
bool 	ResetFarsiteEvent(long EventNum, long ThreadNum);
bool 	WaitForFarsiteEvents(long EventNum, long NumEvents, bool All, unsigned long Wait);
bool 	WaitForOneFarsiteEvent(long EventNum, long ThreadNum, unsigned long Wait);
HANDLE 	GetFarsiteEvent(long EventNum, long ThreadNum);

HANDLE	GetLandscapeSemaphore();
bool 	CreateLandscapeSemaphore();
void 	CloseLandscapeSemaphore();



#endif // FARSITE_SYNCEVENTS
