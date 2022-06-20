#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <cstring>
using namespace std;

#include "ParserIBMMix.h"
//#include "BinPacking.h"
#include "lefdef.h"
#include "arghandler.h"

CParserIBMMix::CParserIBMMix(void)
{
    stepNode = 600000;
    stepNet  = 400000;
}

CParserIBMMix::~CParserIBMMix(void)
{
}

int CParserIBMMix::ReadFile( const char* file, CPlaceDB &fplan ,const char* predef_nodefile  )
{
    cout << "     AUX file: " << file << endl;

    ifstream out( file );
    if( !out )
    {
	cout << "\tCannot open aux file\n";
	exit(-1);
    }

    char tmp[10000];
    out.getline( tmp, 10000 );

    char tmp1[500];
    char tmp2[500];
    char file_nodes[500];
    char file_nets[500];
    char file_wts[500];
    char file_pl[500];
    char file_scl[500];

    sscanf( tmp, "%s %s %s %s %s %s %s", 
	    tmp1, tmp2, file_nodes, file_nets, file_wts, file_pl, file_scl );

    printf( "\t\t LoadNodes:%s %s %s %s %s\n", file_nodes, file_nets, file_wts, file_pl, file_scl );

    // Read SCL first to know the row height.
    ReadSCLFile( file_scl, fplan );             // read core-row information
    printf( "Loading nodes data using %s\n", predef_nodefile);
    ReadNodesFile( predef_nodefile, fplan );		// blocks & cell width/height
    printf( "Create block name map...\n" );
    fplan.CreateModuleNameMap();

    ReadNetsFile( file_nets, fplan );		// read net file

//     if( fplan.param.plFilename != "" )    // 2005-09-01
// 	ReadPLFile( fplan.param.plFilename.c_str(), fplan );
//     else
	ReadPLFile( file_pl, fplan );			// initial placement

    fplan.ClearModuleNameMap();


    /*
    // 2005/2/4 Output result aux
    ofstream out2( "out.aux" );
    if( !out2 )
    {
	cout << "Cannot open out.aux file!\n";
	exit(0);
    }
    out2 << "RowBasedPlacement : " << file_nodes << " " << file_nets << " " 
	<< file_wts << " out.pl " << file_scl << endl;
    */

    return 0;
}




// Read AUX file
int CParserIBMMix::ReadFile( const char* file, CPlaceDB &fplan  )
{
    cout << "     AUX file: " << file << endl;

    ifstream out( file );
    if( !out )
    {
	cout << "\tCannot open aux file\n";
	exit(-1);
    }

    char tmp[10000];
    out.getline( tmp, 10000 );

    char tmp1[500];
    char tmp2[500];
    char file_nodes[500];
    char file_nets[500];
    char file_wts[500];
    char file_pl[500];
    char file_scl[500];

    sscanf( tmp, "%s %s %s %s %s %s %s", 
	    tmp1, tmp2, file_nodes, file_nets, file_wts, file_pl, file_scl );

    printf( "\t\t%s %s %s %s %s\n", file_nodes, file_nets, file_wts, file_pl, file_scl );

    // Read SCL first to know the row height.
    ReadSCLFile( file_scl, fplan );             // read core-row information
    
    ReadNodesFile( file_nodes, fplan );		// blocks & cell width/height
    printf( "Create block name map...\n" );
    fplan.CreateModuleNameMap();

    ReadNetsFile( file_nets, fplan );		// read net file

//     if( fplan.param.plFilename != "" )    // 2005-09-01
// 	ReadPLFile( fplan.param.plFilename.c_str(), fplan );
//     else
	ReadPLFile( file_pl, fplan );			// initial placement

    fplan.ClearModuleNameMap();

    /*
    // 2005/2/4 Output result aux
    ofstream out2( "out.aux" );
    if( !out2 )
    {
	cout << "Cannot open out.aux file!\n";
	exit(0);
    }
    out2 << "RowBasedPlacement : " << file_nodes << " " << file_nets << " " 
	<< file_wts << " out.pl " << file_scl << endl;
    */
    return 0;
}



int CParserIBMMix::ReadNodesFile( const char *file, CPlaceDB& fplan )
{
	string path;
	gArg.GetString( "path", &path );
	path += file;
	ifstream in( path.c_str() );
	if( !in )
	{
		cerr << "\tCannot open nodes file: " << file << endl;
		exit(-1);
	}
	
	int nNodes, nTerminals;
	nNodes = nTerminals = -1;

	int lineNumber = 0;

	// check file format string
	char tmp[10000], tmp2[10000], tmp3[10000];
	in.getline( tmp, 10000 );
	lineNumber++;

#if 0
	if( strcmp( "UCLA nodes 1.0", tmp ) != 0 )
	{
		cerr << "Nodes file header error (not UCLA nodes 1.0)\n";
		return -1;
	}
#endif

	// check file header
	int checkFormat = 0;
	while( in.getline( tmp, 10000 ) )
	{
		lineNumber++;

		//cout << tmp << endl;
		if( tmp[0] == '#' )		continue;
		if( strncmp( "NumNodes", tmp, 8 ) == 0 )
		{
			char* pNumber = strrchr( tmp, ':' );
			nNodes = atoi( pNumber+1 );

			// 2005-12-18 (allocate mem) #donnie
			fplan.ReserveModuleMemory( nNodes );
			
			checkFormat++;
		}
		else if( strncmp( "NumTerminals", tmp, 12 ) == 0 )
		{
			char* pNumber = strrchr( tmp, ':' );
			nTerminals = atoi( pNumber+1 );
			checkFormat++;
		}

		if( checkFormat == 2 )
			break;

	}

	if( checkFormat != 2 )
	{
		cerr << "** Block file header error (miss NumNodes or NumTerminals)\n";
	}

	cout << "     NumNodes: " << nNodes;
	if( nNodes > 1000 )

	    cout << " (= " << nNodes / 1000 << "k)";
	cout << endl;
	cout << "    Terminals: " << nTerminals << endl;

	// Read modules and terminals.
	char name[10000];
	char type[10000];
	double w, h;
    //p double oldH = -1;
	int nReadModules = 0;
	int nReadTerminals = 0;
	while( in.getline( tmp, 10000 ) )
	{
		lineNumber++;

		if( tmp[0] == '\0' )
			continue;
		type[0] = '\0';
		sscanf( tmp, "%s %s %s %s",
					name, tmp2, tmp3, type );

		// 2005-12-05 (FARADAY testcases)
		if( strcmp( tmp2, "terminal" ) == 0 )
		{
		    nReadTerminals++;
		    fplan.AddModule( name, 1, 1, true );
		    continue;
		}

		w = atof( tmp2 );
		h = atof( tmp3 );

		/*if( h > fplan.m_rowHeight * 10 )
		{
		    printf( "    Large MACRO %s\n", name );
		}*/

#if 0
        if( oldH != -1 && h != oldH && strcmp( type, "terminal" ) != 0)
        {
            cerr << "The program cannot handle mixed-size benchmark currently.";
            exit(0);
        }
        oldH = h;
#endif

		if( strcmp( type, "terminal" ) == 0 )
		{
			nReadTerminals++;
            fplan.AddModule( name, w, h, true );
		}
		else if( strcmp( type, "terminal_NI" ) == 0 ) // (frank) 2022-05-13 consider terminal_NI
		{
			nReadTerminals++;
			fplan.AddModule( name, w, h, true, true );
		}
		else 
		{
			nReadModules++;
			fplan.AddModule( name, w, h, false );
		}

		if( nNodes > stepNode && nReadModules % stepNode == 0 )
		    printf( "#%d...\n", nReadModules );
		
	}

	// check if modules number and terminal number match
	if( nReadModules+nReadTerminals != nNodes )
	{
		cerr << "Error: There are " << nReadModules << " modules in the file\n";
		exit(-1);
	}
	if( nReadTerminals != nTerminals )
	{
		cerr << "Error: There are " << nReadTerminals << " terminals in the file\n";
		exit(-1);
	}

	fplan.m_nModules = nNodes + nTerminals;

    // 2005/03/11
    fplan.m_modules.resize( fplan.m_modules.size() );


    // TEST: memory upper bound
    //int *ptr;
    //while( true )
    //{
    //    ptr = new int [5000000];

    //    for( int i=0; i<5000000; i++ )
    //        ptr[i] = i;

    //    cout << "*";
    //    flush(cout);
    //    sleep(1);
    //}

	// === debug ===
	//fplan.PrintModules();
	//fplan.PrintTerminals();
	// =============

	return 0;
}




int CParserIBMMix::ReadPLFile( const char *file, CPlaceDB& fplan )
{
	cout << "Initialize position with the file: " << file << "\n";

	string path;
	gArg.GetString( "path", &path );
	path += file;
	ifstream in( path.c_str() );
	if( !in )
	{
		cerr << "\tCannot open PL file: " << file << endl;
		exit(-1);
	}
	
	int lineNumber = 0;

	// check file format string
	char tmp[10000];
	in.getline( tmp, 10000 );
	lineNumber++;
	//if( strcmp( "UCLA pl 1.0", tmp ) != 0 )
	//{
	//	cerr << "PL file header format error (UCLA pl 1.0)\n";
	//	return -1;
	//}

	char name [10000];
	char dir[1000];
	float x, y;
	while( in.getline( tmp, 10000 ) )
	{
		lineNumber++;

		//cout << tmp << endl;
		if( tmp[0] == '#' )		continue;
		if( tmp[0] == '\0' )	continue;

		for( int i=0; i<(int)strlen(tmp); i++ )
		{
			if( tmp[i] == '(' )		tmp[i] = ' ';
			if( tmp[i] == ',' )		tmp[i] = ' ';
			if( tmp[i] == ')' )		tmp[i] = ' ';
			if( tmp[i] == ':' )		tmp[i] = ' ';
			if( tmp[i] == '=' )		tmp[i] = ' ';
			if( tmp[i] == '\r' )		tmp[i] = ' ';
		}
	
		name[0] = '\0';	
		int ret = sscanf( tmp, "%s %f %f %s ", name, &x, &y, dir );
		if( ret <= 0 )
		    continue;

		if( ret != 4 && ret != 3 )
		{
		    //cerr << "Error in the PL file: <" << tmp << ">\n";
		    //exit(-1);
		    printf( "Syntax (may) error in line %d. Please check. (ret = %d)\n", 
			    lineNumber, ret );
		    continue;	// skip this line...
		}

		if( ret == 3 )
		{
		    //printf( "Block %s does not has orientation.\n", name );
		    dir[0] = 'N';
		    dir[1] = '\0';
		}
		
		int moduleId = fplan.GetModuleId( name );
		if( moduleId == -1 )
		{
			cerr << "Error: module name " << name << " not found in line " 
				<< lineNumber << " file: " << file << endl;
			exit(-1);
		}
		
		fplan.SetModuleLocation( moduleId, x, y);
		fplan.SetModuleOrientation( moduleId, orientInt( dir ) );
	}

	return 0;
}



int CParserIBMMix::ReadNetsFile( const char *file, CPlaceDB& fplan )
{
	string path;
	gArg.GetString( "path", &path );
	path += file;
	ifstream in( path.c_str() );
	if( !in )
	{
		cerr << "Cannot open net file: " << file << endl;
		exit(-1);
	}

	int nNets, nPins;
	nNets = nPins = -1;

	int lineNumber = 0;

	// check file format string
	char tmp[10000];
	in.getline( tmp, 10000 );
	lineNumber++;

#if 0
	if( strcmp( "UCLA nets 1.0", tmp ) != 0 )
	{
		cerr << "Nets file header error (UCLA nets 1.0)\n";
		exit(-1);
	}
#endif

	// check file header
	int checkFormat = 0;
	while( in.getline( tmp, 10000 ) )
	{
		lineNumber++;

		//cout << tmp << endl;
		if( tmp[0] == '#' )		continue;
		if( strncmp( "NumNets", tmp, 7 ) == 0 )
		{
			char* pNumber = strrchr( tmp, ':' );
			nNets = atoi( pNumber+1 );
			fplan.ReserveNetMemory( nNets );
			checkFormat++;
		}
		else if( strncmp( "NumPins", tmp, 7 ) == 0 )
		{
			char* pNumber = strrchr( tmp, ':' );
			nPins = atoi( pNumber+1 );
			fplan.ReservePinMemory( nPins );
			checkFormat++;
		}

		if( checkFormat == 2 )
			break;
	}

	if( checkFormat != 2 )
	{
		cerr << "** Net file header error\n";
	}

	cout << "         Nets: " << nNets << endl;
	cout << "         Pins: " << nPins << endl;

	char tmp1[2000], tmp2[2000], tmp3[2000], tmp4[2000];
	int maxDegree = 0;
	int degree;
	int nReadPins = 0;
	int nReadNets = 0;
	while( in.getline( tmp, 10000 ) )
	{
	    lineNumber++;

	    if( tmp[0] == '\0' )
		continue;

	    sscanf( tmp, "%s %s %d", tmp1, tmp2, &degree );
	    if( strcmp( tmp1, "NetDegree" ) != 0 || degree < 0 ) 
	    {
		cerr << "Syntax unsupport in line " << lineNumber << ": " 
		    << tmp1 << endl;
		return 01;
	    }
	    Net net;
	    int vCount;
	    int moduleId;
	    int pinId;
	    double xOff, yOff;
	    if( degree > maxDegree )    
		maxDegree = degree;
	    net.reserve( degree );  // reserve the memory for "degree" pins
	    nReadPins += degree;	    // will read "degree" pins
	    
	    for( int j=0; j<degree; j++ )
	    {
		in.getline( tmp, 10000 );
		lineNumber++;
		tmp3[0] = '\0';
		tmp4[0] = '\0';
		vCount = sscanf( tmp, "%s %s : %s %s", tmp1, tmp2, tmp3, tmp4 );
		
		moduleId = fplan.GetModuleId( tmp1 );
		
		if( tmp3[0] != '\0' )
		    xOff = atof( tmp3 );
		else
		    xOff = 0;
		if( tmp4[0] != '\0' )
		    yOff = atof( tmp4 );
		else
		    yOff = 0;
		pinId = fplan.AddPin( moduleId, xOff, yOff );
		net.push_back( pinId );

		// 2005/2/2 (donnie)
		// TODO: Remove duplicate netsIds
		// 2007/3/9 (indark)
		// remove duplicated netsIds
		bool found = false;
		for(unsigned int z = 0 ; z < fplan.m_modules[moduleId].m_netsId.size() ; z++ ){
			if ( nReadNets == fplan.m_modules[moduleId].m_netsId[z] ){
				found = true;
				break;
			}
		}
		if (!found) 
			fplan.m_modules[moduleId].m_netsId.push_back( nReadNets );
	    }
	    fplan.AddNet( net );
	    nReadNets++;

	    if( nReadNets % stepNet == 0 && nNets > stepNet )
		printf( "#%d...\n", nReadNets );
		
	}

	// TODO: if nReadNets > nNets may have memory problem.
	// check if modules number and terminal number match
	if( nNets != nReadNets )
	{
		cerr << "Error: There are " << nReadNets << " nets in the file\n";
		exit(-1);
	}
	if( nReadPins != nPins )
	{
		cerr << "Error: There are " << nReadPins << " pins in the file\n";
		exit(-1);
	}

	fplan.m_nPins = nPins;
	fplan.m_nNets = nNets;

#if 1
    cout << "Max net degree= " << maxDegree << endl;
#endif

    // 2005/03/11
    fplan.m_pins.resize( fplan.m_pins.size() );
    fplan.m_nets.resize( fplan.m_nets.size() );

    // === debug ===
    //fplan.PrintNets();
    // =============

    return 0;
}


// Modified by Jin on 20050129
int CParserIBMMix::ReadSCLFile( const char *file, CPlaceDB& fplan )
{
	// In this version, we don't care the pre-placed location.
	// Read Numrows, Height, and Numsites only.

	string path;
	gArg.GetString( "path", &path );
	path += file;
	ifstream in( path.c_str() );
	if( !in )
	{
		cerr << "Cannot open .scl file: " << file << endl;
		exit(-1);
	}

	int nRows, nSites, height;
	nRows = nSites = height = -1;
	//double bottom = -1;
	//double left = -1;


	int lineNumber = 0;

	// check file format string
	char tmp[10000];
	in.getline( tmp, 10000 );
	lineNumber++;

#if 0
	if( strcmp( "UCLA scl 1.0", tmp ) != 0 )
	{
		cerr << "SCL file header error (UCLA scl 1.0)\n";
		exit(-1);
	}
#endif

	char tmp1[500], tmp2[500], tmp3[500], tmp4[500], tmp5[500], tmp6[500];
	
	vector<CSiteRow> &vSites = fplan.m_sites;
	double SiteWidth = 1;   // default value is 1 (2005/2/14 donnie)
	ORIENT orient = OR_N; // 2006-04-23 (donnie)
	while( in.getline( tmp, 10000 ) )
	{
		tmp1[0] = tmp2[0] = tmp3[0] = tmp4[0] = tmp5[0] = tmp6[0] = '\0';
		sscanf( tmp, "%s %s %s %s %s %s", tmp1, tmp2, tmp3, tmp4, tmp5, tmp6 );

		if( strcmp( tmp1, "Numrows" ) == 0 || strcmp( tmp1, "NumRows" ) == 0 )
		{
			nRows = atoi( tmp3 );
			////test code
			//printf("get numrows %f\n", atoi( tmp3 ) );
			////@test code
		}
		else if( strcmp( tmp1, "CoreRow" ) == 0 && strcmp( tmp2, "Horizontal" ) == 0 ) // start of a row
		{
			vSites.push_back( CSiteRow() );
			////test code
			//printf("get corerow, site size: %d\n", vSites.size() );
			////@test code
		}
		else if( strcmp( tmp1, "Coordinate" ) == 0 )
		{
			vSites.back().m_bottom = atof( tmp3 );
			////test code
			//printf("get coordinate %f\n", atof( tmp3 ) );
			////@test code
		}
		else if( strcmp( tmp1, "Height" ) == 0 )
		{
			vSites.back().m_height = atof( tmp3 );
			////test code
			//printf("get height %f\n", atof( tmp3 ) );
			////@test code
		}
		else if( strcmp( tmp1, "Sitewidth" ) == 0 )
		{
			SiteWidth = atof( tmp3 );
			vSites.back().m_step = atof( tmp3 );// donnie
			////test code
			//printf("get sitewidth %f\n", atof( tmp3 ) );
			////@test code
		}	
		else if( strcmp( tmp4, "Numsites" ) == 0 || strcmp( tmp4, "NumSites" ) == 0 )
		{
			vSites.back().m_interval.push_back( atof( tmp3 ) );
			vSites.back().m_interval.push_back( ( atof( tmp6 )*SiteWidth ) + atof( tmp3 ) );
			////test code
			//printf("get numsites: %f %f\n", atof(tmp3), ( atof( tmp6 )*SiteWidth ) + atof( tmp3 ) );
			////@test code
		}
		else if( strcmp( tmp1, "Siteorient" ) == 0 || strcmp( tmp1, "SiteOrient" ) == 0 ) // donnie 2006-04-23
		{
		    if( strcmp( tmp3, "S" ) == 0 || strcmp( tmp3, "FS" ) == 0 )
			orient = OR_S;
		    else
			orient = OR_N;	
		    vSites.back().m_orient = orient;
		}


	}

	//cout << "     Numrows: " << nRows << "\n";
	//cout << "      Height: " << height << "\n";
	//cout << "    Numsites: " << nSites << "\n";
	//cout << " Core region: (" << left << "," << bottom << ")-(" 
	//	 << left+nSites << "," << bottom + nRows * height << ")\n";

	fplan.m_rowHeight = vSites.back().m_height;
	//fplan.SetCoreRegion( left, bottom, left+nSites, bottom + nRows*height );
	//fplan.SetCoreRegion( vSites.front().m_interval.front(), vSites.front().m_bottom, vSites.front().m_interval.back(), vSites.back().m_bottom + vSites.back().m_height );
	fplan.SetCoreRegion(); 
   
	// Romove the sites occupied by the fixed module


	//included in CPlaceDB
	//@Romove the sites occupied by the fixed module



	return 0;
}


//int CParserIBMMix::ReadSCLFile( const char *file, CPlaceDB& fplan )
//{
//	// In this version, we don't care the pre-placed location.
//	// Read Numrows, Height, and Numsites only.
//
//	ifstream in( file );
//	if( !in )
//	{
//		cerr << "Cannot open .scl file: " << file << endl;
//		exit(-1);
//	}
//
//	int nRows, nSites, height;
//	nRows = nSites = height = -1;
//	double bottom = -1;
//	double left = -1;
//
//
//	int lineNumber = 0;
//
//	// check file format string
//	char tmp[1000];
//	in.getline( tmp, 1000 );
//	lineNumber++;
//	if( strcmp( "UCLA scl 1.0", tmp ) != 0 )
//	{
//		cerr << "Nets file header error (UCLA scl 1.0)\n";
//		exit(-1);
//	}
//
//	char tmp1[50], tmp2[50], tmp3[50], tmp4[50], tmp5[50], tmp6[50];
//	int check = 0;
//	while( in.getline( tmp, 1000 ) )
//	{
//		tmp1[0] = tmp2[0] = tmp3[0] = tmp4[0] = tmp5[0] = tmp6[0] = '\0';
//		sscanf( tmp, "%s %s %s %s %s %s", tmp1, tmp2, tmp3, tmp4, tmp5, tmp6 );
//
//		if( strcmp( tmp1, "Numrows" ) == 0 ||
//            strcmp( tmp1, "NumRows" ) == 0 )
//		{
//			nRows = atoi( tmp3 );
//			check++;
//		}
//		else if( strcmp( tmp1, "Height" ) == 0 )
//		{
//			height = atoi( tmp3 );
//			check++;
//		}
//		else if( strcmp( tmp4, "Numsites" ) == 0 || strcmp( tmp4, "NumSites" ) == 0)
//		{
//			nSites = atoi( tmp6 );
//			left = atoi( tmp3 );
//			check++;
//		}
//		else if( strcmp( tmp1, "Coordinate" ) == 0 )
//		{
//			bottom = atoi( tmp3 );
//			check++;
//		}
//		if( check == 4 )
//			break;
//	}
//
//	cout << "     Numrows: " << nRows << "\n";
//	cout << "      Height: " << height << "\n";
//	cout << "    Numsites: " << nSites << "\n";
//	cout << " Core region: (" << left << "," << bottom << ")-(" 
//		 << left+nSites << "," << bottom + nRows * height << ")\n";
//
//	fplan.m_rowHeight = height;
//	fplan.SetCoreRegion( left, bottom, left+nSites, bottom + nRows*height );
//
//	return 0;
//}

