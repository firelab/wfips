//------------------------------------------------------------------------------
//
//  Event Functions
//
//------------------------------------------------------------------------------
#include "stdafx.h"

#include "fsxsync.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*FarsiteEvent 	hBurnEvent;
FarsiteEvent 	hMoistEvent;
FarsiteEvent   hBurnupEvent;
FarsiteEvent	hIntegEvent;
FarsiteEvent   hCrossEvent;
FarsiteEvent 	hBurnThreadEvent;
FarsiteEvent 	hMoistThreadEvent;
FarsiteEvent   hBurnupThreadEvent;
FarsiteEvent	hIntegThreadEvent;
FarsiteEvent	hCrossThreadEvent;*/


/*bool AllocFarsiteEvents(int EventNum, int numevents, char *basename, bool ManReset, bool InitState)
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

bool	FreeFarsiteEvents(int EventNum)
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


bool SetFarsiteEvent(int EventNum, int ThreadNum)
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

bool ResetFarsiteEvent(int EventNum, int ThreadNum)
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

bool WaitForFarsiteEvents(int EventNum, int numevents, bool All, unsigned int Wait)
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


bool WaitForOneFarsiteEvent(int EventNum, int ThreadNum, unsigned int Wait)
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


HANDLE GetFarsiteEvent(int EventNum, int ThreadNum)
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
}*/


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


bool FarsiteEvent::AllocEvents(int numevents, char *basename, bool ManReset, bool InitState)
{
     if(numevents==NumEvents)
     	return true;

	FreeEvents();

     bool NoErr=true;
     int i;
     char Name[128]="";

     hEvent=new HANDLE[numevents];
	if(hEvent==NULL)
     	return false;

	for(i=0; i<numevents; i++)
	{    
		sprintf(Name, "%s_%02ld", basename, i+1);
		//CA2T str(Name);   
		hEvent[i]=CreateEvent(NULL, ManReset, InitState, TEXT(Name));//str);
		//LPCTSTR str = A2T(Name);
		//hEvent[i]=CreateEvent(NULL, ManReset, InitState, (LPWSTR)Name);
		if(hEvent[i]==NULL)
		{	
			NoErr=true;
			break;
		}
		if(GetLastError() == ERROR_ALREADY_EXISTS)
		{
			TRACE1("uh oh Event %s already exists!\n", Name);
		}
		NumEvents++;
	}


     return NoErr;
}


bool FarsiteEvent::FreeEvents()
{
	unsigned int i;
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


HANDLE FarsiteEvent::GetEvent(int ThreadNum)
{
	if(ThreadNum>NumEvents-1)
     	return NULL;


     return hEvent[ThreadNum];
}


bool FarsiteEvent::SetEvent(int ThreadNum)
{
	//TRACE1("In FarsiteEvent::SetEvent( ThreadNum = %ld)\n", ThreadNum);
	if(ThreadNum<NumEvents)
     	return ::SetEvent(hEvent[ThreadNum]);

     return false;
}


bool FarsiteEvent::ResetEvent(int ThreadNum)
{
	if(ThreadNum<NumEvents)
     	return ::ResetEvent(hEvent[ThreadNum]);

     return false;
}                                          


bool FarsiteEvent::WaitForEvents(int numevents, bool All, unsigned int Wait)
{
	//ALM DWORD ret = WaitForMultipleObjects(numevents, hEvent, (BOOL)All, Wait);
	//WaitForMultipleObjectsEx(numevents, hEvent, (BOOL)All, Wait, FALSE);
	/*if(ret >= WAIT_OBJECT_0 && ret < (WAIT_OBJECT_0 + numevents))
	{
		TRACE3("WaitForEvents(numevents= %ld,  All = %d, Wait = %lu) returned", numevents, (BOOL)All, Wait);
		TRACE1(" normally with eventID: %ld\n", ret - WAIT_OBJECT_0);
		//hEvent[ret-WAIT_OBJECT_0] = 
	}
	else if(ret >= WAIT_ABANDONED_0 && ret < (WAIT_ABANDONED_0 + numevents))
	{
		TRACE3("WaitForEvents(numevents= %ld,  All = %d, Wait = %lu) returned", numevents, (BOOL)All, Wait);
		TRACE1(" abondoned with eventID: %ld\n", ret - WAIT_ABANDONED_0);
	}
	else if(ret == WAIT_FAILED)
	{
		TRACE3("WaitForEvents(numevents= %ld,  All = %d, Wait = %lu) returned WAIT_FAILED\n", numevents, (BOOL)All, Wait);
	}
	else if(ret == WAIT_TIMEOUT)
	{
		TRACE3("WaitForEvents(numevents= %ld,  All = %d, Wait = %lu) returned WAIT_TIMEOUT\n", numevents, (BOOL)All, Wait);
	}
	else
	{
		TRACE3("WaitForEvents(numevents= %ld,  All = %d, Wait = %lu) returned undecipherable: ", numevents, (BOOL)All, Wait);
		TRACE1(" %ld\n", ret);
	}*/
     return true;
}


bool FarsiteEvent::WaitForOneEvent(int ThreadNum, unsigned int Wait)
{
	WaitForSingleObject(hEvent[ThreadNum], Wait);

     return true;
}


//------------------------------------------------------------------------------
//
//    Landscape Semaphore Functions
//
//------------------------------------------------------------------------------
                                                                             
/*HANDLE hLandscapeSemaphore=0;


HANDLE GetLandscapeSemaphore()
{
	return hLandscapeSemaphore;
}


bool CreateLandscapeSemaphore()
{
     CloseLandscapeSemaphore();
     hLandscapeSemaphore=CreateSemaphore(NULL, 1, 1, (LPWSTR)"LANDSCAPE_SEMAPHORE");
     if(hLandscapeSemaphore==NULL)
     	return false;

	return true;
}


void CloseLandscapeSemaphore()
{
	if(hLandscapeSemaphore)
     	CloseHandle(hLandscapeSemaphore);

     hLandscapeSemaphore=0;
}*/
