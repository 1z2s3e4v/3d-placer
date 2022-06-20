#include "libmptree.h"
#include "CornerTree.h"
#include "mptreeutil.h"
#include <algorithm>
using namespace N_ntumptree;

void MPTree::AddMoudle(mptree_module& mod){

	m_modules.push_back(mod);
}

void MPTree::SetInfo(mptree_info& info){
	m_info = info;
}
		
void MPTree::SortModules( )
{
	if (m_modules.size() > 0 ){
		vector<mptree_module>::iterator ite;
		ite = m_modules.begin();
		sort(m_modules.begin(),m_modules.end(), mptree_module::Lesser);
		while (ite->y < (m_info.Rgn_Top + m_info.Rgn_Bottom)/2){
			ite++;
		}
		sort(ite,m_modules.end(), mptree_module::Greater);
	}
}






void MPTree::Plot(){
	CornerTree CT;

	char filename[100];

	if ( m_modules.size() > 0 ){
		// 		for(int i = 0 ; i < m_modules.size(); i++){
		// 			printf("MPMOD: %d (%f,%f)\n",i, m_modules[i].x,m_modules[i].y);
		// 		}

		cout << "MPTree::Plot() AddModules" << endl;
		CT.AddModules( m_modules,m_info);
		//		CT.CreateBTrees();
		//	cout << "ListModules()" << endl;
		//CT.ListModules();

		sprintf(filename, "CT-init-%d.plt",m_info.partID);
		CT.OutputGNUplot( filename,-1,false);

	}
	// 	
}




void CornerTree::AddModules( vector<mptree_module>& mpmods, mptree_info& info){


	Module dummyM;	
	Module_Info dummyM_info;

	m_modules.clear();
	m_modulesInfo.clear();
	m_matching.clear();

	dummyM_info.rotate= false;	
	dummyM_info.flip= false;	

	m_offset_X = info.Rgn_Left;
	m_offset_Y = info.Rgn_Bottom;

	for (unsigned int i = 0 ; i < mpmods.size() ; i++){
		// 		printf("MPMOD: %d (%f,%f)\n",i, mpmods[i].x,mpmods[i].y);
		if(mpmods[i].is_fixed){

			m_modules.push_back( dummyM );
			m_modulesInfo.push_back( dummyM_info );


			Module& m = m_modules.back();
			//Module_Info& m_info = m_modulesInfo.back();

			strcpy( m.name, mpmods[i].name.c_str() );	    // module name
			m.x = mpmods[i].x;
			m.y = mpmods[i].y;




			m.width  = mpmods[i].width;
			m.height = mpmods[i].height;
			m.area   = mpmods[i].width * mpmods[i].height;
			m.no_rotate = true;	// all module are not allowed to be rotated
			m.is_fixed  = true;
			m.is_pad = true;
			m.id       = m_modules.size()-1;
			m_matching.push_back(-1);
		}
		else{



			m_modules.push_back( dummyM );
			m_modulesInfo.push_back( dummyM_info );


			Module& m = m_modules.back();
			//Module_Info& m_info = m_modulesInfo.back();

			strcpy( m.name, mpmods[i].name.c_str() );	    // module name
			m.x = mpmods[i].x;
			m.y = mpmods[i].y;

			//   			m.x = m_offset_X;
			//   			m.y = m_offset_Y;

			m.width  = mpmods[i].width;
			m.height = mpmods[i].height;
			m.area   = mpmods[i].width * mpmods[i].height;
			m.no_rotate = mpmods[i].no_rotate;
			m.is_fixed  = false;
			m.id       = m_modules.size()-1;
			m_matching.push_back(-1);

			//  			cout << "M:"<< m.x << "," <<  m.y << endl;
		}


	}
	m_stdCoreBottom = 0;
	m_stdCoreTop =  (info.Rgn_Top -info.Rgn_Bottom );
	m_stdCoreLeft = 0;
	m_stdCoreRight = (info.Rgn_Right - info.Rgn_Left);
	
	m_offset_X = info.Rgn_Left;
	m_offset_Y = info.Rgn_Bottom;
	
	info.ShowInfo();

	m_rowHeight = info.Row_Height;

	m_chipWidth = (info.Rgn_Right - info.Rgn_Left);
	m_chipHeight =  (info.Rgn_Top -info.Rgn_Bottom );
	
	OffsetRelocate();

	CUT_Y_LEFT = -1;
	CUT_Y_RIGHT = -1;
	CreateCoreRegion();

}



void MPTree::Optimize( )
{
	CornerTree CT;
	mptree_module mptmod;
	char filename[100];
	int _N_Fix = 0;
	int _N_nonFix = 0;
	vector<mptree_module>::iterator ite;


	if ( m_modules.size() > 0 ){

		for (unsigned int i = 0 ; i < m_modules.size() ; i++){
			if(m_modules[i].is_fixed){
				_N_Fix++;
			}else{
				_N_nonFix++;
			}
		}
		cout <<"_N_nonFix" << endl;
		if (_N_nonFix <= 20 || true) {		//avoid bug
			cout << "Insert Dummy module to avoid bug\n" << endl;
			mptmod.cx = 0.0;
			mptmod.cy = 0.0;
			mptmod.x  = 0.0;
			mptmod.y  = 0.0;
			mptmod.width   = 0.0;
			mptmod.height  = 0.0;
			mptmod.name    = "Dummy-LB";
			mptmod.is_fixed = false;
			mptmod.placedb_moduleID = -1;
			AddMoudle( mptmod );
			AddMoudle( mptmod );
			cout << "Insert Dummy module to avoid bug\n" << endl;
			mptmod.cx = 0.0;
			mptmod.cy = 1E10;
			mptmod.x  = 0.0;
			mptmod.y  = 1E10;
			mptmod.width   = 0.0;
			mptmod.height  = 0.0;
			mptmod.name    = "Dummy-LT";
			mptmod.is_fixed = false;
			mptmod.placedb_moduleID = -1;
			AddMoudle( mptmod );
			AddMoudle( mptmod );
			cout << "Insert Dummy module to avoid bug\n" << endl;
			mptmod.cx = 1E10;
			mptmod.cy = 1E10;
			mptmod.x  = 1E10;
			mptmod.y  = 1E10;
			mptmod.width   = 0.0;
			mptmod.height  = 0.0;
			mptmod.name    = "Dummy-RT";
			mptmod.is_fixed = false;
			mptmod.placedb_moduleID = -1;
			AddMoudle( mptmod );
			AddMoudle( mptmod );
			cout << "Insert Dummy module to avoid bug\n" << endl;
			mptmod.cx = 1E10;
			mptmod.cy = 0.0;
			mptmod.x  = 1E10;
			mptmod.y  = 0.0;
			mptmod.width   = 0.0;
			mptmod.height  = 0.0;
			mptmod.name    = "Dummy-RB";
			mptmod.is_fixed = false;
			mptmod.placedb_moduleID = -1;
			AddMoudle( mptmod );
			AddMoudle( mptmod );




		}



		cout << "MPTree::Plot() AddModules" << endl;
		CT.AddModules( m_modules,m_info);
		CT.ListModules();
		CT.Optimize(10,5);
		sprintf(filename, "CT-%d.plt",m_info.partID);
		CT.OutputGNUplot( filename,-1,false);
		CT.OffsetRestore();


	}



	for(unsigned int i = 0 ; i< m_modules.size();i++){	//location update
		CT.GetModuleXY(i,m_modules[i].cx,m_modules[i].cy);
		CT.GetModuleRotate( i,m_modules[i].rotate);
	}


	for (int i = 0 ; i < 8 ; i++){		//dummy removal
		for (ite = m_modules.begin() ; ite != m_modules.end() ; ++ite){
			if (ite->placedb_moduleID == -1)
				break;
		}
		if (ite != m_modules.end() )
			m_modules.erase(ite);
	}
}







