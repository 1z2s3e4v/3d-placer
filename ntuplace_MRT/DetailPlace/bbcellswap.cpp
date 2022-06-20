#include <cstdio>
#include <list>
#include <algorithm>
using namespace std;

#include "bbcellswap.h"

CPlaceDB* CompareModuleById::m_pDB = 0;
//double CellSwap::m_improve = 0;
CellSwap::CellSwap( CPlaceDB& db, vector<CSegment>& set_segments )
    : m_pDB( &db ) , m_segments( set_segments )
{
#if 0
	//Initialize class CompareModuleById
	CompareModuleById::m_pDB = m_pDB;
	
	//Initialize m_segments
	for( vector<CSiteRow>::const_iterator iteRow = m_pDB->m_sites.begin() ;
		   iteRow != m_pDB->m_sites.end() ; iteRow++ )
	{
		for( unsigned int iInterval = 0 ; iInterval < iteRow->m_interval.size() ; iInterval=iInterval+2 )
		{
			m_segments.push_back( CSegment( iteRow->m_bottom, 
								iteRow->m_interval[iInterval], 
								iteRow->m_interval[iInterval+1] ) );
		}
	}

	//Add all id's of non-Macro modules into m_segments
	for( unsigned int iModule = 0 ; iModule < m_pDB->m_modules.size() ; iModule++ )
	{
		const Module& curModule = m_pDB->m_modules[iModule];
		//Skip Macro modules
		if( curModule.m_height != m_pDB->m_rowHeight )
			continue;

		//Find the corresponding segment of this module,
		//and insert the module id into the segment
		vector<CSegment>::iterator iteFindSegment;
		for( iteFindSegment = m_segments.begin() ; 
			iteFindSegment != m_segments.end() ; iteFindSegment++ )
		{
			if( iteFindSegment->m_bottom == curModule.m_y &&
					iteFindSegment->m_left <= curModule.m_x &&
					iteFindSegment->m_right >= curModule.m_x )
			{
				iteFindSegment->AddModuleId( iModule );
				break;
			}
		}	

		//Warning: this module is not on any segments
		if( iteFindSegment == m_segments.end() )
		{
			cerr << "Warning: Module " << iModule << " is not on any segments" << endl;
		}
	}
	
	//Sort the module id's by their x coordinates
	for( vector<CSegment>::iterator iteSegment = m_segments.begin();
			iteSegment != m_segments.end() ; iteSegment++ )
	{
		sort( iteSegment->m_module_ids.begin(), 
				iteSegment->m_module_ids.end(),
				CompareModuleById::CompareXCoor );
	}
#endif
#if 0 
	//test code
	cout << "Test for sorted modules" << endl;
	for( vector<CSegment>::iterator iteSegment = m_segments.begin();
			iteSegment != m_segments.end() ; iteSegment++ )
	{
		cout << "segment y coordinate: " << iteSegment->m_bottom << endl;

		for( vector<int>::iterator iteId = iteSegment->m_module_ids.begin();
				iteId != iteSegment->m_module_ids.end() ; iteId++ )
		{
			cout << *iteId << "(" << m_pDB->m_modules[*iteId].m_x << ") ";
		}
		cout << endl;
	}
	//@test code
#endif
}

CellSwap::~CellSwap()
{
}

void CellSwap::Solve(const int& bbWindowSize, const int& bbOverlapSize, const int& bbIteration)
{
	int window_size = bbWindowSize;
	int overlap_size = bbOverlapSize;
	int iteration_number = bbIteration;
	int show_detail = 0;

	printf( "(%d, %d, %d)\n", window_size, overlap_size, iteration_number );
	fflush( stdout );
	
#if 0
	cout << "*******Details of BBCellSwap*********" << endl;
	cout << "window_size:      " << window_size << endl;
	cout << "overlap_size:     " << overlap_size << endl;
	cout << "iteration_number: " << iteration_number << endl;
	//cout << "show_detail:      " << show_detail << endl;
#endif

	vector<int> item_vector;

	//const double origWL = m_pDB->GetHPWLp2p();

	if( show_detail )
		cout << "Segment size: " << m_segments.size() << endl;

	for( int i = 0 ; i < iteration_number ; i++ )
	{
		int iSegment = 0;
		for( vector<CSegment>::iterator iteSegment = m_segments.begin() ;
				iteSegment != m_segments.end() ; iteSegment++ )
		{
			if( show_detail )
				printf("<%d>", iSegment );

			iSegment++;
			//cout << ">";
			item_vector.clear();
			//No cells in this segment
			if( iteSegment->m_module_ids.size() < 1 )
			{
				if( show_detail )
					cout << endl;
				
				continue;
			}
			//One cell in this segment
			//Check the solution packing the cell to the left or right
			else if( iteSegment->m_module_ids.size() == 1 )
			{
				int moduleId = iteSegment->m_module_ids.front();
				const Module& curModule = m_pDB->m_modules[moduleId];
				double origX = curModule.m_x;
				double origW = m_pDB->GetModuleTotalNetLength(moduleId);

				double leftX = iteSegment->m_left;
				m_pDB->SetModuleLocation( moduleId, leftX, curModule.m_y );
				double leftW = m_pDB->GetModuleTotalNetLength(moduleId);

				double rightX = iteSegment->m_right - curModule.m_width;
				m_pDB->SetModuleLocation( moduleId, rightX, curModule.m_y );
				double rightW = m_pDB->GetModuleTotalNetLength(moduleId);

				//OrigW is the smallest
				if( origW <= leftW && origW <= rightW )
				{
					m_pDB->SetModuleLocation( moduleId, origX, curModule.m_y );
					if( show_detail )
						cout << "-" << endl;
				}
				//leftW is the smallest
				else if( leftW <= origW && leftW <= rightW )
				{
					m_pDB->SetModuleLocation( moduleId, leftX, curModule.m_y );
					if( show_detail )
						cout << "O" << endl;
				}
				//rightW is the smallest
				else
				{
					m_pDB->SetModuleLocation( moduleId, rightX, curModule.m_y );
					if( show_detail )
						cout << "O" << endl;
				}

				continue;
			}
#if 0
			//If there is free space in the beginning and ending of this segment,
			//move the first module to the beginning and the last module to the end
 
			const Module& m1 = m_pDB->m_modules[iteSegment->m_module_ids.front()];
			const Module& m2 = m_pDB->m_modules[iteSegment->m_module_ids.back()];

			m_pDB->SetModuleLocation( iteSegment->m_module_ids.front(),
										 iteSegment->m_left, 
										 m1.m_y );
			m_pDB->SetModuleLocation( iteSegment->m_module_ids.back(),
										 iteSegment->m_right - m2.m_width,
										 m2.m_y );
#endif		
			if( iteSegment->m_module_ids.size() > 
					static_cast<unsigned int>(window_size) )
			{
				for( vector<int>::iterator iteBegin = iteSegment->m_module_ids.begin() ;
						iteBegin+window_size <= iteSegment->m_module_ids.end() ; 
						iteBegin = iteBegin+(window_size-overlap_size) )
				{
					bool bImprove = SolveVectorCellSwap( iteBegin, iteBegin+window_size );

					if( show_detail )
					{
						if( bImprove )
							cout << "O";
						else
							cout << "-";
					}
				}
			}
			else
			{
				bool bImprove = SolveVectorCellSwap( iteSegment->m_module_ids.begin(), 
						iteSegment->m_module_ids.end() );

				if( show_detail )
				{
					if( bImprove )
						cout << "O";
					else
						cout << "-";
				}

			}
			if( show_detail )
				cout << endl;
		}

	}
}

bool CellSwap::SolveVectorCellSwap( const vector<int>::iterator iteBegin, 
		const vector<int>::iterator iteEnd )
{
	//The original solution is the bound of CellSwap
	CellSwapSolution origSol( m_pDB );

	//test code
	double left_bound = m_pDB->m_modules[*iteBegin].m_x;
	double right_bound = m_pDB->m_modules[*(iteEnd-1)].m_x + 
		m_pDB->m_modules[*(iteEnd-1)].m_width;
	//@test code
	
	for( vector<int>::iterator itePushItem = iteBegin ;
			itePushItem != iteEnd ; itePushItem++ )
	{
		origSol.m_list.push_back( *itePushItem );
		origSol.m_xlocation.push_back( m_pDB->m_modules[*itePushItem].m_x );
	}
	origSol.RecalculateBound();

	CellSwapSolution sol( m_pDB );
	sol.m_bound = 0;
	for( vector<int>::iterator itePushItem = iteBegin ;
			itePushItem != iteEnd ; itePushItem++ )
	{
		sol.m_item.push_back( *itePushItem );
	}

	sol.InitializeNetModuleCount();

	double minX = numeric_limits<double>::max(); 
	double maxX = -numeric_limits<double>::max();
	double totalWidth = 0;

	for( list<int>::iterator iteModuleId = sol.m_item.begin() ;
			iteModuleId != sol.m_item.end() ; iteModuleId++ )
	{
		const Module& curModule = m_pDB->m_modules[*iteModuleId];
		minX = min( minX, curModule.m_x );
		maxX = max( maxX, curModule.m_x + curModule.m_width );
		totalWidth += curModule.m_width;

	}

	//Set currentX
	sol.m_currentX = minX;

	//If there is free space in current range,
	//add a white space module
	if( totalWidth < maxX - minX )
	{
		sol.m_whiteWidth = maxX - minX - totalWidth;
		sol.m_item.push_back( -1 );
	}

	
	DepthFirstBranchAndBoundSolver solver;
	solver.SetBestSolution( origSol );
	CellSwapSolution bestSol( (CellSwapSolution&)solver.Solve( sol ) );

	//Update the module coordinates
	vector<int>::iterator iteUpdate = iteBegin;
	list<int>::iterator iteList = bestSol.m_list.begin();
	list<double>::iterator iteLocation = bestSol.m_xlocation.begin();

	while( iteList != bestSol.m_list.end() )
	{
		if( -1 != *iteList )
		{
			Module& curModule = m_pDB->m_modules[*iteList];
			//Update coordiantes
			m_pDB->SetModuleLocation( *iteList,
					*iteLocation,
					curModule.m_y );

			//test code
			if( *iteLocation < left_bound-0.01 || 
					(*iteLocation) + curModule.m_width > right_bound+0.01 )
			{
				fprintf(stderr, 
						"Warning: Module is placed out of bound in SolveVectorCellSwap()\n");
				fprintf(stderr, "Row bottom %.2f left_bound %.2f right_bound %.2f\n",
						curModule.m_y, left_bound, right_bound );
				fprintf(stderr, "Module %d left %.2f right %.2f\n", 
						*iteList, *iteLocation, (*iteLocation)+curModule.m_width );
			
				//fprintf(stderr, "white space width: %.2f list order: ", maxX - minX - totalWidth );
				//for( list<int>::iterator ite = bestSol.m_list.begin() ; 
				//	ite != bestSol.m_list.end() ; ite++ )
				//{
				//    fprintf(stderr, "%d ", *ite );
				//}
				//fprintf(stderr, "\n" );
				
				
				//m_pDB->OutputGnuplotFigureWithZoom( "bb_bug", false, true, true );
				//getchar();
			}
			//@test code
			
			//Update module order
			*iteUpdate = *iteList;
			iteUpdate++;
		}


		iteList++;
		iteLocation++;
	}

	
	if( bestSol.m_bound < origSol.m_bound )
	{
		return true;
	}
	else
	{
		return false;
	}
}  


///////////////////////////////////////////////////

void CellSwapSolutionIterator::Reset ()
{
	m_ite = m_sol.m_item.begin();
}

bool CellSwapSolutionIterator::IsDone () const
{
	return m_ite == m_sol.m_item.end();
}

Solution& CellSwapSolutionIterator::operator* () const
{
	CellSwapSolution* pSol = new CellSwapSolution( m_sol.m_pDB );
	pSol->m_whiteWidth = m_sol.m_whiteWidth;
	pSol->m_xlocation = m_sol.m_xlocation;
	//pSol->m_pDB = m_sol.m_pDB;

	//Push m_ite into list
	pSol->m_list = m_sol.m_list;
	pSol->m_list.push_back( *m_ite );

	//Copy m_item except for m_ite
	for( list<int>::const_iterator copyIte = m_sol.m_item.begin() ;
			copyIte != m_sol.m_item.end() ; copyIte++ )
	{
		if( copyIte != m_ite )
			pSol->m_item.push_back( *copyIte );
	}

	const Module& curModule = m_sol.m_pDB->m_modules[*m_ite];

	//Move *m_ite module to new location
	if( -1 != *m_ite )
	{
		m_sol.m_pDB->SetModuleLocation( *m_ite, m_sol.m_currentX, curModule.m_y );
		pSol->m_xlocation.push_back( m_sol.m_currentX );

		//Calculte the new m_currentX
		pSol->m_currentX = m_sol.m_currentX + curModule.m_width;
	}
	else
	{
		pSol->m_xlocation.push_back( m_sol.m_currentX );

		//Calculte the new m_currentX
		pSol->m_currentX = m_sol.m_currentX + m_sol.m_whiteWidth;
	}


	//Calculate the new m_bound
	pSol->m_bound = m_sol.m_bound;	
	pSol->m_net_module_count = m_sol.m_net_module_count;
	if( -1 != *m_ite )
	{
		for( vector<int>::const_iterator iteNetId = curModule.m_netsId.begin() ;
				iteNetId != curModule.m_netsId.end() ; iteNetId++ )
		{
			pSol->m_net_module_count[*iteNetId]--;

			//No module in this net is unplaced
			//Calculate the net length
			if( 0 == pSol->m_net_module_count[*iteNetId] )
			{
				pSol->m_bound += m_sol.m_pDB->GetNetLength( *iteNetId );
			}
		}
	}

	//pSol->PushList( *m_ite );
	return (*pSol);
}

//Count the remain modules of each net in the solution
void CellSwapSolution::InitializeNetModuleCount(void)
{
	for( list<int>::iterator iteModuleId = m_item.begin() ;
			iteModuleId != m_item.end() ; iteModuleId++ )
	{
		if( -1 == *iteModuleId )
			continue;

		for( vector<int>::iterator iteNetId = m_pDB->m_modules[*iteModuleId].m_netsId.begin() ;
				iteNetId != m_pDB->m_modules[*iteModuleId].m_netsId.end() ; iteNetId++ )
		{
			m_net_module_count[*iteNetId] = 0;
		}
	}

	for( list<int>::iterator iteModuleId = m_item.begin() ;
			iteModuleId != m_item.end() ; iteModuleId++ )
	{
		if( -1 == *iteModuleId )
			continue;

		for( vector<int>::iterator iteNetId = m_pDB->m_modules[*iteModuleId].m_netsId.begin() ;
				iteNetId != m_pDB->m_modules[*iteModuleId].m_netsId.end() ; iteNetId++ )
		{
			m_net_module_count[*iteNetId] = m_net_module_count[*iteNetId]+1;
		}
	}

#if 0
	//test code
	cout << "m_net_module_count content: " << endl;
	for( map<int, int>::iterator iteCount = m_net_module_count.begin() ;
			iteCount != m_net_module_count.end() ; iteCount++ )
	{
		printf("map[%d]: %d\n", iteCount->first, iteCount->second );
	}
	//@test code
#endif
}


CellSwapSolution::CellSwapSolution( /*int size*/ CPlaceDB* const pDB )
	//: _rowNum(1), 
	//  _rowWidth(0),
	//  _whiteWidth(0)
{
	// allocate memory
	//m_iist.reserve( size );
	m_pDB = pDB;
}

CellSwapSolution::CellSwapSolution( const CellSwapSolution& sol )
{
	*this = sol;
}

bool CellSwapSolution::IsFeasible() const
{
	return true;
	//if( _rowNum == 1 )
	//return true;
	//else
	//return false;
}

Solution& CellSwapSolution::Clone() const
{
	return reinterpret_cast<Solution&>(*new CellSwapSolution( *this ));
}


double CellSwapSolution::Objective() const
{
	return m_bound;
}

double CellSwapSolution::Bound() const
{
	return m_bound;
}

SolutionIterator& CellSwapSolution::Successors() const
{
	CellSwapSolutionIterator* ite = new CellSwapSolutionIterator(*this);
	return (*ite);
}

void CellSwapSolution::Print() const
{
	cout << "Current: ";
	printf("(%d) ", m_item.size());
	for( list<int>::const_iterator iteModuleId = m_list.begin() ;
			iteModuleId != m_list.end() ; iteModuleId++ )
		cout << *iteModuleId << " ";	
	cout << endl;
	cout << "Location: ";
	for( list<double>::const_iterator iteModuleLocation = m_xlocation.begin() ;
			iteModuleLocation != m_xlocation.end() ; iteModuleLocation++ )
		cout << *iteModuleLocation << " ";
	cout << endl;

	cout << "Bound: " << m_bound << endl;
}
#if 0
double CellSwapSolution::GetCellNetBound( const int& id ) const
{
	printf( "GetCellNetBound\n" );
	vector<int> netsId( m_pDB->GetModuleNets( id ) );
	int size = netsId.size();

	printf( "(%d) ", id );
	for( int i=0; i<size; i++ )
	{
		printf( "%d ", netsId[i] );
	}
	printf( "\n" );

	return 0;
}
#endif

#if 0
double CellSwapSolution::GetCellNetLength( const int& id ) const
{
	return 0;
}
#endif

void CellSwapSolution::RecalculateBound(void)
{
	if( IsComplete() )
	{
		set<int> net_set;

		//Collet all involved net id
		for( list<int>::iterator iteModuleId = m_list.begin() ;
				iteModuleId != m_list.end() ; iteModuleId++ )
		{
			//Skip Whitespace
			if( -1 == *iteModuleId )
				continue;

			for( vector<int>::iterator iteNetId = m_pDB->m_modules[*iteModuleId].m_netsId.begin() ;
					iteNetId != m_pDB->m_modules[*iteModuleId].m_netsId.end() ; iteNetId++ )
			{
				net_set.insert( *iteNetId );
			}
		}

		//Calculate the involved wirelength
		m_bound = 0;
		for( set<int>::iterator iteNetId = net_set.begin() ;
				iteNetId != net_set.end() ; iteNetId++ )
		{
			m_bound += m_pDB->GetNetLength( *iteNetId ); 
		}
	}
}
