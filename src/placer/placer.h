#ifndef PLACER_H
#define PLACER_H
#include <tuple>
#include <vector>
#include <map>
#include <unordered_map>
#include <ctime>
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

#include "ntuplace/PlaceDB/placedb.h"
#include "ntuplace/NLPlace/mlnlp.h"
#include "ntuplace/PlaceCommon/ParamPlacement.h"

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
    bool order_place();
    void cell_spreading();
    void cell_spreading(int); // 2d global
    bool shrunked_2d_ntuplace(); // 2d place
    bool shrunked_2d_ntuplace(string); // 2d place
    void rand_place(int); // die_by_die
    void rand_ball_place();
    void init_run_dir();
    void clear();

    /* partition + die-by-die Place */
    bool pseudo3d_ntuplace();
    bool coloquinte_place();
    bool random_d2dplace();
    bool ntu_d2dplace();
    bool pin3d_ntuplace();
    bool shrunk2d_ntuplace();
    void mincut_partition();
    void mincut_k_partition();
    void init_place_ball();
    void run_ntuplace3(string caseName);
    void run_ntuplace3(string caseName, string otherPara);
    void run_ntuplace4(string caseName);
    void run_hmetis(int k, double ufactor, string caseName); // (k-way part)
    bool read_pl_and_set_pos(string fileName);
    bool read_pl_and_set_pos(string fileName, int dieId);
    bool read_pl_and_set_pos_for_ball(string fileName);


    /* connect to NTUplace Datebase */
    void set_ntuplace_param(CPlaceDB&);
    void create_placedb(CPlaceDB&);
    void create_placedb(CPlaceDB&, int dieId);
    void load_from_placedb(CPlaceDB&);
    /* GlobalPlace + Legal + DetailPlace*/
    bool true3d_placement();
    bool half3d_placement();
    bool ntuplace3d(); // ntuplace3d (remember to replace dir 'ntuplace' to 'ntuplace3d_bak')
    void global_place(bool& isLegal, double& totalHPWL);
    void legal_place();
    void detail_place();
    void ntu_d2d_global(bool& isLegal, double& totalHPWL);
    bool ntu_d2d_legal_detail();
    bool pin3d_ntu_d2d_legal_detail();

    void create_aux_form(AUX&, int dieId, string caseName);
    void create_aux_form_for_ball(AUX&, string caseName);
    void add_project_pin(AUX&, int dieID);
    void add_project_ball(AUX&);
    int cal_ball_num();
    int cal_HPWL();
};

#endif