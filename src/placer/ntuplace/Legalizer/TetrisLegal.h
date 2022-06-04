#ifndef _CTETRISLEGAL_
#define _CTETRISLEGAL_

#include "../PlaceDB/placedb.h"
#include "../PlaceCommon/util.h"


namespace Jin
{
    class CLegalLocation
    {
	public:
	    CLegalLocation( const int& set_site_index, const double& set_xcoor ) :
		m_site_index( set_site_index ), 
		m_xcoor( set_xcoor ),
		m_shift(0), m_wirelength(0) {}
	    int m_site_index; //Site index for the legal location (in y direction)
	    double m_xcoor;	  //x coordinate

	    //Used for sort
	    double m_shift;	  //Record the shift from the original location of a cell
	    double m_wirelength;  //Record the wirelength when cell is placed to the location
    };

    //Compare m_shift of class 'CLegalLocation'
    class LessShift
    {
	public:
	    bool operator() (const CLegalLocation& l1, const CLegalLocation& l2)
	    {
		return l1.m_shift < l2.m_shift;
	    }
    };

    //Compare m_wirelength of class 'CLegalLocation'
    class LessWirelength
    {
	public:
	    bool operator() (const CLegalLocation& l1, const CLegalLocation& l2)
	    {
		return l1.m_wirelength < l2.m_wirelength;
	    }
    };

    //Used to keep the terminal information for each free site interval
    class CTerminal
    {
	public:
	    enum Type
	    {
		Left,
		Right
	    };

	    CTerminal( const double& set_xcoor, const Type& set_type, const int& set_row ) :
		m_xcoor( set_xcoor ), m_type( set_type ), m_row( set_row ) {}

	    double m_xcoor; //X coordinate of this terminal
	    Type m_type;	//Terminal direction (left or right)
	    int m_row;		//Index of row which contains this terminal
    };

    class LessXCoor
    {
	public:
	    static CPlaceDB* m_placedb;

	    //Used to raise the priority of macros
	    static double m_macro_factor;

	    //Compare two cells (first key is x coordinate and second key is width, third is height)
	    bool operator()( const int& mid1, const int& mid2 );

	    bool operator()( const CTerminal& t1, const CTerminal& t2 )
	    {
		return t1.m_xcoor < t2.m_xcoor;
	    }

    };

    class LessXCoorMacroFirst
    {
	public:
	    static CPlaceDB* m_placedb;

	    //Compare two cells (first key is x coordinate and second key is width, third is height)
	    bool operator()( const int& mid1, const int& mid2 );

	    bool operator()( const CTerminal& t1, const CTerminal& t2 )
	    {
		return t1.m_xcoor < t2.m_xcoor;
	    }

    };

    //Macros have higher priority than cells with the same x coordinate
    class LessXCoorMacroPrior
    {
	public:
	    static CPlaceDB* m_placedb;
	    bool operator()( const int& mid1, const int& mid2 );

	    static bool BL( const int& mid1, const int& mid2 );
	    static bool BR( const int& mid1, const int& mid2 );
	    static bool TL( const int& mid1, const int& mid2 );
	    static bool TR( const int& mid1, const int& mid2 );
    };

    inline double Rounding( const double& d1 )
    {
	return std::floor(d1+0.5);
    }

}

class CTetrisLegal
{
    public:
	CTetrisLegal( CPlaceDB& placedb ); 
	~CTetrisLegal(void){}

	//Solve the Tetris legalization, "true" indicates success
	bool Solve(const double& util, 
		const bool& bMacroLegal = false,	//Integration with macro legalizer
		const bool& bRobust = false,        //Enable or disable the robust legalizer
		const double& stop_prelegal_factor = 0.85 //Legalizer will stop at this factor
		);
	//prelegal factor legalizer
	
	// (donnie) 2007-07-10 Return more infomation
	bool SolveAndReturn(const double& util, 
		const bool& bMacroLegal = false,	//Integration with macro legalizer
		const bool& bRobust = false,        //Enable or disable the robust legalizer
		const double& stop_prelegal_factor = 0.85, //Legalizer will stop at this factor
		int tetrisDir = 0,
		int *pResultDir = NULL,
		double *pResultDiff = NULL
		);

	
    private:
	//Calculate new cell locations to obtain legalization order
	void CalculateNewLocation(const double& prelegal_factor);
	//Calculate legalization order by new cell locations
	void CalculateCellOrder(void);
	//Legalize all cells by cell order, "true" indicates success
	bool LegalizeByCellOrder(void);
	//Get all candidate legalization locations for a given cellid and return the best location
	void GetLegalLocations( const int& cellid, std::vector<Jin::CLegalLocation>& legalLocations );
	//Place a cell to legal location, "ture" indicates success
	bool PlaceCellToLegalLocation( const int& cellid );
	//For a given y coordinate (ycoor), return the site index it locates in
	//Each site covers [bottom, top) range
	int GetSiteIndex( const double& ycoor );
    public:
	CPlaceDB& m_placedb;
    private:
	double m_width_factor;		//Used to calculate new locations of cells
	//Typical value is between 1.0 and 2.0
	double m_left_factor;		//X search by is calculated by the original cell location
	//minus (m_left_factor times average_cell_width)
	//Typical value is between 1.0 and 2.0
	double m_average_cell_width;//Average standard cell width
	int m_row_limit;			//Search for the m_row_limit upward and downward rows for
	//legal positions. -1 means the row_limit is determined by
	//m_row_factor * cell height
	std::vector<int> m_cell_order; //Used as legalization order
	std::vector<CSiteRow> m_free_sites; //Record the unused sites
	double m_site_bottom, m_site_height; //Keep the bottom y coordinate of the sites and the 
	//height of each site

	//Remove ocupied free site in m_free_sites[rowId] by the given xstart and width
	void UpdateFreeSite( const int& rowId, const double& xstart, const double& width );
    private:
	int m_unlegal_count;	//Number of unlegalized cells
	double m_prelegal_factor;  //Prelegal factor
    public:
	int GetUnlegalCount(void)
	{
	    return m_unlegal_count;
	}
    private:
	bool DoLeft(const double& prelegal_factor); //Legalize to left
	vector<CPoint> m_origLocations;	//Record the original locations
	vector<CSiteRow> m_origSites;	//Record the original m_free_sites
	vector<CPoint> m_bestLocations;	//Record the best locations
	double m_best_prelegal_factor;	//Record the best prelegal factor
	double m_best_cost;		//Record the best cost
	vector<CSiteRow> m_best_sites;   //Record the resulting sites of the best result
	void SaveOrig(void); //Save original solution
	void SaveBest(const double& best_prelegal_factor, const double& best_cost); //Save best solution
	void RestoreOrig(void); //Restore orignal solution
	void RestoreBest(void); //Restore best solution
	void ReversePlacement(void);	//Reverse coordinates of all modules
	void ReverseLegalizationData(void);	//Reverse m_coreRgn of m_placedb, m_free_sites, and pin offsets
	//void SetReverseSite(void);	//Set reversed m_free_sites
	bool DoRight(const double& prelegal_factor); //Legalize to right
	double m_chip_left_bound;	//Record the chip left boundary 
	//(for CalculateCellNewLocation())
	//best results for left (1) and right (2)
	double m_best_prelegal_factor_left;
	double m_best_cost_left;	
	vector<CPoint> m_bestLocations_left;
	vector<CSiteRow> m_best_sites_left;
	double m_row_factor;		//Search row limit = m_row_factor*(current cell's height)

	bool DoLeftMacroFirst(const double& prelegal_factor );	//Legalize to left and macros 
	//have higher priority
	void CalculateCellOrderMacroFirst(void);	//Calculate the cell order for legalization and
	//macros have higher priority
	bool m_bMacroLegalized;	//Integration with Macro legalizer
	//Ture means the input placement is macro legalized,
	//and thus the row limit for each macro is set to 1

	//*******************************************************
	//* Functions for legalization to certain vertical line *
	//*******************************************************
    private:
	bool DoLeftWithVerticalLine( const double& prelegal_factor,
		const double& vertical_line_xcoor );
	void CalculateNewLocation( const double& prelegal_factor, 
		const double& vertical_line_xcoor );	
	void CalculateCellOrder( const double& vertical_line_xcoor );

	//Remove m_free_sites in the right of a given vertical_line
	void RemoveFreeSite( const double& vertical_line_xcoor );
	bool DoRightWithVerticalLine( const double& prelegal_factor,
		const double& vertical_line_xcoor );
	bool DoVerticalLine( const double& prelegal_factor,
		const double& vertical_line_xcoor );

	//********************************
	//* Functions for Diamond Search *
	//********************************
    private:
	//Get a diamond free sites
	void GetDiamondSiteRows( const CPoint& center,				 //center of diamond 
		const double& radius,				 //radius of diamond
		std::vector<CSiteRow>& sites );	 //put the resulting diamond sites into "sites"

	//**************************************************************************
	//* Get legal locations for a macro and a set of given free sites          *
	//* The difference between 'TowardOrig' and 'TowardLeft' is that           *
	//* 'TowardOrig' finds the closest location to original location           *
	//* for each interval (multiple locations may be return for one row) while *
	//* 'TowardLeft' only returns the leftest legal location for each row      *
	//**************************************************************************
	//Note that since the return row index of legalLocations is based on the 
	//input 'sites', and thus you have to update the index to original m_free_sites
	void GetMacroLegalLocationsTowardOrig( const int& cellid,	 //Target macro id
		const std::vector<CSiteRow>& sites,					 //Given free sites
		std::vector<Jin::CLegalLocation>& legalLocations     //Put the resulting locations 
		//into legalLocations
		); 
	void GetMacroLegalLocationsTowardLeft( const int& cellid,	 //Target macro id
		const std::vector<CSiteRow>& sites,					 //Given free sites
		std::vector<Jin::CLegalLocation>& legalLocations,    //Put the resulting locations 
		//into legalLocations
		const double& left_bound							 //Search left boundary	
		); 

	//Return the index of the best location of given locations for a given cell
	int ReturnBestLocationIndex( const int& cellid, 
		std::vector<Jin::CLegalLocation>& legalLocations );

	//Remove the m_free_sites under a given module
	void UpdateFreeSite( const int& cellid );

	double m_max_module_height;
	double m_max_module_width;

	//Aggressively diamond search the legal locations for a macro
	//If success, return 'true'
	bool AggressiveMacroDiamondSearch( const int& cellid,
		std::vector<Jin::CLegalLocation>& legalLocations );
    public:
	//MacroShifter all macros with height >= macroFactor*m_placedb.m_rowHeight
	//to a legal location without overlapping with other macro or preplaced modules
	//Parameter 'makeFixed' distinguishes to fixed all processed macros or not
	bool MacroShifter( const double& macroFactor, const bool& makeFixed );
	//To reapply MacroShifter, you have to execute RestoreFreeSite() first
	void RestoreFreeSite(void)
	{
	    m_free_sites.clear();
	    m_free_sites = m_placedb.m_sites;
	}
	//Robust Macro Shifter
	bool RobustMacroShifter( const double& macroFator, const bool& makeFixed );


    private:
	//Get the legal locations for a given cellid within a given range 
	//[start_site_index, end_site_index] of free sites
	void GetCellLegalLocationsTowardLeft( const int& cellid,
		const int start_site_index, 
		const int end_site_index,
		std::vector<Jin::CLegalLocation>& legalLocations,    //Put the resulting locations 
		//into legalLocations
		const double& left_bound );							 //Search left boundary	

	//Aggressively search cell legal locations
	//The search row limit will be expanded until the cell shift 
	//is smaller than the search row limit
	//If it is impossible to find a legal location, it will return false
	bool AggressiveCellLegalLocationsSearch( const int& cellid,
		std::vector<Jin::CLegalLocation>& legalLocations );

	//Returns the minimu shift between the original cell location and 
	//the locations in 'legalLocations'
	double ReturnMinimumShift( const int& cellid,
		const std::vector<Jin::CLegalLocation>& locations );
	double m_macro_factor;							//In LessXCoor, macro has higher priority by m_macro_factor

	//Variables and functions used for 4-directional macro shifter
    private:
	std::vector<int> m_macro_ids;
	std::vector<CPoint> m_macro_shifter_orig_positions;		//Record the original macro positions (left-bottom)
	//before macro shifter
	std::vector<CPoint> m_macro_shifter_best_positions;		//Record the best macro positions (left-bottom)
	//in 4-directional macro shifter
	double m_macro_sfhiter_min_shifting;					//Record the minimum shift of 
	//each direction of macro shifter
	void MacroShifterSaveOrigPosition(void);
	void MacroShifterRestoreOrigPosition(void);
	void MacroShifterSaveBestPosition(void);
	void MacroShifterRestoreBestPosition(void);

	//Refactoring for m_free_sites
    private:
	//Prepare for left and right free sites
	void PrepareLeftRightFreeSites(void);
	void PrepareNonMacroLeftRightFreeSites(const std::vector<int>& macro_ids);
	void SetLeftFreeSites(void){ m_free_sites = m_left_free_sites; }
	void SetRightFreeSites(void){ m_free_sites = m_right_free_sites; }
	std::vector<CSiteRow> m_left_free_sites;
	std::vector<CSiteRow> m_right_free_sites;

	//Refactoring for m_process_list
	//Put all movable module ids into m_process_list
	void SetProcessList(void);
	void SetNonMacroProcessList( const vector<int>& macro_ids );
	//CalculateCellOrder() and CalculateNewLocation() only act on modules
	//in the m_process_list
	std::vector<int> m_process_list;

	bool DoLeftRightUntil( const double& util, const double& stop_prelegal_factor );


	void SaveGlobalResult(void);
	void RestoreGlobalResult(void);

	vector<CPoint> m_globalLocations;
    public:
	//Legalize for a legalized solution (to fix alignment caused overlapping)
	bool RemoveOverlap(void);

    private:
	void SaveModuleWidth(void);
	void ExpandModuleWidthToSiteStep(void);
	void RestoreModuleWidth(void);
	std::vector<double> m_orig_widths;
    public:
	//Added by Jin 20070711
	bool SolveRegion( const CRect& region, 
		const std::vector<int>& m_module_ids, 
		const double& util = 1.0, 
		const double& stop_prelegal_factor = 0.85 );
	std::vector<CSiteRow> ExtractSites( const std::vector<CSiteRow>& source_sites,
		const CRect& region );
	void ApplySites( const std::vector<CSiteRow>& source_sites,
		const CRect& region,
		std::vector<CSiteRow>& target_sites );
    private: // donnie 2007-07-10
	int m_tetrisDir;   // 0 both    1 left  2 right
	int m_resultTetrisDir;
	double m_resultTetrisDiff;
    private:
	//Added by Jin 20070716
	void ReverseFreeSites(void);
};


// 2007-07-17 (donnie)
class CTopDownLegal
{
    public:

	CTopDownLegal( CPlaceDB* );
	~CTopDownLegal();
	bool Legal( double util, double stop_prelegal_factor );
	bool Legal( CRect&, vector<int>& blocks );
	
	double GetRegionFreeSpace( CRect& rect );
	void   GetBlockList( CRect& rect, vector<int> *pList );
	void   GetBlockList( CRect& rect, vector<int> *pList, vector<int>& blocks );
	double GetBlockArea( vector<int>& list );
	double GetSiteSpace( const vector<CSiteRow>& source_sites, const CRect& region );
	
    private:

	CPlaceDB*     m_pDB;
	CTetrisLegal* m_pLegal;
	vector<bool>  m_blockLegalized;
   
        double m_util;
	double m_stop_prelegal_factor;	
};

#endif 

