#ifndef PLACEBIN_H
#define PLACEBIN_H

#include "../PlaceDB/placedb.h"
//#include "lefdef.h"

#include <vector>
using namespace std;

class CCellmoving;

class CPlaceBin 
{
    friend class CCellmoving; //by tchsu

    //Added by Jin 20060310
    friend class CTetrisLegal; 
    //@Added by Jin 20060310
    
public:
    
    CPlaceBin( CPlaceDB& );
    CPlaceBin();

    void   CreateGrid( int binNumberX, int binNumberY );
    void   CreateGrid( double binWidth, double binHeight );

    //Brian 2007-05-06

    double m_dAvgPitch;
    double m_dLevelNum;
    //void   CreateGridNet( int binNumber );
    //void   CreateGridNet();
    void   UpdateBinFreeSpaceNet(); 
    void   UpdateBinUsageNet();
    //void   CatchCongParam();
    void   OutputCongMap(string filename);

    //@Brian 2007-05-06

    void   UpdateBinUsage();
    void   UpdateBinFreeSpace();

    double GetNonZeroBinPercent(); 
    double GetTotalOverflowPercent( const double& util );
    double GetMaxUtil();
    int    GetBinNumberH()   { return m_binNumberH; }
    int    GetBinNumberW()   { return m_binNumberW; }
    double GetBinWidth()     { return m_binWidth;   }
    double GetBinHeight()    { return m_binHeight;  }

    void   ShowInfo( const double& targetUtil );
    double GetPenalty( const double& targetUtil );
    double GetOverflowRatio( const double& util );

    void   OutputBinUtil( string filename, bool msg = true );  // for gnuplot TODO

private:
    
    void   CreateGrid();
    void   ClearBinUsage();

public:
    
    double GetBinX( const int& binX )
    { return m_pDB->m_coreRgn.left + binX * m_binWidth; }
    double GetBinY( const int& binY )
    { return m_pDB->m_coreRgn.bottom + binY * m_binHeight; }

    // 2006-10-23 donnie
    int GetHorizontalBinNumber( const double& x )
    { return max( 0, static_cast<int>( floor( (x - m_pDB->m_coreRgn.left) / m_binWidth ) ) ); }
    int GetVerticalBinNumber( const double& y )
    { return max( 0, static_cast<int>( floor( (y - m_pDB->m_coreRgn.bottom) / m_binHeight ) ) ); }

private:

    CPlaceDB* m_pDB;
    double m_totalMovableArea;
    double m_binWidth;
    double m_binHeight;
    int    m_binNumberW;
    int    m_binNumberH;


public:	// indark
    vector< vector< double > > m_binSpace;
    vector< vector< double > > m_binUsage;

};


// 2007/07/09 (donnie)
class CGreedyRefinement
{
    public:

	CGreedyRefinement( CPlaceDB* pDB );
	~CGreedyRefinement();

	void CreateBlockList();

	void CreateGrid( int gridHNumber, int gridVNumber );

	void RefineBlocks();
	bool RefineBlock( int blockId );

    private:

	int    GetBinNumberH()   { return m_pBin->GetBinNumberH(); }
	int    GetBinNumberW()   { return m_pBin->GetBinNumberW(); }
	double GetBinWidth()     { return m_pBin->GetBinWidth();   }
	double GetBinHeight()    { return m_pBin->GetBinHeight();  }

	int    GetHorizontalBinNumber( int blockId );
	int    GetVerticalBinNumber( int blockId );
	
	double GetCostChange( int blockId, int direction );
	double GetHPWLChange( int blockId, int direction );
	double GetDensityChange( int blockId, int oldBinX, int oldBinY, int newBinX, int newBinY );
	
    private:

	CPlaceDB*   m_pDB;
	CPlaceBin*  m_pBin;
	vector<int> m_blockList;    // refinement sequence

	static const int m_directionCount;
	static const int m_xMove[8];	    // define 8 directions
	static const int m_yMove[8];
};



#endif
