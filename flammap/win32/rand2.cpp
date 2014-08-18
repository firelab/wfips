//------------------------------------------------------------------------------
//
//  Random Number Generator from Recipies in C, page 282
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


//#include <stdio.h>
//#include <stdlib.h>
//#include <conio.h>
#include <math.h>

#define IM1 2147483563
#define IM2 2147483399
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014
#define IA2 40692
#define IQ1 53668
#define IQ2 52774
#define IR1 12211
#define IR2 3791
#define NTAB 32
#define NDIV (1+IMM1/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)
/*
double rand2(long *idum);


void main()
{
	long i, idum;
     double result;

     idum=-1;
	for(i=0; i<40; i++)
     {	result=rand2(&idum);
		printf("%s %lf\n", "Random Number= ", result);
     }
     getch();
}
*/

double rand2(long *idum)
{
	long j, k;
     static long idum2=123456789;
     static long iy=0;
     static long iv[NTAB];
     double temp;

     if(*idum<=0)
     {	if(-(*idum)<1)
     		*idum=1;
     	else
          	*idum=-(*idum);
		idum2=(*idum);

          for(j=NTAB+7; j>=0; j--)
          {	k=(*idum)/IQ1;
          	*idum=IA1*(*idum-k*IQ1)-k*IR1;
               if(*idum<0)
               	*idum+=IM1;
               if(j<NTAB)
               	iv[j]=*idum;

          }
          iy=iv[0];
     }
     k=(*idum)/IQ1;
     *idum=IA1*(*idum-k*IQ1)-k*IR1;
     if(*idum<0)
     	*idum+=IM1;
	k=idum2/IQ2;
     idum2=IA2*(idum2-k*IQ2)-k*IR2;
     if(idum2<0)
     	idum2+=IM2;
     j=iy/NDIV;
     if(j<0)
          j=labs(j);
     iy=iv[j]-idum2;
	iv[j]=*idum;
     if(iy<1)
     	iy+=IMM1;
     if((temp=AM*iy)>RNMX)
     	return RNMX;

	if(temp<0.0)
     	temp*=-1.0;

     return temp;
}







