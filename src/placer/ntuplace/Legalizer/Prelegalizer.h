#ifndef _PRELEGALIZER_
#define _PRELEGALIZER_
#include "ParamPlacement.h"
//#include "placedb.h"
class CPlaceDB;
class CPrelegalizer
{
public:
	//CPrelegalizer(void);
	//~CPrelegalizer(void);
	static void Prelegalizer(CPlaceDB& fplan, const SCALE_TYPE& type, const double& scale_factor);
};

#endif
