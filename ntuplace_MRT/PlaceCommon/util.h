// 2004/10/30 Tung-Chieh Chen


#ifndef UTIL_H
#define UTIL_H

#include <cmath>
#include <algorithm>
#include <functional>
#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

#ifdef WIN32

#include <sys/timeb.h>

#else

#include <sys/time.h>
#include <sys/resource.h>

#endif

#define PI 3.14159265358979

inline bool isNaN( double f )
{
    char tmp[50];
    static string nanStr = "nan";
    sprintf( tmp, "%g", f );
    return ( nanStr == tmp );
}

inline double rand_bool()
{
      return double(rand()%2);
}

inline double rand_01()
{
      return double(rand()%1000)/1000;
}

// 2006-06-16
inline void get_page_fault_num( long& minflt, long& majflt )
{
    rusage time;
    getrusage( RUSAGE_SELF, &time );
    minflt = time.ru_minflt;
    majflt = time.ru_majflt;
}

inline double seconds()
{
#ifdef WIN32	// Windows
    struct __timeb64 tstruct;
    _ftime64( &tstruct );
    return (double)tstruct.time + 0.001*tstruct.millitm;
#else		// Linux
    rusage time;
    getrusage(RUSAGE_SELF,&time);
    //return (double)(1.0*time.ru_utime.tv_sec+0.000001*time.ru_utime.tv_usec);	// user time

    return (double)(1.0*time.ru_utime.tv_sec + 0.000001*time.ru_utime.tv_usec + // user time +
	            1.0*time.ru_stime.tv_sec + 0.000001*time.ru_stime.tv_usec); // system time
#endif

    // clock() loop is about 72min. (or 4320 sec)
    //return double(clock())/CLOCKS_PER_SEC;
}

inline double seconds_real()
{
    // problem when over-day occurs
    struct timeval tv;	// seconds from 00:00am
    struct timezone tz;
    gettimeofday( &tv, &tz );
    return (double)( tv.tv_sec + 0.000001*tv.tv_usec );
}

inline void error(char *msg, const char *msg2="" )
{
  printf(msg,msg2);
  cout << endl;
  throw 1;
}


inline int distance( int location, int start, int end )
{
    if( location >= start && location <= end )  // between start/end
        return 0;
    
    if( location < start )
        return start - location;

    return location - end;
}

inline double distance( double location, double start, double end )
{
    if( location >= start && location <= end )  // between start/end
        return 0.0;
    
    if( location < start )
        return start - location;

    return location - end;
}

inline double round( double v )
{
    return floor( v+0.5 );
#if 0
    double fv = floor( v );
    if( (v - fv) >= 0.5 )
    {
        return fv+1.0;
    }
    return fv;
#endif
}

inline double max( double d1, double d2 )
{
    return d1 > d2 ? d1 : d2;
}

inline double min( double d1, double d2 )
{
    return d1 < d2 ? d1 : d2;
}

inline double getOverlap( double x1, double x2, double x3, double x4 )
{
	//assert( x1 <= x2 );
	//assert( x3 <= x4 );

	//double left= -DBL_MAX , right= -DBL_MAX ;
	double left, right;
	if( x1 >= x3 && x1 < x4 )	left = x1;		// x3---x1---x4
	else if( x3 >= x1 && x3 < x2 )	left = x3;		// x1---x3---x2
	else return 0;

	if( x2 > x3 && x2 <= x4 )	right = x2;		// x3---x2---x4
	else if( x4 > x1 && x4 <= x2 )	right = x4;		// x1---x4---x2
	else return 0;

	assert( right >= left );

	return (right - left);
}

inline double getOverlapArea( double left1, double bottom1, double right1, double top1,
						  double left2, double bottom2, double right2, double top2 )
{
	assert( left1 <= right1 );
	assert( bottom1 <= top1 );
	assert( left2 <= right2 );
	assert( bottom2 <= top2 );

	double rangeH;
	rangeH = getOverlap( left1, right1, left2, right2 );
	if( rangeH == 0 )
		return 0;

	double rangeV;
	rangeV = getOverlap( bottom1, top1, bottom2, top2 );
	if( rangeV == 0 )
		return 0;

	return (rangeH * rangeV);
}


struct DoubleInt
{
    //struct sortByDoubleHighLow : public binary_function < DoubleInt, DoubleInt, bool > 
    //{
    //    bool operator() ( DoubleInt s1, DoubleInt s2  ) 
    //    { 
    //        return ( s1.d > s2.d );
    //    }
    //};
    //struct sortByDoubleLowHigh : public binary_function < DoubleInt, DoubleInt, bool > 
    //{
    //    bool operator() ( DoubleInt s1, DoubleInt s2  ) 
    //    { 
    //        return ( s1.d < s2.d );
    //    }
    //};
    static bool Lesser( const DoubleInt& s1, const DoubleInt& s2 )
    {
        return ( s1.d < s2.d );
    }
    static bool Greater( const DoubleInt& s1, const DoubleInt& s2 )
    {
        return ( s1.d > s2.d );
    }
    double d;
    int i;
};

class CRect
{
    public:
	CRect( double top=0, double bottom=0, double left=0, double right=0 )
	{
	    this->left   = left;
	    this->right  = right;
	    this->top    = top;
	    this->bottom = bottom;
	}
	void Print()
	{
	    cout << "(" << left << "," << bottom << ")-(" << right << "," << top << ")\n"; 
	}
	double left, right, top, bottom;
};

class CPoint
{

public:
	
    //friend double Distance( const CPoint& p1,  const CPoint& p2 )
    static double Distance( const CPoint& p1,  const CPoint& p2 )
    {
        return fabs( p1.x - p2.x) + fabs( p1.y - p2.y );
    }

    // 2006-08-05 (donnie)
    //friend double XDistance( const CPoint& p1,  const CPoint& p2 )
    static double XDistance( const CPoint& p1,  const CPoint& p2 )
    {
	double dis_x = fabs( p1.x - p2.x);
	double dis_y = fabs( p1.y - p2.y);
	if( dis_x > dis_y )
	    return dis_x - dis_y + dis_y * sqrt(2);
	else
	    return dis_y - dis_x + dis_x * sqrt(2);
    }

public:
    CPoint() {};
    CPoint( double x, double y )
    {
        this->x = x;
        this->y = y;
    }
    void Print()
    {
        cout << "(" << x << "," << y << ")";
    }
    double x, y;
};


void ShowSystemInfo();         // 2005-12-26
double GetPeakMemoryUsage();   // 2005-12-26
string GetHostName();          // 2006-03-23

// 2006-12-21 (donnie)
class DataHandler
{
public:
    DataHandler();
    ~DataHandler();
    void Insert( double );
    void Clear();
    void Sort();
    void OutputFile( string );
private:
    vector<double> m_data;
};


#endif
