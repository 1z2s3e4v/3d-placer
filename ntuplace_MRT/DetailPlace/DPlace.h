#ifndef _DPLACE_
#define _DPLACE_

#include "bbcellswap.h"
#include "placedb.h"
#include "detail.h"
#include "ParamPlacement.h"
#include <vector>
#include <cstdio>
#include <iostream>
#include <fstream>

class CDetailPlacerParam
{
	friend class CDetailPlacer;
	friend std::ostream& operator<<( std::ostream& ostr, const CDetailPlacerParam& dpParam );
	private:
		//For bbcellswap
		bool bRunBBCellSwap;
		int bbWindowSize;
		int bbOverlapSize;
		int bbIteration;
	public:
		//Set the default parameter
		CDetailPlacerParam(void); 
		void SetRunBBCellSwap(void) { bRunBBCellSwap = true; }
		void SetBBWindowSize( const int& set_size ) { bbWindowSize = set_size; }
		void SetBBOverlapSize( const int& set_size ) { bbOverlapSize = set_size; }
		void SetBBIteration( const int& set_iteration ) { bbIteration = set_iteration; }
};

std::ostream& operator<< (std::ostream& ostr, const CDetailPlacerParam& dpParam );

class CDetailPlacer
{
	public:
		CDetailPlacer( CPlaceDB& set_placedb, const CParamPlacement& param, const CDetailPlacerParam& dpParam );
		~CDetailPlacer(void){}
		CPlaceDB& m_placedb;
		const CParamPlacement& m_param;
		const CDetailPlacerParam& m_dpParam;
		void DetailPlace(void);
		std::vector<CSegment> m_segments;
		void InitForBBCellSwap(void);
	private:
		vector<int> m_fixedId;	// for congestion aware 
};

//Class for white space allocation
class CWSDistribution
{
	public:
		CWSDistribution( CPlaceDB& set_placedb, 
				std::vector<CSegment>& set_segments ) : 
			m_placedb( set_placedb ),
			m_segments( set_segments ){};
		~CWSDistribution(void){}	
		CPlaceDB& m_placedb;
		std::vector<CSegment>& m_segments;
		void DoIt(void);
		
};

//Class for Dynamic Programming Cell
class CDPCell
{
public:
	enum DPDirect
	{
		dp_notset,
		dp_left,
		dp_topleft
	};

	CDPCell(const double& set_cost, const DPDirect& set_dirct = dp_notset ) :
		m_cost( set_cost ), m_dirct( set_dirct ) {}
	~CDPCell(void){}

	double m_cost;
	DPDirect m_dirct;
};

class CDPOneSegment
{
	public:
		CDPOneSegment(CPlaceDB& set_placedb, 
				CSegment& set_segment, 
				const int& set_dpsite_width = 1 ); 
		~CDPOneSegment(void){}
		CPlaceDB& m_placedb;
		CSegment& m_segment;
		int m_dpsite_width;	//Width of dp sites
		int m_dpsite_num;   //Number of dp sites
		int m_module_num;   //Number of modules (cells)
		double m_left;		//Left boundary of the leftmost site
		double m_right;		//Right boundary of the rightmost site

		//m_dpsites[site index][cell index]
		vector< vector<CDPCell> > m_dpsites;
		const vector<int>& m_module_ids;
		void DoIt(void);
		double GetModuleTotalNetLength( const int& module_id );
};

class ComparePseudoSegment
{
    public:
	bool operator() (const CSegment& s, const CSegment& compSeg )
	{
	    if( s.m_bottom == compSeg.m_bottom )
	    {
		if( s.m_left <= compSeg.m_left )
		{
		    return s.m_right < compSeg.m_right;
		}
		else
		{
		    return false;
		}		    
	    }
	    else
	    {
		return s.m_bottom < compSeg.m_bottom;
	    }
	}
};

#endif
