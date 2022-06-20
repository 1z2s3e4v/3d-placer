/// 2005-12-10 donnie
//
//  Output Bookshelf format for NTUplace



#include "placedb.h"

#include <string>
using namespace std;

void CPlaceDB::OutputBookshelf( const char* prefix, bool setOutOrientN )
{
    cout << "Output Bookshelf format\n";

    string pre( prefix );

    OutputAUX( (pre + ".aux").c_str(), prefix );
    OutputNodes( (pre + ".nodes").c_str(), setOutOrientN );
    OutputWTS( (pre + ".wts").c_str() );
    OutputNets( (pre + ".nets").c_str(), setOutOrientN );
    OutputSCL( (pre + ".scl").c_str() );
    OutputPL( (pre + ".pl").c_str(), setOutOrientN );
    
}

void CPlaceDB::OutputAUX( const char* filename, const char* prefix )
{
    cout << "   " << filename << endl;

    ofstream out( filename );
    if( !out )
    {
	cerr << "Cannot open output file\n";
	return;
    }
    
    out << "RowBasedPlacement : " 
	<< prefix << ".nodes " 
	<< prefix << ".nets " 
	<< prefix << ".wts "
	<< prefix << ".pl " 
	<< prefix << ".scl \n\n";

}

void CPlaceDB::OutputNets( const char* filename, bool setOutOrientN )
{
    cout << "   " << filename << endl;

    FILE* out;
    out = fopen( filename, "w" );
    if( !out )
    {
	cerr << "Cannot open output file\n";
	return;
    }
   
    fprintf( out, "UCLA nets 1.0\n\n" );
    fprintf( out, "NumNets : %d\n", m_nets.size() );
    fprintf( out, "NumPins : %d\n", GetUsedPinNum() );
    for( int i=0; i<(int)m_nets.size(); i++ )
    {
	fprintf( out, "NetDegree : %d\n", m_nets[i].size() );
	for( int j=0; j<(int)m_nets[i].size(); j++ )
	{
	    int pinId = m_nets[i][j];
	    int moduleId = m_pins[ pinId ].moduleId;

	    int oldOrient = m_modules[moduleId].m_orient;
	    if( !setOutOrientN )
		SetModuleOrientation( moduleId, 0 );    // rotate back to "N"
	    
            fprintf( out, " %10s B : %.0f %.0f\n", 
		    m_modules[ moduleId ].GetName().c_str(),
		    m_pins[pinId].xOff,
		    m_pins[pinId].yOff );  

	    if( !setOutOrientN )
		SetModuleOrientation( moduleId, oldOrient );

	    
	}
    }
    fprintf( out, "\n" );
    fclose( out );
}

void CPlaceDB::OutputWTS( const char* filename )
{
    cout << "   " << filename << endl;
    
    FILE* out;
    out = fopen( filename, "w" );
    if( !out )
    {
	cerr << "Cannot open output file\n";
	return;
    }
    
    fprintf( out, "UCLA wts 1.0\n\n" );

    // pad
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( ModuleInCore( i ) )
	    continue;	    
	fprintf( out, " %30s           0\n", 
		m_modules[i].GetName().c_str() );
    }

    // non-pad 
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( ! ModuleInCore( i ) )
	    continue;
	fprintf( out, " %30s %10.0f \n",
		m_modules[i].GetName().c_str(),
		m_modules[i].GetWidth() * m_modules[i].GetHeight() );
    }
    fprintf( out, "\n\n" );
    fclose( out ); 
}

void CPlaceDB::OutputNodes( const char* filename, bool setOutOrientN )
{
    cout << "Output Nodes file:" << filename << endl;
    
    FILE* out;
    out = fopen( filename, "w" );
    if( !out )
    {
	cerr << "Cannot open output file\n";
	return;
    }
    
    fprintf( out, "UCLA nodes 1.0\n\n" );
    fprintf( out, "NumNodes : %d\n", m_modules.size() );
    fprintf( out, "NumTerminals : %d\n\n", CalculateFixedModuleNumber() );

    // non-terminal
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( m_modules[i].m_isFixed )
	    continue;	    
	double w = m_modules[i].GetWidth();
	double h = m_modules[i].GetHeight();
	
	if( !setOutOrientN && m_modules[i].isRotated() )
	    swap( w, h );
	
	fprintf( out, " %30s %10.0f %10.0f\n", 
		m_modules[i].GetName().c_str(),  
		w,
		h );
    }

    // terminal 
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( !m_modules[i].m_isFixed )
	    continue;
	double w = m_modules[i].GetWidth();
	double h = m_modules[i].GetHeight();
	
	if( !setOutOrientN && m_modules[i].isRotated() )
	    swap( w, h );
	
	fprintf( out, " %10s %10.0f %10.0f terminal\n",
		m_modules[i].GetName().c_str(),
		w,
		h );
    }
    fprintf( out, "\n\n" );
    fclose( out ); 
}

void CPlaceDB::OutputSCL( const char* filename )
{
    
    cout << "   " << filename << endl;
    
    FILE* out;
    out = fopen( filename, "w" );
    if( !out )
    {
	cerr << "Cannot open output file\n";
	return;
    }
    
    fprintf( out, "UCLA scl 1.0\n" );
    fprintf( out, "# Created       :\n" );
    fprintf( out, "# User          :\n\n" );
    fprintf( out, "NumRows : %d\n\n", m_sites.size() );

    char* ori[2] = { "N", "Y" };
    for( int i=0; i<(int)m_sites.size(); i++ )
    {
	double step = m_sites[i].m_step;
	if( step == 0 )
	    step = 1.0;
	fprintf( out, "CoreRow Horizontal\n" );
	fprintf( out, " Coordinate    : %8.0f\n", m_sites[i].m_bottom );
	fprintf( out, " Height        : %8.0f\n", m_sites[i].m_height );
	fprintf( out, " Sitewidth     : %8.0f\n", step );
	fprintf( out, " Sitespacing   : %8.0f\n", step );
	fprintf( out, " Siteorient    : 1\n" );//%s\n", ori[i % 2] );
	fprintf( out, " Sitesymmetry  : 1\n" );
	for( int j=0; j<(int)m_sites[i].m_interval.size(); j+=2 )
	{
	    assert( (j+1) < (int)m_sites[i].m_interval.size() );
	    fprintf( out, " SubrowOrigin  : %8.0f Numsites : %8.0f\n", 
		    m_sites[i].m_interval[j], 
		    (m_sites[i].m_interval[j+1]-m_sites[i].m_interval[j])/step );
	}
	fprintf( out, "End\n" );
    }
    fprintf( out, "\n" );
    fclose( out ); 
}


