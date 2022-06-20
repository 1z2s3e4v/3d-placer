#include "PlaceDBQP.h"
#include <iomanip>

void checkMatrix(CQP& qp, vector<double>& vec_B)
{
	for(unsigned int i=0; i<qp.mapA.size(); i++)
	{
		
		map<int,double>::iterator iter;
		iter=qp.mapA[i].find(i);
		assert(iter!=qp.mapA[i].end());
		assert(iter->second>0);

		double dio=iter->second;
	//	cout<<"\n i:"<<iter->first<<" dio:"<<dio;

		double count=0;

		for(iter=qp.mapA[i].begin(); iter!=qp.mapA[i].end(); iter++)
		{
			assert(iter->first>=0);
			assert((unsigned int)iter->first<qp.mapA.size());
			double v2;
			qp.getValue(iter->first,i,v2);
			assert(v2==iter->second);

			count+=iter->second;
			
		}
		//if(count<0)
		//{
		//	cout<<"\n i:"<<i<<" count:"<<count<<" dio:"<<dio;
		//}
		//assert(count<0);
	//	cout<<" count:"<<count<<"\n";
		if( (dio+count)!=0 )
		{
		//	assert(vec_B[i]!=0);
		}
	}
}
void printMatrix(CQP& qp)
{
	unsigned int size=qp.mapA.size();
	cout<<" Matrix:size:"<<qp.mapA.size()<<"\n";

	for(unsigned int i=0; i<size; i++)
	{
		vector<double> vec;
		vec.resize(size);
		for(unsigned int j=0; j<size; j++)
		{
			vec[j]=0;
		}
		map<int,double>::iterator iter;
//		int count=0;
		for(iter=qp.mapA[i].begin(); iter!=qp.mapA[i].end(); iter++)
		{
			vec[iter->first]=iter->second;
			//cout<<"i:"<<i<<" j:"<<iter->first<<" value:"<<iter->second<<"\n";
		}
		for(unsigned int j=0; j<size; j++)
		{
			cout<<setw(6)<<setprecision(3)<<vec[j]<<"|";
			//if(vec[j]!=-2733.97)
			//	cout<<vec[j]<<" | ";
			//else
			//	cout<<"   | ";
		}
		cout<<"\n";
	}

}




CPlaceDBQPLevel::CPlaceDBQPLevel(void)
{
	m_levelID=-1;
}

CPlaceDBQPLevel::~CPlaceDBQPLevel(void)
{
}


CPlaceDBQPRegionNet::CPlaceDBQPRegionNet(int netID)
{
	this->m_netID=netID;

	this->m_nLeftModules=0;
	this->m_nBottomModules=0;
	this->m_nRightModules=0;
	this->m_nTopModules=0;
	m_nCLeftModules=0; 
	m_nCRightModules=0;
	m_nCTopModules=0;
	m_nCBottomModules=0;

	this->m_isGlobalX=false;
	this->m_isGlobalY=false;

	this->m_localModules.clear();

	this->m_fixedPinX.clear();
	this->m_fixedPinY.clear();
}
void CPlaceDBQPRegionNet::reset()
{
	this->m_nLeftModules=0;
	this->m_nBottomModules=0;
	this->m_nRightModules=0;
	this->m_nTopModules=0;
	m_nCLeftModules=0; 
	m_nCRightModules=0;
	m_nCTopModules=0;
	m_nCBottomModules=0;

	this->m_isGlobalX=false;
	this->m_isGlobalY=false;
	this->m_fixedPinX.clear();
	this->m_fixedPinY.clear();
}
CPlaceDBQPRegionNet::~CPlaceDBQPRegionNet(void)
{
}

CPlaceDBQPRegion::CPlaceDBQPRegion(void)
{

}

CPlaceDBQPRegion::CPlaceDBQPRegion(CPlaceDB& db,CPlaceDBQPPlacer& paqp)
{
	m_pDB = &db;
	m_pPqap = &paqp;

}

CPlaceDBQPRegion::~CPlaceDBQPRegion(void)
{
}


//only run QP on X or Y 


void CPlaceDBQPRegion::setRgn(double top, double bottom, double left, double right)
{
	this->m_Rgn.top=top;
	this->m_Rgn.bottom=bottom;
	this->m_Rgn.left=left;
	this->m_Rgn.right=right;
}

void CPlaceDBQPRegion::initRegionNet()
{
    //----------------------------------------------------
    // Modify log: (tellux) 
    // 2005/11/09: Change the outside region movabel module to movable pin (improve accuracy of large module)
    // 2005/11/09: Add m_regionNets[i].m_nCLeftModules... to give data to hMetis partitioner
    //----------------------------------------------------

    if( m_moduleIDs.size() == 0 ) //if there is no movable module inside
	return;

    set<int> netSet;
    localModuleIDSet.clear();
    realModuleID2regionModuleID.clear();

    //fill netsID into netSet from modules in this region
    for( unsigned int i=0; i<m_moduleIDs.size(); i++ )
    {
	int id = m_moduleIDs[i];
	this->safeMoveCenter( id, m_pDB->m_modules[id].m_cx, m_pDB->m_modules[id].m_cy );
	realModuleID2regionModuleID[id] = i;
	localModuleIDSet.insert( id );

	for( unsigned int j=0; j<m_pDB->m_modules[id].m_netsId.size(); j++ )
	{
	    netSet.insert( m_pDB->m_modules[id].m_netsId[j] );
	}
	//double cx=0.5*(m_Rgn.left+m_Rgn.right);
	//double cy=0.5*(m_Rgn.top+m_Rgn.bottom);
	//m_pDB->SetModuleLocation(m_moduleIDs[i],cx,cy);
    }


    //fill into m_regionNets
    m_regionNets.reserve( netSet.size() );
    set<int>::iterator it;
    for( it=netSet.begin(); it!=netSet.end(); ++it )
    {
	CPlaceDBQPRegionNet rgNet(*it);

	//only consider net degree >1 
	if( m_pDB->m_nets[*it].size() > 1 )
	{
	    m_regionNets.push_back(rgNet);
	}
    }

    //------
    //establish pins and m_localModules
    //------
    double region_centerX = 0.5 * (m_Rgn.left + m_Rgn.right);
    double region_centerY = 0.5 * (m_Rgn.top  + m_Rgn.bottom);

    //for all region nets
    for( unsigned int i=0; i<m_regionNets.size(); i++ )
    {
	//for all cells in the net
	for( unsigned int j=0; j<m_pDB->m_nets[m_regionNets[i].m_netID].size(); j++ )
	{
	    int pinID    = m_pDB->m_nets[m_regionNets[i].m_netID][j];
	    int moduleID = m_pDB->m_pins[pinID].moduleId;

	    //for movable module
	    if( m_pDB->m_modules[moduleID].m_isFixed == false )
	    {
		set<int>::iterator it;
		it = localModuleIDSet.find( moduleID );

		if( it == localModuleIDSet.end() ) //the module is not in the region (use pin positions)
		{
		    double pinX = m_pDB->m_pins[pinID].absX;
		    double pinY = m_pDB->m_pins[pinID].absY;

		    if( pinX <= m_Rgn.left ) //module in the left
		    {
			++m_regionNets[i].m_nLeftModules;					
		    }
		    else if( pinX <= region_centerX )
		    {
			m_regionNets[i].m_fixedPinX.push_back( pinX );
			++m_regionNets[i].m_nCLeftModules;
		    }
		    else if( pinX < m_Rgn.right )
		    {
			m_regionNets[i].m_fixedPinX.push_back( pinX );
			++m_regionNets[i].m_nCRightModules;
		    }
		    else //module in the right
		    {
			++m_regionNets[i].m_nRightModules;					
		    }


		    if( pinY <= m_Rgn.bottom ) //module in the left
		    {
			++m_regionNets[i].m_nBottomModules;					
		    }
		    else if( pinY <= region_centerY )
		    {
			m_regionNets[i].m_fixedPinY.push_back( pinY );
			++m_regionNets[i].m_nCBottomModules;
		    }
		    else if( pinY < m_Rgn.top )
		    {
			m_regionNets[i].m_fixedPinY.push_back( pinY );
			++m_regionNets[i].m_nCTopModules;
		    }
		    else //module in the right
		    {
			++m_regionNets[i].m_nTopModules;					
		    }
		}
		else // the module is inside the region
		{

		    m_regionNets[i].m_localModules.push_back( moduleID );
		    m_regionNets[i].m_localModulesPinID.push_back( pinID );

		}
	    }
	    else //for fixed modules
	    {

		//m_regionNets[i].m_fixedPinX.push_back(m_pDB->m_pins[pinID].absX);
		//m_regionNets[i].m_fixedPinY.push_back(m_pDB->m_pins[pinID].absY);
		double pinX = m_pDB->m_pins[pinID].absX;
		double pinY = m_pDB->m_pins[pinID].absY;

		if( pinX <= m_Rgn.left ) //module in the left
		{
		    ++m_regionNets[i].m_nLeftModules;					
		}
		else if( pinX <= region_centerX )
		{
		    m_regionNets[i].m_fixedPinX.push_back( pinX );
		    ++m_regionNets[i].m_nCLeftModules;
		}
		else if( pinX < m_Rgn.right )
		{
		    m_regionNets[i].m_fixedPinX.push_back( pinX );
		    ++m_regionNets[i].m_nCRightModules;
		}
		else //module in the right
		{
		    ++m_regionNets[i].m_nRightModules;					
		}


		if(pinY<=m_Rgn.bottom) //module in the left
		{
		    ++m_regionNets[i].m_nBottomModules;					
		}
		else if(pinY<=region_centerY)
		{
		    m_regionNets[i].m_fixedPinY.push_back(pinY);
		    ++m_regionNets[i].m_nCBottomModules;
		}
		else if(pinY<m_Rgn.top)
		{
		    m_regionNets[i].m_fixedPinY.push_back(pinY);
		    ++m_regionNets[i].m_nCTopModules;
		}
		else //module in the right
		{
		    ++m_regionNets[i].m_nTopModules;					
		}
	    }
	}

	//determine if the net is global
	if( m_regionNets[i].m_nLeftModules>0 && m_regionNets[i].m_nRightModules>0 )
	{
	    m_regionNets[i].m_isGlobalX = true;
	}
	if( m_regionNets[i].m_nBottomModules>0 && m_regionNets[i].m_nTopModules>0 )
	{
	    m_regionNets[i].m_isGlobalY = true;
	}
    }

    //check
    for(int i=0; i<(int)this->m_regionNets.size(); i++)
    {
	if( m_regionNets[i].m_localModules.size() <= 0 )
	{
	    cout<<"\n=======================ERROR===============NET ID:"<<m_regionNets[i].m_netID<<" \n";
	    cout<<"\n pinSize:"<<m_pDB->m_nets[m_regionNets[i].m_netID].size();
	    for(unsigned int j=0; j<m_pDB->m_nets[m_regionNets[i].m_netID].size(); j++)
	    {
		int pinID=m_pDB->m_nets[m_regionNets[i].m_netID][j];
		int moduleID=m_pDB->m_pins[pinID].moduleId;
		cout<<" mod::"<<moduleID<<" mname:"<<m_pDB->m_modules[moduleID].m_name;
		set<int>::iterator it;
		it=localModuleIDSet.find(moduleID);

		if( it!=localModuleIDSet.end() )
		{
		    cout<<" in set";
		}
		else
		    cout<<" NOT in set";
		cout<<"\n";
	    }
	    cout<<"\n";

	    for(unsigned int k=0; k<m_moduleIDs.size(); k++)
	    {
		for(unsigned int j=0; j<m_pDB->m_modules[m_moduleIDs[k]].m_netsId.size(); j++)
		{
		    if(m_pDB->m_modules[m_moduleIDs[k]].m_netsId[j]==m_regionNets[i].m_netID)
			cout<<" FIND IN NET mid:"<<m_moduleIDs[k]<<" ";
		}
	    }

	    bool find=false;
	    for(unsigned int k=0; k<m_moduleIDs.size(); k++)
	    {

		//if((m_pDB->m_modules[m_moduleIDs[i]].m_cx<m_Rgn.left))
		//	m_pDB->

		for(unsigned int j=0; j<m_pDB->m_modules[m_moduleIDs[k]].m_netsId.size(); j++)
		{
		    if(m_regionNets[i].m_netID==m_pDB->m_modules[m_moduleIDs[k]].m_netsId[j])
		    {
			find=true;
			cout<<"\nFind!!";

			cout << " failRid:" << m_regionNets[i].m_netID 
			    << " rightRid:"<< m_regionNets[i].m_netID
			    << " module:"  << m_moduleIDs[k]
			    << " mname"    << m_pDB->m_modules[m_moduleIDs[k]].m_name;

			set<int>::iterator it;
			it=localModuleIDSet.find(m_moduleIDs[k]);

			if( it!=localModuleIDSet.end() )
			{
			    cout<<" in set";
			}
			break;
		    }
		}

	    }

	    cout<<"\n rss:"<<m_regionNets.size();
	    cout<<"\n i:"<<i<<" rs:"<<m_regionNets[i].m_localModules.size()<<"\n";
	}
	assert(m_regionNets[i].m_localModules.size()>0);
    }

}

double CPlaceDBQPRegion::getMaxSize()
{
    double width=m_Rgn.right-m_Rgn.left;
    double height=m_Rgn.top-m_Rgn.bottom;
    assert(width*height>=0);

    if(width>height)
	return width;
    else
	return height;
}

void CPlaceDBQPRegion::safeMoveCenter(int i,double cx, double cy)
{
    //int i=moduleID;
    if( m_pDB->m_modules[i].m_isFixed == true )
	return;

    double x = cx - m_pDB->m_modules[i].m_width*(double)0.5;
    double y = cy - m_pDB->m_modules[i].m_height*(double)0.5;


    if( (x + m_pDB->m_modules[i].m_width) > m_Rgn.right)
    {
	x = m_Rgn.right - m_pDB->m_modules[i].m_width;
    }
    else if(x < m_Rgn.left)
    {
	x = m_Rgn.left;
    }
    if( (y + m_pDB->m_modules[i].m_height)>m_Rgn.top)
    {
	y = m_Rgn.top-m_pDB->m_modules[i].m_height;
    }
    else if(y<m_Rgn.bottom)
    {
	y = m_Rgn.bottom;
    }

    m_pDB->SetModuleLocation( i, x, y );

}

CPlaceDBQPPlacer::CPlaceDBQPPlacer(CPlaceDB& db)
{
	m_pDB=&db;
}

CPlaceDBQPPlacer::~CPlaceDBQPPlacer(void)
{
}

void CPlaceDBQPPlacer::safeMoveCenter( int i, double cx, double cy )
{
//	int i=moduleID;
	if(m_pDB->m_modules[i].m_isFixed==true)
		return;

	double x = cx - m_pDB->m_modules[i].m_width * (double)0.5;
	double y = cy - m_pDB->m_modules[i].m_height * (double)0.5;


	if( ( x + m_pDB->m_modules[i].m_width ) > m_pDB->m_coreRgn.right )
	{
		x = m_pDB->m_coreRgn.right - m_pDB->m_modules[i].m_width;
	}
	else if( x < m_pDB->m_coreRgn.left )
	{
		x = m_pDB->m_coreRgn.left;
	}
	if( ( y + m_pDB->m_modules[i].m_height ) > m_pDB->m_coreRgn.top )
	{
		y = m_pDB->m_coreRgn.top - m_pDB->m_modules[i].m_height;
	}
	else if( y < m_pDB->m_coreRgn.bottom )
	{
		y=m_pDB->m_coreRgn.bottom;
	}

	m_pDB->SetModuleLocation( i,  x, y );

}
void CPlaceDBQPPlacer::findFloatingNet()
{
	//cout<<"\nStart check floating net!!";
	m_isFloatingNets.clear();
	m_isFloatingNets.resize( m_pDB->m_nets.size() ,true);

	for(unsigned int i=0; i<m_pDB->m_nets.size(); i++)
	{
		for(unsigned int j=0; j<m_pDB->m_nets[i].size(); j++)
		{
			int pinID=m_pDB->m_nets[i][j];
			int modID=m_pDB->m_pins[pinID].moduleId;
			if(m_pDB->m_modules[modID].m_isFixed==true)
			{
				m_isFloatingNets[i]=false;
				break;
			}
		}
	}
	int fcount=0;
	int fmcount=0;
	isModule_float.resize(m_pDB->m_modules.size(),true);
	bool end=false;
	while(end==false)
	{
	    bool change=false;
	    for(unsigned int i=0; i<m_pDB->m_modules.size(); i++)
	    {
		if(isModule_float[i]==true)
		{
		    if(m_pDB->m_modules[i].m_isFixed==false)
		    {
			bool isFix=false;
			for(unsigned int j=0; j<m_pDB->m_modules[i].m_netsId.size(); j++)
			{
			    if(m_isFloatingNets[m_pDB->m_modules[i].m_netsId[j]]==false)
			    {
				isFix=true;
				break;
			    }
			}
			if(isFix==true)
			{
			    //cout<<" fix:"<<i<<" ";
			    for(unsigned int j=0; j<m_pDB->m_modules[i].m_netsId.size(); j++)
			    {
				m_isFloatingNets[m_pDB->m_modules[i].m_netsId[j]]=false;

			    }
			    isModule_float[i]=false;
			    change=true;
			}
		    }
		    else
		    {
			isModule_float[i]=false;
		    }
		}

	    }

	    if(change==false)
	    {
		end=true;
	    }
	}

	fcount=0;
	for(unsigned int i=0; i<this->m_isFloatingNets.size(); i++)
	{
		if(m_isFloatingNets[i]==true)
			++fcount;
	}
	for(unsigned int i=0; i<isModule_float.size(); i++)
	{
		if(isModule_float[i]==true)
			++fmcount;
	}

	if( fcount > 0 || fmcount > 0 )
	    cout<<"\nEnd of checking floating net.  Floating net # = "<< fcount << "  Floating module # = "<<fmcount<<"\n";

}



void CPlaceDBQPPlacer::QPplace()
{
    double top    = m_pDB->m_coreRgn.top;
    double left   = m_pDB->m_coreRgn.left;
    double bottom = m_pDB->m_coreRgn.bottom;
    double right  = m_pDB->m_coreRgn.right;
    
    //init something
    //isModuleTempFix.resize(m_pDB->m_modules.size(),false);
    //for(unsigned int i=0; i<m_pDB->m_modules.size(); i++)
    //{
    //	if(m_pDB->m_modules[i].m_isFixed==true)
    //	{
    //		isModuleTempFix[i]=true;
    //	}
    //}

    //------------------------------------------------
    // Find floating net
    //------------------------------------------------
    //	cout<<"\nStart Finding floating net:";
    findFloatingNet();
    //	cout<<"\nFinish finding floating net";
    
    //------------------------------------------------
    // Initialize the top level region
    //------------------------------------------------
    //	level_now.m_levelID=0;
    //	level_now.m_regions.resize(1);
    
    CPlaceDBQPRegion topRegion( *m_pDB, *this );

    //	level_now.m_regions[0]=topRegion;
    topRegion.m_moduleIDs.reserve( m_pDB->m_modules.size() );
    
    //insert all movable modules into the top region
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
    	if( m_pDB->m_modules[i].m_isFixed == false )
    	{
    	    topRegion.m_moduleIDs.push_back( i );
    	}
    	else
    	{
    	    topRegion.m_fixModuleIDs.push_back( i );
    	}
    }
    topRegion.setRgn( top, bottom, left, right );

    // (donnie) 2006-02-04 remove msg
    //m_pDB->CalcHPWL();
    //double wl=m_pDB->GetHPWLp2p();
    //cout<<"\nInitial WL:"<<wl;
    
    QPplaceCore( topRegion );
    //m_pDB->CalcHPWL();
}

// (donnie) 2006-02-04
void CPlaceDBQPPlacer::QPplace( const double& left, const double& bottom, const double& right, const double& top, 
	                        const vector<int>& movModules, const vector<int>& fixModules )
{
    findFloatingNet();

    CPlaceDBQPRegion topRegion( *m_pDB, *this );

    topRegion.m_moduleIDs = movModules;
    topRegion.m_fixModuleIDs = fixModules;
    topRegion.setRgn( top, bottom, left, right );

    QPplaceCore( topRegion );
}

void CPlaceDBQPPlacer::QPplaceCore( CPlaceDBQPRegion& topRegion )
{
    topRegion.initRegionNet();
    //cout << "\nBuild matrix...";
    topRegion.buildMatrix();
    //cout << " Done!!\nStart QP...";
    topRegion.pureQP();
    //cout << " Done!!";
}

void CPlaceDBQPRegion::buildMatrix()
{

    //1.establish move points (add star-node)
    //2.fill into matrix
    solverX.mapA.clear();
    solverY.mapA.clear();
    vec_Bx.clear();
    vec_By.clear();
    vec_mvY.clear();
    vec_mvX.clear();	
    vec_Kx.clear();
    vec_Ky.clear();
    vec_SPx.clear();
    vec_SPy.clear();
    vec_Kx.resize(m_moduleIDs.size(),0);
    vec_Ky.resize(m_moduleIDs.size(),0);
    vec_SPx.resize(m_moduleIDs.size(),0);
    vec_SPy.resize(m_moduleIDs.size(),0);

    if(m_moduleIDs.size()==0) //if there is no movable module inside
	return;

    double fc = 0.01;
    double globalWeight = 1;
    vector<int> rgnNetDegree;
    vector<int> rgnNet2StarPos;

    //----------------------------------------------------------------------
    // Find the # of multi-terminal net
    //----------------------------------------------------------------------

    rgnNetDegree.resize(m_regionNets.size());
    rgnNet2StarPos.resize(m_regionNets.size());
    int multinet_count=0;
    for(unsigned int i=0; i<m_regionNets.size(); i++)
    {
	rgnNetDegree[i]=m_regionNets[i].m_localModules.size() + 
	    m_regionNets[i].m_nLeftModules  + 
	    m_regionNets[i].m_nRightModules +
	    m_regionNets[i].m_fixedPinX.size();
	assert(m_regionNets[i].m_localModules.size()>0);
	if( rgnNetDegree[i]>2)
	{
	    rgnNet2StarPos[i] = multinet_count;
	    ++multinet_count;
	}
	else
	{
	    rgnNet2StarPos[i] = -1;
	}
    }


    //----------------------------------------------------------------------
    //build matrix A and vector Bx, By and initial X,Y vector
    //----------------------------------------------------------------------

    //initial X
    int size_of_matrix = m_moduleIDs.size() + multinet_count;

    double region_centerX = (m_Rgn.left + m_Rgn.right) / 2;
    vec_mvX.resize( size_of_matrix, region_centerX );
    vec_Bx.resize( size_of_matrix, 0 );

    for(int i=0; i<size_of_matrix; i++)
    {
	solverX.setValue( i, i, 0 );
    }

    //for all regionNets, fill data into matrix A and vector B

    for(unsigned int i=0; i<m_regionNets.size(); i++)
    {
	if( rgnNetDegree[i]>2) // 3--k terminal net
	{

	    assert(rgnNet2StarPos[i]!=-1);

	    double netWeight=1;
	    double netWeightX=netWeight;

	    //calc net weight
	    if( m_regionNets[i].m_isGlobalX == true )
	    {
		netWeightX = globalWeight * netWeight;
	    }

	    netWeightX *= ( (double)rgnNetDegree[i] / ((double)rgnNetDegree[i] - 1) );


	    //add value to matrix A (w.r.t star node)
	    int starPos = m_moduleIDs.size() + rgnNet2StarPos[i];
	    assert( starPos < size_of_matrix );
	    solverX.setValue( starPos, starPos, netWeightX * rgnNetDegree[i] );

	    double fixSumX=0;

	    for( unsigned int j=0; j<m_regionNets[i].m_fixedPinX.size(); j++ )
	    {
		fixSumX += m_regionNets[i].m_fixedPinX[j];
	    }


	    //cout<<"\n FX:"<<fixSumX<<" FY:"<<fixSumY;
	    double bx_vec_sum = m_Rgn.left * m_regionNets[i].m_nLeftModules +
		                m_Rgn.right * m_regionNets[i].m_nRightModules + fixSumX;


	    //calc value of vector B (the information of fixed pin)
	    vec_Bx[starPos] = netWeightX * bx_vec_sum;

	    for( unsigned int j=0; j<m_regionNets[i].m_localModules.size(); j++ )
	    {
		//module position in the vector X,Y and matrix A
		int modulePos = realModuleID2regionModuleID[ m_regionNets[i].m_localModules[j] ];

		//x
		solverX.increment( modulePos, modulePos, netWeightX );
		solverX.decrement( modulePos, starPos,netWeightX );
		solverX.decrement( starPos, modulePos,netWeightX );

		vec_Bx[starPos]   += netWeightX * (m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[j]].xOff);
		vec_Bx[modulePos] -= netWeightX * (m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[j]].xOff);
	    }

	}
	else if(rgnNetDegree[i]>1) // 2-terminal net, may be movable-movable or fix-movable
	{
	    //calc net weight
	    double netWeight=1;
	    double netWeightX=netWeight;
	    if(m_regionNets[i].m_isGlobalX==true)
	    {
		netWeightX = globalWeight * netWeight;
	    }
	    //movable-movable
	    if( m_regionNets[i].m_localModules.size() == 2 ) 
	    {
		int pos0=realModuleID2regionModuleID[m_regionNets[i].m_localModules[0]];
		int pos1=realModuleID2regionModuleID[m_regionNets[i].m_localModules[1]];

		vec_Bx[pos0] += netWeightX * 
		                ( m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[1]].xOff - 
				  m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[0]].xOff );
		vec_Bx[pos1] += netWeightX * 
		                ( m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[0]].xOff - 
				  m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[1]].xOff );

		solverX.increment( pos0, pos0, netWeightX );
		solverX.increment( pos1, pos1, netWeightX );
		solverX.decrement( pos0, pos1, netWeightX );
		solverX.decrement( pos1, pos0, netWeightX );
	    }
	    else //fixed Pin - movable module
	    {
		int pos0 = realModuleID2regionModuleID[m_regionNets[i].m_localModules[0]];
		double fixSumX=0;
		for(unsigned int j=0; j< m_regionNets[i].m_fixedPinX.size(); j++)
		{
		    fixSumX+=m_regionNets[i].m_fixedPinX[j];
		}

		//assertion: exactly 1 fixed pin
		assert( (m_regionNets[i].m_nLeftModules+m_regionNets[i].m_nRightModules +
			 m_regionNets[i].m_fixedPinX.size() ) == 1 );

		double bx_vec_sum = m_Rgn.left * m_regionNets[i].m_nLeftModules +
		                    m_Rgn.right * m_regionNets[i].m_nRightModules + fixSumX;

		vec_Bx[pos0] += netWeightX * 
		                ( bx_vec_sum - m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[0]].xOff);
		solverX.increment( pos0, pos0, netWeightX );
	    }

	}
	else
	{
	    //1 terminal net !? Don't care.
	}
    }

    //for all floating module, add a pseudo fix pin to region center
    for( unsigned int i=0; i<vec_Bx.size(); i++ )
    {
	//assertion: starNode shouldn't be floating
	double val=0;
	solverX.getValue(i,i,val);
	if(val==0)
	{
	    solverX.setValue(i,i,fc);
	    vec_Bx[i]=fc*region_centerX;
	}
	else if(i<m_moduleIDs.size())
	{
	    if(m_pPqap->isModule_float[m_moduleIDs[i]]==true) //floating module
	    {
		solverX.increment(i,i,fc);
		vec_Bx[i]+=fc*region_centerX;
	    }
	}
    }


    //	checkMatrix(solverX, vec_Bx);



    //------------------------------------------
    // QP on y
    //------------------------------------------


    //----------------------------------------------------------------------
    // Find the # of multi-terminal net
    //----------------------------------------------------------------------
    rgnNetDegree.clear();
    rgnNet2StarPos.clear();
    rgnNetDegree.resize(m_regionNets.size());
    rgnNet2StarPos.resize(m_regionNets.size());
    multinet_count=0;
    for( unsigned int i=0; i<m_regionNets.size(); i++ )
    {

	rgnNetDegree[i] = m_regionNets[i].m_localModules.size() + 
	                  m_regionNets[i].m_nBottomModules  + 
	                  m_regionNets[i].m_nTopModules +
	                  m_regionNets[i].m_fixedPinY.size()
	                  ;
	assert( m_regionNets[i].m_localModules.size() > 0 );
	if( rgnNetDegree[i] > 2 )
	{
	    rgnNet2StarPos[i] = multinet_count;
	    ++multinet_count;
	}
	else
	{
	    rgnNet2StarPos[i] = -1;
	}
    }

    //----------------------------------------------------------------------
    //build matrix A and vector Bx, By and initial X,Y vector
    //----------------------------------------------------------------------

    //initial Y
    size_of_matrix = m_moduleIDs.size() + multinet_count;
    double region_centerY = (m_Rgn.top+m_Rgn.bottom)/2;
    vec_mvY.resize(size_of_matrix,region_centerY);
    vec_By.resize(size_of_matrix,0);
    for( int i=0; i<size_of_matrix; i++ )
    {
	solverY.setValue(i,i,0);
    }

    //for all regionNets, fill data into matrix A and vector B

    for(unsigned int i=0; i<m_regionNets.size(); i++)
    {
	if( rgnNetDegree[i]>2) //3-k terminal net
	{

	    assert(rgnNet2StarPos[i]!=-1);

	    double netWeight=1;
	    double netWeightY=netWeight;

	    //calc net weight
	    if(m_regionNets[i].m_isGlobalY==true)
	    {
		netWeightY=globalWeight*netWeight;
	    }
	    netWeightY*=( (double)rgnNetDegree[i]/((double)rgnNetDegree[i]-1) );

	    //add value to matrix A (w.r.t star node)
	    int starPos=m_moduleIDs.size()+rgnNet2StarPos[i];
	    assert(starPos<size_of_matrix);
	    solverY.setValue(starPos,starPos,netWeightY*rgnNetDegree[i]);			

	    double fixSumY=0;
	    for(unsigned int j=0; j< m_regionNets[i].m_fixedPinY.size(); j++)
	    {
		fixSumY+=m_regionNets[i].m_fixedPinY[j];
	    }

	    double by_vec_sum=m_Rgn.bottom*m_regionNets[i].m_nBottomModules+m_Rgn.top*m_regionNets[i].m_nTopModules+fixSumY;

	    //calc value of vector B (the information of fixed pin)
	    vec_By[starPos]=netWeightY *by_vec_sum;
	    //			cout<<"\n FbX:"<<vec_Bx[starPos]<<" FbY:"<<vec_By[starPos]<<" starPos:"<<starPos;


	    for(unsigned int j=0; j< m_regionNets[i].m_localModules.size(); j++)
	    {
		//module position in the vector X,Y and matrix A
		int modulePos=realModuleID2regionModuleID[m_regionNets[i].m_localModules[j]];

		//y
		solverY.increment(modulePos,modulePos,netWeightY);
		solverY.decrement(modulePos,starPos,netWeightY);
		solverY.decrement(starPos,modulePos,netWeightY);
		vec_By[starPos]+=netWeightY * (m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[j]].yOff);
		vec_By[modulePos]-=netWeightY * (m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[j]].yOff);
	    }

	}
	else if(rgnNetDegree[i]>1) // 2-terminal net, may be movable-movable or fix-movable
	{
	    //calc net weight
	    double netWeight=1;
	    double netWeightY=netWeight;
	    if(m_regionNets[i].m_isGlobalY==true)
	    {
		netWeightY=globalWeight*netWeight;
	    }

	    //movable-movable
	    if(m_regionNets[i].m_localModules.size()==2) 
	    {
		int pos0=realModuleID2regionModuleID[m_regionNets[i].m_localModules[0]];
		int pos1=realModuleID2regionModuleID[m_regionNets[i].m_localModules[1]];

		vec_By[pos0]+=netWeightY * (m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[1]].yOff - m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[0]].yOff);
		vec_By[pos1]+=netWeightY * (m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[0]].yOff - m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[1]].yOff);

		solverY.increment(pos0,pos0,netWeightY);
		solverY.increment(pos1,pos1,netWeightY);
		solverY.decrement(pos0,pos1,netWeightY);
		solverY.decrement(pos1,pos0,netWeightY);
	    }
	    else //fixed Pin - movable module
	    {
		int pos0=realModuleID2regionModuleID[m_regionNets[i].m_localModules[0]];
		double fixSumY=0;
		for(unsigned int j=0; j< m_regionNets[i].m_fixedPinY.size(); j++)
		{
		    fixSumY+=m_regionNets[i].m_fixedPinY[j];
		}
		//assertion: exactly 1 fixed pin
		assert( (m_regionNets[i].m_nBottomModules+m_regionNets[i].m_nTopModules+m_regionNets[i].m_fixedPinY.size())==1 );
		double by_vec_sum=m_Rgn.bottom*m_regionNets[i].m_nBottomModules+m_Rgn.top*m_regionNets[i].m_nTopModules+fixSumY;
		vec_By[pos0]+=netWeightY * ( by_vec_sum - m_pDB->m_pins[m_regionNets[i].m_localModulesPinID[0]].yOff);
		solverY.increment(pos0,pos0,netWeightY);
	    }
	}
	else
	{
	    //1 terminal net !? Don't care.
	}
    }

    //for all floating module, add a pseudo fix pin to region center
    for(unsigned int i=0; i<vec_By.size(); i++)
    {
	//assertion: starNode shouldn't be floating

	double val=0;
	solverY.getValue(i,i,val);
	if(val==0)
	{
	    solverY.setValue(i,i,fc);
	    vec_By[i]=fc*region_centerY;
	}
	else if(i<m_moduleIDs.size())
	{
	    if(m_pPqap->isModule_float[m_moduleIDs[i]]==true) //floating module
	    {
		solverY.increment(i,i,fc);
		vec_By[i]+=fc*region_centerY;
	    }
	}
    }


    /* (???)
    //solve the AX=B
    solverY.solverQP(10000,vec_By,vec_mvY);

    //move modules in this region
    for(unsigned int i=0; i<m_moduleIDs.size(); i++)
    {
	//float cx=m_pDB->m_modules[m_moduleIDs[i]].m_x;
	//float cy=vec_mvY[i]-m_pDB->m_modules[m_moduleIDs[i]].m_height*(float)0.5;
	//	cout<<" Module:"<<m_moduleIDs[i]<<" vx:"<<vec_mvX[i]<<" vy:"<<vec_mvY[i]<<"  ";
	assert(m_pDB->m_modules[m_moduleIDs[i]].m_isFixed==false);
	//m_pDB->SetModuleLocation(m_moduleIDs[i],cx,cy);
	safeMoveCenter(m_moduleIDs[i],m_pDB->m_modules[m_moduleIDs[i]].m_cx,vec_mvY[i]);
    }
    */


}

void CPlaceDBQPRegion::pureQP()
{
    //solve the AX=B
    solverX.solverQP(10000,vec_Bx,vec_mvX);
    //solve the AX=B
    solverY.solverQP(10000,vec_By,vec_mvY);

    for(unsigned int i=0; i<m_moduleIDs.size(); i++)
    {
	//assert(m_pDB->m_modules[m_moduleIDs[i]].m_isFixed==false);
	safeMoveCenter(m_moduleIDs[i],vec_mvX[i],vec_mvY[i]);
	//assert(m_pDB->m_modules[m_moduleIDs[i]].m_cy>m_Rgn.bottom);
    }
}

//use QP to calc new module location according to the diffusion position


//-------------------------------------------------
// Log: 2005.11.28
// Finish the diffusion-QP code, but still very, very buggy :(
//

//-----------------------------------------------------------
//-----------------------------------------------------------
// KPlacer
//-----------------------------------------------------------
//-----------------------------------------------------------
KPlacer::KPlacer( CPlaceDB& db, const int& argc, char* argv[])
{
	m_pDB=&db;
    m_nMovable=0;
    this->m_db2matrix.reserve(m_pDB->m_modules.size());
    this->m_matrix2db.reserve(m_pDB->m_modules.size());
    for(int i=0; i<(int)m_pDB->m_modules.size(); ++i)
    {
        if(m_pDB->m_modules[i].m_isFixed==false)
        {
            m_db2matrix.push_back(m_matrix2db.size());
            m_matrix2db.push_back(i);
            ++m_nMovable;
        }
        else
        {
            m_db2matrix.push_back(-1);
        }
    }
    this->m_kratio=1;
    this->m_avgPotential=100;
    handleArgument( argc, argv);
}
void KPlacer::handleArgument( const int& argc, char* argv[])
{
    int i;
    if( strcmp( argv[1]+1, "aux" ) == 0 )
	i = 3;
    else
	i = 4;
    while( i < argc )
    {
	    // 2006-03-19 (donnie) remove checking "-"
	    if( strlen( argv[i] ) <= 1 )	// bad argument?
	    {
	        i++;
	        continue;
	    }

	    if( strcmp( argv[i]+1, "kratio" ) == 0 )
            m_kratio = atof( argv[++i] );
	    else if( strcmp( argv[i]+1, "avp" ) == 0 )
	        m_avgPotential = atof( argv[++i] );

        i++;
    }
    cout<< "\n Kratio:" << m_kratio << "\n Average Potential:" << m_avgPotential << "\n";
}
void KPlacer::place()
{

    //----------------------------------------------------------------
    // Init QP
    //----------------------------------------------------------------
    initQP();
    m_pDB->OutputGnuplotFigure( "KQP-0.plt", false );


    //---------------------------------------------------------------
    // Move QP
    //----------------------------------------------------------------

    double kratio=m_kratio;
    vector<double> deltaX,deltaY;
    vector<double> fxs,fys;
    deltaX.resize(m_nMovable,0);
    deltaY.resize(m_nMovable,0);

    fxs.resize(m_nMovable,0);
    fys.resize(m_nMovable,0);
    
    CQPForce qpf( *m_pDB, 100 );
    vector< double > grad_potential;

    for(int i=0; i<15; i++)
    {
        deltaX.resize(m_nMovable,0);
        deltaY.resize(m_nMovable,0);

     //   //------------------------------------
     //   //diffusion force
     //   //------------------------------------
     //   vector<int> movableModule,fixModule;
	    //movableModule.reserve(m_pDB->m_modules.size());
	    //fixModule.reserve(m_pDB->m_modules.size());
	    //for(int i=0; i<(int)m_pDB->m_modules.size(); ++i)
	    //{
		   // if(m_pDB->m_modules[i].m_isFixed==true)
		   // {
			  //  fixModule.push_back(i);
		   // }
		   // else
		   // {
			  //  movableModule.push_back(i);
		   // }
	    //}

     //   vector<double> tempFx,tempFy;
	    //double m_w=m_pDB->m_coreRgn.right-m_pDB->m_coreRgn.left;
	    //double m_h=m_pDB->m_coreRgn.top-m_pDB->m_coreRgn.bottom;
	    //CDiffusion diff( *m_pDB, m_w/30, m_h/30, 1.0, m_pDB->m_coreRgn);
	    //diff.init(movableModule,fixModule);
     //   diff.runDiffusionSpreadingGetForce(tempFx,tempFy);

        qpf.GetForce( grad_potential,m_avgPotential );

        for(int j=0; j<(int)this->m_db2matrix.size(); ++j)
        {
            if(m_db2matrix[j]!=-1)
            {
                fxs[m_db2matrix[j]]=grad_potential[2*j];
                fys[m_db2matrix[j]]=grad_potential[2*j+1];
            }
        }

        //random force
        for(int j=0; j<(int)fxs.size(); ++j)
        {
            //cout<<" fxs:"<<fxs[j]<<" fys:"<<fys[j];

            if(fxs[j]==0)
                fxs[j]=0.00001;
            if(fys[j]==0)
                fys[j]=0.00001;
        }

        moveQP( fxs, fys, kratio, deltaX, deltaY );
        moveModule( deltaX, deltaY );

        m_pDB->CalcHPWL();
		double wl=m_pDB->GetHPWLp2p();
		cout<<"\nQP WL:"<<wl<<"\n";
        char filename[100];
		sprintf( filename, "KQP-%d.plt", i+1 );
        m_pDB->OutputGnuplotFigure( filename, false );
    }

}
void KPlacer::initQP()
{
	int matrixSize=m_pDB->GetMovableBlockNumber();
	vector<int> db2matrix;
	vector<int> matrix2db;

	db2matrix.resize(m_pDB->m_modules.size(),-1);
	matrix2db.resize(matrixSize,-1);

	vector<double> vecBx;
	vector<double> vecBy;
	vector<double> vecX;
	vector<double> vecY;
	vecBx.resize(matrixSize,0);
	vecBy.resize(matrixSize,0);
	vecX.resize(matrixSize,0);
	vecY.resize(matrixSize,0);
    CQP matrixA;

	//////////////////////////////////////////
	//build movable module 2 matrix entry map
	/////////////////////////////////////////
	int addCount=0;
	for(unsigned int i=0; i<m_pDB->m_modules.size(); i++)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			assert(addCount<matrixSize);

			vecX[addCount]=m_pDB->m_modules[i].m_cx;
			vecY[addCount]=m_pDB->m_modules[i].m_cy;
			db2matrix[i]=addCount;
			matrix2db[addCount]=i;
			++addCount;
		}
	}

	//////////////////////////////////////////
	// For all net, fill data into matrix
	/////////////////////////////////////////
	double basicK=1;
	for(unsigned int i=0; i<m_pDB->m_nets.size(); i++)
	{
		int moveCount=0;
	    set<int> moveset;
	    for(unsigned int j=0; j<m_pDB->m_nets[i].size(); j++)
	    {
		    if(m_pDB->m_modules[m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId].m_isFixed==false)
		    {
			    moveset.insert(m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId);
		    }
	    }
	    moveCount=moveset.size();

		if(moveCount>0 && m_pDB->m_nets[i].size()>1) 
		{
			/////////////////////////////////////////////////////////////////////////////////////////////////
			// Find out movable module and fixed pin !!IMPORTANT!! "module" and "pin" different kind of IDs
			/////////////////////////////////////////////////////////////////////////////////////////////////
			set<int> moveModuleIDSet;
			set<int> fixPinIDSet;

			for(unsigned int j=0; j<m_pDB->m_nets[i].size(); j++)
			{
				int mID=m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId;
				if(m_pDB->m_modules[mID].m_isFixed==false)
				{
					moveModuleIDSet.insert(mID);
				}
				else
				{
					fixPinIDSet.insert(m_pDB->m_nets[i][j]);
				}
			}

			//////////////////////////////
			// 1 movable module
			//////////////////////////////
			if(moveModuleIDSet.size()==1)// 1 movable module
			{
				double fixK=basicK/(double)(fixPinIDSet.size()+moveModuleIDSet.size()-1);
				set<int>::iterator it;
				it=moveModuleIDSet.begin();
				int entry=db2matrix[*it];
				matrixA.increment(entry,entry,basicK);

				//add 2-pin-net to connect movable module and fixed pin			
				for(it=fixPinIDSet.begin(); it!=fixPinIDSet.end(); ++it)
				{
					vecBx[entry]+= fixK * (m_pDB->m_pins[*it].absX);
					vecBy[entry]+= fixK * (m_pDB->m_pins[*it].absY);
				}
			}
			//////////////////////////////
			// 2 or more movable module
			//////////////////////////////
			else
			{
				double moveK=basicK/(double)(fixPinIDSet.size()+moveModuleIDSet.size()-1);

				///////////////////////////////////////////////////
				//use circle model to connect all movable modules
				///////////////////////////////////////////////////
				vector<int> temp;
				int tc=0; //temp counter
				temp.resize(moveModuleIDSet.size());
				set<int>::iterator it;
				for(it=moveModuleIDSet.begin(); it!=moveModuleIDSet.end(); ++it)
				{
					temp[tc]=*it;
					++tc;
				}
				for(unsigned int j=0; j<temp.size()-1; j++)
				{
					for(unsigned int k=j+1; k<temp.size(); k++)
					{
	                    int entry1=db2matrix[temp[j]];
	                    int entry2=db2matrix[temp[k]];
	                    matrixA.increment(entry1,entry1,moveK);
	                    matrixA.increment(entry2,entry2,moveK);
	                    matrixA.decrement(entry1,entry2,moveK);
	                    matrixA.decrement(entry2,entry1,moveK);
					}
				}
				//if(temp.size()>2)
				//{
				//	add2Pnet(temp[temp.size()-1],temp[0],moveK);
				//}

				/////////////////////////////////////////////
				//add fix connection to each movable modules
				/////////////////////////////////////////////
				if(fixPinIDSet.size()>0)
				{
					double fixK=basicK/(double)(fixPinIDSet.size()+moveModuleIDSet.size()-1);
					for(it=fixPinIDSet.begin(); it!=fixPinIDSet.end(); ++it)
					{
						set<int>::iterator it2;
						for(it2=moveModuleIDSet.begin(); it2!=moveModuleIDSet.end(); ++it2)
						{
							int entry=db2matrix[*it2];
							matrixA.increment(entry,entry,fixK);
							vecBx[entry]+= fixK * (m_pDB->m_pins[*it].absX);
							vecBy[entry]+= fixK * (m_pDB->m_pins[*it].absY);
						}
					}
				}


			}

		}
	}

    //------------------------------------
    //solve the matrix
    //------------------------------------
    matrixA.solverQP(100000,vecBx,vecX);
	matrixA.solverQP(100000,vecBy,vecY);

    //-----------------------------------
    //move module in m_pDB
    //-----------------------------------
	for(unsigned int j=0; j<vecX.size(); j++)
	{
		int i=matrix2db[j];
		if(m_pDB->m_modules[i].m_isFixed==true)
			continue;
		double x=vecX[i]-m_pDB->m_modules[i].m_width*(double)0.5;
		double y=vecY[i]-m_pDB->m_modules[i].m_height*(double)0.5;
        safeMove(i,x,y);
	}
}
void KPlacer::moveQP( vector<double>& fxs, vector<double>& fys, double kratio , vector<double>& deltaX, vector<double>& deltaY )
{
    CQP2 matrixX( m_nMovable );
    CQP2 matrixY( m_nMovable );
    build_matrix( matrixX, matrixY );
    for( int i=0; i<m_nMovable; ++i )
    {
        matrixX.increment(i,i,kratio);
        matrixY.increment(i,i,kratio);
    }
    deltaX.resize(m_nMovable,0);
    deltaX.resize(m_nMovable,0);
	matrixX.solverQP(100000,fxs,deltaX);
	matrixY.solverQP(100000,fys,deltaY);

}

void KPlacer::calcFixedPin( const int& mid, const double& fx, const double& fy, const double& kScale, double& addPinX, double& addPinY, double& addK )
{
	double px=m_pDB->m_modules[mid].m_cx;
	double py=m_pDB->m_modules[mid].m_cy;

	double flength=sqrt(pow(fx,2)+pow(fy,2));
	//double fxUnit=fx/flength;
	//double fyUnit=fy/flength;

	double left=m_pDB->m_coreRgn.left;
	double right=m_pDB->m_coreRgn.right;
	double top=m_pDB->m_coreRgn.top;
	double bottom=m_pDB->m_coreRgn.bottom;

	if(fx>0)
	{
		double dx=(right-px)/fx;
		double dy=0;
		if(fy>0) //pseudo pin is on the top or right edge
		{
			dy=(top-py)/fy;
		}
		else if(fy==0)
		{
			double pl=(right-px);
			double realL=kScale*pl;
			addPinX=realL+px;
			addPinY=py;
			addK=fx/realL;
			return;
		}
		else //pseudo pin is on the bottom or right edge
		{
			dy=fabs((py-bottom)/fy);
		}
		if(dx<=dy) //pseudo pin is on the right edge
		{
			double pl=(right-px)*(flength/fx);
			double realL=fabs(kScale*pl);
			addPinX=realL*(fx/flength)+px;
			addPinY=realL*(fy/flength)+py;
			addK=flength/realL;
		}
		else if(fy>=0)//pseudo pin is on the top edge
		{
			double pl=(top-py)*(flength/fy);
			double realL=kScale*pl;
			addPinX=realL*(fx/flength)+px;
			addPinY=realL*(fy/flength)+py;
			addK=flength/realL;
		}
		else //pseudo pin is on the bottom edge
		{
			double pl=(py-bottom)*(flength/fy);
			double realL=fabs(kScale*pl);
			addPinX=realL*(fx/flength)+px;
			addPinY=realL*(fy/flength)+py;
			addK=flength/realL;
		}

	}
	else if(fx==0)
	{
		if(fy>0)
		{
			double pl=(top-py);
			double realL=kScale*pl;
			addPinX=px;
			addPinY=realL+py;
			addK=fy/realL;
		}
		else if(fy==0) //fx==0, fy==0
		{
			addPinX=0;
			addPinY=0;
			addK=0;
		}
		else
		{
			double pl=(py-bottom);
			double realL=kScale*pl;
			addPinX=px;
			addPinY=-1*realL+py;
			addK=fabs(fy/realL);
		}
	}
	else //fx<0
	{
		double dx=fabs((px-left)/fx);
		double dy=0;
		if(fy>0) //pseudo pin is on the top or left edge
		{
			dy=(top-py)/fy;
		}
		else if(fy==0)
		{
			double pl=(px-left);
			double realL=kScale*pl;
			addPinX=(-1*realL)+px;
			addPinY=py;
			addK=fabs(fx/realL);
			return;
		}
		else //pseudo pin is on the bottom or left edge
		{
			dy=fabs((py-bottom)/fy);
		}

		if(dx<=dy) //pseudo pin is on the left edge
		{
			double pl=(px-left)*(flength/fx);
			double realL=fabs(kScale*pl);
			addPinX=realL*(fx/flength)+px;
			addPinY=realL*(fy/flength)+py;
			addK=flength/realL;
		//	cout<<"\n ak"<<addK<<" fl:"<<flength<<" realL"<<realL<<" pinx:"<<addPinX<<" piny:"<<addPinY<<" py:"<<py<<" fy:"<<fy;
		}
		else if(fy>=0)//pseudo pin is on the top edge
		{
			double pl=(top-py)*(flength/fy);
			double realL=kScale*pl;
			addPinX=realL*(fx/flength)+px;
			addPinY=realL*(fy/flength)+py;
			addK=flength/realL;
		}
		else //pseudo pin is on the bottom edge
		{
			double pl=(py-bottom)*(flength/fy);
			double realL=fabs(kScale*pl);
			addPinX=realL*(fx/flength)+px;
			addPinY=realL*(fy/flength)+py;
			addK=flength/realL;
		}
	}
}
void KPlacer::add2Pnet(const int& moduleID1,const int& moduleID2, const double& k, CQP2& matrix)
{
	matrix.increment(m_db2matrix[moduleID1],m_db2matrix[moduleID1],k);
	matrix.increment(m_db2matrix[moduleID2],m_db2matrix[moduleID2],k);
	matrix.decrement(m_db2matrix[moduleID1],m_db2matrix[moduleID2],k);
	matrix.decrement(m_db2matrix[moduleID2],m_db2matrix[moduleID1],k);
}

void KPlacer::build_matrix( CQP2& matrixX, CQP2& matrixY )
{
    //for all nets
    for( int i=0; i<(int)m_pDB->m_nets.size(); ++i )
    {
        if(m_pDB->m_nets[i].size()<2)
            continue;

        double maxX=m_pDB->m_pins[m_pDB->m_nets[i][0]].absX;
        double minX=m_pDB->m_pins[m_pDB->m_nets[i][0]].absX;
        double maxY=m_pDB->m_pins[m_pDB->m_nets[i][0]].absY;
        double minY=m_pDB->m_pins[m_pDB->m_nets[i][0]].absY;

        int maxXid=m_pDB->m_pins[m_pDB->m_nets[i][0]].moduleId;
        int maxYid=maxXid;
        int minXid=maxYid;
        int minYid=minXid;

        int pinWeight=m_pDB->m_nets[i].size()-1;
        for(int j=1; j<(int)m_pDB->m_nets[i].size(); ++j)
        {
            if(m_pDB->m_pins[m_pDB->m_nets[i][j]].absX<minX)
            {
                minX=m_pDB->m_pins[m_pDB->m_nets[i][j]].absX;
                minXid=m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId;
            }
            else if(m_pDB->m_pins[m_pDB->m_nets[i][j]].absX>maxX)
            {
                maxX=m_pDB->m_pins[m_pDB->m_nets[i][j]].absX;
                maxXid=m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId;
            }

            if(m_pDB->m_pins[m_pDB->m_nets[i][j]].absY<minY)
            {
                minY=m_pDB->m_pins[m_pDB->m_nets[i][j]].absY;
                minYid=m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId;
            }
            else if(m_pDB->m_pins[m_pDB->m_nets[i][j]].absY>maxY)
            {
                maxY=m_pDB->m_pins[m_pDB->m_nets[i][j]].absY;
                maxYid=m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId;
            }            
        }

        //-------------------------------------------
        //build X
        //-------------------------------------------
        if(maxXid!=minXid)
        {
            double lx,k;
            lx=maxX-minX;
            if(lx<=0)
                lx=m_pDB->m_rowHeight/10000;
            k=1/(pinWeight*lx);
            if( m_pDB->m_modules[minXid].m_isFixed==false && m_pDB->m_modules[maxXid].m_isFixed==false )
            {
	            add2Pnet( maxXid, minXid, k, matrixX );
            }
            else if( m_pDB->m_modules[minXid].m_isFixed==false )
            {
                matrixX.increment( m_db2matrix[minXid], m_db2matrix[minXid], k );
            }
            else if( m_pDB->m_modules[maxXid].m_isFixed==false )
            {
                matrixX.increment( m_db2matrix[maxXid], m_db2matrix[maxXid], k );
            }

            for(int j=0; j<(int)m_pDB->m_nets[i].size(); ++j)
            {
                int mID=m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId;
                int entry=m_db2matrix[mID];
                if(mID!=maxXid && mID!=minXid && m_pDB->m_modules[mID].m_isFixed==false)
                {
                    //lx i
                    lx=m_pDB->m_pins[m_pDB->m_nets[i][j]].absX-minX;
                    if(lx<=0)
                        lx=m_pDB->m_rowHeight/10000;
                    k=1/(pinWeight*lx);
                    if(m_pDB->m_modules[minXid].m_isFixed==false)
                    {
	                    add2Pnet( mID, minXid, k, matrixX );
                    }
                    else
                    {
	                    matrixX.increment( entry, entry, k );
                    }

                    //lx i+1
                    lx=maxX-m_pDB->m_pins[m_pDB->m_nets[i][j]].absX;
                    if(lx<=0)
                        lx=m_pDB->m_rowHeight/10000;
                    k=1/(pinWeight*lx);
                    if(m_pDB->m_modules[maxXid].m_isFixed==false)
                    {
                        add2Pnet( mID, maxXid, k, matrixX );
                    }
                    else
                    {
	                    matrixX.increment( entry, entry, k );
                    }
                }
                else if(mID!=maxXid && mID!=minXid && m_pDB->m_modules[mID].m_isFixed==true)
                {
                    //lx i
                    lx=m_pDB->m_pins[m_pDB->m_nets[i][j]].absX-minX;
                    if(lx<=0)
                        lx=m_pDB->m_rowHeight/10000;
                    k=1/(pinWeight*lx);
                    if(m_pDB->m_modules[minXid].m_isFixed==false)
                    {
	                    matrixX.increment( m_db2matrix[minXid], m_db2matrix[minXid], k );
                    }

                    //lx i+1
                    lx=maxX-m_pDB->m_pins[m_pDB->m_nets[i][j]].absX;
                    if(lx<=0)
                        lx=m_pDB->m_rowHeight/10000;
                    k=1/(pinWeight*lx);
                    if(m_pDB->m_modules[maxXid].m_isFixed==false)
                    {
                        matrixX.increment( m_db2matrix[maxXid], m_db2matrix[maxXid], k );
                    }
                }
            }
        }


        //-------------------------------------------
        //build Y
        //-------------------------------------------
        if(maxYid!=minYid)
        {
            double lx,k;
            lx=maxY-minY;
            if(lx<=0)
                lx=m_pDB->m_rowHeight/10000;
            k=1/(pinWeight*lx);
            if( m_pDB->m_modules[minYid].m_isFixed==false && m_pDB->m_modules[maxYid].m_isFixed==false )
            {
	            add2Pnet( maxYid, minYid, k, matrixY );
            }
            else if( m_pDB->m_modules[minYid].m_isFixed==false )
            {
                matrixY.increment( m_db2matrix[minYid], m_db2matrix[minYid], k );
            }
            else if( m_pDB->m_modules[maxYid].m_isFixed==false )
            {
                matrixY.increment( m_db2matrix[maxYid], m_db2matrix[maxYid], k );
            }

            for(int j=0; j<(int)m_pDB->m_nets[i].size(); ++j)
            {
                int mID=m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId;
                int entry=m_db2matrix[mID];
                if(mID!=maxYid && mID!=minYid && m_pDB->m_modules[mID].m_isFixed==false)
                {
                    //lx i
                    lx=m_pDB->m_pins[m_pDB->m_nets[i][j]].absY-minY;
                    if(lx<=0)
                        lx=m_pDB->m_rowHeight/10000;
                    k=1/(pinWeight*lx);
                    if(m_pDB->m_modules[minYid].m_isFixed==false)
                    {
	                    add2Pnet( mID, minYid, k, matrixY );
                    }
                    else
                    {
	                    matrixY.increment( entry, entry, k );
                    }

                    //lx i+1
                    lx=maxY-m_pDB->m_pins[m_pDB->m_nets[i][j]].absY;
                    if(lx<=0)
                        lx=m_pDB->m_rowHeight/10000;
                    k=1/(pinWeight*lx);
                    if(m_pDB->m_modules[maxYid].m_isFixed==false)
                    {
                        add2Pnet( mID, maxYid, k, matrixY );
                    }
                    else
                    {
	                    matrixY.increment( entry, entry, k );
                    }
                }
                else if(mID!=maxYid && mID!=minYid && m_pDB->m_modules[mID].m_isFixed==true)
                {
                    //lx i
                    lx=m_pDB->m_pins[m_pDB->m_nets[i][j]].absY-minY;
                    if(lx<=0)
                        lx=m_pDB->m_rowHeight/10000;
                    k=1/(pinWeight*lx);
                    if(m_pDB->m_modules[minYid].m_isFixed==false)
                    {
	                    matrixY.increment( m_db2matrix[minYid], m_db2matrix[minYid], k );
                    }

                    //lx i+1
                    lx=maxY-m_pDB->m_pins[m_pDB->m_nets[i][j]].absY;
                    if(lx<=0)
                        lx=m_pDB->m_rowHeight/10000;
                    k=1/(pinWeight*lx);
                    if(m_pDB->m_modules[maxYid].m_isFixed==false)
                    {
                        matrixY.increment( m_db2matrix[maxYid], m_db2matrix[maxYid], k );
                    }
                }
            }
        }



    }

}



void KPlacer::safeMove( int& i, double& x, double& y)
{
    if( (x+m_pDB->m_modules[i].m_width)>m_pDB->m_coreRgn.right)
    {
	    x=m_pDB->m_coreRgn.right-m_pDB->m_modules[i].m_width;
    }
    else if(x<m_pDB->m_coreRgn.left)
    {
	    x=m_pDB->m_coreRgn.left;
    }
    if( (y+m_pDB->m_modules[i].m_height)>m_pDB->m_coreRgn.top)
    {
	    y=m_pDB->m_coreRgn.top-m_pDB->m_modules[i].m_height;
    }
    else if(y<m_pDB->m_coreRgn.bottom)
    {
	    y=m_pDB->m_coreRgn.bottom;
    }

    m_pDB->SetModuleLocation(i,x,y);
}
void KPlacer::moveModule(const vector<double>& deltaX, const vector<double>& deltaY)
{
    for( int j=0; j<(int)deltaX.size(); j++)
	{
		int i=m_matrix2db[j];
		if(m_pDB->m_modules[i].m_isFixed==true)
			continue;
		double x=m_pDB->m_modules[i].m_x+deltaX[j];
		double y=m_pDB->m_modules[i].m_y+deltaY[j];

		safeMove(i,x,y);
	}
}

//--------------------------------
//Diffusion
//--------------------------------
CDiffusionBin::CDiffusionBin(void)
{
	this->hasFixedModule=false;
	this->fixedArea=0;
	this->m_usedArea=0;
	dLeft=0;
	dRight=0;
	dTop=0;
	dBottom=0;
}

CDiffusionBin::~CDiffusionBin(void)
{
}


CDiffusion::CDiffusion( CPlaceDB& db , double bw,double bh ,double set_deltaT ,  const CRect& region)
{
	m_db=&db;
	m_binWidth=bw;
	m_binHeight=bh;
	deltaT=set_deltaT;
	maxDensity=1;
	m_Rgn=region;
	maxBinUtil=0;

}
void CDiffusion::init(const vector<int>& setModuleIDs, const vector<int>& setFixedmoduleIDs)
{

	moduleIDs=setModuleIDs;
	FixedmoduleIDs=setFixedmoduleIDs;
	m_binRows=(int)round( getHeight() / m_binHeight);
	m_binColumns=(int)round( getWidth() / m_binWidth);

//	cout<<"\n binRows:"<<m_binRows<<" binCols:"<<m_binColumns;
	m_bins.resize(m_binRows*m_binColumns);
	for(int i=0; i<m_binRows ; i++)
	{
		for(int j=0; j<m_binColumns; j++)
		{
			CDiffusionBin cb;
			cb.m_usedArea=0;
			cb.x=this->m_Rgn.left+j*m_binWidth;
			cb.y=this->m_Rgn.bottom+i*m_binHeight;
			cb.width=m_binWidth;
			cb.height=m_binHeight;
			cb.area=cb.width*cb.height;
			cb.nbi=cb.x+cb.width;
			cb.nbiy=cb.y+cb.height;

			//cb.V_x=0;
			//cb.V_y=0;
			if(i==0)
			{
				cb.bottomID=-1;
			}
			else
			{
				cb.bottomID=(i-1)*m_binColumns+j;
			}
			
			if(i==m_binRows-1)
			{
				cb.topID=-1;
			}
			else
			{
				cb.topID=(i+1)*m_binColumns+j;
			}

			if(j==0)
			{
				cb.leftID=-1;
			}
			else
			{
				cb.leftID=i*m_binColumns+j-1;
			}
			
			if(j==m_binColumns-1)
			{
				cb.rightID=-1;
			}
			else
			{
				cb.rightID=i*m_binColumns+j+1;
			}


			m_bins[i*m_binColumns+j]=cb;
		}
	}
	



	//insert fix modules into bins
	for(int i=0;i<(int)this->FixedmoduleIDs.size(); i++)
	{
		addFixBinModule(FixedmoduleIDs[i]);
	}

	//put all modules into Region (if somebody falls to outside) and add to bins
	double wsum=0;
	for(int i=0;i<(int)moduleIDs.size(); i++)
	{
		if(m_db->m_modules[moduleIDs[i]].m_isFixed==false)
		{
			wsum+=m_db->m_modules[moduleIDs[i]].m_width;
			safeMove(moduleIDs[i],m_db->m_modules[moduleIDs[i]].m_x,m_db->m_modules[moduleIDs[i]].m_y);
		}

	}
	this->moduleWHratio= (wsum/(double)moduleIDs.size())/m_db->m_rowHeight;
//	cout<<"\nWsum:"<<wsum<<" msize:"<<moduleIDs.size()<<" rowHeight:"<<m_db->m_rowHeight<<" ratio:"<<moduleWHratio;
	addmodule();
	updateDensity();
	alphaX=deltaT+(0.5/maxBinUtil)*(moduleWHratio);
	alphaY=deltaT+(0.5/maxBinUtil);
}
void CDiffusion::reset()
{
	for(int i=0; i<(int)m_bins.size() ; i++)
	{
		m_bins[i].m_usedArea=0;
		m_bins[i].density= (m_bins[i].fixedArea)/(m_bins[i].area);
	//	m_bins[i].moduleIDs.clear();

		m_bins[i].dBottom=0;
		m_bins[i].dTop=0;
		m_bins[i].dLeft=0;
		m_bins[i].dRight=0;

	}
	this->maxBinUtil=0;


}

void CDiffusion::addmodule()
{
	//calc movable module's overlapping area with bins
	for(int i=0;i<(int)moduleIDs.size(); i++)
	{
		addRemoveBinModule(moduleIDs[i],false);
	}

	////add module to each bin
	//for(int i=0;i<(int)moduleIDs.size(); i++)
	//{
	//	if(m_db->m_modules[moduleIDs[i]].m_isFixed==false)
	//	{
	//		int binID=getBinID(m_db->m_modules[moduleIDs[i]].m_cx,m_db->m_modules[moduleIDs[i]].m_cy);
	//		assert(binID!=-1);
	//		m_bins[i].moduleIDs.push_back(moduleIDs[i]);

	//	}
	//}
}
void CDiffusion::addmoduleF()
{
	//calc movable module's overlapping area with bins
	for(int i=0;i<(int)moduleIDs.size(); i++)
	{
		assert(moduleIDs[i]<(int)m_db->m_modules.size());
		double x=m_db->m_modules[moduleIDs[i]].m_cx;
		double y=m_db->m_modules[moduleIDs[i]].m_cy;
		int binID=getBinID(x,y);
		if(binID!=-1)
		{
			m_bins[binID].m_usedArea+=m_db->m_modules[moduleIDs[i]].m_area;
		}
		else
		{
			cout<<" x:"<<x<<" y:"<<y;
		}
//		addRemoveBinModule(moduleIDs[i],false);
	}
}
CDiffusion::~CDiffusion(void)
{
}

int CDiffusion::getBinID(const double& locx, const double& locy)
{
	int rowPos=(int)floor((locy-m_Rgn.bottom  ) /m_binHeight );
	if(locy<m_Rgn.bottom)
		return -1;
	if(locy>m_Rgn.top)
		return -1;
	if(locy==m_Rgn.top)
		rowPos=m_binRows-1;


	int colPos=(int)floor((locx-m_Rgn.left  )/m_binWidth);

	if(locx<m_Rgn.left)
		return -1;
	if(locx>m_Rgn.right)
		return -1;
	if(locx==m_Rgn.right)
		colPos=m_binColumns-1;

	int binID=rowPos*m_binColumns+colPos;
	assert(binID<(m_binRows*m_binColumns));
//	if(binID>=m_binRows*m_binColumns)
//		cout<<"\nFFFAAAIIILLLLL!!!";

	return binID;
}

void CDiffusion::addFixBinModule(int blockId)
{
//	int r=1;

	int i=blockId;



	//assert(binID<binRows*binColumns);
	float startX=m_db->m_modules[i].m_x;
	float startY=m_db->m_modules[i].m_y;
	float endX=m_db->m_modules[i].m_x+m_db->m_modules[i].m_width;
	float endY=m_db->m_modules[i].m_y+m_db->m_modules[i].m_height;
	if(endX<=this->m_Rgn.left || startX>=this->m_Rgn.right)
		return;

	if(endY<=this->m_Rgn.bottom || startY>=this->m_Rgn.top)
		return;

	if(startX<this->m_Rgn.left)
		startX=this->m_Rgn.left;
	if(endX>this->m_Rgn.right)
		endX=this->m_Rgn.right;
	if(startY<this->m_Rgn.bottom)
		startY=this->m_Rgn.bottom;
	if(endY>this->m_Rgn.top)
		endY=this->m_Rgn.top;

	int binID=getBinID(startX,startY);
	assert(binID!=-1);
	assert(startX<=endX && startY<=endY);

	if( (m_bins[binID].x+m_binWidth)>=endX && (m_bins[binID].y+m_binHeight)>=endY) //cell is entirely in this bin
	{
		//m_bins[binID].m_blocks.insert(i);
		double area=(endX-startX)*(endY-startY);
		m_bins[binID].fixedArea+=area;
		if(m_bins[binID].fixedArea>=m_bins[binID].area)
		{
			m_bins[binID].hasFixedModule=true;
			
		}
	}
	else
	{
		int binID2=getBinID(endX,endY);
		assert(binID2!=-1);

		int xSpan=binID2%m_binColumns-binID%m_binColumns;
		int ySpan=(int)(binID2/m_binColumns)-(int)(binID/m_binColumns);
		for(int j=0;j<=xSpan;j++)
		{
			for(int k=0;k<=ySpan; k++)
			{
				int bin=binID+j+m_binColumns*k;
				assert(bin<=binID2);
				//if(bin>m_binColumns*m_binRows-1)
				//	cout<<"\n Fail!! Bin:"<<bin<<" xS:"<<xSpan<<" yS:"<<ySpan<<" binID:"<<binID<<" j:"<<j<<" k:"<<k;
				double sy,ey,sx,ex;
				if(k==0)
				{
					sy=startY;
				}
				else
				{
					sy=m_bins[bin].y;
				}
				if(k==ySpan)
				{
					ey=endY;
				}
				else
				{
					ey=m_bins[bin].y+m_binHeight;
				}

				if(j==0)
				{
					sx=startX;
				}
				else
				{
					sx=m_bins[bin].x;
				}

				if(j==xSpan)
				{
					ex=endX;
				}
				else
				{
					ex=m_bins[bin].x+m_binWidth;
				}

				assert(sx>=m_bins[bin].x);
				assert(sy>=m_bins[bin].y);
				assert(ex<=m_bins[bin].x+m_binWidth+0.1);
				assert(ey<=m_bins[bin].y+m_binHeight+0.1);
				assert(sx<=ex && sy<=ey);
				double area=((ex-sx)*(ey-sy));
				m_bins[bin].fixedArea+=area;
				if(m_bins[bin].fixedArea>=m_bins[bin].area)
				{
					m_bins[bin].hasFixedModule=true;
					
				}				

			}
		}
	}
	
}



void CDiffusion::addRemoveBinModule(int blockId,bool remove)
{
	int r=1;
	if(remove==true)
		r=-1;

	int i=blockId;
	//assert(binID<binRows*binColumns);
	float startX=m_db->m_modules[i].m_x;
	float startY=m_db->m_modules[i].m_y;
	float endX=m_db->m_modules[i].m_x+m_db->m_modules[i].m_width;
	float endY=m_db->m_modules[i].m_y+m_db->m_modules[i].m_height;
	if(endX<=this->m_Rgn.left || startX>=this->m_Rgn.right)
		return;

	if(endY<=this->m_Rgn.bottom || startY>=this->m_Rgn.top)
		return;

	if(startX<this->m_Rgn.left)
		startX=this->m_Rgn.left;
	if(endX>this->m_Rgn.right)
		endX=this->m_Rgn.right;
	if(startY<this->m_Rgn.bottom)
		startY=this->m_Rgn.bottom;
	if(endY>this->m_Rgn.top)
		endY=this->m_Rgn.top;

	int binID=getBinID(startX,startY);
	assert(binID!=-1);
	assert(startX<=endX && startY<=endY);


	if( (m_bins[binID].x+m_binWidth)>=endX && (m_bins[binID].y+m_binHeight)>=endY) //cell is entirely in this bin
	{
		//m_bins[binID].m_blocks.insert(i);
		m_bins[binID].m_usedArea+=(r)*(endX-startX)*(endY-startY);

	}
	else
	{
		int binID2=getBinID(endX,endY);
		assert(binID2!=-1);

		int xSpan=binID2%m_binColumns-binID%m_binColumns;
		int ySpan=(int)(binID2/m_binColumns)-(int)(binID/m_binColumns);

		for(int j=0;j<=xSpan;j++)
		{
			for(int k=0;k<=ySpan; k++)
			{
				int bin=binID+j+m_binColumns*k;
				if(bin>binID2)
				{
					cout<<"\nStartX:"<<startX<<" endX:"<<endX<<" startY:"<<startY<<" endY:"<<endY<<" binX:"<<m_bins[binID].x<<" binY:"<<m_bins[binID].y<<" bw:"<<m_bins[binID].width<<" bh:"<<m_bins[binID].height;
					cout<<"\n binID:"<<binID<<" binID2:"<<binID2<<" xSpan:"<<xSpan<<" ySapn:"<<ySpan<<" j:"<<j<<" k:"<<k<<" col:"<<m_binColumns<<" bin:"<<bin<<"\n";
				}
				assert(bin<=binID2);
				//if(bin>m_binColumns*m_binRows-1)
				//	cout<<"\n Fail!! Bin:"<<bin<<" xS:"<<xSpan<<" yS:"<<ySpan<<" binID:"<<binID<<" j:"<<j<<" k:"<<k;
				double sy,ey,sx,ex;

				if(k==0)
				{
					sy=startY;
				}
				else
				{
					sy=m_bins[bin].y;
				}

				if(k==ySpan)
				{
					ey=endY;
				}
				else
				{
					ey=m_bins[bin].y+m_binHeight;
				}


				if(j==0)
				{
					sx=startX;
				}
				else
				{
					sx=m_bins[bin].x;
				}
				if(j==xSpan)
				{
					ex=endX;
				}
				else
				{
					ex=m_bins[bin].x+m_binWidth;
				}
				assert(sx>=m_bins[bin].x);
				assert(sy>=m_bins[bin].y);
				if(ex>m_bins[bin].x+m_binWidth+0.01)
				{
					cout<<"\nStartX:"<<startX<<" endX:"<<endX<<" startY:"<<startY<<" endY:"<<endY<<" binX:"<<m_bins[bin].x<<" binY:"<<m_bins[bin].y<<" bw:"<<m_bins[bin].width<<" bh:"<<m_bins[bin].height<<" mbh:"<<m_binHeight<<" threa:"<<m_bins[bin].x+m_binWidth;
					cout<<"\n binID:"<<binID<<" binID2:"<<binID2<<" xSpan:"<<xSpan<<" ySapn:"<<ySpan<<" j:"<<j<<" k:"<<k<<" col:"<<m_binColumns<<" bin:"<<bin<<" ex:"<<ex<<"\n";

				}
				assert(ex<=m_bins[bin].x+m_binWidth+0.01);
				if(ey>m_bins[bin].y+m_binHeight+0.01)
				{
					cout<<"\nStartX:"<<startX<<" endX:"<<endX<<" startY:"<<startY<<" endY:"<<endY<<" binX:"<<m_bins[bin].x<<" binY:"<<m_bins[bin].y<<" bw:"<<m_bins[bin].width<<" bh:"<<m_bins[bin].height<<" mbh:"<<m_binHeight;
					cout<<"\n binID:"<<binID<<" binID2:"<<binID2<<" xSpan:"<<xSpan<<" ySapn:"<<ySpan<<" j:"<<j<<" k:"<<k<<" col:"<<m_binColumns<<" bin:"<<bin<<" ey:"<<ey<<"\n";

				}
				assert(ey<=m_bins[bin].y+m_binHeight+0.01);
				assert(sx<=ex && sy<=ey);

				m_bins[bin].m_usedArea+=(r)*((ex-sx)*(ey-sy));
				

			}
		}
	}
	
}

void CDiffusion::updateDensity()
{
	this->maxBinUtil=0;
	for(int i=0; i<(int)m_bins.size() ; i++)
	{
		m_bins[i].density= (m_bins[i].m_usedArea+m_bins[i].fixedArea)/(m_bins[i].area);
		if(m_bins[i].density>maxBinUtil)
			maxBinUtil=m_bins[i].density;

	}
}
void CDiffusion::findForce(bool isX, vector<double>& force)
{

	for(int i=0;i<(int)moduleIDs.size(); i++)
	{
		int binID=getBinID(m_db->m_modules[moduleIDs[i]].m_cx,m_db->m_modules[moduleIDs[i]].m_cy);
		if(m_db->m_modules[moduleIDs[i]].m_isFixed==false && binID!=-1)
		{
			//update position according to the velosity
			//int binID=getBinID(m_db->m_modules[moduleIDs[i]].m_cx,m_db->m_modules[moduleIDs[i]].m_cy);
			//assert(binID!=-1);

			if(isX==true ) //horizontal expand
			{
				double newLeft=m_bins[binID].x;
				double newRight=m_bins[binID].x+m_bins[binID].width;
				if(m_bins[binID].leftID!=-1)
				{
					if(m_bins[binID].dLeft!=0)
					{
						newLeft-=m_bins[binID].dLeft;
					}
					else if(m_bins[m_bins[binID].leftID].dRight!=0)
					{
						newLeft+=m_bins[m_bins[binID].leftID].dRight;
					}
				}
				if(m_bins[binID].rightID!=-1)
				{
					if(m_bins[binID].dRight!=0)
					{
						newRight+=m_bins[binID].dRight;
					}
					else if(m_bins[m_bins[binID].rightID].dLeft!=0)
					{
						newRight-=m_bins[m_bins[binID].rightID].dLeft;
					}
				}

				const double ox=m_db->m_modules[moduleIDs[i]].m_x;
				double newx=(newRight*(ox-m_bins[binID].x)+newLeft*(m_bins[binID].x+m_bins[binID].width-ox))/(m_bins[binID].width);


				newx=alphaX*(newx-ox)+ox;
                force[moduleIDs[i]]=newx-ox;								


			}
			else //vertical expand
			{
				double newLeft=m_bins[binID].y;
				double newRight=m_bins[binID].y+m_bins[binID].height;
				if(m_bins[binID].bottomID!=-1)
				{
					if(m_bins[binID].dBottom!=0)
					{
						newLeft-=m_bins[binID].dBottom;
					}
					else if(m_bins[m_bins[binID].bottomID].dTop!=0)
					{
						newLeft+=m_bins[m_bins[binID].bottomID].dTop;
					}
				}
				if(m_bins[binID].topID!=-1)
				{
					if(m_bins[binID].dTop!=0)
					{
						newRight+=m_bins[binID].dTop;
					}
					else if(m_bins[m_bins[binID].topID].dBottom!=0)
					{
						newRight-=m_bins[m_bins[binID].topID].dBottom;
					}
				}

				const double oy=m_db->m_modules[moduleIDs[i]].m_y;
				double newy=(newRight*(oy-m_bins[binID].y)+newLeft*(m_bins[binID].y+m_bins[binID].height-oy))/(m_bins[binID].height);
				newy=alphaY*(newy-oy)+oy;
                force[moduleIDs[i]]=newy-oy;	

			}
		}
	}
}
//calculate all moudle's new location according to the bin velosity, then update bin utilized area 
void CDiffusion::moveAllModule(bool isX)
{

	for(int i=0;i<(int)moduleIDs.size(); i++)
	{
		int binID=getBinID(m_db->m_modules[moduleIDs[i]].m_cx,m_db->m_modules[moduleIDs[i]].m_cy);
		if(m_db->m_modules[moduleIDs[i]].m_isFixed==false && binID!=-1)
		{
			//update position according to the velosity
			//int binID=getBinID(m_db->m_modules[moduleIDs[i]].m_cx,m_db->m_modules[moduleIDs[i]].m_cy);
			//assert(binID!=-1);

			if(isX==true ) //horizontal expand
			{
				double newLeft=m_bins[binID].x;
				double newRight=m_bins[binID].x+m_bins[binID].width;
				if(m_bins[binID].leftID!=-1)
				{
					if(m_bins[binID].dLeft!=0)
					{
						newLeft-=m_bins[binID].dLeft;
					}
					else if(m_bins[m_bins[binID].leftID].dRight!=0)
					{
						newLeft+=m_bins[m_bins[binID].leftID].dRight;
					}
				}
				if(m_bins[binID].rightID!=-1)
				{
					if(m_bins[binID].dRight!=0)
					{
						newRight+=m_bins[binID].dRight;
					}
					else if(m_bins[m_bins[binID].rightID].dLeft!=0)
					{
						newRight-=m_bins[m_bins[binID].rightID].dLeft;
					}
				}

				const double ox=m_db->m_modules[moduleIDs[i]].m_x;
				double newx=(newRight*(ox-m_bins[binID].x)+newLeft*(m_bins[binID].x+m_bins[binID].width-ox))/(m_bins[binID].width);

				newx=alphaX*(newx-ox)+ox;
								

				//confine block in core region
				safeMove(moduleIDs[i],newx,m_db->m_modules[moduleIDs[i]].m_y);

			}
			else //vertical expand
			{
				double newLeft=m_bins[binID].y;
				double newRight=m_bins[binID].y+m_bins[binID].height;
				if(m_bins[binID].bottomID!=-1)
				{
					if(m_bins[binID].dBottom!=0)
					{
						newLeft-=m_bins[binID].dBottom;
					}
					else if(m_bins[m_bins[binID].bottomID].dTop!=0)
					{
						newLeft+=m_bins[m_bins[binID].bottomID].dTop;
					}
				}
				if(m_bins[binID].topID!=-1)
				{
					if(m_bins[binID].dTop!=0)
					{
						newRight+=m_bins[binID].dTop;
					}
					else if(m_bins[m_bins[binID].topID].dBottom!=0)
					{
						newRight-=m_bins[m_bins[binID].topID].dBottom;
					}
				}

				const double oy=m_db->m_modules[moduleIDs[i]].m_y;
				double newy=(newRight*(oy-m_bins[binID].y)+newLeft*(m_bins[binID].y+m_bins[binID].height-oy))/(m_bins[binID].height);
				newy=alphaY*(newy-oy)+oy;

				//confine block in core region
				safeMove(moduleIDs[i],m_db->m_modules[moduleIDs[i]].m_x,newy);

			}
		}
	}
}

void CDiffusion::updateBinVelosity(bool isX)
{

//	cout<<" call:"<<m_binRows*m_binColumns<<" \n";
	for(unsigned int i=0; i<m_bins.size() ; i++)
	{
		if(isX==true)
		{
			double R1=0; //left
			double R2=0; //right
			//calc left
			if(m_bins[i].leftID!=-1)
			{
				if(m_bins[i].density>m_bins[m_bins[i].leftID].density)
				{
					if(m_bins[i].density<this->maxDensity)
					{
						R1=m_bins[i].density-m_bins[m_bins[i].leftID].density;
					}
					else if(m_bins[i].density>=maxDensity && m_bins[m_bins[i].leftID].density<maxDensity)
					{
						R1=maxDensity-m_bins[m_bins[i].leftID].density;
					}
				}
			}

			if(m_bins[i].rightID!=-1)
			{
				if(m_bins[i].density>m_bins[m_bins[i].rightID].density)
				{
					if( m_bins[i].density<this->maxDensity)
					{
						R2=m_bins[i].density-m_bins[m_bins[i].rightID].density;
					}
					else if(m_bins[i].density>=maxDensity && m_bins[m_bins[i].rightID].density<maxDensity)
					{
						R2=maxDensity-m_bins[m_bins[i].rightID].density;
					}
				}
			}

			double rsum=R1+R2;
			if( rsum>0)
			{

				double delta=m_bins[i].width*deltaT;
				if(m_bins[i].density<this->maxDensity)
				{
					delta=delta*m_bins[i].density;
				}
				m_bins[i].dLeft=delta*(R1/rsum);
				m_bins[i].dRight=delta*(R2/rsum);
			}


		}
		else  //isX==false, vertical expand
		{
			double R1=0; //bottom
			double R2=0; //top
			//calc left
			if(m_bins[i].bottomID!=-1)
			{
				if(m_bins[i].density>m_bins[m_bins[i].bottomID].density)
				{
					if(m_bins[i].density<this->maxDensity)
					{
						R1=m_bins[i].density-m_bins[m_bins[i].bottomID].density;
					}
					else if(m_bins[i].density>=maxDensity && m_bins[m_bins[i].bottomID].density<maxDensity)
					{
						R1=maxDensity-m_bins[m_bins[i].bottomID].density;
					}
				}
			}

			if(m_bins[i].topID!=-1)
			{
				if(m_bins[i].density>m_bins[m_bins[i].topID].density)
				{
					if( m_bins[i].density<this->maxDensity)
					{
						R2=m_bins[i].density-m_bins[m_bins[i].topID].density;
					}
					else if(m_bins[i].density>=maxDensity && m_bins[m_bins[i].topID].density<maxDensity)
					{
						R2=maxDensity-m_bins[m_bins[i].topID].density;
					}
				}
			}

			double rsum=R1+R2;
			if( rsum>0)
			{
				double delta=m_bins[i].height*deltaT;
				if(m_bins[i].density<this->maxDensity)
				{
					delta=delta*m_bins[i].density;
				}
				m_bins[i].dBottom=delta*(R1/rsum);
				m_bins[i].dTop=delta*(R2/rsum);
			}
		}


	}
}
void CDiffusion::updateBinCellShifting(bool isX)
{

//	cout<<" call:"<<m_binRows*m_binColumns<<" \n";
	for(unsigned int i=0; i<m_bins.size() ; i++)
	{
		if(isX==true)
		{
			for(int i=0; i<m_binRows ; i++)
			{
				for(int j=0; j<m_binColumns-1; j++)
				{
					double OBim1;
					if(j==0)
					{
						OBim1=m_bins[i*m_binColumns+j].x;
					}
					else
					{
						OBim1=m_bins[i*m_binColumns+j-1].nbi;
					}

					double Ui=m_bins[i*m_binColumns+j].density;
					double Uip1=m_bins[i*m_binColumns+j+1].density;
					double OBip1=m_bins[i*m_binColumns+j+1].x+m_bins[i*m_binColumns+j+1].width;
					m_bins[i*m_binColumns+j].nbi= ( OBim1*(Uip1+1.5)+OBip1*(Ui+1.5) )/(Ui+Uip1+3);
					if(m_bins[i*m_binColumns+j].nbi>(m_bins[i*m_binColumns+j].x+m_bins[i*m_binColumns+j].width))
					{
						m_bins[i*m_binColumns+j].dRight=m_bins[i*m_binColumns+j].nbi-(m_bins[i*m_binColumns+j].x+m_bins[i*m_binColumns+j].width);
					}
					else
					{
						m_bins[i*m_binColumns+j+1].dLeft=(m_bins[i*m_binColumns+j].x+m_bins[i*m_binColumns+j].width)-m_bins[i*m_binColumns+j].nbi;
					}
				}
			}

		}
		else  //isX==false, vertical expand
		{
			for(int i=0; i<m_binRows-1 ; i++)
			{
				for(int j=0; j<m_binColumns; j++)
				{
					double OBim1;
					if(i==0)
					{
						OBim1=m_bins[i*m_binColumns+j].y;
					}
					else
					{
						OBim1=m_bins[(i-1)*m_binColumns+j].nbiy;
					}

					double Ui=m_bins[i*m_binColumns+j].density;
					double Uip1=m_bins[(i+1)*m_binColumns+j].density;
					double OBip1=m_bins[(i+1)*m_binColumns+j].y+m_bins[(i+1)*m_binColumns+j].height;
					m_bins[i*m_binColumns+j].nbiy= ( OBim1*(Uip1+1.5)+OBip1*(Ui+1.5) )/(Ui+Uip1+3);
					if(m_bins[i*m_binColumns+j].nbiy>(m_bins[i*m_binColumns+j].y+m_bins[i*m_binColumns+j].height))
					{
						m_bins[i*m_binColumns+j].dTop=m_bins[i*m_binColumns+j].nbiy-(m_bins[i*m_binColumns+j].y+m_bins[i*m_binColumns+j].height);
					}
					else
					{
						m_bins[(i+1)*m_binColumns+j].dBottom=(m_bins[i*m_binColumns+j].y+m_bins[i*m_binColumns+j].height)-m_bins[i*m_binColumns+j].nbiy;
					}
				}
			}
		}


	}
}


bool CDiffusion::checkFinish()
{
	bool finish=true;
	for(unsigned int i=0; i<m_bins.size() ; i++)
	{
		if(m_bins[i].density>maxDensity)
		{
			finish=false;
			break;
		}
	}
	return finish;
}

void CDiffusion::printBin()
{
	cout<<"\nPrint Bins:";
	for(int i=m_binRows-1; i>=0 ; i--)
	{

		cout<<"\n Row#"<<i<<" ";
		for(int j=0; j<m_binColumns; j++)
		{
	//		printf(" #%d d:%f Vx:%f Vy:%f ",j,m_bins[i*m_binColumns+j].density,m_bins[i*m_binColumns+j].V_x,m_bins[i*m_binColumns+j].V_y);
			printf(" #%d d:%f ",j,m_bins[i*m_binColumns+j].density);
			//cout<<" i:"<<i<<" j:"<<j<<" bin#"<<i*m_binColumns+j;
		}
	}
	cout<<"\n";
}
void CDiffusion::safeMove(const int& i, double x,  double y)
{

	//confine block in core region
	if( (x+m_db->m_modules[i].m_width)>m_Rgn.right)
		x=m_Rgn.right-m_db->m_modules[i].m_width;
	else if(x<m_Rgn.left)
		x=m_Rgn.left;
	if( (y+m_db->m_modules[i].m_height)>m_Rgn.top)
		y=m_Rgn.top-m_db->m_modules[i].m_height;
	else if(y<m_Rgn.bottom)
		y=m_Rgn.bottom;
	
	m_db->SetModuleLocation(i,x,y);
}
void CDiffusion::unitVector( double& vx, double& vy)
{
	if(vx==0&&vy==0)
		return;
	double length=sqrt(vx*vx+vy*vy);
	vx=vx/length;
	vy=vy/length;
}

void CDiffusion::runDiffusionSpreading()
{
	//-------------------------
	// x-direction diffusion
	//-------------------------
	//1.clear movalbe module density data
	this->reset();

	//2.add movable module into bins
	this->addmodule();

	//3.calc new density of bins according to module occupied area
	this->updateDensity();
	alphaX=0.02+(0.5/maxBinUtil)*(moduleWHratio);
	alphaY=0.02+(0.5/maxBinUtil);
	//4.update bin velosity according to new bin density
	this->updateBinVelosity(true);

	//5.move movable modules according to new bin velosity
	this->moveAllModule(true);


	//-------------------------
	// y-direction diffusion
	//-------------------------
	this->reset();

	//2.add movable module into bins
	this->addmodule();

	//3.calc new density of bins according to module occupied area
	this->updateDensity();
	alphaX=deltaT+(0.5/maxBinUtil)*(moduleWHratio);
	alphaY=deltaT+(0.5/maxBinUtil);
	//4.update bin velosity according to new bin density
	this->updateBinVelosity(false);

	//5.move movable modules according to new bin velosity
	this->moveAllModule(false);
}
void CDiffusion::runDiffusionSpreadingS()
{
	cout<<"\n Rows:"<<this->m_binRows<<" Cols:"<<this->m_binColumns;
	//-------------------------
	// x-direction diffusion
	//-------------------------
	//1.clear movalbe module density data
	this->reset();

	//2.add movable module into bins
	this->addmoduleF();

	//3.calc new density of bins according to module occupied area
	this->updateDensity();
	alphaX=0.02+(0.5/maxBinUtil)*(moduleWHratio);
	alphaY=0.02+(0.5/maxBinUtil);
	//4.update bin velosity according to new bin density
	this->updateBinCellShifting(true);

	//5.move movable modules according to new bin velosity
	this->moveAllModule(true);


	//-------------------------
	// y-direction diffusion
	//-------------------------
	this->reset();

	//2.add movable module into bins
	this->addmoduleF();

	//3.calc new density of bins according to module occupied area
	this->updateDensity();
	alphaX=deltaT+(0.5/maxBinUtil)*(moduleWHratio);
	alphaY=deltaT+(0.5/maxBinUtil);
	//4.update bin velosity according to new bin density
	this->updateBinCellShifting(false);

	//5.move movable modules according to new bin velosity
	this->moveAllModule(false);
}
void CDiffusion::runDiffusionSpreadingGetForce( vector<double>& fx, vector<double>& fy )
{
	//-------------------------
	// x-direction diffusion
	//-------------------------
	//1.clear movalbe module density data
	this->reset();
    fx.resize(m_db->m_modules.size(),0);
    fy.resize(m_db->m_modules.size(),0);

	//2.add movable module into bins
	this->addmodule();

	//3.calc new density of bins according to module occupied area
	this->updateDensity();
	alphaX=0.02+(0.5/maxBinUtil)*(moduleWHratio);
	alphaY=0.02+(0.5/maxBinUtil);
	//4.update bin velosity according to new bin density
	this->updateBinVelosity(true);

	//5.move movable modules according to new bin velosity
    this->findForce(true,fx);


	//-------------------------
	// y-direction diffusion
	//-------------------------
	this->reset();

	//2.add movable module into bins
	this->addmodule();

	//3.calc new density of bins according to module occupied area
	this->updateDensity();
	alphaX=deltaT+(0.5/maxBinUtil)*(moduleWHratio);
	alphaY=deltaT+(0.5/maxBinUtil);
	//4.update bin velosity according to new bin density
	this->updateBinVelosity(false);

	//5.move movable modules according to new bin velosity
	findForce(false,fy);
}
