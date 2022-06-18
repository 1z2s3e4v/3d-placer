#include "verify.h"


bool CCheckLegal::check()
{
    double t1=seconds();
    cerr << "\nStart Overlap Checking (binSize: "<< this->m_binSize << ")\n";
    int notInSite=0;
    int notInRow=0;
    int overLap=0;
    ///////////////////////////////////////////////////////
    //1.check all standard cell are on row and in the core region
    //////////////////////////////////////////////////////////
    for(unsigned int i=0; i<m_pDB->m_modules.size(); ++i)
    {
		if(m_pDB->m_modules[i].m_isFixed==false)
		{
			double res=(m_pDB->m_modules[i].m_y-m_pDB->m_coreRgn.bottom)/m_pDB->m_rowHeight;
			int ires=(int)res;
			if( (m_pDB->m_coreRgn.bottom+m_pDB->m_rowHeight*ires)!=m_pDB->m_modules[i].m_y)
			{
			if(m_pDB->m_modules[i].m_height>m_pDB->m_rowHeight)
				cerr<<"\nWarning: Macro:"<<i<<" is not on row!!";
			else
				cerr<<"\nWarning: cell:"<<i<<" is not on row!!";
			++notInRow;

			}
			if((m_pDB->m_modules[i].m_y<m_pDB->m_coreRgn.bottom) || (m_pDB->m_modules[i].m_x<m_pDB->m_coreRgn.left)||
				( (m_pDB->m_modules[i].m_x+m_pDB->m_modules[i].m_width)>m_pDB->m_coreRgn.right) ||
				( (m_pDB->m_modules[i].m_y+m_pDB->m_modules[i].m_height)>m_pDB->m_coreRgn.top))
			{
			//cerr<<"\nWarning: cell:"<<i<<" is not in the core!!";
			fprintf( stderr, "\nWarning: cell: %d (x%.0f %.0f y%.0f) is not in the core!! (%.0f %.0f %.0f %.0f)", 
				i, m_pDB->m_modules[i].m_x, m_pDB->m_modules[i].m_cx, m_pDB->m_modules[i].m_y,
				m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.bottom, m_pDB->m_coreRgn.right, m_pDB->m_coreRgn.top );
			++notInSite;
			}
		}
    }
    if( (seconds()-t1) > 1.0 )
    {
	cerr<<"\nFinish step1, total "<<seconds()-t1<<" seconds";
    }
    t1=seconds();

    ///////////////////////////////////////////
    //2. bin-based overlapping checking
    ///////////////////////////////////////////

    //2.1 build bin

    for(unsigned int k=0; k<m_pDB->m_modules.size(); ++k)
    {
		int binStartX=(int)(m_pDB->m_modules[k].m_x/m_binWidth);
		int binEndX=(int)( (m_pDB->m_modules[k].m_x+m_pDB->m_modules[k].m_width)/m_binWidth);
		int binStartY=(int)(m_pDB->m_modules[k].m_y/m_binHeight);
		int binEndY=(int)((m_pDB->m_modules[k].m_y+m_pDB->m_modules[k].m_height)/m_binHeight);
		legalBinID(binStartX);
		legalBinID(binEndX);
		legalBinID(binStartY);
		legalBinID(binEndY);

		for(int i=binStartX; i<=binEndX; ++i)
		{
			for(int j=binStartY; j<=binEndY; ++j)
			{
				m_moduleBins[i][j].push_back(k);
				//module2bins[k][i]=j;
			}
		}
    }
    //cerr<<"\nFinish build bins";

    //2.2 for all module, check overlapping inside bin
    for(int k=0; k<(int)m_pDB->m_modules.size(); ++k)
    {
		if( m_pDB->m_modules[k].m_isNI ) continue; // (frank) 2022-05-13

		int binStartX=(int)(m_pDB->m_modules[k].m_x/m_binWidth);
		int binEndX=(int)( (m_pDB->m_modules[k].m_x+m_pDB->m_modules[k].m_width)/m_binWidth);
		int binStartY=(int)(m_pDB->m_modules[k].m_y/m_binHeight);
		int binEndY=(int)((m_pDB->m_modules[k].m_y+m_pDB->m_modules[k].m_height)/m_binHeight);
		legalBinID(binStartX);
		legalBinID(binEndX);
		legalBinID(binStartY);
		legalBinID(binEndY);
		//for all bins
		for(int m=binStartX; m<=binEndX; ++m)
		{
			for(int n=binStartY; n<=binEndY; ++n)
			{
			//for all modules in bins
			for(int i=0; i<(int)m_moduleBins[m][n].size(); ++i)
			{
				if(m_moduleBins[m][n][i]!=k)
				{
					int mID=m_moduleBins[m][n][i];

					if( m_pDB->m_modules[mID].m_isNI ) continue; // (frank) 2022-05-13

					double area=getOverlapArea(
						(double)m_pDB->m_modules[k].m_x, 
						(double)m_pDB->m_modules[k].m_y,
						(double)m_pDB->m_modules[k].m_x+(double)m_pDB->m_modules[k].m_width, 
						(double)m_pDB->m_modules[k].m_y+(double)m_pDB->m_modules[k].m_height,
						(double)m_pDB->m_modules[mID].m_x, 
						(double)m_pDB->m_modules[mID].m_y,
						(double)m_pDB->m_modules[mID].m_x+(double)m_pDB->m_modules[mID].m_width, 
						(double)m_pDB->m_modules[mID].m_y+(double)m_pDB->m_modules[mID].m_height );
					if( (abs( area ) > 0.1) && (m_pDB->m_modules[k].m_isFixed==false ||
							m_pDB->m_modules[mID].m_isFixed==false ))
					{
						cerr<<"\nWarning: cell:"<<m_pDB->m_modules[k].m_name<<"("<<m_pDB->m_modules[k].m_x<<
						","<<m_pDB->m_modules[k].m_y<<","<< m_pDB->m_modules[k].m_width
						<<") overlap with cell "<<m_pDB->m_modules[mID].m_name<<
						"("<<m_pDB->m_modules[mID].m_x<<","<<m_pDB->m_modules[mID].m_y
						<<","<< m_pDB->m_modules[mID].m_width<<")!!Area:"<<area<<"";
						++overLap;

					}
				}

			}
			}
		}
    }
    //cerr<<"\n Overlapping checking finished:\nerr1:"<<notInRow<<"\nerr2:"<<notInSite<<"\nerr3:"<<overLap<<"\nTotal "<<seconds()-t1<<" seconds\n ================================================\n";
    cerr << "  err1:"<<notInRow<<"\n  err2:"<<notInSite<<"\n  err3:"<<overLap<<"\nTotal "<<seconds()-t1<<" seconds\n";

    if( notInRow!=0 || notInSite!=0 || overLap!=0 )
	return false;
    else
	return true;
}
