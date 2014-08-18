// LegendData.cpp: implementation of the CLegendData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "rand3.h"


#include "FlamMap_DLL.h"
#include "flammap3.h"

#include <atlcoll.h>


/*static char *unitStrings[] = 
{
	"Meters",			//0
	"Feet",
	"Degrees",
	"Percent",
	"Radians",
	"Class",			//5
	"kg/m^3",
	"10lb/ft^3",
	"m/min",
	"ft/min",
	"kW/m",				//10
	"BTU/ft/sec",
	"kJ/m^2",
	"BTU/ft^2",
	"W/m^2",
	"mph",
	"k/h",				//16
	"Mg/Ha",
	"Tons/Acre",
	"Chains/Hr",		//19
	"Fraction",
	"kg/m^3*100",
	"lb/ft^3",
	"lb/ft^3*1000",
	"Frequency",
	"Minutes"
};

static short elevUnits[] = {0, 1, -1};
static short slopeUnits[] = {2, 3, -1};
static short aspectUnits[] = {2, -1};
static short fuelUnits[] = {5, -1};
static short coverUnits[] = {5, 3, -1};
static short heightUnits[] = {0, 1, -1};
static short baseUnits[] = {0, 1, -1};
static short bulkUnits[] = {6, 21, 22, 23, -1};
static short duffUnits[] = {17, 18, -1};
static short woodyUnits[] = {5, -1};
static short genericUnits[] = {5, -1};
static short flamelenUnits[] = {0, 1, -1};
static short spreadrateUnits[] = {8, 9, 19, -1};
static short intensityUnits[] = {10, 11, -1};
static short heatareaUnits[] = {12, 13, -1};
static short crownstateUnits[] = {5, -1};
static short solrradiationUnits[] = {14, -1};
static short fuelmoisture1Units[] = {20, -1};
static short fuelmoisture10Units[] = {20, -1};
static short midflameUnits[] = {15, 16, -1};
static short horizrateUnits[] = {8, 9, 19, -1};
static short maxspreaddirUnits[] = {4, 2, -1};
static short ellipsedimaUnits[] = {8, 9, 19, -1};
static short ellipsedimbUnits[] = {8, 9, 19, -1};
static short ellipsedimcUnits[] = {8, 9, 19, -1};
static short rosUnits[] = {8, 9, 19, -1};
static short influenceUnits[] = {24, -1};
static short arrivalUnits[] = {25, -1};
static short fliMapUnits[] = {10, 11, -1};
static short burnProbUnits[] = {20, -1};
static short treatmentUnits[] = {5, -1};
static short treatOpportunityUnits[] = {5, -1};*/

/*static short *themeUnits[32] = 
{
	elevUnits,
	slopeUnits,
	aspectUnits,
	fuelUnits,
	coverUnits,
	heightUnits,
	baseUnits,
	bulkUnits,
	duffUnits,
	woodyUnits,
	genericUnits,
	flamelenUnits,
	spreadrateUnits,
	intensityUnits,
	heatareaUnits,
	crownstateUnits,
	solrradiationUnits,
	fuelmoisture1Units,
	fuelmoisture10Units,
	midflameUnits,
	horizrateUnits,
	maxspreaddirUnits,
	ellipsedimaUnits,
	ellipsedimbUnits,
	ellipsedimcUnits,
	rosUnits,
	influenceUnits,
	arrivalUnits,
	fliMapUnits,
	burnProbUnits,
	treatmentUnits,
	treatOpportunityUnits,
};*/

CColorRamp::CColorRamp()
{
	//default values indicate a random ramp
	nColors = 0;
	colors = NULL;
}

CColorRamp::~CColorRamp()
{
	if(colors)
		delete[] colors;
}

void CColorRamp::Create(int _nColors, COLORREF *_colors)
{
	if(colors)
	{
		delete[] colors;
		colors = NULL;
	}
	nColors = _nColors;
	if(nColors > 0)
	{
		colors = new COLORREF[nColors];
		for(int i = 0; i < nColors; i++)
			colors[i] = _colors[i];
	}
}

CLegendEntry::CLegendEntry(float _val, COLORREF _color, int _frequency)
{
	val = _val;
	color = _color;
	frequency = _frequency;
}

CLegendEntry::~CLegendEntry()
{
}

void CLegendEntry::SetVal(float _val)
{
	val = _val;
}

void CLegendEntry::SetFrequency(int _frequency)
{
	frequency = _frequency;
}

void CLegendEntry::SetColor(COLORREF _color)
{
	color = _color;
}

float CLegendEntry::GetVal()
{
	return val;
}

int CLegendEntry::GetFrequency()
{
	return frequency;
}

COLORREF CLegendEntry::GetColor()
{
	return color;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLegendData::CLegendData(CFlamMap *_pFlamMap, int _themeType)
{
	entries = NULL;
	strcpy(title, "");
	hi = lo = 0;
	units = 0;
	categorical = FALSE;
	numEntries = 0;
	pFlamMap = _pFlamMap;
	themeType = _themeType;
	switch(themeType)
	{
	case 0:
		strcpy(title, "Elevation");
		break;
	case 1:
		strcpy(title, "Slope");
		break;
	case 2:
		strcpy(title, "Aspect");
		break;
	case 3:
		strcpy(title, "Fuel Model");
		break;
	case 4:
		strcpy(title, "Canopy Cover");
		break;
	case 5:
		strcpy(title, "Stand Height");
		break;
	case 6:
		strcpy(title, "Canopy Base Height");
		break;
	case 7:
		strcpy(title, "Canopy Bulk Density");
		break;
	case 8:
		strcpy(title, "Duff");
		break;
	case 9:
		strcpy(title, "Woody Fuels");
		break;
	case 10:
		strcpy(title, "Generic Grid");
		break;
	case 11:
		strcpy(title, "Flame Length");
		break;
	case 12:
		strcpy(title, "Spread Rate");
		break;
	case 13:
		strcpy(title, "Fireline Intensity");
		break;
	case 14:
		strcpy(title, "Heat per Unit Area");
		break;
	case 15:
		strcpy(title, "Crown State");
		break;
	case 16:
		strcpy(title, "Solar Radiation");
		break;
	case 17:
		strcpy(title, "1 Hr Fuel Moisture");
		break;
	case 18:
		strcpy(title, "10 Hr Fuel Moisture");
		break;
	case 19:
		strcpy(title, "Midflame Windspeed");
		break;
	case 20:
		strcpy(title, "Horizontal Movement Rate");
		break;
	case 21:
		strcpy(title, "Max Spread Direction");
		break;
	case 22:
		strcpy(title, "Ellipse Dimension A");
		break;
	case 23:
		strcpy(title, "Ellipse Dimension B");
		break;
	case 24:
		strcpy(title, "Ellipse Dimension C");
		break;
	case 25:
		strcpy(title, "MaxSpot (Combined)");
		break;
	case 26:
		strcpy(title, "100 Hr Fuel Moisture");
		break;
	case 27:
		strcpy(title, "1000 Hr Fuel Moisture");
		break;
	case 28:
		strcpy(title, "MaxSpot Direction");
		break;
	case 29:
		strcpy(title, "MaxSpot Distance");
		break;
	case 30:
		strcpy(title, "WindNinja Wind Diection");
		break;
	case 31:
		strcpy(title, "WindNinja Wind Speed");
		break;
	case 32:
		strcpy(title, "MTT Rate of Spread");
		break;
	case 33:
		strcpy(title, "MTT Node Influence");
		break;
	case 34:
		strcpy(title, "MTT Arrival Time");
		break;
	case 35:
		strcpy(title, "MTT FliMap");
		break;
	case 36:
		strcpy(title, "Burn Probability");
		break;
	case 37:
		strcpy(title, "Treatment");
		break;
	case 38:
		strcpy(title, "Treat Opportunity");
		break;
	default:
		strcpy(title, "Unknown");
		break;
	}
	this->decimalPlaces = 2;
	
}

CLegendData::~CLegendData()
{
	DeleteEntries();
}

void CLegendData::SetUnits(short _units)
{
	units = _units;
}

short CLegendData::GetUnits()
{
	return units;
}

void CLegendData::SetHi(double _hi)
{
	hi = _hi;//max(_hi, 1);
}

double CLegendData::GetHi()
{
	return hi;
}

void CLegendData::SetLo(double _lo)
{
	lo = _lo;
}

double CLegendData::GetLo()
{
	return lo;
}

void CLegendData::DeleteEntries()
{
	for(int i = 0; i < numEntries; i++)
		delete entries[i];
	delete[] entries;
	entries = NULL;
	numEntries = 0;
//	while(entries.GetCount() > 0)
//		delete entries.RemoveHead();
}

typedef CAtlList<CLegendEntry *> CLegendEntryList;

void CLegendData::AddEntry(CLegendEntry *entry)
{
	CLegendEntryList entryList;
	CLegendEntry *tEntry;
	POSITION pos, prevPos;
	for(int i = 0; i < numEntries; i++)
	{
		if(entries[i]->GetVal() == NODATA_VAL)
		{
			entryList.AddHead(entries[i]);
			continue;
		}
		pos = entryList.GetHeadPosition(); 
		prevPos = NULL;
		tEntry = NULL;
		while(pos)
		{
			tEntry = entryList.GetNext(pos);
			if(entries[i]->GetVal() < tEntry->GetVal() && tEntry->GetVal() != NODATA_VAL)
				break;
			prevPos = pos;
			tEntry = NULL;
		}
		if(!tEntry)
			entryList.AddTail(entries[i]);
		else
			entryList.InsertBefore(prevPos, tEntry);
	}
	CLegendEntry *newEntry = new CLegendEntry(entry->GetVal(), entry->GetColor());
	pos = entryList.GetHeadPosition(); 
	prevPos = NULL;
	tEntry = NULL;
	while(pos)
	{
		tEntry = entryList.GetNext(pos);
		if(newEntry->GetVal() < tEntry->GetVal() && tEntry->GetVal() != NODATA_VAL)
				break;
			prevPos = pos;
			tEntry = NULL;
		}
		if(!tEntry)
			entryList.AddTail(newEntry);
		else
			entryList.InsertBefore(prevPos, newEntry);
	if(entries)
		delete[] entries;	
	numEntries++;
	entries = new CLegendEntry *[numEntries];
	pos = entryList.GetHeadPosition();
	int e = 0;
	while(pos)
	{
		tEntry = entryList.GetNext(pos);
		entries[e++] = tEntry;
	}
}

COLORREF CLegendData::MapColor(double val)
{
	COLORREF ret(RGB(0, 0, 0));
	if(numEntries > 0)
	{
		if(val < lo || val > hi)
			ret = entries[0]->GetColor();
		else
		{
			for(int i = 1; i < numEntries; i++)
			{
				ret = entries[i]->GetColor();
				if(entries[i]->GetVal() >= val)
					break;
			}
		}
	}
	return ret;
}

void CLegendData::RampColors(COLORREF top, COLORREF bottom, int nColors, COLORREF *colors)
{
	short tR, tB, tG, bR, bB, bG, pR, pB, pG;
	float sR, sB, sG;
	if(nColors <= 1)
	{
		colors[0] = top;
		colors[1] = bottom;
		return;
	}
	tR = GetRValue(top);
	tB = GetBValue(top);
	tG = GetGValue(top);
	bR = GetRValue(bottom);
	bB = GetBValue(bottom);
	bG = GetGValue(bottom);
	sR = ((float)(bR - tR)) / ((float)(nColors - 1));
	sB = ((float)(bB - tB)) / ((float)(nColors - 1));
	sG = ((float)(bG - tG)) / ((float)(nColors - 1));
    //TRACE3("RampColors called tR=%i tB=%i tG=%i ", tR,tB,tG);
	//TRACE3("bR=%i bB=%i bG=%i ",bR,bB,bG);
	//TRACE3("sR=%f sB=%f sG=%f\n",sR,sB,sG);
	for(int c = 0; c < nColors; c++)
	{
		pR = tR + c * sR;
		pG = tG + c * sG;
		pB = tB + c * sB;
		colors[c] = RGB(pR, pG, pB);
		//TRACE1("colors[%d]: ", c);
		//TRACE3("pR=%i pG=%i pB=%i\n", pR, pG, pB);
		//TRACE3("r=%i g=%i b=%i\n", GetRValue(colors[c]), GetGValue(colors[c]), GetBValue(colors[c]));
	}
}

BOOL CLegendData::SetCategorical(BOOL isCat)
{
	BOOL old = categorical;
	categorical = isCat;
	return old;
}

BOOL CLegendData::GetCategorical()
{
	return categorical;
}

char *CLegendData::GetUnitsString()
{
	if(themeType <= WOODY)
		return unitStrings[themeUnits[themeType][units]];
	return "Class";
	//return unitStrings[themeUnits[themeType][units]];
}


void CLegendData::ApplyColorRamp(CColorRamp *ramp)
{//color ramps are applied to all but the NO DATA class which should (better!) be first in the list of entries
Random  random; 


	if(ramp->nColors <= 0)//random colors
	{
		SYSTEMTIME st;
		::GetSystemTime(&st);
		long idum = st.wMilliseconds;
		if(idum > 0)
			idum = -idum;
		int r, g, b;
		for(int i = 1; i < numEntries; i++)
		{
			r = (int)(random.rand2(&idum) * 256.0);
			g = (int)(random.rand2(&idum) * 256.0);
			b = (int)(random.rand2(&idum) * 256.0);
			entries[i]->SetColor(RGB(r, g, b));
		}
	}
	else if(ramp->nColors == 1)
	{
		for(int i = 1; i < numEntries; i++)
			entries[i]->SetColor(ramp->colors[0]);
	}
	else//valid ramp
	{
		COLORREF colors[MAX_CATS];
		int steps = ramp->nColors - 1, nColors;
		double a = ramp->nColors, b = numEntries - 1;
		double step = b / steps; 
		nColors = ceil(step);
		for(int i = 0; i < steps; i++)
		{
			int base = (int) ((double)i * step) + 1;
			RampColors(ramp->colors[i], ramp->colors[i + 1], nColors, colors);
			for(int e = 0; e < nColors; e++)
				entries[base + e]->SetColor(colors[e]);
		}
	}
}

// Applies a predefined legend to an existing legend already constructed
void CLegendData::ApplyPredefinedLegend(CPredefinedLegend * preLegend)
{
	DeleteEntries();
	//hi = lo = NODATA_VAL;
	//decimalPlaces = 0;
	//categorical = FALSE;
		//rect = workArea->GetThemeRect(theme);
	//long nVals = (theme->numEast) * (theme->numNorth);
/*	float val;//s = new float[nVals];
	int t = 1, nUniqueVals = -1;
	long uniqueVals[MAX_CATS];
	uniqueVals[0] = NODATA_VAL;
	for(long r = 0; r < theme->numNorth; r++)
	{
		for(long c = 0; c < theme->numEast; c++)
		{
			val = theme->GetVal(c, r);
			if(val != NODATA_VAL)
			{
				if(lo == NODATA_VAL)
					lo = val;
				else
					lo = min(lo, val);
				if(hi == NODATA_VAL)
					hi = val;
				else
					hi = max(hi, val);
			}
			bool found = (t >= MAX_CATS) ? true : false;
			for(int u = 0; u < t && !found; u++)
			{
				if(uniqueVals[u] == val)
					found = true;
			}
			if(!found)
				uniqueVals[t++] = val;
		}
	}
	if(t < MAX_CATS)
		nUniqueVals = t;*/
	categorical = preLegend->categorical;
	//if(categorical && nUniqueVals <= 0)//too many categories, make continuous
	//	categorical = false;
	decimalPlaces = preLegend->decimalPlaces;
	units = preLegend->units;
	if(!categorical)
	{//straight copy of classes
		for(int c = 0; c < preLegend->numEntries; c++)
		{
			AddEntry(preLegend->entries[c]);
		}
		//for(c = 1; c < numEntries; c++)
		//	entries[c]->SetVal(theme->GetDisplayVal(theme->GetDataUnitsVal(entries[c]->GetVal(), preLegend),this));
	}
	else
	{
		//add the predefined legend's classes
		for(int c = 0; c < preLegend->numEntries; c++)
		{
			AddEntry(preLegend->entries[c]);
		}
		//do unit coneversion
		//for(c = 1; c < numEntries; c++)
		//	entries[c]->SetVal(theme->GetDisplayVal(theme->GetDataUnitsVal(entries[c]->GetVal(), preLegend),this));
	}
}

void CLegendData::ChangeUnits(int destUnits)
{
	float vals[100];
	vals[0] = NODATA_VAL;
	for(int c = 1; c < numEntries; c++)
	{
		vals[c] = pFlamMap->GetDataUnitsVal(entries[c]->GetVal(), this);
		//entries[c]->SetVal(theme->GetDisplayVal(val, this);
		//entries[c]->SetVal(theme->ConvertUnits(entries[c]->GetVal(), GetUnits(), destUnits));
	}
	SetUnits(destUnits);
	for(int c = 1; c < numEntries; c++)
	{
		entries[c]->SetVal(pFlamMap->GetDisplayVal(vals[c], this));
		//entries[c]->SetVal(theme->GetDisplayVal(val, this);
		//entries[c]->SetVal(theme->ConvertUnits(entries[c]->GetVal(), GetUnits(), destUnits));
	}

}

int CLegendData::GetThemeType()
{
	return themeType;
}

int CLegendData::SetThemeType(int _tt)
{
	themeType = _tt;
	return themeType;
}

void CLegendData::LoadDefault()
{
	// 2010/03/01  color ramps changed for WFDSS compatibility

	
	

	switch(themeType)
	{
	case E_DATA:
		{
			double hi = pFlamMap->GetLayerHi(ELEV), lo = max(0, pFlamMap->GetLayerLo(ELEV));
			CColorRamp ramp;
			COLORREF colors[4];
			colors[0] = RGB(0, 0, 255);
			colors[1] = RGB(255, 255, 0);//65535;
			colors[2] = RGB(0, 128, 128);//39168;
			colors[3] = RGB(248, 248, 248);//16711680;
			//colors[4] = RGB(128, 0, 128);//8388736;
			ramp.Create(4, colors);
			SetLo(lo);
			SetHi(hi);
			SetCategorical(FALSE);
			decimalPlaces = 0;
			SetUnits(pFlamMap->GetLayerUnits(ELEV));
			int nCats = 20;
			CLegendEntry entry;
			entry.SetColor(RGB(255, 255, 255));
			entry.SetVal(NODATA_VAL);
			AddEntry(&entry);
			float step = (hi - lo) / (float)nCats;
			for(int c = 0; c < nCats; c++)
			{//create the entries
				entry.SetColor(RGB(0, 0, 0));
				entry.SetVal(pFlamMap->GetDisplayVal(lo + step * (c + 1), this));
				AddEntry(&entry);
			}
			ApplyColorRamp(&ramp);
		}
		break;
	case S_DATA:
		{
			double hi = pFlamMap->GetLayerHi(SLOPE), lo = max(0, pFlamMap->GetLayerLo(SLOPE));
			COLORREF colors[51];
			RampColors(RGB(0,0,255),RGB(255,255,0),11,colors);
            COLORREF *pc;
            pc = &colors[10];
            RampColors(RGB(255,255,0),RGB(255,0,0),31,pc);
			pc = &colors[40];
			RampColors(RGB(255,0,0),RGB(160,0,160),11,pc);

			SetLo(lo);
			SetHi(hi);
			SetCategorical(FALSE);
			decimalPlaces = 0;
			SetUnits(pFlamMap->GetLayerUnits(SLOPE));
			CLegendEntry entry;
			entry.SetColor(RGB(255, 255, 255));
			entry.SetVal(NODATA_VAL);
			AddEntry(&entry);

			for (int i= 0; i < 51; i++)				
			{

                entry.SetColor(colors[i]);
				entry.SetVal(i);
				AddEntry(&entry);
			}
			

		}
		break;
	case A_DATA:
		{
			double hi = pFlamMap->GetLayerHi(ASPECT), lo = max(0, pFlamMap->GetLayerLo(ASPECT));
			SetLo(lo);
			SetHi(hi);
			SetCategorical(FALSE);
			decimalPlaces = 0;
			SetUnits(2);
			CLegendEntry entry;
			entry.SetColor(RGB(255, 255, 255));
			entry.SetVal(NODATA_VAL);
			AddEntry(&entry);
			entry.SetColor(RGB(128, 64, 0));
			entry.SetVal(0.0);
			AddEntry(&entry);
			entry.SetColor(RGB(159, 111, 32));
			entry.SetVal(67.0);
			AddEntry(&entry);
			entry.SetColor(RGB(191, 159, 64));
			entry.SetVal(112.0);
			AddEntry(&entry);
			entry.SetColor(RGB(223, 207, 96));
			entry.SetVal(157.0);
			AddEntry(&entry);
			entry.SetColor(RGB(255, 255, 128));
			entry.SetVal(202.0);
			AddEntry(&entry);
			entry.SetColor(RGB(223, 207, 96));
			entry.SetVal(247.0);
			AddEntry(&entry);
			entry.SetColor(RGB(191, 159, 64));
			entry.SetVal(292.0);
			AddEntry(&entry);
			entry.SetColor(RGB(159, 111, 32));
			entry.SetVal(337.0);
			AddEntry(&entry);
			entry.SetColor(RGB(128, 64, 0));
			entry.SetVal(360.0);
			AddEntry(&entry);
		}
		break;
	case F_DATA:
		{
			double hi = pFlamMap->GetLayerHi(FUEL), lo = max(0, pFlamMap->GetLayerLo(FUEL));
			SetLo(lo);
			SetHi(hi);
			SetCategorical(TRUE);
			decimalPlaces = 0;
			SetUnits(0);
			CLegendEntry entry;
			entry.SetColor(RGB(255, 255, 255));
			entry.SetVal(NODATA_VAL);
			AddEntry(&entry);
			for(int f = 1; f <= pFlamMap->GetLayerHeaderValCount(FUEL); f++)
			{
				entry.SetVal(pFlamMap->GetLayerHeaderVal(FUEL, f));//Header.fuels[f]);
				switch(pFlamMap->GetLayerHeaderVal(FUEL, f))//pFlamMap->Header.fuels[f])

				{
					case 1:
						entry.SetColor(RGB(255, 255, 0));
						break;
					case 2:
						entry.SetColor(RGB(0, 224, 224));
						break;
					case 3:
						entry.SetColor(RGB(255, 184, 0));
						break;
					case 4:
						entry.SetColor(RGB(128, 16, 16));
						break;
					case 5:
						entry.SetColor(RGB(152, 88, 24));
						break;
					case 6:
						entry.SetColor(RGB(128, 48, 48));
						break;
					case 7:
						entry.SetColor(RGB(160, 100, 20));
						break;
					case 8:
						entry.SetColor(RGB(104, 152, 204));
						break;
					case 9:
						entry.SetColor(RGB(0, 132, 132));
						break;
					case 10:
						entry.SetColor(RGB(128, 92, 96));
						break;
					case 11:
						entry.SetColor(RGB(255, 132, 255));
						break;
					case 12:
						entry.SetColor(RGB(192, 88, 192));
						break;
					case 13:
						entry.SetColor(RGB(128, 44, 128));
						break;
					case 91:
						entry.SetColor(RGB(204, 204, 204));
						break;
					case 92:
						entry.SetColor(RGB(152, 152, 152));
						break;
					case 93:
						entry.SetColor(RGB(100, 100, 100));
						break;
					case 98:
						entry.SetColor(RGB(0, 0, 255));
						break;
					case 99:
						entry.SetColor(RGB(0, 0, 0));
						break;
					case 101:
						entry.SetColor(RGB(255, 255, 152));
						break;
					case 102:
						entry.SetColor(RGB(255, 255, 64));
						break;
					case 103:
						entry.SetColor(RGB(255, 255, 96));
						break;
					case 104:
						entry.SetColor(RGB(255, 229, 0));
						break;
					case 105:
						entry.SetColor(RGB(255, 255, 32));
						break;
					case 106:
						entry.SetColor(RGB(255, 204, 0));
						break;
					case 107:
						entry.SetColor(RGB(252, 164, 0));
						break;
					case 108:
						entry.SetColor(RGB(250, 140, 0));
						break;
					case 109:
						entry.SetColor(RGB(249, 102, 0));
						break;
					case 121:
						entry.SetColor(RGB(153, 153, 78));
						break;
					case 122:
						entry.SetColor(RGB(119, 119, 26));
						break;
					case 123:
						entry.SetColor(RGB(136, 136, 52));
						break;
					case 124:
						entry.SetColor(RGB(102, 102, 0));
						break;
					case 141:
						entry.SetColor(RGB(195, 132, 0));
						break;
					case 142:
						entry.SetColor(RGB(173, 111, 13));
						break;
					case 143:
						entry.SetColor(RGB(184, 121, 0));
						break;
					case 144:
						entry.SetColor(RGB(141, 80, 34));
						break;
					case 145:
						entry.SetColor(RGB(130, 70, 41));
						break;
					case 146:
						entry.SetColor(RGB(121, 60, 48));
						break;
					case 147:
						entry.SetColor(RGB(128, 64, 64));
						break;
					case 148:
						entry.SetColor(RGB(128, 32, 32));
						break;
					case 149:
						entry.SetColor(RGB(102, 0, 0));
						break;
					case 161:
						entry.SetColor(RGB(217, 254, 160));
						break;
					case 162:
						entry.SetColor(RGB(173, 223, 128));
						break;
					case 163:
						entry.SetColor(RGB(43, 132, 32));
						break;
					case 164:
						entry.SetColor(RGB(86, 162, 64));
						break;
					case 165:
						entry.SetColor(RGB(0, 102, 0));
						break;
					case 181:
						entry.SetColor(RGB(170, 213, 255));
						break;
					case 182:
						entry.SetColor(RGB(0, 255, 255));
						break;
					case 183:
						entry.SetColor(RGB(136, 183, 229));
						break;
					case 184:
						entry.SetColor(RGB(68, 123, 178));
						break;
					case 185:
						entry.SetColor(RGB(34, 93, 153));
						break;
					case 186:
						entry.SetColor(RGB(0, 193, 193));
						break;
					case 187:
						entry.SetColor(RGB(0, 64, 128));
						break;
					case 188:
						entry.SetColor(RGB(0, 163, 163));
						break;
					case 189:
						entry.SetColor(RGB(0, 101, 101));
						break;
					case 201:
						entry.SetColor(RGB(223, 109, 223));
						break;
					case 202:
						entry.SetColor(RGB(159, 65, 159));
						break;
					case 203:
						entry.SetColor(RGB(95, 20, 95));
						break;
					case 204:
						entry.SetColor(RGB(63, 0, 63));
				}
				AddEntry(&entry);
			}
		}
		break;
	case C_DATA:
		{
			double hi = pFlamMap->GetLayerHi(COVER), lo = max(0, pFlamMap->GetLayerLo(COVER));
			SetLo(lo);
			SetHi(hi);
			SetCategorical(FALSE);
			decimalPlaces = 0;
			SetUnits(pFlamMap->GetLayerUnits(COVER));//>Header.CUnits);
			CLegendEntry entry;
			entry.SetColor(RGB(255, 255, 255));
			entry.SetVal(NODATA_VAL);
			AddEntry(&entry);
			entry.SetColor(RGB(244, 244, 244));
			entry.SetVal(0);
			AddEntry(&entry);
			entry.SetColor(RGB(200, 240, 224));
			entry.SetVal(5);
			AddEntry(&entry);
			entry.SetColor(RGB(176, 232, 208));
			entry.SetVal(15);
			AddEntry(&entry);
			entry.SetColor(RGB(152, 224, 192));
			entry.SetVal(25);
			AddEntry(&entry);
			entry.SetColor(RGB(128, 216, 176));
			entry.SetVal(35);
			AddEntry(&entry);
			entry.SetColor(RGB(96, 192, 144));
			entry.SetVal(45);
			AddEntry(&entry);
			entry.SetColor(RGB(64, 168, 112));
			entry.SetVal(55);
			AddEntry(&entry);
			entry.SetColor(RGB(32, 144, 80));
			entry.SetVal(65);
			AddEntry(&entry);
			entry.SetColor(RGB(24, 128, 64));
			entry.SetVal(75);
			AddEntry(&entry);
			entry.SetColor(RGB(16, 112, 48));
			entry.SetVal(85);
			AddEntry(&entry);
			entry.SetColor(RGB(8, 96, 32));
			entry.SetVal(95);
			AddEntry(&entry);
		}
		break;
	case H_DATA:
		{
			double hi = pFlamMap->GetLayerHi(HEIGHT), lo = max(0, pFlamMap->GetLayerLo(HEIGHT));
			SetLo(lo);
			SetHi(hi);
			SetCategorical(FALSE);
			decimalPlaces = 0;
			SetUnits((pFlamMap->GetLayerUnits(HEIGHT) == 1 || pFlamMap->GetLayerUnits(HEIGHT) == 3) ? 0 : 1);
			CLegendEntry entry;
			entry.SetColor(RGB(255, 255, 255));
			entry.SetVal(NODATA_VAL);
			AddEntry(&entry);
			entry.SetColor(RGB(255, 255, 128));
			entry.SetVal(0);
			AddEntry(&entry);
			entry.SetColor(RGB(255, 192, 96));
			entry.SetVal(25);
			AddEntry(&entry);
			entry.SetColor(RGB(255, 153, 76));
			entry.SetVal(55);
			AddEntry(&entry);
			entry.SetColor(RGB(255, 128, 64));
			entry.SetVal(75);
			AddEntry(&entry);
			entry.SetColor(RGB(198, 99, 35));
			entry.SetVal(165);
			AddEntry(&entry);
			entry.SetColor(RGB(192, 96, 32));
			entry.SetVal(175);
			AddEntry(&entry);
			entry.SetColor(RGB(64, 0, 0));
			entry.SetVal(375);
			AddEntry(&entry);
		
		}
		break;
	case B_DATA:
		{
			double hi = pFlamMap->GetLayerHi(BASE_HEIGHT), lo = max(0, pFlamMap->GetLayerLo(BASE_HEIGHT));
			SetUnits((pFlamMap->GetLayerUnits(BASE_HEIGHT) == 1 || pFlamMap->GetLayerUnits(BASE_HEIGHT) == 3) ? 1 : 0);
			COLORREF colors[101];
			RampColors(RGB(32,96,64),RGB(96,144,128),10,colors);
            COLORREF *pc;
            pc = &colors[9];
            RampColors(RGB(96,144,128),RGB(192,232,224),51,pc);
			pc = &colors[60];
			RampColors(RGB(192,232,224),RGB(224,248,240),41,pc);

			SetLo(lo);
			SetHi(hi);
			SetCategorical(FALSE);
			decimalPlaces = 2;
			SetUnits(pFlamMap->GetLayerUnits(BASE_HEIGHT));
			CLegendEntry entry;
			
			entry.SetColor(RGB(255, 255, 255));
			entry.SetVal(NODATA_VAL);
			AddEntry(&entry);

			entry.SetColor(RGB(244, 244, 244));
			entry.SetVal(0);
			AddEntry(&entry);


			for (int i= 0; i < 100; i++)				
			{

                entry.SetColor(colors[i]);
				entry.SetVal((float) (i) * 0.1 + 0.1);
				AddEntry(&entry);
			}
			
		}
		break;
	case P_DATA:
		{
			double hi = pFlamMap->GetLayerHi(BULK_DENSITY), lo = max(0, pFlamMap->GetLayerLo(BULK_DENSITY));
			
		
			SetLo(lo);
			SetHi(hi);
			SetCategorical(FALSE);
			decimalPlaces = 2;
			SetUnits((pFlamMap->GetLayerUnits(BULK_DENSITY) == 1 || pFlamMap->GetLayerUnits(BULK_DENSITY) == 3) ? 0 : 1);

			int nCats = pFlamMap->GetLayerHeaderValCount(BULK_DENSITY);//>Header.numduff;
			if(nCats <= 0)
				nCats = 20;

			

			CLegendEntry entry;
			entry.SetColor(RGB(255, 255, 255));
			entry.SetVal(NODATA_VAL);
			AddEntry(&entry);

			entry.SetColor(RGB(244, 244, 244));
			entry.SetVal(0);
			AddEntry(&entry);

		
			entry.SetColor(RGB(216, 232, 240));
			entry.SetVal(1);
			AddEntry(&entry);

			entry.SetColor(RGB(209, 227, 237));
			entry.SetVal(2);
			AddEntry(&entry);
		
			entry.SetColor(RGB(203, 223, 234));
			entry.SetVal(3);
			AddEntry(&entry);

			entry.SetColor(RGB(197, 218, 232));
			entry.SetVal(4);
			AddEntry(&entry);

			entry.SetColor(RGB(191, 214, 229));
			entry.SetVal(5);
			AddEntry(&entry);

			entry.SetColor(RGB(184, 209, 226));
			entry.SetVal(6);
			AddEntry(&entry);

			entry.SetColor(RGB(178, 205, 224));
			entry.SetVal(7);
			AddEntry(&entry);
			
			entry.SetColor(RGB(172, 200, 221));
			entry.SetVal(8);
			AddEntry(&entry);

			entry.SetColor(RGB(166, 196, 218));
			entry.SetVal(9);
			AddEntry(&entry);

			entry.SetColor(RGB(160, 192, 216));
			entry.SetVal(10);
			AddEntry(&entry);

			entry.SetColor(RGB(153, 189, 213));
			entry.SetVal(11);
			AddEntry(&entry);

			entry.SetColor(RGB(147, 187, 210));
			entry.SetVal(12);
			AddEntry(&entry);

			entry.SetColor(RGB(140, 184, 207));
			entry.SetVal(13);
			AddEntry(&entry);

			entry.SetColor(RGB(134, 182, 204));
			entry.SetVal(14);
			AddEntry(&entry);

			entry.SetColor(RGB(128, 180, 202));
			entry.SetVal(15);
			AddEntry(&entry);

			entry.SetColor(RGB(121, 177, 199));
			entry.SetVal(16);
			AddEntry(&entry);

			entry.SetColor(RGB(115, 175, 196));
			entry.SetVal(17);
			AddEntry(&entry);

			entry.SetColor(RGB(108, 172, 193));
			entry.SetVal(18);
			AddEntry(&entry);

			entry.SetColor(RGB(102, 170, 190));
			entry.SetVal(19);
			AddEntry(&entry);

			entry.SetColor(RGB(96, 168, 188));
			entry.SetVal(20);
			AddEntry(&entry);

			entry.SetColor(RGB(89, 165, 185));
			entry.SetVal(21);
			AddEntry(&entry);

			entry.SetColor(RGB(83, 163, 182));
			entry.SetVal(22);
			AddEntry(&entry);

			entry.SetColor(RGB(76, 160, 179));
			entry.SetVal(23);
			AddEntry(&entry);

			entry.SetColor(RGB(70, 158, 176));
			entry.SetVal(24);
			AddEntry(&entry);
			
			entry.SetColor(RGB(64, 156, 174));
			entry.SetVal(25);
			AddEntry(&entry);
			
			entry.SetColor(RGB(57, 153, 171));
			entry.SetVal(26);
			AddEntry(&entry);

			entry.SetColor(RGB(51, 151, 168));
			entry.SetVal(27);
			AddEntry(&entry);

			entry.SetColor(RGB(44, 148, 165));
			entry.SetVal(28);
			AddEntry(&entry);

			entry.SetColor(RGB(38, 146, 162));
			entry.SetVal(29);
			AddEntry(&entry);

			entry.SetColor(RGB(32, 144, 160));
			entry.SetVal(30);
			AddEntry(&entry);

			entry.SetColor(RGB(28, 136, 150));
			entry.SetVal(31);
			AddEntry(&entry);

			entry.SetColor(RGB(25, 128, 140));
			entry.SetVal(32);
			AddEntry(&entry);

			entry.SetColor(RGB(22, 120, 131));
			entry.SetVal(33);
			AddEntry(&entry);

			entry.SetColor(RGB(19, 112, 121));
			entry.SetVal(34);
			AddEntry(&entry);

			entry.SetColor(RGB(16, 104, 112));
			entry.SetVal(35);
			AddEntry(&entry);

			entry.SetColor(RGB(12, 96, 102));
			entry.SetVal(36);
			AddEntry(&entry);

			entry.SetColor(RGB(9, 88, 92));
			entry.SetVal(37);
			AddEntry(&entry);

			entry.SetColor(RGB(6, 80, 83));
			entry.SetVal(38);
			AddEntry(&entry);

			entry.SetColor(RGB(3, 72, 73));
			entry.SetVal(39);
			AddEntry(&entry);

			entry.SetColor(RGB(0, 64, 64));
			entry.SetVal(40);
			AddEntry(&entry);




			

			


		
		}
		break;
	case D_DATA:
		{
			double hi = pFlamMap->GetLayerHi(DUFF), lo = max(0, pFlamMap->GetLayerLo(DUFF));
		
			SetLo(lo);
			SetHi(hi);
			SetCategorical(FALSE);
			decimalPlaces = 2;
			SetUnits(pFlamMap->GetLayerUnits(DUFF) - 1);
			

			CLegendEntry entry;
			entry.SetColor(RGB(255, 255, 255));
			entry.SetVal(NODATA_VAL);
			AddEntry(&entry);

			entry.SetColor(RGB(208, 208, 208));
			entry.SetVal(0);
			AddEntry(&entry);

			entry.SetColor(RGB(255, 0, 0));
			entry.SetVal(.01);
			AddEntry(&entry);

			entry.SetColor(RGB(255, 64, 0));
			entry.SetVal(.02);
			AddEntry(&entry);

			entry.SetColor(RGB(255, 128, 0));
			entry.SetVal(.03);
			AddEntry(&entry);

			entry.SetColor(RGB(255, 192, 0));
			entry.SetVal(.04);
			AddEntry(&entry);

			entry.SetColor(RGB(255, 255, 0));
			entry.SetVal(.05);
			AddEntry(&entry);

			entry.SetColor(RGB(192, 240, 0));
			entry.SetVal(.06);
			AddEntry(&entry);

			entry.SetColor(RGB(144, 224, 0));
			entry.SetVal(.07);
			AddEntry(&entry);

			entry.SetColor(RGB(96, 208, 0));
			entry.SetVal(.08);
			AddEntry(&entry);

			entry.SetColor(RGB(48, 192, 0));
			entry.SetVal(.09);
			AddEntry(&entry);

			entry.SetColor(RGB(0, 176, 0));
			entry.SetVal(.10);
			AddEntry(&entry);

			entry.SetColor(RGB(0, 128, 96));
			entry.SetVal(.11);
			AddEntry(&entry);

			entry.SetColor(RGB(0, 112, 128));
			entry.SetVal(.12);
			AddEntry(&entry);

			entry.SetColor(RGB(0, 96, 160));
			entry.SetVal(.13);
			AddEntry(&entry);

			entry.SetColor(RGB(0, 80, 192));
			entry.SetVal(.14);
			AddEntry(&entry);

			entry.SetColor(RGB(0, 64, 224));
			entry.SetVal(.15);
			AddEntry(&entry);

			entry.SetColor(RGB(96, 64, 192));
			entry.SetVal(.16);
			AddEntry(&entry);

			entry.SetColor(RGB(104, 48,176));
			entry.SetVal(.17);
			AddEntry(&entry);

			entry.SetColor(RGB(112, 32, 160));
			entry.SetVal(.18);
			AddEntry(&entry);

			entry.SetColor(RGB(120, 16, 144));
			entry.SetVal(.19);
			AddEntry(&entry);

			entry.SetColor(RGB(128, 0, 128));
			entry.SetVal(.20);
			AddEntry(&entry);

			entry.SetColor(RGB(112, 0, 112));
			entry.SetVal(.21);
			AddEntry(&entry);



		}
		break;
	case W_DATA:
		{
		double hi = pFlamMap->GetLayerHi(WOODY), lo = max(0, pFlamMap->GetLayerLo(WOODY));
		
			SetLo(lo);
			SetHi(hi);
			SetCategorical(FALSE);
			decimalPlaces = 2;
			SetUnits(pFlamMap->GetLayerUnits(WOODY) - 1);
			

			CLegendEntry entry;
			entry.SetColor(RGB(255, 255, 255));
			entry.SetVal(NODATA_VAL);
			AddEntry(&entry);

			entry.SetColor(RGB(208, 208, 208));
			entry.SetVal(0);
			AddEntry(&entry);

			entry.SetColor(RGB(255, 0, 0));
			entry.SetVal(.01);
			AddEntry(&entry);

			entry.SetColor(RGB(255, 64, 0));
			entry.SetVal(.02);
			AddEntry(&entry);

			entry.SetColor(RGB(255, 128, 0));
			entry.SetVal(.03);
			AddEntry(&entry);

			entry.SetColor(RGB(255, 192, 0));
			entry.SetVal(.04);
			AddEntry(&entry);

			entry.SetColor(RGB(255, 255, 0));
			entry.SetVal(.05);
			AddEntry(&entry);

			entry.SetColor(RGB(192, 240, 0));
			entry.SetVal(.06);
			AddEntry(&entry);

			entry.SetColor(RGB(144, 224, 0));
			entry.SetVal(.07);
			AddEntry(&entry);

			entry.SetColor(RGB(96, 208, 0));
			entry.SetVal(.08);
			AddEntry(&entry);

			entry.SetColor(RGB(48, 192, 0));
			entry.SetVal(.09);
			AddEntry(&entry);

			entry.SetColor(RGB(0, 176, 0));
			entry.SetVal(.10);
			AddEntry(&entry);

			entry.SetColor(RGB(0, 128, 96));
			entry.SetVal(.11);
			AddEntry(&entry);

			entry.SetColor(RGB(0, 112, 128));
			entry.SetVal(.12);
			AddEntry(&entry);

			entry.SetColor(RGB(0, 96, 160));
			entry.SetVal(.13);
			AddEntry(&entry);

			entry.SetColor(RGB(0, 80, 192));
			entry.SetVal(.14);
			AddEntry(&entry);

			entry.SetColor(RGB(0, 64, 224));
			entry.SetVal(.15);
			AddEntry(&entry);

			entry.SetColor(RGB(96, 64, 192));
			entry.SetVal(.16);
			AddEntry(&entry);

			entry.SetColor(RGB(104, 48,176));
			entry.SetVal(.17);
			AddEntry(&entry);

			entry.SetColor(RGB(112, 32, 160));
			entry.SetVal(.18);
			AddEntry(&entry);

			entry.SetColor(RGB(120, 16, 144));
			entry.SetVal(.19);
			AddEntry(&entry);

			entry.SetColor(RGB(128, 0, 128));
			entry.SetVal(.20);
			AddEntry(&entry);

			entry.SetColor(RGB(112, 0, 112));
			entry.SetVal(.21);
			AddEntry(&entry);


		}
		break;
	}

	// get the frequencies for each entry
    // by counting the number of cell values which fall within a certain range
	if(numEntries > 0)
	{
		
	

	    for (int col=0; col < pFlamMap->GetNumCols(); col++)
		    for (int row = 0; row < pFlamMap->GetNumRows(); row++)
		    {
	            float cellVal = pFlamMap->GetLayerValueByCell(ELEV,col,row);
				if(cellVal < lo || cellVal > hi)  // cell contains invalid data value
				{
					int freq = entries[0]->GetFrequency() + 1;
					entries[0]->SetFrequency(freq);
				} else // check entries to find where value belongs
				{
					
                   for(int i = 2; i < numEntries; i++)
			       {
				       if (entries[i-1]->GetVal() <= cellVal)
					   {
				          if(entries[i]->GetVal() > cellVal )  // value fits between lo/hi ranges
						  {
							  int freq = entries[i-1]->GetFrequency() + 1;
					          entries[i-1]->SetFrequency(freq);
							  break;
						  }
						  if (i == (numEntries - 1))    // final bin is for values greater than "hi"
						  {
                              int freq = entries[i]->GetFrequency() + 1;
					          entries[i]->SetFrequency(freq);
						  }

					   }
					     
			       }

				}

		    }

	}



}
