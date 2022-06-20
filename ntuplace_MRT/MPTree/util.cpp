#include <vector>
using std::vector;
namespace N_ntumptree{

int SA_TIMES = 50;
int SA_LOCAL = 5;
//int SA_MAX_ITE = 300;
int SA_MAX_ITE = 100;

//double WEIGHT_WASTE = 0.2;
double WEIGHT_BOUNDARY = 0.5;
// double WEIGHT_BOUNDARY = 0.1;
//double WEIGHT_DISPLACEMENT = 0.2;
//double WEIGHT_DISPLACEMENT = 0.0;
double WEIGHT_ORIENTATION = 0.0;
//double WEIGHT_THICKNESS = 0.1;
//double WEIGHT_WIRE = 0.02;
double WEIGHT_WIRE = 0.0;
double WEIGHT_AREA = 0.0;
double WEIGHT_ASPECT = 0.0;


// DAC-07 exp
double WEIGHT_THICKNESS = 0.0;   // old = 0.1
double WEIGHT_DISPLACEMENT = 0.0;// old = 0.2
double WEIGHT_WASTE = 0.5;       // old = 0.2

int LEVEL = 0;

double MAX_ASPECT_RATIO = 8;

double SPACING = 10;

double CUT_Y_LEFT = -1;
double CUT_Y_RIGHT = -1;
double CUT_X_BOTTOM = -1;   // unuse currently
double CUT_X_TOP = -1;	    // unuse currently

vector<int> OPT_SEQ;
}
