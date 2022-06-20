#include <cmath>
#include <set>
#include <vector>
#include <algorithm>
#include <cfloat>
#include <iostream>
#include <fstream>
#include <climits>
using namespace std;

#include "placedb.h"
#include "MyQP.h"
#include "smooth.h"
#include "TetrisLegal.h"
#include "placebin.h"
#include "ParamPlacement.h"
#include "placeutil.h"



double qtime_grad_f = 0;
double qtime_f = 0;
double qdensity;
double totalWL;

double qtime_lse = 0;
double time_update_grid = 0;
double qtime_grad_lse = 0;
double qtime_grad_potential = 0;
double qtime_update_bin = 0;
double qtime_up_potential = 0;
double qtime_exp = 0;
double qtime_sum_exp = 0;
double qtime_log = 0;

double stepSize;   


//void MacroInit::init()
//{
//    int gridSize=param.FPF_gridSize;
//    buildPinGrid( gridSize , gridSize , m_FixedPinPotential );
////        this->OutputFixedPinPotentialGrid("FixPinPreSmooth.dat");
//    GaussianSmooth smooth;
//    int r = param.FPF_GaussianTheta;
//    smooth.Gaussian2D( r, 6*r+1 );
//    smooth.Smooth( m_FixedPinPotential );
////        this->OutputFixedPinPotentialGrid("FixPinAfterSmooth.dat");
//
//    this->m_macroIds.clear();
//    this->m_macroIds.reserve(100);
//    for( int b=0; b<(int)m_pDB->m_modules.size(); b++ )
//    {
//        if(m_pDB->m_modules[b].m_isFixed==false && m_pDB->m_modules[b].m_height > m_pDB->m_rowHeight && m_pDB->m_modules[b].m_isCluster== false) //movable macros
//        {
//            this->m_macroIds.push_back(b);
//        }
//    }
//    this->m_stepX.resize(this->m_macroIds.size(),0);
//    this->m_stepY.resize(this->m_macroIds.size(),0);
//}
//void MacroInit::solve()
//{
//    double startFixedPinPotential=getFixedPinPotential();
//    cout<<"\n Initial PinPotential:"<<startFixedPinPotential;
//    for(int j=0; j<20; ++j)
//    {
//        addFixedPinForce();
//        for(int i=0; i<(int)this->m_macroIds.size(); ++i)
//        {
//            safeMoveCenter(m_macroIds[i], m_pDB->m_modules[m_macroIds[i]].m_cx + m_stepX[i], m_pDB->m_modules[m_macroIds[i]].m_cy + m_stepY[i]);
//        }
//        double newpot=getFixedPinPotential();
//        cout<<"\n iter "<<j<<" PinPotential:"<<newpot;
//        char filename[100];	 
//	    sprintf( filename, "pfig-%d.plt", j );
//	    m_pDB->OutputGnuplotFigure( filename, false, false );
//        if(newpot<0.01)
//        {
//            return;
//        }
//
//    }
//}
//void MacroInit::safeMoveCenter(int i,double cx, double cy)
//{
////	int i=moduleID;
//	if(m_pDB->m_modules[i].m_isFixed==true)
//		return;
//
//	double x= cx-m_pDB->m_modules[i].m_width*(double)0.5;
//	double y= cy-m_pDB->m_modules[i].m_height*(double)0.5;
//
//
//	if( (x+m_pDB->m_modules[i].m_width)>m_pDB->m_coreRgn.right)
//	{
//		x=m_pDB->m_coreRgn.right-m_pDB->m_modules[i].m_width;
//	}
//	else if(x<m_pDB->m_coreRgn.left)
//	{
//		x=m_pDB->m_coreRgn.left;
//	}
//	if( (y+m_pDB->m_modules[i].m_height)>m_pDB->m_coreRgn.top)
//	{
//		y=m_pDB->m_coreRgn.top-m_pDB->m_modules[i].m_height;
//	}
//	else if(y<m_pDB->m_coreRgn.bottom)
//	{
//		y=m_pDB->m_coreRgn.bottom;
//	}
//
//	m_pDB->SetModuleLocation(i,x,y);
//
//}
//
//void MacroInit::findGridNum(  const double x, const double y, int& xIndex, int& yIndex)
//{
//
//
//    xIndex=(int)floor( (x-m_pDB->m_coreRgn.left)/m_gridW );
//    yIndex=(int)floor( (y-m_pDB->m_coreRgn.bottom)/m_gridH );
//    if(xIndex<0)
//        xIndex=0;
//    if(xIndex>=this->m_num_X_grid)
//        xIndex=this->m_num_X_grid-1;
//    if(yIndex<0)
//        yIndex=0;
//    if(yIndex>=this->m_num_Y_grid)
//        yIndex=this->m_num_Y_grid-1;
//
//}
//void MacroInit::buildPinGrid( int x, int y, vector< vector<double> >& fixPinGrid)
//{
//
//    this->m_num_X_grid=x;
//    this->m_num_Y_grid=y;
//    this->m_gridW= (m_pDB->m_coreRgn.right-m_pDB->m_coreRgn.left)/this->m_num_X_grid;
//    this->m_gridH= (m_pDB->m_coreRgn.top-m_pDB->m_coreRgn.bottom)/this->m_num_Y_grid;
//
//    vector< vector<double> > temp_grid;
//
//    //initialize grid
//    temp_grid.resize(this->m_num_X_grid);
//    for( int i=0; i<(int)temp_grid.size(); ++i)
//    {
//        temp_grid[i].resize(this->m_num_Y_grid, 0 );
//        //for(int j=0; j<(int)temp_grid[i].size(); ++j )
//        //{
//        //    temp_grid[i][j].m_cx=(i+0.5)*m_gridW + db.m_coreRgn.left; 
//        //    temp_grid[i][j].m_cy=(j+0.5)*m_gridH + db.m_coreRgn.bottom;
//        //}
//    }
//
//    //for all fixed module, insert fixed pins to the grid
//    for(int i=0; i<(int)m_pDB->m_modules.size(); ++i)
//    {
//        if(m_pDB->m_modules[i].m_isFixed==true)
//        {
//            for( int j=0; j<(int)m_pDB->m_modules[i].m_pinsId.size(); ++j)
//            {
//                double px=m_pDB->m_pins[m_pDB->m_modules[i].m_pinsId[j]].absX;
//                double py=m_pDB->m_pins[m_pDB->m_modules[i].m_pinsId[j]].absY;
//                int xindex, yindex;
//                findGridNum( px, py, xindex,  yindex);
//                ++temp_grid[xindex][yindex];
//
//            }
//        }
//    }
//    fixPinGrid=temp_grid;
//
//}
//double MacroInit::getFixedPinPotential()
//{
//    double value=0;
//    for(int i=0; i<(int)this->m_macroIds.size(); ++i)
//    {
//        int xindex, yindex;
//        double px=m_pDB->m_modules[m_macroIds[i]].m_cx;
//        double py=m_pDB->m_modules[m_macroIds[i]].m_cy;
//        findGridNum( px, py, xindex,  yindex);
//        value+=this->m_FixedPinPotential[xindex][yindex];
//    }
//    return value;
//}
//
//void MacroInit::addFixedPinForce()
//{
//
//    double totalPF_potential=0;  // PF== Pin Force
//
//    for(int i=0; i<(int)this->m_macroIds.size(); ++i)
//    {
//        double currentValue;
//        int xindex, yindex;
//        double px=m_pDB->m_modules[m_macroIds[i]].m_cx;
//        double py=m_pDB->m_modules[m_macroIds[i]].m_cy;
//        findGridNum( px, py, xindex,  yindex);
//        currentValue=this->m_FixedPinPotential[xindex][yindex];
//        if(xindex<(this->m_num_X_grid-1))
//        {
//            m_stepX[i]=currentValue-m_FixedPinPotential[xindex+1][yindex];
//        }
//        else
//        {
//            m_stepX[i]=m_FixedPinPotential[xindex-1][yindex]-currentValue;
//        }
//        if(yindex<(this->m_num_Y_grid-1))
//        {
//            m_stepY[i]=currentValue-m_FixedPinPotential[xindex][yindex+1];
//        }
//        else
//        {
//            m_stepY[i]=m_FixedPinPotential[xindex][yindex-1]-currentValue;
//        }
//
//        totalPF_potential+=sqrt( m_stepX[i]*m_stepX[i] + m_stepY[i]*m_stepY[i] );
//    }
//
//    double ratio=0;
//    double avgStep=totalPF_potential/m_macroIds.size();
//    ratio=(5*m_pDB->m_rowHeight)/avgStep;
//
//    for(int i=0; i<(int)this->m_macroIds.size(); ++i)
//    {
//        m_stepX[i]=ratio*m_stepX[i];
//        m_stepY[i]=ratio*m_stepY[i];
//    }
//}
//-------------------------------------------------
//MyQP modified by tchsu
//-------------------------------------------------



/* Constructor. */
MyQP::MyQP( CPlaceDB& db )
    : _potentialGridR( 2 ),
      m_potentialGridSize( -1 ),
      m_targetUtil( 0.9 )
{
    m_lookAheadLegalization = false;
    m_earlyStop = false;
    m_topLevel = false;
    m_lastNLP = false;
    m_useBellPotentialForPreplaced = true;
    
    m_weightWire = 4.0;
    m_incFactor = 2.0;
    m_smoothR = 5;	// Gaussian smooth R
    m_smoothDelta = 1;
    //this->m_isFixedPinRepel=param.isFPF;

   
    //TODO: compute target density  (evenly distribute?)
    
    m_pDB = &db;
    InitModuleNetPinId();	    
    

    _cellPotentialNorm.resize( m_pDB->m_modules.size() );
    

    x.resize( 2 * m_pDB->m_modules.size() );
    cellLock.resize( m_pDB->m_modules.size(), false );
    xBest.resize( 2 * m_pDB->m_modules.size() );

    x_l.resize( 2 * m_pDB->m_modules.size() );
    x_u.resize( 2 * m_pDB->m_modules.size() );
    grad_f.resize( 2 * m_pDB->m_modules.size() );
    last_grad_f.resize( 2 * m_pDB->m_modules.size() );

    //by tchsu
    this->m_qp_netInfo.resize(m_pDB->m_nets.size());
   
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
	if( m_pDB->m_modules[i].m_isFixed )
	    cellLock[ i ] = true;
    
    m_usePin.resize( m_pDB->m_modules.size() );
    SetUsePin();

    grad_wire.resize( 2 * m_pDB->m_modules.size(), 0.0 );
    grad_potential.resize( 2 * m_pDB->m_modules.size(), 0.0 );

//    //by tchsu
//    if(this->m_isFixedPinRepel==true)
//    {
//        int gridSize=param.FPF_gridSize;
//        buildPinGrid( gridSize , gridSize , m_FixedPinPotential );
//        this->OutputFixedPinPotentialGrid("FixPinPreSmooth.dat");
//        GaussianSmooth smooth;
//        int r = param.FPF_GaussianTheta;
//        smooth.Gaussian2D( r, 6*r+1 );
//        smooth.Smooth( m_FixedPinPotential );
//        this->OutputFixedPinPotentialGrid("FixPinAfterSmooth.dat");
//
//        this->m_macroIds.clear();
//        this->m_macroIds.reserve(100);
//        for( int b=0; b<(int)m_pDB->m_modules.size(); b++ )
//        {
//            if(m_pDB->m_modules[b].m_isFixed==false && m_pDB->m_modules[b].m_height > m_pDB->m_rowHeight && m_pDB->m_modules[b].m_isCluster== false) //movable macros
//            {
//                this->m_macroIds.push_back(b);
//            }
//        }
//        this->m_macroIdsAreaFactor.resize(m_macroIds.size());
//
//        //find max macro size
//        double maxMacroSize=0;
//        for(int i=0; i<(int)this->m_macroIds.size(); ++i)
//        {
//            if(m_pDB->m_modules[m_macroIds[i]].m_area>maxMacroSize)
//                maxMacroSize=m_pDB->m_modules[m_macroIds[i]].m_area;
//        }
//
//        //build macro size factor
//        double factorSum=0;
//        for(int i=0; i<(int)this->m_macroIds.size(); ++i)
//        {
//            m_macroIdsAreaFactor[i]=m_pDB->m_modules[m_macroIds[i]].m_area/maxMacroSize;
//            factorSum+=m_macroIdsAreaFactor[i];
//        }
//        //scale to average = 1
//        double factorAvg=factorSum/m_macroIds.size();
//        for(int i=0; i<(int)this->m_macroIds.size(); ++i)
//        {
//            m_macroIdsAreaFactor[i]*=1/factorAvg;
//        }
//
//
////        cout<<"\n OutFixPinGrid!!\n";
//    }

    
}

MyQP::~MyQP()
{
}

void MyQP::SetUsePin()
{
    //printf( "row height = %f\n", m_pDB->m_rowHeight );
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




bool MyQP::MySolve( double wWire, 
		     double target_density, 
                     int currentLevel,	// for plotting
		     bool noRelaxSmooth
                   )
{

    double time_start = seconds();    
    //printf( "Start Optimization \n" );
    assert( _potentialGridR > 0 );
   
    
    if( m_potentialGridSize <= 0 )
	m_potentialGridSize = static_cast<int>( sqrt(static_cast<double>( m_pDB->m_modules.size()) ) * 0.8 );


    int n, m, nnz_jac_g, nnz_h_lag;
    get_nlp_info( n, m, nnz_jac_g, nnz_h_lag );
    get_bounds_info( n, x_l, x_u );
    get_starting_point( x );
    BoundX( n, x, x_l, x_u );

    if( param.bShow )
	printf( "MEM = %.0f MB\n", GetPeakMemoryUsage() );
    
    m_ite = 0;
    bool isLegal = false;
    //while( true )
    {

	if( param.bShow )
	    printf( "GRID = %d  (width = %.2f)\n", m_potentialGridSize, 
		    ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left )/m_potentialGridSize );

	if( m_topLevel )
	    m_lastNLP = true;
	else
	    m_lastNLP = false;
	
	isLegal = GoSolve( wWire, target_density, currentLevel );

    }

    if( param.bShow )
    {
	    printf( "HPWL = %.0f\n", m_pDB->CalcHPWL() );
	    printf( "\nLevel Time        = %.2f sec = %.2f min\n", 
		    double(seconds()-time_start), double(seconds()-time_start)/60.0  );
	    printf( "Time Sum-Exp      = %.1f sec\n", qtime_sum_exp );
	    printf( "Time up potential = %.1f sec\n", qtime_up_potential );
	    printf( "Time eval_f       = %.2f sec = (WL) %.2f + (P)%.2f\n", qtime_f, qtime_lse, time_update_grid );
	    printf( "Time eval_grad_f  = %.1f sec = (gradWL) %.1f + (gradP) %.1f\n", 
		    qtime_grad_f, qtime_grad_lse, qtime_grad_potential );
    }

    return isLegal;
}


bool MyQP::GoSolve( double wWire, 
		     double target_density, 
                     int currentLevel	// for plotting
                   )
{
    bool runLAL=true;
    double givenTargetUtil = m_targetUtil; // for look ahead legalization
    m_currentStep = param.step;
    
    //m_targetUtil += 0.05;
    m_targetUtil += param.targetDenOver;
    if( m_targetUtil > 1.0 )
	m_targetUtil = 1.0;
    
    
    double time_start = seconds();    
    char filename[100];	    // for gnuplot
    

    int n = 2 * m_pDB->m_modules.size();
    
    //double baseUtil = 0.05;
    double baseUtil = 0.075;
    
    double designUtil = m_pDB->m_totalMovableModuleArea / m_pDB->m_totalFreeSpace;
    if( param.bShow )
	printf( "INFO: Design utilization: %f\n", designUtil );
    if( m_targetUtil > 0 )  // has user-defined target utilization
    {
	// This part is very important for ISPD-06 Placement Contest.
	//double lowest = designUtil + 0.05;  // contest
	double lowest = min( 1.0, designUtil + baseUtil );
	if( m_targetUtil < lowest )
	{
	    if( param.bShow )
	    {
		    printf( "WARNING: Target utilization (%f) is too low\n", m_targetUtil );
		    printf( "         Set target utilization to %f\n", lowest );
	    }
	    m_targetUtil = lowest;
	}
    }
    else // no given utilization
    {
	printf( "No given target utilization. Distribute blocks evenly.\n" );
	//m_targetUtil = designUtil + 0.05;   // contest
	m_targetUtil = designUtil + baseUtil;
	if( m_targetUtil > 1.0 )
	    m_targetUtil = 1.0;	    
    }
    if( param.bShow )
	printf( "DBIN: Target utilization: %f\n", m_targetUtil );
    
    for( unsigned int i=0; i<2*m_pDB->m_modules.size(); i++ )
    {
	grad_f[i] = last_grad_f[i] = 0.0;
    }


    CreatePotentialGrid();   // create potential grid according to "m_potentialGridSize"
    
    //int densityGridSize = 10;	// 1% chip area
    int densityGridSize = m_potentialGridSize / 4;	    // not good in big3
    
    CreateDensityGrid( densityGridSize );	// real density: use 1% area
    UpdateDensityGridSpace( n, x );
    UpdatePotentialGridBase( x );		// init exp potential for each bin, also update ExpBin
    
#if 1
    // gaussian smoothing for base potential
    GaussianSmooth smooth;
    int r = m_smoothR;
    smooth.Gaussian2D( (double)r, 6*r+1 );
    smooth.Smooth( m_basePotential );
    m_basePotentialOri = m_basePotential;
    if( param.bShow )
    {
	sprintf( filename, "gbase%d.dat", currentLevel );
	OutputPotentialGrid( filename );
    }
#endif 


    // TEST
    if( m_smoothDelta == 1 )
    {
	    if( param.bShow )
	    {
	        sprintf( filename, "gbase%d-more.dat", currentLevel );
	        printf( "generate %s...\n", filename );
	        fflush( stdout );
	    }
    	
	    vector< vector< double > > moreSmooth = m_basePotential;
	    r = m_smoothR * 6;
	    int kernel_size = 5*r;
	    if( kernel_size % 2 == 0 )
	        kernel_size++;
	    smooth.Gaussian2D( (double)r, kernel_size );
	    smooth.Smooth( moreSmooth );

	    if( param.bShow )
	    {
	        swap( moreSmooth, m_basePotential );
	        OutputPotentialGrid( filename );
	        swap( moreSmooth, m_basePotential );
	    }

	    // merge base and moreSmooth
	    double binArea = m_potentialGridWidth * m_potentialGridHeight;
	    double halfBinArea = binArea / 2;
	    int changeCount = 0;
	    for( unsigned int i=0; i<moreSmooth.size(); i++ )
	    {
	        for( unsigned int j=0; j<moreSmooth[i].size(); j++ )
	        {
		    double free = binArea - m_basePotential[i][j];
		    if( free < 1e-4 )	// no space
		    {
		        if( moreSmooth[i][j] > halfBinArea )
		        {
			    m_basePotential[i][j] += moreSmooth[i][j] - halfBinArea;
			    changeCount++;
		        }	
		    }
	        }
	    }

	    if( param.bShow )
	    {
	        printf( "change %d\n", changeCount );
	        sprintf( filename, "gbase%d-more-merge.dat", currentLevel );
	        OutputPotentialGrid( filename );
	    }
    }


    if( m_smoothDelta > 1.0 )
	SmoothPotentialBase( double(m_smoothDelta) );   // also update ExpBin
    
    UpdateExpBinPotential( m_targetUtil );
   
#if 1 
    if( param.bShow )
    {
	sprintf( filename, "base%d.dat", currentLevel );
	OutputPotentialGrid( filename );
	// TEST
	/*for( int delta=1; delta<=10; delta++ )
	  {
	  SmoothPotentialBase( (double)delta );
	  sprintf( filename, "base%d-%d.dat", currentLevel, delta );
	  OutputPotentialGrid( filename );
	  }*/
    }
#endif


    assert( m_targetUtil > 0 );
    
    // wirelength 

    UpdateNetsSum( x);
    totalWL = GetQpWL( x );

    // density
    UpdatePotentialGrid( x );
    UpdateDensityGrid( n, x );
    qdensity = GetDensityPanelty();

    // 2006-02-22 weight (APlace ICCAD05)
    _weightWire = 1.0;    
    eval_grad_f( n, x, true, grad_f );
    double totalWireGradient = 0;
    double totalPotentialGradient = 0;

    // TODO: truncation?
    AdjustForce( n, x, grad_wire, grad_potential );
		
    for( int i=0; i<n; i++ )
    {
	    totalWireGradient      += fabs( grad_wire[i] );
	    totalPotentialGradient += fabs( grad_potential[i] );

    }

    
    _weightDensity = 1.0;
    _weightWire = wWire * totalPotentialGradient / totalWireGradient;
    int maxIte = 50;	// max outerIte	   
    
    bool newDir = true;
    double obj_value;
    double beta;	// determined by CG
    eval_f( n, x,  true, obj_value );
    eval_grad_f( n, x, true, grad_f );

    double nnb_real = GetNonZeroDensityGridPercent();
    UpdateDensityGrid( n, x );
    double maxDen = GetMaxDensity();
    double totalOverDen = GetTotalOverDensity();
    double totalOverDenLB = GetTotalOverDensityLB();
    double totalOverPotential = GetTotalOverPotential();
    
    //printf( "INIT f = %f\n", obj_value );
   
    if( param.bShow )
    {	
	printf( " %d-%2d HPWL= %.0f\tDen= %.2f %.2f %.2f %.2f NNB= %.2f Dcost= %4.1f%%  WireW= %.0f ", 
		currentLevel, m_ite, m_pDB->CalcHPWL(), 
		maxDen, totalOverDen, totalOverDenLB, totalOverPotential,
		nnb_real, 
		qdensity * _weightDensity / obj_value * 100.0, _weightWire  
	      );
    }
    else
    {
	    printf( " %d-%2d HPWL= %.0f \t", 
		    currentLevel, m_ite, m_pDB->CalcHPWL() 
	        );
    }
    fflush( stdout );
    if( param.bShow )
    {
	    sprintf( filename, "fig%d-%d.plt", currentLevel, m_ite );
	    m_pDB->OutputGnuplotFigure( filename, false, false );
    }

    double lastTotalOver = 0;
    double lastTotalOverPotential = DBL_MAX;
    double over = totalOverDen;
    int totalIte = 0;
   
    bool hasBestLegalSol = false;
    double bestLegalWL = DBL_MAX;
    int lookAheadLegalCount = 0;
    double totalLegalTime = 0.0;
    //double norm_move = 0.0;
    
    bool startDecreasing = false;
    
    int checkStep = 5; 
    int outStep = 25;
    if( param.bShow == false )
	outStep = INT_MAX;

    int LALnoGoodCount = 0;
   
    
    for( int ite=0; ite<maxIte; ite++ )
    {
	m_ite++;
	int innerIte = 0;
	double old_obj = DBL_MAX;
	double last_obj_value = DBL_MAX;

	m_currentStep = param.step;
	
	newDir = true;
	while( true )	// inner loop, minimize "f" 
	{
	    innerIte++;
	    swap( last_grad_f, grad_f );    // save for computing the congujate gradient direction
	    eval_grad_f( n, x,  true, grad_f );
	    AdjustForce( n, x, grad_f );

	    if( innerIte % checkStep == 0 )
	    {
		old_obj = last_obj_value;    // backup the old value
		eval_f( n, x, true, obj_value );
		last_obj_value = obj_value;
	    }
	    
#if 1
	    // Output solving progress
	    if( innerIte % outStep == 0 && innerIte != 0 )
	    {
		if( innerIte % checkStep != 0 )
		    eval_f( n, x, true, obj_value );
		printf( "\n\t  (%4d): f= %.10g\tstep= %.6f \t %.1fm ", 
			innerIte, 
			obj_value, 
			stepSize,
			double(seconds()-time_start)/60.0
		      );

		fflush( stdout );
	    }
#endif

	    if( innerIte % checkStep == 0 )
	    {
		printf( "." );
		fflush( stdout );

		if( innerIte % 2 * checkStep == 0 )
		{
		    UpdateBlockPosition( x );   // update to placeDB
		    if( m_pDB->CalcHPWL() > bestLegalWL )   // gWL > LAL-WL
		    {
			printf( "X\n" );
			fflush( stdout );
			break;	
		    }
		}

		UpdateDensityGrid( n, x );  // find the exact bin density
		totalOverDen = GetTotalOverDensity();
		totalOverDenLB = GetTotalOverDensityLB();
		totalOverPotential = GetTotalOverPotential();

		lastTotalOver = over;
		over = min( totalOverPotential, totalOverDen ); // TEST

		if( !startDecreasing
			&& over < lastTotalOver 
			&& ite >= 1 
			&& innerIte >= 6 )
		{
		    printf( ">>" );
		    fflush( stdout );
		    startDecreasing = true;
		}

		// 2005-03-11: meet the constraint 
		if( startDecreasing && over < target_density )
		    break;

		    // Cannot further improve 
		    if( obj_value >= 0.99999 * old_obj )   
		    {
		        break;

		    }
	    }


	    // Calculate d_k (conjugate gradient method)
	    if( newDir == true )	
	    {
		    // gradient direction
		    newDir = false;
		    for( int i=0; i<n; i++ )
		        grad_f[i] = -grad_f[i];
	        }
	        else
	        {
		        // conjugate gradient direction
		        FindBeta( n, grad_f, last_grad_f, beta );
		        for( int i=0; i<n; i++ )
		            grad_f[i] = -grad_f[i] + beta * last_grad_f[i];
	        }


	        // Calculate a_k (step size)
	        LineSearch( n, x, grad_f, stepSize );

	        // Update X. (x_{k+1} = x_{k} + \alpha_k * d_k)
	        double move;
	        for( int i=0; i<n; i++ )
	        {
		    move = grad_f[i] * stepSize;
		    x[i] += move;
	    }
	   
	    
	    BoundX( n, x, x_l, x_u );
	    double time_used = seconds();

	    UpdateNetsSum( x );
	    qtime_grad_lse += seconds() - time_used;
	    UpdatePotentialGrid( x );

	}// inner loop

	if( param.bShow )
	{
	    printf( "%d\n", innerIte );
	    fflush( stdout );
	}
	else
	    printf( "\n" );
	totalIte += innerIte;

	UpdateDensityGrid( n, x );
	double nnb_real = GetNonZeroDensityGridPercent();
	maxDen = GetMaxDensity();
	totalOverDen = GetTotalOverDensity();
	totalOverDenLB = GetTotalOverDensityLB();
	totalOverPotential = GetTotalOverPotential();
	
	UpdateBlockPosition( x );   // update to placeDB

#if 1
	if( param.bShow )
	{
	    // output figures
	    sprintf( filename, "fig%d-%d.plt", currentLevel, m_ite );
	    m_pDB->OutputGnuplotFigure( filename, false, false );	// it has "CalcHPWL()"
	    
	    if( m_topLevel && param.bOutTopPL ) // debugging
	    {
		sprintf( filename, "fig%d-%d.pl", currentLevel, m_ite );
		m_pDB->OutputPL( filename );	
	    }

	    sprintf( filename, "potential%d-%d.dat", currentLevel, m_ite );
	    OutputPotentialGrid( filename );
	    
	    sprintf( filename, "util%d-%d", currentLevel, m_ite );
	    CPlaceBin placeBin( *m_pDB );
	    if( param.coreUtil < 1.0 )
		placeBin.CreateGrid( m_pDB->m_rowHeight * 10.0, m_pDB->m_rowHeight * 10.0 ); // ispd06-contest bin size
	    else
		placeBin.CreateGrid( m_potentialGridSize, m_potentialGridSize );
	    placeBin.OutputBinUtil( filename, false );
	}
#endif

	if( param.bShow )
	{
	    printf( " %d-%2d HPWL= %.0f\tDen= %.2f %.4f %.4f %.4f NNB= %.2f LTime= %.1fm Dcost= %4.1f%% WireW= %.0f ", 
		    currentLevel, m_ite, m_pDB->CalcHPWL(), 
		    maxDen, totalOverDen, totalOverDenLB, totalOverPotential,
		    nnb_real, 
		    double(seconds()-time_start)/60.0, 
		    qdensity*_weightDensity /obj_value * 100.0, 
		    _weightWire
		  );
	}
	else
	{
	    printf( " %d-%2d HPWL= %.f\tLTime= %.1fm ", 
		    currentLevel, m_ite, m_pDB->CalcHPWL(), 
		    double(seconds()-time_start)/60.0 
		  );
	}
	fflush( stdout );

	
#if 1
	// 2006-03-06 (CAUTION! Do not use look-ahead legalization when dummy block exists.
	// TODO: check if there is dummy block (m_modules[].m_isDummy)
	if( /*param.bRunLAL*/runLAL && m_ite >= 2 && m_lookAheadLegalization && over < target_density+0.25 )
	//if( param.bRunLAL && m_ite >= 2 && m_lookAheadLegalization && over < target_density+0.10 )	// for ispd06
	//if( startDecreasing && m_lookAheadLegalization ) // test
	{
	    UpdateBlockPosition( x );   // update to placeDB
	    double hpwl = m_pDB->CalcHPWL();
	    if( hpwl > bestLegalWL )
	    {
		printf( "Stop. Good enough.\n" );
		break;	
	    }

	    lookAheadLegalCount++;
	    double oldWL = hpwl;
	    CTetrisLegal legal(*m_pDB);

	    double scale = 0.85;
	    if( givenTargetUtil < 1.0 && givenTargetUtil > 0 )
		scale = 0.9;

	    double legalStart = seconds();
	    m_pDB->RemoveFixedBlockSite(); //indark 
	    // legalizer should handle cell orientation
	    bool bLegal = legal.Solve( givenTargetUtil, false, false, scale );	
	    
	    double legalTime = seconds() - legalStart;
	    totalLegalTime += legalTime;
	    if( param.bShow )
		printf( "LAL Time: %.2f\n", legalTime );
	    if( bLegal )
	    {
		m_pDB->Align();	// 2006-04-02
		
		double WL = m_pDB->GetHPWLdensity( givenTargetUtil );
		if( param.bShow )
		    m_pDB->ShowDensityInfo();
		if( WL < bestLegalWL )
		{
		    // record the best legal solution
		    LALnoGoodCount = 0;
		    if( param.bShow )
			printf( "SAVE BEST! (HPWL=%.0f)(dHPWL= %.0f)(%.2f%%)\n", 
				m_pDB->GetHPWLp2p(), WL, (WL-oldWL)/oldWL*100 );
		    bestLegalWL = WL;
		    hasBestLegalSol = true;
		    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
		    {
			xBest[2*i] = m_pDB->m_modules[i].m_cx;
			xBest[2*i+1] = m_pDB->m_modules[i].m_cy;
		    }	    
		}
		else
		{
		    if( param.bShow )
			printf( "(HPWL=%.0f)(dHPWL= %.0f)(%.2f%%)\n", m_pDB->GetHPWLp2p(), WL, (WL-oldWL)/oldWL*100 );
		    if( (WL-oldWL)/oldWL < 0.075 )
		    {
			if( param.bShow )
			    printf( "Stop. Good enough\n" ); 
			break;
		    }
		    LALnoGoodCount++;
		    if( LALnoGoodCount >= 2 )
			break;
		}
	    }
	}
#endif	

	if( ite >= 2 )
	{
	    if( startDecreasing && over < target_density )
	    {
		printf( "Meet constraint!\n" );
		break;
	    }

	    // cannot reduce totalOverPotential
	    if( ite > 3 && totalOverPotential > lastTotalOverPotential &&
		    totalOverPotential < 1.4 )
	    {
		printf( "Cannot further reduce!\n" );
		break;
	    }
	}

	_weightWire /= m_incFactor;
	lastTotalOverPotential = totalOverPotential;
    
    }// outer loop


    // 2006-03-06 (donnie)
    if( hasBestLegalSol )
    {
	    for( unsigned int i=0; i<x.size(); i++ )
	        x[i] = xBest[i];
	//x = xBest;
	//memcpy( x, xBest, sizeof(double)*n );
    }
    UpdateBlockPosition( x );
    //m_pDB->OutputGnuplotFigure( "LAL_Result.plt",false);
    if( lookAheadLegalCount > 0 && param.bShow )
    {
	    printf( "LAL: Total Count: %d\n", lookAheadLegalCount );
	    printf( "LAL: Total CPU: %.2f\n", totalLegalTime );
	    sprintf( filename, "util-global.dat" );
	    CPlaceBin placeBin( *m_pDB );
	    placeBin.CreateGrid( m_pDB->m_rowHeight * 10.0, m_pDB->m_rowHeight * 10.0 );
	    placeBin.OutputBinUtil( filename );
    }
    
    static int allTotalIte = 0;
    allTotalIte += totalIte;

    if( param.bShow )
    {
	    m_pDB->ShowDensityInfo();
	    printf( "\nLevel Ite %d   Total Ite %d\n", totalIte, allTotalIte );
    }
    
    
    return hasBestLegalSol;
}


void MyQP::FindBeta( const int& n, const vector<double>& grad_f, const vector<double>& last_grad_f, double& beta )
{
    // Polak-Ribiere foumula from APlace journal paper
    // NOTE:
    //   g_{k-1} = -last_grad_f
    //   g_k     = grad_f

    double l2norm = 0;
    for( int i=0; i<n; i++ )
	l2norm += last_grad_f[i] * last_grad_f[i];

    double product = 0;
    for( int i=0; i<n; i++ )
	product += grad_f[i] * ( grad_f[i] + last_grad_f[i] );	// g_k^T ( g_k - g_{k-1} )
    beta = product / l2norm;
}


void MyQP::BoundX( const int& n, vector<double>& x, vector<double>& x_l, vector<double>& x_h, const int& i )
{
    if( x[i] < x_l[i] )      x[i] = x_l[i];
    else if( x[i] > x_h[i] )	x[i] = x_h[i];
}


void MyQP::BoundX( const int& n, vector<double>& x, vector<double>& x_l, vector<double>& x_h )
{
    for( int i=0; i<n; i++ )
    {
	if( x[i] < x_l[i] )             x[i] = x_l[i];
        else if( x[i] > x_h[i] )	x[i] = x_h[i];
    } 
}



void MyQP::AdjustForce( const int& n, const vector<double>& x, vector<double>& f )
{
    if( param.bAdjustForce == false )
	return;//no adjust
    
    double totalGrad = 0;
    int size = n/2;
    for( int i=0; i<size; i++ )
    {
	    double value = f[2*i] * f[2*i] + f[2*i+1] * f[2*i+1];
	    totalGrad += value;
    }
    double avgGrad = sqrt( totalGrad / size );
 
    // Do truncation
    double expMaxGrad = avgGrad * param.truncationFactor;	// x + y
    double expMaxGradSquare = expMaxGrad * expMaxGrad;
    for( int i=0; i<size; i++ )
    {
	    double valueSquare = ( f[2*i]*f[2*i] + f[2*i+1]*f[2*i+1] );
	    if( valueSquare > expMaxGradSquare )
	    {
	        double value = sqrt( valueSquare );
	        f[2*i]   = f[2*i]   * expMaxGrad / value;
	        f[2*i+1] = f[2*i+1] * expMaxGrad / value;
	    }
    }
}


void MyQP::AdjustForce( const int& n, const vector<double>& x, vector<double> grad_wl, vector<double> grad_potential )
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
    double avgGrad = sqrt( totalGrad / size );
 
    // Do truncation
    double expMaxGrad = avgGrad * param.truncationFactor;	// x + y
    double expMaxGradSquare = expMaxGrad * expMaxGrad;
    for( int i=0; i<size; i++ )
    {
	    double valueSquare = 
	        (grad_wl[2*i] + grad_potential[2*i]) * (grad_wl[2*i] + grad_potential[2*i]) + 
	        (grad_wl[2*i+1] + grad_potential[2*i+1]) * (grad_wl[2*i+1] + grad_potential[2*i+1]); 
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


void MyQP::LineSearch( const int& n, /*const*/ vector<double>& x, vector<double>& f, double& stepSize )
{
    int size = n / 2;
    double totalGrad = 0;
    for( int i=0; i<n; i++ )
	totalGrad += f[i] * f[i];
    double avgGrad = sqrt( totalGrad / size );
    stepSize = m_potentialGridWidth / avgGrad * m_currentStep;	
    
    return;
}

bool MyQP::get_nlp_info(int& n, int& m, int& nnz_jac_g, 
			 int& nnz_h_lag/*, IndexStyleEnum& index_style*/)
{

    n = m_pDB->m_modules.size() * 2;  
    m = 0;	    // no constraint
    nnz_jac_g = 0;  // 0 nonzeros in the jacobian since no constraint
    return true;
}


bool MyQP::get_bounds_info(int n, vector<double>& x_l, vector<double>& x_u )
{

  assert(n == (int)m_pDB->m_modules.size() * 2);
  //assert(m == 0);
  
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


//use the module locations in the placeDB as the initial position 
bool MyQP::get_starting_point( vector<double>& x )
{  
  for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
  {
	x[2*i]   = m_pDB->m_modules[i].m_cx;
	x[2*i+1] = m_pDB->m_modules[i].m_cy;
  }
  return true;
}

void MyQP::UpdateNetsSum( const vector<double>& x )
{
    for( unsigned int n=0; n<m_pDB->m_nets.size(); n++ )
    {
        if(m_pDB->m_nets[n].size()<1)
            continue;
        int i=n;
        int BlockId=m_pDB->m_pins[m_pDB->m_nets[i][0]].moduleId;
        
	double maxX=x[2*BlockId]   + m_pDB->m_pins[m_pDB->m_nets[i][0]].xOff;
        double minX=x[2*BlockId]   + m_pDB->m_pins[m_pDB->m_nets[i][0]].xOff;
        double maxY=x[2*BlockId+1] + m_pDB->m_pins[m_pDB->m_nets[i][0]].yOff;
        double minY=x[2*BlockId+1] + m_pDB->m_pins[m_pDB->m_nets[i][0]].yOff;

        int maxXid = m_pDB->m_nets[i][0];
        int maxYid = maxXid;
        int minXid = maxYid;
        int minYid = minXid;

        for(int j=1; j<(int)m_pDB->m_nets[i].size(); ++j)
        {
            BlockId=m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId;
            if(x[2*BlockId]+m_pDB->m_pins[m_pDB->m_nets[i][j]].xOff<minX)
            {
                minX=x[2*BlockId]+m_pDB->m_pins[m_pDB->m_nets[i][j]].xOff;
                minXid=m_pDB->m_nets[i][j];
            }
            else if(x[2*BlockId]+m_pDB->m_pins[m_pDB->m_nets[i][j]].xOff>maxX)
            {
                maxX=x[2*BlockId]+m_pDB->m_pins[m_pDB->m_nets[i][j]].xOff;
                maxXid=m_pDB->m_nets[i][j];
            }

            if(x[2*BlockId+1]+m_pDB->m_pins[m_pDB->m_nets[i][j]].yOff<minY)
            {
                minY=x[2*BlockId+1]+m_pDB->m_pins[m_pDB->m_nets[i][j]].yOff;
                minYid=m_pDB->m_nets[i][j];
            }
            else if(x[2*BlockId+1]+m_pDB->m_pins[m_pDB->m_nets[i][j]].yOff>maxY)
            {
                maxY=x[2*BlockId+1]+m_pDB->m_pins[m_pDB->m_nets[i][j]].yOff;
                maxYid=m_pDB->m_nets[i][j];
            }            
        }
        m_qp_netInfo[i].m_bottomPinId= minYid;
        m_qp_netInfo[i].m_topPinId   = maxYid;
        m_qp_netInfo[i].m_leftPinId  = minXid;
        m_qp_netInfo[i].m_rightPinId = maxXid;
        m_qp_netInfo[i].width  = maxX-minX;
        m_qp_netInfo[i].height = maxY-minY;
        m_qp_netInfo[i].l_h.resize(m_pDB->m_nets[i].size());
        m_qp_netInfo[i].l_v.resize(m_pDB->m_nets[i].size());
        for(int j=0; j<(int)m_pDB->m_nets[i].size(); ++j)
        {
            BlockId=m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId;
            
	    double zero = 1e-12;
	    
	    m_qp_netInfo[i].l_h[j] = x[2*BlockId]   + m_pDB->m_pins[m_pDB->m_nets[i][j]].xOff - minX;
            m_qp_netInfo[i].l_v[j] = x[2*BlockId+1] + m_pDB->m_pins[m_pDB->m_nets[i][j]].yOff - minY;
	   
#if 0 
	    if( m_qp_netInfo[i].l_v[j] > m_qp_netInfo[i].height )
	    {
		printf( "%g !< %g    diff = %g\n", m_qp_netInfo[i].l_v[j], m_qp_netInfo[i].height, 
			m_qp_netInfo[i].l_v[j]-m_qp_netInfo[i].height);
	    }

	    if( m_qp_netInfo[i].l_h[j] > m_qp_netInfo[i].width )
	    {
		printf( "%g !< %g    diff = %g\n", m_qp_netInfo[i].l_h[j], m_qp_netInfo[i].width, 
			m_qp_netInfo[i].l_h[j]-m_qp_netInfo[i].width);
		printf( " min %g  max %g    %g\n", minX, maxX, m_pDB->m_pins[m_pDB->m_nets[i][j]].absX );
	    }
	    assert( m_qp_netInfo[i].l_h[j] >= 0 );
	    assert( m_qp_netInfo[i].l_h[j] <= m_qp_netInfo[i].width );
	    assert( m_qp_netInfo[i].l_v[j] >= 0 );
	    assert( m_qp_netInfo[i].l_v[j] <= m_qp_netInfo[i].height );
#endif
	    if( m_qp_netInfo[i].l_h[j] <= 0 )  
		m_qp_netInfo[i].l_h[j] = zero;
	    if( m_qp_netInfo[i].l_v[j] <= 0 )  
		m_qp_netInfo[i].l_v[j] = zero;
	    if( m_qp_netInfo[i].l_h[j] >= m_qp_netInfo[i].width )  
		m_qp_netInfo[i].l_h[j] = m_qp_netInfo[i].width - zero;
	    if( m_qp_netInfo[i].l_v[j] >= m_qp_netInfo[i].height )  
		m_qp_netInfo[i].l_v[j] = m_qp_netInfo[i].height - zero;
	    
            //if(m_qp_netInfo[i].l_h[j] <=0 && m_pDB->m_nets[i][j] != m_qp_netInfo[i].m_leftPinId )
            //    m_qp_netInfo[i].l_h[j] = zero;
	    
            //if(m_qp_netInfo[i].l_h[j] >= m_qp_netInfo[i].width && m_pDB->m_nets[i][j] != m_qp_netInfo[i].m_rightPinId )
            //    m_qp_netInfo[i].l_h[j] = m_qp_netInfo[i].width - zero;

            //if(m_qp_netInfo[i].l_v[j] <=0 && m_pDB->m_nets[i][j]!=m_qp_netInfo[i].m_bottomPinId )
            //    m_qp_netInfo[i].l_v[j] = zero;

            //if(m_qp_netInfo[i].l_v[j] >= m_qp_netInfo[i].height && m_pDB->m_nets[i][j] != m_qp_netInfo[i].m_topPinId )
            //    m_qp_netInfo[i].l_v[j] = m_qp_netInfo[i].height - zero;
            
	    m_qp_netInfo[i].m_pinId2index[ m_pDB->m_nets[i][j] ] = j;
            
        }
    }
}

double MyQP::GetQpWL(  const vector<double>& x)
{
    totalWL = 0;
    for( unsigned int n=0; n<m_pDB->m_nets.size(); n++ )	// for each net
    {
	    if( m_pDB->m_nets[n].size() < 2 )
	        continue;

	// (donnie) comment: == HPWL ?
	int leftPinId = m_qp_netInfo[n].m_leftPinId;
	int rightPinId = m_qp_netInfo[n].m_rightPinId;
	int topPinId = m_qp_netInfo[n].m_topPinId;
	int bottomPinId = m_qp_netInfo[n].m_bottomPinId;	
        totalWL +=
        (x[2*m_pDB->m_pins[rightPinId].moduleId] + m_pDB->m_pins[rightPinId].xOff)-
        (x[2*m_pDB->m_pins[leftPinId].moduleId] + m_pDB->m_pins[leftPinId].xOff) +
        (x[2*m_pDB->m_pins[topPinId].moduleId+1] + m_pDB->m_pins[topPinId].yOff)-
        (x[2*m_pDB->m_pins[bottomPinId].moduleId+1] + m_pDB->m_pins[bottomPinId].yOff);

    }
    return totalWL;
}
    
//evaluate the wire length force and density force
bool MyQP::eval_f(int n, const vector<double>& x,  bool new_x, double& obj_value)
{
    double time_start = seconds();
    
    totalWL = GetQpWL( x );
    qtime_lse += seconds() - time_start;
    
    double time_start_2 = seconds();
    qdensity = GetDensityPanelty();
    time_update_grid += seconds() - time_start_2;

    obj_value = (totalWL * _weightWire) + (qdensity * _weightDensity);

    //if(this->m_isFixedPinRepel==true)
    //{
    //    obj_value+=getFixedPinPotential();
    //}
    //
    
    qtime_f += seconds() - time_start;    
    return true;
}


void MyQP::PrintPotentialGrid()
{
    for( int i=(int)m_gridPotential.size()-1; i>=0; i-- )
    {
	for( unsigned int j=0; j<m_gridPotential[i].size(); j++ )
	{
	    printf( "%4.1f ", (m_gridPotential[i][j]-m_expBinPotential[i][j])/m_expBinPotential[i][j] );
	}
	printf( "\n" );
    }
    printf( "\n\n" );
}


double MyQP::GetDensityPanelty()
{
    double density = 0;
    for( unsigned int i=0; i<m_gridPotential.size(); i++ )
    {
	    for( unsigned int j=0; j<m_gridPotential[i].size(); j++ )
	    {
	        density += ( m_gridPotential[i][j] - m_expBinPotential[i][j] ) *
		    ( m_gridPotential[i][j] - m_expBinPotential[i][j] );
	    }
    }
    return density;
}

void MyQP::InitModuleNetPinId()
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

	        m_moduleNetPinId[i][j] = pinId;
	    } // each net to the module
    } // each module
}

//evaluate the wire length force gradient and density force gradient
bool MyQP::eval_grad_f(int n, const vector<double>& x, bool new_x, vector<double>& grad_f)
{
    double time_used = seconds();
   
    // grad WL
    if( _weightWire > 0 )	//TEST
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )	// for each block
    {
	    if( m_pDB->m_modules[i].m_isFixed || m_pDB->m_modules[i].m_netsId.size() == 0 )
	        continue;
	    if( cellLock[i] == true )
	        continue;    	
	    grad_wire[ 2*i ] = 0;
	    grad_wire[ 2*i+1 ] = 0;

	    for( unsigned int j=0; j<m_pDB->m_modules[i].m_netsId.size(); j++ )
	    {
	        // for each net connecting to the block
	        int netId = m_pDB->m_modules[i].m_netsId[j];

	        if( m_pDB->m_nets[netId].size() < 2 )  // floating-module
	    	    continue;

	        int selfPinId = m_moduleNetPinId[i][j];

	        if( m_usePin[i] )
		{	    
		    if(m_qp_netInfo[netId].m_leftPinId==selfPinId)
		    {
			double grad=0;
			for(int k=0; k<(int)m_pDB->m_nets[netId].size(); ++k)
			{
			    int pid=m_pDB->m_nets[netId][k];

			    if(pid!=selfPinId)
			    {
				grad+=( (x[2*m_pDB->m_pins[selfPinId].moduleId]+m_pDB->m_pins[selfPinId].xOff)-
					(x[2*m_pDB->m_pins[pid].moduleId]+m_pDB->m_pins[pid].xOff) ) / m_qp_netInfo[netId].l_h[k];
			    }

			}
			grad_wire[ 2*i ] += (2*grad)/(m_pDB->m_nets[netId].size()-1);

		    }
		    else if(m_qp_netInfo[netId].m_rightPinId==selfPinId)
		    {
			double grad=0;
			for(int k=0; k<(int)m_pDB->m_nets[netId].size(); ++k)
			{
			    int pid=m_pDB->m_nets[netId][k];

			    if(pid!=selfPinId)
			    {
				grad+=( (x[2*m_pDB->m_pins[selfPinId].moduleId]+m_pDB->m_pins[selfPinId].xOff)-
					(x[2*m_pDB->m_pins[pid].moduleId]+m_pDB->m_pins[pid].xOff) )/(m_qp_netInfo[netId].width-m_qp_netInfo[netId].l_h[k]);
			    }

			}
			grad_wire[ 2*i ] += (2*grad)/(m_pDB->m_nets[netId].size()-1);

		    }
		    else
		    {
			double grad=( (x[2*m_pDB->m_pins[selfPinId].moduleId]+m_pDB->m_pins[selfPinId].xOff)-
				(x[2*m_pDB->m_pins[m_qp_netInfo[netId].m_leftPinId].moduleId]+
				 m_pDB->m_pins[m_qp_netInfo[netId].m_leftPinId].xOff) )/m_qp_netInfo[netId].l_h[m_qp_netInfo[netId].m_pinId2index[selfPinId]] 
			    +
			    ( (x[2*m_pDB->m_pins[selfPinId].moduleId]+m_pDB->m_pins[selfPinId].xOff)-
			      (x[2*m_pDB->m_pins[m_qp_netInfo[netId].m_rightPinId].moduleId]+
			       m_pDB->m_pins[m_qp_netInfo[netId].m_rightPinId].xOff) )/
			    (m_qp_netInfo[netId].width-m_qp_netInfo[netId].l_h[m_qp_netInfo[netId].m_pinId2index[selfPinId]]);


			grad_wire[ 2*i ] += (2*grad)/(m_pDB->m_nets[netId].size()-1);

		    }

		    if(m_qp_netInfo[netId].m_bottomPinId==selfPinId)
		    {
			double grad=0;
			for(int k=0; k<(int)m_pDB->m_nets[netId].size(); ++k)
			{
			    int pid=m_pDB->m_nets[netId][k];

			    if(pid!=selfPinId)
			    {
				grad+=( (x[2*m_pDB->m_pins[selfPinId].moduleId+1]+m_pDB->m_pins[selfPinId].yOff)-
					(x[2*m_pDB->m_pins[pid].moduleId+1]+m_pDB->m_pins[pid].yOff) )/m_qp_netInfo[netId].l_v[k];
			    }

			}
			grad_wire[ 2*i+1 ] += (2*grad)/(m_pDB->m_nets[netId].size()-1);

		    }
		    else if(m_qp_netInfo[netId].m_topPinId==selfPinId)
		    {
			double grad=0;
			for(int k=0; k<(int)m_pDB->m_nets[netId].size(); ++k)
			{
			    int pid=m_pDB->m_nets[netId][k];

			    if(pid!=selfPinId)
			    {
				grad+=( (x[2*m_pDB->m_pins[selfPinId].moduleId+1]+m_pDB->m_pins[selfPinId].yOff)-
					(x[2*m_pDB->m_pins[pid].moduleId+1]+m_pDB->m_pins[pid].yOff) )/(m_qp_netInfo[netId].height-m_qp_netInfo[netId].l_v[k]);
			    }

			}
			grad_wire[ 2*i+1 ] += (2*grad)/(m_pDB->m_nets[netId].size()-1);

		    }
		    else
		    {
			double grad=( (x[2*m_pDB->m_pins[selfPinId].moduleId+1]+m_pDB->m_pins[selfPinId].yOff)-
				(x[2*m_pDB->m_pins[m_qp_netInfo[netId].m_bottomPinId].moduleId+1]+
				 m_pDB->m_pins[m_qp_netInfo[netId].m_bottomPinId].yOff) )/m_qp_netInfo[netId].l_v[m_qp_netInfo[netId].m_pinId2index[selfPinId]] 
			    +
			    ( (x[2*m_pDB->m_pins[selfPinId].moduleId+1]+m_pDB->m_pins[selfPinId].yOff)-
			      (x[2*m_pDB->m_pins[m_qp_netInfo[netId].m_topPinId].moduleId+1]+
			       m_pDB->m_pins[m_qp_netInfo[netId].m_topPinId].yOff) )/
			    (m_qp_netInfo[netId].height-m_qp_netInfo[netId].l_v[m_qp_netInfo[netId].m_pinId2index[selfPinId]]);


			grad_wire[ 2*i+1 ] += (2*grad)/(m_pDB->m_nets[netId].size()-1);

		    }
		}
	        else
		{
		    if(m_qp_netInfo[netId].m_leftPinId==selfPinId)
		    {
			double grad=0;
			for(int k=0; k<(int)m_pDB->m_nets[netId].size(); ++k)
			{
			    int pid=m_pDB->m_nets[netId][k];

			    if(pid!=selfPinId)
			    {
				grad+=( (x[2*m_pDB->m_pins[selfPinId].moduleId])-
					(x[2*m_pDB->m_pins[pid].moduleId]) )/m_qp_netInfo[netId].l_h[k];
			    }

			}
			grad_wire[ 2*i ] += (2*grad)/(m_pDB->m_nets[netId].size()-1);

		    }
		    else if(m_qp_netInfo[netId].m_rightPinId==selfPinId)
		    {
			double grad=0;
			for(int k=0; k<(int)m_pDB->m_nets[netId].size(); ++k)
			{
			    int pid=m_pDB->m_nets[netId][k];

			    if(pid!=selfPinId)
			    {
				grad+=( (x[2*m_pDB->m_pins[selfPinId].moduleId])-
					(x[2*m_pDB->m_pins[pid].moduleId] ))/(m_qp_netInfo[netId].width-m_qp_netInfo[netId].l_h[k]);
			    }

			}
			grad_wire[ 2*i ] += (2*grad)/(m_pDB->m_nets[netId].size()-1);

		    }
		    else
		    {
			double grad=( (x[2*m_pDB->m_pins[selfPinId].moduleId]-
				    x[2*m_pDB->m_pins[m_qp_netInfo[netId].m_leftPinId].moduleId]
				    )/m_qp_netInfo[netId].l_h[m_qp_netInfo[netId].m_pinId2index[selfPinId]] 
				+
				( (x[2*m_pDB->m_pins[selfPinId].moduleId])-
				  (x[2*m_pDB->m_pins[m_qp_netInfo[netId].m_rightPinId].moduleId]) )/
				(m_qp_netInfo[netId].width-m_qp_netInfo[netId].l_h[m_qp_netInfo[netId].m_pinId2index[selfPinId]]));


			grad_wire[ 2*i ] += (2*grad)/(m_pDB->m_nets[netId].size()-1);

		    }

		    if(m_qp_netInfo[netId].m_bottomPinId==selfPinId)
		    {
			double grad=0;
			for(int k=0; k<(int)m_pDB->m_nets[netId].size(); ++k)
			{
			    int pid=m_pDB->m_nets[netId][k];

			    if(pid!=selfPinId)
			    {
				grad+=( (x[2*m_pDB->m_pins[selfPinId].moduleId+1])-
					(x[2*m_pDB->m_pins[pid].moduleId+1]) )/m_qp_netInfo[netId].l_v[k];
			    }

			}
			grad_wire[ 2*i+1 ] += (2*grad)/(m_pDB->m_nets[netId].size()-1);

		    }
		    else if(m_qp_netInfo[netId].m_topPinId==selfPinId)
		    {
			double grad=0;
			for(int k=0; k<(int)m_pDB->m_nets[netId].size(); ++k)
			{
			    int pid=m_pDB->m_nets[netId][k];

			    if(pid!=selfPinId)
			    {
				grad+=( (x[2*m_pDB->m_pins[selfPinId].moduleId+1])-
					(x[2*m_pDB->m_pins[pid].moduleId+1]) )/(m_qp_netInfo[netId].height-m_qp_netInfo[netId].l_v[k]);
			    }

			}
			grad_wire[ 2*i+1 ] += (2*grad)/(m_pDB->m_nets[netId].size()-1);

		    }
		    else
		    {
			double grad=( (x[2*m_pDB->m_pins[selfPinId].moduleId+1])-
				(x[2*m_pDB->m_pins[m_qp_netInfo[netId].m_bottomPinId].moduleId+1]) )/m_qp_netInfo[netId].l_v[m_qp_netInfo[netId].m_pinId2index[selfPinId]] 
			    +
			    ( (x[2*m_pDB->m_pins[selfPinId].moduleId+1])-
			      (x[2*m_pDB->m_pins[m_qp_netInfo[netId].m_topPinId].moduleId+1]) )/
			    (m_qp_netInfo[netId].height-m_qp_netInfo[netId].l_v[m_qp_netInfo[netId].m_pinId2index[selfPinId]]);


			grad_wire[ 2*i+1 ] += (2*grad)/(m_pDB->m_nets[netId].size()-1);

		    }
		}

	    } // for each pin in the module
    } // for each module
    qtime_grad_lse += seconds() - time_used;
   
    // grad Density
    double time_start_2 = seconds();

    double gradDensityX;
    double gradDensityY;
    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )	    // for each cell
    {
	    if( m_pDB->m_modules[i].m_isFixed || cellLock[i] == true )
	        continue;
    	
	    GetPotentialGrad( x, i, gradDensityX, gradDensityY );	    // bell-shaped potential
	    grad_potential[2*i]   = /*2 **/ gradDensityX;
	    grad_potential[2*i+1] = /*2 **/ gradDensityY;

    } // for each cell

    //if(this->m_isFixedPinRepel==true)
    //{
    //    this->addFixedPinForce(grad_potential);
    //    for(int i=0; i<(int)this->m_macroIds.size(); ++i)
    //    {
    //        grad_wire[2*m_macroIds[i]]=0;
    //        grad_wire[2*m_macroIds[i]+1]=0;

    //    }
    //}

    qtime_grad_potential += seconds() - time_start_2;
    
    // compute total fouce
    for( int i =0; i<n; i++ )
    	grad_f[i] = _weightDensity * grad_potential[i] + grad_wire[i] * _weightWire;
    //if(this->m_isFixedPinRepel==true)
    //{
    //    this->addFixedPinForce(grad_f);
    //}
    
    qtime_grad_f += seconds()-time_used;
    return true;
}


void MyQP::GetPotentialGrad( const vector<double>& x, const int& i, double& gradX, double& gradY )
{
    double cellX = x[i*2];
    double cellY = x[i*2+1];

    double width  = m_pDB->m_modules[i].m_width;
    double height = m_pDB->m_modules[i].m_height;
    double left   = cellX - width  * 0.5 - _potentialRX;
    double bottom = cellY - height * 0.5 - _potentialRY;
    double right  = cellX + ( cellX - left );
    double top    = cellY + ( cellY - bottom );
    if( left   < m_pDB->m_coreRgn.left )	left   = m_pDB->m_coreRgn.left;
    if( bottom < m_pDB->m_coreRgn.bottom )	bottom = m_pDB->m_coreRgn.bottom;
    if( right  > m_pDB->m_coreRgn.right )	right  = m_pDB->m_coreRgn.right;
    if( top    > m_pDB->m_coreRgn.top )	top    = m_pDB->m_coreRgn.top;
    int gx, gy;
    GetClosestGrid( left, bottom, gx, gy );

    if( gx < 0 )	gx = 0;
    if( gy < 0 )	gy = 0;

    int gxx, gyy;
    double xx, yy;
    gradX = 0.0;	
    gradY = 0.0;

    //// TEST (std-cell)
    if( height < m_potentialGridHeight && width < m_potentialGridWidth )
	width = height = 0;

    for( gxx = gx, xx = GetXGrid( gx ); 
	    xx <= right && gx < (int)m_gridPotential.size(); 
	    gxx++, xx += m_potentialGridWidth )
    {

	for( gyy = gy, yy = GetYGrid( gy ); 
		yy <= top && gy < (int)m_gridPotential.size() ; 
		gyy++, yy += m_potentialGridHeight )
	{

	    double gX = 0;
	    double gY = 0;
	    // TEST
	    //if( m_gridPotential[ gxx ][ gyy ] > m_expBinPotential[gxx][gyy] )  // TEST for ispd05
	    {
		gX = ( m_gridPotential[gxx][gyy] - m_expBinPotential[gxx][gyy] ) *
		    _cellPotentialNorm[i] *
		    GetGradPotential( cellX, xx, _potentialRX, width ) *
		    GetPotential(     cellY, yy, _potentialRY, height );
		gY =  ( m_gridPotential[gxx][gyy] - m_expBinPotential[gxx][gyy] ) *
		    _cellPotentialNorm[i] *
		    GetPotential(     cellX, xx, _potentialRX, width  ) *
		    GetGradPotential( cellY, yy, _potentialRY, height );
	    }

	    gradX += gX;
	    gradY += gY;
	}
    } // for each grid
}





void MyQP::UpdateBlockPosition( const vector<double>& x )
{
   for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
   {
       if( m_pDB->m_modules[i].m_isFixed == false )
       {
	    m_pDB->MoveModuleCenter( i, x[i*2], x[i*2+1] ); 
       }
   }
}

void MyQP::CreatePotentialGrid()
{
    //printf( "Create Potential Grid\n" );
    m_gridPotential.clear(); // remove old values
    
    int realGridSize = m_potentialGridSize;
   
    m_gridPotential.resize( realGridSize );
    m_basePotential.resize( realGridSize );
    for( unsigned int i=0; i<m_gridPotential.size(); i++ )
    {
	    m_basePotential[i].resize( realGridSize, 0 );
	    m_gridPotential[i].resize( realGridSize, 0 );
    }
    
    m_potentialGridWidth  = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / m_potentialGridSize;
    m_potentialGridHeight = ( m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom ) / m_potentialGridSize;
    _potentialRX = m_potentialGridWidth  * _potentialGridR;
    _potentialRY = m_potentialGridHeight * _potentialGridR;

}


void MyQP::ClearPotentialGrid()
{
    for( int gx=0; gx<(int)m_gridPotential.size(); gx++ )
	    fill( m_gridPotential[gx].begin(), m_gridPotential[gx].end(), 0.0 );
}

void MyQP::UpdateExpBinPotential( double util )
{
    double binArea = m_potentialGridWidth * m_potentialGridHeight;

    if( util < 0 )
	util = 1.0; // use all space

    double totalFree = 0;
    int zeroSpaceBin = 0;
    m_expBinPotential.resize( m_basePotential.size() );
    for( unsigned int i=0; i<m_basePotential.size(); i++ )
    {
	    m_expBinPotential[i].resize( m_basePotential[i].size() );
	    for( unsigned int j=0; j<m_basePotential[i].size(); j++ )
	    {
	        double base = m_basePotential[i][j];
	        double free = binArea - base;
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

    if( param.bShow )
    {
        printf( "MyQP!!\n");
	    printf( "PBIN: Expect bin potential utilization: %f\n", util );
	    printf( "PBIN: Zero space bin # = %d\n", zeroSpaceBin );
	    printf( "PBIN: Total free potential = %.0f (%.5f)\n", totalFree, m_pDB->m_totalMovableModuleArea / totalFree );
    }

    // TODO: scaling?
    //assert( m_pDB->m_totalMovableModuleArea / totalFree <= 1.000001 );
    double alwaysOver = 0.0;
    if( m_targetUtil > 0.0 && m_targetUtil < 1.0 )
    {
	    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
	    {
	        if( m_pDB->m_modules[i].m_isFixed )
		    continue;
	        if( m_pDB->m_modules[i].m_width >= 2 * m_potentialGridWidth && 
		        m_pDB->m_modules[i].m_height >= 2 * m_potentialGridHeight )
	        {
		    alwaysOver += 
		        (m_pDB->m_modules[i].m_width - m_potentialGridWidth ) * 
		        (m_pDB->m_modules[i].m_height - m_potentialGridHeight ) * 
		        (1.0 - m_targetUtil );
	        }
	    }
	    if( param.bShow )
	        printf( "PBIN: Always over: %.0f (%.1f%%)\n", alwaysOver, alwaysOver/m_pDB->m_totalMovableModuleArea*100.0 );
    }
    m_alwaysOverPotential = alwaysOver;
}

void MyQP::SmoothPotentialBase( const double& delta )
{
    
    // find the max potential (TODO: comnpute one time is enough)
    double maxPotential = 0;
    double avgPotential = 0;
    double totalPotential = 0;
    for( unsigned int i=0; i<m_basePotentialOri.size(); i++ )
	for( unsigned int j=0; j<m_basePotentialOri[i].size(); j++ )
	{
	    totalPotential += m_basePotentialOri[i][j];
	    if( m_basePotentialOri[i][j] > maxPotential )
		maxPotential = m_basePotentialOri[i][j];
	}
    avgPotential = totalPotential / (m_basePotentialOri.size() * m_basePotentialOri.size() );

    if( totalPotential == 0 )
	return; // no preplaced
    
    // apply TSP-style smoothing
    double newTotalPotential = 0;
    for( unsigned int i=0; i<m_basePotential.size(); i++ )
	for( unsigned int j=0; j<m_basePotential[i].size(); j++ )
	{
	    if( m_basePotentialOri[i][j] >= avgPotential )
	    {
		m_basePotential[i][j] = 
		    avgPotential + 
		    pow( ( m_basePotentialOri[i][j] - avgPotential ) / maxPotential, delta ) * maxPotential;
	    }
	    else
	    {
		m_basePotential[i][j] = 
		    avgPotential - 
		    pow( ( avgPotential - m_basePotentialOri[i][j] ) / maxPotential, delta ) * maxPotential;
	    }
	    newTotalPotential += m_basePotential[i][j];
	}
    
    // normalization
    double ratio = totalPotential / newTotalPotential;
    for( unsigned int i=0; i<m_basePotential.size(); i++ )
	for( unsigned int j=0; j<m_basePotential[i].size(); j++ )
	    m_basePotential[i][j] = m_basePotential[i][j] * ratio;

    //printf( "Smooth %.0f (%.0f->%.0f)\n", delta, totalPotential, newTotalPotential );
}

void MyQP::UpdatePotentialGridBase( const vector<double>& x )
{
    double time_start = seconds();

    double binArea = m_potentialGridWidth * m_potentialGridHeight;
    m_binFreeSpace.resize( m_basePotential.size() );
    for( unsigned int i=0; i<m_basePotential.size(); i++ )
    {
	fill( m_basePotential[i].begin(), m_basePotential[i].end(), 0.0 );
	m_binFreeSpace[i].resize( m_basePotential[i].size() );
	fill( m_binFreeSpace[i].begin(), m_binFreeSpace[i].end(), binArea );
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
	
	int gx, gy;
	double cellX = x[i*2];
	double cellY = x[i*2+1];
	double width  = m_pDB->m_modules[i].m_width;
	double height = m_pDB->m_modules[i].m_height;

	double potentialRX = _potentialRX;
	double potentialRY = _potentialRY;
	//double left   = cellX - width * 0.5  - potentialRX;
	//double bottom = cellY - height * 0.5 - potentialRY;
	double left   = cellX - width * 0.5;  // for gaussian smoothing
	double bottom = cellY - height * 0.5; // for gaussian smoothing
	double right  = cellX + (cellX - left);
	double top    = cellY + (cellY - bottom);
	if( left   < m_pDB->m_coreRgn.left )     left   = m_pDB->m_coreRgn.left;
	if( bottom < m_pDB->m_coreRgn.bottom )   bottom = m_pDB->m_coreRgn.bottom;
	if( top    > m_pDB->m_coreRgn.top )      top    = m_pDB->m_coreRgn.top;
	if( right  > m_pDB->m_coreRgn.right )    right  = m_pDB->m_coreRgn.right;
	GetClosestGrid( left, bottom, gx, gy );
	if( gx < 0 )  gx = 0;
	if( gy < 0 )  gy = 0;
      
	double totalPotential = 0;
	vector< potentialStruct > potentialList;      
	int gxx, gyy;
	double xx, yy;

	//if( m_useBellPotentialForPreplaced == false )
	{
	    // "Exact density for the potential"
	    for( gxx = gx, xx = GetXGrid(gx); xx<=right ; gxx++, xx+=m_potentialGridWidth )
	    {
		    for( gyy = gy, yy = GetYGrid(gy); yy<=top ; gyy++, yy+=m_potentialGridHeight )
		    {
		        m_basePotential[gxx][gyy] +=
		    	    getOverlap( left, right, xx, xx+m_potentialGridWidth ) * 
		    	    getOverlap( bottom, top, yy, yy+m_potentialGridHeight );

		        m_binFreeSpace[gxx][gyy] -= 
		    	    getOverlap( left, right, xx, xx+m_potentialGridWidth ) * 
		    	    getOverlap( bottom, top, yy, yy+m_potentialGridHeight );
		    }
	    }
	    continue;
	}
	
	for( gxx = gx, xx = GetXGrid(gx); xx<=right ; gxx++, xx+=m_potentialGridWidth )
	{
	    for( gyy = gy, yy = GetYGrid(gy); yy<=top ; gyy++, yy+=m_potentialGridHeight )
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

	// normalize the potential so that total potential equals the cell area
	double scale = m_pDB->m_modules[i].m_area / totalPotential;
	//printf( "totalPotential = %f\n", totalPotential );

	_cellPotentialNorm[i] = scale;	    // normalization factor for the cell i

	vector< potentialStruct >::const_iterator ite;
	for( ite=potentialList.begin(); ite!=potentialList.end(); ++ite )
	{
	    if(	ite->gx < 0 || ite->gx >= (int)m_gridPotential.size() ||
		ite->gy < 0 || ite->gy >= (int)m_gridPotential[ite->gx].size() )
		continue; // bin may be outside when core-shrinking is applied
	    else
		m_basePotential[ ite->gx ][ ite->gy ] += ite->potential * scale;	    
	}

	
    } // for each cell
    qtime_up_potential += seconds() - time_start;

    m_basePotentialOri = m_basePotential;   // make a copy for TSP-style smoothing
}


void MyQP::UpdatePotentialGrid( const vector<double>& x )
{
    double time_start = seconds();
    ClearPotentialGrid();
    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
    {
	// for each cell. cell ci coordinate is ( x[i*2], x[i*2+1] )

	//if( m_pDB->m_modules[i].m_isOutCore )
	if( m_pDB->BlockOutCore( i ) )
	    continue;

	// preplaced blocks are stored in m_basePotential
	if( m_pDB->m_modules[i].m_isFixed )
	    continue;
	
	int gx, gy;
	double cellX = x[i*2];
	double cellY = x[i*2+1];
	double potentialRX = _potentialRX;
	double potentialRY = _potentialRY;
	double width  = m_pDB->m_modules[i].m_width;
	double height = m_pDB->m_modules[i].m_height;
	double left   = cellX - width * 0.5  - potentialRX;
	double bottom = cellY - height * 0.5 - potentialRY;
	double right  = cellX + (cellX - left);
	double top    = cellY + (cellY - bottom);
	if( left   < m_pDB->m_coreRgn.left )     left   = m_pDB->m_coreRgn.left;
	if( bottom < m_pDB->m_coreRgn.bottom )   bottom = m_pDB->m_coreRgn.bottom;
	if( top    > m_pDB->m_coreRgn.top )      top    = m_pDB->m_coreRgn.top;
	if( right  > m_pDB->m_coreRgn.right )    right  = m_pDB->m_coreRgn.right;
	GetClosestGrid( left, bottom, gx, gy );
       	
	double totalPotential = 0;
	vector< potentialStruct > potentialList;      
	int gxx, gyy;
	double xx, yy;

	//// TEST (convert to std-cell)
	if( height < m_potentialGridHeight && width < m_potentialGridWidth )
	    width = height = 0;

	for( gxx = gx, xx = GetXGrid(gx); xx<=right ; gxx++, xx+=m_potentialGridWidth )
	{
	    for( gyy = gy, yy = GetYGrid(gy); yy<=top ; gyy++, yy+=m_potentialGridHeight )
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

	// normalize the potential so that total potential equals the cell area
	double scale = m_pDB->m_modules[i].m_area / totalPotential;
	//printf( "totalPotential = %f\n", totalPotential );

	_cellPotentialNorm[i] = scale;	    // normalization factor for the cell i
	vector< potentialStruct >::const_iterator ite;
	for( ite=potentialList.begin(); ite!=potentialList.end(); ++ite )
	{
	    m_gridPotential[ ite->gx ][ ite->gy ] += ite->potential * scale;	    
	}
	
    } // for each cell
    qtime_up_potential += seconds() - time_start;

}



double MyQP::GetPotential( const double& x1, const double& x2, const double& r, const double& w )
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


double MyQP::GetGradPotential( const double& x1, const double& x2, const double& r, const double& w )
{
    //double w = 0;
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


double MyQP::GetXGrid( const int& gx )
{
    return m_pDB->m_coreRgn.left + gx * m_potentialGridWidth + 0.5 * m_potentialGridWidth;
}

double MyQP::GetYGrid( const int& gy )
{
    return  m_pDB->m_coreRgn.bottom + gy * m_potentialGridHeight + 0.5 * m_potentialGridHeight;
}



void MyQP::GetClosestGrid( const double& x1, const double& y1, int& gx, int& gy ) 
{
    gx = static_cast<int>( floor( ( x1 - m_pDB->m_coreRgn.left ) / m_potentialGridWidth ) );
    gy = static_cast<int>( floor( ( y1 - m_pDB->m_coreRgn.bottom ) / m_potentialGridHeight ) );
}

void MyQP::ClearDensityGrid()
{
    for( unsigned int i=0; i<m_gridDensity.size(); i++ )
	for( unsigned int j=0; j<m_gridDensity[i].size(); j++ )
	    m_gridDensity[i][j] = 0.0;
}


void MyQP::UpdateDensityGridSpace( const int& n, const vector<double>& x )
{
    double allSpace = m_gridDensityWidth * m_gridDensityHeight;
    for( unsigned int i=0; i<m_gridDensity.size(); i++ )
	for( unsigned int j=0; j<m_gridDensity[i].size(); j++ )
	    m_gridDensitySpace[i][j] = allSpace;
   
    for( int b=0; b<(int)m_pDB->m_modules.size(); b++ )
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
	
	for( int xOff = gx; xOff < (int)m_gridDensity.size(); xOff++ )
	{
	    double binLeft  = m_pDB->m_coreRgn.left + xOff * m_gridDensityWidth;
	    double binRight = binLeft + m_gridDensityWidth;
	    if( binLeft >= right )
		break;
	    
	    for( int yOff = gy; yOff < (int)m_gridDensity[xOff].size(); yOff ++ )
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
    for( unsigned int i=0; i<m_gridDensity.size(); i++ )
	for( unsigned int j=0; j<m_gridDensity[i].size(); j++ )
	{
	    if( m_gridDensitySpace[i][j] < 1e-5 )
	    {
		    m_gridDensitySpace[i][j] = 0.0;
		    zeroSpaceCount ++;
	    }
	    m_totalFreeSpace += m_gridDensitySpace[i][j];
	}
    if( param.bShow )
	printf( "DBIN: zero space bins: %d\n", zeroSpaceCount );
}


void MyQP::UpdateDensityGrid( const int& n, const vector<double>& x )
{
    ClearDensityGrid();
    
    // for each cell b, update the corresponding bin area
    for( int b=0; b<(int)m_pDB->m_modules.size(); b++ )
    {
	    //if(  m_pDB->m_modules[b].m_isOutCore || m_pDB->m_modules[b].m_isFixed )
	    if(  m_pDB->BlockOutCore( b ) || m_pDB->m_modules[b].m_isFixed )
	        continue;

	    double w  = m_pDB->m_modules[b].m_width;
	    double h  = m_pDB->m_modules[b].m_height;

	    // bottom-left 
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
	    //double debug_area = 0;
	    for( int xOff = gx; xOff < (int)m_gridDensity.size(); xOff++ )
	    {
	        double binLeft = m_pDB->m_coreRgn.left + m_gridDensityWidth * xOff;
	        double binRight = binLeft + m_gridDensityWidth;
	        if( binLeft >= right )
		    break;
    	    
	        for( int yOff = gy; yOff < (int)m_gridDensity[xOff].size(); yOff++ )
	        {
		        double binBottom = m_pDB->m_coreRgn.bottom + m_gridDensityHeight * yOff;
		        double binTop    = binBottom + m_gridDensityHeight;
		        if( binBottom >= top )
		            break;

		        double area = 
		            getOverlap( left, right, binLeft, binRight ) *
		            getOverlap( bottom, top, binBottom, binTop );
        		
		        m_gridDensity[xOff][yOff] += area;
		        //debug_area += area;
	        }
	    }

	    // TODO: check precision
	    //printf( " module %d %f %f\n", b, m_pDB->m_modules[b].m_area, debug_area );
	
    } // each module

}

void MyQP::CheckDensityGrid()
{
    double totalDensity = 0;
    for( int i=0; i<(int)m_gridDensity.size(); i++ )
	    for( int j=0; j<(int)m_gridDensity[i].size(); j++ )
	        totalDensity += m_gridDensity[i][j];

    double totalArea = 0;
    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
    {
	    //if( m_pDB->m_modules[i].m_isOutCore == false )
	    if( m_pDB->BlockOutCore( i ) == false )
	        totalArea += m_pDB->m_modules[i].m_area;
    }

    printf( " %f %f\n", totalDensity, totalArea );
}

void MyQP::CreateDensityGrid( int nGrid )
{
    m_gridDensity.resize( nGrid );
    for( int i=0; i<nGrid; i++ )
	m_gridDensity[i].resize( nGrid );
    
    m_gridDensitySpace.resize( nGrid );
    for( int i=0; i<nGrid; i++ )
	m_gridDensitySpace[i].resize( nGrid );
    
    m_gridDensityWidth  = ( (double)m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / nGrid;
    m_gridDensityHeight = ( (double)m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom ) / nGrid;
    m_gridDensityTarget = m_pDB->m_totalModuleArea / ( nGrid * nGrid );

    // 2006-03-21 compute always overflow area
    
    double alwaysOver = 0.0;
    if( m_targetUtil > 0.0 && m_targetUtil < 1.0 )
    {
	for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
	{
	    if( m_pDB->m_modules[i].m_isFixed )
		continue;
	    if( m_pDB->m_modules[i].m_width >= 2*m_gridDensityWidth && m_pDB->m_modules[i].m_height >= 2*m_gridDensityHeight )
		alwaysOver += 
		    (m_pDB->m_modules[i].m_width - m_gridDensityWidth ) * 
		    (m_pDB->m_modules[i].m_height - m_gridDensityHeight ) * 
		    (1.0 - m_targetUtil );
	}
	if( param.bShow )
	    printf( "DBIN: Always over: %.0f (%.1f%%)\n", alwaysOver, alwaysOver/m_pDB->m_totalMovableModuleArea*100.0 );
    }
    m_alwaysOverArea = alwaysOver;
}




double MyQP::GetMaxDensity()
{
    double maxUtilization = 0;
    double binArea = m_gridDensityWidth * m_gridDensityHeight;
    for( int i=0; i<(int)m_gridDensity.size(); i++ )
	for( int j=0; j<(int)m_gridDensity[i].size(); j++ )
	{
	    if( m_gridDensitySpace[i][j] > 1e-5 )
	    {
		//double utilization = m_gridDensity[i][j] / m_gridDensitySpace[i][j];   

		double preplacedArea = binArea - m_gridDensitySpace[i][j];
		double utilization = ( m_gridDensity[i][j] + preplacedArea ) / binArea;   

		if( utilization > maxUtilization )
		    maxUtilization = utilization;
	    }
	}
    return maxUtilization;
}



double MyQP::GetTotalOverDensityLB()
{
    double over = 0;
    for( unsigned int i=0; i<m_gridDensity.size(); i++ )
	for( unsigned int j=0; j<m_gridDensity.size(); j++ )
	{
	    double targetSpace = m_gridDensitySpace[i][j] * m_targetUtil;
	    if( targetSpace > 1e-5 && m_gridDensity[i][j]  > targetSpace  )
    		over += m_gridDensity[i][j] - targetSpace;
	}

    // TODO: remove "1.0"
    return (over -m_alwaysOverArea) / (m_pDB->m_totalMovableModuleArea) + 1.0; 
}


double MyQP::GetTotalOverDensity()
{
    double over = 0;
    for( unsigned int i=0; i<m_gridDensity.size(); i++ )
	for( unsigned int j=0; j<m_gridDensity.size(); j++ )
	{
	    double targetSpace = m_gridDensitySpace[i][j] * m_targetUtil;
	    if( m_gridDensity[i][j]  > targetSpace  )
    		over += m_gridDensity[i][j] - targetSpace;
	}

    // TODO: remove "1.0"
    return ( over - m_alwaysOverArea) / (m_pDB->m_totalMovableModuleArea) + 1.0; 
}


double MyQP::GetTotalOverPotential()
{
    double over = 0;
    for( unsigned int i=0; i<m_gridPotential.size(); i++ )
	for( unsigned int j=0; j<m_gridPotential[i].size(); j++ )
	{
	    if( m_gridPotential[i][j]  > m_expBinPotential[i][j]  )
    		over += m_gridPotential[i][j] - m_expBinPotential[i][j];
	}

    // TODO: remove "1.0"
    return (over - m_alwaysOverPotential) / (m_pDB->m_totalMovableModuleArea) + 1.0; 
}

  
double MyQP::GetNonZeroDensityGridPercent()
{
    double nonZero = 0;
    for( int i=0; i<(int)m_gridDensity.size(); i++ )
	for( int j=0; j<(int)m_gridDensity.size(); j++ )
	{
	    if( m_gridDensity[i][j] > 0 || 
		m_gridDensitySpace[i][j] == 0 
		//|| m_gridDensitySpace[i][j] < m_potentialGridWidth * m_potentialGridHeight 
		)
		nonZero += 1.0;
	}
    return nonZero / m_gridDensity.size() / m_gridDensity.size();
}


double MyQP::GetNonZeroGridPercent()
{
    double nonZero = 0;
    for( int i=0; i<(int)m_gridPotential.size(); i++ )
	    for( int j=0; j<(int)m_gridPotential.size(); j++ )
	        if( m_gridPotential[i][j] > 0 )
		        nonZero += 1.0;
    return nonZero / m_gridPotential.size() / m_gridPotential.size();
}


double MyQP::GetMaxPotential()
{
    double maxDensity = 0;

    for( unsigned int i=0; i<m_gridPotential.size(); i++ )
	for( unsigned int j=0; j<m_gridPotential.size(); j++ )
	    if( m_gridPotential[i][j] > maxDensity )   
		maxDensity = m_gridPotential[i][j];
    return maxDensity;
}


double MyQP::GetAvgPotential()
{
    const double targetDensity = 1.0;
    double avgDensity = 0;
    int overflowCount = 0;
    
    for( unsigned int i=0; i<m_gridPotential.size(); i++ )
	for( unsigned int j=0; j<m_gridPotential.size(); j++ )
	    if( m_gridPotential[i][j] > targetDensity )
	    {
		overflowCount++;
    		avgDensity += m_gridPotential[i][j];
	    }
    return avgDensity / overflowCount;
}



// Output potential data for gnuplot
void MyQP::OutputPotentialGrid( string filename )
{
    int stepSize = (int)m_gridPotential.size() / 100;
    if( stepSize == 0 )
	stepSize = 1;
    FILE* out = fopen( filename.c_str(), "w" );
    double binArea = m_potentialGridWidth * m_potentialGridHeight;
    for( unsigned int j=0; j<m_gridPotential.size(); j+=stepSize )
    {
	    for( unsigned int i=0; i<m_gridPotential.size(); i+=stepSize )
	        fprintf( out, "%.03f ", (m_gridPotential[i][j] + m_basePotential[i][j]) / binArea );
	    fprintf( out, "\n" );
    }
    fprintf( out, "\n" );
    fclose( out );
}

// Output potential data for gnuplot
//void MyQP::OutputFixedPinPotentialGrid( string filename )
//{
//    if(this->m_isFixedPinRepel==false)
//        return;
//    int stepSize = (int)m_FixedPinPotential.size() / 100;
//    if( stepSize == 0 )
//	stepSize = 1;
//    FILE* out = fopen( filename.c_str(), "w" );
//
//    for( unsigned int j=0; j<m_FixedPinPotential.size(); j+=stepSize )
//    {
//	    for( unsigned int i=0; i<m_FixedPinPotential.size(); i+=stepSize )
//	        fprintf( out, "%.03f ", m_FixedPinPotential[i][j]  );
//	    fprintf( out, "\n" );
//    }
//    fprintf( out, "\n" );
//    fclose( out );
//}

// Output potential data for gnuplot
void MyQP::OutputDensityGrid( string filename )
{
    int stepSize = 1;
    FILE* out = fopen( filename.c_str(), "w" );
    for( unsigned int j=0; j<m_gridDensity.size(); j+=stepSize )
    {
	    for( unsigned int i=0; i<m_gridDensity.size(); i+=stepSize )
	    {
	        double targetSpace = m_gridDensitySpace[i][j] * m_targetUtil;
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

void MyQP::findGridNum(  const double x, const double y, int& xIndex, int& yIndex)
{


    xIndex=(int)floor( (x-m_pDB->m_coreRgn.left)/m_gridW );
    yIndex=(int)floor( (y-m_pDB->m_coreRgn.bottom)/m_gridH );
    if(xIndex<0)
        xIndex=0;
    if(xIndex>=this->m_num_X_grid)
        xIndex=this->m_num_X_grid-1;
    if(yIndex<0)
        yIndex=0;
    if(yIndex>=this->m_num_Y_grid)
        yIndex=this->m_num_Y_grid-1;

}
void MyQP::buildPinGrid( int x, int y, vector< vector<double> >& fixPinGrid)
{

    this->m_num_X_grid=x;
    this->m_num_Y_grid=y;
    this->m_gridW= (m_pDB->m_coreRgn.right-m_pDB->m_coreRgn.left)/this->m_num_X_grid;
    this->m_gridH= (m_pDB->m_coreRgn.top-m_pDB->m_coreRgn.bottom)/this->m_num_Y_grid;

    vector< vector<double> > temp_grid;

    //initialize grid
    temp_grid.resize(this->m_num_X_grid);
    for( int i=0; i<(int)temp_grid.size(); ++i)
    {
        temp_grid[i].resize(this->m_num_Y_grid, 0 );
        //for(int j=0; j<(int)temp_grid[i].size(); ++j )
        //{
        //    temp_grid[i][j].m_cx=(i+0.5)*m_gridW + db.m_coreRgn.left; 
        //    temp_grid[i][j].m_cy=(j+0.5)*m_gridH + db.m_coreRgn.bottom;
        //}
    }

    //for all fixed module, insert fixed pins to the grid
    for(int i=0; i<(int)m_pDB->m_modules.size(); ++i)
    {
        if(m_pDB->m_modules[i].m_isFixed==true)
        {
            for( int j=0; j<(int)m_pDB->m_modules[i].m_pinsId.size(); ++j)
            {
                double px=m_pDB->m_pins[m_pDB->m_modules[i].m_pinsId[j]].absX;
                double py=m_pDB->m_pins[m_pDB->m_modules[i].m_pinsId[j]].absY;
                int xindex, yindex;
                findGridNum( px, py, xindex,  yindex);
                ++temp_grid[xindex][yindex];

            }
        }
    }
    fixPinGrid=temp_grid;

}
//double MyQP::getFixedPinPotential()
//{
//    double value=0;
//    for(int i=0; i<(int)this->m_macroIds.size(); ++i)
//    {
//        int xindex, yindex;
//        double px=m_pDB->m_modules[m_macroIds[i]].m_cx;
//        double py=m_pDB->m_modules[m_macroIds[i]].m_cy;
//        findGridNum( px, py, xindex,  yindex);
//        value+=this->m_FixedPinPotential[xindex][yindex];
//    }
//    return value;
//}

//void MyQP::addFixedPinForce( vector<double>& grad_potential)
//{
//    double totalGrad_potential=0;
//    double totalPF_potential=0;  // PF== Pin Force
//    double controlRatio=param.FPF_controlValue; //control the railo of pin force and density force (controlRatio=PF/GF)
//
//    vector< double > vec_xOff;
//    vector< double > vec_yOff;
//    vec_xOff.resize(m_macroIds.size());
//    vec_yOff.resize(m_macroIds.size());
//
//    //for(int i=0; i< (int)(grad_potential.size()/2); ++i)
//    //{
//    //    totalGrad_potential+=sqrt( grad_potential[i]*grad_potential[i] + 
//    //                               grad_potential[2*i+1]*grad_potential[2*i+1] );
//    //}
//    for(int i=0; i<(int)this->m_macroIds.size(); ++i)
//    {
//        double currentValue;
//        int xindex, yindex;
//        double px=m_pDB->m_modules[m_macroIds[i]].m_cx;
//        double py=m_pDB->m_modules[m_macroIds[i]].m_cy;
//        findGridNum( px, py, xindex,  yindex);
//        currentValue=this->m_FixedPinPotential[xindex][yindex];
//        if(xindex<(this->m_num_X_grid-1))
//        {
//            vec_xOff[i]=currentValue-m_FixedPinPotential[xindex+1][yindex];
//        }
//        else
//        {
//            vec_xOff[i]=m_FixedPinPotential[xindex-1][yindex]-currentValue;
//        }
//        if(yindex<(this->m_num_Y_grid-1))
//        {
//            vec_yOff[i]=currentValue-m_FixedPinPotential[xindex][yindex+1];
//        }
//        else
//        {
//            vec_yOff[i]=m_FixedPinPotential[xindex][yindex-1]-currentValue;
//        }
//        vec_xOff[i]*=m_macroIdsAreaFactor[i];
//        vec_yOff[i]*=m_macroIdsAreaFactor[i];
//        totalGrad_potential+=sqrt( grad_potential[2*m_macroIds[i]]*grad_potential[2*m_macroIds[i]] + 
//                                   grad_potential[2*m_macroIds[i]+1]*grad_potential[2*m_macroIds[i]+1] );
//        totalPF_potential+=sqrt( vec_xOff[i]*vec_xOff[i] + vec_yOff[i]*vec_yOff[i] );
//    }
//
//    double ratio=0;
//    if(totalPF_potential>1)
//    {
//        ratio=controlRatio*(totalGrad_potential/totalPF_potential);
//        //if(ratio>1000000)
//        //    ratio=1000000;
//    } 
//    //the '10' and '100' here are dirty value!!
//    //ratio=1;
//
//    for(int i=0; i<(int)this->m_macroIds.size(); ++i)
//    {
//        grad_potential[2*m_macroIds[i]]-=ratio*vec_xOff[i];
//        grad_potential[2*m_macroIds[i]+1]-=ratio*vec_yOff[i];
//    }
//}

//build Rectilinear Minimum Spanning Tree by a O(n^2) time implementation of Prim's algorithm
void MyGlobalRouter::buildMST( const int& netID, vector<int>& parentPinIds )
{
    int netSize=m_pDB->m_nets[netID].size();
    int rootId=0;
    parentPinIds.resize(netSize, rootId);

    //don't consider 0-pin and 1-pin net
    if(netSize<=1)
        return;

    vector<double> dist2tree;
    dist2tree.resize(netSize,0);


    double maxD=0;

    //init
    for(int i=0; i<netSize; ++i)
    {
        parentPinIds[i]=rootId;
        dist2tree[i]=this->dist( m_pDB->m_nets[netID][i], m_pDB->m_nets[netID][rootId] );
        if(dist2tree[i] > maxD)
        {
            maxD = dist2tree[i];
        }

    }

    //add all points
    for(int i=1; i<netSize; ++i)
    {
        double shortestD=maxD+1;
        int shortestId = -1;

        for(int j=0; j<netSize; ++j)
        {
            if( (dist2tree[j]!= 0 ) && (dist2tree[j] < shortestD) )
            {
                shortestId = j;
                shortestD  = dist2tree[j];
            }
        }

        //add pin with shortestId into the tree
        dist2tree[shortestId]=0;

        //update the other nodes
        for(int j=0; j<netSize; ++j)
        {
            if(dist2tree[j]!= 0 )
            {
                double newdist=this->dist( m_pDB->m_nets[netID][j], m_pDB->m_nets[netID][shortestId]);
                if( newdist< dist2tree[j])
                {
                    dist2tree[j]=newdist;
                    parentPinIds[j]=shortestId;
                }
            }
        }
    }

}
double MyGlobalRouter::dist(const int& pinId1, const int& pinId2)
{
    if(pinId1==pinId2)
        return 0;
    else
        return fabs(m_pDB->m_pins[pinId1].absX-m_pDB->m_pins[pinId2].absX) +  fabs(m_pDB->m_pins[pinId1].absY-m_pDB->m_pins[pinId2].absY);
}
void MyGlobalRouter::initResource()
{
    m_routerBin.resize(this->m_nBinCol);
    for(int i=0; i<this->m_nBinCol; ++i)
    {
        this->m_routerBin[i].resize(this->m_nBinRow);
    }
    for(int j=0; j<this->m_nBinCol; ++j)
    {
        for(int k=0; k<this->m_nBinRow; ++k)
        {
            double binX=j*m_binWidth + m_pDB->m_coreRgn.left;
            double binY=k*m_binHeight + m_pDB->m_coreRgn.bottom;

            double binXright = binX + m_binWidth;
            double binYtop = binY + m_binHeight;
			if( binXright > m_pDB->m_coreRgn.right )
				binXright = m_pDB->m_coreRgn.right;
			if( binYtop > m_pDB->m_coreRgn.top )
				binYtop = m_pDB->m_coreRgn.top;
            this->m_routerBin[j][k].m_binArea=(binXright-binX)*(binYtop-binY);
            if(m_routerBin[j][k].m_binArea<=0)
            {
                cerr<<"\nWarning:binArea<=0";
            }
        }
    }


    //---------------------------------------------------------------------
    //for all macros and fixed object, calc the blockageArea over the bins
    for(int i=0; i<(int)m_pDB->m_modules.size(); ++i)
    {
		double w = m_pDB->m_modules[i].m_width;
		double h = m_pDB->m_modules[i].m_height;
		double left   = m_pDB->m_modules[i].m_x;
		double bottom = m_pDB->m_modules[i].m_y;
		double right  = left   + w;
		double top    = bottom + h;
        if( left>=m_pDB->m_coreRgn.right)
            continue;
        if( right<=m_pDB->m_coreRgn.left)
            continue;
        if( bottom >=m_pDB->m_coreRgn.top)
            continue;
        if( top <=m_pDB->m_coreRgn.bottom)
            continue;
        int xid1=getBinXIndex( left );
        int xid2=getBinXIndex( right );
        int yid1=getBinYIndex( bottom );
        int yid2=getBinYIndex( top );

        if(m_pDB->m_modules[i].m_isFixed==true)
        {
            for(int j=xid1; j<=xid2; ++j)
            {
                for(int k=yid1; k<=yid2; ++k)
                {
                    double binX=j*m_binWidth + m_pDB->m_coreRgn.left;
                    double binY=k*m_binHeight + m_pDB->m_coreRgn.bottom;

                    double binXright = binX + m_binWidth;
                    double binYtop = binY + m_binHeight;
				    if( binXright > m_pDB->m_coreRgn.right )
					    binXright = m_pDB->m_coreRgn.right;
				    if( binYtop > m_pDB->m_coreRgn.top )
					    binYtop = m_pDB->m_coreRgn.top;
    				
				    double common_area = 
					    getOverlap( left, right, binX, binXright ) *
					    getOverlap( bottom, top, binY, binYtop );

                    m_routerBin[j][k].m_blockageArea += common_area;
                }
            }
        }
        else if(m_pDB->m_modules[i].m_height > m_pDB->m_rowHeight && m_pDB->m_modules[i].m_isCluster==false) //movable macro
        {
            for(int j=xid1; j<=xid2; ++j)
            {
                for(int k=yid1; k<=yid2; ++k)
                {
                    double binX=j*m_binWidth + m_pDB->m_coreRgn.left;
                    double binY=k*m_binHeight + m_pDB->m_coreRgn.bottom;

                    double binXright = binX + m_binWidth;
                    double binYtop = binY + m_binHeight;
				    if( binXright > m_pDB->m_coreRgn.right )
					    binXright = m_pDB->m_coreRgn.right;
				    if( binYtop > m_pDB->m_coreRgn.top )
					    binYtop = m_pDB->m_coreRgn.top;
    				
				    double common_area = 
					    getOverlap( left, right, binX, binXright ) *
					    getOverlap( bottom, top, binY, binYtop );

                    m_routerBin[j][k].m_blockageArea += common_area*0.5; //for movable macros, only metal 5,6,7,8 can be routed => half resource (0.5)
                }
            }
        }
    }
    //---------------------------------------------------------------------
    //for all bins, calc available tracks
    //int HtracksPerRowHeight=20; //m3 + m5 + m7, 8+8+4=20
    //int VtracksPerRowHeight=28; // m2 + m4 + m6 + m8 8+8+8+4=28

    int fullBinHtracks = (int)floor((m_binHeight/m_pDB->m_rowHeight)*m_HtracksPerRowHeight);
    int fullBinVtracks = (int)floor((m_binWidth/m_pDB->m_rowHeight)*m_VtracksPerRowHeight);
    for(int j=0; j<this->m_nBinCol; ++j)
    {
        for(int k=0; k<this->m_nBinRow; ++k)
        {
            double emptyRatio= 1 - (m_routerBin[j][k].m_blockageArea/m_routerBin[j][k].m_binArea);
            if(emptyRatio<0)
                emptyRatio=0;
            m_routerBin[j][k].m_nH_Track=(int)floor(fullBinHtracks*emptyRatio);
            m_routerBin[j][k].m_nV_Track=(int)floor(fullBinVtracks*emptyRatio);
        }
    }

}
int MyGlobalRouter::getBinXIndex(const double& x)
{
    int id=(int)floor( (x-m_pDB->m_coreRgn.left)/this->m_binWidth);
    if(id<0)
        id=0;
    if(id>=this->m_nBinCol)
        id=m_nBinCol-1;

    return id;

}

int MyGlobalRouter::getBinYIndex(const double& y)
{
    int id=(int)floor( (y-m_pDB->m_coreRgn.bottom)/this->m_binHeight);
    if(id<0)
        id=0;
    if(id>=this->m_nBinRow)
        id=m_nBinRow-1;

    return id;

}
void MyGlobalRouter::findPath( const int& netID, vector<MyGlobalRouterPath>& paths)
{
    paths.clear();
    paths.reserve(this->m_netRMSTs[netID].size());
    for(int i=0; i<(int)m_pDB->m_nets[netID].size(); ++i)
    {
        if( i != this->m_netRMSTs[netID][i] )
        {
            MyGlobalRouterPath path;
            if(findPath2P( m_pDB->m_nets[netID][i], m_pDB->m_nets[netID][m_netRMSTs[netID][i]], path)==true)
                paths.push_back(path);
        }
    }
}
bool MyGlobalRouter::findPath2P( const int& pinID1, const int& pinID2, MyGlobalRouterPath& path)
{
    double x1=m_pDB->m_pins[pinID1].absX;
    double y1=m_pDB->m_pins[pinID1].absY;

    double x2=m_pDB->m_pins[pinID2].absX;
    double y2=m_pDB->m_pins[pinID2].absY;

    int bx1=getBinXIndex(x1);
    int by1=getBinYIndex(y1);

    int bx2=getBinXIndex(x2);
    int by2=getBinYIndex(y2);

    assert(bx1<this->m_nBinCol);
    assert(bx2<this->m_nBinCol);
    assert(by1<this->m_nBinRow);
    assert(by2<this->m_nBinRow);

    if( bx1==bx2 && by1==by2 )
    {
        return false; //the 2 pins are in the same bin, ignore them.
    }

    //vertical line
    if(bx1==bx2)
    {
        path.m_startBinIdX=bx1;
        path.m_startBinIdY=by1;
        path.m_cornerX.resize(1,bx2);
        path.m_cornerY.resize(1,by2);

        this->addVcost(by1,by2,bx1);
        return true;
    }
    //Horizontal line
    if(by1==by2)
    {
        path.m_startBinIdX=bx1;
        path.m_startBinIdY=by1;
        path.m_cornerX.resize(1,bx2);
        path.m_cornerY.resize(1,by2);

        this->addHcost(bx1,bx2,by1);
        return true;
    }

    //z-line
    int xid=-1;
    int yid=-1;
    int maxCost=0; //find the path with min(maxCost)
    bool first=true;
    //----------------------
    //HVH route
    //----------------------
    if( bx1<bx2 && by1>by2 )
    {
        for(int lid=bx1; lid<=bx2; ++lid)
        {
            int mcost=0;
            if(lid!=bx1)
            {
                for(int i=bx1; i<=lid; ++i)
                {
                    if(m_routerBin[i][by1].m_H_used>mcost)
                        mcost=m_routerBin[i][by1].m_H_used;
                }
            }
            for(int i=by2; i<=by1; ++i)
            {
                if(m_routerBin[lid][i].m_V_used>mcost)
                    mcost=m_routerBin[lid][i].m_V_used;
            }
            if(lid!=bx2)
            {
                for(int i=lid; i<=bx2; ++i)
                {
                    if(m_routerBin[i][by2].m_H_used>mcost)
                        mcost=m_routerBin[i][by2].m_H_used;
                }
            }
            if(first==true)
            {
                maxCost=mcost;
                xid=lid;
                yid=-1;
                first=false;
            }
            else if(mcost<maxCost)
            {
                maxCost=mcost;
                xid=lid;
                yid=-1;
            }
        }
    }
    else if( bx1<bx2 && by1<by2 )
    {
        for(int lid=bx1; lid<=bx2; ++lid)
        {
            int mcost=0;
            if(lid!=bx1)
            {
                for(int i=bx1; i<=lid; ++i)
                {
                    if(m_routerBin[i][by1].m_H_used>mcost)
                        mcost=m_routerBin[i][by1].m_H_used;
                }
            }
            for(int i=by1; i<=by2; ++i)
            {
                if(m_routerBin[lid][i].m_V_used>mcost)
                    mcost=m_routerBin[lid][i].m_V_used;
            }
            if(lid!=bx2)
            {
                for(int i=lid; i<=bx2; ++i)
                {
                    if(m_routerBin[i][by2].m_H_used>mcost)
                        mcost=m_routerBin[i][by2].m_H_used;
                }
            }
            if(first==true)
            {
                maxCost=mcost;
                xid=lid;
                yid=-1;
                first=false;
            }
            else if(mcost<maxCost)
            {
                maxCost=mcost;
                xid=lid;
                yid=-1;
            }
        }
    }
    else if( bx1>bx2 && by1<by2 )
    {
        for(int lid=bx2; lid<=bx1; ++lid)
        {
            int mcost=0;
            if(lid!=bx2)
            {
                for(int i=bx2; i<=lid; ++i)
                {
                    if(m_routerBin[i][by2].m_H_used>mcost)
                        mcost=m_routerBin[i][by2].m_H_used;
                }
            }
            for(int i=by1; i<=by2; ++i)
            {
                if(m_routerBin[lid][i].m_V_used>mcost)
                    mcost=m_routerBin[lid][i].m_V_used;
            }
            if(lid!=bx1)
            {
                for(int i=lid; i<=bx1; ++i)
                {
                    if(m_routerBin[i][by1].m_H_used>mcost)
                        mcost=m_routerBin[i][by1].m_H_used;
                }
            }
            if(first==true)
            {
                maxCost=mcost;
                xid=lid;
                yid=-1;
                first=false;
            }
            else if(mcost<maxCost)
            {
                maxCost=mcost;
                xid=lid;
                yid=-1;
            }
        }
    }
    else
    {
        for(int lid=bx2; lid<=bx1; ++lid)
        {
            int mcost=0;
            if(lid!=bx2)
            {
                for(int i=bx2; i<=lid; ++i)
                {
                    if(m_routerBin[i][by2].m_H_used>mcost)
                        mcost=m_routerBin[i][by2].m_H_used;
                }
            }
            for(int i=by2; i<=by1; ++i)
            {
                if(m_routerBin[lid][i].m_V_used>mcost)
                    mcost=m_routerBin[lid][i].m_V_used;
            }
            if(lid!=bx1)
            {
                for(int i=lid; i<=bx1; ++i)
                {
                    if(m_routerBin[i][by1].m_H_used>mcost)
                        mcost=m_routerBin[i][by1].m_H_used;
                }
            }
            if(first==true)
            {
                maxCost=mcost;
                xid=lid;
                yid=-1;
                first=false;
            }
            else if(mcost<maxCost)
            {
                maxCost=mcost;
                xid=lid;
                yid=-1;
            }
        }
    }

    //----------------------
    //VHV route
    //----------------------
    if( bx1<bx2 && by1>by2 )
    {
        for(int lid=by2; lid<=by1; ++lid)
        {
            int mcost=0;
            if(lid!=by1)
            {
                for(int i=lid; i<=by1; ++i)
                {
                    if(m_routerBin[bx1][i].m_V_used>mcost)
                        mcost=m_routerBin[bx1][i].m_V_used;
                }
            }
            for(int i=bx1; i<=bx2; ++i)
            {
                if(m_routerBin[i][lid].m_H_used>mcost)
                    mcost=m_routerBin[i][lid].m_H_used;
            }
            if(lid!=by2)
            {
                for(int i=by2; i<=lid; ++i)
                {
                    if(m_routerBin[bx2][i].m_V_used>mcost)
                        mcost=m_routerBin[bx2][i].m_V_used;
                }
            }
            if(mcost<maxCost)
            {
                maxCost=mcost;
                xid=-1;
                yid=lid;
            }
        }
    }
    else if( bx1<bx2 && by1<by2 )
    {
        for(int lid=by1; lid<=by2; ++lid)
        {
            int mcost=0;
            if(lid!=by1)
            {
                for(int i=by1; i<=lid; ++i)
                {
                    if(m_routerBin[bx1][i].m_V_used>mcost)
                        mcost=m_routerBin[bx1][i].m_V_used;
                }
            }
            for(int i=bx1; i<=bx2; ++i)
            {
                if(m_routerBin[i][lid].m_H_used>mcost)
                    mcost=m_routerBin[i][lid].m_H_used;
            }
            if(lid!=by2)
            {
                for(int i=lid; i<=by2; ++i)
                {
                    if(m_routerBin[bx2][i].m_V_used>mcost)
                        mcost=m_routerBin[bx2][i].m_V_used;
                }
            }
            if(mcost<maxCost)
            {
                maxCost=mcost;
                xid=-1;
                yid=lid;
            }
        }
    }
    else if( bx1>bx2 && by1<by2 )
    {
        for(int lid=by1; lid<=by2; ++lid)
        {
            int mcost=0;
            if(lid!=by2)
            {
                for(int i=lid; i<=by2; ++i)
                {
                    if(m_routerBin[bx2][i].m_V_used>mcost)
                        mcost=m_routerBin[bx2][i].m_V_used;
                }
            }
            for(int i=bx2; i<=bx1; ++i)
            {
                if(m_routerBin[i][lid].m_H_used>mcost)
                    mcost=m_routerBin[i][lid].m_H_used;
            }
            if(lid!=by1)
            {
                for(int i=by1; i<=lid; ++i)
                {
                    if(m_routerBin[bx1][i].m_V_used>mcost)
                        mcost=m_routerBin[bx1][i].m_V_used;
                }
            }
            if(mcost<maxCost)
            {
                maxCost=mcost;
                xid=-1;
                yid=lid;
            }
        }
    }
    else
    {
        for(int lid=by2; lid<=by1; ++lid)
        {
            int mcost=0;
            if(lid!=by2)
            {
                for(int i=by2; i<=lid; ++i)
                {
                    if(m_routerBin[bx2][i].m_V_used>mcost)
                        mcost=m_routerBin[bx2][i].m_V_used;
                }
            }
            for(int i=bx2; i<=bx1; ++i)
            {
                if(m_routerBin[i][lid].m_H_used>mcost)
                    mcost=m_routerBin[i][lid].m_H_used;
            }
            if(lid!=by1)
            {
                for(int i=lid; i<=by1; ++i)
                {
                    if(m_routerBin[bx1][i].m_V_used>mcost)
                        mcost=m_routerBin[bx1][i].m_V_used;
                }
            }
            if(mcost<maxCost)
            {
                maxCost=mcost;
                xid=-1;
                yid=lid;
            }
        }
    }

    if(yid==-1) //HVH route
    {
        assert(xid>=0);
        assert(xid<this->m_nBinCol);
        path.m_startBinIdX=bx1;
        path.m_startBinIdY=by1;
        path.m_cornerX.resize(3);
        path.m_cornerY.resize(3);

        path.m_cornerX[0]=xid;
        path.m_cornerY[0]=by1;

        path.m_cornerX[1]=xid;
        path.m_cornerY[1]=by2;

        path.m_cornerX[2]=bx2;
        path.m_cornerY[2]=by2;

        this->addHcost( bx1, xid, by1);
        this->addVcost( by1, by2, xid);
        this->addHcost( bx2, xid, by2);
    }
    else //VHV route
    {
        assert(xid==-1);
        assert(yid>=0);
        assert(yid<this->m_nBinRow);
        path.m_startBinIdX=bx1;
        path.m_startBinIdY=by1;
        path.m_cornerX.resize(3);
        path.m_cornerY.resize(3);

        path.m_cornerX[0]=bx1;
        path.m_cornerY[0]=yid;

        path.m_cornerX[1]=bx2;
        path.m_cornerY[1]=yid;

        path.m_cornerX[2]=bx2;
        path.m_cornerY[2]=by2;

        this->addVcost( by1, yid, bx1);
        this->addHcost( bx1, bx2, yid);
        this->addVcost( by2, yid, bx2);
    }
    return true;



}

void MyGlobalRouter::route()
{
    //---------------------------------------------------------------
    // Step 1: for all nets, build RMST
    //---------------------------------------------------------------
    m_netRMSTs.reserve(m_pDB->m_nets.size());
    for(int i=0; i<(int)m_pDB->m_nets.size(); ++i)
    {
        vector<int> mst;
        this->buildMST( i , mst);
        m_netRMSTs.push_back(mst);
    }

    //---------------------------------------------------------------
    // Step 1: for all nets, find all 2-p path by pattern route
    //---------------------------------------------------------------
    m_netRMST_Paths.reserve(m_pDB->m_nets.size());
    for(int i=0; i<(int)m_pDB->m_nets.size(); ++i)
    {
        vector<MyGlobalRouterPath> mstPath;
        this->findPath(i,mstPath);
        m_netRMST_Paths.push_back(mstPath);

    }


}

void MyGlobalRouter::order( int& small, int& large)
{
    if(large<small)
    {
        int temp=large;
        large=small;
        small=temp;
    }
}
void MyGlobalRouter::addHcost( int x1, int x2, const int& y)
{
    order( x1, x2 );
    if(x1==x2)
        return;

    for(int i=x1; i<=x2; ++i)
    {
        ++m_routerBin[i][y].m_H_used;
    }
}
void MyGlobalRouter::addVcost( int y1, int y2, const int& x)
{
    order( y1, y2 );
    if(y1==y2)
        return;

    for(int i=y1; i<=y2; ++i)
    {
        ++m_routerBin[x][i].m_V_used;
    }
}

void MyGlobalRouter::outputfig( const char* prefix)
{
//    bool withCellMove=false;
    bool showMsg=true;
    bool withZoom=true;
    bool withOrient=true;
    bool withPin=true;

    m_pDB->CalcHPWL();
    string plt_filename,net_filename,fixed_filename,module_filename,move_filename, dummy_filename, pin_filename;
    FILE* pPlt;
    FILE* pNet;
    FILE* pFixed;
    FILE* pMod;
    FILE* pMove;
    FILE* pDummy;   // donnie 2006-03-02
    FILE* pPin;   // indark 2006-04-24
    if(withZoom){
    	plt_filename = net_filename = fixed_filename = module_filename= move_filename = dummy_filename = pin_filename = prefix ;
    	plt_filename    += ".plt";
    	net_filename    += "_net.dat";
    	fixed_filename  += "_fixed.dat";
    	module_filename += "_mod.dat";
    	move_filename   += "_move.dat";
	dummy_filename  += "_dummy.dat";
	pin_filename	+= "_pin.dat";
    	pPlt   = fopen( plt_filename.c_str(), "w" );
    	pNet   = fopen( net_filename.c_str(), "w" );
    	pFixed = fopen( fixed_filename.c_str(), "w" );
    	pMod   = fopen( module_filename.c_str(), "w" );
    	pMove  = fopen( move_filename.c_str(), "w" );
	pDummy = fopen( dummy_filename.c_str(), "w" );
	pPin   = fopen( pin_filename.c_str(), "w" );
	
    }else{
    	pPlt = fopen( prefix, "w" );
    	pNet = pFixed = pMod = pMove = pDummy = pPin = pPlt;
    	
    }

    if( !(pPlt && pNet && pFixed && pMod && pMove && pDummy && pPin) )
    {
	cerr << "Error, cannot open output file: " << prefix << endl;
	return;
    }
    

    if( showMsg )
    {
    	if (withZoom)
    		//printf( "Output gnuplot figure with prefix: %s\n", prefix );
    		printf( "Output placement: %s\n", prefix );
    	else
		//printf( "Output gnuplot figure: %s\n", prefix );
		printf( "Output placement (single): %s\n", prefix );
    }

    // output title
    fprintf( pPlt, "\nset title \" %s, block= %d, net= %d, HPWL= %.0f \" font \"Times, 22\"\n\n",
	    prefix, (int)m_pDB->m_modules.size(), m_pDB->m_nNets, m_pDB->GetHPWLp2p() );

    fprintf( pPlt, "set size ratio 1\n" );
    
    if(!withZoom)
    	fprintf( pPlt, "set nokey\n\n" ); 

    //if( withCellMove && (int)m_nets.size() < 2000 )
    if (withZoom)
	    fprintf( pPlt, "plot[:][:] '%s' w l lt 4, '%s' w l lt 3, '%s' w l lt 1, '%s' w l lt 7, '%s' w l lt 5, '%s' w l lt 2 \n\n",
	   fixed_filename.c_str(), module_filename.c_str(), move_filename.c_str(),
	   net_filename.c_str(), dummy_filename.c_str(), pin_filename.c_str() );
    else
	fprintf( pPlt, "plot[:][:] '-' w l lt 4, '-' w l lt 3, '-' w l lt 1, '-' w l lt 7, '-' w l lt 5, '-' w l lt 2 \n\n" ); 

    // output Core region
    fprintf( pFixed, "\n# core region\n" ); 
    fprintf( pFixed, "%12.3f, %12.3f\n", m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.bottom );
    fprintf( pFixed, "%12.3f, %12.3f\n", m_pDB->m_coreRgn.right, m_pDB->m_coreRgn.bottom );
    fprintf( pFixed, "%12.3f, %12.3f\n", m_pDB->m_coreRgn.right, m_pDB->m_coreRgn.top );
    fprintf( pFixed, "%12.3f, %12.3f\n", m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.top ); 
    fprintf( pFixed, "%12.3f, %12.3f\n\n", m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.bottom );
    fprintf( pFixed, "\n# die area\n" );
    fprintf( pFixed, "%12.3f, %12.3f\n", m_pDB->m_dieArea.left,  m_pDB->m_dieArea.bottom );
    fprintf( pFixed, "%12.3f, %12.3f\n", m_pDB->m_dieArea.right, m_pDB->m_dieArea.bottom );
    fprintf( pFixed, "%12.3f, %12.3f\n", m_pDB->m_dieArea.right, m_pDB->m_dieArea.top );
    fprintf( pFixed, "%12.3f, %12.3f\n", m_pDB->m_dieArea.left,  m_pDB->m_dieArea.top );
    fprintf( pFixed, "%12.3f, %12.3f\n\n", m_pDB->m_dieArea.left, m_pDB->m_dieArea.bottom );


    // output fixed modules
    fprintf( pFixed, "\n# fixed blocks\n" ); 
    fprintf( pFixed, "0, 0\n\n" ); 
    double x, y, w, h;
    int orient;
    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
    {
	x = m_pDB->m_modules[i].GetX();
	y = m_pDB->m_modules[i].GetY();
	w = m_pDB->m_modules[i].GetWidth();
	h = m_pDB->m_modules[i].GetHeight();
	orient = m_pDB->m_modules[i].GetOrient();
	if( m_pDB->m_modules[i].m_isFixed && !m_pDB->m_modules[i].m_isDummy )
	{
	    fprintf( pFixed, "%12.3f, %12.3f\n", x, y );
	    fprintf( pFixed, "%12.3f, %12.3f\n", x+w, y );
	    fprintf( pFixed, "%12.3f, %12.3f\n", x+w, y+h );
	    fprintf( pFixed, "%12.3f, %12.3f\n", x, y+h ); 
	    fprintf( pFixed, "%12.3f, %12.3f\n\n", x, y );
		//draw orientation indicator
	    if(withOrient){
		switch(orient){
			case 0:		//N
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.8, y+h*0.5 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.8, y+h );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.8 - w*0.1, y+h - h*0.1 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.8, y+h );
				fprintf( pFixed, "%12.3f, %12.3f\n\n", x+w*0.8 + w*0.1, y+h - h*0.1 );
				break;
			case 1:		//W
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.5, y+h*0.8 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x, y + h*0.8 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.1, y+h*0.9 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x, y + h*0.8 );
				fprintf( pFixed, "%12.3f, %12.3f\n\n", x + w*0.1, y + h*0.7 );
			break;
			case 2:		//S
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.2, y+h*0.5 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.2, y );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.2-w*0.1, y+h*0.1 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.2, y );
				fprintf( pFixed, "%12.3f, %12.3f\n\n", x+w*0.2 + w*0.1, y+h*0.1 );
				break;
			case 3:		//E
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.5, y+h*0.2 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w, y+h*0.2 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w - w * 0.1, y+h*0.2 + h * 0.1  );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w, y+h*0.2 );
				fprintf( pFixed, "%12.3f, %12.3f\n\n", x+w - w * 0.1, y+h*0.2 - h * 0.1 );

				break;
			case 4:		//FN
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.2, y+h*0.5 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.2, y + h );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.2-w*0.1, y + h*0.9 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.2, y + h );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.2 + w*0.1, y + h*0.9 );
				fprintf( pFixed, "%12.3f, %12.3f\n\n", x+w*0.2-w*0.1, y + h*0.9 );
				break;
			case 5:		//FW
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.5, y+h*0.8 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w, y + h *0.8 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w-w*0.1, y + h *0.9 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w, y + h *0.8 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w -w*0.1, y + h *0.7 );
				fprintf( pFixed, "%12.3f, %12.3f\n\n", x+w-w*0.1, y + h *0.9 );
				break;
			case 6:		//FS
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.8, y+h*0.5 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.8, y  );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.7, y + h * 0.1 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.8, y );
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.9, y + h * 0.1);
				fprintf( pFixed, "%12.3f, %12.3f\n\n", x+w*0.7, y + h * 0.1 );
				break;
			case 7:		//FE
				fprintf( pFixed, "%12.3f, %12.3f\n", x+w*0.5, y+h*0.2 );
				fprintf( pFixed, "%12.3f, %12.3f\n", x, y + h*0.2  );
				fprintf( pFixed, "%12.3f, %12.3f\n", x + w*0.1, y + h*0.1  );
				fprintf( pFixed, "%12.3f, %12.3f\n", x, y + h*0.2  );
				fprintf( pFixed, "%12.3f, %12.3f\n", x + w*0.1, y + h*0.3  );
				fprintf( pFixed, "%12.3f, %12.3f\n\n", x + w*0.1, y + h*0.1  );
				break;

			default:
				fprintf( pFixed, "%12.3f, %12.3f\n", x, y );
				fprintf( pFixed, "%12.3f, %12.3f\n\n", x + w, y +h );
				fprintf( pFixed, "%12.3f, %12.3f\n", x + w, y );
				fprintf( pFixed, "%12.3f, %12.3f\n\n", x, y +h );
				cerr << "Orientation Error " << endl;	

		}
	    }


	}
    }
    if(!withZoom)
    	fprintf( pPlt, "\nEOF\n\n" );

    // output movable modules
    fprintf( pMod, "\n# blocks\n" ); 
    fprintf( pMod, "0, 0\n\n" );

    if( true )
    {

	for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
	{
	    x = m_pDB->m_modules[i].GetX();
	    y = m_pDB->m_modules[i].GetY();
	    w = m_pDB->m_modules[i].GetWidth();
	    h = m_pDB->m_modules[i].GetHeight();
	    orient = m_pDB->m_modules[i].GetOrient();
	    if( !m_pDB->m_modules[i].m_isFixed && !m_pDB->m_modules[i].m_isDummy )
	    {
		if( (int)m_pDB->m_modules.size()< 50000 || m_pDB->m_modules[i].m_height > m_pDB->m_rowHeight )
		{
		    // draw blocks
		    fprintf( pMod, "%12.3f, %12.3f\n", x, y );
		    fprintf( pMod, "%12.3f, %12.3f\n", x+w, y );
		    fprintf( pMod, "%12.3f, %12.3f\n", x+w, y+h );
		    fprintf( pMod, "%12.3f, %12.3f\n", x, y+h ); 
		    fprintf( pMod, "%12.3f, %12.3f\n\n", x, y );

		    //draw orientation indicator
		    if(withOrient){
			switch(orient){
			    case 0:		//N
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.8, y+h*0.5 );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.8, y+h );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.8 - w*0.1, y+h - h*0.1 );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.8, y+h );
				fprintf( pMod, "%12.3f, %12.3f\n\n", x+w*0.8 + w*0.1, y+h - h*0.1 );
				break;
			    case 1:		//W
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.5, y+h*0.8 );
				fprintf( pMod, "%12.3f, %12.3f\n", x, y + h*0.8 );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.1, y+h*0.9 );
				fprintf( pMod, "%12.3f, %12.3f\n", x, y + h*0.8 );
				fprintf( pMod, "%12.3f, %12.3f\n\n", x + w*0.1, y + h*0.7 );
				break;
			    case 2:		//S
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.2, y+h*0.5 );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.2, y );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.2-w*0.1, y+h*0.1 );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.2, y );
				fprintf( pMod, "%12.3f, %12.3f\n\n", x+w*0.2 + w*0.1, y+h*0.1 );
				break;
			    case 3:		//E
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.5, y+h*0.2 );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w, y+h*0.2 );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w - w * 0.1, y+h*0.2 + h * 0.1  );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w, y+h*0.2 );
				fprintf( pMod, "%12.3f, %12.3f\n\n", x+w - w * 0.1, y+h*0.2 - h * 0.1 );

				break;
			    case 4:		//FN
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.2, y+h*0.5 );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.2, y + h );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.2-w*0.1, y + h*0.9 );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.2, y + h );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.2 + w*0.1, y + h*0.9 );
				fprintf( pMod, "%12.3f, %12.3f\n\n", x+w*0.2-w*0.1, y + h*0.9 );
				break;
			    case 5:		//FW
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.5, y+h*0.8 );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w, y + h *0.8 );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w-w*0.1, y + h *0.9 );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w, y + h *0.8 );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w -w*0.1, y + h *0.7 );
				fprintf( pMod, "%12.3f, %12.3f\n\n", x+w-w*0.1, y + h *0.9 );
				break;
			    case 6:		//FS
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.8, y+h*0.5 );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.8, y  );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.7, y + h * 0.1 );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.8, y );
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.9, y + h * 0.1);
				fprintf( pMod, "%12.3f, %12.3f\n\n", x+w*0.7, y + h * 0.1 );
				break;
			    case 7:		//FE
				fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.5, y+h*0.2 );
				fprintf( pMod, "%12.3f, %12.3f\n", x, y + h*0.2  );
				fprintf( pMod, "%12.3f, %12.3f\n", x + w*0.1, y + h*0.1  );
				fprintf( pMod, "%12.3f, %12.3f\n", x, y + h*0.2  );
				fprintf( pMod, "%12.3f, %12.3f\n", x + w*0.1, y + h*0.3  );
				fprintf( pMod, "%12.3f, %12.3f\n\n", x + w*0.1, y + h*0.1  );
				break;

			    default:
				fprintf( pMod, "%12.3f, %12.3f\n", x, y );
				fprintf( pMod, "%12.3f, %12.3f\n\n", x + w, y +h );
				fprintf( pMod, "%12.3f, %12.3f\n", x + w, y );
				fprintf( pMod, "%12.3f, %12.3f\n\n", x, y +h );
				cerr << "Orientation Error " << endl;	

			}
		    }
		}
		else
		{
		    // draw line
		    fprintf( pMod, "%12.3f, %12.3f\n", x, y+h/2 );
		    fprintf( pMod, "%12.3f, %12.3f\n\n", x+w, y+h/2 );
		}
	    }
	}

    }
    if(!withZoom)
	fprintf( pPlt, "\nEOF\n\n" );

    fprintf( pMove, "\n# cell move\n" ); 
    fprintf( pMove, "0, 0\n\n" ); 
    if(!withZoom)
	fprintf( pPlt, "\nEOF\n\n" );

    // 2005/03/11 output nets
    fprintf( pNet, "\n# nets\n" );
    fprintf( pNet, "0, 0\n\n" ); 
    //if( (int)m_pDB->m_nets.size() < 2000 && (int)m_pDB->m_nets.size() != 0 )
    //{
	    for( int i=0; i<(int)m_pDB->m_nets.size(); i++ )
	    {
            if( (int)m_pDB->m_nets[i].size() >= 2 )
            {
                for(int j=0; j< (int)this->m_netRMST_Paths[i].size(); ++j)
                {
                    double x1,x2,y1,y2;
                    getBinCenter(x1,y1,m_netRMST_Paths[i][j].m_startBinIdX,m_netRMST_Paths[i][j].m_startBinIdY);
                    for(int k=0; k<(int)this->m_netRMST_Paths[i][j].m_cornerX.size(); ++k)
                    {
                        getBinCenter(x2,y2,m_netRMST_Paths[i][j].m_cornerX[k],m_netRMST_Paths[i][j].m_cornerY[k]);
                        fprintf( pNet, "%12.3f, %12.3f\n", x1, y1 );
                        fprintf( pNet, "%12.3f, %12.3f\n", x2, y2 );
                        x1=x2;
                        y1=y2;

                    }
                    fprintf( pNet, "\n" );
                }
            }

	    }
    //}
    if(!withZoom){
	fprintf( pPlt, "\nEOF\n\n" );
    }


    // output dummy modules
    fprintf( pDummy, "\n# dummy modules\n" ); 
    fprintf( pDummy, "0, 0\n\n" ); 
    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
    {
	x = m_pDB->m_modules[i].GetX();
	y = m_pDB->m_modules[i].GetY();
	w = m_pDB->m_modules[i].GetWidth();
	h = m_pDB->m_modules[i].GetHeight();

	if( m_pDB->m_modules[i].m_isDummy )
	{
	    if( (int)m_pDB->m_modules.size()< 50000 || m_pDB->m_modules[i].m_height > m_pDB->m_rowHeight )
	    {
		// draw blocks
		fprintf( pMod, "%12.3f, %12.3f\n", x, y );
		fprintf( pMod, "%12.3f, %12.3f\n", x+w, y );
		fprintf( pMod, "%12.3f, %12.3f\n", x+w, y+h );
		fprintf( pMod, "%12.3f, %12.3f\n", x, y+h ); 
		fprintf( pMod, "%12.3f, %12.3f\n\n", x, y );

		fprintf( pMod, "%12.3f, %12.3f\n", x+w*0.75, y+h );
		fprintf( pMod, "%12.3f, %12.3f\n\n", x+w,      y+h*0.5 );
	    }
	    else
	    {
		// draw line
		fprintf( pMod, "%12.3f, %12.3f\n", x, y+h/2 );
		fprintf( pMod, "%12.3f, %12.3f\n\n", x+w, y+h/2 );
	    }
	}
    }
    if(!withZoom){
	fprintf( pPlt, "\nEOF\n\n" );
    }


    //indark output_ pins
    fprintf( pPin, "\n# pins\n" );
    fprintf( pPin, "\t0, 0\n\n" );
    double pin_x,pin_y;
    double pin_width = m_pDB->m_rowHeight * 0.5;
    if (withPin){
	for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
	{

	    if( m_pDB->m_modules[i].m_isFixed || m_pDB->m_modules[i].m_height > m_pDB->m_rowHeight * 2 )
	    {
		fprintf( pPin, "     # pin for block %s\n", m_pDB->m_modules[i].GetName().c_str() );
		for (int j = 0 ; j < (int)m_pDB->m_modules[i].m_pinsId.size(); j++)
		{
		    int pin_id =  m_pDB->m_modules[i].m_pinsId[j];
		    pin_x = m_pDB->m_pins[pin_id].absX;
		    pin_y = m_pDB->m_pins[pin_id].absY;
		    fprintf( pPin, "%12.3f, %12.3f\n",   pin_x - pin_width*0.5, pin_y - pin_width*0.5 );
		    fprintf( pPin, "%12.3f, %12.3f\n",   pin_x - pin_width*0.5, pin_y + pin_width*0.5 );
		    fprintf( pPin, "%12.3f, %12.3f\n",   pin_x + pin_width*0.5, pin_y + pin_width*0.5 );
		    fprintf( pPin, "%12.3f, %12.3f\n",   pin_x + pin_width*0.5, pin_y - pin_width*0.5 );
		    fprintf( pPin, "%12.3f, %12.3f\n\n", pin_x - pin_width*0.5, pin_y - pin_width*0.5 );
		}
	    }
	}
    }
    if(!withZoom){
	fprintf( pPin, "\nEOF\n\n" );
    }




    
    

    fprintf( pPlt, "pause -1 'Press any key'" );
    if(withZoom){
	fclose( pPlt );
	fclose( pMod );
	fclose( pFixed );
	fclose( pMove );
	fclose( pNet );
	fclose( pDummy );
	fclose( pPin );
    }else{
	fclose( pPlt );
    }

}

void MyGlobalRouter::getBinCenter(double& x, double& y, const int& bx, const int& by)
{
    assert(bx<this->m_nBinCol);
    assert(by<this->m_nBinRow);
    x=m_pDB->m_coreRgn.left+ (bx+0.5)*this->m_binWidth;
    y=m_pDB->m_coreRgn.bottom+ (by+0.5)*this->m_binHeight;
}

void MyGlobalRouter::OutputHGrid( string filename )
{

    int stepSize = m_nBinCol/1000;
    if( stepSize == 0 )
	stepSize = 1;
    FILE* out = fopen( filename.c_str(), "w" );

    for( int j=0; j<this->m_nBinRow; j+=stepSize )
    {
        for( int i=0; i<this->m_nBinCol; i+=stepSize )
        {
            assert((m_routerBin[i][j].m_nH_Track)>=0);
            if(m_routerBin[i][j].m_H_used>=m_routerBin[i][j].m_nH_Track)
                fprintf( out, "%d ", m_routerBin[i][j].m_H_used-m_routerBin[i][j].m_nH_Track  );
            else
                fprintf( out, "%d ", 0  );

            //if(m_routerBin[j][i].m_H_used>=m_routerBin[j][i].m_nH_Track)
            //    fprintf( out, "%.03f ", ((double)this->m_routerBin[j][i].m_H_used-m_routerBin[j][i].m_nH_Track)/((double)m_routerBin[j][i].m_nH_Track)  );
            //else
            //    fprintf( out, "%.03f ", 0  );
        }
	    fprintf( out, "\n" );
    }
    fprintf( out, "\n" );
    fclose( out );
}
void MyGlobalRouter::OutputVGrid( string filename )
{

    int stepSize = m_nBinCol / 1000;
    if( stepSize == 0 )
	stepSize = 1;
    FILE* out = fopen( filename.c_str(), "w" );

    for( int j=0; j<this->m_nBinRow; j+=stepSize )
    {
        for( int i=0; i<this->m_nBinCol; i+=stepSize )
        {
            assert((m_routerBin[i][j].m_nV_Track)>=0);
            if(m_routerBin[i][j].m_V_used>=m_routerBin[i][j].m_nV_Track)
                fprintf( out, "%d ", m_routerBin[i][j].m_V_used-m_routerBin[i][j].m_nV_Track  );
            else
                fprintf( out, "%d ", 0  );
        }
	    fprintf( out, "\n" );
    }
    fprintf( out, "\n" );
    fclose( out );
}

double MyGlobalRouter::calcMacroSpacing(int mId1, int mId2, bool isHspacing)
{
    if(isHspacing==true)  //calc vertical wires and required horizontal space between 2 macros
    {
        double overlapLength=getOverlap(m_pDB->m_modules[mId1].m_y,m_pDB->m_modules[mId1].m_y+m_pDB->m_modules[mId1].m_height
                                       ,m_pDB->m_modules[mId2].m_y,m_pDB->m_modules[mId2].m_y+m_pDB->m_modules[mId2].m_height);
        if(overlapLength<=0)
            return -1;
        else
        {
            double top,bottom,left,right;
            if(m_pDB->m_modules[mId1].m_cx<m_pDB->m_modules[mId2].m_cx)
            {
                left = m_pDB->m_modules[mId1].m_cx;
                right = m_pDB->m_modules[mId2].m_cx;
            }
            else
            {
                left = m_pDB->m_modules[mId2].m_cx;
                right = m_pDB->m_modules[mId1].m_cx;
            }
            if( m_pDB->m_modules[mId1].m_y > m_pDB->m_modules[mId2].m_y )
            {
                top = m_pDB->m_modules[mId2].m_y+m_pDB->m_modules[mId2].m_height;
                bottom = m_pDB->m_modules[mId1].m_y;
            }
            else
            {
                top = m_pDB->m_modules[mId1].m_y+m_pDB->m_modules[mId1].m_height;
                bottom = m_pDB->m_modules[mId2].m_y;
            }
            int leftBId,rightBId,topBId,bottomBId;
            leftBId=this->getBinXIndex(left);
            rightBId=this->getBinXIndex(right);
            topBId=this->getBinYIndex(top);
            bottomBId=this->getBinYIndex(bottom);

            //find the cut line with most vertical line
            int maxLine=0;
            int mintracks=-1;
            for( int i=bottomBId; i<=topBId; ++i)
            {
                int lines=0;
                int tracks=0;
                for( int j=leftBId; j<=rightBId; ++j)
                {
                    lines+=this->m_routerBin[j][i].m_V_used;
                    tracks+=this->m_routerBin[j][i].m_nV_Track;
                }
                if(lines>maxLine)
                    maxLine=lines;
                
                if(mintracks==-1)
                    mintracks=tracks;
                else if(mintracks>tracks)
                    mintracks=tracks;
                
            }
            int requiredTracks=maxLine-mintracks;
            if(requiredTracks<=0)
                return 0;
            else
            {
                double spacing=(((double)requiredTracks)/((double)m_VtracksPerRowHeight))*m_pDB->m_rowHeight
                    -m_pDB->m_modules[mId1].m_width/2-m_pDB->m_modules[mId2].m_width/2;
                if(spacing>0)
                    return spacing;
                else
                    return 0;
            }
        }

    }

    //----------------------------------------------------------
    //calc horizontal wires and required vertical space between 2 macros
    //----------------------------------------------------------
    else 
    {
        double overlapLength=getOverlap(m_pDB->m_modules[mId1].m_x,m_pDB->m_modules[mId1].m_x+m_pDB->m_modules[mId1].m_width
            ,m_pDB->m_modules[mId2].m_x,m_pDB->m_modules[mId2].m_x+m_pDB->m_modules[mId2].m_width);
        if(overlapLength<=0)
            return -1;
        else
        {
            double top,bottom,left,right;
            if(m_pDB->m_modules[mId1].m_cy<m_pDB->m_modules[mId2].m_cy)
            {
                bottom = m_pDB->m_modules[mId1].m_cy;
                top = m_pDB->m_modules[mId2].m_cy;
            }
            else
            {
                bottom = m_pDB->m_modules[mId2].m_cy;
                top = m_pDB->m_modules[mId1].m_cy;
            }
            if( m_pDB->m_modules[mId1].m_x > m_pDB->m_modules[mId2].m_x )
            {
                right = m_pDB->m_modules[mId2].m_x+m_pDB->m_modules[mId2].m_width;
                left = m_pDB->m_modules[mId1].m_x;
            }
            else
            {
                right = m_pDB->m_modules[mId1].m_x+m_pDB->m_modules[mId1].m_width;
                left = m_pDB->m_modules[mId2].m_x;
            }
            int leftBId,rightBId,topBId,bottomBId;
            leftBId=this->getBinXIndex(left);
            rightBId=this->getBinXIndex(right);
            topBId=this->getBinYIndex(top);
            bottomBId=this->getBinYIndex(bottom);

            //find the cut line with most horizontal line
            int maxLine=0;
            int mintracks=-1;
            for( int i=leftBId; i<=rightBId; ++i)
            {
                int lines=0;
                int tracks=0;
                for( int j=bottomBId; j<=topBId; ++j)
                {
                    lines+=this->m_routerBin[i][j].m_H_used;
                    tracks+=this->m_routerBin[i][j].m_nH_Track;
                }
                if(lines>maxLine)
                    maxLine=lines;
                
                if(mintracks==-1)
                    mintracks=tracks;
                else if(mintracks>tracks)
                    mintracks=tracks;
                
            }
            int requiredTracks=maxLine-mintracks;
            if(requiredTracks<=0)
                return 0;
            else
            {
                double spacing=(((double)requiredTracks)/((double)m_HtracksPerRowHeight))*m_pDB->m_rowHeight
                    -m_pDB->m_modules[mId1].m_height/2-m_pDB->m_modules[mId2].m_height/2;
                if(spacing>0)
                    return spacing;
                else
                    return 0;
            }
        }
    }
}
