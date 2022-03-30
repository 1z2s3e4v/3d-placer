#ifndef MODULE_H
#define MODULE_H
#include <iostream>
#include <tuple>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <assert.h>
using namespace std;

//typedef tuple<int, int, int> Pos; // layer(tier/die), row, col
struct Pos;
string pos3d2str(Pos pos);
string pos2d2str(Pos pos);
string pos2str(Pos pos);

class CellLib_C;
class Pin_C;
class Row_C;
class Die_C;
class Net_C;
class Cell_C;
class Chip_C;
class Design_C;

struct BondBox{
    int left_x;
    int right_x;
    int bottom_y;
    int top_y;
};
struct RowFragment{
    vector<Cell_C*> _vCells;
    int startX;
    int endX;
};
struct Pos{
    int x;
    int y;
    int z;
    Pos(){}
    Pos(int _z,int _x, int _y): x(_x), y(_y), z(_z){}
    Pos(int _x, int _y): x(_x), y(_y){}
    string pos3d2str() {return "("+to_string(z)+","+to_string(x)+","+to_string(y)+")";}
    string pos2d2str() {return "("+to_string(x)+","+to_string(y)+")";}
    string to_str() {return pos3d2str();}
    string xy_to_str() {return pos2d2str();}
    string xyz_to_str() {return pos3d2str();}
};

class CellLib_C{
    string _name;
    int _numTech;
    vector<int> _techW; // sizeX in each tech
    vector<int> _techH; // sizeY in each tech
    vector< vector<Pos> > _vTechLibPinOffset; // pin_offset XY in each tech
    unordered_map<string, int> _mPinName2Idx;
    vector<string> _vPinName;
public:
    CellLib_C();
    CellLib_C(string, int, int); // name, pin_num, tech_num
    void add_pin(int, string, Pos); // tech_id, Pos(offsetX, offsetY)
    void add_pin(int, string, int, int); // tech_id, offsetX, offsetY
    void set_size(int, int, int); // tech_id, sizeX, sizeY
    string get_name();
    int get_tech_num();
    int get_pin_num();
    string get_pin_name(int); // pinId
    int get_pin_id(string); // get pin id with pinName
    int get_cell_width(int); // get_cell_width with techId
    int get_cell_height(int); // get_cell_height with techId
    Pos get_pin_offset(int,int); // get pin offset with techId and pinId
};

class Row_C{
    list<RowFragment> _vFragments;
    vector<Cell_C*> _vCells;
    int _id; // y = height*_id
    int _width;
    int _height;
    Die_C* _die;
public:
    Row_C();
    Row_C(int,int,int); // id, width, height
    Row_C(int,int,int, Die_C*); // id, width, height, die
};

class Die_C{
    int _id;
    int _techId;
    int _sizeX;
    int _sizeY;
    // die max utility
    int _maxUtil;
    // die rows
    int _rowHeight;
    vector<Row_C*> _vRows;
    // die technology
public:
    Die_C();
    Die_C(int, int, int, int, int); // sizeX, sizeY, maxUtil, techId, rowHeight
    Die_C(int, int, int, int, int, int); // id, sizeX, sizeY, maxUtil, techId, rowHeight
    int get_id();
    int get_techId();
    int get_max_util();
    int get_row_height();
    int get_row_num();
};

class Pin_C{
    int _id; // pinId in _masterCell
	Cell_C* _cell;
	vector<Net_C*> _vNets;
public:
    Pin_C();
    Pin_C(int id, Cell_C *cell); // for cell_pin
    void add_net(Net_C*);
    int get_id();
    string get_name();
    Pos get_pos(); // real pos
    int get_x(); // real coordinate
    int get_y(); // real coordinate
    int get_net_num();
    Net_C* get_net(int);
    vector<Net_C*>& get_nets();
    Cell_C* get_cell();
};

class Net_C{
    string _name;
    int _id;
	int _HPWL;
    
	vector<Pin_C*> _vPins;
public:
    Net_C();
	Net_C(string name);
    void set_id(int);
    void add_pin(Pin_C*); // cellName, pinName
    string get_name() const;
    int get_id();
    int get_HPWL() const;
    int get_pin_num();
    Pin_C* get_pin(int);
    vector<Pin_C*>& get_pins();
};

class Cell_C{
	string _name;
    int _id;
    Pos _pos = Pos(0,0,0);
	CellLib_C* _masterCell;
	vector<Pin_C*> _vPins;
    int _dieId;
    Die_C* _die;
    Row_C* _row;
public:
    Cell_C();
	Cell_C(string name, CellLib_C* master_cell);
    void set_id(int);
    void set_pos(Pos);
    void set_xy(Pos);
    void set_die(Die_C*);
    string get_name();
    int get_id();
    int get_width();
    int get_height();
    int get_width(int); // get width with techId
    int get_height(int); // get height with techId
    int get_pin_num();
    Pin_C* get_pin(int); // get pin with pinId
    Pin_C* get_pin(string); // get pin with pinName
    vector<Pin_C*>& get_pins();
    Pos get_pos();
    int get_posX();
    int get_posY();
    int get_posZ(); // 0=topDie, 1=botDie
    CellLib_C* get_master_cell();
    int get_die_techId();
    int get_dieId();
};

class Chip_C{
    vector<Die_C*> _vDie;
    // die size (top and bot are same)
    int _sizeX;
    int _sizeY;
    // terminal
    int _ballSizeX;
    int _ballSizeY;
    int _ballSpace;
public:
    Chip_C();
    Chip_C(int, int, int); // sizeX, sizeY, dieNum
    void set_chip_size(int, int); // sizeX, sizeY
    void set_die(int, int, int, int); // dieId, maxUtil, techId, rowHeight
    void set_ball(int,int,int); // ballSizeX, ballSizeY, ballSpace
    int get_die_num();
    Die_C* get_die(int);
    int get_width();
    int get_height();
    int get_ball_width();
    int get_ball_height();
    int get_ball_spacing();
};

class Design_C{
    vector<Net_C*> _vNets;
    vector<Cell_C*> _vCells;
    map<string, Net_C*> _mNets;
    map<string, Cell_C*> _mCells;

public:
    Design_C();
    void add_net(Net_C* net);
    void add_cell(Cell_C* cell);
    int get_cell_num();
    int get_net_num();
    Cell_C* get_cell(string); // get cell with cellName
    Cell_C* get_cell(int); // get cell with cellId
    Net_C* get_net(string); // get net with netName
    Net_C* get_net(int); // get net with netId
};



#endif