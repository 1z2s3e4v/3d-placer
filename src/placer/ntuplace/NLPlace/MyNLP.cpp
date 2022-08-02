#include <cmath>
#include <set>
#include <sys/types.h>
#include <vector>
#include <algorithm>
#include <cfloat>
#include <iostream>
#include <fstream>
#include <climits>
#include <iostream>
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

bool bMulti = false;

bool bFast = true;  // spreading force interpolation

double density;
double gTotalWL;

double time_wire_force = 0;
double time_spreading_force = 0;
double time_via_force = 0; // kaie

// static variables
bool   MyNLP::m_bXArch = false;
double MyNLP::m_yWeight = 1.0;
vector< vector< vector< double > > > MyNLP::m_weightDensity; // (kaie) 2009-09-10 3d
double MyNLP::m_skewDensityPenalty1 = 1.0;
double MyNLP::m_skewDensityPenalty2 = 1.0;

/* Constructor. */
    MyNLP::MyNLP( CPlaceDB& db )
: _potentialGridR( 2 ),
    m_potentialGridSize( -1 ),
    m_pGRMap( NULL )
{

    if( gArg.CheckExist( "multi" ) )
	bMulti = true;

    if( gArg.CheckExist( "nointerpolate" ) )
	bFast = false;
    
    gArg.GetDouble( "skew", &m_skewDensityPenalty1 );
    gArg.GetDouble( "skew2", &m_skewDensityPenalty2 );
    
    m_precision = 0.99999;
    m_weightIncreaseFactor = 2.0;
    m_targetUtil = 1.0;
    gArg.GetDouble( "util", &m_targetUtil );
    m_bRunLAL = true;
    m_binSize = 0.8; 
    m_congWeight = 1.0;
    m_potentialGridPadding = 0;
    m_useEvalF = true;
    m_bXArch = false;
    m_spreadingForceNullifyRatio = 0.0;

    m_smoothR = 5;	// Gaussian smooth R
    if( param.b3d ) // (kaie)
	m_smoothR = 1;

    if( gArg.CheckExist( "nolal" ) )
	m_bRunLAL = false;
    
    m_maxIte = 50;	// max outerIte
    gArg.GetInt( "maxIte", &m_maxIte );    

    if( param.bShow )
    {
	gArg.GetDouble( "precision",  &m_precision );
	gArg.GetDouble( "incFactor",  &m_weightIncreaseFactor );
	gArg.GetDouble( "binSize",    &m_binSize );
	gArg.GetDouble( "congWeight", &m_congWeight );
	gArg.GetDouble( "yWeight",    &m_yWeight );
	gArg.GetDouble( "nullifyRatio",    &m_spreadingForceNullifyRatio );
	gArg.GetInt( "padding",    &m_potentialGridPadding );
	gArg.GetInt( "smoothr", &m_smoothR );
	m_bXArch   = gArg.CheckExist( "x" );
	m_useEvalF = !gArg.CheckExist( "evalhpwl" );

	printf( "\n" );
	printf( "[Analytical Placement Parameters]\n" );
	printf( "    solver precision            = %g\n", m_precision );
	printf( "    weight increasing factor    = %g\n", m_weightIncreaseFactor );
	printf( "    target utilization          = %g\n", m_targetUtil );
	printf( "    bin size factor             = %g\n", m_binSize );
	printf( "    G-smooth r                  = %d\n", m_smoothR );

	if( m_bXArch )
	    printf( "    use X-arch wire model       = %s\n", TrueFalse( m_bXArch ).c_str() );

	if( m_bRunLAL )
	    printf( "    use look-ahead legalization = %s\n", TrueFalse( m_bRunLAL ).c_str() );

	printf( "    congestion weight           = %.2f\n", m_congWeight );

	if( m_yWeight != 1 )
	    printf( "    vertical weight             = %.2f\n", m_yWeight );

	if( m_potentialGridPadding > 0 )
	    printf( "    potential grid padding      = %d\n", m_potentialGridPadding );

	if( m_spreadingForceNullifyRatio > 0 )
	    printf( "    null spreading force ratio  = %.2f\n", m_spreadingForceNullifyRatio );
	printf( "\n" );
    }

    m_lookAheadLegalization = false;
    m_earlyStop = false;
    m_topLevel = false;
    m_useBellPotentialForPreplaced = true;

    m_weightWire = 4.0;
    m_weightTSV = 0.0;
    //gArg.GetDouble("TSV", &m_weightTSV);
    
    m_smoothDelta = 1;

    m_pDB = &db;
    InitModuleNetPinId();	    

    if(param.b3d) // (kaie) number of layers
    	m_pDB->m_totalLayer = param.nlayer;
    else
		m_pDB->m_totalLayer = 1;
    if(param.b3d && !gArg.CheckExist("noZ"))
		m_bMoveZ = true;
    else
		m_bMoveZ = false;

    // scale between 0 to 10
    const double range = 10.0;
    if( m_pDB->m_coreRgn.right > m_pDB->m_coreRgn.top )
	m_posScale = range / m_pDB->m_coreRgn.right;	
    else
	m_posScale = range / m_pDB->m_coreRgn.top;

    _cellPotentialNorm.resize( m_pDB->m_modules.size() );

    x.resize( 2 * m_pDB->m_modules.size() );
    //cellLock.resize( m_pDB->m_modules.size(), false );
    xBest.resize( 2 * m_pDB->m_modules.size() );
    _expX.resize( 2 * m_pDB->m_modules.size() );
    _expPins.resize( 2 * m_pDB->m_pins.size() );
    x_l.resize( 2 * m_pDB->m_modules.size() );
    x_u.resize( 2 * m_pDB->m_modules.size() );
	xMax.resize( m_pDB->m_modules.size() );
	yMax.resize( m_pDB->m_modules.size() );

    // (kaie) 2009-09-12 add z direction
    z.resize( m_pDB->m_modules.size() );
    if(m_bMoveZ)
    {
    	grad_f.resize( 3 * m_pDB->m_modules.size() );
    	last_grad_f.resize( 3 * m_pDB->m_modules.size() );
    	walk_direction.resize(3 * m_pDB->m_modules.size(), 0);
    	last_walk_direction.resize(3 * m_pDB->m_modules.size(), 0);

    // kaie 2009-08-29 3D placement
    	zBest.resize( m_pDB->m_modules.size() );
    	_expZ.resize( m_pDB->m_modules.size() );
    	_expPinsZ.resize( m_pDB->m_pins.size() );
		zMax.resize( m_pDB->m_modules.size() );
    }else
    {
		grad_f.resize( 2 * m_pDB->m_modules.size() );
		last_grad_f.resize( 2 * m_pDB->m_modules.size() );
		walk_direction.resize( 2 * m_pDB->m_modules.size(), 0);
		last_walk_direction.resize(2 * m_pDB->m_modules.size(), 0);
    }
    // @kaie 2009-08-29

    if( m_bXArch ) // 2006-09-12 (donnie)
    {	
		_expXplusY.resize( m_pDB->m_modules.size() );   // exp( x+y /k )  for cells
		_expXminusY.resize( m_pDB->m_modules.size() );  // exp( x-y /k )  for cells
		_expPinXplusY.resize( m_pDB->m_pins.size() );   // exp( x+y /k )  for pins
		_expPinXminusY.resize( m_pDB->m_pins.size() );  // exp( x-y /k )  for pins
    }

    /* 
       for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
       {
       if( m_pDB->m_modules[i].m_isFixed )
       cellLock[ i ] = true;
       }
       */

    m_usePin.resize( m_pDB->m_modules.size() );
    SetUsePin();

    m_nets_sum_exp_xi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_exp_yi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_exp_inv_xi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
    m_nets_sum_exp_inv_yi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
	// frank 2022-07-23 3d
	if(param.b3d){
		m_layer_nets_sum_exp_xi_over_alpha.resize( param.nlayer, vector<double>(m_pDB->m_nets.size(), 0) );
		m_layer_nets_sum_exp_yi_over_alpha.resize( param.nlayer, vector<double>(m_pDB->m_nets.size(), 0) );
		m_layer_nets_sum_exp_inv_xi_over_alpha.resize( param.nlayer, vector<double>(m_pDB->m_nets.size(), 0) );
		m_layer_nets_sum_exp_inv_yi_over_alpha.resize( param.nlayer, vector<double>(m_pDB->m_nets.size(), 0) );
	}
    
    // kaie 2009-08-29 3d placement
    if(m_bMoveZ)
    {
    	m_nets_sum_exp_zi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
    	m_nets_sum_exp_inv_zi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
    }
    // @kaie 2009-08-29

    // (kaie) 2010-10-18 Weighted-Average-Exponential Wirelength Model
    if( param.bUseWAE )
    {
	    m_nets_weighted_sum_exp_xi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
	    m_nets_weighted_sum_exp_yi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
	    m_nets_weighted_sum_exp_inv_xi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
	    m_nets_weighted_sum_exp_inv_yi_over_alpha.resize( m_pDB->m_nets.size(), 0 );
		if(param.b3d){
			vector<double> tmp_v;
			tmp_v.resize( m_pDB->m_nets.size(), 0 );
			m_layer_nets_weighted_sum_exp_xi_over_alpha.resize(param.nlayer, tmp_v);
			m_layer_nets_weighted_sum_exp_yi_over_alpha.resize(param.nlayer, tmp_v);
			m_layer_nets_weighted_sum_exp_inv_xi_over_alpha.resize(param.nlayer, tmp_v);
			m_layer_nets_weighted_sum_exp_inv_yi_over_alpha.resize(param.nlayer, tmp_v);
		}
	}

    if( m_bXArch ) // 2006-09-12 (donnie)
    {
		m_nets_sum_exp_x_plus_y_over_alpha.resize( m_pDB->m_nets.size(), 0 );
		m_nets_sum_exp_x_minus_y_over_alpha.resize( m_pDB->m_nets.size(), 0 );
		m_nets_sum_exp_inv_x_plus_y_over_alpha.resize( m_pDB->m_nets.size(), 0 );
		m_nets_sum_exp_inv_x_minus_y_over_alpha.resize( m_pDB->m_nets.size(), 0 );
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

	// kaie 2009-08-29 3d placement
	if(m_bMoveZ)
	{
	    m_nets_sum_p_z_pos.resize( m_pDB->m_nets.size(), 0 );
	    m_nets_sum_p_inv_z_pos.resize( m_pDB->m_nets.size(), 0 );
	    m_nets_sum_p_z_neg.resize( m_pDB->m_nets.size(), 0 );
	    m_nets_sum_p_inv_z_neg.resize( m_pDB->m_nets.size(), 0 );
	}
	// @kaie 2009-08-29
	
    }

    grad_wire.resize( 2 * m_pDB->m_modules.size(), 0.0 );
    grad_potential.resize( 2 * m_pDB->m_modules.size(), 0.0 );
	    
    // kaie 2009-08-29 TSVs
    if(m_bMoveZ)
    {
	grad_potentialZ.resize( m_pDB->m_modules.size(), 0.0 ); // (kaie) potential in z direction
	grad_via.resize( m_pDB->m_modules.size(), 0.0);
    }
    // @kaie 2009-08-29
    
    /*
       m_totalMovableModuleArea = 0;
       m_totalFixedModuleArea = 0;
       for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
       {
       if( m_pDB->m_modules[i].m_isFixed == false )
       m_totalMovableModuleArea += m_pDB->m_modules[i].m_area;
       else
       {
       if( m_pDB->m_modules[i].m_isOutCore == false )
       {/
       m_totalFixedModuleArea += 
       getOverlap( m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.right,
       m_pDB->m_modules[i].m_x, m_pDB->m_modules[i].m_x + m_pDB->m_modules[i].m_width ) *
       getOverlap( m_pDB->m_coreRgn.bottom, m_pDB->m_coreRgn.top,
       m_pDB->m_modules[i].m_y, m_pDB->m_modules[i].m_y + m_pDB->m_modules[i].m_height ); 
       }	
       }
       }
       if( param.bShow )
       printf( "Total movable area %.0f, fixed area %.0f\n", 
       m_totalMovableModuleArea, m_totalFixedModuleArea );
       */

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

#if 0
int MyNLP::LockBlocks()
{
    // lock blocks in high density bins
    int count = 0;
    int binCount = 0;
    for( unsigned int xx=0; xx<m_gridDensity.size(); xx+=1 )
    {
	for( unsigned int yy=0; yy<m_gridDensity.size(); yy+=1 )
	{
	    double targetSpace = m_gridDensitySpace[xx][yy] * m_targetUtil;
	    if( m_gridDensity[xx][yy] >= targetSpace )
	    {
		binCount++;

		double lx = m_pDB->m_coreRgn.left + xx * m_gridDensityWidth;
		double rx = lx + m_gridDensityWidth;
		double ly = m_pDB->m_coreRgn.bottom + yy * m_gridDensityHeight;
		double ry = ly + m_gridDensityHeight;

		for( unsigned v=0; v<cellLock.size(); v++ )
		{
		    if( cellLock[v] == false && 
			    x[2*v] >= lx && x[2*v] <= rx && 
			    x[2*v+1] >= ly && x[2*v+1] <= ry )
		    {
			cellLock[v] = true;
			//m_pDB->m_modules[v].m_isFixed = true;
			count++;
		    }
		}
	    }
	}
    }
    //printf( "   Add lock = %d\n", count );

    int lockCount;
    lockCount = 0;
    for( unsigned int i=0; i<cellLock.size(); i++ )
	if( cellLock[i] )
	    lockCount++;
    printf( "   Lock # = %d (%d)   bin = %.2f%%\n", 
	    lockCount, count, (double)binCount / m_gridDensity.size() / m_gridDensity.size() );
    return lockCount;
}
#endif

#if 0 
// 2006-04-27
bool MyNLP::BlockMoving()
{
    printf( "W-Force block shifting\n" );

    //int n, m, nnz_jac_g, nnz_h_lag;
    //get_nlp_info( n, m, nnz_jac_g, nnz_h_lag );
    int n = m_pDB->m_modules.size() * 2;
    get_bounds_info( n, x_l, x_u );
    get_starting_point( x );
    Parallel( BoundXThread, n );
    _alpha = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) * 0.005;	// as small as possible

    m_potentialGridSize = static_cast<int>( sqrt(static_cast<double>( m_pDB->m_modules.size()) ) * param.binSize );
    m_potentialGridWidth  = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / m_potentialGridSize;
    m_potentialGridHeight = ( m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom ) / m_potentialGridSize;
    //CreatePotentialGrid();   // create potential grid according to "m_potentialGridSize"
    CreateDensityGrid( m_potentialGridSize );
    _weightDensity = 0.0;
    _weightWire = 1.0;

    m_currentStep = param.step * 0.1;


    UpdateDensityGridSpace( n, x );
    UpdateDensityGrid( n, x );
    //    LockBlocks();

    double obj_value;
    UpdateExpValueForEachCellThread, n );
    Parallel( UpdateExpValueForEachPinThread, n );
    Parallel( UpdateNetsSumExpThread, (int)m_pDB->m_nets.size() );
    //UpdateExpValueForEachCell( n, x, _expX, _alpha );
    //UpdateExpValueForEachPin( n, x, _expPins, _alpha );
    //UpdateNetsSumExp( x, _expX, this );
    totalWL = GetLogSumExpWL( n, x, _expX, _alpha );

    eval_f( n, x, _expX, true, obj_value );
    printf( "init f = %g\n", obj_value );
    m_pDB->ShowDensityInfo();

    for( int l=0; l<400; l++ )
    {	
	eval_grad_f( n, x, _expX, true, grad_f );
	AdjustForce( n, x, grad_wire );
	LineSearch( n, x, grad_wire, stepSize );
	//printf( "step size = %f\n", stepSize );

	double move;
	double totalMove = 0;
	for( int i=0; i<n; i++ )
	{
	    move = grad_wire[i] * stepSize;
	    //x[i] += move;
	    x[i] -= move;
	    totalMove += move;
	}

	//printf( " move = %g\n", totalMove );

	Parallel( BoundXThread, n );
	Parallel( UpdateExpValueForEachCellThread, n );
	Parallel( UpdateExpValueForEachPinThread, n );
	Parallel( UpdateNetsSumExpThread, (int)m_pDB->m_nets.size() );
	//UpdateExpValueForEachCell( n, x, _expX, _alpha );
	//UpdateExpValueForEachPin( n, x, _expPins, _alpha );
	//UpdateNetsSumExp( x, _expX, this );
	eval_f( n, x, _expX, true, obj_value );
	printf( "%d     f = %g\n", l, obj_value );

	if( l % 5 == 0 )
	{
	    UpdateDensityGrid( n, x );
	    //	    if( LockBlocks() == m_pDB->m_modules.size() )
	    //		break;
	    UpdateBlockPosition( x );
	    m_pDB->ShowDensityInfo();
	    printf( "HPWL= %.0f\n", m_pDB->CalcHPWL() );

	    char filename[100]; 
	    sprintf( filename, "bm%d", l );
	    CPlaceBin placeBin( *m_pDB );
	    placeBin.CreateGrid( m_potentialGridSize );
	    placeBin.OutputBinUtil( filename, false );

	    sprintf( filename, "bmfig%d.plt", l );
	    m_pDB->OutputGnuplotFigure( filename, false );
	}

    }

    UpdateBlockPosition( x );
    m_pDB->ShowDensityInfo();
    printf( "HPWL= %.0f\n", m_pDB->CalcHPWL() );


    return false;   // not legal    
} 
#endif

bool MyNLP::MySolve( double wWire, 
	double target_density, 
	int currentLevel	// for plotting
	)
{
    //Added by Jin 20070305
    //if( param.bShow && gArg.CheckExist( "congopt" ) )
    //{
    //m_fixed_point_method.AddAllPseudoModuleAndNet();
    //}
    //@Added by Jin 20070305

    double time_start = seconds();    
    double time_start_real = seconds_real();    
    assert( _potentialGridR > 0 );

    if( m_potentialGridSize <= 0 )
    {
	//m_potentialGridSize = static_cast<int>( sqrt(static_cast<double>( m_pDB->m_modules.size()) ) * m_binSize );
	//m_potentialGridSize = static_cast<int>( pow(static_cast<double>( m_pDB->m_modules.size()), 1.0/3.0) * m_binSize );
	m_potentialGridSize = static_cast<int>( sqrt(static_cast<double>( m_pDB->m_modules.size()) ) * m_binSize / sqrt(static_cast<double>( m_pDB->m_totalLayer)) ) ; // (kaie) 2009-09-10 3d placement
	//m_potentialGridSize = static_cast<int>( sqrt(static_cast<double>( m_pDB->m_modules.size() / m_pDB->m_totalLayer )) * m_binSize);
	//m_potentialGridSize = static_cast<int>( sqrt(static_cast<double>( m_pDB->m_modules.size() ) / m_pDB->m_totalLayer ) * m_binSize);
	//m_potentialGridSize = static_cast<int>( pow(m_potentialGridSize, 2.0/3.0) );

	// if( currentLevel > 1 && param.bShow && gArg.CheckExist( "dede" ) )
	// {
	//     const int orig_nModule = m_pDB->m_pSteinerDecomposition->m_orig_nModules;
	//     fprintf( stdout, "orig_nModule: %d\n", orig_nModule );
	//     fflush( stdout );
	//     m_potentialGridSize = static_cast<int>( 
	// 	    sqrt( static_cast<double>( orig_nModule ) ) * m_binSize );
	// }
#if 0	
	if( currentLevel > 1 && param.bShow && gArg.CheckExist( "congopt" ) )
	{
	    const int orig_nModule = m_pDB->m_pFixedPointMethod->m_orig_modules.size();
	    fprintf( stdout, "orig_nModule: %d\n", orig_nModule );
	    fflush( stdout );
	    m_potentialGridSize = static_cast<int>( 
		    sqrt( static_cast<double>( orig_nModule ) ) * m_binSize );
	}
#endif
    }

    int n, m, nnz_jac_g, nnz_h_lag;
    get_nlp_info( n, m, nnz_jac_g, nnz_h_lag );
    get_bounds_info( n, x_l, x_u );
    Parallel( BoundXThread, m_pDB->m_modules.size() );
    if(m_bMoveZ)
    	Parallel( BoundZThread, m_pDB->m_modules.size() );

    m_ite = 0;
    bool isLegal = false;

    assert( param.dLpNorm_P > 0 );
    if( param.bUseLSE )
    {
	//_alpha = 0.5 * m_potentialGridWidth; // according to APlace ispd04
	//double maxValue = param.dLpNorm_P;	// > 700 leads to overflow
	_alpha = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / 2 / param.dLpNorm_P;
	//_alpha = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) * 0.005;	// as small as possible -- NLP (for contest)
    }
    else
    {
	// Lp-norm
	//_alpha = param.dLpNorm_P;
	_alpha = log10( DBL_MAX ) ;
    }

    if( param.bShow )
	printf( "GRID = %d  (width = %.2f)  alpha= %f  weightWire= %f\n", 
		m_potentialGridSize, 
		( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left )/m_potentialGridSize, 
		_alpha, wWire );

	m_pDB->UpdatePinNetId();
    int counter = 0;
    while( true )
    {
		counter++;

		// save the block position
		vector< CPoint > blockPositions;
		CPlaceUtil::SavePlacement( *m_pDB, blockPositions );
		
		get_starting_point( x, z );
		Parallel( BoundXThread, m_pDB->m_modules.size() );
		
		if(m_bMoveZ) // kaie z-direction move
			Parallel( BoundZThread, m_pDB->m_modules.size() );

		bool succ = GoSolve( wWire, target_density, currentLevel, &isLegal );

		if( succ )
		{
			//Added by Jin 20070305
			//if( param.bShow && gArg.CheckExist( "congopt" ) )
			//{
			//	m_fixed_point_method.RestoreAllPseudoModuleAndNet();
			//}
			//@Added by Jin 20070305
			break;
		}
		_alpha -= 5;
		CPlaceUtil::LoadPlacement( *m_pDB, blockPositions );

		if( param.bShow )
			printf( "\nFailed to solve it. alpha = %f\n\n", _alpha );
		if( _alpha < 80 )
		{
			printf( "\nFailed to solve it. alpha = %f\n\n", _alpha );
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
    }

    //Added by Jin 20070305
    //if( param.bShow && gArg.CheckExist( "congopt" ) )
    //{
    //	m_fixed_point_method.RestoreAllPseudoModuleAndNet();
    //}
    //@Added by Jin 20070305

    return isLegal;
}


bool MyNLP::GoSolve( double wWire, 
	double target_density, 
	int currentLevel,	// for plotting
	bool* isLegal
	)
{

    *isLegal = false;

    time_wire_force = time_spreading_force = time_via_force = 0.0;

    double givenTargetUtil = m_targetUtil; // for look ahead legalization

    ////////////////////////////////////////
    double spreadingExtraReduction = 0.0;
    gArg.GetDouble( "spreadingReduction", &spreadingExtraReduction );
    m_targetUtil -= spreadingExtraReduction;
    ////////////////////////////////////////
	
    m_currentStep = param.step;
	m_currentStepZ = param.stepZ;

    m_targetUtil += param.targetDenOver;
    if( m_targetUtil > 1.0 )
		m_targetUtil = 1.0;

    double time_start = seconds();    
    char filename[100];	    // for gnuplot

    int n;
    if(m_bMoveZ)
		n = 3 * m_pDB->m_modules.size(); // (kaie) 2009-09-12 add z direction
    else
		n = 2 * m_pDB->m_modules.size();

    double designUtil = m_pDB->m_totalMovableModuleVolumn / m_pDB->m_totalFreeSpace;

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
			if( gArg.CheckExist( "forceLowUtil" ) == false )
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

    CreatePotentialGrid();   // Create potential grid according to "m_potentialGridSize"
    
    int densityGridSize = m_potentialGridSize / 4;   // Use larger grid for density computing
    CreateDensityGrid( densityGridSize );
    UpdateDensityGridSpace( n, x, z );

    UpdatePotentialGridBase( x, z );

    //Brian 2007-06-18
    if (true == param.bFlatLevelCong )
    {
		if (true == m_topLevel)
			param.bCongObj = true;
		else
			param.bCongObj = false;
    }
    //@Brian 2007-06-18
    
    //Added by Jin 20081013
    if( true == m_topLevel )
    {
		if( gArg.CheckExist( "timing" ) )
		{
			param.bTiming = true;
		}
    }	
    //@Added by Jin 20081013

    //Brian 2007-04-30
    // if (true == param.bCongObj)
    // {
	// bMulti = false;
	// m_dCongUtil = 1.0;
	// CreatePotentialGridNet();
	// CreateDensityGridNet( densityGridSize );
	// m_NetBoxInfo.resize( m_pDB->m_nets.size() );
	// grad_congestion.resize( m_pDB->m_modules.size() * 2, 0.0 );
	// CatchCongParam();
	// UpdatePotentialGridBaseNet();
	// SmoothBasePotentialNet();
	// UpdateExpBinPotentialNet();
    // }
    //@Brian 2007-04-30

    //Added by Jin 20081013
    //Prepare the timing data of the first time
    // if( true == param.bTiming )
    // {
	// m_pDB->m_pTimingAnalysis->RerunSTA();	
	// grad_timing.resize( m_pDB->m_modules.size() * 2, 0.0 );
    // }
    //@Added by Jin 20081013
    
    // if( gArg.CheckExist( "congPotential" ) )
    // {
	// m_basePotentialOld = m_basePotential;
	// UpdateCongestionBasePotential();    // 2007-04-02 (donnie) Congestion-driven base potential
    // }

    //SmoothBasePotential3D();
    UpdateExpBinPotential( m_targetUtil, true );
    
    // if( gArg.CheckExist( "congExpPotential" ) )
    // {
	// m_expBinPotentialOld = m_expBinPotential; 
	// UpdateCongestion(); 
    // }	

    assert( m_targetUtil > 0 );

    // wirelength
    Parallel( UpdateExpValueForEachCellThread, m_pDB->m_modules.size() );
    Parallel( UpdateExpValueForEachPinThread, m_pDB->m_pins.size() );

    Parallel( UpdateNetsSumExpThread, (int)m_pDB->m_nets.size() );
    /*double totalWL = */GetLogSumExpWL( x, _expX, _alpha, this );
    if(m_bMoveZ)
    	GetLogSumExpVia( z, _expZ, _alpha, this );
    // density
    Parallel( ComputeNewPotentialGridThread, m_pDB->m_modules.size() );
    UpdatePotentialGrid( z );
    UpdateDensityGrid( n, x, z );
    density = GetDensityPanelty();

    //Brian 2007-04-23
    if (true == param.bCongObj)
    {
	ComputeNetBoxInfo();
	ComputeNewPotentialGridNet();
	UpdatePotentialGridNet();
	UpdateDensityGridSpaceNet();
	UpdateDensityGridNet();
	//GetDensityPaneltyNet();
	UpdateGradCong();
    }
    m_dWeightCong = 1.0;
    //@Brian 2007-04-23
    if( !InitObjWeights( wWire ) )
    {
		printf("InitObjWeight OVERFLOW!\n");
		return false;// overflow
    }
    int maxIte = m_maxIte;
    bool newDir = true;
    double obj_value;
    eval_f( n, x, _expX, true, obj_value );
    ComputeBinGrad();
    Parallel( eval_grad_f_thread, m_pDB->m_modules.size() );
    UpdateDensityGrid( n, x, z );
    double maxDen = GetMaxDensity();
    double lastMaxDen = maxDen; 
    double totalOverDen = GetTotalOverDensity();
    double totalOverDenLB = GetTotalOverDensityLB();
    double totalOverPotential = GetTotalOverPotential();
    //Brian 2007-04-30
    double totalOverDenNet = GetTotalOverDensityNet();
    double totalOverPotentialNet = GetTotalOverPotentialNet();
    double maxDenNet = GetMaxDensityNet();
    double lastTotalOverNet = 0;
    double lastTotalOverPotentialNet = DBL_MAX;
    double lastMaxDenNet = maxDenNet; 
    double overNet = totalOverDenNet;
    //@Brian 2007-04-30

    if( obj_value > DBL_MAX * 0.5 ){
		printf("Objective value OVERFLOW!\n");
		return false; // overflow
    }

    if( param.bShow )
    {	
		printf( " %d-%2d HPWL= %.0f\tDen= %.2f %.4f %.4f %.4f Dcost= %4.1f%%  ",  
			currentLevel, m_ite, m_pDB->CalcHPWL(), 
			maxDen, totalOverDen, totalOverDenLB, totalOverPotential,
			density * _weightDensity / obj_value * 100.0 ); 
    }
    else
    {
		printf( " %d-%2d HPWL= %.0f \t", currentLevel, m_ite, m_pDB->CalcHPWL() );
    }
    fflush( stdout );

    double lastTotalOver = 0;
    double lastTotalOverPotential = DBL_MAX;
    double over = totalOverDen;
    int totalIte = 0;

    bool hasBestLegalSol = false;
    double bestLegalWL = DBL_MAX;
    int lookAheadLegalCount = 0;
    double totalLegalTime = 0.0;

    bool startDecreasing = false;

    int checkStep = 5;

    int outStep = 50;
    gArg.GetInt( "outputStep", &outStep );
    if( param.bShow == false )
	outStep = INT_MAX;
    
    int forceBreakLoopCount = INT_MAX;
    gArg.GetInt( "forceBreakLoopCount", &forceBreakLoopCount );
    if( m_topLevel == false )
	forceBreakLoopCount = INT_MAX;

    // Legalization related configurations
    //int tetrisDir = 0;	// 0: both   1: left   2: right	
    int LALnoGoodCount = 0;
    int maxNoGoodCount = 2;
    if( param.bPrototyping )
	maxNoGoodCount = 0;

    vector<Module> bestGPresult;	// for LAL

    static double lastHPWL = 0;	// test
    static double lastTSV = 0; // kaie
    if( currentLevel > 1 )
    {
		lastHPWL = DBL_MAX;
		lastTSV = DBL_MAX; // kaie
    }

    int congestionIteration = 15;
    if (gArg.CheckExist( "congExpPotential" ) || gArg.CheckExist( "congPotential" ))
		gArg.GetInt( "congIte", &congestionIteration );

    if( gArg.CheckExist( "gpfig" ) )
    {
		m_pDB->m_modules_bak = m_pDB->m_modules;

		// char postfix[10];
		// sprintf( postfix, "%d-%d", currentLevel, m_ite );
		// PlotGPFigures( postfix );

		char fn[255];
		sprintf( fn, "base%d", currentLevel );
		/*CMatrixPlotter::OutputGnuplotFigure( m_basePotential, fn, 
			"", // title
			m_potentialGridWidth * m_potentialGridHeight,  // limit
			true, // scale 
			0 );  // limit base
			*/ // comment by kaie 2009-10-09 unused in 3d
    }

    newDir = true;
    bool bUpdateWeight = true;
    for( int ite=0; ite<maxIte; ite++ )
    {
		m_ite++;
		int innerIte = 0;
		double old_obj = DBL_MAX;
		double last_obj_value = DBL_MAX;

		m_currentStep = param.step;

		if( bUpdateWeight == false )
			newDir = false;
		else
			newDir = true;
		bUpdateWeight = true;

		double lastDensityCost = density;   // for startDecreasing determination
		while( true )	// inner loop, minimize "f" 
		{
			innerIte++;
			swap( last_grad_f, grad_f );    // save for computing the congujate gradient direction
			swap( last_walk_direction, walk_direction );

			// Intra-iteration update
			// if (gArg.CheckExist( "congExpPotential" ) || gArg.CheckExist( "congPotential" ))
			// {
			// if( innerIte % congestionIteration == 0 )
			// {
			//     Parallel( UpdateBlockPositionThread, m_pDB->m_modules.size() );
			//     if( gArg.CheckExist( "congExpPotential" ) )
			// 	UpdateCongestion();
			//     else if( gArg.CheckExist( "congPotential" ) )
			//     {
			// 	UpdateCongestionBasePotential();    
			// 	SmoothBasePotential3D();
			// 	UpdateExpBinPotential( m_targetUtil, true );
			//     }
			// }
			// }

			//Brian 2007-04-30
			if (param.bCongObj)
			UpdateGradCong();
			//@Brian 2007-04-30

			ComputeBinGrad();
			Parallel( eval_grad_f_thread, m_pDB->m_modules.size() );

			if( !AdjustForce( n, x, grad_f ) ){
				printf("AdjustForce, NaN or Inf\n");
				return false;	// NaN or Inf
			}

			if( innerIte % checkStep == 0 )
			{
				if( m_useEvalF )
				{
					old_obj = last_obj_value;    // backup the old value
					if(m_bMoveZ)
						LayerAssignment();
						//Parallel( LayerAssignmentThread, m_pDB->m_modules.size() );
					Parallel( UpdateBlockPositionThread, m_pDB->m_modules.size() );
					//m_pDB->CalcHPWL();
					//m_pDB->CalcTSV();
					if(gArg.CheckExist("wsrtsv"))
						UpdateExpBinPotentialTSV(true);
					eval_f( n, x, _expX, true, obj_value );	    
					last_obj_value = obj_value;
				}
				else // Observe the wirelength change
				{
					if(m_bMoveZ)
						LayerAssignment();
						//Parallel( LayerAssignmentThread, m_pDB->m_modules.size() );
					
					Parallel( UpdateBlockPositionThread, m_pDB->m_modules.size() );
					m_pDB->CalcHPWL();
					//m_pDB->CalcTSV();
					if(gArg.CheckExist("wsrtsv"))
						UpdateExpBinPotentialTSV(true);
					if( currentLevel > 1 && m_ite == 1 )
					{
					// Wirelength minimization stage
					//if( (m_pDB->GetHPWL() + m_weightTSV*m_pDB->GetTSVcount()) > (lastHPWL + m_weightTSV*lastTSV) )
					if( m_pDB->GetHPWL() > lastHPWL )
					{
						lastHPWL = 0;
						//lastTSV = 0;
						break;
					}
					}
					else
					{
					// Block spreading stage
					//if( (m_pDB->GetHPWL() + m_weightTSV*m_pDB->GetTSVcount()) < (lastHPWL + m_weightTSV*lastTSV) )
					if( m_pDB->GetHPWL() < lastHPWL )
					{
						lastHPWL = 0;
						//lastTSV = 0;
						break;
					}
					}
					lastHPWL = m_pDB->GetHPWL();
					//lastTSV = m_pDB->GetTSVcount();
				}
			}

	#if 1
			// Output solving progress
			if( innerIte % outStep == 0 /*&& innerIte != 0*/ && m_useEvalF )
			{
				if( innerIte % checkStep != 0 )
					eval_f( n, x, _expX, true, obj_value );
				printf( "\n  (%4d): f %g\t w %g\t p %g\tstep= %.5f \t%.1fm ", 
					innerIte, obj_value, gTotalWL, density, m_stepSize,
					double(seconds()-time_start)/60.0
					);
				fflush( stdout );

			}
	#endif

			if( innerIte % checkStep == 0 )
			{
				printf( "." );
				fflush( stdout );

				// Early exit when current HPWL > bestLegalHPWL
				if( !param.bCong && bestLegalWL != DBL_MAX && innerIte % (2 * checkStep) == 0 )
				{
					if(m_bMoveZ)  
						LayerAssignment();
						//Parallel( LayerAssignmentThread, m_pDB->m_modules.size() );

					Parallel( UpdateBlockPositionThread, m_pDB->m_modules.size() );
					if( m_pDB->CalcHPWL() > bestLegalWL )   // gWL > LAL-WL
					{
					printf( "X\n" );
					fflush( stdout );
					break;	
					}
				}
				UpdateDensityGrid( n, x, z);  // find the exact bin density
				totalOverDen = GetTotalOverDensity();
				totalOverPotential = GetTotalOverPotential();
				lastTotalOver = over;
				if( param.bCong )
					over = totalOverPotential; // Congestion-driven. Cannot see totalOverDensity 
				else
					over = min( totalOverPotential, totalOverDen ); 
				//Brian 2007-06-18
				if (param.bCongObj)
				{
					UpdateDensityGridNet();
					lastMaxDenNet = maxDenNet;
					maxDenNet = GetMaxDensityNet();
					totalOverDenNet = GetTotalOverDensityNet();
					totalOverPotentialNet = GetTotalOverPotentialNet();
					lastTotalOverNet = overNet;
					overNet = min( totalOverPotentialNet, totalOverDenNet );
				}
				//@Brian 2007-06-18

				/*
				if( !startDecreasing
				&& over < lastTotalOver 
				//	&& m_ite >= 0 
				&& innerIte >= 6 )
				{
				printf( ">>" );
				fflush( stdout );
				startDecreasing = true;
				}*/

				if( !startDecreasing )
				{
					if( ( innerIte <= 10 && m_ite == 1 ) || 
					( innerIte <= 5 ) )    // need to wait until "stable"
					lastDensityCost = density;
					else if( density < lastDensityCost * 0.99 )
					{
					printf( ">>" );
					fflush( stdout );
					startDecreasing = true;
					}	
				}

				if( startDecreasing && over < target_density && m_ite != 1 )	// no early stop at ite 1
					break;  // 2005-03-11 (donnie) Meet the constraint

				//Brian 2007-06-18
				if (false == param.bCongObj)
				{
					if( m_useEvalF && obj_value >= m_precision * old_obj)    // Cannot further reduce "f"
					break;
				}
				else
				{
					if ( (m_useEvalF && obj_value >= m_precision * old_obj && lastMaxDenNet < maxDenNet) || 
						innerIte >400)
					break; 
				}
				//@Brian 2007-06-18

			} // check in the inner loop

			if( newDir == true )	
			{
				// gradient direction
				newDir = false;
				for( int i=0; i<n; i++ )
				{
					grad_f[i] = -grad_f[i];
					walk_direction[i] = grad_f[i];
				}
			}
			else
			{
				// conjugate gradient direction
				if( FindBeta( n, grad_f, last_grad_f, m_beta ) == false ){
					printf("FindBeta OVERFLOW!\n");
					return false;   // overflow?
				}
				Parallel( UpdateGradThread, n );
			}

			LineSearch( n, x, walk_direction, m_stepSize ); // Calculate a_k (step size)
			Parallel( UpdateXThread, m_pDB->m_modules.size() );	    // Update X. (x_{k+1} = x_{k} + \alpha_k * d_k)
			/*double max_z = 0;
			for(unsigned int i = 0; i < z.size(); i++)
			{
				printf("%.2f ", z[i]);
				if(fabs(z[i]) > max_z) max_z = fabs(z[i]);
			}
			for(unsigned int i = 0; i < z.size(); i++)
			{
				z[i] /= max_z;
				printf("%.2f ", z[i]);
			}*/
			Parallel( BoundXThread, m_pDB->m_modules.size() );
			if(m_bMoveZ)
				Parallel( BoundZThread, m_pDB->m_modules.size() );

			// New block positions must be ready
			// 1. UpdateExpValueForEachCellThread    (wire force)
			// 2. UpdateExpValueForEachPinThread     (wire force)
			// 3. ComputeNewPotentialGridThread      (spreading force)
			Parallel( UpdateNLPDataThread, m_pDB->m_modules.size(), m_pDB->m_pins.size(), m_pDB->m_modules.size() );

			// New EXP values must be ready
			double time_used = seconds();
			Parallel( UpdateNetsSumExpThread, (int)m_pDB->m_nets.size() );
			time_wire_force += seconds() - time_used;

			time_used = seconds();
			UpdatePotentialGrid(z);
			time_spreading_force += seconds() - time_used;

			//Brian 2007-04-30
			if (true == param.bCongObj)
			{
				Parallel( UpdateBlockPositionThread, m_pDB->m_modules.size() );
				ComputeNetBoxInfo();
				ComputeNewPotentialGridNet();
				UpdatePotentialGridNet();
			}
			//@Brian 2007-04-30
		
			//Added by Jin 20081013
			//Rerun in the end of the inner loop
			// if( true == param.bTiming )
			// {
			// Parallel( UpdateBlockPositionThread, m_pDB->m_modules.size() );
			// m_pDB->m_pTimingAnalysis->RerunSTA();	
			// }
			//@Added by Jin 20081013

			// donnie 2007-07-10
			if( innerIte == forceBreakLoopCount )
			{
				printf( "b" );
				bUpdateWeight = false;
				break;
			}
		}// inner loop

		if( param.bShow )
		{
			printf( "%d\n", innerIte );
			fflush( stdout );
		}
		else
			printf( "\n" );
		totalIte += innerIte;

		UpdateDensityGrid( n, x, z );
		maxDen = GetMaxDensity();
		totalOverDen = GetTotalOverDensity();
		totalOverDenLB = GetTotalOverDensityLB();
		totalOverPotential = GetTotalOverPotential();

		//Brian 2007-04-30
		if (param.bCongObj)
		{
			UpdateDensityGridNet();
			maxDenNet = GetMaxDensityNet();
			totalOverDenNet = GetTotalOverDensityNet();
			totalOverPotentialNet = GetTotalOverPotentialNet();
		}
		//@Brian 2007-04-30

		if(m_bMoveZ)
			LayerAssignment();
			//Parallel( LayerAssignmentThread, m_pDB->m_modules.size() );
		
		Parallel( UpdateBlockPositionThread, m_pDB->m_modules.size() );  // update to placeDB
		
		//Added by Jin 20081013
		//Rerun the STA after the inner loop
		// if( true == param.bTiming )
		// {
		//     m_pDB->m_pTimingAnalysis->RerunSTA();	
		// }
		//@Added by Jin 20081013

		if( obj_value > DBL_MAX * 0.5 ){
			printf("Objective value OVERFLOW!\n");;
			return false; // overflow
		}

		if( param.bShow )
		{
			printf( " %d-%2d HPWL= %.0f\tDen= %.2f %.4f %.4f %.4f LCPU= %.1fm Dcost= %4.1f%% ", 
				currentLevel, m_ite, m_pDB->CalcHPWL(), 
				maxDen, totalOverDen, totalOverDenLB, totalOverPotential,
				double(seconds()-time_start)/60.0, 
				0.5 * density * _weightDensity /obj_value * 100.0 ); 

			/*char filelevel[256];
			sprintf(filelevel, "%s.gp-level%d-%d.plt", param.outFilePrefix.c_str(), currentLevel, m_ite);
			m_pDB->OutputGnuplotFigure3D(filelevel, false, false);*/

			// if( param.bCong )
			// {
			// // Inter-iteration update
			// if( gArg.CheckExist( "congExpPotential" ) )
			//     UpdateCongestion();
			// else if( gArg.CheckExist( "congPotential" ) )
			// {
			//     UpdateCongestionBasePotential();    
			//     SmoothBasePotential3D();
			//     UpdateExpBinPotential( m_targetUtil, true );
			// }
			// }

			// if( gArg.CheckExist( "gpfig" ) )
			// {
			// char postfix[10];
			// sprintf( postfix, "%d-%d", currentLevel, m_ite );  
			// PlotGPFigures( postfix );
			// }
		}
		else
		{
			printf( " %d-%2d HPWL= %.f\tLCPU= %.1fm ", 
				currentLevel, m_ite, m_pDB->CalcHPWL(), double(seconds()-time_start)/60.0 );
		}
		fflush( stdout );


		bool spreadEnough = totalOverPotential < 1.3;
		bool increaseOverPotential = totalOverPotential > lastTotalOverPotential;
		bool increaseMaxDen = maxDen > lastMaxDen;
		bool enoughIteration = ite > 3;
		bool notEfficientOptimize = 0.5 * density * _weightDensity / obj_value * 100.0 > 95;

		//Brian 2007-04-30
		//bool spreadEnoughNet = totalOverPotentialNet < 1.3;	    // commented by donnie
		//bool increaseOverPotentialNet = totalOverPotentialNet > lastTotalOverPotentialNet; // commented by donnie
		//bool increaseMaxDenNet = maxDenNet > lastMaxDenNet;	// commented by donnie
		//@Brian 2007-04-30

		//PrintPotentialGrid();
		if( enoughIteration && notEfficientOptimize )
		{
			printf( "Failed to further optimize\n" );
			break;
		}

		if( enoughIteration && increaseOverPotential && increaseMaxDen && spreadEnough )
		{
			printf( "Cannot further reduce over potential!\n" ); // skip LAL
			break;
		}

		if( param.bCong && enoughIteration && increaseOverPotential )
		{
			printf( "Cannot further reduce over potential!\n" );
			break;
		}


	#if 1
		// 2006-03-06 (CAUTION! Do not use look-ahead legalization when dummy block exists.
		// TODO: check if there is dummy block (m_modules[].m_isDummy)
		//int startLALIte = 2;
		//if( param.bPrototyping )
		//    startLALIte = 1;

		int startLALIte = 1;

		if( m_bRunLAL 
			&& startDecreasing  // 2006-10-23
			&& m_ite >= startLALIte 
			&& m_lookAheadLegalization 
			&& over < target_density + 0.20
			//&& over < target_density + 0.25 
		)
		{
			if(m_bMoveZ)
				LayerAssignment();
				//Parallel( LayerAssignmentThread, m_pDB->m_modules.size() );
			
			Parallel( UpdateBlockPositionThread, m_pDB->m_modules.size() );
			double hpwl = m_pDB->CalcHPWL();
			if( hpwl > bestLegalWL && !param.bCong )
			{
			printf( "Stop. Good enough.\n" );
			break;	// stop placement
			}

			lookAheadLegalCount++;
			double oldWL = hpwl;
			
	//kaie
			m_pDB->m_modules3d.resize(m_pDB->m_totalLayer);
			for(int layer = 0; layer < m_pDB->m_totalLayer; layer++)
				m_pDB->m_modules3d[layer].clear();

			for(unsigned int i = 0; i < m_pDB->m_modules.size(); i++)
			{
				int layer = (int)m_pDB->m_modules[i].m_z;
				assert(layer >= 0 && layer <= m_pDB->m_totalLayer);
				m_pDB->m_modules3d[layer].push_back(i);
			}

			vector<CSiteRow> m_sites_bak = m_pDB->m_sites;
			for(unsigned int i = 0; i < m_pDB->m_modules.size(); i++)
			m_pDB->m_modules[i].m_isFixed = true;

			bool legalStart = seconds();
			bool bLegal = true;

			for(int layer = 0; layer < m_pDB->m_totalLayer; layer++)
			{
			printf("layer = %d\n", layer);
			m_pDB->m_layer = layer;
			m_pDB->m_sites = m_pDB->m_sites3d[layer];

			for(unsigned int i = 0; i < m_pDB->m_modules3d[layer].size(); i++)
				m_pDB->m_modules[m_pDB->m_modules3d[layer][i]].m_isFixed = false;
			m_pDB->RemoveFixedBlockSite();

			double cellarea = 0;
			for(unsigned int i = 0; i < m_pDB->m_modules3d[layer].size(); i++)
				cellarea += m_pDB->m_modules[m_pDB->m_modules3d[layer][i]].m_area;
			double rowarea = 0;
			for(unsigned int i = 0; i < m_pDB->m_sites.size(); i++)
				for(unsigned int j = 0; j < m_pDB->m_sites[i].m_interval.size(); j+= 2){
				rowarea += m_pDB->m_rowHeight * (m_pDB->m_sites[i].m_interval[j+1]-m_pDB->m_sites[i].m_interval[j]);
			}
			//printf("%.0f , %.0f\n", cellarea, rowarea);
			if(rowarea < cellarea)
			{
				printf("Row area is not enough!!\n");
				///exit(0);
			}

			m_pDB->m_sites_for_legal = m_pDB->m_sites;

			CTetrisLegal legal(*m_pDB);
			legal.m_layer = layer;

			double scale = 0.85; // hpwl driven
			if( givenTargetUtil < 1.0 && givenTargetUtil > 0 )
				scale = 0.9; // with density constraint

			bLegal = bLegal && legal.Solve( givenTargetUtil, false, false, scale );

			/*CTetrisLegal legal( *m_pDB );

			double scale = 0.85;    // hpwl driven
			if( givenTargetUtil < 1.0 && givenTargetUtil > 0 )
			scale = 0.9;	    // with density constraint

			double legalStart = seconds();
			m_pDB->RemoveFixedBlockSite(); //indark 
			// cell orientation is not optimized in the legalizer

			bool bLegal = legal.Solve( givenTargetUtil, false, false, scale );*/

			for(unsigned int i = 0; i < m_pDB->m_modules3d[layer].size(); i++)
				m_pDB->m_modules[m_pDB->m_modules3d[layer][i]].m_isFixed = true;
			}

			m_pDB->m_sites = m_sites_bak;
			for(int layer = 0; layer < m_pDB->m_totalLayer; layer++)
			for(unsigned int i = 0; i < m_pDB->m_modules3d[layer].size(); i++)
				m_pDB->m_modules[m_pDB->m_modules3d[layer][i]].m_isFixed = false;

	//@kaie

			double legalTime = seconds() - legalStart;
			totalLegalTime += legalTime;

			if( param.bShow )
			printf( "[LAL] %d trial.  CPU Time = %.2f\n", lookAheadLegalCount, legalTime );
			if( bLegal )
			{
			m_pDB->Align();	// 2006-04-02

			double WL;

			// if( gArg.CheckExist( "congExpPotential" ) )
			//     WL = UpdateCongestion();
			// else if( gArg.CheckExist( "congPotential" ) )
			//     WL = UpdateCongestionBasePotential();    
			// else
				WL = m_pDB->GetHPWLdensity( givenTargetUtil );

			if( param.bShow )
			{
				if( param.bCong )
				printf( "[LAL] Overflow= %.0f\n", WL );
				else
				{
				m_pDB->ShowDensityInfo();
				printf( "[LAL] HPWL= %.0f   dHPWL= %.0f (%.2f%%)\n", m_pDB->GetHPWLp2p(), WL, (WL-oldWL)/oldWL*100 );
				}
				/*char postfix[10];
				sprintf( postfix, "%d-%d.lal", currentLevel, m_ite );
				PlotGPFigures( postfix );*/
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
			}
			else
			{
				// For WL minimization.
				if( !param.bCong && (WL-oldWL)/oldWL < 0.075 )
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
		}
	#endif	

		if( param.bPrototyping 
			&& startDecreasing 
			&& over < target_density 
			&& ite >= 0 )
		{
			printf( "Meet constraint! (prototyping)\n" );
			break;
		}

		if( /*ite >= 2 &&*/ startDecreasing && over < target_density )
		{
			printf( "Meet constraint!\n" );
			break;
		}


		if( bUpdateWeight )
			UpdateObjWeights();

		//@Brian 2007-04-30
		lastTotalOverPotential = totalOverPotential;
		lastMaxDen = maxDen;

		lastTotalOverPotentialNet = totalOverPotentialNet;
		lastMaxDenNet = maxDenNet;
		//@Brian 2007-04-30

    }// outer loop

    // 2006-03-06 (donnie)
    if( hasBestLegalSol )
    {
	m_pDB->m_modules_bak = bestGPresult;
	x = xBest;
	*isLegal = true;
    }

    // char filegp[100];
    // sprintf(filegp, "%s.gp-%d.plt", param.outFilePrefix.c_str(), currentLevel);
    // m_pDB->OutputGnuplotFigure3D( filegp, false, false);
    if(m_bMoveZ)
    {
    	LayerAssignment();
		//Parallel( LayerAssignmentThread, m_pDB->m_modules.size() );
    	//LayerAssignment();
    }
    Parallel( UpdateBlockPositionThread, m_pDB->m_modules.size() );
    
    // printf("#TSVs = %d\n", (int)m_pDB->CalcTSV());
    // sprintf(filegp, "%s.gpl-%d.plt", param.outFilePrefix.c_str(), currentLevel);
    // m_pDB->OutputGnuplotFigure3D( filegp, false, false);
    /*for(unsigned int i = 0; i < m_pDB->m_modules.size(); i++)
    {
	    double integer;
	    if((int)modf(m_pDB->m_modules[i].m_z, &integer) != 0) printf("%.2f\n", m_pDB->m_modules[i].m_z);
    }*/
    //m_pDB->CalcTSV();
    //printf("\nTSV count = %d\n", m_pDB->GetTSVcount());
    if( lookAheadLegalCount > 0 && param.bShow )
    {
	printf( "[LAL] Total Count: %d\n", lookAheadLegalCount );
	printf( "[LAL] Total CPU: %.2f\n", totalLegalTime );
	sprintf( filename, "util.gp" );
	CPlaceBin placeBin( *m_pDB );
	placeBin.CreateGrid( m_pDB->m_rowHeight * 10.0 );
	placeBin.OutputBinUtil( filename );
    }

    if( param.bShow )
    {
	static int allTotalIte = 0;
	allTotalIte += totalIte;
	m_pDB->ShowDensityInfo();
	printf( "\nLevel Ite %d   Total Ite %d\n", totalIte, allTotalIte );
    }

    return true;
    //return hasBestLegalSol;
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

// static 
void* MyNLP::UpdateXThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    for( int i=pMsg->index1; i<pMsg->index2; i++ )
    {
	if(pMsg->pNLP->m_bMoveZ)
	{
	    pMsg->pNLP->x[2*i] += pMsg->pNLP->walk_direction[3*i] * pMsg->pNLP->m_stepSizeX;
	    pMsg->pNLP->x[2*i+1] += pMsg->pNLP->walk_direction[3*i+1] * pMsg->pNLP->m_stepSizeY;
	}else
	{
	    pMsg->pNLP->x[2*i] += pMsg->pNLP->walk_direction[2*i] * pMsg->pNLP->m_stepSize;
	    pMsg->pNLP->x[2*i+1] += pMsg->pNLP->walk_direction[2*i+1] * pMsg->pNLP->m_stepSize;
	}
	
	
	if(pMsg->pNLP->m_bMoveZ)
	{
	    double scale = pMsg->pNLP->m_potentialGridSize;
	    gArg.GetDouble("testscale", &scale);
	    pMsg->pNLP->z[i] += pMsg->pNLP->walk_direction[3*i+2] * pMsg->pNLP->m_stepSizeZ / scale; // /  pMsg->pNLP->m_potentialGridWidth;
	}
    	//printf("%d (%.2f, %.2f, %.2f)\n", i, pMsg->pNLP->x[2*i], pMsg->pNLP->x[2*i+1], pMsg->pNLP->z[i]);
    }
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static
void* MyNLP::UpdateNLPDataThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);

    double time_used = seconds();

    // cells * 2   
    UpdateExpValueForEachCell( pMsg->index2, *pMsg->pX, *pMsg->pZ, *pMsg->pExpX, *pMsg->pExpZ, pMsg->alpha, pMsg->index1, pMsg->pNLP );

    // pins
    UpdateExpValueForEachPin( 
	    pMsg->index4, *pMsg->pX, *pMsg->pZ, pMsg->pNLP->_expPins, pMsg->pNLP->_expPinsZ, 
	    pMsg->alpha, pMsg->pNLP, pMsg->pUsePin, pMsg->index3 );

    time_wire_force += seconds() - time_used;	// wrong when th != 1

    // cells
    time_used = seconds();
    ComputeNewPotentialGrid( *pMsg->pX, *pMsg->pZ, pMsg->pNLP, pMsg->index5, pMsg->index6 );
    time_spreading_force += seconds() - time_used;

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
    {
	//if( i%3 == 2 ) continue;
	norm += pMsg->pNLP->last_grad_f[i] * pMsg->pNLP->last_grad_f[i];
    }
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
    {
	//if( i%3 == 2 ) continue;
	product += pMsg->pNLP->grad_f[i] * ( pMsg->pNLP->grad_f[i] + pMsg->pNLP->last_grad_f[i] );
    }
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

    long double l2norm = 0;
    long double product = 0;

    bool bMoveZ = false;
    if(param.b3d && !gArg.CheckExist("noZ"))
		bMoveZ = true;

    //const double maxGradF = 1.0;

    // Compute the scaling factor to avoid overflow
    double maxGradF = 0;
    for( int i=0; i<n; i++ )
    {
	if( bMoveZ && i%3 == 2 ) continue;
	//assert( fabs(grad_f[i]) < DBL_MAX * 0.95 );
	//assert( fabs(last_grad_f[i]) < DBL_MAX * 0.95 );
	if( grad_f[i] > maxGradF )
	    maxGradF = grad_f[i];
	if( last_grad_f[i] > maxGradF )
	    maxGradF = last_grad_f[i];
    }
    assert( maxGradF != 0 );
	// if(fabs(maxGradF) < 1.0e-10) // kaie
    // {
	// 	beta = 0;
	// 	return true;
    // }

#if 1    
    for( int i=0; i<n; i++ )
    {
	if( bMoveZ && i%3 == 2 ) continue;
	l2norm += (last_grad_f[i]/maxGradF) * (last_grad_f[i] / maxGradF);
    }
    for( int i=0; i<n; i++ )
    {
	if( bMoveZ && i%3 == 2 ) continue;
	//product += grad_f[i] * ( grad_f[i] + last_grad_f[i] ) / maxGradF / maxGradF;	// g_k^T ( g_k - g_{k-1} )
	//assert( fabs( (grad_f[i]/maxGradF) * ( (grad_f[i]+last_grad_f[i]) / maxGradF) ) < DBL_MAX * 0.95 );
	product += (grad_f[i]/maxGradF) * ( (grad_f[i] - last_grad_f[i]) / maxGradF) ;	// g_k^T ( g_k - g_{k-1} )
    }
#else
    // Parallelization changes the results
    //l2norm = Parallel( FindGradL2NormThread, n );	
    //product = Parallel( FindGradProductThread, n );
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

// (kaie) 2009-09-10 3d placement
// void MyNLP::LayerAssignment()
// {
// 	vector<double> totalArea;
//     totalArea.resize(m_pDB->m_totalLayer, 0.0);
//     double layerThickness = (m_pDB->m_front - m_pDB->m_back) / (double)m_pDB->m_totalLayer;
//     vector<int> middleblocks;
//     double threshould_factor = 0.1;
//     double threshould_b = threshould_factor * layerThickness;
//     double threshould_t = (1.0-threshould_factor) * layerThickness;

//     double coreArea = (m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left) * (m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom);

//     for(unsigned int i = 0; i < m_pDB->m_modules.size(); i++)
//     {
// 		if(m_pDB->m_modules[i].m_isFixed)
// 			totalArea[(int)(floor((z[i] - 0.5*layerThickness)/layerThickness))] += m_pDB->m_modules[i].m_area;
// 		else
// 		{
// 			z[i] -= 0.5 * layerThickness;
// 			double layer = floor(z[i]/layerThickness);
// 			double offset = fmod(z[i], layerThickness);
// 			if(offset < threshould_b)
// 			{
// 				z[i] = (layer+0.5) * layerThickness;
// 				totalArea[layer] += m_pDB->m_modules[i].m_area;
// 			}
// 			else if(offset > threshould_t)
// 			{
// 				z[i] = (layer+1.5) * layerThickness;
// 				totalArea[layer+1] += m_pDB->m_modules[i].m_area;
// 			}
// 			else
// 				middleblocks.push_back(i);
// 		}
//     }

//     for(unsigned int i = 0; i < totalArea.size(); i++) 
// 		printf("%d: %lf\n", i, totalArea[i]/coreArea);

//     printf("%lf\n", m_targetUtil);
//     for(unsigned int i = 0; i < middleblocks.size(); i++)
//     {
// 		int blockId = middleblocks[i];
// 		//z[blockId] -= 0.5 * layerThickness;
// 		double layer = floor(z[blockId]/layerThickness);
// 		double offset = fmod(z[blockId], layerThickness);
// 		//printf("%d, %lf, %lf\n", blockId, z[blockId], layer);
// 		if(offset > 0.5 && (totalArea[(int)layer+1] + m_pDB->m_modules[blockId].m_area < coreArea * m_targetUtil * 0.9))
// 			layer += 1;
// 		else if((totalArea[(int)layer+1] < totalArea[(int)layer]) && (totalArea[(int)layer] + m_pDB->m_modules[blockId].m_area > coreArea * m_targetUtil * 0.9))
// 			layer += 1;
// 		totalArea[(int)layer] += m_pDB->m_modules[blockId].m_area;
// 		z[blockId] = (layer + 0.5 * layerThickness);
//     }
// }

void MyNLP::LayerAssignment( const int& n, vector<double>& z, MyNLP* pNLP, int index1, int index2 )
{
	if(param.bShow){
		cout << "---------------------------- Some LayerAssignment Result---------------------------------\n";
		for(int i=0;i<z.size();++i){
			if(i%400 == 0)
				cout << "cell["<<i<<"].z = " << z[i] << "\n";
		}
		cout << "-----------------------------------------------------------------------------------------\n";
	}
    if( index2 > n ) index2 = n;

	//cout << "---------------------------------------- index: " << index1 << " ~ " << index2 << "\n";
	bool reverse = false;
	double cutline = pNLP->m_pDB->m_dCutline;
	if(param.nlayer == 2 && pNLP->m_pDB->m_rowNums.size()==2){ // cad contest 2022
		// check the more side of two die
		int lower=0, higher=0;
		for(int i= index1; i < index2; i++){
			z[i] -= 0.5; // z_after = [0:1]
			if(z[i] <= 0.5) lower++;
			else higher++;
		}
		if(lower < higher) cutline = 1-cutline;
	}

	for( int i= index1; i < index2; i++){
		if(param.nlayer == 2 && pNLP->m_pDB->m_rowNums.size()==2){ // cad contest 2022
			z[i] = (z[i] <= cutline) ? 0.5 : 1.5;
		} else{ // origin
			double layerThickness = (pNLP->m_pDB->m_front - pNLP->m_pDB->m_back) / (double)(pNLP->m_pDB->m_totalLayer);
			double z_after = z[i] - 0.5 * layerThickness;
			double layer = floor(z_after/layerThickness);
			double offset = fmod(z_after, layerThickness);
			if(offset > 0.5*layerThickness) layer += 1;
			z_after = (layer+0.5)*layerThickness;
			assert(fabs(z[i]-z_after) <= 0.5 * layerThickness);
			z[i] = z_after;
		}
    }
}
void MyNLP::LayerAssignment()
{
	if(param.bShow){
		cout << "---------------------------- Some LayerAssignment Result---------------------------------\n";
		for(int i=0;i<z.size();++i){
			if(i%400 == 0)
				cout << "cell["<<i<<"].z = " << z[i] << "\n";
		}
		cout << "-----------------------------------------------------------------------------------------\n";
	}
	bool reverse = false;
	double cutline = m_pDB->m_dCutline;
	// check the more side of two die
	int lower=0, higher=0;
	for(int i = 0; i < m_pDB->m_modules.size(); ++i){
		z[i] -= 0.5; // z_after = [0:1]
		if(z[i] <= 0.5) lower++;
		else higher++;
	}
	if(lower < higher) cutline = 1-cutline;

	vector<pair<int,double> > z_ori0; // z_ori[moduleId] = z_ori  // die0
	vector<pair<int,double> > z_ori1; // z_ori[moduleId] = z_ori  // die1
	vector<double> total_area(2, 0.0);
	for(int i = 0; i < m_pDB->m_modules.size(); ++i){
		if(!m_pDB->m_modules[i].m_isVia){
			//z[i] = (z[i] <= cutline) ? 0.5 : 1.5;
			if(z[i] <= cutline){
				z_ori0.emplace_back(pair<int,double>(i,z[i]));
				total_area[0] += (m_pDB->m_modules[i].m_widths[0]*m_pDB->m_modules[i].m_heights[0]);
				z[i] = 0.5;
			} else{
				z_ori1.emplace_back(pair<int,double>(i,z[i]));
				total_area[1] += (m_pDB->m_modules[i].m_widths[1]*m_pDB->m_modules[i].m_heights[1]);
				z[i] = 1.5;
			}
		} else{
			z[i] = 0.5;
		}
    }

	// check the max_util
	vector<double> valid_area(2, 0.0);
	double coreArea = (m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left) * (m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom);
	for(int k=0;k<param.nlayer;++k)
		valid_area[k] = coreArea * (double)m_pDB->m_maxUtils[k];
	//cout << "\n\033[34m[LayerAssignment]\033[0m - Top-Die:" << total_area[0] << "/" << valid_area[0] << "(" << setprecision(2) << total_area[0]/valid_area[0] << "), Bot-Die:" << total_area[1] << "/" << valid_area[1] << "(" << setprecision(2) << total_area[1]/valid_area[1] << "), cell_num: " << z_ori0.size() << ":" << z_ori1.size() << "\n";
	int moved_num = 0;
	if(total_area[0] > valid_area[0]){ // too many cell in top-die, move cells to bot-die for matching die's max_utilization
		// move the cells with higher z
		sort(z_ori0.begin(), z_ori0.end(), [](pair<int,double> const& l, pair<int,double> const& r){ return l.second > r.second; });
		for(int i=0;i<z_ori0.size();++i){
			int moduleId = z_ori0[i].first;
			total_area[0] -= (m_pDB->m_modules[moduleId].m_widths[0] * m_pDB->m_modules[moduleId].m_heights[0]);
			z[moduleId] = 1.5;
			++moved_num;
			if(total_area[0] < valid_area[0])
				break;
		}
	} else if(total_area[1] > valid_area[1]){ // too many cell in bot-die, move cells to top-die for matching die's max_utilization
		// move the cells with lower z
		sort(z_ori1.begin(), z_ori1.end(), [](pair<int,double> const& l, pair<int,double> const& r){ return l.second < r.second; });
		for(int i=0;i<z_ori1.size();++i){
			int moduleId = z_ori1[i].first;
			total_area[1] -= (m_pDB->m_modules[moduleId].m_widths[1]*m_pDB->m_modules[moduleId].m_heights[1]);
			z[moduleId] = 0.5;
			--moved_num;
			if(total_area[1] < valid_area[1])
				break;
		}
	}
	if(moved_num != 0){
		if(moved_num > 0)
			cout << "\033[34m[LayerAssignment]\033[0m - " << abs(moved_num) << " modules have been changed to bot-die by max_util constr.\n";
		if(moved_num < 0)
			cout << "\033[34m[LayerAssignment]\033[0m - " << abs(moved_num) << " modules have been changed to top-die by max_util constr.\n";
		cout << "\n\033[34m[LayerAssignment]\033[0m - cell_num: " << z_ori0.size()-moved_num << "/" << z_ori1.size()+moved_num << "\n";
	}
}

void* MyNLP::LayerAssignmentThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    LayerAssignment( (int)pMsg->pZ->size(), *pMsg->pZ, pMsg->pNLP, pMsg->index1, pMsg->index2);
    if( param.nThread > 1 )
	    pthread_exit( NULL );
    return NULL;
}

void MyNLP::BoundZ( const int& n, vector<double>& z, const double& z_l, const double& z_u, int index1, int index2)
{
    if( index2 > n ) index2 = n;
    for( int i=index1; i<index2; i++ )
    {
		if( z[i] < z_l) z[i] = z_l;
		else if( z[i] > z_u ) z[i] = z_u;
    }
}

void* MyNLP::BoundZThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    double z_l = pMsg->pNLP->m_pDB->m_back;
    double z_u = pMsg->pNLP->m_pDB->m_front;
    double layerThickness = (z_u-z_l)/(pMsg->pNLP->m_pDB->m_totalLayer);
    z_l += layerThickness * 0.5;
    z_u -= layerThickness * 0.5;
    BoundZ( (int)pMsg->pZ->size(), *pMsg->pZ, z_l, z_u, pMsg->index1, pMsg->index2);
    if( param.nThread > 1 )
	    pthread_exit( NULL );
    return NULL;
}
// @(kaie) 2009-09-10

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
    if( index2*2 > n )
		index2 = n;
    for( int i=index1; i<index2; i++ ) // index for modules
    {
		if( x[2*i] < x_l[2*i] )             x[2*i] = x_l[2*i];
		else if( x[2*i] > x_h[2*i] )	x[2*i] = x_h[2*i];
		if( x[2*i+1] < x_l[2*i+1] )             x[2*i+1] = x_l[2*i+1];
		else if( x[2*i+1] > x_h[2*i+1] )    x[2*i+1] = x_h[2*i+1];
    } 
}


bool MyNLP::AdjustForce( const int& n, const vector<double>& x, vector<double>& f )
{
    if( param.bAdjustForce == false )
	return true;//no adjust

    double totalGrad = 0;
    double totalGradZ = 0;
    int size;
    if(m_bMoveZ) 
	size = n/3; // x, y, and z directions
    else
	size = n/2;

    static int outCount = 0;
    outCount++; 

    if( gArg.CheckExist( "plotForce" ) )
    {
	char filename[255];
	sprintf( filename, "force%d.dat", outCount );
	DataHandler data1;
	for( int i=0; i<size; i++ )
	{
	    if(m_bMoveZ)
	    	data1.Insert( f[3*i] * f[3*i] + f[3*i+1] * f[3*i+1] + f[3*i+2] * f[3*i+2] );
	    else
		data1.Insert( f[2*i] * f[2*i] + f[2*i+1] * f[2*i+1] );
	}
	data1.Sort();
	data1.OutputFile( filename );
    }

    for( int i=0; i<size; i++ )
    {
	if(m_bMoveZ)
	    totalGrad += f[3*i] * f[3*i] + f[3*i+1] * f[3*i+1];
	else
	    totalGrad += f[2*i] * f[2*i] + f[2*i+1] * f[2*i+1];
	if(m_bMoveZ)
	    totalGradZ += f[3*i+2] * f[3*i+2];
    }
    double avgGrad = sqrt( totalGrad / size );
    double avgGradZ = sqrt( totalGradZ / size );

    //if( isNaN( totalGrad ) )
    //	return false;

    // Do truncation
    double expMaxGrad = avgGrad * param.truncationFactor;	// x + y
    double expMaxGradSquare = expMaxGrad * expMaxGrad;
    double expMaxGradZ = avgGradZ * param.truncationFactor; // z
    double expMaxGradSquareZ = expMaxGradZ * expMaxGradZ;
    for( int i=0; i<size; i++ )
    {
	double valueSquare;
	double valueSquareZ = 0;
	if(m_bMoveZ)
	    valueSquare = ( f[3*i] * f[3*i] + f[3*i+1] * f[3*i+1] );
	else
	    valueSquare = ( f[2*i] * f[2*i] + f[2*i+1] * f[2*i+1] );
	if(m_bMoveZ)
	    valueSquareZ = ( f[3*i+2] * f[3*i+2] );
	if( valueSquare > expMaxGradSquare )
	{
	    double value = sqrt( valueSquare );
	    if(m_bMoveZ)
	    {
	    	f[3*i]   = f[3*i]   * expMaxGrad / value;
	    	f[3*i+1] = f[3*i+1] * expMaxGrad / value;
	    }else
	    {
		f[2*i]   = f[2*i]   * expMaxGrad / value;
		f[2*i+1] = f[2*i+1] * expMaxGrad / value;
	    }
	}
	if(m_bMoveZ)
	{
	    if( valueSquareZ > expMaxGradSquareZ )
	    {
		double valueZ = sqrt( valueSquareZ );
		f[3*i+2] = f[3*i+2] * expMaxGradZ / valueZ;
	    }
	}
    }

    if( gArg.CheckExist( "plotForce" ) )
    {
	char filename[255];
	sprintf( filename, "force%db.dat", outCount );
	DataHandler data1;
	for( int i=0; i<size; i++ )
	{
	    if(m_bMoveZ)
	    	data1.Insert( f[3*i] * f[3*i] + f[3*i+1] * f[3*i+1] + f[3*i+2] * f[3*i+2]);
	    else
		data1.Insert( f[2*i] * f[2*i] + f[2*i+1] * f[2*i+1] );
	}
	data1.Sort();
	data1.OutputFile( filename );
    }

    return true;
}


bool MyNLP::AdjustForce( const int& n, const vector<double>& x, vector<double> grad_wl, vector<double> grad_potential, vector<double> grad_potentialZ)
{
    double totalGrad = 0;
    double totalGradZ = 0;
    int size;
    if(m_bMoveZ)
	size = n/3; // x, y, and z
    else
	size = n/2;
    for( int i=0; i<size; i++ )
    {
	//printf("%lf, %lf, %lf, %.f\n", grad_wl[2*i], grad_potential[2*i], grad_wl[2*i+1], grad_potential[2*i]+1);
	double value = 
	    (grad_wl[2*i] + grad_potential[2*i]) * (grad_wl[2*i] + grad_potential[2*i]) + 
	    (grad_wl[2*i+1] + grad_potential[2*i+1]) * (grad_wl[2*i+1] + grad_potential[2*i+1]);
	if(m_bMoveZ)
	{
	    //printf("Z: %lf, %lf\n", grad_via[i], grad_potentialZ[i]);
	    totalGradZ += (grad_via[i] + grad_potentialZ[i]) * (grad_via[i] + grad_potentialZ[i]);
	}
	totalGrad += value;
    }

    if( isNaN( totalGrad ) )
	return false;
    assert( !isNaN( totalGrad ) );	// it is checked in GoSolve()
    if( isNaN( totalGradZ ) )
	return false;
    assert( !isNaN( totalGradZ ) );

    double avgGrad = sqrt( totalGrad / size );
    double avgGradZ = sqrt( totalGradZ / size );

    // Do truncation
    double expMaxGrad = avgGrad * param.truncationFactor;	// x + y
    double expMaxGradSquare = expMaxGrad * expMaxGrad;
    double expMaxGradZ = avgGradZ * param.truncationFactor; // z
    double expMaxGradSquareZ = expMaxGradZ * expMaxGradZ;
    for( int i=0; i<size; i++ )
    {
	double valueSquare = 
	    (grad_wl[2*i] + grad_potential[2*i]) * (grad_wl[2*i] + grad_potential[2*i]) + 
	    (grad_wl[2*i+1] + grad_potential[2*i+1]) * (grad_wl[2*i+1] + grad_potential[2*i+1]);
	double valueSquareZ = 0;
	if(m_bMoveZ)
	    valueSquareZ += (grad_via[i] + grad_potentialZ[i]) * (grad_via[i] + grad_potentialZ[i]); 
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
	if(m_bMoveZ)
	{
	    if( valueSquareZ == 0 )
	    {
	    	grad_via[i] = 0;
	    	grad_potentialZ[i] = 0;
	    }
	    else
	    {
	    	if( valueSquareZ > expMaxGradSquareZ )
	    	{
		    double valueZ = sqrt( valueSquareZ );
		    grad_via[i] = grad_via[i] * expMaxGradZ / valueZ;
		    grad_potentialZ[i] = grad_potentialZ[i] * expMaxGradZ / valueZ;
	    	}
	    }
	}

    }
    return true;
}


void MyNLP::LineSearch( const int& n, /*const*/ vector<double>& x, vector<double>& f, double& stepSize )
{
    int size;
    if(m_bMoveZ)
	size = n / 3;// x, y, and z
    else
	size = n / 2;
    long double totalGradX = 0, totalGradY = 0, totalGradZ = 0;
    double avgGrad, avgGradX, avgGradY, avgGradZ;

    /*for( int i=0; i<n; i++ )
	totalGrad += f[i] * f[i];
    avgGrad = sqrt( totalGrad / size );
    stepSize = sqrt(m_potentialGridWidth * m_potentialGridHeight * m_potentialGridThickness) / avgGrad * m_currentStep;
    printf("%.2f\n", stepSize);*/
    for( int i = 0; i < size; i++ )
    {
	if(m_bMoveZ)
	{
	    totalGradX += f[3*i] * f[3*i];
	    totalGradY += f[3*i+1] * f[3*i+1];
	    totalGradZ += f[3*i+2] * f[3*i+2];
	}else
	{
	    totalGradX += f[2*i] * f[2*i];
	    totalGradY += f[2*i+1] * f[2*i+1];
	}
    }
    avgGradX = sqrt( totalGradX / size );
    avgGradY = sqrt( totalGradY / size );
    avgGrad  = sqrt( (totalGradX + totalGradY) / size );    

    m_stepSizeX = (m_potentialGridWidth) / avgGradX * m_currentStep;
    m_stepSizeY = (m_potentialGridHeight) / avgGradY * m_currentStep;

    if(m_bMoveZ)
    {
	avgGradZ = sqrt( totalGradZ / size );
	m_stepSizeZ = (m_potentialGridThickness) / avgGradZ * m_currentStepZ;
	if(param.bShow)
		cout << "####################################### m_stepSizeZ=" << m_stepSizeZ << ", avgGradZ="<<avgGradZ<< "\n";
	//printf("sz = %lf\n", m_stepSizeZ);
    }
    
    stepSize = m_potentialGridWidth / avgGrad * m_currentStep;

    return;
}

bool MyNLP::get_nlp_info(int& n, int& m, int& nnz_jac_g, 
	int& nnz_h_lag/*, IndexStyleEnum& index_style*/)
{
    if(m_bMoveZ)
    	n = m_pDB->m_modules.size() * 3;
    else
	n = m_pDB->m_modules.size() * 2;
    m = 0;	    // no constraint
    nnz_jac_g = 0;  // 0 nonzeros in the jacobian since no constraint
    return true;
}


bool MyNLP::get_bounds_info(int n, vector<double>& x_l, vector<double>& x_u )
{
    if(m_bMoveZ)
    	assert(n == (int)m_pDB->m_modules.size() * 3);
    else
		assert(n == (int)m_pDB->m_modules.size() * 2);
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


bool MyNLP::get_starting_point( vector<double>& x, vector<double>& z)
{
	xMax.resize(m_pDB->m_nets.size(),0);
	yMax.resize(m_pDB->m_nets.size(),0);
	zMax.resize(m_pDB->m_nets.size(),0);

    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
		x[2*i]   = m_pDB->m_modules[i].m_cx;
		x[2*i+1] = m_pDB->m_modules[i].m_cy;
		z[i]	 = m_pDB->m_modules[i].m_cz;
		for(int j=0;j<m_pDB->m_modules[i].m_pinsId.size();++j){
			int pinId = m_pDB->m_modules[i].m_pinsId[j];
			int netId = m_pDB->m_pinNetId[pinId];
			xMax[netId] = max(xMax[netId], m_pDB->m_modules[i].m_cx+m_pDB->m_pins[ pinId ].xOff);
			yMax[netId] = max(yMax[netId], m_pDB->m_modules[i].m_cy+m_pDB->m_pins[ pinId ].yOff);
			if(param.b3d) 
				zMax[netId] = max(zMax[netId], m_pDB->m_modules[i].m_cz);
		}
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
	// kaie 2009-08-29
	m_threadInfo[0].pZ      = const_cast< vector<double>* >(&z);
	m_threadInfo[0].pExpZ   = const_cast< vector<double>* >(&_expZ);
	// @kaie 2009-08-29
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
	    // kaie 2009-08-29
	    m_threadInfo[i].pZ	    = const_cast< vector<double>* >(&z);
	    m_threadInfo[i].pExpZ   = const_cast< vector<double>* >(&_expZ);
	    // @kaie 2009-08-29
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


// static
void* MyNLP::UpdateExpValueForCellAndPinThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    UpdateExpValueForEachCell( pMsg->index2, *pMsg->pX, *pMsg->pZ, *pMsg->pExpX, *pMsg->pExpZ, pMsg->alpha, pMsg->index1, pMsg->pNLP );
    UpdateExpValueForEachPin(  pMsg->index4, *pMsg->pX, *pMsg->pZ, pMsg->pNLP->_expPinsZ, pMsg->pNLP->_expPinsZ, pMsg->alpha, pMsg->pNLP, pMsg->pUsePin, pMsg->index3 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static
void* MyNLP::UpdateExpValueForEachCellThread( void* arg )   
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    UpdateExpValueForEachCell( pMsg->index2, *pMsg->pX, *pMsg->pZ, *pMsg->pExpX, *pMsg->pExpZ, pMsg->alpha, pMsg->index1, pMsg->pNLP );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static
void MyNLP::UpdateExpValueForEachCell( const int& index2, const vector<double>& x, const vector<double>& z,
	vector<double>& expX, vector<double>& expZ, const double& inAlpha, 
	const int& index1, MyNLP* pNLP ) 
{
    if( param.bUseLSE || param.bUseWAE )
    {
		for( int i=index1; i<index2; i++ )
		{
			expX[2*i]   = exp( x[2*i] / inAlpha );
			expX[2*i+1] = exp( x[2*i+1] / inAlpha );
			if(pNLP->m_bMoveZ)
				expZ[i]     = exp( z[i] / inAlpha );
		}

		if( m_bXArch ) // 2007-09-12
		{
			for( int i=index1; i<index2; i++ )
			{
			pNLP->_expXplusY[i]   = exp( (x[2*i] + x[2*i+1]) / inAlpha );   // exp( x+y /k )
			pNLP->_expXminusY[i]  = exp( (x[2*i] - x[2*i+1]) / inAlpha );   // exp( x-y /k )
			}
		}
    }
    else
    {
		// Lp-norm
		for( int i=index1; i<index2; i++ )
		{
			expX[2*i]   = pow( x[2*i]   * pNLP->m_posScale, inAlpha ); 
			expX[2*i+1] = pow( x[2*i+1] * pNLP->m_posScale, inAlpha );
			if(pNLP->m_bMoveZ)
				expZ[i]     = pow( z[i] * pNLP->m_posScale, inAlpha); 
		}
    }
}



/*static*/ 
void* MyNLP::UpdateExpValueForEachPinThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    UpdateExpValueForEachPin( 
	    pMsg->index2, 
	    *pMsg->pX,
	    *pMsg->pZ, 
	    pMsg->pNLP->_expPins,
	    pMsg->pNLP->_expPinsZ, 
	    pMsg->alpha, 
	    pMsg->pNLP, 
	    pMsg->pUsePin, 
	    pMsg->index1 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static
void MyNLP::UpdateExpValueForEachPin( const int& index2, const vector<double>& x, const vector<double>& z, 
	vector<double>& expPins, vector<double>& expPinsZ, const double& inAlpha,
	MyNLP* pNLP, const vector<bool>* pUsePin, const int& index1 )
{
    for( int pinId=index1; pinId<index2; pinId++ )
    {
	int blockId = pNLP->m_pDB->m_pins[pinId].moduleId;

	// TODO: floating-pin! (PlaceDB Bug!)
	if( blockId >= (int)pNLP->m_pDB->m_modules.size() )
	    continue;

	// 2006-02-20
	if( (*pUsePin)[blockId] == false )
	    continue;	// save time

	double xx = x[ 2*blockId ]   + pNLP->m_pDB->m_pins[ pinId ].xOff;
	double yy = x[ 2*blockId+1 ] + pNLP->m_pDB->m_pins[ pinId ].yOff;
	double zz = z[ blockId ];

	if( param.bUseLSE || param.bUseWAE )
	{
		int netId = pNLP->m_pDB->m_pinNetId[pinId];
	    expPins[2*pinId]   = exp( (xx-pNLP->xMax[netId]) / inAlpha );
	    expPins[2*pinId+1] = exp( (yy-pNLP->yMax[netId]) / inAlpha );
	    if(pNLP->m_bMoveZ)
	    	expPinsZ[pinId] = exp( (zz-pNLP->zMax[netId]) / inAlpha );
	    
		//cout << "pin[" << pinId << "]: xx=" << xx << ", xMax[" << netId << "]=" << pNLP->xMax[netId] << "\n";
		//cout << "pin[" << pinId << "]: yy=" << yy << ", yMax[" << netId << "]=" << pNLP->yMax[netId] << "\n";

	    if( m_bXArch ) // 2006-09-12
	    {
		pNLP->_expPinXplusY[pinId]  = exp( (xx+yy) / inAlpha );
		pNLP->_expPinXminusY[pinId] = exp( (xx-yy) / inAlpha );
	    }
	}
	else
	{
	    // Lp-norm
	    expPins[2*pinId]   = pow( xx * pNLP->m_posScale, inAlpha );
	    expPins[2*pinId+1] = pow( yy * pNLP->m_posScale, inAlpha );
	    if(pNLP->m_bMoveZ)
	    	expPinsZ[pinId]    = pow( zz * pNLP->m_posScale, inAlpha );
	    //assert( !isNaN( expPins[2*pinId] ) );
	    //assert( !isNaN( expPins[2*pinId+1] ) );
	    /*if( expPins[2*pinId] == 0 || expPins[2*pinId+1] == 0 )
	      {
	      printf( "%g %g %g\n", xx, yy, pNLP->m_posScale );
	      }*/
	    assert( expPins[2*pinId] != 0 );
	    assert( expPins[2*pinId+1] != 0 );
	    if(pNLP->m_bMoveZ)
	    	assert( expPinsZ[pinId] != 0 );
	}
    }
}


// static
void* MyNLP::UpdateNetsSumExpThread( void* arg )
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    UpdateNetsSumExp( *pMsg->pX, *pMsg->pZ, *pMsg->pExpX, *pMsg->pExpZ, pMsg->pNLP, pMsg->index1, pMsg->index2 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static
void MyNLP::UpdateNetsSumExp( const vector<double>& x, const vector<double>& z, const vector<double>& expX, const vector<double>& expZ, MyNLP* pNLP, int index1, int index2 )
{
    double sum_exp_xi_over_alpha;
    double sum_exp_inv_xi_over_alpha;
    double sum_exp_yi_over_alpha;
    double sum_exp_inv_yi_over_alpha;
    double sum_exp_zi_over_alpha;
    double sum_exp_inv_zi_over_alpha;
    if( index2 > (int)pNLP->m_pDB->m_nets.size() )
		index2 = (int)pNLP->m_pDB->m_nets.size();
    for( int n=index1; n<index2; n++ )
    {
		if( pNLP->m_pDB->m_nets[n].size() == 0 )
			continue;

		calc_sum_exp_using_pin(
			pNLP->m_pDB->m_nets[n].begin(), pNLP->m_pDB->m_nets[n].end(), x, z, expX, expZ,
			sum_exp_xi_over_alpha, sum_exp_inv_xi_over_alpha,
			sum_exp_yi_over_alpha, sum_exp_inv_yi_over_alpha,
			sum_exp_zi_over_alpha, sum_exp_inv_zi_over_alpha,
			pNLP->m_pDB, &pNLP->m_usePin, pNLP->_expPins, pNLP->_expPinsZ);

		pNLP->m_nets_sum_exp_xi_over_alpha[n]     = sum_exp_xi_over_alpha;
		pNLP->m_nets_sum_exp_yi_over_alpha[n]     = sum_exp_yi_over_alpha;
		pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] = sum_exp_inv_xi_over_alpha;
		pNLP->m_nets_sum_exp_inv_yi_over_alpha[n] = sum_exp_inv_yi_over_alpha;
		
		// kaie 2009-08-29
		if(pNLP->m_bMoveZ)
		{
			pNLP->m_nets_sum_exp_zi_over_alpha[n]	  = sum_exp_zi_over_alpha;
			pNLP->m_nets_sum_exp_inv_zi_over_alpha[n] = sum_exp_inv_zi_over_alpha;
		}
		// @kaie 2009-08-29

		// frank 2022-07-31
		if(param.b3d){
			vector<double> layer_sum_exp_xi_over_alpha;
			vector<double> layer_sum_exp_inv_xi_over_alpha;
			vector<double> layer_sum_exp_yi_over_alpha;
			vector<double> layer_sum_exp_inv_yi_over_alpha;
			vector<double> layer_sum_exp_zi_over_alpha;
			vector<double> layer_sum_exp_inv_zi_over_alpha;
			calc_sum_exp_using_pin_for_layers(
				pNLP->m_pDB->m_nets[n].begin(), pNLP->m_pDB->m_nets[n].end(), x, z, expX, expZ,
				layer_sum_exp_xi_over_alpha, layer_sum_exp_inv_xi_over_alpha,
				layer_sum_exp_yi_over_alpha, layer_sum_exp_inv_yi_over_alpha,
				layer_sum_exp_zi_over_alpha, layer_sum_exp_inv_zi_over_alpha,
				pNLP->m_pDB, &pNLP->m_usePin, pNLP->_expPins, pNLP->_expPinsZ);

			for(int layer=0;layer<param.nlayer;++layer){
				pNLP->m_layer_nets_sum_exp_xi_over_alpha[layer][n]     = layer_sum_exp_xi_over_alpha[layer];
				pNLP->m_layer_nets_sum_exp_yi_over_alpha[layer][n]     = layer_sum_exp_yi_over_alpha[layer];
				pNLP->m_layer_nets_sum_exp_inv_xi_over_alpha[layer][n] = layer_sum_exp_inv_xi_over_alpha[layer];
				pNLP->m_layer_nets_sum_exp_inv_yi_over_alpha[layer][n] = layer_sum_exp_inv_yi_over_alpha[layer];
				// pNLP->m_layer_nets_sum_exp_zi_over_alpha[layer][n]	  	= layer_sum_exp_zi_over_alpha[layer];
				// pNLP->m_layer_nets_sum_exp_inv_zi_over_alpha[layer][n] = layer_sum_exp_inv_zi_over_alpha[layer];
			}
		}

		if( param.bUseWAE ) // (kaie) 2010-10-18 Weighted-Average-Exponential Wirelength Model
		{
			double weighted_sum_exp_xi_over_alpha;
			double weighted_sum_exp_inv_xi_over_alpha;
			double weighted_sum_exp_yi_over_alpha;
			double weighted_sum_exp_inv_yi_over_alpha;
	

			calc_weighted_sum_exp_using_pin(
				pNLP->m_pDB->m_nets[n].begin(), pNLP->m_pDB->m_nets[n].end(), x, expX,
				weighted_sum_exp_xi_over_alpha, weighted_sum_exp_inv_xi_over_alpha,
				weighted_sum_exp_yi_over_alpha, weighted_sum_exp_inv_yi_over_alpha,
				pNLP->m_pDB, &pNLP->m_usePin, pNLP->_expPins);

			pNLP->m_nets_weighted_sum_exp_xi_over_alpha[n]     = weighted_sum_exp_xi_over_alpha;
			pNLP->m_nets_weighted_sum_exp_yi_over_alpha[n]     = weighted_sum_exp_yi_over_alpha;
			pNLP->m_nets_weighted_sum_exp_inv_xi_over_alpha[n] = weighted_sum_exp_inv_xi_over_alpha;
			pNLP->m_nets_weighted_sum_exp_inv_yi_over_alpha[n] = weighted_sum_exp_inv_yi_over_alpha;

			if(param.b3d){
				vector<double> layer_weighted_sum_exp_xi_over_alpha;
				vector<double> layer_weighted_sum_exp_inv_xi_over_alpha;
				vector<double> layer_weighted_sum_exp_yi_over_alpha;
				vector<double> layer_weighted_sum_exp_inv_yi_over_alpha;
				calc_weighted_sum_exp_using_pin_for_layers(
					pNLP->m_pDB->m_nets[n].begin(), pNLP->m_pDB->m_nets[n].end(), x, expX, z, expZ,
					layer_weighted_sum_exp_xi_over_alpha, layer_weighted_sum_exp_inv_xi_over_alpha,
					layer_weighted_sum_exp_yi_over_alpha, layer_weighted_sum_exp_inv_yi_over_alpha,
					pNLP->m_pDB, &pNLP->m_usePin, pNLP->_expPins);
				
				for(int layer=0;layer<param.nlayer;++layer){
					pNLP->m_layer_nets_weighted_sum_exp_xi_over_alpha[layer][n]     = layer_weighted_sum_exp_xi_over_alpha[layer];
					pNLP->m_layer_nets_weighted_sum_exp_yi_over_alpha[layer][n]     = layer_weighted_sum_exp_yi_over_alpha[layer];
					pNLP->m_layer_nets_weighted_sum_exp_inv_xi_over_alpha[layer][n] = layer_weighted_sum_exp_inv_xi_over_alpha[layer];
					pNLP->m_layer_nets_weighted_sum_exp_inv_yi_over_alpha[layer][n] = layer_weighted_sum_exp_inv_yi_over_alpha[layer];
				}
			}

		}
			
		if( m_bXArch )
		{
			double sum_exp_x_plus_y_over_alpha;
			double sum_exp_x_minus_y_over_alpha;
			double sum_exp_inv_x_plus_y_over_alpha;
			double sum_exp_inv_x_minus_y_over_alpha;

			calc_sum_exp_using_pin_XHPWL(
				pNLP->m_pDB->m_nets[n].begin(), pNLP->m_pDB->m_nets[n].end(), 
				pNLP,
				sum_exp_x_plus_y_over_alpha,     sum_exp_x_minus_y_over_alpha,	// reuse variables
				sum_exp_inv_x_plus_y_over_alpha, sum_exp_inv_x_minus_y_over_alpha );

			pNLP->m_nets_sum_exp_x_plus_y_over_alpha[n]      = sum_exp_x_plus_y_over_alpha;
			pNLP->m_nets_sum_exp_x_minus_y_over_alpha[n]     = sum_exp_x_minus_y_over_alpha;
			pNLP->m_nets_sum_exp_inv_x_plus_y_over_alpha[n]  = sum_exp_inv_x_plus_y_over_alpha;
			pNLP->m_nets_sum_exp_inv_x_minus_y_over_alpha[n] = sum_exp_inv_x_minus_y_over_alpha;
		}
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

	    //// kaie 2009-08-29
	    if(pNLP->m_bMoveZ)
	    {
	    	pNLP->m_nets_sum_p_z_pos[n]	    = pow( pNLP->m_nets_sum_exp_zi_over_alpha[n], 1/pNLP->_alpha-1 );
	    	pNLP->m_nets_sum_p_inv_z_pos[n] = pow( pNLP->m_nets_sum_exp_inv_zi_over_alpha[n], 1/pNLP->_alpha-1 );
	    	pNLP->m_nets_sum_p_z_neg[n]	    = pow( pNLP->m_nets_sum_exp_zi_over_alpha[n], -1/pNLP->_alpha-1 );
	    	pNLP->m_nets_sum_p_inv_z_neg[n] = pow( pNLP->m_nets_sum_exp_inv_zi_over_alpha[n], -1/pNLP->_alpha-1 );
	    }
	    // @kaie 2009-08-29
	    
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

    for( int n=index1; n<index2; n++ )	// for each net
    {
		if( pNLP->m_pDB->m_nets[n].size() == 0 ) continue;
        if( param.bUseWAE )
        {
            if( true == param.bNLPNetWt )
            {
                totalWL +=
                    NetWeightCalc( pNLP->m_pDB->m_nets[n].size() ) *
                    (pNLP->m_nets_weighted_sum_exp_xi_over_alpha[n] / pNLP->m_nets_sum_exp_xi_over_alpha[n] -
                     pNLP->m_nets_weighted_sum_exp_inv_xi_over_alpha[n] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] +
                     m_yWeight * (pNLP->m_nets_weighted_sum_exp_yi_over_alpha[n] / pNLP->m_nets_sum_exp_yi_over_alpha[n] -
                        pNLP->m_nets_weighted_sum_exp_inv_yi_over_alpha[n] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[n]));
            }else if(param.b3d){
				for(int layer=0;layer<param.nlayer;++layer){
					if(pNLP->m_layer_nets_sum_exp_xi_over_alpha[layer][n] != 0 && pNLP->m_layer_nets_sum_exp_inv_xi_over_alpha[layer][n] != 0){
						totalWL += pNLP->m_layer_nets_weighted_sum_exp_xi_over_alpha[layer][n] / pNLP->m_layer_nets_sum_exp_xi_over_alpha[layer][n] -
									pNLP->m_layer_nets_weighted_sum_exp_inv_xi_over_alpha[layer][n] / pNLP->m_layer_nets_sum_exp_inv_xi_over_alpha[layer][n];
					}else{
						//cout << "xWL of Net["<<n<<"] in layer " << layer << " is 0.\n";
					}
					if(pNLP->m_layer_nets_sum_exp_yi_over_alpha[layer][n] != 0 && pNLP->m_layer_nets_sum_exp_inv_yi_over_alpha[layer][n] != 0){
						totalWL += m_yWeight * (pNLP->m_layer_nets_weighted_sum_exp_yi_over_alpha[layer][n] / pNLP->m_layer_nets_sum_exp_yi_over_alpha[layer][n] -
					 							pNLP->m_layer_nets_weighted_sum_exp_inv_yi_over_alpha[layer][n] / pNLP->m_layer_nets_sum_exp_inv_yi_over_alpha[layer][n]);
					}else{
						//cout << "yWL of Net["<<n<<"] in layer " << layer << " is 0.\n";
					}
				}
			}else
            {
                totalWL +=
                    (pNLP->m_nets_weighted_sum_exp_xi_over_alpha[n] / pNLP->m_nets_sum_exp_xi_over_alpha[n] -
                     pNLP->m_nets_weighted_sum_exp_inv_xi_over_alpha[n] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] +
                     m_yWeight * (pNLP->m_nets_weighted_sum_exp_yi_over_alpha[n] / pNLP->m_nets_sum_exp_yi_over_alpha[n] -
                        pNLP->m_nets_weighted_sum_exp_inv_yi_over_alpha[n] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[n]));
            }
        }else if( param.bUseLSE )
		{
			if( false == m_bXArch )
			{
				/*totalWL += 
				log( pNLP->m_nets_sum_exp_xi_over_alpha[n] ) +	    // max(x)
				log( pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] ) +  // -min(x)
				log( pNLP->m_nets_sum_exp_yi_over_alpha[n] ) +	    // max(y)
				log( pNLP->m_nets_sum_exp_inv_yi_over_alpha[n] ) ;  // -min(y)*/
				if ( true == param.bNLPNetWt )
				{
					totalWL += 
					NetWeightCalc( pNLP->m_pDB->m_nets[n].size() ) * 
					(log( pNLP->m_nets_sum_exp_xi_over_alpha[n] ) +	    // max(x)
					log( pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] ) +  // -min(x)
					m_yWeight * (log( pNLP->m_nets_sum_exp_yi_over_alpha[n] ) +	    // max(y)
						log( pNLP->m_nets_sum_exp_inv_yi_over_alpha[n] ) ) ) ;
				}
				// else if(param.b3d && param.nlayer==2){
				// 	totalWL += 
				// 	log( pNLP->m_layer_nets_sum_exp_xi_over_alpha[0][n] ) +	    		// max(x) in die0
				// 	log( pNLP->m_layer_nets_sum_exp_inv_xi_over_alpha[0][n] ) +  		// -min(x) in die0
				// 	m_yWeight * (log( pNLP->m_layer_nets_sum_exp_yi_over_alpha[0][n] ) +// max(y) in die0
				// 	log( pNLP->m_layer_nets_sum_exp_inv_yi_over_alpha[0][n] ) ) +  		// -min(y) in die0
				// 	log( pNLP->m_layer_nets_sum_exp_xi_over_alpha[1][n] ) +	    		// max(x) in die1
				// 	log( pNLP->m_layer_nets_sum_exp_inv_xi_over_alpha[1][n] ) +  		// -min(x) in die1
				// 	m_yWeight * (log( pNLP->m_layer_nets_sum_exp_yi_over_alpha[1][n] ) +// max(y) in die1
				// 	log( pNLP->m_layer_nets_sum_exp_inv_yi_over_alpha[1][n] ) ) ;  		// -min(y) in die1
				// }
				else
				{
					totalWL += 
					log( pNLP->m_nets_sum_exp_xi_over_alpha[n] ) +	    // max(x)
					log( pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] ) +  // -min(x)
					m_yWeight * (log( pNLP->m_nets_sum_exp_yi_over_alpha[n] ) +	    // max(y)
						log( pNLP->m_nets_sum_exp_inv_yi_over_alpha[n] ) ) ;  // -min(y)
				}
			}
			else
			{
			// X-HPWL
			totalWL += 
				(sqrt(2)-1.0) * ( log( pNLP->m_nets_sum_exp_xi_over_alpha[n] ) +	    // max(x)
						log( pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] ) +      // -min(x)
						log( pNLP->m_nets_sum_exp_yi_over_alpha[n] ) +	    // max(y)
						log( pNLP->m_nets_sum_exp_inv_yi_over_alpha[n] )       // -min(y)
						) -
				(sqrt(2)/2.0-1.0) * ( 
						log( pNLP->m_nets_sum_exp_x_plus_y_over_alpha[n] ) +	    // max(x+y)
						log( pNLP->m_nets_sum_exp_inv_x_plus_y_over_alpha[n] ) +      // -min(x+y)
						log( pNLP->m_nets_sum_exp_x_minus_y_over_alpha[n] ) +	    // max(x-y)
						log( pNLP->m_nets_sum_exp_inv_x_minus_y_over_alpha[n] )       // -min(x-y)
						);
			}
		} else
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
    if( param.bUseWAE )
        return totalWL;
    else if( param.bUseLSE )
	return totalWL * pNLP->_alpha;
    else
	return totalWL / pNLP->m_posScale;
}

// (kaie) 2009-10-14
void* MyNLP::GetLogSumExpViaThread( void* arg )   
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    pMsg->pNLP->m_threadResult[pMsg->threadId] = GetLogSumExpVia( 
	    *pMsg->pZ, *pMsg->pExpZ, pMsg->alpha, pMsg->pNLP, pMsg->index1, pMsg->index2 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

double MyNLP::GetLogSumExpVia( const vector<double>& z,	    // unuse
	const vector<double>& expZ, // unuse
	const double& alpha,	    // unuse
	MyNLP* pNLP, int index1, int index2 )
{

    if( index2 > (int)pNLP->m_pDB->m_nets.size() ) // check boundary
	index2 = (int)pNLP->m_pDB->m_nets.size();

    double totalVia = 0;

    for( int n=index1; n<index2; n++ )	// for each net
    {
		if( pNLP->m_pDB->m_nets[n].size() == 0 )
			continue;
		if( param.bUseLSE )
		{
			/*totalVia += 
			log( pNLP->m_nets_sum_exp_zi_over_alpha[n] ) +	    // max(z)
			log( pNLP->m_nets_sum_exp_inv_zi_over_alpha[n] ) ;*/  // -min(z)
			if ( true == param.bNLPNetWt )
			{
			totalVia += 
				NetWeightCalc( pNLP->m_pDB->m_nets[n].size() ) * 
				(log( pNLP->m_nets_sum_exp_zi_over_alpha[n] ) +	    // max(z)
				log( pNLP->m_nets_sum_exp_inv_zi_over_alpha[n] ) );  // -min(z)
			}
			else
			{
				totalVia += 
					log( pNLP->m_nets_sum_exp_xi_over_alpha[n] ) +	    // max(z)
					log( pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] );  // -min(z)
			}
		}
		else
		{
			// LP-norm
			double invAlpha = 1.0 / pNLP->_alpha;
			totalVia += 
			pow( pNLP->m_nets_sum_exp_zi_over_alpha[n], invAlpha ) - 
			pow( pNLP->m_nets_sum_exp_inv_zi_over_alpha[n], -invAlpha );
			/*totalVia += 
			pow( pNLP->m_nets_sum_exp_zi_over_alpha[n], invAlpha ) - 
			pow( pNLP->m_nets_sum_exp_inv_zi_over_alpha[n], -invAlpha );*/
			//assert( !isNaN( totalVia ) );
		}
    }
    if( param.bUseLSE )
	return totalVia * pNLP->_alpha;
    else
	return totalVia / pNLP->m_posScale;
}
// @(kaie) 2009-10-14

bool MyNLP::eval_f(int n, const vector<double>& x, const vector<double>& expX, bool new_x, double& obj_value)
{
    //totalWL = GetLogSumExpWL( x, expX, _alpha, this );
    double totalWL = Parallel( GetLogSumExpWLThread, m_pDB->m_nets.size() );
    double totalVia = 0;
    if(m_bMoveZ)
		totalVia = Parallel( GetLogSumExpViaThread, m_pDB->m_nets.size() );
    gTotalWL = totalWL;
    //gTotalVia = totalVia;
    density = GetDensityPanelty();

    //Brian 2007-06-18
    /*if (param.bCongObj)
      {
      GetDensityPaneltyNet();
      obj_value = (totalWL * _weightWire) + 
      0.5 * (density * _weightDensity) +
      0.5 * m_dDensityNet * m_dWeightCong;   // correct. 
      }
      else
      {*/
    if( bMulti )
    {
		obj_value = (totalWL * _weightWire) + 0.5 * (density) + (totalVia * _weightWire * m_weightTSV);   // correct. 
    }
    else
    {
		obj_value = (totalWL * _weightWire) + 0.5 * (density * _weightDensity) + (totalVia * _weightWire * m_weightTSV)*0.01;   // correct. 
    }
    //}
    //@Brian 2007-06-18

#if 0
    assert( !isNaN( totalWL ) );
    assert( !isNaN( density ) );
#endif

    return true;
}

#if 0
bool MyNLP::eval_f_HPWL(int n, const vector<double>& x, const vector<double>& expX, bool new_x, double& obj_value)
{
    //UpdateBlockPosition( x, this );
    Parallel( UpdateBlockPositionThread, m_pDB->m_modules.size() );
    double totalWL = m_pDB->CalcHPWL();

    density = GetDensityPanelty();

    obj_value = (totalWL * _weightWire) + (density * _weightDensity);

    return true;
}
#endif

void MyNLP::PrintPotentialGrid()
{
    for( int k=(int)m_gridPotential.size()-1; k>=0; k--)
    {
	printf("===layer%d===\n", k);
    	for( int i=(int)m_gridPotential[k].size()-1; i>=0; i-- )
	{
	    for( unsigned int j=0; j<m_gridPotential[k][i].size(); j++ )
		printf( "%4.1f ", (m_gridPotential[k][i][j]-m_expBinPotential[k][i][j])/m_expBinPotential[k][i][j] );
	    printf( "\n" );
    	}
    	printf( "\n\n" );
    }
}


double MyNLP::GetDensityPanelty()
{
    double density = 0;
    for( unsigned int k = 0; k < m_gridPotential.size(); k++)
    {
	for( unsigned int i=0; i<m_gridPotential[k].size(); i++ )
	{
	    for( unsigned int j=0; j<m_gridPotential[k][i].size(); j++ )
	    {
		if( bMulti )
		{

		    double p = m_weightDensity[k][i][j] * 
			( m_gridPotential[k][i][j] - m_expBinPotential[k][i][j] ) *
			( m_gridPotential[k][i][j] - m_expBinPotential[k][i][j] );

		    if( m_skewDensityPenalty1 != 1.0 )
		    {
			if( m_gridPotential[k][i][j] <  m_expBinPotential[k][i][j] )
			    p /= m_skewDensityPenalty2;
			else
			    p *= m_skewDensityPenalty1;
		    }

		    density += p;
		}
		else
		{
		    density += 
			( m_gridPotential[k][i][j] - m_expBinPotential[k][i][j] ) *
			( m_gridPotential[k][i][j] - m_expBinPotential[k][i][j] );
		}
	    }
	}
    }

    return density;
}


void MyNLP::InitModuleNetPinId()
{
    //printf( "Init module-net-pin id\n" );
    m_moduleNetPinId.resize( m_pDB->m_modules.size() );
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	m_moduleNetPinId[i].resize( m_pDB->m_modules[i].m_netsId.size(), -1 );
	for( unsigned int j=0; j<m_pDB->m_modules[i].m_netsId.size(); j++ )
	{
	    int netId = m_pDB->m_modules[i].m_netsId[j];
	    int pinId = -1;
	    for( unsigned int p=0; p<m_pDB->m_nets[netId].size(); p++ )
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

    if(pMsg->pNLP->m_bMoveZ)
    {
    	tStart = seconds();
    	UpdateGradVia( pMsg->pNLP, pMsg->index1, pMsg->index2 );
    	time_via_force += seconds() - tStart;
    }
    
    //for( int i=2*pMsg->index1; i<2*pMsg->index2; i++ )
    for(int i = pMsg->index1; i < pMsg->index2; i++) // index for modules
    {
	//printf("%d, %.2f, %.2f, %.2f, %.2f, %.2f, %.2f\n", i, pMsg->pNLP->grad_wire[2*i], pMsg->pNLP->grad_wire[2*i+1], pMsg->pNLP->grad_potential[2*i], pMsg->pNLP->grad_potential[2*i+1], pMsg->pNLP->grad_via[i], pMsg->pNLP->grad_potentialZ[i]);
	//printf("%d: %lf, %lf, %lf\n", i, f[3*i], f[3*i+1], f[3*i+2]);
	//Brian 2007-04-30 // x and y directions
	if (param.bCongObj)
	{
	    double grad_f_x =  
		pMsg->pNLP->_weightWire    * pMsg->pNLP->grad_wire[2*i] +
		pMsg->pNLP->_weightDensity * pMsg->pNLP->grad_potential[2*i] +
		pMsg->pNLP->m_dWeightCong  * pMsg->pNLP->grad_congestion[2*i];
	    double grad_f_y =
		pMsg->pNLP->_weightWire    * pMsg->pNLP->grad_wire[2*i+1] +
		pMsg->pNLP->_weightDensity * pMsg->pNLP->grad_potential[2*i+1] +
		pMsg->pNLP->m_dWeightCong  * pMsg->pNLP->grad_congestion[2*i+1];
	    if(pMsg->pNLP->m_bMoveZ)
	    {
		double grad_f_z =
		    pMsg->pNLP->_weightWire * pMsg->pNLP->m_weightTSV * pMsg->pNLP->grad_via[i] +
		    pMsg->pNLP->_weightDensity * pMsg->pNLP->grad_potentialZ[i];
		pMsg->pNLP->grad_f[3*i] = grad_f_x;
		pMsg->pNLP->grad_f[3*i+1] = grad_f_y;
		pMsg->pNLP->grad_f[3*i+2] = grad_f_z;
	    }else
	    {
		pMsg->pNLP->grad_f[2*i] = grad_f_x;
		pMsg->pNLP->grad_f[2*i+1] = grad_f_y;
	    }
	}
	else
	{
	    if( bMulti )
	    {
		double grad_f_x =  
		    pMsg->pNLP->_weightWire    * pMsg->pNLP->grad_wire[2*i] +
		    pMsg->pNLP->grad_potential[2*i];
		double grad_f_y =
		    pMsg->pNLP->_weightWire    * pMsg->pNLP->grad_wire[2*i+1] +
		    pMsg->pNLP->grad_potential[2*i+1];
	  	if(pMsg->pNLP->m_bMoveZ)
	    	{
		    // (kaie) 2009-09-12 z direction
		    double grad_f_z =
			pMsg->pNLP->_weightWire * pMsg->pNLP->m_weightTSV * pMsg->pNLP->grad_via[i] +
			pMsg->pNLP->grad_potentialZ[i];
		    // @(kaie) 2009-09-12

		    pMsg->pNLP->grad_f[3*i] = grad_f_x;
		    pMsg->pNLP->grad_f[3*i+1] = grad_f_y;
		    pMsg->pNLP->grad_f[3*i+2] = grad_f_z;
	    	}else
	    	{
		    pMsg->pNLP->grad_f[2*i] = grad_f_x;
		    pMsg->pNLP->grad_f[2*i+1] = grad_f_y;
	    	}
	    }
	    else
	    {
		double grad_f_x =  
		    pMsg->pNLP->_weightWire    * pMsg->pNLP->grad_wire[2*i] +
		    pMsg->pNLP->_weightDensity * pMsg->pNLP->grad_potential[2*i];
		double grad_f_y =
		    pMsg->pNLP->_weightWire    * pMsg->pNLP->grad_wire[2*i+1] +
		    pMsg->pNLP->_weightDensity * pMsg->pNLP->grad_potential[2*i+1];
	    	if(pMsg->pNLP->m_bMoveZ)
	    	{
		    // (kaie) 2009-09-12 z direction
		    double grad_f_z =
			pMsg->pNLP->_weightWire * pMsg->pNLP->m_weightTSV * pMsg->pNLP->grad_via[i] +
			pMsg->pNLP->_weightDensity * pMsg->pNLP->grad_potentialZ[i];
		    // @(kaie) 2009-09-12
		    pMsg->pNLP->grad_f[3*i] = grad_f_x;
		    pMsg->pNLP->grad_f[3*i+1] = grad_f_y;
		    pMsg->pNLP->grad_f[3*i+2] = grad_f_z;
	    	}else
	    	{
		    pMsg->pNLP->grad_f[2*i] = grad_f_x;
		    pMsg->pNLP->grad_f[2*i+1] = grad_f_y;
	    	}
	    }
       	}
        //@Brian 2007-04-30
    }
    
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

#if 0
void MyNLP::ComputeTotalForces( const unsigned int& blockNumber )
{
    // Compute total forces
    for( unsigned int i=0; i<2*blockNumber; i++ )
	grad_f[i] =  
	    _weightWire    * grad_wire[i] +
	    _weightDensity * grad_potential[i]; 
}
#endif

#if 0
void MyNLP::NullifySpreadingForces( double ratio )
{
    vector< pair< double, int > >  forceVector;
    forceVector.resize( m_pDB->m_modules.size() );
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	double strength = 
	    grad_potential[2*i] * grad_potential[2*i] + 
	    grad_potential[2*i+1] * grad_potential[2*i+1];
	forceVector[i] = make_pair< double, int >( strength, i );
    }
    sort( forceVector.begin(), forceVector.end() );

    unsigned int nullifyNumber = static_cast<unsigned int>( m_pDB->m_modules.size() * ratio );
    for( unsigned int i=0; i<nullifyNumber; i++ )
    {
	int index = forceVector.size() - i - 1;
	int moduleId = forceVector[index].second;
	grad_potential[moduleId*2] = 0.0;
	grad_potential[moduleId*2+1] = 0.0;
    }
    ComputeTotalForces( m_pDB->m_modules.size() );
}
#endif

#if 0
void MyNLP::ApplySteinerWireForce()  // Experimental function
{
    // Update PlaceDB
    Parallel( UpdateBlockPositionThread, m_pDB->m_modules.size() );

    unsigned int originalBlockNumber = m_pDB->m_modules.size();

    // Decompose hyperedges
    CSteinerDecomposition* pDec = NULL;
    pDec = new CSteinerDecomposition( *m_pDB );  
    pDec->Update();
    assert( m_pDB->m_modules.size() > originalBlockNumber );

    // Adjust wire force direction
    for( unsigned int i=0; i<originalBlockNumber; i++ )
    {
	int modId = i;
	double modX = m_pDB->m_modules[i].m_x;
	double modY = m_pDB->m_modules[i].m_y;
	double modWidth = m_pDB->m_modules[i].m_width;
	double modHeight = m_pDB->m_modules[i].m_height;

	// original force
	double forceX = grad_wire[2*i];
	double forceY = grad_wire[2*i+1];
	
	int hCount = 0;
	int vCount = 0;
	for( unsigned int j=0; j<m_pDB->m_modules[i].m_netsId.size(); j++ )
	{
	    int netId = m_pDB->m_modules[i].m_netsId[j];
	    assert( m_pDB->m_nets[netId].size() == 2 );

	    int pin1 = m_pDB->m_nets[netId][0];
	    int pin2 = m_pDB->m_nets[netId][1];
	    
	    int anotherPin;
	    if( m_pDB->m_pins[pin1].moduleId == modId )
	    {
		anotherPin = pin2;
		assert( m_pDB->m_pins[pin2].moduleId != modId );
	    }
	    else 
	    {
		anotherPin = pin1;
		assert( m_pDB->m_pins[pin2].moduleId == modId );
	    }
	   
	    if( m_pDB->m_pins[anotherPin].absX > modX + modWidth )
	       hCount++;	
	    else if( m_pDB->m_pins[anotherPin].absX < modX - modWidth )
	       hCount--;	
	    
	    if( m_pDB->m_pins[anotherPin].absY > modY + modHeight )
	       vCount++;	
	    else if( m_pDB->m_pins[anotherPin].absY < modY - modHeight )
	       vCount--;	
	}

	double strength = ( forceX * forceX + forceY * forceY );

	if( vCount == 0 && hCount == 0 )
	{
	    grad_wire[2*i]   = 0;
	    grad_wire[2*i+1] = 0;
	}
	else
	{
	    double scale = strength / ( vCount * vCount + hCount * hCount );
	    grad_wire[2*i]   = -hCount * scale;
	    grad_wire[2*i+1] = -vCount * scale;
	}
    } // for each block

    ComputeTotalForces( originalBlockNumber );

    // Restore hyperedges
    pDec->Restore();  
    delete pDec;
}
#endif

bool MyNLP::eval_grad_f(int n, const vector<double>& x, const vector<double>& expX, bool new_x, vector<double>& grad_f)
{
   
    // grad WL
    if( _weightWire > 0 )	//TEST
	Parallel( UpdateGradWireThread, (int)m_pDB->m_modules.size() );

    // grad Density
    Parallel( UpdateGradPotentialThread, (int)m_pDB->m_modules.size() );
 
    if(m_bMoveZ) 
	Parallel( UpdateGradViaThread, (int)m_pDB->m_modules.size() );
    
    if(m_bMoveZ)
    {
    	if(n%3 != 0) printf("ERROR: Inconsistent module number!\n");
    	n /= 3;
    }else
    {
	if(n%2 != 0) printf("ERROR: Inconsistent module number!\n");
	n /= 2;
    }
    // compute total fouce, x and y directions
    if( bMulti )
    {
	for( int i =0; i<n; i++ )
	{
	    double grad_f_x =  
		_weightWire * grad_wire[2*i] + 
		grad_potential[2*i];
	    double grad_f_y =
		_weightWire * grad_wire[2*i+1] +
		grad_potential[2*i+1];
	    if(m_bMoveZ)
	    {
		// (kaie) 2009-09-12 z direction
		double grad_f_z =
		    _weightWire * m_weightTSV * grad_via[i] +
		    grad_potentialZ[i];
		// @(kaie) 2009-09-12
		grad_f[3*i] = grad_f_x;
		grad_f[3*i+1] = grad_f_y;
		grad_f[3*i+2] = grad_f_z;
	    }else
	    {
		grad_f[2*i] = grad_f_x;
		grad_f[2*i+1] = grad_f_y;
	    }
	}
    }
    else
    {
	for( int i =0; i<n; i++ )
	{
	    double grad_f_x =  
		_weightWire * grad_wire[2*i] +
		_weightDensity * grad_potential[2*i];
	    double grad_f_y = 
		_weightWire * grad_wire[2*i+1] +
		_weightDensity * grad_potential[2*i+1];
	    if(m_bMoveZ)
	    {
		// (kaie) 2009-09-12 z direction
		double grad_f_z =
		    _weightWire * m_weightTSV * grad_via[i] +
		    _weightDensity * grad_potentialZ[i];
		// @(kaie) 2009-09-12
		grad_f[3*i] = grad_f_x;
		grad_f[3*i+1] = grad_f_y;
		grad_f[3*i+2] = grad_f_z;
	    }else
	    {
		grad_f[2*i] = grad_f_x;
		grad_f[2*i+1] = grad_f_y;
	    }
	}
    }
    return true;
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
    for( int i=index1; i<index2; i++ )	// for each block
    {
	if( pNLP->m_pDB->m_modules[i].m_isFixed || pNLP->m_pDB->m_modules[i].m_netsId.size() == 0 )
	    continue;

	//if( pNLP->cellLock[i] == true )
	//    continue;
	
	pNLP->grad_wire[ 2*i ] = 0;
	pNLP->grad_wire[ 2*i+1 ] = 0;

	for( unsigned int j=0; j<pNLP->m_pDB->m_modules[i].m_netsId.size(); j++ )
	{
	    // for each net connecting to the block
	    int netId = pNLP->m_pDB->m_modules[i].m_netsId[j];
	    if( pNLP->m_pDB->m_nets[netId].size() == 0 )  // floating-module
	    	continue;

	    int selfPinId = pNLP->m_moduleNetPinId[i][j];
	    //if( selfPinId == -1 )
	    //	continue;   

            /*double sum_delta_x = 0; // (kaie) 2010-10-19 Weighted-Average-Exponential Wirelength Model
            double sum_delta_inv_x = 0;
            double sum_delta_y = 0;
            double sum_delta_inv_y = 0;*/

            /*if( gArg.CheckExist("WAE") )
            {
                double xx_j = pNLP->x[2*i];
                double yy_j = pNLP->x[2*i+1];
                double exp_xj, exp_yj;
                if( pNLP->m_usePin[i] )
                {
                    xx_j += pNLP->m_pDB->m_pins[ selfPinId ].xOff;
                    yy_j += pNLP->m_pDB->m_pins[ selfPinId ].yOff;
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
                    double xx_k = pNLP->x[ 2*moduleId_k ];
                    double yy_k = pNLP->x[ 2*moduleId_k+1 ];
                    double exp_xk, exp_yk;
                    if( pNLP->m_usePin[ moduleId_k ] )
                    {
                        xx_k += pNLP->m_pDB->m_pins[ pinId_k ].xOff;
                        yy_k += pNLP->m_pDB->m_pins[ pinId_k ].yOff;
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
                }
            }*/
	    
	    if( pNLP->m_usePin[i] )
	    {
		assert( selfPinId != -1 );
                /*if( gArg.CheckExist("WAE") )
                {
                    double T_x = pNLP->_expPins[ 2*selfPinId ] / pNLP->m_nets_sum_exp_xi_over_alpha[netId];
                    double T_y = pNLP->_expPins[ 2*selfPinId+1 ] / pNLP->m_nets_sum_exp_yi_over_alpha[netId];
                    double T_inv_x = 1.0 / pNLP->_expPins[ 2*selfPinId ] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId];
                    double T_inv_y = 1.0 / pNLP->_expPins[ 2*selfPinId+1 ] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId];
                    if ( true == param.bNLPNetWt )
                    {
                        pNLP->grad_wire[ 2*i ] +=
                            NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) *
                            ((T_x - T_x*T_x*sum_delta_x) - (T_inv_x - T_inv_x*T_inv_x*sum_delta_inv_x));
                        pNLP->grad_wire[ 2*i+1 ] +=
                            NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) *
                            ((T_y - T_y*T_y*sum_delta_y) - (T_inv_y - T_inv_y*T_inv_y*sum_delta_inv_y));
                    }
                    else
                    {
                        pNLP->grad_wire[ 2*i ] +=
                            ((T_x - T_x*T_x*sum_delta_x) - (T_inv_x - T_inv_x*T_inv_x*sum_delta_inv_x));
                        pNLP->grad_wire[ 2*i+1 ] +=
                            ((T_y - T_y*T_y*sum_delta_y) - (T_inv_y - T_inv_y*T_inv_y*sum_delta_inv_y));
                    }
                }else */if( param.bUseLSE )
		{
		    if( false == m_bXArch )
		    {
		        if ( true == param.bNLPNetWt )
		        {
		            pNLP->grad_wire[ 2*i ] += 
				        NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) * 
        			    (pNLP->_expPins[ 2*selfPinId ] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
        			    1.0 / pNLP->_expPins[ 2*selfPinId ] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId]) ;
        			pNLP->grad_wire[ 2*i+1 ] += 
        			    NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) * 
        			    (pNLP->_expPins[ 2*selfPinId+1 ] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
        			    1.0 / pNLP->_expPins[ 2*selfPinId+1 ] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId] );
		        }
		        else
		        {
        			pNLP->grad_wire[ 2*i ] += 
        			    pNLP->_expPins[ 2*selfPinId ] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
        			    1.0 / pNLP->_expPins[ 2*selfPinId ] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId];
        			pNLP->grad_wire[ 2*i+1 ] += 
        			    pNLP->_expPins[ 2*selfPinId+1 ] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
        			    1.0 / pNLP->_expPins[ 2*selfPinId+1 ] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId];
	        	}
		    }
		    else
		    {
			pNLP->grad_wire[ 2*i ] += 
			    (sqrt(2.0) - 1.0 ) *
			    (
			    pNLP->_expPins[ 2*selfPinId ] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
			    1.0 / pNLP->_expPins[ 2*selfPinId ] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId]
			    ) -
			    (sqrt(2.0)/2.0 - 1.0) *
			    (
			    pNLP->_expPinXplusY[ selfPinId ] / pNLP->m_nets_sum_exp_x_plus_y_over_alpha[netId] -
			    1.0 / pNLP->_expPinXplusY[ selfPinId ] / pNLP->m_nets_sum_exp_inv_x_plus_y_over_alpha[netId] +
			    pNLP->_expPinXminusY[ selfPinId ] / pNLP->m_nets_sum_exp_x_minus_y_over_alpha[netId] -
			    1.0 / pNLP->_expPinXminusY[ selfPinId ] / pNLP->m_nets_sum_exp_inv_x_minus_y_over_alpha[netId] 
			    );
			pNLP->grad_wire[ 2*i+1 ] += 
			    (sqrt(2.0) - 1.0 ) *
			    (
			    pNLP->_expPins[ 2*selfPinId+1 ] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
			    1.0 / pNLP->_expPins[ 2*selfPinId+1 ] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId]
			    ) -
			    (sqrt(2.0)/2.0 - 1.0) *
			    (
			    pNLP->_expPinXplusY[ selfPinId ] / pNLP->m_nets_sum_exp_x_plus_y_over_alpha[netId] -
			    1.0 / pNLP->_expPinXplusY[ selfPinId ] / pNLP->m_nets_sum_exp_inv_x_plus_y_over_alpha[netId] -
			    pNLP->_expPinXminusY[ selfPinId ] / pNLP->m_nets_sum_exp_x_minus_y_over_alpha[netId] +
			    1.0 / pNLP->_expPinXminusY[ selfPinId ] / pNLP->m_nets_sum_exp_inv_x_minus_y_over_alpha[netId] 
			    );
			
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
		    pNLP->grad_wire[ 2*i ] += 
			pNLP->m_nets_sum_p_x_pos[netId]     * pNLP->_expPins[2*selfPinId] / xx -
			pNLP->m_nets_sum_p_inv_x_neg[netId] / pNLP->_expPins[2*selfPinId] / xx;
		    pNLP->grad_wire[ 2*i+1 ] += 
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
		
                /*if( gArg.CheckExist("WAE") )
                {
                    double T_x = pNLP->_expX[2*i] / pNLP->m_nets_sum_exp_xi_over_alpha[netId];
                    double T_y = pNLP->_expX[2*i+1] / pNLP->m_nets_sum_exp_yi_over_alpha[netId];
                    double T_inv_x = 1.0 / pNLP->_expX[2*i] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId];
                    double T_inv_y = 1.0 / pNLP->_expX[2*i+1] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId];
                    if ( true == param.bNLPNetWt )
                    {
                        pNLP->grad_wire[ 2*i ] =
                            NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) *
                            ((T_x - T_x*T_x*sum_delta_x) - (T_inv_x - T_inv_x*T_inv_x*sum_delta_inv_x));
                        pNLP->grad_wire[ 2*i+1 ] =
                            NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) *
                            ((T_y - T_y*T_y*sum_delta_y) - (T_inv_y - T_inv_y*T_inv_y*sum_delta_inv_y));
                    }
                    else
                    {
                        pNLP->grad_wire[ 2*i ] =
                            ((T_x - T_x*T_x*sum_delta_x) - (T_inv_x - T_inv_x*T_inv_x*sum_delta_inv_x));
                        pNLP->grad_wire[ 2*i+1 ] =
                            ((T_y - T_y*T_y*sum_delta_y) - (T_inv_y - T_inv_y*T_inv_y*sum_delta_inv_y));
                    }
                }else */if( param.bUseLSE )
		{
		    if( false == m_bXArch )
		    {
    		        if ( true == param.bNLPNetWt )
    		        {
    		            pNLP->grad_wire[ 2*i ] += 
    		                NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) * 
            			    (pNLP->_expX[2*i] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
            			    1.0 / pNLP->_expX[2*i] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId]) ;
            			pNLP->grad_wire[ 2*i+1 ] += 
            			    NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) * 
            			    (pNLP->_expX[2*i+1] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
            			    1.0 / pNLP->_expX[2*i+1] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId]);
		        }
		        else
		        {
        			pNLP->grad_wire[ 2*i ] += 
        			    pNLP->_expX[2*i] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
        			    1.0 / pNLP->_expX[2*i] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId];
        			pNLP->grad_wire[ 2*i+1 ] += 
        			    pNLP->_expX[2*i+1] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
        			    1.0 / pNLP->_expX[2*i+1] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId];
			}
		    }
		    else
		    {
			pNLP->grad_wire[ 2*i ] += 
			    (sqrt(2.0) - 1.0 ) *
			    (
			    pNLP->_expX[ 2*i ] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
			    1.0 / pNLP->_expX[ 2*i ] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId]
			    ) -
			    (sqrt(2.0)/2.0 - 1.0) *
			    (
			    pNLP->_expXplusY[ i ] / pNLP->m_nets_sum_exp_x_plus_y_over_alpha[netId] -
			    1.0 / pNLP->_expXplusY[ i ] / pNLP->m_nets_sum_exp_inv_x_plus_y_over_alpha[netId] +
			    pNLP->_expXminusY[ i ] / pNLP->m_nets_sum_exp_x_minus_y_over_alpha[netId] -
			    1.0 / pNLP->_expXminusY[ i ] / pNLP->m_nets_sum_exp_inv_x_minus_y_over_alpha[netId] 
			    );
			pNLP->grad_wire[ 2*i+1 ] += 
			    (sqrt(2.0) - 1.0 ) *
			    (
			    pNLP->_expX[ 2*i+1 ] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
			    1.0 / pNLP->_expX[ 2*i+1 ] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId]
			    ) -
			    (sqrt(2.0)/2.0 - 1.0) *
			    (
			    pNLP->_expXplusY[ i ] / pNLP->m_nets_sum_exp_x_plus_y_over_alpha[netId] -
			    1.0 / pNLP->_expXplusY[ i ] / pNLP->m_nets_sum_exp_inv_x_plus_y_over_alpha[netId] -
			    pNLP->_expXminusY[ i ] / pNLP->m_nets_sum_exp_x_minus_y_over_alpha[netId] +
			    1.0 / pNLP->_expXminusY[ i ] / pNLP->m_nets_sum_exp_inv_x_minus_y_over_alpha[netId] 
			    );
		    }
		}
		else
		{
		    // Lp-norm
		    double xx = pNLP->x[ 2*i ];
		    double yy = pNLP->x[ 2*i+1 ];
		    xx *= pNLP->m_posScale;
		    yy *= pNLP->m_posScale;
		    pNLP->grad_wire[ 2*i ] += 
			pNLP->m_nets_sum_p_x_pos[netId]     * pNLP->_expX[2*i] / xx  -
			pNLP->m_nets_sum_p_inv_x_neg[netId] / pNLP->_expX[2*i] / xx;
		    pNLP->grad_wire[ 2*i+1 ] += 
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

	} // for each pin in the module
    } // for each module
    
    // 2006-09-27 Y-weight (donnie)
    for( int i=index1; i<index2; i++ )	// for each block
	pNLP->grad_wire[ 2*i+1 ] *= m_yWeight;
}

// kaie
// static
void* MyNLP::UpdateGradViaThread( void* arg )   
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    UpdateGradVia( pMsg->pNLP, pMsg->index1, pMsg->index2 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

//static
void MyNLP::UpdateGradVia( MyNLP* pNLP, int index1, int index2 )
{
    if( index2 > (int)pNLP->m_pDB->m_modules.size() )
	index2 = (int)pNLP->m_pDB->m_modules.size();
    for( int i=index1; i<index2; i++ )	// for each block
    {
	if( pNLP->m_pDB->m_modules[i].m_isFixed || pNLP->m_pDB->m_modules[i].m_netsId.size() == 0 )
	    continue;

	//if( pNLP->cellLock[i] == true )
	//    continue;
	
	pNLP->grad_via[ i ] = 0;

	for( unsigned int j=0; j<pNLP->m_pDB->m_modules[i].m_netsId.size(); j++ )
	{
	    // for each net connecting to the block
	    int netId = pNLP->m_pDB->m_modules[i].m_netsId[j];
	    if( pNLP->m_pDB->m_nets[netId].size() == 0 )  // floating-module
	    	continue;

	    int selfPinId = pNLP->m_moduleNetPinId[i][j];
	    //if( selfPinId == -1 )
	    //	continue;   
	    
	    if( pNLP->m_usePin[i] )
	    {
		assert( selfPinId != -1 );
		if( param.bUseLSE )
		{
		    if ( true == param.bNLPNetWt )
		    {
		        pNLP->grad_via[ i ] += 
			    NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) * 
        		    (pNLP->_expPinsZ[ selfPinId ] / pNLP->m_nets_sum_exp_zi_over_alpha[netId] -
        		    1.0 / pNLP->_expPinsZ[ selfPinId ] / pNLP->m_nets_sum_exp_inv_zi_over_alpha[netId]) ;
		    }
		    else
		    {
        		pNLP->grad_via[ i ] += 
        		    pNLP->_expPinsZ[ selfPinId ] / pNLP->m_nets_sum_exp_zi_over_alpha[netId] -
        		    1.0 / pNLP->_expPinsZ[ selfPinId ] / pNLP->m_nets_sum_exp_inv_zi_over_alpha[netId];
	            }
		}
		else
		{
		    // LP-norm
		    double zz = pNLP->z[i];
		    zz *= pNLP->m_posScale;
		    pNLP->grad_via[ i ] += 
			pNLP->m_nets_sum_p_z_pos[netId]     * pNLP->_expPinsZ[selfPinId] / zz -
			pNLP->m_nets_sum_p_inv_z_neg[netId] / pNLP->_expPinsZ[selfPinId] / zz;
#if 0
		    assert( !isNaN( pNLP->grad_via[ i ] ) );
		    assert( fabs( pNLP->grad_via[ i ] ) < DBL_MAX * 0.95 );
#endif
		}
	    }
	    else
	    {
		// use cell centers
		
		if( param.bUseLSE )
		{
    		    if ( true == param.bNLPNetWt )
    		    {
    		        pNLP->grad_via[ i ] += 
    		            NetWeightCalc( pNLP->m_pDB->m_nets[netId].size() ) * 
            		    (pNLP->_expZ[i] / pNLP->m_nets_sum_exp_zi_over_alpha[netId] -
            		    1.0 / pNLP->_expZ[i] / pNLP->m_nets_sum_exp_inv_zi_over_alpha[netId]) ;
		    }
		    else
		    {
        		pNLP->grad_via[ i ] += 
        		    pNLP->_expZ[i] / pNLP->m_nets_sum_exp_zi_over_alpha[netId] -
        		    1.0 / pNLP->_expZ[i] / pNLP->m_nets_sum_exp_inv_zi_over_alpha[netId];
		    }
		}
		else
		{
		    // Lp-norm
		    double zz = pNLP->z[ i ];
		    zz *= pNLP->m_posScale;
		    pNLP->grad_via[ i ] += 
			pNLP->m_nets_sum_p_z_pos[netId]     * pNLP->_expZ[2*i] / zz  -
			pNLP->m_nets_sum_p_inv_z_neg[netId] / pNLP->_expZ[2*i] / zz;
#if 0
		    assert( !isNaN( pNLP->grad_via[ i ] ) );
		    assert( fabs( pNLP->grad_via[ i ] ) < DBL_MAX * 0.95 );
#endif
		}
	    }

	} // for each pin in the module
	//printf("via force(%d): %lf\n", i, pNLP->grad_via[i]);
    } // for each module
    
}
// @kaie

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
    double gradDensityZ;
    if( index2 > (int)pNLP->m_pDB->m_modules.size() )
	index2 = (int)pNLP->m_pDB->m_modules.size();
    for( int i=index1; i<index2; i++ )	    // for each cell
    {
	if( pNLP->m_pDB->m_modules[i].m_isFixed )
	    continue;
	//if( pNLP->cellLock[i] == true )
	//    continue;

	pNLP->grad_potential[2*i] = 0.0;
	pNLP->grad_potential[2*i+1] = 0.0;
	if(pNLP->m_bMoveZ)
	    pNLP->grad_potentialZ[i] = 0.0;
	
	if( bFast )
	{
	    double width  = pNLP->m_pDB->m_modules[i].GetWidth(pNLP->z[i]-0.5);
	    double height = pNLP->m_pDB->m_modules[i].GetHeight(pNLP->z[i]-0.5);
	    //double thickness = pNLP->m_pDB->m_modules[i].m_thickness;
	    if( height >= pNLP->m_potentialGridHeight || width >= pNLP->m_potentialGridWidth )// || thickness >= pNLP->m_potentialGridThickness )
	    {
			GetPotentialGrad( pNLP->x, pNLP->z, i, gradDensityX, gradDensityY, gradDensityZ, pNLP );
			//printf("%lf, %lf, %lf\n", gradDensityX, gradDensityY, gradDensityZ);
	    }
	    else
	    {
			GetPotentialGradFast( pNLP->x, pNLP->z, i, gradDensityX, gradDensityY, gradDensityZ, pNLP );
			//printf("(Fast) %lf, %lf, %lf\n", gradDensityX, gradDensityY, gradDensityZ);
	    }
	}
	else
	    GetPotentialGrad( pNLP->x, pNLP->z, i, gradDensityX, gradDensityY, gradDensityZ, pNLP );	    // bell-shaped potential

	gradDensityX *= pNLP->_cellPotentialNorm[i];
	gradDensityY *= pNLP->_cellPotentialNorm[i];
	gradDensityZ *= pNLP->_cellPotentialNorm[i];

	// TODO bin-based gradient computation
	
	pNLP->grad_potential[2*i]   += gradDensityX;
	pNLP->grad_potential[2*i+1] += gradDensityY;
	if(pNLP->m_bMoveZ)
	    pNLP->grad_potentialZ[i] += gradDensityZ;
	//printf( "cell %d  spreading force (%g %g %g)\n", i, gradDensityX, gradDensityY, gradDensityZ);
    } // for each cell
//exit(0);
}


// 2007-07-10 (donnie)
void MyNLP::ComputeBinGrad()
{
    //int size = m_binGradX.size();
    for(int k = 0; k<m_pDB->m_totalLayer; k++)
    {
	int size = m_binGradX[k].size();
    	for( int i=0; i<size; i++ )
	{
	    for( int j=0; j<size; j++ )
	    {
		double x = m_pDB->m_coreRgn.left + i * m_potentialGridWidth;
		double y = m_pDB->m_coreRgn.bottom + j * m_potentialGridHeight;
		double z = m_pDB->m_back + k * m_potentialGridThickness + 0.5 * m_potentialGridThickness;
#if 0
	    assert( i < m_binGradX.size() );
	    assert( i < m_binGradY.size() );
	    assert( j < m_binGradX[i].size() );
	    assert( j < m_binGradY[i].size() );
#endif
		GetPointPotentialGrad( x, y, z, m_binGradX[k][i][j], m_binGradY[k][i][j], m_binGradZ[k][i][j]);
	    }
    	}
    }
#if 0
    // 2007-07-17 (donnie) handle boundary condition
    for( int i=0; i<size; i++ )
    {
	if( m_binGradY[i][0] > 0 )
	    m_binGradY[i][0] = 0;
	if( m_binGradY[i][size-1] < 0 )
	    m_binGradY[i][size-1] = 0;
	if( m_binGradX[0][i] > 0 )
	    m_binGradX[0][i] = 0;
	if( m_binGradX[size-1][i] < 0 )
	    m_binGradX[size-1][i] = 0;
    }
#endif
}

// static 2007-07-10 (donnie)
void MyNLP::GetPotentialGradFast( const vector<double>& x, const vector<double>& z, const int& i, double& gradX, double& gradY, double& gradZ, MyNLP* pNLP  )
{

    if( pNLP->m_pDB->m_modules[i].m_isFixed )
    {
	gradX = 0;
	gradY = 0;
	gradZ = 0;
	return;
    }
    
    double cellX = x[i*2];
    double cellY = x[i*2+1];
    double cellZ = z[i];

    // find 4 bins  (gx, gy) - (gx+1, gy+1)
    int gx, gy, gz;
    pNLP->GetClosestGrid( cellX, cellY, cellZ, gx, gy, gz);

#if 0
    assert( gx >= 0 );
    assert( gy >= 0 );
    assert( gx+1 < (int)pNLP->m_binGradX.size() );
    assert( gy+1 < (int)pNLP->m_binGradX.size() );
#endif
    double gxx = pNLP->m_pDB->m_coreRgn.left + gx * pNLP->m_potentialGridWidth;
    double gyy = pNLP->m_pDB->m_coreRgn.bottom + gy * pNLP->m_potentialGridHeight;
    //double gzz = pNLP->m_pDB->m_back + gz * pNLP->m_potentialGridThickness;
    double alpha = (cellX - gxx) / pNLP->m_potentialGridWidth;
    double beta  = (cellY - gyy) / pNLP->m_potentialGridHeight;
    //double gamma = (cellZ - gzz) / pNLP->m_potentialGridThickness;
#if 0
    assert( alpha >= 0 );
    assert( alpha <= 1.0 );
    assert( beta >= 0 );
    assert( beta <= 1.0 );
#endif

    // interpolate x force


//printf("gx = %d, gy = %d, gz = %d\n", gx, gy, gz);
//printf("%d, %d, %d\n", (int)pNLP->m_binGradX[0].size(), (int)pNLP->m_binGradX[0][0].size(), (int)pNLP->m_binGradX.size());
//printf("%d, %d, %d\n", (int)pNLP->m_gridPotential[0].size(), (int)pNLP->m_gridPotential[0][0].size(), (int)pNLP->m_gridPotential.size());

    gradX = (pNLP->m_binGradX[gz][gx][gy] + 
	alpha * ( pNLP->m_binGradX[gz][gx+1][gy] - pNLP->m_binGradX[gz][gx][gy] ) +
	beta  * ( pNLP->m_binGradX[gz][gx][gy+1] - pNLP->m_binGradX[gz][gx][gy] ) +
	alpha * beta * ( pNLP->m_binGradX[gz][gx][gy] + pNLP->m_binGradX[gz][gx+1][gy+1] - 
		pNLP->m_binGradX[gz][gx][gy+1] - pNLP->m_binGradX[gz][gx+1][gy] ));
    /*if(pNLP->m_bMoveZ)
    {   
	gradX = gradX * (1.0 - gamma) +
	    gamma * (pNLP->m_binGradX[gz+1][gx][gy] +
	    alpha * ( pNLP->m_binGradX[gz+1][gx+1][gy] - pNLP->m_binGradX[gz+1][gx][gy] ) +
	    beta  * ( pNLP->m_binGradX[gz+1][gx][gy+1] - pNLP->m_binGradX[gz+1][gx][gy] ) +
	    alpha * beta * ( pNLP->m_binGradX[gz+1][gx][gy] + pNLP->m_binGradX[gz+1][gx+1][gy+1] -
		pNLP->m_binGradX[gz+1][gx][gy+1] - pNLP->m_binGradX[gz+1][gx+1][gy] ));
    }*/

    // interpolate y force
    gradY = (pNLP->m_binGradY[gz][gx][gy] + 
	alpha * ( pNLP->m_binGradY[gz][gx+1][gy] - pNLP->m_binGradY[gz][gx][gy] ) +
	beta  * ( pNLP->m_binGradY[gz][gx][gy+1] - pNLP->m_binGradY[gz][gx][gy] ) +
	alpha * beta * ( pNLP->m_binGradY[gz][gx][gy] + pNLP->m_binGradY[gz][gx+1][gy+1] - 
		pNLP->m_binGradY[gz][gx][gy+1] - pNLP->m_binGradY[gz][gx+1][gy] ));
    /*if(pNLP->m_bMoveZ)
    { 	
	gradY = gradY * (1.0 - gamma) +
	    gamma * (pNLP->m_binGradY[gz+1][gx][gy] +
	    alpha * ( pNLP->m_binGradY[gz+1][gx+1][gy] - pNLP->m_binGradY[gz+1][gx][gy] ) +
	    beta  * ( pNLP->m_binGradY[gz+1][gx][gy+1] - pNLP->m_binGradY[gz+1][gx][gy] ) +
	    alpha * beta * ( pNLP->m_binGradY[gz+1][gx][gy] + pNLP->m_binGradY[gz+1][gx+1][gy+1] -
		pNLP->m_binGradY[gz+1][gx][gy+1] - pNLP->m_binGradY[gz+1][gx+1][gy] ));
    }*/

    // interpolate z force
    if(pNLP->m_bMoveZ)
    {
	gradZ = 0;/*(1.0-gamma) * (pNLP->m_binGradZ[gz][gx][gy] +
	    alpha * ( pNLP->m_binGradZ[gz][gx+1][gy] - pNLP->m_binGradZ[gz][gx][gy] ) +
	    beta  * ( pNLP->m_binGradZ[gz][gx][gy+1] - pNLP->m_binGradZ[gz][gx][gy] ) +
	    alpha * beta * ( pNLP->m_binGradZ[gz][gx][gy] + pNLP->m_binGradZ[gz][gx+1][gy+1] -
		pNLP->m_binGradZ[gz][gx][gy+1] - pNLP->m_binGradZ[gz][gx+1][gy] )) +
	    gamma * (pNLP->m_binGradZ[gz+1][gx][gy] +
	    alpha * ( pNLP->m_binGradZ[gz+1][gx+1][gy] - pNLP->m_binGradZ[gz+1][gx][gy] ) +
	    beta  * ( pNLP->m_binGradZ[gz+1][gx][gy+1] - pNLP->m_binGradZ[gz+1][gx][gy] ) +
	    alpha * beta * ( pNLP->m_binGradZ[gz+1][gx][gy] + pNLP->m_binGradZ[gz+1][gx+1][gy+1] -
		pNLP->m_binGradZ[gz+1][gx][gy+1] - pNLP->m_binGradZ[gz+1][gx+1][gy] ));*/
    }
    
#if 0
    // TEST
    if( cellY + pNLP->m_pDB->m_rowHeight * 2 > pNLP->m_pDB->m_coreRgn.top )
    {
	assert( gy+1 < pNLP->m_binGradX[gx].size() );	
	assert( gy+1 < pNLP->m_binGradX[gx+1].size() );	
	assert( gy+1 < pNLP->m_binGradY[gx].size() );	
	assert( gy+1 < pNLP->m_binGradY[gx+1].size() );	
	// top boundary
	printf( " cell %d     x forces %g %g      y forces %g %g\n",  
		i, pNLP->m_binGradX[gx][gy+1], pNLP->m_binGradX[gx+1][gy+1],
		pNLP->m_binGradY[gx][gy+1], pNLP->m_binGradY[gx+1][gy+1] );
	printf( "                         %g %g               %g %g\n", 
		pNLP->m_binGradX[gx][gy], pNLP->m_binGradX[gx+1][gy],
		pNLP->m_binGradY[gx][gy], pNLP->m_binGradY[gx+1][gy] );
    }
#endif
}

// (donnie) 2007-07-10
void MyNLP::GetPointPotentialGrad( double cellX, double cellY, double cellZ, double& gradX, double& gradY, double& gradZ)
{
    MyNLP* pNLP = this;
    double left   = cellX - pNLP->_potentialRX;
    double bottom = cellY - pNLP->_potentialRY;
    double back   = cellZ - pNLP->_potentialRZ;
    double right  = cellX + ( cellX - left );
    double top    = cellY + ( cellY - bottom );
    double front  = cellZ + ( cellZ - back );
    if( left   < pNLP->m_pDB->m_coreRgn.left  - pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth )     
	left   = pNLP->m_pDB->m_coreRgn.left  - pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
    if( right  > pNLP->m_pDB->m_coreRgn.right + pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth )    
	right  = pNLP->m_pDB->m_coreRgn.right + pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
    if( bottom < pNLP->m_pDB->m_coreRgn.bottom - pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight )   
	bottom = pNLP->m_pDB->m_coreRgn.bottom - pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
    if( top    > pNLP->m_pDB->m_coreRgn.top    + pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight )      
	top    = pNLP->m_pDB->m_coreRgn.top    + pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
    if( back   < pNLP->m_pDB->m_back - pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness )
	back   = pNLP->m_pDB->m_back - pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness;
    if( front  > pNLP->m_pDB->m_front + pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness )
	front  = pNLP->m_pDB->m_front + pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness;
    
    int gx, gy, gz;
    pNLP->GetClosestGrid( left, bottom, back, gx, gy, gz);

#if 0
    assert( gx >= 0 );
    assert( gy >= 0 );
    assert( gx < (int)pNLP->m_gridPotential.size() );
    assert( gy < (int)pNLP->m_gridPotential.size() );
#endif

    const double width = 0;
    const double height = 0;
    const double thickness = 0;

    int gxx, gyy, gzz;
    double xx, yy, zz;
    gradX = 0.0;	
    gradY = 0.0;
    gradZ = 0.0;

    for( gzz = gz, zz = pNLP->GetZGrid( gz ); zz <= front && gzz<(int)pNLP->m_gridPotential.size();
		    gzz++, zz += pNLP->m_potentialGridThickness )
    {
    	for( gxx = gx, xx = pNLP->GetXGrid( gx ); xx <= right && gxx<(int)pNLP->m_gridPotential[gzz].size(); 
			gxx++, xx += pNLP->m_potentialGridWidth )
    	{
	    for( gyy = gy, yy = pNLP->GetYGrid( gy ); yy <= top && gyy<(int)pNLP->m_gridPotential[gzz][gxx].size(); 
			gyy++, yy += pNLP->m_potentialGridHeight )
	    {
		double gX =
		    GetGradPotential( cellX, xx, pNLP->_potentialRX, width ) *
		    GetPotential(     cellY, yy, pNLP->_potentialRY, height ) *
		    GetPotential(     cellZ, zz, pNLP->_potentialRZ, thickness ) *
		    ( pNLP->m_gridPotential[gzz][gxx][gyy] - pNLP->m_expBinPotential[gzz][gxx][gyy] );
	    	double gY =
		    GetPotential(     cellX, xx, pNLP->_potentialRX, width  ) *
		    GetGradPotential( cellY, yy, pNLP->_potentialRY, height ) *
		    GetPotential(     cellZ, zz, pNLP->_potentialRZ, thickness ) *
		    ( pNLP->m_gridPotential[gzz][gxx][gyy] - pNLP->m_expBinPotential[gzz][gxx][gyy] );
	    	double gZ = 0;
		if(pNLP->m_bMoveZ)
		{ 
		    gZ = 
		    	GetPotential(     cellX, xx, pNLP->_potentialRX, width  ) *
		    	GetPotential(     cellY, yy, pNLP->_potentialRY, height ) *
		    	GetGradPotential( cellZ, zz, pNLP->_potentialRZ, thickness ) *
		    	( pNLP->m_gridPotential[gzz][gxx][gyy] - pNLP->m_expBinPotential[gzz][gxx][gyy] );
		}
	    
	    	if( m_skewDensityPenalty1 != 1.0 )
	    	{
		    if( pNLP->m_gridPotential[gzz][gxx][gyy] < pNLP->m_expBinPotential[gzz][gxx][gyy] )
		    	gX /= m_skewDensityPenalty2;
		    else
		    	gX *= m_skewDensityPenalty1;
		    if( pNLP->m_gridPotential[gzz][gxx][gyy] < pNLP->m_expBinPotential[gzz][gxx][gyy] )
		    	gY /= m_skewDensityPenalty2;
		    else
		    	gY *= m_skewDensityPenalty1;
		    if(pNLP->m_bMoveZ)
		    {
		    	if( pNLP->m_gridPotential[gzz][gxx][gyy] < pNLP->m_expBinPotential[gzz][gxx][gyy] )
		    	    gZ /= m_skewDensityPenalty2;
		    	else
		    	    gZ *= m_skewDensityPenalty1;
		    }
	    	}
	    
	    	if( bMulti )
	    	{
		    gradX += gX * m_weightDensity[gzz][gxx][gyy];
		    gradY += gY * m_weightDensity[gzz][gxx][gyy];
		    if(pNLP->m_bMoveZ)
		    	gradZ += gZ * m_weightDensity[gzz][gxx][gyy];
	    	}
	    	else
	    	{		
		    gradX += gX;
		    gradY += gY;
		    if(pNLP->m_bMoveZ)
		    	gradZ += gZ;
	    	}
	    }
	}
    } // for each grid
}



// static
void MyNLP::GetPotentialGrad( const vector<double>& x, const vector<double>& z, const int& i, double& gradX, double& gradY, double& gradZ, MyNLP* pNLP )
{
    double cellX = x[i*2];
    double cellY = x[i*2+1];
    double cellZ = z[i];
   
    double width  = pNLP->m_pDB->m_modules[i].GetWidth(z[i]-0.5);
    double height = pNLP->m_pDB->m_modules[i].GetHeight(z[i]-0.5);
    double thickness = pNLP->m_pDB->m_modules[i].m_thickness;
    //// use square to model small std-cells
    if( height < pNLP->m_potentialGridHeight && width < pNLP->m_potentialGridWidth && thickness < pNLP->m_potentialGridThickness )
	width = height = thickness = 0;
   
    
    double left   = cellX - width  * 0.5 - pNLP->_potentialRX;
    double bottom = cellY - height * 0.5 - pNLP->_potentialRY;
    double back   = cellZ - thickness * 0.5 - pNLP->_potentialRZ;
    double right  = cellX + ( cellX - left );
    double top    = cellY + ( cellY - bottom );
    double front  = cellZ + ( cellZ - back );;

    if( left   < pNLP->m_pDB->m_coreRgn.left  - pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth )     
	left   = pNLP->m_pDB->m_coreRgn.left  - pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
    if( right  > pNLP->m_pDB->m_coreRgn.right + pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth )    
	right  = pNLP->m_pDB->m_coreRgn.right + pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
    if( bottom < pNLP->m_pDB->m_coreRgn.bottom - pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight )   
	bottom = pNLP->m_pDB->m_coreRgn.bottom - pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
    if( top    > pNLP->m_pDB->m_coreRgn.top    + pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight )      
	top    = pNLP->m_pDB->m_coreRgn.top    + pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
    if( back   < pNLP->m_pDB->m_back - pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness )
	back   = pNLP->m_pDB->m_back - pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness;
    if( front  > pNLP->m_pDB->m_front + pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness )
	front  = pNLP->m_pDB->m_front + pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness;

    
    int gx, gy, gz;
    pNLP->GetClosestGrid( left, bottom, back, gx, gy, gz );
    
    int gxx, gyy, gzz;
    double xx, yy, zz;
    gradX = 0.0;	
    gradY = 0.0;
    gradZ = 0.0;
    for( gzz = gz, zz = pNLP->GetZGrid( gz ); zz <= front && gzz<(int)pNLP->m_gridPotential.size();
	    gzz++, zz += pNLP->m_potentialGridThickness )
    {
    	for( gxx = gx, xx = pNLP->GetXGrid( gx ); xx <= right && gxx<(int)pNLP->m_gridPotential[gzz].size(); 
		gxx++, xx += pNLP->m_potentialGridWidth )
    	{

	    for( gyy = gy, yy = pNLP->GetYGrid( gy ); yy <= top && gyy<(int)pNLP->m_gridPotential[gzz][gxx].size(); 
		    gyy++, yy += pNLP->m_potentialGridHeight )
	    {

		double gX =
		    GetGradPotential( cellX, xx, pNLP->_potentialRX, width ) *
		    GetPotential(     cellY, yy, pNLP->_potentialRY, height ) *
		    GetPotential(     cellZ, zz, pNLP->_potentialRZ, thickness ) *
		    ( pNLP->m_gridPotential[gzz][gxx][gyy] - pNLP->m_expBinPotential[gzz][gxx][gyy] );
	    	double gY =
		    GetPotential(     cellX, xx, pNLP->_potentialRX, width  ) *
		    GetGradPotential( cellY, yy, pNLP->_potentialRY, height ) *
		    GetPotential(     cellZ, zz, pNLP->_potentialRZ, thickness ) *
		    ( pNLP->m_gridPotential[gzz][gxx][gyy] - pNLP->m_expBinPotential[gzz][gxx][gyy] );
	    	double gZ = 0;
		if(pNLP->m_bMoveZ)
		{
		    gZ =  
		    	GetPotential(     cellX, xx, pNLP->_potentialRX, width  ) *
		    	GetPotential(     cellY, yy, pNLP->_potentialRY, height ) *
		    	GetGradPotential( cellZ, zz, pNLP->_potentialRZ, thickness ) *
		    	( pNLP->m_gridPotential[gzz][gxx][gyy] - pNLP->m_expBinPotential[gzz][gxx][gyy] );
		}
	        /*
	        printf( "cell %d (%g %g) bin %d %d (%g %g) p %g exp %g gradient %g %g\n", 
	          i, cellX, cellY, gxx, gyy, 
	          xx, yy,
	          pNLP->m_gridPotential[gxx][gyy], pNLP->m_expBinPotential[gxx][gyy],
	          gX, gY );
	          */
	    
	    	if( m_skewDensityPenalty1 != 1.0 )
	    	{
		    if( pNLP->m_gridPotential[gzz][gxx][gyy] < pNLP->m_expBinPotential[gzz][gxx][gyy] )
		    	gX /= m_skewDensityPenalty2;
		    else
		    	gX *= m_skewDensityPenalty1;
		    if( pNLP->m_gridPotential[gzz][gxx][gyy] < pNLP->m_expBinPotential[gzz][gxx][gyy] )
		    	gY /= m_skewDensityPenalty2;
		    else
		    	gY *= m_skewDensityPenalty1;
		    if(pNLP->m_bMoveZ)
		    {
		    	if( pNLP->m_gridPotential[gzz][gxx][gyy] < pNLP->m_expBinPotential[gzz][gxx][gyy] )
		    	    gZ /= m_skewDensityPenalty2;
		    	else
		    	    gZ *= m_skewDensityPenalty1;
		    }
	    	}

	    	if( bMulti )
	    	{
		    gradX += gX * m_weightDensity[gzz][gxx][gyy];
		    gradY += gY * m_weightDensity[gzz][gxx][gyy];
		    if(pNLP->m_bMoveZ)
		    	gradZ += gZ * m_weightDensity[gzz][gxx][gyy];
	    	}
	    	else
	    	{
		    gradX += gX;
		    gradY += gY;
		    if(pNLP->m_bMoveZ)
		    	gradZ += gZ;
	    	}
	    }
	}
    } // for each grid
}

// (kaie) 2010-10-18 Weighted-Average-Exponential Wirelength Model
void MyNLP::calc_weighted_sum_exp_using_pin(
	const vector<int>::const_iterator& begin, const vector<int>::const_iterator& end,
	const vector<double>& x, const vector<double>& expX,
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

		if( (*pUsePin)[blockId] /*&& blockId != id*/ )  // macro or self pin
			//if( blockId != id )
		{
			// handle pins
			weighted_sum_exp_xi_over_alpha     += xx * expPins[ 2*pinId ];
			weighted_sum_exp_inv_xi_over_alpha += xx * 1.0 / expPins[ 2*pinId ];
			weighted_sum_exp_yi_over_alpha     += yy * expPins[ 2*pinId+1 ];
			weighted_sum_exp_inv_yi_over_alpha += yy * 1.0 / expPins[ 2*pinId+1 ];
		}
		else
		{
			// use block center
			//assert( expX[2*blockId] != 0);
			//assert( expX[2*blockId+1] != 0 );
			weighted_sum_exp_xi_over_alpha     += xx * expX[2*blockId];
			weighted_sum_exp_inv_xi_over_alpha += xx * 1.0 / expX[2*blockId];
			weighted_sum_exp_yi_over_alpha     += yy * expX[2*blockId+1];
			weighted_sum_exp_inv_yi_over_alpha += yy * 1.0 / expX[2*blockId+1];
		}
	}
}

// (frank) 2022-07-22 3D Weighted-Average-Exponential Wirelength Model
void MyNLP::calc_weighted_sum_exp_using_pin_for_layers(
	const vector<int>::const_iterator& begin, const vector<int>::const_iterator& end,
	const vector<double>& x, const vector<double>& expX,
	const vector<double>& z, const vector<double>& expZ,
	vector<double>& layer_weighted_sum_exp_xi_over_alpha, vector<double>& layer_weighted_sum_exp_inv_xi_over_alpha,
	vector<double>& layer_weighted_sum_exp_yi_over_alpha, vector<double>& layer_weighted_sum_exp_inv_yi_over_alpha,
	const CPlaceDB* pDB, const vector<bool>* pUsePin, const vector<double>& expPins,
	int id )
{
	layer_weighted_sum_exp_xi_over_alpha.resize(param.nlayer, 0);
	layer_weighted_sum_exp_inv_xi_over_alpha.resize(param.nlayer, 0);
	layer_weighted_sum_exp_yi_over_alpha.resize(param.nlayer, 0);
	layer_weighted_sum_exp_inv_yi_over_alpha.resize(param.nlayer, 0);

	vector<int>::const_iterator ite;
	int pinId, pinIndex;
	int blockId;
	for( ite=begin, pinIndex = 0; ite!=end; ++ite, pinIndex++ )
	{
		// for each pin of the net
		pinId   = *ite;
		blockId = pDB->m_pins[ pinId ].moduleId;

		int layer = (z[2*blockId] < pDB->m_dCutline)? 0:1;

		double xx = x[2*blockId];
		double yy = x[2*blockId+1];
		if( (*pUsePin)[blockId] )
		{
			xx += pDB->m_pins[ pinId ].xOff;
			yy += pDB->m_pins[ pinId ].yOff;
		}

		if( (*pUsePin)[blockId] /*&& blockId != id*/ )  // macro or self pin
			//if( blockId != id )
		{
			if(pDB->m_modules[blockId].m_isVia){
				// handle ball
				for(int k=0;k<param.nlayer;++k){
					layer_weighted_sum_exp_xi_over_alpha[k]     += xx * expPins[ 2*pinId ];
					layer_weighted_sum_exp_inv_xi_over_alpha[k] += xx * 1.0 / expPins[ 2*pinId ];
					layer_weighted_sum_exp_yi_over_alpha[k]     += yy * expPins[ 2*pinId+1 ];
					layer_weighted_sum_exp_inv_yi_over_alpha[k] += yy * 1.0 / expPins[ 2*pinId+1 ];
				}
			} else{
				// handle pins
				layer_weighted_sum_exp_xi_over_alpha[layer]     += xx * expPins[ 2*pinId ];
				layer_weighted_sum_exp_inv_xi_over_alpha[layer] += xx * 1.0 / expPins[ 2*pinId ];
				layer_weighted_sum_exp_yi_over_alpha[layer]     += yy * expPins[ 2*pinId+1 ];
				layer_weighted_sum_exp_inv_yi_over_alpha[layer] += yy * 1.0 / expPins[ 2*pinId+1 ];
			}
		}
		else
		{
			// use block center
			//assert( expX[2*blockId] != 0);
			//assert( expX[2*blockId+1] != 0 );
			layer_weighted_sum_exp_xi_over_alpha[layer]     += xx * expX[2*blockId];
			layer_weighted_sum_exp_inv_xi_over_alpha[layer] += xx * 1.0 / expX[2*blockId];
			layer_weighted_sum_exp_yi_over_alpha[layer]     += yy * expX[2*blockId+1];
			layer_weighted_sum_exp_inv_yi_over_alpha[layer] += yy * 1.0 / expX[2*blockId+1];
		}
	}
}

// static
void MyNLP::calc_sum_exp_using_pin( 
	const vector<int>::const_iterator& begin, const vector<int>::const_iterator& end,
	const vector<double>& x, const vector<double>& z, const vector<double>& expX, const vector<double>& expZ,
	double& sum_exp_xi_over_alpha, double& sum_exp_inv_xi_over_alpha,
	double& sum_exp_yi_over_alpha, double& sum_exp_inv_yi_over_alpha, 
        double& sum_exp_zi_over_alpha, double& sum_exp_inv_zi_over_alpha,
	const CPlaceDB* pDB, const vector<bool>* pUsePin, const vector<double>& expPins, const vector<double>& expPinsZ,
        int id	)
{
    sum_exp_xi_over_alpha = 0;
    sum_exp_inv_xi_over_alpha = 0;
    sum_exp_yi_over_alpha = 0;
    sum_exp_inv_yi_over_alpha = 0;
    sum_exp_zi_over_alpha = 0;
    sum_exp_inv_zi_over_alpha = 0;

    vector<int>::const_iterator ite;
    int pinId;
    int blockId;
    for( ite=begin; ite!=end; ++ite )
    {
	// for each pin of the net
	pinId   = *ite;
	blockId = pDB->m_pins[ pinId ].moduleId;
	
	if( (*pUsePin)[blockId] /*&& blockId != id*/ )	// macro or self pin
	//if( blockId != id )	
	{
	    // handle pins
	    sum_exp_xi_over_alpha     += expPins[ 2*pinId ];
	    sum_exp_inv_xi_over_alpha += 1.0 / expPins[ 2*pinId ];
	    sum_exp_yi_over_alpha     += expPins[ 2*pinId+1 ];
	    sum_exp_inv_yi_over_alpha += 1.0 / expPins[ 2*pinId+1 ];
	    if((int)expPinsZ.size() != 0)
	    {
	    	sum_exp_zi_over_alpha     += expPinsZ[ pinId ];
	    	sum_exp_inv_zi_over_alpha += 1.0 / expPinsZ[ pinId ];
	    }
	}
	else
	{
	    // use block center
	    //assert( expX[2*blockId] != 0);
	    //assert( expX[2*blockId+1] != 0 );
	    sum_exp_xi_over_alpha     += expX[2*blockId];
	    sum_exp_inv_xi_over_alpha += 1.0 / expX[2*blockId];
	    sum_exp_yi_over_alpha     += expX[2*blockId+1];
	    sum_exp_inv_yi_over_alpha += 1.0 / expX[2*blockId+1];
	    if((int)expZ.size() != 0)
	    {
	    	sum_exp_zi_over_alpha     += expZ[ blockId ];
	    	sum_exp_inv_zi_over_alpha += 1.0 / expZ[ blockId] ;
	    }
	}
    }
} 
void MyNLP::calc_sum_exp_using_pin_for_layers( 
	const vector<int>::const_iterator& begin, const vector<int>::const_iterator& end,
	const vector<double>& x, const vector<double>& z, const vector<double>& expX, const vector<double>& expZ,
	vector<double>& layer_sum_exp_xi_over_alpha, vector<double>& layer_sum_exp_inv_xi_over_alpha,
	  vector<double>& layer_sum_exp_yi_over_alpha, vector<double>& layer_sum_exp_inv_yi_over_alpha,
	  vector<double>& layer_sum_exp_zi_over_alpha, vector<double>& layer_sum_exp_inv_zi_over_alpha, 
	const CPlaceDB* pDB, const vector<bool>* pUsePin, const vector<double>& expPins, const vector<double>& expPinsZ,
        int id	)
{
    layer_sum_exp_xi_over_alpha.resize(param.nlayer, 0);
    layer_sum_exp_inv_xi_over_alpha.resize(param.nlayer, 0);
    layer_sum_exp_yi_over_alpha.resize(param.nlayer, 0);
    layer_sum_exp_inv_yi_over_alpha.resize(param.nlayer, 0);
    layer_sum_exp_zi_over_alpha.resize(param.nlayer, 0);
    layer_sum_exp_inv_zi_over_alpha.resize(param.nlayer, 0);

    vector<int>::const_iterator ite;
    int pinId;
    int blockId;
    for( ite=begin; ite!=end; ++ite )
    {
		// for each pin of the net
		pinId   = *ite;
		blockId = pDB->m_pins[ pinId ].moduleId;

		int layer = (z[2*blockId] < pDB->m_dCutline)? 0:1;
		
		if( (*pUsePin)[blockId] /*&& blockId != id*/ )	// macro or self pin
		//if( blockId != id )	
		{
			// handle pins
			if(pDB->m_modules[blockId].m_isVia){
				for(int k=0;k>param.nlayer;++k){
					layer_sum_exp_xi_over_alpha[k]     += expPins[ 2*pinId ];
					layer_sum_exp_inv_xi_over_alpha[k] += 1.0 / expPins[ 2*pinId ];
					layer_sum_exp_yi_over_alpha[k]     += expPins[ 2*pinId+1 ];
					layer_sum_exp_inv_yi_over_alpha[k] += 1.0 / expPins[ 2*pinId+1 ];
					layer_sum_exp_zi_over_alpha[k]     += expPinsZ[ pinId ];
					layer_sum_exp_inv_zi_over_alpha[k] += 1.0 / expPinsZ[ pinId ];
				}
			}
			else{
				layer_sum_exp_xi_over_alpha[layer]     += expPins[ 2*pinId ];
				layer_sum_exp_inv_xi_over_alpha[layer] += 1.0 / expPins[ 2*pinId ];
				layer_sum_exp_yi_over_alpha[layer]     += expPins[ 2*pinId+1 ];
				layer_sum_exp_inv_yi_over_alpha[layer] += 1.0 / expPins[ 2*pinId+1 ];
				layer_sum_exp_zi_over_alpha[layer]     += expPinsZ[ pinId ];
				layer_sum_exp_inv_zi_over_alpha[layer] += 1.0 / expPinsZ[ pinId ];
			}
		}
		else
		{
			// use block center
			//assert( expX[2*blockId] != 0);
			//assert( expX[2*blockId+1] != 0 );
			layer_sum_exp_xi_over_alpha[layer]     += expX[2*blockId];
			layer_sum_exp_inv_xi_over_alpha[layer] += 1.0 / expX[2*blockId];
			layer_sum_exp_yi_over_alpha[layer]     += expX[2*blockId+1];
			layer_sum_exp_inv_yi_over_alpha[layer] += 1.0 / expX[2*blockId+1];
			layer_sum_exp_zi_over_alpha[layer]     += expZ[ blockId ];
			layer_sum_exp_inv_zi_over_alpha[layer] += 1.0 / expZ[ blockId] ;
		}
    }
} 


// static 2006-09-12 (donnie)
void MyNLP::calc_sum_exp_using_pin_XHPWL( 
	const vector<int>::const_iterator& begin, const vector<int>::const_iterator& end,
	const MyNLP* pNLP,
	double& sum_exp_x_plus_y_over_alpha,     double& sum_exp_x_minus_y_over_alpha,
	double& sum_exp_inv_x_plus_y_over_alpha, double& sum_exp_inv_x_minus_y_over_alpha
        )
{
    sum_exp_x_plus_y_over_alpha  = 0;
    sum_exp_x_minus_y_over_alpha = 0;
    sum_exp_inv_x_plus_y_over_alpha  = 0;
    sum_exp_inv_x_minus_y_over_alpha = 0;

    vector<int>::const_iterator ite;
    int pinId;
    int blockId;
    for( ite=begin; ite!=end; ++ite )
    {
	// for each pin of the net
	pinId   = *ite;
	blockId = pNLP->m_pDB->m_pins[ pinId ].moduleId;
	
	if( pNLP->m_usePin[blockId] )	// macro or self pin
	{
	    // handle pins
	    sum_exp_x_plus_y_over_alpha      += pNLP->_expPinXplusY[ pinId ];
	    sum_exp_x_minus_y_over_alpha     += pNLP->_expPinXminusY[ pinId ];
	    sum_exp_inv_x_plus_y_over_alpha  += 1.0 / pNLP->_expPinXplusY[ pinId ];
	    sum_exp_inv_x_minus_y_over_alpha += 1.0 / pNLP->_expPinXminusY[ pinId ];
	}
	else
	{
	    // use block center
	    sum_exp_x_plus_y_over_alpha      += pNLP->_expXplusY[ blockId ];
	    sum_exp_x_minus_y_over_alpha     += pNLP->_expXminusY[ blockId ];
	    sum_exp_inv_x_plus_y_over_alpha  += 1.0 / pNLP->_expXplusY[ blockId ];
	    sum_exp_inv_x_minus_y_over_alpha += 1.0 / pNLP->_expXminusY[ blockId ];
	}
    }
} 




void* MyNLP::UpdateBlockPositionThread( void* arg )   
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    UpdateBlockPosition( *pMsg->pX, *pMsg->pZ, pMsg->pNLP, pMsg->index1, pMsg->index2 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static
void MyNLP::UpdateBlockPosition( const vector<double>& x, const vector<double>& z, MyNLP* pNLP, int index1, int index2 )
{
    if( index2 > (int)pNLP->m_pDB->m_modules.size() )
	    index2 = (int)pNLP->m_pDB->m_modules.size();

    /*double core_left = pNLP->m_pDB->m_coreRgn.left;
    double core_right = pNLP->m_pDB->m_coreRgn.right;
    double core_top = pNLP->m_pDB->m_coreRgn.top;
    double core_bottom = pNLP->m_pDB->m_coreRgn.bottom;
    double core_width = core_right - core_left;
    double core_height = core_top - core_bottom;

    double h_center = core_left + core_width * 0.5;
    double v_center = core_bottom + core_height * 0.5;*/

    for( int i=index1; i<index2; i++ )
    {
	if( pNLP->m_pDB->m_modules[i].m_isFixed == false )
	{
	    //printf("%d, %.2f-%.2f-%.2f\n", i, x[i*2], x[i*2+1], z[i]);
	    pNLP->m_pDB->MoveModuleCenter(i, x[i*2], x[i*2+1], z[i]);
	    //pNLP->m_pDB->MoveModuleCenter( i, x[i*2], x[i*2+1] );
	    //pNLP->m_pDB->m_modules[i].m_cz = z[i];*/
	    /*if(pNLP->m_pDB->m_modules[i].m_cx < h_center && pNLP->m_pDB->m_modules[i].m_cy < v_center) // 0
		pNLP->m_pDB->m_modules[i].m_z = 0;
	    else if(pNLP->m_pDB->m_modules[i].m_cx < h_center)    // 1
		pNLP->m_pDB->m_modules[i].m_z = 1;
	    else if(pNLP->m_pDB->m_modules[i].m_cy < v_center)    // 3
		pNLP->m_pDB->m_modules[i].m_z = 3;
	    else
		pNLP->m_pDB->m_modules[i].m_z = 2;*/
	}
    }
}

void MyNLP::CreatePotentialGrid()
{
    //printf( "Create Potential Grid\n" );
    m_gridPotential.clear(); // remove old values
   
    int realGridSize = m_potentialGridSize + m_potentialGridPadding;	// padding
   
    // (donnie) 2007-07-10  add m_binGradX & m_binGradY
    //printf("realGridSize = %d\n", realGridSize);
    
    m_newPotential.resize( m_pDB->m_modules.size() );
    //m_gridPotential.resize( realGridSize );
    //m_basePotential.resize( realGridSize );
    //m_binGradX.resize( realGridSize + 1 );
    //m_binGradY.resize( realGridSize + 1 );
    //m_binGradZ.resize( realGridSize + 1 );
    //m_weightDensity.resize( realGridSize );

    m_gridPotential.resize( m_pDB->m_totalLayer );
    m_basePotential.resize( m_pDB->m_totalLayer );
    m_binGradX.resize( m_pDB->m_totalLayer+1 );
    m_binGradY.resize( m_pDB->m_totalLayer+1 );
    m_binGradZ.resize( m_pDB->m_totalLayer+1 );
    m_weightDensity.resize( m_pDB->m_totalLayer );

    //for( int layer = 0; layer < realGridSize; layer++ )
    for( int layer = 0; layer < m_pDB->m_totalLayer; layer++ )
    {
	m_gridPotential[layer].resize( realGridSize );
	m_basePotential[layer].resize( realGridSize );
	m_binGradX[layer].resize( realGridSize+1 );
	m_binGradY[layer].resize( realGridSize+1 );
	m_binGradZ[layer].resize( realGridSize+1 );
	m_weightDensity[layer].resize( realGridSize );
	for( int i=0; i<realGridSize; i++ )
	{
	    m_basePotential[layer][i].resize( realGridSize, 0 );
	    m_gridPotential[layer][i].resize( realGridSize, 0 );
	    m_binGradX[layer][i].resize( realGridSize+1, 0 );
	    m_binGradY[layer][i].resize( realGridSize+1, 0 );
	    m_binGradZ[layer][i].resize( realGridSize+1, 0 );
	    m_weightDensity[layer][i].resize( realGridSize, 1 );
	}
	m_binGradX[layer][ realGridSize ].resize( realGridSize+1, 0 );
	m_binGradY[layer][ realGridSize ].resize( realGridSize+1, 0 );
	m_binGradZ[layer][ realGridSize ].resize( realGridSize+1, 0 );
    }
    m_binGradX[ m_pDB->m_totalLayer ].resize( realGridSize+1 );
    m_binGradY[ m_pDB->m_totalLayer ].resize( realGridSize+1 );
    m_binGradZ[ m_pDB->m_totalLayer ].resize( realGridSize+1 );
    for(int i = 0; i <= realGridSize; i++)
    {
	    m_binGradX[ m_pDB->m_totalLayer ][i].resize( realGridSize+1, 0 );
	    m_binGradY[ m_pDB->m_totalLayer ][i].resize( realGridSize+1, 0 );
	    m_binGradZ[ m_pDB->m_totalLayer ][i].resize( realGridSize+1, 0 );
    }

    m_potentialGridWidth  = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / m_potentialGridSize;
    m_potentialGridHeight = ( m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom ) / m_potentialGridSize;
    //m_potentialGridThickness = (m_pDB->m_front - m_pDB->m_back ) / (m_potentialGridSize);
    m_potentialGridThickness = (m_pDB->m_front - m_pDB->m_back ) / (m_pDB->m_totalLayer);

    _potentialRX = m_potentialGridWidth  * _potentialGridR;
    _potentialRY = m_potentialGridHeight * _potentialGridR;
    //_potentialRZ = 0.0;
    _potentialRZ = 0.5 * m_potentialGridThickness;
    //_potentialRZ = m_potentialGridThickness * _potentialGridR; 
    
}


void MyNLP::ClearPotentialGrid()
{
    for(int gz = 0; gz < (int)m_gridPotential.size(); gz++)
    {
	for( int gx=0; gx<(int)m_gridPotential[gz].size(); gx++ )
	    fill( m_gridPotential[gz][gx].begin(), m_gridPotential[gz][gx].end(), 0.0 );
    }
}

void MyNLP::UpdateExpBinPotentialTSV( bool showMsg )
{
    /*double totalCellArea = 0;
    for(int i = 0; i < (int)m_pDB->m_modules.size(); i++)
    {
	totalCellArea += m_pDB->m_modules[i].m_area;
    }
    double avgCellArea = totalCellArea / (double)m_pDB->m_modules.size();*/
    double TSVarea = m_pDB->TSVarea;
    //printf("%.0f\n", TSVarea);
    //double TSVsize = 4;
    //gArg.GetDouble("TSVsize", &TSVsize);
    //TSVarea = TSVsize * avgCellArea;
    //TSVarea = 9 * m_pDB->m_rowHeight * m_pDB->m_rowHeight;

    for(int i = 0; i < (int)m_pDB->m_nets.size(); i++) // for each net
    {
	double max_x = m_pDB->m_coreRgn.left, max_y = m_pDB->m_coreRgn.bottom,  max_z = 0;
	double min_x = m_pDB->m_coreRgn.right, min_y = m_pDB->m_coreRgn.top,  min_z = m_pDB->m_totalLayer-1;
	for(int j = 0; j < (int)m_pDB->m_nets[i].size(); j++) // for each pin
	{
	    int pinId = m_pDB->m_nets[i][j];
	    double pin_x, pin_y, pin_z;
	    m_pDB->GetPinLocation(pinId, pin_x, pin_y, pin_z);

	    if(pin_x < min_x) min_x = pin_x;
	    else if(pin_x > max_x) max_x = pin_x;
	    
	    if(pin_y < min_y) min_y = pin_y;
	    else if(pin_y > max_y) max_y = pin_y;
	    
	    if(pin_z < min_z) min_z = pin_z;
	    else if(pin_z > max_z) max_z = pin_z;
	}
	
	//double numberTSV = max_z - min_z;
	//double TSVarea = 4 * m_pDB->m_rowHeight * m_pDB->m_rowHeight;
	int gx_min, gy_min, gz_min;
	int gx_max, gy_max, gz_max;
	GetClosestGrid(min_x, min_y, min_z, gx_min, gy_min, gz_min);
	GetClosestGrid(max_x, max_y, max_z, gx_max, gy_max, gz_max);
	/*gx_max = min(gx_max, (int)m_basePotential[0].size()-1);
	gy_max = min(gy_max, (int)m_basePotential[0][0].size()-1);
	gz_max = min(gz_max, (int)m_basePotential.size()-1);*/
	double avgTSVarea = TSVarea / ( (gx_max-gx_min+1) * (gy_max-gy_min+1) );
	//printf("TSV are:%.2f, Average TSV area:%.2f\n", TSVarea, avgTSVarea);
	for(int gzz = gz_min; gzz <= gz_max; gzz++ )
	{
	    for(int gxx = gx_min; gxx <= gx_max; gxx++ )
	    {
	        for(int gyy = gy_min; gyy <= gy_max; gyy++ )
	        {
		    if(m_expBinPotential[gzz][gxx][gyy] < 1e-10)
			continue;
		    if(m_expBinPotential[gzz][gxx][gyy] < avgTSVarea)
		    {
			//totalFree -= m_expBinPotential[gzz][gxx][gyy];
			m_expBinPotential[gzz][gxx][gyy] = 0;
			//zeroSpaceBin++;
		    }else
		    {
			m_expBinPotential[gzz][gxx][gyy] -= avgTSVarea;
			//totalFree -= avgTSVarea;
		    }	
		}
	    }
	}
    }
}

double MyNLP::UpdateExpBinPotential( double util, bool showMsg)
{
    // Default is -1. Spread to the whole chip
    if( util < 0 ) 
	util = 1.0; // use all space

    m_expBinPotential.resize( m_basePotential.size() );
    double totalFree = 0;
    int zeroSpaceBin = 0;
    for(int k = 0; k < (int)m_expBinPotential.size(); k++)
    {
		m_expBinPotential[k].resize( m_basePotential[k].size() );
		for( unsigned int i=0; i<m_basePotential[k].size(); i++ )
		{
			m_expBinPotential[k][i].resize( m_basePotential[k][i].size() );
			for( unsigned int j=0; j<m_basePotential[k][i].size(); j++ )
			{
				double base = m_basePotential[k][i][j];
				double overlapX = getOverlap( 
					GetXGrid(i)-m_potentialGridWidth*0.5, GetXGrid(i)+m_potentialGridWidth*0.5, 
					m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.right );
				double overlapY = getOverlap( 
					GetYGrid(j)-m_potentialGridHeight*0.5, GetYGrid(j)+m_potentialGridHeight*0.5, 
					m_pDB->m_coreRgn.bottom, m_pDB->m_coreRgn.top );
				double overlapZ = getOverlap(
					GetZGrid(k)-m_potentialGridThickness*0.5, GetZGrid(k)+m_potentialGridThickness*0.5,
					m_pDB->m_back, m_pDB->m_front );
				double realBinVolumn = overlapX * overlapY * overlapZ;
				double free = realBinVolumn - base;
				if( free > 1e-4 )
				{
					m_expBinPotential[k][i][j] = free * util;
					totalFree += m_expBinPotential[k][i][j];
				}
				else
				{
					m_expBinPotential[k][i][j] = 0.0;
					zeroSpaceBin++;
				}
			} 
		}
    }
 
    //printf("%.2f / %.2f\n", m_pDB->m_totalMovableModuleVolumn , totalFree);
    double algUtil = m_pDB->m_totalMovableModuleVolumn / totalFree;
    if( param.bShow && showMsg )
    {
	printf( "PBIN: Zero space bin #= %d (%d%%).  Algorithm utilization= %.4f\n", 
		zeroSpaceBin, 100*zeroSpaceBin/m_potentialGridSize/m_potentialGridSize/m_pDB->m_totalLayer,
		algUtil );
    }

    double alwaysOver = 0.0;
    if( m_targetUtil > 0.0 && m_targetUtil < 1.0 )
    {
	for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
	{
	    if( m_pDB->m_modules[i].m_isFixed )
		continue;
	    if( m_pDB->m_modules[i].GetWidth(z[i]-0.5) >= 2 * m_potentialGridWidth && 
		    m_pDB->m_modules[i].GetHeight(z[i]-0.5) >= 2 * m_potentialGridHeight &&
		    m_pDB->m_modules[i].m_thickness >= 2 * m_potentialGridThickness)
	    {
		alwaysOver += 
		    (m_pDB->m_modules[i].GetWidth(z[i]-0.5) - m_potentialGridWidth ) * 
		    (m_pDB->m_modules[i].GetHeight(z[i]-0.5) - m_potentialGridHeight ) * 
		    (m_pDB->m_modules[i].m_thickness - m_potentialGridThickness ) *
		    (1.0 - m_targetUtil );
	    }
	}
	if( param.bShow )
	    printf( "PBIN: Always over: %.0f (%.1f%%)\n", alwaysOver, alwaysOver/m_pDB->m_totalMovableModuleVolumn*100.0 );
    }
    m_alwaysOverPotential = alwaysOver;

    return algUtil;
}

// Level smoothing
void MyNLP::LevelSmoothBasePotential( const double& delta, const int& layer)
{
    if( delta <= 1.0 )
       return;

    vector< vector< double > > oldPotential = m_basePotential[layer];

    double maxPotential = 0;
    double avgPotential = 0;
    double totalPotential = 0;
    for( unsigned int i=0; i<oldPotential.size(); i++ )
	for( unsigned int j=0; j<oldPotential[i].size(); j++ )
	{
	    totalPotential += oldPotential[i][j];
	    maxPotential = max( maxPotential, oldPotential[i][j] );
	}
    avgPotential = totalPotential / (oldPotential.size() * oldPotential.size() );

    if( totalPotential == 0 )
	return; // no preplaced
    
    // apply TSP-style smoothing
    double newTotalPotential = 0;
    for( unsigned int i=0; i<m_basePotential.size(); i++ )
	for( unsigned int j=0; j<m_basePotential[i].size(); j++ )
	{
	    if( oldPotential[i][j] >= avgPotential )
	    {
		m_basePotential[layer][i][j] = 
		    avgPotential + 
		    pow( ( oldPotential[i][j] - avgPotential ) / maxPotential, delta ) * maxPotential;
	    }
	    else
	    {
		m_basePotential[layer][i][j] = 
		    avgPotential - 
		    pow( ( avgPotential - oldPotential[i][j] ) / maxPotential, delta ) * maxPotential;
	    }
	    newTotalPotential += m_basePotential[layer][i][j];
	}
    
    // normalization
    double ratio = totalPotential / newTotalPotential;
    for( unsigned int i=0; i<m_basePotential[layer].size(); i++ )
	for( unsigned int j=0; j<m_basePotential[layer][i].size(); j++ )
	    m_basePotential[layer][i][j] = m_basePotential[layer][i][j] * ratio;
}

void MyNLP::LevelSmoothBasePotential3D( const double& delta )
{
    if( delta <= 1.0 )
       return;

    vector< vector< vector<double> > > oldPotential = m_basePotential;

    double maxPotential = 0;
    double avgPotential = 0;
    double totalPotential = 0;
    for( unsigned int k=0; k<oldPotential.size(); k++ )
    	for( unsigned int i=0; i<oldPotential[k].size(); i++ )
	    for( unsigned int j=0; j<oldPotential[k][i].size(); j++ )
	    {
	    	totalPotential += oldPotential[k][i][j];
	    	maxPotential = max( maxPotential, oldPotential[k][i][j] );
	    }
    avgPotential = totalPotential / (oldPotential.size() * oldPotential.size() * oldPotential.size() );

    if( totalPotential == 0 )
	return; // no preplaced
    
    // apply TSP-style smoothing
    double newTotalPotential = 0;
    for( unsigned int k=0; k<m_basePotential.size(); k++ )
    	for( unsigned int i=0; i<m_basePotential.size(); i++ )
	    for( unsigned int j=0; j<m_basePotential[i].size(); j++ )
	    {
	    	if( oldPotential[k][i][j] >= avgPotential )
	    	{
		    m_basePotential[k][i][j] = 
		    	avgPotential + 
		    	pow( ( oldPotential[k][i][j]-avgPotential ) / maxPotential, delta ) * maxPotential;
	        }
	        else
	        {
		    m_basePotential[k][i][j] = 
		    	avgPotential - 
		    	pow( ( avgPotential-oldPotential[k][i][j] ) / maxPotential, delta ) * maxPotential;
	    	}
	    	newTotalPotential += m_basePotential[k][i][j];
	    }
    
    // normalization
    double ratio = totalPotential / newTotalPotential;
    for( unsigned int k=0; k<m_basePotential.size(); k++)
    	for( unsigned int i=0; i<m_basePotential[k].size(); i++ )
	    for( unsigned int j=0; j<m_basePotential[k][i].size(); j++ )
	    	m_basePotential[k][i][j] = m_basePotential[k][i][j] * ratio;
}

void MyNLP::UpdatePotentialGridBase( const vector<double>& x, const vector<double>& z)
{
    double binVolumn = m_potentialGridWidth * m_potentialGridHeight * m_potentialGridThickness;
    
    m_binFreeSpace.resize( m_basePotential.size() );
    for(int k = 0; k < (int)m_binFreeSpace.size(); k++)
    {
		m_binFreeSpace[k].resize( m_basePotential[k].size() );
		for( unsigned int i=0; i<m_basePotential[k].size(); i++ )
		{
			fill( m_basePotential[k][i].begin(), m_basePotential[k][i].end(), 0.0 );
			m_binFreeSpace[k][i].resize( m_basePotential[k][i].size() );
			fill( m_binFreeSpace[k][i].begin(), m_binFreeSpace[k][i].end(), binVolumn );
		}
    }

    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
    {
		// for each cell. cell ci coordinate is ( x[i*2], x[i*2+1] )

		if( m_pDB->m_modules[i].m_isFixed == false )
			continue;

		// TODO: BUG when shrinking core?
		//if( m_pDB->m_modules[i].m_isOutCore )
		if( m_pDB->BlockOutCore( i ) )
			continue;	// pads?

		double cellX = x[i*2];
		double cellY = x[i*2+1];
		double cellZ = z[i];
		double width  = m_pDB->m_modules[i].GetWidth(z[i]-0.5);
		double height = m_pDB->m_modules[i].GetHeight(z[i]-0.5);
		double thickness = m_pDB->m_modules[i].m_thickness;

		// exact block range
		double left   = cellX - width * 0.5;  
		double bottom = cellY - height * 0.5; 
		double back   = cellZ - thickness * 0.5;
		double right  = cellX + (cellX - left);
		double top    = cellY + (cellY - bottom);
		double front  = cellZ + (cellZ - back);;

		if( left   < m_pDB->m_coreRgn.left )     
			left   = m_pDB->m_coreRgn.left;
		if( right  > m_pDB->m_coreRgn.right )    
			right  = m_pDB->m_coreRgn.right;
		if( bottom < m_pDB->m_coreRgn.bottom )   
			bottom = m_pDB->m_coreRgn.bottom;
		if( top    > m_pDB->m_coreRgn.top  )      
			top    = m_pDB->m_coreRgn.top;
		if( back   < m_pDB->m_back )
			back   = m_pDB->m_back;
		if( front  > m_pDB->m_front )
			front  = m_pDB->m_front;
		
		int gx, gy, gz;
		GetClosestGrid( left, bottom, back, gx, gy, gz);
		int gxx, gyy, gzz;
		double xx, yy, zz;

		// Exact density for the base potential"
		for( gzz = gz, zz = GetZGrid(gz); zz <= front && gzz < (int)m_basePotential.size();
			gzz++, zz+=m_potentialGridThickness )
		{
			for( gxx = gx, xx = GetXGrid(gx); xx <= right && gxx < (int)m_basePotential[gzz].size(); 
				gxx++, xx+=m_potentialGridWidth )
			{
				for( gyy = gy, yy = GetYGrid(gy); yy <= top && gyy < (int)m_basePotential[gzz][gxx].size(); 
					gyy++, yy+=m_potentialGridHeight )
				{
					m_basePotential[gzz][gxx][gyy] +=
						getOverlap( left, right, xx, xx+m_potentialGridWidth ) * 
						getOverlap( bottom, top, yy, yy+m_potentialGridHeight ) *
					getOverlap( back, front, zz, zz+m_potentialGridThickness);

					m_binFreeSpace[gzz][gxx][gyy] -= 
						getOverlap( left, right, xx, xx+m_potentialGridWidth ) * 
						getOverlap( bottom, top, yy, yy+m_potentialGridHeight ) *
					getOverlap( back, front, zz, zz+m_potentialGridThickness);
					//printf("(%d, %d, %d) %lf, %lf\n", gx, gx, gy, m_basePotential[gzz][gxx][gyy], m_binFreeSpace[gzz][gxx][gyy]);
				}
			}
		}

    } // for each cell

}


// static
void* MyNLP::ComputeNewPotentialGridThread( void* arg )   
{
    ThreadInfo* pMsg = reinterpret_cast<MyNLP::ThreadInfo*>(arg);
    ComputeNewPotentialGrid( *pMsg->pX, *pMsg->pZ, pMsg->pNLP, pMsg->index1, pMsg->index2 );
    if( param.nThread > 1 )
	pthread_exit( NULL );
    return NULL;
}

// static
void MyNLP::ComputeNewPotentialGrid( const vector<double>& x, const vector<double>& z, MyNLP* pNLP, int index1, int index2 )
{
    if( index2 > (int)pNLP->m_pDB->m_modules.size() )
       index2 = (int)pNLP->m_pDB->m_modules.size();

    for( int i=index1; i<index2; i++ )
    {
	//if( pNLP->m_pDB->m_modules[i].m_isOutCore )
	if( pNLP->m_pDB->BlockOutCore( i) )
	    continue;

	// preplaced blocks are stored in m_basePotential
	if( pNLP->m_pDB->m_modules[i].m_isFixed )
	    continue;
	
	double cellX = x[i*2];
	double cellY = x[i*2+1];
	double cellZ = z[i];
	
	double potentialRX = pNLP->_potentialRX;
	double potentialRY = pNLP->_potentialRY;
	double potentialRZ = pNLP->_potentialRZ;
	
	double width  = pNLP->m_pDB->m_modules[i].GetWidth(z[i]-0.5);
	double height = pNLP->m_pDB->m_modules[i].GetHeight(z[i]-0.5);
	double thickness = pNLP->m_pDB->m_modules[i].m_thickness;
	//// (convert to std-cell)
	if( height < pNLP->m_potentialGridHeight && width < pNLP->m_potentialGridWidth)// && thickness < pNLP->m_potentialGridThickness )
	    width = height = 0;//thickness = 0;

	double left   = cellX - width * 0.5  - potentialRX;
	double bottom = cellY - height * 0.5 - potentialRY;
	double back   = cellZ - thickness * 0.5 - potentialRZ;
	double right  = cellX + (cellX - left);
	double top    = cellY + (cellY - bottom);
	double front  = cellZ + (cellZ - back);
	
	//printf("cellZ: %.0f, thickness: %.0f\n", cellZ, thickness);
	//printf("front: %.0f, back: %.0f\n", front, back);

	if( left   < pNLP->m_pDB->m_coreRgn.left  - pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth )     
	    left   = pNLP->m_pDB->m_coreRgn.left  - pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
	if( right  > pNLP->m_pDB->m_coreRgn.right + pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth )    
	    right  = pNLP->m_pDB->m_coreRgn.right + pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
	if( bottom < pNLP->m_pDB->m_coreRgn.bottom - pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight )   
	    bottom = pNLP->m_pDB->m_coreRgn.bottom - pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
	if( top    > pNLP->m_pDB->m_coreRgn.top    + pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight )      
	    top    = pNLP->m_pDB->m_coreRgn.top    + pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
	if( back   < pNLP->m_pDB->m_back - pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness )
	    back   = pNLP->m_pDB->m_back - pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness;
	if( front  > pNLP->m_pDB->m_front + pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness )
	    front  = pNLP->m_pDB->m_front + pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness;
	
	int gx, gy, gz;
	pNLP->GetClosestGrid( left, bottom, back, gx, gy, gz);
	int gxx, gyy, gzz;
        double xx, yy, zz;

	double totalPotential = 0;
	vector< potentialStruct >& potentialList = pNLP->m_newPotential[i];     
        potentialList.clear();	

	for( gzz = gz, zz = pNLP->GetZGrid(gz); zz <= front && gzz<(int)pNLP->m_gridPotential.size();
		gzz++, zz+=pNLP->m_potentialGridThickness )
	{
	    for( gxx = gx, xx = pNLP->GetXGrid(gx); xx <= right && gxx<(int)pNLP->m_gridPotential[gzz].size(); 
		    gxx++, xx+=pNLP->m_potentialGridWidth )
	    {
	    	for( gyy = gy, yy = pNLP->GetYGrid(gy); yy <= top && gyy<(int)pNLP->m_gridPotential[gzz][gxx].size(); 
			gyy++, yy+=pNLP->m_potentialGridHeight )
	    	{
		    double potential = GetPotential( cellX, xx, potentialRX, width ) *
		                   GetPotential( cellY, yy, potentialRY, height ) *
				   GetPotential( cellZ, zz, potentialRZ, thickness );
		    if( potential > 0 )
		    {
		    	totalPotential += potential;
		    	potentialList.push_back( potentialStruct( gxx, gyy, gzz, potential ) );
		    }
	    	}
	    }
	}

	// normalize the potential so that total potential equals the cell area
	double scale = pNLP->m_pDB->m_modules[i].m_area * pNLP->m_pDB->m_modules[i].m_thickness / totalPotential; // kaie 3d
	//printf( "totalPotential = %f\n", totalPotential );
	
	pNLP->_cellPotentialNorm[i] = scale;	    // normalization factor for the cell i
    } // for each cell
}

double MyNLP::GetDensityProjection(const double& z, const double& k)
{
	double d = fabs(z-k);
	if(d <= 0.5) return 1-2*d*d;
	else if(d > 0.5 && d <= 1) return 2*(d-1)*(d-1);
	else return 0.0;
}

double MyNLP::GetGradDensityProjection(const double& z, const double& k)
{
	if(z >= k)
	{
		if( (z-k) <= 0.5) return (-4*z+4*k);
		else if( (z-k) > 0.5 && (z-k) <= 1) return (4*z-4*k-4);
		else return 0.0;
	}else
	{
		if( (k-z) <= 0.5) return (-4*z+4*k);
		else if( (k-z) > 0.5 && (k-z) <= 1) return (-4*k+4*z+4);
		else return 0.0;
	}
}

void MyNLP::UpdatePotentialGrid(const vector<double>& z)
{
    ClearPotentialGrid();
    for( unsigned int i=0; i<m_newPotential.size(); i++ )	// for each cell
    {
	for( unsigned int j=0; j<m_newPotential[i].size(); j++ ) // for each bin
	{
	    int gx = m_newPotential[i][j].gx;
	    int gy = m_newPotential[i][j].gy;
	    int gz = m_newPotential[i][j].gz;

	    m_gridPotential[ gz ][ gx ][ gy ] +=
		m_newPotential[i][j].potential * _cellPotentialNorm[i];
	}
    }
}

/*double MyNLP::GetPotential( const double& x1, const double& x2, const double& r )
{
    double d = fabs( x1 - x2 );

    if( d <= r * 0.5 )
	return 1.0 - 2 * d * d / ( r * r );
    else if( d <= r )
	return 2 * ( d - r ) * ( d - r ) / ( r * r );
    else
	return 0;
}*/

double MyNLP::GetPotential( const double& x1, const double& x2, const double& r, const double& w )
{
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
/*
double MyNLP::GetGradPotential( const double& x1, const double& x2, const double& r )
{
    double d;
    if( x1 >= x2 )  // right half
    {
	d = x1 - x2;	// d >= 0
	if( d <= r * 0.5 )
	    return -4.0 * d / ( r * r );
	else if( d <= r )
	    return +4.0 * ( d - r ) / ( r * r );
	else
	    return 0;
    }
    else    // left half
    {
	d = x2 - x1;	// d >= 0	
	if( d <= r * 0.5 )
	    return +4.0 * d / ( r * r );
	else if( d <= r )
	    return -4.0 * ( d - r ) / ( r * r );
	else
	    return 0;
    }
}*/

double MyNLP::GetGradPotential( const double& x1, const double& x2, const double& r, const double& w )
{
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

/*double MyNLP::GetGradGradPotential( const double& x1, const double& x2, const double& r )
{
    double d = fabs( x1 - x2 );

    if( d <= r * 0.5 )
	return -4.0 / ( r * r );
    else if( d <= r )
	return +4.0 / ( r * r );
    else
	return 0;
}*/
	    
/*void   MyNLP::GetGridCenter( const int& gx, const int& gy, double& x1, double& y1 )
{
    assert( gx <= m_potentialGridSize );
    assert( gy <= m_potentialGridSize );
    assert( gx >= 0 );
    assert( gy >= 0 );
    
    x1 = m_pDB->m_coreRgn.left   + gx * m_potentialGridWidth  + 0.5 * m_potentialGridWidth;
    y1 = m_pDB->m_coreRgn.bottom + gy * m_potentialGridHeight + 0.5 * m_potentialGridHeight; 
}*/

double MyNLP::GetXGrid( const int& gx )
{
    return m_pDB->m_coreRgn.left + (gx - m_potentialGridPadding + 0.5) * m_potentialGridWidth;  
}

double MyNLP::GetYGrid( const int& gy )
{
    return  m_pDB->m_coreRgn.bottom + (gy - m_potentialGridPadding + 0.5) * m_potentialGridHeight;
}

double MyNLP::GetZGrid( const int& gz )
{
    return  m_pDB->m_back + (gz - m_potentialGridPadding + 0.5) * m_potentialGridThickness;
}

void MyNLP::GetClosestGrid( const double& x1, const double& y1, const double& z1, int& gx, int& gy, int& gz) 
{
    double left   = m_pDB->m_coreRgn.left   - m_potentialGridWidth     * m_potentialGridPadding;
    double bottom = m_pDB->m_coreRgn.bottom - m_potentialGridHeight    * m_potentialGridPadding;
    double back   = m_pDB->m_back;// 	    - m_potentialGridThickness * m_potentialGridPadding;
    gx = static_cast<int>( floor( (x1 - left )   / m_potentialGridWidth ) );
    gy = static_cast<int>( floor( (y1 - bottom ) / m_potentialGridHeight ) );
    gz = static_cast<int>( floor( (z1 - back )   / m_potentialGridThickness ) );
    
    if( gx < 0 )
	gx = 0;
    if( gy < 0 )
	gy = 0;
    if( gz < 0 )
	gz = 0;
    
    if( gx > (int)m_gridPotential[0].size() - 1)
	gx = (int)m_gridPotential[0].size() - 1;
    if( gy > (int)m_gridPotential[0][0].size() - 1)
	gy = (int)m_gridPotential[0][0].size() - 1;
    if( gz > (int)m_gridPotential.size() -1 )
	gz = (int)m_gridPotential.size() - 1;

#if 0    
    // DEBUG
    if( gy >= m_gridPotential.size() || gy < 0 )
    {
	printf( "gridHeight= %f, x1= %f, y1= %f, bottom= %f, top= %f, gy= %d\n", 
		m_potentialGridHeight, x1, y1, m_pDB->m_coreRgn.bottom, m_pDB->m_coreRgn.top , gy );
    }
    if( gx >= m_gridPotential.size() || gx < 0)
    {
	printf( "gridWidth = %f, y1 = %f, x1 = %f, left = %f, right = %f, gx = %d\n", 
		m_potentialGridWidth, y1, x1, m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.right, gx );
    }
    assert( gx >= 0 );
    assert( gy >= 0 );
    assert( gx < (int)m_gridPotential.size() );
    assert( gy < (int)m_gridPotential.size() );
#endif
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
    
    if( gx > (int)m_gridPotential[0].size() )
	gx = (int)m_gridPotential[0].size() - 1;
    if( gy > (int)m_gridPotential[0][0].size() )
	gy = (int)m_gridPotential[0][0].size() - 1;

#if 0    
    // DEBUG
    if( gy >= m_gridPotential.size() || gy < 0 )
    {
	printf( "gridHeight= %f, x1= %f, y1= %f, bottom= %f, top= %f, gy= %d\n", 
		m_potentialGridHeight, x1, y1, m_pDB->m_coreRgn.bottom, m_pDB->m_coreRgn.top , gy );
    }
    if( gx >= m_gridPotential.size() || gx < 0)
    {
	printf( "gridWidth = %f, y1 = %f, x1 = %f, left = %f, right = %f, gx = %d\n", 
		m_potentialGridWidth, y1, x1, m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.right, gx );
    }
    assert( gx >= 0 );
    assert( gy >= 0 );
    assert( gx < (int)m_gridPotential.size() );
    assert( gy < (int)m_gridPotential.size() );
#endif
}

void MyNLP::ClearDensityGrid()
{
    for(int k = 0; k < (int)m_gridDensity.size(); k++)
    	for( unsigned int i=0; i<m_gridDensity[k].size(); i++ )
	    for( unsigned int j=0; j<m_gridDensity[k][i].size(); j++ )
	    	m_gridDensity[k][i][j] = 0.0;
}


void MyNLP::UpdateDensityGridSpace( const int& n, const vector<double>& x, const vector<double>& z)
{
    double allSpace = m_gridDensityWidth * m_gridDensityHeight * m_gridDensityThickness;
    for( unsigned int k = 0; k < m_gridDensity.size(); k++)
    	for( unsigned int i=0; i<m_gridDensity[k].size(); i++ )
	    	for( unsigned int j=0; j<m_gridDensity[k][i].size(); j++ )
				m_gridDensitySpace[k][i][j] = allSpace;
   
    
    // for each cell b, update the corresponding bin area
    for( int b=0; b<(int)m_pDB->m_modules.size(); b++ )
    {
		if( false == m_pDB->m_modules[b].m_isFixed )
			continue;

		double w  = m_pDB->m_modules[b].GetWidth(z[b]-0.5);
		double h  = m_pDB->m_modules[b].GetHeight(z[b]-0.5);
		double t  = m_pDB->m_modules[b].m_thickness;
		double left   = x[b*2]   - w * 0.5;
		double bottom = x[b*2+1] - h * 0.5;
		double back   = z[b]     - t * 0.5;
		double right  = left   + w;
		double top    = bottom + h;
		double front  = back   + t;
		
		if( w == 0 || h == 0 )
			continue;
		
		// find nearest bottom-left gird
		int gx = static_cast<int>( floor( (left   - m_pDB->m_coreRgn.left)   / m_gridDensityWidth ) );
		int gy = static_cast<int>( floor( (bottom - m_pDB->m_coreRgn.bottom) / m_gridDensityHeight ) );
		int gz = static_cast<int>( floor( (back   - m_pDB->m_back)           / m_gridDensityThickness ) );
		if( gx < 0 )  gx = 0;
		if( gy < 0 )  gy = 0;
		if( gz < 0 )  gz = 0;

		for( int zOff = gz; zOff < (int)m_gridDensity.size(); zOff++ )
		{
			double binBack = m_pDB->m_back + zOff * m_gridDensityThickness;
			double binFront = binBack + m_gridDensityThickness;
			if( binBack >= binFront )
				break;
			
			for( int xOff = gx; xOff < (int)m_gridDensity[zOff].size(); xOff++ )
			{
				double binLeft  = m_pDB->m_coreRgn.left + xOff * m_gridDensityWidth;
				double binRight = binLeft + m_gridDensityWidth;
				if( binLeft >= right )
					break;
			
				for( int yOff = gy; yOff < (int)m_gridDensity[zOff][xOff].size(); yOff ++ )
				{
					double binBottom = m_pDB->m_coreRgn.bottom + yOff * m_gridDensityHeight;
					double binTop    = binBottom + m_gridDensityHeight;
					if( binBottom >= top )
						break;

					m_gridDensitySpace[zOff][xOff][yOff] -= 
						getOverlap( left, right, binLeft, binRight ) * 
						getOverlap( bottom, top, binBottom, binTop ) *
					getOverlap( back, front, binBack, binFront );
				}
			}
		}

    } // each module

    int zeroSpaceCount = 0;
    m_totalFreeSpace = 0;
    for( unsigned int k = 0; k < m_gridDensity.size(); k++)
	for( unsigned int i=0; i<m_gridDensity[k].size(); i++ )
	    for( unsigned int j=0; j<m_gridDensity[k][i].size(); j++ )
	    {
		if( m_gridDensitySpace[k][i][j] < 1e-5 )
		{
		    m_gridDensitySpace[k][i][j] = 0.0;
		    zeroSpaceCount ++;
		}
		m_totalFreeSpace += m_gridDensitySpace[k][i][j];
	    }
    
    if( param.bShow )
    {
		printf( "DBIN: Zero space bin #= %d.  Total free space= %.0f.\n", zeroSpaceCount, m_totalFreeSpace );
		//printf( "[DB]   total free space: %.0f\n", m_pDB->m_totalFreeSpace );
    }
}


void MyNLP::UpdateDensityGrid( const int& n, const vector<double>& x, const vector<double>& z)
{
    ClearDensityGrid();
    // for each cell b, update the corresponding bin area
    for( unsigned int b=0; b<m_pDB->m_modules.size(); b++ )
    {
		//if(  m_pDB->m_modules[b].m_isOutCore || m_pDB->m_modules[b].m_isFixed )
		if(  m_pDB->BlockOutCore(b) || m_pDB->m_modules[b].m_isFixed )
			continue;

		double w  = m_pDB->m_modules[b].GetWidth(z[b]-0.5);
		double h  = m_pDB->m_modules[b].GetHeight(z[b]-0.5);
		double t  = m_pDB->m_modules[b].m_thickness;

		// rectangle range 
		double left   = x[b*2]   - w * 0.5;
		double bottom = x[b*2+1] - h * 0.5;
		double back   = z[b]     - t * 0.5;
		double right  = left   + w;
		double top    = bottom + h;
		double front  = back   + t;
		
		// find nearest gird
		int gx = static_cast<int>( floor( (left - m_pDB->m_coreRgn.left) / m_gridDensityWidth ) );
		int gy = static_cast<int>( floor( (bottom - m_pDB->m_coreRgn.bottom) / m_gridDensityHeight ) );
		int gz = static_cast<int>( floor( (back - m_pDB->m_back) / m_gridDensityThickness ) );
		if( gx < 0 ) gx = 0;
		if( gy < 0 ) gy = 0;
		if( gz < 0 ) gz = 0;

		// Block is always inside the core region. Do not have to check boundary.
		for( unsigned int zOff = gz; zOff < m_gridDensity.size(); zOff++ )
		{
			double binBack  = m_pDB->m_back + m_gridDensityThickness * zOff;
			double binFront = binBack + m_gridDensityThickness;
			if(binBack >= binFront )
				break;
			
			for( unsigned int xOff = gx; xOff < m_gridDensity[zOff].size(); xOff++ )
			{
				double binLeft  = m_pDB->m_coreRgn.left + m_gridDensityWidth * xOff;
				double binRight = binLeft + m_gridDensityWidth;
				if( binLeft >= right )
				break;
			
				for( unsigned int yOff = gy; yOff < m_gridDensity[zOff][xOff].size(); yOff++ )
				{
				double binBottom = m_pDB->m_coreRgn.bottom + m_gridDensityHeight * yOff;
				double binTop    = binBottom + m_gridDensityHeight;
				if( binBottom >= top )
					break;

				double volumn = 
					getOverlap( left, right, binLeft, binRight ) *
					getOverlap( bottom, top, binBottom, binTop ) *
				getOverlap( back, front, binBack, binFront );

				m_gridDensity[zOff][xOff][yOff] += volumn;
				}
			}
		}
    } // each module
}

void MyNLP::CheckDensityGrid()
{
    double totalDensity = 0;
    for( int k = 0; k < (int)m_gridDensity.size(); k++)
    	for( int i=0; i<(int)m_gridDensity[k].size(); i++ )
	    for( int j=0; j<(int)m_gridDensity[k][i].size(); j++ )
	    	totalDensity += m_gridDensity[k][i][j];

    double totalVolumn = 0;
    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
    {
	//if( m_pDB->m_modules[i].m_isOutCore == false )
	if( m_pDB->BlockOutCore(i) == false )
	    totalVolumn += m_pDB->m_modules[i].m_area * m_pDB->m_modules[i].m_thickness;
    }

    printf( " %f %f\n", totalDensity, totalVolumn );
}

void MyNLP::CreateDensityGrid( int nGrid )
{
    m_gridDensity.resize( nGrid );
    m_gridDensitySpace.resize( nGrid );

    for(int k = 0; k < (int)m_gridDensity.size(); k++)
    {
		m_gridDensity[k].resize( nGrid );
		for( int i=0; i<nGrid; i++ )
			m_gridDensity[k][i].resize( nGrid );
		
		m_gridDensitySpace[k].resize( nGrid );
		for( int i=0; i<nGrid; i++ )
			m_gridDensitySpace[k][i].resize( nGrid );
    }
    
    m_gridDensityWidth  = ( (double)m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / nGrid;
    m_gridDensityHeight = ( (double)m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom ) / nGrid;
    m_gridDensityThickness = ( (double)m_pDB->m_front - m_pDB->m_back ) / nGrid;
    m_gridDensityTarget = m_pDB->m_totalModuleVolumn / ( nGrid * nGrid * nGrid );
    
    //printf( "Density Target Area = %f\n", m_gridDensityTarget );
    //printf( "Design Density = %f\n", m_gridDensityTarget/m_gridDensityWidth/m_gridDensityHeight );
    // 2006-03-21 compute always overflow area
    
    double alwaysOver = 0.0;
    if( m_targetUtil > 0.0 && m_targetUtil < 1.0 )
    {
		for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
		{
			if( m_pDB->m_modules[i].m_isFixed )
				continue;
			if( m_pDB->m_modules[i].GetWidth(z[i]-0.5) >= 2*m_gridDensityWidth && m_pDB->m_modules[i].GetHeight(z[i]-0.5) >= 2*m_gridDensityHeight && m_pDB->m_modules[i].m_thickness >= 2*m_gridDensityThickness )
				alwaysOver += 
					(m_pDB->m_modules[i].GetWidth(z[i]-0.5) - m_gridDensityWidth ) * 
					(m_pDB->m_modules[i].GetHeight(z[i]-0.5) - m_gridDensityHeight ) * 
					(m_pDB->m_modules[i].m_thickness - m_gridDensityThickness ) *
					(1.0 - m_targetUtil );
		}
		if( param.bShow )
			printf( "DBIN: Always over: %.0f (%.1f%%)\n", alwaysOver, alwaysOver/m_pDB->m_totalMovableModuleVolumn*100.0 );
    }
    m_alwaysOverVolumn = alwaysOver;
}

// Get potential/density grid information //////////////////////////////////////////////////////////////////////

double MyNLP::GetMaxDensity()
{
    double maxUtilization = 0;
    double binVolumn = m_gridDensityWidth * m_gridDensityHeight * m_gridDensityThickness;
    for( int k = 0; k < (int)m_gridDensity.size(); k++)
    	for( int i=0; i<(int)m_gridDensity[k].size(); i++ )
	    for( int j=0; j<(int)m_gridDensity[k][i].size(); j++ )
	    {
	    	if( m_gridDensitySpace[k][i][j] > 1e-5 )
	    	{
		    double preplacedVolumn = binVolumn - m_gridDensitySpace[k][i][j];
		    double utilization = ( m_gridDensity[k][i][j] + preplacedVolumn ) / binVolumn;   
		    if( utilization > maxUtilization )
		    	maxUtilization = utilization;
	        }
	    }
    return maxUtilization;
}

#if 0
double MyNLP::GetAvgOverDensity()
{
    double avgDensity = 0;
    int overflowCount = 0;
    for( unsigned int i=0; i<m_gridDensity.size(); i++ )
	for( unsigned int j=0; j<m_gridDensity.size(); j++ )
	    if( m_gridDensity[i][j] > m_gridDensitySpace[i][j] )
	    {
		overflowCount++;
    		avgDensity += m_gridDensity[i][j];
	    }
    return avgDensity / overflowCount / m_gridDensityTarget;
}
#endif

double MyNLP::GetTotalOverDensityLB()
{
    double over = 0;
    for( unsigned int k = 0; k < m_gridDensity.size(); k++)
	for( unsigned int i=0; i<m_gridDensity[k].size(); i++ )
	    for( unsigned int j=0; j<m_gridDensity[k][i].size(); j++ )
	    {
	    	double targetSpace = m_gridDensitySpace[k][i][j] * m_targetUtil;
	    	if( targetSpace > 1e-5 && m_gridDensity[k][i][j]  > targetSpace  )
    		    over += m_gridDensity[k][i][j] - targetSpace;
	    }
    // TODO: remove "1.0"
    return (over -m_alwaysOverVolumn) / (m_pDB->m_totalMovableModuleVolumn) + 1.0; 
}


double MyNLP::GetTotalOverDensity()
{
    double over = 0;
    for(unsigned int k = 0; k < m_gridDensity.size(); k++)
    	for( unsigned int i=0; i<m_gridDensity[k].size(); i++ )
	    for( unsigned int j=0; j<m_gridDensity[k][i].size(); j++ )
	    {
	    	double targetSpace = m_gridDensitySpace[k][i][j] * m_targetUtil;
	    	if( m_gridDensity[k][i][j]  > targetSpace  )
    		    over += m_gridDensity[k][i][j] - targetSpace;
	    }
    // TODO: remove "1.0"
    return ( over - m_alwaysOverVolumn) / (m_pDB->m_totalMovableModuleVolumn) + 1.0; 
}


double MyNLP::GetTotalOverPotential()
{
    double over = 0;
    for(unsigned int k = 0; k < m_gridPotential.size(); k++)
    	for( unsigned int i=0; i<m_gridPotential[k].size(); i++ )
	    for( unsigned int j=0; j<m_gridPotential[k][i].size(); j++ )
	    { 
	    	if( m_gridPotential[k][i][j]  > m_expBinPotential[k][i][j]  )
    		    over += m_gridPotential[k][i][j] - m_expBinPotential[k][i][j];
	    }
    // TODO: remove "1.0"
    return (over - m_alwaysOverPotential) / (m_pDB->m_totalMovableModuleVolumn) + 1.0; 
}

  
double MyNLP::GetNonZeroDensityGridPercent()
{
    double nonZero = 0;
    for(int k = 0; k < (int)m_gridDensity.size(); k++)
    	for( int i=0; i<(int)m_gridDensity[k].size(); i++ )
	    for( int j=0; j<(int)m_gridDensity[k][i].size(); j++ )
	    	if( m_gridDensity[k][i][j] > 0 || m_gridDensitySpace[k][i][j] == 0 )
		    nonZero += 1.0;
    return nonZero / m_gridDensity.size() / m_gridDensity.size() / m_gridDensity.size();
}


double MyNLP::GetNonZeroGridPercent()
{
    double nonZero = 0;
    for(int k = 0; k < (int)m_gridPotential.size(); k++)
    	for( int i=0; i<(int)m_gridPotential[k].size(); i++ )
	    for( int j=0; j<(int)m_gridPotential[k][i].size(); j++ )
	    	if( m_gridPotential[k][i][j] > 0 )
		    nonZero += 1.0;
    return nonZero / m_gridPotential.size() / m_gridPotential.size() / m_gridPotential.size();
}


double MyNLP::GetMaxPotential()
{
    double maxDensity = 0;
    for(unsigned int k = 0; k < m_gridPotential.size(); k++)
    	for( unsigned int i=0; i<m_gridPotential[k].size(); i++ )
	    for( unsigned int j=0; j<m_gridPotential[k][i].size(); j++ )
	    	if( m_gridPotential[k][i][j] > maxDensity )   
		    maxDensity = m_gridPotential[k][i][j];
    return maxDensity;
}


double MyNLP::GetAvgPotential()
{
    const double targetDensity = 1.0;
    double avgDensity = 0;
    int overflowCount = 0;
   
    for(unsigned int k = 0; k < m_gridPotential.size(); k++)
	for( unsigned int i=0; i<m_gridPotential[k].size(); i++ )
	    for( unsigned int j=0; j<m_gridPotential[k][i].size(); j++ )
	    	if( m_gridPotential[k][i][j] > targetDensity )
	        {
		    overflowCount++;
    		    avgDensity += m_gridPotential[k][i][j];
	    	}
    return avgDensity / overflowCount;
}


// Output potential/density grid to a file /////////////////////////////////////////////////////////////////////


// Output potential data for gnuplot
void MyNLP::OutputPotentialGrid( string filename, const int& k)
{
    int stepSize = (int)m_gridPotential[k].size() / 100;
    if( stepSize == 0 )
	stepSize = 1;
    FILE* out = fopen( filename.c_str(), "w" );
    double binArea = m_potentialGridWidth * m_potentialGridHeight;
    for( unsigned int j=0; j<m_gridPotential[k].size(); j+=stepSize )
    {
	for( unsigned int i=0; i<m_gridPotential[k].size(); i+=stepSize )
	    fprintf( out, "%.03f ", (m_gridPotential[k][i][j] + m_basePotential[k][i][j]) / binArea );
	fprintf( out, "\n" );
    }
    fprintf( out, "\n" );
    fclose( out );
}


// Output potential data for gnuplot
void MyNLP::OutputDensityGrid( string filename, const int& k )
{
    int stepSize = 1;
    FILE* out = fopen( filename.c_str(), "w" );
    for( unsigned int j=0; j<m_gridDensity[k].size(); j+=stepSize )
    {
	for( unsigned int i=0; i<m_gridDensity[k].size(); i+=stepSize )
	{
	    double targetSpace = m_gridDensitySpace[k][i][j] * m_targetUtil;
	    if( m_gridDensity[k][i][j] > targetSpace )
	    {
		// % overflow
		fprintf( out, "%.03f ", (m_gridDensity[k][i][j]-targetSpace) / m_pDB->m_totalMovableModuleVolumn * 100 );
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

// double MyNLP::UpdateCongestionBasePotential()
// {
//     //printf( "\n[CMP] Modify base potential\n" );

//     int congBinSize = m_potentialGridSize;
//     m_pDB->CreateCongMap( congBinSize, congBinSize );
//     int overBinNumber = 0;

//     if( m_pGRMap == NULL )
// 	m_pGRMap = new GRouteMap( m_pDB );
//     m_pGRMap->LoadCongMap( m_pDB->m_pCongMap );

//     // ICCAD EXP
//     double minDensity = 0.00;
//     double maxDensity = 0.50;

//     // TEST
//     //double minDensity = 0.20;	 
//     //double maxDensity = 0.45;   

//     // Fill cong[x][y]
//     double totalOver = 0;
//     vector< vector< double > > cong;
//     cong.resize( congBinSize );
//     for( int x=0; x<congBinSize; x++ )
//     {
// 	cong[x].resize( congBinSize );
// 	for( int y=0; y<congBinSize; y++ )
// 	{
// 	    double density = m_pGRMap->GetBinWireDensity( x, y );
// 	    if( density > maxDensity )
// 	    {
// 		totalOver += density - maxDensity;
// 		overBinNumber++;
// 	    }
// 	    density = max( minDensity, density );
// 	    density = min( maxDensity, density );
// 	    cong[x][y] = density - minDensity;
// 	}
//     }

// 	#if 0
// 		// Smooth cong[][]
// 		GaussianSmooth smooth;
// 		int r = m_smoothR;
// 		smooth.Gaussian2D( r, 6*r+1 );
// 		smooth.Smooth( cong );
// 	#endif

//     double smoothMax = 0;
//     double smoothMin = 10.0;
//     double smoothTotal = 0;
//     for( int x=0; x<congBinSize; x++ )
// 	for( int y=0; y<congBinSize; y++ )
// 	{
// 	    smoothMax = max( smoothMax, cong[x][y] );
// 	    smoothMin = min( smoothMin, cong[x][y] );
// 	    smoothTotal += cong[x][y];
// 	}

//     // Shift cong from [smoothMin, smoothMax] to [0, smoothMax-smoothMin]
//     for( int x=0; x<congBinSize; x++ )
// 	for( int y=0; y<congBinSize; y++ )
// 	    cong[x][y] -= smoothMin;
//     smoothTotal -= smoothMin * congBinSize * congBinSize;
//     smoothMax -= smoothMin;
//     smoothMin = 0;

//     double binArea = m_potentialGridWidth * m_potentialGridHeight;
//     double ratio   = m_congWeight * (m_totalFreeSpace - m_pDB->m_totalMovableModuleArea) / smoothTotal;

//     if( gArg.CheckExist( "cmpcmp" ) )
//     {
// 	#if 0
// 		if( totalOver < 1e-10 )	    // no overflow
// 		{
// 			m_basePotential = m_basePotentialOld;
// 			return 0.0;
// 		}
// 	#endif

// 	// 2006-04-04 (donnie) add loop limit
// 	for( unsigned int i=0; i<10; i++ )
// 	{
// 	    double totalAdd = 0;
// 	    for(int k = 0; k < m_potentialGridSize; k++)
// 	    	for( int x=0; x<m_potentialGridSize; x++ )
// 		    for( int y=0; y<m_potentialGridSize; y++ )
// 		    {
// 		    	double bound = max( binArea, m_basePotentialOld[k][x][y] );
// 		    	m_basePotential[k][x][y] = m_basePotentialOld[k][x][y] + ratio * cong[x][y];
// 		    	m_basePotential[k][x][y] = min( m_basePotential[k][x][y], bound );

// 		    	assert( m_basePotential[k][x][y] >= m_basePotentialOld[k][x][y] );
// 		    	totalAdd += m_basePotential[k][x][y] - m_basePotentialOld[k][x][y];
// 		    }
// 	    //printf( "[CMP] Add potential   %.0f\n", totalAdd );
// 	    //printf( "[CMP] Total freespace %.0f\n", m_totalFreeSpace );

// 	    double algUtil = UpdateExpBinPotential( m_targetUtil, false );
// 	    if( algUtil > 0.9 )
// 		break;
// 	    ratio *= 1.1;
// 	}
//     }


// 	#if 0
// 		// Generating figures for the ICCAD-07 submission
// 		for( unsigned int i=0; i<cong.size(); i++ )
// 		for( unsigned int j=0; j<cong[i].size(); j++ )
// 			cong[i][j] *= ratio; 
// 		char filename [100];

// 		vector< vector< double > > plot;
// 		binArea = m_potentialGridWidth * m_potentialGridHeight;

// 		plot = cong;
// 		for( unsigned int i=0; i<plot.size(); i++ )
// 		for( unsigned int j=0; j<plot[i].size(); j++ )
// 			plot[i][j] /= binArea; 
// 		sprintf( filename, "iccad.metal" );
// 		CMatrixPlotter::OutputGnuplotFigure( plot, filename, filename, 1 );

// 		plot = m_basePotentialOld;
// 		for( unsigned int i=0; i<plot.size(); i++ )
// 		for( unsigned int j=0; j<plot[i].size(); j++ )
// 			plot[i][j] /= binArea; 
// 		sprintf( filename, "iccad.preplaced" );
// 		CMatrixPlotter::OutputGnuplotFigure( plot, filename, filename, 1 );
		
// 		plot = m_basePotential;
// 		for( unsigned int i=0; i<plot.size(); i++ )
// 		for( unsigned int j=0; j<plot[i].size(); j++ )
// 			plot[i][j] /= binArea; 
// 		sprintf( filename, "iccad.potential" );
// 		CMatrixPlotter::OutputGnuplotFigure( plot, filename, filename, 1 );
// 		//////////////////////////////
		
// 		printf( "\n[CMP] TBinOver= %5.0f in %4d bins (%.0f%%) AllocWS %5.1f%% \n", 
// 			totalOver, 
// 			overBinNumber, 
// 			100.0*overBinNumber/congBinSize/congBinSize, 
// 			ratio*smoothTotal*100.0/m_totalFreeSpace ); 
// 	#endif 

//     return totalOver;


// }

// double MyNLP::UpdateCongestion()
// {
//     int congBinSize = m_potentialGridSize;
//     m_pDB->CreateCongMap( congBinSize, congBinSize );
//     int overBinNumber = 0;
//     vector< vector< double > > cong;
//     cong.resize( congBinSize );
    
//     if( m_pGRMap == NULL )
// 	m_pGRMap = new GRouteMap( m_pDB );
//     m_pGRMap->LoadCongMap( m_pDB->m_pCongMap );
    
//     // 2007-03-29
//     if( gArg.CheckExist( "cmp" ) )
//     {
// 	//double minDensity = 0.5;	// full of tracks 
// 	double minDensity = 0.4;	 
// 	double maxDensity = 1.0;   

// 	// Fill cong[x][y]
// 	double totalOver = 0;
// 	for( int x=0; x<congBinSize; x++ )
// 	{
// 	    cong[x].resize( congBinSize );
// 	    for( int y=0; y<congBinSize; y++ )
// 	    {
// 		double density = m_pGRMap->GetBinWireDensity( x, y );
// 		density = max( minDensity, density );
// 		density = min( maxDensity, density );
// 		cong[x][y] = density - minDensity;
// 		if( cong[x][y] > 0 )
// 		{
// 		    totalOver += cong[x][y];
// 		    overBinNumber++;
// 		}
// 	    }
// 	}

// 	// Smooth cong[][]
// 	GaussianSmooth smooth;
// 	int r = m_smoothR;
// 	smooth.Gaussian2D( r, 6*r+1 );
// 	smooth.Smooth( cong );

// 	double maxDen = 0;
// 	for( int x=0; x<congBinSize; x++ )
// 	    for( int y=0; y<congBinSize; y++ )
// 		maxDen = max( maxDen, cong[x][y] );

// 	double binArea = m_potentialGridWidth * m_potentialGridHeight;
// 	double ratio1  = m_congWeight * m_totalFreeSpace / totalOver;
// 	double ratio2  = binArea / maxDen;   // expect potential cannot be underflow
// 	double ratio   = min( ratio1, ratio2 );

// 	if( gArg.CheckExist( "cmpcmp" ) )
// 	{
// 	    if( totalOver < 1e-10 )	    // no overflow
// 	    {
// 		m_expBinPotential = m_expBinPotentialOld;
// 		return 0.0;
// 	    }

// 	    for(int layer = 0; layer < 2*m_pDB->m_totalLayer-1; layer++)
// 	    {
// 	    	for( int x=0; x<m_potentialGridSize; x++ )
// 		    for( int y=0; y<m_potentialGridSize; y++ )
// 		    {
// 			m_expBinPotential[layer][x][y] = m_expBinPotentialOld[layer][x][y] - ratio * cong[x][y];
// 		    	// TODO: consider preplaced blocks?
// 		    	m_expBinPotential[layer][x][y] = max( m_expBinPotential[layer][x][y], 0 );
// 		} 
// 	    }
// 	}

// 	printf( "\n    [CMP] TBinOver= %5.0f in %4d bins (%.0f%%) AllocWS %5.1f%% ", 
// 		totalOver, 
// 		overBinNumber, 
// 		100.0*overBinNumber/congBinSize/congBinSize, 
// 		ratio*totalOver*100.0/m_totalFreeSpace ); 

// 	return totalOver;
//     } // cmp



//     // Use crazying's CCongMap

//     double cut = -4.0;  
//     double maxOver = -0.0; 
    
//     // Fill cong[x][y]
//     for( int x=0; x<congBinSize; x++ )
//     {
// 	cong[x].resize( congBinSize );
// 	for( int y=0; y<congBinSize; y++ )
// 	{
// 	    double over = m_pDB->m_pCongMap->GetBinOverflow( x, y );
// 	    if( over > maxOver )
// 		over = maxOver;
// 	    if( over > cut )
// 	    {
// 		overBinNumber++;
// 		cong[x][y] = over;
// 	    }
// 	    else
// 		cong[x][y] = cut;
// 	}
//     }
    
//     // scale: > 0 
//     for( int x=0; x<m_potentialGridSize; x++ )
// 	for( int y=0; y<m_potentialGridSize; y++ )
// 	{
// 	    cong[x][y] = cong[x][y] - cut;
// 	}

//     // Range: [minCong maxCong]
//     double minCong = 0.0;
//     double maxCong = ( maxOver - cut );

//     /*
//     double maxCong = DBL_MIN;
//     double minCong = DBL_MAX;
//     for( int x=0; x<m_potentialGridSize; x++ )
// 	for( int y=0; y<m_potentialGridSize; y++ )
// 	{
// 	    if( cong[x][y] > maxCong )
// 		maxCong = cong[x][y];
// 	    if( cong[x][y] < minCong )
// 		minCong = cong[x][y];
// 	}
//     */

//     // Smoothing
//     GaussianSmooth smooth;
//     int r = m_smoothR;
//     smooth.Gaussian2D( r, 6*r+1 );
//     smooth.Smooth( cong );

//     /*double*/ maxCong = DBL_MIN;
//     /*double*/ minCong = DBL_MAX;
//     double totalOver = 0;
//     double oldTotal = 0;
//     for( int layer = 0; layer < 2*m_pDB->m_totalLayer-1; layer++)
//     {
//     	for( int x=0; x<m_potentialGridSize; x++ )
// 	    for( int y=0; y<m_potentialGridSize; y++ )
// 	    {
// 	    	if( cong[x][y] > 0 )
// 		    totalOver += cong[x][y];
// 	    	else
// 		    cong[x][y] = 0;

// 	    	if( cong[x][y] > maxCong )
// 		    maxCong = cong[x][y];
// 	    	if( cong[x][y] < minCong )
// 		    minCong = cong[x][y];
// 	    	oldTotal += m_expBinPotentialOld[layer][x][y];
// 	    }
//     }
//     // scale > 0 
//     for( int x=0; x<m_potentialGridSize; x++ )
// 	for( int y=0; y<m_potentialGridSize; y++ )
// 	    cong[x][y] -= minCong;
//     totalOver -= m_potentialGridSize * m_potentialGridSize * minCong;
//     maxCong -= minCong;
//     minCong -= minCong;

//     double binArea = m_potentialGridWidth * m_potentialGridHeight;
//     double ratio1  = m_congWeight * m_totalFreeSpace / totalOver;
//     double ratio2  = binArea / maxCong;   // expect potential cannot be underflow
//     double ratio   = min( ratio1, ratio2 );

//     if( totalOver < 1e-10 )
//     {
// 	m_expBinPotential = m_expBinPotentialOld;
// 	return 0.0;
//     }

//     for(int layer = 0; layer < 2*m_pDB->m_totalLayer-1; layer++)
//     {
//     	for( int x=0; x<m_potentialGridSize; x++ )
// 	    for( int y=0; y<m_potentialGridSize; y++ )
// 	    {
// 	    	m_expBinPotential[layer][x][y] = m_expBinPotentialOld[layer][x][y] - ratio * cong[x][y];
// 	    	// TODO: consider preplaced blocks
// 	    } 
//     }
    
//     printf( "\n    TBinOver= %5.0f in %4d bins (%.0f%%) AllocWS %5.1f%% (min %.2f max %5.2f) TOverCut %5d  ", 
// 	    totalOver, 
// 	    overBinNumber, 
// 	    100.0*overBinNumber/congBinSize/congBinSize, 
// 	    ratio*totalOver*100.0/m_totalFreeSpace, 
// 	    minCong, maxCong,
// 	    (int)m_pDB->m_pCongMap->GetTotalOverflow() );
 
//     return totalOver;
// }

// 2007-04-02 (donnie) Extracted base potential smoothing
void MyNLP::SmoothBasePotential(const int& layer)
{
    // Gaussian smoothing 
    GaussianSmooth smooth;
    int r = m_smoothR;
    smooth.Gaussian2D( r, 6*r+1 );
    smooth.Smooth( m_basePotential[layer] );

    // Level smoothing 
    LevelSmoothBasePotential( double(m_smoothDelta), layer);

    // Increase the height of bins with density = 1.0
    if( m_smoothDelta != 1 )
	return;

#if 0
    if( param.bShow )
    {
	printf( "Generating base potential... " );
	fflush( stdout );
    }
#endif

    static vector< vector< double > > moreSmooth;
   
    // Save CPU time. Compute only at the first time.
    if( moreSmooth.size() != m_basePotential[layer].size() )
    {
	moreSmooth = m_basePotential[layer];
	r = m_smoothR * 6;		
	int kernel_size = 5*r;
	if( kernel_size % 2 == 0 )
	    kernel_size++;
	smooth.Gaussian2D( r, kernel_size );
	smooth.Smooth( moreSmooth );
    }

    // Merge basePotential and moreSmooth
    double binArea = m_potentialGridWidth * m_potentialGridHeight;
    double halfBinArea = binArea / 2;
    int changeCount = 0;
    double scale = 3;
    for( unsigned int i=0; i<moreSmooth.size(); i++ )
	for( unsigned int j=0; j<moreSmooth[i].size(); j++ )
	{
	    double free = binArea - m_basePotential[layer][i][j];
	    if( free < 1e-4 && moreSmooth[i][j] > halfBinArea ) // no space or high enough
	    {
		m_basePotential[layer][i][j] += (moreSmooth[i][j] - halfBinArea) * scale;
		changeCount++;
	    }	
	}

#if 0
    if( param.bShow )
	printf( "%d changed\n", changeCount );
#endif

}

// (kaie) 2009-09-10 extracted base potential smoothing 3d
void MyNLP::SmoothBasePotential3D()
{
    // Gaussian smoothing 
    GaussianSmooth smooth;
    int r = m_smoothR;
    smooth.Gaussian3D( r, 6*r+1 );
    smooth.Smooth( m_basePotential );
    
    // Level smoothing 
    LevelSmoothBasePotential3D( double(m_smoothDelta) );
    // Increase the height of bins with density = 1.0
    if( m_smoothDelta != 1 )
	return;

#if 0
    if( param.bShow )
    {
	printf( "Generating base potential... " );
	fflush( stdout );
    }
#endif

    static vector< vector< vector< double > > > moreSmooth;
   
    // Save CPU time. Compute only at the first time.
    if( moreSmooth.size() != m_basePotential.size() )
    {
	moreSmooth = m_basePotential;
	r = m_smoothR * 6;		
	int kernel_size = 5*r;
	if( kernel_size % 2 == 0 )
	    kernel_size++;
	smooth.Gaussian3D( r, kernel_size );
	smooth.Smooth( moreSmooth );
    }

    // Merge basePotential and moreSmooth
    double binVolumn = m_potentialGridWidth * m_potentialGridHeight * m_potentialGridThickness;
    double halfBinVolumn = binVolumn / 2;
    int changeCount = 0;
    double scale = 3;
    for( unsigned int k=0; k<moreSmooth.size(); k++ )
    	for( unsigned int i=0; i<moreSmooth[k].size(); i++ )
	    for( unsigned int j=0; j<moreSmooth[k][i].size(); j++ )
	    {
	    	double free = binVolumn - m_basePotential[k][i][j];
	    	if( free < 1e-4 && moreSmooth[k][i][j] > halfBinVolumn ) // no space or high enough
	    	{
		    m_basePotential[k][i][j] += (moreSmooth[k][i][j] - halfBinVolumn) * scale;
		    changeCount++;
	    	}	
	    }

#if 0
    if( param.bShow )
	printf( "%d changed\n", changeCount );
#endif

}
// @(kaie) 2009-09-10

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
    
//     if( m_pDB->m_pCongMap != NULL )
//     {
// 	sprintf( filename, "cong%s", name );
// 	m_pDB->m_pCongMap->OutputBinOverflowFigure( filename );
//     }

//     if( m_pGRMap != NULL )
//     {
// 	sprintf( filename, "gr%s", name );
// 	m_pGRMap->OutputGnuplotFigure( filename );
//     }

//     double binArea = m_potentialGridWidth * m_potentialGridHeight;
//     for(int layer = 0; layer < 2*m_pDB->m_totalLayer-1; layer++)
//     {
// 	sprintf( filename, "exp%s_%d", name, layer );
//     	CMatrixPlotter::OutputGnuplotFigure( m_expBinPotential[layer], filename, filename, 
// 	    binArea, true );
//     	sprintf( filename, "pot%s_%d", name, layer );
//     	CMatrixPlotter::OutputGnuplotFigure( m_gridPotential[layer], filename, filename,
// 	   binArea, true );
//     }
    
//     sprintf( filename, "util%s", name );
//     CPlaceBin placeBin( *m_pDB );
//     if( param.coreUtil < 1.0 )
// 	placeBin.CreateGrid( m_pDB->m_rowHeight * 10.0 ); // ispd06-contest bin size
//     else
// 	placeBin.CreateGrid( m_potentialGridSize );
//     placeBin.OutputBinUtil( filename, false );
// }


//@ Brian 2007-04-18 Calculate Net Weight In NLP
double MyNLP::NetWeightCalc(int netDegree)
{
    double netWeight = 1.0;
    switch (netDegree)
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


//Brian 2007-04-30

void MyNLP::CreatePotentialGridNet()
{

    int realGridSize = m_potentialGridSize;
    m_gridPotentialNet.clear(); 
       
    m_newPotentialNet.resize( m_pDB->m_nets.size() );
    m_gridPotentialNet.resize( realGridSize );
    
    m_basePotentialNet.resize( realGridSize );
    //m_binFreeSpaceNet.resize( realGridSize );
    m_expBinPotentialNet.resize( realGridSize );

    for( int i=0; i<realGridSize; i++ )
    {
	m_gridPotentialNet[i].resize( realGridSize, 0 );
	
        m_basePotentialNet[i].resize( realGridSize, 0 );
	//m_binFreeSpaceNet[i].resize( realGridSize, 0 );
	m_expBinPotentialNet[i].resize( realGridSize, 0 );
    }
    
    
}

void MyNLP::CreateDensityGridNet(int nGrid)
{
    m_gridDensityNet.resize( nGrid );
    m_gridDensitySpaceNet.resize( nGrid );
    for( int i=0; i<nGrid; i++ )
    {
        m_gridDensityNet[i].resize( nGrid, 0 );
	m_gridDensitySpaceNet[i].resize( nGrid, 0 );
    }
}

// void MyNLP::CatchCongParam()
// {
//     double totalPitch = 0.0;
//     double levelNum = 0;
//     for( unsigned int i = 2 ; i < m_pDB->m_pLEF->m_metalPitch.size() ; i++ )
//     {
//         levelNum++;
//         totalPitch += m_pDB->m_pLEF->m_metalPitch[i] * m_pDB->m_pLEF->m_lefUnit;
// 	}
//     m_dAvgPitch = totalPitch / levelNum;
//     m_dLevelNum = levelNum / 2;
//     m_dMacroOccupy = 10;
// }

void MyNLP::ComputeNetBoxInfo()
{
    double alwaysOverDensity = 0.0;
    double alwaysOverPotential = 0.0;
    double totalNetDensity = 0.0;
    for (unsigned int i = 0; i < m_pDB->m_nets.size(); i++)
    {
        if (m_pDB->m_nets[i].size() < 2)
            continue;
        int pinId = m_pDB->m_nets[i][0];
        double left = m_pDB->m_pins[pinId].absX;
        double right = left;
        double top = m_pDB->m_pins[pinId].absY;
        double bottom = top;
        
        for (unsigned int j = 1; j < m_pDB->m_nets[i].size(); j++)
        {
            pinId = m_pDB->m_nets[i][j];
            left = min( left, m_pDB->m_pins[pinId].absX);
            right = max( right, m_pDB->m_pins[pinId].absX);
            bottom = min( bottom, m_pDB->m_pins[pinId].absY);
            top = max( top, m_pDB->m_pins[pinId].absY);
        }
        m_NetBoxInfo[i].m_dWidth = right - left;
        m_NetBoxInfo[i].m_dHeight = top - bottom;
        m_NetBoxInfo[i].m_dArea = m_NetBoxInfo[i].m_dWidth * m_NetBoxInfo[i].m_dHeight;
        m_NetBoxInfo[i].m_dCenterX = left + m_NetBoxInfo[i].m_dWidth / 2;
        m_NetBoxInfo[i].m_dCenterY = bottom + m_NetBoxInfo[i].m_dHeight / 2;
        if (m_NetBoxInfo[i].m_dArea > 0)
        {
            m_NetBoxInfo[i].m_dDensity = (m_dAvgPitch / m_dLevelNum) * 
                (m_NetBoxInfo[i].m_dWidth + m_NetBoxInfo[i].m_dHeight) / m_NetBoxInfo[i].m_dArea;
        }
        else
        {
            m_NetBoxInfo[i].m_dDensity = 1.0 / m_dLevelNum;
        }

        totalNetDensity += m_NetBoxInfo[i].m_dDensity * m_NetBoxInfo[i].m_dArea;

        /*if (m_dCongUtil > 0.0 && m_dCongUtil < 1.0)
        {
            if( m_NetBoxInfo[i].m_dWidth >= 2 * m_potentialGridWidth && m_NetBoxInfo[i].m_dHeight >= 2 * m_potentialGridHeight )
            {
		        alwaysOverPotential += 
		            (m_NetBoxInfo[i].m_dWidth - m_potentialGridWidth ) * 
		            (m_NetBoxInfo[i].m_dHeight - m_potentialGridHeight ) * 
		            (1.0 - m_dCongUtil );
            }

            if( m_NetBoxInfo[i].m_dWidth >= 2 * m_gridDensityWidth && m_NetBoxInfo[i].m_dHeight >= 2 * m_gridDensityHeight )
            {
		        alwaysOverDensity += 
		            (m_NetBoxInfo[i].m_dWidth - m_gridDensityWidth ) * 
		            (m_NetBoxInfo[i].m_dHeight - m_gridDensityHeight ) * 
		            (1.0 - m_dCongUtil );
            }
        }*/
    }

    m_alwaysOverPotentialNet = alwaysOverPotential;
    m_alwaysOverDensityNet = alwaysOverDensity;
    m_dTotalNetDensity = totalNetDensity;
    //Brian 2007-06-18
    if (param.bCongStopDynamic)
    {
        double totalChip = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) * 
                           ( m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom );
        m_dCongUtil = totalNetDensity / totalChip + param.dCongTargetUtil;
    	//if (m_dCongUtil > 1.0)
    	    //m_dCongUtil = 1.0;
    }
    else
    {
        m_dCongUtil = param.dCongTargetUtil;
    	//if (m_dCongUtil > 1.0)
    	     //m_dCongUtil = 1.0;
    }
    //@Brian 2007-06-18

}


void MyNLP::ComputeNewPotentialGridNet()
{

    for( unsigned int i=0; i < m_pDB->m_nets.size(); i++ )
    {
	if (m_pDB->m_nets[i].size() < 2)
	    continue;

	double cellX = m_NetBoxInfo[i].m_dCenterX;
	double cellY = m_NetBoxInfo[i].m_dCenterY;
	double potentialRX = _potentialRX;
	double potentialRY = _potentialRY;
	double width  = m_NetBoxInfo[i].m_dWidth;
	double height = m_NetBoxInfo[i].m_dHeight;

	if( height < m_potentialGridHeight && width < m_potentialGridWidth )
	    width = height = 0;

	double left   = cellX - width * 0.5  - potentialRX;
	double bottom = cellY - height * 0.5 - potentialRY;
	double right  = cellX + (cellX - left);
	double top    = cellY + (cellY - bottom);
	if( left   <  m_pDB->m_coreRgn.left  -  m_potentialGridPadding *  m_potentialGridWidth )     
	    left   =  m_pDB->m_coreRgn.left  -  m_potentialGridPadding *  m_potentialGridWidth;
	if( right  >  m_pDB->m_coreRgn.right +  m_potentialGridPadding *  m_potentialGridWidth )    
	    right  =  m_pDB->m_coreRgn.right +  m_potentialGridPadding *  m_potentialGridWidth;
	if( bottom <  m_pDB->m_coreRgn.bottom -  m_potentialGridPadding *  m_potentialGridHeight )   
	    bottom =  m_pDB->m_coreRgn.bottom -  m_potentialGridPadding *  m_potentialGridHeight;
	if( top    >  m_pDB->m_coreRgn.top    +  m_potentialGridPadding *  m_potentialGridHeight )      
	    top    =  m_pDB->m_coreRgn.top    +  m_potentialGridPadding *  m_potentialGridHeight;

	int gx, gy;
	GetClosestGrid( left, bottom, gx, gy );
	int gxx, gyy;
	double xx, yy;

	double totalPotential = 0;
	vector< potentialStruct >& potentialList =  m_newPotentialNet[i];     
	potentialList.clear();	

	for( gxx = gx, xx =  GetXGrid(gx); xx <= right && gxx<(int) m_gridPotentialNet.size(); 
		gxx++, xx+= m_potentialGridWidth )
	{
	    for( gyy = gy, yy =  GetYGrid(gy); yy <= top && gyy<(int) m_gridPotentialNet.size(); 
		    gyy++, yy+= m_potentialGridHeight )
	    {
		double potential = GetPotential( cellX, xx, potentialRX, width ) *
		    GetPotential( cellY, yy, potentialRY, height );
		if( potential > 0 )
		{
		    totalPotential += potential;
		    potentialList.push_back( potentialStruct( gxx, gyy, potential ) );
		}
	    }
	}

	// normalize the potential so that total potential equals the net total density
	double scale =  m_NetBoxInfo[i].m_dArea * m_NetBoxInfo[i].m_dDensity / totalPotential;

	m_NetBoxInfo[i].m_dNorm = scale;	    // normalization factor for the net i
    } // for each net
}

void MyNLP::ClearPotentialGridNet()
{
    for( int gx=0; gx<(int)m_gridPotentialNet.size(); gx++ )
	fill( m_gridPotentialNet[gx].begin(), m_gridPotentialNet[gx].end(), 0.0 );
}

void MyNLP::UpdatePotentialGridNet()
{
    ClearPotentialGridNet();
    for( unsigned int i=0; i<m_newPotentialNet.size(); i++ )	// for each cell
	for( unsigned int j=0; j<m_newPotentialNet[i].size(); j++ ) // for each bin
	{
	    int gx = m_newPotentialNet[i][j].gx;
	    int gy = m_newPotentialNet[i][j].gy;
        m_gridPotentialNet[ gx ][ gy ] += m_newPotentialNet[i][j].potential * m_NetBoxInfo[i].m_dNorm;
	}
}

void MyNLP::ClearDensityGridNet()
{
    for( unsigned int i=0; i<m_gridDensityNet.size(); i++ )
	for( unsigned int j=0; j<m_gridDensityNet[i].size(); j++ )
	    m_gridDensityNet[i][j] = 0.0;
}

void MyNLP::UpdateDensityGridNet()
{
    ClearDensityGridNet();
    
    // for each net b, update the corresponding bin area
    for( unsigned int b=0; b<m_pDB->m_nets.size(); b++ )
    {
	    if(  m_pDB->m_nets.size() < 2 )
	        continue;

        double w  = m_NetBoxInfo[b].m_dWidth;
        double h  = m_NetBoxInfo[b].m_dHeight;

	    // rectangle range 
        double left   = m_NetBoxInfo[b].m_dCenterX - w * 0.5;
        double bottom = m_NetBoxInfo[b].m_dCenterY - h * 0.5;
	    double right  = left   + w;
	    double top    = bottom + h;

	    // find nearest gird
	    int gx = static_cast<int>( floor( (left - m_pDB->m_coreRgn.left) / m_gridDensityWidth ) );
	    int gy = static_cast<int>( floor( (bottom - m_pDB->m_coreRgn.bottom) / m_gridDensityHeight ) );
	    if( gx < 0 ) gx = 0;
	    if( gy < 0 ) gy = 0;

	    // Block is always inside the core region. Do not have to check boundary.
	    for( unsigned int xOff = gx; xOff < m_gridDensityNet.size(); xOff++ )
	    {
	        double binLeft  = m_pDB->m_coreRgn.left + m_gridDensityWidth * xOff;
	        double binRight = binLeft + m_gridDensityWidth;
	        if( binLeft >= right )
		        break;
    	    
	        for( unsigned int yOff = gy; yOff < m_gridDensityNet[xOff].size(); yOff++ )
	        {
		        double binBottom = m_pDB->m_coreRgn.bottom + m_gridDensityHeight * yOff;
		        double binTop    = binBottom + m_gridDensityHeight;
		        if( binBottom >= top )
		            break;

		        double area = 
		            getOverlap( left, right, binLeft, binRight ) *
		            getOverlap( bottom, top, binBottom, binTop );
                m_gridDensityNet[xOff][yOff] += area * m_NetBoxInfo[b].m_dDensity;
	        }
	    }
    } // each net
}

void MyNLP::GetDensityPaneltyNet()
{
    double den = 0;
    for( unsigned int i=0; i<m_gridPotentialNet.size(); i++ )
    {
        //double expPotential = m_potentialGridWidth * m_potentialGridHeight * m_dCongUtil;
	    for( unsigned int j=0; j<m_gridPotentialNet[i].size(); j++ )
	    {
		den += 
		    ( m_gridPotentialNet[i][j] - m_expBinPotentialNet[i][j] ) *
		    ( m_gridPotentialNet[i][j] - m_expBinPotentialNet[i][j] );

	    }
    }
    m_dDensityNet = den;
}


void MyNLP::UpdateGradCong()
{
    double gradDensityX;
    double gradDensityY;

    for ( unsigned int i = 0; i < grad_congestion.size(); i++)
        grad_congestion[i] = 0;
    
    for( unsigned int i = 0; i < m_pDB->m_nets.size(); i++ )	    // for each net
    {
	    if( m_pDB->m_nets[i].size() < 2 )
	        continue;

	    GetCongGrad( i, gradDensityX, gradDensityY);	    // bell-shaped potential

        for (unsigned int j = 0; j < m_pDB->m_nets[i].size(); j++)
        {
            int pinId = m_pDB->m_nets[i][j];
            int moduleId = m_pDB->m_pins[pinId].moduleId;
            if ( moduleId >= (int)m_pDB->m_modules.size() )
                continue;
            if( m_pDB->m_modules[moduleId].m_isFixed || m_pDB->m_modules[moduleId].m_netsId.size() == 0 )
	            continue;
            double smoothX = 1.0;
            double smoothY = 1.0;
            if (param.bCongSmooth)
            {
                CongSmooth(moduleId, i, pinId, smoothX, smoothY);
                assert(smoothX >= 0 && smoothX < 1.0001);
                assert(smoothY >= 0 && smoothY < 1.0001);
            }
	        grad_congestion[2 * moduleId]     += gradDensityX * smoothX;
	        grad_congestion[2 * moduleId + 1] += gradDensityY * smoothY;
        }
    } // for each net
}

void MyNLP::CongSmooth(int moduleId, int netId,  int pinId, double& smoothX, double& smoothY)
{
    if (m_usePin[moduleId])
    {
        smoothX = _expPins[ 2 * pinId ] / m_nets_sum_exp_xi_over_alpha[netId] -
        	      1.0 / _expPins[ 2 * pinId ] / m_nets_sum_exp_inv_xi_over_alpha[netId];
        smoothY = _expPins[ 2*pinId+1 ] / m_nets_sum_exp_yi_over_alpha[netId] -
        		  1.0 / _expPins[ 2*pinId+1 ] / m_nets_sum_exp_inv_yi_over_alpha[netId];
    }
    else
    {
        smoothX = _expX[2 * moduleId] / m_nets_sum_exp_xi_over_alpha[netId] -
            	  1.0 / _expX[2 * moduleId] / m_nets_sum_exp_inv_xi_over_alpha[netId];
        smoothY = _expX[2*moduleId+1] / m_nets_sum_exp_yi_over_alpha[netId] -
            	  1.0 / _expX[2*moduleId+1] / m_nets_sum_exp_inv_yi_over_alpha[netId];
    }
    smoothX = fabs(smoothX);
    smoothY = fabs(smoothY);
}

void MyNLP::GetCongGrad(const int& i, double& gradX, double& gradY)
{
    double cellX = m_NetBoxInfo[i].m_dCenterX;
    double cellY = m_NetBoxInfo[i].m_dCenterY;

    double width  = m_NetBoxInfo[i].m_dWidth;
    double height = m_NetBoxInfo[i].m_dHeight;
    //// use square to model small std-cells
    if( height < m_potentialGridHeight && width < m_potentialGridWidth )
	    width = height = 0;
    
    double left   = cellX - width  * 0.5 - _potentialRX;
    double bottom = cellY - height * 0.5 - _potentialRY;
    double right  = cellX + ( cellX - left );
    double top    = cellY + ( cellY - bottom );
    if( left   < m_pDB->m_coreRgn.left  - m_potentialGridPadding * m_potentialGridWidth )     
	    left   = m_pDB->m_coreRgn.left  - m_potentialGridPadding * m_potentialGridWidth;
    if( right  > m_pDB->m_coreRgn.right + m_potentialGridPadding * m_potentialGridWidth )    
	    right  = m_pDB->m_coreRgn.right + m_potentialGridPadding * m_potentialGridWidth;
    if( bottom < m_pDB->m_coreRgn.bottom - m_potentialGridPadding * m_potentialGridHeight )   
	    bottom = m_pDB->m_coreRgn.bottom - m_potentialGridPadding * m_potentialGridHeight;
    if( top    > m_pDB->m_coreRgn.top    + m_potentialGridPadding * m_potentialGridHeight )      
	    top    = m_pDB->m_coreRgn.top    + m_potentialGridPadding * m_potentialGridHeight;
    
    int gx, gy;
    GetClosestGrid( left, bottom, gx, gy );
    
    int gxx, gyy;
    double xx, yy;
    gradX = 0.0;	
    gradY = 0.0;
    //double expBinPotential = m_potentialGridWidth * m_potentialGridHeight * m_dCongUtil;
    for( gxx = gx, xx = GetXGrid( gx ); xx <= right && gxx<(int)m_gridPotentialNet.size(); 
	    gxx++, xx += m_potentialGridWidth )
    {

	    for( gyy = gy, yy = GetYGrid( gy ); yy <= top && gyy<(int)m_gridPotentialNet.size(); 
		    gyy++, yy += m_potentialGridHeight )
	    {

	        double gX =
		    GetGradPotential( cellX, xx, _potentialRX, width ) *
		    GetPotential(     cellY, yy, _potentialRY, height ) *
		    ( m_gridPotentialNet[gxx][gyy] - m_expBinPotentialNet[gxx][gyy] * m_dCongUtil) *
            m_NetBoxInfo[i].m_dNorm;
	        double gY =
		    GetPotential(     cellX, xx, _potentialRX, width  ) *
		    GetGradPotential( cellY, yy, _potentialRY, height ) *
		    ( m_gridPotentialNet[gxx][gyy] - m_expBinPotentialNet[gxx][gyy] * m_dCongUtil) *
		    m_NetBoxInfo[i].m_dNorm;
	        
	        gradX += gX;
	        gradY += gY;
	    }
    } // for each grid

}


bool MyNLP::AdjustForceNet( vector<double>& p_grad_wl, vector<double>& p_grad_potential, vector<double>& p_grad_congestion )
{
    double totalGrad = 0;
    int size = m_pDB->m_modules.size();
    for( int i = 0; i < size; i++ )
    {
	    double value = 
            (p_grad_wl[2*i] + p_grad_potential[2*i] + p_grad_congestion[2*i]) * 
            (p_grad_wl[2*i] + p_grad_potential[2*i] + p_grad_congestion[2*i]) + 
	        (p_grad_wl[2*i+1] + p_grad_potential[2*i+1] + p_grad_congestion[2*i+1]) * 
            (p_grad_wl[2*i+1] + p_grad_potential[2*i+1] + p_grad_congestion[2*i+1]); 
	    totalGrad += value;
    }
    
    if( isNaN( totalGrad ) )
	    return false;
    assert( !isNaN( totalGrad ) );	// it is checked in GoSolve()
    
    double avgGrad = sqrt( totalGrad / size );
 
    // Do truncation
    double expMaxGrad = avgGrad * param.truncationFactor;	// x + y
    double expMaxGradSquare = expMaxGrad * expMaxGrad;
    for( int i = 0; i < size; i++ )
    {
	    double valueSquare = 
            (p_grad_wl[2*i] + p_grad_potential[2*i] + p_grad_congestion[2*i]) * 
            (p_grad_wl[2*i] + p_grad_potential[2*i] + p_grad_congestion[2*i]) + 
	        (p_grad_wl[2*i+1] + p_grad_potential[2*i+1] + p_grad_congestion[2*i+1]) * 
            (p_grad_wl[2*i+1] + p_grad_potential[2*i+1] + p_grad_congestion[2*i+1]);
	    if( valueSquare == 0 )
	    {
	        // avoid value = 0 let to inf
	        p_grad_wl[2*i] = p_grad_wl[2*i+1] = 0;
	        p_grad_potential[2*i] = p_grad_potential[2*i+1] = 0;
            p_grad_congestion[2*i] = p_grad_congestion[2*i+1] = 0;
	    }
	    else
	    {
	        if( valueSquare > expMaxGradSquare )
	        {
		        double value = sqrt( valueSquare );
		        p_grad_wl[2*i]   = p_grad_wl[2*i]   * expMaxGrad / value;
		        p_grad_wl[2*i+1] = p_grad_wl[2*i+1] * expMaxGrad / value;
		        p_grad_potential[2*i]   = p_grad_potential[2*i]   * expMaxGrad / value;
		        p_grad_potential[2*i+1] = p_grad_potential[2*i+1] * expMaxGrad / value;
                p_grad_congestion[2*i]   = p_grad_congestion[2*i]   * expMaxGrad / value;
		        p_grad_congestion[2*i+1] = p_grad_congestion[2*i+1] * expMaxGrad / value;
	        }
	    }
	
    }
    return true;
}

double MyNLP::GetTotalOverDensityNet()
{
    double over = 0;
    for( unsigned int i=0; i<m_gridDensityNet.size(); i++ )
	for( unsigned int j=0; j<m_gridDensityNet[i].size(); j++ )
	{
	    double targetSpace = m_gridDensitySpaceNet[i][j] * m_dCongUtil;
	    //double targetSpace = m_gridDensityWidth * m_gridDensityHeight * m_dCongUtil;
	    if( m_gridDensityNet[i][j]  > targetSpace  )
    		over += m_gridDensityNet[i][j] - targetSpace;
	}

    return ( over - m_alwaysOverDensityNet) / m_dTotalNetDensity + 1.0; 
}

double MyNLP::GetTotalOverPotentialNet()
{
    double over = 0;
    for( unsigned int i=0; i<m_gridPotentialNet.size(); i++ )
	for( unsigned int j=0; j<m_gridPotentialNet[i].size(); j++ )
	{
	    //double targetSpace = m_potentialGridWidth * m_potentialGridHeight * m_dCongUtil;
	    if( m_gridPotentialNet[i][j]  > m_expBinPotentialNet[i][j] * m_dCongUtil )
    		over += m_gridPotentialNet[i][j] - m_expBinPotentialNet[i][j] * m_dCongUtil;
	}

    // TODO: remove "1.0"
    return (over - m_alwaysOverPotentialNet) / m_dTotalNetDensity + 1.0; 
}

double MyNLP::GetMaxDensityNet()
{
    double maxUtilization = 0;
    double binArea = m_gridDensityWidth * m_gridDensityHeight;
    for( int i=0; i<(int)m_gridDensityNet.size(); i++ )
	for( int j=0; j<(int)m_gridDensityNet[i].size(); j++ )
	{
	    if( m_gridDensitySpaceNet[i][j] > 1e-5 )
	    {
		double preplacedarea = binArea - m_gridDensitySpaceNet[i][j];
		double utilization = (m_gridDensityNet[i][j] + preplacedarea)  / binArea;   
		//double utilization = m_gridDensityNet[i][j]  / binArea;
		if( utilization > maxUtilization )
		    maxUtilization = utilization;
	    }
	}
    return maxUtilization;
}

//@Brian 2007-04-30

//Brian 2007-07-23

void MyNLP::UpdateDensityGridSpaceNet()
{
    
    double allSpace = m_gridDensityWidth * m_gridDensityHeight;
    for( unsigned int i=0; i<m_gridDensitySpaceNet.size(); i++ )
	for( unsigned int j=0; j<m_gridDensitySpaceNet[i].size(); j++ )
	    m_gridDensitySpaceNet[i][j] = allSpace;
   
    
    // for each cell b, update the corresponding bin area
    for( int b=0; b<(int)m_pDB->m_modules.size(); b++ )
    {
	if( false == m_pDB->m_modules[b].m_isFixed )
	    continue;

	double w  = m_pDB->m_modules[b].GetWidth(z[b]-0.5);
	double h  = m_pDB->m_modules[b].GetHeight(z[b]-0.5);
	double left   = x[b*2]   - w * 0.5;
	double bottom = x[b*2+1] - h * 0.5;
	double right  = left   + w;
	double top    = bottom + h;

	if( w == 0 || h == 0 )
	    continue;

	if (h <= m_dMacroOccupy * m_pDB->m_rowHeight)
	    continue;

	double occupyRatio = ((double)(((int)(h / m_pDB->m_rowHeight / m_dMacroOccupy)) - 1)) / 2.0 / m_dLevelNum;
	if (occupyRatio > 1)
	    occupyRatio = 1;
	if (occupyRatio < 0)
	    occupyRatio = 0;	
	
	// find nearest bottom-left gird
	int gx = static_cast<int>( floor( (left   - m_pDB->m_coreRgn.left)   / m_gridDensityWidth ) );
	int gy = static_cast<int>( floor( (bottom - m_pDB->m_coreRgn.bottom) / m_gridDensityHeight ) );
	if( gx < 0 )  gx = 0;
	if( gy < 0 )  gy = 0;
	
	for( int xOff = gx; xOff < (int)m_gridDensitySpaceNet.size(); xOff++ )
	{
	    double binLeft  = m_pDB->m_coreRgn.left + xOff * m_gridDensityWidth;
	    double binRight = binLeft + m_gridDensityWidth;
	    if( binLeft >= right )
		break;
	    
	    for( int yOff = gy; yOff < (int)m_gridDensitySpaceNet[xOff].size(); yOff ++ )
	    {
		double binBottom = m_pDB->m_coreRgn.bottom + yOff * m_gridDensityHeight;
		double binTop    = binBottom + m_gridDensityHeight;
		if( binBottom >= top )
		    break;

		m_gridDensitySpaceNet[xOff][yOff] -= 
		    getOverlap( left, right, binLeft, binRight ) * 
		    getOverlap( bottom, top, binBottom, binTop ) * occupyRatio;
	    }
	}

    } // each module

    for( unsigned int i=0; i<m_gridDensitySpaceNet.size(); i++ )
	for( unsigned int j=0; j<m_gridDensitySpaceNet[i].size(); j++ )
	{
	    if( m_gridDensitySpaceNet[i][j] < 1e-5 )
	    {
		m_gridDensitySpaceNet[i][j] = 0.0;
	    }
	}
}

void MyNLP::SmoothBasePotentialNet()
{

    // Gaussian smoothing 
    GaussianSmooth smooth;
    int r = m_smoothR;
    smooth.Gaussian2D( r, 6*r+1 );
    smooth.Smooth( m_basePotentialNet );

    static vector< vector< double > > moreSmooth;
   
    // Save CPU time. Compute only at the first time.
    if( moreSmooth.size() != m_basePotentialNet.size() )
    {
	moreSmooth = m_basePotentialNet;
	r = m_smoothR * 6;		
	int kernel_size = 5*r;
	if( kernel_size % 2 == 0 )
	    kernel_size++;
	smooth.Gaussian2D( r, kernel_size );
	smooth.Smooth( moreSmooth );
    }

    // Merge basePotential and moreSmooth
    double binArea = m_potentialGridWidth * m_potentialGridHeight;
    double halfBinArea = binArea / 2;
    double scale = 3;
    for( unsigned int i=0; i<moreSmooth.size(); i++ )
	for( unsigned int j=0; j<moreSmooth[i].size(); j++ )
	{
	    double free = binArea - m_basePotentialNet[i][j];
	    if( free < 1e-4 && moreSmooth[i][j] > halfBinArea ) // no space or high enough
	    {
		m_basePotentialNet[i][j] += (moreSmooth[i][j] - halfBinArea) * scale;
	    }	
	}

}

void MyNLP::UpdateExpBinPotentialNet()
{
    //double totalFree = 0;
    //int zeroSpaceBin = 0;
    m_expBinPotentialNet.resize( m_basePotentialNet.size() );
    for( unsigned int i=0; i<m_basePotentialNet.size(); i++ )
    {
	m_expBinPotentialNet[i].resize( m_basePotentialNet[i].size() );
	for( unsigned int j=0; j<m_basePotentialNet[i].size(); j++ )
	{
	    double base = m_basePotentialNet[i][j];
	    double overlapX = getOverlap( 
		    GetXGrid(i)-m_potentialGridWidth*0.5, GetXGrid(i)+m_potentialGridWidth*0.5, 
		    m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.right );
	    double overlapY = getOverlap( 
		    GetYGrid(i)-m_potentialGridWidth*0.5, GetYGrid(i)+m_potentialGridHeight*0.5, 
		    m_pDB->m_coreRgn.bottom, m_pDB->m_coreRgn.top );
	    double realBinArea = overlapX * overlapY;
	    double free = realBinArea - base;
	    
	    if( free > 1e-4 )
	    {
		m_expBinPotentialNet[i][j] = free;
	    }
	    else
	    {
		m_expBinPotentialNet[i][j] = 0.0;
	    }
	} 
    }
}


void MyNLP::UpdatePotentialGridBaseNet()
{
    //double binArea = m_potentialGridWidth * m_potentialGridHeight;
    //m_binFreeSpaceNet.resize( m_basePotentialNet.size() );
    for( unsigned int i=0; i<m_basePotentialNet.size(); i++ )
    {
	fill( m_basePotentialNet[i].begin(), m_basePotentialNet[i].end(), 0.0 );
	//m_binFreeSpaceNet[i].resize( m_basePotentialNet[i].size() );
	//fill( m_binFreeSpaceNet[i].begin(), m_binFreeSpaceNet[i].end(), binArea );
    }

    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
    {
	// for each cell. cell ci coordinate is ( x[i*2], x[i*2+1] )

	if( m_pDB->m_modules[i].m_isFixed == false )
	    continue;

	if( m_pDB->BlockOutCore( i ) )
	    continue;	// pads?

	double cellX = x[i*2];
	double cellY = x[i*2+1];
	double width  = m_pDB->m_modules[i].GetWidth(z[i]-0.5);
	double height = m_pDB->m_modules[i].GetHeight(z[i]-0.5);

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

	if( width == 0 || height == 0 )
	    continue;

	if (height <= m_dMacroOccupy * m_pDB->m_rowHeight)
	    continue;

	double occupyRatio = ((double)(((int)(height / m_pDB->m_rowHeight / m_dMacroOccupy)) - 1)) / 2.0 / m_dLevelNum;
	if (occupyRatio > 1)
	    occupyRatio = 1;
	if (occupyRatio < 0)
	    occupyRatio = 0;	
	
	int gx, gy;
	GetClosestGrid( left, bottom, gx, gy );
	int gxx, gyy;
	double xx, yy;

	// Exact density for the base potential"
	for( gxx = gx, xx = GetXGrid(gx); xx <= right && gxx < (int)m_basePotentialNet.size(); 
		gxx++, xx+=m_potentialGridWidth )
	{
	    for( gyy = gy, yy = GetYGrid(gy); yy <= top && gyy < (int)m_basePotentialNet[gxx].size(); 
		    gyy++, yy+=m_potentialGridHeight )
	    {
		m_basePotentialNet[gxx][gyy] +=
		    getOverlap( left, right, xx, xx+m_potentialGridWidth ) * 
		    getOverlap( bottom, top, yy, yy+m_potentialGridHeight ) * occupyRatio;

		//m_binFreeSpaceNet[gxx][gyy] -= 
		    //getOverlap( left, right, xx, xx+m_potentialGridWidth ) * 
		    //getOverlap( bottom, top, yy, yy+m_potentialGridHeight ) * occupyRatio;
	    }
	}

    } // for each cell

}

//@Brian 2007-07-23

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
    for( unsigned int i=0; i<binWireForce.size(); i++ )
    {
	binWireForce[i].resize( m_gridPotential.size()/mergeCount, 0 );
	binSpreadingForce[i].resize( m_gridPotential.size()/mergeCount, 0 );
	ratio[i].resize( m_gridPotential.size()/mergeCount, 0 );
    }
    

    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	if( m_pDB->m_modules[i].m_isFixed )
	    continue;

	double cellX = x[2*i];
	double cellY = x[2*i+1];
	double cellZ = z[i];
	int gx, gy, gz;
	GetClosestGrid( cellX, cellY, cellZ, gx, gy, gz);

	gx /= mergeCount;
	gy /= mergeCount;
	gz /= mergeCount;
	
	if( gx >= (int)binWireForce.size() )
	    gx = (int)binWireForce.size() - 1;
	if( gy >= (int)binWireForce.size() )
	    gy = (int)binWireForce.size() - 1;
	if( gz >= (int)binWireForce.size() )
	    gz = (int)binWireForce.size() - 1;
	
	if( gx >= (int)binWireForce.size() || gy >= (int)binWireForce.size() || gz >= (int)binWireForce.size() )
	{
	    printf( "(%d %d %d)   (%d)\n", gx, gy, gz, binWireForce.size() );
	}
	assert( gx >= 0 );
	assert( gy >= 0 );
	assert( gz >= 0 );
	assert( gx < (int)binWireForce.size() );
	assert( gy < (int)binWireForce.size() );
	assert( gz < (int)binWireForce.size() );
	binWireForce[gx][gy] += fabs( grad_wire[ 2*i ] ) + fabs( grad_wire[ 2*i+1 ] );	
	binSpreadingForce[gx][gy] += fabs( grad_potential[ 2*i ] ) + fabs( grad_potential[ 2*i+1 ] );	
    }

    for( unsigned int i=0; i<ratio.size(); i++ )
	for( unsigned int j=0; j<ratio[i].size(); j++ )
	{
	    ratio[i][j] = binSpreadingForce[i][j] / binWireForce[i][j];
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
    int n;
    if(m_bMoveZ)
	n = 3 * m_pDB->m_modules.size();
    else
	n = 2 * m_pDB->m_modules.size();
    
    // 2006-02-22 find weights
    
    _weightWire = _weightDensity = 1.0;     // init values to call eval_grad_f

    ComputeBinGrad();
    Parallel( eval_grad_f_thread, m_pDB->m_modules.size() );
    double totalWireGradient = 0;
    double totalPotentialGradient = 0;
    double totalCongGradient = 0.0;
    double totalViaGradient = 0;
    double totalPotentialZGradient = 0;

    //Brian 2007-04-30
    if (param.bCongObj)
    {
	if( !AdjustForceNet( grad_wire, grad_potential, grad_congestion ) ) // truncation
		{cout << "\n !AdjustForceNet()\n";
	    return false;	// bad values in grad_wire or grad_potential
		}
    }
    else
    {
	if( !AdjustForce( n, x, grad_wire, grad_potential, grad_potentialZ) ) // truncation
		{cout << "\n !AdjustForce()\n";
	    return false;	// bad values in grad_wire or grad_potential
		}
    }
    //@Brian 2007-04-30
    int size = n/2;
    if(m_bMoveZ)
		size = n/3;
    else
		size = n/2;
    for(int i = 0; i < size; i++)
    {
	    // x direction
	    if(isNaN(grad_wire[2*i])) {cout<<"\n In x direction: grad_wire["<<2*i<<"] isNaN.\n"; return false;}
	    if(fabs(grad_wire[2*i]) > DBL_MAX * 0.95) {cout<<"\n In x direction: grad_wire["<<2*i<<"] inf.\n"; return false;}
	    assert(fabs(grad_wire[2*i]) < DBL_MAX * 0.95);
	    assert(!isNaN(grad_wire[2*i]));
	    totalWireGradient += fabs(grad_wire[2*i]);
	    totalPotentialGradient += fabs(grad_potential[2*i]);

	    if(param.bCongObj)
		    totalCongGradient += fabs(grad_congestion[2*i]);

	    // y direction
	    if(isNaN(grad_wire[2*i+1])) {cout<<"\n In y direction: grad_wire["<<2*i+1<<"] isNaN.\n"; return false;}
	    if(fabs(grad_wire[2*i+1]) > DBL_MAX * 0.95) {cout<<"\n In y direction: grad_wire["<<2*i+1<<"] inf.\n"; return false;}
	    assert(fabs(grad_wire[2*i+1]) < DBL_MAX * 0.95);
	    assert(!isNaN(grad_wire[2*i+1]));
	    totalWireGradient += fabs(grad_wire[2*i+1]);
	    totalPotentialGradient += fabs(grad_potential[2*i+1]);

	    if(param.bCongObj)
		    totalCongGradient += fabs(grad_congestion[2*i+1]);

	    // z direction
	    if(m_bMoveZ)
	    {
	    	if(isNaN(grad_via[i])){cout<<"\n In m_bMoveZ: grad_via["<<i<<"] isNaN.\n"; return false;}
	    	if(fabs(grad_via[i]) > DBL_MAX * 0.95) {cout<<"\n In m_bMoveZ: grad_via["<<i<<"] inf.\n"; return false;}
	    	assert(fabs(grad_via[i]) < DBL_MAX * 0.95);
	    	assert(!isNaN(grad_via[i]));
	    	//totalWireGradient += fabs(grad_via[i]);
	    	//totalPotentialGradient += fabs(grad_potentialZ[i]);
	    	totalViaGradient += fabs(grad_via[i]);
	    	totalPotentialZGradient += fabs(grad_potentialZ[i]);
	    }
    }
    
    // comment by kaie 2009-09-14
    /*for( int i=0; i<n; i++ )
    {
   
    	if( isNaN( grad_wire[i] ) )
    	    return false;
	if( fabs( grad_wire[i] ) > DBL_MAX * 0.95 ) 
    	    return false;
    	
    	assert( fabs( grad_wire[i] ) < DBL_MAX * 0.95 );
    	assert( !isNaN( grad_wire[i] ) );
    	
    	totalWireGradient      += fabs( grad_wire[i] );
    	totalPotentialGradient += fabs( grad_potential[i] );
	
	//Brian 2007-04-30
        if (param.bCongObj)
            totalCongGradient += fabs(grad_congestion[i]);
        //@Brian 2007-04-30
    }*/
    if( fabs( totalWireGradient ) > DBL_MAX * 0.95 ) {
		cout<<"\n totalWireGradient inf.\n";
	return false;
	}
    if( fabs( totalPotentialGradient ) > DBL_MAX * 0.95 ) {
		cout<<"\n totalPotentialGradient inf.\n";
	return false;
	}
    //Brian 2007-04-30
    if( fabs( totalCongGradient ) > DBL_MAX * 0.95 ) {
		cout<<"\n totalCongGradient inf.\n";
	return false;
	}
    //@Brian 2007-04-30
    //(kaie) 2009-10-19
    if(m_bMoveZ)
    {
    	if( fabs( totalViaGradient) > DBL_MAX * 0.95 ){
			cout<<"\n totalViaGradient inf.\n";
	    return false;
		}
    	if( fabs( totalPotentialZGradient) > DBL_MAX * 0.95 ){
			cout<<"\n totalPotentialZGradient inf.\n";
	    return false;
		}
    }
    //@(kaie) 2009-10-19
    // Fix density weight, change wire weight 
    //Brian 2007-04-23
    if (param.bCongObj)
    {
        _weightDensity = 1.0;
        m_dWeightCong = param.dCongWeight * totalPotentialGradient  / totalCongGradient;
        _weightWire = wWire * totalPotentialGradient  / totalWireGradient;
        if( param.bShow )
        {
            printf( "\n WireForce: %f, PotentialForce: %f, CongestionForce: %f ", 
		    totalWireGradient, totalPotentialGradient, totalCongGradient);
            printf( "\n Congestion Weight Parameter = %f ", param.dCongWeight );
            printf( "\n WireWeight: %f, PotentialWeight: %f, CongestionWeight: %f \n ", 
		    _weightWire, _weightDensity, m_dWeightCong);
        }
        assert( wWire == 0 || _weightWire > 0 );
    }
    else
    {
	/*
        _weightDensity = 1.0;
        _weightWire = wWire * totalPotentialGradient / totalWireGradient;
        */

	// fix "wire" change "spreading"
	_weightDensity = totalWireGradient / totalPotentialGradient;
	//_weightDensity = totalViaGradient / totalPotentialGradient;
	_weightWire = wWire;
	if(m_bMoveZ)
	{
	    double _weightTSV = (double)m_potentialGridSize;
		_weightTSV = param.dWeightTSV; /// frank: temp value
	    // gArg.GetDouble("TSV", &_weightTSV);
	    //m_weightTSV = _weightTSV * wWire;
	    //m_weightTSV = _weightTSV;
	    m_weightTSV = _weightTSV * (totalWireGradient/*-totalViaGradient*/) / totalViaGradient;
		//m_weightTSV = 0.0001;
		//m_weightTSV = wWire * _weightTSV;
	    //m_weightTSV = (m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left) / sqrt(m_pDB->m_totalLayer);
	    printf("weight TSV = %lf\n", m_weightTSV);
	}

	gArg.GetDouble( "weightWire", &_weightWire );
	
	if( param.bShow )
	{
	    printf( "Force strength: %.0f (wire)  %.0f (spreading)\n", 
		    totalWireGradient, totalPotentialGradient );
	}	
        assert( wWire == 0 || _weightWire > 0 );
    }
    //@Brian 2007-04-23
//    printf( " INIT: LogSumExp WL= %.0f, gradWL= %.0f\n", totalWL, totalWireGradient );
//    printf( " INIT: DensityPenalty= %.0f, gradPenalty= %.0f\n", density, totalPotentialGradient ); 

    for( unsigned int k = 0; k < m_weightDensity.size(); k++)
	for( unsigned int i=0; i<m_weightDensity[k].size(); i++ )
	    for( unsigned int j=0; j<m_weightDensity[k][i].size(); j++ )
		m_weightDensity[k][i][j] = _weightDensity;
    return true;
}
  
void MyNLP::UpdateObjWeights()
{

    //_weightWire /= m_weightIncreaseFactor;
    
    //Brian 2007-07-23
    if (param.bCongObj)
    {
	_weightWire /= m_weightIncreaseFactor;
	m_dWeightCong /= param.dCongDivRatio;
    }
    //@Brian 2007-07-23
    else
    {
	_weightDensity *= m_weightIncreaseFactor;
	//if(m_bMoveZ)
	//    m_weightTSV *= m_weightIncreaseFactor * 0.90;

	for( unsigned int k = 0; k < m_weightDensity.size(); k++)
	{
		for( unsigned int i=0; i<m_weightDensity[k].size(); i++ )
		{
		    for( unsigned int j=0; j<m_weightDensity[k][i].size(); j++ )
		    {

			if( gArg.CheckExist( "ada" ) && m_topLevel )
			{

			    double util = m_gridPotential[k][i][j] / m_expBinPotential[k][i][j];

			    // util > 1.0    overflow
			    // util < 1.0    underflow

			    util = max( 1.0, util );
			    util = min( 2.0, util );

			    util = util * util;
		
			    //util = ( util - 1.0 ) * 0.5 + 1.5;

			    m_weightDensity[k][i][j] = m_weightDensity[k][i][j] * ( util );
			}
			else
			    m_weightDensity[k][i][j] = m_weightDensity[k][i][j] * m_weightIncreaseFactor;
		    }
		}
	}
    }
    
}

