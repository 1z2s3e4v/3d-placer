#ifndef VERIFY_H
#define VERIFY_H
#include "placedb.h"

class CCheckLegal
{
	public:
		CCheckLegal( CPlaceDB& db )
		{
			m_binSize= (int)(sqrt(static_cast<double>(db.m_modules.size()))/2);
			m_pDB = &db;
			m_binWidth=(db.m_coreRgn.right-db.m_coreRgn.left)/m_binSize;
			m_binHeight=(db.m_coreRgn.top-db.m_coreRgn.bottom)/m_binSize;
			int averageBinModule=10*(int)(db.m_modules.size()/(m_binSize*m_binSize));
			m_moduleBins.resize(m_binSize);
			for(int i=0; i<m_binSize; ++i)
			{
				m_moduleBins[i].resize(m_binSize);
				for(int j=0; j<m_binSize; ++j)
				{
					m_moduleBins[i][j].reserve(averageBinModule);
				}
			}
		};
		~CCheckLegal(){};

		bool check();
	private:
		int m_binSize;
		double m_binWidth;
		double m_binHeight;
		CPlaceDB* m_pDB;
		vector< vector< vector<int> > > m_moduleBins;
		void legalBinID(int& x)
		{
			if(x<0)
				x=0;
			if(x>=m_binSize)
				x=m_binSize-1;
		}
		double GetBinX( const int& binX )
		{
			return m_pDB->m_coreRgn.left + binX * m_binWidth; 
		}
		double GetBinY( const int& binY )
		{ 
			return m_pDB->m_coreRgn.bottom + binY * m_binHeight; 
		}
};
#endif
