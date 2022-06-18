#include "SteinerDecomposition.h"

//#include "MinimumSpanningTree.h"
//#include "steiner.h"

using namespace std;
using namespace Jin;


CStoredModuleData::CStoredModuleData( const Module& m ) :
    m_nPinsId( m.m_pinsId.size() ),
    m_netsId( m.m_netsId )
{
}

CSteinerDecomposition::CSteinerDecomposition( CPlaceDB& placedb ) :
    m_placedb( placedb ),
    //Assume the prior "m_orig_nModules" modules must be the original ones
    //Note that the Module::m_pinsId and Module::m_netsId data must be modified
    m_orig_nModules( placedb.m_modules.size() ),
    //The prior "m_orig_nPins" pins must be the original ones
    m_orig_nPins( placedb.m_pins.size() ),
    m_orig_nets( placedb.m_nets )
{
    //Record the necessary data (m_nPinsId, m_netsId) 
    for( int i = 0 ; i < m_orig_nModules ; i++ )
    {
	m_module_data.push_back( CStoredModuleData( m_placedb.m_modules[i] ) );
    }

#if 1
    //check the pin degree must > 0 
    vector<int> pinDegree;
    pinDegree.resize( m_placedb.m_pins.size(), 0 );
    for( unsigned int iNet = 0 ; iNet < m_placedb.m_nets.size() ; iNet++ )
    {
	//fprintf( stderr, "net %6d: ", iNet );
	for( unsigned int iPin = 0 ; iPin < m_placedb.m_nets[iNet].size() ; iPin++ )
	{
	    int pinId = m_placedb.m_nets[iNet][iPin];
	    pinDegree[pinId]++;
	    //fprintf( stderr, "%d ", pinId );
	}
	//fprintf( stderr, "\n" );
    }
    
    //Check the pin degree must < 2
    for( unsigned int i = 0 ; i < pinDegree.size() ; i++ )
    {
	if( pinDegree[i] > 1 )
	{
	    fprintf( stderr, "Warning: pin %d has wrong degree %d(1)\n", i, pinDegree[i] );
	}
    }
    //@test code
#endif
}

void CSteinerDecomposition::Restore(void)
{
    m_placedb.m_modules.resize( m_orig_nModules );
    for( int i = 0 ; i < m_orig_nModules ; i++ )
    {
	m_placedb.m_modules[i].m_pinsId.resize( m_module_data[i].m_nPinsId );
	m_placedb.m_modules[i].m_netsId = m_module_data[i].m_netsId;
    }

    m_placedb.m_pins.resize( m_orig_nPins );
    m_placedb.m_nets = m_orig_nets;
}

namespace Jin
{
//CPoint predicator for mapPointToPinIndex
class CPointPredicator
{
public:
    bool operator()( const CPoint& p1, const CPoint& p2 )
    {
	if( p1.x == p2.x )
	    return p1.y < p2.y;
	else
	    return p1.x < p2.x;
    }
};
}

//Define this to 1 when treat Steiner "pins" as exactly one pin
#define _MERGEPIN_ 0

void CSteinerDecomposition::Update(void)
{
    //Prepare data
    //1. Remove all pseudo-modules
    m_placedb.m_modules.resize( m_orig_nModules );
    //2. Remove all nets and pseudo-pins in each module
    for( int i = 0 ; i < m_orig_nModules ; i++ )
    {
	 m_placedb.m_modules[i].m_pinsId.resize( m_module_data[i].m_nPinsId );
	 m_placedb.m_modules[i].m_netsId.clear();
    }
    //3. Remove all nets
    m_placedb.m_nets.clear();
    //4. Remove all pseudo-pins
    m_placedb.m_pins.resize( m_orig_nPins );
    
    if( !pgFlute )
    {
	pgFlute = new CFlute;
    }
    
    
    //Decomposite each net
    for( unsigned int orig_net_id = 0 ; orig_net_id < m_orig_nets.size() ; orig_net_id++ )
    {
	const Net& curNet = m_orig_nets[orig_net_id];
	
	//Skip net with # of terminal < 2
	if( curNet.size() < 2 )
	{
	    continue;
	}
	//test code
	//fprintf( stderr, "Net size: %d\n", curNet.size() );
	//fflush( stderr );
	//@test code
	
	//point-to-pin-index map
	map<CPoint, int, CPointPredicator > mapPointToPinIndex;

	//If # of terminals > MAXD,
	//decomposition by mst
	if( curNet.size() > MAXD )
	{
	    vector<CPoint> points;
	    points.reserve( curNet.size() );
	    for( unsigned int i = 0 ; i < curNet.size() ; i++ )
	    {
		int pinId = curNet[i];
		double x, y;
		m_placedb.GetPinLocation( pinId, x, y );

		CPoint pinLocation( x, y );
		points.push_back( pinLocation );
		
		mapPointToPinIndex[ pinLocation ] = pinId;
	    }

	    CMinimumSpanningTree mst;
	    mst.Solve( points );	    

#if !_MERGEPIN_
	    set<int> addedPinIds;
#endif
	    
	    for( unsigned int i = 0 ; i < mst.m_mst_array.size() ; i=i+2 )
	    {
		CPoint p[2];
		p[0] = mst.m_mst_array[i];
		p[1] = mst.m_mst_array[i+1];	

		int pIndex[2];
		pIndex[0] = 0;
		pIndex[1] = 1;
		map<CPoint, int, CPointPredicator >::iterator iteFind;
#if _MERGEPIN_		
		for( int j = 0 ; j < 2 ; j++ )
		{
		    iteFind = mapPointToPinIndex.find( p[j] );

		    //point not found -> add a pseudo-pin and a pseudo-module
		    if( iteFind == mapPointToPinIndex.end() )
		    {
			fprintf( stderr, 
				"Warning: pin (%.2f,%.2f) not found in mapPointToPinIndex\n",
				p[j].x, p[j].y );
		    }
		    else
		    {
			pIndex[j] = iteFind->second;
		    }

		}	    
#else
		for( int j = 0 ; j < 2 ; j++ )
		{
		    iteFind = mapPointToPinIndex.find( p[j] );

		    //point not found -> add a pseudo-pin and a pseudo-module
		    if( iteFind == mapPointToPinIndex.end() )
		    {
			fprintf( stderr, 
				"Warning: pin (%.2f,%.2f) not found in mapPointToPinIndex\n",
				p[j].x, p[j].y );
		    }
		    else
		    {
			set<int>::iterator iteAdded = addedPinIds.find( iteFind->second );
			
			//The pin is connected to this multi-terminal net already
			//Add a pseudo pin to keep the degree of each pin = 1
			if( iteAdded != addedPinIds.end() )
			{
			    int moduleId = m_placedb.m_pins[ iteFind->second ].moduleId;
			    pIndex[j] = AddPseudoPinOnModule( moduleId, p[j] );

			    //test code
			    //fprintf( stderr, "Add pseudo pin %d\n", pIndex[j] );
			    //@test code
			}
			else
			{
			    //test code
			    //fprintf( stderr, "Pin %d not found\n", iteFind->second );
			    //@test code
			    pIndex[j] = iteFind->second;
			    addedPinIds.insert( iteFind->second );
			}
		    }

		}	    
		
#endif
		AddPseudoNet( pIndex[0], pIndex[1] );
				
	    }
	    
	    continue;
	}
	

	//Decomposite by Flute	
	//Add integral points into mapPointToPinIndex
	for( unsigned int i = 0 ; i < curNet.size() ; i++ )
	{
	    int pinIndex = curNet[i];
	    CPoint pinPosition; 
	    m_placedb.GetPinLocation( pinIndex, pinPosition.x, pinPosition.y );
	    pinPosition.x = static_cast<int>( pinPosition.x );
	    pinPosition.y = static_cast<int>( pinPosition.y );
	
	    map<CPoint, int, CPointPredicator >::iterator iteFind = mapPointToPinIndex.find( pinPosition );
	    
	    //If pinPosition is already in mapPointToPinIndex, 
	    //add a pseudo-net between the existing pin and the current pin
	    if( iteFind != mapPointToPinIndex.end() )
	    {
#if _MERGEPIN_
		AddPseudoNet( iteFind->second, pinIndex );
#else
		const Pin& pin1 = m_placedb.m_pins[ iteFind->second ];
		int pseudoPinId1 = AddPseudoPinOnModule( pin1.moduleId, CPoint( pin1.absX, pin1.absY ) );
		const Pin& pin2 = m_placedb.m_pins[ pinIndex ];
		int pseudoPinId2 = AddPseudoPinOnModule( pin2.moduleId, CPoint( pin2.absX, pin2.absY ) );

		AddPseudoNet( pseudoPinId1, pseudoPinId2 );
#endif
	    }
	    //If pinPosition is not in mapPointToPinIndex,
	    //add it into mapPointToPinIndex
	    else
	    {
		mapPointToPinIndex[ pinPosition ] = pinIndex;
	    }

	}//@Add integral points into mapPointToPinIndex	    

	//Skip solving the net with # of terminals < 2
	if( mapPointToPinIndex.size() < 2 )
	{
	    continue;
	}
	
	//Solve by flute	
	vector<CPoint> points;
	points.reserve( mapPointToPinIndex.size() );
	for( map<CPoint, int, CPointPredicator >::iterator ite = mapPointToPinIndex.begin() ;
		ite != mapPointToPinIndex.end() ; ite++ )
	{
	    points.push_back( ite->first );	
	}
	
	
	//test code
	//fprintf(stderr,"points.size(): %d\n", points.size() );
	//fflush( stderr );
	//@test code
	
	pgFlute->Solve( points );

#if !_MERGEPIN_
	//Record the added pins to generate new pins in the same location
	set<int> addedPinIds;
#endif

	//Add pseudo-pins and pseudo-modules for Steiner points	
	for( unsigned int i = 0 ; i < pgFlute->m_steiner_array.size() ; i=i+2 )
	{
	    CPoint p[2];
	    p[0] = pgFlute->m_steiner_array[i];
	    p[1] = pgFlute->m_steiner_array[i+1];	

	    int pIndex[2];
	    map<CPoint, int, CPointPredicator >::iterator iteFind;

#if _MERGEPIN_
	    //This segment of code considers pins with same coordinate as one pin,
	    //so one pin may connect to multiple nets

	    for( int j = 0 ; j < 2 ; j++ )
	    {
		iteFind = mapPointToPinIndex.find( p[j] );

		
		//point not found -> add a pseudo-pin and a pseudo-module
		if( iteFind == mapPointToPinIndex.end() )
		{
		    pIndex[j] = AddPseudoPinAndModule( p[j] );
		    mapPointToPinIndex[ p[j] ] = pIndex[j];
		}
		else
		{
		    pIndex[j] = iteFind->second;
		}
		
	    }	    
	    //@This segment of code considers pins with same coordinate as one pin,
	    //@so one pin may connect to multiple nets
#else
	    //This segment distinguish pins with same coordinate as different ones
	    for( int j = 0 ; j < 2 ; j++ )
	    {
		iteFind = mapPointToPinIndex.find( p[j] );

		//point not found -> add a pseudo-pin and a pseudo-module
		if( iteFind == mapPointToPinIndex.end() )
		{
		    pIndex[j] = AddPseudoPinAndModule( p[j] );
		    mapPointToPinIndex[ p[j] ] = pIndex[j];
		}
		else
		{
		    //This point locates on a pseudo pin
		    //Add another pseudo pin on the same module
		    if( iteFind->second >= m_orig_nPins )
		    {
			int moduleId = m_placedb.m_pins[ iteFind->second ].moduleId;
			pIndex[j] = AddPseudoPinOnModule( moduleId, p[j] );
		    }
		    //This point locates on a real pin
		    //Connect the net to the real pin 
		    else
		    {
			set<int>::iterator iteAdded = addedPinIds.find( iteFind->second );
			
			//The pin is connected to this multi-terminal net already
			//Add a pseudo pin to keep the degree of each pin = 1
			if( iteAdded != addedPinIds.end() )
			{
			    int moduleId = m_placedb.m_pins[ iteFind->second ].moduleId;
			    pIndex[j] = AddPseudoPinOnModule( moduleId, p[j] );

			    //test code
			    //fprintf( stderr, "Add pseudo pin %d\n", pIndex[j] );
			    //@test code
			}
			else
			{
			    //test code
			    //fprintf( stderr, "Pin %d not found\n", iteFind->second );
			    //@test code
			    pIndex[j] = iteFind->second;
			    addedPinIds.insert( iteFind->second );
			}
		    }
		}
		
	    }	    
#endif
	    AddPseudoNet( pIndex[0], pIndex[1] );

	    
	    //@This segment distinguish pins with same coordinate as different ones

	    //test code
	    //fprintf( stderr, "PseudoNet: %d %d\n", pIndex[0], pIndex[1] );
	    //@test code
	}//@Add pseudo-pins and pseudo-modules for Steiner points

    }
    //@Decomposite each net
	
#if 1 
    //test code
    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	for( unsigned int j = 0 ; j < m_placedb.m_modules[i].m_pinsId.size() ; j++ )
	{
	    if( static_cast<int>(i) != m_placedb.m_pins[ m_placedb.m_modules[i].m_pinsId[j] ].moduleId )
	    {
		fprintf( stderr, "Warning: Incorrect pin info %d != m_placedb.m_pins[m_placedb.m_modules[%d].m_pinsId[%d]].moduleId (%d)\n",
			i, i, j, m_placedb.m_pins[ m_placedb.m_modules[i].m_pinsId[j] ].moduleId );
	    }
	}
    }
    //check the pin degree must > 0 
    vector<int> pinDegree;
    pinDegree.resize( m_placedb.m_pins.size(), 0 );
    for( unsigned int iNet = 0 ; iNet < m_placedb.m_nets.size() ; iNet++ )
    {
	//fprintf( stderr, "net %6d: ", iNet );
	for( unsigned int iPin = 0 ; iPin < m_placedb.m_nets[iNet].size() ; iPin++ )
	{
	    int pinId = m_placedb.m_nets[iNet][iPin];
	    pinDegree[pinId]++;
	    //fprintf( stderr, "%d ", pinId );
	}
	//fprintf( stderr, "\n" );
    }
#if 0
    for( unsigned int i = 0 ; i < pinDegree.size() ; i++ )
    {
	if( ( ( i >= m_orig_nPins) && ( pinDegree[i] < 3 ) ) ||
	    (pinDegree[i] < 1 ) )
	{
	    fprintf( stderr, "Warning: pin %d has wrong degree %d\n", i, pinDegree[i] );
	}
    }
#endif
    //Check the pin degree must < 2
    for( unsigned int i = 0 ; i < pinDegree.size() ; i++ )
    {
	if( pinDegree[i] > 1 )
	{
	    fprintf( stderr, "Warning: pin %d has wrong degree %d(2)\n", i, pinDegree[i] );
	}
    }
    //@test code
#endif
}

int CSteinerDecomposition::AddPseudoPinAndModule( const CPoint& p )
{
    //m_isDummy in module?
    int moduleId = m_placedb.m_modules.size();
    int pinId = m_placedb.m_pins.size();

    //Add module
    //fix the pseudo-pin
    //m_placedb.m_modules.push_back( Module( "pseudo-module", 0.0, 0.0, false ) );    // movable cell
    m_placedb.m_modules.push_back( Module( "pseudo-module", 0.0, 0.0, true ) );       // fixed cell
    //m_placedb.m_modules.push_back( Module( "pseudo-module", 0.001, 0.001, false ) );
    
    m_placedb.SetModuleLocation( moduleId, p.x, p.y );
    m_placedb.m_modules[ moduleId ].m_pinsId.push_back( pinId );
    m_placedb.m_modules[ moduleId ].m_isCluster = false;

    //Add pin
    m_placedb.m_pins.push_back( Pin( 0.0, 0.0 ) );
    m_placedb.m_pins[ pinId ].moduleId = moduleId;
    m_placedb.CalcPinLocation( pinId );

    return pinId;    
}

int CSteinerDecomposition::AddPseudoPinOnModule( const int& moduleId, const CPoint& p )
{
    int pinId = m_placedb.m_pins.size();
    const Module& curModule = m_placedb.m_modules[ moduleId ];
    
    //maintain Module::m_pinsId
    m_placedb.m_modules[ moduleId ].m_pinsId.push_back( pinId );

    //Add pin
    Pin new_pin( p.x - curModule.m_cx, p.y - curModule.m_cy );
    new_pin.moduleId = moduleId;
    new_pin.absX = p.x;
    new_pin.absY = p.y;
    
    m_placedb.m_pins.push_back( new_pin );
    
#if 0
    m_placedb.m_pins.push_back( Pin( p.x - curModule.m_cx, p.y - curModule.m_cy ) );
    m_placedb.m_pins[ pinId ].moduleId = moduleId;
    m_placedb.m_pins[ pinId ].absX = p.x;
    m_placedb.m_pins[ pinId ].absY = p.y;
#endif

    return pinId;
}

void CSteinerDecomposition::AddPseudoNet( const int& i1, const int& i2 )
{
    if( i1 == i2 )
    {
	fprintf( stderr, "Warning: Add a pseudo net for the same pin %d\n", i1 );
	return;
    }

    if( m_placedb.m_pins[ i1 ].moduleId == m_placedb.m_pins[ i2 ].moduleId )
    {
	return;
    }
    
    int netId = m_placedb.m_nets.size();

    //Add pseudo-net
    m_placedb.m_nets.push_back( Net() );
    m_placedb.m_nets[ netId ].push_back( i1 );
    m_placedb.m_nets[ netId ].push_back( i2 );

    //Update module info
    m_placedb.m_modules[ m_placedb.m_pins[ i1 ].moduleId ].m_netsId.push_back( netId );
    m_placedb.m_modules[ m_placedb.m_pins[ i2 ].moduleId ].m_netsId.push_back( netId );
    
    //return netId;
}
