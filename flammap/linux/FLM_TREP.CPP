/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: flm_trep.cpp
* Desc: FlamMap Text Report - Elevation, Aspect, etc distributions.
*
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/

#include "stdafx.h"
#include <stdio.h>
#include <math.h>

#include "flammap3.h"
#include "fsxsync.h"
#include "FlamMap_DLL.h"
//#include <atlimage.h>
//#include <AtlBase.h>
//#include <AtlConv.h>
//#include "pdflib.hpp"
//#include "Histogram.h"
#include <iostream>


#define max(a,b)      (((a) > (b)) ? (a) : (b))

#define min(a,b)      (((a) < (b)) ? (a) : (b))

using namespace std;

void Heading (FILE *out, FlamMap *fm);
void Distribution (char Name[], int num, int Hi, int Lo, int Values[],  FILE *out, FlamMap *fm, int DATA);
void Fuel_Distribution (FlamMap *fm, FILE *out, IndexItem *items);
void  FMS_Distribution (char Name[], int num, int Hi, int Lo,
                   int Values[],  FILE *out, FlamMap *fm, int fuel, int DATA  );

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: CritiqueTXT
* Desc: create output text report, Distributions
*
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
int FlamMap::CritiqueTXT(char *FileName)
{
FILE *out;
short hi, lo;
int nEbins = 0, nSBins = 0, nAbins = 0, nCbins = 0, nHbins = 0, nBbins = 0; 
int nPbins = 0, nDbins = 0, nWbins = 0;
int numFuel = Header.numfuel;
celldata cell;
crowndata cfuel;
grounddata gfuel;
int *tMins = NULL, *tMaxs = NULL;
int nVals, fuel;
double avg1, avg2;

   if (!LandFileOpen)
       return 0;

   out = fopen(FileName, "wt");
   if (!out)
     return 0;

   Heading (out, this ); 

   Distribution ("Elevation", Header.numelev,  Header.hielev,  Header.loelev,  Header.elevs, out, this, E_DATA);
   Distribution ("Slope",     Header.numslope, Header.hislope, Header.loslope, Header.slopes,out, this, S_DATA);
   Distribution ("Aspect",    Header.numaspect,Header.hiaspect,Header.loaspect,Header.aspects,out, this, A_DATA);	

   numFuel = Header.numfuel;
   IndexItem *items = new IndexItem[numFuel];

   Fuel_Distribution (this, out, items );
	Distribution ("Canopy Cover",    Header.numcover,Header.hicover,Header.locover,Header.covers, out, this, C_DATA);	
   if(this->HaveCrownFuels())
   {
	   Distribution ("Stand Height",    Header.numheight,Header.hiheight,Header.loheight,Header.heights, out, this, H_DATA);	
	   Distribution ("Crown Base Height",    Header.numbase,Header.hibase,Header.lobase,Header.bases, out, this, B_DATA);	
	   Distribution ("Crown Bulk Density",    Header.numdensity,Header.hidensity,Header.lodensity,Header.densities, out, this, P_DATA);	

   }
   if(HaveGroundFuels())
   {
	   Distribution ("Duff",    Header.numduff,Header.hiduff,Header.loduff,Header.duffs, out, this, D_DATA);	
	   Distribution ("Woody Fuels",    Header.numwoody,Header.hiwoody,Header.lowoody,Header.woodies, out, this, W_DATA);	
   }
   fprintf(out, "\n\nFuel Model Specific Distributions\n");

   for (int f = 0; f < numFuel; f++)   {
     fuel = Header.fuels[items[f].loc];
     fprintf(out, "\n***************************************************************\n\tFuel Model %d\n"
                     "***************************************************************\n", fuel);

     FMS_Distribution ("Elevation",
                   Header.numelev, Header.hielev, Header.loelev, Header.elevs,
                   out, this, fuel, E_DATA);

     FMS_Distribution ("Slope",
                   Header.numslope,Header.hislope,Header.loslope,Header.slopes,
                   out, this, fuel, S_DATA);

     FMS_Distribution ("Aspect",
                   Header.numaspect,Header.hiaspect,Header.loaspect,Header.aspects,
                   out, this, fuel, A_DATA);

     FMS_Distribution ("Canopy Cover",
                   Header.numcover, Header.hicover,Header.locover,Header.covers,
                   out, this, fuel, C_DATA);

    if ( HaveCrownFuels() ) {

       FMS_Distribution ( "Stand Height",
                     Header.numheight,Header.hiheight,Header.loheight,Header.heights,
                     out, this, fuel, H_DATA);

       FMS_Distribution ("Crown Base Height",
                     Header.numbase,Header.hibase,Header.lobase,Header.bases,
                     out, this, fuel, B_DATA);

       FMS_Distribution ("Crown Bulk Density",
                     Header.numdensity,Header.hidensity,Header.lodensity,Header.densities,
                     out, this, fuel, P_DATA);   }

    if ( HaveGroundFuels() ) {
       FMS_Distribution ( "Duff",
                     Header.numduff,Header.hiduff,Header.loduff,Header.duffs,
                     out, this, fuel, D_DATA);
 
       FMS_Distribution ( "Woody Fuels",
                     Header.numwoody,Header.hiwoody,Header.lowoody,Header.woodies,
                     out, this, fuel, W_DATA);   }
  }  /* for */ 

   delete[] items; 
   fclose(out);
	  return 1;
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: CT_Heading
* Desc: Put the text report heading to output file. 
*   In: out....opened file handle
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
void Heading (FILE *out, FlamMap *fm)
{
short hi, lo; 

   fprintf(out, "FlamMap Landscape File Critique\nLandscape File: %s\n", fm->GetLandFileName());
   fprintf(out, "\tLatitude: %ld\n", fm->GetLatitude());
   fprintf(out, "\tCell Resolution X: %.2f\tCell Resolution Y: %.2f\n", fm->GetCellResolutionX(), fm->GetCellResolutionX());
   fprintf(out, "\tNum Cells East: %ld\tNum Cells North: %ld\n", fm->GetNumEast(), fm->GetNumNorth());
   fprintf(out, "\tUTM North: %f\n\tUTM South: %f\n\tUTM East: %f\n\tUTM West: %f\n",
	               	 fm->GetNorthUtm(), fm->GetSouthUtm(), fm->GetEastUtm(), fm->GetWestUtm());

   fprintf(out, "\nThemes present\n"
	               	"\tTheme                Units      Range\n"
		                "\t--------------------------------------------------------\n");

   hi = fm->Header.hielev;
   lo = max(0, fm->Header.loelev);
   fprintf(out, "\tElevation     %12s  \t%d - %d\n", fm->GetTheme_UnitsString(E_DATA), lo, hi);


   hi = fm->Header.hislope;
   lo = max(0, fm->Header.loslope);
   fprintf(out, "\tSlope         %12s  \t%d - %d\n", fm->GetTheme_UnitsString(S_DATA), lo, hi);

   hi = fm->Header.hiaspect;
   lo = max(0, fm->Header.loaspect);
   fprintf(out, "\tAspect        %12s  \t%d - %d\n", fm->GetTheme_UnitsString(A_DATA), lo, hi);

   hi = fm->Header.hifuel;
   lo = max(0, fm->Header.lofuel);
   fprintf(out, "\tFuel          %12s  \t%d - %d\n", fm->GetTheme_UnitsString(F_DATA), lo, hi);

   hi = fm->Header.hicover;
   lo = max(0, fm->Header.locover);
   fprintf(out, "\tCanopy Cover  %12s  \t%d - %d\n", fm->GetTheme_UnitsString(C_DATA), lo, hi);

   if ( fm->HaveCrownFuels()) {
     hi = fm->Header.hiheight;
     lo = max(0, fm->Header.loheight);
     fprintf(out, "\tStand Height  %12s  \t%d - %d\n", fm->GetTheme_UnitsString(H_DATA), lo, hi);

     hi = fm->Header.hibase;
     lo = max(0, fm->Header.lobase);
     fprintf(out, "\tBase Height   %12s  \t%d - %d\n", fm->GetTheme_UnitsString(B_DATA), lo, hi);

     hi = fm->Header.hidensity;
     lo = max(0, fm->Header.lodensity);
     fprintf(out, "\tBulk Density  %12s  \t%d - %d\n", fm->GetTheme_UnitsString(P_DATA), lo, hi);}  

   if ( fm->HaveGroundFuels()) {
      hi = fm->Header.hiduff;
      lo = max(0, fm->Header.loduff);
      fprintf(out, "\tDuff          %12s  \t%d - %d\n", fm->GetTheme_UnitsString(D_DATA), lo, hi);

      hi = fm->Header.hiwoody;
      lo = max(0, fm->Header.lowoody);
      fprintf(out, "\tWoody Fuels   %12s  \t%d - %d\n", fm->GetTheme_UnitsString(W_DATA), lo, hi); }
    
   fprintf(out, "\n");
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: Distribution
* Desc: Put a single distribution table to the report 
*
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
void  Distribution (char Name[], int num, int Hi, int Lo, int Values[],  FILE *out, FlamMap *fm, int DATA)
{
int nVals; 
int *tMins, *tMaxs, e1, e2;
celldata cell;
crowndata cfuel;
grounddata gfuel;

    if ( num > 10 || num < 0) {
       nVals = 10;
       tMins = new int[nVals + 1];
       tMaxs = new int[nVals + 1];
       int lo = max(0, Lo), hi = Hi;
       tMins[0] = 0;
       tMaxs[0] = 0;
       for ( int i = 1; i <= nVals; i++)  
	   {
          if ( A_DATA == DATA) 
		  {
            tMins[i] = ( i - 1) * 36;      
            tMaxs[i] = i * 36; 
		  } 
          else 
		  {
            tMins[i] = lo + ((i - 1) * ((hi - lo)/9));
            tMaxs[i] = lo + ((i) * ((hi - lo)/9)); 
		  } 
	   }  
	}
    else  
	{
       nVals = num;
       tMins = new int[nVals + 1];
       tMaxs = new int[nVals + 1];
       tMins[0] = 0;
       tMaxs[0] = 0;
       for ( int i = 1; i <= nVals; i++)  
	   {
          tMins[i] = tMaxs[i] = Values[i];
	   }  
	}

     //CrossTab *tStats = new CrossTab(nVals, tMins, tMaxs);
     int64_t nCells = (int64_t)fm->GetNumEast() * (int64_t)fm->GetNumNorth();
   /*   for ( int64_t p = 0; p < nCells; p++)    
	  {
         fm->GetCellDataFromMemory(p, cell, cfuel, gfuel);
		 switch(DATA)
		 {
		 case E_DATA:
            tStats->AddObs(cell.e); 
			break;
		 case S_DATA:
            tStats->AddObs(cell.s);
 			break;
		 case A_DATA:
            tStats->AddObs(cell.a);
 			break;
		 case F_DATA:
            tStats->AddObs(cell.f);
			break;
		 case C_DATA:
            tStats->AddObs(cell.c);
			break;
		 case H_DATA:
            tStats->AddObs(cfuel.h);
			break;
		 case B_DATA:
            tStats->AddObs(cfuel.b);
			break;
		 case P_DATA:
            tStats->AddObs(cfuel.p);
			break;
		 case D_DATA:
            tStats->AddObs(gfuel.d);
			break;
		 case W_DATA:
            tStats->AddObs(gfuel.w);
			break;
		 }
      }

	  fprintf(out, "\n%s distribution\n", Name);
	  fprintf(out, "\t%9s               Frequency        Percent      Overall Percent\n"
              		"\t---------------------------------------------------------------------\n",Name);

	  for (int v = 0; v < nVals; v++)	
	  {
		   e1 = tMins[v + 1];
		   e2 = tMaxs[v + 1];
		   fprintf(out, "\t %6ld - %6ld  %10ld\t\t%5.2f\t\t%5.2f\n", 
               e1, e2, tStats->freqs[v + 1], 
  			tStats->nValidObs > 0 ? ((double)tStats->freqs[v + 1]) / ((double)tStats->nValidObs) * 100.0: 0.0,
 				tStats->nObs > 0 ? ((double)tStats->freqs[v + 1]) / ((double)tStats->nObs) * 100.0 : 0.0); 
	  }

	  fprintf(out, "\t     No Data      %10ld\t\t%5.2f\t\t%5.2f\n\n", tStats->freqs[0], 
	 	tStats->nValidObs > 0 ? ((double)tStats->freqs[0]) / ((double)tStats->nValidObs) * 100.0 : 0.0,
	 	tStats->nObs > 0 ? ((double)tStats->freqs[0]) / ((double)tStats->nObs) * 100.0 : 0.0);
  	delete tStats;ALM */
  	delete[] tMins;
  	delete[] tMaxs;
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: Fuel_Distribution
* Desc: Fuel Distribution section of report. This part of the report is 
*        done different and uses a sort. 
*
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
void Fuel_Distribution (FlamMap *fm, FILE *out, IndexItem *items)
{
int nVals; 
celldata cell;
crowndata cfuel;
grounddata gfuel;
int64_t nCells;
int numFuel;
    
    numFuel = fm->Header.numfuel;
    //CrossTab fStats (numFuel, fm->Header.fuels, fm->Header.fuels);
    nCells = (int64_t)fm->GetNumEast() * (int64_t)fm->GetNumNorth();

    for ( int64_t p = 0; p < nCells; p++)    {
       fm->GetCellDataFromMemory (p, cell, cfuel, gfuel);
     //  fStats.AddObs(cell.f);  
	}
	/*
    for ( int f = 1; f < fStats.nBins; f++) {
      items[f - 1].freq = fStats.freqs[f];
      items[f - 1].loc = f;   }
	  */
    qsort((void *)items, (size_t)numFuel, sizeof(IndexItem), IndexCompare);
    fprintf(out, "\nFuels distribution\n\tFuel Type     Frequency        Percent      Overall Percent\n"
									   "\t-----------------------------------------------------------\n");
/*ALM    for ( int f = 0; f < numFuel; f++)     {
      fprintf(out, "\t %ld\t  %10ld\t\t%5.2f\t\t%5.2f\n", fm->Header.fuels[items[f].loc], fStats.freqs[items[f].loc],
               fStats.nValidObs > 0 ? ((double)fStats.freqs[items[f].loc]) / ((double)fStats.nValidObs) * 100.0: 0.0,
               fStats.nObs > 0 ? ((double)fStats.freqs[items[f].loc]) / ((double)fStats.nObs) * 100.0 : 0.0);
    }

    fprintf(out, "\t No Data %10ld\t\t%5.2f\t\t%5.2f\n", fStats.freqs[0],
 		fStats.nValidObs > 0 ? ((double)fStats.freqs[0]) / ((double)fStats.nValidObs) * 100.0 : 0.0,
 		fStats.nObs > 0 ? ((double)fStats.freqs[0]) / ((double)fStats.nObs) * 100.0 : 0.0);
}
*/
}
/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: FMS_Distribution
* Desc: Do a single Fuel Model Specific Distribution section of the report
*       This will do an Elevation, Slope, etc, see below 
*   In: see calls to this function
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
void  FMS_Distribution (char Name[], int num, int Hi, int Lo,
                   int Values[],  FILE *out, FlamMap *fm,
                   int fuel, int DATA  )
{
int nVals; 
int *tMins, *tMaxs, e1, e2;
celldata cell;
crowndata cfuel;
grounddata gfuel;
double avg1, avg2;

    if ( num > 10 || num < 0) {
        nVals = 10;
        tMins = new int[nVals + 1];
        tMaxs = new int[nVals + 1];
        int lo = max(0, Lo), hi = Hi;
        tMins[0] = 0;
        tMaxs[0] = 0;
        for ( int i = 1; i <= nVals; i++){
           if ( !strcasecmp ( Name,"Aspect" )) {
             tMins[i] = ( i - 1) * 36;      
             tMaxs[i] = i * 36; }         
           else {
             tMins[i] = lo + ((i - 1) * ((hi - lo)/9));
             tMaxs[i] = lo + ((i) * ((hi - lo)/9)); } 
         } /* For i */
     }  /* if */ 

    else   {
      nVals = num;
      tMins = new int[nVals + 1];
      tMaxs = new int[nVals + 1];
      tMins[0] = 0;
      tMaxs[0] = 0;
      for(int i = 1; i <= nVals; i++)       {
         tMins[i] = tMaxs[i] = Values[i];  }   }

   //CrossTab *tStats = new CrossTab(nVals, tMins, tMaxs);

   int64_t nCells = (int64_t)fm->GetNumEast() * (int64_t)fm->GetNumNorth();
   /*ALM
    for ( int64_t p = 0; p < nCells; p++)   {
       fm->GetCellDataFromMemory(p, cell, cfuel, gfuel);
       if ( cell.f != fuel)
         continue;
       if ( !_stricmp (Name,"Elevation") )
          tStats->AddObs(cell.e);
       if ( !_stricmp (Name,"Slope"))
         tStats->AddObs(cell.s);
       if ( !_stricmp (Name,"Aspect"))
         tStats->AddObs(cell.a);
       if ( !_stricmp (Name,"Canopy Cover"))
         tStats->AddObs(cell.c);
       if ( !_stricmp (Name,"Stand Height"))
         tStats->AddObs(cfuel.h);
       if ( !_stricmp (Name,"Crown Base Height"))
         tStats->AddObs(cfuel.b);
       if ( !_stricmp (Name,"Crown Bulk Density"))
         tStats->AddObs(cfuel.p);
       if ( !_stricmp (Name,"Duff"))
         tStats->AddObs(gfuel.d);
       if ( !_stricmp (Name,"Woody Fuels"))
         tStats->AddObs(gfuel.w);
     }
	 */
    fprintf(out, "\t%s Range Distribution (Fuel Model %d, %s)\n", Name, fuel, fm->GetTheme_UnitsString(DATA));
    fprintf(out, "\t%-20s  Frequency        Percent      Overall Percent\n"
                 "\t---------------------------------------------------------------------\n",Name);
    for (int v = 0; v < nVals; v++)  {
      e1 = tMins[v + 1];
      e2 = tMaxs[v + 1];
   /*ALM   fprintf(out, "\t %6ld - %6ld  %10ld\t\t%5.2f\t\t%5.2f\n", e1, e2, tStats->freqs[v + 1],
                   tStats->nValidObs > 0 ? ((double)tStats->freqs[v + 1]) / ((double)tStats->nValidObs) * 100.0: 0.0,
                   tStats->nObs > 0 ? ((double)tStats->freqs[v + 1]) / ((double)tStats->nObs) * 100.0 : 0.0);
   }

   fprintf(out, "\t     No Data      %10ld\t\t%5.2f\t\t%5.2f\n", tStats->freqs[0],
                 tStats->nValidObs > 0 ? ((double)tStats->freqs[0]) / ((double)tStats->nValidObs) * 100.0 : 0.0,
                 tStats->nObs > 0 ? ((double)tStats->freqs[0]) / ((double)tStats->nObs) * 100.0 : 0.0);


    avg1 = tStats->Average();
    avg2 = tStats->OverallAverage();
    fprintf(out, "\t---------------------------------------------------------------------\n"
                 "\tAvg (valid obs only): %.1f  Overall Average: %.1f\n\n",
                 avg1, avg2);

    delete tStats;*/
	}
    delete[] tMins;
    delete[] tMaxs;
}
