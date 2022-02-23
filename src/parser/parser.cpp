#include "parser.h"

Parser::Parser(){}
Parser::Parser(string filaName){
    read_file(filaName);
}

bool Parser::read_file(string fileName){
    _isOK = true;
    countReadInfo = 0;
    cout << BLUE << "[Parser]" << RESET << " - Parsing \"" << fileName << "\"...\n";
    ifstream file(fileName);
    string line, label;
    while(getline(file, line)){
        if(line=="") continue;
        stringstream ss(line);
        ss >> label;
        if(label == "NumInstances"){ // Netlist (Insts)
            ss >> numInst;
            int countInst = 0;
            while(countInst < numInst){
                getline(file, line);
                if(line=="") continue;
                stringstream ss2(line);
                // add inst
                Inst inst;
                ss2 >> label >> inst.name >> inst.libCellName;
                v_inst.emplace_back(inst);
                countInst++;
            }
        }
        else if(label == "NumNets"){ // Netlist (Nets)
            ss >> numNet;
            int countNet = 0;
            while(countNet < numNet){
                getline(file, line);
                if(line=="") continue;
                stringstream ss2(line);
                // add net
                Net net;
                ss2 >> label >> net.name >> net.numPins;
                int countPin = 0;
                while(countPin < net.numPins){ // get pins
                    getline(file, line);
                    if(line=="") continue;
                    replace(line.begin(), line.end(), '/', ' '); // replace '/' to ' '
                    stringstream ss3(line);
                    // add pin
                    Pin pin;
                    ss3 >> label >> pin.instName >> pin.libPinName;
                    net.v_pin.emplace_back(pin);
                    countPin++;
                }
                v_net.emplace_back(net);
                countNet++;
            }
        }
        else if(label == "DieSize"){ // DieSize
            ss >> topDie.ll_x >> topDie.ll_y >> topDie.ur_x >> topDie.ur_y;
            botDie.ll_x = topDie.ll_x;
            botDie.ll_y = topDie.ll_y;
            botDie.ur_x = topDie.ur_x;
            botDie.ur_y = topDie.ur_y;
        }
        else if(label == "TopDieMaxUtil"){ // TopDieMaxUtil
            ss >> topDie.maxUtil;
        }
        else if(label == "BottomDieMaxUtil"){ // BottomDieMaxUtil
            ss >> botDie.maxUtil;
        }
        else if(label == "TopDieRows"){ // TopDieRows
            ss >> topDie.rowStartX >> topDie.rowStartY >> topDie.rowLength >> topDie.rowHeight >> topDie.rowRepeatCount;
        }
        else if(label == "BottomDieRows"){ // BottomDieRows
            ss >> botDie.rowStartX >> botDie.rowStartY >> botDie.rowLength >> botDie.rowHeight >> botDie.rowRepeatCount;
        }
        else if(label == "NumTechnologies"){ // technologies
            ss >> numTech;
            int countTech = 0;
            while(countTech < numTech){
                getline(file, line);
                if(line=="") continue;
                stringstream ss2(line);
                // add tech
                Tech tech;
                ss2 >> label >> tech.name >> tech.numLibCell;
                int countLibCell = 0;
                while(countLibCell < tech.numLibCell){ // get libCells
                    getline(file, line);
                    if(line=="") continue;
                    stringstream ss3(line);
                    // add libCell
                    LibCell libCell;
                    ss3 >> label >> libCell.name >> libCell.sizeX >> libCell.sizeY >> libCell.numLibPin;
                    int countLibPin = 0;
                    while(countLibPin < libCell.numLibPin){ // get libCells
                        getline(file, line);
                        if(line=="") continue;
                        stringstream ss3(line);
                        // add libPin
                        LibPin libPin;
                        ss3 >> label >> libPin.name >> libPin.locationX >> libPin.locationY;
                        libCell.v_libPin.emplace_back(libPin);
                        countLibPin++;
                    }
                    tech.v_libCell.emplace_back(libCell);
                    countLibCell++;
                }
                v_tech.emplace_back(tech);
                countTech++;
            }
        }
        else if(label == "TopDieTech"){ // TopDieTech 
            ss >> topDie.dieTech;
        }
        else if(label == "BottomDieTech"){ // BottomDieTech 
            ss >> botDie.dieTech;
        }
        else if(label == "TerminalSize"){ // TerminalSize
            ss >> terminal.sizeX >> terminal.sizeY;
        }
        else if(label == "TerminalSpacing"){ // TerminalSpacing
            ss >> terminal.spacing;
        }
    }
    cout << BLUE << "[Parser]" << RESET << " - Parsing \"" << fileName << "\""<< GREEN <<" successfully!" << RESET << "\n";
    return _isOK;
}

bool Parser::ok(){
    return _isOK;
}

void Parser::print_info(){
    cout << "numInst = " << numInst << "\n";
    cout << "numNet  = " << numNet << "\n";
    cout << "numTech = " << numTech << "\n";
    for(Inst inst : v_inst){
        cout << "Inst " << inst.name << "(" << inst.libCellName << ")\n";
    }
    for(Net net : v_net){
        cout << "Net " << net.name << "\n";
        for(Pin pin : net.v_pin){
            cout << "  Pin " << pin.instName << "/" << pin.libPinName << "\n";
        }
    }
    for(Tech tech : v_tech){
        cout << "Tech " << tech.name << "\n";
        for(LibCell libCell : tech.v_libCell){
            cout << "  LibCell " << libCell.name << "(" << libCell.sizeX << "*" << libCell.sizeY << ")\n";
            for(LibPin libPin : libCell.v_libPin){
                cout << "    LibPin " << libPin.name << "(" << libPin.locationX << "," << libPin.locationY << ")\n";
            }
        }
    }
    cout << "TopDie:\n";
    cout << "  size = " << topDie.ur_x-topDie.ll_x << "*" << topDie.ur_y-topDie.ll_y << "\n";
    cout << "  maxUtil = " << topDie.maxUtil << "\n";
    cout << "  dieTech = " << topDie.dieTech << "\n";
    cout << "  rows = " << topDie.rowLength << "*" << topDie.rowHeight << "*" << topDie.rowRepeatCount << "\n";
    cout << "BottomDie:\n";
    cout << "  size = " << botDie.ur_x-botDie.ll_x << "*" << botDie.ur_y-botDie.ll_y << "\n";
    cout << "  maxUtil = " << botDie.maxUtil << "\n";
    cout << "  dieTech = " << botDie.dieTech << "\n";
    cout << "  rows = " << botDie.rowLength << "*" << botDie.rowHeight << "*" << botDie.rowRepeatCount << "\n";
    cout << "Terminal: " << terminal.sizeX << "*" << terminal.sizeY << ", spacing=" << terminal.spacing << "\n";
}