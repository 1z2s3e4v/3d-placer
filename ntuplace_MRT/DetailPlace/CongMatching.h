#include "DPlace.h"

//Used to record the corresponding position inside m_segments of a module
class CSegmentPositionIndex
{
    public:
	int m_segId, m_orderId;
	CSegmentPositionIndex(void) : m_segId( -1 ), m_orderId( -1 ){}
	CSegmentPositionIndex( const int& segId, const int& orderId ) :
	    m_segId( segId ), m_orderId( orderId ){}
};

class CCongMatching
{
    private:
	CPlaceDB& m_placedb;
	std::vector<CSegment> m_segments;
	//module ids are stored in the decreasing order of module widths
	std::vector<int> m_module_order;
	std::map<int, CSegmentPositionIndex> m_segment_position_map;
	//bool CompareModuleWidth( const int& m1, const int& m2 );
	class CompareModuleWidth
	{
	    public:
		static std::vector<Module>* m_pMod; 
		bool operator() ( const int& m1, const int& m2 )
		{
		    return (*m_pMod)[m1].m_width > (*m_pMod)[m2].m_width;
		}
	};
	int m_xdim, m_ydim;
	//boundary coordinates
	double m_left, m_right, m_top, m_bottom;
	//length of hcuts and vcuts
	double m_hlength, m_vlength;

	std::vector< std::vector<int> > m_net_bins;
	CRect ComputeNetBBox( const Net& n );
	int GetHBinIndex( const double& x )
	{
	    int index = static_cast<int>(floor((x-m_left)/m_hlength));

	    index = max(0,index);
	    index = min(m_xdim-1,index);
	    return index;
	}
	int GetVBinIndex( const double& y )
	{
	    int index = static_cast<int>(floor((y-m_bottom)/m_vlength));

	    index = max(0,index);
	    index = min(m_ydim-1, index);

	    return index;
	}
	int ComputeNetOverlap( const Net& n );
	int ComputeModuleOverlap( const int& mid );
	int ComputeModuleOverlap( const std::vector<int>& module_ids );
	//Perform congestion matching on the modules and positions
	//Return the resulting matching in matching_results
	void DoMatching( const std::vector<int>& module_ids,
	       const std::vector<CPoint>& positions,
	       std::vector<int>& matching_results );
	//Find a given number of swap-able modules and corresponding positions
	bool FindModulesAndPositions( const int& number, 
		std::vector<int>& module_ids,
		std::vector<CPoint>& positions );
	//Find the x coordinate of the right-most free space for a given module id
	double FindRightBound( const int& module_id );
	
	void AddNetToBins( const Net& n );
	void RemoveNetFromBins( const Net& n );
	int GetBinTotalCost( void )
	{
	    int cost = 0;
	    for( unsigned int i = 0 ; i < m_net_bins.size() ; i++ )
	    {
		for( unsigned int j = 0 ; j < m_net_bins[i].size() ; j++ )
		{
		    cost += m_net_bins[i][j];
		}
	    }
	    return cost;
	}
	
	std::vector<int> m_saved_module_ids;
	std::vector<CPoint> m_saved_positions;
	std::vector< std::vector<int> > m_saved_net_bins;
	std::vector<CSegment> m_saved_segments;
	std::map<int, CSegmentPositionIndex> m_saved_segment_position_map;
	void SaveStatus( const std::vector<int>& module_ids,
		const std::vector<CPoint>& positions )
	{
	    m_saved_module_ids = module_ids;
	    m_saved_positions = positions;
	    m_saved_net_bins = m_net_bins;
	    m_saved_segments = m_segments;
	    m_saved_segment_position_map = m_segment_position_map;
	}
	void RestoreStatus( void )
	{
	    for( unsigned int i = 0 ; i < m_saved_module_ids.size() ; i++ )
	    {
		const int mid = m_saved_module_ids[i];
		m_placedb.SetModuleLocation( mid,
			m_saved_positions[ i ].x,
			m_saved_positions[ i ].y );
	    }
	    m_net_bins = m_saved_net_bins;
	    m_segments = m_saved_segments;
	    m_segment_position_map = m_saved_segment_position_map;
	}
	void ApplyAssignment( const std::vector<int>& module_ids,
		const std::vector<CPoint>& positions,
		const std::vector<int>& assignment );

	void SetRegionalModuleOrder( const CRect& region );
	int GetMaxBinCount( const CRect& region );
    public:
	CCongMatching( CPlaceDB& set_placedb, 
		//std::vector<CSegment> set_segments,
	       	const int& xdim, const int& ydim); 
	//Solve congestion matching once
	//return if get a better result
	bool SolveRegionToWholeChip( const int& size, const int& xbins, const int& ybins );
	bool SolveRegionOnce( const int& size, const CRect& region );
	bool SolveOnce( const int& size, const CRect& region );
	bool Solve( void );
};

