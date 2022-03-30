#include "module.h"

//string pos3d2str(Pos pos) {return "("+to_string(get<0>(pos))+","+to_string(get<1>(pos))+","+to_string(get<2>(pos))+")";}
//string pos2d2str(Pos pos) {return "("+to_string(get<1>(pos))+","+to_string(get<2>(pos))+")";}

//-----------------------------------------------------------------------------------------------------//
CellLib_C::CellLib_C(){}
CellLib_C::CellLib_C(string name, int pin_num, int tech_num){
    _name = name;
    _techW.resize(tech_num, 0);
    _techH.resize(tech_num, 0);
    _vTechLibPinOffset.resize(tech_num, vector<Pos>(pin_num));
    _vPinName.clear();
}
void CellLib_C::add_pin(int tech_id, string pinName, Pos offset){
    assert(tech_id < _vTechLibPinOffset.size());
    // find the id for the pin. If not exist than push_back(pinName). Then set pin_offset.
    int pinId;
    auto it = _mPinName2Idx.find(pinName);
    if(it == _mPinName2Idx.end()){
        pinId = _vPinName.size();
        _mPinName2Idx[pinName] = pinId;
        _vPinName.emplace_back(pinName);
    } else{
        pinId = it->second;
    }
    _vTechLibPinOffset[tech_id][pinId] = offset;
}
void CellLib_C::add_pin(int tech_id, string pinName, int offsetX, int offsetY){
    add_pin(tech_id, pinName, Pos(offsetX, offsetY));
}
void CellLib_C::set_size(int tech_id, int sizeX, int sizeY){
    assert(tech_id < _techW.size() && tech_id < _techH.size());
    _techW[tech_id] = sizeX;
    _techH[tech_id] = sizeY;
}
int CellLib_C::get_pin_num(){
    return _vPinName.size();
}
string CellLib_C::get_pin_name(int pinId){
    return _vPinName[pinId];
}
int CellLib_C::get_pin_id(string pinName){
    return _mPinName2Idx[pinName];
}
int CellLib_C::get_cell_width(int techId){
    return _techW[techId];
}
int CellLib_C::get_cell_height(int techId){
    return _techH[techId];
}
Pos CellLib_C::get_pin_offset(int techId,int pinId){
    return _vTechLibPinOffset[techId][pinId];
}
//-----------------------------------------------------------------------------------------------------//
Row_C::Row_C(){}
//-----------------------------------------------------------------------------------------------------//
Die_C::Die_C(){}
Die_C::Die_C(int sizeX, int sizeY, int maxUtil, int techId, int rowHeight){
    _techId = techId;
    _sizeX = sizeX;
    _sizeY = sizeY;
    _maxUtil = maxUtil;
    _rowHeight = rowHeight;
}
int Die_C::get_techId(){
    return _techId;
}
int Die_C::get_cell_width(Cell_C* cell){
    return cell->get_width(_techId);
}
int Die_C::get_cell_height(Cell_C* cell){
    return cell->get_height(_techId);
}
//-----------------------------------------------------------------------------------------------------//
Pin_C::Pin_C(int id, Cell_C *cell){
    _id = id;
    _cell = cell;
}
void Pin_C::add_net(Net_C * net){
    _vNets.emplace_back(net);
}
Pos Pin_C::get_pos(){
    Pos cellPos = _cell->get_pos();
    Pos pinOffset = _cell->get_master_cell()->get_pin_offset(_cell->get_die_techId(), _id);
    return Pos(cellPos.z, cellPos.x+pinOffset.x, cellPos.y+pinOffset.y);
}
int Pin_C::get_x(){
    Pos cellPos = _cell->get_pos();
    Pos pinOffset = _cell->get_master_cell()->get_pin_offset(_cell->get_die_techId(), _id);
    return cellPos.x+pinOffset.x;
}
int Pin_C::get_y(){
    Pos cellPos = _cell->get_pos();
    Pos pinOffset = _cell->get_master_cell()->get_pin_offset(_cell->get_die_techId(), _id);
    return cellPos.y+pinOffset.y;
}
int Pin_C::get_net_num(){
    return _vNets.size();
}
Net_C* Pin_C::get_net(int i){
    return _vNets[i];
}
vector<Net_C*>& Pin_C::get_nets(){
    return _vNets;
}
Cell_C* Pin_C::get_cell(){
    return _cell;
}
//-----------------------------------------------------------------------------------------------------//
Net_C::Net_C(){}
Net_C::Net_C(string name){
    _name = name;
    _HPWL = 0;
}
void Net_C::set_id(int id){
    _id = id;
}
void Net_C::add_pin(Pin_C* pin){
    _vPins.emplace_back(pin);
}
string Net_C::get_name() const{
    return _name;
}
int Net_C::get_id(){
    return _id;
}
int Net_C::get_HPWL() const{
    return _HPWL;
}
int Net_C::get_pin_num(){
    return _vPins.size();
}
Pin_C* Net_C::get_pin(int i){
    return _vPins[i];
}
vector<Pin_C*>& Net_C::get_pins(){
    return _vPins;
}
//-----------------------------------------------------------------------------------------------------//
Cell_C::Cell_C(){}
Cell_C::Cell_C(string name, CellLib_C* masterCell){
    _name = name;
    _masterCell = masterCell;
    for(int i=0;i<masterCell->get_pin_num();++i){
        Pin_C* pin = new Pin_C(i,this);
        _vPins.emplace_back(pin);
    }
}
void Cell_C::set_id(int id){
    _id = id;
}
void Cell_C::set_pos(Pos pos){
    _pos = pos;
}
string Cell_C::get_name(){
    return _name;
}
int Cell_C::get_id(){
    return _id;
}
int Cell_C::get_width(){
    return _masterCell->get_cell_width(_die->get_techId());
}
int Cell_C::get_height(){
    return _masterCell->get_cell_width(_die->get_techId()); 
}
int Cell_C::get_width(int techId){
    return _masterCell->get_cell_width(techId);
}
int Cell_C::get_height(int techId){
    return _masterCell->get_cell_width(techId);
}
int Cell_C::get_pin_num(){
    return _vPins.size();
}
Pin_C* Cell_C::get_pin(int pinId){
    return _vPins[pinId];
}
Pin_C* Cell_C::get_pin(string pinName){
    return _vPins[_masterCell->get_pin_id(pinName)];
}
vector<Pin_C*>& Cell_C::get_pins(){
    return _vPins;
}
Pos Cell_C::get_pos(){
    return _pos;
}
int Cell_C::get_posX(){
    return _pos.x;
}
int Cell_C::get_posY(){
    return _pos.y;
}
int Cell_C::get_posZ(){
    return _pos.z;
}
CellLib_C* Cell_C::get_master_cell(){
    return _masterCell;
}
int Cell_C::get_die_techId(){
    return _die->get_techId();
}
//-----------------------------------------------------------------------------------------------------//
Chip_C::Chip_C(){}
Chip_C::Chip_C(int sizeX, int sizeY){
    _sizeX = sizeX;
    _sizeY = sizeY;
}
void Chip_C::set_chip_size(int sizeX, int sizeY){
    _sizeX = sizeX;
    _sizeY = sizeY;
}
void Chip_C::set_top_die(int maxUtil, int techId, int rowHeight){
    topDie = new Die_C(_sizeX, _sizeY, maxUtil, techId, rowHeight);
}
void Chip_C::set_bot_die(int maxUtil, int techId, int rowHeight){
    botDie = new Die_C(_sizeX, _sizeY, maxUtil, techId, rowHeight);
}
void Chip_C::set_ball(int ballSizeX, int ballSizeY, int ballSpace){
    _ballSizeX = ballSizeX;
    _ballSizeY = ballSizeY;
    _ballSpace = ballSpace;
}
Die_C* Chip_C::get_die(int dieId){
    return (dieId==0)? topDie : botDie;
}
//-----------------------------------------------------------------------------------------------------//
Design_C::Design_C(){}
void Design_C::add_cell(Cell_C* cell){
    cell->set_id(_vCells.size());
    _mCells.emplace(cell->get_name(),cell);
    _vCells.emplace_back(cell);
}
void Design_C::add_net(Net_C* net){
    net->set_id(_vNets.size());
    _mNets.emplace(net->get_name(),net);
    _vNets.emplace_back(net);
}
Cell_C* Design_C::get_cell(string cellName){
    return _mCells[cellName];
}
Cell_C* Design_C::get_cell(int cellId){
    return _vCells[cellId];
}
Net_C* Design_C::get_net(string netName){
    return _mNets[netName];
}
Net_C* Design_C::get_net(int netId){
    return _vNets[netId];
}
