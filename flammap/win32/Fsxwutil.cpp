//******************************************************************************
//	FSXWUTIL.CPP	Utility functions for FARSITE
//                  Clip loops, Merge Fires, Rediscretize etc.
//
//				Copyright 1994, 1995, 1996
//				Mark A. Finney, Systems for Environemntal Management
//******************************************************************************

#include "flmglbvar.h"
//#include "fsxwattk.h"
#include "fsxpfront.h"
//#include "fsairatk.h"
#include "fsxsync.h"
#include "vec.h"
#include "rand2.h"

const double PI=acos(-1.0);
//extern const double PI;

AreaPerimeter::AreaPerimeter()
{	area=0.0;
	perimeter=0.0;
	sperimeter=0.0;
	cuumslope[0]=0.0;
	cuumslope[1]=0.0;
}


AreaPerimeter::~AreaPerimeter()
{

}


void AreaPerimeter::arp(long CurrentFire)
{// calculates area and perimeter as a planimeter (with triangles)
	long i, j, numx;
	double xpt1, ypt1, xpt2, ypt2, aangle, zangle;
	double ediff, e1, e2, newarea;
	double hdist, gdist, xdiff, ydiff, DiffAngle;

	area=0.0;
	perimeter=0.0;
	sperimeter=0.0; 	  // private AreaPerimeter-class members
     numx=GetNumPoints(CurrentFire);
	if(numx>0)
	{    startx=GetPerimeter2Value(0, 0);     // must use old perim array
		starty=GetPerimeter2Value(0, 1);     // new array not merged or clipped yet
		e1=GetElev(0);
		i=0;
		while(i<numx)
		{    i++;
			xpt1=GetPerimeter2Value(i, 0);
			ypt1=GetPerimeter2Value(i, 1);
			zangle=direction(xpt1, ypt1);        // reference angle
			if(zangle!=999.9)	// make sure that startx,starty!=x[0]y[0]
				break;
		}
		e2=GetElev(i);
		xdiff=xpt1-startx;
		ydiff=ypt1-starty;
		ediff=fabs(e1-e2);
		hdist=sqrt(pow2(xdiff)+pow2(ydiff));
		gdist=sqrt(pow2(ediff)+pow2(hdist));
		perimeter=hdist;
		sperimeter=gdist;
		i++;
		for(j=i; j<numx; j++)
		{	xpt2=GetPerimeter2Value(j, 0);
			ypt2=GetPerimeter2Value(j, 1);
			e2=GetElev(j);
			xdiff=xpt2-xpt1;
			ydiff=ypt2-ypt1;
			ediff= fabs(e1-e2);
			hdist=sqrt(pow2(xdiff)+pow2(ydiff));
			gdist=sqrt(pow2(ediff)+pow2(hdist));
			perimeter+=hdist;
			sperimeter+=gdist;
			newarea=.5*(startx*ypt1-xpt1*starty+xpt1*ypt2-xpt2*ypt1+xpt2*starty-startx*ypt2);
			newarea=fabs(newarea);
			aangle=direction(xpt2, ypt2);
			if(aangle!=999.9)
			{	DiffAngle=aangle-zangle;
				if(DiffAngle>PI)
					DiffAngle=-(2.0*PI-DiffAngle);
				else if(DiffAngle<-PI)
					DiffAngle=(2.0*PI+DiffAngle);
				if(DiffAngle>0.0)
					area-=newarea;
				else if(DiffAngle<0.0)
					area+=newarea;
				zangle=aangle;
			}
			xpt1=xpt2;
			ypt1=ypt2;
			e1=e2;
		}
		xdiff=startx-xpt1;
		ydiff=starty-ypt1;
		e2=GetElev(0);
		ediff= fabs(e1-e2);
		hdist=sqrt(pow2(xdiff)+pow2(ydiff));
		gdist=sqrt(pow2(ediff)+pow2(hdist));
		perimeter+=hdist;
		sperimeter+=gdist;
		area/=(10000.0*pow2(MetricResolutionConvert()));		// ha always
		perimeter/=(1000.0*MetricResolutionConvert());       		// km always
		sperimeter/=(1000.0*MetricResolutionConvert());			// km always
	}
}


double APolygon::direction(double xpt1, double ypt1)
{// calculates sweep direction for angle determination
	double zangle=999.9, xdiff, ydiff;

	xdiff=xpt1-startx;
	ydiff=ypt1-starty;
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



long APolygon::Overlap(long CurrentFire)
{// determines if point is inside or outside a fire polygon (CurrentFire)
	long NumVertex=GetNumPoints(CurrentFire);
	long count=0, count1, count2, inside=0;
	double Aangle=0.0, Bangle;
	double CuumAngle=0.0, DiffAngle;
	double Sxpt, Sypt;

	while(count<NumVertex)    // make sure that startx,starty!=x[0]y[0]
	{	Sxpt=GetPerimeter1Value(CurrentFire, count, XCOORD);
		Sypt=GetPerimeter1Value(CurrentFire, count, YCOORD);
		Aangle=direction(Sxpt, Sypt);
		count++;
		if(Aangle!=999.9)
			break;
	}
	for(count1=count; count1<=NumVertex; count1++)
	{    if(count1==NumVertex)
			count2=count-1;
		else
			count2=count1;
		Sxpt=GetPerimeter1Value(CurrentFire, count2, XCOORD);
		Sypt=GetPerimeter1Value(CurrentFire, count2, YCOORD);
		Bangle=direction(Sxpt, Sypt);
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


void CompareRect::DetermineHiLo(double xpt, double ypt)
{// find bounding box for fire polygon
	if(xpt<Xlo)
		Xlo=xpt;
	else if(xpt>Xhi)
		Xhi=xpt;
	if(ypt<Ylo)
		Ylo=ypt;
	else	if(ypt>Yhi)
		Yhi=ypt;
}


void CompareRect::WriteHiLo(long FireNum)
{// write bounding box for fire polygon
	if(GetNumPoints(FireNum))
	{	SetPerimeter1(FireNum, GetNumPoints(FireNum), Xlo, Xhi);
		SetFireChx(FireNum, GetNumPoints(FireNum), Ylo, Yhi);       // not really firechx, but write to same positions
	}
}


void CompareRect::InitRect(long NumFire)
{// get bounding rectangle for fire polygon
	long NumPoints=GetNumPoints(NumFire);

     // load hi for lo and vice-versa to initiate for comparison
	Xhi=GetPerimeter1Value(NumFire, NumPoints, XCOORD);
	Xlo=GetPerimeter1Value(NumFire, NumPoints, YCOORD);
	Yhi=GetPerimeter1Value(NumFire, NumPoints, ROSVAL);
	Ylo=GetPerimeter1Value(NumFire, NumPoints, FLIVAL);
}


bool CompareRect::BoundCross(long Fire1, long Fire2)
{// find out if bounding rectangles on 2 fires cross each other
	Xlo=GetPerimeter1Value(Fire1, GetNumPoints(Fire1), XCOORD);
	Xhi=GetPerimeter1Value(Fire1, GetNumPoints(Fire1), YCOORD);
	Ylo=GetPerimeter1Value(Fire1, GetNumPoints(Fire1), ROSVAL);
	Yhi=GetPerimeter1Value(Fire1, GetNumPoints(Fire1), FLIVAL);
	xlo=GetPerimeter1Value(Fire2, GetNumPoints(Fire2), XCOORD);
	xhi=GetPerimeter1Value(Fire2, GetNumPoints(Fire2), YCOORD);
	ylo=GetPerimeter1Value(Fire2, GetNumPoints(Fire2), ROSVAL);
	yhi=GetPerimeter1Value(Fire2, GetNumPoints(Fire2), FLIVAL);
//	XLO=0.0; XHI=0.0; YLO=0.0; YHI=0.0;
	if(!XOverlap())
		return false;
	else if(!YOverlap())
		return false;

	return true;
}


bool CompareRect::MergeInAndOutOK1(long Fire1, long Fire2)
{// check to see if bounding rectangles allow mergers between inward and outward burning fires
     double XDiff1, XDiff2;
     double YDiff1, YDiff2;

     XDiff1=Xhi-Xlo;
     XDiff2=xhi-xlo;
     YDiff1=Yhi-Ylo;
     YDiff2=yhi-ylo;

     if(GetInout(Fire2)==2)
     {    if(XDiff1>=XDiff2)
          {    //return false;
               if(XDiff1<=YDiff1 || YDiff1>=YDiff2)    // if overlap dimension is smallest ||
                    return false;                      // both dimensions overlap
          }
          else if(YDiff1>=YDiff2)
          {    //return false;
               if(YDiff1<=XDiff1 || XDiff1>=XDiff2)
                    return false;
          }
     }
     else if(GetInout(Fire1)==2)
     {    if(XDiff2>=XDiff1)
          {    //return false;
               if(XDiff2<=YDiff2 || YDiff2>=YDiff1)
                    return false;
          }
          else if(YDiff2>=YDiff1)
          {    //return false;
               if(YDiff2<=XDiff2 || XDiff2>=XDiff1)
                    return false;
          }
     }

     return true;
}


bool Intersections::MergeInAndOutOK2(long Fire1, long Fire2)
{// check to see if outward burning fire is inside another outward fire
     bool MergeOK=false;
	long i, j, NumFires, NumPoints;
     long TargetFire=-1;

     if(GetInout(Fire1)==3 || GetInout(Fire2)==3)
          return true;

     if(GetInout(Fire1)==1)
     	TargetFire=Fire1;
     if(GetInout(Fire2)==1)
     {	if(TargetFire==Fire1)
			return true;		// means that both are outward burning
		TargetFire=Fire2;
     }

     NumFires=GetNumFires();
     NumPoints=GetNumPoints(TargetFire);
     for(i=0; i<NumFires; i++)
     {	if(TargetFire==i)
     		continue;
     	else if(GetInout(i)!=1)
          	continue;
          for(j=0; j<NumPoints; j++)
          {	startx=GetPerimeter1Value(TargetFire, j, XCOORD);
          	starty=GetPerimeter1Value(TargetFire, j, YCOORD);
			if(Overlap(i))
               {	MergeOK=true;
               	break;
               }
          }
          if(MergeOK)
          	break;
     }

     return MergeOK;
}


bool CompareRect::XOverlap()
{// computations for determining overlapping bounding rectangles, X-Dimension
	bool RectOverlap=0;
	if(Xlo>=xlo && Xlo<=xhi)
	{	XLO=Xlo;
		RectOverlap=1;
	}
	else if(xlo>=Xlo && xlo<=Xhi)
	{	XLO=xlo;
		RectOverlap=1;
	}
	if(Xhi<=xhi && Xhi>=xlo)
	{	XHI=Xhi;
		RectOverlap=1;
	}
	else	if(xhi<=Xhi && xhi>=Xlo)
	{	XHI=xhi;
		RectOverlap=1;
	}

	return RectOverlap;
}


bool CompareRect::YOverlap()
{// computations for determining overlapping bounding rectangles, Y-Dimension
	bool RectOverlap=0;
	if(Ylo>=ylo && Ylo<=yhi)
	{	YLO=Ylo;
		RectOverlap=1;
	}
	else if(ylo>=Ylo && ylo<=Yhi)
	{	YLO=ylo;
		RectOverlap=1;
	}
	if(Yhi<=yhi && Yhi>=ylo)
	{	YHI=Yhi;
		RectOverlap=1;
	}
	else	if(yhi<=Yhi && yhi>=Ylo)
	{	YHI=yhi;
		RectOverlap=1;
	}

	return RectOverlap;
}


//static long idum;   // this is for the random numbers in Intersections::EliminateDuplicatePoints

Intersections::Intersections()
{
	crossout=0;
	intersect=0;
	interpoint=0;
     crossnumalloc=0;
     intersectnumalloc=0;
     interpointnumalloc=0;
     newisectnumalloc=0;
     NewIsect=0;
     AltIsect=0;
     NumCrossThreads=0;
     crossthread=0;
/*     SYSTEMTIME tb;
     GetSystemTime(&tb);
     srand(tb.wMilliseconds);
     idum=-((rand()%20)+1);				// initialize random num gen
*/
}


Intersections::~Intersections()
{
	FreeCrossout();
     FreeInterPoint();
     FreeIntersection();
     FreeNewIntersection();
     CloseCrossThreads();
}


void Intersections::ResetIntersectionArrays()
{
     FreeSwap();                 	//XUtilities 	 member
	FreeCrossout();               //Intersections member
     FreeInterPoint();
     FreeIntersection();
     FreeNewIntersection();
}


void Intersections::FindMergeSpans(long FireNum)
{// find vertices on fire polygons that are in overlapping rectangle
 // IF THIS FUNCTION IS USED, REPLACE USE OF XLO,XHI,YLO,YHI IN "FindExternalPt()"!!!!!!!

	double xpt, ypt;
	short First=0, Second=0;
	SpanAStart=0;
	SpanAEnd=GetNumPoints(FireNum);

	for(long i=0; i<GetNumPoints(FireNum); i++)
	{	xpt=GetPerimeter1Value(FireNum, i, XCOORD);
		ypt=GetPerimeter1Value(FireNum, i, YCOORD);
		if(xpt<=XHI && xpt>=XLO)
		{	if(ypt<=YHI && ypt>=YLO)
			{	if(!First)
				{    if(i>0)                // don't start at beginning
					{	SpanAStart=i-1;   // start span at point before inside point
						First=1;          // first has started
						Second=1;         // search for second
					}
					else      // if origin of fire inside box
					{	//SpanAStart=0;     // search whole fire
						//SpanAEnd=GetNumPoints(FireNum);
						break;
					}
				}
			}
			else	if(Second)
			{	SpanAEnd=i;
				//break;
			}
		}
		else	if(Second)
		{	SpanAEnd=i;
			//break;
		}
	}
//	if(i>GetNumPoints(FireNum))  // if hit end of fire array and still inside
//		SpanAEnd=GetNumPoints(FireNum);
}


void CompareRect::ExchangeRect(long FireNum)
{// determine combined bounding rectangle for merged fires
	if(xlo<Xlo)
		Xlo=xlo;
	if(xhi>Xhi)
		Xhi=xhi;
	if(ylo<Ylo)
		Ylo=ylo;
	if(yhi>Yhi)
		Yhi=yhi;
	WriteHiLo(FireNum);		// rewrite bounding rectangle to first perimeter
}


void Intersections::AllocIntersection(long Number)
{// allocate intersetion points array
	if(Number<=0)
          return;

	if(Number>=intersectnumalloc)
     {	FreeIntersection();
          nmemb=2*Number;
          if((intersect=new long[nmemb])==NULL)
			intersect=0;
          else
          {	intersectnumalloc=Number;
               ZeroMemory(intersect, nmemb*sizeof(long));
          }
     }
     else
     {    ZeroMemory(intersect, intersectnumalloc*2*sizeof(long));
          //for(long i=0; i<intersectnumalloc; i++)
          //{	intersect[i*2]=0;
          //     intersect[i*2+1]=0;
          //}
	}
}

void Intersections::FreeIntersection()
{
	if(intersect)
		delete[] intersect;//GlobalFree(intersect);
	intersect=0;
     intersectnumalloc=0;
}

void Intersections::AllocInterPoint(long Number)
{
	if(Number<=0)
          return;

     long i;

	if(Number>=interpointnumalloc)
     {    FreeInterPoint();
          nmemb=NUMDATA*Number;
          if((interpoint=new double[nmemb])==NULL)//(double *) GlobalAlloc(GMEM_FIXED, nmemb*sizeof(double)))==NULL)
               interpoint=0;
		else
          {	interpointnumalloc=Number;
              	ZeroMemory(interpoint, nmemb*sizeof(double));
          }
     }
     else
          ZeroMemory(interpoint, interpointnumalloc*NUMDATA*sizeof(double));
     //{    for(i=0; i<interpointnumalloc; i++)
     //     {  interpoint[i*NUMDATA]=0.0;
     //        	interpoint[i*NUMDATA+1]=0.0;
     //          interpoint[i*NUMDATA+2]=0.0;
     //         	interpoint[i*NUMDATA+3]=0.0;
     //         	interpoint[i*NUMDATA+4]=0.0;
     //     }
	//}
}

void Intersections::FreeInterPoint()
{
	if(interpoint)
		delete[] interpoint;//GlobalFree(interpoint);
	interpoint=0;
     interpointnumalloc=0;
}

void Intersections::GetIntersection(long Number, long *XOrder, long *YOrder)
{
	Number*=2;
	*XOrder=intersect[Number];
	*YOrder=intersect[++Number];
}

void Intersections::SetIntersection(long Number, long XOrder, long YOrder)
{
	Number*=2;
	intersect[Number]=XOrder;
	intersect[++Number]=YOrder;
}

void Intersections::GetInterPointCoord(long Number, double *XCoord, double *YCoord)
{
	Number*=NUMDATA;
	*XCoord=interpoint[Number];
	*YCoord=interpoint[++Number];
}

void Intersections::GetInterPointFChx(long Number, double *Ros, double *Fli, double *Rct)
{
	Number*=NUMDATA;
	*Ros=interpoint[Number+2];
	*Fli=interpoint[Number+3];
     *Rct=interpoint[Number+4];
}

void Intersections::SetInterPoint(long Number, double XCoord, double YCoord, double Ros, double Fli, double Rct)
{
	Number*=NUMDATA;
	if(XCoord==0 || YCoord==0)
		XCoord=YCoord=1;
	interpoint[Number]=XCoord;
	interpoint[++Number]=YCoord;
	interpoint[++Number]=Ros;
	interpoint[++Number]=Fli;
	interpoint[++Number]=Rct;
}

/*
long StandardizePolygon::FindExternalPoint(long CurrentFire, long)
{
     long i, j, k;
     long CurPoint, LastPoint, NextPoint;
     long NumPoints=GetNumPoints(CurrentFire);
     double xpt, ypt, xl, xn, yl, yn, tx, ty, midx, midy;
     double dist, dist1, dist2, xdiffl, ydiffl, xdiffn, ydiffn, xdiff, angle;

     CurPoint=0;
     for(i=0; i<NumPoints; i++)
     {	xpt=GetPerimeter1Value(CurrentFire, CurPoint, XCOORD);
		ypt=GetPerimeter1Value(CurrentFire, CurPoint, YCOORD);

          for(j=0; j<NumPoints; j++)
          {	xl=GetPerimeter1Value(CurrentFire, j, XCOORD);
               if(xl>xpt)
               	continue;
               if(j==CurPoint)
               	continue;
               yl=GetPerimeter1Value(CurrentFire, j, YCOORD);
               k=j+1;
               if(k>NumPoints-1)
               	k=0;
               if(k==CurPoint)
               	continue;
               yn=GetPerimeter1Value(CurrentFire, k, YCOORD);
               if(yl>=ypt && yn<=ypt)
				break;
               else if(yl<=ypt && yn>=ypt)
               	break;
          }
          if(j==NumPoints)
          {    // make sure it is convex or flat in the leftward direction
               LastPoint=CurPoint-1;
               if(LastPoint<0)
                    LastPoint=NumPoints-1;
               NextPoint=CurPoint+1;
               if(NextPoint>NumPoints-1)
                    NextPoint=0;
               xl=GetPerimeter1Value(CurrentFire, LastPoint, XCOORD);
               yl=GetPerimeter1Value(CurrentFire, LastPoint, YCOORD);
               xn=GetPerimeter1Value(CurrentFire, NextPoint, XCOORD);
               yn=GetPerimeter1Value(CurrentFire, NextPoint, YCOORD);

     		xdiffl=xpt-xl;
               ydiffl=ypt-yl;
     		xdiffn=xn-xpt;
               ydiffn=yn-ypt;
               dist1=sqrt(pow2(xdiffl)+pow2(ydiffl));
               dist2=sqrt(pow2(xdiffn)+pow2(ydiffn));
               if(dist1<dist2)
               {	tx=xpt-xn*dist1/dist2;
               	ty=ypt-yn*dist1/dist2;
                    xdiffl=xl-tx;
                    ydiffl=yl-ty;
                    midx=xl-xdiffl/2.0;
                    midy=yl-ydiffl/2.0;
                    dist=dist2*2;
               }
               else if(dist2<dist1)
               {	tx=xpt-xl*dist2/dist1;
               	ty=ypt-yl*dist2/dist1;
                    xdiffl=xn-tx;
                    ydiffl=yn-ty;
                    midx=xn-xdiffl/2.0;
                    midy=yn-ydiffl/2.0;
                    dist=dist1*2;
               }
               else	// ==
               {	xdiffl=xn-xl;
                    ydiffl=yn-yl;
                    midx=xn-xdiffl/2.0;
                    midy=yn-ydiffl/2.0;
                    dist=dist2*2;
               }
               if(xdiffl==0.0)
               {	if(ydiffl>0.0)
               		angle=PI;
               	else
                    	angle=0.0;
               }
          	else
               	angle=atan2(ydiffl, xdiffl)+PI/2.0;
               tx=xpt+dist*cos(angle);
               ty=ypt+dist*sin(angle);
               dist1=pow2(tx-xpt)+pow2(ty-ypt);
               dist2=pow2(tx-midx)+pow2(ty-midy);
               if(dist2>dist1)               // convex
               	break;
               //if(xl>=xpt && xn>=xpt)
               //    	break;
          }
          CurPoint++;
          if(CurPoint>NumPoints-1)
          	CurPoint=0;
     }

     if(i==NumPoints)
     	CurPoint=-1;

	return CurPoint;
}
*/


long StandardizePolygon::FindExternalPoint(long CurrentFire, long type)
{// matches xcoord with bounding x coordinate to ensure outside starting point for crosscompare
     bool FirstTime=true;
	long i, nump, inout, coord1, coord2, OutPoint=0, mult=1, Reverse=1;
	double testpt1, testpt2, max1, max2;

     nump=GetNumPoints(CurrentFire);
     inout=GetInout(CurrentFire);
	max1=GetPerimeter1Value(CurrentFire, nump, type);
     switch(type)
     {    case 0:       // using west as max1
               coord1=0;
               coord2=1;
               if(inout==1)
                    max2=GetPerimeter1Value(CurrentFire, nump, 3); // north max
               else
                    max2=GetPerimeter1Value(CurrentFire, nump, 2); // south max
               mult=1;
               break;
          case 1:        // using east as max1
               coord1=0;
               coord2=1;
               if(inout==1)
                    max2=GetPerimeter1Value(CurrentFire, nump, 2); // south max
               else
                    max2=GetPerimeter1Value(CurrentFire, nump, 3); // north max
               mult=-1;
               break;
          case 2:        // using south as max1
               coord1=1;
               coord2=0;
               if(inout==1)
                    max2=GetPerimeter1Value(CurrentFire, nump, 0); // west max
               else
                    max2=GetPerimeter1Value(CurrentFire, nump, 1); // east max
               mult=-1;
               break;
          case 3:        // using north as max1
               coord1=1;
               coord2=0;
               if(inout==1)
                    max2=GetPerimeter1Value(CurrentFire, nump, 1); // east max
               else
                    max2=GetPerimeter1Value(CurrentFire, nump, 0); // east max
               mult=1;
               break;
     }
     if(inout==2)
          Reverse=-1;
     mult*=Reverse;
     max2*=mult;
	for(i=0; i<nump; i++)
	{	testpt1=GetPerimeter1Value(CurrentFire, i, coord1);
		testpt2=GetPerimeter1Value(CurrentFire, i, coord2)*mult;
		if(testpt1==max1)
		{    if(FirstTime)
               {    OutPoint=i;
                    max2=testpt2;
                    FirstTime=false;
               }
               else if(testpt2<=max2)
               {    OutPoint=i;
                    max2=testpt2;
               }
		}
	}

	return OutPoint;
}


void Intersections::CheckIllogicalExpansions(long CurrentFire)
{// determines if point is inside or outside a fire polygon (CurrentFire)
 // tests for each perim1 pt if inside perim2 polygon
	long NumVertex=GetNumPoints(CurrentFire);
	long count, count1, count2;//, inside;
	long i, j, k;
	double Aangle, Bangle;
	double CuumAngle, DiffAngle;
	double Sxpt, Sypt, xptl, yptl, xptn, yptn;
	long n, isect1, isect2;
	bool Continue;
	double xpt2, ypt2, xpt2l, ypt2l, xpt2n, ypt2n;
	double yceptl, yceptn, slopel, slopen;
	double xdiffl, ydiffl, xdiffn, ydiffn;


	for(i=0; i<NumVertex; i++)
	{	startx=GetPerimeter1Value(CurrentFire, i, XCOORD);
		starty=GetPerimeter1Value(CurrentFire, i, YCOORD);
		count=0;
		CuumAngle=0.0;
		while(count<NumVertex)    // make sure that startx,starty!=x[0]y[0]
		{	Sxpt=GetPerimeter2Value(count, 0);
			Sypt=GetPerimeter2Value(count, 1);
			Aangle=direction(Sxpt, Sypt);
			count++;
			if(Aangle!=999.9)
				break;
		}
		for(count1=count; count1<=NumVertex; count1++)
		{    if(count1==NumVertex)
				count2=count-1;
			else
				count2=count1;
			Sxpt=GetPerimeter2Value(count2, 0);
			Sypt=GetPerimeter2Value(count2, 1);
			Bangle=direction(Sxpt, Sypt);
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
		if((GetInout(CurrentFire)==1 && fabs(CuumAngle)>PI) ||        // if absolute value of CuumAngle is>PI then inside polygon
			(GetInout(CurrentFire)==2 && fabs(CuumAngle)<PI))
		{    j=i-1;
			k=i+1;
			if(j<0)
				j=NumVertex-1;
			if(k>NumVertex-1)
				k=0;
			Continue=false;
			for(n=0; n<numcross; n++) // check for intersections involving point
			{	GetIntersection(n, &isect1, &isect2);
				if(isect1==j)
				{    if(k!=0)
						i=k;
					Continue=true;
					break;
				}
				else if(isect1==i)
				{	if(k<NumVertex-1 && k!=0)
						i=k+1;
					Continue=true;
					break;
				}
				else if(isect1==k)
				{    if(k<NumVertex-2 && k!=0)
						i=k+2;
					Continue=true;
					break;
				}
			}
			if(Continue)
				continue;

			if(GetPerimeter1Value(CurrentFire, i, ROSVAL)==0.0)
				continue;

			xptl=GetPerimeter1Value(CurrentFire, j, XCOORD);
			yptl=GetPerimeter1Value(CurrentFire, j, YCOORD);
			xptn=GetPerimeter1Value(CurrentFire, k, XCOORD);
			yptn=GetPerimeter1Value(CurrentFire, k, YCOORD);

			xpt2=GetPerimeter2Value(i, XCOORD);
			ypt2=GetPerimeter2Value(i, YCOORD);
			xpt2l=GetPerimeter2Value(j, XCOORD);
			ypt2l=GetPerimeter2Value(j, YCOORD);
			xpt2n=GetPerimeter2Value(k, XCOORD);
			ypt2n=GetPerimeter2Value(k, YCOORD);

			xdiffl=xpt2l-xpt2;
			ydiffl=ypt2l-ypt2;
			if(xdiffl!=0.0)
			{	slopel=ydiffl/xdiffl;
				yceptl=yptl-(slopel*xptl); //  ycept from xptl on perim1
			}
			else
				slopel=0.0;
			xdiffn=xpt2n-xpt2;
			ydiffn=ypt2n-ypt2;
			if(xdiffn!=0.0)
			{	slopen=ydiffn/xdiffn;
				yceptn=yptn-(slopen*xptn); //  ycept from xptn on perim1
			}
			else
				slopen=0.0;
			Continue=true;
			if(xdiffl!=0.0 && xdiffn!=0.0 && slopel!=slopen)
			{	Sxpt=(yceptn-yceptl)/(slopel-slopen); //xptl-((xpt2l-xpt2)/(xpt2l-xpt2n))*((xptl-xptn)/(xpt2l-xpt2n));
				Sypt=slopel*Sxpt+yceptl;
				Continue=false;
			}
			else	if(xdiffl==0.0 && xdiffn!=0.0)
			{	Sypt=slopen*xptl+yceptn;
				Sxpt=xptl;
				Continue=false;
			}
			else	if(xdiffl!=0.0 && xdiffn==0.0)
			{	Sypt=slopel*xptn+yceptl;
				Sxpt=xptn;
				Continue=false;
			}
			if(!Continue)
			{	if(Sxpt==startx && Sypt==starty)
				{	if(k!=0)
						i=k;
				}
				else
				{	SetPerimeter1(CurrentFire, i, Sxpt, Sypt);
					if(k<NumVertex-2 && k!=0)
						i=k+2;
					else
						i=NumVertex;
				}
			}
		}
	}
}


bool Intersections::SwapBarriersAndFires()
{
     long i, j;
     bool NoneAhead;

     for(i=0; i<GetNumFires(); i++)
     {    if(GetInout(i)>1)
          {    NoneAhead=true;
               for(j=i+1; j<GetNumFires(); j++)
               {    if(GetInout(j)==1)
                    {    NoneAhead=false;
                         /*if(SwapFirePerims(i, j)>0)
                         {    if(GetNumAttacks()>0)
                         	{	SetNewFireNumberForAttack(j, i);
                              	SetNewFireNumberForAttack(i, j);
                              }
                              if(GetNumAirAttacks()>0)
                         		SetNewFireNumberForAirAttack(i, j);
                         }*/
                    }
               }
               if(NoneAhead)
                    return true;
          }
     }

     return true;

/*	long i, j, k;
     long NumBarrierPoints;
     double xpt, ypt, ros, fli;

     for(i=GetNumFires()-1; i>0; i--)
     {	if(GetInout(i)<3)
     	{	for(j=i-1; j>-1; j--)
          	{	if(GetInout(j)==3)
               	{	tranz(j, 0);	// move barrier to perimeter2
                    	NumBarrierPoints=GetNumPoints(j);
					FreePerimeter1(j);
                         AllocPerimeter1(j, GetNumPoints(i)+1);
                         for(k=0; k<=GetNumPoints(i); k++)
                         {	xpt=GetPerimeter1Value(i, k, XCOORD);
						ypt=GetPerimeter1Value(i, k, YCOORD);
						ros=GetPerimeter1Value(i, k, ROSVAL);
						fli=GetPerimeter1Value(i, k, FLIVAL);
	               	     SetPerimeter1(j, k, xpt, ypt);
     	               	SetFireChx(j, k, ros, fli);
                         }
                         SetNumPoints(j, GetNumPoints(i));
                         SetInout(j, GetInout(i));
                         FreePerimeter1(i);
                         AllocPerimeter1(i, NumBarrierPoints+1);
                         SetNumPoints(i, NumBarrierPoints);
                         SetInout(i, 3);
                         tranz(i, NumBarrierPoints+1);
                    	break;	// exit for loop
                    }
               }
          }
     }

     return true;
*/
}


void Intersections::CrossesWithBarriers(long FireNum)
{// determines overlap between fires and barriers
	long i;

     if(GetInout(FireNum)==0)
		return;

     for(i=0; i<GetNumFires(); i++)
     {    if(GetInout(i)<3)
     		continue;
		else if(BoundCross(FireNum, i))
		{	SpanAStart=SpanBStart=0;
			SpanAEnd=GetNumPoints(FireNum);
			SpanBEnd=GetNumPoints(i);
   	     	CrossCompare(&FireNum, i); // won't change FireNum because no rediscretizing
     	}
     }
}


void Intersections::CrossFires(int checkmergers, long *firenum)
{// if bounding rectangles overlap, then search for crosspoints
	long type, startchex, chex, i; //, oldchex,
	long TotalFires, CurrentFire=0, NextFire, OldFire, TempFire;
	bool CheckOutwardOnly=false;
	long InwardFiresPresent=0;
	double flitest;

	if(!checkmergers) 			// if checking crosses within a single fire
	{	TotalFires=*firenum+1;		// total fires is 1
		startchex=0;			// start checking
	}
	else                          // if checking for mergers
	{    if(GetInout(0)==3)
               SwapBarriersAndFires();
     	// *firenum=0;			// start at fire 0
		TotalFires=GetNumFires();// total fires,
          startchex=1;  	     	// start checking after 1st fire
		CheckOutwardOnly=true;
	}						// for each separate fire
	for(CurrentFire=*firenum; CurrentFire<TotalFires; CurrentFire++)
	{    chex=CurrentFire+startchex;
		if(checkmergers)
		{    chex=0;
               OldFire=CurrentFire;     // save temp fire for post-frontal
          	if(CheckOutwardOnly)
			{	if(GetInout(CurrentFire)>1)
				{    //if(GetInout(CurrentFire)==3)
                    	//	BarriersPresent=1;
                    	//else
                         if(GetInout(CurrentFire)==2)
                         	InwardFiresPresent=1;
				}
                    if(CurrentFire==TotalFires-1)           // if last fire
				{    CheckOutwardOnly=false;
                    	if(InwardFiresPresent>0)
					{	CurrentFire=-1;
						continue;
                         }
				}
                    if(GetInout(CurrentFire)>1)
					continue;
			}
			else if(GetInout(CurrentFire)!=2)
				continue;

               //---------------------------------------------------------------------
			// don't merge inward fires with negative fli vertices, avoid bad crosses
               //---------------------------------------------------------------------
               else
               {	for(i=0; i<GetNumPoints(CurrentFire); i++)
               	{	flitest=GetPerimeter1Value(CurrentFire, i, FLIVAL);
                    	if(flitest<0.0)
                         {    RePositionFire(&CurrentFire);
                              if(CheckPostFrontal(GETVAL) && OldFire!=CurrentFire)
          					SetNewFireNumber(OldFire, CurrentFire, post.AccessReferenceRingNum(1, GETVAL));

                         	break;
                         }
                    }
                    if(flitest<0.0)
                    	continue;
               }
               //---------------------------------------------------------------------
               //---------------------------------------------------------------------

			if((GetInout(CurrentFire))==0)
				continue; 	   					  // don't check crosses for merged fires

			//tranz(CurrentFire, 0);
			//rediscretize(&CurrentFire);
               RePositionFire(&CurrentFire);
               if(CheckPostFrontal(GETVAL) && OldFire!=CurrentFire)
          		SetNewFireNumber(OldFire, CurrentFire, post.AccessReferenceRingNum(1, GETVAL));

			//if(CheckOutwardOnly && CurrentFire==TotalFires-1)
			//	continue;
			//else
               //if(GetInout(CurrentFire)==2)
			//	chex=1;                        		  // can't have primary fire merge with interior fire
		}
          else
          {	if(GetInout(CurrentFire)==3)
	          {	//tranz(CurrentFire, 0);
				//rediscretize(&CurrentFire);
	     	     RePositionFire(&CurrentFire);
               	continue;
               }
               else if(GetInout(CurrentFire)>0)
               	RemoveDuplicatePoints(CurrentFire);
          }
		if((GetInout(CurrentFire))==0)
			continue; 							  // don't check crosses for merged fires
          //----------------------------------------------------
		// for preventing mergers of small inward with large outward fires
          /*
		FireArea=fabs(arp(1, CurrentFire));
          */
          //----------------------------------------------------
          OldFire=CurrentFire;              			       // store old fire number for later use
		do										  // check for all mergers with "CurrentFire" fire
		{	NoCrosses=0;							  // CurrentFire number of fire mergers
			for(NextFire=chex; NextFire<TotalFires; NextFire++) // for same and other fire depending on check
			{    if(GetInout(CurrentFire)==0)
				{	NoCrosses=0;           // exit do
					break;			   // exit for
				}
				if(GetInout(NextFire)==0)
					continue;     // terminate check if no fire
				if(checkmergers)
				{    if(CheckOutwardOnly)
                    	{	if(NextFire<CurrentFire+1 && GetInout(NextFire)<3)
     	                    	continue;
          	          }
                         else if(GetInout(CurrentFire)==2 && NextFire==0)
                         {	if(GetInout(NextFire)<3)	// only allow mergers with barriers
                         		continue;
                         }
                     	if(CurrentFire==NextFire)
						continue;
					else if(GetInout(NextFire)==2)
					{	if(CheckOutwardOnly)
							continue;
						else	if(GetInout(CurrentFire)==2)
						{    //oldchex=NextFire;
							//tranz(NextFire, 0);
							//rediscretize(&NextFire);
							//NextFire=oldchex;	// save old position in for-loop
							continue;	  		// don't check crosses between inward burning fires
						}
						else	if(CurrentFire==0)
							continue;       // don't merge fire 0 with inward burning fires
					}

                         //-----------------------------------------------------------------------
                         // prevent merging if fire area of inward is < than outward
                         /*
                         if(GetInout(CurrentFire)==2)  // prevent merging
					{	if(arp(1, NextFire) >= FireArea)
							continue;
					}
					else if(GetInout(NextFire)==2)
					{	if(FireArea >= fabs(arp(1, NextFire)))
							continue;
					}*/
                         //-----------------------------------------------------------------------
                         //-----------------------------------------------------------------------
				}
				if(CurrentFire==NextFire)			  // if searching within a fire
				{    SpanAStart=SpanBStart=0;
					SpanAEnd=GetNumPoints(CurrentFire);
					SpanBEnd=GetNumPoints(NextFire);
					for(type=0; type<4; type++)
					{    ReorderPerimeter(CurrentFire, FindExternalPoint(CurrentFire, type));
						//flitest=GetPerimeter1Value(CurrentFire, 0, FLIVAL);
						//if(flitest<0.0 && type<3)
                              //	continue;

                              if(CrossCompare(&CurrentFire, NextFire))	// if it doesn't work, reorder and try again
                              	break;
					}
					*firenum=TotalFires=CurrentFire;     // update totalfires && *firenum
				}                                         // force exit from for-loops
				else	if(BoundCross(CurrentFire, NextFire))// if searching between fires & bounding rectangles overlap
				{    if(MergeInAndOutOK2(CurrentFire, NextFire))
                         {    SpanAStart=SpanBStart=0;
     					SpanAEnd=GetNumPoints(CurrentFire);
	     				SpanBEnd=GetNumPoints(NextFire);
		     		//------- do this stuff if crosses examined--------
			     	//------- only inside overlapping region-----------
				     //------- not working yet 9/20/1994----------------
					     //FindMergeSpans(NextFire);     // find overlapping bounding box
     					//SpanBStart=SpanAStart;        //
	     				//SpanBEnd=SpanAEnd;
		     			//FindMergeSpans(CurrentFire);
                              CrossCompare(&CurrentFire, NextFire);	// compare the fires pt by pt
     			     	TotalFires=GetNewFires();
                         }
				}
			}
               /*if(checkmergers)
			{	if(GetInout(CurrentFire)!=0) 		// fire maybe in new array after rediscretize
				{	tranz(CurrentFire, 0);		// transfer to perim2
					rediscretize(&CurrentFire);
				}
				if(GetInout(CurrentFire)==2)
					chex=0;
				else
					chex=CurrentFire+startchex;
			}*/
		} while(NoCrosses>0);
		CurrentFire=OldFire;		// restore old fire number as current number before ++
	}
}


bool Intersections::AllocCrossThreads()
{
	if(NumCrossThreads==GetMaxThreads())
     	return true;

     CloseCrossThreads();
     crossthread=new CrossThread[GetMaxThreads()];

     if(!crossthread)
	{	NumCrossThreads=0;

          return false;
     }

    	NumCrossThreads=GetMaxThreads();

     return true;
}


void Intersections::CloseCrossThreads()
{
	long m;

     if(NumCrossThreads==0)
     	return;
     for(m=0; m<NumCrossThreads; m++)
		crossthread[m].SetRange(0, -1);
     for(m=0; m<NumCrossThreads; m++)
     	crossthread[m].StartCrossThread(m, 0, 0);
     Sleep(50);
     WaitForFarsiteEvents(EVENT_CROSS, NumCrossThreads, true, INFINITE);

	FreeCrossThreads();
}

void Intersections::FreeCrossThreads()
{
	if(crossthread)
     	delete[] crossthread;

     crossthread=0;
     NumCrossThreads=0;
}


bool Intersections::EliminateCrossPoints(long CurrentFire)
{
     bool Modified=false;
     long i, j, NumPts, BadPt;
     double *perimeter1;
     double xpt, ypt, xptn, yptn;
     double xdiff, ydiff, dist, offset;
     //double, xoffset, yoffset, xa, ya;

     // dithers duplicate points that cross;
     NumPts=GetNumPoints(CurrentFire);
     for(i=0; i<numcross; i++)
     {    BadPt=0;
          if(labs(intersect[i*2])>labs(intersect[i*2+1]))
               continue;
          if(intersect[i*2]<0)
               BadPt=labs(intersect[i*2]);
          //else if(intersect[i*2+1]<0)
          //     BadPt=labs(intersect[i*2]);
          if(BadPt>0)
          {    Modified=true;
               xpt=GetPerimeter1Value(CurrentFire, BadPt, XCOORD);
               ypt=GetPerimeter1Value(CurrentFire, BadPt, YCOORD);
               j=BadPt+1;
               if(j>NumPts-1)
                    j-=NumPts;
               xptn=GetPerimeter1Value(CurrentFire, j, XCOORD);
               yptn=GetPerimeter1Value(CurrentFire, j, YCOORD);
               xdiff=xpt-xptn;
               ydiff=ypt-yptn;
               dist=sqrt(pow2(xdiff)+pow2(ydiff));

               if(dist<1e-4) // basically zero, so get rid of the point, this will avoid endless loop
               {    perimeter1=GetPerimeter1Address(CurrentFire, 0);
                    CopyMemory(&perimeter1[BadPt*NUMDATA], &perimeter1[(BadPt+1)*NUMDATA], (NumPts-BadPt)*NUMDATA*sizeof(double));
                    NumPts--;
                    for(j=i+1; j<numcross; j++)
                    {    if(labs(intersect[j*2])==BadPt || labs(intersect[j*2+1])==BadPt)
                         {    intersect[j*2]=0;
                              intersect[j*2+1]=0;
                         }
                         else
                         {    if(labs(intersect[j*2])>BadPt)
                              {    if(intersect[j*2]<0)
                                        intersect[j*2]+=1;
                                   else
                                        intersect[j*2]-=1;
                              }
                              if(labs(intersect[j*2+1])>BadPt)
                              {    if(intersect[j*2+1]<0)
                                        intersect[j*2+1]+=1;
                                   else
                                        intersect[j*2+1]-=1;
                              }
                         }
                    }
               }
               else         // not zero distance between points, so just dither it by 0.1m or ft
               {    /*
                    xoffset=((double) (rand2(&idum)*100))/999.0;	// randomize distance offset
                    xoffset*=dist;
                    if(xoffset>0.1)
                         xoffset=0.1;
                    yoffset=((double) (rand2(&idum)*100))/999.0;	// randomize distance offset
                    yoffset*=dist;
                    if(yoffset>0.1)
                         yoffset=0.1;
                    xa=((double) ((rand2(&idum)*20))/20.0+1.0)*PI;	// randomize xdirection offset
                    ya=((double) ((rand2(&idum)*20))/20.0+1.0)*PI;	// randomize ydirection offset
                    xpt=xpt-xoffset*cos(xa);//*xdiff/dist;
                    ypt=ypt-yoffset*sin(ya);//*ydiff/dist;
                    */
                    offset=0.1*dist;
                    j=0;
                    while(offset>0.1)
                    {    offset*=0.1;
                         j++;
                         if(j>50)
                              break;
                    };
                    xpt-=offset*xdiff/dist;
                    ypt-=offset*ydiff/dist;
                    SetPerimeter1(CurrentFire, BadPt, xpt, ypt);
                    for(j=i+1; j<numcross; j++)
                    {    if(labs(intersect[j*2])==BadPt || labs(intersect[j*2+1])==BadPt)
                         {    intersect[j*2]=labs(intersect[j*2]);
                              intersect[j*2+1]=labs(intersect[j*2+1]);
                         }
                    }
               }
          }
     }

     if(NumPts<GetNumPoints(CurrentFire))
          SetNumPoints(CurrentFire, NumPts);

     return Modified;
}


bool Intersections::CrossCompare(long *CurrentFire, long NextFire)
{
     bool      Result=true, Repeat=false;
     long      i, j, k, m, begin, end, threadct;
     long      range, cxx, NewFires1, NewFires2, *PostFires, OldFire;  	      // for post frontal stuff
     long      inside=0, NumCheckReverse=0, CheckReverse=0, begincross=0;
     long      TestCross=-1, NumTimes=0, PriorNumPoints;
	double	fract, interval, ipart, RainRate, Interval, r1, r2, D1, angle1, angle2;
     double    xpt, ypt, crxpt, crypt, crxptn, cryptn, newx, newy;
     long      CrossMemAlloc=GetNumPoints(*CurrentFire)+GetNumPoints(NextFire);

	AllocIntersection(CrossMemAlloc);      // allocate array of intersection orders
     AllocInterPoint(CrossMemAlloc);	    // allocate array of intersection points
     AllocCrossThreads();
     do
     {    Repeat=false;
          interval=((double) GetNumPoints(*CurrentFire))/((double) NumCrossThreads);
          fract=modf(interval, &ipart);
          range=(long) interval;
          if(fract>0.0)
              	range++;

          begin=threadct=0;
          for(i=0; i<NumCrossThreads; i++)
          {	end=begin+range;
               if(begin>=GetNumPoints(*CurrentFire))
                   	continue;
               if(end>GetNumPoints(*CurrentFire))
                    	end=GetNumPoints(*CurrentFire);
               crossthread[i].SetRange(begin, end);
               threadct++;
               begin=end;
          }

          for(j=0; j<threadct; j++)
         	     crossthread[j].StartCrossThread(j, *CurrentFire, NextFire);
          WaitForFarsiteEvents(EVENT_CROSS, threadct, true, INFINITE);

          numcross=0;

          j=0;
          for(i=0; i<threadct; i++)
               j+=crossthread[i].GetNumCross();
          if(j>=CrossMemAlloc)
          {    CrossMemAlloc+=j;
               AllocIntersection(CrossMemAlloc);      // allocate array of intersection orders
               AllocInterPoint(CrossMemAlloc);	    // allocate array of intersection points
          }

          for(i=0; i<threadct; i++)
          {    j=crossthread[i].GetNumCross();
               CopyMemory(&intersect[numcross*2], crossthread[i].GetIsect(), j*2*sizeof(long));
               CopyMemory(&interpoint[numcross*NUMDATA], crossthread[i].GetIpoint(), j*NUMDATA*sizeof(double));
               numcross+=j;
          }
          if(numcross>0 && *CurrentFire==NextFire)
          {    if(EliminateCrossPoints(*CurrentFire))
               {    numcross=0;
                    Repeat=true;
                    continue;
               }
               else
                    Repeat=false;
          }

          if(numcross>0 && NumTimes>0 && *CurrentFire==NextFire)
          {	if(GetInout(*CurrentFire)==2)
			{    for(k=0; k<numcross; k++)
                    {
                    GetIntersection(k, &i, &j);
                    if(CheckReverse==0)
				{    GetInterPointCoord(k, &newx, &newy);
                         m=j+1;
                         if(m>GetNumPoints(*CurrentFire)-1)
                              m=0;
                         xpt=GetPerimeter1Value(*CurrentFire, i, XCOORD);
                         ypt=GetPerimeter1Value(*CurrentFire, i, YCOORD);
                         crxpt=GetPerimeter1Value(*CurrentFire, j, XCOORD);
                         crypt=GetPerimeter1Value(*CurrentFire, j, YCOORD);
                         crxptn=GetPerimeter1Value(*CurrentFire, m, XCOORD);
                         cryptn=GetPerimeter1Value(*CurrentFire, m, YCOORD);

                         startx=newx;
                         starty=newy;
					angle1=direction(xpt, ypt);
					angle2=direction(crxptn, cryptn);
					D1=sin(angle2)*cos(angle1)-cos(angle2)*sin(angle1);
					if(D1>=0.0 || (pow2(newx-xpt)+ pow2(newy-ypt))<1e-12)
					{	startx=xpt-(xpt-crxpt)/2.0;
						starty=ypt-(ypt-crypt)/2.0;
						if(!Overlap(*CurrentFire))
							inside=1;
						else
							inside=-1;
					}
					else
						inside=-1; 		// don't check on mirror of cross order
					begincross=i+1;
                         if(begincross>GetNumPoints(*CurrentFire)-1)
                              begincross=0;
					TestCross=i;
					CheckReverse=1;
				}
				else if(i==TestCross)
					CheckReverse*=-1;
                    }
			}
          }
	     if(numcross>0 && *CurrentFire==NextFire)
     	{    Repeat=false;
               if(NumTimes==0)  // do this always for inward and outward fires
		     {	OrganizeCrosses(*CurrentFire);
                    PriorNumPoints=GetNumPoints(*CurrentFire);
				     //count1=SpanAStart-1;
     				//SpanAEnd=SpanBEnd=GetNumPoints(*CurrentFire);
                    numcross=0;
                    Repeat=true;
		     }
		     else if(GetInout(*CurrentFire)==2 && inside*CheckReverse==1)// && GetInout(*CurrentFire)==2)// reverse decision if
     		{	ReorderPerimeter(*CurrentFire, begincross);  // reorder without origin inside loop
	     			//count1=SpanAStart-1;           			// start again
		     	numcross=0;
			    	Repeat=true;
                    if(++NumCheckReverse>=GetNumPoints(*CurrentFire))
			     {	CheckReverse=-1;					// don't repeat it
		    		     inside=0;                               // prevent repeat
     	    		}
	         		else
		    			CheckReverse=0;   // must repeat until 1/2 vertices examined in case of twin spike
		     }
               else
                    Repeat=false;
     		NumTimes=1;
     	}
     } while(Repeat==true);

     //-------------------------------------------------------------
	// NEW ALGORITHM PRESERVES ONLY OUTERPERIMETER
	//-------------------------------------------------------------
	if(numcross>0)		// was !=0 but now prevents single crossovers, that shouldn't happen anyway??!!
	{    if(*CurrentFire==NextFire && numcross>=CrossMemAlloc)
		{    if(GetInout(*CurrentFire)==1)
			{	FindOuterFirePerimeter(*CurrentFire);
				rediscretize(CurrentFire, true);
				numcross=0;
			}
			else
				numcross=1;   // eliminate by forcing into trap for non-even numcrosses
		}
	}
	//-------------------------------------------------------------

	if(numcross>0)		// was !=0 but now prevents single crossovers, that shouldn't happen anyway??!!
	{	cxx=(long) numcross/2.0;
		if(cxx*2!=numcross && GetInout(NextFire)<3)// || numcross>=CrossMemAlloc)		// trap for odd number of cross points
		{	SetNumPoints(NextFire, 0);                        // and overly large number of crosses on
			SetInout(NextFire, 0);                            // spaghetti fronts
			IncSkipFires(1);
			FreePerimeter1(NextFire);
			numcross=0;
		}
		else if(*CurrentFire!=NextFire && GetInout(NextFire)<3)  // if merging two fires then
		{	ExchangeRect(*CurrentFire);	          // determine and write combined bounding rectangle
               OrganizeIntersections(*CurrentFire);    // change order of intersected points
			NoCrosses++; 	                         // tally the number of times a fire has merged
		}
		if(numcross>0)
		{    if(*CurrentFire==NextFire)			// added 6/25/1995
			{	//if(CheckExpansions(GETVAL))		// if model options set for checking expansions
				//	CheckIllogicalExpansions(*CurrentFire);
				writenum=1;		// make sure perimeter is written in CleanPerimeter

                    NewFires1=GetNewFires();

				CleanPerimeter(*CurrentFire);      // REMOVE IF USING OLD ALGORITHM
                    if(CheckPostFrontal(GETVAL) && GetNumPoints(*CurrentFire)>0)       // for
                    {    NewFires2=GetNewFires();
                    	if((PostFires=new long[NewFires2-NewFires1+1])!=NULL)//(long *) GlobalAlloc(GMEM_FIXED, (NewFires2-NewFires1+1)*sizeof(long)))!=NULL)
                         {    PostFires[0]=*CurrentFire;
                         	for(i=1; i<(NewFires2-NewFires1+1); i++)
                         		PostFires[i]=NewFires1+i-1;
     	                    post.CorrectFireRing(numcross, intersect, interpoint, PostFires, NewFires2-NewFires1+1, PriorNumPoints);
				     	delete[] PostFires;//GlobalFree(PostFires);
                         }
                    }
				//FindOuterFirePerimeter(*CurrentFire);
				if(GetNumPoints(*CurrentFire)>0)	// SET TO 0
                    {    rediscretize(CurrentFire, false);  	// REMOVE IF USING OLD ALGORITHM
                         if(CheckPostFrontal(GETVAL) && NextFire!=*CurrentFire)
                              SetNewFireNumber(NextFire, *CurrentFire, post.AccessReferenceRingNum(1, GETVAL));
                    }
			}
			else
			{    if(GetInout(NextFire)==3)		// if merging fire with barrier
                    {    OrganizeIntersections(*CurrentFire);
                         MergeBarrier(CurrentFire, NextFire);
                    	//NoCrosses--;
                    }
               	else
	               {   	if(GetInout(*CurrentFire)==2)
					{	if(GetNumPoints(*CurrentFire)<GetNumPoints(NextFire))
							writenum=1;
					}
					if(!MergeFire(CurrentFire, NextFire))    	    // merge the fires
						Result=false;
                         Result=false;
                    }
			}
		}
	}
	else	if(*CurrentFire==NextFire)		// if no crosspoints, and searching within same fire
	{    if(GetNumPoints(*CurrentFire)>0)   // if points in fire
		{	tranz(*CurrentFire, 0);
			rediscretize(CurrentFire, false);
               if(CheckPostFrontal(GETVAL) && NextFire!=*CurrentFire)
                    SetNewFireNumber(NextFire, *CurrentFire, post.AccessReferenceRingNum(1, GETVAL));
		}
	}                             	// no crosses and searching between fires
     else if(GetInout(*CurrentFire)==1 && GetInout(NextFire)==1)
          CheckEnvelopedFires(*CurrentFire, NextFire);
     else if(GetInout(NextFire)==3)
     {	if(GetInout(*CurrentFire)==1)
	          CheckEnvelopedFires(*CurrentFire, NextFire);
     	else if(GetInout(*CurrentFire)==2)
      		MergeBarrierNoCross(CurrentFire, NextFire);
     }
	//FreeIntersection();
	//FreeInterPoint();

	return Result;
}



bool Intersections::CrossCompare1(long *CurrentFire, long NextFire)
{// determines which points crossed into alread burned areas
	int crossyn=0;
	bool Result=true;
	long CrossMemAlloc=GetNumPoints(*CurrentFire)+GetNumPoints(NextFire);
	long begincross=0, CheckReverse=0, TestCross, NumCheckReverse=0, NumTimes=0;
	long count1, count3, count4=0, count5, inside=0, cxx=0, PriorNumPoints;
     long i, NewFires1, NewFires2, *PostFires;  	      // for post frontal stuff
	double crxdiff, crydiff=0, crxpt, crxptn, crypt, cryptn;
	double slope=0, crslope=0, xcommon, ycommon, ycept=0, crycept=0;
	double xpt, ypt, xptn, yptn=0, xdiff, ydiff=0, newx=0, newy=0;
	double ros1=0, ros2, ros3, fli1=0, fli2, fli3, fli4;
     double angle1, angle2, D1;
     double rcx1, rcx2, rcx3;

     numcross=0;
	AllocIntersection(CrossMemAlloc);      // allocate array of intersection orders
	AllocInterPoint(CrossMemAlloc);	    // allocate array of intersection points
	for(count1=SpanAStart; count1<SpanAEnd; count1++)   // for each point in a given fire
	{	xpt=GetPerimeter1Value(*CurrentFire, count1, XCOORD);
		ypt=GetPerimeter1Value(*CurrentFire, count1, YCOORD);
		if(count1!=GetNumPoints(*CurrentFire)-1)
			count4=count1+1;
		else
			count4=0;
		xptn=GetPerimeter1Value(*CurrentFire, count4, XCOORD);
		yptn=GetPerimeter1Value(*CurrentFire, count4, YCOORD);
		xdiff=xpt-xptn;
		ydiff=ypt-yptn;
          if(fabs(xdiff)<1e-9)
          	xdiff=0.0;
		if(xdiff!=0.0)		/* slope and intercept of target perimeter segment */
		{	slope=ydiff/xdiff;
			ycept=ypt-(slope*xpt);
		}
		for(count3=SpanBStart; count3<SpanBEnd; count3++)	// FOR ALL FIRES CHECK FOR ALL CROSSES FROM THE FIRST ELEMENT
		{	crxpt=GetPerimeter1Value(NextFire, count3, XCOORD);
			crypt=GetPerimeter1Value(NextFire, count3, YCOORD);
			if(count3!=GetNumPoints(NextFire)-1)
				count5=count3+1;
			else
				count5=0;
			if(*CurrentFire==NextFire)	// don't check for crosses on same or adjacent points within same array
			{	if(count1==count3 || count1==count5 || count4==count3 || count4==count5)
					continue;
			}
			crxptn=GetPerimeter1Value(NextFire, count5, XCOORD);
			cryptn=GetPerimeter1Value(NextFire, count5, YCOORD);
			crxdiff=crxpt-crxptn;
			crydiff=crypt-cryptn;
               if(fabs(crxdiff)<1e-9)
               	crxdiff=0.0;
			if(crxdiff!=0.0)		// slope and intercept of other perimeter segments
			{	crslope=crydiff/crxdiff;
				crycept=crypt-(crslope*crxpt);
			}
			if(xdiff!=0.0 && crxdiff!=0.0 && slope!=crslope)		 // see if x-dimension is common
			{	xcommon=(crycept-ycept)/(slope-crslope);
				if((xcommon>xptn && xcommon<=xpt) || (xcommon<xptn && xcommon>=xpt))
				{	if((xcommon>=crxpt && xcommon<crxptn) || (xcommon<=crxpt && xcommon>crxptn))
					{    if(xpt!=crxpt && xcommon!=xptn && xcommon!=crxptn)
						{	newx=xcommon;
							ycommon=ycept+slope*xcommon;
							newy=ycommon;
							crossyn=1;
						}
					}
				}
			}
			else                                    // see if y-dimension is common
			{	if(xdiff==0.0 && crxdiff!=0.0)
				{	ycommon=crslope*xpt+crycept;
					if((ycommon<yptn && ycommon>=ypt) || (ycommon>yptn && ycommon<=ypt))
					{   if((xpt<=crxpt && xpt>crxptn) || (xpt>=crxpt && xpt<crxptn))
						{	newx=xpt;
							newy=ycommon;
							crossyn=1;
						}
					}
				}
				else
				{	if(xdiff!=0.0 && crxdiff==0.0)
					{	ycommon=slope*crxpt+ycept;
						if((ycommon<cryptn && ycommon>=crypt) || (ycommon>cryptn && ycommon<=crypt))
						{    if((crxpt<=xpt && crxpt>xptn) || (crxpt>=xpt && crxpt<xptn))
							{	newx=crxpt;
								newy=ycommon;
								crossyn=1;
							}
						}
					}
				}
			}
			if(crossyn==1)
			{    if(NumTimes>0 && *CurrentFire==NextFire)
				{	if(GetInout(*CurrentFire)==2)
					{    if(CheckReverse==0)
						{    startx=newx; starty=newy;
							angle1=direction(xpt, ypt);
							angle2=direction(crxptn, cryptn);
							D1=sin(angle2)*cos(angle1)-cos(angle2)*sin(angle1);
							if(D1>=0.0)
							{	startx=xpt-(xpt-crxpt)/2.0;
								starty=ypt-(ypt-crypt)/2.0;
								if(!Overlap(*CurrentFire))
									inside=1;
								else
									inside=-1;
							}
							else if(newx==xpt && newy==ypt)
							{	startx=xpt-(xpt-crxpt)/2.0;
								starty=ypt-(ypt-crypt)/2.0;
								if(!Overlap(*CurrentFire))
									inside=1;
								else
									inside=-1;
							}
							else
								inside=-1; 		// don't check on mirror of cross order
							begincross=count1+1;
							TestCross=count1;
							CheckReverse=1;
						}
						else if(count1==TestCross)
							CheckReverse*=-1;
					}
				}
				if(numcross<CrossMemAlloc)
				{	SetIntersection(numcross, count1, count3);	// point of order count1 in first fire, count3 on second fire
					ros1=GetPerimeter1Value(*CurrentFire, count1, ROSVAL);
					fli1=GetPerimeter1Value(*CurrentFire, count1, FLIVAL);
					rcx1=GetPerimeter1Value(*CurrentFire, count1, RCXVAL);
					ros2=GetPerimeter1Value(*CurrentFire, count4, ROSVAL);
					fli2=GetPerimeter1Value(*CurrentFire, count4, FLIVAL);
					rcx2=GetPerimeter1Value(*CurrentFire, count4, RCXVAL);
					ros1=(ros1+ros2)/2.0;				// average rate of spread for perimeter segment
					fli3=(fabs(fli1)+fabs(fli2))/2.0;				// average fireline intensity for perimeter segment
					if(fli1<0.0 || fli2<0.0)
						fli3*=-1.0;
					fli1=fli3;
					rcx1=(rcx1+rcx2)/2.0;				// average rate of spread for perimeter segment
                         ros2=GetPerimeter1Value(NextFire, count3, ROSVAL);
					fli2=GetPerimeter1Value(NextFire, count3, FLIVAL);
					rcx2=GetPerimeter1Value(NextFire, count3, RCXVAL);
					ros3=GetPerimeter1Value(NextFire, count5, ROSVAL);
					fli3=GetPerimeter1Value(NextFire, count5, FLIVAL);
					rcx3=GetPerimeter1Value(NextFire, count5, FLIVAL);
					ros2=(ros2+ros3)/2.0;				// average rate of spread for perimeter segment
					fli4=(fabs(fli2)+fabs(fli3))/2.0;				// average rate of spread for perimeter segment
					if(fli2<0.0 || fli3<0.0)
						fli4*=-1.0;
					fli2=fli4;
					rcx2=(rcx2+rcx3)/2.0;				// average rate of spread for perimeter segment
					ros3=(ros1+ros2)/2.0;			  		// ROS for intersection point
					fli4=(fabs(fli1)+fabs(fli2))/2.0;			  		// FLI for intersection point
					if(fli1<0.0 && fli2<0.0)
						fli4*=-1.0;
					fli3=fli4;
					rcx3=(rcx1+rcx2)/2.0;			  		// ROS for intersection point
					SetInterPoint(numcross, newx, newy, ros3, fli3, rcx3);	 //write intersected points
                    	if(GetInout(*CurrentFire)==2 && GetInout(NextFire)==1)
                         {	if(fli2<0.0)        // check for stationary points on outward fire
                         	{	numcross=0;	// if found dont merge with inward burning fire
                              	//FreeIntersection();
                                   //FreeInterPoint();

                                   return false;
                              }
                         }
					numcross++;	// allow outward fire to get only outward fire perimeter
				}
			}
			crossyn=0;
		}
		if(count1==SpanAEnd-1 && *CurrentFire==NextFire)
		{    if(numcross>0 && numcross<CrossMemAlloc)
			{	if(NumTimes==0)  // do this always for inward and outward fires
				{	OrganizeCrosses(*CurrentFire);
                    	PriorNumPoints=GetNumPoints(*CurrentFire);
					count1=SpanAStart-1;
					SpanAEnd=SpanBEnd=GetNumPoints(*CurrentFire);
					numcross=0;
				}
				else if(GetInout(*CurrentFire)==2 && inside*CheckReverse==1)// && GetInout(*CurrentFire)==2)// reverse decision if
				{	ReorderPerimeter(*CurrentFire, begincross);  // reorder without origin inside loop
					count1=SpanAStart-1;           			// start again
					numcross=0;
					if(++NumCheckReverse>=GetNumPoints(*CurrentFire))
					{	CheckReverse=-1;					// don't repeat it
						inside=0;                               // prevent repeat
					}
					else
						CheckReverse=0;   // must repeat until 1/2 vertices examined in case of twin spike
				}
				NumTimes=1;
			}
			/*
			if(GetInout(*CurrentFire)==2 && numcross>0)
			{	switch(CheckReverse)
				{	case 0:
						OrganizeCrosses(*CurrentFire);
						//if(GetNumPoints(*CurrentFire)>SpanAEnd)
						//{
							count1=SpanAStart-1;
							SpanAEnd=SpanBEnd=GetNumPoints(*CurrentFire);
							numcross=0;
						//}
						CheckReverse=1;	// don't do this again
						break;
					case 1:
						writenum=-1;
						CleanPerimeter(*CurrentFire);
						if(writenum<0)
						{    GetIntersection(0, &count1, &count3);
							ReorderPerimeter(*CurrentFire, count1+1);
							count1=SpanAStart-1;
							numcross=0;
						}
						CheckReverse=2;	// don't do this again
						break;
					case 2:
						break;
				}
			}
			*/
		}
	}
     
	//-------------------------------------------------------------
	// NEW ALGORITHM PRESERVES ONLY OUTERPERIMETER
	//-------------------------------------------------------------
	if(numcross>0)		// was !=0 but now prevents single crossovers, that shouldn't happen anyway??!!
	{    if(*CurrentFire==NextFire && numcross>=CrossMemAlloc)
		{    if(GetInout(*CurrentFire)==1)
			{	FindOuterFirePerimeter(*CurrentFire);
				rediscretize(CurrentFire, true);
				numcross=0;
			}
			else
				numcross=1;   // eliminate by forcing into trap for non-even numcrosses
		}
	}
	//-------------------------------------------------------------

	if(numcross>0)		// was !=0 but now prevents single crossovers, that shouldn't happen anyway??!!
	{	cxx=(int) numcross/2.0;
		if(cxx*2!=numcross && GetInout(NextFire)<3)// || numcross>=CrossMemAlloc)		// trap for odd number of cross points
		{	SetNumPoints(NextFire, 0);                        // and overly large number of crosses on
			SetInout(NextFire, 0);                            // spaghetti fronts
			IncSkipFires(1);
			FreePerimeter1(NextFire);
			numcross=0;
		}
		else if(*CurrentFire!=NextFire && GetInout(NextFire)<3)  // if merging two fires then
		{	ExchangeRect(*CurrentFire);	          // determine and write combined bounding rectangle
               OrganizeIntersections(*CurrentFire);    // change order of intersected points
			NoCrosses++; 	                         // tally the number of times a fire has merged
		}
		if(numcross>0)
		{    if(*CurrentFire==NextFire)			// added 6/25/1995
			{	//if(CheckExpansions(GETVAL))		// if model options set for checking expansions
				//	CheckIllogicalExpansions(*CurrentFire);
				writenum=1;		// make sure perimeter is written in CleanPerimeter

                    NewFires1=GetNewFires();

				CleanPerimeter(*CurrentFire);      // REMOVE IF USING OLD ALGORITHM
                    if(CheckPostFrontal(GETVAL))       // for
                    {    NewFires2=GetNewFires();
                    	if((PostFires=new long[NewFires2-NewFires1+1])!=NULL)//(long *) GlobalAlloc(GMEM_FIXED, (NewFires2-NewFires1+1)*sizeof(long)))!=NULL)
                         {    PostFires[0]=*CurrentFire;
                         	for(i=1; i<(NewFires2-NewFires1+1); i++)
                         		PostFires[i]=NewFires1+i-1;
     	                    post.CorrectFireRing(numcross, intersect, interpoint, PostFires, NewFires2-NewFires1+1, PriorNumPoints);
				     	delete[] PostFires;//GlobalFree(PostFires);
                         }
                    }
				//FindOuterFirePerimeter(*CurrentFire);
				if(GetNumPoints(*CurrentFire)>0)	// SET TO 0
					rediscretize(CurrentFire, false);  	// REMOVE IF USING OLD ALGORITHM
                    if(CheckPostFrontal(GETVAL) && NextFire!=*CurrentFire)
                         SetNewFireNumber(NextFire, *CurrentFire, post.AccessReferenceRingNum(1, GETVAL));
			}
			else
			{    if(GetInout(NextFire)==3)		// if merging fire with barrier
                    {    OrganizeIntersections(*CurrentFire);
                         MergeBarrier(CurrentFire, NextFire);
                    	//NoCrosses--;
                    }
               	else
	               {   	if(GetInout(*CurrentFire)==2)
					{	if(GetNumPoints(*CurrentFire)<GetNumPoints(NextFire))
							writenum=1;
					}
					if(!MergeFire(CurrentFire, NextFire))    	    // merge the fires
						Result=false;
                         Result=false;
                    }
			}
		}
	}
	else	if(*CurrentFire==NextFire)		// if no crosspoints, and searching within same fire
	{    if(GetNumPoints(*CurrentFire)>0)   // if points in fire
		{	tranz(*CurrentFire, 0);
			rediscretize(CurrentFire, false);
               if(CheckPostFrontal(GETVAL) && NextFire!=*CurrentFire)
                    SetNewFireNumber(NextFire, *CurrentFire, post.AccessReferenceRingNum(1, GETVAL));
     	}
	}                             	// no crosses and searching between fires
     else if(GetInout(*CurrentFire)==1 && GetInout(NextFire)==1)
          CheckEnvelopedFires(*CurrentFire, NextFire);
     else if(GetInout(NextFire)==3)
          CheckEnvelopedFires(*CurrentFire, NextFire);
	//FreeIntersection();
	//FreeInterPoint();

	return Result;
}


void Intersections::CheckEnvelopedFires(long Fire1, long Fire2)
{
     long i, j, k, m;
     long NumPts1, NumPts2;
     long NumSmall, FireSmall, FireBig, NumBig;
     long Inside1, Inside2;
     double XLo1, XLo2, XHi1, XHi2;
     double YLo1, YLo2, YHi1, YHi2;
     double fli;

     NumPts1=GetNumPoints(Fire1);
     NumPts2=GetNumPoints(Fire2);
     XLo1=GetPerimeter1Value(Fire1, NumPts1, XCOORD);
     XHi1=GetPerimeter1Value(Fire1, NumPts1, YCOORD);
     YLo1=GetPerimeter1Value(Fire1, NumPts1, XCOORD);
     YHi1=GetPerimeter1Value(Fire1, NumPts1, YCOORD);
     XLo2=GetPerimeter1Value(Fire2, NumPts2, XCOORD);
     XHi2=GetPerimeter1Value(Fire2, NumPts2, YCOORD);
     YLo2=GetPerimeter1Value(Fire2, NumPts2, XCOORD);
     YHi2=GetPerimeter1Value(Fire2, NumPts2, YCOORD);
     Inside1=0;
     Inside2=1;

     // fires must overlap to get to this point, thus must find out which
     // one is smaller and could fit inside larger one
     if((XHi1-XLo1)*(YHi1-YLo1)<(XHi2-XLo2)*(YHi2-YLo2))
     {    NumSmall=NumPts1;
          NumBig=NumPts2;
          FireSmall=Fire1;
          FireBig=Fire2;
     }
     else
     {    NumSmall=NumPts2;
          NumBig=NumPts1;
          FireSmall=Fire2;
          FireBig=Fire1;
     }

     for(i=0; i<NumSmall; i++)
     {    startx=GetPerimeter1Value(FireSmall, i, XCOORD);
          starty=GetPerimeter1Value(FireSmall, i, YCOORD);
          fli=GetPerimeter1Value(FireSmall, i, FLIVAL);
          if(fli>=0.0)                       // don't test for extinguished points
               Inside1=Overlap(FireBig);
          else
               continue;
          if(Inside1)
          {    Inside2=0;
               for(j=0; j<GetNewFires(); j++)
               {    if(GetInout(j)==2)
                    {    //NumPtsIn=GetNumPoints(j);
                         if(BoundCross(FireBig, j))             // if(overlap of outer and inner)
                         {    Inside1=0;
                         	for(m=0; m<GetNumPoints(j); m++)
                         	{   	startx=GetPerimeter1Value(j, m, XCOORD);
	                              starty=GetPerimeter1Value(j, m, YCOORD);
                                   Inside1=Overlap(FireBig);
                                   if(Inside1)
                                   	break;
                              }
     	                  	if(Inside1)//!MergeInAndOutOK(FireBig, j))   // if(Inward is inside big one)
                              {    for(k=0; k<NumSmall; k++)
                                   {    fli=GetPerimeter1Value(FireSmall, k, FLIVAL);
                                        if(fli>0.0)    // no points on barren ground
                                        {    startx=GetPerimeter1Value(FireSmall, k, XCOORD);
                                             starty=GetPerimeter1Value(FireSmall, k, YCOORD);
                                             Inside2=Overlap(j);
                                             if(Inside2)
                                                  break;
                                        }
                                   }
                              }
                              else
                                   continue;
                         }
                    }
                    if(Inside2)
                         break;
               }
               i=NumSmall;         // force exit from for loop
          }
     }
     if(!Inside2)
     {    SetInout(FireSmall, 0);
     	SetNumPoints(FireSmall, 0);
	     IncSkipFires(1);
		FreePerimeter1(FireSmall);
          if(CheckPostFrontal(GETVAL))
               SetNewFireNumber(FireSmall, -1, post.AccessReferenceRingNum(1, GETVAL));
     }
}


void StandardizePolygon::ReorderPerimeter(long CurrentFire, long NewBeginning)
{// reorders points in CurrentFire from NewBeginning because startpoint is inside loop or crossover
	long count, newcount=0;
	double xpt, ypt, ros, fli, rcx;

     if(NewBeginning==0)
     	return;
	if(GetPerimeter2Value(GETVAL, GETVAL)<GetNumPoints(CurrentFire))	// if less than NewBeginning # of pts in perim2
	{	FreePerimeter2();
		AllocPerimeter2(GetNumPoints(CurrentFire)+1);
	}
	for(count=0; count<NewBeginning; count++)		// read early points into perim2
	{    xpt=GetPerimeter1Value(CurrentFire, count, XCOORD);
		ypt=GetPerimeter1Value(CurrentFire, count, YCOORD);
		ros=GetPerimeter1Value(CurrentFire, count, ROSVAL);
		fli=GetPerimeter1Value(CurrentFire, count, FLIVAL);
          rcx=GetPerimeter1Value(CurrentFire, count, RCXVAL);
		SetPerimeter2(count, xpt, ypt, ros, fli, rcx);
	}
	for(count=NewBeginning; count<GetNumPoints(CurrentFire); count++)
	{    xpt=GetPerimeter1Value(CurrentFire, count, XCOORD);  	// read remaining points to difft
		ypt=GetPerimeter1Value(CurrentFire, count, YCOORD);	// ...position in perim1
		ros=GetPerimeter1Value(CurrentFire, count, ROSVAL);
		fli=GetPerimeter1Value(CurrentFire, count, FLIVAL);
          rcx=GetPerimeter1Value(CurrentFire, count, RCXVAL);
		SetPerimeter1(CurrentFire, newcount, xpt, ypt);
		SetFireChx(CurrentFire, newcount, ros, fli);
          SetReact(CurrentFire, newcount, rcx);
		++newcount;
	}
	for(count=0; count<NewBeginning; count++)              // read early points at end
	{    GetPerimeter2(count, &xpt, &ypt, &ros, &fli, &rcx);
//     	xpt=GetPerimeter2Value(count, 0);	//... of perim1
//		ypt=GetPerimeter2Value(count, 1);
//		ros=GetPerimeter2Value(count, 2);
//		fli=GetPerimeter2Value(count, 3);
		SetPerimeter1(CurrentFire, newcount, xpt, ypt);
		SetFireChx(CurrentFire, newcount, ros, fli);
          SetReact(CurrentFire, newcount, rcx);
		++newcount;
	}
}


void StandardizePolygon::RemoveDuplicatePoints(long CurrentFire)
{
	long i, j, k, orient, NumPoints;
     double xpt, ypt, ros, negfli, fli, xptl, yptl, xptn, yptn;
     double dist, distn;
     double xlo, xhi, ylo, yhi, rcx;

	NumPoints=GetNumPoints(CurrentFire);
     if(NumPoints==0)
     	return;

     //FreePerimeter2();
     AllocPerimeter2(NumPoints+1);
     j=0;
     negfli=1.0;
	xptl=xlo=xhi=GetPerimeter1Value(CurrentFire, NumPoints-1, XCOORD);
	yptl=ylo=yhi=GetPerimeter1Value(CurrentFire, NumPoints-1, YCOORD);
     for(i=0; i<NumPoints; i++)
     {    xpt=GetPerimeter1Value(CurrentFire, i, XCOORD);
		ypt=GetPerimeter1Value(CurrentFire, i, YCOORD);
          dist=pow2(xpt-xptl)+pow2(ypt-yptl);
          k=i+1;
          if(k>NumPoints-1)
          	k-=NumPoints;
          xptn=GetPerimeter1Value(CurrentFire, k, XCOORD);
		yptn=GetPerimeter1Value(CurrentFire, k, YCOORD);
          distn=pow2(xptn-xptl)+pow2(yptn-yptl);
          if(dist>1e-12 && distn>1e-12)
          {    orient=Orientation(vec(xptl, yptl), vec(xpt, ypt), vec(xptn, yptn));
          	if(orient==0 && distn<=dist)
          	{	xpt=xptn;
               	ypt=yptn;
               	negfli=GetPerimeter1Value(CurrentFire, i, FLIVAL);
                    i++;
               }
          	ros=GetPerimeter1Value(CurrentFire, i, ROSVAL);
			fli=GetPerimeter1Value(CurrentFire, i, FLIVAL);
			rcx=GetPerimeter1Value(CurrentFire, i, RCXVAL);
               if(negfli<0.0 && fli>=0.0)               // don't remove suppressed points
               	fli=negfli;
               negfli=1.0;
          	SetPerimeter2(j, xpt, ypt, ros, fli, rcx);
               j++;
     	     xptl=xpt;
	          yptl=ypt;
               if(xpt<xlo)
               	xlo=xpt;
               if(xpt>xhi)
               	xhi=xpt;
               if(ypt<ylo)
               	ylo=ypt;
               if(ypt>yhi)
               	yhi=ypt;
          }
     }
     if(j>0 && j<NumPoints)
     {    SetPerimeter2(j, xlo, xhi, ylo, yhi, 0.0);
          SwapFirePerims(CurrentFire, -(j+1));
     	SetNumPoints(CurrentFire, j);
     }
}


void StandardizePolygon::BoundingBox(long NumFire)
{
	double xpt, ypt, Xlo, Xhi, Ylo, Yhi;
	//long NumFire=GetNewFires()-1;
	long NumPoint=GetNumPoints(NumFire);

	Xlo=Xhi=GetPerimeter1Value(NumFire, 0, 0);
	Ylo=Yhi=GetPerimeter1Value(NumFire, 0, 1);
	for(int i=1; i<NumPoint; i++)
	{	xpt=GetPerimeter1Value(NumFire, i, 0);
		ypt=GetPerimeter1Value(NumFire, i, 1);
		if(xpt<Xlo) Xlo=xpt;
		else
		{	if(xpt>Xhi)
				Xhi=xpt;
		}
		if(ypt<Ylo) Ylo=ypt;
		else
		{	if(ypt>Yhi)
			Yhi=ypt;
		}
	}
	SetPerimeter1(NumFire, NumPoint, Xlo, Xhi);
	SetFireChx(NumFire, NumPoint, Ylo, Yhi);
}


void Intersections::MergeBarrierNoCross(long *CurrentFire, long NextFire)
{// merge active fire front with vector fire barrier
	long i, NumPts, inside;
     double ros, fli;

     NumPts=GetNumPoints(*CurrentFire);
     for(i=0; i<NumPts; i++)
     {	startx=GetPerimeter1Value(*CurrentFire, i, XCOORD);
	     starty=GetPerimeter1Value(*CurrentFire, i, YCOORD);
         	fli=GetPerimeter1Value(*CurrentFire, i, FLIVAL);
          if(fli>=0.0)
          {	inside=Overlap(NextFire);
			if(inside)
			{	ros=GetPerimeter1Value(*CurrentFire, i, ROSVAL);
                    if(fli==0.0)
                    	fli=1.0;
	          	SetFireChx(*CurrentFire, i, ros, -fli);
			}
          }
     }
	//rediscretize(CurrentFire);
}


void Intersections::MergeBarrier(long *CurrentFire, long NextFire)
{// neutralize points intersecting and falling inside barrier
     long i, j, Start, End, NumPts, NumNewPoints, inside;
     double xpt, ypt, ros, fli, rcx, dist;

     if(numcross<2)
          return;

     NumPts=GetNumPoints(*CurrentFire);
     AllocPerimeter2(NumPts+numcross+1);
     Start=NumNewPoints=0;
     for(i=0; i<=numcross; i++)          // for each cross
     {    if(i<numcross)
               End=GetSpan(i, 0);         // use start of crossed span 1 as end point
          else
               End=NumPts;
          for(j=Start; j<=End; j++)       // transfer from beginning of perim1 to end
          {    startx=xpt=GetPerimeter1Value(*CurrentFire, j, XCOORD);
               starty=ypt=GetPerimeter1Value(*CurrentFire, j, YCOORD);
               ros=GetPerimeter1Value(*CurrentFire, j, ROSVAL);
               fli=GetPerimeter1Value(*CurrentFire, j, FLIVAL);
               rcx=GetPerimeter1Value(*CurrentFire, j, RCXVAL);
               if(j<NumPts && fli>=0.0)
               {    inside=Overlap(NextFire);
		         	if(inside)
                    {    if(fli>0.0)
                              fli*=-1.0;
                         if(fli==0.0)
                           	fli=-1.0;
                    }
               }
               else
               	inside=0;
               SetPerimeter2(NumNewPoints++, xpt, ypt, ros, fli, rcx);
          }
          Start=End+1;               // set new start point
          if(i<numcross)
          {    GetInterPointCoord(i, &xpt, &ypt);
               dist=pow2(xpt-startx)+pow2(ypt-starty);
               if(dist>0.1)         // don't allow duplicate points
               {   	GetInterPointFChx(i, &ros, &fli, &rcx);
                    if(fli>0.0)
                         fli*=-1.0;
                    else if(fli==0.0)
                         fli=-1.0;
                   	SetPerimeter2(NumNewPoints++, xpt, ypt, ros, fli, rcx);
                    startx=xpt;    // in case of multiple crosses on same span
                    starty=ypt;
               }
          }
     }
     if(NumNewPoints>(NumPts+1))
     {    FreePerimeter1(*CurrentFire);
          AllocPerimeter1(*CurrentFire, NumNewPoints);
     }
     tranz(*CurrentFire, NumNewPoints);
     SetNumPoints(*CurrentFire, NumNewPoints-1);
}

/*
void Intersections::MergeBarrier(long *CurrentFire, long NextFire)
{// neutralize points intersecting and falling inside barrier
     long i, j, Start, End, NumPts, NumNewPoints, inside;
     double xpt, ypt, ros, fli, rcx, dist;

     if(numcross<2)
          return;

     NumPts=GetNumPoints(*CurrentFire);
     AllocPerimeter2(NumPts+numcross+1);
     Start=NumNewPoints=0;
     for(i=0; i<=numcross; i++)
     {    if(i<numcross)
	     	End=GetSpan(i, 0);
     	else
          	End=NumPts-1;
		for(j=Start; j<=End; j++)
		{	xpt=GetPerimeter1Value(*CurrentFire, j, XCOORD);
	          ypt=GetPerimeter1Value(*CurrentFire, j, YCOORD);
     	     ros=GetPerimeter1Value(*CurrentFire, j, ROSVAL);
          	fli=GetPerimeter1Value(*CurrentFire, j, FLIVAL);
	          rcx=GetPerimeter1Value(*CurrentFire, j, RCXVAL);
     	    	SetPerimeter2(NumNewPoints++, xpt, ypt, ros, fli, rcx);
          }
          Start=End+1;
          if(Start>NumPts-1)
          	Start=NumPts-1;
          if(i<numcross)
          {	GetInterPointCoord(i, &xpt, &ypt);
	          dist=pow2(xpt-startx)+pow2(ypt-starty);
     	     if(dist>0.1)         // don't allow duplicate points
           	{    GetInterPointFChx(i, &ros, &fli, &rcx);
                    if(fli>0.0)
                         fli*=-1.0;
                    else if(fli==0.0)
                         fli=-1.0;
	     	    	SetPerimeter2(NumNewPoints++, xpt, ypt, ros, fli, rcx);
     		}
		}
     }
     j=NumPts;
	xpt=GetPerimeter1Value(*CurrentFire, j, XCOORD);
	ypt=GetPerimeter1Value(*CurrentFire, j, YCOORD);
     ros=GetPerimeter1Value(*CurrentFire, j, ROSVAL);
     fli=GetPerimeter1Value(*CurrentFire, j, FLIVAL);
     SetPerimeter2(NumNewPoints, xpt, ypt, ros, fli, 0.0);
     if(NumNewPoints>NumPts)
     {    FreePerimeter1(*CurrentFire);
          AllocPerimeter1(*CurrentFire, NumNewPoints+1);
     }
     tranz(*CurrentFire, NumNewPoints+1);
     SetNumPoints(*CurrentFire, NumNewPoints);
     NumPts=NumNewPoints;

	for(i=0; i<NumPts; i++)
	{	startx=GetPerimeter1Value(*CurrentFire, i, XCOORD);
	     starty=GetPerimeter1Value(*CurrentFire, i, YCOORD);
    		inside=Overlap(NextFire);
          if(inside)
          {    ros=GetPerimeter1Value(*CurrentFire, i, ROSVAL);
          	fli=GetPerimeter1Value(*CurrentFire, i, FLIVAL);
	          if(fli>0.0)
          	     fli*=-1.0;
               if(fli==0.0)
                  	fli=-1.0;
               SetFireChx(*CurrentFire, i, ros, fli);
          }
     }
}
*/

void Intersections::OrganizeIntersections(long CurrentFire)
{
	bool Reverse;//, Continue;
     long i, j, k, m, n;
     long samechek1, samechek2;
	double x1, y1, x2, y2, rcx1, rcx2;
     double xpt, ypt, ros1, fli1, ros2, fli2, diff1, diff2;

     for(i=0; i<numcross; i++)
     {	samechek1=GetSpan(i, 0);
          j=i+1;
          while(j<numcross)
          {    samechek2=GetSpan(j, 0);
          	if(samechek2!=samechek1)
               	break;
          	j++;
          }
          if(j>=numcross && samechek2!=samechek1)
			break;
          if(j-i==1)
          	continue;

         	xpt=GetPerimeter1Value(CurrentFire, samechek1, XCOORD);
	    	ypt=GetPerimeter1Value(CurrentFire, samechek1, YCOORD);
          do
         	{    Reverse=false;
         		for(k=i; k<j; k++)
	          {	GetInterPointCoord(k, &x1, &y1);//ipoints[k*4];
     	    	     diff1=pow2(xpt-x1)+pow2(ypt-y1);
                    for(m=k+1; m<j; m++)
                    {    GetInterPointCoord(m, &x2, &y2);
     		          diff2=pow2(xpt-x2)+pow2(ypt-y2);
                         if(diff2<diff1)
                         {	Reverse=true;
						GetInterPointFChx(m, &ros2, &fli2, &rcx2);
						GetInterPointFChx(k, &ros1, &fli1, &rcx1);
						SetInterPoint(k, x2, y2, ros2, fli2, rcx2);
						SetInterPoint(m, x1, y1, ros1, fli1, rcx1);
						GetIntersection(m, &n, &samechek2);
						GetIntersection(k, &n, &samechek1);
						SetIntersection(m, n, samechek1);
						SetIntersection(k, n, samechek2);
                              k=j+1;	// force break out of outer for statement
                              break;
                         }
                    }
          	}
          } while(Reverse);
          i=j-1;
     }
}


/*
void Intersections::OrganizeIntersections(long CurrentFire)
{// reorders intersections and interpoints for intersections between fires
 // to order they would be encountered
     long inc, inc1, inc2, samechek1, samechek2, xchek;
     double xpt, ypt, ros1, fli1, ros2, fli2, xdiff, ydiff;
     double samepoint1x=0, samepoint1y=0, samepoint2x=0, samepoint2y=0;
	double dist1, dist2;

     for(inc=0; inc<numcross; inc++)
	{    samechek1=GetSpan(inc, 0);    // reverses order of spike intersections where two fires merge
		inc2=inc+1;                   // and form internal fires
          if(inc2>numcross-1)
              	break;
  		samechek2=GetSpan(inc2, 0);
  		if(samechek2==samechek1)
  		{    GetInterPointCoord(inc, &samepoint1x, &samepoint1y);
  			GetInterPointCoord(inc2, &samepoint2x, &samepoint2y);
  			xpt=GetPerimeter1Value(CurrentFire, samechek1, XCOORD);
  			ypt=GetPerimeter1Value(CurrentFire, samechek1, YCOORD);
  			xdiff=pow2(xpt-samepoint1x);
  			ydiff=pow2(ypt-samepoint1y);
  			dist1=xdiff+ydiff;
  			xdiff=pow2(xpt-samepoint2x);
  			ydiff=pow2(ypt-samepoint2y);
  			dist2=xdiff+ydiff;
  			if(dist1>dist2)
  			{	do
  				{    if(++inc2<numcross)
  						samechek2=GetSpan(inc2, 0);
  					else
                             	break;
     			} while(samechek2==samechek1);
				inc1=inc;	    	// start switching here, end switching at inc2-1
				inc=inc2-1;   	// go on to next intersection in for loop
				while(inc2>inc1)
				{    inc2--;
					GetIntersection(inc2, &xchek, &samechek2);		// reorder addresses and intersection points
					GetIntersection(inc1, &xchek, &samechek1);
					GetInterPointCoord(inc2, &samepoint2x, &samepoint2y);
					GetInterPointCoord(inc1, &samepoint1x, &samepoint1y);
					GetInterPointFChx(inc2, &ros2, &fli2);
					GetInterPointFChx(inc1, &ros1, &fli1);
					SetIntersection(inc1, xchek, samechek2);
					SetIntersection(inc2, xchek, samechek1);
					SetInterPoint(inc1, samepoint2x, samepoint2y, ros2, fli2);
					SetInterPoint(inc2, samepoint1x, samepoint1y, ros1, fli1);
					inc1++;
				}
			}
		}
	}
}
*/

void Intersections::OrganizeCrosses(long CurrentFire)
{// reorders intersections and adds points between multiple intersections on a span
	long StartCheck, NextCheck, SwitchCheck1, SwitchCheck2;
	long Isect1a, Isect1b, Isect2a, Isect2b;
	long NumNewPoints=0, NumPointsWritten=0, NextPoint;
	bool CheckFound=false, CrossFound=false;
	double point1x, point2x, point1y, point2y;
	double xpt1, ypt1, ros1, fli1, rcx1;
	double xpt2, ypt2, ros2, fli2, rcx2;
	double xpt3, ypt3, ros3, fli3, rcx3;
	double xdiff, ydiff, dist1, dist2;

	for(StartCheck=0; StartCheck<numcross; StartCheck++)
	{    NextCheck=StartCheck;
		GetIntersection(StartCheck, &Isect1a, &Isect1b);
		GetIntersection(++NextCheck, &Isect2a, &Isect2b);
		while(Isect1a==Isect2a)
		{	GetIntersection(++NextCheck, &Isect2a, &Isect2b);
			CheckFound=true;
			CrossFound=true;
		};
		if(CheckFound)
		{    CheckFound=false;
			NextCheck--;
			xpt1=GetPerimeter1Value(CurrentFire, Isect1a, XCOORD);
			ypt1=GetPerimeter1Value(CurrentFire, Isect1a, YCOORD);
			for(SwitchCheck1=StartCheck; SwitchCheck1<NextCheck; SwitchCheck1++)
			{    GetIntersection(SwitchCheck1, &Isect1a, &Isect1b);
				GetInterPointCoord(SwitchCheck1, &point1x, &point1y);
				xdiff=pow2(xpt1-point1x);
				ydiff=pow2(ypt1-point1y);
				dist1=xdiff+ydiff;
				for(SwitchCheck2=SwitchCheck1+1; SwitchCheck2<=NextCheck; SwitchCheck2++)
				{	GetIntersection(SwitchCheck2, &Isect2a, &Isect2b);
					GetInterPointCoord(SwitchCheck2, &point2x, &point2y);
					xdiff=pow2(xpt1-point2x);
					ydiff=pow2(ypt1-point2y);
					dist2=xdiff+ydiff;
					if(dist2<dist1)
					{    dist1=dist2;
						SetIntersection(SwitchCheck1, Isect2a, Isect2b);
						GetInterPointFChx(SwitchCheck2, &ros2, &fli2, &rcx2);
						SetInterPoint(SwitchCheck1, point2x, point2y, ros2, fli2, rcx2);
						SetIntersection(SwitchCheck2, Isect1a, Isect1b);
						GetInterPointFChx(SwitchCheck1, &ros2, &fli2, &rcx2);
						SetInterPoint(SwitchCheck2, point1x, point1y, ros2, fli2, rcx2);
						Isect1a=Isect2a;
						Isect1b=Isect2b;
						point1x=point2x;
						point1y=point2y;
					}
				}
			}
			StartCheck=NextCheck;
		}
	}
	if(CrossFound)
	{	//FreePerimeter2();
		AllocPerimeter2(GetNumPoints(CurrentFire) + 2*numcross);
		for(StartCheck=0; StartCheck<numcross; StartCheck++)
		{	NextCheck=StartCheck+1;
			if(NextCheck>numcross-1)
				NextCheck=0;
			GetIntersection(StartCheck, &Isect1a, &Isect1b);
			GetIntersection(NextCheck, &Isect2a, &Isect2b);
			for(NextPoint=NumPointsWritten; NextPoint<=Isect2a; NextPoint++)
			{	xpt1=GetPerimeter1Value(CurrentFire, NextPoint, XCOORD);
				ypt1=GetPerimeter1Value(CurrentFire, NextPoint, YCOORD);
				ros1=GetPerimeter1Value(CurrentFire, NextPoint, ROSVAL);
				fli1=GetPerimeter1Value(CurrentFire, NextPoint, FLIVAL);
                    rcx1=GetPerimeter1Value(CurrentFire, NextPoint, RCXVAL);
				SetPerimeter2(NextPoint+NumNewPoints, xpt1, ypt1, ros1, fli1, rcx1);
			}
			NumPointsWritten=NextPoint;
			if(Isect1a==Isect2a)
			{	GetInterPointCoord(StartCheck, &xpt1, &ypt1);
				GetInterPointFChx(StartCheck, &ros1, &fli1, &rcx1);
				GetInterPointCoord(NextCheck, &xpt2, &ypt2);
				GetInterPointFChx(NextCheck, &ros2, &fli2, &rcx2);
				xpt3=xpt2-(xpt2-xpt1)/2.0;
				ypt3=ypt2-(ypt2-ypt1)/2.0;
				dist1=sqrt(pow2(xpt2-xpt1)+pow2(ypt2-ypt1));
				if(dist1>1e-8)
				{	ros3=(ros1+ros2)/2.0;
					fli3=(fabs(fli1)+fabs(fli2))/2.0;
                         rcx3=(rcx1+rcx2)/2.0;
					if(fli1<0.0 || fli2<0.0)
						fli3*=-1.0;
					SetPerimeter2(NumPointsWritten+NumNewPoints, xpt3, ypt3, ros3, fli3, rcx3);
					NumNewPoints++;
				}
				else
				{    dist1+=1;
					xpt3+=dist1;
				}
			}
		}
		for(NextPoint=NumPointsWritten; NextPoint<=GetNumPoints(CurrentFire); NextPoint++)
		{	xpt1=GetPerimeter1Value(CurrentFire, NextPoint, XCOORD);
			ypt1=GetPerimeter1Value(CurrentFire, NextPoint, YCOORD);
			ros1=GetPerimeter1Value(CurrentFire, NextPoint, ROSVAL);
			fli1=GetPerimeter1Value(CurrentFire, NextPoint, FLIVAL);
			rcx1=GetPerimeter1Value(CurrentFire, NextPoint, FLIVAL);
			SetPerimeter2(NextPoint+NumNewPoints, xpt1, ypt1, ros1, fli1, rcx1);
		}
		NumPointsWritten=NextPoint;
		FreePerimeter1(CurrentFire);
		AllocPerimeter1(CurrentFire, NumPointsWritten+NumNewPoints);
		tranz(CurrentFire, NumPointsWritten+NumNewPoints);
		SetNumPoints(CurrentFire, NumPointsWritten+NumNewPoints-1);
	}
}


bool Intersections::MergeFire(long *CurrentFire, long NextFire)
{// clips loops and merges fires, finds and stores enclaves
	long xsect1=0, xsect2=0, xsect3=0, xsect4=0, xsect5, xsect6, xsect7, xsect8;
	long inc=0, outcross=0, offcheck=0;
	long fxend=0, bxend=0, Firstin=0, Secondin=0;
	long numchek=0, chekct=0, newnump=0, crosstype=0;
	long freadstart=0, freadend, breadstart=0, breadend=0;
	long forewrite=0, backwrite=0, xwrite=0;
	long *PostFires;
	double diff1, diff2; //, diffshort=0;

	noffset1=0; noffset2=0;
	if(*CurrentFire==NextFire)			// if merging loops on one fire
	{    /*------------------------------------------------------------------//
		// ##### Eliminated because based on point density 12/23/1994 #####
		if(GetInout(*CurrentFire)==1)			// for outward fires only
		{	diff2=GetNumPoints(*CurrentFire)/2;
			for(numchek=0;numchek<numcross;numchek++)
			{    GetIntersection(numchek, &xsect1, &xsect2);
				diff1=xsect2-xsect1;
				if(diff1>diff2)
				{ 	if(numchek!=0)
					{	GetIntersection(numchek-1, &xsect3, &xsect4);
						diff1=xsect4-xsect3;
						if(xsect1==xsect3)
						{	if(diff1>0 && diff1<diff2)
								noffset1=xsect4+1;
							else
								noffset1=xsect1+1;
						}
						else
							noffset1=xsect1+1;
					}
					else
						noffset1=xsect1+1;  // offsets for array addresses
					noffset2=GetNumPoints(*CurrentFire);
					noffset2=noffset2-noffset1;
					GetIntersection(++numchek, &xsect3, &xsect4);
					if(xsect3==xsect1)
					{	while(xsect3==xsect1)
						{ 	GetIntersection(++inc+numchek, &xsect3, &xsect6);
						}
					}
					else
					{	if(xsect4==xsect2)
						{	while(xsect4==xsect2)
							{//	xsect4=intersect[++inc+numchek][1];
								GetIntersection(++inc+numchek, &xsect5, &xsect4);
							}
						}
					}
					outcross=inc+numchek; // offset for numchek
					inc=0;
					numchek--;
				}
			}
		}
		--------------------------------------------------------------------*/
		long NumExistingFires=GetNewFires();
		numchek=0;
		AllocSwap(GetNumPoints(*CurrentFire));
		while(numchek<numcross)        	// for all line crosses
		{    offcheck=(numchek+outcross);
			GetOffcheck(&offcheck);
			xsect1=intercode(offcheck,0);
			xsect2=intercode(offcheck,1);  // =0 the first time through by default
			fxend=offcheck;
			offcheck=(++numchek+outcross);
			GetOffcheck(&offcheck);
			xsect3=intercode(offcheck,0);
			xsect4=intercode(offcheck,1);
			if(xsect3!=xsect2)                     		// if two crosses in a row
			{	xsect5=xsect3;
				xsect6=xsect4;
				if(xsect5!=xsect1 && xsect6!=xsect2)	// if only one cross on 1-2 span
				{    xsect7=xsect1;
					xsect8=xsect2;
					while(xsect5<xsect8)
					{	offcheck=(++numchek+outcross);
						GetOffcheck(&offcheck);
						xsect5=intercode(offcheck,0);
						xsect6=intercode(offcheck,1);
						if(xsect8<=xsect6)
						{    xsect7=xsect5;
							xsect8=xsect6;
						}
					}
					while(xsect6!=xsect7)
					{	offcheck=(++numchek+outcross);
						GetOffcheck(&offcheck);
						xsect6=intercode(offcheck,1);
					}
					if(xsect5==xsect2 && xsect6==xsect1)
					{	offcheck=(numchek+outcross-1);   // BACKS UP ONE IN INTERSECT[] TO ELIMINATE SINGLE CROSS-OVERS
						GetOffcheck(&offcheck);
						xsect7=intercode(offcheck,0);
						xsect8=intercode(offcheck,1);
						if(xsect7>xsect4)
							xsect4=xsect7;		//  TEST 12/29/1994
						if(xsect7==xsect4 && xsect8==xsect3)	// IF DOUBLE LOOP WITH INTERNAL FIRE
						{	if(xsect4-xsect3>2)	// if internal fire is > minimum point number 10
							{    backwrite=0;
								breadstart=xsect3+1;
								bxend=offcheck;
								breadend=xsect4;
								xwrite=backwrite;
								readnum=*CurrentFire;
								writenum=GetNewFires();
								MergeWrite(bxend, breadstart, breadend, &xwrite);
								backwrite=xwrite;
								AllocPerimeter1(GetNewFires(), xwrite+1);
								SetInout(GetNewFires(),2);
								SetNumPoints(GetNewFires(), xwrite);
								SwapTranz(GetNewFires(), xwrite);
								BoundaryBox(xwrite);
								IncNewFires(1);
							}
						}
						else
						{	fxend=-1;
						}
					}
					else
					{	fxend=-1;
					}
					freadend=xsect1;
					xwrite=forewrite;
					readnum=*CurrentFire;
					writenum=NextFire;
					MergeWrite(fxend, freadstart, freadend, &xwrite);
					forewrite=xwrite;
					offcheck=(1+numchek+outcross);
					GetOffcheck(&offcheck);
					xsect7=intercode(offcheck,0);
					if(xsect7!=xsect5)
						freadstart=xsect5+1;
					else
						freadstart=-1;
				}
				else			// more than one cross on 1-2 span
				{	if(xsect5==xsect1)  // if spike loop
					{	while(xsect3==xsect1)
						{    offcheck=(++numchek+outcross);
							GetOffcheck(&offcheck);
							xsect3=intercode(offcheck,0);
						}
						offcheck=(--numchek+outcross);
						GetOffcheck(&offcheck);
						xsect3=intercode(offcheck,0);
						xsect4=intercode(offcheck,1);
						fxend=offcheck;
						xsect5=xsect3;
						xsect7=xsect3;
						xsect6=xsect4;
						xsect8=xsect4;
						while(xsect5<xsect8)	// traps reverse spike
						{    offcheck=(++numchek+outcross);
							GetOffcheck(&offcheck);
							xsect5=intercode(offcheck,0);
							xsect6=intercode(offcheck,1);
							if(xsect8<=xsect6)
							{	xsect8=xsect6;
								xsect7=xsect5;
							}
						}
						while(xsect6!=xsect7)
						{	offcheck=(++numchek+outcross);
							GetOffcheck(&offcheck);
							xsect6=intercode(offcheck,1);
						}
						/*if(xsect5>xsect4)
						{	xsect4=xsect5;		// TEST 12/29/1994
							inc=-1;
						}*/
						if(xsect5==xsect4 && xsect6==xsect3)
						{	if(xsect4-xsect3>2)	// if internal fire is > minimum point number 10
							{	backwrite=0;
								breadstart=xsect3+1+inc;  // +inc is TEST
								bxend=offcheck;
								breadend=xsect4;
								xwrite=backwrite;
								readnum=*CurrentFire;
								writenum=GetNewFires();
								MergeWrite(bxend, breadstart, breadend, &xwrite);
								backwrite=xwrite;
								AllocPerimeter1(GetNewFires(), xwrite+1);
								SetInout(GetNewFires(),2);
								SetNumPoints(GetNewFires(), xwrite);
								SwapTranz(GetNewFires(), xwrite);
								BoundaryBox(xwrite);
								IncNewFires(1);
								inc=0;				// inc is TEST
							}
						}
						else
							fxend=-1;
						freadend=xsect1;
						xwrite=forewrite;
						readnum=*CurrentFire;
						writenum=NextFire;
						MergeWrite(fxend, freadstart, freadend, &xwrite);
						forewrite=xwrite;
						offcheck=(1+numchek+outcross);
						GetOffcheck(&offcheck);
						xsect5=intercode(offcheck,0);
						if(xsect5!=xsect8)
							freadstart=xsect8+1;
						else
							freadstart=-1;
					}
					else   // spike starts on 1-2 span, "alternate spike loop"
					{    while(xsect6==xsect2)
						{	offcheck=(++numchek+outcross);
							GetOffcheck(&offcheck);
							xsect6=intercode(offcheck,1);
						}
						offcheck=(--numchek+outcross);
						GetOffcheck(&offcheck);
						xsect5=intercode(offcheck,0);
						xsect6=intercode(offcheck,1);
						xsect7=xsect5;
						xsect8=xsect6;
						while(xsect5<xsect8)
						{	offcheck=(++numchek+outcross);
							GetOffcheck(&offcheck);
							xsect5=intercode(offcheck,0);
							xsect6=intercode(offcheck,1);
							if(xsect8<=xsect6)
							{	xsect8=xsect6;
								xsect7=xsect5;
							}
						}
						while(xsect6!=xsect7)
						{	offcheck=(++numchek+outcross);
							GetOffcheck(&offcheck);
							xsect6=intercode(offcheck,1);
						}
					    /*	if(xsect5>xsect4)
						{	xsect4=xsect5;		// TEST 12/29/1994
							inc=-1;
						}*/
						if(xsect5==xsect4 && xsect6==xsect3)
						{	if(xsect4-xsect3>2)	// if internal fire is > minimum point number 20
							{	backwrite=0;
								breadstart=xsect3+1+inc;	// +inc is TEST
								bxend=offcheck;
								breadend=xsect4;
								xwrite=backwrite;
								readnum=*CurrentFire;
								writenum=GetNewFires();
								MergeWrite(bxend, breadstart, breadend, &xwrite);
								backwrite=xwrite;
								AllocPerimeter1(GetNewFires(), xwrite+1);
								SetInout(GetNewFires(),2);
								SetNumPoints(GetNewFires(), xwrite);
								SwapTranz(GetNewFires(), xwrite);
								BoundaryBox(xwrite);
								IncNewFires(1);
								inc=0;				// inc is TEST
							}
						}
						else
							fxend=-1;
						freadend=xsect1;
						xwrite=forewrite;
						readnum=*CurrentFire;
						writenum=NextFire;
						MergeWrite(fxend, freadstart, freadend, &xwrite);
						forewrite=xwrite;
						offcheck=(1+numchek+outcross);
						GetOffcheck(&offcheck);
						xsect7=intercode(offcheck,0);
						if(xsect7!=xsect5)
							freadstart=xsect8+1;
						else
							freadstart=-1;
					}
				}
				offcheck=(++numchek+outcross);
				GetOffcheck(&offcheck);
			}
			else				    // clip single loop
			{    freadend=xsect1;   //freadstart=0 by default or other freadstart
				xwrite=forewrite;
				readnum=*CurrentFire;
				writenum=NextFire;
				MergeWrite(fxend, freadstart, freadend, &xwrite);
				forewrite=xwrite;
				offcheck=(++numchek+outcross);
				GetOffcheck(&offcheck);
				xsect5=intercode(offcheck,0);
				if(offcheck!=numcross && xsect5!=xsect3)    // no twin spike loop on xsect3 span
					freadstart=xsect3+1;
				else
					freadstart=-1;
			}
		}
		freadend=GetNumPoints(*CurrentFire)-1;
		fxend=-1;
		xwrite=forewrite;
		readnum=*CurrentFire;
		writenum=NextFire;
		MergeWrite(fxend, freadstart, freadend, &xwrite);
		forewrite=xwrite;
		offcheck=(++numchek+outcross);
		GetOffcheck(&offcheck);
		OldNumPoints=GetNumPoints(*CurrentFire);	// store old number of points for rediscretize
		if(GetInout(*CurrentFire)==2)
		{	SetNumPoints(*CurrentFire, forewrite);
			//FreeSwap();                    		// must go before rediscretize which also uses swapperim
			rediscretize(CurrentFire, true);
		}
		else
		{	if(forewrite>8 || forewrite>((double) OldNumPoints)/2.0) // normal exit from MergeFire
			{    SetNumPoints(*CurrentFire, forewrite);
				//FreeSwap();                    		// must go before rediscretize which also uses swapperim
				rediscretize(CurrentFire, true);
			}
			else	// reset new inward fires
			{    for(numchek=NumExistingFires; numchek<GetNewFires(); numchek++)
				{	FreePerimeter1(numchek);
					SetNumPoints(numchek, 0);
					SetInout(numchek, 0);
				}
				//-----------------------------------------------------
				//----- New Sequence Finds Only OuterPerim
				FindOuterFirePerimeter(*CurrentFire);
				//FreeSwap();                    		// must go before rediscretize which also uses swapperim
				rediscretize(CurrentFire, false);
				//-----------------------------------------------------

				//-----------------------------------------------------
				//----- Old Sequence Leaves Fire In WITHOUT LOOP CLIPPING
				//FreeSwap();
				//SetNewFires(NumExistingFires);
				//-----------------------------------------------------

				return false;
			}
		}
	}                        // if merging two different fires
	else	if(numcross==2)	// only two intersections between fires, no internal fires
	{    writenum=*CurrentFire;
		//FreePerimeter2();
		AllocPerimeter2(2*GetNumPoints(*CurrentFire)+2*GetNumPoints(NextFire));// be safe and allocate enough for both arrays
		GetIntersection(numchek, &xsect1, &xsect2);
		GetIntersection(++numchek, &xsect3, &xsect4);
		startx=GetPerimeter1Value(*CurrentFire, 0, XCOORD);
		starty=GetPerimeter1Value(*CurrentFire, 0, YCOORD);
		Firstin=Overlap(NextFire);
		startx=GetPerimeter1Value(NextFire, 0, XCOORD);
		starty=GetPerimeter1Value(NextFire, 0, YCOORD);
		Secondin=Overlap(*CurrentFire);
		if(GetInout(*CurrentFire)==2)
		{	if(Secondin) Secondin=0;
			else	Secondin=1;
		}
		else	if(GetInout(NextFire)==2)
		{    if(Firstin) Firstin=0;
			else	Firstin=1;
		}
		if(!Firstin)                            // origin of 1st fire is not inside
		{	if(!Secondin)    				// origin of 2nd fire is not inside
				crosstype=1;
			else
				crosstype=2;				// 2nd only
		}
		else
		{	if(!Secondin)
				crosstype=3;                  // 1st only
			else
				crosstype=4;				// both origins within overlap
		}
		switch(crosstype)
		{ 	case 1:	xwrite=forewrite;        // if both origins NOT within overlap
					freadstart=0;
					freadend=xsect1;
					fxend=numchek-1;
					readnum=*CurrentFire;
					MergeWrite(fxend, freadstart, freadend, &xwrite);
					freadstart=xsect2+1;
					freadend=GetNumPoints(NextFire)-1;
					fxend=-1;
					readnum=NextFire;
					MergeWrite(fxend, freadstart, freadend, &xwrite);
					freadstart=0;
					freadend=xsect4;
					fxend=numchek;
					readnum=NextFire;
					MergeWrite(fxend, freadstart, freadend, &xwrite);
					freadstart=xsect3+1;
					freadend=GetNumPoints(*CurrentFire)-1;
					fxend=-1;
					readnum=*CurrentFire;
					MergeWrite(fxend, freadstart, freadend, &xwrite);
					forewrite=xwrite;
					break;
			case 2:   xwrite=forewrite=0;			// if only origin on fire2 is within overlap
					freadstart=0;
					freadend=xsect1;
					fxend=numchek-1;
					readnum=*CurrentFire;
					MergeWrite(fxend, freadstart, freadend, &xwrite);
					freadstart=xsect2+1;
					freadend=xsect4;
					fxend=numchek;
					readnum=NextFire;
					MergeWrite(fxend, freadstart, freadend, &xwrite);
					freadstart=xsect3+1;
					freadend=GetNumPoints(*CurrentFire)-1;
					fxend=-1;
					readnum=*CurrentFire;
					MergeWrite(fxend, freadstart, freadend, &xwrite);
					forewrite=xwrite;
					break;
			case 3:	xwrite=forewrite;			// if only origin on fire1 is within overlap
					freadstart=xsect1+1;
					freadend=xsect3;
					fxend=numchek;
					readnum=*CurrentFire;
					MergeWrite(fxend, freadstart, freadend, &xwrite);
					freadstart=xsect4+1;
					freadend=GetNumPoints(NextFire)-1;
					fxend=-1;
					readnum=NextFire;
					MergeWrite(fxend, freadstart, freadend, &xwrite);
					freadstart=0;
					freadend=xsect2;
					fxend=numchek-1;
					readnum=NextFire;
					MergeWrite(fxend, freadstart, freadend, &xwrite);
					forewrite=xwrite;
					break;
			case 4:	xwrite=forewrite;             // if both fire origins within overlap
					freadstart=xsect1+1;
					freadend=xsect3;
					fxend=numchek;
					readnum=*CurrentFire;
					MergeWrite(fxend, freadstart, freadend, &xwrite);
					freadstart=xsect4+1;
					freadend=xsect2;                     
					fxend=numchek-1;
					readnum=NextFire;
					MergeWrite(fxend, freadstart, freadend, &xwrite);
					forewrite=xwrite;
					break;
		}
          if(CheckPostFrontal(GETVAL))       // for
          {	PostFires=new long[2];
          	PostFires[0]=*CurrentFire;
               PostFires[1]=NextFire;
          	post.MergeFireRings(PostFires, 2, intersect, interpoint, numcross, forewrite);
               delete[] PostFires;
          }

		OldNumPoints=GetNumPoints(*CurrentFire);	// store old number of points for rediscretize
		SetNumPoints(*CurrentFire, forewrite);
		if(GetInout(*CurrentFire)==1 && GetInout(NextFire)==1)
			SetInout(*CurrentFire,1);
		else
			SetInout(*CurrentFire,2);
		SetInout(NextFire,0);
		FreePerimeter1(NextFire);
		IncSkipFires(1);
		SetNumPoints(NextFire, 0);
          //if(GetNumAttacks()>0)
     	//	SetNewFireNumberForAttack(NextFire, *CurrentFire);
          //if(GetNumAirAttacks()>0)
     	//	SetNewFireNumberForAirAttack(NextFire, *CurrentFire);

          NextFire=*CurrentFire;
		rediscretize(CurrentFire, true);
          if(CheckPostFrontal(GETVAL) && NextFire!=*CurrentFire)
          	SetNewFireNumber(NextFire, *CurrentFire, post.AccessReferenceRingNum(1, GETVAL));

	}
	else   // merging 2 fires with internal loops formed
	{    long SpanStart, SpanEnd, SpanNext, trychek, Target;
		double XStart, YStart, XTest, YTest, XTarget, YTarget, NXTarget, NYTarget;
		double DistToNext=0, DistToLast=0, NDistToLast=0, diff3=0, diff4=0, area;
		long readfire=0, inside=0, outward=0, ctfire1=0, ctfire2=0;
		long endloop=-1;
		long P2NumAlloc=GetNumPoints(*CurrentFire)+GetNumPoints(NextFire)+numcross;
		long SwapNumAlloc=GetNumPoints(*CurrentFire)+GetNumPoints(NextFire);
          long i, NewFires1, NewFires2;

		NewFires1=GetNewFires();      // bookkeeping for postfrontal stuff
		AllocCrossout(numcross);		// array for holding status of crosspoints 1=used
		//FreePerimeter2();			// must GlobalFree perim2 because tranz not called
		AllocPerimeter2(P2NumAlloc);  // allocate enough memory for combined number of points
		AllocSwap(SwapNumAlloc);		// allocate enough memory for combined number of points
		startx=GetPerimeter1Value(*CurrentFire, 0, XCOORD);
		starty=GetPerimeter1Value(*CurrentFire, 0, YCOORD);
		inside=Overlap(NextFire);
		ctfire2=*CurrentFire;
		if(GetInout(NextFire)==2)
		{	if(inside) inside=0;
			else	inside=1;
		}
		if(!inside)              // if first point on fire1 is within 2nd fire
		{    SpanEnd=GetSpan(numchek, 0);
			freadstart=0; freadend=SpanEnd; fxend=numchek; ctfire1=*CurrentFire;			// fxend=numchek;
			readnum=*CurrentFire; writenum=ctfire2;
			MergeWrite(fxend, freadstart, freadend, &xwrite);
			SetCrossout(numchek, 1);
			chekct++;
		}
		else				  // first point not within 2nd fire
		{	readfire=1;
			endloop=0;
		}
		while(chekct<numcross)
		{    do
			{	readfire=abs(readfire-1);
				if(readfire)
					ctfire1=NextFire;                // SWITCH FIRE PERIMETER TO READ FROM
				else
					ctfire1=*CurrentFire;
				if(readfire && fxend==-1)           // IF READING FIRE 2 FROM ORIGIN OF ARRAY
					SpanStart=-1;
				else
				{	SpanStart=GetSpan(numchek, readfire);
					GetInterPointCoord(numchek, &XStart, &YStart);
					if(SpanStart<GetNumPoints(ctfire1)-1)
						Target=SpanStart+1;							// Target is the next point on fire
					else
						Target=0;
					XTarget=GetPerimeter1Value(ctfire1, Target, XCOORD);
					YTarget=GetPerimeter1Value(ctfire1, Target, YCOORD);
					diff1=pow2(XStart-XTarget);
					diff2=pow2(YStart-YTarget);
					DistToNext=DistToLast=sqrt(diff1+diff2);
				}
				SpanEnd=2500000;
				trychek=0;
				do
				{    if(trychek!=numchek)
					{	if(GetCrossout(trychek)==0)					// IF NEXT INTERSECT HASN'T ALREADY BEEN USED
						{	SpanNext=GetSpan(trychek, readfire);
							//if(Target==0 && SpanNext==0)			// IF READING FROM LAST POINT IN ARRAY AND CROSS IS AT ZERO
							//	SpanStart=-1;
							if(SpanNext>=SpanStart && SpanNext<=SpanEnd)
							{	if(SpanNext==SpanStart)		// IF NEXT INTERSECT IS >= TO CURRENT INTERSECT
								{    GetInterPointCoord(trychek, &XTest, &YTest);
									diff1=pow2(XTest-XTarget);
									diff2=pow2(YTest-YTarget);
									diff1=sqrt(diff1+diff2);
									diff3=pow2(XTest-XStart);
									diff4=pow2(YTest-YStart);
									diff2=sqrt(diff3+diff4);
									if(diff1<=DistToNext && diff2<=DistToLast)
									{	SpanEnd=SpanNext;
										numchek=trychek;
										DistToLast=diff2;
									}
								}
								else	if(SpanNext==SpanEnd)	// && NDistToLast!=-1
								{	GetInterPointCoord(trychek, &XTest, &YTest);
									NXTarget=GetPerimeter1Value(ctfire1, SpanNext, XCOORD);
									NYTarget=GetPerimeter1Value(ctfire1, SpanNext, YCOORD);
									diff1=pow2(XTest-NXTarget);
									diff2=pow2(YTest-NYTarget);
									diff1=sqrt(diff1+diff2);
									if(diff1<=NDistToLast)
									{	SpanEnd=SpanNext;
										numchek=trychek;
										NDistToLast=diff1;
									}
								}
								else
								{	SpanEnd=SpanNext;
									numchek=trychek;
									GetInterPointCoord(trychek, &XTest, &YTest);
									NXTarget=GetPerimeter1Value(ctfire1, SpanNext, XCOORD);
									NYTarget=GetPerimeter1Value(ctfire1, SpanNext, YCOORD);
									diff1=pow2(XTest-NXTarget);
									diff2=pow2(YTest-NYTarget);
									NDistToLast=sqrt(diff1+diff2);
								}
							}
						}
					}
					trychek++;
				} while(trychek<numcross);
				if(SpanEnd==2500000)  			// if no match for above spanend
				{	SpanEnd=GetNumPoints(ctfire1)-1;
					fxend=-1;
					readfire=abs(readfire-1);				// DON'T SWITCH FIRES YET
				}
				else
					fxend=numchek;
				freadstart=SpanStart+1;
				freadend=SpanEnd;
				readnum=ctfire1;
				writenum=ctfire2;
				if(writenum==*CurrentFire)
				{	if(xwrite+freadend-freadstart<P2NumAlloc)
						MergeWrite(fxend, freadstart, freadend, &xwrite);
					else
					{    /*FindOuterFirePerimeter(*CurrentFire);
						NumPts=GetNumPoints(*CurrentFire);
						Xlo=GetPerimeter1Value(*CurrentFire, NumPts, 0);	// use OldNumPoints from perim1[count]
						Xhi=GetPerimeter1Value(*CurrentFire, NumPts, 1);
						Ylo=GetPerimeter1Value(*CurrentFire, NumPts, 2);
						Yhi=GetPerimeter1Value(*CurrentFire, NumPts, 3);
						SetPerimeter2(NumPts, Xlo, Xhi, Ylo, Yhi);
						tranz(*CurrentFire, NumPts+1);
						SetNumPoints(*CurrentFire, NumPts);
						FindOuterFirePerimeter(NextFire);
						NumPts=GetNumPoints(NextFire);
						Xlo=GetPerimeter1Value(NextFire, NumPts, 0);	// use OldNumPoints from perim1[count]
						Xhi=GetPerimeter1Value(NextFire, NumPts, 1);
						Ylo=GetPerimeter1Value(NextFire, NumPts, 2);
						Yhi=GetPerimeter1Value(NextFire, NumPts, 3);
						SetPerimeter2(NumPts, Xlo, Xhi, Ylo, Yhi);
						tranz(*CurrentFire, NumPts+1);
						SetNumPoints(*CurrentFire, NumPts);
						*/
						numchek=endloop;
						chekct=numcross;
						outward=1;
						xwrite=0;
						fxend=-1;
						newnump=0;
						//tranz(*CurrentFire, 0);   // must transfer to perim2 for rediscretize
					}     // won't rediscretize if newnump==0
				}
				else
				{	if(xwrite+freadend-freadstart<SwapNumAlloc)
						MergeWrite(fxend, freadstart, freadend, &xwrite);
					else
					{    numchek=endloop;
						if(!outward)
							newnump=0;
						outward=1;
						chekct=numcross;
						xwrite=0;
						fxend=-1;
					}
				}
				if(GetCrossout(numchek)==0)
				{    SetCrossout(numchek,1);
					chekct++;                                  // INCREMENT CHEKCT
				}
				if(fxend==-1 && readfire)	   // different breakout criteria for outside first fire
					break;
			} while(numchek!=endloop);

			if(outward==0)                        // if outward fire has not been found
			{    if(xwrite>0)
					area=arp(2, xwrite);
				else
					area=0;
				if(area>0.0)
				{	outward=1;		// outside fire has been identified, there can be only 1 outward fire
					newnump=xwrite;	// after merger
					ctfire2=GetNewFires();
				}
				else	if(area<0.0)          	   // this was an inward fire, but
				{    if(xwrite>2)  			   // don't write very small "irrelevant" enclaves
					{	if(GetInout(*CurrentFire)==2 || GetInout(NextFire)==2)
						{	outward=1;		// outside fire has been identified, there can be only 1 outward fire
							newnump=xwrite;	// after merger
							ctfire2=GetNewFires();
							SetInout(*CurrentFire, 2);
						}
						else
						{    AllocPerimeter1(GetNewFires(), xwrite+1);
							tranz(GetNewFires(), xwrite);			// transfer points from perim2 to newfire array
							BoundaryBox(xwrite);
							SetNumPoints(GetNewFires(), xwrite);	// because this is an inward burning fire
							SetInout(GetNewFires(), 2);
							IncNewFires(1);
						}
					}
				}
			}
			else	if(xwrite>2) 							// don't write very small "irrelevant" enclaves
			{	AllocPerimeter1(GetNewFires(), xwrite+1);
				SwapTranz(GetNewFires(), xwrite);
				BoundaryBox(xwrite);
				SetNumPoints(GetNewFires(), xwrite);
				SetInout(GetNewFires(),2);
				IncNewFires(1);
				ctfire2=GetNewFires();
			}
			if(chekct<numcross)
			{    numchek=-1;
				xwrite=0;
				do
				{	inside=GetCrossout(++numchek);     // find next loop, inward or outward
				} while(inside);
				endloop=numchek;
				inside=1;			// reset inside for all but possibly first fire
				readfire=1;
			}
		}

          i=0;
		if(CheckPostFrontal(GETVAL))       // for
          {	NewFires2=GetNewFires();
              	if((PostFires=new long[(NewFires2-NewFires1+2)])!=NULL)
          	{	PostFires[0]=*CurrentFire;
     	          PostFires[1]=NextFire;
                    for(i=2; i<(NewFires2-NewFires1+2); i++)
                  		PostFires[i]=NewFires1+i-2;
         	          post.MergeFireRings(PostFires, NewFires2-NewFires1+2,
                    					intersect, interpoint, numcross, newnump);
          	     delete[] PostFires;//GlobalFree(PostFires);
			}
          }

		OldNumPoints=GetNumPoints(*CurrentFire);	// store old number of points for rediscretize
		if(newnump>P2NumAlloc)
			newnump=0;
		else if(newnump>0)
			SetNumPoints(*CurrentFire, newnump);
		SetNumPoints(NextFire, 0);
		SetInout(NextFire, 0);
		FreePerimeter1(NextFire);
		IncSkipFires(1);

          //if(GetNumAttacks()>0)
     	//	SetNewFireNumberForAttack(NextFire, *CurrentFire);
          //if(GetNumAirAttacks()>0)
     	//	SetNewFireNumberForAirAttack(NextFire, *CurrentFire);

          NextFire=*CurrentFire;
		if(newnump>0)
			rediscretize(CurrentFire, true);     // only rediscretize outer fire
          if(CheckPostFrontal(GETVAL) && NextFire!=*CurrentFire)
          	SetNewFireNumber(NextFire, *CurrentFire, post.AccessReferenceRingNum(1, GETVAL));
	}

	return true;
}


void Intersections::BoundaryBox(long NumPoints)
{// determines bounding box for new fire
	double xpt, ypt, Xlo, Xhi, Ylo, Yhi;

	Xlo=Xhi=GetPerimeter1Value(GetNewFires(), 0, XCOORD);
	Ylo=Yhi=GetPerimeter1Value(GetNewFires(), 0, YCOORD);
	for(int i=1; i<NumPoints; i++)
	{	xpt=GetPerimeter1Value(GetNewFires(), i, XCOORD);
		ypt=GetPerimeter1Value(GetNewFires(), i, YCOORD);
		if(xpt<Xlo) Xlo=xpt;
		else
		{	if(xpt>Xhi)
				Xhi=xpt;
		}
		if(ypt<Ylo) Ylo=ypt;
		else
		{	if(ypt>Yhi)
				Yhi=ypt;
		}
	}
	SetPerimeter1(GetNewFires(), NumPoints, Xlo, Xhi);
	SetFireChx(GetNewFires(), NumPoints, Ylo, Yhi);
}


void Intersections::AllocCrossout(long Number)
{
     if(Number)
     {    long i;

     	if(Number>=crossnumalloc)
		{   	FreeCrossout();
	     	nmemb=Number;
			//if((crossout=(long *) GlobalAlloc(GMEM_FIXED, nmemb*sizeof(long)))!=NULL)
			if((crossout=new long[nmemb])!=NULL)
			{	for(i=0; i<Number; i++)
					crossout[i]=0;
               	crossnumalloc=Number;
			}
			else
				crossout=0;
          }
          else
          {	for(i=0; i<crossnumalloc; i++)
				crossout[i]=0;
          }
     }
}

void Intersections::FreeCrossout()
{
	if(crossout)
		delete[] crossout;//GlobalFree(crossout);
	crossout=0;
     crossnumalloc=0;
}

long Intersections::GetCrossout(long Number)
{
	return crossout[Number];
}

void Intersections::SetCrossout(long Number, long Value)
{
	crossout[Number]=Value;
}


long Intersections::GetSpan(long Number, long ReadFire)
{
	long SpanBlank, Span;

	if(ReadFire)
		GetIntersection(Number, &SpanBlank, &Span);
	else
		GetIntersection(Number, &Span, &SpanBlank);
	SpanBlank=0;

	return Span;
}


double Intersections::arp(int PerimNum, long count)
{// calculates fire area for determining orientation of fire perimeter
 //if positive, then outward burning, if negative inward burning
	long count1, count2=0, NumPoints=0;
	double xpt1, ypt1, xpt2, ypt2, aangle, zangle;
	double area=0.0, DiffAngle, newarea;

	switch(PerimNum)
	{	case 1:   NumPoints=GetNumPoints(count);
				if(!NumPoints) break;
				startx=GetPerimeter1Value(count, 0, XCOORD);
				starty=GetPerimeter1Value(count, 0, YCOORD);
				while(count2<NumPoints)
				{    count2++;
					xpt1=GetPerimeter1Value(count, count2, XCOORD);
					ypt1=GetPerimeter1Value(count, count2, YCOORD);
					zangle=direction(xpt1, ypt1);        // reference angle
					if(zangle!=999.9)	// make sure that startx,starty!=x[0]y[0]
						break;
				}
				break;
		case 2:   NumPoints=count;
				if(!NumPoints) break;
				startx=GetPerimeter2Value(0, 0);
				starty=GetPerimeter2Value(0, 1);
				while(count2<NumPoints)
				{    count2++;
					xpt1=GetPerimeter2Value(count2, 0);
					ypt1=GetPerimeter2Value(count2, 1);
					zangle=direction(xpt1, ypt1);        // reference angle
					if(zangle!=999.9)	// make sure that startx,starty!=x[0]y[0]
						break;
				}
				break;
	}
	count2++;
	if(NumPoints<3)                   		// don't work on line fires
		return area;

	for(count1=count2;count1<NumPoints;count1++)
	{	switch(PerimNum)
		{	case 1:	xpt2=GetPerimeter1Value(count, count1, XCOORD);
					ypt2=GetPerimeter1Value(count, count1, YCOORD);
					break;
			case 2:	xpt2=GetPerimeter2Value(count1, 0);
					ypt2=GetPerimeter2Value(count1, 1);
					break;
		}
		newarea=.5*(startx*ypt1-xpt1*starty+xpt1*ypt2-xpt2*ypt1+xpt2*starty-startx*ypt2);
		newarea=fabs(newarea);
		aangle=direction(xpt2, ypt2);
		if(aangle!=999.9)
		{	DiffAngle=aangle-zangle;
			if(DiffAngle>PI)
				DiffAngle=-(2.0*PI-DiffAngle);
			else if(DiffAngle<-PI)
				DiffAngle=(2.0*PI+DiffAngle);
			if(DiffAngle>0.0)
				area-=newarea;
			else if(DiffAngle<0.0)
				area+=newarea;
			zangle=aangle;
		}
		xpt1=xpt2;
		ypt1=ypt2;
	}

	return area;
}


void Intersections::GetOffcheck(long *Offcheck)
{
	if(*Offcheck>=numcross)
		*Offcheck-=(numcross*(long) (*Offcheck/numcross));
}

long Intersections::intercode(long offcheck, long xypt)
{// translates crosspoint into offset coordinates to avoid 1st element in array
	long xsect;

	xsect=GetSpan(offcheck, xypt);
	if(noffset1>0)
	{    if(xsect<noffset1)
			xsect=xsect+noffset2;
		else
			xsect=xsect-noffset1;
	}
	return xsect;
}



void Intersections::MergeWrite(long xend, long readstart, long readend, long *xwrite)
{// writes segments of fire perimeter to perimeter2 array[][]
// or to perimeter 1 array if the fire is a new inward burning fire polygon
	long countx, count1, inc;
	double xpt, ypt, ros1, fli1, xpti, ypti, rcx1;

	if(readstart>=0)   /* to eliminate twin spike loops */
	{    inc=*xwrite;
		if(GetInout(writenum)!=0)	/* if existing fire */
		{   	for(countx=readstart;countx<=readend;countx++)  /* write points between loops */
			{    if(countx>=noffset2)
					count1=countx-noffset2;	/* decodes the offset array address */
				else
					count1=countx+noffset1;
				xpt=GetPerimeter1Value(readnum, count1, XCOORD);
				ypt=GetPerimeter1Value(readnum, count1, YCOORD);
				if(countx==readstart && inc!=0)
				{	xpti=GetPerimeter2Value(--inc, 0);	// check against previous point
					ypti=GetPerimeter2Value(inc, 1);
					if(xpt==xpti && ypt==ypti)
						*xwrite=inc;
					else
						inc++;
				}
				ros1=GetPerimeter1Value(readnum, count1, ROSVAL);
				fli1=GetPerimeter1Value(readnum, count1, FLIVAL);
                    rcx1=GetPerimeter1Value(readnum, count1, RCXVAL);
				SetPerimeter2(*xwrite, xpt, ypt, ros1, fli1, rcx1);
				inc++;
				*xwrite=inc;
			}
			if(xend>=0)					// unless at end of array, write last crosspoint
			{	GetInterPointCoord(xend, &xpti, &ypti);
				if(xpti!=xpt || ypti!=ypt)		// check against intersection point
				{	GetInterPointFChx(xend, &ros1, &fli1, &rcx1);
					SetPerimeter2(*xwrite, xpti, ypti, ros1, fli1, rcx1);    // must write XPTI & YPTI here not XPT & YPT !!!!!!!!
					inc++;
					*xwrite=inc;
				}
			}
		}
		else       // new fire front, inward burning
		{    for(countx=readstart;countx<=readend;countx++)  // write points between loops
			{    if(countx>=noffset2)
					count1=countx-noffset2;	// decodes the offset array address
				else
					count1=countx+noffset1;
				xpt=GetPerimeter1Value(readnum, count1, XCOORD);
				ypt=GetPerimeter1Value(readnum, count1, YCOORD);
				if(countx==readstart && inc!=0)
				{    xpti=swapperim[--inc*NUMDATA];	// check against previous point
					ypti=swapperim[inc*NUMDATA+1];
					if(xpt==xpti && ypt==ypti)
						*xwrite=inc;
					else
						inc++;
				}
				ros1=GetPerimeter1Value(readnum, count1, ROSVAL);
				fli1=GetPerimeter1Value(readnum, count1, FLIVAL);
				rcx1=GetPerimeter1Value(readnum, count1, RCXVAL);
				SetSwap(*xwrite, xpt, ypt, ros1, fli1, rcx1);	// must write XPTI here not XPT!!!!!!!!
				*xwrite=++inc;
			}
			if(xend>=0)						// unless at end of array, write last crosspoint
			{	GetInterPointCoord(xend, &xpti, &ypti);
				if(xpti!=xpt || ypti!=ypt)		// check against intersection point
				{	GetInterPointFChx(xend, &ros1, &fli1, &rcx1);
					SetSwap(*xwrite, xpti, ypti, ros1, fli1, rcx1);	// must write XPTI here not XPT!!!!!!!!
					*xwrite=++inc;
				}
			}
		}
	}
}


XUtilities::XUtilities()
{
	swapperim=0;
     swapnumalloc=0;
}


XUtilities::~XUtilities()
{
	FreeSwap();
}


void XUtilities::AllocSwap(long NumPoint)
{
	if(NumPoint)
     {	if(NumPoint>=swapnumalloc)
     	{	FreeSwap();
	     	nmemb=NUMDATA*NumPoint;              // dimension swap array to 5X original
			//if((swapperim=(double *) GlobalAlloc(GMEM_FIXED, nmemb*sizeof(double)))==NULL)
			if((swapperim=new double[nmemb])==NULL)
			{	swapperim=0;
				NumPoint=-1;		// debugging
			}
			swapnumalloc=NumPoint;
     	}
     }
}


void XUtilities::FreeSwap()
{
	if(swapperim)
		delete[] swapperim;//GlobalFree(swapperim);
    	swapnumalloc=0;
     swapperim=0;
}

void XUtilities::SetSwap(long NumPoint, double xpt, double ypt, double ros, double fli, double rcx)
{
	if(NumPoint<swapnumalloc && NumPoint>=0)
	{	NumPoint*=NUMDATA;
		swapperim[NumPoint]=xpt;
		swapperim[++NumPoint]=ypt;
		swapperim[++NumPoint]=ros;
		swapperim[++NumPoint]=fli;
		swapperim[++NumPoint]=rcx;
	}
	else
		NumPoint=-1;		// debugging

	//return 1;
}


void XUtilities::GetSwap(long NumPoint, double *xpt, double *ypt, double *ros, double *fli, double *rcx)
{
	NumPoint*=NUMDATA;
	*xpt=swapperim[NumPoint];
	*ypt=swapperim[++NumPoint];
	*ros=swapperim[++NumPoint];
	*fli=swapperim[++NumPoint];
	*rcx=swapperim[++NumPoint];
}


void XUtilities::RePositionFire(long *firenum)
{
     long FireCount, FireNum;//, NumPoints;

     FireCount=0;
	do                 				  		// checks for merged fires
	{	FireNum=GetInout(FireCount);        		// and overwrites arrays
		if(FireNum==0)
			break;
		FireCount++;
	} while(FireCount<=*firenum);
     if(FireCount<*firenum)
     {    SwapFirePerims(FireCount, *firenum);
		//if(GetNumAttacks()>0)                   // update fire number associated with attack
     	//	SetNewFireNumberForAttack(*firenum, FireCount);
          //if(GetNumAirAttacks()>0)
     	//	SetNewFireNumberForAirAttack(*firenum, FireCount);
          //*firenum=FireCount;
     }
}


void XUtilities::RestoreDeadPoints(long firenum)
{
	long i, nump=GetNumPoints(firenum);
	double xpt1, ypt1, ros1, fli1, rcx1;
     //double xhi, xlo, yhi, ylo;

     //xhi=xlo=GetPerimeter1Value(firenum, 0, XCOORD);
     //yhi=ylo=GetPerimeter1Value(firenum, 0, YCOORD);
     for(i=0; i<nump; i++)
	{    fli1=GetPerimeter2Value(i, FLIVAL);
		if(fli1<0.0)
     	{    GetPerimeter2(i, &xpt1, &ypt1, &ros1, &fli1, &rcx1);
     		SetPerimeter1(firenum, i, xpt1, ypt1);
	          SetFireChx(firenum, i, ros1, fli1);
               SetReact(firenum, i, rcx1);
	     }
          else
          {	xpt1=GetPerimeter1Value(firenum, i, XCOORD);
          	ypt1=GetPerimeter1Value(firenum, i, YCOORD);
          }
          //if(xpt1<xlo)
          //	xlo=xpt1;
          //if(xpt1>xhi)        // cant do "else if" because of vertical lines
          //	xhi=xpt1;
          //if(ypt1<ylo)
          //	ylo=ypt1;
          //if(ypt1>yhi)        // cant do "else if" becuause of horizontal lines
          //	yhi=ypt1;

     }
     //SetPerimeter1(firenum, nump, xlo, xhi);    // reset bounding box
	//SetFireChx(firenum, nump, ylo, yhi);
}


void XUtilities::rediscretize(long *firenum, bool Reorder)
{// adds midpoint between vertices exceeding maximum resolution
	long i, firetype, firet, nump, newnump, count=0, count1, count2=0;//, inc=0;
	double xpt=0, ypt=0, xptn=0, yptn=0, newx, newy, ros1=0, ros2=0, avgros, avgrcx;
	double xdiff, ydiff, xyhyp=0.01, fli1=0, fli2=0, avgfli, rcx1, rcx2;
	double MaxDistSquared=pow2(GetPerimRes()*MetricResolutionConvert());
	//AttackCrew* crew;

	//NewFireNumber=*firenum;             // INITIALIZE NEWFIRENUMBER as current fire
	firetype=GetInout(*firenum);
	ExNumPts=newnump=nump=GetNumPoints(*firenum); // INIIALIZE EXTINGUISHED NUMBER OF POINTS

	do                 				// checks for merged fires
	{	firet=GetInout(count);        // and overwrites arrays
		if(firet==0)
			break;
		count++;
	} while(count<=*firenum);           // searches for extinguished fires in array sequence
	if(count>*firenum)
		count=*firenum;
     if(Reorder==false)
     {	if(GetInout(*firenum)==2)      // inward burning
      	{	if(*firenum>=GetNumFires() && count<GetNumFires())   //
		     	count=*firenum;
          }
     }

	double Xlo, Xhi, Ylo, Yhi;
	bool FirstTime=true;
	//double Xlo=GetPerimeter1Value(*firenum, OldNumPoints, 0);	// use OldNumPoints from perim1[count]
	//double Xhi=GetPerimeter1Value(*firenum, OldNumPoints, 1);
	//double Ylo=GetPerimeter1Value(*firenum, OldNumPoints, 2);
	//double Yhi=GetPerimeter1Value(*firenum, OldNumPoints, 3);
	FreePerimeter1(*firenum);		// free original perimeter array
	newnump*=4;              	  	// dimension swap array to 4X original
	AllocSwap(newnump);
	if(firetype==2 && nump<3)		// if internal fire && smaller than 10 points */
	{	firetype=0;				// then eliminate the fire*/
		count2=0;
		ExNumPts=nump;                // XUtilities.ExNumPts, stores extinguished array size for dist meth check 2
		IncSkipFires(1);
	}
	else
	{    for(count1=0;count1<=nump;count1++)
		{    if(count1<nump)
			{    if(FirstTime)
				{	GetPerimeter2(count1, &xpt, &ypt, &ros1, &fli1, &rcx1);
					if(xpt<=GetLoEast())
						continue;
					if(xpt>=GetHiEast())
						continue;
					if(ypt<=GetLoNorth())
						continue;
					if(ypt>=GetHiNorth())
						continue;
					Xlo=Xhi=xpt;
					Ylo=Yhi=ypt;
					FirstTime=false;
				}
				GetPerimeter2(count1, &xptn, &yptn, &ros2, &fli2, &rcx2);
				// added to check for zero points 5/31/1995
				if(xptn<=GetLoEast())
					continue;
				if(xptn>=GetHiEast())
					continue;
				if(yptn<=GetLoNorth())
					continue;
				if(yptn>=GetHiNorth())
					continue;
				if(xptn<Xlo)       // determine bounding boxes
					Xlo=xptn;
				else if(xptn>Xhi)
					Xhi=xptn;
				if(yptn<Ylo)
					Ylo=yptn;
				else if(yptn>Yhi)
					Yhi=yptn;
			}
			else
				GetSwap(0, &xptn, &yptn, &ros2, &fli2, &rcx2);
			if(firetype<3)				// don't check resolution of barriers
			{	xdiff=xpt-xptn;
				ydiff=ypt-yptn;
				xyhyp=pow2(xdiff)+pow2(ydiff);	// hypotenuse distance between points
			}
			if(xyhyp>=MaxDistSquared)	// adds points to line segment
			{    newx=xpt-xdiff/2.0;			// adds points to line segment
				newy=ypt-ydiff/2.0;
				SetSwap(count2, xpt, ypt, ros1, fli1, rcx1);
				count2++;
				avgros=(ros1+ros2)/2.0;
				avgfli=(fabs(fli1)+fabs(fli2))/2.0;
			     if(fli1<0.0 && fli2<0.0)
					avgfli*=-1.0;
				else if(fli1<0.0 || fli2<0.0)
                    {	startx=newx;
                    	starty=newy;
                         for(i=0; i<GetNumFires(); i++)
                    	{	if(GetInout(i)<3)
                         		continue;
                         	if(Overlap(i))
                              	avgfli*=-1.0;
                         }
                    }
                    avgrcx=(rcx1+rcx2)/2.0;
				SetSwap(count2, newx, newy, avgros, avgfli, avgrcx);
				count2++;
			}
			else if(xyhyp>1e-12)
			{    SetSwap(count2, xpt, ypt, ros1, fli1, rcx1);
				count2++;
			}
			xpt=xptn;
			ypt=yptn;
			ros1=ros2;
			fli1=fli2;
		}                         // COORDINATES AS LAST POINT
	}
	if(firetype==1 && count2<3)		// if external fire && smaller than 4 points, can happen with precis. loss
	{	ExNumPts=nump;                // XUtilities.ExNumPts, stores extinguished array size for dist meth check 2
		firetype=0;				// then eliminate the fire*/
		count2=0;
		IncSkipFires(1);
	}
	SetNumPoints(count, count2);
	SetInout(count, firetype);
	if(count2!=0)
	{    SetSwap(count2, Xlo, Xhi, Ylo, Yhi, 0.0);
		count2++;
		AllocPerimeter1(count, count2);
		SwapTranz(count, count2);
	}
	if(*firenum!=count)					// if write points to new array
	{    //if(GetNumAttacks()>0)
     	//	SetNewFireNumberForAttack(*firenum, count);
          //if(GetNumAirAttacks()>0)
     	//	SetNewFireNumberForAirAttack(*firenum, count);
     	SetInout(*firenum,0);              // reset fire direction
		SetNumPoints(*firenum, 0);		// resent number of points in fire
          *firenum=count;				// update new fire number
	}
	//FreeSwap();
}


void XUtilities::SwapTranz(long writefire, long nump)
{// swap transfer file contents to perimeter1 array[][][]
	double xpt, ypt, ros, fli, rcx;

	for(long count1=0; count1<nump; count1++)
	{    GetSwap(count1, &xpt, &ypt, &ros, &fli, &rcx);
		SetPerimeter1(writefire, count1, xpt, ypt);
		SetFireChx(writefire, count1, ros, fli);
          SetReact(writefire, count1, rcx);
	}
}


void XUtilities::tranz(long count, long nump)
{// transfers points between arrays
	long ct1;
	double xpt=0, ypt=0, ros=0, fli=0, rcx;

	if(nump==0)
	{	nump=OldNumPoints=GetNumPoints(count);	// OldNumPoints is XUtilities::
		if(nump>0)							// for rediscretizing and knowing
		{	//FreePerimeter2();					// location of bounding box
               if(!SwapFirePerims(-1, count))   // backup method
               {    AllocPerimeter2(nump);
     			for(ct1=0;ct1<nump;ct1++)
	     		{	xpt=GetPerimeter1Value(count, ct1, XCOORD);
		     		ypt=GetPerimeter1Value(count, ct1, YCOORD);
			     	ros=GetPerimeter1Value(count, ct1, ROSVAL);
				     fli=GetPerimeter1Value(count, ct1, FLIVAL);
                         rcx=GetPerimeter1Value(count, ct1, RCXVAL);
     				SetPerimeter2(ct1, xpt, ypt, ros, fli, rcx);
	     		}
               }
		}
		else
			nump=-1;	// for debugging
	}
	else
	{	if(!SwapFirePerims(count, -nump))
          {    for(ct1=0;ct1<nump;ct1++)          // backup method
               {    GetPerimeter2(ct1, &xpt, &ypt, &ros, &fli, &rcx);
         			SetPerimeter1(count, ct1, xpt, ypt);
             		SetFireChx(count, ct1, ros, fli);
		          SetReact(count, ct1, rcx);
               }
          }
	}
}


