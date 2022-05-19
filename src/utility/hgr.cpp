#include "hgr.h"

HGR::HGR(){}
HGR::HGR(string name){
    _numNet = 0;
    _numNode = 0;
    set_dir_and_circuit_name("./", name);
}
HGR::HGR(string dir, string name){
    _numNet = 0;
    _numNode = 0;
    set_dir_and_circuit_name(dir, name);
}
void HGR::set_dir_and_circuit_name(string dir, string name){
    if(dir[dir.size()-1] != '/')
        dir = dir + "/";
    _dir = dir;
    _circuit_name = name;
}
void HGR::add_net(string netName){
    if(_mNets.find(netName) == _mNets.end()){
        _vNets.emplace_back(netName);
        _mNets.emplace(netName,_numNet);
        _vNetNodes.emplace_back(vector<int>());
        _numNet++;
    }
    else{
        //cout << BLUE << "[HGR]" << RESET << " - Warning: Net \'" << netName << "\' has been added.\n";
    }
}
void HGR::add_node(string netName, string nodeName){
    if(_mNodes.find(nodeName) == _mNodes.end()){
        _vNodes.emplace_back(nodeName);
        _mNodes.emplace(nodeName, _numNode);
        _numNode++;
    }
    int netId = _mNets[netName];
    int nodeId = _mNodes[nodeName];
    if(find(_vNetNodes[netId].begin(),_vNetNodes[netId].end(),nodeId) == _vNetNodes[netId].end()){
        _vNetNodes[netId].emplace_back(nodeId);
    }
    else{
        //cout << BLUE << "[HGR]" << RESET << " - Warning: Node \'" << nodeName << "\' has been added in Net \'" << netName << "\'.\n";
    }
}

void HGR::write_hgr(){
    string fileName = _dir + _circuit_name + ".hgr";
    ofstream fout(fileName);
    fout << _numNet << " " << _numNode << "\n";
    for(int i=0;i<_vNets.size();++i){
        for(int j=0;j<_vNetNodes[i].size();++j){
            fout << _vNetNodes[i][j]+1 << " ";
        }
        fout << "\n";
    }
    fout.close();
}
void HGR::read_part_result(int k){
    _vNodePart.clear();
    _vNodePart.resize(_numNode,-1);
    _vPartNum.clear();
    _vPartNum.resize(k,0);
    string fileName = _dir + _circuit_name + ".hgr.part." + to_string(k);
    ifstream fin(fileName);
    string line;
    int i=0;
    while(getline(fin, line)){
        stringstream ss(line);
        if(i < _vNodePart.size()){
            int part;
            ss >> part;
            _vNodePart[i] = part;
            ++_vPartNum[part];
            ++i;
        }
        else
            cout << BLUE << "[HGR]" << RESET << " - Warning: node-partition result out-of-range (node size = " << _vNodePart.size() << ").\n"; 
    }
}
int HGR::get_part_result(string nodeName){
    int part = -1;
    if(_mNodes.find(nodeName) != _mNodes.end())
        part = _vNodePart[_mNodes[nodeName]];
    else
        cout << BLUE << "[HGR]" << RESET << " - Warning: Cannot find Node \'" << nodeName << "\'.\n";
    return part;
}
int HGR::get_part_size(int partId){
    return _vPartNum[partId];
}