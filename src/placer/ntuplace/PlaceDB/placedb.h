#include "../PlaceCommon/util.h"
#ifndef PLACEDB_H
#define PLACEDB_H
/**
 *
@author Indark#eda
Placement Database
2005-10-25
*/

// 2007-07-17 (donnie)
// I removed the liberty information connected to CPlaceDB to save memory.
// The liberty info should not be embedded into Pins directly to save memory.


#include <cstdio>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <cassert>
#include <set>

// kaie
#include <set>
#include <cassert>
#include <cmath>
#include <list>
#include <bitset>
#include <cstring>
//

using namespace std;
 



class CParserLEFDEF;
class Liberty;

//Added by Jin 20060629 
class CCongMap;

//Added by Jin 20070306
class CFixedPointMethod;
class CSteinerDecomposition;

extern double programStartTime;
extern CParserLEFDEF parserLEFDEF;

//Added by Jin 20081013
class TimingAnalysis;

enum ORIENT{
	OR_N,OR_W,OR_S,OR_E,OR_FN,OR_FW,OR_FS,OR_FE
//         0    1    2    3    4     5     6     7	    
//         |    -    |    -    |     -     |     -
	};


class CSiteRow
{
public:
    static bool Lesser( const CSiteRow& r1, const CSiteRow& r2  ) 
    { 
        return ( r1.m_bottom < r2.m_bottom );
    }
    static bool Greater( const CSiteRow& r1, const CSiteRow& r2  ) 
    { 
        return ( r1.m_bottom > r2.m_bottom );
    }


public:
    bool isInside( const double& x, const double width )
    {
        vector<double>::const_iterator ite;
        for( ite=m_interval.begin(); ite!=m_interval.end(); ite+=2 )
        {
            if( *ite > x )
                return false;
            //cout << "  sites(" << *ite << " " << *(ite+1) << ") ";
            if( *ite <= x && *(ite+1) >= x )
                return true;
        }
        return false;
    }

public:
	CSiteRow(void);
	~CSiteRow(void);
	CSiteRow(const double& set_bottom, const double& set_height, const double& set_step ) :
	    m_bottom( set_bottom ), 
	    m_height( set_height ), 
	    m_step( set_step ), 
	    m_orient( OR_N )
	{}
	
	double m_bottom;        // The bottom y coordinate of this row of sites
	double m_height;        // The height of this row of sites
	double m_step;		//The minimum x step of row.	by indark

    float  m_z;		// (kaie) 2009-07-05  The layer number of this row of sites 
	string m_name;		// (kaie) 2009-12-04
	string m_macro;		// (kaie) 2009-12-04

	std::vector<double> m_interval;
	ORIENT m_orient;	// donnie 2006-04-23  N (0) or S (1)
};


const int PIN_DIRECTION_OUT = 0;
const int PIN_DIRECTION_IN = 1;
const int PIN_DIRECTION_UNDEFINED = -1;

class Pin
{
    public:
	Pin(): direction(PIN_DIRECTION_UNDEFINED) /*: libPinId(-1)*/ {}

	Pin( const string& name, const float& x, const float& y ) 
	    : pinName(name), xOff(x), yOff(y), direction(PIN_DIRECTION_UNDEFINED)/*,*/ /*direction(-1)*/ {}   // 2005-08-29
	Pin( const float& x, const float& y ) 
	    : xOff(x), yOff(y), direction(PIN_DIRECTION_UNDEFINED) /*, libPinId(-1),*/ /*direction(-1)*/ {}

	string	pinName;
	int	moduleId;
	float	xOff;
	float	yOff;
	float	absX;
	float	absY;

	//Added by Jin 20081008
	int    direction; // 0 output  1 input  -1 not-define
	//@Added by Jin 20081008
#if 0
	int     libPinId;  // 2006-06-19 (donnie)
	//short int direction; // 0 output  1 input  -1 not-define
	char    direction; // 0 output  1 input  -1 not-define   // 2007-07-17 (donnie) save memory
#endif
};

enum BLOCK_TYPE { BT_OTHER, BT_PI, BT_PO, BT_P_INOUT, BT_PIN, BT_DFF };

class Module
{
    public:
	
	Module()
	{
	    Init();
	}

	Module( string name, float width=0, float height=0, bool isFixed=false, bool isNI=false ) 
	{
	    Init();
	    m_x = (float)0.0;
	    m_y = (float)0.0;
        m_z = (float)0.0;
	    m_name    = name;
	    m_width   = width;
	    m_height  = height;
        m_thickness = (float)1.0;
	    m_area    = width*height;
	    m_isFixed = isFixed;
        m_isNI    = isNI; // 2022-05-13 (frank)
	    assert( m_area >= 0 );
	}
	
	void Init()
	{
	    m_width  = -1;
	    m_height = -1;
	    m_area   = -1;
	    m_orient =  0;
	    m_lefCellId = -1;
	    m_libCellId = -1;
	    m_isDummy   = false;
	    m_isCluster = false;
	    m_blockType = BT_OTHER;
	    m_isFixed   = false;
        m_isNI      = false; // 2022-05-13 (frank)
	}
	
	void CalcCenter()
	{ 
	    m_cx = m_x + (float)0.5 * m_width;
	    m_cy = m_y + (float)0.5 * m_height;
        m_cz = m_z + (float)0.5 * m_thickness;
	}

	string GetName()		{ return m_name; } 
	float GetWidth()		{ return m_width; }
	float GetHeight()		{ return m_height; }
    float GetThickness()		{ return m_thickness; }
	float GetX()			{ return m_x; }
	float GetY()			{ return m_y; }
    float GetZ()			{ return m_z; }	//(kaie) 2009-07-05 vertical position
	void  SetZ(const float _z)	{ m_z = _z; }
	float GetArea()			{ return m_area; }
	short int GetOrient()		{ return m_orient; }

	float  m_x, m_y;
	float  m_cx, m_cy;
    float  m_z, m_cz; //(kaie) 2009-07-05 vertical position
	string m_name;
	float  m_width, m_height;
    float  m_thickness;
	float  m_area;
	char   m_orient;	    // 2005-10-20 (donnie) current orientation  2007-07-17 (donnie) save memory
	//short int m_orient;	    // 2005-10-20 (donnie) current orientation
	//short int m_type;	    // module type id (in cell library)
	short int m_lefCellId;	    // 2006-06-22 (donnie) LEF Cell ID
	short int m_libCellId;	    // 2006-06-19 (donnie) Liberty Cell ID
	bool   m_isFixed;	    // fixed block or pads
	//bool   m_isPin;             // 2006-05-22 determine PIs and POs 
	//bool   m_isOutCore;
	bool   m_isDummy;	    // 2006-03-02 (donnie)
	bool   m_isCluster;         // 2006-03-20 (tchsu)
	bool   m_isMacro;	    // 2007-01-05 (indark)
	BLOCK_TYPE m_blockType;	    // 2006-06-22 (donnie)
    bool   m_isNI;          // 2022-05-13 (frank)

	vector<int> m_pinsId;
	vector<int> m_netsId;

	vector<int> m_inBlocksId;	// for TimeMachine
	vector<int> m_outBlocksId;	// for TimeMachine

	bool isRotated();
};

typedef vector<int> Net;


class CPlaceDB
{

    friend class ParserBookshelf;
    friend class CBinPacking;
    friend class CLegalizer;
    friend class CNetinfo;
    friend class permutation;
    friend class CDiamondLegalizer;
    friend class CPrelegalizer;
    friend class de_Detail; // assignment detailed placer
    friend class CParserLEFDEF;
    friend class CParserIBMMix;

    public:
    CPlaceDB(  );
    ~CPlaceDB(void);
    void Init();


    // 2005-12-18 speedup #donnie
    void ReserveModuleMemory( const int& n );
    void ReserveNetMemory( const int& n );
    void ReservePinMemory( const int& n );
    
    inline void AddModule( const string& name, float width, float height, const bool& isFixed, const bool& isNI); // (frank) 2022-05-13 consider terminal_NI
    inline void AddModule( const string& name, float width, float height, const bool& isFixed );
    inline void AddModuleNetId( const int& moduleId, const int& netId );
    inline void SetModuleFixed( const int& moduleId );			    // 2005-10-20
    void SetModuleOrientation( const int& moduleId, const int& orient );    // 2005-10-20
    //void SetModuleType( const int& moduleId, const int& type );		    // 2005-10-20
    //void SetModuleIsPin( const int& moduleId, const bool& isPin );          // 2006-05-22

    inline void SetDieArea( double left, double bottom, double right, double top );
    void SetCoreRegion();    // Set core region according to the m_sites.
    inline void SetCoreRegion( double left, double bottom, double right, double top );

    inline int GetModuleId( const string& name );	// Include blocks and pads
    //Added by Jin 20090106
    inline int GetNetId( const string& name );
    int GetMovableBlockNumber();

    void AddNet( Net n );
    void AddNet( Net n, const char* name );
    void AddNet( Net n, const char* name, bool isPin );
    void AddNet( set<int> n );
    int AddPin( const int& moduleId, const float& xOff, const float& yOff, 
	    const int dir = PIN_DIRECTION_UNDEFINED );
    int AddPin( const int& moduleId, const string& pinName, const float& xOff, const float& yOff, 
	    const int dir = PIN_DIRECTION_UNDEFINED );	// 2005-08-29

    // Call CreateNameMap() after reading all modules and terminals and before reading nets.
    int    CreateModuleNameMap();
    void   ClearModuleNameMap();  // Clear memory
    void   PrintModules();

    void   PrintNets();
    //void CalcChipWH();

    double CalcHPWL();       // Calc center-to-center, pin-to-pin HPWL
    double GetHPWL( double* xwire, double* ywire );       // Calc pin-to-pin HPWL
    double GetHPWL()	{ return m_HPWL;   }
    double GetHPWLp2p()	{ return m_HPWLp2p;   }
    double GetHPWLdensity( double util );

    // kaie 2009-11-25
    void CalcModuleLeftBottomLocation( const int& id );
    void SetModuleOrientationCenter( const int& moduleId, const int& orient );
    void SetModuleOrientationBest(bool macroonly);
    double CalcHPWLModule( const int& id );
    // @kaie 2009-11-25
    
    // 2006-05-23 (donnie) X-Arch
    double CalcXHPWL();
    //double GetXNetLength( int netId );
    double GetXNetLength( vector<int>& pinsId );
    double GetXHPWL() { return m_XHPWL; }

    // 2006-05-26 (indark) LpNorm
    double CalcLpNorm();
    double GetNetLpNorm( int netId );
    double GetLpNorm() { return m_LpNorm; }

    double GetWidth()   { return (m_coreRgn.right-m_coreRgn.left); }
    double GetHeight()  { return (m_coreRgn.top-m_coreRgn.bottom); }

    double GetNetLength( int netId );   // Get the HPWL of a single net.
    double GetNetLength( vector<int>& pinsId );   // Get the HPWL of the given pinsId.
    double GetNetLength( vector<int>& pinsId, double* xwire, double* ywire ); 

    int    GetModuleNumber() { return (int)m_modules.size(); }
    int	   GetNetNumber()    { return (int)m_nets.size(); }
    int    GetModuleOrient(int moduleID) { return (int)m_modules[moduleID].m_orient;};
    
    void OutputMatlabFigure( const char* filename );
    // void OutputGnuplotFigure( const char* filename, bool withCellMove, bool showMsg)
    // 	{ OutputGnuplotFigureWithZoom(filename,withCellMove,showMsg,false); }
    // void OutputGnuplotFigure( const char* filename, bool withCellMove) 
    // 	{ OutputGnuplotFigureWithZoom(filename,withCellMove,false,false); }
    // // void OutputPngFigure( const char* filename, bool withCellMove)
    // //     { OutputPngFigure(filename, withCellMove, true); }
    // void OutputGnuplotFigureWithZoom( const char* filename, bool withCellMove, bool showMsg, bool withZoom, bool withOrient =true , bool withPin = false );
    // //void OutputPngFigure( const char* filename, bool withCellMove, bool showMsg, bool withOrient = true , bool withPin = true );
    // void OutputGnuplotFigureWithMacroPin( const char* filename, bool withCellMove )
    // 	{ OutputGnuplotFigureWithZoom(filename,withCellMove,false,false,true,true); }
    // void OutputAstroDump( const char* filename );

    // // (kaie) 2009-07-05 3D ICs
    // void OutputGnuplotFigure3D( const char* filename, bool withCellMove, bool showMsg, bool outputlayer = false)
    // 	{
	//     OutputGnuplotFigureWithZoom3D(filename, withCellMove, showMsg, false);
	//     if(outputlayer)
	// 	for(int layer = 0; layer < m_totalLayer; layer++)
	// 	{
	// 	    char filename_layer[1000];
	// 	    sprintf(filename_layer, "%s.3d-%d.plt", filename, layer);
	// 	    OutputGnuplotFigureWithZoom3DByLayer(filename_layer, withCellMove, showMsg, false, layer); 
	//     	}
	// }
    // void OutputGnuplotFigureWithZoom3D( const char* filename, bool withCellMove, bool showMsg, bool withZoom, bool withOrient = true, bool withPin = false, bool bNets = false );
    // void OutputGnuplotFigureWithZoom3DByLayer( const char* filename, bool withCellMove, bool showMsg, bool withZoom, int layer, bool withOrient = true, bool withPin = false, bool nNets = false);

    double GetFixBlockArea( const double& left, const double& bottom, const double& right, const double& top );

    inline vector<int> GetModulePins( const int id );
    inline vector<int> GetModuleNets( const int& id );
    
    void CalcModuleCenter( const int& id );
    void GetModuleCenter( const int& id, double& x, double& y );
    void SetModuleLocation( const int& id, float x, float y );
    void SetModuleLocation( const int& id, float x, float y, float z ) // gnr
    {
        assert(id<(int)m_modules.size());
        m_modules[id].m_x = x;
        m_modules[id].m_y = y;
        m_modules[id].m_z = z;
        m_modules[id].CalcCenter();
    }
    bool MoveModuleCenter( const int& id, float cx, float cy );
    bool MoveModuleCenter( const int& id, float cx, float cy, float cz ); // (kaie) 3d

    // Pin ================================
    void UpdatePinPosition();	// 2006-10-01 update pin positions according to current module positions
    int GetPinNumber() const
    {
	return (int)m_pins.size();
    }
    void CalcPinLocation( const int& pid )
    {
	m_pins[pid].absX = m_modules[m_pins[pid].moduleId].m_cx + m_pins[pid].xOff;
	m_pins[pid].absY = m_modules[m_pins[pid].moduleId].m_cy + m_pins[pid].yOff;
    }
    void CalcPinLocation( const int& pid, float cx, float cy ) 
    {
	m_pins[pid].absX = cx + m_pins[pid].xOff;
	m_pins[pid].absY = cy + m_pins[pid].yOff;		
    }
    void GetPinLocation( const int& pid, double &x, double &y ) const
    {
	x = m_pins[pid].absX;
	y = m_pins[pid].absY;
    }
    // (kaie) 2009-09-16
    void GetPinLocation( const int& pid, double &x, double &y, double &z) const
    {
	x = m_pins[pid].absX;
	y = m_pins[pid].absY;
	z = m_modules[m_pins[pid].moduleId].m_z;
    }
    // @(kaie) 2009-09-16
    string GetPinName( int pid )
    {
	return m_pins[pid].pinName;
    }
    // ====================================

    /*void FixModuleOrientation()	// all set to !rotate	
    {
	//for( int i=0; i<(int)m_modules.size(); i++ )
	//	m_modules[i].rotate = false;
    }*/

    inline void SaveBlockLocation();
    inline void RestoreBlockLocation();
    inline void SaveBestBlockLocation();
    inline void RestoreBestBlockLocation();

    void MoveBlockToLeft( double factor );
    void MoveBlockToCenter( double factor );
    void MoveBlockToBottom( double factor );

    int m_nModules;			
    int m_nPins, m_nNets;

    void OutputGSRC( const char* filename );
    //void OutputPL( const char* filename, bool setOutOrientN=false, bool bMsg=true );

    // 2006-03-06
    void SetAllBlockMovable();
    // // (kaie) 2009-06-25
    // void CalcModuleLeftBottomLocation( const int& id );
    // void SetModuleOrientationCenter( const int& moduleId, const int& orient );
    // vector<int> FixedBlockSet;
    // void RestoreFixedBlocks();
    // void SetMacroFixed( const double ratio);
    // class SortCompareModules
    // {
	// public:
	//     static std::vector<Module>* m_pMod;
	//     static bool asending;
	//     bool operator() ( const int& m1, const int& m2 )
	//     {
	// 	if(asending)
	//     	    return (*m_pMod)[m1].m_area < (*m_pMod)[m2].m_area;
	// 	else
	// 	    return (*m_pMod)[m1].m_area > (*m_pMod)[m2].m_area;
	//     }
    // };
    // void SetModuleOrientationBest(bool macroonly, bool rotate, bool flip);
    // void SetModuleNoFlip();
    // double CalcHPWLModule( const int& id );
    // // @(kaie) 2009-06-25
    
    // 2005-12-02 (donnie)
public:
    int  CalculateFixedModuleNumber();
    int  GetUsedPinNum();
    bool ModuleInCore( const int& i );
    void OutputBookshelf( const char* prefix, bool setOutOrientN=false );
    void OutputNodes( const char* filename, bool setOutOrientN=false );
    void OutputSCL( const char* filename );
private:
    void OutputAUX( const char* filename, const char* prefix );

    void OutputNets( const char* filename, bool setOutOrientN=false );
    //void OutputSCL( const char* filename );
    void OutputWTS( const char* filename );
    
public:

    //double m_rowHeight;			// height of the rows
    float m_rowHeight;			// height of the rows

//    private: 
//	by indark
public:
    map<string, int> m_moduleMapNameId;

    //Added by Jin 20090105
    map<string, int> m_netMapNameId;
    int    CreateNetNameMap();
    
    vector<Module> m_modules;
    vector<Module> m_modules_bak;
    vector<Module> m_modules_bak_best;
    vector<Net>    m_nets;
    //vector<double> m_netsWeight;    // 2006-01-10 (donnie)
    vector<float> m_netsWeight;    // 2006-01-10 (donnie)
    vector<string> m_netsName;	    // 2006-4-1 (donnie)
    vector<bool> m_netsIsPin;		// kaie 2009-11-18
    vector<Pin> m_pins;
    vector<Pin> m_pins_bak;
    vector<Pin> m_pins_bak_best;
    double m_HPWL;				// Half-Perimeter Wirelength (center2center)
    double m_HPWLp2p;				// Half-Perimeter Wirelength (pin2pin)
    double m_XHPWL;				// 2006-05-23 (donnie) X-arch HPWL 
    double m_LpNorm;				// 2006-05-26 (indark) LpNorm WL
    double m_totalModuleArea;			// Sum of module area
    double m_totalFreeSpace;			// 2006-03-02 (donnie)
    double m_totalMovableModuleArea;		// 2006-03-02 (donnie)
    int    m_totalMovableModuleNumber;		// 2006-03-02 (donnie)
    double m_totalMovableLargeMacroArea;	// 2006-03-13 (donnie)
    CRect  m_coreRgn;				// Core region
    CRect  m_coreRgn2;				// Core region (backup)
    CRect  m_coreRgnShrink;
    CRect  m_dieArea;

    // 2006-09-18
    vector<int> m_pinNetId;
    vector< vector<int> > m_moduleNetPinId;
    void UpdateModuleNetPinId();
    void UpdatePinNetId();

    //Added by Jin
public:
    vector<CSiteRow> m_sites;
    //@Added by Jin

    //Added by kaie 3D ICs
    int m_layer;
    int m_totalLayer;
    double m_front;
    double m_back;
    double m_totalModuleVolumn;
    double m_totalMovableModuleVolumn;
    vector< vector<CSiteRow> > m_sites3d;
    vector< vector<int> >m_modules3d;
    void Folding2();
    void Folding4();
    int TSVlefid;
    float TSVwidth;
    float TSVheight;
    float TSVarea;
    double CalcTSV();
    int totalTSVcount;
    int GetTSVcount()	{ return totalTSVcount; }
    //@Added by kaie

    void AdjustCoordinate();
    void CheckRowHeight(double row_height);
    int Align();  // updated by crazying
    
    vector<CSiteRow>::iterator GetRow( double y );
    void RestoreCoreRgn(void);
    void ShowRows();
    
    //private:	//commented by indark
    void GetRegionWeightedCenter(double left, double bottom, 
	                         double right, double top, 
				 double& x, double& y);
    void GetCorePinCenter(double& x, double& y);


    int part_options [9];

public:

    //Added by Jin
    CPoint GetRegionWeightedCenter( const double &left, const double &right, 
	                            const double &bottom, const double &top );
    CPoint GetCoreCenter( void );
    void RemoveFixedBlockSite();
    //@Added by Jin

    // donnie
    //bool CheckStdCellOrient();
    void ShowDBInfo();
    void ShowDensityInfo(); // 2006-03-02 (donnie)

    int CreateDummyFixedBlock();
    int realModuleNumber;
    void RemoveDummyFixedBlock();

    //Added by Jin 20060228
    double GetModuleTotalNetLength( const int& mid );
    //Remove the sites under macros (without fixing any module)
    void RemoveMacroSite();	
    //@Added by Jin 20060228
    
    //Added by Jin 20060331 
    vector<CSiteRow> m_sites_for_legal;
    //@Added by Jin 20060331

    double m_util;	// by donnie. calcualted in ShowDBInfo();

    // 2006-06-19 (donnie)
    void ConnectLiberty( Liberty* pLibDB );
    CParserLEFDEF* m_pLEF;
    Liberty* m_pLib;
    

    //Added by Jin 20060414
private:
    //Make m_sites align to site step
    void FixFreeSiteBySiteStep(void);
    //@Added by Jin 20060414


public:
    double GetNetLoad( const int& netId );	// 2006-06-22 (donnie)
    //void IdentifyDFF();				// 2006-06-22 (donnie)
    void InitializeBlockInOut();		// 2006-06-22 (donnie)
    int  GetNetIdByBlockAndPin( const int& blockId, const int& pinId );
    //void CheckLibertyNetwork();			// 2006-06-22 (donnie)
    set<int> m_groundNets;

    //Added by Jin 20060629
public:
    CCongMap* m_pCongMap;
    //void CreateCongMap( const int& hdim, const int& vdim );
    void ClearCongMap( void );
    //@Added by Jin 20060629
    
#if 0 
    //Added by Jin 20060718
    void OutputCongMapFigure(void)
    {
	OutputGnuplotFigureWithZoom( "CongMap.plt", false, true, false, false, false, true );
    }
    //@Added by Jin 20060718
#endif
    
    // 2006-09-18 (donnie)
    //void   ShowSteinerWL();
    //void   ComputeSteinerWL();
    //double GetSteinerWL( const Net& net, int steinerType=0, int routingType=0 );
    //double GetTotalSteinerWL( const int& steinerType, const int& routingType );

public:   
    // 2007-01-31 (donnie)
    double m_gCellWidth;
    double m_gCellHeight; 
    double m_gCellLeft;
    double m_gCellBottom;
    double m_gCellNumberHorizontal;
    double m_gCellNumberVertical;
    
    //Added by Jin 20070306
public:
    CFixedPointMethod* m_pFixedPointMethod;
    CSteinerDecomposition* m_pSteinerDecomposition;
    //@Added by Jin 20070306

public:
    // donnie  2007-07-17
    bool BlockOutCore( int );

    //Added by Jin 20081013
    TimingAnalysis* m_pTimingAnalysis;

    //Added by kaie 20091112
    void LayerAssignmentByPartition(const int layer);

    //Added by kaie 20100301
    void ResizeCoreRegion3d(const double wsratio);

    //Added by kaie 20100302
    void AdjustStandardCellOrientation();
    void AdjustNetConnection();

};



///////////////////////////////////////////////////////////////////////


void CPlaceDB::AddModule( const string& name, float width, float height, const bool& isFixed )
{
    assert( width >= 0 );
    assert( height >= 0 );
    m_modules.push_back( Module( name, width, height, isFixed ) );
}

void CPlaceDB::AddModule( const string& name, float width, float height, const bool& isFixed, const bool& isNI)
{
    assert( width >= 0 );
    assert( height >= 0 );
    m_modules.push_back( Module( name, width, height, isFixed, isNI ) );
}

void CPlaceDB::AddModuleNetId( const int& moduleId, const int& netId )
{
    m_modules[moduleId].m_netsId.push_back( netId );
}

void CPlaceDB::SetModuleFixed( const int& moduleId )	// 2005-10-20
{
    m_modules[moduleId].m_isFixed = true;
}

void CPlaceDB::SetDieArea( double left, double bottom, double right, double top )
{
    m_dieArea.left   = left;
    m_dieArea.bottom = bottom;
    m_dieArea.right  = right;
    m_dieArea.top    = top;
}

void CPlaceDB::SetCoreRegion( double left, double bottom, double right, double top )
{
    m_coreRgn.bottom = bottom;
    m_coreRgn.left   = left;
    m_coreRgn.right  = right;
    m_coreRgn.top    = top;
}

int CPlaceDB::GetModuleId( const string& name )	// Include blocks and pads
{
    map<string, int>::const_iterator ite = m_moduleMapNameId.find( name );
    if( ite == m_moduleMapNameId.end() )
	return -1;
    return ite->second;
}

//Added by Jin 20090105
int CPlaceDB::GetNetId( const string& name )	// Include blocks and pads
{
    map<string, int>::const_iterator ite = m_netMapNameId.find( name );
    if( ite == m_netMapNameId.end() )
	return -1;
    return ite->second;
}

vector<int> CPlaceDB::GetModulePins( int id )
{
    return m_modules[id].m_pinsId;
}

void CPlaceDB::SaveBlockLocation()
{
    m_modules_bak = m_modules;  // save block positions
    m_pins_bak = m_pins;        // save pin positions
}

void CPlaceDB::RestoreBlockLocation()
{
    m_modules = m_modules_bak;  // restore block positions
    m_pins = m_pins_bak;        // restore pin positions
}

void CPlaceDB::SaveBestBlockLocation()
{
    m_modules_bak_best = m_modules;  // save block positions
    m_pins_bak_best = m_pins;        // save pin positions
}

void CPlaceDB::RestoreBestBlockLocation()
{
    m_modules = m_modules_bak_best;  // restore block positions
    m_pins = m_pins_bak_best;        // restore pin positions
}

vector<int> CPlaceDB::GetModuleNets( const int& i )
{
    return m_modules[i].m_netsId;
}

/*FM Partition*/ //(kaie) 2009-11-11

struct FMNet
{
public:
	int id;
	string name;
	vector<int> cellId;	// connected cell list
	
	int block_cells[2];	// count for connected cells in two blocks
	int block_cells_bak[2];	// backup
};

enum Block{BLK_A, BLK_B};

struct FMCell
{
public:
	int id;
	int placeId;
	vector<int> netId;	// connected net list

	bool isFree;	// locked or unlocked
	int gain;	// cell gain
	bool block;	// associated block
	double area;

	list<int>::iterator gain_ptr;	// pointer to gain bucket
};

struct FMSolution
{
        FMSolution(){};
        FMSolution(const int _gain, const int _cell, const double A_size, const double B_size)
                :gain(_gain), move_cell(_cell), part_A_size(A_size), part_B_size(B_size){};
        ~FMSolution(){};

        int gain;       // solution gain
        int move_cell;  // moved cell
        double part_A_size;
        double part_B_size;
};

class FM_Partition
{
public:
	// Parsing
	inline void Parsing(char*);
	FM_Partition(CPlaceDB& placedb, vector<int> pmodules);
	~FM_Partition(void){}

	// Debugging
	void PrintNets();
	void PrintCells();
	void PrintGainBucket();
	void PrintPartitions();

	// Partition
	void init();
	void InitCellGains();
	void UpdateCellGains(int);
	int Par();
	int Partition(); // main partition function

	// Output
	void OutputResult(char*);

	// Data
	double imbalance_ratio;
	vector<FMNet> nets;
        int num_nets;
        vector<FMCell> cells;
        int num_cells;
        int free_cells;
        int num_pins;
        int pass;

	// Cut-Size
	int cut_size;
	vector<int> cut;
	int GetCutSize();

	// Gain Bucket
	int p_max, p_min;
	void UpdateGainBucket(int, int, int);
	vector<list<int> > gain_bucket;
	int max_gain_ptr;       // pointer to max-gain

	// Partitions
	set<int> partition_A, partition_B;
	//int partition_A_size, partition_B_size;
	double partition_A_size, partition_B_size;
	//unsigned int low_bound, up_bound;
	double low_bound, up_bound;
	int num_solutions;
	vector<FMSolution> solutions;

	// Run Time
	clock_t run_time;
};

#endif
