#ifndef __NETLIST__
#define __NETLIST__

#include <string>
#include <map>
#include <vector>
#include "liberty.h"

using std::string;
using std::map;
using std::vector;

#define NVT 0			// no voltage
#define SVT 1			// standard Vt
#define HVT 2			// high Vt (slow and low-power)
#define LVT 3			// low Vt (fast and leaky)
#define INIT_DFF_VT SVT		// initial DFF Vt
#define INIT_CKBUF_VT SVT	// initial CKBUF Vt
#define INIT_CELL_VT SVT	// initial ordinary cell vt

#define DFF_CLK_PIN "CK"

    struct Fanin {		// a fanin node of a driven pin
	unsigned int nodeId;		// driving node id; only one source
	string pin;			// driving pin name
	double transitionTime;		// input transition time
	//Added by Jin 20081014
	int netId;
    };

    struct FanoutNodeInfo {
	unsigned int nodeId;		// driven node id
	string pin;			// driven pin name
    };

    struct Fanout {		// fanout nodes of a driving pin
	vector<FanoutNodeInfo> nodes;	// could be more than one
	double loadCap;			// capacitance (net + fanout)
	//Added by Jin 20081014
	int netId;
    };

namespace STA
{
    struct Node {		// a node in the netlist
	// basic structure for netlist
	string cellName;		// instance name
	struct cellInfo* cellTypePtr;		// cell type pointer (W.-P.)
	map<string, Fanin> faninMap;	// fanin of driven pins
	map<string, Fanout> fanoutMap;	// fanout of driving pins
	int voltageType;		// SVT, LVT, or HVT
	// for STA
	double delay;			// intrinsic delay of a gate or
	// external delay of a PIO or
	// setup requirement of a DFF
	double maxCD;			// maximum cumulated delay at output pin
	string latestPin;		// input pin having latest arrival time
	double clkLatency;		// clock latency; 0 for PIO
	double requiredTime;		// required time at the output pin
	double slack;			// requiredTime - maxCD
	int outputSignal;		// RISING or FALLING ?
	// for graph search/topological sort
	bool visited;			// visited in BFS?
	unsigned int nVisitedFanin;	// # of visited fanin nodes
	// for sensitivity update
	//	unsigned int heapIndex;		// index of the sensitivity in heap
	
	//Added by Jin 20081014
	double Loadgradient;
	double ATgradient;
	string ATpin;
	int ATnetId;
	double RTgradient;
	string RTnode;
	int RTnetId;
    };
}
    
    struct NetNodeInfo {	// a node of a net
	unsigned int nodeId;
	string pin;
    };

namespace STA
{
    struct Net {		// a net in the netlist
	string name;
	double loadCap;
	vector<NetNodeInfo> nodes;
    };
}

    void readNetlist(char *, char *, char *, Liberty&);
    void dumpOptimizedNetlist(void);
    void dumpPowerReport(void);
    double loadCap(const STA::Node&, const string&, int);
    double loadCap(const STA::Node&, const string&);
    bool isCKBUF(const string& cellType);
    void updateOutputLoad();
    void decomposeSubCkts();

#endif
