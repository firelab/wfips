#ifndef LANDSCAPE_DATA_HEADER
#define LANDSCAPE_DATA_HEADER
//#ifdef BUILD_FLAMMAPDLL 
//#define FLAMMAPDLL_EXPORT __declspec(dllexport) 
//#else 
//#define FLAMMAPDLL_EXPORT __declspec(dllimport) 
//#endif 
class FlamMap;

typedef struct
{// structure for holding basic cell information
	short e;                 // elevation
	short s;                 // slope
	short a;                 // aspect
	short f;                 // fuel models
	short c;                 // canopy cover
}celldata;


typedef struct
{// structure for holding optional crown fuel information
	short h;				// canopy height
	short b;				// crown base
	short p;				// bulk density
}crowndata;


typedef struct
{// structure for holding duff and woody fuel information
     short d;				// duff model
	short w;				// coarse woody model
}grounddata;


struct CanopyCharacteristics
{	// contains average values, landscape wide, temporary until themes are used
	double DefaultHeight;
	double DefaultBase;
	double DefaultDensity;
	double Height;
	double CrownBase;
	double BulkDensity;
	double Diameter;
	double FoliarMC;
	int Tolerance;
	int Species;
     CanopyCharacteristics();
};


typedef struct
{
	short elev;
     short slope;
     short aspect;
     short fuel;
     short cover;               // READ OR DERIVED FROM LANDSCAPE DATA
     double aspectf;
     double height;
     double base;
     double density;
     double duff;
     int woody;
} LandscapeStruct;


class LandscapeData
{
public:
	LandscapeData(FlamMap *_pFlamMap);
	LandscapeStruct ld;
	FlamMap *pFlamMap;
	void ElevConvert(short);
	void SlopeConvert(short);
	void AspectConvert(short);
	void FuelConvert(short);
	void CoverConvert(short);
     void HeightConvert(short);
     void BaseConvert(short);
     void DensityConvert(short);
     void DuffConvert(short);
     void WoodyConvert(short);
};

#endif
