#include "btree.h"
#include "fplan.h"
//#include "graph.h"
#include "libmptree.h"

#ifndef CORNER_TREE_H
#define CORNER_TREE_H
namespace N_ntumptree{
struct point
{
    double x;
    double y;
};

class CornerTree
{

    public:
	CornerTree();
	~CornerTree();
	void Init();
	void LoadModules( const char* filename );	/// Load modules and initial positions from a file
	void LoadSettings( const char* filename );
	void AddModules(  vector<mptree_module>& mpmods, mptree_info& info);	/// Load modules
	void AddSettings( );
	void Optimize( int times=50, int local=7 );	/// Use SA to optimize the floorplan
	void OutputGNUplot( const char* filename, int level=-1, bool plotNets=false );	/// Output gnuplot file
	void ListModules();		
	void OutputTemp( const char* filename );	/// Output temp file format	
	void OutputDump( const char* filename );	/// 2005-10-07 output astro dump file
	void ParseModuleHierarchy();			/// 2005-11-21 (donnie) 
	
	void GetModuleXY( int m, double& x, double& y ); /// 2006-02-10 (indark)
	void GetModuleRotate( int m, bool& rotate ); /// 2006-04-15 (indark)
    private:
	B_Tree* m_btrees;	/// Save corner B*-trees
				/// [0] left-bottom
				/// [1] left-top
				/// [2] right-top
				/// [3] right-bottom
	double m_chipWidth;
	double m_chipHeight;
	double m_coreLeft, m_coreRight;
	double m_coreTop, m_coreBottom;
	point m_cornerLB, m_cornerLT, m_cornerRB, m_cornerRT;
	Modules	     m_modules;		/// Store modules positions/widths/heights
	Modules_Info m_modulesInfo;	/// Store modules positions/orientations
	Nets	     m_network;		/// vector< vector<Pin*> >
	vector<int> m_modulesRegion;	/// Store modules regionId
	vector<int> m_modulesNodeId;	/// Store modules node id in the B*-tree
	vector<int> m_matching;		/// Modules with the same values are clustered together
	int m_clusterCount;
	
	// 2005-09-05
	double m_stdCoreLeft, m_stdCoreRight;
	double m_stdCoreTop, m_stdCoreBottom;
	double m_rowHeight;

	// 2005-09-06
	void GetPinXY( Pin* p, double& x, double& y );
	//void GetModuleXY( int m, double& x, double& y );
	int m_moduleNumber;

	// 2005-09-07
	int GetModuleIdByName( const char* name );
	void SetBTreeConstraints();
	vector<node_relation> m_moduleRelations;

        vector<int> clusterID; // added by unisun, 2005-09-21

	// 2005-10-06
	void CreateBlockages( int region );
	int m_nBlockages;
	
	public:
	// 2006-02-06 - indark
	double m_offset_X;
	double m_offset_Y;
	void OffsetRelocate();
	void OffsetRestore();
	
    private:
	void CreateBTrees();		/// Create the internal B*-trees
	void UpdateModulePositions();	/// Load module positions from m_btrees
	void CreateCoreRegion();	/// Create core region according to the corner fixed blocks and pad sizes

    public:
	void ClusterModules();		/// Find modules with equal width/height, create clustered modules
	void DeclusterModules();	/// Decluster modules
	// added by unisun, 2005-07-25
	void AddSpacing();              // add extra space (defined in fplan.h)
	void RestoreSpacing();          // restore the original coordinate
	
	
	// added by unisun, 2005-09-23
	vector<string> orientation;
	void Compaction();
	double DimensionalCompaction(Modules&, int, string, vector<int>&);				
	void ModuleFlip();
	vector<int> regions;

    private: // 2005-11-22
	vector< vector< vector<int> > > m_groups;	// [level][group_id][]
	vector< vector<int> > m_hNets;		   
        vector< double >      m_hNetsWeight;	
	void OutputGNUplotModule( ofstream& outPlt, int i );	
	void CreateHierarchyNets( const vector<int>& regionId, const vector<int>& nodeId );
	
};
}
#endif

