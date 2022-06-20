#ifndef BBSOLVE
#define BBSOLVE

#include <vector>
#include <set>
#include <iostream>
#include <cmath>
#include <limits>
#include <cstdio>
#include <cstdlib>
using namespace std;


class SolutionIterator;
class Solution	// abstract
{
    public:
	virtual ~Solution() {};
	virtual bool IsFeasible () const = 0;
	virtual bool IsComplete () const = 0;
	virtual double Objective () const = 0;
	virtual double Bound () const = 0;
	virtual Solution& Clone () const = 0;	// return a new object
	virtual SolutionIterator& Successors () const = 0;
	virtual void Print() const = 0;
};

class SolutionIterator	// abstract
{
    public:
	virtual ~SolutionIterator () {};
	virtual void Reset () = 0;
	virtual bool IsDone () const = 0;
	virtual Solution& operator* () const = 0;   // return a new object
	virtual void operator++ () = 0;
};

class Solver	// abstract
{
    public:
	Solver() : bestSolution(NULL), bestObjective(numeric_limits<double>::max ()) {}
	virtual ~Solver() { delete bestSolution; }
	virtual Solution& Solve (Solution const&);
	virtual void SetBestSolution( const Solution& bestSol );
	
    protected:
	Solution* bestSolution;
	double bestObjective;
	void UpdateBest (Solution const& sol);
	virtual void DoSolve (Solution const&) = 0;
};

class DepthFirstBranchAndBoundSolver : public Solver
{
    public:
        virtual void DoSolve (Solution const&);
	virtual ~DepthFirstBranchAndBoundSolver() {};
};

class DepthFirstSolver : public Solver
{
    public:
        virtual void DoSolve (Solution const&);
	virtual ~DepthFirstSolver() {};
};



#endif
