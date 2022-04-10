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
    for(Cell_C* cell : _vCell){
        cell->set_xy(Pos(0,0));
        cell->set_die(_pChip->get_die(0));
    }
    for(Net_C* net : _vNet){
        net->set_ball_xy(Pos(0,0));
    }
}

void Placer_C::clear(){
    _pChip = nullptr;
    _pDesign = nullptr;

    _vCell.clear();
    _vNet.clear();

}