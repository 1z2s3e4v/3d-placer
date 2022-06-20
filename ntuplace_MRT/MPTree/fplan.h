// Project: B*-trees floorplanning
// Advisor: Yao-Wen Chang  <ywchang@cis.nctu.edu.tw>
// Authors: Jer-Ming Hsu   <barz@cis.nctu.edu.tw>
// 	    Hsun-Cheng Lee <gis88526@cis.nctu.edu.tw>
// Sponsor: Arcadia Inc.
// Date:    7/19/2000 ~

//======================================================
// 2005-07-10
// (donnie) use "double" instead of "int"
//======================================================

//---------------------------------------------------------------------------
#ifndef fplanH
#define fplanH
//---------------------------------------------------------------------------
#include <vector>
#include <string>
#include <fstream>
#include <map>
#include <set>
#include <cstdio>
#include <climits>
//---------------------------------------------------------------------------
using namespace std;

namespace N_ntumptree{
struct bound_box // For WL calculating
{
   double x1, y1, x2, y2;
   bound_box() { x1=y1=INT_MAX; x2=y2=INT_MIN; }
};

struct Pin
{



    int mod;
    int net;
    double x,y;    // relative position
    double ax,ay;  // absolute position
    Pin( double x_=-1.0, double y_=-1.0){ x=x_, y=y_; }
};
typedef vector<Pin> Pins;
typedef Pin* Pin_p;
typedef vector<Pin_p> Net;
typedef vector<Net > Nets;

struct size
{
    size( double width, double height )
    { 
	w = width;
	h = height;
    }
    double w;
    double h; 
};

enum Module_Type { MT_HARD, MT_SOFT, MT_CLUSTER, MT_RECLINEAR };

class Module
{
    public:
	Module()
	{
	    is_fixed = is_corner = is_pad = no_rotate = false;
	    is_dummy   = false;
	    create_bkg = true;
	    is_extra   = false;
	    id = -1;
	    x = y = area = width = height = -1;
	    type = MT_HARD;
	};
	
	int id;
	char name[500];
	double width, height;
	double x,y;
	double cx, cy;
	double area;
	Pins pins;
	Module_Type type;
	bool is_fixed;	    // fixed block or blockage. we should not change its coordinates
	bool is_corner;	    // corner block. set it as a root of the b*-tree
	bool no_rotate;	    // no rotate for a block
	bool create_bkg;    // create blockage is fixed && !dummy
	bool is_dummy;	    // create a dummy node corresponding to the fixed block/blockage
	bool is_pad;
	bool is_extra;      // extra block from the setting file
	set<int> nets;	     // nets for bookshelf format (group "pseudo nets" in MTK proj)
	vector<size> sizes;  // 2005-09-07: candidate sizes for MT_CLUSTER
	vector<int>  layers; //             the corresponding layers for "sizes"
        int currentLayer;

    	vector<int> group_id;	// 2005-11-22
};
typedef vector<Module> Modules;

struct Module_Info
{
    bool rotate, flip;
    double x,y;
    double rx,ry;
    double ox, oy, orx, ory;	// By unisun. 
    int lxi, lyi, rxi, ryi;	// By unisun
};

/// The data strucutre for calculating the waste space.
//  (By unisun)
struct EmptyBox
{
      double x, y, rx, ry;
};

typedef vector<Module_Info> Modules_Info;

class CornerTree;

class FPlan
{
    friend class CornerTree;
    
    public:
	FPlan(double calpha);
	virtual ~FPlan() {};

	void read( char* );
	void read_simple( char *file );
	void read_bookshelf_fplan( const char *file );
    private:
	void read_bookshelf_pads( const char* );
	void read_bookshelf_block( const char *file );
	void read_bookshelf_nets( const char* );
	// 2004-11-03, 2005-06-21 update (donnie)
	// For GSRC benchmark network
	Modules pads;
	vector< vector<int> > m_network;
	map<string, int> module_table;
	map<string, int> net_table;	// For both bookshelf-GSRC and YAL-MCNC
	void find_net_bound_box( bound_box &net_bb,int mod, int type );	// 2004/11/3

    public:
	virtual void init()		=0;
	virtual void packing();
	virtual void perturb()	=0;    
	virtual void keep_sol()	=0;
	virtual void keep_best()	=0;
	virtual void recover()	=0;
	virtual void recover_best() =0;
	virtual double getCost();   


	int    size()         { return modules_N; }
	double getTotalArea() { return TotalArea; }
	double getArea()      { return Area;      }
	double getWireLength(){ return WireLength;}
	double getWidth()     { return Width;     }
	double getHeight()    { return Height;    }
	//double getAR()        { return Width/Height; }
	double getAR()        { return Height/Width; }
	double getDeadSpace();
	double getWasteArea() { return TotalWasteArea; } 
	
	void outputGNUplot(string filename);	// unisun

	int addModule( const char* name, double width, double height, 
		double x=0, double y=0, bool no_rotate=false, bool fixed=false );    // (donnie) return moduleId
	double getTotalModuleDisplacement();
	double getMaxThickness();
	double getModuleX( int id )    { return modules_info[id].x; }
	double getModuleY( int id )    { return modules_info[id].y; }
	bool   getModuleRotate( int id ) { return modules_info[id].rotate; }
	
	// outline-constraint
	double k3;
	double outline_width;
	double outline_height;
	double outline_ratio;
	bool isFit()
	{
	    return ( Height <= outline_height && Width <= outline_width );
	}

	// information
	void list_information();
	void show_modules();    
	void normalize_cost(int);
	void outDraw( const char* filename );		// output drawing file

    // 2005-09-07
    protected:
	virtual double getOrientationCost() = 0;
	vector<int> resizableModules;
	
    public:
	void createResizableList();
	
    protected:
	void clear();
	double calcWireLength();
	double calcWireLengthBK();  // bookshelf wire    
	void scaleIOPad(); 

	double Area;
	double Width,Height;
	double WireLength;  /// Changed from "int" to "double." (donnie)
	double TotalArea;

	vector<float> WasteArea;    // unisun
        double TotalWasteArea;	    // unisun
	vector<EmptyBox> boxes;	    // unisun

    public:
	int nFixedBlocks;	    // number of fixed blocks (donnie, 2005-10-02)

    protected:	
	int modules_N;    
	Modules modules;
	Module  root_module;
	Modules_Info modules_info;    
	Nets network;
	double norm_area, norm_wire, norm_waste;
	double cost_alpha;
	vector<vector<int> > connection;

    private:
	void read_dimension(Module&);
	void read_IO_list(Module&,bool parent);
	void read_network();
	void create_network();   

	//map<string,int> net_table;
	string filename; 

    public:
	int input_type;

	// 2005-11-24 nets
    public:
	void addTwoPinNet( int node1, int node2, double weight );
	int netCount;
    private:
	vector<double> netsWeight;
};


void error( const char *msg, const char *msg2="");
bool rand_bool();
double rand_01();
double seconds();
      
}
//---------------------------------------------------------------------------
#endif
