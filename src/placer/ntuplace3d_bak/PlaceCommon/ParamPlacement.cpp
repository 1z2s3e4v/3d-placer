#include <string>
#include <iostream>
#include <cstdio>
using namespace std;

#include "ParamPlacement.h"

CParamPlacement param;	// global variable (2006-03-22) donnie

CParamPlacement::CParamPlacement()
{
    hmetis_ubfactor   = -1;	// (2006-02-13) -1 means nouse
    hmetis_run        = 1;     // hMETIS: run #
    hmetis_ctype      = 2;     // hMETIS: coarsening type
    hmetis_rtype      = 2;     // hMETIS: refinement type
    hmetis_vcycle     = 3;     // hMETIS: v-cycle (change default=3 2006-02-02)
    hmetis_debug      = 0;     // hMETIS: debug level
    minBlocks = 1;             // minimum blocks in a region
    ubfactor  = 0.95;	       // The unbalanced factor of NTUplace.   (UNUSE)
    coreShrinkFactor = 1.00;
    coreShrinkWidthFactor = 1.00;
    preLegalFactor  = 0.995;
    bFractionalCut  = true;
    bPrePartitioning = false;
    bRefineParts    = false;       // Bisection/Level refinement
    bRefinePartsAll = false;
    bRefineUseMetis = true;        // hMETIS or FM
    n_repart = 1;
    
    // flow
    bRunMPTreeLegal = false;
    bMPTreeRotate = false;
    bRunMacroLegal = false;
    bRunMacroShifter = true;
    //bRunDetail = true;
    //bRunLegal = true;
    bRunCellMatching = true;
    bRunBB = true;
    //bRunGlobal = true;
    //bRunLAL = true;

    // stats
    n_ctype_count[1] = 0;
    n_ctype_count[2] = 0;
    n_ctype_count[3] = 0;
    n_ctype_count[4] = 0;
    n_ctype_count[5] = 0;

    aspectRatio = 1.0;

    scaleType = SCALE_TO_MIDLINE;
    //coreUtil = 0.96;                // The miracle number: 0.96.
    //coreUtil = -1;                  // for NLP 
    coreUtil = 1.00;                  // for NLP 

    bShow = false;                 // Show the parameters
    bLog = false;                  // Log the result
    bPlot = false;
    outFilePrefix = "out";         // out_global.pl & out_legal.pl
    outFilePrefixAssigned = false;

    // input type

    useAUX = true;
    plFilename = "";
    nodesFilename = "";
    
    // cell matching (Assignment) detailed placer
    de_MW = 90;
    de_MM = 128;
    de_btime = true;
    de_time_constrain = 28800;	// 8 hours
    de_window = 20; //chang to 20 by tchsu 2006_06_17
    //cellMatchingStop = 0.2;	// 2006-04-01
    //cellMatchingStopAssigned = false;

    // 2005-12-15
    setOutOrientN = false;	// false: keep original orient
    bHandleOrientation = false;
    
    //	2006-01-16 indark
    n_MacroRowHeight = 1;

    // NLP
    step = 0.3;		    // NLP solving step size propotional to the grid size
    stepAssigned = false;
    weightWire = 1.0;	    // init wire force weight 
    
    //precision = 0.9999; // 2006-0629
    //precision = 0.99999; // 2006-09-23
    
    topDblStep = false;
    //binSize = 0.8;
    //startDelta = 4.0;
    startDelta = 1.5;
    endDelta = 1.0;
    startDensity = 1.10;	// default = 1.10 for ispd06 contest    from 10% to 1%
    endDensity = 1.01;
    //targetDenOver = 0.05;	// default = 0.05 for ispd06 contest
    targetDenOver = 0.00;
    bRunMacroShifter = true;
    truncationFactor = 1.0;
    bAdjustForce = true;
    bOutTopPL = false;
    bQP = true;
    //clusterRatio = 5;
    //clusterBlock = 6000;
    //maxLevel = INT_MAX;
    bPrototyping = false;

    bPerturb = false;	// perturb when declustering?
    
    //Congestion mode defaultly closed
    bCong = false;

    // X-arch
    //bXArch = false;

    // Astro Dump file
    bOutAstro = false;
    
    dLpNorm_P = 300; // default "P" for lp-norm / "alpha" in LSE
    bUseLSE = true;
    bWireModelParameterAssigned = false;

    // 2006-06-14
    nThread = 1;
    
    //Brian 2007-04-30
    bCongObj = false;
    bCongStopDynamic = true;
    dCongDivRatio = 2.7;
    dCongTargetUtil = 0.1;
    dCongWeight = 1.0;
    bCongSmooth = false;
    bFlatLevelCong = false;
    bDrawNetCongMap = false;
    //@Brian 2007-04-20

    //Brian 2007-04-18
    //Use Net Weight in NLP
    bNLPNetWt = false;

    //Added by Jin 20081013
    bTiming = false;

    // Added by Frank 20220605
    b3d = false;
    nlayer = 1;
    bLayerPreAssign = false;
    dWeightTSV = 0.0;
}

void CParamPlacement::Print()
{
    cout << "\nParameters:\n";
    cout << "  Seed              = " << seed << endl;
    //cout << "  Out Prefix        = " << outFilePrefix << endl;
    cout << "  Cell Orientation  = " << TrueFalse( bHandleOrientation ) << endl;
    cout << "  target_util       = " << coreUtil << endl;
    cout << "  thread #          = " << nThread << endl;
    //cout << "  liberty           = " << libertyFilename << endl;
    
    cout << "\nFlow:\n";
    //cout << "  Global            = " << TrueFalse( bRunGlobal ) << endl;
    //cout << "    LAL             = " << TrueFalse( bRunLAL ) << endl;
    //cout << "  Legal             = " << TrueFalse( bRunLegal ) << endl;
    //cout << "  Detailed          = " << TrueFalse( bRunDetail ) << endl;
    cout << "    BB              = " << TrueFalse( bRunBB ) << endl;
    cout << "    CM              = " << TrueFalse( bRunCellMatching ) << endl;
    
    cout << "\nMin-Cut Placer:\n";
    cout << "  part_run    = " << hmetis_run << endl;
    cout << "  part_ctype  = " << hmetis_ctype << endl;
    cout << "  part_rtype  = " << hmetis_rtype << endl;
    cout << "  part_vcycle = " << hmetis_vcycle << endl;
    //cout << "  ubfactor    = " << ubfactor << endl;
    cout << "  ubfactor    = " << hmetis_ubfactor << endl;
    //cout << "  core_shrink       = " << coreShrinkFactor << endl;
    //cout << "  core_shrink_width = " << coreShrinkWidthFactor << endl;
    //cout << "  pre_legal_factor  = " << preLegalFactor << endl;
    cout << "  fractional_cut    = " << TrueFalse( bFractionalCut ) << endl;
    cout << "  refine_parts      = " << TrueFalse( bRefineParts ) << endl;
    cout << "  n_repart          = " << n_repart << endl;
    //cout << "  refine_use_hmetis = " << bRefineUseMetis << endl;
    //cout << "  prelegal_type     = " << scaleType << endl;
    cout << "  aspect_ratio      = " << aspectRatio << endl;
    cout << "  MacroRowHeight    = " << n_MacroRowHeight << endl;
    
    cout << "\nNLP:\n";
    //cout << "  prototying        = " << TrueFalse( bPrototyping ) << endl;
    cout << "  use lse           = " << TrueFalse( bUseLSE ) << endl;
    cout << "  macro shifter     = " << TrueFalse( bRunMacroShifter ) << endl;
    cout << "  perturb           = " << TrueFalse( bPerturb ) << endl;
    //cout << "  bin size          = " << binSize << endl;
    cout << "  step size         = " << step << endl;
    cout << "  wire weight       = " << weightWire << endl;
    //cout << "  precision         = " << precision << endl;
    cout << "  topdblstep        = " << topDblStep << endl;
    cout << "  init delta        = " << startDelta << endl;
    cout << "  end delta         = " << endDelta << endl;
    cout << "  start density     = " << startDensity << endl;
    cout << "  end density       = " << endDensity << endl;
    cout << "  target over       = " << targetDenOver << endl;
    cout << "  adjust force      = " << TrueFalse( bAdjustForce ) << endl;
    cout << "  truncation        = " << truncationFactor << endl;
    cout << "  out top pl        = " << TrueFalse( bOutTopPL ) << endl;
    cout << "  qp                = " << TrueFalse( bQP ) << endl;
    //cout << "  c_block           = " << clusterBlock << endl;
    //cout << "  c_ratio           = " << clusterRatio << endl;
    
    //cout << "\nCell Matching:\n";
    //cout << "  stop              = " << cellMatchingStop << endl;

    cout << "  qp                = " << TrueFalse( b3d ) << endl;
    cout << "  nlayer            = " << nlayer << endl;
    cout << "\n";

    //Added by Jin 20060510
    //cout << "Congestion:" << endl;
    //cout << "  bCong             = " << bCong << endl << endl;
}

string TrueFalse( bool b )
{
    if( b )
	return string( "true" );
    else
	return string( "false" );
}

string CParamPlacement::GetParamString( bool runMincut )
{
	
    char s[500];
    //sprintf( s, "seed=%u run=%d ctype=%d rtype=%d vcycle=%d ubfactor=%.2f util=%.2f shrink=%.2f %.2f %.2f, fracCut=%d prepart=%d refine=%d hmetis=%d scale=%d ar=%.2f",
    //            seed, 
    //            hmetis_run, hmetis_ctype, hmetis_rtype, hmetis_vcycle, 
    //            ubfactor, coreUtil, coreShrinkFactor, coreShrinkWidthFactor, preLegalFactor, 
    //            bFractionalCut, bPrePartitioning, bRefineParts, bRefineUseMetis, scaleType, aspectRatio );
    
    //sprintf( s, "see=%u run=%d c=%d r=%d v=%d util=%.2f prel=%.2f frac=%d prep=%d ref=%d scal=%d ar=%.2f",
    //            seed, 
    //            hmetis_run, hmetis_ctype, hmetis_rtype, hmetis_vcycle, 
    //            coreUtil, preLegalFactor, 
    //            bFractionalCut, bPrePartitioning, bRefineParts, scaleType, aspectRatio );

    if( runMincut )
    {
	sprintf( s, "sd%d run%d ct%d util%.2f prel%.3f fr%d ref%d:%d st%d ub%d",
		(int)seed, 
		(int)hmetis_run, (int)hmetis_ctype, 
		coreUtil, preLegalFactor, 
		bFractionalCut, bRefineParts, n_repart,
		(int)scaleType,
		hmetis_ubfactor );
    }
    else
    {
	sprintf( s, "u%.2f s%.2f", coreUtil, step );
    }

    return string(s);
}

