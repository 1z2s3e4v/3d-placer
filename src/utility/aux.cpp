#include "aux.h"
#include <fstream>
#include <sstream>


AUX::AUX(){}
AUX::AUX(string name){
    numTerminals = 0;
    numPins = 0;
    set_dir_and_circuit_name("./", name);
}
AUX::AUX(string dir, string name){
    numTerminals = 0;
    numPins = 0;
    set_dir_and_circuit_name(dir, name);
}
void AUX::set_circuit_name(string name){
    numTerminals = 0;
    numPins = 0;
    _circuit_name = name;
}
void AUX::set_dir_and_circuit_name(string dir, string name){
    if(dir[dir.size()-1] != '/')
        dir = dir + "/";
    _dir = dir;
    _circuit_name = name;
}
bool AUX::check_net_exist(string netName){
    return (_mNets.find(netName) != _mNets.end());
}
void AUX::remove_open_net(){
    auto it = _mNets.begin();
    for(; it != _mNets.end(); ) {
        if (it->second.vPins.size() == 1) {
            it = _mNets.erase(it);
            numPins--;
        } else {
            ++it;
        }
    }
}

void AUX::write_files(){
    write_aux();
    write_nets();
    write_nodes();
    write_pl();
    write_scl();
    write_wts();
}
void AUX::write_aux(){
    string fileName = _dir + _circuit_name + ".aux";
    ofstream fout(fileName);
    fout << "RowBasedPlacement :  " 
        << _circuit_name << ".nodes  "
        << _circuit_name << ".nets  "
        << _circuit_name << ".wts  "
        << _circuit_name << ".pl  "
        << _circuit_name << ".scl"
        << "\n";
    fout.close();
}
void AUX::write_nodes(){
    string fileName = _dir + _circuit_name + ".nodes";
    ofstream fout(fileName);
    time_t ttime = time(0);
    char* dt = ctime(&ttime);
    fout << "UCLA nodes 1.0" << "\n";
    fout << "# Created\t:\t" << dt << "\n";
    fout << "# User\t\t:\t" << getenv("USER") << "\n";
    fout << "\n";
    fout << "NumNodes :\t\t" << _vNodes.size() << "\n";
    fout << "NumTerminals :\t" << numTerminals << "\n";
    for(AuxNode& node : _vNodes){
        fout << "\t" << node.name << "\t" << node.width << "\t" << node.height << "\n";
    }
    fout.close();
}
void AUX::write_nets(){
    string fileName = _dir + _circuit_name + ".nets";
    ofstream fout(fileName);
    time_t ttime = time(0);
    char* dt = ctime(&ttime);
    fout << "UCLA nets 1.0" << "\n";
    fout << "# Created\t:\t" << dt << "\n";
    fout << "# User\t\t:\t" << getenv("USER") << "\n";
    fout << "\n";
    fout << "NumNets :\t" << _mNets.size() << "\n";
    fout << "NumPins :\t" << numPins << "\n";
    fout << "\n";
    for(auto& net : _mNets){
        fout << "NetDegree : " << net.second.degree << "\t" << net.second.name << "\n";
        for(AuxPin& pin : net.second.vPins){
            fout << "\t" << pin.cellName << "\t" << pin.IO << " : " << pin.x_offset << "\t" << pin.y_offset << "\n";
        }
    }
    fout.close();
}
void AUX::write_wts(){
    string fileName = _dir + _circuit_name + ".wts";
    ofstream fout(fileName);
    time_t ttime = time(0);
    char* dt = ctime(&ttime);
    fout << "UCLA wts 1.0" << "\n";
    fout << "# Created\t:\t" << dt << "\n";
    fout << "# User\t\t:\t" << getenv("USER") << "\n";
    fout << "\n";
    fout.close();
}
void AUX::write_pl(){
    string fileName = _dir + _circuit_name + ".pl";
    ofstream fout(fileName);
    time_t ttime = time(0);
    char* dt = ctime(&ttime);
    fout << "UCLA pl 1.0" << "\n";
    fout << "# Created\t:\t" << dt << "\n";
    fout << "# User\t\t:\t" << getenv("USER") << "\n";
    fout << "\n";
    for(AuxNode& node : _vNodes){
        fout << node.name << "\t" << node.x << "\t" << node.y << "\t: N";
        if(node.fixed) fout << " /FIXED";
        fout << "\n";
    }
    fout.close();
}
void AUX::write_scl(){
    string fileName = _dir + _circuit_name + ".scl";
    ofstream fout(fileName);
    time_t ttime = time(0);
    char* dt = ctime(&ttime);
    fout << "UCLA scl 1.0" << "\n";
    fout << "# Created\t:\t" << dt << "\n";
    fout << "# User\t\t:\t" << getenv("USER") << "\n";
    fout << "\n";
    fout << "NumRows : " << _vRows.size() << "\n\n";
    int count_row = 0;
    for(AuxRow& row : _vRows){
        int coord = row.height * count_row;
        fout << "CoreRow Horizontal" << "\n";
        fout << "  Coordinate     :   " << coord << "\n";
        fout << "  Height         :   " << row.height << "\n";
        fout << "  Sitewidth      :   " << 1 << "\n";
        fout << "  Sitespacing    :   " << 1 << "\n";
        fout << "  Siteorient     :   " << 0 << "\n";
        fout << "  Sitesymmetry   :   " << 0 << "\n";
        fout << "  SubrowOrigin   :   " << 0 << "\tNumSites  :  " << row.width << "\n";
        fout << "End\n";
        ++count_row;
    }
    fout.close();
}

void AUX::add_node(string name, int w, int h, int x, int y, int isTerminl){
    AuxNode node;
    node.name = name;
    node.width = w;
    node.height = h;
    node.x = x;
    node.y = y;
    node.fixed = isTerminl;
    if(isTerminl) ++numTerminals;
    _vNodes.emplace_back(node);
}
void AUX::add_net(string name){
    AuxNet net;
    net.degree = 0;
    net.name = name;
    _mNets.emplace(name, net);
}
void AUX::add_pin(string netName, string cellName, char IO, int x_offset, int y_offset){
    add_pin(netName, cellName, IO, float(x_offset), float(y_offset));
}
void AUX::add_pin(string netName, string cellName, char IO, float x_offset, float y_offset){
    AuxPin pin;
    pin.cellName = cellName;
    pin.x_offset = x_offset;
    pin.y_offset = y_offset;
    pin.IO = IO;
    _mNets[netName].vPins.emplace_back(pin);
    _mNets[netName].degree++;
    ++numPins;
}
void AUX::add_row(int w, int h){
    AuxRow row;
    row.width = w;
    row.height = h;
    _vRows.emplace_back(row);
}
void AUX::set_default_rows(int w, int h, int n){
    for(int i=0;i<n;++i){
        add_row(w,h);
    }
}
void AUX::read_pl(vector<AuxNode>& vPlacedNode){
    string fileName = _circuit_name + ".out.lg.pl";
    read_pl(fileName, vPlacedNode);
}
void AUX::read_pl(string fileName, vector<AuxNode>& vPlacedNode){
    ifstream fin(fileName);
    string line;
    int count_line = 0;
    while(getline(fin, line)){
        count_line++;
        if(count_line == 1 || line[0] == '#' || line == "") continue;
        stringstream ss(line);
        string name;
        int x,y;
        ss >> name >> x >> y;
        AuxNode node;
        node.name = name;
        node.x = x;
        node.y = y;
        vPlacedNode.emplace_back(node);      
        //cout << "AUX: " << vPlacedNode.size() << " - " << node.name << " (" << node.x << "," <<node.y << ")\n";
    }
    fin.close();
}