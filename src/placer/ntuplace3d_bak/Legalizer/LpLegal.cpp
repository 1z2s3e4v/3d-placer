
#include "LpLegal.h"
#include "arghandler.h"

#include <unistd.h>

#include <vector>
#include <algorithm>
#include <climits>
using namespace std;

CLPLegal::CLPLegal( CPlaceDB* pDB )
{
    m_pDB = pDB;
}

void CLPLegal::GenLPProblem( vector<int>& blockList, vector<Bin>& grid, int binStart )
{
    GenLPProblem( blockList, 0, blockList.size(), grid, binStart );
}

void CLPLegal::GenLPProblem( 
	//vector<int>& blockList, 
	vector<int> blockList, 
	unsigned int start, unsigned int end, vector<Bin>& grid, int binStart )
{
    vector<int> blockList2 = blockList;

  
    if( end > blockList.size() )
    {
	printf( "GenLPProblem   [%d:%d] of %d %d grid %d of %d\n",
 start,
 end,
 blockList.size(),
 &blockList,
		binStart,
 grid.size() ); 
    }	
    assert( end <= blockList.size() );
    assert( end > start );
    assert( binStart < grid.size() );
    
    FILE* out = fopen( "/tmp/lplegal.in", "w" );
    if( out == NULL )
    {
	printf( "Failed to create the file\n" );
	exit(0);
    }
    

    blockList = blockList2;

    //printf( "GenLPProblem   [%d:%d] of %d %d grid %d of %d\n", start, end, blockList.size(), &blockList, 
    //	    binStart, grid.size() ); 

    fprintf( out, "Minimize\n obj: " );
    
    int varCount = 0;    
    bool first = true;
    for( unsigned int b=start; b<end; b++ )
    {
	if( b >= blockList.size() )
	{
	    printf( "b = %d of %d\n", b, blockList.size() );
	}
	assert( b < blockList.size() );
	int i = blockList[b];
	assert( i < (int)m_pDB->m_modules.size() );

	if( m_pDB->m_modules[i].m_isFixed )
	    continue;
	if( m_pDB->m_modules[i].m_height > m_pDB->m_rowHeight )
	    continue;

	for( unsigned int g=binStart; g<grid.size(); g++ )
	{
	    varCount++;
	    int distance = (int)round( 
		    fabs( grid[g].x + grid[g].width/2 - m_pDB->m_modules[i].m_x) +
		    fabs( grid[g].y + grid[g].height/3 - m_pDB->m_modules[i].m_y) );
	    if( first )
		first = false;
	    else
		fprintf( out, "+ " );
	    fprintf( out, "%d x%d_%d ", distance, i, g );
	    fflush( out );
	}
    }


    int constraintCount = 0;    
    fprintf( out, "\nSubject To\n" );

    for( unsigned int b=start; b<end; b++ )
    {
	int i = blockList[b];
	bool first = true;
	fprintf( out, " c%d: ", ++constraintCount );
	for( unsigned int g=binStart; g<grid.size(); g++ )
	{
	    if( first )
		first = false;
	    else
		fprintf( out, "+ " );
	    fprintf( out, "x%d_%d ", i, g );
	}
	//fprintf( out, ">= 1\n" );
	fprintf( out, "= 1\n" );
    }

    double totalBlockArea = 0;
    for( unsigned int b=start; b<end; b++ )
    {
	int i = blockList[b];
	totalBlockArea += m_pDB->m_modules[i].m_area;
    }
    double totalFreeSpace = 0; 
    for( unsigned int g=binStart; g<grid.size(); g++ )
	totalFreeSpace += grid[g].area;

    double ratio = 1.0;
    if( totalFreeSpace < totalBlockArea )
    {
	ratio = totalBlockArea / totalFreeSpace * 1.01;
	printf( "Warning, free %.0f   need %.0f\n", totalFreeSpace, totalBlockArea );
    }

    // area constraint  
    for( unsigned int g=binStart; g<grid.size(); g++ )
    {
	fprintf( out, " c%d: ", ++constraintCount );
	bool first = true;

	for( unsigned int b=start; b<end; b++ )
	{
	    int i = blockList[b];
	    if( first )
		first = false;
	    else
		fprintf( out, "+ " );

	    int size = (int)round( m_pDB->m_modules[i].m_area );
	    fprintf( out, "%d x%d_%d ", size, i, g );
	}
	double area = grid[g].area;
	if( area < 0 )
	    fprintf( out, "<= 0\n" );
	else
	    fprintf( out, "<= %.0f\n", area * ratio );
    }
   

    /*
    // Integer variables 
    fprintf( out, "General\n" );
    for( unsigned int b=start; b<end; b++ )
    {
	int i = blockList[b];
	for( unsigned int g=binStart; g<grid.size(); g++ )
	{
	    fprintf( out, " x%d_%d\n", i, g );
	}
    }
   */
    fprintf( out, "End\n" ); 
    fclose( out );
   
    assert( varCount != 0 ); 
    //printf( "Variable # = %d   constraint # = %d\n", varCount, constraintCount );
}

void CLPLegal::LoadResults( char* filename, vector<int>& moduleBin, vector<Bin>& grid, int binStart )
{
    FILE* in = fopen( filename, "r" );

    char * line = NULL;
    size_t len = 0;
			
    int count = 0;
    while( true )
    {
	if( -1 == getline( &line, &len, in ) )
	    break;
	int num;
	int blockId;
	int binId;
	float value;
	sscanf( line, "%d x%d_%d %*s %f", &num, &blockId, &binId, &value );
	
	
	if( value == 1 )
	{
	    count++;
	    //printf( "block %d bin %d\n", blockId, binId );

	    assert( blockId < (int)moduleBin.size() );
	    moduleBin[blockId] = binId;

	    assert( binId < (int)grid.size() );
	    grid[binId].movBlockList.push_back( blockId );

	    grid[binId].area -= m_pDB->m_modules[blockId].m_area;
	    //assert( grid[binId].area >= 0 );
	}
    }
    if (line)
	free(line);
   
    assert( count != 0 ); 
    //printf( "%d blocks updated\n", count );
}


void CLPLegal::LoadResults2( char* filename, vector<int>& moduleBin, vector<Bin>& grid, int binStart )
{
    FILE* in = fopen( filename, "r" );

    char * line = NULL;
    size_t len = 0;
			
    int count = 0;
    while( true )
    {
	if( -1 == getline( &line, &len, in ) )
	    break;
	int num;
	int blockId;
	int binId;
	float value;
	sscanf( line, "%d x%d_%d %*s %f", &num, &blockId, &binId, &value );
	printf( "block %d bin %d   value %f\n", blockId, binId, value );

	assert( blockId < (int)moduleBin.size() );
	moduleBin[blockId] = binId;
	assert( binId < (int)grid.size() );

	if( value > 0.5 )
	{
	    count++;
	    grid[binId].movBlockList.push_back( blockId );
	    grid[binId].area -= m_pDB->m_modules[blockId].m_area;
	    if( grid[binId].area < 0 )
	    {
		printf( "Bin %d left area %.0f\n", binId, grid[binId].area );
	    }
	}
	//assert( grid[binId].area >= 0 );
    }
    if (line)
	free(line);
   
    //printf( "%d blocks updated\n", count );
}



// Use LP to solve Bonn place min-cost flow problem
bool CLPLegal::TopDownLegal()
{    
    printf( "\nLp Legal -- Top Down\n" );
    
    double left   = m_pDB->m_coreRgn.left;
    double right  = m_pDB->m_coreRgn.right;
    double top    = m_pDB->m_coreRgn.top;
    double bottom = m_pDB->m_coreRgn.bottom;
    
    double startX = (left + right) * 0.5;
    double startY = (top + bottom) * 0.5;
 
    // prepare the legalization list
    vector< pair< double, int > > modulePriority;
    modulePriority.resize( m_pDB->m_modules.size() );
    int mCount = 0;
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	if( m_pDB->m_modules[i].m_isFixed )
	    continue;

	double dis = 
	    fabs( m_pDB->m_modules[i].m_cx - startX ) +
	    fabs( m_pDB->m_modules[i].m_cy - startY );
	modulePriority[ mCount++ ] = make_pair( dis, i );
    }
    modulePriority.resize( mCount );
    // from closest to farest
    sort( modulePriority.begin(), modulePriority.end() );

    vector< int > blockList;
    blockList.resize( mCount );
    for( unsigned int i=0; i<modulePriority.size(); i++ )
	blockList[i] = modulePriority[i].second;
 
    // use moduleBin to store results
    vector<int> moduleBin;
    moduleBin.resize( m_pDB->m_modules.size(), -1 );
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
	if( m_pDB->m_modules[i].m_isFixed || m_pDB->m_modules[i].m_height > m_pDB->m_rowHeight*2 )
	    moduleBin[i] = -2;
  
    // top-level
    vector<Bin> grid;
    BinPart( left, bottom, right, top, startX, startY, blockList, grid, moduleBin );
    
    // TODO: sort the blocks
    for( unsigned int i=0; i<84; i++ )
    {
	printf( "\n\nSolve part %d, %d blocks\n", i, grid[i].movBlockList.size() );
	BinPart( grid[i].x, grid[i].y, 
		grid[i].x + grid[i].width, 
		grid[i].y + grid[i].height, startX, startY, 
		grid[i].movBlockList, grid, moduleBin );
    }

    printf( "Total displacement = %f\n", GetTotalDisplacement( grid, moduleBin ) );
    
    return true;
}

double CLPLegal::GetTotalDisplacement( vector<Bin>& bin, vector<int>& moduleBin )
{
    double totalDis = 0;
    for( unsigned int i=0; i<moduleBin.size(); i++ )
    {
	if( moduleBin[i] == -2 )
	    continue;
	if( moduleBin[i] == -1 )
	{
	    printf( "module %d is not assigned\n", i );
	    continue;
	}
	double newX = bin[ moduleBin[i] ].x + 0.5 * bin[ moduleBin[i] ].width;
	double newY = bin[ moduleBin[i] ].y + 0.5 * bin[ moduleBin[i] ].height;
	totalDis += fabs( newX - m_pDB->m_modules[i].m_cx );
	totalDis += fabs( newY - m_pDB->m_modules[i].m_cy );
    }
    return totalDis;
}

void CLPLegal::BinPart( double left, double bottom, double right, double top, 
	double startX, double startY, 
	vector<int> blockList, 
	vector<Bin>& grid,
        vector<int>& moduleBin )
{

    double xRange = right - left;
    double yRange = top - bottom;
    assert( xRange > 0 );
    assert( yRange > 0 );
   
    // try xDim-by-yDim bins
    int xDim = 2;
    int yDim = 2;

    gArg.GetInt( "xDim", &xDim );
    gArg.GetInt( "yDim", &yDim );
    printf( "Grid %d x %d\n", xDim, yDim );
   
    /* 
    vector< double > xCoordinates;
    vector< double > yCoordinates;
    xCoordinates.resize( xDim );
    yCoordinates.resize( yDim );

    // center of bins (TODO) y-coordinates need to align rows
    for( int i=0; i<xDim; i++ )
	xCoordinates[i] = left + xRange / xDim * ( 0.5 + i );
    for( int i=0; i<yDim; i++ )
	yCoordinates[i] = bottom + yRange / yDim * ( 0.5 + i );
    */
    
    //vector<Bin> grid;
    int oldGridNumber = grid.size();
    grid.resize( oldGridNumber + xDim * yDim );
    printf( "grid resize from %d to %d\n", oldGridNumber, grid.size() );
    for( int i=0; i<xDim; i++ )
	for( int j=0; j<yDim; j++ )
	{
	    double width = xRange / xDim;
	    double height = yRange / yDim;
	    int index = oldGridNumber+j*xDim+i;
	    assert( (int)index < grid.size() );
	    grid[index].x = left + width * i;
	    grid[index].y = bottom + height * j;
	    grid[index].width = width;
	    grid[index].height = height;
	    grid[index].area = width * height;

	    // TODO: handle preplaced blocks
	}
  
     
    int probSize = 512;
    gArg.GetInt( "probSize", &probSize );
    printf( "Progressive problem size %d    oldGrid# %d\n", probSize, oldGridNumber ); 

    unsigned int currentIndex = 0;
    
    // progressive loop
    //printf( "\n" );
    while( true )
    {
	if( currentIndex + probSize > blockList.size() )
	   probSize = blockList.size() - currentIndex; 

	printf( "\rindex %d", currentIndex );

	fflush( stdout );
	
	//GenLPProblem( blockList, grid );
	//printf( "GenLPProblem:  %d [%d:%d] \n", blockList.size(), currentIndex, currentIndex+probSize );
	
	vector<int> bL = blockList;

	GenLPProblem( blockList, currentIndex, currentIndex+probSize, grid, oldGridNumber );

	blockList = bL;

	// Solve
	int timeLimit = INT_MAX;
	gArg.GetInt( "tmlim", &timeLimit );
	if( timeLimit < INT_MAX )
	{
	    char command[200];
	    sprintf( command, "/home/donnie/bin/glpsol -m /tmp/lplegal.in --cpxlp -o /tmp/legal.out --tmlim %d > /tmp/legal.log", 
		    timeLimit );
	    system( command );
	}
	else
	    system( "/home/donnie/bin/glpsol -m /tmp/lplegal.in --cpxlp -o /tmp/legal.out > /tmp/legal.log" );
	//system( "/home/donnie/bin/glpsol -m /tmp/lplegal.in --cpxlp -o /tmp/legal.out > /tmp/legal.log" );
	
	system( "grep \"x.*     1 \" /tmp/legal.out > /tmp/legal.out.x" );		// Integer
	system( "grep \"x.*     0\\.......   \" /tmp/legal.out > /tmp/legal.out.x2" );	// non-integer

	// Load results
	LoadResults( "/tmp/legal.out.x", moduleBin, grid, oldGridNumber ); 
	LoadResults2( "/tmp/legal.out.x2", moduleBin, grid, oldGridNumber ); 

	blockList = bL;

	currentIndex += probSize;
	if( currentIndex >= blockList.size() )
	{
	    //printf( "curr %d   of %d\n", currentIndex, blockList.size() );
	    break;  // no more blocks
	}
	if( currentIndex + probSize > blockList.size() )
	   probSize = blockList.size() - currentIndex; 
    }
   
    printf( "\n" );

    // check results

    //return true;
}



/*   // Exact site legalization
bool CLPLegal::Legal()
{

    printf( "\nLP Legal\n" );
    
    // Generate congestion map
    double left   = m_pDB->m_coreRgn.left;
    double right  = m_pDB->m_coreRgn.right;
    double top    = m_pDB->m_coreRgn.top;
    double bottom = m_pDB->m_coreRgn.bottom;

    double startX = (left + right) * 0.5;
    double startY = (top + bottom) * 0.5;
   
    double ratio = 0.02;
    gArg.GetDouble( "legalRange", &ratio );
    printf( "Legal range ratio %g\n", ratio ); 
    double width = (right - left) * ratio;
    double height = (top - bottom) * ratio;
    
    double boundLeft = startX - width;
    double boundRight =  startX + width;
    double boundTop = startY + height;
    double boundBottom = startY - height;
    
    // Select blocks
    vector< int > blockList;
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	if( m_pDB->m_modules[i].m_isFixed )
	    continue;
	
	if( m_pDB->m_modules[i].m_cx > boundLeft &&
	    m_pDB->m_modules[i].m_cx < boundRight &&
	    m_pDB->m_modules[i].m_cy > boundBottom &&
	    m_pDB->m_modules[i].m_cy < boundTop )
	{
	    blockList.push_back( i );    
	}
    }
    printf( "%d blocks selected\n", blockList.size() );
    
    // Generate LP problem
    double boundRatio = 0.05;
    gArg.GetDouble( "legalBound", &boundRatio ); 
    printf( "Legal bound ratio %g\n", boundRatio ); 
    double legalWidth = (right - left) * 0.05;
    double legalHeight = (top - bottom) * 0.05; 
    int rowHeight = int( m_pDB->m_rowHeight );
    int baseY = int( m_pDB->m_coreRgn.bottom );
    int boundLeftInt   = (int)floor( boundLeft - legalWidth );
    int boundRightInt  = (int)ceil( boundRight + legalWidth );
    int boundTopInt    = (int)ceil( boundTop + legalHeight );
    int boundBottomInt = (int)floor( boundBottom - legalHeight );
    boundBottomInt = (boundBottomInt - baseY) / rowHeight * rowHeight + baseY;    // row alignment
    boundTopInt = (boundTopInt - baseY) / rowHeight * rowHeight + baseY;    // row alignment
 
    FILE* out = fopen( "/tmp/lplegal.in", "w" );
    
    vector< vector< vector<int> > > blockShareSite;
    int xSize = boundRightInt - boundLeftInt;
    int ySize = (boundTopInt - boundBottomInt) / rowHeight;
    blockShareSite.resize( xSize );
    for( unsigned int i=0; i<blockShareSite.size(); i++ )
	blockShareSite[i].resize( ySize );
    printf( " %d x %d site\n", xSize, ySize );
    
    // total variable number =
    // (rangeTop-rangeBottom)/rowHeight  *  (rangeRight-rangeLeft)/siteWidth  *  block#
    fprintf( out, "Minimize\n obj: " );
    bool first = true;
    for( unsigned int i=0; i<blockList.size(); i++ )
    {
	int blockId = blockList[i];
    
	int rangeLeft   = (int)floor( m_pDB->m_modules[blockId].m_cx - legalWidth );
	int rangeRight  = (int)ceil(  m_pDB->m_modules[blockId].m_cx + legalWidth );
	int rangeTop    = (int)ceil(  m_pDB->m_modules[blockId].m_cy + legalHeight );
	int rangeBottom = (int)floor( m_pDB->m_modules[blockId].m_cy - legalHeight );
	rangeBottom = (rangeBottom - baseY) / rowHeight * rowHeight + baseY;    // row alignment
	rangeTop    = (rangeTop - baseY ) / rowHeight * rowHeight + baseY;

	for( int x=rangeLeft; x<rangeRight; x++ )
	    for( int y=rangeBottom; y<rangeTop; y+= rowHeight )
	    {
		int distance = (int)round( 
			fabs(x - m_pDB->m_modules[blockId].m_x) + 
			fabs(y - m_pDB->m_modules[blockId].m_y) );
		if( first )
		    first = false;
		else
		    fprintf( out, "+ " );
		int indexX = x - boundLeftInt;
		int indexY = (y-boundBottomInt) / rowHeight;
		assert( indexX >= 0 );
		assert( indexY >= 0 );
		assert( indexX < xSize );
		assert( indexY < ySize );
		blockShareSite[indexX][indexY].push_back( i );
		fprintf( out, "%d x%d_%d_%d ", distance, i, indexX, indexY );
	    }	
    }
    fprintf( out, "\nSubject To\n" );
   
    int constraintCount = 1;
    
    // Equ (5) constraint: one cell can be only placed at one slot
    for( unsigned int i=0; i<blockList.size(); i++ )
    {
	int blockId = blockList[i];
    
	int rangeLeft   = (int)floor( m_pDB->m_modules[blockId].m_cx - legalWidth );
	int rangeRight  = (int)ceil(  m_pDB->m_modules[blockId].m_cx + legalWidth );
	int rangeTop    = (int)ceil(  m_pDB->m_modules[blockId].m_cy + legalHeight );
	int rangeBottom = (int)floor( m_pDB->m_modules[blockId].m_cy - legalHeight );
	rangeTop = (rangeTop - baseY) / rowHeight * rowHeight + baseY;    // row alignment
	rangeBottom = (rangeBottom - baseY) / rowHeight * rowHeight + baseY;    // row alignment

	bool first = true;
	fprintf( out, " c%d: ", constraintCount++ );
	for( int x=rangeLeft; x<rangeRight; x++ )
	    for( int y=rangeBottom; y<rangeTop; y+= rowHeight )
	    {
		if( first )
		    first = false;
		else
		    fprintf( out, "+ " );
		int indexX = x - boundLeftInt;
		int indexY = (y-boundBottomInt) / rowHeight;
		fprintf( out, "x%d_%d_%d ", i, indexX, indexY );
	    }
	fprintf( out, "= 1\n" );
    }		
    
    
    // Equ (6) constraint: cells cannot be overlapped
    for( int i=boundLeftInt; i<boundRightInt; i++ )
	for( int j=boundBottomInt; j<boundTopInt; j+=rowHeight )
	{
	    // for each grid
	    int indexX = i-boundLeftInt;
	    int indexY = (j-boundBottomInt) / rowHeight;
	    if( blockShareSite[indexX][indexY].size() <= 1 ) 
		continue;

	    int first = true;
	    fprintf( out, " c%d: ", constraintCount++ );
	    for( unsigned int m=0; m<blockShareSite[indexX][indexY].size(); m++ )
	    {
		int vectorId = blockShareSite[indexX][indexY][m];
		int blockId = blockList[vectorId];
		int width = (int)m_pDB->m_modules[blockId].m_width;
		int left = max( i-width+1, (int)floor(m_pDB->m_modules[blockId].m_cx - legalWidth) );
		for( ; left<=i; left++ )
		{
		    if( first )
			first = false;
		    else
			fprintf( out, "+ " );
		    int indexX = left - boundLeftInt;
		    int indexY = (j-boundBottomInt) / rowHeight;
		    fprintf( out, "x%d_%d_%d ", vectorId, indexX, indexY );
		} 
	    }	
	    fprintf( out, "<= 1\n" );
	}
  
    // Equ (4). x_{i,j,k} \in {0, 1} 
    fprintf( out, "Binary\n" );
    for( int i=boundLeftInt; i<boundRightInt; i++ )
	for( int j=boundBottomInt; j<boundTopInt; j+=rowHeight )
	{
	    // for each grid
	    int indexX = i-boundLeftInt;
	    int indexY = (j-boundBottomInt) / rowHeight;
	    for( unsigned int m=0; m<blockShareSite[indexX][indexY].size(); m++ )
	    {
		int vectorId = blockShareSite[indexX][indexY][m];
		fprintf( out, " x%d_%d_%d\n", vectorId, indexX, indexY );
	    }	
	}
    fprintf( out, "End\n" ); 
    
    // Solve Lp problem

    // Load LP results
    
    
    return false;
}
*/
