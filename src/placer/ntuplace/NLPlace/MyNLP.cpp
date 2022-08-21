#include <algorithm>
#include <cfloat>
#include <climits>
#include <cmath>
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <sys/types.h>
#include <vector>
using namespace std;

#include <pthread.h>
#include <semaphore.h>

#include "../Legalizer/TetrisLegal.h"
#include "../PlaceCommon/ParamPlacement.h"
#include "../PlaceCommon/placebin.h"
#include "../PlaceCommon/placeutil.h"
#include "../PlaceDB/placedb.h"
#include "MyNLP.h"
#include "smooth.h"
//#include "CongMap.h"
#include "../PlaceCommon/arghandler.h"
#include "../PlaceCommon/util.h"
//#include "../PlaceDB/SteinerDecomposition.h"
#include "../PlaceCommon/BlockMatching.h"
//#include "GRouteMap.cpp"

// HLBFGS optimzation
#include "../lib/hlbfgs/HLBFGS.h"

// Added by kaie
#include "../lib/lpsolve55/lp_lib.h"

bool bMulti = false;

bool bFast = true; // spreading force interpolation

double density;
double wirelength;
double gTotalWL;

double time_wire_force = 0;
double time_spreading_force = 0;
double time_via_force = 0; // kaie

// static variables
bool MyNLP::m_bXArch = false;
double MyNLP::m_yWeight = 2.0;
vector<vector<vector<double>>> MyNLP::m_weightDensity; // (kaie) 2009-09-10 3d
double MyNLP::m_skewDensityPenalty1 = 1.0;
double MyNLP::m_skewDensityPenalty2 = 1.0;

#define REPLACE_SQRT2 1.414213562373095048801L

/* Constructor. */
MyNLP::MyNLP(CPlaceDB &db)
    : _potentialGridR(2), m_potentialGridSize(-1), m_pGRMap(NULL) {

  if (gArg.CheckExist("multi"))
    bMulti = true;

  if (!param.bFast) //if (gArg.CheckExist("nointerpolate"))
    bFast = false;

  gArg.GetDouble("skew", &m_skewDensityPenalty1);
  gArg.GetDouble("skew2", &m_skewDensityPenalty2);

  m_precision = 0.99999;
  m_weightIncreaseFactor = 2.0;
  m_targetUtil = 1.0;
  gArg.GetDouble("util", &m_targetUtil);
  m_bRunLAL = true;
  m_binSize = 0.8;
  m_congWeight = 1.0;
  m_potentialGridPadding = 0;
  m_useEvalF = true;
  m_bXArch = false;
  m_spreadingForceNullifyRatio = 0.0;

  m_smoothR = 5; // Gaussian smooth R
  if (param.b3d) // (kaie)
    m_smoothR = 1;

  if (gArg.CheckExist("nolal"))
    m_bRunLAL = false;

  m_maxIte = 50; // max outerIte
  gArg.GetInt("maxIte", &m_maxIte);

  if (param.bShow) {
    gArg.GetDouble("precision", &m_precision);
    gArg.GetDouble("incFactor", &m_weightIncreaseFactor);
    gArg.GetDouble("binSize", &m_binSize);
    gArg.GetDouble("congWeight", &m_congWeight);
    gArg.GetDouble("yWeight", &m_yWeight);
    gArg.GetDouble("nullifyRatio", &m_spreadingForceNullifyRatio);
    gArg.GetInt("padding", &m_potentialGridPadding);
    gArg.GetInt("smoothr", &m_smoothR);
    m_bXArch = gArg.CheckExist("x");
    m_useEvalF = !gArg.CheckExist("evalhpwl");

    printf("\n");
    printf("[Analytical Placement Parameters]\n");
    printf("    solver precision            = %g\n", m_precision);
    printf("    weight increasing factor    = %g\n", m_weightIncreaseFactor);
    printf("    target utilization          = %g\n", m_targetUtil);
    printf("    bin size factor             = %g\n", m_binSize);
    printf("    G-smooth r                  = %d\n", m_smoothR);

    if (m_bXArch)
      printf("    use X-arch wire model       = %s\n",
             TrueFalse(m_bXArch).c_str());

    if (m_bRunLAL)
      printf("    use look-ahead legalization = %s\n",
             TrueFalse(m_bRunLAL).c_str());

    printf("    congestion weight           = %.2f\n", m_congWeight);

    if (m_yWeight != 1)
      printf("    vertical weight             = %.2f\n", m_yWeight);

    if (m_potentialGridPadding > 0)
      printf("    potential grid padding      = %d\n", m_potentialGridPadding);

    if (m_spreadingForceNullifyRatio > 0)
      printf("    null spreading force ratio  = %.2f\n",
             m_spreadingForceNullifyRatio);
    printf("\n");
  }

  m_lookAheadLegalization = false;
  m_earlyStop = false;
  m_topLevel = false;
  m_useBellPotentialForPreplaced = true;

  m_weightWire = 4.0;
  m_weightTSV = 0.0;
  // gArg.GetDouble("TSV", &m_weightTSV);

  m_smoothDelta = 1;

  m_pDB = &db;
  InitModuleNetPinId();

  if (param.b3d) // (kaie) number of layers
    m_pDB->m_totalLayer = param.nlayer;
  else
    m_pDB->m_totalLayer = 1;
  if (param.b3d && !param.noZ)
    m_bMoveZ = true;
  else
    m_bMoveZ = false;

  // scale between 0 to 10
  const double range = 10.0;
  if (m_pDB->m_coreRgn.right > m_pDB->m_coreRgn.top)
    m_posScale = range / m_pDB->m_coreRgn.right;
  else
    m_posScale = range / m_pDB->m_coreRgn.top;

  nModule_ = m_pDB->m_nModules;
  // eDensity Init
  if(param.bUseEDensity){
    InitFillerPlace(); // Evenly distribute filler
    InitEDensity();
  }


  _cellPotentialNorm.resize(m_pDB->m_modules.size());

  x.resize(2 * m_pDB->m_modules.size());
  // cellLock.resize( m_pDB->m_modules.size(), false );
  xBest.resize(2 * m_pDB->m_modules.size());
  _expX.resize(2 * m_pDB->m_modules.size());
  _expPins.resize(2 * m_pDB->m_pins.size());
  x_l.resize(2 * m_pDB->m_modules.size());
  x_u.resize(2 * m_pDB->m_modules.size());
  xMax.resize(m_pDB->m_modules.size());
  yMax.resize(m_pDB->m_modules.size());

  // (kaie) 2009-09-12 add z direction
  z.resize(m_pDB->m_modules.size());
  if (m_bMoveZ) {
    grad_f.resize(3 * m_pDB->m_modules.size());
    last_grad_f.resize(3 * m_pDB->m_modules.size());
    walk_direction.resize(3 * m_pDB->m_modules.size(), 0);
    last_walk_direction.resize(3 * m_pDB->m_modules.size(), 0);

    // kaie 2009-08-29 3D placement
    zBest.resize(m_pDB->m_modules.size());
    _expZ.resize(m_pDB->m_modules.size());
    _expPinsZ.resize(m_pDB->m_pins.size());
    zMax.resize(m_pDB->m_modules.size());
  } else {
    grad_f.resize(2 * m_pDB->m_modules.size());
    last_grad_f.resize(2 * m_pDB->m_modules.size());
    walk_direction.resize(2 * m_pDB->m_modules.size(), 0);
    last_walk_direction.resize(2 * m_pDB->m_modules.size(), 0);
  }
  // @kaie 2009-08-29

  if (m_bXArch) // 2006-09-12 (donnie)
  {
    _expXplusY.resize(m_pDB->m_modules.size());  // exp( x+y /k )  for cells
    _expXminusY.resize(m_pDB->m_modules.size()); // exp( x-y /k )  for cells
    _expPinXplusY.resize(m_pDB->m_pins.size());  // exp( x+y /k )  for pins
    _expPinXminusY.resize(m_pDB->m_pins.size()); // exp( x-y /k )  for pins
  }

  /*
     for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
     {
     if( m_pDB->m_modules[i].m_isFixed )
     cellLock[ i ] = true;
     }
     */

  m_usePin.resize(m_pDB->m_modules.size());
  SetUsePin();

  m_nets_sum_exp_xi_over_alpha.resize(m_pDB->m_nets.size(), 0);
  m_nets_sum_exp_yi_over_alpha.resize(m_pDB->m_nets.size(), 0);
  m_nets_sum_exp_inv_xi_over_alpha.resize(m_pDB->m_nets.size(), 0);
  m_nets_sum_exp_inv_yi_over_alpha.resize(m_pDB->m_nets.size(), 0);
  // frank 2022-07-23 3d
  if (param.b3d && param.bF2FhpwlEnhance) {
    m_layer_nets_sum_exp_xi_over_alpha.resize(
        param.nlayer, vector<double>(m_pDB->m_nets.size(), 0));
    m_layer_nets_sum_exp_yi_over_alpha.resize(
        param.nlayer, vector<double>(m_pDB->m_nets.size(), 0));
    m_layer_nets_sum_exp_inv_xi_over_alpha.resize(
        param.nlayer, vector<double>(m_pDB->m_nets.size(), 0));
    m_layer_nets_sum_exp_inv_yi_over_alpha.resize(
        param.nlayer, vector<double>(m_pDB->m_nets.size(), 0));
  }

  // kaie 2009-08-29 3d placement
  if (m_bMoveZ) {
    m_nets_sum_exp_zi_over_alpha.resize(m_pDB->m_nets.size(), 0);
    m_nets_sum_exp_inv_zi_over_alpha.resize(m_pDB->m_nets.size(), 0);
  }
  // @kaie 2009-08-29

  // (kaie) 2010-10-18 Weighted-Average-Exponential Wirelength Model
  if (param.bUseWAE) {
    m_nets_weighted_sum_exp_xi_over_alpha.resize(m_pDB->m_nets.size(), 0);
    m_nets_weighted_sum_exp_yi_over_alpha.resize(m_pDB->m_nets.size(), 0);
    m_nets_weighted_sum_exp_inv_xi_over_alpha.resize(m_pDB->m_nets.size(), 0);
    m_nets_weighted_sum_exp_inv_yi_over_alpha.resize(m_pDB->m_nets.size(), 0);
    if (param.b3d && param.bF2FhpwlEnhance) {
      vector<double> tmp_v;
      tmp_v.resize(m_pDB->m_nets.size(), 0);
      m_layer_nets_weighted_sum_exp_xi_over_alpha.resize(param.nlayer, tmp_v);
      m_layer_nets_weighted_sum_exp_yi_over_alpha.resize(param.nlayer, tmp_v);
      m_layer_nets_weighted_sum_exp_inv_xi_over_alpha.resize(param.nlayer, tmp_v);
      m_layer_nets_weighted_sum_exp_inv_yi_over_alpha.resize(param.nlayer, tmp_v);
    }
  }

  if (m_bXArch) // 2006-09-12 (donnie)
  {
    m_nets_sum_exp_x_plus_y_over_alpha.resize(m_pDB->m_nets.size(), 0);
    m_nets_sum_exp_x_minus_y_over_alpha.resize(m_pDB->m_nets.size(), 0);
    m_nets_sum_exp_inv_x_plus_y_over_alpha.resize(m_pDB->m_nets.size(), 0);
    m_nets_sum_exp_inv_x_minus_y_over_alpha.resize(m_pDB->m_nets.size(), 0);
  }

  if (param.bUseLSE == false) {
    m_nets_sum_p_x_pos.resize(m_pDB->m_nets.size(), 0);
    m_nets_sum_p_y_pos.resize(m_pDB->m_nets.size(), 0);
    m_nets_sum_p_inv_x_pos.resize(m_pDB->m_nets.size(), 0);
    m_nets_sum_p_inv_y_pos.resize(m_pDB->m_nets.size(), 0);
    m_nets_sum_p_x_neg.resize(m_pDB->m_nets.size(), 0);
    m_nets_sum_p_y_neg.resize(m_pDB->m_nets.size(), 0);
    m_nets_sum_p_inv_x_neg.resize(m_pDB->m_nets.size(), 0);
    m_nets_sum_p_inv_y_neg.resize(m_pDB->m_nets.size(), 0);

    // kaie 2009-08-29 3d placement
    if (m_bMoveZ) {
      m_nets_sum_p_z_pos.resize(m_pDB->m_nets.size(), 0);
      m_nets_sum_p_inv_z_pos.resize(m_pDB->m_nets.size(), 0);
      m_nets_sum_p_z_neg.resize(m_pDB->m_nets.size(), 0);
      m_nets_sum_p_inv_z_neg.resize(m_pDB->m_nets.size(), 0);
    }
    // @kaie 2009-08-29
  }

  grad_wire.resize(2 * m_pDB->m_modules.size(), 0.0);
  grad_potential.resize(2 * m_pDB->m_modules.size(), 0.0);

  // kaie 2009-08-29 TSVs
  if (m_bMoveZ) {
    grad_potentialZ.resize(m_pDB->m_modules.size(),
                           0.0); // (kaie) potential in z direction
    grad_via.resize(m_pDB->m_modules.size(), 0.0);
  }


  m_threadInfo.resize(param.nThread);
  m_threadResult.resize(param.nThread);
}

MyNLP::~MyNLP() { 
  // if(m_pGRMap==NULL) delete m_pGRMap;
}

void MyNLP::SetUsePin() {
  int effectivePinCount = 0;
  for (unsigned int i = 0; i < m_pDB->m_modules.size(); i++) {
    bool usePin = false;
    for (unsigned int p = 0; p < m_pDB->m_modules[i].m_pinsId.size(); p++) {
      int pinId = m_pDB->m_modules[i].m_pinsId[p];
      if (m_pDB->m_pins[pinId].xOff != 0.0 ||
          m_pDB->m_pins[pinId].yOff != 0.0) {
        usePin = true;
        break;
      }
    }

    // 2006-04-23 (donnie)
    if (param.bHandleOrientation &&
        m_pDB->m_modules[i].m_height == m_pDB->m_rowHeight &&
        m_pDB->m_modules[i].m_isFixed == false)
      usePin = false;

    if (usePin)
      effectivePinCount++;
    m_usePin[i] = usePin;
  }
  if (param.bShow)
    printf("Effective Pin # = %d\n", effectivePinCount);
}

bool MyNLP::MySolve(double wWire, double target_density, int currentLevel) {  // for plotting
  double time_start = seconds();
  double time_start_real = seconds_real();
  assert(_potentialGridR > 0);

  if (m_potentialGridSize <= 0) {
    m_potentialGridSize = static_cast<int>(
        sqrt(static_cast<double>(m_pDB->m_modules.size())) * m_binSize /
        sqrt(static_cast<double>(m_pDB->m_totalLayer))); // (kaie) 2009-09-10 3d placement
  }

  int n, m, nnz_jac_g, nnz_h_lag;
  get_nlp_info(n, m, nnz_jac_g, nnz_h_lag);
  get_bounds_info(n, x_l, x_u);
  Parallel(BoundXThread, m_pDB->m_modules.size());
  if (m_bMoveZ)
    Parallel(BoundZThread, m_pDB->m_modules.size());

  m_ite = 0;
  bool isLegal = false;

  assert(param.dLpNorm_P > 0);
  if (param.bUseLSE) {
    //_alpha = 0.5 * m_potentialGridWidth; // according to APlace ispd04
    // double maxValue = param.dLpNorm_P;	// > 700 leads to overflow
    _alpha =
        (m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left) / 2 / param.dLpNorm_P;
    //_alpha = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) * 0.005;	//
    //as small as possible -- NLP (for contest)
  } else {
    // Lp-norm
    //_alpha = param.dLpNorm_P;
    _alpha = log10(DBL_MAX);
  }

  if (param.bShow){
    printf("GRID = %d  (width = %.2f)  alpha= %f  weightWire= %f\n",
           m_potentialGridSize,
           (m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left) /
               m_potentialGridSize,
           _alpha, wWire);
  }

  m_pDB->UpdatePinNetId();

  // eDensity Init
  if(param.bUseEDensity){
    get_starting_point(x, z);
    if(param.bPlot) plotPL("filler-insert", -1);
    if(param.bPre2dPlace){
      FillerSpreading(wWire, target_density); // Fix cells and Spread fillers with eDensity
      if(param.bPlot) plotPL("filler-spread", -1);
    }
  }

  int counter = 0;
  while (true) { // iterative GoSolve() until succ, else --> reduce alpha
    counter++;

    // save the block position
    vector<CPoint> blockPositions;
    CPlaceUtil::SavePlacement(*m_pDB, blockPositions);

    get_starting_point(x, z);
    Parallel(BoundXThread, m_pDB->m_modules.size());
    if (m_bMoveZ) // kaie z-direction move
      Parallel(BoundZThread, m_pDB->m_modules.size());

    bool succ = GoSolve(wWire, target_density, currentLevel, &isLegal);

    if (succ) {
      break;
    }
    else{
      _alpha -= 5;
      CPlaceUtil::LoadPlacement(*m_pDB, blockPositions);

      if (param.bShow)
        printf("\nFailed to solve it. alpha = %f\n\n", _alpha);
      if (_alpha < 80) {
        printf("\nFailed to solve it. alpha = %f\n\n", _alpha);
        printf("Fatal error in analytical solver. Exit program.\n");
        exit(0);
      }
    }
  }

  if (param.bShow) {
    double realLevelTime = double(seconds_real() - time_start_real);
    double levelTime = double(seconds() - time_start);
    printf("HPWL = %.0f\n", m_pDB->CalcHPWL());
    printf("Time wire = %.0f s   spreading = %.0f s\n", time_wire_force,
           time_spreading_force);
    printf("Level Time (Real) = %.2f (%.2f) sec = %.2f (%.2f) min (%.2fX)\n",
           levelTime, realLevelTime, levelTime / 60.0, realLevelTime / 60.0,
           levelTime / realLevelTime);
  }

  return isLegal;
}

bool MyNLP::GoSolve(double wWire, double target_density,
                    int currentLevel, // for plotting
                    bool *isLegal) {

  *isLegal = false;
  time_wire_force = time_spreading_force = time_via_force = 0.0;
  double givenTargetUtil = m_targetUtil; // for look ahead legalization
  ////////////////////////////////////////
  double spreadingExtraReduction = 0.0;
  m_targetUtil -= spreadingExtraReduction;
  ////////////////////////////////////////
  m_currentStep = param.step;
  m_currentStepZ = param.stepZ;
  m_targetUtil += param.targetDenOver;
  if (m_targetUtil > 1.0)
    m_targetUtil = 1.0;
  double time_start = seconds();
  char filename[100]; // for gnuplot

  int n;
  if (m_bMoveZ)
    n = 3 * m_pDB->m_modules.size(); // (kaie) 2009-09-12 add z direction
  else
    n = 2 * m_pDB->m_modules.size();

  double designUtil = m_pDB->m_totalMovableModuleVolumn / m_pDB->m_totalFreeSpace;

  double baseUtil = 0.05; // experience value preventing over-spreading
  gArg.GetDouble("baseUtil", &baseUtil);
  m_targetUtil = min(1.0, m_targetUtil + baseUtil);

  double lowestUtil = min(1.0, designUtil + baseUtil);
  if (m_targetUtil > 0) // has user-defined target utilization
  {
    if (m_targetUtil < lowestUtil) {
      if (param.bShow)
        printf("NOTE: Target utilization (%f) is too low\n", m_targetUtil);
      if (gArg.CheckExist("forceLowUtil") == false)
        m_targetUtil = lowestUtil;
    }
  } else // no given utilization
  {
    printf("No given target utilization.\n"); //  Distribute blocks evenly
    m_targetUtil = lowestUtil;
  }
  
  if (param.bShow) {
    printf("INFO: Design utilization: %f\n", designUtil);
    printf("DBIN: Target utilization: %f\n", m_targetUtil);
  }

  fill(grad_f.begin(), grad_f.end(), 0.0);
  fill(last_grad_f.begin(), last_grad_f.end(), 0.0);

  CreatePotentialGrid(); // Create potential grid according to
                         // "m_potentialGridSize"
  int densityGridSize =
      m_potentialGridSize / 4; // Use larger grid for density computing
  CreateDensityGrid(densityGridSize);
  UpdateDensityGridSpace(n, x, z);

  UpdatePotentialGridBase(x, z);

  // SmoothBasePotential3D();
  UpdateExpBinPotential(m_targetUtil, true);

  assert(m_targetUtil > 0);

  // wirelength
  Parallel(UpdateExpValueForEachCellThread, m_pDB->m_modules.size());
  Parallel(UpdateExpValueForEachPinThread, m_pDB->m_pins.size());
  Parallel(UpdateNetsSumExpThread, (int)m_pDB->m_nets.size());
  wirelength = GetLogSumExpWL(x, _expX, _alpha, this); // WA
  if (m_bMoveZ)
    GetLogSumExpVia(z, _expZ, _alpha, this);
  // density
  Parallel(ComputeNewPotentialGridThread, m_pDB->m_modules.size());
  UpdatePotentialGrid(z);
  UpdateDensityGrid(n, x, z);
  density = GetDensityPanelty();
  if(param.bUseEDensity){
    updateDensityForceBin();
  }

  ///////////// [1] - Init /////////////
  m_dWeightCong = 1.0;
  if (!InitObjWeights(wWire)) {
    printf("InitObjWeight OVERFLOW!\n");
    return false; // overflow
  }
  int maxIte = m_maxIte;
  bool newDir = true;
  double obj_value;
  eval_f(n, x, _expX, true, obj_value);
  if(param.bUseEDensity){
    updateDensityForceBin();
  }
  ComputeBinGrad();
  Parallel(eval_grad_f_thread, m_pDB->m_modules.size());
  UpdateDensityGrid(n, x, z);
  double maxDen = GetMaxDensity();
  double lastMaxDen = maxDen;
  double totalOverDen = GetTotalOverDensity();
  double totalOverDenLB = GetTotalOverDensityLB();
  double totalOverPotential = GetTotalOverPotential();
  double totalOverDenNet = GetTotalOverDensityNet();
  double totalOverPotentialNet = GetTotalOverPotentialNet();
  double maxDenNet = GetMaxDensityNet();
  double lastTotalOverNet = 0;
  double lastTotalOverPotentialNet = DBL_MAX;
  double lastMaxDenNet = maxDenNet;
  double overNet = totalOverDenNet;

  if (obj_value > DBL_MAX * 0.5) {
    printf("Objective value OVERFLOW!\n");
    return false; // overflow
  }

  if (param.bShow) {
    printf(" %d-%2d HPWL= %.0f\tDen= %.2f %.4f %.4f %.4f Dcost= %4.1f%% \n",
           currentLevel, m_ite, m_pDB->CalcHPWL(), maxDen, totalOverDen,
           totalOverDenLB, totalOverPotential,
           density * _weightDensity / obj_value * 100.0);
  } else {
    printf(" %d-%2d HPWL= %.0f \t", currentLevel, m_ite, m_pDB->CalcHPWL());
  }
  fflush(stdout);

  double lastTotalOver = 0;
  double lastTotalOverPotential = DBL_MAX;
  double over = totalOverDen;
  int totalIte = 0;
  double bestLegalWL = DBL_MAX;
  int lookAheadLegalCount = 0;
  double totalLegalTime = 0.0;
  bool startDecreasing = false;
  int checkStep = 5;
  int outStep = 50;
  if (param.bShow == false) outStep = INT_MAX;
  int forceBreakLoopCount = INT_MAX;
  if (m_topLevel == false) forceBreakLoopCount = INT_MAX;

  // Legalization related configurations
  // int tetrisDir = 0;	// 0: both   1: left   2: right
  int LALnoGoodCount = 0;
  int maxNoGoodCount = 2;

  vector<Module> bestGPresult; // for LAL (Look Ahead Legal)

  static double lastHPWL = 0; // test
  static double lastTSV = 0;  // kaie
  if (currentLevel > 1) {
    lastHPWL = DBL_MAX;
    lastTSV = DBL_MAX; // kaie
  }
  ////////////////////////////////////////////////////////////

  // [2] - Iteration Optimization
  newDir = true;
  bool bUpdateWeight = true;
  int global_iter = 0;
  for (int ite = 0; ite < maxIte; ite++) { //////////////////////////////////////////////////////////////////////
    m_ite++;
    int innerIte = 0;
    double old_obj = DBL_MAX;
    double last_obj_value = DBL_MAX;

    m_currentStep = param.step;
    if (bUpdateWeight == false) newDir = false;
    else newDir = true;
    bUpdateWeight = true;

    // [2.1] - inner loop, minimize "f" 
    double lastDensityCost = density; // for startDecreasing determination
    while (true) {                     
      innerIte++;
      global_iter++;
      swap(last_grad_f, grad_f); // save for computing the congujate gradient direction
      swap(last_walk_direction, walk_direction);
      // [2.1.1] - Compute BinGrad and Adjust Force
      if(param.bUseEDensity){
        updateDensityForceBin();
      }
      ComputeBinGrad();
      Parallel(eval_grad_f_thread, m_pDB->m_modules.size());

      if (!AdjustForce(n, x, grad_f)) {
        printf("AdjustForce, NaN or Inf\n");
        return false; // NaN or Inf
      }

      if (innerIte % checkStep == 0) {
        if (m_useEvalF) {
          old_obj = last_obj_value; // backup the old value
          if (m_bMoveZ) 
            LayerAssignment();
          Parallel(UpdateBlockPositionThread, m_pDB->m_modules.size());
          // m_pDB->CalcHPWL();
          eval_f(n, x, _expX, true, obj_value);
          last_obj_value = obj_value;
        } else // Observe the wirelength change
        {
          if (m_bMoveZ)
            LayerAssignment();
          Parallel(UpdateBlockPositionThread, m_pDB->m_modules.size());
          m_pDB->CalcHPWL();
          if (m_pDB->GetHPWL() < lastHPWL) {
            lastHPWL = 0;
            break;
          }
          lastHPWL = m_pDB->GetHPWL();
        }
      }

      // Output solving progress
      if (innerIte % outStep == 0 && m_useEvalF) {
        if (innerIte % checkStep != 0)
          eval_f(n, x, _expX, true, obj_value);
        printf("\n  (%4d): f %g\t w %g\t p %g\tstep= %.5f \t%.1fm ", innerIte,
               obj_value, gTotalWL, density, m_stepSize,
               double(seconds() - time_start) / 60.0);
        fflush(stdout);
      }

      // [2.1.2] - Check result in the inner loop
      if (innerIte % checkStep == 0) {
        printf(".");
        fflush(stdout);
        
        UpdateDensityGrid(n, x, z); // find the exact bin density
        totalOverDen = GetTotalOverDensity();
        totalOverPotential = GetTotalOverPotential();
        lastTotalOver = over;
        over = min(totalOverPotential, totalOverDen);

        if (!startDecreasing) {
          if ((innerIte <= 10 && m_ite == 1) || (innerIte <= 5)) // need to wait until "stable"
            lastDensityCost = density;
          else if (density < lastDensityCost * 0.99) {
            printf(">>");
            fflush(stdout);
            startDecreasing = true;
          }
        }

        if (startDecreasing && over < target_density && m_ite != 1) // no early stop at ite 1
          break;        // 2005-03-11 (donnie) Meet the constraint
        if (m_useEvalF && obj_value >= m_precision * old_obj){ // Cannot further reduce "f"
          break; // 2005-03-11 (Brian) Meet the constraint
        }
      } // check in the inner loop

      // [2.1.3] - Compute beta and Gradient Direction
      if (newDir == true) {
        // gradient direction
        newDir = false;
        for (int i = 0; i < n; i++) {
          grad_f[i] = -grad_f[i];
          walk_direction[i] = grad_f[i];
        }
      } else {
        // gradient direction
        if (FindBeta(n, grad_f, last_grad_f, m_beta) == false) {
          printf("FindBeta OVERFLOW!\n");
          return false; // overflow?
        }
        Parallel(UpdateGradThread, n);
      }
      LineSearch(n, x, walk_direction, m_stepSize); // Calculate a_k (step size)
      
      // TODO: open this comment
      xMax.resize(m_pDB->m_nets.size(), 0);
      yMax.resize(m_pDB->m_nets.size(), 0);
      zMax.resize(m_pDB->m_nets.size(), 0);
      Parallel(UpdateXThread, m_pDB->m_modules.size()); // Update X. (x_{k+1} = x_{k} + \alpha_k * d_k)
      Parallel(BoundXThread, m_pDB->m_modules.size());
      if (m_bMoveZ)
        Parallel(BoundZThread, m_pDB->m_modules.size());
      if(param.bPlot && global_iter%5==0){
        char fileName[128];
        sprintf(fileName, "gp-%04d", global_iter);
        //plotPL(string(move(fileName)), global_iter);
        draw_field(global_iter, "");
      } 
      

      // [2.1.4] - Update WL & Density Force
      // New block positions must be ready
      // 1. UpdateExpValueForEachCellThread    (wire force)
      // 2. UpdateExpValueForEachPinThread     (wire force)
      // 3. ComputeNewPotentialGridThread      (spreading force)
      Parallel(UpdateNLPDataThread, m_pDB->m_modules.size(), m_pDB->m_pins.size(), m_pDB->m_modules.size());
      // New EXP values must be ready
      double time_used = seconds();
      Parallel(UpdateNetsSumExpThread, (int)m_pDB->m_nets.size());
      time_wire_force += seconds() - time_used;
      if(param.bUseEDensity){
        if(m_bMoveZ)
          updateBinsGCellDensityVolumn();
        else
          updateBinsGCellDensityArea();
      }
      time_used = seconds();
      UpdatePotentialGrid(z);
      time_spreading_force += seconds() - time_used; 

      if (innerIte == forceBreakLoopCount) {
        printf("b");
        bUpdateWeight = false;
        break;
      }
    } // inner loop ///////////////////////////////////////////////////////////////////////

    // [2.2] - check result for this iter //////////
    if (param.bShow) {
      printf("%d\n", innerIte);
      fflush(stdout);
    } else
      printf("\n");
    totalIte += innerIte;

    if(param.bUseEDensity){
      updateDensityForceBin();
    }
    UpdateDensityGrid(n, x, z);
    maxDen = GetMaxDensity();
    totalOverDen = GetTotalOverDensity();
    totalOverDenLB = GetTotalOverDensityLB();
    totalOverPotential = GetTotalOverPotential();

    if (m_bMoveZ)
      LayerAssignment();
    Parallel(UpdateBlockPositionThread, m_pDB->m_modules.size()); // update to placeDB

    if (obj_value > DBL_MAX * 0.5) {
      printf("Objective value OVERFLOW!\n");
      return false; // overflow
    }

    if (param.bShow) {
      printf(" %d-%2d HPWL= %.0f\tDen= %.2f %.4f %.4f %.4f LCPU= %.1fm Dcost= %4.1f%% ",
             currentLevel, m_ite, m_pDB->CalcHPWL(), maxDen, totalOverDen,
             totalOverDenLB, totalOverPotential,
             double(seconds() - time_start) / 60.0,
             0.5 * density * _weightDensity / obj_value * 100.0);
    } else {
      printf(" %d-%2d HPWL= %.f\tLCPU= %.1fm\n", currentLevel, m_ite,
             m_pDB->CalcHPWL(), double(seconds() - time_start) / 60.0);
    }
    fflush(stdout);

    bool spreadEnough = totalOverPotential < 1.3;
    bool increaseOverPotential = totalOverPotential > lastTotalOverPotential;
    bool increaseMaxDen = maxDen > lastMaxDen;
    bool enoughIteration = ite > 3;
    bool notEfficientOptimize = 0.5 * density * _weightDensity / obj_value * 100.0 > 95;
    // PrintPotentialGrid();
    if (enoughIteration && notEfficientOptimize) {
      printf("Failed to further optimize (enoughIteration && notEfficientOptimize)\n");
      break;
    }
    if (enoughIteration && increaseOverPotential && increaseMaxDen && spreadEnough) {
      printf("Cannot further reduce over potential! (skip LAL)\n");
      break;
    }
    if (startDecreasing && over < target_density) {
      printf("Meet constraint! (startDecreasing && over < target_density)\n");
      break;
    }
    if(ite >= 2){
      printf("Meet constraint! (ite >= 2)\n");
      break;
    }
    
    // [2.3] - Update Weights
    if (bUpdateWeight)
      UpdateObjWeights();

    lastTotalOverPotential = totalOverPotential;
    lastMaxDen = maxDen;
    lastTotalOverPotentialNet = totalOverPotentialNet;
    lastMaxDenNet = maxDenNet;

  } // outer loop /////////////////////////////////////////////////////////////////////////////////////////////////////

  // [3] - Layer Assignment
  if (m_bMoveZ)
    LayerAssignment();
  Parallel(UpdateBlockPositionThread, m_pDB->m_modules.size());
  printf("#TSVs = %d\n", (int)m_pDB->CalcTSV());

  if (param.bShow) {
    static int allTotalIte = 0;
    allTotalIte += totalIte;
    m_pDB->ShowDensityInfo();
    printf("\nLevel Ite %d   Total Ite %d\n", totalIte, allTotalIte);
  }

  return true;
}

// static
void *MyNLP::UpdateGradThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  // cout << "----------m_beta = " <<pMsg->pNLP->m_beta << "\n";
  if (param.bUseNAG) {
    for (int i = pMsg->index1; i < pMsg->index2; i++) {
      pMsg->pNLP->walk_direction[i] =
          pMsg->pNLP->grad_f[i] +
          pMsg->pNLP->m_beta * pMsg->pNLP->last_walk_direction[i] +
          pMsg->pNLP->m_beta * (pMsg->pNLP->grad_f[i] - pMsg->pNLP->last_grad_f[i]);
    }
  } else {
    for (int i = pMsg->index1; i < pMsg->index2; i++) {
      pMsg->pNLP->walk_direction[i] =
          -pMsg->pNLP->grad_f[i] +
          pMsg->pNLP->m_beta * pMsg->pNLP->last_walk_direction[i];
    }
  }
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

// static
void *MyNLP::UpdateXThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  for (int i = pMsg->index1; i < pMsg->index2; i++) {
    if (pMsg->pNLP->m_bMoveZ) {
      pMsg->pNLP->x[2 * i] +=
          pMsg->pNLP->walk_direction[3 * i] * pMsg->pNLP->m_stepSizeX;
      pMsg->pNLP->x[2 * i + 1] +=
          pMsg->pNLP->walk_direction[3 * i + 1] * pMsg->pNLP->m_stepSizeY;
    } else {
      pMsg->pNLP->x[2 * i] +=
          pMsg->pNLP->walk_direction[2 * i] * pMsg->pNLP->m_stepSize;
      pMsg->pNLP->x[2 * i + 1] +=
          pMsg->pNLP->walk_direction[2 * i + 1] * pMsg->pNLP->m_stepSize;
    }

    if (pMsg->pNLP->m_bMoveZ) {
      double scale = pMsg->pNLP->m_potentialGridSize;
      gArg.GetDouble("testscale", &scale);
      pMsg->pNLP->z[i] += pMsg->pNLP->walk_direction[3 * i + 2] *
                          pMsg->pNLP->m_stepSizeZ / scale; // /  pMsg->pNLP->m_potentialGridWidth;
    }

    // TODO: open this comment
    if (param.bStabilityEnhance) {
      for (int j = 0; j < pMsg->pNLP->m_pDB->m_modules[i].m_pinsId.size(); ++j) {
        int pinId = pMsg->pNLP->m_pDB->m_modules[i].m_pinsId[j];
        int netId = pMsg->pNLP->m_pDB->m_pinNetId[pinId];
        pMsg->pNLP->xMax[netId] = max(pMsg->pNLP->xMax[netId], pMsg->pNLP->m_pDB->m_modules[i].m_cx + pMsg->pNLP->m_pDB->m_pins[pinId].xOff);
        pMsg->pNLP->yMax[netId] = max(pMsg->pNLP->yMax[netId], pMsg->pNLP->m_pDB->m_modules[i].m_cy + pMsg->pNLP->m_pDB->m_pins[pinId].yOff);
        if (param.b3d && pMsg->pNLP->m_bMoveZ)
          pMsg->pNLP->zMax[netId] = max(pMsg->pNLP->zMax[netId], pMsg->pNLP->m_pDB->m_modules[i].m_cz);
      }
    }
    // printf("%d (%.2f, %.2f, %.2f)\n", i, pMsg->pNLP->x[2*i],
    // pMsg->pNLP->x[2*i+1], pMsg->pNLP->z[i]);
  }
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

// static
void *MyNLP::UpdateNLPDataThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);

  double time_used = seconds();

  // cells * 2
  UpdateExpValueForEachCell(pMsg->index2, *pMsg->pX, *pMsg->pZ, *pMsg->pExpX,
                            *pMsg->pExpZ, pMsg->alpha, pMsg->index1,
                            pMsg->pNLP);

  // pins
  UpdateExpValueForEachPin(pMsg->index4, *pMsg->pX, *pMsg->pZ,
                           pMsg->pNLP->_expPins, pMsg->pNLP->_expPinsZ,
                           pMsg->alpha, pMsg->pNLP, pMsg->pUsePin,
                           pMsg->index3);

  time_wire_force += seconds() - time_used; // wrong when th != 1

  // cells
  time_used = seconds();
  ComputeNewPotentialGrid(*pMsg->pX, *pMsg->pZ, pMsg->pNLP, pMsg->index5,
                          pMsg->index6);
  time_spreading_force += seconds() - time_used;

  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

// static
void *MyNLP::FindGradL2NormThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  double norm = 0;
  for (int i = pMsg->index1; i < pMsg->index2; i++) {
    // if( i%3 == 2 ) continue;
    norm += pMsg->pNLP->last_grad_f[i] * pMsg->pNLP->last_grad_f[i];
  }
  pMsg->pNLP->m_threadResult[pMsg->threadId] = norm;

  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

// static
void *MyNLP::FindGradProductThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  double product = 0;
  for (int i = pMsg->index1; i < pMsg->index2; i++) {
    // if( i%3 == 2 ) continue;
    product += pMsg->pNLP->grad_f[i] *
               (pMsg->pNLP->grad_f[i] + pMsg->pNLP->last_grad_f[i]);
  }
  pMsg->pNLP->m_threadResult[pMsg->threadId] = product;

  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

bool MyNLP::FindBeta(const int &n, const vector<double> &grad_f,
                     const vector<double> &last_grad_f, double &beta) {
  // Polak-Ribiere foumula from APlace journal paper
  // NOTE:
  //   g_{k-1} = -last_grad_f
  //   g_k     = grad_f

  long double l2norm = 0;
  long double product = 0;

  bool bMoveZ = false;
  if (param.b3d && !param.noZ)
    bMoveZ = true;

  // const double maxGradF = 1.0;

  // Compute the scaling factor to avoid overflow
  double maxGradF = 0;
  for (int i = 0; i < n; i++) {
    if (bMoveZ && i % 3 == 2)
      continue;
    if (grad_f[i] > maxGradF)
      maxGradF = grad_f[i];
    if (last_grad_f[i] > maxGradF)
      maxGradF = last_grad_f[i];
  }
  assert(maxGradF != 0);

#if 1
  for (int i = 0; i < n; i++) {
    if (bMoveZ && i % 3 == 2)
      continue;
    l2norm += (last_grad_f[i] / maxGradF) * (last_grad_f[i] / maxGradF);
  }
  for (int i = 0; i < n; i++) {
    if (bMoveZ && i % 3 == 2)
      continue;
    product += (grad_f[i] / maxGradF) * ((grad_f[i] - last_grad_f[i]) / maxGradF); // g_k^T ( g_k - g_{k-1} )
  }
#else
  // Parallelization changes the results
  // l2norm = Parallel( FindGradL2NormThread, n );
  // product = Parallel( FindGradProductThread, n );
#endif

  if (product == 0) {
    printf("product == 0 \n");
  }

  if (l2norm == 0){
    printf("l2norm == 0 \n");
    return false; // Failed. Some problem (overflow) during the computation.
  }

  beta = (double)(product / l2norm);
  if (beta > DBL_MAX * 0.9) {
    // Beta is too large. Overflow may be occured.
    printf("Beta is too large. Overflow may be occured. ( %g = %g / %g)\n", beta, (double)product, (double)l2norm);
    return false;
  }
  return true;
}

void MyNLP::LayerAssignment(const int &n, vector<double> &z, MyNLP *pNLP,
                            int index1, int index2) {
  if (param.bShow) {
    cout << "---------------------------- Some LayerAssignment "
            "Result---------------------------------\n";
    for (int i = 0; i < z.size(); ++i) {
      if (i % 400 == 0)
        cout << "cell[" << i << "].z = " << z[i] << "\n";
    }
    cout << "------------------------------------------------------------------"
            "-----------------------\n";
  }
  if (index2 > n)
    index2 = n;

  // cout << "---------------------------------------- index: " << index1 << " ~
  // " << index2 << "\n";
  bool reverse = false;
  double cutline = pNLP->m_pDB->m_dCutline;
  if (param.nlayer == 2 &&
      pNLP->m_pDB->m_rowNums.size() == 2) { // cad contest 2022
    // check the more side of two die
    int lower = 0, higher = 0;
    for (int i = index1; i < index2; i++) {
      z[i] -= 0.5; // z_after = [0:1]
      if (z[i] <= 0.5)
        lower++;
      else
        higher++;
    }
    if (lower < higher)
      cutline = 1 - cutline;
  }

  for (int i = index1; i < index2; i++) {
    if (param.nlayer == 2 &&
        pNLP->m_pDB->m_rowNums.size() == 2) { // cad contest 2022
      z[i] = (z[i] <= cutline) ? 0.5 : 1.5;
    } else { // origin
      double layerThickness = (pNLP->m_pDB->m_front - pNLP->m_pDB->m_back) /
                              (double)(pNLP->m_pDB->m_totalLayer);
      double z_after = z[i] - 0.5 * layerThickness;
      double layer = floor(z_after / layerThickness);
      double offset = fmod(z_after, layerThickness);
      if (offset > 0.5 * layerThickness)
        layer += 1;
      z_after = (layer + 0.5) * layerThickness;
      assert(fabs(z[i] - z_after) <= 0.5 * layerThickness);
      z[i] = z_after;
    }
  }
}
void MyNLP::LayerAssignment() {
  if (param.bShow) {
    cout << "---------------------------- Some LayerAssignment "
            "Result---------------------------------\n";
    for (int i = 0; i < z.size(); ++i) {
      if (i % 400 == 0)
        cout << "cell[" << i << "].z = " << z[i] << "\n";
    }
    cout << "------------------------------------------------------------------"
            "-----------------------\n";
  }
  bool reverse = false;
  double cutline = m_pDB->m_dCutline;
  // check the more side of two die
  int lower = 0, higher = 0;
  for (int i = 0; i < m_pDB->m_modules.size(); ++i) {
    if(m_pDB->m_modules[i].m_isFiller || m_pDB->m_modules[i].m_isVia) continue;
    z[i] -= 0.5; // z_after = [0:1]
    if (z[i] <= 0.5)
      lower++;
    else
      higher++;
  }
  if (lower < higher)
    cutline = 1 - cutline;

  vector<pair<int, double>> z_ori0; // z_ori[moduleId] = z_ori  // die0
  vector<pair<int, double>> z_ori1; // z_ori[moduleId] = z_ori  // die1
  vector<double> total_area(2, 0.0);
  for (int i = 0; i < m_pDB->m_modules.size(); ++i) {
    if (!m_pDB->m_modules[i].m_isVia && !m_pDB->m_modules[i].m_isFiller) {
      // z[i] = (z[i] <= cutline) ? 0.5 : 1.5;
      if (z[i] <= cutline) {
        z_ori0.emplace_back(pair<int, double>(i, z[i]));
        total_area[0] += (m_pDB->m_modules[i].m_widths[0] *
                          m_pDB->m_modules[i].m_heights[0]);
        z[i] = 0.5;
      } else {
        z_ori1.emplace_back(pair<int, double>(i, z[i]));
        total_area[1] += (m_pDB->m_modules[i].m_widths[1] *
                          m_pDB->m_modules[i].m_heights[1]);
        z[i] = 1.5;
      }
    } else if(m_pDB->m_modules[i].m_isFiller) {
      z[i] = m_pDB->m_modules[i].m_z + 0.5;
    } else {
      z[i] = 0.5;
    }
  }

  // check the max_util
  vector<long long> valid_area(2, 0);
  long long coreArea = (m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left) *
                       (m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom);
  for (int k = 0; k < param.nlayer; ++k)
    valid_area[k] = coreArea * (double)m_pDB->m_maxUtils[k] * 0.995;
  // cout << "\n\033[34m[LayerAssignment]\033[0m - Top-Die:" << total_area[0] <<
  // "/" << valid_area[0] << "(" << setprecision(2) <<
  // total_area[0]/valid_area[0] << "), Bot-Die:" << total_area[1] << "/" <<
  // valid_area[1] << "(" << setprecision(2) << total_area[1]/valid_area[1] <<
  // "), cell_num: " << z_ori0.size() << ":" << z_ori1.size() << "\n";
  int moved_num = 0;
  if (total_area[0] >
      valid_area[0]) { // too many cell in top-die, move cells to bot-die for
                       // matching die's max_utilization
    // move the cells with higher z
    sort(z_ori0.begin(), z_ori0.end(),
         [](pair<int, double> const &l, pair<int, double> const &r) {
           return l.second > r.second;
         });
    for (int i = 0; i < z_ori0.size(); ++i) {
      int moduleId = z_ori0[i].first;
      total_area[0] -= ((long long)m_pDB->m_modules[moduleId].m_widths[0] *
                        m_pDB->m_modules[moduleId].m_heights[0]);
      z[moduleId] = 1.5;
      ++moved_num;
      if (total_area[0] < valid_area[0])
        break;
    }
  } else if (total_area[1] >
             valid_area[1]) { // too many cell in bot-die, move cells to top-die
                              // for matching die's max_utilization
    // move the cells with lower z
    sort(z_ori1.begin(), z_ori1.end(),
         [](pair<int, double> const &l, pair<int, double> const &r) {
           return l.second < r.second;
         });
    for (int i = 0; i < z_ori1.size(); ++i) {
      int moduleId = z_ori1[i].first;
      total_area[1] -= ((long long)m_pDB->m_modules[moduleId].m_widths[1] *
                        m_pDB->m_modules[moduleId].m_heights[1]);
      z[moduleId] = 0.5;
      --moved_num;
      if (total_area[1] < valid_area[1])
        break;
    }
  }
  if (moved_num != 0) {
    if (moved_num > 0)
      cout << "\n\033[34m[LayerAssignment]\033[0m - " << abs(moved_num)
           << " modules have been changed to bot-die by max_util constr.";
    if (moved_num < 0)
      cout << "\n\033[34m[LayerAssignment]\033[0m - " << abs(moved_num)
           << " modules have been changed to top-die by max_util constr.";
    cout << "\n\033[34m[LayerAssignment]\033[0m - cell_num: "
         << z_ori0.size() - moved_num << "/" << z_ori1.size() + moved_num
         << "\n";
  }
}

void *MyNLP::LayerAssignmentThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  LayerAssignment((int)pMsg->pZ->size(), *pMsg->pZ, pMsg->pNLP, pMsg->index1,
                  pMsg->index2);
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

void MyNLP::BoundZ(const int &n, vector<double> &z, const double &z_l,
                   const double &z_u, int index1, int index2) {
  if (index2 > n)
    index2 = n;
  for (int i = index1; i < index2; i++) {
    if (z[i] < z_l)
      z[i] = z_l;
    else if (z[i] > z_u)
      z[i] = z_u;
  }
}

void *MyNLP::BoundZThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  double z_l = pMsg->pNLP->m_pDB->m_back;
  double z_u = pMsg->pNLP->m_pDB->m_front;
  double layerThickness = (z_u - z_l) / (pMsg->pNLP->m_pDB->m_totalLayer);
  z_l += layerThickness * 0.5;
  z_u -= layerThickness * 0.5;
  BoundZ((int)pMsg->pZ->size(), *pMsg->pZ, z_l, z_u, pMsg->index1,
         pMsg->index2);
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}
// @(kaie) 2009-09-10

void MyNLP::BoundX(const int &n, vector<double> &x, vector<double> &x_l,
                   vector<double> &x_h, const int &i) {
  if (x[i] < x_l[i])
    x[i] = x_l[i];
  else if (x[i] > x_h[i])
    x[i] = x_h[i];
}

void *MyNLP::BoundXThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  BoundX((int)pMsg->pX->size(), *pMsg->pX, pMsg->pNLP->x_l, pMsg->pNLP->x_u,
         pMsg->index1, pMsg->index2);
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

// static
void MyNLP::BoundX(const int &n, vector<double> &x, vector<double> &x_l,
                   vector<double> &x_h, int index1, int index2) {
  if (index2 * 2 > n)
    index2 = n;
  for (int i = index1; i < index2; i++) // index for modules
  {
    if (x[2 * i] < x_l[2 * i])
      x[2 * i] = x_l[2 * i];
    else if (x[2 * i] > x_h[2 * i])
      x[2 * i] = x_h[2 * i];
    if (x[2 * i + 1] < x_l[2 * i + 1])
      x[2 * i + 1] = x_l[2 * i + 1];
    else if (x[2 * i + 1] > x_h[2 * i + 1])
      x[2 * i + 1] = x_h[2 * i + 1];
  }
}

bool MyNLP::AdjustForce(const int &n, const vector<double> &x,
                        vector<double> &f) {
  if (param.bAdjustForce == false)
    return true; // no adjust

  double totalGrad = 0;
  double totalGradZ = 0;
  int size;
  if (m_bMoveZ)
    size = n / 3; // x, y, and z directions
  else
    size = n / 2;

  static int outCount = 0;
  outCount++;

  if (gArg.CheckExist("plotForce")) {
    char filename[255];
    sprintf(filename, "force%d.dat", outCount);
    DataHandler data1;
    for (int i = 0; i < size; i++) {
      if (m_bMoveZ)
        data1.Insert(f[3 * i] * f[3 * i] + f[3 * i + 1] * f[3 * i + 1] +
                     f[3 * i + 2] * f[3 * i + 2]);
      else
        data1.Insert(f[2 * i] * f[2 * i] + f[2 * i + 1] * f[2 * i + 1]);
    }
    data1.Sort();
    data1.OutputFile(filename);
  }

  for (int i = 0; i < size; i++) {
    if (m_bMoveZ)
      totalGrad += f[3 * i] * f[3 * i] + f[3 * i + 1] * f[3 * i + 1];
    else
      totalGrad += f[2 * i] * f[2 * i] + f[2 * i + 1] * f[2 * i + 1];
    if (m_bMoveZ)
      totalGradZ += f[3 * i + 2] * f[3 * i + 2];
  }
  double avgGrad = sqrt(totalGrad / size);
  double avgGradZ = sqrt(totalGradZ / size);

  // if( isNaN( totalGrad ) )
  //	return false;

  // Do truncation
  double expMaxGrad = avgGrad * param.truncationFactor; // x + y
  double expMaxGradSquare = expMaxGrad * expMaxGrad;
  double expMaxGradZ = avgGradZ * param.truncationFactor; // z
  double expMaxGradSquareZ = expMaxGradZ * expMaxGradZ;
  for (int i = 0; i < size; i++) {
    double valueSquare;
    double valueSquareZ = 0;
    if (m_bMoveZ)
      valueSquare = (f[3 * i] * f[3 * i] + f[3 * i + 1] * f[3 * i + 1]);
    else
      valueSquare = (f[2 * i] * f[2 * i] + f[2 * i + 1] * f[2 * i + 1]);
    if (m_bMoveZ)
      valueSquareZ = (f[3 * i + 2] * f[3 * i + 2]);
    if (valueSquare > expMaxGradSquare) {
      double value = sqrt(valueSquare);
      if (m_bMoveZ) {
        f[3 * i] = f[3 * i] * expMaxGrad / value;
        f[3 * i + 1] = f[3 * i + 1] * expMaxGrad / value;
      } else {
        f[2 * i] = f[2 * i] * expMaxGrad / value;
        f[2 * i + 1] = f[2 * i + 1] * expMaxGrad / value;
      }
    }
    if (m_bMoveZ) {
      if (valueSquareZ > expMaxGradSquareZ) {
        double valueZ = sqrt(valueSquareZ);
        f[3 * i + 2] = f[3 * i + 2] * expMaxGradZ / valueZ;
      }
    }
  }

  if (gArg.CheckExist("plotForce")) {
    char filename[255];
    sprintf(filename, "force%db.dat", outCount);
    DataHandler data1;
    for (int i = 0; i < size; i++) {
      if (m_bMoveZ)
        data1.Insert(f[3 * i] * f[3 * i] + f[3 * i + 1] * f[3 * i + 1] +
                     f[3 * i + 2] * f[3 * i + 2]);
      else
        data1.Insert(f[2 * i] * f[2 * i] + f[2 * i + 1] * f[2 * i + 1]);
    }
    data1.Sort();
    data1.OutputFile(filename);
  }

  return true;
}

bool MyNLP::AdjustForce(const int &n, const vector<double> &x,
                        vector<double> grad_wl, vector<double> grad_potential,
                        vector<double> grad_potentialZ) {
  double totalGrad = 0;
  double totalGradZ = 0;
  int size;
  if (m_bMoveZ)
    size = n / 3; // x, y, and z
  else
    size = n / 2;
  for (int i = 0; i < size; i++) {
    // printf("%lf, %lf, %lf, %.f\n", grad_wl[2*i], grad_potential[2*i],
    // grad_wl[2*i+1], grad_potential[2*i]+1);
    double value = (grad_wl[2 * i] + grad_potential[2 * i]) *
                       (grad_wl[2 * i] + grad_potential[2 * i]) +
                   (grad_wl[2 * i + 1] + grad_potential[2 * i + 1]) *
                       (grad_wl[2 * i + 1] + grad_potential[2 * i + 1]);
    if (m_bMoveZ) {
      // printf("Z: %lf, %lf\n", grad_via[i], grad_potentialZ[i]);
      totalGradZ += (grad_via[i] + grad_potentialZ[i]) *
                    (grad_via[i] + grad_potentialZ[i]);
    }
    totalGrad += value;
  }

  if (isNaN(totalGrad))
    return false;
  assert(!isNaN(totalGrad)); // it is checked in GoSolve()
  if (isNaN(totalGradZ))
    return false;
  assert(!isNaN(totalGradZ));

  double avgGrad = sqrt(totalGrad / size);
  double avgGradZ = sqrt(totalGradZ / size);

  // Do truncation
  double expMaxGrad = avgGrad * param.truncationFactor; // x + y
  double expMaxGradSquare = expMaxGrad * expMaxGrad;
  double expMaxGradZ = avgGradZ * param.truncationFactor; // z
  double expMaxGradSquareZ = expMaxGradZ * expMaxGradZ;
  for (int i = 0; i < size; i++) {
    double valueSquare = (grad_wl[2 * i] + grad_potential[2 * i]) *
                             (grad_wl[2 * i] + grad_potential[2 * i]) +
                         (grad_wl[2 * i + 1] + grad_potential[2 * i + 1]) *
                             (grad_wl[2 * i + 1] + grad_potential[2 * i + 1]);
    double valueSquareZ = 0;
    if (m_bMoveZ)
      valueSquareZ += (grad_via[i] + grad_potentialZ[i]) * (grad_via[i] + grad_potentialZ[i]);
    if (valueSquare == 0) {
      // avoid value = 0 let to inf
      grad_wl[2 * i] = grad_wl[2 * i + 1] = 0;
      grad_potential[2 * i] = grad_potential[2 * i + 1] = 0;
    } else {
      if (valueSquare > expMaxGradSquare) {
        double value = sqrt(valueSquare);
        grad_wl[2 * i] = grad_wl[2 * i] * expMaxGrad / value;
        grad_wl[2 * i + 1] = grad_wl[2 * i + 1] * expMaxGrad / value;
        grad_potential[2 * i] = grad_potential[2 * i] * expMaxGrad / value;
        grad_potential[2 * i + 1] = grad_potential[2 * i + 1] * expMaxGrad / value;
      }
    }
    if (m_bMoveZ) {
      if (valueSquareZ == 0) {
        grad_via[i] = 0;
        grad_potentialZ[i] = 0;
      } else {
        if (valueSquareZ > expMaxGradSquareZ) {
          double valueZ = sqrt(valueSquareZ);
          grad_via[i] = grad_via[i] * expMaxGradZ / valueZ;
          grad_potentialZ[i] = grad_potentialZ[i] * expMaxGradZ / valueZ;
        }
      }
    }
  }
  return true;
}

void MyNLP::LineSearch(const int &n, /*const*/ vector<double> &x,
                       vector<double> &f, double &stepSize) {
  int size;
  if (m_bMoveZ)
    size = n / 3; // x, y, and z
  else
    size = n / 2;
  long double totalGradX = 0, totalGradY = 0, totalGradZ = 0;
  double avgGrad, avgGradX, avgGradY, avgGradZ;

  /*for( int i=0; i<n; i++ )
      totalGrad += f[i] * f[i];
  avgGrad = sqrt( totalGrad / size );
  stepSize = sqrt(m_potentialGridWidth * m_potentialGridHeight *
  m_potentialGridThickness) / avgGrad * m_currentStep; printf("%.2f\n",
  stepSize);*/
  for (int i = 0; i < size; i++) {
    if (m_bMoveZ) {
      totalGradX += f[3 * i] * f[3 * i];
      totalGradY += f[3 * i + 1] * f[3 * i + 1];
      totalGradZ += f[3 * i + 2] * f[3 * i + 2];
    } else {
      totalGradX += f[2 * i] * f[2 * i];
      totalGradY += f[2 * i + 1] * f[2 * i + 1];
    }
  }
  avgGradX = sqrt(totalGradX / size);
  avgGradY = sqrt(totalGradY / size);
  avgGrad = sqrt((totalGradX + totalGradY) / size);

  m_stepSizeX = (m_potentialGridWidth) / avgGradX * m_currentStep;
  m_stepSizeY = (m_potentialGridHeight) / avgGradY * m_currentStep;

  if (m_bMoveZ) {
    avgGradZ = sqrt(totalGradZ / size);
    m_stepSizeZ = (m_potentialGridThickness) / avgGradZ * m_currentStepZ;
    if (param.bShow)
      cout << "####################################### m_stepSizeZ="
           << m_stepSizeZ << ", avgGradZ=" << avgGradZ << "\n";
    // printf("sz = %lf\n", m_stepSizeZ);
  }

  stepSize = m_potentialGridWidth / avgGrad * m_currentStep;

  return;
}

bool MyNLP::get_nlp_info(int &n, int &m, int &nnz_jac_g,
                         int &nnz_h_lag /*, IndexStyleEnum& index_style*/) {
  if (m_bMoveZ)
    n = m_pDB->m_modules.size() * 3;
  else
    n = m_pDB->m_modules.size() * 2;
  m = 0;         // no constraint
  nnz_jac_g = 0; // 0 nonzeros in the jacobian since no constraint
  return true;
}

bool MyNLP::get_bounds_info(int n, vector<double> &x_l, vector<double> &x_u) {
  if (m_bMoveZ)
    assert(n == (int)m_pDB->m_modules.size() * 3);
  else
    assert(n == (int)m_pDB->m_modules.size() * 2);
  for (unsigned int i = 0; i < m_pDB->m_modules.size(); i++) {
    if (m_pDB->m_modules[i].m_isFixed) {
      x_l[2 * i] = m_pDB->m_modules[i].m_cx;
      x_u[2 * i] = m_pDB->m_modules[i].m_cx;
      x_l[2 * i + 1] = m_pDB->m_modules[i].m_cy;
      x_u[2 * i + 1] = m_pDB->m_modules[i].m_cy;
    } else {
      x_l[2 * i] = m_pDB->m_coreRgn.left + m_pDB->m_modules[i].m_width * 0.5;
      x_u[2 * i] = m_pDB->m_coreRgn.right - m_pDB->m_modules[i].m_width * 0.5;
      x_l[2 * i + 1] =
          m_pDB->m_coreRgn.bottom + m_pDB->m_modules[i].m_height * 0.5;
      x_u[2 * i + 1] =
          m_pDB->m_coreRgn.top - m_pDB->m_modules[i].m_height * 0.5;
    }
  }
  return true;
}

bool MyNLP::get_starting_point(vector<double> &x, vector<double> &z) {
  xMax.resize(m_pDB->m_nets.size(), 0);
  yMax.resize(m_pDB->m_nets.size(), 0);
  zMax.resize(m_pDB->m_nets.size(), 0);

  for (unsigned int i = 0; i < m_pDB->m_modules.size(); i++) {
    x[2 * i] = m_pDB->m_modules[i].m_cx;
    x[2 * i + 1] = m_pDB->m_modules[i].m_cy;
    z[i] = m_pDB->m_modules[i].m_cz;
    if (param.bStabilityEnhance) {
      for (int j = 0; j < m_pDB->m_modules[i].m_pinsId.size(); ++j) {
        int pinId = m_pDB->m_modules[i].m_pinsId[j];
        int netId = m_pDB->m_pinNetId[pinId];
        xMax[netId] = max(xMax[netId], m_pDB->m_modules[i].m_cx + m_pDB->m_pins[pinId].xOff);
        yMax[netId] = max(yMax[netId], m_pDB->m_modules[i].m_cy + m_pDB->m_pins[pinId].yOff);
        if (param.b3d && m_bMoveZ)
          zMax[netId] = max(zMax[netId], m_pDB->m_modules[i].m_cz);
      }
    }
  }
  return true;
}

double MyNLP::Parallel(void *(func)(void *arg), int max1, int max2, int max3) {
  int threadNum = param.nThread;

  if (threadNum == 1) {
    m_threadInfo[0].index1 = 0;
    m_threadInfo[0].index2 = max1;
    m_threadInfo[0].index3 = 0;
    m_threadInfo[0].index4 = max2;
    m_threadInfo[0].index5 = 0;
    m_threadInfo[0].index6 = max3;
    m_threadInfo[0].pX = const_cast<vector<double> *>(&x);
    m_threadInfo[0].pExpX = const_cast<vector<double> *>(&_expX);
    m_threadInfo[0].alpha = _alpha;
    m_threadInfo[0].pNLP = this;
    m_threadInfo[0].pDB = m_pDB;
    m_threadInfo[0].pUsePin = &m_usePin;
    m_threadInfo[0].threadId = 0;
    // kaie 2009-08-29
    m_threadInfo[0].pZ = const_cast<vector<double> *>(&z);
    m_threadInfo[0].pExpZ = const_cast<vector<double> *>(&_expZ);
    // @kaie 2009-08-29
    func((void *)(&m_threadInfo[0]));
    return m_threadResult[0];
  } else {
    int jobCount1 = max1 / threadNum + 1;
    int jobCount2 = max2 / threadNum + 1;
    int jobCount3 = max3 / threadNum + 1;
    vector<pthread_t> threads;
    threads.resize(threadNum);
    for (int i = 0; i < threadNum; i++) {
      int start1 = i * jobCount1;
      int start2 = i * jobCount2;
      int start3 = i * jobCount3;
      int end1 = start1 + jobCount1;
      int end2 = start2 + jobCount2;
      int end3 = start3 + jobCount3;
      if (end1 > max1)
        end1 = max1;
      if (end2 > max2)
        end2 = max2;
      if (end3 > max3)
        end3 = max3;
      m_threadInfo[i].index1 = start1;
      m_threadInfo[i].index2 = end1;
      m_threadInfo[i].index3 = start2;
      m_threadInfo[i].index4 = end2;
      m_threadInfo[i].index5 = start3;
      m_threadInfo[i].index6 = end3;
      m_threadInfo[i].pX = const_cast<vector<double> *>(&x);
      m_threadInfo[i].pExpX = const_cast<vector<double> *>(&_expX);
      m_threadInfo[i].alpha = _alpha;
      m_threadInfo[i].pNLP = this;
      m_threadInfo[i].pDB = m_pDB;
      m_threadInfo[i].pUsePin = &m_usePin;
      m_threadInfo[i].threadId = i;
      // kaie 2009-08-29
      m_threadInfo[i].pZ = const_cast<vector<double> *>(&z);
      m_threadInfo[i].pExpZ = const_cast<vector<double> *>(&_expZ);
      // @kaie 2009-08-29
      pthread_create(&threads[i], NULL, func, (void *)(&m_threadInfo[i]));
    }

    void *thread_result;
    double res = 0;
    for (int i = 0; i < threadNum; i++) {
      pthread_join(threads[i], &thread_result);
      res += m_threadResult[i];
    }
    return res;
  }
}

// static
void *MyNLP::UpdateExpValueForCellAndPinThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  UpdateExpValueForEachCell(pMsg->index2, *pMsg->pX, *pMsg->pZ, *pMsg->pExpX,
                            *pMsg->pExpZ, pMsg->alpha, pMsg->index1,
                            pMsg->pNLP);
  UpdateExpValueForEachPin(pMsg->index4, *pMsg->pX, *pMsg->pZ,
                           pMsg->pNLP->_expPinsZ, pMsg->pNLP->_expPinsZ,
                           pMsg->alpha, pMsg->pNLP, pMsg->pUsePin,
                           pMsg->index3);
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

// static
void *MyNLP::UpdateExpValueForEachCellThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  UpdateExpValueForEachCell(pMsg->index2, *pMsg->pX, *pMsg->pZ, *pMsg->pExpX,
                            *pMsg->pExpZ, pMsg->alpha, pMsg->index1,
                            pMsg->pNLP);
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

// static
void MyNLP::UpdateExpValueForEachCell(
    const int &index2, const vector<double> &x, const vector<double> &z,
    vector<double> &expX, vector<double> &expZ, const double &inAlpha,
    const int &index1, MyNLP *pNLP) {
  if (param.bUseLSE || param.bUseWAE) {
    for (int i = index1; i < index2; i++) {
      expX[2 * i] = exp(x[2 * i] / inAlpha);
      expX[2 * i + 1] = exp(x[2 * i + 1] / inAlpha);
      if (pNLP->m_bMoveZ)
        expZ[i] = exp(z[i] / inAlpha);
    }

    if (m_bXArch) // 2007-09-12
    {
      for (int i = index1; i < index2; i++) {
        pNLP->_expXplusY[i] =
            exp((x[2 * i] + x[2 * i + 1]) / inAlpha); // exp( x+y /k )
        pNLP->_expXminusY[i] =
            exp((x[2 * i] - x[2 * i + 1]) / inAlpha); // exp( x-y /k )
      }
    }
  } else {
    // Lp-norm
    for (int i = index1; i < index2; i++) {
      expX[2 * i] = pow(x[2 * i] * pNLP->m_posScale, inAlpha);
      expX[2 * i + 1] = pow(x[2 * i + 1] * pNLP->m_posScale, inAlpha);
      if (pNLP->m_bMoveZ)
        expZ[i] = pow(z[i] * pNLP->m_posScale, inAlpha);
    }
  }
}

/*static*/
void *MyNLP::UpdateExpValueForEachPinThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  UpdateExpValueForEachPin(pMsg->index2, *pMsg->pX, *pMsg->pZ,
                           pMsg->pNLP->_expPins, pMsg->pNLP->_expPinsZ,
                           pMsg->alpha, pMsg->pNLP, pMsg->pUsePin,
                           pMsg->index1);
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

// static
void MyNLP::UpdateExpValueForEachPin(
    const int &index2, const vector<double> &x, const vector<double> &z,
    vector<double> &expPins, vector<double> &expPinsZ, const double &inAlpha,
    MyNLP *pNLP, const vector<bool> *pUsePin, const int &index1) {
  for (int pinId = index1; pinId < index2; pinId++) {
    int blockId = pNLP->m_pDB->m_pins[pinId].moduleId;

    // TODO: floating-pin! (PlaceDB Bug!)
    if (blockId >= (int)pNLP->m_pDB->m_modules.size())
      continue;

    // 2006-02-20
    if ((*pUsePin)[blockId] == false)
      continue; // save time

    double xx = x[2 * blockId] + pNLP->m_pDB->m_pins[pinId].xOff;
    double yy = x[2 * blockId + 1] + pNLP->m_pDB->m_pins[pinId].yOff;
    double zz = z[blockId];

    if (param.bUseLSE || param.bUseWAE) {
      int netId = pNLP->m_pDB->m_pinNetId[pinId];
      if (param.bStabilityEnhance) {
        expPins[2 * pinId] = exp((xx - pNLP->xMax[netId]) / inAlpha);
        expPins[2 * pinId + 1] = exp((yy - pNLP->yMax[netId]) / inAlpha);
        if (pNLP->m_bMoveZ)
          expPinsZ[pinId] = exp((zz - pNLP->zMax[netId]) / inAlpha);
      } else {
        expPins[2 * pinId] = exp((xx) / inAlpha);
        expPins[2 * pinId + 1] = exp((yy) / inAlpha);
        if (pNLP->m_bMoveZ)
          expPinsZ[pinId] = exp((zz) / inAlpha);
      }

      // cout << "pin[" << pinId << "]: xx=" << xx << ", xMax[" << netId << "]="
      // << pNLP->xMax[netId] << "\n"; cout << "pin[" << pinId << "]: yy=" << yy
      // << ", yMax[" << netId << "]=" << pNLP->yMax[netId] << "\n";

      if (m_bXArch) // 2006-09-12
      {
        pNLP->_expPinXplusY[pinId] = exp((xx + yy) / inAlpha);
        pNLP->_expPinXminusY[pinId] = exp((xx - yy) / inAlpha);
      }
    } else {
      // Lp-norm
      expPins[2 * pinId] = pow(xx * pNLP->m_posScale, inAlpha);
      expPins[2 * pinId + 1] = pow(yy * pNLP->m_posScale, inAlpha);
      if (pNLP->m_bMoveZ)
        expPinsZ[pinId] = pow(zz * pNLP->m_posScale, inAlpha);
      assert(expPins[2 * pinId] != 0);
      assert(expPins[2 * pinId + 1] != 0);
      if (pNLP->m_bMoveZ)
        assert(expPinsZ[pinId] != 0);
    }
  }
}

// static
void *MyNLP::UpdateNetsSumExpThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  UpdateNetsSumExp(*pMsg->pX, *pMsg->pZ, *pMsg->pExpX, *pMsg->pExpZ, pMsg->pNLP,
                   pMsg->index1, pMsg->index2);
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

// static
void MyNLP::UpdateNetsSumExp(const vector<double> &x, const vector<double> &z,
                             const vector<double> &expX,
                             const vector<double> &expZ, MyNLP *pNLP,
                             int index1, int index2) {
  double sum_exp_xi_over_alpha;
  double sum_exp_inv_xi_over_alpha;
  double sum_exp_yi_over_alpha;
  double sum_exp_inv_yi_over_alpha;
  double sum_exp_zi_over_alpha;
  double sum_exp_inv_zi_over_alpha;
  if (index2 > (int)pNLP->m_pDB->m_nets.size())
    index2 = (int)pNLP->m_pDB->m_nets.size();
  for (int n = index1; n < index2; n++) {
    if (pNLP->m_pDB->m_nets[n].size() == 0)
      continue;

    calc_sum_exp_using_pin(
        pNLP->m_pDB->m_nets[n].begin(), pNLP->m_pDB->m_nets[n].end(), x, z,
        expX, expZ, sum_exp_xi_over_alpha, sum_exp_inv_xi_over_alpha,
        sum_exp_yi_over_alpha, sum_exp_inv_yi_over_alpha, sum_exp_zi_over_alpha,
        sum_exp_inv_zi_over_alpha, pNLP->m_pDB, &pNLP->m_usePin, pNLP->_expPins,
        pNLP->_expPinsZ);

    pNLP->m_nets_sum_exp_xi_over_alpha[n] = sum_exp_xi_over_alpha;
    pNLP->m_nets_sum_exp_yi_over_alpha[n] = sum_exp_yi_over_alpha;
    pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] = sum_exp_inv_xi_over_alpha;
    pNLP->m_nets_sum_exp_inv_yi_over_alpha[n] = sum_exp_inv_yi_over_alpha;

    // kaie 2009-08-29
    if (pNLP->m_bMoveZ) {
      pNLP->m_nets_sum_exp_zi_over_alpha[n] = sum_exp_zi_over_alpha;
      pNLP->m_nets_sum_exp_inv_zi_over_alpha[n] = sum_exp_inv_zi_over_alpha;
    }
    // @kaie 2009-08-29

    // frank 2022-07-31
    if (param.b3d && param.bF2FhpwlEnhance) {
      vector<double> layer_sum_exp_xi_over_alpha;
      vector<double> layer_sum_exp_inv_xi_over_alpha;
      vector<double> layer_sum_exp_yi_over_alpha;
      vector<double> layer_sum_exp_inv_yi_over_alpha;
      vector<double> layer_sum_exp_zi_over_alpha;
      vector<double> layer_sum_exp_inv_zi_over_alpha;
      calc_sum_exp_using_pin_for_layers(
          pNLP->m_pDB->m_nets[n].begin(), pNLP->m_pDB->m_nets[n].end(), x, z,
          expX, expZ, layer_sum_exp_xi_over_alpha,
          layer_sum_exp_inv_xi_over_alpha, layer_sum_exp_yi_over_alpha,
          layer_sum_exp_inv_yi_over_alpha, layer_sum_exp_zi_over_alpha,
          layer_sum_exp_inv_zi_over_alpha, pNLP->m_pDB, &pNLP->m_usePin,
          pNLP->_expPins, pNLP->_expPinsZ);

      for (int layer = 0; layer < param.nlayer; ++layer) {
        pNLP->m_layer_nets_sum_exp_xi_over_alpha[layer][n] =
            layer_sum_exp_xi_over_alpha[layer];
        pNLP->m_layer_nets_sum_exp_yi_over_alpha[layer][n] =
            layer_sum_exp_yi_over_alpha[layer];
        pNLP->m_layer_nets_sum_exp_inv_xi_over_alpha[layer][n] =
            layer_sum_exp_inv_xi_over_alpha[layer];
        pNLP->m_layer_nets_sum_exp_inv_yi_over_alpha[layer][n] =
            layer_sum_exp_inv_yi_over_alpha[layer];
        // pNLP->m_layer_nets_sum_exp_zi_over_alpha[layer][n]	  	=
        // layer_sum_exp_zi_over_alpha[layer];
        // pNLP->m_layer_nets_sum_exp_inv_zi_over_alpha[layer][n] =
        // layer_sum_exp_inv_zi_over_alpha[layer];
      }
    }

    if (param.bUseWAE) // (kaie) 2010-10-18 Weighted-Average-Exponential
                       // Wirelength Model
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

      pNLP->m_nets_weighted_sum_exp_xi_over_alpha[n] =
          weighted_sum_exp_xi_over_alpha;
      pNLP->m_nets_weighted_sum_exp_yi_over_alpha[n] =
          weighted_sum_exp_yi_over_alpha;
      pNLP->m_nets_weighted_sum_exp_inv_xi_over_alpha[n] =
          weighted_sum_exp_inv_xi_over_alpha;
      pNLP->m_nets_weighted_sum_exp_inv_yi_over_alpha[n] =
          weighted_sum_exp_inv_yi_over_alpha;

      if (param.b3d && param.bF2FhpwlEnhance) {
        vector<double> layer_weighted_sum_exp_xi_over_alpha;
        vector<double> layer_weighted_sum_exp_inv_xi_over_alpha;
        vector<double> layer_weighted_sum_exp_yi_over_alpha;
        vector<double> layer_weighted_sum_exp_inv_yi_over_alpha;
        calc_weighted_sum_exp_using_pin_for_layers(
            pNLP->m_pDB->m_nets[n].begin(), pNLP->m_pDB->m_nets[n].end(), x,
            expX, z, expZ, layer_weighted_sum_exp_xi_over_alpha,
            layer_weighted_sum_exp_inv_xi_over_alpha,
            layer_weighted_sum_exp_yi_over_alpha,
            layer_weighted_sum_exp_inv_yi_over_alpha, pNLP->m_pDB,
            &pNLP->m_usePin, pNLP->_expPins);

        for (int layer = 0; layer < param.nlayer; ++layer) {
          pNLP->m_layer_nets_weighted_sum_exp_xi_over_alpha[layer][n] =
              layer_weighted_sum_exp_xi_over_alpha[layer];
          pNLP->m_layer_nets_weighted_sum_exp_yi_over_alpha[layer][n] =
              layer_weighted_sum_exp_yi_over_alpha[layer];
          pNLP->m_layer_nets_weighted_sum_exp_inv_xi_over_alpha[layer][n] =
              layer_weighted_sum_exp_inv_xi_over_alpha[layer];
          pNLP->m_layer_nets_weighted_sum_exp_inv_yi_over_alpha[layer][n] =
              layer_weighted_sum_exp_inv_yi_over_alpha[layer];
        }
      }
    }

    if (m_bXArch) {
      double sum_exp_x_plus_y_over_alpha;
      double sum_exp_x_minus_y_over_alpha;
      double sum_exp_inv_x_plus_y_over_alpha;
      double sum_exp_inv_x_minus_y_over_alpha;

      calc_sum_exp_using_pin_XHPWL(
          pNLP->m_pDB->m_nets[n].begin(), pNLP->m_pDB->m_nets[n].end(), pNLP,
          sum_exp_x_plus_y_over_alpha,
          sum_exp_x_minus_y_over_alpha, // reuse variables
          sum_exp_inv_x_plus_y_over_alpha, sum_exp_inv_x_minus_y_over_alpha);

      pNLP->m_nets_sum_exp_x_plus_y_over_alpha[n] = sum_exp_x_plus_y_over_alpha;
      pNLP->m_nets_sum_exp_x_minus_y_over_alpha[n] =
          sum_exp_x_minus_y_over_alpha;
      pNLP->m_nets_sum_exp_inv_x_plus_y_over_alpha[n] =
          sum_exp_inv_x_plus_y_over_alpha;
      pNLP->m_nets_sum_exp_inv_x_minus_y_over_alpha[n] =
          sum_exp_inv_x_minus_y_over_alpha;
    }
  }

  if (param.bUseLSE == false) // for Lp-norm
  {
    for (int n = index1; n < index2; n++) {
      pNLP->m_nets_sum_p_x_pos[n] =
          pow(pNLP->m_nets_sum_exp_xi_over_alpha[n], 1 / pNLP->_alpha - 1);
      pNLP->m_nets_sum_p_y_pos[n] =
          pow(pNLP->m_nets_sum_exp_yi_over_alpha[n], 1 / pNLP->_alpha - 1);
      pNLP->m_nets_sum_p_inv_x_pos[n] =
          pow(pNLP->m_nets_sum_exp_inv_xi_over_alpha[n], 1 / pNLP->_alpha - 1);
      pNLP->m_nets_sum_p_inv_y_pos[n] =
          pow(pNLP->m_nets_sum_exp_inv_yi_over_alpha[n], 1 / pNLP->_alpha - 1);

      pNLP->m_nets_sum_p_x_neg[n] =
          pow(pNLP->m_nets_sum_exp_xi_over_alpha[n], -1 / pNLP->_alpha - 1);
      pNLP->m_nets_sum_p_y_neg[n] =
          pow(pNLP->m_nets_sum_exp_yi_over_alpha[n], -1 / pNLP->_alpha - 1);
      pNLP->m_nets_sum_p_inv_x_neg[n] =
          pow(pNLP->m_nets_sum_exp_inv_xi_over_alpha[n], -1 / pNLP->_alpha - 1);
      pNLP->m_nets_sum_p_inv_y_neg[n] =
          pow(pNLP->m_nets_sum_exp_inv_yi_over_alpha[n], -1 / pNLP->_alpha - 1);

      //// kaie 2009-08-29
      if (pNLP->m_bMoveZ) {
        pNLP->m_nets_sum_p_z_pos[n] =
            pow(pNLP->m_nets_sum_exp_zi_over_alpha[n], 1 / pNLP->_alpha - 1);
        pNLP->m_nets_sum_p_inv_z_pos[n] = pow(
            pNLP->m_nets_sum_exp_inv_zi_over_alpha[n], 1 / pNLP->_alpha - 1);
        pNLP->m_nets_sum_p_z_neg[n] =
            pow(pNLP->m_nets_sum_exp_zi_over_alpha[n], -1 / pNLP->_alpha - 1);
        pNLP->m_nets_sum_p_inv_z_neg[n] = pow(
            pNLP->m_nets_sum_exp_inv_zi_over_alpha[n], -1 / pNLP->_alpha - 1);
      }
      // @kaie 2009-08-29
    }
  }
}

// static
void *MyNLP::GetLogSumExpWLThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  pMsg->pNLP->m_threadResult[pMsg->threadId] =
      GetLogSumExpWL(*pMsg->pX, *pMsg->pExpX, pMsg->alpha, pMsg->pNLP,
                     pMsg->index1, pMsg->index2);
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

// static
double MyNLP::GetLogSumExpWL(const vector<double> &x,    // unuse
                             const vector<double> &expX, // unuse
                             const double &alpha,        // unuse
                             MyNLP *pNLP, int index1, int index2) {

  if (index2 > (int)pNLP->m_pDB->m_nets.size()) // check boundary
    index2 = (int)pNLP->m_pDB->m_nets.size();

  double totalWL = 0;

  for (int n = index1; n < index2; n++) // for each net
  {
    if (pNLP->m_pDB->m_nets[n].size() == 0)
      continue;
    if (param.bUseWAE) {
      if (true == param.bNLPNetWt) {
        totalWL +=
            NetWeightCalc(pNLP->m_pDB->m_nets[n].size()) *
            (pNLP->m_nets_weighted_sum_exp_xi_over_alpha[n] /
                 pNLP->m_nets_sum_exp_xi_over_alpha[n] -
             pNLP->m_nets_weighted_sum_exp_inv_xi_over_alpha[n] /
                 pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] +
             m_yWeight * (pNLP->m_nets_weighted_sum_exp_yi_over_alpha[n] /
                              pNLP->m_nets_sum_exp_yi_over_alpha[n] -
                          pNLP->m_nets_weighted_sum_exp_inv_yi_over_alpha[n] /
                              pNLP->m_nets_sum_exp_inv_yi_over_alpha[n]));
      } else if (param.b3d && param.bF2FhpwlEnhance) {
        for (int layer = 0; layer < param.nlayer; ++layer) {
          if (pNLP->m_layer_nets_sum_exp_xi_over_alpha[layer][n] != 0 &&
              pNLP->m_layer_nets_sum_exp_inv_xi_over_alpha[layer][n] != 0) {
            totalWL +=
                pNLP->m_layer_nets_weighted_sum_exp_xi_over_alpha[layer][n] /
                    pNLP->m_layer_nets_sum_exp_xi_over_alpha[layer][n] -
                pNLP->m_layer_nets_weighted_sum_exp_inv_xi_over_alpha[layer]
                                                                     [n] /
                    pNLP->m_layer_nets_sum_exp_inv_xi_over_alpha[layer][n];
          } else {
            // cout << "xWL of Net["<<n<<"] in layer " << layer << " is 0.\n";
          }
          if (pNLP->m_layer_nets_sum_exp_yi_over_alpha[layer][n] != 0 &&
              pNLP->m_layer_nets_sum_exp_inv_yi_over_alpha[layer][n] != 0) {
            totalWL +=
                m_yWeight *
                (pNLP->m_layer_nets_weighted_sum_exp_yi_over_alpha[layer][n] /
                     pNLP->m_layer_nets_sum_exp_yi_over_alpha[layer][n] -
                 pNLP->m_layer_nets_weighted_sum_exp_inv_yi_over_alpha[layer][n] /
                     pNLP->m_layer_nets_sum_exp_inv_yi_over_alpha[layer][n]);
          } else {
            // cout << "yWL of Net["<<n<<"] in layer " << layer << " is 0.\n";
          }
        }
      } else {
        totalWL +=
            (pNLP->m_nets_weighted_sum_exp_xi_over_alpha[n] /
                 pNLP->m_nets_sum_exp_xi_over_alpha[n] -
             pNLP->m_nets_weighted_sum_exp_inv_xi_over_alpha[n] /
                 pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] +
             m_yWeight * (pNLP->m_nets_weighted_sum_exp_yi_over_alpha[n] /
                              pNLP->m_nets_sum_exp_yi_over_alpha[n] -
                          pNLP->m_nets_weighted_sum_exp_inv_yi_over_alpha[n] /
                              pNLP->m_nets_sum_exp_inv_yi_over_alpha[n]));
      }
    } else if (param.bUseLSE) {
      if (false == m_bXArch) {
        /*totalWL +=
        log( pNLP->m_nets_sum_exp_xi_over_alpha[n] ) +	    // max(x)
        log( pNLP->m_nets_sum_exp_inv_xi_over_alpha[n] ) +  // -min(x)
        log( pNLP->m_nets_sum_exp_yi_over_alpha[n] ) +	    // max(y)
        log( pNLP->m_nets_sum_exp_inv_yi_over_alpha[n] ) ;  // -min(y)*/
        if (true == param.bNLPNetWt) {
          totalWL +=
              NetWeightCalc(pNLP->m_pDB->m_nets[n].size()) *
              (log(pNLP->m_nets_sum_exp_xi_over_alpha[n]) +     // max(x)
               log(pNLP->m_nets_sum_exp_inv_xi_over_alpha[n]) + // -min(x)
               m_yWeight *
                   (log(pNLP->m_nets_sum_exp_yi_over_alpha[n]) + // max(y)
                    log(pNLP->m_nets_sum_exp_inv_yi_over_alpha[n])));
        } else if (param.b3d && param.bF2FhpwlEnhance) {
          totalWL +=
              log(pNLP->m_layer_nets_sum_exp_xi_over_alpha[0][n]) + // max(x) in
                                                                    // die0
              log(pNLP->m_layer_nets_sum_exp_inv_xi_over_alpha[0][n]) + // -min(x)
                                                                     // in die0
              m_yWeight * (log(pNLP->m_layer_nets_sum_exp_yi_over_alpha[0][n]) + // max(y) in die0
                           log(pNLP->m_layer_nets_sum_exp_inv_yi_over_alpha[0][n])) + // -min(y) in die0
              log(pNLP->m_layer_nets_sum_exp_xi_over_alpha[1][n]) + // max(x) in
                                                                    // die1
              log(pNLP->m_layer_nets_sum_exp_inv_xi_over_alpha[1][n]) + // -min(x)
                                                                     // in die1
              m_yWeight * (log(pNLP->m_layer_nets_sum_exp_yi_over_alpha[1][n]) + // max(y) in die1
                           log(pNLP->m_layer_nets_sum_exp_inv_yi_over_alpha[1][n])); // -min(y) in die1
        } else {
          totalWL +=
              log(pNLP->m_nets_sum_exp_xi_over_alpha[n]) +     // max(x)
              log(pNLP->m_nets_sum_exp_inv_xi_over_alpha[n]) + // -min(x)
              m_yWeight *
                  (log(pNLP->m_nets_sum_exp_yi_over_alpha[n]) +     // max(y)
                   log(pNLP->m_nets_sum_exp_inv_yi_over_alpha[n])); // -min(y)
        }
      } else {
        // X-HPWL
        totalWL +=
            (sqrt(2) - 1.0) *
                (log(pNLP->m_nets_sum_exp_xi_over_alpha[n]) +     // max(x)
                 log(pNLP->m_nets_sum_exp_inv_xi_over_alpha[n]) + // -min(x)
                 log(pNLP->m_nets_sum_exp_yi_over_alpha[n]) +     // max(y)
                 log(pNLP->m_nets_sum_exp_inv_yi_over_alpha[n])   // -min(y)
                 ) -
            (sqrt(2) / 2.0 - 1.0) *
                (log(pNLP->m_nets_sum_exp_x_plus_y_over_alpha[n]) + // max(x+y)
                 log(pNLP->m_nets_sum_exp_inv_x_plus_y_over_alpha
                         [n]) + // -min(x+y)
                 log(pNLP->m_nets_sum_exp_x_minus_y_over_alpha[n]) + // max(x-y)
                 log(pNLP->m_nets_sum_exp_inv_x_minus_y_over_alpha
                         [n]) // -min(x-y)
                );
      }
    } else {
      // LP-norm
      double invAlpha = 1.0 / pNLP->_alpha;
      totalWL +=
          pow(pNLP->m_nets_sum_exp_xi_over_alpha[n], invAlpha) -
          pow(pNLP->m_nets_sum_exp_inv_xi_over_alpha[n], -invAlpha) +
          m_yWeight *
              (pow(pNLP->m_nets_sum_exp_yi_over_alpha[n], invAlpha) -
               pow(pNLP->m_nets_sum_exp_inv_yi_over_alpha[n], -invAlpha));
    }
  }
  if (param.bUseWAE)
    return totalWL;
  else if (param.bUseLSE)
    return totalWL * pNLP->_alpha;
  else
    return totalWL / pNLP->m_posScale;
}

// (kaie) 2009-10-14
void *MyNLP::GetLogSumExpViaThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  pMsg->pNLP->m_threadResult[pMsg->threadId] =
      GetLogSumExpVia(*pMsg->pZ, *pMsg->pExpZ, pMsg->alpha, pMsg->pNLP,
                      pMsg->index1, pMsg->index2);
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

double MyNLP::GetLogSumExpVia(const vector<double> &z,    // unuse
                              const vector<double> &expZ, // unuse
                              const double &alpha,        // unuse
                              MyNLP *pNLP, int index1, int index2) {

  if (index2 > (int)pNLP->m_pDB->m_nets.size()) // check boundary
    index2 = (int)pNLP->m_pDB->m_nets.size();

  double totalVia = 0;

  for (int n = index1; n < index2; n++) // for each net
  {
    if (pNLP->m_pDB->m_nets[n].size() == 0)
      continue;
    if (param.bUseLSE) {
      /*totalVia +=
      log( pNLP->m_nets_sum_exp_zi_over_alpha[n] ) +	    // max(z)
      log( pNLP->m_nets_sum_exp_inv_zi_over_alpha[n] ) ;*/  // -min(z)
      if (true == param.bNLPNetWt) {
        totalVia += NetWeightCalc(pNLP->m_pDB->m_nets[n].size()) *
                    (log(pNLP->m_nets_sum_exp_zi_over_alpha[n]) +     // max(z)
                     log(pNLP->m_nets_sum_exp_inv_zi_over_alpha[n])); // -min(z)
      } else {
        totalVia += log(pNLP->m_nets_sum_exp_xi_over_alpha[n]) +    // max(z)
                    log(pNLP->m_nets_sum_exp_inv_xi_over_alpha[n]); // -min(z)
      }
    } else {
      // LP-norm
      double invAlpha = 1.0 / pNLP->_alpha;
      totalVia += pow(pNLP->m_nets_sum_exp_zi_over_alpha[n], invAlpha) -
                  pow(pNLP->m_nets_sum_exp_inv_zi_over_alpha[n], -invAlpha);
      /*totalVia +=
      pow( pNLP->m_nets_sum_exp_zi_over_alpha[n], invAlpha ) -
      pow( pNLP->m_nets_sum_exp_inv_zi_over_alpha[n], -invAlpha );*/
      // assert( !isNaN( totalVia ) );
    }
  }
  if (param.bUseLSE)
    return totalVia * pNLP->_alpha;
  else
    return totalVia / pNLP->m_posScale;
}
// @(kaie) 2009-10-14

bool MyNLP::eval_f(int n, const vector<double> &x, const vector<double> &expX,
                   bool new_x, double &obj_value) {
  // totalWL = GetLogSumExpWL( x, expX, _alpha, this );
  lastWL_ = curWL_;
  double totalWL = Parallel(GetLogSumExpWLThread, m_pDB->m_nets.size());
  curWL_ = totalWL;
  double totalVia = 0;
  if (m_bMoveZ)
    totalVia = Parallel(GetLogSumExpViaThread, m_pDB->m_nets.size());
  gTotalWL = totalWL;
  // gTotalVia = totalVia;
  density = GetDensityPanelty();
  if (bMulti) {
    obj_value = (totalWL * _weightWire) + 0.5 * (density) +
                (totalVia * _weightWire * m_weightTSV); // correct.
  } else if (!m_bMoveZ) {
    obj_value = (totalWL * _weightWire) + 0.5 * (density * _weightDensity);
    if(param.bShow)
      cout << "obj_value = " << totalWL << " * " << _weightWire << " + 0.5 * " << density << " * " << _weightDensity << " = " << obj_value << "\n";
  } else {
    obj_value = (totalWL * _weightWire) + 0.5 * (density * _weightDensity) +
                (totalVia * _weightWire * m_weightTSV) * 0.01; // correct.
  }


  return true;
}

void MyNLP::PrintPotentialGrid() {
  for (int k = (int)m_gridPotential.size() - 1; k >= 0; k--) {
    printf("===layer%d===\n", k);
    for (int i = (int)m_gridPotential[k].size() - 1; i >= 0; i--) {
      for (unsigned int j = 0; j < m_gridPotential[k][i].size(); j++)
        printf("%4.1f ",
               (m_gridPotential[k][i][j] - m_expBinPotential[k][i][j]) /
                   m_expBinPotential[k][i][j]);
      printf("\n");
    }
    printf("\n\n");
  }
}

double MyNLP::GetDensityPanelty() {
  double density = 0;
  for (unsigned int k = 0; k < m_gridPotential.size(); k++) {
    for (unsigned int i = 0; i < m_gridPotential[k].size(); i++) {
      for (unsigned int j = 0; j < m_gridPotential[k][i].size(); j++) {
        if (bMulti) {

          double p = m_weightDensity[k][i][j] *
                     (m_gridPotential[k][i][j] - m_expBinPotential[k][i][j]) *
                     (m_gridPotential[k][i][j] - m_expBinPotential[k][i][j]);

          if (m_skewDensityPenalty1 != 1.0) {
            if (m_gridPotential[k][i][j] < m_expBinPotential[k][i][j])
              p /= m_skewDensityPenalty2;
            else
              p *= m_skewDensityPenalty1;
          }

          density += p;
        } else {
          density += (m_gridPotential[k][i][j] - m_expBinPotential[k][i][j]) *
                     (m_gridPotential[k][i][j] - m_expBinPotential[k][i][j]);
        }
      }
    }
  }

  return density;
}

void MyNLP::InitModuleNetPinId() {
  // printf( "Init module-net-pin id\n" );
  m_moduleNetPinId.resize(m_pDB->m_modules.size());
  for (unsigned int i = 0; i < m_pDB->m_modules.size(); i++) {
    m_moduleNetPinId[i].resize(m_pDB->m_modules[i].m_netsId.size(), -1);
    for (unsigned int j = 0; j < m_pDB->m_modules[i].m_netsId.size(); j++) {
      int netId = m_pDB->m_modules[i].m_netsId[j];
      int pinId = -1;
      for (unsigned int p = 0; p < m_pDB->m_nets[netId].size(); p++) {
        if (m_pDB->m_pins[m_pDB->m_nets[netId][p]].moduleId == (int)i) {
          pinId = m_pDB->m_nets[netId][p];
          break;
        }
      }
      assert(pinId != -1); // net without pin?
      m_moduleNetPinId[i][j] = pinId;
    } // each net to the module
  }   // each module
}

// static
void *MyNLP::eval_grad_f_thread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);

  double tStart = seconds();
  UpdateGradWire(pMsg->pNLP, pMsg->index1, pMsg->index2); // wire forces
  time_wire_force += seconds() - tStart; // CPU time is wrong if th != 1

  tStart = seconds();
  UpdateGradPotential(pMsg->pNLP, pMsg->index1, pMsg->index2); // spreading forces
  time_spreading_force += seconds() - tStart;

  if (pMsg->pNLP->m_bMoveZ) {
    tStart = seconds();
    UpdateGradVia(pMsg->pNLP, pMsg->index1, pMsg->index2);
    time_via_force += seconds() - tStart;
  }

  // for( int i=2*pMsg->index1; i<2*pMsg->index2; i++ )
  for (int i = pMsg->index1; i < pMsg->index2; i++) // index for modules
  {
    // directions
    //cout << "_weightDensity = " << pMsg->pNLP->_weightDensity << "\n";
    double grad_f_x =
        pMsg->pNLP->_weightWire * pMsg->pNLP->grad_wire[2 * i] +
        pMsg->pNLP->_weightDensity * pMsg->pNLP->grad_potential[2 * i];
    double grad_f_y =
        pMsg->pNLP->_weightWire * pMsg->pNLP->grad_wire[2 * i + 1] +
        pMsg->pNLP->_weightDensity * pMsg->pNLP->grad_potential[2 * i + 1];

    if (pMsg->pNLP->m_bMoveZ) {
      // (kaie) 2009-09-12 z direction
      double grad_f_z =
          pMsg->pNLP->_weightWire * pMsg->pNLP->m_weightTSV *
              pMsg->pNLP->grad_via[i] +
          pMsg->pNLP->_weightDensity * pMsg->pNLP->grad_potentialZ[i];
      // @(kaie) 2009-09-12
      pMsg->pNLP->grad_f[3 * i] = grad_f_x;
      pMsg->pNLP->grad_f[3 * i + 1] = grad_f_y;
      pMsg->pNLP->grad_f[3 * i + 2] = grad_f_z;
    } else {
      pMsg->pNLP->grad_f[2 * i] = grad_f_x;
      pMsg->pNLP->grad_f[2 * i + 1] = grad_f_y;
    }
  }

  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

bool MyNLP::eval_grad_f(int n, const vector<double> &x,
                        const vector<double> &expX, bool new_x,
                        vector<double> &grad_f) {

  // grad WL
  if (_weightWire > 0) // TEST
    Parallel(UpdateGradWireThread, (int)m_pDB->m_modules.size());

  // grad Density
  Parallel(UpdateGradPotentialThread, (int)m_pDB->m_modules.size());

  if (m_bMoveZ)
    Parallel(UpdateGradViaThread, (int)m_pDB->m_modules.size());

  if (m_bMoveZ) {
    if (n % 3 != 0)
      printf("ERROR: Inconsistent module number!\n");
    n /= 3;
  } else {
    if (n % 2 != 0)
      printf("ERROR: Inconsistent module number!\n");
    n /= 2;
  }
  // compute total fouce, x and y directions
  if (bMulti) {
    for (int i = 0; i < n; i++) {
      double grad_f_x = _weightWire * grad_wire[2 * i] + grad_potential[2 * i];
      double grad_f_y =
          _weightWire * grad_wire[2 * i + 1] + grad_potential[2 * i + 1];
      if (m_bMoveZ) {
        // (kaie) 2009-09-12 z direction
        double grad_f_z =
            _weightWire * m_weightTSV * grad_via[i] + grad_potentialZ[i];
        // @(kaie) 2009-09-12
        grad_f[3 * i] = grad_f_x;
        grad_f[3 * i + 1] = grad_f_y;
        grad_f[3 * i + 2] = grad_f_z;
      } else {
        grad_f[2 * i] = grad_f_x;
        grad_f[2 * i + 1] = grad_f_y;
      }
    }
  } else {
    for (int i = 0; i < n; i++) {
      double grad_f_x = _weightWire * grad_wire[2 * i] +
                        _weightDensity * grad_potential[2 * i];
      double grad_f_y = _weightWire * grad_wire[2 * i + 1] +
                        _weightDensity * grad_potential[2 * i + 1];
      if (m_bMoveZ) {
        // (kaie) 2009-09-12 z direction
        double grad_f_z = _weightWire * m_weightTSV * grad_via[i] +
                          _weightDensity * grad_potentialZ[i];
        // @(kaie) 2009-09-12
        grad_f[3 * i] = grad_f_x;
        grad_f[3 * i + 1] = grad_f_y;
        grad_f[3 * i + 2] = grad_f_z;
      } else {
        grad_f[2 * i] = grad_f_x;
        grad_f[2 * i + 1] = grad_f_y;
      }
    }
  }
  return true;
}

// static
void *MyNLP::UpdateGradWireThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  UpdateGradWire(pMsg->pNLP, pMsg->index1, pMsg->index2);
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

// static
void MyNLP::UpdateGradWire(MyNLP *pNLP, int index1, int index2) {
  if (index2 > (int)pNLP->m_pDB->m_modules.size())
    index2 = (int)pNLP->m_pDB->m_modules.size();
  for (int i = index1; i < index2; i++) // for each block
  {
    if (pNLP->m_pDB->m_modules[i].m_isFixed ||
        pNLP->m_pDB->m_modules[i].m_netsId.size() == 0)
      continue;

    pNLP->grad_wire[2 * i] = 0;
    pNLP->grad_wire[2 * i + 1] = 0;

    for (unsigned int j = 0; j < pNLP->m_pDB->m_modules[i].m_netsId.size(); j++) {
      // for each net connecting to the block
      int netId = pNLP->m_pDB->m_modules[i].m_netsId[j];
      if (pNLP->m_pDB->m_nets[netId].size() == 0) // floating-module
        continue;

      int selfPinId = pNLP->m_moduleNetPinId[i][j];

      if (pNLP->m_usePin[i]) {
        if (param.bUseWAE) {
          if (param.b3d && param.bF2FhpwlEnhance) {
            if(pNLP->m_pDB->m_modules[i].m_isVia){ // via
              for(int k=0;k<param.nlayer;++k){
                if (pNLP->m_layer_nets_sum_exp_xi_over_alpha[k][netId] != 0 && pNLP->m_layer_nets_sum_exp_inv_xi_over_alpha[k][netId] != 0) {
                  pNLP->grad_wire[2 * i] +=
                      pNLP->x[2 * i] * pNLP->_expPins[2 * selfPinId] / pNLP->m_layer_nets_sum_exp_xi_over_alpha[k][netId] -
                      pNLP->x[2 * i] * (1.0/pNLP->_expPins[2 * selfPinId]) / pNLP->m_layer_nets_sum_exp_inv_xi_over_alpha[k][netId];
                }
                if (pNLP->m_layer_nets_sum_exp_yi_over_alpha[k][netId] != 0 && pNLP->m_layer_nets_sum_exp_inv_yi_over_alpha[k][netId] != 0) {
                  pNLP->grad_wire[2 * i + 1] +=
                      pNLP->x[2 * i + 1] * pNLP->_expPins[2 * selfPinId + 1] / pNLP->m_layer_nets_sum_exp_yi_over_alpha[k][netId] -
                      pNLP->x[2 * i + 1] * (1.0/pNLP->_expPins[2 * selfPinId + 1]) / pNLP->m_layer_nets_sum_exp_inv_yi_over_alpha[k][netId];
                }
              }
              pNLP->grad_wire[2 * i] /= param.nlayer;
              pNLP->grad_wire[2 * i + 1] /= param.nlayer;
            } else{ // cells
              int layer = (pNLP->z[i] < pNLP->m_pDB->m_dCutline) ? 0 : 1;
              if (pNLP->m_layer_nets_sum_exp_xi_over_alpha[layer][netId] != 0 && pNLP->m_layer_nets_sum_exp_inv_xi_over_alpha[layer][netId] != 0) {
                pNLP->grad_wire[2 * i] +=
                    pNLP->x[2 * i] * pNLP->_expPins[2 * selfPinId] / pNLP->m_layer_nets_sum_exp_xi_over_alpha[layer][netId] -
                    pNLP->x[2 * i] * (1.0/pNLP->_expPins[2 * selfPinId]) / pNLP->m_layer_nets_sum_exp_inv_xi_over_alpha[layer][netId];
              }
              if (pNLP->m_layer_nets_sum_exp_yi_over_alpha[layer][netId] != 0 && pNLP->m_layer_nets_sum_exp_inv_yi_over_alpha[layer][netId] != 0) {
                pNLP->grad_wire[2 * i + 1] +=
                    pNLP->x[2 * i + 1] * pNLP->_expPins[2 * selfPinId + 1] / pNLP->m_layer_nets_sum_exp_yi_over_alpha[layer][netId] -
                    pNLP->x[2 * i + 1] * (1.0/pNLP->_expPins[2 * selfPinId + 1]) / pNLP->m_layer_nets_sum_exp_inv_yi_over_alpha[layer][netId];
              }
            }
          } else{ // !param.bF2FhpwlEnhance
            pNLP->grad_wire[2 * i] +=
                pNLP->x[2 * i] * pNLP->_expPins[2 * selfPinId] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
                pNLP->x[2 * i] * (1.0/pNLP->_expPins[2 * selfPinId]) / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId];
            pNLP->grad_wire[2 * i + 1] +=
                pNLP->x[2 * i + 1] * pNLP->_expPins[2 * selfPinId + 1] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
                pNLP->x[2 * i + 1] * (1.0/pNLP->_expPins[2 * selfPinId + 1]) / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId];
          }
        } else if (param.bUseLSE) {
          if (false == m_bXArch) {
            if (true == param.bNLPNetWt) {
              pNLP->grad_wire[2 * i] +=
                  NetWeightCalc(pNLP->m_pDB->m_nets[netId].size()) *
                  (pNLP->_expPins[2 * selfPinId] /
                       pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
                   1.0 / pNLP->_expPins[2 * selfPinId] /
                       pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId]);
              pNLP->grad_wire[2 * i + 1] +=
                  NetWeightCalc(pNLP->m_pDB->m_nets[netId].size()) *
                  (pNLP->_expPins[2 * selfPinId + 1] /
                       pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
                   1.0 / pNLP->_expPins[2 * selfPinId + 1] /
                       pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId]);
            } else {
              pNLP->grad_wire[2 * i] +=
                  pNLP->_expPins[2 * selfPinId] / pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
                  1.0 / pNLP->_expPins[2 * selfPinId] / pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId];
              pNLP->grad_wire[2 * i + 1] +=
                  pNLP->_expPins[2 * selfPinId + 1] / pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
                  1.0 / pNLP->_expPins[2 * selfPinId + 1] / pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId];
            }
          } else {
            pNLP->grad_wire[2 * i] +=
                (sqrt(2.0) - 1.0) *
                    (pNLP->_expPins[2 * selfPinId] /
                         pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
                     1.0 / pNLP->_expPins[2 * selfPinId] /
                         pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId]) -
                (sqrt(2.0) / 2.0 - 1.0) *
                    (pNLP->_expPinXplusY[selfPinId] /
                         pNLP->m_nets_sum_exp_x_plus_y_over_alpha[netId] -
                     1.0 / pNLP->_expPinXplusY[selfPinId] /
                         pNLP->m_nets_sum_exp_inv_x_plus_y_over_alpha[netId] +
                     pNLP->_expPinXminusY[selfPinId] /
                         pNLP->m_nets_sum_exp_x_minus_y_over_alpha[netId] -
                     1.0 / pNLP->_expPinXminusY[selfPinId] /
                         pNLP->m_nets_sum_exp_inv_x_minus_y_over_alpha[netId]);
            pNLP->grad_wire[2 * i + 1] +=
                (sqrt(2.0) - 1.0) *
                    (pNLP->_expPins[2 * selfPinId + 1] /
                         pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
                     1.0 / pNLP->_expPins[2 * selfPinId + 1] /
                         pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId]) -
                (sqrt(2.0) / 2.0 - 1.0) *
                    (pNLP->_expPinXplusY[selfPinId] /
                         pNLP->m_nets_sum_exp_x_plus_y_over_alpha[netId] -
                     1.0 / pNLP->_expPinXplusY[selfPinId] /
                         pNLP->m_nets_sum_exp_inv_x_plus_y_over_alpha[netId] -
                     pNLP->_expPinXminusY[selfPinId] /
                         pNLP->m_nets_sum_exp_x_minus_y_over_alpha[netId] +
                     1.0 / pNLP->_expPinXminusY[selfPinId] /
                         pNLP->m_nets_sum_exp_inv_x_minus_y_over_alpha[netId]);
          }
        } else {
          // LP-norm
          double xx = pNLP->x[2 * i] + pNLP->m_pDB->m_pins[selfPinId].xOff;
          double yy = pNLP->x[2 * i + 1] + pNLP->m_pDB->m_pins[selfPinId].yOff;
          // assert( xx != 0 );
          // assert( yy != 0 );
          xx *= pNLP->m_posScale;
          yy *= pNLP->m_posScale;
          pNLP->grad_wire[2 * i] += pNLP->m_nets_sum_p_x_pos[netId] *
                                        pNLP->_expPins[2 * selfPinId] / xx -
                                    pNLP->m_nets_sum_p_inv_x_neg[netId] /
                                        pNLP->_expPins[2 * selfPinId] / xx;
          pNLP->grad_wire[2 * i + 1] +=
              pNLP->m_nets_sum_p_y_pos[netId] *
                  pNLP->_expPins[2 * selfPinId + 1] / yy -
              pNLP->m_nets_sum_p_inv_y_neg[netId] /
                  pNLP->_expPins[2 * selfPinId + 1] / yy;
        }
      } else {
        if (param.bUseLSE) {
          if (false == m_bXArch) {
            if (true == param.bNLPNetWt) {
              pNLP->grad_wire[2 * i] +=
                  NetWeightCalc(pNLP->m_pDB->m_nets[netId].size()) *
                  (pNLP->_expX[2 * i] /
                       pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
                   1.0 / pNLP->_expX[2 * i] /
                       pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId]);
              pNLP->grad_wire[2 * i + 1] +=
                  NetWeightCalc(pNLP->m_pDB->m_nets[netId].size()) *
                  (pNLP->_expX[2 * i + 1] /
                       pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
                   1.0 / pNLP->_expX[2 * i + 1] /
                       pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId]);
            } else {
              pNLP->grad_wire[2 * i] +=
                  pNLP->_expX[2 * i] /
                      pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
                  1.0 / pNLP->_expX[2 * i] /
                      pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId];
              pNLP->grad_wire[2 * i + 1] +=
                  pNLP->_expX[2 * i + 1] /
                      pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
                  1.0 / pNLP->_expX[2 * i + 1] /
                      pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId];
            }
          } else {
            pNLP->grad_wire[2 * i] +=
                (sqrt(2.0) - 1.0) *
                    (pNLP->_expX[2 * i] /
                         pNLP->m_nets_sum_exp_xi_over_alpha[netId] -
                     1.0 / pNLP->_expX[2 * i] /
                         pNLP->m_nets_sum_exp_inv_xi_over_alpha[netId]) -
                (sqrt(2.0) / 2.0 - 1.0) *
                    (pNLP->_expXplusY[i] /
                         pNLP->m_nets_sum_exp_x_plus_y_over_alpha[netId] -
                     1.0 / pNLP->_expXplusY[i] /
                         pNLP->m_nets_sum_exp_inv_x_plus_y_over_alpha[netId] +
                     pNLP->_expXminusY[i] /
                         pNLP->m_nets_sum_exp_x_minus_y_over_alpha[netId] -
                     1.0 / pNLP->_expXminusY[i] /
                         pNLP->m_nets_sum_exp_inv_x_minus_y_over_alpha[netId]);
            pNLP->grad_wire[2 * i + 1] +=
                (sqrt(2.0) - 1.0) *
                    (pNLP->_expX[2 * i + 1] /
                         pNLP->m_nets_sum_exp_yi_over_alpha[netId] -
                     1.0 / pNLP->_expX[2 * i + 1] /
                         pNLP->m_nets_sum_exp_inv_yi_over_alpha[netId]) -
                (sqrt(2.0) / 2.0 - 1.0) *
                    (pNLP->_expXplusY[i] /
                         pNLP->m_nets_sum_exp_x_plus_y_over_alpha[netId] -
                     1.0 / pNLP->_expXplusY[i] /
                         pNLP->m_nets_sum_exp_inv_x_plus_y_over_alpha[netId] -
                     pNLP->_expXminusY[i] /
                         pNLP->m_nets_sum_exp_x_minus_y_over_alpha[netId] +
                     1.0 / pNLP->_expXminusY[i] /
                         pNLP->m_nets_sum_exp_inv_x_minus_y_over_alpha[netId]);
          }
        } else {
          // Lp-norm
          double xx = pNLP->x[2 * i];
          double yy = pNLP->x[2 * i + 1];
          xx *= pNLP->m_posScale;
          yy *= pNLP->m_posScale;
          pNLP->grad_wire[2 * i] +=
              pNLP->m_nets_sum_p_x_pos[netId] * pNLP->_expX[2 * i] / xx -
              pNLP->m_nets_sum_p_inv_x_neg[netId] / pNLP->_expX[2 * i] / xx;
          pNLP->grad_wire[2 * i + 1] +=
              pNLP->m_nets_sum_p_y_pos[netId] * pNLP->_expX[2 * i + 1] / yy -
              pNLP->m_nets_sum_p_inv_y_neg[netId] / pNLP->_expX[2 * i + 1] / yy;
        }
      }

    } // for each pin in the module

    // wirelength feature for gnn partition
    if(param.nGNNFeature > 0
      && !pNLP->m_pDB->m_modules[i].m_isVia && !pNLP->m_pDB->m_modules[i].m_isFiller ){
      pNLP->m_pDB->m_modules[i].m_vFeatures[5] = pNLP->grad_wire[2*i]; // pullForce_x
      pNLP->m_pDB->m_modules[i].m_vFeatures[6] = pNLP->grad_wire[2*i+1]; // pullForce_y
    }
  }   // for each module

  // 2006-09-27 Y-weight (donnie)
  for (int i = index1; i < index2; i++) // for each block
    pNLP->grad_wire[2 * i + 1] *= m_yWeight;
}

// kaie
// static
void *MyNLP::UpdateGradViaThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  UpdateGradVia(pMsg->pNLP, pMsg->index1, pMsg->index2);
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

// static
void MyNLP::UpdateGradVia(MyNLP *pNLP, int index1, int index2) {
  if (index2 > (int)pNLP->m_pDB->m_modules.size())
    index2 = (int)pNLP->m_pDB->m_modules.size();
  for (int i = index1; i < index2; i++) // for each block
  {
    if (pNLP->m_pDB->m_modules[i].m_isFixed ||
        pNLP->m_pDB->m_modules[i].m_netsId.size() == 0)
      continue;

    // if( pNLP->cellLock[i] == true )
    //    continue;

    pNLP->grad_via[i] = 0;

    for (unsigned int j = 0; j < pNLP->m_pDB->m_modules[i].m_netsId.size(); j++) {
      // for each net connecting to the block
      int netId = pNLP->m_pDB->m_modules[i].m_netsId[j];
      if (pNLP->m_pDB->m_nets[netId].size() == 0) // floating-module
        continue;

      int selfPinId = pNLP->m_moduleNetPinId[i][j];
      // if( selfPinId == -1 )
      //	continue;

      if (pNLP->m_usePin[i]) {
        assert(selfPinId != -1);
        if (param.bUseLSE) {
          if (true == param.bNLPNetWt) {
            pNLP->grad_via[i] +=
                NetWeightCalc(pNLP->m_pDB->m_nets[netId].size()) *
                (pNLP->_expPinsZ[selfPinId] /
                     pNLP->m_nets_sum_exp_zi_over_alpha[netId] -
                 1.0 / pNLP->_expPinsZ[selfPinId] /
                     pNLP->m_nets_sum_exp_inv_zi_over_alpha[netId]);
          } else {
            pNLP->grad_via[i] +=
                pNLP->_expPinsZ[selfPinId] /
                    pNLP->m_nets_sum_exp_zi_over_alpha[netId] -
                1.0 / pNLP->_expPinsZ[selfPinId] /
                    pNLP->m_nets_sum_exp_inv_zi_over_alpha[netId];
          }
        } else {
          // LP-norm
          double zz = pNLP->z[i];
          zz *= pNLP->m_posScale;
          pNLP->grad_via[i] += pNLP->m_nets_sum_p_z_pos[netId] *
                                   pNLP->_expPinsZ[selfPinId] / zz -
                               pNLP->m_nets_sum_p_inv_z_neg[netId] /
                                   pNLP->_expPinsZ[selfPinId] / zz;
#if 0
					assert( !isNaN( pNLP->grad_via[ i ] ) );
					assert( fabs( pNLP->grad_via[ i ] ) < DBL_MAX * 0.95 );
#endif
        }
      } else {
        // use cell centers

        if (param.bUseLSE) {
          if (true == param.bNLPNetWt) {
            pNLP->grad_via[i] +=
                NetWeightCalc(pNLP->m_pDB->m_nets[netId].size()) *
                (pNLP->_expZ[i] / pNLP->m_nets_sum_exp_zi_over_alpha[netId] -
                 1.0 / pNLP->_expZ[i] /
                     pNLP->m_nets_sum_exp_inv_zi_over_alpha[netId]);
          } else {
            pNLP->grad_via[i] +=
                pNLP->_expZ[i] / pNLP->m_nets_sum_exp_zi_over_alpha[netId] -
                1.0 / pNLP->_expZ[i] /
                    pNLP->m_nets_sum_exp_inv_zi_over_alpha[netId];
          }
        } else {
          // Lp-norm
          double zz = pNLP->z[i];
          zz *= pNLP->m_posScale;
          pNLP->grad_via[i] +=
              pNLP->m_nets_sum_p_z_pos[netId] * pNLP->_expZ[2 * i] / zz -
              pNLP->m_nets_sum_p_inv_z_neg[netId] / pNLP->_expZ[2 * i] / zz;
#if 0
					assert( !isNaN( pNLP->grad_via[ i ] ) );
					assert( fabs( pNLP->grad_via[ i ] ) < DBL_MAX * 0.95 );
#endif
        }
      }

    } // for each pin in the module
      // printf("via force(%d): %lf\n", i, pNLP->grad_via[i]);
  }   // for each module
}
// @kaie

// static
void *MyNLP::UpdateGradPotentialThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  UpdateGradPotential(pMsg->pNLP, pMsg->index1, pMsg->index2);
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

// static
void MyNLP::UpdateGradPotential(MyNLP *pNLP, int index1, int index2) {
  double gradDensityX;
  double gradDensityY;
  double gradDensityZ;
  if (index2 > (int)pNLP->m_pDB->m_modules.size())
    index2 = (int)pNLP->m_pDB->m_modules.size();
  for (int i = index1; i < index2; i++) // for each cell
  {
    if (pNLP->m_pDB->m_modules[i].m_isFixed || pNLP->m_pDB->m_modules[i].m_isVia)
      continue;
    // if( pNLP->cellLock[i] == true )
    //    continue;

    pNLP->grad_potential[2 * i] = 0.0;
    pNLP->grad_potential[2 * i + 1] = 0.0;
    if (pNLP->m_bMoveZ)
      pNLP->grad_potentialZ[i] = 0.0;

    if (param.bUseEDensity){
      GetPotentialGrad_eDensity(pNLP->x, pNLP->z, i, gradDensityX, gradDensityY, gradDensityZ, pNLP);
    } else{
      if (bFast) {
        double width = pNLP->m_pDB->m_modules[i].GetWidth(pNLP->z[i] - 0.5);
        double height = pNLP->m_pDB->m_modules[i].GetHeight(pNLP->z[i] - 0.5);
        // double thickness = pNLP->m_pDB->m_modules[i].m_thickness;
        if (height >= pNLP->m_potentialGridHeight || width >= pNLP->m_potentialGridWidth) // || thickness >= pNLP->m_potentialGridThickness )
        {
          GetPotentialGrad(pNLP->x, pNLP->z, i, gradDensityX, gradDensityY, gradDensityZ, pNLP);
          // printf("%lf, %lf, %lf\n", gradDensityX, gradDensityY, gradDensityZ);
        } else {
          GetPotentialGradFast(pNLP->x, pNLP->z, i, gradDensityX, gradDensityY, gradDensityZ, pNLP);
          // printf("(Fast) %lf, %lf, %lf\n", gradDensityX, gradDensityY, gradDensityZ);
        }
      } else
        GetPotentialGrad(pNLP->x, pNLP->z, i, gradDensityX, gradDensityY, gradDensityZ, pNLP); // bell-shaped potential
      gradDensityX *= pNLP->_cellPotentialNorm[i];
      gradDensityY *= pNLP->_cellPotentialNorm[i];
      gradDensityZ *= pNLP->_cellPotentialNorm[i];
    }

    // TODO bin-based gradient computation

    pNLP->grad_potential[2 * i] += gradDensityX;
    pNLP->grad_potential[2 * i + 1] += gradDensityY;
    if (pNLP->m_bMoveZ)
      pNLP->grad_potentialZ[i] += gradDensityZ;
    // if(param.bShow)
    //   printf( "cell %d  spreading force (%g %g %g)\n", i, gradDensityX, gradDensityY, gradDensityZ);

    // potential feature for gnn partition
    if(param.nGNNFeature > 0
      && !pNLP->m_pDB->m_modules[i].m_isVia && !pNLP->m_pDB->m_modules[i].m_isFiller ){
      pNLP->m_pDB->m_modules[i].m_vFeatures[7] = gradDensityX; // pushForce_x
      pNLP->m_pDB->m_modules[i].m_vFeatures[8] = gradDensityY; // pushForce_y
    }
  } // for each cell
}

// 2007-07-10 (donnie)
void MyNLP::ComputeBinGrad() {
  // int size = m_binGradX.size();
  for (int k = 0; k < m_pDB->m_totalLayer; k++) {
    int size = m_binGradX[k].size();
    for (int i = 0; i < size; i++) {
      for (int j = 0; j < size; j++) {
        double x = m_pDB->m_coreRgn.left + i * m_potentialGridWidth;
        double y = m_pDB->m_coreRgn.bottom + j * m_potentialGridHeight;
        double z = m_pDB->m_back + k * m_potentialGridThickness +
                   0.5 * m_potentialGridThickness;
#if 0
	    assert( i < m_binGradX.size() );
	    assert( i < m_binGradY.size() );
	    assert( j < m_binGradX[i].size() );
	    assert( j < m_binGradY[i].size() );
#endif
        GetPointPotentialGrad(x, y, z, m_binGradX[k][i][j], m_binGradY[k][i][j],
                              m_binGradZ[k][i][j]);
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
void MyNLP::GetPotentialGradFast(const vector<double> &x,
                                 const vector<double> &z, const int &i,
                                 double &gradX, double &gradY, double &gradZ,
                                 MyNLP *pNLP) {

  if (pNLP->m_pDB->m_modules[i].m_isFixed) {
    gradX = 0;
    gradY = 0;
    gradZ = 0;
    return;
  }

  double cellX = x[i * 2];
  double cellY = x[i * 2 + 1];
  double cellZ = z[i];

  // find 4 bins  (gx, gy) - (gx+1, gy+1)
  int gx, gy, gz;
  pNLP->GetClosestGrid(cellX, cellY, cellZ, gx, gy, gz);

#if 0
    assert( gx >= 0 );
    assert( gy >= 0 );
    assert( gx+1 < (int)pNLP->m_binGradX.size() );
    assert( gy+1 < (int)pNLP->m_binGradX.size() );
#endif
  double gxx = pNLP->m_pDB->m_coreRgn.left + gx * pNLP->m_potentialGridWidth;
  double gyy = pNLP->m_pDB->m_coreRgn.bottom + gy * pNLP->m_potentialGridHeight;
  // double gzz = pNLP->m_pDB->m_back + gz * pNLP->m_potentialGridThickness;
  double alpha = (cellX - gxx) / pNLP->m_potentialGridWidth;
  double beta = (cellY - gyy) / pNLP->m_potentialGridHeight;
  // double gamma = (cellZ - gzz) / pNLP->m_potentialGridThickness;
#if 0
    assert( alpha >= 0 );
    assert( alpha <= 1.0 );
    assert( beta >= 0 );
    assert( beta <= 1.0 );
#endif

  // interpolate x force

  // printf("gx = %d, gy = %d, gz = %d\n", gx, gy, gz);
  // printf("%d, %d, %d\n", (int)pNLP->m_binGradX[0].size(),
  // (int)pNLP->m_binGradX[0][0].size(), (int)pNLP->m_binGradX.size());
  // printf("%d, %d, %d\n", (int)pNLP->m_gridPotential[0].size(),
  // (int)pNLP->m_gridPotential[0][0].size(), (int)pNLP->m_gridPotential.size());

  gradX =
      (pNLP->m_binGradX[gz][gx][gy] +
       alpha *
           (pNLP->m_binGradX[gz][gx + 1][gy] - pNLP->m_binGradX[gz][gx][gy]) +
       beta *
           (pNLP->m_binGradX[gz][gx][gy + 1] - pNLP->m_binGradX[gz][gx][gy]) +
       alpha * beta *
           (pNLP->m_binGradX[gz][gx][gy] +
            pNLP->m_binGradX[gz][gx + 1][gy + 1] -
            pNLP->m_binGradX[gz][gx][gy + 1] -
            pNLP->m_binGradX[gz][gx + 1][gy]));

  // interpolate y force
  gradY =
      (pNLP->m_binGradY[gz][gx][gy] +
       alpha *
           (pNLP->m_binGradY[gz][gx + 1][gy] - pNLP->m_binGradY[gz][gx][gy]) +
       beta *
           (pNLP->m_binGradY[gz][gx][gy + 1] - pNLP->m_binGradY[gz][gx][gy]) +
       alpha * beta *
           (pNLP->m_binGradY[gz][gx][gy] +
            pNLP->m_binGradY[gz][gx + 1][gy + 1] -
            pNLP->m_binGradY[gz][gx][gy + 1] -
            pNLP->m_binGradY[gz][gx + 1][gy]));
  /*if(pNLP->m_bMoveZ)
  {
      gradY = gradY * (1.0 - gamma) +
          gamma * (pNLP->m_binGradY[gz+1][gx][gy] +
          alpha * ( pNLP->m_binGradY[gz+1][gx+1][gy] -
  pNLP->m_binGradY[gz+1][gx][gy] ) + beta  * ( pNLP->m_binGradY[gz+1][gx][gy+1]
  - pNLP->m_binGradY[gz+1][gx][gy] ) + alpha * beta * (
  pNLP->m_binGradY[gz+1][gx][gy] + pNLP->m_binGradY[gz+1][gx+1][gy+1] -
              pNLP->m_binGradY[gz+1][gx][gy+1] -
  pNLP->m_binGradY[gz+1][gx+1][gy] ));
  }*/

  // interpolate z force
  if (pNLP->m_bMoveZ) {
    gradZ = 0; /*(1.0-gamma) * (pNLP->m_binGradZ[gz][gx][gy] +
     alpha * ( pNLP->m_binGradZ[gz][gx+1][gy] - pNLP->m_binGradZ[gz][gx][gy] ) +
     beta  * ( pNLP->m_binGradZ[gz][gx][gy+1] - pNLP->m_binGradZ[gz][gx][gy] ) +
     alpha * beta * ( pNLP->m_binGradZ[gz][gx][gy] +
     pNLP->m_binGradZ[gz][gx+1][gy+1] - pNLP->m_binGradZ[gz][gx][gy+1] -
     pNLP->m_binGradZ[gz][gx+1][gy] )) + gamma * (pNLP->m_binGradZ[gz+1][gx][gy]
     + alpha * ( pNLP->m_binGradZ[gz+1][gx+1][gy] -
     pNLP->m_binGradZ[gz+1][gx][gy] ) + beta  * (
     pNLP->m_binGradZ[gz+1][gx][gy+1] - pNLP->m_binGradZ[gz+1][gx][gy] ) + alpha
     * beta * ( pNLP->m_binGradZ[gz+1][gx][gy] +
     pNLP->m_binGradZ[gz+1][gx+1][gy+1] - pNLP->m_binGradZ[gz+1][gx][gy+1] -
     pNLP->m_binGradZ[gz+1][gx+1][gy] ));*/
  }
}

// (donnie) 2007-07-10
void MyNLP::GetPointPotentialGrad(double cellX, double cellY, double cellZ,
                                  double &gradX, double &gradY, double &gradZ) {
  MyNLP *pNLP = this;
  double left = cellX - pNLP->_potentialRX;
  double bottom = cellY - pNLP->_potentialRY;
  double back = cellZ - pNLP->_potentialRZ;
  double right = cellX + (cellX - left);
  double top = cellY + (cellY - bottom);
  double front = cellZ + (cellZ - back);
  if (left < pNLP->m_pDB->m_coreRgn.left -
                 pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth)
    left = pNLP->m_pDB->m_coreRgn.left -
           pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
  if (right > pNLP->m_pDB->m_coreRgn.right +
                  pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth)
    right = pNLP->m_pDB->m_coreRgn.right +
            pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
  if (bottom < pNLP->m_pDB->m_coreRgn.bottom -
                   pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight)
    bottom = pNLP->m_pDB->m_coreRgn.bottom -
             pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
  if (top > pNLP->m_pDB->m_coreRgn.top +
                pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight)
    top = pNLP->m_pDB->m_coreRgn.top +
          pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
  if (back < pNLP->m_pDB->m_back -
                 pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness)
    back = pNLP->m_pDB->m_back -
           pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness;
  if (front > pNLP->m_pDB->m_front +
                  pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness)
    front = pNLP->m_pDB->m_front +
            pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness;

  int gx, gy, gz;
  pNLP->GetClosestGrid(left, bottom, back, gx, gy, gz);

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

  for (gzz = gz, zz = pNLP->GetZGrid(gz);
       zz <= front && gzz < (int)pNLP->m_gridPotential.size();
       gzz++, zz += pNLP->m_potentialGridThickness) {
    for (gxx = gx, xx = pNLP->GetXGrid(gx);
         xx <= right && gxx < (int)pNLP->m_gridPotential[gzz].size();
         gxx++, xx += pNLP->m_potentialGridWidth) {
      for (gyy = gy, yy = pNLP->GetYGrid(gy);
           yy <= top && gyy < (int)pNLP->m_gridPotential[gzz][gxx].size();
           gyy++, yy += pNLP->m_potentialGridHeight) {
        double gX = GetGradPotential(cellX, xx, pNLP->_potentialRX, width) *
                    GetPotential(cellY, yy, pNLP->_potentialRY, height) *
                    GetPotential(cellZ, zz, pNLP->_potentialRZ, thickness) *
                    (pNLP->m_gridPotential[gzz][gxx][gyy] -
                     pNLP->m_expBinPotential[gzz][gxx][gyy]);
        double gY = GetPotential(cellX, xx, pNLP->_potentialRX, width) *
                    GetGradPotential(cellY, yy, pNLP->_potentialRY, height) *
                    GetPotential(cellZ, zz, pNLP->_potentialRZ, thickness) *
                    (pNLP->m_gridPotential[gzz][gxx][gyy] -
                     pNLP->m_expBinPotential[gzz][gxx][gyy]);
        double gZ = 0;
        if (pNLP->m_bMoveZ) {
          gZ = GetPotential(cellX, xx, pNLP->_potentialRX, width) *
               GetPotential(cellY, yy, pNLP->_potentialRY, height) *
               GetGradPotential(cellZ, zz, pNLP->_potentialRZ, thickness) *
               (pNLP->m_gridPotential[gzz][gxx][gyy] -
                pNLP->m_expBinPotential[gzz][gxx][gyy]);
        }

        if (m_skewDensityPenalty1 != 1.0) {
          if (pNLP->m_gridPotential[gzz][gxx][gyy] <
              pNLP->m_expBinPotential[gzz][gxx][gyy])
            gX /= m_skewDensityPenalty2;
          else
            gX *= m_skewDensityPenalty1;
          if (pNLP->m_gridPotential[gzz][gxx][gyy] <
              pNLP->m_expBinPotential[gzz][gxx][gyy])
            gY /= m_skewDensityPenalty2;
          else
            gY *= m_skewDensityPenalty1;
          if (pNLP->m_bMoveZ) {
            if (pNLP->m_gridPotential[gzz][gxx][gyy] <
                pNLP->m_expBinPotential[gzz][gxx][gyy])
              gZ /= m_skewDensityPenalty2;
            else
              gZ *= m_skewDensityPenalty1;
          }
        }

        if (bMulti) {
          gradX += gX * m_weightDensity[gzz][gxx][gyy];
          gradY += gY * m_weightDensity[gzz][gxx][gyy];
          if (pNLP->m_bMoveZ)
            gradZ += gZ * m_weightDensity[gzz][gxx][gyy];
        } else {
          gradX += gX;
          gradY += gY;
          if (pNLP->m_bMoveZ)
            gradZ += gZ;
        }
      }
    }
  } // for each grid
}

// static
void MyNLP::GetPotentialGrad(const vector<double> &x, const vector<double> &z,
                             const int &i, double &gradX, double &gradY,
                             double &gradZ, MyNLP *pNLP) {
  double cellX = x[i * 2];
  double cellY = x[i * 2 + 1];
  double cellZ = z[i];

  double width = pNLP->m_pDB->m_modules[i].GetWidth(z[i] - 0.5);
  double height = pNLP->m_pDB->m_modules[i].GetHeight(z[i] - 0.5);
  double thickness = pNLP->m_pDB->m_modules[i].m_thickness;
  //// use square to model small std-cells
  if (height < pNLP->m_potentialGridHeight &&
      width < pNLP->m_potentialGridWidth &&
      thickness < pNLP->m_potentialGridThickness)
    width = height = thickness = 0;

  double left = cellX - width * 0.5 - pNLP->_potentialRX;
  double bottom = cellY - height * 0.5 - pNLP->_potentialRY;
  double back = cellZ - thickness * 0.5 - pNLP->_potentialRZ;
  double right = cellX + (cellX - left);
  double top = cellY + (cellY - bottom);
  double front = cellZ + (cellZ - back);
  ;

  if (left < pNLP->m_pDB->m_coreRgn.left -
                 pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth)
    left = pNLP->m_pDB->m_coreRgn.left -
           pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
  if (right > pNLP->m_pDB->m_coreRgn.right +
                  pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth)
    right = pNLP->m_pDB->m_coreRgn.right +
            pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
  if (bottom < pNLP->m_pDB->m_coreRgn.bottom -
                   pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight)
    bottom = pNLP->m_pDB->m_coreRgn.bottom -
             pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
  if (top > pNLP->m_pDB->m_coreRgn.top +
                pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight)
    top = pNLP->m_pDB->m_coreRgn.top +
          pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
  if (back < pNLP->m_pDB->m_back -
                 pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness)
    back = pNLP->m_pDB->m_back -
           pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness;
  if (front > pNLP->m_pDB->m_front +
                  pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness)
    front = pNLP->m_pDB->m_front +
            pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness;

  int gx, gy, gz;
  pNLP->GetClosestGrid(left, bottom, back, gx, gy, gz);

  int gxx, gyy, gzz;
  double xx, yy, zz;
  gradX = 0.0;
  gradY = 0.0;
  gradZ = 0.0;
  for (gzz = gz, zz = pNLP->GetZGrid(gz);
       zz <= front && gzz < (int)pNLP->m_gridPotential.size();
       gzz++, zz += pNLP->m_potentialGridThickness) {
    for (gxx = gx, xx = pNLP->GetXGrid(gx);
         xx <= right && gxx < (int)pNLP->m_gridPotential[gzz].size();
         gxx++, xx += pNLP->m_potentialGridWidth) {

      for (gyy = gy, yy = pNLP->GetYGrid(gy);
           yy <= top && gyy < (int)pNLP->m_gridPotential[gzz][gxx].size();
           gyy++, yy += pNLP->m_potentialGridHeight) {

        double gX = GetGradPotential(cellX, xx, pNLP->_potentialRX, width) *
                    GetPotential(cellY, yy, pNLP->_potentialRY, height) *
                    GetPotential(cellZ, zz, pNLP->_potentialRZ, thickness) *
                    (pNLP->m_gridPotential[gzz][gxx][gyy] -
                     pNLP->m_expBinPotential[gzz][gxx][gyy]);
        double gY = GetPotential(cellX, xx, pNLP->_potentialRX, width) *
                    GetGradPotential(cellY, yy, pNLP->_potentialRY, height) *
                    GetPotential(cellZ, zz, pNLP->_potentialRZ, thickness) *
                    (pNLP->m_gridPotential[gzz][gxx][gyy] -
                     pNLP->m_expBinPotential[gzz][gxx][gyy]);
        double gZ = 0;
        if (pNLP->m_bMoveZ) {
          gZ = GetPotential(cellX, xx, pNLP->_potentialRX, width) *
               GetPotential(cellY, yy, pNLP->_potentialRY, height) *
               GetGradPotential(cellZ, zz, pNLP->_potentialRZ, thickness) *
               (pNLP->m_gridPotential[gzz][gxx][gyy] -
                pNLP->m_expBinPotential[gzz][gxx][gyy]);
        }
        /*
        printf( "cell %d (%g %g) bin %d %d (%g %g) p %g exp %g gradient %g
        %g\n", i, cellX, cellY, gxx, gyy, xx, yy,
          pNLP->m_gridPotential[gxx][gyy], pNLP->m_expBinPotential[gxx][gyy],
          gX, gY );
          */

        if (m_skewDensityPenalty1 != 1.0) {
          if (pNLP->m_gridPotential[gzz][gxx][gyy] <
              pNLP->m_expBinPotential[gzz][gxx][gyy])
            gX /= m_skewDensityPenalty2;
          else
            gX *= m_skewDensityPenalty1;
          if (pNLP->m_gridPotential[gzz][gxx][gyy] <
              pNLP->m_expBinPotential[gzz][gxx][gyy])
            gY /= m_skewDensityPenalty2;
          else
            gY *= m_skewDensityPenalty1;
          if (pNLP->m_bMoveZ) {
            if (pNLP->m_gridPotential[gzz][gxx][gyy] <
                pNLP->m_expBinPotential[gzz][gxx][gyy])
              gZ /= m_skewDensityPenalty2;
            else
              gZ *= m_skewDensityPenalty1;
          }
        }

        if (bMulti) {
          gradX += gX * m_weightDensity[gzz][gxx][gyy];
          gradY += gY * m_weightDensity[gzz][gxx][gyy];
          if (pNLP->m_bMoveZ)
            gradZ += gZ * m_weightDensity[gzz][gxx][gyy];
        } else {
          gradX += gX;
          gradY += gY;
          if (pNLP->m_bMoveZ)
            gradZ += gZ;
        }
      }
    }
  } // for each grid
}

// (kaie) 2010-10-18 Weighted-Average-Exponential Wirelength Model
void MyNLP::calc_weighted_sum_exp_using_pin(
    const vector<int>::const_iterator &begin,
    const vector<int>::const_iterator &end, const vector<double> &x,
    const vector<double> &expX, double &weighted_sum_exp_xi_over_alpha,
    double &weighted_sum_exp_inv_xi_over_alpha,
    double &weighted_sum_exp_yi_over_alpha,
    double &weighted_sum_exp_inv_yi_over_alpha, const CPlaceDB *pDB,
    const vector<bool> *pUsePin, const vector<double> &expPins, int id) {
  weighted_sum_exp_xi_over_alpha = 0;
  weighted_sum_exp_inv_xi_over_alpha = 0;
  weighted_sum_exp_yi_over_alpha = 0;
  weighted_sum_exp_inv_yi_over_alpha = 0;

  vector<int>::const_iterator ite;
  int pinId, pinIndex;
  int blockId;
  for (ite = begin, pinIndex = 0; ite != end; ++ite, pinIndex++) {
    // for each pin of the net
    pinId = *ite;
    blockId = pDB->m_pins[pinId].moduleId;

    double xx = x[2 * blockId];
    double yy = x[2 * blockId + 1];
    if ((*pUsePin)[blockId]) {
      xx += pDB->m_pins[pinId].xOff;
      yy += pDB->m_pins[pinId].yOff;
    }

    if ((*pUsePin)[blockId] /*&& blockId != id*/) // macro or self pin
                                                  // if( blockId != id )
    {
      // handle pins
      weighted_sum_exp_xi_over_alpha += xx * expPins[2 * pinId];
      weighted_sum_exp_inv_xi_over_alpha += xx * 1.0 / expPins[2 * pinId];
      weighted_sum_exp_yi_over_alpha += yy * expPins[2 * pinId + 1];
      weighted_sum_exp_inv_yi_over_alpha += yy * 1.0 / expPins[2 * pinId + 1];
    } else {
      // use block center
      // assert( expX[2*blockId] != 0);
      // assert( expX[2*blockId+1] != 0 );
      weighted_sum_exp_xi_over_alpha += xx * expX[2 * blockId];
      weighted_sum_exp_inv_xi_over_alpha += xx * 1.0 / expX[2 * blockId];
      weighted_sum_exp_yi_over_alpha += yy * expX[2 * blockId + 1];
      weighted_sum_exp_inv_yi_over_alpha += yy * 1.0 / expX[2 * blockId + 1];
    }
  }
}

// (frank) 2022-07-22 3D Weighted-Average-Exponential Wirelength Model
void MyNLP::calc_weighted_sum_exp_using_pin_for_layers(
    const vector<int>::const_iterator &begin,
    const vector<int>::const_iterator &end, const vector<double> &x,
    const vector<double> &expX, const vector<double> &z,
    const vector<double> &expZ,
    vector<double> &layer_weighted_sum_exp_xi_over_alpha,
    vector<double> &layer_weighted_sum_exp_inv_xi_over_alpha,
    vector<double> &layer_weighted_sum_exp_yi_over_alpha,
    vector<double> &layer_weighted_sum_exp_inv_yi_over_alpha,
    const CPlaceDB *pDB, const vector<bool> *pUsePin,
    const vector<double> &expPins, int id) {
  layer_weighted_sum_exp_xi_over_alpha.resize(param.nlayer, 0);
  layer_weighted_sum_exp_inv_xi_over_alpha.resize(param.nlayer, 0);
  layer_weighted_sum_exp_yi_over_alpha.resize(param.nlayer, 0);
  layer_weighted_sum_exp_inv_yi_over_alpha.resize(param.nlayer, 0);

  vector<int>::const_iterator ite;
  int pinId, pinIndex;
  int blockId;
  for (ite = begin, pinIndex = 0; ite != end; ++ite, pinIndex++) {
    // for each pin of the net
    pinId = *ite;
    blockId = pDB->m_pins[pinId].moduleId;

    int layer = (z[blockId] < pDB->m_dCutline) ? 0 : 1;

    double xx = x[2 * blockId];
    double yy = x[2 * blockId + 1];
    if ((*pUsePin)[blockId]) {
      xx += pDB->m_pins[pinId].xOff;
      yy += pDB->m_pins[pinId].yOff;
    }

    if ((*pUsePin)[blockId] /*&& blockId != id*/) // macro or self pin
                                                  // if( blockId != id )
    {
      if (pDB->m_modules[blockId].m_isVia) {
        // handle ball
        for (int k = 0; k < param.nlayer; ++k) {
          layer_weighted_sum_exp_xi_over_alpha[k] += xx * expPins[2 * pinId];
          layer_weighted_sum_exp_inv_xi_over_alpha[k] +=
              xx * 1.0 / expPins[2 * pinId];
          layer_weighted_sum_exp_yi_over_alpha[k] +=
              yy * expPins[2 * pinId + 1];
          layer_weighted_sum_exp_inv_yi_over_alpha[k] +=
              yy * 1.0 / expPins[2 * pinId + 1];
        }
      } else {
        // handle pins
        layer_weighted_sum_exp_xi_over_alpha[layer] += xx * expPins[2 * pinId];
        layer_weighted_sum_exp_inv_xi_over_alpha[layer] +=
            xx * 1.0 / expPins[2 * pinId];
        layer_weighted_sum_exp_yi_over_alpha[layer] +=
            yy * expPins[2 * pinId + 1];
        layer_weighted_sum_exp_inv_yi_over_alpha[layer] +=
            yy * 1.0 / expPins[2 * pinId + 1];
      }
    } else {
      // use block center
      // assert( expX[2*blockId] != 0);
      // assert( expX[2*blockId+1] != 0 );
      layer_weighted_sum_exp_xi_over_alpha[layer] += xx * expX[2 * blockId];
      layer_weighted_sum_exp_inv_xi_over_alpha[layer] +=
          xx * 1.0 / expX[2 * blockId];
      layer_weighted_sum_exp_yi_over_alpha[layer] += yy * expX[2 * blockId + 1];
      layer_weighted_sum_exp_inv_yi_over_alpha[layer] +=
          yy * 1.0 / expX[2 * blockId + 1];
    }
  }
}

// static
void MyNLP::calc_sum_exp_using_pin(
    const vector<int>::const_iterator &begin,
    const vector<int>::const_iterator &end, const vector<double> &x,
    const vector<double> &z, const vector<double> &expX,
    const vector<double> &expZ, double &sum_exp_xi_over_alpha,
    double &sum_exp_inv_xi_over_alpha, double &sum_exp_yi_over_alpha,
    double &sum_exp_inv_yi_over_alpha, double &sum_exp_zi_over_alpha,
    double &sum_exp_inv_zi_over_alpha, const CPlaceDB *pDB,
    const vector<bool> *pUsePin, const vector<double> &expPins,
    const vector<double> &expPinsZ, int id) {
  sum_exp_xi_over_alpha = 0;
  sum_exp_inv_xi_over_alpha = 0;
  sum_exp_yi_over_alpha = 0;
  sum_exp_inv_yi_over_alpha = 0;
  sum_exp_zi_over_alpha = 0;
  sum_exp_inv_zi_over_alpha = 0;

  vector<int>::const_iterator ite;
  int pinId;
  int blockId;
  for (ite = begin; ite != end; ++ite) {
    // for each pin of the net
    pinId = *ite;
    blockId = pDB->m_pins[pinId].moduleId;

    if ((*pUsePin)[blockId] /*&& blockId != id*/) // macro or self pin
    // if( blockId != id )
    {
      // handle pins
      sum_exp_xi_over_alpha += expPins[2 * pinId];
      sum_exp_inv_xi_over_alpha += 1.0 / expPins[2 * pinId];
      sum_exp_yi_over_alpha += expPins[2 * pinId + 1];
      sum_exp_inv_yi_over_alpha += 1.0 / expPins[2 * pinId + 1];
      if ((int)expPinsZ.size() != 0) {
        sum_exp_zi_over_alpha += expPinsZ[pinId];
        sum_exp_inv_zi_over_alpha += 1.0 / expPinsZ[pinId];
      }
    } else {
      // use block center
      // assert( expX[2*blockId] != 0);
      // assert( expX[2*blockId+1] != 0 );
      sum_exp_xi_over_alpha += expX[2 * blockId];
      sum_exp_inv_xi_over_alpha += 1.0 / expX[2 * blockId];
      sum_exp_yi_over_alpha += expX[2 * blockId + 1];
      sum_exp_inv_yi_over_alpha += 1.0 / expX[2 * blockId + 1];
      if ((int)expZ.size() != 0) {
        sum_exp_zi_over_alpha += expZ[blockId];
        sum_exp_inv_zi_over_alpha += 1.0 / expZ[blockId];
      }
    }
  }
}
void MyNLP::calc_sum_exp_using_pin_for_layers(
    const vector<int>::const_iterator &begin,
    const vector<int>::const_iterator &end, const vector<double> &x,
    const vector<double> &z, const vector<double> &expX,
    const vector<double> &expZ, vector<double> &layer_sum_exp_xi_over_alpha,
    vector<double> &layer_sum_exp_inv_xi_over_alpha,
    vector<double> &layer_sum_exp_yi_over_alpha,
    vector<double> &layer_sum_exp_inv_yi_over_alpha,
    vector<double> &layer_sum_exp_zi_over_alpha,
    vector<double> &layer_sum_exp_inv_zi_over_alpha, const CPlaceDB *pDB,
    const vector<bool> *pUsePin, const vector<double> &expPins,
    const vector<double> &expPinsZ, int id) {
  layer_sum_exp_xi_over_alpha.resize(param.nlayer, 0);
  layer_sum_exp_inv_xi_over_alpha.resize(param.nlayer, 0);
  layer_sum_exp_yi_over_alpha.resize(param.nlayer, 0);
  layer_sum_exp_inv_yi_over_alpha.resize(param.nlayer, 0);
  layer_sum_exp_zi_over_alpha.resize(param.nlayer, 0);
  layer_sum_exp_inv_zi_over_alpha.resize(param.nlayer, 0);

  vector<int>::const_iterator ite;
  int pinId;
  int blockId;
  for (ite = begin; ite != end; ++ite) {
    // for each pin of the net
    pinId = *ite;
    blockId = pDB->m_pins[pinId].moduleId;

    int layer = (z[blockId] < pDB->m_dCutline) ? 0 : 1;

    if ((*pUsePin)[blockId] /*&& blockId != id*/) // macro or self pin
    // if( blockId != id )
    {
      // handle pins
      if (pDB->m_modules[blockId].m_isVia) {
        for (int k = 0; k > param.nlayer; ++k) {
          layer_sum_exp_xi_over_alpha[k] += expPins[2 * pinId];
          layer_sum_exp_inv_xi_over_alpha[k] += 1.0 / expPins[2 * pinId];
          layer_sum_exp_yi_over_alpha[k] += expPins[2 * pinId + 1];
          layer_sum_exp_inv_yi_over_alpha[k] += 1.0 / expPins[2 * pinId + 1];
          layer_sum_exp_zi_over_alpha[k] += expPinsZ[pinId];
          layer_sum_exp_inv_zi_over_alpha[k] += 1.0 / expPinsZ[pinId];
        }
      } else {
        layer_sum_exp_xi_over_alpha[layer] += expPins[2 * pinId];
        layer_sum_exp_inv_xi_over_alpha[layer] += 1.0 / expPins[2 * pinId];
        layer_sum_exp_yi_over_alpha[layer] += expPins[2 * pinId + 1];
        layer_sum_exp_inv_yi_over_alpha[layer] += 1.0 / expPins[2 * pinId + 1];
        if(!param.noZ){
          layer_sum_exp_zi_over_alpha[layer] += expPinsZ[pinId];
          layer_sum_exp_inv_zi_over_alpha[layer] += 1.0 / expPinsZ[pinId];
        }
      }
    } else {
      // use block center
      layer_sum_exp_xi_over_alpha[layer] += expX[2 * blockId];
      layer_sum_exp_inv_xi_over_alpha[layer] += 1.0 / expX[2 * blockId];
      layer_sum_exp_yi_over_alpha[layer] += expX[2 * blockId + 1];
      layer_sum_exp_inv_yi_over_alpha[layer] += 1.0 / expX[2 * blockId + 1];
      if(!param.noZ){
        layer_sum_exp_zi_over_alpha[layer] += expZ[blockId];
        layer_sum_exp_inv_zi_over_alpha[layer] += 1.0 / expZ[blockId];
      }
    }
  }
}

// static 2006-09-12 (donnie)
void MyNLP::calc_sum_exp_using_pin_XHPWL(
    const vector<int>::const_iterator &begin,
    const vector<int>::const_iterator &end, const MyNLP *pNLP,
    double &sum_exp_x_plus_y_over_alpha, double &sum_exp_x_minus_y_over_alpha,
    double &sum_exp_inv_x_plus_y_over_alpha,
    double &sum_exp_inv_x_minus_y_over_alpha) {
  sum_exp_x_plus_y_over_alpha = 0;
  sum_exp_x_minus_y_over_alpha = 0;
  sum_exp_inv_x_plus_y_over_alpha = 0;
  sum_exp_inv_x_minus_y_over_alpha = 0;

  vector<int>::const_iterator ite;
  int pinId;
  int blockId;
  for (ite = begin; ite != end; ++ite) {
    // for each pin of the net
    pinId = *ite;
    blockId = pNLP->m_pDB->m_pins[pinId].moduleId;

    if (pNLP->m_usePin[blockId]) // macro or self pin
    {
      // handle pins
      sum_exp_x_plus_y_over_alpha += pNLP->_expPinXplusY[pinId];
      sum_exp_x_minus_y_over_alpha += pNLP->_expPinXminusY[pinId];
      sum_exp_inv_x_plus_y_over_alpha += 1.0 / pNLP->_expPinXplusY[pinId];
      sum_exp_inv_x_minus_y_over_alpha += 1.0 / pNLP->_expPinXminusY[pinId];
    } else {
      // use block center
      sum_exp_x_plus_y_over_alpha += pNLP->_expXplusY[blockId];
      sum_exp_x_minus_y_over_alpha += pNLP->_expXminusY[blockId];
      sum_exp_inv_x_plus_y_over_alpha += 1.0 / pNLP->_expXplusY[blockId];
      sum_exp_inv_x_minus_y_over_alpha += 1.0 / pNLP->_expXminusY[blockId];
    }
  }
}

void *MyNLP::UpdateBlockPositionThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  UpdateBlockPosition(*pMsg->pX, *pMsg->pZ, pMsg->pNLP, pMsg->index1,
                      pMsg->index2);
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

// static
void MyNLP::UpdateBlockPosition(const vector<double> &x,
                                const vector<double> &z, MyNLP *pNLP,
                                int index1, int index2) {
  if (index2 > (int)pNLP->m_pDB->m_modules.size())
    index2 = (int)pNLP->m_pDB->m_modules.size();

  /*double core_left = pNLP->m_pDB->m_coreRgn.left;
  double core_right = pNLP->m_pDB->m_coreRgn.right;
  double core_top = pNLP->m_pDB->m_coreRgn.top;
  double core_bottom = pNLP->m_pDB->m_coreRgn.bottom;
  double core_width = core_right - core_left;
  double core_height = core_top - core_bottom;

  double h_center = core_left + core_width * 0.5;
  double v_center = core_bottom + core_height * 0.5;*/

  for (int i = index1; i < index2; i++) {
    if (pNLP->m_pDB->m_modules[i].m_isFixed == false) {
      // printf("%d, %.2f-%.2f-%.2f\n", i, x[i*2], x[i*2+1], z[i]);
      pNLP->m_pDB->MoveModuleCenter(i, x[i * 2], x[i * 2 + 1], z[i]);
      // pNLP->m_pDB->MoveModuleCenter( i, x[i*2], x[i*2+1] );
      // pNLP->m_pDB->m_modules[i].m_cz = z[i];*/
      /*if(pNLP->m_pDB->m_modules[i].m_cx < h_center &&
      pNLP->m_pDB->m_modules[i].m_cy < v_center) // 0
          pNLP->m_pDB->m_modules[i].m_z = 0;
      else if(pNLP->m_pDB->m_modules[i].m_cx < h_center)    // 1
          pNLP->m_pDB->m_modules[i].m_z = 1;
      else if(pNLP->m_pDB->m_modules[i].m_cy < v_center)    // 3
          pNLP->m_pDB->m_modules[i].m_z = 3;
      else
          pNLP->m_pDB->m_modules[i].m_z = 2;*/
      // potential feature for gnn partition
      if(param.nGNNFeature > 0
        && !pNLP->m_pDB->m_modules[i].m_isVia && !pNLP->m_pDB->m_modules[i].m_isFiller ){
        pNLP->m_pDB->m_modules[i].m_vFeatures[0] = z[i]-0.5; // z (layer)
        pNLP->m_pDB->m_modules[i].m_vFeatures[1] = x[i * 2]; // cx
        pNLP->m_pDB->m_modules[i].m_vFeatures[2] = x[i * 2 + 1]; // cy
      }
    }
  }
}

void MyNLP::CreatePotentialGrid() {
  // printf( "Create Potential Grid\n" );
  m_gridPotential.clear(); // remove old values

  int realGridSize = m_potentialGridSize + m_potentialGridPadding; // padding

  // (donnie) 2007-07-10  add m_binGradX & m_binGradY
  // printf("realGridSize = %d\n", realGridSize);

  m_newPotential.resize(m_pDB->m_modules.size());
  // m_gridPotential.resize( realGridSize );
  // m_basePotential.resize( realGridSize );
  // m_binGradX.resize( realGridSize + 1 );
  // m_binGradY.resize( realGridSize + 1 );
  // m_binGradZ.resize( realGridSize + 1 );
  // m_weightDensity.resize( realGridSize );

  m_gridPotential.resize(m_pDB->m_totalLayer);
  m_basePotential.resize(m_pDB->m_totalLayer);
  m_binGradX.resize(m_pDB->m_totalLayer + 1);
  m_binGradY.resize(m_pDB->m_totalLayer + 1);
  m_binGradZ.resize(m_pDB->m_totalLayer + 1);
  m_weightDensity.resize(m_pDB->m_totalLayer);

  // for( int layer = 0; layer < realGridSize; layer++ )
  for (int layer = 0; layer < m_pDB->m_totalLayer; layer++) {
    m_gridPotential[layer].resize(realGridSize);
    m_basePotential[layer].resize(realGridSize);
    m_binGradX[layer].resize(realGridSize + 1);
    m_binGradY[layer].resize(realGridSize + 1);
    m_binGradZ[layer].resize(realGridSize + 1);
    m_weightDensity[layer].resize(realGridSize);
    for (int i = 0; i < realGridSize; i++) {
      m_basePotential[layer][i].resize(realGridSize, 0);
      m_gridPotential[layer][i].resize(realGridSize, 0);
      m_binGradX[layer][i].resize(realGridSize + 1, 0);
      m_binGradY[layer][i].resize(realGridSize + 1, 0);
      m_binGradZ[layer][i].resize(realGridSize + 1, 0);
      m_weightDensity[layer][i].resize(realGridSize, 1);
    }
    m_binGradX[layer][realGridSize].resize(realGridSize + 1, 0);
    m_binGradY[layer][realGridSize].resize(realGridSize + 1, 0);
    m_binGradZ[layer][realGridSize].resize(realGridSize + 1, 0);
  }
  m_binGradX[m_pDB->m_totalLayer].resize(realGridSize + 1);
  m_binGradY[m_pDB->m_totalLayer].resize(realGridSize + 1);
  m_binGradZ[m_pDB->m_totalLayer].resize(realGridSize + 1);
  for (int i = 0; i <= realGridSize; i++) {
    m_binGradX[m_pDB->m_totalLayer][i].resize(realGridSize + 1, 0);
    m_binGradY[m_pDB->m_totalLayer][i].resize(realGridSize + 1, 0);
    m_binGradZ[m_pDB->m_totalLayer][i].resize(realGridSize + 1, 0);
  }

  m_potentialGridWidth =
      (m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left) / m_potentialGridSize;
  m_potentialGridHeight =
      (m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom) / m_potentialGridSize;
  // m_potentialGridThickness = (m_pDB->m_front - m_pDB->m_back ) /
  // (m_potentialGridSize);
  m_potentialGridThickness =
      (m_pDB->m_front - m_pDB->m_back) / (m_pDB->m_totalLayer);

  _potentialRX = m_potentialGridWidth * _potentialGridR;
  _potentialRY = m_potentialGridHeight * _potentialGridR;
  //_potentialRZ = 0.0;
  _potentialRZ = 0.5 * m_potentialGridThickness;
  //_potentialRZ = m_potentialGridThickness * _potentialGridR;
}

void MyNLP::ClearPotentialGrid() {
  for (int gz = 0; gz < (int)m_gridPotential.size(); gz++) {
    for (int gx = 0; gx < (int)m_gridPotential[gz].size(); gx++)
      fill(m_gridPotential[gz][gx].begin(), m_gridPotential[gz][gx].end(), 0.0);
  }
}

void MyNLP::UpdateExpBinPotentialTSV(bool showMsg) {
  /*double totalCellArea = 0;
  for(int i = 0; i < (int)m_pDB->m_modules.size(); i++)
  {
      totalCellArea += m_pDB->m_modules[i].m_area;
  }
  double avgCellArea = totalCellArea / (double)m_pDB->m_modules.size();*/
  double TSVarea = m_pDB->TSVarea;
  // printf("%.0f\n", TSVarea);
  // double TSVsize = 4;
  // gArg.GetDouble("TSVsize", &TSVsize);
  // TSVarea = TSVsize * avgCellArea;
  // TSVarea = 9 * m_pDB->m_rowHeight * m_pDB->m_rowHeight;

  for (int i = 0; i < (int)m_pDB->m_nets.size(); i++) // for each net
  {
    double max_x = m_pDB->m_coreRgn.left, max_y = m_pDB->m_coreRgn.bottom,
           max_z = 0;
    double min_x = m_pDB->m_coreRgn.right, min_y = m_pDB->m_coreRgn.top,
           min_z = m_pDB->m_totalLayer - 1;
    for (int j = 0; j < (int)m_pDB->m_nets[i].size(); j++) // for each pin
    {
      int pinId = m_pDB->m_nets[i][j];
      double pin_x, pin_y, pin_z;
      m_pDB->GetPinLocation(pinId, pin_x, pin_y, pin_z);

      if (pin_x < min_x)
        min_x = pin_x;
      else if (pin_x > max_x)
        max_x = pin_x;

      if (pin_y < min_y)
        min_y = pin_y;
      else if (pin_y > max_y)
        max_y = pin_y;

      if (pin_z < min_z)
        min_z = pin_z;
      else if (pin_z > max_z)
        max_z = pin_z;
    }

    // double numberTSV = max_z - min_z;
    // double TSVarea = 4 * m_pDB->m_rowHeight * m_pDB->m_rowHeight;
    int gx_min, gy_min, gz_min;
    int gx_max, gy_max, gz_max;
    GetClosestGrid(min_x, min_y, min_z, gx_min, gy_min, gz_min);
    GetClosestGrid(max_x, max_y, max_z, gx_max, gy_max, gz_max);
    /*gx_max = min(gx_max, (int)m_basePotential[0].size()-1);
    gy_max = min(gy_max, (int)m_basePotential[0][0].size()-1);
    gz_max = min(gz_max, (int)m_basePotential.size()-1);*/
    double avgTSVarea =
        TSVarea / ((gx_max - gx_min + 1) * (gy_max - gy_min + 1));
    // printf("TSV are:%.2f, Average TSV area:%.2f\n", TSVarea, avgTSVarea);
    for (int gzz = gz_min; gzz <= gz_max; gzz++) {
      for (int gxx = gx_min; gxx <= gx_max; gxx++) {
        for (int gyy = gy_min; gyy <= gy_max; gyy++) {
          if (m_expBinPotential[gzz][gxx][gyy] < 1e-10)
            continue;
          if (m_expBinPotential[gzz][gxx][gyy] < avgTSVarea) {
            // totalFree -= m_expBinPotential[gzz][gxx][gyy];
            m_expBinPotential[gzz][gxx][gyy] = 0;
            // zeroSpaceBin++;
          } else {
            m_expBinPotential[gzz][gxx][gyy] -= avgTSVarea;
            // totalFree -= avgTSVarea;
          }
        }
      }
    }
  }
}

double MyNLP::UpdateExpBinPotential(double util, bool showMsg) {
  // Default is -1. Spread to the whole chip
  if (util < 0)
    util = 1.0; // use all space

  m_expBinPotential.resize(m_basePotential.size());
  double totalFree = 0;
  int zeroSpaceBin = 0;
  for (int k = 0; k < (int)m_expBinPotential.size(); k++) {
    m_expBinPotential[k].resize(m_basePotential[k].size());
    for (unsigned int i = 0; i < m_basePotential[k].size(); i++) {
      m_expBinPotential[k][i].resize(m_basePotential[k][i].size());
      for (unsigned int j = 0; j < m_basePotential[k][i].size(); j++) {
        double base = m_basePotential[k][i][j];
        double overlapX =
            getOverlap(GetXGrid(i) - m_potentialGridWidth * 0.5,
                       GetXGrid(i) + m_potentialGridWidth * 0.5,
                       m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.right);
        double overlapY =
            getOverlap(GetYGrid(j) - m_potentialGridHeight * 0.5,
                       GetYGrid(j) + m_potentialGridHeight * 0.5,
                       m_pDB->m_coreRgn.bottom, m_pDB->m_coreRgn.top);
        double overlapZ =
            getOverlap(GetZGrid(k) - m_potentialGridThickness * 0.5,
                       GetZGrid(k) + m_potentialGridThickness * 0.5,
                       m_pDB->m_back, m_pDB->m_front);
        double realBinVolumn = overlapX * overlapY * overlapZ;
        double free = realBinVolumn - base;
        if (free > 1e-4) {
          m_expBinPotential[k][i][j] = free * util;
          totalFree += m_expBinPotential[k][i][j];
        } else {
          m_expBinPotential[k][i][j] = 0.0;
          zeroSpaceBin++;
        }
      }
    }
  }

  // printf("%.2f / %.2f\n", m_pDB->m_totalMovableModuleVolumn , totalFree);
  double algUtil = m_pDB->m_totalMovableModuleVolumn / totalFree;
  if (param.bShow && showMsg) {
    printf("PBIN: Zero space bin #= %d (%d%%).  Algorithm utilization= %.4f\n",
           zeroSpaceBin,
           100 * zeroSpaceBin / m_potentialGridSize / m_potentialGridSize /
               m_pDB->m_totalLayer,
           algUtil);
  }

  double alwaysOver = 0.0;
  if (m_targetUtil > 0.0 && m_targetUtil < 1.0) {
    for (unsigned int i = 0; i < m_pDB->m_modules.size(); i++) {
      if (m_pDB->m_modules[i].m_isFixed)
        continue;
      if (m_pDB->m_modules[i].GetWidth(z[i] - 0.5) >=
              2 * m_potentialGridWidth &&
          m_pDB->m_modules[i].GetHeight(z[i] - 0.5) >=
              2 * m_potentialGridHeight &&
          m_pDB->m_modules[i].m_thickness >= 2 * m_potentialGridThickness) {
        alwaysOver +=
            (m_pDB->m_modules[i].GetWidth(z[i] - 0.5) - m_potentialGridWidth) *
            (m_pDB->m_modules[i].GetHeight(z[i] - 0.5) -
             m_potentialGridHeight) *
            (m_pDB->m_modules[i].m_thickness - m_potentialGridThickness) *
            (1.0 - m_targetUtil);
      }
    }
    if (param.bShow)
      printf("PBIN: Always over: %.0f (%.1f%%)\n", alwaysOver,
             alwaysOver / m_pDB->m_totalMovableModuleVolumn * 100.0);
  }
  m_alwaysOverPotential = alwaysOver;

  return algUtil;
}

// Level smoothing
void MyNLP::LevelSmoothBasePotential(const double &delta, const int &layer) {
  if (delta <= 1.0)
    return;

  vector<vector<double>> oldPotential = m_basePotential[layer];

  double maxPotential = 0;
  double avgPotential = 0;
  double totalPotential = 0;
  for (unsigned int i = 0; i < oldPotential.size(); i++)
    for (unsigned int j = 0; j < oldPotential[i].size(); j++) {
      totalPotential += oldPotential[i][j];
      maxPotential = max(maxPotential, oldPotential[i][j]);
    }
  avgPotential = totalPotential / (oldPotential.size() * oldPotential.size());

  if (totalPotential == 0)
    return; // no preplaced

  // apply TSP-style smoothing
  double newTotalPotential = 0;
  for (unsigned int i = 0; i < m_basePotential.size(); i++)
    for (unsigned int j = 0; j < m_basePotential[i].size(); j++) {
      if (oldPotential[i][j] >= avgPotential) {
        m_basePotential[layer][i][j] =
            avgPotential +
            pow((oldPotential[i][j] - avgPotential) / maxPotential, delta) *
                maxPotential;
      } else {
        m_basePotential[layer][i][j] =
            avgPotential -
            pow((avgPotential - oldPotential[i][j]) / maxPotential, delta) *
                maxPotential;
      }
      newTotalPotential += m_basePotential[layer][i][j];
    }

  // normalization
  double ratio = totalPotential / newTotalPotential;
  for (unsigned int i = 0; i < m_basePotential[layer].size(); i++)
    for (unsigned int j = 0; j < m_basePotential[layer][i].size(); j++)
      m_basePotential[layer][i][j] = m_basePotential[layer][i][j] * ratio;
}

void MyNLP::LevelSmoothBasePotential3D(const double &delta) {
  if (delta <= 1.0)
    return;

  vector<vector<vector<double>>> oldPotential = m_basePotential;

  double maxPotential = 0;
  double avgPotential = 0;
  double totalPotential = 0;
  for (unsigned int k = 0; k < oldPotential.size(); k++)
    for (unsigned int i = 0; i < oldPotential[k].size(); i++)
      for (unsigned int j = 0; j < oldPotential[k][i].size(); j++) {
        totalPotential += oldPotential[k][i][j];
        maxPotential = max(maxPotential, oldPotential[k][i][j]);
      }
  avgPotential = totalPotential / (oldPotential.size() * oldPotential.size() *
                                   oldPotential.size());

  if (totalPotential == 0)
    return; // no preplaced

  // apply TSP-style smoothing
  double newTotalPotential = 0;
  for (unsigned int k = 0; k < m_basePotential.size(); k++)
    for (unsigned int i = 0; i < m_basePotential.size(); i++)
      for (unsigned int j = 0; j < m_basePotential[i].size(); j++) {
        if (oldPotential[k][i][j] >= avgPotential) {
          m_basePotential[k][i][j] =
              avgPotential +
              pow((oldPotential[k][i][j] - avgPotential) / maxPotential,
                  delta) *
                  maxPotential;
        } else {
          m_basePotential[k][i][j] =
              avgPotential -
              pow((avgPotential - oldPotential[k][i][j]) / maxPotential,
                  delta) *
                  maxPotential;
        }
        newTotalPotential += m_basePotential[k][i][j];
      }

  // normalization
  double ratio = totalPotential / newTotalPotential;
  for (unsigned int k = 0; k < m_basePotential.size(); k++)
    for (unsigned int i = 0; i < m_basePotential[k].size(); i++)
      for (unsigned int j = 0; j < m_basePotential[k][i].size(); j++)
        m_basePotential[k][i][j] = m_basePotential[k][i][j] * ratio;
}

void MyNLP::UpdatePotentialGridBase(const vector<double> &x,
                                    const vector<double> &z) {
  double binVolumn =
      m_potentialGridWidth * m_potentialGridHeight * m_potentialGridThickness;

  m_binFreeSpace.resize(m_basePotential.size());
  for (int k = 0; k < (int)m_binFreeSpace.size(); k++) {
    m_binFreeSpace[k].resize(m_basePotential[k].size());
    for (unsigned int i = 0; i < m_basePotential[k].size(); i++) {
      fill(m_basePotential[k][i].begin(), m_basePotential[k][i].end(), 0.0);
      m_binFreeSpace[k][i].resize(m_basePotential[k][i].size());
      fill(m_binFreeSpace[k][i].begin(), m_binFreeSpace[k][i].end(), binVolumn);
    }
  }

  for (int i = 0; i < (int)m_pDB->m_modules.size(); i++) {
    // for each cell. cell ci coordinate is ( x[i*2], x[i*2+1] )

    if (m_pDB->m_modules[i].m_isFixed == false)
      continue;

    // TODO: BUG when shrinking core?
    // if( m_pDB->m_modules[i].m_isOutCore )
    if (m_pDB->BlockOutCore(i))
      continue; // pads?

    double cellX = x[i * 2];
    double cellY = x[i * 2 + 1];
    double cellZ = z[i];
    double width = m_pDB->m_modules[i].GetWidth(z[i] - 0.5);
    double height = m_pDB->m_modules[i].GetHeight(z[i] - 0.5);
    double thickness = m_pDB->m_modules[i].m_thickness;

    // exact block range
    double left = cellX - width * 0.5;
    double bottom = cellY - height * 0.5;
    double back = cellZ - thickness * 0.5;
    double right = cellX + (cellX - left);
    double top = cellY + (cellY - bottom);
    double front = cellZ + (cellZ - back);
    ;

    if (left < m_pDB->m_coreRgn.left)
      left = m_pDB->m_coreRgn.left;
    if (right > m_pDB->m_coreRgn.right)
      right = m_pDB->m_coreRgn.right;
    if (bottom < m_pDB->m_coreRgn.bottom)
      bottom = m_pDB->m_coreRgn.bottom;
    if (top > m_pDB->m_coreRgn.top)
      top = m_pDB->m_coreRgn.top;
    if (back < m_pDB->m_back)
      back = m_pDB->m_back;
    if (front > m_pDB->m_front)
      front = m_pDB->m_front;

    int gx, gy, gz;
    GetClosestGrid(left, bottom, back, gx, gy, gz);
    int gxx, gyy, gzz;
    double xx, yy, zz;

    // Exact density for the base potential"
    for (gzz = gz, zz = GetZGrid(gz);
         zz <= front && gzz < (int)m_basePotential.size();
         gzz++, zz += m_potentialGridThickness) {
      for (gxx = gx, xx = GetXGrid(gx);
           xx <= right && gxx < (int)m_basePotential[gzz].size();
           gxx++, xx += m_potentialGridWidth) {
        for (gyy = gy, yy = GetYGrid(gy);
             yy <= top && gyy < (int)m_basePotential[gzz][gxx].size();
             gyy++, yy += m_potentialGridHeight) {
          m_basePotential[gzz][gxx][gyy] +=
              getOverlap(left, right, xx, xx + m_potentialGridWidth) *
              getOverlap(bottom, top, yy, yy + m_potentialGridHeight) *
              getOverlap(back, front, zz, zz + m_potentialGridThickness);

          m_binFreeSpace[gzz][gxx][gyy] -=
              getOverlap(left, right, xx, xx + m_potentialGridWidth) *
              getOverlap(bottom, top, yy, yy + m_potentialGridHeight) *
              getOverlap(back, front, zz, zz + m_potentialGridThickness);
          // printf("(%d, %d, %d) %lf, %lf\n", gx, gx, gy,
          // m_basePotential[gzz][gxx][gyy], m_binFreeSpace[gzz][gxx][gyy]);
        }
      }
    }

  } // for each cell
}

// static
void *MyNLP::ComputeNewPotentialGridThread(void *arg) {
  ThreadInfo *pMsg = reinterpret_cast<MyNLP::ThreadInfo *>(arg);
  ComputeNewPotentialGrid(*pMsg->pX, *pMsg->pZ, pMsg->pNLP, pMsg->index1,
                          pMsg->index2);
  if (param.nThread > 1)
    pthread_exit(NULL);
  return NULL;
}

// static
void MyNLP::ComputeNewPotentialGrid(const vector<double> &x,
                                    const vector<double> &z, MyNLP *pNLP,
                                    int index1, int index2) {
  if (index2 > (int)pNLP->m_pDB->m_modules.size())
    index2 = (int)pNLP->m_pDB->m_modules.size();

  for (int i = index1; i < index2; i++) {
    // if( pNLP->m_pDB->m_modules[i].m_isOutCore )
    if (pNLP->m_pDB->BlockOutCore(i))
      continue;

    // preplaced blocks are stored in m_basePotential
    if (pNLP->m_pDB->m_modules[i].m_isFixed)
      continue;

    double cellX = x[i * 2];
    double cellY = x[i * 2 + 1];
    double cellZ = z[i];

    double potentialRX = pNLP->_potentialRX;
    double potentialRY = pNLP->_potentialRY;
    double potentialRZ = pNLP->_potentialRZ;

    double width = pNLP->m_pDB->m_modules[i].GetWidth(z[i] - 0.5);
    double height = pNLP->m_pDB->m_modules[i].GetHeight(z[i] - 0.5);
    double thickness = pNLP->m_pDB->m_modules[i].m_thickness;
    //// (convert to std-cell)
    if (height < pNLP->m_potentialGridHeight &&
        width < pNLP->m_potentialGridWidth) // && thickness <
                                            // pNLP->m_potentialGridThickness )
      width = height = 0;                   // thickness = 0;

    double left = cellX - width * 0.5 - potentialRX;
    double bottom = cellY - height * 0.5 - potentialRY;
    double back = cellZ - thickness * 0.5 - potentialRZ;
    double right = cellX + (cellX - left);
    double top = cellY + (cellY - bottom);
    double front = cellZ + (cellZ - back);

    // printf("cellZ: %.0f, thickness: %.0f\n", cellZ, thickness);
    // printf("front: %.0f, back: %.0f\n", front, back);

    if (left < pNLP->m_pDB->m_coreRgn.left -
                   pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth)
      left = pNLP->m_pDB->m_coreRgn.left -
             pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
    if (right > pNLP->m_pDB->m_coreRgn.right +
                    pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth)
      right = pNLP->m_pDB->m_coreRgn.right +
              pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
    if (bottom < pNLP->m_pDB->m_coreRgn.bottom -
                     pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight)
      bottom = pNLP->m_pDB->m_coreRgn.bottom -
               pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
    if (top > pNLP->m_pDB->m_coreRgn.top +
                  pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight)
      top = pNLP->m_pDB->m_coreRgn.top +
            pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
    if (back < pNLP->m_pDB->m_back - pNLP->m_potentialGridPadding *
                                         pNLP->m_potentialGridThickness)
      back = pNLP->m_pDB->m_back -
             pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness;
    if (front > pNLP->m_pDB->m_front + pNLP->m_potentialGridPadding *
                                           pNLP->m_potentialGridThickness)
      front = pNLP->m_pDB->m_front +
              pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness;

    int gx, gy, gz;
    pNLP->GetClosestGrid(left, bottom, back, gx, gy, gz);
    int gxx, gyy, gzz;
    double xx, yy, zz;

    double totalPotential = 0;
    vector<potentialStruct> &potentialList = pNLP->m_newPotential[i];
    potentialList.clear();

    for (gzz = gz, zz = pNLP->GetZGrid(gz);
         zz <= front && gzz < (int)pNLP->m_gridPotential.size();
         gzz++, zz += pNLP->m_potentialGridThickness) {
      for (gxx = gx, xx = pNLP->GetXGrid(gx);
           xx <= right && gxx < (int)pNLP->m_gridPotential[gzz].size();
           gxx++, xx += pNLP->m_potentialGridWidth) {
        for (gyy = gy, yy = pNLP->GetYGrid(gy);
             yy <= top && gyy < (int)pNLP->m_gridPotential[gzz][gxx].size();
             gyy++, yy += pNLP->m_potentialGridHeight) {
          double potential = GetPotential(cellX, xx, potentialRX, width) *
                             GetPotential(cellY, yy, potentialRY, height) *
                             GetPotential(cellZ, zz, potentialRZ, thickness);
          if (potential > 0) {
            totalPotential += potential;
            potentialList.push_back(potentialStruct(gxx, gyy, gzz, potential));
          }
        }
      }
    }

    // normalize the potential so that total potential equals the cell area
    double scale = pNLP->m_pDB->m_modules[i].m_area * // TODO: change to width*height
                   pNLP->m_pDB->m_modules[i].m_thickness /
                   totalPotential; // kaie 3d
    // printf( "totalPotential = %f\n", totalPotential );

    pNLP->_cellPotentialNorm[i] = scale; // normalization factor for the cell i
  }                                      // for each cell
}

double MyNLP::GetDensityProjection(const double &z, const double &k) {
  double d = fabs(z - k);
  if (d <= 0.5)
    return 1 - 2 * d * d;
  else if (d > 0.5 && d <= 1)
    return 2 * (d - 1) * (d - 1);
  else
    return 0.0;
}

double MyNLP::GetGradDensityProjection(const double &z, const double &k) {
  if (z >= k) {
    if ((z - k) <= 0.5)
      return (-4 * z + 4 * k);
    else if ((z - k) > 0.5 && (z - k) <= 1)
      return (4 * z - 4 * k - 4);
    else
      return 0.0;
  } else {
    if ((k - z) <= 0.5)
      return (-4 * z + 4 * k);
    else if ((k - z) > 0.5 && (k - z) <= 1)
      return (-4 * k + 4 * z + 4);
    else
      return 0.0;
  }
}

void MyNLP::UpdatePotentialGrid(const vector<double> &z) {
  ClearPotentialGrid();
  for (unsigned int i = 0; i < m_newPotential.size(); i++) // for each cell
  {
    for (unsigned int j = 0; j < m_newPotential[i].size(); j++) // for each bin
    {
      int gx = m_newPotential[i][j].gx;
      int gy = m_newPotential[i][j].gy;
      int gz = m_newPotential[i][j].gz;

      m_gridPotential[gz][gx][gy] +=
          m_newPotential[i][j].potential * _cellPotentialNorm[i];
    }
  }
}

/*double MyNLP::GetPotential( const double& x1, const double& x2, const double&
r )
{
    double d = fabs( x1 - x2 );

    if( d <= r * 0.5 )
        return 1.0 - 2 * d * d / ( r * r );
    else if( d <= r )
        return 2 * ( d - r ) * ( d - r ) / ( r * r );
    else
        return 0;
}*/

double MyNLP::GetPotential(const double &x1, const double &x2, const double &r,
                           const double &w) {
  double d = fabs(x1 - x2);
  double a = 4.0 / (w + r) / (w + 2 * r);
  double b = 4.0 / r / (w + 2.0 * r);

  if (d <= w * 0.5 + r * 0.5)
    return 1.0 - a * d * d;
  else if (d <= w * 0.5 + r)
    return b * (d - r - w * 0.5) * (d - r - w * 0.5);
  else
    return 0.0;
}
/*
double MyNLP::GetGradPotential( const double& x1, const double& x2, const
double& r )
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

double MyNLP::GetGradPotential(const double &x1, const double &x2,
                               const double &r, const double &w) {
  double d;
  double a = 4.0 / (w + r) / (w + 2.0 * r);
  double b = 4.0 / r / (w + 2.0 * r);

  if (x1 >= x2) // right half
  {
    d = x1 - x2; // d >= 0
    if (d <= w * 0.5 + r * 0.5)
      return -2.0 * a * d;
    else if (d <= w * 0.5 + r)
      return +2.0 * b * (d - r - w * 0.5);
    else
      return 0;
  } else // left half
  {
    d = x2 - x1; // d >= 0
    if (d <= w * 0.5 + r * 0.5)
      return +2.0 * a * d;
    else if (d <= w * 0.5 + r)
      return -2.0 * b * (d - r - w * 0.5);
    else
      return 0;
  }
}

/*double MyNLP::GetGradGradPotential( const double& x1, const double& x2, const
double& r )
{
    double d = fabs( x1 - x2 );

    if( d <= r * 0.5 )
        return -4.0 / ( r * r );
    else if( d <= r )
        return +4.0 / ( r * r );
    else
        return 0;
}*/

/*void   MyNLP::GetGridCenter( const int& gx, const int& gy, double& x1, double&
y1 )
{
    assert( gx <= m_potentialGridSize );
    assert( gy <= m_potentialGridSize );
    assert( gx >= 0 );
    assert( gy >= 0 );

    x1 = m_pDB->m_coreRgn.left   + gx * m_potentialGridWidth  + 0.5 *
m_potentialGridWidth; y1 = m_pDB->m_coreRgn.bottom + gy * m_potentialGridHeight
+ 0.5 * m_potentialGridHeight;
}*/

double MyNLP::GetXGrid(const int &gx) {
  return m_pDB->m_coreRgn.left +
         (gx - m_potentialGridPadding + 0.5) * m_potentialGridWidth;
}

double MyNLP::GetYGrid(const int &gy) {
  return m_pDB->m_coreRgn.bottom +
         (gy - m_potentialGridPadding + 0.5) * m_potentialGridHeight;
}

double MyNLP::GetZGrid(const int &gz) {
  return m_pDB->m_back +
         (gz - m_potentialGridPadding + 0.5) * m_potentialGridThickness;
}

void MyNLP::GetClosestGrid(const double &x1, const double &y1, const double &z1,
                           int &gx, int &gy, int &gz) {
  double left =
      m_pDB->m_coreRgn.left - m_potentialGridWidth * m_potentialGridPadding;
  double bottom =
      m_pDB->m_coreRgn.bottom - m_potentialGridHeight * m_potentialGridPadding;
  double back =
      m_pDB
          ->m_back; // 	    - m_potentialGridThickness * m_potentialGridPadding;
  gx = static_cast<int>(floor((x1 - left) / m_potentialGridWidth));
  gy = static_cast<int>(floor((y1 - bottom) / m_potentialGridHeight));
  gz = static_cast<int>(floor((z1 - back) / m_potentialGridThickness));

  if (gx < 0)
    gx = 0;
  if (gy < 0)
    gy = 0;
  if (gz < 0)
    gz = 0;

  if (gx > (int)m_gridPotential[0].size() - 1)
    gx = (int)m_gridPotential[0].size() - 1;
  if (gy > (int)m_gridPotential[0][0].size() - 1)
    gy = (int)m_gridPotential[0][0].size() - 1;
  if (gz > (int)m_gridPotential.size() - 1)
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

void MyNLP::GetClosestGrid(const double &x1, const double &y1, int &gx,
                           int &gy) {
  double left =
      m_pDB->m_coreRgn.left - m_potentialGridWidth * m_potentialGridPadding;
  double bottom =
      m_pDB->m_coreRgn.bottom - m_potentialGridHeight * m_potentialGridPadding;
  gx = static_cast<int>(floor((x1 - left) / m_potentialGridWidth));
  gy = static_cast<int>(floor((y1 - bottom) / m_potentialGridHeight));

  if (gx < 0)
    gx = 0;
  if (gy < 0)
    gy = 0;

  if (gx > (int)m_gridPotential[0].size())
    gx = (int)m_gridPotential[0].size() - 1;
  if (gy > (int)m_gridPotential[0][0].size())
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

void MyNLP::ClearDensityGrid() {
  for (int k = 0; k < (int)m_gridDensity.size(); k++)
    for (unsigned int i = 0; i < m_gridDensity[k].size(); i++)
      for (unsigned int j = 0; j < m_gridDensity[k][i].size(); j++)
        m_gridDensity[k][i][j] = 0.0;
}

void MyNLP::UpdateDensityGridSpace(const int &n, const vector<double> &x,
                                   const vector<double> &z) {
  double allSpace =
      m_gridDensityWidth * m_gridDensityHeight * m_gridDensityThickness;
  for (unsigned int k = 0; k < m_gridDensity.size(); k++)
    for (unsigned int i = 0; i < m_gridDensity[k].size(); i++)
      for (unsigned int j = 0; j < m_gridDensity[k][i].size(); j++)
        m_gridDensitySpace[k][i][j] = allSpace;

  // for each cell b, update the corresponding bin area
  for (int b = 0; b < (int)m_pDB->m_modules.size(); b++) {
    if (false == m_pDB->m_modules[b].m_isFixed)
      continue;

    double w = m_pDB->m_modules[b].GetWidth(z[b] - 0.5);
    double h = m_pDB->m_modules[b].GetHeight(z[b] - 0.5);
    double t = m_pDB->m_modules[b].m_thickness;
    double left = x[b * 2] - w * 0.5;
    double bottom = x[b * 2 + 1] - h * 0.5;
    double back = z[b] - t * 0.5;
    double right = left + w;
    double top = bottom + h;
    double front = back + t;

    if (w == 0 || h == 0)
      continue;

    // find nearest bottom-left gird
    int gx = static_cast<int>(
        floor((left - m_pDB->m_coreRgn.left) / m_gridDensityWidth));
    int gy = static_cast<int>(
        floor((bottom - m_pDB->m_coreRgn.bottom) / m_gridDensityHeight));
    int gz = static_cast<int>(
        floor((back - m_pDB->m_back) / m_gridDensityThickness));
    if (gx < 0)
      gx = 0;
    if (gy < 0)
      gy = 0;
    if (gz < 0)
      gz = 0;

    for (int zOff = gz; zOff < (int)m_gridDensity.size(); zOff++) {
      double binBack = m_pDB->m_back + zOff * m_gridDensityThickness;
      double binFront = binBack + m_gridDensityThickness;
      if (binBack >= binFront)
        break;

      for (int xOff = gx; xOff < (int)m_gridDensity[zOff].size(); xOff++) {
        double binLeft = m_pDB->m_coreRgn.left + xOff * m_gridDensityWidth;
        double binRight = binLeft + m_gridDensityWidth;
        if (binLeft >= right)
          break;

        for (int yOff = gy; yOff < (int)m_gridDensity[zOff][xOff].size();
             yOff++) {
          double binBottom =
              m_pDB->m_coreRgn.bottom + yOff * m_gridDensityHeight;
          double binTop = binBottom + m_gridDensityHeight;
          if (binBottom >= top)
            break;

          m_gridDensitySpace[zOff][xOff][yOff] -=
              getOverlap(left, right, binLeft, binRight) *
              getOverlap(bottom, top, binBottom, binTop) *
              getOverlap(back, front, binBack, binFront);
        }
      }
    }

  } // each module

  int zeroSpaceCount = 0;
  m_totalFreeSpace = 0;
  for (unsigned int k = 0; k < m_gridDensity.size(); k++)
    for (unsigned int i = 0; i < m_gridDensity[k].size(); i++)
      for (unsigned int j = 0; j < m_gridDensity[k][i].size(); j++) {
        if (m_gridDensitySpace[k][i][j] < 1e-5) {
          m_gridDensitySpace[k][i][j] = 0.0;
          zeroSpaceCount++;
        }
        m_totalFreeSpace += m_gridDensitySpace[k][i][j];
      }

  if (param.bShow) {
    printf("DBIN: Zero space bin #= %d.  Total free space= %.0f.\n",
           zeroSpaceCount, m_totalFreeSpace);
    // printf( "[DB]   total free space: %.0f\n", m_pDB->m_totalFreeSpace );
  }
}

void MyNLP::UpdateDensityGrid(const int &n, const vector<double> &x,
                              const vector<double> &z) {
  ClearDensityGrid();
  // for each cell b, update the corresponding bin area
  for (unsigned int b = 0; b < m_pDB->m_modules.size(); b++) {
    // if(  m_pDB->m_modules[b].m_isOutCore || m_pDB->m_modules[b].m_isFixed )
    if (m_pDB->BlockOutCore(b) || m_pDB->m_modules[b].m_isFixed)
      continue;

    double w = m_pDB->m_modules[b].GetWidth(z[b] - 0.5);
    double h = m_pDB->m_modules[b].GetHeight(z[b] - 0.5);
    double t = m_pDB->m_modules[b].m_thickness;

    // rectangle range
    double left = x[b * 2] - w * 0.5;
    double bottom = x[b * 2 + 1] - h * 0.5;
    double back = z[b] - t * 0.5;
    double right = left + w;
    double top = bottom + h;
    double front = back + t;

    // find nearest gird
    int gx = static_cast<int>(
        floor((left - m_pDB->m_coreRgn.left) / m_gridDensityWidth));
    int gy = static_cast<int>(
        floor((bottom - m_pDB->m_coreRgn.bottom) / m_gridDensityHeight));
    int gz = static_cast<int>(
        floor((back - m_pDB->m_back) / m_gridDensityThickness));
    if (gx < 0)
      gx = 0;
    if (gy < 0)
      gy = 0;
    if (gz < 0)
      gz = 0;

    // Block is always inside the core region. Do not have to check boundary.
    for (unsigned int zOff = gz; zOff < m_gridDensity.size(); zOff++) {
      double binBack = m_pDB->m_back + m_gridDensityThickness * zOff;
      double binFront = binBack + m_gridDensityThickness;
      if (binBack >= binFront)
        break;

      for (unsigned int xOff = gx; xOff < m_gridDensity[zOff].size(); xOff++) {
        double binLeft = m_pDB->m_coreRgn.left + m_gridDensityWidth * xOff;
        double binRight = binLeft + m_gridDensityWidth;
        if (binLeft >= right)
          break;

        for (unsigned int yOff = gy; yOff < m_gridDensity[zOff][xOff].size();
             yOff++) {
          double binBottom =
              m_pDB->m_coreRgn.bottom + m_gridDensityHeight * yOff;
          double binTop = binBottom + m_gridDensityHeight;
          if (binBottom >= top)
            break;

          double volumn = getOverlap(left, right, binLeft, binRight) *
                          getOverlap(bottom, top, binBottom, binTop) *
                          getOverlap(back, front, binBack, binFront);

          m_gridDensity[zOff][xOff][yOff] += volumn;
        }
      }
    }
  } // each module
}

void MyNLP::CheckDensityGrid() {
  double totalDensity = 0;
  for (int k = 0; k < (int)m_gridDensity.size(); k++)
    for (int i = 0; i < (int)m_gridDensity[k].size(); i++)
      for (int j = 0; j < (int)m_gridDensity[k][i].size(); j++)
        totalDensity += m_gridDensity[k][i][j];

  double totalVolumn = 0;
  for (int i = 0; i < (int)m_pDB->m_modules.size(); i++) {
    // if( m_pDB->m_modules[i].m_isOutCore == false )
    if (m_pDB->BlockOutCore(i) == false)
      totalVolumn +=
          m_pDB->m_modules[i].m_area * m_pDB->m_modules[i].m_thickness;
  }

  printf(" %f %f\n", totalDensity, totalVolumn);
}

void MyNLP::CreateDensityGrid(int nGrid) {
  m_gridDensity.resize(nGrid);
  m_gridDensitySpace.resize(nGrid);

  for (int k = 0; k < (int)m_gridDensity.size(); k++) {
    m_gridDensity[k].resize(nGrid);
    for (int i = 0; i < nGrid; i++)
      m_gridDensity[k][i].resize(nGrid);

    m_gridDensitySpace[k].resize(nGrid);
    for (int i = 0; i < nGrid; i++)
      m_gridDensitySpace[k][i].resize(nGrid);
  }

  m_gridDensityWidth =
      ((double)m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left) / nGrid;
  m_gridDensityHeight =
      ((double)m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom) / nGrid;
  m_gridDensityThickness = ((double)m_pDB->m_front - m_pDB->m_back) / nGrid;
  m_gridDensityTarget = m_pDB->m_totalModuleVolumn / (nGrid * nGrid * nGrid);

  // printf( "Density Target Area = %f\n", m_gridDensityTarget );
  // printf( "Design Density = %f\n",
  // m_gridDensityTarget/m_gridDensityWidth/m_gridDensityHeight );
  // 2006-03-21 compute always overflow area

  double alwaysOver = 0.0;
  if (m_targetUtil > 0.0 && m_targetUtil < 1.0) {
    for (unsigned int i = 0; i < m_pDB->m_modules.size(); i++) {
      if (m_pDB->m_modules[i].m_isFixed)
        continue;
      if (m_pDB->m_modules[i].GetWidth(z[i] - 0.5) >= 2 * m_gridDensityWidth &&
          m_pDB->m_modules[i].GetHeight(z[i] - 0.5) >=
              2 * m_gridDensityHeight &&
          m_pDB->m_modules[i].m_thickness >= 2 * m_gridDensityThickness)
        alwaysOver +=
            (m_pDB->m_modules[i].GetWidth(z[i] - 0.5) - m_gridDensityWidth) *
            (m_pDB->m_modules[i].GetHeight(z[i] - 0.5) - m_gridDensityHeight) *
            (m_pDB->m_modules[i].m_thickness - m_gridDensityThickness) *
            (1.0 - m_targetUtil);
    }
    if (param.bShow)
      printf("DBIN: Always over: %.0f (%.1f%%)\n", alwaysOver,
             alwaysOver / m_pDB->m_totalMovableModuleVolumn * 100.0);
  }
  m_alwaysOverVolumn = alwaysOver;
}

// Get potential/density grid information
// //////////////////////////////////////////////////////////////////////

double MyNLP::GetMaxDensity() {
  double maxUtilization = 0;
  double binVolumn =
      m_gridDensityWidth * m_gridDensityHeight * m_gridDensityThickness;
  for (int k = 0; k < (int)m_gridDensity.size(); k++)
    for (int i = 0; i < (int)m_gridDensity[k].size(); i++)
      for (int j = 0; j < (int)m_gridDensity[k][i].size(); j++) {
        if (m_gridDensitySpace[k][i][j] > 1e-5) {
          double preplacedVolumn = binVolumn - m_gridDensitySpace[k][i][j];
          double utilization =
              (m_gridDensity[k][i][j] + preplacedVolumn) / binVolumn;
          if (utilization > maxUtilization)
            maxUtilization = utilization;
        }
      }
  return maxUtilization;
}

double MyNLP::GetTotalOverDensityLB() {
  double over = 0;
  for (unsigned int k = 0; k < m_gridDensity.size(); k++)
    for (unsigned int i = 0; i < m_gridDensity[k].size(); i++)
      for (unsigned int j = 0; j < m_gridDensity[k][i].size(); j++) {
        double targetSpace = m_gridDensitySpace[k][i][j] * m_targetUtil;
        if (targetSpace > 1e-5 && m_gridDensity[k][i][j] > targetSpace)
          over += m_gridDensity[k][i][j] - targetSpace;
      }
  // TODO: remove "1.0"
  return (over - m_alwaysOverVolumn) / (m_pDB->m_totalMovableModuleVolumn) +
         1.0;
}

double MyNLP::GetTotalOverDensity() {
  double over = 0;
  for (unsigned int k = 0; k < m_gridDensity.size(); k++)
    for (unsigned int i = 0; i < m_gridDensity[k].size(); i++)
      for (unsigned int j = 0; j < m_gridDensity[k][i].size(); j++) {
        double targetSpace = m_gridDensitySpace[k][i][j] * m_targetUtil;
        if (m_gridDensity[k][i][j] > targetSpace)
          over += m_gridDensity[k][i][j] - targetSpace;
      }
  // TODO: remove "1.0"
  return (over - m_alwaysOverVolumn) / (m_pDB->m_totalMovableModuleVolumn) +
         1.0;
}

double MyNLP::GetTotalOverPotential() {
  double over = 0;
  for (unsigned int k = 0; k < m_gridPotential.size(); k++)
    for (unsigned int i = 0; i < m_gridPotential[k].size(); i++)
      for (unsigned int j = 0; j < m_gridPotential[k][i].size(); j++) {
        if (m_gridPotential[k][i][j] > m_expBinPotential[k][i][j])
          over += m_gridPotential[k][i][j] - m_expBinPotential[k][i][j];
      }
  // TODO: remove "1.0"
  return (over - m_alwaysOverPotential) / (m_pDB->m_totalMovableModuleVolumn) +
         1.0;
}

double MyNLP::GetNonZeroDensityGridPercent() {
  double nonZero = 0;
  for (int k = 0; k < (int)m_gridDensity.size(); k++)
    for (int i = 0; i < (int)m_gridDensity[k].size(); i++)
      for (int j = 0; j < (int)m_gridDensity[k][i].size(); j++)
        if (m_gridDensity[k][i][j] > 0 || m_gridDensitySpace[k][i][j] == 0)
          nonZero += 1.0;
  return nonZero / m_gridDensity.size() / m_gridDensity.size() /
         m_gridDensity.size();
}

double MyNLP::GetNonZeroGridPercent() {
  double nonZero = 0;
  for (int k = 0; k < (int)m_gridPotential.size(); k++)
    for (int i = 0; i < (int)m_gridPotential[k].size(); i++)
      for (int j = 0; j < (int)m_gridPotential[k][i].size(); j++)
        if (m_gridPotential[k][i][j] > 0)
          nonZero += 1.0;
  return nonZero / m_gridPotential.size() / m_gridPotential.size() /
         m_gridPotential.size();
}

double MyNLP::GetMaxPotential() {
  double maxDensity = 0;
  for (unsigned int k = 0; k < m_gridPotential.size(); k++)
    for (unsigned int i = 0; i < m_gridPotential[k].size(); i++)
      for (unsigned int j = 0; j < m_gridPotential[k][i].size(); j++)
        if (m_gridPotential[k][i][j] > maxDensity)
          maxDensity = m_gridPotential[k][i][j];
  return maxDensity;
}

double MyNLP::GetAvgPotential() {
  const double targetDensity = 1.0;
  double avgDensity = 0;
  int overflowCount = 0;

  for (unsigned int k = 0; k < m_gridPotential.size(); k++)
    for (unsigned int i = 0; i < m_gridPotential[k].size(); i++)
      for (unsigned int j = 0; j < m_gridPotential[k][i].size(); j++)
        if (m_gridPotential[k][i][j] > targetDensity) {
          overflowCount++;
          avgDensity += m_gridPotential[k][i][j];
        }
  return avgDensity / overflowCount;
}

// Output potential/density grid to a file
// /////////////////////////////////////////////////////////////////////

// Output potential data for gnuplot
void MyNLP::OutputPotentialGrid(string filename, const int &k) {
  int stepSize = (int)m_gridPotential[k].size() / 100;
  if (stepSize == 0)
    stepSize = 1;
  FILE *out = fopen(filename.c_str(), "w");
  double binArea = m_potentialGridWidth * m_potentialGridHeight;
  for (unsigned int j = 0; j < m_gridPotential[k].size(); j += stepSize) {
    for (unsigned int i = 0; i < m_gridPotential[k].size(); i += stepSize)
      fprintf(out, "%.03f ",
              (m_gridPotential[k][i][j] + m_basePotential[k][i][j]) / binArea);
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  fclose(out);
}

// Output potential data for gnuplot
void MyNLP::OutputDensityGrid(string filename, const int &k) {
  int stepSize = 1;
  FILE *out = fopen(filename.c_str(), "w");
  for (unsigned int j = 0; j < m_gridDensity[k].size(); j += stepSize) {
    for (unsigned int i = 0; i < m_gridDensity[k].size(); i += stepSize) {
      double targetSpace = m_gridDensitySpace[k][i][j] * m_targetUtil;
      if (m_gridDensity[k][i][j] > targetSpace) {
        // % overflow
        fprintf(out, "%.03f ",
                (m_gridDensity[k][i][j] - targetSpace) /
                    m_pDB->m_totalMovableModuleVolumn * 100);
      } else {
        fprintf(out, "%.03f ", 0.0);
      }
    }
    fprintf(out, "\n");
  }
  fprintf(out, "\n");
  fclose(out);
}

// 2007-04-02 (donnie) Extracted base potential smoothing
void MyNLP::SmoothBasePotential(const int &layer) {
  // Gaussian smoothing
  GaussianSmooth smooth;
  int r = m_smoothR;
  smooth.Gaussian2D(r, 6 * r + 1);
  smooth.Smooth(m_basePotential[layer]);

  // Level smoothing
  LevelSmoothBasePotential(double(m_smoothDelta), layer);

  // Increase the height of bins with density = 1.0
  if (m_smoothDelta != 1)
    return;

#if 0
    if( param.bShow )
    {
	printf( "Generating base potential... " );
	fflush( stdout );
    }
#endif

  static vector<vector<double>> moreSmooth;

  // Save CPU time. Compute only at the first time.
  if (moreSmooth.size() != m_basePotential[layer].size()) {
    moreSmooth = m_basePotential[layer];
    r = m_smoothR * 6;
    int kernel_size = 5 * r;
    if (kernel_size % 2 == 0)
      kernel_size++;
    smooth.Gaussian2D(r, kernel_size);
    smooth.Smooth(moreSmooth);
  }

  // Merge basePotential and moreSmooth
  double binArea = m_potentialGridWidth * m_potentialGridHeight;
  double halfBinArea = binArea / 2;
  int changeCount = 0;
  double scale = 3;
  for (unsigned int i = 0; i < moreSmooth.size(); i++)
    for (unsigned int j = 0; j < moreSmooth[i].size(); j++) {
      double free = binArea - m_basePotential[layer][i][j];
      if (free < 1e-4 &&
          moreSmooth[i][j] > halfBinArea) // no space or high enough
      {
        m_basePotential[layer][i][j] +=
            (moreSmooth[i][j] - halfBinArea) * scale;
        changeCount++;
      }
    }

#if 0
    if( param.bShow )
	printf( "%d changed\n", changeCount );
#endif
}

// (kaie) 2009-09-10 extracted base potential smoothing 3d
void MyNLP::SmoothBasePotential3D() {
  // Gaussian smoothing
  GaussianSmooth smooth;
  int r = m_smoothR;
  smooth.Gaussian3D(r, 6 * r + 1);
  smooth.Smooth(m_basePotential);

  // Level smoothing
  LevelSmoothBasePotential3D(double(m_smoothDelta));
  // Increase the height of bins with density = 1.0
  if (m_smoothDelta != 1)
    return;

#if 0
    if( param.bShow )
    {
	printf( "Generating base potential... " );
	fflush( stdout );
    }
#endif

  static vector<vector<vector<double>>> moreSmooth;

  // Save CPU time. Compute only at the first time.
  if (moreSmooth.size() != m_basePotential.size()) {
    moreSmooth = m_basePotential;
    r = m_smoothR * 6;
    int kernel_size = 5 * r;
    if (kernel_size % 2 == 0)
      kernel_size++;
    smooth.Gaussian3D(r, kernel_size);
    smooth.Smooth(moreSmooth);
  }

  // Merge basePotential and moreSmooth
  double binVolumn =
      m_potentialGridWidth * m_potentialGridHeight * m_potentialGridThickness;
  double halfBinVolumn = binVolumn / 2;
  int changeCount = 0;
  double scale = 3;
  for (unsigned int k = 0; k < moreSmooth.size(); k++)
    for (unsigned int i = 0; i < moreSmooth[k].size(); i++)
      for (unsigned int j = 0; j < moreSmooth[k][i].size(); j++) {
        double free = binVolumn - m_basePotential[k][i][j];
        if (free < 1e-4 &&
            moreSmooth[k][i][j] > halfBinVolumn) // no space or high enough
        {
          m_basePotential[k][i][j] +=
              (moreSmooth[k][i][j] - halfBinVolumn) * scale;
          changeCount++;
        }
      }
}

//@ Brian 2007-04-18 Calculate Net Weight In NLP
double MyNLP::NetWeightCalc(int netDegree) {
  double netWeight = 1.0;
  switch (netDegree) {
  case 0:
  case 1:
  case 2:
  case 3:
    netWeight = 1.0;
    break;
  case 4:
    // flute
    // netWeight = 1.0224948875255623721881390593047;
    netWeight = 1.04108;
    break;
  case 5:
    // flute
    // netWeight = 1.049758555532227587654839386941;
    netWeight = 1.07653;
    break;
  case 6:
    // flute
    // netWeight = 1.0755001075500107550010755001076;
    netWeight = 1.10338;
    break;
  case 7:
    // flute
    // netWeight = 1.0996261271167802946998020672971;
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
    // flute
    // netWeight = 1.1944577161968466316292403248925;
    netWeight = 1.34034;
    break;
  default:
    // flute
    // netWeight = 1.405086412814388084867219333989;
    netWeight = 1.37924;
    break;
  }

  return netWeight;
}

// Brian 2007-04-30

void MyNLP::CreatePotentialGridNet() {

  int realGridSize = m_potentialGridSize;
  m_gridPotentialNet.clear();

  m_newPotentialNet.resize(m_pDB->m_nets.size());
  m_gridPotentialNet.resize(realGridSize);

  m_basePotentialNet.resize(realGridSize);
  // m_binFreeSpaceNet.resize( realGridSize );
  m_expBinPotentialNet.resize(realGridSize);

  for (int i = 0; i < realGridSize; i++) {
    m_gridPotentialNet[i].resize(realGridSize, 0);

    m_basePotentialNet[i].resize(realGridSize, 0);
    // m_binFreeSpaceNet[i].resize( realGridSize, 0 );
    m_expBinPotentialNet[i].resize(realGridSize, 0);
  }
}

void MyNLP::CreateDensityGridNet(int nGrid) {
  m_gridDensityNet.resize(nGrid);
  m_gridDensitySpaceNet.resize(nGrid);
  for (int i = 0; i < nGrid; i++) {
    m_gridDensityNet[i].resize(nGrid, 0);
    m_gridDensitySpaceNet[i].resize(nGrid, 0);
  }
}

// void MyNLP::CatchCongParam()
// {
//     double totalPitch = 0.0;
//     double levelNum = 0;
//     for( unsigned int i = 2 ; i < m_pDB->m_pLEF->m_metalPitch.size() ; i++ )
//     {
//         levelNum++;
//         totalPitch += m_pDB->m_pLEF->m_metalPitch[i] *
//         m_pDB->m_pLEF->m_lefUnit;
// 	}
//     m_dAvgPitch = totalPitch / levelNum;
//     m_dLevelNum = levelNum / 2;
//     m_dMacroOccupy = 10;
// }

void MyNLP::ComputeNetBoxInfo() {
  double alwaysOverDensity = 0.0;
  double alwaysOverPotential = 0.0;
  double totalNetDensity = 0.0;
  for (unsigned int i = 0; i < m_pDB->m_nets.size(); i++) {
    if (m_pDB->m_nets[i].size() < 2)
      continue;
    int pinId = m_pDB->m_nets[i][0];
    double left = m_pDB->m_pins[pinId].absX;
    double right = left;
    double top = m_pDB->m_pins[pinId].absY;
    double bottom = top;

    for (unsigned int j = 1; j < m_pDB->m_nets[i].size(); j++) {
      pinId = m_pDB->m_nets[i][j];
      left = min(left, m_pDB->m_pins[pinId].absX);
      right = max(right, m_pDB->m_pins[pinId].absX);
      bottom = min(bottom, m_pDB->m_pins[pinId].absY);
      top = max(top, m_pDB->m_pins[pinId].absY);
    }
    m_NetBoxInfo[i].m_dWidth = right - left;
    m_NetBoxInfo[i].m_dHeight = top - bottom;
    m_NetBoxInfo[i].m_dArea =
        m_NetBoxInfo[i].m_dWidth * m_NetBoxInfo[i].m_dHeight;
    m_NetBoxInfo[i].m_dCenterX = left + m_NetBoxInfo[i].m_dWidth / 2;
    m_NetBoxInfo[i].m_dCenterY = bottom + m_NetBoxInfo[i].m_dHeight / 2;
    if (m_NetBoxInfo[i].m_dArea > 0) {
      m_NetBoxInfo[i].m_dDensity =
          (m_dAvgPitch / m_dLevelNum) *
          (m_NetBoxInfo[i].m_dWidth + m_NetBoxInfo[i].m_dHeight) /
          m_NetBoxInfo[i].m_dArea;
    } else {
      m_NetBoxInfo[i].m_dDensity = 1.0 / m_dLevelNum;
    }

    totalNetDensity += m_NetBoxInfo[i].m_dDensity * m_NetBoxInfo[i].m_dArea;

    /*if (m_dCongUtil > 0.0 && m_dCongUtil < 1.0)
    {
        if( m_NetBoxInfo[i].m_dWidth >= 2 * m_potentialGridWidth &&
    m_NetBoxInfo[i].m_dHeight >= 2 * m_potentialGridHeight )
        {
                    alwaysOverPotential +=
                        (m_NetBoxInfo[i].m_dWidth - m_potentialGridWidth ) *
                        (m_NetBoxInfo[i].m_dHeight - m_potentialGridHeight ) *
                        (1.0 - m_dCongUtil );
        }

        if( m_NetBoxInfo[i].m_dWidth >= 2 * m_gridDensityWidth &&
    m_NetBoxInfo[i].m_dHeight >= 2 * m_gridDensityHeight )
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
  // Brian 2007-06-18
  if (param.bCongStopDynamic) {
    double totalChip = (m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left) *
                       (m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom);
    m_dCongUtil = totalNetDensity / totalChip + param.dCongTargetUtil;
    // if (m_dCongUtil > 1.0)
    // m_dCongUtil = 1.0;
  } else {
    m_dCongUtil = param.dCongTargetUtil;
    // if (m_dCongUtil > 1.0)
    // m_dCongUtil = 1.0;
  }
  //@Brian 2007-06-18
}

void MyNLP::ComputeNewPotentialGridNet() {

  for (unsigned int i = 0; i < m_pDB->m_nets.size(); i++) {
    if (m_pDB->m_nets[i].size() < 2)
      continue;

    double cellX = m_NetBoxInfo[i].m_dCenterX;
    double cellY = m_NetBoxInfo[i].m_dCenterY;
    double potentialRX = _potentialRX;
    double potentialRY = _potentialRY;
    double width = m_NetBoxInfo[i].m_dWidth;
    double height = m_NetBoxInfo[i].m_dHeight;

    if (height < m_potentialGridHeight && width < m_potentialGridWidth)
      width = height = 0;

    double left = cellX - width * 0.5 - potentialRX;
    double bottom = cellY - height * 0.5 - potentialRY;
    double right = cellX + (cellX - left);
    double top = cellY + (cellY - bottom);
    if (left <
        m_pDB->m_coreRgn.left - m_potentialGridPadding * m_potentialGridWidth)
      left =
          m_pDB->m_coreRgn.left - m_potentialGridPadding * m_potentialGridWidth;
    if (right >
        m_pDB->m_coreRgn.right + m_potentialGridPadding * m_potentialGridWidth)
      right = m_pDB->m_coreRgn.right +
              m_potentialGridPadding * m_potentialGridWidth;
    if (bottom < m_pDB->m_coreRgn.bottom -
                     m_potentialGridPadding * m_potentialGridHeight)
      bottom = m_pDB->m_coreRgn.bottom -
               m_potentialGridPadding * m_potentialGridHeight;
    if (top >
        m_pDB->m_coreRgn.top + m_potentialGridPadding * m_potentialGridHeight)
      top =
          m_pDB->m_coreRgn.top + m_potentialGridPadding * m_potentialGridHeight;

    int gx, gy;
    GetClosestGrid(left, bottom, gx, gy);
    int gxx, gyy;
    double xx, yy;

    double totalPotential = 0;
    vector<potentialStruct> &potentialList = m_newPotentialNet[i];
    potentialList.clear();

    for (gxx = gx, xx = GetXGrid(gx);
         xx <= right && gxx < (int)m_gridPotentialNet.size();
         gxx++, xx += m_potentialGridWidth) {
      for (gyy = gy, yy = GetYGrid(gy);
           yy <= top && gyy < (int)m_gridPotentialNet.size();
           gyy++, yy += m_potentialGridHeight) {
        double potential = GetPotential(cellX, xx, potentialRX, width) *
                           GetPotential(cellY, yy, potentialRY, height);
        if (potential > 0) {
          totalPotential += potential;
          potentialList.push_back(potentialStruct(gxx, gyy, potential));
        }
      }
    }

    // normalize the potential so that total potential equals the net total
    // density
    double scale =
        m_NetBoxInfo[i].m_dArea * m_NetBoxInfo[i].m_dDensity / totalPotential;

    m_NetBoxInfo[i].m_dNorm = scale; // normalization factor for the net i
  }                                  // for each net
}

void MyNLP::ClearPotentialGridNet() {
  for (int gx = 0; gx < (int)m_gridPotentialNet.size(); gx++)
    fill(m_gridPotentialNet[gx].begin(), m_gridPotentialNet[gx].end(), 0.0);
}

void MyNLP::UpdatePotentialGridNet() {
  ClearPotentialGridNet();
  for (unsigned int i = 0; i < m_newPotentialNet.size(); i++) // for each cell
    for (unsigned int j = 0; j < m_newPotentialNet[i].size();
         j++) // for each bin
    {
      int gx = m_newPotentialNet[i][j].gx;
      int gy = m_newPotentialNet[i][j].gy;
      m_gridPotentialNet[gx][gy] +=
          m_newPotentialNet[i][j].potential * m_NetBoxInfo[i].m_dNorm;
    }
}

void MyNLP::ClearDensityGridNet() {
  for (unsigned int i = 0; i < m_gridDensityNet.size(); i++)
    for (unsigned int j = 0; j < m_gridDensityNet[i].size(); j++)
      m_gridDensityNet[i][j] = 0.0;
}

void MyNLP::UpdateDensityGridNet() {
  ClearDensityGridNet();

  // for each net b, update the corresponding bin area
  for (unsigned int b = 0; b < m_pDB->m_nets.size(); b++) {
    if (m_pDB->m_nets.size() < 2)
      continue;

    double w = m_NetBoxInfo[b].m_dWidth;
    double h = m_NetBoxInfo[b].m_dHeight;

    // rectangle range
    double left = m_NetBoxInfo[b].m_dCenterX - w * 0.5;
    double bottom = m_NetBoxInfo[b].m_dCenterY - h * 0.5;
    double right = left + w;
    double top = bottom + h;

    // find nearest gird
    int gx = static_cast<int>(
        floor((left - m_pDB->m_coreRgn.left) / m_gridDensityWidth));
    int gy = static_cast<int>(
        floor((bottom - m_pDB->m_coreRgn.bottom) / m_gridDensityHeight));
    if (gx < 0)
      gx = 0;
    if (gy < 0)
      gy = 0;

    // Block is always inside the core region. Do not have to check boundary.
    for (unsigned int xOff = gx; xOff < m_gridDensityNet.size(); xOff++) {
      double binLeft = m_pDB->m_coreRgn.left + m_gridDensityWidth * xOff;
      double binRight = binLeft + m_gridDensityWidth;
      if (binLeft >= right)
        break;

      for (unsigned int yOff = gy; yOff < m_gridDensityNet[xOff].size();
           yOff++) {
        double binBottom = m_pDB->m_coreRgn.bottom + m_gridDensityHeight * yOff;
        double binTop = binBottom + m_gridDensityHeight;
        if (binBottom >= top)
          break;

        double area = getOverlap(left, right, binLeft, binRight) *
                      getOverlap(bottom, top, binBottom, binTop);
        m_gridDensityNet[xOff][yOff] += area * m_NetBoxInfo[b].m_dDensity;
      }
    }
  } // each net
}

void MyNLP::GetDensityPaneltyNet() {
  double den = 0;
  for (unsigned int i = 0; i < m_gridPotentialNet.size(); i++) {
    // double expPotential = m_potentialGridWidth * m_potentialGridHeight *
    // m_dCongUtil;
    for (unsigned int j = 0; j < m_gridPotentialNet[i].size(); j++) {
      den += (m_gridPotentialNet[i][j] - m_expBinPotentialNet[i][j]) *
             (m_gridPotentialNet[i][j] - m_expBinPotentialNet[i][j]);
    }
  }
  m_dDensityNet = den;
}

void MyNLP::UpdateGradCong() {
  double gradDensityX;
  double gradDensityY;

  for (unsigned int i = 0; i < grad_congestion.size(); i++)
    grad_congestion[i] = 0;

  for (unsigned int i = 0; i < m_pDB->m_nets.size(); i++) // for each net
  {
    if (m_pDB->m_nets[i].size() < 2)
      continue;

    GetCongGrad(i, gradDensityX, gradDensityY); // bell-shaped potential

    for (unsigned int j = 0; j < m_pDB->m_nets[i].size(); j++) {
      int pinId = m_pDB->m_nets[i][j];
      int moduleId = m_pDB->m_pins[pinId].moduleId;
      if (moduleId >= (int)m_pDB->m_modules.size())
        continue;
      if (m_pDB->m_modules[moduleId].m_isFixed ||
          m_pDB->m_modules[moduleId].m_netsId.size() == 0)
        continue;
      double smoothX = 1.0;
      double smoothY = 1.0;
      if (param.bCongSmooth) {
        CongSmooth(moduleId, i, pinId, smoothX, smoothY);
        assert(smoothX >= 0 && smoothX < 1.0001);
        assert(smoothY >= 0 && smoothY < 1.0001);
      }
      grad_congestion[2 * moduleId] += gradDensityX * smoothX;
      grad_congestion[2 * moduleId + 1] += gradDensityY * smoothY;
    }
  } // for each net
}

void MyNLP::CongSmooth(int moduleId, int netId, int pinId, double &smoothX,
                       double &smoothY) {
  if (m_usePin[moduleId]) {
    smoothX =
        _expPins[2 * pinId] / m_nets_sum_exp_xi_over_alpha[netId] -
        1.0 / _expPins[2 * pinId] / m_nets_sum_exp_inv_xi_over_alpha[netId];
    smoothY =
        _expPins[2 * pinId + 1] / m_nets_sum_exp_yi_over_alpha[netId] -
        1.0 / _expPins[2 * pinId + 1] / m_nets_sum_exp_inv_yi_over_alpha[netId];
  } else {
    smoothX =
        _expX[2 * moduleId] / m_nets_sum_exp_xi_over_alpha[netId] -
        1.0 / _expX[2 * moduleId] / m_nets_sum_exp_inv_xi_over_alpha[netId];
    smoothY =
        _expX[2 * moduleId + 1] / m_nets_sum_exp_yi_over_alpha[netId] -
        1.0 / _expX[2 * moduleId + 1] / m_nets_sum_exp_inv_yi_over_alpha[netId];
  }
  smoothX = fabs(smoothX);
  smoothY = fabs(smoothY);
}

void MyNLP::GetCongGrad(const int &i, double &gradX, double &gradY) {
  double cellX = m_NetBoxInfo[i].m_dCenterX;
  double cellY = m_NetBoxInfo[i].m_dCenterY;

  double width = m_NetBoxInfo[i].m_dWidth;
  double height = m_NetBoxInfo[i].m_dHeight;
  //// use square to model small std-cells
  if (height < m_potentialGridHeight && width < m_potentialGridWidth)
    width = height = 0;

  double left = cellX - width * 0.5 - _potentialRX;
  double bottom = cellY - height * 0.5 - _potentialRY;
  double right = cellX + (cellX - left);
  double top = cellY + (cellY - bottom);
  if (left <
      m_pDB->m_coreRgn.left - m_potentialGridPadding * m_potentialGridWidth)
    left =
        m_pDB->m_coreRgn.left - m_potentialGridPadding * m_potentialGridWidth;
  if (right >
      m_pDB->m_coreRgn.right + m_potentialGridPadding * m_potentialGridWidth)
    right =
        m_pDB->m_coreRgn.right + m_potentialGridPadding * m_potentialGridWidth;
  if (bottom <
      m_pDB->m_coreRgn.bottom - m_potentialGridPadding * m_potentialGridHeight)
    bottom = m_pDB->m_coreRgn.bottom -
             m_potentialGridPadding * m_potentialGridHeight;
  if (top >
      m_pDB->m_coreRgn.top + m_potentialGridPadding * m_potentialGridHeight)
    top = m_pDB->m_coreRgn.top + m_potentialGridPadding * m_potentialGridHeight;

  int gx, gy;
  GetClosestGrid(left, bottom, gx, gy);

  int gxx, gyy;
  double xx, yy;
  gradX = 0.0;
  gradY = 0.0;
  // double expBinPotential = m_potentialGridWidth * m_potentialGridHeight *
  // m_dCongUtil;
  for (gxx = gx, xx = GetXGrid(gx);
       xx <= right && gxx < (int)m_gridPotentialNet.size();
       gxx++, xx += m_potentialGridWidth) {

    for (gyy = gy, yy = GetYGrid(gy);
         yy <= top && gyy < (int)m_gridPotentialNet.size();
         gyy++, yy += m_potentialGridHeight) {

      double gX = GetGradPotential(cellX, xx, _potentialRX, width) *
                  GetPotential(cellY, yy, _potentialRY, height) *
                  (m_gridPotentialNet[gxx][gyy] -
                   m_expBinPotentialNet[gxx][gyy] * m_dCongUtil) *
                  m_NetBoxInfo[i].m_dNorm;
      double gY = GetPotential(cellX, xx, _potentialRX, width) *
                  GetGradPotential(cellY, yy, _potentialRY, height) *
                  (m_gridPotentialNet[gxx][gyy] -
                   m_expBinPotentialNet[gxx][gyy] * m_dCongUtil) *
                  m_NetBoxInfo[i].m_dNorm;

      gradX += gX;
      gradY += gY;
    }
  } // for each grid
}

bool MyNLP::AdjustForceNet(vector<double> &p_grad_wl,
                           vector<double> &p_grad_potential,
                           vector<double> &p_grad_congestion) {
  double totalGrad = 0;
  int size = m_pDB->m_modules.size();
  for (int i = 0; i < size; i++) {
    double value = (p_grad_wl[2 * i] + p_grad_potential[2 * i] +
                    p_grad_congestion[2 * i]) *
                       (p_grad_wl[2 * i] + p_grad_potential[2 * i] +
                        p_grad_congestion[2 * i]) +
                   (p_grad_wl[2 * i + 1] + p_grad_potential[2 * i + 1] +
                    p_grad_congestion[2 * i + 1]) *
                       (p_grad_wl[2 * i + 1] + p_grad_potential[2 * i + 1] +
                        p_grad_congestion[2 * i + 1]);
    totalGrad += value;
  }

  if (isNaN(totalGrad))
    return false;
  assert(!isNaN(totalGrad)); // it is checked in GoSolve()

  double avgGrad = sqrt(totalGrad / size);

  // Do truncation
  double expMaxGrad = avgGrad * param.truncationFactor; // x + y
  double expMaxGradSquare = expMaxGrad * expMaxGrad;
  for (int i = 0; i < size; i++) {
    double valueSquare =
        (p_grad_wl[2 * i] + p_grad_potential[2 * i] +
         p_grad_congestion[2 * i]) *
            (p_grad_wl[2 * i] + p_grad_potential[2 * i] +
             p_grad_congestion[2 * i]) +
        (p_grad_wl[2 * i + 1] + p_grad_potential[2 * i + 1] +
         p_grad_congestion[2 * i + 1]) *
            (p_grad_wl[2 * i + 1] + p_grad_potential[2 * i + 1] +
             p_grad_congestion[2 * i + 1]);
    if (valueSquare == 0) {
      // avoid value = 0 let to inf
      p_grad_wl[2 * i] = p_grad_wl[2 * i + 1] = 0;
      p_grad_potential[2 * i] = p_grad_potential[2 * i + 1] = 0;
      p_grad_congestion[2 * i] = p_grad_congestion[2 * i + 1] = 0;
    } else {
      if (valueSquare > expMaxGradSquare) {
        double value = sqrt(valueSquare);
        p_grad_wl[2 * i] = p_grad_wl[2 * i] * expMaxGrad / value;
        p_grad_wl[2 * i + 1] = p_grad_wl[2 * i + 1] * expMaxGrad / value;
        p_grad_potential[2 * i] = p_grad_potential[2 * i] * expMaxGrad / value;
        p_grad_potential[2 * i + 1] =
            p_grad_potential[2 * i + 1] * expMaxGrad / value;
        p_grad_congestion[2 * i] =
            p_grad_congestion[2 * i] * expMaxGrad / value;
        p_grad_congestion[2 * i + 1] =
            p_grad_congestion[2 * i + 1] * expMaxGrad / value;
      }
    }
  }
  return true;
}

double MyNLP::GetTotalOverDensityNet() {
  double over = 0;
  for (unsigned int i = 0; i < m_gridDensityNet.size(); i++)
    for (unsigned int j = 0; j < m_gridDensityNet[i].size(); j++) {
      double targetSpace = m_gridDensitySpaceNet[i][j] * m_dCongUtil;
      // double targetSpace = m_gridDensityWidth * m_gridDensityHeight *
      // m_dCongUtil;
      if (m_gridDensityNet[i][j] > targetSpace)
        over += m_gridDensityNet[i][j] - targetSpace;
    }

  return (over - m_alwaysOverDensityNet) / m_dTotalNetDensity + 1.0;
}

double MyNLP::GetTotalOverPotentialNet() {
  double over = 0;
  for (unsigned int i = 0; i < m_gridPotentialNet.size(); i++)
    for (unsigned int j = 0; j < m_gridPotentialNet[i].size(); j++) {
      // double targetSpace = m_potentialGridWidth * m_potentialGridHeight *
      // m_dCongUtil;
      if (m_gridPotentialNet[i][j] > m_expBinPotentialNet[i][j] * m_dCongUtil)
        over +=
            m_gridPotentialNet[i][j] - m_expBinPotentialNet[i][j] * m_dCongUtil;
    }

  // TODO: remove "1.0"
  return (over - m_alwaysOverPotentialNet) / m_dTotalNetDensity + 1.0;
}

double MyNLP::GetMaxDensityNet() {
  double maxUtilization = 0;
  double binArea = m_gridDensityWidth * m_gridDensityHeight;
  for (int i = 0; i < (int)m_gridDensityNet.size(); i++)
    for (int j = 0; j < (int)m_gridDensityNet[i].size(); j++) {
      if (m_gridDensitySpaceNet[i][j] > 1e-5) {
        double preplacedarea = binArea - m_gridDensitySpaceNet[i][j];
        double utilization = (m_gridDensityNet[i][j] + preplacedarea) / binArea;
        // double utilization = m_gridDensityNet[i][j]  / binArea;
        if (utilization > maxUtilization)
          maxUtilization = utilization;
      }
    }
  return maxUtilization;
}

//@Brian 2007-04-30

// Brian 2007-07-23

void MyNLP::UpdateDensityGridSpaceNet() {

  double allSpace = m_gridDensityWidth * m_gridDensityHeight;
  for (unsigned int i = 0; i < m_gridDensitySpaceNet.size(); i++)
    for (unsigned int j = 0; j < m_gridDensitySpaceNet[i].size(); j++)
      m_gridDensitySpaceNet[i][j] = allSpace;

  // for each cell b, update the corresponding bin area
  for (int b = 0; b < (int)m_pDB->m_modules.size(); b++) {
    if (false == m_pDB->m_modules[b].m_isFixed)
      continue;

    double w = m_pDB->m_modules[b].GetWidth(z[b] - 0.5);
    double h = m_pDB->m_modules[b].GetHeight(z[b] - 0.5);
    double left = x[b * 2] - w * 0.5;
    double bottom = x[b * 2 + 1] - h * 0.5;
    double right = left + w;
    double top = bottom + h;

    if (w == 0 || h == 0)
      continue;

    if (h <= m_dMacroOccupy * m_pDB->m_rowHeight)
      continue;

    double occupyRatio =
        ((double)(((int)(h / m_pDB->m_rowHeight / m_dMacroOccupy)) - 1)) / 2.0 /
        m_dLevelNum;
    if (occupyRatio > 1)
      occupyRatio = 1;
    if (occupyRatio < 0)
      occupyRatio = 0;

    // find nearest bottom-left gird
    int gx = static_cast<int>(
        floor((left - m_pDB->m_coreRgn.left) / m_gridDensityWidth));
    int gy = static_cast<int>(
        floor((bottom - m_pDB->m_coreRgn.bottom) / m_gridDensityHeight));
    if (gx < 0)
      gx = 0;
    if (gy < 0)
      gy = 0;

    for (int xOff = gx; xOff < (int)m_gridDensitySpaceNet.size(); xOff++) {
      double binLeft = m_pDB->m_coreRgn.left + xOff * m_gridDensityWidth;
      double binRight = binLeft + m_gridDensityWidth;
      if (binLeft >= right)
        break;

      for (int yOff = gy; yOff < (int)m_gridDensitySpaceNet[xOff].size();
           yOff++) {
        double binBottom = m_pDB->m_coreRgn.bottom + yOff * m_gridDensityHeight;
        double binTop = binBottom + m_gridDensityHeight;
        if (binBottom >= top)
          break;

        m_gridDensitySpaceNet[xOff][yOff] -=
            getOverlap(left, right, binLeft, binRight) *
            getOverlap(bottom, top, binBottom, binTop) * occupyRatio;
      }
    }

  } // each module

  for (unsigned int i = 0; i < m_gridDensitySpaceNet.size(); i++)
    for (unsigned int j = 0; j < m_gridDensitySpaceNet[i].size(); j++) {
      if (m_gridDensitySpaceNet[i][j] < 1e-5) {
        m_gridDensitySpaceNet[i][j] = 0.0;
      }
    }
}

void MyNLP::SmoothBasePotentialNet() {

  // Gaussian smoothing
  GaussianSmooth smooth;
  int r = m_smoothR;
  smooth.Gaussian2D(r, 6 * r + 1);
  smooth.Smooth(m_basePotentialNet);

  static vector<vector<double>> moreSmooth;

  // Save CPU time. Compute only at the first time.
  if (moreSmooth.size() != m_basePotentialNet.size()) {
    moreSmooth = m_basePotentialNet;
    r = m_smoothR * 6;
    int kernel_size = 5 * r;
    if (kernel_size % 2 == 0)
      kernel_size++;
    smooth.Gaussian2D(r, kernel_size);
    smooth.Smooth(moreSmooth);
  }

  // Merge basePotential and moreSmooth
  double binArea = m_potentialGridWidth * m_potentialGridHeight;
  double halfBinArea = binArea / 2;
  double scale = 3;
  for (unsigned int i = 0; i < moreSmooth.size(); i++)
    for (unsigned int j = 0; j < moreSmooth[i].size(); j++) {
      double free = binArea - m_basePotentialNet[i][j];
      if (free < 1e-4 &&
          moreSmooth[i][j] > halfBinArea) // no space or high enough
      {
        m_basePotentialNet[i][j] += (moreSmooth[i][j] - halfBinArea) * scale;
      }
    }
}

void MyNLP::UpdateExpBinPotentialNet() {
  // double totalFree = 0;
  // int zeroSpaceBin = 0;
  m_expBinPotentialNet.resize(m_basePotentialNet.size());
  for (unsigned int i = 0; i < m_basePotentialNet.size(); i++) {
    m_expBinPotentialNet[i].resize(m_basePotentialNet[i].size());
    for (unsigned int j = 0; j < m_basePotentialNet[i].size(); j++) {
      double base = m_basePotentialNet[i][j];
      double overlapX =
          getOverlap(GetXGrid(i) - m_potentialGridWidth * 0.5,
                     GetXGrid(i) + m_potentialGridWidth * 0.5,
                     m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.right);
      double overlapY =
          getOverlap(GetYGrid(i) - m_potentialGridWidth * 0.5,
                     GetYGrid(i) + m_potentialGridHeight * 0.5,
                     m_pDB->m_coreRgn.bottom, m_pDB->m_coreRgn.top);
      double realBinArea = overlapX * overlapY;
      double free = realBinArea - base;

      if (free > 1e-4) {
        m_expBinPotentialNet[i][j] = free;
      } else {
        m_expBinPotentialNet[i][j] = 0.0;
      }
    }
  }
}

void MyNLP::UpdatePotentialGridBaseNet() {
  // double binArea = m_potentialGridWidth * m_potentialGridHeight;
  // m_binFreeSpaceNet.resize( m_basePotentialNet.size() );
  for (unsigned int i = 0; i < m_basePotentialNet.size(); i++) {
    fill(m_basePotentialNet[i].begin(), m_basePotentialNet[i].end(), 0.0);
    // m_binFreeSpaceNet[i].resize( m_basePotentialNet[i].size() );
    // fill( m_binFreeSpaceNet[i].begin(), m_binFreeSpaceNet[i].end(), binArea
    // );
  }

  for (int i = 0; i < (int)m_pDB->m_modules.size(); i++) {
    // for each cell. cell ci coordinate is ( x[i*2], x[i*2+1] )

    if (m_pDB->m_modules[i].m_isFixed == false)
      continue;

    if (m_pDB->BlockOutCore(i))
      continue; // pads?

    double cellX = x[i * 2];
    double cellY = x[i * 2 + 1];
    double width = m_pDB->m_modules[i].GetWidth(z[i] - 0.5);
    double height = m_pDB->m_modules[i].GetHeight(z[i] - 0.5);

    // exact block range
    double left = cellX - width * 0.5;
    double bottom = cellY - height * 0.5;
    double right = cellX + (cellX - left);
    double top = cellY + (cellY - bottom);

    if (left < m_pDB->m_coreRgn.left)
      left = m_pDB->m_coreRgn.left;
    if (right > m_pDB->m_coreRgn.right)
      right = m_pDB->m_coreRgn.right;
    if (bottom < m_pDB->m_coreRgn.bottom)
      bottom = m_pDB->m_coreRgn.bottom;
    if (top > m_pDB->m_coreRgn.top)
      top = m_pDB->m_coreRgn.top;

    if (width == 0 || height == 0)
      continue;

    if (height <= m_dMacroOccupy * m_pDB->m_rowHeight)
      continue;

    double occupyRatio =
        ((double)(((int)(height / m_pDB->m_rowHeight / m_dMacroOccupy)) - 1)) /
        2.0 / m_dLevelNum;
    if (occupyRatio > 1)
      occupyRatio = 1;
    if (occupyRatio < 0)
      occupyRatio = 0;

    int gx, gy;
    GetClosestGrid(left, bottom, gx, gy);
    int gxx, gyy;
    double xx, yy;

    // Exact density for the base potential"
    for (gxx = gx, xx = GetXGrid(gx);
         xx <= right && gxx < (int)m_basePotentialNet.size();
         gxx++, xx += m_potentialGridWidth) {
      for (gyy = gy, yy = GetYGrid(gy);
           yy <= top && gyy < (int)m_basePotentialNet[gxx].size();
           gyy++, yy += m_potentialGridHeight) {
        m_basePotentialNet[gxx][gyy] +=
            getOverlap(left, right, xx, xx + m_potentialGridWidth) *
            getOverlap(bottom, top, yy, yy + m_potentialGridHeight) *
            occupyRatio;

        // m_binFreeSpaceNet[gxx][gyy] -=
        // getOverlap( left, right, xx, xx+m_potentialGridWidth ) *
        // getOverlap( bottom, top, yy, yy+m_potentialGridHeight ) *
        // occupyRatio;
      }
    }

  } // for each cell
}

//@Brian 2007-07-23

void MyNLP::PlotBinForceStrength() {
  static int count = 1;

  vector<vector<double>> binWireForce;
  vector<vector<double>> binSpreadingForce;
  vector<vector<double>> ratio;

  int mergeCount = 5;

  binWireForce.resize(m_gridPotential.size() / mergeCount);
  binSpreadingForce.resize(m_gridPotential.size() / mergeCount);
  ratio.resize(m_gridPotential.size() / mergeCount);
  for (unsigned int i = 0; i < binWireForce.size(); i++) {
    binWireForce[i].resize(m_gridPotential.size() / mergeCount, 0);
    binSpreadingForce[i].resize(m_gridPotential.size() / mergeCount, 0);
    ratio[i].resize(m_gridPotential.size() / mergeCount, 0);
  }

  for (unsigned int i = 0; i < m_pDB->m_modules.size(); i++) {
    if (m_pDB->m_modules[i].m_isFixed)
      continue;

    double cellX = x[2 * i];
    double cellY = x[2 * i + 1];
    double cellZ = z[i];
    int gx, gy, gz;
    GetClosestGrid(cellX, cellY, cellZ, gx, gy, gz);

    gx /= mergeCount;
    gy /= mergeCount;
    gz /= mergeCount;

    if (gx >= (int)binWireForce.size())
      gx = (int)binWireForce.size() - 1;
    if (gy >= (int)binWireForce.size())
      gy = (int)binWireForce.size() - 1;
    if (gz >= (int)binWireForce.size())
      gz = (int)binWireForce.size() - 1;

    if (gx >= (int)binWireForce.size() || gy >= (int)binWireForce.size() ||
        gz >= (int)binWireForce.size()) {
      printf("(%d %d %d)   (%d)\n", gx, gy, gz, binWireForce.size());
    }
    assert(gx >= 0);
    assert(gy >= 0);
    assert(gz >= 0);
    assert(gx < (int)binWireForce.size());
    assert(gy < (int)binWireForce.size());
    assert(gz < (int)binWireForce.size());
    binWireForce[gx][gy] += fabs(grad_wire[2 * i]) + fabs(grad_wire[2 * i + 1]);
    binSpreadingForce[gx][gy] +=
        fabs(grad_potential[2 * i]) + fabs(grad_potential[2 * i + 1]);
  }

  for (unsigned int i = 0; i < ratio.size(); i++)
    for (unsigned int j = 0; j < ratio[i].size(); j++) {
      ratio[i][j] = binSpreadingForce[i][j] / binWireForce[i][j];
    }

  char fn[255];
  sprintf(fn, "wire%d", count);
  CMatrixPlotter::OutputGnuplotFigure(binWireForce, fn);
  sprintf(fn, "spreading%d", count);
  CMatrixPlotter::OutputGnuplotFigure(binSpreadingForce, fn);
  sprintf(fn, "ratio%d", count);
  CMatrixPlotter::OutputGnuplotFigure(ratio, fn);
  count++;
}

bool MyNLP::InitObjWeights(double wWire) {
  int n;
  if (m_bMoveZ)
    n = 3 * m_pDB->m_modules.size();
  else
    n = 2 * m_pDB->m_modules.size();

  // 2006-02-22 find weights

  _weightWire = _weightDensity = 1.0; // init values to call eval_grad_f

  if(param.bUseEDensity){
    updateDensityForceBin();
    densityGradSum_ = 0.0;
  }
  ComputeBinGrad();
  Parallel(eval_grad_f_thread, m_pDB->m_modules.size());
  double totalWireGradient = 0;
  double totalPotentialGradient = 0;
  double totalCongGradient = 0.0;
  double totalViaGradient = 0;
  double totalPotentialZGradient = 0;

  if (!AdjustForce(n, x, grad_wire, grad_potential, grad_potentialZ)) // truncation
  {
    cout << "\n !AdjustForce()\n";
    return false; // bad values in grad_wire or grad_potential
  }
  
  int size = n / 2;
  if (m_bMoveZ)
    size = n / 3;
  else
    size = n / 2;
  for (int i = 0; i < size; i++) {
    // x direction
    if (isNaN(grad_wire[2 * i])) {
      cout << "\n In x direction: grad_wire[" << 2 * i << "] isNaN.\n";
      return false;
    }
    if (fabs(grad_wire[2 * i]) > DBL_MAX * 0.95) {
      cout << "\n In x direction: grad_wire[" << 2 * i << "] inf.\n";
      return false;
    }
    assert(fabs(grad_wire[2 * i]) < DBL_MAX * 0.95);
    assert(!isNaN(grad_wire[2 * i]));
    totalWireGradient += fabs(grad_wire[2 * i]);
    totalPotentialGradient += fabs(grad_potential[2 * i]);

    if (param.bCongObj)
      totalCongGradient += fabs(grad_congestion[2 * i]);

    // y direction
    if (isNaN(grad_wire[2 * i + 1])) {
      cout << "\n In y direction: grad_wire[" << 2 * i + 1 << "] isNaN.\n";
      return false;
    }
    if (fabs(grad_wire[2 * i + 1]) > DBL_MAX * 0.95) {
      cout << "\n In y direction: grad_wire[" << 2 * i + 1 << "] inf.\n";
      return false;
    }
    assert(fabs(grad_wire[2 * i + 1]) < DBL_MAX * 0.95);
    assert(!isNaN(grad_wire[2 * i + 1]));
    totalWireGradient += fabs(grad_wire[2 * i + 1]);
    totalPotentialGradient += fabs(grad_potential[2 * i + 1]);

    if (param.bCongObj)
      totalCongGradient += fabs(grad_congestion[2 * i + 1]);

    // z direction
    if (m_bMoveZ) {
      if (isNaN(grad_via[i])) {
        cout << "\n In m_bMoveZ: grad_via[" << i << "] isNaN.\n";
        return false;
      }
      if (fabs(grad_via[i]) > DBL_MAX * 0.95) {
        cout << "\n In m_bMoveZ: grad_via[" << i << "] inf.\n";
        return false;
      }
      assert(fabs(grad_via[i]) < DBL_MAX * 0.95);
      assert(!isNaN(grad_via[i]));
      // totalWireGradient += fabs(grad_via[i]);
      // totalPotentialGradient += fabs(grad_potentialZ[i]);
      totalViaGradient += fabs(grad_via[i]);
      totalPotentialZGradient += fabs(grad_potentialZ[i]);
    }
  }

  if (fabs(totalWireGradient) > DBL_MAX * 0.95) {
    cout << "\n totalWireGradient inf.\n";
    return false;
  }
  if (fabs(totalPotentialGradient) > DBL_MAX * 0.95) {
    cout << "\n totalPotentialGradient inf.\n";
    return false;
  }
  // Brian 2007-04-30
  if (fabs(totalCongGradient) > DBL_MAX * 0.95) {
    cout << "\n totalCongGradient inf.\n";
    return false;
  }
  //@Brian 2007-04-30
  //(kaie) 2009-10-19
  if (m_bMoveZ) {
    if (fabs(totalViaGradient) > DBL_MAX * 0.95) {
      cout << "\n totalViaGradient inf.\n";
      return false;
    }
    if (fabs(totalPotentialZGradient) > DBL_MAX * 0.95) {
      cout << "\n totalPotentialZGradient inf.\n";
      return false;
    }
  }
  //@(kaie) 2009-10-19
  // Fix density weight, change wire weight
  if(param.bUseEDensity) 
    _weightDensity = param.initDensityPenalty / 50; // TODO: fix this bug...
  else 
    _weightDensity = totalWireGradient / totalPotentialGradient ;
  if(param.bShow)
    cout << "_weightDensity = " << totalWireGradient << " / " << totalPotentialGradient << " = " << _weightDensity << "\n";
  //cout << "_weightDensity = " << _weightDensity << " = " << totalWireGradient << " / " << totalPotentialGradient << "\n";
  //_weightDensity = totalViaGradient / totalPotentialGradient;
  _weightWire = wWire;
  if (m_bMoveZ) {
    double _weightTSV = (double)m_potentialGridSize;
    _weightTSV = param.dWeightTSV; /// frank: temp value
    // gArg.GetDouble("TSV", &_weightTSV);
    // m_weightTSV = _weightTSV * wWire;
    // m_weightTSV = _weightTSV;
    m_weightTSV = _weightTSV * (totalWireGradient /*-totalViaGradient*/) /
                  totalViaGradient;
    // m_weightTSV = 0.0001;
    // m_weightTSV = wWire * _weightTSV;
    // m_weightTSV = (m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left) /
    // sqrt(m_pDB->m_totalLayer);
    printf("weight TSV = %lf\n", m_weightTSV);
  }

  gArg.GetDouble("weightWire", &_weightWire);

  if (param.bShow) {
    printf( " ------------------------ INIT INFO ------------------------\n");
    printf( " Force strength: %.0f (wire)  %.0f (spreading)\n", totalWireGradient, totalPotentialGradient);
    printf( " _weightDensity = %f\n", _weightDensity);
    // printf( " INIT: WL= %.0f, gradWL= %.0f\n", totalWL, totalWireGradient ); 
    // printf( " INIT: DensityPenalty= %.0f, gradPenalty=%.0f\n", density, totalPotentialGradient );
    // printf( " INIT: m_weightTSV= %.0f, gradPenalty=%.0f\n", m_weightTSV, totalPotentialGradient );
    printf( " -----------------------------------------------------------\n");
  }
  assert(wWire == 0 || _weightWire > 0);

  for (unsigned int k = 0; k < m_weightDensity.size(); k++)
    for (unsigned int i = 0; i < m_weightDensity[k].size(); i++)
      for (unsigned int j = 0; j < m_weightDensity[k][i].size(); j++)
        m_weightDensity[k][i][j] = _weightDensity;
  return true;
}

void MyNLP::UpdateObjWeights() {

  //_weightWire /= m_weightIncreaseFactor;

  // Brian 2007-07-23
  if (param.bCongObj) {
    _weightWire /= m_weightIncreaseFactor;
    m_dWeightCong /= param.dCongDivRatio;
  }
  //@Brian 2007-07-23
  else {
    if(param.bUseEDensity){
      double cof_min=0.95, cof_max=1.05, cof=1.0;
      double p = (curWL_ - lastWL_) / param.denRefWL;
      if(p < 0){
        cof = cof_max;
      } else {
        cof = max(cof_min, pow(cof_max, 1-p));
      }
      _weightDensity *= cof;
    }
    else{
      _weightDensity *= m_weightIncreaseFactor;
    }
    if(param.bShow)
      cout << "_weightDensity = " << _weightDensity << "\n";
    // if(m_bMoveZ)
    //    m_weightTSV *= m_weightIncreaseFactor * 0.90;

    for (unsigned int k = 0; k < m_weightDensity.size(); k++) {
      for (unsigned int i = 0; i < m_weightDensity[k].size(); i++) {
        for (unsigned int j = 0; j < m_weightDensity[k][i].size(); j++) {

          if (gArg.CheckExist("ada") && m_topLevel) {

            double util = m_gridPotential[k][i][j] / m_expBinPotential[k][i][j];

            // util > 1.0    overflow
            // util < 1.0    underflow

            util = max(1.0, util);
            util = min(2.0, util);

            util = util * util;

            // util = ( util - 1.0 ) * 0.5 + 1.5;

            m_weightDensity[k][i][j] = m_weightDensity[k][i][j] * (util);
          } else
            m_weightDensity[k][i][j] =
                m_weightDensity[k][i][j] * m_weightIncreaseFactor;
        }
      }
    }
  }
}


// eDensity Functions
static int 
fastModulo(const int input, const int ceil) {
  return input >= ceil? input % ceil : input;
}

void
MyNLP::InitFillerPlace(){
  nModule_ = m_pDB->m_nModules;
  nFiller_ = 0;
  vUsedArea_.resize(param.nlayer, 0.0);
  vCellN_.resize(param.nlayer, 0);
  vTargetDensity_.resize(param.nlayer, 0);
  vTotalFillerArea_.resize(param.nlayer, 0);
  vMovableArea_.resize(param.nlayer, 0);
  vector<double> vSumW(param.nlayer, 0);
  vector<double> vSumH(param.nlayer, 0);
  for(int i=0;i<m_pDB->m_nModules;++i){
    if(m_pDB->m_modules[i].m_isVia || m_pDB->m_modules[i].m_isFixed) continue;
    int layer = m_pDB->m_modules[i].m_z;
    //cout << "cell["<<i<<"]: layer="<<layer<<", area="<<m_pDB->m_modules[i].m_widths[layer] * m_pDB->m_modules[i].m_heights[layer]<<"\n";
    vUsedArea_[layer] += m_pDB->m_modules[i].m_widths[layer] * m_pDB->m_modules[i].m_heights[layer];
    vSumW[layer] += m_pDB->m_modules[i].m_widths[layer];
    vSumH[layer] += m_pDB->m_modules[i].m_heights[layer];
    vCellN_[layer]++;
  }

  coreArea_ = (m_pDB->m_coreRgn.right-m_pDB->m_coreRgn.left) * (m_pDB->m_coreRgn.top-m_pDB->m_coreRgn.bottom);
  cout << "-------------------- ePlace Filler Info -------------------\n";
  for(int k=0;k<param.nlayer;++k){
    // get fillerCnt
    double usedArea = vUsedArea_[k];
    fillerW_ = max(1, vSumW[k] / vCellN_[k]);
    fillerH_ = max(1, vSumH[k] / vCellN_[k]);
    vTargetDensity_[k] = vUsedArea_[k] / (coreArea_ * m_pDB->m_maxUtils[k]) + 0.01;
    vMovableArea_[k] = coreArea_ * vTargetDensity_[k]; //m_pDB->m_maxUtils[k];
    vTotalFillerArea_[k] = vMovableArea_[k] - vUsedArea_[k];
    int fillerCnt = vTotalFillerArea_[k] / (fillerW_*fillerH_);
    nFiller_ += fillerCnt;
    if(1){
      cout << "Layer " << k << ":\n";
      cout << "  coreArea = " << coreArea_ << "\n";
      cout << "  usedArea = " << usedArea << "\n";
      cout << "  vMovableArea = " << vMovableArea_[k] << "\n";
      cout << "  filler size = " << fillerW_ << " * " << fillerH_ << "\n";
      cout << "  TargetDensity_ = " << vTargetDensity_[k] << "\n";
      cout << "  TotalFillerArea_ = " << vTotalFillerArea_[k] << "\n"; 
      cout << "  fillerCnt = " << fillerCnt << "\n";
    }

    for(int i=0; i<fillerCnt; i++) {
      // instability problem between g++ and clang++!
      int randX = rand() % (int)(m_pDB->m_coreRgn.right-m_pDB->m_coreRgn.left-fillerW_);
      int randY = rand() % (int)(m_pDB->m_coreRgn.top-m_pDB->m_coreRgn.bottom-fillerH_);
      // place filler cells on random coordi and
      // set size as avgDx and avgDy
      int moduleId = m_pDB->m_modules.size();
      m_pDB->AddModule( "Filler"+to_string(i), fillerW_, fillerH_, false );
      Module& curModule = m_pDB->m_modules.back();
      curModule.m_widths.resize(param.nlayer,fillerW_);
      curModule.m_heights.resize(param.nlayer,fillerH_);
      m_pDB->SetModuleLocation( moduleId, randX, randY);
      m_pDB->SetModuleOrientation( moduleId, 0 ); // orientInt('N')=0
      m_pDB->SetModuleLayerAssign( moduleId, k);
      curModule.m_isFiller = true;
      curModule.m_cz = curModule.m_z + 0.5;
    }
  }
  cout << "----------------------------------------------------------\n";
  m_pDB->m_nModules = m_pDB->m_modules.size();
}
void
MyNLP::InitEDensity(){
  // find bin num
  int64_t totalBinArea = coreArea_;
  double averagePlaceInstArea = (vUsedArea_[0]+vUsedArea_[1]) / nModule_;
  int64_t idealBinArea = std::round((averagePlaceInstArea) / ((vTargetDensity_[0]+vTargetDensity_[1])/2));
  int idealBinCnt = max(4, totalBinArea / idealBinArea); // the smallest we allow is 2x2 bins
  int foundBinCnt = 2;
  // find binCnt: 2, 4, 8, 16, 32, 64, ...
  // s.t. binCnt^2 <= idealBinCnt <= (binCnt*2)^2.
  for(foundBinCnt = 2; foundBinCnt <= 1024; foundBinCnt *= 2) {
    if( foundBinCnt * foundBinCnt <= idealBinCnt 
        && 4 * foundBinCnt * foundBinCnt > idealBinCnt ) {
      break;
    }
  }
  if(foundBinCnt<1024)
    foundBinCnt *= 2;
  // initialize bin grid structure
  // binNumX_ = m_potentialGridSize + m_potentialGridPadding + 1;
  // binNumY_ = m_potentialGridSize + m_potentialGridPadding + 1;
  binNumX_ = foundBinCnt;
  binNumY_ = foundBinCnt;
  binNumZ_ = param.nlayer;
  binSizeX_ = (m_pDB->m_coreRgn.right-m_pDB->m_coreRgn.left) / binNumX_;
  binSizeY_ = (m_pDB->m_coreRgn.top-m_pDB->m_coreRgn.bottom) / binNumY_;
  binSizeZ_ = 1;
  if(1){
    cout << "-------------------- eDensity Init Info -------------------\n";
    cout << "initialize bin grid structure:\n";
    cout << "  binNumX_ = " << binNumX_ << "\n";
    cout << "  binNumY_ = " << binNumY_ << "\n";
    cout << "  binNumZ_ = " << binNumZ_ << "\n";
    cout << "  binSizeX_ = " << binSizeX_ << "\n";
    cout << "  binSizeY_ = " << binSizeY_ << "\n";
    cout << "  binSizeZ_ = " << binSizeZ_ << "\n";
    cout << "-----------------------------------------------------------\n";
  }
  // initialize bins_ vector
  int binNum = binNumZ_*binNumX_*binNumY_;
  bins_.resize(binNumZ_, vector<vector<Bin*> >(binNumX_, vector<Bin*>(binNumY_)));
  for(int k=0;k<binNumZ_;++k){
    for(int i=0;i<binNumX_;++i){
      for(int j=0;j<binNumY_;++j){
        Bin* bin = new Bin(i, j, k, i*binSizeX_, j*binSizeY_, k*binSizeZ_, i*binSizeX_+binSizeX_, j*binSizeY_+binSizeY_, k*binSizeZ_+binSizeZ_, vTargetDensity_[k]);
        bins_[k][i][j] = bin;
      }
    }
  }
  // updateBinsNonPlaceVolumn, only initialized once
  for(int k=0;k<binNumZ_;++k){
    for(int i=0;i<binNumX_;++i){
      for(int j=0;j<binNumY_;++j){
        bins_[k][i][j]->nonPlaceVolumn_ = 0;
        bins_[k][i][j]->nonPlaceArea_ = 0;
      }
    }
  }

  // initialize fft structrue based on bins
  fft_.resize(param.nlayer);
  for(int k=0;k<param.nlayer;++k){
    unique_ptr<FFT> fft(new FFT(binNumX_, binNumY_, binSizeX_, binSizeY_));
    fft_[k] = std::move(fft);
  }

  // update densitySize and densityScale in each gCell
  cellDensitySizes_.resize(2*m_pDB->m_nModules);
  cellDensityScales_.resize(m_pDB->m_nModules);
  for(int i=0;i<m_pDB->m_nModules;++i){
    float scaleX = 0, scaleY = 0;
    float densitySizeX = 0, densitySizeY = 0;
    int layer = m_pDB->m_modules[i].m_z;
    if( m_pDB->m_modules[i].m_widths[layer] < REPLACE_SQRT2 * binSizeX_ ) {
      scaleX = static_cast<float>(m_pDB->m_modules[i].m_widths[layer]) / static_cast<float>( REPLACE_SQRT2 * binSizeX_);
      densitySizeX = REPLACE_SQRT2 * static_cast<float>(binSizeX_);
    } else {
      scaleX = 1.0;
      densitySizeX = m_pDB->m_modules[i].m_widths[layer];
    }

    if( m_pDB->m_modules[i].m_heights[layer] < REPLACE_SQRT2 * binSizeY_ ) {
      scaleY = static_cast<float>(m_pDB->m_modules[i].m_heights[layer]) / static_cast<float>( REPLACE_SQRT2 * binSizeY_);
      densitySizeY = REPLACE_SQRT2 * static_cast<float>(binSizeY_);
    } else {
      scaleY = 1.0;
      densitySizeY = m_pDB->m_modules[i].m_heights[layer];
    }

    cellDensitySizes_[2*i] = densitySizeX;
    cellDensitySizes_[2*i+1] = densitySizeY;
    cellDensityScales_[i] = scaleX * scaleY;
  }
}

void
MyNLP::FillerSpreading(double wWire, double target_density){
  cout << "Start FillerSpreading ...............\n";
  for(int i=0;i<nModule_;++i){
    m_pDB->m_modules[i].m_isFixed = true;
  }
  
  Parallel(BoundXThread, m_pDB->m_modules.size());
  if (m_bMoveZ) // kaie z-direction move
    Parallel(BoundZThread, m_pDB->m_modules.size());

  m_currentStep = param.step;
  if (m_targetUtil > 1.0)
    m_targetUtil = 1.0;

  int n;
  if (m_bMoveZ)
    n = 3 * m_pDB->m_modules.size(); // (kaie) 2009-09-12 add z direction
  else
    n = 2 * m_pDB->m_modules.size();

  double designUtil = m_pDB->m_totalMovableModuleVolumn / m_pDB->m_totalFreeSpace;

  double baseUtil = 0.05; // experience value preventing over-spreading
  m_targetUtil = min(1.0, m_targetUtil + baseUtil);

  double lowestUtil = min(1.0, designUtil + baseUtil);
  if (m_targetUtil > 0) // has user-defined target utilization
  {
    if (m_targetUtil < lowestUtil) {
      if (param.bShow)
        printf("NOTE: Target utilization (%f) is too low\n", m_targetUtil);
      if (gArg.CheckExist("forceLowUtil") == false)
        m_targetUtil = lowestUtil;
    }
  } else // no given utilization
  {
    printf("No given target utilization.\n"); //  Distribute blocks evenly
    m_targetUtil = lowestUtil;
  }
  
  if (param.bShow) {
    printf("INFO: Design utilization: %f\n", designUtil);
    printf("DBIN: Target utilization: %f\n", m_targetUtil);
  }

  fill(grad_f.begin(), grad_f.end(), 0.0);
  fill(last_grad_f.begin(), last_grad_f.end(), 0.0);

  CreatePotentialGrid(); // Create potential grid according to
                         // "m_potentialGridSize"
  int densityGridSize =
      m_potentialGridSize / 4; // Use larger grid for density computing
  CreateDensityGrid(densityGridSize);
  UpdateDensityGridSpace(n, x, z);
  UpdatePotentialGridBase(x, z);
  // SmoothBasePotential3D();
  UpdateExpBinPotential(m_targetUtil, true);
  assert(m_targetUtil > 0);
  // wirelength
  Parallel(UpdateExpValueForEachCellThread, m_pDB->m_modules.size());
  Parallel(UpdateExpValueForEachPinThread, m_pDB->m_pins.size());
  Parallel(UpdateNetsSumExpThread, (int)m_pDB->m_nets.size());
  wirelength = GetLogSumExpWL(x, _expX, _alpha, this); // WA
  if (m_bMoveZ)
    GetLogSumExpVia(z, _expZ, _alpha, this);
  // density
  Parallel(ComputeNewPotentialGridThread, m_pDB->m_modules.size());
  UpdatePotentialGrid(z);
  UpdateDensityGrid(n, x, z);
  density = GetDensityPanelty();
  if(param.bUseEDensity){
    updateDensityForceBin();
  }

  ///////////// [1] - Init /////////////
  m_dWeightCong = 1.0;
  if (!InitObjWeights(wWire)) {
    printf("InitObjWeight OVERFLOW!\n");
  }
  int maxIte = m_maxIte;
  bool newDir = true;
  double obj_value;
  eval_f(n, x, _expX, true, obj_value);
  if(param.bUseEDensity){
    updateDensityForceBin();
  }
  ComputeBinGrad();
  Parallel(eval_grad_f_thread, m_pDB->m_modules.size());
  UpdateDensityGrid(n, x, z);
  double maxDen = GetMaxDensity();
  double lastMaxDen = maxDen;
  double totalOverDen = GetTotalOverDensity();
  double totalOverDenLB = GetTotalOverDensityLB();
  double totalOverPotential = GetTotalOverPotential();
  double totalOverDenNet = GetTotalOverDensityNet();
  double totalOverPotentialNet = GetTotalOverPotentialNet();
  double maxDenNet = GetMaxDensityNet();
  double lastTotalOverNet = 0;
  double lastTotalOverPotentialNet = DBL_MAX;
  double lastMaxDenNet = maxDenNet;
  double overNet = totalOverDenNet;

  if (obj_value > DBL_MAX * 0.5) {
    printf("Objective value OVERFLOW!\n");
  }
  fflush(stdout);

  double lastTotalOver = 0;
  double lastTotalOverPotential = DBL_MAX;
  double over = totalOverDen;
  int totalIte = 0;
  double bestLegalWL = DBL_MAX;
  int lookAheadLegalCount = 0;
  double totalLegalTime = 0.0;
  bool startDecreasing = false;
  int checkStep = 5;
  int outStep = 50;
  if (param.bShow == false) outStep = INT_MAX;
  int forceBreakLoopCount = INT_MAX;
  if (m_topLevel == false) forceBreakLoopCount = INT_MAX;

  // Legalization related configurations
  // int tetrisDir = 0;	// 0: both   1: left   2: right
  int LALnoGoodCount = 0;
  int maxNoGoodCount = 2;

  vector<Module> bestGPresult; // for LAL (Look Ahead Legal)

  static double lastHPWL = 0; // test
  static double lastTSV = 0;  // kaie
  ////////////////////////////////////////////////////////////

  // [2] - Iteration Optimization
  newDir = true;
  bool bUpdateWeight = true;
  int global_iter = 0;
  for (int ite = 0; ite < maxIte; ite++) { //////////////////////////////////////////////////////////////////////
    m_ite++;
    int innerIte = 0;
    double old_obj = DBL_MAX;
    double last_obj_value = DBL_MAX;

    m_currentStep = param.step;
    if (bUpdateWeight == false) newDir = false;
    else newDir = true;
    bUpdateWeight = true;

    // [2.1] - inner loop, minimize "f" 
    double lastDensityCost = density; // for startDecreasing determination
    while (true) {                     
      innerIte++;
      global_iter++;
      swap(last_grad_f, grad_f); // save for computing the congujate gradient direction
      swap(last_walk_direction, walk_direction);
      // [2.1.1] - Compute BinGrad and Adjust Force
      if(param.bUseEDensity){
        updateDensityForceBin();
      }
      ComputeBinGrad();
      Parallel(eval_grad_f_thread, m_pDB->m_modules.size());

      if (!AdjustForce(n, x, grad_f)) {
        printf("AdjustForce, NaN or Inf\n");
      }

      if (innerIte % checkStep == 0) {
        if (m_useEvalF) {
          old_obj = last_obj_value; // backup the old value
          if (m_bMoveZ) 
            LayerAssignment();
          Parallel(UpdateBlockPositionThread, m_pDB->m_modules.size());
          // m_pDB->CalcHPWL();
          eval_f(n, x, _expX, true, obj_value);
          last_obj_value = obj_value;
        } else // Observe the wirelength change
        {
          if (m_bMoveZ)
            LayerAssignment();
          Parallel(UpdateBlockPositionThread, m_pDB->m_modules.size());
          m_pDB->CalcHPWL();
          if (m_pDB->GetHPWL() < lastHPWL) {
            lastHPWL = 0;
            break;
          }
          lastHPWL = m_pDB->GetHPWL();
        }
      }

      // [2.1.3] - Compute beta and Gradient Direction
      if (newDir == true) {
        // gradient direction
        newDir = false;
        for (int i = 0; i < n; i++) {
          grad_f[i] = -grad_f[i];
          walk_direction[i] = grad_f[i];
        }
      } else {
        // gradient direction
        if (FindBeta(n, grad_f, last_grad_f, m_beta) == false) {
          printf("FindBeta OVERFLOW!\n");
        }
        Parallel(UpdateGradThread, n);
      }
      LineSearch(n, x, walk_direction, m_stepSize); // Calculate a_k (step size)
      
      xMax.resize(m_pDB->m_nets.size(), 0);
      yMax.resize(m_pDB->m_nets.size(), 0);
      zMax.resize(m_pDB->m_nets.size(), 0);
      Parallel(UpdateXThread, m_pDB->m_modules.size()); // Update X. (x_{k+1} = x_{k} + \alpha_k * d_k)
      Parallel(BoundXThread, m_pDB->m_modules.size());
      if (m_bMoveZ)
        Parallel(BoundZThread, m_pDB->m_modules.size());
      if(param.bPlot && global_iter%5==0){
        char fileName[128];
        sprintf(fileName, "gp-%04d", global_iter);
        //plotPL(string(move(fileName)), global_iter);
        draw_field(global_iter, "plot-filler/");
      } 
      

      // [2.1.4] - Update WL & Density Force
      // New block positions must be ready
      // 1. UpdateExpValueForEachCellThread    (wire force)
      // 2. UpdateExpValueForEachPinThread     (wire force)
      // 3. ComputeNewPotentialGridThread      (spreading force)
      Parallel(UpdateNLPDataThread, m_pDB->m_modules.size(), m_pDB->m_pins.size(), m_pDB->m_modules.size());
      // New EXP values must be ready
      double time_used = seconds();
      Parallel(UpdateNetsSumExpThread, (int)m_pDB->m_nets.size());
      time_wire_force += seconds() - time_used;
      if(param.bUseEDensity){
        if(m_bMoveZ)
          updateBinsGCellDensityVolumn();
        else
          updateBinsGCellDensityArea();
      }
      time_used = seconds();
      UpdatePotentialGrid(z);
      time_spreading_force += seconds() - time_used; 

      if (innerIte == forceBreakLoopCount) {
        printf("b");
        bUpdateWeight = false;
        break;
      }
    } // inner loop ///////////////////////////////////////////////////////////////////////

    // [2.2] - check result for this iter //////////
    if (param.bShow) {
      printf("%d\n", innerIte);
      fflush(stdout);
    } else
      printf("\n");
    totalIte += innerIte;

    if(param.bUseEDensity){
      updateDensityForceBin();
    }
    UpdateDensityGrid(n, x, z);
    maxDen = GetMaxDensity();
    totalOverDen = GetTotalOverDensity();
    totalOverDenLB = GetTotalOverDensityLB();
    totalOverPotential = GetTotalOverPotential();

    if (m_bMoveZ)
      LayerAssignment();
    Parallel(UpdateBlockPositionThread, m_pDB->m_modules.size()); // update to placeDB

    if (obj_value > DBL_MAX * 0.5) {
      printf("Objective value OVERFLOW!\n");
    }

    fflush(stdout);

    bool spreadEnough = totalOverPotential < 1.3;
    bool increaseOverPotential = totalOverPotential > lastTotalOverPotential;
    bool increaseMaxDen = maxDen > lastMaxDen;
    bool enoughIteration = ite > 3;
    bool notEfficientOptimize = 0.5 * density * _weightDensity / obj_value * 100.0 > 95;
    // PrintPotentialGrid();
    if (enoughIteration && notEfficientOptimize) {
      printf("Failed to further optimize (enoughIteration && notEfficientOptimize)\n");
      break;
    }
    if (enoughIteration && increaseOverPotential && increaseMaxDen && spreadEnough) {
      printf("Cannot further reduce over potential! (skip LAL)\n");
      break;
    }
    if (startDecreasing && over < target_density) {
      printf("Meet constraint! (startDecreasing && over < target_density)\n");
      break;
    }
    if(ite >= 2){
      printf("Meet constraint! (ite >= 2)\n");
      break;
    }
    
    // [2.3] - Update Weights
    if (bUpdateWeight)
      UpdateObjWeights();

    lastTotalOverPotential = totalOverPotential;
    lastMaxDen = maxDen;
    lastTotalOverPotentialNet = totalOverPotentialNet;
    lastMaxDenNet = maxDenNet;

  } // outer loop /////////////////////////////////////////////////////////////////////////////////////////////////////
  cout << "FillerSpreading Finished !\n";
}

void MyNLP::updateDensityForceBin(){
  // copy density to utilize FFT
  for(int k=0;k<bins_.size();++k){
    for(int i=0;i<bins_[k].size();++i){
      for(int j=0;j<bins_[k][i].size();++j){
        Bin* bin = bins_[k][i][j];
        fft_[k]->updateDensity(bin->x_, bin->y_, bin->density_);
        // if(i%10==0 && j%10==0)
        //   cout << "bins_["<<k<<"]["<<i<<"]["<<j<<"]: ("<<bin->x_<<","<<bin->y_<<"), density="<<bin->density_<<"\n";
      }
    }
    // do FFT
    fft_[k]->doFFT();
  }

  // update electroPhi and electroForce
  // update sumPhi_ for nesterov loop
  sumPhi_.resize(param.nlayer, 0);
  for(int k=0;k<bins_.size();++k){
    for(int i=0;i<bins_[k].size();++i){
      for(int j=0;j<bins_[k][i].size();++j){
        Bin* bin = bins_[k][i][j];
        pair<float, float> eForcePair = fft_[k]->getElectroForce(bin->x_, bin->y_);
        bin->electroForceX_ = eForcePair.first;
        bin->electroForceY_ = eForcePair.second;

        float electroPhi = fft_[k]->getElectroPhi(bin->x_, bin->y_);
        bin->electroPhi_ = electroPhi;

        if(m_bMoveZ){
          sumPhi_[k] += electroPhi * 
              static_cast<float>(bin->nonPlaceVolumn_+ bin->instPlacedVolumn_ + bin->fillerVolumn_);  
        } else{
          sumPhi_[k] += electroPhi * 
              static_cast<float>(bin->nonPlaceArea_+ bin->instPlacedArea_ + bin->fillerArea_);  
        }
        
      }
    }
  }
}

void MyNLP::GetPotentialGrad_eDensity(const vector<double> &x, const vector<double> &z,
                             const int &i, double &gradX, double &gradY,
                             double &gradZ, MyNLP *pNLP) {
  double cellX = x[i * 2];
  double cellY = x[i * 2 + 1];
  double cellZ = z[i];

  double width = pNLP->m_pDB->m_modules[i].GetWidth(z[i] - 0.5);
  double height = pNLP->m_pDB->m_modules[i].GetHeight(z[i] - 0.5);
  double thickness = pNLP->m_pDB->m_modules[i].m_thickness;
  //// use square to model small std-cells
  if (height < pNLP->m_potentialGridHeight &&
      width < pNLP->m_potentialGridWidth &&
      thickness < pNLP->m_potentialGridThickness)
    width = height = thickness = 0;

  double left = cellX - width * 0.5 - pNLP->_potentialRX;
  double bottom = cellY - height * 0.5 - pNLP->_potentialRY;
  double back = cellZ - thickness * 0.5 - pNLP->_potentialRZ;
  double right = cellX + (cellX - left);
  double top = cellY + (cellY - bottom);
  double front = cellZ + (cellZ - back);
  ;

  if (left < pNLP->m_pDB->m_coreRgn.left - pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth)
    left = pNLP->m_pDB->m_coreRgn.left - pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
  if (right > pNLP->m_pDB->m_coreRgn.right + pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth)
    right = pNLP->m_pDB->m_coreRgn.right + pNLP->m_potentialGridPadding * pNLP->m_potentialGridWidth;
  if (bottom < pNLP->m_pDB->m_coreRgn.bottom - pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight)
    bottom = pNLP->m_pDB->m_coreRgn.bottom - pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
  if (top > pNLP->m_pDB->m_coreRgn.top + pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight)
    top = pNLP->m_pDB->m_coreRgn.top + pNLP->m_potentialGridPadding * pNLP->m_potentialGridHeight;
  if (back < pNLP->m_pDB->m_back - pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness)
    back = pNLP->m_pDB->m_back - pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness;
  if (front > pNLP->m_pDB->m_front + pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness)
    front = pNLP->m_pDB->m_front + pNLP->m_potentialGridPadding * pNLP->m_potentialGridThickness;

  // get overlaping bins
  pair<int, int> pairX = pNLP->getDensityMinMaxIdxX(left, right);
  pair<int, int> pairY = pNLP->getDensityMinMaxIdxY(bottom, top);
  pair<int, int> pairZ = pNLP->getDensityMinMaxIdxZ(back, front);

  gradX = 0.0;
  gradY = 0.0;
  gradZ = 0.0;
  for(int gz = pairZ.first; gz < pairZ.second; gz++) {
    for(int gx = pairX.first; gx < pairX.second; gx++) {
      for(int gy = pairY.first; gy < pairY.second; gy++) {
        if(gz>=pNLP->binNumZ_ || gx>=pNLP->binNumX_ || gy>=pNLP->binNumY_) continue;
        Bin* bin = pNLP->bins_[gz][gx][gy];
        //double overlapArea = pNLP->getOverlapDensityArea(bin, left, right, bottom, top) * pNLP->m_cellDensityScales[i];
        if(pNLP->m_bMoveZ){
          double overlapVolumn = pNLP->getOverlapDensityVolumn(bin, left, right, bottom, top, back, front) * pNLP->cellDensityScales_[i];
          gradX -= overlapVolumn * bin->electroForceX_;
          gradY -= overlapVolumn * bin->electroForceY_;
          // gradZ += overlapVolumn * bin->electroForceZ_;
        } else{
          if(gz != pNLP->m_pDB->m_modules[i].m_z) continue;
          double overlapArea = pNLP->getOverlapDensityArea(bin, left, right, bottom, top) * pNLP->cellDensityScales_[i];
          gradX -= overlapArea * bin->electroForceX_;
          gradY -= overlapArea * bin->electroForceY_;
        }
        pNLP->densityGradSum_ += gradX + gradY;
      }
    }
  } // for each grid

  if(pNLP->m_bMoveZ){
    // only get gradZ
    int gx, gy, gz;
    pNLP->GetClosestGrid(left, bottom, back, gx, gy, gz);
    int gxx, gyy, gzz;
    double xx, yy, zz;
    for (gzz = gz, zz = pNLP->GetZGrid(gz);
        zz <= front && gzz < (int)pNLP->m_gridPotential.size();
        gzz++, zz += pNLP->m_potentialGridThickness) {
      for (gxx = gx, xx = pNLP->GetXGrid(gx);
          xx <= right && gxx < (int)pNLP->m_gridPotential[gzz].size();
          gxx++, xx += pNLP->m_potentialGridWidth) {
        for (gyy = gy, yy = pNLP->GetYGrid(gy);
            yy <= top && gyy < (int)pNLP->m_gridPotential[gzz][gxx].size();
            gyy++, yy += pNLP->m_potentialGridHeight) {
          double gZ = 0;
          if (pNLP->m_bMoveZ) {
            gZ = GetPotential(cellX, xx, pNLP->_potentialRX, width) *
                GetPotential(cellY, yy, pNLP->_potentialRY, height) *
                GetGradPotential(cellZ, zz, pNLP->_potentialRZ, thickness) *
                (pNLP->m_gridPotential[gzz][gxx][gyy] -
                  pNLP->m_expBinPotential[gzz][gxx][gyy]);
          }

          if (m_skewDensityPenalty1 != 1.0) {
            if (pNLP->m_bMoveZ) {
              if (pNLP->m_gridPotential[gzz][gxx][gyy] <
                  pNLP->m_expBinPotential[gzz][gxx][gyy])
                gZ /= m_skewDensityPenalty2;
              else
                gZ *= m_skewDensityPenalty1;
            }
          }

          if (bMulti) {
            if (pNLP->m_bMoveZ)
              gradZ += gZ * m_weightDensity[gzz][gxx][gyy];
          } else {
            if (pNLP->m_bMoveZ){
              gradZ += gZ;
              pNLP->densityGradSum_ += gradX;
            }
          }
        }
      }
    } // for each grid
  }
  
}

pair<int, int> 
MyNLP::getDensityMinMaxIdxX(int left, int right){
  int lowerIdx = left/binSizeX_;
  int upperIdx = 
   ( fastModulo(right, binSizeX_) == 0)? 
   right / binSizeX_ : right / binSizeX_ + 1;
  return std::make_pair(lowerIdx, upperIdx);
}
pair<int, int> 
MyNLP::getDensityMinMaxIdxY(int bottom, int top){
  int lowerIdx = bottom/binSizeY_;
  int upperIdx = 
   ( fastModulo(top, binSizeY_) == 0)? 
   top / binSizeY_ : top / binSizeY_ + 1;
  return std::make_pair(lowerIdx, upperIdx);
}
pair<int, int> 
MyNLP::getDensityMinMaxIdxZ(double back, double front){
  //return std::make_pair(0, 2);
  int lowerIdx = back/binSizeZ_;
  int upperIdx = 
   ( fastModulo(front, binSizeZ_) == 0)? 
   front / binSizeZ_ : front / binSizeZ_ + 1;
  return std::make_pair(lowerIdx, upperIdx);
}
double 
MyNLP::getOverlapDensityArea(Bin* bin, int left, int right, int bottom, int top) {
  int rectLx = max(bin->lx_, left), 
      rectLy = max(bin->ly_, bottom),
      rectUx = min(bin->ux_, right), 
      rectUy = min(bin->uy_, top);
  if( rectLx >= rectUx || rectLy >= rectUy ) {
    return 0;
  }
  else {
    return static_cast<float>(rectUx - rectLx) 
      * static_cast<float>(rectUy - rectLy);
  }
}
double 
MyNLP::getOverlapDensityVolumn(Bin* bin, int left, int right, int bottom, int top, int back, int front) {
  int rectLx = max(bin->lx_, left), 
      rectLy = max(bin->ly_, bottom),
      rectUx = min(bin->ux_, right), 
      rectUy = min(bin->uy_, top);
  double  rectLz = (bin->lz_ > back)? bin->lz_ : back;
  double  rectUz = (bin->uz_ < back)? bin->uz_ : front;
  if( rectLx >= rectUx || rectLy >= rectUy || rectLz >= rectUz) {
    return 0;
  }
  else {
    return static_cast<float>(rectUx - rectLx) 
      * static_cast<float>(rectUy - rectLy)
      * static_cast<float>(rectUz - rectLz);
  }
}

void 
MyNLP::updateBinsGCellDensityVolumn(){
  // clear the Bin-area info
  for(int k=0;k<bins_.size();++k){
    for(int i=0;i<bins_[k].size();++i){
      for(int j=0;j<bins_[k][i].size();++j){
        bins_[k][i][j]->instPlacedVolumn_ = 0;
        bins_[k][i][j]->fillerVolumn_ = 0;
      }
    }
  }

  for (int i = 0; i < m_pDB->m_modules.size(); i++){ // for each cell
    double cellX = x[i * 2];
    double cellY = x[i * 2 + 1];
    double cellZ = z[i];

    double width = m_pDB->m_modules[i].GetWidth(z[i] - 0.5);
    double height = m_pDB->m_modules[i].GetHeight(z[i] - 0.5);
    double thickness = m_pDB->m_modules[i].m_thickness;
    //// use square to model small std-cells
    if (height < m_potentialGridHeight && width < m_potentialGridWidth && thickness < m_potentialGridThickness)
      width = height = thickness = 0;

    double left = cellX - width * 0.5 - _potentialRX;
    double bottom = cellY - height * 0.5 - _potentialRY;
    double back = cellZ - thickness * 0.5 - _potentialRZ;
    double right = cellX + (cellX - left);
    double top = cellY + (cellY - bottom);
    double front = cellZ + (cellZ - back);

    if (left < m_pDB->m_coreRgn.left - m_potentialGridPadding * m_potentialGridWidth)
      left = m_pDB->m_coreRgn.left - m_potentialGridPadding * m_potentialGridWidth;
    if (right > m_pDB->m_coreRgn.right + m_potentialGridPadding * m_potentialGridWidth)
      right = m_pDB->m_coreRgn.right + m_potentialGridPadding * m_potentialGridWidth;
    if (bottom < m_pDB->m_coreRgn.bottom - m_potentialGridPadding * m_potentialGridHeight)
      bottom = m_pDB->m_coreRgn.bottom - m_potentialGridPadding * m_potentialGridHeight;
    if (top > m_pDB->m_coreRgn.top + m_potentialGridPadding * m_potentialGridHeight)
      top = m_pDB->m_coreRgn.top + m_potentialGridPadding * m_potentialGridHeight;
    if (back < m_pDB->m_back - m_potentialGridPadding * m_potentialGridThickness)
      back = m_pDB->m_back - m_potentialGridPadding * m_potentialGridThickness;
    if (front > m_pDB->m_front + m_potentialGridPadding * m_potentialGridThickness)
      front = m_pDB->m_front + m_potentialGridPadding * m_potentialGridThickness;

    pair<int, int> pairX = getDensityMinMaxIdxX(left, right);
    pair<int, int> pairY = getDensityMinMaxIdxY(bottom, top);
    pair<int, int> pairZ = getDensityMinMaxIdxZ(back, front);

    // The following function is critical runtime hotspot 
    // for global placer.
    for(int gz = pairZ.first; gz < pairZ.second; gz++) {
      for(int gx = pairX.first; gx < pairX.second; gx++) {
        for(int gy = pairY.first; gy < pairY.second; gy++) {
          if(gz>=binNumZ_ || gx>=binNumX_ || gy>=binNumY_) continue;
          if( !m_pDB->m_modules[i].m_isVia && !m_pDB->m_modules[i].m_isFiller ) {
            // normal cells
            Bin* bin = bins_[gz][gx][gy];
            double overlapVolumn = getOverlapDensityVolumn(bin, left, right, bottom, top, back, front) * cellDensityScales_[i];
            bin->instPlacedVolumn_ += overlapVolumn; 
          } else if( m_pDB->m_modules[i].m_isFiller ) {
            Bin* bin = bins_[gz][gx][gy];
            double overlapVolumn = getOverlapDensityVolumn(bin, left, right, bottom, top, back, front)  * cellDensityScales_[i];
            bin->fillerVolumn_ += overlapVolumn;
          }
        }
      }
    }
  }  

  overflowVolumn_ = 0.0;

  // update density and overflowArea 
  // for nesterov use and FFT library
  for(int k=0;k<bins_.size();++k){
    for(int i=0;i<bins_[k].size();++i){
      for(int j=0;j<bins_[k][i].size();++j){
        Bin* bin = bins_[k][i][j];
        double binVolumn = (bin->ux_-bin->lx_) * (bin->uy_-bin->ly_); 
        bin->density_ =  
            ( static_cast<float> (bin->instPlacedVolumn_)
              + static_cast<float> (bin->fillerVolumn_) 
              + static_cast<float> (bin->nonPlaceVolumn_) )
            / static_cast<float>(binVolumn * bin->targetDensity_);

        double addingOverflowVolumn = static_cast<float>(bin->instPlacedVolumn_) 
                                + static_cast<float>(bin->nonPlaceVolumn_)
                                - (binVolumn * bin->targetDensity_);
        overflowVolumn_ += (addingOverflowVolumn > 0)? addingOverflowVolumn : 0;
      }
    }
  }
}

void 
MyNLP::updateBinsGCellDensityArea(){
  // clear the Bin-area info
  for(int k=0;k<bins_.size();++k){
    for(int i=0;i<bins_[k].size();++i){
      for(int j=0;j<bins_[k][i].size();++j){
        bins_[k][i][j]->instPlacedArea_ = 0;
        bins_[k][i][j]->fillerArea_ = 0;
      }
    }
  }

  for (int i = 0; i < m_pDB->m_modules.size(); i++){ // for each cell
    double cellX = x[i * 2];
    double cellY = x[i * 2 + 1];
    double cellZ = z[i];

    double width = m_pDB->m_modules[i].GetWidth(z[i] - 0.5);
    double height = m_pDB->m_modules[i].GetHeight(z[i] - 0.5);
    double thickness = m_pDB->m_modules[i].m_thickness;
    //// use square to model small std-cells
    if (height < m_potentialGridHeight && width < m_potentialGridWidth && thickness < m_potentialGridThickness)
      width = height = thickness = 0;

    double left = cellX - width * 0.5 - _potentialRX;
    double bottom = cellY - height * 0.5 - _potentialRY;
    double back = cellZ - thickness * 0.5 - _potentialRZ;
    double right = cellX + (cellX - left);
    double top = cellY + (cellY - bottom);
    double front = cellZ + (cellZ - back);

    if (left < m_pDB->m_coreRgn.left - m_potentialGridPadding * m_potentialGridWidth)
      left = m_pDB->m_coreRgn.left - m_potentialGridPadding * m_potentialGridWidth;
    if (right > m_pDB->m_coreRgn.right + m_potentialGridPadding * m_potentialGridWidth)
      right = m_pDB->m_coreRgn.right + m_potentialGridPadding * m_potentialGridWidth;
    if (bottom < m_pDB->m_coreRgn.bottom - m_potentialGridPadding * m_potentialGridHeight)
      bottom = m_pDB->m_coreRgn.bottom - m_potentialGridPadding * m_potentialGridHeight;
    if (top > m_pDB->m_coreRgn.top + m_potentialGridPadding * m_potentialGridHeight)
      top = m_pDB->m_coreRgn.top + m_potentialGridPadding * m_potentialGridHeight;
    if (back < m_pDB->m_back - m_potentialGridPadding * m_potentialGridThickness)
      back = m_pDB->m_back - m_potentialGridPadding * m_potentialGridThickness;
    if (front > m_pDB->m_front + m_potentialGridPadding * m_potentialGridThickness)
      front = m_pDB->m_front + m_potentialGridPadding * m_potentialGridThickness;

    pair<int, int> pairX = getDensityMinMaxIdxX(left, right);
    pair<int, int> pairY = getDensityMinMaxIdxY(bottom, top);
    pair<int, int> pairZ = getDensityMinMaxIdxZ(back, front);

    // The following function is critical runtime hotspot 
    // for global placer.
    for(int gz = pairZ.first; gz < pairZ.second; gz++) {
      for(int gx = pairX.first; gx < pairX.second; gx++) {
        for(int gy = pairY.first; gy < pairY.second; gy++) {
          if(gz>=binNumZ_ || gx>=binNumX_ || gy>=binNumY_) continue;
          if(gz != m_pDB->m_modules[i].m_z) continue;
          if( !m_pDB->m_modules[i].m_isVia && !m_pDB->m_modules[i].m_isFiller ) {
            // normal cells
            Bin* bin = bins_[gz][gx][gy];
            double overlapArea = getOverlapDensityArea(bin, left, right, bottom, top) * cellDensityScales_[i];
            bin->instPlacedArea_ += overlapArea; 
            //cout << "bin["<<gz<<"]["<<gx<<"]["<<gy<<"]: cell["<<i<<"] pos("<<cellZ<<","<<cellX<<","<<cellY<<"), bbox("<<back<<","<<left<<","<<bottom<<")->("<<front<<","<<right<<","<<top<<") --> overlap volumn="<<overlapArea<<"\n";
          } else if( m_pDB->m_modules[i].m_isFiller ) {
            Bin* bin = bins_[gz][gx][gy];
            double overlapArea = getOverlapDensityArea(bin, left, right, bottom, top)  * cellDensityScales_[i];
            bin->fillerArea_ += overlapArea;
          }
        }
      }
    }
  }  

  overflowArea_ = 0.0;

  // update density and overflowArea 
  // for nesterov use and FFT library
  for(int k=0;k<bins_.size();++k){
    for(int i=0;i<bins_[k].size();++i){
      for(int j=0;j<bins_[k][i].size();++j){
        Bin* bin = bins_[k][i][j];
        double binArea = (bin->ux_-bin->lx_) * (bin->uy_-bin->ly_); 
        bin->density_ =  
            ( static_cast<float> (bin->instPlacedArea_)
              + static_cast<float> (bin->fillerArea_) 
              + static_cast<float> (bin->nonPlaceArea_) )
            / static_cast<float>(binArea * bin->targetDensity_);

        double addingOverflowArea = static_cast<float>(bin->instPlacedArea_) 
                                + static_cast<float>(bin->nonPlaceArea_)
                                - (binArea * bin->targetDensity_);
        overflowArea_ += (addingOverflowArea > 0)? addingOverflowArea : 0;
      }
    }
  }
}

// eDensity Model Bin
Bin::Bin() 
  : x_(0), y_(0), z_(0), lx_(0), ly_(0), lz_(0),
  ux_(0), uy_(0), uz_(0), 
  nonPlaceArea_(0), instPlacedArea_(0),
  fillerArea_(0),
  nonPlaceVolumn_(0), instPlacedVolumn_(0),
  fillerVolumn_(0),
  density_ (0),
  targetDensity_(0),
  electroPhi_(0), 
  electroForceX_(0), electroForceY_(0), electroForceZ_(0) {}
  Bin::Bin(int x, int y, int lx, int ly, int ux, int uy, float targetDensity)
  : Bin() {
  x_ = x;
  y_ = y;
  lx_ = lx; 
  ly_ = ly;
  ux_ = ux;
  uy_ = uy;
  targetDensity_ = targetDensity;
}
Bin::Bin(int x, int y, int z, int lx, int ly, int lz, int ux, int uy, int uz, float targetDensity) 
  : Bin() {
  x_ = x;
  y_ = y;
  z_ = z;
  lx_ = lx; 
  ly_ = ly;
  lz_ = lz; 
  ux_ = ux;
  uy_ = uy;
  uz_ = uz;
  targetDensity_ = targetDensity;
}
Bin::~Bin() {
  x_ = y_ = z_ = 0;
  lx_ = ly_ = lz_ = ux_ = uy_ = uz_ = 0;
  nonPlaceArea_ = instPlacedArea_ = fillerArea_ = 0;
  nonPlaceVolumn_ = instPlacedVolumn_ = fillerVolumn_ = 0;
  electroPhi_ = electroForceX_ = electroForceY_ = electroForceZ_ = 0;
  density_ = targetDensity_ = 0;
}

void MyNLP::plotBoxPLT( ofstream& stream, double x1, double y1, double x2, double y2 )
{
    stream << x1 << ", " << y1 << endl << x2 << ", " << y1 << endl
           << x2 << ", " << y2 << endl << x1 << ", " << y2 << endl
           << x1 << ", " << y1 << endl << endl;
}
void MyNLP::plotPL(string fileName, int iter){
  string outFile = param.plotDir + fileName + ".plt";
  ofstream outfile( outFile.c_str() , ios::out );

  // head
  outfile << " " << endl;
  outfile << "set terminal png size 4000,3000" << endl;
  outfile << "set output " << "\"" << param.plotDir << fileName << ".png\"" << endl;
  outfile << "set size ratio 0.5" << endl;
  outfile << "set nokey" << endl << endl;

  if(iter >= 0){
    outfile << "set title \"Iter = " << iter << "\" font \",50\"" << endl;
  }
  // chip
  int chipW = (m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left);
  int chipH = (m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom);
  double bot_chip_offset = (m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left) + 5;
  outfile << "plot[:][:]  '-' w l lt 3 lw 2, '-' with filledcurves closed fc \"grey90\" fs border lc \"red\", '-' with filledcurves closed fc \"grey90\" fs border lc \"blue\"" << endl << endl;
  outfile << "# bounding box" << endl;
  plotBoxPLT( outfile, 0, 0, chipW, chipH ); // top chip
  plotBoxPLT( outfile, bot_chip_offset, 0, bot_chip_offset + chipW,chipH ); // bot chip
  outfile << "EOF" << endl;

  // cell
  outfile << "# cells" << endl;
  for(int i=0;i<m_pDB->m_nModules;++i){
    Module& module = m_pDB->m_modules[i];
    if(i == nModule_) {
      outfile << "EOF" << endl;
      outfile << "# fillers" << endl;
    }
    if(i < m_pDB->m_nModules){ // cells and vias
      double cellW = module.GetWidth(z[i]-0.5);
      double cellH = module.GetHeight(z[i]-0.5);
      if (z[i] < 1) {
          plotBoxPLT( outfile, x[2*i], x[2*i+1], x[2*i]+cellW, x[2*i+1]+cellH );
      } else {
          plotBoxPLT( outfile, bot_chip_offset+x[2*i], x[2*i+1], bot_chip_offset+x[2*i]+cellW, x[2*i+1]+cellH );
      }
    } else{ // filler
      if (z[i] < 1) {
          plotBoxPLT( outfile, x[2*i], x[2*i+1], x[2*i]+module.m_widths[0], x[2*i+1]+module.m_heights[0] );
      } else {
          plotBoxPLT( outfile, bot_chip_offset+x[2*i], x[2*i+1], bot_chip_offset+x[2*i]+module.m_widths[1], x[2*i+1]+module.m_heights[1] );
      }
    }
  }
  outfile << "EOF" << endl << endl;
  outfile.close();
  system(("gnuplot " + outFile + " &> tmp.txt; rm " + outFile).c_str());
}

void MyNLP::draw_field(int iter, string subDir){
  string dir = param.plotDir2 + subDir + "iter-" + to_string(iter)+"/";
  string mkdir_cmd = "mkdir -p " + dir;
  system(mkdir_cmd.c_str());

  // die.csv
  ofstream out_die( (dir+"die.csv").c_str() , ios::out );
  out_die << m_pDB->m_coreRgn.left << ", " << m_pDB->m_coreRgn.right << "\n";
  out_die << m_pDB->m_coreRgn.bottom << ", " << m_pDB->m_coreRgn.top;
  out_die.close();
  // cell-*.csv
  ofstream out_cellUp( (dir+"cell-up.csv").c_str() , ios::out );
  ofstream out_cellDown( (dir+"cell-down.csv").c_str() , ios::out );
  ofstream out_hb( (dir+"hb.csv").c_str() , ios::out );
  ofstream out_fillerUp( (dir+"filler-up.csv").c_str() , ios::out );
  ofstream out_fillerDown( (dir+"filler-down.csv").c_str() , ios::out );
  for(int i=0;i<m_pDB->m_modules.size();++i){
    if(m_pDB->m_modules[i].m_isVia) {
      out_hb << x[2*i] << ", " << x[2*i+1] << ", " << m_pDB->m_modules[i].m_width << ", " << m_pDB->m_modules[i].m_height << "\n";
    } else if(m_pDB->m_modules[i].m_isFiller) {
      if(z[i] < 1) {
        out_fillerUp << x[2*i] << ", " << x[2*i+1] << ", " << m_pDB->m_modules[i].m_widths[0] << ", " << m_pDB->m_modules[i].m_heights[0] << "\n";
      } else {
        out_fillerDown << x[2*i] << ", " << x[2*i+1] << ", " << m_pDB->m_modules[i].m_widths[1] << ", " << m_pDB->m_modules[i].m_heights[1] << "\n";
      }
    } else {
      if(z[i] < 1) {
        out_cellUp << x[2*i] << ", " << x[2*i+1] << ", " << m_pDB->m_modules[i].m_widths[0] << ", " << m_pDB->m_modules[i].m_heights[0] << "\n";
      } else {
        out_cellDown << x[2*i] << ", " << x[2*i+1] << ", " << m_pDB->m_modules[i].m_widths[1] << ", " << m_pDB->m_modules[i].m_heights[1] << "\n";
      }
    }
  }
  out_cellUp.close();
  out_cellDown.close();
  out_hb.close();
  out_fillerUp.close();
  out_fillerDown.close();
  // field-up-*.cvs
  ofstream out_fieldUpX( (dir+"field-up-x.csv").c_str() , ios::out );
  ofstream out_fieldUpY( (dir+"field-up-y.csv").c_str() , ios::out );
  for(int y=0;y<binNumY_;++y){
    for(int x=0;x<binNumX_;++x){
      out_fieldUpX << bins_[0][x][y]->electroForceX_;
      out_fieldUpY << bins_[0][x][y]->electroForceY_;
      if(x != binNumX_-1){
        out_fieldUpX << ", ";
        out_fieldUpY << ", ";
      }
    }
      out_fieldUpX << "\n";
      out_fieldUpY << "\n";
  }
  out_fieldUpX.close();
  out_fieldUpY.close();
  // field-down-*.cvs
  ofstream out_fieldDownX( (dir+"field-down-x.csv").c_str() , ios::out );
  ofstream out_fieldDownY( (dir+"field-down-y.csv").c_str() , ios::out );
  for(int y=0;y<binNumY_;++y){
    for(int x=0;x<binNumX_;++x){
      out_fieldDownX << bins_[1][x][y]->electroForceX_;
      out_fieldDownY << bins_[1][x][y]->electroForceY_;
      if(x != binNumX_-1){
        out_fieldDownX << ", ";
        out_fieldDownY << ", ";
      }
    }
      out_fieldDownX << "\n";
      out_fieldDownY << "\n";
  }
  out_fieldDownX.close();
  out_fieldDownY.close();
}