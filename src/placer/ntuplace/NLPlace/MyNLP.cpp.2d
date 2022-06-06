#include <cmath>
#include <set>
#include <vector>
#include <algorithm>
#include <cfloat>
#include <iostream>
#include <fstream>
#include <climits>
using namespace std;

#include <pthread.h>
#include <semaphore.h>

#include "../PlaceDB/placedb.h"
#include "MyNLP.h"
#include "smooth.h"
#include "../Legalizer/TetrisLegal.h"
#include "../PlaceCommon/placebin.h"
#include "../PlaceCommon/ParamPlacement.h"
#include "../PlaceCommon/placeutil.h"
//#include "CongMap.h"
#include "../PlaceCommon/arghandler.h"
#include "../PlaceCommon/util.h"
//#include "../PlaceDB/SteinerDecomposition.h"
#include "../PlaceCommon/BlockMatching.h"
//#include "GRouteMap.cpp"

// HLBFGS optimzation
#include "../lib/hlbfgs/HLBFGS.h"

//Added by kaie
#include "../lib/lpsolve55/lp_lib.h"

MyNLP *mynlp;

//bool bMulti = false;

bool bFast = true;  // spreading force interpolation

double gDensity;
double gTotalWL;

double time_wire_force = 0;
double time_spreading_force = 0;

// static variables
double MyNLP::m_yWeight = 1.0;
vector< vector< double > > MyNLP::m_weightDensity;

//double pi = 3.1415926;

/* Constructor. */
    MyNLP::MyNLP( CPlaceDB& db )
: _potentialGridR( 2 ),
    m_potentialGridHSize( -1 ),
    m_potentialGridVSize( -1 ),
    m_pGRMap( NULL )
{

    //if( gArg.CheckExist( "multi" ) )
	//bMulti = true;

    m_precision = 0.99999;
    m_weightIncreaseFactor = 2.0;
    m_targetUtil = 1.0;
    gArg.GetDouble( "util", &m_targetUtil );
    m_bRunLAL = true;
    m_binSize = 0.8; 
    m_potentialGridPadding = 0;

    m_smoothR = 5;	// Gaussian smooth R

    if( gArg.CheckExist( "nolal" ) )
	m_bRunLAL = false;
    
    m_maxIte = 50;	// max outerIte
    gArg.GetInt( "maxIte", &m_maxIte );    

    if( gArg.IsDev() )
    {
	gArg.GetDouble( "precision",  &m_precision );
	gArg.GetDouble( "incFactor",  &m_weightIncreaseFactor );
	gArg.GetDouble( "binSize",    &m_binSize );
	gArg.GetDouble( "yWeight",    &m_yWeight );
	gArg.GetInt( "padding",    &m_potentialGridPadding );
	gArg.GetInt( "smoothr", &m_smoothR );

	printf( "\n" );
	printf( "[Analytical Placement Parameters]\n" );
	printf( "    solver precision            = %g\n", m_precision );
	printf( "    weight increasing factor    = %g\n", m_weightIncreaseFactor );
	printf( "    target utilization          = %g\n", m_targetUtil );
	printf( "    bin size factor             = %g\n", m_binSize );
	printf( "    G-smooth r                  = %d\n", m_smoothR );

	if( m_bRunLAL )
	    printf( "    use look-ahead legalization = %s\n", TrueFalse( m_bRunLAL ).c_str() );

	if( m_yWeight != 1 )
	    printf( "    vertical weight             = %.2f\n", m_yWeight );

	if( m_potentialGridPadding > 0 )
	    printf( "    potential grid padding      = %d\n", m_potentialGridPadding );

	printf( "\n" );
    }

    m_lookAheadLegalization = false;
    m_earlyStop = false;
    m_topLevel = false;

    //m_weightWire = 4.0;
    m_smoothDelta = 1;

    m_mdtime = 0; // kaie

    m_pDB = &db;
    InitModuleNetPinId();	    

    // scale between 0 to 10
    const double range = 10.0;
    if( m_pDB->m_coreRgn.right > m_pDB->m_coreRgn.top )
	m_posScale = range / m_pDB->m_coreRgn.right;	
    else
	m_posScale = range / m_pDB->m_coreRgn.top;	

    _cellPotentialNorm.resize( m_pDB->m_modules.size() );

    x.resize( 2 * m_pDB->m_modules.size() );
    xBest.resize( 2 * m_pDB->m_modules.size() );
    if( !param.bUseSLSE  /*&& !gArg.CheckExist("SWAE")*/ ) // kaie
    {
    	_expX.resize( 2 * m_pDB->m_modules.size() );
    	_expPins.resize( 2 * m_pDB->m_pins.size() );
    }
    x_l.resize( 2 * m_pDB->m_modules.size() );
    x_u.resize( 2 * m_pDB->m_modules.size() );
    grad_f.resize( 2 * m_pDB->m_modules.size() );
    last_grad_f.resize( 2 * m_pDB->m_modules.size() );
    walk_direction.resize(2 * m_pDB->m_modules.size(), 0);
    last_walk_direction.resize(2 * m_pDB->m_modules.size(), 0);

    if( param.bUseMacroRT ) // (kaie) macro rotation
    {
	grad_f_r.resize( m_pDB->m_modules.size() );
	last_grad_f_r.resize( m_pDB->m_modules.size() );
	walk_direction_r.resize( m_pDB->m_modules.size(), 0);
	last_walk_direction_r.resize( m_pDB->m_modules.size(), 0);
    }

    m_usePin.resize( m_pDB->m_modules.size() );
    SetUsePin();

    m_nets_sum_exp_xi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_exp_yi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_exp_inv_xi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_exp_inv_yi_over_alpha.resize( m_pDB->m_nets.size(), 0 );

    // (kaie) 2011-05-11 Weighted-Average-Exponential Wirelength Model
    /*if( gArg.CheckExist("WAE") || gArg.CheckExist("SWAE") )
    {
	m_nets_weighted_sum_exp_xi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
	m_nets_weighted_sum_exp_yi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
	m_nets_weighted_sum_exp_inv_xi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
	m_nets_weighted_sum_exp_inv_yi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
	//sum_delta_x.resize( m_pDB->m_pins.size() );
	//sum_delta_inv_x.resize( m_pDB->m_pins.size() );
	//sum_delta_y.resize( m_pDB->m_pins.size() );
	//sum_delta_inv_y.resize( m_pDB->m_pins.size() );
    }*/

    // (kaie) 2010-09-23 Stable-LSE
    if( param.bUseSLSE /*|| gArg.CheckExist("SWAE")*/ )
    {
    	m_nets_max_xi.resize( m_pDB->m_nets.size(), 0 );
    	m_nets_min_xi.resize( m_pDB->m_nets.size(), 0 );
    	m_nets_max_yi.resize( m_pDB->m_nets.size(), 0 );
    	m_nets_min_yi.resize( m_pDB->m_nets.size(), 0 );
    	expXnet.resize( m_pDB->m_nets.size() );
	for(unsigned int i = 0 ; i < m_pDB->m_nets.size(); i++)
	    expXnet[i].resize( 4 * m_pDB->m_nets[i].size(), 0 );
	//pin_position.resize( 2 * m_pDB->m_pins.size(), 0.0 );
    }

    if( param.bUseLSE == false )
    {
	m_nets_sum_p_x_pos.resize( m_pDB->m_nets.size(), 0 );
	m_nets_sum_p_y_pos.resize( m_pDB->m_nets.size(), 0 );
	m_nets_sum_p_inv_x_pos.resize( m_pDB->m_nets.size(), 0 );
	m_nets_sum_p_inv_y_pos.resize( m_pDB->m_nets.size(), 0 );
	m_nets_sum_p_x_neg.resize( m_pDB->m_nets.size(), 0 );
	m_nets_sum_p_y_neg.resize( m_pDB->m_nets.size(), 0 );
	m_nets_sum_p_inv_x_neg.resize( m_pDB->m_nets.size(), 0 );
	m_nets_sum_p_inv_y_neg.resize( m_pDB->m_nets.size(), 0 );
    }

    grad_wire.resize( 2 * m_pDB->m_modules.size(), 0.0 );
    grad_potential.resize( 2 * m_pDB->m_modules.size(), 0.0 );

    m_weightedForce = false;
    weightF.resize( m_pDB->m_modules.size() );// (kaie) weighted force

    // (kaie) macro rotation
    if( param.bUseMacroRT )
    {
	rotate_degree.resize( m_pDB->m_modules.size(), 0.0);
	rotate_off.resize( 2 * m_pDB->m_modules.size(), 0.0);
	grad_rotate.resize( m_pDB->m_modules.size(), 0.0 );
	grad_potentialR.resize( m_pDB->m_modules.size(), 0.0 );
    }

    m_totalMovableModuleArea = 0;
    for( unsigned int i = 0; i < m_pDB->m_modules.size(); i++ )
    {
    	if( m_pDB->m_modules[i].m_isFixed == false )
	    m_totalMovableModuleArea += m_pDB->m_modules[i].m_area;
    }

    m_threadInfo.resize( param.nThread );
    m_threadResult.resize( param.nThread );

}

MyNLP::~MyNLP()
{
    delete m_pGRMap;
}

void MyNLP::SetUsePin()
{
    int effectivePinCount = 0;
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	bool usePin = false;
	for( unsigned int p=0; p<m_pDB->m_modules[i].m_pinsId.size(); p++ )
	{
	    int pinId = m_pDB->m_modules[i].m_pinsId[p];

	    if( m_pDB->m_pins[pinId].xOff != 0.0 || m_pDB->m_pins[pinId].yOff != 0.0 )
	    {
		usePin = true;
		break;
	    }
	}

	// 2006-04-23 (donnie)
	if( param.bHandleOrientation && 
		m_pDB->m_modules[i].m_height == m_pDB->m_rowHeight &&
		m_pDB->m_modules[i].m_isFixed == false )
	    usePin = false;	    

	if( usePin )
	    effectivePinCount++;
	m_usePin[i] = usePin;
    }
    if( param.bShow )
	printf( "Effective Pin # = %d\n", effectivePinCount );
}

bool MyNLP::MySolve( double wWire, 
	double target_density, 
	int currentLevel	// for plotting
	)
{
    double time_start = seconds();    
    double time_start_real = seconds_real();    
    assert( _potentialGridR > 0 );

    if( m_potentialGridHSize <= 0 || m_potentialGridVSize <= 0 )
    {
	m_potentialGridHSize = static_cast<int>( sqrt(static_cast<double>( m_pDB->m_modules.size()) ) * m_binSize );
	m_potentialGridVSize = static_cast<int>( sqrt(static_cast<double>( m_pDB->m_modules.size()) ) * m_binSize );
    }

    int n, m, nnz_jac_g, nnz_h_lag;
    get_nlp_info( n, m, nnz_jac_g, nnz_h_lag );
    get_bounds_info( n, x_l, x_u );

    m_ite = 0;
    bool isLegal = false;

    assert( param.dLpNorm_P > 0 );
    if( param.bUseLSE )
    {
	//_alpha = 0.5 * m_potentialGridWidth; // according to APlace ispd04
	//double maxValue = param.dLpNorm_P;	// > 700 leads to overflow
	
	//_alpha = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) * 0.005; // as small as possible -- NLP (for contest)
	//_alpha = max(abs(m_pDB->m_coreRgn.right), abs(m_pDB->m_coreRgn.left)) / log10(DBL_MAX);
	_alpha = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / 2 / param.dLpNorm_P;
	gArg.GetDouble("alpha", &_alpha);
	double scale = 1.0;
	gArg.GetDouble("ascale", &scale);
	_alpha /= scale; 
    }
    else
    {
	// Lp-norm
	//_alpha = param.dLpNorm_P;
	_alpha = log10( DBL_MAX ) ;
    }

    if( param.bShow )
	printf( "GRID = %d x %d (width = %.2f, height = %.2f)  alpha= %f  weightWire= %f\n", 
		m_potentialGridHSize, m_potentialGridVSize,
		( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left )/m_potentialGridHSize, 
		( m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom )/m_potentialGridVSize,
		_alpha, wWire );

    // (kaie) classify macros
    int num_macros = 0;
    //double avg_area = m_pDB->m_totalModuleArea / m_pDB->m_modules.size();
    for(unsigned int i = 0; i < m_pDB->m_modules.size(); i++)
    {
    	//if(m_pDB->m_modules[i].m_isFixed)
	    //weightF[i] = 0.0;
	//else
	if(m_pDB->m_modules[i].m_area > m_pDB->m_rowHeight)
	    weightF[i] = 1.0 / m_pDB->m_modules[i].m_area;
	else
	    weightF[i] = 1.0;

	if(//m_pDB->m_modules[i].m_area > 5 * avg_area &&
		m_pDB->m_modules[i].m_height > m_pDB->m_rowHeight &&
		m_pDB->m_modules[i].m_isFixed == false &&
		m_pDB->BlockOutCore(i) == false &&
		m_pDB->m_modules[i].m_isCluster == false)
	{
	    m_pDB->m_modules[i].m_isMacro = true;
	    num_macros++;
	}else
	    m_pDB->m_modules[i].m_isMacro = false;
    }
    printf("#Macros = %d\n", num_macros);
    // @(kaie)

    int counter = 0;
    while( true )
    {
	counter++;

	// save the block position
	vector< CPoint > blockPositions;
	CPlaceUtil::SavePlacement( *m_pDB, blockPositions );
	get_starting_point( x );
	Parallel( BoundXThread, n );
	printf("alpha = %f\n", _alpha);

	bool succ = GoSolve( wWire, target_density, currentLevel, &isLegal );

	if( succ )
	{
	    break;
	}
	// (kaie) 2009-09-28 Stable-LSE
	//if( param.bUseSLSE )
	//    _alpha /= 2;
	//else
	    _alpha -= 5;
	
	CPlaceUtil::LoadPlacement( *m_pDB, blockPositions );

	if( gArg.IsDev() )
	    printf( "\nFailed to solve it. alpha = %f\n\n", _alpha );
	if( _alpha < 80 )
	{
	    printf( "Fatal error in analytical solver. Exit program.\n" ); 
	    exit(0);
	}
    }

    if( param.bShow )
    {
	double realLevelTime = double(seconds_real()-time_start_real);
	double levelTime     = double(seconds()-time_start);
	printf( "HPWL = %.0f\n", m_pDB->CalcHPWL() );
	printf( "Time wire = %.0f s   spreading = %.0f s\n", time_wire_force, time_spreading_force );
	printf( "Level Time (Real) = %.2f (%.2f) sec = %.2f (%.2f) min (%.2fX)\n", 
		levelTime, realLevelTime, levelTime/60.0, realLevelTime/60.0, levelTime/realLevelTime );
	if(gArg.CheckExist("mod")) // (kaie) macro rotation
	    printf( "Macro Orientation Determination Time = %.0f s(%.0f%%)\n", m_mdtime, m_mdtime/levelTime * 100.0);
    }

    return isLegal;
}

bool MyNLP::GoSolve( double wWire, 
	double target_density, 
	int currentLevel,	// for plotting
	bool* isLegal
	)
{

    *isLegal = false;

    time_wire_force = time_spreading_force = 0.0;

    m_currentStep = param.step;

    m_targetUtil += param.targetDenOver;
    if( m_targetUtil > 1.0 )
	m_targetUtil = 1.0;

    double time_start = seconds();    

    int n = 2 * m_pDB->m_modules.size();

    double designUtil = m_pDB->m_totalMovableModuleArea / m_pDB->m_totalFreeSpace;

    double baseUtil = 0.05;  // experience value preventing over-spreading
    gArg.GetDouble( "baseUtil", &baseUtil );
    m_targetUtil = min( 1.0, m_targetUtil + baseUtil );

    double lowestUtil = min( 1.0, designUtil + baseUtil );
    if( m_targetUtil > 0 )  // has user-defined target utilization
    {
	if( m_targetUtil < lowestUtil )
	{
	    if( param.bShow )
		printf( "NOTE: Target utilization (%f) is too low\n", m_targetUtil );
	    m_targetUtil = lowestUtil;
	}
    }
    else // no given utilization
    {
	printf( "No given target utilization.\n" ); //  Distribute blocks evenly
	m_targetUtil = lowestUtil;
    }

    if( param.bShow )
    {
	printf( "INFO: Design utilization: %f\n", designUtil );
	printf( "DBIN: Target utilization: %f\n", m_targetUtil );
    }

    fill( grad_f.begin(), grad_f.end(), 0.0 ); 
    fill( last_grad_f.begin(), last_grad_f.end(), 0.0 ); 

    // Wirelength
    if( param.bUseSLSE /*|| gArg.CheckExist("SWAE")*/ )
    {
    	Parallel( UpdateMaxMinPinForEachNetThread, m_pDB->m_nets.size() ); // (kaie) 2009-09-23 SLSE
    	Parallel( UpdateExpValueForEachNetThread, m_pDB->m_nets.size() ); // (kaie) 2009-09-23 SLSE 
    }else
    {
        Parallel( UpdateExpValueForEachCellThread, m_pDB->m_modules.size() );
        Parallel( UpdateExpValueForEachPinThread, m_pDB->m_pins.size() );
    }
    Parallel( UpdateNetsSumExpThread, (int)m_pDB->m_nets.size() );
    //if( gArg.CheckExist("WAE") || gArg.CheckExist("SWAE") ) ComputeNetSumDelta(); // (kaie) Weighted-Average Wirelength Model

    // Density Grid
    int densityGridHSize = m_potentialGridHSize / 4;   // Use larger grid for density computing
    int densityGridVSize = m_potentialGridVSize / 4;
    CreateDensityGrid( densityGridHSize, densityGridVSize );
    UpdateDensityGridSpace( x );
    UpdateDensityGrid( x );
    double maxDen = GetMaxDensity();
    double totalOverDen = GetTotalOverDensity();

    // Potential Grid
    CreatePotentialGrid();   // Create potential grid according to "m_potentialGridHSize" and "m_potentialGridVSize"
    UpdatePotentialGridBase( x ); // Fixed blocks
    SmoothBasePotential();
    UpdateExpBinPotential( /*m_targetUtil*/ 1.0, true );
    Parallel( ComputeNewPotentialGridThread, m_pDB->m_modules.size() );
    UpdatePotentialGrid();
    double totalOverPotential = GetTotalOverPotential();

    if( !InitObjWeights( wWire ) )
    {
	printf("InitObjWeight OVERFLOW!\n");
	return false;// overflow
    }

    //if( /*m_prototype &&*/ currentLevel > 1 )
    /*{
	Optimization();
	return true;
	char filename[256];
	sprintf(filename, "test%d.plt", currentLevel);
	m_pDB->OutputGnuplotFigureWithZoom( filename, false, false, false, false, false);
    }*/

    double obj_value = eval_f();
    //ComputeBinGrad();
    Parallel( eval_grad_f_thread, m_pDB->m_modules.size() );

    if( obj_value > DBL_MAX * 0.5 )
    {
	printf("Objective value OVERFLOW!\n");
	return false; // overflow
    }

    if( param.bShow )
    {	
	printf( " %d-%2d HPWL= %.0f\tDen= %.2f %.4f %.4f Dcost= %4.1f%%  ",  
		currentLevel, m_ite, m_pDB->CalcHPWL(), 
		maxDen, totalOverDen, totalOverPotential,
		gDensity * _weightDensity / obj_value * 100.0 ); 
    }
    else
    {
	printf( " %d-%2d HPWL= %.0f \t", currentLevel, m_ite, m_pDB->CalcHPWL() );
    }
    fflush( stdout );

    // Lookahead Legalization
    bool hasBestLegalSol = false;
    double bestLegalWL = DBL_MAX;
    int lookAheadLegalCount = 0;
    double totalLegalTime = 0.0;
    //int tetrisDir = 0;        // 0: both   1: left   2: right
    int LALnoGoodCount = 0;
    int maxNoGoodCount = 2;
    if( param.bPrototyping )
    	maxNoGoodCount = 0;
    vector<Module> bestGPresult;        // for LAL
    // (kaie) macro rotation
    vector<char> bestOrient;
    if( m_lookAheadLegalization && m_macroRotate)
	bestOrient.resize(m_pDB->m_modules.size());
    // @(kaie) macro rotation

    // Convergence Criteria
    int totalIte = 0;
    //bool startDecreasing = false;
    int checkStep = 5;
    int outStep = 50;
    gArg.GetInt( "outputStep", &outStep );
    if( param.bShow == false )
    	outStep = INT_MAX;
    double lastMaxDen = DBL_MAX;
    double lastTotalOverDen = DBL_MAX;
    
    if( gArg.CheckExist( "gpfig" ) )
    {
	m_pDB->m_modules_bak = m_pDB->m_modules;

	char postfix[10];
	sprintf( postfix, "%d-%d", currentLevel, m_ite );
	//PlotGPFigures( postfix );

	char fn[255];
	sprintf( fn, "base%d", currentLevel );
	CMatrixPlotter::OutputGnuplotFigure( m_basePotential, fn, 
		"", // title
		m_potentialGridWidth * m_potentialGridHeight,  // limit
		true, // scale 
		0 );  // limit base
    }

    int maxIte = m_maxIte;
    bool newDir = true;
    bool bUpdateWeight = true;
    for( int ite = 0; ite < maxIte; ite++ )
    {
	m_ite++;
	int innerIte = 0;
	double old_obj = DBL_MAX;
	double last_obj_value = DBL_MAX;

	if( bUpdateWeight == false )
	    newDir = false;
	else
	    newDir = true;
	bUpdateWeight = true;

	while( true )	// inner loop, minimize "f" 
	{
	    innerIte++;
	    swap( last_grad_f, grad_f );    // save for computing the congujate gradient direction
	    swap( last_walk_direction, walk_direction );

	    if(m_macroRotate) // (kaie) macro rotation
	    {
		swap( last_grad_f_r, grad_f_r );
		swap( last_walk_direction_r, walk_direction_r );
	    }

	    //ComputeBinGrad();
	    Parallel( eval_grad_f_thread, m_pDB->m_modules.size() );

	    if(m_weightedForce) WeightedForce( n, grad_f, weightF );
	    if( !AdjustForce( n, x, grad_f ) )
	    {
		printf("AdjustForce, NaN or Inf\n");
		return false;	// NaN or Inf
	    }

	    if(m_macroRotate)
	    {
		if(m_weightedForce) WeightedForce( m_pDB->m_modules.size(), grad_rotate, grad_potentialR, weightF );
		if(!AdjustForceR( m_pDB->m_modules.size(), grad_rotate, grad_potentialR))
		{
		    printf("AdjustForceR, NaN or Inf\n");
		    return false; // NaN or Inf
		}
	    }

	    if( innerIte % checkStep == 0 )
	    {
	    	old_obj = last_obj_value;    // backup the old value
		obj_value = eval_f();
		last_obj_value = obj_value;
	    }

	    // Output solving progress
	    if( innerIte % outStep == 0 /*&& innerIte != 0*/ )
	    {
		if( innerIte % checkStep != 0 )
		    obj_value = eval_f();
		printf( "\n  (%4d): f %g\t w %g\t p %g\tstep= %.5f \t%.1fm ", 
			innerIte, obj_value, gTotalWL, gDensity, m_stepSize,
			double(seconds()-time_start)/60.0
		      );
		fflush( stdout );
	    }

	    if( innerIte % checkStep == 0 )
	    {
		printf( "." );
		fflush( stdout );

		if( obj_value >= m_precision * old_obj)    // Cannot further reduce "f"
		    break;
	    }

	    if( newDir == true )
	    {
		// gradient direction
		newDir = false;
		for( int i = 0; i < n; i++ )
		{
		    grad_f[i] = -grad_f[i];
		    walk_direction[i] = grad_f[i];
		}
	    }
	    else
	    {
		// conjugate gradient direction
		if( FindBeta( n, grad_f, last_grad_f, m_beta ) == false )
		{
		    printf("FindBeta OVERFLOW!\n");
		    return false;   // overflow?
		}
		Parallel( UpdateGradThread, n );
		if(m_macroRotate)
		{
		    if( FindBeta( m_pDB->m_modules.size(), grad_f_r, last_grad_f_r, m_beta_r ) == false )
		    {
			printf("FindBetaR OVERFLOW!\n");
			m_beta_r = 0;
			//return false;
		    }
		    Parallel( UpdateGradRThread, m_pDB->m_modules.size() );
		}
	    }

	    LineSearch( n, x, walk_direction, m_stepSize ); // Calculate a_k (step size)
	    Parallel( UpdateXThread, n );	    // Update X. (x_{k+1} = x_{k} + \alpha_k * d_k)
	    Parallel( BoundXThread, n );
	    if(m_macroRotate)
	    {
		LineSearchR( m_pDB->m_modules.size(), walk_direction_r, m_stepSizeR );
		m_stepSizeR /= fabs( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left );
		Parallel( UpdateRThread, m_pDB->m_modules.size()  ); // (kaie) macro rotation
	    }

	    // New block positions must be ready
	    // 1. UpdateExpValueForEachCellThread    (wire force)
	    // 2. UpdateExpValueForEachPinThread     (wire force)
	    // 3. ComputeNewPotentialGridThread      (spreading force)

	    double time_used = seconds();
	    if( param.bUseSLSE /*|| gArg.CheckExist("SWAE")*/)
	    {
	    	Parallel( UpdateMaxMinPinForEachNetThread, m_pDB->m_nets.size() ); // (kaie) 2009-09-23 SLSE
	    	Parallel( UpdateExpValueForEachNetThread, m_pDB->m_nets.size() ); // (kaie) 2009-09-23 SLSE
	    }else
	    {
		Parallel( UpdateExpValueForEachCellThread, m_pDB->m_modules.size() );
		Parallel( UpdateExpValueForEachPinThread, m_pDB->m_pins.size() );
	    }
	    Parallel( UpdateNetsSumExpThread, (int)m_pDB->m_nets.size() );
	    //if( gArg.CheckExist("WAE") || gArg.CheckExist("SWAE") ) ComputeNetSumDelta(); // (kaie) Weighted-Average Wirelength Model
	    time_wire_force += seconds() - time_used;

	    time_used = seconds();
	    Parallel( ComputeNewPotentialGridThread, m_pDB->m_modules.size() );
	    UpdatePotentialGrid();
	    time_spreading_force += seconds() - time_used;

	}// inner loop

	if( param.bShow )
	{
	    printf( "%d\n", innerIte );
	    fflush( stdout );
	}
	else
	    printf( "\n" );
	totalIte += innerIte;

	Parallel( UpdateBlockPositionThread, m_pDB->m_modules.size() );  // update to placeDB
	if(m_macroRotate) // (kaie) macro rotation
	    UpdateBlockOrientation();

	UpdateDensityGrid( x );
	maxDen = GetMaxDensity();
	totalOverDen = GetTotalOverDensity();
	totalOverPotential = GetTotalOverPotential();
	
	if( obj_value > DBL_MAX * 0.5 )
	{
	    printf("Objective value OVERFLOW!\n");;
	    return false; // overflow
	}

	if( param.bShow )
	{
	    printf( " %d-%2d HPWL= %.0f\tDen= %.2f %.4f %.4f LCPU= %.1fm Dcost= %4.1f%%", 
		    currentLevel, m_ite, m_pDB->CalcHPWL(), 
		    maxDen, totalOverDen, totalOverPotential,
		    double(seconds()-time_start)/60.0, 
		    0.5 * gDensity * _weightDensity /obj_value * 100.0); 

	    if( gArg.CheckExist( "gpfig" ) )
	    {
		char postfix[10];
		sprintf( postfix, "%d-%d", currentLevel, m_ite );  
		//PlotGPFigures( postfix );
	    }
	}
	else
	{
	    printf( " %d-%2d HPWL= %.f\tLCPU= %.1fm ", 
		    currentLevel, m_ite, m_pDB->CalcHPWL(), double(seconds()-time_start)/60.0 );
	}
	fflush( stdout );

	bool spreadEnough = totalOverDen < target_density + 0.2;
	bool increaseOverDen = totalOverDen > lastTotalOverDen;
	bool increaseMaxDen = maxDen > lastMaxDen;
	bool enoughIteration = ite > 3;
	bool notEfficientOptimize = 0.5 * gDensity * _weightDensity / obj_value * 100.0 > 95;

	if( enoughIteration && notEfficientOptimize )
	{
	    printf( "Failed to further optimize" );
	    break;
	}

	if( enoughIteration && increaseOverDen && increaseMaxDen && spreadEnough )
	{
	    printf( "Cannot further reduce over density!\n" ); // skip LAL
	    break;
	}

	int startLALIte = 1;
	if( m_bRunLAL 
		//&& startDecreasing  // 2006-10-23
		&& m_ite >= startLALIte 
		&& m_lookAheadLegalization 
		&& totalOverDen < target_density + 0.2
	  )
	{
	    Parallel( UpdateBlockPositionThread, m_pDB->m_modules.size() );
	    double hpwl = m_pDB->CalcHPWL();
	    if( hpwl > bestLegalWL )
	    {
		printf( "Stop. Good enough.\n" );
		break;	// stop placement
	    }

	    lookAheadLegalCount++;
	    double oldWL = hpwl;

	    double scale = 0.85;    // hpwl driven
	    if( m_targetUtil < 1.0 && m_targetUtil > 0 )
	    	scale = 0.9;        // with density constraint

	    m_pDB->RemoveFixedBlockSite(); // kaie

	    double legalStart = seconds();

	    CTetrisLegal* legal = new CTetrisLegal( *m_pDB );
	    bool bLegal = legal->Solve( m_targetUtil, false, false, scale );
	   
	    double legalTime = seconds() - legalStart;
	    totalLegalTime += legalTime;

	    if( param.bShow )
		printf( "[LAL] %d trial.  CPU Time = %.2f\n", lookAheadLegalCount, legalTime );

	    if( bLegal )
	    {
		m_pDB->Align();	// 2006-04-02

		double WL = m_pDB->GetHPWLdensity( m_targetUtil );

		if( param.bShow )
		{
		    m_pDB->ShowDensityInfo();
		    printf( "[LAL] HPWL= %.0f   dHPWL= %.0f (%.2f%%)\n", m_pDB->GetHPWLp2p(), WL, (WL-oldWL)/oldWL*100 );
		}

		if( WL < bestLegalWL )
		{
		    // record the best legal solution
		    LALnoGoodCount = 0;
		    if( param.bShow )
			printf( "[LAL] SAVE BEST! \n" );

		    bestLegalWL = WL;
		    hasBestLegalSol = true;
		    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
		    {
			xBest[2*i] = m_pDB->m_modules[i].m_cx;
			xBest[2*i+1] = m_pDB->m_modules[i].m_cy;
		    } 

		    // Save the GP file (donnie) 2006-09-06
		    Parallel( UpdateBlockPositionThread, m_pDB->m_modules.size() );

		    //m_pDB->m_modules_bak = m_pDB->m_modules; 
		    bestGPresult = m_pDB->m_modules;

		    if(m_macroRotate) // (kaie) macro rotation
		    {
			for(int i = 0; i < (int)m_pDB->m_modules.size(); i++)
			    bestOrient[i] = m_pDB->m_modules[i].m_orient;
		    }

		    x = xBest;
		}
		else
		{
		    // For WL minimization.
		    if( (WL-oldWL)/oldWL < 0.075 )
		    {
			if( param.bShow )
			    printf( "[LAL] Stop. Good enough\n" ); 
			break;
		    }
		    LALnoGoodCount++;
		    if( LALnoGoodCount >= maxNoGoodCount )
		    {
			if( param.bShow )
			    printf( "[LAL] Stop. Too many times\n" );
			break;
		    }
		}
	    }
	    delete legal; // kaie
	    legal = NULL; // kaie
	}

	if( param.bPrototyping 
		//&& startDecreasing 
		&& totalOverDen < target_density 
		&& ite >= 0 )
	{
	    printf( "Meet constraint! (prototyping)\n" );
	    break;
	}

	if( /*startDecreasing && */totalOverDen < target_density )
	{
	    printf( "Meet constraint!\n" );
	    break;
	}

	/*if( hasBestLegalSol )
	{
	    x = xBest;
	    //m_pDB->m_modules = bestGPresult;
	    //bUpdateWeight = false;
	}*/

	if( bUpdateWeight )
	    UpdateObjWeights();

	lastTotalOverDen = totalOverDen;
	lastMaxDen = maxDen;

    }// outer loop

    // 2006-03-06 (donnie)
    if( hasBestLegalSol )
    {
	m_pDB->m_modules_bak = bestGPresult;
	x = xBest;
	if(m_macroRotate) // (kaie) macro rotation
	{
	    for(unsigned int i = 0; i < m_pDB->m_modules.size(); i++)
	    	m_pDB->SetModuleOrientation(i, bestOrient[i]);
	}
	*isLegal = true;
    }

    if( !m_topLevel /*&& gArg.CheckExist("postplace")*/ /*m_prototype && currentLevel == 1*/ )
    {
	bool bFast_bak = bFast;
	bFast = false;

	Optimization();

	bFast = bFast_bak;
    }
    /*char filename[256];
    sprintf(filename, "test%d.plt", currentLevel);
    m_pDB->OutputGnuplotFigureWithZoom( filename, false, false, false, false, false);*/

    Parallel( UpdateBlockPositionThread, m_pDB->m_modules.size() );
    
    if( lookAheadLegalCount > 0 && param.bShow )
    {
	printf( "[LAL] Total Count: %d\n", lookAheadLegalCount );
	printf( "[LAL] Total CPU: %.2f\n", totalLegalTime );
    }

    if( param.bShow )
    {
	static int allTotalIte = 0;
	allTotalIte += totalIte;
	m_pDB->ShowDensityInfo();
	printf( "\nLevel Ite %d   Total Ite %d\n", totalIte, allTotalIte );
    }

    return true;
}

void MyNLP::Optimization()
{
    struct inner {
	static void evalfunc(int N, double* x, double *prev_x, double* f, double* g)
	{
	    int n = mynlp->m_pDB->m_modules.size()*2;
	    mynlp->Parallel( BoundXThread, n);
	    if( param.bUseSLSE )
	    {
		mynlp->Parallel( UpdateMaxMinPinForEachNetThread, mynlp->m_pDB->m_nets.size() );
		mynlp->Parallel( UpdateExpValueForEachNetThread, mynlp->m_pDB->m_nets.size() );
	    }else
	    {
		mynlp->Parallel( UpdateExpValueForEachCellThread, mynlp->m_pDB->m_modules.size() );
		mynlp->Parallel( UpdateExpValueForEachPinThread, mynlp->m_pDB->m_pins.size() );
	    }
	    mynlp->Parallel( UpdateNetsSumExpThread, mynlp->m_pDB->m_nets.size() );
	    mynlp->Parallel( ComputeNewPotentialGridThread, mynlp->m_pDB->m_modules.size() );
	    mynlp->UpdatePotentialGrid();
	    *f = mynlp->eval_f();
	    mynlp->Parallel( eval_grad_f_thread, mynlp->m_pDB->m_modules.size());
	    //WeightedForce( n, grad_f, weightF );
	    if( !mynlp->AdjustForce( n, mynlp->x, mynlp->grad_f ) )
	    {
		printf("AdjustForce, NaN or Inf\n");
		return;   // NaN or Inf
	    }
	    if( mynlp->m_macroRotate )
	    {
		//mynlp->WeightedForce( mynlp->m_pDB->m_modules.size(), mynlp->grad_rotate, mynlp->grad_potentialR, mynlp-     >weightF );
		if( !mynlp->AdjustForceR( mynlp->m_pDB->m_modules.size(), mynlp->grad_rotate, mynlp->grad_potentialR ) )
		{
		    printf("AdjustForceR, NaN or Inf\n");
		    return; // NaN or Inf
		}
	    }
	    vector<double> &grad_f = mynlp->grad_f;
	    for(unsigned int i = 0; i < grad_f.size(); i++)
	    {
		g[i] = grad_f[i];
	    }
    	}

	static void newiteration(int iter, int call_iter, double *x, double* f, double *g,  double* gnorm)
	{
	    //mynlp->Parallel( BoundXThread, mynlp->m_pDB->m_modules.size()*2 );
	    //std::cout << iter <<": " << call_iter <<" " << *f <<" " << *gnorm  << std::endl;
	    std::cout << ".";
	    //printf("."); fflush(stdout);
	}
	
	static void Optimize_by_HLBFGS(int N, double *init_x, int num_iter, int M, int T)
	{
	    double parameter[20];
	    int info[20];
	    INIT_HLBFGS(parameter, info);
	    info[3] = 1;
	    info[4] = num_iter;
	    info[6] = T;
	    info[7] = 0;
	    info[10] = 1;
	    info[11] = 1;
	    parameter[5] = 0.001;
	    HLBFGS(N, M, init_x, evalfunc, 0, HLBFGS_UPDATE_Hessian, newiteration, parameter, info);
 	}
    };

    int n = this->m_pDB->m_modules.size() * 2;

    // db to x
    vector<Module> &modules = m_pDB->m_modules;
    for(size_t i = 0; i < modules.size(); i++)
    {
	//if(modules[i].m_isFixed) continue;
	x[i*2] = modules[i].m_cx;
	x[i*2+1] = modules[i].m_cy;
    }

    // gradient evalutaion for inital objective weights
    if( param.bUseSLSE )
    {
	this->Parallel( UpdateMaxMinPinForEachNetThread, this->m_pDB->m_nets.size() );
	this->Parallel( UpdateExpValueForEachNetThread, this->m_pDB->m_nets.size() );
    }else
    {
	this->Parallel( UpdateExpValueForEachCellThread, this->m_pDB->m_modules.size() );
	this->Parallel( UpdateExpValueForEachPinThread, this->m_pDB->m_pins.size() );
    }
    this->Parallel( UpdateNetsSumExpThread, this->m_pDB->m_nets.size() );
    this->Parallel( ComputeNewPotentialGridThread, this->m_pDB->m_modules.size() );
    this->UpdatePotentialGrid();
    this->eval_f();
    this->Parallel( eval_grad_f_thread, this->m_pDB->m_modules.size());
    //WeightedForce( n, grad_f, weightF );
    if( !this->AdjustForce( n, this->x, this->grad_f ) )
    {
	printf("AdjustForce, NaN or Inf\n");
	//return false;   // NaN or Inf
    }
    if( m_macroRotate )
    {
	//WeightedForce( m_pDB->m_modules.size(), grad_rotate, grad_potentialR, weightF );
	if( !this->AdjustForceR( m_pDB->m_modules.size(), grad_rotate, grad_potentialR))
	{
	    printf("AdjustForceR, NaN or Inf\n");
	    //return false; // NaN or Inf
	}
    }

    ///////////////////////////////////////////////////////////
    // optimize using HLBFGS
    ///////////////////////////////////////////////////////////

    mynlp = this;

    //printf("run...\n"); fflush(stdout);

    //double lastTotalOverPotential = GetTotalOverPotential();
    //double lastMaxDen = GetMaxDensity();

    //double weightDensity_bak = _weightDensity;

    // set objective weights
    double totalPotentialGradient = 0;
    double totalWireGradient = 0;
    for(int i = 0; i < n; i++) {
	totalPotentialGradient += fabs( grad_potential[i] );
	totalWireGradient += fabs(grad_wire[i]);
    }
    double weightWire_bak = _weightWire;
    double weightDensity_bak = _weightDensity;

    this->_weightWire = 1;
    this->_weightDensity = totalWireGradient / totalPotentialGradient;

    //for(unsigned int i = 0; i < 5; i++)
    //int ite = 0;
    //while(true)
    {
	// db to x
	/*for(size_t i = 0; i < modules.size(); i++)
	{
	    if(modules[i].m_isFixed) continue;
	    x[i*2] = modules[i].m_cx;
	    x[i*2+1] = modules[i].m_cy;
	}*/

	inner::Optimize_by_HLBFGS(n, &x[0], 50, 15, 0);

	//Parallel( BoundXThread, m_pDB->m_modules.size()*2 );

    	// x to db
    	for(unsigned int i = 0; i < m_pDB->m_modules.size(); i++)
    	{
	    if(m_pDB->m_modules[i].m_isFixed == false)
	    {
	    	m_pDB->MoveModuleCenter(i, x[i*2], x[i*2+1]);
	    }
    	}

	/*double obj_value;
	eval_f( n, x, _expX, true, obj_value );
	double totalOverPotential = GetTotalOverPotential();
	double maxDen = GetMaxDensity();

	bool spreadEnough = totalOverPotential < 1.5;
	bool increaseOverPotential = totalOverPotential > lastTotalOverPotential;
	bool increaseMaxDen = maxDen > lastMaxDen;
	bool notEfficientOptimize = 0.5 * density * _weightDensity / obj_value * 100.0 > 95;

	if(  notEfficientOptimize )
	{
	    printf( "Failed to further optimize" );
	    break;
	}

	if( increaseOverPotential && increaseMaxDen && spreadEnough )
	{
	    printf( "Cannot further reduce over potential!\n" ); // skip LAL
	    break;
	}

	lastTotalOverPotential = totalOverPotential;
	lastMaxDen = maxDen;
	ite++;
	UpdateObjWeights();*/
    }

    _weightWire = weightWire_bak;
    _weightDensity = weightDensity_bak;
}

// static 
void* MyNLP::UpdateGradThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    for( int i=pMsg->index1; i<pMsg->index2; i++ )
    {
	pMsg->pNLP->walk_direction[i] = -pMsg->pNLP->grad_f[i] + pMsg->pNLP->m_beta * pMsg->pNLP->last_walk_direction[i];
    }
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

void* MyNLP::UpdateGradRThread( void* arg ) // (kaie) macro rotation
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    for( int i=pMsg->index1; i<pMsg->index2; i++ )
    {
	pMsg->pNLP->walk_direction_r[i] = -pMsg->pNLP->grad_f_r[i]
				+ pMsg->pNLP->m_beta_r * pMsg->pNLP->last_walk_direction_r[i];
	//assert(!isNaN(pMsg->pNLP->walk_direction_r[i]));
	//assert(pMsg->pNLP->walk_direction_r[i] < DBL_MAX * 0.5);
    }
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static 
void* MyNLP::UpdateXThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    for( int i=pMsg->index1; i<pMsg->index2; i++ )
    {
	//double scale = 1;
	//if(pMsg->pNLP->m_pDB->m_modules[i/2].m_height > pMsg->pNLP->m_pDB->m_rowHeight)
	    //scale = min(pMsg->pNLP->m_pDB->m_totalModuleArea / (double)pMsg->pNLP->m_pDB->m_modules.size() / pMsg->pNLP->m_pDB->m_modules[i/2].m_area, 1.0);

	pMsg->pNLP->x[i] += pMsg->pNLP->walk_direction[i] * pMsg->pNLP->m_stepSize;
	//pMsg->pNLP->x[i] +=
	    //pMsg->pNLP->walk_direction[i] * pMsg->pNLP->m_stepSize * scale;
    }
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

void* MyNLP::UpdateRThread( void* arg ) // (kaie) macro rotation
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    double base = 1.0;
    for( int i=pMsg->index1; i<pMsg->index2; i++ )
    {
	pMsg->pNLP->rotate_degree[i] += pMsg->pNLP->walk_direction_r[i] * pMsg->pNLP->m_stepSizeR;
	//printf("%lf, %lf, %lf\n", pMsg->pNLP->rotate_degree[i], pMsg->pNLP->walk_direction_r[i], pMsg->pNLP->m_stepSizeR);
	//assert(!isNaN(pMsg->pNLP->rotate_degree[i]));
	pMsg->pNLP->rotate_degree[i] = fmod(pMsg->pNLP->rotate_degree[i], base);
	double pi = 3.1415926;
	pMsg->pNLP->rotate_off[ 2*i ] = cos(pMsg->pNLP->rotate_degree[i] * 2 * pi );
	pMsg->pNLP->rotate_off[ 2*i+1 ] = sin(pMsg->pNLP->rotate_degree[i] * 2 * pi );
    }
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static 
void* MyNLP::FindGradL2NormThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    double norm = 0;
    for( int i=pMsg->index1; i<pMsg->index2; i++ )
	norm += pMsg->pNLP->last_grad_f[i] * pMsg->pNLP->last_grad_f[i];
    pMsg->pNLP->m_threadResult[pMsg->threadId] = norm;

    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static 
void* MyNLP::FindGradProductThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    double product = 0;
    for( int i=pMsg->index1; i<pMsg->index2; i++ )
	product += pMsg->pNLP->grad_f[i] * ( pMsg->pNLP->grad_f[i] + pMsg->pNLP->last_grad_f[i] );
    pMsg->pNLP->m_threadResult[pMsg->threadId] = product;

    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

bool MyNLP::FindBeta( const int& n, const vector<double>& grad_f, const vector<double>& last_grad_f, double& beta )
{
    // Polak-Ribiere foumula from APlace journal paper
    // NOTE:
    //   g_{k-1} = -last_grad_f
    //   g_k     = grad_f

    /*long*/ double l2norm = 0;
    /*long*/ double product = 0;

    //const double maxGradF = 1.0;

    // Compute the scaling factor to avoid overflow
    double maxGradF = 0;
    for( int i=0; i<n; i++ )
    {
	//assert( fabs(grad_f[i]) < DBL_MAX * 0.95 );
	//assert( fabs(last_grad_f[i]) < DBL_MAX * 0.95 );
	if( grad_f[i] > maxGradF )
	    maxGradF = grad_f[i];
	if( last_grad_f[i] > maxGradF )
	    maxGradF = last_grad_f[i];
    }
    //assert( maxGradF != 0 );
    if(fabs(maxGradF) < 1.0e-10) // kaie
    {
	beta = 0;
	return true;
    }

#if 1    
    for( int i=0; i<n; i++ )
	l2norm += (last_grad_f[i]/maxGradF) * (last_grad_f[i] / maxGradF);
    for( int i=0; i<n; i++ )
    {
	//product += grad_f[i] * ( grad_f[i] + last_grad_f[i] ) / maxGradF / maxGradF;	// g_k^T ( g_k - g_{k-1} )
	//assert( fabs( (grad_f[i]/maxGradF) * ( (grad_f[i]+last_grad_f[i]) / maxGradF) ) < DBL_MAX * 0.95 );
	product += (grad_f[i]/maxGradF) * ( (grad_f[i] - last_grad_f[i]) / maxGradF) ;	// g_k^T ( g_k - g_{k-1} )
    }
#else
    // Parallelization changes the results
    l2norm = Parallel( FindGradL2NormThread, n );	
    product = Parallel( FindGradProductThread, n );
#endif

    if( product == 0 )
    {
	printf( "product == 0 \n" );
    }

    if( l2norm == 0 )
	return false;  // Failed. Some problem (overflow) during the computation. 

    beta = (double) (product / l2norm);
    if( beta > DBL_MAX * 0.9 )
    {
	// Beta is too large. Overflow may be occured.
	printf( "%g = %g / %g\n", beta, (double)product, (double)l2norm );
	return false;
    }
    return true;
}

void MyNLP::BoundX( const int& n, vector<double>& x, vector<double>& x_l, vector<double>& x_h, const int& i )
{
    if( x[i] < x_l[i] )      x[i] = x_l[i];
    else if( x[i] > x_h[i] )	x[i] = x_h[i];
}

void* MyNLP::BoundXThread( void* arg )   
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    BoundX( (int)pMsg->pX->size(), *pMsg->pX, pMsg->pNLP->x_l, pMsg->pNLP->x_u, pMsg->index1, pMsg->index2 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static
void MyNLP::BoundX( const int& n, vector<double>& x, vector<double>& x_l, vector<double>& x_h, int index1, int index2 )
{
    if( index2 > n )
	index2 = n;
    for( int i = index1; i < index2; i++ )
    {
	if( x[i] < x_l[i] )             x[i] = x_l[i];
	else if( x[i] > x_h[i] )	x[i] = x_h[i];
    } 
}

bool MyNLP::AdjustForce( const int& n, const vector<double>& x, vector<double>& f )
{
    if( param.bAdjustForce == false )
	return true;//no adjust

    double totalGrad = 0;
    int size = n/2;

    static int outCount = 0;
    outCount++; 

    if( gArg.CheckExist( "plotForce" ) )
    {
	char filename[255];
	sprintf( filename, "force%d.dat", outCount );
	DataHandler data1;
	for( int i=0; i<size; i++ )
	    data1.Insert( f[2*i] * f[2*i] + f[2*i+1] * f[2*i+1] );
	data1.Sort();
	data1.OutputFile( filename );
    }

    for( int i=0; i<size; i++ )
	totalGrad += f[2*i] * f[2*i] + f[2*i+1] * f[2*i+1];
    double avgGrad = sqrt( totalGrad / size );

    //if( isNaN( totalGrad ) )
    //	return false;

    // Do truncation
    double expMaxGrad = avgGrad * param.truncationFactor;	// x + y
    double expMaxGradSquare = expMaxGrad * expMaxGrad;
    for( int i=0; i<size; i++ )
    {
	double valueSquare = ( f[2*i] * f[2*i] + f[2*i+1] * f[2*i+1] );
	if( valueSquare > expMaxGradSquare )
	{
	    double value = sqrt( valueSquare );
	    f[2*i]   = f[2*i]   * expMaxGrad / value;
	    f[2*i+1] = f[2*i+1] * expMaxGrad / value;
	}
    }

    if( gArg.CheckExist( "plotForce" ) )
    {
	char filename[255];
	sprintf( filename, "force%db.dat", outCount );
	DataHandler data1;
	for( int i=0; i<size; i++ )
	    data1.Insert( f[2*i] * f[2*i] + f[2*i+1] * f[2*i+1] );
	data1.Sort();
	data1.OutputFile( filename );
    }

    return true;
}


bool MyNLP::AdjustForce( const int& n, const vector<double>& x, vector<double> grad_wl, vector<double> grad_potential )
{
    double totalGrad = 0;
    int size = n/2;
    for( int i=0; i<size; i++ )
    {
	double value = 
	    (grad_wl[2*i] + grad_potential[2*i]) * (grad_wl[2*i] + grad_potential[2*i]) + 
	    (grad_wl[2*i+1] + grad_potential[2*i+1]) * (grad_wl[2*i+1] + grad_potential[2*i+1]); 
	totalGrad += value;
    }

    if( isNaN( totalGrad ) )
	return false;
    //assert( !isNaN( totalGrad ) );	// it is checked in GoSolve()

    double avgGrad = sqrt( totalGrad / size );

    // Do truncation
    double expMaxGrad = avgGrad * param.truncationFactor;	// x + y
    double expMaxGradSquare = expMaxGrad * expMaxGrad;
    for( int i=0; i<size; i++ )
    {
	double valueSquare = 
	    (grad_wl[2*i] + grad_potential[2*i]) * (grad_wl[2*i] + grad_potential[2*i]) + 
	    (grad_wl[2*i+1] + grad_potential[2*i+1]) * (grad_wl[2*i+1] + grad_potential[2*i+1]); 
	if( valueSquare == 0 )
	{
	    // avoid value = 0 let to inf
	    grad_wl[2*i] = grad_wl[2*i+1] = 0;
	    grad_potential[2*i] = grad_potential[2*i+1] = 0;
	}
	else
	{
	    if( valueSquare > expMaxGradSquare )
	    {
		double value = sqrt( valueSquare );
		grad_wl[2*i]   = grad_wl[2*i]   * expMaxGrad / value;
		grad_wl[2*i+1] = grad_wl[2*i+1] * expMaxGrad / value;
		grad_potential[2*i]   = grad_potential[2*i]   * expMaxGrad / value;
		grad_potential[2*i+1] = grad_potential[2*i+1] * expMaxGrad / value;
	    }
	}

    }
    return true;
}

// (kaie) macro rotation
bool MyNLP::WeightedForce( const int& n, vector<double>& f1, vector<double>& f2, vector<double> weight)
{
    int size = n;
    double totalF = 0;
    double totalWeightF = 0;
    int div = 1;
    if((int)weight.size() > size) div = 2;

    for( int i=0; i<size; i++ )
    {
	double value = 
	    (f1[i] + f2[i]) * (f1[i] + f2[i]); 
	totalF += value;
	totalWeightF += weight[i/div] * weight[i/div] * value;
    }

    double avgF = sqrt( totalF / size );
    double sqrWeightF = sqrt( totalWeightF );
    double nFactor = avgF / sqrWeightF * sqrt(size);
    
    for( int i = 0; i < size; i++ )
    {
	f1[i] = f1[i] * weight[i/div] * nFactor;
	f2[i] = f2[i] * weight[i/div] * nFactor;
    }

    return true;
}

bool MyNLP::WeightedForce( const int& n, vector<double>& f, vector<double> weight)
{
    int size = n / 2;
    double totalF = 0;
    double totalWeightF = 0;
    for( int i=0; i<size; i++ )
    {
	double value = f[2*i] * f[2*i] + f[2*i+1] * f[2*i+1]; 
	totalF += value;
	totalWeightF += weight[i] * weight[i] * value;
    }

    double avgF = sqrt( totalF / size );
    double sqrWeightF = sqrt( totalWeightF );
    double nFactor = avgF / sqrWeightF * sqrt(size);
    
    for( int i = 0; i < size; i++ )
    {
	f[2*i] = f[2*i] * weight[i] * nFactor;
	f[2*i+1] = f[2*i+1] *weight[i] * nFactor;
    }

    return true;
}

bool MyNLP::AdjustForceR( const int& n, vector<double> grad_rotate, vector<double> grad_potentialR )
{
    double totalGradR = 0;
    int size = n;
    for( int i=0; i<size; i++ )
    {
	double value = 
	    (grad_rotate[i] + grad_potentialR[i]) * (grad_rotate[i] + grad_potentialR[i]); 
	totalGradR += value;
    }

    if( isNaN( totalGradR ) )
    {
	printf("%lf\n", totalGradR);
	return false;
    }
    //assert( !isNaN( totalGradR ) );	// it is checked in GoSolve()

    double avgGradR = sqrt( totalGradR / size );

    // Do truncation
    double expMaxGradR = avgGradR * param.truncationFactor / 2.0;	// x + y
    //expMaxGradR = min(sqrt(DBL_MAX*0.5), expMaxGradR);
    double expMaxGradRSquare = expMaxGradR * expMaxGradR;
    for( int i=0; i<size; i++ )
    {
	double valueSquareR = 
	    (grad_rotate[i] + grad_potentialR[i]) * (grad_rotate[i] + grad_potentialR[i]); 
	if( valueSquareR == 0 )
	{
	    // avoid value = 0 let to inf
	    grad_rotate[i] = 0;
	    grad_potentialR[i] = 0;
	}
	else
	{
	    if( valueSquareR > expMaxGradRSquare )
	    {
		double valueR = sqrt( valueSquareR );
		grad_rotate[i]   = grad_rotate[i]   * expMaxGradR / valueR;
		grad_potentialR[i]   = grad_potentialR[i]   * expMaxGradR / valueR;
	    }
	}
	//assert(!isNaN(grad_rotate[i]));
	//assert(!isNaN(grad_potentialR[i]));
	//assert(grad_rotate[i] < DBL_MAX * 0.5);
	//assert(grad_potentialR[i] < DBL_MAX * 0.5);
    }
    return true;
}

bool MyNLP::AdjustForceR( const int& n, vector<double> grad_rotate, vector<double> grad_potentialR, vector<double> weightR )
{
    double totalGradR = 0, totalWeightGradR = 0;
    int size = n;
    for( int i = 0; i < size; i++ )
    {
	double value = (grad_rotate[i] + grad_potentialR[i]) * (grad_rotate[i] + grad_potentialR[i]); 
	totalGradR += value;
	totalWeightGradR += weightR[i] * weightR[i] * value;
    }

    if( isNaN( totalGradR ) )
    {
	printf("%lf\n", totalGradR);
	return false;
    }
    //assert( !isNaN( totalGradR ) );	// it is checked in GoSolve()

    if( isNaN( totalWeightGradR ) )
    {
	printf("%lf\n", totalWeightGradR );
	return false;
    }
    //assert( !isNaN( totalWeightGradR ) );

    double avgGradR = sqrt( totalGradR / size );
    double sqrWeightGradR = sqrt( totalWeightGradR );
    double nFactor = avgGradR / sqrWeightGradR * sqrt(size);
    
    for( int i = 0; i < size; i++ )
    {
	grad_rotate[i] = grad_rotate[i] * weightR[i] * nFactor;
	grad_potentialR[i] = grad_potentialR[i] * weightR[i] * nFactor;
    }

    // Do truncation
    double expMaxGradR = avgGradR * param.truncationFactor / 2.0;	// x + y
    //expMaxGradR = min(sqrt(DBL_MAX*0.5), expMaxGradR);
    double expMaxGradRSquare = expMaxGradR * expMaxGradR;
    for( int i = 0; i < size; i++ )
    {
	double valueSquareR = 
	    (grad_rotate[i] + grad_potentialR[i]) * (grad_rotate[i] + grad_potentialR[i]); 
	if( valueSquareR == 0 )
	{
	    // avoid value = 0 let to inf
	    grad_rotate[i] = 0;
	    grad_potentialR[i] = 0;
	}
	else
	{
	    if( valueSquareR > expMaxGradRSquare )
	    {
		double valueR = sqrt( valueSquareR );
		grad_rotate[i]   = grad_rotate[i]   * expMaxGradR / valueR;
		grad_potentialR[i]   = grad_potentialR[i]   * expMaxGradR / valueR;
	    }
	}
	//assert(!isNaN(grad_rotate[i]));
	//assert(!isNaN(grad_potentialR[i]));
	//assert(grad_rotate[i] < DBL_MAX * 0.5);
	//assert(grad_potentialR[i] < DBL_MAX * 0.5);
    }
    return true;
}
// @(kaie) macro rotation


void MyNLP::LineSearch( const int& n, /*const*/ vector<double>& x, vector<double>& f, double& stepSize )
{
    int size = n / 2;
    /*long*/ double totalGrad = 0;
    double avgGrad;

    for( int i=0; i<n; i++ )
	totalGrad += f[i] * f[i];
    avgGrad = sqrt( totalGrad / size );
    stepSize = m_potentialGridWidth / avgGrad * m_currentStep;	
    return;
}

void MyNLP::LineSearchR( const int& n, vector<double>& f_r, double& stepSizeR )
{
    int size = n;
    double totalGradR = 0;
    double avgGradR;

    for(int i = 0; i<n; i++)
	totalGradR += f_r[i] * f_r[i];
    avgGradR = sqrt( totalGradR / size );
    double m_rotateStep = 1.0;
    gArg.GetDouble("rstep", &m_rotateStep);
    m_rotateStep *= m_stepSize;
    //printf("rstep = %lf\n", m_rotateStep);
    if(avgGradR == 0)
	stepSizeR = 0;
    else
        stepSizeR = 1.0 / avgGradR * m_rotateStep;
    //assert(!isNaN(stepSizeR));
    //assert(stepSizeR < DBL_MAX * 0.5);
    return;
}

bool MyNLP::get_nlp_info(int& n, int& m, int& nnz_jac_g, 
	int& nnz_h_lag/*, IndexStyleEnum& index_style*/)
{
    n = m_pDB->m_modules.size() * 2;
    m = 0;	    // no constraint
    nnz_jac_g = 0;  // 0 nonzeros in the jacobian since no constraint
    return true;
}

bool MyNLP::get_bounds_info(int n, vector<double>& x_l, vector<double>& x_u )
{
    //assert(n == (int)m_pDB->m_modules.size() * 2);
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	if( m_pDB->m_modules[i].m_isFixed )
	{
	    x_l[2*i] = m_pDB->m_modules[i].m_cx;
	    x_u[2*i] = m_pDB->m_modules[i].m_cx;
	    x_l[2*i+1] = m_pDB->m_modules[i].m_cy;
	    x_u[2*i+1] = m_pDB->m_modules[i].m_cy;
	}
	else
	{
	    x_l[2*i]   = m_pDB->m_coreRgn.left   + m_pDB->m_modules[i].m_width  * 0.5;  
	    x_u[2*i]   = m_pDB->m_coreRgn.right  - m_pDB->m_modules[i].m_width  * 0.5;  
	    x_l[2*i+1] = m_pDB->m_coreRgn.bottom + m_pDB->m_modules[i].m_height * 0.5;
	    x_u[2*i+1] = m_pDB->m_coreRgn.top    - m_pDB->m_modules[i].m_height * 0.5;
	}
    }
    return true;
}

bool MyNLP::get_starting_point( vector<double>& x )
{
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	x[2*i]   = m_pDB->m_modules[i].m_cx;
	x[2*i+1] = m_pDB->m_modules[i].m_cy;
    }
    return true;
}

double MyNLP::Parallel( void* (func)(void* arg), int max1, int max2, int max3 )
{
    int threadNum = param.nThread;

    if( threadNum == 1 )
    {
	m_threadInfo[0].index1  = 0;
	m_threadInfo[0].index2  = max1;
	m_threadInfo[0].index3  = 0;
	m_threadInfo[0].index4  = max2;
	m_threadInfo[0].index5  = 0;
	m_threadInfo[0].index6  = max3;
	m_threadInfo[0].pX      = const_cast< vector<double>* >(&x); 
	m_threadInfo[0].pExpX   = const_cast< vector<double>* >(&_expX);
	m_threadInfo[0].alpha   = _alpha;
	m_threadInfo[0].pNLP    = this;
	m_threadInfo[0].pDB     = m_pDB;
	m_threadInfo[0].pUsePin = &m_usePin;
	m_threadInfo[0].threadId = 0;
	func( (void*)(&m_threadInfo[0]) );
	return m_threadResult[0];
    }
    else
    {
	int jobCount1 = max1 / threadNum + 1;
	int jobCount2 = max2 / threadNum + 1;
	int jobCount3 = max3 / threadNum + 1;
	vector<pthread_t> threads;
	threads.resize( threadNum );
	for( int i=0; i<threadNum; i++ )
	{
	    int start1 = i * jobCount1;
	    int start2 = i * jobCount2;
	    int start3 = i * jobCount3;
	    int end1 = start1 + jobCount1;
	    int end2 = start2 + jobCount2;
	    int end3 = start3 + jobCount3;
	    if( end1 > max1 )	end1 = max1;
	    if( end2 > max2 )	end2 = max2;
	    if( end3 > max3 )	end3 = max3;
	    m_threadInfo[i].index1  = start1;
	    m_threadInfo[i].index2  = end1;
	    m_threadInfo[i].index3  = start2;
	    m_threadInfo[i].index4  = end2;
	    m_threadInfo[i].index5  = start3;
	    m_threadInfo[i].index6  = end3;
	    m_threadInfo[i].pX      = const_cast< vector<double>* >(&x); 
	    m_threadInfo[i].pExpX   = const_cast< vector<double>* >(&_expX);
	    m_threadInfo[i].alpha   = _alpha;
	    m_threadInfo[i].pNLP    = this;
	    m_threadInfo[i].pDB     = m_pDB;
	    m_threadInfo[i].pUsePin = &m_usePin;
	    m_threadInfo[i].threadId = i;
	    pthread_create( &threads[i], NULL, func, (void*)(&m_threadInfo[i]) );
	}

	void* thread_result;
	double res = 0;
	for( int i=0; i<threadNum; i++ )
	{
	    pthread_join( threads[i], &thread_result );
	    res += m_threadResult[i];
	}
	return res;
    }
}

// (kaie) 2009-09-23 Stable-LSE
// static
void* MyNLP::UpdateMaxMinPinForEachNetThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    UpdateMaxMinPinForEachNet( *pMsg->pX, pMsg->pNLP, pMsg->pUsePin, pMsg->index1, pMsg->index2 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

void MyNLP::UpdateMaxMinPinForEachNet( const vector<double>& x, MyNLP* pNLP, const vector<bool>* pUsePin, int index1, int index2 )
{
    if( index2 > (int)pNLP->m_pDB->m_nets.size() )
	index2 = (int)pNLP->m_pDB->m_nets.size();
    for( int n=index1; n<index2; n++ )
    {
	if( pNLP->m_pDB->m_nets[n].size() == 0 )
		continue;
	    
	int pid = pNLP->m_pDB->m_nets[n][0];
	int moduleId = pNLP->m_pDB->m_pins[pid].moduleId;
	
	double cx = x[ 2*moduleId ];
	double cy = x[ 2*moduleId+1 ];
	
	if( (*pUsePin)[moduleId] )
	{
            if( pNLP->m_macroRotate ) // (kaie) macro rotation
            {
                //double pi = 3.1415926;
		cx += pNLP->m_pDB->m_pins[ pid ].xOff * pNLP->rotate_off[ 2*moduleId ]//cos( pNLP->rotate_degree[ moduleId ] * 2 * pi )
		    - pNLP->m_pDB->m_pins[ pid ].yOff * pNLP->rotate_off[ 2*moduleId+1 ];//sin( pNLP->rotate_degree[ moduleId ] * 2 * pi );
		cy += pNLP->m_pDB->m_pins[ pid ].xOff * pNLP->rotate_off[ 2*moduleId+1 ]//sin( pNLP->rotate_degree[ moduleId ] * 2 * pi )
		    + pNLP->m_pDB->m_pins[ pid ].yOff * pNLP->rotate_off[ 2*moduleId ];//cos( pNLP->rotate_degree[ moduleId ] * 2 * pi );
	    }else
	    {
		cx += pNLP->m_pDB->m_pins[ pid ].xOff;
		cy += pNLP->m_pDB->m_pins[ pid ].yOff;
	    }
	    //cx += pNLP->m_pDB->m_pins[pid].xOff;
	    //cy += pNLP->m_pDB->m_pins[pid].yOff;
	}
	//pNLP->pin_position[ 2*pid ] = cx;
	//pNLP->pin_position[ 2*pid+1 ] = cy;

	double maxX = cx;
	double minX = cx;
	double maxY = cy;
	double minY = cy;
	for( int j = 1; j < (int)pNLP->m_pDB->m_nets[n].size(); j++ )
	{
	    pid = pNLP->m_pDB->m_nets[n][j];
	    moduleId = pNLP->m_pDB->m_pins[pid].moduleId;
	    
	    cx = x[ 2*moduleId ];
	    cy = x[ 2*moduleId+1 ];

	    if( (*pUsePin)[moduleId] )
	    {
            	if( pNLP->m_macroRotate ) // (kaie) macro rotation
            	{
                    //double pi = 3.1415926;
		    cx += pNLP->m_pDB->m_pins[ pid ].xOff * pNLP->rotate_off[ 2*moduleId ]//cos( pNLP->rotate_degree[ moduleId ] * 2 * pi )
		    	- pNLP->m_pDB->m_pins[ pid ].yOff * pNLP->rotate_off[ 2*moduleId+1 ];//sin( pNLP->rotate_degree[ moduleId ] * 2 * pi );
		    cy += pNLP->m_pDB->m_pins[ pid ].xOff * pNLP->rotate_off[ 2*moduleId+1 ]//sin( pNLP->rotate_degree[ moduleId ] * 2 * pi )
		    	+ pNLP->m_pDB->m_pins[ pid ].yOff * pNLP->rotate_off[ 2*moduleId ];//cos( pNLP->rotate_degree[ moduleId ] * 2 * pi );
	    	}else
	    	{
		    cx += pNLP->m_pDB->m_pins[ pid ].xOff;
		    cy += pNLP->m_pDB->m_pins[ pid ].yOff;
	    	}
		//cx += pNLP->m_pDB->m_pins[pid].xOff;
		//cy += pNLP->m_pDB->m_pins[pid].yOff;
	    }
	    //pNLP->pin_position[ 2*pid ] = cx;
	    //pNLP->pin_position[ 2*pid+1 ] = cy;

	    maxX = max( maxX, cx );
	    minX = min( minX, cx );
	    maxY = max( maxY, cy );
	    minY = min( minY, cy );
	}
	pNLP->m_nets_max_xi[n] = maxX;
	pNLP->m_nets_min_xi[n] = minX;
	pNLP->m_nets_max_yi[n] = maxY;
	pNLP->m_nets_min_yi[n] = minY;
    }
}

void* MyNLP::UpdateExpValueForEachNetThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    UpdateExpValueForEachNetStable( *pMsg->pX, pMsg->alpha, pMsg->pNLP, pMsg->pUsePin, pMsg->index1, pMsg->index2 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

void MyNLP::UpdateExpValueForEachNetStable( const vector<double>& x, const double& inAlpha, MyNLP* pNLP, const vector<bool>* pUsePin, int index1, int index2 )
{
    if( index2 > (int)pNLP->m_pDB->m_nets.size() );
	index2 = (int)pNLP->m_pDB->m_nets.size();
    for( int n = index1; n < index2; n++ ) // for each net
    {
	//pNLP->expXnet[n].resize( 4 * pNLP->m_pDB->m_nets[n].size(), 0 );

	vector<int>::const_iterator begin = pNLP->m_pDB->m_nets[n].begin();
	vector<int>::const_iterator end = pNLP->m_pDB->m_nets[n].end();
	vector<int>::const_iterator ite;
	int pinId, pinIndex;
	int blockId;
	for( ite = begin, pinIndex = 0; ite != end; ++ite, pinIndex++) // for each pin
	{
	    pinId = *ite;
	    blockId = pNLP->m_pDB->m_pins[ pinId ].moduleId;

	    if( blockId >= (int)pNLP->m_pDB->m_modules.size() )
		continue;

	    double xx /*= pNLP->pin_position[ 2*pinId ];*/ = x[ 2*blockId ];
	    double yy /*= pNLP->pin_position[ 2*pinId+1 ];*/ = x[ 2*blockId+1 ];
	    double xx1, xx2, yy1, yy2;

	    if( (*pUsePin)[blockId] ) // macro or self pin
	    {
		if( pNLP->m_macroRotate ) // (kaie) macro rotation
		{
	    	    //double pi = 3.1415926;
	    	    //printf("%lf, %lf\n", cos( pNLP->rotate_degree[ blockId ] * 2 * pi ), sin( pNLP->rotate_degree[ blockId ] * 2 * pi ));
	    	    xx += pNLP->m_pDB->m_pins[ pinId ].xOff * pNLP->rotate_off[ 2*blockId ] //cos( pNLP->rotate_degree[ blockId ] * 2 * pi )
			- pNLP->m_pDB->m_pins[ pinId ].yOff * pNLP->rotate_off[ 2*blockId+1 ]; //sin( pNLP->rotate_degree[ blockId ] * 2 * pi );
	    	    yy += pNLP->m_pDB->m_pins[ pinId ].xOff * pNLP->rotate_off[ 2*blockId+1 ] //sin( pNLP->rotate_degree[ blockId ] * 2 * pi )
			+ pNLP->m_pDB->m_pins[ pinId ].yOff * pNLP->rotate_off[ 2*blockId ]; //cos( pNLP->rotate_degree[ blockId ] * 2 * pi );
		}else
		{
	    	    xx += pNLP->m_pDB->m_pins[ pinId ].xOff;
	    	    yy += pNLP->m_pDB->m_pins[ pinId ].yOff;
		}
		//xx += pNLP->m_pDB->m_pins[ pinId ].xOff;
		//yy += pNLP->m_pDB->m_pins[ pinId ].yOff;
	    }

	    xx1 = xx - pNLP->m_nets_max_xi[n];
	    xx2 = pNLP->m_nets_min_xi[n] - xx;
	    yy1 = yy - pNLP->m_nets_max_yi[n];
	    yy2 = pNLP->m_nets_min_yi[n] - yy;

	    pNLP->expXnet[n][ 4*pinIndex ]   = exp( xx1 / inAlpha ); // xi
	    pNLP->expXnet[n][ 4*pinIndex+1 ] = exp( xx2 / inAlpha ); // -xi
	    pNLP->expXnet[n][ 4*pinIndex+2 ] = exp( yy1 / inAlpha ); // yi
	    pNLP->expXnet[n][ 4*pinIndex+3 ] = exp( yy2 / inAlpha ); // -yi
	    /*printf("Net%d: %.2f, %.2f, %.2f, %.2f -- %.2f, %.2f, %.2f, %.2f\n", n,
		xx1, xx2, yy1, yy2,
		pNLP->expXnet[n][ 4*pinIndex ],
		pNLP->expXnet[n][ 4*pinIndex+1 ],
		pNLP->expXnet[n][ 4*pinIndex+2 ],
		pNLP->expXnet[n][ 4*pinIndex+3 ]);*/
	}
    }
}
// @(kaie) 2009-09-23

// static
void* MyNLP::UpdateExpValueForCellAndPinThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    UpdateExpValueForEachCell( pMsg->index2, *pMsg->pX, *pMsg->pExpX, pMsg->alpha, pMsg->index1, pMsg->pNLP );
    UpdateExpValueForEachPin(  pMsg->index4, *pMsg->pX, *pMsg->pExpX, pMsg->alpha, pMsg->pNLP, pMsg->pUsePin, pMsg->index3 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static
void* MyNLP::UpdateExpValueForEachCellThread( void* arg )   
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    UpdateExpValueForEachCell( pMsg->index2, *pMsg->pX, *pMsg->pExpX, pMsg->alpha, pMsg->index1, pMsg->pNLP );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static
void MyNLP::UpdateExpValueForEachCell( const int& index2, const vector<double>& x, 
	vector<double>& expX, const double& inAlpha, 
	const int& index1, MyNLP* pNLP ) 
{

    if( param.bUseLSE )
    {
	for( int i = index1; i < index2; i++ )
	{
	    expX[2*i]   = exp( x[2*i] / inAlpha );
	    expX[2*i+1] = exp( x[2*i+1] / inAlpha );
	}
    }
    else
    {
	// Lp-norm
	for( int i = index1; i < index2; i++ )
	{
	    expX[2*i]   = pow( x[2*i]   * pNLP->m_posScale, inAlpha ); 
	    expX[2*i+1] = pow( x[2*i+1] * pNLP->m_posScale, inAlpha ); 
	}
    }
}

/*static*/ 
void* MyNLP::UpdateExpValueForEachPinThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    UpdateExpValueForEachPin( pMsg->index2, *pMsg->pX, pMsg->pNLP->_expPins, pMsg->alpha, pMsg->pNLP, pMsg->pUsePin, pMsg->index1 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static
void MyNLP::UpdateExpValueForEachPin( const int& index2, const vector<double>& x, 
	vector<double>& expPins, const double& inAlpha,
	MyNLP* pNLP, const vector<bool>* pUsePin, const int& index1 )
{
    for( int pinId = index1; pinId < index2; pinId++ )
    {
	int blockId = pNLP->m_pDB->m_pins[pinId].moduleId;

	// TODO: doubleing-pin! (PlaceDB Bug!)
	if( blockId >= (int)pNLP->m_pDB->m_modules.size() )
	    continue;

	// 2006-02-20
	if( (*pUsePin)[blockId] == false )
	    continue;	// save time

	// (kaie) macro rotation
	double xx = x[ 2*blockId ];
	double yy = x[ 2*blockId+1 ];
	if( pNLP->m_macroRotate )
	{
	    double pi = 3.1415926;
	    //printf("%lf, %lf\n", cos( pNLP->rotate_degree[ blockId ] * 2 * pi ), sin( pNLP->rotate_degree[ blockId ] * 2 * pi ));
	    xx += pNLP->m_pDB->m_pins[ pinId ].xOff * cos( pNLP->rotate_degree[ blockId ] * 2 * pi )
		- pNLP->m_pDB->m_pins[ pinId ].yOff * sin( pNLP->rotate_degree[ blockId ] * 2 * pi );
	    yy += pNLP->m_pDB->m_pins[ pinId ].xOff * sin( pNLP->rotate_degree[ blockId ] * 2 * pi )
		+ pNLP->m_pDB->m_pins[ pinId ].yOff * cos( pNLP->rotate_degree[ blockId ] * 2 * pi );
	}else
	{
	    xx += pNLP->m_pDB->m_pins[ pinId ].xOff;
	    yy += pNLP->m_pDB->m_pins[ pinId ].yOff;
	}
	// @(kaie) macro rotation 

	if( param.bUseLSE )
	{
	    expPins[2*pinId]   = exp( xx / inAlpha );
	    expPins[2*pinId+1] = exp( yy / inAlpha );
	}
	else
	{
	    // Lp-norm
	    expPins[2*pinId]   = pow( xx * pNLP->m_posScale, inAlpha );
	    expPins[2*pinId+1] = pow( yy * pNLP->m_posScale, inAlpha );
	    //assert( !isNaN( expPins[2*pinId] ) );
	    //assert( !isNaN( expPins[2*pinId+1] ) );
	    /*if( expPins[2*pinId] == 0 || expPins[2*pinId+1] == 0 )
	      {
	      printf( "%g %g %g\n", xx, yy, pNLP->m_posScale );
	      }*/
	    //assert( expPins[2*pinId] != 0 );
	    //assert( expPins[2*pinId+1] != 0 );
	}
	//printf("expPin[%d]: %f, %f\n", pinId, expPins[2*pinId], expPins[2*pinId+1]);
    }
}

// static
void* MyNLP::UpdateNetsSumExpThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    UpdateNetsSumExp( *pMsg->pX, *pMsg->pExpX, pMsg->alpha, pMsg->pNLP, pMsg->index1, pMsg->index2 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static
void MyNLP::UpdateNetsSumExp( const vector<double>& x, const vector<double>& expX, const double& inAlpha, MyNLP* pNLP, int index1, int index2 )
{
    double sum_exp_xi_over_alpha;
    double sum_exp_inv_xi_over_alpha;
    double sum_exp_yi_over_alpha;
    double sum_exp_inv_yi_over_alpha;
    if( index2 > (int)pNLP->m_pDB->m_nets.size() )
	index2 = (int)pNLP->m_pDB->m_nets.size();
    for( int n=index1; n<index2; n++ )
    {
	if( pNLP->m_pDB->m_nets[n].size() == 0 )
	    continue;

	calc_sum_exp_using_pin(
		pNLP->m_pDB->m_nets[n].begin(), pNLP->m_pDB->m_nets[n].end(), x, expX, pNLP->expXnet[n],
		sum_exp_xi_over_alpha, sum_exp_inv_xi_over_alpha,
		sum_exp_yi_over_alpha, sum_exp_inv_yi_over_alpha,
		pNLP->m_pDB, &pNLP->m_usePin, pNLP->_expPins
		);
	
	pNLP->m_nets_sum_exp_xi_over_alpha[n]     = sum_exp_xi_over_alpha;
	pNLP->m_nets_sum_exp_yi_over_alpha[n]     = sum_exp_yi_over_alpha;
	pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] = sum_exp_inv_xi_over_alpha;
	pNLP->m_nets_sum_exp_inv_yi_over_alpha[n] = sum_exp_inv_yi_over_alpha;


	/*if( gArg.CheckExist("WAE") || gArg.CheckExist("SWAE") ) // (kaie) 2011-05-11 Weighted-Average-Exponential Wirelength Model
	{
	    double weighted_sum_exp_xi_over_alpha;
	    double weighted_sum_exp_inv_xi_over_alpha;
	    double weighted_sum_exp_yi_over_alpha;
	    double weighted_sum_exp_inv_yi_over_alpha;

	    calc_weighted_sum_exp_using_pin(
		pNLP->m_pDB->m_nets[n].begin(), pNLP->m_pDB->m_nets[n].end(), x, expX, pNLP->expXnet[n],
		weighted_sum_exp_xi_over_alpha, weighted_sum_exp_inv_xi_over_alpha,
		weighted_sum_exp_yi_over_alpha, weighted_sum_exp_inv_yi_over_alpha,
		pNLP->m_pDB, &pNLP->m_usePin, pNLP->_expPins);

	    pNLP->m_nets_weighted_sum_exp_xi_over_alpha[n]     = weighted_sum_exp_xi_over_alpha;
	    pNLP->m_nets_weighted_sum_exp_yi_over_alpha[n]     = weighted_sum_exp_yi_over_alpha;
	    pNLP->m_nets_weighted_sum_exp_inv_xi_over_alpha[n] = weighted_sum_exp_inv_xi_over_alpha;
	    pNLP->m_nets_weighted_sum_exp_inv_yi_over_alpha[n] = weighted_sum_exp_inv_yi_over_alpha;
	}*/

    }

    if( param.bUseLSE == false )  // for Lp-norm
    {
	for( int n=index1; n<index2; n++ )
	{
	    pNLP->m_nets_sum_p_x_pos[n]     = pow( pNLP->m_nets_sum_exp_xi_over_alpha[n], 1/pNLP->_alpha-1 );
	    pNLP->m_nets_sum_p_y_pos[n]     = pow( pNLP->m_nets_sum_exp_yi_over_alpha[n], 1/pNLP->_alpha-1 );
	    pNLP->m_nets_sum_p_inv_x_pos[n] = pow( pNLP->m_nets_sum_exp_inv_xi_over_alpha[n], 1/pNLP->_alpha-1 );
	    pNLP->m_nets_sum_p_inv_y_pos[n] = pow( pNLP->m_nets_sum_exp_inv_yi_over_alpha[n], 1/pNLP->_alpha-1 );

	    pNLP->m_nets_sum_p_x_neg[n]     = pow( pNLP->m_nets_sum_exp_xi_over_alpha[n], -1/pNLP->_alpha-1 );
	    pNLP->m_nets_sum_p_y_neg[n]     = pow( pNLP->m_nets_sum_exp_yi_over_alpha[n], -1/pNLP->_alpha-1 );
	    pNLP->m_nets_sum_p_inv_x_neg[n] = pow( pNLP->m_nets_sum_exp_inv_xi_over_alpha[n], -1/pNLP->_alpha-1 );
	    pNLP->m_nets_sum_p_inv_y_neg[n] = pow( pNLP->m_nets_sum_exp_inv_yi_over_alpha[n], -1/pNLP->_alpha-1 );

#if 0
	    assert( !isNaN( pNLP->m_nets_sum_p_x_pos[n] ) );
	    assert( !isNaN( pNLP->m_nets_sum_p_y_pos[n] ) );
	    assert( !isNaN( pNLP->m_nets_sum_p_inv_x_pos[n] ) );
	    assert( !isNaN( pNLP->m_nets_sum_p_inv_y_pos[n] ) );
	    assert( !isNaN( pNLP->m_nets_sum_p_x_neg[n] ) );
	    assert( !isNaN( pNLP->m_nets_sum_p_y_neg[n] ) );
	    assert( !isNaN( pNLP->m_nets_sum_p_inv_x_neg[n] ) );
	    assert( !isNaN( pNLP->m_nets_sum_p_inv_y_neg[n] ) );
#endif
	}
    }
}

// static
void* MyNLP::GetLogSumExpWLThread( void* arg )   
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    pMsg->pNLP->m_threadResult[pMsg->threadId] = GetLogSumExpWL( 
	    *pMsg->pX, *pMsg->pExpX, pMsg->alpha, pMsg->pNLP, pMsg->index1, pMsg->index2 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

//static
double MyNLP::GetLogSumExpWL( const vector<double>& x,	    // unuse
	const vector<double>& expX, // unuse
	const double& alpha,	    // unuse
	MyNLP* pNLP, int index1, int index2 )
{

    if( index2 > (int)pNLP->m_pDB->m_nets.size() ) // check boundary
	index2 = (int)pNLP->m_pDB->m_nets.size();

    double totalWL = 0;
    for( int n = index1; n < index2; n++ )	// for each net
    {
	if( pNLP->m_pDB->m_nets[n].size() == 0 )
	    continue;
	/*if( gArg.CheckExist("WAE") || gArg.CheckExist("SWAE") )
	{
	    if( true == param.bNLPNetWt )
	    {
		totalWL +=
		    NetWeightCalc( pNLP->m_pDB->m_nets[n].size() ) *
		    (pNLP->m_nets_weighted_sum_exp_xi_over_alpha[n] / pNLP->m_nets_sum_exp_xi_over_alpha[n] -
		     pNLP->m_nets_weighted_sum_exp_inv_xi_over_alpha[n] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] +
		     m_yWeight * (pNLP->m_nets_weighted_sum_exp_yi_over_alpha[n] / pNLP->m_nets_sum_exp_yi_over_alpha[n] -
			pNLP->m_nets_weighted_sum_exp_inv_yi_over_alpha[n] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[n]));
	    }else
	    {
		totalWL +=
		    (pNLP->m_nets_weighted_sum_exp_xi_over_alpha[n] / pNLP->m_nets_sum_exp_xi_over_alpha[n] -
		     pNLP->m_nets_weighted_sum_exp_inv_xi_over_alpha[n] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] +
		     m_yWeight * (pNLP->m_nets_weighted_sum_exp_yi_over_alpha[n] / pNLP->m_nets_sum_exp_yi_over_alpha[n] -
			pNLP->m_nets_weighted_sum_exp_inv_yi_over_alpha[n] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[n]));
	    }
	}else */if( param.bUseLSE )
	{
	    /*totalWL += 
	      log( pNLP->m_nets_sum_exp_xi_over_alpha[n] ) +	    // max(x)
	      log( pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] ) +  // -min(x)
	      log( pNLP->m_nets_sum_exp_yi_over_alpha[n] ) +	    // max(y)
	      log( pNLP->m_nets_sum_exp_inv_yi_over_alpha[n] ) ;  // -min(y)*/
	    if( param.bUseSLSE )
	    {
		if ( true == param.bNLPNetWt )
		{
		    totalWL +=
		    NetWeightCalc( pNLP->m_pDB->m_nets[n].size() ) *
		    (log( pNLP->m_nets_sum_exp_xi_over_alpha[n] ) + pNLP->m_nets_max_xi[n]/pNLP->_alpha +
		     log( pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] ) - pNLP->m_nets_min_xi[n]/pNLP->_alpha +
		     m_yWeight * (log( pNLP->m_nets_sum_exp_yi_over_alpha[n] ) + pNLP->m_nets_max_yi[n]/pNLP->_alpha +
			 log( pNLP->m_nets_sum_exp_inv_yi_over_alpha[n] ) - pNLP->m_nets_min_yi[n]/pNLP->_alpha) ) ;
		}
		else
		{
		    totalWL +=
		    log( pNLP->m_nets_sum_exp_xi_over_alpha[n] ) + pNLP->m_nets_max_xi[n]/pNLP->_alpha +
		    log( pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] ) - pNLP->m_nets_min_xi[n]/pNLP->_alpha +
		    m_yWeight * (log( pNLP->m_nets_sum_exp_yi_over_alpha[n] ) + pNLP->m_nets_max_yi[n]/pNLP->_alpha +
			log( pNLP->m_nets_sum_exp_inv_yi_over_alpha[n] ) - pNLP->m_nets_min_yi[n]/pNLP->_alpha) ;
		}
	    }
	    else
	    {
		if ( true == param.bNLPNetWt )
		{
		    totalWL += 
		    NetWeightCalc( pNLP->m_pDB->m_nets[n].size() ) * 
		    (log( pNLP->m_nets_sum_exp_xi_over_alpha[n] ) +	          // max(x)
		     log( pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] ) +		  // -min(x)
		     m_yWeight * (log( pNLP->m_nets_sum_exp_yi_over_alpha[n] ) +  // max(y)
		     	log( pNLP->m_nets_sum_exp_inv_yi_over_alpha[n] ) ) ) ;    // -min(y)
		}
		else
		{
		    totalWL += 
		    log( pNLP->m_nets_sum_exp_xi_over_alpha[n] ) +	          // max(x)
		    log( pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] ) +            // -min(x)
		    m_yWeight * (log( pNLP->m_nets_sum_exp_yi_over_alpha[n] ) +	  // max(y)
			log( pNLP->m_nets_sum_exp_inv_yi_over_alpha[n] ) ) ;      // -min(y)
		}
	    }
	}
	else
	{
	    // LP-norm
	    double invAlpha = 1.0 / pNLP->_alpha;
	    totalWL += 
		pow( pNLP->m_nets_sum_exp_xi_over_alpha[n], invAlpha ) - 
		pow( pNLP->m_nets_sum_exp_inv_xi_over_alpha[n], -invAlpha ) +
		m_yWeight * ( pow( pNLP->m_nets_sum_exp_yi_over_alpha[n], invAlpha ) - 
			pow( pNLP->m_nets_sum_exp_inv_yi_over_alpha[n], -invAlpha ) );
	    /*totalWL += 
	      pow( pNLP->m_nets_sum_exp_xi_over_alpha[n], invAlpha ) - 
	      pow( pNLP->m_nets_sum_exp_inv_xi_over_alpha[n], -invAlpha ) +
	      pow( pNLP->m_nets_sum_exp_yi_over_alpha[n], invAlpha ) - 
	      pow( pNLP->m_nets_sum_exp_inv_yi_over_alpha[n], -invAlpha );*/
	    //assert( !isNaN( totalWL ) );
	}
    }

    /*if( gArg.CheckExist("WAE") || gArg.CheckExist("SWAE")  )
    	return totalWL;
    else */if( param.bUseLSE )
	return totalWL * pNLP->_alpha;
    else
	return totalWL / pNLP->m_posScale;
}

double MyNLP::eval_f()
{
    gTotalWL = Parallel( GetLogSumExpWLThread, m_pDB->m_nets.size() );
    gDensity = GetDensityPanelty();

    double obj_value = 0;

    /*if( bMulti )
    {
	obj_value = (gTotalWL * _weightWire) + 0.5 * (density);
    }
    else*/
    //{
	obj_value = (gTotalWL * _weightWire) + 0.5 * (gDensity * _weightDensity);
    //}

#if 0
    assert( !isNaN( gTotalWL ) );
    assert( !isNaN( gDensity ) );
#endif

    return obj_value;
}

void MyNLP::PrintPotentialGrid()
{
    for( unsigned int i = m_gridPotential.size()-1; i>=0; i-- )
    {
	for( unsigned int j = 0; j < m_gridPotential[i].size(); j++ )
	    printf( "%4.1f ", (m_gridPotential[i][j]-m_expBinPotential[i][j])/m_expBinPotential[i][j] );
	printf( "\n" );
    }
    printf( "\n\n" );
}

double MyNLP::GetDensityPanelty()
{
    double density = 0;
    for( unsigned int i = 0; i < m_gridPotential.size(); i++ )
    {
	for( unsigned int j = 0; j < m_gridPotential[i].size(); j++ )
	{
	    /*if( bMulti )
	    {
		double p = m_weightDensity[i][j] * 
		    ( m_gridPotential[i][j] - m_expBinPotential[i][j] ) *
		    ( m_gridPotential[i][j] - m_expBinPotential[i][j] );

		density += p;
	    }
	    else*/
	    {
		density += 
		    ( m_gridPotential[i][j] - m_expBinPotential[i][j] ) *
		    ( m_gridPotential[i][j] - m_expBinPotential[i][j] );
	    }
	}
    }

    return density;
}

void MyNLP::InitModuleNetPinId()
{
    //printf( "Init module-net-pin id\n" );
    m_moduleNetPinId.resize( m_pDB->m_modules.size() );
    for( unsigned int i = 0; i < m_pDB->m_modules.size(); i++ )
    {
	m_moduleNetPinId[i].resize( m_pDB->m_modules[i].m_netsId.size(), -1 );
	for( unsigned int j = 0; j < m_pDB->m_modules[i].m_netsId.size(); j++ )
	{
	    int netId = m_pDB->m_modules[i].m_netsId[j];
	    int pinId = -1;
	    for( unsigned int p = 0; p < m_pDB->m_nets[netId].size(); p++ )
	    {
		if( m_pDB->m_pins[ m_pDB->m_nets[netId][p] ].moduleId == (int)i )
		{
		    pinId = m_pDB->m_nets[netId][p];
		    break;
		}
	    }
	    assert( pinId != -1 );  // net without pin?
#if 0
	    if( pinId == -1 )
	    {
		printf( "Floating net %s (%d) in module %s (%d) (net # = %d, pin # = %d)\n", 
			m_pDB->m_netsName[netId].c_str(), netId, 
			m_pDB->m_modules[i].m_name.c_str(), i,
			m_pDB->m_modules[i].m_netsId.size(), 
			m_pDB->m_modules[i].m_pinsId.size() );	
		for( unsigned int p=0; p<m_pDB->m_nets[netId].size(); p++ )
		{
		    printf( " pin %s %d (module %d)  ", 
			    m_pDB->m_pins[ m_pDB->m_nets[netId][p] ].pinName.c_str(),
			    m_pDB->m_nets[netId][p], 
			    m_pDB->m_pins[ m_pDB->m_nets[netId][p] ].moduleId );
		}
		printf( "\n" );
	    }
#endif
	    m_moduleNetPinId[i][j] = pinId;
	} // each net to the module
    } // each module
}


// static 
void* MyNLP::eval_grad_f_thread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);

    double tStart = seconds();	
    UpdateGradWire( pMsg->pNLP, pMsg->index1, pMsg->index2 );       // wire forces
    time_wire_force += seconds() - tStart;			    // CPU time is wrong if th != 1

    tStart = seconds();
    UpdateGradPotential( pMsg->pNLP, pMsg->index1, pMsg->index2 );  // spreading forces
    time_spreading_force += seconds() - tStart;

    for( int i=2*pMsg->index1; i<2*pMsg->index2; i++ )
    {
	/*if( bMulti )
	{
	    pMsg->pNLP->grad_f[i] =  
	        pMsg->pNLP->_weightWire    * pMsg->pNLP->grad_wire[i] +
	        pMsg->pNLP->grad_potential[i];
	}
	else*/
	{
	    pMsg->pNLP->grad_f[i] =  
	        pMsg->pNLP->_weightWire    * pMsg->pNLP->grad_wire[i] +
	        pMsg->pNLP->_weightDensity * pMsg->pNLP->grad_potential[i];
	}
    }
    if( pMsg->pNLP->m_macroRotate ) // (kaie) macro rotation
    {
	for( int i=pMsg->index1; i<pMsg->index2; i++ )
	{
	    //printf("%lf, %lf\n", pMsg->pNLP->grad_rotate[i], pMsg->pNLP->grad_potentialR[i]);
	    //assert(!isNaN(pMsg->pNLP->grad_rotate[i]));
	    //assert(!isNaN(pMsg->pNLP->grad_potentialR[i]));
	    //assert(pMsg->pNLP->grad_rotate[i] < DBL_MAX * 0.5);
	    //assert(pMsg->pNLP->grad_potentialR[i] < DBL_MAX * 0.5);
	    /*if( bMulti )
	    {
		pMsg->pNLP->grad_f_r[i] =
		    pMsg->pNLP->_weightWire    * pMsg->pNLP->grad_rotate[i] +
		    pMsg->pNLP->grad_potentialR[i];
	    }else*/
	    {
		pMsg->pNLP->grad_f_r[i] =
		    pMsg->pNLP->_weightWire    * pMsg->pNLP->grad_rotate[i] +
		    pMsg->pNLP->_weightDensity * pMsg->pNLP->grad_potentialR[i];
	    }
	}
    }
    
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

bool MyNLP::eval_grad_f(int n, const vector<double>& x, const vector<double>& expX, bool new_x, vector<double>& grad_f)
{
    // grad WL
    if( _weightWire > 0 )	//TEST
    {
	Parallel( UpdateGradWireThread, (int)m_pDB->m_modules.size() );
    }

    // grad Density
    Parallel( UpdateGradPotentialThread, (int)m_pDB->m_modules.size() );
    
    // compute total fouce
    /*if( bMulti )
    {
	for( int i = 0; i < n; i++ )
	    grad_f[i] =  
		_weightWire * grad_wire[i] + grad_potential[i];
    }
    else*/
    {
	for( int i =0; i<n; i++ )
	    grad_f[i] =  
		_weightWire * grad_wire[i] + _weightDensity * grad_potential[i];
    }
    return true;
}

// Weighted-Average Wirelength Model
void MyNLP::ComputeNetSumDelta()
{
    for( unsigned int i = 0; i < m_pDB->m_nets.size(); i++ ) // for each net
    {
	//double sum_exp_xi_over_alpha_2 = m_nets_sum_exp_xi_over_alpha[i] * m_nets_sum_exp_xi_over_alpha[i];
	//double sum_exp_inv_xi_over_alpha_2 = m_nets_sum_exp_inv_xi_over_alpha[i] * m_nets_sum_exp_inv_xi_over_alpha[i];
	//double sum_exp_yi_over_alpha_2 = m_nets_sum_exp_yi_over_alpha[i] * m_nets_sum_exp_yi_over_alpha[i];
	//double sum_exp_inv_yi_over_alpha_2 = m_nets_sum_exp_inv_yi_over_alpha[i] * m_nets_sum_exp_inv_yi_over_alpha[i];

    	for( unsigned int j = 0; j < m_pDB->m_nets[i].size(); j++ ) // for each pin
	{
	    sum_delta_x[     m_pDB->m_nets[i][j] ] = 0;
	    sum_delta_inv_x[ m_pDB->m_nets[i][j] ] = 0;
	    sum_delta_y[     m_pDB->m_nets[i][j] ] = 0;
	    sum_delta_inv_y[ m_pDB->m_nets[i][j] ] = 0;
	}

	for( unsigned int j = 0; j < m_pDB->m_nets[i].size(); j++ ) // for each pair of pins
	{
	    int pinId_j = m_pDB->m_nets[i][j];
	    int moduleId_j = m_pDB->m_pins[pinId_j].moduleId;
	    double xx_j = x[ 2*moduleId_j   ];//pinX[2*selfPinId];
	    double yy_j = x[ 2*moduleId_j+1 ];//pinX[2*selfPinId+1];
	    double exp_xj, exp_yj, exp_inv_xj, exp_inv_yj;
	    if( m_usePin[moduleId_j] )
	    {
		xx_j += m_pDB->m_pins[pinId_j].xOff;
		yy_j += m_pDB->m_pins[pinId_j].yOff;
	    }
	    /*if( gArg.CheckExist("SWAE") )
	    {
	    	exp_xj = expXnet[i][ 4*j ];
		exp_inv_xj = expXnet[i][ 4*j+1 ];
		exp_yj = expXnet[i][ 4*j+2 ];
		exp_inv_yj = expXnet[i][ 4*j+3 ];
	    }else*/
	    {
	    	if(m_usePin[moduleId_j] )
		{
		    exp_xj = _expPins[  2*pinId_j  ];
		    exp_inv_xj = 1.0 / _expPins[  2*pinId_j  ];
		    exp_yj = _expPins[ 2*pinId_j+1 ];
		    exp_inv_yj = 1.0 / _expPins[ 2*pinId_j+1 ];
		}else
		{
		    exp_xj = _expX[  2*moduleId_j  ];
		    exp_inv_xj = 1.0 / _expX[  2*moduleId_j  ];
		    exp_yj = _expX[ 2*moduleId_j+1 ];
		    exp_inv_yj = 1.0 / _expX[ 2*moduleId_j+1 ];
		}
	    }

	    for( unsigned int k = j; k < m_pDB->m_nets[i].size(); k++ )
	    {
	    	int pinId_k = m_pDB->m_nets[i][k];
		int moduleId_k = m_pDB->m_pins[pinId_k].moduleId;
		double xx_k = x[ 2*moduleId_k   ];//pinX[ 2*pinId_k ];
		double yy_k = x[ 2*moduleId_k+1 ];//pinX[ 2*pinId_k+1 ];
		double exp_xk, exp_yk, exp_inv_xk, exp_inv_yk;
		if( m_usePin[ moduleId_k ] )
		{
		    xx_k += m_pDB->m_pins[pinId_k].xOff;
		    yy_k += m_pDB->m_pins[pinId_k].yOff;
		}
		/*if( gArg.CheckExist("SWAE") )
		{
		    exp_xk = expXnet[i][ 4*k ];
		    exp_inv_xk = expXnet[i][ 4*k+1 ];
		    exp_yk = expXnet[i][ 4*k+2 ];
		    exp_inv_yk = expXnet[i][ 4*k+3 ];
		}else*/
		{
		    if( m_usePin[ moduleId_k ] )
		    {
			exp_xk = _expPins[  2*pinId_k  ];
			exp_inv_xk = 1.0 / _expPins[  2*pinId_k  ];
			exp_yk = _expPins[ 2*pinId_k+1 ];
			exp_inv_yk = 1.0 / _expPins[ 2*pinId_k+1 ];
		    }else
		    {
			exp_xk = _expX[  2*moduleId_k  ];
			exp_inv_xk = 1.0 / _expX[  2*moduleId_k  ];
			exp_yk = _expX[ 2*moduleId_k+1 ];
			exp_inv_yk = 1.0 / _expX[ 2*moduleId_k+1 ];
		    }
		}

		double delta_x = (xx_k - xx_j) / _alpha;
		double delta_y = (yy_k - yy_j) / _alpha;

		//double exp_xk_xj = exp_xk/exp_xj;
		//double exp_xj_xk = exp_xj/exp_xk;
		//double exp_yk_yj = exp_yk/exp_yj;
		//double exp_yj_yk = exp_yj/exp_yk;
		double exp_xk_xj = exp_xk * exp_inv_xj;
		double exp_xj_xk = exp_xj * exp_inv_xk;
		double exp_yk_yj = exp_yk * exp_inv_yj;
		double exp_yj_yk = exp_yj * exp_inv_yk;

		double delta_x_jk     =  delta_x * exp_xk_xj;
		double delta_inv_x_jk = -delta_x * exp_xj_xk;
		double delta_y_jk     =  delta_y * exp_yk_yj;
		double delta_inv_y_jk = -delta_y * exp_yj_yk;

		sum_delta_x[     pinId_j ] += delta_x_jk;
		sum_delta_inv_x[ pinId_j ] += delta_inv_x_jk;
		sum_delta_y[     pinId_j ] += delta_y_jk;
		sum_delta_inv_y[ pinId_j ] += delta_inv_y_jk;

		sum_delta_x[     pinId_k ] += delta_inv_x_jk;
		sum_delta_inv_x[ pinId_k ] += delta_x_jk;
		sum_delta_y[     pinId_k ] += delta_inv_y_jk;
		sum_delta_inv_y[ pinId_k ] += delta_y_jk;
	    }
	}

    	/*for( unsigned int j = 0; j < m_pDB->m_nets[i].size(); j++ ) // for each pin
	{
	    int pinId = m_pDB->m_nets[i][j];
	    int moduleId = m_pDB->m_pins[pinId].moduleId;

	    double exp_x_2, exp_y_2;

	    if(m_usePin[moduleId])
	    {
		exp_x_2 = _expPins[ 2*pinId ] * _expPins[ 2*pinId ];
		exp_y_2 = _expPins[2*pinId+1] * _expPins[2*pinId+1];
	    }else
	    {
	    	exp_x_2 = _expX[ 2*moduleId ] * _expX[ 2*moduleId ];
		exp_y_2 = _expX[2*moduleId+1] * _expX[2*moduleId+1];
	    }
	    sum_delta_x[pinId] *= exp_x_2 / sum_exp_xi_over_alpha_2;
	    sum_delta_inv_x[pinId] *= 1.0 / exp_x_2 / sum_exp_inv_xi_over_alpha_2;
	    sum_delta_y[pinId] *= exp_y_2 / sum_exp_yi_over_alpha_2;;
	    sum_delta_inv_y[pinId] *= 1.0 / exp_y_2 / sum_exp_inv_yi_over_alpha_2;
	}*/
    }
}

// static
void* MyNLP::UpdateGradWireThread( void* arg )   
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    UpdateGradWire( pMsg->pNLP, pMsg->index1, pMsg->index2 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

//static
void MyNLP::UpdateGradWire( MyNLP* pNLP, int index1, int index2 )
{
    if( index2 > (int)pNLP->m_pDB->m_modules.size() )
	index2 = (int)pNLP->m_pDB->m_modules.size();

    for( int i = index1; i < index2; i++ )	// for each block
    {
	if( pNLP->m_pDB->m_modules[i].m_isFixed || pNLP->m_pDB->m_modules[i].m_netsId.size() == 0 )
	    continue;

	pNLP->grad_wire[ 2*i ] = 0;
	pNLP->grad_wire[ 2*i+1 ] = 0;
	if( pNLP->m_macroRotate )
	    pNLP->grad_rotate[i] = 0;

	for( unsigned int j = 0; j < pNLP->m_pDB->m_modules[i].m_netsId.size(); j++ )
	{
	    // for each net connecting to the block
	    int netId = pNLP->m_pDB->m_modules[i].m_netsId[j];
	    if( pNLP->m_pDB->m_nets[netId].size() == 0 )  // doubleing-module
	    	continue;

	    int selfPinId = pNLP->m_moduleNetPinId[i][j];
	    if( selfPinId == -1 )
	    	continue;   
	    int selfPinIndex = -1;
	    for( unsigned int k = 0; k < pNLP->m_pDB->m_nets[netId].size(); k++ )
	    {
		if( pNLP->m_pDB->m_nets[netId][k] == selfPinId ) selfPinIndex = k;
	    }
	    if( selfPinIndex == -1 )
		continue;

	    // (kaie) 2011-05-11 Weighted-Average-Exponential Wirelength Model
	    //double sum_delta_x = 0;
	    //double sum_delta_inv_x = 0;
	    //double sum_delta_y = 0;
	    //double sum_delta_inv_y = 0;

	    /*if( gArg.CheckExist("WAE") || gArg.CheckExist("SWAE") )
	    {
		sum_delta_x     = pNLP->sum_delta_x[     selfPinId ];
		sum_delta_inv_x = pNLP->sum_delta_inv_x[ selfPinId ];
		sum_delta_y     = pNLP->sum_delta_y[     selfPinId ];
		sum_delta_inv_y = pNLP->sum_delta_inv_y[ selfPinId ];*/

		/*double xx_j = pNLP->x[2*i];//pinX[2*selfPinId];
		double yy_j = pNLP->x[2*i+1];//pinX[2*selfPinId+1];
		double exp_xj, exp_yj;
		if( pNLP->m_usePin[i] )
		{
		    xx_j += pNLP->m_pDB->m_pins[selfPinId].xOff;
		    yy_j += pNLP->m_pDB->m_pins[selfPinId].yOff;
		    exp_xj = pNLP->_expPins[ 2*selfPinId ];
		    exp_yj = pNLP->_expPins[ 2*selfPinId+1 ];
		}else
		{
		    exp_xj = pNLP->_expX[ 2*i ];
		    exp_yj = pNLP->_expX[ 2*i+1 ];
		}

		for(unsigned int k = 0; k < pNLP->m_pDB->m_nets[netId].size(); k++)
		{
		    int pinId_k = pNLP->m_pDB->m_nets[netId][k];
		    int moduleId_k = pNLP->m_pDB->m_pins[pinId_k].moduleId;
		    double xx_k = pNLP->x[2*moduleId_k];//pinX[ 2*pinId_k ];
		    double yy_k = pNLP->x[2*moduleId_k+1];//pinX[ 2*pinId_k+1 ];
		    double exp_xk, exp_yk;
		    if( pNLP->m_usePin[ moduleId_k ] )
		    {
		        xx_k += pNLP->m_pDB->m_pins[pinId_k].xOff;
			yy_k += pNLP->m_pDB->m_pins[pinId_k].yOff;
			exp_xk = pNLP->_expPins[ 2*pinId_k ];
			exp_yk = pNLP->_expPins[ 2*pinId_k+1 ];
		    }else
		    {
			exp_xk = pNLP->_expX[ 2*moduleId_k ];
			exp_yk = pNLP->_expX[ 2*moduleId_k+1 ];
		    }

		    double delta_x = (xx_k - xx_j) / pNLP->_alpha;
		    double delta_y = (yy_k - yy_j) / pNLP->_alpha;

		    sum_delta_x     +=  delta_x * (exp_xk/exp_xj);//exp(delta_x);
		    sum_delta_inv_x += -delta_x * (exp_xj/exp_xk);//exp(-delta_x);
		    sum_delta_y     +=  delta_y * (exp_yk/exp_yj);//exp(delta_y);
		    sum_delta_inv_y += -delta_y * (exp_yj/exp_yk);//exp(-delta_y);

		}*/
	    //}

	    double grad_wire_x = 0, grad_wire_y = 0;

	    if( pNLP->m_usePin[i] )
	    {
		assert( selfPinId != -1 );
                /*if( gArg.CheckExist("WAE") || gArg.CheckExist("SWAE") )
		{
		    double T_x, T_y, T_inv_x, T_inv_y;
		    if( gArg.CheckExist("SWAE") )
		    {
			T_x = pNLP->expXnet[netId][  4*selfPinIndex  ] / pNLP->m_nets_sum_exp_xi_over_alpha[netId];
			T_y = pNLP->expXnet[netId][ 4*selfPinIndex+2 ] / pNLP->m_nets_sum_exp_yi_over_alpha[netId];
			T_inv_x = pNLP->expXnet[netId][ 4*selfPinIndex+1] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId];
			T_inv_y = pNLP->expXnet[netId][ 4*selfPinIndex+3] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId];
		    }else
		    {
			T_x = pNLP->_expPins[ 2*selfPinId ] / pNLP->m_nets_sum_exp_xi_over_alpha[netId];
			T_y = pNLP->_expPins[ 2*selfPinId+1 ] / pNLP->m_nets_sum_exp_yi_over_alpha[netId];
			T_inv_x = 1.0 / pNLP->_expPins[ 2*selfPinId ] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId];
			T_inv_y = 1.0 / pNLP->_expPins[ 2*selfPinId+1 ] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId];
		    }
		    if ( true == param.bNLPNetWt )
		    {
			grad_wire_x =
			    NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) *
			    (T_x - T_inv_x);
			    //(T_x-T_x*T_x*sum_delta_x) - (T_inv_x-T_inv_x*T_inv_x*sum_delta_inv_x);
			grad_wire_y =
			    NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) *
			    (T_y - T_inv_y);
			    //(T_y-T_y*T_y*sum_delta_y) - (T_inv_y-T_inv_y*T_inv_y*sum_delta_inv_y);
		    }else
		    {
			grad_wire_x = T_x - T_inv_x;
			    //(T_x-T_x*T_x*sum_delta_x) - (T_inv_x-T_inv_x*T_inv_x*sum_delta_inv_x);
			grad_wire_y = T_y - T_inv_y;
			    //(T_y-T_y*T_y*sum_delta_y) - (T_inv_y-T_inv_y*T_inv_y*sum_delta_inv_y);
		    }
		}
		else */if( param.bUseLSE )
		{
		    if( param.bUseSLSE )
		    {
			if ( true == param.bNLPNetWt )
		        {
		            grad_wire_x = 
			    	NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) * 
        		    	(pNLP->expXnet[netId][ 4*selfPinIndex ] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
        		    	 pNLP->expXnet[netId][ 4*selfPinIndex+1 ] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId]) ;
        		    grad_wire_y = 
        		    	NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) * 
        		    	(pNLP->expXnet[netId][ 4*selfPinIndex+2 ] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
        		     	 pNLP->expXnet[netId][ 4*selfPinIndex+3 ] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId] );
		        }
		        else
		        {
        		    grad_wire_x = 
        		    	pNLP->expXnet[netId][ 4*selfPinIndex ] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
        		    	pNLP->expXnet[netId][ 4*selfPinIndex+1 ] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId];
        		    grad_wire_y = 
        		    	pNLP->expXnet[netId][ 4*selfPinIndex+2 ] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
        		    	pNLP->expXnet[netId][ 4*selfPinIndex+3 ] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId];
	        	}
		    }
		    else
		    {
			if ( true == param.bNLPNetWt )
		        {
		            grad_wire_x = 
			    	NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) * 
        		    	(pNLP->_expPins[ 2*selfPinId ] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
        		    	 1.0 / pNLP->_expPins[ 2*selfPinId ] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId]) ;
        		    grad_wire_y = 
        		    	NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) * 
        		    	(pNLP->_expPins[ 2*selfPinId+1 ] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
        		    	 1.0 / pNLP->_expPins[ 2*selfPinId+1 ] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId] );
		        }
		        else
		        {
        		    grad_wire_x = 
        		    	pNLP->_expPins[ 2*selfPinId ] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
        		    	1.0 / pNLP->_expPins[ 2*selfPinId ] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId];
        		    grad_wire_y = 
        		    	pNLP->_expPins[ 2*selfPinId+1 ] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
        		    	1.0 / pNLP->_expPins[ 2*selfPinId+1 ] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId];
	        	}
		    }
		}
		else
		{
		    // LP-norm
		    double xx = pNLP->x[ 2*i ]   + pNLP->m_pDB->m_pins[ selfPinId ].xOff;
		    double yy = pNLP->x[ 2*i+1 ] + pNLP->m_pDB->m_pins[ selfPinId ].yOff;
		    // assert( xx != 0 );
		    // assert( yy != 0 ); 
		    xx *= pNLP->m_posScale;
		    yy *= pNLP->m_posScale;
		    grad_wire_x = 
			pNLP->m_nets_sum_p_x_pos[netId]     * pNLP->_expPins[2*selfPinId] / xx -
			pNLP->m_nets_sum_p_inv_x_neg[netId] / pNLP->_expPins[2*selfPinId] / xx;
		    grad_wire_y = 
			pNLP->m_nets_sum_p_y_pos[netId]     * pNLP->_expPins[2*selfPinId+1] / yy -
			pNLP->m_nets_sum_p_inv_y_neg[netId] / pNLP->_expPins[2*selfPinId+1] / yy;
#if 0
		    assert( !isNaN( pNLP->grad_wire[ 2*i ] ) );
		    assert( !isNaN( pNLP->grad_wire[ 2*i+1 ] ) );
		    assert( fabs( pNLP->grad_wire[ 2*i ] ) < DBL_MAX * 0.95 );
		    assert( fabs( pNLP->grad_wire[ 2*i+1 ] ) < DBL_MAX * 0.95 );
#endif
		}
	    }
	    else
	    {
		// use cell centers
		/*if( gArg.CheckExist("WAE") || gArg.CheckExist("SWAE"))
		{
		    double T_x, T_y, T_inv_x, T_inv_y;
		    if( gArg.CheckExist("SWAE") )
		    {
			T_x = pNLP->expXnet[netId][  4*selfPinIndex  ] / pNLP->m_nets_sum_exp_xi_over_alpha[netId];
			T_y = pNLP->expXnet[netId][ 4*selfPinIndex+2 ] / pNLP->m_nets_sum_exp_yi_over_alpha[netId];
			T_inv_x = pNLP->expXnet[netId][ 4*selfPinIndex+1] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId];
			T_inv_y = pNLP->expXnet[netId][ 4*selfPinIndex+3] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId];
		    }else
		    {
			T_x = pNLP->_expX[2*i] / pNLP->m_nets_sum_exp_xi_over_alpha[netId];
			T_y = pNLP->_expX[2*i+1] / pNLP->m_nets_sum_exp_yi_over_alpha[netId];
			T_inv_x = 1.0 / pNLP->_expX[2*i] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId];
			T_inv_y = 1.0 / pNLP->_expX[2*i+1] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId];
		    }
		    if ( true == param.bNLPNetWt )
		    {
			grad_wire_x =
			    NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) *
			    (T_x - T_inv_x);
			    //(T_x-T_x*T_x*sum_delta_x) - (T_inv_x-T_inv_x*T_inv_x*sum_delta_inv_x);
			grad_wire_y =
			    NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) *
			    (T_y - T_inv_y);
			    //(T_y-T_y*T_y*sum_delta_y) - (T_inv_y-T_inv_y*T_inv_y*sum_delta_inv_y);
		    }else
		    {
			grad_wire_x = T_x - T_inv_x;
			    //(T_x-T_x*T_x*sum_delta_x) - (T_inv_x-T_inv_x*T_inv_x*sum_delta_inv_x);
			grad_wire_y = T_y - T_inv_y;
			    //(T_y-T_y*T_y*sum_delta_y) - (T_inv_y-T_inv_y*T_inv_y*sum_delta_inv_y);
		    }
		}
		else */if( param.bUseLSE )
		{
		    if( param.bUseSLSE )
		    {
			if ( true == param.bNLPNetWt )
		        {
		            grad_wire_x = 
				NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) * 
        			(pNLP->expXnet[netId][ 4*selfPinIndex ] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
        			 pNLP->expXnet[netId][ 4*selfPinIndex+1 ] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId]) ;
        		    grad_wire_y = 
        			NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) * 
        			(pNLP->expXnet[netId][ 4*selfPinIndex+2 ] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
        			 pNLP->expXnet[netId][ 4*selfPinIndex+3 ] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId] );
		        }
		        else
		        {
        		    grad_wire_x = 
        			pNLP->expXnet[netId][ 4*selfPinIndex ] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
        			pNLP->expXnet[netId][ 4*selfPinIndex+1 ] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId];
        		    grad_wire_y = 
        			pNLP->expXnet[netId][ 4*selfPinIndex+2 ] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
        			pNLP->expXnet[netId][ 4*selfPinIndex+3 ] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId];
	        	}
		    }
		    else
		    {
    		        if ( true == param.bNLPNetWt )
    		        {
    		            grad_wire_x = 
    		                NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) * 
            			(pNLP->_expX[2*i] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
            			1.0 / pNLP->_expX[2*i] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId]) ;
            		    grad_wire_y = 
            			NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) * 
            			(pNLP->_expX[2*i+1] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
            			1.0 / pNLP->_expX[2*i+1] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId]);
		        }
		        else
		        {
        		    grad_wire_x = 
        			pNLP->_expX[2*i] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
        			1.0 / pNLP->_expX[2*i] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId];
        		    grad_wire_y = 
        			pNLP->_expX[2*i+1] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
        			1.0 / pNLP->_expX[2*i+1] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId];
			}
		    }
		}
		else
		{
		    // Lp-norm
		    double xx = pNLP->x[ 2*i ];
		    double yy = pNLP->x[ 2*i+1 ];
		    xx *= pNLP->m_posScale;
		    yy *= pNLP->m_posScale;
		    //assert( xx != 0);
		    //assert( yy != 0);
		    grad_wire_x = 
			pNLP->m_nets_sum_p_x_pos[netId]     * pNLP->_expX[2*i] / xx  -
			pNLP->m_nets_sum_p_inv_x_neg[netId] / pNLP->_expX[2*i] / xx;
		    grad_wire_y = 
			pNLP->m_nets_sum_p_y_pos[netId]     * pNLP->_expX[2*i+1] / yy -
			pNLP->m_nets_sum_p_inv_y_neg[netId] / pNLP->_expX[2*i+1] / yy;
#if 0
		    assert( !isNaN( pNLP->grad_wire[ 2*i ] ) );
		    assert( !isNaN( pNLP->grad_wire[ 2*i+1 ] ) );
		    assert( fabs( pNLP->grad_wire[ 2*i ] ) < DBL_MAX * 0.95 );
		    assert( fabs( pNLP->grad_wire[ 2*i+1 ] ) < DBL_MAX * 0.95 );
#endif
		}
	    }

	    pNLP->grad_wire[ 2*i ]   += grad_wire_x;
	    pNLP->grad_wire[ 2*i+1 ] += grad_wire_y;

	    // (kaie) macro rotation 
	    if( pNLP->m_macroRotate && pNLP->m_usePin[i] && pNLP->m_pDB->m_modules[i].m_isMacro ) // i: block index
	    {
		double xOff = pNLP->m_pDB->m_pins[selfPinId].xOff;
		double yOff = pNLP->m_pDB->m_pins[selfPinId].yOff;
		double rDegree = pNLP->rotate_degree[i] * 2 * 3.1415926;
		//assert(!isNaN(grad_wire_x));
		//assert(!isNaN(grad_wire_y));
		//assert(!isNaN(sin(rDegree)));
		//assert(!isNaN(cos(rDegree)));
		pNLP->grad_rotate[ i ] += ( grad_wire_x * ( -xOff * sin(rDegree) - yOff * cos(rDegree) )
		    + grad_wire_y * ( xOff * cos(rDegree) - yOff * sin(rDegree)) );
		//assert(!isNaN(pNLP->grad_rotate[i]));
	    }
	    // @(kaie) macro rotation
	} // for each pin in the module
    } // for each module
    
    // 2006-09-27 Y-weight (donnie)
    for( int i = index1; i < index2; i++ )	// for each block
	pNLP->grad_wire[ 2*i+1 ] *= m_yWeight;
}

// (kaie) macro rotation
double MyNLP::GetGradDensityProjection( const double& t1, const double& t2 )
{
    double base = 1.0;
    double _t1 = fmod(t1, base);

    if( _t1 < t2 - 0.5 ) _t1 += 1.0;
    else if( _t1 > t2 + 0.5 ) _t1 -= 1.0;

    double d;
    double a = 32, b = 32;

    if(_t1 >= t2)
    {
        d = _t1 - t2;
        if( d <= 0.125 )
            return -2.0 * a * d;
        else if( d <= 0.25 )
            return +2.0 * b * ( d - 0.25 );
        else
            return 0.0;
    }else
    {
        d = t2 - _t1;
        if( d <= 0.125 )
            return +2.0 * a * d;
        else if( d <= 0.25 )
            return -2.0 * b * ( d - 0.25 );
        else
            return 0.0;
    }
}

double MyNLP::GetDensityProjection( const double& t1, const double& t2 )
{
    double base = 1.0;
    double _t1 = fmod(t1, base);

    if( _t1 < t2 - 0.5 ) _t1 += 1.0;
    else if( _t1 > t2 + 0.5 ) _t1 -= 1.0;

    double d = fabs(_t1 - t2);
    double a = 32, b = 32;

    if( d <= 0.125 )
        return 1.0 - a * d * d;
    else if( d <= 0.25 )
        return b * ( d - 0.25 ) * ( d - 0.25 );
    else
        return 0.0;
}
// @(kaie) macro rotation

// static
void* MyNLP::UpdateGradPotentialThread( void* arg )   
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    UpdateGradPotential( pMsg->pNLP, pMsg->index1, pMsg->index2 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static
void MyNLP::UpdateGradPotential( MyNLP* pNLP, int index1, int index2 )
{
    double gradDensityX;
    double gradDensityY;
    double gradDensityR; // (kaie) cross potential

    if( index2 > (int)pNLP->m_pDB->m_modules.size() )
	index2 = (int)pNLP->m_pDB->m_modules.size();
    
    for( int i = index1; i < index2; i++ )	    // for each cell
    {
	if( pNLP->m_pDB->m_modules[i].m_isFixed )
	    continue;

	/*if( bFast )
	{
	    double width  = pNLP->m_pDB->m_modules[i].m_width;
	    double height = pNLP->m_pDB->m_modules[i].m_height;

	    if( height >= pNLP->m_potentialGridHeight || width >= pNLP->m_potentialGridWidth )
	    	GetPotentialGrad( pNLP->x, i, gradDensityX, gradDensityY, gradDensityR, pNLP );
	    else
	    {
	    	GetPotentialGradFast( pNLP->x, i, gradDensityX, gradDensityY, pNLP );
		gradDensityR = 0;
	    }
	}else*/
	    GetPotentialGrad( pNLP->x, i, gradDensityX, gradDensityY, gradDensityR, pNLP ); // bell-shaped potential

	gradDensityX *= pNLP->_cellPotentialNorm[i];
	gradDensityY *= pNLP->_cellPotentialNorm[i];
	gradDensityR *= pNLP->_cellPotentialNorm[i];

	pNLP->grad_potential[2*i]   = gradDensityX;
	pNLP->grad_potential[2*i+1] = gradDensityY;
	//assert( !isNaN( pNLP->grad_potential[2*i] ) );
	//assert( !isNaN( pNLP->grad_potential[2*i+1] ) );
	if(pNLP->m_macroRotate)
	{
	    pNLP->grad_potentialR[i] = gradDensityR;
	    //assert(!isNaN(pNLP->grad_potentialR[i]));
	}
    } // for each cell
}

// 2007-07-10 (donnie)
/*void MyNLP::ComputeBinGrad()
{
    for( unsigned int i = 0; i < m_binGradX.size(); i++ )
    {
	for( unsigned int j = 0; j < m_binGradX[i].size(); j++ )
	{
	    double x = m_pDB->m_coreRgn.left + i * m_potentialGridWidth;
	    double y = m_pDB->m_coreRgn.bottom + j * m_potentialGridHeight;
	    GetPointPotentialGrad( x, y, m_binGradX[i][j], m_binGradY[i][j] );
	}
    }
}*/

// static 2007-07-10 (donnie)
/*void MyNLP::GetPotentialGradFast( const vector<double>& x, const int& i, double& gradX, double& gradY, MyNLP* pNLP  )
{
    if( pNLP->m_pDB->m_modules[i].m_isFixed )
    {
	gradX = 0;
	gradY = 0;
	return;
    }

    double cellX = x[i*2];
    double cellY = x[i*2+1];

    // find 4 bins  (gx, gy) - (gx+1, gy+1)
    int gx, gy;
    pNLP->GetClosestGrid( cellX, cellY, gx, gy );

    double gxx = pNLP->m_pDB->m_coreRgn.left + gx * pNLP->m_potentialGridWidth;
    double gyy = pNLP->m_pDB->m_coreRgn.bottom + gy * pNLP->m_potentialGridHeight;
    double alpha = (cellX - gxx) / pNLP->m_potentialGridWidth;
    double beta  = (cellY - gyy) / pNLP->m_potentialGridHeight;

    // interpolate x force
    gradX = pNLP->m_binGradX[gx][gy] +
	    alpha * ( pNLP->m_binGradX[gx+1][gy] - pNLP->m_binGradX[gx][gy] ) +
	    beta  * ( pNLP->m_binGradX[gx][gy+1] - pNLP->m_binGradX[gx][gy] ) +
	    alpha * beta * ( pNLP->m_binGradX[gx][gy] + pNLP->m_binGradX[gx+1][gy+1] -
			     pNLP->m_binGradX[gx][gy+1] - pNLP->m_binGradX[gx+1][gy] );

    // interpolate y force
    gradY = pNLP->m_binGradY[gx][gy] +
	    alpha * ( pNLP->m_binGradY[gx+1][gy] - pNLP->m_binGradY[gx][gy] ) +
	    alpha * beta * ( pNLP->m_binGradY[gx][gy] + pNLP->m_binGradY[gx+1][gy+1] -
			     pNLP->m_binGradY[gx][gy+1] - pNLP->m_binGradY[gx+1][gy] );
}*/

// (donnie) 2007-07-10
/*void MyNLP::GetPointPotentialGrad( double cellX, double cellY, double& gradX, double& gradY )
{
    MyNLP* pNLP = this;
    double left   = cellX - pNLP->_potentialRX;
    double bottom = cellY - pNLP->_potentialRY;
    double right  = cellX + ( cellX - left );
    double top    = cellY + ( cellY - bottom );
    if( left   < pNLP->m_pDB->m_coreRgn.left  - pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth )
	left   = pNLP->m_pDB->m_coreRgn.left  - pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
    if( right  > pNLP->m_pDB->m_coreRgn.right + pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth )
	right  = pNLP->m_pDB->m_coreRgn.right + pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
    if( bottom < pNLP->m_pDB->m_coreRgn.bottom - pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight )
	bottom = pNLP->m_pDB->m_coreRgn.bottom - pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
    if( top    > pNLP->m_pDB->m_coreRgn.top    + pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight )
	top    = pNLP->m_pDB->m_coreRgn.top    + pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;

    int gx, gy;
    pNLP->GetClosestGrid( left, bottom, gx, gy );
    const double width = 0;
    const double height = 0;

    int gxx, gyy;
    double xx, yy;
    gradX = 0.0;
    gradY = 0.0;
    for( gxx = gx, xx = pNLP->GetXGrid( gx ); xx <= right && gxx<(int)pNLP->m_gridPotential.size();
	 gxx++, xx += pNLP->m_potentialGridWidth )
    {
	for( gyy = gy, yy = pNLP->GetYGrid( gy ); yy <= top && gyy<(int)pNLP->m_gridPotential[gxx].size();
	     gyy++, yy += pNLP->m_potentialGridHeight )
	{
	    double gX =
		GetGradPotential( cellX, xx, pNLP->_potentialRX, width,  pNLP->_alpha ) *
		GetPotential(     cellY, yy, pNLP->_potentialRY, height, pNLP->_alpha ) *
		( pNLP->m_gridPotential[gxx][gyy] - pNLP->m_expBinPotential[gxx][gyy] );
	    double gY =
		GetPotential(     cellX, xx, pNLP->_potentialRX, width,  pNLP->_alpha ) *
		GetGradPotential( cellY, yy, pNLP->_potentialRY, height, pNLP->_alpha ) *
		( pNLP->m_gridPotential[gxx][gyy] - pNLP->m_expBinPotential[gxx][gyy] );

	    if( bMulti )
	    {
		gradX += gX * m_weightDensity[gxx][gyy];
		gradY += gY * m_weightDensity[gxx][gyy];
	    }
	    else
	    {
		gradX += gX;
		gradY += gY;
	    }
	}
    } // for each grid
}*/

// static
void MyNLP::GetPotentialGrad( const vector<double>& x, const int& i, double& gradX, double& gradY, double& gradR, MyNLP* pNLP )
{
    double cellX = x[i*2];
    double cellY = x[i*2+1];

    double width  = pNLP->m_pDB->m_modules[i].m_width;
    double height = pNLP->m_pDB->m_modules[i].m_height;
   
    // (kaie) cross potential 
    double left, bottom, right, top;
    if(pNLP->m_macroRotate && pNLP->m_pDB->m_modules[i].m_isMacro)
    {
	left   = cellX - max(width, height) * 0.5 - pNLP->_potentialRX;
	bottom = cellY - max(width, height) * 0.5 - pNLP->_potentialRY;
    }else
    {
	left   = cellX - width  * 0.5 - pNLP->_potentialRX;
	bottom = cellY - height * 0.5 - pNLP->_potentialRY;
    }
    right  = cellX + ( cellX - left );
    top    = cellY + ( cellY - bottom );
    // @(kaie) cross potential

    if( left   < pNLP->m_pDB->m_coreRgn.left  - pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth )     
	left   = pNLP->m_pDB->m_coreRgn.left  - pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
    if( right  > pNLP->m_pDB->m_coreRgn.right + pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth )    
	right  = pNLP->m_pDB->m_coreRgn.right + pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
    if( bottom < pNLP->m_pDB->m_coreRgn.bottom - pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight )   
	bottom = pNLP->m_pDB->m_coreRgn.bottom - pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
    if( top    > pNLP->m_pDB->m_coreRgn.top    + pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight )      
	top    = pNLP->m_pDB->m_coreRgn.top    + pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
    
    int gx, gy;
    pNLP->GetClosestGrid( left, bottom, gx, gy );
    
    // (kaie) cross potential
    double hDensity = 1.0, vDensity = 0.0, hGradDensity = 0.0, vGradDensity = 0.0;
    if(pNLP->m_macroRotate && pNLP->m_pDB->m_modules[i].m_isMacro)
    {
	hDensity = (GetDensityProjection( pNLP->rotate_degree[i], 0.0 ) +
		    GetDensityProjection( pNLP->rotate_degree[i], 0.5 ) );
	//assert( hDensity >= 0 && hDensity <= 1.0 );

	vDensity = 1.0 - hDensity;
	hGradDensity = (GetGradDensityProjection( pNLP->rotate_degree[i], 0.0 ) +
			GetGradDensityProjection( pNLP->rotate_degree[i], 0.5 ) );
	vGradDensity = (GetGradDensityProjection( pNLP->rotate_degree[i], 0.25 ) +
			GetGradDensityProjection( pNLP->rotate_degree[i], 0.75 ) );
    }
    // @(kaie) cross potential
    
    int gxx, gyy;
    double xx, yy;
    gradX = 0.0;	
    gradY = 0.0;
    gradR = 0.0;
    for( gxx = gx, xx = pNLP->GetXGrid( gx ); xx <= right && gxx < (int)pNLP->m_gridPotential.size(); 
	    gxx++, xx += pNLP->m_potentialGridWidth )
    {

	for( gyy = gy, yy = pNLP->GetYGrid( gy ); yy <= top && gyy < (int)pNLP->m_gridPotential[gxx].size(); 
		gyy++, yy += pNLP->m_potentialGridHeight )
	{

	    // (kaie) cross potential
	    double gX_h = GetGradPotential( cellX, xx, pNLP->_potentialRX, width,  pNLP->_alpha ) *
			  GetPotential(     cellY, yy, pNLP->_potentialRY, height, pNLP->_alpha );
	    double gX_v = GetGradPotential( cellX, xx, pNLP->_potentialRX, height, pNLP->_alpha ) *
			  GetPotential(     cellY, yy, pNLP->_potentialRY, width,  pNLP->_alpha );
	    double gX   = (hDensity * gX_h + vDensity * gX_v) *
			  ( pNLP->m_gridPotential[gxx][gyy] - pNLP->m_expBinPotential[gxx][gyy] );

	    double gY_h = GetPotential(     cellX, xx, pNLP->_potentialRX, width,  pNLP->_alpha ) *
			  GetGradPotential( cellY, yy, pNLP->_potentialRY, height, pNLP->_alpha );
	    double gY_v = GetPotential(     cellX, xx, pNLP->_potentialRX, height, pNLP->_alpha ) *
			  GetGradPotential( cellY, yy, pNLP->_potentialRY, width,  pNLP->_alpha );
	    double gY   = (hDensity * gY_h + vDensity * gY_v) *
			  ( pNLP->m_gridPotential[gxx][gyy] - pNLP->m_expBinPotential[gxx][gyy] );

	    double gR = 0;
	    if(pNLP->m_macroRotate && pNLP->m_pDB->m_modules[i].m_isMacro)
	    {
	        double gR_h = GetPotential(     cellX, xx, pNLP->_potentialRX, width,  pNLP->_alpha ) *
			      GetPotential(     cellY, yy, pNLP->_potentialRY, height, pNLP->_alpha );
	        double gR_v = GetPotential(     cellX, xx, pNLP->_potentialRX, height, pNLP->_alpha ) *
			      GetPotential(     cellY, yy, pNLP->_potentialRY, width,  pNLP->_alpha );
	        gR         = (hGradDensity * gR_h + vGradDensity * gR_v) *
			     ( pNLP->m_gridPotential[gxx][gyy] - pNLP->m_expBinPotential[gxx][gyy] );
	    }
	    // @(kaie) cross potential
	    
	    /*if( bMulti )
	    {
		gradX += gX * m_weightDensity[gxx][gyy];
		gradY += gY * m_weightDensity[gxx][gyy];
		gradR += gR * m_weightDensity[gxx][gyy]; // (kaie) cross potential
	    }
	    else*/
	    {
		gradX += gX;
		gradY += gY;
		gradR += gR; // (kaie) cross potential
	    }
	}
    } // for each grid
}

// (kaie) 2011-05-11 Weighted-Average-Exponential Wirelength Model
void MyNLP::calc_weighted_sum_exp_using_pin(
	const vector<int>::const_iterator& begin, const vector<int>::const_iterator& end,
	const vector<double>& x, const vector<double>& expX, const vector<double>& expXnet_n,
	double& weighted_sum_exp_xi_over_alpha, double& weighted_sum_exp_inv_xi_over_alpha,
	double& weighted_sum_exp_yi_over_alpha, double& weighted_sum_exp_inv_yi_over_alpha,
	const CPlaceDB* pDB, const vector<bool>* pUsePin, const vector<double>& expPins,
	int id  )
{
    weighted_sum_exp_xi_over_alpha = 0;
    weighted_sum_exp_inv_xi_over_alpha = 0;
    weighted_sum_exp_yi_over_alpha = 0;
    weighted_sum_exp_inv_yi_over_alpha = 0;

    vector<int>::const_iterator ite;
    int pinId, pinIndex;
    int blockId;
    for( ite=begin, pinIndex = 0; ite!=end; ++ite, pinIndex++ )
    {
	// for each pin of the net
	pinId   = *ite;
	blockId = pDB->m_pins[ pinId ].moduleId;

	double xx = x[2*blockId];
	double yy = x[2*blockId+1];
	if( (*pUsePin)[blockId] )
	{
	    xx += pDB->m_pins[ pinId ].xOff;
	    yy += pDB->m_pins[ pinId ].yOff;
	}

	/*if( gArg.CheckExist("SWAE") ) // (kaie) 2009-09-23 Stable-WAE
	{
	    weighted_sum_exp_xi_over_alpha     += xx * expXnet_n[ 4*pinIndex ];
	    weighted_sum_exp_inv_xi_over_alpha += xx * expXnet_n[ 4*pinIndex+1 ];
	    weighted_sum_exp_yi_over_alpha     += yy * expXnet_n[ 4*pinIndex+2 ];
	    weighted_sum_exp_inv_yi_over_alpha += yy * expXnet_n[ 4*pinIndex+3 ];
	}
	else */if( (*pUsePin)[blockId] )  // macro or self pin
	{
	    // handle pins
	    weighted_sum_exp_xi_over_alpha     += xx * expPins[ 2*pinId ];
	    weighted_sum_exp_inv_xi_over_alpha += xx * 1.0 / expPins[ 2*pinId ];
	    weighted_sum_exp_yi_over_alpha     += yy * expPins[ 2*pinId+1 ];
	    weighted_sum_exp_inv_yi_over_alpha += yy * 1.0 / expPins[ 2*pinId+1 ];
	}else
	{
	    // use block center
	    weighted_sum_exp_xi_over_alpha     += xx * expX[2*blockId];
	    weighted_sum_exp_inv_xi_over_alpha += xx * 1.0 / expX[2*blockId];
	    weighted_sum_exp_yi_over_alpha     += yy * expX[2*blockId+1];
	    weighted_sum_exp_inv_yi_over_alpha += yy * 1.0 / expX[2*blockId+1];
	}
    }
}

// static
void MyNLP::calc_sum_exp_using_pin( 
	const vector<int>::const_iterator& begin, const vector<int>::const_iterator& end,
	const vector<double>& x, const vector<double>& expX, const vector<double>& expXnet_n,
	double& sum_exp_xi_over_alpha, double& sum_exp_inv_xi_over_alpha,
	double& sum_exp_yi_over_alpha, double& sum_exp_inv_yi_over_alpha, 
        const CPlaceDB* pDB, const vector<bool>* pUsePin, const vector<double>& expPins,
	int id	)
{
    sum_exp_xi_over_alpha = 0;
    sum_exp_inv_xi_over_alpha = 0;
    sum_exp_yi_over_alpha = 0;
    sum_exp_inv_yi_over_alpha = 0;

    vector<int>::const_iterator ite;
    int pinId, pinIndex;
    int blockId;
    for( ite = begin, pinIndex = 0; ite != end; ++ite, pinIndex++ )
    {
	// for each pin of the net
	pinId   = *ite;
	blockId = pDB->m_pins[ pinId ].moduleId;

	if( param.bUseSLSE ) // (kaie) 2009-09-23 Stable-LSE
	{
	    sum_exp_xi_over_alpha     += expXnet_n[ 4*pinIndex ];
	    sum_exp_inv_xi_over_alpha += expXnet_n[ 4*pinIndex+1 ];
	    sum_exp_yi_over_alpha     += expXnet_n[ 4*pinIndex+2 ];
	    sum_exp_inv_yi_over_alpha += expXnet_n[ 4*pinIndex+3 ];
	}
	else
	{	
	    if( (*pUsePin)[blockId] )	// macro or self pin
	    {
	    	// handle pins
	    	sum_exp_xi_over_alpha     += expPins[ 2*pinId ];
	    	sum_exp_inv_xi_over_alpha += 1.0 / expPins[ 2*pinId ];
	    	sum_exp_yi_over_alpha     += expPins[ 2*pinId+1 ];
	    	sum_exp_inv_yi_over_alpha += 1.0 / expPins[ 2*pinId+1 ];
	    }
	    else
	    {
	    	// use block center
	    	sum_exp_xi_over_alpha     += expX[2*blockId];
	    	sum_exp_inv_xi_over_alpha += 1.0 / expX[2*blockId];
	    	sum_exp_yi_over_alpha     += expX[2*blockId+1];
	    	sum_exp_inv_yi_over_alpha += 1.0 / expX[2*blockId+1];
	    }
	}
    }
} 

void* MyNLP::UpdateBlockPositionThread( void* arg )   
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    UpdateBlockPosition( *pMsg->pX, pMsg->pNLP, pMsg->index1, pMsg->index2 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static
void MyNLP::UpdateBlockPosition( const vector<double>& x, MyNLP* pNLP, int index1, int index2 )
{
    if( index2 > (int)pNLP->m_pDB->m_modules.size() )
	index2 = (int)pNLP->m_pDB->m_modules.size();
    for( int i = index1; i < index2; i++ )
    {
	if( pNLP->m_pDB->m_modules[i].m_isFixed == false )
	    pNLP->m_pDB->MoveModuleCenter( i, x[i*2], x[i*2+1] ); 
    }
}

void MyNLP::UpdateBlockOrientation()
{
    int count = 0;
    int count_n = 0;
    vector<int> m_macro_r;
    vector<int> m_macro_f;
    vector<int> m_dir;
    for( unsigned int i = 0; i < m_pDB->m_modules.size(); i++ )
    {
	if( m_pDB->m_modules[i].m_isMacro  )
	{
	    int module_orient = m_pDB->m_modules[i].m_orient;
	    //printf("%lf => ", rotate_degree[i]);

	    double m_degree = rotate_degree[i];
	    double base = 1.0;
	    
	    m_degree = fmod(fmod(m_degree, base) + base, base);
	    //printf("%lf\n", m_degree);

	    //assert(m_degree >= 0 && m_degree < 1);

	    int dir = (int)floor(m_degree*4.0); // rotate_degree/0.25
	    double offset = m_degree - dir*0.25;
	    bool next = offset > 0.125;
	    if(next) dir++;

	    //printf("%lf = %d, %lf\n", m_degree, dir, offset);
	    if(module_orient < 4)
	        dir = (module_orient + dir) % 4;
	    else
	    	dir = (module_orient - 4 + dir) % 4 + 4;

	    if(dir == module_orient) continue;

	    if( (abs(dir-module_orient)%2 == 1)
		&& ((m_pDB->m_modules[i].m_cx - m_pDB->m_modules[i].m_height * 0.5 < m_pDB->m_coreRgn.left)
		|| (m_pDB->m_modules[i].m_cx + m_pDB->m_modules[i].m_height * 0.5 > m_pDB->m_coreRgn.right)
		|| (m_pDB->m_modules[i].m_cy - m_pDB->m_modules[i].m_width * 0.5 < m_pDB->m_coreRgn.bottom)
		|| (m_pDB->m_modules[i].m_cy + m_pDB->m_modules[i].m_width * 0.5 > m_pDB->m_coreRgn.top)))
	    {
		m_macro_f.push_back(i);
		if(dir < 4){
		    if(dir > module_orient)
			m_pDB->SetModuleOrientationCenter(i, (dir+3)%4);
		    else
			m_pDB->SetModuleOrientationCenter(i, (dir+1)%4);
		}else
		{
		    if(dir > module_orient)
			m_pDB->SetModuleOrientationCenter(i, (dir-1)%4+4);
		    else
			m_pDB->SetModuleOrientationCenter(i, (dir+1)%4+4);
		}
	    }else if(gArg.CheckExist("mod"))
	    {
	    	if(!next && offset > 0.115)
	    	{
		    m_dir.push_back(dir);
		    m_macro_r.push_back(i);
		    count_n++;
	    	}else if(next && offset < 0.135)
	    	{
		    m_dir.push_back(dir+8);
		    m_macro_r.push_back(i);
		    count_n++;
	    	}else
	    	{
		    m_macro_f.push_back(i);
		    m_pDB->SetModuleOrientationCenter(i, (dir));
		}
	    }else
	    {
	    	m_macro_f.push_back(i);
		m_pDB->SetModuleOrientationCenter(i, (dir));
	    }
	    
	    rotate_degree[i] = 0;
	    count++;
	}
    }

    double start_time = seconds();
    if(gArg.CheckExist("mod") && count_n > 0)
    {
	int n_macros = (int)m_macro_r.size();

	vector< vector<double> > m_overlap1, m_overlap2, m_overlap3, m_overlap4, m_overlap_f1, m_overlap_f2;
	m_overlap1.resize(m_macro_r.size());
	m_overlap2.resize(m_macro_r.size());
	m_overlap3.resize(m_macro_r.size());
	m_overlap4.resize(m_macro_r.size());
	m_overlap_f1.resize(m_macro_r.size());
	m_overlap_f2.resize(m_macro_r.size());
	for(unsigned i = 0; i < m_macro_r.size(); i++)
	{
	    m_overlap1[i].resize(m_macro_r.size(), 0);
	    m_overlap2[i].resize(m_macro_r.size(), 0);
	    m_overlap3[i].resize(m_macro_r.size(), 0);
	    m_overlap4[i].resize(m_macro_r.size(), 0);
	    m_overlap_f1[i].resize(m_macro_f.size(), 0); // with macros with fixed orient
	    m_overlap_f2[i].resize(m_macro_f.size(), 0); // with macros with fixed orient
	}

    	for(unsigned i = 0; i < m_macro_r.size(); i++)
    	{
	    double width_i = m_pDB->m_modules[m_macro_r[i]].m_width;
	    double height_i = m_pDB->m_modules[m_macro_r[i]].m_height;
	    for(unsigned j = i+1; j < m_macro_r.size(); j++)
	    {
	    	double x_diff = fabs(m_pDB->m_modules[m_macro_r[i]].m_cx - m_pDB->m_modules[m_macro_r[j]].m_cx);
	    	double y_diff = fabs(m_pDB->m_modules[m_macro_r[i]].m_cy - m_pDB->m_modules[m_macro_r[j]].m_cy);
	    	double width_j = m_pDB->m_modules[m_macro_r[j]].m_width;
	    	double height_j = m_pDB->m_modules[m_macro_r[j]].m_height;

	    	m_overlap1[i][j] = m_overlap1[j][i] =
		    max( (width_i + width_j)/2-x_diff, 0 ) * 
		    max( (height_i + height_j)/2-y_diff, 0 ); // u, v
	    	m_overlap2[i][j] = m_overlap2[j][i] =
		    max( (width_i + height_j)/2-x_diff, 0 ) *
		    max( (height_i + width_j)/2-y_diff, 0 ); // u, vR
	    	m_overlap3[i][j] = m_overlap3[j][i] =
		    max( (height_i + width_j)/2-x_diff, 0 ) *
		    max( (width_i + height_j)/2-y_diff, 0 ); // uR, v
	    	m_overlap4[i][j] = m_overlap4[j][i] =
		    max( (height_i + height_j)/2-x_diff, 0 ) *
		    max( (width_i + width_j)/2-y_diff, 0 ); // uR, vR

	    	//assert(m_overlap1[i][j] >= 0);
	    	//assert(m_overlap2[i][j] >= 0);
	    	//assert(m_overlap3[i][j] >= 0);
	    	//assert(m_overlap4[i][j] >= 0);
	    }

	    for(unsigned k = 0; k < m_macro_f.size(); k++)
	    {
		double x_diff = fabs(m_pDB->m_modules[m_macro_r[i]].m_cx - m_pDB->m_modules[m_macro_f[k]].m_cx);
		double y_diff = fabs(m_pDB->m_modules[m_macro_r[i]].m_cy - m_pDB->m_modules[m_macro_f[k]].m_cy);
		double width_k = m_pDB->m_modules[m_macro_f[k]].m_width;
		double height_k = m_pDB->m_modules[m_macro_f[k]].m_height;

		m_overlap_f1[i][k] = 
		    max( (width_i + width_k)/2-x_diff, 0 ) *
		    max( (height_i + height_k)/2-y_diff, 0 ); // u, v
		m_overlap_f2[i][k] =
		    max( (height_i + width_k)/2-x_diff, 0 ) *
		    max( (width_i + height_k)/2-y_diff, 0 ); // uR, v

		//assert(m_overlap_f1[i][k] >= 0);
		//assert(m_overlap_f2[i][k] >= 0);
	    }
    	}

	// ILP formulation
    	lprec *lp;
    	int Ncol, *colno = NULL, ret = 0;
    	REAL *row = NULL;

    	Ncol = n_macros * (1 + n_macros); // N + N^2

    	lp = make_lp(0, Ncol);
    	if(lp == NULL)
	    ret = 1; // couldn't construct a new model...

	//variables
    	if(ret == 0)
    	{
	    // create space large enough for one row 
	    colno = (int*)malloc(Ncol * sizeof(*colno));
	    row = (REAL*)malloc(Ncol * sizeof(*row));
	    if((colno == NULL) || (row == NULL))
	    	ret = 2;

	    for(int i = 0; i < Ncol; i++)
	    	set_binary(lp, 1+i, true);
    	}

    	//constraints
    	if(ret == 0)
    	{
	    for(int u = 0; u < n_macros; u++)
	    {
	    	for(int v = u+1; v < n_macros; v++)
	    	{
		    int col_u = 1+u;
		    int col_v = 1+v;
		    int col_uv = 1+n_macros+u*n_macros+v;

		    // r_uv - r_u <= 0
		    int k = 0;
		    colno[k] = col_uv;
		    row[k++] = 1;
		    colno[k] = col_u;
		    row[k++] = -1;
		    if(!add_constraintex(lp, k, row, colno, LE, 0))
			    ret = 3;

		    // r_uv - r_v <= 0
		    k = 0;
		    colno[k] = col_uv;
		    row[k++] = 1;
		    colno[k] = col_v;
		    row[k++] = -1;
		    if(!add_constraintex(lp, k, row, colno, LE, 0))
			    ret = 3;

		    // r_u + r_v - r_uv <= 1
		    k = 0;
		    colno[k] = col_u;
		    row[k++] = 1;
		    colno[k] = col_v;
		    row[k++] = 1;
		    colno[k] = col_uv;
		    row[k++] = -1;
		    if(!add_constraintex(lp, k, row, colno, LE, 1))
			    ret = 3;
		}
	    }
	    set_add_rowmode(lp, FALSE);
	}

	// objective function
	vector<double> cost;
	cost.resize(Ncol, 0);
	double c0 = 0;
	if(ret == 0)
	{
	    for(int u = 0; u < n_macros; u++)
	    {
		for(int v = u+1; v < n_macros; v++)
		{
		    double c1 = 0, c2 = 0, c3 = 0;
		    int col_u = 1+u;
		    int col_v = 1+v;
		    int col_uv = 1+n_macros+u*n_macros+v;

		    c0 += m_overlap1[u][v];
		    c1 = m_overlap3[u][v] - m_overlap1[u][v];
		    c2 = m_overlap2[u][v] - m_overlap1[u][v];
		    c3 = m_overlap1[u][v] + m_overlap4[u][v] - m_overlap2[u][v] - m_overlap3[u][v];

		    cost[col_u-1] += c1;
		    cost[col_v-1] += c2;
		    cost[col_uv-1] += c3;
		}

		for(unsigned k = 0; k < m_macro_f.size(); k++)
		{
		    int col_u = 1+u;
		    c0 += m_overlap_f1[u][k];
		    cost[col_u-1] += m_overlap_f2[u][k] - m_overlap_f1[u][k];
		}
	    }

	    int k = 0;
	    //colno[k] = 0;
	    //row[k++] = c0;
	    for(int i = 0; i < Ncol; i++)
	    {
		colno[k] = i+1;
		row[k++] = cost[i];
	    }

	    if(!set_obj_fnex(lp, k, row, colno))
		ret = 4;
	}

	if(ret == 0)
	{
	    set_minim(lp); // set the object direction to minimize

	    //write_LP(lp, stdout);
	    //write_lp(lp, "model.lp");

	    set_verbose(lp, IMPORTANT);

	    ret = solve(lp);
	    if(ret == OPTIMAL)
		ret = 0;
	    else
		ret = 5;
	}

	if(ret == 0) {
	    // objective value
	    //printf("Objective value: %f\n", get_objective(lp)+c0);

	    /* variable values */
	    get_variables(lp, row);
	    /*for(j = 0; j < Ncol; j++)
	    printf("%s: %f\n", get_col_name(lp, j + 1), row[j]);*/
	    //int count = 0;
	    for(int i = 0; i < n_macros; i++)
	    {
		if(row[i] == 1)
		{
		    if(m_dir[i] < 8) // dir+1
		    {
			if(m_dir[i] < 4)
			    m_dir[i] = (m_dir[i]+1)%4;
			else
			    m_dir[i] = (m_dir[i]+1)%4+4;
		    }
		    else // dir-1
		    {
			if(m_dir[i] < 4)
			    m_dir[i] = (m_dir[i]+3)%4;
			else
			    m_dir[i] = (m_dir[i]+3)%4+4;
		    }
		    //count++;
		}
	    }
	    //printf("count = %d\n", count);
	}

	/* free allocated memory */
	free(row);
	free(colno);
	delete_lp(lp);

    }

    for(unsigned int i = 0; i < m_macro_r.size(); i++)
	m_pDB->SetModuleOrientationCenter(m_macro_r[i], (m_dir[i]%8));

    double mdtime = seconds() - start_time;
    m_mdtime += mdtime;

    printf("%d(%d) macros are rotated\n", count, count_n);
}

void MyNLP::CreatePotentialGrid()
{
    //printf( "Create Potential Grid\n" );
    m_gridPotential.clear(); // remove old values
   
    int realGridHSize = m_potentialGridHSize + m_potentialGridPadding;	// padding
    int realGridVSize = m_potentialGridVSize + m_potentialGridPadding;  // padding
   
    m_newPotential.resize( m_pDB->m_modules.size() );

    m_gridPotential.resize( realGridHSize );
    m_basePotential.resize( realGridHSize );
    //m_binGradX.resize( realGridHSize+1 );
    //m_binGradY.resize( realGridHSize+1 );
    m_weightDensity.resize( realGridHSize );
    for( int i = 0; i < realGridHSize; i++ )
    {
	m_basePotential[i].resize( realGridVSize, 0 );
	m_gridPotential[i].resize( realGridVSize, 0 );
	//m_binGradX[i].resize( realGridVSize+1, 0 );
	//m_binGradY[i].resize( realGridVSize+1, 0 );
	m_weightDensity[i].resize( realGridVSize, 1 );
    }
    //m_binGradX[ realGridHSize ].resize( realGridVSize+1, 0 );
    //m_binGradY[ realGridHSize ].resize( realGridVSize+1, 0 );
    
    m_potentialGridWidth  = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / m_potentialGridHSize;
    m_potentialGridHeight = ( m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom ) / m_potentialGridVSize;

    _potentialRX = m_potentialGridWidth  * _potentialGridR;
    _potentialRY = m_potentialGridHeight * _potentialGridR;
}

void MyNLP::ClearPotentialGrid()
{
    for( unsigned int gx = 0; gx < m_gridPotential.size(); gx++ )
	fill( m_gridPotential[gx].begin(), m_gridPotential[gx].end(), 0.0 );
}

double MyNLP::UpdateExpBinPotential( double util, bool showMsg )
{
    // Default is -1. Spread to the whole chip
    if( util < 0 ) 
	util = 1.0; // use all space

    double totalFree = 0;
    int zeroSpaceBin = 0;
    m_expBinPotential.resize( m_basePotential.size() );
    for( unsigned int i = 0; i < m_basePotential.size(); i++ )
    {
	m_expBinPotential[i].resize( m_basePotential[i].size() );
	for( unsigned int j = 0; j < m_basePotential[i].size(); j++ )
	{
	    double base = m_basePotential[i][j];
	    double overlapX = getOverlap( 
		    GetXGrid(i)-m_potentialGridWidth*0.5, GetXGrid(i)+m_potentialGridWidth*0.5, 
		    m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.right );
	    double overlapY = getOverlap( 
		    GetYGrid(j)-m_potentialGridHeight*0.5, GetYGrid(j)+m_potentialGridHeight*0.5, 
		    m_pDB->m_coreRgn.bottom, m_pDB->m_coreRgn.top );
	    double realBinArea = overlapX * overlapY;
	    double free = realBinArea - base; // reduce max. density bound for fixed cells
	    
	    if( free > 1e-4 )
	    {
		m_expBinPotential[i][j] = free * util;
		totalFree += m_expBinPotential[i][j];
	    }
	    else
	    {
		m_expBinPotential[i][j] = 0.0;
		zeroSpaceBin++;
	    }
	} 
    }

    double algUtil = m_pDB->m_totalMovableModuleArea / totalFree;
    if( param.bShow && showMsg )
    {
	printf( "PBIN: Zero space bin #= %d (%d%%).  Algorithm utilization= %.4f\n", 
		zeroSpaceBin, 100*zeroSpaceBin/m_potentialGridHSize/m_potentialGridVSize,
		algUtil );
    }

    double alwaysOver = 0.0;
    /*if( m_targetUtil > 0.0 && m_targetUtil < 1.0 )
    {
	for( unsigned int i = 0; i < m_pDB->m_modules.size(); i++ )
	{
	    if( m_pDB->m_modules[i].m_isFixed )
		continue;
	    if( m_pDB->m_modules[i].m_width >= 2 * m_potentialGridWidth && 
		m_pDB->m_modules[i].m_height >= 2 * m_potentialGridHeight )
	    {
		alwaysOver += 
		    ( m_pDB->m_modules[i].m_width - m_potentialGridWidth ) * 
		    ( m_pDB->m_modules[i].m_height - m_potentialGridHeight ) * 
		    ( 1.0 - m_targetUtil );
	    }
	}
	if( param.bShow )
	    printf( "PBIN: Always over: %.0f (%.1f%%)\n", alwaysOver, alwaysOver/ m_pDB->m_totalMovableModuleArea*100.0 );
    }*/
    m_alwaysOverPotential = alwaysOver;

    return algUtil;
}

// Level smoothing
void MyNLP::LevelSmoothBasePotential( const double& delta )
{
    if( delta <= 1.0 )
       return;

    vector< vector< double > > oldPotential = m_basePotential;

    double maxPotential = 0;
    double avgPotential = 0;
    double totalPotential = 0;
    for( unsigned int i = 0; i < oldPotential.size(); i++ )
    {
	for( unsigned int j = 0; j < oldPotential[i].size(); j++ )
	{
	    totalPotential += oldPotential[i][j];
	    maxPotential = max( maxPotential, oldPotential[i][j] );
	}
    }
    avgPotential = totalPotential / (oldPotential.size() * oldPotential[0].size() );

    if( totalPotential == 0 )
	return; // no preplaced
    
    // apply TSP-style smoothing
    double newTotalPotential = 0;
    for( unsigned int i = 0; i < m_basePotential.size(); i++ )
    {
	for( unsigned int j = 0; j < m_basePotential[i].size(); j++ )
	{
	    if( oldPotential[i][j] >= avgPotential )
	    {
		m_basePotential[i][j] = 
		    avgPotential + 
		    pow( ( oldPotential[i][j] - avgPotential ) / maxPotential, delta ) * maxPotential;
	    }
	    else
	    {
		m_basePotential[i][j] = 
		    avgPotential - 
		    pow( ( avgPotential - oldPotential[i][j] ) / maxPotential, delta ) * maxPotential;
	    }
	    newTotalPotential += m_basePotential[i][j];
	}
    }
    
    // normalization
    double ratio = totalPotential / newTotalPotential;
    for( unsigned int i = 0; i < m_basePotential.size(); i++ )
    {
	for( unsigned int j = 0; j < m_basePotential[i].size(); j++ )
	{
	    m_basePotential[i][j] = m_basePotential[i][j] * ratio;
	}
    }
}

void MyNLP::UpdatePotentialGridBase( const vector<double>& x )
{
    double binArea = m_potentialGridWidth * m_potentialGridHeight;
    m_binFreeSpace.resize( m_basePotential.size() );
    for( unsigned int i = 0; i < m_basePotential.size(); i++ )
    {
	fill( m_basePotential[i].begin(), m_basePotential[i].end(), 0.0 );
	m_binFreeSpace[i].resize( m_basePotential[i].size() );
	fill( m_binFreeSpace[i].begin(), m_binFreeSpace[i].end(), binArea );
    }

    for( unsigned int i = 0; i < m_pDB->m_modules.size(); i++ ) // for all fixed cells
    {
	// for each cell. cell ci coordinate is ( x[i*2], x[i*2+1] )

	if( m_pDB->m_modules[i].m_isFixed == false )
	    continue;

	if( m_pDB->BlockOutCore( i ) )
	    continue;	// pads?

	double cellX = x[i*2];
	double cellY = x[i*2+1];
	double width  = m_pDB->m_modules[i].m_width;
	double height = m_pDB->m_modules[i].m_height;

	// exact block range
	double left   = cellX - width * 0.5; 
	double bottom = cellY - height * 0.5; 
	double right  = cellX + (cellX - left);
	double top    = cellY + (cellY - bottom);

	if( left   < m_pDB->m_coreRgn.left )     
	    left   = m_pDB->m_coreRgn.left;
	if( right  > m_pDB->m_coreRgn.right )    
	    right  = m_pDB->m_coreRgn.right;
	if( bottom < m_pDB->m_coreRgn.bottom )   
	    bottom = m_pDB->m_coreRgn.bottom;
	if( top    > m_pDB->m_coreRgn.top  )      
	    top    = m_pDB->m_coreRgn.top;

	int gx, gy;
	GetClosestGrid( left, bottom, gx, gy );
	int gxx, gyy;
	double xx, yy;

	double tUtil = 1.0;
	gArg.GetDouble("util", &tUtil);

	xx = GetXGrid(gx);
	if( tUtil < 1.0 ) xx -= m_potentialGridWidth * 0.5;
	// Exact density for the base potential"
	for( gxx = gx/*, xx = GetXGrid(gx) - m_potentialGridWidth * 0.5*/;
		xx <= right && gxx < (int)m_basePotential.size(); 
		gxx++, xx+=m_potentialGridWidth )
	{
	    yy = GetYGrid(gy);
	    if( tUtil < 1.0 ) yy -= m_potentialGridHeight * 0.5;
	    for( gyy = gy/*, yy = GetYGrid(gy) - m_potentialGridHeight * 0.5*/;
	    	    yy <= top && gyy < (int)m_basePotential[gxx].size(); 
		    gyy++, yy+=m_potentialGridHeight )
	    {
		m_basePotential[gxx][gyy] +=
		    getOverlap( left, right, xx, xx+m_potentialGridWidth ) * 
		    getOverlap( bottom, top, yy, yy+m_potentialGridHeight );

		m_binFreeSpace[gxx][gyy] -= 
		    getOverlap( left, right, xx, xx+m_potentialGridWidth ) * 
		    getOverlap( bottom, top, yy, yy+m_potentialGridHeight );
	    }
	}

    } // for each cell
}

// static
void* MyNLP::ComputeNewPotentialGridThread( void* arg )   
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    ComputeNewPotentialGrid( *pMsg->pX, pMsg->pNLP, pMsg->index1, pMsg->index2 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static
void MyNLP::ComputeNewPotentialGrid( const vector<double>& x, MyNLP* pNLP, int index1, int index2 )
{
    if( index2 > (int)pNLP->m_pDB->m_modules.size() )
        index2 = (int)pNLP->m_pDB->m_modules.size();

    for( int i = index1; i < index2; i++ ) // for all movable cells
    {
	if( pNLP->m_pDB->BlockOutCore( i) )
	    continue;

	// preplaced blocks are stored in m_basePotential
	if( pNLP->m_pDB->m_modules[i].m_isFixed )
	    continue;
	
	double cellX = x[i*2];
	double cellY = x[i*2+1];
	double potentialRX = pNLP->_potentialRX;
	double potentialRY = pNLP->_potentialRY;
	double width  = pNLP->m_pDB->m_modules[i].m_width;
	double height = pNLP->m_pDB->m_modules[i].m_height;

	// (kaie) cross potential
	double left, bottom, right, top;
	if(pNLP->m_macroRotate && pNLP->m_pDB->m_modules[i].m_isMacro)
	{
	    left   = cellX - max(width, height) * 0.5 - potentialRX;
	    bottom = cellY - max(width, height) * 0.5 - potentialRY;
	}else
	{
	    left   = cellX - width * 0.5  - potentialRX;
	    bottom = cellY - height * 0.5 - potentialRY;
	}
	right  = cellX + (cellX - left);
	top    = cellY + (cellY - bottom);
	// @(kaie) cross potential
	
	if( left   < pNLP->m_pDB->m_coreRgn.left  - pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth )     
	    left   = pNLP->m_pDB->m_coreRgn.left  - pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
	if( right  > pNLP->m_pDB->m_coreRgn.right + pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth )    
	    right  = pNLP->m_pDB->m_coreRgn.right + pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
	if( bottom < pNLP->m_pDB->m_coreRgn.bottom - pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight )   
	    bottom = pNLP->m_pDB->m_coreRgn.bottom - pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
	if( top    > pNLP->m_pDB->m_coreRgn.top    + pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight )      
	    top    = pNLP->m_pDB->m_coreRgn.top    + pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
	
	int gx, gy;
	pNLP->GetClosestGrid( left, bottom, gx, gy );
	int gxx, gyy;
        double xx, yy;

	double totalPotential = 0;
	vector< potentialStruct >& potentialList = pNLP->m_newPotential[i];     
        potentialList.clear();	

	// (kaie) cross potential
	double hDensity = 1.0, vDensity = 0.0;
	if(pNLP->m_macroRotate && pNLP->m_pDB->m_modules[i].m_isMacro)
	{
	    hDensity = (GetDensityProjection( pNLP->rotate_degree[i], 0.0 ) +
			GetDensityProjection( pNLP->rotate_degree[i], 0.5 ) );
	    //assert(hDensity >= 0 && hDensity <= 1.0);

	    vDensity = 1.0 - hDensity;
	}
	// @(kaie) cross potential

	for( gxx = gx, xx = pNLP->GetXGrid(gx); xx <= right && gxx < (int)pNLP->m_gridPotential.size(); 
		gxx++, xx+=pNLP->m_potentialGridWidth )
	{
	    for( gyy = gy, yy = pNLP->GetYGrid(gy); yy <= top && gyy < (int)pNLP->m_gridPotential[gxx].size(); 
		    gyy++, yy+=pNLP->m_potentialGridHeight )
	    {
		// (kaie) cross potential
		double hPotential = GetPotential( cellX, xx, potentialRX, width,  pNLP->_alpha ) *
				    GetPotential( cellY, yy, potentialRY, height, pNLP->_alpha );
		double vPotential = GetPotential( cellX, xx, potentialRX, height, pNLP->_alpha ) *
				    GetPotential( cellY, yy, potentialRY, width,  pNLP->_alpha );
		double potential = hDensity * hPotential + vDensity * vPotential;
		// @(kaie) cross potential

		if( potential > 0 )
		{
		    totalPotential += potential;
		    potentialList.push_back( potentialStruct( gxx, gyy, potential ) );
		}
	    }
	}

	// normalize the potential so that total potential equals the cell area
	double scale = pNLP->m_pDB->m_modules[i].m_area / totalPotential;
	
	pNLP->_cellPotentialNorm[i] = scale;	    // normalization factor for the cell i
    } // for each cell
}

void MyNLP::UpdatePotentialGrid()
{
    ClearPotentialGrid();
    for( unsigned int i = 0; i < m_newPotential.size(); i++ )	// for each cell
    {
	for( unsigned int j = 0; j < m_newPotential[i].size(); j++ ) // for each bin
	{
	    int gx = m_newPotential[i][j].gx;
	    int gy = m_newPotential[i][j].gy;
	    m_gridPotential[ gx ][ gy ] += m_newPotential[i][j].potential * _cellPotentialNorm[i];
	}
    }
}

double MyNLP::Sigmoid(double alpha, double t)
{
    t = alpha*t;
    if(t > 0) {
	if(t < 0.5f) {
	    double d = t-0.5;
	    return 1-2*d*d;
	} else {
	    return 1;
	}
    } else {
	if(t > -0.5f) {
	    double d = t+0.5;
	    return 2*d*d;
	} else {
	    return 0;
	}
    }
}

double MyNLP::SigmoidGrad(double alpha, double t)
{
    t = alpha*t;
    if(t > 0) {
	if(t < 0.5f) {
	    return -4*alpha*(t-0.5);
	} else {
	    return 0;
	}
    } else {
	if(t > -0.5f) {
	    return 4*alpha*(t+0.5);
	} else {
	    return 0;
	}
    }
}

double MyNLP::GetSigmoidPotential( const double& x1, const double& x2, const double& r, const double& w, const double& alpha )
{
    double d = fabs( x1 - x2 );
    double t = (-d+max(w,r)/2)/max(w,r)/alpha;

    return w*Sigmoid(alpha, t);
}

double MyNLP::GetSigmoidGradPotential( const double& x1, const double& x2, const double& r, const double& w, const double& alpha )
{
    double d = fabs( x1 - x2 );
    double t = (-d+max(w,r)/2)/max(w,r)/alpha;

    if(x1 >= x2)
	return -w*SigmoidGrad(alpha, t)/max(w,r)/alpha;
    else
	return w*SigmoidGrad(alpha, t)/max(w,r)/alpha;
}

double MyNLP::GetPotential( const double& x1, const double& x2, const double& r, const double& w, const double& alpha )
{
    //return GetSigmoidPotential(x1, x2, r, w, alpha);

    double d = fabs( x1 - x2 );
    double a = 4.0 / ( w + r ) / ( w + 2 * r );
    double b = 4.0 / r / ( w + 2.0 * r );
    
    if( d <= w * 0.5 + r * 0.5 )
	return 1.0 - a * d * d;
    else if( d <= w * 0.5 + r )
	return b * ( d - r - w * 0.5 ) * ( d - r - w * 0.5);
    else
	return 0.0;
}

double MyNLP::GetGradPotential( const double& x1, const double& x2, const double& r, const double& w, const double& alpha )
{
    //return GetSigmoidGradPotential(x1, x2, r, w, alpha);

    double d;
    double a = 4.0 / ( w + r ) / ( w + 2.0 * r );
    double b = 4.0 / r / ( w + 2.0 * r );

    if( x1 >= x2 )  // right half
    {
	d = x1 - x2;	// d >= 0
	if( d <= w * 0.5 + r * 0.5 )
	    return -2.0 * a * d;
	else if( d <= w * 0.5 + r )
	    return +2.0 * b * ( d - r - w * 0.5);
	else
	    return 0;
    }
    else    // left half
    {
	d = x2 - x1;	// d >= 0	
	if( d <= w * 0.5 + r * 0.5 )
	    return +2.0 * a * d;
	else if( d <= w * 0.5 + r )
	    return -2.0 * b * ( d - r - w * 0.5);
	else
	    return 0;
    }
}

double MyNLP::GetXGrid( const int& gx )
{
    return m_pDB->m_coreRgn.left + (gx - m_potentialGridPadding + 0.5) * m_potentialGridWidth;  
}

double MyNLP::GetYGrid( const int& gy )
{
    return  m_pDB->m_coreRgn.bottom + (gy - m_potentialGridPadding + 0.5) * m_potentialGridHeight;
}
	
void MyNLP::GetClosestGrid( const double& x1, const double& y1, int& gx, int& gy ) 
{
    double left   = m_pDB->m_coreRgn.left   - m_potentialGridWidth  * m_potentialGridPadding;
    double bottom = m_pDB->m_coreRgn.bottom - m_potentialGridHeight * m_potentialGridPadding;
    gx = static_cast<int>( floor( (x1 - left ) / m_potentialGridWidth ) );
    gy = static_cast<int>( floor( (y1 - bottom ) / m_potentialGridHeight ) );
    
    if( gx < 0 )
	gx = 0;
    if( gy < 0 )
	gy = 0;
    
    if( gx > (int)m_gridPotential.size() )
	gx = (int)m_gridPotential.size() - 1;
    if( gy > (int)m_gridPotential[0].size() )
	gy = (int)m_gridPotential[0].size() - 1;
}

void MyNLP::ClearDensityGrid()
{
    for( unsigned int i = 0; i < m_gridDensity.size(); i++ )
    {
	for( unsigned int j = 0; j < m_gridDensity[i].size(); j++ )
	{
	    m_gridDensity[i][j] = 0.0;
	}
    }
}

void MyNLP::UpdateDensityGridSpace( const vector<double>& x ) // for fixed blocks, find free spaces
{
    double allSpace = m_gridDensityWidth * m_gridDensityHeight;
    for( unsigned int i = 0; i < m_gridDensity.size(); i++ )
    {
	for( unsigned int j = 0; j < m_gridDensity[i].size(); j++ )
	{
	    m_gridDensitySpace[i][j] = allSpace;
	}
    }
    
    // for each cell b, update the corresponding bin area
    for( unsigned int b = 0; b < m_pDB->m_modules.size(); b++ )
    {
	if( false == m_pDB->m_modules[b].m_isFixed )
	    continue;

	double w  = m_pDB->m_modules[b].m_width;
	double h  = m_pDB->m_modules[b].m_height;
	double left   = x[b*2]   - w * 0.5;
	double bottom = x[b*2+1] - h * 0.5;
	double right  = left   + w;
	double top    = bottom + h;

	if( w == 0 || h == 0 )
	    continue;
	
	// find nearest bottom-left gird
	int gx = static_cast<int>( floor( (left   - m_pDB->m_coreRgn.left)   / m_gridDensityWidth ) );
	int gy = static_cast<int>( floor( (bottom - m_pDB->m_coreRgn.bottom) / m_gridDensityHeight ) );
	if( gx < 0 )  gx = 0;
	if( gy < 0 )  gy = 0;
	
	for( unsigned int xOff = gx; xOff < m_gridDensity.size(); xOff++ )
	{
	    double binLeft  = m_pDB->m_coreRgn.left + xOff * m_gridDensityWidth;
	    double binRight = binLeft + m_gridDensityWidth;
	    if( binLeft >= right )
		break;
	    
	    for( unsigned int yOff = gy; yOff < m_gridDensity[xOff].size(); yOff ++ )
	    {
		double binBottom = m_pDB->m_coreRgn.bottom + yOff * m_gridDensityHeight;
		double binTop    = binBottom + m_gridDensityHeight;
		if( binBottom >= top )
		    break;

		m_gridDensitySpace[xOff][yOff] -= 
		    getOverlap( left, right, binLeft, binRight ) * 
		    getOverlap( bottom, top, binBottom, binTop );
	    }
	}

    } // each module

    int zeroSpaceCount = 0;
    m_totalFreeSpace = 0;
    for( unsigned int i = 0; i < m_gridDensity.size(); i++ )
    {
	for( unsigned int j = 0; j < m_gridDensity[i].size(); j++ )
	{
	    if( m_gridDensitySpace[i][j] < 1e-5 )
	    {
		m_gridDensitySpace[i][j] = 0.0;
		zeroSpaceCount ++;
	    }
	    m_totalFreeSpace += m_gridDensitySpace[i][j];
	}
    }
    
    if( param.bShow )
    {
	printf( "DBIN: Zero space bin #= %d.  Total free space= %.0f.\n", zeroSpaceCount, m_totalFreeSpace );
	//printf( "[DB]   total free space: %.0f\n", m_pDB->m_totalFreeSpace );
    }
}

void MyNLP::UpdateDensityGrid( const vector<double>& x ) // for movable blocks
{
    ClearDensityGrid();
    
    // for each cell b, update the corresponding bin area
    for( unsigned int b = 0; b < m_pDB->m_modules.size(); b++ )
    {
	if( m_pDB->BlockOutCore(b) || m_pDB->m_modules[b].m_isFixed )
	    continue;

	double w  = m_pDB->m_modules[b].m_width;
	double h  = m_pDB->m_modules[b].m_height;

	// rectangle range 
	double left   = x[b*2]   - w * 0.5;
	double bottom = x[b*2+1] - h * 0.5;
	double right  = left   + w;
	double top    = bottom + h;

	// find nearest gird
	int gx = static_cast<int>( floor( (left - m_pDB->m_coreRgn.left) / m_gridDensityWidth ) );
	int gy = static_cast<int>( floor( (bottom - m_pDB->m_coreRgn.bottom) / m_gridDensityHeight ) );
	if( gx < 0 ) gx = 0;
	if( gy < 0 ) gy = 0;

	// Block is always inside the core region. Do not have to check boundary.
	for( unsigned int xOff = gx; xOff < m_gridDensity.size(); xOff++ )
	{
	    double binLeft  = m_pDB->m_coreRgn.left + m_gridDensityWidth * xOff;
	    double binRight = binLeft + m_gridDensityWidth;
	    if( binLeft >= right )
		break;
	    
	    for( unsigned int yOff = gy; yOff < m_gridDensity[xOff].size(); yOff++ )
	    {
		double binBottom = m_pDB->m_coreRgn.bottom + m_gridDensityHeight * yOff;
		double binTop    = binBottom + m_gridDensityHeight;
		if( binBottom >= top )
		    break;

		double area = 
		    getOverlap( left, right, binLeft, binRight ) *
		    getOverlap( bottom, top, binBottom, binTop );
		m_gridDensity[xOff][yOff] += area;
	    }
	}
    } // each module
}

void MyNLP::CheckDensityGrid()
{
    double totalDensity = 0;
    for( unsigned int i = 0; i < m_gridDensity.size(); i++ )
    {
	for( unsigned int j = 0; j < m_gridDensity[i].size(); j++ )
	{
	    totalDensity += m_gridDensity[i][j];
	}
    }

    double totalArea = 0;
    for( unsigned int i = 0; i < m_pDB->m_modules.size(); i++ )
    {
	if( m_pDB->BlockOutCore(i) == false )
	    totalArea += m_pDB->m_modules[i].m_area;
    }

    printf( " %f %f\n", totalDensity, totalArea );
}

void MyNLP::CreateDensityGrid( int xGrid, int yGrid )
{
    m_gridDensity.resize( xGrid );
    m_gridDensitySpace.resize( xGrid );
    for( int i = 0; i < xGrid; i++ )
    {
	m_gridDensity[i].resize( yGrid );
	m_gridDensitySpace[i].resize( yGrid );
    }
    
    m_gridDensityWidth  = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / xGrid;
    m_gridDensityHeight = ( m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom ) / yGrid;
    
    double alwaysOver = 0.0;
    /*if( m_targetUtil > 0.0 && m_targetUtil < 1.0 )
    {
	for( unsigned int i = 0; i < m_pDB->m_modules.size(); i++ )
	{
	    if( m_pDB->m_modules[i].m_isFixed )
		continue;
	    if( m_pDB->m_modules[i].m_width >= 2*m_gridDensityWidth &&
	    	m_pDB->m_modules[i].m_height >= 2*m_gridDensityHeight )
	    {
		alwaysOver += 
		    ( m_pDB->m_modules[i].m_width - m_gridDensityWidth ) * 
		    ( m_pDB->m_modules[i].m_height - m_gridDensityHeight ) * 
		    ( 1.0 - m_targetUtil );
	    }
	}
	if( param.bShow )
	    printf( "DBIN: Always over: %.0f (%.1f%%)\n", alwaysOver, alwaysOver/ m_pDB->m_totalMovableModuleArea*100.0 );
    }*/
    m_alwaysOverArea = alwaysOver;
}

double MyNLP::GetMaxDensity()
{
    double maxUtilization = 0;
    double binArea = m_gridDensityWidth * m_gridDensityHeight;
    for( unsigned int i = 0; i < m_gridDensity.size(); i++ )
    {
	for( unsigned int j = 0; j < m_gridDensity[i].size(); j++ )
	{
	    if( m_gridDensitySpace[i][j] > 1e-5 )
	    {
		double preplacedArea = binArea - m_gridDensitySpace[i][j];
		double utilization = ( m_gridDensity[i][j] + preplacedArea ) / binArea;   
		if( utilization > maxUtilization )
		    maxUtilization = utilization;
	    }
	}
    }
    return maxUtilization;
}

double MyNLP::GetTotalOverDensityLB()
{
    double over = 0;
    for( unsigned int i = 0; i < m_gridDensity.size(); i++ )
    {
	for( unsigned int j = 0; j < m_gridDensity[i].size(); j++ )
	{
	    double targetSpace = m_gridDensitySpace[i][j] /** m_targetUtil*/;
	    if( targetSpace > 1e-5 && m_gridDensity[i][j]  > targetSpace  )
    		over += m_gridDensity[i][j] - targetSpace;
	}
    }

    // TODO: remove "1.0"
    return (over /*- m_alwaysOverArea*/) / (m_totalMovableModuleArea) + 1.0;
    //return (over - m_alwaysOverArea) / (m_pDB->m_totalMovableModuleArea-m_pDB->m_totalMovableLargeMacroArea) + 1.0;
}

double MyNLP::GetTotalOverDensity()
{
    double over = 0;
    for( unsigned int i = 0; i < m_gridDensity.size(); i++ )
    {
	for( unsigned int j = 0; j < m_gridDensity[i].size(); j++ )
	{
	    double targetSpace = m_gridDensitySpace[i][j] /** m_targetUtil*/;
	    if( m_gridDensity[i][j] > targetSpace )
    		over += m_gridDensity[i][j] - targetSpace;
	}
    }

    // TODO: remove "1.0"
    return (over /*- m_alwaysOverArea*/) / (m_totalMovableModuleArea) + 1.0;
    //return (over - m_alwaysOverArea) / (m_pDB->m_totalMovableModuleArea-m_pDB->m_totalMovableLargeMacroArea) + 1.0;
}

double MyNLP::GetTotalOverPotential()
{
    double over = 0;
    for( unsigned int i = 0; i < m_gridPotential.size(); i++ )
    {
	for( unsigned int j = 0; j < m_gridPotential[i].size(); j++ )
	{
	    if( m_gridPotential[i][j]  > m_expBinPotential[i][j]  )
    		over += m_gridPotential[i][j] - m_expBinPotential[i][j];
	}
    }

    // TODO: remove "1.0"
    return (over - m_alwaysOverPotential) / (m_totalMovableModuleArea) + 1.0;
    //return (over - m_alwaysOverPotential) / (m_pDB->m_totalMovableModuleArea-m_pDB->m_totalMovableLargeMacroArea) + 1.0;
}

double MyNLP::GetNonZeroDensityGridPercent()
{
    double nonZero = 0;
    for( unsigned int i = 0; i < m_gridDensity.size(); i++ )
    {
	for( unsigned int j = 0; j < m_gridDensity[i].size(); j++ )
	{
	    if( m_gridDensity[i][j] > 0 || m_gridDensitySpace[i][j] == 0 )
		nonZero += 1.0;
	}
    }
    return nonZero / m_gridDensity.size() / m_gridDensity[0].size();
}

double MyNLP::GetNonZeroGridPercent()
{
    double nonZero = 0;
    for( unsigned int i = 0; i < m_gridPotential.size(); i++ )
    {
	for( unsigned int j = 0; j < m_gridPotential[i].size(); j++ )
	{
	    if( m_gridPotential[i][j] > 0 )
		nonZero += 1.0;
	}
    }
    return nonZero / m_gridPotential.size() / m_gridPotential[0].size();
}

double MyNLP::GetMaxPotential()
{
    double maxPotential = 0;
    for( unsigned int i = 0; i < m_gridPotential.size(); i++ )
    {
	for( unsigned int j = 0; j < m_gridPotential[i].size(); j++ )
	{
	    if( m_gridPotential[i][j] > maxPotential )
		maxPotential = m_gridPotential[i][j];
	}
    }
    return maxPotential;
}

double MyNLP::GetAvgPotential()
{
    const double targetPotential = 1.0;
    double avgPotential = 0;
    int overflowCount = 0;
    
    for( unsigned int i = 0; i < m_gridPotential.size(); i++ )
    {
	for( unsigned int j = 0; j < m_gridPotential[i].size(); j++ )
	{
	    if( m_gridPotential[i][j] > targetPotential )
	    {
		overflowCount++;
    		avgPotential += m_gridPotential[i][j];
	    }
	}
    }
    return avgPotential / overflowCount;
}

// Output potential data for gnuplot
void MyNLP::OutputPotentialGrid( string filename )
{
    int stepSize = (int)m_gridPotential.size() / 100;
    if( stepSize == 0 )
	stepSize = 1;
    int hSize = m_gridPotential.size();
    int vSize = m_gridPotential[0].size();
    FILE* out = fopen( filename.c_str(), "w" );
    double binArea = m_potentialGridWidth * m_potentialGridHeight;
    for( int j = 0; j < vSize; j += stepSize )
    {
	for( int i = 0; i < hSize; i += stepSize )
	    fprintf( out, "%.03f ", (m_gridPotential[i][j] + m_basePotential[i][j]) / binArea );
	fprintf( out, "\n" );
    }
    fprintf( out, "\n" );
    fclose( out );
}

// Output potential data for gnuplot
void MyNLP::OutputDensityGrid( string filename )
{
    int stepSize = 1;
    int hSize = m_gridDensity.size();
    int vSize = m_gridDensity[0].size();
    FILE* out = fopen( filename.c_str(), "w" );
    for( int j = 0; j < vSize; j += stepSize )
    {
	for( int i = 0; i < hSize; i += stepSize )
	{
	    double targetSpace = m_gridDensitySpace[i][j] /** m_targetUtil*/;
	    if( m_gridDensity[i][j] > targetSpace )
	    {
		// % overflow
		fprintf( out, "%.03f ", (m_gridDensity[i][j]-targetSpace) / m_pDB->m_totalMovableModuleArea * 100 );
	    }
	    else
	    {
		fprintf( out, "%.03f ", 0.0 );
	    }
	}
	fprintf( out, "\n" );
    }
    fprintf( out, "\n" );
    fclose( out );
}

// 2007-04-02 (donnie) Extracted base potential smoothing
void MyNLP::SmoothBasePotential()
{
    // Gaussian smoothing 
    GaussianSmooth smooth;
    int r = m_smoothR;
    smooth.Gaussian2D( (double)r, 6*r+1 );
    smooth.Smooth( m_basePotential );

    // Level smoothing 
    LevelSmoothBasePotential( double(m_smoothDelta) );

    // Increase the height of bins with density = 1.0
    if( m_smoothDelta != 1 )
	return;

    static vector< vector< double > > moreSmooth;
   
    // Save CPU time. Compute only at the first time.
    if( moreSmooth.size() != m_basePotential.size() )
    {
	moreSmooth = m_basePotential;
	r = m_smoothR * 6;		
	int kernel_size = 5*r;
	if( kernel_size % 2 == 0 )
	    kernel_size++;
	smooth.Gaussian2D( (double)r, kernel_size );
	smooth.Smooth( moreSmooth );
    }

    // Merge basePotential and moreSmooth
    double binArea = m_potentialGridWidth * m_potentialGridHeight;
    double halfBinArea = binArea / 2;
    int changeCount = 0;
    double scale = 3;
    for( unsigned int i = 0; i < moreSmooth.size(); i++ )
    {
	for( unsigned int j = 0; j < moreSmooth[i].size(); j++ )
	{
	    double free = binArea - m_basePotential[i][j];
	    if( free < 1e-4 && moreSmooth[i][j] > halfBinArea ) // no space or high enough
	    {
		m_basePotential[i][j] += (moreSmooth[i][j] - halfBinArea) * scale;
		changeCount++;
	    }	
	}
    }
}

// void MyNLP::PlotGPFigures( char* name )
// {
//     char filename[100];

//     sprintf( filename, "mov%s.plt", name );
//     m_pDB->OutputGnuplotFigureWithZoom( filename, true, false, false, false, false);
//     //                                            cellmove showMsg zoom orient pin
//     m_pDB->m_modules_bak = m_pDB->m_modules;
    
//     sprintf( filename, "fig%s.plt", name );
//     m_pDB->OutputGnuplotFigureWithZoom( filename, false, false, false, false, false);
//     //                                            cellmove showMsg zoom orient pin

//     if( m_topLevel && param.bOutTopPL ) 
//     {
// 	sprintf( filename, "fig%s.pl", name );
// 	m_pDB->OutputPL( filename );	
//     }
    
//     double binArea = m_potentialGridWidth * m_potentialGridHeight;
//     sprintf( filename, "exp%s", name );
//     CMatrixPlotter::OutputGnuplotFigure( m_expBinPotential, filename, filename, 
// 	    binArea, true );
//     sprintf( filename, "pot%s", name );
//     CMatrixPlotter::OutputGnuplotFigure( m_gridPotential, filename, filename,
// 	   binArea, true );

//     sprintf( filename, "util%s", name );
//     CPlaceBin placeBin( *m_pDB );
//     if( param.coreUtil < 1.0 )
// 	placeBin.CreateGrid( m_pDB->m_rowHeight * 10.0, m_pDB->m_rowHeight * 10.0 ); // ispd06-contest bin size
//     else
// 	placeBin.CreateGrid( m_potentialGridHSize, m_potentialGridVSize );
//     placeBin.OutputBinUtil( filename, false );
// }

//@ Brian 2007-04-18 Calculate Net Weight In NLP
double MyNLP::NetWeightCalc( int netDegree )
{
    double netWeight = 1.0;
    switch ( netDegree )
    {
        case 0:
        case 1:
        case 2:
        case 3:
            netWeight = 1.0;
            break;
        case 4:
	    //flute
            //netWeight = 1.0224948875255623721881390593047;
	    netWeight = 1.04108;
            break;
        case 5:
	    //flute
            //netWeight = 1.049758555532227587654839386941;
            netWeight = 1.07653;
	    break;
        case 6:
            //flute
	    //netWeight = 1.0755001075500107550010755001076;
            netWeight = 1.10338;
	    break;
        case 7:
	    //flute
            //netWeight = 1.0996261271167802946998020672971;
            netWeight = 1.14604;
	    break;
        case 8:
	    netWeight = 1.17265;
	    break;
        case 9:
	    netWeight = 1.20709;
	    break;
        case 10:
	    netWeight = 1.23765;
	    break;
        case 11:
	    netWeight = 1.26653;
	    break;
        case 12:
	    netWeight = 1.28944;
	    break;
        case 13:
	    //flute
            //netWeight = 1.1944577161968466316292403248925;
	    netWeight = 1.34034;
            break;
        default:
	    //flute
            //netWeight = 1.405086412814388084867219333989;
            netWeight = 1.37924;
	    break;
    }
    
    return netWeight;
}

void MyNLP::PlotBinForceStrength()
{
    static int count = 1;
    
    vector< vector<double> > binWireForce;
    vector< vector<double> > binSpreadingForce;
    vector< vector<double> > ratio;

    int mergeCount = 5;

    binWireForce.resize( m_gridPotential.size()/mergeCount );
    binSpreadingForce.resize( m_gridPotential.size()/mergeCount );
    ratio.resize( m_gridPotential.size()/mergeCount );
    for( unsigned int i = 0; i < binWireForce.size(); i++ )
    {
	binWireForce[i].resize( m_gridPotential[i].size()/mergeCount, 0 );
	binSpreadingForce[i].resize( m_gridPotential[i].size()/mergeCount, 0 );
	ratio[i].resize( m_gridPotential[i].size()/mergeCount, 0 );
    }

    for( unsigned int i = 0; i < m_pDB->m_modules.size(); i++ )
    {
	if( m_pDB->m_modules[i].m_isFixed )
	    continue;

	double cellX = x[2*i];
	double cellY = x[2*i+1];
	int gx, gy;
	GetClosestGrid( cellX, cellY, gx, gy );

	gx /= mergeCount;
	gy /= mergeCount;
	
	if( gx >= (int)binWireForce.size() )
	    gx  = (int)binWireForce.size() - 1;
	if( gy >= (int)binWireForce.size() )
	    gy  = (int)binWireForce.size() - 1;
	
	if( gx >= (int)binWireForce.size() || gy >= (int)binWireForce.size() )
	{
	    printf( "(%d %d)   (%d)\n", gx, gy, (int)binWireForce.size() );
	}

	//assert( gx >= 0 );
	//assert( gy >= 0 );
	//assert( gx < (int)binWireForce.size() );
	//assert( gy < (int)binWireForce.size() );

	binWireForce[gx][gy] += fabs( grad_wire[ 2*i ] ) + fabs( grad_wire[ 2*i+1 ] );	
	binSpreadingForce[gx][gy] += fabs( grad_potential[ 2*i ] ) + fabs( grad_potential[ 2*i+1 ] );	
    }

    for( unsigned int i = 0; i < ratio.size(); i++ )
    {
	for( unsigned int j = 0; j < ratio[i].size(); j++ )
	{
	    ratio[i][j] = binSpreadingForce[i][j] / binWireForce[i][j];
	}
    }
    
    char fn[255];
    sprintf( fn, "wire%d", count );
    CMatrixPlotter::OutputGnuplotFigure( binWireForce, fn );
    sprintf( fn, "spreading%d", count );
    CMatrixPlotter::OutputGnuplotFigure( binSpreadingForce, fn );
    sprintf( fn, "ratio%d", count );
    CMatrixPlotter::OutputGnuplotFigure( ratio, fn );
    count++;
}

bool MyNLP::InitObjWeights( double wWire )
{    
    int n = 2 * m_pDB->m_modules.size();
    
    _weightWire = _weightDensity = 1.0;     // init values to call eval_grad_f

    //ComputeBinGrad();
    Parallel( eval_grad_f_thread, m_pDB->m_modules.size() );
    
    double totalWireGradient = 0;
    double totalPotentialGradient = 0;

    if( m_weightedForce )
    	WeightedForce( 2*m_pDB->m_modules.size(), grad_wire, grad_potential, weightF );
    
    if( !AdjustForce( n, x, grad_wire, grad_potential ) ) // truncation
    {
	printf("Init: AdjustForce: NaN or Inf\n");
	return false;	// bad values in grad_wire or grad_potential
    }

    if(m_macroRotate) // (kaie) macro rotation
    {
	if(m_weightedForce)
	    WeightedForce( m_pDB->m_modules.size(), grad_rotate, grad_potentialR, weightF );
        
	if( !AdjustForceR( m_pDB->m_modules.size(), grad_rotate, grad_potentialR ) )
	{
	    printf("Init: AdjustForceR: NaN or Inf\n");
	    return false;
	}
    }
    
    for( int i = 0; i < n; i++ ) // for each vector
    {
    	if( isNaN( grad_wire[i] ) )
    	    return false;
	if( fabs( grad_wire[i] ) > DBL_MAX * 0.95 ) 
    	    return false;
    	
    	//assert( fabs( grad_wire[i] ) < DBL_MAX * 0.95 );
    	//assert( !isNaN( grad_wire[i] ) );

	if( isNaN( grad_potential[i] ) )
	    return false;
	if( fabs( grad_potential[i] ) > DBL_MAX * 0.95 )
	    return false;
	
	//assert( fabs( grad_potential[i] ) < DBL_MAX * 0.95 );
	//assert( !isNaN( grad_potential[i] ) );

    	totalWireGradient      += fabs( grad_wire[i] );
    	totalPotentialGradient += fabs( grad_potential[i] );
    }

    if( fabs( totalWireGradient ) > DBL_MAX * 0.95 ) 
	return false;
    if( fabs( totalPotentialGradient ) > DBL_MAX * 0.95 ) 
	return false;
	
    // fix "wire" change "spreading"
    _weightDensity = totalWireGradient / totalPotentialGradient;
    _weightWire = wWire;

    gArg.GetDouble( "weightWire", &_weightWire );

    if( param.bShow )
    {
	printf( "Force strength: %.0f (wire)  %.0f (spreading)\n", totalWireGradient, totalPotentialGradient );
    }	

    assert( wWire == 0 || _weightWire > 0 );
    
    //printf( " INIT: LogSumExp WL= %.0f, gradWL= %.0f\n", totalWL, totalWireGradient );
    //printf( " INIT: DensityPenalty= %.0f, gradPenalty= %.0f\n", density, totalPotentialGradient ); 

    for( unsigned int i = 0; i < m_weightDensity.size(); i++ )
	for( unsigned int j = 0; j < m_weightDensity[i].size(); j++ )
	    m_weightDensity[i][j] = _weightDensity;
    
    return true;
}
  
void MyNLP::UpdateObjWeights()
{
    //_weightWire /= m_weightIncreaseFactor;
    
    _weightDensity *= m_weightIncreaseFactor;

    for( unsigned int i=0; i<m_weightDensity.size(); i++ )
    {
        for( unsigned int j=0; j<m_weightDensity[i].size(); j++ )
        {
	    m_weightDensity[i][j] = m_weightDensity[i][j] * m_weightIncreaseFactor;
        }
    }
}

