//----------------------------------------------------------------------------
//  FlamMap version 2.0, Command Line
//  
//       Mark A. Finney, 2004
//----------------------------------------------------------------------------
///---------------------------------------------------------------------------

#pragma hdrstop

//---------------------------------------------------------------------------

//#pragma argsused
#include "omffr_port.h"

#include <process.h>
#include <direct.h>

#include "flammap.h"
#include "fsxsync.h"

#define FSPRO_VERSION true
#define NODESPREAD_VERSION false

bool LandFileOpen=false;
bool WINDOW_ON=false;
Acceleration AccelConst;

/*
void main(int argc, char* argv[])
{
    /*
     if(argc<10)
     {    printf("Program: FlamMap, v2.0\n");
          printf("Requires 9 Arguments:\n");
          printf("  LandscapeFileName\n");
          printf("  FuelMoistureFileName\n");
          printf("  OutputFileName\n");
          printf("  CustomFuelModelFileName or -1 for no file\n");
          printf("  Windspeed (mph)\n");
          printf("  WindDirection (degrees), -1 for uphill\n");
          printf("  WeatherFileName or -1 for no file\n");
          printf("  WindFileName or -1 for no file\n");
          printf("  DateFileName or -1 for no file\n");
          printf("	Foliar Moisture Content (for canopy ignition, %%, e.g. 100)\n");
		printf("  Crown Fire Calculation Method (0=Finney 1998, 1=Scott&Reinhardt 2001)\n");
		printf("	Desired Resolution of Calculations (in native LCP units)\n\n");

          printf("numargs=%d\n", argc);
          exit(0);
     }

     bool WndOK=false, WtrOK=false;
     long CFMethod, FoliarMC;
	double Resolution;
     FlamMap f;
     ResetNewFuels();

	//chdir("e:\\fb\\mfinney\\farsite4\\schley\\");

	if(argc>12)
		Resolution=atof(argv[12]);
	else
		Resolution=-1.0;
	f.SetResolution(Resolution);


	if(!f.LoadLandscapeFile(argv[1]))
     {    printf("Landscape File Not Found or could not be read, TERMINATING\n");

          exit(0);
     }
     if(!f.LoadFuelMoistureFile(argv[2]))
     {    printf("FuelMoisture File Not Found or could not be read, TERMINATING\n");

          exit(0);
     }
     if(!f.LoadOutputFile(argv[3]))
     {    printf("Output File Not Found or could not be read, TERMINATING\n");

          exit(0);
     }
     if(strcmp(argv[4], "-1"))
     {    if(!f.LoadCustomFuelFile(argv[4]))
          {    printf("CustomFuelModel File Not Found or could not be read, TERMINATING\n");

               exit(0);
          }
     }
     SetConstWind(atof(argv[5]), atof(argv[6]));

     if(strcmp(argv[7], "-1"))
     {    if(!f.LoadWeatherFile(argv[7]))
          {    printf("Weather File Not Found or could not be read, TERMINATING\n");

               exit(0);
          }
		WtrOK=true;
     }
     if(strcmp(argv[8], "-1"))
     {    if(!WtrOK)
          {    printf("Wtr file mising, Wind File irrelevant, TERMINATING\n");

               exit(0);
          }
          if(!f.LoadWindFile(argv[8]))
          {    printf("WindFile File Not Found or could not be read, TERMINATING\n");

               exit(0);
          }
          WndOK=true;
     }
     if(strcmp(argv[9], "-1"))
     {    if(!WtrOK || !WndOK)
          {    printf("Wtr or Wnd file mising, DateFile irrelevant, TERMINATING\n");

               exit(0);
          }
          if(!f.LoadDateFile(argv[9]))
          {    printf("DateFile File Not Found or could not be read, TERMINATING\n");

               exit(0);
          }
     }
     FoliarMC=100;
     CFMethod=0;
     if(argc>10)
     {    FoliarMC=atol(argv[10]);
          if(FoliarMC<50)
               FoliarMC=100;

          if(argc>11)
               CFMethod=atol(argv[11]);
          if(CFMethod<0)
               CFMethod=0;
          else if(CFMethod>1)
               CFMethod=1;
     }

     SetFoliarMoistureContent(FoliarMC);
     SetCrownFireCalculation(CFMethod);
     if(FSPRO_VERSION || NODESPREAD_VERSION)
	     EnableSpotting(true);
     else
     	EnableSpotting(false);

     f.RunFlamMap();

     printf("FlamMap Run Successful, Outputs Written\n");


     bool WndOK=false, WtrOK=false;
     long CFMethod, FoliarMC;
	double Resolution;
     FlamMap f;
     ResetNewFuels();

	//chdir("e:\\fb\\mfinney\\farsite4\\schley\\");

        Resolution=-1.0;
	f.SetResolution(Resolution);


     FoliarMC=100;
     CFMethod=0;

     SetFoliarMoistureContent(FoliarMC);
     SetCrownFireCalculation(CFMethod);
     if(FSPRO_VERSION || NODESPREAD_VERSION)
	     EnableSpotting(true);
     else
     	EnableSpotting(false);

     //f.RunFlamMap();
    LandscapeStruct lcp_data;
    lcp_data.elev = 1000;
    lcp_data.slope = 50;
    lcp_data.aspect = 270;
    lcp_data.fuel = 1;
    lcp_data.cover = 0;
    lcp_data.aspectf = 270;
    lcp_data.height = 0;
    lcp_data.base = 0;
    lcp_data.density = 0;
    lcp_data.duff = 0;
    lcp_data.woody = 0;

    FuelMoistStruct moist_data;
    moist_data.fm_1 = 1;
    moist_data.fm_10 = 3;
    moist_data.fm_100 = 5;
    moist_data.fm_1000 = 5;
    moist_data.fm_herb = 5;
    moist_data.fm_wood = 5;

    float wind_spd = 1.0;
    float wind_dir = 270.0;
    OutputFBStruct output;
    f.burn->pointBurnThread = new BurnThread(f.burn->env);
    f.RunFlamMapPoint(lcp_data, moist_data, wind_spd, wind_dir, output);

     //printf("FlamMap Run Successful, Outputs Written\n");

}
*/
//---------------------------------------------------------------------------





FlamMap::FlamMap()
{
     burn=new Burn();
	Resolution=-1.0;
}


FlamMap::~FlamMap()
{
  	long i;

	
     MaxTime=0.0;
  	if(LandFileOpen)
		LoadLCP();
     FreeWeatherData(0);
     FreeWindData(0);
	for(i=0; i<NUMOUTPUTS; i++)
     	SelectOutputLayer(i, false);

     if(burn)
          delete burn;

     CloseLandFile();
	FreeWeatherData(0);
     FreeWindData(0);
     FreeFarsiteEvents(EVENT_BURN);
     FreeFarsiteEvents(EVENT_MOIST);
     //	FreeFarsiteEvents(EVENT_BURNUP);
     //   FreeFarsiteEvents(EVENT_INTEG);
     //	FreeFarsiteEvents(EVENT_CROSS);
     FreeFarsiteEvents(EVENT_BURN_THREAD);
     FreeFarsiteEvents(EVENT_MOIST_THREAD);
     //	FreeFarsiteEvents(EVENT_BURNUP_THREAD);
     //   FreeFarsiteEvents(EVENT_INTEG_THREAD);
     //   FreeFarsiteEvents(EVENT_CROSS_THREAD);

}


bool FlamMap::LoadLCP()
{
	long i;

     if(LandFileOpen)
     {	CloseLandFile();
     	for(i=0; i<NUMOUTPUTS; i++)
          	FreeOutputLayer(i);
      	LandFileOpen=false;
     }
	if(strlen(GetLCPFileNameJustMade())>0)
     {	SetLandFileName(GetLCPFileNameJustMade());
     	if(!OpenLandFile())
          {    return false;
          }
          else
          {	LandFileOpen=true;
			ReadHeader();
			SetCustFuelModelID(HaveCustomFuelModels());
			SetConvFuelModelID(HaveFuelConversions());
          }
     }

     return true;
}

void FlamMap::StartMoistThread()
{
    unsigned int MoistThreadID;
    //HANDLE hMoistThread;
    TerminateMoist=false;
    //hMoistEvent=CreateEvent(NULL, TRUE, FALSE, "MoistEvent");
    hMoistThread=(HANDLE) ::_beginthreadex(NULL, 0, &FlamMap::RunMoistThread, this, NULL, &MoistThreadID);
    CloseHandle(hMoistThread);
}

unsigned _stdcall FlamMap::RunMoistThread(void *FarIntFace)
{
	static_cast <FlamMap *> (FarIntFace)->MoistThread();

     return 1;
}


void FlamMap::MoistThread()
{
	double Interval;

	// use the shortest interval, ten hour at present, longer ones will also be taken care of
     MoistSimTime=GetActualTimeStep();//+burn.SIMTIME;
     Interval=GetMoistCalcInterval(SIZECLASS_10HR, FM_INTERVAL_TIME);
	do
     {	MoistSimTime+=Interval;
     	burn->env->CalcMapFuelMoistures(MoistSimTime);
          if(TerminateMoist)
          	break;
     } while(MoistSimTime<MaxTime);      // MaxTime is class variable
     TerminateMoist=true;
}


bool FlamMap::RunFuelMoistureCalculations()
{    // from FARSITE 4, PrecalculateFuelMoistures()
	char Line[64]="";
	long mo, dy, hr, mn;
     double Interval, start, end;//, SimTime;

     if(ConstFuelMoistures(GETVAL))
          return true;

     start=GetJulianDays(GetStartMonth());
     start+=GetStartDay();
     start*=1440.0;
     start+=(GetStartHour()/100.0)*60.0;
     end=GetJulianDays(GetEndMonth());
     end+=GetEndDay();
     end*=1440.0;
     end+=(GetEndHour()/100.0)*60.0;
     MaxTime=end-start;

	//SimTime=GetActualTimeStep();//+burn.SIMTIME;

     //if(!burn->env->CalcMapFuelMoistures(MaxTime))//(SimTime))
     //     return false;
	TerminateMoist=false;
	MoistThread();

     return true;
}


void FlamMap::ResetThreads()
{
     burn->env->ResetAllThreads();
     FreeFarsiteEvents(EVENT_BURN);
     FreeFarsiteEvents(EVENT_MOIST);
//	FreeFarsiteEvents(EVENT_BURNUP);
//   FreeFarsiteEvents(EVENT_INTEG);
//	FreeFarsiteEvents(EVENT_CROSS);
     FreeFarsiteEvents(EVENT_BURN_THREAD);
     FreeFarsiteEvents(EVENT_MOIST_THREAD);
//	FreeFarsiteEvents(EVENT_BURNUP_THREAD);
//   FreeFarsiteEvents(EVENT_INTEG_THREAD);
//   FreeFarsiteEvents(EVENT_CROSS_THREAD);
     AllocFarsiteEvents(EVENT_BURN, GetMaxThreads(), "FlamEventBurn", false, false);
     AllocFarsiteEvents(EVENT_MOIST, GetMaxThreads(), "FlamEventMoist", false, false);
//     AllocFarsiteEvents(EVENT_BURNUP, GetMaxThreads(), "SyncEventBurnup", false, false);
//     AllocFarsiteEvents(EVENT_INTEG, GetMaxThreads(), "SyncEventInteg", false, false);
//     AllocFarsiteEvents(EVENT_CROSS, GetMaxThreads(), "SyncEventCross", false, false);
     AllocFarsiteEvents(EVENT_BURN_THREAD, GetMaxThreads(), "FlamEventBurnThread", true, false);
     AllocFarsiteEvents(EVENT_MOIST_THREAD, GetMaxThreads(), "FlamEventMoistThread", true, false);
//     AllocFarsiteEvents(EVENT_BURNUP_THREAD, GetMaxThreads(), "SyncEventBurnupThread", true, false);
//     AllocFarsiteEvents(EVENT_INTEG_THREAD, GetMaxThreads(), "SyncEventIntegThread", true, false);
//     AllocFarsiteEvents(EVENT_CROSS_THREAD, GetMaxThreads(), "SyncEventCrossThread", true, false);
//     NumSimThreads=GetMaxThreads();
}

void FlamMap::StartBurnThread()
{
#ifndef FLMP_ENABLE_THREADS
    burn->StartLandscapeThreads();    // non-threaded version
#else
    unsigned int BurnThreadID;
    TerminateBurn=false;
    hBurnThread=(HANDLE) ::_beginthreadex(NULL, 0, &FlamMap::RunBurnThread, this, NULL, &BurnThreadID);
    //CloseHandle(hBurnThread);
#endif
}

unsigned _stdcall FlamMap::RunBurnThread(void *FarIntFace)
{
    static_cast <FlamMap *> (FarIntFace)->BurnThread();
    return 1;
}

void FlamMap::BurnThread()
{
    TerminateBurn=false;
    burn->StartLandscapeThreads();   // need way to display progress from all threads
    TerminateBurn=true;
}


long FlamMap::RunFlamMapThreads()
{
     RunFuelMoistureCalculations();
     StartBurnThread();
     WaitForSingleObject(hBurnThread, INFINITE);
     CloseHandle(hBurnThread);

     return 1;
}

#ifdef FLMP_ENABLE_OMFFR
void FlamMap::RunFlamMapPoint(LandscapeStruct lcp_data, FuelMoistStruct moist_data,
                              double wind_spd, double wind_dir, OutputFBStruct &output)
{
    SetConstWind(wind_spd, wind_dir);
    SetInitialFuelMoistures(lcp_data.fuel, moist_data.fm_1, moist_data.fm_10,
                            moist_data.fm_100, moist_data.fm_herb,
                            moist_data.fm_wood);
	burn->pointBurnThread->BurnTheLandscape(lcp_data, moist_data.fm_1, moist_data.fm_10, moist_data.fm_100, output);
}

#endif
void FlamMap::RunFlamMap()
{ // multithreaded function
	if(LandFileOpen)
     {	if(InquireOutputSelections())
     	{    long i;
          	//double prog;
               long procs;

               if(GetThreadOpts()==0)
               {	SYSTEM_INFO sysinf;
				GetSystemInfo(&sysinf);
                    procs=sysinf.dwNumberOfProcessors;
               }
               else
               	procs=GetThreadOpts();
			//procs=1;
               SetMaxThreads(procs);
               ResetThreads();
               if(RunFlamMapThreads())
          	{    if(FSPRO_VERSION)
               		WriteFriskToDisk();
               	else
                    {	if(GetOutputOption(FLAMELENGTH))
				     	WriteOutputLayerToDisk(FLAMELENGTH);
					if(GetOutputOption(SPREADRATE))
				     	WriteOutputLayerToDisk(SPREADRATE);
					if(GetOutputOption(INTENSITY))
			     		WriteOutputLayerToDisk(INTENSITY);
					if(GetOutputOption(HEATAREA))
				     	WriteOutputLayerToDisk(HEATAREA);
					if(GetOutputOption(CROWNSTATE))
				     	WriteOutputLayerToDisk(CROWNSTATE);
					if(GetOutputOption(SOLARRADIATION))
				     	WriteOutputLayerToDisk(SOLARRADIATION);
					if(GetOutputOption(FUELMOISTURE1))
				     	WriteOutputLayerToDisk(FUELMOISTURE1);
					if(GetOutputOption(FUELMOISTURE10))
				     	WriteOutputLayerToDisk(FUELMOISTURE10);
					if(GetOutputOption(MIDFLAME))
				     	WriteOutputLayerToDisk(MIDFLAME);
					if(GetOutputOption(HORIZRATE))
			     		WriteOutputLayerToDisk(HORIZRATE);
					if(GetOutputOption(MAXSPREADDIR))
				     	WriteOutputLayerToDisk(MAXSPREADDIR);
					if(GetOutputOption(ELLIPSEDIM_A))
				     	WriteOutputLayerToDisk(ELLIPSEDIM_A);
					if(GetOutputOption(ELLIPSEDIM_B))
				     	WriteOutputLayerToDisk(ELLIPSEDIM_B);
					if(GetOutputOption(ELLIPSEDIM_C))
				     	WriteOutputLayerToDisk(ELLIPSEDIM_C);
					if(GetOutputOption(MAXSPOT))
			     		WriteOutputLayerToDisk(MAXSPOT);

	          	     for(i=0; i<NUMOUTPUTS; i++)
     		          	SelectOutputLayer(i, false);
          		     SetOutputBaseName("");
                    }
               }
          }
     }
}


void FlamMap::ResetFlamMap()
{
	long i;

  	if(LandFileOpen)
		LoadLCP();
     FreeWeatherData(0);
     FreeWindData(0);
	for(i=0; i<NUMOUTPUTS; i++)
     	SelectOutputLayer(i, false);
     SetOutputBaseName("");
     SetCustFuelModelName("");
     SetFuelMoistureFileName("");
     SetWeatherFileName("");
     SetWindFileName("");
     SetLandFileName("");
}


bool FlamMap::InquireOutputSelections()
{	// need to have some outputs selected
	long i;

     for(i=0; i<NUMOUTPUTS; i++)
     {    if(GetOutputOption(i))
     		return true;
     }

	return false;
}


bool FlamMap::LoadLandscapeFile(char *FileName)
{
     CloseLandFile();

     SetLandFileName(FileName);
     if(!OpenLandFile())
     {	//::MessageBox(HWindow, FileName,
     	//	"Error Loading LCP", MB_OK);
          //lcp->SetCheck(BF_UNCHECKED);
     	//tlcp->SetText("");

     	return false;
     }

     LandFileOpen=true;
     ReadHeader();
	if(Resolution>0.0)
		ResampleLandscape(Resolution);
     SetCustFuelModelID(HaveCustomFuelModels());
     SetConvFuelModelID(HaveFuelConversions());

     return true;
}


void FlamMap::SetResolution(double resolution)
{
	if(resolution>0.0)
		Resolution=resolution;

}

bool FlamMap::LoadFuelMoistureFile(char *FileName)
{

	long ModNum, F1, F10, F100, FLW, FLH;
     FILE *CurrentFile;

     if((CurrentFile=fopen(FileName, "r"))==NULL)
     	return false;

	while(!feof(CurrentFile))
     {    fscanf(CurrentFile,"%ld", &ModNum);
          if(feof(CurrentFile))
               break;
          fscanf(CurrentFile, "%ld %ld %ld %ld %ld", &F1, &F10, &F100, &FLH, &FLW);
		if(ModNum>256)
		{	//::MessageBox(HWindow, "Correct Fuel Moisture File Before Proceeding",
			//		"Error: Model Number>50", MB_OK);
               //fms->SetCheck(BF_UNCHECKED);
               //tfms->SetText("");

			return false;
		}
		if(F1<=0 || F10<=0 || F100<=0 || FLW<=0 || FLH<=0)
		{	//::MessageBox(HWindow, "Correct Before Proceeding",
			//		"Error: Fuel Moisture <=0", MB_OK);
               //fms->SetCheck(BF_UNCHECKED);
               //tfms->SetText("");

			return false;
		}
          SetInitialFuelMoistures(ModNum, F1, F10, F100, FLH, FLW);
	}
     fclose(CurrentFile);

     return true;
}


bool FlamMap::LoadCustomFuelFile(char *FileName)
{
	SetCustFuelModelID(false);
	char *ptr, dynamic[32]="", code[32]="";
     char Line[256]="", head[64]="", ErrMsg[256]="", BackupFile[256]="", comment[256]="";
     bool Metric=false, BadFile=false;
	long num, count, ModNum, FileFormat=0;
     double s1, slh, slw;
     NewFuel newfuel, newfuelm;
     FILE *fout;
     FILE *CurrentFile;

     CurrentFile=fopen(FileName, "r");
     memset(&newfuel, 0x0, sizeof(NewFuel));
     memset(&newfuelm, 0x0, sizeof(NewFuel));
     do
     {    rewind(CurrentFile);
     	memset(head, 0x0, 64*sizeof(char));
     	memset(Line, 0x0, sizeof(Line));
	     fgets(Line, 255, CurrentFile);
     	sscanf(Line, "%s", head);
	    	AccessFuelModelUnits(0);
     	if(!strcmp(strlwr(head), "metric"))
	     {    Metric=true;
     	     AccessFuelModelUnits(1);
	     }
     	else if(strcmp(strlwr(head), "english"))   // no header in file
	     {    if(atol(head)>256)
     	     {     //::MessageBox(Client->HWindow, "Bad Header in File", "Custom Fuel Model File Error", MB_OK);
          	     fclose(CurrentFile);

	               return false;
     	     }
               memset(head, 0x0, 64*sizeof(char));
     		rewind(CurrentFile);
	     }
          if(FileFormat>0)
          {    if(FileFormat==1)
          	{  	strcat(BackupFile, FileName);
	               strcat(BackupFile, ".old");
     	          CopyFile(reinterpret_cast<LPCSTR>(FileName), reinterpret_cast<LPCSTR>(BackupFile), false);
          	     fclose(CurrentFile);
               	CurrentFile=fopen(BackupFile, "r");
			     if((access(FileName, 02))==-1)
			     {	SetFileAttributes(reinterpret_cast<LPCSTR>(FileName), FILE_ATTRIBUTE_NORMAL);
				     DeleteFile(reinterpret_cast<LPCSTR>(FileName));
			     }
                    fout=fopen(FileName, "w");
                    if(Metric)
                    	fprintf(fout, "METRIC\n");
                    else
                    	fprintf(fout, "ENGLISH\n");
               }
               else
               	fout=NULL;
          	break;
          }

	     memset(Line, 0x0, 256*sizeof(char));
	     fgets(Line, 255, CurrentFile);
	     if(feof(CurrentFile))
	         	break;
	     if(strlen(Line)==0 || !strncmp(Line, "\n", 1))
	         	continue;
	     num=sscanf(Line, "%ld %s %lf %lf %lf %lf %lf %s",
	               &ModNum, head, &newfuel.h10, &newfuel.h100,
	               &newfuel.lh, &newfuel.lw, &s1, comment);
          //if(!strcmp(strlwr(comment), "dynamic"))
          //  	FileFormat=2;
          //else if(!strcmp(strlwr(comment), "static"))
          //	FileFormat=2;
          //else
          if(strstr(strlwr(comment), "d"))
          	FileFormat=2;
          else if(strstr(strlwr(comment), "s"))
          	FileFormat=2;
          else
          	FileFormat=1;
     } while(FileFormat>0);

     rewind(CurrentFile);
     memset(head, 0x0, 64*sizeof(char));
     fgets(Line, 255, CurrentFile);
    	sscanf(Line, "%s", head);
    	if(strcmp(strlwr(head), "metric") && strcmp(strlwr(head), "english"))
     	rewind(CurrentFile);

     count=0;
	while(!feof(CurrentFile))
	{    memset(Line, 0x0, 256*sizeof(char));
     	fgets(Line, 255, CurrentFile);
          if(strlen(Line)==0 || !strncmp(Line, "\n", 1))
          	continue;
          //if(feof(CurrentFile))
          //	break;
          memset(comment, 0x0, 256*sizeof(char));
          if(FileFormat==1)
          {	num=sscanf(Line, "%ld %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %s",
               	&ModNum, &newfuel.h1, &newfuel.h10, &newfuel.h100,
	               &newfuel.lh, &newfuel.lw, &s1, &slh, &slw,
     	          &newfuel.depth, &newfuel.xmext, &newfuel.heatd, &newfuel.heatl, comment);
          	if(num<13)
          		break;
               sprintf(newfuel.code, "FM%ld", ModNum);
               newfuel.dynamic=0;
          }
          else
          {    memset(code, 0x0, 32*sizeof(char));
          	memset(dynamic, 0x0, 32*sizeof(char));
          	num=sscanf(Line, "%ld %s %lf %lf %lf %lf %lf %s %lf %lf %lf %lf %lf %lf %lf %s",
               	&ModNum, code, &newfuel.h1, &newfuel.h10, &newfuel.h100,
	               &newfuel.lh, &newfuel.lw, dynamic, &s1, &slh, &slw,
     	          &newfuel.depth, &newfuel.xmext, &newfuel.heatd, &newfuel.heatl, comment);
          	if(num<15)
          		break;
               //if(!strcmp(strupr(dynamic), "DYNAMIC"))
               //	newfuel.dynamic=1;
          	if(strstr(strlwr(dynamic), "d"))
               	newfuel.dynamic=1;
               else
               	newfuel.dynamic=0;
               strncpy(newfuel.code, code, 7);
		}

          newfuel.sav1=(long) s1;
          newfuel.savlh=(long) slh;
          newfuel.savlw=(long) slw;
          memset(newfuel.desc, 0x0, 256*sizeof(char));
          if(strlen(comment)>0)
	     {    ptr=strstr(Line, comment);
          	strncpy(newfuel.desc, ptr, 64);
          }
          if(Metric)     // convert to english
          {    memcpy(&newfuelm, &newfuel, sizeof(NewFuel));
          	newfuel.h1/=2.2417;
               newfuel.h10/=2.2417;
               newfuel.h100/=2.2417;
               newfuel.lh/=2.2417;
               newfuel.lw/=2.2417;
               newfuel.sav1=s1*30.480060960;
               newfuel.savlh=slh*30.480060960;
               newfuel.savlw=slw*30.480060960;
               newfuel.depth/=30.480060960;
               newfuel.heatd/=2.324375;
               newfuel.heatl/=2.324375;
          }
          count++;
		if(ModNum<14 || ModNum>256)
		{    sprintf(ErrMsg, "Model Data Error Line => %ld", count);
          	//::MessageBox(Client->HWindow, "Fuel Model Number >256 or <14", ErrMsg, MB_OK);
               BadFile=true;
		}
		else if(newfuel.xmext<=0.0)
		{    sprintf(ErrMsg, "Model Data Error Line => %ld", count);
          	//::MessageBox(Client->HWindow, "Extinction Moisture = 0", ErrMsg, MB_OK);
               BadFile=true;
		}
          else if(newfuel.h1+newfuel.h10+newfuel.h100<=0.0)
          {    sprintf(ErrMsg, "Model Data Error Line => %ld", count);
          	//::MessageBox(Client->HWindow, "Fuel Model Has No Dead Fuel", ErrMsg, MB_OK);
               BadFile=true;
          }
		else if(newfuel.depth<=0.0)
		{    sprintf(ErrMsg, "Model Data Error Line => %ld", count);
          	//::MessageBox(Client->HWindow, "Depth = 0.0", ErrMsg, MB_OK);
               BadFile=true;
		}
		else if(newfuel.heatl<6000)
		{    sprintf(ErrMsg, "Model Data Error Line => %ld", count);
          	//::MessageBox(Client->HWindow, "Live Heat Content Too Low", ErrMsg, MB_OK);
               BadFile=true;
		}
		else if(newfuel.heatd<4000)
		{    sprintf(ErrMsg, "Model Data Error Line => %ld", count);
          	//::MessageBox(Client->HWindow, "Dead Heat Content Too Low", ErrMsg, MB_OK);
               BadFile=true;
		}
		else if(newfuel.sav1>4000 || newfuel.savlh>4000 || newfuel.savlw>4000)
		{    sprintf(ErrMsg, "Model Data Error Line => %ld", count);
          	//::MessageBox(Client->HWindow, "SAV Ratios Out of Range", ErrMsg, MB_OK);
               BadFile=true;
		}

          if(BadFile)
          {    fclose(CurrentFile);
			return false;
          }
		newfuel.xmext/=100.0;
          newfuelm.xmext=newfuel.xmext;
          newfuel.number=newfuelm.number=ModNum;
          SetNewFuel(&newfuel);
	     if(FileFormat==1 && fout!=NULL)
          {    if(newfuel.dynamic==0)
               	sprintf(dynamic, "static");
          	else
               	sprintf(dynamic, "dynamic");
          	if(Metric)
	     		fprintf(fout, "%ld %s %lf %lf %lf %lf %lf %s %ld %ld %ld %lf %ld %lf %lf %s\n",
     	          	newfuelm.number, newfuelm.code, newfuelm.h1, newfuelm.h10, newfuelm.h100,
	     	          newfuelm.lh, newfuelm.lw, dynamic, newfuelm.sav1, newfuelm.savlh, newfuelm.savlw,
     	     	     newfuelm.depth, (long) (newfuelm.xmext*100.0), newfuelm.heatd, newfuelm.heatl, newfuelm.desc);
               else
	     		fprintf(fout, "%ld %s %lf %lf %lf %lf %lf %s %ld %ld %ld %lf %ld %lf %lf %s\n",
     	          	newfuel.number, newfuel.code, newfuel.h1, newfuel.h10, newfuel.h100,
	     	          newfuel.lh, newfuel.lw, dynamic, newfuel.sav1, newfuel.savlh, newfuel.savlw,
     	     	     newfuel.depth, (long) (newfuelm.xmext*100.0), newfuel.heatd, newfuel.heatl, newfuel.desc);
          }
	}
	fclose(CurrentFile);
     if(fout)
     {	fclose(fout);
   		//::MessageBox(Client->HWindow, "Existing File Renamed with .OLD extension",
		//	"Custom Fuel Model File Converted to new Format", MB_OK);
     }

	SetCustFuelModelID(true);

	return true;
}

/*
bool FlamMap::LoadCustomFuelFile(char *FileName)
{
	SetCustFuelModelID(false);
     char UnitsString[256];
     bool Metric=false, BadFile=false;
	long ModNum, ModNumX;
     double FS1, FSLH, FSLW, FHD, FHL, extinct;
	double F1, F10, F100, FLH=1.0, FLW=1.0, FD, FEXT;   // 1000 hr fuels here as place holder

     FILE *CurrentFile;

     CurrentFile=fopen(FileName, "r");
     fscanf(CurrentFile, "%s", UnitsString);
     if(!strcmp(strlwr(UnitsString), "metric"))
          Metric=true;
     else if(strcmp(strlwr(UnitsString), "english"))   // no header in file
     {    if(atol(UnitsString)>50)
          {     //::MessageBox(HWindow, "Bad Header in File", "Custom Fuel Model File Error", MB_OK);
               fclose(CurrentFile);
               //fmd->SetCheck(BF_UNCHECKED);
               //tfmd->SetText("");

               return false;
          }
     	rewind(CurrentFile);
     }

	while(!feof(CurrentFile))
	{    fscanf(CurrentFile, "%ld", &ModNum);
          if(feof(CurrentFile))
               break;
          fscanf(CurrentFile,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
               &F1, &F10, &F100, &FLH, &FLW, &FS1, &FSLH, &FSLW, &FD, &extinct, &FHD, &FHL);
          if(Metric)     // convert to english
          {    F1/=2.2417;
               F10/=2.2417;
               F100/=2.2417;
               FLW/=2.2417;
               FLH/=2.2417;
               FS1*=30.48;
               FSLH*=30.48;
               FSLW*=30.48;
               FD/=30.48;
               FHD/=2.324375;
               FHL/=2.324375;
          }

		if(ModNum<14 || ModNum>50)
		{	//::MessageBox(HWindow, "Correct Before Continuing",
			//	"Error: Fuel Model Number >50 or <14", MB_OK);
               BadFile=true;
		}
		else if(extinct<=0.0)
		{	//::MessageBox(HWindow, "Correct Before Continuing",
			//	"Error: Extinction Moisture = 0", MB_OK);
               BadFile=true;
		}
          else if(F1+F10+F100<=0.0)
          {	//::MessageBox(HWindow, "Correct Before Continuing",
			//	"Error: A Fuel Model Has No Dead Fuel", MB_OK);
               BadFile=true;
          }
		else if(FD<=0.0)
		{	//::MessageBox(HWindow, "Correct Before Continuing",
			//	"Error: Depth = 0.0", MB_OK);
               BadFile=true;
		}
		else if(FHL<6000)
		{	//::MessageBox(HWindow, "Correct Before Continuing",
			//	"Error: Live Heat Content Too Low", MB_OK);
               BadFile=true;
		}
		else if(FHD<4000)
		{	//::MessageBox(HWindow, "Correct Before Continuing",
			//	"Error: Dead Heat Content Too Low", MB_OK);
               BadFile=true;
		}
		else if(FS1>4000 || FSLW>4000 || FSLW>4000)
		{	//::MessageBox(HWindow, "Correct Before Continuing",
			//	"Error: SAV Ratios Out of Range", MB_OK);
               BadFile=true;
		}

          if(BadFile)
          {    fclose(CurrentFile);
			return false;
          }
		ModNumX=ModNum-14;
		FEXT=((double) extinct)/100.0;
		SetFuel(ModNumX, F1, F10, F100, FLH, FLW, (long) FS1, (long) FSLH,
                              (long) FSLW, (long) FHD, (long) FHL, FD, FEXT, "");
	}
	fclose(CurrentFile);
	SetCustFuelModelID(true);

     return true;
}
*/


bool FlamMap::LoadWeatherFile(char *FileName)
{
     long StationNumber;
     long count=0, month, day, ppt, hmorn, haft, Tmin, Tmax, Hmax, Hmin, elref;
     bool Metric=false, BadData=false;
     long i, fpos=0;
     char UnitsString[256]="", ErMsg[256]="";

     FILE *CurrentFile;

     FreeWeatherData(0);

     if((CurrentFile=fopen(FileName, "r"))==NULL)
     	return false;

     fscanf(CurrentFile, "%s", UnitsString);
     if(!strcmp(strlwr(UnitsString), "metric"))
     {    Metric=true;
          fpos=ftell(CurrentFile);
     }
     else if(!strcmp(strlwr(UnitsString), "english"))
     {    Metric=false;
          fpos=ftell(CurrentFile);
     }
     else                        // no header, english default
          rewind(CurrentFile);

	while(!feof(CurrentFile))
	{	fscanf(CurrentFile, "%ld", &month);
		if(feof(CurrentFile))
               break;
		fscanf(CurrentFile,"%ld %ld %ld %ld %ld %ld %ld %ld %ld",
			  &day, &ppt, &hmorn, &haft, &Tmin, &Tmax, &Hmax, &Hmin, &elref);
		count++;
          if(month<1 || month>12)
     	{    sprintf(ErMsg, "%s %ld", "Month Data Out of Range, Line -> ", count);
               //::MessageBox(HWindow, ErMsg, " Bad Weather File", MB_OK);
               BadData=true;
		}
          else if(day<1 || day>31)
		{	sprintf(ErMsg, "%s %ld", "Day Data Out of Range, Line -> ", count);
               //::MessageBox(HWindow, ErMsg, " Bad Weather File",				MB_OK);
               BadData=true;
		}
          else if(hmorn<0 || haft>2400)
		{	sprintf(ErMsg, "%s %ld", "Hour Data Out of Range, Line -> ", count);
               //::MessageBox(HWindow, ErMsg, " Bad Weather File",				MB_OK);
               BadData=true;
		}
          else if(Hmax>100 || Hmin<2)
		{	sprintf(ErMsg, "%s %ld", "Humid Data Out of Range, Line -> ", count);
               //::MessageBox(HWindow, ErMsg, " Bad Weather File",				MB_OK);
               BadData=true;
		}
          else if(elref<0)
		{	sprintf(ErMsg, "%s %ld", "Elevation Too Low, Line -> ", count);
               //::MessageBox(HWindow, ErMsg, " Bad Weather File",  				MB_OK);
               BadData=true;
		}
          if(BadData)
          {	fclose(CurrentFile);
               //wtr->SetCheck(BF_UNCHECKED);

			return false;
          }
	}
	StationNumber=AllocWeatherData(0, count);
	if(StationNumber>=0)
	{	fseek(CurrentFile, fpos, SEEK_SET);
          for(i=0; i<count; i++)
		{	fscanf(CurrentFile,"%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", &month,
				  &day, &ppt, &hmorn, &haft, &Tmin, &Tmax, &Hmax, &Hmin, &elref);
               //if(feof(CurrentFile))
               //     break;
               if(Metric)          // convert to english
               {    ppt*=3.93;     // convert to 1/100 inch
                    Tmin*=1.8;
                    Tmin+=32;
                    Tmax*=1.8;
                    Tmax+=32;
                    elref*=3.28;
               }
			if(Hmax>99)
				Hmax=99;
			if(Hmin>99)
				Hmin=99;
			SetWeatherData(StationNumber, i, month, day, ppt, hmorn, haft, Tmin, Tmax, Hmax, Hmin, elref, 0, 0);
		}
		SetWeatherData(StationNumber, i, 13, day, ppt, hmorn, haft, Tmin, Tmax, Hmax, Hmin, elref, 0, 0);
		fclose(CurrentFile);     // 13 indicates end of weather inputs
	}

     return true;
}


bool FlamMap::LoadWindFile(char *FileName)
{
	long count=0;
     double wss, month, wwwinddir, day, hhour, cloudcover;
     bool Metric=false, BadData=false;
     long i, fpos=0;
     char UnitsString[256]="", ErMsg[256]="";

	FILE *CurrentFile;

     FreeWindData(0);

     if((CurrentFile=fopen(FileName, "r"))==NULL)
     	return false;

     fscanf(CurrentFile, "%s", UnitsString);
     if(!strcmp(strlwr(UnitsString), "metric"))
     {    Metric=true;
          fpos=ftell(CurrentFile);
     }
     else if(!strcmp(strlwr(UnitsString), "english"))
     {    Metric=false;
          fpos=ftell(CurrentFile);
     }
     else
          rewind(CurrentFile);

	while(!feof(CurrentFile))
	{    fscanf(CurrentFile, "%lf", &month);
          if(feof(CurrentFile))
               break;
          fscanf(CurrentFile,"%lf %lf %lf %lf %lf", &day, &hhour, &wss, &wwwinddir, &cloudcover);
		count++;
		if(month<1 || month>12)
		{    sprintf(ErMsg, "%s %ld", "Month Out of Range, Line ->", count);
               //::MessageBox(HWindow, ErMsg, " Bad Wind File", MB_OK);
               BadData=true;
          }
          else if(day>31 || day<1)
		{    sprintf(ErMsg, "%s %ld", "Day Out of Range, Line ->", count);
               //::MessageBox(HWindow, ErMsg, " Bad Wind File", MB_OK);
               BadData=true;
          }
          else if(hhour<0 || hhour>2359)
		{    sprintf(ErMsg, "%s %ld", "Hour Out of Range, Line ->", count);
               //::MessageBox(HWindow, ErMsg, " Bad Wind File", MB_OK);
               BadData=true;
          }
          else if(cloudcover>100 || cloudcover<0)
		{    sprintf(ErMsg, "%s %ld", "Cloud Cover Out of Range, Line ->", count);
               //::MessageBox(HWindow, ErMsg, " Bad Wind File", MB_OK);
               BadData=true;
          }

          if(BadData)
          {    fclose(CurrentFile);
          	//wnd->SetCheck(BF_UNCHECKED);

			return false;
		}
	}
	long StationNumber=AllocWindData(0, count);
	if(StationNumber>=0)
	{    fseek(CurrentFile, fpos, SEEK_SET);
		for(i=0; i<count; i++)
		{    fscanf(CurrentFile,"%lf %lf %lf %lf %lf %lf", &month, &day, &hhour, &wss, &wwwinddir, &cloudcover);
               //if(feof(CurrentFile))
               //     break;
               if(Metric)
                    wss*=0.5402; //(0.62125/1.15);     10m wind kph to 20ft wind mph
			SetWindData(StationNumber, i, (long) month, (long) day, (long) hhour,
               		(long) wss, (long) wwwinddir, (long) cloudcover);
		}
		SetWindData(StationNumber, i, 13, (long) day, (long) hhour, (long) wss,
          			(long) wwwinddir, (long) cloudcover); // signal end of wind data
		fclose(CurrentFile);
	}

     return true;
}


bool FlamMap::SetDates(long StartMonth, long StartDay, long StartHour,
                         long EndMonth, long EndDay, long EndHour)
{
     long StartDate, EndDate;
	char Start[16]="";

     StartDate=GetJulianDays(StartMonth);
     EndDate=GetJulianDays(EndMonth);

     if(StartDate>EndDate)
     {	if(GetWindMonth(0, 0)<=GetMaxWindObs(0))
          {   	//::MessageBox(HWindow, "Start Month Later Than End Month",
          	//	"Illogical Start and End Months", MB_OK);
          	return false;
          }
     }
     else if(StartDate==EndDate)	//compare days
     {	if(StartDay>EndDay)
     	{    //::MessageBox(HWindow, "Start Day Later Than End Day",
          	//	"Illogical Start and End Days", MB_OK);

          	return false;
          }
     }
     StartDate+=StartDay;
     EndDate+=EndDay;
	if(StartDate==EndDate)
     {	if(StartHour>=EndHour)
     	{	//::MessageBox(HWindow, "Start Hour Later Than End Hour",
          	//	"Illogical Start and End Times", MB_OK);

          	return false;
          }
     }
     SetStartMonth(StartMonth);
     SetStartDay(StartDay);
     SetStartHour(StartHour);
     SetStartDate(StartDate);
     SetEndMonth(EndMonth);
     SetEndDay(EndDay);
     SetEndHour(EndHour);
     SetEndDate(EndDate);
     ConstFuelMoistures(0);
     EnvironmentChanged(true, 0, 0);
     EnvironmentChanged(true, 0, 1);
     EnvironmentChanged(true, 0, 2);
     EnvironmentChanged(true, 0, 3);

	return true;
}


void FlamMap::SelectOutput(long LayerID, bool On)
{
     SelectOutputLayer(LayerID, On);
}


bool FlamMap::LoadOutputFile(char *FileName)
{
     if(FSPRO_VERSION)
     {	SelectOutput(MAXSPREADDIR, true);
	     SelectOutput(ELLIPSEDIM_A, true);
     	SelectOutput(ELLIPSEDIM_B, true);
	     SelectOutput(ELLIPSEDIM_C, true);
     	SelectOutput(MAXSPOT, true);
     	SelectOutput(INTENSITY, true);
     }
     else
     {    if(NODESPREAD_VERSION)
		{	SelectOutput(MAXSPREADDIR, true);
			SelectOutput(ELLIPSEDIM_A, true);
     		SelectOutput(ELLIPSEDIM_B, true);
			SelectOutput(ELLIPSEDIM_C, true);
     		SelectOutput(MAXSPOT, true);
			SelectOutput(SPREADRATE, true);
     		SelectOutput(INTENSITY, true);
			SelectOutput(CROWNSTATE, true);
	     	if(ConstFuelMoistures(-1)==0)
		     {	SelectOutput(FUELMOISTURE1, true);
			     SelectOutput(FUELMOISTURE10, true);
			}
		}
		else
		{	SelectOutput(FLAMELENGTH, true);
     		SelectOutput(SPREADRATE, true);
     		SelectOutput(INTENSITY, true);
			//SelectOutput(HEATAREA, true);
     		SelectOutput(CROWNSTATE, true);
			//SelectOutput(SOLARRADIATION, true);
     		//SelectOutput(FUELMOISTURE1, true);
			//SelectOutput(FUELMOISTURE10, true);
     		//SelectOutput(MIDFLAME, true);
			//SelectOutput(HORIZRATE, true);
		}
     }
     SetOutputBaseName(FileName);

     return true;
}


bool FlamMap::LoadDateFile(char *FileName)
{
     long smo, sday, shour, emo, eday, ehour;
     char krap[256]="";
     FILE *fin=fopen(FileName, "r");

     if(fin==0)
     {	printf("File Not Found %s, Not using conditioning period\n\n");

		return true;
     }
     fscanf(fin, "%s %ld\n", krap, &smo);
     fscanf(fin, "%s %ld\n", krap, &sday);
     fscanf(fin, "%s %ld\n", krap, &shour);
     fscanf(fin, "%s %ld\n", krap, &emo);
     fscanf(fin, "%s %ld\n", krap, &eday);
     fscanf(fin, "%s %ld\n", krap, &ehour);

     fclose(fin);

     return SetDates(smo, sday, shour, emo, eday, ehour);
}


