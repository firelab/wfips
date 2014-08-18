#define _POSIX_SOURCE 1
// FlamMap.cpp : Defines the entry point for the DLL application.
//FlamMapLX Ported by:  Adam Moreno of Beartooth Carbon Consulting Inc. 2011
#define BOOL_STR(b) (b?"true":"false")
#include "stdafx.h"
#include "FlamMap_DLL.h"
#include "flammap3.h"
//#include <windows.h>
#include <stdio.h>
#ifdef _MANAGED
#pragma managed(push, off)
#endif

#include <unistd.h>
#define GetCurrentDir getcwd

const double PI=acos(-1.0);
//global instance ID for Event Names
int GFlamMapInstance = 1;
int critsec = 0;
//CRITICAL_SECTION FlamMapInstanceCS;
#ifdef BUILD_FLAMMAPDLL 
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
   // Perform actions based on the reason for calling.
    switch( ul_reason_for_call ) 
    { 
        case DLL_PROCESS_ATTACH:
         // Initialize once for each new process.
         // Return FALSE to fail DLL load.
			if(!critsec)
			{
				InitializeCriticalSection(&FlamMapInstanceCS);
				critsec = 1;
			}
            break;

        case DLL_THREAD_ATTACH:
         // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
         // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:
         // Perform any necessary cleanup.
			if(critsec)
			{
				DeleteCriticalSection(&FlamMapInstanceCS);
				critsec = 0;
			}
            break;
    }
     return TRUE;
}
#endif
#ifdef _MANAGED
#pragma managed(pop)
#endif

CFlamMap::CFlamMap()
{
	m_strLCPName[0] = NULL;
	m_strCustFuelFileName[0] = NULL;
	pFlamMap = new FlamMap();
	m_runningFlamMap = m_moisturesSet = false;
	//EnterCriticalSection(&FlamMapInstanceCS);
	GFlamMapInstance++;
	pFlamMap->SetInstanceID(GFlamMapInstance);
	//LeaveCriticalSection(&FlamMapInstanceCS);
	maxSpotDirLayer = maxSpotDxLayer = NULL;
}
/*
CFlamMap::~CFlamMap()
{
	if(pFlamMap)
		delete pFlamMap;
	if(maxSpotDirLayer)
		delete[] maxSpotDirLayer;
	if(maxSpotDxLayer)
		delete[] maxSpotDxLayer;
/*ALM	if(windNinjaDirLayer)
		delete[] windNinjaDirLayer;
	if(windNinjaSpeedLayer)
		delete[] windNinjaSpeedLayer;
		*
}
*/
FLAMMAPRUNSTATE CFlamMap::GetRunState()
{
	if(pFlamMap)
		return pFlamMap->runState;
	return Stopped;
}

int CFlamMap::GetTotalRunTime()
{
	if(	pFlamMap)
	{
		if(pFlamMap->timeLaunch > 0)
		{
			if(pFlamMap->timeFinish > 0)
			{
				time_t elapsed = pFlamMap->timeFinish - pFlamMap->timeLaunch;
				if(elapsed > 0)
				{
					return elapsed;
				}
			}
			//must be running...
			time_t now;
			time( &now );
			if(now - pFlamMap->timeLaunch > 0)
			{
				return now - pFlamMap->timeLaunch;
			}
			//something's wrong...
			return -1;
		}

	}
	return 0;

}

int CFlamMap::GetTheme_DistanceUnits()
{
	return pFlamMap->GetTheme_DistanceUnits();
}

int CFlamMap::GetNumCols()
{
	return pFlamMap->GetNumEast();//>Header.numeast;
}

int CFlamMap::GetNumRows()
{
	return pFlamMap->GetNumNorth();//>Header.numnorth;
}

double CFlamMap::GetWest()
{
	printf("GetWest1 = %f", pFlamMap->GetWestUtm());
	return pFlamMap->GetWestUtm();//>Header.WestUtm;
}

double CFlamMap::GetSouth()
{
	return pFlamMap->GetSouthUtm();//>Header.SouthUtm;
}

double CFlamMap::GetEast()
{
	return pFlamMap->GetEastUtm();//>Header.WestUtm;
}

double CFlamMap::GetNorth()
{
	return pFlamMap->GetNorthUtm();//>Header.SouthUtm;
}

double CFlamMap::GetCellSize()
{
	return pFlamMap->Header.XResol;
}

int CFlamMap::SetAnalysisArea(double tEast, double tWest, double tNorth, double tSouth)
{
	return pFlamMap->SetAnalysisArea(tEast, tWest, tNorth, tSouth);
}
/*
int CFlamMap::TrimLandscape()
{
	return pFlamMap->TrimLandscape();
}
*/
float CFlamMap::GetLayerValueByCell(int _layer, int col, int row)
{
	return pFlamMap->GetLayerValueByCell(_layer, col, row);
}

int CFlamMap::GetUnits(int _layer)
{
	return pFlamMap->GetTheme_Units(_layer);
}

float CFlamMap::GetLayerValue(int _layer, double east, double north)
{
	float ret = -1;
	celldata tCell;
	crowndata tCrown;
	grounddata tGround;
	//int64_t posit = pFlamMap->GetCellPosition(east, north);
	//pFlamMap->GetCellDataFromMemory(posit, tCell, tCrown, tGround);
	int64_t posit;// = pFlamMap->GetCellPosition(east, north);
	pFlamMap->CellData(east, north, tCell, tCrown, tGround, &posit);
	switch(_layer)
	{
	case E_DATA:
		ret = tCell.e;
		break;
	case S_DATA:
		ret = tCell.s;
		break;
	case A_DATA:
		ret = tCell.a;
		break;
	case F_DATA:
		ret = tCell.f;
		break;
	case C_DATA:
		ret = tCell.c;
		break;
	case H_DATA:
		ret = tCrown.h;
		break;
	case B_DATA:
		ret = tCrown.b;
		break;
	case P_DATA:
		ret = tCrown.p;
		break;
	case D_DATA:
		ret = tGround.d;
		break;
	case W_DATA:
		ret = tGround.w;
		break;
	}
	return ret;
}

char *CFlamMap::GetLandscapeFileName()
{
	return m_strLCPName;
}

//initialization, associate with a landscape file
int CFlamMap::SetLandscapeFile(char *_lcpFileName)
{
	strcpy(m_strLCPName, _lcpFileName);
	int ret = (int)pFlamMap->LoadLandscapeFile(m_strLCPName, this);
	return ret;
}

int CFlamMap::LoadLCPHeader(headdata head)
{
	int ret = (int)pFlamMap->LoadLCPHeader(head);
	return ret;
}

int CFlamMap::LoadLandscape(short landscape)
{
	int ret = (int)pFlamMap->LoadLandscape(landscape);
	return ret;
}

int CFlamMap::SetUseDiskForLCP(int set)
{
	if(pFlamMap)
	{
		pFlamMap->CantAllocLCP = (set == 0) ? false : true;
		return 1;
	}
	return 0;
}

int CFlamMap::SetFuelMoistureFile(char *_fuelMoistureFileName)
{
	int ret = (int)pFlamMap->LoadFuelMoistureFile(_fuelMoistureFileName);
	m_moisturesSet = true;
	return ret;
}

char *CFlamMap::GetFuelMoistureFile()
{
	//int ret = (int)pFlamMap->LoadFuelMoistureFile(_fuelMoistureFileName);
	return pFlamMap->FuelMoistureFileName;
}

int CFlamMap::SetCustomFuelsFile(char *_fuelMoistureFileName)
{
	if(pFlamMap->LoadCustomFuelFile(_fuelMoistureFileName))
	{
		strcpy(m_strCustFuelFileName, _fuelMoistureFileName);
		return 1;
	}
	return 0;
}

char *CFlamMap::GetCustomFuelsFile()
{
	return m_strCustFuelFileName;
}

double CFlamMap::SetCalculationResolution(double _resolution)
{
	//always native lcp resolution for now, parameter ignored....
	return 0.0;
}
double CFlamMap::GetCalculationResolution()
{
	if(strlen(pFlamMap->LandFName) > 0)
	{
		return pFlamMap->Header.XResol;
	}
	return 0;
}

int CFlamMap::SetStartProcessor(int _procNum)
{
	return pFlamMap->SetStartProcessor(_procNum);
}

int CFlamMap::GetStartProcessor()
{
	return pFlamMap->GetStartProcessor();
}

int CFlamMap::SetNumberThreads(int _numThreads)
{
	pFlamMap->icf.i_NumPro = _numThreads;
	return 0;
}
int CFlamMap::GetNumberThreads()
{
	//alm SYSTEM_INFO sysInfo;
	//ALM GetSystemInfo(&sysInfo);
	int numCPU;
	 numCPU = sysconf( _SC_NPROCESSORS_ONLN );
	return numCPU;
}

int CFlamMap::GetNumberMTTThreads()//get MTT Threads from Inputs file...
{
	return pFlamMap->icf.i_NumPro;
}

double CFlamMap::GetMTT_SpotProbability()
{
	return pFlamMap->icf.f_MTT_SpotProbability;
}

int CFlamMap::SetMoistures(int fuelModel, int _fm1, int _fm10, int _fm100,
		int _fmHerb, int _fmWoody)
{
	int ret = pFlamMap->SetMoistures(fuelModel, _fm1, _fm10, _fm100, _fmHerb, _fmWoody);
	if(ret)
		m_moisturesSet = true;
	return ret;
}

int CFlamMap::SetAllMoistures(int _fm1, int _fm10, int _fm100,
		int _fmHerb, int _fmWoody)
{
	pFlamMap->SetAllMoistures( _fm1, _fm10, _fm100, _fmHerb, _fmWoody);
	m_moisturesSet = true;
	return 1;//always succeeds
}

int CFlamMap::SetWindType(int _windType)
{//dummy, always const
	return 0;
}

int CFlamMap::GetWindType()
{
	//dummy. always const
	return 1;
}

int CFlamMap::SetWindDirection(double _windDir)
{
	pFlamMap->ConstWindDirection = _windDir;
	return 1;
}

double CFlamMap::GetWindDirection()
{
     return pFlamMap->ConstWindDirection;
}

int CFlamMap::SetConstWind(double _windSpeed, double _windDir)
{
	pFlamMap->SetConstWind(_windSpeed, _windDir);
	return 1;
}

int CFlamMap::SetWindSpeed(double _windSpeed)
{
	pFlamMap->ConstWindSpeed = _windSpeed;
	return 1;
}

double CFlamMap::GetWindSpeed()
{
	return pFlamMap->ConstWindSpeed;
}

int CFlamMap::SetCanopyHeight(double _height)
{
	pFlamMap->CanopyChx.Height=pFlamMap->CanopyChx.DefaultHeight= _height;
	return 1;
}

double CFlamMap::GetCanopyHeight()
{
	return pFlamMap->CanopyChx.Height;
}

int CFlamMap::SetCanopyBaseHeight(double _baseHeight)
{
	pFlamMap->CanopyChx.CrownBase=pFlamMap->CanopyChx.DefaultBase=_baseHeight;
	return 1;
}

double CFlamMap::GetCanopyBaseHeight()
{
	return pFlamMap->CanopyChx.CrownBase;

}

int CFlamMap::SetBulkDensity(double _bulkDensity)
{
	pFlamMap->CanopyChx.BulkDensity=pFlamMap->CanopyChx.DefaultDensity=_bulkDensity;
	return 1;
}

double CFlamMap::GetBulkDensity()
{
	return pFlamMap->CanopyChx.BulkDensity;
}

int CFlamMap::SetFoliarMoistureContent(int _moisturePercent)
{
	pFlamMap->SetFoliarMoistureContent(_moisturePercent);	
	return 1;
}

double CFlamMap::GetFoliarMoistureContent()
{
	return pFlamMap->GetFoliarMC();
}

//fuel moisture conditioning

/*ALM int CFlamMap::SetUseFixedFuelMoistures(int _useFixedFuels)
{
	int set = _useFixedFuels;
	if(_useFixedFuels < 0)
		set = 1;
		
	pFlamMap->ConstFuelMoistures( set);
	return 1;
}
*/
int CFlamMap::GetUseFixedFuelMoistures()
{
	return (int)pFlamMap->ConstFuelMoistures(-1);
}



int CFlamMap::SetWindsFile(char *_wndFileName)
{
	int ret = pFlamMap->LoadWindFile(_wndFileName);
	if(ret)
		pFlamMap->SetWindFileName(_wndFileName);
	return ret;
}

char *CFlamMap::GetWindsFile()
{
	return pFlamMap->GetWindFileName();
}

int CFlamMap::SetConditioningPeriod(int _startMonth, int _startDay, 
									int _startHour, int _startMinute,
									int _endMonth, int _endDay, 
									int _endHour, int _endMinute)
{
	pFlamMap->SetStartMonth(_startMonth);
	pFlamMap->SetStartDay(_startDay);
	pFlamMap->SetStartHour(_startHour*100);
	pFlamMap->SetStartMin(_startMinute);
	pFlamMap->SetEndMonth(_endMonth);
	pFlamMap->SetEndDay(_endDay);
	pFlamMap->SetEndHour(_endHour*100);
	pFlamMap->SetEndMin(_endMinute);
	return true;
}

int CFlamMap::GetConditioningStartMonth()
{
	return pFlamMap->GetStartMonth();
}

int CFlamMap::GetConditioningStartDay()
{
	return pFlamMap->GetStartDay();
}

int CFlamMap::GetConditioningStartHour()
{
	return pFlamMap->GetStartHour();
}

int CFlamMap::GetConditioningStartMinute()
{
	return pFlamMap->GetStartMin();
}

int CFlamMap::GetConditioningEndMonth()
{
	return pFlamMap->GetEndMonth();
}

int CFlamMap::GetConditioningEndDay()
{
	return pFlamMap->GetEndDay();
}

int CFlamMap::GetConditioningEndHour()
{
	return pFlamMap->GetEndHour();
}

int CFlamMap::GetConditioningEndMinute()
{
	return pFlamMap->GetEndMin();
}

int CFlamMap::SetUseScottReinhardt(int _select)
{
	return pFlamMap->SetCrownFireCalculation(_select);
	//return 1;
}

int CFlamMap::GetUseScottReinhardt()
{
	return pFlamMap->GetCrownFireCalculation();
}

int CFlamMap::SetSpreadDirectionFromNorth(int _select, double _degrees)
{
	pFlamMap->SetOutputDirection((_select != 0) ? ABSOLUTEDIR : RELATIVEDIR);
	pFlamMap->SetOffsetFromMax(_degrees);
	return 1;
}

int CFlamMap::GetUseSpreadDirectionFromNorth()
{
	int ret = 0;
	if(pFlamMap->GetOutputDirection() == RELATIVEDIR)
		ret = 1;
	return ret;
}

double CFlamMap::GetSpreadDirectionFromNorth()
{
	return pFlamMap->GetOffsetFromMax();
}

//output memory or file based
int CFlamMap::SelectOutputReturnType(int _returnType)
{//0 = memory, 1 = files
	m_outputsType = _returnType;
	return 1;
}
//outputs to calculate
int CFlamMap::SelectOutputLayer(int _layer, int _select)
{
	printf("In CFlamMap::SelectOutputLayer\n");
	return pFlamMap->SelectOutputLayer(_layer, _select);
}

float *CFlamMap::GetOutputLayer(int _layer)
{
	/*if(pFlamMap->outlayer[_layer])
	{
		float *retLayer = 
	}*/
	int64_t i,j, nR = GetNumRows(), nC = GetNumCols();
	switch(_layer)
	{
	case MAXSPOT_DIR:
		{
			if(!pFlamMap->outlayer[MAXSPOT])
				return NULL;
			if(maxSpotDirLayer)
			{
				delete[] maxSpotDirLayer;
				maxSpotDirLayer = NULL;
			}
			maxSpotDirLayer = new float[nC * nR];
			for(i=0; i < GetNumRows(); i++)
			{  	
				for(j=0; j < GetNumCols(); j++)
			 {
					 double tmp = pFlamMap->outlayer[MAXSPOT][i*nC+j];
					 double dist;
					 double wdir = 0.0;
					 if(tmp != 0.0)
						 wdir = modf(tmp, &dist);
					 wdir *= 10.0;
					 wdir *= 180.0/PI;
					 maxSpotDirLayer[i*nC + j] = wdir;
				}
			}
			return maxSpotDirLayer;
		}
	case MAXSPOT_DX:
		{
			if(!pFlamMap->outlayer[MAXSPOT])
				return NULL;
			if(maxSpotDxLayer)
			{
				delete[] maxSpotDxLayer;
				maxSpotDxLayer = NULL;
			}
			maxSpotDxLayer = new float[nC * nR];
			for(i=0; i < GetNumRows(); i++)
			{  	
				for(j=0; j < GetNumCols(); j++)
			 {
					 double tmp = pFlamMap->outlayer[MAXSPOT][i*nC+j];
					 double dist = 0.0;
					 double wdir = 0.0;
					 if(tmp != 0.0)
						 wdir = modf(tmp, &dist);
					 maxSpotDxLayer[i*nC + j] = dist;
				}
			}
			return maxSpotDxLayer;

		}
		break;
	/*ALM case WINDDIRGRID:
		{
			int64_t numNorth = pFlamMap->nWindRows, numEast = pFlamMap->nWindCols;
			if(!pFlamMap->windDirGrid)
				return NULL;
			if(windNinjaDirLayer)
			{
				delete[] windNinjaDirLayer;
				windNinjaDirLayer = NULL;
			}
			windNinjaDirLayer = new float[numNorth*numEast];
			float wgVal;
			for(int64_t i = 0; i < numNorth; i++)
			{
				for(int64_t j = 0; j < numEast; j++)
				{
					windNinjaDirLayer[i*numEast + j] = pFlamMap->windDirGrid[i][j];
				}
			}	
			return windNinjaDirLayer;
		}*/
	/*ALM case WINDSPEEDGRID:
		{
			if(!pFlamMap->windSpeedGrid)
				return NULL;
			int64_t numNorth = pFlamMap->nWindRows, numEast = pFlamMap->nWindCols;
			if(windNinjaSpeedLayer)
			{
				delete[] windNinjaSpeedLayer;
				windNinjaSpeedLayer = NULL;
			}
			windNinjaSpeedLayer = new float[numNorth*numEast];
			float wgVal;
			for(int64_t i = 0; i < numNorth; i++)
			{
				for(int64_t j = 0; j < numEast; j++)
				{
					windNinjaSpeedLayer[i*numEast + j] = pFlamMap->windSpeedGrid[i][j];
				}
			}	
			return windNinjaSpeedLayer;
			*/

			/*int numNorth = pFlamMap->nWindRows, numEast = pFlamMap->nWindCols;
			lastOutputLayer = new float[numNorth*numEast];
			float wgVal;
			for(int i = 0; i < numNorth; i++)
			{
				for(int j = 0; j < numEast; j++)
				{
					if(_layer == WINDDIRGRID)
						wgVal = pFlamMap->windDirGrid[i][j];
					else//assume speed
						wgVal =pFlamMap->windSpeedGrid[i][j];
					lastOutputLayer[i*numEast + j] = wgVal;
				}
			}*/
		//}
		break;
	default:
		//lastOutputLayer  = new float[GetNumCols() * GetNumRows()];
		//memcpy(lastOutputLayer, pFlamMap->outlayer[_layer], GetNumCols() * GetNumRows() * sizeof(float));
		return pFlamMap->outlayer[_layer];
	}
	return NULL;
}
char *CFlamMap::GetTempOutputFileName(int _layer, char buf[], int sz)
{
	if(buf)
	{
		buf[0] = 0;
		if(_layer >= 0 || _layer < NUMOUTPUTS)
		switch(_layer)
		{
		case MAXSPOT_DIR:
		case MAXSPOT_DX:
			{//need to create a new file with cracked MAXSPOT values
				//if(pFlamMap->outFiles[MAXSPOT].m_hFile != CFile::hFileNull && pFlamMap->outFiles[MAXSPOT].GetFilePath().GetLength() > 0)
				//{
				char bName[512];
				pFlamMap->GetNextBinFileName(bName);
				//CA2T szr(bName);
				//ALM CFile tFile;
				FILE * tFile;//ALM
				tFile = fopen(bName, "w+");//ALM
				//if(tFile.Open(szr, FILE::modeCreate | FILE::modeReadWrite | FILE::typeBinary | FILE::shareDenyNone) == 0)//error Opening file...
				if (tFile==NULL)//ALM
					return buf;
				//crack the contents of MAXSPOT and put ito new file...
				int numNorth = pFlamMap->GetNumNorth(), numEast = pFlamMap->GetNumEast();//, layerType = Layer;
				int64_t offset;
				float spotVal = 0.0, sdir = 0.0, dist = 0.0;
				for(int64_t i = 0; i < numNorth; i++)
				{
					for(int64_t j = 0; j < numEast; j++)
					{
						offset = i * (int64_t)numEast + j;
						spotVal = pFlamMap->GetOutputLayerVal(MAXSPOT, offset);//outlayer[MAXSPOT][i*GetNumEast()+j];
						double spotVal2 = spotVal;
						double dist2 = dist;
						double spotval = spotVal2;
						double dist = dist2;
						if(spotVal != 0.0)
							sdir = modf(spotVal, &dist);
						else
						{
							sdir = 0.0;
							dist = 0.0;
						}
						sdir *= 10.0;
						if(_layer == MAXSPOT_DIR)
						{
							sdir *= 180.0/PI;
							//fwrite(&wdir, sizeof(float), 1, stream);
							//ALM tFile.Write(&sdir, sizeof(float));
							fwrite(&sdir, sizeof(float), 1, tFile);
						}
						else//distance
							fwrite(&dist, sizeof(float), 1, tFile);
							//ALM tFile.Write(&dist, sizeof(float));
					}
				}
				//ALM tFile.Close();
				fclose (tFile);
				strncpy(buf, bName, sz);
			}
		break;
		case WINDDIRGRID:
		case WINDSPEEDGRID:
			break;//not supported
		default:
			{
				//ALM pFlamMap->outFiles[_layer].Flush();
				printf("in GetTempOutputFileName flushing file %i", _layer);
				fflush (pFlamMap->outFiles[_layer]);//ALM
				//fpos_t pos;
				//fgetpos(pFlamMap->outFiles[_layer], pos);
				printf("\nERROR: attempting to set temp memory file in defualt case.  See code FlamMap.cpp *CFlamMap::GetTempOutputFileName\n");
				//FILE* pos = pFlamMap->outFiles[_layer]
				//fscanf(pos, %s, buf);
				//CW2A pz(pFlamMap->outFiles[_layer].GetFilePath());
				//strncpy(buf, pz, sz);
				//CW2A pz(pFlamMap->outFiles[_layer].GetFilePath());
				//ALM strncpy(buf, pFlamMap->outFiles[_layer], sz);
			}
	}
	return buf;
}
	return NULL;
}

char *CFlamMap::GetOutputGridFileName(int _layer, char *buf, int bufLen)
{//stub does nothing!
	return buf;
}

int CFlamMap::CanLaunchFlamMap()
{
	int ret = 0;//assume false
	if(m_runningFlamMap)
	{
		printf("Can not launch FlamMap: Object already running!\n");
		ret = 0;
	}
	if(strlen(pFlamMap->GetLandFileName()) <= 0)
	{
		printf("Can not launch FlamMap: No landscape file loaded\n");
		ret = 0;
	}
	if(m_moisturesSet)
	{
		//ret = pFlamMap->IndividualFuelMoisturesSet();
		ret = 1;
	}
	else
	{
		printf("Can not launch FlamMap: Moistures not set\n");
		ret = 0;
	}
	return ret;

}

void CFlamMap::ResetBurn()
{
	pFlamMap->ResetBurn();
}

/* ---------------------------------------------------------------------------
* Name: LaunchFlamMap
* Desc: prepare and run FlamMap
* Note-1: If windninja gridded winds is specified in the inputs file it will 
*          get run. 
*     
*  Ret: 1 = OK   
*       0 = error occured while attempting to run FlamMap
*     < 0 = (negative number)error occured while attempt to run WindNinja, 
*           the return value is the error number use the following function
*           to retreive the error message cflammap.CommandFileError(errnum);
----------------------------------------------------------------------------- */
int CFlamMap::LaunchFlamMap(int _runAsync)
{
int i; 
int procs;
	 //printf("in launchFlammap\n");
  if ( !CanLaunchFlamMap() ){
   // printf("!CanLaunchFlamMap()\n");
	  return 0; 
  }
		m_runningFlamMap = true;
		pFlamMap->EnableSpotting(1);
	
	 //ALM pFlamMap->ResetThreads();

/* Gridded Winds - Windninja, LoadGridWIndsWN2() will check and */
/*  only do only if previous set to do so */   
	if (pFlamMap->icf.b_hasValidGriddedWindsFiles)	
		 i = pFlamMap->LoadGridWindsFromFiles(&i);
	else
		i = pFlamMap->LoadGridWindsWN2(&i);

  if ( i < 0 ) {   /* error, bad inputs or ninja pooped out */
     char *a = this->CommandFileError(i); 
     printf ("Error: %s \n", a); 
     m_runningFlamMap = false;
     return i; }  /* return negative error code, see notes above */


  printf("Beginning Burn");
  i = pFlamMap->RunFlamMapThreads();   
 // printf("In LaunchFlamMap i = %i m_runningFlamMap = %s\n", i, BOOL_STR(m_runningFlamMap));
  if ( i == 0 ) {                  /* Error occured */
	  printf("in Flammap i == 0");
	    m_runningFlamMap = false;
     return 0; }
  /*
	 if ( this->QueryFlamMapProgress() != 1.0){
		 printf("QueryFlamMapProgress() !=1\n");
		 printf("%f", this->QueryFlamMapProgress());
		   m_runningFlamMap = false;
     return 0; }
	 */
 	m_runningFlamMap = false;
 	return 1;
}


/************************************************************************/
/*ALM double CFlamMap::GetConditioningProgress()
{
	if(!pFlamMap)
		return 0.0;
	return pFlamMap->conditioningProgress;
}
*/
/*ALM double CFlamMap::GetWindNinjaProgress()
{
	if(!pFlamMap)
		 return 0.0;
	return pFlamMap->WN2.Get_Progress();
}*/

double CFlamMap::GetFireBehaviorProgress()
{
	int i, NumTasks;
	double Finished = 0.0, *prog;

	prog=pFlamMap->GetThreadProgress();
	NumTasks=pFlamMap->GetMaxThreads();
	Finished = 0.0;
	for(i=0; i<NumTasks; i++)
		Finished+=prog[i];
	Finished/=(double) NumTasks;
	return Finished;
}


/***********************************************************************/
double CFlamMap::QueryFlamMapProgress()
//returns percent complete as integer 0..100
{
int i, NumTasks;
float f; 
double Finished = 0.0, *prog,d;
 printf("in query\n");
 //printf("%s", pFlamMap->runState);
  	switch(pFlamMap->runState) {
    	case Stopped:
			printf("Stopped");
	    case Loading:
			printf("Loading");
		     return 0.0;

	    //case WindNinja:
       //d = pFlamMap->WN2.Get_Progress(); 
       //return d * 0.2; /* Assume 20% of FlamMap runtime is in ninja */
       //break; 

    	/*ALM case Conditioning:
       f = pFlamMap->cfmc.Get_ProgressF(); 
       return 0.2 + ( (double) f * 0.6 ); 
 */
    	case FireBehavior:
			printf("FireBehavior");
		     return 0.8 + (GetFireBehaviorProgress() * 0.2);//100% when FireBehavior completes

    	case Complete:
			printf("Complete");
		     return 1.0;
	
     default: //invalid state!
		 printf("invalid");
		     return 0.0;
	}

}


/*******************************************************************/
bool CFlamMap::ResetFlamMapProgress()
{
	if(m_runningFlamMap)
		return false;
	pFlamMap->ResetProgress();
	return true;

}

int CFlamMap::CancelFlamMap()
{
	printf("TerminateBurn = True in CancelFlammap\n");
	pFlamMap->TerminateBurn = pFlamMap->TerminateMoist = true;
 //pFlamMap->WN2.Cancel();
//ALM pFlamMap->cfmc.Terminate(); 
	return 1;
}

int CFlamMap::WriteOutputLayerToDisk(int _layer, char *name)
{
	//printf("in WriteOutputLayerToDisk %s\n", name);
	return pFlamMap->WriteOutputLayerToDisk(_layer, name);
}

int CFlamMap::WriteOutputLayerBinary(int _layer, char *name)
{
	return pFlamMap->WriteOutputLayerBinary(_layer, name);
}

int CFlamMap::CritiqueTXT(char *FileName)
{
	return pFlamMap->CritiqueTXT(FileName);
}
/*ALM
int CFlamMap::CritiquePDF(char *FileName)
{
	return pFlamMap->CritiquePDF(FileName);
}
*/
/*ALM
int CFlamMap::GenerateImage(int _layer, char *_fileName)
{
	return pFlamMap->GenerateImage(_layer, _fileName);
}
*/
/*ALM
int CFlamMap::GenerateLegendImage(int _layer, char *_fileName)
{
	return pFlamMap->GenerateLegendImage(_layer, _fileName);
}
*/

int CFlamMap::ResampleLandscape(double NewRes)
{
	return pFlamMap->ResampleLandscape(NewRes);
}

int CFlamMap::LoadFlamMapTextSettingsFile(char *FileName)
{

	return 0;
}

/*........................................................
* Desc: High level call to read, check and set contents
*        of the input command file 
* Note-1: If the command file loads and checks out OK, then
*          the mandatory fuel moistures have been set, 
*          So m_moisturesSet needs to be set here in the
*          CFlamMap class where we can get at it
* Ret: 1 = ok, else error code, see CommandFileError()         
..........................................................*/
int CFlamMap::CommandFileLoad(char cr_PthFN[] )
{
int i;
  i = pFlamMap->LoadInputFile (cr_PthFN);
  if ( i == 1 )              /* OK                     */
    m_moisturesSet = true;   /* See Note-1 above       */
  else
    m_moisturesSet = false;
  return i; 
}


/* Get the error text message from the error number */
char *CFlamMap::CommandFileError(int i_ErrNum)
{
  return pFlamMap->LoadInputError (i_ErrNum);
}

char *CFlamMap::GetErrorMessage(int errNum)
{
	return CommandFileError(errNum);
}


int CFlamMap::LoadFuelConversionFile(char *_fileName)
{
	FILE *fcnv = fopen(_fileName, "rt");
	if(!fcnv)
		return 0;
	char buf[256];
	fgets(buf, 256, fcnv);
	int fmFrom, fmTo, nRead, count = 0;
	while(!feof(fcnv))
	{
		if(strlen(buf) > 0)
		{
			nRead = sscanf(buf, "%d %d", &fmFrom, &fmTo);
			if(nRead == 2)
			{
				if(fmFrom > 0 && fmFrom < 257 && fmTo > 0 && fmTo < 257)
				{
					pFlamMap->SetFuelConversion(fmFrom, fmTo);
					count++;
				}
			}
		}
		fgets(buf, 256, fcnv);
	}
	fclose(fcnv);
	return count;
}


double CFlamMap::GetMttResolution()
{
	return pFlamMap->icf.f_Res;
}
int CFlamMap::GetMttSimTime()
{
	return pFlamMap->icf.i_SimTim;
}
int CFlamMap::GetMttPathInterval()
{
	return pFlamMap->icf.i_TraPth;
}
char *CFlamMap::GetMttIgnitionFile()
{
	return pFlamMap->icf.cr_IFN;
}

char *CFlamMap::GetMttBarriersFile()
{
	return pFlamMap->icf.cr_BarrierFileName;
}

int CFlamMap::GetMttFillBarriers()
{
	return pFlamMap->icf.i_MTT_FillBarriers;
}
/*ALM
float CFlamMap::GetDataUnitsVal(float val, CLegendData *pLegend)
{
	return pFlamMap->GetDataUnitsVal(val, pLegend);
}

float CFlamMap::GetDisplayVal(float val, CLegendData *pLegend)
{
	return pFlamMap->GetDisplayVal(val, pLegend);
}
*/
	//Header Access helpers....
int CFlamMap::GetLayerHi(int layer)
{
	switch(layer)
	{
	case ELEV:
		return pFlamMap->Header.hielev;
	case SLOPE:
		return pFlamMap->Header.hislope;
	case ASPECT:
		return pFlamMap->Header.hiaspect;
	case FUEL:
		return pFlamMap->Header.hifuel;
	case COVER:
		return pFlamMap->Header.hicover;
	case HEIGHT:
		return pFlamMap->Header.hiheight;
	case BASE_HEIGHT:
		return pFlamMap->Header.hibase;
	case BULK_DENSITY:
		return pFlamMap->Header.hidensity;
	case WOODY:
		return pFlamMap->Header.hiwoody;
	case DUFF:
		return pFlamMap->Header.hiduff;
	}
	return NODATA_VAL;
}

int CFlamMap::GetLayerLo(int layer)
{
	switch(layer)
	{
	case ELEV:
		return pFlamMap->Header.loelev;
	case SLOPE:
		return pFlamMap->Header.loslope;
	case ASPECT:
		return pFlamMap->Header.loaspect;
	case FUEL:
		return pFlamMap->Header.lofuel;
	case COVER:
		return pFlamMap->Header.locover;
	case HEIGHT:
		return pFlamMap->Header.loheight;
	case BASE_HEIGHT:
		return pFlamMap->Header.lobase;
	case BULK_DENSITY:
		return pFlamMap->Header.lodensity;
	case WOODY:
		return pFlamMap->Header.lowoody;
	case DUFF:
		return pFlamMap->Header.loduff;
	}
	return NODATA_VAL;
}

int CFlamMap::GetLayerHeaderValCount(int layer)
{
	switch(layer)
	{
	case ELEV:
		return pFlamMap->Header.numelev;
	case SLOPE:
		return pFlamMap->Header.numslope;
	case ASPECT:
		return pFlamMap->Header.numaspect;
	case FUEL:
		return pFlamMap->Header.numfuel;
	case COVER:
		return pFlamMap->Header.numcover;
	case HEIGHT:
		return pFlamMap->Header.numheight;
	case BASE_HEIGHT:
		return pFlamMap->Header.numbase;
	case BULK_DENSITY:
		return pFlamMap->Header.numdensity;
	case WOODY:
		return pFlamMap->Header.numwoody;
	case DUFF:
		return pFlamMap->Header.numduff;
	}
	return -1;
}

int CFlamMap::GetLayerHeaderVal(int layer, int loc)
{
	if(layer < ELEV || layer > WOODY)
		return NODATA_VAL;
	switch(layer)
	{
	case ELEV:
		if(loc > 0 && loc <= pFlamMap->Header.numelev)
			return pFlamMap->Header.elevs[loc];
	case SLOPE:
		if(loc > 0 && loc <= pFlamMap->Header.numslope)
			return pFlamMap->Header.slopes[loc];
	case ASPECT:
		if(loc > 0 && loc <= pFlamMap->Header.numaspect)
			return pFlamMap->Header.aspects[loc];
	case FUEL:
		if(loc > 0 && loc <= pFlamMap->Header.numfuel)
			return pFlamMap->Header.fuels[loc];
	case COVER:
		if(loc > 0 && loc <= pFlamMap->Header.numcover)
			return pFlamMap->Header.covers[loc];
	case HEIGHT:
		if(loc > 0 && loc <= pFlamMap->Header.numheight)
			return pFlamMap->Header.heights[loc];
	case BASE_HEIGHT:
		if(loc > 0 && loc <= pFlamMap->Header.numbase)
			return pFlamMap->Header.bases[loc];
	case BULK_DENSITY:
		if(loc > 0 && loc <= pFlamMap->Header.numdensity)
			return pFlamMap->Header.densities[loc];
	case WOODY:
		if(loc > 0 && loc <= pFlamMap->Header.numwoody)
			return pFlamMap->Header.woodies[loc];
	case DUFF:
		if(loc > 0 && loc <= pFlamMap->Header.numduff)
			return pFlamMap->Header.duffs[loc];
	}
	return NODATA_VAL;
}

short CFlamMap::GetLayerUnits(int layer)
{
	switch(layer)
	{
	case ELEV:
		return pFlamMap->Header.EUnits;
	case SLOPE:
		return pFlamMap->Header.SUnits;
	case ASPECT:
		return pFlamMap->Header.AUnits;
	case FUEL:
		return 1;//pFlamMap->Header.;
	case COVER:
		return pFlamMap->Header.CUnits;
	case HEIGHT:
		return pFlamMap->Header.HUnits;
	case BASE_HEIGHT:
		return pFlamMap->Header.BUnits;
	case BULK_DENSITY:
		return pFlamMap->Header.PUnits;
	case WOODY:
		return 1;//pFlamMap->Header.numwoody;
	case DUFF:
		return pFlamMap->Header.DUnits;
	}
	return -1;
}

int CFlamMap::GetNumWindRows()
{
	return pFlamMap->nWindRows;
}

int CFlamMap::GetNumWindCols()
{
	return pFlamMap->nWindCols;
}


int CFlamMap::GetWindsResolution()
{
	return pFlamMap->windsResolution;
}



/*.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
* Name: Set_GWN_NoSw  Set_GWN_YesSw
* Desc: Used By FSPRo FireScenario::RunFlamMap() to cause Gridded 
*        Winds WindNinja run/not run. 
*   In:
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- */
void CFlamMap::GWN_Set_NoSw ()
{ 
   strcpy (pFlamMap->icf.cr_GGW,e_GW_No); 
}
void CFlamMap::GWN_Set_YesSw ()
{ 
   strcpy (pFlamMap->icf.cr_GGW,e_GW_Yes); 
}




/*.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
* Name: GWN_Set_SpeedDir
* Desc: Used by FSPRo, See FireScenario::RunFlamMap() 
*       Set inputs where FlamMap::LoadGridWindsWN2() will see them
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- */
/*ALMvoid CFlamMap::GWN_Set_SpeedDir (int WindSpeed, int WindDir)
{   
  pFlamMap->icf.f_WinSpe = (float) WindSpeed; 
  pFlamMap->icf.f_WinDir = (float) WindDir;
}*/
      
/*.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
* Name: GWN_Set_Yes
* Desc: Used by FSPRo, set inputs so that FlamMap will run gridded winds
*        windninja. 
*       See FireScenario::RunFlamMap() 
*       The inputs are put where FlamMap::LoadGridWindsWN2() will see them 
*         inside the FlamMap class. 
* 
.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- */
/*ALM void CFlamMap::GWN_Set_Yes (
             float f_Resolution, 
             float f_WindHeight, 
             int l_procs, 
             bool Diurnal,  
             int i_Month, int i_Day, int i_Year,
             int i_Second, int i_Minute, int i_Hour, int i_TimeZone,      
             float f_Temp, float f_CloudCover, 
             float f_Longitude)
{
  strcpy (pFlamMap->icf.cr_GGW,e_GW_Yes);    /* Yes do Gridded Winds
 
  pFlamMap->icf.f_GriWinRes = f_Resolution ;

  pFlamMap->icf.f_GriWinHei = f_WindHeight;  // Gridded Wind Height 

  pFlamMap->SetMaxThreads(l_procs); 

  d_WDI *w = &pFlamMap->icf.s_WDI;
  strcpy (w->cr_YesNo,e_GW_No);
  if ( Diurnal ) {
     strcpy (w->cr_YesNo,e_GW_Yes);   /* if have diurnal data to use
	    w->i_Day = i_Day; 
     w->i_Mth = i_Month;
     w->i_Year = i_Year; 
     w->i_Sec = i_Second;
     w->i_Min = i_Minute; 
     w->i_Hour = i_Hour; 
     w->i_TimeZone = i_TimeZone; 
     w->f_AirTemp = f_Temp; 
     w->f_CloudCov = f_CloudCover;
     w->f_Longitude = f_Longitude; 
   }
}
*/
// Test Larry - for WNS........................................

float **CFlamMap::Get_windDirGrid()
{
  return  this->pFlamMap->Get_windDirGrid(); 
}

float **CFlamMap::Get_windSpdGrid()
{
  return  this->pFlamMap->Get_windSpdGrid(); 
}

bool CFlamMap::has_GriddedWinds ()
{
  return  this->pFlamMap->has_GriddedWind(); 
}

bool CFlamMap::GridWindRowCol(int *Rows, int *Cols)
{
  return this->pFlamMap->GridWindRowCol(Rows,Cols);
}

/**************************************************************
* Name: Get_ConditionInputs
* Desc: Read Moisture Conditioning Inputs from a file. 
*       Used By FSPRo to have FlamMap read the mositure 
*       conditioning inputs from a FSPRo inputs file and load
*       them into proper place in FlamMap
**************************************************************/
int CFlamMap::FSPRo_ConditionInputs (char cr_PthFN[]) 
{
 int i; 
    i = pFlamMap->FSPRo_ConditionInputs (cr_PthFN);
    return i;
}


/****************************************************************************/
/*bool  CFlamMap::WindNinjaRun (int *ai_Err ) 
{
char *a;

   pFlamMap->LoadGridWindsWN2(ai_Err);
   if ( *ai_Err < 0 ) {   /* error, bad inputs or ninja pooped out *
      a = this->CommandFileError(*ai_Err); 
      printf ("Error: %s \n", a); 
 //      m_runningFlamMap = false;
     return false; }  /* return negative error code, see notes above *
  return true; 	
}*/

/**************************************************************************
* Name: MCR_ifConditioning
* Desc: See if inputs file had Moiture Conditioning Inputs, which means
*        we need to run the condtioning calculations, build map, etc
*  Ret: true....Yes we have Moist Cond inputs
**************************************************************************/
/*bool CFlamMap::MCR_ifConditioning()
{
bool b; 
  b = this->pFlamMap->ConstFuelMoistures(GETVAL); // chk Constant Fule Mosit Switch //
  if ( b == true )    // it's set do do Constant Fuel Moist //   
    return false;     //So we're not doing Conditioning //
 return true; 
}*/

/***************************************************************************/
/*ALM char * CFlamMap::WindNinjaError (int i_Err)
{
char *a;
    a = this->CommandFileError(i_Err); 
    return a; 
}
*/



// **********************************************************************************
// 8-27-09  Add the following functions 


void CFlamMap::GetWindCorners(double *X, double *Y)
{
   *X = pFlamMap->windsXllCorner;
   *Y = pFlamMap->windsYllCorner;
}

void CFlamMap::AllocWindGrids(int wndRows, int wndCols, double wndRes, double wndXLL, double wndYLL)
{
  pFlamMap->AllocWindGrids(wndRows, wndCols, wndRes, wndXLL, wndYLL);
}

// **********************************************************************************/
/*ALMchar * CFlamMap::Get_CondDLLErrMes()
{
  return this->pFlamMap->Get_CondDLLErrMes();
}
*/

