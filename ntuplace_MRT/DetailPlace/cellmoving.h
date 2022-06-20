#ifndef CELLMOVING_H
#define CELLMOVING_H

/////////////////////////////////////////////////////////
//
// cell moving by tchsu =>move cell to fit the bin density constraint
// IMPORTANT!! ALL LARGE MACROS MUST CHANGE TO "FIXED" AND THE SITES UNDER THEM SHOULD BE REMOVED BEFORE USE THIS CLASS!!
//
////////////////////////////////////////////////////////

#include "placedb.h"
#include "placebin.h"

#include <vector>
using namespace std;


class CCellmovingRowSegment
{
  public:
    CCellmovingRowSegment(double start, double length)
    {
        m_absXstart = start;
        m_length = length;

    };

    ~CCellmovingRowSegment()
    {
    };
    map < double, int >m_rowmodule;     //x-coordinate,module ID
    double m_absXstart;
    double m_length;
    bool insert_module(CPlaceDB & db, int mID);
    void findEmptySpace(const double &cutLine, CPlaceDB & db, bool isRight,
                        double &empty);
    void shiftCell(const double &cutLine, CPlaceDB & db, bool isRight,
                   double &step);

    //x: the x coordinate of the first module to move
    //length: the distance between the new x of the module and the cutLine
    void findFirstModule(const double &cutLine, CPlaceDB & db,
                         bool isRight, double &step, double &x,
                         double &length);

    //update module location according to x and length (prevent overlapping)
    void updateRowModule(const double &cutLine, CPlaceDB & db,
                         bool isRight, const double &x,
                         const double &length);
    void printRow(const double &cutLine);
    void printRow(CPlaceDB & db, const double &cutLine);
};



class CCellmoving
{
  public:
    CCellmoving(CPlaceDB & db)
    {
        m_pDB = &db;
        showMsg = true;
    };

    ~CCellmoving()
    {
    };

    void redistributeCell(double targetUtil);


  private:

    bool showMsg;
    vector < vector < CCellmovingRowSegment > >m_coreRows;
    vector < map < double, int > >m_coreRowsMap;
    CPlaceDB *m_pDB;

    double redistributeCellUnitRun(double targetUtil, bool isGoRight);  //return processed HPWL
    void insertModule(int mID);
    void OutputDensityGrid(double target, string filename,
                           CPlaceBin & placeBin);

    //move flux area of cells from bin1 to bin2
    void moveCell(double flux, int binX1, int binY1, int binX2, int binY2,
                  CPlaceBin & placeBin);

    //return flux area from bin 1 to bin 2
    double findFlux(double target, int binX1, int binY1, int binX2,
                    int binY2, CPlaceBin & placeBin);
    int findRowPos(double y);
    void moveSegmentCell(const double &targetStepXmove, const int &rowID,
                         const int &segID, const double &cutLine,
                         const bool & isGoRight, double &areaGain);
//      void moveSegmentSingleCell(const double& targetStepXmove, const int& rowID, const int& segID, const double& cutLine, const bool& isGoRight,double& stepGain);

//      void findCellLength(const double& cutLine, const int& rowID, const int& segID, const bool& isRight, double& space);
};


#endif
