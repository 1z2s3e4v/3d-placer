#include "placer.h"

using namespace std;

Placer_C::Placer_C(Chip_C* p_pChip, Design_C* p_pDesign, clock_t p_start)
:_vCell(p_pDesign->get_cells()), _vNet(p_pDesign->get_nets()), _mCell(p_pDesign->get_cells_map()), _mNet(p_pDesign->get_nets_map())
{
    _pDesign = p_pDesign;
    _pChip = p_pChip;
    _tStart = p_start;
    _vCell = p_pDesign->get_cells();
    _vNet = p_pDesign->get_nets();
    _vCellBestPos.resize(_vCell.size(),Pos(0,0,0));
}

void Placer_C::run(){
    cout << BLUE << "[Placer]" << RESET << " - Start\n";
    // init place
    //init_place();
    //ntu_d2dplace();
    pin3d_ntuplace();
	//order_place();

    // update the HPWL
    for(Net_C* net : _vNet){
        net->update_bbox();
    }
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
void Placer_C::order_place(){
    int curDie = 0;
    int curRow = 0;
    int curX = 0;
    for(Cell_C* cell : _vCell){
        if(curX + cell->get_width(curDie) <= _pChip->get_width()){
            cell->set_xy(Pos(curDie, curX, curRow*_pChip->get_die(curDie)->get_row_height()));
            cell->set_die(_pChip->get_die(curDie));
            curX += cell->get_width(curDie);
        } else if(curRow+1 < _pChip->get_die(curDie)->get_row_num()){
            curRow += 1;
            curX = 0;
            cell->set_xy(Pos(curDie, curX, curRow*_pChip->get_die(curDie)->get_row_height()));
            cell->set_die(_pChip->get_die(curDie));
            curX += cell->get_width(curDie);
        } else if(curDie < 2){
            curDie += 1;
            curRow = 0;
            curX = 0;
            cell->set_xy(Pos(curDie, curX, curRow*_pChip->get_die(curDie)->get_row_height()));
            cell->set_die(_pChip->get_die(curDie));
            curX += cell->get_width(curDie);
        } else{
            cout << BLUE << "[Placer]" << RESET << " - " << YELLOW << "Warning! " << RESET << "No more space to place for cell \'" << cell->get_name() << "\', the cell is placed at (0,0,0).\n";
            cell->set_xy(Pos(0,0));
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
    for(Net_C* net : _vNet){
        if(net->is_cross_net()){
            int x = rand()%(_pChip->get_width());
            int y = rand()%(_pChip->get_height());
            net->set_ball_xy(Pos(x,y));
        }
    }
}
void Placer_C::pin3d_ntuplace(){
    string cmd_clean = "rm -rf " + _RUNDIR + "; mkdir -p " + _RUNDIR;
    system(cmd_clean.c_str());
    // 0. Init place with hmetis and NTUplace
    // partition
    mincut_partition(); // set_die() for each cell
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
        read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
    }
    // run ntuplace for die1
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux1;
        create_aux_form(aux1, 1, "die1");
        // check nets
        aux1.remove_open_net();
        aux1.write_files();
        run_ntuplace3("die1");
        read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 1);
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
        read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
    }

    // 1. Replace die0 with projected die1
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 0, "die0");
        add_project_pin(aux, 1);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die0");
        read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
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
        read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
    }

    // 2. Replace die1 with projected die0 and balls
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux;
        create_aux_form(aux, 1, "die1");
        add_project_pin(aux, 0);
        add_project_ball(aux);
        // check nets
        aux.remove_open_net();
        aux.write_files();
        run_ntuplace3("die1");
        read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 0);
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
        read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
    }

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
        read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
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
        read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
    }
}
void Placer_C::ntu_d2dplace(){
    string cmd_clean = "rm -rf " + _RUNDIR + "; mkdir -p " + _RUNDIR;
    system(cmd_clean.c_str());
    // init_place or partition
    //order_place();
    mincut_partition(); // set_die() for each cell
    rand_place(0);
    rand_place(1);
    rand_ball_place();
    //init_place_ball();

    // Run NTUplace
    // run ntuplace for die0
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
    //string cmd_clean = "rm -rf *.pl *.plt *.log " + _RUNDIR;
    //system(cmd_clean.c_str());
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
    for(Cell_C* cell : _vCell){
        int part = (hgr.get_part_size(0) >= hgr.get_part_size(1))? 
            hgr.get_part_result(cell->get_name()) : (hgr.get_part_result(cell->get_name())+1)%2;
        cell->set_die(_pChip->get_die(part));
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

void Placer_C::read_pl_and_set_pos(string fileName, int dieId){
    AUX aux;
    vector<AuxNode> v_auxNode;
    if(aux.read_pl(fileName, v_auxNode)){
        for(AuxNode node : v_auxNode){
            if(_mCell.find(node.name) == _mCell.end()) continue;
            Cell_C* cell = _mCell[node.name];
            if(cell->get_dieId() == dieId){
                cell->set_xy(Pos(node.x,node.y));
                //cout << "Place: " << _mCell[node.name]->get_name() << " " << _mCell[node.name]->get_pos().pos3d2str() << "\n";
                if(!cell->check_drc()){
                    cout << BLUE << "[Placer]" << RESET << " - " << YELLOW << "Warning! " << RESET << "NTUplaced Cell \'" << cell->get_name() << "\' at " + cell->get_pos().pos3d2str() +" position not valid.\n";
                }
            }
        }
    }
    else{
        cout << BLUE << "[Placer]" << RESET << " - " << RED << "Error. " << RESET << "NTUplacer failed.\n";
    }
}
void Placer_C::read_pl_and_set_pos_for_ball(string fileName){
    int ball_w = _pChip->get_ball_width() + _pChip->get_ball_spacing();
    int ball_h = _pChip->get_ball_height() + _pChip->get_ball_spacing();
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
    }
}
void Placer_C::run_ntuplace3(string caseName){
    cout << BLUE << "[Place]" << RESET << " - Running ntuplace3 for \'" << caseName << "\'...\n";
    // ex: ./bin/ntuplace-r -aux ./run_tmp/die0/die0.aux -out ./run_tmp/die0 > ./run_tmp/die0-ntuplace.log
    string cmd = "./bin/ntuplace-r -aux " + _RUNDIR + caseName + "/" + caseName + ".aux -out " + _RUNDIR + caseName + " > " + _RUNDIR + caseName + "-ntuplace.log";
    system(cmd.c_str());
    cout << BLUE << "[Place]" << RESET << " - Running ntuplace3 for \'" << caseName << "\'" << GREEN << " completed!\n" << RESET;
}
void Placer_C::run_ntuplace4(string caseName){
    cout << BLUE << "[Place]" << RESET << " - Running ntuplace4 for \'" << caseName << "\'...\n";
    // ex: ./bin/ntuplace-r -aux ./run_tmp/die0/die0.aux -out ./run_tmp/die0 > ./run_tmp/die0-ntuplace.log
    string cmd = "./bin/ntuplace4 -aux " + _RUNDIR + caseName + "/" + caseName + ".aux -out " + _RUNDIR + caseName + " -noCong -noCongLG -noPGAvoid > " + _RUNDIR + caseName + "-ntuplace.log";
    system(cmd.c_str());
    cout << BLUE << "[Place]" << RESET << " - Running ntuplace4 for \'" << caseName << "\'" << GREEN << " completed!\n" << RESET;
}
void Placer_C::run_hmetis(int k, double ufactor, string caseName){
    cout << BLUE << "[Place]" << RESET << " - Running hmetis for \'" << caseName << "\'...\n";
    string fileName = _RUNDIR + caseName + ".hgr";
    string cmd = "bin/hmetis -ufactor=" + to_string(ufactor) + " " + fileName + " " + to_string(k) + " > " + _RUNDIR + caseName + "-hmetis.log";
    system(cmd.c_str());
    cout << BLUE << "[Place]" << RESET << " - Running hmetis for \'" << caseName << "\'" << GREEN << " completed!\n" << RESET;
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
    int numRow = _pChip->get_height() / ball_h;
    int shrink = int(_pChip->get_ball_spacing()/2.0+0.5);
    for(int i=0;i<numRow;++i){
        aux.add_row(shrink+i*ball_h, ball_h, 1, 1, 0, 0, shrink, _pChip->get_width()-_pChip->get_ball_spacing());
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
void Placer_C::clear(){
    _pChip = nullptr;
    _pDesign = nullptr;

    _vCell.clear();
    _vNet.clear();
}
