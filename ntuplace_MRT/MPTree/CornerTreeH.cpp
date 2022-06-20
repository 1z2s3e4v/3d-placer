#include "CornerTree.h"
#include "mptreeutil.h"

#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;
using namespace N_ntumptree;

const char sep[] = "/";

string getLevelName( string str, int level )
{
    string retName;
    string nextStr;
    
    do
    {
	if( str == "" )
	{
	    retName = "";
	    break;
	}

	string::size_type pos = str.find( sep );

	if( pos == string::npos )
	    nextStr = "";
	else
	    nextStr = str.substr( pos+1 );

	retName = str.substr( 0, pos );
	
	str = nextStr;    
	--level;
    }while( level >= 0 );
    
    return retName;
}

void CornerTree::ParseModuleHierarchy()
{
    printf( "\nParse module hierarchy\n\n" );

    vector< pair< string, int > > name_list;
    
    // extract modules names
    for( int i=0; i<(int)m_modules.size(); i++ )
    {
	string name( m_modules[i].name );
	if( name.find( sep ) == string::npos )
	    continue;

	name_list.push_back( make_pair( m_modules[i].name, i ) );
    }
    printf( "\t# Module with hierarchy info = %d\n", name_list.size() );
   
    sort( name_list.begin(), name_list.end() );
    
    // print-out
    vector< pair< string, int > >::iterator ite, ite_end;
    ite_end = name_list.end();
    /*for( ite=name_list.begin(); ite!=ite_end; ++ite )
    {
	printf( "%3d %s\n", ite->second, ite->first.c_str() );
    }*/
   
    // assign group id to modules (from top level)
    int level = 0;
    int group_id;
    vector< vector<int> > dummy_level;
    vector<int> dummy_group;
    string previous_group_name;
    while( true )
    {
	previous_group_name = "";
	group_id = -1;

	m_groups.push_back( dummy_level );	
	for( ite=name_list.begin(); ite!=ite_end; ++ite )
	{
	    int moduleId = ite->second;
	    
	    string levelName = getLevelName( ite->first, level );

	    if( levelName == "" )
		continue;
	
	    if( levelName != previous_group_name )
	    {
		group_id++;
		m_groups[level].push_back( dummy_group );
		previous_group_name = levelName;
	    }
	   
	    m_modules[moduleId].group_id.push_back( group_id );
	    assert(  (int)m_modules[moduleId].group_id.size() == level+1 );
	    m_groups[level][group_id].push_back( moduleId );
	    
	    //printf( "%3d %d %d %s %s\n", 
		//    ite->second, level, group_id, levelName.c_str(), ite->first.c_str() );
	
	    //printf( "%3d %d %d %s \n", 
	    //	    ite->second, level, group_id, levelName.c_str() );
	}

	if( group_id <= 0 )
	    break;  // no more groups
	    
	level++;
    }

    printf( "\tTotal level # = %d\n", m_groups.size() );
    for( int i=0; i<(int)m_groups.size(); i++ )
    {
	if( m_groups[i].size() > 0 )
		printf( "\tLevel %d has %d groups\n", i, m_groups[i].size() );
    }	
    
    // plot figure for testing
    OutputGNUplot( "L0.plt", 0 ); 
    OutputGNUplot( "L1.plt", 1 );
    OutputGNUplot( "L2.plt", 2 );
    OutputGNUplot( "L3.plt", 3 );

    // add pseudo nets (in CreateBTrees )

}


void CornerTree::CreateHierarchyNets( const vector<int>& regionId, const vector<int>& nodeId )
{
    // Create nets region by region
    set<int> nets;
    set<int> netsOri;
    set<int>::iterator ite;

    m_hNets.clear();
    m_hNetsWeight.clear();
    
    //for( int i=0; i<4; i++ )	// for each region
    //int i=0;
    {
	int max_level = 3;
	if( (int)m_groups.size() < max_level )
	    max_level = m_groups.size();

	int l, g, m;	
	l = 2;
	//for( l=0; l<max_level; l++ )    // for each level
	{
	    for( g=0; g<(int)m_groups[l].size(); g++ )  // for each group
	    {
		nets.clear();
		netsOri.clear();
		for( m=0; m<(int)m_groups[l][g].size(); m++ )	 // for each module in the group
		{
		    int moduleId = m_groups[l][g][m];
		    //if( (int)m_modules[moduleId].group_id.size() < l )  // module not in this level
		    //	continue;
		    if( nodeId[moduleId] < 0 )	// no corresponding node
			continue;
		    //if( regionId[moduleId] == i /*&& m_modules[m].group_id[l] == g*/ )
		    {
			nets.insert( nodeId[moduleId] );
			netsOri.insert( moduleId );
		    }
		}

		if( nets.size() <= 1 )
		    continue;	// do not add pseudo nets

		
		printf( "L %d  G %d  size= %d \n", l, g, nets.size()/*, netsOri.size()*/ );
	
		// add h-net for plotting	
		vector<int> hNet;
		for( ite=netsOri.begin(); ite!=netsOri.end(); ++ite )
		{
		    hNet.push_back( *ite );
		}
		
		vector<int> tempNet;	// clique model
		double netWeight = 1.0 / ((double)hNet.size() - 1.0);
		for( int i=0; i<(int)hNet.size()-1; i++ )
		{
		    for( int j=1; j<(int)hNet.size(); j++ )
		    {
			if( m_modules[hNet[i]].is_fixed && m_modules[hNet[j]].is_fixed )
			    continue;
			tempNet.clear();
			tempNet.push_back( hNet[i] );
			tempNet.push_back( hNet[j] );
			m_hNets.push_back( tempNet );
			m_hNetsWeight.push_back( netWeight );
			//printf( "   %d %d\n", hNet[i], hNet[j] );
		    }
		}
		
	    } // for each group
	} // for each level
    } // for each region

    
    printf( "\nAdd pseudo nets\n" );
    for( int i=0; i<(int)m_hNets.size(); i++ )
    {
	double netWeight = m_hNetsWeight[i];

	// assume all two terminal nets
	int block1 = m_hNets[i][0];
	int block2 = m_hNets[i][1];

	int region1 = regionId[block1];
	int region2 = regionId[block2];

	if( region1 == region2 )
	{
	    int node1 = nodeId[block1];
	    int node2 = nodeId[block2];
	    m_btrees[region1].addTwoPinNet( node1, node2, netWeight );
	    //printf( "  R%d  %d -- %d \n", region1, node1, node2 );
	}
	else
	{
	    // TODO: terminal propagation
	}
	    
    }
    
}
