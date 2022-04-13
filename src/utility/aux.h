#ifndef AUX_H
#define AUX_H

#include <ctime>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
using namespace std;

struct AuxPin{
    string cellName;
    float x_offset;
    float y_offset;
    char IO;
};

struct AuxNet{
    int degree;
    string name;
    vector<AuxPin> vPins;
};
struct AuxNode{
    string name;
    int width;
    int height;
    int x;
    int y;
    int fixed;
};
struct AuxRow{
    int width;
    int height;
};

class AUX{
    int numTerminals;
    int numPins;
    map<string,AuxNet> _mNets;
    vector<AuxNode> _vNodes;
    vector<AuxRow> _vRows;
    string _dir;
    string _circuit_name;
public:
    AUX(); 
    AUX(string); // circuit name
    AUX(string, string); // dir name, circuit name
    void set_circuit_name(string);
    void set_dir_and_circuit_name(string, string);
    bool check_net_exist(string);
    void remove_open_net();

    void write_files(); 
    void write_aux();
    void write_nets();
    void write_nodes();
    void write_pl();
    void write_scl();
    void write_wts();

    void add_node(string, int, int, int, int, int); // name, width, height, x, y, isTerminal
    void add_net(string); // name, degree
    void add_pin(string, string, char, int, int); // netName, cellName, I/O, x_offest, y_offset
    void add_pin(string, string, char, float, float); // netName, cellName, I/O, x_offest, y_offset
    void set_default_rows(int,int,int); // row_width * row_height * row_num
    void add_row(int,int); // row_width * row_height

    void read_pl(vector<AuxNode>& vPlacedNode);
    void read_pl(string fileName, vector<AuxNode>& vPlacedNode);
};

#endif