#include "util.h"
#include <vector>

class CMinimumSpanningTree
{
public:
    CMinimumSpanningTree(void){}
    ~CMinimumSpanningTree(void){}

    void Solve(const std::vector<CPoint>& points);

    std::vector<CPoint> m_mst_array;
    //Record the orignal index of each point in the input array
    std::vector<int> m_mst_index_array;
};
