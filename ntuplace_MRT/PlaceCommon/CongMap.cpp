//Created by Jin 20060602
#include "CongMap.h"
#include "placeutil.h"
#include "GlobalRouting.h"
#include "util.h"
#include <limits>
#include <algorithm>

using namespace std;

CCongMap::CCongMap( const int& hdim, const int& vdim, const CRect& bbox, const CParserLEFDEF* const pLEF ):
    m_hdim(hdim), m_vdim(vdim), m_left(bbox.left), m_right(bbox.right),
    m_top(bbox.top), m_bottom(bbox.bottom),
    m_hlength( (m_right-m_left)/static_cast<double>(m_hdim) ),
    m_vlength( (m_top-m_bottom)/static_cast<double>(m_vdim) ),
    m_maxBinLength(0.0),
    m_num_wire_per_hcut(0.0), 
    m_num_wire_per_vcut(0.0),
    m_hcut_layer_count(0),
    m_vcut_layer_count(0)
{
    //Initialization of m_numLocalNet and m_lengthLocalNet
    m_numLocalNet.resize( m_hdim );
    m_lengthLocalNet.resize( m_hdim );
    for( int i = 0 ; i < m_hdim ; i++ )
    {
	m_numLocalNet[i].resize( m_vdim, 0 );
	m_lengthLocalNet[i].resize( m_vdim, 0.0 );
    }

    
    //Compute capacities for H/V cuts
    if( pLEF && pLEF->m_metalPitch.size() > 0 )
    {
	double average_vpitch = 0.0;
	double vcut_layer_count = 0.0;
	double average_hpitch = 0.0;
	double hcut_layer_count = 0.0;
	
	int start_layer = 2;
	if( gArg.CheckExist( "nocellblockage" ) )
	{
	    start_layer = 1;
	}
		
	
	for( unsigned int i = start_layer ; i < pLEF->m_metalPitch.size() ; i++ )
	{
	    if( pLEF->m_metalDir[i] == CParserLEFDEF::VERTICAL )
	    {
		average_hpitch += pLEF->m_metalPitch[i]*pLEF->m_lefUnit;
		hcut_layer_count += 1.0;
	    }
	    else if( pLEF->m_metalDir[i] == CParserLEFDEF::HORIZONTAL )
	    {
		average_vpitch += pLEF->m_metalPitch[i]*pLEF->m_lefUnit;
		vcut_layer_count += 1.0;
	    }
	    else
	    {
		fprintf( stderr, "Warning: Undefined metal layer direction %d\n",
			pLEF->m_metalDir[i] );
	    }

	    //printf("%.3f\n", pLEF->m_metalPitch[i]*pLEF->m_lefUnit);
	}

	m_hcut_layer_count = static_cast<int>( hcut_layer_count );
	m_vcut_layer_count = static_cast<int>( vcut_layer_count );

	average_hpitch = average_hpitch / hcut_layer_count;
	average_vpitch = average_vpitch / vcut_layer_count;

	m_num_wire_per_hcut = (m_hlength / average_hpitch)*hcut_layer_count;
	m_num_wire_per_vcut = (m_vlength / average_hpitch)*vcut_layer_count;

	m_maxBinLength = (m_num_wire_per_vcut*m_hlength) + (m_num_wire_per_hcut*m_vlength);
	
	// show less info (donnie) 2006-09-26
	
	
	//test code
	if( gArg.IsDev() && !gArg.CheckExist( "noCongMapInfo" ) )
	{
	    fprintf( stdout, "    Bounding box for CongMap: (%.2f,%.2f,%.2f,%.2f)\n", m_left, m_right, m_top, m_bottom );
	    fprintf( stdout, "    Bin width: %.2f Bin height: %.2f\n", m_hlength, m_vlength );
	    fprintf( stdout, "    Bin width capacity: %.2f height capacity: %.2f\n", 
		    m_num_wire_per_hcut, m_num_wire_per_vcut );
	}
	//@test code
    
	//printf("hcut capacity: %f\nvcut capacity: %f\n", m_num_wire_per_hcut, m_vcut_capaicty );
    }
    else
    {
	if( !gArg.CheckExist( "nocheck" ) )
	    fprintf(stderr, "Warning: LEF data was not set up\n");
    }

    //initialization of m_HCuts and m_VCuts
    m_HCuts.resize( m_vdim+1 );
    for( unsigned int i = 0 ; i < m_HCuts.size() ; i++ )
    {
	m_HCuts[i].resize( m_hdim, CCut(0.0, CCut::HCut, m_num_wire_per_hcut ) );
    }
    m_VCuts.resize( m_hdim+1 );
    for( unsigned int i = 0 ; i < m_VCuts.size() ; i++ )
    {
	m_VCuts[i].resize( m_vdim, CCut(0.0, CCut::VCut, m_num_wire_per_vcut ) );
    }

}

int CCongMap::GetHBinIndex( const double& x )
{
    int index = static_cast<int>(floor((x-m_left)/m_hlength));
    
    index = max(0,index);
    index = min(m_hdim-1,index);

#if 0
    if( gArg.CheckExist( "nocheck" ) == false )
	if( x < m_left || x > m_right )
	{
	    fprintf( stderr, "Warning: x coordinate %.2f exceeds left %.2f right %.2f\n",
		    x, m_left, m_right );
	}
#endif
    return index;
}

int CCongMap::GetVBinIndex( const double& y )
{
    int index = static_cast<int>(floor((y-m_bottom)/m_vlength));
    
    index = max(0,index);
    index = min(m_vdim-1, index);

#if 0
    if( gArg.CheckExist( "nocheck" ) == false )
	if( y < m_bottom || y > m_top )
	{
	    fprintf( stderr, "Warning: y coordinate %.2f exceeds bottom %.2f top %.2f\n",
		    y, m_bottom, m_top );
	}
#endif
    
    return index;
}


void CCongMap::AddTwoPinNet( const CPoint& p1, const CPoint& p2 )
{
    double bbox_left = min( p1.x, p2.x );
    double bbox_right = max( p1.x, p2.x );
    double bbox_top = max( p1.y, p2.y );
    double bbox_bottom = min( p1.y, p2.y );

    int left_bin_index = GetHBinIndex( bbox_left );
    int right_bin_index = GetHBinIndex( bbox_right );
    int top_bin_index = GetVBinIndex( bbox_top );
    int bottom_bin_index = GetVBinIndex( bbox_bottom );


    //test code
    //double total_cong = static_cast<double>((right_bin_index-left_bin_index)+(top_bin_index-bottom_bin_index));
    //double increased_cong = 0.0;
    //@test code

    //this net do not cross any cuts
    if( (left_bin_index == right_bin_index) && 
	    (top_bin_index == bottom_bin_index ) )
    {
	//Record the number and the length of local nets
	m_numLocalNet[left_bin_index][top_bin_index]++;
	m_lengthLocalNet[left_bin_index][top_bin_index] += CPoint::Distance( p1, p2 );
	
	//test code
	//printf("p1(%f,%f) p2(%f,%f) Distance: %f\n", 
	//	p1.x, p1.y, p2.x, p2.y, CPoint::Distance( p1, p2 ) );
	//@test code
	
	return;
    }
    //this net crosses only vertical bins
    else if( left_bin_index == right_bin_index )
    {
	for( int i = bottom_bin_index+1 ; i <= top_bin_index ; i++ )
	{
	    GetCut( left_bin_index, i, Bottom ).IncreCong(1.0);
	    
	    //test code
	    //increased_cong += 1.0;
	    //@test code
	}
    }
    //this net crosses only horizontal bins
    else if( top_bin_index == bottom_bin_index ) 
    {
	for( int i = left_bin_index+1 ; i <= right_bin_index ; i++ )
	{
	    GetCut( i, top_bin_index, Left ).IncreCong(1.0);
	    
	    //test code
	    //increased_cong += 1.0;
	    //@test code
	}
    }
    else
    {

	CPoint left_point, right_point;

	if( p1.x < p2.x )
	{
	    left_point = p1;
	    right_point = p2;
	}
	else
	{
	    left_point = p2;
	    right_point = p1;
	}

	int horizontal_path_num = right_bin_index - left_bin_index;
	int vertical_path_num = top_bin_index - bottom_bin_index;
	int total_path_num = horizontal_path_num + vertical_path_num;

	int increasing_row_index, decreasing_row_index, increasing_col_index, decreasing_col_index;

	//Add the congestion for the boundary bins
	if( left_point.y < right_point.y )
	{
	    increasing_row_index = top_bin_index;
	    decreasing_row_index = bottom_bin_index;
	    increasing_col_index = right_bin_index;
	    decreasing_col_index = left_bin_index;
	}
	else
	{
	    increasing_row_index = bottom_bin_index;
	    decreasing_row_index = top_bin_index;
	    increasing_col_index = left_bin_index;
	    decreasing_col_index = right_bin_index;
	}

	//bottom boundary 
	for( int i = left_bin_index+1 ; i <= right_bin_index ; i++ )
	{
	    int crossed_path_num = horizontal_path_num - ( i - (left_bin_index + 1) );
	    double cong = static_cast<double>(crossed_path_num)/static_cast<double>(total_path_num);
	    m_VCuts[i][decreasing_row_index].IncreCong(cong);
	    //test code
	    //increased_cong += cong; 
	    //@test code
	}	

	//top boundary
	for( int i = left_bin_index+1 ; i <= right_bin_index ; i++ )
	{
	    int crossed_path_num = i - left_bin_index;
	    double cong = static_cast<double>(crossed_path_num)/static_cast<double>(total_path_num);
	    m_VCuts[i][increasing_row_index].IncreCong(cong);
	    //test code
	    //increased_cong += cong; 
	    //@test code
	}	

	//right boundary
	for( int i = bottom_bin_index+1 ; i <= top_bin_index ; i++ )
	{
	    int crossed_path_num = i - bottom_bin_index;
	    double cong = static_cast<double>(crossed_path_num)/static_cast<double>(total_path_num);
	    m_HCuts[i][increasing_col_index].IncreCong(cong);
	    //test code
	    //increased_cong += cong; 
	    //@test code
	}

	//left boundray
	for( int i = bottom_bin_index+1 ; i <= top_bin_index ; i++ )
	{
	    int crossed_path_num = vertical_path_num - (i - (bottom_bin_index+1));
	    double cong = static_cast<double>(crossed_path_num)/static_cast<double>(total_path_num);
	    m_HCuts[i][decreasing_col_index].IncreCong(cong);
	    //test code
	    //increased_cong += cong; 
	    //@test code
	}


	//Add the congestion for inner bins
	double inner_cong = 1.0/static_cast<double>(total_path_num);
	for( int i = left_bin_index+1 ; i <= right_bin_index ; i++ )
	{
	    for( int j = bottom_bin_index + 1 ; j < top_bin_index ; j++ )
	    {
		m_VCuts[i][j].IncreCong(inner_cong);
		//test code
		//increased_cong += inner_cong; 
		//@test code
	    }
	}

	for( int i = bottom_bin_index+1 ; i <= top_bin_index ; i++ )
	{
	    for( int j = left_bin_index+1 ; j < right_bin_index ; j++ )
	    {
		m_HCuts[i][j].IncreCong(inner_cong);
		//test code
		//increased_cong += inner_cong; 
		//@test code
	    }
	}
    }

    //test code
    //if(abs(increased_cong-total_cong)>0.00001 )
    //{
    //	fprintf( stderr, "Warning: congestion error %f/%f\n", increased_cong, total_cong );
    //}
    //@test code
}

void CCongMap::RemoveTwoPinNet( const CPoint& p1, const CPoint& p2 )
{
    double bbox_left = min( p1.x, p2.x );
    double bbox_right = max( p1.x, p2.x );
    double bbox_top = max( p1.y, p2.y );
    double bbox_bottom = min( p1.y, p2.y );

    int left_bin_index = GetHBinIndex( bbox_left );
    int right_bin_index = GetHBinIndex( bbox_right );
    int top_bin_index = GetVBinIndex( bbox_top );
    int bottom_bin_index = GetVBinIndex( bbox_bottom );

    //this net do not cross any cuts
    if( (left_bin_index == right_bin_index) && 
	    (top_bin_index == bottom_bin_index ) )
    {
	//Record the number and the length of local nets
	m_numLocalNet[left_bin_index][top_bin_index]++;
	m_lengthLocalNet[left_bin_index][top_bin_index] += CPoint::Distance( p1, p2 );
	
	return;
    }
    //this net crosses only vertical bins
    else if( left_bin_index == right_bin_index )
    {
	for( int i = bottom_bin_index+1 ; i <= top_bin_index ; i++ )
	{
	    GetCut( left_bin_index, i, Bottom ).DecreCong(1.0);
	}
    }
    //this net crosses only horizontal bins
    else if( top_bin_index == bottom_bin_index ) 
    {
	for( int i = left_bin_index+1 ; i <= right_bin_index ; i++ )
	{
	    GetCut( i, top_bin_index, Left ).DecreCong(1.0);
	}
    }
    else
    {

	CPoint left_point, right_point;

	if( p1.x < p2.x )
	{
	    left_point = p1;
	    right_point = p2;
	}
	else
	{
	    left_point = p2;
	    right_point = p1;
	}

	int horizontal_path_num = right_bin_index - left_bin_index;
	int vertical_path_num = top_bin_index - bottom_bin_index;
	int total_path_num = horizontal_path_num + vertical_path_num;

	int increasing_row_index, decreasing_row_index, increasing_col_index, decreasing_col_index;

	//Remove the congestion for the boundary bins
	if( left_point.y < right_point.y )
	{
	    increasing_row_index = top_bin_index;
	    decreasing_row_index = bottom_bin_index;
	    increasing_col_index = right_bin_index;
	    decreasing_col_index = left_bin_index;
	}
	else
	{
	    increasing_row_index = bottom_bin_index;
	    decreasing_row_index = top_bin_index;
	    increasing_col_index = left_bin_index;
	    decreasing_col_index = right_bin_index;
	}

	//bottom boundary 
	for( int i = left_bin_index+1 ; i <= right_bin_index ; i++ )
	{
	    int crossed_path_num = horizontal_path_num - ( i - (left_bin_index + 1) );
	    double cong = static_cast<double>(crossed_path_num)/static_cast<double>(total_path_num);
	    m_VCuts[i][decreasing_row_index].DecreCong(cong);
	}	

	//top boundary
	for( int i = left_bin_index+1 ; i <= right_bin_index ; i++ )
	{
	    int crossed_path_num = i - left_bin_index;
	    double cong = static_cast<double>(crossed_path_num)/static_cast<double>(total_path_num);
	    m_VCuts[i][increasing_row_index].DecreCong(cong);
	}	

	//right boundary
	for( int i = bottom_bin_index+1 ; i <= top_bin_index ; i++ )
	{
	    int crossed_path_num = i - bottom_bin_index;
	    double cong = static_cast<double>(crossed_path_num)/static_cast<double>(total_path_num);
	    m_HCuts[i][increasing_col_index].DecreCong(cong);
	}

	//left boundray
	for( int i = bottom_bin_index+1 ; i <= top_bin_index ; i++ )
	{
	    int crossed_path_num = vertical_path_num - (i - (bottom_bin_index+1));
	    double cong = static_cast<double>(crossed_path_num)/static_cast<double>(total_path_num);
	    m_HCuts[i][decreasing_col_index].DecreCong(cong);
	}


	//Remove the congestion for inner bins
	double inner_cong = 1.0/static_cast<double>(total_path_num);
	for( int i = left_bin_index+1 ; i <= right_bin_index ; i++ )
	{
	    for( int j = bottom_bin_index + 1 ; j < top_bin_index ; j++ )
	    {
		m_VCuts[i][j].DecreCong(inner_cong);
	    }
	}

	for( int i = bottom_bin_index+1 ; i <= top_bin_index ; i++ )
	{
	    for( int j = left_bin_index+1 ; j < right_bin_index ; j++ )
	    {
		m_HCuts[i][j].DecreCong(inner_cong);
	    }
	}
    }

}

#if 0
void CCongMap::AddTwoPinNetContribution( const int& i1, const CPoint& p1, const int& i2, const CPoint& p2 )
{
    double bbox_left = min( p1.x, p2.x );
    double bbox_right = max( p1.x, p2.x );
    double bbox_top = max( p1.y, p2.y );
    double bbox_bottom = min( p1.y, p2.y );

    int left_bin_index = GetHBinIndex( bbox_left );
    int right_bin_index = GetHBinIndex( bbox_right );
    int top_bin_index = GetVBinIndex( bbox_top );
    int bottom_bin_index = GetVBinIndex( bbox_bottom );

    //this net do not cross any cuts
    if( (left_bin_index == right_bin_index) && 
	    (top_bin_index == bottom_bin_index ) )
    {
	//Record the number and the length of local nets
	m_numLocalNet[left_bin_index][top_bin_index]++;
	m_lengthLocalNet[left_bin_index][top_bin_index] += CPoint::Distance( p1, p2 );
	
	//test code
	//printf("p1(%f,%f) p2(%f,%f) Distance: %f\n", 
	//	p1.x, p1.y, p2.x, p2.y, CPoint::Distance( p1, p2 ) );
	//@test code
	
	return;
    }
    //this net crosses only vertical bins
    else if( left_bin_index == right_bin_index )
    {
	for( int i = bottom_bin_index+1 ; i <= top_bin_index ; i++ )
	{
	    GetCut( left_bin_index, i, Bottom ).AddContribution( i1, 0.5 );
	    GetCut( left_bin_index, i, Bottom ).AddContribution( i2, 0.5 );
	}
    }
    //this net crosses only horizontal bins
    else if( top_bin_index == bottom_bin_index ) 
    {
	for( int i = left_bin_index+1 ; i <= right_bin_index ; i++ )
	{
	    GetCut( i, top_bin_index, Left ).AddContribution( i1, 0.5 );
	    GetCut( i, top_bin_index, Left ).AddContribution( i2, 0.5 );
	}
    }
    else
    {

	CPoint left_point, right_point;

	if( p1.x < p2.x )
	{
	    left_point = p1;
	    right_point = p2;
	}
	else
	{
	    left_point = p2;
	    right_point = p1;
	}

	int horizontal_path_num = right_bin_index - left_bin_index;
	int vertical_path_num = top_bin_index - bottom_bin_index;
	int total_path_num = horizontal_path_num + vertical_path_num;

	int increasing_row_index, decreasing_row_index, increasing_col_index, decreasing_col_index;

	//Add the congestion for the boundary bins
	if( left_point.y < right_point.y )
	{
	    increasing_row_index = top_bin_index;
	    decreasing_row_index = bottom_bin_index;
	    increasing_col_index = right_bin_index;
	    decreasing_col_index = left_bin_index;
	}
	else
	{
	    increasing_row_index = bottom_bin_index;
	    decreasing_row_index = top_bin_index;
	    increasing_col_index = left_bin_index;
	    decreasing_col_index = right_bin_index;
	}

	//bottom boundary 
	for( int i = left_bin_index+1 ; i <= right_bin_index ; i++ )
	{
	    int crossed_path_num = horizontal_path_num - ( i - (left_bin_index + 1) );
	    double cong = static_cast<double>(crossed_path_num)/static_cast<double>(total_path_num);
	    //m_VCuts[i][decreasing_row_index].IncreCong(cong);
	    m_VCuts[i][decreasing_row_index].AddContribution( i1, cong/2.0 );
	    m_VCuts[i][decreasing_row_index].AddContribution( i2, cong/2.0 );
	}	

	//top boundary
	for( int i = left_bin_index+1 ; i <= right_bin_index ; i++ )
	{
	    int crossed_path_num = i - left_bin_index;
	    double cong = static_cast<double>(crossed_path_num)/static_cast<double>(total_path_num);
	    //m_VCuts[i][increasing_row_index].IncreCong(cong);
	    m_VCuts[i][increasing_row_index].AddContribution( i1, cong/2.0 );
	    m_VCuts[i][increasing_row_index].AddContribution( i2, cong/2.0 );
	}	

	//right boundary
	for( int i = bottom_bin_index+1 ; i <= top_bin_index ; i++ )
	{
	    int crossed_path_num = i - bottom_bin_index;
	    double cong = static_cast<double>(crossed_path_num)/static_cast<double>(total_path_num);
	    //m_HCuts[i][increasing_col_index].IncreCong(cong);
	    m_HCuts[i][increasing_col_index].AddContribution( i1, cong/2.0 );
	    m_HCuts[i][increasing_col_index].AddContribution( i2, cong/2.0 );
	}

	//left boundray
	for( int i = bottom_bin_index+1 ; i <= top_bin_index ; i++ )
	{
	    int crossed_path_num = vertical_path_num - (i - (bottom_bin_index+1));
	    double cong = static_cast<double>(crossed_path_num)/static_cast<double>(total_path_num);
	    //m_HCuts[i][decreasing_col_index].IncreCong(cong);
	    m_HCuts[i][decreasing_col_index].AddContribution( i1, cong/2.0 );
	    m_HCuts[i][decreasing_col_index].AddContribution( i2, cong/2.0 );
	}


	//Add the congestion for inner bins
	double inner_cong = 1.0/static_cast<double>(total_path_num);
	for( int i = left_bin_index+1 ; i <= right_bin_index ; i++ )
	{
	    for( int j = bottom_bin_index + 1 ; j < top_bin_index ; j++ )
	    {
		//m_VCuts[i][j].IncreCong(inner_cong);
		m_VCuts[i][j].AddContribution( i1, inner_cong/2.0 );
		m_VCuts[i][j].AddContribution( i2, inner_cong/2.0 );
	    }
	}

	for( int i = bottom_bin_index+1 ; i <= top_bin_index ; i++ )
	{
	    for( int j = left_bin_index+1 ; j < right_bin_index ; j++ )
	    {
		//m_HCuts[i][j].IncreCong(inner_cong);
		m_HCuts[i][j].AddContribution( i1, inner_cong/2.0 );
		m_HCuts[i][j].AddContribution( i2, inner_cong/2.0 );
	    }
	}
    }

}
#endif

//Replace by path-based congestion map
#if 0
void CCongMap::AddTwoPinNet( const CPoint& p1, const CPoint& p2 )
{
    double bbox_left = min( p1.x, p2.x );
    double bbox_right = max( p1.x, p2.x );
    double bbox_top = max( p1.y, p2.y );
    double bbox_bottom = min( p1.y, p2.y );

    int left_bin_index = GetHBinIndex( bbox_left );
    int right_bin_index = GetHBinIndex( bbox_right );
    int top_bin_index = GetVBinIndex( bbox_top );
    int bottom_bin_index = GetVBinIndex( bbox_bottom );


    //this net do not cross any cuts
    if( (left_bin_index == right_bin_index) && 
	    (top_bin_index == bottom_bin_index ) )
    {
	return;
    }
    //this net crosses vertical bins
    else if( left_bin_index == right_bin_index )
    {
	for( int i = bottom_bin_index+1 ; i <= top_bin_index ; i++ )
	{
	    m_HCuts[i][left_bin_index].IncreCong(1.0);
	}
    }
    //this net crosses horizontal bins
    else if( top_bin_index == bottom_bin_index ) 
    {
	for( int i = left_bin_index+1 ; i <= right_bin_index ; i++ )
	{
	    m_VCuts[i][top_bin_index].IncreCong(1.0);
	}
    }
    else
    {
	double bbox_width = bbox_right - bbox_left;
	double bbox_height = bbox_top - bbox_bottom;

	double left_cong = (m_hlength*(left_bin_index+1)+m_left-bbox_left)/bbox_width;
	double right_cong = (bbox_right - (m_hlength*right_bin_index+m_left))/bbox_width;
	double hmiddle_cong = m_hlength/bbox_width;

	//test code
	double total_cong = 0;

	if( left_cong < 0 || right_cong < 0 ||
		left_cong > 1 || right_cong > 1 )
	{
	    printf("Warning: illegal H congestion (%f,%f)\n", 
		    left_cong, right_cong );
	}
	//@test code

	double top_cong = (bbox_top-(m_vlength*top_bin_index+m_bottom))/bbox_height;
	double bottom_cong = (m_vlength*(bottom_bin_index+1)+m_bottom-bbox_bottom)/bbox_height;
	double vmiddle_cong = m_vlength/bbox_height;

	//test code
	if( top_cong < 0 || bottom_cong < 0 ||
		top_cong > 1 || bottom_cong > 1 )
	{
	    printf("Warning: illegal V congestion (%f,%f)\n",
		    top_cong, bottom_cong );
	}	    
	//@test code

	//Add congestion for HCuts
	//printf("%.3f %.3f %.3f\n", left_cong, hmiddle_cong, right_cong );
	for( int i = bottom_bin_index+1 ; i <= top_bin_index ; i++ )
	{
	    //test code
	    total_cong = 0;
	    //@test code

	    m_HCuts[i][left_bin_index].IncreCong(left_cong);
	    m_HCuts[i][right_bin_index].IncreCong(right_cong);

	    for( int j = left_bin_index+1 ; j < right_bin_index ; j++ )
	    {
		m_HCuts[i][j].IncreCong(hmiddle_cong);

		//test code
		total_cong += hmiddle_cong;
		if( hmiddle_cong < 0 || hmiddle_cong > 1 )
		{
		    printf("Warning: illegal hmiddle congestion %f\n", hmiddle_cong );
		}
		//@test code
	    }

	    //test code
	    total_cong += (left_cong + right_cong );
	    if( total_cong > 1.01 )
	    {
		printf("Warning: illegal total cong %f\n", total_cong );
	    }
	    //@test code
	}

	//Add congestion for VCuts
	for( int i = left_bin_index+1 ; i <= right_bin_index ; i++ )
	{
	    m_VCuts[i][top_bin_index].IncreCong(top_cong);
	    m_VCuts[i][bottom_bin_index].IncreCong(bottom_cong);

	    for( int j = bottom_bin_index+1 ; j < top_bin_index ; j++ )
	    {
		m_VCuts[i][j].IncreCong(vmiddle_cong);

		//test code
		if( vmiddle_cong < 0 || vmiddle_cong > 1 )
		{
		    printf("Warning: illegal vmiddle congestion %f\n", vmiddle_cong );
		}
		//@test code
	    }
	}
    }

}
#endif

double CCongMap::GetBinOverflow( const int& i, const int& j )
{
//    double overflow = m_HCuts[ j ][ i ].GetCong() - m_num_wire_per_hcut +
//                      m_HCuts[ j+1 ][ i ].GetCong() - m_num_wire_per_hcut +
//		      m_VCuts[ i ][ j ].GetCong() - m_num_wire_per_vcut +
//		      m_VCuts[ i+1 ][ j ].GetCong() - m_num_wire_per_vcut;

    double top_overflow = 0.0;
    double bottom_overflow  = 0.0;
    double left_overflow = 0.0;
    double right_overflow = 0.0;
    
    double cut_count = 0.0; 

    //Return overflow with the maximum overflow of each cut
    if( gArg.CheckExist( "maxOverflow" ) )
    {
	double overflow = -numeric_limits<double>::max();
	
	if( i != 0 )
	{
	    //left_overflow = GetCut( i, j, Left ).GetCong() - m_num_wire_per_vcut;
	    left_overflow = GetCut( i, j, Left ).GetOverflow();
	    overflow = max( overflow, left_overflow );
	}
	else if( i != m_hdim - 1 )
	{
	    //right_overflow = GetCut( i, j, Right ).GetCong() - m_num_wire_per_vcut;
	    right_overflow = GetCut( i, j, Right ).GetOverflow();
	    overflow = max( overflow, right_overflow );
	}

	if( j != 0 )
	{
	    //bottom_overflow = GetCut( i, j, Bottom ).GetCong() - m_num_wire_per_hcut;
	    bottom_overflow = GetCut( i, j, Bottom ).GetOverflow();
	    overflow = max( overflow, bottom_overflow );
	}
	else if( j != m_vdim - 1 )
	{
	    //top_overflow = GetCut( i, j, Top ).GetCong() - m_num_wire_per_hcut;
	    top_overflow = GetCut( i, j, Top ).GetOverflow();
	    overflow = max( overflow, top_overflow );
	}
	
	return overflow;
    }

    
    if( i != 0 )
    {
	//left_overflow = GetCut( i, j, Left ).GetCong() - m_num_wire_per_vcut;
	left_overflow = GetCut( i, j, Left ).GetOverflow();
	cut_count += 1.0;
    }
    else if( i != m_hdim - 1 )
    {
	//right_overflow = GetCut( i, j, Right ).GetCong() - m_num_wire_per_vcut;
	right_overflow = GetCut( i, j, Right ).GetOverflow();
	cut_count += 1.0;
    }

    if( j != 0 )
    {
	//bottom_overflow = GetCut( i, j, Bottom ).GetCong() - m_num_wire_per_hcut;
	bottom_overflow = GetCut( i, j, Bottom ).GetOverflow();
	cut_count += 1.0;
    }
    else if( j != m_vdim - 1 )
    {
	//top_overflow = GetCut( i, j, Top ).GetCong() - m_num_wire_per_hcut;
	top_overflow = GetCut( i, j, Top ).GetOverflow();
	cut_count += 1.0;
    }
    
    double overflow = (top_overflow + bottom_overflow + left_overflow + right_overflow) / cut_count;

    return overflow;
}

double CCongMap::GetBinCongestion( const int& i, const int& j )
{
    double top_congestion = 0.0;
    double bottom_congestion  = 0.0;
    double left_congestion = 0.0;
    double right_congestion = 0.0;
    
    double cut_count = 0.0; 
    
    if( i != 0 )
    {
	left_congestion = GetCut( i, j, Left ).GetCong();
	cut_count += 1.0;
    }
    else if( i != m_hdim - 1 )
    {
	right_congestion = GetCut( i, j, Right ).GetCong();
	cut_count += 1.0;
    }

    if( j != 0 )
    {
	bottom_congestion = GetCut( i, j, Bottom ).GetCong();
	cut_count += 1.0;
    }
    else if( j != m_vdim - 1 )
    {
	top_congestion = GetCut( i, j, Top ).GetCong();
	cut_count += 1.0;
    }
    
    double congestion = (top_congestion + bottom_congestion + left_congestion + right_congestion) / cut_count;

    return congestion;
}

double CCongMap::GetTotalOverflow(void)
{
    double overflow = 0.0;

    for( unsigned int i = 0 ; i < m_HCuts.size() ; i++ )
    {
	for( unsigned int j = 0 ; j < m_HCuts[i].size() ; j++ )
	{
	    //if( m_HCuts[i][j].GetCong() > m_num_wire_per_hcut )
	    if( m_HCuts[i][j].GetOverflow() > 0.0 )
	    {
		//overflow += (m_HCuts[i][j].GetCong() - m_num_wire_per_hcut);
		overflow += m_HCuts[i][j].GetOverflow();
	    }
	}
    }

    for( unsigned int i = 0 ; i < m_VCuts.size() ; i++ )
    {
	for( unsigned int j = 0 ; j < m_VCuts[i].size() ; j++ )
	{
	    //if( m_VCuts[i][j].GetCong() > m_num_wire_per_vcut )
	    if( m_VCuts[i][j].GetOverflow() > 0.0 )
	    {
		//overflow += ( m_VCuts[i][j].GetCong() - m_num_wire_per_vcut );
		overflow += m_VCuts[i][j].GetOverflow();
	    }
	}
    }

    return overflow;
 
}

double CCongMap::GetMaxOverflow(void)
{
    //double max_overflow = m_HCuts[0][0].GetCong() - m_num_wire_per_hcut;
    double max_overflow = m_HCuts[0][0].GetOverflow();

    for( unsigned int i = 0 ; i < m_HCuts.size() ; i++ )
    {
	for( unsigned int j = 0 ; j < m_HCuts[i].size() ; j++ )
	{
	    //max_overflow = max( max_overflow, m_HCuts[i][j].GetCong() - m_num_wire_per_hcut );
	    max_overflow = max( max_overflow, m_HCuts[i][j].GetOverflow() );
	}
    }

    for( unsigned int i = 0 ; i < m_VCuts.size() ; i++ )
    {
	for( unsigned int j = 0 ; j < m_VCuts[i].size() ; j++ )
	{
	    //max_overflow = max( max_overflow, m_VCuts[i][j].GetCong() - m_num_wire_per_vcut );
	    max_overflow = max( max_overflow, m_VCuts[i][j].GetOverflow() );
	}
    }
   
    return max_overflow; 
}

double CCongMap::GetTwoPinNetMaxOverflow( const CPoint& p1, const CPoint& p2 )
{
    double bbox_left = min( p1.x, p2.x );
    double bbox_right = max( p1.x, p2.x );
    double bbox_top = max( p1.y, p2.y );
    double bbox_bottom = min( p1.y, p2.y );

    int left_bin_index = GetHBinIndex( bbox_left );
    int right_bin_index = GetHBinIndex( bbox_right );
    int top_bin_index = GetVBinIndex( bbox_top );
    int bottom_bin_index = GetVBinIndex( bbox_bottom );
   
    // (donnie) In the same tile 
    if( left_bin_index == right_bin_index &&
	    bottom_bin_index == top_bin_index )
    {
	return 0.0;
    }

    //double max_overflow = GetCut( left_bin_index, bottom_bin_index, Right ).GetCong() - m_num_wire_per_vcut;
    double max_overflow = GetCut( left_bin_index, bottom_bin_index, Right ).GetOverflow();

    for( int i = left_bin_index ; i <= right_bin_index  ; i++ )
    {
	for( int j = bottom_bin_index ; j < top_bin_index ; j++ )
	{
	    //max_overflow = max( max_overflow, 
	    //	    GetCut( i, j, Top ).GetCong() - m_num_wire_per_hcut );
	    max_overflow = max( max_overflow, GetCut( i, j, Top ).GetOverflow() );
	}
    }

    for( int i = left_bin_index ; i < right_bin_index ; i++ )
    {
	for( int j = bottom_bin_index ; j <= top_bin_index ; j++ )
	{
	    //max_overflow = max( max_overflow, 
	    //	    GetCut( i, j, Right ).GetCong() - m_num_wire_per_vcut );
	    max_overflow = max( max_overflow, GetCut( i, j, Right ).GetOverflow() );
	}
    }
   
    return max_overflow; 
}

void CCongMap::AddBlockageByRatio( const CPoint& p1, const CPoint& p2, const double& blockage_ratio )
{
    double bbox_left = min( p1.x, p2.x );
    double bbox_right = max( p1.x, p2.x );
    double bbox_top = max( p1.y, p2.y );
    double bbox_bottom = min( p1.y, p2.y );

    int left_bin_index = GetHBinIndex( bbox_left );
    int right_bin_index = GetHBinIndex( bbox_right );
    int top_bin_index = GetVBinIndex( bbox_top );
    int bottom_bin_index = GetVBinIndex( bbox_bottom );

    //Blockage do not cross any cuts
    if( left_bin_index == right_bin_index &&
	    top_bin_index == bottom_bin_index )
    {
	return;
    }

    //Add blockage ratio to all inner cuts
    for( int i = left_bin_index ; i < right_bin_index ; i++ )
    {
	for( int j = bottom_bin_index + 1 ; j < top_bin_index ; j++ )
	{
	    GetCut( i, j, Right ).IncreBlockageRatio( blockage_ratio );
	    //printf( "(%d,%d) Right %f\n", i, j, blockage_ratio );
	}
    }
    for( int i = left_bin_index + 1 ; i < right_bin_index ; i++ )
    {
	for( int j = bottom_bin_index ; j < top_bin_index ; j++ )
	{
	    GetCut( i, j, Top ).IncreBlockageRatio( blockage_ratio );
	    //printf( "(%d,%d) Top %f\n", i, j, blockage_ratio );
	}
    }

    //Add blockage ratio to all boundary cuts
    //case 1: top and bottom boundaries, vertical cut
    
    //Compute real blockage ratio for top and bottom boundaries
    double bottom_base_ycoor = m_bottom + ( m_vlength * ( bottom_bin_index + 1 ) );
    double bottom_overlap_ratio = ( bottom_base_ycoor - bbox_bottom ) / m_vlength;
    double bottom_blockage_ratio = blockage_ratio * bottom_overlap_ratio;

    double top_base_ycoor = m_bottom + ( m_vlength * top_bin_index );
    double top_overlap_ratio = ( bbox_top - top_base_ycoor ) / m_vlength;
    double top_blockage_ratio = blockage_ratio * top_overlap_ratio;
    
    //Add blockage ratio for top and bottom boundaries
    for( int i = left_bin_index ; i < right_bin_index ; i++ )
    {
	GetCut( i, bottom_bin_index, Right ).IncreBlockageRatio( bottom_blockage_ratio );
	GetCut( i, top_bin_index, Right ).IncreBlockageRatio( top_blockage_ratio );
	//printf( "(%d,%d) Right %f\n", i, bottom_bin_index, bottom_blockage_ratio );
	//printf( "(%d,%d) Right %f\n", i, top_bin_index, top_blockage_ratio );
    }
    
    //case 2: left and right boundaries, horizontal cut
    double left_base_xcoor = m_left + ( m_hlength * ( left_bin_index + 1 ) );
    double left_overlap_ratio = ( left_base_xcoor - bbox_left ) / m_hlength;
    double left_blockage_ratio = blockage_ratio * left_overlap_ratio;

    double right_base_xcoor = m_left + ( m_hlength * right_bin_index );
    double right_overlap_ratio = ( bbox_right - right_base_xcoor ) / m_hlength;
    double right_blockage_ratio = blockage_ratio * right_overlap_ratio;
    
    for( int j = bottom_bin_index ; j < top_bin_index ; j++ )
    {
	GetCut( left_bin_index, j, Top ).IncreBlockageRatio( left_blockage_ratio );
	GetCut( right_bin_index, j, Top ).IncreBlockageRatio( right_blockage_ratio );
	//printf( "(%d,%d) Top %f\n", left_bin_index, j, left_blockage_ratio );
	//printf( "(%d,%d) Top %f\n", right_bin_index, j, right_blockage_ratio );
    }
}

    
double CCongMap::GetMaxHCutCongestion(void)
{
    double max_congestion = m_HCuts[0][0].GetCong();

    for( unsigned int i = 0 ; i < m_HCuts.size() ; i++ )
    {
	for( unsigned int j = 0 ; j < m_HCuts[i].size() ; j++ )
	{
	    max_congestion = max( max_congestion, m_HCuts[i][j].GetCong() );
	}
    }

    return max_congestion/static_cast<double>(m_hcut_layer_count);
}

double CCongMap::GetMaxVCutCongestion(void)
{
    double max_congestion = m_VCuts[0][0].GetCong();
    
    for( unsigned int i = 0 ; i < m_VCuts.size() ; i++ )
    {
	for( unsigned int j = 0 ; j < m_VCuts[i].size() ; j++ )
	{
	    max_congestion = max( max_congestion, m_VCuts[i][j].GetCong() );
	}
    }

    return max_congestion/static_cast<double>(m_vcut_layer_count);
}

double CCongMap::GetMaxCongestion(void)
{
    return max( GetMaxVCutCongestion(), GetMaxHCutCongestion() ); 
}

CCut& CCongMap::GetCut( const int& i, const int& j, const CutPosition& pos )
{
    if( pos == Top )
    {
	return m_HCuts[ j+1 ][ i ];
    }
    else if( pos == Bottom )
    {
	return m_HCuts[ j ][ i ];
    }
    else if( pos == Left )
    {
	return m_VCuts[ i ][ j ];
    }
    else if( pos == Right )
    {
	return m_VCuts[ i+1 ][ j ];
    }
    else
    {
	fprintf( stderr, "Warning: Undifined cut position in GetCut()\n" );
	return m_VCuts[i][j];
    }
}

void CCongMap::clear(void)
{
    for( unsigned int i = 0 ; i < m_HCuts.size() ; i++ )
    {
	for( unsigned int j = 0 ; j < m_HCuts[i].size() ; j++ )
	{
	    m_HCuts[i][j].clear();
	}
    }

    for( unsigned int i = 0 ; i < m_VCuts.size() ; i++ )
    {
	for( unsigned int j = 0 ; j < m_VCuts[i].size() ; j++ )
	{
	    m_VCuts[i][j].clear();
	}
    }

}


void CCongMap::AddTwoPinNetByUpperLowerL( const CPoint& p1, const CPoint& p2 )
{
    double bbox_left = min( p1.x, p2.x );
    double bbox_right = max( p1.x, p2.x );
    double bbox_top = max( p1.y, p2.y );
    double bbox_bottom = min( p1.y, p2.y );

    int left_bin_index = GetHBinIndex( bbox_left );
    int right_bin_index = GetHBinIndex( bbox_right );
    int top_bin_index = GetVBinIndex( bbox_top );
    int bottom_bin_index = GetVBinIndex( bbox_bottom );
    
    //test code
    double total_cong = static_cast<double>((right_bin_index-left_bin_index)+(top_bin_index-bottom_bin_index));
    double increased_cong = 0.0;
    //@test code

    //this net do not cross any cuts
    if( (left_bin_index == right_bin_index) && 
	    (top_bin_index == bottom_bin_index ) )
    {
	return;
    }
    //this net crosses only vertical bins
    else if( left_bin_index == right_bin_index )
    {
	for( int i = bottom_bin_index+1 ; i <= top_bin_index ; i++ )
	{
	    GetCut( left_bin_index, i, Bottom ).IncreCong(1.0);
	    //test code
	    increased_cong += 1.0;
	    //@test code
	}
    }
    //this net crosses only horizontal bins
    else if( top_bin_index == bottom_bin_index ) 
    {
	for( int i = left_bin_index+1 ; i <= right_bin_index ; i++ )
	{
	    GetCut( i, top_bin_index, Left ).IncreCong(1.0);
	    //test code
	    increased_cong += 1.0;
	    //@test code
	}
    }
    else
    {
	//For top and bottom boundaries
	for( int i = left_bin_index ; i < right_bin_index ; i++ )
	{
	    GetCut( i, top_bin_index, Right ).IncreCong( 0.5 );
	    GetCut( i, bottom_bin_index, Right ).IncreCong( 0.5 );
	    //test code
	    increased_cong += 1.0;
	    //@test code
	}

	//For left and 
	for( int j = bottom_bin_index ; j < top_bin_index ; j++ )
	{
	    GetCut( left_bin_index, j, Top ).IncreCong( 0.5 );
	    GetCut( right_bin_index, j, Top ).IncreCong( 0.5 ); 
	    //test code
	    increased_cong += 1.0;
	    //@test code
	}
    }
    
    //test code
    if(abs(increased_cong-total_cong)>0.00001 )
    {
	fprintf( stderr, "Warning: congestion error %f/%f\n", increased_cong, total_cong );
    }
    //@test code
}

void CCongMap::OutputBinOverflowFigure( const std::string& filename )
{
    string pltname = filename + ".plt";
    string datname = "dat/" + pltname + ".dat";

    FILE* pPlt = fopen( pltname.c_str(), "w" );

    fprintf( pPlt, "set title \" %s \" font \"Times, 22\"\n", pltname.c_str() );
    
    fprintf( pPlt, "set pm3d map\n");
    fprintf( pPlt, "set size ratio 1\n" );
    fprintf( pPlt, "set cbrange [-10:4]\n" );
    fprintf( pPlt, "set xrange [0:%d]\n", m_hdim - 1 );
    fprintf( pPlt, "set yrange [0:%d]\n", m_vdim - 1 );
    //fprintf( pPlt, "set palette model RGB defined ( -10 \"green\", -8 \"dark-green\","
    //	           " -6 \"yellow\", -4 \"dark-yellow\", -2 \"blue\", "
    //		   "0 \"dark-blue\", 0 \"red\", 4 \"dark-red\" )\n" );
    //fprintf( pPlt, "set palette model RGB defined ( -10 \"green\", -2 \"dark-green\","
    //	           " -2 \"yellow\", -0 \"dark-yellow\", 0 \"blue\", "
    //		   "2 \"dark-blue\", 2 \"red\", 4 \"dark-red\" )\n" );
    fprintf( pPlt, "set palette model RGB defined ( -10 \"white\", -4 \"black\", "
	           "-4 \"dark-yellow\", -2 \"yellow\", -2 \"green\", 0 \"green\", 0 \"blue\", "
		   "2 \"dark-blue\", 2 \"dark-red\", 4 \"dark-red\" )\n" );
    fprintf( pPlt, "splot '%s' matrix with pm3d\n", datname.c_str() );
    fprintf( pPlt, "pause -1\n" );
    
    fclose( pPlt );
    
    FILE* OverflowData = fopen( datname.c_str(), "w" );

    for( int j = 0 ; j < m_vdim ; j++ )
    {
	for( int i = 0 ; i < m_hdim ; i++ )
	{
	    double overflow = GetBinOverflow( i, j );
	    fprintf( OverflowData, "%6.2f ", overflow );		
	}
	fprintf( OverflowData, "\n" );
    }

    fclose( OverflowData ); 

    // Reduce output in NLPlace (donnie)
    //if( param.bShow )
    //	fprintf( stdout, "Generated %s\n", pltname.c_str() );

    if( gArg.CheckExist( "congHVplt" ) )
	OutputHVCutOverflowFigure( filename );  // 2007-02-16 (donnie)
}

// 2007-02-16 (donnie)
void CCongMap::OutputHVCutOverflowFigure( const string& filename )
{

    vector< vector< double > >  hCut, vCut;
    string filename1 = filename + ".h";
    string filename2 = filename + ".v";
    char title1 [255];
    char title2 [255];

    //Modified by Jin 20070328
    //sprintf( title1, "H Cut Capacity = %.2f", m_num_wire_per_hcut );
    //sprintf( title2, "V Cut Capacity = %.2f", m_num_wire_per_vcut );
    sprintf( title1, "H Cut Capacity = %.2f", m_num_wire_per_hcut );
    sprintf( title2, "V Cut Capacity = %.2f", m_num_wire_per_vcut );

    hCut.resize( m_HCuts.size() );
    for( unsigned int i = 0 ; i < m_HCuts.size() ; i++ )
    {
	hCut[i].resize( m_HCuts[i].size() );
	for( unsigned int j = 0 ; j < m_HCuts[i].size() ; j++ )
	{
	    hCut[i][j] = m_HCuts[i][j].GetCong();
	}
    }

    //Modified by Jin 20070328
    //CMatrixPlotter::OutputGnuplotFigure( hCut, filename1.c_str(), title1, m_num_wire_per_hcut );
    CMatrixPlotter::OutputGnuplotFigure( hCut, filename1.c_str(), title1, m_num_wire_per_hcut );

    vCut.resize( m_VCuts.size() );
    for( unsigned int i = 0 ; i < m_VCuts.size() ; i++ )
    {
	vCut[i].resize( m_VCuts[i].size() );
	for( unsigned int j = 0 ; j < m_VCuts[i].size() ; j++ )
	{
	    vCut[i][j] = m_VCuts[i][j].GetCong();
	}
    }
    
    //Modified by Jin 20070328
    //CMatrixPlotter::OutputGnuplotFigure( hCut, filename2.c_str(), title2, m_num_wire_per_vcut );
    CMatrixPlotter::OutputGnuplotFigure( hCut, filename2.c_str(), title2, m_num_wire_per_vcut );
   
}

void CCongMap::OutputCutOverflowFigure( const string& filename )
{
    string pltname = filename + ".plt";
    FILE* pPlt = fopen( pltname.c_str(), "w" );

    fprintf( pPlt, "set title \" %s \" font \"Times, 22\"\n"
	           "set size ratio 1\n"
		   "set nokey\n\n", pltname.c_str() ); 
    
    fprintf( pPlt, "plot[:][:] '-' w l lt 4, "
	    //Specify the congestion colors
	    "'-' w l lt 3, '-' w l lt 2, '-' w l lt 8, '-' w l lt 1, '-' w l lt 4, '-' w l lt 5\n\n" ); 

    fprintf( pPlt, "\n# core region\n" );
    fprintf( pPlt, "%12.3f, %12.3f\n", m_left, m_bottom );
    fprintf( pPlt, "%12.3f, %12.3f\n", m_right, m_bottom );
    fprintf( pPlt, "%12.3f, %12.3f\n", m_right, m_top );
    fprintf( pPlt, "%12.3f, %12.3f\n", m_left, m_top );
    fprintf( pPlt, "%12.3f, %12.3f\n\n", m_left, m_bottom );

    fprintf( pPlt, "\nEOF\n\n" ); 

    vector<CPoint> cong[6];

    //Computation for horizontal cuts
    for( unsigned int i = 0 ; i < m_HCuts.size() ; i++ )
    {
	for( unsigned int j = 0 ; j < m_HCuts[i].size() ; j++ )
	{
	    double ycoor = m_bottom + (i*m_vlength);
	    double xleftcoor = m_left + (j*m_hlength);

	    CPoint p1(xleftcoor, ycoor );
	    CPoint p2(xleftcoor+m_hlength, ycoor);

	    if( m_HCuts[i][j].GetOverflow() >= 4.0 )
	    {
		cong[5].push_back(p1);
		cong[5].push_back(p2);
	    }
	    else if( m_HCuts[i][j].GetOverflow() >= 3.0 ) 
	    {
		cong[4].push_back(p1);
		cong[4].push_back(p2);
	    }
	    else if( m_VCuts[i][j].GetOverflow() >= 2.0 )
	    {
		cong[3].push_back(p1);
		cong[3].push_back(p2);
	    }
	    else if( m_HCuts[i][j].GetOverflow() >= 1.0 )
	    {
		cong[2].push_back(p1);
		cong[2].push_back(p2);
	    }
	    else if( m_HCuts[i][j].GetOverflow() >= 0.0 ) 
	    {
		cong[1].push_back(p1);
		cong[1].push_back(p2);
	    }
	    else if( m_VCuts[i][j].GetOverflow() >= -1.0 )
	    {
		//cout << "k3" << endl;
		cong[0].push_back(p1);
		cong[0].push_back(p2);
	    }

	}
    }

    //Computation for vertical cuts
    for( unsigned int i = 0 ; i < m_VCuts.size() ; i++ )
    {
	for( unsigned int j = 0 ; j < m_VCuts[i].size() ; j++ )
	{
	    double xcoor = m_left + (i*m_hlength);
	    double ybottomcoor = m_bottom + (j*m_vlength);

	    CPoint p1(xcoor, ybottomcoor );
	    CPoint p2(xcoor, ybottomcoor + m_vlength );

	    if( m_VCuts[i][j].GetOverflow() > 4.0 )
	    {
		cong[5].push_back(p1);
		cong[5].push_back(p2);
	    }
	    else if( m_VCuts[i][j].GetOverflow() > 3.0 ) 
	    {
		cong[4].push_back(p1);
		cong[4].push_back(p2);
	    }
	    else if( m_VCuts[i][j].GetOverflow() > 2.0 )
	    {
		cong[3].push_back(p1);
		cong[3].push_back(p2);
	    }

	    else if( m_VCuts[i][j].GetOverflow() > 1.0 )
	    {
		cong[2].push_back(p1);
		cong[2].push_back(p2);
	    }
	    else if( m_VCuts[i][j].GetOverflow() > 0.0 ) 
	    {
		cong[1].push_back(p1);
		cong[1].push_back(p2);
	    }
	    else if( m_VCuts[i][j].GetOverflow() > -1.0 )
	    {
		cong[0].push_back(p1);
		cong[0].push_back(p2);
	    }
	}
    }



    fprintf(pPlt, "#Congestion lines\n");

    for( int i = 0 ; i < 6 ; i++ )
    {
	fprintf(pPlt, "0, 0\n\n");

	for( unsigned int j = 0 ; j < cong[i].size() ; j=j+2 )
	{
	    fprintf( pPlt, "%12.3f, %12.3f\n%12.3f %12.3f\n\n",
		    cong[i][j].x, cong[i][j].y,
		    cong[i][j+1].x, cong[i][j+1].y );
	}

	fprintf(pPlt, "\nEOF\n\n");
    }

    fprintf( pPlt, "pause -1 'Press any key'\n" );
    
    fclose( pPlt );

}

void CCongMap::OutputBinLocalNetLengthFigure( const std::string& filename )
{
    string pltname = filename + ".plt";
    string datname = "dat/" + pltname + ".dat";
    
    //Compute value of each drawing level 
    double mean = m_maxBinLength;
    double delta = 0.1 * mean;
   
    FILE* pPlt = fopen( pltname.c_str(), "w" );

    fprintf( pPlt, "set title \" %s \" font \"Times, 22\"\n", pltname.c_str() );
    
    fprintf( pPlt, "set pm3d map\n");
    fprintf( pPlt, "set size ratio 1\n" );
    fprintf( pPlt, "set cbrange [%8.2f:%8.2f]\n", mean - 5.0*delta, mean + 2.0*delta );
    fprintf( pPlt, "set xrange [0:%d]\n", m_hdim - 1 );
    fprintf( pPlt, "set yrange [0:%d]\n", m_vdim - 1 );
    fprintf( pPlt, "set palette model RGB defined ( %8.2f \"green\", %8.2f \"dark-green\","
	           " %8.2f \"yellow\", %8.2f \"dark-yellow\", %8.2f \"blue\", "
		   " %8.2f \"dark-blue\", %8.2f \"red\", %8.2f \"dark-red\" )\n",
		   mean - 5.0*delta, mean - 4.0*delta, 
		   mean - 3.0*delta, mean - 2.0*delta, mean - delta,
		   mean, mean + delta, mean + 2.0*delta );
    fprintf( pPlt, "splot '%s' matrix with pm3d\n", datname.c_str() );
    fprintf( pPlt, "pause -1\n" );
    
    fclose( pPlt );
    
    FILE* LengthData = fopen( datname.c_str(), "w" );

    for( int j = 0 ; j < m_vdim ; j++ )
    {
	for( int i = 0 ; i < m_hdim ; i++ )
	{
	    fprintf( LengthData, "%8.2f ", m_lengthLocalNet[i][j] );		
	}
	fprintf( LengthData, "\n" );
    }

    fclose( LengthData ); 

    if( param.bShow )
    {
	fprintf( stdout, "Generated %s\n", pltname.c_str() );
    }
}


void CCongMap::OutputBinTotalLengthFigure( const std::string& filename )
{
    string pltname = filename + ".plt";
    string datname = "dat/" + pltname + ".dat";
    
    //Compute the value of each drawing level
    double mean = m_maxBinLength;
    double delta = 0.1 * mean;
   
    FILE* pPlt = fopen( pltname.c_str(), "w" );

    fprintf( pPlt, "set title \" %s \" font \"Times, 22\"\n", pltname.c_str() );
    
    fprintf( pPlt, "set pm3d map\n");
    fprintf( pPlt, "set size ratio 1\n" );
    //fprintf( pPlt, "set cbrange [%8.2f:%8.2f]\n", mean - 5.0*delta, mean + 2.0*delta );
    fprintf( pPlt, "set cbrange [0:%8.2f]\n", mean + 2.0*delta );
    fprintf( pPlt, "set xrange [0:%d]\n", m_hdim - 1 );
    fprintf( pPlt, "set yrange [0:%d]\n", m_vdim - 1 );
    //fprintf( pPlt, "set palette model RGB defined ( %8.2f \"green\", %8.2f \"dark-green\","
    //	           " %8.2f \"yellow\", %8.2f \"dark-yellow\", %8.2f \"blue\", "
    //		   " %8.2f \"dark-blue\", %8.2f \"red\", %8.2f \"dark-red\" )\n",
    //		   mean - 5.0*delta, mean - 4.0*delta, 
    //		   mean - 3.0*delta, mean - 2.0*delta, mean - delta,
    //		   mean, mean + delta, mean + 2.0*delta );
    fprintf( pPlt, "set palette model RGB defined ( 0 \"green\", 1 \"dark-green\","
	           " 2 \"yellow\", 3 \"dark-yellow\", 4 \"blue\", "
		   " 5 \"dark-blue\", 6 \"red\", 7 \"dark-red\" )\n" );
    fprintf( pPlt, "splot '%s' matrix with pm3d\n", datname.c_str() );
    fprintf( pPlt, "pause -1\n" );
    
    fclose( pPlt );
   

    //Output data file of total length 
    FILE* LengthData = fopen( datname.c_str(), "w" );
    //const double average_global_net_length = (m_hlength + m_vlength)/2.0;

    for( int j = 0 ; j < m_vdim ; j++ )
    {
	for( int i = 0 ; i < m_hdim ; i++ )
	{
	    //double num_global_nets = GetBinTotalCongestion( i, j );
	    double total_global_hlength = ( GetCut( i, j, Left ).GetCong() + GetCut( i, j, Right ).GetCong() ) 
		                   * ( m_hlength * 0.5 );
	    double total_global_vlength = ( GetCut( i, j, Top ).GetCong() + GetCut( i, j, Bottom ).GetCong() )
				   * ( m_vlength * 0.5 );
	    //double total_length = num_global_nets * average_global_net_length 
	    //	+ m_lengthLocalNet[i][j];	
	    double total_length = total_global_hlength + total_global_vlength + m_lengthLocalNet[i][j];	
	    fprintf( LengthData, "%8.2f ", total_length );		
	}
	fprintf( LengthData, "\n" );
    }

    fclose( LengthData ); 

    if( param.bShow )
    {
	fprintf( stdout, "Generated %s\n", pltname.c_str() );
    }
}

double CCongMap::GetBinTotalCongestion( const int& i, const int& j )
{
    double cong = GetCut( i, j, Top ).GetCong() + 
		  GetCut( i, j, Bottom ).GetCong() +
		  GetCut( i, j, Left ).GetCong() +
		  GetCut( i, j, Right ).GetCong();

    return cong;
}

double CCongMap::GetMaxBinTotalLength(void)
{

    double max_length = m_lengthLocalNet[0][0];

    for( int j = 0 ; j < m_vdim ; j++ )
    {
	for( int i = 0 ; i < m_hdim ; i++ )
	{
	    max_length = max( max_length, m_lengthLocalNet[i][j] );
	}
    }

    return max_length;

}


// 2006-02-06 (donnie)
int GetRegionNetFluxOverflow( const CPlaceDB* pDB, const double& x1, const double& y1, const double& x2, const double& y2 )
{
    set<int> netsInsideRegion;

    // collect nets inside the region
    for( unsigned int mid=0; mid<pDB->m_modules.size(); mid++ )
    {
	double mX1 = pDB->m_modules[mid].m_x;
	double mY1 = pDB->m_modules[mid].m_y;
	double mX2 = pDB->m_modules[mid].m_x + pDB->m_modules[mid].m_width;
	double mY2 = pDB->m_modules[mid].m_y + pDB->m_modules[mid].m_height;
	
	// module inside the region?
	if( mX2 >= x1 && mX1 <= x2 && mY2 >= y1 && mY1 <= y2 )
	{
	    for( unsigned net=0; net<pDB->m_modules[mid].m_netsId.size(); net++ )
		netsInsideRegion.insert( pDB->m_modules[mid].m_netsId[net] );
	}
    } // for each module

    int flux = 0;

    // for each net
    set<int>::const_iterator ite;
    for( ite=netsInsideRegion.begin(); ite!=netsInsideRegion.end(); ite++ )
    {
	int numPinInside = 0;
	int numPinOutside = 0;
	
	for( unsigned int p=0; p<pDB->m_nets[*ite].size(); p++ )
	{
	    double pinX, pinY;
	    int pinId = pDB->m_nets[*ite][p];
	    pDB->GetPinLocation( pinId, pinX, pinY );
	    if( pinX >= x1 && pinX <= x2 && pinY >= y1 && pinY <= y2 )
		numPinInside++;
	    else
		numPinOutside++;
	}

	if( numPinInside > 0 && numPinOutside > 0 )
	    flux++;
    }

    return flux;
}


void CCongOptimizer::OptimizeCongestion( const int& i, const int& j, const CCongMap::CutPosition& pos )
{
    if( ( i == 0 && pos == CCongMap::Left ) ||
	( i == m_hdim - 1 && pos == CCongMap::Right ) ||
	( j == 0 && pos == CCongMap::Bottom ) ||
	( j == m_vdim - 1 && pos == CCongMap::Top ) ||
	( i < 0 ) ||
	( i >= m_hdim ) ||
	( j < 0 ) ||
	( j >= m_vdim ) )
    {
	fprintf( stderr, "Warning: incorrect specified cut i: %d j: %d pos: %d\n",
		i, j, pos );
	return;
    }

    //Perform global routing
    CGlobalRouting groute( m_placedb );
    groute.ProbalisticMethod( m_congmap );

    //Make the first parition (part[0]) the left/bottom one
    int first_xindex = i;
    int first_yindex = j;
    int second_xindex = i;
    int second_yindex = j;

    if( pos == CCongMap::Top )
	second_yindex++;
    else if( pos == CCongMap::Bottom )
	first_yindex--;
    else if( pos == CCongMap::Left )
	first_xindex--;
    else if( pos == CCongMap::Right )
       	second_xindex++;
    else
	fprintf( stderr, "Warning: incorrect cut direction %d\n", pos );
    
    double bin_width = m_congmap.m_hlength;
    double bin_height = m_congmap.m_vlength;

    CRect first_bbox, second_bbox;
    first_bbox.left = m_congmap.m_left + ( bin_width * first_xindex );
    first_bbox.right = m_congmap.m_left + bin_width;
    first_bbox.bottom = m_congmap.m_bottom + ( bin_height * first_yindex );
    first_bbox.top = first_bbox.bottom + bin_height;
    second_bbox.left = m_congmap.m_left + ( bin_width * second_xindex );
    second_bbox.right = m_congmap.m_left + bin_width;
    second_bbox.bottom = m_congmap.m_bottom + ( bin_height * second_yindex );
    second_bbox.top = second_bbox.bottom + bin_height;
    
    CCongPart part[] = { CCongPart( first_xindex, first_yindex, first_bbox ), 
			 CCongPart( second_xindex, second_yindex, second_bbox ) };

    //Add corresponding modules to two partitions
    for( unsigned int i = 0 ; i < m_placedb.m_modules.size() ; i++ )
    {
	const Module& curModule = m_placedb.m_modules[i];

	int mod_xindex = m_congmap.GetHBinIndex( curModule.m_cx );
	int mod_yindex = m_congmap.GetVBinIndex( curModule.m_cy );

	if( mod_xindex == part[0].m_xindex && mod_yindex == part[0].m_yindex )
	    part[0].AddModuleId( i );
	else if( mod_xindex == part[1].m_xindex && mod_yindex == part[1].m_yindex )
	    part[1].AddModuleId( i );
    }

    //test code
    fprintf( stdout, "part[0]: (%d)\n", part[0].m_module_ids.size() );
    for( unsigned int i = 0 ; i < part[0].m_module_ids.size() ; i++ )
    {
	fprintf( stdout, "%d ", part[0].m_module_ids[i] );
    }
    fprintf( stdout, "\n" );
    fprintf( stdout, "part[1]: (%d)\n", part[1].m_module_ids.size() );
    for( unsigned int i = 0 ; i < part[1].m_module_ids.size() ; i++ )
    {
	fprintf( stdout, "%d ", part[1].m_module_ids[i] );
    }
    fprintf( stdout, "\n" );
    exit(0);
    //@test code
}

#if 0
void CCut::AddContribution( const int& mid, const double& cong )
{
    map<int, double>::iterator iteFind = m_module_contribution.find( mid );

    if( iteFind == m_module_contribution.end() )
    {
	m_module_contribution.insert( pair<int, double>( mid, cong ) );
    }
    else
    {
	iteFind->second = iteFind->second + cong;
    }
}
#endif
    
void CCongOptimizer::Observe(void)
{
    //Perform global routing
    CGlobalRouting groute( m_placedb );
    groute.ProbalisticMethod( m_congmap );

    //Find the most overflowed-bin
    int max_overflow_i, max_overflow_j;
    double max_overflow = -numeric_limits<double>::max();
    
    for( int i = 0 ; i < m_hdim ; i++ )
    {
	for( int j = 0 ; j < m_vdim ; j++ )
	{
	    if( m_congmap.GetBinOverflow( i, j ) > max_overflow )
	    {
		max_overflow = m_congmap.GetBinOverflow( i, j );
		max_overflow_i = i;
		max_overflow_j = j;
	    }
	}
    }

    CRect bin_bbox;
    bin_bbox.left = m_congmap.m_left + ( m_congmap.m_hlength * max_overflow_i );
    bin_bbox.right = bin_bbox.left + m_congmap.m_hlength;
    bin_bbox.bottom = m_congmap.m_bottom + ( m_congmap.m_vlength * max_overflow_j );    
    bin_bbox.top = bin_bbox.bottom + m_congmap.m_vlength;
    
    
    double bin_cx, bin_cy;
    m_congmap.ComputeBinCenterLocation( max_overflow_i, max_overflow_j, bin_cx, bin_cy );
    fprintf( stdout, "(%.2f,%.2f) c(%.2f,%.2f) (%.2f,%.2f)\n",
	    bin_bbox.left,
	    bin_bbox.bottom,
	    bin_cx,
	    bin_cy,
	    bin_bbox.right,
	    bin_bbox.top );
    fprintf( stdout, "max overflowed bin (%d,%d): %.2f\n",
	    max_overflow_i,
	    max_overflow_j,
	    max_overflow );
    
    //CCut& curCut1 = m_congmap.GetCut( max_overflow_i, max_overflow_j, CCongMap::Left );
    fprintf( stdout, "*************Left: %.2f\n", m_congmap.GetCutOverflow( max_overflow_i, max_overflow_j, CCongMap::Left ) );
#if 0
    for( map<int, double>::iterator ite = curCut1.m_module_contribution.begin() ; 
	    ite != curCut1.m_module_contribution.end() ; ite++ )
    {
	fprintf( stdout, "%6d %.2f\n", ite->first, ite->second );
    }
#endif
    //CCut& curCut2 = m_congmap.GetCut( max_overflow_i, max_overflow_j, CCongMap::Right );
    fprintf( stdout, "*************Right: %.2f\n", m_congmap.GetCutOverflow( max_overflow_i, max_overflow_j, CCongMap::Right ) );
#if 0
    for( map<int, double>::iterator ite = curCut2.m_module_contribution.begin() ; 
	    ite != curCut2.m_module_contribution.end() ; ite++ )
    {
	fprintf( stdout, "%6d %.2f\n", ite->first, ite->second );
    }
#endif
    //CCut& curCut3 = m_congmap.GetCut( max_overflow_i, max_overflow_j, CCongMap::Bottom );
    fprintf( stdout, "*************Bottom: %.2f\n", m_congmap.GetCutOverflow( max_overflow_i, max_overflow_j, CCongMap::Bottom ) );
#if 0
    for( map<int, double>::iterator ite = curCut3.m_module_contribution.begin() ; 
	    ite != curCut3.m_module_contribution.end() ; ite++ )
    {
	fprintf( stdout, "%6d %.2f\n", ite->first, ite->second );
    }
#endif
    //CCut& curCut4 = m_congmap.GetCut( max_overflow_i, max_overflow_j, CCongMap::Top );
    fprintf( stdout, "*************Top: %.2f\n", m_congmap.GetCutOverflow( max_overflow_i, max_overflow_j, CCongMap::Top ) );
#if 0
    for( map<int, double>::iterator ite = curCut4.m_module_contribution.begin() ; 
	    ite != curCut4.m_module_contribution.end() ; ite++ )
    {
	fprintf( stdout, "%6d %.2f\n", ite->first, ite->second );
    }
#endif
    
#if 0 
    fprintf( stdout, "************ALL*********\n" );
    vector<ContributionItem> all_contribution;
    m_congmap.ComputeSortedBinContribution( max_overflow_i, max_overflow_j, all_contribution );

    for( vector<ContributionItem>::iterator ite = all_contribution.begin() ;
	    ite != all_contribution.end() ; ite++ )
    {
	const Module& curModule = m_placedb.m_modules[ ite->first ];
	fprintf( stdout, "%6d (%10.2f,%10.2f) %5.2f dist: %7.2f\n", 
		ite->first, 
		curModule.m_cx,
		curModule.m_cy,
		ite->second,
		CPoint::Distance( CPoint( curModule.m_cx, curModule.m_cy ), CPoint( bin_cx, bin_cy ) ) );
    }
#endif
    //exit(0);
}

#if 0
void CCongMap::ComputeSortedBinContribution( const int& i, 
	const int& j, 
	vector<ContributionItem>& cons )
{
    cons.clear();
    if( i < 0 || i >= m_hdim || j < 0 || j >= m_vdim )
    {
	fprintf( stderr, "Warning: inccorect cong bin index (%d,%d)\n", i, j );
	return;
    }
    map<int, double> all_contribution;
    
    CCut& curCut1 = GetCut( i, j, Left );
    for( map<int, double>::iterator ite = curCut1.m_module_contribution.begin() ; 
	    ite != curCut1.m_module_contribution.end() ; ite++ )
    {
	map<int, double>::iterator iteFind = all_contribution.find( ite->first );
	if( iteFind == all_contribution.end() )
	{
	    all_contribution[ ite->first ] = ite->second;
	}
	else
	{
	    iteFind->second = iteFind->second + ite->second;
	}
    }
    CCut& curCut2 = GetCut( i, j, Right );
    for( map<int, double>::iterator ite = curCut2.m_module_contribution.begin() ; 
	    ite != curCut2.m_module_contribution.end() ; ite++ )
    {
	map<int, double>::iterator iteFind = all_contribution.find( ite->first );
	if( iteFind == all_contribution.end() )
	{
	    all_contribution[ ite->first ] = ite->second;
	}
	else
	{
	    iteFind->second = iteFind->second + ite->second;
	}
    }
    CCut& curCut3 = GetCut( i, j, Bottom );
    for( map<int, double>::iterator ite = curCut3.m_module_contribution.begin() ; 
	    ite != curCut3.m_module_contribution.end() ; ite++ )
    {
	map<int, double>::iterator iteFind = all_contribution.find( ite->first );
	if( iteFind == all_contribution.end() )
	{
	    all_contribution[ ite->first ] = ite->second;
	}
	else
	{
	    iteFind->second = iteFind->second + ite->second;
	}
    }
    CCut& curCut4 = GetCut( i, j, Top );
    for( map<int, double>::iterator ite = curCut4.m_module_contribution.begin() ; 
	    ite != curCut4.m_module_contribution.end() ; ite++ )
    {
	map<int, double>::iterator iteFind = all_contribution.find( ite->first );
	if( iteFind == all_contribution.end() )
	{
	    all_contribution[ ite->first ] = ite->second;
	}
	else
	{
	    iteFind->second = iteFind->second + ite->second;
	}
    }
    
    cons.reserve( all_contribution.size() );
    
    for( map<int, double>::iterator ite = all_contribution.begin() ;
	    ite != all_contribution.end() ; ite++ )
    {
	cons.push_back( *ite );
    }

    sort( cons.begin(), cons.end(), CompareContributionItemByContribution() );    
}
#endif
