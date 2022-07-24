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
    // output result
    //output_result(); 
    // visualization (output svg.html)
    if(!_paramHdl.check_flag_exist("no_dump")){
        draw_layout_result();
        draw_layout_result_plt();
    }
    
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

int* getRandRGB(){
    int* color = new int(3);
    color[0] = rand()%250;
    color[1] = rand()%250;
    color[2] = rand()%250;
    return color;
}
void DmMgr_C::draw_layout_result(){// output in dir "./draw/<case-name>.html"
    string outFile = "./draw/" + _paramHdl.get_case_name() + ".html";
    system("mkdir -p ./draw/");
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
    cout << BLUE << "[DM]" << RESET << " - Visualize the layout in \'" << outFile << "\'.\n";
}

void plotBoxPLT( ofstream& stream, double x1, double y1, double x2, double y2 )
{
    stream << x1 << ", " << y1 << endl << x2 << ", " << y1 << endl
           << x2 << ", " << y2 << endl << x1 << ", " << y2 << endl
           << x1 << ", " << y1 << endl << endl;
}


void DmMgr_C::draw_layout_result_plt(){// output in dir "./draw/<case-name>.plt"
    string outFile = "./draw/" + _paramHdl.get_case_name() + ".plt";
    system("mkdir -p ./draw/");
    ofstream outfile( outFile.c_str() , ios::out );

    outfile << " " << endl;
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
    outfile << "plot[:][:]  '-' w l lt 3 lw 2, '-' with filledcurves closed fc \"grey90\" fs border lc \"red\", '-' w l lt 1" << endl << endl;
    
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
    

    outfile << "pause -1 'Press any key to close.'" << endl;
    outfile.close();

    cout << BLUE << "[DM]" << RESET << " - Visualize the plt layout in \'" << outFile << "\'.\n";
}



