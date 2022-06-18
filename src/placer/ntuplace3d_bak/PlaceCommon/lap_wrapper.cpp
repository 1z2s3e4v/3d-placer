#include "lap_wrapper.h"

#include <cassert>
#include <cstdlib>
using namespace std;

CLapWrapper::CLapWrapper() :
    m_dim(0), 
    m_assigncost(NULL),
    m_u(NULL), 
    m_v(NULL),
    m_colsol(NULL), 
    m_rowsol(NULL)
{
}

CLapWrapper::~CLapWrapper()
{
    DeleteArrays();
}

void CLapWrapper::DeleteArrays()
{
    delete [] m_assigncost;
    delete [] m_u;
    delete [] m_v;
    delete [] m_colsol;
    delete [] m_rowsol;
}

void CLapWrapper::SetDimension( const int& dim )
{
    DeleteArrays();

    m_assigncost = new cost* [dim];
    for( int i = 0; i < dim; i++ )
	m_assigncost[i] = new cost[dim];

    m_dim = dim;
    m_rowsol = new col[dim];
    m_colsol = new row[dim];
    m_u = new cost[dim];
    m_v = new cost[dim];
}

void CLapWrapper::AssignCost( const int& i, const int& j, const int& value )
{
    assert( i < m_dim );
    assert( j < m_dim );
    m_assigncost[i][j] = value;
}

int CLapWrapper::Solve()
{
    return lap::lap( m_dim, m_assigncost, m_rowsol, m_colsol, m_u, m_v );
}

int CLapWrapper::GetRowSol( const unsigned int& index )
{
    assert( index < m_dim );
    return m_rowsol[index];
}

int CLapWrapper::GetColSol( const unsigned int& index )
{
    assert( index < m_dim );
    return m_colsol[index];
}

cost CLapWrapper::GetU( const unsigned int& index )
{
    assert( index < m_dim );
    return m_u[index];
}

cost CLapWrapper::GetV( const unsigned int& index )
{
    assert( index < m_dim );
    return m_v[index];
}

