
#include "placeutil.h"
#include "placedb.h"
#include "lefdef.h"
#include "MinimumSpanningTree.h"
#include "GlobalRouting.h"
#include "steiner.h"
#include "CongMap.h"
#include "GRouteMap.h"

void CPlaceDBScaling::YShift( CPlaceDB& db, const double& shift )
{
    //printf( "x shift = %f\n", shift );

    // core region    
    db.m_coreRgn.bottom += shift;
    db.m_coreRgn.top    += shift;
    db.m_dieArea.bottom += shift;
    db.m_dieArea.top    += shift;

    // blocks
    for( unsigned int i=0; i<db.m_modules.size(); i++ )
    {
	db.m_modules[i].m_y  += shift;
	db.m_modules[i].m_cy += shift;
    }

    if( db.m_modules_bak.size() == db.m_modules.size() )
    {
	for( unsigned int i=0; i<db.m_modules.size(); i++ )
	{
	    db.m_modules_bak[i].m_y  += shift;
	    db.m_modules_bak[i].m_cy += shift;
	}
    }
    
    // pins
    for( unsigned int i=0; i<db.m_pins.size(); i++ )
	db.m_pins[i].absY += shift;
   
    // sites
    for( unsigned int i=0; i<db.m_sites.size(); i++ )
	db.m_sites[i].m_bottom += shift;
}


void CPlaceDBScaling::XShift( CPlaceDB& db, const double& shift )
{
    //printf( "x shift = %f\n", shift );

    // core region    
    db.m_coreRgn.left  += shift;
    db.m_coreRgn.right += shift;
    db.m_dieArea.left  += shift;
    db.m_dieArea.right += shift;

    // blocks
    for( unsigned int i=0; i<db.m_modules.size(); i++ )
    {
	db.m_modules[i].m_x += shift;
	db.m_modules[i].m_cx += shift;
    }
    
    if( db.m_modules_bak.size() == db.m_modules.size() )
    {
	for( unsigned int i=0; i<db.m_modules.size(); i++ )
	{
	    db.m_modules_bak[i].m_x += shift;
	    db.m_modules_bak[i].m_cx += shift;
	}
    }

    // pins
    for( unsigned int i=0; i<db.m_pins.size(); i++ )
	db.m_pins[i].absX += shift;
   
    // sites
    for( unsigned int i=0; i<db.m_sites.size(); i++ )
	for( unsigned int j=0; j<db.m_sites[i].m_interval.size(); j++ )
	    db.m_sites[i].m_interval[j] += shift;
}

void CPlaceDBScaling::YScale( CPlaceDB& db, const double& scale )
{
    // core region
    db.m_coreRgn.top    *= scale;
    db.m_coreRgn.bottom *= scale;
    db.m_dieArea.bottom *= scale;
    db.m_dieArea.top    *= scale;

    // blocks
    for( unsigned int i=0; i<db.m_modules.size(); i++ )
    {
	db.m_modules[i].m_y  *= scale;
	db.m_modules[i].m_cy *= scale;
	db.m_modules[i].m_height *= scale;
	db.m_modules[i].m_area   *= scale;
    }

    // pins
    for( unsigned int i=0; i<db.m_pins.size(); i++ )
    {
	db.m_pins[i].yOff *= scale;
	db.m_pins[i].absY *= scale;
    }
   
    // sites
    for( unsigned int i=0; i<db.m_sites.size(); i++ )
	db.m_sites[i].m_bottom *= scale;
}

void CPlaceDBScaling::XScale( CPlaceDB& db, const double& scale )
{
    //printf( "x scale = %f\n", scale );

    // core region
    db.m_coreRgn.left  *= scale;
    db.m_coreRgn.right *= scale;
    db.m_dieArea.left  *= scale;
    db.m_dieArea.right *= scale;

    // blocks
    for( unsigned int i=0; i<db.m_modules.size(); i++ )
    {
	db.m_modules[i].m_x  *= scale;
	db.m_modules[i].m_cx *= scale;
	db.m_modules[i].m_width *= scale;
	db.m_modules[i].m_area  *= scale;
    }

    // pins
    for( unsigned int i=0; i<db.m_pins.size(); i++ )
    {
	db.m_pins[i].xOff *= scale;
	db.m_pins[i].absX *= scale;
    }
   
    // sites
    for( unsigned int i=0; i<db.m_sites.size(); i++ )
    {
	db.m_sites[i].m_step *= scale;
	for( unsigned int j=0; j<db.m_sites[i].m_interval.size(); j++ )
	    db.m_sites[i].m_interval[j] *= scale;
    }
}


////////////////////////////////

// 2006-04-23 (donnie)
void CPlaceFlipping::RemoveCellPinOffset( CPlaceDB& db )
{
    printf( "Remove cell pin offset\n" );
    int count = 0;
    for( unsigned int p=0; p<db.m_pins.size(); p++ )
    {
	int id = db.m_pins[p].moduleId;
	if( db.m_modules[id].m_isFixed )
	    continue;
	if( db.m_modules[id].m_height != db.m_rowHeight )
	    continue;

	// not fixed std cell
	db.m_pins[p].xOff = 0.0;
	db.m_pins[p].yOff = 0.0;
	count++;
    }
    printf( "   %d pins\n", count );
}

// 2006-04-23 (donnie)
void CPlaceFlipping::CheckCellOrientation( CPlaceDB& db )
{
    printf( "Check cell orientations\n" );
    // assume no channels
    // 
    double bottom = db.m_sites[0].m_bottom;
    double rowStep = db.m_rowHeight;
    for( unsigned int i=0; i<db.m_modules.size(); i++ )
    {
	if( db.m_modules[i].m_isFixed )
	    continue;
	if( db.m_modules[i].m_height != db.m_rowHeight )
	    continue;
	
	int rowNumber = static_cast<int>( (db.m_modules[i].m_y - bottom) / rowStep );
	if( rowNumber < (int)db.m_sites.size() && rowNumber >= 0 )
	{
	    if( db.m_sites[rowNumber].m_orient == OR_N )
	    {
		if( db.m_modules[i].m_orient != OR_N && db.m_modules[i].m_orient != OR_FN )
		{
		    printf( "Cell %d orient %s, Row %d (y %.0f) orient %s\n", 
			    i, orientStr( db.m_modules[i].m_orient ), 
			    rowNumber, db.m_modules[i].m_y, orientStr( db.m_sites[rowNumber].m_orient ) );
		}
	    }
	    else
	    {
		if( db.m_modules[i].m_orient != OR_S && db.m_modules[i].m_orient != OR_FS )
		{
		    printf( "Cell %d orient %s, Row %d (y %.0f) orient %s\n", 
			    i, orientStr( db.m_modules[i].m_orient ), 
			    rowNumber, db.m_modules[i].m_y, orientStr( db.m_sites[rowNumber].m_orient ) );
		}
	    }
	}
    }
}

// 2006-04-23 (donnie)
void CPlaceFlipping::CellFlipping( CPlaceDB& db )
{
    printf( "Cell flipping (beta)...\n" );
    fflush( stdout );
	    
    // sort cells
    vector< pair< double, int > > cellSeq;
    cellSeq.reserve( db.m_modules.size() );

    for( unsigned int i=0; i<db.m_modules.size(); i++ )
    {
	if( db.m_modules[i].m_isFixed )
	    continue;

	if( db.m_modules[i].m_height == db.m_rowHeight )
	{
	    cellSeq.push_back( make_pair( db.m_modules[i].m_x, i ) );
	}
    }
    sort( cellSeq.begin(), cellSeq.end() );
    
    double beforeWL = db.CalcHPWL();
    double bottom = db.m_sites[0].m_bottom;
    double rowStep = db.m_rowHeight;
    for( unsigned int i=0; i<cellSeq.size(); i++ )
    {
	int id = cellSeq[i].second;

	assert( db.m_modules[id].m_orient % 2 == 0 );	// N, S, FN, FS
	int bestOrient = -1;
	double wl1, wl2;
	
	int rowNumber = static_cast<int>( (db.m_modules[id].m_y - bottom) / rowStep );
	if( rowNumber < (int)db.m_sites.size() && rowNumber >= 0 )
	{
	    if( db.m_sites[rowNumber].m_orient == OR_N )
	    {
		db.SetModuleOrientation( id, OR_N );
		wl1 = db.GetModuleTotalNetLength( id );
		db.SetModuleOrientation( id, OR_FN );
		wl2 = db.GetModuleTotalNetLength( id );
		if( wl1 < wl2 )
		    bestOrient = OR_N;
		else 
		    bestOrient = OR_FN;
	    }
	    else
	    {
		db.SetModuleOrientation( id, OR_S );
		wl1 = db.GetModuleTotalNetLength( id );
		db.SetModuleOrientation( id, OR_FS );
		wl2 = db.GetModuleTotalNetLength( id );
		if( wl1 < wl2 )
		    bestOrient = OR_S;
		else 
		    bestOrient = OR_FS;
	    }
	}	   
        else
	{
	    printf( "   Cell %d out of row\n", id );
	}	    
	assert( bestOrient != -1 );
	db.SetModuleOrientation( id, bestOrient );    
    }
    
    double afterWL = db.CalcHPWL();
    printf( "  beforeWL = %.0f\n", beforeWL );
    printf( "  afterWL  = %.0f (%.02f%%)\n", afterWL, (afterWL-beforeWL)/beforeWL*100.0 );
    
}


void OutputGnuplotPM3DScript( const char* filename, const char* dat, int xrange, int yrange )
{
   FILE* file;
   file = fopen( filename, "w" );
   if( !file )
   {
       fprintf( stderr, "Cannot create a file\n" );
       return;
   }
   fprintf( file, "set pm3d corners2color c1 map\n" );
   fprintf( file, "set size ratio 1\n" );
   fprintf( file, "set cbrange [0:3]\n" );
   fprintf( file, "set xrange [0:%d]\n", xrange-1 );
   fprintf( file, "set yrange [0:%d]\n", yrange-1 );
   //fprintf( file, "set palette model RGB defined (0 \"green\", "
   //               "0.4 \"dark-green\", 0.8 \"dark-yellow\", "
   //               "1.0 \"yellow\", 1.0 \"blue\", "
   //               "1.4 \"dark-blue\", 1.4 \"red\", 3 \"dark-red\" )\n" ); 
   fprintf( file, "set palette model RGB defined (0 \"white\", "
	          "1.0 \"black\", 1.0 \"blue\", 1.4 \"dark-blue\", "
		  "1.4 \"dark-red\", 3 \"red\" )\n" ); 
   fprintf( file, "splot '%s' matrix with pm3d\n", dat );
   fprintf( file, "pause -1\n" );
   fclose( file );
}

void OutputGnuplotPM3D( const char* filename, const char* dat, 
	int xrange, int yrange, double zrangeBase, double zrange, 
	string title, double limit )
{
    char png [255];
    sprintf( png, "plot.png" );

    if( title != "" )
	sprintf( png, "%s.png", title.c_str() );

    FILE* file;
    file = fopen( filename, "w" );


    fprintf( file, "#set output \"%s\"\n", png );
    fprintf( file, "#set term png\n" );

    fprintf( file, "\nset colorbox vertical user origin .75, .15 size .04, .7\n\n" );
    
    if( title != "" )
	fprintf( file, "set title \"%s\"\n", title.c_str() );
    fprintf( file, "set pm3d corners2color c1 map\n" );
    fprintf( file, "set view map\n" );
    fprintf( file, "set size ratio 1\n" );
    fprintf( file, "set xrange [0:%d]\n", xrange-1 );
    fprintf( file, "set yrange [0:%d]\n", yrange-1 );

    if( zrange < limit )
	zrange = (int)ceil(limit);

    if( zrange > 0 )
	fprintf( file, "set cbrange [%.2f:%.2f]\n", zrangeBase, zrange );

    if( limit > 0 )
    {	
	// black and white
	fprintf( file, "set palette model RGB defined "
		"(0 \"white\", %.1f \"black\", %.1f \"dark-red\", %.1f \"red\" )\n",
		limit, limit, zrange );
	// rainbow schemes
	fprintf( file, "#set palette model RGB defined ( 0 \"dark-blue\", 1 \"light-blue\","
		" 1.5 \"green\", 2 \"yellow\", 3 \"light-red\", "
		" 4 \"dark-red\" )\n" );
    }
    else
    {
	// black and white
	fprintf( file, "#set palette model RGB defined "
		"(0 \"white\", %.1f \"black\", %.1f \"dark-red\", %.1f \"red\" )\n",
		limit, limit, zrange );
	// rainbow schemes
	fprintf( file, "set palette model RGB defined ( 0 \"dark-blue\", 1 \"light-blue\","
		" 1.5 \"green\", 2 \"yellow\", 3 \"light-red\", "
		" 4 \"dark-red\" )\n" );
    }


    fprintf( file, "#set palette model RGB defined ( 0 \"green\", 1 \"dark-green\","
	    " 2 \"yellow\", 3 \"dark-yellow\", 4 \"blue\", "
	    " 5 \"dark-blue\", 6 \"red\", 7 \"dark-red\" )\n" );

    fprintf( file, "#set palette rgbformulae 22,13,-31\n" );

    fprintf( file, "#set palette\n" );

    fprintf( file, "set pal maxcolor 33\n" );  // 2007-03-27 (donnie)
    fprintf( file, "splot '%s' matrix with pm3d\n", dat );

    fprintf( file, "\n" );
    fprintf( file, "set contour both\n" );
    fprintf( file, "set cntrparam levels 7\n" );
    fprintf( file, "set nosurface\n" );
    fprintf( file, "#splot '%s' matrix with lines\n", dat );
    fprintf( file, "pause -1\n" );
    fclose( file );
}


////////////////////////////////////////////////////

// 2007-03-30 (donnie)

void CPlaceUtil::WriteISPDGRCFormat( CPlaceDB* pDB, const char* filename )
{
    printf( "Output ISPD GR Contest File '%s'\n", filename );
    
    int binNumber = (int)( sqrt( pDB->m_modules.size() ) * 0.8 );

    // 2007-04-04 (donnie)
    if( gArg.CheckExist( "track" ) )
    {
	int trackNumber = -1;
        gArg.GetInt( "track", &trackNumber );
	assert( trackNumber != -1 );
	printf( "[INFO] Set track # to %d\n", trackNumber );
	binNumber = (int)round( (pDB->m_coreRgn.right - pDB->m_coreRgn.left) / trackNumber );
    }
    
    pDB->CreateCongMap( binNumber, binNumber );
    
    int gridX = pDB->m_pCongMap->GetHDim();
    int gridY = pDB->m_pCongMap->GetVDim();
    int nLayers = 2;	// 2D routing
    int hCap = (int)ceil( pDB->m_pCongMap->m_num_wire_per_hcut );
    int vCap = (int)ceil( pDB->m_pCongMap->m_num_wire_per_vcut );
    int minWidth = 1;
    int minSpacing = 1;
    int viaSpacing = 1;
    int lowerX = (int)floor( pDB->m_coreRgn.left );
    int lowerY = (int)floor( pDB->m_coreRgn.bottom );
    int binWidth = (int)ceil( pDB->m_pCongMap->GetHLength() ); 
    int binHeight = (int)ceil( pDB->m_pCongMap->GetVLength() );
    int netNumber = pDB->m_nets.size();
    
    int upperX = lowerX + binWidth * gridX - 1;
    int upperY = lowerY + binHeight * gridY - 1;
    printf( "[GenGR] Bounding box: %d %d to %d %d\n", lowerX, lowerY, upperX, upperY );
    printf( "   Bin width = %d   height = %d\n", binWidth, binHeight );

    // Compute capacity for each edge
    GRouteMap map( pDB );
    map.LoadCongMap( pDB->m_pCongMap );
    map.ComputeAllCapacity();

    int diffNumber = 0;
    double maxHCap = 0;
    double maxVCap = 0;
    
    for( int i=0; i<gridX; i++ )
	for( int j=0; j<gridY; j++ )
	{
	    maxHCap = max( maxHCap, map.GetHCap( i, j ) );
	    maxVCap = max( maxVCap, map.GetVCap( i, j ) );
	}    
    
    for( int i=0; i<gridX; i++ )
	for( int j=0; j<gridY; j++ )
	{
	    if( j != gridY-1 && map.GetHCap( i, j ) < maxHCap )
		diffNumber++;	
	    if( i != gridX-1 && map.GetVCap( i, j ) < maxVCap )   
		diffNumber++;
	}    

    printf( "   maxHCap = %.0f  maxVCap = %.0f\n", maxHCap, maxVCap );
    printf( "   Capacity change: %d (%d%%)\n", diffNumber, (int)100*diffNumber/(gridX*gridY*2) );

    // For ISPD 05/06. 6 metal layers. M1/M2 are fully blocked.
    hCap = (int)ceil( maxHCap );	
    vCap = (int)ceil( maxVCap );
        
    FILE* out = fopen( filename, "w" );
    fprintf( out, "grid %d %d %d\n", gridX, gridY, nLayers );
    fprintf( out, "vertical capacity 0 %d\n", vCap );
    fprintf( out, "horizontal capacity %d 0\n", hCap );
    fprintf( out, "minimum width   %d %d\n", minWidth, minWidth );
    fprintf( out, "minimum spacing %d %d\n", minSpacing, minSpacing );
    fprintf( out, "via spacing     %d %d\n", viaSpacing, viaSpacing );
    fprintf( out, "%d %d %d %d\n\n", lowerX, lowerY, binWidth, binHeight );

    // 2006-04-04 (donnie) Add 1 safe margin for the global router
    lowerX++;
    lowerY++;

    fprintf( out, "num net %d\n", netNumber );
    for( unsigned int i=0; i<pDB->m_nets.size(); i++ )
    {
	fprintf( out, "n%d %d %d 1\n", i, i, pDB->m_nets[i].size() );
	for( unsigned int j=0; j<pDB->m_nets[i].size(); j++ )
	{
	    int pinId = pDB->m_nets[i][j];
	    int x = (int)round( pDB->m_pins[pinId].absX );
	    int y = (int)round( pDB->m_pins[pinId].absY );
	    x = max( lowerX, x );
	    x = min( upperX, x );
	    y = max( lowerY, y );
	    y = min( upperY, y ); 
	    fprintf( out, "%d\t%d\t1\n", x, y );
	} 
    }

    // Output capacity adjustment
    fprintf( out, "\n\n%d\n", diffNumber );
    for( int i=0; i<gridX; i++ )
	for( int j=0; j<gridY; j++ )
	{
	    if( j != gridY-1 && map.GetHCap( i, j ) < maxHCap )	
	    {
		fprintf( out, "%d %d 2\t%d %d 2\t %d\n", i, j, i, j+1, (int)ceil( map.GetHCap( i, j ) ) ); 
	    }
	    if( i != gridX-1 && map.GetVCap( i, j ) < maxVCap )
	    {
		fprintf( out, "%d %d 1\t%d %d 1\t %d\n", i, j, i+1, j, (int)ceil( map.GetVCap( i, j ) ) ); 
	    }
	}    

    fprintf( out, "\n" );
    fclose( out ); 
}

void CPlaceUtil::WriteXGRFormat( CPlaceDB* pDB, const char* filename )
{
    printf( "Output GRouting file '%s'\n", filename );

    //double binWidth =  24;
    //double binHeight = 24;
    //double wireWidth =  2;
    double binWidth =  30;
    double binHeight = 30;
    double wireWidth =  1.5;
    int layerV = 2;
    int layerH = 2;

    double chipWidth = pDB->m_coreRgn.right - pDB->m_coreRgn.left;
    double chipHeight = pDB->m_coreRgn.top - pDB->m_coreRgn.bottom;
    double x0 = pDB->m_coreRgn.left;
    double y0 = pDB->m_coreRgn.bottom;

    // Compute real grid number
    double gridW = floor( chipWidth / binWidth );
    double gridH = floor( chipHeight / binHeight );

    // Update real grid size
    binWidth = chipWidth / gridW;
    binHeight = chipHeight / gridH;

    double capVertical = ceil( binHeight / wireWidth * layerV);
    double capHorizontal = ceil( binWidth / wireWidth * layerH );

    if( pDB->m_pLEF != NULL )
    {
	binWidth  = pDB->m_gCellWidth;
	binHeight = pDB->m_gCellHeight;
	gridW     = pDB->m_gCellNumberHorizontal;
	gridH     = pDB->m_gCellNumberVertical;
	x0        = pDB->m_gCellLeft;
	y0        = pDB->m_gCellBottom;

	// calculate `capVertical' and `capHorizontal'
	capVertical = 0;
	capHorizontal = 0;

	for( unsigned int i=0; i<pDB->m_pLEF->m_metalPitch.size(); i++ )
	{
	    if( i == 1 )
	    {
		printf( "SKIP METAL 1\n" );
		continue;
	    }

	    if( pDB->m_pLEF->m_metalDir[i] == CParserLEFDEF::VERTICAL )
	    {
		int num = (int)round( binWidth / ( pDB->m_pLEF->m_metalPitch[i] * pDB->m_pLEF->m_defUnit ) );
		capHorizontal += num;
		printf( "GCELL METAL %d VERTICAL PITCH %.2f TRACK %d\n", i, pDB->m_pLEF->m_metalPitch[i], num );
	    }

	    if( pDB->m_pLEF->m_metalDir[i] == CParserLEFDEF::HORIZONTAL )
	    {
		int num = (int)round( binHeight / ( pDB->m_pLEF->m_metalPitch[i] * pDB->m_pLEF->m_defUnit ) );
		capVertical += num;
		printf( "GCELL METAL %d HORIZONTAL PITCH %.2f TRACK %d\n", i, pDB->m_pLEF->m_metalPitch[i], num );
	    }
	}
    }
    
    printf( "  grid %.0f %.0f\n", gridW, gridH );
    printf( "  vertical capacity %.0f\n", capVertical );
    printf( "  horizontal capacity %.0f\n", capHorizontal );
    printf( "  vertical grid length %.2f\n", binHeight );
    printf( "  horizontal grid length %.2f\n", binWidth );
    printf( "  num net %d\n", pDB->m_nets.size() );
  

    FILE* out = fopen( filename, "w" );
    fprintf( out, "grid %.0f %.0f\n", gridW, gridH );
    fprintf( out, "vertical capacity %.0f\n", capVertical );
    fprintf( out, "horizontal capacity %.0f\n", capHorizontal );
    fprintf( out, "num net %d\n", pDB->m_nets.size() );

    vector< vector<int> > pinCount;
    pinCount.resize( (int)round(gridW) );
    for( int i=0; i<gridW; i++ )
	pinCount[i].resize( (int)round(gridH), 0 );

    for( unsigned int i=0; i<pDB->m_nets.size(); i++ )
    {
	fprintf( out, "net%d %d %d\n", i, i, pDB->m_nets[i].size() );

	for( unsigned int j=0; j<pDB->m_nets[i].size(); j++ )
	{
	    double x, y;
	    pDB->GetPinLocation( pDB->m_nets[i][j], x, y );
	    double binX = floor( (x - x0)/binWidth );
	    double binY = floor( (y - y0)/binHeight );
	    if( binX < 0 )   binX = 0;
	    if( binY < 0 )   binY = 0;
	    if( binX >= gridW )  binX = gridW-1;
	    if( binY >= gridH )  binY = gridH-1; 
	    fprintf( out, "  %.0f %.0f\n", binX, binY );
	    pinCount[(int)binX][(int)binY]++;
	}
    }
    fclose( out );

    // Plot pin density map
    string name = string( filename ) + ".plt";
    FILE* outPlt = fopen( name.c_str(), "w" );
    fprintf( outPlt, "set pm3d map\n" );
    fprintf( outPlt, "set size ratio 1\n" );
    fprintf( outPlt, "splot \"dat/%s.plt.dat\" matrix with pm3d\n", filename );
    fprintf( outPlt, "pause -1\n" );
    fclose( outPlt );

    string name2 = "dat/" + string( filename ) + ".plt.dat";
    FILE* out2 = fopen( name2.c_str(), "w" );
    for( unsigned int y=0; y<pinCount[0].size(); y++ )
    {
	for( unsigned int x=0; x<pinCount.size(); x++ )
	    fprintf( out2, "%d ", pinCount[x][y] );
	fprintf( out2, "\n" );
    }
    fclose( out2 );

}

// 2006-09-23 (donnie)
void CPlaceUtil::CreateRowSites( CPlaceDB* pDB, const double& utilization, const double& aspectRatio )
{
    // aspectRatio = height / width
    
    double totalBlockArea = 0;
    for( unsigned int i=0; i<pDB->m_modules.size(); i++ )
	//if( !pDB->m_modules[i].m_isOutCore )
	if( !pDB->m_modules[i].m_isFixed )
	    totalBlockArea += pDB->m_modules[i].m_area;

    if( totalBlockArea == 0 )
    {
	printf( "Error, total block area = 0\n" );
	return;
    }

    // dieArea * utilization = A
    double dieArea = totalBlockArea / utilization;
    double rowHeight = pDB->m_rowHeight;

    // height * width = dieArea
    // height / width = aspectRatio 

    double height = sqrt( dieArea * aspectRatio );
    int nRows = static_cast<int>( height / rowHeight );
    height = nRows * rowHeight;
    double width = floor( dieArea / height );
    dieArea = width * height;
    double step = pDB->m_sites[0].m_step;

    printf( "\n" );
    printf( "[Create Sites]\n" );
    printf( "    Site Width       = %.0f\n", step );
    printf( "    Row Height       = %.0f\n", rowHeight );
    printf( "    Row #            = %d\n",   nRows );
    printf( "    Die Height       = %.0f\n", height );
    printf( "    Die Width        = %.0f\n", width );
    printf( "    Total Block Area = %.0f\n", totalBlockArea );
    printf( "    Die Area         = %.0f\n", dieArea );
    printf( "    Utilization      = %.2f%%\n", totalBlockArea/dieArea*100 );
    printf( "\n" );
   
    pDB->m_coreRgn.right = pDB->m_coreRgn.left + width;
    pDB->m_coreRgn.top   = pDB->m_coreRgn.bottom + height;

    pDB->m_sites.clear();

    for( int i=0; i<nRows; i++ )
    {
	pDB->m_sites.push_back( CSiteRow( pDB->m_coreRgn.bottom + rowHeight*i, rowHeight, step ) );
	pDB->m_sites.back().m_interval.push_back( pDB->m_coreRgn.left );
	pDB->m_sites.back().m_interval.push_back( pDB->m_coreRgn.right );
    }
    
}



// 2006-10-21 /////////////////////////////////////////////////////////////////

void CPlaceUtil::SavePlacement( const CPlaceDB& db, vector<CPoint>& pos )    // static
{
    pos.clear();
    pos.reserve( db.m_modules.size() );
    for( unsigned int i=0; i<db.m_modules.size(); i++ )
	pos.push_back( CPoint( db.m_modules[i].m_x, db.m_modules[i].m_y ) );
}

void CPlaceUtil::LoadPlacement( CPlaceDB& db, const vector<CPoint>& pos )   // static
{
    for( unsigned int i=0; i<db.m_modules.size(); i++ )
	db.SetModuleLocation( i, pos[i].x, pos[i].y );
}

double CPlaceUtil::GetDisplacement( const vector<CPoint>& p1, const vector<CPoint>& p2 )   // static
{
    double dis = 0;
    assert( p1.size() == p2.size() );
    for( unsigned int i=0; i<p1.size(); i++ )
	dis += fabs( p1[i].x - p2[i].x ) + fabs( p1[i].y - p2[i].y );
    return dis;
}

double CPlaceUtil::GetMaxDisplacement( const vector<CPoint>& p1, const vector<CPoint>& p2 )   // static
{
    double dis = 0;
    assert( p1.size() == p2.size() );
    for( unsigned int i=0; i<p1.size(); i++ )
	dis = max( dis, fabs( p1[i].x - p2[i].x ) + fabs( p1[i].y - p2[i].y ) );
    return dis;
}

int CPlaceUtil::GetDisplacementCount( const vector<CPoint>& p1, const vector<CPoint>& p2, const double dis )
{
    int count = 0;
    for( unsigned int i=0; i<p1.size(); i++ )
	if( fabs( p1[i].x - p2[i].x ) + fabs( p1[i].y - p2[i].y ) > dis )
	    count++;
    return count;
}

// 2007-07-18 (donnie)
void CPlaceUtil::ShowMigrationInfo( /*const*/ CPlaceDB& db, const vector<CPoint>& p1, const vector<CPoint>& p2 )  // static
{
    double bbox = ( db.m_coreRgn.right - db.m_coreRgn.left ) + ( db.m_coreRgn.top - db.m_coreRgn.bottom );
    double totalDisplacement = GetDisplacement( p1, p2 );
    double avgDisplacement = totalDisplacement / db.m_modules.size();
    double maxDisplacement = GetMaxDisplacement( p1, p2 );
    int moveCount = GetDisplacementCount( p1, p2, bbox * 0.015 );

    vector<CPoint> positions;
    SavePlacement( db, positions );

    LoadPlacement( db, p1 );
    double oldHPWL = db.CalcHPWL();
    
    vector<double> netLength;
    netLength.resize( db.m_nets.size() );
    for( unsigned int i=0; i<db.m_nets.size(); i++ )
	netLength[i] = db.GetNetLength( i );

    LoadPlacement( db, p2 );
    double newHPWL = db.CalcHPWL();

    double totalNetLengthChange = 0;
    double maxNetLengthChange = 0;
    for( unsigned int i=0; i<db.m_nets.size(); i++ )
    {
	double lenChange = fabs( db.GetNetLength( i ) - netLength[i] );
	maxNetLengthChange = max( maxNetLengthChange, lenChange );
	totalNetLengthChange += lenChange;
    }
    double avgNetLengthChange = totalNetLengthChange / db.m_nets.size();
   
    LoadPlacement( db, positions ); 
    
    printf( "\n" );
    printf( "# Average displacement per cell:         %.2f%% BBox\n", 100.0 * avgDisplacement / bbox );
    printf( "# Max displacement of the cell:          %.2f%% BBox\n", 100.0 * maxDisplacement / bbox );
    printf( "# Cell number with > 1.5%% BBox movement: %.2f%%\n",     100.0 * moveCount / (double)db.m_modules.size() );
    printf( "# HPWL change:                           %.2f%%\n",      100.0 * (newHPWL-oldHPWL) / oldHPWL );
    printf( "# Average net length change per net:     %.2f%% BBox\n", 100.0 * avgNetLengthChange / bbox );
    printf( "# Max net length change of the net:      %.2f%% BBox\n", 100.0 * maxNetLengthChange / bbox );
    printf( "\n" );
}

double CPlaceUtil::GetTotalSiteWidth( vector<CSiteRow>& sites )
{
    double w = 0;
    for( unsigned int y=0; y<sites.size(); y++ )
    {
	assert( sites[y].m_interval.size() % 2 == 0 );
	for( unsigned int x=0; x<sites[y].m_interval.size(); x+=2  )
	{
	    assert( sites[y].m_interval[x+1] >= sites[y].m_interval[x] );
	    w += sites[y].m_interval[x+1] - sites[y].m_interval[x];
	}
    }
    return w;
}

void CPlaceUtil::SimulateGateSizing( CPlaceDB* db )
{
    printf( "Simulate gate sizing\n" );
}

double CPlaceUtil::GetTotalOverlapArea( CPlaceDB& db )
{
    double timeStart = seconds();
    vector<CSiteRow> backupSites = db.m_sites;
    vector<CPoint> pos;
    SavePlacement( db, pos ); 

    db.RemoveFixedBlockSite();
    double oldSpace = GetTotalSiteWidth( db.m_sites );
    
    vector<bool> m_movBlock;
    m_movBlock.resize( db.m_modules.size(), false );
    double totalBlockSiteWidth = 0;
    int blockCount = 0;
    int outCount = 0;
    for( unsigned int i=0; i<db.m_modules.size(); i++ )
    {
	if( db.m_modules[i].m_isFixed )
	    continue;
	if( db.BlockOutCore( i ) )
	{
	    if( db.m_modules[i].m_height >= db.m_rowHeight )
		outCount++;
	    continue;
	}
	// snap to the cloest site
	double newX = round( db.m_modules[i].m_x );
	double newY = round( (db.m_modules[i].m_y - db.m_coreRgn.bottom) / db.m_rowHeight ) 
				* db.m_rowHeight + db.m_coreRgn.bottom;
	db.SetModuleLocation( i, newX, newY );
	
	blockCount++;
	m_movBlock[i] = true;
	db.m_modules[i].m_isFixed = true;
	totalBlockSiteWidth += db.m_modules[i].m_width * ceil( db.m_modules[i].m_height / db.m_rowHeight );
    }
    db.RemoveFixedBlockSite();

    double newSpace = GetTotalSiteWidth( db.m_sites );
    double useSpace = oldSpace - newSpace;
    double overWidth = totalBlockSiteWidth - useSpace;
    double overArea = overWidth * db.m_rowHeight;

    printf( "\n" );
    printf( "# Total movable block number:         %d\n", blockCount );
    if( outCount > 0 )
    printf( "# NOTE: %d blocks are outside the core.\n", outCount );
    printf( "# Overlap area of placement region:   %.2f%%\n", 100*overWidth/oldSpace );
    printf( "# Overlap area of movable block area: %.2f%%\n", 100*overWidth/totalBlockSiteWidth );
    printf( "# Overlap analysis CPU:               %.2f sec\n", seconds() - timeStart );
    printf( "\n" );

    db.m_sites = backupSites;
    for( unsigned int i=0; i<db.m_modules.size(); i++ )
	db.m_modules[i].m_isFixed = !m_movBlock[i];
    LoadPlacement( db, pos );
    
    return overArea;
}


// 2007-01-25 ///////////////////////////////////////////////////////////////////
CPlacePlotWithNet::CPlacePlotWithNet( CPlaceDB* pDB )
{
    m_pDB = pDB;
}

void CPlacePlotWithNet::OutputGnuplotFigure( const char* filename )
{
    CreateXSteinerNets();
    
    FILE* file = fopen( filename, "w" );

    PlotHeader( file );
    PlotCells( file );
    for( int layer=0; layer<4; layer++ )
	PlotNets( file, layer );

    fprintf( file, "\npause -1 'Press any key'\n" );

    fclose( file );
}

void CPlacePlotWithNet::PlotHeader( FILE* file )
{
    fprintf( file, "set title \" WL: H %.0f V %.0f +45 %.0f -45 %.0f Total %.0f\" font \"Times, 22\"\n",
	  m_totalLength[0], m_totalLength[1], m_totalLength[2], m_totalLength[3],
	 m_totalLength[0]+m_totalLength[1]+m_totalLength[2]+m_totalLength[3] );
    fprintf( file, "set size ratio 1\n" );
    fprintf( file, "set nokey\n" );
    fprintf( file, "plot[:][:] '-' w l lt 3, '-' w l lt 1, '-' w l lt 6, '-' w l lt 2, '-' w l lt 5\n\n" );
    //                              cell          H          V        +45        -45
    //                              blue         red      brown     lt green   lt blue

    // core
    fprintf( file, "%.3f, %.3f\n", m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.top );
    fprintf( file, "%.3f, %.3f\n", m_pDB->m_coreRgn.right, m_pDB->m_coreRgn.top );
    fprintf( file, "%.3f, %.3f\n", m_pDB->m_coreRgn.right, m_pDB->m_coreRgn.bottom );
    fprintf( file, "%.3f, %.3f\n", m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.bottom );
    fprintf( file, "%.3f, %.3f\n\n", m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.top );
}

void CPlacePlotWithNet::PlotCells( FILE* file )
{
    for( unsigned int i=0; i<m_pDB->m_modules.size(); i++ )
    {
	double left   = m_pDB->m_modules[i].m_x;
	double right  = left + m_pDB->m_modules[i].m_width;
	double bottom = m_pDB->m_modules[i].m_y;
	double top    = bottom + m_pDB->m_modules[i].m_height;
	fprintf( file, "%.3f, %.3f\n", left, bottom );
	fprintf( file, "%.3f, %.3f\n", right, bottom );
	fprintf( file, "%.3f, %.3f\n", right, top );
	fprintf( file, "%.3f, %.3f\n", left, top );
	fprintf( file, "%.3f, %.3f\n\n", left, bottom );
    }
    fprintf( file, "EOF\n\n" );
}

void CPlacePlotWithNet::PlotNets( FILE* file, const int& layer )
{
    for( unsigned int i=0; i<m_segments[layer].size(); i+=2 )
    {
	fprintf( file, "%.3f, %.3f\n",   m_segments[layer][i].x, m_segments[layer][i].y );
	fprintf( file, "%.3f, %.3f\n\n", m_segments[layer][i+1].x, m_segments[layer][i+1].y );
    }
    fprintf( file, "EOF\n\n" );
}


void CPlacePlotWithNet::CreateXSteinerNets() 
{
    m_segments.clear();
    m_segments.resize( 4 );	// 4 layers   (H, V, +45, -45)

    for( unsigned int n=0; n<m_pDB->m_nets.size(); n++ )
	DecomposeXSteinerNet( m_pDB->m_nets[n] );

    ComputeTotalLength();
}

void CPlacePlotWithNet::ComputeTotalLength()
{
    m_totalLength.resize( 4, 0 );
    for( unsigned int i=0; i<2; i++ )
	for( unsigned int p=0; p<m_segments[i].size(); p+=2 )
	    m_totalLength[i] += CPoint::Distance( m_segments[i][p], m_segments[i][p+1] );
    for( unsigned int i=2; i<4; i++ )
	for( unsigned int p=0; p<m_segments[i].size(); p+=2 )
	    m_totalLength[i] += CPoint::XDistance( m_segments[i][p], m_segments[i][p+1] );

    printf( "XSteiner WL\n" );
    printf( "   Horizontal: %.0f\n", m_totalLength[0] );
    printf( "   Vertical:   %.0f\n", m_totalLength[1] );
    printf( "   +45 degree: %.0f\n", m_totalLength[2] );
    printf( "   -45 degree: %.0f\n", m_totalLength[3] );
    printf( "   Total:      %.0f\n", m_totalLength[0]+m_totalLength[1]+m_totalLength[2]+m_totalLength[3] );
}

void CPlacePlotWithNet::DecomposeXSteinerNet( const Net& curNet )
{

    // steinerType 0    FLUTE (default)
    // steinerType 1    Minimum spanning tree (MST)

    int steinerType = 0; 

    if( curNet.size() <= 1 )
       return;

    if( curNet.size() >= 501 )
	steinerType = 1; // FLUTE bug?

    if( 2 == curNet.size() )
    {
	double x, y;
	m_pDB->GetPinLocation( curNet[0], x, y );
	CPoint p1(x,y);
	m_pDB->GetPinLocation( curNet[1], x, y );
	CPoint p2(x,y);
	CreateXSegments( p1, p2 );
	return;
    }

    // curNet.size() > 2 

    // Construct the steiner tree 
    vector<CPoint> points;
    for( unsigned int j = 0 ; j < curNet.size() ; j++ )
    {
	double x, y;
	m_pDB->GetPinLocation( curNet[j], x, y );
	points.push_back( CPoint( x, y ) );
    }

    if( steinerType == 1 )
    {
	CMinimumSpanningTree mst;
	mst.Solve( points );
	assert( mst.m_mst_array.size() % 2 == 0 );
	for( unsigned int j = 0 ; j < mst.m_mst_array.size() ; j=j+2 )
	    CreateXSegments( mst.m_mst_array[j], mst.m_mst_array[j+1] );
	return;
    }
    else
    {
	if( pgFlute == NULL )
	{
	    printf( "Create FLUTE\n" );
	    fflush( stdout );
	    pgFlute = new CFlute();
	}
	pgFlute->Solve( points );
	assert( pgFlute->m_steiner_array.size() % 2 == 0 );
	for( unsigned int j=0; j<pgFlute->m_steiner_array.size(); j=j+2 )
	    CreateXSegments( pgFlute->m_steiner_array[j], pgFlute->m_steiner_array[j+1] );
	return;
    }
}

void CPlacePlotWithNet::CreateXSegments( const CPoint& pOne, const CPoint& pTwo )
{
    CPoint p1, p2;	// p1.x <= p2.x
    if( pOne.x > pTwo.x )
    {
	p1 = pTwo;
	p2 = pOne;
    }
    else
    {
	p1 = pOne;
	p2 = pTwo;
    }
	
    double horizontalDiff = fabs( p1.x - p2.x );
    double verticalDiff   = fabs( p1.y - p2.y );

    if( horizontalDiff == verticalDiff )
    {
	if( horizontalDiff == 0 )
	    return;
	else
	{
	    if( p1.y < p2.y )
	    {
		// +45
		m_segments[2].push_back( p1 );
		m_segments[2].push_back( p2 );
	    }
	    else 
	    {
		// -45
		assert( p1.y > p2.y );
		m_segments[3].push_back( p1 );
		m_segments[3].push_back( p2 );
	    }
	    return;
	}
    }

    if( horizontalDiff == 0 )
    {
	// Vertical
	m_segments[1].push_back( p1 );
	m_segments[1].push_back( p2 );
	return;
    }

    if( verticalDiff == 0 )
    {
	// Horizontal
	m_segments[0].push_back( p1 );
	m_segments[0].push_back( p2 );
	return;
    }

    double diff = fabs( horizontalDiff - verticalDiff );
    double position = 0;
    //  position 0          position 1
    //  ---------p2        p1----------
    //  |         |        |          |
    //  p1---------        ----------p2
    if( p1.y > p2.y )
	position = 1;

    if( horizontalDiff > verticalDiff )
    {
	// H
	m_segments[0].push_back(p1);
	m_segments[0].push_back( CPoint( p1.x+diff, p1.y ) );
        if( position == 0 )
	{
	    // +45
	    m_segments[2].push_back( CPoint( p1.x+diff, p1.y ) );
	    m_segments[2].push_back(p2);
	}
	else
	{
	    // -45
	    m_segments[3].push_back( CPoint( p1.x+diff, p1.y ) );
	    m_segments[3].push_back(p2);
	}
    }	
    else
    {
	// horizontalDiff < verticalDiff 
	if( position == 0 )
	{
	    // V
	    m_segments[1].push_back(p1);
	    m_segments[1].push_back( CPoint( p1.x, p1.y+diff ) );
	    // +45
	    m_segments[2].push_back( CPoint( p1.x, p1.y+diff ) );
	    m_segments[2].push_back(p2);
	}
	else
	{
	    // V
	    m_segments[1].push_back(p1);
	    m_segments[1].push_back( CPoint( p1.x, p1.y-diff ) );
	    // -45
	    m_segments[3].push_back( CPoint( p1.x, p1.y-diff ) );
	    m_segments[3].push_back(p2);
	}
    }

}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CMatrixPlotter::OutputGnuplotFigure( 
	const vector< vector< double > >& matrix, 
	const char* filename, 
	string title, 
	double limit,
        bool scale,
        double limitBase )
{
    string plt = string( filename ) + ".plt" ;
    string dat = "dat/" + string( filename ) + ".plt.dat";
    system( "mkdir dat 1> /dev/null 2> /dev/null" );

    double maxZ = 0;
    for( unsigned int y=0; y<matrix[0].size(); y++ )
	for( unsigned int x=0; x<matrix.size(); x++ )
	    if( matrix[x][y] > maxZ )
		maxZ = matrix[x][y];
    
    if( scale && limit > 0 )
	OutputGnuplotPM3D( plt.c_str(), dat.c_str(), matrix.size()+1, matrix[0].size()+1, 
		limitBase, (maxZ/limit), title, 1.0 );
    else
	OutputGnuplotPM3D( plt.c_str(), dat.c_str(), matrix.size()+1, matrix[0].size()+1, 
		limitBase, (maxZ), title, limit );

    FILE* out = fopen( dat.c_str(), "w" );
    for( unsigned int yy=0; yy<matrix[0].size()+1; yy++ )
    {
	unsigned int y = yy;
	if( y == matrix[0].size() )
	    y -= 1;
	for( unsigned int xx=0; xx<matrix.size()+1; xx++ )
	{
	    unsigned int x = xx;
	    if( x == matrix.size() )
		x -= 1;
	    if( scale && limit > 0 )
		fprintf( out, "%.2f ", matrix[x][y]/limit );
	    else
		fprintf( out, "%.2f ", matrix[x][y] );
	}
	fprintf( out, "\n" );
    }
    fclose( out );
}

void CMatrixPlotter::OutputGnuplotFigure( 
	const vector< vector< float > >& matrix, 
	const char* filename, 
	string title, 
	float limit,
        bool scale,
        float limitBase )
{
    string plt = string( filename ) + ".plt" ;
    string dat = "dat/" + string( filename ) + ".plt.dat";
    system( "mkdir dat 1> /dev/null 2> /dev/null" );

    float maxZ = 0;
    for( unsigned int y=0; y<matrix[0].size(); y++ )
	for( unsigned int x=0; x<matrix.size(); x++ )
	    if( matrix[x][y] > maxZ )
		maxZ = matrix[x][y];
    
    if( scale && limit > 0 )
	OutputGnuplotPM3D( plt.c_str(), dat.c_str(), matrix.size()+1, matrix[0].size()+1, 
		limitBase, (maxZ/limit), title, 1.0 );
    else
	OutputGnuplotPM3D( plt.c_str(), dat.c_str(), matrix.size()+1, matrix[0].size()+1, 
		limitBase, (maxZ), title, limit );

    FILE* out = fopen( dat.c_str(), "w" );
    for( unsigned int yy=0; yy<matrix[0].size()+1; yy++ )
    {
	unsigned int y = yy;
	if( y == matrix[0].size() )
	    y -= 1;
	for( unsigned int xx=0; xx<matrix.size()+1; xx++ )
	{
	    unsigned int x = xx;
	    if( x == matrix.size() )
		x -= 1;
	    if( scale && limit > 0 )
		fprintf( out, "%.2f ", matrix[x][y]/limit );
	    else
		fprintf( out, "%.2f ", matrix[x][y] );
	}
	fprintf( out, "\n" );
    }
    fclose( out );
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void CPlaceFlux::OutputGnuplotFigure( const char* filename )
{
    CMatrixPlotter::OutputGnuplotFigure( m_flux, filename );
}

void CPlaceFlux::UpdateFlux( 
	const double& x, const double& y, 
	const double& width, const double& height,
        const int& xNum, const int& yNum )
{
    m_flux.resize( xNum );
    for( int i=0; i<xNum; i++ )
	m_flux[i].resize( yNum );

    for( int i=0; i<xNum; i++ )
	for( int j=0; j<yNum; j++ )
	{
	    double x1 = x + i * width;
	    double y1 = y + j * height;
	    m_flux[i][j] = ::GetRegionNetFluxOverflow( m_pDB, x1, y1, x1 + width, y1 + height );
	}
}

void CPlaceFlux::UpdateFlux( const int& xNum, const int& yNum )
{
    double w = m_pDB->m_coreRgn.right - m_pDB->m_coreRgn.left;
    double h = m_pDB->m_coreRgn.top - m_pDB->m_coreRgn.bottom;
    UpdateFlux( m_pDB->m_coreRgn.left, m_pDB->m_coreRgn.bottom,
	w / xNum, h / yNum, xNum, yNum );
}
////////////////////////////////////////////////////////////////////////////////////////////////
