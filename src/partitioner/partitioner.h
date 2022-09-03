#ifndef PARTITIONER_H
#define PARTITIONER_H

#include <fstream>
#include <vector>
#include <map>
#include "cell.h"
#include "net.h"
#include "../dataModel/module.h"
using namespace std;

class Partitioner
{
public:
    // constructor and destructor
    Partitioner() :
        _cutSize(0), _netNum(0), _cellNum(0), _maxPinNum(0), _bFactor(0),
        _accGain(0), _maxAccGain(0), _iterNum(0) {
        // parseInput(inFile);
        _partSize[0] = 0;
        _partSize[1] = 0;
        _partArea[0] = 0;
        _partArea[1] = 0;
        _cellPart.resize(2,vector<int>());
    }
    ~Partitioner() {
        clear();
    }

    // basic access methods
    int getCutSize() const          { return _cutSize; }
    int getNetNum() const           { return _netNum; }
    int getCellNum() const          { return _cellNum; }
    double getBFactor() const       { return _bFactor; }
    int getPartSize(int part) const { return _partSize[part]; }
    double getPartArea(int part) const { return _partArea[part]; }
    

    // modify method
    // void parseInput(vector<Cell_C*>& v_Cell, Chip_C* p_Chip);
    void parseInput(vector<Cell_C*>& v_Cell, Chip_C* p_Chip, vector <Cell_C*>& bin_cell, double (& maxArea)[2], double cutline, bool inh_part);
    // void parseInput(fstream& inFile);
    void initial_partition();
    void inherit_partition(vector<vector<int> >& cellPart);
    void initiate_gain(int gain_2_pin, int gain_3_pin, int gain_mult, bool gain_altr);
    Cell* find_cell_to_move();
    void move_cell(Cell* cell);
    void incr_or_decr_cell_gain(Cell* cell, int incr_or_decr);
    void update_gain(int gain_2_pin, int gain_3_pin, int gain_mult, bool gain_altr);
    void calc_cutsize();
    void trace_back();
    void partition(int gain_2_pin, int gain_3_pin, int gain_mult, bool gain_altr);
    vector<vector<int> >& get_part_result();// {return _cellPart;}
    bool verification_hard();
    bool verification_soft();

    // member functions about reporting
    void printSummary() const;
    void reportNet() const;
    void reportCell() const;
    void writeResult(fstream& outFile);

private:
    int                 _cutSize;       // cut size
    int                 _partSize[2];   // size (cell number) of partition A(0) and B(1)
    double                 _partArea[2];   // area size (cell area) of partition A(0) and B(1)
    int                 _netNum;        // number of nets
    int                 _cellNum;       // number of cells
    int                 _maxPinNum;     // Pmax for building bucket list
    double              _bFactor;       // the balance factor to be met
    Node*               _maxGainCell;   // pointer to max gain cell
    vector<Net_FM*>        _netArray;      // net array of the circuit
    vector<Cell*>       _cellArray;     // cell array of the circuit
    map<int, Node*>     _bList[2];      // bucket list of partition A(0) and B(1)
    map<int, int>       _netName2Id;    // mapping from net name to id
    map<int, int>       _cellName2Id;   // mapping from cell name to id

    int                 _accGain;       // accumulative gain
    int                 _maxAccGain;    // maximum accumulative gain
    int                 _maxAccGainStep; // step of maximum accumulative gain
    int                 _moveNum;       // number of cell movements
    int                 _iterNum;       // number of iterations
    int                 _bestMoveNum;   // store best number of movements
    int                 _unlockNum[2];  // number of unlocked cells
    vector<int>         _moveStack;     // history of cell movement
    int                 _maxGain;        // max gain value among all free cell's
    bool                _canBeFromSide[2]; // whether can be from side or not
    vector<Cell*>       _cellAlone;      // cell does not be connected to other cells in the bin
    vector<vector<int> > _cellPart; 
    vector<Cell_C*>     _vCell;
    Chip_C*             _pChip;
    double              _maxArea[2];
    bool                _earlyBreak;
    bool                _legalResult_hard;
    bool                _legalResult_soft;
    double              _cutline;

    // Clean up partitioner
    void clear();
};

#endif  // PARTITIONER_H
