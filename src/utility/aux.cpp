#include "aux.h"
#include <fstream>
#include <sstream>

AUX::AUX(){}
AUX::AUX(string name){
    set_circuit_name(name);
}
void AUX::set_circuit_name(string name){
    _circuit_name = name;
}

void AUX::write_files(){
    void write_aux();
    void write_nets();
    void write_nodes();
    void write_pl();
    void write_scl();
    void write_wts();
}
void AUX::write_aux(){
    string fileName = _circuit_name + ".aux";
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
    string fileName = _circuit_name + ".nodes";
    ofstream fout(fileName);
    time_t ttime = time(0);
    char* dt = ctime(&ttime);
    fout << "UCLA nodes 1.0" << "\n";
    fout << "# Created\t:\t" << dt << "\n";
    fout << "# User\t\t:\t" << getenv("USER") << "\n";
    fout << "\n";
    fout << "NumNodes :\t\t" << _vNodes.size() << "\n";
    fout << "NumTerminals :\t" << numTerminals << "\n";
    for(Node& node : _vNodes){
        fout << "\t" << node.name << "\t" << node.width << "\t" << node.height << "\n";
    }
    fout.close();
}
void AUX::write_nets(){
    string fileName = _circuit_name + ".nodes";
    ofstream fout(fileName);
    time_t ttime = time(0);
    char* dt = ctime(&ttime);
    fout << "UCLA nets 1.0" << "\n";
    fout << "# Created\t:\t" << dt << "\n";
    fout << "# User\t\t:\t" << getenv("USER") << "\n";
    fout << "\n";
    fout << "NumNets :\t\t" << _mNets.size() << "\n";
    fout << "NumPins :\t" << numPins << "\n";
    fout << "\n";
    for(auto& net : _mNets){
        fout << "NetDegree : " << net.second.degree << "\t" << net.second.name << "\n";
        for(Pin& pin : net.second.vPins){
            fout << "\t" << pin.cellName << "\t" << pin.IO << " : " << pin.x_offset << "\t" << pin.y_offset << "\n";
        }
    }
    fout.close();
}
void AUX::write_wts(){
    string fileName = _circuit_name + ".nodes";
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
    string fileName = _circuit_name + ".nodes";
    ofstream fout(fileName);
    time_t ttime = time(0);
    char* dt = ctime(&ttime);
    fout << "UCLA pl 1.0" << "\n";
    fout << "# Created\t:\t" << dt << "\n";
    fout << "# User\t\t:\t" << getenv("USER") << "\n";
    fout << "\n";
    for(Node& node : _vNodes){
        fout << node.name << "\t" << node.x << "\t" << node.y << "\t: N";
        if(node.fixed) fout << " /FIXED";
        fout << "\n";
    }
    fout.close();
}
void AUX::write_scl(){
    string fileName = _circuit_name + ".nodes";
    ofstream fout(fileName);
    time_t ttime = time(0);
    char* dt = ctime(&ttime);
    fout << "UCLA scl 1.0" << "\n";
    fout << "# Created\t:\t" << dt << "\n";
    fout << "# User\t\t:\t" << getenv("USER") << "\n";
    fout << "\n";
    fout << "NumRows : " << _vRows.size();
    int count_row = 0;
    for(Row& row : _vRows){
        int coord = row.height * count_row;
        fout << "CoreRow Horizontal" << "\n";
        fout << " Coordinate     :   " << coord << "\n";
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
    Node node;
    node.name = name;
    node.width = w;
    node.height = h;
    node.x = x;
    node.y = y;
    node.fixed = isTerminl;
    if(isTerminl) ++numTerminals;
    _vNodes.emplace_back(node);
}
void AUX::add_net(string name, int degree){
    Net net;
    net.name = name;
    net.degree = degree;
    _mNets.emplace(name, net);
}
void AUX::add_pin(string netName, string cellName, char IO, int x_offset, int y_offset){
    add_pin(netName, cellName, IO, float(x_offset), float(y_offset));
}
void AUX::add_pin(string netName, string cellName, char IO, float x_offset, float y_offset){
    Pin pin;
    pin.cellName = cellName;
    pin.x_offset = x_offset;
    pin.y_offset = y_offset;
    pin.IO = IO;
    _mNets[netName].vPins.emplace_back(pin);
    ++numPins;
}
void AUX::add_row(int w, int h){
    Row row;
    row.width = w;
    row.height = h;
    _vRows.emplace_back(row);
}
void AUX::set_default_rows(int w, int h, int n){
    for(int i=0;i<n;++i){
        add_row(w,h);
    }
}
void AUX::read_pl(vector<Node>& vPlacedNode){
    string fileName = _circuit_name + ".out.lg.pl";
    read_pl(fileName, vPlacedNode);
}
void AUX::read_pl(string fileName, vector<Node>& vPlacedNode){
    ifstream fin(fileName);
    string line;
    int count_line = 0;
    while(getline(fin, line)){
        if(line[0] == '#' || line == "" || count_line == 0) continue;
        stringstream ss(line);
        string name;
        int x,y;
        ss >> name >> x >> y;
        Node node;
        node.name = name;
        node.x = x;
        node.y = y;
        vPlacedNode.emplace_back(node);
    }
    fin.close();
}