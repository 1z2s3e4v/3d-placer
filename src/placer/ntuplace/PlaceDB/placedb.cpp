#include <cmath>
#include <ctime>
#include <map>
#include <set>
#include <list>
#include <fstream>
#include <algorithm>
#include <cfloat>
using namespace std;

#include "placedb.h"
#include "../PlaceCommon/util.h"
#include "../PlaceCommon/placebin.h"
#include "../PlaceCommon/placeutil.h"
//#include "liberty.h"
//#include "XSteiner.h"
//#include "lefdef.h"

//Added by Jin 20060331
#include "../PlaceCommon/ParamPlacement.h"
//@Added by Jin 20060331

//Added by Jin 20060629
//#include "CongMap.h"

//Added by Jin 20060726
//#include "steiner.h"  // FLUTE
//@Added by Jin 20060726

//Added by Jin 20060907
//#include "MinimumSpanningTree.h"
//@Added by Jin 20060907

//Added by Jin 20060918
//#include "GlobalRouting.h"
//@Added by Jin 20060918

//indark 20070211
#include <stdlib.h>

//Added by Jin 20070306
//#include "FixedPointMethod.h"
//@Added by Jin 20070306

const double EXPECT_DENSITY = 1.00;



CSiteRow::CSiteRow(void)
: m_bottom(0)
, m_height(0)
{
}

CSiteRow::~CSiteRow(void)
{
}


/////////////////////////////////////////////////////

bool Module::isRotated()
{
    return ( (m_orient % 2) == 1 );	// E, W, FE, FW
}

/////////////////////////////////////////////////////

CPlaceDB::CPlaceDB( ) :
    m_pTimingAnalysis( NULL )
{
    m_pLib = NULL;
    m_pLEF = NULL;
    m_pCongMap = NULL;
    m_gCellWidth  = 0.0;
    m_gCellHeight = 0.0;
    m_gCellLeft   = 0.0;
    m_gCellBottom = 0.0;
    m_gCellNumberHorizontal = 0.0;
    m_gCellNumberVertical   = 0.0;

    m_pFixedPointMethod = NULL;
    m_pSteinerDecomposition = NULL;
#if 0
    //Added by Jin 20070306
    if( gArg.IsDev() && gArg.CheckExist( "congopt" ) )
	m_pFixedPointMethod = new CFixedPointMethod( *this );
    //@Added by Jin 20070306
#endif
}

CPlaceDB::~CPlaceDB(void)
{
    delete m_pCongMap;
    m_pCongMap = NULL;

    //if( m_pTimingAnalysis != NULL )
    //{
    //	delete m_pTimingAnalysis;
    //	m_pTimingAnalysis = NULL;
    //}
#if 0
    if( gArg.IsDev() && gArg.CheckExist( "congopt" ) )
    {
	delete m_pFixedPointMethod;
	m_pFixedPointMethod = NULL;
    }
#endif
}


// 2007-07-17 (donnie)
bool CPlaceDB::BlockOutCore( int i )
{
    if( m_modules[i].m_cx < m_coreRgn.left || m_modules[i].m_cx > m_coreRgn.right ||
	    m_modules[i].m_cy < m_coreRgn.bottom || m_modules[i].m_cy > m_coreRgn.top )
	return true;
    return false;
}

void CPlaceDB::Init()
{
    //---------------
    m_nModules = m_modules.size();
    m_nNets    = m_nets.size();
    m_nPins    = m_pins.size();
    //printf( "Module # = %d\n", m_nModules );
    //printf( "Net # = %d\n", m_nNets );
    //printf( "Pin # = %d\n", m_nPins );
    //---------------

    // 2005-12-28, 2006-03-02 add movableModuleArea & freeSpace
    m_totalModuleArea = 0;
    m_totalMovableModuleArea = 0;
    m_totalFreeSpace = (m_coreRgn.right - m_coreRgn.left) * (m_coreRgn.top - m_coreRgn.bottom);
    m_totalMovableModuleNumber = 0;
    m_totalMovableLargeMacroArea = 0;
    double avg_area;
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	//m_modules[i].m_isOutCore = false;
	if( m_modules[i].m_isFixed )
	{
	   if( m_modules[i].m_cx < m_coreRgn.left || m_modules[i].m_cx > m_coreRgn.right ||
	       m_modules[i].m_cy < m_coreRgn.bottom || m_modules[i].m_cy > m_coreRgn.top )
	   {
	       //m_modules[i].m_isOutCore = true;
	       continue;
	   }
	   else
	   {
		m_totalFreeSpace -= 
		    getOverlap( m_coreRgn.left, m_coreRgn.right, m_modules[i].m_x, m_modules[i].m_x + m_modules[i].m_width ) *
		    getOverlap( m_coreRgn.bottom, m_coreRgn.top, m_modules[i].m_y, m_modules[i].m_y + m_modules[i].m_height );
	   }
	}
	else
	{
	    m_totalMovableModuleNumber ++;
	    m_totalMovableModuleArea += m_modules[i].m_area;
	    if( m_modules[i].m_height > m_rowHeight * 2 )
	    {
	    	m_totalMovableLargeMacroArea += m_modules[i].m_area;
	    }
	}
	m_totalModuleArea += m_modules[i].m_area;
    }
    avg_area = m_totalModuleArea / m_modules.size();
   
    // 2005-12-27 (donnie) sort netsId in each block
    // 2007-01-02 (indark) add m_IsMacro;
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	sort( m_modules[i].m_netsId.begin(), m_modules[i].m_netsId.end() );
	// NOTE: For now we only concider area
	//if (m_modules[i].m_area > 1000 * avg_area && m_modules[i].m_isFixed == false && m_modules[i].m_isOutCore == false  ) {
	if (m_modules[i].m_area > 1000 * avg_area && m_modules[i].m_isFixed == false && BlockOutCore( i ) == false  ) {
		m_modules[i].m_isMacro = true;
	}
		
	
	
    }
   
    // check rows
    if( m_sites.size() == 0 )
    {
	printf( "Error, no row info in placement DB.\n" );
	exit(0);
    }
    
    //Added by Jin 20070306
    //fprintf( stdout, "**********************************************INIT\n" );
    //fflush( stdout );
    //m_pFixedPointMethod = new CFixedPointMethod( *this );
    //@Added by Jin 20070306
    
}

double CPlaceDB::GetFixBlockArea( const double& left, const double& bottom, 
			const double& right, const double& top )
{
    double a = 0;
    //int id;
    if( right <= left )
    {
	printf( "WARNING! right=%f left=%f\n", right, left );
    }
    assert( right > left );
    assert( top > bottom );
    //cout << "Fixed modules= " << m_partitions[partId].fixModuleList.size() << endl;
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( m_modules[i].m_isFixed )
	{
	    a += getOverlapArea( left, bottom, right, top, m_modules[i].m_x, m_modules[i].m_y,
		    m_modules[i].m_x+m_modules[i].m_width, m_modules[i].m_y+m_modules[i].m_height );
	}
    }

    return a;
    
    //double rgn_area = (top - bottom) * (right-left);
    //double b = 0;
    //b = rgn_area - a ;
    //b *= EXPECT_DENSITY;
    //return rgn_area - b;
}


void CPlaceDB::PrintModules()
{
    int size = (int)m_modules.size();
    for( int i=0; i<size; i++ )
    {
	    // modified by indark for missalianment 
	    printf( "Block %-15s at lb(%5g, %5g) missalign:%d center(%5g, %5g) w= %-5g h= %-5g pin # = %d\n", 
		m_modules[i].GetName().c_str(),
	        m_modules[i].GetX(),
	        m_modules[i].GetY(),
		(int)(m_modules[i].GetX()) % (int)(m_sites[0].m_step) ,	
		m_modules[i].m_cx,
		m_modules[i].m_cy,
		m_modules[i].GetWidth(), 
		m_modules[i].GetHeight(), 
	        m_modules[i].m_pinsId.size() );
	for( int j=0; j<(int)m_modules[i].m_pinsId.size(); j++ )
	{
	    int pinId = m_modules[i].m_pinsId[j];
	    printf( "   Pin rel(%5g, %5g) abs(%5g, %5g)  module_size(%5g,%5g)\n", 
		    m_pins[pinId].xOff, m_pins[pinId].yOff,
		    m_pins[pinId].absX, m_pins[pinId].absY,
		m_modules[i].GetWidth(),m_modules[i].GetHeight()
		  );
	    assert( fabs( m_pins[pinId].xOff ) <= m_modules[i].GetWidth() * 0.5 );
	    assert( fabs( m_pins[pinId].yOff ) <= m_modules[i].GetHeight() * 0.5 );
	    assert( m_pins[pinId].absX >= m_modules[i].GetX() );
	    assert( m_pins[pinId].absY >= m_modules[i].GetY() );
	}
    }
    printf( "Block # = %d\n", size );
}

int CPlaceDB::CreateModuleNameMap()
{

    // TODO: try to make it faster ("sorting")?
    
    int id = 0;
    int size = static_cast<int>(m_modules.size());
    for( int i=0; i<size; ++i )
    {
	// 2005-12-18
	if( false == (m_moduleMapNameId.insert( pair<string,int>( m_modules[i].GetName(), id )).second ) )
	{
	    cerr << "Error, block name duplicate: " << m_modules[i].GetName() << endl;
	    return -1;
	}
		
	/*if( m_moduleMapNameId.find( m_modules[i].GetName() ) 
		!= m_moduleMapNameId.end() )
	{
	    cerr << "Error, block name duplicate: " << m_modules[i].GetName() << endl;
	    return -1;
	}
	m_moduleMapNameId[name] = id;*/
	
	id++;
    }
    return 0;
}

int CPlaceDB::CreateNetNameMap()
{

    // TODO: try to make it faster ("sorting")?
    
    int id = 0;
    int size = static_cast<int>(m_nets.size());
    for( int i=0; i<size; ++i )
    {
	// 2005-12-18
	if( false == (m_netMapNameId.insert( pair<string,int>( m_netsName[i], id )).second ) )
	{
	    cerr << "Error, net name duplicate: " << m_netsName[i] << endl;
	    return -1;
	}
		
	/*if( m_moduleMapNameId.find( m_modules[i].GetName() ) 
		!= m_moduleMapNameId.end() )
	{
	    cerr << "Error, block name duplicate: " << m_modules[i].GetName() << endl;
	    return -1;
	}
	m_moduleMapNameId[name] = id;*/
	
	id++;
    }
    return 0;
}

void CPlaceDB::ClearModuleNameMap()
{
    m_moduleMapNameId.clear();
}

void CPlaceDB::PrintNets()
{
    int size = (int)m_nets.size();
    for( int i=0; i<size; i++ )
    {
	printf( "Net %d: ", i );
	for( int j=0; j<(int)m_nets[i].size(); j++ )
	{
	    int pinId = m_nets[i][j];
#if 1
	    if( pinId >= (int)m_pins.size() )
	    {
		printf( "pin id = %d out of bound (%d)\n", pinId, m_pins.size() );
		exit(0);
	    }
#endif
	    int moduleId = m_pins[pinId].moduleId;
#if 1
	    if( moduleId >= (int)m_modules.size() )
	    {
		printf( "macro id = %d out of bound (%d)\n", moduleId, m_modules.size() );
		exit(0);
	    }
#endif
	    printf( "%s ", m_modules[moduleId].GetName().c_str() );
	}
	printf( "\n" );
    }
    printf( "Net # = %d\n", size );
}

double CPlaceDB::GetNetLength( vector<int>& pinsId )
{
    if( pinsId.size() <= 1 )
	return 0.0;

    int pid = pinsId[0];
    double cx, cy;
    GetPinLocation( pid, cx, cy );
    double maxX = cx;
    double minX = cx;
    double minY = cy;
    double maxY = cy;
    for( int j=1; j<(int)pinsId.size(); j++ )
    {
	pid = pinsId[j];
	GetPinLocation( pid, cx, cy );
	minX = min( minX, cx );
	maxX = max( maxX, cx );
	minY = min( minY, cy );
	maxY = max( maxY, cy );
    }
    return ( (maxX-minX) + (maxY-minY) );
}


double CPlaceDB::GetNetLength( vector<int>& pinsId, double* xwire, double* ywire )
{
    if( pinsId.size() <= 1 )
    {
	if( xwire != NULL )
	    *xwire = 0;
	if( ywire != NULL )
	    *ywire = 0;
	return 0.0;
    }

    int pid = pinsId[0];
    double cx, cy;
    GetPinLocation( pid, cx, cy );
    double maxX = cx;
    double minX = cx;
    double minY = cy;
    double maxY = cy;
    for( int j=1; j<(int)pinsId.size(); j++ )
    {
	pid = pinsId[j];
	GetPinLocation( pid, cx, cy );
	minX = min( minX, cx );
	maxX = max( maxX, cx );
	minY = min( minY, cy );
	maxY = max( maxY, cy );
    }
    if( xwire != NULL )
	*xwire = maxX-minX;
    if( ywire != NULL )
	*ywire = maxY-minY;
    return ( (maxX-minX) + (maxY-minY) );
}



double CPlaceDB::GetNetLength( int netId )
{
    assert( netId < (int)m_nets.size() );
    return GetNetLength( m_nets[netId] );

#if 0
    if( m_nets[netId].size() <= 1 )
	return 0.0;

    int pid = m_nets[netId][0];
    double cx, cy;
    GetPinLocation( pid, cx, cy );
    double maxX = cx;
    double minX = cx;
    double minY = cy;
    double maxY = cy;
    for( int j=1; j<(int)m_nets[netId].size(); j++ )
    {
	pid = m_nets[netId][j];
	GetPinLocation( pid, cx, cy );
	minX = min( minX, cx );
	maxX = max( maxX, cx );
	minY = min( minY, cy );
	maxY = max( maxY, cy );
    }
    return ( (maxX-minX) + (maxY-minY) );
#endif
}

// 2006-09-27
double CPlaceDB::GetHPWL( double* xwire, double* ywire )
{
    *xwire = 0;
    *ywire = 0;
    for( unsigned int i=0; i<m_nets.size(); i++ )
    {
	double x, y;
	GetNetLength( m_nets[i], &x, &y );
	*xwire += x;
	*ywire += y;
    }
    return *xwire + *ywire;
}

double CPlaceDB::CalcHPWL()
{
    //for( int i=0; i<(int)m_modules.size(); i++ )
    //{
    //	m_modules[i].CalcCenter();
    //}

    double minX, minY, maxX, maxY;
    minX = minY = maxX = maxY = 0;
    double HPWL = 0;
    double cx, cy;
    int pid;
    bool firstPin = false;
    for( int i=0; i<(int)m_nets.size(); i++ )
    {
	firstPin = true;
	for( int j=0; j<(int)m_nets[i].size(); j++ )
	{

	    pid = m_nets[i][j];
	    if( pid >= (int)m_pins.size() )
	    {
		printf( "ERROR net[%d][%d] pid=%d pin.size()=%d\n net#=%d", i, j, pid, m_pins.size(), m_nets.size() );
	    }
	    assert( pid < (int)m_pins.size() );
	    assert( m_pins[pid].moduleId < (int)m_modules.size() );
	    cx = m_modules[ m_pins[pid].moduleId ].m_cx;	// block center
	    cy = m_modules[ m_pins[pid].moduleId ].m_cy;	// block center

	    //cout << "CalcHPWL(): BlockCenter: pid=" << pid 
	    //	<< " moduleId=" << m_pins[pid].moduleId 
	    //	<< " x=" << cx << " y=" << cy << endl;

	    if( firstPin )
	    {
		minX = maxX = cx;
		minY = maxY = cy;
		firstPin = false;
	    }
	    else
	    {
		minX = min( minX, cx );
		maxX = max( maxX, cx );
		minY = min( minY, cy );
		maxY = max( maxY, cy );
	    }
	}

	//test code
	//		if( maxX < minX )
	//		{
	//			printf("Waring: (maxX, minX): (%f, %f)", maxX, minX );
	//			flush(cout);
	//		}
	//@test code
	assert( maxX >= minX );
	assert( maxY >= minY );

	HPWL += ( (maxX - minX) + (maxY - minY) );

    }
    m_HPWL = HPWL;	// center 2 center WL

    // Pin-to-pin WL
    HPWL = 0;
    for( unsigned int i=0; i<m_nets.size(); i++ )
	HPWL += GetNetLength( m_nets[i] );
    m_HPWLp2p = HPWL;	// pin2pinWL

    return m_HPWLp2p;
}


/*
//void CPlaceDB::SetBBox( const int& w, const int& h )
//{
//	m_bboxWidth = w;
//	m_bboxHeight = h;
//	cout << "   BBox Width: " << m_bboxWidth << endl;
//	cout << "  BBox height: " << m_bboxHeight << endl;
//
//	// Create the top-level partition.
//	// The range is the entire bounding box.
//	// All modules are in the partition.
//	m_partitions.push_back( Partition( -1, m_bboxHeight, 0, 0, m_bboxWidth, V_CUT ) );
//	
//    m_partitions[0].peerPartitionId = 0;
//
//	// Add blocks into the top-level partition.
//	// Don't add terminals (pads)
//	for( int i=0; i<(int)m_modules.size(); i++ )
//		m_partitions[0].moduleList.push_back( i );
//
//}*/


void CPlaceDB::MoveBlockToLeft( double factor )
{
    double left = m_coreRgn.left;
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
        double x, y;
        if( m_modules[i].m_isFixed == false )
        {
            x = m_modules[i].m_x;
            y = m_modules[i].m_y;
            x = (x-left) * factor + left;
            SetModuleLocation( i, x, y );
        }
    }
}

void CPlaceDB::MoveBlockToBottom( double factor )
{
    double bottom = m_coreRgn.bottom;
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
        double x, y;
        if( m_modules[i].m_isFixed == false )
        {
            x = m_modules[i].m_x;
            y = m_modules[i].m_y;
            y = (y-bottom) * factor + bottom;
            SetModuleLocation( i, x, y );
        }
    }
}

void CPlaceDB::MoveBlockToCenter( double factor )
{
    double center = 0.5 * (m_coreRgn.left + m_coreRgn.right);
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
        double x, y;
        if( m_modules[i].m_isFixed == false )
        {
            x = m_modules[i].m_x;
            y = m_modules[i].m_y;
            x = (x-center) * factor + center;
            SetModuleLocation( i, x, y );
        }
    }
}

/*
//void CPlaceDB::CreateBBox( const double &ar, const double &ws )
//{
//	cout << "Create bounding box\n";
//	
//	// calc total area
//    double maxModuleLength = 0;
//	double totalArea = 0;
//	for( int i=0; i<(int)m_modules.size(); i++ )
//	{
//		totalArea += m_modules[i].GetArea();
//        maxModuleLength = max ( maxModuleLength, m_modules[i].m_width );
//        maxModuleLength = max ( maxModuleLength, m_modules[i].m_height );
//	}
//
//    if( ar > 0 )
//    {
//	    // Igor
//	    m_bboxWidth = (int) sqrt( (1.0 + ws) * (double)totalArea * ar );
//	    m_bboxHeight = (int) sqrt( (1.0 + ws) * (double)totalArea / ar );
//    	
//	    // Mine
//	    //m_bboxWidth = (int) sqrt( (double)totalArea * ar / (1.0 - ws) );
//	    //m_bboxHeight = (int) sqrt( (double)totalArea / ar / (1.0 - ws) );
//    }
//
//    if( m_bboxHeight < maxModuleLength )
//    {
//        //m_bboxHeight = maxModuleLength;// + 1;
//        //m_bboxWidth = int( (1.0 + ws) * totalArea / m_bboxHeight );
//        m_bboxWidth = maxModuleLength;// + 1;
//        m_bboxHeight = int( (1.0 + ws) * totalArea / m_bboxWidth );
//    }
//	
//	cout << "   Total area: " << totalArea << endl;
//	cout << " Aspect ratio: " << ar << endl;
//	cout << "       Max WS: " << ws << endl;
//	cout << "   BBox Width: " << m_bboxWidth << endl;
//	cout << "  BBox height: " << m_bboxHeight << endl;
//	
//	// Create the top-level partition.
//	// The range is the entire bounding box.
//	// All modules are in the partition.
//	m_partitions.push_back( Partition( -1, m_bboxHeight, 0, 0, m_bboxWidth, V_CUT ) );
//	
//    m_partitions[0].peerPartitionId = 0;
//
//    // Add blocks into the top-level partition.
//	// Don't add terminals (pads)
//	for( int i=0; i<(int)m_modules.size(); i++ )
//		m_partitions[0].moduleList.push_back( i );
//}*/
//void CPlaceDB::CalcChipWH()
//{
//	double width = 0;
//	double height = 0;
//	for( int i=0; i<(int)m_modules.size(); i++ )
//	{
//		if( !m_modules[i].rotate )
//		{
//			width = max( width, (m_modules[i].m_x+m_modules[i].m_width) );
//			height = max( height, (m_modules[i].m_y+m_modules[i].m_height) );
//		}
//		else
//		{
//			width = max( width, (m_modules[i].m_x+m_modules[i].m_height) );
//			height = max( height, (m_modules[i].m_y+m_modules[i].m_width) );
//		}
//	}
//	m_width = width;
//	m_height = height;
//}*/



//void CPlaceDB::ShowInformation()
//{
//	//CalcChipWH();
//	//m_totalModuleArea = 0;
//	//for( int i=0; i<(int)m_modules.size(); i++ )
//	//{
//	//	m_totalModuleArea += m_modules[i].m_area;
//	//}
//
//	//cout << "Width= " << m_width << endl;
//	//cout << "Height= " << m_height << endl;
//	//cout << "Dead space: " << GetDeadspace() << "%\n";
//}


//void CPlaceDB::OutputGSRC( const char* file )
//{
//	char filename[200];
//
//	// out "blocks"
//	sprintf( filename, "%s.blocks", file );
//	FILE *out;
//	out = fopen( filename, "w" );
//	fprintf( out, "UCSC blocks 1.0\n\n" );
//
//	fprintf( out, "NumSoftRectangularBlocks : 0\n" );
//	fprintf( out, "NumHardRectilinearBlocks : %d\n", m_modules.size() );
//	fprintf( out, "NumTerminals : %d\n\n", m_terminals.size() );
//	for( int i=0; i<(int)m_modules.size(); i++ )
//	{
//		fprintf( out, "%s hardrectilinear 4 (0, 0) (0, %d) (%d, %d) (%d, 0)\n", 
//			m_modules[i].m_name.c_str(), 
//			m_modules[i].m_height, 
//			m_modules[i].m_width, m_modules[i].m_height,
//			m_modules[i].m_width );
//	}
//	fprintf( out, "\n" );
//	for( int i=0; i<(int)m_terminals.size(); i++ )
//	{
//		fprintf( out, "%s terminal\n", m_terminals[i].m_name.c_str() );
//	}
//	fprintf( out, "\n\n" );
//	fclose( out );
//	cout << "Save " << filename << endl;
//
//
//	// out "nets"
//	sprintf( filename, "%s.nets", file );
//	out = fopen( filename, "w" );
//	fprintf( out, "UCLA nets 1.0\n\n" );
//
//	fprintf( out, "NumNets : %d\n", m_nNets );
//	fprintf( out, "NumPins : %d\n", m_nPins );
//	for( int i=0; i<(int)m_nets.size(); i++ )
//	{
//		fprintf( out, "NetDegree : %d\n", m_nets[i].size() );
//		for( int j=0; j<(int)m_nets[i].size(); j++ )
//		{
//			if( m_nets[i][j] < (int)m_modules.size() )
//				fprintf( out, "%s B\n", m_modules[m_nets[i][j]].m_name.c_str() );
//			else
//				fprintf( out, "%s B\n", m_terminals[m_nets[i][j]-m_modules.size()].m_name.c_str() );
//		}
//	}
//	fprintf( out, "\n\n" );
//	fclose( out );
//	cout << "Save " << filename << endl;
//
//
//	// out "pl"
//	sprintf( filename, "%s.pl", file );
//	out = fopen( filename, "w" );
//	fprintf( out, "UCLA pl 1.0\n\n" );
//
//	for( int i=0; i<(int)m_modules.size(); i++ )
//	{
//        fprintf( out, "%s\t%d\t%d : N\n", 
//			m_modules[i].m_name.c_str(), 
//			m_modules[i].m_x, m_modules[i].m_y );
//	}
//	fprintf( out, "\n" );
//	for( int i=0; i<(int)m_terminals.size(); i++ )
//	{
//        fprintf( out, "%s\t%d\t%d : N\n", 
//			m_terminals[i].m_name.c_str(), 
//			m_terminals[i].m_x, m_terminals[i].m_y );
//	}
//	fprintf( out, "\n\n" );
//	fclose( out );
//	cout << "Save " << filename << endl;
//
//}


// void CPlaceDB::OutputPL( const char* file, bool setOutOrientN, bool bMsg )
// {
//     if( bMsg == false )
// 	printf( "Output PL file: %s\n", file );

//     // out "pl"
//     FILE *out = fopen( file, "w" );
//     fprintf( out, "UCLA pl 1.0\n\n" );

//     char* orientN = "N";

//     for( int i=0; i<(int)m_modules.size(); i++ )
//     {
// 	if( m_modules[i].m_name.substr( 0, 2 ) == "__" )
// 	    continue;	// skip dummy blocks

// 	if( strcmp( orientStr( m_modules[i].m_orient ), "BOGUS" ) == 0 )
// 	{
// 	    printf( "OutputPL: BOGUS %d ", m_modules[i].m_orient );
// 	} 

// 	char* orient;
// 	if( setOutOrientN )
// 	    orient = orientN;
// 	else 
// 	    orient = orientStr( m_modules[i].m_orient );

// 	//fprintf( out, "%25s %15.5f %15.5f : %s\n", 
// 	//	m_modules[i].m_name.c_str(), 
// 	//	m_modules[i].m_x, 
// 	//	m_modules[i].m_y, 
// 	//	orient );
// 	//	
// 	//fprintf( out, "%25s %15.2f %15.2f : %s\n", 
// 	//	m_modules[i].m_name.c_str(), 
// 	//	m_modules[i].m_x, 
// 	//	m_modules[i].m_y, 
// 	//	orient );
	
// 	// 2007-03-30 (donnie) Reduce file size
// 	fprintf( out, "%s\t%.0f\t%.0f : %s", 
// 		m_modules[i].m_name.c_str(), 
// 		m_modules[i].m_x, 
// 		m_modules[i].m_y, 
// 		orient );
//   	// (kaie) 2009-06-25 Add Fixing Information
// 	if(m_modules[i].m_isFixed)
// 	{	
// 		if(m_modules[i].m_isNI)
// 			fprintf(out, " /FIXED_NI\n");
// 		else
// 			fprintf(out, " /FIXED\n");
// 	}
// 	else
// 		fprintf(out, "\n");
//     }
//     fprintf( out, "\n\n" );
//     fclose( out );
// }

void CPlaceDB::RestoreCoreRgn(void)
{
    m_coreRgn = m_coreRgn2;
}





//void CPlaceDB::GetRegionWeightedCenter(double left, double bottom, double right, double top, double& x, double& y)
//{
//    //=============================================
//    // FIX ME: Calculate weighted center by sites.
//    //=============================================
//    x = 0.5 * (left+right);
//    y = 0.5 * (top+bottom);
//}



CPoint CPlaceDB::GetRegionWeightedCenter( const double &left, const double &right,
										    const double &bottom, const double &top )
{
	//FIXME
	//row height may be discrete or unequal
	
	//return CPoint( 0.5*(left+right), 0.5*(top+bottom) ); 
	
	double site_bottom = m_sites.begin()->m_bottom;
	double unit_height = m_sites.begin()->m_height;
	
	if( (top <= site_bottom) || (bottom >= m_sites[ m_sites.size() - 1 ].m_bottom + unit_height) )
	{
		return CPoint(0, 0);
	} 
	
	int bottom_index =  static_cast<int>( floor( ( bottom - site_bottom ) / unit_height ) );
	bottom_index = static_cast<int>(max( -1, bottom_index ));
	//bottom_index = static_cast<int>(min(bottom_index, m_sites.size()-1) );
	int top_index = (int)ceil( ( top - site_bottom ) / unit_height);
	top_index = static_cast<int>(min( (int)m_sites.size() + 1, top_index ));
	//top_index = static_cast<int>(max( 0, top_index ));
	

	
	
	double totalArea = 0;
	double sumX = 0;
	double sumY = 0;
	
	for( int i = bottom_index + 1  ; i < top_index - 1 ; i++ )
	{
		vector<double> &currentInterval = m_sites[i].m_interval;
		double effective_height = m_sites[i].m_height;
		
		
		double centerY = m_sites[i].m_bottom + ( m_sites[i].m_height / 2.0 );
		
		for( int j = 0 ; j < (signed)currentInterval.size() ; j = j+2 )
		{
			double interval_left = currentInterval[j];
			double interval_right = currentInterval[j+1];
			
			if( interval_left >= right ||
					interval_right <= left )
					continue;	
			
			double effective_left = 0;
			double effective_right = 0;
			double centerX = 0;
			
			if( interval_left >= left && interval_right <= right )
			{
				effective_left = interval_left;
				effective_right = interval_right;
			}
			else if( interval_right > right && interval_left >= left )
			{
				effective_left = interval_left;
				effective_right = right;
			}
			else if( interval_left < left && interval_right <= right )
			{
				effective_left = left;
				effective_right = interval_right;
			}
			else if( interval_left < left && interval_right > right )
			{
				effective_left = left;
				effective_right = right;
			}
			else
			{
				cerr << "Warning: Undetermined Interval Status\n";
			}			
			
			centerX = ( effective_left + effective_right ) / 2.0;
			double effective_area = ( effective_right - effective_left ) * effective_height;
			
			totalArea += effective_area;
			sumX += centerX * effective_area;
			sumY += centerY * effective_area;	
		}
		
	}
	
	// add reminder top and bottom sites
	//double bottom_reminder = m_sites[bottom_index+1].m_bottom  - bottom;
    // BUG: 2005/03/16
    double bottom_reminder = m_sites[bottom_index].m_bottom + unit_height  - bottom;

	//test code
	//printf("bottom_reminder: %f\n", bottom_reminder );
	//@test code

	// top and bottom are in different site rows
	if( top_index - bottom_index > 1 )
	{
		if( bottom_reminder > 0 && bottom_index >= 0 )
		{
			//test code
			//cout << "Have Bottom Reminder\n";
			//@test code		
			
			int i = bottom_index;
			vector<double> &currentInterval = m_sites[i].m_interval;
			double effective_height = bottom_reminder;
			
			
			double centerY = m_sites[i].m_bottom + unit_height - ( bottom_reminder / 2.0 );
	
			
			for( int j = 0 ; j < (signed)currentInterval.size() ; j = j+2 )
			{
				double interval_left = currentInterval[j];
				double interval_right = currentInterval[j+1];
				
				if( interval_left >= right ||
						interval_right <= left )
						continue;	
				
				double effective_left = 0;
				double effective_right = 0;
				double centerX = 0;
				
				if( interval_left >= left && interval_right <= right )
				{
					effective_left = interval_left;
					effective_right = interval_right;
				}
				else if( interval_right > right && interval_left >= left )
				{
					effective_left = interval_left;
					effective_right = right;
				}
				else if( interval_left < left && interval_right <= right )
				{
					effective_left = left;
					effective_right = interval_right;
				}
				else if( interval_left < left && interval_right > right )
				{
					effective_left = left;
					effective_right = right;
				}
				else
				{
					cerr << "Warning: Undetermined Interval Status\n";
				}			
				
				centerX = ( effective_left + effective_right ) / 2.0;
				double effective_area = ( effective_right - effective_left ) * effective_height;
				
				totalArea += effective_area;
				
				//test code
				//printf("totalArea: %f\n", totalArea);
				//@test code
				
				sumX += centerX * effective_area;
				sumY += centerY * effective_area;	
			}
			
			
		}
		else if( bottom_reminder < 0 )
		{
			
			cerr << "Warning: Wrong bottom_reminder: " << bottom_reminder << endl;
			cerr << "bottom: " << bottom << " bottom_index: " << bottom_index 
						<< " m_sites[bottom_index].m_bottom: " << m_sites[bottom_index].m_bottom << endl;
			cerr << "unit_height: " << unit_height << " m_sites.size(): " << (int)m_sites.size() << endl;
						
		}
		
		double top_reminder = top - m_sites[top_index-1].m_bottom ;
		
		//test code
		//printf("top_reminder: %f\n", top_reminder );
		//@test code
		
		//test code
		//printf("top_reminder: %f\n", top_reminder );
		//@test code
		
		if( top_reminder > 0 && top_index - 1 < static_cast<int>(m_sites.size()) )
		{
			//test code
			//cout << "Have Top Reminder\n";
			//@test code
			int i = top_index - 1;
			vector<double> &currentInterval = m_sites[i].m_interval;
			double effective_height = top_reminder;
			
			
                                                    // 2005/03/11: BUG FIX
			double centerY = m_sites[i].m_bottom + ( top_reminder / 2.0 );
			
			for( int j = 0 ; j < (signed)currentInterval.size() ; j = j+2 )
			{
				double interval_left = currentInterval[j];
				double interval_right = currentInterval[j+1];
				
				if( interval_left >= right ||
						interval_right <= left )
						continue;	
				
				double effective_left = 0;
				double effective_right = 0;
				double centerX = 0;
				
				if( interval_left >= left && interval_right <= right )
				{
					effective_left = interval_left;
					effective_right = interval_right;
				}
				else if( interval_right > right && interval_left >= left )
				{
					effective_left = interval_left;
					effective_right = right;
				}
				else if( interval_left < left && interval_right <= right )
				{
					effective_left = left;
					effective_right = interval_right;
				}
				else if( interval_left < left && interval_right > right )
				{
					effective_left = left;
					effective_right = right;
				}
				else
				{
					cerr << "Warning: Undetermined Interval Status\n";
				}			
				
				centerX = ( effective_left + effective_right ) / 2.0;
				double effective_area = ( effective_right - effective_left ) * effective_height;
				
				totalArea += effective_area;
				
				//test code
				//printf("totalArea: %f\n", totalArea);
				//@test code
				
				sumX += centerX * effective_area;
				sumY += centerY * effective_area;	
			}		
		}
		else if( top_reminder < 0 )
		{
			cerr << "unit_height: " << unit_height << " m_sites.size(): " << (int)m_sites.size() << endl;
			cerr << "Warning: Wrong top_reminder" << top_reminder <<  "\n";
			cerr << "top: " << top << " top_index: " << top_index 
				<< " m_sites[top_index].m_bottom: " << m_sites[top_index].m_bottom << endl; 
		}
	
	}//@top and bottom are in different site rows
	
	// top and bottom are in the same site row
	else if( top_index == bottom_index + 1 )
	{
			int i = bottom_index;

			vector<double> &currentInterval = m_sites[i].m_interval;
			double effective_height = top - bottom;
			
			if( effective_height < 0 )
			{
				cerr << "Warning: Error Effective Height\n";
			}
			
			double centerY = (top+bottom)/2.0;
			
			for( int j = 0 ; j < (signed)currentInterval.size() ; j = j+2 )
			{
				double interval_left = currentInterval[j];
				double interval_right = currentInterval[j+1];
				
				if( interval_left >= right ||
						interval_right <= left )
						continue;	
				
				double effective_left = 0;
				double effective_right = 0;
				double centerX = 0;
				
				if( interval_left >= left && interval_right <= right )
				{
					effective_left = interval_left;
					effective_right = interval_right;
				}
				else if( interval_right > right && interval_left >= left )
				{
					effective_left = interval_left;
					effective_right = right;
				}
				else if( interval_left < left && interval_right <= right )
				{
					effective_left = left;
					effective_right = interval_right;
				}
				else if( interval_left < left && interval_right > right )
				{
					effective_left = left;
					effective_right = right;
				}
				else
				{
					cerr << "Warning: Undetermined Interval Status\n";
				}			
				
				centerX = ( effective_left + effective_right ) / 2.0;
				double effective_area = ( effective_right - effective_left ) * effective_height;
				
				totalArea += effective_area;
				sumX += centerX * effective_area;
				sumY += centerY * effective_area;	
			}				
	}//@top and bottom are in the same site row
	
	else
	{
		cerr << "Warning: Undetermined top and bottom status\n";
	}
	
	double centerX, centerY;
	if( totalArea > 0 )
	{
		centerX = ( sumX / totalArea );
		centerY = ( sumY / totalArea );
	}
	else
	{
		centerX = (left + right) / 2.0;
		centerY = (top + bottom) / 2.0;	
	}
		
	//test code
//	if( (int)((top-bottom)*(right-left)) != (int)totalArea )
//	{
//		printf("*******************Begin*************\n");
//		printf("given area: %f\n", (top-bottom)*(right-left) );
//		printf("(%f, %f)\ttotalArea: %f\n", centerX, centerY, totalArea );
//		printf("bottom: (%d, %f) top: (%d, %f)\n", bottom_index, m_sites[bottom_index].m_bottom, 
//																								top_index, m_sites[top_index].m_bottom );
//		
//		printf("left: %f, right: %f, bottom: %f, top: %f\n", left, right, bottom, top );
//		printf("unit_height: %f, sites size: %d\n", unit_height, m_sites.size() );
//		printf("sites bottom: %f, sites top: %f\n", m_sites[0].m_bottom, m_sites[m_sites.size()-1].m_bottom );
//		printf("*******************End*************\n");
//		flush( cout );
//	}
	//@test code
	
	return CPoint( centerX, centerY );
		
}

CPoint CPlaceDB::GetCoreCenter( void )
{
    double sumX = 0, sumY = 0;
    int nFixedPin = 0;

    for( int i = 0 ; i < static_cast<int> (m_modules.size()) ; i++ )
    {
	if( m_modules[i].m_isFixed )
	{
	    nFixedPin += (int)m_modules[i].m_pinsId.size();
	    for( int j = 0 ; j < static_cast<int> ( m_modules[i].m_pinsId.size() ) ; j++ )
	    {
		sumX = sumX + m_modules[i].m_cx + m_pins[ m_modules[i].m_pinsId[j] ].xOff;
		sumY = sumY + m_modules[i].m_cy + m_pins[ m_modules[i].m_pinsId[j] ].yOff;
	    }
	}
    }

    if( nFixedPin == 0 )    // no pads
	return CPoint( 0.5*( m_coreRgn.top + m_coreRgn.bottom ), 0.5*( m_coreRgn.left + m_coreRgn.right ) );
    else
	return CPoint( sumX / static_cast<double>(nFixedPin), sumY / static_cast<double>(nFixedPin) );

}




void CPlaceDB::SetCoreRegion()    // Set core region according to the m_sites.
{
    // Scan m_sites to create core region bounding box
    m_coreRgn.bottom = m_sites.front().m_bottom;
    m_coreRgn.top    = m_sites.back().m_bottom + m_sites.back().m_height;
    m_coreRgn.left   = m_sites.front().m_interval.front();
    m_coreRgn.right  = m_sites.front().m_interval.back();
    //printf( "right= %g\n", m_coreRgn.right );
    for( int i=1; i<(int)m_sites.size(); i++ )
    {
	m_coreRgn.left  = min( m_coreRgn.left, m_sites[i].m_interval.front() );
	m_coreRgn.right = max( m_coreRgn.right, m_sites[i].m_interval.back() );
	//printf( "right= %g\n", m_coreRgn.right );
    }
    cout << "Set core region from site info: ";
    m_coreRgn.Print();

    // Add fixed blocks to fill "non-sites"
    //int number = CreateDummyFixedBlock();
    //cout << "Add " << number << " fixed blocks\n";

    //number = CreateDummyBlock();
    //cout << "Add " << number << " dummy blocks\n";

    // 2005/03/04
    m_coreRgn2 = m_coreRgn;
    m_coreRgnShrink = m_coreRgn;
}


void CPlaceDB::CalcModuleCenter( const int& id )
    {
	m_modules[id].m_cx = m_modules[id].m_x + m_modules[id].GetWidth() * 0.5;
	m_modules[id].m_cy = m_modules[id].m_y + m_modules[id].GetHeight() * 0.5;
    }

void CPlaceDB::GetModuleCenter( const int& id, double& x, double& y )
{
    assert( id < (int)m_modules.size() );
    x = m_modules[id].m_cx;
    y = m_modules[id].m_cy;
}

void CPlaceDB::UpdatePinPosition()
{
    for( unsigned int id=0; id<m_modules.size(); id++ )
	for( unsigned int i=0; i<m_modules[id].m_pinsId.size(); i++ )
	    CalcPinLocation( m_modules[id].m_pinsId[i], m_modules[id].m_cx, m_modules[id].m_cy );
}

void CPlaceDB::SetModuleLocation( const int& id, float x, float y)
{
    assert( id < (int)m_modules.size() );
    m_modules[id].m_x = x;
    m_modules[id].m_y = y;
    m_modules[id].m_cx = x + m_modules[id].m_width * (float)0.5;
    m_modules[id].m_cy = y + m_modules[id].m_height * (float)0.5;
    //m_modules[id].rotate = rotate;
    for( int i=0; i<(int)m_modules[id].m_pinsId.size(); i++ )
    {
	CalcPinLocation( m_modules[id].m_pinsId[i], m_modules[id].m_cx, m_modules[id].m_cy );
    }
}

bool CPlaceDB::MoveModuleCenter( const int& id, float cx, float cy )
{
    if( m_modules[id].m_isFixed )
	return false;
    m_modules[id].m_x = cx-m_modules[id].m_width * (float)0.5;
    m_modules[id].m_y = cy-m_modules[id].m_height * (float)0.5;
    m_modules[id].m_cx = cx;
    m_modules[id].m_cy = cy;
    for( int i=0; i<(int)m_modules[id].m_pinsId.size(); i++ )
    {
	CalcPinLocation( m_modules[id].m_pinsId[i], cx, cy );
    }
    return true;
}

void CPlaceDB::ShowRows()
{
    printf( "Row information:\n" );
    for( int i=0; i<(int)m_sites.size(); i++ )
    {
	printf( "   ROW %d y %10.0f    x %10.0f -- %10.0f -- %10.0f \n", 
		i, m_sites[i].m_bottom, m_sites[i].m_interval[0], m_sites[i].m_interval[1],m_sites[i].m_height );
    }
}
    

void CPlaceDB::OutputMatlabFigure( const char* filename )
{

	FILE* out = fopen( filename, "w" );
	if( !out )
	{
		cerr << "Error, cannot open output file: " << filename << endl;
		return;
	}

	printf( "Output matlab figure: %s\n", filename );

    // output title
    fprintf( out, "\ntitle('%s, block= %d, net= %d, HPWL= %.0f')\n",
                    filename, (int)m_modules.size(), m_nNets, GetHPWL() );

	// output Dead space
	//CalcChipWH();
	//fprintf( out, "\n%% black background (deadspace)\n" ); 
    //fprintf( out, "rectangle( 'Position', [0,0,%d,%d], 'FaceColor', [0 0 0] )\n",
	//					m_width, m_height );

	// output Core region
	fprintf( out, "\n%% core region\n" ); 
	fprintf( out, "%%rectangle( 'Position', [%.2f, %.2f, %.2f, %.2f], 'LineStyle', ':' )\n",
		m_coreRgn.left, 
		m_coreRgn.bottom, 
		m_coreRgn.right-m_coreRgn.left, 
		m_coreRgn.top-m_coreRgn.bottom );

	//fprintf( out, "rectangle( 'Position', [%d,%d,%d,%d], 'FaceColor', [0.5 0.5 0.5] )\n",
	//					0, 0, m_bboxWidth, m_bboxHeight );
	//fprintf( out, "rectangle( 'Position', [%d,%d,%d,%d] )\n",
	//					0, 0, m_bboxWidth, m_bboxHeight );


	// output modules
	fprintf( out, "\n%% modules\n" ); 
	double w, h;
	for( int i=0; i<(int)m_modules.size(); i++ )
	{
		if( m_modules[i].m_name.substr( 0, 2 ) == "__" )
			continue;


		//if( !m_modules[i].rotate )
		//{
			w = m_modules[i].GetWidth();
			h = m_modules[i].GetHeight();
		//}
		//else
		//{
		//	w = m_modules[i].GetHeight();
		//	h = m_modules[i].GetWidth();
		//}
 
        if( (int)m_modules.size() < 100000 || m_modules[i].m_isFixed )
        {
            // draw movable blocks only when total module # < 100k
		    fprintf( out, "rectangle( 'Position', [%.2f, %.2f, %.2f, %.2f] )\n",
						    m_modules[i].GetX(), 
						    m_modules[i].GetY(),
						    w,
						    h );
		    fprintf( out, "line( [%.2f, %.2f], [%.2f, %.2f], 'Color', 'Black'  )\n",
							    m_modules[i].GetX() + w*0.75, 
							    m_modules[i].GetX() + w, 
							    m_modules[i].GetY() + h, 
							    m_modules[i].GetY() + h*0.5);

			//========================================================================
			// Output pin location -- unmark these if you want to output pin locations
			//------------------------------------------------------------------------
			//for( int j=0; j<(int)m_modules[i].m_pinsId.size(); j++ )
			//{
			//	double x, y;
			//	GetPinLocation( m_modules[i].m_pinsId[j], x, y );
			//	fprintf( out, "rectangle( 'Position', [%.2f, %.2f, %.2f, %.2f] )\n",
			//					x - 1.0, 
			//					y - 1.0, 
			//					2.0, 
			//					2.0 );
			//}
			//========================================================================


        }

		if( m_modules[i].m_isFixed )
		{
            double border = w < h ? w * 0.1 : h * 0.1;
			fprintf( out, "rectangle( 'Position', [%.2f, %.2f, %.2f, %.2f] )\n",
							m_modules[i].GetX() + border, 
							m_modules[i].GetY() + border,
							w - border*2,
							h - border*2 );
		}

	}

	////// output modules names
	//for( int i=0; i<(int)m_modules.size(); i++ )
	//{
	//	m_modules[i].CalcCenter();
	//	fprintf( out, "text( %f, %f, '%s', 'HorizontalAlignment', 'center', 'HorizontalAlignment', 'center' )\n",
	//					m_modules[i].m_cx, 
	//					m_modules[i].m_cy,
	//					m_modules[i].GetName().c_str() );

	//}
	
	// output terminals (pads)
	//fprintf( out, "\n%% pads (terminals)\n" ); 
	//for( int i=0; i<(int)m_terminals.size(); i++ )
	//{
	//	double x = m_terminals[i].m_x;
	//	int y = m_terminals[i].m_y;
	//	//if( x == 0 )	x = -1;
	//	//if( y == 0 )	y = -1;

	//	//fprintf( out, "rectangle( 'Position', [%d,%d,%d,%d], 'FaceColor', [1 1 1] )\n",
	//	//				x, 
	//	//				y,
	//	//				1,
	//	//				1 );

	//	//fprintf( out, "rectangle( 'Position', [%d,%d,%d,%d] )\n",
	//	//				x, 
	//	//				y,
	//	//				1,
	//	//				1 );

	//	//fprintf( out, "rectangle( 'Position', [%.1f,%.1f,%.1f,%.1f] )\n",
	//	//				x+0.1, 
	//	//				y+0.1,
	//	//				0.8,
	//	//				0.8 );


	//	//fprintf( out, "rectangle( 'Position', [%.1f,%.1f,%.1f,%.1f] )\n",
	//	//				x-0.1, 
	//	//				y-0.1,
	//	//				0.2,
	//	//				0.2 );

	//	fprintf( out, "rectangle( 'Position', [%.2f, %.2f, %.2f, %.2f] )\n",
	//					m_terminals[i].GetX(), 
	//					m_terminals[i].GetY(),
	//					m_terminals[i].GetWidth(),
	//					m_terminals[i].GetHeight() );
	//	fprintf( out, "rectangle( 'Position', [%.2f, %.2f, %.2f, %.2f] )\n",
	//					m_terminals[i].GetX()+0.1, 
	//					m_terminals[i].GetY()+0.1,
	//					m_terminals[i].GetWidth()-0.2,
	//					m_terminals[i].GetHeight()-0.2 );

	//	//fprintf( out, "line( [%d,%d], [%d,%d], 'Color', 'Black'  )\n",
	//	//					x, x+1, y, y+1 );
	//	//fprintf( out, "line( [%d,%d], [%d,%d], 'Color', 'Black'  )\n",
	//	//					x+1, x, y, y+1 );


	//}

    // 2004/11/15 output nets
	if( (int)m_nets.size() < 2000 )
	{
	    fprintf( out, "\n%% nets\n" ); 
	    for( int i=0; i<(int)m_nets.size(); i++ )
	    {
		double x1, x2, y1, y2;
		if( (int)m_nets[i].size() >= 2 )
		{
		    GetPinLocation( m_nets[i][0], x1, y1 );
		    for( int j=1; j<(int)m_nets[i].size(); j++ )
		    {
			GetPinLocation( m_nets[i][j], x2, y2 );
			fprintf( out, "line( [%.2f,%.2f], [%.2f,%.2f] )\n",
				x1, x2, y1, y2 );

		    }
		}
	    }
	}

// 	// output partitions
// 	if( (int)m_partitions.size() < 5000 )
// 	{
// 		fprintf( out, "\n%% partitons\n" ); 
// 		for( int i=0; i<(int)m_partitions.size(); i++ )
// 		{
// 
// 		    fprintf( out, "rectangle( 'Position', [%.2f, %.2f, %.2f, %.2f], 'LineStyle', ':' )\n",
// 						    m_partitions[i].left, 
// 						    m_partitions[i].bottom,
// 						    m_partitions[i].right-m_partitions[i].left,
// 						    m_partitions[i].top-m_partitions[i].bottom );
// 
// 
//    //         if( m_partitions[i].childPart1 == 0 && m_partitions[i].childPart0 == 0)
// 			//	continue;
// 
// 			//double x1, x2, y1, y2;
//    //         x1 = x2 = y1 = y2 = 0;
// 			//if( m_partitions[i].cutDir == V_CUT )
// 			//{
// 			//	y1 = m_partitions[i].top;
// 			//	y2 = m_partitions[i].bottom;
//    //             if( m_partitions[i].childPart0 != 0 )
//    //             {
// 			//	    x1 = x2 = m_partitions[ m_partitions[i].childPart0 ].right;
//    //             }
//    //             else if( m_partitions[i].childPart1 != 0 )
//    //             {
// 			//	    x1 = x2 = m_partitions[ m_partitions[i].childPart1 ].left;
//    //             }
// 
// 			//}
// 			//else if( m_partitions[i].cutDir == H_CUT )
// 			//{
// 			//	x1 = m_partitions[i].left;
// 			//	x2 = m_partitions[i].right;
//    //             if( m_partitions[i].childPart0 != 0 )
//    //             {
// 			//	    y1 = y2 = m_partitions[m_partitions[i].childPart0].top;
//    //             }
//    //             else if( m_partitions[i].childPart1 != 0 )
//    //             {
// 			//	    y1 = y2 = m_partitions[m_partitions[i].childPart1].bottom;
//    //             }
// 			//}
// 			//
// 			////                    x1 x2    y1 y2
// 			//fprintf( out, "line( [%.2f, %.2f], [%.2f, %.2f], 'LineStyle', ':' )\n",
// 			//					x1, x2, y1, y2 );
// 		}
// 	}


	fclose( out );

}


// void CPlaceDB::OutputGnuplotFigureWithZoom( 
// 	const char* prefix, 
// 	bool withCellMove, 
// 	bool showMsg, 
// 	bool withZoom, 
// 	bool withOrient , 
// 	bool withPin )
// {
//     if( gArg.CheckExist( "png" ) )
//     {
// 	OutputPngFigure(prefix, withCellMove, showMsg, withOrient, withPin);
// 	return;
//     }
	
//     CalcHPWL();
//     string plt_filename,net_filename,fixed_filename,module_filename,move_filename, dummy_filename, pin_filename;
//     FILE* pPlt;
//     FILE* pNet;
//     FILE* pFixed;
//     FILE* pMod;
//     FILE* pMove;
//     FILE* pDummy;   // donnie 2006-03-02
//     FILE* pPin;   // indark 2006-04-24
//     if(withZoom){
//     	plt_filename = net_filename = fixed_filename = module_filename = 
// 	    move_filename = dummy_filename = pin_filename = prefix ;
//     	plt_filename    += ".plt";
//     	net_filename    += "_net.dat";
//     	fixed_filename  += "_fixed.dat";
//     	module_filename += "_mod.dat";
//     	move_filename   += "_move.dat";
// 	dummy_filename  += "_dummy.dat";
// 	pin_filename	+= "_pin.dat";

// 	net_filename    = "dat/" + net_filename;
// 	fixed_filename  = "dat/" + fixed_filename;
// 	module_filename = "dat/" + module_filename;
// 	move_filename   = "dat/" + move_filename;
// 	dummy_filename  = "dat/" + dummy_filename;
// 	pin_filename    = "dat/" + pin_filename;
// 	system( "mkdir dat 1> /dev/null 2> /dev/null" );
	
//     	pPlt   = fopen( plt_filename.c_str(), "w" );
//     	pNet   = fopen( net_filename.c_str(), "w" );
//     	pFixed = fopen( fixed_filename.c_str(), "w" );
//     	pMod   = fopen( module_filename.c_str(), "w" );
//     	pMove  = fopen( move_filename.c_str(), "w" );
// 	pDummy = fopen( dummy_filename.c_str(), "w" );
// 	pPin   = fopen( pin_filename.c_str(), "w" );
	
//     }else{
//     	pPlt = fopen( prefix, "w" );
//     	pNet = pFixed = pMod = pMove = pDummy = pPin = pPlt;
    	
//     }

//     if( !(pPlt && pNet && pFixed && pMod && pMove && pDummy && pPin) )
//     {
// 	cerr << "Error, cannot open output file: " << prefix << endl;
// 	return;
//     }
    

//     if( showMsg )
//     	if (withZoom)
//     		//printf( "Output gnuplot figure with prefix: %s\n", prefix );
//     		printf( "Output placement: %s\n", prefix );
//     	else
// 		//printf( "Output gnuplot figure: %s\n", prefix );
// 		printf( "Output placement (single): %s\n", prefix );

//     // output title
//     fprintf( pPlt, "\nset title \" %s, block= %d, net= %d, HPWL= %.0f \" font \"Times, 22\"\n\n",
// 	    prefix, (int)m_modules.size(), m_nNets, GetHPWLp2p() );

//     fprintf( pPlt, "set size ratio 1\n" );
    
//     if(!withZoom)
//     	fprintf( pPlt, "set nokey\n\n" ); 


//     //Congestion color map
//     //Color	    Congestion  Value
//     //Blue	    +1		3
//     //Green	    0		2
//     //Light Brown   -1		8
//     //Red	    -2		1
//     //Magenta	    -3		4
//     //Light Blue    -4 or less	5
    
//     //if( withCellMove && (int)m_nets.size() < 2000 )
//     if (withZoom)
//     {
// 	fprintf( pPlt, "plot[:][:] '%s' w l lt 3, '%s' w l lt 4, '%s' w l lt 1, '%s' w l lt 7, '%s' w l lt 5, '%s' w l lt 2\n\n", 
// 		fixed_filename.c_str(), module_filename.c_str(), move_filename.c_str(),
// 		net_filename.c_str(), dummy_filename.c_str(), pin_filename.c_str() );
//     }
//     else
//     {
// 	string drawType[2];
// 	drawType[0] = "l";
// 	drawType[1] = "filledcurve";
// 	int type = 0;
// 	if( gArg.CheckExist( "cong" ) && (int)m_modules.size()< 50000 ) // filled color
// 	    type = 1;
// 	fprintf( pPlt, "plot[:][:] '-' w l lt 3, '-' w %s lt 4, '-' w l lt 1, '-' w l lt 7, '-' w l lt 5, '-' w l lt 2\n\n", 
// 		drawType[type].c_str() ); 

//     }
    
//     // output Core region
//     // 2007-03-30 (donnie) Change %.3f to %.3f to reduce the file size
//     fprintf( pFixed, "\n# core region\n" ); 
//     fprintf( pFixed, "%.3f, %.3f\n", m_coreRgn.left, m_coreRgn.bottom );
//     fprintf( pFixed, "%.3f, %.3f\n", m_coreRgn.right, m_coreRgn.bottom );
//     fprintf( pFixed, "%.3f, %.3f\n", m_coreRgn.right, m_coreRgn.top );
//     fprintf( pFixed, "%.3f, %.3f\n", m_coreRgn.left, m_coreRgn.top ); 
//     fprintf( pFixed, "%.3f, %.3f\n\n", m_coreRgn.left, m_coreRgn.bottom );
//     fprintf( pFixed, "\n# die area\n" );
//     fprintf( pFixed, "%.3f, %.3f\n", m_dieArea.left,  m_dieArea.bottom );
//     fprintf( pFixed, "%.3f, %.3f\n", m_dieArea.right, m_dieArea.bottom );
//     fprintf( pFixed, "%.3f, %.3f\n", m_dieArea.right, m_dieArea.top );
//     fprintf( pFixed, "%.3f, %.3f\n", m_dieArea.left,  m_dieArea.top );
//     fprintf( pFixed, "%.3f, %.3f\n\n", m_dieArea.left, m_dieArea.bottom );


  
//     // output movable modules
//     fprintf( pMod, "\n# blocks\n" ); 
//     fprintf( pMod, "0, 0\n\n" );

//     double x, y, w, h;
//     int orient;
//     for( int i=0; i<(int)m_modules.size(); i++ )
//     {
// 	x = m_modules[i].GetX();
// 	y = m_modules[i].GetY();
// 	w = m_modules[i].GetWidth();
// 	h = m_modules[i].GetHeight();
// 	orient = m_modules[i].GetOrient();
// 	//if( !m_modules[i].m_isFixed && !m_modules[i].m_isDummy && !m_modules[i].m_isMacro )
// 	if( !m_modules[i].m_isFixed && !m_modules[i].m_isDummy )
// 	{
// 	    if( (int)m_modules.size()< 50000 || m_modules[i].m_height > m_rowHeight )
// 	    {
// 		// draw blocks
// 		fprintf( pMod, "%.3f, %.3f\n", x, y );
// 		fprintf( pMod, "%.3f, %.3f\n", x+w, y );
// 		fprintf( pMod, "%.3f, %.3f\n", x+w, y+h );
// 		fprintf( pMod, "%.3f, %.3f\n", x, y+h ); 
// 		fprintf( pMod, "%.3f, %.3f\n\n", x, y );

// 		//draw orientation indicator
// 		if(withOrient){
// 		    switch(orient){
// 			case 0:		//N
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.8, y+h*0.5 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.8, y+h );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.8 - w*0.1, y+h - h*0.1 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.8, y+h );
// 			    fprintf( pMod, "%.3f, %.3f\n\n", x+w*0.8 + w*0.1, y+h - h*0.1 );
// 			    break;
// 			case 1:		//W
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.5, y+h*0.8 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x, y + h*0.8 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.1, y+h*0.9 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x, y + h*0.8 );
// 			    fprintf( pMod, "%.3f, %.3f\n\n", x + w*0.1, y + h*0.7 );
// 			    break;
// 			case 2:		//S
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.2, y+h*0.5 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.2, y );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.2-w*0.1, y+h*0.1 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.2, y );
// 			    fprintf( pMod, "%.3f, %.3f\n\n", x+w*0.2 + w*0.1, y+h*0.1 );
// 			    break;
// 			case 3:		//E
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.5, y+h*0.2 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w, y+h*0.2 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w - w * 0.1, y+h*0.2 + h * 0.1  );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w, y+h*0.2 );
// 			    fprintf( pMod, "%.3f, %.3f\n\n", x+w - w * 0.1, y+h*0.2 - h * 0.1 );

// 			    break;
// 			case 4:		//FN
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.2, y+h*0.5 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.2, y + h );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.2-w*0.1, y + h*0.9 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.2, y + h );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.2 + w*0.1, y + h*0.9 );
// 			    fprintf( pMod, "%.3f, %.3f\n\n", x+w*0.2-w*0.1, y + h*0.9 );
// 			    break;
// 			case 5:		//FW
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.5, y+h*0.8 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w, y + h *0.8 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w-w*0.1, y + h *0.9 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w, y + h *0.8 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w -w*0.1, y + h *0.7 );
// 			    fprintf( pMod, "%.3f, %.3f\n\n", x+w-w*0.1, y + h *0.9 );
// 			    break;
// 			case 6:		//FS
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.8, y+h*0.5 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.8, y  );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.7, y + h * 0.1 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.8, y );
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.9, y + h * 0.1);
// 			    fprintf( pMod, "%.3f, %.3f\n\n", x+w*0.7, y + h * 0.1 );
// 			    break;
// 			case 7:		//FE
// 			    fprintf( pMod, "%.3f, %.3f\n", x+w*0.5, y+h*0.2 );
// 			    fprintf( pMod, "%.3f, %.3f\n", x, y + h*0.2  );
// 			    fprintf( pMod, "%.3f, %.3f\n", x + w*0.1, y + h*0.1  );
// 			    fprintf( pMod, "%.3f, %.3f\n", x, y + h*0.2  );
// 			    fprintf( pMod, "%.3f, %.3f\n", x + w*0.1, y + h*0.3  );
// 			    fprintf( pMod, "%.3f, %.3f\n\n", x + w*0.1, y + h*0.1  );
// 			    break;

// 			default:
// 			    fprintf( pMod, "%.3f, %.3f\n", x, y );
// 			    fprintf( pMod, "%.3f, %.3f\n\n", x + w, y +h );
// 			    fprintf( pMod, "%.3f, %.3f\n", x + w, y );
// 			    fprintf( pMod, "%.3f, %.3f\n\n", x, y +h );
// 			    cerr << "Orientation Error " << endl;	

// 		    }
// 		}
// 	    }
// 	    else
// 	    {
// 		// draw line
// 		fprintf( pMod, "%.3f, %.3f\n", x, y+h/2 );
// 		fprintf( pMod, "%.3f, %.3f\n\n", x+w, y+h/2 );
// 	    }
// 	}
//     }

//     if(!withZoom)
// 	fprintf( pPlt, "\nEOF\n\n" );

//     // output fixed modules
//     fprintf( pFixed, "\n# fixed blocks\n" ); 
//     fprintf( pFixed, "0, 0\n\n" ); 

//     for( int i=0; i<(int)m_modules.size(); i++ )
//     {
// 	x = m_modules[i].GetX();
// 	y = m_modules[i].GetY();
// 	w = m_modules[i].GetWidth();
// 	h = m_modules[i].GetHeight();
// 	orient = m_modules[i].GetOrient();
// 	if( m_modules[i].m_isFixed && !m_modules[i].m_isDummy )
// 	{
// 	    fprintf( pFixed, "%.3f, %.3f\n", x, y );
// 	    fprintf( pFixed, "%.3f, %.3f\n", x+w, y );
// 	    fprintf( pFixed, "%.3f, %.3f\n", x+w, y+h );
// 	    fprintf( pFixed, "%.3f, %.3f\n", x, y+h ); 
// 	    fprintf( pFixed, "%.3f, %.3f\n\n", x, y );
// 	    //draw orientation indicator
// 	    if(withOrient){
// 		switch(orient){
// 			case 0:		//N
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.8, y+h*0.5 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.8, y+h );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.8 - w*0.1, y+h - h*0.1 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.8, y+h );
// 				fprintf( pFixed, "%.3f, %.3f\n\n", x+w*0.8 + w*0.1, y+h - h*0.1 );
// 				break;
// 			case 1:		//W
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.5, y+h*0.8 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x, y + h*0.8 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.1, y+h*0.9 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x, y + h*0.8 );
// 				fprintf( pFixed, "%.3f, %.3f\n\n", x + w*0.1, y + h*0.7 );
// 				break;
// 			case 2:		//S
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.2, y+h*0.5 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.2, y );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.2-w*0.1, y+h*0.1 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.2, y );
// 				fprintf( pFixed, "%.3f, %.3f\n\n", x+w*0.2 + w*0.1, y+h*0.1 );
// 				break;
// 			case 3:		//E
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.5, y+h*0.2 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w, y+h*0.2 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w - w * 0.1, y+h*0.2 + h * 0.1  );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w, y+h*0.2 );
// 				fprintf( pFixed, "%.3f, %.3f\n\n", x+w - w * 0.1, y+h*0.2 - h * 0.1 );

// 				break;
// 			case 4:		//FN
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.2, y+h*0.5 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.2, y + h );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.2-w*0.1, y + h*0.9 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.2, y + h );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.2 + w*0.1, y + h*0.9 );
// 				fprintf( pFixed, "%.3f, %.3f\n\n", x+w*0.2-w*0.1, y + h*0.9 );
// 				break;
// 			case 5:		//FW
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.5, y+h*0.8 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w, y + h *0.8 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w-w*0.1, y + h *0.9 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w, y + h *0.8 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w -w*0.1, y + h *0.7 );
// 				fprintf( pFixed, "%.3f, %.3f\n\n", x+w-w*0.1, y + h *0.9 );
// 				break;
// 			case 6:		//FS
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.8, y+h*0.5 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.8, y  );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.7, y + h * 0.1 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.8, y );
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.9, y + h * 0.1);
// 				fprintf( pFixed, "%.3f, %.3f\n\n", x+w*0.7, y + h * 0.1 );
// 				break;
// 			case 7:		//FE
// 				fprintf( pFixed, "%.3f, %.3f\n", x+w*0.5, y+h*0.2 );
// 				fprintf( pFixed, "%.3f, %.3f\n", x, y + h*0.2  );
// 				fprintf( pFixed, "%.3f, %.3f\n", x + w*0.1, y + h*0.1  );
// 				fprintf( pFixed, "%.3f, %.3f\n", x, y + h*0.2  );
// 				fprintf( pFixed, "%.3f, %.3f\n", x + w*0.1, y + h*0.3  );
// 				fprintf( pFixed, "%.3f, %.3f\n\n", x + w*0.1, y + h*0.1  );
// 				break;

// 			default:
// 				fprintf( pFixed, "%.3f, %.3f\n", x, y );
// 				fprintf( pFixed, "%.3f, %.3f\n\n", x + w, y +h );
// 				fprintf( pFixed, "%.3f, %.3f\n", x + w, y );
// 				fprintf( pFixed, "%.3f, %.3f\n\n", x, y +h );
// 				cerr << "Orientation Error " << endl;	

// 		}
// 	    }


// 	}
//     }
//     if(!withZoom)
//     	fprintf( pPlt, "\nEOF\n\n" );

    
//     double threshold = (m_coreRgn.top-m_coreRgn.bottom+m_coreRgn.right-m_coreRgn.left) * 0.02;
     
//     fprintf( pMove, "\n# cell move\n" ); 
//     fprintf( pMove, "0, 0\n\n" ); 
//     if( withCellMove )
//     {
// 	// Output legalization cell shifting
// 	double x1, y1, x2, y2;
// 	for( int i=0; i<(int)m_modules.size(); i++ )
// 	{
// 	    x1 = m_modules_bak[i].m_cx;
// 	    y1 = m_modules_bak[i].m_cy;
// 	    x2 = m_modules[i].m_cx;
// 	    y2 = m_modules[i].m_cy;

// 	    if( fabs( x1-x2 ) + fabs( y1-y2 ) > threshold )
// 	    //if( (int)m_modules.size() < 100000 || m_modules[i].m_isFixed )
// 	    {
// 		// draw movable blocks only when total module # < 100k
// 		fprintf( pMove, "%.3f, %.3f\n", x1, y1 );
// 		fprintf( pMove, "%.3f, %.3f\n\n", x2, y2 );
// 	    }

// 	}
//     }
//     if(!withZoom)
// 	fprintf( pPlt, "\nEOF\n\n" );

//     // 2005/03/11 output nets
//     fprintf( pNet, "\n# nets\n" );
//     fprintf( pNet, "0, 0\n\n" ); 
//     if( (int)m_nets.size() < 2000 && (int)m_nets.size() != 0 )
//     {
// 	for( int i=0; i<(int)m_nets.size(); i++ )
// 	{
// 	    double x1, x2, y1, y2;
// 	    if( (int)m_nets[i].size() >= 2 )
// 	    {
// 		GetPinLocation( m_nets[i][0], x1, y1 );
// 		for( int j=1; j<(int)m_nets[i].size(); j++ )
// 		{
// 		    GetPinLocation( m_nets[i][j], x2, y2 );
// 		    fprintf( pNet, "%.3f, %.3f\n", x1, y1 );
// 		    fprintf( pNet, "%.3f, %.3f\n\n", x2, y2 );
// 		}
// 	    }
// 	}
//     }
//     if(!withZoom){
// 	fprintf( pPlt, "\nEOF\n\n" );
//     }


//     // output dummy modules
//     fprintf( pDummy, "\n# dummy modules\n" ); 
//     fprintf( pDummy, "0, 0\n\n" ); 
//     for( int i=0; i<(int)m_modules.size(); i++ )
//     {
// 	x = m_modules[i].GetX();
// 	y = m_modules[i].GetY();
// 	w = m_modules[i].GetWidth();
// 	h = m_modules[i].GetHeight();

// 	if( m_modules[i].m_isDummy )
// 	{
// 	    if( (int)m_modules.size()< 50000 || m_modules[i].m_height > m_rowHeight )
// 	    {
// 		// draw blocks
// 		fprintf( pMod, "%.3f, %.3f\n", x, y );
// 		fprintf( pMod, "%.3f, %.3f\n", x+w, y );
// 		fprintf( pMod, "%.3f, %.3f\n", x+w, y+h );
// 		fprintf( pMod, "%.3f, %.3f\n", x, y+h ); 
// 		fprintf( pMod, "%.3f, %.3f\n\n", x, y );

// 		fprintf( pMod, "%.3f, %.3f\n", x+w*0.75, y+h );
// 		fprintf( pMod, "%.3f, %.3f\n\n", x+w,      y+h*0.5 );
// 	    }
// 	    else
// 	    {
// 		// draw line
// 		fprintf( pMod, "%.3f, %.3f\n", x, y+h/2 );
// 		fprintf( pMod, "%.3f, %.3f\n\n", x+w, y+h/2 );
// 	    }
// 	}
//     }
//     if(!withZoom){
// 	fprintf( pPlt, "\nEOF\n\n" );
//     }


//     //indark output_ pins
//     fprintf( pPin, "\n# pins\n" );
//     fprintf( pPin, "\t0, 0\n\n" );
//     double pin_x,pin_y;
//     double pin_width = m_rowHeight * 0.5;
//     if (withPin){
// 	for( int i=0; i<(int)m_modules.size(); i++ )
// 	{

// 	    if( m_modules[i].m_isFixed || m_modules[i].m_height > m_rowHeight * 2 )
// 	    {
// 		fprintf( pPin, "     # pin for block %s\n", m_modules[i].GetName().c_str() );
// 		for (int j = 0 ; j < (int)m_modules[i].m_pinsId.size(); j++)
// 		{
// 		    int pin_id =  m_modules[i].m_pinsId[j];
// 		    pin_x = this->m_pins[pin_id].absX;
// 		    pin_y = this->m_pins[pin_id].absY;
// 		    fprintf( pPin, "%.3f, %.3f\n",   pin_x - pin_width*0.5, pin_y - pin_width*0.5 );
// 		    fprintf( pPin, "%.3f, %.3f\n",   pin_x - pin_width*0.5, pin_y + pin_width*0.5 );
// 		    fprintf( pPin, "%.3f, %.3f\n",   pin_x + pin_width*0.5, pin_y + pin_width*0.5 );
// 		    fprintf( pPin, "%.3f, %.3f\n",   pin_x + pin_width*0.5, pin_y - pin_width*0.5 );
// 		    fprintf( pPin, "%.3f, %.3f\n\n", pin_x - pin_width*0.5, pin_y - pin_width*0.5 );
// 		}
// 	    }
// 	}
//     }
//     if(!withZoom){
// 	fprintf( pPin, "\nEOF\n\n" );
//     }

//     fprintf( pPlt, "pause -1 'Press any key'" );
//     if(withZoom){
// 	fclose( pPlt );
// 	fclose( pMod );
// 	fclose( pFixed );
// 	fclose( pMove );
// 	fclose( pNet );
// 	fclose( pDummy );
// 	fclose( pPin );
//     }else{
// 	fclose( pPlt );
//     }

// }


// // nicky 2007-01-22
// void CPlaceDB::OutputPngFigure( const char* prefix, bool withCellMove, bool showMsg, bool withOrient , bool withPin )
// {
//     // coloring modes setting
//     // set blocks' color              set coloring mode
//     // Color Map:                     Mode:
//     // --------------------           ----------------------
//     // Color     Value                Mode        Value
//     // --------------------           ----------------------
//     // RED        1                   FILLED       1
//     // GREEN      2                   BORDER       2
//     // BLUE       3
//     // BLACK      4
//     // WHITE      5
//     // COFFEA     6
//     // PURPLE     7
//     // SKYBLUE    8
//     /////////////////////////////////////////////////////////////////////
//     int cell_color = 3, cell_mode = 2;
//     int macro_color = 7, macro_mode = 2;
//     int orient_color = 4;
//     int pin_color = 8;
//     int move_color = 1; 
//     int image_width = 1024;
//     int image_height = 1024;
//     /////////////////////////////////////////////////////////////////////

//     CalcHPWL();
//     string image_filename, orient_filename, pin_filename, move_filename;

//     FILE* pImage;
//     FILE* pOrient;
//     FILE* pPin;
//     FILE* pMove;

//     image_filename = orient_filename = pin_filename = move_filename =  prefix;
//     image_filename  += ".im";
//     orient_filename += ".ori";
//     pin_filename    += ".pin";
//     move_filename   += ".mov";

//     pImage  = fopen( image_filename.c_str(), "w" );  
//     pOrient = fopen( orient_filename.c_str(), "w" );
//     pPin    = fopen( pin_filename.c_str(), "w" );
//     pMove   = fopen( move_filename.c_str(), "w" );

//     if( !(pImage && pOrient && pPin && pMove) )
//     {
// 	cerr << "Error, cannot open output file: " << prefix << endl;
// 	return;
//     }

//     if( showMsg )
// 	printf( "Output placement in PNG format: %s\n", prefix);
    
//     // output image size
//     gArg.GetInt( "pngsize", &image_width );
//     image_height = image_width;
//     fprintf( pImage, "%d %d\n", image_width, image_height ); 

//     // output title
//     fprintf( pImage, "title: %s, block= %d, net= %d, HPWL= %.0f \n", 
// 	    prefix, (int)m_modules.size(), m_nNets, GetHPWLp2p());

//     // find max shift value
//     double curMin = m_coreRgn.top;
//     if(m_coreRgn.left <= m_coreRgn.top)
// 	curMin = m_coreRgn.left;
//     if(m_dieArea.left <= curMin)
// 	curMin = m_dieArea.left;
//     if(m_dieArea.top <= curMin)
// 	curMin = m_dieArea.top;
//     if(curMin < 0)
// 	fprintf( pImage, "%lf\n", (-1)*curMin);
//     else
// 	fprintf( pImage, "0\n");

//     // find scaling factor
//     double curMax = m_coreRgn.right;
//     if(m_coreRgn.top >= curMax)
// 	curMax = m_coreRgn.top;
//     if(m_dieArea.right >= curMax)
// 	curMax = m_dieArea.right;
//     if(m_dieArea.top >= curMax)
// 	curMax = m_dieArea.top;
//     fprintf( pImage, "%lf\n", curMax);

//     if( withOrient )
// 	fprintf( pImage, "withOrient %d\n", orient_color );
//     else
// 	fprintf( pImage, "withoutOrient %d\n", orient_color );

//     fprintf( pImage, "%s\n", orient_filename.c_str() );

//     if( withPin )
// 	fprintf( pImage, "withPin %d\n", pin_color );
//     else
// 	fprintf( pImage, "withoutPin %d\n", pin_color );

//     fprintf( pImage, "%s\n", pin_filename.c_str() );

//     if( withCellMove )
// 	fprintf( pImage, "withCellMove %d\n", move_color );
//     else
// 	fprintf( pImage, "withoutCellMove %d\n", move_color );

//     fprintf( pImage, "%s\n", move_filename.c_str() );

//     // output Core region
//     fprintf( pImage, "%.3f %.3f\n", m_coreRgn.right, m_coreRgn.bottom );
//     fprintf( pImage, "%.3f %.3f\n", m_coreRgn.left, m_coreRgn.top ); 

//     // output die area
//     fprintf( pImage, "%.3f %.3f\n", m_dieArea.right, m_dieArea.bottom );
//     fprintf( pImage, "%.3f %.3f\n", m_dieArea.left,  m_dieArea.top );


//     // output movable modules
//     double x, y, w, h;
//     int orient;
//     for( int i=0; i<(int)m_modules.size(); i++ )
//     {
// 	x = m_modules[i].GetX();
// 	y = m_modules[i].GetY();
// 	w = m_modules[i].GetWidth();
// 	h = m_modules[i].GetHeight();
// 	orient = m_modules[i].GetOrient();
// 	if( !m_modules[i].m_isFixed && !m_modules[i].m_isDummy && !m_modules[i].m_isMacro)
// 	{
// 	    //if( (int)m_modules.size()< 50000 || m_modules[i].m_height > m_rowHeight )
// 	    {
// 		// draw blocks
// 		fprintf( pImage, "%.3f %.3f\n", x+w, y );
// 		fprintf( pImage, "%.3f %.3f\n", x, y+h ); 

// 		// cell color and mode
// 		fprintf( pImage, "%d %d\n", cell_color, cell_mode );

// 		//draw orientation indicator
// 		if(withOrient){
// 		    switch(orient){
// 			case 0:		//N
// 			    fprintf( pOrient, "5\n");
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.8, y+h*0.5 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.8, y+h );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.8 - w*0.1, y+h - h*0.1 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.8, y+h );
// 			    fprintf( pOrient, "%.3f %.3f\n\n", x+w*0.8 + w*0.1, y+h - h*0.1 );
// 			    break;
// 			case 1:		//W
// 			    fprintf( pOrient, "5\n");
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.5, y+h*0.8 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x, y + h*0.8 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.1, y+h*0.9 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x, y + h*0.8 );
// 			    fprintf( pOrient, "%.3f %.3f\n\n", x + w*0.1, y + h*0.7 );
// 			    break;
// 			case 2:		//S
// 			    fprintf( pOrient, "5\n");
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.2, y+h*0.5 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.2, y );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.2-w*0.1, y+h*0.1 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.2, y );
// 			    fprintf( pOrient, "%.3f %.3f\n\n", x+w*0.2 + w*0.1, y+h*0.1 );
// 			    break;
// 			case 3:		//E
// 			    fprintf( pOrient, "5\n");
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.5, y+h*0.2 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w, y+h*0.2 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w - w * 0.1, y+h*0.2 + h * 0.1  );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w, y+h*0.2 );
// 			    fprintf( pOrient, "%.3f %.3f\n\n", x+w - w * 0.1, y+h*0.2 - h * 0.1 );

// 			    break;
// 			case 4:		//FN
// 			    fprintf( pOrient, "6\n");
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.2, y+h*0.5 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.2, y + h );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.2-w*0.1, y + h*0.9 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.2, y + h );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.2 + w*0.1, y + h*0.9 );
// 			    fprintf( pOrient, "%.3f %.3f\n\n", x+w*0.2-w*0.1, y + h*0.9 );
// 			    break;
// 			case 5:		//FW
// 			    fprintf( pOrient, "6\n");
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.5, y+h*0.8 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w, y + h *0.8 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w-w*0.1, y + h *0.9 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w, y + h *0.8 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w -w*0.1, y + h *0.7 );
// 			    fprintf( pOrient, "%.3f %.3f\n\n", x+w-w*0.1, y + h *0.9 );
// 			    break;
// 			case 6:		//FS
// 			    fprintf( pOrient, "6\n");
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.8, y+h*0.5 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.8, y  );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.7, y + h * 0.1 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.8, y );
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.9, y + h * 0.1);
// 			    fprintf( pOrient, "%.3f %.3f\n\n", x+w*0.7, y + h * 0.1 );
// 			    break;
// 			case 7:		//FE
// 			    fprintf( pOrient, "6\n");
// 			    fprintf( pOrient, "%.3f %.3f\n", x+w*0.5, y+h*0.2 );
// 			    fprintf( pOrient, "%.3f %.3f\n", x, y + h*0.2  );
// 			    fprintf( pOrient, "%.3f %.3f\n", x + w*0.1, y + h*0.1  );
// 			    fprintf( pOrient, "%.3f %.3f\n", x, y + h*0.2  );
// 			    fprintf( pOrient, "%.3f %.3f\n", x + w*0.1, y + h*0.3  );
// 			    fprintf( pOrient, "%.3f %.3f\n\n", x + w*0.1, y + h*0.1  );
// 			    break;

// 			default:
// 			    fprintf( pOrient, "4\n");
// 			    fprintf( pOrient, "%.3f %.3f\n", x, y );
// 			    fprintf( pOrient, "%.3f %.3f\n\n", x + w, y +h );
// 			    fprintf( pOrient, "%.3f %.3f\n", x + w, y );
// 			    fprintf( pOrient, "%.3f %.3f\n\n", x, y +h );
// 			    cerr << "Orientation Error " << endl;	

// 		    }
// 		}

// 	    }/*
// 		else
// 		{
// 	    // draw line
// 	    fprintf( pImage, "%.3f %.3f\n", x, y+h/2 );
// 	    fprintf( pImage, "%.3f %.3f\n\n", x+w, y+h/2 );
// 	    }*/
// 	}
//     }


//     // output fixed modules
//     for( int i=0; i<(int)m_modules.size(); i++ )
//     {
// 	x = m_modules[i].GetX();
// 	y = m_modules[i].GetY();
// 	w = m_modules[i].GetWidth();
// 	h = m_modules[i].GetHeight();
// 	orient = m_modules[i].GetOrient();
// 	//if (m_modules[i].m_isMacro) 
// 	if( m_modules[i].m_isFixed && !m_modules[i].m_isDummy  ||m_modules[i].m_isMacro )
// 	{
// 	    fprintf( pImage, "%.3f %.3f\n", x+w, y );
// 	    fprintf( pImage, "%.3f %.3f\n", x, y+h ); 

// 	    // output color and mode
// 	    fprintf( pImage, "%d %d\n", macro_color, macro_mode);  // first set color; second set mode

// 	    // draw orientation indicator
// 	    if(withOrient){
// 		switch(orient){
// 		    case 0:		//N
// 			fprintf( pOrient, "5\n");
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.8, y+h*0.5 );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.8, y+h );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.8 - w*0.1, y+h - h*0.1 );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.8, y+h );
// 			fprintf( pOrient, "%.3f %.3f\n\n", x+w*0.8 + w*0.1, y+h - h*0.1 );
// 			break;
// 		    case 1:		//W
// 			fprintf( pOrient, "5\n");
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.5, y+h*0.8 );
// 			fprintf( pOrient, "%.3f %.3f\n", x, y + h*0.8 );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.1, y+h*0.9 );
// 			fprintf( pOrient, "%.3f %.3f\n", x, y + h*0.8 );
// 			fprintf( pOrient, "%.3f %.3f\n\n", x + w*0.1, y + h*0.7 );
// 			break;
// 		    case 2:		//S
// 			fprintf( pOrient, "5\n");
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.2, y+h*0.5 );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.2, y );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.2-w*0.1, y+h*0.1 );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.2, y );
// 			fprintf( pOrient, "%.3f %.3f\n\n", x+w*0.2 + w*0.1, y+h*0.1 );
// 			break;
// 		    case 3:		//E
// 			fprintf( pOrient, "5\n");
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.5, y+h*0.2 );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w, y+h*0.2 );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w - w * 0.1, y+h*0.2 + h * 0.1  );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w, y+h*0.2 );
// 			fprintf( pOrient, "%.3f %.3f\n\n", x+w - w * 0.1, y+h*0.2 - h * 0.1 );

// 			break;
// 		    case 4:		//FN
// 			fprintf( pOrient, "6\n");
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.2, y+h*0.5 );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.2, y + h );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.2-w*0.1, y + h*0.9 );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.2, y + h );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.2 + w*0.1, y + h*0.9 );
// 			fprintf( pOrient, "%.3f %.3f\n\n", x+w*0.2-w*0.1, y + h*0.9 );
// 			break;
// 		    case 5:		//FW
// 			fprintf( pOrient, "6\n");
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.5, y+h*0.8 );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w, y + h *0.8 );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w-w*0.1, y + h *0.9 );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w, y + h *0.8 );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w -w*0.1, y + h *0.7 );
// 			fprintf( pOrient, "%.3f %.3f\n\n", x+w-w*0.1, y + h *0.9 );
// 			break;
// 		    case 6:		//FS
// 			fprintf( pOrient, "6\n");
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.8, y+h*0.5 );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.8, y  );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.7, y + h * 0.1 );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.8, y );
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.9, y + h * 0.1);
// 			fprintf( pOrient, "%.3f %.3f\n\n", x+w*0.7, y + h * 0.1 );
// 			break;
// 		    case 7:		//FE
// 			fprintf( pOrient, "6\n");
// 			fprintf( pOrient, "%.3f %.3f\n", x+w*0.5, y+h*0.2 );
// 			fprintf( pOrient, "%.3f %.3f\n", x, y + h*0.2  );
// 			fprintf( pOrient, "%.3f %.3f\n", x + w*0.1, y + h*0.1  );
// 			fprintf( pOrient, "%.3f %.3f\n", x, y + h*0.2  );
// 			fprintf( pOrient, "%.3f %.3f\n", x + w*0.1, y + h*0.3  );
// 			fprintf( pOrient, "%.3f %.3f\n\n", x + w*0.1, y + h*0.1  );
// 			break;

// 		    default:
// 			fprintf( pOrient, "4\n");
// 			fprintf( pOrient, "%.3f %.3f\n", x, y );
// 			fprintf( pOrient, "%.3f %.3f\n\n", x + w, y +h );
// 			fprintf( pOrient, "%.3f %.3f\n", x + w, y );
// 			fprintf( pOrient, "%.3f %.3f\n\n", x, y +h );
// 			cerr << "Orientation Error " << endl;	

// 		}
// 	    }


// 	}
//     }


//     // output cell move
//     if( withCellMove )
//     {
// 	// Output legalization cell shifting
// 	double x1, y1, x2, y2;
// 	for( int i=0; i<(int)m_modules.size(); i++ )
// 	{
// 	    x1 = m_modules_bak[i].m_cx;
// 	    y1 = m_modules_bak[i].m_cy;
// 	    x2 = m_modules[i].m_cx;
// 	    y2 = m_modules[i].m_cy;

// 	    if( (int)m_modules.size() < 100000 || m_modules[i].m_isFixed )
// 	    {
// 		// draw movable blocks only when total module # < 100k
// 		fprintf( pMove, "%.3f %.3f\n", x1, y1 );
// 		fprintf( pMove, "%.3f %.3f\n\n", x2, y2 );
// 	    }

// 	}
//     }

//     /*
//        if(!withZoom)
//        fprintf( pPlt, "\nEOF\n\n" );


//     // 2005/03/11 output nets
//     fprintf( pNet, "\n# nets\n" );
//     fprintf( pNet, "0, 0\n\n" ); 
//     if( (int)m_nets.size() < 2000 && (int)m_nets.size() != 0 )
//     {
//     for( int i=0; i<(int)m_nets.size(); i++ )
//     {
//     double x1, x2, y1, y2;
//     if( (int)m_nets[i].size() >= 2 )
//     {
//     GetPinLocation( m_nets[i][0], x1, y1 );
//     for( int j=1; j<(int)m_nets[i].size(); j++ )
//     {
//     GetPinLocation( m_nets[i][j], x2, y2 );
//     fprintf( pNet, "%.3f, %.3f\n", x1, y1 );
//     fprintf( pNet, "%.3f, %.3f\n\n", x2, y2 );
//     }
//     }
//     }
//     }
//     if(!withZoom){
//     fprintf( pPlt, "\nEOF\n\n" );
//     }



//     // output dummy modules
//     for( int i=0; i<(int)m_modules.size(); i++ )
//     {
//     x = m_modules[i].GetX();
//     y = m_modules[i].GetY();
//     w = m_modules[i].GetWidth();
//     h = m_modules[i].GetHeight();

//     if( m_modules[i].m_isDummy )
//     {
//     //if( (int)m_modules.size()< 50000 || m_modules[i].m_height > m_rowHeight )
//     {
//     // draw blocks
//     fprintf( pImage, "%.3f %.3f\n", x+w, y );
//     fprintf( pImage, "%.3f %.3f\n", x, y+h ); 

//     }
//     else
//     {
//     // draw line
//     fprintf( pImage, "%.3f %.3f\n", x, y+h/2 );
//     fprintf( pImage, "%.3f %.3f\n\n", x+w, y+h/2 );
//     }
//     }
//     }*/

//     // output pins
//     double pin_x,pin_y;
//     double pin_width = m_rowHeight * 0.5;
//     if (withPin){

// 	for( int i=0; i<(int)m_modules.size(); i++ )
// 	{
// 	    if( m_modules[i].m_isFixed || m_modules[i].m_height > m_rowHeight * 2 )
// 	    {
// 		//fprintf( pPin, "     # pin for block %s\n", m_modules[i].GetName().c_str() );
// 		for (int j = 0 ; j < (int)m_modules[i].m_pinsId.size(); j++)
// 		{
// 		    int pin_id =  m_modules[i].m_pinsId[j];
// 		    pin_x = this->m_pins[pin_id].absX;
// 		    pin_y = this->m_pins[pin_id].absY;
// 		    fprintf( pPin, "%.3f %.3f\n",   pin_x - pin_width*0.5, pin_y - pin_width*0.5 );
// 		    fprintf( pPin, "%.3f %.3f\n",   pin_x - pin_width*0.5, pin_y + pin_width*0.5 );
// 		    fprintf( pPin, "%.3f %.3f\n",   pin_x + pin_width*0.5, pin_y + pin_width*0.5 );
// 		    fprintf( pPin, "%.3f %.3f\n",   pin_x + pin_width*0.5, pin_y - pin_width*0.5 );
// 		    fprintf( pPin, "%.3f %.3f\n",   pin_x - pin_width*0.5, pin_y - pin_width*0.5 );
// 		}
// 	    }
// 	}
//     }


//     fclose( pImage );
//     fclose( pPin );
//     fclose( pMove );
//     fclose( pOrient );

//     //indark
//     //automatic !
//     string cmd = __PLOTTER_BIN__ ;
//     cmd += "  " ;
//     cmd += image_filename;
//     cmd += " 1> /dev/null 2> /dev/null";
//     //	cout << cmd << endl;
//     system(cmd.c_str());
//     cmd = "rm ";
//     cmd += image_filename;
//     cmd += "  ";
//     cmd += pin_filename;
//     cmd += "  ";
//     cmd += move_filename;
//     cmd += "  ";
//     cmd += orient_filename;
//     cmd += " 1> /dev/null 2> /dev/null";
//     //	cout << cmd << endl;
//     if (!gArg.CheckExist("NoRemovePngFiles"))
// 	system(cmd.c_str());
    
// }




/*!
  \fn CPlaceDB::RemoveFixedBlockSite
  */
void CPlaceDB::RemoveFixedBlockSite()
{
    if( param.bShow ) // 2006-04-26
	printf( "Remove sites under fixed blocks\n" );

    /// @todo implement me
    double row_top =  m_sites.back().m_bottom +  m_sites.back().m_height;
    double row_bottom =  m_sites.front().m_bottom;

    for( vector<Module>::iterator iteModule =  m_modules.begin() ;
	    iteModule <  m_modules.end() ;
	    iteModule++ )
    {
	double module_top = iteModule->m_y + iteModule->m_height;
	double module_bottom = iteModule->m_y;
	double module_left = iteModule->m_x;
	double module_right = iteModule->m_x + iteModule->m_width;

	//Added by Jin 20070311
	//Skip 0 width modules
	//if( module_left == module_right )
	if( iteModule->m_width == 0 )
	{
	    continue;
	}   
	
	if( iteModule->m_isFixed && // Find a fixed module
		!iteModule->m_isNI && // skip terminal_NI
		module_bottom < row_top && // Confirm that the module is overlapped with the rows in y coordinates
		module_top > row_bottom ) 
	{
	    vector<CSiteRow>::iterator iteBeginRow, iteEndRow;

	    // find the begin row
	    for( iteBeginRow =  m_sites.begin() ; 
		    iteBeginRow <  m_sites.end() ; 
		    iteBeginRow++ )
	    {
		if( iteBeginRow->m_bottom + iteBeginRow->m_height > module_bottom )
		    break;
	    }

	    for( iteEndRow = iteBeginRow ;
		    iteEndRow <  m_sites.end() ;
		    iteEndRow++ )
	    {
		if( iteEndRow->m_bottom + iteEndRow->m_height >= module_top )
		    break;
	    }

	    if( iteEndRow == m_sites.end() )
		iteEndRow--;


	    assert( iteBeginRow !=  m_sites.end() );

	    for( vector<CSiteRow>::iterator iteRow = iteBeginRow ;
		    iteRow <= iteEndRow ;
		    iteRow++ )
	    {
		double interval[2];
		for( int i = 0 ; i < (signed)iteRow->m_interval.size() ; i++ )
		{
		    interval[ i % 2 ] = iteRow->m_interval[i];
		    if( ( i % 2 ) == 1 ) // Get two terminals of the interval
		    {
			if( interval[0] >= module_right || interval[1] <= module_left )  // screen unnecessary checks
			    continue;

			if( interval[0] >= module_left && interval[1] <= module_right )
			{
			    //    ---
			    // MMMMMMMMM
			    iteRow->m_interval.erase( vector<double>::iterator(&(iteRow->m_interval[i])) );
			    iteRow->m_interval.erase( vector<double>::iterator(&(iteRow->m_interval[i-1])) );
			    i = i - 2;
			}
			else if( interval[1] > module_right && interval[0] >= module_left )
			{
			    // ------      -----
			    // MMM      MMMMM
			    iteRow->m_interval[i-1] = module_right;
			}
			else if( interval[0] < module_left && interval[1] <= module_right )
			{
			    // ---------       -----
			    //     MMMMM          MMMMM
			    iteRow->m_interval[i] = module_left;
			}
			else if( interval[0] < module_left && interval[1] > module_right )
			{
			    // -----------
			    //    MMMM
			    iteRow->m_interval[i] = module_left;
			    iteRow->m_interval.insert( vector<double>::iterator(&(iteRow->m_interval[i+1])), interval[1] );
			    iteRow->m_interval.insert( vector<double>::iterator(&(iteRow->m_interval[i+1])), module_right );
			    i = i + 2;
			}
			else
			{
			    printf("Warning: Module Romoving Error\n");
			    //exit(-1);
			}

		    }
		}
	    }

	    //@remove the occupied sites

	}
    }

    //Align each interval to site step
    FixFreeSiteBySiteStep();

#if 0 
    //test code
    ofstream sitefile( "sites.log" );
    for( vector<CSiteRow>::iterator iteRow = m_sites.begin() ; 
	    iteRow != m_sites.end() ; iteRow++ )
    {
	sitefile << "Row bottom: " << iteRow->m_bottom << " ";

	for( unsigned int iInterval = 0 ; 
		iInterval != iteRow->m_interval.size() ; iInterval=iInterval+2 )
	{
	    sitefile << "(" << iteRow->m_interval[iInterval] 
		<< "," << iteRow->m_interval[iInterval+1]  << ") ";
	}
	sitefile << endl;
    }

    sitefile.close();
    cout << "Dump site.log" << endl;
    //@test code
#endif


}

void CPlaceDB::AdjustCoordinate( )		//by indark
{

    double min_x,min_y;
    min_x = 0.0;
    min_y = 0.0;

    for (int i = 0 ; i < (int)m_sites.size() ; i++ ){
	if (min_y  > m_sites[i].m_bottom )
	    min_y = m_sites[i].m_bottom;
	for(int j = 0 ; j < (int)m_sites[i].m_interval.size() ; j++ ){
	    if (min_x > m_sites[i].m_interval[j] )
		min_x = m_sites[i].m_interval[j];
	}

    }


    min_x = -min_x;
    min_y = -min_y;

    cout << "Adjust X:" << min_x << "Y:" << min_y << endl;

    for (int i = 0 ; i < (int)m_sites.size() ; i++ ){
	m_sites[i].m_bottom += min_y;
	for(int j = 0 ; j < (int)m_sites[i].m_interval.size() ; j++ ){
	    m_sites[i].m_interval[j] += min_x;
	}
    }

    for (int i = 0 ; i < (int)m_modules.size() ; i++ ){
	m_modules[i].m_x += min_x;
	m_modules[i].m_y += min_y;
	m_modules[i].m_cx += min_x;
	m_modules[i].m_cy += min_y;
    }
    for (int i = 0 ; i < (int)m_pins.size() ; i++ ){
	m_pins[i].absX += min_x;
	m_pins[i].absY += min_y;

    }
    m_dieArea.bottom += min_y;
    m_dieArea.top += min_y;
    m_dieArea.left += min_x;
    m_dieArea.right += min_x;
    SetCoreRegion();

}

void CPlaceDB::CheckRowHeight(double row_height)		//by indark
{
    for (unsigned int i = 0 ; i < m_sites.size() ; i ++ ){
	if (m_sites[i].m_height == 0)
	    m_sites[i].m_height = row_height;
    }
}

//Modified by Jin 20060323
int CPlaceDB::Align()
{
    m_modules_bak = m_modules;

    // by donnie 2006-04-02
    double xShift = 0 - m_coreRgn.left;
    double xScale = 1.0 / m_sites[0].m_step;
    
    CPlaceDBScaling::XShift( *this, xShift );
    CPlaceDBScaling::XScale( *this, xScale );

    double site_step = m_sites[0].m_step;
    double n_slot;
    double aligned_locx;
    int move_count = 0;
    for (unsigned int i = 0 ; i < m_modules.size() ; i++ )
    {
	const Module& curModule = m_modules[i];
	if( !curModule.m_isFixed )
	{
	    //n_slot = floor ( ( curModule.m_x+(site_step/2.0) / site_step) );
	    //aligned_locx = n_slot * site_step;
	    
	    
	    // TODO: alignment is not necesary from "0"
	    n_slot = floor ( ( ( curModule.m_x + (site_step/2.0) ) / site_step) );  // by donnie
	    aligned_locx = n_slot * site_step;
	    
	    if ( aligned_locx != curModule.m_x )
	    {
		move_count++;
		//printf( "x %.0f --> %.0f (n_slot %.0f step %.0f)\n", 
		//	curModule.m_x, aligned_locx, n_slot, site_step );
		SetModuleLocation( i, aligned_locx, curModule.m_y );	
	    }
	}

    }

    // by donnie 2006-04-02
    CPlaceDBScaling::XScale( *this, 1.0 / xScale );
    CPlaceDBScaling::XShift( *this, -xShift );
    
    if( param.bShow )
    {
	printf( "\nAlignment moves %d modules\n", move_count );
    }
    return move_count;

}

#if 0
void CPlaceDB::SetModuleType( const int & moduleId, const int & type )
{    
    m_modules[moduleId].m_lefCellId = type;
}
#endif

#if 0
void CPlaceDB::SetModuleIsPin( const int & moduleId, const bool & isPin )
{
    m_modules[moduleId].m_isPin = isPin;
}
#endif

// (kaie) 2009-07-20 Rotating r.s.t center
void CPlaceDB::CalcModuleLeftBottomLocation(const int& id)
{
	m_modules[id].m_x = m_modules[id].m_cx - m_modules[id].GetWidth() * 0.5;
	m_modules[id].m_y = m_modules[id].m_cy - m_modules[id].GetHeight() * 0.5;
}

void CPlaceDB::SetModuleOrientationCenter( const int & moduleId, const int & orient )
{
    int _orient = m_modules[moduleId].m_orient;
    int _start,_end,_count;
    //if (m_modules[moduleId].m_pinsId.size() != 0)
    {

	if ( (_orient %2 )!= (orient %2 )  )
	    swap(m_modules[moduleId].m_width,m_modules[moduleId].m_height);

	/*if((orient / 4 )!= (_orient / 4 )){
	  for(int i = 0 ; i < m_modules[moduleId].m_pinsId.size() ; i++){
	  m_pins[m_modules[moduleId].m_pinsId[i]].xOff =
	  -m_pins[m_modules[moduleId].m_pinsId[i]].xOff;

	  }

	  }*/
	if( _orient >= 4 )
	{
	    // flip back
	    for(int i = 0 ; i < (int)m_modules[moduleId].m_pinsId.size() ; i++){
		m_pins[m_modules[moduleId].m_pinsId[i]].xOff =
		    -m_pins[m_modules[moduleId].m_pinsId[i]].xOff;
	    }
	}

	_start = _orient %  4;
	_end = (orient %  4) + 4;
	_count = (_end - _start) % 4;
	for (int j = 0 ; j < _count ; j++ ){

	    /*
	    // clockwise 90 degree
	    for(int i = 0 ; i < m_modules[moduleId].m_pinsId.size() ; i++){
	    swap(m_pins[m_modules[moduleId].m_pinsId[i]].xOff ,
	    m_pins[m_modules[moduleId].m_pinsId[i]].yOff);
	    m_pins[m_modules[moduleId].m_pinsId[i]].yOff =
	    -m_pins[m_modules[moduleId].m_pinsId[i]].yOff;
	    }*/
	    // COUNTER-clockwise 90 degree
	    for(int i = 0 ; i < (int)m_modules[moduleId].m_pinsId.size() ; i++){
		swap(m_pins[m_modules[moduleId].m_pinsId[i]].xOff ,
			m_pins[m_modules[moduleId].m_pinsId[i]].yOff);
		m_pins[m_modules[moduleId].m_pinsId[i]].xOff =
		    -m_pins[m_modules[moduleId].m_pinsId[i]].xOff;
	    }

	}

	if( orient >= 4 )
	{
	    // flip new
	    for(int i = 0 ; i < (int)m_modules[moduleId].m_pinsId.size() ; i++){
		m_pins[m_modules[moduleId].m_pinsId[i]].xOff =
		    -m_pins[m_modules[moduleId].m_pinsId[i]].xOff;

	    }
	}

	CalcModuleLeftBottomLocation(moduleId);
	for(int i = 0 ; i < (int)m_modules[moduleId].m_pinsId.size() ; i++){
	    CalcPinLocation(m_modules[moduleId].m_pinsId[i]);
	}


    }


    m_modules[moduleId].m_orient = orient;

}
// @(kaie) 2009-07-20

// (kaie) 2009-08-13

vector<Module>* CPlaceDB::SortCompareModules::m_pMod = 0;
bool CPlaceDB::SortCompareModules::asending = false;

void CPlaceDB::SetModuleOrientationBest(bool macroonly, bool rotate, bool flip)
{
	//printf("Set best module orientations...\n");
	double HPWL1 = CalcHPWL();

	// sort modules
	vector<int> ProcessList;
	for(unsigned int i = 0; i < m_modules.size(); i++)
	    ProcessList.push_back(i);
	SortCompareModules::m_pMod = &m_modules;
	SortCompareModules::asending = false;
	if(gArg.CheckExist("assort")) SortCompareModules::asending = true;
	if(!gArg.CheckExist("nosort"))
	    sort(ProcessList.begin(), ProcessList.end(), SortCompareModules());

	//int count = 0;
	while(true)
	{
		//for(unsigned int i = 0; i < m_modules.size(); i++)
		for(unsigned int i = 0; i < ProcessList.size(); i++)
		{
			int mId = ProcessList[i];	
			//printf("%lf\n", m_modules[mId].m_area);

			if(m_modules[mId].m_isFixed || BlockOutCore(mId) == true 
				|| m_modules[mId].m_isCluster == true)
				continue;
			if(macroonly && m_modules[mId].m_height <= m_rowHeight) // do not rotate standard cells
				continue;

			double min_wirelength = CalcHPWLModule(mId);
			int moduleOrient = m_modules[mId].m_orient;
			int bestOrient = moduleOrient;
		
			//double moduleARatio = m_modules[mId].m_height / m_modules[mId].m_width;
			//double targetLRatio = 0.9;
			//double targetURatio = 1.1;

			if(m_modules[mId].m_height > m_rowHeight)
			{
			    moduleOrient++;
			    for(int j = 1; j < 8; j++, moduleOrient++)
			    {
				moduleOrient %= 8;

				if(!flip && moduleOrient >= 4) continue;
				if(!rotate && j%2 == 1) continue;
				
				SetModuleOrientationCenter(mId, moduleOrient);
				double wlength = CalcHPWLModule(mId);
				if(wlength < min_wirelength)
				{
				    min_wirelength = wlength;
				    bestOrient = moduleOrient;
				}
			    }
			    //if((fliponly && m_modules[mId].m_width == m_modules[mId].m_height) || 
			//	(!fliponly /*&& (moduleARatio > targetLRatio && moduleARatio < targetURatio)*/))
			    /*{
				moduleOrient++;
				for(int j = 1; j < 8; j++, moduleOrient++)
				{
					moduleOrient %= 8;
					SetModuleOrientationCenter(mId, moduleOrient);
					double wlength = CalcHPWLModule(mId);
					if(wlength < min_wirelength)
					{
						min_wirelength = wlength;
						bestOrient = moduleOrient;
					}
				}
				//if(bestOrient != moduleOrient%8) count++;
			    }else
			    {
				moduleOrient += 2;
				for(int j = 1; j < 4; j++, moduleOrient += 2)
				{
					moduleOrient %= 8;
					SetModuleOrientationCenter(mId, moduleOrient);
					double wlength = CalcHPWLModule(mId);
					if(wlength < min_wirelength)
					{
						min_wirelength = wlength;
						bestOrient = moduleOrient;
					}
				}
				//if(bestOrient != moduleOrient %8) count++;
			    }*/
			}else
			{
			    moduleOrient += 4;
			    moduleOrient %= 8;
			    SetModuleOrientationCenter(mId, moduleOrient);
			    double wlength = CalcHPWLModule(mId);
			    if(wlength < min_wirelength)
			    {
				min_wirelength = wlength;
				bestOrient = moduleOrient;
			    }
			    //if(bestOrient != moduleOrient %8) count++;
			}
			SetModuleOrientationCenter(mId, bestOrient);
    		}
		//printf("%d macros are rotated\n", count);
		double HPWL2 = CalcHPWL();
		if(fabs(HPWL1-HPWL2) < 1.0e-10) break;
		//else printf("HPWL = %lf\n", HPWL2);
		HPWL1 = HPWL2;
	}
}

void CPlaceDB::SetModuleNoFlip()
{
	for(unsigned int i = 0; i < m_modules.size(); i++)
	{
		if(m_modules[i].m_orient >= 4)
		SetModuleOrientationCenter(i, m_modules[i].m_orient-4);
	}
}

double CPlaceDB::CalcHPWLModule( const int& id )
{
	double HPWL = 0;
	for(unsigned int i = 0; i < m_modules[id].m_netsId.size(); i++)
	{
		int netId = m_modules[id].m_netsId[i];
		HPWL += GetNetLength(netId);
	}
	return HPWL;
}
// @(kaie) 2009-08-13

void CPlaceDB::SetModuleOrientation( const int & moduleId, const int & orient )
{
    int _orient = m_modules[moduleId].m_orient;
    int _start,_end,_count;
    //if (m_modules[moduleId].m_pinsId.size() != 0)
    {

	if ( (_orient %2 )!= (orient %2 )  )
	    swap(m_modules[moduleId].m_width,m_modules[moduleId].m_height);

	/*if((orient / 4 )!= (_orient / 4 )){
	  for(int i = 0 ; i < m_modules[moduleId].m_pinsId.size() ; i++){
	  m_pins[m_modules[moduleId].m_pinsId[i]].xOff =
	  -m_pins[m_modules[moduleId].m_pinsId[i]].xOff;

	  }

	  }*/
	if( _orient >= 4 )
	{
	    // flip back
	    for(int i = 0 ; i < (int)m_modules[moduleId].m_pinsId.size() ; i++){
		m_pins[m_modules[moduleId].m_pinsId[i]].xOff =
		    -m_pins[m_modules[moduleId].m_pinsId[i]].xOff;
	    }
	}

	_start = _orient %  4;
	_end = (orient %  4) + 4;
	_count = (_end - _start) % 4;
	for (int j = 0 ; j < _count ; j++ ){

	    /*
	    // clockwise 90 degree
	    for(int i = 0 ; i < m_modules[moduleId].m_pinsId.size() ; i++){
	    swap(m_pins[m_modules[moduleId].m_pinsId[i]].xOff ,
	    m_pins[m_modules[moduleId].m_pinsId[i]].yOff);
	    m_pins[m_modules[moduleId].m_pinsId[i]].yOff =
	    -m_pins[m_modules[moduleId].m_pinsId[i]].yOff;
	    }*/
	    // COUNTER-clockwise 90 degree
	    for(int i = 0 ; i < (int)m_modules[moduleId].m_pinsId.size() ; i++){
		swap(m_pins[m_modules[moduleId].m_pinsId[i]].xOff ,
			m_pins[m_modules[moduleId].m_pinsId[i]].yOff);
		m_pins[m_modules[moduleId].m_pinsId[i]].xOff =
		    -m_pins[m_modules[moduleId].m_pinsId[i]].xOff;
	    }

	}

	if( orient >= 4 )
	{
	    // flip new
	    for(int i = 0 ; i < (int)m_modules[moduleId].m_pinsId.size() ; i++){
		m_pins[m_modules[moduleId].m_pinsId[i]].xOff =
		    -m_pins[m_modules[moduleId].m_pinsId[i]].xOff;

	    }
	}

	CalcModuleCenter(moduleId);
	for(int i = 0 ; i < (int)m_modules[moduleId].m_pinsId.size() ; i++){
	    CalcPinLocation(m_modules[moduleId].m_pinsId[i]);
	}


    }


    m_modules[moduleId].m_orient = orient;

}


// donnie
int CPlaceDB::CalculateFixedModuleNumber()
{
    int num = 0;
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( m_modules[i].m_isFixed )
	    num++;
    }
    return num;
}

// donnie
int CPlaceDB::GetUsedPinNum()
{
    int num = 0;
    for( int i=0; i<(int)m_nets.size(); i++ )
	num += m_nets[i].size();
    return num;
}

// donnie
bool CPlaceDB::ModuleInCore( const int& i )
{
    if( !m_modules[i].m_isFixed )
	return true;

    // check only left-bottom
    double x = m_modules[i].m_x;
    double y = m_modules[i].m_y;
    if( x >= m_coreRgn.left && x <= m_coreRgn.right && 
	    y >= m_coreRgn.bottom && y <= m_coreRgn.top )
	return true;
    else
	return false;
}

// bool CPlaceDB::CheckStdCellOrient()
// {
//     bool ret = true;
//     double w, h;
//     for( int i=0; i<(int)m_modules.size(); i++ )
//     {
// 	if( m_modules[i].m_isFixed )
// 	    continue;

// 	w = m_modules[i].GetWidth();
// 	h = m_modules[i].GetHeight();
// 	if( h != m_rowHeight && w <= m_rowHeight )
// 	{
// 	    cout << "WARN: std-cell " << m_modules[i].GetName() 
// 		<< " orient " << orientStr( m_modules[i].m_orient )
// 		<< " height " << h 
// 		<< endl;
// 	    ret = false; 
// 	}
//     }
//     return ret;
// }


// 2006-02-16
int CPlaceDB::GetMovableBlockNumber()
{
    int n = 0;
    for( int i=0; i<(int)m_modules.size(); ++i )
    {
	if( false == m_modules[i].m_isFixed )
	{
	    n++;
	}
    }
    return n;
}

//2005-12-17
void CPlaceDB::ShowDBInfo()
{

    double coreArea = (m_coreRgn.right-m_coreRgn.left)*(m_coreRgn.top-m_coreRgn.bottom);

    // calc total area, cell area, macro area
    double cellArea = 0;
    double macroArea = 0;
    double moveArea = 0;
    double fixedArea = 0;
    int nCell = 0;
    int nMacro = 0;
    int nFixed = 0;
    //int smallBlock = 0;
    for( int i=0; i<(int)m_modules.size(); ++i )
    {
	//if( m_modules[i].m_isOutCore )
	//    continue;

	if( m_modules[i].m_height == m_rowHeight )
	{
	    nCell++;
	    cellArea += m_modules[i].m_area;
	}
	else if( m_modules[i].m_height > m_rowHeight )
	{
	    nMacro++;
	    macroArea += m_modules[i].m_area;
	}

	if( m_modules[i].m_isFixed )
	{
	    nFixed++;
	    fixedArea += m_modules[i].m_area;
	}
	else
	{
	    moveArea += m_modules[i].m_area;
	}
    }

#if 0
    int inPinNum = 0;
    int outPinNum = 0;
    int undefPinNum = 0;
    for( unsigned int i = 0; i<m_pins.size(); i++ )
    {
	if( m_pins[i].direction == 0 )        outPinNum++;
	else if( m_pins[i].direction == 1 )   inPinNum++;
	else
	{
	    if( m_pins[i].pinName != "VSS" && m_pins[i].pinName != "VDD" )
		printf( "Pin %s has no direction\n", m_pins[i].pinName.c_str() );
	    undefPinNum++;
	}
    }
#endif

    double fixedAreaInCore = GetFixBlockArea( m_coreRgn.left, m_coreRgn.bottom,  m_coreRgn.right, m_coreRgn.top );

#if 0
    // Need to remove sites under fixed macros first
    double totalSiteArea = 0;
    for( unsigned int i=0; i<m_sites.size(); i++ )
    {
	assert( m_sites[i].m_interval.size() % 2 == 0 );
	for( unsigned int j=0; j<m_sites[i].m_interval.size(); j+=2 )
	{
	    totalSiteArea += m_sites[i].m_interval[j+1] - m_sites[i].m_interval[j];
	}
    }
    totalSiteArea *= m_rowHeight;
    if( fabs( ( totalSiteArea - (coreArea - fixedAreaInCore) ) / totalSiteArea ) > 0.01 )
    {
	printf( "WARNING! SiteArea != CoreArea - FixedAreaInCore\n" );
    }
#endif

    int pin2 = 0, pin3 = 0, pin10 = 0, pin100 = 0;
    for( unsigned int i=0; i<m_nets.size(); i++ )
	if( m_nets[i].size() == 2 )
	    pin2++;
	else if( m_nets[i].size() < 10 )
	    pin3++;
	else if( m_nets[i].size() < 100 )
	    pin10++;
	else
	    pin100++;
    
    int rowNum = static_cast<int>((m_coreRgn.top-m_coreRgn.bottom)/m_rowHeight);
    int modNum = static_cast<int>(m_modules.size());
    int netNum = static_cast<int>(m_nets.size());
    int pinNum = m_pins.size();
    double hpwl = CalcHPWL();
    
    printf( "\n<<<< DATABASE SUMMARIES >>>>\n\n" );
    printf( "         Core region: ");
    m_coreRgn.Print();
    printf( "   Row Height/Number: %.0f / %d (site step %d)\n", m_rowHeight, rowNum, (int)m_sites[0].m_step );
    printf( "           Core Area: %.0f (%g)\n", coreArea, coreArea ); 
    printf( "           Cell Area: %.0f (%.2f%%)\n", cellArea, 100.0*cellArea/coreArea );
    if( nMacro > 0 )
    {
	printf( "          Macro Area: %.0f (%.2f%%)\n", macroArea, 100.0*macroArea/coreArea );
	printf( "  Macro/(Macro+Cell): %.2f%%\n", 100.0*macroArea/(macroArea+cellArea) );
    }
    printf( "        Movable Area: %.0f (%.2f%%)\n", moveArea, 100.0*moveArea/coreArea );
    if( nFixed > 0 )
    {
	printf( "          Fixed Area: %.0f (%.2f%%)\n", fixedArea, 100.0*fixedArea/coreArea );
	printf( "  Fixed Area in Core: %.0f (%.2f%%)\n", fixedAreaInCore, 100.0*fixedAreaInCore/coreArea );
    }
    //printf( "   (Macro+Cell)/Core: %.2f%%\n", 100.0*(macroArea+cellArea)/coreArea );
    printf( "     Placement Util.: %.2f%% (=move/freeSites)\n", 100.0*moveArea/(coreArea-fixedAreaInCore) );
    printf( "        Core Density: %.2f%% (=usedArea/core)\n", 100.0*(moveArea+fixedAreaInCore)/coreArea );
    //printf( "           Site Area: %.0f (%.0f)", totalSiteArea, coreArea-fixedAreaInCore );
    printf( "              Cell #: %d (=%dk)\n", nCell , (nCell/1000) );
    printf( "            Object #: %d (=%dk) (fixed: %d) (macro: %d)\n", modNum, modNum/1000, nFixed, nMacro );
    if( nMacro < 20 )
    {
	for( int i=0; i<(int)m_modules.size(); ++i )
	    if( m_modules[i].m_height > m_rowHeight )
		printf( " Macro: %s\n", m_modules[i].GetName().c_str() );
    }
    printf( "               Net #: %d (=%dk)\n", netNum, netNum/1000 );
    printf( "                  Pin 2 (%d) 3-10 (%d) 11-100 (%d) 100- (%d)\n", pin2, pin3, pin10, pin100 );
    printf( "               Pin #: %d\n", pinNum );
    //printf( "               Pin #: %d (in: %d  out: %d  undefined: %d)\n", pinNum, inPinNum, outPinNum, undefPinNum );
    printf( "     Pin-to-Pin HPWL: %.0f (%g)\n", hpwl, hpwl );

    if( gArg.IsDev() )
    {
	double xhpwl = CalcXHPWL();
	printf( "    Pin-to-Pin XHPWL: %.0f (%g)(%.2f)\n", xhpwl, xhpwl, xhpwl/hpwl );
	//double lpnormwire = CalcLpNorm();
	//printf( "             Lp-Norm: %.0f (%g)(%.2f)\n", lpnormwire, lpnormwire, lpnormwire/hpwl ) ;
    }

    ShowDensityInfo();
    printf( "\n" );

    m_util = moveArea/(coreArea-fixedAreaInCore);
}

// 2006-09-21 (donnie)
// void CPlaceDB::ShowSteinerWL()
// {
//     double hpwl = CalcHPWL();

//     double xSteinerWL = GetTotalSteinerWL( 0, 1 );  // FLUTE, X-arch
//     double mSteinerWL = GetTotalSteinerWL( 0, 0 );  // FLUTE, Manhanttan
//     printf( "        M Steiner WL: %.0f (%g)(%.2f)\n", mSteinerWL, mSteinerWL, mSteinerWL/hpwl );
//     printf( "        X Steiner WL: %.0f (%g)(%.2f)\n", xSteinerWL, xSteinerWL, xSteinerWL/hpwl );
// }

void CPlaceDB::ShowDensityInfo()
{
    double wire = CalcHPWL();

    CPlaceBin placeBin( *this );

    double penalty;

    if( param.coreUtil < 1.0 )
    {
	placeBin.CreateGrid( m_rowHeight * 10.0, m_rowHeight * 10.0 );  // ISPD-06 contest standard
	penalty = placeBin.GetPenalty( param.coreUtil );
	printf( "  Density Penalty %.2f = %.2f \t(dHPWL = %.0f)\n", param.coreUtil, penalty, wire*(1+penalty/100.0) );
    }
    else if( param.coreUtil >= 1.0 )
    {
	placeBin.CreateGrid( static_cast<int>( sqrt( m_modules.size() ) ), static_cast<int>( sqrt( m_modules.size() ) ) );
	penalty = placeBin.GetOverflowRatio( param.coreUtil );
	printf( "        Overflow Ratio = %.3f%%\n", penalty * 100.0 );
    }

    //placeBin.ShowInfo( 0.9 );
    //placeBin.ShowInfo( 0.8 );
}

void CPlaceDB::AddNet( Net n )
{
    m_nets.push_back( n );
}

void CPlaceDB::AddNet( Net n, const char* name )
{
    m_nets.push_back( n );
    m_netsName.resize( m_nets.size() );
    m_netsName[ m_nets.size()-1 ] = name;
}

void CPlaceDB::AddNet( set<int> n )
{
    Net dummy;
    m_nets.push_back( dummy );
    Net& net = m_nets[ m_nets.size()-1 ];
    net.reserve( n.size() );
    set<int>::const_iterator ite;
    for( ite=n.begin(); ite!=n.end(); ite++ )
    {
	net.push_back( *ite );
    }
}

int CPlaceDB::AddPin( const int& moduleId, const float& xOff, const float& yOff, const int dir )
{
    //cout << "AddPin( " << xOff << ", " << yOff << ")\n";
    m_pins.push_back( Pin( xOff, yOff ) );
    int pid = (int)m_pins.size() - 1;
    m_modules[moduleId].m_pinsId.push_back( pid );
    m_pins[pid].moduleId = moduleId;
    m_pins[pid].direction = dir;
    return pid;
}

int CPlaceDB::AddPin( const int& moduleId, const string& pinName, 
	const float& xOff, const float& yOff, const int dir )	// 2005-08-29
{
    m_pins.push_back( Pin( pinName, xOff, yOff ) );
    int pid = (int)m_pins.size() - 1;
    m_modules[moduleId].m_pinsId.push_back( pid );
    m_pins[pid].moduleId = moduleId;
    m_pins[pid].direction = dir;
    return pid;
}


// Memory allocation
void CPlaceDB::ReserveModuleMemory( const int& n )
{
    m_modules.reserve( n );
}
void CPlaceDB::ReserveNetMemory( const int& n )
{
    CPlaceDB::m_nets.reserve( n );
}
void CPlaceDB::ReservePinMemory( const int& n )
{
    m_pins.reserve( n );
}


void CPlaceDB::RemoveDummyFixedBlock()
{
    m_modules.resize( realModuleNumber );
}

// Create dummy blocks for the non-placing sites
int CPlaceDB::CreateDummyFixedBlock()
{
    double currentX;
    char name[1000];
    int counter = 0;
    realModuleNumber = m_modules.size();
    for( int i=0; i<(int)m_sites.size(); i++ )
    {
	currentX = m_coreRgn.left;
	for( int j=0; j<(int)m_sites[i].m_interval.size(); j+=2 )
	{
	    if( m_sites[i].m_interval[j] > currentX )
	    {
		sprintf( name, "__%d", counter );
		AddModule( name, float(m_sites[i].m_interval[j]-currentX), m_sites[i].m_height, true );
		SetModuleLocation( (int)m_modules.size()-1, currentX, m_sites[i].m_bottom );
		counter++;
	    }
	    currentX = m_sites[i].m_interval[j+1];
	}
	if( currentX < m_coreRgn.right )
	{
	    sprintf( name, "__%d", counter );
	    AddModule( name, m_coreRgn.right-currentX, m_sites[i].m_height, true );
	    SetModuleLocation( (int)m_modules.size()-1, currentX, m_sites[i].m_bottom );
	    counter++;
	}

    } // for m_site
    return counter;
}

//Added by Jin 20060228
double CPlaceDB::GetModuleTotalNetLength( const int& mid )
{
    double result = 0.0;
    const Module& curModule = m_modules[mid];

    for( vector<int>::const_iterator iteNetId = curModule.m_netsId.begin() ;
	    iteNetId != curModule.m_netsId.end() ; iteNetId++ )
    {
	result += GetNetLength( *iteNetId );
    }

    return result;
}
//@Added by Jin 20060228

//Added by Jin 20060302
void CPlaceDB::RemoveMacroSite()
{

    printf( "Remove sites under macros...\n" );

    /// @todo implement me
    double row_top =  m_sites.back().m_bottom +  m_sites.back().m_height;
    double row_bottom =  m_sites.front().m_bottom;

    for( vector<Module>::iterator iteModule =  m_modules.begin() ;
	    iteModule <  m_modules.end() ;
	    iteModule++ )
    {
	double module_top = iteModule->m_y + iteModule->m_height;
	double module_bottom = iteModule->m_y;
	double module_left = iteModule->m_x;
	double module_right = iteModule->m_x + iteModule->m_width;
	if( iteModule->m_height > m_rowHeight && 
		module_bottom < row_top && // Confirm that the module is overlapped 
		// with the rows in y coordinates
		module_top > row_bottom )

	{
	    vector<CSiteRow>::iterator iteBeginRow, iteEndRow;

	    // find the begin row
	    for( iteBeginRow =  m_sites.begin() ; 
		    iteBeginRow <  m_sites.end() ; 
		    iteBeginRow++ )
	    {

		if( iteBeginRow->m_bottom + iteBeginRow->m_height > module_bottom )
		{
		    break;
		}
		//if( iteBeginRow->m_bottom == iteModule->m_y )
		//{
		//	break;
		//}
		//else if( iteBeginRow->m_bottom > iteModule->m_y )
		//{
		//	if( iteBeginRow >  m_sites.begin() )
		//	{
		//		iteBeginRow--;
		//	}

		//	break;
		//}
	    }




	    for( iteEndRow = iteBeginRow ;
		    iteEndRow <  m_sites.end() ;
		    iteEndRow++ )
	    {
		if( iteEndRow->m_bottom + iteEndRow->m_height >= module_top )
		{
		    break;
		}
	    }

	    if( iteEndRow ==  m_sites.end() )
		iteEndRow--;


	    assert( iteBeginRow !=  m_sites.end() );

	    for( vector<CSiteRow>::iterator iteRow = iteBeginRow ;
		    iteRow <= iteEndRow ;
		    iteRow++ )
	    {
		double interval[2];
		for( int i = 0 ; i < (signed)iteRow->m_interval.size() ; i++ )
		{
		    interval[ i % 2 ] = iteRow->m_interval[i];
		    if( ( i % 2 ) == 1 ) // Get two terminals of the interval
		    {
			if( interval[0] >= module_right || interval[1] <= module_left )  // screen unnecessary checks
			    continue;

			if( interval[0] >= module_left && interval[1] <= module_right )
			{
			    iteRow->m_interval.erase( vector<double>::iterator(&(iteRow->m_interval[i])) );
			    iteRow->m_interval.erase( vector<double>::iterator(&(iteRow->m_interval[i-1])) );
			    i = i - 2;
			}
			else if( interval[1] > module_right && interval[0] >= module_left )
			{
			    iteRow->m_interval[i-1] = module_right;
			}
			else if( interval[0] < module_left && interval[1] <= module_right )
			{
			    iteRow->m_interval[i] = module_left;
			}
			else if( interval[0] < module_left && interval[1] > module_right )
			{
			    iteRow->m_interval[i] = module_left;
			    iteRow->m_interval.insert( vector<double>::iterator(&(iteRow->m_interval[i+1])), interval[1] );
			    iteRow->m_interval.insert( vector<double>::iterator(&(iteRow->m_interval[i+1])), module_right );
			    i = i + 2;

			}
			else
			{
			    printf("Warning: Module Romoving Error\n");
			    //exit(-1);
			}

		    }
		}
	    }

	    //@remove the occupied sites

	}
    }

    //Align site interval to site step
    FixFreeSiteBySiteStep();

#if 0 
    //test code
    ofstream sitefile( "sites.log" );
    for( vector<CSiteRow>::iterator iteRow = m_sites.begin() ; 
	    iteRow != m_sites.end() ; iteRow++ )
    {
	sitefile << "Row bottom: " << iteRow->m_bottom << " ";

	for( unsigned int iInterval = 0 ; 
		iInterval != iteRow->m_interval.size() ; iInterval=iInterval+2 )
	{
	    sitefile << "(" << iteRow->m_interval[iInterval] 
		<< "," << iteRow->m_interval[iInterval+1]  << ") ";
	}
	sitefile << endl;
    }

    sitefile.close();
    cout << "Dump site.log" << endl;
    //@test code
#endif
}

//@Added by Jin 20060302


double CPlaceDB::GetHPWLdensity( double util )
{
    CalcHPWL();

    if( util < 0 )
	return m_HPWLp2p;

    CPlaceBin placeBin( *this );
    placeBin.CreateGrid( m_rowHeight * 10.0, m_rowHeight * 10.0 );

    double penalty = placeBin.GetPenalty( util );
    return m_HPWLp2p*(1+penalty/100.0);
}

//2006-03-06
void CPlaceDB::SetAllBlockMovable()
{

    printf( "Set all block movable... " );
    int count=0;
    for( unsigned int i=0; i<m_modules.size(); i++ )
    {
	//if( m_modules[i].m_isOutCore == false )
	if( BlockOutCore( i ) == false )
	{
	    if( m_modules[i].m_isFixed )
	    {
		count++;
		m_modules[i].m_isFixed = false;
// 		MoveModuleCenter( i, 0.0, 0.0);
	    	FixedBlockSet.push_back(i);
	    }
	}
    }
    printf( "%d blocks\n", count ); 
}

//(kaie) 2009-06-25
void CPlaceDB::SetMacroFixed(const double ratio)
{
    printf("Set Macro Fixed.... ");
    int number = 0;
    double avg_area = m_totalModuleArea/m_modules.size();
    for(unsigned int i = 0; i < m_modules.size(); i++)
    {
	    if(m_modules[i].m_area > avg_area * ratio && m_modules[i].m_height > m_rowHeight)
	    {
		    SetModuleFixed(i);
		    number++;
	    }
    }
    printf("end\n");
    printf("%d modules are fixed\n", number);
}
void CPlaceDB::RestoreFixedBlocks()
{
    printf("Restore fixed blocks... ");
    for( unsigned int i = 0; i < FixedBlockSet.size(); i++)
    {
	    m_modules[FixedBlockSet[i]].m_isFixed = m_modules_bak[FixedBlockSet[i]].m_isFixed = true;
    }
    printf("end\n");
}
// @(kaie) 2009-06-25

void CPlaceDB::FixFreeSiteBySiteStep(void)
{
    double chip_left_bound = m_coreRgn.left;
    double site_step = m_sites.front().m_step;
    for( vector<CSiteRow>::iterator iteSite = m_sites.begin() ;
	    iteSite != m_sites.end() ; iteSite++ )
    {
	for( unsigned int i = 0 ; i < iteSite->m_interval.size() ; i=i+2 )
	{
	    double interval_left = iteSite->m_interval[i];
	    double interval_right = iteSite->m_interval[i+1];


	    // 2006-9-28 (donnie) remove small sites
	    // assume legal block width > 1
	    double old_width = interval_right - interval_left;
	    if( old_width < site_step )	
	    {
		iteSite->m_interval.erase( static_cast<vector<double>::iterator>(&iteSite->m_interval[i+1]) );
		iteSite->m_interval.erase( static_cast<vector<double>::iterator>(&iteSite->m_interval[i]) );
		i = i-2;
		continue;
	    }


	    double new_left = ceil( (interval_left-chip_left_bound)/site_step ) * site_step + chip_left_bound;
	    double new_right = floor( (interval_right-chip_left_bound)/site_step ) * site_step + chip_left_bound;
	    double new_width = new_right - new_left;
	    if( new_width > 0 )
	    {
		iteSite->m_interval[i] = new_left;
		iteSite->m_interval[i+1] = new_right;
	    }
	    else
	    {
		iteSite->m_interval.erase( static_cast<vector<double>::iterator>(&iteSite->m_interval[i+1]) );
		iteSite->m_interval.erase( static_cast<vector<double>::iterator>(&iteSite->m_interval[i]) );
		i = i-2;

		if( new_width < 0 )
		{
		    fprintf( stderr, "Warning: FixFreeSiteBySiteStep() computes illegal site interval\n" );
		    fprintf( stderr, " site bottom: %.2f orig left: %.2f orig right: %.2f new left: %.2f new right: %.2f\n",
			    iteSite->m_bottom, interval_left, interval_right, new_left, new_right );
		    fprintf( stderr, " chip left: %.2f  site step: %.2f \n",
			    chip_left_bound, site_step );
		}
	    }

	}	    
    }
}



// 2006-05-23 (donnie)
double CPlaceDB::CalcXHPWL()
{
    double XHPWL = 0;
    for( int i=0; i<(int)m_nets.size(); i++ )
	XHPWL += GetXNetLength( m_nets[i] );
    return XHPWL;
}

// 2006-05-23 (donnie)
double CPlaceDB::GetXNetLength( vector<int>& pinsId )
{
    if( pinsId.size() <= 1 )
	return 0.0;

    int min45Id;   // min point of the TL-BR line  "\"   BL    
    int max45Id;   // max point of the TL-BR line  "\"   TR
    int min135Id;   // min point of the TR-BL line  "/"  BR
    int max135Id;   // max point of the TR-BL line  "/"  TL
    double min45, max45;
    double min135, max135;
    double x1, x2, y1, y2;

    int pinId = pinsId[0];
    assert( pinId <= (int)m_pins.size() );
    x1 = x2 = m_pins[pinId].absX; 
    y1 = y2 = m_pins[pinId].absY;
    min45Id = max45Id = min135Id = max135Id = pinId;
    min45 = max45 = m_pins[pinId].absX + m_pins[pinId].absY;   // (x+y)
    min135 = max135 = m_pins[pinId].absX - m_pins[pinId].absY; // (x-y)

    for( int i=1; i<(int)pinsId.size(); i++ )
    {
	pinId = pinsId[i];
	if( m_pins[pinId].absX > x2 )   
	    x2 = m_pins[pinId].absX;
	else if( m_pins[pinId].absX < x1 )   
	    x1 = m_pins[pinId].absX;

	if( m_pins[pinId].absY > y2 )   
	    y2 = m_pins[pinId].absY;
	else if( m_pins[pinId].absY < y1 )   
	    y1 = m_pins[pinId].absY;

	double line45  = m_pins[pinId].absX + m_pins[pinId].absY;
	double line135 = m_pins[pinId].absX - m_pins[pinId].absY;
	if( line45 < min45 )
	{
	    min45 = line45;
	    min45Id = pinId;
	}
	else if( line45 > max45 )
	{
	    max45 = line45;
	    max45Id = pinId;
	}
	if( line135 < min135 )
	{
	    min135 = line135;
	    min135Id = pinId;
	}
	else if( line135 > max135 )
	{
	    max135 = line135;
	    max135Id = pinId;
	}
    }

    double XHPWL = 
	(sqrt(2) - 1) *   ( x2 - x1 + y2 - y1 ) - 
	(sqrt(2)/2 - 1) * ( max45 - min45 + max135 - min135 );
    //assert( max135 >= min135 );
    //assert( max45 >= min45 );
    /*
       double reduce;
       double ratio = ( 2 - sqrt( 2.0 ) ) * 0.5;
    // BL
    reduce = ( min45 - (x1+y1) ) * ratio;
    assert( reduce >= -1e-5 );
    XHPWL -= reduce;
    // TR
    reduce = ( (x2+y2) - max45 ) * ratio;
    assert( reduce >= -1e-5 );
    XHPWL -= reduce;
    // BR
    reduce = ( (x2-y1) - max135 ) * ratio;
    assert( reduce >= -1e-5 );
    XHPWL -= reduce;
    // TL
    reduce = ( min135 - (x1-y2) ) * ratio;
    assert( reduce >= -1e-5 );
    XHPWL -= reduce;
    */
    assert( XHPWL >= 0.0 );
    return XHPWL;	   
}


// void CPlaceDB::OutputAstroDump( const char* filename )
// {
//     string s;
//     string s_new;
//     int s_index;
//     int i_x;
//     int i_y;

//     cout << "Output dump file: " << filename << endl;
//     ofstream out( filename );
//     out << "; ** MACROS **\n";
//     out << "define _cell (geGetEditCell)\n";

//     for( int i=0; i<(int)m_modules.size(); i++ )
//     {
// 	if( m_modules[i].m_isFixed )
// 	    continue;

// 	//double w = m_modules[i].width;
// 	//double h = m_modules[i].height;
// 	//FIXME 
// 	//if( m_modules[i].rotate )
// 	//	swap( w, h );

// 	double x = m_modules[i].m_cx / (parserLEFDEF.m_defUnit);
// 	double y = m_modules[i].m_cy /(parserLEFDEF.m_defUnit);
// 	string degree;
// 	string flip;
// 	degree = "0";
// 	flip = "no";

// 	if( m_modules[i].m_orient == 0 || m_modules[i].m_orient == 4 )
// 	    degree = "0";
// 	else if( m_modules[i].m_orient == 3 || m_modules[i].m_orient == 7 )
// 	    degree = "270";
// 	else if( m_modules[i].m_orient == 2 || m_modules[i].m_orient == 6 )

// 	    degree = "180";
// 	else if( m_modules[i].m_orient == 1 || m_modules[i].m_orient == 5 )
// 	    degree = "90";
// 	else
// 	{
// 	    printf( "ERROR orientation= %d\n", m_modules[i].m_orient );
// 	    exit(0);
// 	}	

// 	// 	if( orientation[i] == "FN" )
// 	// 	{
// 	// 		x += w; 
// 	// 	}
// 	// 	else if( orientation[i] == "E" )
// 	// 	{
// 	// 		y += h;
// 	// 	}
// 	// 	else if( orientation[i] == "FE" )
// 	// 	{
// 	// 		y += h;
// 	// 		x += w;
// 	// 	}
// 	// 	else if( orientation[i] == "S" )
// 	// 	{
// 	// 		y += h;
// 	// 		x += w;
// 	// 	}
// 	// 	else if( orientation[i] == "FS" )
// 	// 	{
// 	// 		y += h;
// 	// 	}
// 	// 	else if( orientation[i] == "W" )
// 	// 	{
// 	// 		x += w;
// 	// 	}

// 	if( m_modules[i].m_orient >= 4 )
// 	    flip = "X";

// 	// Special Process
// 	s = m_modules[i].m_name;

// 	if ( s.substr(s.length()-1).compare("]")== 0){
// 	    s_index = s.find_last_of("/");
// 	    //s_new = s.substr(0,s.length() - s_index);
// 	    s_new = s.substr( s_index + 1); 
// 	    s_new = s.substr(0, s.length() -s_new.length()) + "\\\\"  + s_new;
// 	    // s_new += "\\";
// 	    // s_new += s.substr(s_index+1);
// 	}else{
// 	    s_new = s;
// 	}

// 	i_x = static_cast<int>(x);
// 	i_y = static_cast<int>(y);

// 	out << "dbCellInstPlaceAndStatus _cell \"" << s_new
// 	    << "\" \"" << degree   
// 	    << "\" \"" << flip << "\" \"cc\" '("
// 	    << x << " "
// 	    << y << ") \"Placed\"\n";
//     }


//     // output placement blockages
//     //     out << "; ** PLACEMENT BLOCKAGES **\n";
//     //     out << "axPurgePlaceBlockage (geGetEditCell)\n";
//     //     out << "axPurgeSoftPlaceBlockage (geGetEditCell)\n";
//     //     out << "define _cell (geGetEditCell)\n";
//     //     for( int i=0; i<(int)m_modules.size(); i++ )
//     //     {
//     //         if( m_modules[i].is_pad )
//     // 	    continue;
//     //         if( strnm_pCongMap( m_modules[i].name, "C_", 2 ) == 0 )
//     //             continue;   // cluster
//     // 	if( m_modules[i].is_dummy )
//     // 	    continue;
//     // 	if( m_modules[i].is_fixed )
//     // 	    continue;
//     // 
//     //         double w = m_modules[i].width;
//     //         double h = m_modules[i].height;
//     //         if( m_modulesInfo[i].rotate )
//     //                 swap( w, h );
//     // 
//     //         double x = m_modules[i].x;
//     //         double y = m_modules[i].y;
//     // 
//     // 	x -= SPACING;
//     // 	y -= SPACING;
//     // 	w += 2 * SPACING;
//     // 	h += 2 * SPACING;
//     // 
//     // 	out << "define _obj (dbCreateRectangle _cell 221 '((" 
//     // 	    << x << " " << y << ")(" << x+w << " " << y+h << ")))\n";	
//     //     }

// }

double CPlaceDB::GetNetLpNorm( int netId )
{
    double x_max = 0.0;
    double y_max = 0.0;
    double x_min = 0.0;
    double y_min = 0.0;
    double p = param.dLpNorm_P;
    double _lpnorm;
    int pinId;

    int pid = m_nets[netId][0];
    double cx, cy;
    GetPinLocation( pid, cx, cy );
    double maxX = cx;
    double minX = cx;
    double minY = cy;
    double maxY = cy;
    double hpwl;


    if( m_nets[netId].size() <= 1 )
	return 0.0;

    //double scale = 1.0 / m_coreRgn.right; // avoid overflow
    double scale = 10.0 / m_coreRgn.right; // avoid overflow
    //double scale = 1.0;

    for( int i=0; i<(int)m_nets[netId].size(); i++ ){
	pinId = m_nets[netId][i];
	x_max += pow(static_cast<double>(m_pins[pinId].absX * scale),p);
	x_min += pow(static_cast<double>(m_pins[pinId].absX * scale),-p);
	y_max += pow(static_cast<double>(m_pins[pinId].absY * scale),p);
	y_min += pow(static_cast<double>(m_pins[pinId].absY * scale),-p);

	//pid = m_nets[netId][i];
	//GetPinLocation( pid, cx, cy );
	//minX = min( minX, cx );
	//maxX = max( maxX, cx );
	//minY = min( minY, cy );
	//maxY = max( maxY, cy );
    }
    x_max = pow(x_max, 1/p);
    x_min = pow(x_min, -1/p);
    y_max = pow(y_max, 1/p);
    y_min = pow(y_min, -1/p);
    // 	printf(" %d: X MAX  %f %f (%f) MIN %f %f (%f)\n", m_nets[netId].size(),  x_max ,maxX ,( x_max -maxX ) / maxX  ,x_min , minX , (x_min-minX) / minX);

    // 	printf(" %d: Y MAX  %f %f (%f) MIN %f %f (%f)\n", m_nets[netId].size(),  y_max ,maxY ,( y_max -maxY ) / maxY  ,y_min , minY , (y_min-minY) / minY);
    hpwl = maxX - minX + maxY - minY;
    _lpnorm = (x_max - x_min + y_max - y_min) / scale;


    // 	printf(" %d: %f %f (%f) \n",netId, _lpnorm , hpwl , (_lpnorm - hpwl ) / hpwl);

    return _lpnorm;

}


// 2006-05-26 (indark)
double CPlaceDB::CalcLpNorm()
{
    double _lpnorm = 0;
    for( int i=0; i<(int)m_nets.size(); i++ )
	_lpnorm += GetNetLpNorm( i );
    return _lpnorm;
}


#if 0
// 2006-06-19 (donnie)
void CPlaceDB::ConnectLiberty( Liberty* pLibDB )
{
    printf( "Connect PlaceDB with LibertyDB\n" );
    m_pLib = pLibDB;
    for( unsigned int i=0; i<m_modules.size(); i++ )
    {
	if( m_modules[i].m_blockType == BT_PI ||
		m_modules[i].m_blockType == BT_PO )
	    continue;

	// link cells
	int libCellId;
	if( m_pLEF == NULL )
	{
	    // no LEF lib
	    libCellId = pLibDB->getCellId( m_modules[i].m_name.c_str() );
	    if( libCellId >= 0 )
		m_modules[i].m_libCellId = libCellId;
	    else
		printf( "  Cannot find liberty cell %s\n", m_modules[i].m_name.c_str() );
	}
	else
	{
	    // with LEF lib
	    int lefCellId = m_modules[i].m_lefCellId;
	    libCellId = pLibDB->getCellId( m_pLEF->m_modules[lefCellId].m_name.c_str() );
	    if( libCellId >= 0 )
		m_pLEF->m_modules[lefCellId].m_libCellId = m_modules[i].m_libCellId = libCellId;
	    else
		printf( "  Cannot find liberty cell %s(%s)\n", m_pLEF->m_modules[lefCellId].m_name.c_str(), m_modules[i].m_name.c_str() );
	}	   

	// link pins if liberty cell is found
	if( libCellId >= 0 )
	    for( unsigned int p=0; p<m_modules[i].m_pinsId.size(); p++ )
	    {
		int pinId = m_modules[i].m_pinsId[p];

		if( m_pins[pinId].pinName == "VDD" ||
			m_pins[pinId].pinName == "VSS" ||
			m_pins[pinId].pinName == "vdd" ||
			m_pins[pinId].pinName == "vss" )
		    continue; // no need for connection for P/G

		if( m_pins[pinId].libPinId != -1 )
		{
		    printf( "Error, Pin %s of Cell %s is already connected. (multiplt connection?)\n", 
			    m_pins[pinId].pinName.c_str(), m_modules[i].m_name.c_str() );
		    exit(0);
		}
		int connectLibPinId = -1;
		for( unsigned int q=0; q<m_pLib->m_cells[libCellId].pinsId.size(); q++ ) // find the corresponding pin
		{
		    int libPinId = m_pLib->m_cells[libCellId].pinsId[q];
		    if( m_pLib->m_pins[libPinId].name == m_pins[pinId].pinName )
		    {
			connectLibPinId = libPinId; // find!
			assert( m_pins[pinId].direction == -1 || m_pins[pinId].direction == m_pLib->m_pins[libPinId].direction ); // undefined
			m_pins[pinId].direction = m_pLib->m_pins[connectLibPinId].direction;   // copy port direction
			m_pins[pinId].libPinId = connectLibPinId; // connect pointer
			//printf( "cell %s pin %s direction %d\n", 
			//	m_modules[i].m_name.c_str(), m_pins[pinId].pinName.c_str(), m_pins[pinId].direction );
			break;
		    }
		}
		if( connectLibPinId == -1 )
		{
		    printf( "Cannot find corresponding pin %s in cell %s\n", m_pins[pinId].pinName.c_str(), m_modules[i].m_name.c_str() );
		}
	    }

    } // each cell
}
#endif


#if 0
// 2006-06-22 (donnie)
double CPlaceDB::GetNetLoad( const int& netId )
{
    // TODO: consider wire load

    double totalCap = 0;
    int outCount = 0;
    for( unsigned int i=0; i<m_nets[netId].size(); i++ )
    {
	int pinId = m_nets[netId][i];
	int modId = m_pins[pinId].moduleId;

	if( m_pins[pinId].direction == PIN_DIRECTION_OUT )
	    outCount++;
	else if( m_pins[pinId].direction == PIN_DIRECTION_IN )
	{
	    if( m_pins[pinId].libPinId < 0 )
	    {
		if( m_modules[modId].m_blockType == BT_PO )
		{
		    // TODO: use user specified load
		}
		else
		{
		    printf( "Net %d: Port %s of Cell %s (%s)(%d) has no corresponding LibPin\n", 
			    netId, m_pins[pinId].pinName.c_str(), m_modules[modId].m_name.c_str(), 
			    m_pLEF->m_modules[m_modules[modId].m_lefCellId].m_name.c_str(),
			    m_modules[modId].m_libCellId );
		}
	    }
	    else
		totalCap += m_pLib->m_pins[ m_pins[pinId].libPinId ].cap;
	}
	else 
	{
	    printf( "Port %s of Cell %s (net %d) direction undefined\n", 
		    m_pins[pinId].pinName.c_str(), m_modules[modId].m_name.c_str(), netId );
	}
    }
    return totalCap;
}
#endif

// void CPlaceDB::IdentifyDFF()
// {
//     int count = 0;
//     for( unsigned int i=0; i<m_modules.size(); i++ )
//     {
// 	if( m_modules[i].m_lefCellId >= 0 )
// 	{
// 	    if( m_pLEF->m_modules[ m_modules[i].m_lefCellId ].m_name.substr( 0, 3 ) == "DFF" )
// 	    {
// 		assert( m_modules[i].m_blockType != BT_PI );
// 		assert( m_modules[i].m_blockType != BT_PO );
// 		assert( m_modules[i].m_blockType != BT_PIN );
// 		m_modules[i].m_blockType = BT_DFF;
// 		count++;
// 	    }
// 	}
//     }
//     printf( "%d DFFs are identified.\n", count );
// }


int CPlaceDB::GetNetIdByBlockAndPin( const int& blockId, const int& pinId )
{
    for( unsigned int n=0; n<m_modules[blockId].m_netsId.size(); n++ )
    {
	int netId = m_modules[blockId].m_netsId[n];
	for( unsigned p=0; p<m_nets[netId].size(); p++ )
	    if( m_nets[netId][p] == pinId )
		return netId;
    }

    //printf( "Cannot find the net id by block %s(%d) and pin %s(%d)\n", // floating?
    //	    m_modules[blockId].m_name.c_str(), blockId, 
    //	    m_pins[pinId].pinName.c_str(), pinId );
    //exit(0);
    return -1; // not found
}

#if 0
void CPlaceDB::InitializeBlockInOut()
{
    for( unsigned int i=0; i<m_modules.size(); i++ )
    {
	m_modules[i].m_inBlocksId.clear();
	m_modules[i].m_outBlocksId.clear();

	for( unsigned int p=0; p<m_modules[i].m_pinsId.size(); p++ )
	{
	    int pinId = m_modules[i].m_pinsId[p];

	    if( m_pins[pinId].pinName == "VDD" ||
		    m_pins[pinId].pinName == "VSS" ||
		    m_pins[pinId].pinName == "vdd" ||
		    m_pins[pinId].pinName == "vss" )
		continue;

	    if( m_pins[pinId].direction == PIN_DIRECTION_IN )
	    {
		// find the driving pin
		int netId = GetNetIdByBlockAndPin( i, pinId );

		if( netId < 0 )
		    continue; // floating-port

		if( m_groundNets.find( netId ) != m_groundNets.end() )
		    continue;	// port connects to GND

		int driveCount = 0;
		bool foundSelfCount = 0;
		//printf( "Net %d: ", netId );
		for( unsigned netPin=0; netPin<m_nets[netId].size(); netPin++ )
		{
		    int drivePin = m_nets[netId][netPin];
		    //printf( "%s(%s)(%d) ", 
		    //	    m_modules[m_pins[drivePin].moduleId].m_name.c_str(), 
		    //	    m_pins[drivePin].pinName.c_str(),
		    //	    m_pins[drivePin].direction );
		    if( m_pins[drivePin].direction == PIN_DIRECTION_OUT )
		    {
			driveCount++;
			assert( driveCount == 1 );  // multiplt driving?
			assert( m_pins[drivePin].moduleId != (int)i );   // driving self?
			m_modules[i].m_inBlocksId.push_back( m_pins[drivePin].moduleId );
			//break; // unmark to speed up 
		    }
		    else if( m_pins[drivePin].direction == PIN_DIRECTION_IN )
		    {
			if( m_pins[drivePin].moduleId == (int)i )
			    foundSelfCount++;
		    }
		}
		//printf( "\n" );
		assert( driveCount == 1 );  // exact 1 drive
		assert( foundSelfCount == 1 );  // exact 1 pin from the same net
	    }
	    else if( m_pins[pinId].direction == PIN_DIRECTION_OUT )
	    {
		// find load pins
		int netId = GetNetIdByBlockAndPin( i, pinId );

		if( netId < 0 )
		    continue; // floating-port

		for( unsigned netPin=0; netPin<m_nets[netId].size(); netPin++ )
		{
		    int loadPin = m_nets[netId][netPin];
		    if( m_pins[loadPin].direction == PIN_DIRECTION_IN )
		    {
			assert( m_pins[loadPin].moduleId != (int)i );    // driving self?
			m_modules[i].m_outBlocksId.push_back( m_pins[loadPin].moduleId );
		    }
		    else if( m_pins[loadPin].direction == PIN_DIRECTION_OUT )
		    {
			assert( m_pins[loadPin].moduleId == (int)i );	// single driving
		    }
		}// for each pin connects to the net
	    }
	    else
	    {
		printf( "Pin %d of Cell %d has no direction\n", pinId, i );
	    }

	}// for each pin

	m_modules[i].m_inBlocksId.resize( m_modules[i].m_inBlocksId.size() );
	m_modules[i].m_outBlocksId.resize( m_modules[i].m_outBlocksId.size() );
    }// for each cell	
}
#endif

// void CPlaceDB::ComputeSteinerWL()
// {
//     double totalXSteinerWL = 0.0;
//     double totalMSTwire = 0.0;

//     int zeroCount = 0;

//     for( unsigned int i = 0 ; i < m_nets.size() ; i++ )
//     {
// 	if( i%1000 == 0 )
// 	    printf( "%d of %d ... (%.0f)(%.0f)\n", i, (int)m_nets.size(), totalXSteinerWL, totalMSTwire );

// 	set< pair<double, double> > setTerminals;
// 	for( unsigned int j=0; j<m_nets[i].size(); j++ )
// 	{
// 	    int pinId = m_nets[i][j];
// 	    //terminals.push_back( CPoint( m_pins[pinId].absX, m_pins[pinId].absY ) );
// 	    setTerminals.insert( make_pair( m_pins[pinId].absX, m_pins[pinId].absY ) );
// 	}

// 	set< pair<double, double> >::const_iterator ite;
// 	vector<CPoint> terminals;
// 	for( ite = setTerminals.begin(); ite != setTerminals.end(); ite++ )
// 	    terminals.push_back( CPoint( ite->first, ite->second ) );

// 	if( terminals.size() <= 1 )
// 	    continue;

// 	if( terminals.size() == 2 )
// 	{
// 	    double xDiff = fabs( terminals[0].x - terminals[1].x );
// 	    double yDiff = fabs( terminals[0].y - terminals[1].y );
// 	    double wire = sqrt( xDiff * xDiff + yDiff * yDiff );
// 	    totalMSTwire += wire;
// 	    totalXSteinerWL += CPoint::XDistance( terminals[0], terminals[1] );
// 	    continue;
// 	}

// 	CMinimumSpanningTree mst;
// 	mst.Solve( terminals );

// 	for( unsigned int j=0; j<mst.m_mst_array.size()-1; j+=2 )
// 	{
// 	    double xDiff = fabs( mst.m_mst_array[j].x - mst.m_mst_array[j+1].x );
// 	    double yDiff = fabs( mst.m_mst_array[j].y - mst.m_mst_array[j+1].y );
// 	    totalMSTwire += sqrt( xDiff * xDiff + yDiff * yDiff );
// 	}

// 	/* // debug	
// 	   printf( "%d\n", (int)mst.m_mst_array.size()/2 );
// 	   for( unsigned int j=0; j<mst.m_mst_array.size()-1; j+=2 )
// 	   printf( " %.0f %.0f   %.0f %.0f\n", 
// 	   mst.m_mst_array[j].x, mst.m_mst_array[j].y,
// 	   mst.m_mst_array[j+1].x, mst.m_mst_array[j+1].y );
// 	   */

// 	CXSteinerTree xsteiner;
// 	xsteiner.Solve( mst.m_mst_array );

// 	if( xsteiner.m_xst_array.size() == 0 )
// 	{
// 	    zeroCount++;
// 	    printf( "(%d) net [%d] mst_seg %d", zeroCount, i, mst.m_mst_array.size() );
// 	    //for( unsigned int j=0; j<terminals.size(); j++ )
// 	    //   printf( " (%.0f %.0f) ", terminals[j].x, terminals[j].y );
// 	    for( unsigned int j=0; j<mst.m_mst_array.size()-1; j+=2 )
// 		printf( " (%.0f %.0f)-(%.0f %.0f) ", 
// 			mst.m_mst_array[j].x, mst.m_mst_array[j].y,
// 			mst.m_mst_array[j+1].x, mst.m_mst_array[j+1].y );
// 	    printf( "\n" );	    

// 	    // use MST wire
// 	    for( unsigned int j=0; j<mst.m_mst_array.size()-1; j+=2 )
// 	    {
// 		double xDiff = fabs( mst.m_mst_array[j].x - mst.m_mst_array[j+1].x );
// 		double yDiff = fabs( mst.m_mst_array[j].y - mst.m_mst_array[j+1].y );
// 		totalXSteinerWL += sqrt( xDiff * xDiff + yDiff * yDiff );
// 	    }

// 	}
// 	else
// 	{
// 	    //printf( "*" );
// 	    for( unsigned int j=0; j<xsteiner.m_xst_array.size()-1; j+=2 )
// 	    {
// 		double xDiff = fabs( xsteiner.m_xst_array[j].x - xsteiner.m_xst_array[j+1].x );
// 		double yDiff = fabs( xsteiner.m_xst_array[j].y - xsteiner.m_xst_array[j+1].y );
// 		totalXSteinerWL += sqrt( xDiff * xDiff + yDiff * yDiff );
// 	    }
// 	}
//     }
//     printf( "Total XST WL = %.0f\n", totalXSteinerWL );
//     printf( "Total MST WL = %.0f\n", totalMSTwire );
// }


// double CPlaceDB::GetSteinerWL( const Net& curNet, int steinerType/*=0*/, int routingType/*=0*/ )
// {
//     // steinerType 0    FLUTE
//     // steinerType 1    Minimum spanning tree (MST)

//     // routingType 0	Manhanttan
//     // routingType 1    X

//     if( curNet.size() <= 1 )
// 	return 0;

//     //if( curNet.size() >= 1001 )
//     if( curNet.size() > MAXD )
// 	steinerType = 1; // FLUTE bug?

//     if( 2 == curNet.size() )
//     {
// 	double x, y;
// 	GetPinLocation( curNet[0], x, y );
// 	CPoint p1(x,y);
// 	GetPinLocation( curNet[1], x, y );
// 	CPoint p2(x,y);
// 	if( routingType == 1 )
// 	    return CPoint::XDistance( p1, p2 );
// 	return CPoint::Distance( p1, p2 );
//     }

//     // curNet.size() > 2 

//     // Construct the steiner tree 
//     vector<CPoint> points;
//     for( unsigned int j = 0 ; j < curNet.size() ; j++ )
//     {
// 	double x, y;
// 	GetPinLocation( curNet[j], x, y );
// 	points.push_back( CPoint( x, y ) );
//     }

//     double wire = 0;
//     if( steinerType == 1 )
//     {
// 	CMinimumSpanningTree mst;
// 	mst.Solve( points );
// 	assert( mst.m_mst_array.size() % 2 == 0 );
// 	for( unsigned int j = 0 ; j < mst.m_mst_array.size() ; j=j+2 )
// 	{
// 	    if( routingType == 1 )
// 		wire += CPoint::XDistance( mst.m_mst_array[j], mst.m_mst_array[j+1] );
// 	    else
// 		wire += CPoint::Distance( mst.m_mst_array[j], mst.m_mst_array[j+1] );
// 	}
//     }
//     else
//     {
// 	if( pgFlute == NULL )
// 	{
// 	    printf( "Create FLUTE\n" );
// 	    fflush( stdout );
// 	    pgFlute = new CFlute();
// 	}
// 	pgFlute->Solve( points );
// 	assert( pgFlute->m_steiner_array.size() % 2 == 0 );
// 	for( unsigned int j=0; j<pgFlute->m_steiner_array.size(); j=j+2 )
// 	{
// 	    if( routingType == 1 )
// 		wire += CPoint::XDistance( pgFlute->m_steiner_array[j], pgFlute->m_steiner_array[j+1] );
// 	    else
// 		wire += CPoint::Distance( pgFlute->m_steiner_array[j], pgFlute->m_steiner_array[j+1] );
// 	}
//     }
//     return wire;
// }

// double CPlaceDB::GetTotalSteinerWL( const int& steinerType, const int& routingType )
// {
//     double wire = 0;
//     for( unsigned int i=0; i<m_nets.size(); i++ )
//     {
// 	if( i != 0 && i % 200000 == 0 )
// 	{
// 	    printf( "#%d...\n", i );
// 	    fflush( stdout );
// 	}
// 	wire += GetSteinerWL( m_nets[i], steinerType, routingType );
//     }
//     return wire;
// }


// void CPlaceDB::CreateCongMap( const int& hdim, const int& vdim )
// {
//     //TODO: need a better method to create and delete the congestion map
//     if( m_pCongMap == NULL )
//     {
// 	printf( "[DB] Create CongMap\n" );
// 	m_pCongMap = new CCongMap( hdim, vdim, m_coreRgn, m_pLEF );
//     }
//     else
//     {
// 	delete m_pCongMap;
// 	m_pCongMap = new CCongMap( hdim, vdim, m_coreRgn, m_pLEF );
//     }

//     CGlobalRouting groute( *this );

//     groute.ProbalisticMethod( *m_pCongMap );
//     //groute.FastRoute( *m_pCongMap );

//     //OutputCongMapFigure();
//     //m_pCongMap->OutputBinOverflowFigure( "Overflow" );

//     //delete m_pCongMap;
// }

void CPlaceDB::ClearCongMap( void )
{
    delete m_pCongMap;
    m_pCongMap = NULL;
}

void CPlaceDB::UpdateModuleNetPinId()
{
    // update m_moduleNetPinId;

    m_moduleNetPinId.resize( m_modules.size() );
    for( unsigned int i=0; i<m_modules.size(); i++ )
    {
	m_moduleNetPinId[i].resize( m_modules[i].m_netsId.size(), -1 );
	for( unsigned int j=0; j<m_modules[i].m_netsId.size(); j++ )
	{
	    int netId = m_modules[i].m_netsId[j];
	    int pinId = -1;
	    for( unsigned int p=0; p<m_nets[netId].size(); p++ )
	    {
		if( m_pins[ m_nets[netId][p] ].moduleId == (int)i )
		{
		    pinId = m_nets[netId][p];
		    break;
		}
	    }
	    assert( pinId != -1 );  // net without pin?
	    m_moduleNetPinId[i][j] = pinId;
	} // each net to the module
    } // each module
}

void CPlaceDB::UpdatePinNetId()
{
    // update m_pinNetId;
    m_pinNetId.resize( m_pins.size() );
    for( unsigned int i=0; i<m_pins.size(); i++ )
    {
	int modId = m_pins[i].moduleId;
	bool found = false;
	for( unsigned int n=0; n<m_modules[modId].m_netsId.size(); n++ )
	{
	    int netId = m_modules[modId].m_netsId[n];

	    for( unsigned int p=0; p<m_nets[netId].size(); p++ )
	    {
		if( m_pins[ m_nets[netId][p] ].moduleId == modId )
		{
		    m_pinNetId[i] = netId;
		    found = true;
		}
	    }
	}// for each net connected to the module
	assert( found == true );
    }
}

