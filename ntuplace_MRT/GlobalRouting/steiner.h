extern "C"
{
#include "flute.h"
}

#include "util.h"
#include <vector>

class CFlute
{
    private:
	int* m_x;
	int* m_y;
    public:
	CFlute(void)
	{
	    readLUT();
	    m_x = new int[MAXD];
	    m_y = new int[MAXD];
	}
	~CFlute(void)
	{
	    delete[] m_x;
	    delete[] m_y;
	}
	void Solve( const std::vector<CPoint>& points );
	
	std::vector<CPoint> m_steiner_array;
};

extern CFlute* pgFlute;
