#include "mptreelegal.h"
#include "ParamPlacement.h"

#include <cstdio>
#include <algorithm>

using namespace std;


CMPTreeLegal::~CMPTreeLegal()
{
}

CMPTreeLegal::CMPTreeLegal( CPlaceDB * pDB )
{
	m_pDB = pDB;
	m_macroNumber = 0;
}

void CMPTreeLegal::Init( int ratio, int level)
{
    printf( "MP-tree Legalizer Initialization (ratio %d)\n", ratio );

// 	double _width = m_pPart->right - m_pPart->left;
// 	double _height = m_pPart->top - m_pPart->bottom;
// 	double _ratio =0.15;
	mptree_module mptmod;
	mptree_info mpinfo;
// 	cout << "MPTree:" << m_partID << endl;

	printf( "Add fixed modules\n" );	
	int fixedModuleCounter = 0;
	for (unsigned int i =0 ; i <m_pDB->m_modules.size(); i++)
	{
		int id = i;
		bool& moduleIsFixed = m_pDB->m_modules[id].m_isFixed;
		//bool& moduleIsOutCore = m_pDB->m_modules[id].m_isOutCore;
		bool moduleIsOutCore = m_pDB->BlockOutCore( id );
		bool moduleSizeIsEnough = m_pDB->m_modules[id].m_area > 2;
		if( moduleIsFixed && !moduleIsOutCore && moduleSizeIsEnough )
		{
			///FIXME: the area > 2 is not a good criteria
			mptmod.cx = m_pDB->m_modules[id].m_cx;
			mptmod.cy = m_pDB->m_modules[id].m_cy;
			mptmod.x  = m_pDB->m_modules[id].m_x;
			mptmod.y  = m_pDB->m_modules[id].m_y;
			mptmod.width   = m_pDB->m_modules[id].m_width;
			mptmod.height  = m_pDB->m_modules[id].m_height;
			mptmod.name    = m_pDB->m_modules[id].m_name;
			mptmod.placedb_moduleID = id;
			mptmod.is_fixed = true;
			mptmod.no_rotate = true;
			mptmod.rotate = false;

			mpt.AddMoudle( mptmod );
			fixedModuleCounter++;
			printf( "   %s (%f,%f)\n", m_pDB->m_modules[id].m_name.c_str(), mptmod.x, mptmod.y );
		}
	}
	printf( "   Total %d fixed module(s)\n", fixedModuleCounter );
	

	mpt.SortModules();
	
	//calculate average area
	double sum_area = 0.0;
	int n_movable =0;
	for(unsigned int i = 0 ; i < m_pDB->m_modules.size() ; i++)
	{
		//if ((!m_pDB->m_modules[i].m_isFixed) && (!m_pDB->m_modules[i].m_isOutCore))
		if ((!m_pDB->m_modules[i].m_isFixed) && (!m_pDB->BlockOutCore(i)))
		{
			sum_area += m_pDB->m_modules[i].m_area;
			n_movable++;
		}
	}
	double avg_area = sum_area / n_movable;

	printf( "Add placeable modules\n" );	
	/*int*/ m_macroNumber = 0;
	double placeArea = 0.0;
	for(unsigned int i = 0 ; i < m_pDB->m_modules.size() ; i++)
	{
		int id = i;
/*		if (( m_pDB->m_modules[id].m_width > _ratio * _width || 
		     m_pDB->m_modules[id].m_height > _ratio * _height )&& 
		     m_pDB->m_modules[id].m_height >(  m_pDB->m_rowHeight * _macroheight)){*/
//		if ( m_pDB->m_modules[id].m_height >(  m_pDB->m_rowHeight * _macroheight)){

		//area ratio
		bool moduleSizeIsEnough = m_pDB->m_modules[id].m_area > ratio * avg_area;
		bool& moduleIsFixed = m_pDB->m_modules[id].m_isFixed;
		bool& moduleIsClustered = m_pDB->m_modules[id].m_isCluster;
		if( moduleSizeIsEnough && !moduleIsFixed && !moduleIsClustered )
		{
		     	
			mptmod.cx = m_pDB->m_modules[id].m_cx;
			mptmod.cy = m_pDB->m_modules[id].m_cy;
			mptmod.x  = m_pDB->m_modules[id].m_x;
			mptmod.y  = m_pDB->m_modules[id].m_y;

			mptmod.width   = m_pDB->m_modules[id].m_width;
			mptmod.height  = m_pDB->m_modules[id].m_height;
			mptmod.name    = m_pDB->m_modules[id].m_name;
			mptmod.is_fixed = false;
			mptmod.placedb_moduleID = id;
			mptmod.no_rotate = (!param.bMPTreeRotate);
			mptmod.rotate = false;
			mpt.AddMoudle( mptmod );
	//		cout << "Found:" << m_pDB->m_modules[id].m_name << 
	//			"\t" << mptmod.x << "," << mptmod.y <<
	//			"\t" << mptmod.width << "," << mptmod.height << endl;
			m_macroNumber++;
			placeArea += m_pDB->m_modules[id].m_area;
			
		}
	}
	printf( "   Total %d placeable modules\n", m_macroNumber );
	printf( "   Placeable module area / total movable area = %.2f\n\n\n", 100.0*placeArea/sum_area );
	
	mpinfo.Rgn_Top = m_pDB->m_coreRgn.top;
	mpinfo.Rgn_Bottom  = m_pDB->m_coreRgn.bottom;
	mpinfo.Rgn_Left = m_pDB->m_coreRgn.left;
	mpinfo.Rgn_Right= m_pDB->m_coreRgn.right;
	mpinfo.Row_Height = m_pDB->m_rowHeight;
	mpinfo.partID = level;
	//mpinfo.ShowInfo();
	
	if( m_macroNumber > 0)
	{
		mpt.SetInfo(mpinfo);
		mpt.Plot();
	}
}

void CMPTreeLegal::Optimize()
{
    if( m_macroNumber > 0 )
	mpt.Optimize();
}

int CMPTreeLegal::Update(bool setfixed )
{
/*	cout << "B:m_pPart->moduleList.size():"<<m_pPart->moduleList.size() <<endl;
	cout << "B:m_pPart->fixmoduleList.size():"<<m_pPart->fixModuleList.size() <<endl;*/
// 	int _pre = m_pPart->fixModuleList.size();
	int _pre = 0;
	int old_orient;
	int new_orient;
	int try_orient;
	double Best_HPWL;
	double _HPWL;
	int Best_orient;
				
	
	vector<int>::iterator ite;
// 	m_pPart->fixModuleList.clear();
	for (unsigned int i =0 ; i < mpt.m_modules.size(); i++){		
		mptree_module& mptmod = mpt.m_modules[i];		//update placedb
		if (param.bMPTreeRotate){
			Best_HPWL = 1E37;
			Best_orient = -1;
			
			
			old_orient = m_pDB->GetModuleOrient(mptmod.placedb_moduleID);
//			cout << m_pDB->m_modules[mptmod.placedb_moduleID].GetName() << ":" << old_orient << endl;
			if (mptmod.rotate)
				new_orient = (old_orient - (old_orient % 2) + ((old_orient +1)  % 2));
			else
				new_orient = old_orient;
			for( int j = 0 ; j < 4 ;j++){
				try_orient = (new_orient + j * 2) % 8;
				m_pDB->SetModuleOrientation(mptmod.placedb_moduleID, try_orient);
				_HPWL = m_pDB->CalcHPWL();
//				cout << "Trying:" << try_orient << " " << _HPWL << endl;
				if(  _HPWL < Best_HPWL ){
					Best_HPWL = _HPWL;
					Best_orient = try_orient;
				}
			}
						
			m_pDB->SetModuleOrientation(mptmod.placedb_moduleID, Best_orient);
			
			
//			cout << m_pDB->m_modules[mptmod.placedb_moduleID].GetName() << ": ### " <<
//					m_pDB->GetModuleOrient(mptmod.placedb_moduleID) << endl;
		}
		m_pDB->MoveModuleCenter(mptmod.placedb_moduleID,
					mptmod.cx,mptmod.cy);
		if(setfixed)
			m_pDB->SetModuleFixed(mptmod.placedb_moduleID);
		
// 		m_pPart->fixModuleList.push_back(mpt.m_modules[i].placedb_moduleID);	//update fix module list
		
// 		ite = find(m_pPart->moduleList.begin(),m_pPart->moduleList.end(), mpt.m_modules[i].placedb_moduleID);
// 		if (ite != m_pPart->moduleList.end()){
// 			cout << "To find:"<<*ite << endl;
// 			m_pPart->moduleList.erase(ite);
// 		}
		
	}
	
// 	cout << "A:m_pPart->moduleList.size():"<<m_pPart->moduleList.size() <<endl;
// 	for(unsigned int i = 0 ; i < m_pPart->moduleList.size() ; i++){
// // 		cout << "m_pPart->moduleList:"<<m_pPart->moduleList[i] <<endl;
// 	}
// 	cout << "A:m_pPart->fixmoduleList.size():"<<m_pPart->fixModuleList.size() <<endl;
// 	for(unsigned int i = 0 ; i < m_pPart->fixModuleList.size() ; i++){
// // 		cout << "m_pPart->fixmoduleList:"<<m_pPart->fixModuleList[i] <<endl;
// 	}
	return mpt.m_modules.size()- _pre;
// 	
	
	
}

// void CMPTreeLegal::Init4Part(int start_partID, int end_partID ,vector<CPartition>& pPartArray)
// 
// {
// 	double _width = pPartArray[end_partID].top -pPartArray[start_partID].bottom;
// 	double _height =  pPartArray[end_partID].right - pPartArray[start_partID].left;
// 	double _ratio =0.15;
// 	CPartition* _pPart;
// 	mptree_module mptmod;
// 	mptree_info mpinfo;
// 	
// 	//fixed macros
// 	for ( int k = start_partID ; k <= end_partID ; k++){
// 		_pPart = &pPartArray[k];
// 		for (unsigned int i =0 ; i <_pPart->fixModuleList.size(); i++){
// 			int id = _pPart->fixModuleList[i];
// 				mptmod.cx = m_pDB->m_modules[id].m_cx;
// 				mptmod.cy = m_pDB->m_modules[id].m_cy;
// 				mptmod.x  = m_pDB->m_modules[id].m_x;
// 				mptmod.y  = m_pDB->m_modules[id].m_y;
// 				mptmod.width   = m_pDB->m_modules[id].m_width;
// 				mptmod.height  = m_pDB->m_modules[id].m_height;
// 				mptmod.name    = m_pDB->m_modules[id].m_name;
// 				mptmod.placedb_moduleID = id;
// 				mptmod.is_fixed = true;
// 	
// 				mpt.AddMoudle( mptmod );
// 				cout << "Found Fixed Module:" << m_pDB->m_modules[id].m_name << 
// 					"\t" << mptmod.x << "," << mptmod.y << endl;
// 		}
// 	}
// 	//sorting
// 		mpt.SortModules();
// 	// movable macros
// 	for ( int k = start_partID ; k <= end_partID ; k++){
// 		_pPart = &pPartArray[k];
// 		for(unsigned int i = 0 ; i < _pPart->moduleList.size() ; i++){
// 			int id = _pPart->moduleList[i];
// 			if (( m_pDB->m_modules[id].m_width > _ratio * _width || 
// 				m_pDB->m_modules[id].m_height > _ratio * _height )&& 
// 				m_pDB->m_modules[id].m_height >(  m_pDB->m_rowHeight * 1)){
// 				cout << "Cri: "<< (  m_pDB->m_rowHeight * 1) << endl;
// 				mptmod.cx = m_pDB->m_modules[id].m_cx;
// 				mptmod.cy = m_pDB->m_modules[id].m_cy;
// 				mptmod.x  = m_pDB->m_modules[id].m_x;
// 				mptmod.y  = m_pDB->m_modules[id].m_y;
// 				mptmod.width   = m_pDB->m_modules[id].m_width;
// 				mptmod.height  = m_pDB->m_modules[id].m_height;
// 				mptmod.name    = m_pDB->m_modules[id].m_name;
// 				mptmod.is_fixed = false;
// 				mptmod.placedb_moduleID = id;
// 				mpt.AddMoudle( mptmod );
// 				cout << "Found:" << m_pDB->m_modules[id].m_name << 
// 					"\t" << mptmod.width << "," << mptmod.height << endl;
// 			}
// 						
// 		}
// 
// 		
// 	}
// 	
// 	mpinfo.Rgn_Top = pPartArray[end_partID].top;
// 	mpinfo.Rgn_Bottom  = pPartArray[start_partID].bottom;
// 	mpinfo.Rgn_Left =  pPartArray[start_partID].left;
// 	mpinfo.Rgn_Right=  pPartArray[end_partID].right;
// 	mpinfo.Row_Height = m_pDB->m_rowHeight;
// 	mpinfo.partID = -((end_partID-2)/4);
// 	mpt.SetInfo(mpinfo);
// 	mpt.Plot();
// 	mpt.Optimize();
// 	
// 	
// }
// 
// int CMPTreeLegal::Update4Part( int start_partID, int end_partID, vector< CPartition > & pPartArray )
// {
// 	int _pre ;
// 	vector<int>::iterator ite;
// 	CPartition* _pPart;
// 	
// 	for ( int k = start_partID ; k <= end_partID ; k++){
// 		_pPart = &pPartArray[k];
// 		_pPart->fixModuleList.clear();
// 		_pre = _pPart->fixModuleList.size();
// 		for (unsigned int i =0 ; i < mpt.m_modules.size(); i++){		
// 			mptree_module& mptmod = mpt.m_modules[i];		//update placedb
// 			m_pDB->MoveModuleCenter(mptmod.placedb_moduleID,
// 						mptmod.cx,mptmod.cy);
// 			m_pDB->SetModuleFixed(mptmod.placedb_moduleID);
// 			
// 			_pPart->fixModuleList.push_back(mpt.m_modules[i].placedb_moduleID);	//update fix module list
// 			
// 			ite = find(_pPart->moduleList.begin(),_pPart->moduleList.end(), mpt.m_modules[i].placedb_moduleID);
// 			if (ite != _pPart->moduleList.end()){
// 				cout << "Erasing..."<<*ite << endl;
// 				_pPart->moduleList.erase(ite);
// 			}
// 			
// 		}
// 	}
// 	return mpt.m_modules.size();
// }
// 




