#include "dm.h"

bool sortByVal(const pair<int, int> &a, const pair<int, int> &b){ 
    return (a.second > b.second); 
} 

DmMgr_C::DmMgr_C(){};
DmMgr_C::DmMgr_C(Parser_C& parser, ParamHdl_C& paramHdl, clock_t tStart){
    cout << BLUE << "[DM]" << RESET << " - Construct Data Structure...\n";
    _paramHdl = paramHdl;
    vector<ParserTech>& v_tech = parser.get_techs();
    vector<ParserInst>& v_inst = parser.get_insts();
    vector<ParserNet>& v_net = parser.get_nets();
    _pChip = new Chip_C();
    _pDesign = new Design_C();
    // tech lib
    for(int tech_id=0;tech_id<v_tech.size();++tech_id){
        ParserTech tech = v_tech[tech_id];
        _vTechName.emplace_back(tech.name);
        _mTechName2Id[tech.name] = tech_id;
        vector<ParserLibCell>& v_libcell = tech.v_libCell;
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
            vector<ParserLibPin>& v_libPin = v_libcell[i].v_libPin;
            for(int j=0;j<v_libPin.size();++j){
                _vCellLib[i]->add_pin(tech_id, v_libPin[j].name, Pos(v_libPin[j].locationX, v_libPin[j].locationY));
            }
        }
    }    
    // cells
    unordered_map<int,int> mCellDegree;
    for(ParserInst& inst : v_inst){
        Cell_C* newCell = new Cell_C(inst.name, _mCellLib[inst.libCellName]);
        _pDesign->add_cell(newCell);
    }
    // nets
    unordered_map<int,int> mNetDegree;
    for(ParserNet& net : v_net){
        Net_C* newNet = new Net_C(net.name);
        for(ParserPin& pin : net.v_pin){
            Pin_C* p_Pin = _pDesign->get_cell(pin.instName)->get_pin(pin.libPinName);
            newNet->add_pin(p_Pin);
        }
        _pDesign->add_net(newNet);
        // degree
        auto it = mNetDegree.find(net.numPins);
        if(it==mNetDegree.end()) mNetDegree[net.numPins] = 1;
        else mNetDegree[net.numPins]++; 
    }
    for(auto it : mNetDegree){
        _vSortedNetDegree.emplace_back(pair<int,int>(it.first,it.second));   
    }
    sort(_vSortedNetDegree.begin(), _vSortedNetDegree.end(), sortByVal); 
    // cells degree
    vector<Cell_C*> v_cell = _pDesign->get_cells();
    for(Cell_C* cell : v_cell){
        int cellDegree = 0;
        for(int i=0;i<cell->get_pin_num();++i){
            if(cell->get_pin(i)->get_net() != nullptr) cellDegree++;
        }
        auto it = mCellDegree.find(cellDegree);
        if(it==mCellDegree.end()) mCellDegree[cellDegree] = 1;
        else mCellDegree[cellDegree]++; 
    }
    for(auto it : mCellDegree){
        _vSortedCellDegree.emplace_back(pair<int,int>(it.first,it.second));   
    }
    sort(_vSortedCellDegree.begin(), _vSortedCellDegree.end(), sortByVal); 
    // dies
    ParserDie topDie = parser.get_top_die_info();
    ParserDie botDie = parser.get_bot_die_info();
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
    cout << BLUE << "[DM]" << RESET << " - Construct Data Structure"<< GREEN <<" successfully!" << RESET << "\n";
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
    _vSortedNetDegree.clear();
    _vSortedCellDegree.clear();
}

void DmMgr_C::run(){
    cout << BLUE << "[DM]" << RESET << " - Start\n";
    // init place
    _pPlacer = new Placer_C(_pChip, _pDesign, _paramHdl, _tStart);
    if(_paramHdl.check_flag_exist("safe_mode")){
        _pPlacer->run_safe_mode();
        print_result();
        output_result(_paramHdl.get_output_fileName());
    }
    _pPlacer->run();
    
    cout << BLUE << "[DM]" << RESET << " - Finish!\n";
}

void DmMgr_C::print_result(){
    vector<long long> vHPWL(2,0);
    long long totalHPWL = 0;
    for(int i=0;i<_pDesign->get_net_num();++i){
        Net_C* net = _pDesign->get_net(i);
        vHPWL[0] += net->get_HPWL(0);
        vHPWL[1] += net->get_HPWL(1);
        totalHPWL += (long long)net->get_HPWL(0) + (long long)net->get_HPWL(1);
    }
    cout << "------------------------ HPWL Result ------------------------\n";
    cout << "Top-Die  HPWL   = " << vHPWL[0] << "\n";
    cout << "Bot-Die  HPWL   = " << vHPWL[1] << "\n";
    cout << "Total    HPWL   = " << totalHPWL << "\n";
    cout << "Terminal Usage  = " << _pChip->get_d2d_nets().size() << "/" << _pChip->get_max_ball_num() << "\n";
}

void DmMgr_C::print_info(){
    cout << BLUE << "[DM]" << RESET << " - Print information:  ------------------------------------------------\n";
    cout << "numCell=" << _pDesign->get_cell_num() << ", numNet=" << _pDesign->get_net_num() << ", , numTech=" << _vTechName.size() << "\n";
    // Dies
    cout << "Chip size = " << _pChip->get_width() << "*" << _pChip->get_height() << "\n";
    for(int i=0;i<_pChip->get_die_num();++i){
        cout << "Die[" << i << "]: " <<  "rowHeight=" << _pChip->get_die(i)->get_row_height() << ", rowNum=" << _pChip->get_die(i)->get_row_num() << "\n";
    }
    // Ball
    cout << "Terminal: " << _pChip->get_ball_width() << "*" << _pChip->get_ball_height() << ", spacing=" << _pChip->get_ball_spacing() << ", maxNum=" << _pChip->get_max_ball_num() << "\n";
    // Net Degree
    if(_vSortedNetDegree.size() != 0){
        cout << "Net Degree: " << "max=" << _vSortedNetDegree[0].second << "(deg=" << _vSortedNetDegree[0].first << "), min=" << _vSortedNetDegree.back().second << "(deg=" << _vSortedNetDegree.back().first << ")\n";
        for(int i=0; i<_vSortedNetDegree.size();++i){
            if(i<3 || i==_vSortedNetDegree.size()-1)
                cout << "  degree=" << _vSortedNetDegree[i].first << ": " << _vSortedNetDegree[i].second << " nets\n";
            else if(i==3)
                cout << "  ...\n";
        }
    }
    else{
        cout << "Max Net Degree = 0\n";
    }
    // Cell Degree
    if(_vSortedCellDegree.size() != 0){
        cout << "Cell Degree: " << "max=" << _vSortedCellDegree[0].second << "(deg=" << _vSortedCellDegree[0].first << "), min=" << _vSortedCellDegree.back().second << "(deg=" << _vSortedCellDegree.back().first << ")\n";
        for(int i=0; i<_vSortedCellDegree.size();++i){
            if(i<3 || i==_vSortedCellDegree.size()-1)
                cout << "  degree=" << _vSortedCellDegree[i].first << ": " << _vSortedCellDegree[i].second << " cells\n";
            else if(i==3)
                cout << "  ...\n";
        }
    }
    else{
        cout << "Max Cell Degree = 0\n";
    }
    cout << "--------------------------------------------------------------------------\n";
}
void DmMgr_C::dump_info(){
    system("mkdir -p dump");
    ofstream fout("dump/data_info.txt");
    fout << BLUE << "[DM]" << RESET << " - Print information: ------------------------------------------------\n";
    fout << "numCell=" << _pDesign->get_cell_num() << ", numNet=" << _pDesign->get_net_num() << ", , numTech=" << _vTechName.size() << "\n";
    // Cell
    for(int i=0;i<_pDesign->get_cell_num();++i){
        Cell_C* cell = _pDesign->get_cell(i);
        fout << "Cell " << cell->get_name() << "(" << cell->get_master_cell()->get_name() << "), pos(" << cell->get_pos().to_str() << ", size=" <<cell->get_width() << "*" << cell->get_height()
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
    for(int i=0;i<_pChip->get_die_num();++i){
        fout << "Die[" << i << "]: " << "tech=" << _vTechName[_pChip->get_die(i)->get_techId()] << ", maxUtil=" << _pChip->get_die(i)->get_max_util() << ", rowHeight=" << _pChip->get_die(i)->get_row_height() << ", rowNum=" << _pChip->get_die(i)->get_row_num() << "\n";
    }
    // Ball
    fout << "Ball: " << _pChip->get_ball_width() << "*" << _pChip->get_ball_height() << ", spacing=" << _pChip->get_ball_spacing() << "\n";
    fout << "--------------------------------------------------------------------------\n";
    fout.close();
}
void DmMgr_C::output_result(string fileName){
    ofstream fout(_paramHdl.get_output_fileName());
    Die_C* topDie = _pChip->get_die(0);
    vector<Cell_C*>& v_topCells = topDie->get_cells();
    fout << "TopDiePlacement " << v_topCells.size() << "\n";
    for(Cell_C* cell : v_topCells){
        fout << "Inst " << cell->get_name() << " " << cell->get_posX() << " " << cell->get_posY() << "\n";
    }
    Die_C* botDie = _pChip->get_die(1);
    vector<Cell_C*>& v_botCells = botDie->get_cells();
    fout << "BottomDiePlacement " << v_botCells.size() << "\n";
    for(Cell_C* cell : v_botCells){
        fout << "Inst " << cell->get_name() << " " << cell->get_posX() << " " << cell->get_posY() << "\n";
    }
    vector<Net_C*>& v_d2dNets = _pChip->get_d2d_nets();
    fout << "NumTerminals " << v_d2dNets.size() << "\n";
    for(Net_C* net : v_d2dNets){
        fout << "Terminal " << net->get_name() << " " << net->get_ball_pos().x << " " << net->get_ball_pos().y << "\n";
    }
    fout.close();
}
void DmMgr_C::output_result(){
    output_result(_paramHdl.get_output_fileName());
}

void DmMgr_C::output_aux_form(int dieId){  // output in dir "./aux/<case-name>/"
    string caseName = _paramHdl.get_case_name();
    string aux_dir = "./aux/" + caseName + "/";
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
    // rows
    aux.set_default_rows(_pChip->get_width(), _pChip->get_die(dieId)->get_row_height(), _pChip->get_die(dieId)->get_row_num());

    // write
    aux.write_files();
    cout << BLUE << "[DM]" << RESET << " - Output aux format in \'" << aux_dir << "\'.\n";
}
