/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: flam_nn.h
* Desc: Nearest Neighbor class used to fill in missing values in
*        and elevation grid
*
* Date: 4-5-08
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/

/*.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
/*                         Nearest Neighbor Class                            */
/* To Use Class:                                                             */
/*  - use Init() function                                                    */
/*  - specify number of rows and columns                                     */
/*  - give address of a single demension of array,                           */
/*  - specify what the missing value is                                      */
/*  - call Fill_Mngr () - see notes in function heading                      */
/*                                                                           */
/*                                                                           */
#define e_AvgFil     5          /* siz of area to take average on     */
#define e_SweepAvg  15          /* siz of area to avg in the Sweep procedur  */
#define e_MaxMis (double) 0.99    /* max % of missing values allowed in grid  */

/*.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
typedef class c_NN {

public:
   int row;                     /* Rows in grid                             */
   int col;                     /* Cols in grid                             */

   double *dr;                   /* adr of single dim array grid             */

   double  d_MisVal;              /* Missing Value                            */

   int  filled;                 /* cnt how many missin vals get filled      */
   int  passes;                 /* passes taken to do all filling in        */

   char  cr_Message[500];

          c_NN();
   int    Fill_Mngr  ();
   double Average (int i_Row, int i_Col, int i_Span);
   int    Random   ();
   void   AvgFill  (int r, int c, int i_Span);
   int    GetCenter (int *ar, int *ac);
   int    Init (int r, int c, double *dr, double d_MisVal);
   int    Sweep ();

   double GetCell (int r, int c);
   int   PutCell (int r, int c, double f);
   int   CountMissing ();
   void   Display ();
   void   CutIt ();
   int    DumpTest (float f_XLLCORNER,float f_YLLCORNER,float f_CELLSIZE, char cr_FN[] );

  } d_NN;
