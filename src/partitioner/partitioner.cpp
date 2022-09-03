#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <cmath>
#include <map>
#include <set>
#include "partitioner.h"
using namespace std;

void Partitioner::parseInput(vector<Cell_C*>& v_Cell, Chip_C* p_Chip, vector <Cell_C*>& bin_cell, double(& maxArea)[2], double cutline, bool inh_part) 
{
    // cerr << "parseInput" << endl;
    _vCell = v_Cell;
    _pChip = p_Chip;
    _cutline = cutline;
    _maxArea[0] = maxArea[0];
    _maxArea[1] = maxArea[1];
    // _maxArea[0] = (double) _pChip->get_die(0)->get_width() * (double) _pChip->get_die(0)->get_height() * _pChip->get_die(0)->get_max_util() - used_area[0];
    // _maxArea[1] = (double) _pChip->get_die(1)->get_width() * (double) _pChip->get_die(1)->get_height() * _pChip->get_die(1)->get_max_util() - used_area[1];
    // cout << _maxArea[0] << "," << _maxArea[1] << "\n";
    // for (Cell_C* cell_ex : _vCell) {
    for (Cell_C* cell_ex : bin_cell) {
        int cellId = _cellNum;
        int cellName = cell_ex->get_id();
        Cell* cell = new Cell(cellName, cell_ex->get_dieId(), cellId);
        _cellArray.push_back(cell);
        _cellName2Id[cellName] = cellId;
        ++ _cellNum;
        
        for (Pin_C* pin : cell_ex->get_pins()) {
            if (pin->get_net() != nullptr) {
                int netName = pin->get_net()->get_id();
                // a newly seen net
                if (_netName2Id.count(netName) == 0) {
                    int netId = _netNum;
                    Net_FM* net = new Net_FM(netName);
                    _netArray.emplace_back(net);
                    _netName2Id[netName] = netId;
                    cell->addNet(netId);
                    cell->incPinNum();
                    net->addCell(cellId);
                    ++ _netNum;
                    
                } else {
                    int netId = _netName2Id[netName];
                    Net_FM* net = _netArray[netId];
                    if (find(net->getCellList().begin(), net->getCellList().end(), cellId) == net->getCellList().end()) {
                        cell->addNet(netId);
                        cell->incPinNum();
                        net->addCell(cellId);
                    } //else {cout << "here~~~~\n";}
                    
                }
            }
            
        }
        
        if (_maxPinNum < cell->getPinNum()) { 
            _maxPinNum = cell->getPinNum();
        }
        
        
    }
    if (inh_part) {
        for (int i=0; i < bin_cell.size(); ++i) {
            Cell_C* cell_ex = bin_cell[i];
            Cell* cell = _cellArray[i];
            _partSize[cell_ex->get_dieId()] += 1;
            _partArea[cell_ex->get_dieId()] += _vCell[cell->getName()]
                ->get_width(_pChip->get_die(cell_ex->get_dieId())->get_techId()) * 
                _vCell[cell->getName()]->get_height(_pChip->get_die(cell_ex->get_dieId())->get_techId());

            for (int j=0; j<cell->getNetList().size(); j++) {
                _netArray[cell->getNetList()[j]]->incPartCount(cell_ex->get_dieId()); 
            }
        }
    }
    // for (Net_FM* net : _netArray) {
    //     if (net->getCellList().size() == 1) {
    //         cout << _cellArray[net->getCellList()[0]] << "\n";
    //         // _cellAlone.push_back(_cellArray[net->getCellList()[0]]);
    //         // _cellArray[net->getCellList()[0]]->lock();
    //     }
    // }
    
}

void Partitioner::initial_partition() {
    
    // for (int i=0; i<_cellNum; i++) {
    //     Cell* cell = _cellArray[i];
    //     int part = _vCell[cell->getName()]->get_dieId();
    //     cell->setPart(part);
    //     _partSize[part] += 1;
    //     _partArea[part] += _vCell[cell->getName()]->get_width(_pChip->get_die(part)->get_techId()) * _vCell[cell->getName()]->get_height(_pChip->get_die(part)->get_techId());

    //     for (int j=0; j<cell->getNetList().size(); j++) {
    //         _netArray[cell->getNetList()[j]]->incPartCount(part); 
    //     }

    // }
    for (int i=0; i<_cellNum; i++) {
        Cell* cell = _cellArray[i];

        if (i < _cellNum * (1 - _cutline)) {
            cell->setPart(0);
            _partSize[0] += 1;
            _partArea[0] += _vCell[cell->getName()]->get_width(_pChip->get_die(0)->get_techId()) * _vCell[cell->getName()]->get_height(_pChip->get_die(0)->get_techId());

            for (int j=0; j<cell->getNetList().size(); j++) {
                _netArray[cell->getNetList()[j]]->incPartCount(0); 
            }
            
        } else {
            cell->setPart(1);
            _partSize[1] += 1;
            _partArea[1] += _vCell[cell->getName()]->get_width(_pChip->get_die(1)->get_techId()) * _vCell[cell->getName()]->get_height(_pChip->get_die(1)->get_techId());
            for (int j=0; j<cell->getNetList().size(); j++) {
                _netArray[cell->getNetList()[j]]->incPartCount(1); 
            }
            
        }
    }
    //cout << "~~~~~~~~~~~~~_partArea[1] = " << _partArea[1] <<"\n";

    return;

}

void Partitioner::inherit_partition(vector<vector<int> >& cellPart) {
    // cerr << "inherit partition" << endl;
    // cerr << "cellPart[0].size() = " << cellPart[0].size() << endl;
    // cerr << "cellPart[1].size() = " << cellPart[1].size() << endl;
    // cerr << "cellPart[0]: " << endl;
    for (int i=0; i<cellPart[0].size(); ++i) {
        int cellIdx = cellPart[0][i];
        // cerr << cellIdx << " ";
        Cell* cell = _cellArray[cellIdx];
        cell->setPart(0);
        _partSize[0] += 1;
        _partArea[0] += _vCell[cell->getName()]->get_width(_pChip->get_die(0)->get_techId()) * _vCell[cell->getName()]->get_height(_pChip->get_die(0)->get_techId());

        for (int j=0; j<cell->getNetList().size(); j++) {
            _netArray[cell->getNetList()[j]]->incPartCount(0); 
        }
    }
    // cerr << "cellPart[1]: " << endl;
    for (int cellIdx : cellPart[1]) {
        // cerr << cellIdx << " ";
        Cell* cell = _cellArray[cellIdx];
        cell->setPart(1);
        _partSize[1] += 1;
        _partArea[1] += _vCell[cell->getName()]->get_width(_pChip->get_die(1)->get_techId()) * _vCell[cell->getName()]->get_height(_pChip->get_die(1)->get_techId());
        for (int j=0; j<cell->getNetList().size(); j++) {
            _netArray[cell->getNetList()[j]]->incPartCount(1); 
        }
    }
    return;
}

void Partitioner::initiate_gain(int gain_2_pin, int gain_3_pin, int gain_mult, bool gain_altr) {

    // cout << "initiate gain start" << endl;

    // init bucket list
    for (int g=-1*_maxPinNum; g<=gain_mult*_maxPinNum; g++) {
        _bList[0][g] = NULL;
        _bList[1][g] = NULL;
    }

    //init var
    _accGain = 0;
    _maxAccGain = 0;    
    _maxAccGainStep = 0; 
    _moveNum = 0; 
    _moveStack.clear();

    for (int i=0; i<_cellNum; i++) {
        Cell* cell = _cellArray[i];
        Node* node = cell->getNode();

        // init cell
        cell->setGain(0);
        cell->unlock();
        node->setPrev(NULL);
        node->setNext(NULL);

        // calc cell's init gain
        vector<int>& netList = cell->getNetList();
        for (int j=0; j<netList.size(); j++){
            Net_FM* net = _netArray[netList[j]];
            
            int from_num = net->getPartCount(cell->getPart());
            int to_num = net->getPartCount(1 - cell->getPart());
            
            //  for 2-pin net, encourage cut
            if (from_num == 1) {
                if (to_num == 1) {
                    cell->incGain();
                    if (gain_altr) {
                        for (int i=0; i<gain_2_pin-1; ++i) {
                            cell->incGain();
                        }
                    }
                } 
                else if (to_num == 2) {
                    cell->incGain();
                    if (gain_altr) {
                        for (int i=0; i<gain_3_pin-1; ++i) {
                            cell->incGain();
                        }
                    }
                }
                else {
                    cell->incGain();
                    if (gain_altr) {
                        for (int i=0; i<gain_mult-1; ++i) {
                            cell->incGain();
                        }
                    }
                }
            }
            if (to_num == 0) {
                if (from_num == 2) {
                    cell->decGain();
                    if (gain_altr) {
                        for (int i=0; i<gain_2_pin-1; ++i) {
                        cell->decGain();
                        }
                    }
                }
                else if (from_num == 3) {
                    cell->decGain();
                    if (gain_altr) {
                        for (int i=0; i<gain_3_pin-1; ++i) {
                        cell->decGain();
                        }
                    }
                }
                else {
                    cell->decGain();
                    if (gain_altr) {
                        for (int i=0; i<gain_mult-1; ++i) {
                        cell->decGain();
                        }
                    }
                } 
            } 
        }
        
        // build bucket list
        if (_bList[cell->getPart()][cell->getGain()] == NULL) {
            _bList[cell->getPart()][cell->getGain()] = node;

        } else { 
            Node* n1 = _bList[cell->getPart()][cell->getGain()];
            n1->setPrev(node);
            node->setNext(n1);
            _bList[cell->getPart()][cell->getGain()] = node;
        }

        
        if (i == 0 || cell->getGain() > _maxGain) {
            _maxGain = cell->getGain();
            _maxGainCell = cell->getNode();
            
        }
    }
     
    // cout << "initiate gain done" << endl;
    return;
}

Cell* Partitioner::find_cell_to_move() {

    _bFactor = 0.5;
    float lower_bound = (1 - _bFactor) / 2 * _cellNum;
    float upper_bound = (1 + _bFactor) / 2 * _cellNum;

    if (getPartSize(0) - 1 < lower_bound || getPartSize(1) + 1 > upper_bound) {
        _canBeFromSide[0] = false;
    } else {
        _canBeFromSide[0] = true;
    }
    if (getPartSize(1) - 1 < lower_bound || getPartSize(0) + 1 > upper_bound) {
        _canBeFromSide[1] = false;
    } else {
        _canBeFromSide[1] = true;
    }

    // if (getPartSize(0) - 1 < lower_bound || getPartSize(1) + 1 > upper_bound) {
    //     _canBeFromSide[0] = false;
    // } else {
    //     _canBeFromSide[0] = true;
    // }
    // if (getPartSize(1) - 1 < lower_bound || getPartSize(0) + 1 > upper_bound) {
    //     _canBeFromSide[1] = false;
    // } else {
    //     _canBeFromSide[1] = true;
    // }

    // if ((float) getPartSize(0) / _cellNum < 0.65) {
    //     _canBeFromSide[0] = false;
    // } else {
    //     _canBeFromSide[0] = true;
    // }
    // if ((float) getPartSize(1) / _cellNum < 0.3) {
    //     _canBeFromSide[1] = false;
    // } else {
    //     _canBeFromSide[1] = true;
    // }

    // cout << "getPartArea(0)=" << getPartArea(0) << ", _maxArea[0]=" << _maxArea[0] << "\n";
    // if (getPartArea(0) >= _maxArea[0] || (float) getPartSize(0) / _cellNum > 0.75) {
    //     // cout << "  --> getPartArea(0) >= _maxArea[0]\n"; 
    //     _canBeFromSide[1] = false;
    // } else {
    //     // cout << "  --> getPartArea(0) < _maxArea[0]\n"; 
    //     _canBeFromSide[1] = true;
    // }
    // // cout << "getPartArea(1)=" << getPartArea(1) << ", _maxArea[1]=" << _maxArea[1] << "\n";
    // if (getPartArea(1) >= _maxArea[1] || (float) getPartSize(1) / _cellNum > 0.25) {
    //     // cout << "  --> getPartArea(1) >= _maxArea[1]\n"; 
    //     _canBeFromSide[0] = false;
    // } else {
    //     // cout << "  --> getPartArea(1) < _maxArea[1]\n"; 
    //     _canBeFromSide[0] = true;
    // }

    Node* node_to_move = _maxGainCell;
    Cell* cell_to_move = _cellArray[node_to_move->getId()];
    int from_side = cell_to_move->getPart();
    int to_side = 1 - from_side;

    // cout << "partSize: "<<_partSize[0] << "," << _partSize[1] <<endl;

    if ( ! _canBeFromSide[from_side]) {
        
        int legal_from_side = 1 - from_side;
        // cout << "  _canBeFromSide[" << from_side << "] == false\n";
        // cout << "  _maxGain = " << _maxGain << "\n";
        bool exist = false;
        for (int i=_maxGain; i>-1*_maxPinNum; i--) {
            if (_bList[legal_from_side][i] != NULL) {
                node_to_move = _bList[legal_from_side][i];
                cell_to_move = _cellArray[node_to_move->getId()];
                from_side = legal_from_side;
                to_side = 1 - from_side;
                exist = true;
                // cout << "  here\n";
                break;
            }
            if (! exist) {
                _earlyBreak = true;
            }
        }
        
    }
    // cout << "  ==> from_side=" << from_side << "\n";
     
    // cout << "cell_to_move: " << cell_to_move->getNode() << " " << cell_to_move->getNode()->getId() << endl;
    
    return cell_to_move;
}

void Partitioner::move_cell(Cell* cell) {

    // cout << "cell moving" << endl;

    Node* node = cell->getNode();
    Node* next_node = node->getNext();
    Node* prev_node = node->getPrev();

    int from_side = cell->getPart();
    int to_side = 1 - from_side;

    if (next_node != NULL) {
        next_node->setPrev(prev_node);
    } else {
        _bList[cell->getPart()][cell->getGain()] = NULL; // tail
    }
    if (prev_node != NULL) {
        prev_node->setNext(next_node);
    } else {
        _bList[cell->getPart()][cell->getGain()] = next_node; // head
    }
   

    node->setNext(NULL);
    node->setPrev(NULL);
    cell->lock();
    cell->setPart(to_side);
    _partSize[from_side] -= 1;
    _partSize[to_side] += 1;
    _partArea[from_side] -= _vCell[cell->getName()]->get_width(_pChip->get_die(from_side)->get_techId()) * _vCell[cell->getName()]->get_height(_pChip->get_die(from_side)->get_techId());
    _partArea[to_side] += _vCell[cell->getName()]->get_width(_pChip->get_die(to_side)->get_techId()) * _vCell[cell->getName()]->get_height(_pChip->get_die(to_side)->get_techId());
    _moveNum += 1;
    _moveStack.push_back(cell->getNode()->getId());
    _accGain += cell->getGain();

    

    // cout << "moving cell done" << endl;
    return;
}

void Partitioner::incr_or_decr_cell_gain(Cell* cell, int incr_or_decr) {
    // incr: 0, decr: 1
   
    Node* node = cell->getNode();
    Node* prev_node = node->getPrev();
    Node* next_node = node->getNext();

    if (incr_or_decr == 0) { //incr
        
        if (next_node != NULL) {
            next_node->setPrev(prev_node);
        }
        if (prev_node != NULL) {
            prev_node->setNext(next_node);
        }
        if (prev_node == NULL) {
            _bList[cell->getPart()][cell->getGain()] = next_node;
        }

        cell->incGain();

        if (_bList[cell->getPart()][cell->getGain()] != NULL) {
            _bList[cell->getPart()][cell->getGain()]->setPrev(node);
            node->setNext(_bList[cell->getPart()][cell->getGain()]);
            node->setPrev(NULL);
            _bList[cell->getPart()][cell->getGain()] = node;
        } else {
            _bList[cell->getPart()][cell->getGain()] = node;
            node->setPrev(NULL);
            node->setNext(NULL);
        }
    } else { // decr
        
        if (next_node != NULL) {
            next_node->setPrev(prev_node);
        }
        if (prev_node != NULL) {
            prev_node->setNext(next_node);
        }
        if (prev_node == NULL) {
            _bList[cell->getPart()][cell->getGain()] = next_node;
        }

        cell->decGain();

        if (_bList[cell->getPart()][cell->getGain()] != NULL) {
            _bList[cell->getPart()][cell->getGain()]->setPrev(node);
            node->setNext(_bList[cell->getPart()][cell->getGain()]);
            node->setPrev(NULL);
            _bList[cell->getPart()][cell->getGain()] = node;
        } else {
            _bList[cell->getPart()][cell->getGain()] = node;
            node->setPrev(NULL);
            node->setNext(NULL);
        }
    }
    
    return;
}


void Partitioner::update_gain(int gain_2_pin, int gain_3_pin, int gain_mult, bool gain_altr) {

    // cout << "update gain" << endl;

    Cell* cell = find_cell_to_move();
    
    int from_side = cell->getPart();
    int to_side = 1 - from_side;
    // cell->lock();
    move_cell(cell);

    

    // before move
    vector<int>& netList = cell->getNetList();
    for (int i=0; i<netList.size(); i++){
        Net_FM* net = _netArray[netList[i]]; 
        int from_num = net->getPartCount(from_side);
        int to_num = net->getPartCount(to_side);
    
        if (to_num == 0) {
            //  increment gains of all free cells on n
            //  for 2-pin net, decrement gain of the free cell to dicourage from gathering
            if (from_num == 2) {
                for (int j=0; j<net->getCellList().size(); j++) {
                    if ( ! _cellArray[net->getCellList()[j]]->getLock()) {
                        incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 0);
                        if (gain_altr) {
                            for (int g=0; g<gain_2_pin-1; ++g) {
                                incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 0);  
                            }   
                        }        
                    }
                }
            }
            else if (from_num == 3) {
                for (int j=0; j<net->getCellList().size(); j++) {
                    if ( ! _cellArray[net->getCellList()[j]]->getLock()) {
                        incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 0);
                        if (gain_altr) {
                            for (int g=0; g<gain_3_pin-1; ++g) {
                                incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 0);  
                            }   
                        }        
                    }
                }
            }
            else {
                for (int j=0; j<net->getCellList().size(); j++) {
                    if ( ! _cellArray[net->getCellList()[j]]->getLock()) {
                        incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 0);
                        if (gain_altr) {
                            for (int g=0; g<gain_mult-1; ++g) {
                                incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 0);  
                            }   
                        }   
                    }
                }
            }
                
        } else if (to_num == 1) {
            // decrement gain of the only T cell on n, if it is free
            // for 2-pin net, increment gain of the free cell to encourage cut
            if (from_num == 1) {
                for (int j=0; j<net->getCellList().size(); j++) {
                    if (! _cellArray[net->getCellList()[j]]->getLock() && _cellArray[net->getCellList()[j]]->getPart() == to_side) {
                        incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 1);
                        if (gain_altr) {
                            for (int g=0; g<gain_2_pin-1; ++g) {
                                incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 1);    
                            } 
                        }  
                    }
                } 
            }
            else if (from_num == 2) {
                for (int j=0; j<net->getCellList().size(); j++) {
                    if (! _cellArray[net->getCellList()[j]]->getLock() && _cellArray[net->getCellList()[j]]->getPart() == to_side) {
                        incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 1);
                        if (gain_altr) {
                            for (int g=0; g<gain_3_pin-1; ++g) {
                                incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 1);    
                            } 
                        }  
                    }
                } 
            }
            else {
                for (int j=0; j<net->getCellList().size(); j++) {
                    if (! _cellArray[net->getCellList()[j]]->getLock() && _cellArray[net->getCellList()[j]]->getPart() == to_side) {
                        incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 1);
                        if (gain_altr) {
                            for (int g=0; g<gain_mult-1; ++g) {
                                incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 1);    
                            } 
                        }   
                    }
                }
            }      
        }
    
    
        // move
        net->incPartCount(to_side);
        net->decPartCount(from_side);
    
        // after move
        from_num = net->getPartCount(from_side);
        to_num = net->getPartCount(to_side);

        if (from_num == 0) {
            // decrement gains of all free cells on n
            // for 2-pin net, increment gain of the free cell to encourage cut
            if (to_num == 2) {
                for (int j=0; j<net->getCellList().size(); j++) {
                    if ( ! _cellArray[net->getCellList()[j]]->getLock()) {
                        incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 1);
                        if (gain_altr) {
                            for (int g=0; g<gain_2_pin-1; ++g) {
                                incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 1);
                            }     
                        }        
                    }
                }
            }
            else if (to_num == 3) {
                for (int j=0; j<net->getCellList().size(); j++) {
                    if ( ! _cellArray[net->getCellList()[j]]->getLock()) {
                        incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 1);
                        if (gain_altr) {
                            for (int g=0; g<gain_3_pin-1; ++g) {
                                incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 1);
                            }     
                        }        
                    }
                }
            }
            else {
                for (int j=0; j<net->getCellList().size(); j++) {
                    if ( ! _cellArray[net->getCellList()[j]]->getLock()) {
                        incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 1);
                        if (gain_altr) {
                            for (int g=0; g<gain_mult-1; ++g) {
                                incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 1);
                            }     
                        }
                    }
                }
            }
        } else if (from_num == 1) {
            // increment gain of the only F cell on n, if it is free
            //  for 2-pin net, decrement gain of the free cell to dicourage from gathering
            if (to_num == 1) {
                for (int j=0; j<net->getCellList().size(); j++) {
                    if (_cellArray[net->getCellList()[j]]->getPart() == from_side && (! _cellArray[net->getCellList()[j]]->getLock())) {
                        incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 0);
                        if (gain_altr) {
                            for (int g=0; g<gain_2_pin-1; ++g) {
                                incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 0);
                            }
                        }
                    }
                }
            }
            else if (to_num == 2) {
                for (int j=0; j<net->getCellList().size(); j++) {
                    if (_cellArray[net->getCellList()[j]]->getPart() == from_side && (! _cellArray[net->getCellList()[j]]->getLock())) {
                        incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 0);
                        if (gain_altr) {
                            for (int g=0; g<gain_3_pin-1; ++g) {
                                incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 0);
                            }
                        }
                    }
                }
            }
            else {
                for (int j=0; j<net->getCellList().size(); j++) {
                    if (_cellArray[net->getCellList()[j]]->getPart() == from_side && (! _cellArray[net->getCellList()[j]]->getLock())) {
                        incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 0);
                        if (gain_altr) {
                            for (int g=0; g<gain_mult-1; ++g) {
                                incr_or_decr_cell_gain(_cellArray[net->getCellList()[j]], 0);
                            }
                        }
                    }
                }
            }   
        }
    }
     
    
    int prefer_from_side = (_maxArea[0] - _partArea[0] > _maxArea[1] - _partArea[1]) ? 1 : 0;// (_partSize[0] > _partSize[1]) ? 0 : 1;
    for (int g=_maxPinNum; g>-1*_maxPinNum; g--) {
        if (_bList[prefer_from_side][g] != NULL) {
            _maxGainCell = _bList[prefer_from_side][g];
            _maxGain = g;
            
            break;
        } else if (_bList[1 - prefer_from_side][g] != NULL) {
            _maxGainCell = _bList[1 - prefer_from_side][g];
            _maxGain = g;
            break;
        }
    }
        
    // cout << "update gain done~~~~~~~~~~~~~~~~~~~~~~~~" << endl;    
    return;
}

void Partitioner::calc_cutsize() {
    for (int i=0; i<_netNum; i++) {
        // cout << _netArray[i]->getCellList().size() << ":" << _netArray[i]->getPartCount(0) << "," << _netArray[i]->getPartCount(1)<< "\n";
        if (_netArray[i]->getPartCount(0) > 0 && _netArray[i]->getPartCount(1) > 0) {
            _cutSize += 1;
        }
    }
}

void Partitioner::trace_back() {
    for (int i=_moveStack.size() - 1; i>_maxAccGainStep; i--) {
        
        Cell* cell = _cellArray[_moveStack[i]];
        
        _partSize[cell->getPart()] -= 1;
        _partSize[1 - cell->getPart()] += 1;
        _partArea[cell->getPart()] -= _vCell[cell->getName()]->get_width(_pChip->get_die(cell->getPart())->get_techId()) * _vCell[cell->getName()]->get_height(_pChip->get_die(cell->getPart())->get_techId());
        _partArea[1 - cell->getPart()] += _vCell[cell->getName()]->get_width(_pChip->get_die(1 - cell->getPart())->get_techId()) * _vCell[cell->getName()]->get_height(_pChip->get_die(1 - cell->getPart())->get_techId());

        for (int j=0; j<cell->getNetList().size(); j++) {
            _netArray[cell->getNetList()[j]]->decPartCount(cell->getPart());
            _netArray[cell->getNetList()[j]]->incPartCount(1 - cell->getPart());
        } 

        cell->setPart(1 - cell->getPart());
        
    }
}

bool Partitioner::verification_hard() {
    // double used_area[2] = {0.0, 0.0};
    // for (Cell* cell : _cellArray) {
    //     int side = cell->getPart();
    //     used_area[side] += _vCell[cell->getName()]->get_width(_pChip->get_die(side)->get_techId()) * _vCell[cell->getName()]->get_height(_pChip->get_die(side)->get_techId());
    // }
    // if (used_area[0] > _maxArea[0] || used_area[1] > _maxArea[1]) {
    //     return false;
    // }
    if (getPartArea(0) > _maxArea[0] || getPartArea(1) > _maxArea[1]) {
        return false;
    }
    return true;
}

bool Partitioner::verification_soft() { // Todo
    if (((double)(getPartArea(0) / getPartArea(1)) < 0.33) || ((double)(getPartArea(1) / getPartArea(0)) < 0.33)) {
        return false;
    }
    // if (((getPartSize(0) / getPartSize(1)) < 1 / 3) || ((getPartSize(1) / getPartSize(0)) < 1 / 3)) {
    //     return false;
    // }
    return true;
}

vector<vector<int> >& Partitioner::get_part_result() {
    
    if (_legalResult_hard) {
        bool inv = false;
        int larger_side = getPartSize(0) > getPartSize(1) ? 0 : 1;
        if (larger_side == 1) {
            double temp_area[2] = {0.0, 0.0};
            for (Cell* cell : _cellArray) {
                int temp_side = 1 - cell->getPart();
                temp_area[temp_side] += _vCell[cell->getName()]->get_width(_pChip->get_die(temp_side)->get_techId()) * _vCell[cell->getName()]->get_height(_pChip->get_die(temp_side)->get_techId());
            }
            if (temp_area[0] > getPartArea(0) && temp_area[0] <= _maxArea[0] && temp_area[1] <= _maxArea[1]) {
                inv = true;
            }
        }
        if (inv) {
            // cout << "inv\n";
            for (Cell* cell : _cellArray) {
                _cellPart[1 - cell->getPart()].emplace_back(cell->getName());
            }
        } else {
            for (Cell* cell : _cellArray) {
                _cellPart[cell->getPart()].emplace_back(cell->getName());
            }
        }
        
    } else {
        int side = (_maxArea[0] < _maxArea[1]) ? 1 : 0;
        double rest_area[2] = {_maxArea[0], _maxArea[1]};
        for (Cell* cell : _cellArray) {
            rest_area[side] -= _vCell[cell->getName()]->get_width(_pChip->get_die(side)->get_techId()) * _vCell[cell->getName()]->get_height(_pChip->get_die(side)->get_techId());
            if (rest_area[side] >= 0) {
                _cellPart[side].emplace_back(cell->getName());
            } else {
                _cellPart[1 - side].emplace_back(cell->getName());
                rest_area[side] += _vCell[cell->getName()]->get_width(_pChip->get_die(side)->get_techId()) * _vCell[cell->getName()]->get_height(_pChip->get_die(side)->get_techId());
                side = 1 - side;
            }
            
        }
        // cout << "xxxxxxxxxx\n";
    }

    return _cellPart;
}

void Partitioner::partition(int gain_2_pin, int gain_3_pin, int gain_mult, bool gain_altr) {
    // cerr << "partition" << endl;

    // Partitioner::initial_partition();
    Partitioner::calc_cutsize();
    // cout << " Cutsize: " << _cutSize << endl;
    
    vector<int> gain;
    int count = 0;
    _legalResult_hard = false;
    _legalResult_soft = false;
    for (int iter=0; iter<2; iter++) {

        if (_maxArea[0] < 0 || _maxArea[1] < 0) {
            break;
        }

        initiate_gain(gain_2_pin, gain_3_pin, gain_mult, gain_altr);
        // _earlyBreak = false;
        int _maxAccGain_soft, _maxAccGainStep_soft;

        // Node* pre;
        for (int step=0; step<_cellNum; step++) {

            
            // if (_earlyBreak) {
            //     break;
            // }
            

            // cout << "step" << step <<endl;
            update_gain(gain_2_pin, gain_3_pin, gain_mult, gain_altr);

            
            bool hard_constraint = verification_hard();
            bool soft_constraint = verification_soft();
            // if (_accGain >= _maxAccGain && hard_constraint) {
            //     _maxAccGain = _accGain;
            //     _maxAccGainStep = step;
            //     _legalResult_hard = true;
            //     cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << "size : "<< getPartSize(0) <<"; " << getPartArea(0) << "," << _maxArea[0] << "\n";
            // }
            if ((_accGain >= _maxAccGain || _accGain >= _maxAccGain_soft) && hard_constraint) {
                if (soft_constraint) {
                    _maxAccGain_soft = _accGain;
                    _maxAccGainStep_soft = step;
                    _legalResult_soft = true;
                    // cout << "soft " << "("<< getPartSize(0) <<", " << getPartArea(0) << ", " << _maxArea[0] <<") (" << getPartSize(1) <<", " << getPartArea(1) << ", " << _maxArea[1]  << ")\n";
                } 
                if (_accGain >= _maxAccGain) {
                    _maxAccGain = _accGain;
                    _maxAccGainStep = step;
                    // cout << "hard " << "("<< getPartSize(0) <<", " << getPartArea(0) << ", " << _maxArea[0] <<") (" << getPartSize(1) <<", " << getPartArea(1) << ", " << _maxArea[1]  << ")\n";
                }
                
                _legalResult_hard = true; 
            }
            if (_legalResult_soft) {
                _maxAccGain = _maxAccGain_soft;
                _maxAccGainStep = _maxAccGainStep_soft;
            }

            // pre = _maxGainCell;

        }
        // cout << "maxAccGain: "<< _maxAccGain << endl;
        // cout << "maxAccGainStep: "<< _maxAccGainStep << endl;
        // cout << "accGain: " << _accGain << endl;
        gain.push_back(_maxAccGain);
        
        trace_back();

        
    }
 
    calc_cutsize();
}


void Partitioner::printSummary() const
{
    cout << endl;
    cout << "==================== Summary ====================" << endl;
    cout << " Cutsize: " << _cutSize << endl;
    cout << " Total cell number: " << _cellNum << endl;
    cout << " Total net number:  " << _netNum << endl;
    cout << " Cell Number of partition A: " << _partSize[0] << endl;
    cout << " Cell Number of partition B: " << _partSize[1] << endl;
    cout << " Cell Area of partition A: " << _partArea[0] << endl;
    cout << " Cell Area of partition B: " << _partArea[1] << endl;
    cout << "=================================================" << endl;
    cout << endl;
    return;
}

void Partitioner::reportNet() const
{
    cout << "Number of nets: " << _netNum << endl;
    for (size_t i = 0, end_i = _netArray.size(); i < end_i; ++i) {
        cout << setw(8) << _netArray[i]->getName() << ": ";
        vector<int> cellList = _netArray[i]->getCellList();
        for (size_t j = 0, end_j = cellList.size(); j < end_j; ++j) {
            cout << setw(8) << _cellArray[cellList[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::reportCell() const
{
    cout << "Number of cells: " << _cellNum << endl;
    for (size_t i = 0, end_i = _cellArray.size(); i < end_i; ++i) {
        cout << setw(8) << _cellArray[i]->getName() << ": ";
        vector<int> netList = _cellArray[i]->getNetList();
        for (size_t j = 0, end_j = netList.size(); j < end_j; ++j) {
            cout << setw(8) << _netArray[netList[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::writeResult(fstream& outFile)
{
    stringstream buff;
    buff << _cutSize;
    outFile << "Cutsize = " << buff.str() << '\n';
    buff.str("");
    buff << _partSize[0];
    outFile << "G1 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 0) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    buff.str("");
    buff << _partSize[1];
    outFile << "G2 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        if (_cellArray[i]->getPart() == 1) {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    return;
}

void Partitioner::clear()
{
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i) {
        delete _cellArray[i];
    }
    for (size_t i = 0, end = _netArray.size(); i < end; ++i) {
        delete _netArray[i];
    }
    return;
}