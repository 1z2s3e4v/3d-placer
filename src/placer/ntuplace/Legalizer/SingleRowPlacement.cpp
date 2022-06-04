#include "arghandler.h"
#include "SingleRowPlacement.h"
#include "placedb.h"

#include <algorithm>
#include <climits>
using namespace std;

bool msg = false;


CSingleRowPlacementLegal::CSingleRowPlacementLegal( CPlaceDB *pDB )
: m_pDB( pDB )
{
}

CSingleRowPlacementLegal::~CSingleRowPlacementLegal()
{
}

void CSingleRowPlacementLegal::ImportRows()
{
    // TODO Site step is not considered. Assume step = 1.
    int rowId = 0;
    for( unsigned int i=0; i<m_pDB->m_sites.size(); i++ )
    {
	int y = (int)m_pDB->m_sites[i].m_bottom;
	for( unsigned int j=0; j<m_pDB->m_sites[i].m_interval.size(); j+= 2 )
	{
	    if( m_pDB->m_sites[i].m_interval[j] == m_pDB->m_sites[i].m_interval[j+1] )
		continue;
	   
	    unsigned int div = 1;
	    
	    int len = (int)(m_pDB->m_sites[i].m_interval[j+1] - m_pDB->m_sites[i].m_interval[j]);
	    for( unsigned int d=0; d<div; d++ )
	    {
		int start =  (int)m_pDB->m_sites[i].m_interval[j] + int(len * d / div);
		int end   =  (int)m_pDB->m_sites[i].m_interval[j] + int(len * (d+1) / div);
		m_rows.push_back( 
			CSingleRowPlacement( 
			    rowId++, 
			    start,
			    end,
			    y ) );

	    }
	   
	   /* 
	    int center = int( (m_pDB->m_sites[i].m_interval[j] + m_pDB->m_sites[i].m_interval[j+1]) *0.5);
	    m_rows.push_back( 
		    CSingleRowPlacement( 
			rowId++, 
			(int)m_pDB->m_sites[i].m_interval[j], 
			center, 
			y ) );
	    m_rows.push_back( 
		    CSingleRowPlacement( 
			rowId++, 
			center, 
			(int)m_pDB->m_sites[i].m_interval[j+1], 
			y ) );
*/
	}
    }
}

bool CSingleRowPlacementLegal::ImportCells( vector<CPoint> *pOriginalPositions )
{
    //double cX = ( m_pDB->m_coreRgn.left + m_pDB->m_coreRgn.right ) * 0.5; 
    //double cY = ( m_pDB->m_coreRgn.top  + m_pDB->m_coreRgn.bottom ) * 0.5;
    
    m_cellIdMap.resize( m_pDB->m_modules.size() + m_rows.size() * 2, -1 );
    m_sequence.resize( m_pDB->m_modules.size() );
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
	if( m_pDB->m_modules[i].m_height > m_pDB->m_rowHeight )
	{
	    fprintf( stderr, "Warning: Legalizer currently does not support macros\n" );
	    return false;
	}

	int x = (int)m_pDB->m_modules[i].m_cx;
	if( pOriginalPositions != NULL )
	    x = (int)(*pOriginalPositions)[i].x;
	
	int id = CSingleRowPlacement::CreateCell( 
		x,  // center
		(int)m_pDB->m_modules[i].m_y,	// bottom
		(int)m_pDB->m_modules[i].m_width );

	/*float priority = 
	    fabs( m_pDB->m_modules[i].m_cx - cX ) +
	    fabs( m_pDB->m_modules[i].m_cy - cY );
	*/

	/*
	float priority = 
	    fabs( m_pDB->m_modules[i].m_cx - m_pDB->m_coreRgn.left ) +
	    fabs( m_pDB->m_modules[i].m_cy - m_pDB->m_coreRgn.bottom );
	*/
	
	/*float priority = 
	    fabs( m_pDB->m_modules[i].m_cx - m_pDB->m_coreRgn.left );
	*/
	/*float priority = 
	    fabs( m_pDB->m_modules[i].m_cx - m_pDB->m_coreRgn.right ) +
	    fabs( m_pDB->m_modules[i].m_cy - m_pDB->m_coreRgn.top );
	*/
	
	float priority = 
	    fabs( m_pDB->m_modules[i].m_cx - 0.5*(m_pDB->m_coreRgn.right+m_pDB->m_coreRgn.left) ) +
	    fabs( m_pDB->m_modules[i].m_cy - 0.5*(m_pDB->m_coreRgn.top+m_pDB->m_coreRgn.bottom) );
	priority *= -1;
	
	//float priority =
	//    fabs( m_pDB->m_modules[i].m_cy - m_pDB->m_coreRgn.top );
	
	//m_cellIdMap.insert( make_pair<int,int>( id, i ) );
	m_cellIdMap[id] = i;
	
	//m_sequence.push_back( make_pair<float,int>( priority, id ) );
	m_sequence[seqCount++] = make_pair( priority, id ); // <float,int>

	int tolerent = int( m_pDB->m_rowHeight * 2.5 );
	for( unsigned int row=0; row<m_rows.size(); row++ )
	{
	    if( abs( m_rows[row].m_y - m_rows[row].m_cells[id].preferY ) > tolerent )
		continue;
	    if( m_rows[row].m_leftBoundary - m_rows[row].m_cells[id].preferCenter > tolerent*5 )
		continue;
	    if( m_rows[row].m_cells[id].preferCenter - m_rows[row].m_rightBoundary > tolerent*5 )
		continue;
	    m_rows[row].m_cells[id].candidateRows.push_back( row );
	}


    }
    
    m_cellIdMap.resize( seqCount + m_rows.size() * 2 );
    m_sequence.resize( seqCount );
    
    //printf( "Sorting..." );
    //fflush( stdout );
    sort( m_sequence.begin(), m_sequence.end() );
    //printf( "done\n" );
    //fflush( stdout );
    
    return true;
}

bool CSingleRowPlacementLegal::PlaceCells()
{
    //map< float, int >::iterator ite;
    vector< pair< float, int > >::iterator ite;
    for( ite=m_sequence.begin(); ite!=m_sequence.end(); ite++ )
    {
	if( false == PlaceCell( ite->second ) )
	    return false;
    }
  
    int replaceTimes = 0;
    gArg.GetInt( "replace", &replaceTimes );
    
    for( int i=0; i<replaceTimes; i++ )
    for( ite=m_sequence.begin(); ite!=m_sequence.end(); ite++ )
    	ReplaceCell( ite->second );
    
   /* 
    for( ite=m_sequence.begin(); ite!=m_sequence.end(); ite++ )
	ReplaceCell( ite->second );
    
    for( ite=m_sequence.begin(); ite!=m_sequence.end(); ite++ )
	ReplaceCell( ite->second );

    for( ite=m_sequence.begin(); ite!=m_sequence.end(); ite++ )
	ReplaceCell( ite->second );
*/
    return true;
}

bool CSingleRowPlacementLegal::EstPlaceCellCost( int id, int rowId, int* cost )
{
    //printf( "R%d place %d\n", rowId, id );
    if( !m_rows[rowId].PlaceCell( id ) )
	return false;
    //printf( "R%d get cost %d\n", rowId, id );
    *cost = m_rows[rowId].GetCostChange();
    //printf( "R%d remove %d\n", rowId, id );
    m_rows[rowId].RemoveCell( id );
    //printf( "R%d get cost %d\n", rowId, id );
    int c = m_rows[rowId].GetCostChange();
    assert( *cost + c == 0 );
    return true;
}

bool CSingleRowPlacementLegal::PlaceCell( int id )
{
    
    int minCost = INT_MAX;
    int minCostRow = -1;

    //for( unsigned int row=0; row<m_rows[0].m_cells[id].candidateRows.size(); row++ )
    //{
    //	int i = m_rows[0].m_cells[id].candidateRows[row];

    int tolerent = int( m_pDB->m_rowHeight * 2.5 );
    
    // test
    //int tolerent = INT_MAX;
    
    int max = int( 
	    m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left + 
	    m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom );
    while( tolerent < max*2 )
    {
	for( unsigned int row=0; row<m_rows.size(); row++ )
	{
	    if( abs( m_rows[row].m_y - m_rows[row].m_cells[id].preferY ) > tolerent )
		continue;
	    if( m_rows[row].m_leftBoundary - m_rows[row].m_cells[id].preferCenter > tolerent*3 )
		continue;
	    if( m_rows[row].m_cells[id].preferCenter - m_rows[row].m_rightBoundary > tolerent*3 )
		continue;

	    int cost;
	    if( !EstPlaceCellCost( id, row, &cost ) )
		continue;

	    if( cost < minCost )
	    {
		minCost = cost;
		minCostRow = row;
	    }
	}

	if( minCostRow != -1 )
	    break;
	tolerent *= 2;
	//printf( "X" );
	//fflush( stdout );
    }

#if 0
    if( minCostRow == -1 )
    {
	//printf ("F" );
	fflush( stdout );
	for( unsigned int i=0; i<m_rows.size(); i++ )
	{
	    int cost;
	    if( !EstPlaceCellCost( id, i, &cost ) )
		continue;
	    if( cost < minCost )
	    {
		minCost = cost;
		minCostRow = i;
	    }
	}
    }
#endif
    
    if( minCostRow == -1 )
    {
	printf( "cannot find row for cell %d\n", id );
	return false;
    }
    
    
    static int totalCost = 0;
    totalCost += minCost ;
    
    m_rows[minCostRow].PlaceCell( id );
    m_rows[minCostRow].GetCostChange();
    static int count=0;
    if( count % 2000 == 0 )
    //printf( "[%d] Place cell %d to row %d, cost = %d  %d\n", count, id, minCostRow, minCost, totalCost );
    printf( "[%d] current cost = %d\n", count, totalCost );
    count++;

    
    //if( count> 11 )
    //if( count> 2000 )
    //	msg = true;
    
    return true;
}


bool CSingleRowPlacementLegal::TryPlaceCell( int id, int rowId )
{
    int oldRowId = m_rows[rowId].m_cells[id].rowId;
    if( oldRowId == rowId )
	return false;

    int reduceCost = 0;
    m_rows[oldRowId].RemoveCell( id );
    reduceCost = m_rows[oldRowId].GetCostChange();

    int addCost = 0;
    if( !m_rows[rowId].PlaceCell( id ) )
    {
	m_rows[oldRowId].PlaceCell( id );
	m_rows[oldRowId].GetCostChange();
	return false;
    }
    
    addCost = m_rows[rowId].GetCostChange();

    if( reduceCost + addCost <= 0 )
	return true;

    m_rows[rowId].RemoveCell( id );
    m_rows[rowId].GetCostChange();
    bool succ = m_rows[oldRowId].PlaceCell( id );
    assert( succ );
    m_rows[oldRowId].GetCostChange();

    return false;
}


void CSingleRowPlacementLegal::ReplaceCell( int id )
{
    for( unsigned int row=0; row<m_rows[0].m_cells[id].candidateRows.size(); row++ )
    {
	int i = m_rows[0].m_cells[id].candidateRows[row];
	
	if( abs( m_rows[i].m_y - m_rows[i].m_cells[id].preferY ) >  // min displacement > current displacement
	       abs( m_rows[ m_rows[i].m_cells[id].rowId ].m_y - m_rows[i].m_cells[id].preferY ) +
	       abs( m_rows[i].m_cells[id].x - m_rows[i].m_cells[id].preferCenter ) )
	    continue;
	
	if( TryPlaceCell( id, i ) )
	{
	    //printf( "Replace cell %d to row %d\n", id, i );
	}
    }
    
    static int count = 0;
    if( count % 1000 == 0 )
    printf( "[%d] Replace cell %d \n", count, id );
    count++;
}

void CSingleRowPlacementLegal::UpdateCellPositions()
{
    //map< float, int >::iterator ite;
    vector< pair< float, int > >::iterator ite;
    for( ite=m_sequence.begin(); ite!=m_sequence.end(); ite++ )
    {
	int cellId = ite->second;
	int rowId = CSingleRowPlacement::m_cells[cellId].rowId;
	assert( rowId != -1 );
	int y = m_rows[rowId].m_y;
	int x = m_rows[rowId].GetCellX( cellId );
	m_pDB->SetModuleLocation( m_cellIdMap[cellId], (float)x, (float)y );
    }
    //printf( "%d cell updated\n", m_sequence.size() );
}

bool CSingleRowPlacementLegal::Legalize()
{
    // NOTE (donnie)
    // It is not tested to call ImportCells() before ImportRows()
    ImportRows();
    printf( "SRPL: %d rows imported\n", m_rows.size() );
    
    if( false == ImportCells() )
	return false;
    printf( "SRPL: %d cells imported, seq size %d\n", m_cellIdMap.size(), m_sequence.size() );
    fflush( stdout );

    if( false == PlaceCells() )
	return false;

    UpdateCellPositions();
    return true;
}


// 2007-07-09 (donnie) Load a legal placement and minimize the 
void CSingleRowPlacementLegal::PostLegalSingleRowOpt( vector<CPoint>& preferredPositions )
{
    assert( preferredPositions.size() == m_pDB->m_modules.size() );

    ImportRows();
    ImportCells( &preferredPositions );
    
    vector< pair< float, int > >::iterator ite;
    for( ite=m_sequence.begin(); ite!=m_sequence.end(); ite++ )
    {   
	int blockId = ite->second;
	int rowId = -1;
	 
	// can be faster by binary search 
	for( unsigned int row=0; row<m_rows.size(); row++ )
	{
	    if( m_rows[row].m_y != m_rows[row].m_cells[blockId].preferY )
		continue;
	    if( m_rows[row].m_leftBoundary - m_rows[row].m_cells[blockId].preferCenter > 0 )
		continue;
	    if( m_rows[row].m_cells[blockId].preferCenter - m_rows[row].m_rightBoundary > 0 )
		continue;

	    rowId = row;
	    break;
	}

	if( rowId == -1 )
	{
	    printf( "Error: Cannot find row for cell %d\n", blockId );
	    exit(0);
	}

	m_rows[rowId].PlaceCell( blockId );
    }

    UpdateCellPositions();
}


//////////////////////////////////////////////////////////////////////////////////////////

// TODO: Use vertical row center for "y"

CSingleRowPlacement::CSingleRowPlacement( int rowId, int left, int right, int y ) :
    m_rowId( rowId ),
    m_leftBoundary( left ),
    m_rightBoundary( right ),
    m_totalWidth( 0 ),
    m_capacity( m_rightBoundary - m_leftBoundary ),
    m_y( y )
{
    int id = CreateCell( m_leftBoundary, m_y, 0 );
    m_cellOrder.insert( make_pair(m_cells[id].preferCenter, id) ); // <int,int>
    m_cells[id].x = m_leftBoundary;
    m_cells[id].rowId = m_rowId;

    int id2 = CreateCell( m_rightBoundary, m_y, 0 );
    m_cellOrder.insert( make_pair(m_cells[id2].preferCenter, id2) ); // <int,int>
    m_cells[id2].x = m_rightBoundary;
    m_cells[id2].rowId = m_rowId;

    ConnectClusters( id, id2 );    
}

CSingleRowPlacement::~CSingleRowPlacement()
{
}

void CSingleRowPlacement::NewBackup()   // need to call after GetCostChange()
{
    assert( m_modifyList.size() == 0 ); 
    m_backupCells.clear();
    m_bBackup = true;
}

void CSingleRowPlacement::Restore()
{
    printf( "Restore size %d\n", m_backupCells.size() );
    vector< pair<int,cellInfo> >::iterator ite;
    for( ite=m_backupCells.begin(); ite!=m_backupCells.end(); ite++ )
    {
	printf( "Restore %d in row %d\n", ite->first, m_rowId );
	if( m_cells[ ite->first ].rowId == -1 && ite->second.rowId != -1 )
	    m_cellOrder.insert( make_pair( BoundX( ite->second.preferCenter ), ite->first) ); // <int,int>
	else if( m_cells[ ite->first ].rowId != -1 && ite->second.rowId == -1 )
	    m_cellOrder.erase( GetCellIterator( ite->first ) );
	else
	{
	    printf( " m_cells[ ite->first ].rowId == %d && ite->second.rowId != %d\n",
		    m_cells[ ite->first ].rowId, ite->second.rowId );
	}
	m_cells[ ite->first ] = ite->second;
    }
    m_backupCells.clear();
    m_bBackup = false;
}

void CSingleRowPlacement::PrintCell( unsigned int cellId )
{
    printf( "Cell %d   (preferC= %d  width= %d  cost= %d) ", 
	    cellId , m_cells[cellId].preferCenter, 
	    m_cells[cellId].width, m_cells[cellId].cost );
    
    if( m_cells[cellId].parentCell != -1 )
	printf( "CLUMPED" );
    else if( m_cells[cellId].rowId == m_rowId )
    {
	printf( "PLACED %d", m_cells[cellId].x );
	assert( m_cells[cellId].x >= m_leftBoundary );
	assert( m_cells[cellId].x + m_cells[cellId].width <= m_rightBoundary );
    }
    
    printf( "\n" );
    printf( "   pointers: %d <-- %d (%d)--> %d \n", 
	    m_cells[cellId].preCell, cellId, 
	    m_cells[cellId].parentCell, m_cells[cellId].nextCell );
   
    if( m_cells[cellId].parentCell >= 0 )
    {
	//printf( "   parent: %d\n", m_cells[cellId].parentCell );
	assert( m_cells[cellId].parentCell != 0 );
	assert( m_cells[ m_cells[cellId].parentCell ].x >= m_leftBoundary );
	//printf( "   clumping position: %d\n", GetCellX( cellId ) );
    }
    
    if( m_cells[cellId].subCells.size() > 0 )
    {
	printf( "   subcells: " );
	for( unsigned int i=0; i<m_cells[cellId].subCells.size(); i++ )
	    printf( "%d ", m_cells[cellId].subCells[i] );
	printf( "\n" );
    }
    
    assert( m_cells[cellId].slop.size() == m_cells[cellId].slopChangePoint.size() + 1 );
    printf( "   slops: %d ", m_cells[cellId].slop[0] );
    for( unsigned int i=0; i<m_cells[cellId].slopChangePoint.size(); i++ )
    {
	printf( "(%d) ", m_cells[cellId].slopChangePoint[i] );
	printf( "%d ", m_cells[cellId].slop[i+1] );
    }
    printf( "\n" );
    //printf( " ==>  best region: [%d %d]\n", 
    //	    m_cells[cellId].optLeft, m_cells[cellId].optRight );
}

void CSingleRowPlacement::PrintAllCells()
{
    int cost = 0;
    printf( "\n\nSRP: cell# = %d\n", m_cells.size() );
    printf( "SRP%d: placed_cluster# = %d\n", m_rowId, m_cellOrder.size() );
    printf( "SRP%d: region [%d %d]\n", m_rowId, m_leftBoundary, m_rightBoundary );
    printf( "SRP%d: MAP: ", m_rowId );
    multimap<int,int>::const_iterator ite;
    for( ite=m_cellOrder.begin(); ite!=m_cellOrder.end(); ite++ )
    {
	printf( "%d@%dp%dw%d ", 
		ite->second, 
		GetCellX( ite->second ), 
		ite->first, 
		m_cells[ite->second].originalWidth );
	fflush( stdout );
	cost += GetCellCost( ite->second );
    }
    printf( "\n" );
    
    printf( "SRP%d: LINK: ", m_rowId );
    int cell = m_cellOrder.begin()->second;
    while( cell >= 0 )
    {
	printf( "%d@%d(w%d) ", cell, m_cells[cell].x, m_cells[cell].width );
	if( m_cells[cell].preCell != -1 )
	assert( m_cells[ m_cells[cell].preCell ].nextCell == cell );
	cell = m_cells[cell].nextCell;
    }
    printf( "\n" );

    printf( "SRP%d: Total cost= %d\n", m_rowId, cost );

    for( ite=m_cellOrder.begin(); ite!=m_cellOrder.end(); ite++ )
	PrintCell( ite->second );
    
    //for( unsigned int i=0; i<m_cells.size(); i++ )
    //	PrintCell( i );
}

/////////////////////////////////////////////////////////////////////////////////

// static
int CSingleRowPlacement::CreateCell( int x, int y, int width )
{
    cellInfo cell;
    cell.x = 0;
    cell.preferCenter = x;   
    cell.preferY = y;
    cell.cost = 0;
    cell.width = cell.originalWidth = width;
    cell.preCell = cell.nextCell = cell.parentCell = -1;
    cell.rowId = -1; // UNPLACE
   
    m_cells.push_back( cell );

    int cellId = (int)m_cells.size() - 1;
    InitCellSlop( cellId );
    
    return cellId;
}

// static
void CSingleRowPlacement::InitCellSlop( int id )
{
    //assert( m_cells[id].subCells.size() ==  0 );
    m_cells[id].subCells.clear();
    m_cells[id].width = m_cells[id].originalWidth;
    m_cells[id].slop.resize( 2 );
    m_cells[id].slopChangePoint.resize( 1 );
    m_cells[id].slop[0] = -1;
    m_cells[id].slop[1] = 1;
    m_cells[id].slopChangePoint[0] = m_cells[id].preferCenter - m_cells[id].width/2;
    m_cells[id].optLeft = m_cells[id].optRight = m_cells[id].preferCenter - m_cells[id].width/2;
}

vector<CSingleRowPlacement::cellInfo> CSingleRowPlacement::m_cells;

//////////////////////////////////////////////////////////////////////////////////

int CSingleRowPlacement::GetCostChange()
{
    int costChange = 0;
    set<int>::iterator ite;
    for( ite = m_modifyList.begin(); ite != m_modifyList.end(); ite++ )
    {
	if( msg )
	if( m_cells[ *ite ].rowId != m_rowId && m_cells[ *ite ].rowId != -1 )
	{
	    printf( "cell %d   rowId %d   current row %d\n", *ite, m_cells[ *ite ].rowId , m_rowId );
	}
	assert( m_cells[ *ite ].rowId == -1 ||
		m_cells[ *ite ].rowId == m_rowId );
		
	int newCost = GetCellCost( *ite );
	assert( newCost >= 0 );
	costChange += newCost - m_cells[ *ite ].cost;
	m_cells[ *ite ].cost = newCost;
	
	if( m_cells[ *ite ].subCells.size() != 0 )
	{
	    for( unsigned int i=0; i<m_cells[ *ite ].subCells.size(); i++ )
	    {
		newCost = GetCellCost( m_cells[ *ite ].subCells[i] );
		assert( newCost >= 0 );
		costChange += newCost - m_cells[ m_cells[ *ite ].subCells[i] ].cost;
		m_cells[ m_cells[ *ite ].subCells[i] ].cost = newCost;
	    }
	}
    }
    m_modifyList.clear();
    return costChange;
}

void CSingleRowPlacement::RemoveCell( int id )
{
    //if( m_bBackup && m_modifyList.find( id ) == m_modifyList.end() )
    //	m_backupCells.push_back( make_pair<int,cellInfo>( id, m_cells[id] ) );
    m_modifyList.insert( id );

#if 0
    if( msg )
	printf( "RemoveCell %d from row %d\n", id, m_rowId );
    fflush( stdout );
#endif

    
    multimap<int,int>::iterator ite = GetCellIterator( id );
    multimap<int,int>::iterator ite2 = ite;
    int nextCellId = (++ite2)->second;
    m_cellOrder.erase( ite );
    m_totalWidth -= m_cells[id].originalWidth;
    m_cells[id].rowId = -1;
    int nextClusterId = m_cells[id].nextCell;
    
    if( m_cells[id].subCells.size() > 0 )
    {

#if 0
	if( msg )
	printf( "HEAD " );
	fflush( stdout );
#endif
	
	// leading cell of the cluster
	UnplaceCluster( id );
	InitCellSlop( id );

	vector<int> replaceList;

	int currentId = nextCellId;
	multimap<int,int>::iterator ite = GetCellIterator( currentId );
	while( ite->second != nextClusterId )
	{
	    replaceList.push_back( ite->second );
	    ite++;
	}

	for( unsigned int i=0; i<replaceList.size(); i++ )
	{
	    m_cells[ replaceList[i] ].rowId = -1;
	    m_cells[ replaceList[i] ].parentCell = -1;
	    m_cellOrder.erase( GetCellIterator( replaceList[i] ) );
	    m_totalWidth -= m_cells[ replaceList[i] ].originalWidth;
	}	    

	for( unsigned int i=0; i<replaceList.size(); i++ )
	{
#if 0
	    if( msg )
	    printf( "HEAD: Replace cell %d \n", replaceList[i] );
#endif
	    PlaceCell( replaceList[i] );
	}
    }
    else if( m_cells[id].parentCell != -1 )
    {
#if 0
	if( msg )
	printf( "MEMBER " );
	fflush( stdout );
#endif
	
	// cluster member
	int handle = m_cells[id].parentCell;
	m_cells[id].parentCell = -1;
	nextClusterId = m_cells[handle].nextCell;

	UnplaceCluster( handle );
	InitCellSlop( handle );

	vector<int> replaceList;
	int currentId = handle;
	multimap<int,int>::iterator ite = GetCellIterator( currentId );
	while( ite->second != nextClusterId )
	{
	    replaceList.push_back( ite->second );
	    ite++;
	}

	for( unsigned int i=0; i<replaceList.size(); i++ )
	{
	    m_cells[ replaceList[i] ].rowId = -1;
	    m_cells[ replaceList[i] ].parentCell = -1;
	    m_cellOrder.erase( GetCellIterator( replaceList[i] ) );
	    m_totalWidth -= m_cells[ replaceList[i] ].originalWidth;
	}

	for( unsigned int i=0; i<replaceList.size(); i++ )
	{
#if 0
	    if( msg )
	    printf( "MEMBER: Replace %d\n", replaceList[i] );
#endif
	    PlaceCell( replaceList[i] );
	}
    }
    else
    {
#if 0
	// alone
	if( msg )
	printf( "REMOVE alone    %d %d %d\n", m_cells[id].preCell, id, m_cells[id].nextCell );
#endif
	UnplaceCluster( id );
    }
	 
    // Chain effect
    //printf( "chain\n" );
    int currentId = nextClusterId;
    while( m_cells[ currentId ].optLeft != m_cells[ currentId ].optRight &&
	    m_cells[ currentId].x != m_cells[ currentId ].optLeft )
    {
#if 0
	if( msg )
	printf( "==> current %d (subsize%d) [%d %d] x %d\n", currentId, 
		m_cells[currentId].subCells.size(),
		m_cells[ currentId ].optLeft, m_cells[ currentId ].optRight,
		m_cells[ currentId].x );
#endif	
	if( m_cells[currentId].subCells.size() == 0 )
	{
#if 0
	    if( msg )
	    printf( "PLACE SINGLE\n" );
#endif
	    UnplaceCluster( currentId );
	    PlaceCell( currentId );
	    currentId = m_cells[ currentId ].nextCell;
	}
	else
	{
	    // decluster and place
	    int nextCluster = m_cells[currentId].nextCell;
#if 0
	    if( msg )
	    printf( "PLACe CHAIN to %d   %d<- ->%d %d\n", nextCluster,
		 m_cells[currentId].preCell, 
		 m_cells[currentId].nextCell,
		m_cells[currentId].parentCell  );
#endif
	    UnplaceCluster( currentId );
	    InitCellSlop( currentId );

	    vector<int> replaceList;
	    multimap<int,int>::iterator ite = GetCellIterator( currentId );
	    while( ite->second != nextCluster )
	    {
		replaceList.push_back( ite->second );
		ite++;
#if 0
		if( msg )
		printf( "ADD %d\n", ite->second );
#endif
	    }

	    for( unsigned int i=0; i<replaceList.size(); i++ )
	    {
		m_cells[ replaceList[i] ].rowId = -1;
		m_cells[ replaceList[i] ].parentCell = -1;
		m_cellOrder.erase( GetCellIterator( replaceList[i] ) );
		m_totalWidth -= m_cells[ replaceList[i] ].originalWidth;
	    }

	    for( unsigned int i=0; i<replaceList.size(); i++ )
	    {
#if 0
		if( msg )
		printf( "CHAIN: Replace %d\n", replaceList[i] );
#endif
		PlaceCell( replaceList[i] );
	    }

	    currentId = nextCluster;
	}
    }
}

int CSingleRowPlacement::BoundX( int x )
{
    if( x > m_leftBoundary )
	return min( x, m_rightBoundary-1 ); // smallest site = 1
    return m_leftBoundary;
}

multimap<int,int>::iterator CSingleRowPlacement::GetCellIterator( int id )
{
    //printf( "GetCellIterator(%d)\n", id );
    multimap<int,int>::iterator ite = m_cellOrder.find( BoundX( m_cells[id].preferCenter ) );
    int count=0;
    while( ite->second != id )
    {
	ite++;
	if( ite == m_cellOrder.end() )
	{
	    printf( "Cannot find %d (rowId %d) in row %d \n", id, m_cells[id].rowId, m_rowId );
	    PrintAllCells();
	}
	assert( ite != m_cellOrder.end() );
	assert( count++ < 10000 );
    }
    return ite;
}


bool CSingleRowPlacement::ValidCell( int id )
{
    if( m_cells[id].preCell == -1 && m_cells[id].nextCell == -1 )
	return false;
    return true;
}

bool CSingleRowPlacement::PlaceCell( int cellId )
{
#if 1
    if( msg )
    printf( "PlaceCell %d (p%dw%d) into row %d. %d %d %d", 
	    cellId, 
	    m_cells[cellId].preferCenter,
	    m_cells[cellId].width, 
	    m_rowId,
	 m_cells[cellId].preCell, 
	 m_cells[cellId].nextCell,
	m_cells[cellId].parentCell  );
    fflush( stdout );
    assert( m_cells[cellId].preCell == -1 );
    assert( m_cells[cellId].nextCell == -1 );
    assert( m_cells[cellId].parentCell == -1 );
#endif

    if( m_cells[cellId].rowId == -1 )
    {
	if( m_totalWidth + m_cells[cellId].width > m_capacity )
	{
	    //fprintf( stderr, "Row %d overflow\n", m_rowId );
	    return false;
	}
	
	m_cellOrder.insert( make_pair( BoundX( m_cells[cellId].preferCenter ), cellId) ); // <int,int>
	m_totalWidth += m_cells[cellId].originalWidth;
	m_cells[cellId].rowId = m_rowId;
    }
#if 1
    else
    {
	// Only happen when Collapse() and PlaceCell()
    }
    assert( m_cells[cellId].rowId == m_rowId );
    // When rowId != m_rowId, the cell must be unplaced from other row first.
#endif
    
    m_modifyList.insert( cellId );
   
    multimap<int,int>::iterator ite = GetCellIterator( cellId );
    multimap<int,int>::iterator ite2 = ite;
    
    int preCellId = (--ite)->second;
    int nextCellId = (++ite2)->second; 
    
    int preClusterId = preCellId;
    if( m_cells[preClusterId].parentCell != -1 )
	preClusterId = m_cells[preClusterId].parentCell;
    int nextClusterId = m_cells[preClusterId].nextCell;

    if( ( m_cells[preCellId].parentCell != -1 && 
		m_cells[preCellId].parentCell == m_cells[nextCellId].parentCell ) ||
	    preCellId == m_cells[nextCellId].parentCell )
    {
#if 1
	if( msg )
	printf( "  clu %d <--*--> %d  cel %d(%d) <-- %d --> %d(%d)\n", 
		preClusterId, 
		nextClusterId,
		preCellId, 
		m_cells[preCellId].parentCell,
		cellId, 
		nextCellId, 
		m_cells[nextCellId].parentCell );
#endif

	// cluster member
	int handle = preClusterId;

	UnplaceCluster( handle );
	InitCellSlop( handle );

	vector<int> replaceList;
	int currentId = handle;
	multimap<int,int>::iterator ite = GetCellIterator( currentId );
	while( ite->second != nextClusterId )
	{
	    replaceList.push_back( ite->second );
	    ite++;
	}

	for( unsigned int i=0; i<replaceList.size(); i++ )
	{
	    m_cells[ replaceList[i] ].rowId = -1;
	    m_cells[ replaceList[i] ].parentCell = -1;
	    m_cellOrder.erase( GetCellIterator( replaceList[i] ) );
	    m_totalWidth -= m_cells[ replaceList[i] ].originalWidth;
	}

	for( unsigned int i=0; i<replaceList.size(); i++ )
	{
#if 0
	    if( msg )
	    printf( "PLACE-MEMBER: Replace %d\n", replaceList[i] );
#endif
	    PlaceCell( replaceList[i] );
	}

	return true;
    }

#if 1
    if( msg )
    printf( "   clusters %d <-- --> %d    cells %d <-- %d --> %d\n", 
	    preClusterId, nextClusterId,
	    preCellId, cellId, nextCellId );
#endif
    assert( preClusterId >= 0 );
    if( nextClusterId == -1 )
    {
	PrintAllCells();
    }	
    assert( nextClusterId >= 0 );
    
    int optRight = 
	min( m_rightBoundary - m_cells[cellId].width, 
	    max( m_leftBoundary, m_cells[cellId].optRight /*- m_cells[cellId].width/2 */) );
    int optLeft = 
	min( m_rightBoundary - m_cells[cellId].width, 
	    max( m_leftBoundary, m_cells[cellId].optLeft /*- m_cells[cellId].width/2 */) );

    int preClusterRight = m_cells[preClusterId].x + m_cells[preClusterId].width;
   
    //printf( "optRight %d  optLeft %d   preClusterRight %d\n", optRight, optLeft, preClusterRight );
    
    if( preClusterRight <= optRight )
    {
	m_cells[cellId].x = 
	    min( m_rightBoundary - m_cells[cellId].width,
		    max( m_leftBoundary, 
			max( preClusterRight, optLeft ) ) );
	
#if 0
    if( msg )
	printf( "  X= %d\n", m_cells[cellId].x );
#endif
	
	m_cells[cellId].preCell = preClusterId;
	m_cells[cellId].nextCell = nextClusterId;
	m_cells[preClusterId].nextCell = cellId;

	assert( nextClusterId != -1 );
	//if( nextClusterId != -1 )
	{
	    m_cells[nextClusterId].preCell = cellId;

	    if( m_cells[cellId].x + m_cells[cellId].width > m_cells[nextClusterId].x )
	    {
		// Overlap with the next cell
		UnplaceCluster( nextClusterId );
		PlaceCell( nextClusterId );
	    }
	}
	//if( msg )
	//printf( "  PLACED %d .. ", cellId );
    }
    else
    {
	// Overlap with the previous cell
	UnplaceCluster( preClusterId );
	CollapseCell( preClusterId, cellId );
	PlaceCell( preClusterId );
    }
    //if( msg )
    //printf( "back" );
    return true;
}

void CSingleRowPlacement::UnplaceCluster( int cell )
{
    ConnectClusters( m_cells[cell].preCell, m_cells[cell].nextCell );
    m_cells[cell].preCell = m_cells[cell].nextCell = -1;
    //m_cells[cell].x = m_leftBoundary - 1;
}

void CSingleRowPlacement::ConnectClusters( int from, int to )
{
    m_cells[ from ].nextCell = to;
    assert( to != -1 );
    //if( to != -1 )
	m_cells[ to ].preCell = from;
}

int CSingleRowPlacement::GetCellCost( int id )
{
    if( m_cells[id].rowId == -1 )
    {
	//fprintf( stderr, "Warning: GetCellCost() for an unplaced cell\n" );
	return 0;
    }
    assert( m_cells[id].rowId == m_rowId );
    int y = abs( m_cells[id].preferY - m_y );
    return abs( GetCellX( id ) + m_cells[id].originalWidth/2 - m_cells[id].preferCenter ) + y;
}

int CSingleRowPlacement::GetCellX( int id )
{
    if( m_cells[id].parentCell == -1 )
	return m_cells[id].x;

    int parentId = m_cells[id].parentCell;
    assert( m_cells[parentId].parentCell == -1 );
    int x = m_cells[parentId].x;

    multimap<int,int>::iterator ite = GetCellIterator( parentId );
    
    //multimap<int,int>::iterator ite = m_cellOrder.find( BoundX( m_cells[parentId].preferCenter ) );
    //while( ite->second != parentId )
    //	ite++;
    //assert( ite != m_cellOrder.end() );
    //
    int count = 0;
    while( ite->second != id )
    {
	x += m_cells[ite->second].originalWidth;
	ite++;
	assert( count++ < 10000 );
    }
    return x;
}

void CSingleRowPlacement::CollapseCellFromCenter( map<int,int>::iterator ite1, map<int,int>::iterator ite2 )
{
#if 0
    if( msg )
    printf( "Collapse from %d and %d (c)\n", ite1->second, ite2->second );
    assert( ite1->second != ite2->second );
#endif
    InitCellSlop( ite1->second );

    map<int,int>::iterator ite=ite1;
    ite++;
    for( ; ite!=ite2; ite++ )
    {
	assert( m_cells[ite->second].subCells.size() == 0 );
	CollapseCell( ite1->second, ite->second );
    }
}

void CSingleRowPlacement::CollapseCell( int lastCellId, int cellId )
{
#if 0
    if( msg )
    printf( "Collapse %d and %d\n", lastCellId, cellId );
    assert( lastCellId != cellId );
#endif


    int w = 0;
    map<int,int>::iterator iteCell = GetCellIterator( lastCellId );
    while( iteCell->second != cellId )
    {
	w += m_cells[iteCell->second].originalWidth;
	iteCell++;
    }

#if 0    
    assert( lastCellId >= 0 );
    assert( cellId >= 0 ); 
#endif
    m_cells[lastCellId].width += m_cells[cellId].width;
    m_cells[lastCellId].subCells.push_back( cellId );
    m_cells[cellId].parentCell = lastCellId;
    for( unsigned int i=0; i<m_cells[cellId].subCells.size(); i++ )
    {
	m_cells[cellId].width -= m_cells[ m_cells[cellId].subCells[i] ].width;
	m_cells[lastCellId].subCells.push_back( m_cells[cellId].subCells[i] );
	m_cells[ m_cells[cellId].subCells[i] ].parentCell = lastCellId;
    }
   
    // need to update "optLeft" "optRight" "slop" "slopChangePoint" "cost"
    unsigned int i=0, j=0;
    vector<int> slops;
    vector<int> points;
    vector<int>& s1 = m_cells[lastCellId].slop;
    vector<int>& s2 = m_cells[cellId].slop;
    vector<int>& p1 = m_cells[lastCellId].slopChangePoint;
    vector<int> p2 = m_cells[cellId].slopChangePoint;

#if 0
    if( p1.size() > m_cells[lastCellId].subCells.size()+1 )
	printf( "p1 size %d   subcellsize %d\n", p1.size(), m_cells[lastCellId].subCells.size()+1 );
    if( p2.size() > m_cells[cellId].subCells.size()+1 )
	printf( "p2 size %d   subcellsize %d\n", p2.size(), m_cells[cellId].subCells.size()+1 );
    assert( p1.size() <= m_cells[lastCellId].subCells.size()+1 );
    assert( p2.size() <= m_cells[cellId].subCells.size()+1 );
#endif
    
    m_cells[cellId].subCells.clear();
    m_cells[cellId].width = m_cells[cellId].originalWidth;

    for( unsigned int i=0; i<p2.size(); i++ )
	p2[i] -= w;
    
    slops.push_back( s1[0] + s2[0] );
    while( i < p1.size() && j < p2.size() )
    {
	if( p1[i] == p2[j] )
	{
	    points.push_back( p1[i] );
	    i++;
	    j++;
	}	
	else if( p1[i] < p2[j] )
	{
	    points.push_back( p1[i] );
	    i++;
	}	    
	else // p1[i] > p2[j]
	{
	    points.push_back( p2[j] );
	    j++;
	}
	slops.push_back( s1[i] + s2[j] );
    }

    if( i < p1.size() )
    {
	assert( j == p2.size() );
	while( i < p1.size() )
	{
	    points.push_back( p1[i] );
	    i++;
	    slops.push_back( s1[i] + s2[j] );
	}
    }
    else if( j < p2.size() )
    {
	assert( i == p1.size() );
	while( j < p2.size() ) 
	{
	    points.push_back( p2[j] );
	    j++;
	    slops.push_back( s1[i] + s2[j] );
	}
    }
    
#if 0
    assert( slops.size() == points.size() + 1 );
#endif
    
    slops.resize( slops.size() );
    points.resize( points.size() ); 

#if 0
    printf( "   slops1: %d ", s1[0] );
    for( unsigned int i=0; i<p1.size(); i++ )
    {
	printf( "(%d) ", p1[i] );
	printf( "%d ", s1[i+1] );
    }
    printf( "\n" );
    printf( "   slops2: %d ", s2[0] );
    for( unsigned int i=0; i<p2.size(); i++ )
    {
	printf( "(%d) ", p2[i] );
	printf( "%d ", s2[i+1] );
    }
    printf( "\n" );
    printf( "   slops: %d ", slops[0] );
    for( unsigned int i=0; i<points.size(); i++ )
    {
	printf( "(%d) ", points[i] );
	printf( "%d ", slops[i+1] );
    }
    printf( "\n" );
#endif


    for( unsigned int i=1; i<slops.size(); i++ )
    {
	if( slops[i] == slops[i-1] )
	{
	    printf( "ERROR in slop\n" );
	    exit(0);
	}
    }
    
    m_cells[lastCellId].slop = slops;
    m_cells[lastCellId].slopChangePoint = points;
    
    vector<int>::const_iterator ite;
    ite = upper_bound( slops.begin(), slops.end(), 0 );
    ite--;
    //printf( "%d %d\n", ite - slops.begin(), *ite );
    assert( ite+1 != slops.end() );
    assert( *(ite+1) > 0 );
    if( *ite == 0 )
    {
	m_cells[lastCellId].optLeft  = points[ ite - slops.begin() - 1 ];
	m_cells[lastCellId].optRight = points[ ite - slops.begin() ];
	//printf( "   opt range %d %d\n", 
	//	m_cells[lastCellId].optLeft , m_cells[lastCellId].optRight );
    }
    else // *ite < 0 
    {
	m_cells[lastCellId].optLeft = m_cells[lastCellId].optRight = points[ ite - slops.begin() ];
    }
#if 0
    assert( m_cells[lastCellId].optLeft <= m_cells[lastCellId].optRight );
#endif

    InitCellSlop( cellId );
}



