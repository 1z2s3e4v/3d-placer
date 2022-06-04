// 2006-02-27 Tung-Chieh Chen
// The same metric in ISPD 2006 placement contest

//#include "arghandler.h"
#include "../PlaceDB/placedb.h"
#include "placebin.h"
#include "placeutil.h"

#include <vector>
#include <algorithm>
using namespace std;

CPlaceBin::CPlaceBin( CPlaceDB& db )
{
    m_pDB = &db;
    m_totalMovableArea = 0;
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
	if( m_pDB->m_modules[i].m_isFixed == false )
	    // tranformed macros are still in "movable area". (donnie) 2007-07-17
	    m_totalMovableArea += m_pDB->m_modules[i].m_area;
    
}

void CPlaceBin::CreateGrid( int xGrid, int yGrid )
{
    m_binWidth  = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left )   / xGrid;
    m_binHeight = ( m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom ) / yGrid;
    m_binNumberH = xGrid;
    m_binNumberW = yGrid;
    CreateGrid();
    UpdateBinFreeSpace(); 
    UpdateBinUsage();
}

void CPlaceBin::CreateGrid( double width, double height )
{
    m_binWidth   = width;
    m_binHeight  = height;
    m_binNumberW = static_cast<int>(ceil( ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / width ));
    m_binNumberH = static_cast<int>(ceil( ( m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom ) / height ));

    CreateGrid();     
    UpdateBinFreeSpace(); 
    UpdateBinUsage();
}

void CPlaceBin::CreateGrid()
{
    m_binSpace.resize( m_binNumberW );
    m_binUsage.resize( m_binNumberW );
    for( int i=0; i<m_binNumberW; i++ )
    {
	m_binSpace[i].resize( m_binNumberH );
	m_binUsage[i].resize( m_binNumberH );
    }
}

void CPlaceBin::ClearBinUsage()
{
    for( int i=0; i<m_binNumberW; i++ )
	for( int j=0; j<m_binNumberH; j++ )
	    m_binUsage[i][j] = 0.0;
}

void CPlaceBin::UpdateBinUsage()
{
    ClearBinUsage();
    for( unsigned int b=0; b<m_pDB->m_modules.size(); b++ )
    {
	//if( m_pDB->m_modules[b].m_isOutCore || m_pDB->m_modules[b].m_isFixed )
	if( m_pDB->BlockOutCore( b ) || m_pDB->m_modules[b].m_isFixed )
	    continue;
	
	if( m_pDB->m_modules[b].m_height > m_pDB->m_rowHeight )
	    continue;	// transform macros

	double debug_area = 0;

	double w = m_pDB->m_modules[b].m_width;
	double h = m_pDB->m_modules[b].m_height;

	// bottom-left
	double left   = m_pDB->m_modules[b].m_x;
	double bottom = m_pDB->m_modules[b].m_y;
	double right  = left   + w;
	double top    = bottom + h;

	// find nearest gird
	int binX = GetHorizontalBinNumber( left );
	int binY = GetVerticalBinNumber( bottom );

	for( int xOff=binX; xOff<m_binNumberW; xOff++ )
	{
	    double binX = GetBinX( xOff );
	    if( binX >= right )
		break;

	    for( int yOff=binY; yOff<m_binNumberH; yOff++ )
	    {
		double binY = GetBinY( yOff );
		if( binY >= top )
		    break;

		double binXright = binX + m_binWidth;
		double binYtop = binY + m_binHeight;
		if( binXright > m_pDB->m_coreRgn.right )
		    binXright = m_pDB->m_coreRgn.right;
		if( binYtop > m_pDB->m_coreRgn.top )
		    binYtop = m_pDB->m_coreRgn.top;

		double common_area = 
		    getOverlap( left, right, binX, binXright ) *
		    getOverlap( bottom, top, binY, binYtop );

		m_binUsage[xOff][yOff] += common_area;

		debug_area += common_area;
	    }

	} // for each bin

	if( debug_area != m_pDB->m_modules[b].m_area )
	{
	    // cell may outside the region
	    //printf( "error in area\n" );
	}	    

    } // for each block
}


void CPlaceBin::UpdateBinFreeSpace()
{
    // calculate total space in bins
    for( int i=0; i<m_binNumberW; i++ )
    {
	for( int j=0; j<m_binNumberH; j++ )
	{
	    double left = GetBinX( i );
	    double right = left + m_binWidth;
	    double bottom = GetBinY( j );
	    double top = bottom + m_binHeight;
	    if( right > m_pDB->m_coreRgn.right )
		right = m_pDB->m_coreRgn.right;
	    if( top > m_pDB->m_coreRgn.top )
		top = m_pDB->m_coreRgn.top;
	    m_binSpace[i][j] = (right-left) * (top-bottom);
	}
    }


    for( unsigned int b=0; b<m_pDB->m_modules.size(); b++ )
    {

	if( !m_pDB->m_modules[b].m_isFixed &&
		    m_pDB->m_modules[b].m_height == m_pDB->m_rowHeight )
	    continue;
	
	double w = m_pDB->m_modules[b].m_width;
	double h = m_pDB->m_modules[b].m_height;

	// bottom-left
	double left   = m_pDB->m_modules[b].m_x;
	double bottom = m_pDB->m_modules[b].m_y;
	double right  = left   + w;
	double top    = bottom + h;

	// find nearest gird
	int binX = GetHorizontalBinNumber( left );
	int binY = GetVerticalBinNumber( bottom );

	for( int xOff=binX; xOff<m_binNumberW; xOff++ )
	{
	    double binX = GetBinX( xOff );
	    if( binX >= right )
		break;

	    for( int yOff=binY; yOff<m_binNumberH; yOff++ )
	    {
		double binY = GetBinY( yOff );
		if( binY >= top )
		    break;

		double binXright = binX + m_binWidth;
		double binYtop = binY + m_binHeight;
		if( binXright > m_pDB->m_coreRgn.right )
		    binXright = m_pDB->m_coreRgn.right;
		if( binYtop > m_pDB->m_coreRgn.top )
		    binYtop = m_pDB->m_coreRgn.top;

		double common_area = 
		    getOverlap( left, right, binX, binXright ) *
		    getOverlap( bottom, top, binY, binYtop );
		m_binSpace[xOff][yOff] -= common_area;
	    }

	} // for each bin

    } // for each block
}

double CPlaceBin::GetNonZeroBinPercent()
{
    int nonZero = 0;
    for( int i=0; i<m_binNumberW; i++ )
    {
	for( int j=0; j<m_binNumberH; j++ )
	{
	    if( m_binUsage[i][j] > 0.0 )
		nonZero++;
	}
    }
    return (double)nonZero / m_binNumberW / m_binNumberH;
}

double CPlaceBin::GetTotalOverflowPercent( const double& util )
{
    double over = 0;
    for( int i=0; i<m_binNumberW; i++ )
    {
	for( int j=0; j<m_binNumberH; j++ )
	{
	    double targetUsage = m_binSpace[i][j] * util;
	    if( m_binUsage[i][j] > targetUsage )
	    {
		over += m_binUsage[i][j] - targetUsage;
	    }
	}
    }
    return over / m_totalMovableArea;
}

double CPlaceBin::GetMaxUtil()
{
    double maxUtil = 0;
    for( int i=0; i<m_binNumberW; i++ )
    {
	for( int j=0; j<m_binNumberH; j++ )
	{
	    if( m_binSpace[i][j] < 0.00000001 )   //  2007-04-29 (donnie)
		continue;
	    double util = (double)m_binUsage[i][j] / m_binSpace[i][j];
	    if( util > maxUtil )
		maxUtil = util;
	}
    }
    return maxUtil;
}

double CPlaceBin::GetOverflowRatio( const double& targetUtil )
{
    double totalOver = 0;
    for( int i=0; i<m_binNumberW; i++ )
    {
	for( int j=0; j<m_binNumberH; j++ )
	{
	    double util = m_binUsage[i][j] / m_binSpace[i][j];
	    if( util > targetUtil )
		totalOver += m_binUsage[i][j] - m_binSpace[i][j]*targetUtil;
	}
    }
    return totalOver / m_totalMovableArea;
}	


double CPlaceBin::GetPenalty( const double& targetUtil )
{
    if(targetUtil == 1.0) return 0.0;

    double totalOver = 0;
    for( int i=0; i<m_binNumberW; i++ )
    {
	for( int j=0; j<m_binNumberH; j++ )
	{
	    double util = m_binUsage[i][j] / m_binSpace[i][j];
	    if( util > targetUtil )
	    {
		totalOver += m_binUsage[i][j] - m_binSpace[i][j]*targetUtil;
	    }
	}
    }
    //double scaled_overflow_per_bin = 
    //	(totalOver * m_binWidth * m_binHeight * targetUtil) / (m_totalMovableArea * 400 );

    // 2006-04-14 (ISPD06: m_binWidth * m_binHeight / 400 )
    double scaled_overflow_per_bin = 
	(totalOver * 36 * targetUtil) / m_totalMovableArea;
	
    return scaled_overflow_per_bin * scaled_overflow_per_bin;
}

void CPlaceBin::OutputBinUtil( string filename, bool msg )
{
    if( msg )
	printf( "Output utilization: %s\n", filename.c_str() );


    if( m_binUsage.size() == 0 )
    {
	fprintf( stderr, "CPlaceBin: Bin size is not assigned\n" );
	return;
    }
    
    double areaPerBin = m_binWidth * m_binHeight;
    CMatrixPlotter::OutputGnuplotFigure( m_binUsage, filename.c_str(), filename, areaPerBin, true, 0.0 );
    
#if 0 
    string plt = filename + ".plt";
    string dat = "dat/" + filename + ".plt.dat";
    system( "mkdir dat 1> /dev/null 2> /dev/null" );

    //OutputGnuplotPM3DScript( plt.c_str(), dat.c_str(), m_binNumberH, m_binNumberW );
    OutputGnuplotPM3D( plt.c_str(), dat.c_str(), 
	    m_binNumberH, m_binNumberW, 0.0, GetMaxUtil(), filename.c_str(), 1.0 );
    
    FILE* out = fopen( dat.c_str(), "w" );

    if( out == NULL )
    {
	printf( "Failed to create a file\n" );
	return;
    }

    for( int j=0; j<m_binNumberH; j++ )
    {
	for( int i=0; i<m_binNumberW; i++ )
	    fprintf( out, "%.2f ", m_binUsage[i][j] / areaPerBin );
	fprintf( out, "\n" );
    }

    fclose( out );
#endif
}

void CPlaceBin::ShowInfo( const double& targetUtil )
{

    printf( "Phase 0: Total %d rows are processed.\n", 
	    int( (m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom) / m_pDB->m_rowHeight) );
    printf( "ImageWindow=(%d %d %d %d) w/ row_height=%d\n", 
	    (int)m_pDB->m_coreRgn.left, (int)m_pDB->m_coreRgn.bottom,
	    (int)m_pDB->m_coreRgn.right, (int)m_pDB->m_coreRgn.top,
	    (int)m_pDB->m_rowHeight );
   
    int vioNum = 0; 
    double totalFreeSpace = 0;
    double totalOver = 0;
    double maxOver = 0;
    for( int i=0; i<m_binNumberW; i++ )
    {
	for( int j=0; j<m_binNumberH; j++ )
	{
	    if( m_binSpace[i][j] == 0 )
		continue;
	    
	    totalFreeSpace += m_binSpace[i][j];
	    
	    double util = m_binUsage[i][j] / m_binSpace[i][j];
	    if( util > targetUtil )
	    {
		totalOver += m_binUsage[i][j] - m_binSpace[i][j]*targetUtil;
		vioNum ++;
	    }
	    if( (util-targetUtil) > maxOver )
		maxOver = (util-targetUtil) ;

	    //if( m_binUsage[i][j] / m_binSpace[i][j] > targetUtil )
	    //	printf( "bin %d %d musage %d free %d\n", 
	    //		i, j, (int)m_binUsage[i][j], (int)m_binSpace[i][j] );
	}
    }

    int totalBinNumber = m_binNumberW*m_binNumberH;
    
    printf( "Total Row Area=%d\n", 
	    (int)((m_pDB->m_coreRgn.top-m_pDB->m_coreRgn.bottom)*(m_pDB->m_coreRgn.right-m_pDB->m_coreRgn.left)) );
    
    printf( "Phase 1: CMAP Dim: %d x %d BinSize: %d x %d Total %d bins.\n",
	    m_binNumberW, m_binNumberH, (int)m_binWidth, (int)m_binHeight,
	    totalBinNumber );
    
    printf( "Total movable area: %d\n", (int)m_totalMovableArea ); 
    
    printf( "Target density: %f\n", targetUtil );
    
    printf( "Violation num:: %d (%f)    Avg overflow: %f  Max overflow: %f\n",
	    vioNum, (double)vioNum/totalBinNumber, 
	    totalOver/vioNum, 
	    maxOver );
    
    printf( "Overflow per bin: %f       Total overflow amount: %f\n",
	    totalOver/totalBinNumber, totalOver );

    double scaled_overflow_per_bin = 
	(totalOver * m_binWidth * m_binHeight * targetUtil) / (m_totalMovableArea * 400 );
	
    printf( "Scaled Overflow per bin: %f\n", scaled_overflow_per_bin*scaled_overflow_per_bin );    
    
    /*
        Phase 0: Total 890 rows are processed.
	ImageWindow=(459 459 11151 11139) w/ row_height=12
	Total Row Area=114190560
	Phase 1: CMAP Dim: 90 x 89 BinSize: 120 x 120 Total 8010 bins.
	NumNodes: 211447 NumTerminals: 543
	Phase 2: Node file processing is done. Total 211447 objects (terminal 543)
	Total 211447 entries in ObjectDB
	Total movable area: 53307432
	Phase 3: Solution PL file processing is done.
	Total 211447 objects (terminal 543)
	Phase 4: Congestion map construction is done.
	Total 211447 objects (terminal 543)
	Phase 5: Congestion map analysis is done.
	Total 8010 (90 x 89) bins. Target density: 0.900000
	Violation num: 77 (0.009613)    Avg overflow: 0.031305  Max overflow: 0.100000
	Overflow per bin: 2.699625      Total overflow amount: 21624.000000
	Scaled Overflow per bin: 0.000173
    */

}


//Brian 2007-05-06

// void CPlaceBin::CreateGridNet( int nGrid )
// {
//     m_binWidth  = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left )   / nGrid;
//     m_binHeight = ( m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom ) / nGrid;
//     m_binNumberH = nGrid;
//     m_binNumberW = nGrid;
//     CatchCongParam();
//     CreateGridNet();    
//     UpdateBinFreeSpaceNet(); 
//     UpdateBinUsageNet();
// }

// void CPlaceBin::CatchCongParam()
// {
//     double totalPitch = 0.0;
//     double levelNum = 0;
//     for( unsigned int i = 2 ; i < m_pDB->m_pLEF->m_metalPitch.size() ; i++ )
//     {
//         levelNum++;
//         totalPitch += m_pDB->m_pLEF->m_metalPitch[i] * m_pDB->m_pLEF->m_lefUnit;
// 	}
//     m_dAvgPitch = totalPitch / levelNum;
//     m_dLevelNum = levelNum / 2;
// }

// void CPlaceBin::CreateGridNet()
// {
//     m_binSpace.resize( m_binNumberW );
//     m_binUsage.resize( m_binNumberW );
//     for( int i=0; i<m_binNumberW; i++ )
//     {
// 	    m_binSpace[i].resize( m_binNumberH );
// 	    m_binUsage[i].resize( m_binNumberH );
//     }
// }

void CPlaceBin::UpdateBinFreeSpaceNet()
{
    // calculate total space in bins
    for( int i=0; i<m_binNumberW; i++ )
    {
	    for( int j=0; j<m_binNumberH; j++ )
	    {
	        double left = GetBinX( i );
	        double right = left + m_binWidth;
	        double bottom = GetBinY( j );
	        double top = bottom + m_binHeight;
	        if( right > m_pDB->m_coreRgn.right )
		        right = m_pDB->m_coreRgn.right;
	        if( top > m_pDB->m_coreRgn.top )
		        top = m_pDB->m_coreRgn.top;
	        m_binSpace[i][j] = (right-left) * (top-bottom);
	    }
    }


    for( unsigned int b=0; b<m_pDB->m_nets.size(); b++ )
    {

	    if( m_pDB->m_nets[b].size() < 2 )
	        continue;

        int pinId = m_pDB->m_nets[b][0];
        double left = m_pDB->m_pins[pinId].absX;
        double right = left;
        double top = m_pDB->m_pins[pinId].absY;
        double bottom = top;
        
        for (unsigned int j = 1; j < m_pDB->m_nets[b].size(); j++)
        {
            pinId = m_pDB->m_nets[b][j];
            left = min( left, m_pDB->m_pins[pinId].absX);
            right = max( right, m_pDB->m_pins[pinId].absX);
            bottom = min( bottom, m_pDB->m_pins[pinId].absY);
            top = max( top, m_pDB->m_pins[pinId].absY);
        }

	    double w = right - left;
	    double h = top - bottom;
        double area = w * h;
        double density = 1.0;
        if (area > 0)
        {
            density = (m_dAvgPitch / m_dLevelNum) * (w + h) / area;
        }
        else
        {
            density = 1.0 / m_dLevelNum;
        }

	    // find nearest gird
	    int binX = GetHorizontalBinNumber( left );
	    int binY = GetVerticalBinNumber( bottom );

	    for( int xOff=binX; xOff<m_binNumberW; xOff++ )
	    {
	        double binX = GetBinX( xOff );
	        if( binX >= right )
		        break;

	        for( int yOff=binY; yOff<m_binNumberH; yOff++ )
	        {
		        double binY = GetBinY( yOff );
		        if( binY >= top )
		            break;

		        double binXright = binX + m_binWidth;
		        double binYtop = binY + m_binHeight;
		        if( binXright > m_pDB->m_coreRgn.right )
		            binXright = m_pDB->m_coreRgn.right;
		        if( binYtop > m_pDB->m_coreRgn.top )
		            binYtop = m_pDB->m_coreRgn.top;

		        double common_area = 
		            getOverlap( left, right, binX, binXright ) *
		            getOverlap( bottom, top, binY, binYtop );
		        m_binSpace[xOff][yOff] -= common_area * density;
	        }

	    } // for each bin

    } // for each net
}

void CPlaceBin::UpdateBinUsageNet()
{
    ClearBinUsage();
    for( unsigned int b=0; b<m_pDB->m_nets.size(); b++ )
    {
	    if( m_pDB->m_nets[b].size() < 2 )
	        continue;

        int pinId = m_pDB->m_nets[b][0];
        double left = m_pDB->m_pins[pinId].absX;
        double right = left;
        double top = m_pDB->m_pins[pinId].absY;
        double bottom = top;
        
        for (unsigned int j = 1; j < m_pDB->m_nets[b].size(); j++)
        {
            pinId = m_pDB->m_nets[b][j];
            left = min( left, m_pDB->m_pins[pinId].absX);
            right = max( right, m_pDB->m_pins[pinId].absX);
            bottom = min( bottom, m_pDB->m_pins[pinId].absY);
            top = max( top, m_pDB->m_pins[pinId].absY);
        }

	    double w = right - left;
	    double h = top - bottom;
        double area = w * h;
        double density = 1.0;
        if (area > 0)
        {
            density = (m_dAvgPitch / m_dLevelNum) * (w + h) / area;
        }
        else
        {
            density = 1.0 / m_dLevelNum;
        }

	    // find nearest gird
	    int binX = GetHorizontalBinNumber( left );
	    int binY = GetVerticalBinNumber( bottom );

	    for( int xOff=binX; xOff<m_binNumberW; xOff++ )
	    {
	        double binX = GetBinX( xOff );
	        if( binX >= right )
		        break;

	        for( int yOff=binY; yOff<m_binNumberH; yOff++ )
	        {
		        double binY = GetBinY( yOff );
		        if( binY >= top )
		            break;

		        double binXright = binX + m_binWidth;
		        double binYtop = binY + m_binHeight;
		        if( binXright > m_pDB->m_coreRgn.right )
		            binXright = m_pDB->m_coreRgn.right;
		        if( binYtop > m_pDB->m_coreRgn.top )
		            binYtop = m_pDB->m_coreRgn.top;

		        double common_area = 
		            getOverlap( left, right, binX, binXright ) *
		            getOverlap( bottom, top, binY, binYtop );

		        m_binUsage[xOff][yOff] += common_area * density;
	        }

	    } // for each bin

    } // for each net
}

void CPlaceBin::OutputCongMap(string filename)
{
    // TODO (donnie) Use CMatrixPlotter
    
    double binArea = m_binWidth * m_binHeight;
    double maxValue = 0;
    
    for (unsigned int i = 0; i < m_binUsage.size(); i++)
    {
    	for (unsigned int j = 0; j < m_binUsage[i].size(); j++)
    	{
    	   m_binUsage[i][j] /= binArea; 
    	   if (m_binUsage[i][j] > maxValue)
    	       maxValue = m_binUsage[i][j];
    	}
    }
    string title = "Net Overlap Congestion Map";
    printf( "Output Net Overlap Congestion Map: %s\n", filename.c_str() );
    printf("Max Bin Usage = %f\n", maxValue);
	
    string pltname = filename + ".plt";
    string datname = "dat/" + pltname + ".dat";
    string pngname = filename + ".png";

    FILE* pPlt = fopen( pltname.c_str(), "w" );

    fprintf( pPlt, "set output \"%s\"\n", pngname.c_str() );
    fprintf( pPlt, "set term png\n" );
    fprintf( pPlt, "set title \" %s \"\n", pltname.c_str() );
    
    fprintf( pPlt, "set pm3d map\n");
    fprintf( pPlt, "set size ratio 1\n" );
    fprintf( pPlt, "set cbrange [0:2]\n" );
    fprintf( pPlt, "set xrange [0:%d]\n", m_binUsage.size());
    fprintf( pPlt, "set yrange [0:%d]\n", m_binUsage[0].size());
    fprintf( pPlt, "set palette model RGB defined ( 0 \"white\", 0.4 \"black\", "
	           "0.4 \"dark-yellow\", 0.8 \"yellow\", 0.8 \"green\", 1.0 \"green\", 1.0 \"blue\", "
		   "1.5 \"dark-blue\", 1.5 \"dark-red\", 2.0 \"dark-red\" )\n" );
    fprintf( pPlt, "splot '%s' matrix with pm3d\n", datname.c_str() );
    
    fclose( pPlt );
    
    FILE* OverflowData = fopen( datname.c_str(), "w" );

    for( unsigned int i = 0 ; i <  m_binUsage[0].size() ; i++ )
    {
    	for( unsigned int j = 0 ; j < m_binUsage.size() ; j++ )
    	{
    	    fprintf( OverflowData, "%6.2f ", m_binUsage[j][i] );		
    	}
    	fprintf( OverflowData, "\n" );
    }

    fclose( OverflowData ); 

    
}

//@Brian 2007-05-06


/////////////////////////////////////////////////////////////
// CGreedyRefinment   
// 2007-07-09
// Donnie
/////////////////////////////////////////////////////////////

const int CGreedyRefinement::m_directionCount = 8;
// directions                                N   E   S   W  NW  NE  SE  SW
const int CGreedyRefinement::m_xMove[8] = {  0,  1,  0, -1, -1,  1, -1,  1 };
const int CGreedyRefinement::m_yMove[8] = {  1,  0, -1,  0,  1,  1, -1, -1 };

CGreedyRefinement::CGreedyRefinement( CPlaceDB* pDB )
:   m_pDB( pDB ),
    m_pBin( NULL )
{
    m_pBin = new CPlaceBin( *m_pDB );
}

CGreedyRefinement::~CGreedyRefinement()
{
    delete m_pBin;
}

void CGreedyRefinement::CreateBlockList()
{
    vector< pair< float, int > > seq;
    seq.resize( m_pDB->m_modules.size() );
    int seqCount = 0;
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	if( i % 50000 == 0 && i != 0 )
	{
	    printf( "%d...\n", i );
	    fflush( stdout );
	}

	if( m_pDB->m_modules[i].m_isFixed )
	    continue;
	if( m_pDB->m_modules[i].m_height > GetBinHeight() )
	    continue;
	
	float priority =
	    - fabs( m_pDB->m_modules[i].m_cy - 0.5 * ( m_pDB->m_coreRgn.top + m_pDB->m_coreRgn.bottom ) )
	    - fabs( m_pDB->m_modules[i].m_cx - 0.5 * ( m_pDB->m_coreRgn.left + m_pDB->m_coreRgn.right ) );
	
	seq[seqCount++] = make_pair( priority, i ); // <float,int>
    }
    seq.resize( seqCount );
    m_blockList.resize( seqCount );
    printf( "Sorting (%d)...", seqCount );
    fflush( stdout );
    sort( seq.begin(), seq.end() );
    vector< pair< float, int> >::iterator ite;
    int count = 0;
    for( ite = seq.begin(); ite != seq.end(); ite++ )
	m_blockList[count++] = ite->second;
    printf( "done\n" );
    fflush( stdout );
}

void CGreedyRefinement::CreateGrid( int gridHNumber, int gridVNumber )
{
    printf( "   Create grids %d x %d\n", gridHNumber, gridVNumber );
    m_pBin->CreateGrid( gridHNumber, gridVNumber );
    CreateBlockList();
    printf( "   Grid size %f x %f ( %.1f row height)\n", GetBinWidth(),
	    GetBinHeight(), GetBinHeight()/m_pDB->m_rowHeight );
    printf( "   (overflow= %g  hpwl= %.0f)\n", 
	    m_pBin->GetTotalOverflowPercent( 1.0 ), 
	    m_pDB->CalcHPWL() );
}

void CGreedyRefinement::RefineBlocks()
{
    int totalCount = 0;
    double hpwl = m_pDB->CalcHPWL();
    while( true )
    {
	//if( totalCount % 20 == 0 && totalCount != 0 )
	    printf( "Ite %d ...", totalCount );
	totalCount++;
	
	int moveCount = 0;
	for( unsigned int i=0; i<m_blockList.size(); i++ )
	{
	    //printf( "%d: (%g)", i, m_pBin->GetTotalOverflowPercent( 1.0 ) );
	    if( RefineBlock( m_blockList[i] ) )
		moveCount++;
	}

	
	
	printf( " %d block moved  (overflow= %g  hpwl= %.0f)\n", 
		moveCount, m_pBin->GetTotalOverflowPercent( 1.0 ), 
		m_pDB->CalcHPWL() );
	
	if( moveCount == 0 )
	    break;

	if( m_pDB->GetHPWL() >= hpwl*0.99  && totalCount > 4 )
	//if( m_pDB->GetHPWL() >= hpwl  )
	    break;
	
	hpwl = m_pDB->GetHPWL();
	
	//if( totalCount > 20 )
	//    break;
    }
}

bool CGreedyRefinement::RefineBlock( int blockId )
{
    int bestDir = -1;
    double bestCost = 0;
    for( int i=0; i<m_directionCount; i++ )
    {
	double cost = GetCostChange( blockId, i );
	if( cost < bestCost )
	{
	    bestCost = cost;
	    bestDir = i;
	}
    }
    if( bestDir > -1 )
    {
	float oldX = m_pDB->m_modules[blockId].m_cx;
	float oldY = m_pDB->m_modules[blockId].m_cy;
	float newX = oldX + m_xMove[bestDir] * GetBinWidth();
	float newY = oldY + m_yMove[bestDir] * GetBinHeight();

	assert( newX > m_pDB->m_coreRgn.left );
	assert( newX < m_pDB->m_coreRgn.right );
	assert( newY > m_pDB->m_coreRgn.bottom );
	assert( newY < m_pDB->m_coreRgn.top );
	
	// update density
	double blockArea = m_pDB->m_modules[blockId].m_area;
	int oldBinX = GetHorizontalBinNumber( blockId );
	int oldBinY = GetVerticalBinNumber( blockId );
	int newBinX = oldBinX + m_xMove[ bestDir ];
	int newBinY = oldBinY + m_yMove[ bestDir ];

#if 0	
	printf( "Move block %d from (%d %d) %.0f to (%d %d) %.0f dir %d, cost %.5f\n", 
		blockId, oldBinX, oldBinY, m_pBin->m_binUsage[oldBinX][oldBinY],
		newBinX, newBinY, m_pBin->m_binUsage[newBinX][newBinY],
		bestDir, bestCost );
#endif

	m_pDB->MoveModuleCenter( blockId, newX, newY );
	m_pBin->m_binUsage[oldBinX][oldBinY] -= blockArea;
	m_pBin->m_binUsage[newBinX][newBinY] += blockArea;
	return true;	
    }
    return false;
}
			
double CGreedyRefinement::GetCostChange( int blockId, int direction )
{
    double alpha = 0.1;
    double beta = 100.0;

    //alpha = 0;

    gArg.GetDouble( "beta", &beta );
    
    assert( direction < m_directionCount );
    
    int oldBinX = GetHorizontalBinNumber( blockId );
    int oldBinY = GetVerticalBinNumber( blockId );
    int newBinX = oldBinX + m_xMove[ direction ];
    int newBinY = oldBinY + m_yMove[ direction ];

    // boundary check (only accept cost < 0 )
    if( newBinY < 0 || newBinX < 0 )
	return 1;
    if( newBinX >= GetBinNumberW() )
	return 1;
    if( newBinY >= GetBinNumberH() )
	return 1;
   
    double c1 = GetHPWLChange( blockId, direction );
    double c2 = GetDensityChange( blockId, oldBinX, oldBinY, newBinX, newBinY );

    if( c2 <= 0 )
    {
	if( c1 > 0 )
	    return c1;
	else
	    return c1 + c2;
    }
    
    return alpha * GetHPWLChange( blockId, direction ) + 
	beta * GetDensityChange( blockId, oldBinX, oldBinY, newBinX, newBinY );
}

double CGreedyRefinement::GetHPWLChange( int blockId, int direction )
{
    // TODO: speed up
    double oldHPWL = 0;
    double newHPWL = 0;
    for( unsigned int i=0; i<m_pDB->m_modules[blockId].m_netsId.size(); i++ )
    {
	int n = m_pDB->m_modules[blockId].m_netsId[i];
	oldHPWL += m_pDB->GetNetLength( m_pDB->m_nets[n] );
    }
    float oldX = m_pDB->m_modules[blockId].m_cx;
    float oldY = m_pDB->m_modules[blockId].m_cy;
    float newX = oldX + m_xMove[direction] * GetBinWidth();
    float newY = oldY + m_yMove[direction] * GetBinHeight();
    m_pDB->MoveModuleCenter( blockId, newX, newY );
    for( unsigned int i=0; i<m_pDB->m_modules[blockId].m_netsId.size(); i++ )
    {
	int n = m_pDB->m_modules[blockId].m_netsId[i];
	newHPWL += m_pDB->GetNetLength( m_pDB->m_nets[n] );
    }
    m_pDB->MoveModuleCenter( blockId, oldX, oldY );
    return newHPWL - oldHPWL; // < 0 is BETTER
}

double CGreedyRefinement::GetDensityChange( int blockId, int oldBinX, int oldBinY, int newBinX, int newBinY )
{
    
    
    double targetDensity = 1.0;
    double blockArea = m_pDB->m_modules[blockId].m_area;

    double oldOver = 
	max( 0, ( m_pBin->m_binUsage[oldBinX][oldBinY] / m_pBin->m_binSpace[oldBinX][oldBinY] 
		- targetDensity ) ) +
	max( 0, ( m_pBin->m_binUsage[newBinX][newBinY] / m_pBin->m_binSpace[newBinX][newBinY] 
		- targetDensity ) );
    double newOver = 
	max( 0, ( (m_pBin->m_binUsage[oldBinX][oldBinY] - blockArea) / m_pBin->m_binSpace[oldBinX][oldBinY] 
		- targetDensity ) ) +
	max( 0, ( (m_pBin->m_binUsage[newBinX][newBinY] + blockArea) / m_pBin->m_binSpace[newBinX][newBinY] 
		- targetDensity ) );
/* 
    double oldOver = m_pBin->m_binUsage[oldBinX][oldBinY] / m_pBin->m_binSpace[oldBinX][oldBinY];
    double newOver = m_pBin->m_binUsage[newBinX][newBinY] / m_pBin->m_binSpace[newBinX][newBinY];
  */  
#if 0 
    if( (newOver - oldOver) < 0 )
    {
	printf( "(%d) cost %.5f old %.0f (%.0f) (%d %d)  new %.0f (%.0f) (%d %d) block %.0f \n", 
		blockId, newOver-oldOver,
		m_pBin->m_binUsage[oldBinX][oldBinY], m_pBin->m_binSpace[oldBinX][oldBinY],
	     oldBinX, oldBinY,
	    m_pBin->m_binUsage[newBinX][newBinY], m_pBin->m_binSpace[newBinX][newBinY],
	    newBinX, newBinY,
	   blockArea );
    }	
#endif
    return newOver - oldOver; // < 0 is BETTER
}

int CGreedyRefinement::GetHorizontalBinNumber( int blockId )
{
    return m_pBin->GetHorizontalBinNumber( m_pDB->m_modules[blockId].m_cx );
}

int CGreedyRefinement::GetVerticalBinNumber( int blockId )
{
    return m_pBin->GetVerticalBinNumber( m_pDB->m_modules[blockId].m_cy );
}




