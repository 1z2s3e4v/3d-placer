#include "placer.h"
#include <bits/types/clock_t.h>
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
    // init place
    //init_place();
    //ntu_d2dplace();
    pin3d_ntuplace();
	//order_place();
    //true3d_placement();

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

void Placer_C::true3d_placement(){
    cout << BLUE << "[Placer]" << RESET << " - Start True3d Placement Flow.\n";
    double part_time_start=0, total_part_time=0;
    ////////////////////////////////////////////////////////////////
    // Global Placement
    ////////////////////////////////////////////////////////////////
    bool isLegal = false; 
    double wl1 = 0; // gp-wire
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 1]" << RESET << ": Global Placement.\n";
    global_place(isLegal, wl1); /////////////////////////////////////////////// main function
    //ntu_d2d_global();
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
    part_time_start = (float)clock() / CLOCKS_PER_SEC;
    cout << BLUE << "[Placer]" << RESET << " - " << BLUE << "[STAGE 2]" << RESET << ": Legalization + Detail Placement.\n";
    ntu_d2d_legal_detail(); ////////////////////////////////////////////////////// main function
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    int total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: total HPWL = " << total_hpwl << ".\n";
}
void Placer_C::global_place(bool& isLegal, double& totalHPWL){ // Analytical Global Placement
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
    //placedb.ShowDBInfo();
	//CPlaceUtil::GetTotalOverlapArea( placedb ); // 2007-07-22 (donnie)
    // if( 1 ){ // spreading
	//     globalLocalSpreading( &placedb_die0, 1.01 );
    // }
    placedb_die0.RemoveFixedBlockSite();
    isLegal = multilevel_nlp( placedb_die0, 5, 1.0 ); // multilevel_nlp(placedb, gCType, gWeightLevelDecreaingRate)
    load_from_placedb(placedb_die0);

    // die1
    CPlaceDB placedb_die1;
    create_placedb(placedb_die1, 1);
    //placedb.ShowDBInfo();
	//CPlaceUtil::GetTotalOverlapArea( placedb ); // 2007-07-22 (donnie)
    // if( 1 ){ // spreading
	//     globalLocalSpreading( &placedb_die0, 1.01 );
    // }
    placedb_die0.RemoveFixedBlockSite();
    isLegal = multilevel_nlp( placedb_die0, 5, 1.0 ); // multilevel_nlp(placedb, gCType, gWeightLevelDecreaingRate)
    load_from_placedb(placedb_die0);
    // rand_place(0);
    // rand_place(1);
    // rand_ball_place();
}
void Placer_C::legal_place(){
}
void Placer_C::detail_place(){
}
void Placer_C::ntu_d2d_global(){
    // partition
    if(_pChip->get_die(0)->get_row_num()==_pChip->get_die(1)->get_row_num())
        mincut_partition();
    else 
        mincut_k_partition();
    cout << BLUE << "[Placer]" << RESET << " - Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell_num = " << _pChip->get_die(1)->get_cells().size() << "\n";
    
    rand_place(0);
    rand_place(1);
    rand_ball_place();
    // run ntuplace (nolegal and nodetail) for die0
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux0;
        create_aux_form(aux0, 0, "die0");
        add_project_pin(aux0, 1);
        // check nets
        aux0.remove_open_net();
        aux0.write_files();
        run_ntuplace3("die0", "-nolegal -nodetail -devdev");
        read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
    }
    // run ntuplace (noglobal) for die1 
    if(_pChip->get_die(1)->get_cells().size() > 0){
        AUX aux1;
        create_aux_form(aux1, 1, "die1");
        add_project_pin(aux1, 0);
        // check nets
        aux1.remove_open_net();
        aux1.write_files();
        run_ntuplace3("die1", "-nolegal -nodetail");
        read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 1);
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
        read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
    }
}
void Placer_C::ntu_d2d_legal_detail(){
    // run ntuplace (noglobal) for die0
    if(_pChip->get_die(0)->get_cells().size() > 0){
        AUX aux0;
        create_aux_form(aux0, 0, "die0");
        add_project_pin(aux0, 1);
        // check nets
        aux0.remove_open_net();
        aux0.write_files();
        run_ntuplace3("die0", "-noglobal");
        read_pl_and_set_pos(_RUNDIR+"die0.ntup.pl", 0);
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
        read_pl_and_set_pos(_RUNDIR+"die1.ntup.pl", 1);
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
        read_pl_and_set_pos_for_ball(_RUNDIR+"ball.ntup.pl");
    }
}
void Placer_C::set_ntuplace_param(CPlaceDB& placedb){
    // bShow >>
    // param.bPlot = true;
    // param.bShow = true;
    // param.bLog = true;
    // param.bOutTopPL = true;
    // <<
    param.bRunInit = true;
    param.seed = 1;
    param.bUseLSE = true; // default
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
    cout << "placedb.m_util =================================== " << placedb.m_util << "\n";
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
    double coreCenterX = (placedb.m_coreRgn.left + placedb.m_coreRgn.right ) * 0.5;
	double coreCenterY = (placedb.m_coreRgn.top + placedb.m_coreRgn.bottom ) * 0.5;
	for( unsigned int i = 0; i < placedb.m_modules.size(); i++ ){
	    if( !placedb.m_modules[i].m_isFixed &&
	    	 fabs(placedb.m_modules[i].m_x) < 1e-5 && fabs(placedb.m_modules[i].m_y) < 1e-5 )
	    	placedb.MoveModuleCenter(i, coreCenterX, coreCenterY);
	}
}
void Placer_C::load_from_placedb(CPlaceDB& placedb){
    for( unsigned int i=0; i<placedb.m_modules.size(); i++ ){
        if( !placedb.m_modules[i].m_isFixed ){
            _mCell[placedb.m_modules[i].GetName()]->set_pos(Pos(placedb.m_modules[i].GetX(), placedb.m_modules[i].GetY()));
        }
    }
}

void Placer_C::order_place(){
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
void Placer_C::pin3d_ntuplace(){
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
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - Init: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";

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
    if(_pChip->get_die(1)->get_cells().size() > 0){
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
    total_part_time = (float)clock() / CLOCKS_PER_SEC - part_time_start;
    cout << BLUE << "[Placer]" << RESET << " - D2D-PL: runtime = " << total_part_time << " sec = " << total_part_time/60.0 << " min.\n";
    int total_hpwl = cal_HPWL();
    cout << BLUE << "[Placer]" << RESET << " - LG+DP: total HPWL = " << total_hpwl << ".\n";
}
void Placer_C::ntu_d2dplace(){
    // init_place or partition
    //order_place();
    mincut_partition(); // set_die() for each cell
    //mincut_k_partition();
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
    //cout << "hmetis partition result: " << max(hgr.get_part_size(0),hgr.get_part_size(1)) << " : " << min(hgr.get_part_size(0),hgr.get_part_size(1)) << "\n";
    for(Cell_C* cell : _vCell){
        int part = (hgr.get_part_size(0) >= hgr.get_part_size(1))? 
            hgr.get_part_result(cell->get_name()) : (hgr.get_part_result(cell->get_name())+1)%2;
        cell->set_die(_pChip->get_die(part));
    }
    //cout << "Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell)num = " << _pChip->get_die(1)->get_cells().size() << "\n";

    // move cells for matching die's max_utilization
    long long int areaUsing = 0;
    long long int max_usage = _pChip->get_width()*_pChip->get_height()*_pChip->get_die(0)->get_max_util();
    Die_C* die = _pChip->get_die(0);
    vector<Cell_C*>& v_cell = die->get_cells();
    for(Cell_C* cell : v_cell){
        areaUsing += cell->get_width()*cell->get_height();
    }
    int count_move = 0;
    while(areaUsing > max_usage){
        Cell_C* cell = v_cell.back();
        areaUsing -= cell->get_width()*cell->get_height();
        cell->set_die(_pChip->get_die(1));
        ++count_move;
    }
    if(count_move > 0) cout << BLUE << "[Placer]" << RESET << " - " << count_move << " cells moved from die1->die0 for max-util.\n";
    //cout << "Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell)num = " << _pChip->get_die(1)->get_cells().size() << "\n";
}
void Placer_C::mincut_k_partition(){
    int row_sum = _pChip->get_die(0)->get_row_num()+_pChip->get_die(1)->get_row_num();
    int k_part = min(row_sum, 20);
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
    }
    //cout << "Die[0].cell_num = " << _pChip->get_die(0)->get_cells().size() << ", Die[1].cell)num = " << _pChip->get_die(1)->get_cells().size() << "\n";
    // move cells for matching die's max_utilization
    long long int areaUsing = 0;
    long long int max_usage = _pChip->get_width()*_pChip->get_height()*_pChip->get_die(0)->get_max_util();
    Die_C* die = _pChip->get_die(0);
    vector<Cell_C*>& v_cell = die->get_cells();
    for(Cell_C* cell : v_cell){
        areaUsing += cell->get_width()*cell->get_height();
    }
    int count_move = 0;
    while(areaUsing > max_usage){
        Cell_C* cell = v_cell.back();
        areaUsing -= cell->get_width()*cell->get_height();
        cell->set_die(_pChip->get_die(1));
        ++count_move;
    }
    if(count_move > 0) cout << BLUE << "[Partition]" << RESET << " - " << count_move << " cells moved from die1->die0.\n";
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
    }
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
