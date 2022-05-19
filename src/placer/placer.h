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
#include "../utility/hgr.h"
#include "../dataModel/module.h"
#include "../utility/paramHdl.h"

using namespace std;

class Placer_C{
    ParamHdl_C _paramHdl;
    
    Chip_C* _pChip;
    Design_C* _pDesign;
    vector<Pos> _vCellBestPos;
    vector<Cell_C*>& _vCell;
    vector<Net_C*>& _vNet;
    map<string,Cell_C*>& _mCell;
    map<string,Net_C*>& _mNet;
    clock_t _tStart;
    string _RUNDIR = "./run_tmp/";
public:
    //Place_C();
    Placer_C(Chip_C*, Design_C*, ParamHdl_C&, clock_t);
    
    void run();
    void init_place();
    void order_place();
    void rand_place(int); // die_by_die
    void rand_ball_place();
    void init_run_dir();
    void clear();

    /* partition + die-by-die NTUplace3 */
    void ntu_d2dplace();
    void pin3d_ntuplace();
    void mincut_partition();
    void mincut_k_partition();
    void init_place_ball();
    void run_ntuplace3(string caseName);
    void run_ntuplace4(string caseName);
    void run_hmetis(int k, double ufactor, string caseName); // (k-way part)
    void read_pl_and_set_pos(string fileName, int dieId);
    void read_pl_and_set_pos_for_ball(string fileName);

    void create_aux_form(AUX&, int dieId, string caseName);
    void create_aux_form_for_ball(AUX&, string caseName);
    void add_project_pin(AUX&, int dieID);
    void add_project_ball(AUX&);
    int cal_ball_num();
};

#endif