//------------------------------------------------------------------------------
//
//   crossthread.cpp
//   FARSITE 4.0
//   10/20/2000
//   Mark A. Finney (USDA For. Serv., Fire Sciences Laboratory)
//
//   Contains functions for threading the comparisons of crossing segements of
//        a fire perimeter (declarations in fsx4.hpp)
//
//------------------------------------------------------------------------------

#include "flm4.hpp"
#include "flmglbvar.h"
#include "fsxsync.h"


CrossThread::CrossThread()
{
     NumCross=0;
     NumAlloc=0;
     intersect=0;
     interpoint=0;
     hXEvent=0;
     ThreadOrder=-1;
     CurrentFire=NextFire=0;
}


CrossThread::~CrossThread()
{
     FreeCross();

}

bool CrossThread::AllocCross(long Num)
{
     if(Num<NumAlloc)
          return true;

     FreeCross();
     if((intersect=new long[Num*2])==NULL)
          return false;
     if((interpoint=new double[Num*NUMDATA])==NULL)
     {    FreeCross();

          return false;
     }
     NumAlloc=Num;
     NumCross=0;

     return true;
}


bool CrossThread::ReallocCross(long Num)
{
     long OldNum, OldNumCross;
     long *tempisect;
     double *tempipt;

     if(intersect)
     {    tempisect=new long[NumAlloc*2];
          if(tempisect==NULL)
               return false;
          CopyMemory(tempisect, intersect, NumAlloc*2*sizeof(long));
     }
     if(interpoint)
     {    tempipt=new double[NumAlloc*NUMDATA];
          if(tempipt==NULL)
          {    delete[] tempisect;

               return false;
          }
          CopyMemory(tempipt, interpoint, NumAlloc*NUMDATA*sizeof(double));
     }
     OldNum=NumAlloc;
     OldNumCross=NumCross;
     FreeCross();
     AllocCross(Num);
     CopyMemory(intersect, tempisect, OldNum*2*sizeof(long));
     CopyMemory(interpoint, tempipt, OldNum*NUMDATA*sizeof(double));
     NumCross=OldNumCross;
     delete[] tempisect;
     delete[] tempipt;

     return true;
}


void CrossThread::FreeCross()
{
     if(intersect)
          delete[] intersect;
     if(interpoint)
          delete[] interpoint;
     intersect=0;
     interpoint=0;
     NumAlloc=0;
     NumCross=0;
}


unsigned __stdcall CrossThread::RunCrossThread(void *crossthread)
{
	static_cast <CrossThread *> (crossthread)->CrossCompare();

     return 1;
}

void CrossThread::StartCrossThread(long threadorder, long currentfire, long nextfire)
{
	HANDLE hXThread;

     CurrentFire=currentfire;
     NextFire=nextfire;
     if(ThreadOrder<0)
     {	ThreadOrder=threadorder;
          hXEvent=GetFarsiteEvent(EVENT_CROSS_THREAD, ThreadOrder);
          hXThread=(HANDLE) ::_beginthreadex(NULL, 0, &CrossThread::RunCrossThread, this, NULL, &ThreadID);
          CloseHandle(hXThread);
     }
     else
     	SetEvent(hXEvent);
}


void CrossThread::SetRange(long spanastart, long spanaend)
{
     SpanAStart=spanastart;
     SpanAEnd=spanaend;
     SpanBStart=0;
}


long CrossThread::GetNumCross()
{
     return NumCross;
}


long *CrossThread::GetIsect()
{
     return intersect;
}


double* CrossThread::GetIpoint()
{
     return interpoint;
}


void CrossThread::SetIntersection(long Number, long XOrder, long YOrder)
{
     if(Number>=NumAlloc)
          ReallocCross(Number*2);

	Number*=2;
	intersect[Number]=XOrder;
	intersect[++Number]=YOrder;
}


void CrossThread::SetInterPoint(long Number, double XCoord, double YCoord, double Ros, double Fli, double Rct)
{
     if(Number>=NumAlloc)
          ReallocCross(Number*2);

	Number*=NUMDATA;
	if(XCoord==0 || YCoord==0)
		XCoord=YCoord=1;
	interpoint[Number]=XCoord;
	interpoint[++Number]=YCoord;
	interpoint[++Number]=Ros;
	interpoint[++Number]=Fli;
	interpoint[++Number]=Rct;
}


void CrossThread::CrossCompare()
{// determines which points crossed into alread burned areas
	bool crossyn, ptcross1, ptcross2, SinglePtCross;
	long i, j, k, m, n, dup1, dup2;
     long NewFires1, NewFires2, *PostFires;  	      // for post frontal stuff
	double xpt, ypt, xptl, yptl, xptn, yptn, newx, newy;
     double crxpt, crypt, crxptl, cryptl, crxptn, cryptn;
	double ros1, ros2, ros3, fli1, fli2, fli3, fli4;
     double rcx1, rcx2, rcx3, xg, yg;
     long CrossMemAlloc=GetNumPoints(CurrentFire)+GetNumPoints(NextFire);

     do
     {    NumCross=0;
     	SpanBEnd=GetNumPoints(NextFire);
	     AllocCross(CrossMemAlloc);      // allocate array of intersection orders
	     for(i=SpanAStart; i<SpanAEnd; i++)   // for each point in a given fire
     	{	xpt=GetPerimeter1Value(CurrentFire, i, XCOORD);
	     	ypt=GetPerimeter1Value(CurrentFire, i, YCOORD);
		     if(i<GetNumPoints(CurrentFire)-1)
     			k=i+1;
     		else
	     		k=0;
     		xptn=GetPerimeter1Value(CurrentFire, k, XCOORD);
	     	yptn=GetPerimeter1Value(CurrentFire, k, YCOORD);
		     for(j=SpanBStart; j<SpanBEnd; j++)	// FOR ALL FIRES CHECK FOR ALL CROSSES FROM THE FIRST ELEMENT
     		{	crxpt=GetPerimeter1Value(NextFire, j, XCOORD);
	     		crypt=GetPerimeter1Value(NextFire, j, YCOORD);
		     	if(j!=GetNumPoints(NextFire)-1)
			     	m=j+1;
     			else
	     			m=0;
		     	if(CurrentFire==NextFire)	// don't check for crosses on same or adjacent points within same array
			     {	if(i==j || i==m || k==j || k==m)
				     	continue;
     			}
	     		crxptn=GetPerimeter1Value(NextFire, m, XCOORD);
		     	cryptn=GetPerimeter1Value(NextFire, m, YCOORD);

                    dup1=dup2=1;   // flag for forcing check of crosspt only within bounds of each span
                    crossyn=Cross(xpt, ypt, xptn, yptn, crxpt, crypt, crxptn, cryptn, &newx, &newy, &dup1, &dup2);

                    ptcross1=ptcross2=false;
                    SinglePtCross=false;

                    if(dup1<0 && dup2<0)
                    {    if(CurrentFire==NextFire)
                              ptcross1=ptcross2=false;
                         else
                              ptcross1=true; // ignore cross if only at one point
                         SinglePtCross=true;
                    }
                    else if(dup1<0)          // if cross duplicates xpt, ypt
                    {    n=i-1;
                         if(n<0)
                              n=GetNumPoints(CurrentFire)-1;
                         xptl=GetPerimeter1Value(CurrentFire, n, XCOORD);
          	          yptl=GetPerimeter1Value(CurrentFire, n, YCOORD);
                         dup1=1;
                         dup2=0;
                         if(!Cross(xptl, yptl, xptn, yptn, crxpt, crypt, crxptn, cryptn, &xg, &yg, &dup1, &dup2))
                              ptcross1=true;
                         else if(dup1<0)
                              ptcross1=true;
                         else
                              SinglePtCross=true;
                    }
                    else if(dup2<0)     // if cross duplicates crxpt, crypt
                    {    n=j-1;
                         if(n<0)
                              n=GetNumPoints(NextFire)-1;
                         crxptl=GetPerimeter1Value(NextFire, n, XCOORD);
          	     	cryptl=GetPerimeter1Value(NextFire, n, YCOORD);
                         dup1=0;
                         dup2=1;
                         if(!Cross(xpt, ypt, xptn, yptn, crxptl, cryptl, crxptn, cryptn, &xg, &yg, &dup1, &dup2))
                              ptcross2=true;  // means this is just a single pt cross
                         else if(dup1<0)
                              ptcross2=true;
                         else
                              SinglePtCross=true;
                    }

     			if(crossyn==true)
	     		{    if(ptcross1==true || ptcross2==true)  // just a single pt cross
                              continue;
                         if(SinglePtCross)
                         {    if(CurrentFire==NextFire)
                                   SetIntersection(NumCross, -i, -j);	// point of order i in first fire, j on second fire
                              else
                                   SetIntersection(NumCross, i, j);	// point of order i in first fire, j on second fire
                         }
                         else
                              SetIntersection(NumCross, i, j);	// point of order i in first fire, j on second fire
		     		ros1=GetPerimeter1Value(CurrentFire, i, ROSVAL);
     				fli1=GetPerimeter1Value(CurrentFire, i, FLIVAL);
	     			rcx1=GetPerimeter1Value(CurrentFire, i, RCXVAL);
		     		ros2=GetPerimeter1Value(CurrentFire, k, ROSVAL);
			     	fli2=GetPerimeter1Value(CurrentFire, k, FLIVAL);
     			     rcx2=GetPerimeter1Value(CurrentFire, k, RCXVAL);
	     			ros1=(ros1+ros2)/2.0;				// average rate of spread for perimeter segment
		     		fli3=(fabs(fli1)+fabs(fli2))/2.0;				// average fireline intensity for perimeter segment
     				if(fli1<0.0 || fli2<0.0)
	     				fli3*=-1.0;
		     		fli1=fli3;
			     	rcx1=(rcx1+rcx2)/2.0;				// average rate of spread for perimeter segment
                         ros2=GetPerimeter1Value(NextFire, j, ROSVAL);
	     			fli2=GetPerimeter1Value(NextFire, j, FLIVAL);
		     		rcx2=GetPerimeter1Value(NextFire, j, RCXVAL);
			     	ros3=GetPerimeter1Value(NextFire, m, ROSVAL);
				     fli3=GetPerimeter1Value(NextFire, m, FLIVAL);
     				rcx3=GetPerimeter1Value(NextFire, m, RCXVAL);
	     			ros2=(ros2+ros3)/2.0;				// average rate of spread for perimeter segment
		     		fli4=(fabs(fli2)+fabs(fli3))/2.0;				// average rate of spread for perimeter segment
			     	if(fli2<0.0 || fli3<0.0)
				     	fli4*=-1.0;
     				fli2=fli4;
	     			rcx2=(rcx2+rcx3)/2.0;				// average rate of spread for perimeter segment
		     		ros3=(ros1+ros2)/2.0;			  	// ROS for intersection point
     				fli4=(fabs(fli1)+fabs(fli2))/2.0;			  		// FLI for intersection point
			     	if(fli1<0.0 && fli2<0.0)
				     	fli4*=-1.0;
     				fli3=fli4;
	     			rcx3=(rcx1+rcx2)/2.0;			  		// ROS for intersection point
		     		SetInterPoint(NumCross, newx, newy, ros3, fli3, rcx3);	 //write intersected points
                         if(GetInout(CurrentFire)==2 && GetInout(NextFire)==1)
                         {	if(fli2<0.0)        // check for stationary points on outward fire
                        	     {	NumCross=0;	// if found dont merge with inward burning fire

                                                  //CrossError=true;
                              }
                         }
				     NumCross++;	          // allow outward fire to get only outward fire perimeter
			     }
		     	crossyn=0;
	     	}
     	}
          SetFarsiteEvent(EVENT_CROSS, ThreadOrder);
          WaitForSingleObject(hXEvent, INFINITE);
          ResetEvent(hXEvent);
     }while(SpanAEnd>-1);
     SetFarsiteEvent(EVENT_CROSS, ThreadOrder);
}


bool CrossThread::Cross(double xpt1, double ypt1, double xpt2, double ypt2,
				  double xpt1n, double ypt1n, double xpt2n, double ypt2n,
				  double *newx, double *newy, long *dup1, long *dup2)
{
	double xdiff1, ydiff1, xdiff2, ydiff2, ycept1, ycept2;
	double slope1, slope2, ycommon, xcommon;
	bool intersection=false, OnSpan1, OnSpan2;

     if(*dup1!=0)       // success only if crosspt is within Span1
          OnSpan1=true;
     else
          OnSpan1=false;
     if(*dup2!=0)       // success only if crosspt is within Span2
          OnSpan2=true;
     else
          OnSpan2=false;

	xdiff1=xpt2-xpt1;
	ydiff1=ypt2-ypt1;
     if(fabs(xdiff1)<1e-9)
          xdiff1=0.0;
	if(xdiff1!=0.0)
	{	slope1=ydiff1/xdiff1;
		ycept1=ypt1-(slope1*xpt1);
	}
	else
	{	slope1=1.0;
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
	{	slope2=1.0;					     // SLOPE NON-ZERO
          ycept2=xpt1n;
     }
     *dup1=*dup2=0;
     if(fabs(slope1-slope2)<1e-9)
     {    if(fabs(ycept1-ycept2)<1e-9)
          {    if(xdiff1==0.0 && xdiff2==0.0)
               {    if(OnSpan1 && OnSpan2)
                    {    if((ypt1<=ypt1n && ypt1>ypt2n) || (ypt1>=ypt1n && ypt1<ypt2n))
                         {    *dup1=-1;
                              intersection=true;
                              *newx=xpt1;
                              *newy=ypt1;
                         }
                         if((ypt1n<=ypt1 && ypt1n>ypt2) || (ypt1n>=ypt1 && ypt1n<ypt2))
                         {    *dup2=-1;
                              intersection=true;
                              *newx=xpt1n;
                              *newy=ypt1n;
                         }
                    }
                    else if(OnSpan1)
                    {    if((ypt1n<=ypt1 && ypt1n>ypt2) || (ypt1n>=ypt1 && ypt1n<ypt2))
                         {    *dup2=-1;
                              intersection=true;
                              *newx=xpt1n;
                              *newy=ypt1n;
                         }
                    }
                    else if(OnSpan2)
                    {    if((ypt1<=ypt1n && ypt1>ypt2n) || (ypt1>=ypt1n && ypt1<ypt2n))
                         {    *dup1=-1;
                              intersection=true;
                              *newx=xpt1;
                              *newy=ypt1;
                         }
                    }
               }
               else
               {    if(OnSpan1 && OnSpan2)
                    {    if((xpt1<=xpt1n && xpt1>xpt2n) || (xpt1>=xpt1n && xpt1<xpt2n))
                         {    *dup1=-1;
                              intersection=true;
                              *newx=xpt1;
                              *newy=ypt1;
                         }
                         if((xpt1n<=xpt1 && xpt1n>xpt2) || (xpt1n>=xpt1 && xpt1n<xpt2))
                         {    *dup2=-1;
                              intersection=true;
                              *newx=xpt1n;
                              *newy=ypt1n;
                         }
                    }
                    else if(OnSpan1)
                    {    if((xpt1n<=xpt1 && xpt1n>xpt2) || (xpt1n>=xpt1 && xpt1n<xpt2))
                         {    *dup2=-1;
                              intersection=true;
                              *newx=xpt1n;
                              *newy=ypt1n;
                         }
                    }
                    else if(OnSpan2)
                    {    if((xpt1<=xpt1n && xpt1>xpt2n) || (xpt1>=xpt1n && xpt1<xpt2n))
                         {    *dup1=-1;
                              intersection=true;
                              *newx=xpt1;
                              *newy=ypt1;
                         }
                    }
               }
          }
     }
     else
     {    if(xdiff1!=0.0 && xdiff2!=0.0)
     	{	xcommon=(ycept2-ycept1)/(slope1-slope2);
	     	*newx=xcommon;
		     ycommon=ycept1+slope1*xcommon;
     		*newy=ycommon;
	     	if(OnSpan1 && OnSpan2)
               {    if((*newx>=xpt1 && *newx<xpt2) || (*newx<=xpt1 && *newx>xpt2))
		          {    *dup1=1;
                         if((*newx>=xpt1n && *newx<xpt2n) || (*newx<=xpt1n && *newx>xpt2n))
     			     {    *dup2=1;
                        	     intersection=true;
                         }
                    }
     		}
               else if(OnSpan1)
               {    if((*newx>=xpt1 && *newx<xpt2) || (*newx<=xpt1 && *newx>xpt2))
		          {    *dup1=1;
                         intersection=true;
                    }
               }
               else if(OnSpan2)
               {    if((*newx>=xpt1n && *newx<xpt2n) || (*newx<=xpt1n && *newx>xpt2n))
                    {    *dup2=1;
                  	     intersection=true;
                    }
               }
          }
     	else	if(xdiff1==0.0 && xdiff2!=0.0)
          {	ycommon=slope2*xpt1+ycept2;
     		*newx=xpt1;
	     	*newy=ycommon;
		     if(OnSpan1 && OnSpan2)
               {    if((*newy>=ypt1 && *newy<ypt2) || (*newy<=ypt1 && *newy>ypt2))
     		     {    *dup1=1;
                         if((*newx>=xpt1n && *newx<xpt2n) || (*newx<=xpt1n && *newx>xpt2n))
          		     {    intersection=true;
                              *dup2=1;
                         }
                    }
     		}
               else if(OnSpan1)
               {    if((*newy>=ypt1 && *newy<ypt2) || (*newy<=ypt1 && *newy>ypt2))
     		     {    *dup1=1;
                         intersection=true;
                    }
               }
               else if(OnSpan2)
               {    if((*newx>=xpt1n && *newx<xpt2n) || (*newx<=xpt1n && *newx>xpt2n))
                    {    intersection=true;
                         *dup2=1;
                    }
               }
	     }
     	else	if(xdiff2==0.0 && xdiff1!=0.0)
          {	ycommon=slope1*xpt1n+ycept1;
		     *newx=xpt1n;
     		*newy=ycommon;
               if(OnSpan1 && OnSpan2)
               {   	if((*newy>=ypt1n && *newy<ypt2n) || (*newy<=ypt1n && *newy>ypt2n))
		    	     {    *dup2=1;
                         if((*newx>=xpt1 && *newx<xpt2) || (*newx<=xpt1 && *newx>xpt2))
		 	          {	intersection=true;
                              *dup1=1;
                         }
                    }
               }
               else if(OnSpan1)
               {    if((*newx>=xpt1 && *newx<xpt2) || (*newx<=xpt1 && *newx>xpt2))
		 	     {	intersection=true;
                         *dup1=1;
                    }
               }
               else if(OnSpan2)
               {   	if((*newy>=ypt1n && *newy<ypt2n) || (*newy<=ypt1n && *newy>ypt2n))
		    	     {    *dup2=1;
                         intersection=true;
                    }
               }
     	}
          if(intersection)
          {    if(sqrt(pow2(*newx-xpt1)+pow2(*newy-ypt1))<1e-9)
                   *dup1=-1;
               if(sqrt(pow2(*newx-xpt1n)+pow2(*newy-ypt1n))<1e-9)
                   *dup2=-1;
          }
     }

	return intersection;
}

