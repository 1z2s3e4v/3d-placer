// placeutil.h
// created by donnie
// 2005-12-20


#ifndef PLACEUTIL
#define PLACEUTIL

#include "placedb.h"
#include "ParamPlacement.h"

double PlaceLegalize( CPlaceDB& fplan, 
                      CParamPlacement& param, 
		      double& wl1, double& wl2 );

void OutputGnuplotPM3DScript( const char* filename, const char* dat, int xrange, int yrange );
void OutputGnuplotPM3D( const char* filename, const char* dat, 
	int xrange, int yrange, double zrangeBase, double zrange, string title="", double limit=0 );

// 2006-04-02 (donnie)
class CPlaceDBScaling
{
    public:
	static void XShift( CPlaceDB&, const double& shift );
	static void XScale( CPlaceDB&, const double& scale );
	static void YShift( CPlaceDB&, const double& shift );
	static void YScale( CPlaceDB&, const double& scale );
};

class CPlaceFlipping
{
    public:
	static void RemoveCellPinOffset( CPlaceDB& );
	static void CheckCellOrientation( CPlaceDB& );
	static void CellFlipping( CPlaceDB& );
};

class CPlaceUtil
{
    public:
	static void WriteXGRFormat( CPlaceDB*, const char* filename );   // Generate test case for X-GlobalRouter (XGR)
	static void WriteISPDGRCFormat( CPlaceDB* pDB, const char* filename );

	// 2006-09-23 (donnie)
	static void CreateRowSites( CPlaceDB*, const double& whitespace, const double& aspectRatio ); 

	// 2006-10-21 (donnie)
	static void SavePlacement( const CPlaceDB&, vector<CPoint>& pos );
	static void LoadPlacement( CPlaceDB&, const vector<CPoint>& pos );
	
	// 2007-06-24 (donnie)
	// 2007-07-18 (donnie)
	static double GetDisplacement( const vector<CPoint>& p1, const vector<CPoint>& p2 );
	static double GetMaxDisplacement( const vector<CPoint>& p1, const vector<CPoint>& p2 );
	static int    GetDisplacementCount( const vector<CPoint>& p1, const vector<CPoint>& p2, const double dis );
	static void   ShowMigrationInfo( /*const*/ CPlaceDB& db, const vector<CPoint>& p1, const vector<CPoint>& p2 );

	// 2007-07-22 (donnie)
	static double GetTotalOverlapArea( CPlaceDB& db );
	static double GetTotalSiteWidth( vector<CSiteRow>& site );
	static void   SimulateGateSizing( CPlaceDB* db );
};

class CPlacePlotWithNet
{
    public:
	CPlacePlotWithNet( CPlaceDB* pDB );
	void CreateXSteinerNets();
	void OutputGnuplotFigure( const char* filename );

    private:
	void DecomposeXSteinerNet( const Net& net );
	void CreateXSegments( const CPoint& p1, const CPoint& p2 );
	void ComputeTotalLength();
	void PlotHeader( FILE* );
	void PlotCells( FILE* );
	void PlotNets( FILE*, const int& layer );
    
    private:
	vector<double> m_totalLength;
	vector< vector<CPoint> > m_segments;	    // segments[layer][point1], segments[layer][point2], ... 
        CPlaceDB* m_pDB;	
};

class CMatrixPlotter
{
    public:
	static void OutputGnuplotFigure( 
		const vector< vector< double > >&, 
		const char* filename, 
		string title="", 
		double limit=0,
	        bool scale=false,
		double limitBase=0 );
	static void OutputGnuplotFigure( 
		const vector< vector< float > >&, 
		const char* filename, 
		string title="", 
		float limit=0,
	        bool scale=false,
		float limitBase=0 );

};

// 2007-02-07 (donnie)
class CPlaceFlux
{
    public:
	CPlaceFlux( CPlaceDB* db ) : m_pDB( db ) {};
	void OutputGnuplotFigure( const char* filename );
	void UpdateFlux( const int& xNum, const int& yNum );
	void UpdateFlux( const double& x, const double& y, 
		const double& width, const double& height,
	        const int& xNum, const int& yNum	);
    private:
	CPlaceDB* m_pDB;
	//vector< vector< int > > m_flux;
	vector< vector< double > > m_flux;  // Use "double" because of CMatrixPlotter.
};

/*
 * Exmaple usage of the CPlaceFlux
 *
 * name = param.outFilePrefix + ".ntup.flux";
 * CPlaceFlux fluxMap( &placedb );
 * fluxMap.UpdateFlux( gridSize, gridSize );
 * fluxMap.OutputGnuplotFigure( name.c_str() );
 *
 * CSteinerDecomposition* pDec = NULL;
 * pDec = new CSteinerDecomposition( placedb );
 * pDec->Update();
 * name = param.outFilePrefix + ".ntup.fluxS";
 * fluxMap.UpdateFlux( gridSize, gridSize );
 * fluxMap.OutputGnuplotFigure( name.c_str() );
 * pDec->Restore();
 * delete pDec;
 */



#endif
