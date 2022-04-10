#ifndef PLACER_H
#define PLACER_H
#include <tuple>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <climits>
#include <algorithm>
#include <iostream>
#include <numeric>
#include "../utility/color.h"
#include "../dataModel/module.h"

using namespace std;

class Placer_C{
    Chip_C* _pChip;
    Design_C* _pDesign;
    vector<Pos> _vCellBestPos;
    vector<Cell_C*> _vCell;
    vector<Net_C*> _vNet;
    clock_t _tStart;
public:
    Placer_C();
    Placer_C(Chip_C*, Design_C*, clock_t);
    
    void run();
    void init_place();
    void clear();
};

#endif