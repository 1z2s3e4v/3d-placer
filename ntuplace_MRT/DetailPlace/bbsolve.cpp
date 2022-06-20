#include "bbsolve.h" 


void Solver::SetBestSolution( const Solution& bestSol )
{
	if( bestSol.IsComplete() )
	{
		bestObjective = bestSol.Objective();
		bestSolution = &(bestSol.Clone());
	}
}

Solution& Solver::Solve (Solution const& initial)
{
	//printf( "Solve\n" );
	//bestSolution = 0;
	//bestObjective = numeric_limits<int>::max ();
	DoSolve (initial);
	if (bestSolution == 0)
	{
		//throw domain_error ("no feasible solution found");
		printf( "no feasible solution found" );
		exit(0);
	}
	return *bestSolution;
}

void Solver::UpdateBest (Solution const& solution)
{
	if (solution.IsFeasible() &&
			solution.Objective () < bestObjective)
	{
		delete bestSolution;
		bestSolution = &(solution.Clone ());
		bestObjective = solution.Objective ();
	}
}

///////////////

void DepthFirstBranchAndBoundSolver::DoSolve (
		Solution const& solution)
{
	if (solution.IsComplete ())
	{
		UpdateBest (solution);
		//solution.Print();
	}
	else
	{

		//printf( "Current: " );
		//solution.Print();
		SolutionIterator& i = solution.Successors ();
		while (!i.IsDone ()) {
			Solution& successor = dynamic_cast<Solution&> (*i);

			//successor.Print();
			if (successor.IsFeasible () &&
					successor.Bound () < bestObjective)
				DoSolve (successor);

			delete &successor;
			++i;
		}
		delete &i;
	}
}


void DepthFirstSolver::DoSolve (
		Solution const& solution)
{
	if (solution.IsComplete ())
		UpdateBest (solution);
	else
	{
		SolutionIterator& i = solution.Successors ();
		while (!i.IsDone ()) {
			Solution& successor = dynamic_cast<Solution&> (*i);
			DoSolve (successor);
			delete &successor;
			++i;
		}
		delete &i;
	}
}
