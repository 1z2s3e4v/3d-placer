#ifndef PARSER_H
#define PARSER_H

#include "../utility/color.h"

#include <iostream>
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
    string instName;
    string libCellName;
};
struct Net{
    string netName;
    int numPins;
    vector<Pin> v_pin;
};
class Parser{
public:
    Parser();
    Parser(string); // input fileName
    bool read_file(string); // input fileName

    bool ok();
    void print_info();
private:
    bool _isOK = true;
    int countReadInfo = 0;

    int numInstance;
    int numNet;
    vector<Inst> v_inst;
    vector<Net> v_net;
};

#endif