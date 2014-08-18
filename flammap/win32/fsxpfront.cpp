//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//	Post frontal combustion calculations
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "fsxpfront.h"
#include "flmglbvar.h"
#include "burnupcw.h"	// burnup program
#include "fsxsync.h"
#include <math.h>
#include <process.h>

const double PI=acos(-1.0);
//extern const double PI;

//------------------------------------------------------------------------------
//  Global data for firering structures
//------------------------------------------------------------------------------
static long PRECISION=12;		// number of sampling points
static long BURNUP_TIMESTEP=15;	// seconds

static long NumRingStructs=0;
static long NumRings=0;
static RingStruct *FirstRing=0;
static RingStruct *NextRing=0;
static RingStruct *CurRing=0;

//static HANDLE *hRingSyncEvent=0;
//static HANDLE *hIntegSyncEvent=0;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//  Global functions for allocating/deallocating/accessing firering structures
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


FireRing* AllocFireRing(long NumPoints, double start, double end)
{
     long curplace, i, ThisRing;
     double Ring;

     modf(((double) NumRings/(double) RINGS_PER_STRUCT), &Ring);
     ThisRing=(long) Ring;
     curplace=NumRings-ThisRing*RINGS_PER_STRUCT;

	if(FirstRing==0)
     {	if((FirstRing=new RingStruct)==NULL)
     		return 0;
	     CurRing=FirstRing;
          CurRing->NumFireRings=0;
          CurRing->StructNum=0;
          NumRingStructs++;
          for(i=0; i<RINGS_PER_STRUCT; i++)
          {	CurRing->firering[i].perimpoints=0;
          	CurRing->firering[i].mergepoints=0;
               CurRing->firering[i].NumPoints=0;
          }
     }
     else if(ThisRing>=NumRingStructs)
     {    if(CurRing->StructNum!=NumRingStructs-1)
			GetLastRingStruct();
     	CurRing->next=(RingStruct *) new RingStruct;
		NextRing=(RingStruct *) NextRing;
     	CurRing=NextRing;
          CurRing->NumFireRings=0;
          CurRing->StructNum=NumRingStructs;
          NumRingStructs++;
          for(i=0; i<RINGS_PER_STRUCT; i++)
          {	CurRing->firering[i].perimpoints=0;
          	CurRing->firering[i].mergepoints=0;
               CurRing->firering[i].NumPoints=0;
          }
     }
     else if(ThisRing!=CurRing->StructNum)
     	GetRing(NumRings);
	CurRing->firering[curplace].perimpoints=new PerimPoints[NumPoints];
     for(i=0; i<NumPoints; i++)
     	ZeroMemory(&CurRing->firering[curplace].perimpoints[i], sizeof(PerimPoints));
     CurRing->firering[curplace].NumPoints=new long[2];
	CurRing->firering[curplace].NumPoints[0]=NumPoints;
	CurRing->firering[curplace].NumFires=1;
	CurRing->firering[curplace].StartTime=start;
	CurRing->firering[curplace].ElapsedTime=end-start;
     CurRing->firering[curplace].mergepoints=0;
     CurRing->firering[curplace].NumMergePoints[0]=0;
     CurRing->firering[curplace].NumMergePoints[1]=0;

     CurRing->NumFireRings++;
	NumRings++;

     return &CurRing->firering[curplace];
}


void GetLastRingStruct()
{
     long i;

	CurRing=FirstRing;
	for(i=0; i<NumRingStructs; i++)
     {	NextRing=(RingStruct *) CurRing->next;
     	CurRing=NextRing;
     }
}


void FreeAllFireRings()
{
     long i, j;

	CurRing=FirstRing;
	for(i=0; i<NumRingStructs; i++)
     {    if(CurRing!=NULL)
     	{	for(j=0; j<RINGS_PER_STRUCT; j++)
	     	{    if(CurRing->firering[j].perimpoints)
     			{    delete[] CurRing->firering[j].perimpoints;
					if(CurRing->firering[j].NumPoints)
						delete[] CurRing->firering[j].NumPoints;
                    	if(CurRing->firering[j].mergepoints)
                    		delete[] CurRing->firering[j].mergepoints;
					CurRing->firering[j].perimpoints=0;
					CurRing->firering[j].NumPoints=0;
					CurRing->firering[j].mergepoints=0;
     			}
               }
 	     }
          NextRing=(RingStruct *) CurRing->next;
     	delete CurRing;
     	CurRing=NextRing;
     }
     NumRings=0;
     NumRingStructs=0;
     FirstRing=0;
     NextRing=0;
     CurRing=0;
}


void FreeFireRing(long RingNum)
{
	long i, curplace, ThisRing;
     double Ring;

     modf(((double) RingNum/(double) RINGS_PER_STRUCT), &Ring);
     ThisRing=(long) Ring;
     curplace=RingNum-ThisRing*RINGS_PER_STRUCT;

     if(CurRing->StructNum!=ThisRing)
	{	CurRing=FirstRing;
		for(i=0; i<ThisRing; i++)
     	{	NextRing=(RingStruct *) CurRing->next;
     		CurRing=NextRing;
	     }
     }
     if(CurRing->firering[curplace].perimpoints)
     {    try
     	{   	delete[] CurRing->firering[curplace].perimpoints;
	     	if(CurRing->firering[curplace].NumPoints)
				delete[] CurRing->firering[curplace].NumPoints;
			CurRing->firering[curplace].perimpoints=0;
			CurRing->firering[curplace].NumPoints=0;
     	     if(CurRing->firering[curplace].mergepoints)
				delete[] CurRing->firering[curplace].mergepoints;
			CurRing->firering[curplace].mergepoints=0;
               CurRing->firering[curplace].OriginalFireNumber=-(CurRing->firering[curplace].OriginalFireNumber+1);
          }
          catch ( ... )
          {

          }
     }
}


FireRing* GetRing(long RingNum)
{
	long i, curplace, ThisRing;
     double Ring;

     if(RingNum<0)
     	return (FireRing *) NULL;

     modf(((double) RingNum/(double) RINGS_PER_STRUCT), &Ring);
     ThisRing=(long) Ring;
     curplace=RingNum-ThisRing*RINGS_PER_STRUCT;//-1;

     if(CurRing->StructNum!=ThisRing)
	{    CurRing=FirstRing;
		for(i=0; i<ThisRing; i++)
     	{	NextRing=(RingStruct *) CurRing->next;
     		CurRing=NextRing;
	     }
     }
     if(CurRing==NULL)
     	return NULL;

	return &CurRing->firering[curplace];
}


FireRing* GetSpecificRing(long FireNumber, double StartTime)
{
	long i, j;

     CurRing=FirstRing;
	j=0;
	for(i=0; i<NumRings; i++)
     {	if(fabs(CurRing->firering[j].StartTime-StartTime)<1e-9)
		{	if(CurRing->firering[j].OriginalFireNumber==FireNumber)
				break;
          }
          else
          	continue;
     	j++;
     	if(j==RINGS_PER_STRUCT)
          {	NextRing=(RingStruct *) CurRing->next;
          	CurRing=NextRing;
               j=0;
          }
     }

     return &CurRing->firering[j];
}


void CondenseRings(long RingNum)
{
// searches starting with RingNum for FireRings with no points.  Then it shifts
// the pointers down to the earliest vacant slot.  This function is called after
// all mergers are completed.

     if(RingNum<0)
		return;
     else if(RingNum==GetNumRings()-1)
     	return;

	FireRing *ring1, *ring2;
	long i, j, TotalPts;
     long NewRingNum;//=GetNumRings();
     long NewStructNum;

     for(i=RingNum; i<GetNumRings(); i++)
     {	ring1=GetRing(i);
          if(!ring1)
          	continue;
          if(ring1->mergepoints)
          {	delete[] ring1->mergepoints;
               ring1->mergepoints=0;
               ring1->NumMergePoints[0]=0;
          }
          if(ring1->perimpoints==NULL)
          {    FreeFireRing(i);
               TotalPts=0;
               for(j=i+1; j<GetNumRings(); j++)
               {    ring2=GetRing(j);
          		if(!ring2)
          			continue;
                    if(ring2->perimpoints)
                    {    TotalPts=ring2->NumPoints[ring2->NumFires-1];
                         ring1->perimpoints=ring2->perimpoints;
                         ring1->NumPoints=ring2->NumPoints;
     	               ring2->perimpoints=0;
          	          ring2->NumPoints=0;
               	     ring1->NumFires=ring2->NumFires;
                    	ring1->OriginalFireNumber=ring2->OriginalFireNumber;
                         ring1->StartTime=ring2->StartTime;
                    	ring2->OriginalFireNumber=0.0;
                         ring2->StartTime=0.0;
                         ring1->ElapsedTime=ring2->ElapsedTime;
                         ring2->NumFires=0;
                         ring2->OriginalFireNumber=-1;

                         break;
                    }
               }
          }
     }

     // free up CurRing if no points left!!!!!
     RingStruct *LastRing;
     CurRing=LastRing=FirstRing;
     NewStructNum=0;
	for(i=0; i<NumRingStructs; i++)
     {    if(CurRing!=NULL)
     	{    NewRingNum=0;
          	for(j=0; j<RINGS_PER_STRUCT; j++)
          	{	if(CurRing->firering[j].perimpoints!=NULL)
               		NewRingNum++;
               }
			CurRing->NumFireRings=NewRingNum;
               if(NewRingNum>0)
               	NewStructNum++;
               else
	     	{    NextRing=(RingStruct *) CurRing->next;
                    LastRing->next=NextRing;
                    if(CurRing==LastRing)         // will only occur if there are no rings now
                    {    delete FirstRing;
                         FirstRing=LastRing=0;
                    }
                    else
                         delete CurRing;
                    CurRing=LastRing;
               }
 	     }
          if(CurRing==NULL)
               break;
          LastRing=CurRing;
          NextRing=(RingStruct *) CurRing->next;
     	CurRing=NextRing;
     }
     CurRing=FirstRing;
     NewRingNum=0;
     for(i=0; i<NewStructNum; i++)
     {	NewRingNum+=CurRing->NumFireRings;
     	NextRing=(RingStruct *) CurRing->next;
		CurRing=NextRing;
     }
     SetNumRings(NewRingNum);//(NewStructNum-1)*RINGS_PER_STRUCT+NewRingNum);
     NumRingStructs=NewStructNum;
     CurRing=FirstRing;
}


void SetNewFireNumber(long OldNum, long NewNum, long RefNum)
{
     bool found=false;
	long i;
     FireRing *ring;

     for(i=RefNum; i<GetNumRings(); i++)
     {	ring=GetRing(i);
     	if(OldNum==ring->OriginalFireNumber)
          {    ring->OriginalFireNumber=NewNum;
               found=true;
	          break;
          }
     }
     if(!found)        // for debugging
          found=true;
}


void SetNumRings(long NewNumRings)
{
	NumRings=NewNumRings;
}

long GetNumRings()
{
	return NumRings;
}

long GetNumRingStructs()
{
	return NumRingStructs;
}



bool AddToCurrentFireRing(FireRing *firering, long PointNum, long SurfFuelType,
	long WoodyModel, double DuffLoad, double *mx, double CrownLoadingBurned)
{
	if(!firering)
     	return false;

	long i, NumMx, NumFire;
     double x, y, r, f, c;

     NumFire=firering->OriginalFireNumber;
     f=GetPerimeter2Value(PointNum, FLIVAL);
     if(f<0.0)
     {    x=GetPerimeter2Value(PointNum, XCOORD);
          y=GetPerimeter2Value(PointNum, YCOORD);
          r=fabs(GetPerimeter2Value(PointNum, ROSVAL));
          f=fabs(f);
          c=GetPerimeter2Value(PointNum, RCXVAL);
     }
     else
     {    x=GetPerimeter1Value(NumFire, PointNum, XCOORD);
          y=GetPerimeter1Value(NumFire, PointNum, YCOORD);
          r=fabs(GetPerimeter1Value(NumFire, PointNum, ROSVAL));
          f=fabs(GetPerimeter1Value(NumFire, PointNum, FLIVAL));
          c=GetPerimeter1Value(NumFire, PointNum, RCXVAL);
     }
     firering->perimpoints[PointNum].x2=x;
    	firering->perimpoints[PointNum].y2=y;
	if(c<1e-2)
     	c=0.0;
     firering->perimpoints[PointNum].hist.ReactionIntensity[1]=(float) c;
     if(fabs(r)>0.0 && c>0.0)
	     firering->perimpoints[PointNum].hist.FlameResidenceTime[1]=(float) ((f*60.0)/(r*c)); // seconds
     else
         	firering->perimpoints[PointNum].hist.FlameResidenceTime[1]=0.0;

     firering->perimpoints[PointNum].hist.WoodyFuelType=(unsigned char) WoodyModel;
     firering->perimpoints[PointNum].hist.SurfaceFuelModel=(unsigned char) SurfFuelType;
     if(DuffLoad<0.1)
          DuffLoad=0.1;
	firering->perimpoints[PointNum].hist.DuffLoad=(short) (DuffLoad*10.0);
	//mx=GetAllCurrentMoistures(&NumMx);

     if(mx)
     {    for(i=0; i<MAXNO; i++)
		     firering->perimpoints[PointNum].hist.Moistures[i]=(unsigned char) (mx[i]*100.0);
     }
     //for(i=NumMx-1; i<20; i++)
	//	firering->perimpoints[PointNum].hist.Moistures[i]=0;
     firering->perimpoints[PointNum].hist.LastWtRemoved=0.0;	// initialize last values of integration
     firering->perimpoints[PointNum].hist.WeightPolyNum=0;
     firering->perimpoints[PointNum].hist.FlamePolyNum=0;
     firering->perimpoints[PointNum].hist.CrownLoadingBurned=(float) CrownLoadingBurned;

     return true;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

RingBurn::RingBurn()
{
	burnup=0;
	hThread=0;
     burnup=0;
     ring=0;
     //hRingEvent=0;
     ThreadStarted=false;
}

RingBurn::~RingBurn()
{
   	if(burnup)
		delete burnup;
}


void RingBurn::SetRange(FireRing *rring, long firenum, long begin, long end)
{
	ring=rring;
     FireNum=firenum;
     Begin=begin;
     End=end;
     CurOldRing=FirstRing;
}

HANDLE RingBurn::StartBurnThread(long ID)
{
	if(ThreadStarted==false)
     {   	burnup=new BurnUp();
          ThreadOrder=ID;
          hRingEvent=GetFarsiteEvent(EVENT_BURNUP_THREAD, ThreadOrder);
        	hThread=(HANDLE) ::_beginthreadex(NULL, 0, &RingBurn::RunBurnThread, this, CREATE_SUSPENDED, &ThreadID);
          ResumeThread(hThread);
          CloseHandle(hThread);
          hThread=0;
     }
     else
     	SetEvent(hRingEvent);//SetFarsiteEvent(EVENT_BURNUP_THREAD, ThreadOrder);

     return hThread;
}

unsigned __stdcall RingBurn::RunBurnThread(void *ringburn)
{
	static_cast <RingBurn *> (ringburn)->BurnThread();

     return 1;
}


void RingBurn::BurnThread()
{
	long k, m, num, ma, lastpt, nextpt;
     double depth, total, DuffMx;
	bool SimCont;
     long NumCalcs;
     double coefx, coefy;//*coefx, *coefy;
     WoodyData wd[20];

     ThreadStarted=true;
     do
     {    if(End<0)
     		break;
     	for(k=Begin; k<End; k++)
	    	{    GetWoodyData(ring->perimpoints[k].hist.WoodyFuelType, ring->perimpoints[k].hist.SurfaceFuelModel, &num, wd, &depth, &total);
     	     for(m=0; m<num; m++)
          	   	wd[m].FuelMoisture=((double) ring->perimpoints[k].hist.Moistures[m])/100.0;
               // this is from Harrington
               DuffMx=-0.347+6.42*wd[2].FuelMoisture;
               if(DuffMx<0.10)
               	DuffMx=0.10;
               burnup->SetFuelInfo(num, &(wd[0].SurfaceAreaToVolume));
     	     burnup->SetFireDat(5000,
          	ring->perimpoints[k].hist.ReactionIntensity[1],
               ring->perimpoints[k].hist.FlameResidenceTime[1],
     	     		0.0, depth, 27.0, 1.8, 0.4, (double) BURNUP_TIMESTEP,
          	          (double) ring->perimpoints[k].hist.DuffLoad/100.0, DuffMx);//ring->perimpoints[k].hist.Moistures[num-1]);   // use last value of fms for duff
               lastpt=k-1;
     	     if(FireNum==0)
               {	if(lastpt<0)
	          		lastpt=ring->NumPoints[FireNum]-1;
               }
               else if(lastpt<ring->NumPoints[FireNum-1])
               	lastpt=ring->NumPoints[FireNum]-1;
               nextpt=k+1;
               if(nextpt>ring->NumPoints[FireNum]-1)
               {	if(FireNum==0)
               		nextpt=0;
               	else
	               	nextpt=ring->NumPoints[FireNum-1];
               }

               CalculateArea(ring, k, lastpt, nextpt);

               if(ring->perimpoints[k].Area==0.0)
               	continue;
	          else if(CombustionHistoryExists(k))    // check to see if useable one exists already
               {	ring->perimpoints[k].hist.TotalWeight=total+(double) ring->perimpoints[k].hist.DuffLoad/100.0;
     	          //ring->perimpoints[k].Area=-1.0;
     	     	continue;
               }
               if(burnup->CheckData())
     	     {    if(burnup->StartLoop())
          		{    do
               	     {	SimCont=burnup->BurnLoop();

	                    } while(SimCont==true);

     	               ma=burnup->GetSamplePoints(0, PRECISION);//, &coefx, &coefy, &NumCalcs);
          	          ring->perimpoints[k].hist.WeightPolyNum=(unsigned char) ma;
               	     for(m=0; m<ma; m++)
                    	{    burnup->GetSample(m, &coefx, &coefy);
                         	ring->perimpoints[k].hist.WeightCoefX[m]=(float) coefx;//coefx[m];
	                   		ring->perimpoints[k].hist.WeightCoefY[m]=(float) coefy;//coefy[m];
     	               }
	                    ring->perimpoints[k].hist.TotalTime=(float) coefx;//[ma-1];//NumCalcs*(float) BURNUP_TIMESTEP/60.0;	// time in minutes
     	               ma=burnup->GetSamplePoints(1, PRECISION);//, &coefx, &coefy, &NumCalcs);
          	          ring->perimpoints[k].hist.FlamePolyNum=(unsigned char) ma;
               	     for(m=0; m<ma; m++)
	                    {    burnup->GetSample(m, &coefx, &coefy);
                         	ring->perimpoints[k].hist.FlameCoefX[m]=(float) coefx;//coefx[m];
     	               	ring->perimpoints[k].hist.FlameCoefY[m]=(float) coefy;//coefy[m];
          	          }
               	     ring->perimpoints[k].hist.FlameTime=(float) coefx;//[ma-1]
                         //(float) NumCalcs*(float) BURNUP_TIMESTEP/60.0;     // time in minutes
	               }
     	          //ring->perimpoints[k].Area=-1.0;
          	}
	          if(ring->perimpoints[k].hist.WeightPolyNum==0 || ring->perimpoints[k].hist.TotalTime<0.1) // seconds
     	     {	//ring->perimpoints[k].Area=0.0;
                    ring->perimpoints[k].hist.TotalTime=0.1;//ring->perimpoints[k].hist.FlameResidenceTime[1]/60.0;
                    ring->perimpoints[k].hist.TotalWeight=                // just set to amount consumed in flaming front
                         ring->perimpoints[k].hist.ReactionIntensity[1]*
                         ring->perimpoints[k].hist.FlameResidenceTime[1]/18600.0;
                    //ring->perimpoints[k].hist.TotalWeight=total+(double) ring->perimpoints[k].hist.DuffLoad/10.0;
                    ring->perimpoints[k].hist.WeightPolyNum=2;
                    ring->perimpoints[k].hist.FlamePolyNum=2;
                    ring->perimpoints[k].hist.FlameCoefX[1]=0.1;//ring->perimpoints[k].hist.FlameResidenceTime[1]/60.0;
                    ring->perimpoints[k].hist.FlameCoefY[0]=1.0;
                    ring->perimpoints[k].hist.FlameCoefY[1]=1.0;
                    ring->perimpoints[k].hist.WeightCoefX[1]=0.1;//ring->perimpoints[k].hist.FlameResidenceTime[1]/60.0;
                    ring->perimpoints[k].hist.WeightCoefY[0]=1.0;
                    ring->perimpoints[k].hist.WeightCoefY[1]=0.0;
                    ring->perimpoints[k].hist.FlameTime=ring->perimpoints[k].hist.TotalTime=0.1; //ring->perimpoints[k].hist.FlameResidenceTime[1]/60.0;
               }
               else
	               ring->perimpoints[k].hist.TotalWeight=total+(double) ring->perimpoints[k].hist.DuffLoad/100.0;
     	     ring->perimpoints[k].hist.LastWtRemoved=0.0;	// initialize last values of integration
	     }
          SetFarsiteEvent(EVENT_BURNUP, ThreadOrder);  //SetEvent(hRingSyncEvent[ThreadOrder]);
          WaitForSingleObject(hRingEvent, INFINITE);
          ResetEvent(hRingEvent);
          if(Begin<0 || End<0)
          	break;
     }while(End>-1);

     SetFarsiteEvent(EVENT_BURNUP, ThreadOrder);       //SetEvent(hRingSyncEvent[ThreadOrder]);
}


bool RingBurn::CombustionHistoryExists(long CurIndex)
{
     bool Exists=false;
	long i, j, k, m, n, nrings, nfires;
     double heat, xheat, heatrat;
     FireRing *xring;
     BurnHistory *hist, *xhist;

     hist=&ring->perimpoints[CurIndex].hist;
     heat=hist->ReactionIntensity[1]*hist->FlameResidenceTime[1];
	if(CurIndex>Begin)
     {    for(i=Begin; i<CurIndex; i++)
     	{    if(ring->perimpoints[i].Area<=0.0)
				continue;
          	xhist=&ring->perimpoints[i].hist;
               if(xhist->WeightPolyNum==0)
                   	return false;
               if(hist->SurfaceFuelModel!=xhist->SurfaceFuelModel)
                   	continue;
               if(hist->WoodyFuelType!=xhist->WoodyFuelType)
                   	continue;
               if(hist->DuffLoad!=xhist->DuffLoad)
                   	continue;
               xheat=xhist->ReactionIntensity[1]*xhist->FlameResidenceTime[1];
			if(heat>xheat)
                   	heatrat=heat/xheat;
               else
                   	heatrat=xheat/heat;
               if(heatrat>1.5)
                   	continue;
               for(m=2; m<20; m++)	// ignore small stuff, will appear in reaction intensity
               {	if(abs(hist->Moistures[m]-xhist->Moistures[m])>5)// 5 percent tolerance
                		break;
               }
               if(m==20)
               {	Exists=true;
                   	break;
               }
          }
     }

     if(!Exists && ring->StartTime>0.0)
     {
     nrings=GetNumRings();
     nfires=GetNumFires();
     for(i=nrings-nfires-1; i>=nrings-3*nfires-1; i--)	// for the last one and before
     {    if(i<0)
     		continue;
     	xring=GetOldRing(i);	// local copy
          if(xring->perimpoints==NULL)
          	continue;
     	for(j=0; j<xring->NumFires; j++)
          {	if(j==0)
               	n=0;
               else
               	n=xring->NumPoints[j-1];
          	for(k=n; k<xring->NumPoints[j]; k++)
               {    xhist=&xring->perimpoints[k].hist;
                    if(xhist->WeightPolyNum==0)
                    	continue;//return false;
               	if(hist->SurfaceFuelModel!=xhist->SurfaceFuelModel)
                    	continue;
                    if(hist->WoodyFuelType!=xhist->WoodyFuelType)
                    	continue;
                    if(hist->DuffLoad!=xhist->DuffLoad)
                    	continue;
               	xheat=xhist->ReactionIntensity[1]*xhist->FlameResidenceTime[1];
				if(heat>xheat)
                        	heatrat=heat/xheat;
                    else
                        	heatrat=xheat/heat;
                    if(heatrat>1.5)
                        	continue;
                    for(m=2; m<20; m++)	// ignore small stuff, will appear in reaction intensity
                    {	if(abs(hist->Moistures[m]-xhist->Moistures[m])>5)// 5 percent tolerance
                    		break;
                    }
                    if(m==20)
                    {	Exists=true;
                     	break;
                    }
               }
               if(Exists)
               	break;
          }
          if(Exists)
              	break;
     }
     }

     if(Exists)
     {	CopyMemory(hist->FlameCoefX, xhist->FlameCoefX, 18*sizeof(float));
     	CopyMemory(hist->FlameCoefY, xhist->FlameCoefY, 18*sizeof(float));
     	CopyMemory(hist->WeightCoefX, xhist->WeightCoefX, 18*sizeof(float));
     	CopyMemory(hist->WeightCoefY, xhist->WeightCoefY, 18*sizeof(float));
          hist->FlamePolyNum=xhist->FlamePolyNum;
          hist->WeightPolyNum=xhist->WeightPolyNum;
          hist->FlameTime=xhist->FlameTime;
          hist->TotalTime=xhist->TotalTime;
          hist->LastWtRemoved=0.0;
     }

     return Exists;
}


FireRing* RingBurn::GetOldRing(long RingNum)
{
	long i, curplace, ThisRing;
     double Ring;

     if(RingNum<0)
     	return (FireRing *) NULL;

     modf(((double) RingNum/(double) RINGS_PER_STRUCT), &Ring);
     ThisRing=(long) Ring;
     curplace=RingNum-ThisRing*RINGS_PER_STRUCT;//-1;

     if(CurOldRing->StructNum!=ThisRing)
	{    CurOldRing=FirstRing;
		for(i=0; i<ThisRing; i++)
     	{	NextOldRing=(RingStruct *) CurOldRing->next;
     		CurOldRing=NextOldRing;
	     }
     }
     if(CurOldRing==NULL)
     	return NULL;

	return &CurOldRing->firering[curplace];
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//	BurnupFireRings class
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

BurnupFireRings::BurnupFireRings()
{
	ringburn=0;
	NumRingBurn=0;
//	hBurnSemaphore=0;
}


BurnupFireRings::~BurnupFireRings()
{
	CloseBurnupFireRings();
}


void BurnupFireRings::BurnFireRings(long StartRingNum, long EndRingNum)
{
	long i, j, m, start;
     long begin, end, range, threadct; // prevct
     double fract, ipart, interval;

     FireRing *ring;

    	AllocRingBurn();	// try to alloc ring burn
     for(i=StartRingNum; i<EndRingNum; i++)	// can parallelize here
     {	ring=GetRing(i);
     	if(!ring)
          	continue;
          start=0;
		if(!ring->NumPoints)
          	continue;
		for(j=0; j<ring->NumFires; j++)
          {	interval=((double) (ring->NumPoints[j]-start))/(double) NumRingBurn;
          	fract=modf(interval, &ipart);
               range=(long) interval;
               if(fract>0.0)
               	range++;
               threadct=0;
               for(m=0; m<NumRingBurn; m++)
               {    begin=m*range+start;
               	end=(m+1)*range+start;
                    if(begin>=ring->NumPoints[j])
                    	continue;
                    if(end>ring->NumPoints[j])
                    	end=ring->NumPoints[j];
               	ringburn[m].SetRange(ring, j, begin, end);
                    threadct++;
               }
               for(m=0; m<threadct; m++)
               	ringburn[m].StartBurnThread(m);
               WaitForFarsiteEvents(EVENT_BURNUP, threadct, true, INFINITE);

               start=ring->NumPoints[j];
          }
	}
}


void BurnupFireRings::CloseBurnupFireRings()
{
	long m;//, threadct=0;

     if(NumRingBurn>0)
     {	for(m=0; m<NumRingBurn; m++)
          	ringburn[m].SetRange(NULL, 0, -1, -1);
	     for(m=0; m<NumRingBurn; m++)
     	    	ringburn[m].StartBurnThread(m);
          Sleep(50);
          WaitForFarsiteEvents(EVENT_BURNUP, NumRingBurn, true, INFINITE); // 2000
     }
     FreeRingBurn();
}


bool BurnupFireRings::AllocRingBurn()
{
     if(NumRingBurn==GetMaxThreads())
     	return true;

     CloseBurnupFireRings();
     ringburn=new RingBurn[GetMaxThreads()];

	if(ringburn)
     {    NumRingBurn=GetMaxThreads();

     	return true;
     }

     return false;
}


void BurnupFireRings::FreeRingBurn()
{
     if(ringburn)
     	delete[] ringburn;
     NumRingBurn=0;
     ringburn=0;
}


void BurnupFireRings::ResetAllThreads()
{
	CloseBurnupFireRings();
}




//------------------------------------------------------------------------------
//
//	Post frontal combustion class member functions
//
//------------------------------------------------------------------------------



PostFrontal::PostFrontal()
{
	MergeReferenceRingNum=-1;
     NumPFI=0;
     pfi=0;
     WeightLossErrorTolerance(1.0);
}


PostFrontal::~PostFrontal()
{
     ResetAllThreads();
//     CloseAllThreads();
}


void PostFrontal::ResetAllThreads()
{
     bup.ResetAllThreads();
	CloseAllThreads();
}


long PostFrontal::BurnupPrecision(long LoHi)
{
     long Prec;

     switch(LoHi)
     {    case 0:
               PRECISION=12;
               BURNUP_TIMESTEP=15;
               Prec=0;
               WeightLossErrorTolerance(1.0);
               break;
          case 1:
               PRECISION=18;
               BURNUP_TIMESTEP=10;
               Prec=1;
               WeightLossErrorTolerance(0.25);
               break;
          default:
               if(PRECISION==12)
                    Prec=0;
               else
                    Prec=1;
               break;
     }

     return Prec;
}


long PostFrontal::AccessReferenceRingNum(long Merge, long Number)
{
     if(Number<0)
     {	if(Merge)
     		return MergeReferenceRingNum;
     	else
          	return ReferenceFireRingNum;
     }
     else
     {	if(Merge)
     		MergeReferenceRingNum=Number;
     	else
          	ReferenceFireRingNum=Number;
     }

     return 0;
}


FireRing* PostFrontal::SetupFireRing(long NumFire, double Start, double End)
{
	long i, NumPoints;
     double x, y, r, f, c;
     FireRing* firering;

     if(NumRings==0)               	// only at very beginning
     	MergeReferenceRingNum=-1;

     NumPoints=GetNumPoints(NumFire);
	if((firering=AllocFireRing(NumPoints, Start, End))==NULL)
     	return NULL;
	firering->StartTime=Start;
	firering->ElapsedTime=End-Start;
     firering->OriginalFireNumber=NumFire;

	for(i=0; i<NumPoints; i++)
	{    GetPerimeter2(i, &x, &y, &r, &f, &c);
     	firering->perimpoints[i].x1=x;
     	firering->perimpoints[i].y1=y;
          firering->perimpoints[i].hist.ReactionIntensity[0]=(float) c;
          if(fabs(r)>0.0 && c>0.0)
	          firering->perimpoints[i].hist.FlameResidenceTime[0]=fabs((float) ((f*60.0)/(r*c)));	// sec
     	else
          	firering->perimpoints[i].hist.FlameResidenceTime[0]=0.0;
		firering->perimpoints[i].Status=0;
          firering->perimpoints[i].SizeMult=1.0;
	     firering->perimpoints[i].hist.LastIntegTime=-1.0;
	}
     ReferenceFireRingNum=NumRings-1;   // store for CorrectFireRing

     if(MergeReferenceRingNum<0)
     	MergeReferenceRingNum=NumRings-1;

	return firering;
}


void PostFrontal::ComputePostFrontal(double CurrentTime, double *s, double *f)
{
	long i, j, k, nrings, start;
     double RingFlameWt, RingSmolderWt;
     double fract, interval, ipart;
     long range, begin, end, threadct;//, prevct;
     FireRing *ring;

    	AllocPFI();
     nrings=GetNumRings();
	SimLevel_FlameWeightConsumed=SimLevel_SmolderWeightConsumed=0.0;
	for(i=0; i<nrings; i++)
     {    ring=GetRing(i);
          if(ring->perimpoints==NULL)
          	continue;

          start=0;
		RingFlameWt=RingSmolderWt=0.0;
		for(j=0; j<ring->NumFires; j++)
          {    interval=((double) (ring->NumPoints[j]-start)/(double) GetMaxThreads());
          	fract=modf(interval, &ipart);
               range=(long) interval;
               if(fract>0.0)
               	range++;
               threadct=0;
               for(k=0; k<NumPFI; k++)   // number of postfrontal integration objects
               {	begin=k*range+start;
               	end=begin+range;
                    if(begin>=ring->NumPoints[j])
                    	continue;
                    if(end>ring->NumPoints[j])
                    	end=ring->NumPoints[j];
                    pfi[k].SetRange(ring, j, CurrentTime, begin, end);
                    threadct++;
               }
               for(k=0; k<threadct; k++)		// only start as many as have data, not NumPFI
				pfi[k].StartIntegThread(k);
               WaitForFarsiteEvents(EVENT_INTEG, threadct, true, INFINITE);
               //WaitForMultipleObjects(NumPFI, hIntegSyncEvent, TRUE, INFINITE);

               for(k=0; k<threadct; k++)		// add up from all segments of ring
               {	RingSmolderWt+=pfi[k].SmolderWeightConsumed;
				RingFlameWt+=pfi[k].FlameWeightConsumed;
               }
               start=ring->NumPoints[j];
          }
          if(RingSmolderWt>0.0)
               SimLevel_SmolderWeightConsumed+=RingSmolderWt;	// add up from entire ring
          SimLevel_FlameWeightConsumed+=RingFlameWt;

		if(RingSmolderWt+RingFlameWt==0.0)
          {	if(ring->OriginalFireNumber<0)
	             	FreeFireRing(i);
          	else
               {	ring->OriginalFireNumber+=1;
				ring->OriginalFireNumber*=-1;
               }
          }
     }
     *s+=SimLevel_SmolderWeightConsumed;
     *f+=SimLevel_FlameWeightConsumed;
     CondenseRings(0);
}

void PostFrontal::UpdateAttackPoints(FireRing *firering, long firenum)
{
// fix up firering because of new points added by Direct Attack

	bool PtFound;
	long i, j, m, lastpt=0;
     long NewPoints;
     PerimPoints *tempperim;

     m=0;
     NewPoints=GetNumPoints(firenum);
     tempperim=new PerimPoints[NewPoints];
     for(i=0; i<NewPoints; i++)
     {    x1=GetPerimeter2Value(i, XCOORD);
    		y1=GetPerimeter2Value(i, YCOORD);
          PtFound=false;
     	for(j=0; j<firering->NumPoints[0]; j++)
     	{    if(firering->perimpoints[j].Status<0)
          		continue;
          	if(pow2(firering->perimpoints[j].x1-x1)+pow2(firering->perimpoints[j].y1-y1)<1e-10)
			{	CopyMemory(&tempperim[m], &firering->perimpoints[j], sizeof(PerimPoints));
                    tempperim[m].x2=GetPerimeter1Value(firenum, i, XCOORD);  // replace always
                    tempperim[m++].y2=GetPerimeter1Value(firenum, i, YCOORD);
                    lastpt=j;
               	firering->perimpoints[j].Status=-1;
                    PtFound=true;
	               break;
               }
          }
          if(PtFound==false)//j==firering->NumPoints[0])      // if point not found
          {    tempperim[m].x1=x1;
		 	tempperim[m].y1=y1;
               tempperim[m].x2=GetPerimeter1Value(firenum, i, XCOORD);
    		     tempperim[m].y2=GetPerimeter1Value(firenum, i, YCOORD);
               tempperim[m].Status=1;
               tempperim[m].hist=firering->perimpoints[lastpt].hist;
              	tempperim[m].l1=tempperim[m].l2=tempperim[m].h=tempperim[m].Area=0.0;
              	tempperim[m++].SizeMult=1.0;
          }
	}
     delete[] firering->perimpoints;
     firering->perimpoints=new PerimPoints[NewPoints];
     CopyMemory(firering->perimpoints, tempperim, NewPoints*sizeof(PerimPoints));
     delete[] tempperim;
     firering->NumPoints[0]=NewPoints;

}


void PostFrontal::UpdateFireRing(FireRing *firering, long firenum, long NewPoints)
{
// fix up firering because of new points added in Intersect::OrganizeCrosses
// and then reorders points in the firering based on current ordering that may
// have changed when crosses were identified

	bool PtFound;
	long i, j, m, n, p, LastPt=-1;
     PerimPoints *tempperim;

     m=0;
     tempperim=new PerimPoints[NewPoints];
     for(i=0; i<NewPoints; i++)
     {    x2=GetPerimeter1Value(firenum, i, XCOORD);
    		y2=GetPerimeter1Value(firenum, i, YCOORD);
          PtFound=false;
     	for(j=0; j<firering->NumPoints[0]; j++)
     	{    if(firering->perimpoints[j].Status<0)
          		continue;
          	if(pow2(firering->perimpoints[j].x2-x2)+pow2(firering->perimpoints[j].y2-y2)<1e-10)
			{	CopyMemory(&tempperim[m++], &firering->perimpoints[j], sizeof(PerimPoints));
               	firering->perimpoints[j].Status=-1;
                    LastPt=j;
                    PtFound=true;
	               break;
               }
          }
          if(PtFound==false)//j==firering->NumPoints[0])      // if point not found
          {    n=LastPt;
              	if(n<0)
                   	n+=firering->NumPoints[0];
               p=LastPt+1;
               if(p>firering->NumPoints[0]-1)
               	p-=firering->NumPoints[0];
              	xm1=firering->perimpoints[n].x1;
		 	ym1=firering->perimpoints[n].y1;
		     xm2=firering->perimpoints[n].x2;
		     ym2=firering->perimpoints[n].y2;
               xn1=firering->perimpoints[p].x1;
           	yn1=firering->perimpoints[p].y1;
	          xn2=firering->perimpoints[p].x2;
     	    	yn2=firering->perimpoints[p].y2;
           	tempperim[m].x2=x2;
		 	tempperim[m].y2=y2;
               if(fabs(xm2-xn2)>1e-9)
		          tempperim[m].x1=xm1-(xm1-xn1)*(xm2-x2)/(xm2-xn2);
               else
         	          tempperim[m].x1=xm1;
               if(fabs(ym2-yn2)>1e-9)
		    	     tempperim[m].y1=ym1-(ym1-yn1)*(ym2-y2)/(ym2-yn2);
               else
               	tempperim[m].y1=ym1;
               tempperim[m].Status=1;
               tempperim[m].hist=firering->perimpoints[n].hist;
	          tempperim[m].Area=-1;
	          tempperim[m].l1=tempperim[m].l2=tempperim[m].h=0.0;
              	tempperim[m++].SizeMult=1.0;
          }
	}
     delete[] firering->perimpoints;
     firering->perimpoints=new PerimPoints[NewPoints];
     CopyMemory(firering->perimpoints, tempperim, NewPoints*sizeof(PerimPoints));
     delete[] tempperim;
     firering->NumPoints[0]=NewPoints;
}


void PostFrontal::UpdatePointOrder(FireRing *firering, long firenum)
{
// reorders points in the firering based on current ordering that may have
// changed when crosses were identified

     long i, j, k;
     PerimPoints *tempperim;

     x2=GetPerimeter1Value(firenum, 0, XCOORD);
	y2=GetPerimeter1Value(firenum, 0, YCOORD);
     for(i=0; i<firering->NumPoints[0]; i++)
	{	if(pow2(firering->perimpoints[i].x2-x2)+pow2(firering->perimpoints[i].y2-y2)<1e-8)
	 		break;
     }
     if(i==0)
     	return;

     tempperim=new PerimPoints[firering->NumPoints[0]];
     CopyMemory(tempperim, firering->perimpoints, firering->NumPoints[0]*sizeof(PerimPoints));
     for(j=0; j<firering->NumPoints[0]; j++)
     {	k=j+i;
     	if(k>firering->NumPoints[0]-1)
          	k-=firering->NumPoints[0];
	     CopyMemory(&firering->perimpoints[j], &tempperim[k], sizeof(PerimPoints));
     }
     delete[] tempperim;
}


void PostFrontal::UpdateMergeOrder(FireRing *firering, long firenum)
{
// reorders points in the firering based on current ordering that may have
// changed when crosses were identified

     bool PtFound;
     long i, j, k, m, n, p, q, r, s, LastID=0, NewTotal;
     long Start=0;
     MergePoints *tempmerge=0;
     PerimPoints *tempperim=0;

     k=0;
     tempmerge=new MergePoints[GetNumPoints(firenum)];
     // update mergepoints array with exact points in perim2 array for outer fire
     for(i=0; i<GetNumPoints(firenum); i++)
     {	x2=GetPerimVal(1, firenum, i, XCOORD);
     	y2=GetPerimVal(1, firenum, i, YCOORD);
          PtFound=false;
          for(j=Start; j<firering->NumMergePoints[0]+Start; j++)
          {    p=j;
          	if(p>firering->NumMergePoints[0]-1)
               	p-=firering->NumMergePoints[0];
          	x1=firering->mergepoints[p].x;
          	y1=firering->mergepoints[p].y;
               if(pow2(x2-x1)+pow2(y2-y1)<1e-10)
               {   	tempmerge[k].x=x2;
          		tempmerge[k].y=y2;
          		tempmerge[k].Status=1;
          		tempmerge[k].FireID=0;
          		tempmerge[k++].VertexID=LastID=firering->mergepoints[p].VertexID;
                    PtFound=true;
                    Start=p;
                    break;
               }
          }
          if(PtFound==false)//j==firering->NumMergePoints[0])
          {   	tempmerge[k].x=x2;
          	tempmerge[k].y=y2;
          	tempmerge[k].Status=1;
          	tempmerge[k].FireID=0;
          	tempmerge[k++].VertexID=-(LastID+1);
               LastID++;
          }
     }

     if(k>firering->NumMergePoints[0])      // fire acquired more pts since becoming ring
     {	delete[] firering->mergepoints;
          firering->mergepoints=new MergePoints[GetNumPoints(firenum)];

          NewTotal=10*(k-firering->NumMergePoints[0])+firering->NumPoints[firering->NumFires-1]+1;
          tempperim=new PerimPoints[firering->NumPoints[firering->NumFires-1]];
          CopyMemory(tempperim, firering->perimpoints, firering->NumPoints[firering->NumFires-1]*sizeof(PerimPoints));
          delete[] firering->perimpoints;
          firering->perimpoints=new PerimPoints[NewTotal];

          CopyMemory(firering->mergepoints, tempmerge, k*sizeof(MergePoints));
          firering->NumMergePoints[0]=k;
          firering->NumMergePoints[1]=0;

          for(i=0; i<firering->NumMergePoints[0]; i++)
          {    firering->mergepoints[i].VertexID=-1;
               tempmerge->VertexID=-1;
          }
          r=0;
          for(i=0; i<firering->NumMergePoints[0]; i++)
          {    x1=firering->mergepoints[i].x;
               y1=firering->mergepoints[i].y;
               PtFound=false;
               for(j=0; j<firering->NumFires; j++)
               {    if(j==0)
                         k=0;
                    else
                         k=firering->NumPoints[j-1];
                    for(m=k; m<firering->NumPoints[j]; m++)
                    {    x2=tempperim[m].x2;
                         y2=tempperim[m].y2;
                         if(pow2(x1-x2)+pow2(y1-y2)>1e-10)
                              continue;
                         firering->mergepoints[i].VertexID=m;
                         tempmerge->VertexID=m;
                         {    PtFound=true;

                              break;
                         }
                    }
                    if(PtFound)
                         break;
               }
          }
          CopyMemory(firering->perimpoints, tempperim, firering->NumPoints[firering->NumFires-1]*sizeof(PerimPoints));
          for(i=0; i<firering->NumMergePoints[0]; i++)
          {    if(firering->mergepoints[i].VertexID>=0)
                    continue;
               // find the previous point with correspoinding perimpoint
               p=i;
               do
               {    p--;
                    if(p<0)
                         p=firering->NumMergePoints[0]-1;
               }while(firering->mergepoints[p].VertexID<0);
               p=firering->mergepoints[p].VertexID;

               // find the right fire among those in perimpoints
               for(j=0; j<firering->NumFires; j++)
               {    if(firering->mergepoints[p].VertexID<firering->NumPoints[j])
                         break;    // j=firenum
               }

               // find next point with corresponding perimpoint
               /*
               q=i;
               do
               {    q++;
                    if(q>=firering->NumMergePoints[0])
                         q=0;
               }while(firering->mergepoints[q].VertexID<0);
               q=firering->mergepoints[q].VertexID;
               */
               q=p+1;
               if(q>=firering->NumPoints[j])
               {    if(j==0)
                         q=0;
                    else
                         q=firering->NumPoints[j-1];
               }

               x2=firering->mergepoints[i].x;
               y2=firering->mergepoints[i].y;

               xm2=firering->perimpoints[p].x2;
               ym2=firering->perimpoints[p].y2;
               xn2=firering->perimpoints[q].x2;
               yn2=firering->perimpoints[q].y2;

               xm1=firering->perimpoints[p].x1;
               ym1=firering->perimpoints[p].y1;
               xn1=firering->perimpoints[q].x1;
               yn1=firering->perimpoints[q].y1;

               if(p>q)
                    memmove(&firering->perimpoints[q+1], &firering->perimpoints[q],
                    (firering->NumPoints[firering->NumFires-1]-q)*sizeof(PerimPoints));
               else
                    memmove(&firering->perimpoints[p+2], &firering->perimpoints[p+1],
                    (firering->NumPoints[firering->NumFires-1]-(p+1))*sizeof(PerimPoints));
               for(r=0; r<firering->NumMergePoints[0]; r++)
               {    if(firering->mergepoints[r].VertexID>p)
                        firering->mergepoints[r].VertexID+=1;
               }
               m=p+1;

			if(fabs(xm2-xn2)>1e-9)
                   	x1=xm1-(xm1-xn1)*(xm2-x2)/(xm2-xn2);
               else
                  	x1=xm1;
               if(fabs(ym2-yn2)>1e-9)
                    y1=ym1-(ym1-yn1)*(ym2-y2)/(ym2-yn2);
               else
                  	y1=ym1;
               firering->perimpoints[m].x1=x1;
               firering->perimpoints[m].y1=y1;
               firering->perimpoints[m].x2=x2;
               firering->perimpoints[m].y2=y2;
               firering->perimpoints[m].Status=1;
               firering->perimpoints[m].SizeMult=1.0;
               firering->perimpoints[m].hist=firering->perimpoints[p].hist;
		     firering->perimpoints[m].Area=-1.0;
              	firering->mergepoints[i].VertexID=m;
               for(r=j; r<firering->NumFires; r++)
                    firering->NumPoints[r]+=1;
          }
          if(tempperim)
               delete[] tempperim;
     }
     else
     {    CopyMemory(firering->mergepoints, tempmerge, k*sizeof(MergePoints));
          firering->NumMergePoints[1]=0;
     }
     if(tempmerge)
        	delete[] tempmerge;
          /*
          m=0;
          for(r=0; r<firering->NumFires; r++)
          {    if(r==0)
                    s=0;
               else
                    s=firering->NumPoints[r-1];
          for(i=s; i<firering->NumPoints[r]; i++)      // for each perim point
          {    if(m>=NewTotal)      				// safety valve
          		m--;
          	CopyMemory(&firering->perimpoints[m++], &tempperim[i], sizeof(PerimPoints));
          	for(j=0; j<k; j++)
               {    if(tempmerge[j].VertexID==-(i+1))
               	{    q=1;
                         do
                    	{	p=j-(q++);
	                    	if(p<0)
     	                    	p+=k;
                         }while(tempmerge[p].VertexID<0);
                         q=1;
                         do
                         {  	n=j+(q++);
                    		if(n>k-1)
                    			n-=k;
                         }while(tempmerge[n].VertexID<0);

                    	x2=tempmerge[j].x;
                    	y2=tempmerge[j].y;
                    	xm2=tempmerge[p].x;
                    	ym2=tempmerge[p].y;
                    	xn2=tempmerge[n].x;
                    	yn2=tempmerge[n].y;
                    	xm1=tempperim[tempmerge[p].VertexID].x1;//firering->perimpoints[tempmerge[p].VertexID].x1;
                    	ym1=tempperim[tempmerge[p].VertexID].y1;//firering->perimpoints[tempmerge[p].VertexID].y1;
                    	xn1=tempperim[tempmerge[n].VertexID].x1;// firering->perimpoints[tempmerge[n].VertexID].x1;
                    	yn1=tempperim[tempmerge[n].VertexID].y1;//firering->perimpoints[tempmerge[n].VertexID].y1;
					if(fabs(xm2-xn2)>1e-9)
	                    	x1=xm1-(xm1-xn1)*(xm2-x2)/(xm2-xn2);
                         else
                         	x1=xm1;
                         if(fabs(ym2-yn2)>1e-9)
	                    	y1=ym1-(ym1-yn1)*(ym2-y2)/(ym2-yn2);
                         else
                         	y1=ym1;
                    	firering->perimpoints[m].x1=x1;
                    	firering->perimpoints[m].y1=y1;
                    	firering->perimpoints[m].x2=x2;
                    	firering->perimpoints[m].y2=y2;
                         firering->perimpoints[m].Status=1;
                         firering->perimpoints[m].SizeMult=1.0;
                         firering->perimpoints[m].hist=tempperim[tempmerge[p].VertexID].hist;
		              	firering->perimpoints[m].Area=-1.0;
                    	tempmerge[j].VertexID=m++;
                         if(m>=NewTotal)      				// safety valve
          				m--;

                    	break;
                    }
               }
          }
          firering->NumPoints[r]=m;
          }
          if(tempperim)
          	delete[] tempperim;
     }
     CopyMemory(firering->mergepoints, tempmerge, k*sizeof(MergePoints));
     if(tempmerge)
     	delete[] tempmerge;
     firering->NumMergePoints[0]=k;
     firering->NumMergePoints[1]=0;
    */
}


void PostFrontal::CorrectFireRing(long NumIsects, long *isects, double *ipoint, long *fires, long NumPerims, long NewPoints)
{
// corrects the firering on a single fire for overlaps.  Does this by first splicing
// the intersection points at the edges of the overlap into the firering array.  Then
// it flags each itersection point with a 2.  Next it IDs all other points that
// are currently on the outer edge of the fire with a 1 by comparing the outermost
// edge of the firering with the current corrected perimeters for each fire (that
// includes the main fire front and all inward burning fires that were created
// in the crossover correction (prior to this step).  Then it reorders the perimeter if
// the first point on the fire is inside an overlapping portion. Finally it calls the
// FillOuterRing function to correctly pair-up the intersections that define the overlaps,
// which calls the InterpolateOverlaps function, and so on...

     bool Found, WriteIt;
	long i, j, k, m, n, NumInsert, TotalPts, SumStatus;
     long NPOld, NPNew, NPOut, CurStat, NumFire, FireType;
     long Start, StartOld, FirstCross;
	double xpt, ypt, x1a, x1b, x2a, x2b, y1a, y1b, y2a, y2b;
     double xold, yold, xnew, ynew, dist;

     FireRing *firering, **newring=0;
	PerimPoints *tempperim;

     if(ReferenceFireRingNum==-1)
     	return;
     firering=GetRing(ReferenceFireRingNum);	//-1

//     if(NewPoints<firering->NumPoints[0])
//     	RemoveDuplicatePoints(firering);
//     else if(NewPoints>firering->NumPoints[0])       // fix up firering because of new points added in Intersect::OrganizeCrosses
//     {
     	UpdateFireRing(firering, fires[0], NewPoints);

	     for(j=0; j<firering->NumPoints[0]; j++)
	     {	if(firering->perimpoints[j].Status>0)	// new intersection point added in UpdateMergeOrder
			{	firering->perimpoints[j].Status=0;
               }
          }
//     }
//     else
//     	UpdatePointOrder(firering, fires[0]); // if point number is correct, then only check order of firering

     tempperim=new PerimPoints[(firering->NumPoints[0]+NumIsects*2)];
     Start=-1;
     TotalPts=0;
     NumInsert=0;

     // splice intersection points into perimeter array
	for(i=0; i<NumIsects; i++)
     {	FirstCross=isects[i*2];
     	//SecondCross=isects[i*2+1];
          Start++;
     	for(j=Start; j<=FirstCross; j++)
          {	CopyMemory(&tempperim[j+NumInsert], &firering->perimpoints[j], sizeof(PerimPoints));
           	TotalPts++;
          }
        	Start=FirstCross;
          x1a=tempperim[FirstCross+NumInsert].x1;
		x2a=tempperim[FirstCross+NumInsert].x2;
		y1a=tempperim[FirstCross+NumInsert].y1;
		y2a=tempperim[FirstCross+NumInsert].y2;
          k=FirstCross+1;
          if(k>firering->NumPoints[0]-1)
          	k-=firering->NumPoints[0];
          x1b=firering->perimpoints[k].x1;
          x2b=firering->perimpoints[k].x2;
          y1b=firering->perimpoints[k].y1;
          y2b=firering->perimpoints[k].y2;

		if(pow2(ipoint[i*5]-x2a)+pow2(ipoint[i*5+1]-y2a)<1e-10)
          {	ipoint[i*5]=x2a-0.01*(x2a-x2b);
          	ipoint[i*5+1]=y2a-0.01*(y2a-y2b);
          }
		else if(pow2(ipoint[i*5]-x2b)+pow2(ipoint[i*5+1]-y2b)<1e-10)
          {	ipoint[i*5]=x2b-0.01*(x2b-x2a);
          	ipoint[i*5+1]=y2b-0.01*(y2b-y2a);
          }

          if(fabs(x2a-x2b)>1e-10)
	          xpt=x1a-(x1a-x1b)*(x2a-ipoint[i*5])/(x2a-x2b);
          else
             	xpt=x1a;
          if(fabs(y2a-y2b)>1e-10)
	          ypt=y1a-(y1a-y1b)*(y2a-ipoint[i*5+1])/(y2a-y2b);
          else
              	ypt=y1a;

          NumInsert++;
		tempperim[FirstCross+NumInsert].x1=xpt;
		tempperim[FirstCross+NumInsert].y1=ypt;
	     tempperim[FirstCross+NumInsert].x2=ipoint[i*5];
		tempperim[FirstCross+NumInsert].y2=ipoint[i*5+1];
	     tempperim[FirstCross+NumInsert].hist=firering->perimpoints[k].hist;
	     tempperim[FirstCross+NumInsert].SizeMult=1.0;
          tempperim[FirstCross+NumInsert].Status=2;
          tempperim[FirstCross+NumInsert].Area=-1.0;
	     TotalPts++;
     }
     for(j=Start+1; j<firering->NumPoints[0]; j++)
     {	CopyMemory(&tempperim[j+NumInsert], &firering->perimpoints[j], sizeof(PerimPoints));
        	TotalPts++;
     }
	delete[] firering->perimpoints;
     firering->perimpoints=new PerimPoints[TotalPts];
     CopyMemory(firering->perimpoints, tempperim, TotalPts*sizeof(PerimPoints));
     firering->NumPoints[0]=TotalPts;

	//find out which perim points are still on the new perimeter, flag with 1
	NPNew=GetNumPoints(fires[0]);
     NPOld=NPOut=firering->NumPoints[0]=TotalPts;
     for(k=0; k<NPNew; k++)
     {	xnew=GetPerimVal(2, fires[0], k, XCOORD);
     	ynew=GetPerimVal(2, fires[0], k, YCOORD);
          for(j=0; j<NPOld; j++)	//(j=StartOld; j<NPOld+EndOld; j++)
          {    CurStat=firering->perimpoints[j].Status;
              	if(CurStat!=1)			// 1 means match was found
	          {    xold=firering->perimpoints[j].x2;
				yold=firering->perimpoints[j].y2;
                   	dist=pow2(xold-xnew)+pow2(yold-ynew);
	               if(dist<1e-12 && CurStat!=2)
				{	firering->perimpoints[j].Status=1;
                    	NPOut--;

                         break;
                    }
               }
          }
     }

	// Allocate New Rings and move inward burning perim segments to newrings
     if(NumPerims>1)
     {	newring=new FireRing*[NumPerims];
	     newring[0]=firering;
     	for(k=1; k<NumPerims; k++)
     		newring[k]=SpawnFireRing(fires[k], NPOut*2, firering->StartTime, firering->ElapsedTime);
     	for(k=1; k<NumPerims; k++)
          {    Found=false;
               m=0;
               for(n=k-1; n>-1; n--)
               {  	if(Found)
               		break;
                   	j=0;
                   	xnew=GetPerimVal(1, fires[k], j, XCOORD);
     			ynew=GetPerimVal(1, fires[k], j, YCOORD);
                   	for(i=0; i<newring[n]->NumPoints[0]; i++)	//(j=StartOld; j<NPOld+EndOld; j++)
	     		{	xold=newring[n]->perimpoints[i].x2;
					yold=newring[n]->perimpoints[i].y2;
                        	dist=pow2(xold-xnew)+pow2(yold-ynew);
	          		if(dist<1e-12 && Found==true)     // same pt and 1st already found
                        		break;
	          		else if(dist<1e-12 && Found==false) // same pt and 1st not found
                        		WriteIt=true;
                        	else if(dist>=1e-12 && Found==true) // not same pt and 1st found
						WriteIt=true;
                        	else if(newring[n]->perimpoints[i].Status==1)  // already on outside
                        		WriteIt=false;
                         else
                            	WriteIt=false;
                       	if(WriteIt)
                        	{    CopyMemory(&newring[k]->perimpoints[m], &newring[n]->perimpoints[i], sizeof(PerimPoints));
                              if(Found)
                              {	newring[n]->perimpoints[i].Status=-1;
              	              		newring[n]->perimpoints[i].SizeMult=0.0;
    	              	         		newring[n]->perimpoints[i].Area=-1.0;
                              }
                              else
                                  	newring[k]->perimpoints[m].Status=1;
                              newring[k]->perimpoints[m].SizeMult=0.0;  // make all of them zero
                              m++;
         	                    Found=true;
                        	}
                   	}
               }
	          newring[k]->NumPoints[0]=m;
          }
     }

     // flag
     for(i=1; i<NumPerims; i++)
     {	NPNew=GetNumPoints(fires[i]);
          for(j=0; j<NPNew; j++)
     	{	xnew=GetPerimVal(1, fires[i], j, XCOORD);
	     	ynew=GetPerimVal(1, fires[i], j, YCOORD);
			for(k=0; k<newring[i]->NumPoints[0]; k++)
			{    if(newring[i]->perimpoints[k].Status==1)
               		continue;
               	xold=newring[i]->perimpoints[k].x2;
				yold=newring[i]->perimpoints[k].y2;
                    dist=pow2(xold-xnew)+pow2(yold-ynew);
                    if(dist<1e-12)
                    {    if(newring[i]->perimpoints[k].Status!=2)
	                    	newring[i]->perimpoints[k].Status=1;
                    	newring[i]->perimpoints[k].SizeMult=1.0;
                         break;
                    }
          	}
          }
     }

     for(k=1; k<NumPerims; k++)
	{	RemoveRingEnclaves(newring[k]);
		FillOuterRing(newring[k]);
	     FillMergeArray(newring[k]);
     }

//reorder perimeter if first outside points have been eliminated (status flag==0)

     SumStatus=0;
     for(i=0; i<TotalPts; i++)
     {	if(firering->perimpoints[i].Status<2)
     		SumStatus+=abs(firering->perimpoints[i].Status);  // abs to accomodate -1
     	else
          {	if(SumStatus<i)		// there was a zero status perim point
			{    for(j=0; j<TotalPts; j++)
               		tempperim[j].Status=firering->perimpoints[j].Status;
               	m=0;				// new-order point counter
               	for(j=TotalPts-1; j>(-1); j--)
               	{	if(firering->perimpoints[j].Status==2)
                    	{	for(k=j; k<TotalPts; k++)
                         		//CopyMemory(&tempperim[m++], &firering->perimpoints[k], sizeof(PerimPoints));
                         		CopyMemory(&firering->perimpoints[m++], &tempperim[k], sizeof(PerimPoints));
                    		for(k=0; k<j; k++)
                         		//CopyMemory(&tempperim[m++], &firering->perimpoints[k], sizeof(PerimPoints));
                         		CopyMemory(&firering->perimpoints[m++], &tempperim[k], sizeof(PerimPoints));
						break;
                         }
                    }
			     //CopyMemory(firering->perimpoints, tempperim, TotalPts*sizeof(PerimPoints));
	               break;			// reordering done, get out
               }
               else
	               break;
          }
     }

	if(tempperim)
	     delete[] tempperim;

     if(NumPerims>1)
     	RemoveRingEnclaves(firering);
	FillOuterRing(firering);

     FillMergeArray(firering);
     if(newring)
     	delete[] newring;
}


void PostFrontal::RemoveDuplicatePoints(FireRing *firering)
{
	long i, j, NumPts;
     double xpt, ypt, xn, yn, dist;

     NumPts=firering->NumPoints[0];
     xpt=firering->perimpoints[0].x2;
     ypt=firering->perimpoints[0].y2;
     for(i=1; i<=NumPts; i++)
     {    j=i;
     	if(j==NumPts)
          	j=0;
     	xn=firering->perimpoints[j].x2;
	     yn=firering->perimpoints[j].y2;
		dist=pow2(xpt-xn)+pow2(ypt-yn);
          if(dist<1e-12)
          {	MoveMemory(&firering->perimpoints[j], &firering->perimpoints[j+1],
          		(NumPts-j-1)*sizeof(PerimPoints));
          	if(i<NumPts-1)
	          	NumPts--;
          }
          xpt=xn;
          ypt=yn;
     }
     firering->NumPoints[0]=NumPts;
}


void PostFrontal::FillMergeArray(FireRing *firering)
{
	long i, j, k, NumPoints, StartPt;

     NumPoints=0;
     for(i=0; i<firering->NumPoints[firering->NumFires-1]; i++)      // countem
     {	if(firering->perimpoints[i].SizeMult==1.0)
     		NumPoints++;
     }
     if(firering->mergepoints)
     	delete[] firering->mergepoints;
     firering->mergepoints=new MergePoints[NumPoints];
     firering->NumMergePoints[0]=NumPoints;
     firering->NumMergePoints[1]=0;
     k=0;
     for(i=0; i<firering->NumFires; i++)
     {    StartPt=0;
     	if(i>0)
          	StartPt=firering->NumPoints[i-1];
     	for(j=StartPt; j<firering->NumPoints[i]; j++)
	     {	if(firering->perimpoints[j].SizeMult==1.0)
     		{    firering->perimpoints[j].Status=1;
               	firering->mergepoints[k].Status=0;
               	firering->mergepoints[k].FireID=i;
     			firering->mergepoints[k].VertexID=j;
				firering->mergepoints[k].x=firering->perimpoints[j].x2;
				firering->mergepoints[k++].y=firering->perimpoints[j].y2;
     	     }
          }
     }
}


void PostFrontal::RemoveRingEnclaves(FireRing *ring)
{
	long i, j, k;
     long IslandPts=0, NewPts=0;
     PerimPoints *tempperim;

     tempperim=new PerimPoints[ring->NumPoints[ring->NumFires-1]];

	for(i=0; i<ring->NumFires; i++)
     {	k=0;
     	if(i>0)
          {	k=ring->NumPoints[i-1];
          	ring->NumPoints[i-1]-=IslandPts;
          }
          for(j=k; j<ring->NumPoints[i]; j++)
          {	if(ring->perimpoints[j].Status<0)
          		IslandPts++;
          	else
               {	CopyMemory(&tempperim[NewPts],
               		  &ring->perimpoints[j], sizeof(PerimPoints));
               	NewPts++;
               }
          }
     }

     ring->NumPoints[ring->NumFires-1]=NewPts;//-=IslandPts;
     delete[] ring->perimpoints;
     ring->perimpoints=new PerimPoints[NewPts];
     CopyMemory(ring->perimpoints, tempperim, NewPts*sizeof(PerimPoints));
     delete[] tempperim;
}


void PostFrontal::FillOuterRing(FireRing *firering)
{
// goes through list of intersections and pairs-up those that define an overlapping
// area, either a single loop or a double loop (the only two kinds).  Once IDd,
// the intersections are copied to the tempperim array and processed through the
// InterpolateOverlaps function that determines the fractional area occupied
// by each quadrilateral relative to the actual area of the overlapped area
// this will be used to reduce the post-frontal combustion later on.

     bool Exit;
	long i, j, k;
     long Start1, End1, Start2, Num1, Num2; //End2;
     long CurStat, StoreCount;
     double x1a, x2a, x2b, y1a, y2a, y2b;
     double dist;

	PerimPoints *tempperim;

     Exit=false;
     for(i=0; i<firering->NumPoints[0]; i++)
     {    do         		// loop until beginning of overlapped section
	     {    CurStat=firering->perimpoints[i++].Status;
          	if(i>firering->NumPoints[0]-1)
               {	Exit=true;
               	break;
               }
     	} while(CurStat<2);
          if(Exit)
          	break;

     	Start1=i-1;
          Num1=1;	// needs to be 2 because j==i
          for(j=i; j<firering->NumPoints[0]; j++)
          {    CurStat=firering->perimpoints[j].Status;
               Num1++;
               if(CurStat==2)
               	break;
          }
          if(j>=firering->NumPoints[0])
               j=firering->NumPoints[0]-1;       //debugging
          End1=i=j;
          x2a=firering->perimpoints[Start1].x2;
		y2a=firering->perimpoints[Start1].y2;
		x2b=firering->perimpoints[End1].x2;
		y2b=firering->perimpoints[End1].y2;
          dist=pow2(x2a-x2b)+pow2(y2a-y2b);
          if(dist<1e-8)				// if single intersect point
          {	tempperim=new PerimPoints[Num1];
               for(k=0; k<Num1; k++)
               {	j=Start1+k;
                 	CopyMemory(&tempperim[k], &firering->perimpoints[j], sizeof(PerimPoints));
               }
               InterpolateOverlaps(tempperim, Num1);
               if(Verts)
	               delete[] Verts;
               for(k=0; k<Num1; k++)
   	          {    j=Start1+k;
               	firering->perimpoints[j].x2=tempperim[k].x2;
		          firering->perimpoints[j].y2=tempperim[k].y2;
                    firering->perimpoints[j].SizeMult=tempperim[k].SizeMult;
	          	firering->perimpoints[j].Status=tempperim[k].Status;
                    firering->perimpoints[j].Area=tempperim[k].Area;
                    firering->perimpoints[j].hist=tempperim[k].hist;
               }
          }
          else							   // if double intersect point
          {    Exit=false;
               do                               // find Start2=End1;
              	{	do
		          {    ++j;
                    	if(j>firering->NumPoints[0]-1)
					{	Exit=true;
                         	break;
                         }
                    	CurStat=firering->perimpoints[j].Status;
	         		} while(CurStat<2);
               	if(Exit)
                    	break;
				x1a=firering->perimpoints[j].x2;
				y1a=firering->perimpoints[j].y2;
                    dist=pow2(x1a-x2b)+pow2(y1a-y2b);
               } while(dist>1e-9);
              	if(Exit)
                   	break;
			Start2=j;
               Num2=1;
               do
              	{	do
		          {	++j;
                    	if(j>firering->NumPoints[0]-1)
					{	Exit=true;
                         	break;
                         }
                    	CurStat=firering->perimpoints[j].Status;
                         Num2++;
	         		} while(CurStat<2);
               	if(Exit)
                    	break;
				x1a=firering->perimpoints[j].x2;
				y1a=firering->perimpoints[j].y2;
                    dist=pow2(x1a-x2a)+pow2(y1a-y2a);
               } while(dist>1e-9);
			//End2=j;
              	if(Exit)
                   	break;
          	tempperim=new PerimPoints[Num1+Num2];
               for(StoreCount=0; StoreCount<Num1; StoreCount++)
               {    k=StoreCount+Start1;
                   	CopyMemory(&tempperim[StoreCount], &firering->perimpoints[k], sizeof(PerimPoints));
               }
               for(StoreCount=0; StoreCount<Num2; StoreCount++)
               {    k=StoreCount+Start2;
                   	CopyMemory(&tempperim[Num1+StoreCount], &firering->perimpoints[k], sizeof(PerimPoints));
               }
               InterpolateOverlaps(tempperim, Num1+Num2);
               if(Verts)
	               delete[] Verts;
               for(StoreCount=0; StoreCount<Num1; StoreCount++)
               {    k=Start1+StoreCount;
                    if(tempperim[StoreCount].SizeMult<1e-6)
                         tempperim[StoreCount].SizeMult=1e-6;
                    else if(tempperim[StoreCount].SizeMult>1.0)
                         tempperim[StoreCount].SizeMult=1.0;
                    firering->perimpoints[k].SizeMult=tempperim[StoreCount].SizeMult;
                    firering->perimpoints[k].Status=tempperim[StoreCount].Status;
                    firering->perimpoints[k].Area=-1.0;//tempperim[StoreCount].Area;
               }
               for(StoreCount=0; StoreCount<Num2; StoreCount++)
               {    k=Start2+StoreCount;
                    if(tempperim[Num1+StoreCount].SizeMult<1e-6)
                         tempperim[Num1+StoreCount].SizeMult=1e-6;
                    else if(tempperim[Num1+StoreCount].SizeMult>1.0)
                         tempperim[Num1+StoreCount].SizeMult=1.0;
                    firering->perimpoints[k].SizeMult=tempperim[Num1+StoreCount].SizeMult;
                    firering->perimpoints[k].Status=tempperim[Num1+StoreCount].Status;
                    firering->perimpoints[k].Area=-1.0;//tempperim[Num1+StoreCount].Status
               }
          }
		if(tempperim)
         	     delete[] tempperim;
     }
}


void PostFrontal::InterpolateOverlaps(PerimPoints *verts, long numverts)
{
// uses a non-spatial technique for aportioning area to each set of vertices
// by the fraction of the original area required to achieve the
// correct area but only by quadrilateral sections that are not spatially accurate

     bool Written;
	long i, j, k;
     double p, q, nx1, ny1;
     double Dir1, Dir2, xdiff, ydiff, sindiff;
     double SubArea, AFract, SuperArea, TargetArea, TargetPerim;
     double a, b, c, d, part1, part2;

	// clean up segment crosses by inserting intersection point when cross is found
     SuperArea=0.0;
	for(i=1; i<numverts; i++)
     {    j=i-1;
     	if(verts[j].Status==2 && verts[i].Status==2)
          	continue;
          if(pow2(verts[j].x1-verts[i].x1)+pow2(verts[j].y1-verts[i].y1)>1e-9)
	     {	if(Cross(verts[j].x1, verts[j].y1, verts[j].x2, verts[j].y2, true,
	          	verts[i].x1, verts[i].y1, verts[i].x2, verts[i].y2, true, &nx1, &ny1))
	          /*{    nx1=verts[j].x2;
               	ny1=verts[j].y2;
               	verts[j].x2=verts[i].x2;		// replace points with intersection
     	     	verts[j].y2=verts[i].y2;
          		verts[i].x2=nx1;
          		verts[i].y2=ny1;
               }
               */
	          {	verts[j].x2=nx1;		// replace points with intersection
     	     	verts[j].y2=ny1;
          		verts[i].x2=nx1;
          		verts[i].y2=ny1;
               }
          }
          a=pow2(verts[j].x1-verts[j].x2)+pow2(verts[j].y1-verts[j].y2);
		b=pow2(verts[j].x1-verts[i].x1)+pow2(verts[j].y1-verts[i].y1);
		c=pow2(verts[i].x1-verts[i].x2)+pow2(verts[i].y1-verts[i].y2);
		d=pow2(verts[i].x2-verts[j].x2)+pow2(verts[i].y2-verts[j].y2);
		p=pow2(verts[j].x1-verts[i].x2)+pow2(verts[j].y1-verts[i].y2);
		q=pow2(verts[i].x1-verts[j].x2)+pow2(verts[i].y1-verts[j].y2);
          part1=4.0*p*q;
          part2=pow2(b+d-a-c);
          if(part2<part1)
          	SuperArea+=0.25*sqrt(part1-part2);
     }
     Verts=new double[numverts*2];
     for(i=0; i<numverts; i++)
     {    Verts[i*2]=verts[i].x1;
          Verts[i*2+1]=verts[i].y1;
     }

     /*
     k=0;
     for(i=0; i<numverts; i++)
	{	if(verts[i].Status==2)
     		k++;
     }
     Verts=new double[(numverts+k)*2];
	if(!Verts)
     	return;
     j=0;
     for(i=0; i<numverts; i++)
     {    Written=false;
     	if(verts[i].Status==2)
     	{  	switch(j)
	     	{    case 0:
	               	Verts[(i+j)*2]=verts[i].x2;
     		     	Verts[(i+j)*2+1]=verts[i].y2;
	          	     j++;
                         Written=true;
                         break;
               	case 2:
	               	Verts[(i+j)*2]=verts[i].x2;
     		     	Verts[(i+j)*2+1]=verts[i].y2;
	          	     j++;
                         Written=true;
                         break;
	          }
          }

          Verts[(i+j)*2]=verts[i].x1;
          Verts[(i+j)*2+1]=verts[i].y1;

    	     if(!Written && verts[i].Status==2)
     	{    switch(j)
	     	{    case 1:
	          	     j++;
	               	Verts[(i+j)*2]=verts[i].x2;
     		     	Verts[(i+j)*2+1]=verts[i].y2;
                         break;
               	case 3:
	          	     j++;
	               	Verts[(i+j)*2]=verts[i].x2;
     		     	Verts[(i+j)*2+1]=verts[i].y2;
                         break;
	          }
          }
     }
     AreaPerim(numverts+k, Verts, &TargetArea, &TargetPerim);
     */

     AreaPerim(numverts, Verts, &TargetArea, &TargetPerim);
     TargetArea=fabs(TargetArea);

	// compute proportioned area for each point
     x1=verts[0].x1;
     y1=verts[0].y1;
     xn1=verts[0].x2;
     yn1=verts[0].y2;
	for(i=1; i<numverts; i++)
     {	x2=verts[i].x1;
	     y2=verts[i].y1;
          xn2=verts[i].x2;
          yn2=verts[i].y2;
          xdiff=x1-xn2;
		ydiff=y1-yn2;
     	p=sqrt(pow2(xdiff)+pow2(ydiff));
		if(p>1e-9)
		    	Dir1=atan2(ydiff, xdiff);
		else
     		Dir1=0.0;
		xdiff=x2-xn1;
		ydiff=y2-yn1;
		q=sqrt(pow2(xdiff)+pow2(ydiff));
	     if(q>1e-9)
	     	Dir2=atan2(ydiff, xdiff);
		else
     		Dir2=0.0;
		sindiff=sin(Dir1)*cos(Dir2)-cos(Dir1)*sin(Dir2);
          SubArea=fabs(0.5*p*q*sindiff);
          if(SubArea<1e-9)
          	continue;
          if(SuperArea<1e-9)
               SuperArea=SubArea+TargetArea;
          if(SuperArea>1e-9)
          	AFract=SubArea/SuperArea*TargetArea/SuperArea;
          else
               AFract=1.0;
          if(AFract<0.0)
               AFract=0.0;
          else if(AFract>1.0)
               AFract=1.0;
          verts[i-1].SizeMult=AFract;
          verts[i-1].Status=1;
          if(verts[i].Status==2)      // if current point is on the edge of overlap
          {    verts[i].Status=1;
          	i++;
               if(i>numverts-1)
               	continue;
          	x1=verts[i].x1;
		     y1=verts[i].y1;
		     xn1=verts[i].x2;
		     yn1=verts[i].y2;
          }
          else
          {    x1=x2;
	          y1=y2;
     	     xn1=xn2;
          	yn1=yn2;
          }
     }
}


bool PostFrontal::Cross(double xpt1, double ypt1, double xpt2, double ypt2, bool OnSpan1,
		         double xpt1n, double ypt1n, double xpt2n, double ypt2n, bool OnSpan2,
                   double *newx, double *newy)
{
// computes crosses between two spans defined by their endpoint
// returns true if the intersection occurrs within the segment
// and updates the intersection point newx,newy regardless

	double xdiff1, ydiff1, xdiff2, ydiff2, ycept1, ycept2;
	double slope1, slope2, ycommon, xcommon;
	bool Intersection=false;

	xdiff1=xpt2-xpt1;
	ydiff1=ypt2-ypt1;
     if(fabs(xdiff1)<1e-9)
     	xdiff1=0.0;
	if(xdiff1!=0.0)
	{	slope1=ydiff1/xdiff1;
		ycept1=ypt1-(slope1*xpt1);
	}
	else
	{	slope1=1.0;                     // SLOPE NON-ZERO
          ycept1=xpt1;
     }
	xdiff2=xpt2n-xpt1n;
	ydiff2=ypt2n-ypt1n;
     if(fabs(xdiff2)<1e-9)
     	xdiff2=0.0;
	if(xdiff2!=0.0)
	{	slope2=ydiff2/xdiff2;
		ycept2=ypt1n-(slope2*xpt1n);
	}
	else
	{	slope2=1.0;					// SLOPE NON-ZERO
          ycept2=xpt1n;
     }
	if(fabs(slope1-slope2)<1e-9)
	{    if(fabs(ycept1-ycept2)<1e-9)
          {    if(xdiff1==0.0 && xdiff2==0.0)
               {    if(OnSpan1 && OnSpan2)
                    {    if(((ypt1<=ypt1n && ypt1>ypt2n) || (ypt1>=ypt1n && ypt1<ypt2n)) &&
                            ((ypt1n<=ypt1 && ypt1n>ypt2) || (ypt1n>=ypt1 && ypt1n<ypt2)))
                         {    Intersection=true;
                              *newx=xpt1;
                              *newy=ypt1;
                         }
                    }
                    else if(OnSpan1)
                    {    if((ypt1<=ypt1n && ypt1>ypt2n) || (ypt1>=ypt1n && ypt1<ypt2n))
                         {    Intersection=true;
                              *newx=xpt1;
                              *newy=ypt1;
                         }
                    }
                    else if(OnSpan2)
                    {    if((ypt1n<=ypt1 && ypt1n>ypt2) || (ypt1n>=ypt1 && ypt1n<ypt2))
                         {    Intersection=true;
                              *newx=xpt1n;
                              *newy=ypt1n;
                         }
                    }
               }
               else
               {    if(OnSpan1 && OnSpan2)
                    {    if(((xpt1<=xpt1n && xpt1>xpt2n) || (xpt1>=xpt1n && xpt1<xpt2n)) &&
                            ((xpt1n<=xpt1 && xpt1n>xpt2) || (xpt1n>=xpt1 && xpt1n<xpt2)))
                         {    Intersection=true;
                              *newx=xpt1;
                              *newy=ypt1;
                         }
                    }
                    else if(OnSpan1)
                    {    if((xpt1<=xpt1n && xpt1>xpt2n) || (xpt1>=xpt1n && xpt1<xpt2n))
                         {    Intersection=true;
                              *newx=xpt1;
                              *newy=ypt1;
                         }
                    }
                    else if(OnSpan2)
                    {    if((xpt1n<=xpt1 && xpt1n>xpt2) || (xpt1n>=xpt1 && xpt1n<xpt2))
                         {    Intersection=true;
                              *newx=xpt1n;
                              *newy=ypt1n;
                         }
                    }
               }
          }
	}
	else
	{	if(xdiff1!=0.0 && xdiff2!=0.0)
		{	xcommon=(ycept2-ycept1)/(slope1-slope2);
			*newx=xcommon;
			ycommon=ycept1+slope1*xcommon;
			*newy=ycommon;
               if(OnSpan1 && OnSpan2)
               {	if((*newx>=xpt1 && *newx<=xpt2) || (*newx<=xpt1 && *newx>=xpt2))
				{	if((*newx>=xpt1n && *newx<=xpt2n) || (*newx<=xpt1n && *newx>=xpt2n))
						Intersection=true;
				}
               }
               else if(OnSpan1)
               {	if((*newx>=xpt1 && *newx<=xpt2) || (*newx<=xpt1 && *newx>=xpt2))
					Intersection=true;
               }
               else if(OnSpan2)
               {	if((*newx>=xpt1n && *newx<=xpt2n) || (*newx<=xpt1n && *newx>=xpt2n))
					Intersection=true;
               }
		}
		else
		{	if(xdiff1==0.0 && xdiff2!=0.0)
			{	ycommon=slope2*xpt1+ycept2;
				*newx=xpt1;
				*newy=ycommon;
                    if(OnSpan1 && OnSpan2)
               	{	if((*newy>=ypt1 && *newy<=ypt2) || (*newy<=ypt1 && *newy>=ypt2))
					{	if((*newx>=xpt1n && *newx<=xpt2n) || (*newx<=xpt1n && *newx>=xpt2n))
							Intersection=true;
					}
	               }
	               else if(OnSpan1)
	               {	if((*newy>=ypt1 && *newy<=ypt2) || (*newy<=ypt1 && *newy>=ypt2))
						Intersection=true;
	               }
	               else if(OnSpan2)
	               {	if((*newx>=xpt1n && *newx<=xpt2n) || (*newx<=xpt1n && *newx>=xpt2n))
						Intersection=true;
	               }
				//if((*newy>ypt1 && *newy<ypt2) || (*newy<ypt1 && *newy>ypt2))
				//{	if((*newx>xpt1n && *newx<xpt2n) || (*newx<xpt1n && *newx>xpt2n))
				//		Intersection=true;
				//}
			}
			else
			{	if(xdiff1!=0.0 && xdiff2==0.0)
				{	ycommon=slope1*xpt1n+ycept1;
					*newx=xpt1n;
					*newy=ycommon;
					if(OnSpan1 && OnSpan2)
	               	{   	if((*newy>=ypt1n && *newy<=ypt2n) || (*newy<=ypt1n && *newy>=ypt2n))
						{	if((*newx>=xpt1 && *newx<=xpt2) || (*newx<=xpt1 && *newx>=xpt2))
								Intersection=true;
						}
		          	}                                                                     
		               else if(OnSpan1)
	               	{   	if((*newx>=xpt1 && *newx<=xpt2) || (*newx<=xpt1 && *newx>=xpt2))
							Intersection=true;
		          	}
	          	     else if(OnSpan2)
	               	{   	if((*newy>=ypt1n && *newy<=ypt2n) || (*newy<=ypt1n && *newy>=ypt2n))
							Intersection=true;
		          	}
               	    	//if((*newy>ypt1n && *newy<ypt2n) || (*newy<ypt1n && *newy>ypt2n))
					//{	if((*newx>xpt1 && *newx<xpt2) || (*newx<xpt1 && *newx>xpt2))
					//		Intersection=true;
					//}
				}
			}
		}
	}

	return Intersection;
}


void PostFrontal::AreaPerim(long NumPoints, double *verts, double *area, double *perimeter)
{
	// calculates area and perimeter as a planimeter (with triangles)

	long i, j;
	double xpt1, ypt1, xpt2, ypt2, aangle, zangle;
	double newarea, xdiff, ydiff, DiffAngle, hdist;

	*area=0.0;
	*perimeter=0.0;
	if(NumPoints>0)
	{    StartX=verts[0];     // must use old perim array
		StartY=verts[1];     // new array not merged or clipped yet
		j=0;
		while(j<NumPoints)
		{    j++;
			xpt1=verts[j*2];
			ypt1=verts[j*2+1];
			zangle=Direction(xpt1, ypt1);        // reference angle
			if(zangle!=999.9)	// make sure that startx,starty!=x[0]y[0]
				break;
		}
		xdiff=xpt1-StartX;
		ydiff=ypt1-StartY;
		hdist=sqrt(pow2(xdiff)+pow2(ydiff));
		*perimeter=hdist;
		j++;
		for(i=j; i<NumPoints; i++)
		{	xpt2=verts[i*2];
			ypt2=verts[i*2+1];
			xdiff=xpt2-xpt1;
			ydiff=ypt2-ypt1;
			hdist=sqrt(pow2(xdiff)+pow2(ydiff));
			*perimeter+=hdist;
			newarea=.5*(StartX*ypt1-xpt1*StartY+xpt1*ypt2-xpt2*ypt1+xpt2*StartY-StartX*ypt2);
			newarea=fabs(newarea);
			aangle=Direction(xpt2, ypt2);
			if(aangle!=999.9)
			{	DiffAngle=aangle-zangle;
				if(DiffAngle>PI)
					DiffAngle=-(2.0*PI-DiffAngle);
				else if(DiffAngle<-PI)
					DiffAngle=(2.0*PI+DiffAngle);
				if(DiffAngle>0.0)
					*area-=newarea;
				else if(DiffAngle<0.0)
					*area+=newarea;
				zangle=aangle;
			}
			xpt1=xpt2;
			ypt1=ypt2;
		}
		//*area/=(10000.0*pow2(MetricResolutionConvert()));		// ha always
		//*perimeter/=(1000.0*MetricResolutionConvert());       	// km always
	}
}


double PostFrontal::Direction(double xpt, double ypt)
{
	// calculates sweep direction for angle determination

	double zangle=999.9, xdiff, ydiff;

	xdiff=xpt-StartX;
	ydiff=ypt-StartY;
     if(fabs(xdiff)<1e-9)
          xdiff=0.0;
     if(fabs(ydiff)<1e-9)
          ydiff=0.0;

	if(xdiff!=0.0)
	{	zangle=atan(ydiff/xdiff);
		if(xdiff>0.0)
			zangle=(PI/2.0)-zangle;
		else
			zangle=(3.0*PI/2.0)-zangle;
	}
	else
	{	if(ydiff>=0.0)
			zangle=0;
		else if(ydiff<0.0)
			zangle=PI;
	}
	return zangle;
}


long PostFrontal::Overlap(long NumPoints, double *verts)
{
	// determines if point is inside or outside a fire polygon defined by verts

	long i, j, k, inside=0;
	double Aangle=0.0, Bangle;
	double CuumAngle=0.0, DiffAngle;
	double xpt, ypt;

	j=0;
	while(j<NumPoints)    // make sure that startx,starty!=x[0]y[0]
	{	xpt=verts[j*2];
		ypt=verts[j*2+1];
		Aangle=Direction(xpt, ypt);
		j++;
		if(Aangle!=999.9)
			break;
	}
	for(i=j; i<=NumPoints; i++)
	{    if(i==NumPoints)
			k=j-1;
		else
			k=j;
		xpt=verts[k*2];
		ypt=verts[k*2+1];
		Bangle=Direction(xpt, ypt);
		if(Bangle!=999.9)
		{	DiffAngle=Bangle-Aangle;
			if(DiffAngle>PI)
				DiffAngle=-(2.0*PI-DiffAngle);
			else if(DiffAngle<-PI)
				DiffAngle=(2.0*PI+DiffAngle);
			CuumAngle-=DiffAngle;
			Aangle=Bangle;
		}
	}
	if(fabs(CuumAngle)>PI)        // if absolute value of CuumAngle is>PI
		inside=1;                // then point is inside polygon

	return inside;
}


FireRing *PostFrontal::SpawnFireRing(long NumFire, long NumAlloc, double Start, double Elapsed)
{// loads first information into new fire ring structure
	long i, NumPoints;
     double x, y;
     FireRing* firering;

     NumPoints=GetNumPoints(NumFire);
	if((firering=AllocFireRing(NumAlloc, Start, Elapsed+Start))==NULL)
     	return NULL;
	//firering->StartTime=Start;
	//firering->ElapsedTime=Elapsed;
     firering->OriginalFireNumber=NumFire;
	firering->NumPoints[0]=NumPoints;

	for(i=0; i<NumPoints; i++)
	{    x=GetPerimeter1Value(NumFire, i, XCOORD);
          y=GetPerimeter1Value(NumFire, i, YCOORD);
     	firering->perimpoints[i].x2=x;
     	firering->perimpoints[i].y2=y;
          firering->perimpoints[i].x1=x;
     	firering->perimpoints[i].y1=y;
          firering->perimpoints[i].SizeMult=1.0;
          firering->perimpoints[i].Area=-1.0;
          firering->perimpoints[i].Status=0;
          firering->perimpoints[i].l1=firering->perimpoints[i].l2=firering->perimpoints[i].h=0.0;
     }
     //BurnupFireRings(NumRings-1, NumRings);
     ReferenceFireRingNum=NumRings;   // store for CorrectFireRing

     return firering;
}



void PostFrontal::MergeFireRings(long *Fires, long NumPerims, long *isects,
						double *ipoints, long NumIsects, long CurNumPts)
{
// corrects a fire ring after merging of two fires by comparing the post-merger
// outer perimeter with the pre-merger outer perimeters of the two fires,
// Fire1 and Fire2.  Once the same points have been found, and intersection
// points have been added to the pre-merger perimeters, both pre-merger perimeters
// are copied into the structure for Fire1.  Fire2 is eliminated.  Fire1 is then
// processed through the interpolation routine to apportion the overlapping area
// among the points that define its edge.  If a vertex is already in an overlap and
// again falls within one of these new overlaps, it's area influence factor (SizeMult)
// is reduced to zero (a point can only be in 1 overlap at a time).

     bool Reverse=false;
     long i, j, k, m, n, p, s, SumStatus, FireType, NumPoints;
     long Start, End, First, StartPt, EndPt, NumFire, FirstCross, TotalPts, TotalFires;
	double xold, yold, xpt, ypt;
     FireRing *testring, *ring[2]={0, 0}, *newring;
     PerimPoints *tempperim;
     MergePoints *tempmerge;
     long *tempnum, TempMergeAlloc;
     long OldNum[2];


// find the correct fires
	for(i=MergeReferenceRingNum; i<GetNumRings(); i++)
     {	testring=GetRing(i);
     	if(!ring[0])
          {	if(Fires[0]==testring->OriginalFireNumber)
               {	ring[0]=testring;
                    continue;
          	}
          }
          if(!ring[1])
          {    if(Fires[1]==testring->OriginalFireNumber)
                    ring[1]=testring;
          }
     	if(ring[0] && ring[1])
	      	break;
     }

	for(i=0; i<2; i++)
     {	if(ring[i]->NumMergePoints[0]==0)
		{	if(GetNumPoints(ring[i]->OriginalFireNumber)!=ring[i]->NumPoints[0])       // fix up firering because of new points added in Intersect::OrganizeCrosses
		      	UpdateFireRing(ring[i], ring[i]->OriginalFireNumber, GetNumPoints(ring[i]->OriginalFireNumber));
	      	else
	      		UpdatePointOrder(ring[i], ring[i]->OriginalFireNumber); 		   // if point number is correct, then only check order of firering
	     }
     	FillMergeArray(ring[i]);
     }

// reorder ring[0] && ring[1] to match current order of Fire1 and Fire2
	OldNum[0]=ring[0]->NumMergePoints[0];
	OldNum[1]=ring[1]->NumMergePoints[0];          
  	UpdateMergeOrder(ring[0], Fires[0]);
     UpdateMergeOrder(ring[1], Fires[1]);
                                               
// allocate temporary data structures
     TempMergeAlloc=(ring[0]->NumMergePoints[0]+ring[1]->NumMergePoints[0]+NumIsects*2);
     tempmerge=new MergePoints[TempMergeAlloc];
     ZeroMemory(tempmerge, (ring[0]->NumMergePoints[0]+ring[1]->NumMergePoints[0]+NumIsects*2)*sizeof(MergePoints));
	TotalPts=0;
     for(i=0; i<2; i++)
     {	for(j=0; j<ring[i]->NumFires; j++)
     		TotalPts+=ring[i]->NumPoints[j];
     }
     tempperim=new PerimPoints[TotalPts+NumIsects*2];
     ZeroMemory(tempperim, (TotalPts+NumIsects*2)*sizeof(PerimPoints));

// insert intersection points into mergepoint array
	TotalPts=0;
    	double temp[5];
     long q1, q2;

	for(i=0; i<2; i++)                    // for each ring
     {    switch(i)
	     {	case 0:
          		NumFire=Fires[0];
                    break;
          	case 1:
               	NumFire=Fires[1];
                    if(isects[1]>isects[((NumIsects-1)*2)+1]) // reverse 'em
                    {    Reverse=true;
                    	for(j=0; j<NumIsects/2; j++)
	                    {	m=isects[j*2+1];
						isects[j*2+1]=isects[(NumIsects-j)*2-1];
						isects[(NumIsects-j)*2-1]=m;
                              q1=j*5;
                              q2=(NumIsects-j)*5-5;
				        	CopyMemory(temp, &ipoints[q1], 5*sizeof(double));
			               CopyMemory(&ipoints[q1], &ipoints[q2], 5*sizeof(double));
			               CopyMemory(&ipoints[q2], temp, 5*sizeof(double));
     	               }
                    }
                    break;
     	}
		Start=0;
		for(j=0; j<NumIsects; j++)
     	{	FirstCross=isects[j*2+i];
     		for(k=Start; k<=FirstCross; k++)
	          	CopyMemory(&tempmerge[TotalPts++], &ring[i]->mergepoints[k], sizeof(MergePoints));
               if(FirstCross>=Start)
          	   	Start=FirstCross+1;
               tempmerge[TotalPts].Status=2;
		     tempmerge[TotalPts].x=ipoints[j*5];
		     tempmerge[TotalPts].y=ipoints[j*5+1];
		     tempmerge[TotalPts].FireID=-1;                    // flag as unknown for now
               tempmerge[TotalPts].VertexID=-(NumFire+1);        // flag as neg number of fire
               TotalPts++;
	     }
	     for(j=Start; j<ring[i]->NumMergePoints[0]; j++)
	     	CopyMemory(&tempmerge[TotalPts++], &ring[i]->mergepoints[j], sizeof(MergePoints));
     }

	delete[] ring[0]->mergepoints;
     ring[0]->mergepoints=new MergePoints[TotalPts];
     CopyMemory(ring[0]->mergepoints, tempmerge, TotalPts*sizeof(MergePoints));
     ring[0]->NumMergePoints[0]+=NumIsects;
     ring[0]->NumMergePoints[1]=ring[0]->NumMergePoints[0]+ring[1]->NumMergePoints[0]+NumIsects;//TotalPts;
     delete[] ring[1]->mergepoints;
     ring[1]->mergepoints=0;
     ring[1]->NumMergePoints[0]=0;

	if(Reverse)     // need to restore original order
     {	for(j=0; j<NumIsects/2; j++)
	     {	m=isects[j*2+1];
			isects[j*2+1]=isects[(NumIsects-j)*2-1];
			isects[(NumIsects-j)*2-1]=m;
               q1=j*5;
               q2=(NumIsects-j)*5-5;
	        	CopyMemory(temp, &ipoints[q1], 5*sizeof(double));
               CopyMemory(&ipoints[q1], &ipoints[q2], 5*sizeof(double));
               CopyMemory(&ipoints[q2], temp, 5*sizeof(double));
     	}
     }

// now insert intersection points into ring[0] and ring[1] arrays and store together in tempperim
     TotalFires=ring[0]->NumFires+ring[1]->NumFires;
     tempnum=new long[TotalFires];
     ZeroMemory(tempnum, TotalFires*sizeof(long));
     TotalFires=0;
     for(i=0; i<2; i++)   // initialize numpoints
     {	for(j=0; j<ring[i]->NumFires; j++)
     	{	tempnum[TotalFires]=ring[i]->NumPoints[j];
			if(i>0)
	     		tempnum[TotalFires]+=ring[0]->NumPoints[ring[0]->NumFires-1];
          	TotalFires++;
          }
     }

     TotalPts=0;
     for(i=0; i<2; i++)
     {	CopyMemory(&tempperim[TotalPts], &ring[i]->perimpoints[0],
     		ring[i]->NumPoints[ring[i]->NumFires-1]*sizeof(PerimPoints));
     	TotalPts+=ring[i]->NumPoints[ring[i]->NumFires-1];
     }

     for(i=0; i<2; i++)            // for each merged ring
     {	if(i==0)
          {    j=0;
           	Start=0;
          }
     	else
          {	j=ring[0]->NumMergePoints[0];
               Start=tempnum[ring[0]->NumFires-1];
          }
     	for(k=j; k<ring[0]->NumMergePoints[i]; k++)
     	{	if(ring[0]->mergepoints[k].Status==2)	// if new intersection point
               {    x2=ring[0]->mergepoints[k].x;
			     y2=ring[0]->mergepoints[k].y;
               	s=k-1;
                    if(i==0)
                    {  	if(s<0)
                     		s=ring[0]->NumMergePoints[0]-1;
                    }
                    else if(s<ring[0]->NumMergePoints[0])
                    	s=ring[0]->NumMergePoints[1]-1;
               	xpt=ring[0]->mergepoints[s].x;
               	ypt=ring[0]->mergepoints[s].y;
                    p=0;
               	for(m=Start; m<TotalPts; m++)
               	{   	xm2=tempperim[m].x2;
				  	ym2=tempperim[m].y2;
                         if(m>=tempnum[p])
                         	p++;
                        	if((pow2(xpt-xm2)+pow2(ypt-ym2))<1e-9)	// point same
                         {    if(m<TotalPts-1)
	                         	MoveMemory(&tempperim[m+2], &tempperim[m+1], (TotalPts-m-1)*sizeof(PerimPoints));
                              TotalPts++;
                              for(n=0; n<TotalFires; n++)
                         	{    if(m<tempnum[n])
                              		tempnum[n]++;	// increase number of points in fire
                              }
                         	tempperim[m+1].x2=x2;
                              tempperim[m+1].y2=y2;
                              n=m+2;
                         	if(n>tempnum[p]-1)
                              {	if(p==0)
                              		n=0;
                              	else
	                              	n=tempnum[p-1]; //n=0;
                              }
                              xn2=tempperim[n].x2;
                              yn2=tempperim[n].y2;
                              xm1=tempperim[m].x1;
                              ym1=tempperim[m].y1;
						xn1=tempperim[n].x1;
                              yn1=tempperim[n].y1;

						if(fabs(xn2-xm2)>1e-9)
	          				x1=xn1-(xn1-xm1)*(xn2-x2)/(xn2-xm2);
	          			else
     	        				x1=xn1;
	     	     		if(fabs(yn2-ym2)>1e-9)
		     	     		y1=yn1-(yn1-ym1)*(yn2-y2)/(yn2-ym2);
          				else
              					y1=yn1;
                              tempperim[m+1].x1=x1;
                              tempperim[m+1].y1=y1;
		     			tempperim[m+1].SizeMult=1.0;
     	     			tempperim[m+1].Status=2;
                         	tempperim[m+1].Area=0.0;
                         	tempperim[m+1].l1=tempperim[m+1].l2=tempperim[m+1].h=-1;
                         	tempperim[m+1].hist=tempperim[n].hist;
                              break;
                         }
                    }
               }
          }
     }

	delete[] ring[0]->perimpoints;
     ring[0]->perimpoints=new PerimPoints[TotalPts];
     CopyMemory(ring[0]->perimpoints, tempperim, TotalPts*sizeof(PerimPoints));

// aggregate descriptive data for new ring[0], incl. numfires, numpts etc
	//for(j=0; j<ring[1]->NumFires; j++)
     //	tempnum[ring[0]->NumFires+j]+=tempnum[ring[0]->NumFires-1];

     ring[0]->NumFires+=ring[1]->NumFires;
     delete[] ring[0]->NumPoints;
     ring[0]->NumPoints=new long[ring[0]->NumFires];
	for(i=0; i<ring[0]->NumFires; i++)
     	ring[0]->NumPoints[i]=tempnum[i];
     delete[] tempnum;
     delete[] tempperim;
     delete[] ring[1]->perimpoints;
     ring[1]->perimpoints=0;
     delete[] ring[1]->NumPoints;
     ring[1]->NumPoints=0;

// update mergfire array with correct vertexnumbers and fire numbers;
	//ring[0]->NumMergePoints[1]+=ring[0]->NumMergePoints[0];  // accumulate NumMergePoints

	for(i=0; i<2; i++)				// for each set of fires
     {    switch(i)
     	{	case 0:
          		Start=0;
                    End=ring[0]->NumFires-ring[1]->NumFires;
                    StartPt=0;
                    break;
          	case 1:
               	Start=End;
                    End=ring[0]->NumFires;
                    StartPt=ring[0]->NumMergePoints[0];
                    break;
          }
    		for(j=StartPt; j<ring[0]->NumMergePoints[i]; j++)	     // for each mergepoint
          {	x2=ring[0]->mergepoints[j].x;
          	y2=ring[0]->mergepoints[j].y;
          	for(k=Start; k<End; k++)						// for each perimfire
               {	if(k==0)
               		First=0;
               	else
                    	First=ring[0]->NumPoints[k-1];
                    for(m=First; m<ring[0]->NumPoints[k]; m++)
                    {	xn2=ring[0]->perimpoints[m].x2;
                    	yn2=ring[0]->perimpoints[m].y2;
	    	               if(pow2(x2-xn2)+pow2(y2-yn2)<1e-10) 		// test for same point
     	         	     {	ring[0]->mergepoints[j].Status-= (short) 4;		// flag it
          	         		ring[0]->mergepoints[j].FireID=k;
               	          ring[0]->mergepoints[j].VertexID=m;
                              k=End;
	                         break;
     	               }
                    }
               }
          }
     }

     for(i=0; i<ring[0]->NumMergePoints[1]; i++)
     {	if(ring[0]->mergepoints[i].Status<0)
         		ring[0]->mergepoints[i].Status+= (short) 4;  // unflag it
          else
          	ring[0]->mergepoints[i].Status=0;	// only for debugging
     }

// relabel status of mergepoints by comparing with current perimeter2 array
     for(m=1; m<NumPerims; m++)
	{	NumFire=Fires[m];
     	if(m<2)
     	{    FireType=2;
          	NumPoints=CurNumPts;
               newring=0;
          }
     	else
          {	FireType=1;
			NumPoints=GetNumPoints(NumFire);
              	newring=SpawnFireRing(NumFire, NumPoints, ring[0]->StartTime, ring[0]->ElapsedTime);
     	}
		Start=0;
		for(i=0; i<NumPoints; i++)
	     {	xpt=GetPerimVal(FireType, NumFire, i, XCOORD);
     		ypt=GetPerimVal(FireType, NumFire, i, YCOORD);
          	for(j=abs(Start); j<2; j++)
	          {    StartPt=0;
               	if(j>0)
          	     	StartPt=ring[0]->NumMergePoints[0];
	          	for(k=StartPt; k<ring[0]->NumMergePoints[j]; k++)
     	     	{    //if(ring[0]->mergepoints[k].Status!=1)
          	     	//{
                              xold=ring[0]->mergepoints[k].x;
	          	     	yold=ring[0]->mergepoints[k].y;
     	          	     if(pow2(xpt-xold)+pow2(ypt-yold)<1e-10)
          	          	{	if(m==0 && ring[0]->mergepoints[k].Status==2)
	                         	{    Start=abs(Start)-1;         // alternate fires for searching
                                   	j=2;
     	                    		break;
          	                    }
               	     		if(newring)
                              	{    n=ring[0]->mergepoints[k].VertexID;
                                   	newring->perimpoints[i].x1=ring[0]->perimpoints[n].x1;
                              		newring->perimpoints[i].y1=ring[0]->perimpoints[n].y1;
                              		newring->perimpoints[i].hist=ring[0]->perimpoints[n].hist;
								if(ring[0]->mergepoints[k].Status!=2)
		               	     	{	ring[0]->perimpoints[n].Status=-1;
		               	     		ring[0]->perimpoints[n].Area=-1;
                                        	ring[0]->perimpoints[n].SizeMult=1.0;
                                             ring[0]->mergepoints[k].Status=-1;
                                        }
	                              }
                                   else
								ring[0]->mergepoints[k].Status=1;
                                   j=2; // force exit from loop
                                 	break;
	                    	}
                         //}
                    }
               }
          }
          if(newring)
          	FillMergeArray(newring);
     }

// re-order arrays if 1st point is inside
     SumStatus=0;
     for(i=0; i<ring[0]->NumMergePoints[0]; i++)
     {	if(ring[0]->mergepoints[i].Status<2)
     		SumStatus+=abs(ring[0]->mergepoints[i].Status);
     	else
          {	if(SumStatus<i)		// there was a zero status perim point
			{    for(j=0; j<ring[0]->NumMergePoints[0]; j++)
               	{	tempmerge[j].Status=ring[0]->mergepoints[j].Status;
                    	tempmerge[j].FireID=ring[0]->mergepoints[j].FireID;
                        	tempmerge[j].VertexID=ring[0]->mergepoints[j].VertexID;
                    }
               	m=0;				// new-order point counter
               	for(j=ring[0]->NumMergePoints[0]-1; j>(-1); j--)
               	{	if(ring[0]->mergepoints[j].Status==2)
                    	{	for(k=j; k<ring[0]->NumMergePoints[0]; k++)
                         		CopyMemory(&ring[0]->mergepoints[m++], &tempmerge[k], sizeof(MergePoints));
                    		for(k=0; k<j; k++)
                         		CopyMemory(&ring[0]->mergepoints[m++], &tempmerge[k], sizeof(MergePoints));
						break;
                         }
                    }
	               break;			// reordering done, get out
               }
               else
	               break;
          }
     }

     PartitionMergeRing(ring[0]);
     if(NumPerims>2)
     	RemoveRingEnclaves(ring[0]);
     //FillMergeArray(ring[0]);

// update mergepoints array with exact points in perim2 array for outer fire
     if(CurNumPts>TempMergeAlloc)
     {    delete[] tempmerge;
          tempmerge=new MergePoints[CurNumPts];
     }
     if(CurNumPts>ring[0]->NumMergePoints[1])
     {    delete[] ring[0]->mergepoints;
          ring[0]->mergepoints=new MergePoints[CurNumPts];
     }
     for(i=0; i<CurNumPts; i++)
     {	xpt=GetPerimVal(2, 0, i, XCOORD);
     	ypt=GetPerimVal(2, 0, i, YCOORD);
          for(j=0; j<ring[0]->NumMergePoints[1]; j++)
          {    xold=ring[0]->mergepoints[j].x;
          	yold=ring[0]->mergepoints[j].y;
               if(pow2(xpt-xold)+pow2(ypt-yold)<1e-10)
               {   	tempmerge[i].x=xpt;
          		tempmerge[i].y=ypt;
          		tempmerge[i].Status=0;
          		tempmerge[i].FireID=0;
          		tempmerge[i].VertexID=ring[0]->mergepoints[i].VertexID;
                    break;
               }
          }
     }

     CopyMemory(ring[0]->mergepoints, tempmerge, CurNumPts*sizeof(MergePoints));
     delete[] tempmerge;
     ring[1]->OriginalFireNumber=-1;
     ring[0]->NumMergePoints[0]=CurNumPts;
     ring[0]->NumMergePoints[1]=0;

}


void PostFrontal::PartitionMergeRing(FireRing *firering)
{
// goes through list of intersections and pairs-up those that define an overlapping
// area, either a single loop or a double loop (the only two kinds).  Once IDd,
// the intersections are copied to the tempperim array and processed through the
// InterpolateOverlaps function that determines the fractional area occupied
// by each quadrilateral relative to the actual area of the overlapped area
// this will be used to reduce the post-frontal combustion later on.

     bool Exit, TestIt;
	long i, j, k, m[2]={0, 0}, n, o, p, q, r;
     long Start[2];
     long End[2]={0, 0};
     long Num[2]={0, 0};
     long CurStat, StoreCount;
     double xa[2], ya[2], xb[2], yb[2];

	PerimPoints *tempperim;

     Exit=false;
	Start[0]=0;
   	Start[1]=firering->NumMergePoints[0];
	do
	{	for(k=0; k<2; k++)
     	{    for(i=Start[k]; i<firering->NumMergePoints[k]; i++)
	     	{    do         		// loop until beginning of overlapped section
		     	{    CurStat=firering->mergepoints[i++].Status;
                         m[k]++;
		          	if(i>firering->NumMergePoints[k]-1)
		               {	Exit=true;
	     	          	break;
	          	     }
                         if(CurStat==2 && k==1)
                         {	x1=firering->mergepoints[i-1].x;
						y1=firering->mergepoints[i-1].y;
                              if(pow2(x1-xb[0])+pow2(y1-yb[0])<1e-10)
                              	break;
                              else
                              	CurStat=1;
                         }
		     	} while(CurStat<2);
		          if(Exit)
		          	break;

                    firering->mergepoints[i-1].Status-= (short) 4;
	     		Start[k]=i-1;
		          Num[k]=1;	// needs to be 2 because j==i

                    if(k==1)
                    	i=firering->NumMergePoints[0];
    	               for(j=i; j<firering->NumMergePoints[k]; j++)
		          {    CurStat=firering->mergepoints[j].Status;
                         m[k]++;
	     	          Num[k]++;
                         if(CurStat==2)
                         {    if(k==0)
                         		break;
                         	else
                              {   	x1=firering->mergepoints[j].x;
							y1=firering->mergepoints[j].y;
     	                         if(pow2(x1-xa[0])+pow2(y1-ya[0])<1e-10)
          	                    	break;
               	               else
                    	          	CurStat=1;
                              }
                         }
		          }
                    if(j>=firering->NumMergePoints[k])
                         End[k]=firering->NumMergePoints[k]-1;  // debugging
		          End[k]=j;
                    firering->mergepoints[j].Status-=(short) 4;
	          	xa[k]=firering->mergepoints[Start[k]].x;
		     	ya[k]=firering->mergepoints[Start[k]].y;
			     xb[k]=firering->mergepoints[End[k]].x;
				yb[k]=firering->mergepoints[End[k]].y;
                    break;
               }
          }
          if(Exit)
          	break;

		i=End[1]+1;
          if(i>firering->NumMergePoints[1]-1)
          	i=firering->NumMergePoints[0];
          if(firering->mergepoints[i].Status==0)
          {	Num[1]=Start[1]-End[1]+1;
               k=Start[1];
               Start[1]=End[1];
               End[1]=k;
          }
          else if(Start[1]>End[1])
          {    Num[1]=firering->NumMergePoints[1]-Start[1]+(End[1]-firering->NumMergePoints[0])+1;
               k=Start[1];
               Start[1]=End[1];
               End[1]=k;
          }
          else
          	Num[1]=End[1]-Start[1]+1;


	     tempperim=new PerimPoints[Num[0]+Num[1]];
          for(StoreCount=0; StoreCount<Num[0]; StoreCount++)
          {    k=StoreCount+Start[0];           // must be start
               if(k>firering->NumMergePoints[0]-1)
               	k-=firering->NumMergePoints[0];
			n=firering->mergepoints[k].VertexID;
              	CopyMemory(&tempperim[StoreCount], &firering->perimpoints[n], sizeof(PerimPoints));
          }
          for(StoreCount=0; StoreCount<Num[1]; StoreCount++)
          {    k=StoreCount+Start[1];//End[1];
               if(k>firering->NumMergePoints[1]-1)
               {	k-=firering->NumMergePoints[1];
                    k+=firering->NumMergePoints[0];
               }
			n=firering->mergepoints[k].VertexID;
              	CopyMemory(&tempperim[Num[0]+StoreCount], &firering->perimpoints[n], sizeof(PerimPoints));
          }
          InterpolateOverlaps(tempperim, Num[0]+Num[1]);
          for(StoreCount=0; StoreCount<Num[0]; StoreCount++)
          {    k=StoreCount+Start[0];           // must be start
               if(k>firering->NumMergePoints[0]-1)
               	k-=firering->NumMergePoints[0];
			n=firering->mergepoints[k].VertexID;
               if(tempperim[StoreCount].SizeMult<1e-6)
                     tempperim[StoreCount].SizeMult=1e-6;
               else if(tempperim[StoreCount].SizeMult>1.0)
                     tempperim[StoreCount].SizeMult=1.0;
              	firering->perimpoints[n].SizeMult=tempperim[StoreCount].SizeMult;
              	firering->perimpoints[n].Status=tempperim[StoreCount].Status;//1;
          }
          for(StoreCount=0; StoreCount<Num[1]; StoreCount++)
          {    k=StoreCount+Start[1];//End[1];
               if(k>firering->NumMergePoints[1]-1)
               {	k-=firering->NumMergePoints[1];
                    k+=firering->NumMergePoints[0];
               }
			n=firering->mergepoints[k].VertexID;
               if(tempperim[Num[0]+StoreCount].SizeMult<1e-6)
                     tempperim[Num[0]+StoreCount].SizeMult=1e-6;
               else if(tempperim[Num[0]+StoreCount].SizeMult>1.0)
                     tempperim[Num[0]+StoreCount].SizeMult=1.0;
              	firering->perimpoints[n].SizeMult=tempperim[Num[0]+StoreCount].SizeMult;
               firering->perimpoints[n].Status=tempperim[Num[0]+StoreCount].Status;
          }

          // find other points inside of overlap and set to zero influence
     	for(o=0; o<firering->NumPoints[firering->NumFires-1]; o++)
          {    TestIt=true;
          	if(firering->perimpoints[o].SizeMult<1.0)
          	{    for(p=0; p<2; p++)
               	{    for(q=0; q<Num[p]; q++)
                    	{    r=q+Start[p];
                              if(r>firering->NumMergePoints[1]-1)
                                   break;
                              n=firering->mergepoints[r].VertexID;
                          	if(o==n)
                              {	//q=Num[p]+1;
                              	//p=2;
                                   TestIt=false;
                                   break;
                              }
                         }
                         if(!TestIt)
                         	break;
                    }
                    if(TestIt)
                    {    StartX=firering->perimpoints[o].x2;
                    	StartY=firering->perimpoints[o].y2;
                    	if(Overlap(Num[0]+Num[1]+4, Verts))
                    		firering->perimpoints[o].SizeMult=0.0;
                    }
               }
          }
          Start[0]=End[0];	// not Start[1]
		if(tempperim)
         	     delete[] tempperim;
          if(Verts)
	          delete[] Verts;
     } while(m[0]<firering->NumMergePoints[0]);
}


double PostFrontal::GetPerimVal(long PerimLocation, long NumFire, long PointNum, long TYPE)
{
// wraps the global get-functions for fire perimeters
     double value;

	switch(PerimLocation)
     {	case 1:		// new inward burning fire, so look in perimeter1
     		value=GetPerimeter1Value(NumFire, PointNum, TYPE);
     		break;
     	case 2:		// external fire perimeter, so look on perimeter2
     		value=GetPerimeter2Value(PointNum, TYPE);
     		break;
     }

     return value;
}


double PostFrontal::ComputeSmoke(double CurrentTime, long Species)
{

	return 0.0;
}

double PostFrontal::ComputeHeat(double CurrentTime)
{

	return 0.0;
}


void PostFrontal::GetRingPoint(FireRing *ring, long PointNum)
{
	x1=ring->perimpoints[PointNum].x1;
	y1=ring->perimpoints[PointNum].y1;
	x2=ring->perimpoints[PointNum].x2;
	y2=ring->perimpoints[PointNum].y2;
}

void PostFrontal::CloseAllThreads()
{
	long m;//, prevct;

     if(NumPFI>0)
     {	for(m=0; m<NumPFI; m++)
          	pfi[m].SetRange(NULL, 0, 0.0, -1, -1);
	     for(m=0; m<NumPFI; m++)
     		pfi[m].StartIntegThread(m);
	     Sleep(50);
     	WaitForFarsiteEvents(EVENT_INTEG, NumPFI, true, INFINITE); //2000
	}
     FreePFI();
}


bool PostFrontal::AllocPFI()
{
	if(NumPFI==GetMaxThreads())
     	return true;

	CloseAllThreads();
	pfi=new PFIntegration[GetMaxThreads()];

     if(pfi)
     {	NumPFI=GetMaxThreads();
//     	AllocFarsiteEvents(EVENT_INTEG, NumPFI, "SyncEventInteg", false, false);

		return true;
     }

     return false;
}


void PostFrontal::FreePFI()
{
     if(pfi)
     	delete[] pfi;//GlobalFree(pfi);
     pfi=0;
     NumPFI=0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
// Integration functions, members of PFINtegration
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

PFIntegration::PFIntegration()
{
	hThread=0;
     //hIntegEvent=0;
     ThreadStarted=false;
     Begin=-1;
     End=-1;
     ring=0;
}


PFIntegration::~PFIntegration()
{
}

void PFIntegration::SetRange(FireRing *rring, long firenum, double currenttime, long begin, long end)
{
	Begin=begin;
     End=end;
     ring=rring;
     CurrentTime=currenttime;
     FireNum=firenum;
}


HANDLE PFIntegration::StartIntegThread(long ID)
{
	if(ThreadStarted==false)
	{	//char sID[16]="";

          ThreadOrder=ID;
          //sprintf(sID, "INTEGEVENT_%02ld", ID);
          hIntegEvent=GetFarsiteEvent(EVENT_INTEG_THREAD, ThreadOrder);
          //if(!hIntegEvent)
	     //     hIntegEvent=CreateEvent(NULL, TRUE, FALSE, sID);
        	hThread=(HANDLE) ::_beginthreadex(NULL, 0, &PFIntegration::RunIntegThread, this, CREATE_SUSPENDED, &ThreadID);
          ResumeThread(hThread);
          CloseHandle(hThread);
          hThread=0;
     }
     else
         SetEvent(hIntegEvent);//SetFarsiteEvent(EVENT_INTEG_THREAD, ThreadOrder);

     return hThread;
}


unsigned __stdcall PFIntegration::RunIntegThread(void *pfi)
{
	static_cast <PFIntegration *> (pfi)->IntegThread();

     return 1;
}


void PFIntegration::IntegThread()
{
     //long prevct;

     ThreadStarted=true;
	do
     {    if(End<0)
     		break;
     	FlameWeightConsumed=SmolderWeightConsumed=0.0;
		Integrate();
          SetFarsiteEvent(EVENT_INTEG, ThreadOrder);	//SetEvent(hIntegSyncEvent[ThreadOrder]);
          WaitForSingleObject(hIntegEvent, INFINITE);
          ResetEvent(hIntegEvent);
          if(Begin<0 || End<0)
          	break;
     } while(End>-1);

	SetFarsiteEvent(EVENT_INTEG, ThreadOrder);	//SetEvent(hIntegSyncEvent[ThreadOrder]);
}


void RingBurn::CalculateArea(FireRing *ring, long pt, long ptl, long ptn)
{
     double Area, l1, l2, h, x, y, FeetToMeters;

     l1=l2=0.0;
	x1=ring->perimpoints[pt].x1;
	y1=ring->perimpoints[pt].y1;
	x2=ring->perimpoints[pt].x2;
	y2=ring->perimpoints[pt].y2;
	GetWholePolygon(ring, ptl, false);
     GetSubPolygon(1.0, false);
     x=pow2(b2+d2-a2-c2);
     y=4.0*q2*p2;
     if(y-x>1e-8)
          Area=0.25*sqrt(y-x);
     else
          Area=0.0;
     if(a2>1e-8)
          l1=sqrt(a2);
     if(c2>1e-8)
          l2=sqrt(c2);
	GetWholePolygon(ring, ptn, false);
     GetSubPolygon(1.0, false);
     x=pow2(b2+d2-a2-c2);
     y=4.0*q2*p2;
     if(y-x>1e-8)
          Area+=0.25*sqrt(y-x);//sqrt(4.0*q2*p2-pow2(b2+d2-a2-c2));
     if(a2>1e-8)
          l1+=sqrt(a2);
     if(c2>1e-8)
          l2+=sqrt(c2);
	if(l1+l2>0.0)
		h=(2.0*Area)/(l1+l2);
     else
     	h=0.0;
     FeetToMeters=pow2(MetricResolutionConvert());

     ring->perimpoints[pt].Area=Area*FeetToMeters;
     ring->perimpoints[pt].l1=l1*MetricResolutionConvert();
     ring->perimpoints[pt].l2=l2*MetricResolutionConvert();
     ring->perimpoints[pt].h=h*MetricResolutionConvert();
}


//void PFIntegration::GetRingPoint(FireRing *ring, long PointNum)
//{
//	x1=ring->perimpoints[PointNum].x1;
//	y1=ring->perimpoints[PointNum].y1;
//	x2=ring->perimpoints[PointNum].x2;
//	y2=ring->perimpoints[PointNum].y2;
//}


void RingBurn::GetWholePolygon(FireRing *ring, long PointNum, bool Reverse)
{
     xn1=ring->perimpoints[PointNum].x1;
     yn1=ring->perimpoints[PointNum].y1;
     xn2=ring->perimpoints[PointNum].x2;
     yn2=ring->perimpoints[PointNum].y2;
     xm1=xn1-(xn1-x1)/2.0;	// mid points on span
     ym1=yn1-(yn1-y1)/2.0;
     xm2=xn2-(xn2-x2)/2.0;
     ym2=yn2-(yn2-y2)/2.0;

	if(!Reverse)
     {	xip=x2;
	     yip=y2;
     	xmip=xm2;
	     ymip=ym2;
     }
     else
     {	xip=x1;
	     yip=y1;
     	xmip=xm1;
	     ymip=ym1;
     }
}


void RingBurn::GetSubPolygon(double end, bool Reverse)
{
	if(!Reverse)
     {	xi=x2-(x2-x1)*end;
	     yi=y2-(y2-y1)*end;
	     xmi=xm2-(xm2-xm1)*end;
	     ymi=ym2-(ym2-ym1)*end;
     }
     else
     {	xi=x1-(x1-x2)*end;
	     yi=y1-(y1-y2)*end;
	     xmi=xm1-(xm1-xm2)*end;
	     ymi=ym1-(ym1-ym2)*end;
     }

     a2=pow2(xi-xmi)+pow2(yi-ymi);
     b2=pow2(xmi-xmip)+pow2(ymi-ymip);
     c2=pow2(xmip-xip)+pow2(ymip-yip);
     d2=pow2(xip-xi)+pow2(yip-yi);
     p2=pow2(xi-xmip)+pow2(yi-ymip);
     q2=pow2(xmi-xip)+pow2(ymi-yip);

     xip=xi;  // for next time around
	yip=yi;
	xmip=xmi;
	ymip=ymi;
}


double PFIntegration::Interpolate(FireRing *ring, long j, double CurrentTime, long type)
{
     long m;//, DistFromEnd;
	float result;
     float TimeNow;
     float *xx, *yy, dy;
     unsigned long ma, pos;

	TimeNow=(float) fabs(CurrentTime);
	if(type==1)
	{    xx=ring->perimpoints[j].hist.FlameCoefX;
	     yy=ring->perimpoints[j].hist.FlameCoefY;
	     ma=ring->perimpoints[j].hist.FlamePolyNum;
		if(TimeNow<=xx[0])
     	{    if(xx[0]<1e-9)
				return 1.0;

          	result=1.0-((1.0-yy[0])*TimeNow/xx[0]);

          	return result;
          }
	     else if(TimeNow>xx[ma-1])
     	{	if(xx[ma-1]<ring->perimpoints[j].hist.WeightCoefX[ring->perimpoints[j].hist.WeightPolyNum-1])
          		return 0.0;
          	else
	          	return yy[ma-1];
          }
     }
     else
     {    xx=ring->perimpoints[j].hist.WeightCoefX;
	     yy=ring->perimpoints[j].hist.WeightCoefY;
	     ma=ring->perimpoints[j].hist.WeightPolyNum;
		if(TimeNow<xx[0])
     		return (double) yy[0];
	     else if(TimeNow>xx[ma-1])
     		return (double) yy[ma-1];
     }

     locate(xx-1, ma, TimeNow, &pos);
     /*
     m=(pos-1);
     if(m<0)
     	m=0;
     DistFromEnd=ma-m;
     if(DistFromEnd<2)
         	result=yy[m]-(yy[m]-yy[m+1])*((xx[m]-TimeNow)/(xx[m]-xx[m+1]));
     else if(DistFromEnd<4)
         	polint(&xx[m]-1, &yy[m]-1, DistFromEnd, TimeNow, &result, &dy);
     else
        	polint(&xx[m]-1, &yy[m]-1, 4, TimeNow, &result, &dy);
     */
	//Linear Interpolation

     m=(pos-1);
     if(m<0)
     	m=0;
	result=yy[m]-(yy[m]-yy[m+1])*((xx[m]-TimeNow)/(xx[m]-xx[m+1]));

	if(result>1.0000000000)
     	result=1.0000000000;

     return (double) result;

}


double PFIntegration::CalculateTemporalIntegrationSteps(FireRing *ring, long j, double *CurrentTime,
	long *NumCalcs, double *BaseFract)

{
     double TimeInc=0.0;
	double Tolerance=WeightLossErrorTolerance(GETVAL);
     double ct, et, st, s_tt, e_tt;
     double t1=0.0, t2;
     double wt1, wt2, wtest, wtmid;

     ct=*CurrentTime;
     st=ring->StartTime;
     et=st+ring->ElapsedTime;//*ring->perimpoints[j].SizeMult;
     s_tt=st+ring->perimpoints[j].hist.TotalTime;
     e_tt=et+ring->perimpoints[j].hist.TotalTime;

	if(ct-et>1e-9)				// later than endtime of quadrangle
     {   	if(ct<=s_tt)		// ct occurs before burnout finished at startpoint of quadrangle
          {    *BaseFract=1.0;
               t1=ct-et;		// most recent time
               t2=ring->perimpoints[j].hist.LastIntegTime;  //ct-          	// oldest time
               if(t2<0.0)
               	t2=0.0;
          }
          else if(ct<=e_tt)	// ct occurs after burnout finished at start but before it finished at endpoint of quadrangle
          {    t1=ct-et;//s_tt-et;
               t2=ring->perimpoints[j].hist.LastIntegTime;//ct-et;
               if(t2<0.0)
               	t2=0.0;
               *BaseFract=(e_tt-ct)/(e_tt-s_tt);
          }
          else 		     // ct occurs after burnout finished after start and endpoints of qudrangle
          {	if(ring->perimpoints[j].hist.LastIntegTime>=ring->perimpoints[j].hist.TotalTime)
          	{	*NumCalcs=1;
               	*BaseFract=0.0;
                    *NumCalcs=0;

                    return 0.0;
               }
              	else
               {    t1=e_tt-et;
               	t2=ring->perimpoints[j].hist.LastIntegTime;
	               if(t2<0.0)
     	          	t2=0.0;
      			*BaseFract=(t1-t2)/(ct-t2);
			}
          }

          // time-based determination of timestep, with timestep =~0.5
     	//*NumCalcs=(long) ((t1-t2)/0.5);

	     // weight error-based determination of timestep
     	wt1=Interpolate(ring, j, t1, 0);
          wt2=Interpolate(ring, j, t2, 0);

	     wtest=(wt1+wt2)/2.0;
          wtmid=Interpolate(ring, j, (t1+t2)/2.0, 0);
	     *NumCalcs=(long) ((fabs(wtest-wtmid)*ring->perimpoints[j].hist.TotalWeight)/Tolerance)+1;	// 2 kg difference maximum

		if(*NumCalcs>200)
          	*NumCalcs=200;
          if(t1<(ring->perimpoints[j].hist.FlameTime))
          {	if(*NumCalcs<10)
          		*NumCalcs=10;
          }

	     TimeInc=(t1-t2)/((double) *NumCalcs);

          //while(TimeInc>ring->perimpoints[j].hist.TotalTime)
          //{    *NumCalcs+=1;
		//     TimeInc=(t1-t2)/((double) *NumCalcs);
          //}

	     if(t1-t2>20.0*ring->ElapsedTime)
          	*BaseFract=1.0;
          else if(t1-t2<0.05*ring->ElapsedTime)
          	*BaseFract=-1.0;
     }
     else	//if(ct==et)	// ct is exactly at the end point of quadrangle
     {	t1=0.0;  	// recent
          t2=ct-st;	// oldest
          *BaseFract=1.0;

	     // weight error-based determination of timestep
		wt1=Interpolate(ring, j, t1, 0);
          wt2=Interpolate(ring, j, t2, 0);
	     wtest=(wt1+wt2)/2.0;
          wtmid=Interpolate(ring, j, (t1+t2)/2.0, 0);
	     *NumCalcs=(long) ((fabs(wtest-wtmid)*ring->perimpoints[j].hist.TotalWeight)/Tolerance)+1;	// 2 kg difference maximum
	     if(*NumCalcs>200)
     		*NumCalcs=200;
          else if(*NumCalcs<5)
         		*NumCalcs=5;

          // time-based determination of timestep with timestep =~0.5
          // *NumCalcs=(long) ((t2-t1)/0.5);

          TimeInc=(t2-t1)/((double) *NumCalcs);

          //while(TimeInc>ring->perimpoints[j].hist.TotalTime)
          //{    *NumCalcs+=1;
		//     TimeInc=(t2-t1)/((double) *NumCalcs);
          //}
     }
     if(t1<1e-3)
     	t1=0.0;
     *CurrentTime=t1;       // produces relative time since end of polygon

     return TimeInc;
}


void PFIntegration::CalculateSpatialIntegrationSteps(FireRing *ring, long j, double CurrentRelativeTime, long *NumCalcs)
{
//	all times in minutes

	double wt1, wt2, wtest, wtmid;
     double Tolerance=WeightLossErrorTolerance(GETVAL);
     double TimeDiff, TimeNow;

	TimeDiff=ring->ElapsedTime*ring->perimpoints[j].SizeMult;
     TimeNow=CurrentRelativeTime;//CurrentTime-ring->StartTime;
     wt1=Interpolate(ring, j, TimeNow, 0);   // calculate most recent value for weight history
     TimeNow=CurrentRelativeTime+TimeDiff;
     wt2=Interpolate(ring, j, TimeNow, 0);
     wtest=(wt1+wt2)/2.0;
	TimeNow=CurrentRelativeTime+TimeDiff/2.0;
     wtmid=Interpolate(ring, j, TimeNow, 0);

     *NumCalcs=(long) ((fabs(wtest-wtmid)*ring->perimpoints[j].hist.TotalWeight)/Tolerance)+1;	// 2 kg difference maximum
	if(*NumCalcs>200)
     	*NumCalcs=200;
     if(*NumCalcs<2)
     	*NumCalcs=2;

    // *NumCalcs=ring->ElapsedTime*ring->perimpoints[j].SizeMult/0.5+1;
}


bool PFIntegration::Integrate()//(FireRing *ring, double CurrentTime, double *FlameWt, double *SmolderWt)
{
 // Integrates the burnup function over time and space.  It starts by calculating
 //	the timesteps required for agiven level of precision for the time-integration (e.g.
 //  differencing the burnup function called at two successive times.  Then for each
 //  time-time step, it determines the spatial-timestep required to integrate over the
 //  area represented by the point's trajectory (which is really just a time-gradient for
 //	dt.  The integration proceeds by computing the surface of the burnup function at
 //	increments of time (for the spatial integration) at intervals defined by the time
 //  integration.
 //
 //
 // CurrentTime in minutes of elapsed simulation time
 // FlameWt is Mg
 // SmolderWt is Mg

	long j, k;//, p;
     long lastpt, nextpt;
     long NumSpatCalcs=1, NumTimeCalcs=1;
     double CurTempTime, TempTimeInc;
     double AllPts_WeightConsumption, AllPts_FlameConsumption;
     double TotalWeight, FlameWeight, TimeFract, RelativeTime, NetWeight;

     AllPts_WeightConsumption=0.0;
     AllPts_FlameConsumption=0.0;

     lastpt=Begin-1;
     if(FireNum==0)
     {    if(Begin==0)
        		lastpt=ring->NumPoints[FireNum]-1;
     }
     else if(lastpt<ring->NumPoints[FireNum-1])
         	lastpt=ring->NumPoints[FireNum]-1;

     for(j=Begin; j<End; j++)
     {    // find number of calclations for integration between StartTime
         	// and StartTime+Elapsed Time for each point trajectory
          //if(ring->perimpoints[j].hist.TotalWeight-ring->perimpoints[j].hist.LastWtRemoved<1e-2) // if a given point already burned out
          if(ring->perimpoints[j].hist.LastIntegTime>=ring->perimpoints[j].hist.TotalTime) // if a given point already burned out
          {	lastpt=j;
               ring->perimpoints[j].hist.CurWtRemoved=0;
          	continue;
          }
          if(ring->perimpoints[j].SizeMult==0.0)				// if the point has been merged out
          {	lastpt=j;
               ring->perimpoints[j].hist.CurWtRemoved=0;
              	continue;
          }
          if(ring->perimpoints[j].hist.WeightPolyNum==0)    	// no post frontal calculations here
          {	lastpt=j;
               ring->perimpoints[j].hist.CurWtRemoved=0;
              	continue;
          }

          nextpt=j+1;
          if(nextpt>ring->NumPoints[FireNum]-1)
          {	if(FireNum==0)
                  	nextpt=0;
             	else
                  	nextpt=ring->NumPoints[FireNum-1];
          }
          if(ring->perimpoints[j].Area==0.0)
              	continue;

          TimeFract=1.0;	// fraction of polygon that is still actively burning
          RelativeTime=CurrentTime;
          TempTimeInc=CalculateTemporalIntegrationSteps(ring, j, &RelativeTime, &NumTimeCalcs, &TimeFract);
          if(TempTimeInc==0.0)
              	continue;
          if(RelativeTime==0.0)
          {    ring->perimpoints[j].hist.CurWtRemoved=0.0;
               ring->perimpoints[j].hist.FlameWtRemoved=0.0;
               for(k=0; k<NumTimeCalcs; k++)
              	{	FirstIntegration(ring, j, TempTimeInc, k+1, &TotalWeight, &FlameWeight);
                    if(TotalWeight>0.0)
			     {	if(ring->perimpoints[j].SizeMult<1.0)
	                    {   	TotalWeight*=ring->perimpoints[j].SizeMult;
                              FlameWeight*=ring->perimpoints[j].SizeMult;
                    	}
			     	NetWeight=((float) TotalWeight)- ring->perimpoints[j].hist.LastWtRemoved;

			          if(NetWeight<0.0)
                             	NetWeight=0.0;
		     	     AllPts_WeightConsumption+=NetWeight; // sum up total weights for all quadrangles
                         ring->perimpoints[j].hist.CurWtRemoved+=NetWeight;
         		          AllPts_FlameConsumption+=FlameWeight;//*NetWeight;
		               ring->perimpoints[j].hist.FlameWtRemoved+=FlameWeight;
                         if(NetWeight>0.0)
						ring->perimpoints[j].hist.LastWtRemoved=TotalWeight; 	// subtract weight burned by point
			     }
               }
               AllPts_WeightConsumption+=ring->perimpoints[j].hist.CrownLoadingBurned/1000.0*ring->perimpoints[j].Area;
               AllPts_FlameConsumption+=ring->perimpoints[j].hist.CrownLoadingBurned/1000.0*ring->perimpoints[j].Area;
               ring->perimpoints[j].hist.CurWtRemoved+=ring->perimpoints[j].hist.CrownLoadingBurned/1000.0*ring->perimpoints[j].Area;
               ring->perimpoints[j].hist.FlameWtRemoved+=ring->perimpoints[j].hist.CrownLoadingBurned/1000.0*ring->perimpoints[j].Area;
          }
          else
          {    CurTempTime=RelativeTime-((double) (NumTimeCalcs-1)*TempTimeInc);
               ring->perimpoints[j].hist.CurWtRemoved=0.0;
               ring->perimpoints[j].hist.FlameWtRemoved=0.0;
               for(k=0; k<NumTimeCalcs; k++)
               {	CalculateSpatialIntegrationSteps(ring, j, CurTempTime, &NumSpatCalcs);
                   	SecondaryIntegration(ring, j, CurTempTime, TempTimeInc, NumSpatCalcs, &TotalWeight, &FlameWeight);

			     if(TotalWeight>0.0)
			     {    if(ring->perimpoints[j].SizeMult<1.0)
	                    {   	TotalWeight*=ring->perimpoints[j].SizeMult;  	// account for diminished size of polygon after mergers
                              FlameWeight*=ring->perimpoints[j].SizeMult;
                    	}
			          AllPts_WeightConsumption+=TotalWeight;		 		// sum up total weights for all quadrangles
                         ring->perimpoints[j].hist.CurWtRemoved+=TotalWeight;
		               ring->perimpoints[j].hist.FlameWtRemoved+=FlameWeight;
         		          AllPts_FlameConsumption+=FlameWeight;
					ring->perimpoints[j].hist.LastWtRemoved=TotalWeight; 	// subtract weight burned by point
			     }
	              	CurTempTime+=TempTimeInc;			// increment means going back in burnup-time history
		     }
          }
          lastpt=j;
   	     ring->perimpoints[j].hist.LastIntegTime=CurrentTime-
              	ring->StartTime-ring->ElapsedTime;
     }
     SmolderWeightConsumed=AllPts_WeightConsumption-AllPts_FlameConsumption;
     FlameWeightConsumed=AllPts_FlameConsumption;

     return true;
}


void PFIntegration::FirstIntegration(FireRing *ring, long j, double TimeInc, long NumCalcs,
								double *Weight, double *Flame)
{
     // Calculates the Weight loss (total and flaming) by integrating the
	// 	Weight-Remaining curve AND Flame Fraction curve - forward in time
     //	over the area and radial distance of the fire quadrangle over the
     // 	given timestep.  This routine begins at the time the fire just enters
     //   the quadrangle and continues until the combustion wave just reaches the
     //	end of the quadrangle.  Here the time and space intervals for the integration
     //	are the same.
     //
     //	*ring		= the fire ring
     //	j			= the point on the fire ring
     //	TimeInc		= the time difference over which the curves are to be integrated
     //	NumCalcs		= the number of spatial/temporal divisions of the curve to use in the integration
     //
     //	returns:	 *Weight (total weight) and *Flame (flame weight).
     //

	long i;
     double Fract, Fract1, OldFract, AreaFract, CuumArea;
     double wt1, wt2, wtmid;
     double ft1_a, ft1_b, ft2_a, ft2_b, ftmid;
     double TempWeight, FlameWeight, TotalWeight;
     double StartTime, CurrentTime;
     double Area, l1, l1c, l2, h, dldh;

	*Weight=*Flame=0.0;
     CuumArea=TotalWeight=FlameWeight=0.0;
     CurrentTime=StartTime=(double) NumCalcs*TimeInc;
     wt1=Interpolate(ring, j, CurrentTime, 0);
     ft1_a=Interpolate(ring, j, CurrentTime, 1);

     l1=l1c=ring->perimpoints[j].l1;
     l2=ring->perimpoints[j].l2;
     h=ring->perimpoints[j].h;
     dldh=(l2-l1)/h;

	// if the end of the curve has past the StartTime, increase number of calcs to maintain
     // definition along the active part of the front
	if(ring->perimpoints[j].hist.TotalTime<StartTime)
     {	NumCalcs+=(long) ((StartTime-ring->perimpoints[j].hist.TotalTime)/TimeInc+1.0);
      	TimeInc=StartTime/(double) NumCalcs;
     }

     OldFract=Fract=0.0;
     for(i=0; i<NumCalcs; i++)
     {	CurrentTime-=(double) TimeInc;	// most recent
	     wt2=Interpolate(ring, j, CurrentTime, 0);
          ft2_a=Interpolate(ring, j, CurrentTime, 1);
		if((CurrentTime+TimeInc)>ring->perimpoints[j].hist.TotalTime)
          {	Fract1=((double) (i+1)*TimeInc)/(double) ring->ElapsedTime;
               AreaFract=(ring->perimpoints[j].hist.TotalTime-CurrentTime)/TimeInc;
     	     if(AreaFract>1e-9)
               {    Fract+=(Fract1-Fract)*AreaFract;
	     		l2=l1+Fract*h*dldh;
                    Area=0.5*(l1c+l2)*(Fract-OldFract)*h;
               	OldFract=Fract;
               }
               else
               	Area=0.0;
               wtmid=(wt1+wt2)/2.0;
	          TempWeight=(1.0-wtmid)*Area*ring->perimpoints[j].hist.TotalWeight;

	     	l2=l1+Fract1*h*dldh;
               Area=0.5*(l1c+l2)*(Fract1-OldFract)*h;
               if(Area<1e-9)
                   	Area=0.0;
	          TempWeight+=Area*ring->perimpoints[j].hist.TotalWeight;

               OldFract=Fract1;
          }
          else
          {  	Fract=((double) (i+1)*TimeInc)/(double) ring->ElapsedTime;
               l2=l1+Fract*h*dldh;
               Area=0.5*(l1c+l2)*(Fract-OldFract)*h;
               wtmid=(wt1+wt2)/2.0;
	          TempWeight=(1.0-wtmid)*Area*ring->perimpoints[j].hist.TotalWeight;
               AreaFract=1.0;
			OldFract=Fract;
          }
          TotalWeight+=TempWeight;
          ft2_b=Interpolate(ring, j, CurrentTime+TimeInc, 1);
          if(ft2_b>0.0 || ft2_a>0.0)
          {	ft1_b=Interpolate(ring, j, CurrentTime, 1);
               ftmid=(ft1_a+ft1_b+ft2_a+ft2_b)/4.0;
               FlameWeight+=ftmid*(wt2-wt1)/2.0*Area*ring->perimpoints[j].hist.TotalWeight;//TempWeight;//AreaBurned;
          }
          wt1=wt2;
          ft1_a=ft2_a;
          l1c=l2;
          CuumArea+=Area;
     }
     *Weight=TotalWeight;
     *Flame=FlameWeight;// /CuumArea;	// actual fraction of net weight
}


void PFIntegration::SecondaryIntegration(FireRing *ring, long j, double CurTempTime, double TempTimeInc, long NumSpatCalcs,
                    									double *Weight, double *Flame)
{
     // Calculates the Weight loss (total and flaming) by integrating the
	// 	Weight-Remaining curve AND Flame Fraction curve - backward in time
     //	over the area and radial distance of the fire quadrangle over the
     // 	given timestep.  The first time this is called for a given quadrangle, the
     //	fire front has just reached the leading edge of the quadrangle.  The time-
     //	and space-steps used for integration may be different
     //
     //	*ring		= the fire ring
     //	j			= the point on the fire ring
     //	CurTempTime	= the current temporal time at the leading edge of the quadrangle
     //	TempTimeInc	= the time difference over which the curves are to be integrated
     //	NumSpatCalcs	= the number of spatial divisions of the curve to use in the integration
     //
     //	returns:	 *Weight (total weight) and *Flame (flame weight).
     //


	long n, p, q, NumFlameSpatCalcs, NumFlameTempCalcs;
     double Fract1, Fract2;
     double ft_temp1, ft_temp2, ft_spat1, ft_spat2, ftmid;
     double FlameWeight, TotalWeight;
     double SpatTimeInc, NextSpatTime;
     double FlameTempTimeInc, FlameSpatTimeInc;
     double FlameStartTime1, FlameStartTime2, FlameSpatTime1, FlameSpatTime2;
     double WeightTime1, WeightTime2;
     double Time1, Time2;


     SpatTimeInc=ring->ElapsedTime/(double) NumSpatCalcs;
     NextSpatTime=CurTempTime; 						// start at most recent time period

     *Weight=*Flame=0.0;
     TotalWeight=FlameWeight=0.0;
     Fract1=0.0;

	ft_temp1=Interpolate(ring, j, NextSpatTime-TempTimeInc, 1);
     if(ft_temp1>0.0)	// flaming still going on
     {    NumFlameSpatCalcs=(long) (ring->ElapsedTime/(ring->perimpoints[j].hist.FlameTime/(double) PRECISION))+1;
          NumFlameTempCalcs=(long) (TempTimeInc/(ring->perimpoints[j].hist.FlameTime/(double) PRECISION))+1;
          if(NumFlameSpatCalcs>20)
          	NumFlameSpatCalcs=20;
          if(NumFlameTempCalcs>20)
          	NumFlameTempCalcs=20;
          FlameSpatTimeInc=ring->ElapsedTime/(double) NumFlameSpatCalcs;
		FlameTempTimeInc=TempTimeInc/(double) NumFlameTempCalcs;

		FlameStartTime1=FlameStartTime2=NextSpatTime-TempTimeInc;	// start at the earliest time
          if(FlameStartTime1<1e-9)
          	FlameStartTime1=FlameStartTime2=0.0;
		for(p=0; p<NumFlameTempCalcs; p++)
          {    ft_temp1=Interpolate(ring, j, FlameStartTime1, 1);     // more recent
              	FlameStartTime2+=FlameTempTimeInc;
              	ft_temp2=Interpolate(ring, j, FlameStartTime2, 1);     // less recent

               FlameSpatTime1=FlameStartTime1;
               FlameSpatTime2=FlameStartTime2;
               Time1=FlameStartTime1;
               Time2=FlameStartTime2;
               Fract1=0.0;
               for(q=0; q<NumFlameSpatCalcs; q++)
               {    FlameSpatTime1+=FlameSpatTimeInc;
                    FlameSpatTime2+=FlameSpatTimeInc;
                  	ft_spat1=Interpolate(ring, j, FlameSpatTime1, 1);
				ft_spat2=Interpolate(ring, j, FlameSpatTime2, 1);
                    ftmid=(ft_temp1+ft_temp2+ft_spat1+ft_spat2)/4.0;	// flaming fraction is an instantaneous observation, so average over the time span

                   	Fract2=((double) (q+1)/(double) NumFlameSpatCalcs);

                    WeightTime1=CalcWeightLoss(j, Fract1, Fract2, Time1, FlameSpatTime1);
                    WeightTime2=CalcWeightLoss(j, Fract1, Fract2, Time2, FlameSpatTime2);

                    FlameWeight+=ftmid*(WeightTime2-WeightTime1);
                    TotalWeight+=(WeightTime2-WeightTime1);

                    Time1=FlameSpatTime1;
                    Time2=FlameSpatTime2;
                    ft_temp1=ft_spat1;
                    ft_temp2=ft_spat2;
                    Fract1=Fract2;
               }
	          FlameStartTime1=FlameStartTime2;
          }
     }
     else
     {	for(n=1; n<=NumSpatCalcs; n++)
     	{	NextSpatTime+=SpatTimeInc;					   // start at present and integrate backward in time
               Fract2=(double) n/(double) NumSpatCalcs;
          	Time1=NextSpatTime-SpatTimeInc;
          	Time2=NextSpatTime;
             	WeightTime2=CalcWeightLoss(j, Fract1, Fract2, Time1, Time2);
              	Time1-=TempTimeInc;
          	Time2-=TempTimeInc;
             	WeightTime1=CalcWeightLoss(j, Fract1, Fract2, Time1, Time2);
              	TotalWeight+=(WeightTime2-WeightTime1);	// temporary for testing
               Fract1=Fract2;
          }
     }
     *Weight=TotalWeight;
     *Flame=FlameWeight;	// actual fraction of net weight
}



double PFIntegration::CalcWeightLoss(long j, double Fract1, double Fract2, double Time1, double Time2)
{
	// calculates the total weight lossed between two time periods
     // j= the point number in the ring
	// n= the iteration count for the spatial calculations
     // Time1 = the most recent time, beginning of integration
     // Time2 = the oldest time, end of integration

     double wt1, wt2, wtmid, TotalWeightLoss;
     double SpatTimeInc;
     double AreaFract, Fract;
	double Area, l1, l2c, l2, h, dldh;

	l1=ring->perimpoints[j].l1;
     l2=l2c=ring->perimpoints[j].l2;
     h=ring->perimpoints[j].h;
     dldh=(l1-l2)/h;
     l1=l2+Fract2*h*dldh;
     l2c=l2+Fract1*h*dldh;
  	Area=0.5*(l1+l2c)*(Fract2-Fract1)*h;

     if(Time1>ring->perimpoints[j].hist.TotalTime)
	{	TotalWeightLoss=Area*(1.0-ring->perimpoints[j].hist.WeightCoefY[ring->perimpoints[j].hist.WeightPolyNum-1])*
               ring->perimpoints[j].hist.TotalWeight;

     	return TotalWeightLoss;
     }

     SpatTimeInc=Time2-Time1;

     wt1=Interpolate(ring, j, Time1, 0);
     if(Time2>ring->perimpoints[j].hist.TotalTime)	// OK, in seconds
     {	AreaFract=(SpatTimeInc-(Time2-ring->perimpoints[j].hist.TotalTime))/SpatTimeInc;
     	wt2=ring->perimpoints[j].hist.WeightCoefY[ring->perimpoints[j].hist.WeightPolyNum-1];
          if(AreaFract>1e-9)
          {   	wtmid=(wt2+wt1)/2.0;				// weight fraction is a cumulative observation of mass remaining
	        	Fract=Fract1+(Fract2-Fract1)*AreaFract;
               l1=l2+Fract*h*dldh;
         		Area=0.5*(l1+l2c)*(Fract-Fract1)*h;
          }
          else
          {    Area=0.0;
              	wtmid=1.0;
               Fract=Fract1;
          }
          TotalWeightLoss=(1.0-wtmid)*Area*ring->perimpoints[j].hist.TotalWeight;

          l1=l2+Fract2*h*dldh;
    		Area=0.5*(l1+l2c)*(Fract2-Fract)*h;
          TotalWeightLoss+=(1.0-wt2)*Area*ring->perimpoints[j].hist.TotalWeight;
      }
      else
      {    wt2=Interpolate(ring, j, Time2, 0);
           wtmid=(wt2+wt1)/2.0;				// weight fraction is a cumulative observation of mass remaining
           TotalWeightLoss=(1.0-wtmid)*Area*ring->perimpoints[j].hist.TotalWeight;    // weight remaining
      }

      return TotalWeightLoss;
}

/*
void PostFrontal::InterpolateOverlaps(PerimPoints *verts, long numverts)
{
// applies a procedure that estimates the partial contribution to burned area
// of each point in an overlapped zone. The zone is described by a polygon *verts,
// with numverts).  The procedure computes the spread distance for each point,
// and the spread distance along that rout of a fictional point originating
// on the opposite side of the zone.  Then it apportions the distance based
// on the relative spread rates of each point.


	long *vertavg=0;
     double *CompDist;
	long i, j, k, m, n, p, q, r, NumAvg, Start, End;
	double x1, x2, y1, y2;
	double x1a, x1b, x2a, x2b;
     double y1a, y1b, y2a, y2b;
     double nx1, nx2, ny1, ny2;
     double dist1, dist2, AvgX, AvgY;

     for(i=0; i<numverts; i++)
     {    if(verts[i].Status>0)       	// only overlapped points
          	continue;
     	x1=verts[i].x1;               // get primary test line
	     x2=verts[i].x2;
	     y1=verts[i].y1;
	     y2=verts[i].y2;
          for(j=0; j<numverts; j++)	// for each segment within the zone
          {	if(j==i)                 // that doesn't contain the start point i
          		continue;
          	k=j+1;
			if(k==i)
               	continue;
     		if(k>numverts-1)
          		k-=numverts;

               x1a=verts[j].x1;
               x2a=verts[j].x2;
               y1a=verts[j].y1;
               y2a=verts[j].y2;

               x1b=verts[k].x1;
               x2b=verts[k].x2;
               y1b=verts[k].y1;
               y2b=verts[k].y2;

               // determine crosses with each segment
               if(Cross(x1, y1, x2, y2, false, x1a, y1a, x1b, y1b, true, &nx1, &ny1))	// must cross timestep1
               {	Cross(x1, y1, x2, y2, false, x2a, y2a, x2b, y2b, false, &nx2, &ny2); 			// take any cross on ts2
                	break;
               }
          }

		// apportion distances based on relative spread rates
          dist1=sqrt(pow2(x1-x2)+pow2(y1-y2));
          dist2=sqrt(pow2(nx1-nx2)+pow2(ny1-ny2));
		verts[i].x2=nx1-(nx1-x1)*(dist1/(dist1+dist2));
		verts[i].y2=ny1-(ny1-y1)*(dist1/(dist1+dist2));
          verts[i].Status=1;
     }
     // average points to least number of times

     vertavg=(long *) GlobalAlloc(GMEM_FIXED, numverts, sizeof(long));
     CompDist=(double *) GlobalAlloc(GMEM_FIXED, numverts, sizeof(double));
     Start=End=1;
     for(i=Start; i<numverts-End; i++)
     {	j=numverts-1-i;
		if(verts[i].Status>1)
          	break;
		if(verts[j].Status>1)
          	break;

          x2a=verts[i].x2;
          y2a=verts[i].y2;
          x2b=verts[j].x2;
          y2b=verts[j].y2;

          AvgX=x2a+x2b;
          AvgY=y2a+y2b;
          NumAvg=2;
          CompDist[NumAvg-2]=pow2(x2a-x2b)+pow2(y2a-y2b);

          //for(k=i+1; k<j-1; k++)
          for(k=1; k<numverts-2; k++)
          {	n=i+k;
          	if(n>numverts-1)
               	break;
	         	p=j-k;
               if(p<1)
               	break;

			for(q=0; q<2; q++)
               {	switch(q)
               	{	case 0:
						r=n;
                    		break;
                    	case 1:
						r=p;
                    		break;
                    }

	          	if(verts[r].Status>1)
     	     	{	//numverts=0;		// force exit from for loop
          	     	break;
	               }
				x2=verts[r].x2;
          	     y2=verts[r].y2;
	          	dist2=pow2(x2a-x2)+pow2(y2a-y2);
	               for(m=0; m<NumAvg-1; m++)
     	          {    if(dist2<=CompDist[r])
	     	          {	AvgX+=x2;
				          AvgY+=y2;
	          	          vertavg[NumAvg-2]=r;
			     	     NumAvg++;
          	               CompDist[NumAvg-2]=dist2;
                              switch(r)
                              {	case 0:	Start=r; break;
                              	case 1:	End=r; break;
                              }
                    	     break;
		               }
     		          else
          		     {    dist2=pow2(x2b-x2)+pow2(y2b-y2);
     	     		     if(dist2<=CompDist[m])
	               		{	AvgX+=x2;
				     	     AvgY+=y2;
			                    vertavg[NumAvg-2]=r;
				     	     NumAvg++;
	     	                    CompDist[NumAvg-2]=dist2;
	                              switch(r)
     	                         {	case 0:	Start=r; break;
          	                    	case 1:	End=r; break;
               	               }
                    	          break;
     			     	}
		               }
     		     }
	          }
          }
          AvgX/=(double) NumAvg;
    	     AvgY/=(double) NumAvg;
		verts[i].x2=AvgX;
		verts[i].y2=AvgY;
		verts[j].x2=AvgX;
		verts[j].y2=AvgY;
    	     for(k=0; k<NumAvg-2; k++)
         	{    verts[vertavg[k]].x2=AvgX;
			verts[vertavg[k]].y2=AvgY;
          }
     }
	if(vertavg)
	     GlobalFree(vertavg);
}
*/


/*
void PostFrontal::InterpolateOverlaps(PerimPoints *verts, long numverts)
{
	long i;
     long NumA, NumB, numchanges;
	double x1a, x1b, x2a, x2b;
     double y1a, y1b, y2a, y2b;
     double dist, MaxDist, Angle, RefAngle, CosDiff;


     NumA=1;
     NumB=0;
     numchanges=1;
     for(i=1; i<numverts; i++)
     {  	if(verts[i].Status==0)
          {	if(NumA>0)
          		NumA++;
          	else
               	NumB++;
          }
          else
     	{	numchanges++;		// =2 if single, else >2
          	if(numchanges==2)
               	NumA*=-1;
          }
     }
     NumA=fabs(NumA);

     // write the mid points of the 1st series of points inside the cross
     x1a=verts[1].x1;
     y1a=verts[1].y1;
     x1b=verts[numverts-2].x1;
     y1b=verts[numverts-2].y1;
     verts[1].x2=verts[numverts-2].x2=x1a-(x1a-x1b)*0.5;
     verts[1].y2=verts[numverts-2].y2=y1a-(y1a-y1b)*0.5;
     verts[1].Status=verts[numverts-2].Status=1;
	if(numchanges>2)
     {	x1a=verts[NumA-1].x1;
          y1a=verts[NumA-1].y1;
          x1b=verts[NumA+2].x1;
          y1b=verts[NumA+2].y1;
          verts[NumA-1].x2=verts[NumA+2].x2=x1a-(x1a-x1b)*0.5;
          verts[NumA-1].y2=verts[NumA+2].y2=y1a-(y1a-y1b)*0.5;
     	verts[NumA-1].Status=verts[NumA+2].Status=1;
     }

     // construct the line down the middle
	x2a=verts[1].x2;
     y2a=verts[1].y2;
     if(numchanges>2)
     {    x2b=verts[NumA-1].x2;
          y2b=verts[NumA-1].y2;
          if(fabs(y2b-y2a)>0.0 || fabs(x2b-x2a)>0.0)
	     	RefAngle=atan2(y2b-y2a, x2b-x2a);
          else
          	RefAngle=-1;
          //MaxDist=pow2(x2a-x2b)+pow2(y2a-y2b);
     }
     else
     {    MaxDist=-1;
     	for(i=2; i<numverts-2; i++)
	     {	x1a=verts[i].x1;
	          y1a=verts[i].y1;
			dist=pow2(x2a-x1a)+pow2(y2a-y1a);
			if(dist>MaxDist)
	          {    MaxDist=dist;
                    x2b=verts[i].x1;
                    y2b=verts[i].y1;
               }
     	}
          RefAngle=atan2(y2a-y2b, x2a-x2b);
     }

     // project all Status=0 points to the center line
	//MaxDist=sqrt(MaxDist);
     for(i=0; i<numverts; i++)
     {	if(verts[i].Status>0)       	// only overlapped points
          	continue;

          x1a=verts[i].x1;
          y1a=verts[i].y1;
          if(fabs(y2a-y1a)>0.0 || fabs(x2a-x1a)>0.0)
	          Angle=atan2(y2a-y1a, x2a-x1a);
          CosDiff=cos(RefAngle)*cos(Angle)+sin(RefAngle)*sin(Angle);
          //dist=sqrt(pow2(x2a-x1a)+pow2(y2a-y1a));
          verts[i].x2=x2a-(x2a-x2b)*((x2a-x1a)*CosDiff)/(x2a-x2b);
          verts[i].y2=y2a-(y2a-y2b)*((y2a-y1a)*CosDiff)/(y2a-y2b);
          verts[i].Status=1;
     }
}


long PostFrontal::SetFireRing(long NumFire, double Start, double End)
{// loads first information into new fire ring structure
	long i, NumPoints;
     double x, y, r, f, c;
     FireRing* firering;

     if(NumRings==0)               	// only at very beginning
     	MergeReferenceRingNum=-1;

     NumPoints=GetNumPoints(NumFire);
	if((firering=AllocFireRing(NumPoints, Start, End))==NULL)
     	return -1;
	firering->StartTime=Start;
	firering->ElapsedTime=End-Start;
     firering->OriginalFireNumber=NumFire;

	for(i=0; i<NumPoints; i++)
	{    GetPerimeter2(i, &x, &y, &r, &f, &c);
     	firering->perimpoints[i].x1=x;
     	firering->perimpoints[i].y1=y;
          firering->perimpoints[i].hist.ReactionIntensity[0]=c;
          if(fabs(r)>0.0 && c>0.0)
	          firering->perimpoints[i].hist.FlameResidenceTime[0]=f/(r/60.0)/c;	// sec/min
     	else
          	firering->perimpoints[i].hist.FlameResidenceTime[0]=0.0;
          x=GetPerimeter1Value(NumFire, i, XCOORD);
          y=GetPerimeter1Value(NumFire, i, YCOORD);
          r=GetPerimeter1Value(NumFire, i, ROSVAL);
          f=GetPerimeter1Value(NumFire, i, FLIVAL);
          c=GetPerimeter1Value(NumFire, i, RCXVAL);
     	firering->perimpoints[i].x2=x;
     	firering->perimpoints[i].y2=y;
          firering->perimpoints[i].hist.ReactionIntensity[1]=c;
          if(fabs(r)>0.0 && c>0.0)
	          firering->perimpoints[i].hist.FlameResidenceTime[1]=f/(r/60.0)/c;
     	else
          	firering->perimpoints[i].hist.FlameResidenceTime[1]=0.0;
          firering->perimpoints[i].SizeMult=1.0;
     }
     ReferenceFireRingNum=NumRings-1;   // store for CorrectFireRing

     if(MergeReferenceRingNum<0)
     	MergeReferenceRingNum=NumRings;

     return NumRings;
}


*/



//------------------------------------------------------------------------------
//
// 	PostFrontal Data Storage stuff
//
//------------------------------------------------------------------------------


PFrontData::PFrontData()
{
//   Initialize Emissions factors, g/kg

     pm25f=67.4-0.95*66.8;
     pm25s=67.4-0.75*66.8;
     pm10f=1.18*pm25f;
     pm10s=1.18*pm25s;
     ch4f=42.7-0.95*43.2;
     ch4s=42.7-0.75*43.2;
     coF=961-0.95*984.0;
     coS=961-0.75*984.0;
     co2f=0.95*1833.0;
     co2s=0.75*1833.0;

     numarrays=0;
     number=0;
     pf1=0;
     pf2=0;
     MaxTime=MaxSmolder=MaxFlaming=MaxTotal=-1.0;
     FractFlameAtMax=0.0;
}


PFrontData::~PFrontData()
{
	if(pf1)
		delete[] pf1;
}


double PFrontData::GetMax(long Species, long Phase)
{
	double MaxVal;

	switch(Phase)
     {	case PF_FLAMING:
     		MaxVal=MaxFlaming*GetFlameMult(Species); break;
     	case PF_SMOLDERING:
     		MaxVal=MaxSmolder*GetSmolderMult(Species); break;
          case PF_TOTAL:
     		MaxVal=MaxTotal*(1.0-FractFlameAtMax)*GetSmolderMult(Species)+
               	  MaxTotal*FractFlameAtMax*GetFlameMult(Species);
                      break;
          default: MaxVal=0.0; break;
     }

     return MaxVal;
}


void PFrontData::SetData(double Time, double Flame, double Smolder)
{
	if(number>=numarrays*50)
     {    PFrontStruct *temp;

     	pf2=new PFrontStruct[(numarrays+1)*50];
          if(pf1!=NULL)
	     	CopyMemory(pf2, pf1, numarrays*50*sizeof(PFrontStruct));
	     temp=pf1;
     	pf1=pf2;
          pf2=temp;
          if(pf2!=NULL)
	          delete[] pf2;
    	     pf2=0;
          numarrays++;
     }
     pf1[number].Time=Time;
     pf1[number].Flaming=Flame;
     pf1[number].Smoldering=Smolder;

	if(MaxTime<Time)
     	MaxTime=Time;
     if(MaxSmolder<Smolder)
     	MaxSmolder=Smolder;
     if(MaxFlaming<Flame)
     	MaxFlaming=Flame;
     if(MaxTotal<Flame+Smolder)
     {	MaxTotal=Flame+Smolder;
          if(MaxTotal>0.0)
           	FractFlameAtMax=Flame/MaxTotal;
          else
               FractFlameAtMax=1.0;
     }

     number++;
}



void PFrontData::ResetData()
{
	if(pf1)
     	delete[] pf1;
     pf1=0;
     number=0;
     numarrays=0;
     MaxTime=MaxSmolder=MaxFlaming=MaxTotal=-1.0;
     FractFlameAtMax=0.0;
}


double PFrontData::GetFlameMult(long Species)
{	// returns Mg or GJ
	double mult;

	switch(Species)
     {	case PF_FUELWEIGHT:
     		mult=0.001;  //
               break;
		case PF_TOTALHEAT:
     		mult=0.0186; // GJ
               break;
		case PF_PM25:
     		mult=pm25f/1e6;
               break;
		case PF_PM10:
     		mult=pm10f/1e6;
               break;
		case PF_CH4:
     		mult=ch4f/1e6;
               break;
		case PF_CO:
     		mult=coF/1e6;
               break;
		case PF_CO2:
     		mult=co2f/1e6;
               break;
          default:
          	mult=1.0/1e6;
     }

     return mult;
}


double PFrontData::GetSmolderMult(long Species)
{	// returns Mg or GJ
	double mult;

	switch(Species)
     {	case PF_FUELWEIGHT:
     		mult=0.001;
               break;
		case PF_TOTALHEAT:
     		mult=0.0186;  // GJ
               break;
		case PF_PM25:
     		mult=pm25s/1e6;
               break;
		case PF_PM10:
     		mult=pm10s/1e6;
               break;
		case PF_CH4:
     		mult=ch4s/1e6;
               break;
		case PF_CO:
     		mult=coS/1e6;
               break;
		case PF_CO2:
     		mult=co2s/1e6;
               break;
          default:
          	mult=1.0/1e6;
     }

     return mult;
}


double PFrontData::GetPostFrontalProducts(long Species, long Phase, long Num)
{
	if(Num>=number)
     	return -1.0;
     if(pf1==NULL)
     	return 0.0;

     double val=0.0;

     if(Phase==PF_FLAMING || Phase==PF_TOTAL)
     	val+=pf1[Num].Flaming*GetFlameMult(Species);
     if(Phase==PF_SMOLDERING || Phase==PF_TOTAL)
     	val+=pf1[Num].Smoldering*GetSmolderMult(Species);

     return val;
}


double PFrontData::GetTime(long Num)
{
	if(Num>=number)
     	return -1.0;

     return pf1[Num].Time;
}






