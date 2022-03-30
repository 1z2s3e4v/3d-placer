#ifndef PARSER_H
#define PARSER_H

#include "../utility/color.h"

#include <iostream>
#include <boost/format.hpp>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
using namespace std;

struct Pin{
    string instName;
    string libPinName;
};
struct Inst{
    string name;
    string libCellName;
};
struct Net{
    string name;
    int numPins;
    vector<Pin> v_pin;
};

struct Die{
    // die size (top and bot are same)
    int ll_x; // LowerLeft
    int ll_y;
    int ur_x; // UpperRight
    int ur_y;
    // die max utility
    int maxUtil;
    // die rows
    int rowStartX;
    int rowStartY;
    int rowLength;
    int rowHeight;
    int rowRepeatCount;
    // die technology
    string dieTech;
};

struct LibPin{
    string name;
    int locationX;
    int locationY;
};

struct LibCell{
    string name;
    int sizeX;
    int sizeY;
    int numLibPin;
    vector<LibPin> v_libPin;
};

struct Tech{
    string name;
    int numLibCell;
    vector<LibCell> v_libCell;
};

struct Terminal{
    int sizeX;
    int sizeY;
    int spacing;
};
class Parser_C{
public:
    Parser_C();
    Parser_C(string); // input fileName
    bool read_file(string); // input fileName
    vector<Inst>& get_insts();
    vector<Net>& get_nets();
    vector<Tech>& get_techs();
    Die get_top_die_info();
    Die get_bot_die_info();
    Terminal get_terminal_info();

    bool ok();
    void print_info();
private:
    bool _isOK = true;
    int countReadInfo = 0;

    int numInst;
    int numNet;
    vector<Inst> v_inst;
    vector<Net> v_net;
    Die topDie;
    Die botDie;
    Terminal terminal;

    int numTech;
    vector<Tech> v_tech;
};

#endif