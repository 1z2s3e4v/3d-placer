#ifndef MACROSP_H
#define MACROSP_H
#include "placedb.h"
#include "ParamPlacement.h"
#include <vector>
#include <map>
#include <set>
#include <list>
#include <queue>
#include <climits>

/**
	@author Indark <indark@eda.ee.ntu.edu.tw>
*/

class CSPCGraphEdge{
	public:
		CSPCGraphEdge(int from_nodeID, int to_nodeID, double weight);
		int m_to_nodeID;
		int m_from_nodeID;
		double m_weight;
// 		double m_priority;
		double m_slack;
		bool m_isCritical;
		void ShowEdge();
	
};


class CSPCGraphNode{
	public:
		CSPCGraphNode(double value , double length, bool fixed, int m_nodeID);
		static bool Greater(const CSPCGraphNode& s1, const CSPCGraphNode& s2);
		static bool Lesser(const CSPCGraphNode& s1, const CSPCGraphNode& s2);
		list <CSPCGraphEdge> m_outedge;
		list<CSPCGraphEdge> m_inedge;
		set<int> m_edge_nodeID;
		double m_value;
		double m_length;
		double m_rcx;
// 		double m_low;
// 		double m_high;
		double m_Left;
		double m_Right;
		double m_cx;	//for plot only 
// 		double m_cy;	//for plot only 	
		double m_solution_x;	//for plot only 
		double m_solution_y;	
		double m_solution_cx;	//for plot only
		double m_solution_cy;	
		bool m_isFixed;
		int m_nodeID;
// 		int m_moduleID;
		void ShowNode();    
		
		double m_front_spacing;
		double m_back_spacing;


};






class CSPCGraph{
	public:
		CSPCGraph();
		static const int SOURCE = -2;
		static const int  SINK = -1;
		void AddNodes(CSPCGraphNode _node ) 
			{ m_nodes.push_back(_node); };
		static bool Nodes_Greater(const CSPCGraphNode& s1 ,const CSPCGraphNode& s2);
		static bool Nodes_Lesser(const CSPCGraphNode& s1 ,const CSPCGraphNode& s2);

		void SetBound(double lower,double upper)
			{m_lower_bound =lower;m_upper_bound= upper;};
		void SetSource( int _source) { m_source = _source;}
		void SetSink( int _sink) { m_sink = _sink;}
// 		void SetSideBound(double lower,double upper) 
// 			{m_lower_sidebound =lower;m_upper_sidebound= upper;};
		void SortNodes();
// 		map<int,int> m_ModuleID2NodeID; 
		
// 		void CGConstruct();	
		void CGExclusive(CSPCGraph& CG);
		void CGAddSourceSink( );
		void OutputGraph( const char* filename, int NodeID);
		void ReserveMemory();
		void ShowGraph();
		double GetLowerBound(){return m_lower_bound;}
		double GetUpperBound(){return m_upper_bound;}
		vector<CSPCGraphNode> m_nodes;
		double m_pri_thres;
		double CGAnalyze();
		void AddEdge(CSPCGraphNode& s,CSPCGraphNode& t);
		void RemoveEdge(int moduleID_s, int moduleID_t);
		vector<CSPCGraphEdge> m_CriticalEdge;
		int CalcTransCost(int moduleID1, int moduleID2);
		int GetMaxTransCost(){
			return (m_MaxTransCost/2);
		};
		double GetCenter( ){
			return (m_upper_bound + m_lower_bound) /2.0;
		}
		double OverLoad;				// Over length
		void Init(); 
		void ReInit();

			 
		void RemoveDummyEdge();	
		int GetANSI();
		

		


//	private:
		 
		double m_lower_bound;
		double m_upper_bound;
// 		double m_lower_sidebound;			//plot only
// 		double m_upper_sidebound;
		double m_most_neg_slack;
		
		void setNodeSpacing( int nodeID, double front_spacing, double back_spacing);
		
		
		
		
		
		
		
		double m_MaxLeft;
		
		
		
	private:
		bool NodeIntersect(CSPCGraphNode& n1,CSPCGraphNode& n2);
		bool NodeOverlap(CSPCGraphNode& n1,CSPCGraphNode& n2);
		double DistanceNodeRow(CSPCGraphNode& node_row,CSPCGraphNode& n);
		double Y_Displacement(CSPCGraphNode& n1,CSPCGraphNode& n2);
		int m_source;
		int m_sink;
		int m_MaxTransCost;


		
		
	
};


		
		
		
class CMacroSP{
public:
	CMacroSP(CPlaceDB& pDB);
	void Place();
	
	

    ~CMacroSP();
   
    
private:
	CPlaceDB* m_pDB;
	double m_MacroAreaRatio;
	double m_avgArea;
	vector<int> m_MacroList;
	set<int> m_MacroSet;
	int n_movable;
	CSPCGraph HCG;
	CSPCGraph VCG;
	map<int,int> m_NodeID2ModuleID;
	map<int,int> m_ModuleID2NodeID;
	
	bool isMacro( int _moduleID );
	void CalcAvgArea( );
	void NodePreparation();
	void InitSP();
	void ConstractSP( vector< vector<int> >& Graph, vector<int>& SQ );
	int InDeg( vector< vector<int> >& Graph, int index);
	void RemoveNode( vector< vector<int> >& Grpah, int index);
	void CGConstruct( );
	void OutputGraph( const char* filename);
	void OutputMacroGrid( string filename, vector< vector< double> >& _binMacro  );
	
	vector<int> m_SQ_p;
	vector<int> m_SQ_n;
	vector<int> m_SQ_orient;
	
	
	vector<int> m_Best_SQ_p;
	vector<int> m_Best_SQ_n;
	vector<int> m_Best_SQ_orient;

	void Packing();
	void RightPacking();
	void CornerPacking();
	void SolutionPacking();
	void RCXPacking();
	
	void SA();
	void Perturb();
	double CalcCost();
	void Rotate(int _nodeID);
	void SPSwap(int _SQ,int _nodeID1, int _nodeID2);
	void Restore();
	void Accept();
	
	queue<int> m_action;
	
	void SmartBoundaryGravity();
	void BoundaryGravity();
	void BoundaryGravityRecover();
	bool/*double*/ MacroLP(bool ignore); // kaie return false if no solution
	bool/*double*/ MacroLP(){return MacroLP(false);};

	
	double m_BoundSpace;
	void setMacroSpacing();
	
	
	void ApplyBestSP();
	void ApplyPlaceDB();
	void ApplyPlaceDB_CX();
	void ApplyPlaceDB_CX(bool fixed);

	int m_binSize;
	double m_binMacro2Xratio;
	
	vector< vector< double> > m_binMacro;
	vector< vector< double> > m_binStd;
	vector< vector< double> > m_binMacro2X;
	vector< vector< double> > m_binBnd;
	vector< vector< double> > m_binDead;
	
	
	vector< double> m_SpaceArea;
	
	vector< double> m_PinCenter_x;
	vector< double> m_PinCenter_y;
	
	
	double m_StdRgn_x;
	double m_StdRgn_y;
	
	int m_StdRgn_x_idx;
	int m_StdRgn_y_idx;

	
	vector< vector<int> > m_MacroNet;
	vector< vector<int> > m_MacroNetWeight;
	
	
	int GetBinIndexX(double coor,double _binWidth){	return 	(int)floor((double)coor/ (double)_binWidth ); 	};
	int GetBinIndexY(double coor, double _binHeight){	return 	(int)floor((double)coor/ (double)_binHeight ); 	};
	void CalcMacroGrid() { CalcMacroGrid( m_binMacro , m_binSize ); CalcMacroGrid( m_binMacro2X , m_binSize * 2 ); };
	void CalcMacroGrid( vector< vector< double> >& _binMacro ,int _binSize);
	void MoreContract( vector< vector< double> >& _binMacro );
	double GetGrad(int x, int y);
	double GetGrad( int x, int y , vector< vector< double> >& _binMacro , int _binSize );
	
	void SpaceDetect(vector< vector< double> >& _binMacro );
	
	int AddSpace() { m_SpaceArea.push_back(1.0); return m_SpaceArea.size() -1; };
	void IncSpace (int _space_index ) { m_SpaceArea[_space_index] += 1.0; };
	void TermSpace (int _old , int _new ) { m_SpaceArea[_new] += m_SpaceArea[_old]; m_SpaceArea[_old] = 0.0; };
	
	double CalcDeadSpaceCost( );
	double CalcBoundCost();
	
	double CalcDisplacement();
	double CalcDisplacement_CX();
	double CalcDisplacement_SCX();
	double CalcRoundCost();
	double CalcStdCost();
	double CalcWireCost();
	void CalcPinCenter();
	void CalcRCX();
	void CalcStdGrid();
	
	double m_infcost;
	double m_discost;
	double m_rndcost;
	double m_wirecost;
	
	double m_infcost_avg;
	double m_discost_avg;
	double m_rndcost_avg;
	double m_wirecost_avg;
	
	double m_maxHPWL;
	
	double m_cur_reject_rate;


	double m_LP_cost;
	double m_CP_cost;

        double A;
        double B;
        double C;
        double D;



	int m_state;	// + for CP - for LP
	int m_max_state;
	
	void CalcCirIntersec( double cx, double cy, double r, double x1, double y1, double x2, double y2, double& x, double& y);
	
	double Dist( double x1, double y1, double x2, double y2) {return ( sqrt( pow( x1-x2,2) + pow(y1 - y2, 2) )); };

	double GetSmoothedCost ( bool direction );
		
	int ite;
	
	

};










#endif
