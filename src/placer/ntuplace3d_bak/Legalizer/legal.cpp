#include <map>
#include <cassert>
#include <cmath>
using namespace std;

#include "legal.h"
#include "placedb.h"


void CRow::showspace()
{
	cout<<"\n=====ROW SPACE ===\n";

	map<float, float>::iterator iter;
	for(iter=m_empties.begin();iter!=m_empties.end();iter++)
	{
		cout<<" ["<<iter->first<<","<<iter->second<<"] ";
	}
	cout<<'\n';

}
void CRow::showmodule()
{
	cout<<"\n=====ROW Module ===\n";

	map<float,int>::iterator iter;
	for(iter=m_rowmodule.begin();iter!=m_rowmodule.end();iter++)
	{
		cout<<" ["<<iter->first<<","<<iter->second<<"] ";
	}
	cout<<'\n';

}
bool CRow::check_overlap(float x,float w)
{
//	cout<<"\n====overlap\n"<<x<<" "<<w<<" \n";
	map<float,float>::iterator iter;
	iter=m_empties.upper_bound(x);
	if(iter==m_empties.begin())
	{
		return true;             //x<all empty site's start point
	}
	else
	{
		--iter;
		if( (iter->second-(x-iter->first))< w )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}

int CRow::clean_empties()
{
	int count=0;
	map<float,float>::iterator iter;
	vector<float> zeroset;
	for(iter=m_empties.begin();iter!=m_empties.end();iter++)
	{
		if(iter->second==0)
		{
			zeroset.push_back(iter->first);
			count++;
		}
	}
	for(unsigned int i=0;i<zeroset.size();i++)
	{
		m_empties.erase(zeroset[i]);
	}
	return count;
}

bool CRow::add_empty(float x,float w)
{
	bool front; //true:nonempty in front of the space
	bool end;  //true:nonempty after the space
	map<float,float>::iterator iter;
	iter=m_empties.lower_bound(x);
	if(iter==m_empties.begin())
	{
		front=true;
	}
	else
	{
		--iter;
		if( (iter->first+iter->second)==x)
		{
			front=false;
		}
		else
			front=true;
	}
	iter=m_empties.lower_bound(x);

	if(iter==m_empties.end())
	{
		end=true;
	}
	else
	{
		if(iter->first==(x+w))
			end=false;
		else
			end=true;
	}

	if( (front==true) && (end==true) )
	{
		m_empties[x]=w;
	}
	else if( (front==true) && (end==false) )
	{		
		iter=m_empties.lower_bound(x);
		float l=iter->second;

		m_empties[x]=w+l;
		m_empties.erase(iter->first);
		
	}
	else if( (front==false) && (end==true) )
	{
		iter=m_empties.lower_bound(x);
		--iter;
		m_empties[iter->first]=iter->second+w;
	}
	else  // front==false && end == false
	{
		iter=m_empties.lower_bound(x);
		float tail_x=iter->first;
		float tail_w=iter->second;
		--iter;
		m_empties[iter->first]=iter->second+w+tail_w;
		m_empties.erase(tail_x);
	}
	return true;
}
int CRow::find_ncell(float start,float end)
{
	map<float,int>::iterator iter;
	int count=0;
	for(iter=m_rowmodule.lower_bound(start);iter!=m_rowmodule.lower_bound(end);iter++)
	{
		count++;
	}
	return count;
}

bool CRow::insert_module(float x , float w,int mID)
{

	map<float,float>::iterator iter;
	iter=m_empties.upper_bound(x);
	if(iter==m_empties.begin())
	{
		return false;             //x<all empty site's start point
	}
	else
	{
		--iter;
		if( (iter->second-(x-iter->first))< w )
		{
			return false;
		}
		else
		{
			float l2=x-iter->first;
//			cout<<"l2:"<<l2;
			if(l2==0)
			{
				if( ((iter->first+iter->second)-(x+w) )!=0)
				{
					m_empties[x+w]=(iter->first+iter->second)-(x+w);
				}
				m_empties.erase(iter->first);
			}
			else
			{
				
				if((x+w)< (iter->first+iter->second))
				{
//					cout<<"\n--x+w :\n"<<x+w<<" "<<(iter->first+iter->second)-(x+w)<<" \n";
					m_empties[x+w]=(iter->first+iter->second)-(x+w);
				}
				iter->second=l2;
			}
			m_rowmodule[x]=mID;
			return true;
		}
	}

}


////////////////////////////////////////////////////////////////////////////////////////


float CLegalizer::find_place(int mID ,float start,float end,int rowID,float& record)
{


	map<float,float> all=core_rows[rowID].m_empties;

	int height=1;
	if(fplan->m_modules[mID].m_height>fplan->m_rowHeight)
	{
		height = int(fplan->m_modules[mID].m_height/fplan->m_rowHeight);
		if(fplan->m_rowHeight*height<fplan->m_modules[mID].m_height)
			height++;

		if( (rowID+height-1)>=num_rows )
		{

			return start-1;
		}

		for(int i=1;i<height;i++)
		{
			all=mergerow(all,core_rows[rowID+i].m_empties);

		}
	}

	float x=fplan->m_modules[mID].m_x;
	float w=fplan->m_modules[mID].m_width;
	float distance=0;
	float final=0;
	bool find=false;
	bool dstart=false;
	int failcount=0;

	bool iflag=false;
	float istart;

	map<float,float>::iterator iter;

	iter=all.lower_bound(start);

	//break long empty space
	if(iter!=all.begin()&&start!=end)
	{
		if(iter->first>x)
		{
			--iter;
			if( (iter->first+iter->second)>start )
			{
				//cout<<"\n x:"<<x<<" w:"<<w<<" iter->f:"<<iter->first<<" iter->s:"<<iter->second<<" start"<<start<<" end:"<<end<<" mid:"<<mID;
				float ifir=iter->first;
				float isec=iter->second;
		
				//core_rows[rowID].showspace();

				iflag=true;

				all[ifir]=start-ifir;
				istart=ifir;
				//cout<<" \n is:"<<istart<<" if:"<<ifir;
				all[start]=ifir+isec-start;
				//core_rows[rowID].showspace();		
				//cout<<"\n -------";
			}
		}
	}



	for(iter=all.lower_bound(start);iter!=all.lower_bound(end);iter++)
	{

		if(iter->second>=w)
		{

			find=true;
			if((iter->first<=x)&&((iter->first+iter->second)>(x+w)))  //place at original location
			{
				distance=0;
				return x;
			}
			else
			{
				float nx;
				if(iter->first<x)
				{
					nx=iter->first+iter->second-w;
				}
				else
				{
					nx=iter->first;
				}
				float nhpwl=0;
				float oldhpwl=0;
				for(unsigned int k=0;k<fplan->m_modules[mID].m_netsId.size();k++)
				{
					oldhpwl=oldhpwl+fplan->GetNetLength(fplan->m_modules[mID].m_netsId[k]);
				}
				fplan->SetModuleLocation(mID,nx,fplan->m_modules[mID].m_y);

				//for(unsigned int h=0;h<moduleinfo[mID ].nets.size();h++)
				//{					
				//	float old=netinfo[moduleinfo[mID].nets[h]].hpwl();
				//	netinfo[moduleinfo[mID].nets[h]].calc(moduleinfo[mID].nets[h],*fplan);
				//	newhpwl=newhpwl+netinfo[moduleinfo[mID].nets[h]].hpwl()-old;
				//}
				for(unsigned int k=0;k<fplan->m_modules[mID].m_netsId.size();k++)
				{
					nhpwl=nhpwl+fplan->GetNetLength(fplan->m_modules[mID].m_netsId[k]);
				}

				fplan->SetModuleLocation(mID,x,fplan->m_modules[mID].m_y);
				//for(unsigned int h=0;h<moduleinfo[mID ].nets.size();h++)
				//{					
				//	netinfo[moduleinfo[mID].nets[h]].calc(moduleinfo[mID].nets[h],*fplan);				
				//}
				float newhpwl=nhpwl-oldhpwl;
				if(dstart==false)
				{
					distance=newhpwl;
					final=nx;
					dstart=true;
				}
				else if( newhpwl<distance)
				{
					distance=newhpwl;
					final=nx;
				}
				else
					failcount++;


			}
		}
	}


	if(find==true)
	{
		record=distance;
		movetotal=movetotal+distance;
		return final;
	}
	else
		return start-1;  //impossible place

}

bool CLegalizer::move_module(float x,int mid ,int rowID)//new x position,module ID,CPlaceDB
{
	core_rows[rowID].add_empty(fplan->m_modules[mid].m_x,fplan->m_modules[mid].m_width);
	core_rows[rowID].m_rowmodule.erase(fplan->m_modules[mid].m_x);
	fplan->SetModuleLocation(mid,x,fplan->m_modules[mid].m_y);

	if(!core_rows[rowID].insert_module(x,fplan->m_modules[mid].m_width,mid))
	{
		cout<<"\nmove_module FAIL!! x="<<x<<" mid="<<mid<<" width="<<fplan->m_modules[mid].m_width;
		core_rows[rowID].showspace();
	}

	return true;
}

//float CLegalizer::move_module2(float x,float y,int mid)//new x position,module ID,CPlaceDB
//{
//
//	float modify=0;
//	fplan->SetModuleLocation(mid,x,y);
//	for(int i=0;i<(int)moduleinfo[mid].nets.size();i++)
//	{
//		float old=netinfo[moduleinfo[mid].nets[i]].oldhpwl;
//		modify=modify+netinfo[moduleinfo[mid].nets[i]].calc3(moduleinfo[mid].nets[i],*fplan,moduleinfo,mid)-old;
//	}
//
//	return modify;
//}

void CLegalizer::showmod(int mID)
{
	//cout<<"\n mid:"<<mID<<" x:"<<fplan->m_modules[mID].m_x<<" y:"<<fplan->m_modules[mID].m_y<<" w:"<<fplan->m_modules[mID].m_width<<" find:"<<moduleinfo[mID].find;
}

void CLegalizer::PreLegal( float factor )
{
    fplan->MoveBlockToLeft( factor );
    //fplan->MoveBlockToBottom( factor );
}

void CLegalizer::Init()
{

    movetotal = 0;
    ROWHEIGHT = fplan->m_rowHeight;
    num_rows = (int)fplan->m_sites.size();
    //cout<<"\n Rows:"<<num_rows<<'\n';
    float row_length = fplan->m_coreRgn.right-fplan->m_coreRgn.left;

    // initailize CRow
    core_rows.clear();
	core_rows.reserve(num_rows);
    for(int i=0;i<num_rows;i++)
    {
	//CRow row(fplan->m_coreRgn.left,fplan->m_sites[i].m_bottom,row_length);
		core_rows.push_back( CRow(fplan->m_coreRgn.left,fplan->m_sites[i].m_bottom,row_length)  );
    }
    core_rows.resize( core_rows.size() );   // 2005/3/19 (donnie)
    botto = core_rows[0].m_y;

    // build row empty map
    for(unsigned int i=0;i<fplan->m_sites.size();i++)
    {
		int pos=(int)( (fplan->m_sites[i].m_bottom-fplan->m_coreRgn.bottom)/ROWHEIGHT);
		for(unsigned int j=0;j<fplan->m_sites[i].m_interval.size();j=j+2)
		{
			core_rows[pos].m_empties[fplan->m_sites[i].m_interval[j]]=fplan->m_sites[i].m_interval[j+1]-fplan->m_sites[i].m_interval[j];

		}			
    }

    //initialize netinfo

    //netinfo.clear();
    //CNetinfo cni;
    //for(unsigned int i=0; i<fplan->m_nets.size();i++ )
    //      {
    //          netinfo.push_back( cni );
    //      }
    //      netinfo.resize( netinfo.size() );   // 2005/3/19 (donnie)
    //netinfo.resize( fplan->m_nets.size() ); // 2005/3/19 (donnie)

    //for(unsigned int i=0; i<fplan->m_nets.size();i++ )
    //{
    //	netinfo[i].calc( i, *fplan );
    //}


    //initialize moduleinfo
    //moduleinfo.clear();
    //  	CModule_Info cmi;
    //for( unsigned int i=0; i<(int)fplan->m_modules.size(); i++ )
    //      {
    //	moduleinfo.push_back( cmi );
    //      }
    //      moduleinfo.resize( moduleinfo.size() ); // 2005/3/19 (donnie)
    //moduleinfo.resize( fplan->m_modules.size() );   // 2005/3/19 (donnie)

    //for( unsigned int i=0; i<netinfo.size(); i++ )
    //{			
    //	if(netinfo[i].left_d>moduleinfo[netinfo[i].left].left)
    //	{
    //		moduleinfo[netinfo[i].left].left=netinfo[i].left_d;
    //		moduleinfo[netinfo[i].left].left_net=i;
    //	}
    //	if(netinfo[i].right_d>moduleinfo[netinfo[i].right].right)
    //	{
    //		moduleinfo[netinfo[i].right].right=netinfo[i].right_d;
    //		moduleinfo[netinfo[i].right].right_net=i;
    //	}

    //}

    //for(unsigned int i=0; i<fplan->m_nets.size();i++ )
    //{
    //	for(unsigned int j=0;j<fplan->m_nets[i].size();j++)
    //	{
    //		moduleinfo[fplan->m_pins[fplan->m_nets[i][j]].moduleId].nets.push_back(i);
    //		//moduleinfo[fplan->m_pins[fplan->m_nets[i][j]].moduleId].netmap.insert(pair<float,int>(netinfo[i].left_c,1));
    //		//moduleinfo[fplan->m_pins[fplan->m_nets[i][j]].moduleId].netmap.insert(pair<float,int>(netinfo[i].right_c,2));
    //	}

    //}

    // (donnie)
    //for( int i=0; i<(int)fplan->m_modules.size(); i++ )
    //{
    //    moduleinfo[i].nets = fplan->m_modules[i].m_netsId;
    //}

}

bool CLegalizer::legalize(int WINDOW, SCALE_TYPE type)
{
    //int cl=0;
    vector<float> cost;
    cost.resize(fplan->m_modules.size());

    //cout << (unsigned int)fplan->m_modules.size() << " blocks\n";
    //for(unsigned  int i=0; i<(int)fplan->m_modules.size(); i++ )
    //{
    //	float mx=fplan->m_modules[i].m_x;
    //}

    //=====================================
    // Create legalization sequence
    //=====================================
    for( int i=0; i<(int)fplan->m_modules.size(); i++ )
    {
	float nx = fplan->m_modules[i].m_x;
	float ny = fplan->m_modules[i].m_y;
	float nw = fplan->m_modules[i].m_width;
	//float ne = moduleinfo[i].edgecount;
	//float nf = moduleinfo[i].nets.size();

	//float EDGE=0;
	float WIDTH=0;
	//float FAN=0;
	float X=-1;
	float Y=0;
	float NH = fplan->m_coreRgn.right;

	if( type == SCALE_TO_LEFT || type == SCALE_TO_LEFT_BETWEEN_MACRO )
	{
	    nx = nx;
	}
	else if( type == SCALE_TO_RIGHT || type == SCALE_TO_RIGHT_BETWEEN_MACRO )
	{
	    //nx = NH - nx;
	    //assert( nx > 0 );
	    X = 1;
	    WIDTH = 1;
	}
	else if( type == SCALE_TO_MIDLINE || type == SCALE_TO_MIDLINE_BETWEEN_MACRO )
	{
	    float midline_x = ( fplan->m_coreRgn.right + fplan->m_coreRgn.left ) / 2.0;

	    nx += nw * 0.5; 
	    nx = abs( nx - midline_x );
	}
	else
	{
	    cerr << "Warning: Unknown Scale Type\n";				
	}


	if( fplan->m_modules[i].m_height > 5 * fplan->m_rowHeight )
	    cost[i] = X*nx+WIDTH*nw+Y*ny + NH*fplan->m_modules[i].m_height;
	else
	    cost[i] = X*nx + WIDTH*nw +  Y*ny;

	//// TEST by donnie 2006-0316
	//cost[i] = fplan->m_modules[i].m_height * fplan->m_modules[i].m_width;
	
    }

    
	//===================================================
	// Put blocks into rows (affect y-coordinate only)
	//===================================================
	for(unsigned  int i=0; i<fplan->m_modules.size(); i++ )
	{

	    if((fplan->m_modules[i].m_isFixed==false))
	    {
		int y_pos=(int)((fplan->m_modules[i].m_y-botto)/ROWHEIGHT);
		//		cout<<" "<<i;

		float reminder=fplan->m_modules[i].m_y-botto-(float)y_pos*ROWHEIGHT;
		if((reminder>(ROWHEIGHT/2))&& (y_pos+1)<num_rows)
		{
		    y_pos++;
		}
		if(y_pos<0)
		    y_pos=0;

		fplan->SetModuleLocation(i,(int)fplan->m_modules[i].m_x,y_pos*ROWHEIGHT+botto);

	    }
	}

    int fool=0;
    int file=0;



    int cou=0;
    multimap<float,int>::reverse_iterator riter;
    int climb=0;
    int noclimb=0;
    int directput=0;
    //int pushcount=0;
    int parr[41];
    for(int i=0;i<41;i++)
    {
	parr[i]=0;
    }

    //for(int i=0;i<(int)fplan->m_nets.size();i++ )
    //{
    //	netinfo[i].calc(i,*fplan);
    //	for(int j=0;j<(int)netinfo[i].tops.size();j++)
    //	{
    //		moduleinfo[netinfo[i].tops[j]].edgecount++;
    //	}

    //}

    //marked 0319
    //float x_max,x_min,e_max,e_min,w_max,w_min,fan_max,fan_min;
    //bool cflag=false;
    //for( int i=0; i<(int)fplan->m_modules.size(); i++ )
    //{

    //	if(fplan->m_modules[i].m_isFixed==false)
    //	{
    //		if(cflag==false)
    //		{
    //			x_max=fplan->m_modules[i].m_x;
    //			x_min=fplan->m_modules[i].m_x;
    //			w_max=fplan->m_modules[i].m_width;
    //			w_min=fplan->m_modules[i].m_width;
    //			//e_max=moduleinfo[i].edgecount;
    //			//e_min=moduleinfo[i].edgecount;
    //			//fan_max=moduleinfo[i].nets.size();
    //			//fan_min=moduleinfo[i].nets.size();
    //			cflag=true;
    //		}
    //		else
    //		{
    //			if(x_max<fplan->m_modules[i].m_x)
    //				x_max=fplan->m_modules[i].m_x;
    //			else if(x_min>fplan->m_modules[i].m_x)
    //				x_min=fplan->m_modules[i].m_x;

    //			if(w_max<fplan->m_modules[i].m_width)
    //				w_max=fplan->m_modules[i].m_width;
    //			else if(w_min>fplan->m_modules[i].m_width)
    //				w_min=fplan->m_modules[i].m_width;

    //			if(fan_max<moduleinfo[i].nets.size())
    //				fan_max=moduleinfo[i].nets.size();
    //			else if(fan_min>moduleinfo[i].nets.size())
    //				fan_min=moduleinfo[i].nets.size();

    //			if(e_max<moduleinfo[i].edgecount)
    //				e_max=moduleinfo[i].edgecount;
    //			else if(e_min>moduleinfo[i].edgecount)
    //				e_min=moduleinfo[i].edgecount;

    //		}
    //	}
    //	
    //}
    //end of 0319

    //    //=====================================
    //    // Create legalization sequence
    //    //=====================================
    //	for( int i=0; i<(int)fplan->m_modules.size(); i++ )
    //	{
    //
    //		//float nx=(fplan->m_modules[i].m_x-x_min)/(x_max-x_min);
    //		//float nw=(fplan->m_modules[i].m_width-w_min)/(w_max-w_min);
    //		//float ne=(moduleinfo[i].edgecount-e_min)/(e_max-e_min);
    //		//float nf=(moduleinfo[i].nets.size()-fan_min)/(fan_max-fan_min);
    //
    //	//	cout<<"\n "<<nx<<" "<<nw<<" "<<ne<<" "<<nf;
    //		//float xhelf=(x_max-x_min)/2;
    //		float nx=fplan->m_modules[i].m_x;
    //		//if(fplan->m_modules[i].m_x>=xhelf)
    //		//{
    //		//	nx=x_max-fplan->m_modules[i].m_x;
    //		//}
    //		//else
    //		//{
    //		//	nx=fplan->m_modules[i].m_x-x_min;
    //		//}
    //
    //		float nw=fplan->m_modules[i].m_width;
    //		float ne=moduleinfo[i].edgecount;
    //		float nf=moduleinfo[i].nets.size();
    //		float ny=fplan->m_modules[i].m_y;
    //
    //		//float EDGE=1;
    //		//float WIDTH=1;
    //		//float FAN=1;
    //		//float X=2;
    //		float EDGE=0;
    //		float WIDTH=0;
    //		float FAN=0;
    //		float X=-1;
    //		float Y=0;
    //		float NH=fplan->m_coreRgn.right;
    ////		float NH=0;
    //
    //
    //		//moduleinfo[i].cost=X*nx+WIDTH*nw+FAN*nf+EDGE*ne+Y*ny+NH*fplan->m_modules[i].m_height;
    //
    //        if( fplan->m_modules[i].m_height > 4 * fplan->m_rowHeight )
    //            moduleinfo[i].cost=X*nx+WIDTH*nw+FAN*nf+EDGE*ne+Y*ny+NH*fplan->m_modules[i].m_height;
    //        else
    //            moduleinfo[i].cost=X*nx+WIDTH*nw+FAN*nf+EDGE*ne+Y*ny;
    //
    //	}
    //	
    cout<<" \n progress: ";
    multimap<float,int> moduleinwidth;
    for( int i=0; i<(int)fplan->m_modules.size(); i++ )
    {
	moduleinwidth.insert(pair<float,int>(cost[i],i));
    }


    //=======================================
    int step = (int)fplan->m_modules.size()/30;
    int counter = 0;
    if( step == 0 )
	step = 1;
    //=======================================

    int progress = 0;
    
    //int countcc=0;
    for(riter=moduleinwidth.rbegin();riter!=moduleinwidth.rend();riter++)
    {

	//flush( cout );
	if( counter % step == 0 )
	    //if( counter % 50 == 0 )
	{
	    //cout << "*";
	    cout << "[" << progress << "] ";
	    progress++;
	    flush( cout );
	}
	counter++;

	int i=riter->second;

	if((fplan->m_modules[i].m_isFixed==false))
	{
	    //	cout<<" i:"<<i<<" height:"<<fplan->m_modules[i].m_height<<" x"<<fplan->m_modules[i].m_x<<" w:"<<fplan->m_modules[i].m_width<<" n:"<<fplan->m_modules[i].m_name;
	    //	cout<<" c:"<<countcc;


	    int row_pos=(int)( (fplan->m_modules[i].m_y-botto)/ROWHEIGHT );

	    //if no overlap ---> direct place the cell
	    if( (fplan->m_modules[i].m_height == fplan->m_rowHeight) && 
		    (core_rows[row_pos].check_overlap(fplan->m_modules[i].m_x,fplan->m_modules[i].m_width)==false)  )
	    {
		core_rows[row_pos].insert_module(fplan->m_modules[i].m_x,fplan->m_modules[i].m_width,i);
		//	cout<<"dir:"<<i<<"\n";
		directput++;
	    }
	    else
	    {
		//		cout<<"PASS 1!"<<i<<" =====\n";
		//int WINDOW=200;
		if(WINDOW<=0)
		{
		    WINDOW=200;
		}
		bool find=false;
		int fcount=0;
		float record=0;
		float low=0;
		float nx=0;
		float nr=0;
		int first=0;
		int p=0;
		int precord=0;



		for(p=1;p<WINDOW;p++)
		{					
		    for(int j=0;j<2*p+1;j++)
		    {
			int g=(j%2)?1:-1;
			int x=(int)(j+1)/2;

			int row=g*x;
			//int row = 0;  // 1D-search

			float start=fplan->m_modules[i].m_x-(  (p-x)*(ROWHEIGHT) );
			//						float end=fplan->m_modules[i].m_x+fplan->m_modules[i].m_width+( (p-x)*(ROWHEIGHT) );
			float end=fplan->m_modules[i].m_x+fplan->m_modules[i].m_width+( (p-x)*(ROWHEIGHT) );
			if( ( (row_pos+row)>=0 )&& ( (row_pos+row)<(int)core_rows.size() )&&(start!=end))
			{
			    float new_pos=find_place(i,start,end,row_pos+row,record);
			    if(new_pos!=(start-1)  )
			    {
				if(fcount==0)
				{
				    first=2;
				    low=record;
				    nx=new_pos;
				    nr=row_pos+row;
				    precord=p;
				}
				else
				    if(record<low)
				    {
					first=1;
					low=record;
					nx=new_pos;
					nr=row_pos+row;
					climb++;
					precord=p;
				    }
				fcount++;
				if( fcount>2  || p>=25 )
				{		
				    find=true;
				    break;
				}				
			    }
			}

		    }
		    if(find==true)
		    {
			if(first==2)
			    noclimb++;
			fplan->SetModuleLocation(i,nx,(nr)*ROWHEIGHT+botto);
			//for(unsigned int h=0;h<moduleinfo[i].nets.size();h++)
			//{
			//	netinfo[moduleinfo[i].nets[h]].calc(moduleinfo[i].nets[h],*fplan);
			//	
			//}
			int height=1;
			if(fplan->m_modules[i].m_height>fplan->m_rowHeight)
			{
			    height=int(fplan->m_modules[i].m_height/fplan->m_rowHeight);
			    if(fplan->m_rowHeight*height<fplan->m_modules[i].m_height)
				height++;

			}
			for( int q=0; q<height; q++ )
			{
			    int rowId = (int)(nr+q);
			    if(!core_rows[rowId].insert_module(nx,fplan->m_modules[i].m_width,i))
			    {
				cout << "\ninsert" << nr+q << " x:" << nx << " w:" 
				     << fplan->m_modules[i].m_width << " mod:" << i 
				     << " name:" << fplan->m_modules[i].m_name << "\n";

				core_rows[rowId].showspace();

				file++;
			    }
			}
			if((p-1)>=40)
			    parr[40]++;
			else
			    parr[precord-1]++;
			break;
		    }					
		}
		if(find==false)
		{
		    fool++;
		}
	    }

	    /*				int he=(fplan->m_modules[i].m_height/fplan->m_rowHeight);
					if(he>1000)
					{
					char filename[200];
					sprintf( filename, "fig_level_%d_%d.plt", cl ,i);
					fplan->OutputGnuplotFigure( filename );
					fplan->SaveBlockLocation();
					cout<<fplan->m_modules[i].m_name;
					cl++;
					}*/			

	    //if(row_pos==0)
	    //{
	    //	if(core_rows[row_pos].show_fail()!=0)
	    //		cout<<"\n on insert cell:"<<i<<'\n';
	    //}
	}
	cou++;
    }
    //	cout<<"\n---Modules can't be legalize:\n"<<fool<<"\n climb times:"<<climb<<" no climb:"<<noclimb<<
    //		" directput:"<<directput<<" push_count:"<<pushcount<<'\n';
    cout<<"\n---Modules can't be legalize: "<< fool<<"\n";
    //	cout<<"\n---FILE--\n"<<file<<"\n---FILE--\n";


    //	fplan->OutputPL( "fig_legal_ada3.pl" );
    //fplan->CalcHPWL();
    //cout << "Legal HPWL= " << fplan->GetHPWL() << "\n\n";
    //cout<<"movetotal:"<<movetotal<<"\n";


#if 0
    // Output statistic
    cout<<"parr:\n";
    for(int i=0;i<41;i++)
    {
	if(i%8==0)
	    cout<<"\n";
	cout<<" p["<<i<<"]="<<parr[i]<<" ";
    }
#endif 

    if(fool==0)
	return true;    // Success
    else
	return false;   // Fail

}

//
//void permutation::action()
//{
//	
//	//cout<<"\n";
//	//acount++;
//	//cout<<"count:"<<acount<<"  ";
//	//for(int i=0;i<num;i++)
//	//{
//	//	cout<<" "<<obj[vec[i]]<<" ";
//	//}
//	float sx=start_x;  //store start point of new module position
//
//	//move modules to new permutation position
//	//cout<<" \n action:";
//	for(int i=0;i<num;i++)
//	{
//		//cout<<" "<<modules[vec[i]]<<" sx:"<<sx;
//		cl->fplan->SetModuleLocation(modules[vec[i]],sx,cl->fplan->m_modules[modules[vec[i]]].m_y);
//		sx=sx+cl->fplan->m_modules[modules[vec[i]]].m_width;
//
//	}
//
//	//calc new hpwl
//	float newhpwl=0;
//	set<int>::iterator it;
//	for(it=netset.begin();it!=netset.end();++it)
//	{
//		cl->netinfo[*it].calc(*it,*cl->fplan);
//		newhpwl=newhpwl+cl->netinfo[*it].hpwl();
//
//	}
//	//cout<<" n:"<<newhpwl<<" o:"<<hpwl<<"  ";
//	if(flag==false)
//	{
//		flag=true;
//		hpwl=newhpwl;
//		for(int i=0;i<num;i++)
//		{
//			best.push_back(modules[vec[i]]);
//		}
//	}
//	else if(newhpwl<hpwl)
//	{
//		hpwl=newhpwl;
//		for(int i=0;i<num;i++)
//		{
//			best[i]=modules[vec[i]];
//		}
//	}
//	//else   // store modules to best position, this code waste time, should be changed
//	//{
//	//	sx=start_x; 
//	//	for(int i=0;i<num;i++)
//	//	{
//	//		cl->fplan->SetModuleLocation(best[i],sx,cl->fplan->m_modules[best[i]].m_y);
//	//		sx=sx+cl->fplan->m_modules[best[i]].m_width;
//	//	}
//	//}
//
//	//cout<<" \n best:";
//	//for(int i=0;i<num;i++)
//	//{
//	//	cout<<" "<<best[i];
//	//}
//
//}
//
//// go to next state;
//bool permutation::next()
//{
////		action2();
//
//	if(level!=num-1)
//	{
//		if(left[nodeID]==false)
//		{
//			if(calc()==false)
//			{
//				int np=-1;
//				for(int i=nodeID+1;i<num;i++)
//				{
//					if(find_in(i)==false)
//					{
//						np=i;   //find other child node
//						break;
//					}
//				}
//				if(np!=-1)
//				{
//					root[np]=root[nodeID];
//					left[nodeID]=false;
//					nodeID=np;
//					vec[level]=nodeID;
//					return true;
//
//				}
//				else
//				{
//					left[nodeID]=false;
//					nodeID=root[nodeID];
//					
//					for(int i=level;i<num;i++)
//					{
//						vec[i]=-1;
//					}
//
//
//					level--;
//					if(level>=0)
//					{
//						vec[level]=nodeID;
//						return true;
//					}
//					else
//						return false;
//
//				}
//			}
//			left[nodeID]=true;
//			level++;
//			int temp=nodeID;
//			nodeID=find();
//			root[nodeID]=temp;
//			vec[level]=nodeID;
//		}
//		else
//		{
//			int np=-1;
//			for(int i=nodeID+1;i<num;i++)
//			{
//				if(find_in(i)==false)
//				{
//					np=i;   //find other child node
//					break;
//				}
//			}
//			if(np!=-1)
//			{
//				root[np]=root[nodeID];
//				left[nodeID]=false;
//				nodeID=np;
//				vec[level]=nodeID;
//
//			}
//			else
//			{
//				left[nodeID]=false;
//				nodeID=root[nodeID];
//				
//				for(int i=level;i<num;i++)
//				{
//					vec[i]=-1;
//				}
//
//
//				level--;
//				if(level>=0)
//					vec[level]=nodeID;
//				else
//					return false;
//
//			}
//		}
//	}
//	else   // bottom node
//	{
//		acount++;
//		action();
//		nodeID=root[nodeID];
//		vec[level]=-1;
//		--level;
//		
//	}
//	//action2();
//	//cout<<"\n-  -  -  -  -  -  -\n";
//	if(level>=0)
//		return true;
//	else
//		return false;
//
//}
//// find smallest index that does not appear
//int permutation::find()
//{
//	if(vec[0]==-1)
//		return 0;
//
//	for(int i=0;i<num;i++)
//	{
//		for(int j=0;j<num;j++)
//		{
//			if(vec[j]==i)
//				break;
//			if(j==num-1)
//				return i;
//		}
//	}
//	return -1;
//}
//bool permutation::find_in(int pos)
//{
//	for(int i=0;i<num;i++)
//	{
//		if(vec[i]==pos)
//		{
//			return true;
//		}
//	}
//	return false;
//}
//bool permutation::calc()
//{
//
//
//	return true;
//
//	if((num<2)||(flag==false))
//		return true;
//	if(level>2)
//		return true;
//
//	
//
//	for(int i=0;i<=level;i++)
//	{
//		cl->moduleinfo[modules[vec[i]]].find=true;
//	}
//	float sx=start_x;  //store start point of new module position
//
//	//move modules to new permutation position
//	//cout<<" \n action:";
//	float chang=0;
//	for(int i=0;i<=level;i++)
//	{
//		//cout<<" "<<modules[vec[i]]<<" sx:"<<sx;
//		chang=chang+cl->move_module2(sx,cl->fplan->m_modules[modules[vec[i]]].m_y,modules[vec[i]]);
//		if(chang>0)
//			return false;
//		sx=sx+cl->fplan->m_modules[modules[vec[i]]].m_width;
//	}
//
//
//	float newhpwl=0;
//	set<int>::iterator it;
//	for(it=netset.begin();it!=netset.end();++it)
//	{
//	//	cl->netinfo[*it].calc2(*it,*cl->fplan,cl->moduleinfo);
//		newhpwl=newhpwl+cl->netinfo[*it].hpwl();
//
//	}
//
//
//	for(int i=0;i<=level;i++)
//	{
//		cl->moduleinfo[modules[vec[i]]].find=false;
//	}
//
//	//sx=start_x; //move module to best place 
//	//for(int i=0;i<=level;i++)
//	//{
//	//	cl->fplan->SetModuleLocation(best[i],sx,cl->fplan->m_modules[best[i]].m_y);
//	//	sx=sx+cl->fplan->m_modules[best[i]].m_width;
//	//}
//
//	//if(chang>hpwl-base_hpwl)
//	//	return false;
//
//	if(newhpwl>hpwl)
//	{
//		return false;
//	}
//	else
//		return true;
//}

//void CLegalizer::detail()
//{
//	int runcount2=0;
//	vector< vector<int> > chains;
//	int index=0;
////	int tem=0;
//	for(unsigned int i=0;i<core_rows.size();i++)
//	{
//		//if(index<1000)
//		//	cout<<tem<<" <-index:"<<index<<" ";
//		//tem=0;
//		vector<int> subchain;
//		chains.push_back(subchain);
//		index=(int)chains.size()-1;
//		map<float,int>::iterator iter;
//
//		
//		for(iter=core_rows[i].m_rowmodule.begin();iter!=core_rows[i].m_rowmodule.end();iter++)
//		{
//			float mx=iter->first;
//			float width=fplan->m_modules[iter->second].m_width;
//			chains[index].push_back(iter->second);
////			tem++;
//			++iter;
//			if(iter==core_rows[i].m_rowmodule.end())
//				break;
//			if(iter->first!=(mx+width))
//			{
//				//if(index<1000)
//				//	cout<<tem<<" <-index:"<<index<<" ";
//				//tem=0;
//				vector<int> temp;
//				chains.push_back(temp);
//				index++;
//			}
//			--iter;
//
//		}
//		
//	}
//
//	int WS1=6;
//	int WS2=3;
//
//	int WSIZE;     //window size
//	int OVERLAP;   //overlap
//
//	int detailchange=0;	
//	int runcount=0;
//	for(int i=0;i<(int)chains.size();i++ )
//	{
//		
//		//j is the end of window
//		if(chains[i].size()!=0)
//		{
//			if(chains[i].size()>5)
//				WSIZE=WS1;
//			else
//				WSIZE=(int)chains[i].size();
//			OVERLAP=WSIZE-1;
//			for(int j=(WSIZE-1) ; j<= (chains[i].size()-1) ;j=(j+(WSIZE-OVERLAP)))
//			{
//	//			cout<<" "<<j;
//				runcount++;
//				permutation per(WSIZE,*this);
//	//			cout<<" j:"<<j<<" cha.s:"<<chains[i].size()-1;
//
//				for(int k=j-WSIZE+1; k<=j; k++ )
//				{
//		//			cout<<" "<<k;
//					per.modules.push_back(chains[i][k]);
//				}
//	//			cout<<" \nac2";
//				per.init();
//	//			cout<<" \nac3";
//				bool b=per.next();
//	//			cout<<" \nac4";
//				while(b==true)
//				{
//					b=per.next();
//					runcount2++;
//					
//				}
//				float sx=per.start_x; 
//				for(int v=0;v<(int)per.best.size();v++)
//				{
//					fplan->SetModuleLocation(per.best[v],sx,fplan->m_modules[per.best[v]].m_y);
//					sx=sx+fplan->m_modules[per.best[v]].m_width;
//				}
//				
//				for(int m=0;m<(int)per.modules.size();m++)
//				{
//					moduleinfo[per.modules[m]].find=true;
//				}				
//
//
//
//				//put the best result back to chains[i] to perserve the order
//				bool changeflag=false;
//				int bestindex=0;
//	//			cout<<"\n";
//				if(per.flag==true)
//				{
//					for(int k=j-WSIZE+1; k<=j; k++ )
//					{
//		//				cout<<" new:"<<per.best[bestindex]<<" old:"<<chains[i][k];
//						if(per.best[bestindex]!=chains[i][k])
//							changeflag=true;
//						chains[i][k]=per.best[bestindex];
//						bestindex++;
//						
//					}
//					if(changeflag==true)
//						detailchange++;
//				}
//
//
//			}
//		}
//	}
//
//	for(int i=0;i<(int)core_rows.size();i++)
//	{
//		core_rows[i].m_rowmodule.erase(core_rows[i].m_rowmodule.begin(),core_rows[i].m_rowmodule.end());
//	}
//	cout<<"\n detailchange:"<<detailchange<<" runcount:"<<runcount<<"\n";
//	for(unsigned  int i=0; i<(int)fplan->m_modules.size(); i++ )
//	{
//
//		if(fplan->m_modules[i].m_isFixed==false)
//		{
//			int row_pos=(int)( (fplan->m_modules[i].m_y-botto)/ROWHEIGHT );
//			core_rows[row_pos].m_rowmodule[fplan->m_modules[i].m_x]=i;
//		}
//	}
//	cout<<"\n runcount:"<<runcount2<<"\n";
//	//int cc=0;
//	//for(int i=0;i<chains.size();i++ )
//	//{
//	//	int cou=0;
//	//	
//	//	for(int j=0;j<chains[i].size();j++)
//	//	{
//	//		cou++;
//	//		//cout<<" "<<chains[i][j]<<" ";
//	//	}
//	//	if(cou>5)
//	//	{
//	//		cc++;
//	//		cout<<"  chain:"<<i<<" ";
//	//		cout<<cou<<" ";
//	//	}
//	//}
//	//cout<<"  Xchain:"<<cc<<" ";
//
//}
void CNetinfo::add(float x,float y ,float w,float h,int module)
{
	if(left==-1)
	{
		left=module;
		left_c=x+(w/2);
		left_d=0;
		right=module;
		right_c=x+(w/2);
		right_d=0;

		top=module;
		top_c=y+(h/2);
		top_d=0;
		bottom=module;
		bottom_c=y+(h/2);
		bottom_d=0;

	}		
	if( (x+(w/2))<left_c)
	{
		left=module;
		left_d=left_c-(x+(w/2));
		left_c=x+(w/2);
		lefts.clear();
		lefts.push_back(module);

	}
	else if( (x+(w/2))==left_c)
	{
		left=module;
		left_d=left_c-(x+(w/2));
		lefts.push_back(module);
	}

	if( (x+(w/2))>right_c)
	{
		right=module;
		right_c= (x+(w/2));
		right_d= (x+(w/2))-right_c;
		rights.clear();
		rights.push_back(module);

	}
	else if( (x+(w/2))==right_c)
	{
		right=module;
		right_d= (x+(w/2))-right_c;
		rights.push_back(module);

	}

	if( (y+(h/2))<bottom_c)
	{
		bottom=module;
		bottom_d=bottom_c-(y+(h/2));
		bottom_c=(y+(h/2));
		bottoms.clear();
		bottoms.push_back(module);
	}
	else if( (y+(h/2))==bottom_c)
	{
		bottom=module;
		bottom_d=bottom_c-(y+(h/2));
		bottoms.push_back(module);
	}

	if((y+(h/2))>top_c)
	{
		top=module;
		top_d=(y+(h/2))-top_c;
		top_c=(y+(h/2));
		tops.clear();
		tops.push_back(module);
	}
	else if((y+(h/2))==top_c)
	{
		top=module;
		top_d=(y+(h/2))-top_c;
		tops.push_back(module);		
	}
	
}
void CNetinfo::calc2(int netID,CPlaceDB& cf,vector<CModule_Info>& mod)  //mod=CLegalizer.moduleinfo
{

	left=-1;
	right=-1;
	top=-1;
	bottom=-1;
	for(unsigned int i=0;i<cf.m_nets[netID].size();i++)
	{
		if(mod[cf.m_pins[cf.m_nets[netID][i]].moduleId].find==true)
		{
			add(cf.m_modules[cf.m_pins[cf.m_nets[netID][i]].moduleId].m_x,
				cf.m_modules[cf.m_pins[cf.m_nets[netID][i]].moduleId].m_y,
				cf.m_modules[cf.m_pins[cf.m_nets[netID][i]].moduleId].m_width,
				cf.m_modules[cf.m_pins[cf.m_nets[netID][i]].moduleId].m_height,
				cf.m_pins[cf.m_nets[netID][i]].moduleId);
		}
	}
	
	oldhpwl=hpwl();
	
}
float CNetinfo::calc3(int netID,CPlaceDB& cf,vector<CModule_Info>& mod,int last)  //mod=CLegalizer.moduleinfo
{

	if((last==left)||(last==right)||(last==top)||(last==bottom))
	{
		left=-1;
		right=-1;
		top=-1;
		bottom=-1;
		for(unsigned int i=0;i<cf.m_nets[netID].size();i++)
		{
			if(mod[cf.m_pins[cf.m_nets[netID][i]].moduleId].find==true)
			{
				add(cf.m_modules[cf.m_pins[cf.m_nets[netID][i]].moduleId].m_x,
					cf.m_modules[cf.m_pins[cf.m_nets[netID][i]].moduleId].m_y,
					cf.m_modules[cf.m_pins[cf.m_nets[netID][i]].moduleId].m_width,
					cf.m_modules[cf.m_pins[cf.m_nets[netID][i]].moduleId].m_height,
					cf.m_pins[cf.m_nets[netID][i]].moduleId);
			}
		}
	}
	
	oldhpwl=hpwl();
	return oldhpwl;
	
}

/*
bool CLegalizer::compare(int a,int b) // return true if a >= b
{
	if(moduleinfo[a].edgecount>moduleinfo[b].edgecount)
	{
		return true;
	}
	else if(moduleinfo[a].edgecount==moduleinfo[b].edgecount)
	{
		if(moduleinfo[a].nets.size()>moduleinfo[b].nets.size())
			return true;
		else if(moduleinfo[a].nets.size()==moduleinfo[b].nets.size())
		{
			if(fplan->m_modules[a].m_width>=fplan->m_modules[b].m_width)
				return true;
			else
				return false;
		}
		else
			return false;
	}
	else
		return false;
}
*/
map<float,float> CLegalizer::mergerow(map<float,float> a,map<float,float> b)
{
	map<float,float> result;
	multimap<float,bool> emp;  // x-coordinate, true for start, false for end;

	map<float,float>::iterator iter;
	for(iter=a.begin();iter!=a.end();iter++)
	{
		emp.insert(pair<float,bool>(iter->first,true));
		emp.insert(pair<float,bool>(iter->first+iter->second,false));
	}
	for(iter=b.begin();iter!=b.end();iter++)
	{
		emp.insert(pair<float,bool>(iter->first,true));
		emp.insert(pair<float,bool>(iter->first+iter->second,false));
	}
	int height=0;
	float start = 0;
	bool flag=false;
	multimap<float,bool>::iterator iter2;
	for(iter2=emp.begin();iter2!=emp.end();iter2++)
	{
		if(iter2->second==true)
			height++;
		if(iter2->second==false)
			height--;

		if(height==2&&flag==false)
		{
			start=iter2->first;
			flag=true;
		}
		if(height!=2&&flag==true)
		{
			result[start]=iter2->first-start;
			flag=false;
		}

	}
	vector<float> zeroset;
	for(iter=result.begin();iter!=result.end();iter++)
	{
		if(iter->second==0)
		{
			zeroset.push_back(iter->first);

		}
	}
	for(unsigned int i=0;i<zeroset.size();i++)
	{
		result.erase(zeroset[i]);
	}
	return result;

}
void CLegalizer::showspace(map<float,float> a)
{
	cout<<"\n=====ROW SPACE ===\n";

	map<float,float>::iterator iter;
	for(iter=a.begin();iter!=a.end();iter++)
	{
		cout<<" ["<<iter->first<<","<<iter->second<<"] ";
	}
	cout<<'\n';

}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Diamond Macro Legalizer =>legal macors which id exists in set<int> macroSet
// USAGE:
// CLegalizer check(placeDB);
// check.legalize( (int)(max( placeDB.GetHeight(), placeDB.GetWidth() )*1.4/placeDB.m_rowHeight), macroSet );
//
//
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

bool CLegalizer::macroLegal(int WINDOW,const set<int>& macroIDset)
{

    
	//===================================================
	// Put blocks into rows (affect y-coordinate only)
	//===================================================
	set<int>::iterator it;
// 	fplan->OutputGnuplotFigure( "DML_input.plt",false,true);
	for(it=macroIDset.begin(); it!=macroIDset.end(); ++it )
	{
		assert(fplan->m_modules[*it].m_isFixed==false);

// 		cout << " " << *it;
		int y_pos=(int)((fplan->m_modules[*it].m_y-botto)/ROWHEIGHT);
		//		cout<<" "<<i;

		float reminder=fplan->m_modules[*it].m_y-botto-(float)y_pos*ROWHEIGHT;
		if((reminder>(ROWHEIGHT/2))&& (y_pos+1)<num_rows)
		{
		    y_pos++;
		}
		if(y_pos<0)
		{
		    y_pos=0;
		}
		fplan->SetModuleLocation(*it,(int)fplan->m_modules[*it].m_x,y_pos*ROWHEIGHT+botto);
	    
	}

    int fool=0;
    int file=0;



    int cou=0;
    multimap<float,int>::reverse_iterator riter;
    int climb=0;
    int noclimb=0;
    int directput=0;
    //int pushcount=0;
    int parr[41];
    for(int i=0;i<41;i++)
    {
		parr[i]=0;
    }


  //  cout<<" \n progress: ";
    multimap<float,int> moduleinwidth;
  //  for( int i=0; i<(int)fplan->m_modules.size(); i++ )
  //  {
		//moduleinwidth.insert(pair<float,int>(cost[i],i));
  //  }
	for(it=macroIDset.begin(); it!=macroIDset.end(); ++it )
	{
		moduleinwidth.insert(pair<float,int>(fplan->m_modules[*it].m_area,*it));
	}

    //=======================================
    int step = (int)fplan->m_modules.size()/30;
    int counter = 0;
    if( step == 0 )
	step = 1;
    //=======================================

    int progress = 0;
    
    //int countcc=0;
    for(riter=moduleinwidth.rbegin();riter!=moduleinwidth.rend();riter++)
    {

	//flush( cout );
	if( counter % step == 0 )
	    //if( counter % 50 == 0 )
	{
	    //cout << "*";
	    cout << "[" << progress << "] ";
	    progress++;
	    flush( cout );
	}
	counter++;

	int i=riter->second;

	if((fplan->m_modules[i].m_isFixed==false))
	{
	    //	cout<<" i:"<<i<<" height:"<<fplan->m_modules[i].m_height<<" x"<<fplan->m_modules[i].m_x<<" w:"<<fplan->m_modules[i].m_width<<" n:"<<fplan->m_modules[i].m_name;
	    //	cout<<" c:"<<countcc;


	    int row_pos=(int)( (fplan->m_modules[i].m_y-botto)/ROWHEIGHT );

	    //if no overlap ---> direct place the cell
	    if( (fplan->m_modules[i].m_height == fplan->m_rowHeight) && 
		    (core_rows[row_pos].check_overlap(fplan->m_modules[i].m_x,fplan->m_modules[i].m_width)==false)  )
	    {
		core_rows[row_pos].insert_module(fplan->m_modules[i].m_x,fplan->m_modules[i].m_width,i);
		//	cout<<"dir:"<<i<<"\n";
		directput++;
	    }
	    else
	    {
		//		cout<<"PASS 1!"<<i<<" =====\n";
		//int WINDOW=200;
		if(WINDOW<=0)
		{
		    WINDOW=200;
		}
		bool find=false;
		int fcount=0;
		float record=0;
		float low=0;
		float nx=0;
		float nr=0;
		int first=0;
		int p=0;
		int precord=0;



		for(p=1;p<WINDOW;p++)
		{					
		    for(int j=0;j<2*p+1;j++)
		    {
			int g=(j%2)?1:-1;
			int x=(int)(j+1)/2;

			int row=g*x;
			//int row = 0;  // 1D-search

			float start=fplan->m_modules[i].m_x-(  (p-x)*(ROWHEIGHT) );
			//						float end=fplan->m_modules[i].m_x+fplan->m_modules[i].m_width+( (p-x)*(ROWHEIGHT) );
			float end=fplan->m_modules[i].m_x+fplan->m_modules[i].m_width+( (p-x)*(ROWHEIGHT) );
			if( ( (row_pos+row)>=0 )&& ( (row_pos+row)<(int)core_rows.size() )&&(start!=end))
			{
			    float new_pos=find_place(i,start,end,row_pos+row,record);
			    if(new_pos!=(start-1)  )
			    {
				if(fcount==0)
				{
				    first=2;
				    low=record;
				    nx=new_pos;
				    nr=row_pos+row;
				    precord=p;
				}
				else
				    if(record<low)
				    {
					first=1;
					low=record;
					nx=new_pos;
					nr=row_pos+row;
					climb++;
					precord=p;
				    }
				fcount++;
				if( fcount>2  || p>=25 )
				{		
				    find=true;
				    break;
				}				
			    }
			}

		    }
		    if(find==true)
		    {
			if(first==2)
			    noclimb++;
			fplan->SetModuleLocation(i,nx,(nr)*ROWHEIGHT+botto);
			//for(unsigned int h=0;h<moduleinfo[i].nets.size();h++)
			//{
			//	netinfo[moduleinfo[i].nets[h]].calc(moduleinfo[i].nets[h],*fplan);
			//	
			//}
			int height=1;
			if(fplan->m_modules[i].m_height>fplan->m_rowHeight)
			{
			    height=int(fplan->m_modules[i].m_height/fplan->m_rowHeight);
			    if(fplan->m_rowHeight*height<fplan->m_modules[i].m_height)
				height++;

			}
			for( int q=0; q<height; q++ )
			{
			    int rowId = (int)(nr+q);
			    if(!core_rows[rowId].insert_module(nx,fplan->m_modules[i].m_width,i))
			    {
				cout << "\ninsert" << nr+q << " x:" << nx << " w:" 
				     << fplan->m_modules[i].m_width << " mod:" << i 
				     << " name:" << fplan->m_modules[i].m_name << "\n";

				core_rows[rowId].showspace();

				file++;
			    }
			}
			if((p-1)>=40)
			    parr[40]++;
			else
			    parr[precord-1]++;
			break;
		    }					
		}
		if(find==false)
		{
		    fool++;
		}
	    }

	    /*				int he=(fplan->m_modules[i].m_height/fplan->m_rowHeight);
					if(he>1000)
					{
					char filename[200];
					sprintf( filename, "fig_level_%d_%d.plt", cl ,i);
					fplan->OutputGnuplotFigure( filename );
					fplan->SaveBlockLocation();
					cout<<fplan->m_modules[i].m_name;
					cl++;
					}*/			

	    //if(row_pos==0)
	    //{
	    //	if(core_rows[row_pos].show_fail()!=0)
	    //		cout<<"\n on insert cell:"<<i<<'\n';
	    //}
	}
	cou++;
    }
    //	cout<<"\n---Modules can't be legalize:\n"<<fool<<"\n climb times:"<<climb<<" no climb:"<<noclimb<<
    //		" directput:"<<directput<<" push_count:"<<pushcount<<'\n';
    cout<<"\n---Modules can't be legalize: "<< fool<<"\n";
    //	cout<<"\n---FILE--\n"<<file<<"\n---FILE--\n";


    //	fplan->OutputPL( "fig_legal_ada3.pl" );
    //fplan->CalcHPWL();
    //cout << "Legal HPWL= " << fplan->GetHPWL() << "\n\n";
    //cout<<"movetotal:"<<movetotal<<"\n";


#if 0
    // Output statistic
    cout<<"parr:\n";
    for(int i=0;i<41;i++)
    {
	if(i%8==0)
	    cout<<"\n";
	cout<<" p["<<i<<"]="<<parr[i]<<" ";
    }
#endif 

    if(fool==0)
	return true;    // Success
    else
	return false;   // Fail

}

void CNetinfo::show(int netID,CPlaceDB& cf)  //
{
	cout<<"  net:"<<netID<<"---|";
	for(unsigned int i=0;i<cf.m_nets[netID].size();i++)
	{
		cout<<cf.m_nets[netID][i]<<" ";
	}
	cout<<"|  ";
}
void CNetinfo::calc(int netID,CPlaceDB& cf)  //
{
	left=-1;
	right=-1;
	top=-1;
	bottom=-1;
	for(unsigned int i=0;i<cf.m_nets[netID].size();i++)
	{
		add(cf.m_modules[cf.m_pins[cf.m_nets[netID][i]].moduleId].m_x,
		    cf.m_modules[cf.m_pins[cf.m_nets[netID][i]].moduleId].m_y,
		    cf.m_modules[cf.m_pins[cf.m_nets[netID][i]].moduleId].m_width,
		    cf.m_modules[cf.m_pins[cf.m_nets[netID][i]].moduleId].m_height,
		    cf.m_pins[cf.m_nets[netID][i]].moduleId);
	}
	oldhpwl=hpwl();
		
}
