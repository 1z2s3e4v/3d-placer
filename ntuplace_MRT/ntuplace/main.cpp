#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cfloat>
using namespace std;

#include "util.h"
#include "legal.h"
#include "DiamondSearch.h"
#include "DiamondLegalizer.h"
#include "Prelegalizer.h"
#include "detail.h"
#include "lefdef.h"
#include "ParserIBMMix.h"
#include "placedb.h"
#include "randomplace.h"
#include "MyNLP.h"
#include "MyQP.h" //by tchsu
#include "verify.h"
#include "placeutil.h"
#include "CongMap.h"

//Added by Jin 20060223
#include "DPlace.h"
CDetailPlacerParam dpParam;

//Added by Jin 20060224
#include "TetrisLegal.h"

//Added by Jin 20060920
#include "GlobalRouting.h"

//Added by Jin 20061003
#include "SteinerDecomposition.h"

#include "MyNLP.h"
#include "mlnlp.h"	// multilevel nlp global placement
#include "mlqp.h"	// multilevel qp global placement by tchsu

#include "PlaceDBQP.h"
#include "fccluster.h" 

#include "macrolegal.h"
#include "mptreelegal.h"
#include "placebin.h"
#include "cellmoving.h" //by tchsu 2006.03.01

// planet
#include "libdb.h"
#include "liberty.h"

// indark
#include "qpforce.h"

#include "arghandler.h"

//test by Jin 20070125 
#include "WhiteSpaceAllocation.h"
//@test by Jin 20070125 

// 2007-02-13 (donnie)
#include "GRouteMap.h"


// indark
// 2007-02-27 (donnie)
			 
#include "macrosp.h"
			 
// 2007-03-11 (donnie)
#include "BlockMatching.h"
#include "randomplace.h"

// 2007-04-16 (donnie)
#include "LpLegal.h"
#include "SingleRowPlacement.h"

//Added by Jin 20081010
#include "timing.h"
#include "CriticalPath.h"

// 2005-12-05
string outBookshelf;
string outBookshelfNoBlockSite;

// 2005-12-18
bool bPlotOnly = false;

double programStartTime = 0;

double timePacking = 0.0;
double timeDataPrepare = 0.0;
double timePartition = 0.0;

//==============================================
bool bOutInterPL = false;
bool bCellRedistribution = true; //by tchsu 2006.03.10
double gCellRedistributionTarget = 1.0; //by tchsu 2006.03.12
//============================================== by indark
int nMacroLegalRun = 40;
//==============================================
bool bRunMincut = false;

bool bBuildInfo = false;

CParserLEFDEF parserLEFDEF;	// global var for WriteDEF

////////////////////////////////////////////////
// parameters for mlnlp
bool   gRunWCycle = false;
double gWeightLevelDecreaingRate = 1.0;
double gTargetUtil = 1.00;
////////////////////////////////////////////////

bool bShrinkCore = false;

//int gCType = 4;   // "first choice"
int gCType = 5;	    // (donnie) 2007-07-13 change default to "best choice"

bool handleArgument( const int& argc, char* argv[], CParamPlacement& param )
{
    
    param.bUseLSE = false;  // NTUplace3-LE

    param.bRunInit = true;

    int i;
    if( strcmp( argv[1]+1, "aux" ) == 0 )
	i = 3;
    else
	i = 4;
    while( i < argc )
    {
	// 2006-03-19 (donnie) remove checking "-"
	if( strlen( argv[i] ) <= 1 )	// bad argument?
	{
	    i++;
	    continue;
	}

	////////////////////////////////////////////////////////////////////////
	// Public arguments
	////////////////////////////////////////////////////////////////////////

	if( strcmp( argv[i]+1, "seed" ) == 0 )
	    param.seed = (unsigned long)atol( argv[++i] );
	//else if( strcmp( argv[i]+1, "noglobal" ) == 0 )
	//    param.bRunGlobal = false;
	else if( strcmp( argv[i]+1, "nolegal" ) == 0 )
	{
	    gArg.Override( "nolal", "" );
	    gArg.Override( "nolegal", "" );
	    //gArg.Override( "nodetail", "" );
	    //param.bRunLAL = false;
	    //param.bRunLegal = false;
	    //param.bRunDetail = false;
	}
	//else if( strcmp( argv[i]+1, "nodetail" ) == 0 )
	//    param.bRunDetail = false;	
	else if( strcmp( argv[i]+1, "loadpl" ) == 0 )
	{
	    param.plFilename = string( argv[++i] );
	}
	else if( strcmp( argv[i]+1, "plt" ) == 0 )
	{
	    printf( "<<< Plotter >>>\n" );
	    bPlotOnly = true;
	    bOutInterPL = true;
	}
	else if( strcmp( argv[i]+1, "out" ) == 0 )	
	{
	    param.outFilePrefix = argv[++i];
	    param.outFilePrefixAssigned = true;
	}
	else if( strcmp( argv[i]+1, "util" ) == 0 )	// used in both min-cut and nlp 
	{
	    param.coreUtil = atof( argv[++i] );         // legalization
	    gTargetUtil = param.coreUtil;               // for nlp
	    gCellRedistributionTarget = param.coreUtil; // cell shifting
	}
	else if( argv[i][1] == 'd' && argv[i][2] == 'e' && argv[i][3] == 'v' &&
	 	 argv[i][4] == 'd' && argv[i][5] == 'e' && argv[i][6] == 'v' )
	{
	    param.bPlot = true;
	    param.bShow = true;
	    param.bLog = true;
	    bOutInterPL = true;
	    bBuildInfo = true;
	    param.bOutTopPL = true;
	}

	////////////////////////////////////////////////////////////////////////////
	// Development Arguments
	////////////////////////////////////////////////////////////////////////////
	
	if( false == param.bShow )
	{
	    ++i;
	    continue;
	}
	   
        else if( strcmp( argv[i]+1, "noplace" ) == 0 )
	{
	    gArg.Override( "noglobal", "" );
	    gArg.Override( "nolegal", "" );
	    gArg.Override( "nodetail", "" );
	}	    
	else if( strcmp( argv[i]+1, "plot" ) == 0 )
	    param.bPlot = true;
	//else if( strcmp( argv[i]+1, "detail" ) == 0 )
	//    param.bRunDetail = true;
	else if( strcmp( argv[i]+1, "nobb" ) == 0 )
	    param.bRunBB = false;
	//else if( strcmp( argv[i]+1, "nolal" ) == 0 )
	//    param.bRunLAL = false;
	else if( strcmp( argv[i]+1, "log" ) == 0 )
	    param.bLog = true;
	else if( strcmp( argv[i]+1, "loadnodes" ) == 0 )
	    param.nodesFilename = string( argv[++i] );
	else if( strcmp( argv[i]+1, "setoutorientn" ) == 0 )
	    param.setOutOrientN = true;
	else if( strcmp( argv[i]+1, "outbookshelf" ) == 0 )
	{
	    printf( "<< Bookshelf Converter >>\n" );
	    outBookshelf = string( argv[++i] );
	}
	else if( strcmp( argv[i]+1, "outbookshelfnoblocksite" ) == 0 )
	{
	    printf( "<<< Bookshelf Converter (remove preplaced block site) >>\n" );
	    outBookshelfNoBlockSite = string( argv[++i] );
	}
	else if( strcmp( argv[i]+1, "flip" ) == 0 )
	    param.bHandleOrientation = true;
	else if( strcmp( argv[i]+1, "th" ) == 0 )
	    param.nThread = atoi( argv[++i] );
	    
	// min-cut placer	
	else if( strcmp(argv[i]+1, "mincut") == 0 )
	{
	    printf( "<< NTU Min-Cut Placer >>\n" );
	    bRunMincut = true;
	}
	else if( strcmp( argv[i]+1, "run" ) == 0 )
	    param.hmetis_run = atol( argv[++i] );
	else if( strcmp( argv[i]+1, "rtype" ) == 0 )
	    param.hmetis_rtype = atol( argv[++i] );
	else if( strcmp( argv[i]+1, "vcycle" ) == 0 )
	    param.hmetis_vcycle = atol( argv[++i] );
	else if( strcmp( argv[i]+1, "ubfactor" ) == 0 )
	    param.ubfactor = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "ub" ) == 0 )
	    param.hmetis_ubfactor = atoi( argv[++i] );
	else if( strcmp( argv[i]+1, "prepart" ) == 0 )	
	    param.bPrePartitioning = true;
	else if( strcmp( argv[i]+1, "refine" ) == 0 )
	    param.bRefineParts = true;
	else if( strcmp( argv[i]+1, "refineAll" ) == 0 )   
	    param.bRefinePartsAll = true;
	else if( strcmp( argv[i]+1, "ar" ) == 0 )  
	    param.aspectRatio = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "nofrac" ) == 0 )
	    param.bFractionalCut = false;
	else if( strcmp( argv[i]+1, "shrink" ) == 0 )
	    param.coreShrinkFactor = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "shrinkWidth" ) == 0 )
	    param.coreShrinkWidthFactor = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "repart" ) == 0 )
	    param.n_repart = atoi( argv[++i] );

	else if( strcmp( argv[i]+1, "ctype" ) == 0  && param.bShow )	// used in both min-cut and nlp
	{
	    i++;
	    param.hmetis_ctype = atol( argv[i] );
	    gCType = atol( argv[i] );
	}


	// tellux diamond legalizer	
	else if( strcmp( argv[i]+1, "preLegal" ) == 0 )
	    param.preLegalFactor = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "scaleType" ) == 0 )
	    param.scaleType = (SCALE_TYPE)atoi( argv[++i] );

	
	// tellux cell matching detailed placer
	//else if( strcmp( argv[i]+1, "stop" ) == 0 )
	//{
	    //param.cellMatchingStop = atof( argv[++i] );
	    //param.cellMatchingStopAssigned = true;
	//}
	else if( strcmp( argv[i]+1, "de_MW" ) == 0 )	
	    param.de_MW = (unsigned long)atol( argv[++i] );
	else if( strcmp( argv[i]+1, "de_MM" ) == 0 )
	    param.de_MM = (unsigned long)atol( argv[++i] );
	else if( strcmp( argv[i]+1, "de_window" ) == 0 )    
	    param.de_window = atoi( argv[++i] );
	else if( strcmp( argv[i]+1, "de_time" ) == 0 )	
	{
	    param.de_btime=true;
	    param.de_time_constrain = atoi( argv[++i] );
	}
       
	
	// indark macro legalizer
	else if( strcmp(argv[i]+1, "macrolegal") == 0 )	    
	{
	    printf( "<< Macro Legalizer Enabled >>\n" );
	    param.bRunMacroLegal = true;
	}
	else if ( strcmp(argv[i]+1, "macroheight") == 0 )
	    param.n_MacroRowHeight = atoi( argv[++i] );
	else if ( strcmp(argv[i]+1, "macrolegalrun") == 0 )   
	    nMacroLegalRun = atoi( argv[++i] );
	else if ( strcmp(argv[i]+1, "mptreelegal") == 0 )    
	    param.bRunMPTreeLegal = true;
	else if ( strcmp(argv[i]+1, "mptreerotate") == 0 )    
		param.bMPTreeRotate = true;

	else if ( strcmp(argv[i]+1, "nomacroshifter") == 0 )		// nomacroshifter
		param.bRunMacroShifter = false;
	
	// donnie.  NLP placer (2006-03-19)
	else if( strcmp( argv[i]+1, "nlp" ) == 0 )
	{
	    bRunMincut = false;
	    param.bUseLSE = true;
	}
	else if( strcmp( argv[i]+1, "slse" ) == 0 )
	{
	    bRunMincut = false;
	    param.bUseLSE = true;
	    param.bUseSLSE = true;
	}
	else if( strcmp( argv[i]+1, "step" ) == 0 )	 // 2006-03-24
	{
	    param.stepAssigned = true;
	    param.step = atof( argv[++i] );
	}
	else if( strcmp( argv[i]+1, "topdblstep" ) == 0 )
	    param.topDblStep = true;
	else if( strcmp( argv[i]+1, "wwire" ) == 0 )	
	    param.weightWire = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "nlpweightDe" ) == 0 )  
	    gWeightLevelDecreaingRate = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "UTIL" ) == 0 )	     
	    gTargetUtil = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "shrinkCore" ) == 0 )
	    bShrinkCore = true;
	else if( strcmp( argv[i]+1, "startDensity" ) == 0 )
	    param.startDensity = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "endDensity" ) == 0 )
	    param.endDensity = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "startDelta" ) == 0 )
	    param.startDelta = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "endDelta" ) == 0 )
	    param.endDelta = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "targetDenOver" ) == 0 )
	    param.targetDenOver = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "noms" ) == 0 )
	    param.bRunMacroShifter = false;
	else if( strcmp( argv[i]+1, "noadjust" ) == 0 )
	    param.bAdjustForce = false;
	else if( strcmp( argv[i]+1, "truncat" ) == 0 )
	    param.truncationFactor = atof( argv[++i] );
	else if( strcmp( argv[i]+1, "outtoppl" ) == 0 )
	    param.bOutTopPL = true;
	else if( strcmp( argv[i]+1, "noqp" ) == 0 )
	    param.bQP = false;
	else if( strcmp( argv[i]+1, "proto" ) == 0 )
	{
	    param.bPrototyping = true;
	    //param.bRunDetail = false;
	    gArg.Override( "nodetail", "" );
	    param.weightWire = 0.5;
	}
	
	
	
	//Added by Jin 20060223
	else if( strcmp( argv[i]+1, "bbcellswap" ) == 0 )
		dpParam.SetRunBBCellSwap();
	else if( strcmp( argv[i]+2, "bbwindowsize" ) == 0 )
		dpParam.SetBBWindowSize( atoi(argv[++i]) );
	else if( strcmp( argv[i]+1, "bboverlapsize" ) == 0 )
		dpParam.SetBBOverlapSize( atoi(argv[++i]) );
	else if( strcmp( argv[i]+1, "bbiteration" ) == 0 )
		dpParam.SetBBIteration( atoi(argv[++i]) );

	
	// tellux cell redistributor (density constraint)
	else if( strcmp( argv[i]+1, "cr" ) == 0 ) //by tchsu
	{
	    i++;
	    if( atof(argv[i]) < 0 )
		bCellRedistribution = false;
	    else
	    {
		bCellRedistribution = true; 
		gCellRedistributionTarget= atof( argv[i] );
	    }
	}

	//Arguements for congestion mode
	//Added by Jin 20060510
	else if( strcmp( argv[i]+1, "cong" ) == 0 )
	{
	    param.bCong = true;
	    // 2006-09-26
	    //gArg.Override( "nodetail", "" );
	    gArg.Override( "nlp", "" );
	    //gArg.Override( "nocheck", "" );
	    gArg.Override( "steiner", "" );
	    param.bUseLSE = true;
	    bRunMincut = false;
	}

	// donnie 2006-05-11
	else if( strcmp( argv[i]+1, "lib" ) == 0 )
	    param.libertyFilename = argv[++i];

	// donnie 2006-05-23
	//else if( strcmp( argv[i]+1, "x" ) == 0 )
	//    param.bXArch = true;
	
	else if( strcmp( argv[i]+1, "outastro" ) == 0 )
	    param.bOutAstro = true;

	
	//indark 2006-05-26
	else if( strcmp( argv[i]+1, "lpnorm_p" ) == 0 || strcmp( argv[i]+1, "alpha" ) == 0 || strcmp( argv[i]+1, "p" ) == 0 )
	{
	    param.bWireModelParameterAssigned = true;
	    param.dLpNorm_P = atof( argv[++i] );
	}
	else if( strcmp( argv[i]+1, "lpnorm" ) == 0 )
	    param.bUseLSE = false;
	else if( strcmp( argv[i]+1, "perturb" ) == 0 )
	    param.bPerturb = (bool)atoi( argv[++i] );
	
	//Brian 2007-04-18
	else if( strcmp( argv[i]+1, "nlp_netweight" ) == 0 )
	    param.bNLPNetWt = true;
	//indark 2007-04-27
	else if( strcmp( argv[i]+1, "noCM" ) == 0 )
		param.bRunCellMatching = false;
    //Brian 2007-04-30
    else if ( strcmp( argv[i]+1, "congobjdyn" ) == 0 )
    {
        param.bCongObj = true;
        param.bCongStopDynamic = true;
    }
    else if ( strcmp( argv[i]+1, "congsmooth" ) == 0 )
    {
	param.bCongObj = true;
	param.bCongSmooth = true;
    }
    else if ( strcmp( argv[i]+1, "congobjutil" ) == 0 )
    {
        param.bCongObj = true;
        param.dCongTargetUtil = atof( argv[++i] );
    }
    else if ( strcmp( argv[i]+1, "congweight" ) == 0 )
    {
        param.bCongObj = true;
        param.dCongWeight = atof( argv[++i] );
    }
    else if ( strcmp( argv[i]+1, "congdiv" ) == 0 )
    {
        param.bCongObj = true;
        param.dCongDivRatio = atof( argv[++i] );
    }
    else if  ( strcmp( argv[i]+1, "lastlevelcong" ) == 0 )
    {
	    param.bFlatLevelCong = true;
    }
    else if  ( strcmp( argv[i]+1, "drawNetCongMap" ) == 0 )
    {
	    param.bDrawNetCongMap = true;
    }
    
    //@Brian 2007-04-30
	
    else if ( strcmp( argv[i]+1, "MRT") == 0 ) // kaie
    {
	param.bUseMacroRT = true;
	gArg.Override( "BestOrient", "" );
	bRunMincut = false;
	gArg.Override( "nlp", "" );
	param.bUseLSE = true;
	//param.bRunInit = false;
    }
	
	
	//else 
	//{
	//    cout << "Unknown argument: " << argv[i] << endl;
	//    return false;
	//}
	i++;
    }
    return true;
}



///////////////////////////////////////////////////////////////////////////

void printUsage()
{
    printf( "\n" );
    //printf( "Usage: (1) place.exe -aux circuit.aux [-util float]\n" );
    //printf( "       (2) place.exe -lefdef circuit.lef circuit.def [-util float]\n" );
    printf( "Usage: place.exe -aux circuit.aux [-util float] [-out prefix]\n" );
    printf( "Ex: place.exe -aux adaptec1.aux -util 0.8 -out adaptec1\n" );
    printf( "\n" );
    
#if 0
    printf( "Available options:\n" );
    printf( "-loadpl place.pl    // load a placement\n" );
    printf( "-out place.pl       // output placementin 'place.ntup.pl'\n" );
    printf( "-util float         // target density (default = %f)", param.coreUtil );
    printf( "\n\n" ); 
#endif
}

///////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
    gArg.Init( argc, argv );
    programStartTime = seconds();

    // 2007-03-25 (donnie) Change the default mode 
#undef NTUPLACE3_LE

#ifdef NTUPLACE3_LE
    param.bUseLSE = false;   // NTUplace3-LE
    printf( "\n\n" );
    printf( "      NTUplace3-LE (%s)\n\n", __NTUPLACE_VERSION__ );
#else
    param.bUseLSE = true;
    printf( "\n\n" );
    printf( "      NTUplace3 (%s)\n\n", __NTUPLACE_VERSION__ );
#endif

    printf( "      Graduate Institute of Electronics Engineering &\n" );
    printf( "      Department of Electrical Engineering\n" );
    printf( "      National Taiwan University\n\n" );
    printf( "      Copyright 2005-2012\n" );
    printf( "      Tung-Chieh Chen, Zhe-Wei Jiang,\n" );
    printf( "      Tien-Chang Hsu, Hsin-Chen Chen, Yi-Lin Chuang,\n" );
    printf( "      Meng-Kai Hsu, Sheng Chou, Tzu-Hen Lin and Yao-Wen Chang\n" );
    printf( "      All Rights Reserved\n\n" );
    printf( "      THIS PROGRAM IS FOR ACADEMIC USE ONLY AND \n" );
    printf( "      CANNOT BE USED FOR ANY COMMERCIAL PURPOSE.\n" );
    printf( "\n\n" );
    printf( "Build User: %s\n", __BUILD_USER__ );
    printf( "Build Host: %s\n", __BUILD_HOST__ );
    printf( "Build Arch: %s\n", __BUILD_ARCH__ );
    printf( "Build Date: %s\n", __BUILD_DATE__ );
    fflush( stdout );
    if( bBuildInfo )
    {
	cout << "CXX=" << __CXX_VERSION__ << endl;
	cout << "CXXFLAGS=" << __CXXFLAGS__ <<endl;
	cout << "LDFLAGS=" << __LDFLAGS__ <<endl;
    }
    

    ShowSystemInfo();  // 2005-12-26
   
    if( argc < 2 )
    {
	printUsage();
	return 0;
    }

    CPlaceDB placedb;
    
    //param.seed = (unsigned)time(NULL);
    param.seed = 1;

    if( !handleArgument( argc, argv, param ) )
	return -1;

    //Added by Jin 20090929
    //if( gArg.CheckExist( "nlp" ) == true )
    //{
    param.bUseLSE = true; // default
    //gArg.Override( "WAE", "" );
    //}

    if( gArg.CheckExist( "MRT" ) == true ) // kaie
    {
	param.bUseMacroRT = true;
	gArg.Override( "BestOrient", "" );
	bRunMincut = false;
	gArg.Override( "nlp", "" );
	//param.bRunInit = false;
    }else
    {
    	//gArg.Override( "weightWire", "2.0" );
	//gArg.Override( "step", "0.2" );
	param.weightWire = 2.0;
	param.step = 0.2;
	param.stepAssigned = true;
    }

    srand( param.seed );
 
    // 2006-04-27 donnie
    if( param.outFilePrefixAssigned == false )
    {
	if( param.plFilename != "" )
	{
	    param.outFilePrefix = param.plFilename;
	    int len = param.outFilePrefix.length();
	    if( param.outFilePrefix.substr( len-3, 3 ) == ".pl" )
		param.outFilePrefix = param.outFilePrefix.erase( len-3, 3 );
	}
	else
	{
	    param.outFilePrefix = argv[2]; 
	    int len = param.outFilePrefix.length();
	    if( param.outFilePrefix.substr( len-4, 4 ) == ".aux" )
	    {
		param.outFilePrefix = param.outFilePrefix.erase( len-4, 4 );
	    } 
	    else if( param.outFilePrefix.substr( len-4, 4 ) == ".lef" )
	    {
		param.outFilePrefix = argv[3]; // use DEF file
		len = param.outFilePrefix.length();
		param.outFilePrefix = param.outFilePrefix.erase( len-4, 4 );
	    }
	}
    
	// find the basename
	string::size_type pos = param.outFilePrefix.rfind( "/" );
	if( pos != string::npos )
	    param.outFilePrefix = param.outFilePrefix.substr( pos+1 );
	printf( "Output prefix: %s\n", param.outFilePrefix.c_str() );
    }
   

    // 2007-02-13 (donnie)
    if( gArg.IsDev() && gArg.CheckExist( "groute" ) )
    {
	GRouteMap gMap( &placedb );
	string routing, res;
	gArg.GetString( "groute", &routing );
	gArg.GetString( "gres", &res );
	gMap.LoadBoxRouterRes( routing.c_str(), res.c_str() );
	string filename = param.outFilePrefix + ".gmap"; 
	gMap.OutputGnuplotFigure( filename.c_str() );
	return 0;
    }

    // 2007-04-10 (donnie)
    if( gArg.IsDev() && gArg.CheckExist( "gr" ) )
    {
	GRouteMap gMap( &placedb );
	string routing, res;
	gArg.GetString( "gr", &routing );
	gArg.GetString( "res", &res );
	gMap.LoadISPD2007Res( routing.c_str(), res.c_str() );
	string filename = param.outFilePrefix + ".gmap"; 
	gMap.OutputGnuplotFigure( filename.c_str() );
	return 0;
    }


    if( param.bShow )
	param.Print();

    ////////////////////////////////////////////////////////////////
    // Read files
    ////////////////////////////////////////////////////////////////
    double read_time_start = clock();
    if( strcmp( argv[1]+1, "aux" ) == 0 )
    {
	// bookshelf 
	printf( "Use BOOKSHELF placement format\n" );
	
	string filename = argv[2];
	string::size_type pos = filename.rfind( "/" );
	if( pos != string::npos )
	{
	    printf( "    Path = %s\n", filename.substr( 0, pos+1 ).c_str() );
	    gArg.Override( "path", filename.substr( 0, pos+1 ) );
	}
	
	CParserIBMMix parserIBM;
	if( param.nodesFilename.length() > 0 )
	    parserIBM.ReadFile( argv[2], placedb, param.nodesFilename.c_str() );
	else
	    parserIBM.ReadFile( argv[2], placedb );
	//placedb.CheckStdCellOrient(); // comment by kaie

#if 0
	// 2007-03-29 (donnie) Create pseudo LEF data
	const int nLayer = 5;
	placedb.m_pLEF = &parserLEFDEF;
        placedb.m_pLEF->m_metalDir.resize( nLayer + 1 );   // 5 layers (1 to 5)
	placedb.m_pLEF->m_metalDir[0] = CParserLEFDEF::VERTICAL;	
	placedb.m_pLEF->m_metalDir[1] = CParserLEFDEF::HORIZONTAL;	
	placedb.m_pLEF->m_metalDir[2] = CParserLEFDEF::VERTICAL;
	placedb.m_pLEF->m_metalDir[3] = CParserLEFDEF::HORIZONTAL;	
	placedb.m_pLEF->m_metalDir[4] = CParserLEFDEF::VERTICAL;
	placedb.m_pLEF->m_metalDir[5] = CParserLEFDEF::HORIZONTAL;	
	//placedb.m_pLEF->m_metalDir[6] = CParserLEFDEF::VERTICAL;
	placedb.m_pLEF->m_lefUnit = 1.0;
        placedb.m_pLEF->m_metalPitch.resize( nLayer + 1 );
	for( unsigned int i=0; i<nLayer+1; i++ )
	    placedb.m_pLEF->m_metalPitch[i] = 1.0;
#endif
    }
    else if( strcmp( argv[1]+1, "lefdef" ) == 0 )
    {
	// lef/def
	printf( "Use LEF/DEF format\n" );
	parserLEFDEF.ReadLEF( argv[2], placedb  );
	//parserLEFDEF.PrintMacros( false );  // no pins
	parserLEFDEF.ReadDEF( argv[3], placedb  );
	placedb.SetCoreRegion();

	//placedb.PrintModules();
	//placedb.PrintNets();

#if 1	
	//test code

	//Added by Jin 20081013	
	if( gArg.CheckExist( "timing" ) )
	{
	    cout << "********timing mode" << endl;
	    
	    //change all names into lower case
	    for( unsigned int i = 0 ; i < placedb.m_modules.size() ; i++ )
	    {
		string& name = placedb.m_modules[i].m_name;

		for( unsigned int j = 0 ; j < name.size() ; j++ )
		{
		    if( name[j] >= 'A' && name[j] <= 'Z' )
		    {
			name[j] = tolower( name[j] );
		    }
		}
	    }
	    for( unsigned int i = 0 ; i < placedb.m_netsName.size() ; i++ )
	    {
		string& name = placedb.m_netsName[i];

		for( unsigned int j = 0 ; j < name.size() ; j++ )
		{
		    if( name[j] >= 'A' && name[j] <= 'Z' )
		    {
			name[j] = tolower( name[j] );
		    }
		}
	    }
	    
	    //create module and net name maps for timing driven placement
	    printf( "Creating module and net name maps\n" );
	    placedb.CreateModuleNameMap();
	    placedb.CreateNetNameMap();

	    TimingOptimizer to( placedb );
	    string timing_filename;

	    gArg.GetString( "timing", &timing_filename );

	    to.ReadTiming( timing_filename );

	    exit(0);
	    //placedb.m_pTimingAnalysis = new TimingAnalysis( placedb );
	    //placedb.m_pTimingAnalysis->Initialize();
	    //TimingAnalysis ta( placedb );
	    //placedb.m_pTimingAnalysis->ConstructNetlist();
	}
	    /*
	for( unsigned int i = 0 ; i < placedb.m_modules.size() ; i++ )
	{
	    if( placedb.m_modules[i].m_lefCellId < parserLEFDEF.m_modules.size() )
	    {
		cout << placedb.m_modules[i].m_name << " " << parserLEFDEF.m_modules[ placedb.m_modules[i].m_lefCellId ].m_name << endl;
	    }
	    else
	    {
		cout << placedb.m_modules[i].m_name << " " << placedb.m_modules[i].m_blockType << endl;
	    }	
	
	    if( placedb.m_modules[i].m_blockType == BT_PI )
	    {
		cout << placedb.m_modules[i].m_name << " is PI" << endl;
	    }

	    else if( placedb.m_modules[i].m_blockType == BT_PO )
	    {
		cout << placedb.m_modules[i].m_name << " is PO" << endl;
	    }
	}
	    */
	//@test code
#endif
    }
    else
    {
	printUsage();
	return 0;
    }
    printf( "Circuit imported (CPU time: %.0f sec)\n\n", (clock()-read_time_start)/CLOCKS_PER_SEC );
    

#if 0    
    if( param.libertyFilename != "" )
    {
	//CLibDB libDB;
	Liberty libParser1;
	FILE* reportFile = stdout;
	libParser1.parseLib( param.libertyFilename.c_str(), reportFile );
	libParser1.check( reportFile );

	if( strcmp( argv[1]+1, "aux" ) == 0 )
	{
	    placedb.ConnectLiberty( &libParser1 );
	}	   
	if( strcmp( argv[1]+1, "lefdef" ) == 0 )
	{
	    placedb.ConnectLiberty( &libParser1 );
	}

	placedb.IdentifyDFF();
	CTimeMachine tm( placedb );
	tm.CriticalPathMethod();
	
    }
#endif

    ///////////////////////////////////////////////////////
    // Load PL from file (2005-12-06 by donnie)
    ///////////////////////////////////////////////////////
    if( param.plFilename != "" )
    {
	CParserIBMMix parserIBM;
	placedb.CreateModuleNameMap();
	gArg.RemoveOverride( "path" );
	parserIBM.ReadPLFile( param.plFilename.c_str(), placedb );
	placedb.ClearModuleNameMap();
    }
    
   
    // 2007-07-23 (donnie)
    if( gArg.CheckExist( "sizing" ) )
	CPlaceUtil::SimulateGateSizing( &placedb );
    
    placedb.Init(); // set member variables, module member variables, "isOutCore"
    
    if( /*gArg.IsDev() && */gArg.CheckExist( "allmacromove" ) )
    {
	printf( "\n[Set All Macro Movable]\n" );
	placedb.SetAllBlockMovable();
// 	placedb.OutputPL( "allmove.pl", true);
// 	placedb.OutputNodes(  "allmove.nodes", true );
    }

    if( !gArg.CheckExist("loadpl") )
    {
	double coreCenterX = (placedb.m_coreRgn.left + placedb.m_coreRgn.right ) * 0.5;
	double coreCenterY = (placedb.m_coreRgn.top + placedb.m_coreRgn.bottom ) * 0.5;

	for( unsigned int i = 0; i < placedb.m_modules.size(); i++ )
	{
	    if( !placedb.m_modules[i].m_isFixed &&
	    	 fabs(placedb.m_modules[i].m_x) < 1e-5 && fabs(placedb.m_modules[i].m_y) < 1e-5 )
	    	placedb.MoveModuleCenter(i, coreCenterX, coreCenterY);
	}
    }

    // 2006-09-23 donnie
    if( gArg.IsDev() && gArg.CheckExist( "newUtil" ) )
    {
    	double x1 = placedb.m_coreRgn.left;
	double y1 = placedb.m_coreRgn.bottom;
	double w1 = placedb.m_coreRgn.right - placedb.m_coreRgn.left;
	double h1 = placedb.m_coreRgn.top - placedb.m_coreRgn.bottom;

	double util = 1.0;
	gArg.GetDouble( "newUtil", &util );
	CPlaceUtil::CreateRowSites( &placedb, util, 1.0 );
	char name[256];
	sprintf(name, "%s.scl", param.outFilePrefix.c_str()/*, util*/);
	placedb.OutputSCL( name );

	double x2 = placedb.m_coreRgn.left;
	double y2 = placedb.m_coreRgn.bottom;
	double w2 = placedb.m_coreRgn.right - placedb.m_coreRgn.left;
	double h2 = placedb.m_coreRgn.top - placedb.m_coreRgn.bottom;

	// Adjust fixed block positions
	double xScale = w2 / w1;
	double yScale = h2 / h1;
	printf("xScale = %.2f, yScale = %.2f\n", xScale, yScale);

	for(unsigned int i = 0; i < placedb.m_modules.size(); i++)
	{
	    if( placedb.m_modules[i].m_isFixed )
	    {
	    	double xShift = placedb.m_modules[i].m_x - x1;
		double yShift = placedb.m_modules[i].m_y - y1;
		double newX = floor(x2 + xShift * xScale);
		double newY = floor(y2 + yShift * yScale);

		placedb.SetModuleLocation(i, newX, newY);
	    }
	}
	sprintf(name, "%s.pl", param.outFilePrefix.c_str()/*, util*/);
	placedb.OutputPL( name );

    }
    
    // 2005-12-17
    placedb.ShowDBInfo();
    if( gArg.CheckExist( "over" ) )
	CPlaceUtil::GetTotalOverlapArea( placedb ); // 2007-07-22 (donnie)
     
    // 2006-04-26 donnie (for IBM benchmarks)
    if( placedb.m_util > 0.79 )
    {
	if( param.stepAssigned == false )
	{
	    if( param.bShow )
		printf( "**** Set step size to 0.2\n" );
	    param.step = 0.2;
	}
	//if( param.cellMatchingStopAssigned == false )
	if( gArg.CheckExist( "stop" ) == false )
	{
	    if( param.bShow )
		printf( "**** Set cell matching stop to 0.02\n" );
	    //param.cellMatchingStop = 0.02;
	    gArg.Override( "stop", "0.02" );
	}
	if( param.bWireModelParameterAssigned == false )
	{
	    if( param.bUseLSE )
	    {
		if( param.bShow )
		    printf( "**** Set p to 100\n" );
		param.dLpNorm_P = 100;    // 100 for IBM, 300 for ISPD05/06
	    }
	    else
	    {
		if( param.bShow )
		    printf( "**** Set p to 150\n" );
		param.dLpNorm_P = 150;    // 150 for IBM, 300 for ISPD05/06
	    }
	}
    }

   

    if( gArg.IsDev() && gArg.CheckExist( "checkorient" ) )
    {
	printf( "\n[Check Orientations]\n" );
	CPlaceFlipping::CheckCellOrientation( placedb );
    }

    if( gArg.IsDev() && gArg.CheckExist( "nooffset" ) )
    {
	printf( "\n[Remove Pin Offsets]\n" );
	CPlaceFlipping::RemoveCellPinOffset( placedb );
    }
    
    
    //////// PLOTTER ////////////////////////
    
    if( bOutInterPL )
    {
	string name = param.outFilePrefix + ".init";

	if( param.bShow )	
	    placedb.OutputGnuplotFigureWithZoom( name.c_str(), false, true, true, true, true);
	else
	{
	    name = param.outFilePrefix + ".init.plt";
	    placedb.OutputGnuplotFigure( name.c_str(), false );
	}
	
	if( param.bShow && gArg.CheckExist( "fig" ) )
	{
	    // 2006-04-26
	    CPlaceBin placeBin( placedb );
	    // (donnie) Note that we could not know the bin size factor here. (inside NLP placer)
	    placeBin.CreateGrid( static_cast<int>( /*param.binSize **/ sqrt( placedb.m_modules.size() ) ), static_cast<int>( /*param.binSize **/ sqrt( placedb.m_modules.size() ) ) );
	    name = param.outFilePrefix + ".init.density";
	    placeBin.OutputBinUtil( name );
	}

	if( param.bShow && (gArg.CheckExist( "cong" ) || gArg.CheckExist( "drawCongData" )  ) )
	{
	    printf( "[INIT] Generating Congestion Map\n" );
	    int gridNum = static_cast<int>( sqrt( static_cast<double>(placedb.m_modules.size()) ) * 0.8 );

	    printf( "[CONG] Grid %d by %d\n", gridNum, gridNum );

	    CCongMap congmap( gridNum, gridNum, placedb.m_coreRgn, placedb.m_pLEF );
	    CGlobalRouting groute( placedb );
	    groute.ProbalisticMethod( congmap );
	    string filename = param.outFilePrefix + ".init.overflow"; 
	    congmap.OutputBinOverflowFigure( filename.c_str() );
	    filename = param.outFilePrefix + ".init.totalnet"; 
	    congmap.OutputBinTotalLengthFigure( filename.c_str() );
	    printf( "[CONG] Total overflow: %d\n", (int)congmap.GetTotalOverflow() );
	    printf( "[CONG] Maximum congestion: %.2f (V: %.2f H: %.2f)\n", 
		    congmap.GetMaxCongestion(),
		    congmap.GetMaxVCutCongestion(),
		    congmap.GetMaxHCutCongestion() );
	    printf( "[CONG] # VCut layers: %d # HCut layers: %d\n", 
		    congmap.GetNumVCutLayer(), congmap.GetNumHCutLayer() );
	    printf( "[CONG] Max length in a bin: %f\n", congmap.GetMaxBinTotalLength() );
	}

	if( bPlotOnly )
	{
	    if( param.bShow )
	    {
		string script = param.outFilePrefix + ".script";
		placedb.OutputAstroDump( script.c_str() );
	    }
	//Start:=====================(indark)==========================

// 		for(unsigned int i = 0 ; i < placedb.m_modules.size() ; i++){
// 			if (!placedb.m_modules[i].m_isOutCore){
// 				placedb.m_modules[i].m_isFixed = false;
// 			}
// 		}
// 		string file = param.outFilePrefix + ".mac.nodes";
// 		placedb.OutputNodes(file.c_str(),true);
	//End:=====================(indark)==========================
	    return 0;
	}
	
	//if( strcmp( argv[1], "-lefdef" ) == 0 )
	//    parserLEFDEF.WriteDEF( argv[3], "init.def", placedb );
    }

    //////// Bookshelf Converter ////////////

    if( outBookshelf != "" )
    {
	placedb.OutputBookshelf( outBookshelf.c_str(), param.setOutOrientN );
	return 0;
    }
    if( outBookshelfNoBlockSite != "" )
    {
	placedb.RemoveFixedBlockSite();
	placedb.OutputBookshelf( outBookshelfNoBlockSite.c_str(), param.setOutOrientN );
	return 0;
    }

    //===== debugging ======
     //placedb.PrintNets();
     //placedb.PrintModules();
    //======================

     //placedb.AdjustCoordinate();

     //placedb.SetCoreRegion();
     //placedb.ShowRows();
     //cout << "SITE INFO:WIDTH=" << parserLEFDEF.m_coreSiteWidth * parserLEFDEF.m_defUnit
     //		    << "'" << parserLEFDEF.m_coreSiteHeight  * parserLEFDEF.m_defUnit << endl;
     //placedb.CheckRowHeight(parserLEFDEF.m_coreSiteHeight  * parserLEFDEF.m_defUnit );

#if 0
     // donnie test
     placedb.IdentifyDFF();
     placedb.InitializeBlockInOut();
     //for( unsigned int i=0; i<placedb.m_nets.size(); i++ )
     //	 printf( "net %d cap = %f\n", i, placedb.GetNetLoad(i) );
     CTimeMachine::CriticalPathMethod( placedb );
#endif

     
    // MP-tree macro placement using the input as a global placement
    int mpt_ratio = 25;
    gArg.GetInt("mpt_ratio",&mpt_ratio );
    if( param.bRunMPTreeLegal ) // donnie 2006-07-09
    {
	CMPTreeLegal mptLegal( &placedb );
	mptLegal.Init( mpt_ratio, 0 );

	if( !gArg.CheckExist( "skipmptree" ) )
	    mptLegal.Optimize();
	mptLegal.Update( true ); // setFixed

	if( gArg.CheckExist( "mptreeonly" ) )
	{
	    string name = param.outFilePrefix + "-mpt";
	    bool setOutOrientN = false;
	    placedb.OutputNodes( (name + ".nodes").c_str(), setOutOrientN );
	    placedb.OutputPL( (name + ".pl").c_str(), setOutOrientN );
	    placedb.OutputGnuplotFigure( (name + ".plt").c_str(), false );
	    return 0;
	}
    
    
    
    
    }

    if (gArg.CheckExist( "XDP")){
	    vector<int> legaled_moduleID;
	    CMacroLegal mlegal(placedb, param.n_MacroRowHeight,50);
	    mlegal.Legalize(legaled_moduleID);
	    string name = param.outFilePrefix + "-xdp";
	    bool setOutOrientN = false;
	    placedb.OutputNodes( (name + ".nodes").c_str(), setOutOrientN );
	    placedb.OutputPL( (name + ".pl").c_str(), setOutOrientN );
	    placedb.OutputGnuplotFigureWithZoom( (name + ".plt").c_str(), false, true, false, true , true );
    	    exit(0);
    }
    
    if (gArg.CheckExist( "MacroSP")){
    	double MP_start_time = seconds();
	
	CMacroSP SP0(placedb);
	SP0.Place();
	string name = param.outFilePrefix + "-macrosp";
	bool setOutOrientN = false;
	placedb.OutputNodes( (name + ".nodes").c_str(), setOutOrientN );
	placedb.OutputPL( (name + ".pl").c_str(), setOutOrientN );
	placedb.OutputGnuplotFigureWithZoom( (name + ".plt").c_str(), false, true, false, true , true );
	if(gArg.CheckExist("MacroSPOnly"))
	    exit(0);
	
	double MP_time = seconds() - MP_start_time;
	printf(" Macro Placement Time: %f\n", MP_time);
    }
    
    //Added by Jin 20061003
    //Test for decompositing all nets to two-pin nets
    CSteinerDecomposition* pDec = NULL;
    if( gArg.IsDev() && gArg.CheckExist( "decom" ) )
    {
	//test code
	pDec = new CSteinerDecomposition( placedb );
	fprintf( stderr, "orig # mod %d # net %d # pin %d\n", 
		(int)placedb.m_modules.size(), (int)placedb.m_nets.size(), (int)placedb.m_pins.size() );
	fprintf( stderr, "SteinerWL: %.2f\n", placedb.GetTotalSteinerWL( 0, 0 ) );
	pDec->Update();
	fprintf( stderr, "new  # mod %d # net %d # pin %d\n", 
		(int)placedb.m_modules.size(), (int)placedb.m_nets.size(), (int)placedb.m_pins.size() );
	fprintf( stderr, "SteinerWL: %.2f\n", placedb.GetTotalSteinerWL( 0, 0 ) );
	pDec->Update();
	fprintf( stderr, "new2 # mod %d # net %d # pin %d\n", 
		(int)placedb.m_modules.size(), (int)placedb.m_nets.size(), (int)placedb.m_pins.size() );
	fprintf( stderr, "SteinerWL: %.2f\n", placedb.GetTotalSteinerWL( 0, 0 ) );
	//sDec.Restore();
	//fprintf( stdout, "rest # mod %d # net %d # pin %d\n", 
	//	    placedb.m_modules.size(), placedb.m_nets.size(), placedb.m_pins.size() );
	//@test code 
    }
    //@test code 
	
    if( gArg.CheckExist( "spreading" ) )
    {
	printf( "LOCAL SPREADING\n" );
	globalLocalSpreading( &placedb, param.endDensity );
	string file = param.outFilePrefix + ".spreading.plt";
	placedb.OutputGnuplotFigure( file.c_str(), false );
	file = param.outFilePrefix + ".spreading.pl";
	placedb.OutputPL( file.c_str() );
    }

    if( gArg.IsDev() && gArg.CheckExist( "ilr" ) )
    {
	placedb.m_modules_bak = placedb.m_modules;
	placedb.m_modules_bak_best = placedb.m_modules;

	CGreedyRefinement ilr( &placedb );

	double sec = seconds();

	double len = 4 * placedb.m_rowHeight;
	int size = (int)( (placedb.m_coreRgn.top - placedb.m_coreRgn.bottom) / len );
	
	gArg.GetInt( "ilrsize", &size );
	while( len > placedb.m_rowHeight * 2 )
	{
	    ilr.CreateGrid( size, size );
	    ilr.RefineBlocks();

	    len -= placedb.m_rowHeight;// / 2;
	    size = (int)( (placedb.m_coreRgn.top - placedb.m_coreRgn.bottom) / len );
	}

	printf( "ILR time %.2f\n", seconds() - sec );
	string file = param.outFilePrefix + ".ilr.plt";
	placedb.OutputGnuplotFigure( file.c_str(), true );
    }

    
    ////////////////////////////////////////////////////////////////
    // Global Placement
    ////////////////////////////////////////////////////////////////
 
    if( gArg.CheckExist( "test" ) )
    {
	printf( "[AftGP] Generating Congestion Map\n" );
	int gridNum = static_cast<int>( sqrt( static_cast<double>(placedb.m_modules.size()) ) * 0.8 );

	printf( "[CONG] Grid %d by %d\n", gridNum, gridNum );
	while( true )
	{
	    CCongMap congmap( gridNum, gridNum, placedb.m_coreRgn, placedb.m_pLEF );
	    CGlobalRouting groute( placedb );
	    groute.ProbalisticMethod( congmap );
	}
    }

    if( gArg.CheckExist("MRT") )
    {
    	int num_fixed_module = 0;
    	for( unsigned int i = 0; i < placedb.m_modules.size(); i++ )
	{
	    if( placedb.m_modules[i].m_isFixed == true && placedb.BlockOutCore(i) == false )
	    	num_fixed_module += 1;
	}
	if( num_fixed_module == 0 )
	{
	    if( placedb.m_util > 0.85 )
	    	;
	    else if( placedb.m_util > 0.79 )
	    	gArg.Override( "MacroAreaRatio", "125" );
	    else if( placedb.m_util > 0.78 )
	    	gArg.Override( "MacroAreaRatio", "25" );
	    else if( placedb.m_util > 0.75 )
	    	gArg.Override( "MacroAreaRatio", "100" );
	    else if( placedb.m_util > 0.71 )
	    	gArg.Override( "MacroAreaRatio", "50" );
	    else if( placedb.m_util > 0.70 )
	    	gArg.Override( "MacroAreaRatio", "75" );
	    else if( placedb.m_util > 0.65 )
	    	gArg.Override( "MacroAreaRatio", "150" );
	}
    }
   
    double part_time_start;    
    double total_part_time = 0;
    bool isLegal = false; 
    double wl1 = 0; // gp-wire
    if( false == gArg.CheckExist( "noglobal" ) )
    {
	part_time_start = seconds();

	printf( "\n[STAGE 1]: Global Placement (MEM= %.0f MB)\n", GetPeakMemoryUsage() );

	// (kaie) Fix macros
	if(gArg.CheckExist("fixmacro"))
	{
		double m_bMacroAreaRatio = 25;
		gArg.GetDouble("fixmacro", &m_bMacroAreaRatio);
		placedb.SetMacroFixed(m_bMacroAreaRatio);
	}
	
	int number = placedb.CreateDummyFixedBlock();	// site filler
	printf( "Add %d fixed blocks\n", number );
	placedb.RemoveFixedBlockSite();

	/*if (bRunMincut)
	{

	    if( param.coreUtil >= 0.97 )
		param.coreUtil = 0.97;

	    CMinCutPlacer  *pPlacer = new CMinCutPlacer(placedb);
	    CMinCutPlacer  &placer = *pPlacer;
	    placer.param = param;

	    // min-cut placer
	    placer.ShrinkCore(); // shrink core region according to the placer.parameters
	    placer.Init();

	    cout << "\n%%% INIT %%%\n" ;
	    cout << "INIT: Pin-to-pin HPWL= " << placedb.CalcHPWL() << "\n";

	    if( bOutInterPL )
	    {
		string file = placer.param.outFilePrefix + "_gp_init.plt";
		placedb.OutputGnuplotFigure( file.c_str(), false );
	    }

	    placer.RecursivePartition();	    // global placement

	    if( param.bShow )
	    {
		cout << "\n ctype1: " << param.n_ctype_count[1] << endl;
		cout << " ctype2: " << param.n_ctype_count[2] << endl;
		cout << " ctype3: " << param.n_ctype_count[3] << endl;
		cout << " ctype4: " << param.n_ctype_count[4] << endl;
		cout << " ctype5: " << param.n_ctype_count[5] << endl;
	    }

	    placer.RestoreCoreRgnShrink();  // 2006-06-19 (donnie) restore the original core
	    param = placer.param;   // record param.n_ctype_count[]

	    // TODO: restore the old core region

	    delete pPlacer; 
	    pPlacer = NULL; 
	}
	else*/
	{

	    double xMin = placedb.m_coreRgn.left;
	    double yMin = placedb.m_coreRgn.bottom;
	    for( unsigned int i=0; i<placedb.m_modules.size(); i++ )
		if( placedb.m_modules[i].m_isFixed )
		{
		    if( placedb.m_modules[i].m_cx < xMin )
			xMin = placedb.m_modules[i].m_cx;
		    if( placedb.m_modules[i].m_cy < yMin )
			yMin = placedb.m_modules[i].m_cy;
		}
	    for( unsigned int i=0; i<placedb.m_pins.size(); i++ )
	    {
		int modId = placedb.m_pins[i].moduleId;
		if( placedb.m_modules[modId].m_isFixed )
		{
		    if( placedb.m_pins[i].absX < xMin )
			xMin = placedb.m_pins[i].absX;
		    if( placedb.m_pins[i].absY < yMin )
			yMin = placedb.m_pins[i].absY;
		}
	    }
	    if( param.bUseLSE )
	    {
		//xMin -= placedb.m_rowHeight;
		//yMin -= placedb.m_rowHeight;
		xMin = (placedb.m_coreRgn.right + placedb.m_coreRgn.left) * 0.5;
		yMin = (placedb.m_coreRgn.top + placedb.m_coreRgn.bottom) * 0.5;
	    }
	    else
	    {
		// 2006-06-30 (donnie)
		// min padding to create a safe range to avoid overflow for lp-norm 
		xMin -= (placedb.m_coreRgn.right - placedb.m_coreRgn.left) * 0.02;
		yMin -= (placedb.m_coreRgn.top - placedb.m_coreRgn.bottom) * 0.02;
	    }

	    xMin = floor( xMin );
	    yMin = floor( yMin );
	    double xShift = -xMin;
	    double yShift = -yMin;
	    if( param.bShow )
	    {
		printf( " shift (%.0f %.0f)\n", xShift, yShift );
	    }

	    CPlaceDBScaling::XShift( placedb, xShift );
	    CPlaceDBScaling::YShift( placedb, yShift );

	    isLegal = multilevel_nlp( placedb, param.outFilePrefix, gCType,   // 2006-03-18, test new clustering
		    gWeightLevelDecreaingRate );

	    CPlaceDBScaling::YShift( placedb, -yShift );
	    CPlaceDBScaling::XShift( placedb, -xShift );
	}
	total_part_time = seconds() - part_time_start;	    // end global place

	if( isLegal )
	{
	    swap( placedb.m_modules_bak, placedb.m_modules );  // plot GP
	    placedb.UpdatePinPosition();
	    wl1 = placedb.CalcHPWL();	// record GP wire
	    
	    //2007-06-18 Brian
	    if (true == param.bDrawNetCongMap)
	    {
	        int gridNum = static_cast<int>( sqrt( static_cast<double>(placedb.m_modules.size()) ) * 0.8 );
    	    CPlaceBin placeBin( placedb );
    	    placeBin.CreateGridNet( gridNum );
    	    string name = param.outFilePrefix + ".gp.NetCongMap";
    	    placeBin.OutputCongMap( name );    
	    }
	    //@2007-06-18 Brian
	}

	// (kaie) 2009-06-25
	//placedb.RestoreFixedBlocks();
	
	cout << "output gp\n";
	string file = param.outFilePrefix + ".gp.plt";
	placedb.OutputGnuplotFigure( file.c_str(), false );
	file = param.outFilePrefix + ".gp.pl";
	placedb.OutputPL( file.c_str() );
	if( strcmp( argv[1], "-lefdef" ) == 0 )
	{
	    string file = param.outFilePrefix + ".gp.def";
	    parserLEFDEF.WriteDEF( argv[3], file.c_str(), placedb );
	}

	if( gArg.IsDev() )
	{
	    //string file2 = param.outFilePrefix + ".gp.nodes";
	    //placedb.OutputNodes( file2.c_str() );
	    placedb.ShowDensityInfo();
	}

	if(isLegal)
	{
	    swap( placedb.m_modules_bak, placedb.m_modules ); // restore LG
	    placedb.UpdatePinPosition();
	}
	
	printf( "\nGLOBAL: CPU = %.0f sec = %.1f min\n", total_part_time, total_part_time/60.0 );
	printf( "GLOBAL: Pin-to-pin HPWL = %.0f\n", placedb.CalcHPWL() );

#if 0
	if( gArg.IsDev() )
	{
	    double xwire = placedb.CalcXHPWL();
	    printf( "GLOBAL: XHPWL =  %.0f (%g)\n", xwire, xwire );
	}
#endif
	
	if( gArg.CheckExist( "cong" ) || gArg.CheckExist( "drawCongData" ) )
	{
	    printf( "[AftGP] Generating Congestion Map\n" );
	    int gridNum = static_cast<int>( sqrt( static_cast<double>(placedb.m_modules.size()) ) * 0.8 );

	    printf( "[CONG] Grid %d by %d\n", gridNum, gridNum );

	    CCongMap congmap( gridNum, gridNum, placedb.m_coreRgn, placedb.m_pLEF );
	    CGlobalRouting groute( placedb );
	    groute.ProbalisticMethod( congmap );
	    string filename = param.outFilePrefix + ".gp.overflow"; 
	    congmap.OutputBinOverflowFigure( filename.c_str() );
	    filename = param.outFilePrefix + ".gp.totalnet"; 
	    congmap.OutputBinTotalLengthFigure( filename.c_str() );
	    printf( "[CONG] Total overflow: %d\n", (int)congmap.GetTotalOverflow() );
	    printf( "[CONG] Maximum congestion: %.2f (V: %.2f H: %.2f)\n", 
		    congmap.GetMaxCongestion(),
		    congmap.GetMaxVCutCongestion(),
		    congmap.GetMaxHCutCongestion() );
	    printf( "[CONG] # VCut layers: %d # HCut layers: %d\n", 
		    congmap.GetNumVCutLayer(), congmap.GetNumHCutLayer() );
	    printf( "[CONG] Max length in a bin: %f\n", congmap.GetMaxBinTotalLength() );
	}
    } //GP ended
    if( !isLegal )
    {
	    wl1 = placedb.CalcHPWL();
	    
	    //2007-06-18 Brian
	    if (true == param.bDrawNetCongMap)
	    {
	        int gridNum = static_cast<int>( sqrt( static_cast<double>(placedb.m_modules.size()) ) * 0.8 );
        	CPlaceBin placeBin( placedb );
        	placeBin.CreateGridNet( gridNum );
        	string name = param.outFilePrefix + ".gp.NetCongMap";
        	placeBin.OutputCongMap( name );    
	    }
	    //@2007-06-18 Brian
    }
    
   
    ////////////////////////////////////////////////////////////////////////
    //Macro Lefalization
    ////////////////////////////////////////////////////////////////////////
//    double time_LPML = seconds();

    /*{ // (kaie)
	CMacroSP SP0(placedb);
	SP0.Place();
	placedb.OutputGnuplotFigureWithZoom( "test.sp.plt", false, true, false, true , true );
	exit(0);
    }*/
    
    //if(placer.param.bRunMPTreeLegal)
    //	placedb.RemoveFixedBlockSite();

#if 0
    bool bMLFail = false;
    if (bRunMacroLegal )
    {  
	placedb.m_modules_bak = placedb.m_modules;
	vector<int> legaled_moduleID;
	CMacroLegal* mlegal[10];
	mlegal[0]  = new CMacroLegal(placedb, param.n_MacroRowHeight,nMacroLegalRun);
	bMLFail = mlegal[0]->Legalize(legaled_moduleID);
	printf("LP-Macro Legalization Time : %6.1f s\n",seconds() - time_LPML);
	// 	    mlegal[0]->ApplyPlaceDB(true);
	// 	    placedb.RemoveFixedBlockSite();
	// 	for(unsigned int i = 0 ; i < 6 ; i++){
	// 		
	// 		mlegal[i]->Legalize();
	// 		delete mlegal[i];
	// 	}

	//placer.ShrinkCore(); // shrink core region according to the placer.parameters
	if (bMLFail)
	{
	    placedb.m_modules = placedb.m_modules_bak;
	}
	else
	{
	    if( bOutInterPL )
	    {
		string file = param.outFilePrefix + "_macrolegal.pl";
		string file2 = param.outFilePrefix + "_macrolegal.nodes";
		placedb.OutputPL( file.c_str() );
		//placedb.OutputNodes(file2.c_str(),false);
	    }
	    if( param.bPlot )
	    {
		string file = param.outFilePrefix + "_macrolegal.plt";
		placedb.OutputGnuplotFigure( file.c_str(), true );
	    }
	}


	//re initialize
	//placedb.RestoreCoreRgn();
	// 	delete pPlacer;
	// 	pPlacer = new CMinCutPlacer(placedb);
	// 	placer = *pPlacer;
	// 	
	// 	// min-cut placer
	// 	 placer.Init();
	// 
	// 	    cout << "\n%%% INIT %%%\n" ;
	// 	    cout << "INIT: Pin-to-pin HPWL= " << placedb.CalcHPWL() << "\n";
	// 
	// 	    if( bOutInterPL )
	// 	    {
	// 		string file = placer.param.outFilePrefix + "_global_init.plt";
	// 		placedb.OutputGnuplotFigure( file.c_str(), false );
	// 	    }
	// 
	// 	    placer.RecursivePartition();	    // global placement
	// 	
	// 	    file = placer.param.outFilePrefix + "_global-ml.plt";
	// 	    placedb.OutputGnuplotFigure( file.c_str(), false );
	// 	

    }
#endif

    //Test for decompositing all nets to two-pin nets
    if( gArg.IsDev() && gArg.CheckExist( "decom" ) )
    {
	//test code
	pDec->Restore();
	fprintf( stdout, "rest # mod %d # net %d # pin %d\n", 
		(int)placedb.m_modules.size(), (int)placedb.m_nets.size(), (int)placedb.m_pins.size() );
	placedb.ShowDBInfo();
	//@test code
    }
    
    
    //White space allocation
    if( gArg.IsDev() && gArg.CheckExist( "wsaTestAftGP" ) )
    {
	double target_util = placedb.m_util;
	double real_util;
	if( gArg.CheckExist( "util" ) )
	{
	    gArg.GetDouble( "util", &target_util );
	    real_util = target_util;
	}

	CPlaceBin pBin1( placedb );
	pBin1.CreateGrid( placedb.m_rowHeight * 10.0, placedb.m_rowHeight * 10.0 );
	fprintf( stdout, "density penalty before wsa %f\n", pBin1.GetPenalty( real_util ) );
	fflush( stdout );
	fprintf( stdout, "HPWL= %.0f DHPWL= %.0f\n",
		placedb.GetHPWLp2p(), placedb.GetHPWLdensity( real_util ) );
	fflush( stdout );
	//fprintf( stdout, "HPWL= %.0f DHPWL= %.0f\n", placedb.GetHPWLp2p(), placedb.GetHPWLdensity( target_util ) );
	
	if( gArg.CheckExist( "wsaUtil" ) )
	    gArg.GetDouble( "wsaUtil", &target_util );
	
	if( gArg.CheckExist( "wsaOnce" ) )
	{
	    pBin1.OutputBinUtil( "orig_util_once" );
	    placedb.OutputGnuplotFigureWithZoom( "orig_once", false, false, true );
	}
	else if( gArg.CheckExist( "wsaBest" ) )
	{
	    pBin1.OutputBinUtil( "orig_util_best" );
	    placedb.OutputGnuplotFigureWithZoom( "orig_best", false, false, true );
	}
	else
	{
	    pBin1.OutputBinUtil( "orig_util_iterative" );
	    placedb.OutputGnuplotFigureWithZoom( "orig_iterative", false, false, true );
	}
	
	int gridNum = pBin1.GetBinNumberH() * pBin1.GetBinNumberW(); 
	
	int level = 1;
	int wsa_grid_num = 1;
	while( wsa_grid_num < gridNum )
	{
	    level++;
	    wsa_grid_num = wsa_grid_num * 2;
	}

	if( !gArg.CheckExist( "wsaFindGrid" ) )
	{
	    level--;
	    wsa_grid_num = wsa_grid_num / 2;
	}

	if( gArg.CheckExist( "wsaLevel" ) )
	{
	    gArg.GetInt( "wsaLevel", &level );
	    wsa_grid_num = static_cast<int>(pow( 2.0, static_cast<double>(level) ));
	}
	
	fprintf( stdout, "White space allocation level: %d, wsa target util: %.2f\n", level, target_util );
	fprintf( stdout, "# density grids: %d # wsa grids: %d\n", gridNum, wsa_grid_num );
	fflush( stdout );

	
	WhiteSpaceAllocation wsa( placedb, level );
	if( gArg.CheckExist( "wsaOnce" ) )
	    wsa.SolveDensity( target_util );
	else if( gArg.CheckExist( "wsaBest" ) )
	    wsa.SolveDensityBest( target_util );
	else
	    wsa.SolveDensityIteratively( target_util );
	//wsa.SolveRoutability();
	//fprintf( stdout, "HPWL= %.0f DHPWL= %.0f\n", placedb.GetHPWLp2p(), placedb.GetHPWLdensity( target_util ) );

	fprintf( stdout, "End of wsa\n" );	
	fflush( stdout );
	CPlaceBin pBin2( placedb );
	pBin2.CreateGrid( placedb.m_rowHeight * 10.0, placedb.m_rowHeight * 10.0 );
	fprintf( stdout, "density penalty after wsa %f ", pBin2.GetPenalty( real_util ) );
	fprintf( stdout, "HPWL= %.0f DHPWL= %.0f\n",
		placedb.GetHPWLp2p(), placedb.GetHPWLdensity( real_util ) );
	fflush( stdout );
	
	if( gArg.CheckExist( "wsaOnce" ) )
	{
	    pBin2.OutputBinUtil( "after_util_once" );
	    placedb.OutputGnuplotFigureWithZoom( "after_once", false, false, true );
	}
	else if( gArg.CheckExist( "wsaBest" ) )
	{
	    pBin2.OutputBinUtil( "after_util_best" );
	    placedb.OutputGnuplotFigureWithZoom( "after_best", false, false, true );
	}
	else
	{
	    pBin2.OutputBinUtil( "after_util_iterative" );
	    placedb.OutputGnuplotFigureWithZoom( "after_iterative", false, false, true );
	}
    }

    //Added by Jin 20081013
    //test code	
    //if( gArg.CheckExist( "timing" ) )
    //{
    //	cout << "********timing mode after global placement" << endl;
    //
    //	placedb.m_pTimingAnalysis->RerunSTA();
	
	//delete placedb.m_pTimingAnalysis;

	//placedb.m_pTimingAnalysis = new TimingAnalysis( placedb );
	//placedb.m_pTimingAnalysis->Initialize();
	//TimingAnalysis ta( placedb );
	//placedb.m_pTimingAnalysis->ConstructNetlist();
    //}
    //@test code

    ////////////////////////////////////////////////////////////////
    // Legalization
    ////////////////////////////////////////////////////////////////
    double legal_time_start = seconds();
    double total_legal_time = 0;
    double wl2 = 1e20;
   
    vector<CPoint> beforeLegal, afterLegal;
    CPlaceUtil::SavePlacement( placedb, beforeLegal );	// for plotting movement

    if( gArg.CheckExist( "refine" ) )
    {
	printf( "LOCAL REFINEMENT\n" );
	globalRefinement( &placedb, gTargetUtil );
	string file = param.outFilePrefix + ".refine.plt";
	placedb.OutputGnuplotFigure( file.c_str(), false );
	file = param.outFilePrefix + ".refine.pl";
	placedb.OutputPL( file.c_str() );

    }

    //test code
    //placedb.OutputGnuplotFigureWithZoom( "after_global", false, false, true );
    //placedb.OutputPL( "after_global.pl" );
    //@test code

    if( !gArg.CheckExist( "nolegal" ) )	// add "!isLegal" (donnie, 2006-03-06)
    {
	if( strcmp( argv[1], "-lefdef" ) == 0 )
	    printf( "\n[STAGE 2]: Legalization (circuit = %s) MEM= %.0f MB\n\n", argv[3], GetPeakMemoryUsage() );
	else
	    printf( "\n[STAGE 2]: Legalization (circuit = %s) MEM= %.0f MB\n\n", argv[2], GetPeakMemoryUsage() );
	flush( cout );

	if( isLegal )
	   placedb.Align();  //Alignment after LAL
	else
	{
	    placedb.RemoveFixedBlockSite();
	    placedb.m_sites_for_legal = placedb.m_sites; // for BB?
	    //Start:=====================(indark)==========================
#if 1 
	    CTetrisLegal legal(placedb);
	    //test code
	    //bool bMacroShifter = legal.MacroShifter(2, false);
	    //if(bMacroShifter)
	    //{
	    //	cout << "MacroShifter success" << endl;
	    //}
	    //else
	    //{
	    //	cout << "MacroShifter fail" << endl;
	    //}
	    //legal.RestoreFreeSite();
	    //@test code

	    vector<CPoint> p2;
		
	    bool bLegal;

	    if( gArg.CheckExist( "topdownTetris" ) )
	    {
		double prelegal = 0.50;
		CTopDownLegal legal( &placedb );
		bLegal = legal.Legal( gTargetUtil, prelegal );
	    }
	    else if( gArg.IsDev() && gArg.CheckExist( "srpl" ) )
	    {
		CSingleRowPlacementLegal srpl( &placedb );
		bLegal = srpl.Legalize();
	    }
	    else
	    {
		    bLegal = legal.Solve( gTargetUtil, param.bRunMacroLegal, true, 0.8 );
		    
		    //test code
		    //placedb.OutputGnuplotFigureWithZoom( "displacement", true, true, true );
		    //Compute displacement
		    double dis_value = 0.0;
		    for( unsigned int i = 0 ; i < placedb.m_modules.size() ; i++ )
		    {
			double newx = placedb.m_modules[i].m_x;
			double newy = placedb.m_modules[i].m_y;
			double oldx = placedb.m_modules_bak[i].m_x;
			double oldy = placedb.m_modules_bak[i].m_y;
			dis_value += abs( newx - oldx ) + abs( newy - oldy );
		    }
		    fprintf( stdout, "legalization dis_value %.2f\n", dis_value );
    //test code
    //placedb.OutputGnuplotFigureWithZoom( "after_legal", false, false, true );
    //placedb.OutputPL( "after_legal.pl" );
    //@test code
		    //@test code
#if 0 
		    double d = 0.85;
		    gArg.GetDouble( "prelegal", &d );
		    bLegal = legal.Solve( gTargetUtil, param.bRunMacroLegal, true, d );
#endif
	    }
	   
	    if( gArg.IsDev() && gArg.CheckExist( "psrpl" ) )
	    {
		CPlaceUtil::SavePlacement( placedb, p2 );
		/*
		double dis = CPlaceUtil::GetDisplacement( beforeLegal, p2 );
		printf( "\n\nTOTAL DISPLACEMENT= %.0f (avg %.0f)\n\n", dis, dis/placedb.m_modules.size() );
		*/

		CPlaceUtil::ShowMigrationInfo( placedb, beforeLegal, p2 );
		
		printf( "Reduce the displacement...\n" );
		
		CSingleRowPlacementLegal srpl( &placedb );
		srpl.PostLegalSingleRowOpt( beforeLegal );
	    }

	    //if( gArg.IsDev() && gArg.CheckExist( "dis" ) )
	    {
		CPlaceUtil::SavePlacement( placedb, p2 );
		//double dis = CPlaceUtil::GetDisplacement( beforeLegal, p2 );
		//printf( "\n\nTOTAL DISPLACEMENT= %.0f (avg %.0f)\n\n", dis, dis/placedb.m_modules.size() );
		CPlaceUtil::ShowMigrationInfo( placedb, beforeLegal, p2 );
	    }
	    

	    if( !bLegal )
	    {
		cout << "Legalization failed" << endl;
		cout << "Unlegal count: " << legal.GetUnlegalCount() << endl;
		flush(cout);
		exit( 0 );  // fail to legalize, stop
	    }

#else
	    PlaceLegalize( placedb, param, wl1, wl2 );    // Old legalizer
#endif
	}

	if( gArg.IsDev() && gArg.CheckExist( "fig" ) )
	{
	    string filename;
	    CPlaceBin placeBin( placedb );
	    placeBin.CreateGrid( static_cast<int>( sqrt( placedb.m_modules.size() ) ), static_cast<int>( sqrt( placedb.m_modules.size() ) ) );
	    filename = param.outFilePrefix + ".lg.density";
	    placeBin.OutputBinUtil( filename );
	}

	//Alignment after legalization
	placedb.Align();
	total_legal_time = seconds() - legal_time_start; 
	wl2 = placedb.CalcHPWL();
	printf( "LEGAL: Pin-to-pin HPWL = %.0f (%.2f%%)\n", wl2, 100.0*(wl2/wl1-1.0) );
	fflush( stdout );

	string file = param.outFilePrefix + ".lg.plt";
	/*if( gArg.IsDev() )
	{
	    placedb.ShowDensityInfo();
	    CPlaceUtil::SavePlacement( placedb, afterLegal );
	    CPlaceUtil::LoadPlacement( placedb, beforeLegal );
	    placedb.SaveBlockLocation();
	    CPlaceUtil::LoadPlacement( placedb, afterLegal );
	    placedb.OutputGnuplotFigure( file.c_str(), true );
	}
	else*/
	    placedb.OutputGnuplotFigure( file.c_str(), false );

	file = param.outFilePrefix + ".lg.pl";
	placedb.OutputPL( file.c_str() );
	
	if( strcmp( argv[1], "-lefdef" ) == 0 )
	{
	    string file = param.outFilePrefix + ".lg.def";
	    parserLEFDEF.WriteDEF( argv[3], file.c_str(), placedb );
	}

	if( param.bHandleOrientation )  // fix cell N and S according to the site info 
	    CPlaceFlipping::CellFlipping( placedb );

	if( gArg.IsDev() && gArg.CheckExist( "nocheck" ) == false )
	{
	    printf( "[Overlap Checking]: After LG" );	
	    CCheckLegal clegal( placedb );
	    clegal.check();
	}

	// Show Steiner WL after LG
	if( gArg.IsDev() && gArg.CheckExist( "steiner" ) )
	    placedb.ShowSteinerWL();

	///////////////////////////////////////////////////
	if( gArg.CheckExist( "cong" ) || gArg.CheckExist( "drawCongData" ) )
	{
	    printf( "[CONG] (AftLG) Generating Congestion Map\n" );
	    int gridNum = static_cast<int>( sqrt( static_cast<double>(placedb.m_modules.size()) ) * 0.8 );

#if 1
	    //test code
	    if( gArg.CheckExist( "congopt" ) )
	    {
		//fprintf( stdout, "conopt: %d by %d\n", gridNum, gridNum );
		//CCongOptimizer congopt( placedb, gridNum, gridNum );
		//congopt.Observe();
	    }
	    //@test code
#endif
	    printf( "[CONG] Grid %d by %d\n", gridNum, gridNum );

	    CCongMap congmap( gridNum, gridNum, placedb.m_coreRgn, placedb.m_pLEF );
	    CGlobalRouting groute( placedb );
	    groute.ProbalisticMethod( congmap );
	    string filename = param.outFilePrefix + ".lg.overflow"; 
	    congmap.OutputBinOverflowFigure( filename.c_str() );
	    filename = param.outFilePrefix + ".lg.totalnet"; 
	    congmap.OutputBinTotalLengthFigure( filename.c_str() );
	    printf( "[CONG] Total overflow: %d\n", (int)congmap.GetTotalOverflow() );
	    printf( "[CONG] Maximum congestion: %.2f (V: %.2f H: %.2f)\n", 
		    congmap.GetMaxCongestion(),
		    congmap.GetMaxVCutCongestion(),
		    congmap.GetMaxHCutCongestion() );
	    printf( "[CONG] # VCut layers: %d # HCut layers: %d\n", 
		    congmap.GetNumVCutLayer(), congmap.GetNumHCutLayer() );
	    printf( "[CONG] Max length in a bin: %f\n", congmap.GetMaxBinTotalLength() );

	    //test code
	    exit(0);
	    //@test code
	}

    } // Legalization ended
    wl2 = placedb.CalcHPWL();


    ////////////////////////////////////////////////////////////////
    // Detailed Placement
    ////////////////////////////////////////////////////////////////


    double detail_time_start = seconds();
    double wl3 = 0;
    double total_detail_time = 0;

    if( gArg.CheckExist( "nodetail" ) == false )
    {
	if( strcmp( argv[1], "-lefdef" ) == 0 )
	    printf( "\n[STAGE 3]: Deatiled Placement (circuit = %s) MEM= %.0f MB\n\n", argv[3], GetPeakMemoryUsage() );
	else
	    printf( "\n[STAGE 3]: Deatiled Placement (circuit = %s) MEM= %.0f MB\n\n", argv[2], GetPeakMemoryUsage() );
	fflush( stdout );

	vector<Module> oldModules = placedb.m_modules;  // for plotting
	
	//placedb.RemoveMacroSite(); // Macro sites will be remove in dplacer

	//double HPWL1 = placedb.CalcHPWL();
	int count = 0, target = 2;
	if(gArg.CheckExist("rundetailuntil"))
	    gArg.GetInt("rundetailuntil", &target);
	
	while(count < target)
	{
	    CDetailPlacer dplacer( placedb, param, dpParam );
	    dplacer.DetailPlace();

	    if( gArg.CheckExist("BestOrient") )
	    {
	    	placedb.SetModuleOrientationBest(true, false, !gArg.CheckExist("noflip")); // macroonly, norotate, flip
	    }

	    //double HPWL2 = placedb.CalcHPWL();
	    //if(HPWL1-HPWL2 < 1.0e-10) break;
	    //else printf("HPWL = %lf\n", HPWL2);
	    //HPWL1 = HPWL2;

	    count++;
	}

	/*if(gArg.CheckExist("noflip")) // (kaie) macro rotation
	{
	    placedb.SetModuleNoFlip();
	    placedb.SetModuleOrientationBest(true, false, false); // macroonly, norotate, noflip
	}*/

	if( gArg.IsDev() )
	{
	    placedb.m_modules_bak = oldModules;
	    placedb.ShowDensityInfo();
	    string file = param.outFilePrefix + ".dp.plt";
	    placedb.OutputGnuplotFigure( file.c_str(), true );
	    if( bOutInterPL && strcmp( argv[1], "-lefdef" ) == 0 )
	    {
		string file = param.outFilePrefix + ".dp.def";
		parserLEFDEF.WriteDEF( argv[3], file.c_str(), placedb );	
	    }
	}
    }/*else
    {
	if(gArg.CheckExist("noflip")) // (kaie) macro rotation
	{
	    placedb.SetModuleNoFlip();
	    placedb.SetModuleOrientationBest(true, false, false);
	}
    }*/
    wl3 = placedb.CalcHPWL();
    total_detail_time = seconds() - detail_time_start;


    ////////////////////////////////////////////////////////////////
    // cell redistribution by tchsu
    ////////////////////////////////////////////////////////////////
    
    if( bCellRedistribution==true && gCellRedistributionTarget>0 && gCellRedistributionTarget<1.0 )
    {
	placedb.RemoveMacroSite();
	CCellmoving cm( placedb );
	cm.redistributeCell(gCellRedistributionTarget);
	if( gArg.IsDev() && gArg.CheckExist( "nocheck" ) == false )
	{	
	    printf("[Overlap Checking] After Cell Redistribution\n");
	    fflush( stdout );
	    CCheckLegal clegal( placedb );
	    clegal.check();
	}	
    }

    
    //////////////////////////////////////////////////////////////
    // cell alignment after DP
    //////////////////////////////////////////////////////////////
   
    if( !gArg.CheckExist( "noplace" ) && placedb.Align() > 0 )
	printf( " Aligned HPWL: %.0f %.0f %.0f\n", 
		placedb.CalcHPWL(), placedb.GetHPWLp2p(), placedb.GetHPWLdensity(gTargetUtil) );
#if 0
    if( param.bPlot )
    {
	string file = param.outFilePrefix + "_align.plt";	
	placedb.OutputGnuplotFigureWithMacroPin( file.c_str(), true );
    }
#endif


    ////////////////////////////////////////////////////////////////
    // overlap checking
    ////////////////////////////////////////////////////////////////

    if( gArg.IsDev() && gArg.CheckExist( "nocheck" ) == false )
    {

	printf("[Overlap Checking]: After DP");
	fflush( stdout );
	CCheckLegal clegal( placedb );
	clegal.check();
#if 0
	bool bOK = clegal.check();
	int OKIteration = 0;
	if( !bOK )
	{
	    placedb.m_sites = placedb.m_sites_for_legal;
	}

	while( !bOK && OKIteration < 10 )
	{
	    CTetrisLegal legal_remove_overlap(placedb);
	    legal_remove_overlap.RemoveOverlap();

	    placedb.Align();

	    CCheckLegal check_overlap( placedb );
	    bOK = check_overlap.check();

	    OKIteration++;
	}
#endif
    }
    
    //////////////////////////////////////////////////////////////
    // output placement result
    //////////////////////////////////////////////////////////////
    
    
    if( param.bHandleOrientation )
    {
	CPlaceFlipping::CellFlipping( placedb );
	CPlaceFlipping::CellFlipping( placedb );
    }
    
    printf( "\n" );
    string file = param.outFilePrefix + ".ntup.plt";	
    placedb.OutputGnuplotFigure( file.c_str(), false );

    if( strcmp( argv[1], "-lefdef" ) == 0 )
    {
	//test code
	cout << "before output def\n" << endl;
	cout.flush();
	//@test code
	string file = param.outFilePrefix + ".ntup.def";
	parserLEFDEF.WriteDEF( argv[3], file.c_str(), placedb );	    

	if( gArg.CheckExist( "outNetPoints" ) )
	{
	    string file_name = param.outFilePrefix + ".pts";
	    ofstream ofile( file_name.c_str() );
	    
	    if( ofile )
	    {
		for( unsigned int i = 0 ; i < placedb.m_nets.size() ; i++ )
		{
		    ofile << "##" << endl;
		    for( unsigned int j = 0 ; j < placedb.m_nets[i].size() ; j++ )
		    {
			int pin_index = placedb.m_nets[i][j];
			double x, y;
			placedb.GetPinLocation( pin_index, x, y );
			ofile << static_cast<int>(x) << " " << static_cast<int>(y) << endl;
		    }
		    ofile << "@@" << endl;
		}
	    }
	    else
	    {
		fprintf( stderr, "Warning: opening %s is failed\n", file_name.c_str() );
	    }
	    
	    ofile.close();
	}
	//test code
	cout << "after output def\n" << endl;
	cout.flush();
	//@test code
    }
    else
    {
	string file = param.outFilePrefix + ".ntup.pl";
	placedb.OutputPL( file.c_str() );
    }

    if (param.bOutAstro)
    {
	string script = param.outFilePrefix + ".script";
	placedb.OutputAstroDump( script.c_str() );
    }


    

    ///////////////////////////////////////////////////
    //Added by Jin 20060630
    if( gArg.IsDev() && 
	(gArg.CheckExist( "cong" ) || gArg.CheckExist( "drawCongData" ) ) )
    {
	printf( "Generating Congestion Map\n" );
    	int gridNum = static_cast<int>( sqrt( static_cast<double>(placedb.m_modules.size()) ) * 0.8 );
	//test code
	if( gArg.CheckExist( "congopt" ) )
	{
	    fprintf( stdout, "conopt: %d by %d\n", gridNum, gridNum );
	    CCongOptimizer congopt( placedb, gridNum, gridNum );
	    congopt.OptimizeCongestion( 50, 50, CCongMap::Right );
	}
	//@test code
	
	//int gridNum = static_cast<int>(( placedb.m_coreRgn.top - placedb.m_coreRgn.bottom ) / (placedb.m_rowHeight*1.5) );
	printf( "  Grid %d by %d\n", gridNum, gridNum );

	//test code
	if( gArg.CheckExist( "fineGrid" ) )
	    gridNum = gridNum * 4;
	//@test code
	
	CCongMap congmap( gridNum, gridNum, placedb.m_coreRgn, placedb.m_pLEF );
	CGlobalRouting groute( placedb );
	groute.ProbalisticMethod( congmap );
	string filename = param.outFilePrefix + ".ntup.overflow"; 
	congmap.OutputBinOverflowFigure( filename.c_str() );
	filename = param.outFilePrefix + ".ntup.congmap"; 
	congmap.OutputCutOverflowFigure( filename.c_str() );
	filename = param.outFilePrefix + ".ntup.localnet"; 
	congmap.OutputBinLocalNetLengthFigure( filename.c_str() );
	filename = param.outFilePrefix + ".ntup.totalnet"; 
	congmap.OutputBinTotalLengthFigure( filename.c_str() );
	printf( "  Total overflow: %d\n", (int)congmap.GetTotalOverflow() );
	printf( "  Maximum congestion: %.2f (V: %.2f H: %.2f)\n", 
		congmap.GetMaxCongestion(),
		congmap.GetMaxVCutCongestion(),
		congmap.GetMaxHCutCongestion() );
	printf( "  # VCut layers: %d # HCut layers: %d\n", 
		congmap.GetNumVCutLayer(), congmap.GetNumHCutLayer() );
	printf( "  Max length in a bin: %f\n", congmap.GetMaxBinTotalLength() );
	
	// 2007-03-28 (donnie)
	GRouteMap grouteMap( &placedb );
	grouteMap.LoadCongMap( &congmap );
	filename = param.outFilePrefix + ".ntup.cong";
	grouteMap.OutputGnuplotFigure( filename.c_str() );
	
	// 2007-03-30 (donnie)
	filename = param.outFilePrefix + ".ntup.gr";
	CPlaceUtil::WriteISPDGRCFormat( &placedb, filename.c_str() );
	
	printf( "Finished\n" );

	
    }
    //@Added by Jin 20060630
    
    
    ///////////////////////////////////////////////////
    // Show results (donnie, 2006-03-16) 
    ///////////////////////////////////////////////////
    double total_time = seconds() - programStartTime;
    
    if( strcmp( argv[1], "-lefdef" ) == 0 )
	printf( "\nCircuit: %s\n", argv[3] );
    else
	printf( "\nCircuit: %s\n", argv[2] );
    
    if( gArg.CheckExist( "over" ) )
	CPlaceUtil::GetTotalOverlapArea( placedb ); // 2007-07-22 (donnie)
    
    printf( "  Global HPWL= %.0f  Time: %6.0f sec (%.1f min)\n", wl1, total_part_time, total_part_time / 60.0 );
    if( !gArg.CheckExist( "nolegal" ) )
    printf( "   Legal HPWL= %.0f  Time: %6.0f sec (%.1f min)\n", wl2, total_legal_time, total_legal_time / 60.0 );
    if( !gArg.CheckExist( "nodetail" ) )
    printf( "  Detail HPWL= %.0f  Time: %6.0f sec (%.1f min)\n", wl3, total_detail_time, total_detail_time / 60.0 );
    printf( " ===================================================================\n" );
    printf( "         HPWL= %.0f  Time: %6.0f sec (%.1f min)\n", placedb.GetHPWLp2p(), total_time, total_time / 60.0 );
    if( gTargetUtil > 0 && gTargetUtil < 1.0 )
    printf( "        DHPWL= %.0f (util %.2f)\n", placedb.GetHPWLdensity(gTargetUtil), gTargetUtil );

    // 2006-09-22 (donnie)
    if( gArg.IsDev() && gArg.CheckExist( "steiner" ) )
	placedb.ShowSteinerWL();
    if( gArg.IsDev() && gArg.CheckExist( "xst" ) )
	placedb.ComputeSteinerWL(); 
    
    if( gArg.IsDev() )
    {
	double hpwl, xwire, ywire;
	hpwl = placedb.GetHPWL( &xwire, &ywire );
	printf( "HPWL %.0f  (x %.0f  y %.0f)\n", hpwl, xwire, ywire );
	
	if( strcmp( argv[1], "-lefdef" ) == 0 )
	    fprintf( stderr, "\nCircuit: %s\n", argv[3] );
	else
	    fprintf( stderr, "\nCircuit: %s\n", argv[2] );
	
	fprintf( stderr, "HPWL: %.0f (ctype %d)\n", placedb.GetHPWLp2p(), gCType ); 
	if( gTargetUtil > 0 && gTargetUtil < 1.0 )
	{
	    double dhpwl = placedb.GetHPWLdensity(gTargetUtil);
	    fprintf( stderr, "Penalty: %g\n", 100 * ( dhpwl / placedb.GetHPWLp2p() - 1 ) );
	    fprintf( stderr, "DHPWL: %.0f (util=%.2f)\n", dhpwl, gTargetUtil ); 
	}
	fprintf( stderr, "Total CPU: %.0f sec = %.1f min (gCPU = %.0f sec)\n\n", 
		seconds() - programStartTime, (seconds() - programStartTime)/60.0, total_part_time ); 
    }
    
    //2007-06-18 Brian
    if (true == param.bDrawNetCongMap)
    {
        int gridNum = static_cast<int>( sqrt( static_cast<double>(placedb.m_modules.size()) ) * 0.8 );
    	CPlaceBin placeBin( placedb );
    	placeBin.CreateGridNet( gridNum );
    	string name = param.outFilePrefix + ".ntup.NetCongMap";
    	placeBin.OutputCongMap( name );    
    }
    //@2007-06-18 Brian

    // 2007-02-06 (donnie)
    if( gArg.IsDev() && gArg.CheckExist( "fig" ) )
    {
	int gridSize = static_cast<int>( sqrt( placedb.m_modules.size() ) * 0.8 / 4);
	CPlaceBin placeBin( placedb );
	placeBin.CreateGrid( gridSize, gridSize );
	string name = param.outFilePrefix + ".ntup.density";
	placeBin.OutputBinUtil( name );
    }

    ////////////////////////////////////////////////////////////////
    // Log Result
    ////////////////////////////////////////////////////////////////

    //test code
    cout << "before log result" << endl;
    cout.flush();
    //@test code
    
    if( gArg.IsDev() )
    {
	
	cout << "Result logged.\n";
	FILE* out_log;
	out_log = fopen( "log_result.txt", "a" );

	if( out_log )
	{
	    struct tm *newtime;
	    time_t aclock;
	    time( &aclock );                  // Get time in seconds
	    newtime = localtime( &aclock );   // Convert time to struct tm form
	    char timeStr[100];

	    string hostName = GetHostName();

	    double m_MacroAreaRatio = 25;
	    gArg.GetDouble("MacroAreaRatio", &m_MacroAreaRatio);

	    sprintf( timeStr, "%02d/%02d %02d:%02d", 
		    newtime->tm_mon+1, newtime->tm_mday,
		    newtime->tm_hour, newtime->tm_min );
	    fprintf( out_log, "%s,%s, %10.0f, %10.0f, %10.0f, %10.0f,%5.0fs, %5.0fs, %s %s %d %s %d %.0f %.0f\n",
		    timeStr, argv[2], wl1, wl2, wl3, placedb.GetHPWLdensity( gTargetUtil ), 
		    total_time, total_part_time, 
		    param.GetParamString( bRunMincut ).c_str(), param.outFilePrefix.c_str(), gCType,
		    hostName.c_str(), param.bUseLSE, /*param.precision,*/ param.dLpNorm_P,
		    m_MacroAreaRatio);
	    fclose( out_log );
	}
	else
	{
	    cerr << "Warning: Opening log_result.txt failed\n" << endl;
	}
    }

    
    return 0;
}

