// Project: B*-trees floorplanning
// Advisor: Yao-Wen Chang  <ywchang@cis.nctu.edu.tw>
// Authors: Jer-Ming Hsu   <barz@cis.nctu.edu.tw>
// 	    Hsun-Cheng Lee <gis88526@cis.nctu.edu.tw>
// Sponsor: Arcadia Inc.
// Date:    7/19/2000 ~

//---------------------------------------------------------------------------
#ifndef btreeH
#define btreeH
//---------------------------------------------------------------------------
#include <vector>
#include <cassert>
#include <climits>
#include <cstring>
#include "fplan.h"
//---------------------------------------------------------------------------
namespace N_ntumptree{

const int NIL = -1;
typedef bool DIR;
const bool LEFT=0, RIGHT=1;


struct Node
{
    int id,parent,left,right;
    bool rotate, flip;
    bool isleaf(){ return (left==NIL && right==NIL); }
};

struct NodeConstraint
{
    NodeConstraint()
    {
	noLeft = noRight = false;
    }
    bool noLeft;
    bool noRight;
};

struct Contour
{
    int front,back;
};

// By unisun (2005-07-06)
struct Point{
    double coordinate;
    int moduleId;
};

// By unisun (2005-07-06)
class cmp_sort_point{
    public:
	bool operator()(Point p1, Point p2){
	    return p1.coordinate < p2.coordinate;
	}
};

// By donnie (2005-09-07)
class node_relation
{
    public:
	enum relation {X, Y};

	node_relation( relation t, int p, int c )
	{
	    this->type = t;
	    parent = p;
	    child = c;
	} 

	relation type;
	int      parent;
	int      child;
};

// By donnie (2005-10-02) -- vertical constraint graph
struct NodeVerticalConstraint
{
    vector<int> down;
    vector<int> up;
};

class B_Tree : public FPlan
{
  public:
    B_Tree(double calpha=1) : FPlan(calpha), preplaced_root(-1) {}
    virtual ~B_Tree() {};
    
    virtual void init();
    virtual void packing();
    virtual void perturb();
    virtual void keep_sol();
    virtual void keep_best();    
    virtual void recover();
    virtual void recover_best();

    // debuging
    void testing();
    
    // set orientation according to the outline
    void setOrientation();

    // 2005-09-07
    void SetPreplacedRoot( int id );
    void AddNodeRelation( node_relation::relation r, int parent, int child )
    {
	relations.push_back( node_relation( r, parent, child ) );
    };
    virtual double getOrientationCost();
    bool CheckVertConst();

protected:
    
    void show_tree();  
    void place_module(int mod,int abut,bool is_left=true);
    bool legal();
    void clear();
    
    // Auxilary function
    void wire_nodes(int parent,int child,DIR edge);
    int child(int node,DIR d);
    bool legal_tree(int p,int n,int &num);
    void add_changed_nodes(int n);
  
    // SA permutating operation
    void swap_node(Node &n1, Node &n2);
    void swap_node2(Node &n1, Node &n2);
    //void insert_node(Node &parent,Node &node);
    void insert_node(Node &parent,Node &node, int left0right1=-1 ); // 2005-09-07 (donnie)
    void delete_node(Node &node);
    
    bool delete_node2(Node &node,DIR pull);
    void insert_node2(Node &parent,Node &node,
                     DIR edge,DIR push,bool fold=false);

    int contour_root;
    vector<Contour> contour;
    int nodes_root;
    vector<Node> nodes; 

  public: // by donnie
    vector<NodeConstraint> nodesConstraint;
    vector<NodeVerticalConstraint> nodesVertConst;  // 2005-10-02
    
    // By unisun
    void CalculateWasteArea();
    void WasteAreaTest();
 
  private:        
    struct Solution
    {
	int nodes_root;
	vector<Node> nodes;
	
	// 2005-09-07
	vector<double> modulesWidth;	    // width, height, currentLayer
	vector<double> modulesHeight;	    
	vector<int>    modulesLayer;
	
	double cost;
	Solution() { cost = 1; }
	void clear() { cost = 1, nodes.clear(); }
    };
  
    void get_solution(Solution &sol);
    void recover(Solution &sol);
    void recover_partial();

    Solution best_sol, last_sol;
    // for partial recover
    vector<Node> changed_nodes;
    int changed_root;    

    // 2005-09-07
    int preplaced_root;
    vector<node_relation> relations;
    void FixTree();
};

}

//---------------------------------------------------------------------------
#endif
