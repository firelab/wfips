#include "stdafx.h"
#include <math.h>
//#include "nrutil.h"
#include "splinex.h"

/*
void spline(double x[], double y[], int n, double yp1, double ypn, double y2[])
{
	long i, k;
     double p, qn, sig, un, *u;

     u=vector(1, n-1);
     if(yp1>0.99e30)
     	y2[1]=u[1]=0.0;
     else
     {	y2[1]=-0.5;
		u[1]=(3.0/(x[2]-x[1]))*((y2[2]-y[1])/(x[2]-x[1])-yp1);
     }

     for(i=2; i<=n-1; i++)			// decompposition loop of the tridiagonal alorithm. y2
     {	sig=(x[i]-x[i-1])/(x[i+1]-x[i-1]);		// and u are used for temporary storage
     	p=sig*y2[i-1]+2;					// of the decomposed factors
          y2[i]=(sig-1.0)/p;
          u[i]=(y[i+1]-y[i])/(x[i+1]-x[i])-(y[i]-y[i-1])/(x[i]-x[i-1]);
		u[i]=(6.0*u[i]/(x[i+1]-x[i-1])-sig*u[i-1])/p;
     }

     if(ypn>0.99e30)		// the upper bounary condition is set either to be "natural"
          qn=un=0.0;		// or else to have a specified first derivative.
     else
	{	qn=0.5;
	     un=(3.0/(x[n]-x[n-1]))*(ypn-(y[n]-y[n-1])/(x[n]-x[n-1]));
     }

     y2[n]=(un-qn*u[n-1])/(qn*y2[n-1]+1.0);
     for(k=n-1; k>=1; k--)			// this si the backsubstitution loop of the tridiagonal algorithm
     	y2[k]=y2[k]*y2[k+1]+u[k];
	free_vector(u, 1, n-1);
}


void splint(double xa[], double ya[], double y2a[], long n, double x, double *y)
{
	// called after spline()

	void nrerror(char error_text[]);
     long klo, khi, k;
     double h, b, a;

     klo=1;
     khi=n;
     while(khi-klo>1)
     {	k=(khi+klo) >>1;
	     if(xa[k]>x)
          	khi=k;
          else
          	klo=k;
     }
     h=xa[khi]-xa[klo];
	if(h==0.0)
     	nrerror(" bad xa input to routine splint");
     a=(xa[khi]-x)/h;
     b=(x-xa[klo])/h;
     *y=a*ya[klo]+b*ya[khi]+((a*a*a-a)*y2a[klo]+(b*b*b-b)*y2a[khi])*(h*h)/6.0;
}


void polint(float xa[], float ya[], long n, float x, float *y, float *dy)
{
	long i, m, ns=1;
     float den, dif, dift, ho, hp, w;
     float *c, *d;

     dif=fabs(x-xa[1]);
     c=fvector(1, n);
     d=fvector(1, n);
     for(i=1; i<=n; i++)
     {	if((dift=fabs(x-xa[i]))<dif)
     	{	ns=i;
          	dif=dift;
          }
          c[i]=ya[i];
          d[i]=ya[i];
     }
     *y=ya[ns--];
     for(m=1; m<n; m++)
     {	for(i=1; i<=n-m; i++)
     	{	ho=xa[i]-x;
          	hp=xa[i+m]-x;
               w=c[i+1]-d[i];
               if((den=ho-hp) == 0.0)
               	nrerror("error in routine polint");
               den=w/den;
               d[i]=hp*den;
               c[i]=ho*den;
          }
          *y+=(*dy=(2*ns<(n-m) ? c[ns+1] : d[ns--]));
     }
     free_fvector(d, 1, n);
     free_fvector(c, 1, n);
}


#define TINY 1.0e-25
#define FREERETURN {free_vector(d, 1, n); free_vector(c, 1, n); return;}

void ratint(double xa[], double ya[], long n, double x, double *y, double *dy)
{
	long m, i, ns=1;
     double w, t, hh, h, dd, *c, *d;

     c=vector(1, n);
     d=vector(1, n);
     hh=fabs(x-xa[1]);
     for(i=1; i<=n; i++)
     {	h=fabs(x-xa[i]);
     	if(h==0.0)
          {	*y=ya[i];
          	*dy=0.0;
               FREERETURN;
          }
          else if(h<hh)
          {	ns=i;
          	hh=h;
          }
          c[i]=ya[i];
          d[i]=ya[i]+TINY;
     }
     *y=ya[ns--];
     for(m=1; m<n; m++)
     {	for(i=1; i<=n-m; i++)
     	{	w=c[i+1]-d[i];
          	h=xa[i+m]-x;
               t=(xa[i]-x)*d[i]/h;
               dd=t-c[i+1];
               if(dd==0.0)
               	nrerror("Error in routine ratint");
               dd=w/dd;
               d[i]=c[i+1]*dd;
               c[i]=t*dd;
          }
          *y+=(*dy=(2*ns <(n-m) ? c[ns+1] : d[ns--]));
     }
	FREERETURN;
}
*/

void locate(float xx[], unsigned long n, float x, unsigned long *j)
{
	unsigned long ju, jm, jl;
     long ascnd;

     jl=0;
     ju=n+1;
     ascnd=(xx[n]>xx[1]);
     while((ju-jl)>1)
     {	jm=(ju+jl) >>1;
     	if((x>xx[jm])==ascnd)
          	jl=jm;
          else
          	ju=jm;
     }
     *j=jl;
}




