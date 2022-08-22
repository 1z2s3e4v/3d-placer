#include "placer.h"
//#include <bits/types/clock_t.h>
#include <ctime>
#include <cmath>
#include <algorithm>   
#include <vector> 
#include <random>
#include <iostream>
#include <set>
#include <map>

using namespace std;

Placer_C::Placer_C(Chip_C* p_pChip, Design_C* p_pDesign, ParamHdl_C& paramHdl, clock_t p_start)
:_vCell(p_pDesign->get_cells()), _vNet(p_pDesign->get_nets()), _mCell(p_pDesign->get_cells_map()), _mNet(p_pDesign->get_nets_map())
{
    _pDesign = p_pDesign;
    _pChip = p_pChip;
    _paramHdl = paramHdl;
    _tStart = p_start;
    _vCell = p_pDesign->get_cells();
    _vNet = p_pDesign->get_nets();
    _vCellBestPos.resize(_vCell.size(),Pos(0,0,0));

    _RUNDIR = "./run_tmp/" + _paramHdl.get_case_name() + "/";
    _DRAWDIR = "./draw/"+ _paramHdl.get_case_name() + "/";
}

void Placer_C::run_safe_mode(){
    cout << BLUE << "[Placer]" << RESET << " - Start Safe-Mode\n";
    init_run_dir();
    init_draw_dir();
    bool place_succ = false;
    while(!place_succ){
            place_succ = order_place();

        if(!place_succ){
            cout << BLUE << "[Placer]" << RESET << " - ###############################\n";
            cout << BLUE << "[Placer]" << RESET << " - Run Again ...\n";
            continue;
        }
    }

    // update the HPWL
    cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - Finish!\n";
}

void Placer_C::run(){
    cout << BLUE << "[Placer]" << RESET << " - Start\n";
    init_run_dir();
    init_draw_dir();
    bool place_succ = false;
    while(!place_succ){
        if(!_paramHdl.check_flag_exist("flow")){
            // init place
            //init_place();
            //place_succ = order_place();
            //place_succ = random_d2dplace();
            //place_succ = ntu_d2dplace();
            //place_succ = pin3d_ntuplace();
            if(_vCell.size() < 100)
                place_succ = shrunk2d_ntuplace();
            else
                place_succ = shrunk2d_replace();
            //place_succ = true3d_placement(); // our main function
            //place_succ = half3d_placement(); // our main function
            //place_succ = ntuplace3d();// (remember to replace dir 'ntuplace' to 'ntuplace3d_bak')
            //place_succ = coloquinte_place();
            //place_succ = shrunked_2d_ntuplace();
        }
        else{ // assigned flow
            if(_paramHdl.get_para("flow") == "order_place"){
                place_succ = order_place();
            } else if(_paramHdl.get_para("flow") == "random_d2d"){
                place_succ = random_d2dplace();
            } else if(_paramHdl.get_para("flow") == "ntu_d2d"){
                place_succ = ntu_d2dplace();
            } else if(_paramHdl.get_para("flow") == "pin3d"){
                place_succ = pin3d_ntuplace();
            } else if(_paramHdl.get_para("flow") == "shrunk2d"){
                place_succ = shrunk2d_ntuplace();
            } else if(_paramHdl.get_para("flow") == "shrunk2d_replace"){
                place_succ = shrunk2d_replace();
            } else if(_paramHdl.get_para("flow") == "true3d"){
                place_succ = true3d_placement();
            } else if(_paramHdl.get_para("flow") == "true3d2"){
                place_succ = true3d_placement2();
            } else if(_paramHdl.get_para("flow") == "ntuplace3d"){
                place_succ = ntuplace3d();
            } else{
                cout << BLUE << "[Placer]" << RESET << " - Please assign a valid flow name.\n";
                exit(1);
            }
        }

        if(!place_succ){
            cout << BLUE << "[Placer]" << RESET << " - ###############################\n";
            cout << BLUE << "[Placer]" << RESET << " - Run Again ...\n";
            continue;
        }
    }

    // visualization (output svg.html and .plt)
    if(!_paramHdl.check_flag_exist("no_draw") || _paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-result");
        draw_layout_result_plt(false, "-result");
    }

    // update the HPWL
    cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - Finish!\n";
}

void Placer_C::init_place(){
    for(Cell_C* cell : _vCell){
        cell->set_xy(Pos(0,0));
        cell->set_die(_pChip->get_die(0));
    }
    for(Net_C* net : _vNet){
        net->set_ball_xy(Pos(0,0));
    }
}

bool Placer_C::shrunked_2d_ntuplace(){
    return shrunked_2d_ntuplace("");
}
bool Placer_C::shrunked_2d_ntuplace(string para){
    for(Cell_C* cell : _vCell)
        cell->set_die(_pChip->get_die(0));
    rand_place(0);
    // run ntuplace with half cell height
    AUX aux2D;
    string caseName = "shrunked-2d-place";
    // >>> create_aux_form();
    bool placer_succ;
    {
        string aux_dir = _RUNDIR + caseName + "/";
        string cmd = "mkdir -p " + aux_dir;
        system(cmd.c_str());
        aux2D = AUX(aux_dir, caseName);
        // for shrunk 2d
        int rowH = ceil((_pChip->get_die(0)->get_row_height() + _pChip->get_die(1)->get_row_height())/4.0);
        // nodes
        vector<Cell_C*>& v_cells = _vCell;
        for(Cell_C* cell : v_cells){
            int cellW = ceil((cell->get_width(0)+cell->get_width(1))/2.0);
            aux2D.add_node(cell->get_name(), cellW, rowH, cell->get_posX(), cell->get_posY(),0);
            for(int i=0;i<cell->get_pin_num();++i){
                Pin_C* pin = cell->get_pin(i);
                Net_C* net = pin->get_net();
                if(net != nullptr){
                    char IO = 'I';
                    if(!aux2D.check_net_exist(net->get_name())){
                        aux2D.add_net(net->get_name());
                        IO='O';
                    }
                    Pos pin_offset0 = cell->get_master_cell()->get_pin_offset(_pChip->get_die(0)->get_techId() ,pin->get_id());
                    //pin_offset0.x -= cell->get_width()/2; pin_offset0.y -= cell->get_height()/2;
                    Pos pin_offset1 = cell->get_master_cell()->get_pin_offset(_pChip->get_die(1)->get_techId() ,pin->get_id());
                    //pin_offset1.x -= cell->get_width()/2; pin_offset1.y -= cell->get_height()/2;
                    Pos pin_offset = Pos(ceil((pin_offset0.x+pin_offset1.x)/2.0),ceil((pin_offset0.y+pin_offset1.y)/4.0));
                    aux2D.add_pin(net->get_name(), pin->get_cell()->get_name(), IO, pin_offset.x, pin_offset.y);
                }
            }
        }
        // rows
        aux2D.set_default_rows(_pChip->get_width(), rowH, _pChip->get_height()/rowH);
    }// <<< create_aux_form();

    aux2D.write_files();
    run_ntuplace3(caseName, para);
    placer_succ = read_pl_and_set_pos(_RUNDIR+caseName+".ntup.pl");
    if(!placer_succ) return false;
    return true;
}

bool Placer_C::shrunked_2d_replace(){
    for(Cell_C* cell : _vCell){
        cell->set_die(_pChip->get_die(0));
        cell->set_xy(Pos(_pChip->get_width()/2, _pChip->get_height()/2));
    }
    //rand_place(0);
    // run ntuplace with half cell height
    AUX aux2D;
    string caseName = "shrunked-2d-replace";
    // >>> create_aux_form();
    bool placer_succ;
    {
        string aux_dir = _RUNDIR + "/IBM/" + caseName;
        string cmd = "mkdir -p " + aux_dir;
        system(cmd.c_str());
        aux2D = AUX(aux_dir, caseName);
        // for shrunk 2d
        int rowH = ceil((_pChip->get_die(0)->get_row_height() + _pChip->get_die(1)->get_row_height())/4.0);
        // nodes
        vector<Cell_C*>& v_cells = _vCell;
        for(Cell_C* cell : v_cells){
            int cellW = ceil((cell->get_width(0)+cell->get_width(1))/2.0);
            aux2D.add_node(cell->get_name(), cellW, rowH, cell->get_posX(), cell->get_posY(),0);
            for(int i=0;i<cell->get_pin_num();++i){
                Pin_C* pin = cell->get_pin(i);
                Net_C* net = pin->get_net();
                if(net != nullptr){
                    char IO = 'I';
                    if(!aux2D.check_net_exist(net->get_name())){
                        aux2D.add_net(net->get_name());
                        IO='O';
                    }
                    Pos pin_offset0 = cell->get_master_cell()->get_pin_offset(_pChip->get_die(0)->get_techId() ,pin->get_id());
                    //pin_offset0.x -= cell->get_width()/2; pin_offset0.y -= cell->get_height()/2;
                    Pos pin_offset1 = cell->get_master_cell()->get_pin_offset(_pChip->get_die(1)->get_techId() ,pin->get_id());
                    //pin_offset1.x -= cell->get_width()/2; pin_offset1.y -= cell->get_height()/2;
                    Pos pin_offset = Pos(ceil((pin_offset0.x+pin_offset1.x)/2.0),ceil((pin_offset0.y+pin_offset1.y)/4.0));
                    aux2D.add_pin(net->get_name(), pin->get_cell()->get_name(), IO, pin_offset.x, pin_offset.y);
                }
            }
        }
        // rows
        aux2D.set_default_rows(_pChip->get_width(), rowH, _pChip->get_height()/rowH);
    }// <<< create_aux_form();

    aux2D.write_files();
    run_replace(caseName);
    placer_succ = read_pl_and_set_pos(_RUNDIR + "/outputs/IBM/" + caseName + "/experiment0/tiers/0/" + caseName + ".pl");
    if(!placer_succ) return false;
    return true;
}

bool Placer_C::ntuplace3d(){
    cout << BLUE << "[Placer]" << RESET << " - Start NTUplace3d.\n";
    double part_time_start=0, total_part_time=0;
    ////////////////////////////////////////////////////////////////
    // Global Placement
    ////////////////////////////////////////////////////////////////
    bool isLegal = false; 
    double wl1 = 0; // gp-wire
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 1]" << RESET << ": Global Placement.\n";
    // min-cut die-partition
    mincut_partition();
    cout << BLUE << "[Placer]" << RESET << " - Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << "\n";
    cout << BLUE << "[Placer]" << RESET << " - #Terminal = " << cal_ball_num() << "\n";
    // Init spreading and layer assignment
    rand_place(0);
    rand_place(1);
    rand_ball_place();
    // ntuplace 3d analytical global place
    global_place(isLegal, wl1); /////////////////////////////////////////////// main function
    rand_ball_place();
    //ntu_d2d_global(isLegal, wl1);
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Global: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    cout << BLUE << "[Placer]" << RESET << " - Global: total pin2pin HPWL = " << (int)wl1 << ".\n";

    ////////////////////////////////////////////////////////////////
    // Legalization
    ////////////////////////////////////////////////////////////////
    //legal_place();
    
    ////////////////////////////////////////////////////////////////
    // Detail Placement
    ////////////////////////////////////////////////////////////////
    //detail_place();
    bool place_succ;
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 2]" << RESET << ": Legalization + Detail Placement.\n";
    place_succ = ntu_d2d_legal_detail(); ////////////////////////////////////////////////////// main function
    if(!place_succ) return false;
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    int total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    return true;
}

bool Placer_C::true3d_placement2(){
    cout << BLUE << "[Placer]" << RESET << " - Start True3d Placement Flow.\n";
    double part_time_start=0, total_part_time=0;
    int total_hpwl = 0;
    ////////////////////////////////////////////////////////////////
    // Initail Placement
    ////////////////////////////////////////////////////////////////
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 1]" << RESET << ": Initial Placement.\n";
    // cell spreading
    //cell_spreading();
    if(_vCell.size() < 100)
        cell_spreading();
    else{
        shrunked_2d_replace();
        // for(Cell_C* cell : _vCell){
        //     cell->set_die(_pChip->get_die(0));
        //     cell->set_xy(Pos(_pChip->get_width()/2, _pChip->get_height()/2));
        // }
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [1.1] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-1.1-2Dplace");
        draw_layout_result_plt(false, "-1.1-2Dplace");
    }
    // die-partition
    if(_pChip->get_die(0)->get_row_num()==_pChip->get_die(1)->get_row_num())
        mincut_partition();
    else 
        mincut_k_partition();
    init_ball_place();
    cout << BLUE << "[Placer]" << RESET << " - Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << "\n";
    cout << BLUE << "[Placer]" << RESET << " - #Terminal = " << cal_ball_num() << "\n";
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [1.2] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-1.2-tier-partition");
        draw_layout_result_plt(false, "-1.2-tier-partition");
    }
    
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Init: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";

    ////////////////////////////////////////////////////////////////
    // Global Placement
    ////////////////////////////////////////////////////////////////
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 2]" << RESET << ": Global Placement.\n";
    bool isLegal = false; 
    double wl1 = 0; // gp-wire
    // 3d analytical global placement
    global_place(isLegal, wl1); /////////////////////////////////////////////// main function
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Global: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    //cout << BLUE << "[Placer]" << RESET << " - Global: total pin2pin HPWL = " << (int)wl1 << ".\n";
    cout << BLUE << "[Placer]" << RESET << " - Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << "\n";
    cout << BLUE << "[Placer]" << RESET << " - #Terminal = " << cal_ball_num() << "\n";
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [2] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-2-global");
        draw_layout_result_plt(false, "-2-global");
    }

    // check via usage
    if(cal_ball_num() > _pChip->get_max_ball_num()){
        cout << BLUE << "[Placer]" << RESET << " - " << RED << "Terminal num too much." << RESET << " Run again with Pseudo3d Flow..." << "\n";
        if(_vCell.size() < 100)
            return shrunk2d_ntuplace();
        else
            return shrunk2d_replace(); 
    }

    ////////////////////////////////////////////////////////////////
    // D2D Placement with Pin Projection
    ////////////////////////////////////////////////////////////////
    //pin3d_ntu_d2d_legal_detail()
    bool placer_succ = true;
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 3]" << RESET << ": D2D LG+DP with Pin Projection.\n";
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    // 1. Place balls
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        //create_aux_form_for_ball_replace(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball", "-nolegal -nodetail");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        // run_replace("ball");
        // placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR + "/outputs/IBM/" + "ball" + "/experiment0/tiers/0/" + "ball.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.1] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.1-ball-global");
        draw_layout_result_plt(false, "-3.1-ball-global");
    }
    // 2. Place die0 with projected die1
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        //create_aux_form(aux, 0, "die0");
        create_aux_form_replace(aux, 0, "die0");
        add_project_ball(aux);
        //add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        // run_ntuplace3("die0");
        // placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        run_replace("die0");
        placer_succ = read_pl_and_set_pos(_RUNDIR + "/outputs/IBM/" + "die0" + "/experiment0/tiers/0/" + "die0.pl", 0);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        //add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.2] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.2-die0-global");
        draw_layout_result_plt(false, "-3.2-die0-global");
    }
    // 3. Place die1 with projected die0
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux;
        //create_aux_form(aux, 1, "die1");
        create_aux_form_replace(aux, 1, "die1");
        //add_project_pin(aux, 0);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        // run_ntuplace3("die1");
        // placer_succ = read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 1);
        run_replace("die1");
        placer_succ = read_pl_and_set_pos(_RUNDIR + "/outputs/IBM/" + "die1" + "/experiment0/tiers/0/" + "die1.pl", 1);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.3] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.3-die1-global");
        draw_layout_result_plt(false, "-3.3-die1-global");
    }
    // 4. Replace die0 again with projected die1 and balls
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 0, "die0");
        //add_project_pin(aux, 1);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die0");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){ // replace balls
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.4] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.4-die0-legal");
        draw_layout_result_plt(false, "-3.4-die0-legal");
    }
    // 5. Replace die1 again with projected die0 and balls
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 1, "die1");
        //add_project_pin(aux, 0);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die1");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 1);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){ // replace balls
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.5] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.5-die1-legal");
        draw_layout_result_plt(false, "-3.5-die1-legal");
    }
    // 6. Replace die0 again with projected die1 and balls
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 0, "die0");
        //add_project_pin(aux, 1);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die0");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){ // replace balls
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - D2D-PL: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.6-die0-re");
        draw_layout_result_plt(false, "-3.6-die0-re");
    }
    return true;
}

bool Placer_C::true3d_placement(){
    cout << BLUE << "[Placer]" << RESET << " - Start True3d Placement Flow.\n";
    double part_time_start=0, total_part_time=0;
    int total_hpwl = 0;
    ////////////////////////////////////////////////////////////////
    // Initail Placement
    ////////////////////////////////////////////////////////////////
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 1]" << RESET << ": Initial Placement.\n";
    // cell spreading
    //cell_spreading();
    if(_vCell.size() < 100)
        cell_spreading();
    else{
        shrunked_2d_replace();
        // for(Cell_C* cell : _vCell){
        //     cell->set_die(_pChip->get_die(0));
        //     cell->set_xy(Pos(_pChip->get_width()/2, _pChip->get_height()/2));
        // }
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [1.1] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-1.1-2Dplace");
        draw_layout_result_plt(false, "-1.1-2Dplace");
    }
    // die-partition
    if(_pChip->get_die(0)->get_row_num()==_pChip->get_die(1)->get_row_num())
        mincut_partition();
    else 
        mincut_k_partition();
    init_ball_place();
    cout << BLUE << "[Placer]" << RESET << " - Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << "\n";
    cout << BLUE << "[Placer]" << RESET << " - #Terminal = " << cal_ball_num() << "\n";
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [1.2] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-1.2-tier-partition");
        draw_layout_result_plt(false, "-1.2-tier-partition");
    }
    
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Init: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";

    ////////////////////////////////////////////////////////////////
    // Global Placement
    ////////////////////////////////////////////////////////////////
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 2]" << RESET << ": Global Placement.\n";
    bool isLegal = false; 
    double wl1 = 0; // gp-wire
    // 3d analytical global placement
    global_place(isLegal, wl1); /////////////////////////////////////////////// main function
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Global: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    //cout << BLUE << "[Placer]" << RESET << " - Global: total pin2pin HPWL = " << (int)wl1 << ".\n";
    cout << BLUE << "[Placer]" << RESET << " - Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << "\n";
    cout << BLUE << "[Placer]" << RESET << " - #Terminal = " << cal_ball_num() << "\n";
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [2] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-2-global");
        draw_layout_result_plt(false, "-2-global");
    }

    // check via usage
    if(cal_ball_num() > _pChip->get_max_ball_num()){
        cout << BLUE << "[Placer]" << RESET << " - " << RED << "Terminal num too much." << RESET << " Run again with Pseudo3d Flow..." << "\n";
        if(_vCell.size() < 100)
            return shrunk2d_ntuplace();
        else
            return shrunk2d_replace(); 
    }

    ////////////////////////////////////////////////////////////////
    // D2D Placement with Pin Projection
    ////////////////////////////////////////////////////////////////
    //pin3d_ntu_d2d_legal_detail()
    bool placer_succ = true;
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 3]" << RESET << ": D2D LG+DP with Pin Projection.\n";
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    // 1. Place balls
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        //create_aux_form_for_ball_replace(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball", "-nolegal -nodetail");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        // run_replace("ball");
        // placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR + "/outputs/IBM/" + "ball" + "/experiment0/tiers/0/" + "ball.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.1] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.1-ball-global");
        draw_layout_result_plt(false, "-3.1-ball-global");
    }
    // 2. Place die0 with projected die1
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 0, "die0");
        // create_aux_form_replace(aux, 0, "die0");
        add_project_ball(aux);
        //add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die0");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        // run_replace("die0");
        // placer_succ = read_pl_and_set_pos(_RUNDIR + "/outputs/IBM/" + "die0" + "/experiment0/tiers/0/" + "die0.pl", 0);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        //add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.2] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.2-die0-legal");
        draw_layout_result_plt(false, "-3.2-die0-legal");
    }
    // 3. Place die1 with projected die0
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 1, "die1");
        // create_aux_form_replace(aux, 1, "die1");
        //add_project_pin(aux, 0);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die1");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 1);
        // run_replace("die1");
        // placer_succ = read_pl_and_set_pos(_RUNDIR + "/outputs/IBM/" + "die1" + "/experiment0/tiers/0/" + "die1.pl", 1);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.3] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.3-die1-legal");
        draw_layout_result_plt(false, "-3.3-die1-legal");
    }
    // 4. Replace die0 again with projected die1 and balls
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 0, "die0");
        //add_project_pin(aux, 1);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die0");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){ // replace balls
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.4] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.4-die0-re");
        draw_layout_result_plt(false, "-3.4-die0-re");
    }
    // 5. Replace die1 again with projected die0 and balls
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 1, "die1");
        //add_project_pin(aux, 0);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die1");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 1);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){ // replace balls
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.5] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.5-die1-re");
        draw_layout_result_plt(false, "-3.5-die1-re");
    }
    // 6. Replace die0 again with projected die1 and balls
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 0, "die0");
        //add_project_pin(aux, 1);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die0");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){ // replace balls
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - D2D-PL: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.6-die0-re");
        draw_layout_result_plt(false, "-3.6-die0-re");
    }
    return true;
}
void Placer_C::global_place(bool& isLegal, double& totalHPWL){ // Analytical Global Placement
    // 3d
    param.b3d = true;
    param.nlayer = 2;
    param.bLayerPreAssign = true;
    param.dWeightTSV = 0.5;
    //param.step = 5;
    param.stepZ = 2;

    // Setting placedb
    CPlaceDB placedb;
    create_placedb(placedb);
    set_ntuplace_param(placedb);
    // Setting placement region (Current: die0*2) /// TODO: setting die0 & die1
    placedb.m_totalLayer = param.nlayer;
    placedb.m_sites3d.resize(placedb.m_totalLayer);
    placedb.m_sites3d[0].resize(0);
    double row_bottom = placedb.m_coreRgn.bottom;
    double row_height = placedb.m_rowHeight;
    double row_step = placedb.m_sites[0].m_step;
    int count = 0;
    while(true){
        if(row_bottom + row_height > placedb.m_coreRgn.top) break;
        placedb.m_sites3d[0].push_back(CSiteRow(row_bottom, row_height, row_step ));
        placedb.m_sites3d[0].back().m_interval.push_back(placedb.m_coreRgn.left);
        placedb.m_sites3d[0].back().m_interval.push_back(placedb.m_coreRgn.right);
        placedb.m_sites3d[0].back().m_macro = placedb.m_sites[0].m_macro;
        if(count %2 == 0)
            placedb.m_sites3d[0].back().m_orient = OR_FS;
        char row_name[256];
        sprintf(row_name, "CORE_ROW_%d", count);
        placedb.m_sites3d[0].back().m_name = string(row_name);
        row_bottom += row_height;
        count++;
    }
    for(int l = 1; l < placedb.m_totalLayer; l++)
        placedb.m_sites3d[l] = placedb.m_sites3d[0];
    //placedb.ShowDBInfo();
	//CPlaceUtil::GetTotalOverlapArea( placedb ); // 2007-07-22 (donnie)
    placedb.RemoveFixedBlockSite();
    isLegal = multilevel_nlp( placedb, 5, 1.0 ); // multilevel_nlp(placedb, gCType, gWeightLevelDecreaingRate)
    load_from_placedb(placedb);
    set_ball(); // set ball for crossed net

    totalHPWL = cal_HPWL();
}
void Placer_C::legal_place(){
}
void Placer_C::detail_place(){
}
void Placer_C::ntu_d2d_global(bool& isLegal, double& totalHPWL){
    // partition
    if(_pChip->get_die(0)->get_row_num()==_pChip->get_die(1)->get_row_num())
        mincut_partition();
    else 
        mincut_k_partition();
    cout << BLUE << "[Placer]" << RESET << " - Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << "\n";
    
    rand_place(0);
    rand_place(1);
    rand_ball_place();
    
    // die0
    CPlaceDB placedb_die0;
    create_placedb(placedb_die0, 0);
    set_ntuplace_param(placedb_die0);
    if(1){
        placedb_die0.m_totalLayer = param.nlayer;
        placedb_die0.m_sites3d.resize(placedb_die0.m_totalLayer);
        placedb_die0.m_sites3d[0].resize(0);
        double row_bottom = placedb_die0.m_coreRgn.bottom;
		double row_height = placedb_die0.m_rowHeight;
		double row_step = placedb_die0.m_sites[0].m_step;
		int count = 0;
        while(true){
		    if(row_bottom + row_height > placedb_die0.m_coreRgn.top) break;
		    placedb_die0.m_sites3d[0].push_back(CSiteRow(row_bottom, row_height, row_step ));
		    placedb_die0.m_sites3d[0].back().m_interval.push_back(placedb_die0.m_coreRgn.left);
		    placedb_die0.m_sites3d[0].back().m_interval.push_back(placedb_die0.m_coreRgn.right);
		    placedb_die0.m_sites3d[0].back().m_macro = placedb_die0.m_sites[0].m_macro;
		    if(count %2 == 0)
			    placedb_die0.m_sites3d[0].back().m_orient = OR_FS;
		    char row_name[256];
		    sprintf(row_name, "CORE_ROW_%d", count);
		    placedb_die0.m_sites3d[0].back().m_name = string(row_name);
		    row_bottom += row_height;
		    count++;
		}
        for(int l = 1; l < placedb_die0.m_totalLayer; l++)
        	placedb_die0.m_sites3d[l] = placedb_die0.m_sites3d[0];
    }
    //placedb.ShowDBInfo();
	//CPlaceUtil::GetTotalOverlapArea( placedb ); // 2007-07-22 (donnie)
    placedb_die0.RemoveFixedBlockSite();
    isLegal = multilevel_nlp( placedb_die0, 5, 1.0 ); // multilevel_nlp(placedb, gCType, gWeightLevelDecreaingRate)
    load_from_placedb(placedb_die0);

    // die1
    CPlaceDB placedb_die1;
    create_placedb(placedb_die1, 1);
    set_ntuplace_param(placedb_die1);
    if(1){
        placedb_die1.m_totalLayer = param.nlayer;
        placedb_die1.m_sites3d.resize(placedb_die1.m_totalLayer);
        placedb_die1.m_sites3d[0].resize(0);
        double row_bottom = placedb_die1.m_coreRgn.bottom;
		double row_height = placedb_die1.m_rowHeight;
		double row_step = placedb_die1.m_sites[0].m_step;
		int count = 0;
        while(true){
		    if(row_bottom + row_height > placedb_die1.m_coreRgn.top) break;
		    placedb_die1.m_sites3d[0].push_back(CSiteRow(row_bottom, row_height, row_step ));
		    placedb_die1.m_sites3d[0].back().m_interval.push_back(placedb_die1.m_coreRgn.left);
		    placedb_die1.m_sites3d[0].back().m_interval.push_back(placedb_die1.m_coreRgn.right);
		    placedb_die1.m_sites3d[0].back().m_macro = placedb_die1.m_sites[0].m_macro;
		    if(count %2 == 0)
			    placedb_die1.m_sites3d[0].back().m_orient = OR_FS;
		    char row_name[256];
		    sprintf(row_name, "CORE_ROW_%d", count);
		    placedb_die1.m_sites3d[0].back().m_name = string(row_name);
		    row_bottom += row_height;
		    count++;
		}
        for(int l = 1; l < placedb_die1.m_totalLayer; l++)
        	placedb_die1.m_sites3d[l] = placedb_die1.m_sites3d[0];
    }
    //placedb.ShowDBInfo();
	//CPlaceUtil::GetTotalOverlapArea( placedb ); // 2007-07-22 (donnie)
    // if( 1 ){ // spreading
	//     globalLocalSpreading( &placedb_die0, 1.01 );
    // }
    placedb_die1.RemoveFixedBlockSite();
    isLegal = multilevel_nlp( placedb_die1, 5, 1.0 ); // multilevel_nlp(placedb, gCType, gWeightLevelDecreaingRate)
    load_from_placedb(placedb_die1);

    totalHPWL = cal_HPWL();
}
bool Placer_C::pin3d_ntu_d2d_legal_detail(){
    bool placer_succ;
    // run ntuplace for terminals
    if(cal_ball_num() > 0){
        AUX aux2;
        create_aux_form_for_ball(aux2, "ball");
        add_project_pin(aux2, 0);
        add_project_pin(aux2, 1);
        // check nets
        aux2.remove_open_net();
        aux2.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    // run ntuplace (noglobal) for die0
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux0;
        create_aux_form(aux0, 0, "die0");
        //add_project_pin(aux0, 1);
        add_project_ball(aux0);
        // check nets
        aux0.remove_open_net();
        aux0.write_files();
        run_ntuplace3("die0", "-noglobal");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    // run ntuplace (noglobal) for die1 
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux1;
        create_aux_form(aux1, 1, "die1");
        //add_project_pin(aux1, 0);
        add_project_ball(aux1);
        // check nets
        aux1.remove_open_net();
        aux1.write_files();
        run_ntuplace3("die1", "-noglobal");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 1);
        if(!placer_succ) return false;
    }
    int total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    // reRun ntuplace (noglobal) for die0
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux0;
        create_aux_form(aux0, 0, "die0");
        //add_project_pin(aux0, 1);
        add_project_ball(aux0);
        // check nets
        aux0.remove_open_net();
        aux0.write_files();
        run_ntuplace3("die0", "-noglobal");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    // reRun ntuplace for terminals
    if(cal_ball_num() > 0){
        AUX aux2;
        create_aux_form_for_ball(aux2, "ball");
        add_project_pin(aux2, 0);
        add_project_pin(aux2, 1);
        // check nets
        aux2.remove_open_net();
        aux2.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    // reRun ntuplace (noglobal) for die1 
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux1;
        create_aux_form(aux1, 1, "die1");
        //add_project_pin(aux1, 0);
        add_project_ball(aux1);
        // check nets
        aux1.remove_open_net();
        aux1.write_files();
        run_ntuplace3("die1", "-noglobal");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 1);
        if(!placer_succ) return false;
    }
    // reRun ntuplace for terminals
    if(cal_ball_num() > 0){
        AUX aux2;
        create_aux_form_for_ball(aux2, "ball");
        add_project_pin(aux2, 0);
        add_project_pin(aux2, 1);
        // check nets
        aux2.remove_open_net();
        aux2.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    // reRun ntuplace (noglobal) for die0
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux0;
        create_aux_form(aux0, 0, "die0");
        //add_project_pin(aux0, 1);
        add_project_ball(aux0);
        // check nets
        aux0.remove_open_net();
        aux0.write_files();
        run_ntuplace3("die0", "-noglobal");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    // reRun ntuplace for terminals
    if(cal_ball_num() > 0){
        AUX aux2;
        create_aux_form_for_ball(aux2, "ball");
        add_project_pin(aux2, 0);
        add_project_pin(aux2, 1);
        // check nets
        aux2.remove_open_net();
        aux2.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    return true;
}
bool Placer_C::ntu_d2d_legal_detail(){
    bool placer_succ;
    // run ntuplace (noglobal) for die0
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux0;
        create_aux_form(aux0, 0, "die0");
        //add_project_pin(aux0, 1);
        // check nets
        aux0.remove_open_net();
        aux0.write_files();
        run_ntuplace3("die0", "-noglobal");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    // run ntuplace (noglobal) for die1 
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux1;
        create_aux_form(aux1, 1, "die1");
        add_project_pin(aux1, 0);
        // check nets
        aux1.remove_open_net();
        aux1.write_files();
        run_ntuplace3("die1", "-noglobal");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 1);
        if(!placer_succ) return false;
    }
    // run ntuplace for terminals
    if(cal_ball_num() > 0){
        AUX aux2;
        create_aux_form_for_ball(aux2, "ball");
        add_project_pin(aux2, 0);
        add_project_pin(aux2, 1);
        // check nets
        aux2.remove_open_net();
        aux2.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    return true;
}
void Placer_C::set_ntuplace_param(CPlaceDB& placedb){
    param.dLpNorm_P = min(param.dLpNorm_P, _pChip->get_width()/6);
    // 3d
    // param.b3d = true;
    // param.nlayer = 2;
    // bShow >>
    if(_paramHdl.check_flag_exist("debug") || _paramHdl.check_flag_exist("d")){ // debug_mode
        param.bPlot = true;
        param.bShow = true;
        param.bLog = true;
        param.bOutTopPL = true;
    }
    // <<
    param.bRunInit = true;
    param.seed = 1;
    param.bUseLSE = false; // default
    param.bUseWAE = true; // default
    param.weightWire = 2.0;
	param.step = 0.2;
	param.stepAssigned = true;
    //srand( param.seed );
    // param.outFilePrefix = param.plFilename;
    // int len = param.outFilePrefix.length();
    // if( param.outFilePrefix.substr( len-3, 3 ) == ".pl" )
    //     param.outFilePrefix = param.outFilePrefix.erase( len-3, 3 );
    // // find the basename
	// string::size_type pos = param.outFilePrefix.rfind( "/" );
	// if( pos != string::npos )
	//     param.outFilePrefix = param.outFilePrefix.substr( pos+1 );
    // 2006-04-26 donnie (for IBM benchmarks)
    //cout << "placedb.m_util =================================== " << placedb.m_util << "\n";
    if( placedb.m_util > 0.79 ){
        if( param.stepAssigned == false ){
            if( param.bShow )
                printf( "**** Set step size to 0.2\n" );
            param.step = 0.2;
        }
        //if( param.cellMatchingStopAssigned == false )
        if( gArg.CheckExist( "stop" ) == false ){
            if( param.bShow )
                printf( "**** Set cell matching stop to 0.02\n" );
            //param.cellMatchingStop = 0.02;
        }
        if( param.bWireModelParameterAssigned == false ){
            if( param.bUseLSE ){
                if( param.bShow )
                    printf( "**** Set p to 100\n" );
                param.dLpNorm_P = 100;    // 100 for IBM, 300 for ISPD05/06
            }
            else{
                if( param.bShow )
                    printf( "**** Set p to 150\n" );
                param.dLpNorm_P = 150;    // 150 for IBM, 300 for ISPD05/06
            }
        }
    }
    if( param.bShow )
	    param.Print();
}
void Placer_C::create_placedb(CPlaceDB& placedb){
    bool add_dummy_node_as_ball = true;
    // .scl
    int rowH = ceil((_pChip->get_die(0)->get_row_height() + _pChip->get_die(1)->get_row_height())/2.0);
    int rowN = _pChip->get_height()/rowH;
    int rowW = _pChip->get_die(0)->get_width();
    vector<CSiteRow> &vSites = placedb.m_sites;
    for(int i=0;i<rowN;++i){
        vSites.push_back( CSiteRow(i*rowH,rowH,1) );
        vSites.back().m_interval.push_back(0);
        vSites.back().m_interval.push_back(rowW);
    }
    placedb.m_rowHeight = rowH;
    placedb.m_rowHeights.resize(param.nlayer,0);
    placedb.m_rowNums.resize(param.nlayer,0);
    placedb.m_maxUtils.resize(param.nlayer,0.0);
    for(int i=0;i<param.nlayer;++i){
        placedb.m_rowHeights[i] = _pChip->get_die(i)->get_row_height();
        placedb.m_rowNums[i] = _pChip->get_die(i)->get_row_num();
        placedb.m_maxUtils[i] = _pChip->get_die(i)->get_max_util();
    }
    placedb.SetCoreRegion();
    // .node
    vector<Cell_C*>& v_cell = _vCell;
    vector<Net_C*>& v_net = _vNet;
    int moduleNum = v_cell.size();
    if(add_dummy_node_as_ball)
        moduleNum += v_net.size();
    placedb.ReserveModuleMemory(moduleNum);
    for(Cell_C* cell : v_cell){ // Cells
        int cellH = rowH;
        int cellW = ceil((cell->get_width(0)+cell->get_width(1))/2);
        placedb.AddModule( cell->get_name(), cellW, cellH, false );
        // for terminal: placedb.AddModule( name, w, h, true );
        // for terminal_NI: placedb.AddModule( name, w, h, true, true );
        Module& curModule = placedb.m_modules.back();
        curModule.m_widths.resize(param.nlayer,0);
        curModule.m_heights.resize(param.nlayer,0);
        for(int i=0;i<param.nlayer;++i){
            curModule.m_widths[i] = cell->get_width(i);
            curModule.m_heights[i] = cell->get_height(i);
        }
    }
    if(add_dummy_node_as_ball){
        for(Net_C* net: v_net){ // Balls
            string ballName = net->get_name() + "_ball";
            placedb.AddModule( ballName, 0, 0, false );
            Module& curModule = placedb.m_modules.back();
            curModule.m_widths.resize(param.nlayer,0);
            curModule.m_heights.resize(param.nlayer,0);
            curModule.m_isVia = true;
        }
    }
    placedb.m_nModules = moduleNum; //fplan.m_nModules = nNodes + nTerminals;
    placedb.m_modules.resize( placedb.m_modules.size() );
    placedb.CreateModuleNameMap();
    // cutline for layer assignment
    double cutline = 0.5;
    double width_avg0 = 0;
    double width_avg1 = 0;
    for(Cell_C* cell : v_cell){
        width_avg0 += (double)cell->get_width(0)/ v_cell.size();
        width_avg1 += (double)cell->get_width(1)/ v_cell.size();
    }
    cutline = ((width_avg1*_pChip->get_die(1)->get_row_height()) / (width_avg1*_pChip->get_die(1)->get_row_height() + width_avg0*_pChip->get_die(0)->get_row_height())) * (_pChip->get_die(0)->get_max_util() / _pChip->get_die(1)->get_max_util());
    // if(add_dummy_node_as_ball){ 
    //     cutline *= 0.35;
    // }
    placedb.m_dCutline = cutline;
    // .nets
    int nPins = 0;
    for(Net_C* net : _vNet) // Pins 
        nPins += net->get_pin_num();
    if(add_dummy_node_as_ball){ // + Balls
        nPins += _vNet.size();
    }

    placedb.ReserveNetMemory( _vNet.size() );
    placedb.ReservePinMemory( nPins );
    int nReadNets = 0;
    for(Net_C* net : _vNet){
        Net net_db;
        vector<Pin_C*> v_pin = net->get_pins();
        net_db.reserve( v_pin.size()+1 ); // Pins + Ball
        // Pins
        for(int i=0;i<net->get_pin_num();++i){ 
            int moduleId = placedb.GetModuleId( v_pin[i]->get_cell()->get_name() );
            Cell_C* cell = v_pin[i]->get_cell();
            vector<Pos> pin_offsets(2);
            pin_offsets[0] = cell->get_master_cell()->get_pin_offset(_pChip->get_die(0)->get_techId() ,v_pin[i]->get_id());
            //pin_offsets[0].x -= cell->get_width(_pChip->get_die(0)->get_techId())/2; pin_offsets[0].y -= cell->get_height(_pChip->get_die(0)->get_techId())/2;
            pin_offsets[1] = cell->get_master_cell()->get_pin_offset(_pChip->get_die(1)->get_techId() ,v_pin[i]->get_id());
            //pin_offsets[1].x -= cell->get_width(_pChip->get_die(1)->get_techId())/2; pin_offsets[1].y -= cell->get_height(_pChip->get_die(1)->get_techId())/2;
            Pos pin_offset = Pos(ceil((pin_offsets[0].x+pin_offsets[1].x)/2.0),ceil((pin_offsets[0].y+pin_offsets[1].y)/4.0));
            int pinId = placedb.AddPin( moduleId, pin_offset.x, pin_offset.y );
            net_db.push_back( pinId );
            placedb.m_pins[pinId].xOffs.resize(param.nlayer,0);
            placedb.m_pins[pinId].yOffs.resize(param.nlayer,0);
            for(int i=0;i<param.nlayer;++i){
                placedb.m_pins[pinId].xOffs[i] = pin_offsets[i].x;
                placedb.m_pins[pinId].yOffs[i] = pin_offsets[i].y;
            }
            placedb.m_modules[moduleId].m_netsId.push_back( nReadNets );
        }
        // Ball
        if(add_dummy_node_as_ball){
            int BallId = placedb.GetModuleId( net->get_name()+"_ball" );
            int pinId = placedb.AddPin( BallId, 1, 1 );
            net_db.push_back( pinId );
            placedb.m_pins[pinId].xOffs.resize(2,1);
            placedb.m_pins[pinId].yOffs.resize(2,1);
            placedb.m_modules[BallId].m_netsId.push_back( nReadNets );
        }

        placedb.AddNet( net_db );
        nReadNets++;
    }

    placedb.m_nPins = nPins;
	placedb.m_nNets = _vNet.size();
    placedb.m_pins.resize( placedb.m_pins.size() );
    placedb.m_nets.resize( placedb.m_nets.size() );
    // .pl
    for(Cell_C* cell : v_cell){ // Cells
        int moduleId = placedb.GetModuleId( cell->get_name() );
        placedb.SetModuleLocation( moduleId, cell->get_posX(), cell->get_posY());
		placedb.SetModuleOrientation( moduleId, 0 ); // orientInt('N')=0
        if(param.bLayerPreAssign){
            placedb.SetModuleLayerAssign( moduleId, cell->get_posZ());
        }
    }
    if(add_dummy_node_as_ball){ 
        for(Net_C* net: v_net){ // Balls
            int BallId = placedb.GetModuleId( net->get_name()+"_ball" );
            placedb.SetModuleLocation( BallId, net->get_ball_pos().x, net->get_ball_pos().y);
            placedb.SetModuleOrientation( BallId, 0 );
            if(param.bLayerPreAssign){
                placedb.SetModuleLayerAssign( BallId, 0);
            }
        }
    }
    placedb.ClearModuleNameMap();

    placedb.Init(); // set member variables, module member variables, "isOutCore"
    // double coreCenterX = (placedb.m_coreRgn.left + placedb.m_coreRgn.right ) * 0.5;
	// double coreCenterY = (placedb.m_coreRgn.top + placedb.m_coreRgn.bottom ) * 0.5;
	// for( unsigned int i = 0; i < placedb.m_modules.size(); i++ ){
	//     if( !placedb.m_modules[i].m_isFixed &&
	//     	 fabs(placedb.m_modules[i].m_x) < 1e-5 && fabs(placedb.m_modules[i].m_y) < 1e-5 )
	//     	placedb.MoveModuleCenter(i, coreCenterX, coreCenterY);
	// }
}
void Placer_C::create_placedb(CPlaceDB& placedb, int dieId){
    // .scl
    vector<CSiteRow> &vSites = placedb.m_sites;
    for(int i=0;i<_pChip->get_die(dieId)->get_row_num();++i){
        vSites.push_back( CSiteRow(i*_pChip->get_die(dieId)->get_row_height(),_pChip->get_die(dieId)->get_row_height(),1) );
        vSites.back().m_interval.push_back(0);
        vSites.back().m_interval.push_back(_pChip->get_die(dieId)->get_width());
    }
    placedb.m_rowHeight = _pChip->get_die(dieId)->get_row_height();
    placedb.SetCoreRegion();
    // .node
    placedb.ReserveModuleMemory(_pChip->get_die(dieId)->get_cells().size());
    vector<Cell_C*>& v_cell = _pChip->get_die(dieId)->get_cells();
    for(Cell_C* cell : v_cell){
        placedb.AddModule( cell->get_name(), cell->get_width(), cell->get_height(), false );
        // for terminal: placedb.AddModule( name, w, h, true );
        // for terminal_NI: placedb.AddModule( name, w, h, true, true );
    }
    placedb.m_nModules = _pChip->get_die(dieId)->get_cells().size(); //fplan.m_nModules = nNodes + nTerminals;
    placedb.m_modules.resize( placedb.m_modules.size() );
    placedb.CreateModuleNameMap();
    // .nets
    unordered_map<string,vector<Pin_C*> > m_net;
    int nPins = 0;
    for(Cell_C* cell : v_cell){
        for(int i=0;i<cell->get_pin_num();++i){
            ++nPins;
            Pin_C* pin = cell->get_pin(i);
            if(pin->get_net() != nullptr){
                auto it = m_net.find(pin->get_net()->get_name());
                if(it == m_net.end()){
                    vector<Pin_C*> v_pin{pin};
                    m_net.emplace(pin->get_net()->get_name(), v_pin);
                }
                else{
                    it->second.push_back(pin);
                }
            }
        }
    }
    placedb.ReserveNetMemory( m_net.size() );
    placedb.ReservePinMemory( nPins );
    int nReadNets = 0;
    for(auto& it : m_net){
        Net net;
        net.reserve( it.second.size() );
        for(int i=0;i<it.second.size();++i){
            int moduleId = placedb.GetModuleId( it.second[i]->get_cell()->get_name() );
            int xOff = it.second[i]->get_x() - it.second[i]->get_cell()->get_posX();
            int yOff = it.second[i]->get_y() - it.second[i]->get_cell()->get_posY();
            int pinId = placedb.AddPin( moduleId, xOff, yOff );
            net.push_back( pinId );
            // remove duplicated netsIds
            bool found = false; 
            for(unsigned int z = 0 ; z < placedb.m_modules[moduleId].m_netsId.size() ; z++ ){
                if ( nReadNets == placedb.m_modules[moduleId].m_netsId[z] ){
                    found = true;
                    break;
                }
            }
            if (!found) placedb.m_modules[moduleId].m_netsId.push_back( nReadNets );
        }
        placedb.AddNet( net );
        nReadNets++;
    }
    placedb.m_nPins = nPins;
	placedb.m_nNets = m_net.size();
    placedb.m_pins.resize( placedb.m_pins.size() );
    placedb.m_nets.resize( placedb.m_nets.size() );
    // .pl
    for(Cell_C* cell : v_cell){
        int moduleId = placedb.GetModuleId( cell->get_name() );
        char dir[1000]; dir[0] = 'N'; dir[1] = '\0';
        placedb.SetModuleLocation( moduleId, cell->get_posX(), cell->get_posY());
		placedb.SetModuleOrientation( moduleId, 0 ); // orientInt('N')=0
    }
    placedb.ClearModuleNameMap();

    placedb.Init(); // set member variables, module member variables, "isOutCore"
    // double coreCenterX = (placedb.m_coreRgn.left + placedb.m_coreRgn.right ) * 0.5;
	// double coreCenterY = (placedb.m_coreRgn.top + placedb.m_coreRgn.bottom ) * 0.5;
	// for( unsigned int i = 0; i < placedb.m_modules.size(); i++ ){
	//     if( !placedb.m_modules[i].m_isFixed &&
	//     	 fabs(placedb.m_modules[i].m_x) < 1e-5 && fabs(placedb.m_modules[i].m_y) < 1e-5 )
	//     	placedb.MoveModuleCenter(i, coreCenterX, coreCenterY);
	// }
}
void Placer_C::load_from_placedb(CPlaceDB& placedb){
    int count_zChanged = 0;
    for( unsigned int i=0; i<placedb.m_modules.size(); i++ ){
        if( !placedb.m_modules[i].m_isFixed ){
            string moduleName = placedb.m_modules[i].GetName();
            if(moduleName.size() > 5 && moduleName.substr(moduleName.size()-5,5) == "_ball"){ // Ball
                string netName = moduleName.substr(0,moduleName.size()-5);
                Net_C* net = _mNet[netName];
                net->set_ball_xy(Pos(placedb.m_modules[i].GetX(), placedb.m_modules[i].GetY()));
            }
            else{
                Cell_C* cell = _mCell[placedb.m_modules[i].GetName()];
                cell->set_xy(Pos(placedb.m_modules[i].GetX(), placedb.m_modules[i].GetY()));
                if(param.b3d){
                    if(cell->get_posZ() != placedb.m_modules[i].GetZ())
                        ++count_zChanged;
                        //cout << "~~~~~~~~~~~ " << cell->get_name() << ".z() changed from "<<cell->get_posZ()<<" to " << placedb.m_modules[i].GetZ() << "\n";
                    cell->set_die(_pChip->get_die((int)placedb.m_modules[i].GetZ()));
                }
            }
        }
    }
    if(param.b3d){
        cout << BLUE << "[Placer]" << RESET << " - Global: " << count_zChanged << " cells changed die.\n";
    }
}

bool Placer_C::order_place(){
    cout << BLUE << "[Placer]" << RESET << " - Start order_palce flow.\n";
    int curDie = 0;
    int curRow = 0;
    int curX = 0;
    long long valid_area = (long long)_pChip->get_width() * (long long)_pChip->get_height() * _pChip->get_die(curDie)->get_max_util();
    
    for(Cell_C* cell : _vCell){
        cell->set_die(_pChip->get_die(curDie));
        long long cell_area = (long long)(cell->get_width()*cell->get_height());
        if(valid_area > cell_area){
            valid_area -= cell_area;
        }else{
            curDie += 1;
            curRow = 0;
            curX = 0;
            valid_area = (long long)_pChip->get_width() * (long long)_pChip->get_height() * _pChip->get_die(curDie)->get_max_util();
            cell->set_die(_pChip->get_die(curDie));
            cell_area = (long long)(cell->get_width()*cell->get_height());
            valid_area -= cell_area;
        }

        if(curX + cell->get_width(_pChip->get_die(curDie)->get_techId()) <= _pChip->get_width()){
            cell->set_xy(Pos(curDie, curX, curRow*_pChip->get_die(curDie)->get_row_height()));
            cell->set_die(_pChip->get_die(curDie));
            curX += cell->get_width(_pChip->get_die(curDie)->get_techId());
        } else if(curRow+1 < _pChip->get_die(curDie)->get_row_num()){
            curRow += 1;
            curX = 0;
            cell->set_xy(Pos(curDie, curX, curRow*_pChip->get_die(curDie)->get_row_height()));
            cell->set_die(_pChip->get_die(curDie));
            curX += cell->get_width(_pChip->get_die(curDie)->get_techId());
        // } else if(curDie < 1){
        //     curDie += 1;
        //     curRow = 0;
        //     curX = 0;
        //     cell->set_xy(Pos(curDie, curX, curRow*_pChip->get_die(curDie)->get_row_height()));
        //     cell->set_die(_pChip->get_die(curDie));
        //     curX += cell->get_width(_pChip->get_die(curDie)->get_techId());
        } else{
			cout << "_pChip->get_width()=" << _pChip->get_width() << "\n";
			cout << "curX + cell->get_width("<<curDie<<") = " << curX << " + " << cell->get_width(_pChip->get_die(curDie)->get_techId()) << " = " << curX + cell->get_width(_pChip->get_die(curDie)->get_techId()) << "\n";
            cout << BLUE << "[Placer]" << RESET << " - " << YELLOW << "Warning! " << RESET << "No more space to place for cell \'" << cell->get_name() << "\', the cell is placed at (0,0,0).\n";
            cell->set_xy(Pos(0,0,0));
            cell->set_die(_pChip->get_die(0));
        }
    }
    int ball_curX = _pChip->get_ball_spacing() + _pChip->get_ball_width()/2.0;
    int ball_curY = _pChip->get_ball_spacing() + _pChip->get_ball_width()/2.0;
    vector<Net_C*>& v_d2dNets = _pChip->get_d2d_nets();
    v_d2dNets.clear();
    for(Net_C* net : _vNet){
        if(net->is_cross_net()){
            v_d2dNets.emplace_back(net);
            if(ball_curX + _pChip->get_ball_width()/2.0 + _pChip->get_ball_spacing() <= _pChip->get_width() && ball_curY + _pChip->get_ball_height()/2.0 + _pChip->get_ball_spacing() <= _pChip->get_height()){
                net->set_ball_xy(Pos(ball_curX, ball_curY));
                ball_curX += _pChip->get_ball_width() + _pChip->get_ball_spacing();
            } else if(ball_curY + _pChip->get_ball_height()/2.0 + _pChip->get_ball_spacing() <= _pChip->get_height()){
                ball_curX = _pChip->get_ball_spacing() + _pChip->get_ball_width()/2.0;
                ball_curY += _pChip->get_ball_height() + _pChip->get_ball_spacing();
                net->set_ball_xy(Pos(ball_curX, ball_curY));
                ball_curX += _pChip->get_ball_width() + _pChip->get_ball_spacing();
            } else{
                cout << BLUE << "[Placer]" << RESET << " - " << YELLOW << "Warning! " << RESET << "No more space to place for tarminal of net \'" << net->get_name() << "\', the terminal is placed at (0,0).\n";
                net->set_ball_xy(Pos(0,0));
            }
        }
    }
    return true;
}

void Placer_C::cell_spreading(){
    for(Cell_C* cell : _vCell)
        cell->set_die(_pChip->get_die(0));
    // run ntuplace with half cell height
    AUX aux2D;
    string caseName = "cell-spreading";
    // >>> create_aux_form();
    {
        string aux_dir = _RUNDIR + caseName + "/";
        string cmd = "mkdir -p " + aux_dir;
        system(cmd.c_str());
        aux2D = AUX(aux_dir, caseName);
        // shrunking
        int rowH = ceil((_pChip->get_die(0)->get_row_height() + _pChip->get_die(1)->get_row_height())/4.0);
        // nodes
        vector<Cell_C*>& v_cells = _vCell;
        for(Cell_C* cell : v_cells){
            int cellW = ceil((cell->get_width(0)+cell->get_width(1))/2.0);
            aux2D.add_node(cell->get_name(), cellW, rowH, cell->get_posX(), cell->get_posY(),0);
            for(int i=0;i<cell->get_pin_num();++i){
                Pin_C* pin = cell->get_pin(i);
                Net_C* net = pin->get_net();
                if(net != nullptr){
                    char IO = 'I';
                    if(!aux2D.check_net_exist(net->get_name())){
                        aux2D.add_net(net->get_name());
                        IO='O';
                    }
                    Pos pin_offset0 = cell->get_master_cell()->get_pin_offset(_pChip->get_die(0)->get_techId() ,pin->get_id());
                    //pin_offset0.x -= cell->get_width()/2; pin_offset0.y -= cell->get_height()/2;
                    Pos pin_offset1 = cell->get_master_cell()->get_pin_offset(_pChip->get_die(1)->get_techId() ,pin->get_id());
                    //pin_offset1.x -= cell->get_width()/2; pin_offset1.y -= cell->get_height()/2;
                    Pos pin_offset = Pos(ceil((pin_offset0.x+pin_offset1.x)/2.0),ceil((pin_offset0.y+pin_offset1.y)/4.0));
                    aux2D.add_pin(net->get_name(), pin->get_cell()->get_name(), IO, pin_offset.x, pin_offset.y);
                }
            }
        }
        // rows
        aux2D.set_default_rows(_pChip->get_width(), rowH, _pChip->get_height()/rowH);
    }// <<< create_aux_form();

    aux2D.write_files();
    run_ntuplace3(caseName, "-nolegal -nodetail");
    read_pl_and_set_pos(_RUNDIR+caseName+".ntup.pl");
}
void Placer_C::cell_spreading(int dieId){
    rand_place(dieId);
}
void Placer_C::rand_place(int dieId){
    for(Cell_C* cell : _vCell){
        if(cell->get_dieId() == dieId){
            int x = rand()%(_pChip->get_width()-cell->get_width());
            int y = rand()%(_pChip->get_height()-cell->get_height());
            cell->set_xy(Pos(x,y));
        }
    }
}
void Placer_C::rand_ball_place(){
    vector<Net_C*>& v_d2dNets = _pChip->get_d2d_nets();
    v_d2dNets.clear();
    for(Net_C* net : _vNet){
        int x = rand()%(_pChip->get_width());
        int y = rand()%(_pChip->get_height());
        net->set_ball_xy(Pos(x,y));
        if(net->is_cross_net()){
            v_d2dNets.emplace_back(net);
        }
    }
}
void Placer_C::init_ball_place(){
    vector<Net_C*>& v_d2dNets = _pChip->get_d2d_nets();
    v_d2dNets.clear();
    for(Net_C* net : _vNet){
        int sumX=0, sumY=0, avgX=0, avgY=0;
        vector<Pin_C*>& v_pin = net->get_pins();
        for(Pin_C* pin : v_pin){
            sumX += pin->get_x();
            sumY += pin->get_y();
        }
        if(v_pin.size() > 0) {
            avgX = sumX / v_pin.size();
            avgY = sumY / v_pin.size();
        }
        net->set_ball_xy(Pos(avgX, avgY));

        if(net->is_cross_net()){
            v_d2dNets.emplace_back(net);
        }
    }
}
void Placer_C::set_ball(){
    vector<Net_C*>& v_d2dNets = _pChip->get_d2d_nets();
    v_d2dNets.clear();
    for(Net_C* net : _vNet){
        if(net->is_cross_net()){
            v_d2dNets.emplace_back(net);
        }
    }
}

bool Placer_C::shrunk2d_ntuplace(){
    double part_time_start=0, total_part_time=0;
    cout << BLUE << "[Placer]" << RESET << " - Start Pseudo3D Placement Flow (Shrunk2D).\n";
    
    bool placer_succ;
    ////////////////////////////////////////////////////////////////
    // Init Placement (2D Placement)
    ////////////////////////////////////////////////////////////////
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 1]" << RESET << ": Init 2D Placement.\n";
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    placer_succ = shrunked_2d_ntuplace("-nolegal -nodetail");
    if(!placer_succ) return false;

    ////////////////////////////////////////////////////////////////
    // Partition
    ////////////////////////////////////////////////////////////////
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 2]" << RESET << ": Min-Cut Partition.\n";
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    if(_pChip->get_die(0)->get_row_num()==_pChip->get_die(1)->get_row_num())
        mincut_partition(); // set_die() for each cell
    else 
        mincut_k_partition(); // set_die() for each cell
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Partition: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    cout << BLUE << "[Placer]" << RESET << " - Partition result: " << "Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << ".\n";

    ////////////////////////////////////////////////////////////////
    // D2D Placement with Pin Projection
    ////////////////////////////////////////////////////////////////
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 3]" << RESET << ": D2D LG+DP with Pin Projection.\n";
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    // 1. Place balls
    rand_ball_place();
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball", "-nolegal -nodetail");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    // 2. Place die0 with projected die1
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 0, "die0");
        add_project_ball(aux);
        //add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die0");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        //add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    // 3. Place die1 with projected die0
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 1, "die1");
        //add_project_pin(aux, 0);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die1");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 1);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    int total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.3] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    // 4. Replace die0 again with projected die1 and balls
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 0, "die0");
        //add_project_pin(aux, 1);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die0");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    // replace balls
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.4] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    // 5. Replace die0 again with projected die1 and balls
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 1, "die1");
        //add_project_pin(aux, 0);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die1");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 1);
        if(!placer_succ) return false;
    }
    // replace balls
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.5] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    // 6. Replace die0 again with projected die1 and balls
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 0, "die0");
        //add_project_pin(aux, 1);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die0");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    // replace balls
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - D2D-PL: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    return true;
}
bool Placer_C::shrunk2d_replace(){
    double part_time_start=0, total_part_time=0;
    cout << BLUE << "[Placer]" << RESET << " - Start Pseudo3D Placement Flow (Shrunk2D-RePlAce).\n";
    
    bool placer_succ;
    ////////////////////////////////////////////////////////////////
    // Init Placement (2D Placement)
    ////////////////////////////////////////////////////////////////
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 1]" << RESET << ": Init 2D Placement.\n";
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    placer_succ = shrunked_2d_replace();
    if(!placer_succ) return false;
    int total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [1] Init 2D GP (RePlAce) total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-1-2Dplace");
        draw_layout_result_plt(false, "-1-2Dplace");
    }
    ////////////////////////////////////////////////////////////////
    // Partition
    ////////////////////////////////////////////////////////////////
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 2]" << RESET << ": Min-Cut Partition.\n";
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    // if(_pChip->get_die(0)->get_row_num()==_pChip->get_die(1)->get_row_num())
    //     mincut_partition(); // set_die() for each cell
    // else 
    //     mincut_k_partition(); // set_die() for each cell

    // if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
    //     draw_layout_result("-init-tier-partition");
    //     draw_layout_result_plt(false, "-init-tier-partition");
    // }
    // cout << BLUE << "[Placer]" << RESET << " - Partition: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    cout << BLUE << "[Placer]" << RESET << " - Partition result: " << "Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << ".\n";
    bin_based_partition_new();
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Partition: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    cout << BLUE << "[Placer]" << RESET << " - Partition result: " << "Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << ".\n";
    init_ball_place();
    cout << BLUE << "[Placer]" << RESET << " - #Terminal = " << cal_ball_num() << "\n";
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [2] Tier-Partition total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-2-tier-partition");
        draw_layout_result_plt(false, "-2-tier-partition");
    }

    ////////////////////////////////////////////////////////////////
    // D2D Placement with Pin Projection
    ////////////////////////////////////////////////////////////////
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 3]" << RESET << ": D2D LG+DP with Pin Projection.\n";
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    if(cal_ball_num() > 0){
        AUX aux;
        // create_aux_form_for_ball_replace(aux, "ball");
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        // run_replace("ball");
        run_ntuplace3("ball");
        // placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR + "/outputs/IBM/" + "ball" + "/experiment0/tiers/0/" + "ball.pl");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.1] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.1-ball-place");
        draw_layout_result_plt(false, "-3.1-ball-place");
    }
    // 2. Place die0 with projected die1
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        create_aux_form_replace(aux, 0, "die0");
        add_project_ball(aux);
        //add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_replace("die0");
        placer_succ = read_pl_and_set_pos(_RUNDIR + "/outputs/IBM/" + "die0" + "/experiment0/tiers/0/" + "die0.pl", 0);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        //add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.2] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.2-die0-global");
        draw_layout_result_plt(false, "-3.2-die0-global");
    }
    // 3. Place die1 with projected die0
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux;
        create_aux_form_replace(aux, 1, "die1");
        //add_project_pin(aux, 0);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_replace("die1");
        placer_succ = read_pl_and_set_pos(_RUNDIR + "/outputs/IBM/" + "die1" + "/experiment0/tiers/0/" + "die1.pl", 1);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.3] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.3-die1-global");
        draw_layout_result_plt(false, "-3.3-die1-global");
    }
    // 4. legal die0 again with projected die1 and balls
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 0, "die0");
        //add_project_pin(aux, 1);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die0", "-noglobal");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){ // replace balls
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.4] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.4-die0-legal");
        draw_layout_result_plt(false, "-3.4-die0-legal");
    }
    // 5. legal die1 again with projected die0 and balls
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 1, "die1");
        //add_project_pin(aux, 0);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die1", "-noglobal");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 1);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){ // replace balls
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.5] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.5-die1-legal");
        draw_layout_result_plt(false, "-3.5-die1-legal");
    }
    // 6. replace die0 again with projected die1 and balls
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 0, "die0");
        //add_project_pin(aux, 1);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die0", "-noglobal");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    if(cal_ball_num() > 0){ // replace balls
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.6] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    if(!_paramHdl.check_flag_exist("no_draw") || !_paramHdl.check_flag_exist("only_draw_result")){
        draw_layout_result("-3.6-die0-re");
        draw_layout_result_plt(false, "-3.6-die0-re");
    }
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - D2D-PL: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    
    return true;
}

bool Placer_C::pin3d_ntuplace(){
    double part_time_start=0, total_part_time=0;
    cout << BLUE << "[Placer]" << RESET << " - Start Pseudo3D Placement Flow (Pin3D).\n";
    
    ////////////////////////////////////////////////////////////////
    // Partition
    ////////////////////////////////////////////////////////////////
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 1]" << RESET << ": Min-Cut Partition.\n";
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    if(_pChip->get_die(0)->get_row_num()==_pChip->get_die(1)->get_row_num())
        mincut_partition(); // set_die() for each cell
    else 
        mincut_k_partition(); // set_die() for each cell
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Partition: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    cout << BLUE << "[Placer]" << RESET << " - Partition result: " << "Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << ".\n";

    bool placer_succ;
    ////////////////////////////////////////////////////////////////
    // Init Placement
    ////////////////////////////////////////////////////////////////
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 2]" << RESET << ": Init Placement.\n";
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    rand_place(0);
    rand_place(1);
    rand_ball_place();
    // run ntuplace for die0
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux0;
        create_aux_form(aux0, 0, "die0");
        // check nets
        aux0.remove_open_net();
        aux0.write_files();
        run_ntuplace3("die0");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    // run ntuplace for die1
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux1;
        create_aux_form(aux1, 1, "die1");
        // check nets
        aux1.remove_open_net();
        aux1.write_files();
        run_ntuplace3("die1");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 1);
        if(!placer_succ) return false;
    }
    // run ntuplace for terminal
    if(cal_ball_num() > 0){
        AUX aux2;
        create_aux_form_for_ball(aux2, "ball");
        add_project_pin(aux2, 0);
        add_project_pin(aux2, 1);
        // check nets
        aux2.remove_open_net();
        aux2.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Init: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    int total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    ////////////////////////////////////////////////////////////////
    // D2D Placement with Pin Projection
    ////////////////////////////////////////////////////////////////
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 3]" << RESET << ": D2D Placement with Pin Projection.\n";
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    // 1. Replace die0 with projected die1
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 0, "die0");
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die0");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    // replace balls
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.1] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    // 2. Replace die1 with projected die0 and balls
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 1, "die1");
        add_project_pin(aux, 0);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die1");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    // replace balls
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.2] total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    // 3. Replace die0 again with projected die1 and balls
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 0, "die0");
        add_project_pin(aux, 1);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die0");
        placer_succ = read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
        if(!placer_succ) return false;
    }
    // replace balls
    if(cal_ball_num() > 0){
        AUX aux;
        create_aux_form_for_ball(aux, "ball");
        add_project_pin(aux, 0);
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("ball");
        placer_succ = read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
        if(!placer_succ) return false;
    }
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - D2D-PL: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    return true;
}
bool Placer_C::random_d2dplace(){
    cout << BLUE << "[Placer]" << RESET << " - Start Random Placement Flow.\n";
    double part_time_start=0, total_part_time=0;
    ////////////////////////////////////////////////////////////////
    // Partition
    ////////////////////////////////////////////////////////////////
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 1]" << RESET << ": Min-Cut Partition.\n";
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    if(_pChip->get_die(0)->get_row_num()==_pChip->get_die(1)->get_row_num())
        mincut_partition(); // set_die() for each cell
    else 
        mincut_k_partition(); // set_die() for each cell
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Partition: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    cout << BLUE << "[Placer]" << RESET << " - Partition result: " << "Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << ".\n";
    ////////////////////////////////////////////////////////////////
    // Global Placement
    ////////////////////////////////////////////////////////////////
    bool isLegal = false; 
    double wl1 = 0; // gp-wire
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 2]" << RESET << ": Global Random Placement.\n";
    rand_place(0);
    rand_place(1);
    rand_ball_place();
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Global: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    ////////////////////////////////////////////////////////////////
    // Legal + Detail Placement
    ////////////////////////////////////////////////////////////////
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 3]" << RESET << ": Legalization + Detail Placement.\n";
    ntu_d2d_legal_detail(); ////////////////////////////////////////////////////// main function
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    int total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    return true;
}
bool Placer_C::ntu_d2dplace(){
    double part_time_start=0, total_part_time=0;
    cout << BLUE << "[Placer]" << RESET << " - Start NTU_d2d Placement Flow.\n";
    ////////////////////////////////////////////////////////////////
    // Partition
    ////////////////////////////////////////////////////////////////
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 1]" << RESET << ": Min-Cut Partition.\n";
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    if(_pChip->get_die(0)->get_row_num()==_pChip->get_die(1)->get_row_num())
        mincut_partition(); // set_die() for each cell
    else 
        mincut_k_partition(); // set_die() for each cell
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Partition: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    cout << BLUE << "[Placer]" << RESET << " - Partition result: " << "Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << ".\n";
    
    rand_place(0);
    rand_place(1);
    rand_ball_place();
    //init_place_ball();

    ////////////////////////////////////////////////////////////////
    // Run NTUplace
    ////////////////////////////////////////////////////////////////
    // run ntuplace for die0
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 2.1]" << RESET << ": NTUplace for die0.\n";
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux0;
        create_aux_form(aux0, 0, "die0");
        // check nets
        aux0.remove_open_net();
        aux0.write_files();
        run_ntuplace3("die0");
        //run_ntuplace4("die0");
        read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
    }
    // run ntuplace for die1
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 2.2]" << RESET << ": NTUplace for die1.\n";
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux1;
        create_aux_form(aux1, 1, "die1");
        add_project_pin(aux1, 0);
        // check nets
        aux1.remove_open_net();
        aux1.write_files();
        run_ntuplace3("die1");
        //run_ntuplace4("die1");
        read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 1);
    }
    // run ntuplace for terminal
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 2.3]" << RESET << ": NTUplace for terminals.\n";
    int countTerminal = 0;
    for(int i=0;i<_pDesign->get_net_num();++i){
        Net_C* net = _pDesign->get_net(i);
        if(net->is_cross_net()){
            ++countTerminal;
        }
    }
    if(countTerminal > 0){
        AUX aux2;
        create_aux_form_for_ball(aux2, "ball");
        add_project_pin(aux2, 0);
        add_project_pin(aux2, 1);
        // check nets
        aux2.remove_open_net();
        aux2.write_files();
        run_ntuplace3("ball");
        read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
    }
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - d2d_NTUpalce: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    int total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - d2d_NTUpalce: total HPWL = " << CYAN << total_hpwl << RESET << ".\n";
    //string cmd_clean = "rm -rf *.pl *.plt *.log " + _RUNDIR;
    //system(cmd_clean.c_str());
    return true;
}

bool Placer_C::coloquinte_place(){
    cout << BLUE << "[Placer]" << RESET << " - Coloquinte Global Place...\n";
    for(Cell_C* cell : _vCell)
        cell->set_die(_pChip->get_die(0));
    // shrunking
    int rowH = ceil((_pChip->get_die(0)->get_row_height() + _pChip->get_die(1)->get_row_height())/4.0);

    string caseName = "coloquinte";
    string fileName = _RUNDIR + caseName + ".txt";
    ofstream fout(fileName);
    // <x_min> <x_max> <y_min> <y_max>
    fout << 0 << " " << _pChip->get_width()*2 << " " << 0 << " " << _pChip->get_height()*2 << "\n";
    // <cell_cnt>
    fout << _vCell.size() << "\n";
    for(int i=0;i<_vCell.size();++i){
        int cellW = ceil((_vCell[i]->get_width(0)+_vCell[i]->get_width(1))/2.0);
        // <cell_ind> <x_s> <y_s>
        fout << i << " " << cellW << " " << rowH << "\n";
    }
    for(int i=0;i<_vCell.size();++i){
        // <cell_ind> <x> <y> <fixed>
        fout << i << " " << _vCell[i]->get_posX() << " " << _vCell[i]->get_posY() << " " << 0 << "\n";
    }
    // <net_cnt>
    fout << _vNet.size() << "\n";
    for(int i=0;i<_vNet.size();++i){
        // <pin_cnt>
        fout << _vNet[i]->get_pin_num() << "\n";
        for(int j=0;j<_vNet[i]->get_pin_num();++j){
            Pin_C* pin = _vNet[i]->get_pin(j);
            Pos pin_offset0 = pin->get_cell()->get_master_cell()->get_pin_offset(_pChip->get_die(0)->get_techId() ,pin->get_id());
            Pos pin_offset1 = pin->get_cell()->get_master_cell()->get_pin_offset(_pChip->get_die(1)->get_techId() ,pin->get_id());
            Pos pin_offset = Pos(ceil((pin_offset0.x+pin_offset1.x)/2.0),ceil((pin_offset0.y+pin_offset1.y)/4.0));
            // <cell_ind> <x> <y>
            fout << pin->get_cell()->get_id() << " " << pin_offset.x << " " << pin_offset.y << "\n";
        }
    }
    fout.close();
    cout << BLUE << "[Placer]" << RESET << " - Coloquinte Global Place" << GREEN << " completed!\n" << RESET;
    //exit(1);
    return true;
}

void Placer_C::mincut_partition(){
    HGR hgr(_RUNDIR, "circuit");
    for(Net_C* net : _vNet){
        hgr.add_net(net->get_name());
        for(int i=0;i<net->get_pin_num();++i){
            hgr.add_node(net->get_name(), net->get_pin(i)->get_cell()->get_name());
        }
    }
    hgr.write_hgr();
    run_hmetis(2, 0.7, "circuit");
    hgr.read_part_result(2);
    //cout << "hmetis partition result: " << max(hgr.get_part_size(0),hgr.get_part_size(1)) << " : " << min(hgr.get_part_size(0),hgr.get_part_size(1)) << "\n";
    for(Cell_C* cell : _vCell){
        int part = (hgr.get_part_size(0) >= hgr.get_part_size(1))? 
            hgr.get_part_result(cell->get_name()) : (hgr.get_part_result(cell->get_name())+1)%2;
        cell->set_die(_pChip->get_die(part));
    }
    //cout << "Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell)num = " << _pChip->get_die(1)->get_cells().size() << "\n";

    // move cells for matching die's max_utilization
    long long valid_area = (long long)_pChip->get_width() * (long long)_pChip->get_height() * _pChip->get_die(0)->get_max_util();
    //long long total_area = 0;
    Die_C* die = _pChip->get_die(0);
    vector<Cell_C*>& v_cell = die->get_cells();
    vector<int> v_cellId;
    for(Cell_C* cell : v_cell){
        v_cellId.emplace_back(cell->get_id());
        //total_area += (long long)cell->get_width() * (long long)cell->get_height();
    }
    int count_move = 0;
    for(int i=0;i<v_cellId.size();++i){
        Cell_C* cell = _vCell[v_cellId[i]];
        long long cell_area = (long long)(cell->get_width()*cell->get_height());
        if(valid_area > cell_area){
            valid_area -= cell_area;
        }else{
            cell->set_die(_pChip->get_die(1));
            ++count_move;
        }
    }
    if(count_move > 0) cout << BLUE << "[Partition]" << RESET << " - " << count_move << " cells moved from die1->die0.\n";
}
void Placer_C::mincut_k_partition(){
    int row_sum = _pChip->get_die(0)->get_row_num()+_pChip->get_die(1)->get_row_num();
    int k_part = min(row_sum, 30);
    //int slice = round(((double)_pChip->get_die(1)->get_row_num()*_pChip->get_die(1)->get_max_util()) / ((double)_pChip->get_die(0)->get_row_num()*_pChip->get_die(0)->get_max_util()) * k_part) ;
    // cutline for layer assignment
    double width_avg0 = 0;
    double width_avg1 = 0;
    for(Cell_C* cell : _vCell){
        width_avg0 += (double)cell->get_width(0)/ _vCell.size();
        width_avg1 += (double)cell->get_width(1)/ _vCell.size();
    }
    int slice = round((((width_avg1*_pChip->get_die(1)->get_row_height()) / (width_avg1*_pChip->get_die(1)->get_row_height() + width_avg0*_pChip->get_die(0)->get_row_height())) * (_pChip->get_die(0)->get_max_util() / _pChip->get_die(1)->get_max_util())) * k_part);
    //int slice = round(((double)_pChip->get_die(1)->get_row_num()) / ((double)_pChip->get_die(0)->get_row_num()) * k_part) ;
    HGR hgr(_RUNDIR, "circuit");
    for(Net_C* net : _vNet){
        hgr.add_net(net->get_name());
        for(int i=0;i<net->get_pin_num();++i){
            hgr.add_node(net->get_name(), net->get_pin(i)->get_cell()->get_name());
        }
    }
    hgr.write_hgr();
    run_hmetis(k_part, 0.6, "circuit");
    hgr.read_part_result(k_part);
    //cout << "hmetis partition result: " << max(hgr.get_part_size(0),hgr.get_part_size(1)) << " : " << min(hgr.get_part_size(0),hgr.get_part_size(1)) << "\n";
    for(Cell_C* cell : _vCell){
        int part = (hgr.get_part_size(0) >= hgr.get_part_size(1))? 
            hgr.get_part_result(cell->get_name()) : (hgr.get_part_result(cell->get_name())+1)%2;
        if(part < slice){
            cell->set_die(_pChip->get_die(0));
        }
        else{
            cell->set_die(_pChip->get_die(1));
        }
    }

    //cout << "Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell)num = " << _pChip->get_die(1)->get_cells().size() << "\n";
    if(_pChip->get_die(0)->get_cells().size() == 0 || _pChip->get_die(1)->get_cells().size() == 0){
        mincut_partition();
    }else{
        // move cells for matching die's max_utilization
        long long valid_area = (long long)_pChip->get_width() * (long long)_pChip->get_height() * _pChip->get_die(0)->get_max_util();
        Die_C* die = _pChip->get_die(0);
        vector<Cell_C*>& v_cell = die->get_cells();
        vector<int> v_cellId;
        for(Cell_C* cell : v_cell)
            v_cellId.emplace_back(cell->get_id());
        int count_move = 0;
        for(int i=0;i<v_cellId.size();++i){
            Cell_C* cell = _vCell[v_cellId[i]];
            long long cell_area = (long long)(cell->get_width()*cell->get_height());
            if(valid_area > cell_area){
                valid_area -= cell_area;
            }else{
                cell->set_die(_pChip->get_die(1));
                ++count_move;
            }
        }
        if(count_move > 0) cout << BLUE << "[Partition]" << RESET << " - " << count_move << " cells moved from die1->die0.\n";
    }
}
void Placer_C::bin_based_partition_new() {

    double cutline = 0.5;
    double width_avg0 = 0;
    double width_avg1 = 0;
    for(Cell_C* cell : _vCell){ // Todo v_cell ?= _vCell
        width_avg0 += (double)cell->get_width(_pChip->get_die(0)->get_techId())/ _vCell.size();
        width_avg1 += (double)cell->get_width(_pChip->get_die(1)->get_techId())/ _vCell.size();
    }
    cutline = ((width_avg1*_pChip->get_die(1)->get_row_height()) / (width_avg1*_pChip->get_die(1)->get_row_height() + width_avg0*_pChip->get_die(0)->get_row_height())) * (_pChip->get_die(0)->get_max_util() / _pChip->get_die(1)->get_max_util());
    cout << "cutline = " << cutline << "\n";

    // int bins_num = _vCell.size() * 1.1;
    
    int bins_per_row = 1;
    int bins_per_col = 1;
    if (_paramHdl.get_case_name() == "case3") {
        bins_per_row = 5;
        bins_per_col = 5;
    } else if (_paramHdl.get_case_name() == "case4") {
        bins_per_row = 11;
        bins_per_col = 11;
    } 
    cout << _paramHdl.get_case_name() << ": " << bins_per_row <<"\n";
    int bin_width = _pChip->get_die(0)->get_width() / bins_per_row;
    int bin_height = _pChip->get_die(0)->get_height() / bins_per_col;
    vector <vector <vector <Cell_C*>>> bins(bins_per_row, vector< vector <Cell_C*>> (bins_per_col, vector <Cell_C*> ())); 
    // vector<int> cell_num_in_bin;
    for (Cell_C* cell : _vCell) {
        int row_ind = floor(cell->get_posX() / bin_width);
        int col_ind = floor(cell->get_posY() / bin_height);
        bins[row_ind][col_ind].emplace_back(cell);
    }

   
    double used_area[2] = {0.0, 0.0};
    double maxArea[2];
    for (int i=0; i<bins_per_row; ++i) {
        for (int j=0; j<bins_per_col; ++j) {
            Partitioner* partitioner = new Partitioner();
            maxArea[0] = (double) _pChip->get_die(0)->get_width() * (double) _pChip->get_die(0)->get_height() * _pChip->get_die(0)->get_max_util() - used_area[0];
            maxArea[1] = (double) _pChip->get_die(1)->get_width() * (double) _pChip->get_die(1)->get_height() * _pChip->get_die(1)->get_max_util() - used_area[1];
            cout << "place " << maxArea[0] << "," << maxArea[1] << "\n";
            // cout << "place " << used_area[0] << "," << used_area[1] << "\n";
            partitioner->parseInput(_vCell, _pChip, bins[i][j], maxArea, cutline);
            partitioner->partition();
            partitioner->printSummary();
            vector<vector<int> >& cellPart = partitioner->get_part_result();

            bool inv = (cellPart[0].size() >= cellPart[1].size()) ? false : true;
            for (int k=0; k<2; ++k){
                for(int cellId : cellPart[k]){ 
                    Cell_C* cell = _vCell[cellId];
                    cell->set_die(_pChip->get_die(k));
                    used_area[k] += cell->get_width() * cell->get_height();
                }
            }
        }
    }
    
    // Partitioner* partitioner = new Partitioner();
    // partitioner->parseInput(_vCell, _pChip);
    // partitioner->partition();
    // partitioner->printSummary();
    // vector<vector<int> >& cellPart = partitioner->get_part_result();

    // for (int i=0; i<2; ++i){
    //     for(int cellId : cellPart[i]){ 
    //         _vCell[cellId]->set_die(_pChip->get_die(i));
    //     }
    // }
    
}
void Placer_C::bin_based_partition_real() {

    int bins_per_row_top = 45;
    int bins_per_col_top = 45;
    int bins_per_row_bot = 30;
    int bins_per_col_bot = 30;
    int bin_num_top = bins_per_row_top * bins_per_col_top;
    int bin_num_bot = bins_per_row_bot * bins_per_col_bot;
    int total_bin_num = bin_num_top + bin_num_bot;

    HGR hgr(_RUNDIR, "circuit");
    for(Net_C* net : _vNet){
        hgr.add_net(net->get_name());
        for(int i=0;i<net->get_pin_num();++i){
            hgr.add_node(net->get_name(), net->get_pin(i)->get_cell()->get_name());
        }
    }
    hgr.write_hgr();
    run_hmetis(total_bin_num, 0.5, "circuit");
    hgr.read_part_result(total_bin_num);

    vector<int> current_bins_order, best_bins_order;
    vector<double> parts_center_xy;
    int part_width_top = _pChip->get_die(0)->get_width() / bins_per_row_top;
    int part_height_top = _pChip->get_die(0)->get_height() / bins_per_col_top;
    int part_width_bot = _pChip->get_die(1)->get_width() / bins_per_row_bot;
    int part_height_bot = _pChip->get_die(1)->get_height() / bins_per_col_bot;

    for (int i=0; i<total_bin_num; i++) {    
        current_bins_order.push_back(i);
        if (i<bin_num_top) {
            parts_center_xy.push_back((i % bins_per_row_top + 0.5) * part_width_top); // x
            parts_center_xy.push_back((floor(i / bins_per_row_top) + 0.5) * part_height_top); // y
        } else {
            parts_center_xy.push_back(((i - bin_num_top) % bins_per_row_bot + 0.5) * part_width_bot); // x
            parts_center_xy.push_back((floor((i - bin_num_top) / bins_per_row_bot) + 0.5) * part_height_bot); // y
        }
    }
    random_shuffle(current_bins_order.begin(), current_bins_order.end());
    best_bins_order = current_bins_order;
    
    vector<Bin_C*> _vBin(total_bin_num);
    for (int i=0; i<total_bin_num; i++) {   
        int bin_id = current_bins_order[i];
        Bin_C* bin = new Bin_C(bin_id);
        _vBin[bin_id] = bin;
        bin->set_part(i);
        int die = (i >= bin_num_top) ? 1 : 0; 
        bin->set_die(die);
        bin->set_center(Pos(parts_center_xy[2 * (i % bin_num_top)], parts_center_xy[2 * (i % bin_num_top) + 1]));
        vector<int>& cellIdList = hgr.get_nodes_by_part(bin_id);
        bin->set_cellIdList(cellIdList);
    }
    for (Net_C* net : _vNet) {
        string bnet_id(total_bin_num, '0');
        vector<int> connected_bin_id;
        for (int i=0; i<net->get_pin_num(); i++) {
            int bin_id = hgr.get_part_result(net->get_pin(i)->get_cell()->get_name());
            bnet_id[bin_id] = '1'; // Todo: [debug] bin_id = -1 ????
            connected_bin_id.push_back(bin_id);
        }
        map<string, BNet_C*> bnet_set;
        bool not_exist = (bnet_set.find(bnet_id) == bnet_set.end()) ? 1 : 0;
        if (not_exist) {
            BNet_C* bnet = new BNet_C(bnet_id);
            bnet->set_weight(1);
            bnet_set[bnet_id] = bnet;
            for (int bin_id : connected_bin_id) {
                _vBin[bin_id]->add_bnet(bnet);
                bnet->add_bin(_vBin[bin_id]);
            }
            _vBNet.push_back(bnet);
        } else {
            BNet_C* bnet = bnet_set[bnet_id];
            bnet->incr_weight();
        }
    }

    int current_hpwl = cal_HPWL_binbased();
    int best_hpwl = current_hpwl;

    // SA
    int iter_limit = 1000;
    double T = INT_MAX, decrease_rate = 0.995;
    int uphill = 0;
    for (int i=0; i<iter_limit; i++) {
        if (i%200 == 0) {
            cout << "[" << i << "] T = " << T 
                 << ", cur_Hpwl = "      << current_hpwl 
                 << ", best_Hpwl = "     << best_hpwl << "\n";
        }
        
        T = T * decrease_rate;
        int oper = rand() % 2;
        int ind1, ind2;
        while (true) {
            ind1 = rand() % total_bin_num;
            ind2 = rand() % total_bin_num;
            if (ind1 != ind2) {
                break;
            }
        }
        
        int bin1_id = current_bins_order[ind1];
        int bin2_id = current_bins_order[ind2];
        Bin_C* bin1 = _vBin[bin1_id];
        Bin_C* bin2 = _vBin[bin2_id];
        int die_temp = bin1->get_die(); 
        Pos center_temp = bin1->get_center();
        bin1->set_die(bin2->get_die());
        bin2->set_die(die_temp);
        bin1->set_center(bin2->get_center());
        bin2->set_center(center_temp);

        int new_hpwl = cal_HPWL_binbased();
        if (new_hpwl <= current_hpwl) {
            current_hpwl = new_hpwl;
            current_bins_order[ind1] = bin2_id;
            current_bins_order[ind2] = bin1_id;
            bin1->set_part(ind2);
            bin2->set_part(ind1);
            if (new_hpwl <= best_hpwl) {
                best_hpwl = new_hpwl;
                best_bins_order = current_bins_order;
            }
        } else {
            double ran = rand() % 10 / 10;
            int delta = new_hpwl - current_hpwl;
            double accept_rate = exp(-1 * delta / T);
            if (ran < accept_rate) {
                current_hpwl = new_hpwl;
                current_bins_order[ind1] = bin2_id;
                current_bins_order[ind2] = bin1_id;
                bin1->set_part(ind2);
                bin2->set_part(ind1);
                uphill += 1;
            } else {
                int die_temp = bin1->get_die(); 
                Pos center_temp = bin1->get_center();
                bin1->set_die(bin2->get_die());
                bin2->set_die(die_temp);
                bin1->set_center(bin2->get_center());
                bin2->set_center(center_temp);
            }
        }

        // cout << "current bins order: ";
        // for (int i=0; i<total_bin_num; ++i) {
        //     cout << current_bins_order[i] << ",";
        // }
        // cout << "\n";

        

        if (T < 0.0000000000000000000000000001) {
            break;
        }

    }

    for (int ind=0; ind<total_bin_num; ind++) {
        int bin_id = best_bins_order[ind];
        vector<int>& cell_id = hgr.get_nodes_by_part(bin_id);
        int die = (ind >= bin_num_top) ? 1 : 0; 
        for (int j=0; j<cell_id.size(); j++) {
            Cell_C* cell = _vCell[cell_id[j]];
            cell->set_die(_pChip->get_die(die));
            cell->set_xy(Pos(parts_center_xy[2 * (ind % bin_num_top)], parts_center_xy[2 * (ind % bin_num_top) + 1]));
        }
       
    }

    int real_hpwl = cal_HPWL();
    cout << "real_hpwl = " << real_hpwl << "\n";
    
    // cout << "best bins order: ";
    // for (int i=0; i<bin_num; ++i) {
    //     cout << best_bins_order[i] << ",";
    // }
    // cout << "\n";

    cout << BLUE << "[Bin-based Partition]" << RESET << " - " << "HPWL = " << real_hpwl << "\n";


}
void Placer_C::bin_based_partition(int bin_num){

    HGR hgr(_RUNDIR, "circuit");
    for(Net_C* net : _vNet){
        hgr.add_net(net->get_name());
        for(int i=0;i<net->get_pin_num();++i){
            hgr.add_node(net->get_name(), net->get_pin(i)->get_cell()->get_name());
        }
    }
    hgr.write_hgr();
    run_hmetis(bin_num, 0.5, "circuit");
    hgr.read_part_result(bin_num);
    //cout << "hmetis partition result: " << max(hgr.get_part_size(0),hgr.get_part_size(1)) << " : " << min(hgr.get_part_size(0),hgr.get_part_size(1)) << "\n";
    
    vector<int> current_bins_order, best_bins_order;
    vector<double> bins_center_xy;
    int bins_per_row = sqrt(bin_num / 2); 

    //cout << "~~~~~~~~~" << _pChip->get_die(0)->get_width() << "," << _pChip->get_die(0)->get_height() <<"\n";
    for (int i=0; i<bin_num; i++) {    
        current_bins_order.push_back(i);
        if (i<bin_num/2) {
            bins_center_xy.push_back((i % bins_per_row + 0.5) * _pChip->get_die(0)->get_width() / bins_per_row); // x
            bins_center_xy.push_back((floor(i % (bin_num / 2) / bins_per_row) + 0.5) * _pChip->get_die(0)->get_height() / bins_per_row); // y
            //cout << (i % bins_per_row + 0.5) * _pChip->get_die(0)->get_width() / bins_per_row<< " , " << (floor(i % (bin_num / 2) / bins_per_row) + 0.5) * _pChip->get_die(0)->get_height() / bins_per_row <<"\n";
        }
    }

    random_shuffle(current_bins_order.begin(), current_bins_order.end());
    best_bins_order = current_bins_order;

    for(Cell_C* cell : _vCell){
        int part = hgr.get_part_result(cell->get_name());
        int bin = find(current_bins_order.begin(), current_bins_order.end(), part) - current_bins_order.begin();
        int die = 0;//(bin >= bin_num / 2) ? 1 : 0; 
        cell->set_die(_pChip->get_die(die));
        cell->set_xy(Pos(bins_center_xy[2 * (bin % (bin_num / 2))], bins_center_xy[2 * (bin % (bin_num / 2)) + 1]));
    }
    int current_hpwl = cal_HPWL();
    int best_hpwl = current_hpwl;

    // cout << "current bins order: ";
    // for (int i=0; i<bin_num; ++i) {
    //     cout << current_bins_order[i] << ",";
    // }
    //cout << "\n";

    // SA
    int iter_limit = 200;
    double T = 10000000000, decrease_rate = 0.99;
    int uphill = 0;
    for (int i=0; i<iter_limit; i++) {
        if (i%20 == 0) {
            cout << "[" << i << "] T = " << T 
                 << ", cur_Hpwl = "      << current_hpwl 
                 << ", best_Hpwl = "     << best_hpwl << "\n";
        }
        
        T = T * decrease_rate;
        int oper = rand() % 2;
        int ind1, ind2;
        while (true) {
            ind1 = rand() % bin_num;
            ind2 = rand() % bin_num;
            if (ind1 != ind2) {
                break;
            }
        }
        if (i < iter_limit / 2 && oper == 1) {
            while (true) {
                ind1 = rand() % bin_num;
                ind2 = rand() % bin_num;
                if (ind1 != ind2) {
                    break;
                }
            }

        } else {
            
            ind1 = rand() % bin_num;
            int die = (ind1 < bin_num / 2) ? 0 : 1; 
            int col_ind = ind1 % bins_per_row;
            int row_ind = floor(ind1 % (bin_num / 2) / bins_per_row);
            // cout << ind1 <<"(" << die << "," << row_ind << "," << col_ind << ")\n";
            if (row_ind == 0) {
                if (col_ind == 0) { // top left
                    int dir = rand() % 3;
                    if (dir == 0) {ind2 = (die == 0) ? (ind1 + bin_num / 2) : (ind1 - bin_num / 2);} // change die
                    else if (dir == 1) {ind2 = ind1 + 1;} // right
                    else {ind2 = ind1 + bins_per_row;} // down
                } else if (col_ind == bins_per_row - 1) { // top right
                    int dir = rand() % 3;
                    if (dir == 0) {ind2 = (die == 0) ? (ind1 + bin_num / 2) : (ind1 - bin_num / 2);} // change die
                    else if (dir == 1) {ind2 = ind1 - 1;} // left
                    else {ind2 = ind1 + bins_per_row;} // down
                } else {
                    int dir = rand() % 4;
                    if (dir == 0) {ind2 = (die == 0) ? (ind1 + bin_num / 2) : (ind1 - bin_num / 2);} // change die
                    else if (dir == 1) {ind2 = ind1 + 1;} // right
                    else if (dir == 2) {ind2 = ind1 - 1;} // left
                    else {ind2 = ind1 + bins_per_row;} // down
                }
            } else if(row_ind == bins_per_row -1) {
                if (col_ind == 0) { // buttom left
                    int dir = rand() % 3;
                    if (dir == 0) {ind2 = (die == 0) ? (ind1 + bin_num / 2) : (ind1 - bin_num / 2);} // change die
                    else if (dir == 1) {ind2 = ind1 + 1;} // right
                    else {ind2 = ind1 - bins_per_row;} // up
                } else if (col_ind == bins_per_row - 1) { // buttom right
                    int dir = rand() % 3;
                    if (dir == 0) {ind2 = (die == 0) ? (ind1 + bin_num / 2) : (ind1 - bin_num / 2);} // change die
                    else if (dir == 1) {ind2 = ind1 - 1;} // left
                    else {ind2 = ind1 - bins_per_row;} // up
                } else {
                    int dir = rand() % 4;
                    if (dir == 0) {ind2 = (die == 0) ? (ind1 + bin_num / 2) : (ind1 - bin_num / 2);} // change die
                    else if (dir == 1) {ind2 = ind1 + 1;} // right
                    else if (dir == 2) {ind2 = ind1 - 1;} // left
                    else {ind2 = ind1 - bins_per_row;} // up
                }
            } else if (col_ind == 0) {
                int dir = rand() % 4;
                if (dir == 0) {ind2 = (die == 0) ? (ind1 + bin_num / 2) : (ind1 - bin_num / 2);} // change die
                else if (dir == 1) {ind2 = ind1 + 1;} // right
                else if (dir == 2) {ind2 = ind1 + bins_per_row;} // down
                else {ind2 = ind1 - bins_per_row;} // up
            } else if (col_ind == bins_per_row -1) {
                int dir = rand() % 4;
                if (dir == 0) {ind2 = (die == 0) ? (ind1 + bin_num / 2) : (ind1 - bin_num / 2);} // change die
                else if (dir == 1) {ind2 = ind1 - 1;} // left
                else if (dir == 2) {ind2 = ind1 + bins_per_row;} // down
                else {ind2 = ind1 - bins_per_row;} // up
            } else {
                int dir = rand() % 5;
                if (dir == 0) {ind2 = (die == 0) ? (ind1 + bin_num / 2) : (ind1 - bin_num / 2);} // change die
                else if (dir == 1) {ind2 = ind1 - 1;} // left
                else if (dir == 2) {ind2 = ind1 + 1;} // right
                else if (dir == 3) {ind2 = ind1 + bins_per_row;} // down
                else {ind2 = ind1 - bins_per_row;} // up
            }
        }
        if(ind2 > 17) {
            cout << ind2 <<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
        }
        int part1 = current_bins_order[ind1]; // part: partition_id, ind: bin
        int part2 = current_bins_order[ind2];

        vector<int>& c1 = hgr.get_nodes_by_part(part1);
        vector<int>& c2 = hgr.get_nodes_by_part(part2);
        for (int j=0; j<c1.size(); j++) {
            int die = (ind2 >= bin_num / 2) ? 1 : 0; 
            Cell_C* cell = _vCell[c1[j]];
            // cell->set_die(_pChip->get_die(die));
            cell->set_xy(Pos(bins_center_xy[2 * (ind2 % (bin_num / 2))], bins_center_xy[2 * (ind2 % (bin_num / 2)) + 1]));
        }
        for (int j=0; j<c2.size(); j++) {
            int die = (ind1 >= bin_num / 2) ? 1 : 0; 
            Cell_C* cell = _vCell[c2[j]];
            // cell->set_die(_pChip->get_die(die));
            cell->set_xy(Pos(bins_center_xy[2 * (ind1 % (bin_num / 2))], bins_center_xy[2 * (ind1 % (bin_num / 2)) + 1]));
        }

        int new_hpwl = cal_HPWL();
        if (new_hpwl <= current_hpwl) {
            current_hpwl = new_hpwl;
            current_bins_order[ind1] = part2;
            current_bins_order[ind2] = part1;
            if (new_hpwl <= best_hpwl) {
                best_hpwl = new_hpwl;
                best_bins_order = current_bins_order;
            }
        } else {
            double ran = rand() % 10 / 10;
            int delta = new_hpwl - current_hpwl;
            double accept_rate = exp(-1 * delta / T);
            if (ran < accept_rate) {
                current_hpwl = new_hpwl;
                current_bins_order[ind1] = part2;
                current_bins_order[ind2] = part1;
                uphill += 1;
            } else {
                for (int j=0; j<c1.size(); j++) {
                    int die = (ind1 >= bin_num / 2) ? 1 : 0; 
                    Cell_C* cell = _vCell[c1[j]];
                    // cell->set_die(_pChip->get_die(die));
                    cell->set_xy(Pos(bins_center_xy[2 * (ind1 % (bin_num / 2))], bins_center_xy[2 * (ind1 % (bin_num / 2)) + 1]));
                }
                for (int j=0; j<c2.size(); j++) {
                    int die = (ind2 >= bin_num / 2) ? 1 : 0; 
                    Cell_C* cell = _vCell[c2[j]];
                    // cell->set_die(_pChip->get_die(die));
                    cell->set_xy(Pos(bins_center_xy[2 * (ind2 % (bin_num / 2))], bins_center_xy[2 * (ind2 % (bin_num / 2)) + 1]));
                }

            }
        }

        // if (T < 0.1 || uphill > 50) {
        //     break;
        // }

    }

    for (int ind=0; ind<bin_num; ind++) {
        int part = best_bins_order[ind];
        vector<int>& cell_id = hgr.get_nodes_by_part(part);
        int die = (ind >= bin_num / 2) ? 1 : 0; 
        for (int j=0; j<cell_id.size(); j++) {
            Cell_C* cell = _vCell[cell_id[j]];
            cell->set_die(_pChip->get_die(die));
            cell->set_xy(Pos(bins_center_xy[2 * (ind % (bin_num / 2))], bins_center_xy[2 * (ind % (bin_num / 2)) + 1]));
        }
       
    }
    
    // cout << "best bins order: ";
    // for (int i=0; i<bin_num; ++i) {
    //     cout << best_bins_order[i] << ",";
    // }
    // cout << "\n";

    cout << BLUE << "[Bin-based Partition]" << RESET << " - " << "HPWL = " << best_hpwl << "\n";

}
void Placer_C::init_place_ball(){
    int ball_curX = _pChip->get_ball_spacing();
    int ball_curY = _pChip->get_ball_spacing();
    for(Net_C* net : _vNet){
        if(net->is_cross_net()){
            if(ball_curX + _pChip->get_ball_width()/2.0 <= _pChip->get_width() && ball_curY + _pChip->get_ball_height()/2.0 <= _pChip->get_height()){
                net->set_ball_xy(Pos(ball_curX, ball_curY));
                ball_curX += _pChip->get_ball_width() + _pChip->get_ball_spacing();
            } else if(ball_curY + 3*_pChip->get_ball_height()/2.0 <= _pChip->get_height()){
                ball_curX = 0;
                ball_curY += _pChip->get_ball_height() + _pChip->get_ball_spacing();
                net->set_ball_xy(Pos(ball_curX, ball_curY));
                ball_curX += _pChip->get_ball_width() + _pChip->get_ball_spacing();
            } else{
                cout << BLUE << "[Placer]" << RESET << " - " << YELLOW << "Warning! " << RESET << "No more space to place for tarminal of net \'" << net->get_name() << "\', the terminal is placed at (0,0).\n";
                net->set_ball_xy(Pos(0,0));
            }
        }
    }
}

bool Placer_C::read_pl_and_set_pos(string fileName){
    AUX aux;
    vector<AuxNode> v_auxNode;
    if(aux.read_pl(fileName, v_auxNode)){
        for(AuxNode node : v_auxNode){
            if(_mCell.find(node.name) == _mCell.end()) continue;
            Cell_C* cell = _mCell[node.name];
            cell->set_xy(Pos(node.x,node.y));
        }
    }
    else{
        cout << BLUE << "[Placer]" << RESET << " - " << RED << "Error. " << RESET << "NTUplacer failed.\n";
        return false;
    }
    return true;
}
bool Placer_C::read_pl_and_set_pos(string fileName, int dieId){
    AUX aux;
    vector<AuxNode> v_auxNode;
    if(aux.read_pl(fileName, v_auxNode)){
        for(AuxNode node : v_auxNode){
            if(_mCell.find(node.name) == _mCell.end()) continue;
            Cell_C* cell = _mCell[node.name];
            if(cell->get_dieId() == dieId){
                cell->set_xy(Pos(node.x,node.y));
                //cout << "Place: " << _mCell[node.name]->get_name() << " " << _mCell[node.name]->get_pos().pos3d2str() << "\n";
                // if(!cell->check_drc()){
                //     cout << BLUE << "[Placer]" << RESET << " - " << YELLOW << "Warning! " << RESET << "NTUplaced Cell \'" << cell->get_name() << "\' at " + cell->get_pos().pos3d2str() +" position not valid.\n";
                // }
            }
        }
    }
    else{
        cout << BLUE << "[Placer]" << RESET << " - " << RED << "Error. " << RESET << "NTUplacer failed.\n";
        return false;
    }
    return true;
}
bool Placer_C::read_pl_and_set_pos_for_ball(string fileName){
    int shrink = ceil(_pChip->get_ball_spacing()/2.0);
    int ball_w = _pChip->get_ball_width() + shrink*2;
    int ball_h = _pChip->get_ball_height() + shrink*2;
    AUX aux;
    vector<AuxNode> v_auxNode;
    if(aux.read_pl(fileName, v_auxNode)){
        for(AuxNode node : v_auxNode){
            if(_mNet.find(node.name) == _mNet.end()) continue;
            Net_C* net = _mNet[node.name];
            net->set_ball_xy(Pos(node.x+ball_w/2, node.y+ball_h/2));
            if(!(node.x >= 0 && node.x + ball_w <= _pChip->get_width() && node.y >= 0 && node.y + ball_h <= _pChip->get_height())){
                cout << BLUE << "[Placer]" << RESET << " - " << YELLOW << "Warning! " << RESET << "NTUplaced Terminal \'" << net->get_name() << "\' at " + net->get_ball_pos().pos2d2str() +" position not valid.\n";
            }
        }
    }
    else{
        cout << BLUE << "[Placer]" << RESET << " - " << RED << "Error. " << RESET << "NTUplacer failed.\n";
        return false;
    }
    return true;
}
void Placer_C::run_ntuplace3(string caseName){
    run_ntuplace3(caseName, "");
}
void Placer_C::run_ntuplace3(string caseName, string otherPara){
    cout << BLUE << "[Placer]" << RESET << " - Running ntuplace3 for \'" << caseName << "\'...\n";
    // ex: ./bin/ntuplace-r -aux ./run_tmp/die0/die0.aux -out ./run_tmp/die0 > ./run_tmp/die0-ntuplace.log
    string cmd = "./bin/ntuplace-r -aux " + _RUNDIR + caseName + "/" + caseName + ".aux -out " + _RUNDIR + caseName + " " + otherPara + " > " + _RUNDIR + caseName + "-ntuplace.log";
    system(cmd.c_str());
    cout << BLUE << "[Placer]" << RESET << " - Running ntuplace3 for \'" << caseName << "\'" << GREEN << " completed!\n" << RESET;
}
void Placer_C::run_replace(string caseName){
    cout << BLUE << "[Placer]" << RESET << " - Running replace for \'" << caseName << "\'...\n";
    string cmd = "cd " + _RUNDIR + "; ../../bin/RePlAce-static -pcofmax 1.2 -bmflag ibm -bmname " + caseName + " > " + caseName + "-replace.log" + "; cd ../..";
    system(cmd.c_str());
    cout << BLUE << "[Placer]" << RESET << " - Running replace for \'" << caseName << "\'" << GREEN << " completed!\n" << RESET;
}
void Placer_C::run_ntuplace4(string caseName){
    cout << BLUE << "[Placer]" << RESET << " - Running ntuplace4 for \'" << caseName << "\'...\n";
    // ex: ./bin/ntuplace-r -aux ./run_tmp/die0/die0.aux -out ./run_tmp/die0 > ./run_tmp/die0-ntuplace.log
    string cmd = "./bin/ntuplace4 -aux " + _RUNDIR + caseName + "/" + caseName + ".aux -out " + _RUNDIR + caseName + " -noCong -noCongLG -noPGAvoid > " + _RUNDIR + caseName + "-ntuplace.log";
    system(cmd.c_str());
    cout << BLUE << "[Placer]" << RESET << " - Running ntuplace4 for \'" << caseName << "\'" << GREEN << " completed!\n" << RESET;
}
void Placer_C::run_hmetis(int k, double ufactor, string caseName){
    cout << BLUE << "[Placer]" << RESET << " - Running hmetis for \'" << caseName << "\'...\n";
    string fileName = _RUNDIR + caseName + ".hgr";
    string cmd = "bin/hmetis -ufactor=" + to_string(ufactor) + " " + fileName + " " + to_string(k) + " > " + _RUNDIR + caseName + "-hmetis.log";
    system(cmd.c_str());
    cout << BLUE << "[Placer]" << RESET << " - Running hmetis for \'" << caseName << "\'" << GREEN << " completed!\n" << RESET;
}
void Placer_C::create_aux_form(AUX &aux, int dieId, string caseName){  // output in dir "./aux/<case-name>/"
    string aux_dir = _RUNDIR + caseName + "/";
    string cmd = "mkdir -p " + aux_dir;
    system(cmd.c_str());
    aux = AUX(aux_dir, caseName);
    // nodes
    vector<Cell_C*>& v_cells = _pChip->get_die(dieId)->get_cells();
    for(Cell_C* cell : v_cells){
        aux.add_node(cell->get_name(), cell->get_width(), cell->get_height(), cell->get_posX(), cell->get_posY(),0);
        for(int i=0;i<cell->get_pin_num();++i){
            Pin_C* pin = cell->get_pin(i);
            Net_C* net = pin->get_net();
            if(net != nullptr){
                char IO = 'I';
                if(!aux.check_net_exist(net->get_name())){
                    aux.add_net(net->get_name());
                    IO='O';
                }
                Pos pin_offset = cell->get_master_cell()->get_pin_offset(_pChip->get_die(dieId)->get_techId() ,pin->get_id());
                pin_offset.x -= cell->get_width()/2; pin_offset.y -= cell->get_height()/2;
                aux.add_pin(net->get_name(), pin->get_cell()->get_name(), IO, pin_offset.x, pin_offset.y);
            }
        }
    }
    // rows
    aux.set_default_rows(_pChip->get_width(), _pChip->get_die(dieId)->get_row_height(), _pChip->get_die(dieId)->get_row_num());
}
void Placer_C::create_aux_form_for_ball(AUX &aux, string caseName){  // output in dir "./aux/<case-name>/"
    string aux_dir = _RUNDIR + caseName + "/";
    string cmd = "mkdir -p " + aux_dir;
    system(cmd.c_str());
    aux = AUX(aux_dir, caseName);
    // transform balls to nodes
    int ball_w = _pChip->get_ball_width() + _pChip->get_ball_spacing();
    int ball_h = _pChip->get_ball_height() + _pChip->get_ball_spacing();
    vector<Net_C*>& v_nets = _pDesign->get_nets();
    for(Net_C* net : v_nets){
        if(!net->is_cross_net()) continue;
        int ball_x = net->get_ball_pos().x - _pChip->get_ball_width()/2 - int(_pChip->get_ball_spacing()/2.0+0.5);
        int ball_y = net->get_ball_pos().y - _pChip->get_ball_height()/2 - int(_pChip->get_ball_spacing()/2.0+0.5);
        aux.add_node(net->get_name(), ball_w, ball_h, ball_x, ball_y, 0);
        //Pos pin_offset = Pos(ball_w/2, ball_h/2);
        aux.add_pin(net->get_name(), net->get_name(), 'O', 0, 0);
    }
    // rows
    int shrink = ceil(_pChip->get_ball_spacing()/2.0);
    int numRow = (_pChip->get_height()-shrink*2) / ball_h;
    for(int i=0;i<numRow;++i){
        aux.add_row(shrink+i*ball_h, ball_h, 1, 1, 0, 0, shrink, _pChip->get_width()-shrink*2);
    }
}
void Placer_C::create_aux_form_replace(AUX &aux, int dieId, string caseName){  // output in dir "./aux/<case-name>/"
    string aux_dir = _RUNDIR + "/IBM/" + caseName + "/";
    string cmd = "mkdir -p " + aux_dir;
    system(cmd.c_str());
    aux = AUX(aux_dir, caseName);
    // nodes
    vector<Cell_C*>& v_cells = _pChip->get_die(dieId)->get_cells();
    for(Cell_C* cell : v_cells){
        aux.add_node(cell->get_name(), cell->get_width(), cell->get_height(), cell->get_posX(), cell->get_posY(),0);
        for(int i=0;i<cell->get_pin_num();++i){
            Pin_C* pin = cell->get_pin(i);
            Net_C* net = pin->get_net();
            if(net != nullptr){
                char IO = 'I';
                if(!aux.check_net_exist(net->get_name())){
                    aux.add_net(net->get_name());
                    IO='O';
                }
                Pos pin_offset = cell->get_master_cell()->get_pin_offset(_pChip->get_die(dieId)->get_techId() ,pin->get_id());
                pin_offset.x -= cell->get_width()/2; pin_offset.y -= cell->get_height()/2;
                aux.add_pin(net->get_name(), pin->get_cell()->get_name(), IO, pin_offset.x, pin_offset.y);
            }
        }
    }
    // rows
    aux.set_default_rows(_pChip->get_width(), _pChip->get_die(dieId)->get_row_height(), _pChip->get_die(dieId)->get_row_num());
}
void Placer_C::create_aux_form_for_ball_replace(AUX &aux, string caseName){  // output in dir "./aux/<case-name>/"
    string aux_dir = _RUNDIR + "/IBM/" + caseName + "/";
    string cmd = "mkdir -p " + aux_dir;
    system(cmd.c_str());
    aux = AUX(aux_dir, caseName);
    // transform balls to nodes
    int ball_w = _pChip->get_ball_width() + _pChip->get_ball_spacing();
    int ball_h = _pChip->get_ball_height() + _pChip->get_ball_spacing();
    vector<Net_C*>& v_nets = _pDesign->get_nets();
    for(Net_C* net : v_nets){
        if(!net->is_cross_net()) continue;
        int ball_x = net->get_ball_pos().x - _pChip->get_ball_width()/2 - int(_pChip->get_ball_spacing()/2.0+0.5);
        int ball_y = net->get_ball_pos().y - _pChip->get_ball_height()/2 - int(_pChip->get_ball_spacing()/2.0+0.5);
        aux.add_node(net->get_name(), ball_w, ball_h, ball_x, ball_y, 0);
        //Pos pin_offset = Pos(ball_w/2, ball_h/2);
        aux.add_pin(net->get_name(), net->get_name(), 'O', 0, 0);
    }
    // rows
    int shrink = ceil(_pChip->get_ball_spacing()/2.0);
    int numRow = (_pChip->get_height()-shrink*2) / ball_h;
    for(int i=0;i<numRow;++i) {
        aux.add_row(shrink+i*ball_h, ball_h, 1, 1, 0, 0, shrink, _pChip->get_width()-shrink*2);
    }
}
void Placer_C::add_project_pin(AUX &aux, int dieId){
    // project pins of nodes in die[dieId]
    vector<Cell_C*>& v_cells = _pChip->get_die(dieId)->get_cells();
    for(Cell_C* cell : v_cells){
        bool cellAdded = false;
        for(int i=0;i<cell->get_pin_num();++i){
            Pin_C* pin = cell->get_pin(i);
            Net_C* net = pin->get_net();
            if(net != nullptr){
                if(aux.check_net_exist(net->get_name())){
                    if(!cellAdded){
                        //aux.add_node(cell->get_name(), cell->get_width(), cell->get_height(), cell->get_posX(), cell->get_posY(),2);
                        aux.add_node(cell->get_name(), 0, 0, cell->get_posX(), cell->get_posY(),2);
                        cellAdded = true;
                    }
                    Pos pin_offset = cell->get_master_cell()->get_pin_offset(_pChip->get_die(dieId)->get_techId() ,pin->get_id());
                    pin_offset.x -= cell->get_width()/2; pin_offset.y -= cell->get_height()/2;
                    aux.add_pin(net->get_name(), pin->get_cell()->get_name(), 'I', pin_offset.x, pin_offset.y);
                }
            }
        }
    }
}
void Placer_C::add_project_ball(AUX &aux){
    int ball_w = _pChip->get_ball_width();
    int ball_h = _pChip->get_ball_height();
    vector<Net_C*>& v_nets = _pDesign->get_nets();
    for(Net_C* net : v_nets){
        if(!net->is_cross_net()) continue;
        int ball_x = net->get_ball_pos().x - _pChip->get_ball_width()/2;
        int ball_y = net->get_ball_pos().y - _pChip->get_ball_height()/2;
        aux.add_node(net->get_name(), ball_w, ball_h, ball_x, ball_y, 2);
        //Pos pin_offset = Pos(ball_w/2, ball_h/2);
        aux.add_pin(net->get_name(), net->get_name(), 'O', 0, 0);
    }
}

int Placer_C::cal_ball_num(){
    int countTerminal = 0;
    for(int i=0;i<_pDesign->get_net_num();++i){
        Net_C* net = _pDesign->get_net(i);
        if(net->is_cross_net()){
            ++countTerminal;
        }
    }
    return countTerminal;
}
int Placer_C::cal_HPWL(){
    int total_hpwl = 0;
    // update the HPWL
    for(Net_C* net : _vNet){
        net->update_bbox();
        total_hpwl += net->get_HPWL(0) + net->get_HPWL(1);
    }
    return total_hpwl;
}
int Placer_C::cal_HPWL_binbased(){
    int total_hpwl = 0;
    // update the HPWL
    for(BNet_C* bnet : _vBNet){
        bnet->update_bbox();
        total_hpwl += bnet->get_HPWL(0) + bnet->get_HPWL(1);
    }
    return total_hpwl;
}
void Placer_C::init_run_dir(){
    string cmd_clean = "rm -rf " + _RUNDIR + "; mkdir -p " + _RUNDIR;
    system(cmd_clean.c_str());
}
void Placer_C::init_draw_dir(){
    string cmd_clean = "rm -rf " + _DRAWDIR + "; mkdir -p " + _DRAWDIR;
    system(cmd_clean.c_str());
}


int* getRandRGB(){
    int* color = new int(3);
    color[0] = rand()%250;
    color[1] = rand()%250;
    color[2] = rand()%250;
    return color;
}
void Placer_C::draw_layout_result(){ // output in dir "./draw/<case-name>.html"
    draw_layout_result("");
}
void Placer_C::draw_layout_result(string tag){ // output in dir "./draw/<case-name>.html"
    if(tag!="" && tag[0]!='-') tag = "-" + tag;
    string outFile = _DRAWDIR + _paramHdl.get_case_name() + tag + ".html";
    ofstream outfile( outFile.c_str() , ios::out );

    Drawer_C* draw_svg = new Drawer_C(outFile);
    double scaling = 1500.0 / (_pChip->get_width()*2+_pChip->get_width()/10.0);
    draw_svg->setting(_pChip->get_width()*2+_pChip->get_width()/10.0,_pChip->get_height(),scaling,0,50); // outline_x, outline_y, scaling, offset_x, offset_y
    draw_svg->start_svg();
    vector<Pos> diePos;
    
    diePos.push_back(Pos(0,0));
    diePos.push_back(Pos(diePos[0].x+_pChip->get_width()+_pChip->get_width()/10.0,0));
    for(int dieId=0;dieId<2;++dieId){
        // Draw Die0
        draw_svg->drawRect("Die"+to_string(dieId), drawBox(drawPos(diePos[dieId].x,diePos[dieId].y),drawPos(diePos[dieId].x+_pChip->get_width(),diePos[dieId].y+_pChip->get_height())), "white");
        // draw rows
        Die_C* die = _pChip->get_die(dieId);
        for(int i=0;i<die->get_row_num();++i){
            die->get_row_height();
            draw_svg->drawRect("Die"+to_string(dieId)+"_Row"+to_string(i), drawBox(drawPos(diePos[dieId].x,diePos[dieId].y+i*die->get_row_height()),drawPos(diePos[dieId].x+_pChip->get_width(),diePos[dieId].y+i*die->get_row_height()+die->get_row_height())), "gainsboro");
        }
    }

    // set net color
    vector<int*> v_netColor(_pDesign->get_net_num());
    for(int i=0;i<_pDesign->get_net_num();++i){
        v_netColor[i] = getRandRGB();
    }
    // Draw Cells
    double cellFontSize = min(_pChip->get_die(0)->get_row_height(),_pChip->get_die(1)->get_row_height())/4.0;
    double pinSize = min(_pChip->get_die(0)->get_row_height(),_pChip->get_die(1)->get_row_height())/8.0;
    for(int i=0;i<_pDesign->get_cell_num();++i){
        Cell_C* cell = _pDesign->get_cell(i);
        //cout << "Draw: " << cell->get_name() << " " << cell->get_pos().pos3d2str() << "\n";
        int dieX = diePos[cell->get_dieId()].x;
        int dieY = diePos[cell->get_dieId()].y;
        map<string,string> m_para{{"type","cell"}};
        draw_svg->drawRect(cell->get_name(), drawBox(drawPos(dieX+cell->get_posX(),dieY+cell->get_posY()),drawPos(dieX+cell->get_posX()+cell->get_width(),dieY+cell->get_posY()+cell->get_height())), "yellow", 0.1, m_para);
        // cell name lable
        draw_svg->drawText(cell->get_name()+"_label", drawPos(dieX+cell->get_posX()+cell->get_width()/2.0, dieY+cell->get_posY()+cell->get_height()/2.0), cellFontSize, cell->get_name());
        // draw pins
        for(int j=0;j<cell->get_pin_num();++j){
            Pin_C* pin = cell->get_pin(j);
            if(pin->get_net() != nullptr){
                map<string,string> m_para{{"type","pin"},{"cell",pin->get_cell()->get_name()}, {"net",pin->get_net()->get_name()}};
                draw_svg->drawRect(cell->get_name()+"_"+pin->get_name(), drawBox(drawPos(dieX+pin->get_x()-pinSize/2,dieY+pin->get_y()-pinSize/2),drawPos(dieX+pin->get_x()+pinSize/2,dieY+pin->get_y()+pinSize/2)), v_netColor[pin->get_net()->get_id()], 0.6, m_para);
            }
            else{
                map<string,string> m_para{{"type","pin"},{"cell",pin->get_cell()->get_name()}};
                draw_svg->drawRect(cell->get_name()+"_"+pin->get_name(), drawBox(drawPos(dieX+pin->get_x()-pinSize/2,dieY+pin->get_y()-pinSize/2),drawPos(dieX+pin->get_x()+pinSize/2,dieY+pin->get_y()+pinSize/2)), "black", 1, m_para);
            }
        }
    }
    // Draw Balls (Terminals)
    for(int i=0;i<_pDesign->get_net_num();++i){
        Net_C* net = _pDesign->get_net(i);
        if(net->is_cross_net()){
            for(int dieId=0;dieId<2;++dieId){
                int dieX = diePos[dieId].x;
                int dieY = diePos[dieId].y;
                map<string,string> m_para{{"net",net->get_name()}, {"type","Terminal"}};
                draw_svg->drawRect(net->get_name()+"_Terminal", drawBox(drawPos(dieX+net->get_ball_pos().x-_pChip->get_ball_width()/2.0,dieY+net->get_ball_pos().y-_pChip->get_ball_height()/2.0),drawPos(dieX+net->get_ball_pos().x+_pChip->get_ball_width()/2.0,dieY+net->get_ball_pos().y+_pChip->get_ball_height()/2.0)), v_netColor[net->get_id()], 0.6, m_para);
            }
        }
    }
    // Draw HPWL
    for(int i=0;i<_pDesign->get_net_num();++i){
        Net_C* net = _pDesign->get_net(i);
        for(int dieId=0;dieId<2;++dieId){
            int dieX = diePos[dieId].x;
            int dieY = diePos[dieId].y;
            map<string,string> m_para{{"net",net->get_name()}, {"type","BoundingBox"},{"HPWL",to_string(net->get_HPWL(dieId))}};
            
            double ll_x = net->get_ll(dieId).x, ll_y = net->get_ll(dieId).y, ur_x = net->get_ur(dieId).x, ur_y = net->get_ur(dieId).y;
            if(ll_x == ur_x && ll_x != 0) { ll_x -= pinSize/2; ur_x += pinSize/2; }
            if(ll_y == ur_y && ll_y != 0) { ll_y -= pinSize/2; ur_y += pinSize/2; }
            draw_svg->drawBBox(net->get_name()+"_HPWL"+to_string(dieId), drawBox(drawPos(dieX+ll_x,dieY+ll_y),drawPos(dieX+ur_x,dieY+ur_y)), v_netColor[net->get_id()], 0.5, 0.6, 0.5, m_para);
        }
    }
    // Output HPWL Result Text
    vector<long long int> vHPWL(2,0);
    long long int totalHPWL = 0;
    for(int i=0;i<_pDesign->get_net_num();++i){
        Net_C* net = _pDesign->get_net(i);
        vHPWL[0] += net->get_HPWL(0);
        vHPWL[1] += net->get_HPWL(1);
        totalHPWL += net->get_HPWL(0) + net->get_HPWL(1);
    }
    double fontSize = 20;
    draw_svg->drawText("Die0_HPWL_result", drawPos((diePos[0].x+_pChip->get_width()/3.0), diePos[0].y), fontSize, "Top_Die HPWL = " + to_string(vHPWL[0]), 0, -25);
    draw_svg->drawText("Die1_HPWL_result", drawPos((diePos[1].x+_pChip->get_width()/3.0), diePos[1].y), fontSize, "Bot_Die HPWL = " + to_string(vHPWL[1]), 0, -25);
    draw_svg->drawText("Total_HPWL_result", drawPos(diePos[0].x, diePos[0].y), fontSize, "Total HPWL = " + to_string(totalHPWL), 0, -50);

    draw_svg->end_svg();
    cout << BLUE << "[Placer]" << RESET << " - Visualize the layout in \'" << outFile << "\'.\n";
}

void plotBoxPLT( ofstream& stream, double x1, double y1, double x2, double y2 )
{
    stream << x1 << ", " << y1 << endl << x2 << ", " << y1 << endl
           << x2 << ", " << y2 << endl << x1 << ", " << y2 << endl
           << x1 << ", " << y1 << endl << endl;
}
void Placer_C::draw_layout_result_plt(bool show_hpwl){ // output in dir "./draw/<case-name>.plt"
    draw_layout_result_plt(show_hpwl, "");
}
void Placer_C::draw_layout_result_plt(bool show_hpwl, string tag){ // output in dir "./draw/<case-name>.plt"
    if(tag!= "" && tag[0] != '-') tag = "-" + tag;
    string outFile = _DRAWDIR + _paramHdl.get_case_name() + tag + ".plt";
    ofstream outfile( outFile.c_str() , ios::out );

    outfile << " " << endl;
    outfile << "set terminal png size 4000,3000" << endl;
    outfile << "set output " << "\"" << _DRAWDIR << _paramHdl.get_case_name() << tag << ".png\"" << endl;
    // outfile << "set multiplot layout 1, 2" << endl;
    outfile << "set size ratio 0.5" << endl;
    outfile << "set nokey" << endl << endl;

    // Output HPWL Result Text
    vector<long long int> vHPWL(2,0);
    long long int totalHPWL = 0;
    for(int i=0;i<_pDesign->get_net_num();++i){
        Net_C* net = _pDesign->get_net(i);
        vHPWL[0] += net->get_HPWL(0);
        vHPWL[1] += net->get_HPWL(1);
        totalHPWL += net->get_HPWL(0) + net->get_HPWL(1);
    }

    

    double bot_chip_offset = _pChip->get_width() + 5;

    outfile << "set title \"WL_{Total} = " << totalHPWL << "\"" << endl;
    outfile << "set label 1 \"WL_{Top} = " << vHPWL[0] << "\" at " << _pChip->get_width() * 0.1 << "," << _pChip->get_height() * -0.15 <<" left" << endl;
    outfile << "set label 2 \"WL_{Bot} = " << vHPWL[1] << "\" at " << _pChip->get_width() * 1.1 + 5 << "," << _pChip->get_height() * -0.15 <<" left" << endl << endl;

    // for(int i=0; i<cell_list_top.size(); i++){
    //     outfile << "set label " << i + 2 << " \"" << cell_list_top[i]->get_name() << "\" at " << cell_list_top[i]->get_posX() + cell_list_top[i]->get_width() / 2 << "," << cell_list_top[i]->get_posY() + cell_list_top[i]->get_height() / 2 << " center front" << endl;
    // }
    // outfile << "set xrange [0:" << _pChip->get_width() << "]" << endl;
    // outfile << "set yrange [0:" << _pChip->get_height() << "]" << endl;
    // outfile << "plot[:][:] '-' w l lt 3 lw 2, '-' with filledcurves closed fc \"grey90\" fs border lc \"red\", '-' with filledcurves closed fc \"yellow\" fs border lc \"black\", '-' w l lt 1" << endl << endl;
    if (show_hpwl) {
        outfile << "plot[:][:]  '-' w l lt 3 lw 2, '-' with filledcurves closed fc \"grey90\" fs border lc \"red\", '-' w l lt 1" << endl << endl;
    } else {
        outfile << "plot[:][:]  '-' w l lt 3 lw 2, '-' with filledcurves closed fc \"grey90\" fs border lc \"red\"" << endl << endl;
    }
    
    outfile << "# bounding box" << endl;
    plotBoxPLT( outfile, 0, 0, _pChip->get_width(), _pChip->get_height() ); // top chip
    plotBoxPLT( outfile, bot_chip_offset, 0, bot_chip_offset + _pChip->get_width(), _pChip->get_height() ); // bot chip
    outfile << "EOF" << endl;

    outfile << "# cells" << endl;
    for(int i=0;i<_pDesign->get_cell_num();++i){
        Cell_C* cell = _pDesign->get_cell(i);
        if (cell->get_dieId() == 0) {
            plotBoxPLT( outfile, cell->get_posX(), cell->get_posY(), cell->get_posX() + cell->get_width(), cell->get_posY() + cell->get_height() );
        } else {
            plotBoxPLT( outfile, bot_chip_offset + cell->get_posX(), cell->get_posY(), bot_chip_offset + cell->get_posX() + cell->get_width(), cell->get_posY() + cell->get_height() );
        }
    }
    outfile << "EOF" << endl;

    
    // Draw HPWL
    if (show_hpwl) {
        outfile << "# nets" << endl;
        int dieId;
        for(int i=0;i<_pDesign->get_net_num();++i){
            Net_C* net = _pDesign->get_net(i);
            dieId = 0;
            double ll_x = net->get_ll(dieId).x, ll_y = net->get_ll(dieId).y, ur_x = net->get_ur(dieId).x, ur_y = net->get_ur(dieId).y;
            plotBoxPLT( outfile, ll_x, ll_y, ur_x, ur_y);
            dieId = 1;
            ll_x = net->get_ll(dieId).x, ll_y = net->get_ll(dieId).y, ur_x = net->get_ur(dieId).x, ur_y = net->get_ur(dieId).y;
            plotBoxPLT( outfile, bot_chip_offset + ll_x, ll_y, bot_chip_offset + ur_x, ur_y);
        }
        outfile << "EOF" << endl;
    }
    
    

    // outfile << "pause -1 'Press any key to close.'" << endl;
    outfile.close();

    system(("gnuplot " + outFile).c_str());

    cout << BLUE << "[Placer]" << RESET << " - Visualize the plt layout in \'" << outFile << "\'.\n";
}



void Placer_C::clear(){
    _pChip = nullptr;
    _pDesign = nullptr;

    _vCell.clear();
    _vNet.clear();
}
