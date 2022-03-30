#ifndef AUX_H
#define AUX_H

#include <ctime>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <sstream>
using namespace std;

struct Pin{
    string cellName;
    float x_offset;
    float y_offset;
    char IO;
};

struct Net{
    int degree;
    string name;
    vector<Pin> vPins;
};
struct Node{
    string name;
    int width;
    int height;
    int x;
    int y;
    int fixed;
};
struct Row{
    int width;
    int height;
};

class AUX{
    int numTerminals;
    int numPins;
    map<string,Net> _mNets;
    vector<Node> _vNodes;
    vector<Row> _vRows;
    string _circuit_name;
public:
    AUX(); 
    AUX(string); // circuit name
    void set_circuit_name(string);

    void write_files(); 
    void write_aux();
    void write_nets();
    void write_nodes();
    void write_pl();
    void write_scl();
    void write_wts();

    void add_node(string, int, int, int, int, int); // name, width, height, x, y, isTerminal
    void add_net(string, int); // name, degree
    void add_pin(string, string, char, int, int); // netName, cellName, I/O, x_offest, y_offset
    void add_pin(string, string, char, float, float); // netName, cellName, I/O, x_offest, y_offset
    void set_default_rows(int,int,int); // row_width * row_height * row_num
    void add_row(int,int); // row_width * row_height

    void read_pl(vector<Node>& vPlacedNode);
    void read_pl(string fileName, vector<Node>& vPlacedNode);
};

#endif