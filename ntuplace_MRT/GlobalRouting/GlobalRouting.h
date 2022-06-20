#ifndef _GLOBALROUTING_
#define _GLOBALROUTING_

#include <vector>

class CCongMap;
class CPlaceDB;

class CGlobalRouting
{
    private:
	CPlaceDB& m_placedbref;
	std::vector<int> m_rand_net_order;

	//Functions for FastRoute
	void PreRoute( CCongMap& congmap );	
	
    public:
	CGlobalRouting( CPlaceDB& placedbref );
	~CGlobalRouting(void){}

	//Construct the congestion map with probalistic method
	void ProbalisticMethod( CCongMap& congmap ); 

	//Construct the congestion map with FastRoute method
	void FastRoute( CCongMap& congmap );
};

#endif
