#ifndef BIN_H
#define BIN_H
#include <climits>
#include <iostream>
#include <tuple>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <assert.h>
#include <cmath>
#include "../dataModel/module.h"

using namespace std;

class BNet_C;

class Bin_C{

    int _id;
    int _part; // index of part
    Pos _center;
    int _die;
    int _width;
    int _height;
    vector<int> _cellIdList;
    vector<BNet_C*> _bnetList;

public:

    Bin_C(int id) {_id = id;}
    ~Bin_C()  { }

    // access functions
    int get_id() const {return _id;}
    int get_part() const {return _part;}
    Pos get_center() const {return _center;}
    int get_die() const {return _die;}
    int get_width() const {return _width;}
    int get_height() const {return _height;}
    vector<int> get_cellIdList() const {return _cellIdList;}
    vector<BNet_C*> get_bnetList() const {return _bnetList;}


    // set functions
    void set_id(int id) {_id = id;}
    void set_part(int part) {_part = part;}
    void set_center(Pos center) {_center = center;}
    void set_die(int die) {_die = die;}
    void set_width(int width) {_width = width;}
    void set_height(int height) {_height = height;}
    void set_cellIdList(vector<int> cellIdList) {_cellIdList = cellIdList;}
    void set_bnetList(vector<BNet_C*> bnet) {_bnetList = bnet;}
    

    // modify functions
    void add_cell(int cellId) {_cellIdList.push_back(cellId);}
    void add_bnet(BNet_C* bnet) {_bnetList.push_back(bnet);}
    
};

class BNet_C{

    string _id;
    int _weight;
    vector<Pos> _vll; // (min_x,min_y) of bounding box
    vector<Pos> _vur; // (max_x,max_y) of bounding box
    vector<Bin_C*> _vbin;
    Pos _ballPos; 
    int _bin_num_top;

    
    // vector<unordered_set<Pin_C*> > _vPinInEachDie; // ref to module.cpp:216 and 175
    

public:

    BNet_C(string id) {_id = id;}
    ~BNet_C()  { }
    
    // access functions
    string get_id() const {return _id;}
    vector<Bin_C*> get_vbin() {return _vbin;}
    bool is_cross_net(); // crossing die
    int get_HPWL(int);
    int get_total_HPWL();

    // set functions
    void set_binNumTop(int bin_num_top) {_bin_num_top = bin_num_top;}
    void set_weight(int weight) {_weight = weight;}
    void set_vbin(vector<Bin_C*> vbin) {_vbin = vbin;}
    
    // modify functions
    void incr_weight() {_weight += 1;}
    void add_bin(Bin_C* bin) {_vbin.push_back(bin);}
    void update_bbox();
    void update_ballPos();
    
};


#endif