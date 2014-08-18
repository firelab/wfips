/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: flam_nn.cpp
* Desc: NN nearest neighbot class
*       Used to fill in missing elevations in an elevation grid
*       See notes in flam_nn.h, c_NN class.
* Date: 4-5-08
*
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
#include   "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nn_def.h"


/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: Fill_Mngr
* Desc: See flam_nn.h for notes
*       Fill in missing cells in grid
*       Start in the middle with a 3x3 square of cells and work out, stop
*        when the entire square is outside the grid - lower lever functions
*        make sure nothing is done for requests outside the grid.
*       First some randow cells are checked and filled in, this helps to
*        smooth things out a little
*       At the end if there are any missing cells left a simple pass(s)
*        is done over the grid to fill in anything that might have been
*        missed or odd reason - like certain missing inner islands
* Note-1: If by chance there are any missing values still left in the array
*          Sweep() will take care of them.
*  Ret: 1 = ok, 0 error, see NN.cr_Message[]
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
int  c_NN::Fill_Mngr  ()
{
int rc,cc,r,c,Inc,i_Span;
int l,d;
float f, f_Cells, f_Miss;

   f_Cells = (float) (this->row * this->col);
   f_Miss = (float) CountMissing ();

			if ( f_Miss == 0 )                   /* No Missing values found in grid */
				 return 1; 

   if ( f_Miss == f_Cells ){            /* this would mean the entire array  */
     strcpy (this->cr_Message, "Grid has ALL missing values");
     return 0; }                        /* contains missing values - bummer  */

   f = f_Miss / f_Cells;                /* percent of miss vals in grid      */
   if ( f > e_MaxMis ) {
     sprintf (this->cr_Message, "%4.2f percent of grid is missing", f);
     return 0; }

   GetCenter (&rc,&cc);                 /* aprox centr point,non-mis in grid */

   Random ();                           /* fill some random cells with elev  */

   Inc = 0;
   i_Span = e_AvgFil;                   /* siz of cell area to average       */

/*.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
/* Start in center and work out with expanding square                        */
   while (1) {
     Inc++;
     if ( rc-Inc < 0 &&  rc+Inc >= this->row &&   /* End when all are outsid */
          cc-Inc < 0 &&  cc+Inc >= this->col )    /*  of gird                */
        break;
     r = rc-Inc;
     for ( c = cc-Inc; c <= cc+Inc; c++ )    /* go across up row             */
       AvgFill  (r, c,i_Span);

     c = cc+Inc;
     for ( r = rc-Inc; r <= rc+Inc; r++ )    /* go down right hand cols      */
       AvgFill  (r, c,i_Span);

     r = rc+Inc;
     for ( c = cc+Inc; c >= cc-Inc; c-- )    /* do lower row                 */
       AvgFill  (r, c,i_Span);

     c = cc-Inc;
     for ( r = rc+Inc; r >= rc-Inc; r-- )    /* go up left col               */
       AvgFill  (r, c,i_Span);

     this->passes++;                         /* Just cnt for the heck of it  */
   }  /* while */

   Sweep();                                  /* See Note-1 above    */
   return 1;
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: AvgFill
* Desc: Take average of surrounding cells and put the average in
*        the specified cell.
*   In: row,col
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
void c_NN::AvgFill  (int r, int c, int i_Span)
{
float f;

   if ( GetCell(r,c) != d_MisVal )      /* if not missing beat it            */
     return;

   f = Average(r,c,i_Span);
   if ( f != this->d_MisVal ){          /* if we couldn't get one            */
     if (PutCell (r, c, f))             /* fill in with avg                  */
       this->filled++;                  /* if filled within grid count it    */
   }
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: Average
* Desc: Take the average of the surrounding cells
*       Missing values are not used in averaging
*   In: Row,Col
*       i_Span...width/height of area to use
*  Ret: average or missing value if nothing found to average
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
double c_NN::Average (int i_Row, int i_Col, int i_Span)
{
int r,c, i_SRow, i_SCol, i_ERow, i_ECol;
float f, f_Tot, f_Cnt;

    f_Tot = f_Cnt = 0;

   i_SRow = i_Row - (i_Span / 2);
   i_SCol = i_Col - (i_Span / 2);
   i_ERow = i_SRow + i_Span;
   i_ECol = i_SCol + i_Span;

   for ( r = i_SRow; r < i_ERow; r++ ) {
     for ( c = i_SCol; c < i_ECol; c++ ) {
        f = GetCell(r,c);
        if ( f == this->d_MisVal )
          continue;

        f_Tot += f;
        f_Cnt++;
     }
   }

   if ( f_Cnt > 0 )
     f = f_Tot / f_Cnt;
   else
     f = this->d_MisVal;

   return f;
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: CountMissing
* Desc: Count the number of missing values in the array
*  Ret: number of missing found
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
int  c_NN::CountMissing ()
{
int r, c, l;
   l = 0;
   for ( r = 0; r < row; r++ ) {
     for ( c = 0; c < col; c++ ) {
       if ( GetCell(r,c) == d_MisVal )
          l++;  }  }
   return l;
}
/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
int  c_NN::PutCell (int r, int c, double d)
{
int l;

   if ( r < 0 || r >= this->row )
      return 0;

   if ( c < 0 || c >= this->col )
      return 0;
   l = (col * r) + c;
   this->dr[l] = d;
			return 1; 
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
double  c_NN::GetCell (int r, int c)
{
int l;
   if ( r < 0 || r >= this->row )
     return this->d_MisVal;

   if ( c < 0 || c >= this->col )
     return this->d_MisVal;

   l = (col * r) + c;
   return this->dr[l];

}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: Init
* Desc:
*   In: r,c...number of rows and cols
*       fr....pointer to one dimension arrary
*       d_MisVal....missing value to look for and replace
*                   this must be a value less than 0
*  Ret: 1 inputs ok, else 0
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
int c_NN::Init (int r, int c, double *dr, double d_MisVal)
{
   if ( r <= 0 ) return 0;
   if ( c <= 0 ) return 0;
   if ( d_MisVal > 0 ) return 0;

   this->row = r;
   this->col = c;
   this->dr = dr;
   this->d_MisVal = d_MisVal;

   this->filled = 0;
   this->passes = 0;
   return 1;
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name:
* Desc:
*
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
void c_NN::Display ()
{
int r, c;
float f;

    printf ("\n");

   for ( r = 0; r < row; r++ ) {
      for ( c = 0; c < col; c++ ) {
        f = GetCell (r,c);
        printf (" %4.1f ", f);
      }
    printf ("\n");
   }
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: c_NN
* Desc: constructor - inti stuff
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
c_NN::c_NN()
{
  this->filled = 0;
  this->passes = 0;
  strcpy (cr_Message,"");
}




















/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: Sweep
* Desc: Fill in missing grid values by doing a simple sweep across the
*        grid, not a very good way to do business but this
*        function only will get used if the onion has trouble
* Note-1: This condition should never happen but just in case it will
*         prevent and endless loop, usually a couple passes will get
*         all cells filled in.
*  Ret: 1 ok, else 0
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
int  c_NN::Sweep ()
{
int l,r,c, Cnt;
double d;

   Cnt = 0;

   l = CountMissing ();                 /* Get # of missing values in arry   */
   if ( l == this->row * this->col )    /* this would mean the entire array  */
     return 0;                          /* contains missing values - bummer  */

   while (1) {                          /* keep passing thur array until     */
     l = CountMissing ();               /*  all missing values are filled    */
     if ( l == 0)
       break;

     Cnt++;                             /* See Note-1 above                  */
     if ( Cnt == 10000 ) {
       strcat (this->cr_Message,"Logic Error 1");
       return 0; }

     this->passes++;                    /* Count number of passes we do      */

     for ( r = 0; r < this->row; r++ ) {
       for ( c = 0; c < this->col; c++ ) {

         if ( GetCell(r,c) != d_MisVal )
           continue;

           d = Average (r,c,e_SweepAvg);
           if ( d == this->d_MisVal )     /* if we couldn't get one            */
             continue;                    /*  skip it                          */
           if (PutCell (r, c, d))         /* fill in with avg                  */
              this->filled++;                /* count how many we do              */
      } }


   } /* while (1) */

  return 1;
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: GetCenter
* Desc: Get the row & col of cell that we beleive is in the middle of
*       non-missing values
*       by looking for the outer most non-missing cells
*       Assumption is that most grids will have islands of non-missing
*        values.
*  Ret:
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
int c_NN::GetCenter (int *ar, int *ac)
{
int r,c,r1,r2,c1,c2;
float d;

   r1 = -1;
   c1 = this->col + 1;
   c2 = -1;

   for ( r = 0; r < this->row; r++ ) {
     for ( c = 0; c < this->col; c++ ) {
       if ( GetCell(r,c) == this->d_MisVal )
         continue;                      /* skip any missing vals             */

       if (r1 == -1 ) r1 = r;           /* Get top most row                  */
       r2 = r;                          /* Get bot row                       */
       if ( c < c1 ) c1 = c;            /* Get leftmost col                  */
       if ( c > c2 ) c2 = c;            /* get right most col                */
     } }

  *ac = ((c2 - c1) / 2) + c1;           /* take dir then add too to find     */
  *ar = ((r2 - r1) / 2) + r1;           /*  center points                    */
  return 1;
}



/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: Random
* Desc: Hit a bunch of random cells, check for missing and attempt to
*        fill them in
*       This seems to help smooth things out.
*  Ret: 1 ok
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
int  c_NN::Random   ()
{
int i,r,c,i_Span;
int l,m,l_Iter;
double d;

/* Use approx 10 % of tot cells as how many randoms cells we'll do           */
   l_Iter = (this->row * this->col) / 10;

/* Set the siz of area we do averaging on                                    */
   i_Span = (this->row / 100) + (this->col / 100);
   if ( i_Span < 10)                    /* Minmum size we'll do              */
     i_Span = 10;

   for ( l = 0; l < l_Iter; l++ ) {
     r = ( rand () % this->row -1 );
     c = ( rand () % this->col -1 );
     if ( GetCell(r,c) != d_MisVal )
       continue;
     d = Average (r,c,i_Span);
     if ( d == this->d_MisVal )     /* if we couldn't get one            */
       continue;                    /*  skip it                          */
     if (PutCell (r, c, d))         /* fill in with avg                  */
        this->filled++;                /* count how many we do              */

   }

   return 1;
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: CutIt
* Desc: Used to test....
*       Put an inner island of missing values in a grid
*       I used this for testing.
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
void  c_NN::CutIt ()
{
int r,c, rs,re,cs,ce;
int rCen, cCen;

    GetCenter (&rCen, &cCen);

    rs =  rCen - this->row / 10;
    re =  rCen + this->row / 10;

    cs =  cCen - this->col / 10;
    ce =  cCen + this->col / 10;

   for ( r = 400; r < 550; r++ ) {
     for ( c = 700; c < 900; c++ ) {
        PutCell(r,c,this->d_MisVal);
    }}
}




/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: DumpTest
* Desc: For testing only - dump out the array to an .asc grid file
*       type in any file name to dump to
*
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
int c_NN::DumpTest (float f_XLLCORNER,float f_YLLCORNER,float f_CELLSIZE, char cr_FN[] )
{
int c, l,m, row,col; 
FILE *fh;

   fh = fopen  (cr_FN,"w");
   if ( fh == NULL ) {
     printf("Error opening file: \n");
     return 0; }

   fprintf(fh, "NCOLS        %ld\n",  this->col);
   fprintf(fh, "NROWS        %ld\n",  this->row);
   fprintf(fh, "XLLCORNER    %f\n",  f_XLLCORNER);
   fprintf(fh, "YLLCORNER    %f\n",  f_YLLCORNER);
   fprintf(fh, "CELLSIZE     %f\n",  f_CELLSIZE);
   fprintf(fh, "NODATA_VALUE %lf\n",  this->d_MisVal);

   m = (int)this->col * (int)this->row;
   c = 0;

  //printf ("%d %d \n", this->col, this->row);

   for ( row = this->row-1; row >= 0; row-- ) {
				for ( col = 0; col < this->col; col++ ) {
     fprintf (fh, "%10.6lf ", this->dr[row*this->col+col] );
     c++;
     if ( c == this->col ) {
       fprintf (fh,"\n");
       c = 0; }
     }
			}

//   printf ("Put Out: %ld\n", l);

   fclose (fh);
   return 1;

}
