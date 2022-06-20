#include "CongMatching.h"
#include <algorithm>
#include "CongMap.h"
#include "verify.h"

using namespace std;

vector<Module>* CCongMatching::CompareModuleWidth::m_pMod = 0;

CCongMatching::CCongMatching( CPlaceDB& set_placedb, 
	//std::vector<CSegment> set_segments,
	const int& xdim,
	const int& ydim ) :
                m_placedb( set_placedb ), 
		//m_segments( set_segments ),
		m_xdim( xdim ),
		m_ydim( ydim ),
		m_left(m_placedb.m_coreRgn.left), 
		m_right(m_placedb.m_coreRgn.right),
		m_top(m_placedb.m_coreRgn.top), 
		m_bottom(m_placedb.m_coreRgn.bottom),
		m_hlength( (m_right-m_left)/static_cast<double>(m_xdim) ),
		m_vlength( (m_top-m_bottom)/static_cast<double>(m_ydim) )
{
    
    m_module_order.reserve( m_placedb.m_modules.size() );
    for( int i = 0 ; i < static_cast<int>( m_placedb.m_modules.size() ) ; i++ )
    {
	if( m_placedb.m_modules[i].m_height > m_placedb.m_rowHeight || 
		m_placedb.m_modules[i].m_isFixed )
	{
	    continue;
	}
	if( m_placedb.m_modules[i].m_width == 0 )
	    continue;
	    
	m_module_order.push_back( i );
    }

    CompareModuleWidth::m_pMod = &(m_placedb.m_modules);
    sort( m_module_order.begin(), m_module_order.end(), CompareModuleWidth() );

    //Construct m_segments
    for( vector<CSiteRow>::const_iterator iteRow = m_placedb.m_sites.begin() ;
	    iteRow != m_placedb.m_sites.end() ; iteRow++ )
    {
	for( unsigned int iInterval = 0 ; iInterval < iteRow->m_interval.size() ; iInterval=iInterval+2 )
	{
	    m_segments.push_back( CSegment( iteRow->m_bottom,
			iteRow->m_interval[iInterval],
			iteRow->m_interval[iInterval+1] ) );
	}
    }
    for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
    {
	const Module& curModule = m_placedb.m_modules[iModule];
	//Skip Macro modules
	if( curModule.m_height > m_placedb.m_rowHeight || curModule.m_isFixed )
	    continue;
	if( curModule.m_width == 0 )
	    continue;

	//Find the corresponding segment of this module,
	//and insert the module id into the segment
	CSegment compSeg( curModule.m_y, curModule.m_x, curModule.m_x+curModule.m_width );
	vector<CSegment>::iterator iteFindSegment = lower_bound( m_segments.begin(),
		m_segments.end(),
		compSeg,	
		ComparePseudoSegment() );
	if( iteFindSegment != m_segments.end() &&
		iteFindSegment->m_bottom == curModule.m_y )
	{
	    iteFindSegment->AddModuleId( iModule );
	}
	else
	{
	    fprintf(stderr, "Warning: InitFroBBCellSwap(), cannot find legal segment for "
		    "module '%s' at (%.2f, %.2f) w: %.2f h: %.2f\n",
		    curModule.m_name.c_str(), 
		    curModule.m_x, curModule.m_y, curModule.m_width, curModule.m_height );
	    if( iteFindSegment != m_segments.end() )
	    {
		fprintf(stderr, "   iteFindSegment  : bottom %.2f left: %.2f right: %.2f\n",
			iteFindSegment->m_bottom, iteFindSegment->m_left, iteFindSegment->m_right );
		fprintf(stderr, "   iteFindSegment-1: bottom %.2f left: %.2f right: %.2f\n",
			(iteFindSegment-1)->m_bottom, (iteFindSegment-1)->m_left, (iteFindSegment-1)->m_right );
	    }
	}
	//Warning: this module is not on any segments
	if( iteFindSegment == m_segments.end() )
	{
	    cerr << "Warning: Module " << iModule << " is not on any segments" << endl;
	}
    }
    
    //Sort the module id's by their x coordinates
    CompareModuleById::m_pDB = &m_placedb;
    for( vector<CSegment>::iterator iteSegment = m_segments.begin();
	    iteSegment != m_segments.end() ; iteSegment++ )
    {
	sort( iteSegment->m_module_ids.begin(),
		iteSegment->m_module_ids.end(),
		CompareModuleById::CompareXCoor );
    }
    
    //Construct the module id -> segment position map
    for( unsigned int SegId = 0 ; SegId < m_segments.size() ; SegId++ )
    {
	const CSegment& curSeg = m_segments[SegId];
	for( unsigned int orderId = 0 ; orderId < curSeg.m_module_ids.size() ; orderId++ )
	{
	    const int moduleId = curSeg.m_module_ids[ orderId ];
	    CSegmentPositionIndex index = CSegmentPositionIndex( SegId, orderId );
	    m_segment_position_map.insert( pair<int, CSegmentPositionIndex>( moduleId, index ) );
	}
    }
    
    //Construct m_net_bins
    m_net_bins.resize( m_xdim );
    for( int i = 0 ; i < m_xdim ; i++ )
    {
	m_net_bins[i].resize( m_ydim, 0 );
    }
    for( unsigned int netId = 0 ; netId < m_placedb.m_nets.size() ; netId++ )
    {
	const vector<int>& curNet = m_placedb.m_nets[netId];
	AddNetToBins( curNet );
    }
}

void CCongMatching::DoMatching( const vector<int>& module_ids,
      const vector<CPoint>& positions, 
      vector<int>& matching_results )
{
    if( module_ids.empty() )
    {
	fprintf( stderr, "Warning: no modules in the list\n" );
	return;
    }

    if( positions.empty() )
    {
	fprintf( stderr, "Warning: no positions in the list\n" );
    }

    if( module_ids.size() != positions.size() )
    {
	fprintf( stderr, "Warning: module number isn't match with position number\n" );
    }
    
    //compute the cost to each position
    //transform and solve the matching problem
    //return the matching results
    lap2 solver( module_ids.size() );

    for( unsigned int iMod = 0 ; iMod < module_ids.size() ; iMod++ )
    {
	const int mid = module_ids[ iMod ];
	const Module& curModule = m_placedb.m_modules[ mid ];
	const CPoint origPos( curModule.m_x, curModule.m_y );
	const int origCost = ComputeModuleOverlap( mid );
	
	for( unsigned int iPos = 0 ; iPos < positions.size() ; iPos++ )
	{
	    if( iPos == iMod )
	    {
		solver.put( iMod, iPos, 0 );
		continue;
	    }
	    
	    const CPoint& curPos = positions[iPos];
	    m_placedb.SetModuleLocation( mid, curPos.x, curPos.y );
	    const int curCost = ComputeModuleOverlap( mid );
	    solver.put( iMod, iPos, curCost - origCost );
	}

	m_placedb.SetModuleLocation( mid, origPos.x, origPos.y );
    }

    solver.lap_solve();
    solver.getresult( matching_results );
}

bool CCongMatching::FindModulesAndPositions( const int& number,
	vector<int>& module_ids,
	vector<CPoint>& positions )
{
    //vector<Module>& modules = m_placedb.m_modules;
    module_ids.clear();
    positions.clear();
    module_ids.reserve( number );
    positions.reserve( number );
    int start_order = rand() % number;
    int start_mid = m_module_order[ start_order ];
    const double start_width = m_placedb.m_modules[ start_mid ].m_width;
    
    //printf( "start_order %d start_width %.0f\n", start_order, start_width );
    
    CompareModuleWidth::m_pMod = &(m_placedb.m_modules);
    start_order = lower_bound( m_module_order.begin(), 
	    m_module_order.end(), 
	    start_mid, 
	    CompareModuleWidth() ) - m_module_order.begin();
    
    //start_mid = m_module_order[ start_order ];

    for( int i = start_order ; 
	    i < static_cast<int>( m_module_order.size() ) && 
	    static_cast<int>( module_ids.size() ) < number ; 
	    i++ )
    {
	const int mid = m_module_order[i];
	const double width = m_placedb.m_modules[ mid ].m_width;
	const Module& curModule = m_placedb.m_modules[ mid ];
	
	if( width == start_width )
	{
	    module_ids.push_back( mid );
	    positions.push_back( CPoint( curModule.m_x, curModule.m_y ) );
	}
	else
	{
	   double right = FindRightBound( mid );
	   if( right - curModule.m_x >= start_width )
	   {	
		module_ids.push_back( mid );
		positions.push_back( CPoint( curModule.m_x, curModule.m_y ) );
	   }
	}
    }

    if( module_ids.size() < 2 )
    {
	fprintf( stderr, "Warning: no swappable modules are found\n" );
	fflush( stderr );
	module_ids.clear();
	positions.clear();
	return false;
    }
    else
    {
	return true;
    }
    
    
}

double CCongMatching::FindRightBound( const int& module_id )
{
#if 1
    map<int, CSegmentPositionIndex>::iterator iteFind = m_segment_position_map.find( module_id );
    if( iteFind == m_segment_position_map.end() )
    {
	printf( "module %d not found\n", module_id );
	fflush(stdout);
    }
#endif

    const CSegmentPositionIndex& curIndex( m_segment_position_map[ module_id ] );
    if( curIndex.m_segId < 0 || curIndex.m_segId >= static_cast<int>(m_segments.size()) )
    {
	fprintf( stderr, "Warning: incorrect segment index %d\n", curIndex.m_segId );
	fflush( stderr );
    }

    const CSegment curSeg = m_segments[ curIndex.m_segId ];
    if( curIndex.m_orderId < static_cast<int>( curSeg.m_module_ids.size() ) - 1 )
    {
	const int next_order_id = curIndex.m_orderId + 1;
	const int next_mid = curSeg.m_module_ids[ next_order_id ];
	const Module& nextModule = m_placedb.m_modules[ next_mid ];
	
	return nextModule.m_x;
    }
    else
    {
	return curSeg.m_right;
    }
}

CRect CCongMatching::ComputeNetBBox( const Net& n )
{
    double minX = m_placedb.m_pins[ n[0] ].absX;
    double maxX = m_placedb.m_pins[ n[0] ].absX;
    double minY = m_placedb.m_pins[ n[0] ].absY;
    double maxY = m_placedb.m_pins[ n[0] ].absY;

    for( unsigned int i = 1 ; i < n.size() ; i++ )
    {
	minX = min( minX, m_placedb.m_pins[ n[i] ].absX );
	maxX = max( maxX, m_placedb.m_pins[ n[i] ].absX );
	minY = min( minY, m_placedb.m_pins[ n[i] ].absY );
	maxY = max( maxY, m_placedb.m_pins[ n[i] ].absY );
    }

    CRect r;
    r.left = minX;
    r.right = maxX;
    r.bottom = minY;
    r.top = maxY;

    return r;
}

int CCongMatching::ComputeNetOverlap( const Net& n )
{
    CRect bbox = ComputeNetBBox( n );

    int left_bin_index = GetHBinIndex( bbox.left );
    int right_bin_index = GetHBinIndex( bbox.right );
    int bottom_bin_index = GetVBinIndex( bbox.bottom );
    int top_bin_index = GetVBinIndex( bbox.top );

    int cost = 0;
    for( int i = left_bin_index ; i <= right_bin_index ; i++ )
    {
	for( int j = bottom_bin_index ; j <= top_bin_index ; j++ )
	{
	    cost += m_net_bins[i][j];
	}
    }

    return cost;
}

int CCongMatching::ComputeModuleOverlap( const int& mid )
{
    int cost = 0;

    const Module& curModule = m_placedb.m_modules[ mid ];

    for( unsigned int iNet = 0 ; iNet < curModule.m_netsId.size() ; iNet++ )
    {
	const int netid = curModule.m_netsId[ iNet ];
	const Net& curNet = m_placedb.m_nets[ netid ];
	cost += ComputeNetOverlap( curNet );
    }

    return cost;
}

int CCongMatching::ComputeModuleOverlap( const std::vector<int>& module_ids )
{
    int cost = 0;

    for( unsigned int iMod = 0 ; iMod < module_ids.size() ; iMod++ )
    {
	const int mid = module_ids[ iMod ];
	cost += ComputeModuleOverlap( mid );
    }

    return cost;
}

bool CCongMatching::Solve( void )
{
    //test code
    m_placedb.CreateCongMap( m_xdim, m_ydim );
    m_placedb.m_pCongMap->OutputBinOverflowFigure( "before_matching" );
    m_placedb.ClearCongMap();
    m_placedb.m_modules_bak = m_placedb.m_modules;
    //@test code

    CRect region;
    region.left = -17841;
    region.bottom = -2922;
    region.right = 11949;
    region.top = 26860;
    //bool result = SolveRegionOnce( 500, region );
    bool result = SolveRegionToWholeChip( 500, 40, 40 );
    
    //test code
    m_placedb.CreateCongMap( m_xdim, m_ydim );
    m_placedb.m_pCongMap->OutputBinOverflowFigure( "after_matching1" );
    m_placedb.ClearCongMap();
    
    result |= SolveRegionToWholeChip( 500, 40, 40 );
    m_placedb.CreateCongMap( m_xdim, m_ydim );
    m_placedb.m_pCongMap->OutputBinOverflowFigure( "after_matching2" );
    m_placedb.ClearCongMap();
    
    result |= SolveRegionToWholeChip( 500, 40, 40 );
    m_placedb.CreateCongMap( m_xdim, m_ydim );
    m_placedb.m_pCongMap->OutputBinOverflowFigure( "after_matching3" );
    m_placedb.ClearCongMap();
    
    m_placedb.OutputGnuplotFigureWithZoom( "movement", true, true, true );
    //@test code

    return result;
}

bool CCongMatching::SolveRegionToWholeChip( const int& size, const int& xbins, const int& ybins )
{
    if( xbins >= m_xdim || ybins >= m_ydim )
    {
	return false;
    }
    
    vector<CRect> regions;
    int xstep = xbins/2;
    int ystep = ybins/2;

    for( int xindex = 0 ; xindex < m_xdim - 1 ; xindex += xstep )
    {
	CRect region;
	region.left = m_left + static_cast<double>( xindex ) * m_hlength;
	region.right = m_left + ( static_cast<double>( xindex ) + xbins ) * m_hlength;
	region.right = min( region.right, m_right );
	
	for( int yindex = 0 ; yindex < m_ydim - 1 ; yindex += ystep )
	{
	    region.bottom = m_bottom + static_cast<double>( yindex ) * m_vlength;
	    region.top = m_bottom + ( static_cast<double>( yindex ) + ybins ) * m_vlength;
	    region.top = min( region.top, m_top );

	    regions.push_back( region );
	}
    }

    //Purturb the regions to randon order
    const int purturbance = 3 * regions.size();
    for( int i = 0 ; i < purturbance ; i++ )
    {
	int k1 = rand() % regions.size();
	int k2 = rand() % regions.size();

	CRect tmp = regions[ k1 ];
	regions[ k1 ] = regions[ k2 ];
	regions[ k2 ] = tmp;
    }

    bool result = false;

    for( unsigned int i = 0 ; i < regions.size() ; i++ )
    {
	result |= SolveRegionOnce( size, regions[i] );
    }

    return result;
    
}

bool CCongMatching::SolveRegionOnce( const int& size, const CRect& region )
{
    SetRegionalModuleOrder( region );
    bool result = false;
    
    result = SolveOnce( 500, region );
    
#if 0
    for( int i = 0 ; i < 100 ; i++ )
    {
	result |= SolveOnce( 500 );
	
	printf(".\n");
	if( i % 5 == 0 )
	{
	    printf( "%d\n", i );
	}
#if 0
	//test code
	CCheckLegal clegal( m_placedb );
	clegal.check();
	//@test code
#endif
    }
#endif
    return result;
}

bool CCongMatching::SolveOnce( const int& size, const CRect& region )
{
    bool result = false;
   
#if 1 
    const int ITERATION = 5;
    int bestCost, origCost;

    do
    {
	//origCost: the best cost within the loop
	origCost = GetBinTotalCost();
	//origCost = ComputeModuleOverlap( m_module_order );
	//origCost = GetMaxBinCount( region );
	bestCost = origCost;
	for( int i = 0 ; i < ITERATION ; i++ )
	{
	    vector<int> module_ids;
	    vector<CPoint> positions;
	    FindModulesAndPositions( size, module_ids, positions );
#if 0 
	    //test code
	    printf( "m_module_order.size(): %d\n", m_module_order.size() );
	    printf( "******Find results\n" );
	    for( unsigned int i = 0 ; i < module_ids.size() ; i++ )
	    {
		const Module& curModule = m_placedb.m_modules[ module_ids[i] ];
		printf( "%s (%d) width %.2f pos (%.2f,%.2f) right %.2f\n", 
			curModule.m_name.c_str(),
			module_ids[i],
			curModule.m_width,
			curModule.m_x,
			curModule.m_y,
			FindRightBound( module_ids[i] ) );
	    }
	    printf( "******End of results\n" );
	    //@test code
#endif

	    SaveStatus( module_ids, positions );
	    vector<int> matching_results;
	    DoMatching( module_ids, positions, matching_results );
	    
	    //test code
	    printf( "oHPWL %f\n", m_placedb.CalcHPWL() );
	    //@test code
	    ApplyAssignment( module_ids, positions, matching_results );

	    int newCost = GetBinTotalCost();
	    //int newCost = ComputeModuleOverlap( m_module_order );
	    //int newCost = GetMaxBinCount( region );

	    printf( "origCost %d newCost %d\n", origCost, newCost );
	    //test code
	    printf( "nHPWL %f\n", m_placedb.CalcHPWL() );
	    //@test code

	    if( origCost <= newCost )
	    {
		RestoreStatus();
	    }
	    else
	    {
		result = true;
		origCost = newCost;
	    }
	}
	
    }while( origCost < bestCost );
    
#endif

    return result;
}

void CCongMatching::AddNetToBins( const Net& n )
{
    CRect bbox = ComputeNetBBox( n );

    int left_bin_index = GetHBinIndex( bbox.left );
    int right_bin_index = GetHBinIndex( bbox.right );
    int bottom_bin_index = GetVBinIndex( bbox.bottom );
    int top_bin_index = GetVBinIndex( bbox.top );
    
    for( int i = left_bin_index ; i <= right_bin_index ; i++ )
    {
	for( int j = bottom_bin_index ; j <= top_bin_index ; j++ )
	{
	    m_net_bins[i][j]++;
	    //test code
	    //printf( "m_net_bins[%d][%d]++;\n", i, j );
	    //@test code
	}
    }
    
}

void CCongMatching::RemoveNetFromBins( const Net& n )
{
    CRect bbox = ComputeNetBBox( n );

    int left_bin_index = GetHBinIndex( bbox.left );
    int right_bin_index = GetHBinIndex( bbox.right );
    int bottom_bin_index = GetVBinIndex( bbox.bottom );
    int top_bin_index = GetVBinIndex( bbox.top );


    for( int i = left_bin_index ; i <= right_bin_index ; i++ )
    {
	for( int j = bottom_bin_index ; j <= top_bin_index ; j++ )
	{
	    m_net_bins[i][j]--;
	    //test code
	    //printf( "m_net_bins[%d][%d]--;\n", i, j );
	    //@test code
	}
    }
    
}

void CCongMatching::ApplyAssignment( const vector<int>& module_ids,
	const vector<CPoint>& positions,
	const vector<int>& assignment )
{
    //Store all modules' segment position indexex for later update
    vector<CSegmentPositionIndex> index_bak;
    index_bak.reserve( module_ids.size() ); 
    for( unsigned int iMod = 0 ; iMod < module_ids.size() ; iMod++ )
    {
	const int mid = module_ids[ iMod ];
	map<int, CSegmentPositionIndex>::iterator iteFind = 
	    m_segment_position_map.find( mid );
	if( iteFind == m_segment_position_map.end() ) 
	{
	    fprintf( stderr, "Warning: module %s (%d) is not found in m_segment_position_map\n", 
		    m_placedb.m_modules[ mid ].m_name.c_str(),
		    mid );
	}
	index_bak.push_back( iteFind->second );
    }
    
    for( unsigned int iMod = 0 ; iMod < module_ids.size() ; iMod++ )
    {
	const int mid = module_ids[ iMod ];
	const Module& curModule = m_placedb.m_modules[ mid ];
	for( unsigned int iNet = 0 ; iNet < curModule.m_netsId.size() ; iNet++ )
	{
	    const int netid = curModule.m_netsId[ iNet ];
	    const Net& curNet = m_placedb.m_nets[ netid ];
	    RemoveNetFromBins( curNet );
	}

	const int posId = assignment[ iMod ];
	const CPoint& curPos = positions[ posId ];
	m_placedb.SetModuleLocation( mid, curPos.x, curPos.y );

	for( unsigned int iNet = 0 ; iNet < curModule.m_netsId.size() ; iNet++ )
	{
	    const int netid = curModule.m_netsId[ iNet ];
	    const Net& curNet = m_placedb.m_nets[ netid ];
	    AddNetToBins( curNet );
	}
    }
    
    //Update m_segment_position_map and m_segments
    for( unsigned int iMod = 0 ; iMod < module_ids.size() ; iMod++ )
    {
	const int mid = module_ids[ iMod ];
	const int posId = assignment[ iMod ];
	const int mid2 = module_ids[ posId ];

	if( mid == mid2 )
	{
	    continue;
	}

	const CSegmentPositionIndex& mid2_index = index_bak[ posId ];

	m_segments[ mid2_index.m_segId ].m_module_ids[ mid2_index.m_orderId ] = mid;
	m_segment_position_map[ mid ] = mid2_index;
    }
}

void CCongMatching::SetRegionalModuleOrder( const CRect& region )
{
    const double left = region.left;
    const double right = region.right;
    const double bottom = region.bottom;
    const double top = region.top;
    
    m_module_order.clear();
    for( int i = 0 ; i < static_cast<int>( m_placedb.m_modules.size() ) ; i++ )
    {
	const Module& curModule = m_placedb.m_modules[i];
	if( curModule.m_height > m_placedb.m_rowHeight || 
		curModule.m_isFixed )
	{
	    continue;
	}
	if( curModule.m_width == 0 )
	{
	    continue;
	}
	
	if( curModule.m_cx < left || curModule.m_cx > right ||
		curModule.m_cy < bottom || curModule.m_cy > top )
	{
	    continue;
	}
    	
	m_module_order.push_back( i );
    }
    
    CompareModuleWidth::m_pMod = &(m_placedb.m_modules);
    sort( m_module_order.begin(), m_module_order.end(), CompareModuleWidth() );
    
}

int CCongMatching::GetMaxBinCount( const CRect& region )
{
    int left_index = GetHBinIndex( region.left );
    int right_index = GetHBinIndex( region.right );
    int bottom_index = GetVBinIndex( region.bottom );
    int top_index = GetVBinIndex( region.top );

    int cost = m_net_bins[ left_index ][ bottom_index ];
    for( int i = left_index ; i <= right_index ; i++ )
    {
	for( int j = bottom_index ; j <= top_index ; j++ )
	{
	    cost = max( cost, m_net_bins[i][j] );
	}
    }

    return cost;
}
