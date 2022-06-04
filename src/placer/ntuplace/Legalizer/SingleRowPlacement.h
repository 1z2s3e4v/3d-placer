#ifndef SINGLE_ROW_PLACEMENT_H
#define SINGLE_ROW_PLACEMENT_H

#include "util.h"

#include <vector>
#include <map>
#include <set>
using namespace std;

class CPlaceDB;

class CSingleRowPlacement
{

    friend class CSingleRowPlacementLegal;
    
    class cellInfo
    {
	public:
	    int x;
	    int preferCenter;	    // fix
	    int preferY;	    // fix
	    int optLeft;
	    int optRight;
	    int cost;
	    int width;	    
	    int originalWidth;	    // fix
	    int preCell;
	    int nextCell;
	    int parentCell;
	    int rowId;
	    vector<int> subCells;	    // record cluster info
	    vector<int> slop;
	    vector<int> slopChangePoint;
	    vector<int> candidateRows;
    };
 
    public:

	static int CreateCell( int x, int y, int width );

    private:

	static void InitCellSlop( int id );
	static vector<cellInfo> m_cells;

    /////////////////////////////////////////////////////////
	
    public:

	CSingleRowPlacement( int rowId, int left, int right, int y );
	~CSingleRowPlacement();

	bool PlaceCell( int cellId );
	void RemoveCell( int id );
	
	int GetCellX( int id );
	int GetCellCost( int id );

	void PrintCell( unsigned int cellId );
	void PrintAllCells();

	int GetCostChange();
	
	void NewBackup();   // need to call after GetCostChange()
	void Restore();
	
    private:

	int  BoundX( int x );
	void CollapseCell( int lastCellId, int cellId );
	void CollapseCellFromCenter( map<int,int>::iterator ite1, map<int,int>::iterator ite2 );
	void UnplaceCluster( int id );
	void ConnectClusters( int from, int to );
	multimap<int,int>::iterator GetCellIterator( int id );
	bool ValidCell( int id );
	
    private:

	int m_rowId;
	int m_leftBoundary;
	int m_rightBoundary;
	int m_totalWidth;
	int m_capacity; // m_rightBoundary - m_leftBoundary
	int m_y;

	multimap< int, int > m_cellOrder;	    // pair< x, cellId >
	
	set<int> m_modifyList;
	vector< pair<int,cellInfo> > m_backupCells;
	bool m_bBackup;	
};


class CSingleRowPlacementLegal
{
    public:

	CSingleRowPlacementLegal( CPlaceDB* );
	~CSingleRowPlacementLegal();
	bool Legalize();
	void PostLegalSingleRowOpt( vector<CPoint>& preferredPositions );
	
    private:

	void ImportRows();
	bool ImportCells( vector<CPoint> *pOriginalPositions = NULL );
	
	bool PlaceCell( int id );
	bool EstPlaceCellCost( int cellId, int rowId, int* cost ); 
	
	void ReplaceCell( int id );
	bool TryPlaceCell( int id, int rowId );
	
	bool PlaceCells();
	void UpdateCellPositions();
	
    private:

	CPlaceDB* m_pDB;	
	vector<int> m_cellIdMap;
	//map<int,int> m_cellIdMap;
	vector< pair<float,int> > m_sequence;
	//multimap<float,int> m_sequence;
	vector<CSingleRowPlacement> m_rows;
};

// NOTE (donnie) 2007-06-24
// The vector<cellInfo> m_cells is a static member in CSingleRowPlacement.
// Thus, CSingleRowPlacement cannot handle many CPlaceDB at the same time.



#endif
