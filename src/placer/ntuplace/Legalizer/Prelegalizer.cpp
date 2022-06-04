#include <cstdio>
#include <iostream>
#include <list>
using namespace std;

#include "Prelegalizer.h"  
#include "placedb.h"


//CPrelegalizer::CPrelegalizer(void)
//{
//}
//
//CPrelegalizer::~CPrelegalizer(void)
//{
//}

void CPrelegalizer::Prelegalizer(CPlaceDB& fplan, const SCALE_TYPE& type, const double& scale_factor)
{
	if( scale_factor >= 1.0 )
	{
		cerr << "Warning: Scale Factor is Larger of Equal than 1.0\n";
	}


	if( type == SCALE_TO_LEFT )
	{
		double scale_base = fplan.m_coreRgn.left;

		for( int i = 0 ; i < (signed)fplan.m_modules.size() ; i++ )
		{	
			if( !fplan.m_modules[i].m_isFixed )
			{
				double move_left_x = fplan.m_modules[i].m_x;
				double move_cx = ( move_left_x - scale_base ) * scale_factor + scale_base + (fplan.m_modules[i].m_width / 2.0);
				fplan.MoveModuleCenter( i, move_cx, fplan.m_modules[i].m_cy );
			}
		}

	}
	else if( type == SCALE_TO_LEFT_BETWEEN_MACRO )
	{
		list<CRect> macro_list;

		// find all fixed macros
		for( int i = 0 ; i < (signed)fplan.m_modules.size() ; i++ )
		{
			if( fplan.m_modules[i].m_isFixed )
			{
				macro_list.push_back( CRect( fplan.m_modules[i].m_y + fplan.m_modules[i].m_height,
											  fplan.m_modules[i].m_y,
											  fplan.m_modules[i].m_x,
											  fplan.m_modules[i].m_x + fplan.m_modules[i].m_width ) );
			}
		}

		
		for( int i = 0 ; i < (signed)fplan.m_modules.size() ; i++ )
		{	
			if( !fplan.m_modules[i].m_isFixed )
			{
				// skip the modules inside the macro
//				double left = fplan.m_modules[i].m_x;
//				double right = left + fplan.m_modules[i].m_width;
//				double bottom = fplan.m_modules[i].m_y;
//				double top = bottom + fplan.m_modules[i].m_height;
//				
//				
//				bool bInside = false;
//				for( list<CRect>::iterator ite = macro_list.begin() ;
//					ite != macro_list.end() ;
//					ite++ )
//				{
//					if( left < ite->right &&
//					    right > ite->left &&
//					    bottom < ite->top &&
//					    top > ite->bottom )
//					{
//						bInside = true;
//						break;
//					}   	
//					    		
//				}
//				
//				if( bInside )
//					continue;
				
				double scale_base = fplan.m_coreRgn.left;
				double move_left_x = fplan.m_modules[i].m_x;
				
				
				
				//fine the scale_base
				for( list<CRect>::iterator ite = macro_list.begin() ;
					ite != macro_list.end() ;
					ite++ )
				{
					if( ite->right > scale_base &&
						ite->top > fplan.m_modules[i].m_y &&
						ite->bottom < fplan.m_modules[i].m_y + fplan.m_modules[i].m_height &&
						move_left_x >= ite->right )
					{
						scale_base = ite->right;
					}
				}

				double move_cx = ( move_left_x - scale_base ) * scale_factor + scale_base + (fplan.m_modules[i].m_width / 2.0);
				fplan.MoveModuleCenter( i, move_cx, fplan.m_modules[i].m_cy );
			}
		}
		

	}
	else if( type == SCALE_TO_RIGHT )
	{
		double scale_base = fplan.m_coreRgn.right;
		for( int i = 0 ; i < (signed)fplan.m_modules.size() ; i++ )
		{	
			if( !fplan.m_modules[i].m_isFixed )
			{
				double move_right_x = fplan.m_modules[i].m_x + fplan.m_modules[i].m_width;
				double move_cx = ( move_right_x - scale_base ) * scale_factor + scale_base - (fplan.m_modules[i].m_width / 2.0);
				fplan.MoveModuleCenter( i, move_cx, fplan.m_modules[i].m_cy );
			}
		}		
	}
	else if( type == SCALE_TO_RIGHT_BETWEEN_MACRO )
	{
		list<CRect> macro_list;

		// find all fixed macros
		for( int i = 0 ; i < (signed)fplan.m_modules.size() ; i++ )
		{
			if( fplan.m_modules[i].m_isFixed )
			{
				macro_list.push_back( CRect( fplan.m_modules[i].m_y + fplan.m_modules[i].m_height,
											  fplan.m_modules[i].m_y,
											  fplan.m_modules[i].m_x,
											  fplan.m_modules[i].m_x + fplan.m_modules[i].m_width ) );
			}
		}

		
		for( int i = 0 ; i < (signed)fplan.m_modules.size() ; i++ )
		{	
			
			if( !fplan.m_modules[i].m_isFixed )
			{
				double scale_base = fplan.m_coreRgn.right;
				double move_right_x = fplan.m_modules[i].m_x+fplan.m_modules[i].m_width;
				
				//fine the scale_base
				for( list<CRect>::iterator ite = macro_list.begin() ;
					ite != macro_list.end() ;
					ite++ )
				{
					if( ite->left < scale_base &&
						ite->top > fplan.m_modules[i].m_y &&
						ite->bottom < fplan.m_modules[i].m_y + fplan.m_modules[i].m_height &&
						move_right_x <= ite->left )
					{
						scale_base = ite->left;
					}
				}


				double move_cx = ( move_right_x - scale_base ) * scale_factor + scale_base - (fplan.m_modules[i].m_width / 2.0);
				fplan.MoveModuleCenter( i, move_cx, fplan.m_modules[i].m_cy );
			}
		}

	}
	else if( type == SCALE_TO_MIDLINE )
	{
		double scale_base = ( fplan.m_coreRgn.left + fplan.m_coreRgn.right ) / 2.0;
		for( int i = 0 ; i < (signed)fplan.m_modules.size() ; i++ )
		{	
			if( !fplan.m_modules[i].m_isFixed )
			{
				double move_center_x = fplan.m_modules[i].m_cx;
				double move_cx = ( move_center_x - scale_base ) * scale_factor + scale_base;
				fplan.MoveModuleCenter( i, move_cx, fplan.m_modules[i].m_cy );
			}
		}	
	}
	else if( type == SCALE_TO_MIDLINE_BETWEEN_MACRO )
	{
		double midline_x = ( fplan.m_coreRgn.left + fplan.m_coreRgn.right ) / 2.0;
		list<CRect> left_macro_list, right_macro_list;

		for( int i = 0 ; i < (signed)fplan.m_modules.size() ; i++ )
		{
			if( fplan.m_modules[i].m_isFixed )
			{
				if( fplan.m_modules[i].m_x < midline_x )
				{
					left_macro_list.push_back( CRect( fplan.m_modules[i].m_y + fplan.m_modules[i].m_height,
											  fplan.m_modules[i].m_y,
											  fplan.m_modules[i].m_x,
											  fplan.m_modules[i].m_x + fplan.m_modules[i].m_width ) );
				}
				
				if( fplan.m_modules[i].m_x + fplan.m_modules[i].m_width > midline_x )
				{
					right_macro_list.push_back( CRect( fplan.m_modules[i].m_y + fplan.m_modules[i].m_height,
											  fplan.m_modules[i].m_y,
											  fplan.m_modules[i].m_x,
											  fplan.m_modules[i].m_x + fplan.m_modules[i].m_width ) );
				}
			}
		}
		
		// scale modules
		for( int i = 0 ; i < (signed)fplan.m_modules.size() ; i++ )
		{	
			
			if( !fplan.m_modules[i].m_isFixed )
			{
				// module is at the left part of the core
				if( fplan.m_modules[i].m_cx < midline_x )
				{
					double scale_base = midline_x;
					double move_right_x = fplan.m_modules[i].m_x+fplan.m_modules[i].m_width;
					
					//fine the scale_base
					for( list<CRect>::iterator ite = left_macro_list.begin() ;
						ite != left_macro_list.end() ;
						ite++ )
					{
						if( ite->left < scale_base &&
							ite->top > fplan.m_modules[i].m_y &&
							ite->bottom < fplan.m_modules[i].m_y + fplan.m_modules[i].m_height &&
							move_right_x <= ite->left )
						{
							scale_base = ite->left;
						}
					}
					double move_cx = ( move_right_x - scale_base ) * scale_factor + scale_base - (fplan.m_modules[i].m_width / 2.0);
					fplan.MoveModuleCenter( i, move_cx, fplan.m_modules[i].m_cy );
				}
				else // module is at the right part of the core
				{
					double scale_base = midline_x;
					double move_left_x = fplan.m_modules[i].m_x;
					
					
					
					//fine the scale_base
					for( list<CRect>::iterator ite = right_macro_list.begin() ;
						ite != right_macro_list.end() ;
						ite++ )
					{
						if( ite->right > scale_base &&
							ite->top > fplan.m_modules[i].m_y &&
							ite->bottom < fplan.m_modules[i].m_y + fplan.m_modules[i].m_height &&
							move_left_x >= ite->right )
						{
							scale_base = ite->right;
						}
					}
	
					double move_cx = ( move_left_x - scale_base ) * scale_factor + scale_base + (fplan.m_modules[i].m_width / 2.0);
					fplan.MoveModuleCenter( i, move_cx, fplan.m_modules[i].m_cy );
				}

			}//@if( !fplan.m_modules[i].m_isFixed )
			
		}//@scale modules
		
		
	}
	else
	{
		cerr << "Warning: Unknown Scale Type\n";
		return;
	}

	fplan.CalcHPWL();
}
