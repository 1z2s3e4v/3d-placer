#ifndef DIAMOND_LEGALIZER_H
#define DIAMOND_LEGALIZER_H

#include <vector>
#include <map>
#include <list>
using namespace std;

class CPlaceDB;

class CDiamondLegalizer
{
    class CBlock
    {
    public:
        static bool Lesser( const CBlock& b1, const CBlock& b2 )
        {
            return b1.x < b2.x; 
        }
        static bool Greater( const CBlock& b1, const CBlock& b2 )
        {
            return b1.x > b2.x; 
        }

        CBlock( const double& x=0, const double& width=0 )
        {
            this->x = x;
            this->width = width;
        }
        double x;
        double width;
    };

    class CRow
    {
    public:
        static bool Lesser( const CRow& r1, const CRow& r2 )
        {
            return r1.y < r2.y;
        }
        static bool Greater( const CRow& r1, const CRow& r2 )
        {
            return r1.y > r2.y;
        }

        CRow( const double& y=0 )
        {
            this->y = y;
        }
        map< double, CBlock > blocks;
        double y;
    };


public:
    CDiamondLegalizer( CPlaceDB& currentPlaceDB );
    ~CDiamondLegalizer(void);
    bool Legal();

private:
    bool AddBlock( double x, double y, double width, double height );
    
    CPlaceDB* m_pDB;
    vector< CDiamondLegalizer::CRow > m_rows; 

};

#endif
