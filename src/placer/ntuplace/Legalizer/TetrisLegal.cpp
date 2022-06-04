#include "TetrisLegal.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <list>
#include "../PlaceCommon/placebin.h"
#include "../PlaceCommon/ParamPlacement.h"
//#include "macrolegal.h"


//test code
//#include "cellmoving.h"
//#include "WhiteSpaceAllocation.h"
//#include "CongMap.h"
//@test code

//Added by Jin 20060926
//#include "CongMap.h"
//#include "GlobalRouting.h"

//Added by Jin 20070403
//#include "CellSlidingForCongestion.h"

//Added by kaie
#include "../lib/lpsolve55/lp_lib.h"

using namespace std;
using namespace Jin;

CPlaceDB* LessXCoor::m_placedb = 0;
CPlaceDB* LessXCoorMacroFirst::m_placedb = 0;
CPlaceDB* LessXCoorMacroPrior::m_placedb = 0;
double LessXCoor::m_macro_factor = 0.001;

void CTetrisLegal::ReverseFreeSites(void)
{
    vector<CSiteRow> orig_sites = m_free_sites;
    m_free_sites.clear();
    
    //Reverse m_free_sites
    for( unsigned int iRow = 0 ; iRow < orig_sites.size() ; iRow++ )
    {
	CSiteRow& sourceRow = orig_sites[iRow];
	//CSiteRow& curRow = m_free_sites[iRow];

	m_free_sites.push_back( CSiteRow( sourceRow.m_bottom, sourceRow.m_height, sourceRow.m_step ) );

	vector<double>& reverse_interval = m_free_sites.back().m_interval;
	reverse_interval.reserve( sourceRow.m_interval.size() );
	for( vector<double>::reverse_iterator iteInterval = sourceRow.m_interval.rbegin() ;
		iteInterval != sourceRow.m_interval.rend() ; iteInterval++ )
	{
	    //curRow.m_interval.push_back( -(*iteInterval) );
	    reverse_interval.push_back( -(*iteInterval) );	
	}	
    }
}

vector<CSiteRow> CTetrisLegal::ExtractSites( const vector<CSiteRow>& source_sites,
	const CRect& region )
{
    int start_row_index, end_row_index;
    start_row_index = static_cast<int>( ceil( ( region.bottom - source_sites.front().m_bottom ) / 
	source_sites.front().m_height ) );
    end_row_index = static_cast<int>( floor( ( region.top - source_sites.front().m_bottom ) / 
	source_sites.front().m_height ) );

    start_row_index = min( start_row_index, static_cast<int>(source_sites.size()) - 1 );
    start_row_index = max( start_row_index, 0 );
    end_row_index = min( end_row_index, static_cast<int>(source_sites.size()) - 1 );
    end_row_index = max( end_row_index, 0 );

    vector<CSiteRow> result_sites;
    
    for( int row_index = start_row_index ; row_index <= end_row_index ; row_index++ )
    {
	const CSiteRow& curRow = source_sites[ row_index ];
	result_sites.push_back( CSiteRow( curRow.m_bottom, curRow.m_height, curRow.m_step ) );

	for( unsigned int iInterval = 0 ; 
		iInterval < curRow.m_interval.size() ; 
		iInterval = iInterval+2 )
	{
	    double left = curRow.m_interval[iInterval];
	    double right = curRow.m_interval[iInterval+1];
	    
	    if( left >= region.right )
		break;
	    else if ( right <= region.left )
		continue;

	    left = max( left, region.left );
	    right = min( right, region.right );
	    
	    result_sites.back().m_interval.push_back( left );
	    result_sites.back().m_interval.push_back( right );
	}
    }

    return result_sites;
}

void CTetrisLegal::ApplySites( const vector<CSiteRow>& source_sites,
	const CRect& region,
	vector<CSiteRow>& target_sites )
{
    int first_target_row_index;
    first_target_row_index = static_cast<int>( ceil( ( source_sites.front().m_bottom - target_sites.front().m_bottom ) / 
	target_sites.front().m_height ) );
    
    first_target_row_index = min( first_target_row_index, static_cast<int>(target_sites.size()) - 1 );
    first_target_row_index = max( first_target_row_index, 0 );

    for( unsigned int row_index = 0 ; row_index < source_sites.size() ; row_index++ )
    {
	const CSiteRow& curSourceRow = source_sites[ row_index ];
	CSiteRow& curTargetRow = target_sites[ row_index + first_target_row_index ];

	if( curTargetRow.m_interval.empty() )
	{
	    continue;
	}
	
	//test code
	if( curSourceRow.m_bottom != curTargetRow.m_bottom )
	{
	    fprintf( stderr, "Warning: curSourceRow.m_bottom (%.2f) != curTargetRow.m_bottom (%.2f)\n",
		  curSourceRow.m_bottom, curTargetRow.m_bottom );
	}
	//@test code
	
	vector<double> orig_interval = curTargetRow.m_interval;
	curTargetRow.m_interval.clear();
	
	//i1: the last interval index before the region
	//i2: the first interval index after the region
	int i1 = -2;
	int i2 = orig_interval.size();

	for( unsigned int i = 0 ; i < orig_interval.size() ; i = i+2 )
	{
	    double right = orig_interval[i+1];
	    if( right < region.left )
		i1 = i;
	    else
		break;
	}
	
	for( int i = static_cast<int>( orig_interval.size() ) - 2 ; i > -1 ; i = i-2 )
	{
	    double left = orig_interval[i];
	    if( left > region.right )
		i2 = i;
	    else
		break;
	}
	
	
	//Add all front intervals into curTargetRow
	for( int i = 0 ; i <= i1+1 ; i++ )
	{
	    curTargetRow.m_interval.push_back( orig_interval[i] );
	}

	//Add the intervals of source row into target row
	vector<double> temp_interval = curSourceRow.m_interval;
       	
	//i1: the last interval index before the region
	if( i1 < static_cast<int>( orig_interval.size() ) - 2 )
	{
	    double left = orig_interval[ i1 + 2 ];
	    double right = orig_interval[ i1 + 3 ];
	    
	    if( left < region.left )
	    {
		right = min( right, region.left );

		if( !temp_interval.empty() && right == temp_interval.front() )
		{
		    temp_interval.front() = left;
		}
		else
		{
		    curTargetRow.m_interval.push_back( left );
		    curTargetRow.m_interval.push_back( right );
		}
	    }
	}

	//i2: the first interval index after the region
	if( i2 > 1 )
	{
	    double left = orig_interval[ i2 - 2 ];
	    double right = orig_interval[ i2 - 1 ];
	    
	    if( right > region.right )
	    {
		left = max( left, region.right );

		if( !temp_interval.empty() && left ==  temp_interval.back() )
		{
		    temp_interval.back() = right;
		}
		else
		{
		    temp_interval.push_back( left );
		    temp_interval.push_back( right );
		}
	    }
	}

	for( unsigned int i = 0 ; i < temp_interval.size() ; i++ )
	{
	    curTargetRow.m_interval.push_back( temp_interval[i] );
	}
	
	//Add all back intervals into curTargetRow
	for( unsigned int i = i2 ; i < orig_interval.size() ; i++ )
	{
	    curTargetRow.m_interval.push_back( orig_interval[i] );
	}
	
	
#if 0
	for( unsigned int iTargetInterval = 0 ; 
		iTargetInterval < curTargetRow.m_interval.size() ;
		iTargetInterval = iTargetInterval+2 )
	{
	    double target_left = curTargetRow.m_interval[ iTargetInterval ];
	    double target_right = curTargetRow.m_interval[ iTargetInterval+1 ];

	    if( target_right <= region.left )
		continue;
	    else if( target_left >= region.right )
		break;

	    //remove sites inside the region
	    if( target_left < region.left && target_right > region.right )
	    {
		curTargetRow.m_interval.insert( 
			curTargetRow.m_interval.begin() + (iTargetInterval+1),
			target_right );
		curTargetRow.m_interval.insert( 
			curTargetRow.m_interval.begin() + (iTargetInterval+1),
			region.right );
		curTargetRow.m_interval[ iTargetInterval+1 ] = region.left;
	    }
	}
#endif
    }
	    
}


bool CTetrisLegal::SolveRegion( const CRect& region,
	const std::vector<int>& m_module_ids,
	const double& util,
	const double& stop_prelegal_factor )
{
    //Expand cell width to site step
    SaveModuleWidth();
    ExpandModuleWidthToSiteStep();
    
    m_bMacroLegalized = false;

    //backup m_sites, extract m_sites by the given region
    vector<CSiteRow> orig_sites = m_free_sites;
    m_free_sites = ExtractSites( orig_sites, region );
    m_site_bottom = m_free_sites.front().m_bottom;
   
    //SaveGlobalResult();
    PrepareLeftRightFreeSites();

    bool bFinalLegal = false; 
    //SetProcessList();
    m_process_list = m_module_ids;
    bFinalLegal = DoLeftRightUntil( util, stop_prelegal_factor );
    
    RestoreModuleWidth();

    //restore m_sites and apply the resulting sites on m_sites
    if( bFinalLegal )
    {
	//TetrisLegal must record the final sites status	
	ApplySites( m_free_sites, region, orig_sites );
	m_free_sites = orig_sites;
	m_site_bottom = orig_sites.front().m_bottom;
    }
    else
    {
	m_free_sites = orig_sites;
	m_site_bottom = orig_sites.front().m_bottom;
    }
	
    //if( !bFinalLegal )
    //	RestoreGlobalResult();
    
    return bFinalLegal;
}

bool CTetrisLegal::RemoveOverlap(void)
{
    double orig_left_factor = m_left_factor;
    m_left_factor = 0.0;
    SetProcessList();
    SaveGlobalResult();
    PrepareLeftRightFreeSites();
    SaveOrig();

    if( true )
    {	

	if( gArg.IsDev() )
	{
	    cout << "Remove overlapping: " << endl;
	}

	double start = 1.0;
	double step = 0.01;
	double stop = 0.05;

	while( start >= stop )
	{
	    double t = seconds();

	    SetLeftFreeSites();
	    RestoreOrig();

	    if( gArg.IsDev() )
	    {
		cout << "factor: " << start << " ";
	    }

	    bool final = DoLeftMacroFirst(start);

	    if( gArg.IsDev() )
	    {
		if( final )
		    cout << "Success ";
		else
		    cout << "Fail ";
		printf(" runtime %.2f secs\n", seconds()-t );
		flush(cout);
	    }
	    else
	    {
		cout << "*";
	    }

	    if( final )
	    {
		return final;
	    }
	    else
	    {
		start = start - step;
	    }
	}
    }


    m_left_factor = orig_left_factor;

    return false;
}

bool CTetrisLegal::DoLeftRightUntil( const double& util, const double& stop_prelegal_factor )
{
    const double search_step = 0.01;
    //const double search_bound = 0.99;
    const double search_bound = stop_prelegal_factor;

    double prelegal_factor = m_prelegal_factor + search_step;

    bool bFinish = false;
    bool bLegalLeft = false;

    SaveOrig();

    
    //Create CPlaceBin to compute density penalty
    CPlaceBin* pBin = NULL;

    if( !gArg.CheckExist( "cong" ) )
    {
	pBin = new CPlaceBin( m_placedb );
	pBin->CreateGrid( m_placedb.m_rowHeight * 10.0, m_placedb.m_rowHeight * 10.0 );
    }


    if( gArg.IsDev() )
    {
	cout << "Legalize to left" << endl;	
    }

    m_best_cost = numeric_limits<double>::max();
    
    while( (!bFinish) && (prelegal_factor > search_bound) )
    {
	// 2007-07-10 (donnie)
	if( m_tetrisDir != 0 && m_tetrisDir != 1 )
	    break;  // no right Tetris
	
	prelegal_factor = prelegal_factor - search_step;
	//cout << "Legalization factor: " << prelegal_factor << endl; // by donnie
	if( gArg.IsDev() )
	    cout << "Legalization factor: " << prelegal_factor << " ";

	double t = seconds();

	RestoreOrig();
	SetLeftFreeSites();

	bool result = DoLeft( prelegal_factor );
	//bool result = DoLeftMacroFirst( prelegal_factor );

	//If DoLeft() successes, check if the solution is bettern than previous
	if( result )
	{
	    double cost;

	    // if( gArg.CheckExist( "cong" ) )
	    // {
		// int gridNum = static_cast<int>( sqrt( static_cast<double>( m_placedb.m_modules.size()) ) * 0.8 );
		// CCongMap congmap( gridNum, gridNum, m_placedb.m_coreRgn, m_placedb.m_pLEF );
		// CGlobalRouting groute( m_placedb );
		// groute.ProbalisticMethod( congmap );

		// //cost = congmap.GetTotalOverflow();	
		// cost = congmap.GetMaxOverflow();	

		// if( gArg.IsDev() )
		// {
		//     //printf(" TotalOverFlow %.2f ", cost );
		//     printf(" MaxOverFlow %.2f ", cost );
		// }

		// char filename[200];
		// sprintf( filename, "left.overflow.%.2f", prelegal_factor );
		// congmap.OutputBinOverflowFigure( filename );
		
		// //test code
		// if( gArg.CheckExist( "std_dev" ) )
		// {
		//     double displacement = 0.0;
		//     for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
		//     {
		// 	double newX = m_placedb.m_modules[i].m_x;
		// 	double newY = m_placedb.m_modules[i].m_y;
		// 	double oldX = m_origLocations[i].x;
		// 	double oldY = m_origLocations[i].y;

		// 	displacement += abs( newX - oldX ) + abs( newY - oldY );
		//     }
		//     cost = displacement;
		//     fprintf( stdout, " displacement %.0f ", displacement );

		//     double mean_displacement = displacement / static_cast<double>( m_placedb.m_modules.size() );
		//     double variance = 0.0;
		//     for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
		//     {
		// 	double newX = m_placedb.m_modules[i].m_x;
		// 	double newY = m_placedb.m_modules[i].m_y;
		// 	double oldX = m_origLocations[i].x;
		// 	double oldY = m_origLocations[i].y;

		// 	double shift = abs( newX - oldX ) + abs( newY - oldY );
		// 	variance += pow( shift - mean_displacement, 2.0 );
		//     }
		//     variance = variance / static_cast<double>( m_placedb.m_modules.size() );
		//     double std_deviaton = sqrt( variance );
		//     fprintf( stdout, "standard_deviation %f ", std_deviaton );
		//     cost = std_deviaton;
		// }
		// //@test code
	    // }
	    // else
	    {		
		// 2006-08-25 (donnie) Use X-HPWL
		double wirelength;
		//if( param.bXArch )
		if( gArg.CheckExist( "x" ) )
		{
		    wirelength = m_placedb.CalcXHPWL();
		}
		//use Steiner wirelength as cost
		// else if( gArg.CheckExist( "steineropt" ) )
		// {
		//     wirelength = m_placedb.GetTotalSteinerWL( 0, 0 ); 
		// }   
		else
		{
		    wirelength = m_placedb.CalcHPWL();
		}

		cost = wirelength;	

		if( gArg.IsDev() )
		{
		    printf(" oHPWL %.0f ", wirelength);
		}

		if( util > 0 )
		{
		    pBin->UpdateBinUsage();
		    double penalty = pBin->GetPenalty( util );
		    cost = cost * (1.0+(penalty/100.0));
		    if( gArg.IsDev() )
		    {
			printf( " (p = %.2f) ", penalty );
		    }
		}

	    }

	    if( gArg.IsDev() )   // by donnie
	    {
		cout << "Success " << cost << " ";
		printf("runtime %.2f secs\n", seconds()-t);
	    }
	    else
	    {
		printf( "*" );
	    }
	    flush(cout);
	    if( cost < m_best_cost )	
	    {
		SaveBest( prelegal_factor, cost );
		//For congestion mode
		//the process stops when a first legalized result is found
		/*if( param.bCong )
		{
		    bFinish = true;
		}*/
	    }
	    //Be legalized before,
	    //and find a worse cost now:
	    //the search is terminated 
	    else if( !gArg.CheckExist( "nostop" ) && bLegalLeft )
	    {
		bFinish = true;
	    }
	}
	else
	{
	    if( gArg.IsDev() )
	    {
		cout << "Fail at "; 
		const int unlegal_cellindex = m_cell_order.size() - m_unlegal_count;
		printf("(%d)th module %d (%.2f,%.2f) width: %.2f height: %.2f ",
			unlegal_cellindex,
			m_cell_order[unlegal_cellindex], 
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_x,
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_y,
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_width,
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_height );
		printf("runtime %.2f secs\n", seconds()-t);
	    }
	    else
	    {
		printf( "-" );
	    }
	    flush(cout);

	    //Be legalized before,
	    //and find a unlegalized solution now:
	    //the search is terminated
	    if( !gArg.CheckExist( "nostop" ) && bLegalLeft )
	    {
		bFinish = true;
	    }
	}

	bLegalLeft = bLegalLeft | result;


	//configuration to legalize until first success
	if( gArg.IsDev() && gArg.CheckExist( "legalToFirstSuccess" ) && bLegalLeft )
	{
	    bFinish = true;
	}
    }


    //Record the best solution for left
    if(bLegalLeft )
    {
	m_best_prelegal_factor_left = m_best_prelegal_factor;
	m_best_cost_left = m_best_cost;
	m_bestLocations_left = m_bestLocations;
	m_best_sites_left = m_best_sites;
    }

    RestoreOrig();
    ReversePlacement();
    ReverseLegalizationData();
    SaveOrig();
    
    if( !gArg.CheckExist( "cong" ) )
    {
	delete pBin;
	pBin = new CPlaceBin( m_placedb );
	pBin->CreateGrid( m_placedb.m_rowHeight * 10.0, m_placedb.m_rowHeight * 10.0 );
    }

    //test code
    //ofile = fopen( "reverse_site.log", "w" );
    //for( unsigned int i = 0 ; i < m_placedb.m_sites.size() ; i++ )
    //{
    //    const CSiteRow& curRow = m_placedb.m_sites[i];
    //    
    //    fprintf(ofile,"bottom: %.2f ", curRow.m_bottom );
    //    for( unsigned int j = 0 ; j < curRow.m_interval.size() ; j=j+2 )
    //    {
    //	fprintf(ofile, "(%.2f,%.2f) ", curRow.m_interval[j], curRow.m_interval[j+1] );
    //    }
    //    fprintf(ofile, "\n");
    //}
    //fclose( ofile );
    //@test code


    bFinish = false;	
    bool bLegalRight = false;
    prelegal_factor = m_prelegal_factor + search_step;
    m_best_cost = numeric_limits<double>::max();
    if( gArg.IsDev() )
	cout << "Legalize to right" << endl;	
    while( (!bFinish) && (prelegal_factor > search_bound) )
    {
	// 2007-07-10 (donnie)
	if( m_tetrisDir != 0 && m_tetrisDir != 2 )
	    break;  // no right Tetris
	
	prelegal_factor = prelegal_factor - search_step;
	//cout << "Legalization factor: " << prelegal_factor << endl; // by donnie
	if( gArg.IsDev() )
	    cout << "Legalization factor: " << prelegal_factor << " ";

	double t = seconds();

	RestoreOrig();
	SetRightFreeSites();

	bool result = DoRight( prelegal_factor );

	//If DoRight() successes, check if the solution is bettern than previous
	if( result )
	{
	    double cost;

	    // if( gArg.CheckExist( "cong" ) )
	    // {
		// int gridNum = static_cast<int>( sqrt( static_cast<double>( m_placedb.m_modules.size()) ) * 0.8 );
		// CCongMap congmap( gridNum, gridNum, m_placedb.m_coreRgn, m_placedb.m_pLEF );
		// CGlobalRouting groute( m_placedb );
		// groute.ProbalisticMethod( congmap );

		// //cost = congmap.GetTotalOverflow();	
		// cost = congmap.GetMaxOverflow();	

		// if( gArg.IsDev() )
		// {
		//     //printf(" TotalOverFlow %.2f ", cost );
		//     printf(" MaxOverFlow %.2f ", cost );
		// }
		
		// //test code
		// if( gArg.CheckExist( "std_dev" ) )
		// {
		//     char filename[200];
		//     sprintf( filename, "right.overflow.%.2f", prelegal_factor );
		//     congmap.OutputBinOverflowFigure( filename );

		//     double displacement = 0.0;
		//     for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
		//     {
		// 	double newX = m_placedb.m_modules[i].m_x;
		// 	double newY = m_placedb.m_modules[i].m_y;
		// 	double oldX = m_origLocations[i].x;
		// 	double oldY = m_origLocations[i].y;

		// 	displacement += abs( newX - oldX ) + abs( newY - oldY );
		//     }
		//     cost = displacement;
		//     fprintf( stdout, "displacement %.0f ", displacement );

		//     double mean_displacement = displacement / static_cast<double>( m_placedb.m_modules.size() );
		//     double variance = 0.0;
		//     for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
		//     {
		// 	double newX = m_placedb.m_modules[i].m_x;
		// 	double newY = m_placedb.m_modules[i].m_y;
		// 	double oldX = m_origLocations[i].x;
		// 	double oldY = m_origLocations[i].y;

		// 	double shift = abs( newX - oldX ) + abs( newY - oldY );
		// 	variance += pow( shift - mean_displacement, 2.0 );
		//     }
		//     variance = variance / static_cast<double>( m_placedb.m_modules.size() );
		//     double std_deviaton = sqrt( variance );
		//     fprintf( stdout, "standard_deviation %f ", std_deviaton );
		//     cost = std_deviaton;
		// }
		// //@test code
	    // }
	    // else
	    {
  
		// 2006-08-25 (donnie) Use X-HPWL
		double wirelength;
		//if( param.bXArch )
		if( gArg.CheckExist( "x" ) )
		{
		    wirelength = m_placedb.CalcXHPWL();
		}
		//use Steiner wirelength as cost
		// else if( gArg.CheckExist( "steineropt" ) )
		// {
		//     wirelength = m_placedb.GetTotalSteinerWL( 0, 0 ); 
		// }   
		else
		{
		    wirelength = m_placedb.CalcHPWL();
		}
		
		cost = wirelength;

		if( gArg.IsDev() )
		{
		    printf(" oHPWL %.0f ", wirelength);
		}


		if( util > 0 )
		{
		    pBin->UpdateBinUsage();
		    double penalty = pBin->GetPenalty( util );
		    cost = cost * (1.0+(penalty/100.0));

		    if( gArg.IsDev())
		    {
			printf( " (p = %.2f) ", penalty );
		    }
		}

	    }
	    
	    if( gArg.IsDev() )
	    {
		cout << "Success " << cost << " ";
		printf("runtime %.2f secs\n", seconds()-t);
	    }
	    else
	    {
		printf( "*" );
	    }
	    flush(cout);
	    if( cost < m_best_cost )	
	    {
		SaveBest( prelegal_factor, cost );
		
		//For congestion mode
		//the process stops when a first legalized result is found
		/*if( param.bCong )
		{
		    bFinish = true;
		}*/

	    }
	    //Be legalized before,
	    //and find a worse cost now:
	    //the search is terminated 
	    else if( !gArg.CheckExist( "nostop" ) && bLegalRight )
	    {
		bFinish = true;
	    }
	}
	else
	{
	    if( gArg.IsDev() )
	    {
		cout << "Fail at "; 
		const int unlegal_cellindex = m_cell_order.size() - m_unlegal_count;
		printf("(%d)th module %d (%.2f,%.2f) width: %.2f height: %.2f ",
			unlegal_cellindex,
			m_cell_order[unlegal_cellindex], 
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_x,
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_y,
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_width,
			m_placedb.m_modules[m_cell_order[unlegal_cellindex]].m_height );
		printf("runtime %.2f secs\n", seconds()-t);
	    }
	    else
	    {
		printf( "-" );
	    }
	    flush(cout);


	    //Be legalized before,
	    //and find a unlegalized solution now:
	    //the search is terminated
	    if( !gArg.CheckExist( "nostop" ) && bLegalRight )
	    {
		bFinish = true;
	    }
	}

	bLegalRight = bLegalRight | result;

	//configuration to legalize until first success
	if( gArg.IsDev() && gArg.CheckExist( "legalToFirstSuccess" ) && bLegalRight )
	{
	    bFinish = true;
	}
    }

    //test code
    //m_placedb.OutputGnuplotFigureWithZoom( "right", false, true, true );
    //@test code

    bool bFinalLegal = bLegalLeft | bLegalRight;

    //Restore m_coreRgn
    ReverseLegalizationData();

    //Left is better
    if( (bLegalLeft && !bLegalRight) ||
    	    (bLegalLeft && bLegalRight && m_best_cost_left < m_best_cost ) )
    {
	m_best_prelegal_factor = m_best_prelegal_factor_left;
	m_best_cost = m_best_cost_left;
	m_bestLocations = m_bestLocations_left;
	m_best_sites = m_best_sites_left;

	if( gArg.IsDev() )
	{
	    cout << "Best factor: Left " << m_best_prelegal_factor << endl;
	}
	RestoreBest();
	//m_placedb.RemoveMacroSite();
    
	m_resultTetrisDir = 1; // 2007-07-10 (donnie)
	m_resultTetrisDiff =  ( m_best_cost - m_best_cost_left ) / m_best_cost;
    }
    //Right is better
    else if( (!bLegalLeft && bLegalRight) ||
    	    (bLegalLeft && bLegalRight && m_best_cost_left >= m_best_cost ) ) 
    {
	if( gArg.IsDev() )
	{
	    cout << "Best factor: Right " << m_best_prelegal_factor << endl;
	}
	RestoreBest();
	ReverseFreeSites();
	ReversePlacement();
	//m_placedb.RemoveMacroSite();
	m_resultTetrisDir = 2; // 2007-07-10 (donnie)
	m_resultTetrisDiff = ( m_best_cost_left - m_best_cost ) / m_best_cost_left;
    }
    else
    {
	ReversePlacement();
	if( gArg.IsDev() )
	{
	    cout << "Warning: general legalization fail" << endl;
	    ReversePlacement();
	    string prefix = param.outFilePrefix + "_legal_fail";
	    //m_placedb.OutputGnuplotFigureWithZoom( prefix.c_str(), false, true, true );
	}

	RestoreOrig();
    }

    if( !gArg.CheckExist( "cong" ) )
    {
	delete pBin;
	pBin = NULL;
    }

    return bFinalLegal;
}

void CTetrisLegal::SetNonMacroProcessList( const vector<int>& macro_ids )
{
    set<int> macro_sets;

    for( unsigned int i = 0 ; i < macro_ids.size() ; i++ )
    {
	macro_sets.insert( macro_ids[i] );
    }

    m_process_list.clear();

    set<int>::iterator iteSet = macro_sets.begin();

    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	//This module is in macro_ids
	//Do not add into the process list
	if( iteSet != macro_sets.end() && static_cast<int>(i) == *iteSet )
	{
	    iteSet++;
	    continue;
	}	    

	const Module& curModule = m_placedb.m_modules[i];

	//Don't add fixed module
	if( curModule.m_isFixed )
	{
	    continue;
	}

	m_process_list.push_back(i);
    }
}

void CTetrisLegal::SetProcessList(void)
{
    m_process_list.clear();

    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	if( !m_placedb.m_modules[i].m_isFixed )
	    m_process_list.push_back(i);
    }

}

void CTetrisLegal::MacroShifterSaveOrigPosition(void)
{
    m_macro_shifter_orig_positions.clear();
    m_macro_shifter_orig_positions.reserve( m_macro_ids.size() );
    for( unsigned int i = 0 ; i < m_macro_ids.size() ; i++ )
    {
	const Module& curModule = m_placedb.m_modules[ m_macro_ids[i] ];
	m_macro_shifter_orig_positions.push_back( CPoint( curModule.m_x, curModule.m_y ) );
	m_origLocations[ m_macro_ids[i] ] = CPoint( curModule.m_x, curModule.m_y );
    }
}

void CTetrisLegal::MacroShifterRestoreOrigPosition(void)
{
    for( unsigned int i = 0 ; i < m_macro_ids.size() ; i++ )
    {
	m_placedb.SetModuleLocation( m_macro_ids[i], 
		m_macro_shifter_orig_positions[i].x, 
		m_macro_shifter_orig_positions[i].y );
    }

}

void CTetrisLegal::MacroShifterSaveBestPosition(void)
{
    m_macro_shifter_best_positions.clear();
    m_macro_shifter_best_positions.reserve( m_macro_ids.size() );
    for( unsigned int i = 0 ; i < m_macro_ids.size() ; i++ )
    {
	const Module& curModule = m_placedb.m_modules[ m_macro_ids[i] ];
	m_macro_shifter_best_positions.push_back( CPoint( curModule.m_x, curModule.m_y ) );
    }

}

void CTetrisLegal::MacroShifterRestoreBestPosition(void)
{
    for( unsigned int i = 0 ; i < m_macro_ids.size() ; i++ )
    {
	m_placedb.SetModuleLocation( m_macro_ids[i], 
		m_macro_shifter_best_positions[i].x, 
		m_macro_shifter_best_positions[i].y );
    }

}

bool CTetrisLegal::AggressiveCellLegalLocationsSearch( const int& cellid,
	vector<CLegalLocation>& legalLocations )
{
    double site_step = m_placedb.m_sites[0].m_step;

    const Module& curModule = m_placedb.m_modules[cellid];

    if( curModule.m_height > m_placedb.m_rowHeight )
    {
	fprintf(stderr, "Warning: module %d is a macro and should not be processed "
		"by AggressiveCellLegalLocationsSearch()\n", cellid );
    }

    int row_limit;
    if( m_row_limit < 1 )
    {
	row_limit = static_cast<int>(ceil(curModule.m_height/m_placedb.m_rowHeight)*m_row_factor);
    }
    else
    {
	row_limit = m_row_limit;	
    }

    double xbound = ceil( curModule.m_x - (m_left_factor*m_average_cell_width) );
    xbound = floor ( ( ( xbound + (site_step/2.0) ) / site_step) ) * site_step;

    int upward_row_start, upward_row_end;
    int downward_row_start, downward_row_end;

    //Compute the cell located row index
    int cell_row_index = GetSiteIndex( curModule.m_y );

    if( 0 == cell_row_index )
    {
	upward_row_start = cell_row_index + 1;
	downward_row_start = cell_row_index;
    }
    else
    {
	upward_row_start = cell_row_index;
	downward_row_start = cell_row_index - 1;
    }

    const int max_site_index = static_cast<int>(m_free_sites.size())-1;
    upward_row_end = min( max_site_index, upward_row_start + row_limit );
    downward_row_end = max( 0, downward_row_start - row_limit );

    bool bContinue1;
    bool bContinue2;

    double min_shift = numeric_limits<double>::max();

    do
    {
	//Upward Search
	if( upward_row_start <= max_site_index )
	{
	    //Expand the search row end
	    upward_row_end = min( max_site_index, upward_row_start + row_limit );
	    vector<CLegalLocation> search_results;

	    GetCellLegalLocationsTowardLeft( cellid,
		    upward_row_start,
		    upward_row_end,
		    search_results,
		    xbound );

	    if( !search_results.empty() )
	    {
		min_shift = min( min_shift, ReturnMinimumShift( cellid, search_results ) );
		legalLocations.insert( legalLocations.end(), 
			search_results.begin(),
			search_results.end() );
	    }
	}

	//Downward Search
	if( downward_row_start >= 0 )
	{	
	    //Expand the search row end
	    downward_row_end = max( 0, downward_row_start - row_limit );
	    vector<CLegalLocation> search_results;

	    GetCellLegalLocationsTowardLeft( cellid,
		    downward_row_start,
		    downward_row_end,
		    search_results,
		    xbound );

	    if( !search_results.empty() )
	    {
		min_shift = min( min_shift, ReturnMinimumShift( cellid, search_results ) );
		legalLocations.insert( legalLocations.end(), 
			search_results.begin(),
			search_results.end() );
	    }
	}


	//Update the start row of next searching
	upward_row_start = upward_row_end + 1;
	downward_row_start = downward_row_end - 1;

	//Continue status 1: min_shift > max_search_range
	int max_search_row_number = max( upward_row_end - cell_row_index, 
		cell_row_index - downward_row_end );
	double max_search_range = 
	    static_cast<double>(max_search_row_number)*(m_placedb.m_rowHeight);
	bContinue1 = (min_shift > max_search_range);

	//Continue status 2: there are unsearched rows
	bContinue2 = (upward_row_start <= max_site_index) || (downward_row_start >= 0);

    }
    while(bContinue1 && bContinue2);

    if( legalLocations.empty() )
	return false;
    else
	return true;	
}

double CTetrisLegal::ReturnMinimumShift( const int& cellid,
	const vector<CLegalLocation>& locations )
{
    if( locations.empty() )
    {
	cerr << "Warning: 'locations' are empty in ReturnMinimumShift()" << endl;
	return -1;
    }

    double min_shift = numeric_limits<double>::max();
    CPoint p1( m_placedb.m_modules[cellid].m_x, m_placedb.m_modules[cellid].m_y );
    //CPoint p1 = m_origLocations[cellid];

    for( vector<CLegalLocation>::const_iterator iteLoc = locations.begin();
	    iteLoc != locations.end() ; iteLoc++ )
    {
	CPoint p2( iteLoc->m_xcoor, m_free_sites[iteLoc->m_site_index].m_bottom );
	min_shift = min( min_shift, CPoint::Distance( p1, p2 ) );
    }

    return min_shift;
}

void CTetrisLegal::GetCellLegalLocationsTowardLeft( const int& cellid,
	int start_site_index, 
	int end_site_index,
	vector<CLegalLocation>& legalLocations,
	const double& left_bound )
{
    double site_step = m_placedb.m_sites[0].m_step;
    if( start_site_index > end_site_index )
	swap( start_site_index, end_site_index );

    if( start_site_index < 0 || 
	    end_site_index >= static_cast<int>(m_free_sites.size()) ) 
    {
	fprintf( stderr, "Illegal start_site_index and end_site_index in "
		"GetCellLegalLocationsTowardLeft() (%d,%d)\n", 
		start_site_index, end_site_index );
    }

    start_site_index = max(0, start_site_index);
    end_site_index = min( static_cast<int>(m_free_sites.size())-1, end_site_index );

    const Module& curModule = m_placedb.m_modules[cellid];

    if( curModule.m_height > m_placedb.m_rowHeight )
    {
	fprintf( stderr, "Module %d is a macro and should not be processed by "
		"GetCellLegalLocationsTowardLeft()\n", cellid );
    }

    legalLocations.clear();


    for( int iRow = start_site_index ; iRow <= end_site_index ; iRow++ )
    {
	const CSiteRow& curRow = m_free_sites[iRow];	

	for( unsigned int iInterval = 0 ; 
		iInterval < curRow.m_interval.size() ; 
		iInterval = iInterval + 2 )
	{
	    double xstart = curRow.m_interval[iInterval];
	    double xend = curRow.m_interval[iInterval+1];
	    
	    //xstart = floor ( ( ( xstart + (site_step/2.0) ) / site_step) );
	    //xend = floor ( ( ( xend - (site_step/2.0) ) / site_step) );
	    xstart = ceil ( ( xstart / site_step) ) * site_step;
	    xend = floor ( (  xend  / site_step) ) * site_step;
	    
	    //Discard illegal intervals
	    if( xend < left_bound )
		continue;

	    xstart = max( xstart, left_bound );
	    //Check if this interval has enough width
	    if( xend - xstart >= curModule.m_width )
	    {
		legalLocations.push_back( CLegalLocation( iRow, xstart ) );
		break;
	    }	
	}	
    }
}

bool Jin::LessXCoorMacroPrior::BL( const int& mid1, const int& mid2 )
{
    const Module& m1 = m_placedb->m_modules[mid1];
    const Module& m2 = m_placedb->m_modules[mid2];

    if( m1.m_area != m2.m_area )
    	return m1.m_area > m2.m_area;

    double cost1 = m1.m_cx + m1.m_cy - 0.5 * m1.m_width - 0.5 * m1.m_height;
    double cost2 = m2.m_cx + m2.m_cy - 0.5 * m2.m_width - 0.5 * m2.m_height;
    //double cost1 = m1.m_x + m1.m_y;
    //double cost2 = m2.m_x + m2.m_y;

    return cost1 < cost2;
}

bool Jin::LessXCoorMacroPrior::BR( const int& mid1, const int& mid2 )
{
    const Module& m1 = m_placedb->m_modules[mid1];
    const Module& m2 = m_placedb->m_modules[mid2];

    if( m1.m_area != m2.m_area )
    	return m1.m_area > m2.m_area;

    double cost1 = (-m1.m_cx) + m1.m_cy - 0.5 * m1.m_width - 0.5 * m1.m_height;
    double cost2 = (-m2.m_cx) + m2.m_cy - 0.5 * m2.m_width - 0.5 * m2.m_height;
    //double cost1 = (m1.m_x+m1.m_width) + m1.m_y;
    //double cost2 = (m2.m_x+m2.m_width) + m2.m_y;

    return cost1 < cost2;
}

bool Jin::LessXCoorMacroPrior::TL( const int& mid1, const int& mid2 )
{
    const Module& m1 = m_placedb->m_modules[mid1];
    const Module& m2 = m_placedb->m_modules[mid2];

    if(m1.m_area != m2.m_area)
    	return m1.m_area > m2.m_area;

    double cost1 = m1.m_cx + (-m1.m_cy) - 0.5 * m1.m_width - 0.5 * m1.m_height;
    double cost2 = m2.m_cx + (-m2.m_cy) - 0.5 * m2.m_width - 0.5 * m2.m_height;
    //double cost1 = m1.m_x + (m1.m_y+m1.m_height);
    //double cost2 = m2.m_x + (m2.m_y+m2.m_height);

    return cost1 < cost2;
}

bool Jin::LessXCoorMacroPrior::TR( const int& mid1, const int& mid2 )
{
    const Module& m1 = m_placedb->m_modules[mid1];
    const Module& m2 = m_placedb->m_modules[mid2];

    if(m1.m_area != m2.m_area)
    	return m1.m_area > m2.m_area;

    double cost1 = -(m1.m_cx + m1.m_cy) - 0.5 * m1.m_width - 0.5 * m1.m_height;
    double cost2 = -(m2.m_cx + m2.m_cy) - 0.5 * m2.m_width - 0.5 * m2.m_height;
    //double cost1 = (m1.m_x+m1.m_width) + (m1.m_y+m1.m_height);
    //double cost2 = (m2.m_x+m2.m_width) + (m2.m_y+m2.m_height);

    return cost1 < cost2;
}

bool Jin::LessXCoorMacroPrior::operator()( const int& mid1, const int& mid2 )
{
    const Module& m1 = m_placedb->m_modules[mid1];
    const Module& m2 = m_placedb->m_modules[mid2];

    double cost1 = m1.m_cx + m1.m_cy - m1.m_width - m1.m_height;
    double cost2 = m2.m_cx + m2.m_cy - m2.m_width - m2.m_height;

    return cost1 < cost2;

}

bool CTetrisLegal::MacroShifter( const double& macroFactor, const bool& makeFixed )
{
    //test code
    //if( gArg.IsDev() )
    //{
    //	m_placedb.m_modules_bak = m_placedb.m_modules;
    //	m_placedb.OutputGnuplotFigureWithZoom( "macro_shifter_before", false, true, true );
    //}
    //@test code

    //Expand module width to site step
    SaveModuleWidth();
    ExpandModuleWidthToSiteStep();
    
    const double thresholdHeight = macroFactor * m_placedb.m_rowHeight;
    //const double thresholdWidth = m_placedb.m_sites[0].m_step;
    const double thresholdArea = macroFactor * m_placedb.m_rowHeight;
    bool bUseMacroArea = gArg.CheckExist("MacroAreaRatio");
    
    for( unsigned int iMod = 0 ; iMod < m_placedb.m_modules.size() ; iMod++ )
    {
	const Module& curModule = m_placedb.m_modules[iMod];
	if( !curModule.m_isFixed 
		&& ( (!bUseMacroArea && curModule.m_height >= thresholdHeight)
		|| (bUseMacroArea && curModule.m_height > m_placedb.m_rowHeight && curModule.m_area >= thresholdArea) )
	){
	    		m_macro_ids.push_back( iMod );
	}	
    }
   
    if(m_macro_ids.size() == 0) return true; // kaie
 
    double min_shifting = numeric_limits<double>::max();

    bool (*function_array[4])( const int&, const int& );
    function_array[0] = LessXCoorMacroPrior::BL;
    function_array[1] = LessXCoorMacroPrior::BR;
    function_array[2] = LessXCoorMacroPrior::TL;
    function_array[3] = LessXCoorMacroPrior::TR;

    LessXCoorMacroPrior::m_placedb = &m_placedb;

    bool bSuccess = false;
    int bestType = -1;
    MacroShifterSaveOrigPosition();

    for( int iFunction = 0 ; iFunction < 4 ; iFunction++ )
    {
	vector<int> macro_order = m_macro_ids;
	//sort( macro_order.begin(), macro_order.end(), LessXCoorMacroPrior() );
	sort( macro_order.begin(), macro_order.end(), function_array[iFunction] );

	bool result = true;
	MacroShifterRestoreOrigPosition();
	RestoreFreeSite();
	double total_shifting = 0;

	for( unsigned int iOrder = 0 ; iOrder < macro_order.size() && result ; iOrder++ )
	{
	    const int& cellid = macro_order[iOrder];
	    vector<CLegalLocation> legalLocations;

	    if( true == AggressiveMacroDiamondSearch( cellid, legalLocations ) )
	    {
		int bestIndex = ReturnBestLocationIndex( cellid, legalLocations );
		const CLegalLocation& bestLocation = legalLocations[bestIndex];

		const Module& curModule = m_placedb.m_modules[cellid];
		CPoint p1( curModule.m_x, curModule.m_y );
		//CPoint p1 = m_macro_shifter_orig_positions[iOrder];
		CPoint p2( bestLocation.m_xcoor, m_free_sites[bestLocation.m_site_index].m_bottom );

		total_shifting += CPoint::Distance( p1, p2 );

		const double bestx = bestLocation.m_xcoor;
		const double besty = m_free_sites[bestLocation.m_site_index].m_bottom;
		m_placedb.SetModuleLocation( cellid, bestx, besty );

		//test code
		//fprintf(stderr, "Find best (%.2f,%.2f) y: %.2f\n", bestx, bestx+curModule.m_width, besty );
		//@test code
		
		UpdateFreeSite( cellid );		

		//if( makeFixed )
		//{
		//	m_placedb.m_modules[cellid].m_isFixed = true;
		//}
	    }
	    else
	    {
		result = false;
	    }

	}

	//test code
	//if( result )
	//printf("%d Success\n", iFunction);
	//else
	//printf("%d Fail\n", iFunction);
	//@test code

	if( result && total_shifting < min_shifting )
	{
	    bestType = iFunction;
	    min_shifting = total_shifting;
	    MacroShifterSaveBestPosition();
	    bSuccess = true;	
	}

    }	

    if( bSuccess )
    {
	MacroShifterRestoreBestPosition();

	if( 0 == bestType )
	{
	    printf("Best direction is BL\n");
	}
	else if( 1 == bestType )
	{
	    printf("Best direction is BR\n");
	}
	else if( 2 == bestType )
	{
	    printf("Best direction is TL\n");
	}
	else if( 3 == bestType )
	{
	    printf("Best direction is TR\n");
	}
	else
	{
	    fprintf(stderr, "Warning: unknown bestType %d\n", bestType );
	}

	//Fix macros if 'makeFixed' is true
	if( makeFixed )
	{
	    for( unsigned int i = 0 ; i < m_macro_ids.size() ; i++ )
	    {
		const int moduleIndex = m_macro_ids[i];
		m_placedb.m_modules[moduleIndex].m_isFixed;
	    }
	}
    }
    else
    {
	MacroShifterRestoreOrigPosition();
    }

    //Restore module width
    RestoreModuleWidth();
    
    //test code
    //if( gArg.IsDev() )
    //{
    //	m_placedb.OutputGnuplotFigureWithZoom( "macro_shifter", true, true, true );
    //}
    //@test code

    return bSuccess;
}

//kaie Robust MacroShifter
bool CTetrisLegal::RobustMacroShifter( const double& macroFactor, const bool& makeFixed )
{
    //Expand module width to site step
    SaveModuleWidth();
    ExpandModuleWidthToSiteStep();
    
    vector<int> m_macroId;
    const double thresholdHeight = macroFactor * m_placedb.m_rowHeight;
    //const double thresholdWidth = m_placedb.m_sites[0].m_step;
    const double thresholdArea = macroFactor * m_placedb.m_rowHeight;
    bool bUseMacroArea = gArg.CheckExist("MacroAreaRatio");
    
    for( unsigned int iMod = 0 ; iMod < m_placedb.m_modules.size() ; iMod++ )
    {
	const Module& curModule = m_placedb.m_modules[iMod];
	if( !curModule.m_isFixed 
		&& ( (!bUseMacroArea && curModule.m_height >= thresholdHeight)
		|| (bUseMacroArea && curModule.m_height > m_placedb.m_rowHeight && curModule.m_area >= thresholdArea) )
	){
	    		m_macroId.push_back( iMod );
	}	
    }
    
    bool bSuccess = false;

    int n_macros = (int)m_macroId.size();

    double xMin = m_placedb.m_coreRgn.left;
    double yMin = m_placedb.m_coreRgn.bottom;

    double xShift = -floor( xMin );
    double yShift = -floor( yMin );

    //double left = m_placedb.m_coreRgn.left;
    //double right = m_placedb.m_coreRgn.right;
    //double bottom = m_placedb.m_coreRgn.bottom;
    //double top = m_placedb.m_coreRgn.top;
    double Width = m_placedb.m_coreRgn.right - m_placedb.m_coreRgn.left;
    double Height = m_placedb.m_coreRgn.top - m_placedb.m_coreRgn.bottom;

    printf("number of macros: %d\n", n_macros);

    lprec *lp;
    int Ncol, *colno = NULL, ret = 0;
    REAL *row = NULL;

    FILE* CplexModel;
    CplexModel = fopen("cplexmodel.lp", "w");

    Ncol = /*1+*/n_macros*(n_macros+1)+2; // the first element is reserved?
    //printf("Ncol: %d\n", Ncol);
    lp = make_lp(0, Ncol);
    if(lp == NULL)
	ret = 1; /* couldn't construct a new model... */

    fprintf(CplexModel, "minimize +C%d +C%d\n", Ncol-1, Ncol-2);

    //variables
    if(ret == 0)
    {
	/* create space large enough for one row */
	colno = (int*)malloc(Ncol * sizeof(*colno));
	row = (REAL*)malloc(Ncol * sizeof(*row));
	if((colno == NULL) || (row == NULL))
	    ret = 2;

	int count = 0;
	for(int i = 0; i < n_macros; i++)
	{
	    //set_int(lp, 2*i+2, true); // x_i
	    //set_int(lp, 2*i+1+2, true); // y_i
	    for(int j = i+1; j < n_macros; j++)
	    {
		set_binary(lp, 1 + 2*n_macros + 2*count, true); // p_ij
		set_binary(lp, 1 + 2*n_macros + 2*count + 1, true); // q_ij

		//set_binary(lp, /*1+*/2*n_macros+2*(i*n_macros-(i*(i+1)/2)+j-i-1), true); // p_ij
		//set_binary(lp, /*1+*/2*n_macros+2*(i*n_macros-(i*(i+1)/2)+j-i-1)+1, true); // q_ij
		//printf("%d, %d, %d\n", i, j, i*n_macros-(i*(i+1)/2)+j-i);
		
		count ++;
	    }
	    //printf("%d (%lf, %lf)\n", m_macroId[i], m_placedb.m_modules[m_macroId[i]].m_width, m_placedb.m_modules[m_macroId[i]].m_height);
	}
    }
    //printf("(%lf, %lf)(%lf, %lf), (%lf)(%lf)\n", left, bottom, right, top, width, height);

    //constraints
    if(ret == 0)
    {
	set_add_rowmode(lp, true); /* makes building the model faster if it is done rows by row */

	fprintf(CplexModel, "subject to\n");
	// non-overlap constraints
	int count = 0;
	for(int i = 0; i < n_macros; i++)
	{
	    //double m_x_i = m_placedb.m_modules[m_macroId[i]].m_x;
	    //double m_y_i = m_placedb.m_modules[m_macroId[i]].m_y;
	    for(int j = i+1; j < n_macros; j++) // for any two macros
	    {
		int k;
		double bound;

		int x_i = 1 + 2*i;
		int y_i = 1 + 2*i + 1;
		int x_j = 1 + 2*j;
		int y_j = 1 + 2*j + 1;
		//int p_ij = 1+2*n_macros+2*(i*n_macros-(i*(i+1)/2)+j-i-1);
		//int q_ij = 1+2*n_macros+2*(i*n_macros-(i*(i+1)/2)+j-i-1)+1;
		int p_ij = 1 + 2*n_macros + 2*count;
		int q_ij = 1 + 2*n_macros + 2*count + 1;
		//double m_x_j = m_placedb.m_modules[m_macroId[j]].m_x;
		//double m_y_j = m_placedb.m_modules[m_macroId[j]].m_y;
		
		// M_i to the left of M_j
		// x_i + w_i <= x_j + W(p_ij + q_ij)
		// x_i - x_j - Wp_ij - Wq_ij <= -w_i
		//if(m_x_i <= m_x_j)
		//{
		k = 0;
		colno[k] = x_i;
		row[k++] = 1;
		colno[k] = x_j;
		row[k++] = -1;
		colno[k] = p_ij;
		row[k++] = -1 * Width;
		colno[k] = q_ij;
		row[k++] = -1 * Width;
		bound = -m_placedb.m_modules[m_macroId[i]].m_width;
		if(!add_constraintex(lp, k, row, colno, LE, bound))
		    ret = 3;
		fprintf(CplexModel, "%+d C%d %+d C%d %+f C%d %+f C%d <= %+f\n",
		    1, x_i, -1, x_j, -1*Width, p_ij, -1*Width, q_ij, bound);
		//}
		// M_i to the right of M_j
		// x_i - w_j >= x_j - W(1 - p_ij + q_ij)
		// x_i - x_j - Wp_ij + Wq_ij >= w_j - W
		//if(m_x_i >= m_x_j)
		//{
		k = 0;
		colno[k] = x_i;
		row[k++] = 1;
		colno[k] = x_j;
		row[k++] = -1;
		colno[k] = p_ij;
		row[k++] = -1 * Width;
		colno[k] = q_ij;
		row[k++] = 1 * Width;
		bound = m_placedb.m_modules[m_macroId[j]].m_width - Width;
		if(!add_constraintex(lp, k, row, colno, GE, bound))
		    ret = 3;
		fprintf(CplexModel, "%+d C%d %+d C%d %+f C%d %+f C%d >= %+f\n",
		    1, x_i, -1, x_j, -1*Width, p_ij, 1*Width, q_ij, bound);
		//}
		// M_i below M_j
		// y_i + h_i <= y_j + H(1 + p_ij - q_ij)
		// y_i - y_j - Hp_ij + Hq_ij <= H - h_i*/
		//if(m_y_i <= m_y_j)
		//{
		k = 0;
		colno[k] = y_i;
		row[k++] = 1;
		colno[k] = y_j;
		row[k++] = -1;
		colno[k] = p_ij;
		row[k++] = -1 * Height;
		colno[k] = q_ij;
		row[k++] = 1 * Height;
		bound = Height - m_placedb.m_modules[m_macroId[i]].m_height;
		if(!add_constraintex(lp, k, row, colno, LE, bound))
		    ret = 3;
		fprintf(CplexModel, "%+d C%d %+d C%d %+f C%d %+f C%d <= %+f\n",
		    1, y_i, -1, y_j, -1*Height, p_ij, 1*Height, q_ij, bound);
		//}
		// M_i above M_j
		// y_i - h_j >= y_j - H(2 - p_ij - q_ij)	
		// y_i - y_j - Hp_ij - Hq_ij >= h_j - 2H
		//if(m_y_i >= m_y_j)
		//{
		k = 0;
		colno[k] = y_i;
		row[k++] = 1;
		colno[k] = y_j;
		row[k++] = -1;
		colno[k] = p_ij;
		row[k++] = -1 * Height;
		colno[k] = q_ij;
		row[k++] = -1 * Height;
		bound = m_placedb.m_modules[m_macroId[j]].m_height - 2*Height;
		if(!add_constraintex(lp, k, row, colno, GE, bound))
		    ret = 3;
		fprintf(CplexModel, "%+d C%d %+d C%d %+f C%d %+f C%d >= %+f\n",
		    1, y_i, -1, y_j, -1*Height, p_ij, -1*Height, q_ij, bound);
		//}

		count++;
	    }
	}

	//boundary constraints
	for(int i = 0; i < n_macros; i++)
	{
	    double right_bound, left_bound, bottom_bound, top_bound;
	    if(!m_placedb.m_modules[m_macroId[i]].m_isFixed)
	    {
		left_bound = 0;
		bottom_bound = 0;
		right_bound = Width - m_placedb.m_modules[m_macroId[i]].m_width;
		top_bound = Height - m_placedb.m_modules[m_macroId[i]].m_height;
	    }else
	    {
		left_bound = right_bound = m_placedb.m_modules[m_macroId[i]].m_x+xShift;
		bottom_bound = top_bound = m_placedb.m_modules[m_macroId[i]].m_y+yShift;
	    }
	
	    colno[0] = 1+2*i; // x_i
	    row[0] = 1;
	    if(!add_constraintex(lp, 1, row, colno, LE, right_bound)) ret = 3;
	    fprintf(CplexModel, "%+d C%d <= %+f\n", 1, 1+2*i, right_bound);
	    if(!add_constraintex(lp, 1, row, colno, GE, left_bound)) ret = 3;
	    fprintf(CplexModel, "%+d C%d >= %+f\n", 1, 1+2*i, left_bound);

	    colno[0] = 1+2*i+1; // y_i
	    if(!add_constraintex(lp, 1, row, colno, LE, top_bound)) ret = 3;
	    fprintf(CplexModel, "%+d C%d <= %+f\n", 1, 1+2*i+1, top_bound);
	    if(!add_constraintex(lp, 1, row, colno, GE, bottom_bound)) ret = 3;
	    fprintf(CplexModel, "%+d C%d >= %+f\n", 1, 1+2*i+1, bottom_bound);
	}

	//convert absolute objective function to constraints
	int u = 1+(n_macros)*(n_macros+1); // u
	int v = 1+(n_macros)*(n_macros+1)+1; // v
	for(int i = 0; i < n_macros; i++)
	{
	    int k;

	    double m_x = m_placedb.m_modules[m_macroId[i]].m_x;
	    double m_y = m_placedb.m_modules[m_macroId[i]].m_y;

	    int x_i = 1+2*i;
	    int y_i = 1+2*i+1;

	    // x_i - x_j <= u
	    // x_i - u <= x_j
	    k = 0;
	    colno[k] = x_i; // x_i
	    row[k++] = 1;
	    colno[k] = u;
	    row[k++] = -1;
	    if(!add_constraintex(lp, k, row, colno, LE, m_x)) ret = 3;
	    fprintf(CplexModel, "%+d C%d %+d C%d <= %+f\n", 1, x_i, -1, u, m_x);

	    // -x_i + x_j <= u
	    // -x_i - u <= -x_j
	    k = 0;
	    row[k++] = -1;
	    row[k++] = -1;
	    if(!add_constraintex(lp, k, row, colno, LE, -m_x)) ret = 3;
	    fprintf(CplexModel, "%+d C%d %+d C%d <= %+f\n", -1, x_i, -1, u, -m_x);

	    // y_i - y_j <= v
	    // y_i - v <= y_j
	    k = 0;
	    colno[k] = y_i;
	    row[k++] = 1;
	    colno[k] = v;
	    row[k++] = -1;
	    if(!add_constraintex(lp, k, row, colno, LE, m_y)) ret = 3;
	    fprintf(CplexModel, "%+d C%d %+d C%d <= %+f\n", 1, y_i, -1, v, m_y);

	    // -y_i + y_j <= v
	    // -y_i - v <= -y_j
	    k = 0;
	    row[k++] = -1;
	    row[k++] = -1;
	    if(!add_constraintex(lp, k, row, colno, LE, -m_y)) ret = 3;
	    fprintf(CplexModel, "%+d C%d %+d C%d <= %+f\n", -1, y_i, -1, v, -m_y);
	}

	set_add_rowmode(lp, false);
    }

    //objective function
    if(ret == 0)
    {
	colno[0] = 1 + n_macros*(n_macros+1); // u
	colno[1] = 1 + n_macros*(n_macros+1) + 1; // v
	
	row[0] = row[1] = 1;

	if(!set_obj_fnex(lp, 2, row, colno))
	    ret = 4;
    }

    fprintf(CplexModel, "integer\n");
    for(int i = 0, count = 0; i < n_macros; i++)
    {
	for(int j = i+1; j < n_macros; j++)
	{
	    fprintf(CplexModel, "C%d\n", 1 + 2*n_macros + 2*count);
	    fprintf(CplexModel, "C%d\n", 1 + 2*n_macros + 2*count + 1);
	    count++;
	}
    }
    fprintf(CplexModel, "end\n");
    fclose(CplexModel);

    if(ret == 0)
    {
	set_minim(lp);

	set_timeout(lp, 60);

	int ret = write_lp(lp, "model.lp");

	//set_verbose(lp, IMPORTANT);

	set_presolve(lp, PRESOLVE_ROWS | PRESOLVE_COLS | PRESOLVE_LINDEP, get_presolveloops(lp));
	while(true)
	{
	    ret = solve(lp);
	    if(ret == OPTIMAL || ret == SUBOPTIMAL) break;
	}
	//printf("ret = %d\n", ret);

	if(ret == OPTIMAL || ret == SUBOPTIMAL) {bSuccess = true; ret = 0;}
	else ret = 5;
    }


    if( bSuccess )
    {
	//printf("get optimal\n");
	get_variables(lp, row);	

	/*int count = 0;
	for(int i = 0; i < n_macros; i++)
	{
	    double x_i = row[2*i];
	    double y_i = row[2*i+1];
	    double w_i = m_placedb.m_modules[m_macroId[i]].m_width;
	    double h_i = m_placedb.m_modules[m_macroId[i]].m_height;
	    for(int j = i+1; j < n_macros; j++)
	    {
		double x_j = row[2*j];
		double y_j = row[2*j+1];
		double w_j = m_placedb.m_modules[m_macroId[j]].m_width;
		double h_j = m_placedb.m_modules[m_macroId[j]].m_height;
		int p_ij = row[2*n_macros+2*count];
		int q_ij = row[2*n_macros+2*count+1];
	    	//printf("p_%d_%d=%d, q_%d_%d=%d\n", i, j, p_ij, i, j, q_ij);

		if(p_ij == 0 && q_ij == 0)
		{
		    if(!(x_i + w_i <= x_j)) printf("x_%d + w_%d > x_%d\n", i, i, j);
		}else if(p_ij == 1 && q_ij == 0)
		{
		    if(!(y_i + h_i <= y_j)) printf("y_%d + h_%d > y_%d\n", i, i, j);
		}else if(p_ij == 0 && q_ij == 1)
		{
		    if(!(x_i - w_j >= x_j)) printf("x_%d + w_%d < x_%d\n", i, j, j);
		}else if(p_ij == 1 && q_ij == 1)
		{
		    if(!(y_i - h_j >= y_j)) printf("y_%d - h_%d < y_%d\n", i, j, j);
		}

		count++;
	    }
	}*/
	for(int i = 0; i < n_macros; i++)
	{
	    //printf("%d(%lf, %lf) => (%lf, %lf)\n", m_macroId[i], m_placedb.m_modules[m_macroId[i]].m_width, m_placedb.m_modules[m_macroId[i]].m_height, row[2*i]-xShift, row[2*i+1]-yShift);
	    m_placedb.SetModuleLocation( m_macroId[i], row[2*i]-xShift , row[2*i+1]-yShift );
	}
	//print_solution(lp, 1);

	// plot macros
	FILE* plotter = fopen("lpmacroshifter.plt", "w");
	if(plotter == NULL){
	    printf("cannot open file\n");
	    exit(0);
	}

	fprintf(plotter, "\nset title \"%s\" font \"Times, 22\"\n\n", "Macro Shifter Result");
	fprintf(plotter, "set size ratio 1\n");
	fprintf(plotter, "set nokey\n\n");
	fprintf(plotter, "plot[:][:] '-' w l lt 1, '-' w l lt 3\n\n");

	fprintf(plotter, "# outline\n\n");
	fprintf(plotter, "0,0\n\n");
	fprintf(plotter, "%lf, %lf\n", m_placedb.m_coreRgn.left, m_placedb.m_coreRgn.bottom);
	fprintf(plotter, "%lf, %lf\n", m_placedb.m_coreRgn.right, m_placedb.m_coreRgn.bottom);
	fprintf(plotter, "%lf, %lf\n", m_placedb.m_coreRgn.right, m_placedb.m_coreRgn.top);
	fprintf(plotter, "%lf, %lf\n", m_placedb.m_coreRgn.left, m_placedb.m_coreRgn.top);
	fprintf(plotter, "%lf, %lf\n", m_placedb.m_coreRgn.left, m_placedb.m_coreRgn.bottom);
	fprintf(plotter, "\n");

	fprintf(plotter, "EOF\n\n");

	fprintf(plotter, "# blocks\n\n");
	fprintf(plotter, "0,0\n\n");
	for(int i = 0; i < n_macros; i++)
	{
	    double m_x = m_placedb.m_modules[m_macroId[i]].m_x;
	    double m_rx = m_x + m_placedb.m_modules[m_macroId[i]].m_width;
	    double m_y = m_placedb.m_modules[m_macroId[i]].m_y;
	    double m_ry = m_y + m_placedb.m_modules[m_macroId[i]].m_height;
	    fprintf(plotter, "%lf, %lf\n", m_x , m_y );
	    fprintf(plotter, "%lf, %lf\n", m_rx, m_y );
	    fprintf(plotter, "%lf, %lf\n", m_rx, m_ry);
	    fprintf(plotter, "%lf, %lf\n", m_x , m_ry);
	    fprintf(plotter, "%lf, %lf\n", m_x , m_y );
	    fprintf(plotter, "\n");
	}

	fprintf(plotter, "EOF\npause -1\n");
	pclose(plotter);


	//Fix macros if 'makeFixed' is true
	if( makeFixed )
	{
	    for( unsigned int i = 0 ; i < m_macro_ids.size() ; i++ )
	    {
		const int moduleIndex = m_macro_ids[i];
		m_placedb.m_modules[moduleIndex].m_isFixed;
	    }
	}
    }
    /*else
    {
	//MacroShifterRestoreOrigPosition();
	exit(0);
    }*/

    if(row != NULL)
	free(row);
    if(colno != NULL)
	free(colno);

    if(lp!= NULL)
	delete_lp(lp);

    //Restore module width
    RestoreModuleWidth();
    
    return bSuccess;
}

bool CTetrisLegal::AggressiveMacroDiamondSearch( const int& cellid,
	vector<CLegalLocation>& legalLocations )
{
    const Module& curModule = m_placedb.m_modules[cellid];

    if( curModule.m_height <= m_placedb.m_rowHeight )
    {
	fprintf( stderr, "Warning: Module %d is a standard cell and should not be processed by "
		"AggressiveMacroDiamondSearch()\n", cellid );
    }

    double max_horizontal_dist = max( 
	    fabs(curModule.m_x - m_placedb.m_coreRgn.left),
	    fabs(curModule.m_x - m_placedb.m_coreRgn.right) );
    double max_vertical_dist = max(
	    fabs(curModule.m_y - m_placedb.m_coreRgn.bottom ),
	    fabs(curModule.m_y - m_placedb.m_coreRgn.top ) );


    legalLocations.clear();

    double radius_step = max( m_max_module_height, m_max_module_width );	
    double radius = ( curModule.m_width + curModule.m_height ) * 2.0 / 3.0; 
    const CPoint module_center = CPoint( curModule.m_cx, curModule.m_cy );

    //Diamond search stop when all the chip region is covered
    double max_radius = max_horizontal_dist + max_vertical_dist + radius_step;

    //Search continues until at least one legal location is found or 
    //no legal locations can be found
    while( radius < max_radius && legalLocations.empty() )
    {
	vector<CSiteRow> sites;
	GetDiamondSiteRows( module_center,
		radius, sites );
	GetMacroLegalLocationsTowardOrig( cellid,
		sites, legalLocations );

	//Calculate real row index for legalLocations
	if( !sites.empty() )
	{
	    int index_yshift = static_cast<int>( (sites.front().m_bottom - m_free_sites.front().m_bottom)/m_placedb.m_rowHeight );
	    for( unsigned int i = 0 ; i < legalLocations.size() ; i++ )
	    {
		legalLocations[i].m_site_index = legalLocations[i].m_site_index + index_yshift;
	    }
	}
	
	radius += radius_step;
    }

    //No legal locations are found
    if( legalLocations.empty() )
    {
	//test code
	if( gArg.IsDev() )
	    printf("Final try radius %.2f of module %d height %.2f width %.2f (%.2f,%.2f)\n",
		    radius, cellid, curModule.m_height, curModule.m_width,
		    curModule.m_x, curModule.m_y );
	//@test code
	return false;
    }
    else
    {
	return true;
    }
}


void CTetrisLegal::UpdateFreeSite( const int& cellid )
{
    const Module& curModule = m_placedb.m_modules[cellid];
    int start_row_index = GetSiteIndex( curModule.m_y );

    if( curModule.m_height <= m_placedb.m_rowHeight )
    {
	UpdateFreeSite( start_row_index, 
		curModule.m_x, 
		curModule.m_width ); 
    }
    else
    {
	//Update free site for a macro
	int needed_row_count = static_cast<int>( ceil(curModule.m_height/m_site_height) );
	for( int iRow = start_row_index; 
		iRow < start_row_index + needed_row_count ;
		iRow++ )
	{
	    UpdateFreeSite( iRow, curModule.m_x, curModule.m_width );
	}
    }

}

int CTetrisLegal::ReturnBestLocationIndex( const int& cellid, 
	std::vector<Jin::CLegalLocation>& legalLocations )
{
    if( legalLocations.empty() )
    {
	cerr << "Warning: legalLocations are empty, and thus -1 is returned" << endl;
	return -1;
    }
    else if( 1 == legalLocations.size() )
    {
	return 0;
    }

    //Set up the cost for each legalLocations
    const Module& curModule = m_placedb.m_modules[cellid];
    CPoint p1(curModule.m_x,curModule.m_y);
    //CPoint p1 = m_origLocations[cellid];

    for( unsigned int i = 0 ; i < legalLocations.size() ; i++ )
    {
	CLegalLocation& curLoc = legalLocations[i];
	double locx = curLoc.m_xcoor;
	double locy = m_free_sites[ curLoc.m_site_index ].m_bottom;

	CPoint p2( locx, locy );

	curLoc.m_shift = CPoint::Distance(p1, p2);
    }

    double min_shift = legalLocations.front().m_shift;
    list<int> min_shift_indexes(1,0);

    for( unsigned int iLoc = 1 ; iLoc < legalLocations.size() ; iLoc++ )
    {
	CLegalLocation& curLoc = legalLocations[iLoc];
	if( curLoc.m_shift < min_shift )
	{
	    min_shift = curLoc.m_shift;
	    min_shift_indexes.resize(1);
	    min_shift_indexes.front() = iLoc;
	}
	else if( curLoc.m_shift == min_shift )
	{
	    min_shift_indexes.push_back( iLoc );
	}	
    }

    //Only one location has the minimum shift
    //Just return the location	
    if( 1 == min_shift_indexes.size() )
    {
	return min_shift_indexes.front();
    }
    //More than one locations have minimum shift
    //Return the location with minimum wirelength
    else if( 1 < min_shift_indexes.size() )
    {
	//Set up wirelength
	for( list<int>::iterator iteLocIndex = min_shift_indexes.begin() ;
		iteLocIndex != min_shift_indexes.end() ; iteLocIndex++ )
	{
	    CLegalLocation& curLoc = legalLocations[*iteLocIndex];

	    double locx = curLoc.m_xcoor;
	    double locy = m_free_sites[ curLoc.m_site_index ].m_bottom;

	    m_placedb.SetModuleLocation( cellid, locx, locy );
	    curLoc.m_wirelength = m_placedb.GetModuleTotalNetLength( cellid );
	}

	double min_wirelength = numeric_limits<double>::max();
	int min_wirelength_location_index = -1;

	for( list<int>::iterator iteLocIndex = min_shift_indexes.begin() ;
		iteLocIndex != min_shift_indexes.end() ; iteLocIndex++ )
	{
	    CLegalLocation& curLoc = legalLocations[*iteLocIndex];

	    if( curLoc.m_wirelength < min_wirelength )
	    {
		min_wirelength_location_index = *iteLocIndex;
		min_wirelength = curLoc.m_wirelength;
	    }
	}

	if( min_wirelength_location_index < 0 )
	{
	    cerr << "Warning: illegal min_wirelength_location_index " 
		<< min_wirelength_location_index << ", and thus reutrn 0" << endl;
	    return 0;
	}
	else
	{
	    return min_wirelength_location_index;
	}

    }
    else
    {
	cerr << "Warning: incorrect min_shift_indexes size " << min_shift_indexes.size() << 
	    ", and thus return 0" << endl;
	return 0;
    }
}


void CTetrisLegal::GetMacroLegalLocationsTowardOrig( 
	const int& cellid,
	const std::vector<CSiteRow>& sites,
	std::vector<Jin::CLegalLocation>& legalLocations )
{
//test code to dump sites contents
#if 0 
    //test code
    printf("new\n");
    FILE* ofile = fopen( "getmacrolegallocation.log", "w" );
    for( unsigned int i = 0 ; i < sites.size() ; i++ )
    {
	const CSiteRow& curRow = sites[i];

	fprintf(ofile,"bottom: %.2f ", curRow.m_bottom );
	for( unsigned int j = 0 ; j < curRow.m_interval.size() ; j=j+2 )
	{
	    fprintf(ofile, "(%.2f,%.2f) ", curRow.m_interval[j], curRow.m_interval[j+1] );
	}
	fprintf(ofile, "\n");
    }
    fclose( ofile );
    //@test code
#endif
    const double site_step = m_placedb.m_sites[0].m_step;
    
    const Module& curModule = m_placedb.m_modules[cellid];

    if( curModule.m_height <= m_placedb.m_rowHeight )
    {
	fprintf(stderr, "Module %d is a standard cell and should not been processed "
		"by GetMacroLegalLocationsTowardLeft()\n", cellid );

    }

    const double module_width = curModule.m_width;

    //Number of rows needed for this macro
    const int module_row_number = 
	static_cast<int>(ceil(curModule.m_height/m_placedb.m_rowHeight));

    //'terminal_count_array' keeps the terminal count used by scanline algorithm.
    //terminal_count_array[i] is associated with row [sites[i], site[i+module_row_number])	
    int terminal_count_array_size = sites.size() - module_row_number + 1;

    if( terminal_count_array_size <= 0 )
    {
	cerr << "Warning: input sites has no enough height to put target macro" << endl;
	return;
    }

    int* terminal_count_array = new int[terminal_count_array_size];
    for( int i = 0 ; i < terminal_count_array_size ; i++ )	
    {
	terminal_count_array[i] = 0;
    }

    //'rightmost_left_terminals' keeps the x coordinate of the rightmost left terminal of each row
    //rightmost_left_terminals[i] recordes the x coordinate of the rightmost left terminal with 
    //row [ sites[i], site[i+module_row_number] )
    double* rightmost_left_terminals = new double[terminal_count_array_size];

    //Push all terminals into an array and sort it
    vector<CTerminal> terminals;
    int precalculate_terminal_number = 0;
    for( unsigned int i = 0 ; i < sites.size() ; i++ )
    {
	precalculate_terminal_number += sites[i].m_interval.size();
    }

    terminals.reserve( precalculate_terminal_number );
    for( unsigned int iRow = 0 ; iRow < sites.size() ; iRow++ )
    {
	const CSiteRow& curRow = sites[iRow];
	for( unsigned int i = 0 ; i < curRow.m_interval.size() ; i=i+2 )
	{
	    terminals.push_back( 
		    CTerminal( curRow.m_interval[i], CTerminal::Left, iRow ) );
	    terminals.push_back(
		    CTerminal( curRow.m_interval[i+1], CTerminal::Right, iRow ) );
	}		
    }	

    //Sort terminals by their x coordinates
    sort( terminals.begin(), terminals.end(), LessXCoor() );

    //Scanline algorithm
    for( unsigned int iTerminal = 0 ; iTerminal < terminals.size() ; iTerminal++ )
    {
	const CTerminal& curTerminal = terminals[iTerminal];
	const int associated_upper_row = 
	    min( terminal_count_array_size-1, curTerminal.m_row  );
	const int associated_lower_row = 
	    max( 0, curTerminal.m_row - (module_row_number-1));

	//Increase the terminal count for associated arrays
	if( CTerminal::Left == curTerminal.m_type )
	{
	    for( int iRow = associated_lower_row ; iRow <= associated_upper_row ; iRow++ )
	    {
		terminal_count_array[iRow]++;
		rightmost_left_terminals[iRow] = curTerminal.m_xcoor;
	    }
	}
	//Check if there is free spaces for this macro
	else
	{
	    for( int iRow = associated_lower_row ; iRow <= associated_upper_row ; iRow++ )
	    {
		if( module_row_number == terminal_count_array[iRow] )
		{
		    //TODO: compute the integer x_best position
		    double xleft = rightmost_left_terminals[iRow];
		    xleft = ceil( xleft / site_step ) * site_step; 
		    double xright = curTerminal.m_xcoor;
		    xright = floor( xright / site_step ) * site_step;

		    double width = xright - xleft;

		    //This interval is enough for this macro
		    if( width >= module_width )
		    {
			//Compute the feasible x interval
			double feasible_xleft = xleft;
			double feasible_xright = xright - module_width;

			double xbest;

			//Feasible interval is at left of curModule
			//The best location is the right boundary of feasible interval
			if( feasible_xright <= curModule.m_x )
			{
			    xbest = feasible_xright;
			}
			//Feasible interval is at right of curModule
			//The best location is the left boundary of feasible interval
			else if( feasible_xleft >= curModule.m_x )
			{
			    xbest = feasible_xleft;
			}
			//Feasible interval contains the original location of curModule
			//The best location is the orignal x coordinate of curModule
			else
			{
			    xbest = curModule.m_x;
			}

			xbest = floor( ( xbest + ( site_step / 2.0 ) ) / site_step ) * site_step;
			legalLocations.push_back( CLegalLocation( iRow, xbest ) );

			//test code
			//fprintf(stderr, "Find location (%.2f,%.2f) y: %.2f\n", xbest, xbest+curModule.m_width, 
			//	sites[iRow].m_bottom);
			//@test code
		    }	

		}

		terminal_count_array[iRow]--;
	    }
	}

    }


    delete []terminal_count_array;
    delete []rightmost_left_terminals;
    terminal_count_array = NULL;
    rightmost_left_terminals = NULL;

}

void CTetrisLegal::GetMacroLegalLocationsTowardLeft( 
	const int& cellid,
	const std::vector<CSiteRow>& sites,
	std::vector<Jin::CLegalLocation>& legalLocations,
	const double& left_bound )
{
    const double site_step = m_placedb.m_sites[0].m_step;
    const Module& curModule = m_placedb.m_modules[cellid];

    if( curModule.m_height <= m_placedb.m_rowHeight )
    {
	fprintf(stderr, "Module %d is a standard cell and should not been processed "
		"by GetMacroLegalLocationsTowardLeft()\n", cellid );

    }

    const double module_width = curModule.m_width;

    //Number of rows needed for this macro
    const int module_row_number = 
	static_cast<int>(ceil(curModule.m_height/m_placedb.m_rowHeight));

    //'terminal_count_array' keeps the terminal count used by scanline algorithm.
    //terminal_count_array[i] is associated with row [sites[i], site[i+module_row_number])	
    int terminal_count_array_size = sites.size() - module_row_number + 1;

    if( terminal_count_array_size <= 0 )
    {
	cerr << "Warning: input sites has no enough height to put target macro" << endl;
	return;
    }

    int* terminal_count_array = new int[terminal_count_array_size];
    for( int i = 0 ; i < terminal_count_array_size ; i++ )	
    {
	terminal_count_array[i] = 0;
    }

    //'rightmost_left_terminals' keeps the x coordinate of the rightmost left terminal of each row
    //rightmost_left_terminals[i] recordes the x coordinate of the rightmost left terminal with 
    //row [ sites[i], site[i+module_row_number] )
    double* rightmost_left_terminals = new double[terminal_count_array_size];

    //Push all terminals into an array and sort it
    vector<CTerminal> terminals;
    int precalculate_terminal_number = 0;
    for( unsigned int i = 0 ; i < sites.size() ; i++ )
    {
	precalculate_terminal_number += sites[i].m_interval.size();
    }

    terminals.reserve( precalculate_terminal_number );
    for( unsigned int iRow = 0 ; iRow < sites.size() ; iRow++ )
    {
	const CSiteRow& curRow = sites[iRow];
	for( unsigned int i = 0 ; i < curRow.m_interval.size() ; i=i+2 )
	{
	    terminals.push_back( 
		    CTerminal( curRow.m_interval[i], CTerminal::Left, iRow ) );
	    terminals.push_back(
		    CTerminal( curRow.m_interval[i+1], CTerminal::Right, iRow ) );
	}		
    }	

    //Sort terminals by their x coordinates
    sort( terminals.begin(), terminals.end(), LessXCoor() );

    //Add a stop condition when at least one point is found for each row
    //Also, one row can be skip if one point is found
    bool* bFoundLocation = new bool[terminal_count_array_size];
    for( int i = 0 ; i < terminal_count_array_size ; i++ )
    {
	bFoundLocation[i] = false;
    }
    int numFoundLocation = 0;

    //Scanline algorithm
    for( unsigned int iTerminal = 0 ; 
	    iTerminal < terminals.size() && numFoundLocation < terminal_count_array_size ; 
	    iTerminal++ )
    {
	const CTerminal& curTerminal = terminals[iTerminal];
	const int associated_upper_row = 
	    min( terminal_count_array_size-1, curTerminal.m_row  );
	const int associated_lower_row = 
	    max( 0, curTerminal.m_row - (module_row_number-1));

	//Increase the terminal count for associated arrays
	if( CTerminal::Left == curTerminal.m_type )
	{
	    for( int iRow = associated_lower_row ; iRow <= associated_upper_row ; iRow++ )
	    {
		terminal_count_array[iRow]++;
		rightmost_left_terminals[iRow] = curTerminal.m_xcoor;
	    }
	}
	//Check if there is free spaces for this macro
	else
	{
	    for( int iRow = associated_lower_row ; iRow <= associated_upper_row ; iRow++ )
	    {
		//If one location has been found for this row,
		//skip the check of this row
		if( true == bFoundLocation[iRow] )
		    continue;

		if( module_row_number == terminal_count_array[iRow] )
		{
		    double xleft = max( left_bound, rightmost_left_terminals[iRow] );
		    xleft = ceil( xleft / site_step ) * site_step;
		    double xright = curTerminal.m_xcoor;
		    xright = floor( xright / site_step ) * site_step;

		    //May be negative
		    double width = xright - xleft;

		    if( width >= module_width )
		    {
			legalLocations.push_back( 
				CLegalLocation( iRow, xleft ) );

			//To skip unnecessary checkings
			numFoundLocation++;
			bFoundLocation[iRow] = true;
		    }	

		}

		terminal_count_array[iRow]--;
	    }
	}

    }


    delete []terminal_count_array;
    delete []rightmost_left_terminals;
    delete []bFoundLocation;
    terminal_count_array = NULL;
    rightmost_left_terminals = NULL;
    bFoundLocation = NULL;

}

bool Jin::LessXCoor::operator()( const int& mid1, const int& mid2 )
{
    const Module& m1 = m_placedb->m_modules[mid1];
    const Module& m2 = m_placedb->m_modules[mid2];

    double cost1 = m1.m_x;
    double cost2 = m2.m_x;

    //Macro have higher priority than cells
    if( m1.m_height > m_placedb->m_rowHeight )
    {
	cost1 = cost1 - m_macro_factor * m1.m_height - m_macro_factor * m1.m_width;
    }

    if( m2.m_height > m_placedb->m_rowHeight )
    {
	cost2 = cost2 - m_macro_factor * m2.m_height - m_macro_factor * m2.m_width;
    }

    //if( cost1 == cost2 )
    //{
    //	if( m1.m_width == m2.m_width )
    //	    return m1.m_height < m2.m_height;
    //	else
    //	    return m1.m_width < m2.m_width;
    //}
    //else
    //	return cost1 < cost2;

    return cost1 < cost2;
}


bool Jin::LessXCoorMacroFirst::operator()( const int& mid1, const int& mid2 )
{
    const Module& m1 = m_placedb->m_modules[mid1];
    const Module& m2 = m_placedb->m_modules[mid2];

    const bool bMacro1 = (m1.m_height > m_placedb->m_rowHeight) ? true : false;
    const bool bMacro2 = (m2.m_height > m_placedb->m_rowHeight) ? true : false;

    //Both modules are macros or cells,
    //sort them according to their x coordinates
    if( bMacro1 && bMacro2 ) // kaie
    {
	if (m1.m_area == m2.m_area) // kaie
	{
	if( m1.m_x == m2.m_x )
	{
	    if( m1.m_width == m2.m_width )
		return m1.m_height > m2.m_height;
	    else
		return m1.m_width > m2.m_width;
	}
	else
	    return m1.m_x < m2.m_x;
	}else
	    return m1.m_area > m2.m_area;
    }
    //One of these two modules are macros,
    //the macro module has higher priority
    else if( bMacro1 || bMacro2 )
    {
	if( bMacro2 )
	    return false;
	else
	    return true;
    }
    //No module is macro
    else
    {
	if( m1.m_x == m2.m_x )
	{
	    if( m1.m_width == m2.m_width )
		return m1.m_height > m2.m_height;
	    else
		return m1.m_width > m2.m_width;
	}
	else
	{
	    return m1.m_x < m2.m_x;
	}

    }
}

void CTetrisLegal::GetDiamondSiteRows( const CPoint& center,               //center of diamond
	const double& radius,               //radius of diamond
	std::vector<CSiteRow>& sites )     //put the resulting diamond sites into "sites"
{
    //Clear the returning result
    sites.clear();

    double diamond_bottom = center.y - radius;
    double diamond_top = center.y + radius;
    //double diamond_left = center.x - radius;
    //double diamond_right = center.x + radius;

    double free_site_bottom = m_free_sites.front().m_bottom;
    double free_site_height = m_free_sites.back().m_height;

    double free_site_top = m_free_sites.back().m_bottom + free_site_height;

    if( diamond_bottom > free_site_top || diamond_top < free_site_bottom )
    {
	cerr << "Warning: desired diamond region has no overlapping with free sites" << endl;
	return;
    }

    //Find the range of the diamond region in the free sites
    int bottom_site_index = max( 0, 
	    static_cast<int>( ceil( (diamond_bottom - free_site_bottom)/free_site_height)) );
    int top_site_index = min( static_cast<int>(m_free_sites.size()) - 1, 
	    static_cast<int>(floor( (diamond_top-free_site_bottom)/free_site_height ) ) - 1 );

    sites.reserve( top_site_index - bottom_site_index + 1 );

    //For each free site row in the range, add corresponding interval
    //into resulting sites
    for( int iRow = bottom_site_index ; iRow <= top_site_index ; iRow++ )
    {
	const CSiteRow& curRow = m_free_sites[iRow];

	double row_left, row_right;

	//Find the diamond left and right boundary in this row
	//curRow is at lower half of diamond
	
    if( curRow.m_bottom + curRow.m_height < center.y )
	{
	    row_left = floor( center.x - ( curRow.m_bottom + curRow.m_height - diamond_bottom ) );
	    row_right = ceil( center.x + ( curRow.m_bottom + curRow.m_height - diamond_bottom ) );	
	}
	//curRow is at higher half of diamond
	else
	{
        row_left = floor( center.x - ( diamond_top - curRow.m_bottom ) );
        row_right = ceil( center.x + ( diamond_top - curRow.m_bottom ) );
	}

	//Push curRow into resulting sites
	sites.push_back( CSiteRow(curRow.m_bottom, curRow.m_height, curRow.m_step) );
	//sites.back().m_bottom = curRow.m_bottom;
	//sites.back().m_height = curRow.m_height;
	//sites.back().m_step = curRow.m_step;

	sites.back().m_interval.reserve( curRow.m_interval.size() );

	//Push intervals which have overlapping with row_left and row_right
	for( unsigned int i = 0 ; i < curRow.m_interval.size() ; i = i+2 )
	{
	    double left = curRow.m_interval[i];
	    double right = curRow.m_interval[i+1];

	    if( left >= row_right || right <= row_left )
		continue;

	    sites.back().m_interval.push_back( max( left, row_left ) );
	    sites.back().m_interval.push_back( min( right, row_right ) );
	}
    }	
}

//Infeasible due the change of interface
#if 0
bool CTetrisLegal::DoVerticalLine(const double& prelegal_factor,
	const double& vertical_line_xcoor)
{
    bool bLegalLeft = false;
    bool bLegalRight = false;

    bLegalLeft = DoLeftWithVerticalLine( prelegal_factor, vertical_line_xcoor );

    if( bLegalLeft )
	bLegalRight = DoRightWithVerticalLine( prelegal_factor, vertical_line_xcoor );

    return bLegalLeft & bLegalRight;
}


bool CTetrisLegal::DoRightWithVerticalLine(const double& prelegal_factor,
	const double& vertical_line_xcoor)
{
    //ReversePlacement();
    //SetReverseSite();

    bool bLegal = DoLeftWithVerticalLine( prelegal_factor, -vertical_line_xcoor );

    //ReversePlacement();

    return bLegal;
}

bool CTetrisLegal::DoLeftWithVerticalLine(const double& prelegal_factor,
	const double& vertical_line_xcoor)
{
    CalculateNewLocation(prelegal_factor, vertical_line_xcoor);

    CalculateCellOrder(vertical_line_xcoor);
    RemoveFreeSite(vertical_line_xcoor);
    bool bLegal = LegalizeByCellOrder();

    return bLegal;
}

void CTetrisLegal::RemoveFreeSite( const double& vertical_line_xcoor )
{
    for( vector<CSiteRow>::iterator iteRow = m_free_sites.begin() ;
	    iteRow != m_free_sites.end() ; iteRow++ )
    {
	while(true)
	{
	    if( iteRow->m_interval.empty() )
		break;

	    double left = iteRow->m_interval[0];
	    double right = iteRow->m_interval[1];

	    if( left >= vertical_line_xcoor )
		break;
	    else if( right <= vertical_line_xcoor )
	    {
		iteRow->m_interval.erase( iteRow->m_interval.begin() );
		iteRow->m_interval.erase( iteRow->m_interval.begin() );
	    }
	    //left < vertical_line_xcoor and right > vertical_line_xcoor
	    else
	    {
		iteRow->m_interval[0] = vertical_line_xcoor;
		break;
	    }
	}	
    }	
}

void CTetrisLegal::CalculateCellOrder(const double& vertical_line_xcoor)
{
    m_cell_order.clear();
    m_cell_order.reserve( m_placedb.m_modules.size() );

    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	if( !m_placedb.m_modules[i].m_isFixed && 
		m_placedb.m_modules[i].m_cx >= vertical_line_xcoor)
	    m_cell_order.push_back(i);
    }

    LessXCoor::m_placedb = &m_placedb;

    sort( m_cell_order.begin(), m_cell_order.end(), LessXCoor() );	
}

void CTetrisLegal::CalculateNewLocation(const double& prelegal_factor,
	const double& vertical_line_xcoor)
{
    for( unsigned int i = 0 ; i < m_process_list.size() ; i++ )
    {
	Module& curModule = m_placedb.m_modules[ m_process_list[i] ];

	if( curModule.m_isFixed )
	{
	    fprintf(stderr, "module %d should not be processed by CalculateNewLocation()\n", m_process_list[i] );
	    continue;
	}

	//Only modules in the left side of the given vertical line is moved	
	if( curModule.m_cx >= vertical_line_xcoor )
	{
	    double newCX = (curModule.m_cx - vertical_line_xcoor)*prelegal_factor 
		+ vertical_line_xcoor;
	    double newCY = curModule.m_cy;

	    //Rounding for macros
	    if( curModule.m_height > m_placedb.m_rowHeight )
		newCX = Rounding(newCX);

	    m_placedb.MoveModuleCenter( m_process_list[i], newCX, newCY );
	}
    }
}
#endif

void CTetrisLegal::PrepareNonMacroLeftRightFreeSites(const vector<int>& macro_ids)
{
    m_free_sites = m_placedb.m_sites;
    m_right_free_sites.clear();

    for( unsigned int i = 0 ; i < macro_ids.size() ; i++ ) 
    {
	UpdateFreeSite( macro_ids[i] );
    }

    m_left_free_sites = m_free_sites;

    //Reverse m_free_sites
    for( unsigned int iRow = 0 ; iRow < m_left_free_sites.size() ; iRow++ )
    {
	CSiteRow& sourceRow = m_left_free_sites[iRow];
	//CSiteRow& curRow = m_free_sites[iRow];

	m_right_free_sites.push_back( CSiteRow( sourceRow.m_bottom, sourceRow.m_height, sourceRow.m_step ) );

	vector<double>& reverse_interval = m_right_free_sites.back().m_interval;
	reverse_interval.reserve( sourceRow.m_interval.size() );
	for( vector<double>::reverse_iterator iteInterval = sourceRow.m_interval.rbegin() ;
		iteInterval != sourceRow.m_interval.rend() ; iteInterval++ )
	{
	    //curRow.m_interval.push_back( -(*iteInterval) );
	    reverse_interval.push_back( -(*iteInterval) );	
	}	
    }


}

void CTetrisLegal::PrepareLeftRightFreeSites(void)
{
    //Modified by Jin 20070716
    //m_left_free_sites = m_placedb.m_sites;
    m_left_free_sites = m_free_sites;
    
    m_right_free_sites.clear();

    //Reverse m_free_sites
    for( unsigned int iRow = 0 ; iRow < m_left_free_sites.size() ; iRow++ )
    {
	CSiteRow& sourceRow = m_left_free_sites[iRow];
	//CSiteRow& curRow = m_free_sites[iRow];

	m_right_free_sites.push_back( CSiteRow( sourceRow.m_bottom, sourceRow.m_height, sourceRow.m_step ) );

	vector<double>& reverse_interval = m_right_free_sites.back().m_interval;
	reverse_interval.reserve( sourceRow.m_interval.size() );
	for( vector<double>::reverse_iterator iteInterval = sourceRow.m_interval.rbegin() ;
		iteInterval != sourceRow.m_interval.rend() ; iteInterval++ )
	{
	    //curRow.m_interval.push_back( -(*iteInterval) );
	    reverse_interval.push_back( -(*iteInterval) );	
	}	
    }


}


void CTetrisLegal::ReverseLegalizationData(void)
{
    //Reverse the chip core region
    double new_left = -(m_placedb.m_coreRgn.right);
    double new_right = -(m_placedb.m_coreRgn.left);
    m_placedb.m_coreRgn.left = new_left;
    m_placedb.m_coreRgn.right = new_right;

    //Reverse module pin offsets
    for( unsigned int iPin = 0 ; iPin < m_placedb.m_pins.size() ; iPin++ )
    {
	m_placedb.m_pins[iPin].xOff = -m_placedb.m_pins[iPin].xOff;
    }

}

void CTetrisLegal::ReversePlacement(void)
{
    //Reverse module centers
    for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
    {
	const Module& curModule = m_placedb.m_modules[iModule];
	double new_locx = (-curModule.m_cx) - (curModule.m_width/2.0);
	m_placedb.SetModuleLocation( iModule, new_locx, curModule.m_y );
    }

}

//void CTetrisLegal::SetReverseSite(void)
//{
//	for( unsigned int iRow = 0 ; iRow < m_placedb.m_sites.size() ; iRow++ )
//	{
//		const CSiteRow& sourceRow = m_placedb.m_sites[iRow];
//		CSiteRow& curRow = m_free_sites[iRow];
//
//		curRow.m_interval.clear();
//		curRow.m_interval.reserve( sourceRow.m_interval.size() );
//
//		for( vector<double>::const_reverse_iterator iteInterval = sourceRow.m_interval.rbegin() ;
//				iteInterval != sourceRow.m_interval.rend() ; iteInterval++ )
//		{
//			curRow.m_interval.push_back( -(*iteInterval) );
//		}	
//	}
//}

bool CTetrisLegal::DoRight(const double& prelegal_factor)
{
    //ReversePlacement();
    //SetReverseSite();
    //Change m_chip_left_bound
    m_chip_left_bound = m_placedb.m_coreRgn.left;

    bool bLegal = DoLeft( prelegal_factor );

    //ReversePlacement();
    //restore m_chip_left_bound
    m_chip_left_bound = m_placedb.m_coreRgn.left;

    return bLegal;
}

void CTetrisLegal::RestoreGlobalResult(void)
{
    for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
    {
	m_placedb.SetModuleLocation( iModule,
		m_globalLocations[iModule].x, 
		m_globalLocations[iModule].y);
    }
}


void CTetrisLegal::RestoreOrig(void)
{
    for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
    {
	m_placedb.SetModuleLocation( iModule,
		m_origLocations[iModule].x, 
		m_origLocations[iModule].y);
    }
}

void CTetrisLegal::RestoreBest(void)
{
    m_free_sites = m_best_sites;

    for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
    {
	m_placedb.SetModuleLocation( iModule,
		m_bestLocations[iModule].x, 
		m_bestLocations[iModule].y);
    }
}

void CTetrisLegal::SaveGlobalResult(void)
{
    for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
    {
	const Module& curModule = m_placedb.m_modules[iModule];
	m_globalLocations[iModule] = CPoint( curModule.m_x, curModule.m_y );	
    }
}

void CTetrisLegal::SaveOrig(void)
{
    for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
    {
	const Module& curModule = m_placedb.m_modules[iModule];
	m_origLocations[iModule] = CPoint( curModule.m_x, curModule.m_y );	
    }
}

void CTetrisLegal::SaveBest( const double& best_prelegal_factor, const double& best_cost )
{
    m_best_prelegal_factor = best_prelegal_factor;
    //m_best_cost = m_placedb.CalcHPWL();
    m_best_cost = best_cost;
    m_best_sites = m_free_sites;
    
    for( unsigned int iModule = 0 ; iModule < m_placedb.m_modules.size() ; iModule++ )
    {
	const Module& curModule = m_placedb.m_modules[iModule];
	m_bestLocations[iModule] = CPoint( curModule.m_x, curModule.m_y );	
    }
}


int CTetrisLegal::GetSiteIndex( const double& ycoor )
{
    if( ycoor >= m_free_sites.back().m_bottom )
	return m_free_sites.size() - 1;
    else if( ycoor < m_site_bottom + m_site_height )
	return 0;
    else
    {
	return static_cast<int>(floor( (ycoor-m_site_bottom)/m_site_height));
    }
}

CTetrisLegal::CTetrisLegal( CPlaceDB& placedb ) :
    m_placedb( placedb ),
    m_width_factor(1.0),
    m_left_factor(1.0),
    m_row_limit(10),
    m_unlegal_count(0),
    m_prelegal_factor(1.00),
    m_best_prelegal_factor( m_prelegal_factor ),
    m_best_cost( numeric_limits<double>::max() ),
    m_row_factor(10.0),
    m_bMacroLegalized(false),
    m_tetrisDir(0),         // 2007-07-10 (donnie)
    m_resultTetrisDir(0),   //   ..
    m_resultTetrisDiff(0)   //   ..
{

    //Added by Jin 20070308
    if( gArg.CheckExist( "cong" ) )
    {
	m_left_factor = 0.0;
    }
    //@Added by Jin 20070308
    
    //Compute average cell width
    int cell_count = 0;
    double total_width = 0;
    //double max_height = 0.0;
    m_max_module_height = 0.0;
    m_max_module_width = 0.0;
    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	const Module& curModule = m_placedb.m_modules[i];

	m_max_module_height = max( m_max_module_height, curModule.m_height );
	m_max_module_width = max( m_max_module_width, curModule.m_width );
	//Do not include fixed cells and macros
	if( curModule.m_isFixed || curModule.m_height > m_placedb.m_rowHeight )//|| curModule.m_width > 500)
	    continue;

	cell_count++;
	total_width += curModule.m_width;
    }

    double search_range_factor = 0.05;
    //ifstream parFile( "parameter" );
    //if( parFile )
    //{
    //	parFile >> search_range_factor;
    //}
    //parFile.close();

    m_row_limit = max( m_row_limit, 
	    static_cast<int>(ceil(m_max_module_height/m_placedb.m_rowHeight)*search_range_factor));
    //test code
    if( gArg.IsDev() )
	printf("\nsearch_range_factor: %.2f m_row_limit: %d\n", search_range_factor, m_row_limit );
    //@test code
    //m_row_limit = max( m_row_limit, 
    //		static_cast<int>(ceil(m_max_module_height/m_placedb.m_rowHeight)*0.1));
    //m_row_limit = max( m_row_limit, 
    //		static_cast<int>(ceil(m_max_module_height/m_placedb.m_rowHeight)*0.25));

    //cout << "Search row limit: " << m_row_limit << endl;
    m_average_cell_width = total_width / cell_count;
    m_macro_factor = m_average_cell_width * 5.0;
    LessXCoor::m_macro_factor = m_macro_factor;

    m_free_sites = m_placedb.m_sites;
    m_site_bottom = m_free_sites.front().m_bottom;
    m_site_height = m_free_sites.front().m_height;

    m_placedb.m_modules_bak = m_placedb.m_modules;

    //initalize m_origLocations and m_bestLocations
    m_origLocations.resize( m_placedb.m_modules.size() );
    m_bestLocations.resize( m_placedb.m_modules.size() );
    m_globalLocations.resize( m_placedb.m_modules.size() );

    m_chip_left_bound = m_placedb.m_coreRgn.left;

}


// 2007-07-10 (donnie)
bool CTetrisLegal::SolveAndReturn( const double& util,
	const bool& bMacroLegal,
	const bool& bRobust,
	const double& stop_prelegal_factor,
	int tetrisDir,		// 0: both  1: left  2: right
	int* pResultDir,
	double* pResultDiff	)
{
    
    m_tetrisDir = tetrisDir;
    if( Solve( util, bMacroLegal, bRobust, stop_prelegal_factor ) )
    {
	if( pResultDir != NULL )
	    *pResultDir = m_resultTetrisDir;
	if( pResultDiff != NULL )
	    *pResultDiff = m_resultTetrisDiff;
	return true;
    }
    return false;
}



bool CTetrisLegal::Solve( const double& util,
	const bool& bMacroLegal,
	const bool& bRobust,
	const double& stop_prelegal_factor	)
{

// #if 1    
//     //White space allocation
//     if( gArg.IsDev() && ( gArg.CheckExist( "wsaDensity" ) || gArg.CheckExist( "wsaRoutability" ) ) )
//     {
// 	int gridNum = static_cast<int>( sqrt( static_cast<double>(m_placedb.m_modules.size()) ) * 0.8 );

// 	//Make the # levels is one level rougher than the potential grids
// 	//int level = 1;
// 	int level = 0;
// 	int dimension = 1;
// 	while( dimension < gridNum )
// 	{
// 	    level++;
// 	    dimension = dimension * 2;
// 	}
// 	level = level * 2;

// 	fprintf( stdout, "White space allocation level: %d, utiliztion: %.2f\n", level, m_placedb.m_util );

// 	WhiteSpaceAllocation wsa( m_placedb, level );
	
// 	if( gArg.CheckExist( "wsaDensity" ) )
// 	{
// 	    wsa.SolveDensity(m_placedb.m_util);
// 	}
// 	else
// 	{
// 	    wsa.SolveRoutability();
// 	}
//     }
// #endif

    //Expand cell width to site step
    SaveModuleWidth();
    ExpandModuleWidthToSiteStep();
    
    m_bMacroLegalized = bMacroLegal;

    SaveGlobalResult();
    PrepareLeftRightFreeSites();

    bool bFinalLegal = false; 
    SetProcessList();
    bFinalLegal = DoLeftRightUntil( util, stop_prelegal_factor );

    if( !bRobust || bFinalLegal )
    {
	RestoreModuleWidth();
	
	if( gArg.IsDev() && gArg.CheckExist( "cong" ) && gArg.CheckExist( "congCellSliding" ) )
	{
// #if 1 
// 	    //test code for CCellSlidingForCongestion
// 	    //printf( "*******************************Go into congCellSliding.Solve()********************\n" );
// 	    if( true )
// 	    {
// 		m_placedb.OutputGnuplotFigureWithZoom( "sliding_before", false, true, true );
// 		int gridSize = static_cast<int>( sqrt( m_placedb.m_modules.size() ) );
// 		CPlaceBin placeBin( m_placedb );
// 		placeBin.CreateGrid( gridSize, gridSize );
// 		placeBin.OutputBinUtil( "sliding_before.density" );

// 		int gridNum = static_cast<int>( sqrt( static_cast<double>(m_placedb.m_modules.size()) ) * 0.8 );
// 		CCongMap congmap( gridNum, gridNum, m_placedb.m_coreRgn, m_placedb.m_pLEF );
// 		CGlobalRouting groute( m_placedb );
// 		groute.ProbalisticMethod( congmap );
// 		congmap.OutputBinOverflowFigure( "sliding_before.overflow" );
// 		//GRouteMap grouteMap( &m_placedb );
// 		//grouteMap.LoadCongMap( &congmap );
// 		//grouteMap.OutputGnuplotFigure( "sliding_before.cmp" );

// 	    }
// #endif
	
// 	    CCellSlidingForCongestion congCellSliding( m_placedb );
// 	    congCellSliding.Solve( -4.0, CCellSlidingForCongestion::CONG );

// #if 1
// 	    if( true )
// 	    {
// 		m_placedb.OutputGnuplotFigureWithZoom( "sliding_after", false, true, true );
// 		int gridSize = static_cast<int>( sqrt( m_placedb.m_modules.size() ) );
// 		CPlaceBin placeBin( m_placedb );
// 		placeBin.CreateGrid( gridSize, gridSize );
// 		placeBin.OutputBinUtil( "sliding_after.density" );

// 		int gridNum = static_cast<int>( sqrt( static_cast<double>(m_placedb.m_modules.size()) ) * 0.8 );
// 		CCongMap congmap( gridNum, gridNum, m_placedb.m_coreRgn, m_placedb.m_pLEF );
// 		CGlobalRouting groute( m_placedb );
// 		groute.ProbalisticMethod( congmap );
// 		congmap.OutputBinOverflowFigure( "sliding_after.overflow" );
// 		//GRouteMap grouteMap( &m_placedb );
// 		//grouteMap.LoadCongMap( &congmap );
// 		//grouteMap.OutputGnuplotFigure( "sliding_after.cmp" );

// 	    }
// #endif
	    //@test code for CCellSlidingForCongestion
	}
	return bFinalLegal;
    }

    //*********************
    //* Roubust Legalizer *
    //*********************
    
    //Run macro shifter first
    //then legal all cells
    RestoreGlobalResult();
    bool bMSLegal = MacroShifter(10,false);

    if( bMSLegal )
    {
	if( gArg.IsDev() )
	{
	    printf("Macro shifter: Success\n");
	}
	SetNonMacroProcessList( m_macro_ids ); 
	PrepareNonMacroLeftRightFreeSites( m_macro_ids );
	bool final = DoLeftRightUntil( util, stop_prelegal_factor ); 

	if( final )
	{
	    return final;
	}
    }
    else
    {
	if( gArg.IsDev() )
	{
	    printf("Macro shifter: Fail\n");
	}
    }

    //Final robust legalizer
    SetProcessList();
    PrepareLeftRightFreeSites();
    RestoreGlobalResult();
    SaveOrig();

    if( gArg.IsDev() )
    {
	cout << "Robust legalizer: " << endl;
    }

    double start = 1.0;
    double step = 0.05;
    double stop = 0.05;

    while( start >= stop )
    {
	double t = seconds();

	SetLeftFreeSites();
	RestoreOrig();

	if( gArg.IsDev() )
	{
	    cout << "Legalization factor: " << start << " ";
	}

	bool final = DoLeftMacroFirst(start);

	if( gArg.IsDev() )
	{
	    if( final )
		cout << "Success ";
	    else
		cout << "Fail ";
	    printf(" runtime %.2f secs\n", seconds()-t );
	    flush(cout);
	}
	else
	{
	    cout << "*";
	}

	if( final )
	{
	    return final;
	}
	else
	{
	    start = start - step;
	}
    }

    //Restore modules width
    RestoreModuleWidth();
    
    
    return bFinalLegal;

}













bool CTetrisLegal::DoLeft(const double& prelegal_factor)
{
    CalculateNewLocation(prelegal_factor);
    
    if( gArg.CheckExist( "legalMacroFirst" ) )
    {
	CalculateCellOrderMacroFirst();
    }
    else
    {
	CalculateCellOrder();
    }
    
    bool bLegal = LegalizeByCellOrder();
    
    return bLegal;
}

bool CTetrisLegal::DoLeftMacroFirst(const double& prelegal_factor)
{
    CalculateNewLocation(prelegal_factor);
    CalculateCellOrderMacroFirst();
    bool bLegal = LegalizeByCellOrder();

    return bLegal;

}

void CTetrisLegal::CalculateNewLocation(const double& prelegal_factor)
{
    for( unsigned int i = 0 ; i < m_process_list.size() ; i++ )
    {
	Module& curModule = m_placedb.m_modules[ m_process_list[i] ];
	if( curModule.m_isFixed )
	{
	    fprintf(stderr, "module %d should not be processed by CalculateNewLocation()\n", m_process_list[i] );
	    continue;
	}

	double newX = (curModule.m_x - m_chip_left_bound)*prelegal_factor + m_chip_left_bound;
	double newY = curModule.m_y;

	//Rounding for macros
	if( curModule.m_height > m_placedb.m_rowHeight )
	    newX = Rounding(newX);

	m_placedb.SetModuleLocation( m_process_list[i], newX, newY );
    }
}

void CTetrisLegal::CalculateCellOrderMacroFirst(void)
{
#if 0
    m_cell_order.clear();
    m_cell_order.reserve( m_placedb.m_modules.size() );

    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	if( !m_placedb.m_modules[i].m_isFixed )
	    m_cell_order.push_back(i);
    }
#endif
    m_cell_order = m_process_list;

    LessXCoorMacroFirst::m_placedb = &m_placedb;

    //Legal the macros first
    sort( m_cell_order.begin(), m_cell_order.end(), LessXCoorMacroFirst() );

}

void CTetrisLegal::CalculateCellOrder(void)
{
    m_cell_order = m_process_list;

    LessXCoor::m_placedb = &m_placedb;

    sort( m_cell_order.begin(), m_cell_order.end(), LessXCoor() );	
    //Legal the macros first
    //sort( m_cell_order.begin(), m_cell_order.end(), LessXCoorMacroFirst() );
}

bool CTetrisLegal::LegalizeByCellOrder(void)
{
    for( unsigned int i = 0 ; i < m_cell_order.size() ; i++ )
    {
	if( false == PlaceCellToLegalLocation( m_cell_order[i] ) )
	{
	    m_unlegal_count = m_cell_order.size() - i;
	    return false;
	}
    }
    return true;
}


void CTetrisLegal::GetLegalLocations( const int& cellid, vector<CLegalLocation>& legalLocations )
{
    legalLocations.clear();

    const Module& curModule = m_placedb.m_modules[cellid];
    //double xbound = floor( curModule.m_x - (m_left_factor*m_average_cell_width) );


    //const int& row_limit = m_row_limit;
    //Decide the row_limit acoording to cell's width
    int row_limit;
    if( m_row_limit < 1 )
    {
	row_limit = static_cast<int>(ceil(curModule.m_height/m_placedb.m_rowHeight)*m_row_factor);
    }
    else
    {
	row_limit = m_row_limit;	
    }

    //test code
    //Set the row_limit to full chip height
    //row_limit = m_placedb.m_sites.size();
    //@test code


    //Integration with macro legalizer
    //if( m_bMacroLegalized && curModule.m_height > m_placedb.m_rowHeight )
    //	row_limit = 0;

    //test code
    //cout << "row limit: " << row_limit << endl;
    //@test code

    //***********************************************************************
    //* For each row within the row_limit,                                *
    //* add the leftmost possible x location (>=xbound) into legalLocations *
    //***********************************************************************
    //For standard cells
    if( curModule.m_height <= m_placedb.m_rowHeight )
    {
	//int median_site_index = GetSiteIndex( curModule.m_y );
	//int low_site_index = max( 0, median_site_index - row_limit );
	//int high_site_index = min( static_cast<int>(m_free_sites.size())-1, 
	//		median_site_index + row_limit );

	//Enable/Disable GetCellLegalLocationsTowardLeft()
#if 1 
	AggressiveCellLegalLocationsSearch( cellid,
		legalLocations );
	//GetCellLegalLocationsTowardLeft( cellid,
	//		low_site_index,
	//		high_site_index,
	//		legalLocations,
	//		xbound );
#else
	for( int iRow = low_site_index ; iRow <= high_site_index ; iRow++ )
	{
	    const CSiteRow& curRow = m_free_sites[iRow];	

	    for( unsigned int iInterval = 0 ; 
		    iInterval < curRow.m_interval.size() ; 
		    iInterval = iInterval + 2 )
	    {
		double xstart = curRow.m_interval[iInterval];
		double xend = curRow.m_interval[iInterval+1];

		//Discard illegal intervals
		if( xend < xbound )
		    continue;

		xstart = max( xstart, xbound );
		//Check if this interval has enough width
		if( xend - xstart >= curModule.m_width )
		{
		    legalLocations.push_back( CLegalLocation( iRow, xstart ) );
		    break;
		}	
	    }	
	}
#endif		
    }
    //For macros (occupy more than one row)
    else
    {
	//int median_site_index = GetSiteIndex( curModule.m_y );
	//# of row occupied by this macro
	//int needed_row_count = static_cast<int>( ceil(curModule.m_height/m_site_height) );
	//int low_site_index = max( 0, median_site_index - row_limit );
	//int high_site_index = min( static_cast<int>(m_free_sites.size())-needed_row_count,
	//		median_site_index + row_limit );

	AggressiveMacroDiamondSearch( cellid, legalLocations );

#if 0
	if (true)
	{
	    vector<CSiteRow> sites;

	    const int module_row_number = static_cast<int>(ceil(curModule.m_height/m_placedb.m_rowHeight));

	    int highest_associated_row_index = 
		min( static_cast<int>(m_free_sites.size())-1, high_site_index+module_row_number-1 );

	    for( int iRow = low_site_index ; iRow <= highest_associated_row_index ; iRow++ )
	    {
		sites.push_back( m_free_sites[iRow] );
	    }

	    GetMacroLegalLocationsTowardLeft( cellid, sites, legalLocations, xbound );
	    //GetMacroLegalLocationsTowardOrig( cellid, sites, legalLocations);

	    for( unsigned int iLoc = 0 ; iLoc < legalLocations.size() ; iLoc++ )
	    {
		legalLocations[iLoc].m_site_index += low_site_index;
	    }

	    //test code
	    //cout << "legalLocations: ";
	    //for( unsigned int iLoc = 0 ; iLoc < legalLocations.size() ; iLoc++ )
	    //{
	    //	printf("(%d,%.2f) ", legalLocations[iLoc].m_site_index, legalLocations[iLoc].m_xcoor );
	    //}
	    //cout << endl;
	    //flush(cout);
	    //@test code
	}
	else
	{
	    //For each need-for-searching row, find if there is a legal location
	    for( int iRow = low_site_index ; iRow <= high_site_index ; iRow++ )
	    {
		//Find the leftest interval with the upon 'needed rows' which all have 
		//free space for this cell
		//"Scanline algorithm"
		vector<CTerminal> terminals;

		//Push all terminals into an array
		for( int iPushRow = iRow ; iPushRow < iRow+needed_row_count ; iPushRow++ )
		{
		    const CSiteRow& pushRow = m_free_sites[iPushRow];
		    for( unsigned int i = 0 ; i < pushRow.m_interval.size() ; i=i+2 )
		    {
			terminals.push_back( 
				CTerminal( pushRow.m_interval[i], CTerminal::Left, iRow ) );
			terminals.push_back( 
				CTerminal( pushRow.m_interval[i+1], CTerminal::Right, iRow ) );	
		    }
		}	

		//Sort terminals by their x coordinates			
		sort( terminals.begin(), terminals.end(), LessXCoor() );

		//To find a legal location,
		//we have to find overlapped segments with width >= curModule.m_width 
		int overlap_count = 0;
		for( unsigned int i = 0 ; i < terminals.size() ; i++ )
		{
		    const CTerminal& curTerminal = terminals[i];

		    if( curTerminal.m_type == CTerminal::Left )
		    {
			overlap_count++;	
		    }
		    else
		    {
			//Current segment is overlapped by all needed rows
			//Check if the width enough for this Macro
			if( overlap_count == needed_row_count )
			{
			    double xleft = max( xbound, terminals[i-1].m_xcoor );
			    double xright = curTerminal.m_xcoor;
			    double width = xright - xleft;

			    if( width >= curModule.m_width )
			    {
				legalLocations.push_back( 
					CLegalLocation( iRow, xleft ) );
				break;
			    }	
			}

			overlap_count--;
		    }	
		}
	    }//@For each need-for-searching row, find if there is a legal location
	}
#endif

    }
}

bool CTetrisLegal::PlaceCellToLegalLocation( const int& cellid )
{
    vector<CLegalLocation> legalLocations;
    GetLegalLocations( cellid, legalLocations );

#if 0
    //test code
    for( unsigned int i = 0 ; i < legalLocations.size() ; i++ )
    {
	printf("legalLocations[%d] %.2f\n", i, legalLocations[i].m_xcoor );	
    }
    //@Test code
#endif
    
    //Check if no legal locations are found
    if( legalLocations.empty() )
	return false;

    //const Module& curModule = m_placedb.m_modules[cellid];

    //Old method to find the best location in legalLocations
#if 0
    //*****************************************************
    //*Place the cell to the location with lowest cost and*
    //*update m_free_sites                                *
    //*****************************************************
    //Determine a best location among multiple legal locations
    //(put to the from of legalLocations)
    if( legalLocations.size() > 1 )	
    {

	//Calculate shift for each location
	for( vector<CLegalLocation>::iterator iteLoc = legalLocations.begin() ;
		iteLoc != legalLocations.end() ; iteLoc++ )
	{
	    CPoint p1( curModule.m_x, curModule.m_y );
	    CPoint p2( iteLoc->m_xcoor, m_free_sites[iteLoc->m_site_index].m_bottom );
	    iteLoc->m_shift = Distance( p1, p2 ); 
	}

	sort( legalLocations.begin(), legalLocations.end(), LessShift() );

	//There are at least two locations has the lowest shift
	//Remove locations with higher shift and sort by wirelength
	if( legalLocations[0].m_shift == legalLocations[1].m_shift )
	{
	    unsigned int new_size;
	    double min_shift = legalLocations[0].m_shift;

	    //Determine the new size
	    for( new_size = 2 ; 
		    new_size < legalLocations.size() && 
		    legalLocations[new_size].m_shift == min_shift ;
		    new_size++ );

	    legalLocations.resize( new_size, CLegalLocation(-1,-1) );

	    //Calculate wirelength for each location
	    for( vector<CLegalLocation>::iterator iteLoc = legalLocations.begin() ;
		    iteLoc != legalLocations.end() ; iteLoc++ )
	    {
		double xcoor = iteLoc->m_xcoor;
		double ycoor = m_free_sites[iteLoc->m_site_index].m_bottom;
		m_placedb.SetModuleLocation( cellid, xcoor, ycoor );
		iteLoc->m_wirelength = m_placedb.GetModuleTotalNetLength( cellid );
	    }

	    sort( legalLocations.begin(), legalLocations.end(), LessWirelength() );

	}

    }

    CLegalLocation bestLocation = legalLocations.front();
#endif

    int bestIndex = ReturnBestLocationIndex( cellid, legalLocations );
    CLegalLocation bestLocation = legalLocations[bestIndex];

    double xcoor = bestLocation.m_xcoor;
    double ycoor = m_free_sites[bestLocation.m_site_index].m_bottom;
    m_placedb.SetModuleLocation( cellid, xcoor, ycoor );


    //Old update free sites method
#if 0
    if( curModule.m_height <= m_placedb.m_rowHeight )
    {
	UpdateFreeSite( bestLocation.m_site_index, 
		xcoor, 
		curModule.m_width ); 
    }
    else
    {
	//Update free site for a macro
	int needed_row_count = static_cast<int>( ceil(curModule.m_height/m_site_height) );
	for( int iRow = bestLocation.m_site_index ; 
		iRow < bestLocation.m_site_index + needed_row_count ;
		iRow++ )
	{
	    UpdateFreeSite( iRow, xcoor, curModule.m_width );
	}
    }
#endif

    UpdateFreeSite( cellid );

    return true;

}

void CTetrisLegal::UpdateFreeSite( const int& rowId, const double& xstart, const double& width )
{
    CSiteRow& curRow = m_free_sites[rowId];

    const double xleft = xstart;
    const double xright = xstart + width;

    for( unsigned int i = 0 ; i < curRow.m_interval.size() ; i=i+2 )
    {
	double interval_left = curRow.m_interval[i];
	double interval_right = curRow.m_interval[i+1];

	//Avoid unneccessary check
	if( interval_right <= xleft )
	    continue;	

	//No further check is needed
	if( interval_left >= xright )
	    break;

	//Update free sites
	if( interval_left == xleft && interval_right == xright )
	{
	    //This situation is incorrect for Tetris legalizaer
	    //only left == left and right == right stands
	    //cerr <<	"Warning: incorrect module location in UpdateFreeSite()" << endl;
	    //cerr << "Row " << rowId << " module: (" << xleft << "," << xright << ")" 
	    //	<< " interval: (" << interval_left << "," << interval_right << )" << endl;
	    curRow.m_interval.erase( curRow.m_interval.begin()+i );
	    curRow.m_interval.erase( curRow.m_interval.begin()+i );
	    i = i-2;
	}
	else if( interval_right > xright && interval_left >= xleft )
	{
	    curRow.m_interval[i] = xright;
	}
	else if( interval_left < xleft && interval_right <= xright )
	{
	    curRow.m_interval[i+1] = xleft;	
	}
	else if( interval_left < xleft && interval_right > xright )
	{
	    curRow.m_interval[i+1] = xleft;
	    curRow.m_interval.insert( curRow.m_interval.begin()+(i+2), interval_right );
	    curRow.m_interval.insert( curRow.m_interval.begin()+(i+2), xright );
	    i = i+2;
	}
	else
	{
	    cerr << "Warning: UpdateFreeSite() Error" << endl;
	    fprintf(stderr, "bottom: %.2f interval: (%.2f,%.2f) target: (%.2f,%.2f)\n", 
		    curRow.m_bottom, interval_left, interval_right, xleft, xright );
	}
    }
}

void CTetrisLegal::SaveModuleWidth(void)
{
    m_orig_widths.resize( m_placedb.m_modules.size() );
    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	m_orig_widths[i] = m_placedb.m_modules[i].m_width;
    }
}
void CTetrisLegal::RestoreModuleWidth(void)
{
    if( m_orig_widths.size() >= m_placedb.m_modules.size() )
    {
	for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
	{
	    m_placedb.m_modules[i].m_width = m_orig_widths[i];
	}
    }
    else
    {
	fprintf( stderr, "Warning: Cannot restore cell width\n" );
    }
}

void CTetrisLegal::ExpandModuleWidthToSiteStep(void)
{
    double site_step = m_placedb.m_sites.front().m_step;
    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	double orig_width = m_placedb.m_modules[i].m_width;
	double new_width = ceil(orig_width/site_step)*site_step;
	m_placedb.m_modules[i].m_width = new_width;
    }
}



////////////////////////////////////////////////////


CTopDownLegal::CTopDownLegal( CPlaceDB* pDB )
: m_pDB( pDB )
{
    m_blockLegalized.resize( m_pDB->m_modules.size(), false );
    m_pLegal = new CTetrisLegal( *m_pDB );
}

CTopDownLegal::~CTopDownLegal()
{
}

bool CTopDownLegal::Legal( double util, double stop_prelegal_factor )
{
    printf( "Top down legal.  Util= %g   Prelegal= %g\n", util, stop_prelegal_factor );
    m_util = util;
    m_stop_prelegal_factor = stop_prelegal_factor;
    vector<int> blocks;
    GetBlockList( m_pDB->m_coreRgn, &blocks );
    return Legal( m_pDB->m_coreRgn, blocks );
}

bool CTopDownLegal::Legal( CRect& rect, vector<int>& blocks )
{
    if( blocks.size() == 0 )
	return true;
    double space = (rect.right - rect.left) * (rect.top - rect.bottom);
    //double smallest = m_pDB->m_rowHeight * m_pDB->m_rowHeight * 100;
    int part = 5;
    gArg.GetInt( "part", &part );
    double smallest = (m_pDB->m_coreRgn.right-m_pDB->m_coreRgn.left)*(m_pDB->m_coreRgn.top-m_pDB->m_coreRgn.bottom)/part;

    printf( "Legal (%.0f %.0f) - (%.0f %.0f) A= %g   block# %d\n", 
	    rect.left, rect.bottom, rect.right, rect.top, space, (int)blocks.size() );

    // find the cutline
    CRect subRect1 = rect, subRect2 = rect;
    int cutline;
    if( (rect.right - rect.left) > (rect.top - rect.bottom) )
    {
	subRect1.right = (rect.left + rect.right) * 0.5;
	subRect2.left  = (rect.left + rect.right) * 0.5;
	cutline = 1; // vertical
    }
    else
    {
	subRect1.top    = (rect.bottom + rect.top) * 0.5;
	subRect2.bottom = (rect.bottom + rect.top) * 0.5; 
	cutline = 2; //	horizontal
    }

    double free1 = GetRegionFreeSpace( subRect1 );
    double free2 = GetRegionFreeSpace( subRect2 );
    vector<int> blocks1;
    vector<int> blocks2;
    GetBlockList( subRect1, &blocks1, blocks );
    GetBlockList( subRect2, &blocks2, blocks );
    double used1 = GetBlockArea( blocks1 );
    double used2 = GetBlockArea( blocks2 );
    if( used1 >= free1 || used2 >= free2 || space < smallest )
    {
	// if any region is overflow, legal current region
	return m_pLegal->SolveRegion( rect, blocks, m_util, m_stop_prelegal_factor );
    }
    else
    {
	// if both region are not oveflow, legal both region
	bool succ1 = Legal( subRect1, blocks1 );
	bool succ2 = Legal( subRect2, blocks2 );
	
	if( succ1 == false || succ2 == false )
	    return m_pLegal->SolveRegion( rect, blocks, m_util, m_stop_prelegal_factor );
	else
	    return true;
    }	
}

double CTopDownLegal::GetRegionFreeSpace( CRect& rect )
{
    return GetSiteSpace( m_pDB->m_sites, rect );
}


void   CTopDownLegal::GetBlockList( CRect& rect, vector<int> *pList, vector<int>& blocks )
{
    for( unsigned int j=0; j<blocks.size(); j++ )
    {
	int i = blocks[j];
	if( m_pDB->m_modules[i].m_cx < rect.left || m_pDB->m_modules[i].m_cx > rect.right )
	    continue;
	if( m_pDB->m_modules[i].m_cy < rect.bottom || m_pDB->m_modules[i].m_cy > rect.top )
	    continue;
	if( m_blockLegalized[i] )
	    continue;
	pList->push_back( i );
    }
    pList->resize( pList->size() );
}

void   CTopDownLegal::GetBlockList( CRect& rect, vector<int> *pList )
{
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	if( m_pDB->m_modules[i].m_cx < rect.left || m_pDB->m_modules[i].m_cx > rect.right )
	    continue;
	if( m_pDB->m_modules[i].m_cy < rect.bottom || m_pDB->m_modules[i].m_cy > rect.top )
	    continue;
	if( m_blockLegalized[i] )
	    continue;
	if( m_pDB->m_modules[i].m_isFixed )
	    continue;
	pList->push_back( i );
    }
    pList->resize( pList->size() );
}

double CTopDownLegal::GetBlockArea( vector<int>& list )
{
    double area = 0;
    for( unsigned int i=0; i<list.size(); i++ )
	area += m_pDB->m_modules[ list[i] ].m_area;
    return area;
}

double CTopDownLegal::GetSiteSpace( const vector<CSiteRow>& source_sites, const CRect& region )
{
    int start_row_index, end_row_index;
    start_row_index = static_cast<int>( ceil( ( region.bottom - source_sites.front().m_bottom ) / 
	source_sites.front().m_height ) );
    end_row_index = static_cast<int>( floor( ( region.top - source_sites.front().m_bottom ) / 
	source_sites.front().m_height ) );

    start_row_index = min( start_row_index, (int)source_sites.size() - 1 );
    start_row_index = max( start_row_index, 0 );
    end_row_index = min( end_row_index, (int)source_sites.size() - 1 );
    end_row_index = max( end_row_index, 0 );

    double space = 0;
    for( int row_index = start_row_index ; row_index <= end_row_index ; row_index++ )
    {
	const CSiteRow& curRow = source_sites[ row_index ];
	for( unsigned int iInterval = 0 ; 
		iInterval < curRow.m_interval.size() ; 
		iInterval = iInterval+2 )
	{
	    double left = curRow.m_interval[iInterval];
	    double right = curRow.m_interval[iInterval+1];
	    
	    if( left >= region.right )
		break;
	    else if ( right <= region.left )
		continue;

	    left = max( left, region.left );
	    right = min( right, region.right );
	    
	    space += (right - left);
	}
    }
    return space * m_pDB->m_rowHeight;
}


			
