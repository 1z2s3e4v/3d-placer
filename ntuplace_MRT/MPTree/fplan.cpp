// Project: B*-trees floorplanning
// Advisor: Yao-Wen Chang  <ywchang@cis.nctu.edu.tw>
// Authors: Jer-Ming Hsu   <barz@cis.nctu.edu.tw>
// 	    Hsun-Cheng Lee <gis88526@cis.nctu.edu.tw>
// Sponsor: Arcadia Inc.
// Date:    7/19/2000 ~

//---------------------------------------------------------------------------
#include <fstream>
#include <cstdio>
#include <cstring>
#include <climits>
#include <cassert>
#include <iostream>
#include <ctime>
#include <cmath>
#include <cstdlib>
using namespace std;

//#include <sys/time.h>
//#include <sys/resource.h>

#include "fplan.h"
#include "mptreeutil.h"
using namespace N_ntumptree;
//---------------------------------------------------------------------------
char line[100], t1[40], t2[40], t3[40];
ifstream fs;

FPlan::FPlan(double calpha=1){
  norm_area= 1;
  norm_wire= 1;
  norm_waste = 1;
  cost_alpha=calpha;
  outline_ratio = -1;	// default: none
  TotalArea = 0;	// no modules
  modules_N = 0;
  Width = 0;
  Height = 0;
  netCount=0;
}
 
void FPlan::packing()
{
    calcWireLengthBK();	// always calculate wire for MTK proj
    //if(cost_alpha!=1)
    //	calcWireLength();
}

void FPlan::clear(){
  Area = 0; 
  WireLength = 0;
}

double FPlan::getCost()
{

  ///////////////////////////////////////////
  // For Area + Wire packing
  ///////////////////////////////////////////
/*  if(cost_alpha==1)
     return cost_alpha*(Area/norm_area);
  else if(cost_alpha < 1e-4)
     return (WireLength/norm_wire);
  else
     return cost_alpha*(Area/norm_area)+(1-cost_alpha)*(WireLength/norm_wire);
*/
  
 
  // outline cost

  double wasteCost = getWasteArea()/norm_waste; // donnie 2005-07-10
  double wire;		// donnie 2005-11-24
  if( norm_wire == 0 )
      wire = 0;
  else
      wire = WireLength / norm_wire;
      
  double areaCost = Area/norm_area;
  
  double ex_ar_cost = getHeight()/getWidth() - (outline_height/outline_width);
  ex_ar_cost = ex_ar_cost * ex_ar_cost;
  
  if ((outline_height/outline_width) > 8){
  	cout << "Outline problem:" <<(outline_height/outline_width) <<endl;
  	cout << "outline_height:" <<outline_height <<endl;
  	cout << "outline_width:" << outline_width << endl;
  }
 
  double displacement = getTotalModuleDisplacement() / modules_N / outline_width ;
  //double displacement =0.0;

  double ex_width = (getWidth() - outline_width) / outline_width;
  double ex_height = (getHeight() - outline_height) / outline_height;
  if( ex_width < 0 )
      ex_width = -0.1 * ex_width;
  if( ex_height < 0 )
      ex_height = -0.1 * ex_height;

  double thickness = getMaxThickness() / outline_width;
//  double orientation = getOrientationCost();
 double orientation =0.0;

  return wasteCost * WEIGHT_WASTE + 
              wire * WEIGHT_WIRE +
          ex_width * WEIGHT_BOUNDARY + 
         ex_height * WEIGHT_BOUNDARY + 
      displacement * WEIGHT_DISPLACEMENT +
         thickness * WEIGHT_THICKNESS +		
       orientation * WEIGHT_ORIENTATION +
       areaCost *  WEIGHT_AREA +
       ex_ar_cost * WEIGHT_ASPECT
       ;

  //return 10 * wasteCost + Area/norm_area + ex_ar_cost + displacement;
  //return cost_alpha*(Area/norm_area) + (1-cost_alpha)*(WireLength/norm_wire) + k3*ar_cost + wasteCost;
}

/*
double FPlan::getOrientationCost()
{
    double cost = 0;
    for( int i=0; i<modules_N; i++ )
    {
	if( !modules_info[i].rotate )
	{
	    if( modules[i].height / modules[i].width > MAX_ASPECT_RATIO )
		cost += 1.0;
	}
	else
	{
	    if( modules[i].width / modules[i].height > MAX_ASPECT_RATIO )
		cost += 1.0;
	}
    }
    return cost;
}
*/

double FPlan::getDeadSpace()
{
  return 100*(Area-TotalArea)/double(Area);
}

void FPlan::normalize_cost(int t)
{
  norm_area = norm_wire = norm_waste = 0;

  for(int i=0; i < t; i++)
  {
    perturb();
    packing();
    norm_area += Area;
    norm_wire += WireLength;
    norm_waste += TotalWasteArea; 
  }
  
  norm_area /= t;
  norm_wire /= t;
  norm_waste /= t;
  printf("normalize area=%.6f, wire=%.0f, waste=%.0f\n", 
	  norm_area, norm_wire, norm_waste);
}

//---------------------------------------------------------------------------
//   Read
//---------------------------------------------------------------------------
namespace N_ntumptree{
char* tail(char *str){
    str[strlen(str)-1]=0;
    return str;
}
}

void FPlan::read(char *file){
  filename = file; 
  fs.open(file);
  if( !fs )
    error("unable to open file: %s",file);

  bool final=false;
  Module dummy_mod;
  for(int i=0; !fs.eof(); i++){
    // modules
    modules.push_back(dummy_mod);	// new module
    Module &mod = modules.back();
    mod.id = i;
    mod.pins.clear();
    mod.no_rotate = false;

    fs >> t1 >> t2;
    tail(t2);			// remove ";"
    strcpy(mod.name,t2);

    fs >> t1 >> t2;
    if(!strcmp(t2,"PARENT;"))
	final= true;
    
    // dimension
    read_dimension(mod);    
    read_IO_list(mod,final);

    // network
    if(final){
      read_network();
      break;
    }
  }

  root_module = modules.back();
  modules.pop_back();		// exclude the parent module
  modules_N = (int)modules.size();  
  modules_info.resize(modules_N);
  modules.resize(modules_N);

  create_network();

  TotalArea = 0;
  for(int i=0; i < modules_N; i++)
    TotalArea += modules[i].area;

}

void FPlan::read_dimension(Module &mod){
    fs >> t1;
    int min_x=INT_MAX,min_y=INT_MAX,max_x=INT_MIN,max_y=INT_MIN;
    int tx,ty;
    for(int i=0; i < 4;i++){
      fs >> tx >> ty; 
      min_x=min(min_x,tx); max_x=max(max_x,tx);
      min_y=min(min_y,ty); max_y=max(max_y,ty);
    }

    mod.x      = min_x;
    mod.y      = min_y;
    mod.width  = max_x - min_x;
    mod.height = max_y - min_y;
    mod.area   = mod.width * mod.height;
    fs >> t1 >> t2;
}

void FPlan::read_IO_list(Module &mod,bool parent=false){
    // IO list
    while(!fs.eof()){
      Pin p;
      fs.getline(line,100);
      if(strlen(line)==0) continue;

      //sscanf(line,"%s %*s %d %d",t1,&p.x,&p.y);
      char tmp1[500], tmp2[500];
      sscanf( line, "%s %*s %s %s", t1, tmp1, tmp2 );
      p.x = atof( tmp1 );
      p.y = atof( tmp2 );
      
      if(!strcmp(t1,"ENDIOLIST;"))
	break;

      if(parent){ // IO pad is network
       // make unique net id
        net_table.insert(make_pair(string(t1),net_table.size()));
        p.net = net_table[t1];
      }

      p.mod = mod.id;
      p.x -= mod.x;  p.y -= mod.y;	// shift to origin

      mod.pins.push_back(p);
    }
    fs.getline(line,100);
}

void FPlan::read_network(){
    while(!fs.eof()){
      bool end=false;
      int n=0;
      fs >> t1 >> t2;
      if(!strcmp(t1,"ENDNETWORK;"))
        break;
      // determine which module interconnection by name
     unsigned int m_id;
      for(m_id=0; m_id < modules.size(); m_id++)
        if(!strcmp(modules[m_id].name,t2))
   	  break;
      if(m_id== modules.size())
 	error("can't find suitable module name!");
        
      while(!fs.eof()){
        fs >> t1;
        if(t1[strlen(t1)-1]==';'){
 	  tail(t1);
          end=true;
        }

        // make unique net id
        net_table.insert(make_pair(string(t1),net_table.size()));
        modules[m_id].pins[n++].net = net_table[t1];
        if(end) break;
      }
    }
}



//////////////////////////////////////////////////////////////////////////////////
void FPlan::read_bookshelf_fplan( const char *file )
{
    string file_node = file;
    file_node += ".blocks";

    string file_nets = file;
    file_nets += ".nets";

    string file_pads = file;
    file_pads += ".pl";

    read_bookshelf_block( file_node.c_str() );
    read_bookshelf_pads( file_pads.c_str() );
    read_bookshelf_nets( file_nets.c_str() );

}

void FPlan::read_bookshelf_block( const char *fileBlocks )
{
    printf( "Read block file: %s\n", fileBlocks );

    ifstream ifBlocks( fileBlocks );
    if( !ifBlocks )
    {
	cerr << "Cannot open block file: " << fileBlocks << endl;
	return ;
    }

    int nSoftBlocks, nHardBlocks, nTerminals;
    nSoftBlocks = nHardBlocks = nTerminals = -1;

    int lineNumber = 0;

    // check file format string
    char tmp[1000];
    ifBlocks.getline( tmp, 1000 );
    lineNumber++;
    //if( strcmp( "UCSC blocks 1.0", tmp ) != 0 )
    //{
    //	cerr << "Blocks file format error\n";
    //	return ;
    //}

    // check file header
    int checkFormat = 0;
    while( ifBlocks.getline( tmp, 1000 ) )
    {
	lineNumber++;

	//cout << tmp << endl;
	if( tmp[0] == '#' )		continue;
	if( strncmp( "NumSoftRectangularBlocks", tmp, 24 ) == 0 )
	{
	    char* pNumber = strrchr( tmp, ':' );
	    nSoftBlocks = atoi( pNumber+1 );
	    checkFormat++;
	}
	else if( strncmp( "NumHardRectilinearBlocks", tmp, 24 ) == 0 )
	{
	    char* pNumber = strrchr( tmp, ':' );
	    nHardBlocks = atoi( pNumber+1 );
	    checkFormat++;
	}
	else if( strncmp( "NumTerminals", tmp, 12 ) == 0 )
	{
	    char* pNumber = strrchr( tmp, ':' );
	    nTerminals = atoi( pNumber+1 );
	    checkFormat++;
	}

	if( checkFormat == 3 )
	    break;

    }

    if( checkFormat != 3 )
    {
	cerr << "** Block file header error\n";
    }

    cout << "  Soft blocks: " << nSoftBlocks << endl;
    cout << "  Hard blocks: " << nHardBlocks << endl;
    cout << "    Terminals: " << nTerminals << endl;

    // Read modules and terminals.
    Module dummy_mod;
    char name[100];
    char type[100];
    int point;
    int x1, y1, x2, y2, x3, y3, x4, y4;
    int nReadModules = 0;
    int nReadTerminals = 0;
    char readTmp[100];
    while( ifBlocks.getline( tmp, 1000 ) )
    {
	lineNumber++;


	readTmp[0] = '\0';
	sscanf( tmp, "%s", readTmp );
	if( readTmp[0] == '\0' )    // nothing read
	    continue;

	for( int i=0; i<(int)strlen(tmp); i++ )
	{
	    if( tmp[i] == '(' || tmp[i] == ')' || tmp[i] == ',' )
		tmp[i] = ' ';
	}
	sscanf( tmp, "%s %s %d %d %d %d %d %d %d %d %d",
		name, type, &point, 
		&x1, &y1, &x2, &y2, &x3, &y3, &x4, &y4 );

	if( strcmp( type, "hardrectilinear" ) == 0 && point == 4 )
	{
	    nReadModules++;
	    int minX, minY, maxX, maxY, width, height;
	    minX = min( min( x1, x2 ), min( x3, x4 ) );
	    maxX = max( max( x1, x2 ), max( x3, x4 ) );
	    minY = min( min( y1, y2 ), min( y3, y4 ) );
	    maxY = max( max( y1, y2 ), max( y3, y4 ) );
	    width = maxX - minX;
	    height = maxY - minY;

	    //=======================================
	    modules.push_back( dummy_mod );
	    Module &mod = modules.back();
	    mod.id = nReadModules-1;
	    strcpy( mod.name, name );
	    mod.width = width;		// cell width
	    mod.height = height;	// row height
	    mod.area = mod.width * mod.height;
	    //=======================================

	    //fplan.AddModule( name, width, height );
	}
	else if( strcmp( type, "terminal" ) == 0 )
	{
	    nReadTerminals++;
	    //fplan.AddTerminal( name );

	    ////======================================
	    //Module dummy_mod;
	    //pads.push_back( dummy_mod );
	    //Module &mod = pads.back();
	    //mod.id = nReadTerminals-1;
	    //strcpy( mod.name, name );
	    //mod.width = 0;
	    //mod.height = 0;
	    //mod.area = 0;
	    ////mod.x = atoi( t2 );
	    ////mod.y = atoi( t3 );
	    //mod.nets.clear();
	    ////======================================

	}
	else
	{
	    cerr << "Syntax unsupport in line " << lineNumber << ": " 
		<< tmp << endl;
	    exit(-1);
	}

    }

    // check if modules number and terminal number match
    if( nReadModules != nHardBlocks )
    {
	cerr << "Error: There are " << nReadModules << " modules in the file\n";
	exit(-1);
    }
    if( nReadTerminals != nTerminals )
    {
	cerr << "Error: There are " << nReadTerminals << " terminals in the file\n";
	exit(-1);
    }


    root_module = modules.back();
    modules_N = modules.size();  
    modules_info.resize(modules_N);
    modules.resize(modules_N);

    TotalArea = 0;
    for(int i=0; i < modules_N; i++)
    {
	TotalArea += modules[i].area;
	modules_info[i].rotate = false;
	modules_info[i].x = 0;
	modules_info[i].y = 0;
	modules_info[i].rx = modules[i].width;
	modules_info[i].ry = modules[i].height;
    }

    //fplan.CreateModuleNameMap();
    //fplan.m_nModules = nHardBlocks + nTerminals;

    // === debug ===
    //fplan.PrintModules();
    //fplan.PrintTerminals();
    // =============

    return ;
}


void FPlan::read_bookshelf_nets( const char *file )
{
    for( unsigned int m_id=0; m_id<modules.size(); m_id++ )
    {
	//cout << modules[m_id].name << "\t";
	module_table.insert( make_pair( string(modules[m_id].name), m_id ) );
    }	
    for( unsigned int m_id=0; m_id<pads.size(); m_id++ )
    {
	//cout << pads[m_id].name << "\t";
	module_table.insert( make_pair( string(pads[m_id].name), modules_N + m_id ) );
    }	

    printf( "Total modules= %d (modules+pads)\n", module_table.size() );

    printf( "Read net file: %s\n", file );

    ifstream fs;
    fs.open( file );

    if( !fs )
    {
	error( "Unable to open net file: %s", file );
    }

    int nNets = -1;
    int nPins = -1;
    while( !fs.eof() )
    {
	fs >> t1;
	if( strcmp( t1, "NumNets" ) == 0 )
	{
	    fs >> t1 >> t2;
	    nNets = atoi( t2 );
	    break;
	}
    }
    if( fs.eof() )
	error( "%s format error.", file );

    fs >> t1 >> t2 >> t3;
    nPins = atoi( t3 );
    printf( "   NumNets= %d\n", nNets );
    printf( "   NumPins= %d\n", nPins );

    net_table.clear();

    char netName[20];
    m_network.clear();
    int degree;
    int totalNets = 0;
    int totalPins = 0;
    int netId;
    while( !fs.eof() )
    {
	fs >> t1 >> t2 >> t3;
	degree = atoi( t3 );
	if( strcmp( t1, "NetDegree" ) == 0 )
	{
	    netId = totalNets;
	    sprintf( netName, "n%d", netId );
	    net_table.insert( make_pair( string(netName), netId ) );
	    //net_table.insert( make_pair( string(netName), net_table.size() ) );

	    totalNets++;


	    //printf( "\nNet %d:  ", netId );

	    //if( degree > 2 )
	    //	cout << endl;

	    //if( totalNets % 1000 == 1 )
	    //	cout << totalNets << endl;

	    totalPins += degree;
	    vector<int> net;
	    m_network.push_back( net );
	    vector<int> &m = m_network.back();
	    for( int j=0; j<degree; j++ )
	    {
		fs >> t1 >> t2;
		// find module t1 id
		bool found = false;

		map <string, int> :: const_iterator map_ite;
		map_ite = module_table.find(t1); 

		if( map_ite == module_table.end() )
		    found = false;
		else
		{
		    found = true;
		    int id = map_ite->second;

		    if( id >= modules_N )
		    {
			//printf( "P%d ", id-modules_N );
			pads[id-modules_N].nets.insert( netId );
		    }
		    else
		    {
			//printf( "M%d ", id );
			modules[id].nets.insert( netId );
		    }
		    //if( degree > 2 )
		    //	cout << "[" << id << "] ";
		    m.push_back( id );

		}

		/*
		   for( int m_id=0; m_id<modules.size(); m_id++ )		// match cell
		   if( strcmp( modules[m_id].name, t1 ) == 0 )
		   {
		   m.push_back( m_id );
		   found = true;
		   break;
		   }

		   for( int m_id=0; m_id<pads.size(); m_id++ )			// match pads
		   if( strcmp( pads[m_id].name, t1 ) == 0 )
		   {
		   m.push_back( modules_N + m_id );
		   found = true;
		   break;
		   }

*/

		if( !found )
		{
		    error( "Unable to find module name: %s", t1 );
		}

	    }


	}
    }

    printf( "   Total nets= %d\n", totalNets );
    printf( "   m_network.size()= %d\n", m_network.size() );
    printf( "   net_table.size()= %d\n", net_table.size() );
    printf( "   Total pins= %d\n", totalPins );

}

void FPlan::read_bookshelf_pads( const char *file )
{

	printf( "Read pad file: %s\n", file );
	ifstream fs;
	fs.open( file );

	if( !fs )
	{
		error( "Unable to open pl file: %s", file );
	}

	//while( !fs.eof() )
	//{
	//	fs >> t1;
	//	char a;
	//	fs.get( &a, 1 );
	//	//if( strcmp( t1, "p1" ) == 0 )
	//	if( a == 'p' )
	//	{
	//		fs.unget();
	//		break;
	//	}
	//}
	//if( fs.eof() )
	//	error( "%s format error.", file );

	Module dummy_mod;
	int counter = 0;

	//char t4[40], t5[40];

	//char line[1000];
	//fs.getline( line, 1000 );
	//sscanf( line, "%s %s %s", t1, t2, t3 );


	////fs >> t2 >> t3 >> t4 >> t5;
	//pads.push_back( dummy_mod );
	//Module &mod = pads.back();
	//mod.id = counter;
	////mod.pins.clear();
	//strcpy( mod.name, t1 );
	//mod.width = 0;
	//mod.height = 0;
	//mod.area = 0;
	//mod.x = atoi( t2 );
	//mod.y = atoi( t3 );
	//mod.nets.clear();
	////mod.parentClusterId = -1;
	////mod.totalNets = 0;
	//counter++;

	while( !fs.eof() )
	{
		fs.getline( line, 1000 );
		sscanf( line, "%s %s %s", t1, t2, t3 );


		//fs >> t1 >> t2 >> t3 >> t4 >> t5;
		if( t1[0] != 'p' || fs.eof() )
			continue;

		pads.push_back( dummy_mod );
		Module &mod = pads.back();
		mod.id = counter;
		//mod.pins.clear();
		strcpy( mod.name, t1 );
		mod.width = 0;
		mod.height = 0;
		mod.area = 0;
		mod.x = atoi( t2 );
		mod.y = atoi( t3 );
		//mod.cx = mod.x;
		//mod.cy = mod.y;
		//mod.totalNets = 0;

		mod.nets.clear();

		counter++;
	}

	fs.close();

	pads.resize( pads.size() );
	printf( "   Pad number= %d\n", pads.size() );


}
/////////////////////////////////////////////////////////////////////////////////////////




void FPlan::read_simple(char *file)
{
  filename = file; 
  fs.open(file);
  if( !fs )
    error("unable to open file: %s",file);

  Module dummy_mod;
  for(int i=0; !fs.eof(); i++)
  {
    // modules
    modules.push_back(dummy_mod);	// new module
    Module &mod = modules.back();
    mod.id = i;
    mod.pins.clear();
    mod.no_rotate = false;

    fs >> t2;
    strcpy(mod.name, t2);

    fs >> t1 >> t2;
	mod.width = atoi( t1 );
	mod.height = atoi( t2 );
	mod.area = mod.width * mod.height;
  }

  root_module = modules.back();
  modules.pop_back();		// exclude the parent module
  modules_N = modules.size();  
  modules_info.resize(modules_N);
  modules.resize(modules_N);

  //create_network();

  TotalArea = 0;
  for(int i=0; i < modules_N; i++)
    TotalArea += modules[i].area;


}




//---------------------------------------------------------------------------
//   Wire Length Estimate
//---------------------------------------------------------------------------

void FPlan::create_network(){
    network.resize(net_table.size());

    for(int i=0; i < modules_N; i++){
	for(unsigned int j=0; j < modules[i].pins.size(); j++){
	    Pin &p = modules[i].pins[j];
	    network[p.net].push_back(&p);
	}
    }

    for(unsigned int j=0; j < root_module.pins.size(); j++){
	Pin &p = root_module.pins[j];
	network[p.net].push_back(&p);
    }

    connection.resize(modules_N+1);
    for(int i=0; i < modules_N+1; i++){
	connection[i].resize(modules_N+1);
	fill(connection[i].begin(), connection[i].end(), 0);
    }

    for( int i=0; i < (int)network.size(); i++){
	for( int j=0; j < (int)network[i].size()-1; j++){
	    int p= (int)network[i][j]->mod;
	    for(int k=j+1 ; k < (int)network[i].size(); k++){
		int q= network[i][k]->mod;
		connection[p][q]++;
		connection[q][p]++;   
	    }
	}
    }
}


void FPlan::scaleIOPad(){
  double px = Width/double(root_module.width);
  double py = Height/double(root_module.height);
    
  for(int i=0; i < (int)root_module.pins.size(); i++){
    Pin &p = root_module.pins[i];
    p.ax = int(px * p.x);
    p.ay = int(py * p.y);
    
  }      
}


//////////////////////////////////////////////////////////
// 2004/11/3 (donnie) Bookshelf-GSRC WL
//////////////////////////////////////////////////////////
double FPlan::calcWireLengthBK()
{
    int i, mod;
    set<int>::iterator it;
    vector <bound_box> net_bb;
    
    //net_bb.resize(net_table.size());
    net_bb.resize( netCount );	// MTK proj

    for( i=0 ;i<(int)net_bb.size() ;i++ )
    {
	net_bb[i].x2 = net_bb[i].y2 = -1000;
	net_bb[i].x1 = net_bb[i].y1 = 100000;
    }

    //printf( "calc pads\n" );
    for( i=0; i<(int)pads.size(); i++ )
    {
	for( it=pads[i].nets.begin(); it != pads[i].nets.end(); ++it)
	{
	    find_net_bound_box( net_bb[*it], i, 1 );
	}
    }

    //printf( "calc modules\n" );
    for (i=0;i<modules_N;i++)
    {
	mod = i;
	for( it=modules[mod].nets.begin(); it != modules[mod].nets.end(); ++it)
	{
	    find_net_bound_box( net_bb[*it], mod, 0 );
	}
    }

    double WL=0.0;
    for (i=0;i<(int)net_bb.size();i++)
    {
	if( net_bb[i].x2 != -1000 )
	{
	    double netLength = (net_bb[i].x2-net_bb[i].x1) + (net_bb[i].y2-net_bb[i].y1);

#if 1
	    if( netLength < 0 )
		printf( "ERR net length" );
	    if( net_bb[i].x2 > 300000 )
		printf( "err1  " );
	    if( net_bb[i].y2 > 300000 )
		printf( "err2  " );
	    if( net_bb[i].x1 < -50 )
		printf( "err3  " ); 
	    if( net_bb[i].y1 < -50 )
		printf( "err4   " );
#endif

	    WL = WL + netsWeight[i] * double( (net_bb[i].x2-net_bb[i].x1) + (net_bb[i].y2-net_bb[i].y1) );
	}
    }

    WireLength = double (WL);
    return WL;
}


void FPlan::find_net_bound_box(bound_box &net_bb,int mod, int type )
{
    double x, y;
    if( type == 0 )
    {
	x= (modules_info[mod].x + modules_info[mod].rx) * 0.5;
	y= (modules_info[mod].y + modules_info[mod].ry) * 0.5;
    }
    else
    {
	x= pads[mod].x;
	y= pads[mod].y;
    }

    if (x < net_bb.x1)
    {
	net_bb.x1=x;           // x1 < x2
    }
    if (x > net_bb.x2)        // y1 < y2
    {
	net_bb.x2=x;
    }
    if (y < net_bb.y1)
    {
	net_bb.y1=y;
    }
    if (y > net_bb.y2)
    {
	net_bb.y2=y;
    }

}

double FPlan::calcWireLength()
{

    if( this->input_type == 2 )     // bookshelf
	return calcWireLengthBK();

    scaleIOPad();

    WireLength=0;
    // compute absolute position
    for(int i=0; i < modules_N; i++){   
	double mx= modules_info[i].x, my= modules_info[i].y;
	bool rotate= modules_info[i].rotate;
	double w= modules[i].width;

	for(int j=0; j < (int)modules[i].pins.size(); j++){
	    Pin &p = modules[i].pins[j];

	    if(!rotate){      
		p.ax= p.x+mx, p.ay= p.y+my;
	    }
	    else{ // Y' = W - X, X' = Y
		p.ax= p.y+mx, p.ay= (w-p.x)+my;
	    } 
	}
    }

    for(int i=0; i < (int)network.size(); i++){     
	double max_x= INT_MIN, max_y= INT_MIN;      
	double min_x= INT_MAX, min_y= INT_MAX;      

	assert(network[i].size() > 0);
	for(int j=0; j < (int)network[i].size(); j++){
	    Pin &p = *network[i][j];
	    max_x= max(max_x, p.ax), max_y= max(max_y, p.ay);
	    min_x= min(min_x, p.ax), min_y= min(min_y, p.ay);
	}
	//    printf("%d %d %d %d\n",max_x,min_x,max_y,min_y);
	WireLength += (max_x-min_x)+(max_y-min_y);
    }
    return WireLength;
}

//---------------------------------------------------------------------------
//   Modules Information
//---------------------------------------------------------------------------
namespace N_ntumptree{
string query_map(map<string,int> M,int value){
  for(map<string,int>::iterator p=M.begin(); p != M.end(); p++){
    if(p->second == value)
      return p->first;
  }
  return "";
}
}

void FPlan::show_modules()
{
    //printf( "FPlan::show_modules()   module # = %d\n", (int)modules.size() );
    //cout << "Module # = " << (int)modules.size() << endl;
    
    for(int i=0; i < (int)modules.size();i++)
    {
	cout << "   Module: W= " << modules[i].width; 
	cout << "\tH= " << modules[i].height;
	cout << "\tArea= " << modules[i].area;
	cout << "\t" << modules[i].name ;
	if( modules[i].no_rotate )
	    cout << "\t no_rotate ";
	cout << endl;
	//    cout << modules[i].pins.size() << " Pins:\n";
	//    for(int j=0; j < modules[i].pins.size(); j++){
	//      cout << query_map(net_table,modules[i].pins[j].net) << " ";
	//      cout << modules[i].pins[j].x << " " << modules[i].pins[j].y << endl;
	//    }
    }
    
}

void FPlan::outDraw( const char* filename )
{

    FILE *fs = fopen( filename, "w" );
	for( int i=0; i<modules_N; i++ )
	{
		fprintf( fs,"RECT %s %g %g %g %g 0\n",
			modules[i].name,
			modules_info[i].x, 
			modules_info[i].y,
			modules_info[i].rx - modules_info[i].x,
			modules_info[i].ry - modules_info[i].y );
	}
    fclose(fs);
}

void FPlan::list_information(){

    string info = filename + ".info";   
    ofstream of(info.c_str());

    of << modules_N << " " << Width << " " << Height << endl;
    for(int i=0; i < modules_N; i++){
	of << modules_info[i].x  << " " << modules_info[i].rx  << " ";
	of << modules_info[i].y << " " << modules_info[i].ry << endl;
    }
    of << endl;

    calcWireLength(); 
    double x,y,rx,ry;
    for(int i=0; i < (int)network.size(); i++){
	assert(network[i].size()>0);
	x = network[i][0]->ax;
	y = network[i][0]->ay;

	for(int j=1; j < (int)network[i].size(); j++){
	    rx = network[i][j]->ax;
	    ry = network[i][j]->ay;
	    of << x << " " << y << " " << rx << " " << ry << endl;
	    x = rx, y = ry;
	}
    }

    cout << "Num of Module  = " << modules_N << endl;
    cout << "Total Area     = " << TotalArea << endl;
    cout << "Height         = " << Height << endl;
    cout << "Width          = " << Width << endl;
    cout << "bbox width     = " << outline_width << endl;
    cout << "bbox height    = " << outline_height << endl;
    cout << "Aspect Ratio   = " << getAR() << endl; 
    cout << "ORIENTATION_COST = " << getOrientationCost() << endl;
    cout << "TOTAL_COST     = " << getCost() << endl;
    
    if( outline_ratio > 0 )
    {
	if( Height <= outline_height && Width <= outline_width )
	{
	    printf( "Fit into the outline.\n" );
	}
    }

    cout << "\nArea           = " << Area*1e-6 << endl;
    cout << "Wire Length    = " << calcWireLength()*1e-3 << endl;

    printf( "Waste Area     = %.0f\n", getWasteArea() );
    printf( "Dead Space     = %.2f\n", getDeadSpace() );
    printf( "Displacement   = %.0f\n", getTotalModuleDisplacement() );
    
}



//---------------------------------------------------------------------------
//   Auxilliary Functions
//---------------------------------------------------------------------------
namespace N_ntumptree{
void error( const char *msg, const char *msg2/*=""*/)
{
  printf(msg,msg2);
  cout << endl;
  throw 1;
}

bool rand_bool()
{
	return bool( rand()%2 == 0 );
}

double rand_01()
{
  return double(rand()%10000)/10000;
}

double seconds()
{
	//rusage time;
	//getrusage(RUSAGE_SELF,&time);
	//return (double)(1.0*time.ru_utime.tv_sec+0.000001*time.ru_utime.tv_usec);
	return (double)clock()/CLOCKS_PER_SEC;
}
}

void FPlan::outputGNUplot(string filename){
  int i;
  string plt = filename + ".plt";
  ofstream fo;
  fo.open(plt.c_str());
  
  fo << "set xlabel 'width'" << endl;
  fo << "set ylabel 'height'" << endl;
  fo << "plot \"modules.dat\" using 1:2 title \"macros\" with lines lt 1 lw 2";
  if( getWasteArea() > 0 )
      fo << ", \"emptyboxes.dat\" using 1:2 title \"empty\" with lines lt 2 lw 1";
  fo << endl;
  fo << "pause -1 'press any key'" << endl;
  fo.close();
  
  string m_file = "modules.dat";
  fo.open(m_file.c_str());
  for(i=0;i<modules_N;i++){
    fo << modules_info[i].x << " " << modules_info[i].y << endl;
    fo << modules_info[i].rx << " " << modules_info[i].y << endl;
    fo << modules_info[i].rx << " " << modules_info[i].ry << endl;
    fo << modules_info[i].x << " " << modules_info[i].ry << endl;
    fo << modules_info[i].x << " " << modules_info[i].y << endl;
    fo << endl << endl;
  }
  fo.close();

  string m_dummy = "emptyboxes.dat";
  fo.open(m_dummy.c_str());
  for(i=0;i<(int)boxes.size();i++){
    fo << boxes[i].x << " " << boxes[i].y << endl;
    fo << boxes[i].rx << " " << boxes[i].y << endl;
    fo << boxes[i].rx << " " << boxes[i].ry << endl;
    fo << boxes[i].x << " " << boxes[i].ry << endl;
    fo << boxes[i].x << " " << boxes[i].y << endl;
    fo << endl << endl;
  }
  fo.close();
    


  /*fo << "set style line 1 lt 1 lw 2" << endl;
  fo << "set style line 2 lt 2 lw 2" << endl;
  fo << "set xlabel 'width'" << endl;
  fo << "set ylabel 'height'" << endl;
  fo << "plot '-' using 1:2 with lines linestyle 2" << endl;

  fo << endl;
  for(i=0;i<modules_N;i++){
    fo << modules_info[i].x << " " << modules_info[i].y << endl;
    fo << modules_info[i].rx << " " << modules_info[i].y << endl;
    fo << modules_info[i].rx << " " << modules_info[i].ry << endl;
    fo << modules_info[i].x << " " << modules_info[i].ry << endl;
    fo << modules_info[i].x << " " << modules_info[i].y << endl;
    fo << endl << endl;
  }  
  
  fo << "plot '-' using 1:2 with lines linestyle 1" << endl;
  for(i=0;i<dummies.size();i++){
    fo << dummies[i].x << " " << dummies[i].y << endl;
    fo << dummies[i].rx << " " << dummies[i].y << endl;
    fo << dummies[i].rx << " " << dummies[i].ry << endl;
    fo << dummies[i].x << " " << dummies[i].ry << endl;
    fo << dummies[i].x << " " << dummies[i].y << endl;
    fo << endl << endl;
  }
  fo << "EOF " << endl;
  fo.close();*/
}


// donnie
int FPlan::addModule( const char* name, double width, double height, 
	double x/*=0*/, double y/*=0*/, bool no_rotate/*=false*/, bool fixed/*=false*/ )
{
    Module dummy_mod;
    modules.push_back( dummy_mod );
    Module &mod = modules.back();
    mod.id = modules.size();
    strcpy( mod.name, name );
    mod.width = width;          // cell width
    mod.height = height;        // row height
    mod.area = mod.width * mod.height;
    mod.x = x;
    mod.y = y;
    mod.no_rotate = no_rotate;
    mod.cx = x + 0.5 * width;
    mod.cy = y + 0.5 * height;
    
    if( !fixed )
    {
	TotalArea += mod.area;
	root_module = modules.back();
    }
    else
    {
	nFixedBlocks++;
	mod.is_extra = true;
    }

    modules_N = modules.size();
    modules_info.resize( modules_N );	// add one "modules_info"
    modules.resize( modules_N );

    int modId = modules_N-1;
    modules_info[modId].rotate = false;
    modules_info[modId].x = x;
    modules_info[modId].y = y;
    modules_info[modId].rx = x + width;
    modules_info[modId].ry = y + height;

    return modId;    
}

double FPlan::getMaxThickness()
{
    double thickness = 0;
    double rx, ry;
    double t;
    for( int i=0; i<(modules_N-nFixedBlocks); i++ )
    {
	rx = modules_info[i].rx;
	ry = modules_info[i].ry;
	t = min( rx, ry );
	if( t > thickness )
	    thickness = t;
    }
    return thickness;
}

// donnie
double FPlan::getTotalModuleDisplacement()
{
    double displacement = 0;
    double cx, cy;
    double w, h, dis;
    for( int i=0; i<(modules_N-nFixedBlocks); i++ )
    {
	w = modules[i].width;
	h = modules[i].height;
	if( modules_info[i].rotate )
	    swap( w, h );
	// Calculate new center
	cx = modules_info[i].x + 0.5 * w;
	cy = modules_info[i].y + 0.5 * h;
	// Differences with the old center
	dis = fabs( modules[i].cx - cx ) + fabs( modules[i].cy - cy );
	displacement += pow( dis, 1.1 );	// 2005-07-30 (donnie) use dis^2
    }
    return displacement;
}

// donnie
void FPlan::createResizableList()
{
    resizableModules.clear();
    for( int i=0; i<(modules_N-nFixedBlocks); i++ )
    {
	if( modules[i].sizes.size() > 1 )
	    resizableModules.push_back( i );
    }
    printf( " === %d resizable modules ===\n", resizableModules.size() );
}

//donnie
void FPlan::addTwoPinNet( int node1, int node2, double weight )
{
    netsWeight.push_back( weight );
    modules[node1].nets.insert( netCount );
    modules[node2].nets.insert( netCount );
    netCount++;
}
