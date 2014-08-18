//------------------------------------------------------------------------------
//
//  Event Functions
//
//------------------------------------------------------------------------------

#include "fsxsync.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

FarsiteEvent 	hBurnEvent;
FarsiteEvent 	hMoistEvent;
FarsiteEvent   hBurnupEvent;
FarsiteEvent	hIntegEvent;
FarsiteEvent   hCrossEvent;
FarsiteEvent 	hBurnThreadEvent;
FarsiteEvent 	hMoistThreadEvent;
FarsiteEvent   hBurnupThreadEvent;
FarsiteEvent	hIntegThreadEvent;
FarsiteEvent	hCrossThreadEvent;


bool AllocFarsiteEvents(long EventNum, long numevents, char *basename, bool ManReset, bool InitState)
{
	bool ret;

     switch(EventNum)
     {	case 1: ret=hBurnEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 2: ret=hMoistEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 3: ret=hBurnupEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 4: ret=hIntegEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 5: ret=hBurnThreadEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 6: ret=hMoistThreadEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 7: ret=hBurnupThreadEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 8: ret=hIntegThreadEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 9: ret=hCrossEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     	case 10: ret=hCrossThreadEvent.AllocEvents(numevents, basename, ManReset, InitState); break;
     }

     return ret;
}

bool	FreeFarsiteEvents(long EventNum)
{
	bool ret;

     switch(EventNum)
     {	case 1: ret=hBurnEvent.FreeEvents(); break;
     	case 2: ret=hMoistEvent.FreeEvents(); break;
     	case 3: ret=hBurnupEvent.FreeEvents(); break;
     	case 4: ret=hIntegEvent.FreeEvents(); break;
     	case 5: ret=hBurnThreadEvent.FreeEvents(); break;
     	case 6: ret=hMoistThreadEvent.FreeEvents(); break;
     	case 7: ret=hBurnupThreadEvent.FreeEvents(); break;
     	case 8: ret=hIntegThreadEvent.FreeEvents(); break;
     	case 9: ret=hCrossEvent.FreeEvents(); break;
     	case 10: ret=hCrossThreadEvent.FreeEvents(); break;
     }

     return ret;
}


bool SetFarsiteEvent(long EventNum, long ThreadNum)
{
	bool ret;

     switch(EventNum)
     {	case 1: ret=hBurnEvent.SetEvent(ThreadNum); break;
     	case 2: ret=hMoistEvent.SetEvent(ThreadNum); break;
     	case 3: ret=hBurnupEvent.SetEvent(ThreadNum); break;
     	case 4: ret=hIntegEvent.SetEvent(ThreadNum); break;
     	case 5: ret=hBurnThreadEvent.SetEvent(ThreadNum); break;
     	case 6: ret=hMoistThreadEvent.SetEvent(ThreadNum); break;
     	case 7: ret=hBurnupThreadEvent.SetEvent(ThreadNum); break;
     	case 8: ret=hIntegThreadEvent.SetEvent(ThreadNum); break;
     	case 9: ret=hCrossEvent.SetEvent(ThreadNum); break;
     	case 10: ret=hCrossThreadEvent.SetEvent(ThreadNum); break;
     }

     return ret;
}

bool ResetFarsiteEvent(long EventNum, long ThreadNum)
{
	bool ret;

     switch(EventNum)
     {	case 1: ret=hBurnEvent.ResetEvent(ThreadNum); break;
     	case 2: ret=hMoistEvent.ResetEvent(ThreadNum); break;
     	case 3: ret=hBurnupEvent.ResetEvent(ThreadNum); break;
     	case 4: ret=hIntegEvent.ResetEvent(ThreadNum); break;
     	case 5: ret=hBurnThreadEvent.ResetEvent(ThreadNum); break;
     	case 6: ret=hMoistThreadEvent.ResetEvent(ThreadNum); break;
     	case 7: ret=hBurnupThreadEvent.ResetEvent(ThreadNum); break;
     	case 8: ret=hIntegThreadEvent.ResetEvent(ThreadNum); break;
     	case 9: ret=hCrossEvent.ResetEvent(ThreadNum); break;
     	case 10: ret=hCrossThreadEvent.ResetEvent(ThreadNum); break;
     }

     return ret;
}

bool WaitForFarsiteEvents(long EventNum, long numevents, bool All, unsigned long Wait)
{
	bool ret;

     switch(EventNum)
     {	case 1: ret=hBurnEvent.WaitForEvents(numevents, All, Wait); break;
     	case 2: ret=hMoistEvent.WaitForEvents(numevents, All, Wait); break;
     	case 3: ret=hBurnupEvent.WaitForEvents(numevents, All, Wait); break;
     	case 4: ret=hIntegEvent.WaitForEvents(numevents, All, Wait); break;
     	case 5: ret=hBurnThreadEvent.WaitForEvents(numevents, All, Wait); break;
     	case 6: ret=hMoistThreadEvent.WaitForEvents(numevents, All, Wait); break;
     	case 7: ret=hBurnupThreadEvent.WaitForEvents(numevents, All, Wait); break;
     	case 8: ret=hIntegThreadEvent.WaitForEvents(numevents, All, Wait); break;
     	case 9: ret=hCrossEvent.WaitForEvents(numevents, All, Wait); break;
     	case 10: ret=hCrossThreadEvent.WaitForEvents(numevents, All, Wait); break;
     }

     return ret;
}


bool WaitForOneFarsiteEvent(long EventNum, long ThreadNum, unsigned long Wait)
{
	bool ret;

     switch(EventNum)
     {	case 1: ret=hBurnEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 2: ret=hMoistEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 3: ret=hBurnupEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 4: ret=hIntegEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 5: ret=hBurnThreadEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 6: ret=hMoistThreadEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 7: ret=hBurnupThreadEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 8: ret=hIntegThreadEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 9: ret=hCrossEvent.WaitForOneEvent(ThreadNum, Wait); break;
     	case 10: ret=hCrossThreadEvent.WaitForOneEvent(ThreadNum, Wait); break;
     }

     return ret;
}


HANDLE GetFarsiteEvent(long EventNum, long ThreadNum)
{
     HANDLE ret;

     switch(EventNum)
     {	case 1: ret=hBurnEvent.GetEvent(ThreadNum); break;
     	case 2: ret=hMoistEvent.GetEvent(ThreadNum); break;
     	case 3: ret=hBurnupEvent.GetEvent(ThreadNum); break;
     	case 4: ret=hIntegEvent.GetEvent(ThreadNum); break;
     	case 5: ret=hBurnThreadEvent.GetEvent(ThreadNum); break;
     	case 6: ret=hMoistThreadEvent.GetEvent(ThreadNum); break;
     	case 7: ret=hBurnupThreadEvent.GetEvent(ThreadNum); break;
     	case 8: ret=hIntegThreadEvent.GetEvent(ThreadNum); break;
     	case 9: ret=hCrossEvent.GetEvent(ThreadNum); break;
     	case 10: ret=hCrossThreadEvent.GetEvent(ThreadNum); break;
     }

     return ret;
}


//------------------------------------------------------------------------------
//
//  FarsiteEvents
//
//------------------------------------------------------------------------------


FarsiteEvent::FarsiteEvent()
{
	hEvent=0;
     NumEvents=0;
}


FarsiteEvent::~FarsiteEvent()
{
	FreeEvents();
}


bool FarsiteEvent::AllocEvents(long numevents, char *basename, bool ManReset, bool InitState)
{
     if(numevents==NumEvents)
     	return true;

	FreeEvents();

     bool NoErr=true;
     long i;
     char Name[128]="";

     hEvent=new HANDLE[numevents];
	if(hEvent==NULL)
     	return false;

     for(i=0; i<numevents; i++)
     {    sprintf(Name, "%s_%02ld", basename, i+1);

     	hEvent[i]=CreateEvent(NULL, ManReset, InitState, reinterpret_cast<LPCSTR>(Name));
          if(hEvent[i]==NULL)
          {	NoErr=true;
               break;
          }
          NumEvents++;
     }


     return NoErr;
}


bool FarsiteEvent::FreeEvents()
{
	unsigned long i;
     bool NoErr=true;

     for(i=0; i<NumEvents; i++)
     {	if(!CloseHandle(hEvent[i]))
          	NoErr=false;
     }
	if(hEvent)
     	delete[] hEvent;
     NumEvents=0;
	hEvent=0;

     return NoErr;
}


HANDLE FarsiteEvent::GetEvent(long ThreadNum)
{
	if(ThreadNum>NumEvents-1)
     	return NULL;


     return hEvent[ThreadNum];
}


bool FarsiteEvent::SetEvent(long ThreadNum)
{
	if(ThreadNum<NumEvents)
     	return ::SetEvent(hEvent[ThreadNum]);

     return false;
}


bool FarsiteEvent::ResetEvent(long ThreadNum)
{
	if(ThreadNum<NumEvents)
     	return ::ResetEvent(hEvent[ThreadNum]);

     return false;
}                                          


bool FarsiteEvent::WaitForEvents(long numevents, bool All, unsigned long Wait)
{
	WaitForMultipleObjects(numevents, hEvent, All, Wait);

     return true;
}


bool FarsiteEvent::WaitForOneEvent(long ThreadNum, unsigned long Wait)
{
	WaitForSingleObject(hEvent[ThreadNum], Wait);

     return true;
}


//------------------------------------------------------------------------------
//
//    Landscape Semaphore Functions
//
//------------------------------------------------------------------------------
                                                                             
HANDLE hLandscapeSemaphore=0;


HANDLE GetLandscapeSemaphore()
{
	return hLandscapeSemaphore;
}


bool CreateLandscapeSemaphore()
{
     CloseLandscapeSemaphore();
     hLandscapeSemaphore=CreateSemaphore(NULL, 1, 1, reinterpret_cast<LPCSTR>("LANDSCAPE_SEMAPHORE"));
     if(hLandscapeSemaphore==NULL)
     	return false;

	return true;
}


void CloseLandscapeSemaphore()
{
	if(hLandscapeSemaphore)
     	CloseHandle(hLandscapeSemaphore);

     hLandscapeSemaphore=0;
}
