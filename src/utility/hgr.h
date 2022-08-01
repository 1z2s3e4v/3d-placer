#ifndef HGR_H
#define HGR_H

#include "color.h"
#include <ctime>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <sstream>
#include <iostream>
using namespace std;

class HGR{
    int _numNet;
    int _numNode;
    vector<string> _vNets; // netNames
    vector<string> _vNodes; // nodeNames
    map<string, int> _mNets; // netName --> netId
    map<string, int> _mNodes; // cellName --> cellId
    vector<vector<int> > _vNetNodes; // netId --> cellIds
    vector<int> _vNodePart; // cellId --> part
    vector<int> _vPartNum; // node num in each part;
    vector<vector<int> > _vPartNodes;

    string _dir;
    string _circuit_name;
public:
    HGR();
    HGR(string); // circuit name
    HGR(string, string); // dir name, circuit name
    void set_dir_and_circuit_name(string, string); // dir name, circuit name
    void add_net(string); // name, degree
    void add_node(string, string); // net, node name

    void write_hgr();
    void read_part_result(int k); 
    int get_part_result(string); // cell name --> part
    int get_part_size(int);
    vector<int>& get_nodes_by_part(int part);
};

#endif