
#ifndef UTIL_H
#define UTIL_H
#include <vector>
namespace N_ntumptree{
extern int SA_TIMES;
extern int SA_LOCAL;
extern int SA_MAX_ITE;

extern double WEIGHT_WASTE;
extern double WEIGHT_BOUNDARY;
extern double WEIGHT_DISPLACEMENT;
extern double WEIGHT_ORIENTATION;
extern double WEIGHT_THICKNESS;
extern double WEIGHT_WIRE;
extern double WEIGHT_AREA;
extern double WEIGHT_ASPECT;

extern int LEVEL;

extern double MAX_ASPECT_RATIO;	    

extern double SPACING;	// macro-padding

extern double CUT_Y_LEFT;
extern double CUT_Y_RIGHT;
extern double CUT_X_BOTTOM;   // unuse currently
extern double CUT_X_TOP;      // unuse currently

extern vector<int> OPT_SEQ;
}
#endif
