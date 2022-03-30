#ifndef PLACER_H
#define PLACER_H
#include <tuple>
#include <vector>
#include <unordered_map>
#include "../dataModel/module.h"
using namespace std;

class Placer_C{
    Chip_C* _pChip;
    Design_C* _pDesign;
    unordered_map<Cell_C*, Pos> _mCellBestPos;
    vector<Cell_C*> _vCell;
    vector<Net_C*> _vNet;
    clock_t _tStart;
public:
    Placer_C();
    Placer_C(Chip_C*, Design_C*, clock_t);

    void clear();
};

#endif