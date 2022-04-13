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
    ntu_d2dplace();

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
        if(ball_curX + _pChip->get_ball_width() <= _pChip->get_width()-_pChip->get_ball_spacing() && ball_curY + _pChip->get_ball_height() <= _pChip->get_height()-_pChip->get_ball_spacing()){
            net->set_ball_xy(Pos(ball_curX, ball_curY));
            ball_curX += _pChip->get_ball_width() + _pChip->get_ball_spacing();
        } else if(ball_curX + _pChip->get_ball_width() <= _pChip->get_width()-_pChip->get_ball_spacing() && ball_curY+_pChip->get_ball_height() > _pChip->get_height()-_pChip->get_ball_spacing()){
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
void Placer_C::ntu_d2dplace(){
    // init_place or partition
    order_place();
    //mincut_partition(); // set_die() for each cell

    // Run NTUplace3
    string cmd_clean = "rm -rf " + _RUNDIR;
    system(cmd_clean.c_str());
    // run ntuplace3 for die0
    if(_pChip->get_die(0)->get_cells().size() > 0){
        output_aux_form(0, "die0");
        run_ntuplace3("die0");
        read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl");
    }
    // run ntuplace3 for die1
    if(_pChip->get_die(1)->get_cells().size() > 0){
        output_aux_form(1, "die1");
        run_ntuplace3("die1");
        read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl");
    }
    // update the HPWL
    for(Net_C* net : _vNet){
        net->update_HPWL();
    }
    //string cmd_clean = "rm -rf *.pl *.plt *.log " + _RUNDIR;
    //system(cmd_clean.c_str());
}

void Placer_C::mincut_partition(){
    for(int i=0;i<_vCell.size();++i){
        if(i<6){
            _vCell[i]->set_die(_pChip->get_die(0));
            _vCell[i]->set_xy(Pos(0,0));
        }
        else{
            _vCell[i]->set_die(_pChip->get_die(1));
            _vCell[i]->set_xy(Pos(0,0));
        }
    }
}
void Placer_C::read_pl_and_set_pos(string fileName){
    AUX aux;
    vector<AuxNode> v_auxNode;
    aux.read_pl(fileName, v_auxNode);
    for(AuxNode node : v_auxNode){
        Cell_C* cell = _mCell[node.name];
        cell->set_xy(Pos(node.x,node.y));
        //cout << "Place: " << _mCell[node.name]->get_name() << " " << _mCell[node.name]->get_pos().pos3d2str() << "\n";
        if(!cell->check_drc()){
            cout << BLUE << "[Placer]" << RESET << " - " << YELLOW << "Warning! " << RESET << "NTUplaced Cell \'" << cell->get_name() << "\' at " + cell->get_pos().pos3d2str() +" position not valid.\n";
        }
    }
}
void Placer_C::run_ntuplace3(string caseName){
    cout << BLUE << "[Place]" << RESET << " - Running ntuplace3 for \'" << caseName << "\'...\n";
    // ex: ./bin/ntuplace-r -aux ./run_tmp/die0/die0.aux -out ./run_tmp/die0 > ./run_tmp/die0-ntuplace.log
    string cmd = "./bin/ntuplace-r -aux " + _RUNDIR + caseName + "/" + caseName + ".aux -out " + _RUNDIR + caseName + " > " + _RUNDIR + caseName + "-ntuplace.log";
    system(cmd.c_str());
    cout << BLUE << "[Place]" << RESET << " - Running ntuplace3 for \'" << caseName << "\'" << GREEN << " completed!\n" << RESET;
}
void Placer_C::output_aux_form(int dieId, string caseName){  // output in dir "./aux/<case-name>/"
    string aux_dir = _RUNDIR + caseName + "/";
    string cmd = "mkdir -p " + aux_dir;
    system(cmd.c_str());
    AUX aux(aux_dir, caseName);
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
    // check nets
    aux.remove_open_net();
    // rows
    aux.set_default_rows(_pChip->get_width(), _pChip->get_die(dieId)->get_row_height(), _pChip->get_die(dieId)->get_row_num());

    // write
    aux.write_files();
}
void Placer_C::clear(){
    _pChip = nullptr;
    _pDesign = nullptr;

    _vCell.clear();
    _vNet.clear();
}