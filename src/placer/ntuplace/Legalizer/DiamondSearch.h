#ifndef DIAMOND_SEARCH_H
#define DIAMOND_SEARCH_H

#include <vector>
using namespace std;

#include "util.h"

class CDiamondSearch
{
public:
    CDiamondSearch( const double& w, const double& h, const double& x, const double& y );
    ~CDiamondSearch(void);
    void Show( const double& step );
    void GetPoints( const double& step, vector<CPoint>& points );
private:
    double w_step;
    double h_step;
    double x;
    double y;
};

#endif
