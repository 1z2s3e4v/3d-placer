//---------------------------------------------------------------------------
#ifndef sa_btreeH
#define sa_btreeH
//---------------------------------------------------------------------------
#include "fplan.h"
//---------------------------------------------------------------------------
namespace N_ntumptree{
extern double init_avg;
extern double avg_ratio;
extern double lamda;
extern double P;

double SA_Floorplan(FPlan &fp, int k, int local=0, double term_T=0.1, int maxIte=-1);
double Random_Floorplan(FPlan &fp,int times);
}
//---------------------------------------------------------------------------
#endif
