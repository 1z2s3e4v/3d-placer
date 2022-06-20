#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <malloc.h>
#include <cmath>
#include <cctype>
#include <cstring>
#include <climits>
using namespace std;

#include "lefdef.h"

int userData;
FILE* fout;

void dataError() {
          fprintf(fout, "ERROR: returned user data is not correct!\n");
}

void* mallocCB(int size) {
          return malloc(size);
}

void* reallocCB(void* name, int size) {
          return realloc(name, size);
}

void freeCB(void* name) {
      free(name);
        return;
}

void lineNumberCB(int lineNo) {
      fprintf(fout, "Parsed %d number of lines!!\n", lineNo);
        return;
}

char* orientStr(int orient) {
    switch (orient) {
	case 0: return ((char*)"N");
	case 1: return ((char*)"W");
	case 2: return ((char*)"S");
	case 3: return ((char*)"E");
	case 4: return ((char*)"FN");
	case 5: return ((char*)"FW");
	case 6: return ((char*)"FS");
	case 7: return ((char*)"FE");
    };
    return ((char*)"BOGUS");
}

int orientInt(char* c)
{
    assert( strlen(c) <= 2 );
    if( c[0] == 'F' )
    {
	switch( c[1] )
	{
	    case 'N': return 4;
	    case 'W': return 5;
	    case 'S': return 6;
	    case 'E': return 7;
	}
    }
    else
    {
	switch( c[0] )
	{
	    case 'N': return 0;
	    case 'W': return 1;
	    case 'S': return 2;
	    case 'E': return 3;
	}
    }
    return 0;	// Use "N" if any problem occurs.
}


/////////////////////////////////////////////////////////////////////


CParserLEFDEF::CParserLEFDEF()
: m_minBlockHeight( INT_MAX ),
  m_coreSiteWidth( -1 ),
  m_coreSiteHeight( -1 )
{
}

void CParserLEFDEF::PrintMacros( bool showPins )
{
    printf( "LEF macro # = %d\n", m_modules.size() );
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	printf( "   %-10s w = %-5g h = %-5g pin # = %d\n", 
		m_modules[i].GetName().c_str(), 
		m_modules[i].GetWidth(), 
		m_modules[i].GetHeight(),
	        m_modules[i].m_pinsId.size() );
	if( showPins == true )
	{
	    for( int j=0; j<(int)m_modules[i].m_pinsId.size(); j++ )
	    {
		int pinId = m_modules[i].m_pinsId[j];
		printf( "      %-7s x = %-8g y = %-8g\n", 
			m_pins[pinId].pinName.c_str(), m_pins[pinId].xOff, m_pins[pinId].yOff );
		assert( fabs(m_pins[pinId].xOff) <= (m_modules[i].GetWidth()*0.5) );
		assert( fabs(m_pins[pinId].yOff) <= (m_modules[i].GetHeight()*0.5) );
	    }
	}
	
    }
    printf( "Core Site w = %g h= %g\n", m_coreSiteWidth, m_coreSiteHeight );
}

void CParserLEFDEF::CreateModuleNameMap()
{
    printf( "Create LEF macro map\n" );
    m_moduleNameMap.clear();
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	m_moduleNameMap[  m_modules[i].GetName() ] = i;
    }
    printf( "   Map size = %d\n", m_moduleNameMap.size() );
}

int CParserLEFDEF::AddPin( int moduleId, string pinName, float xOff, float yOff )
{
    m_pins.push_back( Pin( pinName, xOff, yOff ) );
    int pid = (int)m_pins.size() - 1;
    m_modules[moduleId].m_pinsId.push_back( pid );
    m_pins[pid].moduleId = moduleId;
    return pid;
}

int CParserLEFDEF::WriteDEF( const char* oldFile, const char* newFile, CPlaceDB& fplan )	
{
    printf( "Output DEF file: %s\n", newFile );
    
    ifstream in( oldFile );
    ofstream out( newFile );

    if( !in )
    {
	printf( "  Cannot open source file: %s\n", oldFile );
	return -1;
    }
    if( !out )
    {
	printf( "  Cannot open target file: %s\n", newFile );
	return -1;
    }

    char line[8000];

    bool skip = false;
    while( !in.eof() )
    {
	in.getline( line, 8000 );

	if( strncmp( line, "COMPONENTS", 10 ) == 0 )
	{
	    skip = true;
	    
	    // output our components
	    WriteComponents( out, fplan );
	}

	if( !skip )
	    out << line << endl;
	
	if( strncmp( line, "END COMPONENTS", 14 ) == 0 )
	    skip = false;
	
    }

    //printf( "Completed\n" );
    return 0; 
}

void CParserLEFDEF::WriteComponents( ostream& out, CPlaceDB& fplan )
{
    out.setf( ios::fixed, ios::floatfield );
    int nCount = 0;
    for( int i=0; i<(int)fplan.m_modules.size(); i++ )
    {
	if( fplan.m_modules[i].m_lefCellId >= 0 )
	{
	    nCount++;
	    if( fplan.m_modules[i].m_lefCellId >= (int)m_modules.size() )
	    {
		printf( " %d  of %d \n", fplan.m_modules[i].m_lefCellId,  m_modules.size() );
	    }
	    assert( fplan.m_modules[i].m_lefCellId < (int)m_modules.size() );
	}
    }
    //printf( "   Components %d --> %d\n", m_nComponents, nCount );
    out << "COMPONENTS " << m_nComponents << " ;\n";
    for( int i=0; i<(int)fplan.m_modules.size(); i++ )
    {
	if( fplan.m_modules[i].m_lefCellId >= 0 )
	{
	    out << "- "; 
	    
	    string name = fplan.m_modules[i].GetName();
	    
	    for( unsigned int iName = 0 ; iName < name.size() ; iName++ )
	    {
		if( name[iName] >= 'A' && name[iName] <= 'Z' )
		    name[iName] = tolower( name[iName] );
	    }

	    out << name;
	    
	    out	<< " " << m_modules[ fplan.m_modules[i].m_lefCellId ].GetName();

	    if( fplan.m_modules[i].m_isFixed )
	    {
		out << " + FIXED ( " << fplan.m_modules[i].GetX() << " " 
		    << fplan.m_modules[i].GetY() << " ) " 
		    << orientStr( fplan.m_modules[i].m_orient );	    
	    }
	    else
	    {
		out << " + PLACED ( " << fplan.m_modules[i].GetX() << " "
		    << fplan.m_modules[i].GetY() << " ) "
		    << orientStr( fplan.m_modules[i].m_orient );
	    }
	    
	    out << " ;\n";
	}
    }
    out << "END COMPONENTS\n";
}
