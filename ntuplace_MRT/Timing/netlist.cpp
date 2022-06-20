#include "sta.h"
#include "netlist.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <climits>
#include <cfloat>

#ifdef VERBOSE
#include <iostream>
#include <sys/time.h>
#endif

using namespace std;
using STA::Net;
using STA::Node;

// reserved keywords
const string PINS = "PINS";
const string COMPONENTS = "COMPONENTS";                        
const string NET = "NET";                                      
const string END = "END";
const string IN = "IN";
const string OUT = "OUT";
const string INOUT = "INOUT";
const string CKBUF = "CKBUF";
const string CLOCK_CYCLE = "Clock_cycle";
const string INPUT_DELAY = "Input_delay";
const string OUTPUT_DELAY = "Output_delay";
const string OPT_NETLIST_FILE_NAME = "design_opt.netlist";
const string POWER_REPORT_FILE_NAME = "leakage.rpt";
// end of keywords

// global variables
vector<Node> netlist;		// the netlist
unsigned int nodeCount;		// # of nodes in the netlist, including PI & PO
unsigned int clkNodeId;		// id of the clock node (port)
double clkCycle;		// clock cycle time
vector<unsigned int> priIn;	// id of PI in the netlist
vector<unsigned int> priOut;	// id of PO in the netlist
vector<unsigned int> flipflop;	// id of DFF in the netlist
map<string, unsigned int> nodeIdMap;	// form node name to node id
map<string, unsigned int> netIdMap;	// form net name to net id
vector<Net> nets;		// net in the netlist
double initLeakage;		// initial leakage power (using standard Vt)
vector<SubCkt> subCkts;		// subcircuit (combinational circuit)
// end of global variables

void netlistAddPins(fstream &netlistFile, Liberty& cellLib)
{
	string pinName;

	while( netlistFile >> pinName ) {
		if( pinName == END ) {		// east PINS and return
			netlistFile >> pinName;
			break;
		}
		// otherwise, add a new node in the netlist
		Node node;
		node.cellName = pinName;
		string cellType;
		netlistFile >> cellType;
                // W.-P. 20070121
		node.cellTypePtr = cellLib.getCellType(cellType);

		node.voltageType = NVT;	// PIO has no voltage type
		node.clkLatency = 0.0;
		node.delay = 0.0;	// initialized as no external delay
		node.maxCD = 0.0;
		node.visited = false;
//		node.heapIndex = INT_MAX;
		netlist.push_back( node );
		nodeIdMap[ pinName ] = nodeCount;
		// add index into PI or PO
		if( node.cellTypePtr->name == IN )
			priIn.push_back( nodeCount );
		else if( node.cellTypePtr->name == OUT )
			priOut.push_back( nodeCount );
		else if( node.cellTypePtr->name == INOUT )
			cout << "inout pin: " << pinName << endl;
		nodeCount++;	// increase node count
	}
}

bool isCKBUF(const string& cellType)
{
	string token;

	for(unsigned int i=0; i<cellType.size() && i<CKBUF.size(); i++)
		token.push_back(cellType[i]);

	return (token == CKBUF);
}

void netlistAddComponents(fstream &netlistFile, Liberty& cellLib)
{
	string cellName;

	initLeakage = 0.0;	// set initial leakage power to be 0
	while( netlistFile >> cellName ) {
		if( cellName == END ) {		// eat COMPONENTS and return
			netlistFile >> cellName;
			break;
		}
		// otherwise, add a new node in the netlist
		Node node;
		node.cellName = cellName;
		string cellType;
		netlistFile >> cellType;
                // W.-P. 20070118
		node.cellTypePtr = cellLib.getCellType(cellType);
		// initialize Vt
		if( node.cellTypePtr->DFF == true )	// DFF
			node.voltageType = INIT_DFF_VT;
		else if( isCKBUF(cellType) )		// CKBUF
			node.voltageType = INIT_CKBUF_VT;
		else	// ordinary cell types
			node.voltageType = INIT_CELL_VT;
		initLeakage += node.cellTypePtr->svt_leakage;
		node.clkLatency = 0.0;
		node.delay = 0.0;
		node.maxCD = 0.0;
		
		//Modified by Jin 20081013
		//node.requiredTime = (double)INT_MAX;
		node.requiredTime = DBL_MAX;
	
		node.visited = false;
		node.nVisitedFanin = 0;
//		node.heapIndex = INT_MAX;
		netlist.push_back( node );
		nodeIdMap[ cellName ] = nodeCount;
		// add index into DFF
		if( node.cellTypePtr->DFF == true )
			flipflop.push_back( nodeCount );
		nodeCount++;	// increase node count
	}
}

void parseCellInstance(const string &token, unsigned int &nodeId, string &pin)
{
	unsigned int i;
	string cellName;

	for(i=0; i<token.size(); i++) {		// find node id
		if( token[i] == '.' )	break;
		cellName.push_back( token[i] );
	}
	nodeId = nodeIdMap[ cellName ];
	for(i++; i<token.size(); i++)		// find pin name
		pin.push_back( token[i] );
}

void netlistAddNet(fstream &netlistFile)
{
	string line;

	getline( netlistFile, line );		// eat the tailing EOL of NET
	while( getline( netlistFile, line ) ) {	// read an entire line
		istringstream iss(line);	// prepare a string stream
		// read net name
		string netName;
		iss >> netName;
		if( netName == END ) {		// eat NET and return
			netlistFile >> netName;
			break;
		}
		Net &net = nets[ netIdMap[netName] ];
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
		while( iss >> token ) {
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
	}
}

void parseNetlist(char *netlistFileName, Liberty& cellLib)
{
#ifdef VERBOSE
	cout << "Parsing netlist file... " << flush;
	timeval time;		// timer
	gettimeofday(&time, NULL);
	double st = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
#endif

	fstream netlistFile;	// open input netlist file
	netlistFile.open(netlistFileName, ios::in);

	nodeCount = 0;		// initialize node count
	string line;
	while( netlistFile >> line ) {
		if( line == PINS )
			netlistAddPins( netlistFile, cellLib );
		else if( line == COMPONENTS )
			netlistAddComponents( netlistFile, cellLib );
		else if( line == NET )
			netlistAddNet( netlistFile );
	}

	netlistFile.close();	// close input netlist file

#ifdef VERBOSE
	gettimeofday(&time, NULL);
        double et = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
	cout << "done in " << et-st << " sec" << endl;
	cout << "\t# of primary input:\t" << priIn.size() << endl;
	cout << "\t# of primary output:\t" << priOut.size() << endl;
	cout << "\t# of flipflop:\t\t" << flipflop.size() << endl;
	cout << "\t# of cell:\t\t"
	     <<	netlist.size() - priIn.size() - priOut.size() - flipflop.size()
	     << endl << endl;
#endif
}

void parseNetLoad(char *netLoadFileName)
{
#ifdef VERBOSE
	cout << "Parsing net load file... " << flush;
	timeval time;			// timer
	gettimeofday(&time, NULL);
	double st = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
#endif

	fstream netLoadFile;		// open input net load file
	netLoadFile.open( netLoadFileName, ios::in );

	unsigned int netCount = 0;	// initialize net count
	string line;
	while( getline( netLoadFile, line ) ) {
		if( line[0] == '#' )	continue;	// skip comment line
		istringstream iss(line);// prepare a string stream
		// add a new net
		Net net;
		iss >> net.name >> net.loadCap;
		nets.push_back( net );
		netIdMap[ net.name ] = netCount;
		netCount++;		// increase net count
	}

	netLoadFile.close();

#ifdef VERBOSE
	gettimeofday(&time, NULL);
        double et = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
	cout << "done in " << et-st << " sec" << endl;
	cout << "\t# of net:\t\t" << netCount << endl << endl;
#endif
}

void parseTimingConstraint(char *timingConstraintFileName)
{
#ifdef VERBOSE
	cout << "Parsing timing constraint file... " << flush;
	timeval time;			// timer
	gettimeofday(&time, NULL);
	double st = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
#endif

	fstream timingConstraintFile;	// open input timing constraint file
	timingConstraintFile.open( timingConstraintFileName, ios::in );

	string keyword;	// keyword
	while( timingConstraintFile >> keyword ) {
		if( keyword == CLOCK_CYCLE ) {	// clock port
			string clkName;
			timingConstraintFile >> clkName >> clkCycle;
			clkNodeId = nodeIdMap[clkName];
		}
		else if( keyword == INPUT_DELAY || keyword == OUTPUT_DELAY ) {
			string pinName;		// ordineray I/O port
			timingConstraintFile >> pinName;
			Node & node = netlist[ nodeIdMap[pinName] ];
			timingConstraintFile >> node.delay; // external delay
		}
		// else, error!
	}

	timingConstraintFile.close();

#ifdef VERBOSE
	gettimeofday(&time, NULL);
        double et = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
	cout << "done in " << et-st << " sec" << endl;
	cout << "\tclock port:\t\t" << netlist[clkNodeId].cellName << endl;
	cout << "\tclock cycle:\t\t" << clkCycle << endl << endl;
#endif
}
/*
void testNetlist()
{
	string cellName;	// input cell name and show its fanin & fanout
	while( cin >> cellName ) {
		Node &node = netlist[ nodeIdMap[cellName] ];
		cout << node.cellName << ' ' << node.cellTypePtr->name << endl;
		cout << "fanin: " << endl;
		map<string, Fanin>::iterator initr = node.faninMap.begin();
		for(; initr != node.faninMap.end(); initr++) {
			cout << initr->first << ' ' <<
				netlist[(initr->second).nodeId].cellName <<
				'.' << (initr->second).pin << endl;
		}
		map<string, Fanout>::iterator outitr = node.fanoutMap.begin();
		cout << "fanout: " << (outitr->second).loadCap << endl;
		for(; outitr != node.fanoutMap.end(); outitr++) {
			cout << outitr->first << ' ';
			vector<FanoutNodeInfo> &out = (outitr->second).nodes;
			for(unsigned int j=0; j<out.size(); j++) {
				cout <<  netlist[out[j].nodeId].cellName
					<< '.' << out[j].pin << ' ';
			}
			cout << endl;
		}
	}
}
*/
void dumpPins(fstream &newNetlistFile)
{
	for(unsigned int i=0; i<priIn.size(); i++) {	// dump PI
		Node &pi = netlist[ priIn[i] ];
		newNetlistFile << pi.cellName << ' ' << pi.cellTypePtr->name
				<< endl;
	}
	for(unsigned int i=0; i<priOut.size(); i++) {	// dump PO
		Node &po = netlist[ priOut[i] ];
		newNetlistFile << po.cellName << ' ' << po.cellTypePtr->name
				<< endl;
	}
}

void dumpComponents(fstream &newNetlistFile)
{
	for(unsigned int i=priIn.size()+priOut.size(); i<netlist.size(); i++) {
		Node &node = netlist[i];
		newNetlistFile << node.cellName << ' ';
		if(node.voltageType == SVT)
			newNetlistFile << node.cellTypePtr->name << endl;
		else if(node.voltageType == LVT)
			newNetlistFile << node.cellTypePtr->lvt_name << endl;
		else if(node.voltageType == HVT)
			newNetlistFile << node.cellTypePtr->hvt_name << endl;
		// else error!
	}
}

void dumpNet(fstream &newNetlistFile)
{
	for(unsigned int i=0; i<nets.size(); i++) {	// net
		Net &net = nets[i];
		newNetlistFile << net.name;
		for(unsigned int j=0; j<net.nodes.size(); j++) {	// node
			NetNodeInfo &nodeInfo = net.nodes[j];
			newNetlistFile << ' ';
			newNetlistFile << netlist[ nodeInfo.nodeId ].cellName;
			if( nodeInfo.pin.size() != 0 )	// not PI or PO
				newNetlistFile << '.' << nodeInfo.pin;
		}
		newNetlistFile << endl;
	}
}

void dumpOptimizedNetlist()
{
#ifdef VERBOSE
	cout << "Dumping optimized netlist... " << flush;
	timeval time;			// timer
	gettimeofday(&time, NULL);
	double st = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
#endif

	// open file to write
	fstream optNetlistFile;
	optNetlistFile.open( OPT_NETLIST_FILE_NAME.c_str(), ios::out );
	// dump pins
	optNetlistFile << PINS << endl;
	dumpPins( optNetlistFile );
	optNetlistFile << END << ' ' << PINS << endl;
	// dump components
	optNetlistFile << COMPONENTS << endl;
	dumpComponents( optNetlistFile );
	optNetlistFile << END << ' ' << COMPONENTS << endl;
	// dump net
	optNetlistFile << NET << endl;
	dumpNet( optNetlistFile );
	optNetlistFile << END << ' ' << NET << endl;
	// close file
	optNetlistFile.close();

#ifdef VERBOSE
	gettimeofday(&time, NULL);
        double et = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
	cout << "done in " << et-st << " sec" << endl;
	cout << "\tsee " << OPT_NETLIST_FILE_NAME << endl << endl;
#endif
}

void dumpPowerReport()
{
#ifdef VERBOSE
        cout << "Dumping power report... " << flush;
        timeval time;                   // timer
        gettimeofday(&time, NULL);
        double st = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
#endif
	
	// open file to write
	fstream powerReportFile;
	powerReportFile.open( POWER_REPORT_FILE_NAME.c_str(), ios::out );
	// report initial leakage
	powerReportFile << fixed << setprecision(2);	// set precision
	powerReportFile << "INITIAL_LEAKAGE " << initLeakage << "nW" << endl;
	// report optimized leakage
	double optLeakage = 0.0;
	for(unsigned int i=priIn.size()+priOut.size(); i<netlist.size(); i++) {
		const Node& node = netlist[i];
		if(node.voltageType == SVT)
			optLeakage += node.cellTypePtr->svt_leakage;
		else if(node.voltageType == LVT)
			optLeakage += node.cellTypePtr->lvt_leakage;
		else if(node.voltageType == HVT)
			optLeakage += node.cellTypePtr->hvt_leakage;
		// else PI or PO
	}
	powerReportFile << "OPTIMIZED_LEAKAGE " << optLeakage << "nW" << endl;
	// report reduced leakage
	powerReportFile << "REDUCED_LEAKAGE " << initLeakage - optLeakage <<
			"nW" << endl;
	// close file
	powerReportFile.close();

#ifdef VERBOSE
        gettimeofday(&time, NULL);
        double et = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
        cout << "done in " << et-st << " sec" << endl;
	double reduction = (initLeakage - optLeakage) / initLeakage * 100.0;
	cout << "\tpower reduction:\t" << reduction << '%' << endl;
        cout << "\tsee " << POWER_REPORT_FILE_NAME << endl << endl;
#endif
}

double loadCap(const Node& node, const string& pin, int voltageType)
{
	// assume: voltageType is either SVT or HVT
	// find out pinsMap
	map<string, pinInfo*>* pinsMap = NULL;
	if(voltageType == SVT)
		pinsMap = (node.cellTypePtr)->svt_pinsMap;
	else if(voltageType == HVT)
		pinsMap = (node.cellTypePtr)->hvt_pinsMap;
	// look up load capacitance
	return ((*pinsMap)[pin])->cap;
}

double loadCap(const Node& node, const string& pin)
{
	// find out pinsMap
	map<string, pinInfo*>* pinsMap = NULL;
	if(node.voltageType == SVT)
		pinsMap = (node.cellTypePtr)->svt_pinsMap;
	else if(node.voltageType == LVT)
		pinsMap = (node.cellTypePtr)->lvt_pinsMap;
	else if(node.voltageType == HVT)
		pinsMap = (node.cellTypePtr)->hvt_pinsMap;
	else if(node.voltageType == NVT)	// node is PO
		return 0.0;
	// look up load capacitance
	return ((*pinsMap)[pin])->cap;
}

void updateOutputLoad()
{
	for(unsigned int i=priIn.size()+priOut.size(); i<netlist.size(); i++) {
		Node& node = netlist[i];
		// for each output pin
		map<string, Fanout>::iterator itr = node.fanoutMap.begin();
		for(; itr!=node.fanoutMap.end(); itr++) {
			Fanout& outPin = itr->second;
			// for each fanout nodes
			vector<FanoutNodeInfo> fanouts = outPin.nodes;
			for(unsigned int j=0; j<fanouts.size(); j++) {
				const Node& n = netlist[fanouts[j].nodeId];
				string pin = fanouts[j].pin;	// driven pin
				outPin.loadCap += loadCap(n, pin);
			}
		}
	}
}

void searchNeighborsInFanout(Node& node, SubCkt& subCkt)
{
	map<string, Fanout>::const_iterator oItr = node.fanoutMap.begin();
	for(; oItr!=node.fanoutMap.end(); oItr++) {	//for each output pin
		const vector<FanoutNodeInfo>& desNodes = (oItr->second).nodes;
		for(unsigned int i=0; i<desNodes.size(); i++) {
			Node& desNode = netlist[ desNodes[i].nodeId ];
			if(desNode.cellTypePtr->name == OUT) {	// PO
				subCkt.pos.push_back(desNodes[i].nodeId);
				continue;
			}
			else if(desNode.cellTypePtr->DFF == true) {	// DFF
				subCkt.desdffs.push_back(desNodes[i].nodeId);
				continue;	// skip visiting the node
			}
			else if(!(desNode.visited)) {	// unvisited node
				desNode.visited = true;
				subCkt.nodes.push_back(desNodes[i].nodeId);
			}
		}
	}
}

void searchNeighborsInFanin(Node& node, SubCkt& subCkt)
{
	map<string, Fanin>::const_iterator iItr = node.faninMap.begin();
	for(; iItr!=node.faninMap.end(); iItr++) {	// foreach input pin
		Node& srcNode = netlist[ (iItr->second).nodeId ];
		if(srcNode.cellTypePtr->name == IN && !(srcNode.visited)) {// PI
			srcNode.visited = true;
			subCkt.pis.push_back((iItr->second).nodeId);
			searchNeighborsInFanout(srcNode, subCkt);
		}
		else if(srcNode.cellTypePtr->DFF && !(srcNode.visited)) { // DFF
			srcNode.visited = true;
			subCkt.srcdffs.push_back((iItr->second).nodeId);
			searchNeighborsInFanout(srcNode, subCkt);
		}
		else if(!(srcNode.visited)) {	// unvisited node
			srcNode.visited = true;
			subCkt.nodes.push_back((iItr->second).nodeId);
		}
	}
}

void decomposeSubCkt(unsigned int fstNodeId, SubCkt& subCkt)
{
	// push the first node into BFS queue
	netlist[fstNodeId].visited = true;
	subCkt.nodes.push_back(fstNodeId);
	unsigned int i = 0;	// top index in the BFS queue
	// BFS
	while(i < subCkt.nodes.size()) {
		Node& node = netlist[ subCkt.nodes[i] ];
		searchNeighborsInFanin(node, subCkt);
		searchNeighborsInFanout(node, subCkt);
		i++;	// pop queue
	}
}

void decomposeSubCkts()
{
#ifdef VERBOSE
        cout << "Decomposing subcircuits... " << flush;
        timeval time;           // timer
        gettimeofday(&time, NULL);
        double st = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
#endif
	
	for(unsigned int i=priIn.size()+priOut.size(); i<netlist.size(); i++) {
		Node& node = netlist[i];
		if(!(node.visited) && !(netlist[i].cellTypePtr->DFF)) {
			// decompose connected component (BFS)
			SubCkt subCkt;
			decomposeSubCkt(i, subCkt);
			subCkts.push_back(subCkt);
		}
	}
	// relax all visited nodes
//	for(unsigned int i=0; i<netlist.size(); i++)
//		netlist[i].visited = false;

#ifdef VERBOSE
        gettimeofday(&time, NULL);
        double et = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
        cout << "done in " << et-st << " sec" << endl;
/*	const SubCkt& subCkt = subCkts[74];
	for(unsigned int i=0; i<subCkt.nodes.size(); i++)
		cout << netlist[subCkt.nodes[i]].cellName << ' ';
	cout << endl;
	for(unsigned int i=0; i<subCkt.pis.size(); i++)
		cout << netlist[subCkt.pis[i]].cellName << ' ';
	cout << endl;
	for(unsigned int i=0; i<subCkt.srcdffs.size(); i++)
		cout << netlist[subCkt.srcdffs[i]].cellName << ' ';
	cout << endl;*/
/*	cout << "\tsubckt\t#cell\t#PI\t#srcDFF\tPO\tdesDFF" << endl;
	unsigned int totalNodes = 0;
	for(unsigned int i=0; i<subCkts.size(); i++) {
		const SubCkt& subCkt = subCkts[i];
		totalNodes += subCkt.nodes.size();
		cout << '\t' << i+1;
		cout << '\t' << subCkt.nodes.size();
		cout << '\t' << subCkt.pis.size();
		cout << '\t' << subCkt.srcdffs.size();
		cout << '\t' << subCkt.pos.size();
		cout << '\t' << subCkt.desdffs.size() << endl;
	}
	cout << "\ttotal # of cells:\t" << totalNodes << endl;
*/	cout << endl;
#endif
}

void readNetlist(char *loadFileName, char *netFileName, char *timingFileName,
		Liberty& cellLib)
{
	parseNetLoad( loadFileName );
	parseNetlist( netFileName, cellLib );
	parseTimingConstraint( timingFileName );
	updateOutputLoad();
	decomposeSubCkts();
	nodeIdMap.clear();
	netIdMap.clear();
}

