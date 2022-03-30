#ifndef DM_H
#define DM_H
#include <ctime>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <time.h>
#include <numeric>
#include <map>
#include <sstream>
#include <set>
#include <math.h>
#include "module.h"
#include "../utility/paramHdl.h"
#include "../parser/parser.h"
#include "../placer/placer.h"
class DmMgr_C{
    ParamHdl_C paramHdl;

    vector<string> _vTechName; // Tech Name Table: _vTech[Techid]=TechName
    unordered_map<string, int> _mTechName2Id; // Tech Name Table: _mTechName2Id[TechName]=TechId
    unordered_map<string, CellLib_C*> _mCellLib; // master cell type table
    vector<CellLib_C*> _vCellLib; // master cell type list
    Chip_C* _pChip;
    Design_C* _pDesign;
    Placer_C* _pPlacer;
    int _total_WL = 0;
    clock_t _tStart;
public:
    DmMgr_C();
    DmMgr_C(Parser_C&, ParamHdl_C&, clock_t);
    void init();
    void run();
    void print_info();
    void dump_info();
    void output_result(string fileName);
};

#endif