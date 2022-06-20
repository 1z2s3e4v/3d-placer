#include <fstream>
#include <cstdio>
#include <string.h>
#include <cstring>
#include <string>
#include <climits>
#include <cassert>
#include <iostream>
#include <cmath>
#include <map>
#include <vector>
#include <list>
#include <sstream>

#include <sys/types.h>
#include <sys/stat.h>


#ifndef Liberty_H
#define Liberty_H

/* Liberty Parser PIs */
#include "si2dr_liberty.h"


#define TABLE_LOOKUP_MODEL          "table_lookup"

using namespace std;

/* Lookup Table Templates Info */
struct lutInfo {
       char    *name;                    /* Template Name */
       char    *var1;
       char    *var2;
       int     i1;       /* Size of Index1 */
       vector<double> index1;
       int     i2;                       /* Size of Index2 */
       vector<double> index2;
};


/* Pin Delay Information */
struct pinDelayInfo {
       string		name;                    /* Template Name */
       struct lutInfo	*delayTable;
       vector<double>	values;
       vector<double>	index1;             
       vector<double>	index2;			
       int		delayArcComponent;       /* 1 : cell_rise,  2 : cell_fall,  3 : rise_transition,  4 : fall_transition */
       //string		relatedPin;
};


/* Pin Information */
struct pinInfo {
       string  name;
       int     direction;  /* 1:input,  0:output,  -1:NULL */
       //double  maxCap;
       //double  minCap;
       double  cap;
       map < string, vector<pinDelayInfo*>* > delayInfo;  //< relatedPin, vector<pinDelayInfo*> >
       map < string, unsigned int > timingSense; //< relatedPin, timing_sense >
};

struct cellInfo {
       string  name;
       string  lvt_name;
       string  hvt_name;
       bool    DFF;
       double  svt_leakage;
       double  lvt_leakage;
       double  hvt_leakage;
       map <string, struct pinInfo*>* svt_pinsMap;
       map <string, struct pinInfo*>* hvt_pinsMap;
       map <string, struct pinInfo*>* lvt_pinsMap;
};


class Liberty{
       public:
//	      int 	parseLib(char*, FILE*);	/* input .lib file and msg output file */
//	      int       parseHVTLib(char*, FILE*); /* input HVT .lib file and msg output file */
//            int       parseLVTLib(char*, FILE*); /* input LVT .lib file and msg output file */
	      int 	parseLib(char*);    /* input .lib file */
	      int       parseHVTLib(char*); /* input HVT .lib */
              int       parseLVTLib(char*); /* input LVT .lib */
//	      int 	check(FILE*);
	      int 	check();
	      cellInfo* getCellType(string);
              double	calculateTiming (const cellInfo*, const string&, const double, const double, const unsigned int, const unsigned int);
              double	calculateTimingwithGradient (const cellInfo*, const string&, const double, const double, const unsigned int, const unsigned int, double&);
	      double    calculateTimingDFF (const cellInfo*, const string&, const double, const double, const unsigned int, const unsigned int);
	      //Added by Jin 20081015
	      double    calculateTimingDFFwithGradient (const cellInfo*, const string&, const double, const double, const unsigned int, const unsigned int, double&);
	      double    calculateTimingHoldSetup (const cellInfo*, /*const string,*/ const double, const double, const unsigned int, const unsigned int);
	      
/*              bool      getCell(const string);  // true:: find the cell  false: not find the cell
	      string    getCellHVTName(const string);
	      string    getCellLVTName(const string);
	      string    getCellSVTName(const string);
*/	      //map <string, cellInfo*>::iterator getCellInfo (string);
              //map <string, pinInfo*>::iterator	getPinInfo (map <string, cellInfo*>::iterator, string);
              
       
       //private:
              char 			*getName(si2drGroupIdT);
//              void 			setLutInfo(si2drGroupIdT, FILE*);
//              void 			setCellInfo (si2drGroupIdT, FILE*);
//	      void                      setHVTCellInfo (si2drGroupIdT, FILE*);
//	      void                      setLVTCellInfo (si2drGroupIdT, FILE*);
              void 			setLutInfo(si2drGroupIdT);
              void 			setCellInfo (si2drGroupIdT);
	      void                      setHVTCellInfo (si2drGroupIdT);
	      void                      setLVTCellInfo (si2drGroupIdT);
              map <string, pinInfo*>	*setPinInfo (si2drGroupsIdT, bool);
              lutInfo			*getRelatedLutInfo (string, map <string, pinInfo*>::const_iterator, int);  
                                        /* 3rd argu  1 : cell_rise,  2 : cell_fall,  3 : rise_transition,  4 : fall_transition */
              //double			*getRelatedDelayValue (string, map <string, pinInfo*>::const_iterator, int);  
	      //                        /* 3rd argu  1 : cell_rise,  2 : cell_fall,  3 : rise_transition,  4 : fall_transition */
              struct lutInfo 	*getLutInfo (char *);
              //int		getLutVar(char*);
//              int 		checkLutInfo(FILE*);
//             int 		checkCellInfo(FILE*);
//	      int               checkHVTCellInfo(FILE*);
//	      int               checkLVTCellInfo(FILE*);
              int 		checkLutInfo();
              int 		checkCellInfo();
	      int               checkHVTCellInfo();
	      int               checkLVTCellInfo();
              //int		checkCalculateTiming();
              //map <string, pinInfo*>::iterator getOutputPin (map <string, cellInfo*>::const_iterator);             
              map<string, cellInfo*>::iterator getCellLocation (string, string);

	      
              vector<lutInfo*>		lutInfoV;
	      int 			cellCount;
              map <string, cellInfo*>	cellsMap;

              //const char *delayArcComponents[10];  /* Components of Delay Arc */
};

#endif
