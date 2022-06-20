#ifndef MACROCLUSTER_H
#define MACROCLUSTER_H
#include "placedb.h"

/**
	@author Indark <indark@eda.ee.ntu.edu.tw>
*/
class CMacroCluster{
public:
    CMacroCluster();
    CMacroCluster(CPlaceDB& pDB);

    ~CMacroCluster();
    
private:
	CPlaceDB* m_pDB;
	bool isMacro(int pdb_moduleID);
	int m_MacroRowHeight;
	int m_MacroAreaRatio;
	void CalcAvgArea( );
	double m_avgArea;
	vector< int > m_MacroList;
	void Cluster (int representive, int clustered);
	set<int> m_ClusteredSet;
	vector< vector<int> > m_Clusters;
	map<int ,int > m_ClusterMap;
	vector<Module> m_ClusterModule;
	void DoClustering();
	
	
	
};

#endif
