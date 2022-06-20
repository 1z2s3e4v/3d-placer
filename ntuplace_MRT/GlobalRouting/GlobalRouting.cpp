#include "GlobalRouting.h"
#include "CongMap.h"
#include "steiner.h"
#include "placeutil.h"
#include "MinimumSpanningTree.h"

CGlobalRouting::CGlobalRouting( CPlaceDB& placedbref ) :
    m_placedbref( placedbref )
{
    //Generate randomized net order
    const int SWAP_NUM = 2*m_placedbref.m_nets.size();
    m_rand_net_order.resize( m_placedbref.m_nets.size() );
    
    for( unsigned int i = 0 ; i < m_rand_net_order.size() ; i++ )
    {
	m_rand_net_order[i] = i;
    }
    
    for( int i = 0 ; i < SWAP_NUM ; i++ )
    {
	int index1 = random()%m_rand_net_order.size();
	int index2 = random()%m_rand_net_order.size();

	swap( m_rand_net_order[index1], m_rand_net_order[index2] );
    }
}

void CGlobalRouting::ProbalisticMethod( CCongMap& congmap )
{
    congmap.clear();
    
    if( pgFlute == NULL )
    {
	printf( "Create FLUTE\n" );
	pgFlute = new CFlute;
    }
    
    CMinimumSpanningTree mst;

    //double totalSteinerWL = 0.0;  // 2006-09-14 (donnie)
    
    for( unsigned int i = 0 ; i < m_placedbref.m_nets.size() ; i++ )
    {
	const Net& curNet = m_placedbref.m_nets[i];

	if( 2 == curNet.size() )
	{
	    double x, y;
	    m_placedbref.GetPinLocation( curNet[0], x, y );
	    CPoint p1(x,y);
	    m_placedbref.GetPinLocation( curNet[1], x, y );
	    CPoint p2(x,y);

	    congmap.AddTwoPinNet( p1, p2 );
	    //totalSteinerWL += CPoint::Distance( p1, p2 );
	}
	else if( curNet.size() > 2 )
	{
	    //Construct the steiner tree
	    vector<CPoint> points;
	    for( unsigned int j = 0 ; j < curNet.size() ; j++ )
	    {
		double x, y;
		m_placedbref.GetPinLocation( curNet[j], x, y );
		points.push_back( CPoint(x, y) );
	    }

	    if( curNet.size() <= MAXD && !gArg.CheckExist( "nosteiner" ) )
	    {
		pgFlute->Solve( points );

		//Add congestion
		for( unsigned int j = 0 ; j < pgFlute->m_steiner_array.size() ; j=j+2 )
		{
		    congmap.AddTwoPinNet( pgFlute->m_steiner_array[j], pgFlute->m_steiner_array[j+1] );
		    //totalSteinerWL += CPoint::Distance( pgFlute->m_steiner_array[j], pgFlute->m_steiner_array[j+1] );
		}
	    }
	    else
	    {
		mst.Solve( points );
#if 0
		//test code
		for( unsigned int j = 0 ; j < mst.m_mst_array.size() ; j++ )
		{
		    int curIndex = mst.m_mst_index_array[j];
		    fprintf( stdout, "[%d] (%.2f,%.2f) [%d] (%.2f,%.2f)\n",
			    j,
			    mst.m_mst_array[j].x,
			    mst.m_mst_array[j].y,
			    curIndex,
			    points[curIndex].x,
			    points[curIndex].y );
		}
		fprintf( stdout, "\n" );
		//@test code
#endif
		
		for( unsigned int j = 0 ; j < mst.m_mst_array.size() ; j=j+2 )
		{
		    congmap.AddTwoPinNet( mst.m_mst_array[j], mst.m_mst_array[j+1] );
#if 0
		    if( gArg.CheckExist( "nosteiner" ) )
		    {
			int first_pin_id = curNet[ mst.m_mst_index_array[j] ];
			int second_pin_id = curNet[ mst.m_mst_index_array[j+1] ];
			
			Pin& firstPin = m_placedbref.m_pins[ first_pin_id ];
			Pin& secondPin = m_placedbref.m_pins[ second_pin_id ];
		/*	
			congmap.AddTwoPinNetContribution( 
				firstPin.moduleId,
				mst.m_mst_array[j], 
				secondPin.moduleId,
				mst.m_mst_array[j+1] ); */
		    }
#endif
		}
	    }
	    
	}
    }

#if 0
    //test code
    fprintf( stdout, "***************************Inside the probabilistic method\n" );
    CPoint p1, p2;
    congmap.ComputeBinCenterLocation( 5, 5, p1.x, p1.y );
    congmap.ComputeBinCenterLocation( 10, 10, p2.x, p2.y );
    congmap.AddBlockageByRatio( p1, p2, 0.8 );
    //@test code
#endif
}

void CGlobalRouting::FastRoute( CCongMap& congmap )
{
    congmap.clear();
    
    PreRoute( congmap );
}

void CGlobalRouting::PreRoute( CCongMap& congmap )
{
    CMinimumSpanningTree mst;
    if( pgFlute == NULL )
    {
	printf( "Create FLUTE\n" );
	pgFlute = new CFlute;
    }

    double totalSteinerWL = 0.0;  // 2006-09-14 (donnie)

    //TODO: Record the flute results
    
    //First, route all nets by upper and lower L's
    for( unsigned int i = 0 ; i < m_rand_net_order.size() ; i++ )
    {
	int net_id = m_rand_net_order[i];
	const Net& curNet = m_placedbref.m_nets[net_id];

	if( 2 == curNet.size() )
	{
	    double x, y;
	    m_placedbref.GetPinLocation( curNet[0], x, y );
	    CPoint p1(x,y);
	    m_placedbref.GetPinLocation( curNet[1], x, y );
	    CPoint p2(x,y);

	    congmap.AddTwoPinNetByUpperLowerL( p1, p2 );
	    totalSteinerWL += CPoint::Distance( p1, p2 );
	}
	else if( curNet.size() > 2 )
	{
	    //Construct the steiner tree
	    vector<CPoint> points;
	    for( unsigned int j = 0 ; j < curNet.size() ; j++ )
	    {
		double x, y;
		m_placedbref.GetPinLocation( curNet[j], x, y );
		points.push_back( CPoint(x, y) );
	    }

	    if( curNet.size() <= MAXD && !gArg.CheckExist( "nosteiner" ) )
	    {
		pgFlute->Solve( points );

		//Add congestion
		for( unsigned int j = 0 ; j < pgFlute->m_steiner_array.size() ; j=j+2 )
		{
		    congmap.AddTwoPinNetByUpperLowerL( pgFlute->m_steiner_array[j], pgFlute->m_steiner_array[j+1] );
		    totalSteinerWL += CPoint::Distance( pgFlute->m_steiner_array[j], pgFlute->m_steiner_array[j+1] );
		}
	    }
	    else
	    {
		mst.Solve( points );

		for( unsigned int j = 0 ; j < mst.m_mst_array.size() ; j=j+2 )
		{
		    congmap.AddTwoPinNet( mst.m_mst_array[j], mst.m_mst_array[j+1] );
		    totalSteinerWL += CPoint::Distance( mst.m_mst_array[j], mst.m_mst_array[j+1] );
		}
	    }

	}	

    }    
    
    printf( "   Total Steiner WL = %.0f (%g)\n", totalSteinerWL, totalSteinerWL );

    //TODO: rip-up & and route each net
    

}
