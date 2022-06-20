// Project: B*-trees floorplanning
// Advisor: Yao-Wen Chang  <ywchang@cis.nctu.edu.tw>
// Authors: Jer-Ming Hsu   <barz@cis.nctu.edu.tw>
// 	    Hsun-Cheng Lee <gis88526@cis.nctu.edu.tw>
// Sponsor: Arcadia Inc.
// Date:    7/19/2000 ~

// 2003/11/19	Modify perturb(), use one random number
//              Modify swap_node() and add swap_node2(), can swap parent and child
//              Modify delete_node(), place subtree with equal probability for left/right child

//---------------------------------------------------------------------------
#include <stack>
#include <set>
#include <algorithm>
#include <iostream>
using namespace std;

#include "btree.h"
#include "mptreeutil.h"
using namespace N_ntumptree;
//---------------------------------------------------------------------------
//double move_rate = 0.5;//0.33;
//double swap_rate = 1.1;//0.67;
double move_rate = 0.33;
double swap_rate = 0.67;
//---------------------------------------------------------------------------
//   Initialization
//---------------------------------------------------------------------------

void B_Tree::clear()
{
  // initial contour value
  contour_root = NIL;
  FPlan::clear();
}

void B_Tree::init()
{
  // initialize contour structure
  contour.resize(modules_N);
    
  // initialize b*tree by complete binary tree
  nodes.resize(modules_N);
  nodes_root=0;
  for(int i=0; i < modules_N; i++)
  {
    nodes[i].id = i;
    nodes[i].parent = (i+1)/2-1;
    nodes[i].left   = (2*i+1 < modules_N ? 2*i+1 : NIL);
    nodes[i].right  = (2*i+2 < modules_N ? 2*i+2 : NIL);
    nodes[i].rotate = false;
    nodes[i].flip   = false;
    // TODO: FIX ME
    //
    // Initial solution must guarantee "noLeft" and "noRight" 
    // for some node with node constraints.
  }
  nodes[0].parent = NIL;
  best_sol.clear();
  last_sol.clear();
  clear();
  /* (unisun) -- for test only
  int i;
  for(i=0;i<1000;i++){
    cout << "i = " << i << endl;
    perturb();
    packing();
  }
  exit(0);*/

  // donnie (2005-10-02)
  nodesVertConst.resize( modules_N );
  nFixedBlocks = 0;
         
  normalize_cost(30);
} 

//---------------------------------------------------------------------------
//   Testing, Debuging tools
//---------------------------------------------------------------------------

bool B_Tree::legal()
{
  int num=0;
  return legal_tree(NIL,nodes_root,num);
}

bool B_Tree::legal_tree(int p,int n,int &num)
{
  num++;
  if(nodes[n].parent!=p) return false;
  if(nodes[n].left != NIL)
    if(legal_tree(n,nodes[n].left,num) != true) return false;

  if(nodes[n].right != NIL)
    if(legal_tree(n,nodes[n].right,num) != true) return false;

  if(p==NIL) // root
    return (num==modules_N);
  return true;
}

void B_Tree::testing()
{
    int p,n;
    Solution E;

    do
    {

	n = rand()%modules_N;
	p = rand()%modules_N;

	while(n==nodes_root)		// n is not root
	    n = rand()%modules_N;

	while(n==p||nodes[n].parent==p||nodes[p].parent==n)	// n != p & n.parent != p
	    p = rand()%modules_N;

	Node &node = nodes[n];
	Node &parent = nodes[p];
	get_solution(E);
	swap_node(parent,node);

    }while(legal());

    cout << "p=" << p << ", n=" << n << endl;
    recover(E);
    show_tree();
    cout << "\n  p=" << p << ", n=" << n << endl;
    swap_node(nodes[p],nodes[n]);
    show_tree();
}

void B_Tree::show_tree()
{
  cout << "root: " << nodes_root << endl;
  for(int i=0; i < modules_N; i++)
  {
    cout << nodes[i].id << ": ";
    cout << nodes[i].left << " ";
    cout << nodes[i].parent << " ";
    cout << nodes[i].right << endl;
  }
}

//---------------------------------------------------------------------------
//   Placement modules
//---------------------------------------------------------------------------

void B_Tree::packing()
{

    // unisun
    WasteArea.resize(1);
    WasteArea[0] = 0;
    boxes.clear();

    stack<int> S;

    clear();
    int p = nodes_root;

    place_module(p,NIL);
    Node &n = nodes[p];
    if(n.right != NIL)      S.push(n.right);
    if(n.left  != NIL)      S.push(n.left);

    // clear vertical constraints (donnie, 2005-10-02)
    for( int i=0; i<(modules_N-nFixedBlocks); i++ )
    {
	nodesVertConst[i].down.clear();
	nodesVertConst[i].up.clear();
    }
    
    // inorder traverse
    while(!S.empty())
    {
	p = S.top();
	S.pop();
	Node &n = nodes[p];

	assert(n.parent != NIL);
	bool is_left = (nodes[n.parent].left == n.id);
	place_module(p,n.parent,is_left);

	// create "up" vertical constraint (donnie, 2005-10-02)
	for( int i=0; i<(int)nodesVertConst[p].down.size(); i++ )
	{
	    nodesVertConst[ nodesVertConst[p].down[i] ].up.push_back( p );
	}	    
	
	if(n.right != NIL)      S.push(n.right);
	if(n.left  != NIL)      S.push(n.left);
    }

    /////CheckVertConst();	// DEBUG 

    //============================================================ 
    // push blocks to insert fixed blocks (donnie, 2005-10-02)
    set<int> moveSet;
    for( int k=0; k<nFixedBlocks; k++ )
    {
	int node = modules_N - nFixedBlocks + k;
	double x1 = modules_info[node].x;
	double x2 = modules_info[node].rx;
	double y1 = modules_info[node].y;
	double y2 = modules_info[node].ry;

	//printf( "x2= %g  x1= %g\n", x2, x1 );
	//outDraw( "test.draw" );
	assert( x2 >= x1 );
	assert( y2 >= y1 );

	for( int i=0; i<(modules_N-nFixedBlocks); i++ )
	{
	    bool xIn = false;
	    bool yIn = false;
	    if( modules_info[i].x >= x1 && modules_info[i].x < x2 )
		xIn = true;
	    if( modules_info[i].rx > x1 && modules_info[i].rx <= x2 )
		xIn = true;
	    if( modules_info[i].x <= x1 && modules_info[i].rx >= x2 )
		xIn = true;

	    if( modules_info[i].y >= y1 && modules_info[i].y < y2 )
		yIn = true;
	    if( modules_info[i].ry > y1 && modules_info[i].ry <= y2 )
		yIn = true;
	    if( modules_info[i].y <= y1 && modules_info[i].ry >= y2 )
		yIn = true;

	    if( xIn && yIn )    // overlap
	    {
		double yShift = y2 - modules_info[i].y;
		//printf( ">>>> Move up %d %g\n", i, yShift );
		if( yShift == 0 )
		    continue;

		assert( yShift > 0 );
		modules_info[i].y  += yShift;
		modules_info[i].ry += yShift;
		for( int j=0; j<(int)nodesVertConst[i].up.size(); j++ )
		    moveSet.insert( nodesVertConst[i].up[j] );
	    }
	}

	set<int> nextMoveSet;
	set<int>::const_iterator ite;
	while( true )
	{
	    //printf( "PASS\n" );
	    for( ite=moveSet.begin(); ite!=moveSet.end(); ite++ )
	    {
		int node = *ite;
		// find new y coordinate
		double yShift = 0;
		for( int j=0; j<(int)nodesVertConst[node].down.size(); j++ )
		{
		    if( modules_info[ nodesVertConst[node].down[j] ].ry > modules_info[node].y )
		    {
			if( modules_info[ nodesVertConst[node].down[j] ].ry - modules_info[node].y > yShift )
			{
			    yShift = modules_info[ nodesVertConst[node].down[j] ].ry - modules_info[node].y;
			}
		    } 
		}
		if( yShift > 0 )
		{
		    //printf( ">>>> Move up %d %g\n", node, yShift );
		    modules_info[node].y  += yShift;	
		    modules_info[node].ry += yShift;
		    for( int j=0; j<(int)nodesVertConst[node].up.size(); j++ )
			nextMoveSet.insert( nodesVertConst[node].up[j] );
		}
	    }

	    if( nextMoveSet.size() == 0 )	// more blocks for moving?
		break;
	    moveSet = nextMoveSet;
	    nextMoveSet.clear();
	}

    } // end handling a fixed block    
    ///////////////////////////////////////////////////////////////////////////////

    // compute Width, Height
    double max_x=-1,max_y=-1;
    //for(int p= contour_root; p != NIL; p=contour[p].front)
    for( int p=0; p<(modules_N-nFixedBlocks); p++ )
    {
	max_x = max(max_x, double(modules_info[p].rx) );  
	max_y = max(max_y, double(modules_info[p].ry) );
    }

    Width  = max_x;
    Height = max_y;
    Area   = Height*Width;

    //CalculateWasteArea();	// by unisun (2005-07-06)
    FPlan::packing(); 	// for wirelength  

    WasteAreaTest();	// by unisun (2005-11-16)
}

bool B_Tree::CheckVertConst()
{
    for( int i=0; i<(modules_N-nFixedBlocks); i++ )
    {
	for( int j=0; j<(int)nodesVertConst[i].up.size(); j++ )
	{
	   int node = nodesVertConst[i].up[j];
	   if( modules_info[node].y < modules_info[i].ry )
	   {
	       printf( "ERROR, modules_info[node].y < modules_info[i].ry\n" );
	       return false;
	   }
	   if( modules_info[node].rx < modules_info[i].x )
	   {
	       printf( "ERROR, modules_info[node].rx < modules_info[i].x\n" );
	       return false;
	   }
	   if( modules_info[node].x > modules_info[i].rx )
	   {
	       printf( "ERROR, modules_info[node].x > modules_info[i].rx\n" );
	       return false;
	   }
	}
    }
    return true;
}

// is_left: default is true
void B_Tree::place_module(int mod,int abut,bool is_left)
{
    Module_Info &mod_mf = modules_info[mod];
    mod_mf.rotate       = nodes[mod].rotate;
    mod_mf.flip         = nodes[mod].flip;

    double w =  modules[mod].width;
    double h =  modules[mod].height;
    if(nodes[mod].rotate)
	swap(w,h);

    if(abut==NIL)			// root node
    {
	contour_root = mod;
	contour[mod].back = NIL;
	contour[mod].front = NIL;
	mod_mf.x  = mod_mf.y = 0.0;
	mod_mf.rx = w, mod_mf.ry = h;
	return;
    }

    int p;   // trace contour from p

    if(is_left)	// left
    {
	double abut_width = (nodes[abut].rotate ? modules[abut].height : 
		modules[abut].width);
	mod_mf.x  = modules_info[abut].x + abut_width;
	mod_mf.rx = mod_mf.x + w;
	p = contour[abut].front;

	contour[abut].front = mod;
	contour[mod].back = abut;

	if(p==NIL)  // no obstacle in Y axis
	{
	    mod_mf.y = 0;
	    mod_mf.ry = h;
	    contour[mod].front = NIL;
	    return;
	}
    }
    else
    {
	// upper
	mod_mf.x = modules_info[abut].x;
	mod_mf.rx = mod_mf.x + w;
	p = abut;

	int n=contour[abut].back;

	if(n==NIL)
	{ // i.e, mod_mf.x==0
	    contour_root = mod;
	    contour[mod].back = NIL;
	}
	else
	{
	    contour[n].front = mod;
	    contour[mod].back = n;
	}
    }

    double min_y = INT_MIN;
    double bx, by;
    assert(p!=NIL);

    for(; p!=NIL ; p=contour[p].front)
    {
	bx = modules_info[p].rx;
	by = modules_info[p].ry;
	min_y = max(min_y, by);
	nodesVertConst[mod].down.push_back( p ); // donnie (2005-10-02)
	
	if(bx >= mod_mf.rx)			 // update contour
	{
	    mod_mf.y = min_y;
	    mod_mf.ry = mod_mf.y + h;
	    if(bx > mod_mf.rx)
	    {
		contour[mod].front = p;
		contour[p].back = mod;
	    }
	    else				// bx==mod_mf.rx
	    { 
		int n= contour[p].front;
		contour[mod].front = n;
		if(n!=NIL)
		    contour[n].back = mod;
	    }
	    break;     
	}
    }

    if(p==NIL)
    {
	mod_mf.y  = (min_y==INT_MIN? 0 : min_y);
	mod_mf.ry = mod_mf.y + h;
	contour[mod].front = NIL;
    }
}

//---------------------------------------------------------------------------
//   Manipulate B*Tree auxilary procedure
//---------------------------------------------------------------------------

// place child in parent's edge
void B_Tree::wire_nodes( int parent, int child, DIR edge )		
{
	assert(parent!=NIL);
	(edge == LEFT ? nodes[parent].left : nodes[parent].right) = child;
	if(child!=NIL) 
		nodes[child].parent = nodes[parent].id;
}

// get node's child
inline int B_Tree::child( int node, DIR d )		
{
	assert( node!=NIL );
	return ( d==LEFT ? nodes[node].left : nodes[node].right);  
}


//---------------------------------------------------------------------------
//   Simulated Annealing Temporal Solution
//---------------------------------------------------------------------------

void B_Tree::get_solution(Solution &sol)
{
  sol.nodes_root = nodes_root;
  sol.nodes = nodes;
  sol.cost = getCost();
  
  sol.modulesWidth.resize( modules_N);
  sol.modulesHeight.resize( modules_N);
  sol.modulesLayer.resize( modules_N );
  
  for( int i=0; i<modules_N; i++ )
  {
      sol.modulesWidth[i]  = modules[i].width;
      sol.modulesHeight[i] = modules[i].height;
      sol.modulesLayer[i]  = modules[i].currentLayer;
  }
}

void B_Tree::keep_sol()
{
  get_solution(last_sol);
}

void B_Tree::keep_best()
{
  get_solution(best_sol);
}

void B_Tree::recover()
{
  recover(last_sol);
  // recover_partial();
}

void B_Tree::recover_best()
{
  recover(best_sol);
}

void B_Tree::recover(Solution &sol)
{
    nodes_root = sol.nodes_root;
    nodes = sol.nodes;
    for( int i=0; i<modules_N; i++ )
    {
	modules[i].width = sol.modulesWidth[i];
	modules[i].height = sol.modulesHeight[i];
	modules[i].currentLayer = sol.modulesLayer[i];
    }

}

void B_Tree::recover_partial()
{
    if(changed_root != NIL)
	nodes_root = changed_root;

    for( unsigned int i=0; i<changed_nodes.size(); i++ )
    {
	Node &n = changed_nodes[i];
	nodes[n.id] = n;
    }
}

void B_Tree::add_changed_nodes(int n)
{
    if(n==NIL) return;

    for( unsigned int i=0; i < changed_nodes.size(); i++)
	if(changed_nodes[i].id == n)
	    return;
    changed_nodes.push_back(nodes[n]);
}

//---------------------------------------------------------------------------
//   Simulated Annealing Permutation Operations
//---------------------------------------------------------------------------

void B_Tree::perturb()
{

    int p,n;
    n = rand() % (modules_N - nFixedBlocks);

    double r = rand_01();

    if( r < move_rate )
    {	
	// delete & insert
	do
	{
	    p = rand() % (modules_N - nFixedBlocks);	
	} while( n==p );    // choose different module

	delete_node(nodes[n]);
	insert_node(nodes[p], nodes[n]);
    }
    else if( r < swap_rate )
    {   
	// swap node
	do
	{
	    p = rand() % (modules_N - nFixedBlocks);
	} while( n==p );	    // choose different module

	swap_node2( nodes[p], nodes[n] );

    }
    else //if( r < rotate_rate )
    {   
       	// rotate node
	int count =0;
	while( modules[n].no_rotate == true )
	{
	    count++;
	    n = rand() % (modules_N - nFixedBlocks);
	    if( count > 10 )
	    {
		//////printf( "warning, cannot find rotatable block\n" );
		break;
	    }
	}
	if( !modules[n].no_rotate )
	   nodes[n].rotate = !nodes[n].rotate;
    }
    //else
    //{														// resize soft module
    //	if( modules_soft_N != 0 )
    //	{
    //		n = modules_soft[ rand()%modules_soft_N ];
    //		B_Tree::soft_resize( n );
    //	}
    //	else
    //		nodes[n].rotate = !nodes[n].rotate;
    //}

    // 2005-09-07 (donnie) Resize clusters
    if( resizableModules.size() > 0 && rand_01() < 0.2 )
    {
	int n = resizableModules[ rand() % resizableModules.size() ];   // choose a cluster
	int l = rand() % modules[n].layers.size();  // choose a layer
	modules[n].width  = modules[n].sizes[l].w;
	modules[n].height = modules[n].sizes[l].h;
	modules[n].currentLayer = l;
	//printf( "*" );
    }
    
    FixTree();	// 2005-09-07 (donnie) Handle pre-defined tree structure.
    
}

void B_Tree::swap_node(Node &n1, Node &n2)
{

    if(n1.left!=NIL)
    {  
	nodes[n1.left].parent  = n2.id;
    }
    if(n1.right!=NIL)
    {
	nodes[n1.right].parent = n2.id;  
    }
    if(n2.left!=NIL)
    {
	nodes[n2.left].parent  = n1.id;
    }
    if(n2.right!=NIL)
    {
	nodes[n2.right].parent = n1.id;  
    }

    if( n1.parent != n1.id )
    {
	if( n1.parent != n1.id && n1.parent != NIL)
	{
	    //add_changed_nodes(n1.parent);
	    if(nodes[n1.parent].left==n1.id)
		nodes[n1.parent].left  = n2.id;
	    else
		nodes[n1.parent].right = n2.id; 
	}
	else
	{
	    changed_root = n1.id;
	    nodes_root = n2.id;
	}
    }

    if( n2.parent != n2.id )
    {
	if( n2.parent != NIL )
	{
	    if(nodes[n2.parent].left==n2.id)
		nodes[n2.parent].left  = n1.id;
	    else
		nodes[n2.parent].right = n1.id; 
	}
	else
	{
	    nodes_root = n1.id;
	}
    }

    swap(n1.left,n2.left);
    swap(n1.right,n2.right);
    swap(n1.parent,n2.parent);
}

void B_Tree::swap_node2( Node &n1, Node &n2) // donnie
{
    if( &n1 == &n2 )
	return;

    if( n1.parent != n2.id && n2.parent != n1.id )
	swap_node( n1, n2 );
    else
    {
	bool leftChild;
	bool parentN1 = ( n1.id == n2.parent );
	if( parentN1 )
	{
	    if( n1.left == n2.id )
	    {
		n1.left = NIL;
		leftChild = true;
	    }
	    else
	    {
		n1.right = NIL;
		leftChild = false;
	    }
	    n2.parent = n2.id;
	}
	else
	{
	    if( n2.left == n1.id )
	    {
		n2.left = NIL;
		leftChild = true;
	    }
	    else
	    {
		n2.right = NIL;
		leftChild = false;
	    }
	    n1.parent = n1.id;
	}

	swap_node( n1, n2 );
	if( parentN1 )
	{
	    n1.parent = n2.id;
	    if( leftChild )
	    {
		n2.left = n1.id;
	    }
	    else
	    {
		n2.right = n1.id;
	    }
	}
	else
	{
	    n2.parent = n1.id;
	    if( leftChild )
	    {
		n1.left = n2.id;
	    }
	    else
	    {
		n1.right = n2.id;
	    }
	}
    }
}

void B_Tree::insert_node( Node &parent, Node &node, int left0right1/*=-1*/ )
{
    node.parent = parent.id;

    if( left0right1 == -1 )
	left0right1 = rand() % 2;

    // 2005-09-08
    if( nodesConstraint.size() > 0 )
    {
	if( nodesConstraint[parent.id].noLeft && nodesConstraint[parent.id].noRight )
	    return;
	if( nodesConstraint[parent.id].noLeft )
	    left0right1 = 1;
	if( nodesConstraint[parent.id].noRight )
	    left0right1 = 0;
    }
    
/*
    if( left0right1 == 0 )
    {
	node.left  = parent.left;
	node.right = NIL;
	if(parent.left!=NIL)
	    nodes[parent.left].parent = node.id;

	parent.left = node.id;

    }
    else
    {
	node.left  = NIL;
	node.right = parent.right;
	if(parent.right!=NIL)
	    nodes[parent.right].parent = node.id;

	parent.right = node.id;

    }*/
    
    
    // Record old child
    int oldChild;
    if( left0right1 == 0 )
	oldChild = parent.left;
    else
	oldChild = parent.right;

    // Set left/right children and set their parent
    if( rand_bool() )
    {
	// left
	node.left  = oldChild;
	node.right = NIL;
	if( oldChild != NIL )
	    nodes[ oldChild ].parent = node.id;
    }
    else
    {
	// right
	node.left  = NIL;
	node.right = oldChild;
	if( oldChild != NIL )
	    nodes[ oldChild ].parent = node.id;
    }

    // Attach to the parent.
    if( left0right1 == 0 )
	parent.left = node.id;	// left
    else
	parent.right = node.id;	// right

}

void B_Tree::delete_node(Node &node)
{
    int child     = NIL;		// pull which child
    int subchild  = NIL;		// child's subtree
    int subparent = NIL; 

    if( !node.isleaf() )
    {
	bool left = rand_bool();			// choose a child to pull up
	if(node.left ==NIL) left=false;
	if(node.right==NIL) left=true;

	//add_changed_nodes(node.left);
	//add_changed_nodes(node.right);

	if(left)
	{
	    child = node.left;			// child will never be NIL
	    if(node.right!=NIL)
	    {
		subchild  = nodes[child].right;
		subparent = node.right;
		nodes[node.right].parent = child; 
		nodes[child].right = node.right;	// abut with node's another child
	    }
	}
	else
	{
	    child = node.right;
	    if(node.left!=NIL)
	    {
		subchild  = nodes[child].left;
		subparent = node.left;
		nodes[node.left].parent = child;
		nodes[child].left = node.left;
	    }
	}
	//add_changed_nodes(subchild);
	nodes[child].parent = node.parent;
    }

    if(node.parent == NIL)			// root
    {			
	// changed_root = nodes_root;
	nodes_root = child;
    }
    else								// let parent connect to child
    {					
	//add_changed_nodes(node.parent);
	if(node.id == nodes[node.parent].left)
	    nodes[node.parent].left  = child;
	else
	    nodes[node.parent].right = child;
    }

    // place subtree
    if(subchild != NIL)
    {
	Node &sc = nodes[subchild];
	assert(subparent != NIL);

	while(1)
	{
	    Node &p = nodes[subparent];

	    // 2003/11/19
	    // if both left and right child NIL, place with equal probability
	    if( p.left == NIL && p.right == NIL )
	    {
		sc.parent = p.id;
		if( rand_bool() )
		{
		    p.left = sc.id;
		}
		else
		{
		    p.right = sc.id;
		}
		break;
	    }
	    else if( p.left == NIL )		// place left 
	    {
		sc.parent = p.id;
		p.left = sc.id;
		break;
	    }
	    else if( p.right == NIL )		// place right
	    {
		sc.parent = p.id;
		p.right = sc.id;
		break;
	    }
	    else
	    {
		subparent = (rand_bool() ? p.left : p.right);
	    }
	}
    }

}


bool B_Tree::delete_node2( Node &node, DIR pull )
{
	DIR npull = !pull;

	int p  = node.parent;
	int n  = node.id;
	int c  = child( n, pull );
	int cn = child( n, npull );

	assert( n!= nodes_root ); // not root;

	DIR p2c = ( nodes[p].left == n ? LEFT : RIGHT );	// current DIR, LEFT or RIGHT

	if(c == NIL)
	{
		// Pull child, but NIL, so pull the other child
		wire_nodes( p, cn, p2c );
		return ( cn!=NIL );   // folding
	}
	else
	{
		wire_nodes( p, c, p2c );
	}

	while( c != NIL )
	{
		int k = child( c, npull );
		wire_nodes( c, cn, npull );
		cn = k;
		n = c;
		c = child(c,pull);
	}

	if(cn != NIL)
	{
		wire_nodes(n,cn,pull);
		return true;
	}
	else 
		return false;

}

/*
   Insert node into parent's left or right subtree according by "edge".
   Push node into parent's subtree in  "push" direction.

   if "fold" is true, then fold the leaf.
   (for the boundary condition of "delete" operation)

   delete <==> insert are permutating operations that can be recoved.
*/

void B_Tree::insert_node2(Node &parent,Node &node,
                        DIR edge=LEFT,DIR push=LEFT,bool fold/*=false*/){
  DIR npush = !push;
  int p= parent.id;
  int n= node.id;
  int c= child(p,edge);

  wire_nodes(p,n,edge);
  wire_nodes(n,c,push);
    
  while(c!=NIL)
  {
    wire_nodes(n,child(c,npush) ,npush);
    n= c;
    c= child(c,push);
  }
  wire_nodes(n,NIL,npush);

  if(fold)
  {
    wire_nodes(nodes[n].parent,NIL,push);
    wire_nodes(nodes[n].parent,n,npush); 
  }
}

void B_Tree::setOrientation()
{
    if( outline_ratio <= 0 )
        return;


    for( int i=0; i<(modules_N-nFixedBlocks); i++ )
    {
        if( modules[i].width > outline_width || modules[i].height > outline_height )
        {
            // rotate it
            nodes[i].rotate = true;
            modules[i].no_rotate = true;
            printf( "fix module %d\n", i );
    
            if( modules[i].height > outline_width || modules[i].width > outline_height )
            {
                printf( "impossible to legal module %d\n", i );
            }

        }
        if( modules[i].height > outline_width || modules[i].width > outline_height )
        {
            modules[i].no_rotate = true;
            printf( "fix module %d\n", i );
        }



    }

}

/*
void B_Tree::CalculateWasteArea(){
    int i,j,k;
    vector<Point> sort_x, sort_y, temp_x, temp_y;

    // store the x (y) and rx (ry) coordinate
    for(i=0;i<modules_N;i++){
	Point p1, p2, p3, p4;
	p1.coordinate = modules_info[i].y;
	p2.coordinate = modules_info[i].ry;
	p3.coordinate = modules_info[i].x;
	p4.coordinate = modules_info[i].rx;
	p1.moduleId = p2.moduleId = p3.moduleId = p4.moduleId = i;
	temp_y.push_back(p1); temp_y.push_back(p2);
	temp_x.push_back(p3); temp_x.push_back(p4);
    }

    // sorting into acesding order (from small to large)

    sort(temp_y.begin(), temp_y.end(), cmp_sort_point());
    sort(temp_x.begin(), temp_x.end(), cmp_sort_point());

    // eliminate duplicate for sorted x coordinates and y coordinates

    for(i=0;i<(int)temp_y.size();i++){
	Point p = temp_y[i];
	if(i == 0){ sort_y.push_back(p); }
	else if (p.coordinate != temp_y[i-1].coordinate){ sort_y.push_back(p); }

	if(modules_info[p.moduleId].y == p.coordinate)
	    modules_info[p.moduleId].lyi = sort_y.size()-1;
	else modules_info[p.moduleId].ryi = sort_y.size()-1;
    }

    for(i=0;i<(int)temp_x.size();i++){
	Point p = temp_x[i];
	if(i == 0){ sort_x.push_back(p); }
	else if (p.coordinate != temp_x[i-1].coordinate){ sort_x.push_back(p); }

	if(modules_info[p.moduleId].x == p.coordinate)
	    modules_info[p.moduleId].lxi = sort_x.size()-1;
	else modules_info[p.moduleId].rxi = sort_x.size()-1;
    }

    // col_max is used to store the maximum 
    int FPMatrix[sort_x.size()][sort_y.size()];
    for(i=0;i<(int)sort_x.size();i++){
	for(j=0;j<(int)sort_y.size();j++){
	    FPMatrix[i][j] = -1;
	}  
    }

    int col_max[sort_x.size()];
    for(i=0;i<(int)sort_x.size();i++) col_max[i] = 0;

    for(i=0;i<modules_N;i++){
	for(j=modules_info[i].lxi;j<modules_info[i].rxi;j++){
	    for(k=modules_info[i].lyi;k<modules_info[i].ryi;k++){
		FPMatrix[j][k] = i;
	    }
	    col_max[j] = max(col_max[j], modules_info[i].lyi);
	}
    }

    TotalWasteArea = 0;
    for(j=0;j<(int)sort_x.size()-1;j++){
	for(k=0;k<col_max[j];k++){
	    if (FPMatrix[j][k] == -1){
		double x = sort_x[j].coordinate, rx = sort_x[j+1].coordinate;
		double y = sort_y[k].coordinate, ry = sort_y[k+1].coordinate;
		double area = (rx - x)*(ry - y);
		EmptyBox D;
		D.x = x; D.y = y; D.rx = rx; D.ry = ry;
		boxes.push_back(D);
		TotalWasteArea += area;
	    }
	}
    }
    //cout << "dummy size = " << dummies.size() << endl;  
}
*/

void B_Tree::CalculateWasteArea(){
    int i,j,k;
    vector<Point> sort_x, sort_y, temp_x, temp_y;

    // store the x (y) and rx (ry) coordinate
    for(i=0;i<modules_N;i++){
        Point p1, p2, p3, p4;
        p1.coordinate = modules_info[i].y;
        p2.coordinate = modules_info[i].ry;
        p3.coordinate = modules_info[i].x;
        p4.coordinate = modules_info[i].rx;
        p1.moduleId = p2.moduleId = p3.moduleId = p4.moduleId = i;
        temp_y.push_back(p1); temp_y.push_back(p2);
        temp_x.push_back(p3); temp_x.push_back(p4);
    }

    // sorting into acesding order (from small to large)

    sort(temp_y.begin(), temp_y.end(), cmp_sort_point());
    sort(temp_x.begin(), temp_x.end(), cmp_sort_point());

    // eliminate duplicate for sorted x coordinates and y coordinates

    for(i=0;i<(int)temp_y.size();i++){
        Point p = temp_y[i];
        if(i == 0){ sort_y.push_back(p); }
        else if (p.coordinate != temp_y[i-1].coordinate){ sort_y.push_back(p); }

        if(modules_info[p.moduleId].y == p.coordinate)
            modules_info[p.moduleId].lyi = sort_y.size()-1;
        else modules_info[p.moduleId].ryi = sort_y.size()-1;
    }

    for(i=0;i<(int)temp_x.size();i++){
        Point p = temp_x[i];
        if(i == 0){ sort_x.push_back(p); }
        else if (p.coordinate != temp_x[i-1].coordinate){ sort_x.push_back(p); }

        if(modules_info[p.moduleId].x == p.coordinate)
            modules_info[p.moduleId].lxi = sort_x.size()-1;
        else modules_info[p.moduleId].rxi = sort_x.size()-1;
    }

    // col_max is used to store the maximum
    int FPMatrix[sort_x.size()][sort_y.size()];
    for(i=0;i<(int)sort_x.size();i++){
        for(j=0;j<(int)sort_y.size();j++){
            FPMatrix[i][j] = -1;
        }
    }

    int col_max[sort_x.size()];
    for(i=0;i<(int)sort_x.size();i++) col_max[i] = 0;

    for(i=0;i<modules_N;i++){
        for(j=modules_info[i].lxi;j<modules_info[i].rxi;j++){
            for(k=modules_info[i].lyi;k<modules_info[i].ryi;k++){
                FPMatrix[j][k] = i;
            }
            col_max[j] = max(col_max[j], modules_info[i].lyi);
        }
    }

    TotalWasteArea = 0;
    for(j=0;j<(int)sort_x.size()-1;j++){
        for(k=0;k<col_max[j];k++){
            if (FPMatrix[j][k] == -1){
                double x = sort_x[j].coordinate, rx = sort_x[j+1].coordinate;
                double y = sort_y[k].coordinate, ry = sort_y[k+1].coordinate;
                double area = (rx - x)*(ry - y);
                EmptyBox D;
                D.x = x; D.y = y; D.rx = rx; D.ry = ry;
                //boxes.push_back(D);
                TotalWasteArea += area;
            }
        }
    }
    //cout << "dummy size = " << dummies.size() << endl;
    // added by unisun, 2005-07-15
    // use contour to capture the waste sapce
   int p = contour_root;
    //double sH = Height, tH;
    //int s = -1;
    double preH, nextH;
    vector<int> collec;
    boxes.clear();
    while(1){
      if (p == NIL) break;
      int preP = contour[p].back, nextP = contour[p].front;
      if (preP == NIL) preH = Height;
      else preH = modules_info[preP].ry;
      if (nextP == NIL) nextH = Height;
      else nextH = modules_info[nextP].ry;

      if (modules_info[p].ry < preH && modules_info[p].ry < nextH){
        double x,y,rx,ry;
        if (preP != NIL){
          x = max(modules_info[preP].rx, modules_info[p].x);
        }else x = modules_info[p].x;

        if (nextP != NIL){
          rx = min(modules_info[nextP].x, modules_info[p].rx);
        }else rx = modules_info[p].rx;

        y = modules_info[p].ry;
        double maxH = max(preH, nextH);
        ry = maxH;
        EmptyBox D;
        D.x = x; D.y = y; D.rx = rx; D.ry = ry;
        boxes.push_back(D);
        assert(rx >= x && ry >= y);
        TotalWasteArea += 0.5*(rx - x)*(ry - y);
      }
      p = contour[p].front;
    }

  /*if (boxes.size() > 0){
    cout << "TotalWasteArea = " << TotalWasteArea << endl;
    cout << "size = " << boxes.size() << endl;
    //cout << "collec size = " << collec.size() << endl;

    // temp, show modules and dummies;
    ofstream fo;
    fo.open("ttt.plt");
    fo << "set xlabel 'width'" << endl;
    fo << "set ylabel 'height'" << endl;
    fo << "plot \"m.dat\" using 1:2 title \"macros\" with lines lt 1 lw 2, \"d.dat\" using 1:2 title \"dummies\" with lines lt 2 lw 1" << endl;
    fo << endl;
    fo.close();

    fo.open("m.dat");
    for(i=0;i<modules_N;i++){

      fo << modules_info[i].x << " " << modules_info[i].y << endl;
      fo << modules_info[i].rx << " " << modules_info[i].y << endl;
      fo << modules_info[i].rx << " " << modules_info[i].ry << endl;
      fo << modules_info[i].x << " " << modules_info[i].ry << endl;
      fo << modules_info[i].x << " " << modules_info[i].y << endl;
      fo << endl << endl;
    }
    fo << "EOF" << endl;
    fo.close();

    fo.open("d.dat");
    for(i=0;i<boxes.size();i++){
      fo << boxes[i].x << " " << boxes[i].y << endl;
      fo << boxes[i].rx << " " << boxes[i].y << endl;
      fo << boxes[i].rx << " " << boxes[i].ry << endl;
      fo << boxes[i].x << " " << boxes[i].ry << endl;
      fo << boxes[i].x << " " << boxes[i].y << endl;
      fo << endl << endl;
    }
    fo.close();

    //exit(0);
  }*/
}
                              

///////////////////////////////////////////////////////////////

// 2005-09-07 (donnie)
void B_Tree::SetPreplacedRoot( int id )
{
    preplaced_root = id;
}

void B_Tree::FixTree()
{
    if( modules_N <= 1 )
	return;

    // Fix root    
    if( preplaced_root != -1 && nodes_root != preplaced_root )
    {
	//printf( "swap nodes %d %d\n", nodes_root, preplaced_root );
	swap_node2( nodes[nodes_root], nodes[preplaced_root] );
    }

    // Fix parent/child relation. (Must follow the sequence.)
    for( int i=0; i<(int)relations.size(); i++ )
    {
	int parent = relations[i].parent;
	int child  = relations[i].child;
	if( relations[i].type == node_relation::X )
	{
	    if( nodes[parent].left != NIL )
	    {
		swap_node2( nodes[child], nodes[ nodes[parent].left ] );
	    }
	    else
	    {
		delete_node( nodes[child] );
		insert_node( nodes[parent], nodes[child], 0 );  // left
		//printf( "insert %d to %d's left\n", child, parent );
	    }
	}
	else
	{
	    if( nodes[parent].right != NIL )
	    {
		swap_node2( nodes[child], nodes[ nodes[parent].right ] );
	    }
	    else
	    {
		delete_node( nodes[child] );
		insert_node( nodes[parent], nodes[child], 1 );  // right
		//printf( "insert %d to %d's right\n", child, parent );
	    }
	}
    }
}

double B_Tree::getOrientationCost()
{
    double cost = 0;
    //double cx, cy;
    double w, h;
    for( int i=0; i<(modules_N-nFixedBlocks); i++ )
    {
	//if( strncmp( modules[i].name,  "C_", 2 ) == 0 )
	//    continue;	// do not consider clusters
	
	w = modules[i].width;
	h = modules[i].height;
	/*if( nodes[i].rotate )
	    swap( w, h );
	
	cx = modules[i].x + 0.5 * w;
        cy = modules[i].y + 0.5 * h;
	
	if( cx == cy )
	    continue;
	*/
	if( !nodes[i].rotate ) // close to X-axis
	{
	    if( h / w > MAX_ASPECT_RATIO )
		cost += (h / w - MAX_ASPECT_RATIO );
	}
	else	    // close to Y-axis
	{
	    if( w / h > MAX_ASPECT_RATIO )
		cost += (w / h - MAX_ASPECT_RATIO );
	}
    }
    return cost;
}

/////////////////////////////////////////////////////////////////////

struct PPoint
{
    double x, y;
    int id;
};

class cmp_point
{
    public:
	bool operator()(PPoint p1, PPoint p2){
	    return p1.x < p2.x;
	}
};        

void B_Tree::WasteAreaTest(){
    unsigned i;
    vector<PPoint> points1, points2;

    for(i=0;i<modules.size();i++){
	PPoint p;
	p.x = modules_info[i].rx;
	p.y = modules_info[i].ry;
	p.id = i;
	points1.push_back(p);
	points2.push_back(p);
    }

    sort(points1.begin(), points1.end(), cmp_point());

    int p;
    int root_point = NIL, end_point = NIL;
    vector<Contour> point_contour;
    point_contour.resize(modules.size());
    for(i=0;i<points1.size();i++){
	//double rx = points1[i].x;	//indark
	double ry = points1[i].y;
	p = points1[i].id;
	if (i == 0){
	    root_point = end_point = p;
	    point_contour[p].front = point_contour[p].back = NIL;
	}else{
	    if (ry >= points2[end_point].y){
		int loc = end_point;
		while(1){
		    if (points2[loc].y > ry) break;
		    if (loc == NIL) break;
		    loc = point_contour[loc].back;
		}
		if (loc == NIL){
		    root_point = end_point = p;
		    point_contour[p].back = point_contour[p].front = NIL;
		}else{
		    point_contour[loc].front = p;
		    point_contour[p].back = loc;
		    point_contour[p].front = NIL;
		    end_point = p;
		}
	    }else{
		point_contour[end_point].front = p;
		point_contour[p].back = end_point;
		point_contour[p].front = NIL;
		end_point = p;
	    }
	}// end if 
    }

    double waste = 0;
    p = root_point;
    while(1){
	if (p == NIL) break;
	if (p == root_point) waste += modules_info[p].rx * modules_info[p].ry;
	else{
	    int pre_p = point_contour[p].back;
	    assert(pre_p != NIL);
	    waste += (modules_info[p].rx - modules_info[pre_p].rx) * modules_info[p].ry;
	}
	p = point_contour[p].front;
    } 
    TotalWasteArea = waste;
}


