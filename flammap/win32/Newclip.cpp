//******************************************************************************
//	NEWCLIP.CPP	New loop-clipping functions for FARSITE
//
//
//				Copyright 1995, 1996
//				Mark A. Finney, Systems for Environemntal Management
//******************************************************************************

#include "flmglbvar.h"
#include "vec.h"

const double PI=acos(-1.0);
//extern const double PI;


bool Intersections::AllocNewIntersection(long NumCross)
{
	if(NumCross>0)
	{    if(NumCross>=newisectnumalloc)
     	{	FreeNewIntersection();
              	if((NewIsect=new long[NumCross*2])!=NULL)
			{	if((AltIsect=new long[NumCross*2])!=NULL)
	     		{	newisectnumalloc=NumCross;

                    	return true;
                    }
                    return false;
               }
               return false;
          }
          return true;
	}
	NewIsect=0;
     AltIsect=0;

	return false;
}

bool Intersections::FreeNewIntersection()
{
	if(NewIsect)
		delete[] NewIsect;
	if(AltIsect)
     	delete[] AltIsect;
	NewIsect=0;
	AltIsect=0;
     newisectnumalloc=0;

	return true;
}


void Intersections::SetNewIntersection(long Alt, long count, long isect1, long isect2)
{
	if(NewIsect && AltIsect)
	{    count*=2;
          switch(Alt)
          {	case 0:
         			NewIsect[count]=isect1;
				NewIsect[count+1]=isect2;
                    break;
          	case 1:
               	AltIsect[count]=isect1;
				AltIsect[count+1]=isect2;
                    break;
               case 2:
              		NewIsect[count]=isect1;
				NewIsect[count+1]=isect2;
				AltIsect[count]=isect1;
				AltIsect[count+1]=isect2;
                    break;
          }
     }
}


void Intersections::GetNewIntersection(long Alt, long count, long *isect1, long *isect2)
{
	if(Alt==0)
	{	if(NewIsect)
		{    count*=2;
			*isect1=NewIsect[count];
			*isect2=NewIsect[count+1];
		}
	}
	else if(AltIsect)
	{    count*=2;
		*isect1=AltIsect[count];
		*isect2=AltIsect[count+1];
	}
}


bool Intersections::TurningNumberOK(long CurrentFire, long StartPoint)
{
	double xpt, ypt, midx, midy;
     double Tx1, Ty1, Tx2, Ty2;
     long i, j, k, l, m, NumPoints;
     long inside;

     NumPoints=GetNumPoints(CurrentFire);
     GetInterPointCoord(StartPoint, &xpt, &ypt);
	GetIntersection(StartPoint, &i, &j);

     for(m=0; m<4; m++)
     {    switch(m)
          {    case 0:   k=i;
                         l=j;
                         break;
               case 1:   k=i+1;
                         l=j;
                         break;
               case 2:   k=i;
                         l=j+1;
                         break;
               case 3:   k=i+1;
                         l=j+1;
                         break;
          }
          if(k>NumPoints)
               k=0;
          if(l>NumPoints)
               l=0;
          Tx1=GetPerimeter1Value(CurrentFire, k, XCOORD);
         	Ty1=GetPerimeter1Value(CurrentFire, k, YCOORD);
          Tx2=GetPerimeter1Value(CurrentFire, l, XCOORD);
        	Ty2=GetPerimeter1Value(CurrentFire, l, YCOORD);
          midx=Tx1-(Tx1-Tx2)/2.0;
          midy=Ty1-(Ty1-Ty2)/2.0;
          startx=xpt-(xpt-midx)/2.0;
          starty=ypt-(ypt-midy)/2.0;
          inside=Overlap(CurrentFire);
          if(!inside)
               return true;
     }

     return false;
}

/*

bool Intersections::TurningNumberOK(long CurrentFire, long StartPoint)
{
	double xpt, ypt, midx, midy;
     double Tx1, Ty1, Tx2, Ty2;
     long i, j, Turn, NumPoints;

     NumPoints=GetNumPoints(CurrentFire);
     GetInterPointCoord(StartPoint, &xpt, &ypt);
	GetIntersection(StartPoint, &i, &j);
     i++;
     if(i==NumPoints)
     	i=0;
    	Tx1=GetPerimeter1Value(CurrentFire, i, XCOORD);
    	Ty1=GetPerimeter1Value(CurrentFire, i, YCOORD);
     Tx2=GetPerimeter1Value(CurrentFire, j, XCOORD);
   	Ty2=GetPerimeter1Value(CurrentFire, j, YCOORD);
     midx=Tx1-(Tx1-Tx2)/2.0;
     midy=Ty1-(Ty1-Ty2)/2.0;
     xpt=xpt-(xpt-midx)/2.0;
     ypt=ypt-(ypt-midy)/2.0;

     Turn=0;
     for(i=0; i<NumPoints; i++)
     {    j=i+1;
     	if(j==NumPoints)
	         	j=0;
     	Tx1=GetPerimeter1Value(CurrentFire, i, XCOORD);
     	Ty1=GetPerimeter1Value(CurrentFire, i, YCOORD);
          Tx2=GetPerimeter1Value(CurrentFire, j, XCOORD);
     	Ty2=GetPerimeter1Value(CurrentFire, j, YCOORD);

          if(Tx1>xpt && Tx2>xpt)
          	continue;
          if(Ty1!=Ty2)
          {    if(Ty1>=ypt && Ty2<ypt)
          		Turn-=1;
               else if(Ty1<=ypt && Ty2>ypt)
               	Turn+=1;
          }
     }
     if(Turn==0)
	   	return true;

     return false;
}
*/

void Intersections::CleanPerimeter(long CurrentFire)
{
	int Terminate;
	bool WritePerim=true;
	long i, icount, subcount;//, CrossCount=0;
	long StartPoint=-1, isect1, isect2;
	long LastStart=-1, isect1T, isect2T;

	AllocNewIntersection(numcross);
	for(i=0; i<numcross; i++)
	{    GetIntersection(i, &isect1, &isect2);
		SetNewIntersection(2, i, isect1+1, isect2+1);
	}

/*	if(GetInout(CurrentFire)==2)
	{    GetIntersection(0, &isect1, &isect2);
		if(isect1==0)
			StartPoint=-1;
	}
*/
	if(writenum==-1)
		WritePerim=false;
	do
	{    FindFirePerimeter(CurrentFire, StartPoint);
		/*do
		{	FindFirePerimeter(CurrentFire, StartPoint);
			if(GetInout(CurrentFire)==1)
				break;
			else if(CrossCount<numcross)// && !WritePerim)
			{	if(writenum<0)
				{	GetIntersection(CrossCount, &isect1, &isect2);
					ReorderPerimeter(CurrentFire, isect1+1);
				}
				break;
			}
			else
				break;
			CrossCount++;
		}while(CrossCount<=numcross/2);
		*/
		if(!WritePerim)
			break;
		//CrossCount=numcross;
		Terminate=1;
		for(icount=0; icount<numcross; icount++)
		{	GetNewIntersection(0, icount, &isect1, &isect2);
			if(isect1>0 && isect1<isect2)
			{    if(LastStart==icount)
				{	SetNewIntersection(0, icount, -isect1, -isect2);
					for(subcount=icount; subcount<numcross; subcount++)
					{	GetNewIntersection(0, subcount, &isect1T, &isect2T);
						if(isect1T==isect2 && isect2T==isect1)
						{	SetNewIntersection(0, subcount, -isect1T, -isect2T);
							break;
						}
					}
					LastStart=-1;
				}
				else
				{    //StartPoint=LastStart=icount;
					//Terminate=0;
					//break;
                         /* */
                    	if(TurningNumberOK(CurrentFire, icount))
                    	{	StartPoint=LastStart=icount;
						Terminate=0;
						break;
                         }
                         else
                         {	SetNewIntersection(0, icount, -isect1, -isect2);
						for(subcount=icount; subcount<numcross; subcount++)
						{	GetNewIntersection(0, subcount, &isect1T, &isect2T);
							if(isect1T==isect2 && isect2T==isect1)
							{	SetNewIntersection(0, subcount, -isect1T, -isect2T);
								break;
							}
                              }
						LastStart=-1;
                         }
                         /* */
				}
			}
		}
          // causes no interal fires to form
          //Terminate=true;

	} while(!Terminate);
	if(WritePerim)
	{	if(writenum<0)
			writenum*=-1;
		SetNumPoints(CurrentFire, writenum); //writecount-1);
		if(writenum==0)				  // done here or in Intersect::CrossCompare
		{	SetInout(CurrentFire, 0);
			FreePerimeter1(CurrentFire);
			IncSkipFires(1);
               if(CheckPostFrontal(GETVAL))
                    SetNewFireNumber(CurrentFire, -1, post.AccessReferenceRingNum(1, GETVAL));
		}
	}
	//FreeNewIntersection();
	//rediscretize(&CurrentFire);  // cant redisc here, because won't change CurrentFire
}                                   // back in crosscompare


void Intersections::FindFirePerimeter(long CurrentFire, long StartPoint)
{
	//double AngleForward, AngleBackward, Do, DiffAngle;
	double xpt, ypt, xptn, yptn, xptl, yptl, xl, yl;
	double xpta, ypta, xptb, yptb, xptc, yptc;
	double xptd, yptd, xpte, ypte;
	double ros1, fli1, ros2, fli2, ros3, fli3, ros4, fli4, ros5, fli5;
     double rcx1, rcx2, rcx3, rcx4, rcx5;
	double mindist, testdist, crossdist1, crossdist2, crossdist3, crossdist4, totaldist;
	double xdiff, ydiff, xdiffl, ydiffl, xdiffn, ydiffn, midx, midy;
	double tempx, tempy, distl, distn, outdist, PerpAngle;
	long numpts=GetNumPoints(CurrentFire);
	long writecount=0, writelimit=numpts*2;
	long pointcount=0, pointcountn=1, pointcountl=numpts-1, pointtoward;
	long crosscount, nextcount; //intercount, intermatch,
	long CrossFound=0, CrossSpan=-1, CrossNext=-1, CrossLastL=-1, CrossLastN=-1;
	long icount, isect1, isect2, istop1=-1, istop2=-1;
	long InternalLoop=-1;

	bool Terminate=false, StartAtCross=false, StartAtReverse=false;
	bool OffsetCrossPoint=false, EliminateTroubleFire=false;
	int concave, clockwise, Reverse, Direction=0, InOut;
	long isect2T, isect1T, isect2M, isect1M;
	long isect1A, isect2A, dup1, dup2, n;
	bool Erase, MateFound, Exit, ptcross1, ptcross2, crossyn;

	if(StartPoint!=-1)
	{    AllocSwap(GetNumPoints(CurrentFire));
		GetNewIntersection(0, StartPoint, &pointcountl, &pointcount);
		pointcountl-=1;
		pointcount-=1;
		istop1=pointcountl; 				// says when to stop looping
		istop2=pointcount;
		StartAtCross=true;

		xptl=GetPerimeter1Value(CurrentFire, pointcountl, XCOORD);
		yptl=GetPerimeter1Value(CurrentFire, pointcountl, YCOORD);
		ros1=GetPerimeter1Value(CurrentFire, pointcountl, ROSVAL);
		fli1=GetPerimeter1Value(CurrentFire, pointcountl, FLIVAL);
		rcx1=GetPerimeter1Value(CurrentFire, pointcountl, RCXVAL);
		pointcountn=pointcountl+1;
		xptn=GetPerimeter1Value(CurrentFire, pointcountn, XCOORD);
		yptn=GetPerimeter1Value(CurrentFire, pointcountn, YCOORD);
		GetInterPointCoord(StartPoint, &xpt, &ypt);
		CrossFound=1;
		CrossLastL=pointcount;
		CrossLastN=pointcount+1;
//		pointcount=pointcountn;
		pointcount=pointcountl;
		SetSwap(writecount++, xpt, ypt, ros1, fli1, rcx1);
/*          if(GetInout(CurrentFire)==3)
			SetInout(GetNewFires(), 3);        // set inward barrier to type 3
         	else
			SetInout(GetNewFires(), 2);		// else set inward fire to type 2
*/
		InOut=2;
	}
	else
	{	//FreePerimeter2();
		AllocPerimeter2(writelimit);//+numcross);
		xpt=GetPerimeter1Value(CurrentFire, pointcount, XCOORD);
		ypt=GetPerimeter1Value(CurrentFire, pointcount, YCOORD);
		xptl=GetPerimeter1Value(CurrentFire, pointcountl, XCOORD);
		yptl=GetPerimeter1Value(CurrentFire, pointcountl, YCOORD);
		xptn=GetPerimeter1Value(CurrentFire, pointcountn, XCOORD);
		yptn=GetPerimeter1Value(CurrentFire, pointcountn, YCOORD);
		ros1=GetPerimeter1Value(CurrentFire, 0, ROSVAL);
		fli1=GetPerimeter1Value(CurrentFire, 0, FLIVAL);
		rcx1=GetPerimeter1Value(CurrentFire, 0, RCXVAL);
		SetPerimeter2(writecount++, xpt, ypt, ros1, fli1, rcx1);
		InOut=GetInout(CurrentFire);
	}

	do
	{    clockwise=Orientation(vec(xptl, yptl), vec(xpt, ypt), vec(xptn, yptn));
		xdiffl=xpt-xptl;
		ydiffl=ypt-yptl;
		xdiffn=xpt-xptn;
		ydiffn=ypt-yptn;
		distl=sqrt(pow2(xdiffl) + pow2(ydiffl));
		distn=sqrt(pow2(xdiffn) + pow2(ydiffn));
		if(distl<distn)
		{	tempx=xpt-xdiffn*distl/distn;
			tempy=ypt-ydiffn*distl/distn;
			xdiff=xptl-tempx;
			ydiff=yptl-tempy;
			midx=xptl-xdiff/2.0;
			midy=yptl-ydiff/2.0;
			outdist=distn*2.0;
		}
		else
		{	if(distn<distl)
			{	tempx=xpt-xdiffl*distn/distl;
				tempy=ypt-ydiffl*distn/distl;
				xdiff=tempx-xptn;
				ydiff=tempy-yptn;
				midx=tempx-xdiff/2.0;
				midy=tempy-ydiff/2.0;
				outdist=distl*2.0;
			}
			else
			{	xdiff=xptl-xptn;
				ydiff=yptl-yptn;
				midx=xptl-xdiff/2.0;
				midy=yptl-ydiff/2.0;
				outdist=distn*2.0;
			}
		}
          if(fabs(xdiff)<1e-12)
               xdiff=0.0;
		if(xdiff==0.0)
		{	if(ydiff>0.0)
				PerpAngle=PI;
			else if(ydiff<0.0)
				PerpAngle=0.0;
		}
		else
			PerpAngle=atan2(ydiff, xdiff)+PI/2.0;
		tempx=xpt+outdist*cos(PerpAngle);
		tempy=ypt+outdist*sin(PerpAngle);
		distn=pow2(tempx-xpt)+pow2(tempy-ypt);	  // distance from outpt to xpt,ypt
		distl=pow2(tempx-midx)+pow2(tempy-midy);  // distance from outpt to midx,midy
		if(distn>distl)
			concave=1;
		else
			concave=0;
		//inside=Inside(vec(tempx, tempy), vec(xptl, yptl), vec(xpt, ypt), vec(xptn, yptn));
		if(InOut==1)
		{    if(CrossFound)
			{	if(clockwise<0 && concave)
					Reverse=0;		// keep it the same
				else if(clockwise==0)
					Reverse=0;
				else
					Reverse=1;
			}
			else
			{    if(writecount==1 && concave)
				{	Reverse=1; 	// can't have concave on first point, because
					StartAtReverse=true;
				}
				else                // always start at extreme point, which must be convex
				{	if((clockwise>=0 && !concave) || (clockwise<=0 && concave))
						Reverse=0;	// keep it the same
					else
						Reverse=1;
				}
			}
		}
		else                     // inward burning fire
		{    if(CrossFound)
			{    if(clockwise<0 && concave)
					Reverse=0;
				else if(clockwise==0)
					Reverse=0;
				else if(InternalLoop<0)
					Reverse=1;
				else
					Reverse=0;
			}
			else
			{    if(writecount==1 && concave)
				{	//Reverse=1;
					Reverse=0;
					StartAtReverse=true;
				}
				else
				{	if((clockwise>=0 && !concave) || (clockwise<=0 && concave))
						Reverse=0;
					else
						Reverse=1;
				}
			}
		}
		if(Reverse)
		{    Direction=Reverse;
			pointtoward=pointcountl;
			pointcountl=pointcountn;
			pointcountn=pointtoward;
			if(!CrossFound)
			{	xptc=xpte=xptl;
				yptc=ypte=yptl;
			}
			else
			{	xptc=xpte=xptn=GetPerimeter1Value(CurrentFire, pointcountn, XCOORD);
				yptc=ypte=yptn=GetPerimeter1Value(CurrentFire, pointcountn, YCOORD);
				if(pointcountl!=0 && pointcountn>pointcountl)
				//	Reverse=0;
					Direction=0;
			}
		}
		else
		{	xptc=xpte=xptn;
			yptc=ypte=yptn;
			pointtoward=pointcountn;
		}
		//Reverse=0;
		ros5=ros2=GetPerimeter1Value(CurrentFire, pointtoward, ROSVAL);
		fli5=fli2=GetPerimeter1Value(CurrentFire, pointtoward, FLIVAL);
		rcx5=rcx2=GetPerimeter1Value(CurrentFire, pointtoward, RCXVAL);
          crossdist1=mindist=sqrt(pow2(xpt-xptc)+pow2(ypt-yptc));
		testdist=0;
		for(crosscount=0; crosscount<numpts; crosscount++)
		{    nextcount=crosscount+1;
			if(nextcount==numpts)
				nextcount=0;
			if(crosscount==pointcount || crosscount==pointtoward ||
			   nextcount==pointcount || nextcount==pointtoward)
				continue;
			if(crosscount==min(CrossLastL, CrossLastN))
			{	if(CrossLastN==0)
				{    if(StartAtReverse)
					{	if(CrossLastL<2)
							continue;
						else	if(Direction==0)
							continue;
					}
					else
					{	if(Direction==1)
							continue;
					}
				}
				else
					continue;
			}
			else if(CrossLastN==0 && CrossLastL==numpts-1 && crosscount==numpts-1)
				continue;
			else if(CrossLastL==0 && CrossLastN==numpts-1 && crosscount==numpts-1)
				continue;
			xpta=GetPerimeter1Value(CurrentFire, crosscount, XCOORD);
			ypta=GetPerimeter1Value(CurrentFire, crosscount, YCOORD);
			xptb=GetPerimeter1Value(CurrentFire, nextcount, XCOORD);
			yptb=GetPerimeter1Value(CurrentFire, nextcount, YCOORD);
               dup1=dup2=1; // want crosses only if on span1 && span2
			crossyn=Cross(xpt, ypt, xptc, yptc, xpta, ypta, xptb, yptb, &xptd, &yptd, &dup1, &dup2);

               ptcross1=ptcross2=false;

               if(dup1<0 && dup2<0)  // indicates single point crosses with single pt
                    ptcross1=ptcross2=false;
               else if(dup1<0)          // if cross duplicates xpt, ypt
               {    dup1=1;
                    dup2=0;
                    if(!Cross(xptl, yptl, xptc, yptc, xpta, ypta, xptb, yptb, &xl, &yl, &dup1, &dup2))
                         ptcross1=true; // means this is just a single pt cross
                    else if(dup1<0)
                         ptcross1=true; // means this is just a single pt cross
                }
               else if(dup2<0)     // if cross duplicates crxpt, crypt
               {    n=crosscount-1;
                    if(n<0)
                         n=GetNumPoints(CurrentFire)-1;
                    xptl=GetPerimeter1Value(CurrentFire, n, XCOORD);
          	   	yptl=GetPerimeter1Value(CurrentFire, n, YCOORD);
                    dup1=0;
                    dup2=1;
                    if(!Cross(xpt, ypt, xptc, yptc, xptl, yptl, xptb, yptb, &xl, &yl, &dup1, &dup2))
                         ptcross2=true; // means this is just a single pt cross
                    else if(dup2<0)
                         ptcross2=true; // means this is just a single pt cross
               }

               if(crossyn)
               {    if(ptcross1==true || ptcross2==true)   // only single pt crosses, so no crosses to process
                         crossyn=false;
               }

               if(crossyn)
			{    testdist=sqrt(pow2(xpt-xptd)+pow2(ypt-yptd));
				if(testdist<1e-9)	// same point
				{	if(!CrossFound)
					{    CrossFound=1;
						testdist=-1;
						CrossSpan=crosscount;
						CrossNext=nextcount;
                              //------------------------------------------------
                              // offset cross point if same one, 10/20/2000

                              xdiffl=xpta-xptb;
						ydiffl=ypta-yptb;
						distl=sqrt(pow2(xdiffl)+pow2(ydiffl));
                              if(distl>0.02)
                                   outdist=0.01;  // maximum distance
                              else	if(distl>0.000002)
						     outdist=distl/2.0;
                              else
							outdist=0.000001;

                              xdiffl=outdist*xdiffl/distl;
     					ydiffl=outdist*ydiffl/distl;
	     				xptd=xpt-xdiffl;
		     			yptd=ypt-ydiffl;

                              //------------------------------------------------
						break;
					}
					continue;
				}
				if(testdist<mindist)
				{	mindist=testdist;
					crossdist2=crossdist1-mindist;
					CrossFound=1;
					CrossSpan=crosscount;
					CrossNext=nextcount;
					xpte=xptd;
					ypte=yptd;
					ros3=GetPerimeter1Value(CurrentFire, crosscount, ROSVAL);
					fli3=GetPerimeter1Value(CurrentFire, crosscount, FLIVAL);
					rcx3=GetPerimeter1Value(CurrentFire, crosscount, RCXVAL);
					ros4=GetPerimeter1Value(CurrentFire, nextcount, ROSVAL);
					fli4=GetPerimeter1Value(CurrentFire, nextcount, FLIVAL);
					rcx4=GetPerimeter1Value(CurrentFire, nextcount, RCXVAL);
					crossdist3=sqrt(pow2(xpta-xptb)+pow2(ypta-yptb));
					crossdist4=sqrt(pow2(xpta-xptd)+pow2(ypta-yptd));
					totaldist=crossdist3+crossdist1;
					crossdist3-=crossdist4;
					if(totaldist>0.0)
					{	ros5=ros1*mindist/totaldist + ros2*crossdist2/totaldist+
							ros3*crossdist3/totaldist + ros4*crossdist4/totaldist;
						fli5=fabs(fli1)*mindist/totaldist + fabs(fli2)*crossdist2/totaldist+
							fabs(fli3)*crossdist3/totaldist + fabs(fli4)*crossdist4/totaldist;
						rcx5=rcx1*mindist/totaldist + rcx2*crossdist2/totaldist+
							rcx3*crossdist3/totaldist + rcx4*crossdist4/totaldist;
						if((fli1<0.0 || fli2<0.0) && (fli3<0.0 || fli4<0.0))
							fli5*=-1.0;
					}
				}
			}
		}

		//-------------------------------------------
		// if writing existing outer or inner polygon
		//-------------------------------------------

		if(StartPoint==-1)
		{	if(CrossFound && StartAtCross)
			{    if(istop1==-1)
				{	istop1=min(pointcount, pointtoward);
					istop2=CrossSpan;
					writecount=0;
				}
				else	if(CrossSpan==istop1 && //CrossFound &&
				  (pointcount==istop2 || pointtoward==istop2))
					Terminate=1;
			}
			else
				SetPerimeter2(writecount++, xpte, ypte, ros5, fli5, rcx5);
		}
		else
		{
		//------------------------------
		// if writing new inward polygon
		//------------------------------
			if(CrossFound && CrossSpan==istop1 &&
			  (pointcount==istop2 || pointtoward==istop2))
				Terminate=1;
			else if(writecount>numpts)
			{	Terminate=1;
				for(icount=0; icount<numcross; icount++)
				{	GetNewIntersection(0, icount, &isect1, &isect2);
					isect1-=1;
					isect2-=1;
					if((isect1==istop2 && isect2==istop1) || (isect1==istop1 && isect2==istop2))
					{    isect1+=1;
						isect2+=1;
						SetNewIntersection(0, icount, -isect1, -isect2);
					}
				}
				writecount=0;
			}
			if(!Terminate)
			{	SetSwap(writecount++, xpte, ypte, ros5, fli5, rcx5);
				if(CrossFound)		// only for inward fires after crossfound
					OffsetCrossPoint=true;
			}
		}
		if(CrossFound)
		{	if(testdist!=0.0)        // if more crosses after last cross
			{    InternalLoop=-1;
				CrossFound=false;
				Erase=false; MateFound=false; Exit=false;
				for(icount=0; icount<numcross; icount++)
				{    GetNewIntersection(0, icount, &isect1, &isect2);
					isect2T=abs(isect2)-1;
					isect1T=abs(isect1)-1;
					if(!MateFound)
					{	if(((isect2T==CrossSpan || isect2T==CrossNext) &&
						   (isect1T==pointcount || isect1T==pointcountn)) ||
						   ((isect1T==CrossSpan || isect1T==CrossNext) &&
						    (isect2T==pointcount || isect2T==pointcountn)))
						{    MateFound=true;
							Erase=true;
							isect2M=isect1T;
							isect1M=isect2T;
                                   if(StartPoint==-1 && isect1<0)
								MateFound=false;
						}
					}
					else
					{	if(isect1T==isect1M && isect2T==isect2M)
						{	Erase=true;
							Exit=true;
						}
					}
					if(Erase)
					{    if(InOut==2 && isect1<0)       // if not 1st time at intersection
						{	GetNewIntersection(1, icount, &isect1A, &isect2A);
							if(isect1A>0 || isect2A>0)   // if 2nd time at intersection
							{	CrossFound=true;
								SetNewIntersection(1, icount, -isect1A, -isect2A);
								if(Exit && OffsetCrossPoint)
								{    xdiffl=xpt-xpte;
									ydiffl=ypt-ypte;
									distl=sqrt(pow2(xdiffl)+pow2(ydiffl));
                                             if(distl>0.02)
                                                  outdist=0.01;  // maximum distance
                                             else	if(distl>0.000002)
										outdist=distl/2.0;
									else
									{	outdist=0.000001;
										CrossFound=false;
										Exit=true;
									}
                                             xdiffl=(distl-outdist)*xdiffl/distl;
     								ydiffl=(distl-outdist)*ydiffl/distl;
	     							tempx=xpt-xdiffl;
		     						tempy=ypt-ydiffl;
			     					SetSwap(writecount-1, tempx, tempy, ros5, fli5, rcx5);
			     					OffsetCrossPoint=false;
								}
							}
							else		// NEW 1/11/1996 to try to exclude illogical enclaves
								Exit=true;
						}
						else
						{	isect1T+=1;
							isect2T+=1;
							if(InOut==2)
							{	if(InternalLoop==-1)
									InternalLoop=icount;
								else
								{	if(icount==InternalLoop+1)
										InternalLoop=icount;
									else
										InternalLoop=-2;
								}	// NEW 1/11/1996 to try to exclude illogical enclaves
								SetNewIntersection(1, icount, -isect2T, -isect1T);
							}
							SetNewIntersection(0, icount, -isect1T, -isect2T);
							CrossFound=true;
							//if(StartPoint==-1)
							//	SetNewIntersection(1, icount, -isect1T, -isect2T);
						}
						Erase=false;
					}
					if(Exit)
						break;
				}
				if(StartPoint>-1 && !CrossFound)
				{	Terminate=1;
					writecount=0;
					GetNewIntersection(0, StartPoint, &isect1, &isect2);
					if(isect1>0)
						isect1*=-1.0;
					if(isect2>0)
						isect2*=-1.0;
					SetNewIntersection(0, StartPoint, isect1, isect2);
					for(icount=0; icount<numcross; icount++)
					{	GetNewIntersection(0, icount, &isect1, &isect2);
						isect1-=1;
						isect2-=1;
						if(isect1==istop2 && isect2==istop1)
						{    isect1+=1;
							isect2+=1;
							SetNewIntersection(0, icount, -isect1, -isect2);
							break;
						}
					}
				}
				pointcountn=CrossNext;
				CrossLastN=pointtoward;       // switch crossed spans
				pointtoward=pointcount;
				pointcountl=pointcount=CrossSpan;
				CrossLastL=pointtoward;
			}
			else			// if crosses last time but not now
			{    CrossFound=0;
				OffsetCrossPoint=false;
				CrossLastL=-1;//CrossSpan=-1;
				CrossLastN=-1;//CrossNext=-1;
				pointcount=pointtoward;
				if(Reverse) // if(Direction)
				{	pointcountl=pointcount+1;
					pointcountn=pointcount-1;
				}
				else
				{	pointcountl=pointcount-1;
					pointcountn=pointcount+1;
				}
				Direction=Reverse;
				if(pointcountn==numpts)
					pointcountn=0;
				else if(pointcountn<0)
					pointcountn=numpts-1;
				if(pointcountl==numpts)
					pointcountl=0;
				else if(pointcountl<0)
					pointcountl=numpts-1;
			}
		}
		else				// if no crosses last & this time through
		{    if(Direction)
			{	pointcountn--;
				pointcountl=pointcount;
				pointcount--;
			}
			else
			{	pointcountn++;
				pointcountl=pointcount;
				pointcount++;
			}
			if(pointcountn==numpts)
				pointcountn=0;
			else if(pointcountn<0)
				pointcountn=numpts-1;
			if(pointcount==numpts)
				pointcount=0;
			else	if(pointcount<0)
				pointcount=numpts-1;
		}
		xptn=GetPerimeter1Value(CurrentFire, pointcountn, XCOORD);
		yptn=GetPerimeter1Value(CurrentFire, pointcountn, YCOORD);
		xptl=xpt;
		yptl=ypt;
		xpt=xpte;
		ypt=ypte;
		ros1=ros5;
		fli1=fli5;
		if(StartPoint==-1)
		{	if(pointcount==0 && !CrossFound)
				Terminate=true;
			if(writecount>=writelimit)
			{	for(icount=0; icount<numcross; icount++)
				{	GetNewIntersection(0, icount, &isect1, &isect2);
					if(isect1>0)
					{	isect1*=-1;
						isect2*=-1;
						SetNewIntersection(0, icount, -isect1, -isect2);
					}
				}
				Terminate=true;
				tranz(CurrentFire, 0);
				writecount=numpts+1;
				EliminateTroubleFire=true;
			}
		}
	}while(!Terminate);
	if(StartPoint==-1)
	{	writenum=writecount-1;	// writenum is Intersections::class data member
		if(InOut==2)
		{	if(arp(2, writenum)>0.0)
			{	writenum*=-1;

			}
			if(EliminateTroubleFire)
				writenum=0;
		}
	}
	else
	{	AllocPerimeter1(GetNewFires(), writecount+1);
          SetNumPoints(GetNewFires(), writecount);
		if(GetInout(CurrentFire)==3)
			SetInout(GetNewFires(), 3);        // set inward barrier to type 3
         	else
			SetInout(GetNewFires(), 2);		// else set inward fire to type 2
		SwapTranz(GetNewFires(), writecount);
		BoundaryBox(writecount);
		if(writecount>2 && arp(1, GetNewFires())<0.0)	// only finish writing new fire
			IncNewFires(1);                              // if numpts>2 && inward fire
		else
		{	FreePerimeter1(GetNewFires());
			SetNumPoints(GetNewFires(), 0);
			SetInout(GetNewFires(), 0);
		}
		//FreeSwap();
	}
}


//-------------------------------------------------------------------------------------
//----------THIS VERSION WORKS 6/25/1995, SAVED WHILE MODIFICAITONS MADE---------------
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------

void StandardizePolygon::FindOuterFirePerimeter(long CurrentFire)
{
	//double AngleForward, AngleBackward, Do, DiffAngle;
	double xpt, ypt, xptn, yptn, xptl, yptl, xl, yl;
	double xpta, ypta, xptb, yptb, xptc, yptc;
	double xptd, yptd, xpte, ypte;
	double ros1, fli1, ros2, fli2, ros3, fli3, ros4, fli4, ros5, fli5;
     double rcx1, rcx2, rcx3, rcx4, rcx5;
	double mindist, testdist, crossdist1, crossdist2, crossdist3, crossdist4, totaldist;
	double xdiff, ydiff, xdiffl, ydiffl, xdiffn, ydiffn, midx, midy;
	double tempx, tempy, distl, distn, outdist, PerpAngle;

	//bool FIRST;
     long n, dup1, dup2;
	long numpts=GetNumPoints(CurrentFire);
	long writecount=0, writelimit=numpts*4;
	long pointcount=0, pointcountn=1, pointcountl=numpts-1, pointtoward;
	long crosscount, nextcount; //intercount, intermatch,
	long CrossFound=0, CrossSpan=-1, CrossNext=-1, CrossLastL=-1, CrossLastN=-1;
	bool Terminate=false;
	bool StartAtReverse=false, ptcross1, ptcross2, crossyn;
	int concave, clockwise, Reverse, Direction=0;

	//FreePerimeter2();
	AllocPerimeter2(writelimit);//+numcross);
	xpt=GetPerimeter1Value(CurrentFire, pointcount, XCOORD);
	ypt=GetPerimeter1Value(CurrentFire, pointcount, YCOORD);
	xptl=GetPerimeter1Value(CurrentFire, pointcountl, XCOORD);
	yptl=GetPerimeter1Value(CurrentFire, pointcountl, YCOORD);
	xptn=GetPerimeter1Value(CurrentFire, pointcountn, XCOORD);
	yptn=GetPerimeter1Value(CurrentFire, pointcountn, YCOORD);
	ros1=GetPerimeter1Value(CurrentFire, 0, ROSVAL);
	fli1=GetPerimeter1Value(CurrentFire, 0, FLIVAL);
	rcx1=GetPerimeter1Value(CurrentFire, 0, RCXVAL);
	SetPerimeter2(writecount++, xpt, ypt, ros1, fli1, rcx1);
	do
	{    clockwise=Orientation(vec(xptl, yptl), vec(xpt, ypt), vec(xptn, yptn));
		xdiffl=xpt-xptl;
		ydiffl=ypt-yptl;
		xdiffn=xpt-xptn;
		ydiffn=ypt-yptn;
		distl=sqrt(pow2(xdiffl) + pow2(ydiffl));
		distn=sqrt(pow2(xdiffn) + pow2(ydiffn));
		if(distl<distn)
		{	tempx=xpt-xdiffn*distl/distn;
			tempy=ypt-ydiffn*distl/distn;
			xdiff=xptl-tempx;
			ydiff=yptl-tempy;
			midx=xptl-xdiff/2.0;
			midy=yptl-ydiff/2.0;
			outdist=distn*2.0;
		}
		else
		{	if(distn<distl)
			{	tempx=xpt-xdiffl*distn/distl;
				tempy=ypt-ydiffl*distn/distl;
				xdiff=tempx-xptn;
				ydiff=tempy-yptn;
				midx=tempx-xdiff/2.0;
				midy=tempy-ydiff/2.0;
				outdist=distl*2.0;
			}
			else
			{	xdiff=xptl-xptn;
				ydiff=yptl-yptn;
				midx=xptl-xdiff/2.0;
				midy=yptl-ydiff/2.0;
				outdist=distn*2.0;
			}
		}
          if(fabs(xdiff)<1e-12)
               xdiff=0.0;
		if(xdiff==0.0)
		{	if(ydiff>0.0)
				PerpAngle=PI;
			else if(ydiff<0.0)
				PerpAngle=0.0;
		}
		else
			PerpAngle=atan2(ydiff, xdiff)+PI/2.0;
		tempx=xpt+outdist*cos(PerpAngle);
		tempy=ypt+outdist*sin(PerpAngle);
		distn=pow2(tempx-xpt)+pow2(tempy-ypt);	  // distance from outpt to xpt,ypt
		distl=pow2(tempx-midx)+pow2(tempy-midy);  // distance from outpt to midx,midy
		if(distn>distl)
			concave=1;
		else
			concave=0;
		//inside=Inside(vec(tempx, tempy), vec(xptl, yptl), vec(xpt, ypt), vec(xptn, yptn));
		if(GetInout(CurrentFire)==1)
		{    if(CrossFound)
			{	if(clockwise<0 && concave)
					Reverse=0;		// keep it the same
				else if(clockwise==0)
				{	//distn=sqrt(pow2(xptl-xptn)+pow2(yptl-yptn));
					/*if(distn<distl)
						Reverse=0;
					else
						Reverse=1;		// change direction
					*/
					Reverse=0;
				}
				else
					Reverse=1;		// change direction
			}
			else
			{    if(writecount==1 && concave)
				{	Reverse=1; 	// can't have concave on first point, because
					StartAtReverse=true;
				}
				else                // always start at extreme point, which must be convex
				{	if((clockwise>=0 && !concave) || (clockwise<=0 && concave))
						Reverse=0;	// keep it the same
					else
						Reverse=1;
				}
			}
		}
		else
		{    if(CrossFound)
			{	if(clockwise<0 && concave)
					Reverse=0;
				else if(clockwise==0)
				{	//distn=sqrt(pow2(xptl-xptn)+pow2(yptl-yptn));
					/*if(distn<distl)
						Reverse=0;
					else
						Reverse=1;		// change direction
					*/
					Reverse=0;
				}
				else
					Reverse=1;		// change direction
			}
			else
			{    if(writecount==1 && concave)
					Reverse=1; 	// can't have concave on first point, because
				else                // always start at extreme point, which must be convex
				{	if((clockwise<=0 && !concave) || (clockwise>=0 && concave))
						Reverse=0;
					else
						Reverse=1;
				}
			}
		}
		if(Reverse)
		{    Direction=Reverse;
			pointtoward=pointcountl;
			pointcountl=pointcountn;
			pointcountn=pointtoward;
			if(!CrossFound)
			{	xptc=xpte=xptl;
				yptc=ypte=yptl;
			}
			else
			{	xptc=xpte=xptn=GetPerimeter1Value(CurrentFire, pointcountn, XCOORD);
				yptc=ypte=yptn=GetPerimeter1Value(CurrentFire, pointcountn, YCOORD);
				if(pointcountl!=0 && pointcountn>pointcountl)
				//	Reverse=0;
					Direction=0;
			}
		}
		else
		{	xptc=xpte=xptn;
			yptc=ypte=yptn;
			pointtoward=pointcountn;
		}
		//Reverse=0;
		ros5=ros2=GetPerimeter1Value(CurrentFire, pointtoward, ROSVAL);
		fli5=fli2=GetPerimeter1Value(CurrentFire, pointtoward, FLIVAL);
          rcx5=rcx2=GetPerimeter1Value(CurrentFire, pointtoward, RCXVAL);
		crossdist1=mindist=sqrt(pow2(xpt-xptc)+pow2(ypt-yptc));
		testdist=0;
		for(crosscount=0; crosscount<numpts; crosscount++)
		{    nextcount=crosscount+1;
			if(nextcount==numpts)
				nextcount=0;
			if(crosscount==pointcount || crosscount==pointtoward ||
			   nextcount==pointcount || nextcount==pointtoward)
				continue;
			if(crosscount==min(CrossLastL, CrossLastN))
			{    if(CrossLastN==0)
				{    if(StartAtReverse)
					{    if(CrossLastL<2)
							continue;
						else if(Direction==0)
							continue;
					}
					else
					{	if(Direction==1)
							continue;
					}
				}
				else
					continue;
			}
			else if(CrossLastN==0 && CrossLastL==numpts-1 && crosscount==numpts-1)
				continue;
			else if(CrossLastL==0 && CrossLastN==numpts-1 && crosscount==numpts-1)
				continue;
			xpta=GetPerimeter1Value(CurrentFire, crosscount, XCOORD);
			ypta=GetPerimeter1Value(CurrentFire, crosscount, YCOORD);
			xptb=GetPerimeter1Value(CurrentFire, nextcount, XCOORD);
			yptb=GetPerimeter1Value(CurrentFire, nextcount, YCOORD);
               dup1=dup2=1; // want crosses only if on span1 && span2
			crossyn=Cross(xpt, ypt, xptc, yptc, xpta, ypta, xptb, yptb, &xptd, &yptd, &dup1, &dup2);

               ptcross1=ptcross2=false;
               if(dup1<0 && dup2<0)      // indicates single point crosses with single pt
                    ptcross1=ptcross2=false;
               else if(dup1<0)          // if cross duplicates xpt, ypt
               {    dup1=1;
                    dup2=0;
                    if(!Cross(xptl, yptl, xptc, yptc, xpta, ypta, xptb, yptb, &xl, &yl, &dup1, &dup2))
                         ptcross1=true; // means this is just a single pt cross
                    else if(dup1<0)
                         ptcross1=true; // means this is just a single pt cross
               }
               else if(dup2<0)     // if cross duplicates crxpt, crypt
               {    n=crosscount-1;
                    if(n<0)
                         n=GetNumPoints(CurrentFire)-1;
                    xptl=GetPerimeter1Value(CurrentFire, n, XCOORD);
          	   	yptl=GetPerimeter1Value(CurrentFire, n, YCOORD);
                    dup1=0;
                    dup2=1;
                    if(!Cross(xpt, ypt, xptc, yptc, xptl, yptl, xptb, yptb, &xl, &yl, &dup1, &dup2))
                         ptcross2=true;   // means this is just a single pt cross
                    else if(dup2<0)
                         ptcross2=true; // means this is just a single pt cross
               }
               if(crossyn)
               {    if(ptcross1==true || ptcross2==true)    // just a single pt cross
                         crossyn=false;
               }

               if(crossyn)
			{	testdist=sqrt(pow2(xpt-xptd)+pow2(ypt-yptd));
				if(testdist<1e-9)	// same point
				{	if(!CrossFound)
					{    CrossFound=1;
						testdist=-1;
						CrossSpan=crosscount;
						CrossNext=nextcount;
						//------------------------------------------------
                              // offset cross point if same one, 10/20/2000
                              xdiffl=xpta-xptb;
						ydiffl=ypta-yptb;
						distl=sqrt(pow2(xdiffl)+pow2(ydiffl));
                              if(distl>0.02)
                                   outdist=0.01;  // maximum distance
                              else	if(distl>0.000002)
						     outdist=distl/2.0;
                              else
							outdist=0.000001;

                              xdiffl=outdist*xdiffl/distl;
     					ydiffl=outdist*ydiffl/distl;
	     				xptd=xpt-xdiffl;
		     			yptd=ypt-ydiffl;
                              //------------------------------------------------
						break;
					}
					continue;
				}
				if(testdist<mindist)
				{	mindist=testdist;
					crossdist2=crossdist1-mindist;
					CrossFound=1;
					CrossSpan=crosscount;
					CrossNext=nextcount;
					xpte=xptd;
					ypte=yptd;
					ros3=GetPerimeter1Value(CurrentFire, crosscount, ROSVAL);
					fli3=GetPerimeter1Value(CurrentFire, crosscount, FLIVAL);
					rcx3=GetPerimeter1Value(CurrentFire, crosscount, RCXVAL);
					ros4=GetPerimeter1Value(CurrentFire, nextcount, ROSVAL);
					fli4=GetPerimeter1Value(CurrentFire, nextcount, FLIVAL);
					rcx4=GetPerimeter1Value(CurrentFire, nextcount, RCXVAL);
					crossdist3=sqrt(pow2(xpta-xptb)+pow2(ypta-yptb));
					crossdist4=sqrt(pow2(xpta-xptd)+pow2(ypta-yptd));
					totaldist=crossdist3+crossdist1;
					crossdist3-=crossdist4;
					if(totaldist>0.0)
					{	ros5=ros1*mindist/totaldist + ros2*crossdist2/totaldist+
							ros3*crossdist3/totaldist + ros4*crossdist4/totaldist;
						fli5=fabs(fli1)*mindist/totaldist + fabs(fli2)*crossdist2/totaldist+
							fabs(fli3)*crossdist3/totaldist + fabs(fli4)*crossdist4/totaldist;
						rcx5=rcx1*mindist/totaldist + rcx2*crossdist2/totaldist+
							rcx3*crossdist3/totaldist + rcx4*crossdist4/totaldist;
						if((fli1<0.0 || fli2<0.0) && (fli3<0.0 || fli4<0.0))
							fli5*=-1.0;
					}
				}
			}
		}
		SetPerimeter2(writecount++, xpte, ypte, ros5, fli5, rcx5);
		if(CrossFound)      // if crosses
		{    if(testdist!=0.0)
			{    pointcountn=CrossNext;
				CrossLastN=pointtoward;       // switch crossed spans
				pointtoward=pointcount;
				pointcountl=pointcount=CrossSpan;
				CrossLastL=pointtoward;
			}
			else			// if crosses last time but not now
			{    CrossFound=0;
				CrossLastL=-1;//CrossSpan=-1;
				CrossLastN=-1;//CrossNext=-1;
				pointcount=pointtoward;
				if(Reverse) // if(Direction)
				{	pointcountl=pointcount+1;
					pointcountn=pointcount-1;
				}
				else
				{	pointcountl=pointcount-1;
					pointcountn=pointcount+1;
				}
				Direction=Reverse;
				if(pointcountn==numpts)
					pointcountn=0;
				else if(pointcountn<0)
					pointcountn=numpts-1;
				if(pointcountl==numpts)
					pointcountl=0;
				else if(pointcountl<0)
					pointcountl=numpts-1;
			}
		}
		else				// if no crosses last & this time through
		{    if(Direction)
			{	pointcountn--;
				pointcountl=pointcount;
				pointcount--;
			}
			else
			{	pointcountn++;
				pointcountl=pointcount;
				pointcount++;
			}
			if(pointcountn==numpts)
				pointcountn=0;
			else if(pointcountn<0)
				pointcountn=numpts-1;
			if(pointcount==numpts)
				pointcount=0;
			else	if(pointcount<0)
				pointcount=numpts-1;
		}
		xptn=GetPerimeter1Value(CurrentFire, pointcountn, XCOORD);
		yptn=GetPerimeter1Value(CurrentFire, pointcountn, YCOORD);
		xptl=xpt;
		yptl=ypt;
		xpt=xpte;
		ypt=ypte;
		ros1=ros5;
		fli1=fli5;
		if((pointcount==0 && !CrossFound) || writecount>=writelimit)
			Terminate=true;
	}while(!Terminate);
	SetNumPoints(CurrentFire, writecount-1);
//	rediscretize(&CurrentFire);
}


bool StandardizePolygon::Cross(double xpt1, double ypt1, double xpt2, double ypt2,
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


void StandardizePolygon::DensityControl(long CurrentFire)
{
     long i;
	long NewPts=0, NumPts=GetNumPoints(CurrentFire);
	double xpt, ypt, xptn, yptn, newx, newy, newrcx;
     double ros, fli, rosn, flin, newr, newf, rcx, rcxn;
     double dist, diffx, diffy, testdist2;

	testdist2=pow2(GetDistRes()*MetricResolutionConvert());
     if(testdist2==0)
     {    SwapFirePerims(-1, CurrentFire);
          return;
     }

     if(testdist2>25000.0)
     	testdist2=25000.0;
	AllocPerimeter2(2*NumPts);
     xpt=GetPerimeter1Value(CurrentFire, NumPts-1, XCOORD);
     ypt=GetPerimeter1Value(CurrentFire, NumPts-1, YCOORD);
     ros=GetPerimeter1Value(CurrentFire, NumPts-1, ROSVAL);
     fli=GetPerimeter1Value(CurrentFire, NumPts-1, FLIVAL);
     rcx=GetPerimeter1Value(CurrentFire, NumPts-1, RCXVAL);
     for(i=0; i<NumPts; i++)
     {	xptn=GetPerimeter1Value(CurrentFire, i, XCOORD);
          yptn=GetPerimeter1Value(CurrentFire, i, YCOORD);
     	rosn=GetPerimeter1Value(CurrentFire, i, ROSVAL);
          flin=GetPerimeter1Value(CurrentFire, i, FLIVAL);
          rcxn=GetPerimeter1Value(CurrentFire, i, RCXVAL);
		diffx=pow2(xpt-xptn);
          diffy=pow2(ypt-yptn);
		dist=diffx+diffy;
          if(dist>testdist2)
          {	newx=xpt-(xpt-xptn)*0.5;
          	newy=ypt-(ypt-yptn)*0.5;
               rosn=GetPerimeter1Value(CurrentFire, i, ROSVAL);
               flin=GetPerimeter1Value(CurrentFire, i, FLIVAL);
			newr=(ros+rosn)/2.0;
               newf=(fli+flin)/2.0;
               newrcx=(rcx+rcxn)/2.0;
               SetPerimeter2(NewPts++, newx, newy, newr, newf, newrcx);
          }
          SetPerimeter2(NewPts++, xptn, yptn, rosn, flin, rcxn);
          xpt=xptn;
          ypt=yptn;
          ros=rosn;
          fli=flin;
     }
	SetNumPoints(CurrentFire, NewPts);
/*

	long i, j, k, m;//count, count2, count3, count4;
	long NumPts=GetNumPoints(CurrentFire);
	double testdist1, testdist2, propdist;
	double xpt1, ypt1, xpt2, ypt2, xpt3, ypt3;
	double ros1, fli1, ros2, fli2;
	double newx, newy, newr, newf;

	//FreePerimeter2();
	AllocPerimeter2(2*NumPts);
	xpt1=GetPerimeter1Value(CurrentFire, 0, XCOORD);
	ypt1=GetPerimeter1Value(CurrentFire, 0, YCOORD);
	ros1=GetPerimeter1Value(CurrentFire, 0, ROSVAL);
	fli1=GetPerimeter1Value(CurrentFire, 0, FLIVAL);
	j=0;//count2=0;
	for(i=0; i<NumPts; i++)
	{    SetPerimeter2(j++, xpt1, ypt1, ros1, fli1);
		k=i+1;
		m=i+2;
		if(m==NumPts)
			m=0;
		else if(k==NumPts)
		{	k=0;
			m=1;
		}
		xpt2=GetPerimeter1Value(CurrentFire, k, XCOORD);
		ypt2=GetPerimeter1Value(CurrentFire, k, YCOORD);
		ros2=GetPerimeter1Value(CurrentFire, k, ROSVAL);
		fli2=GetPerimeter1Value(CurrentFire, k, FLIVAL);
		xpt3=GetPerimeter1Value(CurrentFire, m, XCOORD);
		ypt3=GetPerimeter1Value(CurrentFire, m, YCOORD);
		testdist1=sqrt(pow2(xpt1-xpt2)+pow2(ypt1-ypt2));
		testdist2=sqrt(pow2(xpt2-xpt3)+pow2(ypt2-ypt3));
		if(testdist2<testdist1/2.0)
		{    propdist=testdist2/testdist1;
			newx=xpt2-(xpt2-xpt1)*propdist;
			newy=ypt2-(ypt2-ypt1)*propdist;
			newr=ros2*propdist+ros1*(1.0-propdist);
			newf=fli2*propdist+fli1*(1.0-propdist);
			SetPerimeter2(j++, newx, newy, newr, newf);
		}
		xpt1=xpt2;
		ypt1=ypt2;
	}
	SetNumPoints(CurrentFire, j);
*/
}


/*void StandardizePolygon::DensityControl(long CurrentFire)
{
	long count, count2, count3;
	long NumPts=GetNumPoints(CurrentFire);
	double testdist, mindist, newdist, totaldist=0.0, propdist;
	double xpt1, ypt1, xpt2, ypt2;
	double ros1, fli1, ros2, fli2;
	double newx, newy, newr, newf;
	double SegmentLength, SegDist, NumSegments, WholeSegments;

	xpt2=GetPerimeter1Value(CurrentFire, NumPts-1, 0);
	ypt2=GetPerimeter1Value(CurrentFire, NumPts-1, 1);
	xpt1=GetPerimeter1Value(CurrentFire, 0, 0);
	ypt1=GetPerimeter1Value(CurrentFire, 0, 1);
	mindist=totaldist=20.0; //sqrt(pow2(xpt1-xpt2)+pow2(ypt1-ypt2));
	for(count=1; count<NumPts; count++)
	{    xpt2=GetPerimeter1Value(CurrentFire, count, 0);
		ypt2=GetPerimeter1Value(CurrentFire, count, 1);
		testdist=sqrt(pow2(xpt1-xpt2)+pow2(ypt1-ypt2));
		if(testdist<mindist)
			mindist=testdist;
		totaldist+=testdist;
		xpt1=xpt2;
		ypt1=ypt2;
	}
	FreePerimeter2();
	AllocPerimeter2(totaldist/mindist+NumPts);
	xpt1=GetPerimeter1Value(CurrentFire, 0, 0);
	ypt1=GetPerimeter1Value(CurrentFire, 0, 1);
	ros1=GetPerimeter1Value(CurrentFire, 0, 2);
	fli1=GetPerimeter1Value(CurrentFire, 0, 3);
	count2=0;
	for(count=1; count<=NumPts; count++)
	{    SetPerimeter2(count2++, xpt1, ypt1, ros1, fli1);
		if(count==NumPts)
		{	xpt2=GetPerimeter1Value(CurrentFire, 0, 0);
			ypt2=GetPerimeter1Value(CurrentFire, 0, 1);
			ros2=GetPerimeter1Value(CurrentFire, 0, 2);
			fli2=GetPerimeter1Value(CurrentFire, 0, 3);
		}
		else
		{	xpt2=GetPerimeter1Value(CurrentFire, count, 0);
			ypt2=GetPerimeter1Value(CurrentFire, count, 1);
			ros2=GetPerimeter1Value(CurrentFire, count, 2);
			fli2=GetPerimeter1Value(CurrentFire, count, 3);
		}
		testdist=sqrt(pow2(xpt1-xpt2)+pow2(ypt1-ypt2));
		NumSegments=testdist/mindist;
		if(NumSegments>=1.75)
		{    newdist=0.0;
			SegDist=modf(NumSegments, &WholeSegments);
			if(SegDist<=0.25)
				WholeSegments-=1.0;
			SegmentLength=testdist/WholeSegments;
			for(count3=0; count3<NumSegments; count3++)
			{    newdist+=SegmentLength;
				propdist=SegmentLength/testdist;
				newx=xpt1-(xpt1-xpt2)*propdist;
				newy=ypt1-(ypt1-ypt2)*propdist;
				newr=ros1*propdist+ros2*(1.0-propdist);
				newf=fli1*propdist+fli2*(1.0-propdist);
				SetPerimeter2(count2++, newx, newy, newr, newf);
			};
		}
		xpt1=xpt2;
		ypt1=ypt2;
		ros1=ros2;
		fli1=fli2;
	}
	SetNumPoints(CurrentFire, count2);
}
*/

/*

void Intersections::ReverseSpikeIntersectionOrder(long CurrentFire)
{
	double xpt1, ypt1; //, xpt2, ypt2;
	double ros1, fli1; //ros2,  fli2;
	long count, count2, count3, count4, count5, numsame;
	long samecheck1, ycheck; //, samecheck2
	double samepoint1x=0, samepoint1y=0;//, samepoint2x=0, samepoint2y=0;
	double distmin, distcalc, xdiff, ydiff;

	typedef struct
	{
		long order;
		long number;
		long ycheck;
		double xpt, ypt, ros, fli;
		double dist;
	}  Distances;
	Distances* distances;

	for(count=0; count<numcross; count++)
	{	samecheck1=GetSpan(count, 0);
		count2=count+1;
		while(samecheck1==GetSpan(count2, 0))
			count2++;
		numsame=count2-count;
		if(numsame>1)
		{	distances=(Distances *) calloc(numsame, sizeof(Distances));
			xpt1=GetPerimeter1Value(CurrentFire, samecheck1, 0);
			ypt1=GetPerimeter1Value(CurrentFire, samecheck1, 1);
			for(count3=count; count3<count2; count3++)
			{	GetInterPointCoord(count3, &samepoint1x, &samepoint1y);
				GetInterPointFChx(count3, &ros1, &fli1);
				ycheck=GetSpan(count3, 1);
				xdiff=pow2(xpt1-samepoint1x);
				ydiff=pow2(ypt1-samepoint1y);
				distcalc=xdiff+ydiff;
				count4=count3-count;
				distances[count4].number=count3;
				distances[count4].dist=distcalc;
				distances[count4].xpt=samepoint1x;
				distances[count4].ypt=samepoint1y;
				distances[count4].ros=ros1;
				distances[count4].fli=fli1;
				distances[count4].ycheck=ycheck;
				if(count3==count)
				{	distmin=distcalc;
					distances[count4].order=1;
				}
				else if(distcalc<distmin)
				{	distmin=distcalc;
					distances[count4].order=1;
					for(count5=0; count5<count4-1; count5++)
					{	distances[count5].order+=1;
					}
				}
			}
			count5=1;
			for(count3=count; count3<count2; count3++)
			{    for(count4=0; count4<numsame; count4++)
				{	if(distances[count4].order==count5)
					{	SetIntersection(count3, samecheck1, distances[count4].ycheck);
						SetInterPoint(count3, distances[count4].xpt, distances[count4].ypt,
									  distances[count4].ros, distances[count4].fli);
						count5++;
						break;
					}
				}
			}
			free(distances);
		}
		count=count2-1;
	}
}

*/



/*
void Intersections::InfuseIntersectionsIntoPerimeter(CurrentFire)
{
	double xpt1, ypt1, ros1, fli1;
	double xpt1n, ypt1n, ros1n, fli1n;
	double xpta, ypta, rosa, flia;
	long pointcount, pointcountn=1, intercount=0;
	long xcount=0, xcount1, xcount2;
	long numpts=GetNumPoints(CurrentFire);

	FreePerimeter2();
	AllocPerimeter2(numpts+numcross);
	ReverseSpikeIntersectionOrder(CurrentFire);

	xpt1=GetPerimeter1Value(CurrentFire, pointcount, 0);
	ypt1=GetPerimeter1Value(CurrentFire, pointcount, 1);
	ros1=GetPerimeter1Value(CurrentFire, pointcount, 2);
	fli1=GetPerimeter1Value(CurrentFire, pointcount, 3);

	for(pointcount=0; pointcount<numpts; pointcount++)
	{    xpt1n=GetPerimeter1Value(CurrentFire, pointcountn, 0);
		ypt1n=GetPerimeter1Value(CurrentFire, pointcountn, 1);
		ros1n=GetPerimeter1Value(CurrentFire, pointcountn, 2);
		fli1n=GetPerimeter1Value(CurrentFire, pointcountn, 3);
		SetPerimeter2(pointcount+intercount, xpt1, ypt1, ros1, fli1);

		while(xcount1<=pointcount)
		{	GetIntersection(xcount, &xcount1, &xcount2);
			xcount++;
			if(xcount1==pointcount)
			{    intercount++;
				GetInterPointCoord(xcount, &xpta, &ypta);
				GetInterPointFChx(xcount, &rosa, &flia);
				SetPerimeter2(pointcount+intercount, xpt1, ypt1, ros1, fli1);
			}
		}
		xpt1=xpt1n;
		ypt1=ypt1n;
		ros1=ros1n;
		fli1=fli1n;
		pointcountn++;
	}
}

*/


