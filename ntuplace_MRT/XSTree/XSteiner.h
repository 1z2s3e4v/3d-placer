#ifndef XSteinerH
#define XSTeinerH

#include "datastructure.h"
#include <vector>
#include <iostream>
using namespace std;

#include "util.h"

extern int triCount;
/*
// CPoint class for debug
class CPoint
{
   public:
      CPoint() {};
      CPoint( double x, double y ) { this->x = x;  this->y = y; }	
      void Print() { std::cout << "(" << x << "," << y << ")"; }
      double x, y;
      int routed;   // is routed?!
};*/

// end CPoint class 
class CXSteinerTree
{ 
   public:
      CXSteinerTree() : xst_parm(1), triNum(0), edgeCount(0)
      {
	  nodeTemp[0] = nodeTemp[1] = -1;
	  triCount = 0;
	  nodeCount = 0;
	  refining = 0;
	  refineNum = 0;
	  XSTLength = XSTLengthWithoutTwoTerminal = 0.0;
      };
      ~CXSteinerTree(){};
      
      void Solve(const std::vector<CPoint>& points);     // read terminal in
      void preProcess(const std::vector<CPoint>& points);
      int duplicate(CPoint temp, int count);
      void readPoints(unsigned int size);                // store terminals to p_array for triangulation
      void printTriEdges(unsigned int n);
      int printTriTriangles(unsigned int n);             // must be included in code
      void calOptTriWire(unsigned int tricount);         // compute optimal wirelength every triangle
      Xdistance regionEnclosed(CPoint a, CPoint b, CPoint c, tri& triangle);
      Xdistance calXDist(double x1, double y1, double x2, double y2);
      Xdistance ThreeTerminalWireLength(CPoint a, CPoint b, CPoint c, int &t1, int &t2, int &t3);
      locationFloat findNearestPoint(double x3, double y3, segment &seg);
      int ifMSTEdge2(int node1, int node2);
      void resetRoutedMSTOfOtherTriangle(int triNum);
      int twoTerminalRouting( int t1, int t2 );
      void deleteTerminal( int tNum );
      int oneDirReach( double X1, double Y1, double X2, double Y2 );
      void simpleTwoTerminalRouting( int t1, int t2 );
      void showXST();
      //int comTriangles(tri elem1, tri elem2);           // used for sorting triangles
      
      // public data structure starts   
      std::vector<CPoint> m_xst_array;                   // store all points of XST in this array
      int xst_parm;
   private:
      std::vector<CPoint> arr;
      std::vector<tri> triangles;                        // group of triangles after triangulation
      //Xdistance threeTerDist;
      int triNum;
      //int twoTerminalNets;                             // total num of two terminal nets (maybe no need)
      //double spanningLength;
      //double spanningLengthWithoutTwoTerminal;
      std::vector<segment> XSTseg;                       // segment of XST
      int nodeCount;
      int edgeCount;                                   // total num of XST edges
      int refining;
      int refineNum;
      int nodeTemp[2];                                   // store newly added X-Steiner points
      double XSTLength, XSTLengthWithoutTwoTerminal;
      //std::vector<int> enemyTerminal;
};

#endif
