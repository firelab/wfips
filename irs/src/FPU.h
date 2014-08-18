// FPU.h
// class definitions for CFPU, information about individual FPUs
// 3/13 for IRS

#ifndef FPU_H
#define FPU_H

#include <string>												//C++ standard string class
using std::string;
#include <vector>												//vector class
using std::vector;


class CFPU {

public:
	CFPU();
	CFPU( string FPUid, double Average[365], double Max[365], double FausMax[365] );		// constructor

	~CFPU();													//destructor

	CFPU( const CFPU &fpu );									//copy constructor for 
	const CFPU &operator=( const CFPU &fpu);					//Assignment operator

	// Set and get functions for FWA variables
	void SetFPUID( string id );									//set the FPU ID
	string GetFPUID();											//get the FPU ID
	void SetAverageVector( vector< double > Average );			//set the average vector with a vector
	vector< double > GetAverageVector();						//get the average vector
	void SetAverageIdx( double Value, int Index );				//set an average value using an index
	double GetAverageIdx( int Index);							//get an average value from the vector
	void SetMaxVector( vector< double > Max );					//set the average vector with a vector
	vector< double > GetMaxVector();							//get the max vector
	void SetMaxIdx( double Value, int Index );					//set a max value in the vector
	double GetMaxIdx( int Index );								//get a max value from the vector
	void SetFausMaxVector( vector< double > FausMax );			//set the Faus Max Vector with an index
	vector< double > GetFausMaxVector();						//get the Faus Max Vector
	void SetFausMaxIdx(  double Value, int Index );				//set a Faus Max value 
	double GetFausMaxIdx( int Index );							//get a Faus Max value
	void SetExpectLevelVector( vector< double > expect );		//set the expected level vector with a vector
	vector< double > GetExpectLevelVector();					//get the expected level vector
	void SetExpectLevelIdx( double Value, int Index );			//set an expected level value
	double GetExpectLevelIdx( int Index );						//get an expected level value
	void SetNumFiresVector( vector< int > numfires );			//set the number of fires with a vector
	vector< int > GetNumFiresVector();							//get the vector with the number of fires
	void SetNumFires( int Value, int Julian );					//set the number of fires for a julian day
	int GetNumFires( int Julian );								//get the number of fires for a julian day
	void AddFire( int Julian );									//add a fire to a julian day
	void SetReliance( double reliance );						//set the FPU's reliance on large airtankers
	double GetReliance();										//get the FPU's reliance on large airtankers
	
private:
	string m_FPUID;												//FPU id
	vector< double > m_Average;									//Average number of fires for each day 
	vector< double > m_Max;										//Maximum number of fires for each day
	vector< double > m_FausMax;									//Other Maximum value to use for maximum number of fires
	vector< double > m_ExpectLevel;								//Expected Level for the FPU for a scenario
	vector< int > m_NumFires;									//For a scenario the number of fires for each day
	double m_ATReliance;										//0-1 indicates how much reliance the FPU has on large airtankers

	
};	//end class CFPU

#endif	// FPU_H
