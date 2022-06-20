#include "CongMap.h"
#include "GRouteMap.h"
#include "GlobalRouting.h"
#include <list>

namespace Jin
{
    //Record the module ids in the same row (in the order of x coordinates)
    class CModuleDQueue
    {
	private:
	    std::list<int> m_module_ids;	
	public:
	    CModuleDQueue(void){}
	    ~CModuleDQueue(void){}
	    bool IsEmpty(void){ return m_module_ids.size() == 0; }
	    int GetLeft(void) { return m_module_ids.front(); }
	    int GetRight(void) { return m_module_ids.back(); }
	    void PushLeft( const int& modId ) { m_module_ids.push_front( modId ); }
	    void PushRight( const int& modId ) { m_module_ids.push_back( modId ); }
	    void PopLeft(void) { m_module_ids.pop_front(); }
	    void PopRight(void) { m_module_ids.pop_back(); }
	    int GetModuleNumber(void){ return m_module_ids.size(); }
	    std::list<int>::const_iterator Begin( void ) { return m_module_ids.begin(); }
	    std::list<int>::const_iterator End( void ) { return m_module_ids.end(); }
    };
    
    class CCellSlidingRow
    {
	public:
	    CCellSlidingRow( const double& bottom,
		    const double& height,
		    const double& left,
		    const double& right );
	    ~CCellSlidingRow(void);

	    //Record the same information in the given CSiteRow
	    double m_bottom;
	    double m_height;
	    std::vector<double> m_interval;
	    
	    //Record the module ids in this row
	    CModuleDQueue m_module_queue;
    };

    class ModuleSortElement
    {
	 public:
	     ModuleSortElement( const int& module_id, const double& x, const double& y ) :
		 m_module_id( module_id ),
		 m_x( x ),
		 m_y( y ) {}
	     int m_module_id;
	     double m_x, m_y;
    };
    class ModuleXCoorComparator
    {
	public:
	    bool operator()( const ModuleSortElement& p1, const ModuleSortElement& p2 )
	    {
		if( p1.m_y == p2.m_y )
		    return p1.m_x < p2.m_x;
		else
		    return p1.m_y < p2.m_y;
	    }	
    };
    class BinCongestionComparator
    {
	public:
	    bool operator()( const std::pair<int, double>& p1, const std::pair<int, double>& p2 )
	    {
		return p1.second > p2.second;
	    }
    };
}
//End of namespace Jin

class CPlaceDB;


//This class now assuming the sites are continuous (no macros exists)
class CCellSlidingForCongestion
{
    public:
	CCellSlidingForCongestion( CPlaceDB& placedb );
       	~CCellSlidingForCongestion( void );
	enum SolveType{ CONG, CMP };
	void Solve( const double& congThreshold, const SolveType& type = CONG );
    private:
	CPlaceDB& m_placedb;
	//double m_congThreshold;
	CCongMap* m_pCongMap;
	GRouteMap* m_pGRouteMap;
	//Dimension of horizontal bins
	//int m_hdim;
	//The placement step for each row
	//(assume all rows have a equal step)
	double m_step;
	vector<Jin::CCellSlidingRow> m_rows;
	//Apply cell sliding on a single row
	void SlideOneRow( 
		//const std::vector<double>& bounds, 
		Jin::CCellSlidingRow& row, 
		const double& congThreshold,
	        const SolveType& type = CONG );
	//Compute the white space for a single row of a given congestion map
	//std::vector<double> AllocateWhiteSpaceByCongestionRatio( 
	//	Jin::CCellSlidingRow& row, 
	//	const double& congThreshold );
	
	//Round some value up/down to fit the step size
	double CutUpToStep( const double& d )
	{
	    return ceil( d / m_step ) * m_step;
	}
	double CutDownToStep( const double& d )
	{
	    return floor( d / m_step ) * m_step;
	}
};
