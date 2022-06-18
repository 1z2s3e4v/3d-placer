#ifndef GROUTEMAP
#define GROUTEMAP

#include <vector>
using namespace std;

#include "CongMap.h"

class CPlaceDB;

class GRouteMap
{
    public:
	GRouteMap( CPlaceDB* );
        void InitMap( const int& xSize, const int& ySize );
	
	bool LoadISPD2007Res( const char* in, const char* res );
	bool LoadBoxRouterRes( const char* in, const char* res );
	void LoadCongMap( CCongMap* pCongMap );
	void SetHVCapacity( const int& hCutCap, const int& vCutCap );

	void AddEdge( const int& x1, const int& y1, const int& x2, const int& y2 );
	void AddEdge2007( const int& x1, const int& y1, const int& x2, const int& y2 );
	void OutputGnuplotFigure( const char* filename );
	void ShowMapInfo();
	int  ComputeOverflow( bool show = false );
	void ComputeWireDensity();

	// 2007-03-27
	void AddM1M2MetalDensity( const double& ratio );
	void AddMacroMetalDensity( const double& ratio );
	void AddMacroMetalDensity( const CPoint& p1, const CPoint& p2, const double& ratio );
	void AddPGWireDensity( int& gridSize );

	static double GetNormalizedCopperThickness( const double& wireDensity );

	// 2007-04-09
	static double GetDummyAmount( const double& wireDensity );
	double GetTotalDummyAmount();

	// 2007-03-29 (donnie)
	double GetBinWireDensity( const int& i, const int& j );

    private:
	void LoadSettings( const char* );
	void LoadResults( const char* );
	string ShowCopperThicknessStatistics();

	void LoadSettings2007( const char* );
	void LoadResults2007( const char* );
	int m_left, m_bottom, m_xStep, m_yStep;

    private:
	vector< vector< double > > m_vCut;  // vertical cut
	vector< vector< double > > m_hCut;  // horizontal cut
	int m_vCap;
	int m_hCap;
	int m_nNets;
	int m_totalLength;
	double m_hWirePitch;	// horizontal routing pitch
	double m_vWirePitch;	// vertical routing pitch
	double m_vLayers;	// metal layer # for vertical routing
	double m_hLayers;	// metal layer # for horizontal routing

	static double m_copperThicknessLUTx[13];
	static double m_copperThicknessLUTy[13];
	static int    m_copperThicknessLUTsize;

	static double m_dummyAmountLUTx[13];
	static double m_dummyAmountLUTy[13];
	static int    m_dummyAmountLUTsize;

	vector< vector< double > > m_wireDensity;
	vector< vector< double > > m_copperThickness;

	CPlaceDB* m_pPlaceDB;

	// 04  14  24  34  44
	// 03  13  23  33  43
	// 02  12  22  32  42
	// 01  11  21  31  41
	// 00  10  20  30  40

	//          H(0,1)
	// V(0,0)   Bin(0,0)   V(1,0)
	//          H(0,0)
	
    private:

	double m_binWidth;
	double m_binHeight;
	CRect  m_region;
	int GetBinX( const double& x );
	int GetBinY( const double& y );

	// 2007-03-31 (donnie)
    public:
	void ComputeAllCapacity();
	double GetHCap() { return m_hCap; }
	double GetVCap() { return m_vCap; }
	double GetHCap( const int &i, const int &j );
	double GetVCap( const int &i, const int &j );

    private:
	void RemoveM1M2WireCapacity( const double& ratio );
	void RemoveMacroWireCapacity( const double& ratio );
	void RemoveMacroWireCapacity( const CPoint& p1, const CPoint& p2, const double& ratio );
	vector< vector< double > > m_hCapArray;
	vector< vector< double > > m_vCapArray;

};


#endif
