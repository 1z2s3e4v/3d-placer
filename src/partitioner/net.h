#ifndef NET_H
#define NET_H

#include <vector>
using namespace std;

class Net_FM
{
public:
    // constructor and destructor
    Net_FM(int name) :
        _name(name) {
        _partCount[0] = 0; _partCount[1] = 0;
    }
    ~Net_FM()  { }

    // basic access methods
    int getName()           const { return _name; }
    int getPartCount(int part) const { return _partCount[part]; }
    vector<int>& getCellList() { return _cellList; }

    // set functions
    void setName(const int name) { _name = name; }
    void setPartCount(int part, const int count) { _partCount[part] = count; }

    // modify methods
    void incPartCount(int part)     { ++_partCount[part]; }
    void decPartCount(int part)     { --_partCount[part]; }
    void addCell(const int cellId)  { _cellList.push_back(cellId); }

//private:
    int             _partCount[2];  // Cell number in partition A(0) and B(1)
    int             _name;          // Name of the net
    vector<int>     _cellList;      // List of cells the net is connected to
};

#endif  // NET_H
