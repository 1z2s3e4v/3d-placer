
#ifndef LEGAL_H
#define LEGAL_H
//#include "placedb.h"
#include "Prelegalizer.h"
#include "ParamPlacement.h"
#include "util.h"
#include <ctime>
#include <iostream>
#include <set>
#include <vector>
#include <map>

class CRow
{ 
public:
	CRow( float m_x=0, float m_y=0, float m_length=0 ) 
	{
		this->m_x   = m_x ;
		this->m_y  = m_y ;
		this->m_length    = m_length;
	}
	void showspace();
	void showmodule();
	bool check_overlap(float ,float);
	bool insert_module(float , float,int);
	bool add_empty(float,float);// x,w
	int find_ncell(float,float);//start,end , return # of modules between filled space{start,end}
	int clean_empties(); // return # of 0 length empty

	map<float, float> m_empties;//x-coordinate,length
	map<float, int> m_rowmodule; //x-coordinate,module ID
	float m_x, m_y,m_length;

};


class CModule_Info
{
public:
	CModule_Info()
	{
		left=0;
		right=0;
		left_net=-1;
		right_net=-1;
		find=true;
		edgecount=0;

	}
	bool find;
	float left;
	int left_net;
	float right;
	int right_net;
	vector<int> nets;
//	multimap<float,int> netmap;   //int 1=start; 2=end;
	float max_start;
	float max_end;
	int edgecount;
	float cost;
};

class CNetinfo
{
public:
	CNetinfo()
	{
		left=-1;
		right=-1;
		top=-1;
		bottom=-1;
	}
	void show2()
	{
		cout<<" \n top:"<<top_c<<" bot:"<<bottom_c<<" left:"<<left_c<<" right:"<<right_c;
	}
	void add(float ,float  ,float ,float,int);

	void calc2(int ,CPlaceDB& ,vector<CModule_Info>& );  //mod=CLegalizer.moduleinfo
	float calc3(int ,CPlaceDB& ,vector<CModule_Info>&,int );  //mod=CLegalizer.moduleinfo	
	float hpwl()
	{
		return ((right_c-left_c)+(top_c-bottom_c));
	}

	void show(int netID,CPlaceDB& cf);
	void calc(int netID,CPlaceDB& cf);


	int left;   //left most module ID
	int right;  //right most module ID
	int top;
	int bottom;
	float oldhpwl;

	float left_d;  //left distance to second left most module
	float right_d;
	float top_d;
	float bottom_d;

	float left_c;  //left coordinate
	float right_c;
	float top_c;
	float bottom_c;

	vector<int> tops;
	vector<int> bottoms;
	vector<int> lefts;
	vector<int> rights;
};

class CPlaceDB;
class CLegalizer
{
    public:
	CLegalizer( CPlaceDB& cf )
	{
	    fplan = &cf;
	    Init();
	}

	void Init();    // 2005/03/11 (donnie)

	/*--------------------------------------------------------------------
	  legalize( window size) 
          input:global placed cell configuration
          output:legaized cell configuration
         ---------------------------------------------------------------------*/
	bool legalize(int WINDOW, SCALE_TYPE type);

	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// Diamond Macro Legalizer =>legal macors which id exists in set<int> macroSet
	// USAGE:
	// CLegalizer check(placeDB);
	// check.legalize( (int)(max( placeDB.GetHeight(), placeDB.GetWidth() )*1.4/placeDB.m_rowHeight), macroSet );
	//
	//
	//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	bool macroLegal(int WINDOW,const set<int>& macroIDset);
	void PreLegal( float factor ); 
	//void detail();

	bool move_module(float,int,int);//new x position,module ID,row id
	//	float move_module2(float,float,int);//new x position,new y,module ID
	float find_place(int ,float ,float,int ,float&);



	CPlaceDB* fplan; 
	float ROWHEIGHT;
	vector<CRow> core_rows;
	//vector<CModule_Info> moduleinfo;
	//vector<CNetinfo> netinfo;

	//	bool compare(int,int); // return true if a >= b
	map<float,float> mergerow(map<float,float>,map<float,float>); //input id of rows to be merged
	void MoveBlockToLeft( float  );
	void showspace(map<float,float>);
	void showmod(int);
	float botto;
	int num_rows;
	float movetotal;
};

//
//class permutation
//{
//public:
//	permutation(int n,CLegalizer& cf)
//	{
//		cl=&cf;
//		num=n;
//		level=0;
//		acount=0;
//		nodeID=0;
//		for(int i=0;i<num;i++)
//		{
//			vec.push_back(-1);
//			left.push_back(false);
//			root.push_back(-1);
//		}
//		vec[0]=0;
//
//	}
//
//	void init()  //must call after modules vector has been inserted
//	{
//		//cout<<" \n module:";
//		for(unsigned int i=0;i<modules.size();i++)
//		{
//			cl->moduleinfo[modules[i]].find=false;
//			
//			cout<<" "<<modules[i]<<" w:"<<cl->fplan->m_modules[modules[i]].m_width<<" n:"<<cl->fplan->m_modules[modules[i]].m_name;
//			for(unsigned int h=0;h<cl->moduleinfo[modules[i]].nets.size();h++)
//			{
//				
////				netinfo[cf.moduleinfo[i].nets[h]].calc(cf.moduleinfo[i].nets[h],*cf.fplan);
//				netset.insert(cl->moduleinfo[modules[i]].nets[h]);
//			}
//        
//        }
//		flag=false;
//		start_x=cl->fplan->m_modules[modules[0]].m_x; 
//
//
//
//		//calc basic HPWL 
//		base_hpwl=0;
//		set<int>::iterator it;
//		
//
//
//		for(it=netset.begin();it!=netset.end();++it)
//		{
//			cl->netinfo[*it].calc2(*it,*cl->fplan,cl->moduleinfo);
//			base_hpwl=base_hpwl+cl->netinfo[*it].hpwl();
//
//		}
//
//
//
//
////		cout<<" start_x"<<start_x;
//		//cout<<" \n netset:";
//		//set<int>::iterator it;
//		//for(it=netset.begin();it!=netset.end();++it)
//		//{
//		//	cout<<" "<<*it;
//
//		//}
//		//cout<<" [end]";
//	}
//
//
//
//
//	int nodeID;
//	vector<int> vec;
//	vector<int> root;
//	vector<float> obj;
//	vector<bool> left;
//
//	CLegalizer * cl; 
//	int level;
//	int num;
//	int acount;
//
//	float start_x;
//	float hpwl;
//	float base_hpwl;
//	bool flag;
//
//
//	vector<int> best;
//	vector<int> modules;
//	set<int> netset;
//
//	void action();
//	bool calc();
//
//	// go to next state;
//	bool next();
//
//	// find smallest index that does not appear
//	int find();
//	bool find_in(int pos);
//
//};

/*------------------------------------------------------------

detail place log

[2005_02_20] 
*permutation: large window (6) for longer module chain,small (3) for smaller chain
ibm01 2.0119 e+006  -2.098%
ada1  9.43583e+007  -0.973%
ada3  2.74531e+008  -0.567%

[2005_02_22]
*if chain size<6; use chain size for samller chain
ibm01  2.00335e+006 (-2.51443%)
ada1   9.41522e+007 (-1.18962%)
ada3   2.74158e+008 (-0.702393%)


[2005_02_27] modify legalizer
ibm01 1.99639 (+5.3051%)
ada1  9.27163 (+1.74666%)
ada3  2.69863 (+1.1178%)

--------------------------------------------------------------*/
#endif
