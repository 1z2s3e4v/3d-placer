#ifndef SMOOTH_H
#define SMOOTH_H

#include "util.h"

#include <vector>

class GaussianSmooth
{
    public: // (kaie) change double to float
	void   Smooth( vector< vector<double> >& input );
	double GaussianDiscrete2D( double theta, int x, int y );
	void   Gaussian2D( double theta, int size );
	vector< vector< double> > m_kernel;

	// (kaie) change double to float
	void   Smooth( vector< vector<float> >& input );
	float GaussianDiscrete2D( float theta, int x, int y );
	void   Gaussian2D( float theta, int size );
	vector< vector< float> > m_kernel_f;
	// @(kaie)
};


#endif
