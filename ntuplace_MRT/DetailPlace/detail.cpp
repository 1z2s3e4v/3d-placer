#include <map>
#include <cfloat>
#include "detail.h"

int lap2::lap_solve()

// input:
// m_deg        - problem size
// m_cost - cost matrix

// output:
// m_assignment     - column assigned to row in solution
// m_assignment     - row assigned to column in solution
// u          - dual variables, row reduction numbers
// v          - dual variables, column reduction numbers

{
  bool unassignedfound;
  int  i, imin, numfree = 0, prvnumfree, f, i0, k, freerow;
  int  j, j1, j2, endofpath, last = 0, low, up;
  int min = 0, h, umin, usubmin, v2;
  j2=0;
  endofpath=0;


  vector<int> free;
  free.resize(m_deg,0);
  vector<int> collist;
  collist.resize(m_deg,0);
  vector<int> matches;
  matches.resize(m_deg,0);
  vector<int> d;
  d.resize(m_deg,0);
  vector<int> pred;
  pred.resize(m_deg,0);
  

  vector<int> colsol;
  colsol.resize(m_deg,0);

  vector<int> u;
  u.resize(m_deg,0);

  vector<int> v;
  v.resize(m_deg,0);

  // COLUMN REDUCTION 
  for (j = m_deg-1; j >= 0; j--)    // reverse order gives better results.
  {
    // find minimum cost over rows.
    min = m_cost[0][j]; 
    imin = 0;
    for (i = 1; i < m_deg; i++)  
      if (m_cost[i][j] < min) 
      { 
        min = m_cost[i][j]; 
        imin = i;
      }
    v[j] = min; 

    if (++matches[imin] == 1) 
    { 
      // init assignment if minimum row assigned for first time.
      m_assignment[imin] = j; 
      colsol[j] = imin; 
    }
    else
      colsol[j] = -1;        // row already assigned, column not assigned.
  }

  // REDUCTION TRANSFER
  for (i = 0; i < m_deg; i++) 
    if (matches[i] == 0)     // fill list of unassigned 'free' rows.
      free[numfree++] = i;
    else
      if (matches[i] == 1)   // transfer reduction from rows that are assigned once.
      {
        j1 = m_assignment[i]; 
        min = INF;
        for (j = 0; j < m_deg; j++)  
          if (j != j1)
            if (m_cost[i][j] - v[j] < min) 
              min = m_cost[i][j] - v[j];
        v[j1] = v[j1] - min;
      }

  // AUGMENTING ROW REDUCTION 
  int loopcnt = 0;           // do-loop to be done twice.
  do
  {
    loopcnt++;

    // scan all free rows.
    // in some cases, a free row may be replaced with another one to be scanned next.
    k = 0; 
    prvnumfree = numfree; 
    numfree = 0;             // start list of rows still free after augmenting row reduction.
    while (k < prvnumfree)
    {
      i = free[k]; 
      k++;

      // find minimum and second minimum reduced cost over columns.
      umin = m_cost[i][0] - v[0]; 
      j1 = 0; 
      usubmin = INF;
      for (j = 1; j < m_deg; j++) 
      {
        h = m_cost[i][j] - v[j];
        if (h < usubmin)
          if (h >= umin) 
          { 
            usubmin = h; 
            j2 = j;
          }
          else 
          { 
            usubmin = umin; 
            umin = h; 
            j2 = j1; 
            j1 = j;
          }
      }

      i0 = colsol[j1];
      if (umin < usubmin) 
        // change the reduction of the minimum column to increase the minimum
        // reduced cost in the row to the subminimum.
        v[j1] = v[j1] - (usubmin - umin);
      else                   // minimum and subminimum equal.
        if (i0 >= 0)         // minimum column j1 is assigned.
        { 
          // swap columns j1 and j2, as j2 may be unassigned.
          j1 = j2; 
          i0 = colsol[j2];
        }

      // (re-)assign i to j1, possibly de-assigning an i0.
      m_assignment[i] = j1; 
      colsol[j1] = i;

      if (i0 >= 0)           // minimum column j1 assigned earlier.
        if (umin < usubmin) 
          // put in current k, and go back to that k.
          // continue augmenting path i - j1 with i0.
          free[--k] = i0; 
        else 
          // no further augmenting reduction possible.
          // store i0 in list of free rows for next phase.
          free[numfree++] = i0; 
    }
  }
  while (loopcnt < 2);       // repeat once.

  // AUGMENT SOLUTION for each free row.
  for (f = 0; f < numfree; f++) 
  {
    freerow = free[f];       // start row of augmenting path.

    // Dijkstra shortest path algorithm.
    // runs until unassigned column added to shortest path tree.
    for (j = 0; j < m_deg; j++)  
    { 
      d[j] = m_cost[freerow][j] - v[j]; 
      pred[j] = freerow;
      collist[j] = j;        // init column list.
    }

    low = 0; // columns in 0..low-1 are ready, now none.
    up = 0;  // columns in low..up-1 are to be scanned for current minimum, now none.
             // columns in up..m_deg-1 are to be considered later to find new minimum, 
             // at this stage the list simply contains all columns 
    unassignedfound = false;
    do
    {
      if (up == low)         // no more columns to be scanned for current minimum.
      {
        last = low - 1; 

        // scan columns for up..m_deg-1 to find all indices for which new minimum occurs.
        // store these indices between low..up-1 (increasing up). 
        min = d[collist[up++]]; 
        for (k = up; k < m_deg; k++) 
        {
          j = collist[k]; 
          h = d[j];
          if (h <= min)
          {
            if (h < min)     // new minimum.
            { 
              up = low;      // restart list at index low.
              min = h;
            }
            // new index with same minimum, put on undex up, and extend list.
            collist[k] = collist[up]; 
            collist[up++] = j; 
          }
        }

        // check if any of the minimum columns happens to be unassigned.
        // if so, we have an augmenting path right away.
        for (k = low; k < up; k++) 
          if (colsol[collist[k]] < 0) 
          {
            endofpath = collist[k];
            unassignedfound = true;
            break;
          }
      }

      if (!unassignedfound) 
      {
        // update 'distances' between freerow and all unscanned columns, via next scanned column.
        j1 = collist[low]; 
        low++; 
        i = colsol[j1]; 
        h = m_cost[i][j1] - v[j1] - min;

        for (k = up; k < m_deg; k++) 
        {
          j = collist[k]; 
          v2 = m_cost[i][j] - v[j] - h;
          if (v2 < d[j])
          {
            pred[j] = i;
            if (v2 == min)   // new column found at same minimum value
              if (colsol[j] < 0) 
              {
                // if unassigned, shortest augmenting path is complete.
                endofpath = j;
                unassignedfound = true;
                break;
              }
              // else add to list to be scanned right away.
              else 
              { 
                collist[k] = collist[up]; 
                collist[up++] = j; 
              }
            d[j] = v2;
          }
        }
      } 
    }
    while (!unassignedfound);

    // update column prices.
    for (k = 0; k <= last; k++)  
    { 
      j1 = collist[k]; 
      v[j1] = v[j1] + d[j1] - min;
    }

    // reset row and column assignments along the alternating path.
    do
    {
      i = pred[endofpath]; 
      colsol[endofpath] = i; 
      j1 = endofpath; 
      endofpath = m_assignment[i]; 
      m_assignment[i] = j1;
    }
    while (i != freerow);
  }

  // calculate optimal cost.
  int lapcost = 0;
  for (i = 0; i < m_deg; i++)  
  {
    j = m_assignment[i];
    lapcost = lapcost + m_cost[i][j]; 
  }
//  this->m_assignment=m_assignment;


  return lapcost;
}

void bimatching::find()
{
	//step 1
	for(int i=0;i<n;i++)
	{
		double small=costs.get(i,0);
		for(int j=1;j<n;j++)
		{
			if(costs.get(i,j)<small)
				small=costs.get(i,j);
		}
		for(int j=0;j<n;j++)
		{
			costs.put(i,j,costs.get(i,j)-small);
		}
	}
	//step 2
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<n;j++)
		{
			if( (costs.get(i,j)==0) && (starcol[j]==false)&& (starrow[i]==false))
			{
				zeros.put(i,j,2);
				starcol[j]=true;
				starrow[i]=true;
			}
		}
	}
	next=3;
	while(next!=7)
	{
//		show();
		if(next==3)
			step3();
		else if(next==4)
			step4();
		else if(next==5)
			step5();
		else if(next==6)
			step6(s6in);
		else
			cout<<"\n ERROR!!!";
	}


}

void bimatching::step3()
{
	int count=0;
	for(int i=0;i<n;i++)
	{
		if(starcol[i]==true)
		{
			covercol[i]=true;
			count++;
		}
	}
	if(count==n)
	{
		next=7;
		return;
	}
	else
	{
		next=4;
		return;
	}
}

void bimatching::step4()
{
	bool findflag=false;
	double low=0;
	int x=0;
	int y=0;
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<n;j++)
		{
			if( (covercol[j]==false) && (coverrow[i]==false))
			{
				if(findflag==false)
				{
					low=costs.get(i,j);
					x=i;
					y=j;
					findflag=true;
				}
				else if(costs.get(i,j)<low)
				{
					low=costs.get(i,j);
					x=i;
					y=j;
				}

			}
		}
	}
	if(low!=0)
	{
		s6in=low;
		next=6;
		return;
	}
	else
	{
		zeros.put(x,y,3);        //p0
		p0x=x;
		p0y=y;
		for(int i=0;i<n;i++)
		{
			if(zeros.get(x,i)==2)
			{
				coverrow[x]=true;
				covercol[i]=false;
				next=4;
				return;
			}
		}
		next=5;
		return;

	}
}
void bimatching::step5()
{
	vector<int> vPx,vPy,vSx,vSy;


	int Sx,Sy;
	Sy=p0y;
	Sx=p0x;
	do
	{
		vPx.push_back(Sx);
		vPy.push_back(Sy);
		Sx=findstar(Sy);
		if(Sx!=-1)
		{
			vSx.push_back(Sx);
			vSy.push_back(Sy);

			Sy=findprime(Sx);
		}
	}
	while(Sx!=-1);

	for(unsigned int i=0;i<vPx.size();i++)
	{
		zeros.put(vPx[i],vPy[i],2);
		starcol[vPy[i]]=true;
		starrow[vPx[i]]=true;
	}
	for(unsigned int i=0;i<vSx.size();i++)
	{
		zeros.put(vSx[i],vSy[i],0);
	}

	for(int i=0;i<n;i++)
	{
		coverrow[i]=false;
		covercol[i]=false;
		for(int j=0;j<n;j++)
		{
			if(zeros.get(i,j)==3)
				zeros.put(i,j,0);
		}	
	}
	next=3;
	return;

}
void bimatching::step6(double low)
{
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<n;j++)
		{
			if( (coverrow[i]==true)&&(covercol[j]==true))
			{
				costs.put(i,j,costs.get(i,j)+low);
			}
			else if( (coverrow[i]==false)&&(covercol[j]==false))
			{
				costs.put(i,j,costs.get(i,j)-low);
			}
		}	
	}
	next = 4; 
	return;
}

int bimatching::findstar(int y)  // find a star Sn in column y , return row id
{
	for(int i=0;i<n;i++)
	{
		if(zeros.get(i,y)==2)
		{
			return i;
		}
	}
	return -1;
}
int bimatching::findprime(int x)  // find a prime Pn in row x,return col id
{
	for(int i=0;i<n;i++)
	{
		if(zeros.get(x,i)==3)
		{
			return i;
		}
	}
	return -1;
}



vector<int> bimatching::getresult()
{
	vector<int> v;
	for(int i=0;i<n;i++)
	{
		v.push_back(0);
	}
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<n;j++)
		{
			if(zeros.get(i,j)==2)
			{
				v[i]=j;
			}
		}
	}
	return v;
}
void bimatching::getresult(vector<int>& v)
{
//	v.clear();
	v.resize(n,0);
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<n;j++)
		{
			if(zeros.get(i,j)==2)
			{
				v[i]=j;
			}
		}
	}
}
void bimatching::show()
{
	cout<<"\n next:"<<next<<" ";
	if(next==6)
		cout<<"s6in:"<<s6in;
	cout<<"\n";
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<n;j++)
		{
			cout<<"\t"<< costs.get(i,j)<<",";
			if(zeros.get(i,j)==2)
				cout<<"*";
			if(zeros.get(i,j)==3)
				cout<<"'";
			if( (covercol[j]==true)&&(coverrow[i]==false) )
				cout<<"| ";
			else if( (covercol[j]==false)&&(coverrow[i]==true) )
				cout<<" -";
			else if( (covercol[j]==true)&&(coverrow[i]==true) )
				cout<<"|-";
			else if( (covercol[j]==false)&&(coverrow[i]==false) )
				cout<<"  ";
		}
		cout<<"\n";
	}
	return;
}

void de_Row::showspace()
{
	cout<<"\n=====ROW SPACE ===\n";

	map<double,double>::iterator iter;
	for(iter=m_empties.begin();iter!=m_empties.end();iter++)
	{
	    //modified by Jin 20070727
	    printf( "[%.10f,%.10f] ", iter->first, iter->second );
	    //cout<<" ["<<iter->first<<","<<iter->second<<"] ";
	    //modified by Jin 20070727
	}
	cout<<'\n';

}
void de_Row::showmodule()
{
	cout<<"\n=====ROW Module ===\n";

	map<double,int>::iterator iter;
	for(iter=m_rowmodule.begin();iter!=m_rowmodule.end();iter++)
	{
		cout<<" ["<<iter->first<<","<<iter->second<<"] ";
	}
	cout<<'\n';

}


int de_Row::clean_empties()
{
	int count=0;
	map<double,double>::iterator iter;
	vector<double> zeroset;
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

void de_Detail::remove_module( int mID, int rID)
{
	//modified by Jin 20070727
	//SHOULD NOT use double as the index of map
	m_de_row[rID].add_empty(fplan->m_modules[mID].m_x,fplan->m_modules[mID].m_width);
	double targetX = round( fplan->m_modules[mID].m_x );
	map<double, int>::iterator ite = m_de_row[rID].m_rowmodule.find( targetX );
	if( ite == m_de_row[rID].m_rowmodule.end() )
	{
	    printf( "Warning: mID %d not found\n", mID );
	}
	else if( ite->second != mID )
	{
	    printf( "Warning: mID %d does not equal ite->second %d\n", mID, ite->second );
	}
	else
	{	    
	    m_de_row[rID].m_rowmodule.erase(ite);
	}
#if 0	
	m_de_row[rID].add_empty(fplan->m_modules[mID].m_x,fplan->m_modules[mID].m_width);
	m_de_row[rID].m_rowmodule.erase(fplan->m_modules[mID].m_x);
#endif
	//@modified by Jin 20070727	
	
}

bool de_Row::add_empty(double x,double w)
{
	//added by jin 20070727
	x = round(x);
	w = round(w);
	//added by jin 20070727
	bool front; //true:nonempty in front of the space
	bool end;  //true:nonempty after the space
	map<double,double>::iterator iter;
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
		double l=iter->second;

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
		double tail_x=iter->first;
		double tail_w=iter->second;
		--iter;
		m_empties[iter->first]=iter->second+w+tail_w;
		m_empties.erase(tail_x);
	}
	return true;
}
int de_Row::find_ncell(double start, double end)
{
	map<double,int>::iterator iter;
	int count=0;
	for(iter=m_rowmodule.lower_bound(start);iter!=m_rowmodule.lower_bound(end);iter++)
	{
		count++;
	}
	return count;
}

bool de_Row::insert_module(double x , double w, int mID)
{
	//modified by Jin 20070727
	//SHOULD NOT use double as the index of map
	x = round(x);
	w = round(w);
	//@modified by Jin 20070727	
    
	map<double,double>::iterator iter;
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
			double l2=x-iter->first;
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
				
				if( (x+w)< (iter->first+iter->second))
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
bool de_Row::remove_empty(double x , double w)
{
    //added by jin 20070727
    x = round(x);
    w = round(w);
    //@added by jin 20070727
    map<double,double>::iterator iter;
    iter=m_empties.upper_bound(x);
    if( iter == m_empties.begin() )
	return false;             //x<all empty site's start point

    --iter;
    if( (iter->second-(x-iter->first))< w )
	return false;

    double l2=x-iter->first;
    //cout<<"l2:"<<l2;
    if(l2==0)
    {
	if( ((iter->first+iter->second)-(x+w) )!=0)
	    m_empties[x+w]=(iter->first+iter->second)-(x+w);
	m_empties.erase(iter->first);
    }
    else
    {
	if( (x+w)< (iter->first+iter->second))
	{
	    //cout<<"\n--x+w :\n"<<x+w<<" "<<(iter->first+iter->second)-(x+w)<<" \n";
	    m_empties[x+w]=(iter->first+iter->second)-(x+w);
	}
	iter->second=l2;
    }

    //m_rowmodule[x]=mID;
    return true;
}


void de_Detail::detail(double find_x, double find_y, double find_width, double find_height)
{
	/*
	1.build module size set
	2.build position vector, if it's length > MAXWINDOW , cut it.
	3.calc cost matrix
	4.run bipartite matching
	5.save result
	*/

	int row_s = y2rowID(find_y);
	double top_y = find_y + find_height;
	if( (find_y + find_height) > (fplan->m_coreRgn.top - fplan->m_rowHeight))
	    top_y = fplan->m_coreRgn.top-fplan->m_rowHeight;
		
	int row_e = y2rowID(top_y);

	multimap<double,int> module_map;

	list<int> module_list;   //list<module ID>

	//build module size map
	for(int i=row_s; i<row_e; i++)
	{
	    map<double,int>::iterator iter;
	    //modified by Jin 20070727
	    map<double,int>::iterator begin, end;
	    begin = m_de_row[i].m_rowmodule.lower_bound(find_x);
	    end = m_de_row[i].m_rowmodule.lower_bound(find_x+find_width);
	    for( iter = begin ; iter != end ; iter++ )
	    {
		if( fplan->m_modules[iter->second].m_height == fplan->m_rowHeight ) //don't consider macros
		{   
		    module_map.insert( pair<double,int>( fplan->m_modules[iter->second].m_width, iter->second ) );
		}
	    }
#if 0
	    for( iter = m_de_row[i].m_rowmodule.lower_bound(find_x); 
		    iter != m_de_row[i].m_rowmodule.lower_bound(find_x+find_width); 
		    iter++ )
	    {
		if( fplan->m_modules[iter->second].m_height == fplan->m_rowHeight ) //don't consider macros
		{
		    module_map.insert( pair<double,int>( fplan->m_modules[iter->second].m_width, iter->second ) );
		}
	    }
#endif
	    //@modified by Jin 20070727
	}

	if( pRW==true )
	{
	    for( int i=row_s; i<row_e; i++ )
	    {
		map<double,int>::iterator iter;
		//modified by Jin 20070726
		map<double,int>::iterator begin =  m_de_row[i].m_rowmodule.lower_bound( find_x + find_width + 5*find_width );
		map<double,int>::iterator end = m_de_row[i].m_rowmodule.lower_bound( find_x + find_width + 6*find_width );
		for( iter = begin ; iter != end ; iter++ )
		{
		    if( fplan->m_modules[iter->second].m_height == fplan->m_rowHeight ) //don't consider macros
		    {
			module_map.insert( pair<double,int>( fplan->m_modules[iter->second].m_width, iter->second ) );
		    }
		}
#if 0
		for( iter = m_de_row[i].m_rowmodule.lower_bound( find_x + find_width + 5*find_width ); 
			iter != m_de_row[i].m_rowmodule.lower_bound( find_x + find_width + 6*find_width ); 
			iter++ )
		{
		    if( fplan->m_modules[iter->second].m_height == fplan->m_rowHeight ) //don't consider macros
		    {
			module_map.insert( pair<double,int>( fplan->m_modules[iter->second].m_width, iter->second ) );
		    }
		}
#endif
		//@modified by Jin 20070726
	    }
	}

	multimap<double,int>::reverse_iterator riter;

	//module_list stores ALL modules in current finding window, order by module width,decreaingly. 
	for( riter=module_map.rbegin(); riter!=module_map.rend(); riter++ )
	{
	    module_list.push_back( riter->second );
	    //cout<<" id:"<<riter->second<<" widht:"<<fplan->m_modules[riter->second].m_width;
	}

	while( module_list.size()!=0 )
	{
	    
	    vector<de_Point> position;
	    vector<int> modules;
	    position.reserve(MAXMODULE);
	    modules.reserve(MAXMODULE);

	    int insert_count=0;
	    const double WIDTH=fplan->m_modules[*module_list.begin()].m_width;
	    //cout<<" WIDHT:"<<WIDTH;

	    vector<int> remove_mID;
	    remove_mID.reserve(MAXMODULE);
	    vector<int> remove_rowID;
	    remove_rowID.reserve(MAXMODULE);
	    vector<double> empty_x;
	    empty_x.reserve(MAXMODULE);
	    vector<double> empty_w;
	    empty_w.reserve(MAXMODULE);
	    vector<int> empty_row;
	    empty_row.reserve(MAXMODULE);
	    list<int>::iterator iter;  //*iter means moduleID
	    for(iter=module_list.begin();iter!=module_list.end();)
	    {
		bool insert_flag=false;
		int rowID = y2rowID(fplan->m_modules[*iter].m_y);
		if(fplan->m_modules[*iter].m_width==WIDTH)
		{
		    bool connection=false;
		    if( pIndepent==true )
		    {
			for(int m=0;m<(int)modules.size();m++)
			{
			    if(isConnection(modules[m],*iter)==true)
				connection=true;
			}
		    }
		    if(connection==false)
		    {
			modules.push_back(*iter);
			de_Point dp;
			dp.x=fplan->m_modules[*iter].m_x;
			dp.y=fplan->m_modules[*iter].m_y;
			position.push_back(dp);
			//remove_module(*iter,rowID);
			remove_mID.push_back(*iter);
			remove_rowID.push_back(rowID);
			insert_flag=true;
		    }
		}
		else if(fplan->m_modules[*iter].m_width<WIDTH)
		{
		    map<double,double>::iterator mi=m_de_row[rowID].m_empties.upper_bound(fplan->m_modules[*iter].m_x);
		    if( mi!=m_de_row[rowID].m_empties.end())
		    {
			if(mi->first==fplan->m_modules[*iter].m_width+fplan->m_modules[*iter].m_x)
			{
			    if((fplan->m_modules[*iter].m_width+mi->second)>=WIDTH)
			    {
				bool connection=false;
				if(pIndepent==true)
				{
				    for(int m=0;m<(int)modules.size();m++)
				    {
					if(isConnection(modules[m],*iter)==true)
					    connection=true;
				    }
				}
				if(connection==false)
				{
				    double tail_x=fplan->m_modules[*iter].m_width+fplan->m_modules[*iter].m_x;
				    double tail_w=WIDTH-fplan->m_modules[*iter].m_width;
				    //	m_de_row[rowID].showspace();
				    //	m_de_row[rowID].showmodule();
				    //	cout<<" \n mID:"<<*iter<<" WIDTH:"<<WIDTH<<" mw:"<<fplan->m_modules[*iter].m_width;

				    modules.push_back(*iter);
				    de_Point dp;
				    dp.x=fplan->m_modules[*iter].m_x;
				    dp.y=fplan->m_modules[*iter].m_y;

				    position.push_back(dp);
				    //remove_module(*iter,rowID);
				    remove_mID.push_back(*iter);
				    remove_rowID.push_back(rowID);
				    empty_x.push_back(tail_x);
				    empty_w.push_back(tail_w);
				    empty_row.push_back(rowID);
				    m_de_row[rowID].remove_empty(tail_x,tail_w);
				    insert_flag=true;
				    //if(!m_de_row[rowID].insert_module(dp.x,WIDTH,-1))
				    //{
				    //	cout<<"\n -==========fail!!\ninsert:"<<dp.x<<" w:"<<WIDTH;
				    //	m_de_row[rowID].showspace();
				    //	m_de_row[rowID].showmodule();
				    //	int dd;
				    //	cin>>dd;

				    //}
				}

			    }
			}
		    }
		}


		if(insert_flag==true)
		{	
		    list<int>::iterator iter2=iter;
		    ++iter;
		    module_list.erase(iter2);
		    ++insert_count;
		    if(insert_count>=MAXMODULE)
			break;
		}
		else
		{
		    ++iter;
		}
	    }

	    // Handle whitespace

	    // 2007-7-13 (donnie)
	    // This part was disabled by accident so that the results are worse 1%--3%.
	    // I enabled now.
#if 1
	    if( insert_count<MAXWINDOW )
	    {
		int empty_count=0;
		for( int i=row_s; i<row_e; i++ )
		{
		    map<double,double>::iterator iter;
		    for( iter = m_de_row[i].m_empties.lower_bound(find_x);
			    iter != m_de_row[i].m_empties.lower_bound(find_x+find_width);
			    iter++ )
		    {
			double w = iter->second;
			int num_segments = (int)(w/WIDTH);
			for( int j=0; j<num_segments; j++ )
			{
			    modules.push_back(-1);
			    de_Point dp;
			    dp.x=iter->first+j*WIDTH;
			    dp.y=fplan->m_coreRgn.bottom+i*fplan->m_rowHeight;
			    position.push_back(dp);
			    empty_count++;
			    if(empty_count>=(MAXWINDOW-insert_count))
				break;
			}
		    }
		}
	    }
#endif

	    for(unsigned int i=0;i<empty_x.size();i++)
	    {
		m_de_row[empty_row[i]].add_empty(empty_x[i],empty_w[i]);
	    }

	    //remove selected modules
	    for(unsigned int i=0;i<remove_mID.size();i++)
	    {
		remove_module(remove_mID[i],remove_rowID[i]);
	    }


	    //3. eatablish bimatching object, calc cost matrix
	    int deg=modules.size();  //matrix degree
	    //bimatching matrix(deg);
	    lap2 matrix(deg);
	    
	    //calc cost matrix
	    for(int i=0;i<deg;i++)
	    {
		if( modules[i] == -1 )	// (donnie) whitespace?
		{
		    for(int j=0;j<deg;j++)
		    {
			matrix.put(i,j,0);
		    }
		}
		else
		{
		    //bool set=false;
		    double bestx = position[0].x;
		    double besty = position[0].y;
		    double bestwl = DBL_MAX;
		    //double ox=fplan->m_modules[modules[i]].m_x;
		    //double oy=fplan->m_modules[modules[i]].m_y;
		    CNetLengthCalc netcalc(*fplan,modules[i] );
		    netcalc.init();

		    for(int j=0;j<deg;j++)
		    {
			fplan->SetModuleLocation(modules[i],position[j].x,position[j].y);
			double wl=netcalc.calc();
			//cerr<<" "<<wl; 
			//double wl=0;
			//for(int k=0;k<(int)fplan->m_modules[modules[i]].m_netsId.size();k++)
			//{
			//  wl = wl+fplan->GetNetLength(fplan->m_modules[modules[i]].m_netsId[k]);
			//}
			//cerr<<","<<wl; 
			//if(wl!=wl2)
			//	cerr<<" "<<wl<<","<<wl2;
			//force module find empty space ^^  ==>FOOL!
			//if( (modules[j]!=modules[i])&&(modules[j]!=-1))
			//	wl=wl+100000;
			if(wl<bestwl)
			{
			    bestwl=wl;
			    bestx=position[j].x;
			    besty=position[j].y;
			}
			//matrix.costs.put(i,j,wl);
			matrix.put(i,j,wl);
		    }
		    //fplan->SetModuleLocation(modules[i],ox,oy);
		    fplan->SetModuleLocation(modules[i],bestx,besty);

		}
	    }

	    //	matrix.show();

	    //4.run matching
	    //matrix.find();
	    matrix.lap_solve();
	    vector<int> result;
	    matrix.getresult(result);

	    //5.save result
	    for(int i=0;i<deg;i++)
	    {
		if( modules[i] == -1 )
		    break;
		int pos = y2rowID(position[result[i]].y);

		if( !m_de_row[pos].insert_module(
			    position[result[i]].x,
			    fplan->m_modules[modules[i]].m_width,modules[i]))
		{
		    if( !gArg.CheckExist( "nocheck" ) ) // (donnie) 2007-03-25 
		    { 
			cout<<"\ninsert"<<pos<<" x:"<<position[result[i]].x
			    <<" w:"<<fplan->m_modules[modules[i]].m_width
			    <<" mod:"<<modules[i]
			    <<" name:"<<fplan->m_modules[modules[i]].m_name<<"\n";
			m_de_row[pos].showspace();
		    }
		}
		fplan->SetModuleLocation( modules[i], position[result[i]].x, position[result[i]].y );
	    }
	    //for(int i=0;i<deg;i++)
	    //{
	    //	int pos=y2rowID(position[i].y);
	    //	if(!m_de_row[pos].insert_module(position[i].x,fplan->m_modules[modules[i]].m_width,modules[i]))
	    //	{
	    //		cout<<"\ninsert"<<pos<<" x:"<<position[i].x<<" w:"<<fplan->m_modules[i].m_width<<" mod:"<<modules[i]<<" name:"<<fplan->m_modules[modules[i]].m_name<<"\n";
	    //		m_de_row[pos].showspace();
	    //	}
	    //	fplan->SetModuleLocation(modules[i],position[result[i]].x,position[result[i]].y);
	    //}

	}

}

void de_Detail::grid_run(int window,int overlap)
{
	if(overlap>=window)
		overlap=0;
	//fplan->CalcHPWL();
	//double wl1 = fplan->GetHPWLp2p();

	int x_num=(int)((fplan->m_coreRgn.right-fplan->m_coreRgn.left)/(ROWHEIGHT));
	int y_num=(int)((fplan->m_coreRgn.top-fplan->m_coreRgn.bottom)/(ROWHEIGHT));
//	cout<<"\n total bins:"<<total<<"\n";
	for(int i=0;i<y_num;i=i+window-overlap)
	{
		for(int j=0;j<x_num;j=j+window-overlap)
		{

			detail(fplan->m_coreRgn.left+j*ROWHEIGHT,fplan->m_coreRgn.bottom+i*ROWHEIGHT,
				window*ROWHEIGHT,window*ROWHEIGHT);
//			cout<<" bin:"<<i*x_num+j<<"/"<<total<<" ";

		}
		//fplan->CalcHPWL();
		//double wl2 = fplan->GetHPWLp2p();
		//cout << "\n [detail] Center-to-center HPWL= " << fplan->GetHPWL() << endl;
		//cout << " [detail] Pin-to-pin HPWL= " << fplan->GetHPWLp2p() << " (" << 100.0*(wl2/wl1-1.0) << "%)\n";
	}
}
bool de_Detail::isConnection(int m1, int m2 )
{
	for(int i=0;i<(int)fplan->m_modules[m1].m_netsId.size();i++)
	{
		for(int j=0;j<(int)fplan->m_modules[m2].m_netsId.size();j++)
		{
			if(fplan->m_modules[m1].m_netsId[i]==fplan->m_modules[m2].m_netsId[j])
				return true;

		}
	}
	return false;
}

//void deRunDetail::runDetail(const CParamPlacement& param, CPlaceDB& placedb)
void deRunDetail::runDetail(const CParamPlacement& param, CPlaceDB& placedb, int ite, int indIte )
{
	double total_time=seconds();
	placedb.CalcHPWL();
	double wl1=placedb.GetHPWLp2p();
	double detail_time_start = seconds();
	double wl3 = 0;
	double total_detail_time = 0;
	double part_time_start; 
		//by tellux
		//cout << "\nDeatiled placement... (max time: " << param.de_time_constrain << " sec)\n";
		cout << "\nCell Matching... (" << ite << ", " << indIte << ")\n";
		flush( cout );
		placedb.CalcHPWL();
		wl3 = placedb.GetHPWLp2p();

		placedb.SaveBlockLocation();

		//double totdetime = clock();
		de_Detail de(placedb);
		de.pIndepent = false;

		//FILE* out_detail_log;
		//out_detail_log = fopen( "log_detail.txt", "a" );
		//fprintf( out_detail_log ,"\n===== %s ====" , argv[1]);

		total_detail_time = seconds() - detail_time_start;
		int i=0;
		double preWL=wl3;
		double preWL2=wl3;
		double preWL3=wl3;
		de.pIndepent=false;
		double all_time_start = seconds(); // donnie 2006-03-13
		
		
		//double stop = -0.2; // donnie
		//double stop = -param.cellMatchingStop; // donnie 2006-04-01
		
		// 2006-09-30 (donnie)
		double stop = -m_stop;
		assert( stop < 0 );

		while( total_detail_time < param.de_time_constrain )
		{
			placedb.SaveBlockLocation();
			part_time_start = seconds();


			de.MAXWINDOW = param.de_MW;
			de.MAXMODULE = param.de_MM;
			int run_para1 = param.de_window-i;
			if(run_para1<15)
				run_para1=15+i%5;

			int run_para2=2+(int)(i/2);
			if(run_para2>8)
				run_para2=8;

			de.grid_run(run_para1,run_para2);
			placedb.CalcHPWL();
			double wlx = placedb.GetHPWLp2p();				
			double total_part_timex = double(seconds() - part_time_start);
			double all_time = seconds() - all_time_start;

			printf( " run:%2d HPWL=%.0f (%.3f%%)(%.3f%%)   time: %d sec   all: %d sec\n",
			     i, placedb.GetHPWLp2p(), 100.0*(wlx/preWL-1.0), 100.0*(wlx/wl3-1.0),
			    (int)total_part_timex, (int)all_time );
			fflush( stdout );
		
			/*	
			cout <<"    run:"<<i<<" HPWL= " << placedb.GetHPWLp2p() 
			    << " (" << 100.0*(wlx/preWL-1.0) << "%)"
			    << " (" << 100.0*(wlx/wl3-1.0) << "%)  ";
			double total_part_timex = double(seconds() - part_time_start);
			double all_time = seconds() - all_time_start;
			cout<<"\t time: "<< (int)total_part_timex << " sec    all: " << (int)all_time << " sec" << endl;
			*/

			//fprintf( out_detail_log ,"\n %5.3e , %5.3e , %.2f%% , %4.2fm , MW=%d, MM=%d, Run#=%d, RunP1=%d, RunP2=%d , Inte=%d" 
			//	,  wl3, wlx, (wlx-wl3)*100/wl1,
			//	total_part_timex/60, de.MAXWINDOW,de.MAXMODULE,i,run_para1,run_para2,de.pIndepent
			//	);

			/*
			if( i >= ite )
			{
			    de.pIndepent = true;
			    de.pRW = true;
			}

			if( i >= ite + indIte )
			    break;
			*/

			
			// donnie, change the stop order (2006-03-14)
			
			//if((100.0*(wlx/preWL3-1.0))>-0.06 && de.pIndepent )
			if((100.0*(wlx/preWL-1.0))>stop*3 && de.pIndepent )
			//if((100.0*(wlx/preWL-1.0))>stop*5 && de.pIndepent )
			    break;

			/*
			//if((100.0*(wlx/preWL-1.0))>-0.02)
			if((100.0*(wlx/preWL-1.0))>stop && de.pRW ) // by donnie
			{
			    
			    break;  // for ispd06
			    
			    if(de.pIndepent==false)
			    {
				de.pIndepent=true;
				cout << "  startInd\n";
			    }

			}
			*/

			// 2005/3/21 
			//if((100.0*(wlx/preWL-1.0))>-0.05)
			if((100.0*(wlx/preWL-1.0)) > stop) // by donnie
			{
			    //break;  // for ispd06

			    if(de.pRW==false)
			    {
				de.pRW=true;
				//cout << "  start Double Window \n";
			
				// skip double window (donnie) 2006-03-21	
				de.pIndepent=true;
				cout << "  startInd\n";
			    }
			}

			
			total_detail_time = seconds() - detail_time_start;
			i++;
			preWL3=preWL2;
			preWL2=preWL;
			preWL=wlx;

		}


		wl3 = placedb.GetHPWLp2p();
		cout << "DETAILED: Pin-to-pin HPWL= " << placedb.GetHPWLp2p() << " (" << 100.0*(wl3/wl1-1.0) << "%)\n";
		cout<<" Total runtime:"<<seconds()-total_time<<"\n";
		//double total_det_time = double(clock() - totdetime)/CLOCKS_PER_SEC;
		//fprintf( out_detail_log ,"\n=== Finish: %5.3e , %5.3e , %.2f%%, time:%4.1fm ====" 
		//		,  wl1, wl3, (wl3-wl1)*100/wl1,total_det_time/60 );		
		//fclose( out_detail_log );

#if 0
		if( param.bPlot )
		{
			string file = param.outFilePrefix + "_detail.plt";
			placedb.OutputGnuplotFigure( file.c_str(), true );
			//placedb.OutputGnuplotFigure( "out_legal.plt", false );
		}
		//end of tellux

		if( param.bShow )
		{
		    string file = param.outFilePrefix + "_detail.pl";
		    placedb.OutputPL( file.c_str() );
		}
#endif
}
void CNetLengthCalc::init()
{
	m_bottom.resize(m_pDB->m_modules[m_mID].m_netsId.size(),0);
	m_top.resize(m_pDB->m_modules[m_mID].m_netsId.size(),0);
	m_left.resize(m_pDB->m_modules[m_mID].m_netsId.size(),0);
	m_right.resize(m_pDB->m_modules[m_mID].m_netsId.size(),0);
	m_pinID.resize(m_pDB->m_modules[m_mID].m_netsId.size());
	m_validNet.resize(m_pDB->m_modules[m_mID].m_netsId.size(),true);

	for(int k=0;k<(int)m_pDB->m_modules[m_mID].m_netsId.size();k++)
	{
		int netId=m_pDB->m_modules[m_mID].m_netsId[k];
		int counter=0;
		int pid = m_pDB->m_nets[netId][counter];
		bool flag=false;
		while(m_pDB->m_pins[pid].moduleId==m_mID)
		{
			++counter;
			if(counter >= (int)m_pDB->m_nets[netId].size())
			{
				flag=true;
				break;
			}
			else
			{
				pid = m_pDB->m_nets[netId][counter];
			}
		}
		if(flag==true)
		{
			m_validNet[k]=false;
			continue;
		}
		double cx, cy;
		m_pDB->GetPinLocation( pid, cx, cy );
		double maxX = cx;
		double minX = cx;
		double minY = cy;
		double maxY = cy;

		for( int j=0; j<(int)m_pDB->m_nets[netId].size(); j++ )
		{
			pid = m_pDB->m_nets[netId][j];
			if(m_pDB->m_pins[pid].moduleId==m_mID)
			{
				m_pinID[k].push_back(pid);

			}
			else
			{
				m_pDB->GetPinLocation( pid, cx, cy );
				minX = min( minX, cx );
				maxX = max( maxX, cx );
				minY = min( minY, cy );
				maxY = max( maxY, cy );
			}

		}
		m_bottom[k]=minY;
		m_top[k]=maxY;
		m_left[k]=minX;
		m_right[k]=maxX;


	}
}
double CNetLengthCalc::calc()
{

	double wl=0;
	for(int k=0;k<(int)m_pinID.size();k++)
	{
		if(m_validNet[k]==true)
		{
			double cx, cy;
			int pid=m_pinID[k][0];
			m_pDB->GetPinLocation(pid, cx, cy );
			double maxX = cx;
			double minX = cx;
			double minY = cy;
			double maxY = cy;
			for(unsigned int i=1; i<m_pinID[k].size(); ++i)
			{
				m_pDB->GetPinLocation( m_pinID[k][i], cx, cy );
				minX = min( minX, cx );
				maxX = max( maxX, cx );
				minY = min( minY, cy );
				maxY = max( maxY, cy );
			}
			minX = min( minX, m_left[k] );
			maxX = max( maxX, m_right[k] );
			minY = min( minY, m_bottom[k] );
			maxY = max( maxY, m_top[k] );
			wl+=((maxX-minX) + (maxY-minY));

		}
	}
	return wl;
}
double CNetLengthCalc::dist(double x, double left, double right)
{
	if(x<left)
		return left-x;
	else if(x<=right)
		return 0;
	else 
		return x-right;

}
void CNetLengthCalc::calcOptWindow(double& maxX, double& minX, double& maxY, double& minY)
{
	init();
	vector<double> x_edge;
	vector<double> y_edge;
	x_edge.reserve(2*m_validNet.size());
	y_edge.reserve(2*m_validNet.size());
	for(unsigned int i=0; i<m_validNet.size(); ++i)
	{
		if(m_validNet[i]==true)
		{
			x_edge.push_back(this->m_left[i]);
			x_edge.push_back(this->m_right[i]);
			y_edge.push_back(this->m_top[i]);
			y_edge.push_back(this->m_bottom[i]);
		}
	}
	
	sort(x_edge.begin(),x_edge.end());
	sort(y_edge.begin(),y_edge.end());
	int helf=x_edge.size()/2;
	minX=x_edge[helf-1];
	maxX=x_edge[helf];
	minY=y_edge[helf-1];
	maxY=y_edge[helf];

}
void CNetLengthCalc::initS(const set<int> cellsInRow,const double& left, const double& right)
{
    //step 1: build common net set
    set<int> commonNetID;
    for(set<int>::iterator it=this->m_moduleSet.begin(); it!=this->m_moduleSet.end(); ++it)
    {
        for(int i=0; i<(int)m_pDB->m_modules[*it].m_netsId.size(); ++i)
        {
            commonNetID.insert(m_pDB->m_modules[*it].m_netsId[i]);
        }
    }
	m_bottom.resize(commonNetID.size(),0);
	m_top.resize(commonNetID.size(),0);
	m_left.resize(commonNetID.size(),0);
	m_right.resize(commonNetID.size(),0);
	m_pinID.resize(commonNetID.size());
	m_validNet.resize(commonNetID.size(),true);
    int k=0;

	for(set<int>::iterator it=commonNetID.begin(); it!=commonNetID.end(); ++it)
	{
		int netId=*it;
		int counter=0;
		int pid = m_pDB->m_nets[netId][counter];
		bool flag=false;
        while(this->m_moduleSet.find(m_pDB->m_pins[pid].moduleId)!=this->m_moduleSet.end())
		{
			++counter;
			if(counter>= (int)m_pDB->m_nets[netId].size())
			{
				flag=true;
				break;
			}
			else
			{
				pid = m_pDB->m_nets[netId][counter];
			}
		}
		if(flag==true)
		{
			m_validNet[k]=false;
			continue;
		}
		double cx, cy;
		m_pDB->GetPinLocation( pid, cx, cy );
		double maxX = cx;
		double minX = cx;
		double minY = cy;
		double maxY = cy;

		for( int j=0; j<(int)m_pDB->m_nets[netId].size(); j++ )
		{
			pid = m_pDB->m_nets[netId][j];
            if(m_moduleSet.find(m_pDB->m_pins[pid].moduleId)==m_moduleSet.end())
			{
                //if(cellsInRow.find(m_pDB->m_pins[pid].moduleId)==cellsInRow.end())
                //{
				    m_pDB->GetPinLocation( pid, cx, cy );
				    minX = min( minX, cx );
				    maxX = max( maxX, cx );
				    minY = min( minY, cy );
				    maxY = max( maxY, cy );
        //        }
        //        else
        //        {
				    //m_pDB->GetPinLocation( pid, cx, cy );
        //            double clusterX=m_pDB->m_modules[*cellsInRow.begin()].m_cx;
        //            if(cx<clusterX)
        //            {
				    //    minX = min( minX, left );
				    //    maxX = max( maxX, left );
        //            }
        //            else
        //            {
				    //    minX = min( minX, right );
				    //    maxX = max( maxX, right );
        //            }


				    //minY = min( minY, cy );
				    //maxY = max( maxY, cy );
        //        }

			}
		}
		m_bottom[k]=minY;
		m_top[k]=maxY;
		m_left[k]=minX;
		m_right[k]=maxX;
        ++k;

	}
}
void CNetLengthCalc::calcOptWindowS(double& maxX, double& minX,const set<int> cellsInRow,const double& left, const double& right)
{
	initS(cellsInRow, left, right);
	vector<double> x_edge;
	x_edge.reserve(2*m_validNet.size());
	for(unsigned int i=0; i<m_validNet.size(); ++i)
	{
		if(m_validNet[i]==true)
		{
			x_edge.push_back(this->m_left[i]);
			x_edge.push_back(this->m_right[i]);
		}
	}
	
	sort(x_edge.begin(),x_edge.end());
	int helf=x_edge.size()/2;
	minX=x_edge[helf-1];
	maxX=x_edge[helf];

}
