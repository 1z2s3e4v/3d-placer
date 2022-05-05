#include "dm.h"
#include <bits/types/clock_t.h>
#include <fstream>
#include <type_traits>

DmMgr_C::DmMgr_C(){};
DmMgr_C::DmMgr_C(Parser_C& parser, ParamHdl_C& paramHdl, clock_t tStart){
    cout << BLUE << "[DM]" << RESET << " - Construct Data Structure...\n";
    _paramHdl = paramHdl;
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
}

void DmMgr_C::run(){
    cout << BLUE << "[DM]" << RESET << " - Start\n";
    // init place
    _pPlacer = new Placer_C(_pChip, _pDesign, _tStart);
    _pPlacer->run();
    // output result
    //output_result(); 
    // visualization (output svg.html)
    draw_layout_result();
    cout << BLUE << "[DM]" << RESET << " - Finish!\n";
}

void DmMgr_C::print_result(){
    vector<int> vHPWL(2,0);
    int totalHPWL = 0;
    for(int i=0;i<_pDesign->get_net_num();++i){
        Net_C* net = _pDesign->get_net(i);
        vHPWL[0] += net->get_HPWL(0);
        vHPWL[1] += net->get_HPWL(1);
        totalHPWL += net->get_HPWL(0) + net->get_HPWL(1);
    }
    cout << "------------------------ HPWL Result ------------------------\n";
    cout << "Top-Die HPWL = " << vHPWL[0] << "\n";
    cout << "Bot-Die HPWL = " << vHPWL[1] << "\n";
    cout << "Total   HPWL = " << totalHPWL << "\n";
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
    fout << BLUE << "[DM]" << RESET << " - Print information: --------------------------------------------------------------------------------------\n";
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
    fout << "-------------------------------------------------------------------------------------------------------------------\n";
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
        fout << "Terminal " << net->get_name() << " " << net->get_ball_pos().x << net->get_ball_pos().y << "\n";
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
    for(int i=0;i<_pDesign->get_cell_num();++i){
        Cell_C* cell = _pDesign->get_cell(i);
        //cout << "Draw: " << cell->get_name() << " " << cell->get_pos().pos3d2str() << "\n";
        int dieX = diePos[cell->get_dieId()].x;
        int dieY = diePos[cell->get_dieId()].y;
        map<string,string> m_para{{"type","cell"}};
        draw_svg->drawRect(cell->get_name(), drawBox(drawPos(dieX+cell->get_posX(),dieY+cell->get_posY()),drawPos(dieX+cell->get_posX()+cell->get_width(),dieY+cell->get_posY()+cell->get_height())), "yellow", 0.1, m_para);
        // cell name lable
        draw_svg->drawText(cell->get_name()+"_label", drawPos(dieX+cell->get_posX()+cell->get_width()/2.0, dieY+cell->get_posY()+cell->get_height()/2.0), cell->get_name());
        // draw pins
        for(int j=0;j<cell->get_pin_num();++j){
            Pin_C* pin = cell->get_pin(j);
            if(pin->get_net() != nullptr){
                map<string,string> m_para{{"type","pin"},{"cell",pin->get_cell()->get_name()}, {"net",pin->get_net()->get_name()}};
                draw_svg->drawRect(cell->get_name()+"_"+pin->get_name(), drawBox(drawPos(dieX+pin->get_x()-0.5,dieY+pin->get_y()-0.5),drawPos(dieX+pin->get_x()+0.5,dieY+pin->get_y()+0.5)), v_netColor[pin->get_net()->get_id()], 0.6, m_para);
            }
            else{
                map<string,string> m_para{{"type","pin"},{"cell",pin->get_cell()->get_name()}};
                draw_svg->drawRect(cell->get_name()+"_"+pin->get_name(), drawBox(drawPos(dieX+pin->get_x()-0.5,dieY+pin->get_y()-0.5),drawPos(dieX+pin->get_x()+0.5,dieY+pin->get_y()+0.5)), "black", 1, m_para);
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
            draw_svg->drawBBox(net->get_name()+"_HPWL"+to_string(dieId), drawBox(drawPos(dieX+net->get_ll(dieId).x,dieY+net->get_ll(dieId).y),drawPos(dieX+net->get_ur(dieId).x,dieY+net->get_ur(dieId).y)), v_netColor[net->get_id()], 0.5, 0.6, m_para);
        }
    }
    // Output HPWL Result Text
    vector<int> vHPWL(2,0);
    int totalHPWL = 0;
    for(int i=0;i<_pDesign->get_net_num();++i){
        Net_C* net = _pDesign->get_net(i);
        vHPWL[0] += net->get_HPWL(0);
        vHPWL[1] += net->get_HPWL(1);
        totalHPWL += net->get_HPWL(0) + net->get_HPWL(1);
    }
    draw_svg->drawText("Die0_HPWL_result", drawPos((diePos[0].x+_pChip->get_width()/3.0), diePos[0].y), "Top_Die HPWL = " + to_string(vHPWL[0]), 0, -25);
    draw_svg->drawText("Die1_HPWL_result", drawPos((diePos[1].x+_pChip->get_width()/3.0), diePos[1].y), "Bot_Die HPWL = " + to_string(vHPWL[1]), 0, -25);
    draw_svg->drawText("Total_HPWL_result", drawPos(diePos[0].x, diePos[0].y), "Total HPWL = " + to_string(totalHPWL), 0, -50);

    draw_svg->end_svg();
    cout << BLUE << "[DM]" << RESET << " - Visualize the layout in \'" << outFile << "\'.\n";
}