
#include <unistd.h>
#include <cstdio>
#include <string>
#include <algorithm>
using namespace std;

#include "util.h"

string GetHostName()
{
    char name[500] = "";
    gethostname( name, 500 );
    return string( name );
}

void ShowSystemInfo()
{
    
#ifdef WIN32	// Windows
    printf( "Platform: Windows\n" );
#else		// Linux
    system( "echo 'User:       '`whoami`@`hostname`" );
    system( "echo 'Date:       '`date`" );
    system( "echo 'System:     '`uname -a`" );
    //printf( "Processor:\n" );
    system( "cat /proc/cpuinfo | grep \"model name\"" ); 
#endif

}


double GetPeakMemoryUsage()
{

#if ! (defined(sun) || defined(linux) || defined(__SUNPRO_CC))
    return -1;
#endif

#if defined(sun) || defined(__SUNPRO_CC)
    char procFileName[20];
    unsigned pid=getpid();
    sprintf(procFileName,"/proc/%d/as",pid);
    struct stat buf;
    if (stat(procFileName,&buf))  // no such file on Solaris 2.5 and earlier
    {                             // so we stat another file now
	char procFileNameOld[20];
	sprintf(procFileNameOld,"/proc/%d",pid);
	if (stat(procFileNameOld,&buf))
	    return -1.0;
    }
    return (1./(1024.*1024.)) * static_cast<double>(buf.st_size);
#elif defined(linux)
    char buf[1000];
    ifstream ifs("/proc/self/stat");
    for(unsigned k=0; k!=23; k++) 
	ifs >> buf;
    //  cout << k << ": " << buf << endl; }
    return (1.0/(1024.*1024.)) * atof(buf);
#else
    return -1;
#endif
}

// 2006-12-21 (donnie) //////////////////////////////////////////////////////////////

DataHandler::DataHandler()
{
}

DataHandler::~DataHandler()
{
}

void DataHandler::Insert( double v )
{
    m_data.push_back( v );
}

void DataHandler::Clear()
{
    m_data.clear();
}

void DataHandler::Sort()
{
    sort( m_data.begin(), m_data.end() );
}

void DataHandler::OutputFile( string name )
{
    FILE* out;
    out = fopen( name.c_str(), "w" );
    for( unsigned int i=0; i<m_data.size(); i++ )
    {
	fprintf( out, "%d %f\n", i+1, m_data[i] );
    }
    fclose( out );
}

/////////////////////////////////////////////////////////////////////////////////////

