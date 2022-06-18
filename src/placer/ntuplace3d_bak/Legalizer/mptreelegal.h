#ifndef MPTREELEGAL_H
#define MPTREELEGAL_H
#include "libmptree.h"
//#include "Partition.h"
#include "placedb.h"
/**
	@author Indark <indark@eda.ee.ntu.edu.tw>
*/
class CMPTreeLegal{
public:
	CMPTreeLegal(CPlaceDB* m_pDB );

    ~CMPTreeLegal();
    
    MPTree mpt;
    CPlaceDB* m_pDB;

//     int m_partID;
    
    void Init(int ratio, int level);
 //   void Init4Part(int start_partID, int end_partID ,vector<CPartition>&  pPartArray);
    int Update(bool setfixed);
   // int Update4Part(int start_partID, int end_partID ,vector<CPartition>&  pPartArray);

    void Optimize();
    
private:

    int m_macroNumber;    
    

};

#endif
