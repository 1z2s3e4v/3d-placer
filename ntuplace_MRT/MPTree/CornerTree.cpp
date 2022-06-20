#include "CornerTree.h"
#include "sa.h"
#include "mptreeutil.h"

#include <iostream>
#include <cstdio>
#include <cassert>
#include <cmath>
#include <cstdlib>
using namespace std;
using namespace N_ntumptree;

bool PLOT_NET = false;

string getLevelName( string str, int level );

CornerTree::CornerTree()
{
    m_btrees = NULL;
    
    OPT_SEQ.clear();
    OPT_SEQ.push_back( 0 );
    OPT_SEQ.push_back( 1 );
    OPT_SEQ.push_back( 2 );
    OPT_SEQ.push_back( 3 );
    
    Init();
}

CornerTree::~CornerTree()
{
    //if( NULL != m_btrees )
   // delete [] m_btrees;
}

void CornerTree::Init()
{
    delete m_btrees;
    m_btrees = NULL;
    m_chipWidth = 0;
    m_chipHeight = 0;
    m_clusterCount = 0;
    m_nBlockages = 0;
    m_offset_X = 0;
    m_offset_Y = 0;
}

void CornerTree::LoadModules( const char* filename )
{
    cout << "Load modules from " << filename << endl;
    ifstream in( filename );
    if( !in )
    {
	cerr << "Open input file failed. (filename= " << filename << ")\n";
	return;
    }

    m_stdCoreLeft   = 0;
    m_stdCoreBottom = 0;
    m_stdCoreRight  = 0;
    m_stdCoreTop    = 0;
    
    char line[50000];
    int parseCount;
    int moduleNumber = 0;
    int readModuleNumber = 0;
    int padNumber = 0;
    int readPadNumber = 0;
    char tmp0[500], tmp1[500], tmp2[500], tmp3[500], tmp4[500];
    char tmp5[500], tmp6[500], tmp7[500], tmp8[500], tmp9[500], tmp10[500], tmp11[500];
    int readMode = 0;
    Module m;
    int lineCount = 1;
    while( true )
    {
    
	line[0] = '\0';
        in.getline( line, 10000 );
	line[9999] = '\0';

	if( in.eof() /*&& line[0] == '\0'*/ )
	    break;
	
	lineCount++;
    	if( lineCount % 100000 == 0 )
	    printf( "Parse line %d\n", lineCount );
	
	if( line[0] == '#' )
	    continue;
	
	parseCount = sscanf( line, "%s %s %s %s %s %s %s %s %s %s %s %s", 
		tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11 );

	if( parseCount <= 0 )
	    continue;
	
	if( parseCount >= 2 )
	{
	    if( strcmp( tmp0, "DIEAREA" ) == 0 )
	    {
		// Assume left-bottom is (0, 0).
		m_chipWidth = atof( tmp3 );
		m_chipHeight = atof( tmp4 );
		cout << "DIEAREA width= " << m_chipWidth << " height= " << m_chipHeight << endl;
	    }
	    else if( strcmp( tmp0, "count" ) == 0 )
	    {
		moduleNumber = atoi( tmp2 );
		cout << "expect macro # = " << moduleNumber << endl;
	    }
	    else if( strcmp( tmp0, "file" ) == 0 )
	    {
		cout << "Source file = " << tmp2 << endl;
	    }
	    else if( strcmp( tmp0, "Row" ) == 0 )
	    {
		m_rowHeight = atof( tmp4 );
	    }
	    else if( strcmp( tmp0, "core:" ) == 0 )
	    {
		m_stdCoreLeft   = atof( tmp1 );
		m_stdCoreBottom = atof( tmp2 );
		m_stdCoreRight  = atof( tmp3 );
		m_stdCoreTop    = atof( tmp4 );
		printf( "Std-cell core: (%g, %g)-(%g, %g)\n", 
			m_stdCoreLeft, m_stdCoreBottom, m_stdCoreRight, m_stdCoreTop );
	    }
	    /*else if( parseCount == 8 || parseCount == 9 )
	    {
		// read modules
		m_modules.push_back( m );
		Module& mod = m_modules.back();
		//int name_len = strlen( tmp0 );
		strcpy( mod.name, tmp0 );   // module name
		mod.name[strlen(tmp0)-1] = '\0';
		//cout << tmp0 << " ";
		mod.x = atof( tmp4 );
		mod.y = atof( tmp5 );
		mod.width = atof( tmp6 );
		mod.height = atof( tmp7 );
		mod.area = mod.width * mod.height;

		mod.no_rotate = false;
		mod.is_fixed = false;
		if( parseCount == 9 )
		{
		    if( strcmp( tmp8, "fixed" ) == 0)
		    {
			//mod.is_fixed = true;
			mod.no_rotate = true;
		    }

		    if( strcmp( tmp8, "skip" ) == 0 )	// modules outside the core region
		    {
			mod.no_rotate = true;
			mod.is_fixed = true;
		    }

		    if( strcmp( tmp8, "cfixed" ) == 0 )	// corner block, fixed block
		    {
			mod.is_fixed = true;
			mod.no_rotate = true;
			mod.is_corner = true;
		    }
		}
		
		readModuleNumber++;
	    }*/
	    else if( parseCount == 11 || parseCount == 12 )
	    {
		// 2005-09-05
		
		// read modules
		m_modules.push_back( m );
		Module& mod = m_modules.back();
		strcpy( mod.name, tmp0 );	    // module name
		mod.name[strlen(tmp0)-1] = '\0';    // remove ":"
		// tmp1: x_symmtry
		// tmp2: y_symmtry
		// tmp3: 90_rotate
		// tmp4: degree
		// tmp5: flip
		// tmp6: orientation
		//printf( " %s %s %s %s %s %s\n", tmp1, tmp2, tmp3, tmp4, tmp5, tmp6 );
		string ori = tmp6;
		orientation.push_back(tmp6);
		mod.x = atof( tmp7 );
		mod.y = atof( tmp8 );
		mod.width = atof( tmp9 );
		mod.height = atof( tmp10 );
		mod.area = mod.width * mod.height;
		mod.no_rotate = false;
		mod.is_fixed = false;
		mod.id = m_modules.size() - 1;
		
		if( parseCount == 12 && strcmp( tmp11, "fixed" ) == 0 )
		{
		    printf( "   %s is fixed\n", mod.name );
		    mod.no_rotate = true;
		    mod.is_fixed = true;
		}
		
		//printf( "Module = %-30s", mod.name );
		readModuleNumber++;

		// read pins
		in.getline( line, 10000 );
		line[9999] = '\0';
	        parseCount = sscanf( line, "%s %s %s %s %s %s %s %s %s %s %s %s",
		                tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11 );
		if( strcmp( tmp0, "number" ) != 0 )
		{
		    printf( "Parse error, expect 'number of pins': %s\n", line );
		}
		int nPins = atoi( tmp4 );
		//printf( "\tPin # = %d\n", nPins );

		for( int i=0; i<nPins; i++ )
		{
		    in.getline( line, 10000 );
		    line[9999] = '\0';
		    parseCount = sscanf( line, "%s %s", tmp0, tmp1 );
		    double xOff = atof( tmp0 ) + mod.width * 0.5;
		    double yOff = atof( tmp1 ) + mod.height * 0.5;
		    Pin p;
		    mod.pins.push_back( p );
		    mod.pins.back().x = xOff;
		    mod.pins.back().y = yOff;
		    mod.pins.back().mod = mod.id;
		    mod.pins.back().ax = -1;
		    mod.pins.back().ay = -1;
		    mod.pins.back().net = -1;
		    //printf( "   Pin %d ( %g, %g )\n", i, x, y );
		}
	    }
	    else if( parseCount == 2 && strcmp( tmp0, "pad" ) == 0 )
	    {
		// pad infomation
		in.getline( line, 10000 );
		line[9999] = '\0';
		parseCount = sscanf( line, "%s %s %s %s %s", tmp0, tmp1, tmp2, tmp3, tmp4 );
		padNumber = atoi( tmp4 );
		printf( "Pad # = %d\n", padNumber );
		readMode = 1;
	    }
	    else if( parseCount == 6 && readMode == 1 )
	    {
		// read pads

		m_modules.push_back( m );
		Module& mod = m_modules.back();
		strcpy( mod.name, tmp0 );           // pad name
		// tmp1: orientation
		mod.x = atof( tmp2 );
		mod.y = atof( tmp3 );
		mod.width  = atof( tmp4 );
		mod.height = atof( tmp5 );
		mod.no_rotate = true;
		mod.is_fixed  = true;
		mod.is_pad    = true;
		mod.id = m_modules.size()-1;
		//printf( "Pad = %-30s", mod.name );
		readPadNumber++;
		
		// read pins
		in.getline( line, 10000 );
		line[9999] = '\0';
		parseCount = sscanf( line, "%s %s %s %s %s %s %s %s %s %s %s %s",
			tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11 );
		if( strcmp( tmp0, "number" ) != 0 )
		{
		    printf( "Parse error, expect 'number of pins': %s\n", line );
		}
		int nPins = atoi( tmp4 );
		//printf( "\tPin # = %d\n", nPins );

		// pad pins
		for( int i=0; i<nPins; i++ )
		{
		    in.getline( line, 10000 );
		    line[9999] = '\0';
		    parseCount = sscanf( line, "%s %s", tmp0, tmp1 );
		    //double x = atof( tmp0 );
		    //double y = atof( tmp1 );
		    //printf( "   Pin %d ( %g, %g )\n", i, x, y );
                    double xOff = atof( tmp0 ) + mod.width * 0.5;
                    double yOff = atof( tmp1 ) + mod.height * 0.5;
                    Pin p;
                    mod.pins.push_back( p );
                    mod.pins.back().x = xOff;
                    mod.pins.back().y = yOff;
                    mod.pins.back().mod = mod.id;
                    mod.pins.back().ax = -1;
                    mod.pins.back().ay = -1;
                    mod.pins.back().net = -1;
		}
	    }
	    else if( parseCount == 2 && strcmp( tmp0, "net" ) == 0 )
	    {
		in.getline( line, 10000 );
		line[9999] = '\0';
		parseCount = sscanf( line, "%s %s %s %s %s %s %s %s %s %s %s %s",
		             tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11 );
		int nNets = atoi( tmp4 );
		printf( "Net # = %d\n", nNets );

		for( int i=0; i<nNets; i++ )
		{
		    in.getline( line, 40000 );
		    line[39999] = '\0';
		    
		    // check degree
		    int degree = 1;
		    int ptr1 = 0;
		    for( int l=0; l<(int)strlen(line); l++ )
		    {
			if( line[l] == ':' )
			{
			    ptr1 = l+1;
			}
			if( line[l] == ',' )
			    degree++;
		    }
		    
		    char seps[] = " ,";
		    char* token;
		    token = strtok( line+ptr1, seps );
		    bool macro = true;
		    int macroId = -1, pinId;
		    vector<Pin*> net;
		    while( token != NULL )
		    {
		        // While there are tokens in "string" 
		        //printf( " %s\n", token );
			if( macro )
			{
			    macroId = atoi( token );
			    if( macroId >= (moduleNumber + padNumber) )
			    {
				printf( "Error at net %d, macro id is >= module #\n", i );
				exit(-1);
			    }
			    macro = false;
			}
			else
			{
			    pinId = atoi( token );
			    if( pinId >= (int)m_modules[macroId].pins.size() )
			    {
				printf( "Error at net %d, pin id %d of macro %d is >= pins.size()\n",
					 i, pinId, macroId );
				exit(-1);
			    }
			    m_modules[macroId].pins[pinId].net = i;
			    net.push_back( &m_modules[macroId].pins[pinId] );
			    macro = true;
			}
			
		        // Get next token: 
		        token = strtok( NULL, seps );
		    }
		    m_network.push_back( net ); 
		    //printf( "Net %d, degree = %d \n", i, degree );
		}
		printf( "  Read net complete\n" );
	    }
	    else
	    {
		printf( "Parse error: '%s'\n", line );
		exit(-1);	
	    }
	}
	else
	{
	    printf( "Parse error: '%s' (%d)\n", line, parseCount );
	    exit(-1);
	}
    }
    //m_modules.resize( readModuleNumber );
    //cout << "Read " << readModuleNumber << " modules\n";

    printf( "Std-cell core: (%g, %g)-(%g, %g)\n", 
	    m_stdCoreLeft, m_stdCoreBottom, m_stdCoreRight, m_stdCoreTop );

    if( m_stdCoreRight - m_stdCoreLeft == 0 )
    {
	printf( "\nCore region error! (horizontal)\n\n" );
	exit(-1);
    }
    if( m_stdCoreTop - m_stdCoreBottom == 0 )
    {
	printf( "\nCore region error! (vertical)\n\n" );
	exit( -1 );
    }
    
    m_moduleNumber = moduleNumber;  // no pads
    moduleNumber = moduleNumber + padNumber;	// macro + pad
    printf( "[Testcase]: Macro # = %d, Pad # = %d, Net # = %d\n\n", 
	    moduleNumber, padNumber, m_network.size() );
    m_modules.resize( moduleNumber );
    
    // Create modules info
    // We use "rotate" and "flip" only.
    m_modulesInfo.resize( moduleNumber );
    for( int i=0; i<moduleNumber; i++ )
    {
	m_modulesInfo[i].rotate = m_modulesInfo[i].flip = false;
    }

    CreateCoreRegion();
    
    
}

// 2005-07-26
void CornerTree::CreateCoreRegion()
{
    cout << "Create core region.\n";
    
    // 2005-09-06: (donnie) Use the std-cell core
    m_cornerLB.x = m_cornerLT.x = m_stdCoreLeft;
    m_cornerRB.x = m_cornerRT.x = m_stdCoreRight;
    m_cornerLB.y = m_cornerRB.y = m_stdCoreBottom;
    m_cornerLT.y = m_cornerRT.y = m_stdCoreTop;

    m_coreLeft   = m_stdCoreLeft;
    m_coreRight  = m_stdCoreRight;
    m_coreTop    = m_stdCoreTop;
    m_coreBottom = m_stdCoreBottom;
    
    double x_center = m_chipWidth * 0.5;
    double y_center = m_chipHeight * 0.5;
    
    // Use corner blocks to adjust core region.
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( m_modules[i].is_corner == false )
	    continue;

	if( m_modules[i].x < x_center ) // left
	{
		if( m_modules[i].y < y_center ) // b-2ottom
	    {
		cout << "   Module " << m_modules[i].name << " is the left-bottom corner block.\n";
		if( m_modules[i].y > m_coreBottom )
		{
		    m_coreBottom = m_modules[i].y;
		    cout << "      Set the bottom boundary. (y= " << m_coreBottom << ")\n";
		}
		m_cornerLB.x = m_modules[i].x;
		m_cornerLB.y = m_modules[i].y;
	    }
	    else // top
	    {
		cout << "   Module " << m_modules[i].name << " is the left-top corner block.\n";
		if( m_modules[i].y + m_modules[i].height < m_coreTop )
		{
		    m_coreTop = m_modules[i].y + m_modules[i].height;
		    cout << "      Set the top boundary. (y= " << m_coreTop << ")\n";
		}
		m_cornerLT.x = m_modules[i].x;
		m_cornerLT.y = m_modules[i].y + m_modules[i].height;
	    }
	    if( m_modules[i].x > m_coreLeft )
	    {
		m_coreLeft = m_modules[i].x;
		cout << "      Set the left boundary. (x= " << m_coreLeft << ")\n";
	    }
	}
	else // right
	{
	    if( m_modules[i].y < y_center ) // bottom
	    {
		cout << "   Module " << m_modules[i].name << " is the right-bottom corner block.\n";
		if( m_modules[i].y > m_coreBottom )
		{
		    m_coreBottom = m_modules[i].y;
		    cout << "      Use the bottom boundary. (y= " << m_coreBottom << ")\n";
		}
		m_cornerRB.x = m_modules[i].x + m_modules[i].width;
		m_cornerRB.y = m_modules[i].y;
	    }
	    else // top
	    {
		cout << "   Module " << m_modules[i].name << " is the right-top corner block.\n";
		if( m_modules[i].y + m_modules[i].height < m_coreTop )
		{
		    m_coreTop = m_modules[i].y + m_modules[i].height;
		    cout << "      Use the top boundary. (y= " << m_coreTop << ")\n";
		}
		m_cornerRT.x = m_modules[i].x + m_modules[i].width;
		m_cornerRT.y = m_modules[i].y + m_modules[i].height;
	    }
	    if( m_modules[i].x + m_modules[i].width < m_coreRight )
	    {
		m_coreRight = m_modules[i].x + m_modules[i].width;
		cout << "      Use the right boundary. (x= " << m_coreRight << ")\n";
	    }
	}
    }
    
    cout << "   Core region (" << m_coreLeft << ", " << m_coreBottom << ")-("
	 << m_coreRight << ", " << m_coreTop << ")\n";

}

void CornerTree::ListModules()
{
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	cout << "M" << i << " --> " 
	    << m_modules[i].name << " (x,y)= (" << m_modules[i].x << ","
	    << m_modules[i].y << ") w= " << m_modules[i].width 
	    << " h=" << m_modules[i].height;
	if( m_modules[i].no_rotate )
	    cout << " no_rotate ";
	if( m_modules[i].is_fixed )
	    cout << " fixed ";
	cout << "\n";
    } 
}   

void CornerTree::Optimize( int times, int local )
{

    printf( "[Optimization Macro Placement]\n" );
    
    CreateBTrees();
  
//     OutputGNUplot( "before_optimize_0.plt", 0 );
//     OutputGNUplot( "before_optimize_1.plt", 1 );
//     OutputGNUplot( "before_optimize_2.plt", 2 );
//     OutputGNUplot( "before_optimize_0_nets.plt", 0, true );
//     OutputGNUplot( "before_optimize_1_nets.plt", 1, true );
//     OutputGNUplot( "before_optimize_2_nets.plt", 2, true );
    
    //return;
    
    // 1 2 
    // 0 3
   
    for( int k=0; k<4; k++ )
    {

	assert( OPT_SEQ.size() == 4 );
	int i = OPT_SEQ[k];

	printf( "\n\n Optimize R%d (module # = %d, net # = %d) \n\n", 
		i, m_btrees[i].modules.size(), m_btrees[i].netCount );

	int freeModules = 0;
	for( int k=0; k<(int)m_btrees[i].modules.size(); k++ )
    	    if( !m_btrees[i].modules[k].is_fixed )
		freeModules++;
	if( freeModules == 0 )
	{
		printf( "Skip region %d since it has no movable macro\n", i );
	}	
	else
	{
	    m_btrees[i].init(); // init B*-tree

	    // 2005-10-03 (donnie)
	    // Before optimizing, we add blockages for region i
	    CreateBlockages( i );

	    m_btrees[i].normalize_cost( 1000 );

	    //int times=50, local=7;
	    double term_temp=0.1;
	    /*double last_time =*/  SA_Floorplan( m_btrees[i], times, local, term_temp, SA_MAX_ITE );
	    m_btrees[i].list_information();
	}


#if 0   // remove for MP-tree DAC-2007 submission
	
	// update neighbor outline
	if( i == 0 )
	{
	    double y_slack = m_coreTop - m_coreBottom - m_btrees[0].getHeight();
	    cout << "[Change R1.height from " << m_btrees[1].outline_height << " to " << y_slack << endl;
	    m_btrees[1].outline_height = y_slack;
	    double x_slack = m_coreRight - m_coreLeft - m_btrees[0].getWidth();
	    cout << "[Change R3.width from " << m_btrees[3].outline_width << " to " << x_slack << endl;
	    m_btrees[3].outline_width = x_slack;
	}
	else if( i == 1 )
	{
	    double x_slack = m_coreRight - m_coreLeft - m_btrees[1].getWidth();
	    cout << "[Change R2.width from " << m_btrees[2].outline_width << " to " << x_slack << endl;
	    m_btrees[2].outline_width = x_slack;
	    double y_slack = m_coreTop - m_coreBottom - m_btrees[1].getHeight();
	    cout << "[Change R0.height from " << m_btrees[0].outline_height << " to " << y_slack << endl;
	    m_btrees[0].outline_height = y_slack;
	}
	else if( i == 2 )
	{
	    double y_slack = m_coreTop - m_coreBottom - m_btrees[2].getHeight();
	    cout << "[Change R3.height from " << m_btrees[3].outline_height << " to " << y_slack << endl;
	    m_btrees[3].outline_height = y_slack;
	    double x_slack = m_coreRight - m_coreLeft - m_btrees[2].getWidth();
	    cout << "[Change R1.width from " << m_btrees[1].outline_width << " to " << x_slack << endl;
	    m_btrees[1].outline_width = x_slack;
	}
	else // i == 3
	{
	    double x_slack = m_coreRight - m_coreLeft - m_btrees[3].getWidth();
	    cout << m_coreRight << " " << m_coreLeft << " " << m_btrees[3].getWidth() << endl;
	    cout << "[Change R0.width from " << m_btrees[0].outline_width << " to " << x_slack << endl;
	    m_btrees[0].outline_width = x_slack;
	    double y_slack = m_coreTop - m_coreBottom - m_btrees[3].getHeight();
	    cout << "[Change R2.height from " << m_btrees[2].outline_height << " to " << y_slack << endl;
	    m_btrees[2].outline_height = y_slack;
	}

#endif

    }

    printf( "\n\nUpdate module positions\n\n" );
    UpdateModulePositions();

    double c1 = m_btrees[0].getCost();
    double c2 = m_btrees[1].getCost();
    double c3 = m_btrees[2].getCost();
    double c4 = m_btrees[3].getCost();
    cout << "Floorplan quality:\n";
    cout << "      LB cost = " << c1 << endl;
    cout << "      LT cost = " << c2 << endl;
    cout << "      RT cost = " << c3 << endl;
    cout << "      RB cost = " << c4 << endl;
    cout << "   Total cost = " << (c1+c2+c3+c4) << endl; 
    
}

void CornerTree::CreateBlockages( int region )
{
    double regionWidth  = m_btrees[region].outline_width;
    double regionHeight = m_btrees[region].outline_height;

    printf( "Create blockages for region %d (w=%g h=%g)\n", region, regionWidth, regionHeight );
    
    //  It would be better if we remove all old blockages 
    //  if CreateBlockages() is called twice.
   
    // NOTE: The blockages should be added in the non-decreasing Y 
    //       coordinate order.
     
    //vector<int> blockageId;
    
    // add pad blockage
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( !m_modules[i].is_pad )
	    continue;

	double x = m_modules[i].x;
	double y = m_modules[i].y;
	double w = m_modules[i].width;
	double h = m_modules[i].height;

	switch( region )
	{
	    case 0:
		x = x - m_cornerLB.x;
		y = y - m_cornerLB.y;
		break;

	    case 1:
		x = x - m_cornerLT.x;
		y = m_cornerLT.y - y - h;
		break;

	    case 2:
		x = m_cornerRT.x - x - w;
		y = m_cornerRT.y - y - h;
		break;

	    case 3:
		x = m_cornerRB.x - x - w;
		y = y - m_cornerRB.y;
		break;

	    default:
		printf( "ERROR, wrong region id (%d)\n", region );
		exit(0);
	}

	if( x + w < 0 )
	    continue;
	if( y + h < 0 )
	    continue;
	if( x > regionWidth )
	    continue;
	if( y > regionHeight )
	    continue;

	printf( "   Add blockages %s (%g, %g) w=%g h=%g\n", 
		m_modules[i].name, x, y, w, h );
	m_btrees[region].addModule( "B", w, h, x, y, false, true );

    } // end pad blockages
    

    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( m_modules[i].is_pad )
	    continue;

	if( !m_modules[i].create_bkg )
	    continue;

	// Blockage: fixed macro but not dummy macro
	if( m_modules[i].is_fixed && !m_modules[i].is_dummy )
	{
	    double x = m_modules[i].x;
	    double y = m_modules[i].y;
	    double w = m_modules[i].width;
	    double h = m_modules[i].height;

	    switch( region )
	    {
		case 0:
		    x = x - m_cornerLB.x;
		    y = y - m_cornerLB.y;
		    break;
		    
		case 1:
		    x = x - m_cornerLT.x;
		    y = m_cornerLT.y - y - h;
		    break;
		    
		case 2:
		    x = m_cornerRT.x - x - w;
		    y = m_cornerRT.y - y - h;
		    break;
		    
		case 3:
		    x = m_cornerRB.x - x - w;
		    y = y - m_cornerRB.y;
		    break;
		    
		default:
		    printf( "ERROR, wrong region id (%d)\n", region );
		    exit(0);
	    }

	    if( x + w < 0 )
		continue;
	    if( y + h < 0 )
		continue;
	    if( x > regionWidth )
		continue;
	    if( y > regionHeight )
		continue;
	    
	    printf( "   Add blockages %s (%g, %g) w=%g h=%g\n", 
		    m_modules[i].name, x, y, w, h );
	    m_btrees[region].addModule( "B", w, h, x, y, false, true );
	    
	}
    }
    
}

void CornerTree::OutputGNUplotModule( ofstream& outPlt, int i )
{
    double w = m_modules[i].width;
    double h = m_modules[i].height;
    if( m_modulesInfo[i].rotate )
	swap( w, h );
    outPlt << "\t" << m_modules[i].x << ", " << m_modules[i].y << endl;
    outPlt << "\t" << m_modules[i].x + w << ", " << m_modules[i].y << endl;
    outPlt << "\t" << m_modules[i].x + w << ", " << m_modules[i].y + h << endl;
    outPlt << "\t" << m_modules[i].x << ", " << m_modules[i].y + h << endl;
    outPlt << "\t" << m_modules[i].x << ", " << m_modules[i].y << "\n\n";
}

void CornerTree::OutputGNUplot( const char* filename, int level/*=-1*/, bool plotNets/*=false*/ )
{

    cout << "Output gnuplot figure: " << filename << endl;

    ofstream outPlt( filename );
    if( !outPlt )
    {
	printf( "Fail to open output file!\n" );
	exit(-01);
    }
    outPlt << "set xlabel 'width'\n";
    outPlt << "set ylabel 'height'\n";
    outPlt << "set size ratio 1\n";

    // output labels
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	//// Do not show each module in the cluster.
	//if( m_clusterCount>0 && m_matching[i] != -1 )
	//    continue;
	if( i >= m_moduleNumber )   // no pad name
	    break;

	double w, h;
	w = m_modules[i].width;
	h = m_modules[i].height;
	if( m_modulesInfo[i].rotate )
	{
	    swap( w, h );
	}
	char * name = m_modules[i].name;
//	if( strlen(m_modules[i].name) > 5 )
//	    name = m_modules[i].name+strlen(m_modules[i].name) - 5;
	outPlt << "set label '" << name << "' at " 
	    << m_modules[i].x+w*0.5 << "," << m_modules[i].y+h*0.5 << " center\n";
    }

    //outPlt << "plot \"modules.dat\" using 1:2 title \"macros\" with lines lt 1 lw 2\n";

    double length = m_chipWidth;
    if( m_chipHeight > length )
	length = m_chipHeight;
    double minX = -length * 0.1;
    double maxX = length * 1.1; 

    // 1: red
    // 2: green
    // 3: blue
    // 4: pink
    if( level < 0 )
    {
	outPlt << "plot[" << minX << ":" << maxX << "][" << minX <<":" << maxX 
	    << "] '-' title \"core\" w l lt 3 lw 1, "
	    << "'-' title \"cluster\" w l lt 2 lw 2, "
	    << "'-' title \"macros\" w l lt 1 lw 1, "
	    << "'-' title \"fixed\" w l lt 4 lw 1 \n";
    }
    else
    {
	outPlt << "plot[" << minX << ":" << maxX << "][" << minX <<":" << maxX
	    << "] '-' title \"core\" w l lt 3 lw 1, "
	    << "'-' title \"no_group\" w l lt 1 lw 1";

	//assert( level < (int)m_groups.size() );
	//by indark 
	
	int color = 5;
	for(unsigned  int i=0; i<m_groups[level].size(); i++ ) 
	{
	    if( m_groups[level][i].size() <= 1 )
		continue;
	    
	    outPlt << ", '-' title \"" 
		<< getLevelName( m_modules[ m_groups[level][i][0] ].name, level ) 
		<< "\" w l lt " << color << " lw 2";
	    color++;
	}
	outPlt << ", '-' title \"h_nets\" w l lt 1 lw 1 \n";
    }

    // chip region (blue)
    outPlt << "\n# chip region\n";
    outPlt << "\t0, 0\n";
    outPlt << "\t" << m_chipWidth << ", 0\n";
    outPlt << "\t" << m_chipWidth << ", " << m_chipHeight << endl;
    outPlt << "\t0, " << m_chipHeight << endl;
    outPlt << "\t0, 0 \n";
    // std-cell core (blue)
    outPlt << "\n# std-cell core\n";
    outPlt << "\t" << m_stdCoreLeft  << ", " << m_stdCoreBottom << "\n";
    outPlt << "\t" << m_stdCoreRight << ", " << m_stdCoreBottom << "\n";
    outPlt << "\t" << m_stdCoreRight << ", " << m_stdCoreTop    << "\n";
    outPlt << "\t" << m_stdCoreLeft  << ", " << m_stdCoreTop    << "\n";
    outPlt << "\t" << m_stdCoreLeft  << ", " << m_stdCoreBottom << "\n";
    outPlt << "EOF\n";

    double w, h, xOff, yOff;//, x, y;
   
    if( level < 0 )
    {	
	// clusters (green)
	outPlt << "\n# clusters\n";
	outPlt << "\t0, 0\n";
	outPlt << "\n";
	for( int i=(int)m_modules.size()-m_clusterCount; i<(int)m_modules.size(); i++ )
	{
	    OutputGNUplotModule( outPlt, i );
	}
	outPlt << "EOF\n\n";
    }

    
    if( level >= 0 )
    {
	outPlt << "\n# no group macros\n";
	outPlt << "\t0, 0\n";
	outPlt << "\n";
	for( int i=0; i<(int)m_modules.size()/*-m_clusterCount*/; i++ )
	{
	    //if( (int)m_modules[i].group_id.size() < level+1 )
	    {
		OutputGNUplotModule( outPlt, i );
	    }
	}
	outPlt << "EOF\n\n";

	for( int gid=0; gid<(int)m_groups[level].size(); gid++ )
	{
	    if( m_groups[level][gid].size() <= 1 )
		continue;
	    
	    outPlt << "\n# level " << level << " group " << gid << endl; 
	    for( int i=0; i<(int)m_groups[level][gid].size(); i++ )
	    {
		OutputGNUplotModule( outPlt, m_groups[level][gid][i] );
	    }
	    outPlt << "EOF\n\n";
	}

	
        // h-nets
	outPlt << "\n# nets size = " << m_hNets.size() << "\n";
	outPlt << "\t0, 0\n\n";
	double x1, y1, x2, y2;
	if( plotNets )
	{
	    for( int i=0; i<(int)m_hNets.size(); i++ )
	    {
		if( m_hNets.size() <= 1 )
		    continue;

		outPlt << "\t# net " << i << "   pin = " << m_hNets[i].size() << endl;
		assert( m_hNets[i][0] < (int)m_modules.size() );
		GetModuleXY( m_hNets[i][0], x1, y1 );
		for( int j=1; j<(int)m_hNets[i].size(); j++ )
		{
		    assert( m_hNets[i][j] < (int)m_modules.size() );
		    GetModuleXY( m_hNets[i][j], x2, y2 );
		    outPlt << "\t" << x1 << ", " << y1 << endl;
		    outPlt << "\t" << x2 << ", " << y2 << endl << endl;
		}
	    }
	}
	outPlt << "EOF\n\n";
	
    }

    if( level < 0 )
    {
	// macros (red)
	outPlt << "\n# macros\n";
	outPlt << "\t0, 0\n\n";
	for( int i=0; i<(int)m_modules.size()-m_clusterCount; i++ )
	{
	    // Do not show each module in the cluster.
	    //if( m_clusterCount>0 && m_matching[i] != -1 )
	    //    continue;
	    if( m_modules[i].is_fixed )
		continue;

	    w = m_modules[i].width;
	    h = m_modules[i].height;
	    if( m_modulesInfo[i].rotate )
		swap( w, h );
	    outPlt << "\t#w= " << w << " h= " << h << endl;
	    outPlt << "\t" << m_modules[i].x << ", " << m_modules[i].y << endl;
	    outPlt << "\t" << m_modules[i].x + w << ", " << m_modules[i].y << endl;
	    outPlt << "\t" << m_modules[i].x + w << ", " << m_modules[i].y + h << endl;
	    outPlt << "\t" << m_modules[i].x << ", " << m_modules[i].y + h << endl;
	    outPlt << "\t" << m_modules[i].x << ", " << m_modules[i].y << "\n\n";

	    /*
	    // macro-pins
	    int b = 3;
	    for( int j=0; j<(int)m_modules[i].pins.size(); j++ )
	    {
	    GetPinXY( &m_modules[i].pins[j], x, y );
	    //printf( "mod %d pin %d mod %d\n", i, j, (m_modules[i].pins[j]).mod );
	    outPlt << "\t#pin xOff= " << x << " yOff= " << y << endl;
	    outPlt << "\t" << x-b << ", " << y-b << endl;
	    outPlt << "\t" << x-b << ", " << y+b << endl;
	    outPlt << "\t" << x+b << ", " << y+b << endl;
	    outPlt << "\t" << x+b << ", " << y-b << endl;
	    outPlt << "\t" << x-b << ", " << y-b << endl << endl;
	    }*/
	}
	outPlt << "EOF\n\n";
    }

    if( level < 0 )
    {
	// fixed macros (pink)
	outPlt << "\n# fixed macros\n";
	outPlt << "\t0, 0\n";
	outPlt << "\n";
	for( int i=0; i<(int)m_modules.size()-m_clusterCount; i++ )
	{
	    // Do not show each module in the cluster.
	    //if( m_clusterCount>0 && m_matching[i] != -1 )
	    //    continue;
	    if( !m_modules[i].is_fixed )
		continue;
	    //if( m_modules[i].is_dummy )
	    //    continue;	

	    w = m_modules[i].width;
	    h = m_modules[i].height;
	    if( m_modulesInfo[i].rotate )
		swap( w, h );
	    outPlt << "\t# module " << m_modules[i].name << endl;
	    outPlt << "\t" << m_modules[i].x << ", " << m_modules[i].y << endl;
	    outPlt << "\t" << m_modules[i].x + w << ", " << m_modules[i].y << endl;
	    outPlt << "\t" << m_modules[i].x + w << ", " << m_modules[i].y + h << endl;
	    outPlt << "\t" << m_modules[i].x << ", " << m_modules[i].y + h << endl;
	    outPlt << "\t" << m_modules[i].x << ", " << m_modules[i].y << "\n\n";

	    if( i < m_moduleNumber )    // Do not show pad pins.
	    {
		// macro-pins
		int b = 3;
		for( int j=0; j<(int)m_modules[i].pins.size(); j++ )
		{
		    xOff = m_modules[i].pins[j].x;
		    yOff = m_modules[i].pins[j].y;
		    if( m_modulesInfo[i].rotate )
			swap( xOff, yOff );
		    outPlt << "\t#pin xOff= " << xOff << " yOff= " << yOff << endl;
		    outPlt << "\t" << m_modules[i].x + xOff-b << ", " << m_modules[i].y + yOff-b << endl;
		    outPlt << "\t" << m_modules[i].x + xOff-b << ", " << m_modules[i].y + yOff+b << endl;
		    outPlt << "\t" << m_modules[i].x + xOff+b << ", " << m_modules[i].y + yOff+b << endl;
		    outPlt << "\t" << m_modules[i].x + xOff+b << ", " << m_modules[i].y + yOff-b << endl;
		    outPlt << "\t" << m_modules[i].x + xOff-b << ", " << m_modules[i].y + yOff-b << endl << endl;
		}
	    }

	}

	// nets
	if( PLOT_NET )
	{
	    double x1, y1, x2, y2;
	    printf( "m_network.size() = %d\n", m_network.size() );
	    flush( cout );
	    if( (int)m_network.size() < 1000 )
	    {
		printf( "Output network\n" );
		for( int i=0; i<(int)m_network.size(); i++ )
		{
		    outPlt << "\t#net " << i << endl;
		    if( m_network[i].size() > 200 )
			continue;
		    GetPinXY( m_network[i][0], x1, y1 );
		    for( int j=0; j<(int)m_network[i].size(); j++ )
		    {
			GetPinXY( m_network[i][j], x2, y2 );
			outPlt << "\t" << x1 << ", " << y1 << endl;
			outPlt << "\t" << x2 << ", " << y2 << endl << endl;
		    }
		}
	    }
	}
	outPlt << "EOF\n\n";
    } // end fixed macros & nets
    
    outPlt << "pause -1 'press any key'" << endl;

}

void CornerTree::CreateBTrees()
{

    printf( "CornerTree::CreateBTrees()\n" );
    
     double x_center = (0.0 + m_chipWidth) * 0.5;
     double y_center = (0.0 + m_chipHeight) * 0.5;

    double partArea = m_chipWidth * m_chipHeight * 0.25;
    double usedAreas[4] = {0};
    double module_cx, module_cy;
    double w, h;

    if( CUT_Y_LEFT == -1 )
	CUT_Y_LEFT = y_center;
    if( CUT_Y_RIGHT == -1 )
	CUT_Y_RIGHT = y_center;
    /*if( CUT_X_TOP == -1 )
	CUT_X_TOP = x_center;
    if( CUT_X_BOTTOM == -1 )
	CUT_X_BOTTOM = x_center;
    */

    // TODO: handle both CUT_X_TOP & CUT_X_BOTTOM
    if( CUT_X_TOP != -1 )
	x_center = CUT_X_TOP;
    
    // Create m_moduleRegions
    m_modulesRegion.resize( m_modules.size() );
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	// Use center coordinate to determine the region
	
	w = m_modules[i].width;
	h = m_modules[i].height;
	if( m_modulesInfo[i].rotate )
	{
	    swap( w, h );
	}
	module_cx = m_modules[i].x + w * 0.5;
	module_cy = m_modules[i].y + h * 0.5;

	int regionId;
	if( module_cx <= x_center )
	{
	    // left 
	    if( module_cy <= CUT_Y_LEFT )
		regionId = 0;
	    else
		regionId = 1;
	}
	else
	{
	    // right
	    if( module_cy <= CUT_Y_RIGHT )
		regionId = 3;
	    else
		regionId = 2;

	}
	assert( regionId >= 0 && regionId <= 3 );
	//cout << i << " --> " << regionId << endl;
	m_modulesRegion[i] = regionId;
    
	if( i < m_moduleNumber )
	    usedAreas[regionId] += m_modules[i].area;
    }

    for( int i=0; i<4; i++ )
	printf( "Region %d: util = %g / %g = %g\n", 
		i, usedAreas[i], partArea, usedAreas[i] / partArea );
    
    // Create B*-tree and corresponding m_moduleNodeIds 
    // (Node id in the B*-tree sub-floorplan)
    m_btrees = new B_Tree [4]; 
    m_modulesNodeId.resize( m_modules.size() );   
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	cout << m_modules[i].name <<endl;
	if( m_modules[i].is_pad )
	{
	    m_modulesNodeId[i] = -1;
	    cout << m_modules[i].name << "is PAD" <<endl;
	    continue;
	}
	
	// Skip clustered modules (We consider the WHOLE cluster.)
	if( m_matching[i] != -1 && i<(int)m_modules.size()-m_clusterCount )
	{
	    m_modulesNodeId[i] = -1;
	    continue;
	}

        int regionId = m_modulesRegion[i];
	int modId = -1;

	if( m_modules[i].is_fixed && !m_modules[i].is_dummy )
	{
	    cout << "   No packing of macro " << m_modules[i].name << ".\n";
	    m_modulesNodeId[i] = -1;
	    continue;
	}
	if( m_modules[i].is_fixed && !m_modules[i].is_dummy )
	{
	    // Skip fixed macros that outside the core region 
	    
	    bool skip = false;
	    switch( regionId )
	    {
		case 0:	// LB
		    if( m_modules[i].x < m_cornerLB.x || m_modules[i].y < m_cornerLB.y )
		    {
			cout << "x= "<< m_modules[i].x << " y= "<< m_modules[i].y
			    << " xx= " << m_cornerLB.x << " yy= " << m_cornerLB.y << endl;
			skip = true;
		    }
		    break;
		case 1: // LT
		    if( m_modules[i].x < m_cornerLT.x || m_modules[i].y > m_cornerLT.y )
			skip = true;
		    break;
		case 2: // RT
		    if( m_modules[i].x > m_cornerRT.x || m_modules[i].y > m_cornerRT.y )
			skip = true;
		    break;
		case 3: // RB
		    if( m_modules[i].x > m_cornerRB.x || m_modules[i].y < m_cornerRB.y )
		    {
			cout << "x= "<< m_modules[i].x << " y= "<< m_modules[i].y 
			    << " xx= " << m_cornerRB.x << " yy= " << m_cornerRB.y << endl;
			skip = true;
		    }
	    }
	    if( skip )
	    {
		cout << "   Skip the macro " << m_modules[i].name << " (fixed & outside the core region)\n";
		m_modulesNodeId[i] = -1;
		continue;
	    }
	}

	double cx = m_modules[i].x + 0.5*m_modules[i].width;
	double cy = m_modules[i].y + 0.5*m_modules[i].height;
	switch( regionId )
	{
	    case 0:
		cx = cx - m_coreLeft;
		cy = cy - m_coreBottom;
		break;
	    case 1:
		cx = cx - m_coreLeft;
		cy = m_coreTop - cy;
		break;
	    case 2:
		cx = m_coreRight - cx;
		cy = m_coreTop - cy;
		break;
	    case 3:
		cx = m_coreRight - cx;
		cy = cy - m_coreBottom;
	}

	cout << "Adding :" << m_modules[i].name << "To Region" << regionId << endl;
	// add module 
	modId = m_btrees[regionId].addModule( m_modules[i].name, 
		m_modules[i].width, m_modules[i].height,
		cx - m_modules[i].width * 0.5, 
		cy - m_modules[i].height * 0.5, 
		m_modules[i].no_rotate );	

	m_btrees[regionId].modules[modId].sizes = m_modules[i].sizes;
	m_btrees[regionId].modules[modId].currentLayer = m_modules[i].currentLayer;
	m_btrees[regionId].modules[modId].layers = m_modules[i].layers;
	
	m_modulesNodeId[i] = modId;
    }

    // add hierarchy nets
//    CreateHierarchyNets( m_modulesRegion, m_modulesNodeId );
//by indark

    printf( "\n T[0] modules\n" );
    m_btrees[0].show_modules();
    printf( "\n T[1] modules\n" );
    m_btrees[1].show_modules();
    printf( "\n T[2] modules\n" );
    m_btrees[2].show_modules();
    printf( "\n T[3] modules\n" );
    m_btrees[3].show_modules();
   

    printf( "   === set constraint size === %d %d %d %d\n",
	 m_btrees[0].modules_N, m_btrees[1].modules_N, m_btrees[2].modules_N, m_btrees[3].modules_N );
    m_btrees[0].nodesConstraint.resize( m_btrees[0].modules_N );
    m_btrees[1].nodesConstraint.resize( m_btrees[1].modules_N );
    m_btrees[2].nodesConstraint.resize( m_btrees[2].modules_N );
    m_btrees[3].nodesConstraint.resize( m_btrees[3].modules_N );
    
    
    // set outline of the B*-tree
    w = 0.5 * (m_coreRight - m_coreLeft );
    //h = 0.5 * (m_coreTop - m_coreBottom );
    //for( int i=0; i<4; i++ )
    //{
    //	m_btrees[i].outline_width = w;
    //	m_btrees[i].outline_height = h;
    //}
    printf( "   === set bbox for sub-trees ===\n" );
    m_btrees[0].outline_width  = x_center - m_coreLeft;
    m_btrees[0].outline_height = CUT_Y_LEFT - m_coreBottom;
    m_btrees[1].outline_width  = x_center - m_coreLeft;
    m_btrees[1].outline_height = m_coreTop - CUT_Y_LEFT;
    m_btrees[2].outline_width  = m_coreRight - x_center;
    m_btrees[2].outline_height = m_coreTop - CUT_Y_RIGHT;
    m_btrees[3].outline_width  = m_coreRight - x_center;
    m_btrees[3].outline_height = CUT_Y_RIGHT - m_coreBottom;
    for (int i = 0 ; i <= 3 ; i++){
	cout << "CUT_Y_LEFT:" <<CUT_Y_LEFT <<  "\tm_coreBottom:" <<m_coreBottom << "\t";
    	cout <<m_btrees[i].outline_width <<"," <<m_btrees[i].outline_height <<endl;
    	
    }
    printf( "   === SetBTreeConstraints ===\n" );
    SetBTreeConstraints();
}

/// Load module positions from m_btrees
void CornerTree::UpdateModulePositions()
{
    cout << "Update module positions from child B*-trees\n";
    
    // Update macros 
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( m_modules[i].is_pad )   // pad is always fixed
	    continue;
	
	if( m_modules[i].is_fixed )
	{
	    printf( "   No update for the fixed macro '%s'\n", m_modules[i].name );
	    continue;
	}

	//cout << "Update postion for module " << i << endl;
	
	int regionId = m_modulesRegion[i];
	int nodeId   = m_modulesNodeId[i];

	if( nodeId < 0 )
	    continue;	// no corresponding node (maybe clustered)
	
	double x = m_btrees[regionId].getModuleX( nodeId );
	double y = m_btrees[regionId].getModuleY( nodeId );
	
	// for "clusters"
	if( m_modules[i].sizes.size() > 1 )
	{
	    m_modules[i].currentLayer = m_btrees[regionId].modules[nodeId].currentLayer;
	    m_modules[i].width  = m_btrees[regionId].modules[nodeId].width;
	    m_modules[i].height = m_btrees[regionId].modules[nodeId].height;
	}

        double xx = m_modules[i].width;
        double yy = m_modules[i].height;
	
	m_modulesInfo[i].rotate = m_btrees[regionId].getModuleRotate( nodeId );
	if( m_modulesInfo[i].rotate )
	    swap( xx, yy );

	if( regionId == 0 ) // left-bottom
	{
	    m_modules[i].x = m_coreLeft + x;
	    m_modules[i].y = m_coreBottom + y;
	}
	else if( regionId == 1 ) // left-top
	{
	    m_modules[i].x = m_coreLeft + x;
	    m_modules[i].y = m_coreTop - y - yy;
	}
	else if( regionId == 2 ) // right-top
	{
	    m_modules[i].x = m_coreRight - x - xx;
	    m_modules[i].y = m_coreTop - y - yy;
	}
	else // right-bottom
	{
	    m_modules[i].x = m_coreRight - x - xx;
	    m_modules[i].y = m_coreBottom + y;
	}
    }
}

void CornerTree::OutputTemp( const char* filename )
{
    cout << "Output temp file: " << filename << endl;
    ofstream out( filename );
    out << "DIRAREA 0 0 " << m_chipWidth << " " << m_chipHeight << endl;
    out << "count = " << m_modules.size() << endl;
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	double w = m_modules[i].width;
	double h = m_modules[i].height;
	if( m_modulesInfo[i].rotate )
	{
	    swap( w, h );
	}

	out << m_modules[i].name << ": 1 1 1 " 
	    << m_modules[i].x << " " << m_modules[i].y << " "
	    << w << " " << h;
	if( m_modules[i].is_fixed )
	    out << " skip";
	if( m_modules[i].no_rotate )
	    out << " fixed";
	out << endl;

    }
}

void CornerTree::OutputDump( const char* filename )
{
    cout << "Output dump file: " << filename << endl;
    ofstream out( filename );
    out << "; ** MACROS **\n";
    out << "define _cell (geGetEditCell)\n";

    for( int i=0; i<(int)/*m_modules.size()*/m_moduleNumber; i++ )
    {
	if( m_modules[i].is_pad )
	    continue;
	if( m_modules[i].is_extra )
	    continue;
	if( strncmp( m_modules[i].name, "C_", 2 ) == 0 )
	    continue;	// cluster
	if( m_modules[i].is_fixed )
	    continue;
	
	double w = m_modules[i].width;
	double h = m_modules[i].height;
	if( m_modulesInfo[i].rotate )
 		swap( w, h );

	double x = m_modules[i].x;
	double y = m_modules[i].y;
	string degree;
	string flip;
	degree = "0";
	flip = "no";

	if( orientation[i] == "N" || orientation[i] == "FN" )
		degree = "0";
	else if( orientation[i] == "E" || orientation[i] == "FE" )
		degree = "270";
	else if( orientation[i] == "S" || orientation[i] == "FS" )

		degree = "180";
	else if( orientation[i] == "W" || orientation[i] == "FW" )
		degree = "90";
	else
	{
		printf( "ERROR orientation= %s\n", orientation[i].c_str() );
		exit(0);
	}	

	if( orientation[i] == "FN" )
	{
		x += w; 
	}
	else if( orientation[i] == "E" )
	{
		y += h;
	}
	else if( orientation[i] == "FE" )
	{
		y += h;
		x += w;
	}
	else if( orientation[i] == "S" )
	{
		y += h;
		x += w;
	}
	else if( orientation[i] == "FS" )
	{
		y += h;
	}
	else if( orientation[i] == "W" )
	{
		x += w;
	}

	if( orientation[i].substr( 0, 1 ) == "F" )
		flip = "X";

	out << "dbCellInstPlaceAndStatus _cell \"" << m_modules[i].name 
            << "\" \"" << degree   
            << "\" \"" << flip << "\" \"origin\" '("
	    << x << " "
	    << y << ") \"Placed\"\n";
    }


    // output placement blockages
    out << "; ** PLACEMENT BLOCKAGES **\n";
    out << "axPurgePlaceBlockage (geGetEditCell)\n";
    out << "axPurgeSoftPlaceBlockage (geGetEditCell)\n";
    out << "define _cell (geGetEditCell)\n";
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
        if( m_modules[i].is_pad )
	    continue;
        if( strncmp( m_modules[i].name, "C_", 2 ) == 0 )
            continue;   // cluster
	if( m_modules[i].is_dummy )
	    continue;
	if( m_modules[i].is_fixed )
	    continue;

        double w = m_modules[i].width;
        double h = m_modules[i].height;
        if( m_modulesInfo[i].rotate )
                swap( w, h );

        double x = m_modules[i].x;
        double y = m_modules[i].y;

	x -= SPACING;
	y -= SPACING;
	w += 2 * SPACING;
	h += 2 * SPACING;

	out << "define _obj (dbCreateRectangle _cell 221 '((" 
	    << x << " " << y << ")(" << x+w << " " << y+h << ")))\n";	
    }

}

void CornerTree::ClusterModules()
{
    cout << "Cluster modules\n";
    clusterID.resize(m_moduleNumber);
    for(unsigned i=0;i<clusterID.size();i++) clusterID[i] = -1;
    
    //vector<int> matching;
    m_clusterCount = 0;
    m_matching.resize( m_modules.size() );
    for( int i=0; i<(int)m_modules.size(); i++ )
	m_matching[i] = -1;

    for( int i=0; i<(int)m_modules.size(); i++ )    // first block
    {
	if( m_matching[i] != -1 )
	    continue;

	if( m_modules[i].is_fixed == true )  // Do not have to cluster fixed blocks.
	    continue;
	
	double   w = m_modules[i].width;
	double   h = m_modules[i].height;
	int      matchCount = 0;
	set<int> cluster;
	for( int j=i+1; j<(int)m_modules.size(); j++ )	// similar block
	{
	    // TODO: name matching & location matching
	    if( j>i+15 )
		break;	// simulate "name matching"
			// modules with the same hierarchy usually with similar numbers
	    
	    if( w == m_modules[j].width && h == m_modules[j].height ||
		h == m_modules[j].width && w == m_modules[j].height )    
	    {
		//cout << "  " << m_modules[j].name << " == " << m_modules[i].name << endl;

		// levels must the same (2005-11-24)
		bool sameGroup = true;
		for( int l=0; l<=LEVEL; l++ )
		{
		    if( getLevelName( m_modules[j].name, l ) != getLevelName( m_modules[i].name, l ) &&
			    getLevelName( m_modules[j].name, l ) != "" )
		    {
			sameGroup = false;
			break;
		    }
		}
		if( !sameGroup )
		    continue;

		m_matching[j] = i;
		matchCount++;
		cluster.insert( j );
	    }
	}
	if( matchCount > 0 )
	{
	    m_matching[i] = i;
	    cluster.insert( i );
	    matchCount++;

	    // 7 is not easy to cluster. Change it to 6+1.
	    if( matchCount == 7 )
	    {
		set<int>::const_iterator ite = cluster.begin();
		m_matching[*ite] = -1;
		cluster.erase( ite );
	        matchCount--;	
	    }

	    if( matchCount == 5 )   // 2005-11-23: 5 = 4 + 1
	    {
		set<int>::const_iterator ite = cluster.begin();
		m_matching[*ite] = -1;
		cluster.erase( ite );
	        matchCount--;	
	    }

	    if( matchCount == 11 )   // 2005-11-23: 5 = 4 + 1
	    {
		set<int>::const_iterator ite = cluster.begin();
		m_matching[*ite] = -1;
		cluster.erase( ite );
	        matchCount--;	
	    }
	    
	    cout << "  Create cluster: module #= " << matchCount 
		<< " width= " << w << " height= " << h << endl;

	    // list modules
	    set<int>::const_iterator ite;
	    for( ite = cluster.begin(); ite!=cluster.end(); ite++ ){
   	       clusterID[*ite] = m_clusterCount;
		printf( "      %s\n", m_modules[*ite].name );
            }
	    if( w < h )
		swap( w, h );

	    // find candidate sizes
	    vector<size> sizes;
	    vector<int>  layers;
	    int layer;
	    int currentLayer;
	    double new_w, new_h;
	    for( layer=1; layer<=matchCount; layer++ )
	    {
		if( matchCount % layer == 0 )
		{
		    new_h = h * layer;
		    new_w = w * matchCount / layer;
		    printf( "      %d * %d ==> %g * %g\n", 
			    matchCount/layer, layer, new_w, new_h );
		    sizes.push_back( size( new_w, new_h ) );
		    layers.push_back( layer );
		}
	    }
	    currentLayer = 0;
	    new_w = sizes[currentLayer].w;
	    new_h = sizes[currentLayer].h;	
	    //double new_w = w * 2;
	    //double new_h = h * matchCount / 2;

	    // find center of the cluster
	    double cx = 0, cy = 0;
	    for( ite = cluster.begin(); ite!=cluster.end(); ite++ )
	    {
		cx += m_modules[*ite].x + m_modules[*ite].width * 0.5;
		cy += m_modules[*ite].y + m_modules[*ite].height * 0.5;
	    }
	    cx /= matchCount;
	    cy /= matchCount;
	    double x = cx - new_w * 0.5;
	    double y = cy - new_h * 0.5;
	    cout << "      New width= " << new_w << " height= " << new_h << " at (" 
		<< x << "," << y << ")\n";
	    printf( "      center( %f, %f )\n", cx, cy );

	    Module mod;
	    sprintf( mod.name, "C_%s", m_modules[i].name );
	    mod.width = new_w;
	    mod.height = new_h;
	    mod.area = new_w * new_h;
	    mod.x = x;
	    mod.y = y;
	    mod.no_rotate = false;
	    mod.is_fixed = false;
	    mod.sizes = sizes;
	    mod.layers = layers;
	    mod.currentLayer = currentLayer;

	    // 2005-11-22
	    mod.group_id = m_modules[i].group_id;
	    int moduleId = (int)m_modules.size();
	    for( int level=0; level<(int)mod.group_id.size(); level++ )
	    {
		m_groups[level][ mod.group_id[level] ].push_back( moduleId );
	    }
	    
	    m_modules.push_back( mod );
	    m_matching.push_back( i );
	    m_clusterCount++;

	}// end cluster operation
    }

    // resize all related arrays
    m_matching.resize( m_modules.size(), -1 );
    m_modulesNodeId.resize( m_modules.size() );
    m_modulesRegion.resize( m_modules.size() );
    m_modulesInfo.resize( m_modules.size() );
    for( int i=((int)m_modules.size()-m_clusterCount); i<(int)m_modules.size(); i++ )
    {
	m_modulesInfo[i].rotate = false;
	m_modulesInfo[i].flip = false;
    }	

}

void CornerTree::DeclusterModules()
{
    cout << "Decluster modules\n";
    
    cout << "   " << m_clusterCount << " clusters\n";
    
    for( int i=m_modules.size()-m_clusterCount; i<(int)m_modules.size(); i++ )
    {
	// find cluster size (TODO: Use a table to store cluster size when clustering.)
	int count = 0;
	int id = m_matching[i];
	for( int j=id; j<(int)m_modules.size()-m_clusterCount; j++ )
	{
	    if( m_matching[j] == id )
	       count++;
	}

	double w = m_modules[i].width;	// cluster width
	double h = m_modules[i].height; // cluster height
	
	int layer = m_modules[i].layers[ m_modules[i].currentLayer ];

	printf( "   Cluster '%d' has %d modules (layer = %d) (%d)\n",
	            id, count, layer, m_modulesInfo[i].rotate );
	
	h = h / layer;			// height for one macro
	w = w / (count / layer);	// width for one macro
	
	//w /= 2;			// width for one macro
	//h /= (count/2);		// height for one macro
	
	if( m_modulesInfo[i].rotate )
	    swap( w, h );
	double x = m_modules[i].x;
	double y = m_modules[i].y;
	
	// set macro locations
	int updateCount = 0;
        for( int j=id; j<(int)m_modules.size()-m_clusterCount; j++ )
        {
	    if( m_matching[j] == id )
	    {
		if( !m_modulesInfo[i].rotate )
		{
		    //  ------- --------
		    //  ------- --------
		    //  ------- --------
		    
		    if( m_modules[j].width-w > 1e-5 || m_modules[j].height-h > 1e-5 )
			m_modulesInfo[j].rotate = true;
		    else
			m_modulesInfo[j].rotate = false;
		    
		    m_modules[j].y = y + updateCount % layer * h;
		    m_modules[j].x = x + (int)floor( (double)updateCount / layer) * w;

		    //printf( "      %d (%g, %g)\n", updateCount, m_modules[j].x, m_modules[j].y );
		    updateCount++;
		}
		else
		{
		    //  | | |
		    //  | | |
		    //       
		    //  | | |
		    //  | | |
		    
		    if( m_modules[j].width-w > 1e-5 || m_modules[j].height-h > 1e-5 )
			m_modulesInfo[j].rotate = true;
		    else
			m_modulesInfo[j].rotate = false;
		
		    m_modules[j].y = y + (int)floor((double)updateCount / layer) * h;
		    m_modules[j].x = x + updateCount % layer * w;

		    //printf( "      %d (%g, %g)\n", updateCount, m_modules[j].x, m_modules[j].y );
		    //m_modules[j].x = x + updateCount/2 * w;
		    //m_modules[j].y = y;
		    //if( updateCount % 2 == 1 )
		    //	m_modules[j].y += h;
		    updateCount++;
		}
	    }
	}// for each block
				
	
    }

    
}

// added by unisun, 2005-07-24
void CornerTree::AddSpacing(){
  for(int i=0;i<(int)m_modules.size();i++){
    if( m_modules[i].is_fixed )
	continue;
    m_modules[i].width += 2 * SPACING;
    m_modules[i].height += 2 * SPACING;
    m_modules[i].x -= SPACING;
    m_modules[i].y -= SPACING;
  }
}

void CornerTree::RestoreSpacing(){
  for(int i=0;i<(int)m_modules.size();i++){
    if( m_modules[i].is_fixed )
	continue;
    //cout << "module " << i << endl;
    m_modules[i].width -= 2*SPACING;
    m_modules[i].height -= 2*SPACING;
    m_modules[i].x += SPACING;
    m_modules[i].y += SPACING;
    //cout << m_modules[i].x << " " << m_modules[i].y << " " << m_modules[i].width << " " << m_modules[i].height << endl;
  }
}

void CornerTree::GetModuleXY( int m, double& x, double& y )
{
    double w = m_modules[m].width;
    double h = m_modules[m].height;
    if( m_modulesInfo[m].rotate )
	swap( w, h );
    x = m_modules[m].x + 0.5 * w;
    y = m_modules[m].y + 0.5 * h;
}
//indark 2006-04-15
void CornerTree::GetModuleRotate( int m, bool& rotate )
{
	rotate = m_modulesInfo[m].rotate ;
}

void CornerTree::GetPinXY( Pin* p, double& x, double& y )
{
    int moduleId = p->mod;
    double xx = m_modules[moduleId].x;
    double yy = m_modules[moduleId].y;
    double xOff = p->x;
    double yOff = p->y;
    if( m_modulesInfo[moduleId].rotate )
    {
	swap( xOff, yOff );
    }
    x = xx + xOff;
    y = yy + yOff;
}

void UpdateCoordinates(Modules& m_modules, Modules& gm, vector<int>& pos, string type){
  unsigned i;
  assert(gm.size() == pos.size());
  cout << "type = " << type << endl;
  for(i=0;i<gm.size();i++){
    int id = pos[i];
    char * name = m_modules[id].name;
        if( strlen(m_modules[id].name) > 5 )
        name = m_modules[id].name+strlen(m_modules[id].name) - 5;
    cout << "name = " << name << endl;
    cout << "pre\t" << m_modules[id].x << " " << m_modules[id].y << endl;
    if (type == "H")
      m_modules[id].x = gm[i].x;
    else 
      m_modules[id].y = gm[i].y;
    cout << "after\t" << m_modules[id].x << " " << m_modules[id].y << endl;
  }
}


void CornerTree::Compaction(){
  string orig = "before_compaction.plt";
  string aft = "after_compaction.plt";
  Modules Hgm, Vgm;
  vector<int> Hpos, Vpos;
  unsigned i;
  double x_center = (0.0 + m_chipWidth) * 0.5;
  double w, h, module_cx, module_cy;
  
  regions.resize(m_moduleNumber);
  for(i=0;i<(unsigned)m_moduleNumber;i++) regions[i] = 0;

  for(i=0;i<(unsigned)m_moduleNumber;i++){
    w = m_modules[i].width;
    h = m_modules[i].height;
    if( m_modulesInfo[i].rotate )
    {
      swap( w, h );
    }
    module_cx = m_modules[i].x + w * 0.5;
    module_cy = m_modules[i].y + h * 0.5;

    int regionId;
    if( module_cx <= x_center )
    {
	    // left 
	    if( module_cy <= CUT_Y_LEFT )
		regionId = 0;
	    else
		regionId = 1;
    }
    else
    {
        // right
        if( module_cy <= CUT_Y_RIGHT )
            regionId = 3;
        else
            regionId = 2;
    }
    assert( regionId >= 0 && regionId <= 3 );
    regions[i] = regionId;
  }  
  
  OutputGNUplot(orig.c_str());
  
  for(i=0;i<4;i++){
    int count = 0;
    while(1){
      Hgm.clear(); Vgm.clear(); Hpos.clear(); Vpos.clear();
      double Hdis = DimensionalCompaction(Hgm, i, "H", Hpos);
      double Vdis = DimensionalCompaction(Vgm, i, "V", Vpos);
      if (Hdis == 0.0 && Vdis != 0){
        UpdateCoordinates(m_modules, Vgm, Vpos, "V");
        DeclusterModules();
      }else if (Hdis != 0 && Vdis == 0.0){
        UpdateCoordinates(m_modules, Hgm, Hpos, "H");
        DeclusterModules();
      }else if (Hdis <= Vdis && Hdis != 0.0 && Vdis != 0.0){
        UpdateCoordinates(m_modules, Hgm, Hpos, "H");
        DeclusterModules();
        Vdis = DimensionalCompaction(Vgm, i, "V", Vpos);
        UpdateCoordinates(m_modules, Vgm, Vpos, "V");
        DeclusterModules(); 
      }else if (Hdis > Vdis){
        UpdateCoordinates(m_modules, Vgm, Vpos, "V");
        DeclusterModules();
        Hdis = DimensionalCompaction(Hgm, i, "H", Hpos);
        UpdateCoordinates(m_modules, Hgm, Hpos, "H");
        DeclusterModules();
      }else break;
      count++;
      if (count == 10){
        cout << "too many times!!!" << endl;
        break;
      }
    }
  }
  
  OutputGNUplot(aft.c_str());
}

double CornerTree::DimensionalCompaction(Modules& gm, int regionID, string type, vector<int>& pos){
  assert(type == "H" || type == "V");
  assert(regionID >= 0 && regionID <= 3);
  double dis = 0;
  
  //vector<int> pos, already_get;
  vector<int> already_get;
  already_get.resize(m_moduleNumber);
  //Modules gm;
  unsigned i;
  for(i=0;i<(unsigned)m_moduleNumber;i++) already_get[i] = 0;
  
  pos.clear();
  gm.clear();
  int region_count = 0;
  for(i=0;i<m_modules.size();i++){
    if (i < (unsigned)m_moduleNumber){
      char * name = m_modules[i].name;
        if( strlen(m_modules[i].name) > 5 )
          name = m_modules[i].name+strlen(m_modules[i].name) - 5;
      if (regions[i] == regionID) region_count++;
    }
    if (regions[i] == regionID && i < (unsigned)m_moduleNumber){
      if (clusterID[i] != -1 && already_get[clusterID[i]] == 0){
        //int id = m_matching[i];
        int id = m_modules.size()-m_clusterCount+clusterID[i];
        char * name = m_modules[id].name;
        if( strlen(m_modules[id].name) > 5 )
        name = m_modules[id].name+strlen(m_modules[id].name) - 5;
        double w = m_modules[id].width, h = m_modules[id].height;
        if (m_modulesInfo[id].rotate) swap(w, h);
        Module mod = m_modules[id];
        gm.push_back(mod); pos.push_back(id);
        if (m_modulesInfo[id].rotate) swap(gm[gm.size()-1].width, gm[gm.size()-1].height);
        already_get[clusterID[i]] = 1;
        if (m_modules[id].is_fixed) gm[gm.size()-1].is_fixed = 1;
        else gm[gm.size()-1].is_fixed = 0;
      }else if (clusterID[i] == -1){
        char * name = m_modules[i].name;
        if( strlen(m_modules[i].name) > 5 )
          name = m_modules[i].name+strlen(m_modules[i].name) - 5;
        double w = m_modules[i].width, h = m_modules[i].height;
        if (m_modulesInfo[i].rotate) swap(w, h);
        Module mod = m_modules[i];
        gm.push_back(mod); pos.push_back(i);
        if (m_modulesInfo[i].rotate) swap(gm[gm.size()-1].width, gm[gm.size()-1].height);
        if (m_modules[i].is_fixed) gm[gm.size()-1].is_fixed = 1;
        else gm[gm.size()-1].is_fixed = 0;
      }
    }
  }
  for(i=0;i<gm.size();i++){
    int n = pos[i];
    if (regionID == 0){
      gm[i].x -= m_coreLeft;
      gm[i].y -= m_coreBottom;
    }else if (regionID == 1){
      gm[i].x -= m_coreLeft;
      gm[i].y = -gm[i].y + m_coreTop - gm[i].height;
    }else if (regionID == 2){
      gm[i].x = -gm[i].x + m_coreRight - gm[i].width;
      gm[i].y = -gm[i].y + m_coreTop - gm[i].height;
      char * name = m_modules[n].name;
        if( strlen(m_modules[n].name) > 5 )
        name = m_modules[n].name+strlen(m_modules[n].name) - 5;
    }else{
      gm[i].x = -gm[i].x + m_coreRight - gm[i].width;
      gm[i].y -= m_coreBottom;
    }
    
  }
  
//   GRAPH G;
//   G.Longest_Path(gm, type);
  
  for(i=0;i<gm.size();i++){
    int n = pos[i];
    
    char * name = m_modules[n].name;
    if( strlen(m_modules[n].name) > 5 )
      name = m_modules[n].name+strlen(m_modules[n].name) - 5;

    
    if (regionID == 0){
      gm[i].x = m_coreLeft + gm[i].x;
      gm[i].y = m_coreBottom + gm[i].y;
    }else if (regionID == 1){
      gm[i].x = gm[i].x + m_coreLeft;
      gm[i].y = m_coreTop - gm[i].y - gm[i].height;
    }else if (regionID == 2){
      gm[i].x = m_coreRight - gm[i].x - gm[i].width;
      gm[i].y = m_coreTop - gm[i].y - gm[i].height;
    }else if (regionID == 3){
      gm[i].x = m_coreRight - gm[i].x - gm[i].width;
      gm[i].y = m_coreBottom + gm[i].y;
    }
    if (type == "H"){
      double temp = fabs(gm[i].x - m_modules[n].x);
      dis += pow(temp, 1.5);
    }else{
      double temp = fabs(gm[i].y - m_modules[n].y);
      dis += pow(temp, 1.5);
    }
  }
  return dis;
}

void CornerTree::ModuleFlip()
{

  //////////////////////////////////////////////
  unsigned i;
  double x_center = (0.0 + m_chipWidth) * 0.5;
  double w, h, module_cx, module_cy;

  regions.resize(m_moduleNumber);
  for(i=0;i<(unsigned)m_moduleNumber;i++) regions[i] = 0;

  for(i=0;i<(unsigned)m_moduleNumber;i++){
    w = m_modules[i].width;
    h = m_modules[i].height;
    if( m_modulesInfo[i].rotate )
    {
      swap( w, h );
    }
    module_cx = m_modules[i].x + w * 0.5;
    module_cy = m_modules[i].y + h * 0.5;

    int regionId;
    if( module_cx <= x_center )
    {
            // left
            if( module_cy <= CUT_Y_LEFT )
                regionId = 0;
            else
                regionId = 1;
    }
    else
    {
        // right
        if( module_cy <= CUT_Y_RIGHT )
            regionId = 3;
        else
            regionId = 2;
    }
    assert( regionId >= 0 && regionId <= 3 );
    regions[i] = regionId;
  }
  /////////////////////////////////////////////


    //unsigned i;
    unsigned j;
    int regionID;
    for(i=0;i<m_modules.size(); i++){
	//if (!m_modules[i].is_pad){
	if (i < (unsigned)m_moduleNumber ){
	    m_modulesInfo[i].flip = false;
	    //regionID = m_modulesRegion[i];
	    regionID = regions[i];
	    //int rotate = m_btrees[regionID].getModuleRotate(i);
	    int rotate = m_modulesInfo[i].rotate;
	    double xx = m_modules[i].width, yy = m_modules[i].height;
	    if (rotate){
		swap(xx, yy);
		for(j=0;j<m_modules[i].pins.size();j++){
		    Pin *p = &m_modules[i].pins[j];

		    double pre_px = p->x;
		    p->x = p->y; p->y = yy - pre_px;

		}
		if (orientation[i] == "N") orientation[i] = "E";
		else if (orientation[i] == "E") orientation[i] = "S";
		else if (orientation[i] == "S") orientation[i] = "W";
		else if (orientation[i] == "W") orientation[i] = "N";
		else if (orientation[i] == "FN") orientation[i] = "FW";
		else if (orientation[i] == "FW") orientation[i] = "FS";
		else if (orientation[i] == "FS") orientation[i] = "FE";
		else if (orientation[i] == "FE") orientation[i] = "FN";
	    }
	    int out_num_x = 0, in_num_x = 0;
	    int out_num_y = 0, in_num_y = 0;
	    for(j=0;j<m_modules[i].pins.size();j++){
		Pin *p = &m_modules[i].pins[j];
		if (regionID == 0){
		    if (p->x >= xx*0.5) out_num_x++;
		    else in_num_x++;
		    if (p->y >= yy*0.5) out_num_y++;
		    else in_num_y++;
		}else if (regionID == 1){
		    if (p->x >= xx*0.5) out_num_x++;
		    else in_num_x++;
		    if (p->y <= yy*0.5) out_num_y++;
		    else in_num_y++;
		}else if (regionID == 2){
		    if (p->x <= xx*0.5) out_num_x++;
		    else in_num_x++;
		    if (p->y <= yy*0.5) out_num_y++;
		    else in_num_y++;
		}else{
		    if (p->x <= xx*0.5) out_num_x++;
		    else in_num_x++;
		    if (p->y >= yy*0.5) out_num_y++;
		    else in_num_y++;
		}
	    }
	    if (in_num_x >= out_num_x){ // x-flip module
		char * name = m_modules[i].name;
		if( strlen(m_modules[i].name) > 5 )
		    name = m_modules[i].name+strlen(m_modules[i].name) - 5;
		m_modulesInfo[i].flip = 1;
		for(j=0;j<m_modules[i].pins.size();j++){
		    Pin *p = &m_modules[i].pins[j];
		    p->x = xx - p->x;
		}
		if (orientation[i] == "N") orientation[i] = "FN";
		else if (orientation[i] == "E") orientation[i] = "FE";
		else if (orientation[i] == "S") orientation[i] = "FS";
		else if (orientation[i] == "W") orientation[i] = "FW";
		else if (orientation[i] == "FN") orientation[i] = "N";
		else if (orientation[i] == "FW") orientation[i] = "W";
		else if (orientation[i] == "FS") orientation[i] = "S";
		else if (orientation[i] == "FE") orientation[i] = "E";
	    }

	    if (in_num_y >= out_num_y){ // y-flip module
		m_modulesInfo[i].flip = 1;
		for(j=0;j<m_modules[i].pins.size();j++){
		    Pin *p = &m_modules[i].pins[j];
		    p->y = yy - p->y;
		}
		if (orientation[i] == "N") orientation[i] = "FS";
		else if (orientation[i] == "E") orientation[i] = "FW";
		else if (orientation[i] == "S") orientation[i] = "FN";
		else if (orientation[i] == "W") orientation[i] = "FE";
		else if (orientation[i] == "FN") orientation[i] = "S";
		else if (orientation[i] == "FW") orientation[i] = "E";
		else if (orientation[i] == "FS") orientation[i] = "N";
		else if (orientation[i] == "FE") orientation[i] = "W";
	    }
	}
    }
}

void N_ntumptree::CornerTree::OffsetRelocate( )
{
	for(unsigned int i = 0 ; i < m_modules.size() ; i++){
		m_modules[i].x -= m_offset_X;
		m_modules[i].cx -= m_offset_X;
		m_modules[i].y -= m_offset_Y;
		m_modules[i].cy -= m_offset_Y;
		

	}
}

void N_ntumptree::CornerTree::OffsetRestore( )
{
	for(unsigned int i = 0 ; i < m_modules.size() ; i++){
		m_modules[i].x += m_offset_X;
		m_modules[i].cx += m_offset_X;
		m_modules[i].y += m_offset_Y;
		m_modules[i].cy += m_offset_Y;
		

	}
}
