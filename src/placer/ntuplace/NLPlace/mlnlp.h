#include <string>
#include "../PlaceDB/placedb.h"
//#include "btree.h"
using namespace std;


// multilevel global placement using nonlinear programming
// return true is placement is legal
bool multilevel_nlp( CPlaceDB& placedb,
       int ctype,
       double weightLevelDecreaingRate=2
);


void globalLocalSpreading( CPlaceDB* pDB, double targetDensity );
void globalRefinement( CPlaceDB* pDB, double targetDensity );


