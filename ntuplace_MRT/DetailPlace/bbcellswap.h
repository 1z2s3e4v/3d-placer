#ifndef _BBCELLSWAP_
#define _BBCELLSWAP_

#include <vector>
#include <set>
#include <list>
#include <iostream>
#include <cmath>
#include <map>
#include <limits>
using namespace std;

#include "bbsolve.h"
#include "placedb.h"

class CellSwapSolution;
class CellSwapSolutionIterator : public SolutionIterator
{
    public:
	CellSwapSolutionIterator( const CellSwapSolution& sol ) 
	    : m_sol( sol )
	{ Reset(); }
	virtual ~CellSwapSolutionIterator () {};
	virtual void Reset ();
	virtual bool IsDone () const;
	virtual Solution& operator* () const;
	virtual void operator++ () { m_ite++; }

    private:
	list<int>::const_iterator m_ite; //The iterator points to current item in m_sol
	const CellSwapSolution& m_sol;
};


class CellSwapSolution : public Solution
{
    friend class CellSwapSolutionIterator;
    friend class CellSwap;

    public:
	CellSwapSolution( /*int size*/ CPlaceDB* const pDB );
	CellSwapSolution( const CellSwapSolution& sol );
	
	virtual bool IsFeasible() const;
	virtual bool IsComplete() const { return (m_item.size() ==  0); }
	virtual double Objective() const;
	virtual double Bound() const;
	virtual Solution& Clone() const;
	virtual SolutionIterator& Successors() const;
	virtual void Print() const;
	
	//void PushList( const int& );	// push and calc new _bound
	//void InsertItem( const int& );	

    //private:
	//double GetCellNetBound( const int& ) const;
	//double GetCellNetLength( const int& ) const;
	
    private:
	double      m_bound; //For incrementally compute the solution cost
	list<int>	m_list;  // inserted block id
	double      m_whiteWidth; //The width of the whitespace
	//double      _x;
	//double      _y;
	double      m_currentX;
	//double      _rowWidth;
	//int         _rowNum;
	CPlaceDB*   m_pDB;

	map<int, int> m_net_module_count;
	
	void InitializeNetModuleCount(void);	

	private:
	
	list<int>    m_item;  // to be inserted block id (-1 is whitespace)
	list<double> m_xlocation; //Record the x coordinate of each module in m_list
	
	void RecalculateBound(void); //Recalculate the total wirelength in m_list if this solution
								 //is complete
};

//The placeable interval in each row is called a segment
class CSegment
{
public:
	CSegment( const double& set_bottom, const double& set_left, const double& set_right ) :
		m_bottom( set_bottom ), m_left( set_left ), m_right( set_right ) {}
	~CSegment(void){}
	double m_bottom;	//The bottom y coordinate of this row of sites
	double m_left, m_right;	//The left and right coordinates of this row of sites
	vector<int> m_module_ids;	//Record the ID's of the modules on this segment
	void AddModuleId( const int& id ) { m_module_ids.push_back( id ); }
};

class CellSwap
{
    public:
	CellSwap( CPlaceDB& db, vector<CSegment>& set_segments );
	~CellSwap();
	
	void Solve(const int& bbWindowSize, const int& bbOverlapSize, const int& bbIteration);
	
    private:
	CPlaceDB* m_pDB;
	vector<CSegment>& m_segments;
	bool SolveVectorCellSwap(vector<int>::iterator iteBegin, 
			vector<int>::iterator iteEnd );
	//static double m_improve;
};

class CompareModuleById
{
public:
	static CPlaceDB* m_pDB;
	static bool CompareXCoor( const int& id1, const int& id2 )
	{
		const Module& m1 = m_pDB->m_modules[id1];
		const Module& m2 = m_pDB->m_modules[id2];

		return m1.m_x < m2.m_x;
	}	
};

//CPlaceDB* CompareModuleById::ppDB=0;

#endif
