#ifndef MACROLEGAL_H
#define MACROLEGAL_H
#include "placedb.h"
#include "ParamPlacement.h"
#include <vector>
#include <map>
#include <set>
#include <list>
#include <climits>

/**
	@author Indark <indark@eda.ee.ntu.edu.tw>
*/
class CCGraphEdge{
	public:
	CCGraphEdge(){};
	CCGraphEdge(int from_moduleID, int to_moduleID, double weight, double priority);
	int m_to_moduleID;
	int m_from_moduleID;
	double m_weight;
	double m_priority;
	double m_slack;
	bool	m_isCritical;
	void ShowEdge();
	
};


class CCGraphNode{
	public:
		CCGraphNode();
		CCGraphNode(double value, int moduleID, double low, double high , double length, bool fixed);
		
		static bool Greater(const CCGraphNode& s1, const CCGraphNode& s2);
		static bool Lesser(const CCGraphNode& s1, const CCGraphNode& s2);
		//list<double> m_edgeweight;
		//list <int> m_outedge;
		list <CCGraphEdge> m_outedge;
		//list<int> m_inedge;
		list<CCGraphEdge> m_inedge;
		//vector<int> m_exculsive_moduleID;
		set<int> m_edge_moduleID;
		//set<int> m_inedge_moduleID;
		double m_value;
		double m_length;
		double m_low;
		double m_high;
		double m_Left;
		double m_Right;
		double m_cx;	//for plot only 
		double m_cy;	//for plot only 	
		double m_solution_x;	//for plot only
		double m_solution_y;	
		double m_solution_cx;	//for plot only
		double m_solution_cy;	
		bool m_isFixed;
		int m_moduleID;
		void ShowNode();
		bool m_irresolvable;

};

class CConstraintGraph{

	public:
		static const int SOURCE = -2;
		static const int  SINK = -1;
		void AddNodes(double value, int moduleID, double low, double high,double length, bool fixed);
		static bool Nodes_Greater(const CCGraphNode& s1 ,const CCGraphNode& s2);
		static bool Nodes_Lesser(const CCGraphNode& s1 ,const CCGraphNode& s2);

		void SetBound(double lower,double upper) {m_lower_bound =lower;m_upper_bound= upper;};
		void SetSideBound(double lower,double upper) 
				{m_lower_sidebound =lower;m_upper_sidebound= upper;};
		void SortNodes();
		map<int,int> m_ModuleID2NodeID;
		
		void CGConstruct();
		void nCGConstruct();
		void CGExclusive(CConstraintGraph& CG);
		//void CConstraintGraph::CGAddSourceSink(CRect _rect );
		void CGAddSourceSink(CRect _rect );  // remove "CConstraintGraph" by donnie 2006-12-06
		void OutputGraph( const char* filename, int NodeID);
		void OutputSolutionGraph( const char* filename, bool bWithMove);
		void ReserveMemory();
		void ShowGraph();
		double GetLowerBoound(){return m_lower_bound;}
		double GetUpperBoound(){return m_upper_bound;}
		vector<CCGraphNode> m_nodes;
		double m_pri_thres;
		bool CGAnalyze();
		void AddEdge(CCGraphNode& s,CCGraphNode& t,double priority);
		void RemoveEdge(int moduleID_s, int moduleID_t);
		vector<CCGraphEdge> m_CriticalEdge;
		int CalcTransCost(int moduleID1, int moduleID2);
		int GetMaxTransCost(){
			return (m_MaxTransCost/2);
		};
		double OverLoad;				// Over length
		double GetCenter();
		void Init();
		void ReInit();

			
		void RemoveDummyEdge();	
		int GetANSI();
		

		


//	private:
		 
		double m_lower_bound;
		double m_upper_bound;
		double m_lower_sidebound;			//plot only
		double m_upper_sidebound;
		double m_most_neg_slack;
		
		
		
	private:
		bool NodeIntersect(CCGraphNode& n1,CCGraphNode& n2);
		bool NodeOverlap(CCGraphNode& n1,CCGraphNode& n2);
		double DistanceNodeRow(CCGraphNode& node_row,CCGraphNode& n);
		double Y_Displacement(CCGraphNode& n1,CCGraphNode& n2);
		int m_source;
		int m_sink;
		int m_MaxTransCost;


		
		
		
	
};


class CMacroLegal{

public:
    CMacroLegal(CPlaceDB& db,double thresh_row_high , int _adj_run);

    ~CMacroLegal();
    void Init();
    void ReInit();
    //bool Legalize(bool setFixed);
	bool Legalize(vector<int>& legaled_vector);
    double MacroLP();
    void Apply();
    void ApplyPlaceDB( bool setfixed);
    void FixEdge(CConstraintGraph& _HCG,CConstraintGraph& _VCG);
    void mFixEdge(CConstraintGraph& _HCG,CConstraintGraph& _VCG);
    void bFixEdge(CConstraintGraph& _HCG,CConstraintGraph& _VCG,bool escape);

    void ShowRelation(int moduleID1, int moduleID2);
     void ShowRelation(int moduleID);
     void BoundaryGravity( bool LineGravity);
	void BoundaryGravityRecover();
	int TestDeadSpaceRemove(CConstraintGraph& _HCG ,CConstraintGraph& _VCG);



private:
	CPlaceDB* m_pDB;
	double m_MacroRowHeight;
	int m_AdjRun;
	static const int m_MaxNumMacro = 500;
	static const int m_MaxNumFixedMacro = 1000;
	CConstraintGraph HCG;
	CConstraintGraph VCG;
	vector<int> m_ModuleID;
	set<int> m_ModuleID_set;
	void AutoRowHeight();
	double avg_area;
				
	void ReloadModulePosition();
// 	bool isMacro(int pdb_moduleID){
// 		return 
// 	(m_pDB->m_modules[pdb_moduleID].GetHeight() > m_MacroRowHeight * m_pDB->m_sites[0].m_height )
// 	&& (!m_pDB->m_modules[pdb_moduleID].m_isOutCore );
// 	};
 	bool isMacro(int pdb_moduleID)
	{
	    return
		m_pDB->m_modules[pdb_moduleID].m_height > m_MacroRowHeight * m_pDB->m_rowHeight
		&& ( !m_pDB->BlockOutCore(pdb_moduleID) );   //(kaie) 2009-07-23 
		//(m_pDB->m_modules[pdb_moduleID].m_area > 2000 * avg_area )
		//&& (!m_pDB->BlockOutCore( pdb_moduleID ) );
	    //&& (!m_pDB->m_modules[pdb_moduleID].m_isOutCore );
	};		
	double m_offset_X;
	double m_offset_Y;
	bool isOffset;
	void OffsetRelocate( );
	void OffsetRestore( );
};








#endif
