#include "FixedPointMethod.h"

CFixedPointMethod::CFixedPointMethod(CPlaceDB& placedb) :
    m_placedb( placedb )
{
    fprintf( stdout, "m_placedb: %X\n", &m_placedb );
    fflush( stdout );
}

void CFixedPointMethod::AddAllPseudoModuleAndNet( void )
{
    fprintf( stdout, "m_placedb: %X\n", &m_placedb );
    fflush( stdout );
    fprintf( stdout, "m_placedb.m_modules: %X\n", &(m_placedb.m_modules) );
    fflush( stdout );
    fprintf( stdout, "m_placedb.m_modules.size(): %X\n", m_placedb.m_modules.size() );
    fflush( stdout );
    fprintf( stdout, "m_orig_modules: %X\n", &m_orig_modules );
    fflush( stdout );
    fprintf( stdout, "m_orig_modules.size(): %d\n", m_orig_modules.size() );
    fflush( stdout );
    fprintf( stdout, "&m_orig_modules: %X\n", &m_orig_modules );
    fflush( stdout );
    //m_orig_modules = m_placedb.m_modules;
    m_orig_modules.reserve( m_placedb.m_modules.size() );
    fprintf( stdout, "&m_orig_modules: %X\n", &m_orig_modules );
    fflush( stdout );
    fprintf( stdout, "abc\n" );
    fflush( stdout );
    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	fprintf( stdout, "%d\n", i );
	fflush( stdout );
	m_orig_modules.push_back( m_placedb.m_modules[i] );
    }
    fprintf( stdout, "abc\n" );
    fflush( stdout );
    m_orig_nets = m_placedb.m_nets;
    fprintf( stdout, "abc\n" );
    fflush( stdout );
    m_orig_pins = m_placedb.m_pins;

#if 1
    //test code
    fprintf( stdout, "Before add pseudo modules: %d\n", m_placedb.m_modules.size() );
    fflush( stdout );
    //@test code
#endif
    
    //Add a pseudo module and pseudo net for each module
    for( unsigned int i = 0 ; i < m_orig_modules.size() ; i++ )
    {
	fprintf( stdout, "abc: %d\n", i );
	fflush( stdout );
	Module& curModule = m_placedb.m_modules[i];
	
	CPoint new_pin_location( curModule.m_cx, curModule.m_cy );
	int new_pin_id1 = AddPseudoPinOnModule( i, new_pin_location );
	int new_pin_id2 = AddPseudoPinAndModule( new_pin_location );
	AddPseudoNet( new_pin_id1, new_pin_id2 );
	
#if 0
	//test code
	fprintf( stdout, "%d %d\n", new_pin_id1, new_pin_id2 );
	fflush( stdout );
	//@test code
#endif
    }
#if 0 
    //test code
    fprintf( stdout, "After add pseudo modules: %d\n", m_placedb.m_modules.size() );
    fflush( stdout );
    //@test code
#endif    
}

void CFixedPointMethod::RestoreAllPseudoModuleAndNet( void )
{
#if 0
    m_placedb.m_modules = m_orig_modules;
    m_placedb.m_nets = m_orig_nets;
    m_placedb.m_pins = m_orig_pins;
#endif
    fprintf( stdout, "RestoreAllPseudoMdouleAndNet\n" );
    fflush( stdout );

    fprintf( stdout, "m_orig_modules.size(): %d\n",  m_orig_modules.size() );
    fflush( stdout );
    m_placedb.m_modules.resize( m_orig_modules.size() );
    for( unsigned int i = 0 ; i < m_orig_modules.size() ; i++ )
    {
	m_placedb.m_modules[i].m_pinsId.resize( m_orig_modules[i].m_pinsId.size() );
	fprintf( stdout, "i: %d\n", i );
	fflush( stdout );
	fprintf( stdout, "m_orig_modules[i].m_pinsId.size(): %d\n", 
		m_orig_modules[i].m_pinsId.size() );
	fflush( stdout );
	m_placedb.m_modules[i].m_netsId = m_orig_modules[i].m_netsId;
    }
    
    m_placedb.m_pins.resize( m_orig_pins.size() );
    fprintf( stdout, "m_orig_pins.size(): %d\n",  m_orig_pins.size() );
    fflush( stdout );
    m_placedb.m_nets.resize( m_orig_nets.size() );
    fprintf( stdout, "m_orig_nets.size(): %d\n", m_orig_nets.size() );
    fflush( stdout );
}

int CFixedPointMethod::AddPseudoPinAndModule( const CPoint& p )
{
    //m_isDummy in module?
    int moduleId = m_placedb.m_modules.size();
    int pinId = m_placedb.m_pins.size();

    //Add module
    m_placedb.m_modules.push_back( Module( "pseudo-module", 0.0, 0.0, false ) );
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

void CFixedPointMethod::AddPseudoNet( const int& i1, const int& i2 )
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
		                                
int CFixedPointMethod::AddPseudoPinOnModule( const int& moduleId, const CPoint& p )
{
    int pinId = m_placedb.m_pins.size();
    const Module& curModule = m_placedb.m_modules[ moduleId ];

    //maintain Module::m_pinsId
    m_placedb.m_modules[ moduleId ].m_pinsId.push_back( pinId );

    //Add pin
    m_placedb.m_pins.push_back( Pin( p.x - curModule.m_cx, p.y - curModule.m_cy ) );
    m_placedb.m_pins[ pinId ].moduleId = moduleId;
    m_placedb.m_pins[ pinId ].absX = p.x;
    m_placedb.m_pins[ pinId ].absY = p.y;

    return pinId;
}
		                            
