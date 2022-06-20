#include "sta.h"
#include "liberty.h"
#include "netlist.h"	
#include "heap.h"
//#include "verilog.h"
#include <queue>
#include <climits>

#ifdef VERBOSE
#include <iostream>
#include <sys/time.h>
#endif

using namespace std;
using STA::Node;

// external global variables
extern vector<Node> netlist;            // the netlist
extern vector<SubCkt> subCkts;		// subcircuit (combinational circuit)
extern unsigned int clkNodeId;		// id of the clock node (port)
extern double clkCycle;			// clock cycle time
extern const string IN = "IN";		// keyword
extern const string OUT = "OUT";	// keyword
// end of external global variables

// global variables
Liberty libParser;	// cell library parser
// end of global variables

void calculateClkLatency()
{
	queue<unsigned int> que;	// queue for topological sort
	// visit first level clock buffers
	// note CKBUF has only one input pin and only one output pin
	vector<FanoutNodeInfo>& ckbufs =
		((netlist[clkNodeId].fanoutMap.begin())->second).nodes;

	for(unsigned int i=0; i<ckbufs.size(); i++) {
		Node& ckbuf = netlist[ckbufs[i].nodeId];
		if(ckbuf.cellTypePtr->DFF) {	// a DFF; not a CKBUF
			ckbuf.clkLatency = netlist[clkNodeId].delay;// external
			continue;
		}
		// else must be a CKBUF; increase # of visited fanin
		ckbuf.nVisitedFanin++;
		// update maximum arrival time & corresponding input transition
		ckbuf.maxCD = netlist[clkNodeId].delay;	// exteranl delay
		((ckbuf.faninMap.begin())->second).transitionTime = 0.0;
		// push ckbuf into queue if all the fanins of ckbuf are visited
		if(ckbuf.nVisitedFanin == ckbuf.faninMap.size())
			que.push(ckbufs[i].nodeId);
	}
	
	// visit other clock buffers
	while(que.size() > 0) {
		unsigned int bufId = que.front();
		Node& buf = netlist[bufId];	// buf -> ckbufs
		// calculate buf rise delay
		map<string, Fanin>::iterator faninItr = buf.faninMap.begin();
		Fanout& fanout = (buf.fanoutMap.begin())->second;
		buf.delay = libParser.calculateTiming(buf.cellTypePtr,
				faninItr->first,// driven pin name
				(faninItr->second).transitionTime,
				fanout.loadCap,
				CELL_RISE,	// CKBUF is positive unate
				buf.voltageType);
		// calculate buf rise transition
		double bufTran = libParser.calculateTiming(buf.cellTypePtr,
					faninItr->first,// driven pin name
					(faninItr->second).transitionTime,
					fanout.loadCap,
					RISE_TRAN, // CKBUF is positive unate
					buf.voltageType);
		// arrival time for next stage (driven cell)
		double nextAT = buf.maxCD + buf.delay;
		// retrieve driven cells
		ckbufs = ((buf.fanoutMap.begin())->second).nodes;
		for(unsigned int i=0; i<ckbufs.size(); i++) {
			Node& ckbuf = netlist[ckbufs[i].nodeId];
			if(ckbuf.cellTypePtr->DFF) {	// a DFF; not a CKBUF
				ckbuf.clkLatency = nextAT;
				ckbuf.faninMap[ DFF_CLK_PIN ].transitionTime = bufTran;
				//ckbuf.faninMap["CP"].transitionTime = bufTran;
				continue;
			}
			// else must be a CKBUF; increase # of visited fanin
			ckbuf.nVisitedFanin++;
			// update maximum arrival time & input transition
			ckbuf.maxCD = nextAT;
			Fanin& fanin = (ckbuf.faninMap.begin())->second;
			fanin.transitionTime = bufTran;
			// if all the fanins of ckbuf are visited
			if(ckbuf.nVisitedFanin == ckbuf.faninMap.size())
				que.push(ckbufs[i].nodeId);
		}
		que.pop();
	}
}

void calculateNextStageTiming( const Node& srcNode, Node& desNode,
		const string& desPin, double srcDelay, double srcTran)
{
	// first to find out
	// 	1: the output pin of desNode
	// 	2: timing sense of the output pin related to desPin
	// assume
	// 	1: ordinary cells (neither PIO nor DFF) has only one output pin
	// 	2: desNode must be an ordinary cell
	// 	3: the output pin of desNode is either POS_UNATE or NEG_UNATE
	map<string, Fanout>::iterator outItr = desNode.fanoutMap.begin();
	const Fanout& fanout = outItr->second;
	pinInfo* outPin= (*(desNode.cellTypePtr->svt_pinsMap))[outItr->first];
	int timingSense = (outPin->timingSense)[desPin];
	// calculate timing for desNode	
	//TODO lack of delay of input net
	if(srcNode.cellTypePtr->name == IN) {	// PI (rising) -> desNode
		// determine signal directions
		int delayType = (timingSense == POS_UNATE) ?
				CELL_RISE : CELL_FALL;
		//Modified by Jin 20081014
		// calculate delay and maximum cumulated delay
		//double intDelay = libParser.calculateTiming(desNode.cellTypePtr,
		//		desPin, 0.0, fanout.loadCap, delayType,
		//		desNode.voltageType);	// srcTran == 0.0
		double gradient;
		//###
		double intDelay = libParser.calculateTimingwithGradient(desNode.cellTypePtr,
				desPin, 0.0, fanout.loadCap, delayType,
				desNode.voltageType, gradient);	// srcTran == 0.0
		desNode.Loadgradient = gradient;
		//double intDelay = libParser.calculateTiming(desNode.cellTypePtr,
		//		desPin, 0.0, fanout.loadCap, delayType,
		//		desNode.voltageType );	// srcTran == 0.0
		
		if(srcNode.delay + intDelay > desNode.maxCD) {
			desNode.delay = intDelay;
			desNode.maxCD = srcNode.delay +	// external delay
					intDelay;	// intrinsic delay
			// set propagated clock latency to be 0.0;
			desNode.clkLatency = 0.0;
			// setup input pin whose arrival time is the latest
			desNode.latestPin = desPin;
			// setup output signal type
			desNode.outputSignal = (delayType == CELL_RISE) ?
						RISING : FALLING;

			//Added by Jin 20081014
			desNode.ATgradient = srcNode.Loadgradient;
			const Fanout& srcFanout = srcNode.fanoutMap.begin()->second;
			//desNode.ATpin = desPin;
			desNode.ATnetId = srcFanout.netId;

			//printf( "srcNode %s desNode %s\n", srcNode.cellName.c_str(), desNode.cellName.c_str() );
			//printf( "IN  node %s AT net %d gradient %f\n", desNode.cellName.c_str(), desNode.ATnetId, desNode.ATgradient );
		}
		// set input transition time to be 0.0
		desNode.faninMap[desPin].transitionTime = 0.0; // srcTran == 0.0
	}
	else if(srcNode.cellTypePtr->DFF == true) { // DFF (rising) -> desNode
		// determine signal directions
		int delayType = (timingSense == POS_UNATE) ?
				CELL_RISE : CELL_FALL;
		//###
		// calculate delay and maximum cumulated delay
		double gradient;
		double intDelay = libParser.calculateTimingwithGradient(desNode.cellTypePtr,
				desPin, srcTran, fanout.loadCap, delayType,
				desNode.voltageType, gradient);
		desNode.Loadgradient = gradient;
		//double intDelay = libParser.calculateTiming(desNode.cellTypePtr,
		//		desPin, srcTran, fanout.loadCap, delayType,
		//		desNode.voltageType);
		if(srcDelay + intDelay > desNode.maxCD) { // need to update
			desNode.delay = intDelay; 
			desNode.maxCD = srcDelay +	// delay of DFF
					intDelay;	// intrinsic delay
			// propogate clock latency
			desNode.clkLatency = srcNode.clkLatency;
			// setup input pin whose arrival time is the latest
			desNode.latestPin = desPin;
			// setup output signal type
			desNode.outputSignal = (delayType == CELL_RISE) ?
						RISING : FALLING;
			
			//Added by Jin 20081014
			desNode.ATgradient = srcNode.Loadgradient;
			const Fanout& srcFanout = srcNode.fanoutMap.begin()->second;
			//desNode.ATpin = desPin;
			desNode.ATnetId = srcFanout.netId;

			//printf( "DFF node %s AT net %d gradient %f\n", desNode.cellName.c_str(), desNode.ATnetId, desNode.ATgradient );
		}
		// set input transition time equal to DFF's
		desNode.faninMap[desPin].transitionTime = srcTran;
	}
	else {	
		// srcNode -> desNode
		// determine signal directions
		int delayType;	// output delay type of desNode
		int tranType;	// output transition type of desNode
		if((timingSense==POS_UNATE && srcNode.outputSignal==RISING) || 
		   (timingSense==NEG_UNATE && srcNode.outputSignal==FALLING)) {
			// turn into rising
			delayType = CELL_RISE;
			tranType = RISE_TRAN;
		}
		else {	// turn into falling
			delayType = CELL_FALL;
			tranType = FALL_TRAN;
		}
		//###
		// calculate delay and maximum cumulated delay
		double gradient;
		double intDelay = libParser.calculateTimingwithGradient(desNode.cellTypePtr,
				desPin, srcTran, fanout.loadCap, delayType,
				desNode.voltageType, gradient);
		desNode.Loadgradient = gradient;
		//double intDelay = libParser.calculateTiming(desNode.cellTypePtr,
		//		desPin, srcTran, fanout.loadCap, delayType,
		//		desNode.voltageType);
		if(srcDelay + intDelay > desNode.maxCD) { // need to update
			desNode.delay = intDelay; 
			desNode.maxCD = srcDelay +	// delay of last stage
					intDelay;	// intrinsic delay
			// propogate clock latency
			desNode.clkLatency = srcNode.clkLatency;
			// setup input pin whose arrival time is the latest
			desNode.latestPin = desPin;
			// setup output signal type
			desNode.outputSignal = (delayType == CELL_RISE) ?
						RISING : FALLING;
			
			//Added by Jin 20081014
			desNode.ATgradient = srcNode.Loadgradient;
			const Fanout& srcFanout = srcNode.fanoutMap.begin()->second;
			//desNode.ATpin = desPin;
			desNode.ATnetId = srcFanout.netId;

			//printf( "OR node %s AT net %d gradient %f\n", desNode.cellName.c_str(), desNode.ATnetId, desNode.ATgradient );
		}
		// set input transition time equal to DFF's
		desNode.faninMap[desPin].transitionTime = srcTran;
	}
}

void calATFromPI(const Node& piNode, const vector<FanoutNodeInfo>& nodes,
		queue<unsigned int>& que)
{
	for(unsigned int i=0; i<nodes.size(); i++) {
		Node& desNode = netlist[nodes[i].nodeId];
		if(desNode.cellTypePtr->name == OUT || 	// visit a PO
		   desNode.cellTypePtr->DFF == true) {	// visit a DFF
			continue;
		}
		// else increase # of visited fanin
		desNode.nVisitedFanin++;
		if(desNode.nVisitedFanin == 1)	desNode.maxCD = 0.0;
		// setup clock latency, intrinsic delay, cumulated delay,
		// transition time, and output signal direction
		calculateNextStageTiming(piNode, desNode, nodes[i].pin, 0.0,
					0.0);
		// if all the fanins of the node are visited
		if(desNode.nVisitedFanin == desNode.faninMap.size())
		{
			que.push(nodes[i].nodeId);
		}
	}
}

void calATFromDFF(const Node& dffNode, const vector<FanoutNodeInfo>& nodes,
		queue<unsigned int>& que, double dffDelay, double dffTran )
{
	if(nodes.size() == 0)	return;
	
	for(unsigned int i=0; i<nodes.size(); i++) {
		Node& desNode = netlist[nodes[i].nodeId];
		if(desNode.cellTypePtr->name == OUT || 	// visit a PO
		   desNode.cellTypePtr->DFF == true) {	// visit a DFF
			continue;
		}
		// else increase # of visited fanin
		desNode.nVisitedFanin++;
		if(desNode.nVisitedFanin == 1)	desNode.maxCD = 0.0;
		// setup clock latency, intrinsic delay, cumulated delay,
		// transition time, and output signal direction
		calculateNextStageTiming(dffNode, desNode, nodes[i].pin,
					dffDelay, dffTran);
		// if all the fanins of the node are visited
		if(desNode.nVisitedFanin == desNode.faninMap.size())
		{
			que.push(nodes[i].nodeId);
		}

	}
}

bool checkTiming(Node& desNode, double srcTran)
{
	// assume
	// 	1: PO has only one input
	// 	2: DFF has only one data port "D"
	double delay;		// timing path delay
	double s_clk;		// clock launch latency
	double e_clk;		// clock capture latency
	double e_setup;		// setup time requirement
	
	if(desNode.cellTypePtr->name == OUT) {	// PO has only one input pin
		const Fanin& fanin = desNode.faninMap.begin()->second;
		const Node& srcNode = netlist[fanin.nodeId];
		delay = srcNode.maxCD + desNode.delay;	// plus external delay
		s_clk = srcNode.clkLatency;
		e_clk = 0.0;
		e_setup = 0.0;
	}
	else {	// must be DFF, which has only one input pin "D"
		const Fanin& fanin = desNode.faninMap["D"];
		const Node& srcNode = netlist[fanin.nodeId];
		delay = srcNode.maxCD;
		s_clk = srcNode.clkLatency;
		e_clk = desNode.clkLatency;
		int signalType = (srcNode.outputSignal == RISING) ?
				 SETUP_RISE : SETUP_FALL;
		e_setup = libParser.calculateTimingHoldSetup(
				desNode.cellTypePtr,
				desNode.faninMap[ DFF_CLK_PIN ].transitionTime,
				//desNode.faninMap["CP"].transitionTime,
				srcTran,	// input trantition of "D"
				signalType,
				desNode.voltageType);
		desNode.delay = e_setup;	// store setup time requirement
	}
	// check setup time requirement
	double setupSlack = clkCycle + e_clk - s_clk - delay - e_setup;
	if(setupSlack < 0.0)
		return TIMING_VIOLATED;
	else
		return TIMING_SATISFIED;
}

bool calATFromGivenNodes(queue<unsigned int>& que)
{
	while(que.size() > 0) {
		unsigned int nodeId = que.front();
		Node& srcNode = netlist[nodeId];	// node -> fanouts
		// retrieve desNodes driven by srcNode
		const Fanout& fanout = srcNode.fanoutMap.begin()->second;
		const vector<FanoutNodeInfo>& desNodes = fanout.nodes;
		// calculate fanin trantition for desNodes
		int tranType = (srcNode.outputSignal == RISING) ?
				RISE_TRAN : FALL_TRAN;
		double inTran =	// fanin transition of srcNode
			srcNode.faninMap[srcNode.latestPin].transitionTime;
		double srcTran = libParser.calculateTiming(srcNode.cellTypePtr,
					srcNode.latestPin,
					inTran,
					fanout.loadCap,
					tranType,
					srcNode.voltageType);
		// visit desNodes
		for(unsigned int i=0; i<desNodes.size(); i++) {
			Node& desNode = netlist[desNodes[i].nodeId];
			if(desNode.cellTypePtr->name == OUT || 	// visit a PO
			   desNode.cellTypePtr->DFF == true) {	// visit a DFF
				if(checkTiming(desNode, srcTran) ==
				   TIMING_VIOLATED)
					return TIMING_VIOLATED;
				continue;
			}
			// else increase # of visited fanin
			desNode.nVisitedFanin++;
			if(desNode.nVisitedFanin == 1)	desNode.maxCD = 0.0;
			// setup clock latency, intrinsic delay, cumulated delay
			// transition time, and output signal direction
			calculateNextStageTiming(srcNode, desNode,
				desNodes[i].pin, srcNode.maxCD, srcTran);
			// if all the fanins of ckbuf are visited
			if(desNode.nVisitedFanin == desNode.faninMap.size())
				que.push(desNodes[i].nodeId);
		}
		que.pop();
	}

	return TIMING_SATISFIED;
}

void calArrivalTimeSSTA(const SubCkt& subCkt)
{
	queue<unsigned int> que;	// queue for topological sort
	// visit first level nodes from PI
	for(unsigned int i=0; i<subCkt.pis.size(); i++) {
		Node& piNode = netlist[subCkt.pis[i]];	//piNode -> nodes
		vector<FanoutNodeInfo>& nodes =	// only one output pin of a PI
				(piNode.fanoutMap.begin()->second).nodes;
		calATFromPI(piNode, nodes, que);
	}

	
	// visit first level nodes form DFF
	for(unsigned int i=0; i<subCkt.srcdffs.size(); i++) {
		Node& dffNode = netlist[subCkt.srcdffs[i]];
		map<string, Fanout>::iterator outItr =dffNode.fanoutMap.begin();
		for(; outItr!=dffNode.fanoutMap.end(); outItr++) { // Q and QN				// calculate DFF rise delay
			// note: unnecessary to save in dffNode.delay
			Fanin& fanin = dffNode.faninMap[ DFF_CLK_PIN ];	// clock port
			//Fanin& fanin = dffNode.faninMap["CP"];	// clock port
			const string& outputPin = outItr->first;// Q or QN
			Fanout& fanout = dffNode.fanoutMap[outputPin];
			//Jin 20081015
			double DFFgradient;
			double dffDelay = libParser.calculateTimingDFFwithGradient(
						dffNode.cellTypePtr,
						outputPin,
						fanin.transitionTime,
						fanout.loadCap,
						CELL_RISE,
						dffNode.voltageType,
						DFFgradient );
			dffNode.Loadgradient = DFFgradient;
			// calculate DFF rise transition
			double dffTran = libParser.calculateTimingDFF(
						dffNode.cellTypePtr,
						outputPin,
						fanin.transitionTime,
						fanout.loadCap,
						RISE_TRAN,
						dffNode.voltageType);
			//Jin 20081015
			// visit nodes from DFF
			calATFromDFF(dffNode, (outItr->second).nodes, que,
					dffDelay, dffTran );
		}
	}
	// visit other nodes
	calATFromGivenNodes(que);
}

void calRTFromPO(const SubCkt& subCkt, queue<unsigned int>& que)
{
	for(unsigned int i=0; i<subCkt.pos.size(); i++) {
		Node& poNode = netlist[subCkt.pos[i]];	//poNode -> nodes
		unsigned int desNodeId =	// only on fanin node per PO
			(poNode.faninMap.begin()->second).nodeId;
		Node& desNode =	netlist[desNodeId];
		if(desNode.cellTypePtr->name == IN ||	// a PI
		   desNode.cellTypePtr->DFF == true) {	// a DFF
			continue;
		}
		// delay = clkCycle + e_clk - s_clk - e_setup
		// (int+ext)		0		0
		desNode.requiredTime = clkCycle - poNode.delay // minus external
					- desNode.clkLatency;
		desNode.slack = desNode.requiredTime - desNode.maxCD;
			
		//Added by Jin 20081014
		desNode.RTgradient = 0.0;
		//desNode.RTnode = dffNode.cellName;
		desNode.RTnetId = (poNode.faninMap.begin()->second).netId;
		//printf( "PO  node %s net %d gradient %f\n", desNode.cellName.c_str(), desNode.RTnetId, desNode.RTgradient );
		
		// if all the fanins (fanouts here) are visited
		desNode.nVisitedFanin++;
		if(desNode.nVisitedFanin ==
			(desNode.fanoutMap.begin()->second).nodes.size())
			que.push(desNodeId);
	}
}

void calRTFromDFF(const SubCkt& subCkt, queue<unsigned int>& que)
{
	for(unsigned int i=0; i<subCkt.desdffs.size(); i++) {
		Node& dffNode = netlist[subCkt.desdffs[i]];
		unsigned int desNodeId = dffNode.faninMap["D"].nodeId;
		Node& desNode = netlist[desNodeId];
		if(desNode.cellTypePtr->name == IN ||	// a PI
		   desNode.cellTypePtr->DFF == true) {	// a DFF
			continue;
		}
		// delay = clkCycle + e_clk - s_clk - e_setup
		// 				     dffNode.delay
		double newRT = clkCycle + dffNode.clkLatency -
				desNode.clkLatency - dffNode.delay;
		if(newRT < desNode.requiredTime) {
			desNode.requiredTime = newRT;
			desNode.slack = newRT - desNode.maxCD;

			//Added by Jin 20081014
			desNode.RTgradient = dffNode.Loadgradient;
			//desNode.RTnode = dffNode.cellName;
			desNode.RTnetId = dffNode.faninMap["D"].netId;
			
			//printf( "DFF node %s net %d gradient %f\n", desNode.cellName.c_str(), desNode.RTnetId, desNode.RTgradient );
		}
		// if all the fanins (fanouts here) are visited
		desNode.nVisitedFanin++;
		if(desNode.nVisitedFanin ==
			(desNode.fanoutMap.begin()->second).nodes.size())
			que.push(desNodeId);
	}
}

void calRTFromGivenNodes(queue<unsigned int>& que)
{
	while(que.size() > 0) {
		unsigned int nodeId = que.front();
		Node& srcNode = netlist[nodeId];	// srcNode -> fanins
		// for each input pin
		map<string, Fanin>::iterator inItr = srcNode.faninMap.begin();
		for(; inItr!=srcNode.faninMap.end(); inItr++) {
			unsigned int desNodeId = (inItr->second).nodeId;
			Node& desNode = netlist[desNodeId];
			if(desNode.cellTypePtr->name == IN ||	// a PI
			   desNode.cellTypePtr->DFF == true) {	// a DFF
				continue;
			}
			// else calculate required time & slack
			double newRT = srcNode.requiredTime - srcNode.delay;
			if(newRT < desNode.requiredTime) {
				desNode.requiredTime = newRT;
				desNode.slack = newRT - desNode.maxCD;
				
				//Added by Jin 20081014
				desNode.RTgradient = desNode.Loadgradient;
				//desNode.RTnode = dffNode.cellName;
			//	//desNode.RTnetId = dffNode.faninMap["D"].netId;
			
				//Added by Jin 20081014
				desNode.RTgradient = srcNode.Loadgradient;
				//desNode.RTnode = dffNode.cellName;
				desNode.RTnetId = inItr->second.netId;
			
			//printf( "OR node %s net %d gradient %f\n", desNode.cellName.c_str(), desNode.RTnetId, desNode.RTgradient );
			
			//printf( "node %s net %d gradient %f\n", desNode.cellName.c_str(), desNode.RTnetId, desNode.RTgradient );
			}
			// increase # of visited fanins (actually, fanouts here)
			desNode.nVisitedFanin++;
			// if all the fanins (fanouts here) are visited
			if(desNode.nVisitedFanin ==
			   (desNode.fanoutMap.begin()->second).nodes.size())
				que.push(desNodeId);
		}
		que.pop();
	}
}

void calRequiredTimeSSTA(const SubCkt& subCkt)
{
	queue<unsigned int> que;	// queue for inverse topological sort
	// reset # of visited fanin nodes (actually, fanout nodes here)
	for(unsigned int i=0; i<subCkt.nodes.size(); i++) {
		netlist[subCkt.nodes[i]].nVisitedFanin = 0;
		//Modified by Jin 20081013
		//netlist[subCkt.nodes[i]].requiredTime = (double)INT_MAX;
		netlist[subCkt.nodes[i]].requiredTime = DBL_MAX;
	}
	// visit last level nodes from PO
	calRTFromPO(subCkt, que);
	// visit last level nodes form DFF
	calRTFromDFF(subCkt, que);
	// visit other nodes
	calRTFromGivenNodes(que);
}

void fullSSTA()
{
#ifdef VERBOSE
        cout << "Performing full static timing analysis... " << flush;
        timeval time;           // timer
        gettimeofday(&time, NULL);
        double st = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
#endif

	// calculate clock latency
	calculateClkLatency();
	// STA
	for(unsigned int i=0; i<subCkts.size(); i++) {
		SubCkt& subCkt = subCkts[i];
		if(subCkt.pis.size() == 1)	// skip the CKBUF's
			if(subCkt.pis[0] == clkNodeId)
				continue;

		calArrivalTimeSSTA(subCkt);	// arrival time

		//test code
		for( unsigned int n = 0 ; n < subCkt.nodes.size() ; n++ )
		{
		    Node& node = netlist[ subCkt.nodes[n] ];
		    if( node.nVisitedFanin != node.faninMap.size() )
		    {
			cout << "######### " << node.cellName << " " << node.nVisitedFanin << " " << node.faninMap.size() << endl;
		    }
		}
		//@test code
		
		calRequiredTimeSSTA(subCkt);	// required tiem & slack
	}

#ifdef VERBOSE
        gettimeofday(&time, NULL);
        double et = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
        cout << "done in " << et-st << " sec" << endl << endl;
#endif
}

inline int getTimingSense(Node& node, const string& inputPin)
{
	map<string, Fanout>::iterator outItr = node.fanoutMap.begin();
	pinInfo* outPin= (*(node.cellTypePtr->svt_pinsMap))[outItr->first];
	return (outPin->timingSense)[inputPin];
}

void calTempATFromPI(const Node& srcNode, Node& desNode,
		const string& inputPin, unsigned int voltageType,
		double& maxCD, double& desTran, int& outputSignal)
{
	// calculate srcNode delay
	double srcDelay = srcNode.delay;	// external delay
	// calculate desNode delay
	int timingSense = getTimingSense(desNode, inputPin);
	int delayType = (timingSense == POS_UNATE) ? CELL_RISE : CELL_FALL;
	const Fanout& fanout = desNode.fanoutMap.begin()->second;
	double desDelay = libParser.calculateTiming(desNode.cellTypePtr,
				inputPin, 0.0, fanout.loadCap, delayType,
				voltageType);
	// update arrival time and transition time of desNode
	if(srcDelay + desDelay > maxCD) {
		maxCD = srcDelay + desDelay;
		int tranType = (delayType == CELL_RISE) ? RISE_TRAN : FALL_TRAN;
		desTran = libParser.calculateTiming(desNode.cellTypePtr,
				inputPin, 0.0, fanout.loadCap, tranType,
				voltageType);
		outputSignal = (delayType == CELL_RISE) ? RISING : FALLING;
	}
}

void calTempATFromDFF(Node& srcNode, Node& desNode, const string& inputPin,
		const string& outputPin, unsigned int voltageType,
		double& maxCD, double& desTran, int& outputSignal)
{
	// update output load of srcNode
	double originLoadCap = loadCap(desNode, inputPin);
	double newLoadCap = loadCap(desNode, inputPin, voltageType);
	const Fanout& srcFanout = srcNode.fanoutMap[outputPin];	// Q or QN
	double srcLoadCap = srcFanout.loadCap - originLoadCap + newLoadCap;
	// calculate srcNode delay
	const Fanin& srcFanin = srcNode.faninMap[ DFF_CLK_PIN ];	// clock port
	//const Fanin& srcFanin = srcNode.faninMap["CP"];	// clock port
	double srcDelay = libParser.calculateTimingDFF(srcNode.cellTypePtr,
				outputPin, srcFanin.transitionTime, srcLoadCap,
				CELL_RISE, INIT_DFF_VT);
	// calculate srcNode transition time
	double srcTran = libParser.calculateTimingDFF(srcNode.cellTypePtr,
				outputPin, srcFanin.transitionTime, srcLoadCap,
				RISE_TRAN, INIT_DFF_VT);
	// calculate desNode delay
	int timingSense = getTimingSense(desNode, inputPin);
	int delayType;	// output delay type of desNode
	int tranType;	// transition type of desNode
	if(timingSense == POS_UNATE) {	// keep rising
		delayType = CELL_RISE;
		tranType = RISE_TRAN;
	}
	else {	// turn into falling
		delayType = CELL_FALL;
		tranType = FALL_TRAN;
	}
	const Fanout& desFanout = desNode.fanoutMap.begin()->second;
	double desDelay = libParser.calculateTiming(desNode.cellTypePtr,
				inputPin, srcTran, desFanout.loadCap,
				delayType, voltageType);
	// update arrival time and transition time of desNode
	if(srcDelay + desDelay > maxCD) {
		maxCD = srcDelay + desDelay;
		desTran = libParser.calculateTiming(desNode.cellTypePtr,
				inputPin, srcTran, desFanout.loadCap,
				tranType, voltageType);
		outputSignal = (delayType == CELL_RISE) ? RISING : FALLING;
	}
}

void calTempATFromCell(Node& srcNode, Node& desNode, const string& inputPin,
		unsigned int voltageType, double& maxCD, double& desTran,
		int& outputSignal)
{
	// update load capacitance of srcNode
	double originLoadCap = loadCap(desNode, inputPin);
	double newLoadCap = loadCap(desNode, inputPin, voltageType);
	const Fanout& srcFanout = srcNode.fanoutMap.begin()->second;
	double srcLoadCap = srcFanout.loadCap - originLoadCap + newLoadCap;
	// calculate source delay (source maxCD) & transition time
	// assume the latestPin of srcNode would not change (not accurate!)
	const Fanin& srcFanin = srcNode.faninMap[srcNode.latestPin];
	int srcDelayType;
	int srcTranType;
	if(srcNode.outputSignal == RISING) {
		srcDelayType = CELL_RISE;
		srcTranType = RISE_TRAN;
	}
	else {
		srcDelayType = CELL_FALL;
		srcTranType = FALL_TRAN;
	}
	double srcDelay = srcNode.maxCD - srcNode.delay
			+ libParser.calculateTiming(srcNode.cellTypePtr,
				srcNode.latestPin, srcFanin.transitionTime,
				srcLoadCap, srcDelayType, srcNode.voltageType);
	double srcTran = libParser.calculateTiming(srcNode.cellTypePtr,
				srcNode.latestPin, srcFanin.transitionTime,
				srcLoadCap, srcTranType, srcNode.voltageType);
	// update desNode arrival time and transition time
	const Fanout& desFanout = desNode.fanoutMap.begin()->second;
	int timingSense = getTimingSense(desNode, inputPin);
	int desDelayType;
	int desTranType;
	if((srcNode.outputSignal == RISING && timingSense == POS_UNATE) ||
	    (srcNode.outputSignal == FALLING && timingSense == NEG_UNATE)) {
		desDelayType = CELL_RISE;
		desTranType = RISE_TRAN;
	}
	else {
		desDelayType = CELL_FALL;
		desTranType = FALL_TRAN;
	}
	double desDelay = libParser.calculateTiming(desNode.cellTypePtr,
				inputPin, srcTran, desFanout.loadCap,
				desDelayType, voltageType);
	if(srcDelay + desDelay > maxCD) {
		maxCD = srcDelay + desDelay;
		desTran = libParser.calculateTiming(desNode.cellTypePtr,
				inputPin, srcTran, desFanout.loadCap,
				desTranType, voltageType);
		outputSignal = (desDelayType == CELL_RISE) ? RISING : FALLING;
	}	
}

void calTempRTFromPO(const Node& desNode, const Node& nextNode,
		double& requiredTime)
{
	// calculate required time
	double newRT = clkCycle - desNode.clkLatency
			- nextNode.delay;	// external delay
	if(newRT < requiredTime)
		requiredTime = newRT;
}

void calTempRTFromDFF(const Node& desNode, Node& nextNode, int outputSignal,
		double desTran, double& requiredTime)
{
	// calculate setup requirement
	int signalType = (outputSignal == RISING) ?
			 SETUP_RISE : SETUP_FALL;
	double newSetup = libParser.calculateTimingHoldSetup(
				nextNode.cellTypePtr,
				nextNode.faninMap[ DFF_CLK_PIN ].transitionTime,
				//nextNode.faninMap["CP"].transitionTime,
				desTran,
				signalType,
				INIT_DFF_VT);
	double newRT = clkCycle + nextNode.clkLatency - desNode.clkLatency
			- newSetup;
	if(newRT < requiredTime)
		requiredTime = newRT;
}

void calTempRTFromCell(Node& nextNode, const string& inputPin, int outputSignal,
		double desTran, double& requiredTime)
{
	// calculate nextNode delay
	int timingSense = getTimingSense(nextNode, inputPin);
	int signalType;
	if((outputSignal == RISING && timingSense == POS_UNATE) ||
	   (outputSignal == FALLING && timingSense == NEG_UNATE))
		signalType = CELL_RISE;
	else
		signalType = CELL_FALL;
	double nextDelay = libParser.calculateTiming(nextNode.cellTypePtr,
				inputPin,
				desTran,
				(nextNode.fanoutMap.begin()->second).loadCap,
				signalType,
				nextNode.voltageType);
	// assume the change in transition would not propagate (not accurate!)
	double newRT = nextNode.requiredTime - nextDelay;
	if(newRT < requiredTime)
		requiredTime = newRT;
}

double calSensitivity(unsigned int nid, unsigned int voltageType)
{
	// srcNode -> desNode -> nextNode
	Node& desNode = netlist[nid];
	double maxCD = 0.0;			// arrival time
	double desTran;				// output transition of desNode
	int outputSignal;			// output signal type of desNode
	//Modified by Jin 20081013
	//double requiredTime = (double)INT_MAX;	// required time
	double requiredTime = DBL_MAX;	// required time
	// for each srcNode, update input transition & arrival time
	map<string, Fanin>::const_iterator inItr = desNode.faninMap.begin();
	for(; inItr!=desNode.faninMap.end(); inItr++) {
		const string& inputPin = inItr->first;	// input pin of desNode
		Node& srcNode = netlist[ (inItr->second).nodeId ];
		if(srcNode.cellTypePtr->name == IN)	// a PI
			calTempATFromPI(srcNode, desNode, inputPin, voltageType,
					maxCD, desTran, outputSignal);
		else if(srcNode.cellTypePtr->DFF == true) {	// a DFF
			const string& outputPin = (inItr->second).pin;
			calTempATFromDFF(srcNode, desNode, inputPin, outputPin,
					voltageType, maxCD, desTran,
					outputSignal);
		}
		else	// an ordinary cell
			calTempATFromCell(srcNode, desNode, inputPin,
					voltageType, maxCD, desTran,
					outputSignal);
	}
	// for each nextNode, update input transition & required time
	const vector<FanoutNodeInfo>& fanouts =
		(desNode.fanoutMap.begin()->second).nodes;
	for(unsigned int i=0; i<fanouts.size(); i++) {
		Node &nextNode = netlist[ fanouts[i].nodeId ];
		if(nextNode.cellTypePtr->name == OUT)	// a PO
			calTempRTFromPO(desNode, nextNode, requiredTime);
		else if(nextNode.cellTypePtr->DFF == true)	// a DFF
			calTempRTFromDFF(desNode, nextNode, outputSignal,
					desTran, requiredTime);
		else	// an ordinary cell
			calTempRTFromCell(nextNode, fanouts[i].pin,
					outputSignal, desTran, requiredTime);
	}
	// calculate sensitivity
	double oldSlack = desNode.slack;
	double newSlack = requiredTime - maxCD;
	// retrieve new leakage
	double oldLeakage = desNode.cellTypePtr->svt_leakage;
	double newLeakage = (voltageType == SVT) ?
				desNode.cellTypePtr->svt_leakage :
				desNode.cellTypePtr->hvt_leakage;
	unsigned int p = desNode.faninMap.size() + fanouts.size();

	return (oldLeakage - newLeakage) / (oldSlack - newSlack + p);
//	return (oldLeakage - newLeakage) / (oldSlack - newSlack);
}

void buildSensitivityHeap(const SubCkt& subCkt, Heap& heap)
{
	for(unsigned int i=0; i<subCkt.nodes.size(); i++) {
		HeapItem item;
		item.nodeId = subCkt.nodes[i];
		item.s = calSensitivity(subCkt.nodes[i], HVT);
		heap.push_back(item);
		// store heap index for the node
//		netlist[item.nodeId].heapIndex = i;
	}
	// build a max heap
	buildMaxHeap(heap);
}

bool calATFromCell(Node& node, const Fanout& fanout, queue<unsigned int>& que)
{
	// srcNode -> node -> desNode
	// update the delay of node
	map<string, Fanin>::iterator inItr = node.faninMap.begin();
	for(; inItr!=node.faninMap.end(); inItr++) {
		const Fanin& fanin = inItr->second;
		Node& srcNode = netlist[fanin.nodeId];
		// calculate CD of srcNode
		double srcCD;
		if(srcNode.cellTypePtr->name == IN)
			srcCD = srcNode.delay;	// external delay
		else if(srcNode.cellTypePtr->DFF == true) {
			Fanin& srcFanin = srcNode.faninMap[ DFF_CLK_PIN ]; // clock port
			//Fanin& srcFanin = srcNode.faninMap["CP"]; // clock port
			const string& dffOutputPin = fanin.pin; // Q or QN
			Fanout& srcFanout = srcNode.fanoutMap[dffOutputPin];
			srcCD = libParser.calculateTimingDFF(
				srcNode.cellTypePtr, dffOutputPin,
				srcFanin.transitionTime, srcFanout.loadCap,
				CELL_RISE, srcNode.voltageType);
		}
		else
			srcCD = srcNode.maxCD;
		// update CD of node
		calculateNextStageTiming(srcNode, node, inItr->first, srcCD,
					fanin.transitionTime);
	}
	// srcNode -> node -> desNode
	// calculate fanin trantition for desNodes
	int tranType = (node.outputSignal == RISING) ? RISE_TRAN : FALL_TRAN;
	double inTran =	node.faninMap[node.latestPin].transitionTime;
	double outTran = libParser.calculateTiming(node.cellTypePtr,
			node.latestPin,	inTran,	fanout.loadCap,	tranType,
			node.voltageType);
	// update the delay of fanout node & queue in
	const vector<FanoutNodeInfo>& desNodes = fanout.nodes;
	for(unsigned int i=0; i<desNodes.size(); i++) {
		Node& desNode = netlist[desNodes[i].nodeId];
		if(desNode.cellTypePtr->name == OUT || 	// visit a PO
		   desNode.cellTypePtr->DFF == true) {	// visit a DFF
			if(checkTiming(desNode, outTran) == TIMING_VIOLATED)
				return TIMING_VIOLATED;
			continue;
		}
		// else must be a CKBUF; increase # of visited fanin
		desNode.nVisitedFanin++;
		if(desNode.nVisitedFanin == 1)	desNode.maxCD = 0.0;
		// setup clock latency, intrinsic delay, cumulated delay
		// transition time, and output signal direction
		calculateNextStageTiming(node, desNode,	desNodes[i].pin,
				node.maxCD, outTran);
		// if all the fanins of ckbuf are visited
		if(desNode.nVisitedFanin == desNode.faninMap.size())
			que.push(desNodes[i].nodeId);
	}

	return TIMING_SATISFIED;
}

bool updateFaninTiming(Node& srcNode, Node& swapNode, const string& inputPin,
		queue<unsigned int>& que)
{
	// update output loading of srcNode
	const Fanin& swapFanin = swapNode.faninMap[inputPin];
	Fanout& srcFanout = srcNode.fanoutMap[swapFanin.pin];
	srcFanout.loadCap -= loadCap(swapNode, inputPin, SVT);	// minus orignal
	srcFanout.loadCap += loadCap(swapNode, inputPin, HVT);
	// calculate new maxCD of srcNode
	if(srcNode.cellTypePtr->name == IN) {	// PI
		calATFromPI(srcNode, srcFanout.nodes, que);
	}
	else if(srcNode.cellTypePtr->DFF == true) {	// DFF
		Fanin& srcFanin = srcNode.faninMap[ DFF_CLK_PIN ];	// clock port
		//Fanin& srcFanin = srcNode.faninMap["CP"];	// clock port
		const string& dffOutputPin = swapFanin.pin;	// Q or QN
		double dffDelay = libParser.calculateTimingDFF(
				srcNode.cellTypePtr, dffOutputPin,
				srcFanin.transitionTime, srcFanout.loadCap,
				CELL_RISE, srcNode.voltageType);
		double dffTran = libParser.calculateTimingDFF(
				srcNode.cellTypePtr, dffOutputPin,
				srcFanin.transitionTime, srcFanout.loadCap,
				RISE_TRAN, srcNode.voltageType);
		calATFromDFF(srcNode, srcFanout.nodes, que, dffDelay, dffTran);
	}
	else {	// from an ordinary cell
		srcNode.maxCD = 0.0;
		if(calATFromCell(srcNode, srcFanout, que) == TIMING_VIOLATED)
			return TIMING_VIOLATED;
	}

	return TIMING_SATISFIED;
}

bool swapCell(unsigned int swapNodeId)
{
	// swapping starts
	Node& swapNode = netlist[swapNodeId];	// node to be swapped
	swapNode.voltageType = HVT;
	// update timing of the fanin of the node to be swapped
	queue<unsigned int> que;	// queue for topological sort
	map<string, Fanin>::iterator inItr = swapNode.faninMap.begin();
	for(; inItr!=swapNode.faninMap.end(); inItr++) {
		unsigned int srcNodeId = (inItr->second).nodeId;
		Node& srcNode = netlist[srcNodeId];	// srcNode -> swapNode
		if(updateFaninTiming(srcNode, swapNode, inItr->first, que)
		   == TIMING_VIOLATED)
			return TIMING_VIOLATED;
	}
	// perform incremental STA
	return calATFromGivenNodes(que);
}

void storeState(vector<State>& states, unsigned int nid)
{
	const Node& node = netlist[nid];	// node to be stored
	State state;

	state.nodeId = nid;
	map<string, Fanin>::const_iterator inItr = node.faninMap.begin();
	for(; inItr!=node.faninMap.end(); inItr++)
		state.tran.push_back((inItr->second).transitionTime);
	state.delay = node.delay;
	state.maxCD = node.maxCD;
	state.clkLatency = node.clkLatency;
	state.latestPin = node.latestPin;
	state.outputSignal = node.outputSignal;
	
	states.push_back(state);
}

void storeFanoutConeStates(vector<State>& states, unsigned int srcNodeId)
{
	queue<unsigned int> que;	// queue for BFS
	// push srcNode first
	netlist[srcNodeId].visited = true;
	storeState(states, srcNodeId);
	que.push(srcNodeId);
	// BFS in the fanout cone of srcNode
	while(que.size() > 0) {
		unsigned int srcNodeId = que.front();
		const Node& srcNode = netlist[srcNodeId];
		if(srcNode.cellTypePtr->name != OUT &&	// neither PO
		   srcNode.cellTypePtr->DFF == false) {	// nor DFF
			const vector<FanoutNodeInfo>& desNodes
				= (srcNode.fanoutMap.begin()->second).nodes;
			for(unsigned int i=0; i<desNodes.size(); i++) {
				unsigned int desNodeId = desNodes[i].nodeId;
				Node& desNode = netlist[desNodeId];
				desNode.nVisitedFanin--;
				if(desNode.visited == false) {
					desNode.visited = true;
					storeState(states, desNodeId);
					que.push(desNodeId);
				}
			}
		}
		que.pop();
	}
}

void reset(const SubCkt& subCkt)
{
	const vector<unsigned int>& nodes = subCkt.nodes;	// nodes
	for(unsigned int i=0; i<nodes.size(); i++) {
		Node& node = netlist[nodes[i]];
		node.visited = false;
		node.nVisitedFanin = node.faninMap.size();
	}
	const vector<unsigned int>& pos = subCkt.pos;		// POs
	for(unsigned int i=0; i<pos.size(); i++)
		netlist[pos[i]].visited = false;
	const vector<unsigned int>& desdffs = subCkt.desdffs;	// desDFFs
	for(unsigned int i=0; i<desdffs.size(); i++)
		netlist[desdffs[i]].visited = false;
}

void storeStates(const SubCkt& subCkt, vector<State>& states,
		unsigned int swapNodeId)
{
	reset(subCkt);	// reset visited nVisitedFanin for state backup
	// collect for the fanin node of swapNode
	const Node& swapNode = netlist[swapNodeId];
	map<string, Fanin>::const_iterator inItr = swapNode.faninMap.begin();
	for(; inItr!=swapNode.faninMap.end(); inItr++) {
		unsigned int srcNodeId = (inItr->second).nodeId;
		storeFanoutConeStates(states, srcNodeId);
	}
}

void restoreStates(vector<State>& states)
{
	for(unsigned int i=0; i<states.size(); i++) {
		const State& state = states[i];
		Node& node = netlist[state.nodeId];	// node to be recover
		// recover
		map<string, Fanin>::iterator inItr = node.faninMap.begin();
		for(unsigned int j=0; inItr!=node.faninMap.end(); inItr++,j++)
			(inItr->second).transitionTime = state.tran[j];
		node.delay = state.delay;
		node.maxCD = state.maxCD;
		node.clkLatency = state.clkLatency;
		node.latestPin = state.latestPin;
		node.outputSignal = state.outputSignal;
	}
}
/*
void updateSensitivity(Heap& heap, unsigned int nid)
{
	// update fanin sensitivity
	const Node& node = netlist[nid];
	map<string, Fanin>::const_iterator inItr = node.faninMap.begin();
	for(; inItr!=node.faninMap.end(); inItr++) {
		unsigned int srcNodeId = (inItr->second).nodeId;
		const Node& srcNode = netlist[srcNodeId];
		if(srcNode.heapIndex != INT_MAX) {	// srcNode is in heap
			double newS = calSensitivity(srcNodeId, HVT);
			heapChangeS(heap, srcNode.heapIndex, newS);
		}
	}
	// update fanout sensitivity
	const vector<FanoutNodeInfo>& desNodes =
		(node.fanoutMap.begin()->second).nodes;
	for(unsigned int i=0; i<desNodes.size(); i++) {
		unsigned int desNodeId = desNodes[i].nodeId;
		const Node& desNode = netlist[desNodeId];
		if(desNode.heapIndex != INT_MAX) {
			double newS = calSensitivity(desNodeId, HVT);
			heapChangeS(heap, desNode.heapIndex, newS);
		}
	}
}
*/
void restoreLoadCap(unsigned int nodeId)
{
	// restore load capacitance of fanin nodes
	const Node& node = netlist[nodeId];
	map<string, Fanin>::const_iterator inItr = node.faninMap.begin();
	for(; inItr!=node.faninMap.end(); inItr++) {
		const string& inputPin = inItr->first;		// driven pin
		double SVTLoadCap = loadCap(node, inputPin, SVT);
		double HVTLoadCap = loadCap(node, inputPin, HVT);
		// retrieve source node
		unsigned int srcNodeId = (inItr->second).nodeId;
		Node& srcNode = netlist[srcNodeId];
		// restore output load of driving pin
		const string& srcPin = (inItr->second).pin;	// driving pin
		Fanout& srcFanout = srcNode.fanoutMap[srcPin];
		srcFanout.loadCap -= HVTLoadCap;
		srcFanout.loadCap += SVTLoadCap;
	}
}

void greedyVoltageAssignment(const SubCkt& subCkt)
{
#ifdef VERBOSE
	unsigned int numCandidate = subCkt.nodes.size();
	unsigned int numSuccess = 0;
	unsigned int numFail = 0;
#endif

	Heap maxHeap;
	// calculate sensitivity
	buildSensitivityHeap(subCkt, maxHeap);
	// assign Vt
	while(maxHeap.size() > 0) {
		HeapItem cell = heapExtractMax(maxHeap);
		if(cell.s < 0.0)	break;
		// else swap cell with incremental STA
		// first store states from the fanin of the node to be swapped
		vector<State> states;	// for node-state backup
		storeStates(subCkt, states, cell.nodeId);
		// then swap
		if(swapCell(cell.nodeId) == TIMING_VIOLATED) {
			// restore states if timing violated
			netlist[cell.nodeId].voltageType = SVT;	// swap back
			restoreLoadCap(cell.nodeId);
			restoreStates(states);
#ifdef VERBOSE
			numFail++;
#endif
		}
		else {	// update sensitivities after cell swapping
//			calRequiredTimeSSTA(subCkt);	// recalculate RT
//			updateSensitivity(maxHeap, cell.nodeId);
#ifdef VERBOSE
			numSuccess++;
#endif
		}
		// clear states
		states.clear();
	}

#ifdef VERBOSE
	double sRate = (double)numSuccess / (double)numCandidate * 100.0;
	double fRate = (double)numFail / (double)numCandidate * 100.0;
	cout << "\tsuccess/fail/drop rate (%):\t" << sRate << '\t'
		<< fRate << '\t' << 100.0 - sRate - fRate << endl;
#endif
}

void mtv()
{
	// full standard STA
	fullSSTA();

#ifdef VERBOSE
        cout << "Assigning threshold voltages... " << endl << flush;
        timeval time;           // timer
        gettimeofday(&time, NULL);
        double st = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
#endif
	
	// voltage assignment
	for(unsigned int i=0; i<subCkts.size(); i++) {
		SubCkt& subCkt = subCkts[i];
		if(subCkt.pis.size() == 1)	// skip CKBUF's
			if(subCkt.pis[0] == clkNodeId)
				continue;
		greedyVoltageAssignment(subCkt);
	}

#ifdef VERBOSE
        gettimeofday(&time, NULL);
        double et = (double)time.tv_sec + (double)time.tv_usec * 1e-6;
        cout << endl << "done in " << et-st << " sec" << endl << endl;
#endif
}



#if 0
int main (int argc, char *argv[])
{
	// read in cell libraries; argument 1: std lib 2: HVT lib 3: LVT lib
	libParser.parseLib (argv[1]);
	libParser.parseHVTLib (argv[2]);
	libParser.parseLVTLib (argv[3]);
	// read in netlist; argument 4: .cap 5: .netlist 6: .con
	readNetlist(argv[4], argv[5], argv[6], libParser);
	// enjoy watching MTV!
	mtv();
	// dump results
	dumpOptimizedNetlist();
	dumpPowerReport();
	
//	Verilog classVerilog(libParser);
//	classVerilog.voltageTran(argv[7]);

	return 0;
}
#endif

