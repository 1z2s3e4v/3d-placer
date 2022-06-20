#ifndef _CRITICAL_PATH_
#define _CRITICAL_PATH_

#include "placedb.h"
#include <cfloat>
#include <cstring>

class CriticalPathElement
{
    public:
	enum Type
	{
	    PORT,
	    NET,
	    INST
	};
	Type m_type;

	//string m_name;	//instance or net names

	//for PORT and NET
	string m_net;
	int m_net_id;
	
	//for INST
	string m_inst;
	int m_inst_id;
	
	//for INST and PORT
	string m_fpin;
	int m_fpin_id;

	//for INST and PORT
	string m_fedge;

	//for INST
	string m_tpin;
	int m_tpin_id;

	//for INST
	string m_tedge;	//edge trigger

	double m_delay;
	
	double m_slew;
	double m_load;
	double m_arrival;
	double m_required;

	CriticalPathElement( void ) :
	    m_net_id( -1 ),
	    m_inst_id( -1 ),
	    m_fpin_id( -1 ),
	    m_tpin_id( -1 ),
	    m_delay( DBL_MAX ),
	    m_slew( DBL_MAX ),
	    m_load( DBL_MAX ),
	    m_arrival( DBL_MAX ),
	    m_required( DBL_MAX ) 
	{}

	void ShowInfo( void )
	{
	    string stype = "";
	    if( m_type == PORT )
	    {
		stype = "PORT";
		printf( "%4s %10s (%6d) %3s (%2d) %1s %6.3f %6.3f %6.3f %6.3f\n",
			stype.c_str(),
			m_net.c_str(), m_net_id,
			m_fpin.c_str(), m_fpin_id, m_fedge.c_str(),
			m_slew,
			m_load,
			m_arrival,
			m_required );
	    }
	    else if( m_type == NET )
	    {
		stype = "NET";
		printf( "%4s %10s (%6d) %6.3f %6.3f %6.3f %6.3f %6.3f\n",
			stype.c_str(),
			m_net.c_str(), m_net_id,
			m_delay,
			m_slew,
			m_load,
			m_arrival,
			m_required );
	    }
	    else if( m_type == INST )
	    {
		stype = "INST";
		printf( "%4s %10s (%6d) %3s (%2d) %1s %3s (%2d) %1s %6.3f %6.3f %6.3f %6.3f\n",
			stype.c_str(),
			m_inst.c_str(), m_inst_id,
			m_fpin.c_str(), m_fpin_id, m_fedge.c_str(),
			m_tpin.c_str(), m_tpin_id, m_tedge.c_str(),
			m_delay,
			m_slew,
			m_arrival,
			m_required );
	    }

	    //printf( "%4s %10s (%6d) %10s (%6d) %10s (%6d) %1s %10s (%6d) %1s %6.3f %6.3f %6.3f %6.3f %6.3f\n",
	    //	    stype.c_str(),
	    //	    m_net.c_str(), m_net_id,
	    //	    m_inst.c_str(), m_inst_id,
	    //	    m_fpin.c_str(), m_fpin_id, m_fedge.c_str(),
	    //	    m_tpin.c_str(), m_tpin_id, m_tedge.c_str(),
	    //	    m_delay,
	    //	    m_slew,
	    //	    m_load,
	    //	    m_arrival,
	    //	    m_required );
	}
};

class CriticalPath
{
    public:
	CriticalPath( void ) : m_slack( DBL_MAX ) {}
	~CriticalPath( void ) {}

	double m_slack;
	std::vector< CriticalPathElement > m_elements;

	void AddElement( const CriticalPathElement& element )
	{
	    m_elements.push_back( element );
	}
};

class TimingOptimizer
{
    public:
    double m_slack;
    CPlaceDB& m_placedb;

    TimingOptimizer( CPlaceDB& placedb ) : m_placedb( placedb ){}
    ~TimingOptimizer(void){}

    void ReadTiming( const string& filename );
    void TrimParenthesis( char* str )
    {
	int istr = 0;
	int inew = 0;
	//char tmp[1000];

	//cout << "str " << str << endl;
	while( true )
	{
	    if( str[istr] != '{' && str[istr] != '}' )
	    {
		str[inew] = str[istr];
		inew++;

		if( str[istr] == '\0' )
		{
		    break;
		}
	    }
	    istr++;
	}

	//cout << "tmp " << tmp << endl;
	//strcpy( str, tmp );
    }

    void FixInstAndNetName( char* str )
    {
	TrimParenthesis( str );

	if( str[0] != '\\' )
	{
	    return;
	}
	
	int istr = 0;
	int itmp = 0;
	char tmp[1000];

	//cout << "str " << str << endl;
	while( true )
	{
	    if( str[istr] != '\\' )
	    {
		if( str[istr] == '[' || str[istr] == ']' )
		{
		    tmp[itmp] = '\\';
		    tmp[itmp+1] = str[istr];
		    itmp += 2;
		}
		else
		{
		    tmp[itmp] = str[istr];
		    itmp++;
		}

		if( str[istr] == '\0' )
		{
		    break;
		}
	    }
	    istr++;
	}
	
	strcpy( str, tmp );
	    
    }
    
    std::vector< CriticalPath > m_paths;
};

#endif
