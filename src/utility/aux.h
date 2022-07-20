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
#include <cmath>
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
    int type; // (0=movable, 1=terminal, 2=terminal_NI)
};
struct AuxRow{
    int Coordinate; // ll_y
    int Height; // height
    int Sitewidth; // 1
    int Sitespacing; // 1
    int Siteorient; // 0
    int Sitesymmetry; // 0
    int SubrowOrigin; // ll_x
    int NumSites; // width
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

    void add_node(string, int, int, int, int, int); // name, width, height, x, y, type(0=movable, 1=terminal, 2=terminal_NI)
    void add_net(string); // name, degree
    void add_pin(string, string, char, int, int); // netName, cellName, I/O, x_offest, y_offset
    void add_pin(string, string, char, float, float); // netName, cellName, I/O, x_offest, y_offset
    void set_default_rows(int,int,int); // row_width * row_height * row_num
    void add_row(int,int,int,int,int,int,int,int); // Coordinate, Height, Sitewidth, Sitespacing, Siteorient, Sitesymmetry, SubrowOrigin, NumSites

    bool read_pl(vector<AuxNode>& vPlacedNode);
    bool read_pl(string fileName, vector<AuxNode>& vPlacedNode);
};

#endif