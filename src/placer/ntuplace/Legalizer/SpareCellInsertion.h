#include "placedb.h"

class SpareCellInsertion
{
	public:
		SpareCellInsertion( CPlaceDB& placedb ) : m_placedb( placedb ){}
		CPlaceDB& m_placedb;
};
