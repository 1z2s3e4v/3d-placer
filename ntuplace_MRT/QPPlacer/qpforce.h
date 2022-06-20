#ifndef QPFORCE_H
#define QPFORCE_H
#include "placedb.h"

/**
	@author Indark <indark@eda.ee.ntu.edu.tw>
*/





class CQPForce{
public:
    CQPForce();
    CQPForce( CPlaceDB & pDB , int potentialGridSize );

    ~CQPForce();
    void GetForce( vector< double >&  grad_potential,  double avg_force );
    
	private:
    CPlaceDB* m_pDB;
    
    struct potentialStruct    
    {
	    potentialStruct( const int& x, const int& y, const double& p ) 
		    : gx(x), gy(y), potential(p)
	    {}
	    int gx;
	    int gy;
	    double potential;
    };
    
    vector< vector<potentialStruct> > m_newPotential;
    vector< vector<double> > m_basePotential;	// 2006-03-03 (donnie) preplaced block potential  
    vector< vector<double> > m_basePotentialOri;
    vector< vector<double> > m_gridPotential;
    vector< vector<double> > m_expBinPotential;	// 2006-03-14 (donnie) preplaced block potential 

    vector< double > _cellPotentialNorm;		// cell potential normalization factor
    
    
    int m_potentialGridSize;
    int _potentialGridR;
    int m_interatios;
    double m_potentialGridWidth;
    double m_potentialGridHeight;
    double _potentialRY;
    double _potentialRX;
    
    
    double m_alwaysOverPotential;
    
    
    
    void CreatePotentialGrid();
    void GetClosestGrid( const double& x1, const double& y1, int& gx, int& gy ) ;
    void UpdatePotentialGridBase();
    double GetXGrid( const int& gx );
    double GetYGrid( const int& gy );
    void OutputPotentialGrid( string filename );		       
    double GetPotential( const double& x1, const double& x2, const double& r, const double& w );
    double GetGradPotential( const double& x1, const double& x2, const double& r, const double& w );
    void ComputeNewPotentialGrid();
    void UpdatePotentialGrid();
    void UpdateExpBinPotential( double util );
    void GetPotentialGrad(  const int& i, double& gradX, double& gradY);
    
};


class MyGaussianSmooth
{
	public:
		void   Smooth( vector< vector<double> >& input );
		double GaussianDiscrete2D( double theta, int x, int y );
		void   Gaussian2D( double theta, int size );
		vector< vector< double > > m_kernel;
};


#endif
