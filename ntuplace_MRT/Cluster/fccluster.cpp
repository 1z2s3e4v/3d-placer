#include "fccluster.h"	
#include <iomanip>

struct bestAffinityCompare
{
  bool operator()(const double& s1, const double& s2) const
  {
    return s1>s2;
  }
};
void showDBdata(CPlaceDB * m_pDB)
{
	return;
	int movablePin=0;
	for(unsigned int i=0; i<m_pDB->m_pins.size(); ++i)
	{
		if(m_pDB->m_modules[m_pDB->m_pins[i].moduleId].m_isFixed==false)
		{
			++movablePin;
		}
	}


	double tarea=0;
	int mvCount=0;
	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			tarea+=m_pDB->m_modules[i].m_area;
			++mvCount;
		}
	}
	double avg_movArea=tarea/mvCount;
//	cout<<"\nAvg area:"<<avg_movArea;
	double sigma;
	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			sigma+=pow((m_pDB->m_modules[i].m_area-avg_movArea),2);
		//	cout<<" "<<m_pDB->m_modules[i].m_area-avg_movArea;
		}
	}
	sigma=sqrt(sigma/mvCount);
	cout<<"\n***********DB statis:***********";
	cout<<"\nMovalbe Modules#:"<<mvCount;
	cout<<"\nMovalbe Pins#:"<<movablePin;
	cout<<"\nAvg area:"<<avg_movArea;
	cout<<"\nSD:"<<sigma;
	cout<<"\n******end DBdata***";
	

}

bool CClusterDB_ModuleCompare(const CClusterDB_Module &a, const CClusterDB_Module &b) 
{
	if(a.m_netIDs.size()!=b.m_netIDs.size())
	{
		return a.m_netIDs.size() > b.m_netIDs.size();
	}
	else
	{
		return a.m_area < b.m_area;
	}
}
void CFCClustering::declustering(CPlaceDB& dbsmall, CPlaceDB& dblarge,bool isPertab )
{
	int clusterMove=0;
	int declusterMove=0;
	for(int i=0; i<(int)dbsmall.m_modules.size(); i++)
	{
		if(dbsmall.m_modules[i].m_isFixed==false)
		{
			++clusterMove;
			for(unsigned int j=0; j<this->m_hierarchy[i].size(); j++)
			{
				assert(dblarge.m_modules[m_hierarchy[i][j]].m_isFixed==false);
				if(isPertab==true)
				{				
					double rdx = rand() % (int)(dbsmall.m_modules[i].m_width/2) + (int)(dbsmall.m_modules[i].m_x+dbsmall.m_modules[i].m_width/4);
					double rdy = rand() % (int)(dbsmall.m_modules[i].m_height/2) + (int)(dbsmall.m_modules[i].m_y+dbsmall.m_modules[i].m_height/4);
					//dblarge.SetModuleLocation(m_hierarchy[i][j],rdx,rdy);
					dblarge.MoveModuleCenter(m_hierarchy[i][j],rdx,rdy);
					++declusterMove;
				}
				else
				{
					double rdx=dbsmall.m_modules[i].m_cx;
					double rdy=dbsmall.m_modules[i].m_cy;
					//dblarge.SetModuleLocation(m_hierarchy[i][j],rdx,rdy);
					dblarge.MoveModuleCenter(m_hierarchy[i][j],rdx,rdy);	// debug by donnie
					++declusterMove;
				}
			}
		}
	}
	cout<<"\n LargeDB move#:"<<clusterMove<<" SmallDB move#:"<<declusterMove<<"\n";

}

//main use!!
void CFCClustering::clustering(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy ,int targetClusterNumber, double areaRatio)
{
	this->showMsg=false;
	if(showMsg)
	cout<<"\nStart FC clustering";
	double timestart=seconds();
	dbsmall=dblarge;

	m_pDB=&dbsmall;
	showDBdata(m_pDB);
	m_isNetExist.clear();
	m_isPinExist.clear();
	m_isModuleExist.clear();
	m_hierarchy.clear();

	m_isNetExist.resize(m_pDB->m_nets.size(),true);
	m_isPinExist.resize(m_pDB->m_pins.size(),true);
	m_isModuleExist.resize(m_pDB->m_modules.size(),true);
	m_hierarchy.resize(m_pDB->m_modules.size());
//	m_netSets.resize(m_pDB->m_modules.size());

	for(int i=0; i<(int)m_hierarchy.size(); i++)
	{
		m_hierarchy[i].push_back(i);

	}



	//find maxClusterArea 
	double tarea=0;
	int count=0;
	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			tarea+=m_pDB->m_modules[i].m_area;
			++count;
			//for(int j=0; j<(int)m_pDB->m_modules[i].m_netsId.size(); j++)
			//{
			//	m_netSets[i].insert(m_pDB->m_modules[i].m_netsId[j]);
			//}
		}

	}

	count=targetClusterNumber;
//	cout<<"\nTarea:"<<tarea<<" targetN:"<<targetClusterNumber<<" aratio:"<<areaRatio;
	double maxClusterArea=(tarea/count)*areaRatio;

	cout<<"\nStart UP:"<<seconds()-timestart;
	cout<<" target#:"<<targetClusterNumber<<" Max area:"<<maxClusterArea;

	clustering(targetClusterNumber,  maxClusterArea);
	m_isNetExist.clear();
	m_isPinExist.clear();
	m_isModuleExist.clear();
	hierarchy=m_hierarchy;
	m_hierarchy.clear();
	showDBdata(m_pDB);
	cout<<"\nFinish :"<<seconds()-timestart;

}
void CFCClustering::physicalclustering(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy ,int targetClusterNumber, double areaRatio)
{
	dbsmall=dblarge;
	m_pDB=&dbsmall;
	m_isNetExist.clear();
	m_isPinExist.clear();
	m_isModuleExist.clear();
	m_hierarchy.clear();

	m_isNetExist.resize(m_pDB->m_nets.size(),true);
	m_isPinExist.resize(m_pDB->m_pins.size(),true);
	m_isModuleExist.resize(m_pDB->m_modules.size(),true);
	m_hierarchy.resize(m_pDB->m_modules.size());
	for(int i=0; i<(int)m_hierarchy.size(); i++)
	{
		m_hierarchy[i].push_back(i);

	}


	//find maxClusterArea
	double tarea=0;
	int count=0;
	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			tarea+=m_pDB->m_modules[i].m_area;
			++count;

		}

	}
	count=targetClusterNumber;
	double maxClusterArea=(tarea/count)*areaRatio;
	clustering(targetClusterNumber,  maxClusterArea, true); //bool physical==true
	m_isNetExist.clear();
	m_isPinExist.clear();
	m_isModuleExist.clear();
	hierarchy=m_hierarchy;
	m_hierarchy.clear();


}

///////////////////////////////////////////////
//no use!!
//void CFCClustering::clustering(CPlaceDB& dblarge, CPlaceDB& dbsmall ,int targetClusterNumber, double areaRatio)
//{
//	dbsmall=dblarge;
//	m_pDB=&dbsmall;
//	m_isNetExist.clear();
//	m_isPinExist.clear();
//	m_isModuleExist.clear();
//	m_hierarchy.clear();
//
//	m_isNetExist.resize(m_pDB->m_nets.size(),true);
//	m_isPinExist.resize(m_pDB->m_pins.size(),true);
//	m_isModuleExist.resize(m_pDB->m_modules.size(),true);
//	m_hierarchy.resize(m_pDB->m_modules.size());
//	for(int i=0; i<(int)m_hierarchy.size(); i++)
//	{
//		m_hierarchy[i].push_back(i);
//
//	}
//
//
//	//find maxClusterArea
//	double tarea=0;
//	int count=0;
//	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
//	{
//		if(m_pDB->m_modules[i].m_isFixed==false)
//		{
//			tarea+=m_pDB->m_modules[i].m_area;
//			++count;
//
//		}
//
//	}
//	count=targetClusterNumber;
//	double maxClusterArea=(tarea/count)*areaRatio;
//	clustering(targetClusterNumber,  maxClusterArea);
//	m_isNetExist.clear();
//	m_isPinExist.clear();
//	m_isModuleExist.clear();
//}
/////////////////////////////////////////////////////////////

void CFCClustering::clustering(int targetClusterNumber, double maxClusterArea,bool physical)
{
	if(showMsg)
		cout<<"\nStart FC clustering";
	double timestart=seconds();	

	int numOfClusters=this->countMoveModules();
	if(showMsg)
		cout<<"\n# of Moveable:"<<numOfClusters;
	bool stop=false;
	int noc=numOfClusters;
	while(numOfClusters>targetClusterNumber && stop==false)
	{
		bool mergeFlag=false;
		//sort by vertex degree (same as mPL5's procedure)

		double findnodes=seconds();
		vector<int> nodes;
		nodes.reserve(m_pDB->m_modules.size());
		for(int i=0; i<(int)m_pDB->m_modules.size(); ++i)
		{
			if(m_pDB->m_modules[i].m_isFixed==false)
			{
				if(m_pDB->m_modules[i].m_area<maxClusterArea)
				{
					nodes.push_back(i);
				}
			}
		}
		sort(nodes.begin(),nodes.end(),CFCClusteringVertexCompare(*m_pDB));
		if(showMsg)
		{
			cout<<"\nsort by degree:"<<seconds()-findnodes;
		}
		findnodes=seconds();
		double chooseMaxAtime=0;
		double margeTime=0;
		//for all movable module, merge with heightest affinity module
		for(int i=0; i<(int)nodes.size(); ++i)
		{
			assert(m_pDB->m_modules[nodes[i]].m_isFixed==false && m_pDB->m_modules[nodes[i]].m_area<maxClusterArea);
			if( m_isModuleExist[nodes[i]]==true)
			{
				double temptime=seconds();
				int mergeID=chooseMaxAffinity(nodes[i],maxClusterArea,physical);
				chooseMaxAtime+=seconds()-temptime;

				temptime=seconds();
				if(mergeID!=-1)
				{
					this->mergeModule(nodes[i],mergeID);
					mergeFlag=true;
					--numOfClusters;
					if(numOfClusters==targetClusterNumber)
					{
						break;
					}
				}
				margeTime+=seconds()-temptime;

			}
		}
		int numAfterMerge=countMoveModules();
		if(showMsg)
		{
			cout<<"\nlook around:"<<seconds()-findnodes<<" , choose:"<<chooseMaxAtime<<" , merge:"<<margeTime<<", #ofMove:"<<numAfterMerge;
		}
		if(mergeFlag==false)
		{
			stop=true;
		}
		double decreasingRate=1-(double)numAfterMerge/(double)noc;
		noc=numAfterMerge;
		if(decreasingRate<0.05)
		{
			stop=true;
		}
	}
	if(showMsg)
	{
		cout<<"\nFinish Cluster:"<<seconds()-timestart;
	}
	removeExcessModulePin();
	if(showMsg)
	{
		cout<<"\nTotal Finish:"<<seconds()-timestart;
	}
}



void CFCClustering::removeExcessModulePin()
{
	//--------------------------------------------------------------------
	//remove excess pin,module from placeDB and update ids
	//--------------------------------------------------------------------

	//---------------
	//1. pins
	//---------------
	vector<int> pinIDmap;
	pinIDmap.resize(m_pDB->m_pins.size(),-1);
	vector<Pin> newPins;
	int newPinCount=0;
	for(int i=0; i<(int)this->m_isPinExist.size(); i++)
	{
		if(m_isPinExist[i]==true)
			++newPinCount;
	}
	newPins.reserve(newPinCount);
	for(int i=0; i<(int)m_pDB->m_pins.size(); i++)
	{
		if(m_isPinExist[i]==true)
		{
			pinIDmap[i]=(int)newPins.size();
			newPins.push_back(m_pDB->m_pins[i]);
		}
	}
	m_pDB->m_pins=newPins;

	//update nets, map the old pinID to new pinID
	for(int i=0; i<(int)m_pDB->m_nets.size(); i++)
	{
		for(int j=0; j<(int)m_pDB->m_nets[i].size(); j++)
		{
			assert(this->m_isPinExist[m_pDB->m_nets[i][j]]==true);
			assert(pinIDmap[m_pDB->m_nets[i][j]]!=-1);
			m_pDB->m_nets[i][j]=pinIDmap[m_pDB->m_nets[i][j]];
		}
	}
	//update modules , map the old pinID to new pinID
	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_isModuleExist[i]==true)
		{
			for(int j=0; j<(int)m_pDB->m_modules[i].m_pinsId.size(); j++)
			{
				assert(this->m_isPinExist[m_pDB->m_modules[i].m_pinsId[j]]==true);
				assert(pinIDmap[m_pDB->m_modules[i].m_pinsId[j]]!=-1);
				m_pDB->m_modules[i].m_pinsId[j]=pinIDmap[m_pDB->m_modules[i].m_pinsId[j]];
			}
		}
	}
	//---------------
	//2. modules
	//---------------
	vector<Module> newModules;
	vector< vector<int> > newHierarchy;
	int newModuleCount=0;
	//for debug
//	int hiCount=0;
	for(int i=0; i<(int)this->m_isModuleExist.size(); i++)
	{
		if(m_isModuleExist[i]==true)
		{
			++newModuleCount;
//			hiCount+=this->m_hierarchy[i].size();
		}
	}
//	cout<<"\n # of Hierarchy before trim module:"<<hiCount<<"\n";
	newModules.reserve(newModuleCount);
	newHierarchy.reserve(newModuleCount);
	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_isModuleExist[i]==true)
		{
			int newModuleID=(int)newModules.size();

			//update moduleId of the pin belongs to this module
			for(int j=0; j<(int)m_pDB->m_modules[i].m_pinsId.size(); j++)
			{
				assert(m_pDB->m_modules[i].m_pinsId[j]<(int)m_pDB->m_pins.size());
				m_pDB->m_pins[m_pDB->m_modules[i].m_pinsId[j]].moduleId=newModuleID;
			}
			newModules.push_back(m_pDB->m_modules[i]);
			newHierarchy.push_back(m_hierarchy[i]);

		}
	}
	this->m_hierarchy=newHierarchy;
	m_pDB->m_modules=newModules;
	m_pDB->m_nModules=(int)m_pDB->m_modules.size();
	m_pDB->m_nPins=(int)m_pDB->m_pins.size();

	//hiCount=0;
	//for(int i=0; i<(int)m_hierarchy.size(); i++)
	//{
	//	hiCount+=this->m_hierarchy[i].size();

	//}
	//cout<<"\n # of Hierarchy after trim module:"<<hiCount<<"\n";
}
void CFCClustering::mergeModule(int mID1, int mID2)
{
	m_pDB->m_modules[mID1].m_isCluster=true;
	//1.move pins of module2 to module1
	for(int i=0; i<(int)m_pDB->m_modules[mID2].m_pinsId.size(); i++)
	{
		//m_pDB->m_modules[mID1].m_pinsId.push_back(m_pDB->m_modules[mID2].m_pinsId[i]);
		m_pDB->m_pins[m_pDB->m_modules[mID2].m_pinsId[i]].moduleId=mID1;
		//m_pDB->m_pins[m_pDB->m_modules[mID2].m_pinsId[i]].xOff=0;
		//m_pDB->m_pins[m_pDB->m_modules[mID2].m_pinsId[i]].yOff=0;		
	}

	//2.recalc widht,height,area of module 1
	double a1=m_pDB->m_modules[mID1].m_area;
	double a2=m_pDB->m_modules[mID2].m_area; 

	//Modified by Jin 20061007 
	//double ar1=m_pDB->m_modules[mID1].m_width/m_pDB->m_modules[mID1].m_height; //find aspect ratio
	//double ar2=m_pDB->m_modules[mID2].m_width/m_pDB->m_modules[mID2].m_height;
	double ar1= m_pDB->m_modules[mID1].m_height == 0.0 ? 
	    1.0 : m_pDB->m_modules[mID1].m_width/m_pDB->m_modules[mID1].m_height; //find aspect ratio
	double ar2= m_pDB->m_modules[mID2].m_height == 0.0 ?
	    1.0 : m_pDB->m_modules[mID2].m_width/m_pDB->m_modules[mID2].m_height;
	//@Modified by Jin 20061007

	double newar=(a1*ar1+a2*ar2)/(a1+a2);

	// TEST by donnie
	newar = 1.0;	// square
	

	m_pDB->m_modules[mID1].m_area=a1+a2;
	m_pDB->m_modules[mID1].m_width=sqrt(newar*m_pDB->m_modules[mID1].m_area);

	//Modified by Jin 20061007
	//Original: m_pDB->m_modules[mID1].m_height=m_pDB->m_modules[mID1].m_area/m_pDB->m_modules[mID1].m_width;
	m_pDB->m_modules[mID1].m_height= m_pDB->m_modules[mID1].m_width == 0.0 ?
	    0.0 : m_pDB->m_modules[mID1].m_area/m_pDB->m_modules[mID1].m_width;
	//@Modified by Jin 20061007

	//determine new loaction of mID1 (center of gravity of mID1 and mID2 )
	double new_cx=(m_pDB->m_modules[mID1].m_cx*a1+m_pDB->m_modules[mID2].m_cx*a2)/(a1+a2);
	double new_cy=(m_pDB->m_modules[mID1].m_cy*a1+m_pDB->m_modules[mID2].m_cy*a2)/(a1+a2);	
	m_pDB->MoveModuleCenter(mID1, new_cx, new_cy);

	//3.build the hierarchy
	int tempsize=m_hierarchy[mID1].size();
	m_hierarchy[mID1].resize(tempsize+m_hierarchy[mID2].size());
	for(int i=0; i<(int)m_hierarchy[mID2].size(); i++)
	{
		m_hierarchy[mID1][tempsize+i]=m_hierarchy[mID2][i];		
	}
	m_isModuleExist[mID2]=false;

	//4.rebuild netsID and pinsID of moudle 1
	//m_netSets[mID1].insert(m_netSets[mID2].begin(),m_netSets[mID2].end());
	//m_netSets[mID2].clear();

//	set<int> netSet=m_netSets[mID1];
	set<int> netSet;

	for(int i=0; i<(int)m_pDB->m_modules[mID1].m_netsId.size(); i++)
	{
		netSet.insert(m_pDB->m_modules[mID1].m_netsId[i]);
	}
	for(int i=0; i<(int)m_pDB->m_modules[mID2].m_netsId.size(); i++)
	{
		netSet.insert(m_pDB->m_modules[mID2].m_netsId[i]);
	}

	int netsum=(int)m_pDB->m_modules[mID1].m_netsId.size()+(int)m_pDB->m_modules[mID2].m_netsId.size();
	int pinsum=(int)m_pDB->m_modules[mID1].m_pinsId.size()+(int)m_pDB->m_modules[mID2].m_pinsId.size();

	m_pDB->m_modules[mID1].m_netsId.clear();
	m_pDB->m_modules[mID1].m_netsId.reserve(netsum);

	m_pDB->m_modules[mID1].m_pinsId.clear();
	m_pDB->m_modules[mID1].m_pinsId.reserve(pinsum);

	set<int>::iterator it;
	for(it=netSet.begin(); it!=netSet.end(); ++it)
	{
		int netsize=this->getNetSize(*it);

		if(netsize==1) // inside net
		{
			this->m_isNetExist[*it]=false;
			for(int i=0; i<(int)m_pDB->m_nets[*it].size(); ++i)
			{
				this->m_isPinExist[m_pDB->m_nets[*it][i]]=false;
				assert(m_pDB->m_pins[m_pDB->m_nets[*it][i]].moduleId==mID1);
			}
			m_pDB->m_nets[*it].clear(); //clear the pinsID vector of the net
		}
		else //remove excess pin, if any
		{
			m_pDB->m_modules[mID1].m_netsId.push_back(*it);

			bool appear=false;
			vector<int> newNet;
			newNet.reserve(m_pDB->m_nets[*it].size());
			for(int i=0; i<(int)m_pDB->m_nets[*it].size(); ++i)
			{
				if(m_pDB->m_pins[m_pDB->m_nets[*it][i]].moduleId==mID1)
				{
					if(appear==false)
					{
						newNet.push_back(m_pDB->m_nets[*it][i]);
						m_pDB->m_modules[mID1].m_pinsId.push_back(m_pDB->m_nets[*it][i]);
						m_pDB->m_pins[m_pDB->m_nets[*it][i]].xOff=0;
						m_pDB->m_pins[m_pDB->m_nets[*it][i]].yOff=0;
						m_pDB->m_pins[m_pDB->m_nets[*it][i]].absX=m_pDB->m_modules[mID1].m_cx;
						m_pDB->m_pins[m_pDB->m_nets[*it][i]].absY=m_pDB->m_modules[mID1].m_cy;
						appear=true;
					}
					else  //appear is true => already 1 pin in module 1 (the new clustered module) =>remove excess pin
					{
						this->m_isPinExist[m_pDB->m_nets[*it][i]]=false;
					}
				}
				else
				{
					newNet.push_back(m_pDB->m_nets[*it][i]);
				}
			}
			m_pDB->m_nets[*it]=newNet;

		}
	
	}


}
void CFCClustering::getModuleNeighbors(int mID, set<int>& neighborSet)
{
	neighborSet.clear();
	//for all nets contain this module
	for(int j=0; j<(int)m_pDB->m_modules[mID].m_netsId.size(); j++)
	{
		int netid=m_pDB->m_modules[mID].m_netsId[j];

		//for all pins in this net
		for(int k=0; k<(int)m_pDB->m_nets[netid].size(); k++)
		{
			int pinid=m_pDB->m_nets[netid][k];

			if(m_pDB->m_pins[pinid].moduleId!=mID)
			{
				if(m_pDB->m_modules[m_pDB->m_pins[pinid].moduleId].m_isFixed==false)
				{
					neighborSet.insert(m_pDB->m_pins[pinid].moduleId);

				}
			}
		}
	}	
}

int CFCClustering::chooseMaxAffinity(int mID, double maxClusterArea, bool physical)
{
	map<int,double> affinityMap;

	//for all net belonging to the module, find its affinity
	for(int j=0; j<(int)m_pDB->m_modules[mID].m_netsId.size(); j++)
	{
		int netid=m_pDB->m_modules[mID].m_netsId[j];
		int netSize=getNetSize(netid);

		//for all pins in this net
		for(int k=0; k<(int)m_pDB->m_nets[netid].size(); k++)
		{
			int pinid=m_pDB->m_nets[netid][k];

			if(m_pDB->m_pins[pinid].moduleId!=mID)
			{
				if(m_pDB->m_modules[m_pDB->m_pins[pinid].moduleId].m_isFixed==false)
				{
					int mid=m_pDB->m_pins[pinid].moduleId;
					assert(netSize>1);


					//----------------
					// calc affinity
					//---------------
					double affinity=0;
					if(physical==false)
					{
						affinity=1/( (netSize-1)*(m_pDB->m_modules[mID].m_area + m_pDB->m_modules[mid].m_area));
					}
					else //physical clustering
					{
						double dist=1+sqrt( pow(m_pDB->m_modules[mID].m_cx-m_pDB->m_modules[mid].m_cx,2)+pow(m_pDB->m_modules[mID].m_cy-m_pDB->m_modules[mid].m_cy,2));
						affinity=1/( (netSize-1)*(m_pDB->m_modules[mID].m_area + m_pDB->m_modules[mid].m_area)*dist);
					}

					if(affinityMap.find(mid)!=affinityMap.end()) //the module exists in other net
					{
						affinityMap[mid]+=affinity;						
					}
					else
					{
						affinityMap.insert(pair<int,double>(mid,affinity));
					}

				}
			}
		}
	}


	//--------------------------------------------------------------------------------------------
	// find the max affinity without break the constraint
	//--------------------------------------------------------------------------------------------
	map<int,double>::iterator it;
	int target=-1;
	double maxAffinity=-1;

	for(it=affinityMap.begin(); it!=affinityMap.end(); ++it)
	{
		if(it->second>maxAffinity)
		{
			double combineArea=m_pDB->m_modules[mID].m_area + m_pDB->m_modules[it->first].m_area;
			if(combineArea<= maxClusterArea)
			{
				target=it->first;
				maxAffinity=it->second;
			}
		}

	}

	return target; //if target==-1, means all neighbor cells are larger than maxarea. Don't perform clustering on this cell

}


int CFCClustering::getNetSize(int netID)
{

	set<int> moduleSet;

	//for all pins in this net
	for(int k=0; k<(int)m_pDB->m_nets[netID].size(); k++)
	{
		moduleSet.insert(m_pDB->m_pins[m_pDB->m_nets[netID][k]].moduleId);		
	}

	return (int)moduleSet.size();
}

int CFCClustering::countMoveModules()
{
	int i=0;
	for(int j=0; j<(int)m_pDB->m_modules.size(); j++)
	{
		if(m_pDB->m_modules[j].m_isFixed==false && m_isModuleExist[j]==true)
			++i;
	}
	return i;
}

/////////////////////////////////////////////////
// CClusterDB
////////////////////////////////////////////////

void CClusterDB::placeDbIn(CPlaceDB& db)
{
	this->m_pDB=&db;
	m_movableCount=0;
	//set up modules
	m_modules.resize(db.m_modules.size());
	for(unsigned int i=0; i<db.m_modules.size(); ++i)
	{
		m_modules[i].m_mID=i;
		m_modules[i].m_isExist=true;
		m_modules[i].m_cx=db.m_modules[i].m_cx;
		m_modules[i].m_cy=db.m_modules[i].m_cy;
		m_modules[i].m_area=db.m_modules[i].m_area;
		m_modules[i].m_hierarchy.resize(1,i);
		if(db.m_modules[i].m_isFixed==false)
		{
			++m_movableCount;
		}
	}

	//set up nets
	m_nets.resize(db.m_nets.size());
	for(unsigned int i=0; i<db.m_nets.size(); ++i)
	{
		for(unsigned int j=0; j<db.m_nets[i].size(); ++j)
		{
			int mid=db.m_pins[ db.m_nets[i][j] ].moduleId;
			//if(db.m_modules[mid].m_isFixed==false)
			//{
				m_nets[i].insert( mid );
				this->m_modules[mid].m_netIDs.insert(i);
			//}
		}
	}


//	////////////////////////////////////////////////
//	//build neighborMaps
//	////////////////////////////////////////////////
////	m_neighborMaps.resize(db.m_modules.size() );
//	for(unsigned int i=0; i<m_modules.size(); ++i)
//	{
//		set<int>::iterator netIter;
//		for(netIter=m_modules[i].m_netIDs.begin(); netIter!=m_modules[i].m_netIDs.end(); ++netIter)
//		{
//			set<int>::iterator moduleIter;
//			for(moduleIter=m_nets[*netIter].begin(); moduleIter!=m_nets[*netIter].end(); ++moduleIter)
//			{
//				if(isModuleFix(*moduleIter)==false)
//				{
//					m_modules[i].m_neighborMaps[*moduleIter].insert(*netIter);	
//				}
//				
//			}
//		}
//	}
//
//	////////////////////////////////////////////////////
//	//build initial affinity maps
//	////////////////////////////////////////////////////
//	m_affinityMap.resize(m_neighborMaps.size(),0);
//	for(unsigned int i=0; i<m_affinityMap.size(); ++i)
//	{
//		if(db.m_modules[i].m_isFixed==false)
//		{
//			map<int,set<int> >::iterator it;
//			for(it=m_neighborMaps[i].begin(); it!=m_neighborMaps[i].end(); ++it)
//			{
//				if(db.m_modules[it->first].m_isFixed==false)
//				{
//					double affinity=0;
//					set<int>::iterator netIter;
//					for(netIter=it->second.begin() ; netIter!=it->second.end(); ++netIter)
//					{
//						//count affinity
//						assert(m_nets[*netIter].size()>1);
//						affinity+=1/( (m_nets[*netIter].size()-1)*(m_modules[i].m_area + m_modules[it->first].m_area));
//					}
//					m_affinityMap[i][it->first]=affinity;
//
//				}
//			}
//		}		
//	}
}
//void CClusterDB::placeDbOut( vector< vector<int> >& hierarchy)
//modify m_pDB's module data (this function is called after clustering is finished)

void CClusterDB::placeDbOut()
{

	m_isNetExist.resize(m_pDB->m_nets.size(),true);
	m_isPinExist.resize(m_pDB->m_pins.size(),true);
	m_isModuleExist.resize(m_pDB->m_modules.size(),true);
	m_hierarchy.clear();
	m_hierarchy.resize(m_pDB->m_modules.size());

	//clear pseude net (to bypass small fixed macros)
	this->m_nets.resize(m_pDB->m_nets.size());


	//--------------------------------------------------------------------
	// 1. update hierarchy, move pins to left module
	//--------------------------------------------------------------------
	for(int i=0; i<(int)m_modules.size(); ++i)
	{
		if(m_modules[i].m_isExist==true)
		{
			//update module width and height
			if(this->isModuleFix(i)==false && (m_modules[i].m_hierarchy.size()>1))
			{
				//Modified by Jin 20061003
				//if(m_pDB->m_modules[i].m_area==m_modules[i].m_area)
				//{
				//	cerr<<"\n i"<<i<<" dbArea:"<<m_pDB->m_modules[i].m_area<<" cdbArea:"<<m_modules[i].m_area<<" hiera"<<m_hierarchy[i].size();
				//}
				//if( m_pDB->m_modules[i].m_area==m_modules[i].m_area )
				//{
				    //assert(m_pDB->m_modules[i].m_area!=m_modules[i].m_area);
				    //fprintf( stderr, "Warning: m_pDB->m_modules[%d].m_area"
				    //	    "==m_modules[%d].m_area (%.2f == %.2f)\n",
				    //	    i, i, m_pDB->m_modules[i].m_area, m_modules[i].m_area );
				//}
				//@Modified by Jin 20061003
				
				m_pDB->m_modules[i].m_area=m_modules[i].m_area;
				double newar = 1.0;	// square
				m_pDB->m_modules[i].m_width=sqrt(newar*m_modules[i].m_area);
				//Modified by Jin 20061007
				//m_pDB->m_modules[i].m_height=m_modules[i].m_area/m_pDB->m_modules[i].m_width;
				m_pDB->m_modules[i].m_height= m_pDB->m_modules[i].m_width == 0.0 ? 
				    0.0 : m_modules[i].m_area/m_pDB->m_modules[i].m_width;
				//@Modified by Jin 20061007
			}
			m_hierarchy[i]=m_modules[i].m_hierarchy;
			for(int j=0; j<(int)m_modules[i].m_hierarchy.size(); ++j)
			{
				
				if(m_modules[i].m_hierarchy[j]!=i)
				{
					assert(this->isModuleFix(i)==false);
					for(unsigned int k=0; k<m_pDB->m_modules[m_modules[i].m_hierarchy[j]].m_pinsId.size(); ++k)
					{
						int pid=m_pDB->m_modules[m_modules[i].m_hierarchy[j]].m_pinsId[k];
						m_pDB->m_pins[pid].moduleId=i;
						m_pDB->m_pins[pid].xOff=0;
						m_pDB->m_pins[pid].yOff=0;
						m_pDB->m_pins[pid].absX=m_modules[i].m_cx;
						m_pDB->m_pins[pid].absY=m_modules[i].m_cy;

					}

				}
			}
			if(m_hierarchy[i].size()>1)
			{
				m_pDB->m_modules[i].m_isCluster=true;
				assert(this->isModuleFix(i)==false);
				for(unsigned int k=0; k<m_pDB->m_modules[i].m_pinsId.size(); ++k)
				{
					int pid=m_pDB->m_modules[i].m_pinsId[k];
					m_pDB->m_pins[pid].xOff=0;
					m_pDB->m_pins[pid].yOff=0;
					m_pDB->m_pins[pid].absX=m_modules[i].m_cx;
					m_pDB->m_pins[pid].absY=m_modules[i].m_cy;

				}

			
			}
		}
		else
		{
			m_isModuleExist[i]=false;
		}
	}

	//--------------------------------------------------------------------
	// 2. update m_pDB->m_net, remove unnecessary net and unnecessary pin
	//--------------------------------------------------------------------
	vector<Net> newNetList; // (kaie) Decrease net size by removing unnecessary nets from DB
	for(unsigned int i=0; i<this->m_nets.size(); ++i)
	{
		if(m_nets[i].size()<2)
		{
			m_isNetExist[i]=false;
			for(unsigned int j=0; j<m_pDB->m_nets[i].size(); ++j)
			{
				//test code
				//fprintf( stderr, "TEST: Set m_isPinExist[%d] to false\n", m_pDB->m_nets[i][j] );
				//@test code
				m_isPinExist[m_pDB->m_nets[i][j]]=false;  //remove pins
			}
			//m_pDB->m_nets[i].clear();
		}
		else //remove redundant pin (prevent 1 net has more than 1 pin in a module)
		{
			vector<int> newPinList;
			newPinList.reserve(m_pDB->m_nets[i].size());
			set<int> moduleSet;
			set<int>::iterator it;

			//for all pins in nets[i]
			for(unsigned int j=0; j<m_pDB->m_nets[i].size(); ++j)
			{
				it=moduleSet.find(m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId);
				if(it!=moduleSet.end()) //already 1 pin in this module
				{
					m_isPinExist[m_pDB->m_nets[i][j]]=false;
				}
				else
				{
					moduleSet.insert(m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId);
					newPinList.push_back(m_pDB->m_nets[i][j]);
				}

			}
			//m_pDB->m_nets[i]=newPinList;
			newNetList.push_back(newPinList);
		}
	}
	m_pDB->m_nets = newNetList;


	//--------------------------------------------------------------------
	//3. remove excess pin,module from placeDB and update ids
	//--------------------------------------------------------------------

	//---------------
	//3.1. pins
	//---------------
	vector<int> pinIDmap;
	pinIDmap.resize(m_pDB->m_pins.size(),-1);
	vector<Pin> newPins;
	int newPinCount=0;
	for(int i=0; i<(int)this->m_isPinExist.size(); i++)
	{
		if(m_isPinExist[i]==true)
			++newPinCount;
	}
	newPins.reserve(newPinCount);
	for(int i=0; i<(int)m_pDB->m_pins.size(); i++)
	{
		if(m_isPinExist[i]==true)
		{
			pinIDmap[i]=(int)newPins.size();
			newPins.push_back(m_pDB->m_pins[i]);
		}
	}
	m_pDB->m_pins=newPins;

	//update nets, map the old pinID to new pinID
	for(int i=0; i<(int)m_pDB->m_nets.size(); i++)
	{
		for(int j=0; j<(int)m_pDB->m_nets[i].size(); j++)
		{
		    //test code
		    //if( m_isPinExist[m_pDB->m_nets[i][j]]==false )
		    //{
		    //	fprintf( stderr, "TEST: m_isPinExist[%d] is false\n", m_pDB->m_nets[i][j] );
		    //} 
		    //@test code
		    
		    assert(this->m_isPinExist[m_pDB->m_nets[i][j]]==true);
		    assert(pinIDmap[m_pDB->m_nets[i][j]]!=-1);
		    m_pDB->m_nets[i][j]=pinIDmap[m_pDB->m_nets[i][j]];
		}
	}
	//--------------------------------------------------------------------
	// 3.2. update m_pDB->m_modules[mid].m_netsId/m_pinsId
	//--------------------------------------------------------------------
	for(int i=0; i<(int)m_pDB->m_modules.size(); ++i)
	{
		m_pDB->m_modules[i].m_netsId.clear();
		m_pDB->m_modules[i].m_pinsId.clear();		
	}
	for(int i=0; i<(int)m_pDB->m_nets.size(); ++i)
	{
		for(unsigned int j=0; j<m_pDB->m_nets[i].size(); ++j)
		{
			int mid=m_pDB->m_pins[m_pDB->m_nets[i][j]].moduleId;
			m_pDB->m_modules[mid].m_netsId.push_back(i);
			m_pDB->m_modules[mid].m_pinsId.push_back(m_pDB->m_nets[i][j]);	
		}
	}
	//---------------
	//3.3 clear redundant modules
	//---------------
	vector<Module> newModules;
	vector< vector<int> > newHierarchy;
	int newModuleCount=0;
	//for debug
//	int hiCount=0;
	for(int i=0; i<(int)this->m_isModuleExist.size(); i++)
	{
		if(m_isModuleExist[i]==true)
		{
			++newModuleCount;
//			hiCount+=this->m_hierarchy[i].size();
		}
	}
//	cout<<"\n # of Hierarchy before trim module:"<<hiCount<<"\n";
	newModules.reserve(newModuleCount);
	newHierarchy.reserve(newModuleCount);
	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_isModuleExist[i]==true)
		{
			int newModuleID=(int)newModules.size();

			//update moduleId of the pin belongs to this module
			for(int j=0; j<(int)m_pDB->m_modules[i].m_pinsId.size(); j++)
			{
				assert(m_pDB->m_modules[i].m_pinsId[j]<(int)m_pDB->m_pins.size());
				m_pDB->m_pins[m_pDB->m_modules[i].m_pinsId[j]].moduleId=newModuleID;
			}
			newModules.push_back(m_pDB->m_modules[i]);
			newHierarchy.push_back(m_hierarchy[i]);

		}
	}
	m_hierarchy=newHierarchy;
	m_pDB->m_modules=newModules;
	m_pDB->m_nModules=(int)m_pDB->m_modules.size();
	m_pDB->m_nPins=(int)m_pDB->m_pins.size();
}


void CClusterDB::showNetProfile()
{
	vector<int> nets;
	nets.resize(10,0);
	int netCount=0;
	for(int i=0; i<(int)this->m_nets.size(); ++i)
	{
		if(m_nets[i].size()>1)
		{
			++netCount;
			if(m_nets[i].size()<=10)
			{
				++nets[m_nets[i].size()-2];
			}
			else
			{
				++nets[9];
			}
		}
	}
	cout<<"\n========Net DATA:====== ActiveNET:"<<netCount<<"\n";
	for(unsigned int i=0; i<nets.size(); ++i)
	{
		cout<<i+2<<" degree Net:"<<nets[i]<<" || ";
		if(i==3)
			cout<<"\n";
		if(i==7)
			cout<<"\n";
	}
	cout<<"\n============\n";
}

bool CClusterDB::isModuleFix( int mID)
{
	return m_pDB->m_modules[mID].m_isFixed;

}
int CClusterDB::getMovableCount()
{
	return m_movableCount;
}

void CClusterDB::mergeModule(int mID1, int mID2)
{
	assert(m_pDB->m_modules[mID1].m_isFixed==false && m_pDB->m_modules[mID2].m_isFixed==false);
    //cout<<"Merging "<<mID1<<" with "<<mID2<<endl;
	m_modules[mID2].m_isExist=false;
	double a1=m_modules[mID1].m_area;
	double a2=m_modules[mID2].m_area; 

	//determine new loaction of mID1 (center of gravity of mID1 and mID2 )
	m_modules[mID1].m_cx=(m_modules[mID1].m_cx*a1+m_modules[mID2].m_cx*a2)/(a1+a2);
	m_modules[mID1].m_cy=(m_modules[mID1].m_cy*a1+m_modules[mID2].m_cy*a2)/(a1+a2);	

	m_modules[mID1].m_area=a1+a2;
	m_modules[mID1].m_hierarchy.insert(m_modules[mID1].m_hierarchy.end(),m_modules[mID2].m_hierarchy.begin(),m_modules[mID2].m_hierarchy.end());
	m_modules[mID1].m_netIDs.insert(m_modules[mID2].m_netIDs.begin(),m_modules[mID2].m_netIDs.end());

	//clear the ID of module2 from all its nets, add mID1 to those nets
	set<int>::iterator it;
	for(it=m_modules[mID2].m_netIDs.begin(); it!=m_modules[mID2].m_netIDs.end(); ++it)
	{
		assert(m_nets[*it].find(mID2)!=m_nets[*it].end());

		m_nets[*it].erase(mID2);
		m_nets[*it].insert(mID1);

		//remove the internal net from module 1's net set
		if(m_nets[*it].size()==1)
		{
			m_modules[mID1].m_netIDs.erase(*it);
		}
	}

	////remove the internal net from module 1's net set	
	//set<int> tempset=m_modules[mID1].m_netIDs;
	//for(it=tempset.begin(); it!=tempset.end(); ++it)
	//{
	//	if(this->m_nets[*it].size()<=1)
	//	{
	//		m_modules[mID1].m_netIDs.erase(*it);
	//	}
	//}

	--m_movableCount;


}


/////////////////////////////////////////////////
// CClusterDBFC
////////////////////////////////////////////////
void CClusterDBFC::clustering(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber, double areaRatio)
{


	bool showMsg=false;
	if(showMsg)
	{
		cout<<"\nStart FC clustering:target#:"<<targetClusterNumber;
		

	}
	double timestart=seconds();
	dbsmall=dblarge;

	m_pDB=&dbsmall;
	m_ClusterDB.placeDbIn(*m_pDB);



	//find maxClusterArea 
	double maxClusterArea=m_ClusterDB.findMaxClusterArea(targetClusterNumber, areaRatio);

	//--------------------------------------------------------------------------------
	//main clustering program
	//--------------------------------------------------------------------------------


	//add bypass net to prevnet clustering saturation
	addBypassNet();
	if(showMsg)
	{
		cout<<"\nDB in, add bypass net:"<<seconds()-timestart;
		timestart=seconds();
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
	//	m_ClusterDB.showNetProfile();
	}
	//-------------------
	// 1. merge 2-pin net
	//-------------------
	for(unsigned int i=0; i<m_ClusterDB.m_nets.size(); ++i)
	{
		if(m_ClusterDB.m_nets[i].size()==2)
		{
			set<int>::iterator it;
			it=m_ClusterDB.m_nets[i].begin();
			int mid1=*it;
			++it;
			assert(it!=m_ClusterDB.m_nets[i].end());
			int mid2=*it;
			assert((mid1<(int)m_ClusterDB.m_modules.size()) && (mid2<(int)m_ClusterDB.m_modules.size()) );
			if(m_ClusterDB.isModuleFix(mid1)==false && m_ClusterDB.isModuleFix(mid2)==false)
			{
				if( (m_ClusterDB.m_modules[mid1].m_area +m_ClusterDB.m_modules[mid2].m_area)<=(0.5*maxClusterArea) )
				{
					m_ClusterDB.mergeModule(mid1,mid2);
				}
			}
		}
		if(m_ClusterDB.getMovableCount()<=targetClusterNumber)
		{
			break;
		}
	}
	if(showMsg)
	{
		cout<<"\nMerge 2-pin net:"<<seconds()-timestart;
		timestart=seconds();
	//	m_ClusterDB.showNetProfile();
	}
	//-------------------
	// 2. first choice merge
	//-------------------

	int numOfClusters=m_ClusterDB.getMovableCount();
	if(showMsg)
	{
		cout<<"\n# of Moveable:"<<numOfClusters;
	}
	bool stop=false;
	int noc=numOfClusters;
	while(numOfClusters>targetClusterNumber && stop==false)
	{
		bool mergeFlag=false;
		//sort by vertex degree (same as mPL5's procedure)
		vector<int> nodes;
		nodes.reserve(m_ClusterDB.getMovableCount());
		for(int i=0; i<(int)m_ClusterDB.m_modules.size(); ++i)
		{
			if(m_ClusterDB.m_modules[i].m_isExist==true && m_ClusterDB.isModuleFix(i)==false &&
				m_ClusterDB.m_modules[i].m_isNoLegalNeighbor==false)
			{
				if(m_ClusterDB.m_modules[i].m_area<=(0.5*maxClusterArea))
				{
					nodes.push_back(i);
				}
			}
		}
		sort(nodes.begin(),nodes.end(),CFCDBClusteringVertexCompare(m_ClusterDB));
		if(showMsg)
		{
			cout<<"\nBuild nodes, sorting:"<<seconds()-timestart<<" Size of noes:"<<nodes.size();
			timestart=seconds();
		}
		//vector< CClusterDB_Module > modulesSort=this->m_ClusterDB.m_modules;

		//sort(modulesSort.begin(),modulesSort.end(),CClusterDB_ModuleCompare);


		//for all movable module, merge with heightest affinity module
		for(int i=0; i<(int)nodes.size(); ++i)
		{
			if( m_ClusterDB.m_modules[nodes[i]].m_isExist==true && m_ClusterDB.isModuleFix(nodes[i])==false)
			{
				int mergeID=chooseMaxAffinity( nodes[i] ,maxClusterArea );

				if(mergeID!=-1)
				{
					m_ClusterDB.mergeModule(nodes[i],mergeID);
					mergeFlag=true;
					if(m_ClusterDB.getMovableCount()<=targetClusterNumber)
					{
						stop=true;
						break;
					}
				}
				else
				{
					m_ClusterDB.m_modules[nodes[i]].m_isNoLegalNeighbor=true;
				}

			}
		}
		if(showMsg)
		{
			cout<<"\nRound :"<<seconds()-timestart;
			cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
		//	m_ClusterDB.showNetProfile();
			timestart=seconds();
		}
		if(mergeFlag==false)
		{
			stop=true; //after a round of search, no modules can be merged
		}
		int numAfterMerge=m_ClusterDB.getMovableCount();

		double decreasingRate=1-(double)numAfterMerge/(double)noc;
		noc=numAfterMerge;
		if(decreasingRate<0.001)
		{
			stop=true;
		}
	}
	//if(showMsg)
	//{
	//	cout<<"\nFinish Cluster:"<<seconds()-timestart;
	//}
//	removeExcessModulePin();
	m_ClusterDB.placeDbOut();
	hierarchy=m_ClusterDB.m_hierarchy;
	if(showMsg)
	{
		cout<<"\nDB out :"<<seconds()-timestart;
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount()<<"\n======================\n";
		timestart=seconds();
	}
	//if(showMsg)
	//{
	//	cout<<"\nTotal Finish:"<<seconds()-timestart;
	//}


}

//No Heuristic
void CClusterDBFC::clusteringNH(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber, double areaRatio)
{



	bool showMsg=false;
	if(showMsg)
	{
		cout<<"\nStart FC clustering:target#:"<<targetClusterNumber;
		

	}
	double timestart=seconds();
	dbsmall=dblarge;

	m_pDB=&dbsmall;
	m_ClusterDB.placeDbIn(*m_pDB);

	showDBdata(m_pDB);

	//find maxClusterArea 
	double maxClusterArea=m_ClusterDB.findMaxClusterArea(targetClusterNumber, areaRatio);
	cout<<"\nStart FC clustering:target#:"<<targetClusterNumber<<" Max area:"<<maxClusterArea;
	//--------------------------------------------------------------------------------
	//main clustering program
	//--------------------------------------------------------------------------------


	//add bypass net to prevnet clustering saturation
	if(showMsg)
	{
		cout<<"\nDB in, add bypass net:"<<seconds()-timestart;
		timestart=seconds();
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
	//	m_ClusterDB.showNetProfile();
	}


	//-------------------
	// 2. first choice merge
	//-------------------

	int numOfClusters=m_ClusterDB.getMovableCount();
	if(showMsg)
	{
		cout<<"\n# of Moveable:"<<numOfClusters;
	}
	bool stop=false;
	int noc=numOfClusters;
	while(numOfClusters>targetClusterNumber && stop==false)
	{
		bool mergeFlag=false;
		//sort by vertex degree (same as mPL5's procedure)
		vector<int> nodes;
		nodes.reserve(m_ClusterDB.getMovableCount());
		for(int i=0; i<(int)m_ClusterDB.m_modules.size(); ++i)
		{
			if(m_ClusterDB.m_modules[i].m_isExist==true && m_ClusterDB.isModuleFix(i)==false &&
				m_ClusterDB.m_modules[i].m_isNoLegalNeighbor==false)
			{

				nodes.push_back(i);

			}
		}
		sort(nodes.begin(),nodes.end(),CFCDBClusteringVertexCompare(m_ClusterDB));
		if(showMsg)
		{
			cout<<"\nBuild nodes, sorting:"<<seconds()-timestart<<" Size of noes:"<<nodes.size();
			timestart=seconds();
		}
		//vector< CClusterDB_Module > modulesSort=this->m_ClusterDB.m_modules;

		//sort(modulesSort.begin(),modulesSort.end(),CClusterDB_ModuleCompare);


		//for all movable module, merge with heightest affinity module
		for(int i=0; i<(int)nodes.size(); ++i)
		{
			if( m_ClusterDB.m_modules[nodes[i]].m_isExist==true && m_ClusterDB.isModuleFix(nodes[i])==false)
			{
				int mergeID=chooseMaxAffinity( nodes[i] ,maxClusterArea );

				if(mergeID!=-1)
				{
					m_ClusterDB.mergeModule(nodes[i],mergeID);
					mergeFlag=true;
					if(m_ClusterDB.getMovableCount()<=targetClusterNumber)
					{
						stop=true;
						break;
					}
				}
				else
				{
					m_ClusterDB.m_modules[nodes[i]].m_isNoLegalNeighbor=true;
				}

			}
		}
		if(showMsg)
		{
			cout<<"\nRound :"<<seconds()-timestart;
			cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
		//	m_ClusterDB.showNetProfile();
			timestart=seconds();
		}
		if(mergeFlag==false)
		{
			stop=true; //after a round of search, no modules can be merged
		}
		int numAfterMerge=m_ClusterDB.getMovableCount();

		double decreasingRate=1-(double)numAfterMerge/(double)noc;
		noc=numAfterMerge;
		if(decreasingRate<0.0001)
		{
			stop=true;
		}
	}
	//if(showMsg)
	//{
	//	cout<<"\nFinish Cluster:"<<seconds()-timestart;
	//}
//	removeExcessModulePin();
	m_ClusterDB.placeDbOut();
	hierarchy=m_ClusterDB.m_hierarchy;
	if(showMsg)
	{
		cout<<"\nDB out :"<<seconds()-timestart;
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount()<<"\n======================\n";
		timestart=seconds();
	}
	//if(showMsg)
	//{
	//	cout<<"\nTotal Finish:"<<seconds()-timestart;
	//}
	showDBdata(m_pDB);
	printf("\nFCClustering,  memory usage:%.2f",GetPeakMemoryUsage());
}
void CClusterDBFC::clusteringBLN(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber, double areaRatio)
{
	bool showMsg=false;
	if(showMsg)
	{
		cout<<"\nStart FC-BLN clustering:target#:"<<targetClusterNumber;
	}
	double timestart=seconds();
	dbsmall=dblarge;

	m_pDB=&dbsmall;
	m_ClusterDB.placeDbIn(*m_pDB);

	showDBdata(m_pDB);

	//find maxClusterArea 
	double maxClusterArea=m_ClusterDB.findMaxClusterArea(targetClusterNumber, areaRatio);


    //find max net degree
    int maxDBNetSize=0;
    cout<<"\n";
    for(int i=0; i<(int)m_ClusterDB.m_nets.size(); ++i)
    {
        if((int)m_ClusterDB.m_nets[i].size()>maxDBNetSize)
        {
            maxDBNetSize=m_ClusterDB.m_nets[i].size();
           // cout<<" "<<m_ClusterDB.m_nets[i].size();
        }
    }
    int maxNetSize=20;
    cout<<"\nStart FC clustering:target#:"<<targetClusterNumber<<" Max area:"<<maxClusterArea<<" Max Net Size:"<<maxDBNetSize;
	//--------------------------------------------------------------------------------
	//main clustering program
	//--------------------------------------------------------------------------------


	//add bypass net to prevnet clustering saturation
	if(showMsg)
	{
		cout<<"\nDB in, add bypass net:"<<seconds()-timestart;
		timestart=seconds();
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
	//	m_ClusterDB.showNetProfile();
	}


	//-------------------
	// 2. first choice merge
	//-------------------

	int numOfClusters=m_ClusterDB.getMovableCount();
	if(showMsg)
	{
		cout<<"\n# of Moveable:"<<numOfClusters;
	}
	bool stop=false;
	int noc=numOfClusters;
	while(numOfClusters>targetClusterNumber && stop==false)
	{
		bool mergeFlag=false;
		//sort by vertex degree (same as mPL5's procedure)
		vector<int> nodes;
		nodes.reserve(m_ClusterDB.getMovableCount());
		for(int i=0; i<(int)m_ClusterDB.m_modules.size(); ++i)
		{
			if(m_ClusterDB.m_modules[i].m_isExist==true && m_ClusterDB.isModuleFix(i)==false &&
				m_ClusterDB.m_modules[i].m_isNoLegalNeighbor==false)
			{

				nodes.push_back(i);

			}
		}
		sort(nodes.begin(),nodes.end(),CFCDBClusteringVertexCompare(m_ClusterDB));
		if(showMsg)
		{
			cout<<"\nBuild nodes, sorting:"<<seconds()-timestart<<" Size of noes:"<<nodes.size();
			timestart=seconds();
		}
		//vector< CClusterDB_Module > modulesSort=this->m_ClusterDB.m_modules;

		//sort(modulesSort.begin(),modulesSort.end(),CClusterDB_ModuleCompare);


		//for all movable module, merge with heightest affinity module
		for(int i=0; i<(int)nodes.size(); ++i)
		{
			if( m_ClusterDB.m_modules[nodes[i]].m_isExist==true && m_ClusterDB.isModuleFix(nodes[i])==false)
			{
				int mergeID=chooseMaxAffinity2( nodes[i] ,maxClusterArea, maxNetSize );

				if(mergeID!=-1)
				{
					m_ClusterDB.mergeModule(nodes[i],mergeID);
					mergeFlag=true;
					if(m_ClusterDB.getMovableCount()<=targetClusterNumber)
					{
						stop=true;
						break;
					}
				}
				else
				{
					m_ClusterDB.m_modules[nodes[i]].m_isNoLegalNeighbor=true;
				}

			}
		}
		if(showMsg)
		{
			cout<<"\nRound :"<<seconds()-timestart;
			cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
		//	m_ClusterDB.showNetProfile();
			timestart=seconds();
		}
		if(mergeFlag==false)
		{
			stop=true; //after a round of search, no modules can be merged
		}
		int numAfterMerge=m_ClusterDB.getMovableCount();

		double decreasingRate=1-(double)numAfterMerge/(double)noc;
		noc=numAfterMerge;
        if(decreasingRate<0.1)
        {
            if(maxNetSize<maxDBNetSize)
                maxNetSize+=20;
        }
		if(decreasingRate<0.0001)
		{
			stop=true;
		}
	}
	//if(showMsg)
	//{
	//	cout<<"\nFinish Cluster:"<<seconds()-timestart;
	//}
//	removeExcessModulePin();
	m_ClusterDB.placeDbOut();
	hierarchy=m_ClusterDB.m_hierarchy;
	if(showMsg)
	{
		cout<<"\nDB out :"<<seconds()-timestart;
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount()<<"\n======================\n";
		timestart=seconds();
	}
	//if(showMsg)
	//{
	//	cout<<"\nTotal Finish:"<<seconds()-timestart;
	//}
	showDBdata(m_pDB);
	printf("\nFCClustering,  memory usage:%.2f",GetPeakMemoryUsage());
}
void CClusterDBFC::setNeighborFalse(int mID, vector<bool>& falseFlag)
{
	falseFlag[mID]=false;
	set<int>::iterator netIter;
	for(netIter=m_ClusterDB.m_modules[mID].m_netIDs.begin(); netIter!=m_ClusterDB.m_modules[mID].m_netIDs.end(); ++netIter)
	{
//		int netid=*netIter;
		int netSize=(int)m_ClusterDB.m_nets[*netIter].size();
		if(netSize>1)
		{
			set<int>::iterator mIter; //module iterator
			for(mIter=m_ClusterDB.m_nets[*netIter].begin(); mIter!=m_ClusterDB.m_nets[*netIter].end(); ++mIter)
			{
				if(m_ClusterDB.isModuleFix(*mIter)==false && *mIter!=mID)
				{
					int mid=*mIter;
					falseFlag[mid]=false;

				}
			}
		}

	}
}

void CClusterDBFC::clusteringBC(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber, double areaRatio)
{



	bool showMsg=false;
	if(showMsg)
	{
		cout<<"\nStart FC clustering:target#:"<<targetClusterNumber;
		

	}
	double timestart=seconds();
	dbsmall=dblarge;

	m_pDB=&dbsmall;
	m_ClusterDB.placeDbIn(*m_pDB);

	showDBdata(m_pDB);

	//find maxClusterArea 
	double maxClusterArea=m_ClusterDB.findMaxClusterArea(targetClusterNumber, areaRatio);
	cout<<"\nStart FC clustering:target#:"<<targetClusterNumber<<" Max area:"<<maxClusterArea;
	//--------------------------------------------------------------------------------
	//main clustering program
	//--------------------------------------------------------------------------------


	//add bypass net to prevnet clustering saturation
	if(showMsg)
	{
		cout<<"\nDB in, add bypass net:"<<seconds()-timestart;
		timestart=seconds();
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
	//	m_ClusterDB.showNetProfile();
	}


	//-------------------
	// 2. first choice merge
	//-------------------

	vector<bool> _falseFlag;
	vector<int> _maxAffTarget;
	_falseFlag.resize(m_ClusterDB.m_modules.size(),true);
	_maxAffTarget.resize(m_ClusterDB.m_modules.size(),-1);

	multimap<double,int,bestAffinityCompare> lazyQueue;
	for(unsigned int i=0; i<m_ClusterDB.m_modules.size(); ++i)
	{
		if(m_ClusterDB.isModuleFix(i)==false)
		{
			double maxAff;
			int tar=this->chooseMaxAffinity(i,maxClusterArea,maxAff);
			if(tar!=-1)
				lazyQueue.insert(pair<double,int>(maxAff,i));
			_maxAffTarget[i]=tar;
		}
	}


	int numOfClusters=m_ClusterDB.getMovableCount();
	if(showMsg)
	{
		cout<<"\n# of Moveable:"<<numOfClusters;
	}
	bool stop=false;
//	int bcCount=0;
	while(numOfClusters>targetClusterNumber && stop==false)
	{

		multimap<double,int,bestAffinityCompare>::iterator iter=lazyQueue.begin();
		if(iter==lazyQueue.end())
		{
			stop=true;
		}
		//else if(m_ClusterDB.m_modules[iter->second].m_isExist==false)
		//{
		//	lazyQueue.erase(iter);
		//}
		else if(_falseFlag[iter->second]==false) //the max affinity may changed
		{
			if(m_ClusterDB.m_modules[iter->second].m_isExist==false)
			{
				lazyQueue.erase(iter);
			}
			else
			{
				int mid=iter->second;
				lazyQueue.erase(iter);
				double maxAff;
				int tar=this->chooseMaxAffinity(mid,maxClusterArea,maxAff);
				_maxAffTarget[mid]=tar;
				_falseFlag[mid]=true;
				if(tar!=-1)
				{
					lazyQueue.insert(pair<double,int>(maxAff,mid));
				}
			}
		}
		else
		{
			int mid=iter->second;
			lazyQueue.erase(iter);
			assert(m_ClusterDB.m_modules[mid].m_isExist==true);
			if(m_ClusterDB.m_modules[_maxAffTarget[mid]].m_isExist!=true)
			{
				cerr<<"\n_maxAffTarget:"<<_maxAffTarget[mid]<<" mid:"<<mid;
				double maxAff;
				int tar=this->chooseMaxAffinity(mid,maxClusterArea,maxAff);
				cerr<<" tar:"<<tar;

			}
			this->setNeighborFalse(_maxAffTarget[mid], _falseFlag);
			this->setNeighborFalse(mid, _falseFlag);
			m_ClusterDB.mergeModule(_maxAffTarget[mid],mid);


			//if(m_ClusterDB.m_modules[mid].m_area<maxClusterArea)
			//{
			//	double maxAff;
			//	int tar=this->chooseMaxAffinity(mid,maxClusterArea,maxAff);
			//	_maxAffTarget[mid]=tar;
			//	_falseFlag[mid]=true;
			//	if(tar!=-1)
			//	{
			//		lazyQueue.insert(pair<double,int>(maxAff,mid));
			//	}
			//}
			//if(m_ClusterDB.m_modules[_maxAffTarget[mid]].m_isExist==true)
			//{
			//	m_ClusterDB.mergeModule(mid,_maxAffTarget[mid]);
			//	this->setNeighborFalse(mid, _falseFlag);
			//	if(m_ClusterDB.m_modules[mid].m_area<maxClusterArea)
			//	{
			//		double maxAff;
			//		int tar=this->chooseMaxAffinity(mid,maxClusterArea,maxAff);
			//		_maxAffTarget[mid]=tar;
			//		_falseFlag[mid]=true;
			//		if(tar!=-1)
			//		{
			//			lazyQueue.insert(pair<double,int>(maxAff,mid));
			//		}
			//	}
			//}
			//else
			//{
			//	double maxAff;
			//	int tar=this->chooseMaxAffinity(mid,maxClusterArea,maxAff);
			//	_maxAffTarget[mid]=tar;
			//	_falseFlag[mid]=true;
			//	if(tar!=-1)
			//	{
			//		lazyQueue.insert(pair<double,int>(maxAff,mid));
			//	}
			//}

			



		}

		numOfClusters=m_ClusterDB.getMovableCount();



	}
	//if(showMsg)
	//{
	//	cout<<"\nFinish Cluster:"<<seconds()-timestart;
	//}
//	removeExcessModulePin();
	m_ClusterDB.placeDbOut();
	hierarchy=m_ClusterDB.m_hierarchy;
	if(showMsg)
	{
		cout<<"\nDB out :"<<seconds()-timestart;
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount()<<"\n======================\n";
		timestart=seconds();
	}
	//if(showMsg)
	//{
	//	cout<<"\nTotal Finish:"<<seconds()-timestart;
	//}
	showDBdata(m_pDB);
	printf("\nFCClustering,  memory usage:%.2f",GetPeakMemoryUsage());
}

void CClusterDBFC::logicContraction(int ClusterID, int mID, const vector< set<int> >& oldNet, double& EF, double& IF)
{
	//------------------------------
	//1.Build Cluster Set
	//------------------------------
	set<int> CSet;
	for(int i=0; i<(int)m_ClusterDB.m_modules[ClusterID].m_hierarchy.size(); ++i)
	{
		CSet.insert(m_ClusterDB.m_modules[ClusterID].m_hierarchy[i]);
	}

	//------------------------------
	// 2.For all neighbors of mID, store weight sum in EF(external) or IF(internal) 
	//------------------------------
	EF=0;
	IF=0;
	map<int,double> weightMap;

	//for all net belonging to the module, find its affinity
	set<int>::iterator netIter;
	for(netIter=m_ClusterDB.m_modules[mID].m_netIDs.begin(); netIter!=m_ClusterDB.m_modules[mID].m_netIDs.end(); ++netIter)
	{
//		int netid=*netIter;
		int netSize=(int)oldNet[*netIter].size();
		if(netSize>1)
		{
			set<int>::iterator mIter; //module iterator
			for(mIter=oldNet[*netIter].begin(); mIter!=oldNet[*netIter].end(); ++mIter)
			{
				if(*mIter!=mID)
				{
					int mid=*mIter;
					//----------------
					// calc affinity
					//---------------
					double affinity=0;
					affinity=1/(double)(netSize-1);

					if(weightMap.find(mid)!=weightMap.end()) //the module exists in other net
					{
						weightMap[mid]+=affinity;						
					}
					else
					{
						weightMap.insert(pair<int,double>(mid,affinity));
					}
				}
			}
		}

	}

	map<int,double>::iterator it;


	for(it=weightMap.begin(); it!=weightMap.end(); ++it)
	{
		if(CSet.find(it->first)!=CSet.end()) //module it is inside the cluster
		{
			IF+=it->second;
		}
		else
		{
			EF+=it->second;
		}

	}
//	cout<<" [IF:"<<IF<<" EF:"<<EF<<"] ";
}

//void CClusterDBFC::creatClusterFromSeed(int mID, double maxClusterArea, CClusterDB& db_bk,vector<int>& exPin,vector<int>& totalPin)
void CClusterDBFC::creatClusterFromSeed(int mID, double maxClusterArea)

{
	//cerr<<" [ SG:";
	double min_p=10000000;
	//vector<int> exPinBK=exPin;
	//vector<int> totalPinBK=totalPin;
//	int failCount=0;
	set<int> cluster;
	set<int> bestCluster;
	map<int,int> clusterNetSize;
	cluster.insert(mID);
	set<int> neighbor;
	set<int>::iterator netIter;
	int totalPins=0;
	for(netIter=m_ClusterDB.m_modules[mID].m_netIDs.begin(); netIter!=m_ClusterDB.m_modules[mID].m_netIDs.end(); ++netIter)
	{
//		int netid=*netIter;
		int netSize=(int)m_ClusterDB.m_nets[*netIter].size();
		if(netSize>1)
		{
			set<int>::iterator mIter; //module iterator
			for(mIter=m_ClusterDB.m_nets[*netIter].begin(); mIter!=m_ClusterDB.m_nets[*netIter].end(); ++mIter)
			{
				if(m_ClusterDB.isModuleFix(*mIter)==false && *mIter!=mID)
				{
					 neighbor.insert(*mIter);
				}
			}
			clusterNetSize[*netIter]=netSize;
			totalPins+=netSize-1;
		}

	}
	bestCluster=cluster;
//	int totalPins=m_ClusterDB.m_modules[mID].m_netIDs.size();

	double cSize=m_ClusterDB.m_modules[mID].m_area;
	bool stop=false;
	while(stop==false)
	{
		int mergeID= chooseMaxConnectivity(cluster, neighbor,cSize,clusterNetSize,maxClusterArea);
		if(mergeID!=-1)
		{
			//TODO:find a method to calc exPin of cluster!!
			totalPins+=m_ClusterDB.m_modules[mergeID].m_netIDs.size();
			for(netIter=m_ClusterDB.m_modules[mergeID].m_netIDs.begin(); netIter!=m_ClusterDB.m_modules[mergeID].m_netIDs.end(); ++netIter)
			{
				if(clusterNetSize.find(*netIter)!=clusterNetSize.end())
				{
					if(clusterNetSize[*netIter]<=2) //2-pin nets between cluster and mergeID
					{
						clusterNetSize.erase(*netIter);
					}
					else
					{
						clusterNetSize[*netIter]=clusterNetSize[*netIter]-1;
					}
				}
				else
				{
					clusterNetSize[*netIter]=(int)m_ClusterDB.m_nets[*netIter].size();

				}
				totalPins+=(int)m_ClusterDB.m_nets[*netIter].size()-1;
			}
			cluster.insert(mergeID);
			neighbor.erase(mergeID);
			cSize+=m_ClusterDB.m_modules[mergeID].m_area;

			//calc Rent's rule
			double exPin=(double)clusterNetSize.size();
			double cs=(double)cluster.size();
			double avPin=(double)totalPins/cs;
			exPin=0;
			map<int,int>::iterator mit;
			for(mit=clusterNetSize.begin(); mit!=clusterNetSize.end(); ++mit)
			{
				exPin+=mit->second-1;
			}
			double p=log(exPin/avPin)/log(cs);
		//	cerr<<" [cs:"<<cs<<" avPin:"<<avPin<<" exPin:"<<exPin<<" p:"<<p<<"] ";
			if(p<min_p )
			{
				bestCluster=cluster;
				min_p=p;
		//		cerr<<" "<<min_p<<" "; 
			}
		}
		else
		{
			stop=true;
		}
		//if(failCount>3)
		//{
		//	m_ClusterDB=db_bk;
		//	exPin=exPinBK;
		//	totalPin=totalPinBK;
		//	return;
		//}
		//int mergeID=chooseMaxAffinity( mID ,maxClusterArea );
		//if(mergeID!=-1)
		//{
		//	totalPin[mID]=totalPin[mID]+totalPin[mergeID];
		//	m_ClusterDB.mergeModule( mID,mergeID);
		//	exPin[mID]=m_ClusterDB.m_modules[mID].m_netIDs.size();
		//	double p=log((double)exPin[mID]/(double)totalPin[mID])/log((double)m_ClusterDB.m_modules[mID].m_hierarchy.size());
		//	if(p<min_p)
		//	{
		//		db_bk=m_ClusterDB;
		//		exPinBK=exPin;
		//		totalPinBK=totalPin;
		//	}
		//	else
		//	{
		//		++failCount;
		//	}

		//}
		//else
		//{
		//	m_ClusterDB.m_modules[mID].m_isNoLegalNeighbor=true;
		//	m_ClusterDB=db_bk;
		//	exPin=exPinBK;
		//	totalPin=totalPinBK;
		//	return;
		////	_stop=false;
		//}
	}
	if(bestCluster.size()>1) //merge all cluster
	{
		set<int>::iterator it;
		for(it=bestCluster.begin(); it!=bestCluster.end(); ++it)
		{
			if(*it!=mID)
			{
				m_ClusterDB.mergeModule( mID,*it);
			}
		}
	//	cerr<<" cSize:"<<bestCluster.size()<<"]  ";
	}
	else
	{
	//	cerr<<" fail]  ";
		m_ClusterDB.m_modules[mID].m_isNoLegalNeighbor=true;
	}

}

//Ref. ICCAD2005 Pre-layout Physical Connectivity Prediction with Application in Clustering-Based Placement
void CClusterDBFC::clusteringSG(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber, double areaRatio)
{



	bool showMsg=false;
	if(showMsg)
	{
		cout<<"\nStart FC clustering:target#:"<<targetClusterNumber;		
	}
	double timestart=seconds();
	dbsmall=dblarge;

	m_pDB=&dbsmall;
	m_ClusterDB.placeDbIn(*m_pDB);

	showDBdata(m_pDB);

	//find maxClusterArea 
	double maxClusterArea=m_ClusterDB.findMaxClusterArea(targetClusterNumber, areaRatio);
	cout<<"\nStart FC clustering:target#:"<<targetClusterNumber<<" Max area:"<<maxClusterArea;
	//--------------------------------------------------------------------------------
	//main clustering program
	//--------------------------------------------------------------------------------


	//add bypass net to prevnet clustering saturation
	if(showMsg)
	{
		cout<<"\nDB in, add bypass net:"<<seconds()-timestart;
		timestart=seconds();
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
	//	m_ClusterDB.showNetProfile();
	}


	//-------------------
	// 2. first choice merge
	//-------------------


	int numOfClusters=m_ClusterDB.getMovableCount();
	if(showMsg)
	{
		cout<<"\n# of Moveable:"<<numOfClusters;
	}
	//vector<int> externalPins;
	//externalPins.resize(m_ClusterDB.m_modules.size());
	//vector<int> totalPins;
	//totalPins.resize(m_ClusterDB.m_modules.size());
	//for(int i=0; i<(int)m_ClusterDB.m_modules.size(); ++i)
	//{
	//	externalPins[i]=m_ClusterDB.m_modules[i].m_netIDs.size();
	//	totalPins[i]=m_ClusterDB.m_modules[i].m_netIDs.size();
	//}
	bool stop=false;
	int noc=numOfClusters;
	//vector< set<int> > DBnetsBK=m_ClusterDB.m_nets;
	//CClusterDB db_bak=m_ClusterDB;

	while(numOfClusters>targetClusterNumber && stop==false)
	{
		bool mergeFlag=false;
		//sort by vertex degree (same as mPL5's procedure)
		vector<int> nodes;
		nodes.reserve(m_ClusterDB.getMovableCount());
		for(int i=0; i<(int)m_ClusterDB.m_modules.size(); ++i)
		{
			if(m_ClusterDB.m_modules[i].m_isExist==true && m_ClusterDB.isModuleFix(i)==false &&
				m_ClusterDB.m_modules[i].m_isNoLegalNeighbor==false)
			{

				nodes.push_back(i);

			}
		}
		sort(nodes.begin(),nodes.end(),CFCDBClusteringVertexCompare(m_ClusterDB));
		if(showMsg)
		{
			cout<<"\nBuild nodes, sorting:"<<seconds()-timestart<<" Size of noes:"<<nodes.size();
			timestart=seconds();
		}
		//vector< CClusterDB_Module > modulesSort=this->m_ClusterDB.m_modules;

		//sort(modulesSort.begin(),modulesSort.end(),CClusterDB_ModuleCompare);


		//for all movable module, merge with heightest affinity module
		for(int i=0; i<(int)nodes.size(); ++i)
		{
			if( m_ClusterDB.m_modules[nodes[i]].m_isExist==true && m_ClusterDB.isModuleFix(nodes[i])==false)
			{
				creatClusterFromSeed(nodes[i],maxClusterArea);
	//			cerr<<" Merge:"<<m_ClusterDB.getMovableCount()<<" ";
				if(m_ClusterDB.getMovableCount()<=targetClusterNumber)
				{
					stop=true;
					break;
				}


					//if(mergeID!=-1)
					//{
					//	m_ClusterDB.mergeModule(nodes[i],mergeID);
					//	mergeFlag=true;
					//	if(m_ClusterDB.getMovableCount()<=targetClusterNumber)
					//	{
					//		stop=true;
					//		break;
					//	}
					//}
					//else
					//{
					//	mergeID=chooseMaxAffinity( nodes[i] ,maxClusterArea );
					//	if(mergeID!=-1)
					//	{
					//		m_ClusterDB.mergeModule(nodes[i],mergeID);
					//		mergeFlag=true;
					//		if(m_ClusterDB.getMovableCount()<=targetClusterNumber)
					//		{
					//			stop=true;
					//			break;
					//		}
					//	}
					//	else
					//	{
					//		m_ClusterDB.m_modules[nodes[i]].m_isNoLegalNeighbor=true;
					//	//	_stop=false;
					//	}
					//}
				

			}
		}
		if(showMsg)
		{
			cout<<"\nRound :"<<seconds()-timestart;
			cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
		//	m_ClusterDB.showNetProfile();
			timestart=seconds();
		}
		if(mergeFlag==false)
		{
			stop=true; //after a round of search, no modules can be merged
		}
		int numAfterMerge=m_ClusterDB.getMovableCount();

		double decreasingRate=1-(double)numAfterMerge/(double)noc;
		noc=numAfterMerge;
		if(decreasingRate<0.0001)
		{
			stop=true;
		}
	}
	//if(showMsg)
	//{
	//	cout<<"\nFinish Cluster:"<<seconds()-timestart;
	//}
//	removeExcessModulePin();
	m_ClusterDB.placeDbOut();
	hierarchy=m_ClusterDB.m_hierarchy;
	if(showMsg)
	{
		cout<<"\nDB out :"<<seconds()-timestart;
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount()<<"\n======================\n";
		timestart=seconds();
	}
	//if(showMsg)
	//{
	//	cout<<"\nTotal Finish:"<<seconds()-timestart;
	//}
	showDBdata(m_pDB);
	printf("\nFCClustering,  memory usage:%.2f",GetPeakMemoryUsage());
}

void CClusterDBBC::init()
{

	m_moduleBestAffinity.resize(m_ClusterDB.m_modules.size(),0);
	m_moduleBestAffinityID.resize(m_ClusterDB.m_modules.size(),-1);

	m_updateFlag.resize(m_ClusterDB.m_modules.size(),false);


	////////////////////////////////////////////////////
	//build initial affinity maps
	////////////////////////////////////////////////////
	for(unsigned int i=0; i<m_ClusterDB.m_modules.size(); ++i)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			updateAllAffinity(i,false);
		}		
	}
	m_updateFlag.resize(m_ClusterDB.m_modules.size(),false);
}


void CClusterDBBC::updateAllAffinity(int mID, bool changeFlag)
{
	map<int,double> m_moduleAffinityMap;
	m_moduleAffinityMap.clear();
	//for all net belonging to the module, find its affinity
	set<int>::iterator netIter;
	for(netIter=m_ClusterDB.m_modules[mID].m_netIDs.begin(); netIter!=m_ClusterDB.m_modules[mID].m_netIDs.end(); ++netIter)
	{
		int netSize=(int)m_ClusterDB.m_nets[*netIter].size();
		if(netSize>1)
		{
			set<int>::iterator mIter; //module iterator
			for(mIter=m_ClusterDB.m_nets[*netIter].begin(); mIter!=m_ClusterDB.m_nets[*netIter].end(); ++mIter)
			{
				if(m_ClusterDB.isModuleFix(*mIter)==false && *mIter!=mID && m_ClusterDB.m_modules[*mIter].m_isExist==true )
				{
					int mid=*mIter;
					if((m_ClusterDB.m_modules[mID].m_area + m_ClusterDB.m_modules[mid].m_area)<=m_maxClusterArea)
					{
						
						//----------------
						// calc affinity
						//---------------
						double affinity=0;
						affinity=1/( netSize*(m_ClusterDB.m_modules[mID].m_area + m_ClusterDB.m_modules[mid].m_area));
						if(m_moduleAffinityMap.find(mid)!=m_moduleAffinityMap.end()) //the module exists in other net
						{
							m_moduleAffinityMap[mid]+=affinity;						
						}
						else
						{
							m_moduleAffinityMap.insert(pair<int,double>(mid,affinity));
                            if (changeFlag)
							    m_updateFlag[mid]=true;
						}
					}
				}
			}
		}

	}
	map<int,double>::iterator it;
	//if no legal max affinity module, m_moduleBestAffinityID[i]==-1 !!
	if (m_moduleAffinityMap.size()<1)
	{
		m_moduleBestAffinity[mID]=-1;
		m_moduleBestAffinityID[mID]=-1;	
	}
	else
	{
		double max=0;
		map<int,double>::iterator it;
		for(it=m_moduleAffinityMap.begin(); it!=m_moduleAffinityMap.end(); ++it)
		{
            int moduleId=it->first;
            double affValue=it->second;
            assert(m_ClusterDB.m_modules[moduleId].m_isExist==true);
            if (m_moduleBestAffinity[moduleId]<affValue)
            {
                m_moduleBestAffinity[moduleId]=affValue;
                m_moduleBestAffinityID[moduleId]=mID; 
            }
            
			if(affValue>max)
			{
				m_moduleBestAffinity[mID]=affValue;
				m_moduleBestAffinityID[mID]=moduleId; 
				max=affValue;
			}
		}	
	}
	m_updateFlag[mID]=false;
}

void CClusterDBBC::clustering(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber, double areaRatio)
{

//	printf("\nBC start,  memory usage:%.2f",GetPeakMemoryUsage());
	bool showMsg=false;
	if(showMsg)
	{
		cout<<"\nStart BC clustering:target#:"<<targetClusterNumber;
	}
	double timestart=seconds();
	dbsmall=dblarge;
	m_pDB=&dbsmall;
	m_ClusterDB.placeDbIn(*m_pDB);
	
	if (showMsg)
		showDBdata(m_pDB);

	//find maxClusterArea 
	m_maxClusterArea=m_ClusterDB.findMaxClusterArea(targetClusterNumber, areaRatio);

	//--------------------------------------------------------------------------------
	//main clustering program
	//--------------------------------------------------------------------------------

	if(showMsg)
	{
		cout<<"\nDB in, add bypass net:"<<seconds()-timestart;
		timestart=seconds();
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
	//	m_ClusterDB.showNetProfile();
	}

	//-------------------
	// 2. Best choice merge
	//-------------------

	init();
//	m_ClusterDB.addBypassNet(5);
//	printf("\nFinish init, total %.2f seconds, memory usage:%.2f",seconds()-timestart,GetPeakMemoryUsage());
	timestart=seconds();
	//1.add all moudle to lazy queue
	multimap<double,int,bestAffinityCompare> lazyQueue;
	for(unsigned int i=0; i<m_ClusterDB.m_modules.size(); ++i)
	{
		if(m_moduleBestAffinityID[i]!=-1 && m_ClusterDB.isModuleFix(i)==false)
		{
			lazyQueue.insert(pair<double,int>(m_moduleBestAffinity[i],i));
		}
	}


	int numOfClusters=m_ClusterDB.getMovableCount();
//	printf("\nFinish Lqueue build, total %.2f seconds, memory usage:%.2f",seconds()-timestart,GetPeakMemoryUsage());
	timestart=seconds();
	if(showMsg)
	{
		cout<<"\n# of Moveable:"<<numOfClusters<<" queue size:"<<lazyQueue.size();
	}
	bool stop=false;
	multimap<double,int,bestAffinityCompare>::iterator iter=lazyQueue.begin();
	while(numOfClusters>targetClusterNumber && stop==false)
	{
		iter=lazyQueue.begin();
		if(iter==lazyQueue.end())
		{
			stop=true;
		}
		else 
		{
			int mid=iter->second;
			if (m_ClusterDB.m_modules[mid].m_isExist==false || m_moduleBestAffinityID[mid]==-1)
			{
				lazyQueue.erase(iter);
			}
			else
			{
                if(m_updateFlag[mid]==true || m_ClusterDB.m_modules[m_moduleBestAffinityID[mid]].m_isExist==false) //the max affinity may changed
				{
                    m_updateFlag[mid]=false;
					lazyQueue.erase(iter);
                    if(m_ClusterDB.m_modules[m_moduleBestAffinityID[mid]].m_isExist==false)
                    {
                        //cout<<"The Requested Modules Does not Exists!"<<endl;
					    updateAllAffinity(mid,false);
                    }
					if(m_moduleBestAffinityID[mid]!=-1)
					{
						lazyQueue.insert(pair<double,int>(m_moduleBestAffinity[mid],mid));
					}
				}
				else
				{
					lazyQueue.erase(iter);
					assert(m_moduleBestAffinityID[mid]!=-1);
					assert(m_ClusterDB.isModuleFix(m_moduleBestAffinityID[mid])==false);
					m_ClusterDB.mergeModule(m_moduleBestAffinityID[mid], mid);
					updateAllAffinity(m_moduleBestAffinityID[mid],true);
				}
			}
		}
		numOfClusters=m_ClusterDB.getMovableCount();
	}
//	printf("\nFinish BC, total %.2f seconds, memory usage:%.2f",seconds()-timestart,GetPeakMemoryUsage());
//	timestart=seconds();
	//if(showMsg)
	//{
	//	cout<<"\nFinish Cluster:"<<seconds()-timestart;
	//}
//	removeExcessModulePin();
	m_ClusterDB.placeDbOut();
	hierarchy=m_ClusterDB.m_hierarchy;
//	printf("\nFinish DB-Out, total %.2f seconds, memory usage:%.2f",seconds()-timestart,GetPeakMemoryUsage());
	timestart=seconds();
//	showDBdata(m_pDB);
	if(showMsg)
	{
			
		cout<<"\nDB out :"<<seconds()-timestart;
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount()<<"\n======================\n";
		timestart=seconds();
	}
	
	/*int _count=0;
	int _Totalsize=0;
	for(int i=0; i<(int)hierarchy.size(); i++)
	{
		if(hierarchy[i].size()>1)
		{
			++_count;
			_Totalsize+=hierarchy[i].size();
		}

	}
	double avg=(double)_Totalsize/_count;
	printf("\n#ofClustered:%d , avg Cluster Size:%.3f\n",_count,avg);*/

	// (donnie) remove clusterDB
	m_ClusterDB.m_nets.clear();
	m_ClusterDB.m_modules.clear();
	m_ClusterDB.m_isNetExist.clear();
	m_ClusterDB.m_isPinExist.clear();
	m_ClusterDB.m_isModuleExist.clear();
	m_ClusterDB.m_hierarchy.clear();
}

void CClusterDBBC::clusteringWithAcc(const CPlaceDB& dblarge, CPlaceDB& dbsmall, vector< vector<int> >& hierarchy,int targetClusterNumber, double areaRatio)
{

//	printf("\nBC start,  memory usage:%.2f",GetPeakMemoryUsage());
	bool showMsg=false;
	if(showMsg)
	{
		cout<<"\nStart BC clustering:target#:"<<targetClusterNumber;
	}
	double timestart=seconds();
	dbsmall=dblarge;
	m_pDB=&dbsmall;
	m_ClusterDB.placeDbIn(*m_pDB);
	
	if (showMsg)
		showDBdata(m_pDB);

	//find maxClusterArea 
	m_maxClusterArea=m_ClusterDB.findMaxClusterArea(targetClusterNumber, areaRatio);
	
	//--------------------------------------------------------------------------------
	// Heuristic: Merge Two Pin Nets
	//--------------------------------------------------------------------------------
	for(unsigned int i=0; i<m_ClusterDB.m_nets.size(); ++i)
	{
		if(m_ClusterDB.m_nets[i].size()==2)
		{
			set<int>::iterator it;
			it=m_ClusterDB.m_nets[i].begin();
			int mid1=*it;
			++it;
			assert(it!=m_ClusterDB.m_nets[i].end());
			int mid2=*it;
			assert((mid1<(int)m_ClusterDB.m_modules.size()) && (mid2<(int)m_ClusterDB.m_modules.size()) );
			if(m_ClusterDB.isModuleFix(mid1)==false && m_ClusterDB.isModuleFix(mid2)==false)
			{
				if( (m_ClusterDB.m_modules[mid1].m_area +m_ClusterDB.m_modules[mid2].m_area)<=(0.5*m_maxClusterArea) )
				{
					m_ClusterDB.mergeModule(mid1,mid2);
				}
			}
		}
		if(m_ClusterDB.getMovableCount()<=targetClusterNumber)
		{
			break;
		}
	}

	//--------------------------------------------------------------------------------
	//main best clustering program
	//--------------------------------------------------------------------------------

	if(showMsg)
	{
		cout<<"\nDB in, add bypass net:"<<seconds()-timestart;
		timestart=seconds();
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount();
	//	m_ClusterDB.showNetProfile();
	}

	//-------------------
	// 2. Best choice merge
	//-------------------

	init();
//	m_ClusterDB.addBypassNet(5);
//	printf("\nFinish init, total %.2f seconds, memory usage:%.2f",seconds()-timestart,GetPeakMemoryUsage());
	timestart=seconds();
	//1.add all moudle to lazy queue
	multimap<double,int,bestAffinityCompare> lazyQueue;
	for(unsigned int i=0; i<m_ClusterDB.m_modules.size(); ++i)
	{
		if(m_moduleBestAffinityID[i]!=-1 && m_ClusterDB.isModuleFix(i)==false)
		{
			lazyQueue.insert(pair<double,int>(m_moduleBestAffinity[i],i));
		}
	}


	int numOfClusters=m_ClusterDB.getMovableCount();
//	printf("\nFinish Lqueue build, total %.2f seconds, memory usage:%.2f",seconds()-timestart,GetPeakMemoryUsage());
	timestart=seconds();
	if(showMsg)
	{
		cout<<"\n# of Moveable:"<<numOfClusters<<" queue size:"<<lazyQueue.size();
	}
	bool stop=false;
	multimap<double,int,bestAffinityCompare>::iterator iter=lazyQueue.begin();
	while(numOfClusters>targetClusterNumber && stop==false)
	{
		iter=lazyQueue.begin();
		if(iter==lazyQueue.end())
		{
			stop=true;
		}
		else 
		{
			int mid=iter->second;
			if (m_ClusterDB.m_modules[mid].m_isExist==false || m_moduleBestAffinityID[mid]==-1)
			{
				lazyQueue.erase(iter);
			}
			else
			{
				if(m_updateFlag[mid]==true || m_ClusterDB.m_modules[m_moduleBestAffinityID[mid]].m_isExist==false) //the max affinity may changed
				{
                    m_updateFlag[mid]=false;
					lazyQueue.erase(iter);
                    if(m_ClusterDB.m_modules[m_moduleBestAffinityID[mid]].m_isExist==false)
                    {
                        //cout<<"The Requested Modules Does not Exists!"<<endl;
					    updateAllAffinity(mid,false);
                    }
					if(m_moduleBestAffinityID[mid]!=-1)
					{
						lazyQueue.insert(pair<double,int>(m_moduleBestAffinity[mid],mid));
					}
				}
				else
				{
					lazyQueue.erase(iter);
					assert(m_moduleBestAffinityID[mid]!=-1);
					assert(m_ClusterDB.isModuleFix(m_moduleBestAffinityID[mid])==false);
					m_ClusterDB.mergeModule(m_moduleBestAffinityID[mid], mid);
					updateAllAffinity(m_moduleBestAffinityID[mid],true);
				}
			}
		}
		numOfClusters=m_ClusterDB.getMovableCount();
	}
//	printf("\nFinish BC, total %.2f seconds, memory usage:%.2f",seconds()-timestart,GetPeakMemoryUsage());
//	timestart=seconds();
	//if(showMsg)
	//{
	//	cout<<"\nFinish Cluster:"<<seconds()-timestart;
	//}
//	removeExcessModulePin();
	m_ClusterDB.placeDbOut();
	hierarchy=m_ClusterDB.m_hierarchy;
//	printf("\nFinish DB-Out, total %.2f seconds, memory usage:%.2f",seconds()-timestart,GetPeakMemoryUsage());
	timestart=seconds();
//	showDBdata(m_pDB);
	if(showMsg)
	{
			
		cout<<"\nDB out :"<<seconds()-timestart;
		cout<<"\n# of Moveable:"<<m_ClusterDB.getMovableCount()<<"\n======================\n";
		timestart=seconds();
	}
	
	/*int _count=0;
	int _Totalsize=0;
	for(int i=0; i<(int)hierarchy.size(); i++)
	{
		if(hierarchy[i].size()>1)
		{
			++_count;
			_Totalsize+=hierarchy[i].size();
		}

	}
	double avg=(double)_Totalsize/_count;
	printf("\n#ofClustered:%d , avg Cluster Size:%.3f\n",_count,avg);*/

	// (donnie) remove clusterDB
	m_ClusterDB.m_nets.clear();
	m_ClusterDB.m_modules.clear();
	m_ClusterDB.m_isNetExist.clear();
	m_ClusterDB.m_isPinExist.clear();
	m_ClusterDB.m_isModuleExist.clear();
	m_ClusterDB.m_hierarchy.clear();
}


/*void CClusterDBBC::mergeModule(int mID1,int mID2)
{
	m_ClusterDB.mergeModule(mID1,mID2);
	updateAllAffinity(mID1);
}*/


void CClusterDBFC::addBypassNet()
{
	if(this->bypassMacroRowHeight!=0)
	{
		//for all fixed module
		for(int i=0; i<(int)m_ClusterDB.m_modules.size(); ++i)
		{
			if(m_ClusterDB.isModuleFix(i)==true && m_pDB->m_modules[i].m_height<=bypassMacroRowHeight)
			{
				int pseudoNetID=m_ClusterDB.m_nets.size();
				m_ClusterDB.m_nets.resize(pseudoNetID+1);

				//for all neighbors of the fixed module
				set<int>::iterator netIter;
				for(netIter=m_ClusterDB.m_modules[i].m_netIDs.begin(); netIter!=m_ClusterDB.m_modules[i].m_netIDs.end(); ++netIter)
				{
					set<int>::iterator mIter;
					for(mIter=m_ClusterDB.m_nets[*netIter].begin(); mIter!=m_ClusterDB.m_nets[*netIter].end(); ++mIter)
					{
						if(*mIter!=i)
						{
							m_ClusterDB.m_modules[*mIter].m_netIDs.insert(pseudoNetID);
							m_ClusterDB.m_nets[pseudoNetID].insert(*mIter);
						}

					}
				}

			}
		}
	}
}
void CClusterDB::addBypassNet(int bypassMacroRowHeight)
{
	if(bypassMacroRowHeight>0)
	{
		//for all fixed module
		for(int i=0; i<(int)m_modules.size(); ++i)
		{
			if(isModuleFix(i)==true && m_pDB->m_modules[i].m_height<=bypassMacroRowHeight)
			{
				int pseudoNetID=m_nets.size();
				m_nets.resize(pseudoNetID+1);

				//for all neighbors of the fixed module
				set<int>::iterator netIter;
				for(netIter=m_modules[i].m_netIDs.begin(); netIter!=m_modules[i].m_netIDs.end(); ++netIter)
				{
					set<int>::iterator mIter;
					for(mIter=m_nets[*netIter].begin(); mIter!=m_nets[*netIter].end(); ++mIter)
					{
						if(*mIter!=i)
						{
							m_modules[*mIter].m_netIDs.insert(pseudoNetID);
							m_nets[pseudoNetID].insert(*mIter);
						}

					}
				}

			}
		}
	}
}
double CClusterDB::findMaxClusterArea(const int& targetClusterNumber,const double& areaRatio)
{
	//find maxClusterArea 
	double tarea=0;

	for(int i=0; i<(int)m_pDB->m_modules.size(); i++)
	{
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			tarea+=m_pDB->m_modules[i].m_area;
			//for(int j=0; j<(int)m_pDB->m_modules[i].m_netsId.size(); j++)
			//{
			//	m_netSets[i].insert(m_pDB->m_modules[i].m_netsId[j]);
			//}
		}
	}
//	cout<<"\nTarea:"<<tarea<<" targetN:"<<targetClusterNumber<<" aratio:"<<areaRatio;
	return (tarea/targetClusterNumber)*areaRatio;
}
/*void CClusterDBBC::updateMaxAffinity(int mID)
{
	double max=0;
	set<int> removeSet;
	map<int,double>::iterator it;
	for(it=m_moduleAffinityMap.begin(); it!=m_moduleAffinityMap.end(); ++it)
	{
		if(m_ClusterDB.m_modules[it->first].m_isExist==false)
		{
			removeSet.insert(it->first);
		}
		else if(it->second>max)
		{
			m_moduleBestAffinity[mID]=it->second;
			m_moduleBestAffinityID[mID]=it->first; //if no legal max affinity module, m_moduleBestAffinityID[i]==-1 !!
			max=it->second;

		}
	}
	set<int>::iterator rit;
	for(rit=removeSet.begin(); rit!=removeSet.end(); ++rit)
	{
		m_moduleAffinityMap[mID].erase(*rit);
	}

	if(max==0)
		m_moduleBestAffinityID[mID]=-1;
	else
	{
		m_updateFlag[mID]=false;
	}

}*/

int CClusterDBFC::chooseMaxAffinity(int mID, double maxClusterArea)
{
	map<int,double> affinityMap;

	//for all net belonging to the module, find its affinity
	set<int>::iterator netIter;
	for(netIter=m_ClusterDB.m_modules[mID].m_netIDs.begin(); netIter!=m_ClusterDB.m_modules[mID].m_netIDs.end(); ++netIter)
	{
//		int netid=*netIter;
		int netSize=(int)m_ClusterDB.m_nets[*netIter].size();
		if(netSize>1)
		{
			set<int>::iterator mIter; //module iterator
			for(mIter=m_ClusterDB.m_nets[*netIter].begin(); mIter!=m_ClusterDB.m_nets[*netIter].end(); ++mIter)
			{
				if(m_ClusterDB.isModuleFix(*mIter)==false && *mIter!=mID)
				{
					int mid=*mIter;
					//----------------
					// calc affinity
					//---------------
					double affinity=0;
					affinity=1/( ((double)(netSize-1))*(m_ClusterDB.m_modules[mID].m_area + m_ClusterDB.m_modules[mid].m_area));

					if(affinityMap.find(mid)!=affinityMap.end()) //the module exists in other net
					{
						affinityMap[mid]+=affinity;						
					}
					else
					{
						affinityMap.insert(pair<int,double>(mid,affinity));
					}
				}
			}
		}

	}


	//--------------------------------------------------------------------------------------------
	// find the max affinity without break the constraint
	//--------------------------------------------------------------------------------------------
	map<int,double>::iterator it;
	int target=-1;
	double maxAffinity=-1;

	for(it=affinityMap.begin(); it!=affinityMap.end(); ++it)
	{
		if(it->second>maxAffinity)
		{
			double combineArea=m_ClusterDB.m_modules[mID].m_area + m_ClusterDB.m_modules[it->first].m_area;
			if(combineArea<= maxClusterArea)
			{
				target=it->first;
				maxAffinity=it->second;
			}
		}

	}

	return target; //if target==-1, means all neighbor cells are larger than maxarea. Don't perform clustering on this cell
}
int CClusterDBFC::chooseMaxAffinity2(const int mID, const double maxClusterArea, const int maxNetSize)
{
	map<int,double> affinityMap;

	//for all net belonging to the module, find its affinity
	set<int>::iterator netIter;
	for(netIter=m_ClusterDB.m_modules[mID].m_netIDs.begin(); netIter!=m_ClusterDB.m_modules[mID].m_netIDs.end(); ++netIter)
	{
//		int netid=*netIter;
		int netSize=(int)m_ClusterDB.m_nets[*netIter].size();
		if(netSize>1 && netSize<maxNetSize)
		{
			set<int>::iterator mIter; //module iterator
			for(mIter=m_ClusterDB.m_nets[*netIter].begin(); mIter!=m_ClusterDB.m_nets[*netIter].end(); ++mIter)
			{
				if(m_ClusterDB.isModuleFix(*mIter)==false && *mIter!=mID)
				{
					int mid=*mIter;
					//----------------
					// calc affinity
					//---------------
					double affinity=0;
					affinity=1/( ((double)(netSize-1))*(m_ClusterDB.m_modules[mID].m_area + m_ClusterDB.m_modules[mid].m_area));

					if(affinityMap.find(mid)!=affinityMap.end()) //the module exists in other net
					{
						affinityMap[mid]+=affinity;						
					}
					else
					{
						affinityMap.insert(pair<int,double>(mid,affinity));
					}
				}
			}
		}

	}


	//--------------------------------------------------------------------------------------------
	// find the max affinity without break the constraint
	//--------------------------------------------------------------------------------------------
	map<int,double>::iterator it;
	int target=-1;
	double maxAffinity=-1;

	for(it=affinityMap.begin(); it!=affinityMap.end(); ++it)
	{
		if(it->second>maxAffinity)
		{
			double combineArea=m_ClusterDB.m_modules[mID].m_area + m_ClusterDB.m_modules[it->first].m_area;
			if(combineArea<= maxClusterArea)
			{
				target=it->first;
				maxAffinity=it->second;
			}
		}

	}

	return target; //if target==-1, means all neighbor cells are larger than maxarea. Don't perform clustering on this cell
}
int CClusterDBFC::chooseMaxAffinity(int mID, double maxClusterArea, double& maxAff)
{
	map<int,double> affinityMap;

	//for all net belonging to the module, find its affinity
	set<int>::iterator netIter;
	for(netIter=m_ClusterDB.m_modules[mID].m_netIDs.begin(); netIter!=m_ClusterDB.m_modules[mID].m_netIDs.end(); ++netIter)
	{
//		int netid=*netIter;
		int netSize=(int)m_ClusterDB.m_nets[*netIter].size();
		if(netSize>1)
		{
			set<int>::iterator mIter; //module iterator
			for(mIter=m_ClusterDB.m_nets[*netIter].begin(); mIter!=m_ClusterDB.m_nets[*netIter].end(); ++mIter)
			{
				if(m_ClusterDB.isModuleFix(*mIter)==false && *mIter!=mID)
				{
					int mid=*mIter;
					//----------------
					// calc affinity
					//---------------
					double affinity=0;
					affinity=1/( ((double)(netSize-1))*(m_ClusterDB.m_modules[mID].m_area + m_ClusterDB.m_modules[mid].m_area));

					if(affinityMap.find(mid)!=affinityMap.end()) //the module exists in other net
					{
						affinityMap[mid]+=affinity;						
					}
					else
					{
						affinityMap.insert(pair<int,double>(mid,affinity));
					}
				}
			}
		}

	}


	//--------------------------------------------------------------------------------------------
	// find the max affinity without break the constraint
	//--------------------------------------------------------------------------------------------
	map<int,double>::iterator it;
	int target=-1;
	double maxAffinity=-1;

	for(it=affinityMap.begin(); it!=affinityMap.end(); ++it)
	{
		if(it->second>maxAffinity)
		{
			double combineArea=m_ClusterDB.m_modules[mID].m_area + m_ClusterDB.m_modules[it->first].m_area;
			if(combineArea<= maxClusterArea)
			{
				target=it->first;
				maxAffinity=it->second;
			}
		}

	}

	maxAff=maxAffinity;
	return target; //if target==-1, means all neighbor cells are larger than maxarea. Don't perform clustering on this cell
}
int CClusterDBFC::chooseMaxConnectivity(const set<int>& cluster, const set<int>& neighbor,const double& cSize,  map<int,int>& cNetSize,const double& maxClusterArea)
{

	//set<int> netSet;
	//for(it=cluster.begin(); it!=cluster.end(); ++it)
	//{
	//	netSet.insert(m_ClusterDB.m_modules[*it].m_netIDs.begin(),m_ClusterDB.m_modules[*it].m_netIDs.end());
	//}

	set<int>::iterator it;
	double maxConnectivity=-1;
	int target=-1;
	for(it=neighbor.begin(); it!=neighbor.end(); ++it)
	{
		if( (cSize+m_ClusterDB.m_modules[*it].m_area)<=maxClusterArea)
		{
			double con=0;
			set<int>::iterator netIt;
			//for all nets
			for(netIt=m_ClusterDB.m_modules[*it].m_netIDs.begin(); netIt!=m_ClusterDB.m_modules[*it].m_netIDs.end(); ++netIt)
			{
				if(cNetSize.find(*it)!=cNetSize.end())
				{
					assert(cNetSize[*it]>1);
					con+=1/((double)(cNetSize[*it]-1)*m_ClusterDB.m_modules[*it].m_area);
				}
				//set<int>::iterator cIt;
				////for all clusters
				//for(cIt=cluster.begin(); cIt!=cluster.end(); ++cIt)
				//{
				//	//check if common net exists
				//	if(m_ClusterDB.m_modules[*cIt].m_netIDs.find(*netIt)!=m_ClusterDB.m_modules[*cIt].m_netIDs.end())
				//	{
				//		con+=1/(double)(m_ClusterDB.m_nets[*netIt].size()-1);
				//	}
				//}
			}
			if(con>maxConnectivity)
			{
				maxConnectivity=con;
				target=*it;
			}
		}
	}
	
	return target; //if target==-1, means all neighbor cells are larger than maxarea. Don't perform clustering on this cell
}
int CClusterDBFC::chooseMaxContraction(int mID, double maxClusterArea,const vector< set<int> >& oldNet)
{
//	map<int,double> affinityMap;
	set<int> neighborSet;

	//for all net belonging to the module, find its affinity
	set<int>::iterator netIter;
	for(netIter=m_ClusterDB.m_modules[mID].m_netIDs.begin(); netIter!=m_ClusterDB.m_modules[mID].m_netIDs.end(); ++netIter)
	{
//		int netid=*netIter;
		int netSize=(int)m_ClusterDB.m_nets[*netIter].size();
		if(netSize>1)
		{
			set<int>::iterator mIter; //module iterator
			for(mIter=m_ClusterDB.m_nets[*netIter].begin(); mIter!=m_ClusterDB.m_nets[*netIter].end(); ++mIter)
			{
				
				if(m_ClusterDB.isModuleFix(*mIter)==false && *mIter!=mID && m_ClusterDB.m_modules[*mIter].m_hierarchy.size()==1)
				{
					neighborSet.insert(*mIter);
				}
			}
		}

	}


	//--------------------------------------------------------------------------------------------
	// find the max affinity without break the constraint
	//--------------------------------------------------------------------------------------------
	set<int>::iterator it;
	int target=-1;
	double minLC=-1;
	double _ef,_if;
	for(it=neighborSet.begin(); it!=neighborSet.end(); ++it)
	{

		double combineArea=m_ClusterDB.m_modules[mID].m_area + m_ClusterDB.m_modules[*it].m_area;
		if(combineArea<= maxClusterArea)
		{
			
			logicContraction(mID, *it, oldNet, _ef, _if);

			if(_if>0)
			{
				double LC=_ef/_if;
				if(target==-1)
				{
					target=*it;
					minLC=LC;
				}
				if(LC<minLC)
				{
					target=*it;
					minLC=LC;
				}
			}
		}
		

	}
	//if(target!=-1)
	//	printf(" CF(%d)=%.3f",mID,minLC);

	return target; //if target==-1, means all neighbor cells are larger than maxarea. Don't perform clustering on this cell
}
