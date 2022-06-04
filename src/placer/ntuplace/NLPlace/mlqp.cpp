#include "placedb.h"
#include "MyQP.h"
#include "cluster.h"
#include "PlaceDBQP.h"	    // SolveQP
#include "ParamPlacement.h"
#include "macrolegal.h"
#include "randomplace.h"    
#include "mlqp.h"

#include <cstdio>
using namespace std;


#include "TetrisLegal.h"
using namespace Jin;

extern bool bOutInterPL;
extern bool allMacroMove;

bool multilevel_qp( CPlaceDB& placedb, string outFilePrefix,
                     int ctype,	

		     double incFactor,	// multiplier inc rate
		     double wWire,	// wireWeight
		     //int maxLevel,
		     double weightLevelDecreaingRate//,
		   )
{
    double mlnlp_start = seconds();
    //double target_utilization = 1.0;

    int targetBlock = param.clusterBlock;
    double ratio = param.clusterRatio;
    double target_utilization = param.coreUtil;
    int maxLevel = 100;
    
    if( param.bShow )
    {
	    printf( "   inc factor    = %f\n", incFactor );
	    printf( "   wWire         = %f\n", wWire );
	    printf( "   wWire L Decrease = %f\n", weightLevelDecreaingRate );
	    printf( "   target util   = %f\n", target_utilization );
    }
    
//     printf( "Set all module orient N\n" );
//     for( unsigned int i=0; i< placedb.m_modules.size(); i++ )
// 	if( placedb.m_modules[i].m_isFixed == false )
// 	    placedb.SetModuleOrientation( i, 0 );
   
    int levels = 0;

    // construct a hierarchy of clusters 
    vector<CPlaceDB>* p_placedb_clustered = new vector<CPlaceDB>;
    vector<CPlaceDB>& placedb_clustered = *p_placedb_clustered;
    vector<CClustering>* p_clusters = new vector<CClustering>;
    vector<CClustering>& clusters = *p_clusters;
    //CPlaceDB dummyDB;
    
    CPlaceDB* currentDB = &placedb;
    //int currentBlock = currentDB->m_modules.size();
    
    //currentDB->OutputGnuplotFigure( "init_1.plt", false );
 
    //============================================================
    // add dummy blocks
    int fillNumber = 0;  
    currentDB->Init();	// calculate movable block area	/ free space
    double dummyArea = ( currentDB->m_totalMovableModuleArea / currentDB->m_totalMovableModuleNumber );
    dummyArea *= 25; // make it larger
    double dummyWidth = sqrt( dummyArea );
    double fillUtil = 0.8;
    double fillArea = ( currentDB->m_totalFreeSpace * fillUtil - currentDB->m_totalMovableModuleArea );
    fillNumber = static_cast<int>( fillArea / dummyArea );

    fillNumber = 0; // do not fill dummy blocks
    
    for( int i=0; i<fillNumber; i++ )
    {
	string name = "dummy";
	currentDB->AddModule( name, dummyWidth, dummyWidth, false );
	int id = (int)currentDB->m_modules.size() - 1;
	currentDB->m_modules[id].m_isDummy = true;
	
	double w = currentDB->m_coreRgn.right - currentDB->m_coreRgn.left;
	double h = currentDB->m_coreRgn.top - currentDB->m_coreRgn.bottom;
	w *= 0.8;
	h *= 0.8;
	currentDB->MoveModuleCenter( id, 
		currentDB->m_coreRgn.left + w*0.1 + rand() % (int)w, 
		currentDB->m_coreRgn.bottom + h*0.1 + rand() % (int)h ); 
    }
    if( param.bShow )
	printf( "Fill number = %d\n", fillNumber );
    currentDB->Init();
    //============================================================

    double clustering_start = seconds();
    int currentBlock = currentDB->GetMovableBlockNumber();
    int expLevel = (int)ceil( log( static_cast<double>(currentBlock-fillNumber) / targetBlock ) / log( ratio ) ) + 1;
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
    while( currentBlock-fillNumber > targetBlock && levels < maxLevel-1 )
    {
	    levels++;
	    //placedb_clustered.resize( levels+1 );
    	
	    printf( "level %d, block # %d, movable block # %d > %d, do clustering...", 
		    levels, (int)currentDB->m_modules.size(), currentBlock, targetBlock );
	    fflush( stdout );
    	

	    clusters[levels-1].clustering( *currentDB, placedb_clustered[levels-1], (int)((currentBlock-fillNumber)/ratio), 1.5, ctype );
    	
	    printf( " done\n" );
	    fflush( stdout );
    	
            currentDB = &placedb_clustered[levels-1];
	    currentBlock = currentDB->GetMovableBlockNumber();	
    }
    printf( "block # %d, movable block # %d\n", (int)currentDB->m_modules.size(), currentBlock );
    printf( "clustering time: %.2f\n", seconds() - clustering_start );
   
    
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
#if 1
    if( param.bQP )
    {
	    printf( "Solve QP\n" );
	    CPlaceDBQPPlacer* pqplace = new CPlaceDBQPPlacer( *currentDB );
	    pqplace->QPplace();
	    delete pqplace;
	    pqplace = NULL;
    }
    else
    {
	    // random place
	    CRandomPlace::Place( *currentDB, 0.1 );
    }
    //fixed pin move
    //if(param.isFPF==true)
    //{
    //    MacroInit initMacro(*currentDB);
    //    initMacro.init();
    //    initMacro.solve();
    //}
#endif
    
    if( bOutInterPL )
    {
	    string file = outFilePrefix + "_global_init.plt";
	    currentDB->OutputGnuplotFigure( file.c_str(), false );

    }
  
    if( param.bShow ) 
    {
	    printf( "\n block = %d, net = %d, pin = %d\n", (int)currentDB->m_modules.size(),
		    (int)currentDB->m_nets.size(), (int)currentDB->m_pins.size() );
	    currentDB->OutputGnuplotFigure( "init_qp.plt", false );
    }

    // spread dummy cells
    if( fillNumber > 0 )
    {
	    for( unsigned int i=0; i<currentDB->m_modules.size(); i++ )
	    {
	        if( currentDB->m_modules[i].m_isDummy == true )
	        {
		    double w = currentDB->m_coreRgn.right - currentDB->m_coreRgn.left;
		    double h = currentDB->m_coreRgn.top - currentDB->m_coreRgn.bottom;
		    w *= 0.8;
		    h *= 0.8;
		    currentDB->MoveModuleCenter( i, 
			    currentDB->m_coreRgn.left + w*0.1 + rand() % (int)w, 
			    currentDB->m_coreRgn.bottom + h*0.1 + rand() % (int)h ); 
	        }
	    }
    }

    

    // placement on the clsutered placedb
    int currentLevel = 0;
    double weightWire = wWire;
    
    while( levels > 0 )
    {
	    currentLevel ++;

	    // TEST for > 2 levels
	    if( currentLevel >= 2 )
	        weightWire = weightWire / weightLevelDecreaingRate;

	    MyQP* myqp = new MyQP( *currentDB );
    	
	    bool noRelaxSmooth = false;
      
	    myqp->m_smoothDelta = levelSmoothDelta[currentLevel-1];
	    myqp->m_useBellPotentialForPreplaced = true;
            myqp->m_earlyStop = true;	
	    myqp->m_lookAheadLegalization = false;
	    myqp->m_incFactor  = incFactor;
	    myqp->m_targetUtil = target_utilization;
	    if( param.bShow )
	    {
	        printf( "LEVEL %d of %d (target overflow %f, smooth delta %f)(%.0fMB)\n", 
		        currentLevel, totalLevels, levelTargetOver[currentLevel-1], levelSmoothDelta[currentLevel-1],
		        GetPeakMemoryUsage() );	
	        printf( "[block = %d, net = %d, pin = %d]\n", (int)currentDB->m_modules.size(),
		        (int)currentDB->m_nets.size(), (int)currentDB->m_pins.size() );
	    }
    	
	    // 2006-03-26
	    /*if( currentLevel == 1 )
	    {
	        double oldStep = param.step;
	        param.step = 0.1;
	        myqp->MySolve( weightWire, levelTargetOver[currentLevel-1], currentLevel, noRelaxSmooth );
	        param.step = oldStep;
	    }
	    else	*/
	        myqp->MySolve( weightWire, levelTargetOver[currentLevel-1], currentLevel, noRelaxSmooth );
	    delete myqp;
	    myqp = NULL;

    	
	    // Macro shifter
	    double avgHeight = 0;
	    int count = 0;
	    for( unsigned int i=0; i<currentDB->m_modules.size(); i++ )
	    {
	        if( currentDB->m_modules[i].m_isFixed )
		    continue;
	        count++;
	        avgHeight += currentDB->m_modules[i].m_height;
	    }

	    avgHeight /= count;

        //cout<<"\nStart GRoute!!";
    	
        //double ttt1=seconds();
        //MyGlobalRouter mr(*currentDB,currentDB->m_rowHeight,currentDB->m_rowHeight);
        //mr.initResource();
        //mr.route();
        //mr.OutputHGrid("Hgrid.dat");
        //mr.OutputVGrid("Vgrid.dat");
        //mr.outputfig("routingPlt");

        //cout<<"\nTotal Route Time:"<<seconds()-ttt1<<" seconds.\n";
        //exit(0);
    	
	    // TODO: remove dummy blocks

    	
	    if( bOutInterPL )
	    {
	        char file[200];
	        sprintf( file, "%s_global_level_%d.plt", outFilePrefix.c_str(), currentLevel );
	        currentDB->OutputGnuplotFigure( file, false );
	    }

	    double tUsed = seconds() - mlnlp_start;
	    printf( "\n########### [TotalTime = %.2f sec = %.2f min] ###########\n\n", tUsed, tUsed / 60.0 );
	    if( levels > 1 )
	    {
	        clusters[levels-1].declustering( placedb_clustered[levels-1], placedb_clustered[levels-2] );
    	    
	        clusters.resize( levels-1 ); // donnie
	        placedb_clustered.resize( levels-1 );
    	    
	        currentDB = &placedb_clustered[levels-2];
	        //printf( "\n ************************\n" );
	    }
	    else	// == 1
	    {
	        // to top level
	        clusters[levels-1].declustering( placedb_clustered[levels-1], placedb );
	        clusters.clear();
	        //printf( "\n ******* top level ******\n" );
    	    
	        delete p_clusters;
	        delete p_placedb_clustered;
    	    
	        break;
	    }	    
	    levels--;

    }


    if( param.bShow )
    {
	printf( "LEVEL %d of %d (target density %f, smooth delta %d)(%.0fMB)\n", 
		currentLevel+1, totalLevels, final_target_density, 1, GetPeakMemoryUsage() );	
    }
    
    if( param.topDblStep )  // 2006-03-25
	param.step *= 2.0;

    MyQP* myqp = new MyQP( placedb );
    myqp->m_useBellPotentialForPreplaced = true;
    myqp->m_topLevel = true;
    myqp->m_lookAheadLegalization = true;
    myqp->m_incFactor  = incFactor;
    myqp->m_targetUtil = target_utilization;
    //myqp->m_smoothDelta = finalSmoothDelta;
    myqp->m_smoothDelta = param.endDelta;
    //bool isLegal = myqp->MySolve( weightWire, final_target_density, currentLevel+1, true );
    bool isLegal = myqp->MySolve( weightWire, param.endDensity, currentLevel+1, true );
    delete myqp;
    myqp = NULL;

    if( fillNumber > 0 )
    {
	placedb.m_modules.resize( placedb.m_modules.size() - fillNumber );
    }
    
        cout<<"\nStart GRoute!!";
    	
        double ttt1=seconds();
        MyGlobalRouter mr(placedb,placedb.m_rowHeight,placedb.m_rowHeight);
        mr.initResource();
        mr.route();
        mr.OutputHGrid("Hgrid.dat");
        mr.OutputVGrid("Vgrid.dat");
        mr.outputfig("routingPlt");

        cout<<"\nTotal Route Time:"<<seconds()-ttt1<<" seconds.\n";
        //exit(0);
    return isLegal;
}

mlqp::mlqp( CPlaceDB& placedb, string outFilePrefix, int ctype,  double wWire, double weightLevelDecreaingRate)
{

    if(param.bShow)
        cout<<"\nWelcome to mlQP!!";
    m_weightLevelDecreaingRate=weightLevelDecreaingRate;
    //m_incFactor=incFactor;
    m_outFilePrefix=outFilePrefix;

    m_mlqp_start = seconds();
    //double target_utilization = 1.0;

    int targetBlock = 6000;
    double ratio = 5;
    double target_utilization = 1;
    int maxLevel = 100;

    gArg.GetDouble( "util",   &target_utilization );
	gArg.GetInt( "maxLevel",  &maxLevel );
	gArg.GetInt( "cblock",    &targetBlock );
	gArg.GetDouble( "cratio", &ratio );    

    //if( param.bShow )
    //{
	   //// printf( "   inc factor    = %f\n", incFactor );
	   // printf( "   wWire         = %f\n", wWire );
	   // printf( "   wWire L Decrease = %f\n", weightLevelDecreaingRate );
	   // printf( "   target util   = %f\n", target_utilization );
    //}
    
 //   printf( "Set all module orient N\n" );
 //   for( unsigned int i=0; i< placedb.m_modules.size(); i++ )
	//if( placedb.m_modules[i].m_isFixed == false )
	//    placedb.SetModuleOrientation( i, 0 );
   
    int levels = 0;

    // construct a hierarchy of clusters 
    m_p_placedb_clustered = new vector<CPlaceDB>;
    vector<CPlaceDB>& placedb_clustered = *m_p_placedb_clustered;
    m_p_clusters = new vector<CClustering>;
    vector<CClustering>& clusters = *m_p_clusters;
    //CPlaceDB dummyDB;
    
    m_currentDB = &placedb;
    //int currentBlock = currentDB->m_modules.size();
    
    //currentDB->OutputGnuplotFigure( "init_1.plt", false );
 
    //============================================================
    // add dummy blocks
    //int fillNumber = 0;  
    //currentDB->Init();	// calculate movable block area	/ free space
    //double dummyArea = ( currentDB->m_totalMovableModuleArea / currentDB->m_totalMovableModuleNumber );
    //dummyArea *= 25; // make it larger
    //double dummyWidth = sqrt( dummyArea );
 //   double fillUtil = 0.8;
 //   double fillArea = ( currentDB->m_totalFreeSpace * fillUtil - currentDB->m_totalMovableModuleArea );
 //   fillNumber = static_cast<int>( fillArea / dummyArea );

 //   fillNumber = 0; // do not fill dummy blocks
 //   
 //   for( int i=0; i<fillNumber; i++ )
 //   {
	//string name = "dummy";
	//currentDB->AddModule( name, dummyWidth, dummyWidth, false );
	//int id = (int)currentDB->m_modules.size() - 1;
	//currentDB->m_modules[id].m_isDummy = true;
	//
	//double w = currentDB->m_coreRgn.right - currentDB->m_coreRgn.left;
	//double h = currentDB->m_coreRgn.top - currentDB->m_coreRgn.bottom;
	//w *= 0.8;
	//h *= 0.8;
	//currentDB->MoveModuleCenter( id, 
	//	currentDB->m_coreRgn.left + w*0.1 + rand() % (int)w, 
	//	currentDB->m_coreRgn.bottom + h*0.1 + rand() % (int)h ); 
 //   }
 //   if( param.bShow )
	//printf( "Fill number = %d\n", fillNumber );
 //   currentDB->Init();
    //============================================================

    double clustering_start = seconds();
    int currentBlock = m_currentDB->GetMovableBlockNumber();
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
    while( currentBlock > targetBlock && levels < maxLevel-1 )
    {
	    levels++;
	    //placedb_clustered.resize( levels+1 );
    	
	    printf( "level %d, block # %d, movable block # %d > %d, do clustering...", 
		    levels, (int)m_currentDB->m_modules.size(), currentBlock, targetBlock );
	    fflush( stdout );
    	

	    clusters[levels-1].clustering( *m_currentDB, placedb_clustered[levels-1], (int)((currentBlock)/ratio), 1.5, ctype );
    	
	    printf( " done\n" );
	    fflush( stdout );
    	
        m_currentDB = &placedb_clustered[levels-1];
	    currentBlock = m_currentDB->GetMovableBlockNumber();	
    }
    printf( "block # %d, movable block # %d\n", (int)m_currentDB->m_modules.size(), currentBlock );
    printf( "clustering time: %.2f\n", seconds() - clustering_start );
   
    
    //currentDB->OutputGnuplotFigure( "init_2.plt", false );  // show physical clustering positions

   
    // generate level parameters 
    m_totalLevels = levels + 1;
    double start_density = param.startDensity;
    double final_target_density = param.endDensity;
    double startSmoothDelta = param.startDelta;
    double finalSmoothDelta = param.endDelta;
    //vector<double> levelSmoothDelta;
    //vector<double> levelTargetOver;
    m_levelSmoothDelta.resize( m_totalLevels );
    m_levelTargetOver.resize( m_totalLevels );
    printf( "\n" ); 
    for( int i=0; i<levels; i++ )
    {
	    int currentLevel = i+1;
	    m_levelTargetOver[i] = start_density - ( start_density - final_target_density ) * (currentLevel) / (m_totalLevels);
	    m_levelSmoothDelta[i] = startSmoothDelta - ( startSmoothDelta - finalSmoothDelta ) * (currentLevel) / (m_totalLevels); 

	    if( param.bShow )
	        printf( "Level %d\tBlock %d\tPin %d\tDelta %.3f\tOver %.3f\n", 
		        i+1, (int)placedb_clustered[levels-i-1].m_modules.size(), 
		        (int)placedb_clustered[levels-i-1].m_pins.size(), m_levelSmoothDelta[i], m_levelTargetOver[i] );	
    }
    if( param.bShow )
	printf( "Level %d\tBlock %d\tPin %d\tDelta %.3f\tOver %.3f\n\n", 
		levels+1, (int)placedb.m_modules.size(), 
		(int)placedb.m_pins.size(),
		finalSmoothDelta, final_target_density );
    
    
    // init solution
#if 1
    initPlacement();
    //fixed pin move
    //if(param.isFPF==true)
    //{
    //    MacroInit initMacro(*currentDB);
    //    initMacro.init();
    //    initMacro.solve();
    //}
#endif
    
    if( bOutInterPL )
    {
	    string file = outFilePrefix + "_global_init.plt";
	    m_currentDB->OutputGnuplotFigure( file.c_str(), false );
    }
  
    if( param.bShow ) 
    {
	    printf( "\n block = %d, net = %d, pin = %d\n", (int)m_currentDB->m_modules.size(),
		    (int)m_currentDB->m_nets.size(), (int)m_currentDB->m_pins.size() );
	    m_currentDB->OutputGnuplotFigure( "init_qp.plt", false );
    }

    //// spread dummy cells
    //if( fillNumber > 0 )
    //{
	   // for( unsigned int i=0; i<m_currentDB->m_modules.size(); i++ )
	   // {
	   //     if( m_currentDB->m_modules[i].m_isDummy == true )
	   //     {
		  //  double w = currentDB->m_coreRgn.right - currentDB->m_coreRgn.left;
		  //  double h = currentDB->m_coreRgn.top - currentDB->m_coreRgn.bottom;
		  //  w *= 0.8;
		  //  h *= 0.8;
		  //  currentDB->MoveModuleCenter( i, 
			 //   currentDB->m_coreRgn.left + w*0.1 + rand() % (int)w, 
			 //   currentDB->m_coreRgn.bottom + h*0.1 + rand() % (int)h ); 
	   //     }
	   // }
    //}

    

    // placement on the clsutered placedb
    //int currentLevel = 0;
    //double weightWire = wWire;
    m_currentLevel = 0;
    m_weightWire = wWire;   
    m_levels=levels;
    m_pDB=&placedb;
    m_currentLevel ++;

 //   while( levels > 0 )
 //   {
	//    currentLevel ++;

	//    // TEST for > 2 levels
	//    if( currentLevel >= 2 )
	//        weightWire = weightWire / weightLevelDecreaingRate;

	//    MyQP* myqp = new MyQP( *m_currentDB );
 //   	
	//    bool noRelaxSmooth = false;
 //     
	//    myqp->m_smoothDelta = m_levelSmoothDelta[currentLevel-1];
	//    myqp->m_useBellPotentialForPreplaced = true;
 //           myqp->m_earlyStop = true;	
	//    myqp->m_lookAheadLegalization = false;
	//    myqp->m_incFactor  = incFactor;
	//    myqp->m_targetUtil = target_utilization;
	//    if( param.bShow )
	//    {
	//        printf( "LEVEL %d of %d (target overflow %f, smooth delta %f)(%.0fMB)\n", 
	//	        currentLevel, m_totalLevels, m_levelTargetOver[currentLevel-1], m_levelSmoothDelta[currentLevel-1],
	//	        GetPeakMemoryUsage() );	
	//        printf( "[block = %d, net = %d, pin = %d]\n", m_currentDB->m_modules.size(),
	//	        m_currentDB->m_nets.size(), m_currentDB->m_pins.size() );
	//    }
 //   	
	//    // 2006-03-26
	//    /*if( currentLevel == 1 )
	//    {
	//        double oldStep = param.step;
	//        param.step = 0.1;
	//        myqp->MySolve( weightWire, levelTargetOver[currentLevel-1], currentLevel, noRelaxSmooth );
	//        param.step = oldStep;
	//    }
	//    else	*/
	//        myqp->MySolve( weightWire, m_levelTargetOver[currentLevel-1], currentLevel, noRelaxSmooth );
	//    delete myqp;
	//    myqp = NULL;

 //   	
	//    // Macro shifter
	//    double avgHeight = 0;
	//    int count = 0;
	//    for( unsigned int i=0; i<m_currentDB->m_modules.size(); i++ )
	//    {
	//        if( m_currentDB->m_modules[i].m_isFixed )
	//	    continue;
	//        count++;
	//        avgHeight += m_currentDB->m_modules[i].m_height;
	//    }

	//    avgHeight /= count;

 //       //cout<<"\nStart GRoute!!";
 //   	
 //       //double ttt1=seconds();
 //       //MyGlobalRouter mr(*m_currentDB, m_currentDB->m_rowHeight, m_currentDB->m_rowHeight);
 //       //mr.initResource();
 //       //mr.route();
 //       //mr.OutputHGrid("Hgrid.dat");
 //       //mr.OutputVGrid("Vgrid.dat");
 //       //mr.outputfig("routingPlt");

 //       //cout<<"\nTotal Route Time:"<<seconds()-ttt1<<" seconds.\n";
 //       //exit(0);
 //   	
	//    // TODO: remove dummy blocks

 //   	
	//    if( bOutInterPL )
	//    {
	//        char file[200];
	//        sprintf( file, "%s_global_level_%d.plt", outFilePrefix.c_str(), currentLevel );
	//        m_currentDB->OutputGnuplotFigure( file, false );
	//    }

	//    double tUsed = seconds() - slqp_start;
	//    printf( "\n########### [TotalTime = %.2f sec = %.2f min] ###########\n\n", tUsed, tUsed / 60.0 );
	//    if( levels > 1 )
	//    {
	//        clusters[levels-1].declustering( placedb_clustered[levels-1], placedb_clustered[levels-2] );
 //   	    
	//        clusters.resize( levels-1 ); // donnie
	//        placedb_clustered.resize( levels-1 );
 //   	    
	//        m_currentDB = &placedb_clustered[levels-2];
	//        //printf( "\n ************************\n" );
	//    }
	//    else	// == 1
	//    {
	//        // to top level
	//        clusters[levels-1].declustering( placedb_clustered[levels-1], placedb );
	//        clusters.clear();
	//        //printf( "\n ******* top level ******\n" );
 //   	    
	//        delete p_clusters;
	//        delete p_placedb_clustered;
 //   	    
	//        break;
	//    }	    
	//    levels--;

 //   }


 //   if( param.bShow )
 //   {
	//printf( "LEVEL %d of %d (target density %f, smooth delta %d)(%.0fMB)\n", 
	//	currentLevel+1, m_totalLevels, final_target_density, 1, GetPeakMemoryUsage() );	
 //   }
 //   
 //   if( param.topDblStep )  // 2006-03-25
	//param.step *= 2.0;

 //   MyQP* myqp = new MyQP( placedb );
 //   myqp->m_useBellPotentialForPreplaced = true;
 //   myqp->m_topLevel = true;
 //   myqp->m_lookAheadLegalization = true;
 //   myqp->m_incFactor  = incFactor;
 //   myqp->m_targetUtil = target_utilization;
 //   //myqp->m_smoothDelta = finalSmoothDelta;
 //   myqp->m_smoothDelta = param.endDelta;
 //   //bool isLegal = myqp->MySolve( weightWire, final_target_density, currentLevel+1, true );
 //   bool isLegal = myqp->MySolve( weightWire, param.endDensity, currentLevel+1, true );
 //   delete myqp;
 //   myqp = NULL;

 //   //if( fillNumber > 0 )
 //   //{
	//   // placedb.m_modules.resize( placedb.m_modules.size() - fillNumber );
 //   //}
    
}
void mlqp::currentDBPlace()
{
    if(m_levels<1)
    {
        cerr<<"\nWarning!! reach the top level!! ";
        return;
    }

    

    // TEST for > 2 levels
    if( m_currentLevel >= 2 )
	    m_weightWire = m_weightWire / m_weightLevelDecreaingRate;

    MyQP* myqp = new MyQP( *m_currentDB );

    bool noRelaxSmooth = false;

    myqp->m_smoothDelta = m_levelSmoothDelta[m_currentLevel-1];
    myqp->m_useBellPotentialForPreplaced = true;
        myqp->m_earlyStop = true;	
    myqp->m_lookAheadLegalization = false;
    //myqp->m_incFactor  = m_incFactor;
    myqp->m_targetUtil = param.coreUtil;
    if( param.bShow )
    {
	    printf( "LEVEL %d of %d (target overflow %f, smooth delta %f)(%.0fMB)\n", 
		    m_currentLevel, m_totalLevels, m_levelTargetOver[m_currentLevel-1], m_levelSmoothDelta[m_currentLevel-1],
		    GetPeakMemoryUsage() );	
	    printf( "[block = %d, net = %d, pin = %d]\n", (int)m_currentDB->m_modules.size(),
		    (int)m_currentDB->m_nets.size(), (int)m_currentDB->m_pins.size() );
    }


	myqp->MySolve( m_weightWire, m_levelTargetOver[m_currentLevel-1], m_currentLevel, noRelaxSmooth );
    delete myqp;
    myqp = NULL;


    //// Macro shifter
    //double avgHeight = 0;
    //int count = 0;
    //for( unsigned int i=0; i<m_currentDB->m_modules.size(); i++ )
    //{
	   // if( m_currentDB->m_modules[i].m_isFixed )
	   // continue;
	   // count++;
	   // avgHeight += m_currentDB->m_modules[i].m_height;
    //}

    //avgHeight /= count;

    //cout<<"\nStart GRoute!!";

    //double ttt1=seconds();
    //MyGlobalRouter mr(*m_currentDB, m_currentDB->m_rowHeight, m_currentDB->m_rowHeight);
    //mr.initResource();
    //mr.route();
    //mr.OutputHGrid("Hgrid.dat");
    //mr.OutputVGrid("Vgrid.dat");
    //mr.outputfig("routingPlt");

    //cout<<"\nTotal Route Time:"<<seconds()-ttt1<<" seconds.\n";
    //exit(0);

    // TODO: remove dummy blocks


    if( bOutInterPL )
    {
	    char file[200];
	    sprintf( file, "%s_global_level_%d.plt", m_outFilePrefix.c_str(), m_currentLevel );
	    m_currentDB->OutputGnuplotFigure( file, false );
    }

    double tUsed = seconds() - m_mlqp_start;
    printf( "\n########### [TotalTime = %.2f sec = %.2f min] ###########\n\n", tUsed, tUsed / 60.0 );
}

bool mlqp::decluster()
{

    if(m_levels<1)
        return false;
    vector<CPlaceDB>& placedb_clustered = *m_p_placedb_clustered;
    vector<CClustering>& clusters = *m_p_clusters;

	if( m_levels > 1 )
	{
	    clusters[m_levels-1].declustering( placedb_clustered[m_levels-1], placedb_clustered[m_levels-2] );
    	
	    clusters.resize( m_levels-1 ); 
	    placedb_clustered.resize( m_levels-1 );
    	
	    m_currentDB = &placedb_clustered[m_levels-2];
        m_currentLevel ++;

	}
	else	// == 1
	{
	    // to top level
	    clusters[m_levels-1].declustering( placedb_clustered[m_levels-1], *m_pDB );
	    clusters.clear();
	    //printf( "\n ******* top level ******\n" );
    	
	    delete m_p_clusters;
	    delete m_p_placedb_clustered;
        m_levels--;
    	
	    return false;
	}	    
	m_levels--;
    return true;
}

bool mlqp::endcasePlace()
{
    if( param.bShow )
    {
	printf( "LEVEL %d of %d (target density %f, smooth delta %d)(%.0fMB)\n", 
		m_currentLevel+1, m_totalLevels, param.endDensity, 1, GetPeakMemoryUsage() );	
    }
    
    if( param.topDblStep )  // 2006-03-25
    	param.step *= 2.0;

    MyQP* myqp = new MyQP( *m_pDB );
    myqp->m_useBellPotentialForPreplaced = true;
    myqp->m_topLevel = true;
    myqp->m_lookAheadLegalization = true;
    //myqp->m_incFactor  = m_incFactor;
    myqp->m_targetUtil = param.coreUtil;
    //myqp->m_smoothDelta = finalSmoothDelta;
    myqp->m_smoothDelta = param.endDelta;
    //bool isLegal = myqp->MySolve( weightWire, final_target_density, currentLevel+1, true );
    bool isLegal = myqp->MySolve( m_weightWire, param.endDensity, m_currentLevel+1, true );
    delete myqp; 
    myqp = NULL;
    return isLegal;
}
void mlqp::initPlacement()
{
    if( param.bQP )
    {
	    printf( "Solve QP\n" );
	    CPlaceDBQPPlacer* pqplace = new CPlaceDBQPPlacer( *m_currentDB );
	    pqplace->QPplace();
	    delete pqplace;
	    pqplace = NULL;
    }
    else
    {
	    // random place
	    CRandomPlace::Place( *m_currentDB, 0.1 );
    }
}

void mlqp::currentDBPlaceForGRoute()
{
    if(m_levels<1)
    {
        cerr<<"\nWarning!! reach the top level!! ";
        return;
    }
    if( param.bShow )
    {
	printf( "Placement for Global Router Evaluate!!\n");	
    }
    

    if( param.topDblStep )  // 2006-03-25
    	param.step *= 2.0;

    MyQP* myqp = new MyQP( *m_currentDB );
    myqp->m_useBellPotentialForPreplaced = true;
    myqp->m_topLevel = true;
    myqp->m_lookAheadLegalization = false;
    //myqp->m_incFactor  = m_incFactor;
    myqp->m_targetUtil = param.coreUtil;
    //myqp->m_smoothDelta = finalSmoothDelta;
    myqp->m_smoothDelta = param.endDelta;
    //bool isLegal = myqp->MySolve( weightWire, final_target_density, currentLevel+1, true );
    myqp->MySolve( m_weightWire, param.endDensity, m_currentLevel, true );
    delete myqp; 
    myqp = NULL;
}
