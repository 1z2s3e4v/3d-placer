#ifndef PLACER_H
#define PLACER_H
#include <tuple>
#include <vector>
#include <map>
#include <unordered_map>
#include <cmath>
#include <climits>
#include <algorithm>
#include <iostream>
#include <numeric>
#include "../utility/color.h"
#include "../utility/aux.h"
#include "../dataModel/module.h"

using namespace std;

class Placer_C{
    Chip_C* _pChip;
    Design_C* _pDesign;
    vector<Pos> _vCellBestPos;
    vector<Cell_C*>& _vCell;
    vector<Net_C*>& _vNet;
    map<string,Cell_C*>& _mCell;
    map<string,Net_C*>& _mNet;
    clock_t _tStart;
public:
    //Place_C();
    Placer_C(Chip_C*, Design_C*, clock_t);
    
    void run();
    void init_place();
    void order_place();
    void clear();

    /* partition + die-by-die NTUplace3 */
    string _RUNDIR = "./run_tmp/";
    void ntu_d2dplace();
    void mincut_partition();
    void output_aux_form(int dieId, string caseName);
    void run_ntuplace3(string caseName);
    void read_pl_and_set_pos(string fileName);
};

#endif