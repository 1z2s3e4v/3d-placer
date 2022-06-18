#include "BlockMatching.h"
#include "../PlaceDB/placedb.h"
//#include "SteinerDecomposition.h"
//#include "CongMap.h"

#include <cfloat>
#include <algorithm>
using namespace std;

CBlockMatching::CBlockMatching( CPlaceDB* pDB ) :
    m_pDB( pDB ),
    m_pLapSolve( NULL )
{
    m_pLapSolve = new CLapWrapper();
}

CBlockMatching::~CBlockMatching()
{
    delete m_pLapSolve;
}


// 2007-03-26 (donnie)
// void CBlockMatching::CongestionOptimize()
// {
//     CSteinerDecomposition* pDec = NULL;
//     pDec = new CSteinerDecomposition( *m_pDB );
//     pDec->Update();

//     int congBinSize = (int)round( sqrt( m_pDB->m_modules.size() ) );
//     m_pDB->CreateCongMap( congBinSize, congBinSize );

//     printf( "   BM: CM: TotalOverflow = %.0f\n", m_pDB->m_pCongMap->GetTotalOverflow() );
//     printf( "   BM: CM: MaxOverflow = %.0f\n", m_pDB->m_pCongMap->GetMaxOverflow() );
//     //printf( "   BM: CM: MaxVCutCongestion = %.0f\n", m_pDB->m_pCongMap->GetMaxVCutCongestion() );
//     //printf( "   BM: CM: MaxHCutCongestion = %.0f\n", m_pDB->m_pCongMap->GetMaxHCutCongestion() );
//     //printf( "   BM: CM: MaxCongestion = %.0f\n", m_pDB->m_pCongMap->GetMaxVCutCongestion() );

//     int num = 200;
//     CRect region;
//     region = m_pDB->m_coreRgn;
//     SelectRandomBlock( region, num );
//     SaveBlockLocation();
//     UseCongestionWeight();
//     Matching();

//     m_pDB->CreateCongMap( congBinSize, congBinSize );
//     printf( "   BM: CM: TotalOverflow = %.0f\n", m_pDB->m_pCongMap->GetTotalOverflow() );
//     printf( "   BM: CM: MaxOverflow = %.0f\n", m_pDB->m_pCongMap->GetMaxOverflow() );

//     pDec->Restore();  
//     delete pDec;
// }

void CBlockMatching::HPWLOptimize()
{
    int num = 150;
    //int num = INT_MAX;
    int lastHPWL = (int)round(m_pDB->CalcHPWL());
    int initHPWL = lastHPWL;
    //unsigned int i=0;

    m_usedBlockList.clear();
    //while( true )
    for( int i=0; i<10; i++ )
    {
	//SelectIndependentBlock( num, i );
	//SelectFirstBlock( num, i );
	
	CRect region;
	region = m_pDB->m_coreRgn;
	SelectRandomBlock( region, num );

	if( m_blockList.size() == 0 )
	{
	    int finalHPWL = (int)m_pDB->CalcHPWL();
	    printf( "   Total %d HPWL= %d -> %d (%.2f%%)\n", 
		    m_usedBlockList.size(), initHPWL, finalHPWL, 100.0*(finalHPWL-initHPWL)/initHPWL );
	    break;
	}

	//printf( "S" ); fflush( stdout );	
	SaveBlockLocation();
	
	//printf( "U" ); fflush( stdout );	
	UseHPWLWeight();
	
	//printf( "M" ); fflush( stdout );	
	Matching();
	
	//printf( "#" ); fflush( stdout );	

#if 1
	int newHPWL = (int)m_pDB->CalcHPWL();
	if( newHPWL != lastHPWL )
	{
	    printf( "%d (%d blocks) HPWL= %d -> %d (%.2f%%)\n", 
		    i, (int)m_blockList.size(), lastHPWL, newHPWL, 100.0*(newHPWL-lastHPWL)/lastHPWL );
	    lastHPWL = newHPWL;
	}
#endif
    }

}

void CBlockMatching::SaveBlockLocation()
{
    // Save block locations    
    m_oldX.resize( m_blockList.size() );
    m_oldY.resize( m_blockList.size() );
    for( unsigned int i=0; i<m_blockList.size(); i++ )
    {
	m_oldX[i] = m_pDB->m_modules[ m_blockList[i] ].m_cx;
	m_oldY[i] = m_pDB->m_modules[ m_blockList[i] ].m_cy;
    }
} 

void CBlockMatching::SelectBlocks()
{
    unsigned int num = 100;
    if( num > m_pDB->m_modules.size() )
	num = m_pDB->m_modules.size();

    //SelectFirstBlock( num );
    SelectIndependentBlock( num );
    SaveBlockLocation();
}

void CBlockMatching::SelectRandomBlock( const CRect& region, int num )
{

    // Create the block list
    //printf( "Create the block list in the region\n" );

    vector<int> blocks;
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	// TODO: Do not add macros
	
	if( m_pDB->m_modules[i].m_isFixed )
	    continue;
	if( m_pDB->m_modules[i].m_width == 0 ) // pseudo blocks
	    continue;
	
	if( m_pDB->m_modules[i].m_cx >= region.left &&
	    m_pDB->m_modules[i].m_cx <  region.right &&
	    m_pDB->m_modules[i].m_cy >= region.bottom &&
	    m_pDB->m_modules[i].m_cy <  region.top )
	{
	   blocks.push_back( i ); 
	}
    } // for each block

    if( num > (int)blocks.size() )
	num = blocks.size();
    SelectRandomBlock( blocks, num );
}

void CBlockMatching::SelectRandomBlock( const vector<int>& blocks, const int& num )
{
    assert( num <= (int)blocks.size() );

    //printf( "Random selection\n" );

    m_blockList.clear();
    m_blockList.resize( num );
    vector<int> randomBlocks = blocks;
    for( int i=0; i<num; i++ )
    {
	int choice = rand() % randomBlocks.size();
	//printf( "%d -> %d\n", i, choice );
	m_blockList[i] = randomBlocks[choice];
	randomBlocks.erase( randomBlocks.begin() + choice );
    }
    m_pLapSolve->SetDimension( num );
}

void CBlockMatching::SelectFirstBlock( const unsigned int& num, unsigned int start )
{
    unsigned int actualBlockNum = 0;
    m_blockList.clear();
    m_blockList.resize( num );
    for( unsigned int i=start; i<m_pDB->m_modules.size(); i++ )
    {
	if( false == m_pDB->m_modules[i].m_isFixed )
	{
	    m_blockList[actualBlockNum] = i; 
	    actualBlockNum++;
	}
	if( actualBlockNum >= num )
	    break;
    }
    m_blockList.resize( actualBlockNum );
    m_pLapSolve->SetDimension( actualBlockNum );
}

void CBlockMatching::SelectIndependentBlock( const unsigned int& num, unsigned int start )
{
    set< int > conflictBlock;

    m_blockList.clear();
    for( unsigned int i=start; i<m_pDB->m_modules.size(); i++ )
    {
	// TODO: Do not add large macros

	if( m_pDB->m_modules[i].m_isFixed )
	    continue;
	if( conflictBlock.find( i ) != conflictBlock.end() )
	    continue;

	// 2007-03-25 (donnie)	
	if( m_usedBlockList.find( i ) != m_usedBlockList.end() )
	    continue;
	m_usedBlockList.insert( i );

	m_blockList.push_back( i );

	// add conflict blocks
	for( unsigned int n=0; n<m_pDB->m_modules[i].m_netsId.size(); n++ )
	{
	    int netId = m_pDB->m_modules[i].m_netsId[n];
	    for( unsigned int p=0; p<m_pDB->m_nets[netId].size(); p++ )
	    {
		int pinId = m_pDB->m_nets[netId][p];
		conflictBlock.insert( m_pDB->m_pins[pinId].moduleId );
	    }
	}
	if( m_blockList.size() >= num )
	    break;
    }
    m_pLapSolve->SetDimension( m_blockList.size() );
}

void CBlockMatching::UseHPWLWeight()
{
    // Set LAP edge weights according to the HPWL
    
    for( unsigned int i=0; i<m_blockList.size(); i++ )
    {
	int blockId = m_blockList[i];
	double xx = m_pDB->m_modules[ blockId ].m_cx;
	double yy = m_pDB->m_modules[ blockId ].m_cy;

	// for each candidate position
	for( unsigned int j=0; j<m_blockList.size(); j++ )
	{
	    double newX = m_oldX[j];
	    double newY = m_oldY[j];

	    m_pDB->MoveModuleCenter( blockId, newX, newY );
	    double edgeWeight = 0;
	    for( unsigned int n=0; n<m_pDB->m_modules[blockId].m_netsId.size(); n++ )
	    {
		double w = m_pDB->GetNetLength( m_pDB->m_modules[blockId].m_netsId[n] );
		//double w = m_pDB->GetSteinerWL( m_pDB->m_nets[ m_pDB->m_modules[blockId].m_netsId[n] ], 1, 0 );

		//printf( "(%d:%d:%.0f)", blockId, m_pDB->m_modules[blockId].m_netsId[n], w );
		edgeWeight += w;
	    }

	    //printf( "%d --> %d of %d  = %d\n", i, j, m_blockList.size(), (int)edgeWeight );
	    //printf( " %d\t", (int)edgeWeight );
	    m_pLapSolve->AssignCost( i, j, (int)round(edgeWeight) );
	    
	} // for each candidate position
	//printf( "\n" );

	m_pDB->MoveModuleCenter( blockId, xx, yy );
    } // for each block
}

// 2007-03-26
// void CBlockMatching::UseCongestionWeight()
// {
//     for( unsigned int i=0; i<m_blockList.size(); i++ )
//     {
// 	int blockId = m_blockList[i];
// 	double xx = m_pDB->m_modules[ blockId ].m_cx;
// 	double yy = m_pDB->m_modules[ blockId ].m_cy;

// 	// for each candidate position
// 	for( unsigned int j=0; j<m_blockList.size(); j++ )
// 	{
// 	    double newX = m_oldX[j];
// 	    double newY = m_oldY[j];

// 	    // Remove the old capacity
// 	    for( unsigned int n=0; n<m_pDB->m_modules[blockId].m_netsId.size(); n++ )
// 	    {
// 		CPoint p1, p2;
// 		int netId = m_pDB->m_modules[blockId].m_netsId[n];
// 		assert( m_pDB->m_nets[netId].size() == 2 );
// 		int pin1 = m_pDB->m_nets[netId][0];
// 		int pin2 = m_pDB->m_nets[netId][1];
// 		p1.x = m_pDB->m_pins[pin1].absX;
// 		p1.y = m_pDB->m_pins[pin1].absY;
// 		p2.x = m_pDB->m_pins[pin2].absX;
// 		p2.y = m_pDB->m_pins[pin2].absY;
// 		m_pDB->m_pCongMap->RemoveTwoPinNet( p1, p2 );
// 	    }

// 	    m_pDB->MoveModuleCenter( blockId, newX, newY );

// 	    // Add the new capacity
// 	    for( unsigned int n=0; n<m_pDB->m_modules[blockId].m_netsId.size(); n++ )
// 	    {
// 		CPoint p1, p2;
// 		int netId = m_pDB->m_modules[blockId].m_netsId[n];
// 		assert( m_pDB->m_nets[netId].size() == 2 );
// 		int pin1 = m_pDB->m_nets[netId][0];
// 		int pin2 = m_pDB->m_nets[netId][1];
// 		p1.x = m_pDB->m_pins[pin1].absX;
// 		p1.y = m_pDB->m_pins[pin1].absY;
// 		p2.x = m_pDB->m_pins[pin2].absX;
// 		p2.y = m_pDB->m_pins[pin2].absY;
// 		m_pDB->m_pCongMap->AddTwoPinNet( p1, p2 );
// 	    }

// 	    // Compute cost
// 	    double edgeWeight = 0;
// 	    for( unsigned int n=0; n<m_pDB->m_modules[blockId].m_netsId.size(); n++ )
// 	    {
// 		CPoint p1, p2;
// 		int netId = m_pDB->m_modules[blockId].m_netsId[n];
// 		assert( m_pDB->m_nets[netId].size() == 2 );
// 		int pin1 = m_pDB->m_nets[netId][0];
// 		int pin2 = m_pDB->m_nets[netId][1];
// 		p1.x = m_pDB->m_pins[pin1].absX;
// 		p1.y = m_pDB->m_pins[pin1].absY;
// 		p2.x = m_pDB->m_pins[pin2].absX;
// 		p2.y = m_pDB->m_pins[pin2].absY;

// 		double w = m_pDB->m_pCongMap->GetTwoPinNetMaxOverflow( p1, p2 );

// 		//printf( "(block %d:net %d:weight %.0f)", blockId, m_pDB->m_modules[blockId].m_netsId[n], w );
// 		edgeWeight = max( edgeWeight, w );
// 	    }

// 	    //printf( "%d --> %d of %d  = %d\n", i, j, m_blockList.size(), (int)edgeWeight );
// 	    //printf( " %d\t", (int)edgeWeight );
// 	    m_pLapSolve->AssignCost( i, j, (int)round(edgeWeight) );
	    
// 	} // for each candidate position
// 	//printf( "\n" );

// 	m_pDB->MoveModuleCenter( blockId, xx, yy );
//     } // for each block
// }
						            
void CBlockMatching::UseWireDensityWeight()
{
}
							            
void CBlockMatching::Matching()
{

    if( m_blockList.size() <= 1 )
	return;

    //m_pDB->SaveBlockLocation();
    //printf( "HPWL before = %.0f\n", m_pDB->CalcHPWL() );
    /* int cost =*/ m_pLapSolve->Solve();    // return "cost"
    
    //printf( "$" ); fflush( stdout );

    // Update the placement, move to "row" 
    for( unsigned int i=0; i<m_blockList.size(); i++ )
    {
	int row = m_pLapSolve->GetRowSol( i );
	m_pDB->MoveModuleCenter( m_blockList[i], m_oldX[row], m_oldY[row] );
    }
    //printf( "HPWL after = (row) %.0f\n", m_pDB->CalcHPWL() );

    //m_pDB->OutputGnuplotFigure( "matching.plt", true, true );
    //                                          move  mesg
}

