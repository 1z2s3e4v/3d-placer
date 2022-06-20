#ifndef RANDOMPLACE_H
#define RANDOMPLACE_H
#include "placedb.h"

// change to static

/**
@author Indark
*/
class CRandomPlace{
    public:
	//CPlaceDB*  m_db;
	
	//void place( double );
	static void Place( CPlaceDB& db, double ratio );
	
	//CRandomPlace(CPlaceDB& db);
	//~CRandomPlace();

};

#endif
