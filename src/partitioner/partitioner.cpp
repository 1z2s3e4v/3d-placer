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

void Partitioner::parseInput(vector<Cell_C*>& _vCell, vector<Net_C*>& _vNet) 
{
    
    for (Cell_C* cell_ex : _vCell) {
        int cellId = _cellNum;
        int cellName = cell_ex->get_id();
        Cell* cell = new Cell(cellName, 0, cellId);
        _cellArray.push_back(cell);
        _cellName2Id[cellName] = cellId;
        cell->setPinNum(cell_ex->get_pin_num());
        if (_maxPinNum < _cellArray[cellId] -> getPinNum()) {
            _maxPinNum = _cellArray[cellId] -> getPinNum();
        }
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
                    _cellArray[cellId]->addNet(netId);
                    // _cellArray[cellId]->incPinNum();
                    _netArray[netId]->addCell(cellId);
                    ++ _netNum;
                    
                } else {
                    int netId = _netName2Id[netName];
                    if (find(_netArray[netId]->getCellList().begin(), _netArray[netId]->getCellList().end(), cellId) == _netArray[netId]->getCellList().end()) {
                        _cellArray[cellId]->addNet(netId);
                        // _cellArray[cellId]->incPinNum();
                        _netArray[netId]->addCell(cellId);
                    }
                    
                }
            }
            
        }
        
        
    }
    
    // for (Net_FM* net : _netArray) {
    //     if (net->getCellList().size() == 1) {
    //         _cellAlone.push_back(_cellArray[net->getCellList()[0]]);
    //         _cellArray[net->getCellList()[0]]->lock();
    //     }
    // }
    
}
// void Partitioner::parseInput(fstream& inFile)
// {
//     string str;
//     // Set balance factor
//     inFile >> str;
//     _bFactor = stod(str);

//     // Set up whole circuit
//     while (inFile >> str) {
//         if (str == "NET") {
//             string netName, cellName, tmpCellName = "";
//             inFile >> netName;
//             int netId = _netNum;
//             _netArray.push_back(new Net(netName));
//             _netName2Id[netName] = netId;

//             while (inFile >> cellName) {
//                 if (cellName == ";") {
//                     tmpCellName = "";
//                     break;
//                 }
//                 else {
//                     // a newly seen cell
//                     if (_cellName2Id.count(cellName) == 0) {
//                         int cellId = _cellNum;
//                         _cellArray.push_back(new Cell(cellName, 0, cellId));
//                         _cellName2Id[cellName] = cellId;
//                         _cellArray[cellId]->addNet(netId);
//                         _cellArray[cellId]->incPinNum();
//                         _netArray[netId]->addCell(cellId);
//                         ++_cellNum;
//                         tmpCellName = cellName;
//                         if (_maxPinNum < _cellArray[cellId] -> getPinNum()) {
//                             _maxPinNum = _cellArray[cellId] -> getPinNum();
//                         }
//                     }
//                     // an existed cell
//                     else {
//                         if (cellName != tmpCellName) {
//                             assert(_cellName2Id.count(cellName) == 1);
//                             int cellId = _cellName2Id[cellName];
//                             _cellArray[cellId]->addNet(netId);
//                             _cellArray[cellId]->incPinNum();
//                             _netArray[netId]->addCell(cellId);
//                             tmpCellName = cellName;
//                             if (_maxPinNum < _cellArray[cellId] -> getPinNum()) {
//                                 _maxPinNum = _cellArray[cellId] -> getPinNum();
//                             }
//                         }
//                     }
//                 }
//             }
//             ++_netNum;
//         }
//     }
//     return;
// }


void Partitioner::initial_partition() {
    for (int i=0; i<_cellNum; i++) {
        Cell* cell = _cellArray[i];

        if (i < _cellNum / 2) {
            cell -> setPart(0);
            _partSize[0] += 1;

            for (int j=0; j<cell -> getNetList().size(); j++) {
                _netArray[cell -> getNetList()[j]] -> incPartCount(0); 
            }
            
        } else {
            cell -> setPart(1);
            _partSize[1] += 1;
            for (int j=0; j<cell -> getNetList().size(); j++) {
                _netArray[cell -> getNetList()[j]] -> incPartCount(1); 
            }
            
        }
    }

    return;

}

void Partitioner::initiate_gain() {

    // cout << "initiate gain start" << endl;

    // init bucket list
    for (int g=-1*_maxPinNum; g<=_maxPinNum; g++) {
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
        Node* node = cell -> getNode();

        // init cell
        cell -> setGain(0);
        cell -> unlock();
        node -> setPrev(NULL);
        node -> setNext(NULL);

        // calc cell's init gain
        vector<int>& netList = cell->getNetList();
        for (int j=0; j<netList.size(); j++){
            Net_FM* net = _netArray[netList[j]];
            
            int from_num = net -> getPartCount(cell -> getPart());
            int to_num = net -> getPartCount(1 - cell -> getPart());

            if (from_num == 1) {
                cell -> incGain();
            }
            if (to_num == 0) {
                cell -> decGain(); 
            }
            
        }
        
        // build bucket list
        if (_bList[cell -> getPart()][cell -> getGain()] == NULL) {
            _bList[cell -> getPart()][cell -> getGain()] = node;

        } else { 
            Node* n1 = _bList[cell -> getPart()][cell -> getGain()];
            n1 -> setPrev(node);
            node -> setNext(n1);
            _bList[cell -> getPart()][cell -> getGain()] = node;
        }

        
        if (i == 0 || cell -> getGain() > _maxGain) {
            _maxGain = cell -> getGain();
            _maxGainCell = cell -> getNode();
            
        }
    }
     
    // cout << "initiate gain done" << endl;
    return;
}

Cell* Partitioner::find_cell_to_move() {

    // float lower_bound = (1 - _bFactor) / 2 * _cellNum;
    // float upper_bound = (1 + _bFactor) / 2 * _cellNum;

    // if (Partitioner::getPartSize(0) - 1 < lower_bound || Partitioner::getPartSize(1) + 1 > upper_bound) {
    //     _canBeFromSide[0] = false;
    // } else {
    //     _canBeFromSide[0] = true;
    // }
    // if (Partitioner::getPartSize(1) - 1 < lower_bound || Partitioner::getPartSize(0) + 1 > upper_bound) {
    //     _canBeFromSide[1] = false;
    // } else {
    //     _canBeFromSide[1] = true;
    // }

    if (getPartSize(0) / _cellNum < 0.65) {
        _canBeFromSide[0] = false;
    } else {
        _canBeFromSide[0] = true;
    }
    if (getPartSize(1) / _cellNum < 0.3) {
        _canBeFromSide[0] = false;
    } else {
        _canBeFromSide[0] = true;
    }

    Node* node_to_move = _maxGainCell;
    Cell* cell_to_move = _cellArray[node_to_move -> getId()];
    int from_side = cell_to_move -> getPart();
    int to_side = 1 - from_side;

    // cout << "partSize: "<<_partSize[0] << "," << _partSize[1] <<endl;

    if ( ! _canBeFromSide[from_side]) {
        
        int legal_from_side = 1 - from_side;
        
        for (int i=_maxGain; i>-1*_maxPinNum; i--) {
            if (_bList[legal_from_side][i] != NULL) {
                node_to_move = _bList[legal_from_side][i];
                cell_to_move = _cellArray[node_to_move -> getId()];
                from_side = legal_from_side;
                to_side = 1 - from_side;
                break;
            }
        }
    }
         
     
    // cout << "cell_to_move: " << cell_to_move->getNode() << " " << cell_to_move->getNode()->getId() << endl;
    
    return cell_to_move;
}

void Partitioner::move_cell(Cell* cell) {

    // cout << "cell moving" << endl;

    Node* node = cell -> getNode();
    Node* next_node = node -> getNext();
    Node* prev_node = node -> getPrev();

    int from_side = cell -> getPart();
    int to_side = 1 - from_side;

    if (next_node != NULL) {
        next_node -> setPrev(prev_node);
    } else {
        _bList[cell -> getPart()][cell -> getGain()] = NULL; // tail
    }
    if (prev_node != NULL) {
        prev_node -> setNext(next_node);
    } else {
        _bList[cell -> getPart()][cell -> getGain()] = next_node; // head
    }
   

    node -> setNext(NULL);
    node -> setPrev(NULL);
    cell -> lock();
    cell -> setPart(to_side);
    _partSize[from_side] -= 1;
    _partSize[to_side] += 1;
    _moveNum += 1;
    _moveStack.push_back(cell -> getNode() -> getId());
    _accGain += cell -> getGain();

    

    // cout << "moving cell done" << endl;
    return;
}

void Partitioner::incr_or_decr_cell_gain(Cell* cell, int incr_or_decr) {
    // incr: 0, decr: 1
   
    Node* node = cell -> getNode();
    Node* prev_node = node -> getPrev();
    Node* next_node = node -> getNext();

    if (incr_or_decr == 0) { //incr
        
        if (next_node != NULL) {
            next_node -> setPrev(prev_node);
        }
        if (prev_node != NULL) {
            prev_node -> setNext(next_node);
        }
        if (prev_node == NULL) {
            _bList[cell -> getPart()][cell -> getGain()] = next_node;
        }

        cell -> incGain();

        if (_bList[cell->getPart()][cell->getGain()] != NULL) {
            _bList[cell->getPart()][cell->getGain()] -> setPrev(node);
            node -> setNext(_bList[cell->getPart()][cell->getGain()]);
            node -> setPrev(NULL);
            _bList[cell->getPart()][cell->getGain()] = node;
        } else {
            _bList[cell->getPart()][cell->getGain()] = node;
            node -> setPrev(NULL);
            node -> setNext(NULL);
        }
    } else { // decr
        
        if (next_node != NULL) {
            next_node -> setPrev(prev_node);
        }
        if (prev_node != NULL) {
            prev_node -> setNext(next_node);
        }
        if (prev_node == NULL) {
            _bList[cell -> getPart()][cell -> getGain()] = next_node;
        }

        cell -> decGain();

        if (_bList[cell->getPart()][cell->getGain()] != NULL) {
            _bList[cell->getPart()][cell->getGain()] -> setPrev(node);
            node -> setNext(_bList[cell->getPart()][cell->getGain()]);
            node -> setPrev(NULL);
            _bList[cell->getPart()][cell->getGain()] = node;
        } else {
            _bList[cell->getPart()][cell->getGain()] = node;
            node -> setPrev(NULL);
            node -> setNext(NULL);
        }
    }
    
    return;
}


void Partitioner::update_gain() {

    // cout << "update gain" << endl;

    Cell* cell = Partitioner::find_cell_to_move();
    
    int from_side = cell -> getPart();
    int to_side = 1 - from_side;
    // cell -> lock();
    Partitioner::move_cell(cell);

    

    // before move
    vector<int>& netList = cell->getNetList();
    for (int i=0; i<netList.size(); i++){
        Net_FM* net = _netArray[netList[i]]; 
        int from_num = net -> getPartCount(from_side);
        int to_num = net -> getPartCount(to_side);
    
        if (to_num == 0) {
            //  increment gains of all free cells on n
            for (int j=0; j<net -> getCellList().size(); j++) {
                if ( ! _cellArray[net -> getCellList()[j]] -> getLock()) {
                    Partitioner::incr_or_decr_cell_gain(_cellArray[net -> getCellList()[j]], 0);         
                }
            }
                
        } else if (to_num == 1) {
            // decrement gain of the only T cell on n, if it is free
            for (int j=0; j<net -> getCellList().size(); j++) {
                if (! _cellArray[net -> getCellList()[j]] -> getLock() && _cellArray[net -> getCellList()[j]] -> getPart() == to_side) {
                    Partitioner::incr_or_decr_cell_gain(_cellArray[net -> getCellList()[j]], 1);
                }
            }      
        }
    
    
        // move
        net -> incPartCount(to_side);
        net -> decPartCount(from_side);
    
        // after move
        from_num = net -> getPartCount(from_side);
        to_num = net -> getPartCount(to_side);

        if (from_num == 0) {
            // decrement gains of all free cells on n
            for (int j=0; j<net -> getCellList().size(); j++) {
                if ( ! _cellArray[net -> getCellList()[j]] -> getLock()) {
                    Partitioner::incr_or_decr_cell_gain(_cellArray[net -> getCellList()[j]], 1);         
                }
            }
        } else if (from_num == 1) {
            // increment gain of the only F cell on n, if it is free
            for (int j=0; j<net -> getCellList().size(); j++) {
                if (_cellArray[net -> getCellList()[j]] -> getPart() == from_side && (! _cellArray[net -> getCellList()[j]] -> getLock())) {
                    Partitioner::incr_or_decr_cell_gain(_cellArray[net -> getCellList()[j]], 0);
                }
            }      
        }
    }
     
    
    int prefer_from_side = (_partSize[0] > _partSize[1]) ? 0 : 1;
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
        if (_netArray[i] -> getPartCount(0) > 0 && _netArray[i] -> getPartCount(1) > 0) {
            _cutSize += 1;
        }
    }
}

void Partitioner::trace_back() {
    for (int i=_moveStack.size() - 1; i>_maxAccGainStep; i--) {
        
        Cell* cell = _cellArray[_moveStack[i]];
        
        _partSize[cell -> getPart()] -= 1;
        _partSize[1 - cell -> getPart()] += 1;

        for (int j=0; j<cell -> getNetList().size(); j++) {
            _netArray[cell -> getNetList()[j]] -> decPartCount(cell -> getPart());
            _netArray[cell -> getNetList()[j]] -> incPartCount(1 - cell -> getPart());
        } 

        cell -> setPart(1 - cell -> getPart());
        _cellPart[1 - cell->getPart()].emplace_back(cell->getName());
    }
}

void Partitioner::partition() {


    Partitioner::initial_partition();
    Partitioner::calc_cutsize();
    cout << " Cutsize: " << _cutSize << endl;

    vector<int> gain;
    int count = 0;
    for (int iter=0; iter<2; iter++) {
        
        Partitioner::initiate_gain();

        Node* pre;
        for (int step=0; step<_cellNum; step++) {

            // cout << "step" << step <<endl;
            Partitioner::update_gain();

            if (_accGain >= _maxAccGain) {
                _maxAccGain = _accGain;
                _maxAccGainStep = step;
            }

            pre = _maxGainCell;
        }
        // cout << "maxAccGain: "<< _maxAccGain << endl;
        // cout << "maxAccGainStep: "<< _maxAccGainStep << endl;
        // cout << "accGain: " << _accGain << endl;
        gain.push_back(_maxAccGain);
        
        Partitioner::trace_back();

        // int threhold;
        // if (_cellNum / 2 + _netNum / 2 > 300000) {
        //     threhold = 120;
        // } else if (_cellNum / 2 + _netNum / 2 > 100000) {
        //     threhold = 70;
        // } else if (_cellNum / 2 + _netNum / 2 > 50000) {
        //     threhold = 30;
        // } else {
        //     threhold = 0;
        // }
        
        
        // if (_maxAccGain <= 0) {
        //     // cout << "iter = " << iter << endl;
        //     // cout <<  "maxAccGain = " << _maxAccGain << " <= 0 end" <<endl;
        //     break;
        // } else if (_maxAccGain <= threhold) {
        //     count += 1;
        // }

        // if (count > 1) {
        //     // cout << "iter = " << iter << endl;
        //     // cout << "maxAccGain = " << _maxAccGain << ", is small enough" << endl;
        //     break;
        // }
    }

    for (int i=0; i<gain.size(); i++) {
        // cout << gain[i] << " , ";
    }
 
    Partitioner::calc_cutsize();
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
