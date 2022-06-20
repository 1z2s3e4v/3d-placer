#include "MinimumSpanningTree.h"
#include <algorithm>

using namespace std;

class HeapElement
{
public:
    //Record the index in the original array
    int m_index;
    CPoint m_point;
    double m_cost;
    int m_predessor_index;
    CPoint m_predessor;
    HeapElement(const int& index, const CPoint& p, const double& cost, 
	    const int& pre_index, const CPoint& pre)
    {
	m_index = index;
	m_point = p;
	m_cost = cost;
	m_predessor_index = pre_index;
	m_predessor = pre;
    }
    ~HeapElement(void){}
};

class HeapElementPred
{
public:
    bool operator()( const HeapElement& h1, const HeapElement& h2 )
    {
	return h1.m_cost > h2.m_cost;
    }
};


//Prim's algorithm with Q in binary heap 
//Time complexity: O(E lgV)
void CMinimumSpanningTree::Solve(const vector<CPoint>& points)
{
    m_mst_array.clear();
    m_mst_index_array.clear();
    m_mst_array.reserve( 2*(points.size()-1) );
    m_mst_index_array.reserve( 2*(points.size()-1) );
    
    if( points.size() > 2 )
    {
	//r and Q are the same with boss's slides (Prim's algorithm)
	CPoint r;
	vector<HeapElement> Q;

	r = points.front();
	Q.reserve( points.size()-1 );
	for( unsigned int i = 1 ; i < points.size() ; i++ )
	{
	    double cost = abs(points[i].x-r.x) + abs(points[i].y-r.y);
	    Q.push_back( HeapElement( i, points[i], cost, 0, r ) );
	}
	make_heap(Q.begin(), Q.end(), HeapElementPred() );

	while( Q.size() > 0 )
	{
	    //Extract-Min
	    pop_heap(Q.begin(), Q.end(), HeapElementPred());
	    CPoint u = Q.back().m_point;
	    int u_index = Q.back().m_index;
	    m_mst_index_array.push_back( u_index );
	    m_mst_array.push_back(u);
	    m_mst_index_array.push_back( Q.back().m_predessor_index );
	    m_mst_array.push_back(Q.back().m_predessor);
	    Q.erase(Q.end()-1);

	    //Update costs
	    //for( int i = Q.size()-1 ; i >= 0 ; i-- )
	    for( unsigned int i = 0 ; i < Q.size() ; i++ )
	    {
		CPoint v = Q[i].m_point;

		//w(u,v)
		double new_cost = abs(v.x-u.x)+abs(v.y-u.y);

		if( new_cost < Q[i].m_cost )
		{
		    Q[i].m_cost = new_cost;
		    Q[i].m_predessor_index = u_index;
		    Q[i].m_predessor = u;
		    push_heap( Q.begin(), Q.begin()+i+1, HeapElementPred() ); 
		}
	    }
	    //make_heap( Q.begin(), Q.end(), HeapElementPred() );
	}
    }
    else if( points.size() == 2 )
    {
	m_mst_array = points;
	m_mst_index_array.push_back(0);
	m_mst_index_array.push_back(1);
    }
    else if( points.size() == 1 )
    {
	fprintf(stderr, "Warning: CMinimumSpanningTree::Solve() input contains only one point\n");
	m_mst_array.push_back(points[0]);
	m_mst_array.push_back(points[0]);
	m_mst_index_array.push_back(0);
	m_mst_index_array.push_back(0);
    }
    else //points.size() == 0
    {
	fprintf(stderr, "Warning: CMinimumSpanningTree::Solve() input contains no point\n" );
    }
}
