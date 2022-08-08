#ifndef CELL_H
#define CELL_H

#include <vector>
using namespace std;

class Node
{
    friend class Cell;

public:
    // Constructor and destructor
    Node(const int& id) :
        _id(id), _prev(NULL), _next(NULL) { }
    ~Node() { }

    // Basic access methods
    int getId() const       { return _id; }
    Node* getPrev() const   { return _prev; }
    Node* getNext() const   { return _next; }

    // Set functions
    void setId(const int& id) { _id = id; }
    void setPrev(Node* prev)  { _prev = prev; }
    void setNext(Node* next)  { _next = next; }

private:
    int         _id;    // id of the node (indicating the cell)
    Node*       _prev;  // pointer to the previous node
    Node*       _next;  // pointer to the next node
};

class Cell
{
public:
    // Constructor and destructor
    Cell(int name, bool part, int id) :
        _gain(0), _pinNum(0), _part(part), _lock(false), _name(name) {
        _node = new Node(id);
    }
    ~Cell() { }

    // Basic access methods
    int getGain() const     { return _gain; }
    int getPinNum() const   { return _pinNum; }
    bool getPart() const    { return _part; }
    bool getLock() const    { return _lock; }
    Node* getNode() const   { return _node; }
    int getName() const  { return _name; }
    int getFirstNet() const { return _netList[0]; }
    vector<int>& getNetList()  { return _netList; }

    // Set functions
    void setNode(Node* node)        { _node = node; }
    void setGain(const int gain)    { _gain = gain; }
    void setPart(const bool part)   { _part = part; }
    void setName(const int name) { _name = name; }
    void setPinNum(const int pinNum) {_pinNum = pinNum;}

    // Modify methods
    void move()         { _part = !_part; }
    void lock()         { _lock = true; }
    void unlock()       { _lock = false; }
    void incGain()      { ++_gain; }
    void decGain()      { --_gain; }
    void incPinNum()    { ++_pinNum; }
    void decPinNum()    { --_pinNum; }
    void addNet(const int netId) { _netList.push_back(netId); }

private:
    int             _gain;      // gain of the cell
    int             _pinNum;    // number of pins the cell are connected to
    bool            _part;      // partition the cell belongs to (0-A, 1-B)
    bool            _lock;      // whether the cell is locked
    Node*           _node;      // node used to link the cells together
    int             _name;      // name of the cell
    vector<int>     _netList;   // list of nets the cell is connected to
};

#endif  // CELL_H
