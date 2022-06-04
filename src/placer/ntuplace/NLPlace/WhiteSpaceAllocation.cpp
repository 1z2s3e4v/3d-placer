#include "WhiteSpaceAllocation.h"
#include "placedb.h"
#include "GlobalRouting.h"
#include "CongMap.h"
#include "placeutil.h"
#include "placebin.h"
#include <cmath>


using namespace std;
using namespace Jin;

WhiteSpaceAllocation::WhiteSpaceAllocation( CPlaceDB& placedbref, int level) :
    m_placedb( placedbref )
{
    m_level = max(2, level);

    //test code
    //printf("******level: %d\n", level);
    //@test code
    
    //# of partitions = (2^m_level) - 1
    int num_partitions = static_cast<int>( pow( 2.0, m_level ) - 1 );
    m_partitions.reserve( num_partitions ); 

    //Decide the cut type of the top partition;
    WSAPartition::CutType firstCutType;
    double top_partition_width = abs( m_placedb.m_coreRgn.left - m_placedb.m_coreRgn.right );
    double top_partition_height = abs( m_placedb.m_coreRgn.top - m_placedb.m_coreRgn.bottom );
    if( top_partition_width > top_partition_height )
	firstCutType = WSAPartition::VCut;
    else
	firstCutType = WSAPartition::HCut;
    
    //Construct the top partition
    m_partitions.push_back( WSAPartition( m_placedb.m_coreRgn, false, firstCutType ) );
    
    //Those partitions with index >= m_first_bottom_partition_index are bottom ones
    m_first_bottom_partition_index = static_cast<int>( pow( static_cast<double>(2), m_level - 1 ) - 1 );
    
    //Construct all child partitions
    for( int i = 1 ; i < num_partitions ; i++ )
    {
	const WSAPartition& parent = m_partitions[ GetParentIndex( i ) ];

	//Compute the bounding box and the cut type of the child parition
	double child_left, child_right, child_top, child_bottom;
	child_left = parent.m_bbox.left;
	child_right = parent.m_bbox.right;
	child_top = parent.m_bbox.top;
	child_bottom = parent.m_bbox.bottom;
	WSAPartition::CutType child_cutType;

	//The parent partition contains a HCut
	if( parent.m_cutType == WSAPartition::HCut )
	{
	    child_cutType = WSAPartition::VCut;

	    //First child is the bottom one
	    if( IsFirstChild( i ) )
	    {
		child_top = ( parent.m_bbox.top + parent.m_bbox.bottom ) / 2.0;
	    }
	    //Second child is top one
	    else
	    {
		child_bottom = ( parent.m_bbox.top + parent.m_bbox.bottom ) / 2.0;
	    }
	}
	//The parent partition contains a VCut
	else
	{
	    child_cutType = WSAPartition::HCut;

	    //First child is the left one
	    if( IsFirstChild( i ) )
	    {
		child_right = ( parent.m_bbox.left + parent.m_bbox.right ) / 2.0;
	    }
	    else
	    {
		child_left = ( parent.m_bbox.left + parent.m_bbox.right ) / 2.0;
	    }
	    //Second child is the right one
	}
	
	//Determine if the child is a bottom partition
	bool child_isBottom;
	
	if( i >= m_first_bottom_partition_index )
	{
	    child_isBottom = true;
	}
	else
	{
	    child_isBottom = false;
	}

	m_partitions.push_back( WSAPartition( 
				CRect( child_top, child_bottom, child_left, child_right ), 
				child_isBottom, 
				child_cutType ) );
    }

    //Set bottom partition width and height
    m_bottom_partition_width = m_partitions[m_first_bottom_partition_index].m_bbox.right - 
	m_partitions[m_first_bottom_partition_index].m_bbox.left;
    m_bottom_partition_height = m_partitions[m_first_bottom_partition_index].m_bbox.top - 
	m_partitions[m_first_bottom_partition_index].m_bbox.bottom;

    //First cut is HCut
    if( m_partitions.front().m_cutType == WSAPartition::HCut )
    {
	m_array_x_dimension = static_cast<int>( pow( 2.0, (m_level-1)/2 ) );
	m_array_y_dimension = static_cast<int>( pow( 2.0, m_level/2 ) );
    }
    //VCut
    else
    {
	m_array_x_dimension = static_cast<int>( pow( 2.0, m_level/2 ) );
	m_array_y_dimension = static_cast<int>( pow( 2.0, (m_level-1)/2 ) );
    }
    
    m_bottom_index_array.resize( m_array_x_dimension );
    
    //test code
    //printf("level: %d\n", level );
    //printf("xdim: %d ydim: %d\n", m_array_x_dimension, m_array_y_dimension );
    //@test code
    for( int i = 0 ; i < m_array_x_dimension ; i++ )
    {
	m_bottom_index_array[i].resize( m_array_y_dimension, numeric_limits<int>::min() );
    }
    
    //Record the base x and y coordinates
    m_base_x = m_partitions.front().m_bbox.left;
    m_base_y = m_partitions.front().m_bbox.bottom;
    
    //Write the indexes of bottom partitions into m_bottom_index_array
    for( unsigned int pIndex = static_cast<unsigned int>( m_first_bottom_partition_index ) ; 
	    pIndex < m_partitions.size() ; pIndex++ )
    {
	//Compute the corresponding array index by the center coordinate of each bottom partition
	double cx = ( m_partitions[ pIndex ].m_bbox.left + m_partitions[ pIndex ].m_bbox.right ) / 2.0;
	double cy = ( m_partitions[ pIndex ].m_bbox.top + m_partitions[ pIndex ].m_bbox.bottom ) / 2.0;

	int xIndex = static_cast<int>( ( cx - m_base_x ) / m_bottom_partition_width );
	int yIndex = static_cast<int>( ( cy - m_base_y ) / m_bottom_partition_height );

	m_bottom_index_array[ xIndex ][ yIndex ] = pIndex; 
    }
    //@Initialize m_bottom_index_array


#if 0
    //test code
    printf("m_bottom_index_array content:\n");
    for( unsigned int j = 0 ; j < m_bottom_index_array[0].size() ; j++ )
    {
	for( unsigned int i = 0 ; i < m_bottom_index_array.size() ; i++ )
	{
	   printf( "%3d ", m_bottom_index_array[i][j] ); 
	}
	printf("\n");
    }
    //@test code 
#endif
    
    //Assign modules to bottom partitions or add them into m_fixed_block_ids
    for( unsigned int i = 0 ; i < m_partitions.size() ; i++ )
    {
	m_partitions[i].m_module_ids.clear();
    }
    m_fixed_block_ids.clear();
    
    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	const Module& curModule = m_placedb.m_modules[i];
	
	if( curModule.m_isFixed == true || curModule.m_height > m_placedb.m_rowHeight )
	{
	    m_fixed_block_ids.push_back( i );
	}
	else
	{
	    const int pIndex = ComputeCorrespondingBottomPartitionIndex( curModule.m_x, curModule.m_y );
	    m_partitions[pIndex].m_module_ids.push_back( i );
	}
    }

}


int WhiteSpaceAllocation::ComputeCorrespondingBottomPartitionIndex( const CPoint& p )
{
    return ComputeCorrespondingBottomPartitionIndex( p.x, p.y ); 
}


int WhiteSpaceAllocation::ComputeCorrespondingBottomPartitionIndex( const double& x, const double& y )
{
    int xIndex = static_cast<int>( ( x - m_base_x ) / m_bottom_partition_width );
    int yIndex = static_cast<int>( ( y - m_base_y ) / m_bottom_partition_height );

    xIndex = max( 0, xIndex );
    xIndex = min( m_array_x_dimension - 1, xIndex );
    yIndex = max( 0, yIndex );
    yIndex = min( m_array_y_dimension - 1, yIndex );
    
    return m_bottom_index_array[ xIndex ][ yIndex ]; 
}

double WhiteSpaceAllocation::ComputeFreeSpace( const CRect& rect, const double& util )
{
    return ComputeFreeSpace( rect.left, rect.right, rect.bottom, rect.top, util );
}

//TODO: Now scan all fixed blocks once 
//Can speedup by some other algorithm (scanline?)
double WhiteSpaceAllocation::ComputeFreeSpace( const double& left, 
	const double& right, 
	const double& bottom, 
	const double& top, const double& util )
{
    CRect region;
    region.left = max( left, m_placedb.m_coreRgn.left );
    region.right = min( right, m_placedb.m_coreRgn.right );
    region.bottom = max( bottom, m_placedb.m_coreRgn.bottom );
    region.top = min( top, m_placedb.m_coreRgn.top );

    double white_space = (region.right - region.left) * (region.top - region.bottom);
    
    //Compute the white space
    for( unsigned int i = 0 ; i < m_fixed_block_ids.size() ; i++ )
    {
	const int curId = m_fixed_block_ids[i];
	const Module& curModule = m_placedb.m_modules[curId];

	CRect module_rect;
	module_rect.left = curModule.m_x;
	module_rect.right = curModule.m_x + curModule.m_width;
	module_rect.bottom = curModule.m_y;
	module_rect.top = curModule.m_y + curModule.m_height;

	white_space -= ( ComputeOverlapArea( region, module_rect ) / util );
    }

#if 0
    if( white_space < -0.001 )
    {
	fprintf( stderr, "Warning: negative free_space %f\n", white_space );
    }
#endif
    
    white_space = max( 0.0, white_space );

    return white_space;
}

double WhiteSpaceAllocation::ComputeFixedBlockArea( const CRect& rect )
{
    return ComputeFixedBlockArea( rect.left, rect.right, rect.bottom, rect.top );
}

//TODO: Now scan all fixed blocks once 
//Can speedup by some other algorithm (scanline?)
double WhiteSpaceAllocation::ComputeFixedBlockArea( const double& left, 
	const double& right, 
	const double& bottom, 
	const double& top )
{
    CRect region;
    region.left = max( left, m_placedb.m_coreRgn.left );
    region.right = min( right, m_placedb.m_coreRgn.right );
    region.bottom = max( bottom, m_placedb.m_coreRgn.bottom );
    region.top = min( top, m_placedb.m_coreRgn.top );

    double fixed_block_area = 0.0;
    
    //Compute the white space
    for( unsigned int i = 0 ; i < m_fixed_block_ids.size() ; i++ )
    {
	const Module& curModule = m_placedb.m_modules[i];

	CRect module_rect;
	module_rect.left = curModule.m_x;
	module_rect.right = curModule.m_x + curModule.m_width;
	module_rect.bottom = curModule.m_y;
	module_rect.top = curModule.m_y + curModule.m_height;

	fixed_block_area += ComputeOverlapArea( region, module_rect );
    }

    return fixed_block_area; 
}

double WhiteSpaceAllocation::ComputeOverlapArea( const CRect& r1, const CRect& r2 )
{
    //No overlap
    if( r1.right <= r2.left ||
	r1.left >= r2.right ||
	r1.top <= r2.bottom ||
	r1.bottom >= r2.top )
    {
	return 0.0;
    }

    double overlap_top = min( r1.top, r2.top );
    double overlap_bottom = max( r1.bottom, r2.bottom );
    double overlap_left = max( r1.left, r2.left );
    double overlap_right = min( r1.right, r2.right );

    double overlap_area = (overlap_top - overlap_bottom) * (overlap_right - overlap_left );

    if( overlap_area < 0 )
    {
	fprintf( stderr, "Warning: negative overlap area %f\n", overlap_area );
    }

    return overlap_area;
}

#define _WSA_DEBUG_ 0

void WhiteSpaceAllocation::SolveDensity( const double util, const bool haveCluster )
{
    //Assign modules to bottom partitions or add them into m_fixed_block_ids
    for( unsigned int i = 0 ; i < m_partitions.size() ; i++ )
    {
	m_partitions[i].m_module_ids.clear();
	m_partitions[i].m_new_bbox = m_partitions[i].m_bbox;
    }
    m_fixed_block_ids.clear();
    
    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	const Module& curModule = m_placedb.m_modules[i];
	
	//fixed blocks
	if( curModule.m_isFixed == true )
	{
	    m_fixed_block_ids.push_back( i );
	}
	//macros
	else if( curModule.m_height > m_placedb.m_rowHeight && haveCluster == false )
	{
	    m_fixed_block_ids.push_back( i );
	}
	//standard cells
	else
	{
	    const int pIndex = ComputeCorrespondingBottomPartitionIndex( curModule.m_x, curModule.m_y );
	    m_partitions[pIndex].m_module_ids.push_back( i );
	}
    }

    //Compute the total cell area and total white space for bottom partitions
    for( unsigned int i = static_cast<unsigned int>( m_first_bottom_partition_index ) ;
	    i < m_partitions.size() ; i++ )
    {
	
	WSAPartition& curPartition = m_partitions[i];

	double total_free_space = ComputeFreeSpace( curPartition.m_bbox );
	//double total_free_space = ( curPartition.m_bbox.right - curPartition.m_bbox.left ) *
	//			  ( curPartition.m_bbox.top - curPartition.m_bbox.bottom );

	double cell_area = 0.0;
	for( unsigned int j = 0 ; j < curPartition.m_module_ids.size() ; j++ )
	{
	    const int& curId = curPartition.m_module_ids[j];
	    const Module& curModule = m_placedb.m_modules[ curId ];

	    cell_area += (curModule.m_width * curModule.m_height);
	}
	//double fixed_block_area = ComputeFixedBlockArea( curPartition.m_bbox );

	curPartition.m_cell_area = cell_area;
	curPartition.m_white_space = total_free_space - ( cell_area / util );

#if _WSA_DEBUG_ 
	fprintf( stdout, "%3d (%.2f,%.2f) ca %.2f fa %.2f ws %.2f util %.2f\n", 
		i,
		curPartition.m_bbox.right - curPartition.m_bbox.left,
		curPartition.m_bbox.top - curPartition.m_bbox.bottom,
		curPartition.m_cell_area,
		total_free_space,
		curPartition.m_white_space,
		curPartition.m_cell_area / total_free_space );
#endif
    }
   
    //Bottom-up process:
    //Compute the white space and cell area from child partitions
    for( int i = m_first_bottom_partition_index - 1 ;
	    i >= 0 ; i-- )
    {
	WSAPartition& curPartition = m_partitions[i];
	int childIndex = GetFirstChildIndex( i );

	//test code
	//printf( "child index: %d\n", childIndex );
	//fflush( stdout );
	//@test code
	
	curPartition.m_white_space = m_partitions[ childIndex ].m_white_space + 
				     m_partitions[ childIndex + 1 ].m_white_space;
	
	curPartition.m_cell_area = m_partitions[ childIndex ].m_cell_area + 
				     m_partitions[ childIndex + 1 ].m_cell_area;
	
    }
   
    //Top-down process:
    //For each parent partition, determine the new bounding box for every child partition 
    //based on their white space and cell area
    m_partitions[0].m_new_bbox =  m_partitions[0].m_bbox;
    for( unsigned int i = 0 ; 
	    i < static_cast<unsigned int>( m_first_bottom_partition_index ) ; 
	    i++ )
    {
	WSAPartition& curPartition = m_partitions[i];
	
	int firstChildIndex = GetFirstChildIndex( i );

	//re-compute m_white_space for child partitions according to
	//curPartition.m_new_bbox
	for( int icc = firstChildIndex ; icc < firstChildIndex + 2 ; icc++ )
	{
	    WSAPartition& ccPart = m_partitions[ icc ];
	    //WSAPartition& ccParent = m_partitions[ GetParentIndex( icc ) ];

	    CRect ccBBox = curPartition.m_new_bbox;

	    if( curPartition.m_cutType == WSAPartition::HCut )
	    {
		if( IsFirstChild( icc ) )
		{
		    ccBBox.top = ( ccBBox.top + ccBBox.bottom ) / 2.0;
		}
		else
		{
		    ccBBox.bottom = ( ccBBox.top + ccBBox.bottom ) / 2.0;
		}
	    }
	    else
	    {
		if( IsFirstChild( icc ) )
		{
		    ccBBox.right = ( ccBBox.left + ccBBox.right ) / 2.0;
		}
		else
		{
		    ccBBox.left = ( ccBBox.left + ccBBox.right ) / 2.0;
		}
	    }


	    double cc_free_space = ComputeFreeSpace( ccBBox );
	    ccPart.m_white_space = cc_free_space - ( ccPart.m_cell_area / util );
	   
#if _WSA_DEBUG_ 
	    //test code
	    fprintf( stdout, "set (%d) fs %.2f ws %.2f ca %.2f util %.2f\n", icc, cc_free_space, ccPart.m_white_space,
		  ccPart.m_cell_area, util );
	    fflush( stdout );
	    //@test code
#endif
	}
	    
	WSAPartition& firstChild = m_partitions[ firstChildIndex ];
	WSAPartition& secondChild = m_partitions[ firstChildIndex + 1 ];
	
	double cutline_position;
	
	double total_white_space = firstChild.m_white_space + secondChild.m_white_space;
	double total_free_space = ComputeFreeSpace( curPartition.m_new_bbox );

	double target_free_space;
	double allowable_error = 0.005;
	
#if _WSA_DEBUG_
	if( total_white_space < -1.0 )
	{
	    fprintf( stderr, "Warning: not sufficient white space %.2f in partition %d (cutil %.2f)\n", 
		    total_white_space, i, util );
	    fflush( stderr );
	}
#endif
	
	//If the parent parition do not contain any cell,
	//all children must contain no cell
	if( curPartition.m_cell_area == 0.0 )
	{
#if _WSA_DEBUG_ 
	    //test code
	    fprintf( stdout, "(%3d) no cell\n", i );
	    fflush( stdout );
	    //@test code
#endif	    
	    continue;
	}
	else if( firstChild.m_cell_area == 0.0 )
	{
	    //If there exists no white space, allocate all free space to second child
	    if( total_white_space <= 0.0 )
	    {
		//Approximate the cutline to leftmost position with maximum free space
		target_free_space = 0.01;
		allowable_error = 0.005;
	    }
	    //Allocate white space to meet density target of second child
	    else
	    {
		double second_child_target_free_space = secondChild.m_cell_area / util + 
		    ( secondChild.m_white_space > 0.0 ? secondChild.m_white_space : 0.0 );
		target_free_space = total_free_space - second_child_target_free_space;
	    }
	}
	else if( secondChild.m_cell_area == 0.0 )
	{
	    //If there exists no white space, allocate all free space to first child
	    if( total_white_space <= 0.0 )
	    {
		target_free_space = total_free_space - 0.01;
		allowable_error = 0.005;
	    }
	    //Allocate white space to meet density target of second child
	    else
	    {
		target_free_space = firstChild.m_cell_area / util + 
		    ( firstChild.m_white_space > 0.0 ? firstChild.m_white_space : 0.0 );
	    }
	}
	//If there exists no white space
	//allocate free space according to cell area ratio
	else if( total_white_space <= 0 )
	{
	    target_free_space = total_free_space * ( firstChild.m_cell_area / 
		( firstChild.m_cell_area + secondChild.m_cell_area ) );
	    
	}
	//Both partitions have white sapce > 0
	//Allocate white space propotional to their original white space
	else if( firstChild.m_white_space > 0.0 && secondChild.m_white_space > 0.0 )
	{
	    double target_white_space = total_white_space * ( firstChild.m_white_space /
		    ( firstChild.m_white_space + secondChild.m_white_space ) );
	    target_free_space = ( firstChild.m_cell_area / util ) + target_white_space;
	    
	}
//No such situation
#if 0
	
	//Both partitions have white space <= 0
	//Allocate white space equal to first child's cell area
	else if( firstChild.m_white_space <= 0.0 && secondChild.m_white_space <= 0.0 )
	{
	    cutline_position = ComputeCutLinePositionToMeetFirstChildUtil( curPartition.m_new_bbox,
									   curPartition.m_cutType,
									   firstChild.m_cell_area,
									   util );
	}
#endif
	//One of the partitions have white space <= 0
	//Let the free space of the partition equal the cell area / util
	else
	{
	    if( firstChild.m_white_space <= 0.0 )
	    {
		target_free_space = firstChild.m_cell_area / util;
	    }
	    else
	    {
		double second_child_target_free_space = secondChild.m_cell_area / util;
		target_free_space = total_free_space - second_child_target_free_space;
	    }
	    
	}

#if _WSA_DEBUG_
	if( target_free_space < -0.1 )
	{
	    fprintf( stderr, "Warning: incorrect target_free_space %f\n", target_free_space );
	}
#endif

	
	//Cut-off incorrect target_free_space generated by computation error
	target_free_space = max( 0.01, target_free_space );
	
	cutline_position = ComputeCutLinePosition( curPartition.m_new_bbox,
		curPartition.m_cutType,
		target_free_space,
		allowable_error );
   
	//Determine the new bbox for child partitions
	if( curPartition.m_cutType == WSAPartition::HCut )
	{
	    //Bottom one
	    firstChild.m_new_bbox = curPartition.m_new_bbox;
	    firstChild.m_new_bbox.top = cutline_position;
#if _WSA_DEBUG_ 
	    //test code
	    fprintf( stdout, "(%3d) b %.2f c %.2f t %.2f l %.2f r %.2f ", 
		    i,
		    curPartition.m_new_bbox.bottom,
		    cutline_position,
		    curPartition.m_new_bbox.top,
		    curPartition.m_new_bbox.left,
		    curPartition.m_new_bbox.right );
	    fprintf( stdout, "w1 %.2f w2 %.2f c1 %.2f c2 %.2f tf %.2f\n", 
		    firstChild.m_white_space,
		    secondChild.m_white_space,
		    firstChild.m_cell_area,
		    secondChild.m_cell_area,
		    target_free_space );
	    fflush( stdout );
	    //@test code
#endif	    
	    //Top one
	    secondChild.m_new_bbox = curPartition.m_new_bbox;
	    secondChild.m_new_bbox.bottom = cutline_position;
	}
	//VCut
	else
	{
	    //Left one
	    firstChild.m_new_bbox = curPartition.m_new_bbox;
	    firstChild.m_new_bbox.right = cutline_position;

#if _WSA_DEBUG_	    
	    //test code
	    fprintf( stdout, "(%3d) l %.2f c %.2f r %.2f b %.2f t %.2f ", 
		    i,
		    curPartition.m_new_bbox.left,
		    cutline_position,
		    curPartition.m_new_bbox.right,
		    curPartition.m_new_bbox.bottom,
		    curPartition.m_new_bbox.top );
	    fprintf( stdout, "w1 %.2f w2 %.2f c1 %.2f c2 %.2f tf %.2f\n", 
		    firstChild.m_white_space,
		    secondChild.m_white_space,
		    firstChild.m_cell_area,
		    secondChild.m_cell_area,
		    target_free_space );
	    fflush( stdout );
	    //@test code
#endif
	    
	    //Right one
	    secondChild.m_new_bbox = curPartition.m_new_bbox;
	    secondChild.m_new_bbox.left = cutline_position;
	}

#if _WSA_DEBUG_ 
	//test code
	fprintf( stdout, "fs1 %.2f fs2 %.2f\n", 
		ComputeFreeSpace( firstChild.m_new_bbox ),
		ComputeFreeSpace( secondChild.m_new_bbox ) );
	fflush( stdout );
	//@test code
#endif
    }
    //@Top-down process
   
#if 0 
    //test code
    double total_part_area = 0.0;
    for( unsigned int i = static_cast<unsigned int>( m_first_bottom_partition_index ) ;
	    i < m_partitions.size() - 1; i++ )
    {
	WSAPartition& part1 = m_partitions[i];

	total_part_area += ( part1.m_new_bbox.right - part1.m_new_bbox.left ) *
	     ( part1.m_new_bbox.top - part1.m_new_bbox.bottom );
	
	for( unsigned int j = i + 1 ; j < m_partitions.size() ; j++ )
	{
	    WSAPartition& part2 = m_partitions[j];

	    double overlap_area = ComputeOverlapArea( part1.m_new_bbox, part2.m_new_bbox );

	    if( overlap_area != 0.0 )
	    {
		fprintf( stderr, "Warning: part %d overlap with part %d with area %.2f\n",
			i, j, overlap_area );
		fflush( stderr );
	    }
	}
    }

    fprintf( stdout, "total new partition area: %f\n", total_part_area );
    fflush( stdout );
    //@test code
#endif
    
    //Move all modules of bottom partitions to new bounding boxes
    for( unsigned int i = static_cast<unsigned int>( m_first_bottom_partition_index ) ;
	    i < m_partitions.size() ; i++ )
    {
	WSAPartition& curPartition = m_partitions[i];

	if( curPartition.m_cell_area == 0.0 )
	    continue;
	
	double orig_width = curPartition.m_bbox.right - curPartition.m_bbox.left;
	double orig_height = curPartition.m_bbox.top - curPartition.m_bbox.bottom;
	double new_width = curPartition.m_new_bbox.right - curPartition.m_new_bbox.left;
	double new_height = curPartition.m_new_bbox.top - curPartition.m_new_bbox.bottom;

	double width_ratio = new_width / orig_width;
	double height_ratio = new_height / orig_height;

	double orig_base_x = curPartition.m_bbox.left;
	double orig_base_y = curPartition.m_bbox.bottom;
	double new_base_x = curPartition.m_new_bbox.left;
	double new_base_y = curPartition.m_new_bbox.bottom;

	for( unsigned int iModule = 0 ; iModule < curPartition.m_module_ids.size() ; iModule++ )
	{
	    int ModuleIndex = curPartition.m_module_ids[ iModule ];
	    Module& curModule = m_placedb.m_modules[ ModuleIndex ];  
	    double new_x = ( curModule.m_x - orig_base_x ) * width_ratio + new_base_x;
	    double new_y = ( curModule.m_y - orig_base_y ) * height_ratio + new_base_y;

	    m_placedb.SetModuleLocation( ModuleIndex, new_x, new_y );
	}
#if _WSA_DEBUG_ 
	//test code
	fprintf( stdout, "%3d (%.2f,%.2f)-(%.2f,%.2f) to (%.2f,%.2f)-(%.2f,%.2f) # modules %d\n",
		i,
		curPartition.m_bbox.left, curPartition.m_bbox.bottom,
		curPartition.m_bbox.right, curPartition.m_bbox.top,
		curPartition.m_new_bbox.left, curPartition.m_new_bbox.bottom,
		curPartition.m_new_bbox.right, curPartition.m_new_bbox.top,
		curPartition.m_module_ids.size() );
	fflush( stdout );
	//@test code
#endif
    }
    
}

void WhiteSpaceAllocation::SolveDensityIteratively( const double& util )
{
    CPlaceBin pBin( m_placedb );
    pBin.CreateGrid( m_placedb.m_rowHeight * 10.0, m_placedb.m_rowHeight * 10.0 );
    pBin.UpdateBinUsage();
    //fprintf( stdout, "density penalty before wsa %f\n", pBin.GetPenalty( util ) );
    //fprintf( stdout, "HPWL= %.0f DHPWL= %.0f\n", m_placedb.GetHPWLp2p(), m_placedb.GetHPWLdensity( target_util ) );
    
    double parameter_util = util;
    if( gArg.CheckExist( "util" ) )
    {
	gArg.GetDouble( "util", &parameter_util );
    }


    double max_util = pBin.GetMaxUtil();
    //double max_util = ComputeMaxUtil();
    double previous_cost = m_placedb.GetHPWLdensity( parameter_util );
    
    const double step = 0.1;
    double current_util = max_util - step;
    bool bImprove = true;
    m_placedb.SaveBlockLocation();
    
    fprintf( stdout, "m_placedb.m_rowHeight * 10.0 = %.2f\n", m_placedb.m_rowHeight * 10.0 );
    fprintf( stdout, "mutil %.2f HPWL= %.0f DHPWL= %.0f\n", 
	    max_util, m_placedb.GetHPWLp2p(), m_placedb.GetHPWLdensity( parameter_util ) );
    fflush( stdout );
    
    while( current_util > util && bImprove )
    {
	SolveDensity( current_util );
	double current_cost = m_placedb.GetHPWLdensity( parameter_util );
    
	SolveDensity( current_util );
	
	fprintf( stdout, "cutil %.2f HPWL= %.0f DHPWL= %.0f\n", 
		current_util, m_placedb.GetHPWLp2p(), m_placedb.GetHPWLdensity( parameter_util ) );
	fflush( stdout );

	
	if( current_cost > previous_cost )
	{
	    bImprove = false;
	    m_placedb.RestoreBlockLocation();
	}
	else
	{
	    m_placedb.SaveBlockLocation();
	    previous_cost = current_cost;
	}
	
	current_util -= step;
    }
    fprintf( stdout, "stop\n" );

}
    
void WhiteSpaceAllocation::SolveDensityBest( const double& util )
{
    CPlaceBin pBin( m_placedb );
    pBin.CreateGrid( m_placedb.m_rowHeight * 10.0, m_placedb.m_rowHeight * 10.0 );
    pBin.UpdateBinUsage();
    //fprintf( stdout, "density penalty before wsa %f\n", pBin.GetPenalty( util ) );
    //fprintf( stdout, "HPWL= %.0f DHPWL= %.0f\n", m_placedb.GetHPWLp2p(), m_placedb.GetHPWLdensity( target_util ) );

    double max_util = 1.5;
    
    const double step = 0.05;
    m_placedb.SaveBlockLocation();
    m_placedb.SaveBestBlockLocation();
    double parameter_util = util;
    if( gArg.CheckExist( "util" ) )
    {
	gArg.GetDouble( "util", &parameter_util );
    }
    
    double best_cost = m_placedb.GetHPWLdensity( parameter_util );

    fprintf( stdout, "          HPWL= %.0f DHPWL= %.0f\n", 
	    m_placedb.GetHPWLp2p(), m_placedb.GetHPWLdensity( parameter_util ) );
    fflush( stdout );
    
    for( double current_util = util ; current_util <= max_util ; current_util += step )
    {
	SolveDensity( current_util );
	double current_cost = m_placedb.GetHPWLdensity( parameter_util );
    
	SolveDensity( current_util );
	
	fprintf( stdout, "cutil %.2f HPWL= %.0f DHPWL= %.0f\n", 
		current_util, m_placedb.GetHPWLp2p(), m_placedb.GetHPWLdensity( parameter_util ) );
	fflush( stdout );

	if( current_cost < best_cost )
	{
	    best_cost = current_cost;
	    m_placedb.SaveBestBlockLocation();
	}

	m_placedb.RestoreBlockLocation();	
    }

    m_placedb.RestoreBestBlockLocation();
    
    fprintf( stdout, "     best HPWL= %.0f DHPWL= %.0f\n", 
	    m_placedb.GetHPWLp2p(), m_placedb.GetHPWLdensity( parameter_util ) );
    fflush( stdout );

}

double WhiteSpaceAllocation::ComputeMaxUtil( void )
{
    double max_util = 0.0;
    //Compute the total cell area and total white space for bottom partitions
    for( unsigned int i = static_cast<unsigned int>( m_first_bottom_partition_index ) ;
	    i < m_partitions.size() ; i++ )
    {
	
	WSAPartition& curPartition = m_partitions[i];

	double total_free_space = ComputeFreeSpace( curPartition.m_bbox );
	//double total_free_space = ( curPartition.m_bbox.right - curPartition.m_bbox.left ) *
	//			  ( curPartition.m_bbox.top - curPartition.m_bbox.bottom );

	double cell_area = 0.0;
	for( unsigned int j = 0 ; j < curPartition.m_module_ids.size() ; j++ )
	{
	    const int& curId = curPartition.m_module_ids[j];
	    const Module& curModule = m_placedb.m_modules[ curId ];

	    cell_area += (curModule.m_width * curModule.m_height);
	}
	//double fixed_block_area = ComputeFixedBlockArea( curPartition.m_bbox );

	//fprintf( stdout, "cell area %.2f free space %.2f\n", cell_area, total_free_space );
	
	if( total_free_space > 0.0 )
	{
	    double util = cell_area / total_free_space;
	    max_util = max( util, max_util );
	}
	
    }

    return max_util;

}

//This version does not consider target utilization (util = 1.0)
#if 0
void WhiteSpaceAllocation::Solve( void )
{
    //Compute the total cell area and total white space for bottom partitions
    for( unsigned int i = static_cast<unsigned int>( m_first_bottom_partition_index ) ;
	    i < m_partitions.size() ; i++ )
    {
	
	WSAPartition& curPartition = m_partitions[i];

	double total_free_space = ComputeFreeSpace( curPartition.m_bbox );

	double cell_area = 0.0;
	for( unsigned int j = 0 ; j < curPartition.m_module_ids.size() ; j++ )
	{
	    const int& curId = curPartition.m_module_ids[j];
	    const Module& curModule = m_placedb.m_modules[ curId ];

	    cell_area += (curModule.m_width * curModule.m_height);
	}

	curPartition.m_cell_area = cell_area;
	curPartition.m_white_space = total_free_space - cell_area;

    }
   
    //Bottom-up process:
    //Compute the white space and cell area from child partitions
    for( int i = m_first_bottom_partition_index - 1 ;
	    i >= 0 ; i-- )
    {
	WSAPartition& curPartition = m_partitions[i];
	int childIndex = GetFirstChildIndex( i );

	//test code
	//printf( "child index: %d\n", childIndex );
	//fflush( stdout );
	//@test code
	
	curPartition.m_white_space = m_partitions[ childIndex ].m_white_space + 
				     m_partitions[ childIndex + 1 ].m_white_space;
	
	curPartition.m_cell_area = m_partitions[ childIndex ].m_cell_area + 
				     m_partitions[ childIndex + 1 ].m_cell_area;
	
    }
    
    //Top-down process:
    //For each parent partition, determine the new bounding box for every child partition 
    //based on their white space and cell area
    m_partitions[0].m_new_bbox =  m_partitions[0].m_bbox;
    for( unsigned int i = 0 ; i < static_cast<unsigned int>( m_first_bottom_partition_index ) ; i++ )
    {
	WSAPartition& curPartition = m_partitions[i];
	int firstChildIndex = GetFirstChildIndex( i );
	WSAPartition& firstChild = m_partitions[ firstChildIndex ];
	WSAPartition& secondChild = m_partitions[ firstChildIndex + 1 ];

	double cutline_position;
	
	//Both partitions have white sapce > 0
	//Allocate white space propotional to their original white space
	if( firstChild.m_white_space >= 0.0 && secondChild.m_white_space >= 0.0 )
	{
	    double total_free_space = ComputeFreeSpace( curPartition.m_new_bbox );
	    double total_white_space = total_free_space - curPartition.m_cell_area;
	    double target_white_space_ratio = firstChild.m_white_space /
		( firstChild.m_white_space + secondChild.m_white_space );
	    double target_white_space = total_white_space * target_white_space_ratio;
	    double target_free_space = target_white_space + firstChild.m_cell_area;

	    cutline_position = ComputeCutLinePosition( curPartition.m_new_bbox, curPartition.m_cutType, target_free_space );
	}
	//Both partitions have white space < 0
	//Allocate white space equal to cell area
	else if( firstChild.m_white_space < 0.0 && secondChild.m_white_space < 0.0 )
	{
	    double target_free_space = firstChild.m_cell_area;
	    cutline_position = ComputeCutLinePosition( curPartition.m_new_bbox, curPartition.m_cutType, target_free_space );
	}
	//One of the partitions have white space < 0
	//Let the free space of the partition equal the cell area
	else
	{
	    double target_free_space;

	    if( firstChild.m_white_space < 0.0 )
	    {
		target_free_space = firstChild.m_cell_area;
	    }
	    else
	    {
		double total_free_space = ComputeFreeSpace( curPartition.m_new_bbox );
		target_free_space = total_free_space - secondChild.m_cell_area;
	    }
	    
	    cutline_position = ComputeCutLinePosition( curPartition.m_new_bbox, curPartition.m_cutType, target_free_space );
	}
   
	//Determine the new bbox for child partitions
	if( curPartition.m_cutType == WSAPartition::HCut )
	{
	    //Bottom one
	    firstChild.m_new_bbox = curPartition.m_new_bbox;
	    firstChild.m_new_bbox.top = cutline_position;
	    
	    //Top one
	    secondChild.m_new_bbox = curPartition.m_new_bbox;
	    secondChild.m_new_bbox.bottom = cutline_position;
	}
	//VCut
	else
	{
	    //Left one
	    firstChild.m_new_bbox = curPartition.m_new_bbox;
	    firstChild.m_new_bbox.right = cutline_position;
	    
	    //Right one
	    secondChild.m_new_bbox = curPartition.m_new_bbox;
	    secondChild.m_new_bbox.left = cutline_position;
	}
    }
    //@Top-down process
    
    //Move all modules of bottom partitions to new bounding boxes
    for( unsigned int i = static_cast<unsigned int>( m_first_bottom_partition_index ) ;
	    i < m_partitions.size() ; i++ )
    {
	WSAPartition& curPartition = m_partitions[i];
	
	double orig_width = curPartition.m_bbox.right - curPartition.m_bbox.left;
	double orig_height = curPartition.m_bbox.top - curPartition.m_bbox.bottom;
	double new_width = curPartition.m_new_bbox.right - curPartition.m_new_bbox.left;
	double new_height = curPartition.m_new_bbox.top - curPartition.m_new_bbox.bottom;

	double width_ratio = new_width / orig_width;
	double height_ratio = new_height / orig_height;

	double orig_base_x = curPartition.m_bbox.left;
	double orig_base_y = curPartition.m_bbox.bottom;
	double new_base_x = curPartition.m_new_bbox.left;
	double new_base_y = curPartition.m_new_bbox.bottom;

	for( unsigned int iModule = 0 ; iModule < curPartition.m_module_ids.size() ; iModule++ )
	{
	    int ModuleIndex = curPartition.m_module_ids[ iModule ];
	    Module& curModule = m_placedb.m_modules[ ModuleIndex ];  
	    double new_x = ( curModule.m_x - orig_base_x ) * width_ratio + new_base_x;
	    double new_y = ( curModule.m_y - orig_base_y ) * height_ratio + new_base_y;

	    m_placedb.SetModuleLocation( ModuleIndex, new_x, new_y );
	}
    }
}
#endif

//TODO: (finished?) Have a bug inside this function
//(should add the binary_search_top and binary_search_bottom for binary search)
double WhiteSpaceAllocation::ComputeCutLinePosition( const CRect& rect,
	                                             const WSAPartition::CutType& cutType, 
						     const double& target_free_space,
						     const double& allowable_error )
{
#if 0 
    //test code
    fprintf( stdout, "O l %.2f r %.2f t %.2f b %.2f type %d free_space %.2f\n",
    	  rect.left, rect.right, rect.top, rect.bottom, cutType, target_free_space );
    fflush( stdout );
    //@test code
#endif
    
    if( ComputeFreeSpace( rect ) < target_free_space )
    {
	if( cutType == WSAPartition::HCut )
	{
	    return ( rect.bottom + rect.top ) / 2.0;
	}
	else
	{
	    return ( rect.left + rect.right ) / 2.0;
	}
    }
    
    double cutline_position = 0.0;

    //Set the accuracy of binary search
    const double accuracy = 0.01;
    double space_error;
    
    if( allowable_error <= 0.0 )
    {
	space_error = accuracy * target_free_space;
    }
    else
    {
	space_error = allowable_error;
    }
    
    //Horizontal cut
    if( cutType == WSAPartition::HCut )
    {
	//Compute the first cutline position
	double partition_width = rect.right - rect.left;
	cutline_position = rect.bottom + ( target_free_space / partition_width );

	CRect binary_search_bbox;
	binary_search_bbox.left = rect.left;
	binary_search_bbox.right = rect.right;
	binary_search_bbox.bottom = rect.bottom;
	binary_search_bbox.top = cutline_position;

	double binary_search_free_space = ComputeFreeSpace( binary_search_bbox );
	
	//Record the boundary informations for binary search
	double binary_search_top = rect.top;
	double binary_search_bottom = rect.bottom;
	
	while( abs( binary_search_free_space - target_free_space ) > space_error )
	{
	    if( binary_search_free_space - target_free_space > space_error )
	    {
		binary_search_top = cutline_position;
		cutline_position = ( cutline_position + binary_search_bottom ) / 2.0;
	    }
	    else
	    {
		binary_search_bottom = cutline_position;
		cutline_position = ( cutline_position + binary_search_top ) / 2.0;
	    }

	    binary_search_bbox.top = cutline_position;
	    binary_search_free_space = ComputeFreeSpace( binary_search_bbox );
	} 
    }
    else
    {
	//Compute the first cutline position
	double partition_height = rect.top - rect.bottom;
	cutline_position = rect.left + ( target_free_space / partition_height );

	CRect binary_search_bbox;
	binary_search_bbox.left = rect.left;
	binary_search_bbox.bottom = rect.bottom;
	binary_search_bbox.top = rect.top;
	binary_search_bbox.right = cutline_position;

	double binary_search_free_space = ComputeFreeSpace( binary_search_bbox );

	//Record the boundary informations for binary search
	double binary_search_left = rect.left;
	double binary_search_right = rect.right;

	while( abs( binary_search_free_space - target_free_space ) > space_error )
	{
	    //test code
	    //fprintf( stdout, "bleft %.2f cutline_position %.2f bright %.2f\n",
	    //	    binary_search_left, cutline_position, binary_search_right );
	    //@test code
	    if( binary_search_free_space - target_free_space > space_error )
	    {
		binary_search_right = cutline_position;
		cutline_position = ( cutline_position + binary_search_left ) / 2.0;
	    }
	    else
	    {
		binary_search_left = cutline_position;
		cutline_position = ( cutline_position + binary_search_right ) / 2.0;
	    }

	    binary_search_bbox.right = cutline_position;
	    binary_search_free_space = ComputeFreeSpace( binary_search_bbox );
	} 

    }
#if 0 
    //test code
    fprintf( stdout, "X\n" );
    fflush( stdout );
    //@test code
#endif
    return cutline_position;
}

const double binary_search_accuracy = 0.01;

double WhiteSpaceAllocation::ComputeCutLinePositionToMeetFirstChildUtil( const CRect& rect,
	const Jin::WSAPartition::CutType& cutType,
	const double& first_child_cell_area,
	const double& util )
{
    double cutline_position = 0.0;

    //Set the allowable error of binary search
    const double error = binary_search_accuracy * util;
    
    //Horizontal cut
    if( cutType == WSAPartition::HCut )
    {
	//Compute the first cutline position
	double partition_width = rect.right - rect.left;
	cutline_position = rect.bottom + ( ( first_child_cell_area / util ) / partition_width );

	CRect binary_search_bbox;
	binary_search_bbox.left = rect.left;
	binary_search_bbox.right = rect.right;
	binary_search_bbox.bottom = rect.bottom;
	binary_search_bbox.top = cutline_position;

	double binary_search_bbox_area = ( binary_search_bbox.right - binary_search_bbox.left ) * 
					( binary_search_bbox.top - binary_search_bbox.bottom );
	double fixed_block_area = ComputeFixedBlockArea( binary_search_bbox );
	double found_util = ( first_child_cell_area + fixed_block_area ) / binary_search_bbox_area;

	//Record the boundary informations for binary search
	double binary_search_top = rect.top;
	double binary_search_bottom = rect.bottom;
	
	while( abs( found_util - util ) > error )
	{
	    if( found_util - util < error )
	    {
		binary_search_top = cutline_position;
		cutline_position = ( cutline_position + binary_search_bottom ) / 2.0;
	    }
	    else
	    {
		binary_search_bottom = cutline_position;
		cutline_position = ( cutline_position + binary_search_top ) / 2.0;
	    }


	    //Update binary search box
	    binary_search_bbox.top = cutline_position;
	    binary_search_bbox_area = ( binary_search_bbox.right - binary_search_bbox.left ) * 
		( binary_search_bbox.top - binary_search_bbox.bottom );
	    fixed_block_area = ComputeFixedBlockArea( binary_search_bbox );
	    found_util = ( first_child_cell_area + fixed_block_area ) / binary_search_bbox_area;
	} 
    }
    else
    {
	//Compute the first cutline position
	double partition_height = rect.top - rect.bottom;
	cutline_position = rect.left + ( ( first_child_cell_area / util ) / partition_height );

	CRect binary_search_bbox;
	binary_search_bbox.left = rect.left;
	binary_search_bbox.bottom = rect.bottom;
	binary_search_bbox.top = rect.top;
	binary_search_bbox.right = cutline_position;

	double binary_search_bbox_area = ( binary_search_bbox.right - binary_search_bbox.left ) * 
					( binary_search_bbox.top - binary_search_bbox.bottom );
	double fixed_block_area = ComputeFixedBlockArea( binary_search_bbox );
	double found_util = ( first_child_cell_area + fixed_block_area ) / binary_search_bbox_area;
	
	//Record the boundary informations for binary search
	double binary_search_left = rect.left;
	double binary_search_right = rect.right;

	while( abs( found_util - util ) > error )
	{
	    if( found_util - util < error )
	    {
		binary_search_right = cutline_position;
		cutline_position = ( cutline_position + binary_search_left ) / 2.0;
	    }
	    else
	    {
		binary_search_left = cutline_position;
		cutline_position = ( cutline_position + binary_search_right ) / 2.0;
	    }

	    //Update binary search box
	    binary_search_bbox.right = cutline_position;
	    binary_search_bbox_area = ( binary_search_bbox.right - binary_search_bbox.left ) * 
		( binary_search_bbox.top - binary_search_bbox.bottom );
	    fixed_block_area = ComputeFixedBlockArea( binary_search_bbox );
	    found_util = ( first_child_cell_area + fixed_block_area ) / binary_search_bbox_area;
	} 
    }
    
    return cutline_position;
}

double WhiteSpaceAllocation::ComputeCutLinePositionToMeetSecondChildUtil( const CRect& rect,
	const Jin::WSAPartition::CutType& cutType,
	const double& second_child_cell_area,
	const double& util )
{
    double cutline_position = 0.0;

    //Set the allowable error of binary search
    const double error = binary_search_accuracy * util;
    
    //Horizontal cut
    if( cutType == WSAPartition::HCut )
    {
	//Compute the first cutline position
	double partition_width = rect.right - rect.left;
	cutline_position = rect.top - ( ( second_child_cell_area / util ) / partition_width );

	CRect binary_search_bbox;
	binary_search_bbox.left = rect.left;
	binary_search_bbox.right = rect.right;
	binary_search_bbox.top = rect.top;
	binary_search_bbox.bottom = cutline_position; 

	double binary_search_bbox_area = ( binary_search_bbox.right - binary_search_bbox.left ) * 
					( binary_search_bbox.top - binary_search_bbox.bottom );
	double fixed_block_area = ComputeFixedBlockArea( binary_search_bbox );
	double found_util = ( second_child_cell_area + fixed_block_area ) / binary_search_bbox_area;

	//Record the boundary informations for binary search
	double binary_search_top = rect.top;
	double binary_search_bottom = rect.bottom;
	
	while( abs( found_util - util ) > error )
	{
	    if( found_util - util < error )
	    {
		binary_search_bottom = cutline_position;
		cutline_position = ( cutline_position + binary_search_top ) / 2.0;
	    }
	    else
	    {
		binary_search_top = cutline_position;
		cutline_position = ( cutline_position + binary_search_bottom ) / 2.0;
	    }


	    //Update binary search box
	    binary_search_bbox.bottom = cutline_position;
	    binary_search_bbox_area = ( binary_search_bbox.right - binary_search_bbox.left ) * 
		( binary_search_bbox.top - binary_search_bbox.bottom );
	    fixed_block_area = ComputeFixedBlockArea( binary_search_bbox );
	    found_util = ( second_child_cell_area + fixed_block_area ) / binary_search_bbox_area;
	} 
    }
    else
    {
	//Compute the first cutline position
	double partition_height = rect.top - rect.bottom;
	cutline_position = rect.right - ( ( second_child_cell_area / util ) / partition_height );

	CRect binary_search_bbox;
	binary_search_bbox.right = rect.right;
	binary_search_bbox.bottom = rect.bottom;
	binary_search_bbox.top = rect.top;
	binary_search_bbox.left = cutline_position;

	double binary_search_bbox_area = ( binary_search_bbox.right - binary_search_bbox.left ) * 
					( binary_search_bbox.top - binary_search_bbox.bottom );
	double fixed_block_area = ComputeFixedBlockArea( binary_search_bbox );
	double found_util = ( second_child_cell_area + fixed_block_area ) / binary_search_bbox_area;
	
	//Record the boundary informations for binary search
	double binary_search_left = rect.left;
	double binary_search_right = rect.right;

	while( abs( found_util - util ) > error )
	{
	    if( found_util - util < error )
	    {
		binary_search_left = cutline_position;
		cutline_position = ( cutline_position + binary_search_right ) / 2.0;
	    }
	    else
	    {
		binary_search_right = cutline_position;
		cutline_position = ( cutline_position + binary_search_left ) / 2.0;
	    }

	    //Update binary search box
	    binary_search_bbox.left = cutline_position;
	    binary_search_bbox_area = ( binary_search_bbox.right - binary_search_bbox.left ) * 
		( binary_search_bbox.top - binary_search_bbox.bottom );
	    fixed_block_area = ComputeFixedBlockArea( binary_search_bbox );
	    found_util = ( second_child_cell_area + fixed_block_area ) / binary_search_bbox_area;
	} 
    }
    
    return cutline_position;
}


#if 0
//Have a problem here.
//The ratio is non-monotonic, so we cannot find a ratio by binary search
//Now approximate to target_free_area 
double WhiteSpaceAllocation::ComputeCutLinePositionByWhiteSpaceRatio( const CRect& rect,
	const Jin::WSAPartition::CutType& cutType,
	const double& white_space_ratio,
	const double& first_child_cell_area,
	const double& second_child_cell_area,
        const double& util )
{
    double cutline_position = 0.0;

    //Set the allowable error of binary search
    //const double error = binary_search_accuracy * util;
    
    //Compute total white space
    double total_rect_area = ( rect.right - rect.left ) * ( rect.top - rect.bottom );
    double total_fixed_block_area = ComputeFixedBlockArea( rect );
    double total_cell_area = first_child_cell_area + second_child_cell_area;
    double total_white_space = total_rect_area - ( ( total_fixed_block_area + total_cell_area ) / util ); 
    
    if( total_white_space < 0.0 )
    {
	fprintf( stderr, "Wanring: nagative total_white_space\n" );
	if( cutType == WSAPartition::HCut )
	{
	    return ( rect.top + rect.bottom ) / 2.0;
	}
	else
	{
	    return ( rect.left + rect.right ) / 2.0;
	}
    }
    
    double first_child_expanded_cell_area = first_child_cell_area / util;
    double second_child_expanded_cell_area = second_child_cell_area / util;
    double rect_width = rect.right - rect.left;
    double rect_height = rect.top - rect.bottom;
   
    double search_cutline_position;
    //Compute first possible binary search cut position
    double first_try_first_child_area = 
	total_white_space / ( 1.0 + 1.0/white_space_ratio ) + first_child_expanded_cell_area;
    if( cutType == WSAPartition::HCut )
    {
	search_cutline_position = rect.bottom + first_try_first_child_area / rect_width;
    }
    else
    {
	search_cutline_position = rect.left + first_try_first_child_area / rect_height;
    }

    //Perform binary search
    double search_white_space_ratio;
    CRect first_search_box = rect; 
    CRect second_search_box = rect;
    double first_bound, second_bound;
    
    if( cutType == WSAPartition::HCut )
    {
	first_search_box.top = search_cutline_position;
	second_search_box.bottom = search_cutline_position;
	first_bound = rect.bottom;
	second_bound = rect.top;
    }
    else
    {
	first_search_box.right = search_cutline_position;
	second_search_box.left = search_cutline_position;
	first_bound = rect.left;
	second_bound = rect.right;
    }
    

    //TODO: incomplete here
    double first_white_space = RectArea( first_search_box ) - 
    
    double binary_search_free_space = ComputeFreeSpace( binary_search_bbox );

    while( abs( binary_search_free_space - target_free_space ) > space_error )
    {
	if( binary_search_free_space - target_free_space > space_error )
	{
	    cutline_position = ( cutline_position + binary_search_bbox.bottom ) / 2.0;
	}
	else
	{
	    cutline_position = ( cutline_position + rect.top ) / 2.0;
	}

	binary_search_bbox.top = cutline_position;
	binary_search_free_space = ComputeFreeSpace( binary_search_bbox );
    } 
}
#endif


#if 1 
//Have a problem here.
//The ratio is non-monotonic, so we cannot find a ratio by binary search
//Now approximate to target_free_area 
double WhiteSpaceAllocation::ComputeCutLinePositionByWhiteSpaceRatio( const CRect& rect,
	const Jin::WSAPartition::CutType& cutType,
	const double& white_space_ratio,
	const double& first_child_cell_area,
	const double& second_child_cell_area,
        const double& util )
{
    double cutline_position = 0.0;

    //Set the allowable error of binary search
    //const double error = binary_search_accuracy * util;
    
    //Compute total white space
    double total_rect_area = ( rect.right - rect.left ) * ( rect.top - rect.bottom );
    double total_fixed_block_area = ComputeFixedBlockArea( rect );
    double total_cell_area = first_child_cell_area + second_child_cell_area;
    double total_white_space = total_rect_area - ( ( total_fixed_block_area + total_cell_area ) / util ); 
    
    if( total_white_space < 0 )
    {
	fprintf( stderr, "Wanring: nagative total_white_space\n" );
    }
    
    //Horizontal cut
    if( cutType == WSAPartition::HCut )
    {
	//Compute the first cutline position
	double first_child_possible_white_space = ( white_space_ratio * total_white_space ) / ( 1 + white_space_ratio );
	double partition_width = rect.right - rect.left;
	double possible_height = 
	    ( ( first_child_cell_area / util ) + first_child_possible_white_space ) / partition_width;
	cutline_position = rect.bottom + possible_height;

	CRect possible_bbox;
	possible_bbox.bottom = rect.bottom;
	possible_bbox.left = rect.left;
	possible_bbox.right = rect.right;
	possible_bbox.top = cutline_position;
	

	//Compute the target_free_area by assuming the fixed_block_area 
	//is always equal to that of the determined cutline position
	double fixed_block_area_in_possible_bbox = ComputeFixedBlockArea( possible_bbox );
	double expanded_fixed_block_area_difference = ( ( 1 / util ) - 1 ) * fixed_block_area_in_possible_bbox;
	double approximate_target_free_area = first_child_possible_white_space + 
					      ( first_child_cell_area / util ) +
					      expanded_fixed_block_area_difference;

	return ComputeCutLinePosition( rect, cutType, approximate_target_free_area );
    }
    //Vertical cut
    else
    {
	//Compute the first cutline position
	double first_child_possible_white_space = ( white_space_ratio * total_white_space ) / ( 1 + white_space_ratio );
	double partition_height = rect.top - rect.bottom;
	double possible_width = 
	    ( ( first_child_cell_area / util ) + first_child_possible_white_space ) / partition_height;
	cutline_position = rect.left + possible_width;

	CRect possible_bbox;
	possible_bbox.top = rect.top;
	possible_bbox.bottom = rect.bottom;
	possible_bbox.left = rect.left;
	possible_bbox.right = cutline_position; 
	

	//Compute the target_free_area by assuming the fixed_block_area 
	//is always equal to that of the determined cutline position
	double fixed_block_area_in_possible_bbox = ComputeFixedBlockArea( possible_bbox );
	double expanded_fixed_block_area_difference = ( ( 1 / util ) - 1 ) * fixed_block_area_in_possible_bbox;
	double approximate_target_free_area = first_child_possible_white_space + 
					      ( first_child_cell_area / util ) +
					      expanded_fixed_block_area_difference;

	return ComputeCutLinePosition( rect, cutType, approximate_target_free_area );
    }
}


#endif


//In SolveRoutability() m_white_space represents the congestion of each partition
void WhiteSpaceAllocation::SolveRoutability( const bool haveCluster )
{
    CCongMap congmap( m_array_x_dimension, 
		      m_array_y_dimension, 
		      m_partitions.front().m_bbox, 
		      m_placedb.m_pLEF );
    CGlobalRouting groute( m_placedb );
    groute.ProbalisticMethod( congmap );
    
    //Assign modules to bottom partitions or add them into m_fixed_block_ids
    for( unsigned int i = 0 ; i < m_partitions.size() ; i++ )
    {
	m_partitions[i].m_module_ids.clear();
	m_partitions[i].m_new_bbox = m_partitions[i].m_bbox;
    }
    m_fixed_block_ids.clear();
    
    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	const Module& curModule = m_placedb.m_modules[i];
	
	//fixed blocks
	if( curModule.m_isFixed == true )
	{
	    m_fixed_block_ids.push_back( i );
	}
	//macros
	else if( curModule.m_height > m_placedb.m_rowHeight && haveCluster == false )
	{
	    m_fixed_block_ids.push_back( i );
	}
	//standard cells
	else
	{
	    const int pIndex = ComputeCorrespondingBottomPartitionIndex( curModule.m_x, curModule.m_y );
	    m_partitions[pIndex].m_module_ids.push_back( i );
	}
    }
    
    //test code
    static int counter = 0;
    counter++;
    char temp[10];
    sprintf( temp, "%d", counter );
    string counter_string = temp; 
    
    if( true )
    {
	printf( "************Begin WSA***************\n" );
	fflush(stdout);
	printf( "bbox: (%.2f,%.2f) (%.2f,%.2f)\n", 
		m_partitions.front().m_bbox.left, 
		m_partitions.front().m_bbox.bottom, 
		m_partitions.front().m_bbox.right, 
		m_partitions.front().m_bbox.top );
	string filename = param.outFilePrefix + ".before_wsa" + counter_string;
	m_placedb.OutputGnuplotFigureWithZoom( filename.c_str(), false, false, true );
	filename = param.outFilePrefix + ".before_wsa.overflow" + counter_string;
	congmap.OutputBinOverflowFigure( filename.c_str() );
	//int gridSize = static_cast<int>( sqrt( m_placedb.m_modules.size() ) );
	int gridSize = static_cast<int>( min( m_array_x_dimension, m_array_y_dimension ) ) ;
	CPlaceBin placeBin( m_placedb );
	placeBin.CreateGrid( gridSize, gridSize );
	filename = param.outFilePrefix + ".before_wsa.density" + counter_string;
	placeBin.OutputBinUtil( filename );
    }
    //@test code
    
    //m_white_space = overflow (must >= 0.0)
    const double overflow_threshold = -4.0;
    for( int i = 0 ; i < m_array_x_dimension ; i++ )
    {
	for( int j = 0 ; j < m_array_y_dimension ; j++ )
	{
	    const int partition_index = m_bottom_index_array[i][j];
	    //double cost = congmap.GetBinCongestion( i, j );
	    double cost = congmap.GetBinCongestion( i, j );
	    cost = cost - overflow_threshold;
	    cost = max( 0.0, cost );
	    m_partitions[ partition_index ].m_white_space = cost;
	}
    }
    
    //Compute the total cell area and total white space for bottom partitions
    for( unsigned int i = static_cast<unsigned int>( m_first_bottom_partition_index ) ;
	    i < m_partitions.size() ; i++ )
    {
	
	WSAPartition& curPartition = m_partitions[i];

	double cell_area = 0.0;
	for( unsigned int j = 0 ; j < curPartition.m_module_ids.size() ; j++ )
	{
	    const int& curId = curPartition.m_module_ids[j];
	    const Module& curModule = m_placedb.m_modules[ curId ];

	    cell_area += (curModule.m_width * curModule.m_height);
	}

	curPartition.m_cell_area = cell_area;

#if _WSA_DEBUG_ 
	fprintf( stdout, "(%.2f,%.2f) %.2f\n", 
		curPartition.m_bbox.right - curPartition.m_bbox.left,
		curPartition.m_bbox.top - curPartition.m_bbox.bottom,
		curPartition.m_white_space );
#endif
    }
   
#if _WSA_DEBUG_
    //test code
    double aa = 0.0;
    double bb = 0.0;
    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	const Module& curModule = m_placedb.m_modules[i];
	aa += curModule.m_width * curModule.m_height;

	if( curModule.m_isFixed  == false && curModule.m_height <= m_placedb.m_rowHeight )
	    bb += curModule.m_width * curModule.m_height;
    }
    printf("total cell area: %f\n", aa );
    printf("total movable cell area: %f\n", bb );
    printf("m_fixed_block_ids.size(): %d\n", m_fixed_block_ids.size() );
    //@test code
#endif   

    //test code
    //printf( "m_partitions.size(): %d\n", m_partitions.size() );
    //fflush( stdout );
    //@test code
    
    //Bottom-up process:
    //Compute the white space and cell area from child partitions
    for( int i = m_first_bottom_partition_index - 1 ;
	    i >= 0 ; i-- )
    {
	WSAPartition& curPartition = m_partitions[i];
	int childIndex = GetFirstChildIndex( i );

	//test code
	//printf( "child index: %d\n", childIndex );
	//fflush( stdout );
	//@test code
	
	curPartition.m_white_space = m_partitions[ childIndex ].m_white_space + 
				     m_partitions[ childIndex + 1 ].m_white_space;
	
	curPartition.m_cell_area = m_partitions[ childIndex ].m_cell_area + 
				     m_partitions[ childIndex + 1 ].m_cell_area;
	
    }
   
    //Top-down process
    //Adjust bin area according to congestion (or congestion overflow)
    m_partitions[0].m_new_bbox =  m_partitions[0].m_bbox;
    for( unsigned int i = 0 ; 
	    i < static_cast<unsigned int>( m_first_bottom_partition_index ) ; 
	    i++ )
    {
	WSAPartition& curPartition = m_partitions[i];
	
	int firstChildIndex = GetFirstChildIndex( i );

#if 0
	//re-compute m_white_space for child partitions according to
	//curPartition.m_new_bbox
	for( int icc = firstChildIndex ; icc < firstChildIndex + 2 ; icc++ )
	{
	    WSAPartition& ccPart = m_partitions[ icc ];
	    //WSAPartition& ccParent = m_partitions[ GetParentIndex( icc ) ];

	    CRect ccBBox = curPartition.m_new_bbox;

	    if( curPartition.m_cutType == WSAPartition::HCut )
	    {
		if( IsFirstChild( icc ) )
		{
		    ccBBox.top = ( ccBBox.top + ccBBox.bottom ) / 2.0;
		}
		else
		{
		    ccBBox.bottom = ( ccBBox.top + ccBBox.bottom ) / 2.0;
		}
	    }
	    else
	    {
		if( IsFirstChild( icc ) )
		{
		    ccBBox.right = ( ccBBox.left + ccBBox.right ) / 2.0;
		}
		else
		{
		    ccBBox.left = ( ccBBox.left + ccBBox.right ) / 2.0;
		}
	    }


	    double cc_free_space = ComputeFreeSpace( ccBBox );
	    ccPart.m_white_space = cc_free_space - ( ccPart.m_cell_area / util );
	   
#if _WSA_DEBUG_ 
	    //test code
	    fprintf( stdout, "set (%d) fs %.2f ws %.2f ca %.2f util %.2f\n", icc, cc_free_space, ccPart.m_white_space,
		  ccPart.m_cell_area, util );
	    fflush( stdout );
	    //@test code
#endif
	}
#endif
	
	WSAPartition& firstChild = m_partitions[ firstChildIndex ];
	WSAPartition& secondChild = m_partitions[ firstChildIndex + 1 ];
	
	double cutline_position;
	
	double total_overflow = firstChild.m_white_space + secondChild.m_white_space;
	double total_free_space = ComputeFreeSpace( curPartition.m_new_bbox );
	double total_cell_area = firstChild.m_cell_area + secondChild.m_cell_area;
	double real_white_space = total_free_space - total_cell_area;

	if( real_white_space < 0.0 )
	{
	    fprintf( stderr, "Warning: nagative real_white_space %.2f (round to 0)\n",
		    real_white_space );
	    real_white_space = 0.0;
	}

	double target_free_space;
	double allowable_error = 0.005;
	
	//If the parent parition do not contain any cell,
	//all children must contain no cell
	if( curPartition.m_cell_area == 0.0 )
	{
#if _WSA_DEBUG_ 
	    //test code
	    fprintf( stdout, "(%3d) no cell\n", i );
	    fflush( stdout );
	    //@test code
#endif	    
	    continue;
	}
	else if( firstChild.m_cell_area == 0.0 )
	{
	    //Approximate the cutline to leftmost position with maximum free space
	    target_free_space = 0.01;
	    allowable_error = 0.005;
	}
	else if( secondChild.m_cell_area == 0.0 )
	{
	    target_free_space = total_free_space - 0.01;
	    allowable_error = 0.005;
	}
	//If there is no overflow 
	//allocate free space according to cell area ratio
	else if( total_overflow == 0.0 )
	{
	    target_free_space = total_free_space * ( firstChild.m_cell_area / 
		( firstChild.m_cell_area + secondChild.m_cell_area ) );
	    
	}
	//Both partitions have overflow > 0
	//Allocate white space propotional to their original white space
	else if( firstChild.m_white_space > 0.0 && secondChild.m_white_space > 0.0 )
	{
	    double target_white_space = real_white_space * ( firstChild.m_white_space /
		    ( firstChild.m_white_space + secondChild.m_white_space ) );
	    target_free_space = firstChild.m_cell_area + target_white_space;
	    
	}
//No such situation
#if 0
	
	//Both partitions have white space <= 0
	//Allocate white space equal to first child's cell area
	else if( firstChild.m_white_space <= 0.0 && secondChild.m_white_space <= 0.0 )
	{
	    cutline_position = ComputeCutLinePositionToMeetFirstChildUtil( curPartition.m_new_bbox,
									   curPartition.m_cutType,
									   firstChild.m_cell_area,
									   util );
	}
#endif
	//One of the partitions have white space <= 0
	//Let the free space of the partition equal the cell area / util
	else
	{
	    if( firstChild.m_white_space == 0.0 )
	    {
		target_free_space = firstChild.m_cell_area;
	    }
	    else
	    {
		double second_child_target_free_space = secondChild.m_cell_area;
		target_free_space = total_free_space - second_child_target_free_space;
	    }
	    
	}

#if _WSA_DEBUG_
	if( target_free_space < -0.1 )
	{
	    fprintf( stderr, "Warning: incorrect target_free_space %f\n", target_free_space );
	}
#endif

	
	//Cut-off incorrect target_free_space generated by computation error
	target_free_space = max( 0.01, target_free_space );
	
	cutline_position = ComputeCutLinePosition( curPartition.m_new_bbox,
		curPartition.m_cutType,
		target_free_space,
		allowable_error );
   
	//Determine the new bbox for child partitions
	if( curPartition.m_cutType == WSAPartition::HCut )
	{
	    //Bottom one
	    firstChild.m_new_bbox = curPartition.m_new_bbox;
	    firstChild.m_new_bbox.top = cutline_position;
#if _WSA_DEBUG_ 
	    //test code
	    fprintf( stdout, "(%3d) b %.2f c %.2f t %.2f l %.2f r %.2f ", 
		    i,
		    curPartition.m_new_bbox.bottom,
		    cutline_position,
		    curPartition.m_new_bbox.top,
		    curPartition.m_new_bbox.left,
		    curPartition.m_new_bbox.right );
	    fprintf( stdout, "w1 %.2f w2 %.2f c1 %.2f c2 %.2f tf %.2f\n", 
		    firstChild.m_white_space,
		    secondChild.m_white_space,
		    firstChild.m_cell_area,
		    secondChild.m_cell_area,
		    target_free_space );
	    fflush( stdout );
	    //@test code
#endif	    
	    //Top one
	    secondChild.m_new_bbox = curPartition.m_new_bbox;
	    secondChild.m_new_bbox.bottom = cutline_position;
	}
	//VCut
	else
	{
	    //Left one
	    firstChild.m_new_bbox = curPartition.m_new_bbox;
	    firstChild.m_new_bbox.right = cutline_position;

#if _WSA_DEBUG_	    
	    //test code
	    fprintf( stdout, "(%3d) l %.2f c %.2f r %.2f b %.2f t %.2f ", 
		    i,
		    curPartition.m_new_bbox.left,
		    cutline_position,
		    curPartition.m_new_bbox.right,
		    curPartition.m_new_bbox.bottom,
		    curPartition.m_new_bbox.top );
	    fprintf( stdout, "w1 %.2f w2 %.2f c1 %.2f c2 %.2f tf %.2f\n", 
		    firstChild.m_white_space,
		    secondChild.m_white_space,
		    firstChild.m_cell_area,
		    secondChild.m_cell_area,
		    target_free_space );
	    fflush( stdout );
	    //@test code
#endif
	    
	    //Right one
	    secondChild.m_new_bbox = curPartition.m_new_bbox;
	    secondChild.m_new_bbox.left = cutline_position;
	}

#if _WSA_DEBUG_ 
	//test code
	fprintf( stdout, "fs1 %.2f fs2 %.2f\n", 
		ComputeFreeSpace( firstChild.m_new_bbox ),
		ComputeFreeSpace( secondChild.m_new_bbox ) );
	fflush( stdout );
	//@test code
#endif
    }
    //@Top-down process
   
#if 0 
    //test code
    double total_part_area = 0.0;
    for( unsigned int i = static_cast<unsigned int>( m_first_bottom_partition_index ) ;
	    i < m_partitions.size() - 1; i++ )
    {
	WSAPartition& part1 = m_partitions[i];

	total_part_area += ( part1.m_new_bbox.right - part1.m_new_bbox.left ) *
	     ( part1.m_new_bbox.top - part1.m_new_bbox.bottom );
	
	for( unsigned int j = i + 1 ; j < m_partitions.size() ; j++ )
	{
	    WSAPartition& part2 = m_partitions[j];

	    double overlap_area = ComputeOverlapArea( part1.m_new_bbox, part2.m_new_bbox );

	    if( overlap_area != 0.0 )
	    {
		fprintf( stderr, "Warning: part %d overlap with part %d with area %.2f\n",
			i, j, overlap_area );
		fflush( stderr );
	    }
	}
    }

    fprintf( stdout, "total new partition area: %f\n", total_part_area );
    fflush( stdout );
    //@test code
#endif
    
    //Move all modules of bottom partitions to new bounding boxes
    for( unsigned int i = static_cast<unsigned int>( m_first_bottom_partition_index ) ;
	    i < m_partitions.size() ; i++ )
    {
	WSAPartition& curPartition = m_partitions[i];

	if( curPartition.m_cell_area == 0.0 )
	    continue;
	
	double orig_width = curPartition.m_bbox.right - curPartition.m_bbox.left;
	double orig_height = curPartition.m_bbox.top - curPartition.m_bbox.bottom;
	double new_width = curPartition.m_new_bbox.right - curPartition.m_new_bbox.left;
	double new_height = curPartition.m_new_bbox.top - curPartition.m_new_bbox.bottom;

	double width_ratio = new_width / orig_width;
	double height_ratio = new_height / orig_height;

	double orig_base_x = curPartition.m_bbox.left;
	double orig_base_y = curPartition.m_bbox.bottom;
	double new_base_x = curPartition.m_new_bbox.left;
	double new_base_y = curPartition.m_new_bbox.bottom;

	for( unsigned int iModule = 0 ; iModule < curPartition.m_module_ids.size() ; iModule++ )
	{
	    int ModuleIndex = curPartition.m_module_ids[ iModule ];
	    Module& curModule = m_placedb.m_modules[ ModuleIndex ];  
	    double new_x = ( curModule.m_x - orig_base_x ) * width_ratio + new_base_x;
	    double new_y = ( curModule.m_y - orig_base_y ) * height_ratio + new_base_y;

	    m_placedb.SetModuleLocation( ModuleIndex, new_x, new_y );
	}
#if _WSA_DEBUG_ 
	//test code
	fprintf( stdout, "%3d (%.2f,%.2f)-(%.2f,%.2f) to (%.2f,%.2f)-(%.2f,%.2f) # modules %d\n",
		i,
		curPartition.m_bbox.left, curPartition.m_bbox.bottom,
		curPartition.m_bbox.right, curPartition.m_bbox.top,
		curPartition.m_new_bbox.left, curPartition.m_new_bbox.bottom,
		curPartition.m_new_bbox.right, curPartition.m_new_bbox.top,
		curPartition.m_module_ids.size() );
	fflush( stdout );
	//@test code
#endif
    }

    
    //test code
    if( true )
    {
	CCongMap congmap2( m_array_x_dimension, 
		m_array_y_dimension, 
		m_partitions.front().m_bbox, 
		m_placedb.m_pLEF );
	CGlobalRouting groute( m_placedb );
	groute.ProbalisticMethod( congmap2 );
	string filename = param.outFilePrefix + ".after_wsa" + counter_string;
	m_placedb.OutputGnuplotFigureWithZoom( filename.c_str(), false, false, true );
	filename = param.outFilePrefix + ".after_wsa.overflow" + counter_string;
	congmap2.OutputBinOverflowFigure( filename.c_str() );
	//int gridSize = static_cast<int>( sqrt( m_placedb.m_modules.size() ) );
	int gridSize = static_cast<int>( min( m_array_x_dimension, m_array_y_dimension ) );
	CPlaceBin placeBin( m_placedb );
	placeBin.CreateGrid( gridSize, gridSize );
	filename = param.outFilePrefix + ".after_wsa.density" + counter_string;
	placeBin.OutputBinUtil( filename );
	printf( "************Finish WSA***************\n" );
    }
    //@test code
    
}
