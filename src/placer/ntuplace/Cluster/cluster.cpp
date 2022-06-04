#include "cluster.h"
#include "fccluster.h"	
#include "../PlaceCommon/ParamPlacement.h"
#include "../PlaceDB/placedb.h"

#include <iomanip>
#include <cstdio>
#include <cstring>



void CClustering::declustering(  CPlaceDB& dbsmall, CPlaceDB& dblarge,bool isPertab )
{
	int clusterMove=0;
	int declusterMove=0;
	for(int i=0; i<(int)dbsmall.m_modules.size(); i++)
	{
	    bool preFix=false;
	    //if(dbsmall.m_modules[i].m_isFixed==false)
	    // donnie
	    {
		++clusterMove;

		if( dbsmall.m_modules[i].m_isFixed ) 
		{
		    if( this->m_hierarchy[i].size() > 1 )
			printf( "WARNING: check dbsmall for fixed terminal %d!\n", i );
		    else if(dblarge.m_modules[ m_hierarchy[i][0] ].m_isFixed != dbsmall.m_modules[i].m_isFixed)
		    {
			preFix = true;
			//    cerr<<"\nSmall: x="<<dbsmall.m_modules[i].m_cx<<" y="<<dbsmall.m_modules[i].m_cy;
		    }
		}

		double shift = dbsmall.m_rowHeight * 0.1;
		if( dbsmall.m_modules[i].m_isFixed || (int)m_hierarchy[i].size() == 1 )
		    shift = 0; // cannot shift
		if( param.bPerturb == false )
		    shift = 0;

		for( unsigned int j=0; j<this->m_hierarchy[i].size(); j++)
		{
		    //assert(dblarge.m_modules[m_hierarchy[i][j]].m_isFixed==false);
#if 0
		    if(isPertab==true && dbsmall.m_modules[i].m_isFixed == false)
		    {				
			double rdx = rand() % (int)(dbsmall.m_modules[i].m_width/2) + (int)(dbsmall.m_modules[i].m_x+dbsmall.m_modules[i].m_width/4);
			double rdy = rand() % (int)(dbsmall.m_modules[i].m_height/2) + (int)(dbsmall.m_modules[i].m_y+dbsmall.m_modules[i].m_height/4);
			//dblarge.SetModuleLocation(m_hierarchy[i][j],rdx,rdy);
			dblarge.MoveModuleCenter(m_hierarchy[i][j],rdx,rdy);
			++declusterMove;
		    }
		    else
#endif
		    {

			double rdx = dbsmall.m_modules[i].m_cx;
			double rdy = dbsmall.m_modules[i].m_cy;
			rdx += ( (rand() % 10000) / 10000.0 - 0.5 ) * shift;
			rdy += ( (rand() % 10000) / 10000.0 - 0.5 ) * shift;

			// 2006-10-7 (donnie) bound positions
			if( rdx > dblarge.m_coreRgn.right )
			    rdx = dblarge.m_coreRgn.right;
			else if( rdx < dblarge.m_coreRgn.left )
			    rdx = dblarge.m_coreRgn.left;
			if( rdy > dblarge.m_coreRgn.top )
			    rdy = dblarge.m_coreRgn.top;
			else if( rdy < dblarge.m_coreRgn.bottom )
			    rdy = dblarge.m_coreRgn.bottom;

			dblarge.SetModuleOrientation(m_hierarchy[i][j], dbsmall.m_modules[i].m_orient);//indark MPTree rotation
			dblarge.MoveModuleCenter( m_hierarchy[i][j], rdx, rdy ); // by donnie
			dblarge.m_modules[ m_hierarchy[i][j] ].m_isFixed = dbsmall.m_modules[i].m_isFixed;  // donnie


			if(preFix==true)
			{
			    //	cerr<<"\nLarge: x="<<dblarge.m_modules[m_hierarchy[i][j]].m_cx<<" y="<<dblarge.m_modules[m_hierarchy[i][j]].m_cy<<" sx:"<<rdx<<" sy:"<<rdy;
			}
			++declusterMove;
		    }
		}
	    }
	}
	//cout<<"\n LargeDB move#:"<<clusterMove<<" SmallDB move#:"<<declusterMove<<"\n";
#if 0
	char file[200];
	if( param.bShow )
	{
	    sprintf( file, "dbsmall.plt" );
	    dbsmall.OutputGnuplotFigure( file, false );
	    sprintf( file, "dblarge.plt" );
	    dblarge.OutputGnuplotFigure( file, false );
	}
#endif

	cout<<"\n";

}

void CClustering::clustering(const CPlaceDB& dblarge, CPlaceDB& dbsmall ,int targetClusterNumber, double areaRatio,int ctype)
{
    
    /*
    double startTime=seconds();
    printf( "Memory Used before Clustering : %.0fMB)\n", GetPeakMemoryUsage() );
    */
	if(ctype==1) //first choice clustering
	{
		double start=seconds();
		CFCClustering fcc;
		fcc.clustering(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
		if(showMsg==true)
		{
			cout<<"\n==Type1 Cluster Time:"<<seconds()-start<<" seconds";
		}
	}
	else if(ctype==2) //first choice clustering with physical clustering
	{
		CFCClustering fcc;
		fcc.physicalclustering(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
	}
	else if(ctype==3)
	{
		double start=seconds();
		CClusterDBFC dbfc;
		dbfc.clustering(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
		if(showMsg==true)
		{
			cout<<"\n==Type3 Cluster Time:"<<seconds()-start<<" seconds";
		}
	}
	else if(ctype==4)
	{
		//first choice with DB no acc Heuristic
		double start=seconds();
		CClusterDBFC dbfc;
		dbfc.clusteringNH(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
		if(showMsg==true)
		{
			cout<<"\n==Type4 Cluster Time:"<<seconds()-start<<" seconds";
		}
	}
	else if(ctype==5)
	{
		//best choice clustering 
		double start=seconds();
		CClusterDBBC dbbc;
		dbbc.clustering(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
		if(showMsg==true)
		{
			cout<<"\n==Type5 Cluster Time:"<<seconds()-start<<" seconds";
		}
		
	}
	else if(ctype==6)
	{
		//best choice clustering with acc heuristic
		double start=seconds();
		CClusterDBBC dbbc;
		dbbc.clusteringWithAcc(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
		if(showMsg==true)
		{
			cout<<"\n==Type6 Cluster Time:"<<seconds()-start<<" seconds";
		}
		
		/*
		//first choice with DB no acc Heuristic
		double start=seconds();
		CClusterDBFC dbfc;
		dbfc.clusteringSG(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
		if(showMsg==true)
		{
			cout<<"\n==Type6 Cluster Time:"<<seconds()-start<<" seconds";
		}
		*/
	}
	else if(ctype==7)
	{
		//first choice with DB no acc Heuristic
		double start=seconds();
		CClusterDBFC dbfc;
		dbfc.clusteringBC(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
		if(showMsg==true)
		{
			cout<<"\n==Type7 Cluster Time:"<<seconds()-start<<" seconds";
		}
	}
	else if(ctype==8)
	{
		//first choice with DB no acc Heuristic
		double start=seconds();
		CClusterDBFC dbfc;
		dbfc.clusteringBLN(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
		if(showMsg==true)
		{
			cout<<"\n==Type8 Cluster Time:"<<seconds()-start<<" seconds";
		}
	}
	else
	{
		CFCClustering fcc;
		fcc.clustering(dblarge, dbsmall ,m_hierarchy, targetClusterNumber, areaRatio);
	}
    /*
    FILE* clusterOut;
    string fileName="ClusterOut.txt";
    clusterOut = fopen( fileName.c_str(), "a" );
    fprintf(clusterOut, "%s,  TargetClusterNumber: %10.0f, Cluster Type: %d, Used Time: %10.0f \n", 
        param.outFilePrefix.c_str(), static_cast<double>(targetClusterNumber), ctype, seconds()-startTime);
    fclose(clusterOut);
    printf( "Memory Used after Clustering : %.0fMB)\n", GetPeakMemoryUsage() );
    */
}
