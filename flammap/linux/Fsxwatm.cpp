#include "fsxwatm.h"
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//	AtmosphereGrid Structure Functions
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


AtmosphereGrid::AtmosphereGrid(int numgrids, FlamMap *_pFlamMap)
{
	pFlamMap = _pFlamMap;
	NumGrids=numgrids;
     switch(NumGrids)
     {	case 0:
     		StartGrid=6;
     		AtmGridWND=false;
               AtmGridWTR=false;
               break;
     	case 3:
          	StartGrid=3;
	      	AtmGridWND=true;
          	AtmGridWTR=false;
               break;
          case 6:
          	StartGrid=0;
		     AtmGridWND=true;
	     	AtmGridWTR=true;
               break;
          default:
     		StartGrid=6;
     		AtmGridWND=false;
               AtmGridWTR=false;
          	break;
     }
	TimeIntervals=0;
     Month=0;
     Day=0;
     Hour=0;
     Metric=0;
}


AtmosphereGrid::~AtmosphereGrid()
{
	int i;

	for(i=StartGrid; i<6; i++)
     	atmosphere[i].FreeAtmGrid();
     if(Month)
	     delete[] Month;//free(Month);
     if(Day)
	     delete[] Day;//free(Day);
     if(Hour)
     	delete[] Hour;//free(Hour);
}


void AtmosphereGrid::FreeAtmData()
{
	int i;

	for(i=StartGrid; i<6; i++)
     	atmosphere[i].FreeAtmGrid();
     if(Month)
	     delete[] Month;//free(Month);
     if(Day)
	     delete[] Day;//free(Day);
     if(Hour)
     	delete[] Hour;//free(Hour);
     Month=0;
     Day=0;
     Hour=0;
     Metric=0;
}



bool AtmosphereGrid::ReadInputTable(char * InputFileName)
{
     char InputTemp[256];
     char InputHumid[256];
     char InputRain[256];
     char InputCloud[256];
     char InputSpd[256];
     char InputDir[256];
     char FileName[256];
     char UnitsString[256]="";
     int i, j, fpos;

     TimeIntervals=0;
     memset(ErrMsg, 0x0, sizeof(ErrMsg));
     if((InputTable=fopen(InputFileName, "r"))!=NULL)
     {    fscanf(InputTable, "%s", InputTemp);	// get by header
          fpos=ftell(InputTable);
          fscanf(InputTable, "%s", UnitsString);
		  int temp=0;
		  char c;
		  char tempChar[64];
		  while (UnitsString[temp])
		  {
			c=UnitsString[temp];
			tempChar[temp] = tolower(c);
			temp++;
		  }
		  int temp2 = 0;
		  while (tempChar[temp2])
		  {
			UnitsString[temp2] = tempChar[temp2];
			temp2++;
		  }
          if(!strcmp(UnitsString, "metric"))
               Metric=1;
          else if(!strcmp(UnitsString, "english"))   // english default
               Metric=0;//fseek(InputTable, fpos, SEEK_SET);
          else
          {    rewind(InputTable);
               fscanf(InputTable, "%s", InputTemp);	// get by header
          }
     	while(!feof(InputTable))		// count number of time intervals
     	{    fscanf(InputTable, "%ld", &month);
               if(feof(InputTable))
                    break;
               if(NumGrids<6)
	          {	fscanf(InputTable, "%ld %ld %s %s %s ", &day, &hour,
     				 InputSpd, InputDir, InputCloud);
					//fstream fin(InputSpd);
					FILE* SpdTest = fopen(InputSpd, "r");
					FILE* DirTest = fopen(InputDir, "r");
					FILE* CloudTest = fopen(InputCloud, "r");
					//if((GetFileAttributes(InputSpd))==0xFFFFFFFF)
					if (!SpdTest)
                         sprintf(ErrMsg, "%s %s", InputSpd, "Can't Be Opened or Read");
                    //if((GetFileAttributes(InputDir))==0xFFFFFFFF)
					if (!DirTest)
                         sprintf(ErrMsg, "%s %s", InputDir, "Can't Be Opened or Read");
                    //if((GetFileAttributes(InputCloud))==0xFFFFFFFF)
					if (!CloudTest)
                         sprintf(ErrMsg, "%s %s", InputCloud, "Can't Be Opened or Read");
					fclose(SpdTest);
					fclose(DirTest);
					fclose(CloudTest);
               }
          	else
	          {	fscanf(InputTable, "%ld %ld %s %s %s %s %s %s", &day, &hour,
     				 InputTemp, InputHumid, InputRain, InputSpd, InputDir, InputCloud);
					//ifstream fin(InputTemp);
                    //if((GetFileAttributes(InputTemp))==0xFFFFFFFF)
				    //if(fin.fail())
					FILE* TempTest = fopen(InputTemp, "r");
					FILE* HumidTest = fopen(InputHumid, "r");
					FILE* RainTest = fopen(InputRain, "r");
					FILE* SpdTest = fopen(InputSpd, "r");
					FILE* DirTest = fopen(InputDir, "r");
					FILE* CloudTest = fopen(InputCloud, "r");
					if (!TempTest)
                         sprintf(ErrMsg, "%s %s", InputTemp, "Can't Be Opened or Read");
                    //if((GetFileAttributes(InputHumid))==0xFFFFFFFF)
					if (!HumidTest)
                         sprintf(ErrMsg, "%s %s", InputHumid, "Can't Be Opened or Read");
                    //if((GetFileAttributes(InputRain))==0xFFFFFFFF)
					if (!RainTest)
                         sprintf(ErrMsg, "%s %s", InputRain, "Can't Be Opened or Read");
                    //if((GetFileAttributes(InputSpd))==0xFFFFFFFF)
					if (!SpdTest)
                         sprintf(ErrMsg, "%s %s", InputSpd, "Can't Be Opened or Read");
                    //if((GetFileAttributes(InputDir))==0xFFFFFFFF)
					if (!DirTest)
                         sprintf(ErrMsg, "%s %s", InputDir, "Can't Be Opened or Read");
                    //if((GetFileAttributes(InputCloud))==0xFFFFFFFF)
					if (!CloudTest)
                         sprintf(ErrMsg, "%s %s", InputCloud, "Can't Be Opened or Read");
					fclose(TempTest);
					fclose(RainTest);
					fclose(HumidTest);
					fclose(SpdTest);
					fclose(DirTest);
					fclose(CloudTest);
               }
               if(strlen(ErrMsg)>0)
               {    fclose(InputTable);
                    return false;
               }
               TimeIntervals++;
          };
         	Month=new int[TimeIntervals];//(int *) calloc(TimeIntervals, sizeof(int));
         	Day=new int[TimeIntervals];//(int *) calloc(TimeIntervals, sizeof(int));
          Hour=new int[TimeIntervals];//(int *) calloc(TimeIntervals, sizeof(int));
          //rewind(InputTable);
          fseek(InputTable, fpos, SEEK_SET);
     	if(NumGrids<6)
          	fscanf(InputTable, "%ld %ld %ld %s %s %s ", &month, &day, &hour,
    				 InputSpd, InputDir, InputCloud);
          else
             	fscanf(InputTable, "%ld %ld %ld %s %s %s %s %s %s", &month, &day, &hour,
	    			InputTemp, InputHumid, InputRain, InputSpd, InputDir, InputCloud);
     	for(j=StartGrid; j<6; j++)   // set header information in each file
          {    memset(FileName, 0x0, sizeof(FileName));
          	switch(j)
          	{	case 0: strcpy(FileName, InputTemp); break;
               	case 1: strcpy(FileName, InputHumid); break;
                    case 2: strcpy(FileName, InputRain); break;
                    case 3: strcpy(FileName, InputSpd); break;
               	case 4: strcpy(FileName, InputDir); break;
				case 5: strcpy(FileName, InputCloud); break;
	          }
		     if((ThisFile=fopen(FileName, "r"))==NULL)
             	{	sprintf(ErrMsg, "%s %s", FileName, "Can't Be Opened or Read");
                    fclose(InputTable);

 		    		return false;
               }
     	     else if(!ReadHeaders(j))
        		{	sprintf(ErrMsg, "%s %s", FileName, "Header Not GRASS or GRID");
                    fclose(InputTable);

		         	return false;
               }
               else if(!atmosphere[j].AllocAtmGrid(TimeIntervals))
          	{    sprintf(ErrMsg, "%s %s", FileName, "Memory Not Sufficient for File");
                    for(i=StartGrid; i<j; i++)
                    	atmosphere[i].FreeAtmGrid();
                    fclose(InputTable);

               	return false;
               }
          }
          for(i=0; i<TimeIntervals; i++)
          {    Month[i]=month;
          	Day[i]=day;
	          Hour[i]=hour;
              	for(j=StartGrid; j<6; j++)
          	{	memset(FileName, 0x0, sizeof(FileName));
          		switch(j)
          		{	case 0: strcpy(FileName, InputTemp); break;
               		case 1: strcpy(FileName, InputHumid); break;
                    	case 2: strcpy(FileName, InputRain); break;
     	          	case 3: strcpy(FileName, InputSpd); break;
               		case 4: strcpy(FileName, InputDir); break;
                    	case 5: strcpy(FileName, InputCloud); break;
	          	}
                    if((ThisFile=fopen(FileName, "r"))==NULL)
                    {    sprintf(ErrMsg, "%s %s", FileName, "File Not Found or Cannot Be Read");
                         fclose(InputTable);

                         return false;
                    }
                    if(!CompareHeader(j))
                    {	sprintf(ErrMsg, "%s %s", FileName, "Header Not Same For File Type");
                         fclose(InputTable);

		         		return false;
                    }
                    if(!SetAtmosphereValues(i, j))
                    {	sprintf(ErrMsg, "%s %s", FileName, "Error Reading File Values");
                         fclose(InputTable);

                        	return false;
                    }
               }
               if(NumGrids==6)
	          	fscanf(InputTable, "%ld %ld %ld %s %s %s %s %s %s", &month, &day, &hour,
     				 InputTemp, InputHumid, InputRain, InputSpd, InputDir, InputCloud);
               else
	          	fscanf(InputTable, "%ld %ld %ld %s %s %s", &month, &day, &hour,
     				 InputSpd, InputDir, InputCloud);
          };// while(!feof(InputTable));

     	fclose(InputTable);
     }
     else
     	return false;

     return true;
}


bool AtmosphereGrid::ReadHeaders(int FileNumber)
{
	char TestString[256];
	char TestStringUpper[256];
     char CompGrass[]="north:";
     char CompGrid[]="NCOLS";
     double north, south, east, west, xres, yres;
     int rows, cols;
	  int temp3=0;
		  char c2;
		  char tempChar2[64];
		  while (TestString[temp3])
		  {
			c2=TestString[temp3];
			tempChar2[temp3] = toupper(c2);
			temp3++;
		  }
		  int temp4 = 0;
		  while (tempChar2[temp4])
		  {
			TestStringUpper[temp4] = tempChar2[temp4];
			temp4++;
		  }
	 fscanf(ThisFile, "%s", TestString);
     if(!strcmp(TestString, CompGrass))	// grass file
     {	fscanf(ThisFile, "%lf", &north);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%lf", &south);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%lf", &east);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%lf", &west);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%ld", &rows);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%ld", &cols);
          xres=(east-west)/(double) cols;
     	yres=(north-south)/(double) rows;
          atmosphere[FileNumber].SetHeaderInfo(north, south, east, west, xres, yres);
     }
     else if(!strcmp(TestStringUpper, CompGrid))
     {    fscanf(ThisFile, "%ld", &cols);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%ld", &rows);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%lf", &west);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%lf", &south);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%lf", &xres);
          yres=xres;
          east=west+(double) cols*xres;
          north=south+(double) rows*yres;
          atmosphere[FileNumber].SetHeaderInfo(north, south, east, west, xres, yres);
     }
     else
     {	fclose(ThisFile);

     	return false;
     }

    	fclose(ThisFile);

     return true;
}


bool AtmosphereGrid::CompareHeader(int FileNumber)
{
	char TestString[256];
	char TestStringUpper[256];
     char CompGrass[]="north:";
     char CompGrid[]="NCOLS";
     double north, south, east, west, xres, yres;
     int rows, cols;
	 int temp3=0;
		  char c2;
		  char tempChar2[64];
		  while (TestString[temp3])
		  {
			c2=TestString[temp3];
			tempChar2[temp3] = toupper(c2);
			temp3++;
		  }
		  int temp4 = 0;
		  while (tempChar2[temp4])
		  {
			TestStringUpper[temp4] = tempChar2[temp4];
			temp4++;
		  }


	fscanf(ThisFile, "%s", TestString);
     if(!strcmp(TestString, CompGrass))	// grass file
     {	fscanf(ThisFile, "%lf", &north);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%lf", &south);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%lf", &east);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%lf", &west);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%ld", &rows);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%ld", &cols);
          xres=(east-west)/(double) cols;
     	yres=(north-south)/(double) rows;
          if(!atmosphere[FileNumber].CompareHeader(north, south, east, west, xres, yres))
          {	fclose(ThisFile);

           	return false;
          }
     }
     else if(!strcmp(TestStringUpper, CompGrid))
     {    fscanf(ThisFile, "%ld", &cols);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%ld", &rows);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%lf", &west);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%lf", &south);
     	fscanf(ThisFile, "%s", TestString);
		fscanf(ThisFile, "%lf", &xres);
          fscanf(ThisFile, "%s", TestString);
          fscanf(ThisFile, "%s", TestString);
          yres=xres;
          east=west+(double) cols*xres;
          north=south+(double) rows*yres;
          if(!atmosphere[FileNumber].CompareHeader(north, south, east, west, xres, yres))
          {	fclose(ThisFile);

           	return false;
          }
     }
     else
     {	fclose(ThisFile);

     	return false;
     }
     if(FileNumber==3)		// set gridded weather dimensions for wind display
     {    double SouthDiff=fabs(south-pFlamMap->GetSouthUtm());
          double WestDiff=fabs(west-pFlamMap->GetWestUtm());
          double SouthOffset=SouthDiff-((int) (SouthDiff/yres))*yres;
          double WestOffset=WestDiff-((int) (WestDiff/xres))*xres;
          int   EastNumber, NorthNumber;

          if(pFlamMap->GetEastUtm()<east)
          	EastNumber=(pFlamMap->GetEastUtm()-pFlamMap->GetWestUtm()-WestOffset)/xres;
          else
          	EastNumber=(east-pFlamMap->GetWestUtm()-WestOffset)/xres;
          if(pFlamMap->GetNorthUtm()<north)
          	NorthNumber=(pFlamMap->GetNorthUtm()-pFlamMap->GetSouthUtm()-SouthOffset)/yres;
          else
          	NorthNumber=(north-pFlamMap->GetSouthUtm()-SouthOffset)/yres;
          if(EastNumber<atmosphere[FileNumber].XNumber)
          	EastNumber++;
          if(NorthNumber<atmosphere[FileNumber].YNumber)
          	NorthNumber++;
//         pFlamMap-> SetGridNorthOffset(SouthOffset);
//          pFlamMap->SetGridEastOffset(WestOffset);
     	//pFlamMap->SetGridEastDimension(EastNumber);
//		pFlamMap->SetGridNorthDimension(NorthNumber);
     	//SetGridEastDimension(atmosphere[FileNumber].XNumber);
		//SetGridNorthDimension(atmosphere[FileNumber].YNumber);
     }

     return true;
}


bool AtmosphereGrid::SetAtmosphereValues(int timeinterval, int filenumber)
{
     double value;

	for(int i=0; i<atmosphere[filenumber].NumCellsPerGrid; i++)
     {    fscanf(ThisFile, "%lf", &value);
          if(Metric)
          {    switch(filenumber)
               {    case 0:   value*=1.0;
                              value+=32.0;
                              break;
                    case 2:   value*=3.93;
                              break;
                    case 3:   value*=.5402;  // convert 10m to 20ft, and kmph to miph
                              break;
               }
          }
          if(!atmosphere[filenumber].SetAtmValue(i, timeinterval, (short) value))
          {	fclose(ThisFile);

          	return false;
          }
     }
     fclose(ThisFile);

     return true;
}


bool AtmosphereGrid::GetAtmosphereValue(int FileNumber, double xpt, double ypt, int time, short *value)
{
	if(!atmosphere[FileNumber].GetAtmValue(xpt, ypt, time, value))
	{    switch(FileNumber)
	     {	case 0: *value=70; break;  // default values for ATM variables if no data
          	case 1: *value=40; break;
               default: *value=0; break;
     	}

     	return false;
     }

	return true;
}


void AtmosphereGrid::GetResolution(int FileNumber, double *resx, double *resy)
{
	atmosphere[FileNumber].GetResolution(resx, resy);
}


int AtmosphereGrid::GetTimeIntervals()
{
	return TimeIntervals;
}

int AtmosphereGrid::GetAtmMonth(int count)
{
	if(count<TimeIntervals)
		return Month[count];

     return -1;
}

int AtmosphereGrid::GetAtmDay(int count)
{
	if(count<TimeIntervals)
		return Day[count];

     return -1;
}

int AtmosphereGrid::GetAtmHour(int count)
{
	if(count<TimeIntervals)
		return Hour[count];

     return -1;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//	Atmosphere Structure Functions
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


Atmosphere::Atmosphere()
{
	North=0;
	South=0;
     East=0;
     West=0;
     ResolutionX=0;
     ResolutionY=0;
     Value=0;
}


Atmosphere::~Atmosphere()
{

}


void Atmosphere::SetHeaderInfo(double north, double south, double east, double west,
   			    double resolutionx, double resolutiony)
{
	North=pFlamMap->ConvertUtmToNorthingOffset(north);
     South=pFlamMap->ConvertUtmToNorthingOffset(south);
     East=pFlamMap->ConvertUtmToEastingOffset(east);
     West=pFlamMap->ConvertUtmToEastingOffset(west);
     ResolutionX=resolutionx;
     ResolutionY=resolutiony;
     XNumber=(East-West)/ResolutionX;
     YNumber=(North-South)/ResolutionY;
     NumCellsPerGrid=XNumber*YNumber;
}


bool Atmosphere::GetHeaderInfo(double *north, double *south, double *east, double *west,
     				double *resolutionx, double *resolutiony)
{
	*north=North;
     *south=South;
     *east=East;
     *west=West;
     *resolutionx=ResolutionX;
     *resolutiony=ResolutionY;

     return true;
}


void Atmosphere::GetResolution(double *resolutionx, double *resolutiony)
{
     *resolutionx=ResolutionX;
     *resolutiony=ResolutionY;
}


bool Atmosphere::CompareHeader(double north, double south, double east, double west,
     		    double resolutionx, double resolutiony)
{
	if(North!=pFlamMap->ConvertUtmToNorthingOffset(north))
     	return false;
     if(South!=pFlamMap->ConvertUtmToNorthingOffset(south))
     	return false;
     if(East!=pFlamMap->ConvertUtmToEastingOffset(east))
     	return false;
     if(West!=pFlamMap->ConvertUtmToEastingOffset(west))
     	return false;
     if(ResolutionX!=resolutionx)
     	return false;
     if(ResolutionY!=resolutiony)
     	return false;

     return true;
}



bool Atmosphere::AllocAtmGrid(int timeintervals)
{
	int AllocNumber=(timeintervals+1)*NumCellsPerGrid;

	if((Value=new short[AllocNumber])==NULL)//(short *) calloc(AllocNumber, sizeof(short)))==NULL)	// allocate 2D grid
     {	Value=0;
      	return false;
     }

     return true;
}


bool Atmosphere::FreeAtmGrid()
{
 	if(Value)
     	delete[] Value;//free(Value);
     else
     	return false;

     Value=0;

     return true;
}


bool Atmosphere::GetAtmValue(double xpt, double ypt, int time, short *value)
{
	if(xpt<West || xpt>East)
     	return false;
     if(ypt<South || ypt>North)
     	return false;
	CellX=((int) ((xpt-West)/ResolutionX));
     CellY=((int) ((North-ypt)/ResolutionY));
	*value=Value[time*NumCellsPerGrid+(CellY*XNumber+CellX)];

	return true;
}


bool Atmosphere::SetAtmValue(int CellNumber, int time, short value)
{
	if(Value)
     {    Value[time*NumCellsPerGrid+CellNumber]=(short) value;

          return true;
     }

     return false;
}


