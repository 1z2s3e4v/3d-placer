// 2006-03-05 (donnie)

#include "smooth.h"



#include <cassert>
#include <cmath>
#include <vector>
using namespace std;

void GaussianSmooth::Smooth( vector< vector<double> >& input )
{

    vector< vector< double > > paddedInput;

    int padSize = (int)m_kernel.size() / 2;
    int height = (int)input.size();
    int width = (int)input[0].size();
    int paddedHeight = height + padSize * 2;
    int paddedWidth  = width + padSize * 2;
    
    // pre-normalization
    double total = 0;
    for( int i=0; i<height; i++ )
	for( int j=0; j<width; j++ )
	    total += input[i][j];

    if( total == 0 )
	return;

    paddedInput.resize( paddedHeight );
    for( int i=0; i<height; i++ )
    {
	paddedInput[i+padSize].resize( paddedWidth );

	for( int j=0; j<padSize; j++ )
	    paddedInput[i+padSize][j] = input[i][0];

	for( int j=0; j<width; j++ )
	    paddedInput[i+padSize][j+padSize] = input[i][j];

	for( int j=0; j<padSize; j++ )
	    paddedInput[i+padSize][j+width+padSize] = input[i][width-1];
    }
    for( int i=0; i<padSize; i++ )
    {
	paddedInput[i] = paddedInput[padSize];
	paddedInput[paddedHeight-i-1] = paddedInput[paddedHeight-padSize-1];
    }


    // 2D convolution
    assert( (int)m_kernel.size() == padSize * 2 + 1 );
    for( int i=0; i<height; i++ )
	for( int j=0; j<width; j++ )
	{
	    double sum = 0;
	    for( int m=-padSize; m<=padSize; m++ )
		for( int n=-padSize; n<=padSize; n++ )
		    sum += m_kernel[m+padSize][n+padSize] * paddedInput[i+m+padSize][j+n+padSize];
	    input[i][j] = sum;
	}

    // normalization
    double new_total = 0;
    for( int i=0; i<height; i++ )
	for( int j=0; j<width; j++ )
	    new_total += input[i][j];
    for( int i=0; i<height; i++ )
	for( int j=0; j<width; j++ )
	    input[i][j] = input[i][j] * total / new_total;

}

void GaussianSmooth::Gaussian2D( double theta, int size )
{
    m_kernel.resize( size );
    for( int i=0; i<size; i++ )
    {
	m_kernel[i].resize( size );
	for( int j=0; j<size; j++ )
	{
	    m_kernel[i][j] = GaussianDiscrete2D( theta, i-(size/2), j-(size/2) );
	}
    }

}

double GaussianSmooth::GaussianDiscrete2D( double theta, int x, int y )
{
    double g = 0;
    for(double ySubPixel = y - 0.5; ySubPixel < y + 0.6; ySubPixel += 0.1)
    {
	for(double xSubPixel = x - 0.5; xSubPixel < x + 0.6; xSubPixel += 0.1)
	{
	    g = g + ((1/(2*PI*theta*theta)) * 
		    exp( -(xSubPixel*xSubPixel+ySubPixel*ySubPixel)/
			(2*theta*theta)));
	}
    }
    g = g/121.0;
    return g;
}

void GaussianSmooth::Smooth( vector< vector<float> >& input )
{

    vector< vector< float > > paddedInput;

    int padSize = (int)m_kernel_f.size() / 2;
    int height = (int)input.size();
    int width = (int)input[0].size();
    int paddedHeight = height + padSize * 2;
    int paddedWidth  = width + padSize * 2;
    
    // pre-normalization
    float total = 0;
    for( int i=0; i<height; i++ )
	for( int j=0; j<width; j++ )
	    total += input[i][j];

    if( total == 0 )
	return;

    paddedInput.resize( paddedHeight );
    for( int i=0; i<height; i++ )
    {
	paddedInput[i+padSize].resize( paddedWidth );

	for( int j=0; j<padSize; j++ )
	    paddedInput[i+padSize][j] = input[i][0];

	for( int j=0; j<width; j++ )
	    paddedInput[i+padSize][j+padSize] = input[i][j];

	for( int j=0; j<padSize; j++ )
	    paddedInput[i+padSize][j+width+padSize] = input[i][width-1];
    }
    for( int i=0; i<padSize; i++ )
    {
	paddedInput[i] = paddedInput[padSize];
	paddedInput[paddedHeight-i-1] = paddedInput[paddedHeight-padSize-1];
    }


    // 2D convolution
    assert( (int)m_kernel_f.size() == padSize * 2 + 1 );
    for( int i=0; i<height; i++ )
	for( int j=0; j<width; j++ )
	{
	    float sum = 0;
	    for( int m=-padSize; m<=padSize; m++ )
		for( int n=-padSize; n<=padSize; n++ )
		    sum += m_kernel_f[m+padSize][n+padSize] * paddedInput[i+m+padSize][j+n+padSize];
	    input[i][j] = sum;
	}

    // normalization
    float new_total = 0;
    for( int i=0; i<height; i++ )
	for( int j=0; j<width; j++ )
	    new_total += input[i][j];
    for( int i=0; i<height; i++ )
	for( int j=0; j<width; j++ )
	    input[i][j] = input[i][j] * total / new_total;

}

void GaussianSmooth::Gaussian2D( float theta, int size )
{
    m_kernel_f.resize( size );
    for( int i=0; i<size; i++ )
    {
	m_kernel_f[i].resize( size );
	for( int j=0; j<size; j++ )
	{
	    m_kernel_f[i][j] = GaussianDiscrete2D( theta, i-(size/2), j-(size/2) );
	}
    }

}

float GaussianSmooth::GaussianDiscrete2D( float theta, int x, int y )
{
    float g = 0;
    for(float ySubPixel = y - 0.5; ySubPixel < y + 0.6; ySubPixel += 0.1)
    {
	for(float xSubPixel = x - 0.5; xSubPixel < x + 0.6; xSubPixel += 0.1)
	{
	    g = g + ((1/(2*PI*theta*theta)) * 
		    exp( -(xSubPixel*xSubPixel+ySubPixel*ySubPixel)/
			(2*theta*theta)));
	}
    }
    g = g/121.0;
    return g;
}
