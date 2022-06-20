#pragma once
#include <map>
#include <vector>

using namespace std;

class CQP
{
	public:
		CQP(void);
		~CQP(void);
		map<int,map<int,double> > mapA;

		void increment(const int& i, const int& j,const  double& val)
		{
			double temp;
			getValue(i,j,temp);
			temp+=val;
			setValue(i,j,temp);

		}
		void decrement(const int& i, const int& j,const  double& val)
		{
			double temp;
			getValue(i,j,temp);
			temp-=val;
			setValue(i,j,temp);

		}

		void setValue(const int&i, const int&j, const double& val); //set value: Entry(i,j), value
		void getValue(const int&i, const int&j, double& val);
		void solverQP(const int& iterMax, const vector<double>& vecB, vector<double>& vecX);

		//void checkMapA()
		//{
		//	for(unsigned int i=0; i<mapA.size(); i++)
		//	{
		//		
		//		map<int,double>::iterator iter;
		//		iter=mapA[i].find(i);
		//		assert(iter!=mapA[i].end());
		//		assert(iter->second>0);

		//		double dio=iter->second;

		//		double count=0;

		//		for(iter=mapA[i].begin(); iter!=mapA[i].end(); iter++)
		//		{
		//			double v2;
		//			getValue(iter->first,i,v2);
		//			assert(v2==iter->second);
		//			if(iter->first!=i)
		//			{
		//				count+=iter->second;
		//			}
		//		}
		//		assert(count<=dio);
		//	}
		//}

};

class CQP2
{
	public:
		CQP2(int size);
		~CQP2(void);
		vector<map<int,double> > mapA;

		void increment(const int& i, const int& j,const  double& val)
		{
			double temp;
			getValue(i,j,temp);
			temp+=val;
			setValue(i,j,temp);

		}
		void decrement(const int& i, const int& j,const  double& val)
		{
			double temp;
			getValue(i,j,temp);
			temp-=val;
			setValue(i,j,temp);

		}

		void setValue(const int&i, const int&j, const double& val); //set value: Entry(i,j), value
		void getValue(const int&i, const int&j, double& val);
		void solverQP(const int& iterMax, const vector<double>& vecB, vector<double>& vecX);

};
