#pragma once
#include "../PlaceDB/placedb.h"
#include "../PlaceCommon/util.h"

//-------------------------------------------------------------
//  CFCClustering Usage:
//
//  CFCClustering fcc;
//  fcc.clustering(CPlaceDB& dblarge, CPlaceDB& dbsmall ,int targetClusterNumber=2000, double areaRatio=1.5);  //dblarge=original placeDB, dbsmall=clustered placeDB
//  
//  .. do next level cluster...
//  .. place at top level ...
//
//  fcc.declustering(CPlaceDB& dbsmall, CPlaceDB& dblarge ); //decide the module position of dblarge according to coarser level (dbsmall) module location
//-------------------------------------------------------------

class CClusterDB_Module
{
public:
	CClusterDB_Module()
	{
	    m_isNoLegalNeighbor=false;
	}
	~CClusterDB_Module(){}

	int m_mID;
	bool m_isExist;

    double m_cx;
	double m_cy;
	double m_area;
	vector<int> m_hierarchy;
	set<int> m_netIDs;
	bool m_isNoLegalNeighbor;
	//map<int,set<int> > m_neighborMaps;
};

class CClusterDB
{
public:
	CClusterDB()
	{
	    m_movableCount=0;
	}
	~CClusterDB(){}
	void placeDbIn(CPlaceDB& db);
	void mergeModule(int mID1, int mID2);

	void placeDbOut();

	void addBypassNet(int bypassMacroRowHeight);

	void showNetProfile();
	double findMaxClusterArea(const int& targetClusterNumber,const double& areaRatio); 

	bool isModuleFix( int mID);
	int getMovableCount();

	CPlaceDB * m_pDB;
	vector< set<int> >          m_nets; //put moduleID into set
	vector< CClusterDB_Module > m_modules;
//	vector< map<int,set<int> > >m_neighborMaps;
	//vector< map<int,double> > m_affinityMap;

	vector<bool> m_isNetExist;
	vector<bool> m_isPinExist;
	vector<bool> m_isModuleExist;
	vector< vector<int> > m_hierarchy;
//	void showDBdata();

	int m_movableCount;
};



class CClusterDBFC
{
public:
	CClusterDBFC(int bypassHeight=0)
	{
		bypassMacroRowHeight=bypassHeight;
	}
	~CClusterDBFC(){}

    //with acceleration heuristic
	void clustering(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber=2000, double areaRatio=1.5);

    //no heuristic
	void clusteringNH(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber=2000, double areaRatio=1.5);

    //bypass large net (for acceleration)
	void clusteringBLN(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber=2000, double areaRatio=1.5);

    //seed groth clustering (ISPD06)...bad result
    void clusteringSG(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber=2000, double areaRatio=1.5);

    //best choice
    void clusteringBC(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber=2000, double areaRatio=1.5);
	//	void runClustering(int targetClusterNumber=2000, double areaRatio=1.5);


private:
	int bypassMacroRowHeight;

	void addBypassNet();
	int chooseMaxAffinity(int mID, double maxClusterArea);
	int chooseMaxAffinity2(const int mID, const double maxClusterArea, const int maxNetSize); //for accelectrion
    //int chooseMaxAffinity(int mID, double maxClusterArea, double& aff);
	//int findBestAffinity(double maxClusterArea, int& target);

    //no use
	void logicContraction(int ClusterID, int mID, const vector< set<int> >& oldNet, double& EF, double& IF);
	int chooseMaxContraction(int mID, double maxClusterArea,const vector< set<int> >& oldNet); 
    //~no use

	int chooseMaxConnectivity(const set<int>& cluster, const set<int>& neighbor,const double& cSize,  map<int,int>& cNetSize,const double& maxClusterArea);
	void creatClusterFromSeed(int mID, double maxClusterArea);


	//===== for BC
	void setNeighborFalse(int mID, vector<bool>& falseFlag);
	int chooseMaxAffinity(int mID, double maxClusterArea, double& maxAff);
	//===== ~for BC

    CClusterDB  m_ClusterDB;
    CPlaceDB * m_pDB;

};


class CClusterDBBC
{
public:
	CClusterDBBC()
	{
	}
	~CClusterDBBC(){}
	void clustering(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber=2000, double areaRatio=1.5);
	//	void runClustering(int targetClusterNumber=2000, double areaRatio=1.5);
	void clusteringWithAcc(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber=2000, double areaRatio=1.5);


private:
	double m_maxClusterArea;
	void init();
	void updateAllAffinity(int mID, bool changeFlag);
	//void mergeModule(int mID1,int mID2);
	//void updateMaxAffinity(int mID);

	vector<double> m_moduleBestAffinity;
	vector<int> m_moduleBestAffinityID;
	vector<bool> m_updateFlag;

    CClusterDB  m_ClusterDB;
    CPlaceDB * m_pDB;

};


class CFCClustering
{
public:
	CFCClustering(CPlaceDB& db)
	{
		m_pDB=&db;
//		m_moduleMaxAffinityID.resize(m_pDB->m_modules.size(),-1);
//		m_moduleAffinityMaps.resize(m_pDB->m_modules.size());
		m_isNetExist.resize(m_pDB->m_nets.size(),true);
		m_isPinExist.resize(m_pDB->m_pins.size(),true);
		m_isModuleExist.resize(m_pDB->m_modules.size(),true);
		m_hierarchy.resize(m_pDB->m_modules.size());
		for(int i=0; i<(int)m_hierarchy.size(); i++)
		{
			m_hierarchy[i].push_back(i);

		}
		
	}
	CFCClustering()
	{		
	}
	~CFCClustering(void){}

	void clustering(int targetClusterNumber, double maxClusterArea,bool physical=false);
//	void clustering(CPlaceDB& dblarge, CPlaceDB& dbsmall ,int targetClusterNumber=2000, double areaRatio=1.5);
	void clustering(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber=2000, double areaRatio=1.5);
	void physicalclustering(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber=2000, double areaRatio=1.5);

	void declustering(CPlaceDB& dbsmall, CPlaceDB& dblarge,bool isPertab=true );
	vector< vector<int> > m_hierarchy;

private:
    CPlaceDB * m_pDB;
//	vector<int> m_moduleMaxAffinityID;
//	vector< map<int,double> > m_moduleAffinityMaps;
	vector<bool> m_isNetExist;
	vector<bool> m_isPinExist;
	vector<bool> m_isModuleExist;
//	vector< set<int> > m_netSets;


	void getModuleNeighbors(int mID, set<int>& neighborSet);

//	void setAffinityMap(int mID);

	int getNetSize(int netID);

	int countMoveModules();

	//merge 2 module
	void mergeModule(int mID1, int mID2);

	int chooseMaxAffinity(int mID, double maxClusterArea,bool physical=false);

//	void buildAllAffinity();
//	void updateAffinity(int mID);

	//the last stage  of CFCClustering::clustering()
	void removeExcessModulePin();
	bool showMsg;

};

class CFCClusteringVertexCompare
{
public:

	CFCClusteringVertexCompare(CPlaceDB& v)
	{ 
		m_pDB=&v;
	}

	CPlaceDB * m_pDB;
	bool operator()(const int& x, const int& y) 
	{
	    if(m_pDB->m_modules[x].m_netsId.size()!=m_pDB->m_modules[y].m_netsId.size())
	    {
		return m_pDB->m_modules[x].m_netsId.size()>m_pDB->m_modules[y].m_netsId.size();
	    }
	    else
	    {
		return m_pDB->m_modules[x].m_area<m_pDB->m_modules[y].m_area;
	    }
	}

};
class CFCDBClusteringVertexCompare
{
public:

	CFCDBClusteringVertexCompare(CClusterDB& v)
	{ 
	    m_pDB=&v;
	}

	CClusterDB * m_pDB;
	bool operator()(const int& x, const int& y) 
	{
	    if(m_pDB->m_modules[x].m_netIDs.size()!=m_pDB->m_modules[y].m_netIDs.size())
	    {
		return m_pDB->m_modules[x].m_netIDs.size()>m_pDB->m_modules[y].m_netIDs.size();
	    }
	    else
	    {
		return m_pDB->m_modules[x].m_area<m_pDB->m_modules[y].m_area;
	    }

	}

};

