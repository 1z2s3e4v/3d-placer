#include "solverQP.h"
extern "C" {
#include "errhandl.h"
#include "vector.h"
#include "qmatrix.h"
#include "matrix.h"
#include "operats.h"
#include "precond.h"
#include "rtc.h"
#include "copyrght.h"
#include "itersolv.h"
}

CQP::CQP(void)
{
}

CQP::~CQP(void)
{
	mapA.clear();
}
void CQP::setValue(const int&i, const int&j, const double& val)
{
	mapA[i][j]=val;
}
void CQP::getValue(const int&i, const int&j, double& val)
{
	map<int,double>::iterator iter;
	iter=mapA[i].find(j);

	if(iter!=mapA[i].end()) //the entry ij exists
	{
		val=mapA[i][j];
	}
	else
	{
		val=0;
	}
}
void CQP::solverQP(const int& iterMax, const vector<double>& vecB,vector<double>& vecX)
{
	double eps=0.001;
	double Omega=1.2;

	QMatrix M;
	Vector B;
	Vector X;
	size_t size=mapA.size();



	Q_Constr( &M,  "GW_Seeee", size, False, Rowws, Normal, True ); 
	for(unsigned int i=0; i<mapA.size(); i++)
	{
		map<int,double>::iterator iter;
		int entrySize=mapA[i].size();
		Q_SetLen(&M, i+1, entrySize); 
		int entryCount=0;

		for(iter=mapA[i].begin(); iter!=mapA[i].end(); iter++)
		{
			Q_SetEntry(&M, i+1,entryCount, iter->first+1, iter->second); 
			entryCount++;
		}
	}


	V_Constr(&X, "x", size, Normal, True);
	for(unsigned int i=0;i<mapA.size(); i++)
	{
		V_SetCmp(&X,i+1,vecX[i]);
	}
	V_Constr(&B, "b", size, Normal, True);
	for(unsigned int i=0;i<mapA.size(); i++)
	{
		V_SetCmp(&B,i+1,vecB[i]);
	}



	SetRTCAccuracy( eps );	
	BiCGSTABIter( &M, &X, &B, iterMax, ILUPrecond, Omega );


	for(unsigned int i=0; i<mapA.size(); i++)
	{
		vecX[i]=V_GetCmp(&X,i+1);
	}

	V_Destr(&X);
	V_Destr(&B);
	Q_Destr(&M);



}


//for CQP2
CQP2::CQP2(int size)
{
    mapA.resize(size);
}

CQP2::~CQP2(void)
{
	mapA.clear();
}
void CQP2::setValue(const int&i, const int&j, const double& val)
{
	mapA[i][j]=val;
}
void CQP2::getValue(const int&i, const int&j, double& val)
{
	map<int,double>::iterator iter;
	iter=mapA[i].find(j);

	if(iter!=mapA[i].end()) //the entry ij exists
	{
		val=mapA[i][j];
	}
	else
	{
		val=0;
	}
}
void CQP2::solverQP(const int& iterMax, const vector<double>& vecB,vector<double>& vecX)
{
	double eps=0.001;
	double Omega=1.2;

	QMatrix M;
	Vector B;
	Vector X;
	size_t size=mapA.size();



	Q_Constr( &M,  "GW_Seeee", size, False, Rowws, Normal, True ); 
	for(unsigned int i=0; i<mapA.size(); i++)
	{
		map<int,double>::iterator iter;
		int entrySize=mapA[i].size();
		Q_SetLen(&M, i+1, entrySize); 
		int entryCount=0;

		for(iter=mapA[i].begin(); iter!=mapA[i].end(); iter++)
		{
			Q_SetEntry(&M, i+1,entryCount, iter->first+1, iter->second); 
			entryCount++;
		}
	}


	V_Constr(&X, "x", size, Normal, True);
	for(unsigned int i=0;i<mapA.size(); i++)
	{
		V_SetCmp(&X,i+1,vecX[i]);
	}
	V_Constr(&B, "b", size, Normal, True);
	for(unsigned int i=0;i<mapA.size(); i++)
	{
		V_SetCmp(&B,i+1,vecB[i]);
	}



	SetRTCAccuracy( eps );	
	BiCGSTABIter( &M, &X, &B, iterMax, ILUPrecond, Omega );


	for(unsigned int i=0; i<mapA.size(); i++)
	{
		vecX[i]=V_GetCmp(&X,i+1);
	}

	V_Destr(&X);
	V_Destr(&B);
	Q_Destr(&M);



}