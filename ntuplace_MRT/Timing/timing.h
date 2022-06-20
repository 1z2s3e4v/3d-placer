#ifndef __TIMING__
#define __TIMING__

#include <vector>
#include <string>
#include <cfloat>
#include "placedb.h"
#include "arghandler.h"
#include "sta.h"
#include "netlist.h"

class TimingAnalysis
{
    public:
	TimingAnalysis( CPlaceDB& placedb ) : 
	    m_placedb( placedb ), 
	    m_averageUnitCap(0.0), 
	    m_averageMetalWidth(0.0),
	    m_averageCap(0.0)	{}
	void Initialize( void );
	void ConstructNetlist( void );
	void RunSTA( void );
	void ResetSTA( void );

	CPlaceDB& m_placedb;
	Liberty m_libParser;
	
	//module id map from PlaceDB to STA
	vector<int> m_mid_mapPlaceDBToSTA;
	//module id map from STA to PlaceDB
	vector<int> m_mid_mapSTAToPlaceDB;
	double m_averageUnitCap;
	double m_averageMetalWidth;
	double m_averageCap;

	void SetNetLoad( void );
	void RerunSTA( void );

	double m_WNS;
	double m_TNS;
};

#endif
