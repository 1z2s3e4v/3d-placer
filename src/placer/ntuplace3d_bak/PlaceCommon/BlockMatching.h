#ifndef BLOCK_MATCHING_H
#define BLOCK_MATCHING_H

#include "lap_wrapper.h"
#include "util.h"

#include <vector>
#include <set>
using namespace std;

class CPlaceDB;

class CBlockMatching
{
    public:
	CBlockMatching( CPlaceDB* );
	~CBlockMatching();

	void HPWLOptimize();
	//void CongestionOptimize();

	void SelectBlocks();
	void SelectFirstBlock( const unsigned int& num, unsigned int start=0 );
	void SelectIndependentBlock( const unsigned int& num, unsigned int start=0 );
	void SelectRandomBlock( const CRect& region, int num );
	void SelectRandomBlock( const vector<int>& blocks, const int& num );

	void UseHPWLWeight();
	//void UseCongestionWeight();
	void UseWireDensityWeight();
	
	void Matching();

    private:
	CPlaceDB*     m_pDB;
	vector< int > m_blockList;
	set< int >    m_usedBlockList;
	CLapWrapper*  m_pLapSolve;

	vector<double> m_oldX;
	vector<double> m_oldY;

    private:
	void SaveBlockLocation();

};

#endif
