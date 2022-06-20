////////////////////////////////////////////////////
// Author: Tung-Chieh Chen
//
// Hisotry: 2005-09-07	LoadSettings()
// 
////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>
using namespace std;


#include "CornerTree.h"
#include "mptreeutil.h"
using namespace N_ntumptree;
void CornerTree::LoadSettings( const char* filename )
{
    printf( "Load settings from %s\n", filename );
    ifstream in( filename );

    char line[5000];
    char tmp0[500], tmp1[500], tmp2[500], tmp3[500], tmp4[500], tmp5[500];
    
    while( true )
    {
	line[0] = tmp0[0] = tmp1[0] = tmp2[0] = tmp3[0] = tmp4[0] = tmp5[0] = '\0';
	in.getline( line, 4999 );
	line[4999] = '\0';

	if( line[0] == '#' )
	    continue;

	int count = sscanf( line, "%s %s %s %s %s %s\n", tmp0, tmp1, tmp2, tmp3, tmp4, tmp5 );

	if( count <= 0 && in.eof() )
	    break;

	if( strcmp( tmp0, "" ) == 0 )
	{
	    continue;
	}
	else if( strcmp( tmp0, "LEVEL" ) == 0 )
	{
	    LEVEL = atoi( tmp1 );
	    printf( "  LEVEL = %d\n", LEVEL );
	}
	else if( strcmp( tmp0, "SPACING" ) == 0 )
	{
	    SPACING = atof( tmp1 );
	    printf( "  SPACING = %g\n", SPACING );
	}
	else if( strcmp( tmp0, "SA" ) == 0 )
	{
	    if( strcmp( tmp1, "TIMES" ) == 0)
	    {
		SA_TIMES = atoi( tmp2 );
		printf( "  SA_TIMES = %d\n", SA_TIMES );
	    }
	    else if( strcmp( tmp1, "LOCAL" ) == 0 )
	    {
		SA_LOCAL = atoi( tmp2 );
		printf( "  SA_LOCAL = %d\n", SA_LOCAL );
	    }
	    else if( strcmp( tmp1, "MAXITE" ) == 0 )
	    {
		SA_MAX_ITE = atoi( tmp2 );
		printf( "  SA_MAX_ITE = %d\n", SA_MAX_ITE );
	    }
	    else
	    {
		printf( "  Parse error: %s\n", line );
		exit(-1);
	    }
	}
	else if( strcmp( tmp0, "WEIGHT" ) == 0 )
	{
	    if( strcmp( tmp1, "WASTE" ) == 0 )
	    {
		WEIGHT_WASTE = atof( tmp2 );
		printf( "  WEIGHT_WASTE = %g\n", WEIGHT_WASTE );
	    }
	    else if( strcmp( tmp1, "BOUNDARY" ) == 0 )
	    {
		WEIGHT_BOUNDARY = atof( tmp2 );
		printf( "  WEIGHT_BOUNDARY = %g\n", WEIGHT_BOUNDARY );
	    }
	    else if( strcmp( tmp1, "DISPLACEMENT" ) == 0 )
	    {
		WEIGHT_DISPLACEMENT = atof( tmp2 );
		printf( "  WEIGHT_DISPLACEMENT = %g\n", WEIGHT_DISPLACEMENT );
	    }
	    else if( strcmp( tmp1, "ORIENTATION" ) == 0 )
	    {
		WEIGHT_ORIENTATION = atof( tmp2 );
		printf( "  WEIGHT_ORIENTATION = %g\n", WEIGHT_ORIENTATION );
	    }
	    else if( strcmp( tmp1, "THICKNESS" ) == 0 )
	    {
		WEIGHT_THICKNESS = atof( tmp2 );
		printf( "  WEIGHT_THICKNESS = %g\n", WEIGHT_THICKNESS );
	    }
	    else if( strcmp( tmp1, "WIRE" ) == 0 )
	    {
		WEIGHT_WIRE = atof( tmp2 );
		printf( "  WEIGTH_WIRE = %g\n", WEIGHT_WIRE );
	    }
	    else
	    {
		printf( "  Parse error: %s\n", line );
		exit(-01);
	    }
	}
	else if( strcmp( tmp0, "CORNER" ) == 0 )
	{
	    int id = GetModuleIdByName( tmp1 );
	    m_modules[id].is_corner = true;
	    printf( "  CORNER (%d) %s\n", id, tmp1 );

	    if( id < 0 )
	    {
		printf( "ERROR.\n" );
		exit(0);
	    }

	    // Create dummy nodes if it is fixed. (donnie, 2005-10-06)
	    if( m_modules[id].is_fixed )
	    {
		printf( "   Create a dummy node for block %d\n", id );
		m_modules[id].is_dummy = true;
	    }
	    
	}
	else if( strcmp( tmp0, "FIXED" ) == 0 ) // 2005-10-03
	{
	    int id = GetModuleIdByName( tmp1 );
	    if( id < 0 )
	    {
		printf( "ERROR.\n" );
		exit(0);
	    }
	    m_modules[id].is_fixed = true;
	    printf( "  FIXED (%d) %s\n", id, tmp1 );
	}
	else if( strcmp( tmp0, "DUMMY" ) == 0 ) // 2005-10-06
	{
	    int id = GetModuleIdByName( tmp1 );
	    if( id < 0 )
	    {
		printf( "ERROR.\n" );
		exit(0);
	    }
	    m_modules[id].is_dummy = true;
	    printf( "  DUMMY (%d) %s\n", id, tmp1 );
	}	   
        else if( strcmp( tmp0, "BLOCK" ) == 0) // 2005-10-07
	{
	    double x = atof( tmp2 );
	    double y = atof( tmp3 );
	    double w = atof( tmp4 );
	    double h = atof( tmp5 );
	    Module m;
	    m_modules.push_back( m );
	    Module& mod = m_modules.back();
	    //char name[200];
	    //sprintf( name, "BKG%d", m_nBlockages );
	    //strcpy( mod.name, name );
	    strcpy( mod.name, tmp1 );
	    mod.x = x;
	    mod.y = y;
	    mod.width  = w;
	    mod.height = h;
	    mod.no_rotate = true;
	    mod.is_fixed = true;
	    mod.is_extra = true; 
	    mod.id = m_modules.size() - 1;
	    //m_moduleNumber++;	///// TEST
 	    m_modulesInfo.resize( m_moduleNumber ); 
	    m_modulesInfo[m_moduleNumber-1].rotate = false;
	    m_modulesInfo[m_moduleNumber-1].flip   = false;
	    printf( "  BLOCKAGE %s (%g, %g) w= %g h= %g\n", tmp1, x, y, w, h );
	}	   
        else if( strcmp( tmp0, "NOBKG" ) == 0 )
	{
	    int id = GetModuleIdByName( tmp1 );
	    if( id < 0 )
	    {
		printf( "ERROR.\n" );
		exit(0);
	    }
	    m_modules[id].create_bkg = false;
	    printf( "  NOBKG (%d) %s\n", id, tmp1 );
	}	    
	else if( strcmp( tmp0, "X" ) == 0 || strcmp( tmp0, "Y" ) == 0 )
	{
	    node_relation::relation type = node_relation::X;
	    if( strcmp( tmp0, "Y" ) == 0 )
		type = node_relation::Y;	// right child (Y)
	    int parentId = GetModuleIdByName( tmp1 );
	    int childId  = GetModuleIdByName( tmp2 );
	    printf( "  %s p(%d) --> c(%d)\n", tmp0, parentId, childId );
	    node_relation r( type, parentId, childId );
	    m_moduleRelations.push_back( r );
	    
	    if( parentId < 0 )
	    {
		printf( "ERROR.\n" );
		exit(0);
	    }
	    
	    // Dummy nodes (donnie, 2005-10-06)
	    if( m_modules[parentId].is_fixed && m_modules[parentId].is_dummy == false ) 
	    {
		printf( "   Create a dummy node for block %d\n", parentId );
		m_modules[parentId].is_dummy = true;
	    }
	    if( childId > 0 && m_modules[childId].is_fixed ) 
	    {
		printf( "   Create a dummy node for block %d\n", childId );
		m_modules[childId].is_dummy = true;
	    }
	    
	}
	else if( strcmp( tmp0, "MAX_ASPECT_RATIO" ) == 0 )
	{
	    MAX_ASPECT_RATIO = atof( tmp1 );
	    printf( "  MAX_ASPECT_RATIO = %g\n", MAX_ASPECT_RATIO );

	    for( int i=0; i<m_moduleNumber; i++ )
	    {
		if( m_modules[i].width / m_modules[i].height > MAX_ASPECT_RATIO )
		{
		    printf( "    Macro %d AR = %g\n", i, m_modules[i].width / m_modules[i].height );
		}
		if( m_modules[i].height / m_modules[i].width > MAX_ASPECT_RATIO )
		{
		    printf( "    Macro %d AR = %g\n", i, m_modules[i].height / m_modules[i].width );
		}
	    }
	}
	else if( strcmp( tmp0, "CUT" ) == 0 )
	{
	    if( strcmp( tmp1, "L" ) == 0 )
	    {
		CUT_Y_LEFT = atof( tmp2 );
		printf( "  CUT_Y_LEFT = %g\n", CUT_Y_LEFT );
	    }
	    else if( strcmp( tmp1, "R" ) == 0 )
	    {
		CUT_Y_RIGHT = atof( tmp2 );
		printf( "  CUT_Y_RIGHT = %g\n", CUT_Y_RIGHT );
	    }
	    else if( strcmp( tmp1, "T" ) == 0 )
	    {
		CUT_X_TOP = atof( tmp2 );
		printf( "   CUT_X_TOP = %g\n", CUT_X_TOP );
	    }
	    else if( strcmp( tmp1, "B" ) == 0 )
	    {
		CUT_X_BOTTOM = atof( tmp2 );
		printf( "   CUT_X_BOTTOM = %g\n", CUT_X_BOTTOM );
	    }
	    else
	    {
		printf( "syntax error: %s\n", line );
		exit(-1);
	    }
	}
	else if( strcmp( tmp0, "OPT_SEQ" ) == 0 )
	{
	    OPT_SEQ.clear();
	    OPT_SEQ.push_back( atoi( tmp1 ) );
	    OPT_SEQ.push_back( atoi( tmp2 ) );
	    OPT_SEQ.push_back( atoi( tmp3 ) );
	    OPT_SEQ.push_back( atoi( tmp4 ) );
	    printf( "  OPT_SEQ = %d %d %d %d\n", OPT_SEQ[0], OPT_SEQ[1], OPT_SEQ[2], OPT_SEQ[3] );
	}
	else
	{
	    printf( "  Parse error: %s\n", line );
	    exit(-01);
	}
	
	if( in.eof() )
	    break;
    }
}

int CornerTree::GetModuleIdByName( const char* name )
{
    // 2005-09-07 Linear search
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( strcmp( name, m_modules[i].name ) == 0 )
	    return i;
    }
    printf( "Module name '%s' not found\n", name );
    return -1;
}

void CornerTree::SetBTreeConstraints()
{
    printf( "Set B*-tree constraints\n" );

    vector<bool> hasCorner;
    hasCorner.resize( 4, false );
    
    // Handle preplaced root (corner block)
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	if( m_modules[i].is_corner == true )
	{
	    int region = m_modulesRegion[i];
	    printf( "  R%d root (%s)\n", region, m_modules[i].name );
	    assert( region < 4 );
	    int node = m_modulesNodeId[i];
	    m_btrees[region].SetPreplacedRoot( node );
	    hasCorner[region] = true;
	    
	    // 2005-11-17
	    // Modify corner block width/height
	    if( m_modules[i].is_fixed )
	    {
		double x_diff = m_btrees[region].modules[node].x - 0;
		double y_diff = m_btrees[region].modules[node].y - 0;
		printf( " >>> CORNER ADJUSTMENT: region %d  x_diff %f  y_diff %f <<< \n", region, x_diff, y_diff );
		m_btrees[region].modules[node].x = 0;
		m_btrees[region].modules[node].y = 0;
		m_btrees[region].modules[node].width += x_diff;
		m_btrees[region].modules[node].height += y_diff;
		m_btrees[region].modules[node].area = m_btrees[region].modules[node].width *
		    m_btrees[region].modules[node].height;
	    }
	}
    }

    // TODO: automatic detect corner block (??)
    
    // Handle XY relations
    for( int i=0; i<(int)m_moduleRelations.size(); i++ )
    {
	int parentRegion = m_modulesRegion[ m_moduleRelations[i].parent ];
	int parentNode = m_modulesNodeId[ m_moduleRelations[i].parent ];

	if( m_moduleRelations[i].child == -1 )	    // NIL, use "noLeft" & "noRight"
	{
	    if( m_moduleRelations[i].type == node_relation::X )
	    {
		m_btrees[parentRegion].nodesConstraint[parentNode].noLeft = true;
		printf( "  R%d p(%d) --> LEFT_NIL\n", parentRegion, parentNode );
	    }
	    else
	    {
		m_btrees[parentRegion].nodesConstraint[parentNode].noRight = true;
		printf( "  R%d p(%d) --> RIGHT_NIL\n", parentRegion, parentNode );
	    }
	    continue;
	}
	
	int childRegion  = m_modulesRegion[ m_moduleRelations[i].child ];
	if( parentRegion != childRegion )
	{
	    printf( "Error, parent region is not the same as child region! (%d) --> (%d)\n",
		    m_moduleRelations[i].parent, m_moduleRelations[i].child );
	    exit(-01);
	}
	
	int childNode  = m_modulesNodeId[ m_moduleRelations[i].child ];
	m_btrees[parentRegion].AddNodeRelation( m_moduleRelations[i].type, 
					parentNode, childNode );
        printf( "  R%d p(%d) --> c(%d)\n", parentRegion, parentNode, childNode );

	// adjust block size for fixed-fixed constraint
	if( m_modules[ m_moduleRelations[i].parent ].is_fixed &&
		m_modules[ m_moduleRelations[i].child ].is_fixed )
	{
	    if( m_moduleRelations[i].type == node_relation::X )
	    {
		int region = parentRegion;
		double parent_rx = m_btrees[region].modules[parentNode].x + m_btrees[region].modules[parentNode].width;
		double x_diff = m_btrees[region].modules[childNode].x - parent_rx;
		double y_diff = m_btrees[region].modules[childNode].y - 0;
		m_btrees[region].modules[childNode].x = parent_rx;
		m_btrees[region].modules[childNode].y = 0;
		m_btrees[region].modules[childNode].width += x_diff;
		m_btrees[region].modules[childNode].height += y_diff;
		m_btrees[region].modules[childNode].area = m_btrees[region].modules[childNode].width *
		    m_btrees[region].modules[childNode].height;
		m_btrees[region].modules[childNode].no_rotate = true;
		printf( " >>> X BLOCK ADJUSTMENT: region %d  x_diff %f  y_diff %f <<< \n", region, x_diff, y_diff );
	    }
	    else
	    {

		int region = parentRegion;
		double parent_x = m_btrees[region].modules[parentNode].x;
		double parent_ry = m_btrees[region].modules[parentNode].y + m_btrees[region].modules[parentNode].height;
		double x_diff = m_btrees[region].modules[childNode].x - parent_x;
		double y_diff = m_btrees[region].modules[childNode].y - parent_ry;
		m_btrees[region].modules[childNode].x = parent_x;
		m_btrees[region].modules[childNode].y = parent_ry;
		m_btrees[region].modules[childNode].width += x_diff;
		m_btrees[region].modules[childNode].height += y_diff;
		m_btrees[region].modules[childNode].area = m_btrees[region].modules[childNode].width *
		    m_btrees[region].modules[childNode].height;
		m_btrees[region].modules[childNode].no_rotate = true;
		printf( " >>> Y BLOCK ADJUSTMENT: region %d  x_diff %f  y_diff %f <<< \n", region, x_diff, y_diff );
	    }
	}
    }

    printf( "Complete\n" );
}

