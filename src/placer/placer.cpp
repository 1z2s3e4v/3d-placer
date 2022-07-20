#include "placer.h"
//#include <bits/types/clock_t.h>
#include <ctime>
#include <cmath>

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
}

void Placer_C::run(){
    cout << BLUE << "[Placer]" << RESET << " - Start\n";
    init_run_dir();
    bool place_succ = false;
    while(!place_succ){
        // init place
        //init_place();
        //place_succ = order_place();
        //place_succ = random_d2dplace();
        //place_succ = ntu_d2dplace();
        //place_succ = pin3d_ntuplace();
        //place_succ = shrunk2d_ntuplace();
        place_succ = shrunk2d_replace();
        //place_succ = true3d_placement(); // our main function
        //place_succ = half3d_placement(); // our main function
        //place_succ = ntuplace3d();// (remember to replace dir 'ntuplace' to 'ntuplace3d_bak')
        //place_succ = coloquinte_place();
        //place_succ = shrunked_2d_ntuplace();

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
                    Pos pin_offset1 = cell->get_master_cell()->get_pin_offset(_pChip->get_die(1)->get_techId() ,pin->get_id());
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
    for(Cell_C* cell : _vCell)
        cell->set_die(_pChip->get_die(0));
    rand_place(0);
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
                    Pos pin_offset1 = cell->get_master_cell()->get_pin_offset(_pChip->get_die(1)->get_techId() ,pin->get_id());
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
    //ntu_d2d_global(isLegal, wl1);
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Global: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    cout << BLUE << "[Placer]" << RESET << " - Global: total pin2pin HPWL = " << wl1 << ".\n";

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
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: total HPWL = " << total_hpwl << ".\n";
    return true;
}

bool Placer_C::half3d_placement(){
    cout << BLUE << "[Placer]" << RESET << " - Start Half3d Placement Flow.\n";
    double part_time_start=0, total_part_time=0;
    ////////////////////////////////////////////////////////////////
    // Global Placement
    ////////////////////////////////////////////////////////////////
    bool isLegal = false; 
    double wl1 = 0; // gp-wire
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 1]" << RESET << ": Global Placement.\n";
    // cell spreading
    cell_spreading();
    // die-partition
    mincut_partition();
    rand_ball_place();
    cout << BLUE << "[Placer]" << RESET << " - Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << "\n";
    cout << BLUE << "[Placer]" << RESET << " - #Terminal = " << cal_ball_num() << "\n";
    // 3d analytical global placement
    global_place(isLegal, wl1); /////////////////////////////////////////////// main function
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Global: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    cout << BLUE << "[Placer]" << RESET << " - Global: total pin2pin HPWL = " << wl1 << ".\n";

    rand_ball_place();
    cout << BLUE << "[Placer]" << RESET << " - Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << "\n";
    cout << BLUE << "[Placer]" << RESET << " - #Terminal = " << cal_ball_num() << "\n";
    
    ////////////////////////////////////////////////////////////////
    // D2D Placement with Pin Projection
    ////////////////////////////////////////////////////////////////
    bool placer_succ = true;
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
    cout << BLUE << "[Placer]" << RESET << " - [3.3] total HPWL = " << total_hpwl << ".\n";
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
    cout << BLUE << "[Placer]" << RESET << " - [3.4] total HPWL = " << total_hpwl << ".\n";
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
    cout << BLUE << "[Placer]" << RESET << " - [3.5] total HPWL = " << total_hpwl << ".\n";
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
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: total HPWL = " << total_hpwl << ".\n";
    return true;
}

bool Placer_C::true3d_placement(){
    cout << BLUE << "[Placer]" << RESET << " - Start True3d Placement Flow.\n";
    double part_time_start=0, total_part_time=0;
    ////////////////////////////////////////////////////////////////
    // Global Placement
    ////////////////////////////////////////////////////////////////
    bool isLegal = false; 
    double wl1 = 0; // gp-wire
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 1]" << RESET << ": Global Placement.\n";
    // cell spreading
    cell_spreading();
    // die-partition
    // if(_pChip->get_die(0)->get_row_num()==_pChip->get_die(1)->get_row_num())
         mincut_partition();
    // else 
    //     mincut_k_partition();
    // for(int i=0;i<_vCell.size();++i){
    //     Cell_C* cell = _vCell[i];
    //     // if(i<_vCell.size()/2) cell->set_die(_pChip->get_die(0));
    //     // else cell->set_die(_pChip->get_die(1));
    //     cell->set_die(_pChip->get_die(rand()%2));
    // }
    // Init spreading and layer assignment
    // rand_place(0);
    // rand_place(1);
    rand_ball_place();

    cout << BLUE << "[Placer]" << RESET << " - Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << "\n";
    cout << BLUE << "[Placer]" << RESET << " - #Terminal = " << cal_ball_num() << "\n";
    // 3d analytical global placement
    global_place(isLegal, wl1); /////////////////////////////////////////////// main function
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Global: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    cout << BLUE << "[Placer]" << RESET << " - Global: total pin2pin HPWL = " << wl1 << ".\n";

    rand_ball_place();
    cout << BLUE << "[Placer]" << RESET << " - Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << "\n";
    cout << BLUE << "[Placer]" << RESET << " - #Terminal = " << cal_ball_num() << "\n";
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
    
    place_succ = pin3d_ntu_d2d_legal_detail(); ////////////////////////////////////////////////////// main function
    
    if(!place_succ) return false;
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    int total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: total HPWL = " << total_hpwl << ".\n";
    return true;
}
void Placer_C::global_place(bool& isLegal, double& totalHPWL){ // Analytical Global Placement
    // 3d
    param.b3d = true;
    param.nlayer = 2;
    param.bLayerPreAssign = true;
    param.dWeightTSV = 0.5;
    //param.step = 5;
    param.stepZ = 0.2;

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
    cout << BLUE << "[Placer]" << RESET << " - total HPWL = " << total_hpwl << ".\n";
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
    cout << BLUE << "[Placer]" << RESET << " - total HPWL = " << total_hpwl << ".\n";
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
    cout << BLUE << "[Placer]" << RESET << " - total HPWL = " << total_hpwl << ".\n";
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
    cout << BLUE << "[Placer]" << RESET << " - total HPWL = " << total_hpwl << ".\n";
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
    // param.bPlot = true;
    // param.bShow = true;
    // param.bLog = true;
    // param.bOutTopPL = true;
    // <<
    param.bRunInit = true;
    param.seed = 1;
    param.bUseLSE = false; // default
    param.bUseWAE = true; // default
    param.weightWire = 2.0;
	param.step = 0.2;
	param.stepAssigned = true;
    srand( param.seed );
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
    placedb.ReserveModuleMemory(v_cell.size());
    for(Cell_C* cell : v_cell){
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
    placedb.m_nModules = v_cell.size(); //fplan.m_nModules = nNodes + nTerminals;
    placedb.m_modules.resize( placedb.m_modules.size() );
    placedb.CreateModuleNameMap();
    // .nets
    int nPins = 0;
    for(Net_C* net : _vNet) 
        nPins += net->get_pin_num();
    placedb.ReserveNetMemory( _vNet.size() );
    placedb.ReservePinMemory( nPins );
    int nReadNets = 0;
    for(Net_C* net : _vNet){
        Net net_db;
        vector<Pin_C*> v_pin = net->get_pins();
        net_db.reserve( v_pin.size() );
        for(int i=0;i<net->get_pin_num();++i){
            int moduleId = placedb.GetModuleId( v_pin[i]->get_cell()->get_name() );
            Cell_C* cell = v_pin[i]->get_cell();
            Pos pin_offset0 = cell->get_master_cell()->get_pin_offset(_pChip->get_die(0)->get_techId() ,v_pin[i]->get_id());
            Pos pin_offset1 = cell->get_master_cell()->get_pin_offset(_pChip->get_die(1)->get_techId() ,v_pin[i]->get_id());
            Pos pin_offset = Pos(ceil((pin_offset0.x+pin_offset1.x)/2.0),ceil((pin_offset0.y+pin_offset1.y)/4.0));
            int pinId = placedb.AddPin( moduleId, pin_offset.x, pin_offset.y );
            net_db.push_back( pinId );
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
        placedb.AddNet( net_db );
        nReadNets++;
    }
    placedb.m_nPins = nPins;
	placedb.m_nNets = _vNet.size();
    placedb.m_pins.resize( placedb.m_pins.size() );
    placedb.m_nets.resize( placedb.m_nets.size() );
    // .pl
    for(Cell_C* cell : v_cell){
        int moduleId = placedb.GetModuleId( cell->get_name() );
        placedb.SetModuleLocation( moduleId, cell->get_posX(), cell->get_posY());
		placedb.SetModuleOrientation( moduleId, 0 ); // orientInt('N')=0
        if(param.bLayerPreAssign){
            placedb.SetModuleLayerAssign( moduleId, cell->get_posZ());
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
    if(param.b3d){
        cout << BLUE << "[Placer]" << RESET << " - Global: " << count_zChanged << " cells changed die.\n";
    }
}

bool Placer_C::order_place(){
    int curDie = 0;
    int curRow = 0;
    int curX = 0;
    for(Cell_C* cell : _vCell){
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
        } else if(curDie < 1){
            curDie += 1;
            curRow = 0;
            curX = 0;
            cell->set_xy(Pos(curDie, curX, curRow*_pChip->get_die(curDie)->get_row_height()));
            cell->set_die(_pChip->get_die(curDie));
            curX += cell->get_width(_pChip->get_die(curDie)->get_techId());
        } else{
			cout << "_pChip->get_width()=" << _pChip->get_width() << "\n";
			cout << "curX + cell->get_width("<<curDie<<") = " << curX << " + " << cell->get_width(_pChip->get_die(curDie)->get_techId()) << " = " << curX + cell->get_width(_pChip->get_die(curDie)->get_techId()) << "\n";
            cout << BLUE << "[Placer]" << RESET << " - " << YELLOW << "Warning! " << RESET << "No more space to place for cell \'" << cell->get_name() << "\', the cell is placed at (0,0,0).\n";
            cell->set_xy(Pos(0,0,0));
            cell->set_die(_pChip->get_die(0));
        }
    }
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
                    Pos pin_offset1 = cell->get_master_cell()->get_pin_offset(_pChip->get_die(1)->get_techId() ,pin->get_id());
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
        if(net->is_cross_net()){
            int x = rand()%(_pChip->get_width());
            int y = rand()%(_pChip->get_height());
            net->set_ball_xy(Pos(x,y));
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
    cout << BLUE << "[Placer]" << RESET << " - [3.3] total HPWL = " << total_hpwl << ".\n";
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
    cout << BLUE << "[Placer]" << RESET << " - [3.4] total HPWL = " << total_hpwl << ".\n";
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
    cout << BLUE << "[Placer]" << RESET << " - [3.5] total HPWL = " << total_hpwl << ".\n";
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
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: total HPWL = " << total_hpwl << ".\n";
    return true;
}
bool Placer_C::shrunk2d_replace(){
    double part_time_start=0, total_part_time=0;
    cout << BLUE << "[Placer]" << RESET << " - Start Pseudo3D Placement Flow (Shrunk2D).\n";
    
    bool placer_succ;
    ////////////////////////////////////////////////////////////////
    // Init Placement (2D Placement)
    ////////////////////////////////////////////////////////////////
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 1]" << RESET << ": Init 2D Placement.\n";
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    placer_succ = shrunked_2d_replace();
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
    int total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - [3.3] total HPWL = " << total_hpwl << ".\n";
    // 4. Replace die0 again with projected die1 and balls
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
    cout << BLUE << "[Placer]" << RESET << " - [3.4] total HPWL = " << total_hpwl << ".\n";
    // 5. Replace die1 again with projected die1 and balls
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
    cout << BLUE << "[Placer]" << RESET << " - [3.5] total HPWL = " << total_hpwl << ".\n";
    // 6. Replace die0 again with projected die1 and balls
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
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: total HPWL = " << total_hpwl << ".\n";
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
    cout << BLUE << "[Placer]" << RESET << " - total HPWL = " << total_hpwl << ".\n";
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
    cout << BLUE << "[Placer]" << RESET << " - [3.1] total HPWL = " << total_hpwl << ".\n";
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
    cout << BLUE << "[Placer]" << RESET << " - [3.2] total HPWL = " << total_hpwl << ".\n";
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
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: total HPWL = " << total_hpwl << ".\n";
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
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: total HPWL = " << total_hpwl << ".\n";
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
    cout << BLUE << "[Placer]" << RESET << " - d2d_NTUpalce: total HPWL = " << total_hpwl << ".\n";
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
    int slice = round(((double)_pChip->get_die(1)->get_row_num()) / ((double)_pChip->get_die(0)->get_row_num()) * k_part) ;
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
        Pos pin_offset = Pos(ball_w/2, ball_h/2);
        aux.add_pin(net->get_name(), net->get_name(), 'O', pin_offset.x, pin_offset.y);
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
        Pos pin_offset = Pos(ball_w/2, ball_h/2);
        aux.add_pin(net->get_name(), net->get_name(), 'O', pin_offset.x, pin_offset.y);
    }
    // rows
    int shrink = ceil(_pChip->get_ball_spacing()/2.0);
    int numRow = (_pChip->get_height()-shrink*2) / ball_h;
    for(int i=0;i<numRow;++i){
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
        Pos pin_offset = Pos(ball_w/2, ball_h/2);
        aux.add_pin(net->get_name(), net->get_name(), 'O', pin_offset.x, pin_offset.y);
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
void Placer_C::init_run_dir(){
    string cmd_clean = "rm -rf " + _RUNDIR + "; mkdir -p " + _RUNDIR;
    system(cmd_clean.c_str());
}

void Placer_C::clear(){
    _pChip = nullptr;
    _pDesign = nullptr;

    _vCell.clear();
    _vNet.clear();
}
