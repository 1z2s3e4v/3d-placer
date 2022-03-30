#include "dm.h"
#include <bits/types/clock_t.h>

DmMgr_C::DmMgr_C(){};
DmMgr_C::DmMgr_C(Parser_C& parser, ParamHdl_C& paramHdl, clock_t tStart){
    vector<Tech>& v_tech = parser.get_techs();
    vector<Inst>& v_inst = parser.get_insts();
    vector<Net>& v_net = parser.get_nets();
    _pChip = new Chip_C();
    _pDesign = new Design_C();
    // tech lib
    for(int tech_id=0;tech_id<v_tech.size();++tech_id){
        Tech tech = v_tech[tech_id];
        _vTechName.emplace_back(tech.name);
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
    Die botDie = parser.get_top_die_info();
    _pChip = new Chip_C(topDie.ur_x - topDie.ll_x, 
                       topDie.ur_y - topDie.ll_y);
    _pChip->set_top_die(topDie.maxUtil,
                       _mTechName2Id[topDie.dieTech],
                       topDie.rowHeight);
    _pChip->set_bot_die(botDie.maxUtil,
                       _mTechName2Id[botDie.dieTech],
                       botDie.rowHeight);
    Terminal t = parser.get_terminal_info();
    _pChip->set_ball(t.sizeX, t.sizeY, t.spacing);
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