#include "macrocluster.h"
#include "ParamPlacement.h"

CMacroCluster::CMacroCluster()
{
	
}


CMacroCluster::~CMacroCluster()
{
}


bool CMacroCluster::isMacro( int pdb_moduleID )
{
	//return 	(m_pDB->m_modules[pdb_moduleID].m_area > m_MacroAreaRatio * m_avgArea ) && (!m_pDB->m_modules[pdb_moduleID].m_isOutCore );
	return 	(m_pDB->m_modules[pdb_moduleID].m_area > m_MacroAreaRatio * m_avgArea ) && (!m_pDB->BlockOutCore( pdb_moduleID ) );
}

CMacroCluster::CMacroCluster( CPlaceDB & pDB )
{
	m_MacroRowHeight = 2;
	m_MacroAreaRatio = 25;
	gArg.GetInt("macrorowheight",&m_MacroRowHeight) ;
	gArg.GetInt("macroarearatio",&m_MacroAreaRatio);
	m_pDB = &pDB;
	CalcAvgArea( );
	
	//find all macros
	for (unsigned int i = 0 ; i < m_pDB->m_modules.size(); i++){
		Module& _module = m_pDB->m_modules[i];
		if (isMacro( i)){
			m_MacroList.push_back(i);
		
			_module.m_isMacro = true;
			printf (" Macro %d \n" ,i);
		}
		
	}
	
	
	//NOTE: this searching method encodes the smallest module to be the representitive 
	for (unsigned int i = 0 ; i < m_MacroList.size(); i++ ){
		Module& _module = m_pDB->m_modules[m_MacroList[i]];
		int _moduelID1 = m_MacroList[i];
		// whole dimension matching
		if (_module.m_isFixed) 
			continue;
		for (unsigned int j = i+1 ; j < m_MacroList.size(); j++ ){
			Module& _module2 = m_pDB->m_modules[m_MacroList[j]];
			int _moduelID2 = m_MacroList[j];
			if (_module2.m_isFixed) 
				continue;
			if (_module.m_width == _module2.m_width && _module.m_height == _module2.m_height ||
						 _module.m_height == _module2.m_width && _module.m_height == _module2.m_width
						 || _module.m_height == _module2.m_height
			   )
			{
				if (m_ClusteredSet.find (_moduelID1) != m_ClusteredSet.end() || 
				m_ClusteredSet.find (_moduelID2) != m_ClusteredSet.end()   )
				{
					;
				}else{
					printf ("Cluster Found %d %d \n", _moduelID1,_moduelID2);
					m_ClusteredSet.insert(_moduelID1);
					m_ClusteredSet.insert(_moduelID2);
					Cluster(_moduelID1,_moduelID2);
				}
			}
		}
	}
	
	
	
	DoClustering( );
}

void CMacroCluster::CalcAvgArea( )
{
	double _area = 0.0;
	int _count = 0;
	
	for (unsigned int i = 0 ; i < m_pDB->m_modules.size(); i++){
		//if (!m_pDB->m_modules[i].m_isOutCore && !m_pDB->m_modules[i].m_isFixed){
		if (!m_pDB->BlockOutCore( i ) && !m_pDB->m_modules[i].m_isFixed){
			_count++;
			_area += m_pDB->m_modules[i].m_area;
		}
			 
	}
	m_avgArea = _area / _count;
}

void CMacroCluster::Cluster(int representive, int clustered)
{
	if (m_ClusterMap.find(representive) == m_ClusterMap.end() ){	// starting a cluster
		m_ClusterModule.push_back(m_pDB->m_modules[representive]);
		m_ClusterMap[representive] = m_Clusters.size();
		m_Clusters.resize(m_ClusterMap[representive] +1 );
		m_Clusters[m_ClusterMap[representive]].push_back(representive);
		m_Clusters[m_ClusterMap[representive]].push_back(clustered);
		
	}else{
		m_Clusters[m_ClusterMap[representive]].push_back(clustered);
		
	}
	
	
	
	
	
}

void CMacroCluster::DoClustering( )
{
	for(unsigned int i = 0 ; i < m_Clusters.size(); i++ ) 	//for each set of cluster
	{
		int _rp_moduleID= m_Clusters[i][0];
		m_pDB->SetModuleOrientation( _rp_moduleID, 0); //before clustering all modules should be rotated to N
		Module& _rp = m_pDB->m_modules[_rp_moduleID];
		double _width_offset = _rp.m_width * m_Clusters[i].size() / 2.0;
		double _cx = _rp.m_cx;
		double _cy = _rp.m_cy;
		for(unsigned int j = 0 ; j < m_Clusters[i].size(); j++ ) {
// 			printf (" m_Clusters[i][j] = %d \n", m_Clusters[i][j]);
			m_pDB->SetModuleOrientation( m_Clusters[i][j], 0); //before clustering all modules should be rotated to N
			m_pDB->MoveModuleCenter( m_Clusters[i][j], (j + 0.5 ) * _rp.m_width - _width_offset ,0.0); // move clustering module to original
			
		}

// 		m_pDB->OutputGnuplotFigureWithZoom( "DEBUG",false, true, true, true, true);
		// do some modification with the representive 
		_rp.m_width *= m_Clusters[i].size();
		_rp.m_area *= m_Clusters[i].size();
		
		
		//this for loop changes the pin offsets and moduleID
		for(unsigned int j = 0 ; j < m_Clusters[i].size(); j++ ) {
			// for each clustered macro, change pins 
			Module& _cl = m_pDB->m_modules[m_Clusters[i][j]];
			
			for(unsigned int k = 0 ; k < _cl.m_pinsId.size(); k++ ) {
				m_pDB->m_pins[_cl.m_pinsId[k]].xOff = m_pDB->m_pins[_cl.m_pinsId[k]].absX;
				m_pDB->m_pins[_cl.m_pinsId[k]].yOff = m_pDB->m_pins[_cl.m_pinsId[k]].absY;
				m_pDB->m_pins[_cl.m_pinsId[k]].moduleId = _rp_moduleID;
				//add  pinids to clusted macro
				if ( j > 0 ){
					_rp.m_pinsId.push_back( _cl.m_pinsId[k]);
				}
			}
			//add NetIDs pinids to clusted macro
			for(unsigned int k = 0; k < _cl.m_netsId.size(); k++ ) {
				if ( j > 0 ){
					_rp.m_netsId.push_back( _cl.m_netsId[k]);
				}

			}
			
			
			
			if ( j > 0 ){
				///FIXME: please check if this is required
				_cl.m_pinsId.clear();
				_cl.m_netsId.clear();
				
				_cl.m_width = 0.0;
				_cl.m_height = 0.0;
				_cl.m_area = 0.0;
				_cl.m_cx = _cx;
				_cl.m_cy = _cy;
			}
		}
		
		m_pDB->MoveModuleCenter( _rp_moduleID, _cx, _cy);
		
		
		
		

		
	}
}

