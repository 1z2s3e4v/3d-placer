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

struct ParserPin{
    string instName;
    string libPinName;
};
struct ParserInst{
    string name;
    string libCellName;
};
struct ParserNet{
    string name;
    int numPins;
    vector<ParserPin> v_pin;
};

struct ParserDie{
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

struct ParserLibPin{
    string name;
    int locationX;
    int locationY;
};

struct ParserLibCell{
    string name;
    int sizeX;
    int sizeY;
    int numLibPin;
    vector<ParserLibPin> v_libPin;
};

struct ParserTech{
    string name;
    int numLibCell;
    vector<ParserLibCell> v_libCell;
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
    vector<ParserInst>& get_insts();
    vector<ParserNet>& get_nets();
    vector<ParserTech>& get_techs();
    ParserDie get_top_die_info();
    ParserDie get_bot_die_info();
    Terminal get_terminal_info();

    bool ok();
    void print_info();
private:
    bool _isOK = true;
    int countReadInfo = 0;

    int numInst;
    int numNet;
    vector<ParserInst> v_inst;
    vector<ParserNet> v_net;
    ParserDie topDie;
    ParserDie botDie;
    Terminal terminal;

    int numTech;
    vector<ParserTech> v_tech;
};

#endif