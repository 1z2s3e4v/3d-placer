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
            ss >> numInstance;
            int countInst = 0;
            while(countInst < numInstance){
                getline(file, line);
                if(line=="") continue;
                stringstream ss2(line);
                // add inst
                Inst inst;
                ss2 >> label >> inst.instName >> inst.libCellName;
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
                ss2 >> label >> net.netName >> net.numPins;
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

        }
        else if(label == "TopDieMaxUtil"){ // TopDieMaxUtil

        }
        else if(label == "BottomDieMaxUtil"){ // BottomDieMaxUtil

        }
        else if(label == "TopDieRows"){ // TopDieRows

        }
        else if(label == "BottomDieRows"){ // BottomDieRows

        }
        else if(label == "NumTechnologies"){ // technologies

        }
        else if(label == "TopDieTech"){ // TopDieTech 

        }
        else if(label == "BottomDieTech"){ // BottomDieTech 

        }
        else if(label == "TerminalSize"){ // TerminalSize

        }
        else if(label == "TerminalSpacing"){ // TerminalSpacing

        }
    }
    cout << BLUE << "[Parser]" << RESET << " - Parsing \"" << fileName << "\""<< GREEN <<" successfully!" << RESET << "\n";
    print_info();
    return _isOK;
}

bool Parser::ok(){
    return _isOK;
}

void Parser::print_info(){
    for(Inst inst : v_inst){
        cout << "Inst " << inst.instName << "(" << inst.libCellName << ")\n";
    }
    for(Net net : v_net){
        cout << "Net " << net.netName << "\n";
        for(Pin pin : net.v_pin){
            cout << "  Pin " << pin.instName << "/" << pin.libPinName << "\n";
        }
    }
}