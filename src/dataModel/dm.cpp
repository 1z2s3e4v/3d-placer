#include "dm.h"
#include <bits/types/clock_t.h>
#include <fstream>
#include <type_traits>

DmMgr_C::DmMgr_C(){};
DmMgr_C::DmMgr_C(Parser_C& parser, ParamHdl_C& paramHdl, clock_t tStart){
    cout << BLUE << "[DM]" << RESET << " - Construct Data Structure...\n";
    vector<Tech>& v_tech = parser.get_techs();
    vector<Inst>& v_inst = parser.get_insts();
    vector<Net>& v_net = parser.get_nets();
    _pChip = new Chip_C();
    _pDesign = new Design_C();
    // tech lib
    for(int tech_id=0;tech_id<v_tech.size();++tech_id){
        Tech tech = v_tech[tech_id];
        _vTechName.emplace_back(tech.name);
        _mTechName2Id[tech.name] = tech_id;
        vector<LibCell>& v_libcell = tech.v_libCell;
        for(int i=0;i<v_libcell.size();++i){
            CellLib_C* cellLib;
            if(tech_id==0){ // init the CellLib list with the first tech
                cellLib = new CellLib_C(v_libcell[i].name, v_libcell[i].numLibPin , v_tech.size());
                _mCellLib.emplace(v_libcell[i].name, cellLib); // id, cellLib
                _vCellLib.emplace_back(cellLib);
            }
            else{
                cellLib = _mCellLib[v_libcell[i].name];
            }
            _vCellLib[i]->set_size(tech_id, v_libcell[i].sizeX, v_libcell[i].sizeY);
            vector<LibPin>& v_libPin = v_libcell[i].v_libPin;
            for(int j=0;j<v_libPin.size();++j){
                _vCellLib[i]->add_pin(tech_id, v_libPin[j].name, Pos(v_libPin[j].locationX, v_libPin[j].locationY));
            }
        }
    }    
    // cells
    for(Inst& inst : v_inst){
        Cell_C* newCell = new Cell_C(inst.name, _mCellLib[inst.libCellName]);
        _pDesign->add_cell(newCell);
    }
    // nets
    for(Net& net : v_net){
        Net_C* newNet = new Net_C(net.name);
        for(Pin& pin : net.v_pin){
            Pin_C* p_Pin = _pDesign->get_cell(pin.instName)->get_pin(pin.libPinName);
            newNet->add_pin(p_Pin);
        }
        _pDesign->add_net(newNet);
    }
    // dies
    Die topDie = parser.get_top_die_info();
    Die botDie = parser.get_bot_die_info();
    _pChip = new Chip_C(topDie.ur_x - topDie.ll_x, 
                       topDie.ur_y - topDie.ll_y, 2);
    _pChip->set_die(0, topDie.maxUtil,
                       _mTechName2Id[topDie.dieTech],
                       topDie.rowHeight);
    _pChip->set_die(1, botDie.maxUtil,
                       _mTechName2Id[botDie.dieTech],
                       botDie.rowHeight);
    Terminal t = parser.get_terminal_info();
    _pChip->set_ball(t.sizeX, t.sizeY, t.spacing);
    cout << BLUE << "[DM]" << RESET << " - Construct Data Structure "<< GREEN <<" successfully!" << RESET << "\n";
}

void DmMgr_C::init(){
    _tStart = 0;
    _total_WL = 0;
    _pChip = nullptr;
    _pDesign = nullptr;
    _pPlacer = nullptr;
    _vCellLib.clear();
    _mCellLib.clear();
    _vTechName.clear();
}

void DmMgr_C::run(){
    cout << BLUE << "[DM]" << RESET << " - Start\n";
    // init place
    for(int i=0;i<_pDesign->get_cell_num();++i){
        Cell_C* cell = _pDesign->get_cell(i);
        cell->set_xy(Pos(0,0));
        cell->set_die(_pChip->get_die(0));
    }
    cout << BLUE << "[DM]" << RESET << " - Finish!\n";
}

void DmMgr_C::print_info(){
    cout << BLUE << "[DM]" << RESET << " - Print information: --------------------------------------------------------------------------------------\n";
    cout << "numCell=" << _pDesign->get_cell_num() << ", numNet=" << _pDesign->get_net_num() << ", , numTech=" << _vTechName.size() << "\n";
    // Cell
    for(int i=0;i<_pDesign->get_cell_num();++i){
        Cell_C* cell = _pDesign->get_cell(i);
        cout << "Cell " << cell->get_name() << "(" << cell->get_master_cell()->get_name() << "), pos(" << cell->get_pos().to_str() << ", size=" <<cell->get_width() << "*" << cell->get_height()
            << ", Pins=[";
        for(int j=0;j<cell->get_pin_num();++j){
            Pin_C* pin = cell->get_pin(j);
            cout << pin->get_name() << pin->get_pos().xy_to_str();
            if(j<cell->get_pin_num()-1) cout << ", ";
            else cout << "]\n";
        }
    }
    // Net
    for(int i=0;i<_pDesign->get_net_num();++i){
        Net_C* net = _pDesign->get_net(i);
        cout << "Net " << net->get_name() << "\n";
        for(int j=0;j<net->get_pin_num();++j){
            Pin_C* pin = net->get_pin(j);
            cout << "  Pin " << pin->get_cell()->get_name() << "/" << pin->get_name() << "\n";
        }
    }
    // Dies
    cout << "Chip size = " << _pChip->get_width() << "*" << _pChip->get_height() << "\n";
    for(int i=0;i<_pChip->get_die_num();++i){
        cout << "Die[" << i << "]: " << "tech=" << _vTechName[_pChip->get_die(i)->get_techId()] << ", maxUtil=" << _pChip->get_die(i)->get_max_util() << ", rowHeight=" << _pChip->get_die(i)->get_row_height() << ", rowNum=" << _pChip->get_die(i)->get_row_num() << "\n";
    }
    // Ball
    cout << "Ball: " << _pChip->get_ball_width() << "*" << _pChip->get_ball_height() << ", spacing=" << _pChip->get_ball_spacing() << "\n";
    cout << "-------------------------------------------------------------------------------------------------------------------\n";
}
void DmMgr_C::dump_info(){
    system("mkdir -p dump");
    ofstream fout("dump/data_info.txt");
    fout << "numCell=" << _pDesign->get_cell_num() << ", numNet=" << _pDesign->get_net_num() << ", , numTech=" << _vTechName.size() << "\n";
    // Cell
    for(int i=0;i<_pDesign->get_cell_num();++i){
        Cell_C* cell = _pDesign->get_cell(i);
        fout << "Cell " << cell->get_name() << "(" << cell->get_master_cell()->get_name() << "), pos(" << cell->get_pos().to_str() << ", size = " <<cell->get_width() << "*" << cell->get_height()
            << ", Pins=[";
        for(int j=0;j<cell->get_pin_num();++j){
            Pin_C* pin = cell->get_pin(j);
            fout << pin->get_name() << pin->get_pos().xy_to_str();
            if(j<cell->get_pin_num()-1) fout << ", ";
            else fout << "]\n";
        }
    }
    // Net
    for(int i=0;i<_pDesign->get_net_num();++i){
        Net_C* net = _pDesign->get_net(i);
        fout << "Net " << net->get_name() << "\n";
        for(int j=0;j<net->get_pin_num();++j){
            Pin_C* pin = net->get_pin(j);
            fout << "  Pin " << pin->get_cell()->get_name() << "/" << pin->get_name() << "\n";
        }
    }
    // Dies
    fout << "Chip size = " << _pChip->get_width() << "*" << _pChip->get_height() << "\n";
    for(int i=0;i<1;++i){
        fout << "Die[" << i << "]: maxUtil=" << _pChip->get_die(i)->get_max_util() << ", tech=" << _vTechName[_pChip->get_die(i)->get_techId()] << ", rowNum=" << _pChip->get_die(i)->get_row_num() << "\n";
    }
    // Ball
    fout << "Ball: " << _pChip->get_ball_width() << "*" << _pChip->get_ball_height() << ", spacing=" << _pChip->get_ball_spacing() << "\n";
}
void DmMgr_C::output_result(string fileName){

}