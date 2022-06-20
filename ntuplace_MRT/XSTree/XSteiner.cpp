#include "XSteiner.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <cfloat>

#include  "defs.h"
#include  "decl.h"
#include  "extern.h"
#include  "edge.h"

using namespace std;

/* global variable declaration */
//const int BIGNUM = 10000;
const int BIGNUM =   10000;
const int NEIGHBORNUM = 50;

int triCount = 0;  // control tri index in printTriangles
int MSTDegree[BIGNUM];
edgeStruc MST[BIGNUM][NEIGHBORNUM];
int enemyTerminal[BIGNUM];
int terminalDegree[BIGNUM];
int XST[BIGNUM][NEIGHBORNUM] ;       //暴力Tree資料結構, -1表 invalid
//int edgeCount;  // must be global variables?
//int refining;
//int refineNum;
//int nodeTemp[2];
//double XSTLength, XSTLengthWithoutTwoTerminal;


//CXSteinerTree::CXSteinerTree(){}

//CXSteinerTree::~CXSteinerTree(){}

void CXSteinerTree::readPoints(unsigned int size)
{
     unsigned int i;
     
     for(i=0; i<size; i++)
     {  // datastructure of triangulation package is "double", so use cast
        p_array[i].x = (arr[i].x);
        p_array[i].y = (arr[i].y);         
     }     
}

void CXSteinerTree::printTriEdges(unsigned int n)
{
     edge *e_start, *e;
     point *u, *v;
     unsigned int i;
     for (i = 0; i < n; i++) {
        u = &p_array[i];
        e_start = e = u->entry_pt;
     
     do{
        v = Other_point(e, u);
        if (u < v)
	    if (printf("%d %d\n", u - p_array, v - p_array) == EOF)
	      panic("Error printing results\n");
        e = Next(e, u);
     }while (!Identical_refs(e, e_start));
     
     }
}

int CXSteinerTree::printTriTriangles(unsigned int n)
{
    edge *e_start, *e, *next;
    point *u, *v, *w;
    unsigned int i;
    triangles.resize(BIGNUM) ;

    for( int k = 0 ; k < BIGNUM ; k++ )
    {
	triangles[k].MSTEdgeCount = 0 ;			
    }
    //tri temp ;

    for (i = 0; i < n; i++) 
    {
	u = &p_array[i];
	e_start = e = u->entry_pt;
	do
	{
	    v = Other_point(e, u);
	    if (u < v) 
	    {
		next = Next(e, u);
		w = Other_point(next, u);
		if (u < w)
		    if (Identical_refs(Next(next, w), Prev(e, v))) 
		    {  
			/* Triangle. */
			if (v > w) 
			{ 
			    swap( v, w ); 
			}

			assert( u != v );
			assert( u != w );
			assert( v != w );

			triangles[triCount].t[0] = u - p_array ; 
			triangles[triCount].t[1] = v - p_array ; 
			triangles[triCount].t[2] = w - p_array ;  // No. of triangles starts form 0

			assert(triangles[triCount].t[0] != triangles[triCount].t[1]);
			assert(triangles[triCount].t[1] != triangles[triCount].t[2]);
			assert(triangles[triCount].t[2] != triangles[triCount].t[0]);
			//cout << triangles[triCount].t[0] << "-" << triangles[triCount].t[1] 
			//     << "-" << triangles[triCount].t[2] << endl;

			triangles[triCount].MST[0] = 0 ;
			triangles[triCount].MST[1] = 0 ;
			triangles[triCount].MST[2] = 0 ;

			//Xdistance tempDist ;
			//static int edgeNum = 0 ;
			//int temp ;

			triCount++ ;
		    }
	    }
	    /* Next edge around u. */
	    e = Next(e, u) ;
	}while (!Identical_refs(e, e_start));
    }  

    //cout << "total triangles: " << triCount << endl;
    return triCount;
}

void CXSteinerTree::calOptTriWire(unsigned int tricount)
{
     unsigned int i;
     Xdistance threeTerDist;
     
     for(i=0; i<tricount; i++)
     {
         threeTerDist = regionEnclosed(arr[triangles[i].t[0]], arr[triangles[i].t[1]], arr[triangles[i].t[2]], triangles[i]);         
         
         if(threeTerDist.dist != 0)
         {
             triangles[i].wireLength = threeTerDist.dist;
             triangles[i].lengthImprovement = 0;                
         }
         else
         {
             threeTerDist = ThreeTerminalWireLength(arr[triangles[i].t[0]], arr[triangles[i].t[1]], arr[triangles[i].t[2]], triangles[i].t[0], triangles[i].t[1], triangles[i].t[2]);
             triangles[i].wireLength = threeTerDist.dist;
             Xdistance dis1 = calXDist(arr[triangles[i].t[0]].x, arr[triangles[i].t[0]].y, arr[triangles[i].t[1]].x, arr[triangles[i].t[1]].y);
             Xdistance dis2 = calXDist(arr[triangles[i].t[1]].x, arr[triangles[i].t[1]].y, arr[triangles[i].t[2]].x, arr[triangles[i].t[2]].y ) ;
             triangles[i].lengthImprovement = dis1.dist + dis2.dist - triangles[i].wireLength;
             
	     //assert(triangles[i].lengthImprovement >= 0); // improvement should larger than original
         }         
         //cout << triangles[i].wireLength << endl;
         //triangles[i].wireLength = circumCenterRadius( arr[ triangle[i].t[0] ].x, arr[ triangle[i].t[0] ].y, arr[ triangle[i].t[1] ].x, arr[ triangle[i].t[1] ].y, arr[ triangle[i].t[2] ].x, arr[ triangle[i].t[2] ].y ) ;
     }    
}


Xdistance CXSteinerTree::regionEnclosed(CPoint a, CPoint b, CPoint c, tri& triangle)
{
     double X1, X2, X3, Y1, Y2, Y3;
     double X1new, X2new, X3new, Y1new, Y2new, Y3new;
     double halfsqrt2 = (sqrt(2)/2);   
     unsigned int i;
     int A, B, C;  // routing sequence
     Xdistance tmp1, tmp2, result;
     
     A = triangle.t[0];
     B = triangle.t[1];
     C = triangle.t[2];
     
     triangle.t[0] = A;
     triangle.t[1] = C;
     triangle.t[2] = B;
     
     for(i=0; i<3; i++)
     {
	 if(i == 0) 
	     X1 = a.x, Y1 = a.y, X2 = b.x, Y2 = b.y, X3 = c.x, Y3 = c.y; // check a,b
	 else if(i == 1)
	     X1 = c.x, Y1 = c.y, X2 = b.x, Y2 = b.y, X3 = a.x, Y3 = a.y; // check c,b
	 else
	     X1 = c.x, Y1 = c.y, X2 = a.x, Y2 = a.y, X3 = b.x, Y3 = b.y; // check c,a

	 if(min(X1, X2) <= X3 && X3 <= max(X1, X2) && min(Y1, Y2) <= Y3 && Y3 <= max(Y1, Y2))
	 {
	     tmp1 = calXDist(X1, Y1, X3, Y3);
	     tmp2 = calXDist(X2, Y2, X3, Y3);

	     result.dist = tmp1.dist + tmp2.dist;

	     if( i == 1 )
		 triangle.t[0] = C, triangle.t[1] = A, triangle.t[2] = B ; 
	     else if( i == 2 )
		 triangle.t[0] = A, triangle.t[1] = B, triangle.t[2] = C ; 

	     triangle.enclosed = 1 ;
	     return result ;           
	 }

	 X1new = ( X1 - Y1 )*halfsqrt2 ;   // rotate left 45 degrees
	 Y1new = ( X1 + Y1 )*halfsqrt2 ;

	 X2new = ( X2 - Y2 )*halfsqrt2 ;
	 Y2new = ( X2 + Y2 )*halfsqrt2 ;

	 X3new = ( X3 - Y3 )*halfsqrt2 ;
	 Y3new = ( X3 + Y3 )*halfsqrt2 ;

	 if(min(X1new, X2new) <= X3new && X3new <= max(X1new, X2new) && min(Y1new, Y2new) <= Y3new && Y3new <= max(Y1new, Y2new))
	 {
	     tmp1 = calXDist(X1new, Y1new, X3new, Y3new);
	     tmp2 = calXDist(X2new, Y2new, X3new, Y3new);

	     result.dist = tmp1.dist + tmp2.dist;

	     if( i == 1 )
		 triangle.t[0] = C, triangle.t[1] = A, triangle.t[2] = B ; 
	     else if( i == 2 )
		 triangle.t[0] = A, triangle.t[1] = B, triangle.t[2] = C ; 

	     triangle.enclosed = 1 ;
	     return result ;           
	 }                      
     }    
     
     result.dist = 0;
     
     return result;  
}

Xdistance CXSteinerTree::calXDist(double x1, double y1, double x2, double y2)
{
     Xdistance result;
     double dx, dy;
     
     dx = (x1 > x2)? x1-x2 : x2-x1;
     dy = (y1 > y2)? y1-y2 : y2-y1;
     
     if(dx > dy)
     {
         result.dist = dy*sqrt(2) + dx - dy;      
     }     
     else if(dx < dy)
     {
         result.dist = dx*sqrt(2) + dy - dx;
     }
     else
     {
         result.dist = dx*sqrt(2);
     }
     
     return result;
}

Xdistance CXSteinerTree::ThreeTerminalWireLength(CPoint a, CPoint b, CPoint c, int &t1, int &t2, int &t3)
{
     Xdistance result, tmpDist, dist;
     double t, tmpX1, tmpX2, tmpX3, tmpY1, tmpY2, tmpY3, dx, dy;
     vector<segment> seg;
     seg.resize(20);
     locationFloat nearPoint;
     int k, nearSeg = 0;
     int initial = 0, count = 0;
     
     int A = t1, B = t2, C = t3;
     Xdistance d;
     
     for(k=0; k<3; k++)
     {
         if( k == 0 )
			tmpX1 = a.x, tmpY1 = a.y, tmpX2 = b.x, tmpY2 = b.y, tmpX3 = c.x, tmpY3 = c.y ;  //check a b
	     else if( k == 1 )
			tmpX1 = c.x, tmpY1 = c.y, tmpX2 = b.x, tmpY2 = b.y, tmpX3 = a.x, tmpY3 = a.y ;  //check c b
		 else
			tmpX1 = a.x, tmpY1 = a.y, tmpX2 = c.x, tmpY2 = c.y, tmpX3 = b.x, tmpY3 = b.y ;  //check c a


		 if( tmpX1 > tmpX2 )
		 {
			t = tmpX1 ; tmpX1 = tmpX2 ; tmpX2 = t ;
			t = tmpY1 ; tmpY1 = tmpY2 ; tmpY2 = t ;
		 }

		 tmpDist = calXDist( tmpX1, tmpY1, tmpX2, tmpY2 ) ;
		 dx = (tmpX1 < tmpX2)? tmpX2-tmpX1 : tmpX1-tmpX2;
         dy = (tmpY1 < tmpY2)? tmpY2-tmpY1 : tmpY1-tmpY2;
         
         if(dx == 0 || dy == 0 || dx == dy)  // need refinement
         {
            initial ++;
            continue;      
         }
         
         if(dx > dy)
         {
			if( tmpY1 < tmpY2 )   //左下右上分佈
			{
				seg[count].x1 = tmpX1 ;
				seg[count].y1 = tmpY1 ;
				seg[count].x2 = tmpX1 + dy ;
				seg[count].y2 = tmpY2 ;
				seg[count].type = 2 ;
				seg[count].length = dy*sqrt(2) ;
				count++ ;

				seg[count].x1 = tmpX1 + dy ;
				seg[count].y1 = tmpY2 ;
				seg[count].x2 = tmpX2 ;
				seg[count].y2 = tmpY2 ;
				seg[count].type = 0 ;
				seg[count].length = dx - dy ;
				count++ ;

				seg[count].x1 = tmpX1 + dx - dy ;
				seg[count].y1 = tmpY1 ;
				seg[count].x2 = tmpX2 ;
				seg[count].y2 = tmpY2 ;
				seg[count].type = 2 ;
				seg[count].length = dy*sqrt(2) ;
				count++ ;

				seg[count].x1 = tmpX1 ;
				seg[count].y1 = tmpY1 ;
				seg[count].x2 = tmpX1 + dx - dy ;
				seg[count].y2 = tmpY1 ;
				seg[count].type = 0 ;
				seg[count].length = dx - dy ;
				count++ ;
		

			}
			else   //左上右下分佈
			{
				seg[count].x1 = tmpX1 ;
				seg[count].y1 = tmpY1 ;
				seg[count].x2 = tmpX1 + dx - dy ;
				seg[count].y2 = tmpY1 ;
				seg[count].type = 0 ;
				seg[count].length = dx - dy ;
				count++ ;

				seg[count].x1 = tmpX1 + dx - dy ;
				seg[count].y1 = tmpY1 ;
				seg[count].x2 = tmpX2 ;
				seg[count].y2 = tmpY2 ;
				seg[count].type = 3 ;
				seg[count].length = dy*sqrt(2) ;
				count++ ;

				seg[count].x1 = tmpX1 + dy ;
				seg[count].y1 = tmpY2 ;
				seg[count].x2 = tmpX2 ;
				seg[count].y2 = tmpY2 ;
				seg[count].type = 0 ;
				seg[count].length = dx - dy ;
				count++ ;

				seg[count].x1 = tmpX1 ;
				seg[count].y1 = tmpY1 ;
				seg[count].x2 = tmpX1 + dy ;
				seg[count].y2 = tmpY2 ;
				seg[count].type = 3 ;
				seg[count].length = dy*sqrt(2) ;
				count++ ;
		

			}
		}
		else if( dx < dy )
		{
			if( tmpY1 < tmpY2 )   //左下右上分佈
			{
				seg[count].x1 = tmpX1 ;
				seg[count].y1 = tmpY1 ;
				seg[count].x2 = tmpX1 ;
				seg[count].y2 = tmpY1 + dy - dx ;
				seg[count].type = 1 ;
				seg[count].length = dy - dx ;
				count++ ;

				seg[count].x1 = tmpX1 ;
				seg[count].y1 = tmpY1 + dy - dx ;
				seg[count].x2 = tmpX2 ;
				seg[count].y2 = tmpY2 ;
				seg[count].type = 2 ;
				seg[count].length = dx*sqrt(2) ;
				count++ ;

				seg[count].x1 = tmpX2 ;
				seg[count].y1 = tmpY1 + dx ;
				seg[count].x2 = tmpX2 ;
				seg[count].y2 = tmpY2 ;
				seg[count].type = 1 ;
				seg[count].length = dy - dx ;
				count++ ;

				seg[count].x1 = tmpX1 ;
				seg[count].y1 = tmpY1 ;
				seg[count].x2 = tmpX2 ;
				seg[count].y2 = tmpY1 + dx ;
				seg[count].type = 2 ;
				seg[count].length = dx*sqrt(2) ;
				count++ ;
		

			}
			else   //左上右下分佈
			{
				seg[count].x1 = tmpX1 ;
				seg[count].y1 = tmpY1 ;
				seg[count].x2 = tmpX2 ;
				seg[count].y2 = tmpY2 + dy - dx ;
				seg[count].type = 3 ;
				seg[count].length = dx*sqrt(2) ;
				count++ ;

				seg[count].x1 = tmpX2 ;
				seg[count].y1 = tmpY2 ;
				seg[count].x2 = tmpX2 ;
				seg[count].y2 = tmpY2 + dy - dx ;
				seg[count].type = 1 ;
				seg[count].length = dy - dx ;
				count++ ;

				seg[count].x1 = tmpX1 ;
				seg[count].y1 = tmpY2 + dx ;
				seg[count].x2 = tmpX2 ;
				seg[count].y2 = tmpY2 ;
				seg[count].type = 3 ;
				seg[count].length = dx*sqrt(2) ;
				count++ ;

				seg[count].x1 = tmpX1 ;
				seg[count].y1 = tmpY1 ;
				seg[count].x2 = tmpX1 ;
				seg[count].y2 = tmpY2 + dx ;
				seg[count].type = 1 ;
				seg[count].length = dy - dx ;
				count++ ;
			}      
         }
         
         for(int i=0; i<count; i++)
         {
             nearPoint = findNearestPoint(tmpX3, tmpY3, seg[i]);   
             d = calXDist( tmpX3, tmpY3, nearPoint.x, nearPoint.y ) ;
	     if( i == 0 )
	     {
	         dist = calXDist( tmpX3, tmpY3, nearPoint.x, nearPoint.y ) ;
	     }
	     else if(  d.dist < dist.dist )
	     {
		dist = d ; 
		nearSeg = i ;
	    }    
         }
	 tmpDist.dist += dist.dist ;
         
         if( k == initial )
		 {
			result = tmpDist ;
			if( k == 1 )
				t1 = C, t2 = B, t3 = A ;   //routing sequence
			else if( k == 2 )
				t1 = A, t2 = C, t3 = B ;   //routing sequence
		 }
		 else if( tmpDist.dist < result.dist )
		 {
			result = tmpDist ;
			if( k == 1 )
				t1 = C, t2 = B, t3 = A ;   //routing sequence
			else if( k == 2 )
				t1 = A, t2 = C, t3 = B ;   //routing sequence
	 	 }
              
         count = 0;
     }      
     
     return result;   
}

locationFloat CXSteinerTree::findNearestPoint(double X3, double Y3, segment &seg)
{
    locationFloat result, tempresult ; 
	locationFloat t1, t2, t3 ;  //旋轉暫存
	Xdistance d1, d2;
		
	double halfsqrt2 = sqrt((double)2)/2 ;
	int term = 0 ;

	if( seg.type == 3 )
	{
		t1.x = ( seg.x1 - seg.y1 )*halfsqrt2 ;
		t1.y = ( seg.x1 + seg.y1 )*halfsqrt2 ;

		t2.x = ( seg.x2 - seg.y2 )*halfsqrt2 ;
		t2.y = ( seg.x2 + seg.y2 )*halfsqrt2 ;

		t3.x = ( X3 - Y3 )*halfsqrt2 ;
		t3.y = ( X3 + Y3 )*halfsqrt2 ;

		d1 = calXDist( t3.x, t3.y, t1.x, t1.y ) ;
		d2 = calXDist( t3.x, t3.y, t2.x, t2.y ) ;

		if( (t1.x < t3.x && t3.x < t2.x) || (t2.x < t3.x && t3.x < t1.x) )
		{
			tempresult.x = t3.x ;
			tempresult.y = t1.y ;
		}
		else if( d1.dist < d2.dist )
		{
			tempresult.x = t1.x ;
			tempresult.y = t1.y ;
			term = 1 ;
		}
		else
		{
			tempresult.x = t2.x ;
			tempresult.y = t2.y ;
			term = 2 ;
		}

		result.x = ( tempresult.x + tempresult.y )*halfsqrt2 ;
		result.y = (-tempresult.x + tempresult.y )*halfsqrt2 ;
	}
	else if( seg.type == 2 )
	{
		t1.x = ( seg.x1 + seg.y1 )*halfsqrt2 ;
		t1.y = (-seg.x1 + seg.y1 )*halfsqrt2 ;

		t2.x = ( seg.x2 + seg.y2 )*halfsqrt2 ;
		t2.y = (-seg.x2 + seg.y2 )*halfsqrt2 ;

		t3.x = ( X3 + Y3 )*halfsqrt2 ;
		t3.y = (-X3 + Y3 )*halfsqrt2 ;

		d1 = calXDist( t3.x, t3.y, t1.x, t1.y ) ;
		d2 = calXDist( t3.x, t3.y, t2.x, t2.y ) ;

		if( (t1.x < t3.x && t3.x < t2.x) || (t2.x < t3.x && t3.x < t1.x) )
		{
			tempresult.x = t3.x ;
			tempresult.y = t1.y ;
		}
		else if( d1.dist < d2.dist )
		{
			tempresult.x = t1.x ;
			tempresult.y = t1.y ;
			term = 1 ;
		}
		else
		{
			tempresult.x = t2.x ;
			tempresult.y = t2.y ;
			term = 2 ;
		}

		result.x = ( tempresult.x - tempresult.y )*halfsqrt2 ;
		result.y = ( tempresult.x + tempresult.y )*halfsqrt2 ;
	}
	else if( seg.type == 1 )
	{
		t1.x = seg.x1 ;
		t1.y = seg.y1 ;

		t2.x = seg.x2 ;
		t2.y = seg.y2 ;

		t3.x = X3 ;
		t3.y = Y3 ;
		
		d1 = calXDist( t3.x, t3.y, t1.x, t1.y ) ;
		d2 = calXDist( t3.x, t3.y, t2.x, t2.y ) ;

		if( (t1.y < t3.y && t3.y < t2.y) || (t2.y < t3.y && t3.y < t1.y) )
		{
			result.x = t1.x ;
			result.y = t3.y ;
		}
		else if( d1.dist < d2.dist )
		{
			result.x = t1.x ;
			result.y = t1.y ;
			term = 1 ;
		}
		else
		{
			result.x = t2.x ;
			result.y = t2.y ;
			term = 2 ;
		}

	}
	else
	{

		t1.x = seg.x1 ;
		t1.y = seg.y1 ;

		t2.x = seg.x2 ;
		t2.y = seg.y2 ;

		t3.x = X3 ;
		t3.y = Y3 ;

		d1 = calXDist( t3.x, t3.y, t1.x, t1.y ) ;
		d2 = calXDist( t3.x, t3.y, t2.x, t2.y ) ;

		if( (t1.x < t3.x && t3.x < t2.x) || (t2.x < t3.x && t3.x < t1.x) )
		{
			result.x = t3.x ;
			result.y = t1.y ;
		}
		else if( d1.dist < d2.dist )
		{
			result.x = t1.x ;
			result.y = t1.y ;
			term = 1 ;
		}
		else
		{
			result.x = t2.x ;
			result.y = t2.y ;
			term = 2 ;
		}

	}

	if( abs( static_cast<int>((result.x+0.51) - result.x )) < 0.5  ) // cast double to int
		result.x = (int)(result.x+0.51) ;
	
	if( abs( static_cast<int>((result.y+0.51) - result.y )) < 0.5  ) // cast double to int
		result.y = (int)(result.y+0.51) ;

	if( term == 1 )
		result.terminalNum = seg.terminalNum1 ;
	else if( term == 2 )
		result.terminalNum = seg.terminalNum2 ;
	else
		result.terminalNum = -1 ;
	

	return result ;         
}

int CXSteinerTree::duplicate(CPoint temp, int count)
{
    int k;
    for(k=0; k<count; k++)
    {
        if(arr[k].x == temp.x && arr[k].y == temp.y)
             return k;         
    }    
    
    return -1;    
}

void CXSteinerTree::preProcess(const std::vector<CPoint>& points)
{
     int lines, i, count = 0;
     CPoint temp1, temp2;
     Xdistance dist; 
     int nodetemp, MSTnode1, MSTnode2;
     
     lines = points.size();
     
     if(lines == 1) return;
     //if(lines == 2) twoTerminalNets ++;
     
     for(i=0; i<lines-1; i=i+2)
     {
         temp1.x = points.at(i).x;
         temp1.y = points.at(i).y;
              
         temp2.x = points.at(i+1).x;
         temp2.y = points.at(i+1).y;
         
         dist = calXDist(temp1.x, temp1.y, temp2.x, temp2.y);

         if( (nodetemp = duplicate(temp1, count)) == -1 ) // -1: no duplicate, return terminal index adjacen to temp1
         {
             //temp1.routed = 0;  // unrouted
             arr.push_back(temp1);  
             MSTnode1 = count;
             count ++;    
         }
         else
             MSTnode1 = nodetemp;
                   
         if( (nodetemp = duplicate(temp2, count)) == -1)
         {
             //temp2.routed = 0;
             arr.push_back(temp2);
             MSTnode2 = count;
             count ++;    
         }
         else
             MSTnode2 = nodetemp;

         MST[MSTnode1][MSTDegree[MSTnode1]].nodeNum = MSTnode2; 
	 MSTDegree[MSTnode1] ++;
         MST[MSTnode2][MSTDegree[MSTnode2]].nodeNum = MSTnode1;
	 MSTDegree[MSTnode2] ++;
      }
}

bool comTriangles(tri elem1, tri elem2)
{    /*
     if( elem1.MSTEdgeCount > elem2.MSTEdgeCount && elem1.MSTEdgeCount == 2 )
		 return elem1.MSTEdgeCount > elem2.MSTEdgeCount ;
	
	 if( elem1.MSTEdgeCount < elem2.MSTEdgeCount && elem2.MSTEdgeCount == 2 )
	 	 return elem1.MSTEdgeCount < elem2.MSTEdgeCount ;
     
     if(elem1.lengthImprovement < elem2.lengthImprovement)
         return elem1.lengthImprovement < elem2.lengthImprovement;
     
     
     if(elem1.lengthImprovement > elem2.lengthImprovement)
     	 return elem1.lengthImprovement > elem2.lengthImprovement;
     */
     return elem1.wireLength < elem2.wireLength;    
}

int CXSteinerTree::ifMSTEdge2(int node1, int node2)
{
     for( int k = 0 ; k < MSTDegree[node1] ; k++ )
		if( MST[node1][k].nodeNum == node2  )
			return k ;
     
     return -1;
}

void CXSteinerTree::resetRoutedMSTOfOtherTriangle(int triNum)
{
    int temp1, tri1, tri2, updateTri ;

    if( triangles[triNum].MST[0] == 1 )
    {
	temp1 = ifMSTEdge2( triangles[triNum].t[0], triangles[triNum].t[1] ) ;
	tri1 = MST[ triangles[triNum].t[0] ][ temp1 ].triangleNum[0] ;   // 這個MST edge 的ㄧ個或兩個相鄰三角形
	tri2 = MST[ triangles[triNum].t[0] ][ temp1 ].triangleNum[1] ;

	triangles[ tri1 ].MSTEdgeCount-- ;
	triangles[triNum].MST[0] = 0 ;

	if( MST[ triangles[triNum].t[0] ][ temp1 ].triangleCount == 2 )
	{
	    triangles[ tri2 ].MSTEdgeCount-- ;

	    if( tri1 != triNum )
		updateTri = tri1 ;
	    else
		updateTri = tri2 ;

	    if(	triangles[updateTri].MST[0] == 1 && 
		    ( ( triangles[triNum].t[0] == triangles[updateTri].t[0] && triangles[triNum].t[1] == triangles[updateTri].t[1] ) 
		   || ( triangles[triNum].t[0] == triangles[updateTri].t[1] && triangles[triNum].t[1] == triangles[updateTri].t[0] ) ) )
	    {
		triangles[updateTri].MST[0] = 0 ;
	    }
	    else if( triangles[updateTri].MST[1] == 1 && 
		    ( ( triangles[triNum].t[0] == triangles[updateTri].t[1] && triangles[triNum].t[1] == triangles[updateTri].t[2] ) 
		   || ( triangles[triNum].t[0] == triangles[updateTri].t[2] && triangles[triNum].t[1] == triangles[updateTri].t[1] ) ) )
	    {
		triangles[updateTri].MST[1] = 0 ;
	    }
	    else if( triangles[updateTri].MST[2] == 1 && 
		    ( ( triangles[triNum].t[0] == triangles[updateTri].t[2] && triangles[triNum].t[1] == triangles[updateTri].t[0] ) 
		   || ( triangles[triNum].t[0] == triangles[updateTri].t[0] && triangles[triNum].t[1] == triangles[updateTri].t[2] ) ) )
	    {
		triangles[updateTri].MST[2] = 0 ;
	    }
	    else
	    {	
		/*cout << "Error: in (1) resetRoutedMSTOfOtherTriangle!" << endl;   
		exit(1); */
	    }
	}
    }

    if( triangles[triNum].MST[1] == 1 )
    {
	temp1 = ifMSTEdge2( triangles[triNum].t[1], triangles[triNum].t[2] ) ;
	tri1 = MST[ triangles[triNum].t[1] ][ temp1 ].triangleNum[0] ;
	tri2 = MST[ triangles[triNum].t[1] ][ temp1 ].triangleNum[1] ;

	triangles[ tri1 ].MSTEdgeCount-- ;
	triangles[triNum].MST[1] = 0 ;

	if( MST[ triangles[triNum].t[1] ][ temp1 ].triangleCount == 2 )
	{
	    triangles[ tri2 ].MSTEdgeCount-- ;

	    if( tri1 != triNum )
		updateTri = tri1 ;
	    else
		updateTri = tri2 ;

	    if(	triangles[updateTri].MST[0] == 1 && 
		    ( ( triangles[triNum].t[1] == triangles[updateTri].t[0] && triangles[triNum].t[2] == triangles[updateTri].t[1] ) 
		   || ( triangles[triNum].t[1] == triangles[updateTri].t[1] && triangles[triNum].t[2] == triangles[updateTri].t[0] ) ) )
	    {
		triangles[updateTri].MST[0] = 0 ;
	    }
	    else if( triangles[updateTri].MST[1] == 1 && 
		    ( ( triangles[triNum].t[1] == triangles[updateTri].t[1] && triangles[triNum].t[2] == triangles[updateTri].t[2] ) 
		   || ( triangles[triNum].t[1] == triangles[updateTri].t[2] && triangles[triNum].t[2] == triangles[updateTri].t[1] ) ) )
	    {
		triangles[updateTri].MST[1] = 0 ;
	    }
	    else if( triangles[updateTri].MST[2] == 1 && 
		    ( ( triangles[triNum].t[1] == triangles[updateTri].t[2] && triangles[triNum].t[2] == triangles[updateTri].t[0] ) 
		   || ( triangles[triNum].t[1] == triangles[updateTri].t[0] && triangles[triNum].t[2] == triangles[updateTri].t[2] ) ) )
	    {
		triangles[updateTri].MST[2] = 0 ;
	    }
	    else
	    {	
		/*cout << "Error: in (2) resetRoutedMSTOfOtherTriangle!" << endl;   
		exit(1); */
	    }
	}
    }
    
    if( triangles[triNum].MST[2] == 1 )
    {
	temp1 = ifMSTEdge2( triangles[triNum].t[2], triangles[triNum].t[0] ) ;
	tri1 = MST[ triangles[triNum].t[2] ][ temp1 ].triangleNum[0] ;
	tri2 = MST[ triangles[triNum].t[2] ][ temp1 ].triangleNum[1] ;

	triangles[ tri1 ].MSTEdgeCount-- ;
	triangles[triNum].MST[2] = 0 ;

	if( MST[ triangles[triNum].t[2] ][ temp1 ].triangleCount == 2 )
	{
	    triangles[ tri2 ].MSTEdgeCount-- ;
	    if( tri1 != triNum )
		updateTri = tri1 ;
	    else
		updateTri = tri2 ;

	    if(	triangles[updateTri].MST[0] == 1 && 
		    ( ( triangles[triNum].t[2] == triangles[updateTri].t[0] && triangles[triNum].t[0] == triangles[updateTri].t[1] ) 
		   || ( triangles[triNum].t[2] == triangles[updateTri].t[1] && triangles[triNum].t[0] == triangles[updateTri].t[0] ) ) )
	    {
		triangles[updateTri].MST[0] = 0 ;
	    }
	    else if( triangles[updateTri].MST[1] == 1 && 
		    ( ( triangles[triNum].t[2] == triangles[updateTri].t[1] && triangles[triNum].t[0] == triangles[updateTri].t[2] ) 
		   || ( triangles[triNum].t[2] == triangles[updateTri].t[2] && triangles[triNum].t[0] == triangles[updateTri].t[1] ) ) )
	    {
		triangles[updateTri].MST[1] = 0 ;
	    }
	    else if( triangles[updateTri].MST[2] == 1 && 
		    ( ( triangles[triNum].t[2] == triangles[updateTri].t[2] && triangles[triNum].t[0] == triangles[updateTri].t[0] ) 
		   || ( triangles[triNum].t[2] == triangles[updateTri].t[0] && triangles[triNum].t[0] == triangles[updateTri].t[2] ) ) )
	    {
		triangles[updateTri].MST[2] = 0 ;
	    }
	    else
	    {	
		/*cout << "Error: in (3) resetRoutedMSTOfOtherTriangle!" << endl;   
		printf( "MST[0] = %d  triNum t %d %d %d  updateTri %d %d %d  \n", 
			triangles[updateTri].MST[0],
		     triangles[triNum].t[0], triangles[triNum].t[1] , triangles[triNum].t[2],
		   triangles[updateTri].t[0], triangles[updateTri].t[1], triangles[updateTri].t[2]  );
		exit(1); */
	    }
	}

    }     
}

void CXSteinerTree::deleteTerminal( int tNum )  //消除某一個Terminal
{
	int connectTerminal, temp, tt ;
        int k;
	//arr[ tNum ].routed = -1 ;


	tt = terminalDegree[tNum] ;
	for(  k = 0 ; k < tt ; k++ )
	{
		if( XST[ tNum ][k] == -1 )
		{
			tt++;
			continue ;
		}

		connectTerminal = XST[ tNum ][k] ;
		temp = terminalDegree[ connectTerminal ] ;
		for( int s = 0 ; s < temp ; s++ )  //把連到此點的記錄消除
		{
			if( XST[ connectTerminal ][ s ] == tNum )
			{
				XST[ connectTerminal ][ s ] = -1 ;
				terminalDegree[ connectTerminal ]-- ;
				break ;
			}
			else if( XST[ connectTerminal ][ s ] == -1 )
				temp++ ;

		}
		XST[ tNum ][k] = -1 ;	

	}

	int edgeDelete = 0 ;

	for( k = 0 ; k < edgeCount ; k++ )
	{
	    if( XSTseg[k].terminalNum1 == tNum || XSTseg[k].terminalNum2 == tNum )
	    {
		XSTseg.erase( XSTseg.begin() + k ) ;
		k-- ;
		edgeDelete++ ;
	    }

	    if( edgeDelete == terminalDegree[ tNum ] )
		break ;

	}

	edgeCount -= edgeDelete ;
		
	terminalDegree[ tNum ] = 0 ;
	enemyTerminal[ enemyTerminal[ tNum ] ] = -1 ;
	enemyTerminal[ tNum ] = -1 ;

}

int CXSteinerTree::oneDirReach( double X1, double Y1, double X2, double Y2 )  // -1 : No reach ;  0 : 水平, 1 : 垂直, 2 : 左下右上, 3 : 左上右下
{
    int type = -1 ;
    double X1new, X2new, Y1new, Y2new ;
    double halfsqrt2 = sqrt((double)2)/2 ;

    if( X1 - X2 < 0.2 && X1 - X2 > -0.2 )
	type = 1 ;
    else if( Y1 - Y2 < 0.2 && Y1 - Y2 > -0.2 )
	type = 0 ;
    else 
    {
	X1new = ( X1 - Y1 )*halfsqrt2 ;   //左旋45度
	Y1new = ( X1 + Y1 )*halfsqrt2 ;

	X2new = ( X2 - Y2 )*halfsqrt2 ;
	Y2new = ( X2 + Y2 )*halfsqrt2 ;

	if( X1new - X2new < 0.2 && X1new - X2new > -0.2 )
	    type = 2 ;
	else if( Y1new - Y2new < 0.2 && Y1new - Y2new > -0.2 )
	    type = 3 ;
    }

    return type ;
}

void CXSteinerTree::simpleTwoTerminalRouting( int t1, int t2 ) 
{

    int dx, dy ;

    dx = abs( static_cast<int>(arr[t1].x - arr[t2].x) ) ;
    dy = abs( static_cast<int>(arr[t1].y - arr[t2].y) ) ;


    int t3 = nodeCount ;
    nodeCount++ ;

    if( arr[t1].y < arr[t2].y && dx > dy )  //左下右上低角度分布
    {
	arr[t3].x = arr[t1].x + dy ;
	arr[t3].y = arr[t2].y ;


    }
    else if( arr[t1].y > arr[t2].y && dx > dy )  //左上右下低角度分布
    {
	arr[t3].x = arr[t1].x + dx - dy ;
	arr[t3].y = arr[t1].y ;


    }
    else if( arr[t1].y < arr[t2].y && dx < dy )  //左下右上高角度分布
    {
	arr[t3].x = arr[t1].x ;
	arr[t3].y = arr[t1].y + dy - dx ;


    }
    else if( arr[t1].y > arr[t2].y && dx < dy )  //左上右下高角度分布
    {  
	arr[t3].x = arr[t1].x ;
	arr[t3].y = arr[t2].y + dx ;


    }
    else
	printf("Exception1\n") ;


    XSTseg[edgeCount].x1 = arr[t1].x ;
    XSTseg[edgeCount].y1 = arr[t1].y ;
    XSTseg[edgeCount].x2 = arr[t3].x ;
    XSTseg[edgeCount].y2 = arr[t3].y ;
    XSTseg[edgeCount].terminalNum1 = t1 ;
    XSTseg[edgeCount].terminalNum2 = t3 ;
    XSTseg[edgeCount].valid = 1 ;
    XSTseg[edgeCount].type = oneDirReach( arr[t1].x, arr[t1].y, arr[t3].x, arr[t3].y )  ;
    edgeCount++ ;

    XSTseg[edgeCount].x1 = arr[t2].x ;
    XSTseg[edgeCount].y1 = arr[t2].y ;
    XSTseg[edgeCount].x2 = arr[t3].x ;
    XSTseg[edgeCount].y2 = arr[t3].y ;
    XSTseg[edgeCount].terminalNum1 = t2 ;
    XSTseg[edgeCount].terminalNum2 = t3 ;
    XSTseg[edgeCount].valid = 1 ;
    XSTseg[edgeCount].type = oneDirReach( arr[t2].x, arr[t2].y, arr[t3].x, arr[t3].y )  ;
    edgeCount++ ;


    /*
    arr[t1].routed = 1 ;
    arr[t2].routed = 1 ;
    arr[t3].routed = 1 ;
    */
    int degreeTemp ;

    degreeTemp = terminalDegree[t1]++ ;
    while(1)
    {		
	if( XST[t1][ degreeTemp ] == -1 )
	{
	    XST[t1][ degreeTemp ] = t3 ;
	    break ;
	}
	else
	    degreeTemp++ ;
    }


    degreeTemp = terminalDegree[t2]++ ;
    while(1)
    {		
	if( XST[t2][ degreeTemp ] == -1 )
	{
	    XST[t2][ degreeTemp ] = t3 ;
	    break ;
	}
	else
	    degreeTemp++ ;
    }

    degreeTemp = terminalDegree[t3]++ ;
    while(1)
    {		
	if( XST[t3][ degreeTemp ] == -1 )
	{
	    XST[t3][ degreeTemp ] = t1 ;
	    break ;
	}
	else
	    degreeTemp++ ;
    }

    degreeTemp = terminalDegree[t3]++ ;
    while(1)
    {		
	if( XST[t3][ degreeTemp ] == -1 )
	{
	    XST[t3][ degreeTemp ] = t2 ;
	    break ;
	}
	else
	    degreeTemp++ ;
    }
}


int CXSteinerTree::twoTerminalRouting( int t1, int t2 )
{
    //printf( "two ter routing %d %d\n", t1, t2 );
    if( t1 == t2 )
    {
	printf( "t1 %d    t2 %d\n", t1, t2 );
	cout << "Error: in twoTerminalRouting!!" << endl;
    }

    int type;
    int t3, t4, temp ;
    double dx, dy;
    int degreeTemp ;
    segment tmpXSTseg;

    if( enemyTerminal[t1] != -1 ) //delete enemy
        deleteTerminal( enemyTerminal[t1] ) ;

    if( enemyTerminal[t2] != -1 ) //delete enemy
        deleteTerminal( enemyTerminal[t2] ) ;

    if( arr[t1].x > arr[t2].x )
	temp = t1, t1 = t2, t2 = temp ;

    assert(t1 < nodeCount); assert(t2 < nodeCount); 
    type = oneDirReach( arr[t1].x, arr[t1].y, arr[t2].x, arr[t2].y ) ;
    //cout << type << endl;
    if( type != -1 ) //一個segment可連接兩點
    {	
	tmpXSTseg.x1 = arr[t1].x ;
	tmpXSTseg.y1 = arr[t1].y ;

	tmpXSTseg.x2 = arr[t2].x ;
	tmpXSTseg.y2 = arr[t2].y ;

	tmpXSTseg.terminalNum1 = t1 ;
	tmpXSTseg.terminalNum2 = t2 ;

	tmpXSTseg.valid = 1 ;

	tmpXSTseg.type = type  ;

	XSTseg.push_back(tmpXSTseg);

	edgeCount++ ;

	/*
	arr[t1].routed = 1 ;
        arr[t2].routed = 1 ;
	*/

	degreeTemp = terminalDegree[t1]++ ;
	while(1)
	{		
	    if( XST[t1][ degreeTemp ] == -1 )
	    {
		XST[t1][ degreeTemp ] = t2 ;
		break ;
	    }
	    else
		degreeTemp++ ;
	}

	degreeTemp = terminalDegree[t2]++ ;
	while(1)
	{		
	    if( XST[t2][ degreeTemp ] == -1 )
	    {
		XST[t2][ degreeTemp ] = t1 ;
		break ;
	    }
	    else
		degreeTemp++ ;
	}

	return 0 ;  // no terminal added
    }
    else  //做四個segments及兩個terminals (互為enemy)
    {
	refining = 0;
	if( refining == 1 )
	{
	    printf("Exception4") ;

	    printf("%f %f, %f %f\n", arr[t1].x, arr[t1].y, arr[t2].x, arr[t2].y ) ;
	    printf("%d", refineNum ) ;

	    simpleTwoTerminalRouting( t1, t2 ) ;
	}
	else
	{
	    t3 = nodeCount ;
	    t4 = nodeCount + 1 ;

	    if( t4 > (int)arr.size() )
		arr.resize( t4+1 );
	    
	    nodeCount += 2 ;

	    //dx = abs( static_cast<int>(arr[t1].x - arr[t2].x) ) ;
	    //dy = abs( static_cast<int>(arr[t1].y - arr[t2].y) ) ;
            dx = (arr[t1].x > arr[t2].x) ? arr[t1].x-arr[t2].x : arr[t2].x-arr[t1].x;
	    dy = (arr[t1].y > arr[t2].y) ? arr[t1].y-arr[t2].y : arr[t2].y-arr[t1].y;
	    
	    
	    nodeTemp[0] = t3 ;
	    nodeTemp[1] = t4 ;

	    assert(t3 < nodeCount);  assert(t4 < nodeCount);
	    if( arr[t1].y < arr[t2].y && dx > dy )  //左下右上低角度分布
	    {
		arr[t3].x = arr[t1].x + dy ;
		arr[t3].y = arr[t2].y ;

		arr[t4].x = arr[t1].x + dx - dy ;
		arr[t4].y = arr[t1].y ;
	    }
	    else if( arr[t1].y > arr[t2].y && dx > dy )  //左上右下低角度分布
	    {
		arr[t3].x = arr[t1].x + dx - dy ;
		arr[t3].y = arr[t1].y ;

		arr[t4].x = arr[t1].x + dy ;
		arr[t4].y = arr[t2].y ;
	    }
	    else if( arr[t1].y < arr[t2].y && dx < dy )  //左下右上高角度分布
	    { 
		arr[t3].x = arr[t1].x ;
		arr[t3].y = arr[t1].y + dy - dx ;

		arr[t4].x = arr[t2].x ;
		arr[t4].y = arr[t1].y + dx ;
	    }
	    else if( arr[t1].y > arr[t2].y && dx < dy )  //左上右下高角度分布
	    {
		arr[t3].x = arr[t1].x ;
		arr[t3].y = arr[t2].y + dx ;

		arr[t4].x = arr[t2].x ;
		arr[t4].y = arr[t2].y + dy - dx ;
	    }
	    else
	    {   cout << "t1: " << arr[t1].x << "," << arr[t1].y << endl;
		cout << "t2: " << arr[t2].x << "," << arr[t2].y << endl;
		cout << "dx: " << dx << "," << "dy: " << dy << endl;
		cout << "Error: in 4 parts of twoTerminalRouting!" << endl;  exit(1); }

	    tmpXSTseg.x1 = arr[t1].x ;
	    tmpXSTseg.y1 = arr[t1].y ;
	    tmpXSTseg.x2 = arr[t3].x ;
	    tmpXSTseg.y2 = arr[t3].y ;
	    tmpXSTseg.terminalNum1 = t1 ;
	    tmpXSTseg.terminalNum2 = t3 ;
	    tmpXSTseg.valid = 1 ;
	    tmpXSTseg.type = oneDirReach( arr[t1].x, arr[t1].y, arr[t3].x, arr[t3].y )  ;
	    edgeCount++ ;

	    XSTseg.push_back(tmpXSTseg);

	    tmpXSTseg.x1 = arr[t1].x ;
	    tmpXSTseg.y1 = arr[t1].y ;
	    tmpXSTseg.x2 = arr[t4].x ;
	    tmpXSTseg.y2 = arr[t4].y ;
	    tmpXSTseg.terminalNum1 = t1 ;
	    tmpXSTseg.terminalNum2 = t4 ;
	    tmpXSTseg.valid = 1 ;
	    tmpXSTseg.type = oneDirReach( arr[t1].x, arr[t1].y, arr[t4].x, arr[t4].y )  ;
	    edgeCount++ ;

	    XSTseg.push_back(tmpXSTseg);

	    tmpXSTseg.x1 = arr[t2].x ;
	    tmpXSTseg.y1 = arr[t2].y ;
	    tmpXSTseg.x2 = arr[t4].x ;
	    tmpXSTseg.y2 = arr[t4].y ;
	    tmpXSTseg.terminalNum1 = t2 ;
	    tmpXSTseg.terminalNum2 = t4 ;
	    tmpXSTseg.valid = 1 ;
	    tmpXSTseg.type = oneDirReach( arr[t2].x, arr[t2].y, arr[t4].x, arr[t4].y )  ;
	    edgeCount++ ;

	    XSTseg.push_back(tmpXSTseg);

	    tmpXSTseg.x1 = arr[t2].x ;
	    tmpXSTseg.y1 = arr[t2].y ;
	    tmpXSTseg.x2 = arr[t3].x ;
	    tmpXSTseg.y2 = arr[t3].y ;
	    tmpXSTseg.terminalNum1 = t2 ;
	    tmpXSTseg.terminalNum2 = t3 ;
	    tmpXSTseg.valid = 1 ;
	    tmpXSTseg.type = oneDirReach( arr[t2].x, arr[t2].y, arr[t3].x, arr[t3].y )  ;
	    edgeCount++ ;

	    XSTseg.push_back(tmpXSTseg);

	    /*
	    arr[t1].routed = 1 ;
	    arr[t2].routed = 1 ;
	    arr[t3].routed = 1 ;
	    arr[t4].routed = 1 ;
	    */

	    int degreeTemp = 0;
            
	    degreeTemp = terminalDegree[t1]++ ;
	    while(1)
	    {		
		if( XST[t1][ degreeTemp ] == -1 )
		{
		    XST[t1][ degreeTemp ] = t3 ;
		    break ;
		}
		else
		    degreeTemp++ ;
	    }
            
	    degreeTemp = terminalDegree[t1]++ ;
	    while(1)
	    {		
		if( XST[t1][ degreeTemp ] == -1 )
		{
		    XST[t1][ degreeTemp ] = t4 ;
		    break ;
		}
		else
		    degreeTemp++ ;
	    }

	    degreeTemp = terminalDegree[t2]++ ;
	    while(1)
	    {		
		if( XST[t2][ degreeTemp ] == -1 )
		{
		    XST[t2][ degreeTemp ] = t3 ;
		    break ;
		}
		else
		    degreeTemp++ ;
	    }

	    degreeTemp = terminalDegree[t2]++ ;
	    while(1)
	    {		
		if( XST[t2][ degreeTemp ] == -1 )
		{
		    XST[t2][ degreeTemp ] = t4 ;
		    break ;
		}
		else
		    degreeTemp++ ;
	    }

	    degreeTemp = terminalDegree[t3]++ ;
	    while(1)
	    {		
		if( XST[t3][ degreeTemp ] == -1 )
		{
		    XST[t3][ degreeTemp ] = t1 ;
		    break ;
		}
		else
		    degreeTemp++ ;
	    }

	    degreeTemp = terminalDegree[t3]++ ;
	    while(1)
	    {		
		if( XST[t3][ degreeTemp ] == -1 )
		{
		    XST[t3][ degreeTemp ] = t2 ;
		    break ;
		}
		else
		    degreeTemp++ ;
	    }

	    degreeTemp = terminalDegree[t4]++ ;
	    while(1)
	    {		
		if( XST[t4][ degreeTemp ] == -1 )
		{
		    XST[t4][ degreeTemp ] = t1 ;
		    break ;
		}
		else
		    degreeTemp++ ;
	    }


	    degreeTemp = terminalDegree[t4]++ ;
	    while(1)
	    {		
		if( XST[t4][ degreeTemp ] == -1 )
		{
		    XST[t4][ degreeTemp ] = t2 ;
		    break ;
		}
		else
		    degreeTemp++ ;
	    }
            assert(degreeTemp < NEIGHBORNUM); 
	    if( t3 == t1 || t2 == t3 || t1 == t4 || t4 == t2 )
	    {
		cout << "Error: in twoTerminalRouting!" << endl;
		exit(1);
	    }
            
	    assert(t3 < nodeCount); 
	    assert(t4 < nodeCount);
	    enemyTerminal[t3] = t4 ;
	    enemyTerminal[t4] = t3 ;
	    return 2 ;  // 2 terminals added
	}

    }

    printf( "ERROR" );
    fflush( stdout );
    return -1;
}

void initMSTandXST(int XST[][NEIGHBORNUM], edgeStruc MST[][NEIGHBORNUM])
{
    int e1, e2 ;

    for(e1=0; e1<BIGNUM; e1++)
	for(e2=0; e2<NEIGHBORNUM ;e2++)
	    XST[e1][e2] = -1 ;

    for(e1=0; e1<BIGNUM; e1++)
	for(e2=0; e2<NEIGHBORNUM; e2++)
	{
	    MST[e1][e2].nodeNum = -1 ;
	    MST[e1][e2].triangleCount = 0 ;
	}     

    for(e1=0; e1<BIGNUM; e1++)
	MSTDegree[e1] = 0;
}

void CXSteinerTree::Solve(const std::vector<CPoint>& points)
{ 
     initMSTandXST(XST, MST);       
	  	
     preProcess(points); 
    
     xst_parm = 1;
     /*
     for( unsigned int i=0; i<arr.size(); i++ )
     {
	 printf( "%d  [%f %f]\n", i, arr[i].x, arr[i].y );
     }*/
          
     double xMax = arr[0].x;
     double xMin = arr[0].x;
     double yMax = arr[0].y;
     double yMin = arr[0].y;
     for( unsigned int i=1; i<arr.size(); i++ )
     {
	 if( arr[i].x > xMax ) 
	     xMax = arr[i].x;
	 if( arr[i].x < xMin ) 
	     xMin = arr[i].x;
	 if( arr[i].y > yMax ) 
	     yMax = arr[i].y;
	 if( arr[i].y < yMin ) 
	     yMin = arr[i].y;
	 if( (xMax - xMin) == 0 || (yMax - yMin) == 0 )
	 {
	     m_xst_array = points; // keep the mst result
	     return;
	 }
     }

 
     /*
     cout << "point size: " << points.size() << endl;
     for(unsigned int q=0; q<points.size(); q++)
	 cout << points[q].x << "," << points[q].y << endl;
     */
     /*
     for(unsigned int s=0; s<arr.size(); s++)
	 cout << s << " " << arr[s].x << "," << arr[s].y << endl;
     */
     // Delaunay triangulation starts  
     ///////////////////////////////////////////////////////////////////////////
     cardinal trin; // store num of terminal
     edge *l_cw, *r_ccw;
     unsigned int i;
     point **p_sorted, **p_temp;
     
     trin = arr.size();  // assign total num of terminals to n
     
     if(trin <= 0) { cout << "Error: no terminals in!" << endl;  exit(1); }
     else if(trin == 2) {  m_xst_array = arr;  return; }
     else if(trin < 2) { cout << "Error: no enough terminal, maybe just have one terminal!" << endl; exit(1); }
     else{
         alloc_memory(trin);  // defined in memory.cpp
         readPoints(trin); 
          
         for(i=0; i<trin; i++)
            p_array[i].entry_pt = NULL;
         
        //p_sorted = (point **)malloc((unsigned)n*sizeof(point *));
        
	 p_sorted = new point*[trin];
	
		if (p_sorted == NULL)
			panic("triangulate: not enough memory\n");
		
		//p_temp = (point **)malloc((unsigned)n*sizeof(point *));
		
		p_temp = new point*[trin];
		
		if (p_temp == NULL)
			panic("triangulate: not enough memory\n");
		for (i = 0; i < trin; i++)
			p_sorted[i] = p_array + i;

		merge_sort(p_sorted, p_temp, 0, trin-1);
		  
		//free((char *)p_temp);  
		delete [] p_temp;
		
		divide(p_sorted, 0, trin-1, &l_cw, &r_ccw); // triangulate!

		//free((char *)p_sorted);
		delete [] p_sorted;

		triNum = printTriTriangles(trin);    // triangulation complete and print results (cannot remove!!)
       
	if( trin == 3 )  // modify bug of triangluation package 
	{
	    triNum = 1 ;
	    triangles[0].t[1] = 1 ;  // bug of package too ???????????
	    triangles[0].t[2] = 2 ;
	}

	//////////////////////////////////////////////////////////////////////////
	//cout << "Num of tri: " << triNum << endl;
		
        for(int blute_force=0; blute_force<BIGNUM; blute_force++)
             terminalDegree[blute_force] = 0;   // record terminal degree of every terminal (including enemy)
        
        if(trin > 2)
           calOptTriWire(triNum);	 // compute optimal routing wirelength within three terminals
        
	/*
	cout << "triangles: " << endl;
	for(unsigned int t=0; t<triNum; t++)
	{
	   cout << triangles[t].t[0] << " " << triangles[t].t[1] << " " << triangles[t].t[2] << endl;
	   //cout << arr[triangles[t].t[0]].x << "," << arr[triangles[t].t[0]].y << endl; 
        }*/
	
        sort( triangles.begin(), triangles.begin()+triNum, comTriangles );	// sort by optimal wirelength 
        
	for(i=0; i<static_cast<unsigned int>(triNum); i++)
	{
	    int temp1, temp2 ;

	    if( (temp1 = ifMSTEdge2( triangles[i].t[0], triangles[i].t[1] )) != -1 ) // this edge is MST edge
	    {
		triangles[i].MST[0] = 1 ;
		triangles[i].MSTEdgeCount++ ;

		temp2 = ifMSTEdge2( triangles[i].t[1], triangles[i].t[0] ) ;

		MST[ triangles[i].t[0] ][ temp1 ].triangleNum[  MST[ triangles[i].t[0] ][ temp1 ].triangleCount++  ] = i ;
		MST[ triangles[i].t[1] ][ temp2 ].triangleNum[  MST[ triangles[i].t[1] ][ temp2 ].triangleCount++  ] = i ;
	    }

	    if( (temp1 = ifMSTEdge2( triangles[i].t[2], triangles[i].t[1] )) != -1 )
	    {
		triangles[i].MST[1] = 1 ;
		triangles[i].MSTEdgeCount++ ;

		temp2 = ifMSTEdge2( triangles[i].t[1], triangles[i].t[2] ) ;

		MST[ triangles[i].t[2] ][ temp1 ].triangleNum[  MST[ triangles[i].t[2] ][ temp1 ].triangleCount++  ] = i ;
		MST[ triangles[i].t[1] ][ temp2 ].triangleNum[  MST[ triangles[i].t[1] ][ temp2 ].triangleCount++  ] = i ;
	    }

	    if( (temp1 = ifMSTEdge2( triangles[i].t[2], triangles[i].t[0] )) != -1 )
	    {
		triangles[i].MST[2] = 1 ;
		triangles[i].MSTEdgeCount++ ;

		temp2 = ifMSTEdge2( triangles[i].t[0], triangles[i].t[2] ) ;

		MST[ triangles[i].t[2] ][ temp1 ].triangleNum[  MST[ triangles[i].t[2] ][ temp1 ].triangleCount++  ] = i ;
		MST[ triangles[i].t[0] ][ temp2 ].triangleNum[  MST[ triangles[i].t[0] ][ temp2 ].triangleCount++  ] = i ;
	    }         
	}// end triNum for 
       	
	nodeCount = trin;  // terminal num

	for(int s=0; s<BIGNUM; s++)
	    enemyTerminal[s] = -1;

        
	
	/*
	cout << "MSTEdgeCount: " << endl;
	
	for(int f=0; f<triNum; f++)
	{
	   //cout << triangles[f].t[0] << "," << triangles[f].t[1] << "," << triangles[f].t[2]
	     cout   << " " << triangles[f].MSTEdgeCount << "," << triangles[f].enclosed << endl;
	}*/
	//cout << triangles[4].t[0] << " " << triangles[4].t[1] << " " << triangles[4].t[2] << endl;
	
	//int nothingdo = 0;
        for(int p=0; p<3; p++)
           for(i=0; i<static_cast<unsigned int>(triNum); i++) // routing
	   {
	       //int deleted = 0;
	       int increasedTerminalNum = 0;
	       Xdistance dis1, dis2 ;
	       if( p == 0 && triangles[i].MSTEdgeCount == 2 && triangles[i].enclosed == 0 ) // first route two pins with 2-MST edge
	       {
		   resetRoutedMSTOfOtherTriangle( i ) ;
		   
		   if( triangles[i].enclosed == 1 )
		   {
		       assert( triangles[i].t[0] !=  triangles[i].t[1] );
		       assert( triangles[i].t[1] !=  triangles[i].t[2] );
		       twoTerminalRouting( triangles[i].t[0], triangles[i].t[1] ) ;
		       
		       twoTerminalRouting( triangles[i].t[1], triangles[i].t[2] ) ;

		       cout << "Error: first if in two-terminal net routing!" << endl;
		       exit(1);
		   }
		   else
		   {
		       //nodeTemp[0] = triangles[i].t[0], nodeTemp[1] = triangles[i].t[1];
		       increasedTerminalNum = twoTerminalRouting( triangles[i].t[0], triangles[i].t[1]);
		       
		       dis1 = calXDist( arr[ triangles[i].t[2] ].x, arr[ triangles[i].t[2] ].y, arr[ nodeTemp[0] ].x, arr[ nodeTemp[0] ].y ) ;
		       dis2 = calXDist( arr[ triangles[i].t[2] ].x, arr[ triangles[i].t[2] ].y, arr[ nodeTemp[1] ].x, arr[ nodeTemp[1] ].y ) ;
		       /*
		       if(increasedTerminalNum == 0) 
		       { nodeTemp[0] = triangles[i].t[0], nodeTemp[1] = triangles[i].t[1]; }
                       */
		       assert(nodeTemp[0] < nodeCount);
		       assert(nodeTemp[1] < nodeCount);
		       if( dis1.dist <= dis2.dist )
		       {
			   assert( triangles[i].t[0] !=  triangles[i].t[2] );
			   twoTerminalRouting( triangles[i].t[2], nodeTemp[0] ) ;
		       }
		       else
		       {
			   assert( triangles[i].t[1] !=  triangles[i].t[2] );
			   twoTerminalRouting( triangles[i].t[2], nodeTemp[1] ) ;
		       }

		   }
	       }// end if

	       else if( p == 1 && triangles[i].MSTEdgeCount == 2 && triangles[i].enclosed == 1 ) // route three pins with optimal wire topology
	       {
		   resetRoutedMSTOfOtherTriangle( i ) ;
		   
		   assert( triangles[i].t[0] !=  triangles[i].t[1] );
		   assert( triangles[i].t[1] !=  triangles[i].t[2] );
		   twoTerminalRouting( triangles[i].t[0], triangles[i].t[1] ) ;
		   twoTerminalRouting( triangles[i].t[1], triangles[i].t[2] ) ;
	       }
	       else if( p == 2 && triangles[i].MSTEdgeCount == 1 ) // route terminals with rest edges
	       {
		   if( triangles[i].MST[0] == 1 )
		   {
		       assert( triangles[i].t[0] !=  triangles[i].t[1] );
		       twoTerminalRouting( triangles[i].t[0], triangles[i].t[1] ) ;
		       resetRoutedMSTOfOtherTriangle( i ) ;
		   }		
		   else if( triangles[i].MST[1] == 1 )
		   {
		       assert( triangles[i].t[2] !=  triangles[i].t[1] );
		       
		       twoTerminalRouting( triangles[i].t[1], triangles[i].t[2] ) ;
		       resetRoutedMSTOfOtherTriangle( i ) ;
		   }

		   else if( triangles[i].MST[2] == 1 )
		   {
		       assert( triangles[i].t[0] !=  triangles[i].t[2] );
		       twoTerminalRouting( triangles[i].t[2], triangles[i].t[0] ) ;
		       resetRoutedMSTOfOtherTriangle( i ) ;
		   }

	       }// end else if
	       else  if(  triangles[i].MSTEdgeCount == 0 )
	       {   
		   m_xst_array = points;
		   xst_parm = 0;;
		   return;		   
		   //nothingdo ++;
	       }
	   }// end triNum for
           
	   /*
	   if(nothingdo >= 1)
	   {
	       m_xst_array = points;
	       xst_parm = 0;
	       return;
	   }*/
           
	
           int k;
           for(k = 0 ; k < BIGNUM ; k++ )   // eliminate redundant edges (newly added in twoTeriminalRouting)
   	   { 
   	       if( enemyTerminal[k] != -1 )
	       {
		   deleteTerminal(enemyTerminal[k]) ;	
	       }
           }
           
	       assert((unsigned int)edgeCount == XSTseg.size());
               /*
	       if(XSTseg.size() < points.size()/2) 
	       {
	            cout << "point size: " << points.size() << endl;
		     for(unsigned int q=0; q<points.size(); q++)
		        cout << points[q].x << "," << points[q].y << endl;
			    
	       }*/
	      
	      if( XSTseg.size() < points.size()/2 )
	      {
		  m_xst_array = points;
		  xst_parm = 0;
		  return;
		  /*
		  printf( "XST.size= %d point.size= %d\n", 
			  XSTseg.size(), points.size() );
		  printf( "point = " );
		  for( unsigned int p=0; p<points.size(); p++ )
		  {
		      printf( "(%.0f %.0f) ", points[p].x, points[p].y );
		  }
		  cout << "XSTseg: " << endl;
		  for( unsigned int q=0; q<edgeCount; q++)
		  {
		      cout << "(" << XSTseg[q].x1 << "," << XSTseg[q].y1 << ")" 
			   << " (" << XSTseg[q].x2 << "," << XSTseg[q].y2 << ")" << endl;
		  }
		  printf( "\n" );*/
	      } 
	       assert(XSTseg.size() >= points.size()/2);	       
	       
               showXST();

     }// end if n > 2 
     free_memory();
}

void CXSteinerTree::showXST()
{
    //cout << "edge number: " << edgeCount << endl;
    assert( (unsigned int)edgeCount == XSTseg.size() );
    //assert( XSTseg.size() < points.size());
   
    m_xst_array.clear(); 

    for(  unsigned int k = 0 ; k < XSTseg.size() ; k++ ) //Draw Segments
	if( XSTseg[k].valid == 1 )
	{
	    m_xst_array.push_back( CPoint( XSTseg[k].x1, XSTseg[k].y1 ) );
	    m_xst_array.push_back( CPoint( XSTseg[k].x2, XSTseg[k].y2 ) );
	    //cout << XSTseg[k].x1 << "," << XSTseg[k].y1 << " - " << XSTseg[k].x2 << "," << XSTseg[k].y2 << endl;
	    //cout << XSTseg[k].terminalNum1 << "-" << XSTseg[k].terminalNum2 << endl;
	}
    
    assert( m_xst_array.size() != 0 );
}
