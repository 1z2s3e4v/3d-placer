#include "../PlaceDB/placedb.h"
#include "MyNLP.h"
#include "../Cluster/cluster.h"
//#include "PlaceDBQP.h"	    // SolveQP
#include "../PlaceCommon/ParamPlacement.h"
//#include "mptreelegal.h"
//#include "macrolegal.h"
#include "../PlaceCommon/randomplace.h"    
//#include "SteinerDecomposition.h"
#include "../PlaceCommon/placebin.h"
//#include "WhiteSpaceAllocation.h"
//#include "macrosp.h"
#include "../PlaceCommon/placeutil.h"

//Added by Jin 20070306
//#include "FixedPointMethod.h"
//Added by Jin 20070306

#include <cstdio>
#include <cfloat>
using namespace std;


//test code
//#include "FixedPointMethod.h"
//@test code

#include "../Legalizer/TetrisLegal.h"
using namespace Jin;

extern bool bOutInterPL;
extern bool allMacroMove;

bool multilevel_nlp( CPlaceDB& placedb,
                     int ctype,	
		     double weightLevelDecreaingRate
		   )
{
    double mlnlp_start = seconds();

    int targetBlock = 6000;
    double ratio = 5;
    int maxLevel = INT_MAX;
    
    if( param.bShow )
    {
	gArg.GetInt( "maxLevel",  &maxLevel );
	gArg.GetInt( "cblock",    &targetBlock );
	gArg.GetDouble( "cratio", &ratio );

	printf( "\n" );	
	printf( "[Multilevel Placement Parameters]\n" );
	printf( "    wWire L Decrease = %f\n", weightLevelDecreaingRate );
	printf( "    max level #      = %d\n", maxLevel );
	printf( "    cluster ratio    = %f\n", ratio );
	printf( "    cluster block    = %d\n", targetBlock );
	printf( "\n" );	
    }
    
	printf( "Set all module orient N\n" );
	for( unsigned int i = 0; i < placedb.m_modules.size(); i++ )
	    if( placedb.m_modules[i].m_isFixed == false )
	    	placedb.SetModuleOrientation( i, 0 );
   
    int levels = 0;

    // construct a hierarchy of clusters 
    vector<CPlaceDB>* p_placedb_clustered = new vector<CPlaceDB>;
    vector<CPlaceDB>& placedb_clustered = *p_placedb_clustered;
    vector<CClustering>* p_clusters = new vector<CClustering>;
    vector<CClustering>& clusters = *p_clusters;
    //CPlaceDB dummyDB;
    
    CPlaceDB* currentDB = &placedb;

    double clustering_start = seconds();
    int currentBlock = currentDB->GetMovableBlockNumber();
    int expLevel = (int)ceil( log( static_cast<double>(currentBlock) / targetBlock ) / log( ratio ) ) + 1;
    if( expLevel < 0 )
	expLevel = 0;
    if( param.bShow )
    {
	printf( "Expect level # = %d\n", expLevel );
	if( maxLevel < 200 )
	    printf( "Max level # = %d\n", maxLevel );
    }
   
    placedb_clustered.resize( expLevel+1 );
    clusters.resize( expLevel+1 );
    while( currentBlock > targetBlock && levels < maxLevel-1 && levels < expLevel )
    {
	levels++;

	printf( "level %d, block # %d, movable block # %d > %d, do clustering... (MEM= %.0f MB)", 
		levels, (int)currentDB->m_modules.size(), currentBlock, targetBlock, GetPeakMemoryUsage() );
	fflush( stdout );
	
	clusters[levels-1].clustering( 
		*currentDB, 
		placedb_clustered[levels-1], 
		(int)((currentBlock)/ratio), 
		1.5, 
		ctype );
	
	printf( " done\n" );
	fflush( stdout );
	
        currentDB = &placedb_clustered[levels-1];
	currentBlock = currentDB->GetMovableBlockNumber();	
    }
    printf( "block # %d, movable block # %d\n", (int)currentDB->m_modules.size(), currentBlock );
    printf( "Total clustering time: %.2f\n", seconds() - clustering_start );
    fflush( stdout );
    
    //currentDB->OutputGnuplotFigure( "init_2.plt", false );  // show physical clustering positions
   
    // generate level parameters 
    int totalLevels = levels + 1;
    double start_density = param.startDensity;
    double final_target_density = param.endDensity;
    double startSmoothDelta = param.startDelta;
    double finalSmoothDelta = param.endDelta;
    vector<double> levelSmoothDelta;
    vector<double> levelTargetOver;
    levelSmoothDelta.resize( totalLevels );
    levelTargetOver.resize( totalLevels );
    printf( "\n" ); 
    for( int i=0; i<levels; i++ )
    {
	int currentLevel = i+1;
	levelTargetOver[i] = start_density - ( start_density - final_target_density ) * (currentLevel) / (totalLevels);
	levelSmoothDelta[i] = startSmoothDelta - ( startSmoothDelta - finalSmoothDelta ) * (currentLevel) / (totalLevels); 

	if( param.bShow )
	    printf( "Level %d\tBlock %d\tPin %d\tDelta %.3f\tOver %.3f\n", 
		    i+1, (int)placedb_clustered[levels-i-1].m_modules.size(), 
		    (int)placedb_clustered[levels-i-1].m_pins.size(), levelSmoothDelta[i], levelTargetOver[i] );	
    }
    if( param.bShow )
	printf( "Level %d\tBlock %d\tPin %d\tDelta %.3f\tOver %.3f\n\n", 
		levels+1, (int)placedb.m_modules.size(), 
		(int)placedb.m_pins.size(),
		finalSmoothDelta, final_target_density );
    
    // init solution

    // 2006-09-13 (donnie) check if fixed block exists
    double fixedNum = 0;
    for( unsigned int i=0; i<placedb.m_modules.size(); i++ )
	if( placedb.m_modules[i].m_isFixed )
	    fixedNum += placedb.m_modules[i].m_netsId.size() ;

    if( fixedNum == 0 )
	param.bQP = false; // cannot use QP
    
    // if( !gArg.CheckExist("loadpl") && param.bRunInit )
    // {
    // 	vector<int> fixId;
	// /*for( unsigned int i = 0; i < currentDB->m_modules.size(); i++ )
	// {
	//     if( !currentDB->m_modules[i].m_isCluster )
	//     {
	//     	currentDB->m_modules[i].m_isFixed = true;
	// 	fixId.push_back(i);
	//     }
	// }*/
    // 	//if( param.bQP )
    // 	if(param.bQP || fixId.size() != 0)
	// if(param.bQP)
	// {
	// 	cout << "*************************************** frank: closed because QP not yet...\n";
	//     /*printf( "Solve QP (MEM= %.0f MB)\n", GetPeakMemoryUsage() );
	//     fflush( stdout );
	
	//     // 2006-08-30 Prevent cells too close to the boundary
	//     double coreCenterX = (currentDB->m_coreRgn.left + currentDB->m_coreRgn.right ) * 0.5;
	//     double coreCenterY = (currentDB->m_coreRgn.top + currentDB->m_coreRgn.bottom ) * 0.5;
	//     CRect oldCore = currentDB->m_coreRgn;
	//     //double coreW = (coreCenterX - currentDB->m_coreRgn.left) * 0.9;	// shirnk 10%
	//     //double coreH = (coreCenterY - currentDB->m_coreRgn.bottom) * 0.9; // shirnk 10%
	//     double coreW = (coreCenterX - currentDB->m_coreRgn.left) ;
	//     double coreH = (coreCenterY - currentDB->m_coreRgn.bottom) ;
	//     assert( coreW >= 0 );
	//     assert( coreH >= 0 );
	//     currentDB->m_coreRgn.left = coreCenterX - coreW;    
	//     currentDB->m_coreRgn.right = coreCenterX + coreW;    
	//     currentDB->m_coreRgn.bottom = coreCenterY - coreH;    
	//     currentDB->m_coreRgn.top = coreCenterY + coreH;    

	//     CPlaceDBQPPlacer* pqplace = new CPlaceDBQPPlacer( *currentDB );
	//     pqplace->QPplace();
	//     delete pqplace;
	//     pqplace = NULL;
	
	//     currentDB->m_coreRgn = oldCore;*/
    // }
	// else
	// {
	//     // random place
	//     //CRandomPlace::Place( *currentDB, 0.1 );
	//     //CRandomPlace::Place( *currentDB, 0.01 );
	//     //CRandomPlace::Place( *currentDB, 0.005 );
	//     /*MyNLP* mynlp = new MyNLP( *currentDB );
	//     mynlp->m_smoothDelta = levelSmoothDelta[0];
	//     mynlp->m_earlyStop = true;
	//     mynlp->m_lookAheadLegalization = false;
	//     mynlp->m_macroRotate = false;
	//     mynlp->m_weightedForce = false;
	//     mynlp->m_prototype = true;
	//     mynlp->MySolve( 1, levelTargetOver[0], 0 );
	//     delete mynlp;
	//     mynlp = NULL;*/
	// 	//CRandomPlace::Place( *currentDB, 0.01 );
	// }

	// // for(unsigned int i = 0; i < fixId.size(); i++)
	// //     currentDB->m_modules[fixId[i]].m_isFixed = false;
    // }
	//CPlaceDBScaling::XScale( placedb, 1.0/xScale );
    //CPlaceDBScaling::YScale( placedb, 1.0/yScale );
    
    // transform to 3d
    if( param.b3d ){
		printf("transform to 3d...\n");
		currentDB->m_totalLayer = param.nlayer;
	    //currentDB->Folding2();
	    //

		double layerThickness = (currentDB->m_front - currentDB->m_back) / (double)(currentDB->m_totalLayer);
		if(currentDB->m_totalLayer > 1 && !param.bLayerPreAssign){
			currentDB->LayerAssignmentByPartition(currentDB->m_totalLayer);
		}
                    
	    for(unsigned int i = 0; i < currentDB->m_modules.size(); i++){
			currentDB->m_modules[i].m_cz = currentDB->m_modules[i].m_z + 0.5 * layerThickness;
			//currentDB->m_modules[i].m_z = (int)(0.5 * currentDB->m_totalLayer) * layerThickness;
	    }
	    currentDB->CalcTSV();
	    //printf("#TSV: %d\n", (int)currentDB->GetTSVcount());
	    fprintf( stderr, "#TSV: %d\n", (int)currentDB->GetTSVcount());
	    
	    double core_area = (currentDB->m_coreRgn.right-currentDB->m_coreRgn.left) * (currentDB->m_coreRgn.top-currentDB->m_coreRgn.bottom);
	    vector<double> totalArea3d;
	    totalArea3d.resize(currentDB->m_totalLayer, 0.0);
	    for(unsigned int i = 0; i < currentDB->m_modules.size(); i++){
	    	totalArea3d[(int)currentDB->m_modules[i].m_z] += currentDB->m_modules[i].m_area;
		}
	    for(int l = 0; l < currentDB->m_totalLayer; l++)
		    printf("Layer%d: %.2f(%.2f/%.2f)\n", l, totalArea3d[l] / core_area, totalArea3d[l], core_area);
	    //double layerThickness = (currentDB->m_front - currentDB->m_back) / currentDB->m_totalLayer;
	    //for(unsigned int i = 0; i < currentDB->m_modules.size(); i++)
		    //currentDB->m_modules[i].m_z *= layerThickness;
    }else
	    currentDB->m_totalLayer = 1;

    if( param.bShow ) 
    {
		printf( "\n block= %d, net= %d, pin= %d, MEM= %.0f MB\n", (int)currentDB->m_modules.size(),
		(int)currentDB->m_nets.size(), (int)currentDB->m_pins.size(), GetPeakMemoryUsage() );
    }

    //currentDB->OutputGnuplotFigure( "init_solution.plt", false );  // show initial solution

    // placement on the clsutered placedb
    int currentLevel = 0;
    double weightWire = param.weightWire;
   
    double legalHeight = DBL_MAX;
    // double legalArea = DBL_MAX;
    // bool bUseMacroArea = gArg.CheckExist("MacroAreaRatio");
    // bool bMacroShifter = false;
    
    while( levels > 0 )
    {
	currentLevel ++;

	// TEST for > 2 levels
	if( currentLevel >= 2 )
	    weightWire = weightWire / weightLevelDecreaingRate;

        currentDB->RemoveFixedBlockSite(); //indark 20070411
	// 2006-06-01 (donnie) fix large blocks ///////////////////
	vector<int> setFixId;
	// if(((bUseMacroArea && legalArea < DBL_MAX) || (!bUseMacroArea && legalHeight < DBL_MAX)) && bMacroShifter)
	// {
	//     if( !gArg.CheckExist("noFixMacro") )
	//     {
	//     	for( unsigned int i=0; i<currentDB->m_modules.size(); i++ )
	//     	{
	// 	    if( currentDB->m_modules[i].m_isFixed == false &&
	// 	    	( (!bUseMacroArea && currentDB->m_modules[i].m_height > legalHeight ) ||
	// 		  (bUseMacroArea && currentDB->m_modules[i].m_height > currentDB->m_rowHeight &&
	// 				    currentDB->m_modules[i].m_area > legalArea) ) )
	// 	    {
	// 	    	setFixId.push_back( i );
	// 	    	currentDB->m_modules[i].m_isFixed = true;
	// 	    }
	//     	}
	//     }
	//     if( param.bShow )
	//     {
	// 	if(!bUseMacroArea)
	// 	    printf( "LEVEL %d  Height= %f  Fix #= %d\n", currentLevel, legalHeight, (int)setFixId.size() );
	// 	else
	// 	    printf( "LEVEL %d  Area= %f  Fix #= %d\n", currentLevel, legalArea, (int)setFixId.size() );
	//     }
	// }
	if( legalHeight < DBL_MAX )
	{
	    for( unsigned int i=0; i<currentDB->m_modules.size(); i++ )
	    {
		if( currentDB->m_modules[i].m_isFixed == false && currentDB->m_modules[i].m_height > legalHeight )
		{
		    setFixId.push_back( i );
		    currentDB->m_modules[i].m_isFixed = true;
		}
	    }
	    if( param.bShow )
		printf( "LEVEL %d  Height= %f  Fix #= %d\n", currentLevel, legalHeight, setFixId.size() );
	}
	
	MyNLP* mynlp = new MyNLP( *currentDB );
	mynlp->m_smoothDelta = levelSmoothDelta[currentLevel-1];
	////mynlp->m_useBellPotentialForPreplaced = true;
        mynlp->m_earlyStop = true;	
	mynlp->m_lookAheadLegalization = false;
	// mynlp->m_macroRotate = false; // removed in 3d but don't know why yet
	// mynlp->m_prototype = true; // removed in 3d but don't know why yet
	// if( param.bUseMacroRT ) mynlp->m_macroRotate = true; // (kaie) macro rotating // removed in 3d but don't know why yet
	// mynlp->m_weightedForce = false; // removed in 3d but don't know why yet
	// if( gArg.CheckExist("wf1") ) mynlp->m_weightedForce = true; // removed in 3d but don't know why yet
	if( param.bShow )
	{
	    printf( "LEVEL %d of %d (target overflow %f, smooth delta %f) (%.0fMB)\n", 
		    currentLevel, totalLevels, levelTargetOver[currentLevel-1], levelSmoothDelta[currentLevel-1],
		    GetPeakMemoryUsage() );	
	    printf( "[block = %d, net = %d, pin = %d]\n", (int)currentDB->m_modules.size(),
		    (int)currentDB->m_nets.size(), (int)currentDB->m_pins.size() );
	}

	mynlp->MySolve( weightWire, levelTargetOver[currentLevel-1], 
		currentLevel );
	delete mynlp;
	mynlp = NULL;

	// 2006-06-01 (donnie) Remove fixed blocks
	for( unsigned i=0; i<setFixId.size(); i++ )
	    currentDB->m_modules[ setFixId[i] ].m_isFixed = false;
	
	// Macro shifter
	double avgHeight = 0;
	//double avgArea = 0;
	int count = 0;
	for( unsigned int i=0; i<currentDB->m_modules.size(); i++ )
	{
	    if( currentDB->m_modules[i].m_isFixed )
		continue;
	    count++;
	    avgHeight += currentDB->m_modules[i].m_height;
	    //avgArea += currentDB->m_modules[i].m_area;
	}
	avgHeight /= count;
	//avgArea /= count;
	
	//==================(indark)=============//
	// Multilevel MPTree
	char file[200];
 	currentDB->SaveBlockLocation();
#if 0
	if (param.bRunMPTreeLegal){
		CMPTreeLegal mplegal0( currentDB);
		mplegal0.Init(25,currentLevel);
		mplegal0.Update(true);
		sprintf( file, "%s_MPTree_move_%d.plt", outFilePrefix.c_str(), currentLevel );
		currentDB->OutputGnuplotFigure( file, true );
	}else 
#endif
	// kaie 20100121 macro rotation
	/*if(gArg.CheckExist("BestOrient"))
	{
	    currentDB->SetModuleOrientationBest(true, true, gArg.CheckExist("mflip")); // macroonly, rotate, flip
	}*/
	// @kaie 20100121

	// if (param.bRunMacroLegal && currentLevel == 1){
	//     vector<int> legaled_moduleID;
	//     CMacroLegal* mlegal;
	//     mlegal = new CMacroLegal(*currentDB, param.n_MacroRowHeight,50);
	//     /*bool bMLFail =*/ mlegal->Legalize(legaled_moduleID);
	//     sprintf( file, "%s_ML_move_%d.plt", outFilePrefix.c_str(), currentLevel );
	//     currentDB->OutputGnuplotFigure( file, true );
    //         currentDB->RemoveFixedBlockSite(); //indark
	//     delete mlegal; // kaie
	//     mlegal = NULL; // kaie
	// }
	// else if (param.bRunMacroShifter){

	//     currentDB->RemoveFixedBlockSite();
	//     CTetrisLegal* legal = new CTetrisLegal( *currentDB );
		
	//     double m_MacroAreaRatio = 25;
	//     gArg.GetDouble("MacroAreaRatio", &m_MacroAreaRatio);
	//     //legalHeight = avgHeight * 4;
	//     legalHeight = avgHeight * 5;
	//     //legalArea = currentDB->m_totalModuleArea / (double)currentDB->m_modules.size() * m_MacroAreaRatio;
	//     legalArea = avgArea * m_MacroAreaRatio;
	//     //bool bMacroShifter;
	
	//     if(!bUseMacroArea)
	// 	bMacroShifter = legal->MacroShifter( legalHeight / currentDB->m_rowHeight, false );
	//     else
	// 	bMacroShifter = legal->MacroShifter( legalArea / currentDB->m_rowHeight, false );
		
	//     if(!bMacroShifter)
	//     {
	// 	/*if(!bUseMacroArea)
	// 	    bMacroShifter = legal.RobustMacroShifter( legalHeight / currentDB->m_rowHeight, false);
	// 	else
	// 	    bMacroShifter = legal.RobustMacroShifter( legalArea / currentDB->m_rowHeight, false);*/
	// 	vector<int> legaled_moduleID;
	// 	CMacroLegal* mlegal = new CMacroLegal(*currentDB, legalHeight / currentDB->m_rowHeight ,50);
	// 	bMacroShifter = mlegal->Legalize(legaled_moduleID);
	// 	delete mlegal; // kaie
	// 	mlegal = NULL; // kaie
	//     }

	//     if( param.bShow )
	//     {
	// 	if( false == bMacroShifter )
	// 	    printf( "MACRO SHIFTER FAILED!\n" );
	// 	else
	// 	    printf( "MACRO SHIFTER SUCCEEDED!\n" );
	//     }
	//     delete legal; // kaie
	//     legal = NULL; // kaie
	// }
	
	double tUsed = seconds() - mlnlp_start;
	printf( "\n########### [TotalTime = %.2f sec = %.2f min] ###########\n\n", tUsed, tUsed / 60.0 );
	if( levels > 1 )
	{
	    clusters[levels-1].declustering( placedb_clustered[levels-1], placedb_clustered[levels-2] );
	    
	    clusters.resize( levels-1 ); // donnie
	    placedb_clustered.resize( levels-1 );
	    
	    currentDB = &placedb_clustered[levels-2];
	}
	else	// == 1
	{
	    // to top level
	    clusters[levels-1].declustering( placedb_clustered[levels-1], placedb );
	    clusters.clear();
	    delete p_clusters;
	    //p_clusters = NULL;
	    delete p_placedb_clustered;
	    //p_placedb_clustered = NULL;
	    break;
	}	    
	levels--;
    }

    // if(param.bRunMacroShifter)
    // {
	// placedb.RemoveFixedBlockSite();
	// CTetrisLegal* legal = new CTetrisLegal( placedb );
	// //bool bMacroShifter;
	// if(!bUseMacroArea)
	//     bMacroShifter = legal->MacroShifter( legalHeight / placedb.m_rowHeight, false );
	// else
	//     bMacroShifter = legal->MacroShifter( legalArea / placedb.m_rowHeight, false );

	// // if(!bMacroShifter)
	// // {
    // //     vector<int> legaled_moduleID;
    // //     CMacroLegal* mlegal = new CMacroLegal(placedb, legalHeight / placedb.m_rowHeight ,50);
    // //     bMacroShifter = mlegal->Legalize(legaled_moduleID);
	// //     delete mlegal;
	// //     mlegal = NULL;
	// // }

	// if( param.bShow )
	// {
	//     if( false == bMacroShifter )
	// 	printf( "MACRO SHIFTER FAILED!\n" );
	//     else
	// 	printf( "MACRO SHIFTER SUCCEEDED!\n" );
	// }
	// delete legal;
	// legal = NULL;
    // }

    // 2006-06-01 (donnie) fix large blocks ///////////////////
    vector<int> setFixId;
    // if(bMacroShifter)
    // {
	// //placedb.OutputGnuplotFigure( "MacroShifterTest.plt", false );
    // 	//if( !gArg.CheckExist("noFixMacro") )
	// //{
	//     for( unsigned int i=0; i<placedb.m_modules.size(); i++ )
    // 	    {
	//     	if( placedb.m_modules[i].m_isFixed == false
	//     	    && ( (!bUseMacroArea && placedb.m_modules[i].m_height > legalHeight)
	//     	    || (bUseMacroArea &&  placedb.m_modules[i].m_height > placedb.m_rowHeight && placedb.m_modules[i].m_area > legalArea) )
	//     	){
	//     	    setFixId.push_back( i );
	//     	    placedb.m_modules[i].m_isFixed = true;

	//     	    // fix it.
	//     	    // dHPWL is not correct
	//         }
    // 	    }
	// //}    
    // 	if( param.bShow )
    // 	{
	//     if(!bUseMacroArea)
	//     	printf( "LEVEL %d  Height = %f  Fix # = %d\n", currentLevel+1, legalHeight, (int)setFixId.size() );
	//     else
	//     	printf( "LEVEL %d  Area = %f  Fix # = %d\n", currentLevel+1, legalArea, (int)setFixId.size() );
    // 	}
    // }
	for( unsigned int i=0; i<placedb.m_modules.size(); i++ )
    {
	if( placedb.m_modules[i].m_isFixed == false && placedb.m_modules[i].m_height > legalHeight )
	{
	    setFixId.push_back( i );
	    placedb.m_modules[i].m_isFixed = true;

	    // fix it.
	    // dHPWL is not correct
	}
    }
	if( param.bShow )
    {
	printf( "LEVEL %d  Height = %f  Fix # = %d\n", currentLevel+1, legalHeight, setFixId.size() );
    }
    ////////////////////////////////////////////////////////////

    if( param.bShow )
    {
	printf( "LEVEL %d of %d (target density %f, smooth delta %d)(%.0fMB)\n", 
		currentLevel+1, totalLevels, final_target_density, 1, GetPeakMemoryUsage() );	
    }
    
    if( param.topDblStep )  // 2006-03-25
	param.step *= 2.0;
	
    if( gArg.CheckExist( "notop" ) )
	return false;

    MyNLP* mynlp = new MyNLP( placedb );
	////mynlp->m_useBellPotentialForPreplaced = true;
    mynlp->m_topLevel = true;
    mynlp->m_lookAheadLegalization = true;
    // mynlp->m_macroRotate = false; // removed in 3d but don't know why yet
    // mynlp->m_prototype = false; // removed in 3d but don't know why yet
    // //if(gArg.CheckExist("MRT2")) mynlp->m_macroRotate = true; // (kaie) macro rotating // removed in 3d but don't know why yet
    // mynlp->m_weightedForce = false; // removed in 3d but don't know why yet
    // if( gArg.CheckExist("wf2") ) mynlp->m_weightedForce = true; // removed in 3d but don't know why yet
    mynlp->m_smoothDelta = param.endDelta;
    bool isLegal = mynlp->MySolve( weightWire, param.endDensity, currentLevel+1 );

    //isLegal &= bMacroShifter;
    delete mynlp;
    mynlp = NULL;

    // 2006-06-01 (donnie) Remove fixed blocks
    for( unsigned i=0; i<setFixId.size(); i++ )
	placedb.m_modules[setFixId[i]].m_isFixed = false;

    // kaie macro rotation
    //if(gArg.CheckExist("BestOrient"))
	//placedb.SetModuleOrientationBest(true, false, !gArg.CheckExist("noflip")); // macroonly, norotate, flip

    return isLegal;
}

void globalLocalSpreading( CPlaceDB* pDB, double targetDensity )
{
    double weightDensity = 1.0;
    gArg.GetDouble( "weightDensity", &weightDensity );
    MyNLP* mynlp = new MyNLP( *pDB );
	////mynlp->m_useBellPotentialForPreplaced = true;
    mynlp->m_topLevel = true;
    mynlp->m_lookAheadLegalization = false;
    mynlp->m_smoothDelta = param.endDelta;
    int itePrefix = 7;
    printf( "LS: target over %g\n", targetDensity );
    printf( "LS: weight density %g\n", weightDensity );
    mynlp->MySolve( weightDensity, targetDensity, itePrefix );
    delete mynlp;
    mynlp = NULL;
}

void globalRefinement( CPlaceDB* pDB, double targetDensity )
{
    double weightWire = 0.01;
    double skew = 16;
    gArg.GetDouble( "refineWireWeight", &weightWire );
    gArg.GetDouble( "refineSkew", &skew );
    
    MyNLP* mynlp = new MyNLP( *pDB );
	////mynlp->m_useBellPotentialForPreplaced = true;
    mynlp->m_topLevel = true;
    mynlp->m_lookAheadLegalization = false;
    mynlp->m_smoothDelta = param.endDelta;
    mynlp->m_maxIte = 1;	
	////mynlp->m_skewDensityPenalty1 = skew;
    ////mynlp->m_skewDensityPenalty2 = skew;   
    int itePrefix = 8;
    printf( "LF: target over  %g\n", targetDensity );
    printf( "LF: wire weight  %g\n", weightWire );
    printf( "LF: penalty skew %g\n", skew );
    mynlp->MySolve( weightWire, targetDensity, itePrefix );
    delete mynlp;
    mynlp = NULL;
}

