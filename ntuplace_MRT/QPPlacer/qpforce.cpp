#include "qpforce.h"
#include "ParamPlacement.h"
#include "util.h"
#include <cassert>
#include <cmath>
#include <vector>


using namespace std;

CQPForce::CQPForce()
{
}


CQPForce::~CQPForce()
{
}

CQPForce::CQPForce( CPlaceDB & pDB , int potentialGridSize )
	:m_potentialGridSize(potentialGridSize)
{
    // donnie 2006-09-23
    double binSize = 0.8;
    gArg.GetDouble( "binSize", &binSize );

    _potentialGridR  = 2 ; ///NOTE constant 
    m_pDB = &pDB;
    if( m_potentialGridSize <= 0 )
	m_potentialGridSize = static_cast<int>( sqrt(static_cast<double>( m_pDB->m_modules.size()) ) * binSize );
    CreatePotentialGrid();
    UpdatePotentialGridBase();

    int r = 5;
    MyGaussianSmooth smooth;
    smooth.Gaussian2D( r, 6*r+1 );
    smooth.Smooth( m_basePotential );
    UpdateExpBinPotential(-1);
    m_interatios=0;
    OutputPotentialGrid("qpforce_base.dat");

}





void MyGaussianSmooth::Smooth( vector< vector<double> >& input )
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

void MyGaussianSmooth::Gaussian2D( double theta, int size )
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

double MyGaussianSmooth::GaussianDiscrete2D( double theta, int x, int y )
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






void CQPForce::CreatePotentialGrid()
{
    printf( "Create Potential Grid\n" );
	m_gridPotential.clear(); // remove old values
	///NOTE: Not Used

	int realGridSize = m_potentialGridSize;

	m_newPotential.resize( m_pDB->m_modules.size() );
	m_gridPotential.resize( realGridSize );
	m_basePotential.resize( realGridSize );
	_cellPotentialNorm.resize( m_pDB->m_modules.size() );
	for( int i=0; i<realGridSize; i++ )
	{
		m_basePotential[i].resize( realGridSize, 0 );
		m_gridPotential[i].resize( realGridSize, 0 );
	}

	m_potentialGridWidth  = ( m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left ) / m_potentialGridSize;
	m_potentialGridHeight = ( m_pDB->m_coreRgn.top   - m_pDB->m_coreRgn.bottom ) / m_potentialGridSize;
	_potentialRX = m_potentialGridWidth  * _potentialGridR;
	_potentialRY = m_potentialGridHeight * _potentialGridR;

}


void CQPForce::UpdatePotentialGridBase( )
{
	//double binArea = m_potentialGridWidth * m_potentialGridHeight;
	for( unsigned int i=0; i<m_basePotential.size(); i++ )
	{
		fill( m_basePotential[i].begin(), m_basePotential[i].end(), 0.0 );
		//m_binFreeSpace[i].resize( m_basePotential[i].size() );
		//fill( m_binFreeSpace[i].begin(), m_binFreeSpace[i].end(), binArea );
	}

	for( int i=0; i<(int)m_pDB->m_modules.size(); i++ )
	{
	// for each cell. cell ci coordinate is ( x[i*2], x[i*2+1] )

		if( m_pDB->m_modules[i].m_isFixed == false )
			continue;

	// TODO: BUG when shrinking core?
		//if( m_pDB->m_modules[i].m_isOutCore )
		if( m_pDB->BlockOutCore( i ) )
			continue;	// pads?
	
		int gx, gy;
		double cellX = m_pDB->m_modules[i].m_cx;
		double cellY = m_pDB->m_modules[i].m_cy;
		double width  = m_pDB->m_modules[i].m_width;
		double height = m_pDB->m_modules[i].m_height;

		//double potentialRX = _potentialRX;
		//double potentialRY = _potentialRY;
	//double left   = cellX - width * 0.5  - potentialRX;
	//double bottom = cellY - height * 0.5 - potentialRY;
		double left   = cellX - width * 0.5;  // for gaussian smoothing
		double bottom = cellY - height * 0.5; // for gaussian smoothing
		double right  = cellX + (cellX - left);
		double top    = cellY + (cellY - bottom);
		if( left   < m_pDB->m_coreRgn.left )     left   = m_pDB->m_coreRgn.left;
		if( bottom < m_pDB->m_coreRgn.bottom )   bottom = m_pDB->m_coreRgn.bottom;
		if( top    > m_pDB->m_coreRgn.top )      top    = m_pDB->m_coreRgn.top;
		if( right  > m_pDB->m_coreRgn.right )    right  = m_pDB->m_coreRgn.right;
		GetClosestGrid( left, bottom, gx, gy );
		if( gx < 0 )  gx = 0;
		if( gy < 0 )  gy = 0;
      
		//double totalPotential = 0;
		vector< potentialStruct > potentialList;      
		int gxx, gyy;
		double xx, yy;

	//if( m_useBellPotentialForPreplaced == false )
		{
	    // "Exact density for the potential"
			for( gxx = gx, xx = GetXGrid(gx); xx<=right ; gxx++, xx+=m_potentialGridWidth )
			{
				for( gyy = gy, yy = GetYGrid(gy); yy<=top ; gyy++, yy+=m_potentialGridHeight )
				{
					m_basePotential[gxx][gyy] +=
							getOverlap( left, right, xx, xx+m_potentialGridWidth ) * 
							getOverlap( bottom, top, yy, yy+m_potentialGridHeight );

					//m_binFreeSpace[gxx][gyy] -= 
					//		getOverlap( left, right, xx, xx+m_potentialGridWidth ) * 
					//		getOverlap( bottom, top, yy, yy+m_potentialGridHeight );
				}
			}
			continue;
		}
# if 0 
///NOTE: this part is used for scaling 
		for( gxx = gx, xx = GetXGrid(gx); xx<=right ; gxx++, xx+=m_potentialGridWidth )
		{
			for( gyy = gy, yy = GetYGrid(gy); yy<=top ; gyy++, yy+=m_potentialGridHeight )
			{
				double potential = GetPotential( cellX, xx, potentialRX, width ) *
							GetPotential( cellY, yy, potentialRY, height );
				if( potential > 0 )
				{
					totalPotential += potential;
					potentialList.push_back( potentialStruct( gxx, gyy, potential ) );
				}
			}
		}

	// normalize the potential so that total potential equals the cell area
		double scale = m_pDB->m_modules[i].m_area / totalPotential;
	//printf( "totalPotential = %f\n", totalPotential );

		_cellPotentialNorm[i] = scale;	    // normalization factor for the cell i

		vector< potentialStruct >::const_iterator ite;
		for( ite=potentialList.begin(); ite!=potentialList.end(); ++ite )
		{
			if(	ite->gx < 0 || ite->gx >= (int)m_gridPotential.size() ||
						     ite->gy < 0 || ite->gy >= (int)m_gridPotential[ite->gx].size() )
				continue; // bin may be outside when core-shrinking is applied
			else
				m_basePotential[ ite->gx ][ ite->gy ] += ite->potential * scale;	    
		}

#endif	
	} // for each cell

	m_basePotentialOri = m_basePotential;   // make a copy for TSP-style smoothing


}


void CQPForce::GetClosestGrid( const double& x1, const double& y1, int& gx, int& gy ) 
{
	gx = static_cast<int>( floor( ( x1 - m_pDB->m_coreRgn.left ) / m_potentialGridWidth ) );
	gy = static_cast<int>( floor( ( y1 - m_pDB->m_coreRgn.bottom ) / m_potentialGridHeight ) );

}

//the following two functions return the coordinates of grids 
double CQPForce::GetXGrid( const int& gx )
{
	return m_pDB->m_coreRgn.left + gx * m_potentialGridWidth + 0.5 * m_potentialGridWidth;
}

double CQPForce::GetYGrid( const int& gy )
{
	return  m_pDB->m_coreRgn.bottom + gy * m_potentialGridHeight + 0.5 * m_potentialGridHeight;
}


void CQPForce::OutputPotentialGrid( string filename )
{
	int stepSize = (int)m_gridPotential.size() / 50;
	if( stepSize == 0 )
		stepSize = 1;
	FILE* out = fopen( filename.c_str(), "w" );
	double binArea = m_potentialGridWidth * m_potentialGridHeight;
	for( unsigned int j=0; j<m_gridPotential.size(); j+=stepSize )
	{
		for( unsigned int i=0; i<m_gridPotential.size(); i+=stepSize )
			fprintf( out, "%.03f ", (m_gridPotential[i][j] + m_basePotential[i][j]) / binArea );
		fprintf( out, "\n" );
	}
	fprintf( out, "\n" );
	fclose( out );
}



void CQPForce::ComputeNewPotentialGrid()
{

	for(unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    	{
	// for each cell. cell ci coordinate is ( x[i*2], x[i*2+1] )

		//if( m_pDB->m_modules[i].m_isOutCore )
		if( m_pDB->BlockOutCore( i ) )
			continue;

	// preplaced blocks are stored in m_basePotential
		if( m_pDB->m_modules[i].m_isFixed )
			continue;
	
		int gx, gy;
		double cellX = m_pDB->m_modules[i].m_cx;
		double cellY = m_pDB->m_modules[i].m_cy;
		double potentialRX = _potentialRX;
		double potentialRY = _potentialRY;
		double width  = m_pDB->m_modules[i].m_width;
		double height = m_pDB->m_modules[i].m_height;
		double left   = cellX - width * 0.5  - potentialRX;
		double bottom = cellY - height * 0.5 - potentialRY;
		double right  = cellX + (cellX - left);
		double top    = cellY + (cellY - bottom);
		if( left   < m_pDB->m_coreRgn.left )     left   = m_pDB->m_coreRgn.left;
		if( bottom < m_pDB->m_coreRgn.bottom )   bottom = m_pDB->m_coreRgn.bottom;
		if( top    > m_pDB->m_coreRgn.top )      top    = m_pDB->m_coreRgn.top;
		if( right  > m_pDB->m_coreRgn.right )    right  = m_pDB->m_coreRgn.right;
		GetClosestGrid( left, bottom, gx, gy );
       	
		double totalPotential = 0;
	//vector< potentialStruct > potentialList;      
		vector< potentialStruct >& potentialList = m_newPotential[i];     
		potentialList.clear();	
		int gxx, gyy;
		double xx, yy;

	//// TEST (convert to std-cell)
		if( height < m_potentialGridHeight && width < m_potentialGridWidth )
			width = height = 0;

		for( gxx = gx, xx = GetXGrid(gx); xx<=right ; gxx++, xx+=m_potentialGridWidth )
		{
			for( gyy = gy, yy =GetYGrid(gy); yy<=top ; gyy++, yy+=m_potentialGridHeight )
			{
				double potential = GetPotential( cellX, xx, potentialRX, width ) *
							GetPotential( cellY, yy, potentialRY, height );
				if( potential > 0 )
				{
					totalPotential += potential;
					potentialList.push_back( potentialStruct( gxx, gyy, potential ) );
				}
			}
		}

	// normalize the potential so that total potential equals the cell area
		double scale = m_pDB->m_modules[i].m_area / totalPotential;
	//printf( "totalPotential = %f\n", totalPotential );

	
		_cellPotentialNorm[i] = scale;	    // normalization factor for the cell i

	} // for each cell

}


double CQPForce::GetPotential( const double& x1, const double& x2, const double& r, const double& w )
{
	double d = fabs( x1 - x2 );
	double a = 4.0 / ( w + r ) / ( w + 2 * r );
	double b = 4.0 / r / ( w + 2.0 * r );

	if( d <= w * 0.5 + r * 0.5 )
		return 1.0 - a * d * d;
	else if( d <= w * 0.5 + r )
		return b * ( d - r - w * 0.5 ) * ( d - r - w * 0.5);
	else
		return 0.0;
}


double CQPForce::GetGradPotential( const double& x1, const double& x2, const double& r, const double& w )
{
	double d;
	double a = 4.0 / ( w + r ) / ( w + 2.0 * r );
	double b = 4.0 / r / ( w + 2.0 * r );

	if( x1 >= x2 )  // right half
	{
		d = x1 - x2;	// d >= 0
		if( d <= w * 0.5 + r * 0.5 )
			return -2.0 * a * d;
		else if( d <= w * 0.5 + r )
			return +2.0 * b * ( d - r - w * 0.5);
		else
			return 0;
	}
	else    // left half
	{
		d = x2 - x1;	// d >= 0	
		if( d <= w * 0.5 + r * 0.5 )
			return +2.0 * a * d;
		else if( d <= w * 0.5 + r )
			return -2.0 * b * ( d - r - w * 0.5);
		else
			return 0;
	}
}


void CQPForce::GetForce( vector< double >&  grad_potential, double avg_force )
{
	char buf[100];
	grad_potential.resize ( 2* m_pDB->m_modules.size());
	fill (grad_potential.begin(),grad_potential.end(), 0.0);
			
	ComputeNewPotentialGrid();
	UpdatePotentialGrid();
	sprintf(buf,"potential-%d.dat",++m_interatios);
	OutputPotentialGrid(buf);
	
	
	
	double gradDensityX;
	double gradDensityY;
	double TotalForce = 0.0;
	double _scale;
	int    n_movable = 0;
	for( unsigned int i=0 ;  i<m_pDB->m_modules.size(); i++ )	    // for each cell
	{
		if( m_pDB->m_modules[i].m_isFixed )
			continue;
		GetPotentialGrad(  i, gradDensityX, gradDensityY );	    // bell-shaped potential
		grad_potential[2*i]   = gradDensityX;
		grad_potential[2*i+1] = gradDensityY;
		TotalForce += fabs(gradDensityX);
		TotalForce += fabs(gradDensityY);
		n_movable++;
	} // for each cell
	
	_scale = avg_force / (TotalForce / n_movable) ;
	if (avg_force > 0 ){
		cout << "Force Scale:" << _scale << endl;
		
		for( unsigned int i=0 ;  i<m_pDB->m_modules.size(); i++ )	    // for each cell
		{
			if( m_pDB->m_modules[i].m_isFixed )
				continue;
			grad_potential[2*i]   *= _scale;
			grad_potential[2*i+1] *= _scale;
		}
	}
	
}

void CQPForce::UpdatePotentialGrid()
{
        for( int gx=0; gx<(int)m_gridPotential.size(); gx++ )
		fill( m_gridPotential[gx].begin(), m_gridPotential[gx].end(), 0.0 );

	for( unsigned int i=0; i<m_newPotential.size(); i++ )	// for each cell
		for( unsigned int j=0; j<m_newPotential[i].size(); j++ ) // for each bin
	{
		int gx = m_newPotential[i][j].gx;
		int gy = m_newPotential[i][j].gy;
		m_gridPotential[ gx ][ gy ] += m_newPotential[i][j].potential * _cellPotentialNorm[i];
	}
}



void CQPForce::UpdateExpBinPotential( double util )
{
	double binArea = m_potentialGridWidth * m_potentialGridHeight;

	if( util < 0 )
		util = 1.0; // use all space

	double totalFree = 0;
	int zeroSpaceBin = 0;
	m_expBinPotential.resize( m_basePotential.size() );
	for( unsigned int i=0; i<m_basePotential.size(); i++ )
	{
		m_expBinPotential[i].resize( m_basePotential[i].size() );
		for( unsigned int j=0; j<m_basePotential[i].size(); j++ )
		{
			double base = m_basePotential[i][j];
			double free = binArea - base;
			if( free > 1e-4 )
			{
				m_expBinPotential[i][j] = free * util;
				totalFree += m_expBinPotential[i][j];
			}
			else
			{
				m_expBinPotential[i][j] = 0.0;
				zeroSpaceBin++;
			}
		} 
	}

	if( param.bShow )
	{
		printf( "PBIN: Expect bin potential utilization: %f\n", util );
		printf( "PBIN: Zero space bin # = %d\n", zeroSpaceBin );
		printf( "PBIN: Total free potential = %.0f (%.5f)\n", totalFree, m_pDB->m_totalMovableModuleArea / totalFree );
	}

    // TODO: scaling?
    //assert( m_pDB->m_totalMovableModuleArea / totalFree <= 1.000001 );
// 	double alwaysOver = 0.0;
// 	if( m_targetUtil > 0.0 && m_targetUtil < 1.0 )
// 	{
// 		for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
// 		{
// 			if( m_pDB->m_modules[i].m_isFixed )
// 				continue;
// 			if( m_pDB->m_modules[i].m_width >= 2 * m_potentialGridWidth && 
// 						 m_pDB->m_modules[i].m_height >= 2 * m_potentialGridHeight )
// 			{
// 				alwaysOver += 
// 						(m_pDB->m_modules[i].m_width - m_potentialGridWidth ) * 
// 						(m_pDB->m_modules[i].m_height - m_potentialGridHeight ) * 
// 						(1.0 - m_targetUtil );
// 			}
// 		}
// 		if( param.bShow )
// 			printf( "PBIN: Always over: %.0f (%.1f%%)\n", alwaysOver, alwaysOver/m_pDB->m_totalMovableModuleArea*100.0 );
// 	}
// 	m_alwaysOverPotential = alwaysOver;
}


void CQPForce::GetPotentialGrad(  const int& i, double& gradX, double& gradY)
{
	double cellX = m_pDB->m_modules[i].m_cx;
	double cellY = m_pDB->m_modules[i].m_cy;

	double width  = m_pDB->m_modules[i].m_width;
	double height = m_pDB->m_modules[i].m_height;
	double left   = cellX - width  * 0.5 - _potentialRX;
	double bottom = cellY - height * 0.5 - _potentialRY;
	double right  = cellX + ( cellX - left );
	double top    = cellY + ( cellY - bottom );
	if( left   < m_pDB->m_coreRgn.left )		left   = m_pDB->m_coreRgn.left;
	if( bottom < m_pDB->m_coreRgn.bottom )	        bottom = m_pDB->m_coreRgn.bottom;
	if( right  > m_pDB->m_coreRgn.right )		right  = m_pDB->m_coreRgn.right;
	if( top    > m_pDB->m_coreRgn.top )		top    = m_pDB->m_coreRgn.top;
	int gx, gy;
	GetClosestGrid( left, bottom, gx, gy );

	if( gx < 0 )	gx = 0;
	if( gy < 0 )	gy = 0;

	int gxx, gyy;
	double xx, yy;

    //// use square to model small std-cells
	if( height < m_potentialGridHeight && width < m_potentialGridWidth )
		width = height = 0;

	gradX = 0.0;	
	gradY = 0.0;
	for( gxx = gx, xx = GetXGrid( gx ); 
		    xx <= right && gx < (int)m_gridPotential.size(); 
		    gxx++, xx += m_potentialGridWidth )
	{

		for( gyy = gy, yy = GetYGrid( gy ); 
				   yy <= top && gy < (int)m_gridPotential.size() ; 
				   gyy++, yy += m_potentialGridHeight )
		{

			double gX = 0;
			double gY = 0;
			gX = ( m_gridPotential[gxx][gyy] - m_expBinPotential[gxx][gyy] ) *
					_cellPotentialNorm[i] *
					GetGradPotential( cellX, xx, _potentialRX, width ) *
					GetPotential(     cellY, yy, _potentialRY, height );
			gY =  ( m_gridPotential[gxx][gyy] - m_expBinPotential[gxx][gyy] ) *
					_cellPotentialNorm[i] *
					GetPotential(     cellX, xx, _potentialRX, width  ) *
					GetGradPotential( cellY, yy, _potentialRY, height );
			gradX += gX;
			gradY += gY;
		}
	} // for each grid
}


