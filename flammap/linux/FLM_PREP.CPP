/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: flm_prep.cpp
* Desc: FlamMap PDF report 
* Date: 8-25-08
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/

#include "stdafx.h"
#include <io.h>
#include <math.h>    

#include "flammap3.h"
#include "fsxsync.h"
#include "FlamMap_DLL.h"
//#include <atlimage.h>
#include <AtlBase.h>
#include <AtlConv.h>
#include "pdflib.hpp"
#include "Histogram.h"
#include <iostream>
using namespace std;

/*-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
int  PDF_AddPageNum(PDFlib *pdf, int pageNum, double x, double y, int font);
void PDF_HeadInfo (PDFlib *pdf, char A[], double a, char B[], double b, double y, char Fmt[]);
void PDF_Themes (PDFlib *pdf, double *y, char Label[], char Unit[], short hi, short low, double leading);

void PDF_Histogram (PDFlib *pdf, char cr_Theme[],  int i_num, short Hhi, short Hlo, long H_ary[],
                   FlamMap *fm, int boldfont, double x, double y, char gifname[], int DATA, int fuel  );

void  FuelHistogram (FlamMap *fm, double y, char gifName[], IndexItem items[]);

void PDF_ThemeImage (PDFlib *pdf, int DATA, char Theme[], int boldfont, char jpgName[], FlamMap *fm, int font, int *pageNum);
void PDF_FuelHistogram (PDFlib *pdf, FlamMap *fm, double y, char gifName[], IndexItem items[]);
void PDF_Mngr (PDFlib *pdf, FlamMap *fm, double *Y, char gifName[], int boldfont, double *d_SavY, int fuel );
void PDF_HeadInfo (PDFlib *pdf, char A[], double a, char B[], double b, double *y, char Fmt[], double leading);

#define COL1 100.0
#define COL2 200.0
#define COL3 300.0
#define COL4 400.0 

#define PNX  500
#define PNY  2

void  FuelHistogram (FlamMap *fm, double y, char gifName[], IndexItem items[])
{
CHistogram histogram;
double *pcts;
char **labels;
int i, numFuel, image;
celldata cell;
crowndata cfuel;
grounddata gfuel;
__int64 nCells;


   numFuel = fm->Header.numfuel;
   nCells = (__int64)fm->GetNumEast() * (__int64)fm->GetNumNorth();   
	  CrossTab fStats (numFuel, fm->Header.fuels, fm->Header.fuels);

   for ( __int64 p = 0; p < nCells; p++){
    		fm->GetCellDataFromMemory(p, cell, cfuel, gfuel);
		    fStats.AddObs(cell.f);}
 	 
   for (int f = 1; f < fStats.nBins; f++){
	    	items[f - 1].freq = fStats.freqs[f];
	    	items[f - 1].loc = f;	}
  
  	qsort((void *)items, (size_t)numFuel, sizeof(IndexItem), IndexCompare);
	  labels = new char *[numFuel + 1];
	  pcts = new double[numFuel + 1];

   for ( i = 0; i <= numFuel; i++)	{
	   	labels[i] = new char[64];
	   	if ( i == 0) {
		    	strcpy(labels[i], "ND");
		    	pcts[i] = ((double)fStats.freqs[0]) / ((double)fStats.nObs) * 100.0;		}
		   else {
		    	sprintf(labels[i], "%ld", fm->Header.fuels[items[i - 1].loc]);
		    	pcts[i] = ((double)fStats.freqs[items[i - 1].loc]) / ((double)fStats.nObs) * 100.0; }
	   }  /* for i */
	  
   histogram.CreateImage(gifName, numFuel + 1, pcts, "Fuels", "Percent", fm->GetTheme_UnitsString(F_DATA), labels, max(numFuel * 30, 300), 250);
	  for ( int i = 0; i <= numFuel; i++)	{
	   	delete[] labels[i]; }
	
   delete[] labels;
	  delete[] pcts;
	 
}


/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: CritiquePDF
* Desc: create the entire PDF report 
*
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
int FlamMap::CritiquePDF(char *FileName)
{



int  i;
int  nRetCode = 0;
char buf[256];
char jpgName[256], gifName[256];
int  image;
int  pageNum = 1, pnX = 500, pnY = 2;
double col1 = COL1, col2 = COL2, col3 = COL3, col4 = COL4;
short hi, lo; 
double d_SavY = 0; 

 PDFlib pdf;   
  
 try {	
   strcpy(jpgName, FileName);

   for ( i = strlen(jpgName) -1; i >= 0; i--) {
     if ( jpgName[i] == '.' )
        break;
     jpgName[i] = 0;  }
        
   strcpy (gifName, jpgName);
   strcat (jpgName, "bmp");
   strcat (gifName, "gif");	

	  pdf.set_parameter("hypertextencoding", "host");

   if ( pdf.begin_document(FileName, "") == -1) {
      string str = pdf.get_errmsg();
      const char* p = str.c_str();
      printf("Error: %s\n", p);
      return -2;  }
	
	   pdf.set_info("Creator", "FlamMapdf.dll");
	   pdf.set_info("Author", "FlamMap");
	   pdf.set_info("Title", "Landscape File Critique");

	   pdf.begin_page_ext(a4_width, a4_height, "");

	// Change "host" encoding to "winansi" or whatever you need!
	   int font = pdf.load_font("Helvetica", "host", "");
    if ( font == -1) {
       string str = pdf.get_errmsg();
       const char* p = str.c_str();
       printf("Error: %s\n", p);
       return -2; }

    int boldfont = pdf.load_font("Helvetica-Bold", "host", "");
    if ( boldfont == -1) {
       string str = pdf.get_errmsg();
       const char* p = str.c_str();
       printf("Error: %s\n", p);
       return -2; }

	   pdf.setfont(boldfont, 12);
    double y = 800, left = 100, right = 555, leading = 10;
    pdf.fit_textline("FlamMap Landscape File Critique", a4_width / 2, y, "position={center bottom}");
    pdf.setfont(font, 8);
	   pdf.set_text_pos(50, 750);

    sprintf(buf, "Landscape File: %s",GetLandFileName());
    y -= 2 * leading;
    pdf.fit_textline(buf, a4_width / 2, y, "position={center bottom}");
 
/*.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
    y -= 2 * leading;
    PDF_HeadInfo (&pdf, "Latitude:",         (double)GetLatitude(), "", 0, &y, "%.0f", leading); 
    PDF_HeadInfo (&pdf, "Cell Resolution X:", GetCellResolutionX(), "Cell Resolution Y:", GetCellResolutionY(), &y, "%.2f",leading);  
    PDF_HeadInfo (&pdf, "Num Cells East:",   (double)GetNumEast(),  "Num Cells North:", (double)GetNumNorth(), &y, "%.0f",leading);
    PDF_HeadInfo (&pdf, "UTM North:", GetNorthUtm(), "UTM East:", GetEastUtm(), &y, "%.4f",leading);
	   PDF_HeadInfo (&pdf, "UTM South:", GetSouthUtm(), "UTM West:", GetWestUtm(), &y, "%.4f",leading);

/*.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
    y -= 2 * leading;
	   pdf.setfont(boldfont, 8);
	 
    pdf.fit_textline("Themes Present", 50, y, "position={left bottom}");
	
    y -= leading;
    pdf.setfont(font, 8);
	   pdf.fit_textline("Theme", col1, y, "position={left bottom}");
    pdf.fit_textline("Units", col2, y, "position={left bottom}");
	   pdf.fit_textline("Range", col3, y, "position={left bottom}");
    y -= leading;
    strcpy(buf, "---------------------------------------------------------------------------------------------------------");
    pdf.fit_textline(buf, col1, y, "position={left bottom}");
    y -= leading;

    PDF_Themes (&pdf, &y, "Elevation", GetTheme_UnitsString(E_DATA), Header.hielev,   Header.loelev,   leading);
    PDF_Themes (&pdf, &y, "Slope",     GetTheme_UnitsString(S_DATA), Header.hislope,  Header.loslope,  leading);
    PDF_Themes (&pdf, &y, "Aspect",    GetTheme_UnitsString(A_DATA), Header.hiaspect, Header.loaspect, leading);
    PDF_Themes (&pdf, &y, "Fuels",     GetTheme_UnitsString(F_DATA), Header.hifuel,   Header.lofuel,   leading);
    PDF_Themes (&pdf, &y, "Canopy Cover", GetTheme_UnitsString(C_DATA), Header.hicover,Header.locover,  leading);

    int numThemes = 5;
	   if ( this->HaveCrownFuels() ) {
       PDF_Themes (&pdf, &y, "Stand Height", GetTheme_UnitsString(H_DATA), Header.hiheight,  Header.loheight, leading); 
       PDF_Themes (&pdf, &y, "Canopy Base Height",  GetTheme_UnitsString(B_DATA), Header.hibase,    Header.lobase,   leading); 
       PDF_Themes (&pdf, &y, "Canopy Bulk Density", GetTheme_UnitsString(P_DATA), Header.hidensity, Header.lodensity,leading); 
		     numThemes += 3;	}

	   if ( this->HaveGroundFuels() )	{
      PDF_Themes (&pdf, &y, "Duff",        GetTheme_UnitsString(D_DATA), Header.hidensity, Header.lodensity, leading);
      PDF_Themes (&pdf, &y, "Woody Fuels", GetTheme_UnitsString(W_DATA), Header.hiwoody,   Header.lowoody, leading);		
      numThemes += 2;	}
	
    y -= leading;
	
/*.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.*/
/*  Do Regular Histograms and then Fuel Histogram                                      */    
	  sprintf(buf, "Theme Value Distributions");
  	pdf.fit_textline(buf, 50, y, "position={left bottom}");
   d_SavY = -1; 
   PDF_Mngr (&pdf, this, &y, gifName, boldfont, &d_SavY, -1);  /* Do All Histograms */
    
   int numFuel = Header.numfuel;
   IndexItem *items = new IndexItem[numFuel];
   PDF_FuelHistogram (&pdf, this, d_SavY, gifName,items);            /* The the single Fuel Histo */

// ***********************************************************************************************
/* Theme Graphs */	
	   pageNum = PDF_AddPageNum(&pdf, pageNum, pnX, pnY, font);
	   pdf.end_page_ext("");

	   PDF_ThemeImage (&pdf, E_DATA, "Elevation",boldfont,jpgName,this,font,&pageNum);
    PDF_ThemeImage (&pdf, S_DATA, "Slope",    boldfont,jpgName,this,font,&pageNum);
    PDF_ThemeImage (&pdf, A_DATA, "Aspect",   boldfont,jpgName,this,font,&pageNum);
    PDF_ThemeImage (&pdf, F_DATA, "Fuels",       boldfont,jpgName,this,font,&pageNum);
    PDF_ThemeImage (&pdf, C_DATA, "Canopy Cover",boldfont,jpgName,this,font,&pageNum);
 
	 if ( HaveCrownFuels()) {
    PDF_ThemeImage (&pdf, H_DATA, "Stand Height",       boldfont,jpgName,this,font,&pageNum);
    PDF_ThemeImage (&pdf, B_DATA, "Canopy Base Height", boldfont,jpgName,this,font,&pageNum);
    PDF_ThemeImage (&pdf, P_DATA, "Canopy Bulk Density", boldfont,jpgName,this,font,&pageNum);  }
	
  if ( HaveGroundFuels() ) {
    PDF_ThemeImage (&pdf, D_DATA, "Duff", boldfont,jpgName,this,font,&pageNum);
    PDF_ThemeImage (&pdf, W_DATA, "Woody Fuels", boldfont,jpgName,this,font,&pageNum);	 }

// **********************************************************************************************
//                    fuel model specific distributions
	  y = 800;
	  int nDone = 0;
	  
   pdf.begin_page_ext(a4_width, a4_height, "");
	
   for (int f = 0; f < numFuel; f++) {     //for each fuel...

	   	int fuel = Header.fuels[items[f].loc];
	   	pdf.setfont (boldfont, 12);
	   	sprintf (buf, "Fuel Model %d Theme Value Distributions", fuel);
	   	pdf.fit_textline(buf, 50, y, "position={left bottom}");

	   	y -= 4;
     d_SavY = 0;
     PDF_Mngr (&pdf, this, &y, gifName, boldfont, &d_SavY, fuel);

 	   nDone++;
		   if (nDone % 2 == 0 || nDone == numFuel)	{
		  	  pageNum = PDF_AddPageNum(&pdf, pageNum, pnX, pnY, font);
		   	 pdf.end_page_ext("");
		 	   if ( nDone < numFuel)
				     pdf.begin_page_ext(a4_width, a4_height, "");
			    y = 800;	}
		   else
	    		y -= 130;

  	}    /* for f end */

    delete[] items; 

	   pdf.end_document("");
	  _unlink(gifName);
	  _unlink(jpgName);
 
  }  /* try */
  
  catch ( PDFlib::Exception &ex) 	{
  	 	cout << "PDFlib exception occurred: [" << ex.get_errnum() << "]\n " <<  ex.get_apiname() << ": " << ex.get_errmsg() << endl;// "\n");
	   	return -2;  }
 
 	return nRetCode;
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name:
* Desc:
*
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
int PDF_AddPageNum(PDFlib *pdf, int pageNum, double x, double y, int font)
{
char pgNumBuf[32];
	 pdf->setfont(font, 8);
	 sprintf(pgNumBuf, "%d", pageNum);
	 pdf->show_xy(pgNumBuf, x, y);
	 return pageNum + 1;
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name:
* Desc:
*
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
bool PDF_ThemeBasic(PDFlib *pdf, char *themeName, char *themeUnits, short themeLo, short themeHi, double col1, double col2, double col3, double y)
{
char buf[256];
	 sprintf(buf, "%s", themeName);
	 pdf->fit_textline(buf, col1, y, "position={left bottom}");
	 sprintf(buf, "%s", themeUnits);
	 pdf->fit_textline(buf, col2, y, "position={left bottom}");
	 sprintf(buf, "%d - %d", themeLo, themeHi);
	 pdf->fit_textline(buf, col3, y, "position={left bottom}");
	 return true;
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: PDF_FuelHistogram
* Desc: Put the Fuel Histogram to the PDF, the Fuel Histogram is done slightly
*       different than the other Historgrams
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
void  PDF_FuelHistogram (PDFlib *pdf, FlamMap *fm, double y, char gifName[], IndexItem items[])
{
CHistogram histogram;
double *pcts;
char **labels;
int i, numFuel, image;
celldata cell;
crowndata cfuel;
grounddata gfuel;
__int64 nCells;

   numFuel = fm->Header.numfuel;
   nCells = (__int64)fm->GetNumEast() * (__int64)fm->GetNumNorth();   
	  CrossTab fStats (numFuel, fm->Header.fuels, fm->Header.fuels);

   for ( __int64 p = 0; p < nCells; p++){
    		fm->GetCellDataFromMemory(p, cell, cfuel, gfuel);
		    fStats.AddObs(cell.f);}
 	 
   for (int f = 1; f < fStats.nBins; f++){
	    	items[f - 1].freq = fStats.freqs[f];
	    	items[f - 1].loc = f;	}
  
  	qsort((void *)items, (size_t)numFuel, sizeof(IndexItem), IndexCompare);
	  labels = new char *[numFuel + 1];
	  pcts = new double[numFuel + 1];

   for ( i = 0; i <= numFuel; i++)	{
	   	labels[i] = new char[64];
	   	if ( i == 0) {
		    	strcpy(labels[i], "ND");
		    	pcts[i] = ((double)fStats.freqs[0]) / ((double)fStats.nObs) * 100.0;		}
		   else {
		    	sprintf(labels[i], "%ld", fm->Header.fuels[items[i - 1].loc]);
		    	pcts[i] = ((double)fStats.freqs[items[i - 1].loc]) / ((double)fStats.nObs) * 100.0; }
	   }  /* for i */
	  
   histogram.CreateImage(gifName, numFuel + 1, pcts, "Fuels", "Percent", fm->GetTheme_UnitsString(F_DATA), labels, max(numFuel * 30, 300), 250);
	  for ( int i = 0; i <= numFuel; i++)	{
	   	delete[] labels[i]; }
	
   delete[] labels;
	  delete[] pcts;
	  image = pdf->load_image("auto", gifName, "");
   if (image >= 0)	{
		   pdf->fit_image(image, 50, y, "position={left top} scale=0.5");
	   	pdf->close_image(image);}
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: PDF_Mngr
* Desc: Put a group of histograms to the PDF. This is used to put the historgrams
*       at the top of the report and also the groups of Fuel Mode X histograms
*       at the end of the report. See the d_SavY parameter
*   In: Y....'y' row, this will get modified and sent back
*       d_SavY... send in a -1 when doing the regular histos at top of report
*                  this will cause an empty place to be left in the PDF where the
*                  Fuel histo can get dropped in later. 
*                 Send in a 0 when doing the Fuel Model X histos
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
void PDF_Mngr (PDFlib *pdf, FlamMap *fm, double *Y, char gifName[], int boldfont, 
                double *d_SavY, int fuel )
{
headdata Header;
double y;
    
    y = *Y; 
    memcpy (&Header,&fm->Header, sizeof(headdata));

    PDF_Histogram (pdf, "Elevation",  
                   Header.numelev, Header.hielev, Header.loelev, Header.elevs,  // array of 
                   fm,boldfont,50, y, gifName,E_DATA, fuel );

    PDF_Histogram (pdf,"Slope",
                   Header.numslope,Header.hislope,Header.loslope,Header.slopes,
                   fm,boldfont,210,y,gifName,S_DATA, fuel); 

    PDF_Histogram (pdf,"Aspect",
                   Header.numaspect,Header.hiaspect,Header.loaspect,Header.aspects,
                   fm,boldfont,370,y,gifName,A_DATA, fuel ); 

    if ( *d_SavY == -1 ) {   /* See notes in heading */
  	    y -= 130;
      *d_SavY = y; }

    y -= 130;
    PDF_Histogram (pdf, "Canopy Cover",
                  Header.numcover, Header.hicover,Header.locover,Header.covers,
                 fm, boldfont,50, y,gifName,C_DATA, fuel); 
 	  
    if ( fm->HaveCrownFuels() )	{ 
       PDF_Histogram (pdf, "Stand Height",
                   Header.numheight,Header.hiheight,Header.loheight,Header.heights,
                   fm,boldfont,210,y,gifName,H_DATA,fuel); 
 
       PDF_Histogram (pdf, "Canopy Base Height",
                   Header.numbase,Header.hibase,Header.lobase,Header.bases,
                   fm,boldfont,370,y,gifName,B_DATA,fuel); 
  	    y -= 130;
       PDF_Histogram (pdf, "Canopy Bulk Density",
                   Header.numdensity,Header.hidensity,Header.lodensity,Header.densities,
                   fm,boldfont,50,y,gifName,P_DATA,fuel);   }  
 
	   if ( fm->HaveGroundFuels() ) { 
       PDF_Histogram (pdf, "Duff",
                     Header.numduff,Header.hiduff,Header.loduff,Header.duffs,
                     fm,boldfont,210,y,gifName,D_DATA, fuel ); 
 
       PDF_Histogram (pdf, "Woody Fuels",
                     Header.numwoody,Header.hiwoody,Header.lowoody,Header.woodies,
                     fm, boldfont, 370, y,gifName,W_DATA, fuel);  }
   *Y = y; 
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: PDF_Histogram
* Desc: Put a single histogram to the PDF file
*
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
void PDF_Histogram (PDFlib *pdf, char cr_Theme[],  
                   int i_num, short Hhi, short Hlo, long H_ary[],
                   FlamMap *fm, int boldfont, double x, double y, char gifName[], 
                   int DATA, int fuel )
{	
int nEbins = 0, nSBins = 0, nAbins = 0, nCbins = 0, nHbins = 0;
int nBbins = 0, nPbins = 0, nDbins = 0, nWbins = 0;
celldata cell;
crowndata cfuel;
grounddata gfuel;
long *tMins = NULL, *tMaxs = NULL;
int nVals, image;  // numFuel;
double avg1, avg2;
char buf[300]; 
  
  if ( i_num > 10 || i_num < 0 ) { 
	  	nVals = 10;
		  tMins = new long[nVals + 1];
		  tMaxs = new long[nVals + 1];
    long lo = max(0, Hlo), hi = Hhi;
	  	tMins[0] = 0;
	  	tMaxs[0] = 0;
	  	for (int i = 1; i <= nVals; i++)		
		{
       if ( DATA == A_DATA) 
	   {
         tMins[i] = ( i - 1) * 36;      
         tMaxs[i] = i * 36; } 
       else 
	   {
			      tMins[i] = lo + ((i - 1) * ((hi - lo)/9));
		   	   tMaxs[i] = lo + ((i) * ((hi - lo)/9));	
	   }	
		}	
  }
	 else 
	 {
	   nVals = i_num;              // Original 	nVals = Header.numelev;
	  	tMins = new long[nVals + 1];
  		tMaxs = new long[nVals + 1];
  		tMins[0] = 0;
  		tMaxs[0] = 0;
	  	for ( int i = 1; i <= nVals; i++)	
		{
	   		tMins[i] = tMaxs[i] = H_ary[i];	
		} // 	tMins[i] = tMaxs[i] = Header.elevs[i];
  	}
 
	 CrossTab *tStats = new CrossTab(nVals, tMins, tMaxs);
 	__int64 nCells = (__int64)fm->GetNumEast() * (__int64)fm->GetNumNorth();
	for (__int64 p = 0; p < nCells; p++ )
	{
		fm->GetCellDataFromMemory(p, cell, cfuel, gfuel);

		if ( fuel != -1 && fuel != cell.f )
			continue; 
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
		}
	}

	  pdf->setfont(boldfont, 8); 
	  CHistogram histogram;
	  char **labels;
  	labels = new char *[nVals + 1];
  	double *pcts = new double[nVals + 1];

	  for ( int i = 0; i <= nVals; i++)
	  {
	   	labels[i] = new char[64];
	   	if ( i == 0 )
			    strcpy(labels[i], "ND");
		   else
			    sprintf(labels[i], "%ld", tMins[i]);
	   	pcts[i] = ((double)tStats->freqs[i]) / ((double)tStats->nObs) * 100.0;  	
	  }

	 
	  histogram.CreateImage(gifName, nVals + 1, pcts, cr_Theme, "Percent", fm->GetTheme_UnitsString(DATA), labels, max(nVals * 30, 300), 250);
	 
   for(int i = 0; i <= nVals; i++)	
   {
	   	delete[] labels[i];
   }
  	delete[] labels;
  	delete[] pcts;
  	delete tStats;
  	delete[] tMins;
  	delete[] tMaxs;	
   image = pdf->load_image("auto", gifName, "");
   if (image >= 0) 
   {
   		pdf->fit_image(image, x, y, "position={left top} scale=0.5");
   		pdf->close_image(image);	
   } 
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name:
* Desc:
*
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
void PDF_Themes (PDFlib *pdf, double *y, char Label[], char Unit[], short hi, short low, double leading)
{	
short lo;
double col1 = COL1, col2 = COL2, col3 = COL3, col4 = COL4;
char  buf[250];

	 lo = max(0, low);
	 pdf->fit_textline(Label, col1, *y, "position={left bottom}");	
	 pdf->fit_textline(Unit, col2, *y, "position={left bottom}");	
  sprintf(buf, "%d - %d", lo, hi);
	 pdf->fit_textline(buf, col3, *y, "position={left bottom}");
  *y = *y - leading;
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name: PSF_ThemeImage
* Desc: Put an image (elevation,slope,etc) to the report, 
*
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
void PDF_ThemeImage (PDFlib *pdf, int DATA, char Theme[], int boldfont, char jpgName[], 
                         FlamMap *fm, int font, int *pageNum)
{
int image;   
double y;

   pdf->begin_page_ext(a4_width, a4_height, "");
   y = 800;
   pdf->setfont(boldfont, 12);
   pdf->fit_textline(Theme, 50, y, "position={left bottom}");	
   fm->GenerateImage(DATA, jpgName);
   image = pdf->load_image("auto", jpgName, "");
   if ( image >= 0)
   {
	   	pdf->fit_image(image, 52, 10, "boxsize={540 780} position=center fitmethod=meet");
	   	pdf->close_image(image);	
   }
 
   *pageNum = PDF_AddPageNum(pdf, *pageNum, PNX, PNY, font);  
   pdf->end_page_ext("");

   pdf->begin_page_ext(a4_width, a4_height, "");	
   pdf->setfont(boldfont, 12);
   pdf->fit_textline(Theme, 50, y, "position={left bottom}");	
   fm->GenerateLegendImage(DATA, jpgName);
   image = pdf->load_image("auto", jpgName, "");
   if ( image >= 0)	
   {
	   	pdf->fit_image(image, 52, 10, "boxsize={150 700} position=center fitmethod=auto");
       
	   	pdf->close_image(image);
   }

  	*pageNum = PDF_AddPageNum(pdf, *pageNum, PNX, PNY, font);  
  	pdf->end_page_ext("");
}

/*{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}
* Name:
* Desc:
*
{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}{*}*/
void PDF_HeadInfo (PDFlib *pdf, char A[], double a, char B[], double b, double *y, char Fmt[], double leading)
{
double col1 = COL1, col2 = COL2, col3 = COL3, col4 = COL4;
char buf[300]; 
  
	  pdf->fit_textline(A, col1, *y, "position={left bottom}");
	  sprintf(buf, Fmt, a);
	  pdf->fit_textline(buf, col2, *y, "position={left bottom}");
	  
   if ( strcmp (B,"") != 0)
   {
	   pdf->fit_textline(B, col3, *y, "position={left bottom}");
	   sprintf(buf, Fmt, b);
		  pdf->fit_textline(buf, col4, *y, "position={left bottom}");
   }
   *y = *y - leading; 
}




