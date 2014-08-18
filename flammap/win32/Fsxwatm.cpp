//#include "fsxwatm.h"
#include "flmglbvar.h"


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
//	AtmosphereGrid Structure Functions
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


AtmosphereGrid::AtmosphereGrid(long numgrids)
{
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
	long i;

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
	long i;

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
     long i, j, fpos;

     TimeIntervals=0;
     memset(ErrMsg, 0x0, sizeof(ErrMsg));
     if((InputTable=fopen(InputFileName, "r"))!=NULL)
     {    fscanf(InputTable, "%s", InputTemp);	// get by header
          fpos=ftell(InputTable);
          fscanf(InputTable, "%s", UnitsString);
          if(!strcmp(strlwr(UnitsString), "metric"))
               Metric=1;
          else if(!strcmp(strlwr(UnitsString), "english"))   // english default
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
                    if((GetFileAttributes(reinterpret_cast<LPCSTR>(InputSpd)))==0xFFFFFFFF)
                         sprintf(ErrMsg, "%s %s", InputSpd, "Can't Be Opened or Read");
                    if((GetFileAttributes(reinterpret_cast<LPCSTR>(InputDir)))==0xFFFFFFFF)
                         sprintf(ErrMsg, "%s %s", InputDir, "Can't Be Opened or Read");
                    if((GetFileAttributes(reinterpret_cast<LPCSTR>(InputCloud)))==0xFFFFFFFF)
                         sprintf(ErrMsg, "%s %s", InputCloud, "Can't Be Opened or Read");
               }
          	else
	          {	fscanf(InputTable, "%ld %ld %s %s %s %s %s %s", &day, &hour,
     				 InputTemp, InputHumid, InputRain, InputSpd, InputDir, InputCloud);
                    if((GetFileAttributes(reinterpret_cast<LPCSTR>(InputTemp)))==0xFFFFFFFF)
                         sprintf(ErrMsg, "%s %s", InputTemp, "Can't Be Opened or Read");
                    if((GetFileAttributes(reinterpret_cast<LPCSTR>(InputHumid)))==0xFFFFFFFF)
                         sprintf(ErrMsg, "%s %s", InputHumid, "Can't Be Opened or Read");
                    if((GetFileAttributes(reinterpret_cast<LPCSTR>(InputRain)))==0xFFFFFFFF)
                         sprintf(ErrMsg, "%s %s", InputRain, "Can't Be Opened or Read");
                    if((GetFileAttributes(reinterpret_cast<LPCSTR>(InputSpd)))==0xFFFFFFFF)
                         sprintf(ErrMsg, "%s %s", InputSpd, "Can't Be Opened or Read");
                    if((GetFileAttributes(reinterpret_cast<LPCSTR>(InputDir)))==0xFFFFFFFF)
                         sprintf(ErrMsg, "%s %s", InputDir, "Can't Be Opened or Read");
                    if((GetFileAttributes(reinterpret_cast<LPCSTR>(InputCloud)))==0xFFFFFFFF)
                         sprintf(ErrMsg, "%s %s", InputCloud, "Can't Be Opened or Read");
               }
               if(strlen(ErrMsg)>0)
               {    fclose(InputTable);
                    return false;
               }
               TimeIntervals++;
          };
         	Month=new long[TimeIntervals];//(long *) calloc(TimeIntervals, sizeof(long));
         	Day=new long[TimeIntervals];//(long *) calloc(TimeIntervals, sizeof(long));
          Hour=new long[TimeIntervals];//(long *) calloc(TimeIntervals, sizeof(long));
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


bool AtmosphereGrid::ReadHeaders(long FileNumber)
{
	char TestString[256];
     char CompGrass[]="north:";
     char CompGrid[]="NCOLS";
     double north, south, east, west, xres, yres;
     long rows, cols;

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
     else if(!(strcmp(strupr(TestString), CompGrid)))
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


bool AtmosphereGrid::CompareHeader(long FileNumber)
{
	char TestString[256];
     char CompGrass[]="north:";
     char CompGrid[]="NCOLS";
     double north, south, east, west, xres, yres;
     long rows, cols;

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
     else if(!(strcmp(strupr(TestString), CompGrid)))
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
     {    double SouthDiff=fabs(south-GetSouthUtm());
          double WestDiff=fabs(west-GetWestUtm());
          double SouthOffset=SouthDiff-((long) (SouthDiff/yres))*yres;
          double WestOffset=WestDiff-((long) (WestDiff/xres))*xres;
          long   EastNumber, NorthNumber;

          if(GetEastUtm()<east)
          	EastNumber=(GetEastUtm()-GetWestUtm()-WestOffset)/xres;
          else
          	EastNumber=(east-GetWestUtm()-WestOffset)/xres;
          if(GetNorthUtm()<north)
          	NorthNumber=(GetNorthUtm()-GetSouthUtm()-SouthOffset)/yres;
          else
          	NorthNumber=(north-GetSouthUtm()-SouthOffset)/yres;
          if(EastNumber<atmosphere[FileNumber].XNumber)
          	EastNumber++;
          if(NorthNumber<atmosphere[FileNumber].YNumber)
          	NorthNumber++;
          SetGridNorthOffset(SouthOffset);
          SetGridEastOffset(WestOffset);
     	SetGridEastDimension(EastNumber);
		SetGridNorthDimension(NorthNumber);
     	//SetGridEastDimension(atmosphere[FileNumber].XNumber);
		//SetGridNorthDimension(atmosphere[FileNumber].YNumber);
     }

     return true;
}


bool AtmosphereGrid::SetAtmosphereValues(long timeinterval, long filenumber)
{
     double value;

	for(long i=0; i<atmosphere[filenumber].NumCellsPerGrid; i++)
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


bool AtmosphereGrid::GetAtmosphereValue(long FileNumber, double xpt, double ypt, long time, short *value)
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


void AtmosphereGrid::GetResolution(long FileNumber, double *resx, double *resy)
{
	atmosphere[FileNumber].GetResolution(resx, resy);
}


long AtmosphereGrid::GetTimeIntervals()
{
	return TimeIntervals;
}

long AtmosphereGrid::GetAtmMonth(long count)
{
	if(count<TimeIntervals)
		return Month[count];

     return -1;
}

long AtmosphereGrid::GetAtmDay(long count)
{
	if(count<TimeIntervals)
		return Day[count];

     return -1;
}

long AtmosphereGrid::GetAtmHour(long count)
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
	North=ConvertUtmToNorthingOffset(north);
     South=ConvertUtmToNorthingOffset(south);
     East=ConvertUtmToEastingOffset(east);
     West=ConvertUtmToEastingOffset(west);
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
	if(North!=ConvertUtmToNorthingOffset(north))
     	return false;
     if(South!=ConvertUtmToNorthingOffset(south))
     	return false;
     if(East!=ConvertUtmToEastingOffset(east))
     	return false;
     if(West!=ConvertUtmToEastingOffset(west))
     	return false;
     if(ResolutionX!=resolutionx)
     	return false;
     if(ResolutionY!=resolutiony)
     	return false;

     return true;
}



bool Atmosphere::AllocAtmGrid(long timeintervals)
{
	long AllocNumber=(timeintervals+1)*NumCellsPerGrid;

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


bool Atmosphere::GetAtmValue(double xpt, double ypt, long time, short *value)
{
	if(xpt<West || xpt>East)
     	return false;
     if(ypt<South || ypt>North)
     	return false;
	CellX=((long) ((xpt-West)/ResolutionX));
     CellY=((long) ((North-ypt)/ResolutionY));
	*value=Value[time*NumCellsPerGrid+(CellY*XNumber+CellX)];

	return true;
}


bool Atmosphere::SetAtmValue(long CellNumber, long time, short value)
{
	if(Value)
     {    Value[time*NumCellsPerGrid+CellNumber]=(short) value;

          return true;
     }

     return false;
}


