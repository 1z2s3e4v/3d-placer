#ifndef __MAIN__
#define __MAIN__

#include <vector>
#include <string>
#include <queue>
#include <cfloat>
#include "netlist.h"
#include "heap.h"

//#define VERBOSE		// display runtime messages

#define CELL_RISE 0	// cell rise delay
#define CELL_FALL 1	// cell fall delay
#define RISE_TRAN 2	// rise transition time
#define FALL_TRAN 3	// fall transition time
#define RISING 1	// signal 0 -> 1
#define FALLING -1	// singal 1 -> 0
#define NON_UNATE 0	// timing sense: non_unate
#define POS_UNATE 1	// timing sense: positive_unate
#define NEG_UNATE 2	// timing sense: negative_unate
//#define HOLD_RISE 0	// for hold rise check
//#define HOLD_FALL 1	// for hold fall check
#define SETUP_RISE 2	// for setup rise check
#define SETUP_FALL 3	// for setup fall check
#define TIMING_SATISFIED true
#define TIMING_VIOLATED false

//#define _NODFF_

using std::vector;
using std::string;

struct SubCkt {
	vector<unsigned int> nodes;	// node ID's
	vector<unsigned int> pis;	// PI ID's
	vector<unsigned int> srcdffs;	// source DFF Id's
	vector<unsigned int> pos;	// PO ID's
	vector<unsigned int> desdffs;	// destination DFF Id's
};

struct State {
	unsigned int nodeId;
	vector<double> tran;
	double delay;
	double maxCD;
	double clkLatency;
	string latestPin;
	int outputSignal;
};

void calculateClkLatency();
void calculateNextStageTiming(const STA::Node& srcNode, STA::Node& desNode,
	const string& desPin, double srcDelay, double srcTran);
void calATFromPI(const STA::Node& piNode, const vector<FanoutNodeInfo>& nodes,
	queue<unsigned int>& que);
void calATFromDFF(const STA::Node& dffNode, const vector<FanoutNodeInfo>& nodes,
	queue<unsigned int>& que, double dffDaley, double dffTran);
bool checkTiming(STA::Node& desNode, double srcTran);
bool calATFromGivenNodes(queue<unsigned int>& que);
void calArrivalTimeSSTA(const SubCkt& subCkt);
void calRTFromPO(const SubCkt& subCkt, queue<unsigned int>& que);
void calRTFromDFF(const SubCkt& subCkt, queue<unsigned int>& que);
void calRTFromGivenNodes(queue<unsigned int>& que);
void calRequiredTimeSSTA(const SubCkt& subCkt);
void fullSSTA();
inline int getTimingSense(STA::Node& node, const string& inputPin);
void calTempATFromPI(const STA::Node& srcNode, STA::Node& desNode,
	const string& inputPin, unsigned int voltageType,
	double& maxCD, double& desTran, int& outputSignal);
void calTempATFromDFF(STA::Node& srcNode, STA::Node& desNode, const string& inputPin,
	const string& outputPin, unsigned int voltageType,
	double& maxCD, double& desTran, int& outputSignal);
void calTempATFromCell(STA::Node& srcNode, STA::Node& desNode, const string& inputPin,
	unsigned int voltageType, double& maxCD, double& desTran,
	int& outputSignal);
void calTempRTFromPO(const STA::Node& desNode, const STA::Node& nextNode,
	double& requiredTime);
void calTempRTFromDFF(const STA::Node& desNode, STA::Node& nextNode, int outputSignal,
	double desTran, double& requiredTime);
void calTempRTFromCell(STA::Node& nextNode, const string& inputPin, int outputSignal,
	double desTran, double& requiredTime);
double calSensitivity(unsigned int nid, unsigned int voltageType);
void buildSensitivityHeap(const SubCkt& subCkt, Heap& heap);
bool calATFromCell(STA::Node& node, const Fanout& fanout, queue<unsigned int>& que);
bool updateFaninTiming(STA::Node& srcNode, STA::Node& swapNode, const string& inputPin,
	queue<unsigned int>& que);
bool swapCell(unsigned int swapNodeId);
void storeState(vector<State>& states, unsigned int nid);
void storeFanoutConeStates(vector<State>& states, unsigned int srcNodeId);
void reset(const SubCkt& subCkt);
void storeStates(const SubCkt& subCkt, vector<State>& states,
	unsigned int swapNodeId);
void restoreStates(vector<State>& states);
void restoreLoadCap(unsigned int nodeId);
void greedyVoltageAssignment(const SubCkt& subCkt);
void mtv();


#endif
