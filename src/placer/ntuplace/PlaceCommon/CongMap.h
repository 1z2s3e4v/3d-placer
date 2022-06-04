//Created by Jin 20060602
#ifndef _CONGMAP_
#define _CONGMAP_

#include "util.h"
#include <vector>
#include <map>
//#include "lefdef.h"

class CPlaceDB;
class CCongOptimizer;
//class CCongMap;

// 2007-02-06 (donnie)
int GetRegionNetFluxOverflow( const CPlaceDB*, const double& x1, const double& y1, const double& x2, const double& y2 );

class CCut
{
    //Remove after testing
    //friend class CCongMap;

    private:
	double m_cong;
    public:
	enum Type
	{
	    VCut,
	    HCut
	};
	Type m_type;
	
	CCut( const double& cong, const Type& type, const double& capacity ) :
	    m_cong(cong), m_type(type), m_capacity( capacity ), m_blockage_ratio(0.0) {}
	~CCut(void){}

	//increase or decrease congestion
	void IncreCong( const double& c ) { m_cong += c; }
	void DecreCong( const double& c ) { m_cong -= c; }

	double GetCong(void) const { return m_cong; }

	void clear(void){ m_cong = 0.0; }

	//Record the congestion contribution for each net
	//std::map<int, double> m_module_contribution;

	//void AddContribution( const int& mid, const double& cong );

    private:
	//Edge capacity
	double m_capacity;

    public:
	//Get cut overflow
	double GetOverflow(void) const { return m_cong - ( m_capacity * (1.0 - m_blockage_ratio) ); }

    private:
	//Edge blockage rate
	double m_blockage_ratio;

    public:
	//Increase edge blockage rate ( the reaonable ratio locates in [0.0, 1.0]
	void IncreBlockageRatio( const double& increment ) { m_blockage_ratio += increment; }
};

//pair::first is the index of the recorded module
//pair::second is the corresponding contribution
typedef std::pair<int, double> ContributionItem;

class CompareContributionItemByContribution
{
    public:
	bool operator()( const ContributionItem& c1, const ContributionItem& c2 )
	{
	    return c1.second > c2.second;
	}
};

class CCongMap
{
    friend class CPlaceDB;
    friend class CCongOptimizer;
    private:
	//number of bins in horizontal or vertical direction
	int m_hdim, m_vdim;

	//boundary coordinates
	double m_left, m_right, m_top, m_bottom;

	//length of hcuts and vcuts 
	double m_hlength, m_vlength;
	
	
	//congestion for horizontal and vertical cuts
	//m_HCuts.size() = m_vdim+1, m_VCuts.size() = m_hdim+1
	//  ----------------------  HCuts[2]
	//  |         |          |
	//  |         |          |
	//  ---------------------   HCuts[1]
	//  |         |          |
	//  |         |          |
	//  ---------------------   HCuts[0]
	// VCuts[0] VCuts[1]   VCuts[2]  ...
	std::vector< std::vector<CCut> > m_HCuts;
	std::vector< std::vector<CCut> > m_VCuts;
    
	//Record the number of local nets in each bin
	std::vector< std::vector<int> > m_numLocalNet;
	std::vector< std::vector<double> > m_lengthLocalNet;

	//Record the max local length capacity of a bin
	double m_maxBinLength;

    public:
	//capacities for h and v cuts	
	double m_num_wire_per_hcut, m_num_wire_per_vcut;

    private:
	//# of H and V layers
	int m_hcut_layer_count, m_vcut_layer_count;


    //*********
    //Functions
    //*********
    private:	
	//Get the total congestion of a bin
	double GetBinTotalCongestion( const int& i, const int& j );	
    
    public:
	CCongMap( const int& hdim, const int& vdim, const CRect& bbox, const CParserLEFDEF* const pLEF );
	~CCongMap(void){}

	// 2007-03-27 (donnie)
	int GetVDim() { return m_vdim; }
	int GetHDim() { return m_hdim; }
	double GetHLength() { return m_hlength; }
	double GetVLength() { return m_vlength; }

	//clear m_HCuts and m_VCuts
	void clear(void);
	
	
	//compute the bin index for a given x or y coordinate
	int GetHBinIndex( const double& x );
	int GetVBinIndex( const double& y );

	//add congestion of a two-pin net (probalistic method)
	void AddTwoPinNet( const CPoint& p1, const CPoint& p2 );

	//remove congestion of a two-pin net (probalistic method)
	void RemoveTwoPinNet( const CPoint& p1, const CPoint& p2 );
	
	//add contribution information of a two-pin net (only for -nosteiner mode)
	//void AddTwoPinNetContribution( const int& i1, const CPoint& p1, const int& i2, const CPoint& p2 );

	//add congestion of a two-pin net by upper and lower Ls (FastRoute)
	void AddTwoPinNetByUpperLowerL( const CPoint& p1, const CPoint& p2 );
	
	//Return the overflow of the specified bin (i, j)
	double GetBinOverflow( const int& i, const int& j );

	//Return the average congestion of the specified bin (i, j)
	double GetBinCongestion( const int& i, const int& j );
	
	//Return the total "positive" overflow of all cuts in the chip
	double GetTotalOverflow(void);

	//Return the max overflow of each cut
	double GetMaxOverflow(void);
	
	//Return # of VCut/HCut layers
	int GetNumVCutLayer(void)
	{ return m_vcut_layer_count; }
	int GetNumHCutLayer(void)
	{ return m_hcut_layer_count; }
	
	
	//Return the max congestion of each cut
	double GetMaxVCutCongestion(void);
	double GetMaxHCutCongestion(void);
	double GetMaxCongestion(void);
	
	//Define the cut postion of a bin
	enum CutPosition{
	    Top,
	    Bottom,
	    Left,
	    Right};

	//Get the CCut reference corresponding to a given bin
	CCut& GetCut( const int& i, const int& j, const CutPosition& pos );
	
	//Get the CCut congestion for a given bin
	double GetCutCongestion( const int& i, const int& j, const CutPosition& pos )
	{
	    return GetCut( i, j, pos ).GetCong();
	}

	//Get the CCut overflow of a specified bin
	double GetCutOverflow( const int& i, const int& j, const CutPosition& pos )
	{
	    //double capacity;
	    //if( pos == Left || pos == Right )
	    //	capacity = m_num_wire_per_vcut;
	    //else
	    //	capacity = m_num_wire_per_hcut;

	    //return GetCut( i, j, pos ).GetCong() - capacity;
	    return GetCut( i, j, pos ).GetOverflow();
	}
	
	//Draw the bin overflow distribution
	void OutputBinOverflowFigure( const std::string& filename );	

	//Draw the cut overflow figure
	void OutputHVCutOverflowFigure( const string& filename );    // 2007-02-16 (donnie)  H/V separated
	void OutputCutOverflowFigure( const std::string& filename );

	//Draw local-net length distribution of each bin
	void OutputBinLocalNetLengthFigure( const std::string& filename );

	//Draw total length distribution (including global and local nets) of each bin
	void OutputBinTotalLengthFigure( const std::string& filename );

	//Return the maximum total wirelength of all bins on the chip
	double GetMaxBinTotalLength(void);
	
	//Compute the sorted contributions for a given bin
	//void ComputeSortedBinContribution( const int& i, 
	//	const int& j, 
	//	std::vector<ContributionItem>& cons );

	//Compute the center location for a given bin
	void ComputeBinCenterLocation( const int& i, const int& j, double& x, double& y )
	{
	   x = m_left + ( i * m_hlength ) + ( m_hlength / 2.0 );
	   y = m_bottom + ( j * m_vlength ) + ( m_vlength / 2.0 );
	}

	//Compute the cut location for a specified bin
	CRect ComputeBinBBox( const int& i, const int& j )
	{
	    CRect rect;
	    rect.left = m_left + ( i * m_hlength );
	    rect.right = m_left + ( i * m_hlength ) + m_hlength;
	    rect.bottom = m_bottom + ( j * m_vlength );
	    rect.top = m_bottom + ( j * m_vlength ) + m_vlength;
	    return rect;
	}

	//Get the maximum overflow of all covered cuts for a given two-pin net
	double GetTwoPinNetMaxOverflow( const CPoint& p1, const CPoint& p2 );

	//Increase the edge blockage rate by a given bounding box and a blockage ratio
	void AddBlockageByRatio( const CPoint& p1, const CPoint& p2, const double& blockage_ratio );

};


//Record the information for each partition (for congestion optimization)
class CCongPart
{
    public:
	//x-y index for this part
	int m_xindex, m_yindex;
	CRect m_bbox;
	std::vector<int> m_module_ids;
    public:
	//CCongPart( void ){}
	CCongPart( const int& xindex, const int& yindex, const CRect& bbox ) : 
	    m_xindex( xindex ), m_yindex( yindex ), m_bbox( bbox ) {}
	void AddModuleId( const int& id )
	{
	    m_module_ids.push_back( id );
	}
};

//congestion optimizer
class CCongOptimizer
{
    public:
	CCongOptimizer( CPlaceDB& placedb, 
		const int& hdim, 
		const int& vdim ) : 
	    m_placedb( placedb ),
	    m_congmap( hdim, vdim, placedb.m_coreRgn, placedb.m_pLEF ),
	    m_hdim( hdim ),
	    m_vdim( vdim )
	    {}
	~CCongOptimizer(void){}

	void Observe(void);
	//For FM-based congestion optimization (failed)
	void OptimizeCongestion( const int& i, const int& j, const CCongMap::CutPosition& pos );
    private:
	CPlaceDB& m_placedb;
	CCongMap m_congmap;
	//dimensions for m_congmap
	int m_hdim, m_vdim;
};

#endif
