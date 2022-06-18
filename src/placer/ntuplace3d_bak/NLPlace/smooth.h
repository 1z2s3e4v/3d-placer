#ifndef SMOOTH_H
#define SMOOTH_H

#include "../PlaceCommon/util.h"

#include <vector>

class GaussianSmooth
{
    public: // (kaie) change double to float
	void   Smooth( vector< vector<double> >& input );
	double GaussianDiscrete2D( double theta, int x, int y );
	void   Gaussian2D( double theta, int size );
	vector< vector< double> > m_kernel;

	// // (kaie) change double to float
	// void   Smooth( vector< vector<float> >& input );
	// float GaussianDiscrete2D( float theta, int x, int y );
	// void   Gaussian2D( float theta, int size );
	// vector< vector< float> > m_kernel_f;
	// // @(kaie)

	// (kaie) 2009-09-19 3d
	void   Smooth( vector< vector< vector<double> > >& input );
	double GaussianDiscrete3D( double theta, int x, int y, int z );
	void   Gaussian3D( double theta, int size );
	vector< vector< vector< double > > > m_kernel3d;
	// @(kaie) 2009-09-19
};


#endif
