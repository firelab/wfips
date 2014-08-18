/******************************************************************************
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Random Number Generator
 * Author:   Kyle Shannon <kyle@pobox.com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT 
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105 
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT 
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES 
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER 
 * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY, 
 * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

//------------------------------------------------------------------------------
//
//  Random Number Generator from Recipies in C, page 282
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#include "omffr_rand.h"

Random::Random()
{
     idum2=123456789;
     iy=0;
     memset(iv, 0x0, 32*sizeof(long));;

        ResetRandomSeed();
     // printf("random numbers initialized\n"); // initialize random num gen
     //rand2(&idum);
}


void Random::SetFixedSeed(long Number)
{
        //if(Number>0)
        //      Number*=-1;
        idum=Number;
        //rand2(&idum);
}


void Random::ResetRandomSeed()
{
        t = time(NULL);
            //ftime(&t);
        //srand(t.time+t.millitm);
        srand(t);
     idum=-((rand()%10000)+1);
}
        


long Random::GetCurrentSeed()
{
        return idum;
}


double Random::rand3()
{
        return rand2(&idum);
}


double Random::rand2(long *idum)
{
        long j, k;
     //static long idum2=123456789;
     //static long iy=0;
     //static long iv[NTAB];
     double temp;

     if(*idum<=0)
     {  if(-(*idum)<1)
                *idum=1;
        else
                *idum=-(*idum);
                idum2=(*idum);

          for(j=NTAB+7; j>=0; j--)
          {     k=(*idum)/IQ1;
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

