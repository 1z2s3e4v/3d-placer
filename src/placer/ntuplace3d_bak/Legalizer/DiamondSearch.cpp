#include <iostream>
#include <cstdio>
using namespace std;

#include "DiamondSearch.h"

CDiamondSearch::CDiamondSearch( const double& w, const double& h, const double& x, const double& y )
{
    w_step = w;
    h_step = h;
    this->x = x;
    this->y = y;
}

CDiamondSearch::~CDiamondSearch(void)
{
}

void CDiamondSearch::Show( const double& step )
{
#if 1
    cout << "Step= " << step << endl;
#endif

    double off_x;
    double off_y;
    
    off_x = step;
    off_y = 0;

    cout << off_x << " " << off_y << endl;
    cout << -off_x << " " << off_y << endl;

    while( off_x >= h_step )
    {
        off_x -= h_step;
        off_y += h_step;

        cout << off_x << " " << off_y << endl;
        cout << off_x << " " << -off_y << endl;
        if( off_x != 0 )
        {
            cout << -off_x << " " << off_y << endl;
            cout << -off_x << " " << -off_y << endl;
        }
    }
}

void CDiamondSearch::GetPoints( const double& step, vector<CPoint>& points )
{
    double off_x;
    double off_y;
    
    off_x = step;
    off_y = 0;

    points.push_back( CPoint( x+off_x, y+off_y ) );
    if( off_x != 0 )
        points.push_back( CPoint( x-off_x, y+off_y ) );

    while( off_x >= h_step )
    {
        off_x -= h_step;
        off_y += h_step;

        points.push_back( CPoint( x+off_x, x+off_y ) );
        points.push_back( CPoint( x+off_x, x-off_y ) );
        if( off_x != 0 )
        {
            points.push_back( CPoint( x-off_x, x+off_y ) );
            points.push_back( CPoint( x-off_x, x-off_y ) );
        }
    }
}
