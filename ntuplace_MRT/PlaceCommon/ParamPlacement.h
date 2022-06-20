#ifndef PARAMPLACEMENT_H
#define PARAMPLACEMENT_H
#include <string>

#include "arghandler.h"

enum SCALE_TYPE
{
SCALE_TO_LEFT,
SCALE_TO_RIGHT,
SCALE_TO_MIDLINE,
SCALE_TO_LEFT_BETWEEN_MACRO,
SCALE_TO_RIGHT_BETWEEN_MACRO,
SCALE_TO_MIDLINE_BETWEEN_MACRO
};

string TrueFalse( bool );

class CParamPlacement
{
public:
    CParamPlacement();
    void Print();
    string GetParamString( bool runMincut );    // Get parameter string for logging

    unsigned long int seed;

    int hmetis_run;            // hMETIS: run #
    int hmetis_ctype;          // hMETIS: coarsening type
    int hmetis_rtype;          // hMETIS: refinement type
    int hmetis_vcycle;         // hMETIS: v-cycle
    int hmetis_debug;          // hMETIS: debug level

    int minBlocks;             // minimum blocks in a region
    double ubfactor;	       // The unbalanced factor of NTUplace.   (UNUSE)

    int hmetis_ubfactor;	// 2006-02-13 (donnie) ubfactor for hmetis
    
    double coreShrinkFactor;
    double coreShrinkWidthFactor;
    double preLegalFactor;
    double coreUtil;            // target core utilization

    bool bFractionalCut;
    bool bPrePartitioning;
    bool bRefineParts;          // placement feedback
    bool bRefinePartsAll;	// neighborhood refinement
    bool bRefineUseMetis;       // hMETIS or FM

    int n_repart;               // repartition bad count
    int n_ctype_count[6];

    double aspectRatio;         // The partition aspect ratio

    enum SCALE_TYPE scaleType;
	

    bool bShow;                 // Show the parameters
    bool bLog;                  // Log the result
    bool bPlot;                 // Out the gnuplot figure
    string outFilePrefix;
    bool outFilePrefixAssigned;

    // cell matching detailed placer
    int  de_MW;
    int  de_MM;
    bool de_btime;
    int  de_time_constrain;
    int  de_window;
    //double cellMatchingStop;
    bool   cellMatchingStopAssigned;
	
    bool useAUX;		// true: aux; false: lefdef
    string plFilename;
    string nodesFilename;

    // 2006-05-11
    string libertyFilename;
    

    // 2005-12-15
    bool setOutOrientN;		// modify output for setting all blocks orient N
   
    // NLP
    //double alpha;
    double step;
    double weightWire;
    bool   stepAssigned;
    //double precision;
    bool topDblStep;
    //double binSize;
    double startDelta;
    double endDelta;
    double startDensity;
    double endDensity;
    double targetDenOver;
    bool   bAdjustForce; 
    double truncationFactor;
    bool   bOutTopPL;
    bool   bQP;
    int    clusterBlock;
    double clusterRatio;
    //int    maxLevel;
    bool   bPerturb;
    
    //indark:060116
    int n_MacroRowHeight;	// minimum # of row height a macro should be
    
    bool bRunMPTreeLegal;
    bool bRunMacroLegal;
    bool bMPTreeRotate;
    bool bRunMacroShifter;
    //bool bRunGlobal;
    //bool bRunLAL;
    bool bRunDetail;
    //bool bRunLegal;
    bool bRunBB;
    bool bRunCellMatching;
    bool bHandleOrientation;
    bool bPrototyping;

    //For congestion
    //Added by Jin 20060510
    bool bCong;

    //bool bXArch;    // 2006-05-23 (donnie)


    // Astro Dump file
    bool bOutAstro;
    
    //LpNorm
    double dLpNorm_P;
    bool bUseLSE;
    bool bWireModelParameterAssigned;

    //Stable-LSE
    bool bUseSLSE;

    // Macro rotation force
    bool bUseMacroRT;

    // Run initial placement
    bool bRunInit;

    // multithread
    int nThread;

    //Use NetWeight in NLP
    bool bNLPNetWt;
    
    //Brian 2007-04-30
    bool    bCongObj;
    bool    bCongStopDynamic;
    bool    bFlatLevelCong;
    bool    bCongSmooth;
    bool    bDrawNetCongMap;
    double  dCongDivRatio;
    double  dCongTargetUtil;
    double  dCongWeight;
    //@Brian 2007-04-30

    // format of input files
    enum InputType{
	BOOKSHELF,
	LEFDEF } fileType;

    //Added by Jin 20081013
    bool bTiming;

};

extern CParamPlacement param;	// global variable

#endif
