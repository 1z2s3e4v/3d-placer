#include "randomplace.h"
/*
CRandomPlace::CRandomPlace(CPlaceDB& db)
{
	m_db = &db;
}


CRandomPlace::~CRandomPlace()
{
}
*/

//void CRandomPlace::place( double ratio )
void CRandomPlace::Place( CPlaceDB& db, double ratio )
{
    CPlaceDB* m_db = &db;

    double rdx,rdy;
    double core_w,core_h;

    core_w = m_db->m_coreRgn.right -  m_db->m_coreRgn.left;
    core_h = m_db->m_coreRgn.top - m_db->m_coreRgn.bottom;

    core_w *= ratio;
    core_h *= ratio;

    double left_pad = (m_db->m_coreRgn.right -  m_db->m_coreRgn.left - core_w ) / 2;
    double bottom_pad = (m_db->m_coreRgn.top - m_db->m_coreRgn.bottom - core_h ) / 2;

    for (unsigned int i = 0 ; i < m_db->m_modules.size() ; i++ )
    {
	if (m_db->m_modules[i].m_isFixed == false) 
	{
	    rdx = left_pad + rand() % ((int)core_w*10)/10.0 + m_db->m_coreRgn.left;
	    rdy = bottom_pad + rand() % ((int)core_h*10)/10.0 + m_db->m_coreRgn.bottom;
	    m_db->MoveModuleCenter(i,rdx,rdy);
	}
    }
}
