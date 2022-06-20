#ifndef DETAIL_H
#define DETAIL_H
#include "placedb.h"
#include "ParamPlacement.h"
#include <ctime>
#include <vector>
#include <list>
#include <climits>

class de_Point{
public:
	de_Point()
	{}

	double x;
	double y;
};

template <class T>
class Array2D{
private:
	T * data;
	int n;
public:
	Array2D(int num)
	{
		n=num;
		data=new T[n*n];
	}

	~Array2D()
	{
		delete [] data;
	}
	void put(int i ,int j , T d)
	{
		data[i*n+j]=d;
	}
	T get(int i, int j)
	{
		return data[i*n+j];
	}
};


class bimatching{

public:
	bimatching(int num):zeros(num),covers(num),costs(num)
	{

		n=num;
		next=0;
		starcol=new bool[n];
		starrow=new bool[n];
		covercol=new bool[n];
		coverrow=new bool[n];

		for(int i=0;i<n;i++)
		{
			starcol[i]=false;
			starrow[i]=false;
			covercol[i]=false;
			coverrow[i]=false;
			for(int j=0;j<n;j++)
			{
				zeros.put(i,j,0);
				covers.put(i,j,0);
				costs.put(i,j,0);
			}
		}

	}
	~bimatching()
	{
		delete [] starcol;
		delete [] starrow;
		delete [] covercol;
		delete [] coverrow;


	}

	void find();
	void step3();
	void step4();
	void step5();
	void step6(double low);

	int findstar(int y);  // find a star Sn in column y , return row id

	int findprime(int x);  // find a prime Pn in row x,return col id


	int n;
	int next;
	double s6in;
	int p0x;
	int p0y;
	Array2D<int> zeros;
	Array2D<int> covers;
	Array2D<double> costs;
	bool * starcol;
	bool * starrow;
	bool * covercol;
	bool * coverrow;

	vector<int> getresult();
	void getresult(vector<int>& v);
	void show();

};



class de_Row
{
public:
	de_Row( double m_x=0, double m_y=0, double m_length=0 )
	{
		this->m_x   = m_x ;
		this->m_y  = m_y ;
		this->m_length    = m_length;

		
	}
	void showspace();
	void showmodule();
	bool insert_module(double , double,int);
	bool add_empty(double,double);// x,w
	bool remove_empty(double,double);// x,w

	int find_ncell(double,double);//start,end , return # of modules between filled space{start,end}
	int clean_empties(); // return # of 0 length empty

	map< double, double > m_empties;//x-coordinate,length
	map<double,int> m_rowmodule;//x-coordinate,module ID
	double m_x, m_y,m_length;

};
class lap2
{
public:
    lap2(int deg)
    {
        m_deg=deg;
        m_cost.resize(m_deg);
        for(unsigned int i=0; i<m_cost.size(); ++i)
        {
            m_cost[i].resize(m_deg,0);
        }
        m_assignment.resize(deg,0);
        INF=INT_MAX;
        verbose=false;
    }
    ~lap2(void){};


    void put(const int& i, const int& j, double wl)
    {
        m_cost[i][j] = static_cast<int>(wl);
    }

    int INF;
    int m_deg;
    vector<vector<int> > m_cost;
    vector<int> m_assignment;  
    int lap_solve();
	void getresult(vector<int>& v)
    {
        v=this->m_assignment;
    }
    bool verbose;
};
class CNetLengthCalc
{
public:
	CNetLengthCalc( CPlaceDB& db,int mID )
	{
		m_pDB = &db;
		m_mID=mID;
	}

	void init();
	double calc();
	void calcOptWindow(double& maxX, double& minX, double& maxY, double& minY);
	double dist(double x, double left, double right);

    //for find multiple module's opt window (find X coordinate only!!)
	CNetLengthCalc( CPlaceDB& db, const set<int>& mset )
	{
        assert(mset.size()>0);
		m_pDB = &db;
        m_moduleSet=mset;
	}
    void initS(const set<int> cellsInRow,const double& left, const double& right);
	void calcOptWindowS(double& maxX, double& minX,const set<int> cellsInRow,const double& left, const double& right);

private:

	vector<double> m_bottom;
	vector<double> m_top;
	vector<double> m_left;
	vector<double> m_right;
	vector<vector<int> > m_pinID;
	vector<bool> m_validNet;
    set<int> m_moduleSet;

	int m_mID;
	CPlaceDB* m_pDB;
};
class de_Detail
{
public:
	de_Detail(CPlaceDB& cf )
	{
		direction=0;
		MAXWINDOW=64;
		MAXMODULE=MAXWINDOW;
		pIndepent=false;
		pRW=false;
		fplan=&cf;
		ROWHEIGHT=fplan->m_rowHeight;
		num_rows=(int)fplan->m_sites.size();
//		cout<<"\n Rows:"<<num_rows<<'\n';


		bottom=fplan->m_coreRgn.bottom;
		double row_length=fplan->m_coreRgn.right-fplan->m_coreRgn.left;
		// initailize de_Row
		for(int i=0;i<num_rows;i++)
		{
			de_Row row(fplan->m_coreRgn.left,fplan->m_sites[i].m_bottom,row_length);
			m_de_row.push_back(row);
		}

		// build row empty map
		for(unsigned int i=0;i<fplan->m_sites.size();i++)
		{
			int pos=(int)( (fplan->m_sites[i].m_bottom-fplan->m_coreRgn.bottom)/ROWHEIGHT);
			for(unsigned int j=0;j<fplan->m_sites[i].m_interval.size();j=j+2)
			{
				m_de_row[pos].m_empties[fplan->m_sites[i].m_interval[j]]=fplan->m_sites[i].m_interval[j+1]-fplan->m_sites[i].m_interval[j];
		
			}			
		}

		//insert cells
		for( unsigned int i=0; i<(unsigned int)fplan->m_modules.size(); i++ )
		{
			if(fplan->m_modules[i].m_isFixed==false)
			{
				int height=1;
				if(fplan->m_modules[i].m_height>fplan->m_rowHeight)
				{
					height=(int)(fplan->m_modules[i].m_height/fplan->m_rowHeight);
					if(fplan->m_rowHeight*height<fplan->m_modules[i].m_height)
						height++;
				}

				int pos=y2rowID(fplan->m_modules[i].m_y);
				for(int j=0;j<height;j++)
				{
					m_de_row[pos+j].insert_module(fplan->m_modules[i].m_x,fplan->m_modules[i].m_width,i);
				}
				
			}
		}

	
	}
	int y2rowID(double y)
	{
		return (int)((y-fplan->m_coreRgn.bottom)/fplan->m_rowHeight);
	}

	void detail(double,double,double,double); //start x, start y, width, height
	void grid_run(int,int); //int window,  times of rowheight,int overlap
	bool isConnection(int,int);//mID1,mID2, return true if no connection
	void remove_module(int,int);// mid,rowID

	double ROWHEIGHT;
	double bottom;
	int num_rows;
	int MAXWINDOW;
	int MAXMODULE;
	int direction;
	bool pIndepent;
	bool pRW;
	CPlaceDB * fplan; 
	vector<de_Row> m_de_row;


};


class deRunDetail
{
public:
	deRunDetail() : m_stop(0.2) {}
	~deRunDetail(){}
	//void runDetail(const CParamPlacement& param, CPlaceDB& placedb);
	void runDetail(const CParamPlacement& param, CPlaceDB& placedb, int ite=10, int indIte=5 );

	// 2006-09-30 (donnie)
	double m_stop;

};
#endif
/*--------------------------------------------------------------------------------
[detail] 2005_03_12  ,12 runs, window:100-8*i
ada1  -1.51% 
ada3  -1.43%  time<110sec*12

36runs, 150-4*i%  ,MAXWINDOW=64
ada1  -1.65% , 1343.45sec
ada3  -1.61% , 3302.73 sec

MAXWINDOW=81
ada1  -1.66%
ada3  -1.66% , 4678.78 sec

MAXWINODW=49
ada1  -1.64
ada3  -1.61 , 2227sec

MAXWINDOW=64,36 runs,big->small, small->big
ada1  -1.65  , 1274sec
ada3  -1.63  , 3094sec

Note:
ada1 window size:48
---------------------------------------------------------------------------------*/

