#include "GRouteMap.h"
#include "placeutil.h"
#include "CongMap.h"

#include <iostream>
#include <fstream>
using namespace std;

///////////////////////////////////////////////////////////////////////////

double GRouteMap::m_copperThicknessLUTx[13] = 
    { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5,
      0.6, 0.7, 0.8, 0.9, 1.0, 5.0, 100000.0 };	
double GRouteMap::m_copperThicknessLUTy[13] = 
    { 0.9750, 0.9763, 0.9696, 0.9511, 0.9277, 0.9074,
      0.8845, 0.8590, 0.8350, 0.8000, 0.1000, 0.0500, 0.0000 };
int GRouteMap::m_copperThicknessLUTsize = sizeof( m_copperThicknessLUTx ) / sizeof( double );

double GRouteMap::GetNormalizedCopperThickness( const double& wireDensity )
{
    assert( wireDensity >= 0.0 );

    double* p = lower_bound( 
	    m_copperThicknessLUTx, 
	    m_copperThicknessLUTx + m_copperThicknessLUTsize, 
	    wireDensity );
    p--;

    assert( p != m_copperThicknessLUTx + m_copperThicknessLUTsize );      // out of range
    assert( p != m_copperThicknessLUTx + m_copperThicknessLUTsize - 1 );  // the last element

    // linear interpolation    
    int index = p - m_copperThicknessLUTx;
    if( index < 0 )
	index = 0;
    assert( wireDensity >= m_copperThicknessLUTx[index] );
    assert( wireDensity <= m_copperThicknessLUTx[index+1] );

    return m_copperThicknessLUTy[index] + 
	(m_copperThicknessLUTy[index+1] - m_copperThicknessLUTy[index]) * 
	(wireDensity - m_copperThicknessLUTx[index]) / 
	(m_copperThicknessLUTx[index+1]- m_copperThicknessLUTx[index]);
}

////////////////////////////////////////////////////////////////////////////

double GRouteMap::m_dummyAmountLUTx[13] = 
    { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5,
      0.6, 0.7, 0.8, 0.9, 1.0, 5.0, 100000.0 };	
double GRouteMap::m_dummyAmountLUTy[13] = 
    { 0.3,  0.2,  0.1,  0.05, 0.06, 0.07, 
      0.05, 0.03, 0.01, 0.0, 0.0, 0.0, 0.0 };
int GRouteMap::m_dummyAmountLUTsize = sizeof( m_dummyAmountLUTx ) / sizeof( double );

double GRouteMap::GetDummyAmount( const double& wireDensity )
{
    assert( wireDensity >= 0.0 );

    double* p = lower_bound( 
	    m_dummyAmountLUTx, 
	    m_dummyAmountLUTx + m_dummyAmountLUTsize, 
	    wireDensity );
    p--;

    assert( p != m_dummyAmountLUTx + m_dummyAmountLUTsize );      // out of range
    assert( p != m_dummyAmountLUTx + m_dummyAmountLUTsize - 1 );  // the last element

    // linear interpolation    
    int index = p - m_dummyAmountLUTx;
    if( index < 0 )
	index = 0;
    assert( wireDensity >= m_dummyAmountLUTx[index] );
    assert( wireDensity <= m_dummyAmountLUTx[index+1] );

    return m_dummyAmountLUTy[index] + 
	(m_dummyAmountLUTy[index+1] - m_dummyAmountLUTy[index]) * 
	(wireDensity - m_dummyAmountLUTx[index]) / 
	(m_dummyAmountLUTx[index+1]- m_dummyAmountLUTx[index]);
}

///////////////////////////////////////////////////////////////////////////

// 2007-04-09
double GRouteMap::GetTotalDummyAmount()
{
    double total = 0;
    for( unsigned int i=0; i<m_wireDensity.size(); i++ )
	for( unsigned int j=0; j<m_wireDensity[i].size(); j++ )
	    total += GetDummyAmount( m_wireDensity[i][j] );
    return total;
}


GRouteMap::GRouteMap( CPlaceDB* pDB )
{
    m_totalLength = 0;
    m_pPlaceDB = pDB;
}

void GRouteMap::InitMap( const int& xSize, const int& ySize )
{
    m_hCut.resize( xSize );
    m_vCut.resize( xSize );
    for( int i=0; i<xSize; i++ )
    {
	m_hCut[i].resize( ySize, 0 );
	m_vCut[i].resize( ySize, 0 );
    }
}

bool GRouteMap::LoadBoxRouterRes( const char* input, const char* results )
{
    LoadSettings( input );
    ShowMapInfo();
    LoadResults( results ); 
    ComputeOverflow( true );
    return true;
}

bool GRouteMap::LoadISPD2007Res( const char* input, const char* results )
{
    LoadSettings2007( input );
    ShowMapInfo();
    LoadResults2007( results ); 
    ComputeOverflow( true );
    ComputeWireDensity();
    return true;
}

void GRouteMap::ShowMapInfo()
{
    printf( "   Grid %d %d\n", m_hCut.size(), m_hCut[0].size() );
    printf( "   Vertical capacity %d\n", m_vCap );
    printf( "   Horizontal capacity %d\n", m_hCap );
    printf( "   (%d %d) step (%d %d)\n", m_left, m_bottom, m_xStep, m_yStep );
    printf( "   # Nets %d\n", m_nNets );
}

int GRouteMap::ComputeOverflow( bool show )
{
    int hOver = 0;
    int vOver = 0;
    int overflow = 0;
    for( unsigned int i=0; i<m_hCut.size(); i++ )
	for( unsigned int j=0; j<m_hCut[i].size(); j++ )
	    if( m_hCut[i][j] > m_hCap )
		hOver += (int)round( m_hCut[i][j] - m_hCap );
    for( unsigned int i=0; i<m_vCut.size(); i++ )
	for( unsigned int j=0; j<m_vCut[i].size(); j++ )
	    if( m_vCut[i][j] > m_vCap )
		vOver += (int)round( m_vCut[i][j] - m_vCap );
    overflow = hOver + vOver;
    if( show )
    {
	printf( "   Total net length %d\n", m_totalLength );
	printf( "   Total overflow %d (H cut %d  V cut %d)\n", overflow, hOver, vOver );
    }
    return overflow;
}

void GRouteMap::LoadResults( const char* results )
{
    printf( "Load global routing results: %s\n", results );

    m_totalLength = 0;

    ifstream in( results, ios::in );
    if( !in )
    {
	printf( "   Cannot open the file\n" );
	exit(0);
    }

    char line[500];
    char netName[500];
    int netId, num;
    int netCount = 0;
    while( !in.eof() )
    {
	while( in.peek() == '#' )
	    in.getline( line, 500 );

	if( !in.getline( line, 500 ) )
	    break;
	int k = sscanf( line, "%s %d %d", netName, &netId, &num );
	if( k != 3 )
	    break;

	in.getline( line, 500 );
	if( strcmp( line, "routed" ) != 0 )
	{
	    printf( "Net %s (%d) is not routed!\n", netName, netId );
	    exit(0);
	}
	netCount++;

	// read segments
	while( in.peek() == 'o' )
	{
	    in.getline( line, 500 );
	    int x1, y1, x2, y2, len;
	    sscanf( line, "o (%d,%d)-(%d,%d) %d", &x1, &y1, &x2, &y2, &len );
	    AddEdge( x1, y1, x2, y2 ); 
	}

	// skip "p"
	while( in.peek() == 'p' )
	    in.getline( line, 500 );

    }
    printf( "   %d nets read\n", netCount );
}


void GRouteMap::LoadSettings( const char* input )
{
    printf( "Load global routing settings: %s\n", input );

    FILE* file = fopen( input, "r" );

    if( !file )
    {
	printf( "  Cannot open the file\n" );
	exit(0);
    }

    int xGrid, yGrid;
    fscanf( file, "grid %d %d\n", &xGrid, &yGrid );
    fscanf( file, "vertical capacity %d\n", &m_hCap );	    // vertical routing --> horizontal cut
    fscanf( file, "horizontal capacity %d\n", &m_vCap );    // horizontal routing --> vertical cut
    fscanf( file, "num net %d\n", &m_nNets );
    fclose( file );

    InitMap( xGrid, yGrid );
}

void GRouteMap::LoadResults2007( const char* results )
{
    printf( "Load global routing results (2007): %s\n", results );

    m_totalLength = 0;

    ifstream in( results, ios::in );
    if( !in )
    {
	printf( "   Cannot open the file\n" );
	exit(0);
    }

    char line[500];
    char netName[500];
    int netId, num;
    int netCount = 0;
    while( !in.eof() )
    {
	while( in.peek() == '#' )
	    in.getline( line, 500 );

	if( !in.getline( line, 500 ) )
	    break;
	int k = sscanf( line, "%s %d %d", netName, &netId, &num );
	if( k != 3 )
	    break;

	netCount++;

	// read segments
	for( int i=0; i<num; i++ )
	{
	    in.getline( line, 500 );
	    int x1, y1, x2, y2, tmp1, tmp2;
	    sscanf( line, "(%d,%d,%d)-(%d,%d,%d)", &x1, &y1, &tmp1, &x2, &y2, &tmp2 );
	    AddEdge2007( x1, y1, x2, y2 ); 
	}

	in.getline( line, 500 );
    }
    printf( "   %d nets read\n", netCount );
}



void GRouteMap::LoadSettings2007( const char* input )
{
    printf( "Load global routing settings (2007): %s\n", input );

    FILE* file = fopen( input, "r" );

    if( !file )
    {
	printf( "  Cannot open the file\n" );
	exit(0);
    }

    int xGrid, yGrid, tmp1, tmp2/*, tmp3, tmp4*/;
    fscanf( file, "grid %d %d 2\n", &xGrid, &yGrid );
    fscanf( file, "vertical capacity 0 %d\n", &m_hCap );	    // vertical routing --> horizontal cut
    fscanf( file, "horizontal capacity %d 0\n", &m_vCap );    // horizontal routing --> vertical cut
    fscanf( file, "minimum width %d %d\n", &tmp1, &tmp2 );
    fscanf( file, "minimum spacing %d %d\n", &tmp1, &tmp2 );
    fscanf( file, "via spacing %d %d\n", &tmp1, &tmp2 );
    fscanf( file, "%d %d %d %d\n", &m_left, &m_bottom, &m_xStep, &m_yStep ); 
    fscanf( file, "num net %d\n", &m_nNets );
    
    InitMap( xGrid, yGrid );

    for( int i=0; i<m_nNets; i++ )
    {
	char name[200];
	char* tmp;
	int t1, t2, t3;
	fscanf( file, "%s %d %d %d\n", name, &t1, &t2, &t3 );
	for( int j=0; j<t2; j++ )
	{
	    //fscanf( file, "%d %d\n", &t1, &t3 );
	    size_t len = 0;
	    getline( &tmp, &len, file );
	    free( tmp );
	}
    }

    int adjCount;
    fscanf( file, "%d\n", &adjCount );
    assert( adjCount >=  0 );
    printf( "cap adj %d\n", adjCount );
   
    int x1, y1, x2, y2, cap, t1, t2;
    for( int i=0; i<adjCount; i++ )
    {
	fscanf( file, "%d %d %d %d %d %d %d\n", &x1, &y1, &t1, &x2, &y2, &t2, &cap );
	assert( x1 < xGrid );
	assert( x2 < xGrid );
	assert( y1 < yGrid );
	assert( y2 < yGrid );
	if( x1 == x2 )
	{
	    int used = m_hCap - cap;
	    assert( used >= 0 );
	    m_hCut[x1][y1] += used;
	}
	else if( y1 == y2 )
	{
	    int used = m_vCap - cap;
	    assert( used >= 0 );
	    m_vCut[x1][y1] += used;
	}
	else
	{
	    printf( "Error %d %d %d %d\n", x1, y1, x2, y2 );
	    exit(0);
	}

    }
    fclose( file );
 
    m_vLayers = 3;
    m_hLayers = 3; 

    // In the ISPD cases, I removed 50% of M1M2.
    
    m_hCap--;
    m_vCap--;

    m_hCap = m_hCap * 300 / 250;		// m_hCap is 250%
    m_vCap = m_vCap * 300 / 250;		// m_vCap is 250%

    printf( " original cap hCap %d  vCap %d\n", m_hCap, m_vCap );    
    
    double m1m2BlockRatio = 1.0;
    double macroBlockRatio = 0.5;
    gArg.GetDouble( "m1m2BlockRatio", &m1m2BlockRatio );
    gArg.GetDouble( "macroBlockRatio", &macroBlockRatio );

    // Metal density is added into GRC. 
    // GRC capacity is a fixed value.
    AddM1M2MetalDensity( m1m2BlockRatio );	    // M1 M2
    //AddMacroMetalDensity( macroBlockRatio );	    // M3 M4   in "adjCount"

}



// Add a horizontal/vertical routing edge
void GRouteMap::AddEdge( const int& x1, const int& y1, const int& x2, const int& y2 )
{
    if( (x1 - x2) != 0 && (y1 - y2) != 0 )
    {
	printf( "Diagonal routing is not allowed (%d, %d) - (%d, %d)\n", x1, y1, x2, y2 );
	return;
    }	
    
    if( x1 - x2 == 0 )
    {
	// vertical routing
	int start = y1;
	int end = y2;
	if( start > end )
	    swap( start, end );
	for( int y=start; y<end; y++ )
	    m_hCut[x1][y]++;
	m_totalLength += end - start;
	return;
    }

    // horizontal routing
    int start = x1;
    int end = x2;
    if( start > end )
	swap( start, end );
    for( int x=start; x<end; x++ )
	m_vCut[x][y1]++;
    m_totalLength += end - start;
    return;
}

// Add a horizontal/vertical routing edge
void GRouteMap::AddEdge2007( const int& xx1, const int& yy1, const int& xx2, const int& yy2 )
{
    int x1 = (xx1 - m_left)   / m_xStep;
    int y1 = (yy1 - m_bottom) / m_yStep;
    int x2 = (xx2 - m_left)   / m_xStep;
    int y2 = (yy2 - m_bottom) / m_yStep;
    
    if( (x1 - x2) != 0 && (y1 - y2) != 0 )
    {
	printf( "Diagonal routing is not allowed (%d, %d) - (%d, %d)\n", x1, y1, x2, y2 );
	return;
    }	
    
    if( x1 - x2 == 0 )
    {
	// vertical routing
	int start = y1;
	int end = y2;
	if( start > end )
	    swap( start, end );
	for( int y=start; y<end; y++ )
	    m_hCut[x1][y]++;
	m_totalLength += end - start;
	return;
    }

    // horizontal routing
    int start = x1;
    int end = x2;
    if( start > end )
	swap( start, end );
    for( int x=start; x<end; x++ )
	m_vCut[x][y1]++;
    m_totalLength += end - start;
    return;
}

void GRouteMap::OutputGnuplotFigure( const char* filename )
{
    //printf( "Generate figures: %s\n", filename );
    string file1 = string(filename) + ".h";
    string file2 = string(filename) + ".v";
    char str1 [255];
    char str2 [255];
    sprintf( str1, "H Cut Capacity = %d", m_hCap );
    sprintf( str2, "V Cut Capacity = %d", m_vCap );

    CMatrixPlotter::OutputGnuplotFigure( m_hCut, file1.c_str(), str1, m_hCap, true, 0.0 );
    CMatrixPlotter::OutputGnuplotFigure( m_vCut, file2.c_str(), str2, m_vCap, true, 0.0 );

    // test
    if( m_vCap > 0 && m_hCap > 0 )
    {
    
	string wireDensityFilename = string( filename ) + ".wire";
	string cuThicknessFilename = string( filename ) + ".cu";
	CMatrixPlotter::OutputGnuplotFigure( 
		m_wireDensity, 
		wireDensityFilename.c_str(), "Metal Density", 1.0, false, 0.0 );
	CMatrixPlotter::OutputGnuplotFigure( 
		m_copperThickness, 
		cuThicknessFilename.c_str(), ShowCopperThicknessStatistics().c_str(), 1.0, false, 0.8 );
    }
}

// Compute the wire density according to the h/v cut capacity.
void GRouteMap::ComputeWireDensity()
{
    if( m_vCap <= 0 && m_hCap <= 0 )
	return;

    m_wireDensity.resize( m_hCut.size() );
    for( unsigned int i=0; i<m_wireDensity.size(); i++ )
    {
	m_wireDensity[i].resize( m_hCut[i].size(), 0 );
	for( unsigned int j=0; j<m_wireDensity[i].size(); j++ )
	{
	    double leftCut;
	    double rightCut = m_vCut[i][j];
	    if( i == 0 )
		leftCut = 0;
	    else
		leftCut = m_vCut[i-1][j];

	    double bottomCut;
	    double topCut     = m_hCut[i][j];
	    if( j == 0 )
		bottomCut = 0;
	    else
		bottomCut = m_hCut[i][j-1];

	    m_wireDensity[i][j] = 
		( leftCut / m_vCap + rightCut / m_vCap + 
		  topCut / m_hCap + bottomCut / m_hCap ) / 4;

	    // Assume wire pitch = wire spacing. 
	    // Thus, the maximum wire density is 0.5.
	    m_wireDensity[i][j] *= 0.5;
	}
    }

    // Copper Thickness
    m_copperThickness = m_wireDensity;
    for( unsigned int i=0; i<m_copperThickness.size(); i++ )
	for( unsigned int j=0; j<m_copperThickness[i].size(); j++ )
	    m_copperThickness[i][j] = GetNormalizedCopperThickness( m_copperThickness[i][j] );

   //ShowCopperThicknessStatistics(); 

#if 0
    // Copper Profile
    vector< int > copperProfile;
    copperProfile.resize( 1001, 0 );
    for( unsigned int i=0; i<m_copperThickness.size(); i++ )
	for( unsigned int j=0; j<m_copperThickness[i].size(); j++ )
	    copperProfile[ (int)round( 1000 * m_copperThickness[i][j] ) ]++;
    FILE* profile = fopen( "/tmp/cuProfile.plt.dat", "w" );
    for( unsigned int i=0; i<copperProfile.size(); i++ )
	fprintf( profile, "%d ", copperProfile[i] );
    fclose( profile );
#endif
}

string GRouteMap::ShowCopperThicknessStatistics()
{
    double maxThickness = 0;
    double minThickness = 100;
    double maxGradient = 0;
    double totalThickness = 0;
    double avgThickness;
    double stdThickness = 0;

    for( unsigned int i=0; i<m_copperThickness.size(); i++ )
	for( unsigned int j=0; j<m_copperThickness[i].size(); j++ )
	{
	    maxThickness = max( maxThickness, m_copperThickness[i][j] );
	    minThickness = min( minThickness, m_copperThickness[i][j] );
	    totalThickness += m_copperThickness[i][j];
	}
    avgThickness = totalThickness / m_copperThickness.size() / m_copperThickness.size();
    
    for( unsigned int i=0; i<m_copperThickness.size()-1; i++ )
	for( unsigned int j=0; j<m_copperThickness[i].size()-1; j++ )
	{
	    stdThickness += (m_copperThickness[i][j] - avgThickness) * (m_copperThickness[i][j] - avgThickness);
	}
    stdThickness /= m_copperThickness.size() / m_copperThickness.size();
    stdThickness = sqrt( stdThickness );

    for( unsigned int i=0; i<m_copperThickness.size()-1; i++ )
	for( unsigned int j=0; j<m_copperThickness[i].size()-1; j++ )
	{
	    maxGradient = max( maxGradient, fabs( m_copperThickness[i][j] - m_copperThickness[i+1][j] ) );
	    maxGradient = max( maxGradient, fabs( m_copperThickness[i][j] - m_copperThickness[i][j+1] ) );
	}

    // 2007-04-09 (donnie) add GetTotalDummyAmount()
    char msg[1000];
    sprintf(  msg, "max %.2f  min %.2f  avg %.2f  std %.2f  maxGrad %.2f  dummy %g  HPWL %g",
	 maxThickness, minThickness, avgThickness,
	 stdThickness, maxGradient,
	 GetTotalDummyAmount(),
	 m_pPlaceDB->CalcHPWL() );
    printf( "[CopperThickness] %s\n", msg );
    return string( msg );
}

void GRouteMap::AddM1M2MetalDensity( const double& ratio )
{
    // Ratio = wire density of the total tracks in M1/M2
    // Ratio may is larger than 1.0 if fat wires exist.

    for( unsigned int i=0; i<m_hCut.size(); i++ )
	for( unsigned int j=0; j<m_hCut[i].size(); j++ )
	{
	    m_hCut[i][j] += m_hCap / m_vLayers * ratio;
	    m_vCut[i][j] += m_vCap / m_hLayers * ratio;
	}
}

void GRouteMap::RemoveM1M2WireCapacity( const double& ratio )
{
    // Ratio = wire density of the total tracks in M1/M2
    // Ratio may is larger than 1.0 if fat wires exist.

    for( unsigned int i=0; i<m_hCapArray.size(); i++ )
	for( unsigned int j=0; j<m_hCapArray[i].size(); j++ )
	{
	    m_hCapArray[i][j] -= m_hCap / m_vLayers * ratio;
	    m_vCapArray[i][j] -= m_vCap / m_hLayers * ratio;
	}
}

void GRouteMap::AddMacroMetalDensity( const double& ratio )
{
    for( unsigned int i=0; i<m_pPlaceDB->m_modules.size(); i++ )
    {
	if( m_pPlaceDB->m_modules[i].m_height == m_pPlaceDB->m_rowHeight )
	    continue;  // std-cell

	CPoint p1, p2;
	p1.x = m_pPlaceDB->m_modules[i].m_x;
	p1.y = m_pPlaceDB->m_modules[i].m_y;
	p2.x = p1.x + m_pPlaceDB->m_modules[i].m_width;
	p2.y = p1.y + m_pPlaceDB->m_modules[i].m_height;

	AddMacroMetalDensity( p1, p2, ratio );
    }
}

void GRouteMap::AddMacroMetalDensity( const CPoint& p1, const CPoint& p2, const double& ratio )
{
    int binX1 = GetBinX( p1.x );
    int binY1 = GetBinY( p1.y );
    int binX2 = GetBinX( p2.x );
    int binY2 = GetBinY( p2.y );

    binX1 = max( 0, binX1 );
    binY1 = max( 0, binY1 );
    binX2 = min( (int)m_hCut.size()-1, binX2 );
    binY2 = min( (int)m_hCut[0].size()-1, binY2 );

    if( binX1 == binX2 && binY1 == binY2 )
    {
	// TODO: Handle small macros
	return;
    }

    // TODO: Handle boundary conditions (Macro does not occupy the whole bin.)
    for( int x=binX1; x<=binX2; x++ )
	for( int y=binY1; y<binY2; y++ )
	{
	    m_hCut[x][y] += m_hCap / m_vLayers * ratio;
	    m_vCut[x][y] += m_vCap / m_hLayers * ratio;
	}
}

// 2007-03-27 (donnie)  
void GRouteMap::LoadCongMap( CCongMap* pCongMap )
{
    int xGrid, yGrid;
    xGrid = pCongMap->GetHDim(); 
    yGrid = pCongMap->GetVDim();
    m_hCap = (int)round( pCongMap->m_num_wire_per_hcut );   // TODO: Prevent rouding
    m_vCap = (int)round( pCongMap->m_num_wire_per_vcut );   // TODO: Prevent rouding
    m_nNets = 0;    // No net number since we impport CongMap results
    InitMap( xGrid, yGrid );

    // TODO: Load the correct data from m_pPlaceDB->pLEF->m_metalDir[]
    //                                  m_pPlaceDB->pLEF->m_metalPitch[]
    m_hWirePitch = 1.0;
    m_vWirePitch = 1.0;
    m_vLayers = 3;
    m_hLayers = 3;
    
    // 2007-03-28
    m_region = m_pPlaceDB->m_coreRgn;
    m_binWidth = pCongMap->GetHLength();
    m_binHeight = pCongMap->GetVLength();

    if( m_hCap <= 0 && m_vCap <= 0 )
    {
	// Default wire pitch is 1.0
	m_hWirePitch = 1.0;
	m_vWirePitch = 1.0;
	m_vLayers = 3;
	m_hLayers = 3;
	if( m_hCap == 0 && m_vCap == 0 )
	{
	    m_hCap = (int)round( m_binWidth / m_vWirePitch * m_vLayers );
	    m_vCap = (int)round( m_binHeight / m_hWirePitch * m_hLayers );
	}
    }

    
    //ShowMapInfo();

    // Horizontal cut
    for( unsigned int i=0; i<m_hCut.size(); i++ )
	for( unsigned int j=0; j<m_hCut[i].size(); j++ )
	    m_hCut[i][j] = pCongMap->GetCut( i, j, CCongMap::Top ).GetCong();

    // Vertical cut
    for( unsigned int i=0; i<m_vCut.size(); i++ )
	for( unsigned int j=0; j<m_vCut[i].size(); j++ )
	    m_vCut[i][j] = pCongMap->GetCut( i, j, CCongMap::Right ).GetCong();

    double m1m2BlockRatio = 1.0;
    double macroBlockRatio = 0.5;
    gArg.GetDouble( "m1m2BlockRatio", &m1m2BlockRatio );
    gArg.GetDouble( "macroBlockRatio", &macroBlockRatio );

    // Metal density is added into GRC. 
    // GRC capacity is a fixed value.
    AddM1M2MetalDensity( m1m2BlockRatio );	    // M1 M2
    AddMacroMetalDensity( macroBlockRatio );	    // M3 M4

    //ComputeOverflow();    // return current overflow value
    ComputeWireDensity();
}

// 2007-03-27 (donnie)
void GRouteMap::SetHVCapacity( const int& hCutCap, const int& vCutCap )
{
    m_hCap = hCutCap;
    m_vCap = vCutCap;
}

int GRouteMap::GetBinX( const double& x )
{
    return (int)floor( ( x - m_region.left) / m_binWidth );
}

int GRouteMap::GetBinY( const double& y )
{
    return (int)floor( (y - m_region.bottom) / m_binHeight );
}

double GRouteMap::GetBinWireDensity( const int& i, const int& j )
{
    assert( i < (int)m_wireDensity.size() );
    assert( j < (int)m_wireDensity[i].size() );
    return m_wireDensity[i][j];
}


void GRouteMap::ComputeAllCapacity()
{
    m_hCapArray.resize( m_wireDensity.size() );
    m_vCapArray.resize( m_wireDensity.size() );
    for( unsigned int i=0; i<m_wireDensity.size(); i++ )
    {
	m_hCapArray[i].resize( m_wireDensity[i].size(), m_hCap );
	m_vCapArray[i].resize( m_wireDensity[i].size(), m_vCap );
    }

    double m1m2BlockRatio = 1.0;
    double macroBlockRatio = 0.5;
    gArg.GetDouble( "m1m2BlockRatio", &m1m2BlockRatio );
    gArg.GetDouble( "macroBlockRatio", &macroBlockRatio );
    printf( "[INFO] M1/M2 blockage ratio = %g\n", m1m2BlockRatio );
    printf( "[INFO] Macro blockage ratio = %g\n", macroBlockRatio );
    RemoveM1M2WireCapacity( m1m2BlockRatio );	    // M1 M2
    RemoveMacroWireCapacity( macroBlockRatio );
}

void GRouteMap::RemoveMacroWireCapacity( const double& ratio )
{
    for( unsigned int i=0; i<m_pPlaceDB->m_modules.size(); i++ )
    {
	if( m_pPlaceDB->m_modules[i].m_height == m_pPlaceDB->m_rowHeight )
	    continue;  // std-cell

	CPoint p1, p2;
	p1.x = m_pPlaceDB->m_modules[i].m_x;
	p1.y = m_pPlaceDB->m_modules[i].m_y;
	p2.x = p1.x + m_pPlaceDB->m_modules[i].m_width;
	p2.y = p1.y + m_pPlaceDB->m_modules[i].m_height;

	RemoveMacroWireCapacity( p1, p2, ratio );
    }
}

void GRouteMap::RemoveMacroWireCapacity( const CPoint& p1, const CPoint& p2, const double& ratio )
{
    int binX1 = GetBinX( p1.x );
    int binY1 = GetBinY( p1.y );
    int binX2 = GetBinX( p2.x );
    int binY2 = GetBinY( p2.y );

    binX1 = max( 0, binX1 );
    binY1 = max( 0, binY1 );
    binX2 = min( (int)m_hCut.size()-1, binX2 );
    binY2 = min( (int)m_hCut[0].size()-1, binY2 );

    if( binX1 == binX2 && binY1 == binY2 )
    {
	// TODO: Handle small macros
	return;
    }

    // TODO: Handle boundary conditions (Macro does not occupy the whole bin.)
    for( int x=binX1; x<=binX2; x++ )
	for( int y=binY1; y<binY2; y++ )
	{
	    m_hCapArray[x][y] -= m_hCap / m_vLayers * ratio;
	    m_vCapArray[x][y] -= m_vCap / m_hLayers * ratio;
	}
}

double GRouteMap::GetHCap( const int &i, const int &j )
{
    assert( i < (int)m_hCapArray.size() );
    assert( j < (int)m_hCapArray[i].size() );
    return m_hCapArray[i][j];
}

double GRouteMap::GetVCap( const int &i, const int &j )
{
    assert( i < (int)m_vCapArray.size() );
    assert( j < (int)m_vCapArray[i].size() );
    return m_vCapArray[i][j];
}
