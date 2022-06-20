#ifndef LP_LEGAL_H
#define LP_LEGAL_H

#include "placedb.h"

class CLPLegal
{
    public:

	CLPLegal( CPlaceDB* );
	bool Legal();

	bool TopDownLegal();
	
    private:
    
	CPlaceDB* m_pDB;

	class Bin
	{
	    public:
		double x, y;
		double width, height;
		double area;
		vector<int> movBlockList;
		vector<int> fixBlockList;
	};

	void BinPart( double left, double bottom, double right, double top, 
		double startX, double startY, 
		vector<int> blockList, 
		vector<Bin>& grid,
		vector<int>& moduleBin );
	    
	void GenLPProblem( vector<int>& blockList, vector<Bin>& grid, int binStart );
	void GenLPProblem( 
		vector<int> blockList, 
		//vector<int>& blockList, 
		unsigned int start, unsigned int size, 
		vector<Bin>& grid, int binStart );
	void LoadResults( char* filename, vector<int>& moduleBin, vector<Bin>& grid, int binStart );
	void LoadResults2( char* filename, vector<int>& moduleBin, vector<Bin>& grid, int binStart );

	double GetTotalDisplacement( vector<Bin>& bin, vector<int>& moduleBin );
	
};

#endif
