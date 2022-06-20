#include "liberty.h"
#include "sta.h"
#include <cstdlib>

// Curve Fit data : Ax + By + Cxy + D -OR- A + Bx 
double cFit2 (double load1,   double load2,   double slope1,  double slope2, 
              double delay11, double delay12, double delay21, double delay22,
              double load,    double slope) {

     int    i, j, k;
     double mult, pivot;
     double A, B, C, D;
     double M [] = { load1, slope1, load1*slope1, 1, 1, 0, 0, 0,
                     load1, slope2, load1*slope2, 1, 0, 1, 0, 0,
                     load2, slope1, load2*slope1, 1, 0, 0, 1, 0,
                     load2, slope2, load2*slope2, 1, 0, 0, 0, 1 };
     double V [] = { delay11, delay12, delay21, delay22 };

     for (i = 0; i < 4; ++i) {
         pivot = M [8*i + i];
         for (j = 0; j < 8; ++j) {
             M [8*i + j] /= pivot;
         }
         for (k = 0; k < 4; ++k) {
             if (i == k) continue;
             mult = -1 * M[8*k + i];
             for (j = 0; j < 8; ++j) {
                 M [8*k + j] = M[8*k + j] + mult*M[8*i +j];
             }
         }
     }
 
     A = B = C = D = 0;
     for (j = 0; j < 4; ++j)
         A += V[j] * M[4 + j];
     for (j = 0; j < 4; ++j)
         B += V[j] * M[12 + j];
     for (j = 0; j < 4; ++j)
         C += V[j] * M[20 + j];
     for (j = 0; j < 4; ++j)
         D += V[j] * M[28 + j];

     //modified by Jin
     //return (A*load + B*slope + C*load*slope + D);
     double result = (A*load + B*slope + C*load*slope + D);
     if( result < 0.0 )
     {
	result = 0.000001; 
     }
     return result;
   
}

// Added by Jin 20081014
// Curve Fit data : Ax + By + Cxy + D -OR- A + Bx 
double cFit2withGradient (double load1,   double load2,   double slope1,  double slope2, 
              double delay11, double delay12, double delay21, double delay22,
              double load,    double slope, double& gradient) {

     int    i, j, k;
     double mult, pivot;
     double A, B, C, D;
     double M [] = { load1, slope1, load1*slope1, 1, 1, 0, 0, 0,
                     load1, slope2, load1*slope2, 1, 0, 1, 0, 0,
                     load2, slope1, load2*slope1, 1, 0, 0, 1, 0,
                     load2, slope2, load2*slope2, 1, 0, 0, 0, 1 };
     double V [] = { delay11, delay12, delay21, delay22 };

     for (i = 0; i < 4; ++i) {
         pivot = M [8*i + i];
         for (j = 0; j < 8; ++j) {
             M [8*i + j] /= pivot;
         }
         for (k = 0; k < 4; ++k) {
             if (i == k) continue;
             mult = -1 * M[8*k + i];
             for (j = 0; j < 8; ++j) {
                 M [8*k + j] = M[8*k + j] + mult*M[8*i +j];
             }
         }
     }
 
     A = B = C = D = 0;
     for (j = 0; j < 4; ++j)
         A += V[j] * M[4 + j];
     for (j = 0; j < 4; ++j)
         B += V[j] * M[12 + j];
     for (j = 0; j < 4; ++j)
         C += V[j] * M[20 + j];
     for (j = 0; j < 4; ++j)
         D += V[j] * M[28 + j];

     //modified by Jin
     //return (A*load + B*slope + C*load*slope + D);
     double result = (A*load + B*slope + C*load*slope + D);
     if( result < 0.0 )
     {
	result = 0.000001; 
     }

     gradient = A + C*slope;
     
     return result;
   
}

struct pinInfo* getOutputPinDFF(const cellInfo* cell, const unsigned int library, const string& pinName){
	map <string, struct pinInfo*>* outputPinLib;
	
	switch	( library ){
		case 1:
		       outputPinLib = cell->svt_pinsMap;
		       break;	
		case 2:
		       outputPinLib = cell->hvt_pinsMap;
		       break;
		case 3:
		       outputPinLib = cell->lvt_pinsMap;
		       break;
		default:
		       cout << "Error! Wrong library information (svt, hvt, or lvt)!" << endl;
		       return NULL;
		       break;
	}

	map <string, struct pinInfo*>::iterator pos;

	for( pos = outputPinLib->begin(); pos != outputPinLib->end(); pos++){
		if( ( pos->second->direction == 0 ) && ( pinName == pos->first ) ) return pos->second;
	}

	cout << "Error! Cannot find output pin!" << endl;
}


struct pinInfo* getOutputPin(const cellInfo* cell, const unsigned int library){
	map <string, struct pinInfo*>* outputPinLib;
	
	switch	( library ){
		case 1:
		       outputPinLib = cell->svt_pinsMap;
		       break;	
		case 2:
		       outputPinLib = cell->hvt_pinsMap;
		       break;
		case 3:
		       outputPinLib = cell->lvt_pinsMap;
		       break;
		default:
		       cout << "Error! Wrong library information (svt, hvt, or lvt)!" << endl;
		       return NULL;
		       break;
	}

	map <string, struct pinInfo*>::iterator pos;

	for( pos = outputPinLib->begin(); pos != outputPinLib->end(); pos++){
		if( pos->second->direction == 0 ) return pos->second;
	}

	cout << "Error! Cannot find output pin!" << endl;
}


struct pinDelayInfo* getPinDelayInfo(const pinInfo* outputPin
		                    ,const pinInfo* inputPin
		                    ,const unsigned int arcEdge){
	
	map<string,vector<pinDelayInfo*>*>::iterator pos;
	map < string, vector<pinDelayInfo*>* > delayInfo;
	delayInfo = outputPin->delayInfo;
	
	vector<pinDelayInfo*>* delayInfoPtr = delayInfo[ inputPin->name  ];
	
	if ( delayInfoPtr == NULL ){
		cout << "Error! Cannot find the related input pin!" << endl;
		return NULL;
	}

	if( (*delayInfoPtr)[arcEdge]==NULL ){
		cout << "Error! Cannot find the related delayArcComponent!" << endl;
		return NULL;
	}
	
	return (*delayInfoPtr)[arcEdge];
	
}


struct pinInfo* getInputPin (const cellInfo* faninCell
		            ,const string& inputPinName
			    ,const unsigned int library){
	
	map <string, struct pinInfo*>* pinsMap;
//	const unsigned SVT = 1;
//	const unsigned HVT = 2;
//	const unsigned LVT = 3;
	
	switch ( library  ){
		case SVT:
			pinsMap = faninCell->svt_pinsMap;
			break;
		case HVT:
			pinsMap = faninCell->hvt_pinsMap;
			break;
		case LVT:
			pinsMap = faninCell->lvt_pinsMap;
			break;
		default:
			cout << "Error! Wrong library information (svt, hvt, or lvt)!" << endl;
			return NULL;
			break;
	}

	struct pinInfo* inputPinInfo;
	inputPinInfo = (*pinsMap)[ inputPinName ];
	if(inputPinInfo == NULL){
	   cout << "Error! Cannot find the correspoding inputPin by inputPinName: (" << inputPinName << ")" << endl;
	   cout << "library type: " << library << endl;
	}

	return inputPinInfo;
}


int dumpDelayInfo(pinDelayInfo* delayInfo){
	cout << "\nindex1:\n" << endl;
	for(unsigned int i=0; i<delayInfo->index1.size(); i++){
		cout << delayInfo->index1[i] << " ";
	}

	cout << "\nindex2:\n" << endl;
	for(unsigned int i=0; i<delayInfo->index2.size(); i++){
		cout << delayInfo->index2[i] << " ";
	}

	cout << "\nvalues:" << endl;
	for(unsigned int i=0; i<delayInfo->values.size(); i++){
		if( ( i%delayInfo->delayTable->i1 )==0 )  cout << "\n";
		cout << delayInfo->values[i] << " ";
	}

	cout << "\n";

	return 1;

}
/*
//Find the cell if exits in the cellsMap
bool Liberty::getCell(const string cellName){
        if ( cellsMap.find(cellName) != cellsMap.end() ) return true;
        else return false;
}

//Input a cellName which is indeed in the cellsMap and output cell's SVTName
string Liberty::getCellSVTName(const string cellName){
        map< string, cellInfo*>::iterator ptr = cellsMap.find(cellName);
    	return ptr->second->name;
}

//Input a cellName which is indeed in the cellsMap and output cell's HVTName
string Liberty::getCellHVTName(const string cellName){
        map< string, cellInfo*>::iterator ptr = cellsMap.find(cellName);
        return ptr->second->hvt_name;
}

//Input a cellName which is indeed in the cellsMap and output cell's LVTName
string Liberty::getCellLVTName(const string cellName){
        map< string, cellInfo*>::iterator ptr = cellsMap.find(cellName);
        return ptr->second->lvt_name;
}
*/

// Calculate next gate's transition or delay by given load and slope
double	Liberty::calculateTimingDFF (const cellInfo* cell
				 ,const string& outputPinName
				 ,const double transitionTime
                                 ,const double loadCap
                                 ,const unsigned int arcEdge
				 ,const unsigned int library) {
		 
		 struct pinInfo* outputPin;
		 struct pinDelayInfo* delayInfo;
		 struct pinInfo* inputPin;
		 
		 double index11, index12, index21, index22, result;
		 outputPin	= getOutputPinDFF( cell, library , outputPinName );
		 inputPin	= getInputPin( cell, DFF_CLK_PIN, library );
		 //inputPin	= getInputPin( cell, "CP", library );
		 delayInfo	= getPinDelayInfo( outputPin, inputPin, arcEdge );
     
		 //dumpDelayInfo(delayInfo);
     // Get the index11 and index12 for a given value
		 unsigned int norm1;
		 for( norm1=1; norm1<delayInfo->index1.size()-1; norm1++ ){
		    if( transitionTime<=delayInfo->index1[ norm1 ] ) break;
		 }
		 index11 = delayInfo->index1[ norm1-1 ];
		 index12 = delayInfo->index1[ norm1 ];
		 
		 // Get the index21 and index22 for a given value
		 unsigned int norm2;
		 for( norm2=1; norm2<delayInfo->index2.size()-1; norm2++ ){
		    if( loadCap<=delayInfo->index2[ norm2 ]) break;
		 }
		 index21 = delayInfo->index2[ norm2-1 ];
		 index22 = delayInfo->index2[ norm2 ];
		 
		 unsigned int location;
		 const int size = delayInfo->delayTable->i1;
		 location = (norm1-1) * (size) + (norm2-1);

		 //cout << "inputValues:" << transitionTime << " " << loadCap << "checkCFit2:" << delayInfo->values[ location ] << " " << delayInfo->values[ location+1 ] << " " << delayInfo->values[ location+size ] << " " <<  delayInfo->values[ location+size+1 ] << "\n";
		 
		 result = cFit2(index11, index12, index21, index22, delayInfo->values[ location ], delayInfo->values[ location+1 ], delayInfo->values[ location+size ], delayInfo->values[ location+size+1 ], transitionTime, loadCap);
		 //cout << "\n" <<result << endl;
		 return result;
}

//Added by Jin 20081015
// Calculate next gate's transition or delay by given load and slope
double	Liberty::calculateTimingDFFwithGradient (const cellInfo* cell
				 ,const string& outputPinName
				 ,const double transitionTime
                                 ,const double loadCap
                                 ,const unsigned int arcEdge
				 ,const unsigned int library
				 ,double &gradient ) {
		 
		 struct pinInfo* outputPin;
		 struct pinDelayInfo* delayInfo;
		 struct pinInfo* inputPin;
		 
		 double index11, index12, index21, index22, result;
		 outputPin	= getOutputPinDFF( cell, library , outputPinName );
		 inputPin	= getInputPin( cell, DFF_CLK_PIN, library );
		 //inputPin	= getInputPin( cell, "CP", library );
		 delayInfo	= getPinDelayInfo( outputPin, inputPin, arcEdge );
     
		 //dumpDelayInfo(delayInfo);
     // Get the index11 and index12 for a given value
		 unsigned int norm1;
		 for( norm1=1; norm1<delayInfo->index1.size()-1; norm1++ ){
		    if( transitionTime<=delayInfo->index1[ norm1 ] ) break;
		 }
		 index11 = delayInfo->index1[ norm1-1 ];
		 index12 = delayInfo->index1[ norm1 ];
		 
		 // Get the index21 and index22 for a given value
		 unsigned int norm2;
		 for( norm2=1; norm2<delayInfo->index2.size()-1; norm2++ ){
		    if( loadCap<=delayInfo->index2[ norm2 ]) break;
		 }
		 index21 = delayInfo->index2[ norm2-1 ];
		 index22 = delayInfo->index2[ norm2 ];
		 
		 unsigned int location;
		 const int size = delayInfo->delayTable->i1;
		 location = (norm1-1) * (size) + (norm2-1);

		 //cout << "inputValues:" << transitionTime << " " << loadCap << "checkCFit2:" << delayInfo->values[ location ] << " " << delayInfo->values[ location+1 ] << " " << delayInfo->values[ location+size ] << " " <<  delayInfo->values[ location+size+1 ] << "\n";
		 
		 result = cFit2withGradient(index11, index12, index21, index22, delayInfo->values[ location ], delayInfo->values[ location+1 ], delayInfo->values[ location+size ], delayInfo->values[ location+size+1 ], transitionTime, loadCap, gradient);
		 //cout << "\n" <<result << endl;
		 return result;
}


// Calculate next gate's transition or delay by given load and slope
double	Liberty::calculateTimingHoldSetup (const cellInfo* cell
//				 ,const string inputPinName
				 ,const double transitionTime
                                 ,const double loadCap
                                 ,const unsigned int arcEdge
				 ,const unsigned int library) {
		 
		 struct pinInfo* outputPin;
		 struct pinDelayInfo* delayInfo;
		 struct pinInfo* inputPin;
		 
		 double index11, index12, index21, index22, result;
		 outputPin	= getInputPin( cell, "D", library );
//		 inputPin	= getInputPin( cell, inputPinName, library );
		 inputPin	= getInputPin( cell, DFF_CLK_PIN, library );
		 //inputPin	= getInputPin( cell, "CP", library );
		 delayInfo	= getPinDelayInfo( outputPin, inputPin, arcEdge );
     
		 //dumpDelayInfo(delayInfo);
     // Get the index11 and index12 for a given value
		 unsigned int norm1;
		 for( norm1=1; norm1<delayInfo->index1.size()-1; norm1++ ){
		    if( transitionTime<=delayInfo->index1[ norm1 ] ) break;
		 }
		 index11 = delayInfo->index1[ norm1-1 ];
		 index12 = delayInfo->index1[ norm1 ];
		 
		 // Get the index21 and index22 for a given value
		 unsigned int norm2;
		 for( norm2=1; norm2<delayInfo->index2.size()-1; norm2++ ){
		    if( loadCap<=delayInfo->index2[ norm2 ]) break;
		 }
		 index21 = delayInfo->index2[ norm2-1 ];
		 index22 = delayInfo->index2[ norm2 ];
		 
		 unsigned int location;
		 const int size = delayInfo->delayTable->i1;
		 location = (norm1-1) * (size) + (norm2-1);

		 //cout << "inputValues:" << transitionTime << " " << loadCap << "checkCFit2:" << delayInfo->values[ location ] << " " << delayInfo->values[ location+1 ] << " " << delayInfo->values[ location+size ] << " " <<  delayInfo->values[ location+size+1 ] << "\n";
		 
		 result = cFit2(index11, index12, index21, index22, delayInfo->values[ location ], delayInfo->values[ location+1 ], delayInfo->values[ location+size ], delayInfo->values[ location+size+1 ], transitionTime, loadCap);
		 //cout << "\n" <<result<<endl;
		 return result;
}



// Calculate next gate's transition or delay by given load and slope
double	Liberty::calculateTiming (const cellInfo* cell
				 ,const string& inputPinName
				 ,const double transitionTime
                                 ,const double loadCap
                                 ,const unsigned int arcEdge
				 ,const unsigned int library) {
		 
		 struct pinInfo* outputPin;
		 struct pinDelayInfo* delayInfo;
		 struct pinInfo* inputPin;
		 
		 double index11, index12, index21, index22, result;
		 outputPin	= getOutputPin( cell, library );
		 inputPin	= getInputPin( cell, inputPinName, library );
		 delayInfo	= getPinDelayInfo( outputPin, inputPin, arcEdge );
     
		 //dumpDelayInfo(delayInfo);
     // Get the index11 and index12 for a given value
		 unsigned int norm1;
		 for( norm1=1; norm1<delayInfo->index1.size()-1; norm1++ ){
		    if( transitionTime<=delayInfo->index1[ norm1 ] ) break;
		 }
		 index11 = delayInfo->index1[ norm1-1 ];
		 index12 = delayInfo->index1[ norm1 ];
		 
		 // Get the index21 and index22 for a given value
		 unsigned int norm2;
		 for( norm2=1; norm2<delayInfo->index2.size()-1; norm2++ ){
		    if( loadCap<=delayInfo->index2[ norm2 ]) break;
		 }
		 index21 = delayInfo->index2[ norm2-1 ];
		 index22 = delayInfo->index2[ norm2 ];
		 
		 unsigned int location;
		 const int size = delayInfo->delayTable->i1;
		 location = (norm1-1) * (size) + (norm2-1);

		 //cout << "inputValues:" << transitionTime << " " << loadCap << "checkCFit2:" << delayInfo->values[ location ] << " " << delayInfo->values[ location+1 ] << " " << delayInfo->values[ location+size ] << " " <<  delayInfo->values[ location+size+1 ] << "\n";
		 
		 result = cFit2(index11, index12, index21, index22, delayInfo->values[ location ], delayInfo->values[ location+1 ], delayInfo->values[ location+size ], delayInfo->values[ location+size+1 ], transitionTime, loadCap);
		 //cout << "\t" <<result;
		 return result;
}

//Added by Jin 20081014
// Calculate next gate's transition or delay by given load and slope
double	Liberty::calculateTimingwithGradient (const cellInfo* cell
				 ,const string& inputPinName
				 ,const double transitionTime
                                 ,const double loadCap
                                 ,const unsigned int arcEdge
				 ,const unsigned int library
				 ,double& gradient ) {
		 
		 struct pinInfo* outputPin;
		 struct pinDelayInfo* delayInfo;
		 struct pinInfo* inputPin;
		 
		 double index11, index12, index21, index22, result;
		 outputPin	= getOutputPin( cell, library );
		 inputPin	= getInputPin( cell, inputPinName, library );
		 delayInfo	= getPinDelayInfo( outputPin, inputPin, arcEdge );
     
		 //dumpDelayInfo(delayInfo);
     // Get the index11 and index12 for a given value
		 unsigned int norm1;
		 for( norm1=1; norm1<delayInfo->index1.size()-1; norm1++ ){
		    if( transitionTime<=delayInfo->index1[ norm1 ] ) break;
		 }
		 index11 = delayInfo->index1[ norm1-1 ];
		 index12 = delayInfo->index1[ norm1 ];
		 
		 // Get the index21 and index22 for a given value
		 unsigned int norm2;
		 for( norm2=1; norm2<delayInfo->index2.size()-1; norm2++ ){
		    if( loadCap<=delayInfo->index2[ norm2 ]) break;
		 }
		 index21 = delayInfo->index2[ norm2-1 ];
		 index22 = delayInfo->index2[ norm2 ];
		 
		 unsigned int location;
		 const int size = delayInfo->delayTable->i1;
		 location = (norm1-1) * (size) + (norm2-1);

		 //cout << "inputValues:" << transitionTime << " " << loadCap << "checkCFit2:" << delayInfo->values[ location ] << " " << delayInfo->values[ location+1 ] << " " << delayInfo->values[ location+size ] << " " <<  delayInfo->values[ location+size+1 ] << "\n";
		 
		 //result = cFit2(index11, index12, index21, index22, delayInfo->values[ location ], delayInfo->values[ location+1 ], delayInfo->values[ location+size ], delayInfo->values[ location+size+1 ], transitionTime, loadCap);
		 result = cFit2withGradient(index11, index12, index21, index22, delayInfo->values[ location ], delayInfo->values[ location+1 ], delayInfo->values[ location+size ], delayInfo->values[ location+size+1 ], transitionTime, loadCap, gradient);
		 //cout << "\t" <<result;
		 return result;
}


cellInfo* Liberty::getCellType(string cellType){
     map<string, cellInfo*>::iterator pos;
     pos = cellsMap.find(cellType);
     
     if(pos==cellsMap.end()){ 
	cout << "Error! Cannot find the corresponding cellType!" << endl;
	return NULL;
     }
     
     return pos->second;
}


/* Get Group Name              */
/* I return only the first one */
char* Liberty::getName (si2drGroupIdT group) {

     si2drErrorT    err;
     si2drNamesIdT  groupNames;
     si2drStringT   groupName;

     groupNames = si2drGroupGetNames (group, &err);
     groupName = si2drIterNextName (groupNames, &err);
     si2drIterQuit (groupNames, &err);

     return (char *) groupName;
}


/* Return the Related Pin for the Arc */
//char *getRelatedPin (si2drGroupIdT timingGroup) {
string getRelatedPin (si2drGroupIdT timingGroup) {

     si2drErrorT    err;
     si2drAttrIdT   timingAttr;

     timingAttr = si2drGroupFindAttrByName (timingGroup, "related_pin", &err);
     if (!si2drObjectIsNull (timingAttr, &err)) {
	     char* tmp = strdup ((char *) si2drSimpleAttrGetStringValue (timingAttr, &err));
	     string ret = strtok(tmp, ", \\");
	     free(tmp);
	     return ret;
//	return strtok (strdup ((char *) si2drSimpleAttrGetStringValue (timingAttr, &err)), ", \\");
     }

     //return NULL;

}

/* Return the Related Pin for the Arc */
unsigned int getTimingSense (si2drGroupIdT timingGroup) {

     si2drErrorT    err;
     si2drAttrIdT   timingAttr;
     char* timingSenseChar = NULL;

     timingAttr = si2drGroupFindAttrByName (timingGroup, "timing_sense", &err);
     if (!si2drObjectIsNull (timingAttr, &err)) timingSenseChar = strtok (strdup ((char *) si2drSimpleAttrGetStringValue (timingAttr, &err)), ", \\");

     //cout << timingSenseChar << endl;
     string timingSenseString = timingSenseChar;
     
     if ( timingSenseString == "non_unate" ) return 0;
     else if ( timingSenseString == "positive_unate" ) return 1;
     else if ( timingSenseString == "negative_unate" ) return 2;
     else return 3;

     //return NULL;

}


/* Return the Cap of Pin */
double getPinCap (si2drGroupIdT pinGroup) {

     si2drErrorT    err;
     //si2drGroupIdT  timingGroup;
     si2drAttrIdT   timingAttr;
//     si2drStringT   pinAttrValue;

     //timingGroup = si2drIterNextGroup (timingGroups, &err);
     timingAttr = si2drGroupFindAttrByName (pinGroup, "capacitance", &err);
     if (!si2drObjectIsNull (timingAttr, &err)) return si2drSimpleAttrGetFloat64Value (timingAttr, &err);

     return -1;

     //return NULL;
}



/* Return the Direction of Pin */
/* 0 : input,  1 : output,  -1 : NULL */
int getPinDirection (si2drGroupIdT pinGroup) {

     si2drErrorT   err;
     si2drAttrIdT  pinAttr;
     si2drStringT        pinAttrValue;

     if (si2drObjectIsNull ((pinAttr = si2drGroupFindAttrByName (pinGroup, "direction", &err)), &err))
        return -1;
     pinAttrValue = si2drSimpleAttrGetStringValue (pinAttr, &err);

     if(!strcmp((char *) pinAttrValue, "input")) return 1;
     else if(!strcmp((char *) pinAttrValue, "output")) return 0;
     else return -1;

}


/* Search The Corresponding Lookup Table Templates Info for a Given Name */
struct lutInfo* Liberty::getLutInfo (char *name) {

	  
     for ( unsigned int i=0; i<lutInfoV.size(); i++  ){
        if (!strcmp(lutInfoV[i]->name, name)) return lutInfoV[i];
     }
     return NULL;
}



/* Test if Delay Arc Component */
int  getDelayArcComponent (char *delayArc) {  /* 1 : cell_rise,  2 : cell_fall,  3 : rise_transition,  4 : fall_transition */

     if(!strcmp(delayArc, "cell_rise")) return 1;
     else if (!strcmp(delayArc, "cell_fall")) return 2;
     else if (!strcmp(delayArc, "rise_transition")) return 3;
     else if (!strcmp(delayArc, "fall_transition")) return 4;
     else if (!strcmp(delayArc, "rise_constraint")) return 1;
     else if (!strcmp(delayArc, "fall_constraint")) return 2;
     else return -1;
}


/* Return the leakage power in the cell */
double getLeakage (si2drGroupIdT timingGroup) {

     si2drErrorT    err;
     //si2drGroupIdT  timingGroup;
     si2drAttrIdT   timingAttr;
//     si2drStringT   pinAttrValue;

     //timingGroup = si2drIterNextGroup (timingGroups, &err);
     timingAttr = si2drGroupFindAttrByName (timingGroup, "cell_leakage_power", &err);
     if (!si2drObjectIsNull (timingAttr, &err)) return si2drSimpleAttrGetFloat64Value (timingAttr, &err);

     return -1;

     //return NULL;
}


map <string, struct pinInfo*>* Liberty::setPinInfo (si2drGroupsIdT cellLevelGroups, bool isDFF) {
        
     si2drGroupIdT   cellLevelGroup;
     si2drGroupsIdT  pinLevelGroups;
     si2drGroupIdT   pinLevelGroup;
     si2drGroupsIdT  timingLevelGroups;
     si2drGroupIdT   timingLevelGroup;
     si2drErrorT     err;

     si2drValuesIdT  values;
     si2drValueTypeT valueType;
     si2drInt32T     integerValue;
     si2drFloat64T   floatValue;
     si2drStringT    stringValue;
     si2drBooleanT   booleanValue;
     si2drAttrIdT    arcAttr;
     
     char	*str;
     int	i;
     string	name;
        
     map <string, struct pinInfo*> *pins = new map <string, struct pinInfo*>;
     
     while (!si2drObjectIsNull ((cellLevelGroup = si2drIterNextGroup (cellLevelGroups, &err)), &err)) {
	if (!strcmp ((char *) si2drGroupGetGroupType (cellLevelGroup, &err), "pin")) {
	   struct pinInfo *insertPinInfo = new pinInfo;
           name 		    = getName (cellLevelGroup);
           insertPinInfo->name      = name;
	   insertPinInfo->direction = getPinDirection(cellLevelGroup);
	   insertPinInfo->cap	    = getPinCap(cellLevelGroup);

           pinLevelGroups = si2drGroupGetGroups (cellLevelGroup, &err);           
                      
           vector<pinDelayInfo*> *pinDelayInfoVectorBack = NULL;
	   bool check = false;
	   while (!si2drObjectIsNull ((pinLevelGroup = si2drIterNextGroup (pinLevelGroups, &err)), &err)) {
              //timingType = getTimingType (pinLevelGroup, cellLevelGroup);
	      if (!strcmp ((char *) si2drGroupGetGroupType (pinLevelGroup, &err), "timing")){
		 timingLevelGroups		= si2drGroupGetGroups (pinLevelGroup, &err);
                 string relatedPinName		= getRelatedPin (pinLevelGroup);
		 unsigned int timingSense	= getTimingSense (pinLevelGroup);
		 vector<pinDelayInfo*> *pinDelayInfoVector;
		 if ( check == false ){
		    pinDelayInfoVector = new vector<pinDelayInfo*>;
		 //if(isDFF==false) pinDelayInfoVector->reserve(4);
		 //else pinDelayInfoVector->reserve(2);
		    pinDelayInfoVector->resize(4);
		    insertPinInfo->delayInfo.insert( make_pair( relatedPinName, pinDelayInfoVector ) );
		    insertPinInfo->timingSense.insert( make_pair( relatedPinName, timingSense ) );
		 }else if ( check == true ){
		    pinDelayInfoVector = pinDelayInfoVectorBack;
		 }
		 
                 while (!si2drObjectIsNull ((timingLevelGroup	= si2drIterNextGroup (timingLevelGroups, &err)), &err)) {
                    struct pinDelayInfo *insertPinDelayInfo	= new pinDelayInfo;
                    insertPinDelayInfo->delayTable		= getLutInfo((char *) getName(timingLevelGroup));
                    insertPinDelayInfo->delayArcComponent	= getDelayArcComponent((char *)si2drGroupGetGroupType (timingLevelGroup, &err));
                    if ( check == true ) insertPinDelayInfo->delayArcComponent = insertPinDelayInfo->delayArcComponent + 2;
		    //insertPinDelayInfo->relatedPin		= relatedPinName;
		    //if (isDFF) cout << "\n" << insertPinInfo->name<<"\t"<<insertPinDelayInfo->delayArcComponent;
		    (*pinDelayInfoVector)[insertPinDelayInfo->delayArcComponent-1] = insertPinDelayInfo;
                    

                    if (!si2drObjectIsNull ((arcAttr = si2drGroupFindAttrByName (timingLevelGroup, "index_1", &err)), &err)) {
                       i = 1;
                       values = si2drComplexAttrGetValues (arcAttr, &err);
                       while (1) {
                          si2drIterNextComplexValue (values,
                                                     &valueType,
                                                     &integerValue,
                                                     &floatValue,
                                                     &stringValue,
                                                     &booleanValue,
                                                     &err);
                          if (valueType == SI2DR_UNDEFINED_VALUETYPE) break;
                          str = strtok ((char *) stringValue, ", \\\n");
                          insertPinDelayInfo->index1.push_back( atof (str) );
                          while (str = strtok (NULL, ", \\\n")) {
                             i++;
                             insertPinDelayInfo->index1.push_back( atof (str) );
                          }   
                       }   
                       si2drIterQuit (values, &err);
                    }
		    
                    if (!si2drObjectIsNull ((arcAttr = si2drGroupFindAttrByName (timingLevelGroup, "index_2", &err)), &err)) {
                       i = 1;
                       values = si2drComplexAttrGetValues (arcAttr, &err);
                       while (1) {
                          si2drIterNextComplexValue (values,
                                                     &valueType,
                                                     &integerValue,
                                                     &floatValue,
                                                     &stringValue,
                                                     &booleanValue,
                                                     &err);
                          if (valueType == SI2DR_UNDEFINED_VALUETYPE) break;
                          str = strtok ((char *) stringValue, ", \\\n");
                          insertPinDelayInfo->index2.push_back( atof (str) );
                          while (str = strtok (NULL, ", \\\n")) {
                             i++;
                             insertPinDelayInfo->index2.push_back( atof (str) );
                          }   
                       }   
                       si2drIterQuit (values, &err);
                    }

                    if (!si2drObjectIsNull ((arcAttr = si2drGroupFindAttrByName (timingLevelGroup, "values", &err)), &err)) {
                       i = 1;
                       values = si2drComplexAttrGetValues (arcAttr, &err);
                       while (1) {
                          si2drIterNextComplexValue (values,
                                                     &valueType,
                                                     &integerValue,
                                                     &floatValue,
                                                     &stringValue,
                                                     &booleanValue,
                                                     &err);
                          if (valueType == SI2DR_UNDEFINED_VALUETYPE) break;
                          str = strtok ((char *) stringValue, ", \\\n");
                          insertPinDelayInfo->values.push_back( atof (str) );
                          while (str = strtok (NULL, ", \\\n")) {
                             i++;
                             insertPinDelayInfo->values.push_back( atof (str) );
                          }   
                          //insertPinDelayInfo->index	= i;
                       }   
                       si2drIterQuit (values, &err);
                    }
		 }

	         if( ( isDFF == true ) && ( insertPinInfo->direction == 1 ) && ( check == false ) ){
	            check = true;
	            pinDelayInfoVectorBack = pinDelayInfoVector;
     		 }
	      }
	   }
           pins->insert(make_pair(name, insertPinInfo));
           //pins->push_back(insertPinInfo);
        }
        //si2drIterQuit (cellLevelGroup, &err);
        //si2drIterQuit (cellLevelGroup, &err);
     }
     return pins;
}


string nameFilter(string cellName, string filter){
     int location, length;
     string temp;

     length = cellName.length();
     location = cellName.find(filter);

     temp = cellName.substr(0, location) + cellName.substr(location+4, length-location+4);

     return temp;
}

map<string, cellInfo*>::iterator Liberty::getCellLocation (string cellName, string filter){

     string name = nameFilter(cellName, filter);
     //cout << name << "\n";
    /* 
     for( map<string, cellInfo*>::iterator ptr=cellsMap.begin(); ptr!=cellsMap.end(); ptr++ ){
        if( ptr->first==name ){
	   return ptr;
	}	
     }
*/
     map<string, cellInfo*>::iterator ptr = cellsMap.find(name);
     if(ptr == cellsMap.end() ){
	cout << "Error in HVT parser.\n";
//     	return NULL;
     }
     return ptr;
}



/* Stores Delay, Noise Information for all Arcs */
void Liberty::setLVTCellInfo (si2drGroupIdT libGroup) {

     si2drErrorT     err;
     si2drGroupsIdT  libLevelGroups;
     si2drGroupIdT   libLevelGroup;
     si2drGroupsIdT  cellLevelGroups;
/*     si2drGroupIdT   cellLevelGroup;
     si2drGroupsIdT  busLevelGroups;
     si2drGroupIdT   busLevelGroup;
     si2drGroupsIdT  pinLevelGroups;
     si2drGroupIdT   pinLevelGroup;
     si2drGroupsIdT  timingLevelGroups;
     si2drGroupIdT   timingLevelGroup;
*/
     string	cellName;
     
/*     char *pinName;
     char *busName;
     char *timingType;
     char *relatedPinName;

     int i;
     int cellCount = 0;
     
     struct delayInfo *ptrDelayInfo;
*/
     libLevelGroups = si2drGroupGetGroups (libGroup, &err);
     while (!si2drObjectIsNull ((libLevelGroup = si2drIterNextGroup (libLevelGroups, &err)), &err)) {
        if	(strcmp ((char *) si2drGroupGetGroupType (libLevelGroup, &err), "cell") != 0) continue;
        cellName = getName (libLevelGroup);
        map<string, cellInfo*>::iterator cellLocation = getCellLocation(cellName, "_LVT");
        
	cellLevelGroups 	= si2drGroupGetGroups (libLevelGroup, &err);

        cellLocation->second->lvt_leakage = getLeakage (libLevelGroup); 
	
	cellLocation->second->lvt_pinsMap = setPinInfo (cellLevelGroups, cellLocation->second->DFF);

	cellLocation->second->lvt_name	  = cellName;
				
        //si2drIterQuit (cellLevelGroups, &err);   
     }
     si2drIterQuit (libLevelGroups, &err);

}


/* Stores Delay, Noise Information for all Arcs */
void Liberty::setHVTCellInfo (si2drGroupIdT libGroup) {

     si2drErrorT     err;
     si2drGroupsIdT  libLevelGroups;
     si2drGroupIdT   libLevelGroup;
     si2drGroupsIdT  cellLevelGroups;
/*     si2drGroupIdT   cellLevelGroup;
     si2drGroupsIdT  busLevelGroups;
     si2drGroupIdT   busLevelGroup;
     si2drGroupsIdT  pinLevelGroups;
     si2drGroupIdT   pinLevelGroup;
     si2drGroupsIdT  timingLevelGroups;
     si2drGroupIdT   timingLevelGroup;
*/
     string	cellName;
     
/*     char *pinName;
     char *busName;
     char *timingType;
     char *relatedPinName;

     int i;
     int cellCount = 0;
     
     struct delayInfo *ptrDelayInfo;
*/
     libLevelGroups = si2drGroupGetGroups (libGroup, &err);
     while (!si2drObjectIsNull ((libLevelGroup = si2drIterNextGroup (libLevelGroups, &err)), &err)) {
        if (strcmp ((char *) si2drGroupGetGroupType (libLevelGroup, &err), "cell") != 0) continue;
        cellName = getName (libLevelGroup);
        map<string, cellInfo*>::iterator cellLocation = getCellLocation(cellName, "_HVT");
        
	cellLevelGroups 	= si2drGroupGetGroups (libLevelGroup, &err);

        cellLocation->second->hvt_leakage = getLeakage (libLevelGroup); 
	
	cellLocation->second->hvt_pinsMap = setPinInfo (cellLevelGroups, cellLocation->second->DFF);

	cellLocation->second->hvt_name	  = cellName;
				
        //si2drIterQuit (cellLevelGroups, &err);   
     }
     si2drIterQuit (libLevelGroups, &err);

}


bool checkDFF(string cellName){
     int i;
     i = cellName.find("DFF");
  
     if(i == -1) return false;
     else return true;
     
     return false;
}


/* Stores Delay, Noise Information for all Arcs */
void Liberty::setCellInfo (si2drGroupIdT libGroup) {

     si2drErrorT     err;
     si2drGroupsIdT  libLevelGroups;
     si2drGroupIdT   libLevelGroup;
     si2drGroupsIdT  cellLevelGroups;
/*     si2drGroupIdT   cellLevelGroup;
     si2drGroupsIdT  busLevelGroups;
     si2drGroupIdT   busLevelGroup;
     si2drGroupsIdT  pinLevelGroups;
     si2drGroupIdT   pinLevelGroup;
     si2drGroupsIdT  timingLevelGroups;
     si2drGroupIdT   timingLevelGroup;
*/
     string	cellName;
     
/*     char *pinName;
     char *busName;
     char *timingType;
     char *relatedPinName;

     int i;
     int cellCount = 0;
     
     struct delayInfo *ptrDelayInfo;
*/

     //Set primary input and primary output
     struct cellInfo *PICellInfo = new cellInfo;
     PICellInfo->name	= "IN";
     PICellInfo->DFF	= false;
     cellCount++;
     cellsMap.insert(make_pair("IN", PICellInfo));
     
     struct cellInfo *POCellInfo = new cellInfo;
     POCellInfo->name   = "OUT";
     POCellInfo->DFF    = false;
     cellCount++;
     cellsMap.insert(make_pair("OUT", POCellInfo));
     //End
     
     libLevelGroups = si2drGroupGetGroups (libGroup, &err);
     while (!si2drObjectIsNull ((libLevelGroup = si2drIterNextGroup (libLevelGroups, &err)), &err)) {
        if (strcmp ((char *) si2drGroupGetGroupType (libLevelGroup, &err), "cell") != 0) continue;
        struct cellInfo *insertCellInfo = new cellInfo;
        cellName = getName (libLevelGroup);
        
        insertCellInfo->name	= cellName;
	insertCellInfo->DFF	= checkDFF(cellName);
        cellLevelGroups 	= si2drGroupGetGroups (libLevelGroup, &err);

        insertCellInfo->svt_leakage = getLeakage (libLevelGroup); 
	
	insertCellInfo->svt_pinsMap = setPinInfo (cellLevelGroups, insertCellInfo->DFF);			
	cellsMap.insert(make_pair(cellName, insertCellInfo));
				
	cellCount++;
        si2drIterQuit (cellLevelGroups, &err);   
     }
     si2drIterQuit (libLevelGroups, &err);

}


int Liberty::checkLutInfo(){
     
     cout << lutInfoV.size() << "\n";
     
     for(unsigned int i=0; i<lutInfoV.size(); i++){
        cout << lutInfoV[i]->name << "\n";
        cout << "\t" << lutInfoV[i]->var1 << "\t" << lutInfoV[i]->i1 << "\n\t";
        for(unsigned int j=0; j<lutInfoV[i]->index1.size(); j++){
           cout << lutInfoV[i]->index1[j] << " ";
        }
        cout << "\n";


        cout << "\t" << lutInfoV[i]->var2 << "\t" << lutInfoV[i]->i2 << "\n\t";
        for(unsigned int j=0; j<lutInfoV[i]->index2.size(); j++){
           cout << lutInfoV[i]->index2[j] << " ";
        }
        cout << "\n";
        
     }
}

int Liberty::checkCellInfo () {
     map <string, cellInfo*>::iterator ptr;

     cout << cellsMap.size() << "\n";
     
     for( ptr=cellsMap.begin(); ptr!=cellsMap.end(); ptr++ ){
        cout << ptr->first << "\t" << ptr->second->name << "\t" << ptr->second->svt_leakage << "\n";
	
	if( (ptr->first != "IN") && (ptr->first != "OUT") ){
	for ( map <string, struct pinInfo*>::iterator pinPtr=ptr->second->svt_pinsMap->begin(); pinPtr!=ptr->second->svt_pinsMap->end(); pinPtr++  ){
	   cout << "\t" << pinPtr->first << "\t" <<  pinPtr->second->direction << "\t" << pinPtr->second->cap << "\n";
	   /*
	   for(  map < string, unsigned int >::iterator ptrTimingSense=pinPtr->second->timingSense.begin(); ptrTimingSense!=pinPtr->second->timingSense.end(); ptrTimingSense++ ){
	      cout << "\tTimingSense: "<<ptrTimingSense->first << "\t " << ptrTimingSense->second;
	   }
	   cout << "\n";
	   */
	   /* 
	   for(  map < string, vector<pinDelayInfo*>* >::iterator ptrDelay=pinPtr->second->delayInfo.begin(); ptrDelay!=pinPtr->second->delayInfo.end(); ptrDelay++ ){
	      unsigned int size;
	      //if( ( ptr->second->DFF == true ) ) size = ptrDelay->second->size() -2;
	      //else size = ptrDelay->second->size();
	      size = ptrDelay->second->size();
	      for( unsigned int i=0; i<size; i++ ){
	         cout << "name:" << ptrDelay->first << "\tdelayArcComponent:" << (*(ptrDelay->second))[i]->delayArcComponent << "\n";
		 cout << "\t\t index_1\n";
		 for( unsigned int j=0; j<(*(ptrDelay->second))[i]->index1.size(); j++ ){
		    cout << "\t" << (*(ptrDelay->second))[i]->index1[j];
		 }

		 cout << "\n\t\t index_2\n";
		 for( unsigned int j=0; j<(*(ptrDelay->second))[i]->index2.size(); j++ ){
		    cout << "\t" << (*(ptrDelay->second))[i]->index2[j];
		 }
		 cout << "\n\t\t values\n";
		 for( unsigned int j=0; j<(*(ptrDelay->second))[i]->values.size(); j++ ){
		    cout << "\t" << (*(ptrDelay->second))[i]->values[j];
		 }
		 cout << "\n";
	      }
	   }
	   */
	}
	}
     }
}



int Liberty::checkHVTCellInfo () {
     map <string, cellInfo*>::iterator ptr;

     cout << cellsMap.size() << "\n";
     
     for( ptr=cellsMap.begin(); ptr!=cellsMap.end(); ptr++ ){
        cout << ptr->first << "\t" << ptr->second->name << "\t" << ptr->second->hvt_leakage << "\n";
	for ( map <string, struct pinInfo*>::iterator pinPtr=ptr->second->hvt_pinsMap->begin(); pinPtr!=ptr->second->hvt_pinsMap->end(); pinPtr++  ){
	   cout << "\t" << pinPtr->first << "\t" <<  pinPtr->second->direction << "\t" << pinPtr->second->cap << "\n";
	   for(  map < string, vector<pinDelayInfo*>* >::iterator ptrDelay=pinPtr->second->delayInfo.begin(); ptrDelay!=pinPtr->second->delayInfo.end(); ptrDelay++ ){
	      unsigned int size;
	      if(ptr->second->DFF==true) size = ptrDelay->second->size() -2;
	      else size = ptrDelay->second->size();
	      for( unsigned int i=0; i<size; i++ ){
	         cout << "name:" << ptrDelay->first << "\tdelayArcComponent:" << (*(ptrDelay->second))[i]->delayArcComponent << "\n";
		 cout << "\t\t index_1\n";
		 for( unsigned int j=0; j<(*(ptrDelay->second))[i]->index1.size(); j++ ){
		    cout << "\t" << (*(ptrDelay->second))[i]->index1[j];
		 }

		 cout << "\n\t\t index_2\n";
		 for( unsigned int j=0; j<(*(ptrDelay->second))[i]->index2.size(); j++ ){
		    cout << "\t" << (*(ptrDelay->second))[i]->index2[j];
		 }
		 cout << "\n\t\t values\n";
		 for( unsigned int j=0; j<(*(ptrDelay->second))[i]->values.size(); j++ ){
		    cout << "\t" << (*(ptrDelay->second))[i]->values[j];
		 }
		 cout << "\n";
	      }
	   }
	}
     }
}



int Liberty::checkLVTCellInfo () {

     map <string, cellInfo*>::iterator ptr;

     cout << cellsMap.size() << "\n";
     
     for( ptr=cellsMap.begin(); ptr!=cellsMap.end(); ptr++ ){
        cout << ptr->first << "\t" << ptr->second->name << "\t" << ptr->second->lvt_leakage << "\n";
	for ( map <string, struct pinInfo*>::iterator pinPtr=ptr->second->lvt_pinsMap->begin(); pinPtr!=ptr->second->lvt_pinsMap->end(); pinPtr++  ){
	   cout << "\t" << pinPtr->first << "\t" <<  pinPtr->second->direction << "\t" << pinPtr->second->cap << "\n";
	   for(  map < string, vector<pinDelayInfo*>* >::iterator ptrDelay=pinPtr->second->delayInfo.begin(); ptrDelay!=pinPtr->second->delayInfo.end(); ptrDelay++ ){
	      unsigned int size;
	      if(ptr->second->DFF==true) size = ptrDelay->second->size() -2;
	      else size = ptrDelay->second->size();

	      for( unsigned int i=0; i<size; i++ ){
	         cout << "name:" << ptrDelay->first << "\tdelayArcComponent:" << (*(ptrDelay->second))[i]->delayArcComponent << "\n";
		 cout << "\t\t index_1\n";
		 for( unsigned int j=0; j<(*(ptrDelay->second))[i]->index1.size(); j++ ){
		    cout << "\t" << (*(ptrDelay->second))[i]->index1[j];
		 }

		 cout << "\n\t\t index_2\n";
		 for( unsigned int j=0; j<(*(ptrDelay->second))[i]->index2.size(); j++ ){
		    cout << "\t" << (*(ptrDelay->second))[i]->index2[j];
		 }
		 cout << "\n\t\t values\n";
		 for( unsigned int j=0; j<(*(ptrDelay->second))[i]->values.size(); j++ ){
		    cout << "\t" << (*(ptrDelay->second))[i]->values[j];
		 }
		 cout << "\n";
	      }
	   }
	}
        //if(ptr->second->DFF==true) cin >> a;
     }
}


int Liberty::check () {

    checkLutInfo ();
    checkCellInfo ();
    //checkHVTCellInfo (reportFile);
    //checkLVTCellInfo (reportFile);
    //checkCalculateTiming();

}


void Liberty::setLutInfo (si2drGroupIdT libGroup) {

     si2drErrorT     err;
     si2drGroupsIdT  libLevelGroups;
     si2drGroupIdT   libLevelGroup;
     si2drAttrIdT    lutAttr;
     si2drStringT    lutAttrValue;

     si2drValuesIdT  values;
     si2drValueTypeT valueType;
     si2drInt32T     integerValue;
     si2drFloat64T   floatValue;
     si2drStringT    stringValue;
     si2drBooleanT   booleanValue;

//     int     i;
     char    *str;

     
     libLevelGroups = si2drGroupGetGroups (libGroup, &err);
     while (!si2drObjectIsNull ((libLevelGroup = si2drIterNextGroup (libLevelGroups, &err)), &err)) {
        
	if (!strcmp ((char *) si2drGroupGetGroupType (libLevelGroup, &err), "lu_table_template")) {
           lutInfo *lutInfoPtr = new lutInfo;

           lutInfoPtr->name   = getName (libLevelGroup);

           lutAttr = si2drGroupFindAttrByName (libLevelGroup, "variable_1", &err);
           lutAttrValue = si2drSimpleAttrGetStringValue (lutAttr, &err);
           lutInfoPtr->var1 = (char *) lutAttrValue;
           

           if (!si2drObjectIsNull ((lutAttr = si2drGroupFindAttrByName (libLevelGroup, "variable_2", &err)), &err)) {
              lutAttrValue = si2drSimpleAttrGetStringValue (lutAttr, &err);
              lutInfoPtr->var2 = (char *) lutAttrValue;    
           }

           lutAttr = si2drGroupFindAttrByName (libLevelGroup, "index_1", &err);
           values = si2drComplexAttrGetValues (lutAttr, &err);
           lutInfoPtr->i1 = 1;  // size of index1 
           while (1) {
              si2drIterNextComplexValue (values,
                                         &valueType,
                                         &integerValue,
                                         &floatValue,
                                         &stringValue,
                                         &booleanValue,
                                         &err);
              if (valueType == SI2DR_UNDEFINED_VALUETYPE) break; 
              str = strtok ((char *) stringValue, ", \\\n"); 
	      lutInfoPtr->index1.push_back(atof(str));
              while (str = strtok (NULL, ", \\\n")) {
                 lutInfoPtr->index1.push_back(atof(str));
		 lutInfoPtr->i1++;
	      }
           }
           
           si2drIterQuit (values, &err);
           
           lutInfoPtr->i2 = 1; // size of index2
           if (!si2drObjectIsNull ((lutAttr = si2drGroupFindAttrByName (libLevelGroup, "index_2", &err)), &err)) {
              values = si2drComplexAttrGetValues (lutAttr, &err);
              while (1) {
                 si2drIterNextComplexValue (values,
                                            &valueType,
                                            &integerValue,
                                            &floatValue,
                                            &stringValue,
                                            &booleanValue,
                                            &err);
                 if (valueType == SI2DR_UNDEFINED_VALUETYPE) break;
                 str = strtok ((char *) stringValue, ", \\\n");
                 lutInfoPtr->index2.push_back(atof(str));
		 while (str = strtok (NULL, ", \\\n")) {
                    lutInfoPtr->index2.push_back(atof(str));
		    lutInfoPtr->i2++;
		 }   
              }   
              si2drIterQuit (values, &err);
           }
	   lutInfoV.push_back(lutInfoPtr);
        }
     }
     si2drIterQuit (libLevelGroups, &err);

}



int Liberty::parseLVTLib (char* libFile) {
     // Library Name 
//     char *libName;
     
     // DelayModel 1:LUT 0:Polynomial

     // Units 
/*     int loadUnit;
     double stdLoad;
     int timeUnit;
     int currentUnit;
     int voltageUnit;
     int resistanceUnit;
*/    
      
     si2drErrorT    err;
     si2drGroupIdT  libGroup;
     si2drGroupsIdT topLevelGroups;
/*     si2drAttrIdT   delayModelAttr;
     si2drStringT   delayModelValue;
     si2drAttrIdT   attr;
*/
     si2drPIInit (&err);

     si2drReadLibertyFile (libFile, &err); 
     if (err == SI2DR_INVALID_NAME) {
        printf ("Couldn't open library %s for screening... quitting!\n", libFile);
        exit (0);
     } else if (err == SI2DR_SYNTAX_ERROR) {
        printf ("Library %s has syntax errors... quitting!\n", libFile);
        exit (0);
     }

     topLevelGroups = si2drPIGetGroups (&err);
 
     libGroup = si2drIterNextGroup (topLevelGroups, &err);
     if (si2drObjectIsNull (libGroup, &err)) { // Exit if Not a .lib 
        printf ("Error while reading the library - %s\n", libFile);
        exit (0);
     }

     si2drIterQuit (topLevelGroups, &err);
     setLVTCellInfo (libGroup);
     
#ifdef VERBOSE
     cout << "LVT Liberty parser is OK!" << "\n\n";
#endif
     return 0;
}  


int Liberty::parseHVTLib (char* libFile) {
     // Library Name 
//     char *libName;
     
     // DelayModel 1:LUT 0:Polynomial

     // Units 
/*     int loadUnit;
     double stdLoad;
     int timeUnit;
     int currentUnit;
     int voltageUnit;
     int resistanceUnit;
*/    
      
     si2drErrorT    err;
     si2drGroupIdT  libGroup;
     si2drGroupsIdT topLevelGroups;
/*     si2drAttrIdT   delayModelAttr;
     si2drStringT   delayModelValue;
     si2drAttrIdT   attr;
*/
     si2drPIInit (&err);

     si2drReadLibertyFile (libFile, &err); 
     if (err == SI2DR_INVALID_NAME) {
        printf ("Couldn't open library %s for screening... quitting!\n", libFile);
        exit (0);
     } else if (err == SI2DR_SYNTAX_ERROR) {
        printf ("Library %s has syntax errors... quitting!\n", libFile);
        exit (0);
     }

     topLevelGroups = si2drPIGetGroups (&err);
 
     libGroup = si2drIterNextGroup (topLevelGroups, &err);
     if (si2drObjectIsNull (libGroup, &err)) { // Exit if Not a .lib 
        printf ("Error while reading the library - %s\n", libFile);
        exit (0);
     }

     si2drIterQuit (topLevelGroups, &err);
     setHVTCellInfo (libGroup);
     
#ifdef VERBOSE
     cout << "HVT Liberty parser is OK!" << "\n";
#endif
     return 0;
}  


int Liberty::parseLib (char* libFile) {

     
     	      
     // Library Name 
     char *libName;
     
     // DelayModel 1:LUT 0:Polynomial
     int delayModel;

     // Units 
/*     int loadUnit;
     double stdLoad;
     int timeUnit;
     int currentUnit;
     int voltageUnit;
     int resistanceUnit;
*/    
      
     si2drErrorT    err;
     si2drGroupIdT  libGroup;
     si2drGroupsIdT topLevelGroups;
     si2drAttrIdT   delayModelAttr;
     si2drStringT   delayModelValue;
//     si2drAttrIdT   attr;

     si2drPIInit (&err);

     si2drReadLibertyFile (libFile, &err); 
     if (err == SI2DR_INVALID_NAME) {
        printf ("Couldn't open library %s for screening... quitting!\n", libFile);
        exit (0);
     } else if (err == SI2DR_SYNTAX_ERROR) {
        printf ("Library %s has syntax errors... quitting!\n", libFile);
        exit (0);
     }

     topLevelGroups = si2drPIGetGroups (&err);
 
     libGroup = si2drIterNextGroup (topLevelGroups, &err);
     if (si2drObjectIsNull (libGroup, &err)) { // Exit if Not a .lib 
        printf ("Error while reading the library - %s\n", libFile);
        exit (0);
     }

     si2drIterQuit (topLevelGroups, &err);
     libName = getName (libGroup);

     // Exit if Generic Delay Model 
     if (!si2drObjectIsNull ((delayModelAttr = si2drGroupFindAttrByName (libGroup, "delay_model", &err)), &err)) {
        delayModelValue = si2drSimpleAttrGetStringValue (delayModelAttr, &err);
        if (!strcmp (delayModelValue, TABLE_LOOKUP_MODEL)) {
           delayModel = 1;
        } else if (!strcmp (delayModelValue, "polynomial")) {
           delayModel = 0;
        } else {
           printf ("%s delay model not supported\n\n", delayModelValue);
           exit (0);
        }
     } else {
        printf ("generic_cmos delay model not supported\n\n"); 
        exit (0);
     }

     // Get Units
     //loadUnit       = getLoadUnit       (libGroup);
     //timeUnit       = getTimeUnit       (libGroup);
     //currentUnit    = getCurrentUnit    (libGroup);
     //voltageUnit    = getVoltageUnit    (libGroup);
     //resistanceUnit = getResistanceUnit (libGroup);
     //fprintf(reportFile, "%d %d %d %d %d\n", loadUnit, timeUnit, currentUnit, voltageUnit, resistanceUnit);
     
     setLutInfo (libGroup);
     setCellInfo (libGroup);
     //getNoiseLutInfo (libGroup); 
     //getOCInfo (libGroup);

     //getCellList (libGroup)
     //getArcInfo (libGroup);
     //setOC ();
     
#ifdef VERBOSE
     cout << "SVT Liberty parser is OK!" << "\n";
#endif
     return 0;
}  
