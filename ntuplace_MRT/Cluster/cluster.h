#pragma once

#include "util.h"
#include <vector>

//-------------------------------------------------------------
//  CClustering Usage:
//
//  CClustering fcc;
//  fcc.clustering(CPlaceDB& dblarge, CPlaceDB& dbsmall ,int targetClusterNumber=2000, double areaRatio=1.5, int ctype=1);  //dblarge=original placeDB, dbsmall=clustered placeDB
//  
//  .. do next level cluster...
//  .. place at top level ...
//
//  fcc.declustering(CPlaceDB& dbsmall, CPlaceDB& dblarge ); //decide the module position of dblarge according to coarser level (dbsmall) module location
//
// 
//  ctype:
//  ctype=1 => first choice clustering
//  ctype=2 => first choice clustering with physical clustering
//
//
//-------------------------------------------------------------
class CPlaceDB;
class CClustering
{
public:
	CClustering()
	{
		showMsg=true;
	}
	~CClustering(void){}

	void clustering(const CPlaceDB& dblarge, CPlaceDB& dbsmall ,int targetClusterNumber=2000, double areaRatio=1.5, int ctype=3);
	void declustering( CPlaceDB& dbsmall, CPlaceDB& dblarge, bool isPertab=false );

	bool showMsg;
private:
	vector< vector<int> > m_hierarchy;

};



