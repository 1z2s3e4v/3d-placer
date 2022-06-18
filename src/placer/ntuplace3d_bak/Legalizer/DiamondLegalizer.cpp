#include <algorithm>
#include <vector>
#include <cstdio>
using namespace std;

#include "util.h"


#include "DiamondLegalizer.h"
#include "placedb.h"
#include "DiamondSearch.h"

CDiamondLegalizer::CDiamondLegalizer( CPlaceDB& currentPlaceDB )
{
    m_pDB = &currentPlaceDB;

    // Create "Row" data structure
    vector<CSiteRow>::const_iterator ite;
    for( ite = m_pDB->m_sites.begin(); ite != m_pDB->m_sites.end(); ite++ )
    {
        m_rows.push_back( CDiamondLegalizer::CRow( ite->m_bottom ) ); 
    }
}

CDiamondLegalizer::~CDiamondLegalizer(void)
{
}

bool CDiamondLegalizer::Legal()
{
    // Generate legalizer sequnece
    multimap< double, int > sequence;
    double cost;
    for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
    {
        if( !m_pDB->m_modules[i].m_isFixed )
        {
            cost = m_pDB->m_modules[i].m_x;
            sequence.insert( pair<double,int>( cost, i ) );
        }
    }


    //multimap< double, int >::iterator ite;
    //for( ite = sequence.begin(); ite!= sequence.end(); ite++ )
    //{
    //    cout << m_pDB->m_modules[ ite->second ].m_x << endl;
    //}
    
    int step = (int)sequence.size() / 150;
    int counter = 0;

    multimap< double, int >::iterator ite;
    double width, height;
    for( ite = sequence.begin(); ite!= sequence.end(); ite++ )
    {

        counter++;
        if( counter % step == 0 )
            cout << "*";
        //if( counter > 100 )
        //    break;

        //cout << ".";

        width = m_pDB->m_modules[ ite->second ].m_width;
        height = m_pDB->m_modules[ ite->second ].m_height;
 
        //cout << "========handle block " << ite->second << "   w= " << width << "  h= " << height;
        //cout << "  x= " << m_pDB->m_modules[ ite->second ].m_x << "  y= " << m_pDB->m_modules[ ite->second ].m_y << endl;

        // FIX ME!
        //double y = int( m_pDB->m_modules[ ite->second ].m_y / m_pDB->m_rowHeight ) * m_pDB->m_rowHeight;
        //double x = m_pDB->m_modules[ ite->second ].m_x;
        double y = 0;
        double x = 0;
        m_pDB->SetModuleLocation( ite->second, x, y );

        CDiamondSearch diamondSearch( width, height, x, y );

        double dist = 0;
        vector<CPoint> points;
        bool nextBlock = false;
        while( true )
        {
            points.clear();
            diamondSearch.GetPoints( dist, points );

            if( points.size() == 0 )
                return false;   // Cannot find place to put the block.

            for( int i=0; i<(int)points.size(); i++ )
            {
                //cout << "   try x=" << points[i].x << " y= " << points[i].y << endl;
                if( AddBlock( points[i].x, points[i].y, width, height ) )
                {
                    //cout << "   put x=" << points[i].x << " y= " << points[i].y << endl;
                    //cout << "ok!\n";
                    nextBlock = true;
                    m_pDB->SetModuleLocation( ite->second, points[i].x, points[i].y );
                    break;   // Success, deal with the next block
                }
            }
           
            if( nextBlock )
                break;
            //dist += 1.0;        // site width
            dist += m_pDB->m_rowHeight;
        } // loop for finding position
    } // loop for blocks


    return true;
}

bool CDiamondLegalizer::AddBlock( double x, double y, double width, double height )
{
    // The "height" and "width" must on the site.
    double rowHeight = m_pDB->m_rowHeight;

    // STEP 1: Check if in the site
    double checkHeight = height;
    double currentY = y;
    while( checkHeight > 0 )
    {
        vector<CSiteRow>::iterator ite = m_pDB->GetRow( currentY );
        if( ite == m_pDB->m_sites.end() )
        {
            // currentY must on the rows
            return false;
        }

        ////////cout << "site_y= " << ite->m_bottom << " currentY= " << currentY << endl;
        ////////if( ite->m_bottom != currentY )
        ////////{
        ////////    cout << "m_bottom != currentY " << ite->m_bottom << " " << currentY << endl;
        ////////    exit(0);
        ////////}

        if( !ite->isInside( x, width ) )
            return false;   // not on the site

        currentY += rowHeight;
        checkHeight -= rowHeight;
    } 
    //cout << "find site_row  ";


    // STEP 2: check if overlap
    //cout << "check overlap\n";
    CDiamondLegalizer::CRow dummyRow;
    CDiamondLegalizer::CBlock dummyBlock;
    vector<CDiamondLegalizer::CRow>::iterator iteRow;
    map<double, CDiamondLegalizer::CBlock>::iterator iteBlock;

    checkHeight = height;
    currentY = y;
    while( checkHeight > 0 )
    {
        // Find the row
        dummyRow.y = currentY;

        iteRow = lower_bound( m_rows.begin(), m_rows.end(), dummyRow, CDiamondLegalizer::CRow::Lesser );
        if( iteRow->y != currentY )
            return false;
        if( iteRow == m_rows.end() )
            return false;
        //cout << "row y= " << iteRow->y << " currentY= " << currentY <<  " x= " << x << endl;

        // Find the nearest block
        dummyBlock.x = x;
        if( iteRow->blocks.size() > 0 )
        {
            // FIX ME

            //for( int i=0; i<(int)iteRow->blocks.size(); i++ )
            //{
            //    cout << iteRow->blocks[i].x << " ";
            //}
            //cout << " )\n";

            //iteBlock = lower_bound( iteRow->blocks.begin(), iteRow->blocks.end(), dummyBlock, CDiamondLegalizer::CBlock::Lesser );
            //iteBlock = lower_bound( iteRow->blocks.begin(), iteRow->blocks.end(), x );
            iteBlock = iteRow->blocks.lower_bound( x );
            if( iteBlock == iteRow->blocks.end() )
                iteBlock--;
            //if( iteBlock != iteRow->blocks.end() )
            {
                //cout << " block_range(" << iteBlock->x << " " << iteBlock->width << ") ";
                if( iteBlock->first + iteBlock->second.width > x )
                {
                    //cout << "OVERLAP\n";
                    return false; // overlap
                }
            }
            //else
            //    cout << "end ";
        }

        currentY += rowHeight;
        checkHeight -= rowHeight;
    } 

    
    // Here, we make sure the space is free. Add the block!
    //cout << "Add block\n";
    checkHeight = height;
    currentY = y;
    while( checkHeight > 0 )
    {
        // Find the row
        dummyRow.y = currentY;
        iteRow = lower_bound( m_rows.begin(), m_rows.end(), dummyRow, CDiamondLegalizer::CRow::Lesser );
        assert( iteRow != m_rows.end() );    // currentY must on the rows

        // Find the nearest block
        dummyBlock.x = x;
        //iteBlock = lower_bound( iteRow->blocks.begin(), iteRow->blocks.end(), dummyBlock, CDiamondLegalizer::CBlock::Lesser );
        //iteBlock = lower_bound( iteRow->blocks.begin(), iteRow->blocks.end(), x );
        iteBlock = iteRow->blocks.lower_bound( x );
        //iteRow->blocks.insert( iteBlock, CBlock( x, width ) );
        iteRow->blocks.insert( pair<double, CBlock>( x, CBlock( x, width ) ) );

        //cout << "(";
        //for( int i=0; i<(int)iteRow->blocks.size(); i++ )
        //{
        //    cout << iteRow->blocks[i].x << " " ;
        //}
        //cout << ")\n";

        currentY += rowHeight;
        checkHeight -= rowHeight;
    } 

    return true;
}
