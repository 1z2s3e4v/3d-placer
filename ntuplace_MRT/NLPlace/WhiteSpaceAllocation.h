#include <iostream>
#include <cstdio>
#include <vector>
#include <limits>

#include "util.h"

class CPlaceDB;

namespace Jin
{

class WSAPartition
{
    public:
	enum CutType{ HCut, VCut };
	
	WSAPartition( const CRect& bbox, const bool& isBottom, const CutType& cutType ) : 
	    m_bbox( bbox ), 
	    m_isBottom( isBottom ),
	    m_cutType( cutType ),
	    //m_total_white_space( -std::numeric_limits<double>::max() ),
	    m_cell_area( -std::numeric_limits<double>::max() ),
	    //m_remaining_whitespace( -std::numeric_limits<double>::max() ),
	    //m_increment( -std::numeric_limits<double>::max() )
	    m_new_bbox( -std::numeric_limits<double>::max(),
			-std::numeric_limits<double>::max(),
			-std::numeric_limits<double>::max(),
			-std::numeric_limits<double>::max() )
	{ m_module_ids.clear(); }
	~WSAPartition(void){}	

	//Record the bounding box for this partition
    	CRect m_bbox;
	//Denote if this partition is a bottom one
	bool m_isBottom;
	//Record the cut direction "in" this partition (to divide this partition to children)
	CutType m_cutType;
	//Record the corresponding modules for this partition (only for bottom partitions)
	std::vector<int> m_module_ids;

	//Variables for WSA
	double m_white_space;
	double m_cell_area;
	CRect m_new_bbox;
	//double m_total_white_space, m_total_cell_area;
	//double m_remaining_whitespace;
	//double m_increment;

};

class RectArea
{
    public:
	double operator()( const CRect& r )
	{
	    return (r.right - r.left) * (r.top - r.bottom);
	}
};

}


//Important: the utiliation (util) is specified by (total cell area) / (free space)
//which do not consider fixed blocks
class WhiteSpaceAllocation
{
    public:
	WhiteSpaceAllocation( CPlaceDB& placedbref, int level); 
	~WhiteSpaceAllocation(void){}

	//Perform the white space allocation
	void SolveDensity( const double util = 1.0, const bool haveCluster = false );
	void SolveDensityIteratively( const double& util = 1.0 );
	void SolveDensityBest( const double& util = 1.0 );
	
	//Perform the white space allocation to optimize routability
	void SolveRoutability( const bool haveCluster = false );
	double ComputeMaxUtil( void );
    private:
	CPlaceDB& m_placedb;
	int m_level;
	//Record the horizontal and vertical dimensions of the bottom level
	int m_array_x_dimension, m_array_y_dimension;
	std::vector<Jin::WSAPartition> m_partitions;

	//Record the index of first bottom partition in m_partitions
	int m_first_bottom_partition_index;
	
	//Compute the parent index for a given partition index
	int GetParentIndex( const int& index ) { return (index-1)/2; }
	//Compute the first children index (the second is first+1) for a given partition index
	//In "m_partitions", the first child must be either the left one (for VCut) or the bottom one (for HCut)
	int GetFirstChildIndex( const int& index ) { return 2*index + 1;}

	//For an odd child index, the child must be the first child of its parent partition
	bool IsFirstChild( const int& index ) { return (index%2)==1; }
	//Otherwise, it must be the second one
	bool IsSecondChild( const int& index ) { return (index%2)==0; }
	
	//Variables to record the dimensions of bottom partition
	double m_bottom_partition_width, m_bottom_partition_height;

	//Map the chip area to indexes of bottom partitions
	std::vector< std::vector<int> > m_bottom_index_array;
	//Base x and y coordinate for m_bottom_index_array
	double m_base_x, m_base_y;

	//For a given coordinate, compute the index of the corresponding bottom partition
	int ComputeCorrespondingBottomPartitionIndex( const CPoint& p );
	int ComputeCorrespondingBottomPartitionIndex( const double& x, const double& y );

	//Record the ids of fixed modules and macros (to compute whitespace)
	std::vector<int> m_fixed_block_ids;
	//For a given region, compute the amount of free space (placable space) within it
	double ComputeFreeSpace( const CRect& rect, const double& util = 1.0 );
	double ComputeFreeSpace( const double& left,
		const double& right,
		const double& bottom,
		const double& top,
	        const double& util = 1.0 );
	
	//For a given region, compute the area of fixed within it
	double ComputeFixedBlockArea( const CRect& rect );
	double ComputeFixedBlockArea( const double& left,
		const double& right,
		const double& bottom,
		const double& top );
	//Compute overlapping area for two given rectangles (return 0 if no overlap)
	double ComputeOverlapArea( const CRect& r1, const CRect& r2 );

	//Compute the cut line position for a given partition by binary search (assume util = 1.0) 
	//The target amount of free space is specified for the first child (left or bottom child)
	double ComputeCutLinePosition( const CRect& rect, 
				       const Jin::WSAPartition::CutType& cutType,
				       const double& target_free_space,
				       const double& allowable_error = -1.0 );

	//Compute the cut line position for a given partition to meet first child's target utilizatoin by binary search 
	//The cutline will satisfy ( first_child_cell_area + fixed_block_area ) / total_area = util
	double ComputeCutLinePositionToMeetFirstChildUtil( const CRect& rect, 
				       const Jin::WSAPartition::CutType& cutType,
				       const double& first_child_cell_area,
				       const double& util );
	//Do the same thing as the above function but meet second child's target utilization
	double ComputeCutLinePositionToMeetSecondChildUtil( const CRect& rect, 
				       const Jin::WSAPartition::CutType& cutType,
				       const double& second_child_cell_area,
				       const double& util );
	//Compute the cutline position to meet the white space ratio (first child/second child)
	double ComputeCutLinePositionByWhiteSpaceRatio( const CRect& rect,
		const Jin::WSAPartition::CutType& cutType,
		const double& white_space_ratio,
		const double& first_child_cell_area,
		const double& second_child_cell_area,
	        const double& util );
};
