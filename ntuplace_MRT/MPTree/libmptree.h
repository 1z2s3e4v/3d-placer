#ifndef LIB_MPTREE_H
#define LIB_MPTREE_H
#include <vector>
#include <iostream>
using namespace std;

class mptree_module
{
	public:
// 	mptree_module()
// 		{
// 			is_fixed = is_corner = is_pad = no_rotate = false;
// 			is_dummy   = false;
// 			create_bkg = true;
// 			is_extra   = false;
// 			id = -1;
// 			x = y = area = width = height = -1;
// 			type = MT_HARD;
// 		};
// 	
// 		int id;
		string name;
		double width, height;
		double x,y;
		double cx, cy;
		int placedb_moduleID;
// 		double area;
		bool is_fixed;	    // fixed block or blockage. we should not change its coordinates
// 		bool is_corner;	    // corner block. set it as a root of the b*-tree
 		bool no_rotate;	    // no rotate for a block
		bool rotate;	    // current orientation, 2005-10-20 (donnie)
// 		bool create_bkg;    // create blockage is fixed && !dummy
// 		bool is_dummy;	    // create a dummy node corresponding to the fixed block/blockage
// 		bool is_pad;
// 		bool is_extra;      // extra block from the setting file
// 		set<int> nets;	     // nets for bookshelf format (group "pseudo nets" in MTK proj)
// 		vector<size> sizes;  // 2005-09-07: candidate sizes for MT_CLUSTER
// 		vector<int>  layers; //             the corresponding layers for "sizes"
// 		int currentLayer;
// 
// 		vector<int> group_id;	// 2005-11-22
	static bool Greater(const mptree_module& m1,const mptree_module& m2){
// 		cout << m1.name << "," << m2.name <<endl;
// 		cout << m1.y << "," << m2.y <<endl;
		return (m1.y > m2.y);
		
	};
	static bool Lesser(const mptree_module& m1, const mptree_module& m2){
		//return (m1.y <= m2.y);
		return !Greater(m1,m2);
		
	};
};

class mptree_info
{
	public:
	double Rgn_Top;
	double Rgn_Bottom;
	double Rgn_Left;
	double Rgn_Right;
	
	double Row_Height;
	
	int partID;

	
	void ShowInfo(){
		cout << "\tRgn_Top:" <<Rgn_Top <<endl;
		cout << "\tRgn_Bottom:" <<Rgn_Bottom <<endl;
		cout << "\tRgn_Left:" <<Rgn_Left <<endl;
		cout << "\tRgn_Right:" <<Rgn_Right <<endl;
	}
};


class MPTree
{
	public:

		void AddMoudle(mptree_module& mod);
		void Plot();
		void Optimize();
		void SetInfo(mptree_info& info);
		void SortModules();
		
	//private:
		vector<mptree_module> m_modules;
		vector<mptree_module> m_fixmodules;
		mptree_info m_info;

		
};





#endif

