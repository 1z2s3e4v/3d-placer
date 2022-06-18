#include <string>
#include "cluster.h"
using namespace std;


#if 0

// orignal usage in main.cpp
// removed by donnie
// 2007-07-16

if(true == gArg.CheckExist( "RQP" ))
{
    printf("\n======RUN RQP Global Placer!!=====\n");
    mlqp mqp( placedb, param.outFilePrefix, gCType, gWeightLevelDecreaingRate);
    bool run=true;
    while(run)
    {
	mqp.currentDBPlace();
	run=mqp.decluster();
    }
    placedb.RemoveFixedBlockSite();
    isLegal = mqp.endcasePlace();
}

#endif



bool multilevel_qp( CPlaceDB& placedb, string outFilePrefix, 
       int ctype,
//       int clusterSize, double clusterRatio, double targetNNB, double targetDensity,
       double incFactor, double wWire, //int maxLevel=INT_MAX, 
       double weightLevelDecreaingRate=2//, double targetUtilization=1.0 );
);

class mlqp
{
public:
    mlqp( CPlaceDB& placedb, string outFilePrefix, int ctype,  double wWire, double weightLevelDecreaingRate=2);
    ~mlqp(){}

    CPlaceDB* m_currentDB;
    int m_totalLevels;



    void currentDBPlace();
    void currentDBPlaceForGRoute();
    bool decluster(); //return false if reach the top level
    bool endcasePlace();
    void initPlacement();

private:
    CPlaceDB* m_pDB;
    vector<CPlaceDB>* m_p_placedb_clustered;
    //vector<CPlaceDB>& m_placedb_clustered;
    vector<CClustering>* m_p_clusters;
    //vector<CClustering>& m_clusters;

    vector<double> m_levelSmoothDelta;
    vector<double> m_levelTargetOver;

    double m_weightLevelDecreaingRate;
    //double m_incFactor;
    int m_levels;
    int m_currentLevel;
    double m_weightWire;
    double m_mlqp_start;
    string m_outFilePrefix;

};
