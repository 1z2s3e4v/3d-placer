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
#include <climits> // kaie
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
  //bool get_bounds_info(int n, float* x_l, float* x_u, int m, double* g_l, double* g_u);
  //bool get_bounds_info(int n, double* x_l, double* x_u, int m, double* g_l, double* g_u);
  //bool get_starting_point(int n, bool init_x, double* x, bool init_z, double* z_L, double* z_U,
  //                        int m, bool init_lambda, double* lambda);
  bool get_starting_point( vector<double>& x, vector<double>& z);
  bool eval_f(int n, const vector<double>& x, const vector<double>& expX, bool new_x, double& obj_value);
  bool eval_f_HPWL(int n, const vector<double>& x, const vector<double>& expX, bool new_x, double& obj_value);
  bool eval_grad_f( int n, const vector<double>& x, const vector<double>& expX, bool new_x, vector<double>& grad_f);
  
  // return true is placement is legal
  bool MySolve( double, double target_density, int currentLevel );	// solver setup
  bool BlockMoving();
  int LockBlocks();
 
  int _potentialGridR;
  int m_potentialGridPadding; // 2006-10-18
  int m_potentialGridSize;
  bool   m_lookAheadLegalization;
  bool   m_earlyStop;
  bool   m_topLevel;
  int    m_smoothR;
  bool   m_lastNLP;
  bool   m_useBellPotentialForPreplaced;
  double m_smoothDelta;
  
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
  vector< pair<int,int> > _cellPair;
  static void UpdateBlockPosition( const vector<double>& x, const vector<double>& z, MyNLP* pNLP, int index1=0, int index2=INT_MAX );
  static void BoundX( const int& n, vector<double>& x, vector<double>& x_l, vector<double>& x_h, int index1=0, int index2=INT_MAX );
  inline void BoundX( const int& n, vector<double>& x, vector<double>& x_l, vector<double>& x_h, const int& i );
  static void BoundZ( const int& n, vector<double>& z, const double& z_l, const double& z_u, int index1 = 0, int index2 = INT_MAX ); // kaie 2009-09-10
  void LayerAssignment();
  static void LayerAssignment( const int& n, vector<double>& z, MyNLP* pNLP, int index1 = 0, int index2 = INT_MAX );
  void LineSearch( const int& n, /*const*/ vector<double>& x, vector<double>& grad_f, double& stepSize );
  bool AdjustForce( const int& n, const vector<double>& x, vector<double>& grad_f );
  bool AdjustForce( const int& n, const vector<double>& x, vector<double> grad_wl, vector<double> grad_potential, vector<double> grad_potentialZ );
  bool FindBeta( const int& n, const vector<double>& grad_f, const vector<double>& last_grad_f, double& beta );
  double _weightWire;
  double _weightDensity;
  double _weightVia; // kaie
  double m_stepSizeX, m_stepSizeY, m_stepSizeZ; // kaie
  double m_stepSize;
  double m_beta;    // CG step size
  vector<bool> cellLock;
  vector<double> x;
  vector<double> grad_f;
  vector<double> last_grad_f;
  vector<double> walk_direction;
  vector<double> last_walk_direction;
  vector<double> xBest;
  vector<double> x_l;
  vector<double> x_u;
  vector<double> _expX;
  vector<double> _expPins;
  vector<double> xMax;
  vector<double> yMax;
  vector<double> zMax;
  
  // kaie 2009-08-29
  vector<double> z;
  vector<double> zBest;
  vector<double> _expZ;
  vector<double> _expPinsZ;
  // @kaie 2009-08-29
  
  // 2006-09-12 (donnie) for X-HPWL
  vector<double> _expXplusY;      // exp( x+y / k )   for cells
  vector<double> _expXminusY;     // exp( x-y / k )   for cells
  vector<double> _expPinXplusY;   // exp( x+y / k )   for pins
  vector<double> _expPinXminusY;  // exp( x-y / k )   for pins
 
 /* 
  double* x; 
  double* grad_f;
  double* last_grad_f;
  //double* xBest;   // look ahead legalization 
  float* xBest;   // look ahead legalization 
  //double* x_l;	   // lower bound
  //double* x_u;     // upper bound
  float* x_l;	   // lower bound
  float* x_u;     // upper bound
  double* _expX;   // exp(x)
  double* _expPins;
  double* xBak;
  double* xBak2;
  */
  vector<double> grad_wire;
  vector<double> grad_potential;
  vector<double> grad_potentialZ;
  int m_ite;
  double m_currentStep;
  double m_currentStepZ;

  // kaie
  vector<double> grad_via;
  // @kaie

  // 2007-07-10 (donnie) speedup test
  // 2009-09-10 (kaie) modified for 3d
  vector< vector< vector< double > > > m_binGradX;
  vector< vector< vector< double > > > m_binGradY;
  vector< vector< vector< double > > > m_binGradZ;
  void ComputeBinGrad();
  
  //Brian 2007-04-18
  //Calculate Net Weight in NLP
  static double NetWeightCalc(int netDegree);
  
public:
  double m_incFactor;
  double m_weightWire;
  double m_weightTSV; // (kaie) 2009-09-17

private:
  // threads 2006-06-14
  static void*  UpdateExpValueForEachCellThread( void* arg );
  static void*  UpdateExpValueForEachPinThread( void* arg ); 
  static void*  UpdateExpValueForCellAndPinThread( void* arg );
  static void*  UpdateNetsSumExpThread( void* arg );
  static void*  UpdateGradPotentialThread( void* arg );  // TODO
  static void*  UpdateGradWireThread( void* arg );

  // kaie 2009-08-29
  static void*  UpdateGradViaThread( void* arg );
  static void*  GetLogSumExpViaThread( void* arg );
  // @kaie 2009-08-29

  static void*  LayerAssignmentThread( void* arg );
  static void*  BoundZThread( void* arg ); // kaie 2009-09-10
  static void*  BoundXThread( void* arg ); 
  static void*  ComputeNewPotentialGridThread( void* arg );
  static void*  eval_grad_f_thread( void* arg );
  static void*  UpdateNLPDataThread( void* arg );
  static void*  GetLogSumExpWLThread( void* arg );
  static void*  UpdateBlockPositionThread( void* arg );
  static void*  UpdateXThread( void* arg );
  static void*  UpdateGradThread( void* arg );          // TODO
  static void*  FindGradL2NormThread( void* arg );  // in FindBeta
  static void*  FindGradProductThread( void* arg ); // in FindBeta
  //vector< vector< sem_t > > m_binSemaphore;
  //pthread_mutex_t work_mutex;
  
  // wirelength related functions 
  static void calc_sum_exp_using_pin( 
	  const vector<int>::const_iterator& begin, const vector<int>::const_iterator& end,
	  const vector<double>& x, const vector<double>& z, const vector<double>& expX, const vector<double>& expZ,
	  double& sum_exp_xi_over_alpha, double& sum_exp_inv_xi_over_alpha,
	  double& sum_exp_yi_over_alpha, double& sum_exp_inv_yi_over_alpha,
	  double& sum_exp_zi_over_alpha, double& sum_exp_inv_zi_over_alpha, 
	  const CPlaceDB* pDB, const vector<bool>* pUsePin, const vector<double>& expPins, const vector<double>& expPinsZ,
	  int id=-1 );

  // (kaie) 2010-10-18 Weighted-Average-Exponential Model
  static void calc_weighted_sum_exp_using_pin(
  	  const vector<int>::const_iterator& begin, const vector<int>::const_iterator& end,
  	  const vector<double>& x, const vector<double>& expX,
  	  double& weighted_sum_exp_xi_over_alpha, double& weighted_sum_exp_inv_xi_over_alpha,
  	  double& weighted_sum_exp_yi_over_alpha, double& weighted_sum_exp_inv_yi_over_alpha,
  	  const CPlaceDB* pDB, const vector<bool>* pUsePin, const vector<double>& expPins,
  	  int id=-1 );

  // (frank) 2022-07-22 3D Weighted-Average-Exponential Wirelength Model
  static void calc_weighted_sum_exp_using_pin_for_layers(
  	  const vector<int>::const_iterator& begin, const vector<int>::const_iterator& end,
  	  const vector<double>& x, const vector<double>& expX,
      const vector<double>& z, const vector<double>& expZ,
  	  vector<double>& layer_weighted_sum_exp_xi_over_alpha, vector<double>& layer_weighted_sum_exp_inv_xi_over_alpha,
  	  vector<double>& layer_weighted_sum_exp_yi_over_alpha, vector<double>& layer_weighted_sum_exp_inv_yi_over_alpha,
  	  const CPlaceDB* pDB, const vector<bool>* pUsePin, const vector<double>& expPins,
  	  int id=-1);

  // static 2006-09-12 (donnie)
  static void calc_sum_exp_using_pin_XHPWL( 
	const vector<int>::const_iterator& begin, const vector<int>::const_iterator& end,
	const MyNLP* pNLP,
	double& sum_exp_x_plus_y_over_alpha,     double& sum_exp_x_minus_y_over_alpha,
	double& sum_exp_inv_x_plus_y_over_alpha, double& sum_exp_inv_x_minus_y_over_alpha );
  
  static double GetLogSumExpWL( const vector<double>& x, const vector<double>& expX, const double& alpha, 
	  MyNLP* pNLP, int index1=0, int index2=INT_MAX );

  // (kaie) 2009-10-14
  static double GetLogSumExpVia( const vector<double>& z, const vector<double>& expZ, const double& alpha,
	  MyNLP* pNLP, int index1=0, int index2=INT_MAX );
  // @(kaie) 2009-10-14

  static void   UpdateNetsSumExp( const vector<double>& x, const vector<double>& z, const vector<double>& expX,
		  const vector<double>& expZ, MyNLP* pNLP, int index1=0, int index2=INT_MAX );
  static void   UpdateExpValueForEachCell( const int& n, const vector<double>& x, const vector<double>& z, 
					    vector<double>& expX, vector<double>& expZ, const double& alpha, 
					    const int& startN=0, MyNLP* pNLP=NULL );
  static void   UpdateExpValueForEachPin( const int& n, const vector<double>& x, const vector<double>& z,
					    vector<double>& expPins, vector<double>& expPinsZ, const double& alpha,
					    MyNLP* pNLP, const vector<bool>* pUsePin, const int& startN=0 );

  // LP-norm 2006-06-26
  //static double GetLPNormWL( MyNLP* pNLP, int index1=0, int index2=INT_MAX );
  //static void   UpdateNetsSumP( MyNLP* pNLP, int index1=0, int index2=INT_MAX );
  //static void   UpdatePValueForEachCell( const int& n, const vector<double>& x, vector<double>& expX, const double& alpha, const int& startN=0 );
  //static void   UpdatePValueForEachPin( const int& n, const vector<double>& x, vector<double>& expPins, const double& alpha, 

  double m_posScale;
  
  double _alpha;
  vector<bool> m_usePin;
  void SetUsePin(); 
  vector<double> m_nets_sum_exp_xi_over_alpha;
  vector<double> m_nets_sum_exp_yi_over_alpha;
  vector<double> m_nets_sum_exp_zi_over_alpha; // kaie 2009-08-29
  vector<double> m_nets_sum_exp_inv_xi_over_alpha;
  vector<double> m_nets_sum_exp_inv_yi_over_alpha;
  vector<double> m_nets_sum_exp_inv_zi_over_alpha; // kaie 2009-08-29
  // 3d hpwl (frank 2022-07-20)
  vector<vector<double> > m_layer_nets_sum_exp_xi_over_alpha;
  vector<vector<double> > m_layer_nets_sum_exp_yi_over_alpha;
  vector<vector<double> > m_layer_nets_sum_exp_zi_over_alpha; 
  vector<vector<double> > m_layer_nets_sum_exp_inv_xi_over_alpha;
  vector<vector<double> > m_layer_nets_sum_exp_inv_yi_over_alpha;
  vector<vector<double> > m_layer_nets_sum_exp_inv_zi_over_alpha; 

  void InitModuleNetPinId();
  vector< vector<int> > m_moduleNetPinId;

  // 2010-10-18 (kaie) Weighted-Average-Exponential Wirelength Model
  vector<double> m_nets_weighted_sum_exp_xi_over_alpha;
  vector<double> m_nets_weighted_sum_exp_yi_over_alpha;
  vector<double> m_nets_weighted_sum_exp_inv_xi_over_alpha;
  vector<double> m_nets_weighted_sum_exp_inv_yi_over_alpha;
  // 3d hpwl (frank 2022-07-20)
  vector<vector<double> > m_layer_nets_weighted_sum_exp_xi_over_alpha;
  vector<vector<double> > m_layer_nets_weighted_sum_exp_yi_over_alpha;
  vector<vector<double> > m_layer_nets_weighted_sum_exp_inv_xi_over_alpha;
  vector<vector<double> > m_layer_nets_weighted_sum_exp_inv_yi_over_alpha;

  // 2006-09-12 (donnie) for X-HPWL
  vector<double> m_nets_sum_exp_x_plus_y_over_alpha;
  vector<double> m_nets_sum_exp_x_minus_y_over_alpha;
  vector<double> m_nets_sum_exp_inv_x_plus_y_over_alpha;
  vector<double> m_nets_sum_exp_inv_x_minus_y_over_alpha;
  
  vector<double> m_nets_sum_p_x_pos;
  vector<double> m_nets_sum_p_y_pos;
  vector<double> m_nets_sum_p_inv_x_pos;
  vector<double> m_nets_sum_p_inv_y_pos;
  vector<double> m_nets_sum_p_x_neg;
  vector<double> m_nets_sum_p_y_neg;
  vector<double> m_nets_sum_p_inv_x_neg;
  vector<double> m_nets_sum_p_inv_y_neg;

  // kaie 2009-08-29
  vector<double> m_nets_sum_p_z_pos;
  vector<double> m_nets_sum_p_inv_z_pos;
  vector<double> m_nets_sum_p_z_neg;
  vector<double> m_nets_sum_p_inv_z_neg;
  // @kaie 2009-08-29
  
  // diffusion bin
  vector< vector<double> > m_binForceX;
  vector< vector<double> > m_binForceY;
  void UpdateBinForce();
  //void GetDiffusionGrad( const double* x, const int& i, double& gradX, double& gradY );
  
  // potential grid related variables/functions
  double m_alwaysOverPotential;
  vector< double > _cellPotentialNorm;		// cell potential normalization factor
  vector< vector< vector<double> > > m_gridPotential;
  double m_potentialGridWidth;
  double m_potentialGridHeight;
  double m_potentialGridThickness;
  double _potentialRY;
  double _potentialRX;
  double _potentialRZ; // (kaie) 2009-09-14
  //double _expPotential;
  double GetNonZeroGridPercent();
  double GetMaxPotential();		
  double GetAvgPotential();		
  double GetTotalOverPotential();   // 2006-03-01 
  void   OutputPotentialGrid( string filename, const int& layer);	// for gnuplot
  void   OutputDensityGrid( string filename, const int& layer);		// for gnuplot
  void   PrintPotentialGrid();          
  static void   GetPotentialGrad( const vector<double>& x, const vector<double>& z, const int& i, double& gradX, double& gradY, double& gradZ, MyNLP* pNLP );
  
  // 2007-07-10 (donnie)
  static void GetPotentialGradFast( const vector<double>& x, const vector<double>& z, const int& i, double& gradX, double& gradY, double& gradZ, MyNLP* pNLP );
  void        GetPointPotentialGrad( double x, double y, double z, double& gradX, double& gradY, double& gradZ);
  
  void   CreatePotentialGrid();         
  void   ClearPotentialGrid();          
  static void   ComputeNewPotentialGrid( const vector<double>& x, const vector<double>& z, MyNLP* pNLP, int index1=0, int index2=INT_MAX ); 
  static inline double GetDensityProjection(const double& z, const double& k); // (kaie) 2009-09-10 3d projection
  static inline double GetGradDensityProjection(const double& z, const double& k);
  void   UpdatePotentialGrid(const vector<double>& z);
  void   UpdatePotentialGridBase( const vector<double>& x, const vector<double>& z);	// compute preplaced block potential
  // (kaie) 2009-09-10 modified for 3d
  void   LevelSmoothBasePotential( const double& delta, const int& layer);	        // 2006-03-04
  void   LevelSmoothBasePotential3D( const double& delta);
  double GetGridWidth();
  void   GetGridCenter( const int& gx, const int& gy, double& x1, double& y1 );
  double GetXGrid( const int& gx );
  double GetYGrid( const int& gy );
  double GetZGrid( const int& gz ); // (kaie) 2009-09-18
  double GetDensityPanelty();
  double GetPotentialToGrid( const double& x1, const int& gx );         // 1D
  double GetGradPotentialToGrid( const double& x1, const int& gx );	// 1D
  void   GetClosestGrid( const double& x1, const double& y1, int& gx, int& gy );
  void   GetClosestGrid( const double& x1, const double& y1, const double& z1, int& gx, int& gy, int& gz ); // (kaie) 2009-09-18 add z
  struct potentialStruct    
  {
      potentialStruct( const int& x, const int& y, const int& z, const double& p ) 
	  : gx(x), gy(y), gz(z), potential(p)
	  {}
      potentialStruct( const int& x, const int& y, const double& p )
	  : gx(x), gy(y), potential(p)
	  {}
      int gx;
      int gy;
      int gz;
      double potential;
  };
  vector< vector<potentialStruct> > m_newPotential;  
  double UpdateExpBinPotential( double util, bool showMsg);
  void UpdateExpBinPotentialTSV( bool showMsg ); // kaie 2009-11-11
  // kaie modified for 3d 
  vector< vector < vector<double> > > m_basePotential;	// 2006-03-03 (donnie) smoothed preplaced block potential 
  vector< vector< vector<double> > > m_binFreeSpace;	// 2006-03-16 (donnie) free space in the bin 
  vector< vector< vector<double> > > m_expBinPotential;	// 2006-03-14 (donnie) max expected potential
  vector< vector<double> > m_congPotential;	// 2007-04-02 (donnie) Congestion potential
  void SmoothBasePotential3D();
  void SmoothBasePotential(const int& layer);			// 2007-04-02 (donnie) Smooth the base potential

  static void UpdateGradWire( MyNLP* pNLP, int index1=0, int index2=INT_MAX );		// called in eval_grad_f
  static void UpdateGradPotential( MyNLP* pNLP, int index1=0, int index2=INT_MAX );	// called in eval_grad_f
 
  // kaie 2009-08-29
  static void UpdateGradVia( MyNLP* pNLP, int index1=0, int index2=INT_MAX );
  // @kaie 2009-08-29
  
  //Brian 2007-04-30

  struct NetBoxInfo
  {
        NetBoxInfo():
        m_dCenterX(0), m_dCenterY(0), m_dArea(0), m_dWidth(0), m_dHeight(0), m_dNorm(0), m_dDensity(0){}
        double m_dCenterX;
        double m_dCenterY;
        double m_dArea;
        double m_dWidth;
        double m_dHeight;
        double m_dNorm;
        double m_dDensity;
  };

  vector< vector<potentialStruct> > m_newPotentialNet;
  vector< vector<double> > m_gridPotentialNet;
  vector< vector<double> > m_gridDensityNet;

  //Brian 2007-07-23 Base Potential for macro
  vector< vector<double> > m_gridDensitySpaceNet;
  vector< vector<double> > m_basePotentialNet;	
  //vector< vector<double> > m_binFreeSpaceNet;
  vector< vector<double> > m_expBinPotentialNet;
  double m_dMacroOccupy;//If m_dMacroOccupy = 3, a macro with 3 row height occupies one routing layer
  //@Brian 2007-07-23 Base Potential for macro 
  
  vector< NetBoxInfo > m_NetBoxInfo;
  vector<double> grad_congestion;

  //Added by Jin 20081014
  vector<double> grad_timing;

  double m_dAvgPitch;
  double m_dLevelNum;
  double m_dDensityNet;
  double m_dWeightCong;
  double m_alwaysOverPotentialNet;
  double m_alwaysOverDensityNet;
  double m_dTotalNetDensity;
  
  double m_dCongUtil;

  void CreatePotentialGridNet();
  void CreateDensityGridNet(int nGrid);
  // void CatchCongParam();

  void ComputeNetBoxInfo();
  void ComputeNewPotentialGridNet();

  void UpdatePotentialGridNet();
  void UpdateDensityGridNet();
  //Brian 2007-07-23 Base Potential for macro 
  void UpdateDensityGridSpaceNet();
  //@Brian 2007-07-23 Base Potential for macro 
  void GetDensityPaneltyNet();
  void ClearPotentialGridNet();
  void ClearDensityGridNet();

  //Brian 2007-07-23 Base Potential for macro 
  void SmoothBasePotentialNet();
  void UpdateExpBinPotentialNet();
  void UpdatePotentialGridBaseNet();
  //@Brian 2007-07-23 Base Potential for macro 

  void UpdateGradCong();
  void GetCongGrad(const int& i, double& gradX, double& gradY);
  bool AdjustForceNet( vector<double>& grad_wl, vector<double>& grad_potential, vector<double>& grad_congestion );

  double GetTotalOverDensityNet();
  double GetTotalOverPotentialNet();
  double GetMaxDensityNet();

  void   CongSmooth(int moduleId, int netId, int pinId, double& smoothX, double& smoothY);
 
  //@Brian 2007-04-30
  
  
  // bell-shaped functions
  static inline double GetPotential( const double& x1, const double& x2, const double& r );
  static inline double GetPotential( const double& x1, const double& x2, const double& r, const double& w );
  static inline double GetGradPotential( const double& x1, const double& x2, const double& r );
  static inline double GetGradPotential( const double& x1, const double& x2, const double& r, const double& w );
  static inline double GetGradGradPotential( const double& x1, const double& x2, const double& r ); 

  
  // density grid related functions
  // (kaie) 2009-09-10 modified for 3d
  vector< vector< vector<double> > > m_gridDensity;
  vector< vector< vector<double> > > m_gridDensitySpace;	// avaliable space in the bin
  double m_alwaysOverVolumn;
  //double m_totalMovableModuleArea;
  //double m_totalFixedModuleArea;
  double m_totalFreeSpace;
  double m_gridDensityWidth;
  double m_gridDensityHeight;
  double m_gridDensityThickness;
  double m_gridDensityTarget;
  void   UpdateDensityGrid( const int& n, const vector<double>& x, const vector<double>& z);
  void   UpdateDensityGridSpace( const int& n, const vector<double>& x, const vector<double>& z);
  void   CheckDensityGrid();
  void   CreateDensityGrid( int nGrid );
  void   ClearDensityGrid();
  double GetDensityGridPanelty();
  double GetMaxDensity();
  double GetAvgOverDensity();
  double GetTotalOverDensity();
  double GetTotalOverDensityLB();
  double GetNonZeroDensityGridPercent();
  void   GetDensityGrad( const vector<double>& x, const int& i, double& gradX, double& gradY ); // UNUSE


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

      // kaie 2009-08-29
      vector<double>* pZ;
      vector<double>* pExpZ;
      // @kaie 2009-08-29
  };
  vector< ThreadInfo > m_threadInfo;
  vector< double > m_threadResult;
  double Parallel( void* (func)(void* arg), int max, int max2=0, int max3=0 );

  // 2006-09-19 
  // Update the expBinPotential according to current congestion map
  // return total bin overflow value
  // double UpdateCongestion();
  // (kaie) 2009-09-10 3d
  vector< vector< vector<double> > > m_expBinPotentialOld;
  vector< vector< vector<double> > > m_basePotentialOld; 
  // double UpdateCongestionBasePotential(); 

  // 2006-09-23
private:
  double m_precision;
  double m_weightIncreaseFactor;
  double m_targetUtil;
  double m_binSize;
  double m_congWeight;
  bool   m_bRunLAL;
  static bool m_bXArch; 
  static double m_yWeight;
  bool   m_useEvalF;

  // 2007-01-17
private:
  void ApplySteinerWireForce();
  void ComputeTotalForces( const unsigned int& blockNumber );
  void NullifySpreadingForces( double ratio );
  double m_spreadingForceNullifyRatio;

  // 2007-03-29 (donnie)
private:
  GRouteMap* m_pGRMap;

  // 2007-04-02 (donnie)
  // void PlotGPFigures( char* );

  // 2007-07-10 (donnie)
  void PlotBinForceStrength();
  bool InitObjWeights( double wWire );    // false is overflow
  void UpdateObjWeights();
  // 2009-09-10 (kaie) modified for 3d
  static vector< vector< vector< double > > > m_weightDensity;

public:
  int  m_maxIte;
  static double m_skewDensityPenalty1;
  static double m_skewDensityPenalty2;
  
public:

  bool m_lockBin;


  // 2009-11-13
  bool m_bMoveZ; 
};


#endif
