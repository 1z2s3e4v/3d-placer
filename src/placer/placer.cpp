#include "placer.h"

using namespace std;

Placer_C::Placer_C(){}
Placer_C::Placer_C(Chip_C* p_pChip, Design_C* p_pDesign, clock_t p_start){
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
    init_place();

    cout << BLUE << "[Placer]" << RESET << " - Finish!\n";
}
void Placer_C::init_place(){
    /*for(Cell_C* cell : _vCell){
        cell->set_xy(Pos(0,0));
        cell->set_die(_pChip->get_die(0));
    }
    for(Net_C* net : _vNet){
        net->set_ball_xy(Pos(0,0));
    }*/

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

void Placer_C::clear(){
    _pChip = nullptr;
    _pDesign = nullptr;

    _vCell.clear();
    _vNet.clear();

}