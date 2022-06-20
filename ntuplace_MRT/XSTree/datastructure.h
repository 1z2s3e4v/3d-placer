#include <vector>
using namespace std;

struct edgeStruc   // for MST
{
    edgeStruc()
    {
	nodeNum = -1;
	triangleNum[0] = -1;
	triangleNum[1] = -1;
	triangleCount = 0;
    }
    //int edgeNum ;   // seems not used
    int nodeNum ;     // terminal index of every MST edge
    int triangleNum[2] ;
    int triangleCount ;
} ;
/*
struct location
{
    double x ;
	double y ;
	int routed ; // 1:routed   0:unrouted   -1:eliminated   
};*/

class tri
{
    public:
    tri() 
    {
	t.resize( 3 );
	MST.resize( 3 );
	wireLength = 0.0;
	enclosed = 0;
	MSTEdgeCount = 0;
	lengthImprovement = 0.0;
    }
    vector<int> t ;   //  Terminals and routing sequence
    //int t[3] ;   //  Terminals and routing sequence
    double wireLength ;
    int enclosed ; // 1: 某點在另外兩點間	
    //int MST[3] ;   // ( t[0], t[1] ), ( t[1], t[2] ), ( t[2], t[3] )  是否為MST的Edge
    vector<int> MST ;   // ( t[0], t[1] ), ( t[1], t[2] ), ( t[2], t[3] )  是否為MST的Edge
    int MSTEdgeCount ;   //此三角型有幾個MST的edge  ( 0 or 1 or 2 )
    double lengthImprovement ;
};

struct Xdistance
{
    Xdistance() : dist(0) {};
    double dist;  // true distance     
};

struct locationFloat
{
    locationFloat()
    {
	//terminalNum = 0;
	//routed = 0;
	//x = y = -1;
    }
	double x ;
	double y ;
	int routed ; // 1:routed   0:unrouted   -1:eliminated
	int terminalNum ;  // -1 for general point
} ;

struct segment
{
    segment()
    {
	//type = -1;
	//valid = 1;
	//length = 0.0;
    }
	int terminalNum1 ;
	int terminalNum2 ;

	double x1 ;
	double y1 ;

	double x2 ;
	double y2 ;

	int type ; // 0 : 水平, 1 : 垂直, 2 : 左下右上, 3 : 左上右下   ( 0垂直1, 2垂直3 )  

	int valid ; //1 : valid
	double length ;

} ;
