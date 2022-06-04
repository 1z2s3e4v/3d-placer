// Copyright (C) 2004, 2005 International Business Machines and others.
// All Rights Reserved.
// This code is published under the Common Public License.
//
// $Id: MyNLP.hpp 511 2005-08-26 22:20:20Z andreasw $
//
// Authors:  Carl Laird, Andreas Waechter     IBM    2004-11-05

#ifndef __MYNLP_HPP__
#define __MYNLP_HPP__

//#include "IpTNLP.hpp"

//using namespace Ipopt;

#include "../PlaceDB/placedb.h"

#include <set>
#include <vector>
#include <string>
#include <climits>
using namespace std;

#include <semaphore.h>

class GRouteMap;

class MyNLP 
{
public:
  MyNLP( CPlaceDB& );
  virtual ~MyNLP();

  bool get_nlp_info(int& n, int& m, int& nnz_jac_g, int& nnz_h_lag );
  bool get_bounds_info(int n, vector<double>& x_l, vector<double>& x_u);
  //bool get_bounds_info(int n, double* x_l, double* x_u, int m, double* g_l, double* g_u);
  //bool get_bounds_info(int n, double* x_l, double* x_u, int m, double* g_l, double* g_u);
  //bool get_starting_point(int n, bool init_x, double* x, bool init_z, double* z_L, double* z_U,
  //                        int m, bool init_lambda, double* lambda);
  bool get_starting_point( vector<double>& x );
  double eval_f();
  bool eval_grad_f( int n, const vector<double>& x, const vector<double>& expX,
  			bool new_x, vector<double>& grad_f);

  // HLBFGS optimization
  void Optimization();

  // return true is placement is legal
  bool MySolve( double, double target_density, int currentLevel );	// solver setup
 
  int _potentialGridR;
  int m_potentialGridPadding; // 2006-10-18
  int m_potentialGridHSize;
  int m_potentialGridVSize;
  bool   m_lookAheadLegalization;
  bool   m_earlyStop;
  bool   m_topLevel;
  int    m_smoothR;
  double m_smoothDelta;

  bool   m_macroRotate; // (kaie) macro rotation
  double m_mdtime; // (kaie) macro orientation determization time
  bool   m_weightedForce;

  bool   m_prototype; // (kaie) prototyping

private:
  bool GoSolve( double, double target_density, int currentLevel, bool* isLegal );	// real solver
  
  /**@name Methods to block default compiler methods.
   * The compiler automatically generates the following three methods.
   *  Since the default compiler implementation is generally not what
   *  you want (for all but the most simple classes), we usually 
   *  put the declarations of these methods in the private section
   *  and never implement them. This prevents the compiler from
   *  implementing an incorrect "default" behavior without us
   *  knowing. (See Scott Meyers book, "Effective C++")
   *  
   */
  //@{
  //  MyNLP();
  MyNLP(const MyNLP&);
  MyNLP& operator=(const MyNLP&);
  //@}


  CPlaceDB* m_pDB;
  static void UpdateBlockPosition( const vector<double>& x, MyNLP* pNLP, int index1=0, int index2=INT_MAX );
  void UpdateBlockOrientation(); // (kaie) macro rotation
  static void BoundX( const int& n, vector<double>& x, vector<double>& x_l, vector<double>& x_h, int index1=0, int index2=INT_MAX );
  inline void BoundX( const int& n, vector<double>& x, vector<double>& x_l, vector<double>& x_h, const int& i );
  void LineSearch( const int& n, /*const*/ vector<double>& x, vector<double>& grad_f, double& stepSize );
  void LineSearchR( const int& n, vector<double>& f_r, double& stepSizeR ); // (kaie) macro rotation
  bool AdjustForce( const int& n, const vector<double>& x, vector<double>& grad_f );
  bool AdjustForce( const int& n, const vector<double>& x, vector<double> grad_wl, vector<double> grad_potential );
  bool WeightedForce( const int& n, vector<double>& f, vector<double> weight); // (kaie) macro rotation
  bool WeightedForce( const int& n, vector<double>& f1, vector<double>& f2, vector<double> weight); // (kaie) macro rotation
  bool AdjustForceR( const int& n, vector<double> grad_rotate, vector<double> grad_potentialR ); // (kaie) macro rotation
  bool AdjustForceR( const int& n, vector<double> grad_rotate, vector<double> grad_potentialR, vector<double> weigthR ); // (kaie) macro rotation
  bool FindBeta( const int& n, const vector<double>& grad_f, const vector<double>& last_grad_f, double& beta );
  // (kaie) change double to double 
  double _weightWire;
  double _weightDensity;
  double m_stepSize;
  double m_stepSizeR; // macro rotation
  double m_beta;    // CG step size
  double m_beta_r;  // macro rotation
  vector<double> x;
  vector<double> pinX;
  vector<double> grad_f;
  vector<double> last_grad_f;
  vector<double> walk_direction;
  vector<double> last_walk_direction;
  vector<double> xBest;
  vector<double> x_l;
  vector<double> x_u;
  vector<double> _expX;
  vector<double> _expPins;
  vector<double> grad_f_r;
  vector<double> last_grad_f_r;
  vector<double> walk_direction_r;
  vector<double> last_walk_direction_r;
  // @(kaie) change double to double  

  vector<double> grad_wire; // (kaie) change double to double
  vector<double> grad_potential; // (kaie) change double to double
  int m_ite;
  double m_currentStep;

  // (kaie) macro rotation
  vector<double> grad_rotate;
  vector<double> grad_potentialR;
  vector<double> rotate_degree;
  vector<double> rotate_off;
  vector<double> pin_position;
  vector<double> weightF;
  // @(kaie) macro rotation

  // 2007-07-10 (donnie) speedup test
  //vector< vector< double > > m_binGradX;
  //vector< vector< double > > m_binGradY;
  //void ComputeBinGrad();

  //Brian 2007-04-18
  //Calculate Net Weight in NLP
  static double NetWeightCalc(int netDegree);
  
public:
  double m_incFactor;
  double m_weightWire;

private:
  // threads 2006-06-14
  static void*  UpdateExpValueForEachCellThread( void* arg );
  static void*  UpdateExpValueForEachPinThread( void* arg ); 
  static void*  UpdateExpValueForCellAndPinThread( void* arg );
  static void*  UpdateNetsSumExpThread( void* arg );
  static void*  UpdateGradPotentialThread( void* arg );  // TODO
  static void*  UpdateGradWireThread( void* arg );
  static void*  BoundXThread( void* arg ); 
  static void*  ComputeNewPotentialGridThread( void* arg );
  static void*  eval_grad_f_thread( void* arg );
  static void*  GetLogSumExpWLThread( void* arg );
  static void*  UpdateBlockPositionThread( void* arg );
  static void*  UpdateXThread( void* arg );
  static void*  UpdateRThread( void* arg ); // (kaie) macro rotation
  static void*  UpdateGradThread( void* arg );          // TODO
  static void*  UpdateGradRThread( void* arg ); // (kaie) macro rotation
  static void*  FindGradL2NormThread( void* arg );  // in FindBeta
  static void*  FindGradProductThread( void* arg ); // in FindBeta
  static void*  UpdateMaxMinPinForEachNetThread( void* arg ); // (kaie) 2009-09-23 Stable-LSE
  static void*  UpdateExpValueForEachNetThread( void* arg ); // (kaie) 2009-09-23 Stable-LSE
  
  // wirelength related functions 
  static void calc_sum_exp_using_pin( 
	  const vector<int>::const_iterator& begin, const vector<int>::const_iterator& end,
	  const vector<double>& x, const vector<double>& expX, const vector<double>& expXnet_n,
	  double& sum_exp_xi_over_alpha, double& sum_exp_inv_xi_over_alpha,
	  double& sum_exp_yi_over_alpha, double& sum_exp_inv_yi_over_alpha, 
	  const CPlaceDB* pDB, const vector<bool>* pUsePin, const vector<double>& expPins,
	  int id=-1 );

  // (kaie) 2011-05-11 Weighted-Average-Exponential Model
  static void calc_weighted_sum_exp_using_pin(
	  const vector<int>::const_iterator& begin, const vector<int>::const_iterator& end,
	  const vector<double>& x, const vector<double>& expX, const vector<double>& expXnet_n,
	  double& weighted_sum_exp_xi_over_alpha, double& weighted_sum_exp_inv_xi_over_alpha,
	  double& weighted_sum_exp_yi_over_alpha, double& weighted_sum_exp_inv_yi_over_alpha,
	  const CPlaceDB* pDB, const vector<bool>* pUsePin, const vector<double>& expPins,
	  int id=-1 );

  static double GetLogSumExpWL( const vector<double>& x, const vector<double>& expX, const double& alpha, 
	  MyNLP* pNLP, int index1=0, int index2=INT_MAX );

  static void   UpdateNetsSumExp( const vector<double>& x, const vector<double>& expX,
	  const double& inAlpha, MyNLP* pNLP, int index1=0, int index2=INT_MAX );
  static void   UpdateExpValueForEachCell( const int& n, const vector<double>& x, 
					    vector<double>& expX, const double& alpha, 
					    const int& startN=0, MyNLP* pNLP=NULL );
  static void   UpdateExpValueForEachPin( const int& n, const vector<double>& x, 
					    vector<double>& expPins, const double& alpha,
					    MyNLP* pNLP, const vector<bool>* pUsePin, const int& startN=0 );
  static void   UpdateMaxMinPinForEachNet( const vector<double>& x, MyNLP* pNLP, const vector<bool>* pUsePin, int index1 = 0, int index2 = INT_MAX ); // (kaie) 2009-09-23 Stable-LSE
  static void   UpdateExpValueForEachNetStable( const vector<double>& x, const double& inAlpha, MyNLP* pNLP, const vector<bool>* pUsePin, int index1 = 0, int index2 = INT_MAX); // (kaie) 2009-09-23 Stable-LSE

  double m_posScale;
  
  double _alpha;
  vector<bool> m_usePin;
  void SetUsePin(); 
  vector<double> m_nets_sum_exp_xi_over_alpha;
  vector<double> m_nets_sum_exp_yi_over_alpha;
  vector<double> m_nets_sum_exp_inv_xi_over_alpha;
  vector<double> m_nets_sum_exp_inv_yi_over_alpha;
  void InitModuleNetPinId();
  vector< vector<int> > m_moduleNetPinId;

  // 2011-05-11 (kaie) Weighted-Average-Exponential Wirelength Model
  vector<double> m_nets_weighted_sum_exp_xi_over_alpha;
  vector<double> m_nets_weighted_sum_exp_yi_over_alpha;
  vector<double> m_nets_weighted_sum_exp_inv_xi_over_alpha;
  vector<double> m_nets_weighted_sum_exp_inv_yi_over_alpha;
  vector<double> sum_delta_x;
  vector<double> sum_delta_inv_x;
  vector<double> sum_delta_y;
  vector<double> sum_delta_inv_y;

  void ComputeNetSumDelta();

  // 2009-09-23 (kaie) Stable-LSE
  vector<double> m_nets_max_xi;
  vector<double> m_nets_min_xi;
  vector<double> m_nets_max_yi;
  vector<double> m_nets_min_yi;
  vector< vector<double> > expXnet;
 
  vector<double> m_nets_sum_p_x_pos;
  vector<double> m_nets_sum_p_y_pos;
  vector<double> m_nets_sum_p_inv_x_pos;
  vector<double> m_nets_sum_p_inv_y_pos;
  vector<double> m_nets_sum_p_x_neg;
  vector<double> m_nets_sum_p_y_neg;
  vector<double> m_nets_sum_p_inv_x_neg;
  vector<double> m_nets_sum_p_inv_y_neg;
 
  // diffusion bin
  vector< vector<double> > m_binForceX;
  vector< vector<double> > m_binForceY;
  void UpdateBinForce();
  //void GetDiffusionGrad( const double* x, const int& i, double& gradX, double& gradY );
  
  // potential grid related variables/functions
  double m_alwaysOverPotential;
  vector< double > _cellPotentialNorm;		// cell potential normalization factor
  vector< double > _cellPotentialNormGradX; // normalization gradiant for x
  vector< double > _cellPotentialNormGradY; // normalization gradiant for y
  vector< vector<double> > m_gridPotential;
  double m_potentialGridWidth;
  double m_potentialGridHeight;
  double _potentialRY;
  double _potentialRX;
  double GetNonZeroGridPercent();
  double GetMaxPotential();		
  double GetAvgPotential();		
  double GetTotalOverPotential();   // 2006-03-01 
  void   OutputPotentialGrid( string filename );	// for gnuplot
  void   OutputDensityGrid( string filename );		// for gnuplot
  void   PrintPotentialGrid();          
  static void   GetPotentialGrad( const vector<double>& x, const int& i, double& gradX, double& gradY, double& gradR, MyNLP* pNLP ); // (kaie) add gradR for macro rotation

  // 2007-07-10 (donnie)
  //static void GetPotentialGradFast( const vector<double>& x, const int& i, double& gradX, double& gradY, MyNLP* pNLP );
  //void        GetPointPotentialGrad( double x, double y, double& gradX, double& gradY );

  void   CreatePotentialGrid();         
  void   ClearPotentialGrid();          
  static void   ComputeNewPotentialGrid( const vector<double>& x, MyNLP* pNLP, int index1=0, int index2=INT_MAX ); 
  void   UpdatePotentialGrid();         
  void   UpdatePotentialGridBase( const vector<double>& x );	// compute preplaced block potential
  void   LevelSmoothBasePotential( const double& delta );	        // 2006-03-04
  double GetXGrid( const int& gx );
  double GetYGrid( const int& gy );
  double GetDensityPanelty();
  void   GetClosestGrid( const double& x1, const double& y1, int& gx, int& gy );
  struct potentialStruct    
  {
      potentialStruct( const int& x, const int& y, const double& p ) 
	  : gx(x), gy(y), potential(p)
	  {}
      int gx;
      int gy;
      double potential;
  };
  vector< vector<potentialStruct> > m_newPotential;  
  double UpdateExpBinPotential( double util, bool showMsg ); 
  vector< vector<double> > m_basePotential;	// 2006-03-03 (donnie) smoothed preplaced block potential 
  vector< vector<double> > m_binFreeSpace;	// 2006-03-16 (donnie) free space in the bin 
  vector< vector<double> > m_expBinPotential;	// 2006-03-14 (donnie) max expected potential
  void SmoothBasePotential();			// 2007-04-02 (donnie) Smooth the base potential

  static void UpdateGradWire( MyNLP* pNLP, int index1=0, int index2=INT_MAX );		// called in eval_grad_f
  static void UpdateGradPotential( MyNLP* pNLP, int index1=0, int index2=INT_MAX );	// called in eval_grad_f

  // (kaie) macro rotation
  static inline double GetDensityProjection( const double& t1, const double& t2 );
  static inline double GetGradDensityProjection( const double& t1, const double& t2 ); 
  // @(kaie) macro rotation
  
  // bell-shaped functions
  static inline double GetPotential( const double& x1, const double& x2, const double& r, const double& w, const double& alpha );
  static inline double GetGradPotential( const double& x1, const double& x2, const double& r, const double& w, const double& alpha );
  
  // sigmoid functions
  static inline double Sigmoid(double alpha, double t);
  static inline double SigmoidGrad(double alpha, double t);
  static inline double GetSigmoidPotential( const double& x1, const double& x2, const double& r, const double& w, const double& alpha );
  static inline double GetSigmoidGradPotential( const double& x1, const double& x2, const double& r, const double& w, const double& alpha );

  // density grid related functions
  vector< vector<double> > m_gridDensity;
  vector< vector<double> > m_gridDensitySpace;	// avaliable space in the bin
  double m_alwaysOverArea;
  double m_totalMovableModuleArea;
  double m_totalFreeSpace;
  double m_gridDensityWidth;
  double m_gridDensityHeight;
  void   UpdateDensityGrid( const vector<double>& x );
  void   UpdateDensityGridSpace( const vector<double>& x );
  void   CheckDensityGrid();
  void   CreateDensityGrid( int xGrid, int yGrid );
  void   ClearDensityGrid();
  double GetDensityGridPanelty();
  double GetMaxDensity();
  double GetAvgOverDensity();
  double GetTotalOverDensity();
  double GetTotalOverDensityLB();
  double GetNonZeroDensityGridPercent();

  // 2006-06-14
  struct ThreadInfo
  {
      vector<double>* pX;
      vector<double>* pExpX;
      double          alpha;
      int             index1;
      int             index2;
      int             index3;
      int             index4;
      int             index5;
      int             index6;
      CPlaceDB*       pDB;
      vector<bool>*   pUsePin;
      MyNLP*          pNLP;
      int             threadId;
  };
  vector< ThreadInfo > m_threadInfo;
  vector< double > m_threadResult;
  double Parallel( void* (func)(void* arg), int max, int max2=0, int max3=0 );

  // 2006-09-23
private:
  double m_precision;
  double m_weightIncreaseFactor;
  double m_targetUtil;
  double m_binSize;
  bool   m_bRunLAL;
  static double m_yWeight;

  // 2007-03-29 (donnie)
private:
  GRouteMap* m_pGRMap;

  // 2007-04-02 (donnie)
  //void PlotGPFigures( char* );

  // 2007-07-10 (donnie)
  void PlotBinForceStrength();
  bool InitObjWeights( double wWire );    // false is overflow
  void UpdateObjWeights();
  static vector< vector< double > > m_weightDensity;

public:
  int  m_maxIte;
};

#endif
