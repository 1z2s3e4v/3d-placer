#include "timing.h"
#include "lefdef.h"
//#include "sta.h"

using namespace std;
using STA::Node;
//using STA::Net;

// global variables
extern vector<Node> netlist;		// the netlist
extern unsigned int nodeCount;		// # of nodes in the netlist, including PI & PO
extern unsigned int clkNodeId;		// id of the clock node (port)
extern double clkCycle;		// clock cycle time
extern vector<unsigned int> priIn;	// id of PI in the netlist
extern vector<unsigned int> priOut;	// id of PO in the netlist
extern vector<unsigned int> flipflop;	// id of DFF in the netlist
extern map<string, unsigned int> nodeIdMap;	// form node name to node id
extern map<string, unsigned int> netIdMap;	// form net name to net id
extern vector<STA::Net> nets;		// net in the netlist
extern double initLeakage;		// initial leakage power (using standard Vt)
extern vector<SubCkt> subCkts;		// subcircuit (combinational circuit)
// end of global variables

void TimingAnalysis::Initialize( void )
{
    string libName; 
    gArg.GetString( "liberty", &libName );

    m_libParser.parseLib( const_cast<char*>( libName.c_str() ) );

    const int module_num = m_placedb.m_modules.size();
    m_mid_mapPlaceDBToSTA.resize( module_num, -1 );
    m_mid_mapSTAToPlaceDB.resize( module_num, -1 );
    //m_libParser.parseHVTLib( const_cast<char*>( libName.c_str() ) );
    //m_libParser.parseLVTLib( const_cast<char*>( libName.c_str() ) );

    //cout << "********" << libName << endl;

    const unsigned int LastMetal = 3;
    //compute m_average capacitance for unit square area (note that metal 1 is skipped)
    m_averageUnitCap = 0.0;
    if( m_placedb.m_pLEF->m_unitCapacitance.size() > 2 )
    {
	//for( unsigned int i = 2 ; i < m_placedb.m_pLEF->m_unitCapacitance.size() ; i++ )
	for( unsigned int i = 2 ; i <= LastMetal ; i++ )
	{
	    m_averageUnitCap += m_placedb.m_pLEF->m_unitCapacitance[i];
	}
    }
    m_averageUnitCap = m_averageUnitCap / static_cast<double>( m_placedb.m_pLEF->m_unitCapacitance.size()-2 );

    cout << "m_averageUnitCap: " << m_averageUnitCap << endl;

    m_averageMetalWidth = 0.0;
    if( m_placedb.m_pLEF->m_metalWidth.size() > 2 )
    {
	//for( unsigned int i = 2 ; i < m_placedb.m_pLEF->m_metalWidth.size() ; i++ )
	for( unsigned int i = 2 ; i <= LastMetal  ; i++ )
	{
	    m_averageMetalWidth += m_placedb.m_pLEF->m_metalWidth[i];
	}
    }
    m_averageMetalWidth = m_averageMetalWidth / static_cast<double>( m_placedb.m_pLEF->m_metalWidth.size() - 2 );

    cout << "avergeMetalWidth: " << m_averageMetalWidth << endl;

    m_averageCap = m_averageUnitCap * m_averageMetalWidth;

    cout << "m_averageCap: " << m_averageCap << endl;

}


void TimingAnalysis::ConstructNetlist( void )
{
    //replace "parseNetLoad( loadFileName )"

    //The nets in m_placedb and STA have the same index
    for( unsigned int i = 0 ; i < m_placedb.m_nets.size() ; i++ )
    {
	STA::Net sta_net;
	sta_net.name = m_placedb.m_netsName[i];
	sta_net.loadCap = m_placedb.GetNetLength( i ) * m_averageCap;
	nets.push_back( sta_net );

	netIdMap[ sta_net.name ] = i;

	//test code
	//cout << sta_net.name << " " << sta_net.loadCap << endl;
	//@test code
    }
    //@replace "parseNetLoad( loadFileName )"
    
    //replace netlistAddPins() in "parseNetlist( netFileName, cellLib )"
    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	const Module &curModule = m_placedb.m_modules[i];
	
	if( curModule.m_blockType == BT_PI ) 
	{
	    STA::Node node;

	    if( curModule.m_pinsId.size() < 1 )
	    {
		fprintf( stderr, "Warning: input pin (module %d) does not have any pin\n", i );
		continue;
	    }
	    
	    node.cellName = curModule.m_name;
	    string cellType;
	    cellType = "IN";
	    
	    node.cellTypePtr = m_libParser.getCellType(cellType);

	    //printf( "pin %s celltype %s\n", node.cellName.c_str(), node.cellTypePtr->name.c_str() );
		    
	    node.voltageType = NVT;	// PIO has no voltage type
	    node.clkLatency = 0.0;
	    node.delay = 0.0;	// initialized as no external delay
	    node.maxCD = 0.0;
	    node.visited = false;
	    node.Loadgradient = DBL_MAX;
	    node.ATgradient = DBL_MAX;
	    node.RTgradient = DBL_MAX;
	    node.ATnetId = -1;
	    node.RTnetId = -1;
	    //		node.heapIndex = INT_MAX;
	    nodeIdMap[ node.cellName ] = netlist.size();
	    priIn.push_back( netlist.size() );

	    const int sta_mid = netlist.size();
	    const int placedb_mid = i;
	    m_mid_mapPlaceDBToSTA[ placedb_mid ] = sta_mid;
	    m_mid_mapSTAToPlaceDB[ sta_mid ] = placedb_mid;
	    netlist.push_back( node );
	}
    }
    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	const Module &curModule = m_placedb.m_modules[i];
	
	if( curModule.m_blockType == BT_PO )
	{
	    STA::Node node;

	    if( curModule.m_pinsId.size() < 1 )
	    {
		fprintf( stderr, "Warning: input pin (module %d) does not have any pin\n", i );
		continue;
	    }
	    
	    node.cellName = curModule.m_name;
	    string cellType;
	    cellType = "OUT";
	    
	    node.cellTypePtr = m_libParser.getCellType(cellType);

	    //printf( "pin %s celltype %s\n", node.cellName.c_str(), node.cellTypePtr->name.c_str() );
		    
	    node.voltageType = NVT;	// PIO has no voltage type
	    node.clkLatency = 0.0;
	    node.delay = 0.0;	// initialized as no external delay
	    node.maxCD = 0.0;
	    node.visited = false;
	    node.Loadgradient = DBL_MAX;
	    node.ATgradient = DBL_MAX;
	    node.RTgradient = DBL_MAX;
	    node.ATnetId = -1;
	    node.RTnetId = -1;
	    //		node.heapIndex = INT_MAX;
	    nodeIdMap[ node.cellName ] = netlist.size();
	    priOut.push_back( netlist.size() );

	    const int sta_mid = netlist.size();
	    const int placedb_mid = i;
	    m_mid_mapPlaceDBToSTA[ placedb_mid ] = sta_mid;
	    m_mid_mapSTAToPlaceDB[ sta_mid ] = placedb_mid;
	    netlist.push_back( node );
	}
    }
    //@replace netlistAddPins() in  "parseNetlist( netFileName, cellLib )"
    
    //replace netlistAddComponents() in "parseNetlist( netFileName, cellLib )"
    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	const Module &curModule = m_placedb.m_modules[i];
	
	if( curModule.m_blockType != BT_PI &&
	    curModule.m_blockType != BT_PO /*&&
	    curModule.m_blockType != BT_P_INOUT*/ )
	{
	    Node node;
	    node.cellName = curModule.m_name;
	    string cellType = m_placedb.m_pLEF->m_modules[ curModule.m_lefCellId ].m_name;
	    //netlistFile >> cellType;

	    //cout << "cellname " << node.cellName << " typename " << cellType << endl;
	    
	    // W.-P. 20070118
	    node.cellTypePtr = m_libParser.getCellType(cellType);
	    // initialize Vt
	    if( node.cellTypePtr->DFF == true )	// DFF
		node.voltageType = INIT_DFF_VT;
	    else if( isCKBUF(cellType) )		// CKBUF
		node.voltageType = INIT_CKBUF_VT;
	    else	// ordinary cell types
		node.voltageType = INIT_CELL_VT;
	    
	    node.clkLatency = 0.0;
	    node.delay = 0.0;
	    node.maxCD = 0.0;
	    //node.requiredTime = (double)INT_MAX;
	    node.requiredTime = DBL_MAX;
	    node.visited = false;
	    node.nVisitedFanin = 0;
	    node.Loadgradient = DBL_MAX;
	    node.ATgradient = DBL_MAX;
	    node.RTgradient = DBL_MAX;
	    node.ATnetId = -1;
	    node.RTnetId = -1;
	    //		node.heapIndex = INT_MAX;
	    
	    nodeIdMap[ node.cellName ] = netlist.size();
	    // add index into DFF
	    if( node.cellTypePtr->DFF == true )
		flipflop.push_back( netlist.size() );
	    
	    const int sta_mid = netlist.size();
	    const int placedb_mid = i;
	    m_mid_mapPlaceDBToSTA[ placedb_mid ] = sta_mid;
	    m_mid_mapSTAToPlaceDB[ sta_mid ] = placedb_mid;
	    
	    netlist.push_back( node );
	    
	}
    }
    //@replace netlistAddComponents() in "parseNetlist( netFileName, cellLib )"

    //replace netlistAddNet() in "parseNetlist( netFileName, cellLib )"
    for( unsigned int i = 0 ; i < m_placedb.m_nets.size() ; i++ )
    {
	const Net& curNet = m_placedb.m_nets[i];

	//nets has the same index with m_placedb.m_nets
	STA::Net &net = nets[ i ];

	Fanout* fanout = NULL;
	int srcNodeId = -1;
	string srcPin;

	//Two pass to make sure the input pin of the net is added in net.nodes first
	for( unsigned int iNetPin = 0 ; iNetPin < curNet.size() ; iNetPin++ )
	{
	    const int pinId = curNet[ iNetPin ];
	    const Pin& curPin = m_placedb.m_pins[ pinId ];
	   
	    
	    if( curPin.direction == PIN_DIRECTION_OUT ) 
	    {
		const int moduleId = curPin.moduleId;
		const Module& curModule = m_placedb.m_modules[ moduleId ];
		srcNodeId = nodeIdMap[ curModule.m_name ];
		srcPin = curPin.pinName;
		
		//if( curPin.direction != PIN_DIRECTION_OUT )
		//{
		//    fprintf( stderr, "input pin %s is not a output pin of a cell\n", srcPin.c_str() );
		//}

		//test code
		//printf( "net %s (%d) input pin %s\n", m_placedb.m_netsName[i].c_str(),
		//	i, srcPin.c_str() );
		//@test code
		//parseCellInstance( token, srcNodeId, srcPin );
		fanout = &(netlist[ srcNodeId ].fanoutMap[ srcPin ]);
		fanout->loadCap = net.loadCap;

		//Added by Jin 20081014
		fanout->netId = i;
		
		// add source node into the net
		NetNodeInfo netNodeInfo;
		netNodeInfo.nodeId = srcNodeId;
		netNodeInfo.pin = srcPin;
		net.nodes.push_back( netNodeInfo );
	    }
	}
	
	if( fanout == NULL )
	{
		printf( "skip net %s (has no input pin)\n", m_placedb.m_netsName[i].c_str() );
	}
	
	for( unsigned int iNetPin = 0 ; iNetPin < curNet.size() ; iNetPin++ )
	{
	    const int pinId = curNet[ iNetPin ];
	    const Pin& curPin = m_placedb.m_pins[ pinId ];

	    
	    if( curPin.direction != PIN_DIRECTION_OUT )
	    {
		const int moduleId = curPin.moduleId;
		const Module& curModule = m_placedb.m_modules[ moduleId ];
		unsigned int desNodeId = nodeIdMap[ curModule.m_name ];
		string desPin = curPin.pinName;
	    
		//unsigned int desNodeId;
		//string desPin;
		//parseCellInstance( token, desNodeId, desPin );
		
		// add fanout to the source node
		FanoutNodeInfo desInfo;
		desInfo.nodeId = desNodeId;
		desInfo.pin = desPin;
		if( fanout != NULL )
		{
		    fanout->nodes.push_back( desInfo );
		    // add fanin to the destination node
		    Fanin & srcInfo = netlist[ desNodeId ].faninMap[desPin];
		    srcInfo.nodeId = srcNodeId;
		    srcInfo.pin = srcPin;
		    srcInfo.transitionTime = 0.0;

		    //Added by Jin 20081014
		    srcInfo.netId = i;

		    // add destination node into the net
		    NetNodeInfo netNodeInfo;
		    netNodeInfo.nodeId = desNodeId;
		    netNodeInfo.pin = desPin;
		    net.nodes.push_back( netNodeInfo );
		}
		else
		{
		    netlist[ desNodeId ].faninMap.erase( desPin );
		    fflush( stdout );
		}

	    }
	}
#if 0
	// read source instance
	string token, srcPin;
	iss >> token;
	unsigned int srcNodeId;
	parseCellInstance( token, srcNodeId, srcPin );
	Fanout &fanout = netlist[ srcNodeId ].fanoutMap[ srcPin ];
	fanout.loadCap = net.loadCap;
	// add source node into the net
	NetNodeInfo netNodeInfo;
	netNodeInfo.nodeId = srcNodeId;
	netNodeInfo.pin = srcPin;
	net.nodes.push_back( netNodeInfo );
	// read destination instance
	while( iss >> token ) 
	{
	    unsigned int desNodeId;
	    string desPin;
	    parseCellInstance( token, desNodeId, desPin );
	    // add fanout to the source node
	    FanoutNodeInfo desInfo;
	    desInfo.nodeId = desNodeId;
	    desInfo.pin = desPin;
	    fanout.nodes.push_back( desInfo );
	    // add fanin to the destination node
	    Fanin & srcInfo = netlist[ desNodeId ].faninMap[desPin];
	    srcInfo.nodeId = srcNodeId;
	    srcInfo.pin = srcPin;
	    srcInfo.transitionTime = 0.0;
	    // add destination node into the net
	    NetNodeInfo netNodeInfo;
	    netNodeInfo.nodeId = desNodeId;
	    netNodeInfo.pin = desPin;
	    net.nodes.push_back( netNodeInfo );
	}
#endif
    }
    //@replace netlistAddNet() in "parseNetlist( netFileName, cellLib )"
    
    //replace "parseTimingConstraint( timingFileName )"
    string clkName;
    if( gArg.CheckExist( "clock" ) )
    {
	gArg.GetString( "clock", &clkName );
	clkNodeId = nodeIdMap[ clkName ];
    }
    else
    {
	printf( "No clock name is given by -clock!!\n" );
	exit(0);
    }

    if( gArg.CheckExist( "clock_period" ) )
    {
	gArg.GetDouble( "clock_period", &clkCycle );
    }
    else
    {
	printf( "Warning: no clock period is given. Defaultly set to 0.0\n" );
	clkCycle = 0.0;
    }

    //test code
    printf( "clock name %s clock cycle %f\n", clkName.c_str(), clkCycle );
    //@test code
    
    for( unsigned int i = 0 ; i < priIn.size() ; i++ )
    {
	netlist[ priIn[i] ].delay = 0.0;
	netlist[ priIn[i] ].Loadgradient = 0.0;
    }
    for( unsigned int i = 0 ; i < priOut.size() ; i++ )
    {
	netlist[ priOut[i] ].delay = 0.0;
	netlist[ priOut[i] ].Loadgradient = 0.0;
    }

    for( unsigned int i = 0 ; i < flipflop.size() ; i++ )
    {
	netlist[ flipflop[i] ].Loadgradient = 0.0;
    }

    //test code
    //cout << "OOOOOOOOOOOOOOOOOO " << netlist[10632].cellName << endl;
    //cout << "OOOOOOOOOOOOOOOOOO " << netlist[10632].cellTypePtr->name << endl;
    //@test code
    
    //@replace "parseTimingConstraint( timingFileName )"

    //test code
#if 0
    for( map<string, cellInfo*>::iterator ite = m_libParser.cellsMap.begin() ;
	    ite != m_libParser.cellsMap.end() ; ite++ )
    {
	printf( "cell name: %s\n", ite->second->name.c_str() );
	for( map<string, struct pinInfo*>::iterator itePin = ite->second->svt_pinsMap->begin() ;
		itePin != ite->second->svt_pinsMap->end() ; ite++ )
	{
	    printf( "pin name: %s %s\n", itePin->first.c_str(), itePin->second->name.c_str() );
	
	}
    }
#endif
    /*
    for( unsigned int i = 0 ; i < netlist.size() ; i++ )
    {
	Node& curNode = netlist[i];

	//printf( "Node %d: %s\n", i, curNode.cellName.c_str() );

	for( map<string, Fanin>::iterator ite = curNode.faninMap.begin() ;
		ite != curNode.faninMap.end() ; ite++ )
	{
	    if( ite->first == "" )
	    {
		printf( "Node %d: %s\n", i, curNode.cellName.c_str() );
		printf("Fanin : %s\n", ite->first.c_str() );
	    }
	}
	
	for( map<string, Fanout>::iterator ite = curNode.fanoutMap.begin() ;
		ite != curNode.fanoutMap.end() ; ite++ )
	{
	    if( ite->first == "" )
	    {
		printf( "Node %d: %s\n", i, curNode.cellName.c_str() );
		printf("Fanout: %s\n", ite->first.c_str() );
	    }
	}
    }
    */
    /*
    for( unsigned int i = 0 ; i < nets.size() ; i++ )
    {
	STA::Net& curNet = nets[i];

	printf( "Net %d: %s\n", i, curNet.name.c_str() );
    }
    */
    //@test code
    
    updateOutputLoad();

    decomposeSubCkts();
    //nodeIdMap.clear();
    //netIdMap.clear();

    cout << "SubCkts size() " << subCkts.size() << endl;
    cout << "subCkts[0].nodes.size() " << subCkts[0].nodes.size() << endl;
    
}

void TimingAnalysis::RunSTA( void )
{
    fullSSTA();

    //double WNS = 0.0;    
    //double TNS = 0.0;
    m_WNS = 0.0;
    m_TNS = 0.0;
    for( unsigned int i = 0 ; i < netlist.size() ; i++ )
    {
	Node& curNode = netlist[i];

	//printf( "Node %d: %s slack %f\n", i, curNode.cellName.c_str(), curNode.slack );

	if( curNode.slack < m_WNS )
	{
	    m_WNS = curNode.slack;
	}

	if( curNode.slack < 0.0 )
	{
	    m_TNS += curNode.slack;

	    //test code
	    //printf( "+slack %f\n", curNode.slack );
	    //@test code
	}

    }
    printf( "WNS: %f\n", m_WNS );  
    printf( "TNS: %f\n", m_TNS );
}

void TimingAnalysis::SetNetLoad( void )
{
    //The nets in m_placedb and STA have the same index
    for( unsigned int i = 0 ; i < m_placedb.m_nets.size() ; i++ )
    {
	//nets has the same index with m_placedb.m_nets
	
	const Net& curNet = m_placedb.m_nets[i];
	
	STA::Net &net = nets[ i ];
	net.loadCap = m_placedb.GetNetLength( i ) * m_averageCap;

	Fanout* fanout = NULL;
	int srcNodeId = -1;
	string srcPin;
	
	//Two pass to make sure the input pin of the net is added in net.nodes first
	for( unsigned int iNetPin = 0 ; iNetPin < curNet.size() ; iNetPin++ )
	{
	    const int pinId = curNet[ iNetPin ];
	    const Pin& curPin = m_placedb.m_pins[ pinId ];
	   
	    if( curPin.direction == PIN_DIRECTION_OUT ) 
	    {
		const int moduleId = curPin.moduleId;
		const Module& curModule = m_placedb.m_modules[ moduleId ];
		srcNodeId = nodeIdMap[ curModule.m_name ];
		srcPin = curPin.pinName;
		
		//if( curPin.direction != PIN_DIRECTION_OUT )
		//{
		//    fprintf( stderr, "input pin %s is not a output pin of a cell\n", srcPin.c_str() );
		//}

		//test code
		//printf( "net %s (%d) input pin %s\n", m_placedb.m_netsName[i].c_str(),
		//	i, srcPin.c_str() );
		//@test code
		//parseCellInstance( token, srcNodeId, srcPin );
		fanout = &(netlist[ srcNodeId ].fanoutMap[ srcPin ]);
		fanout->loadCap = net.loadCap;
		// add source node into the net
		//NetNodeInfo netNodeInfo;
		//netNodeInfo.nodeId = srcNodeId;
		//netNodeInfo.pin = srcPin;
		//net.nodes.push_back( netNodeInfo );
	    }
	}
	
	if( fanout == NULL )
	{
	    printf( "skip net %s (has no input pin)\n", m_placedb.m_netsName[i].c_str() );
	    continue;
	}
	
	for( unsigned int iNetPin = 0 ; iNetPin < curNet.size() ; iNetPin++ )
	{
	    const int pinId = curNet[ iNetPin ];
	    const Pin& curPin = m_placedb.m_pins[ pinId ];

	    
	    if( curPin.direction != PIN_DIRECTION_OUT )
	    {
		const int moduleId = curPin.moduleId;
		const Module& curModule = m_placedb.m_modules[ moduleId ];
		unsigned int desNodeId = nodeIdMap[ curModule.m_name ];
		string desPin = curPin.pinName;
	    
		//unsigned int desNodeId;
		//string desPin;
		//parseCellInstance( token, desNodeId, desPin );
		
		// add fanout to the source node
		//FanoutNodeInfo desInfo;
		//desInfo.nodeId = desNodeId;
		//desInfo.pin = desPin;
		//fanout->nodes.push_back( desInfo );
		// add fanin to the destination node
		Fanin & srcInfo = netlist[ desNodeId ].faninMap[desPin];
		//srcInfo.nodeId = srcNodeId;
		//srcInfo.pin = srcPin;
		srcInfo.transitionTime = 0.0;
		// add destination node into the net
		//NetNodeInfo netNodeInfo;
		//netNodeInfo.nodeId = desNodeId;
		//netNodeInfo.pin = desPin;
		//net.nodes.push_back( netNodeInfo );

	    }
	}
	//test code
	//printf( "set net %d load %f\n", i, nets[i].loadCap );
	//@test code
    }

}

void TimingAnalysis::RerunSTA( void )
{
    SetNetLoad();
    ResetSTA();
    
    fullSSTA();
    
    double WNS = 0.0;    
    double TNS = 0.0;
    m_WNS = 0.0;
    m_TNS = 0.0;
    for( unsigned int i = 0 ; i < netlist.size() ; i++ )
    {
	Node& curNode = netlist[i];

	//if( isnan( curNode.slack) )
	//{
	//    cout << "NAN" << endl;
	//}
	
	
	//printf( "Node %d: %s slack %f\n", i, curNode.cellName.c_str(), curNode.slack );

	//WNS = min( WNS, curNode.slack );
	m_WNS = min( m_WNS, curNode.slack );
	//if( curNode.slack < WNS )
	//{
	//    WNS = WNS + curNode.slack;
	//}

	char Slack[100];
	sprintf( Slack, "%f", curNode.slack );
	
	double negative_slack = atof( Slack );
	TNS = TNS + negative_slack;

	if( curNode.slack < 0.0 )
	{
	    m_TNS = m_TNS + curNode.slack;
	}
	//printf( "+negative_slack %f\n", negative_slack );
	
	//if( curNode.slack < 0.0 )
	//{
	//    TNS = TNS + curNode.slack;
	    //test code
	    //@test code
	//}

    }
    
    printf( "\nWNS: %f\n", m_WNS );  
    printf( "TNS: %f\n", m_TNS );
}

void TimingAnalysis::ResetSTA( void )
{
    //for( unsigned int i = 0 ; i < subCkts.size() ; i++ )
    //{
    //	reset( subCkts[i] );
    //}
  
    //PIOs  
    for( unsigned int i = 0 ; i < priOut.size() ; i++ )
    {
	Node& node = netlist[ priOut[i] ];
	
	node.voltageType = NVT;	// PIO has no voltage type
	node.clkLatency = 0.0;
	node.delay = 0.0;	// initialized as no external delay
	node.maxCD = 0.0;
	node.visited = false;
	node.Loadgradient = DBL_MAX;
	node.ATgradient = DBL_MAX;
	node.RTgradient = DBL_MAX;
	node.ATnetId = -1;
	node.RTnetId = -1;
    }
    
    
    //Components
    for( unsigned int i = 0 ; i < netlist.size() ; i++ )
    {
	Node& node = netlist[i];
	
	node.clkLatency = 0.0;
	node.delay = 0.0;
	node.maxCD = 0.0;
	node.requiredTime = DBL_MAX;
	node.visited = false;
	node.nVisitedFanin = 0;
	node.Loadgradient = DBL_MAX;
	node.ATgradient = DBL_MAX;
	node.RTgradient = DBL_MAX;
	node.ATnetId = -1;
	node.RTnetId = -1;
    }
    
    for( unsigned int i = 0 ; i < priIn.size() ; i++ )
    {
	Node& node = netlist[ priIn[i] ];
	
	node.voltageType = NVT;	// PIO has no voltage type
	node.clkLatency = 0.0;
	node.delay = 0.0;	// initialized as no external delay
	node.maxCD = 0.0;
	node.visited = false;
	node.Loadgradient = 0.0;
	node.ATgradient = DBL_MAX;
	node.RTgradient = DBL_MAX;
	node.ATnetId = -1;
	node.RTnetId = -1;
    }
    
    for( unsigned int i = 0 ; i < flipflop.size() ; i++ )
    {
	netlist[ flipflop[i] ].Loadgradient = 0.0;
    }
}
