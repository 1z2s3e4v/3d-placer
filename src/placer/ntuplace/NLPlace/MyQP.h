#ifndef __MYQP_HPP__
#define __MYQP_HPP__


#include "placedb.h"

#include <set>
#include <vector>
#include <string>
using namespace std;


////MyQP modified by tchsu
//class MyQP_fixedPinGrid 
//{
//    MyQP_fixedPinGrid()
//    {
//        m_num_pins=0;
//    }
//    MyQP_fixedPinGrid(double x, double y)
//    {
//        m_cx=x;
//        m_cy=y;
//        m_num_pins=0;
//    }
//    ~MyQP_fixedPinGrid()
//    {
//    }
//
//    double m_cx; //x coordinate of grid center
//    double m_cy; //y coordinate of grid center
//    double m_num_pins; //# of fixed pins in this grid
//}
//class MacroInit
//{
//public:
//    MacroInit(CPlaceDB& db)
//    {
//        m_pDB = &db;
//    }
//    ~MacroInit(){}
//
//    void init();
//    void solve();
//    void safeMoveCenter(int i,double cx, double cy);
//    void findGridNum(  const double x, const double y, int& xIndex, int& yIndex);
//    void buildPinGrid(  int x, int y, vector< vector<double> >& fixPinGrid);    
//
//
//    CPlaceDB* m_pDB;
//    vector< vector<double> > m_FixedPinPotential; //Fixed-Pin potential
//    vector< int > m_macroIds;
//    int m_num_X_grid;
//    int m_num_Y_grid;
//    double m_gridW;
//    double m_gridH;
//    vector< double > m_stepX;
//    vector< double > m_stepY;
//
//    double getFixedPinPotential();
//    void addFixedPinForce();
//
//};
//class MyQP_fixedPinInfo 
//{
//public:
//    MyQP_fixedPinInfo()
//    {
//
//    }
//    ~MyQP_fixedPinInfo()
//    {
//    }
//
//    int m_num_X_grid;
//    int m_num_Y_grid;
//    double m_gridW;
//    double m_gridH;
//    void findGridNum( const CPlaceDB& db, const double x, const double y, int& xIndex, int& yIndex);
//    void buildPinGrid( const CPlaceDB& db, int x, int y, vector< vector<double> >& fixPinGrid);
//};
class MyQP_netInfo 
{
public:

    MyQP_netInfo()
    {
        m_leftPinId=-1;
        m_rightPinId=-1;
        m_topPinId=-1;
        m_bottomPinId=-1;
    }
    ~MyQP_netInfo(){}

    int m_leftPinId;
    int m_rightPinId;
    int m_topPinId;
    int m_bottomPinId;
    double width,height;
    vector<double> l_h;
    vector<double> l_v;
    map<int,int> m_pinId2index;
};


class MyQP 
{
public:
  MyQP( CPlaceDB& );
  virtual ~MyQP();

  bool get_nlp_info(int& n, int& m, int& nnz_jac_g, int& nnz_h_lag );
  bool get_bounds_info(int n, vector<double>& x_l, vector<double>& x_u);

  bool get_starting_point( vector<double>& x );
  bool eval_f(int n, const vector<double>& x,  bool new_x, double& obj_value);
//  bool eval_f_HPWL(int n, const vector<double>& x,bool new_x, double& obj_value);
  bool eval_grad_f( int n, const vector<double>& x, bool new_x, vector<double>& grad_f);
  
  // return true is placement is legal
  bool MySolve( double, double target_density, int currentLevel, bool noRelaxSmooth );	// solver setup
  bool BlockMoving();
  int LockBlocks();
  
  int _potentialGridR;
  int m_potentialGridSize;
  double m_targetUtil;
  double target_nnb;
  bool   m_lookAheadLegalization;
  bool   m_earlyStop;
  bool   m_topLevel;
  int    m_smoothR;
  bool   m_lastNLP;
  bool   m_useBellPotentialForPreplaced;
  double m_smoothDelta;
  bool   m_isFixedPinRepel;
  
private:
  bool GoSolve( double, double target_density, int currentLevel );	// real solver
  

  MyQP(const MyQP&);
  MyQP& operator=(const MyQP&);



  CPlaceDB* m_pDB;
  vector< pair<int,int> > _cellPair;
  void UpdateBlockPosition( const vector<double>& x );

  void BoundX( const int& n, vector<double>& x, vector<double>& x_l, vector<double>& x_h );
  inline void BoundX( const int& n, vector<double>& x, vector<double>& x_l, vector<double>& x_h, const int& i );
  void LineSearch( const int& n, /*const*/ vector<double>& x, vector<double>& grad_f, double& stepSize );
  void AdjustForce( const int& n, const vector<double>& x, vector<double>& grad_f );
  void AdjustForce( const int& n, const vector<double>& x, vector<double> grad_wl, vector<double> grad_potential );
  void FindBeta( const int& n, const vector<double>& grad_f, const vector<double>& last_grad_f, double& beta );
  double _weightWire;
  double _weightDensity;
  vector<bool> cellLock;
  vector<double> x;
  vector<double> grad_f;
  vector<double> last_grad_f;
  vector<double> xBest;
  vector<double> x_l;
  vector<double> x_u;


  //by tchsu
  vector<MyQP_netInfo> m_qp_netInfo;

 
  vector<double> grad_wire;
  vector<double> grad_potential;
  int m_ite;
  double m_currentStep;
  
public:
  double m_incFactor;
  double m_weightWire;

private:
  // wirelength related functions 

  double GetQpWL(  const vector<double>& x );

  void   UpdateNetsSum( const vector<double>& x );
  double _alpha;
  vector<bool> m_usePin;
  void SetUsePin(); 

  void InitModuleNetPinId();
  vector< vector<int> > m_moduleNetPinId;
  

 
  // diffusion bin
  vector< vector<double> > m_binForceX;
  vector< vector<double> > m_binForceY;
  //void UpdateBinForce();
  //void GetDiffusionGrad( const double* x, const int& i, double& gradX, double& gradY );
  
  // potential grid related variables/functions
  double m_alwaysOverPotential;
  vector< double > _cellPotentialNorm;		// cell potential normalization factor
  vector< vector<double> > m_gridPotential;
  double m_potentialGridWidth;
  double m_potentialGridHeight;
  double _potentialRY;
  double _potentialRX;
  //double _expPotential;
  double GetNonZeroGridPercent();
  double GetMaxPotential();
  double GetAvgPotential();
  double GetTotalOverPotential();   // 2006-03-01
  void   OutputPotentialGrid( string filename );	// for gnuplot

  void   OutputDensityGrid( string filename );		// for gnuplot
  void   PrintPotentialGrid();
  void   GetPotentialGrad( const vector<double>& x, const int& i, double& gradX, double& gradY );
  void   CreatePotentialGrid();
  void   ClearPotentialGrid();
  void   UpdatePotentialGrid( const vector<double>& x );
  void   UpdatePotentialGridBase( const vector<double>& x );	    // compute preplaced block potential
  void   SmoothPotentialBase( const double& delta );	    // 2006-03-04
  double GetGridWidth();
  void   GetGridCenter( const int& gx, const int& gy, double& x1, double& y1 );
  double GetXGrid( const int& gx );
  double GetYGrid( const int& gy );
  double GetDensityPanelty();
  double GetPotentialToGrid( const double& x1, const int& gx );         // 1D
  double GetGradPotentialToGrid( const double& x1, const int& gx );	// 1D
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
  void UpdateExpBinPotential( double utl );	// 2006-03-14
  vector< vector<double> > m_basePotential;	// 2006-03-03 (donnie) preplaced block potential 
  vector< vector<double> > m_binFreeSpace;	// 2006-03-16 (donnie) free space in the bin 
  vector< vector<double> > m_basePotentialOri;	// 2006-03-03 (donnie) preplaced block potential 
  vector< vector<double> > m_expBinPotential;	// 2006-03-14 (donnie) preplaced block potential 



  // bell-shaped functions
  inline double GetPotential( const double& x1, const double& x2, const double& r );
  inline double GetPotential( const double& x1, const double& x2, const double& r, const double& w );
  inline double GetGradPotential( const double& x1, const double& x2, const double& r );
  inline double GetGradPotential( const double& x1, const double& x2, const double& r, const double& w );
  inline double GetGradGradPotential( const double& x1, const double& x2, const double& r ); 

  
  // density grid related functions
  vector< vector<double> > m_gridDensity;
  vector< vector<double> > m_gridDensitySpace;	// avaliable space in the bin
  double m_alwaysOverArea;
  //double m_totalMovableModuleArea;
  //double m_totalFixedModuleArea;
  double m_totalFreeSpace;
  double m_gridDensityWidth;
  double m_gridDensityHeight;
  double m_gridDensityTarget;
  void   UpdateDensityGrid( const int& n, const vector<double>& x );
  void   UpdateDensityGridSpace( const int& n, const vector<double>& x );
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

  //fixed-pin force data & functions
  //void   OutputFixedPinPotentialGrid( string filename );	// for gnuplot by tchsu
  vector< vector<double> > m_FixedPinPotential; //Fixed-Pin potential

  int m_num_X_grid;
  int m_num_Y_grid;
  double m_gridW;
  double m_gridH;
  void findGridNum(  const double x, const double y, int& xIndex, int& yIndex);
  void buildPinGrid(  int x, int y, vector< vector<double> >& fixPinGrid);
  //double getFixedPinPotential();
  //void addFixedPinForce( vector<double>& grad_potential);

  vector< int > m_macroIds;
  vector< double > m_macroIdsAreaFactor;


};

//record the path data of 2 point (stores the bin ID)
class MyGlobalRouterPath
{
public:
    MyGlobalRouterPath()
    {
    }
    ~MyGlobalRouterPath(){}

    int m_startBinIdX;
    int m_startBinIdY;
    vector<int> m_cornerX;
    vector<int> m_cornerY;

};
class MyGlobalRouterBin
{
public:
    MyGlobalRouterBin()
    {
        m_blockageArea=0;
        m_binArea=0;
        m_nH_Track=0;
        m_nV_Track=0;
        m_H_used=0;
        m_V_used=0;

    }
    ~MyGlobalRouterBin(){}

    //double m_binWidth;
    //double m_binHeight;

    //double m_HblockageRate; //between 0~1
    //double m_VblockageRate; //between 0~1
    double m_blockageArea;
    double m_binArea;
    int m_nH_Track;
    int m_nV_Track;
    int m_H_used;
    int m_V_used;
};
class MyGlobalRouter
{
public:
    MyGlobalRouter( CPlaceDB& db, double w, double h)
    {
        m_pDB = &db;
        m_binWidth=w;
        m_binHeight=h;
        if( (((m_pDB->m_coreRgn.right-m_pDB->m_coreRgn.left)/w)- floor((m_pDB->m_coreRgn.right-m_pDB->m_coreRgn.left)/w))>0.1 )
        {
            m_nBinCol=(int)floor((m_pDB->m_coreRgn.right-m_pDB->m_coreRgn.left)/w)+1;
        }
        else
        {
            m_nBinCol=(int)floor((m_pDB->m_coreRgn.right-m_pDB->m_coreRgn.left)/w);
        }
        if( (((m_pDB->m_coreRgn.top-m_pDB->m_coreRgn.bottom)/h)- floor((m_pDB->m_coreRgn.top-m_pDB->m_coreRgn.bottom)/h))>0.1 )
        {
            m_nBinRow=(int)floor((m_pDB->m_coreRgn.top-m_pDB->m_coreRgn.bottom)/h)+1;
        }
        else
        {
            m_nBinRow=(int)floor((m_pDB->m_coreRgn.top-m_pDB->m_coreRgn.bottom)/h);
        }
        m_HtracksPerRowHeight=20; //m3 + m5 + m7, 8+8+4=20
        m_VtracksPerRowHeight=28; // m2 + m4 + m6 + m8 8+8+8+4=28
    }
    ~MyGlobalRouter(){};
    void initResource();
    void route();
    void outputfig( const char* prefix);
    void OutputHGrid( string filename );
    void OutputVGrid( string filename );
    double calcMacroSpacing(int mId1, int mId2, bool isHspacing); //return ideal H of V spacing between macros, return -1 if no overlapping, return 0 if the spacing is enough
/*------------------------------------------------------------
____        ___
| 1 | <--->| 2|
|___|      |__|
 
       H spacing=mr.calcMacroSpacing( 1, 2, true)
//----------------------------
___
| 1 |
| _ |
 ^
 |
 |
 v__
| 2 |
| _ |

       V spacing=mr.calcMacroSpacing( 1, 2, false)

------------------------------------------------------------*/
  


    int m_HtracksPerRowHeight; 
    int m_VtracksPerRowHeight;

private:

    void buildMST( const int& netID, vector<int>& parentPinIds );
    void findPath( const int& netID, vector<MyGlobalRouterPath>& paths);
    bool findPath2P( const int& pinID1, const int& pinID2, MyGlobalRouterPath& path);
    void addHcost( int x1, int x2, const int& y);
    void addVcost( int y1, int y2, const int& x);
    void getBinCenter(double& x, double& y, const int& bx, const int& by);

    void order( int& small, int& large);

    double dist(const int& pinId1, const int& pinId2); //return the Manhattan distance of 2 pin
    int getBinXIndex(const double& x);
    int getBinYIndex(const double& y);


    CPlaceDB* m_pDB;

    double m_binWidth;
    double m_binHeight;

    int m_nBinRow;
    int m_nBinCol;

    vector< vector<MyGlobalRouterBin> > m_routerBin;

    vector< vector<int> > m_netRMSTs;
    vector< vector<MyGlobalRouterPath> > m_netRMST_Paths;
};
#endif
