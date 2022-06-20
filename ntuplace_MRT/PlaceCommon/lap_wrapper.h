#ifndef LAP_WRAPPER_H
#define LAP_WRAPPER_H

#include "lap.h"

using lap::cost;
using lap::row;
using lap::col;

class CLapWrapper
{

    public:

	CLapWrapper();
	~CLapWrapper();
	void SetDimension( const int& dim );
	void AssignCost( const int& i, const int& j, const int& value );
	int  Solve();
	int  GetRowSol( const unsigned int& index ); 
	int  GetColSol( const unsigned int& index );
        cost GetU( const unsigned int &index );
	cost GetV( const unsigned int &index );	

    private:

	void DeleteArrays();

	int m_dim;
	cost **m_assigncost;
	cost *m_u;
	cost *m_v;
	row *m_colsol;
	col *m_rowsol;

};

#endif
