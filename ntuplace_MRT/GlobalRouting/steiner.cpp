#include "steiner.h"

using namespace std;

// (donnie)
// global variable
CFlute* pgFlute = NULL;

void CFlute::Solve( const std::vector<CPoint>& points )
{
    m_steiner_array.clear();

    if( points.size() > MAXD )
    {
	fprintf(stderr, "Warning: flute cannot solve more than %d points (current %d)\n", MAXD, points.size());
	return;
    }

    for( unsigned int i = 0 ; i < points.size() ; i++ )
    {
	m_x[i] = static_cast<int>(points[i].x);
	m_y[i] = static_cast<int>(points[i].y);
    }

#if 0
    //test code
    printf("%d ", points.size() );
    for( unsigned int i = 0 ; i < points.size() ; i++ )
    {
	printf("(%f,%f) ", points[i].x, points[i].y );
    }
    printf("\n");
    fflush(stdout);
    //@test code
#endif

#if 0
    //test code
    FILE* outfile = fopen( "flutes.log", "w" );
    fprintf( outfile, "# = %d\n", points.size() );
    fflush( outfile );
    for( int i = 0 ; i < static_cast<int>(points.size()) ; i++ )
    {
	fprintf( outfile, "(%d,%d)\n", m_x[i], m_y[i] );
	fflush( outfile );
    }
    fclose( outfile );
    //@test code
#endif 

    Tree t = flute(points.size(), m_x, m_y, ACCURACY);    

    for( int i = 0 ; i < 2*t.deg - 2 ; i++ )
    {
	if( i == t.branch[i].n )
	    continue;

	int index = t.branch[i].n;

	CPoint p1( static_cast<double>(t.branch[i].x), static_cast<double>(t.branch[i].y));
	CPoint p2( static_cast<double>(t.branch[index].x), static_cast<double>(t.branch[index].y) );

	if( p1.x == p2.x &&
	    p1.y == p2.y )
	    continue;
	
	m_steiner_array.push_back(p1);
	m_steiner_array.push_back(p2);
    }
    
    free( t.branch );
}
