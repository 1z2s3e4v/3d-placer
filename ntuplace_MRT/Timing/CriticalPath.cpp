#include "lefdef.h"
#include "CriticalPath.h"
#include <cmath>
#include <climits>
//#include <cfloat>

using namespace std;

void TimingOptimizer::ReadTiming( const string& filename )
{
    cout << "Reading timing file " << filename << endl; 

    m_paths.clear();
    
    fstream infile;
    infile.open( filename.c_str(), ios::in );

    while( !infile.eof() )
    {
	char temp[1000];

	infile.getline( temp, 1000 );
	//cout << temp << endl;
	
	char tmp1[100];
	char tmp2[100];

	strcpy( tmp1, "" );
	
	sscanf( temp, "%s %s", tmp1, tmp2 );

	//into a path
	if( strcmp( tmp1, "PATH" ) == 0 )
	{
	    //char path_num[100];
	    //strcpy( tmp2, path_num );
		
	    //cout << tmp1 << " " << tmp2 << endl;
		    
	    CriticalPath path;

	    while( true )
	    {
		char line[1000];
		infile.getline( line, 1000 );

		char e1[100];
		char e2[100];
		char e3[100];
		char e4[100];
		char e5[100];
		char e6[100];
		char e7[100];
		char e8[100];
		char e9[100];
		char e10[100];
		char e11[100];
		char e12[100];
		char e13[100];
		char e14[100];
		char e15[100];
		char e16[100];
		char e17[100];

		sscanf( line, "%s %s %s %s", e1, e2, e3, e4 );

		if( path.m_slack == DBL_MAX && strcmp( e2, "{Slack" ) == 0 && strcmp( e3, "Time}" ) == 0 )
		{
		    TrimParenthesis( e4 );
		    path.m_slack = atof( e4 );
		    //cout << e1 << " " << e2 << " " << e3 << " " << path.m_slack << endl;
		    //break;
		}


		if( strcmp( e1, "DATA_PATH" ) == 0 )
		{
		    //test code
		    //cout << "slack " << path.m_slack << endl;
		    //@test code
		
		    
		    while( true )
		    {
			infile.getline( line, 1000 );
		
			sscanf( line, "%s %s %s %s %s "
				      "%s %s %s %s %s "
				      "%s %s %s %s %s "
				      "%s %s", 
				      e1, e2, e3, e4, e5, 
				      e6, e7, e8, e9, e10, 
				      e11, e12, e13, e14, e15, 
				      e16, e17 );

			CriticalPathElement element;
	
			//cout << line << endl;
			//cout.flush();
			
			if( strcmp( e1, "COLUMNS" ) == 0 )
			{
			    continue;
			}
			else if( strcmp( e1, "PORT" ) == 0 )
			{
			    element.m_type = CriticalPathElement::PORT;
			    
			    //fpin
			    TrimParenthesis( e3 );
			    element.m_fpin = e3;

			    //fedge
			    TrimParenthesis( e4 );
			    element.m_fedge = e4;

			    //net
			    TrimParenthesis( e7 );
			    element.m_net = e7;

			    //slew
			    TrimParenthesis( e11 );
			    element.m_slew = atof( e11 );

			    //load
			    TrimParenthesis( e12 );
			    element.m_load = atof( e12 );

			    //arrival
			    TrimParenthesis( e13 );
			    element.m_arrival = atof( e13 );

			    //required
			    TrimParenthesis( e14 );
			    element.m_required = atof( e14 );
			    
			}
			else if( strcmp( e1, "NET" ) == 0 )
			{
			    element.m_type = CriticalPathElement::NET;

			    //net
			    TrimParenthesis( e7 );
			    element.m_net = e7;
			    
			    //delay
			    TrimParenthesis( e9 );
			    element.m_delay = atof( e9 );
			    
			    //slew
			    TrimParenthesis( e11 );
			    element.m_slew = atof( e11 );

			    //load
			    TrimParenthesis( e12 );
			    element.m_load = atof( e12 );

			    //arrival
			    TrimParenthesis( e13 );
			    element.m_arrival = atof( e13 );

			    //required
			    TrimParenthesis( e14 );
			    element.m_required = atof( e14 );
			}
			else if( strcmp( e1, "INST" ) == 0 )
			{
			    element.m_type = CriticalPathElement::INST;
			    
			    //inst
			    //TrimParenthesis( e2 );
			    FixInstAndNetName( e2 );
			    element.m_inst = e2;

			    //fpin
			    TrimParenthesis( e3 );
			    element.m_fpin = e3;

			    //fedge
			    TrimParenthesis( e4 );
			    element.m_fedge = e4;
			
			    //tpin
			    TrimParenthesis( e5 );
			    element.m_tpin = e5;
			    
			    //tedge
			    TrimParenthesis( e6 );
			    element.m_tedge = e6;
			    
			    //delay
			    TrimParenthesis( e9 );
			    element.m_delay = atof( e9 );

			    //slew
			    TrimParenthesis( e11 );
			    element.m_slew = atof( e11 );

			    //load
			    //TrimParenthesis( e12 );
			    //element.m_load = atof( e12 );

			    //arrival
			    TrimParenthesis( e13 );
			    element.m_arrival = atof( e13 );

			    //required
			    TrimParenthesis( e14 );
			    element.m_required = atof( e14 );
			}
			else if( strcmp( e1, "END_DATA_PATH" ) == 0 )
			{
			    break;
			}

			path.AddElement( element );

			//test code
			element.ShowInfo();
			//@test code
		    }

		}
		
		if( strcmp( e1, "END_PATH" ) == 0 )
		{
		    break;
		}
		    
	    }
		
	    m_paths.push_back( path );
	    //printf( "# of paths: %d\n", m_paths.size() );
	}
    }
   
#if 0
    //set module, pin, and net ids
    for( unsigned int iP = 0 ; iP < m_paths.size() ; iP++ )
    {
	CriticalPath& curPath = m_paths[ iP ];

	for( unsigned iE = 0 ; iE < curPath.m_elements.size() ; iE++ )
	{
	    CriticalPathElement& curElement = curPath.m_elements[ iE ];

	    if( curElement.m_type == CriticalPathElement::PORT )
	    {
		//m_fpin_id
		curElement.m_fpin_id = m_placedb.GetModuleId( curElement.m_fpin );
		if( curElement.m_fpin_id == -1 )
		{
		    printf( "Unfound fpin_id %s\n", curElement.m_fpin.c_str() );
		}

		//m_net_id
		curElement.m_net_id = m_placedb.GetNetId( curElement.m_net );
		if( curElement.m_net_id == -1 )
		{
		    printf( "Unfound net_id %s\n", curElement.m_net.c_str() );
		}
	    }
	    else if( curElement.m_type == CriticalPathElement::NET )
	    {
		//m_net_id
		curElement.m_net_id = m_placedb.GetNetId( curElement.m_net );
		if( curElement.m_net_id == -1 )
		{
		    printf( "Unfound net_id %s\n", curElement.m_net.c_str() );
		}
	    }
	    else if( curElement.m_type == CriticalPathElement::INST )
	    {

		//m_inst_id
		curElement.m_inst_id = m_placedb.GetModuleId( curElement.m_inst );
		if( curElement.m_inst_id == -1 )
		{
		    printf( "Unfound inst_id %s\n", curElement.m_inst.c_str() );
		}
		else
		{
		    const int lefModuleId = m_placedb.m_modules[ curElement.m_inst_id ].m_lefCellId;
		    const Module& lefModule = m_placedb.m_pLEF->m_modules[ lefModuleId ];

		    bool foundFPin = false;
		    bool foundTPin = false;
		    
		    for( unsigned int i = 0 ; i < lefModule.m_pinsId.size() && 
			    (foundFPin == false || foundTPin == false) ; 
			    i++ )
		    {
			const int pId = lefModule.m_pinsId[i];
			const Pin& curPin = m_placedb.m_pLEF->m_pins[ pId ];
			
			if( foundFPin == false && curPin.pinName == curElement.m_fpin )
			{
			    curElement.m_fpin_id = i;
			    foundFPin = true;
			}
			else if( foundTPin == false && curPin.pinName == curElement.m_tpin )
			{
			    curElement.m_tpin_id = i;
			    foundTPin = true;
			}
		    }

		    if( curElement.m_fpin_id == -1 )
		    {
			printf( "Unfound fpin_id %s %s\n", curElement.m_inst.c_str(), curElement.m_fpin.c_str() );
		    }
		    
		    if( curElement.m_tpin_id == -1 )
		    {
			printf( "Unfound fpin_id %s %s\n", curElement.m_inst.c_str(), curElement.m_tpin.c_str() );
		    }
		}
	    }
	}
    }
#endif
    
#if 0
    //count the total negative slack;
    double neg_slack = 0.0;
    for( unsigned int i = 0 ; i < m_paths.size() ; i++ )
    {
	if( m_paths[i].m_slack < 0.0 )
	{
	    neg_slack += m_paths[i].m_slack;
	}
    }
    printf( "total negative slack = %.3f\n", neg_slack );
#endif
    
    infile.close();
}
