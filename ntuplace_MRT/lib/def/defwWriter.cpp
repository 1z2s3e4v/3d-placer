/*
 *     This  file  is  part  of  the  Cadence  LEF/DEF  Open   Source
 *  Distribution,  Product Version 5.5, and is subject to the Cadence
 *  LEF/DEF Open Source License Agreement.   Your  continued  use  of
 *  this file constitutes your acceptance of the terms of the LEF/DEF
 *  Open Source License and an agreement to abide by its  terms.   If
 *  you  don't  agree  with  this, you must remove this and any other
 *  files which are part of the distribution and  destroy any  copies
 *  made.
 *
 *     For updates, support, or to become part of the LEF/DEF Community,
 *  check www.openeda.org for details.
 */

 
// This file contains code for implementing the defwriter 5.3
// It has all the functions user can call in their callbacks or
// just their writer to write out the correct lef syntax.
//
// Author: Wanda da Rosa
// Date:   Summer, 1998
//
// Revisions: 11/25/2002 - Added Craig Files fixes from Agilent
//                         Changed all (!name && !*name) to
//                         (!name || !*name)

#include "defwWriter.hpp"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "defiUtil.hpp"


// States of the writer.
#define DEFW_UNINIT            0
#define DEFW_INIT              1
#define DEFW_VERSION           2
#define DEFW_CASESENSITIVE     3
#define DEFW_BUSBIT            4
#define DEFW_DIVIDER           5
#define DEFW_DESIGN            6
#define DEFW_TECHNOLOGY        7
#define DEFW_ARRAY             8
#define DEFW_FLOORPLAN         9
#define DEFW_UNITS            10
#define DEFW_HISTORY          11
#define DEFW_PROP_START       12
#define DEFW_PROP             13
#define DEFW_PROP_END         14
#define DEFW_DIE_AREA         15
#define DEFW_ROW              16
#define DEFW_TRACKS           17
#define DEFW_GCELL_GRID       18
#define DEFW_DEFAULTCAP_START 19
#define DEFW_DEFAULTCAP       20
#define DEFW_DEFAULTCAP_END   21
#define DEFW_CANPLACE         22
#define DEFW_CANNOTOCCUPY     23
#define DEFW_VIA_START        24
#define DEFW_VIA              25
#define DEFW_VIAONE_END       26
#define DEFW_VIA_END          27
#define DEFW_REGION_START     28
#define DEFW_REGION           29
#define DEFW_REGION_END       30
#define DEFW_COMPONENT_START  31
#define DEFW_COMPONENT        32
#define DEFW_COMPONENT_END    33
#define DEFW_PIN_START        34
#define DEFW_PIN              35
#define DEFW_PIN_PORT         36
#define DEFW_PIN_END          37
#define DEFW_PINPROP_START    38
#define DEFW_PINPROP          39
#define DEFW_PINPROP_END      40
#define DEFW_SNET_START       41
#define DEFW_SNET             42
#define DEFW_SNET_OPTIONS     43
#define DEFW_SNET_ENDNET      44
#define DEFW_SNET_END         45
#define DEFW_PATH             46
#define DEFW_SHIELD           47
#define DEFW_NET_START        48
#define DEFW_NET              49
#define DEFW_NET_OPTIONS      50
#define DEFW_NET_ENDNET       51
#define DEFW_NET_END          52
#define DEFW_SUBNET           53
#define DEFW_NOSHIELD         54
#define DEFW_IOTIMING_START   55
#define DEFW_IOTIMING         56
#define DEFW_IOTIMING_END     57
#define DEFW_SCANCHAIN_START  58
#define DEFW_SCANCHAIN        59
#define DEFW_SCAN_FLOATING    60
#define DEFW_SCAN_ORDERED     61
#define DEFW_SCANCHAIN_END    62
#define DEFW_FPC_START        63
#define DEFW_FPC              64
#define DEFW_FPC_OPER         65
#define DEFW_FPC_OPER_SUM     66
#define DEFW_FPC_END          67
#define DEFW_GROUP_START      68
#define DEFW_GROUP            69
#define DEFW_GROUP_END        70 
#define DEFW_BLOCKAGE_START   71
#define DEFW_BLOCKAGE_LAYER   72
#define DEFW_BLOCKAGE_PLACE   73
#define DEFW_BLOCKAGE_RECT    74
#define DEFW_BLOCKAGE_END     75
#define DEFW_SLOT_START       76
#define DEFW_SLOT_LAYER       77
#define DEFW_SLOT_RECT        78
#define DEFW_SLOT_END         79
#define DEFW_FILL_START       80
#define DEFW_FILL_LAYER       81
#define DEFW_FILL_RECT        82
#define DEFW_FILL_END         83
#define DEFW_BEGINEXT_START   84
#define DEFW_BEGINEXT         85
#define DEFW_BEGINEXT_END     86 

#define DEFW_END              87

#define DEFW_DONE            999

#define MAXSYN                88


////////////////////////////////////////////
////////////////////////////////////////////
//
//        Global Variables
//
////////////////////////////////////////////
////////////////////////////////////////////
FILE* defwFile = 0;   // File to write to.
int defwLines = 0;    // number of lines written
int defwState = DEFW_UNINIT;  // Current state of writer
int defwFunc = DEFW_UNINIT;   // Current function of writer
int defwDidNets = 0;  // required section
int defwDidComponents = 0;  // required section
int defwDidInit = 0;  // required section
int defwCounter = 0;  // number of nets, components in section
int defwLineItemCounter = 0; // number of items on current line
int defwFPC = 0;  // Current number of items in constraints/operand/sum
int defwHasInit = 0;    // for defwInit has called
int defwHasInitCbk = 0; // for defwInitCbk has called
int defwPinLayer = 0;   // for defwPin
int defwSpNetShield = 0; // for special net shieldNetName
static double defVersionNum = 5.5;  // default to 5.5
static int    defwObsoleteNum = -1; // keep track the obsolete syntax for error

char defwStateStr[MAXSYN] [80] = {"UNINITIALIZE",         //  0
                                  "INITIALIZE",           //  1
                                  "VERSION",              //  2
                                  "CASESENSITIVE",        //  3
                                  "BUSBIT",               //  4
                                  "DIVIDER",              //  5
                                  "DESIGN",               //  6
                                  "TECHNOLOGY",           //  7
                                  "ARRAY",                //  8
                                  "FLOORPLAN",            //  9
                                  "UNITS",                // 10
                                  "HISTORY",              // 11
                                  "PROPERTYDEFINITIONS",  // 12
                                  "PROPERTYDEFINITIONS",  // 13
                                  "PROPERTYDEFINITIONS",  // 14
                                  "DIEAREA",              // 15
                                  "ROW",                  // 16
                                  "TRACKS",               // 17
                                  "GCELLGRID",            // 18
                                  "DEFAULTCAP",           // 19
                                  "DEFAULTCAP",           // 20
                                  "DEFAULTCAP",           // 21
                                  "CANPLACE",             // 22
                                  "CANNOTOCCUPY",         // 23
                                  "VIA",                  // 24
                                  "VIA",                  // 25
                                  "VIA",                  // 26
                                  "VIA",                  // 27
                                  "REGION",               // 28
                                  "REGION",               // 29
                                  "REGION",               // 30
                                  "COMPONENT",            // 31
                                  "COMPONENT",            // 32
                                  "COMPONENT",            // 33
                                  "PIN",                  // 34
                                  "PIN",                  // 35
                                  "PIN",                  // 36
                                  "PIN",                  // 37
                                  "PINPROPERTY",          // 38
                                  "PINPROPERTY",          // 39
                                  "PINPROPERTY",          // 40
                                  "SNET",                 // 41
                                  "SNET",                 // 42
                                  "SNET",                 // 43
                                  "SNET",                 // 44
                                  "SNET",                 // 45
                                  "PATH",                 // 46
                                  "SHIELD",               // 47
                                  "NET",                  // 48
                                  "NET",                  // 49
                                  "NET",                  // 50
                                  "NET",                  // 51
                                  "NET",                  // 52
                                  "SUBNET",               // 53
                                  "NOSHIELD",             // 54
                                  "IOTIMING",             // 55
                                  "IOTIMING",             // 56
                                  "IOTIMING",             // 57
                                  "SCANCHAIN",            // 58
                                  "SCANCHAIN",            // 59
                                  "SCAN FLOATING",        // 60
                                  "SCAN ORDERED",         // 61
                                  "SCANCHAIN",            // 62
                                  "CONSTRAINTS",          // 63
                                  "CONSTRAINTS",          // 64
                                  "CONSTRAINTS",          // 65
                                  "CONSTRAINTS",          // 66
                                  "CONSTRAINTS",          // 67
                                  "GROUP",                // 68
                                  "GROUP",                // 69
                                  "GROUP",                // 70
                                  "BLOCKAGE",             // 71
                                  "BLOCKAGE",             // 72
                                  "BLOCKAGE",             // 73
                                  "BLOCKAGE",             // 74
                                  "BLOCKAGE",             // 75
                                  "SLOT",                 // 76
                                  "SLOT",                 // 77
                                  "SLOT",                 // 78
                                  "SLOT",                 // 79
                                  "FILL",                 // 80
                                  "FILL",                 // 81
                                  "FILL",                 // 82
                                  "FILL",                 // 83
                                  "BEGINEXT",             // 84
                                  "BEGINEXT",             // 85
                                  "BEGINEXT",             // 86
                                  "DESIGN END",           // 87
                                 };


int defwNewLine() {
  if (! defwFile) return DEFW_BAD_ORDER;
  fprintf(defwFile, "\n");
  return DEFW_OK;
}

// this function is required to be called first to initialize the required
// sections.
// Either this function or defwInitCbk can be called, cannot be both
int defwInit(FILE* f,
            int vers1, int vers2,
	    const char* caseSensitive,
	    const char* dividerChar,
	    const char* busBitChars,
	    const char* designName,
	    const char* technology,  /* optional */
	    const char* array,       /* optional */
	    const char* floorplan,   /* optional */
	    double units             /* optional  (set to -1 to ignore) */
       ) {

  /* Check if the user is valid */
  /* don't need anymore
  if (!defiValidUser())
      exit(77);
  */

  //if (defwFile) return DEFW_BAD_ORDER;
  defwFile = f;
  if (defwHasInitCbk == 1) {  // defwInitCbk has already called, issue an error
     fprintf(stderr,
        "ERROR: defwInitCbk has already called, cannot call defwInit again.\n");
     fprintf(stderr, "Writer Exit.\n");
    exit(DEFW_BAD_ORDER);
  }

  defwState = DEFW_UNINIT;  // Current state of writer
  defwFunc = DEFW_UNINIT;   // Current function of writer
  defwDidNets = 0;  // required section
  defwDidComponents = 0;  // required section
  defwDidInit = 0;  // required section

  if (vers1 < 0 || vers2 < 0) return DEFW_BAD_DATA;
  fprintf(defwFile, "VERSION %d.%d ;\n", vers1, vers2);

  if (caseSensitive == 0 || *caseSensitive == 0) return DEFW_BAD_DATA;
  fprintf(defwFile, "NAMESCASESENSITIVE %s ;\n", caseSensitive);

  if (dividerChar == 0 || *dividerChar == 0) return DEFW_BAD_DATA;
  fprintf(defwFile, "DIVIDERCHAR \"%s\" ;\n", dividerChar);

  if (busBitChars == 0 || *busBitChars == 0) return DEFW_BAD_DATA;
  fprintf(defwFile, "BUSBITCHARS \"%s\" ;\n", busBitChars);

  if (designName == 0 || *designName == 0) return DEFW_BAD_DATA;
  fprintf(defwFile, "DESIGN %s ;\n", designName);

  defwLines = 5;

  if (technology) {
    fprintf(defwFile, "TECHNOLOGY %s ;\n", technology);
    defwLines++;
  }

  if (array) {
    fprintf(defwFile, "ARRAY %s ;\n", array);
    defwLines++;
  }

  if (floorplan) {
    fprintf(defwFile, "FLOORPLAN %s ;\n", floorplan);
    defwLines++;
  }

  if (units != -1.0) {
    fprintf(defwFile, "UNITS DISTANCE MICRONS %d ;\n", ROUND(units));
    defwLines++;
  }

  defwDidInit = 1;
  defwState = DEFW_DESIGN;
  defwHasInit = 1;
  return DEFW_OK;
}


// this function is required to be called first to initialize the variables
// Either this function or defwInit can be called, cannot be both
int defwInitCbk(FILE* f) {

  defwFile = f;
  if (defwHasInit == 1) {  // defwInit has already called, issue an error
    fprintf(stderr,
        "ERROR: defwInit has already called, cannot call defwInit again.\n");
    fprintf(stderr, "Writer Exit.\n");
    exit(DEFW_BAD_ORDER);
  }

  defwState = DEFW_UNINIT;  // Current state of writer
  defwFunc = DEFW_UNINIT;   // Current function of writer
  defwDidNets = 0;  // required section
  defwDidComponents = 0;  // required section
  defwDidInit = 0;  // required section

  defwDidInit = 1;
  defwState = DEFW_INIT;
  defwHasInitCbk = 1;
  return DEFW_OK;
}

int defwVersion(int vers1, int vers2) {
  defwFunc = DEFW_VERSION;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState != DEFW_INIT)  // version follows init 
    return DEFW_BAD_ORDER;
  fprintf(defwFile, "VERSION %d.%d ;\n", vers1, vers2);
  if (vers2 >= 10)
     defVersionNum = vers1 + (vers2 / 100.0);
  else
     defVersionNum = vers1 + (vers2 / 10.0);
  defwLines++;

  defwState = DEFW_VERSION;
  return DEFW_OK;
}

int defwCaseSensitive(const char* caseSensitive) {
  defwFunc = DEFW_CASESENSITIVE;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState != DEFW_VERSION)  // casesensitive follows version
    return DEFW_BAD_ORDER;
  if (strcmp(caseSensitive, "ON") && strcmp(caseSensitive, "OFF"))
        return DEFW_BAD_DATA;     // has to be either ON or OFF
  fprintf(defwFile, "NAMESCASESENSITIVE %s ;\n", caseSensitive);
  defwLines++;

  defwState = DEFW_CASESENSITIVE;
  return DEFW_OK;
}

int defwBusBitChars(const char* busBitChars) {
  defwFunc = DEFW_BUSBIT;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState != DEFW_CASESENSITIVE)  // busbit follows casesensitive
    return DEFW_BAD_ORDER;
  if (busBitChars && busBitChars != 0 && *busBitChars != 0) {
     fprintf(defwFile, "BUSBITCHARS \"%s\" ;\n", busBitChars);
     defwLines++;
  }

  defwState = DEFW_BUSBIT;
  return DEFW_OK;
}

int defwDividerChar(const char* dividerChar) {
  defwFunc = DEFW_DIVIDER;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState != DEFW_BUSBIT)  // divider follows busbit
    return DEFW_BAD_ORDER;
  if (dividerChar && dividerChar != 0 && *dividerChar != 0) {
     fprintf(defwFile, "DIVIDERCHAR \"%s\" ;\n", dividerChar);
     defwLines++;
  }

  defwState = DEFW_DIVIDER;
  return DEFW_OK;
}

int defwDesignName(const char* name) {
  defwFunc = DEFW_DESIGN;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState != DEFW_DIVIDER)  // design follows divider
    return DEFW_BAD_ORDER;
  if (name && name != 0 && *name != 0) {
     fprintf(defwFile, "DESIGN %s ;\n", name);
     defwLines++;
  }

  defwState = DEFW_DESIGN;
  return DEFW_OK;
}

int defwTechnology(const char* technology) {
  defwFunc = DEFW_TECHNOLOGY;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState != DEFW_DESIGN)  // technology follows design
    return DEFW_BAD_ORDER;
  if (technology && technology != 0 && *technology != 0) {
     fprintf(defwFile, "TECHNOLOGY %s ;\n", technology);
     defwLines++;
  }

  defwState = DEFW_TECHNOLOGY;
  return DEFW_OK;
}

int defwArray(const char* array) {
  defwFunc = DEFW_ARRAY;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState < DEFW_DESIGN || defwState >= DEFW_ARRAY)
    return DEFW_BAD_ORDER;     // array follows design
  if (array && array != 0 && *array != 0) {
     fprintf(defwFile, "ARRAY %s ;\n", array);
     defwLines++;
  }

  defwState = DEFW_ARRAY;
  return DEFW_OK;
}

int defwFloorplan(const char* floorplan) {
  defwFunc = DEFW_FLOORPLAN;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState < DEFW_DESIGN || defwState >= DEFW_FLOORPLAN)
    return DEFW_BAD_ORDER;     // floorplan follows design
  if (floorplan && floorplan != 0 && *floorplan != 0) {
     fprintf(defwFile, "FLOORPLAN %s ;\n", floorplan);
     defwLines++;
  }

  defwState = DEFW_FLOORPLAN;
  return DEFW_OK;
}
#define DEFW_UNITS            10

int defwUnits(int units) {
  defwFunc = DEFW_UNITS;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState < DEFW_DESIGN || defwState >= DEFW_UNITS)
    return DEFW_BAD_ORDER;     // units follows design
  if (units != 0) {
     fprintf(defwFile, "UNITS DISTANCE MICRONS %d ;\n", units);
     defwLines++;
  }

  defwState = DEFW_UNITS;
  return DEFW_OK;
}

int defwHistory(const char* string) {
  char* c;
  defwFunc = DEFW_HISTORY;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (string == 0 || *string == 0) return DEFW_BAD_DATA;
  if (defwState < DEFW_DESIGN || defwState > DEFW_HISTORY) 
    return DEFW_BAD_ORDER;

  for (c = (char*)string; *c; c++)
    if (*c == '\n') defwLines++;

  fprintf(defwFile, "HISTORY %s ;\n", string);
  defwLines++;

  defwState = DEFW_HISTORY;
  return DEFW_OK;
}

int defwStartPropDef() {
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_PROP_START) return DEFW_BAD_ORDER;

  fprintf(defwFile, "PROPERTYDEFINITIONS\n");
  defwLines++;

  defwState = DEFW_PROP_START;
  return DEFW_OK;
}


int defwIntPropDef(const char* objType,
                   const char* propName,
                   double leftRange, double rightRange,    /* optional */
                   int    propValue                        /* optional */
               ) {
 
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PROP_START &&
      defwState != DEFW_PROP) return DEFW_BAD_ORDER;
  if ((! objType && ! *objType) || (! propName && ! *propName)) // require
     return DEFW_BAD_DATA;
 
  if (strcmp(objType, "DESIGN") && strcmp(objType, "COMPONENT") &&
      strcmp(objType, "NET") && strcmp(objType, "SPECIALNET") &&
      strcmp(objType, "GROUP") && strcmp(objType, "ROW") &&
      strcmp(objType, "COMPONENTPIN") && strcmp(objType, "REGION"))
     return DEFW_BAD_DATA;
 
  fprintf(defwFile, "   %s %s INTEGER ", objType, propName);
  if (leftRange || rightRange)
     fprintf(defwFile, "RANGE %g %g ", leftRange, rightRange);

  if (propValue)
     fprintf(defwFile, "%d ", propValue);

  fprintf(defwFile, ";\n");
 
  defwLines++;
  defwState = DEFW_PROP;
  return DEFW_OK;
}
 
 
int defwRealPropDef(const char* objType,
                    const char* propName,
                    double leftRange, double rightRange,    /* optional */
                    double propValue                        /* optional */
               ) {
 
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PROP_START &&
      defwState != DEFW_PROP) return DEFW_BAD_ORDER;
  if ((! objType && ! *objType) || (! propName && ! *propName)) // require
     return DEFW_BAD_DATA;
 
  if (strcmp(objType, "DESIGN") && strcmp(objType, "COMPONENT") &&
      strcmp(objType, "NET") && strcmp(objType, "SPECIALNET") &&
      strcmp(objType, "GROUP") && strcmp(objType, "ROW") &&
      strcmp(objType, "COMPONENTPIN") && strcmp(objType, "REGION"))
     return DEFW_BAD_DATA;
 
  fprintf(defwFile, "   %s %s REAL ", objType, propName);
  if (leftRange || rightRange)
     fprintf(defwFile, "RANGE %g %g ", leftRange, rightRange);

  if (propValue)
     fprintf(defwFile, "%g ", propValue);

  fprintf(defwFile, ";\n");
 
  defwLines++;
  defwState = DEFW_PROP;
  return DEFW_OK;
}
 
 
int defwStringPropDef(const char* objType,
                      const char* propName,
                      double leftRange, double rightRange,    /* optional */
                      const char* propValue                   /* optional */
               ) {
 
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PROP_START &&
      defwState != DEFW_PROP) return DEFW_BAD_ORDER;
  if ((! objType && ! *objType) || (! propName && ! *propName))
     return DEFW_BAD_DATA;
 
  if (strcmp(objType, "DESIGN") && strcmp(objType, "COMPONENT") &&
      strcmp(objType, "NET") && strcmp(objType, "SPECIALNET") &&
      strcmp(objType, "GROUP") && strcmp(objType, "ROW") &&
      strcmp(objType, "COMPONENTPIN") && strcmp(objType, "REGION"))
     return DEFW_BAD_DATA;
 
  fprintf(defwFile, "   %s %s STRING ", objType, propName);
  if (leftRange || rightRange)
     fprintf(defwFile, "RANGE %g %g ", leftRange, rightRange);

  if (propValue)
     fprintf(defwFile, "\"%s\" ", propValue);  // string, set quotes

  fprintf(defwFile, ";\n");
 
  defwLines++;
  defwState = DEFW_PROP;
  return DEFW_OK;
}


int defwEndPropDef() {
  defwFunc = DEFW_PROP_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PROP_START &&
      defwState != DEFW_PROP) return DEFW_BAD_ORDER;

  fprintf(defwFile, "END PROPERTYDEFINITIONS\n\n");
  defwLines++;

  defwState = DEFW_PROP_END;
  return DEFW_OK;
}


int defwStringProperty(const char* propName, const char* propValue) {
  if ((defwState != DEFW_ROW) && (defwState != DEFW_REGION) &&
      (defwState != DEFW_COMPONENT) && (defwState != DEFW_PIN) &&
      (defwState != DEFW_SNET) && (defwState != DEFW_NET) &&
      (defwState != DEFW_GROUP) && (defwState != DEFW_PINPROP) &&
      (defwState != DEFW_SNET_OPTIONS) && (defwState != DEFW_NET_OPTIONS) &&
      (defwState != DEFW_BEGINEXT))
      return DEFW_BAD_ORDER;

  // new line for the defwRow of the previous line
  // do not end with newline, may have more than on properties
  fprintf(defwFile, "\n      + PROPERTY %s \"%s\" ", propName, propValue);
  defwLines++;
  return DEFW_OK;
}


int defwRealProperty(const char* propName, double propValue) {
  if ((defwState != DEFW_ROW) && (defwState != DEFW_REGION) &&
      (defwState != DEFW_COMPONENT) && (defwState != DEFW_PIN) &&
      (defwState != DEFW_SNET) && (defwState != DEFW_NET) &&
      (defwState != DEFW_GROUP) && (defwState != DEFW_PINPROP) &&
      (defwState != DEFW_SNET_OPTIONS) && (defwState != DEFW_NET_OPTIONS))
      return DEFW_BAD_ORDER;

  // new line for the defwRow of the previous line
  // do not end with newline, may have more than on properties
  fprintf(defwFile, "\n      + PROPERTY %s %g ", propName, propValue);
  defwLines++;
  return DEFW_OK;
}


int defwIntProperty(const char* propName, int propValue) {
  if ((defwState != DEFW_ROW) && (defwState != DEFW_REGION) &&
      (defwState != DEFW_COMPONENT) && (defwState != DEFW_PIN) &&
      (defwState != DEFW_SNET) && (defwState != DEFW_NET) &&
      (defwState != DEFW_GROUP) && (defwState != DEFW_PINPROP) &&
      (defwState != DEFW_SNET_OPTIONS) && (defwState != DEFW_NET_OPTIONS))
      return DEFW_BAD_ORDER;

  // new line for the defwRow of the previous line
  // do not end with newline, may have more than on properties
  fprintf(defwFile, "\n      + PROPERTY %s %d ", propName, propValue);
  defwLines++;
  return DEFW_OK;
}


int defwDieArea(int xl, int yl, int xh, int yh) {
  defwFunc = DEFW_DIE_AREA;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_DIE_AREA) return DEFW_BAD_ORDER;
  if (xl > xh || yl > yh) return DEFW_BAD_DATA;

  fprintf(defwFile, "DIEAREA ( %d %d ) ( %d %d ) ;\n", xl, yl, xh, yh);
  defwLines++;

  defwState = DEFW_DIE_AREA;
  return DEFW_OK;
}


char* defwAddr(const char* x) {
  return (char*)x;
}


char* defwOrient(int num) {
  switch (num) {
  case 0: return defwAddr("N");
  case 1: return defwAddr("W");
  case 2: return defwAddr("S");
  case 3: return defwAddr("E");
  case 4: return defwAddr("FN");
  case 5: return defwAddr("FW");
  case 6: return defwAddr("FS");
  case 7: return defwAddr("FE");
  };
  return defwAddr("BOGUS ");
}


int defwRow(const char* rowName, const char* rowType,
     int x_orig, int y_orig,
     int orient,
     int do_count, int do_increment,
     int do_x, int do_y) {
  defwFunc = DEFW_ROW;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState > DEFW_ROW) return DEFW_BAD_ORDER;

  if (defwState == DEFW_ROW)
     fprintf(defwFile, ";\n");// add the ; and newline for the previous row.

  if ((! rowName && ! *rowName) || (! rowType && ! *rowType)) // required
     return DEFW_BAD_DATA;

  // do not have ; because the row may have properties
  // do not end with newline, if there is no property, ; need to be concat.
  fprintf(defwFile, "ROW %s %s %d %d %s DO %d BY %d STEP %d %d ",
          rowName, rowType, x_orig, y_orig, defwOrient(orient),
          do_count, do_increment, do_x, do_y);
  defwLines++;

  defwState = DEFW_ROW;
  return DEFW_OK;
}


int defwRowStr(const char* rowName, const char* rowType,
     int x_orig, int y_orig,
     const char* orient,
     int do_count, int do_increment,
     int do_x, int do_y) {
  defwFunc = DEFW_ROW;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState > DEFW_ROW) return DEFW_BAD_ORDER;

  if (defwState == DEFW_ROW)
     fprintf(defwFile, ";\n");// add the ; and newline for the previous row.

  if ((! rowName && ! *rowName) || (! rowType && ! *rowType)) // required
     return DEFW_BAD_DATA;

  // do not have ; because the row may have properties
  // do not end with newline, if there is no property, ; need to be concat.
  fprintf(defwFile, "ROW %s %s %d %d %s DO %d BY %d STEP %d %d ",
          rowName, rowType, x_orig, y_orig, orient,
          do_count, do_increment, do_x, do_y);
  defwLines++;

  defwState = DEFW_ROW;
  return DEFW_OK;
}


int defwTracks(const char* master, int do_start,
     int do_cnt, int do_step, int num_layers, const char** layers) {
  int i;

  defwFunc = DEFW_TRACKS;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState > DEFW_TRACKS) return DEFW_BAD_ORDER;

  if (defwState == DEFW_ROW)
     fprintf(defwFile, ";\n\n");  // add the ; and \n for the previous row

  if (! master && ! *master) // required
     return DEFW_BAD_DATA;
  if (strcmp(master, "X") && strcmp(master, "Y"))
     return DEFW_BAD_DATA;

  
  fprintf(defwFile, "TRACKS %s %d DO %d STEP %d LAYER",
          master, do_start, do_cnt, do_step);
  for (i = 0; i < num_layers; i++) {
     fprintf(defwFile, " %s", layers[i]);
  }
  fprintf(defwFile, " ;\n");
  defwLines++;

  defwState = DEFW_TRACKS;
  return DEFW_OK;
}


int defwGcellGrid(const char* master, int do_start,
     int do_cnt, int do_step) {
  defwFunc = DEFW_GCELL_GRID;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState > DEFW_GCELL_GRID) return DEFW_BAD_ORDER;

  if (defwState == DEFW_ROW)
     fprintf(defwFile, ";\n\n");  // add the ; and \n for the previous row.

  if (! master && ! *master) // required
     return DEFW_BAD_DATA;
  if (strcmp(master, "X") && strcmp(master, "Y"))
     return DEFW_BAD_DATA;

  fprintf(defwFile, "GCELLGRID %s %d DO %d STEP %d ;\n", master, do_start,
          do_cnt, do_step);
  defwLines++;

  defwState = DEFW_GCELL_GRID;
  return DEFW_OK;
}


int defwStartDefaultCap(int count) {
  defwObsoleteNum = DEFW_DEFAULTCAP_START;
  defwFunc = DEFW_DEFAULTCAP_START;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_DEFAULTCAP_START) return DEFW_BAD_ORDER;
  if (defVersionNum >= 5.4) return DEFW_OBSOLETE;

  if (defwState == DEFW_ROW)
     fprintf(defwFile, ";\n\n");  // add the ; and \n for the previous row.

  fprintf(defwFile, "DEFAULTCAP %d\n", count);
  defwLines++;

  defwCounter = count;
  defwState = DEFW_DEFAULTCAP_START;
  return DEFW_OK;
}


int defwDefaultCap(int pins, double cap) {
  defwFunc = DEFW_DEFAULTCAP;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_DEFAULTCAP_START &&
      defwState != DEFW_DEFAULTCAP) return DEFW_BAD_ORDER;

  fprintf(defwFile, "   MINPINS %d WIRECAP %f ;\n", pins, cap);
  defwLines++;
  defwCounter--;

  defwState = DEFW_DEFAULTCAP;
  return DEFW_OK;
}


int defwEndDefaultCap() {
  defwFunc = DEFW_DEFAULTCAP_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_DEFAULTCAP_START &&
      defwState != DEFW_DEFAULTCAP) return DEFW_BAD_ORDER;
  if (defwCounter) return DEFW_BAD_DATA;

  fprintf(defwFile, "END DEFAULTCAP\n\n");
  defwLines++;

  defwState = DEFW_DEFAULTCAP_END;
  return DEFW_OK;
}


int defwCanPlace(const char* master, int xOrig, int yOrig,
	int orient, int doCnt, int doInc, int xStep, int yStep) {
  defwFunc = DEFW_CANPLACE;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState > DEFW_CANPLACE) return DEFW_BAD_ORDER;

  if (defwState == DEFW_ROW)
     fprintf(defwFile, ";\n\n");  // add the ; and \n for the previous row.

  if (! master && ! *master) // required
     return DEFW_BAD_DATA;
  fprintf(defwFile, "CANPLACE %s %d %d %s DO %d BY %d STEP %d %d ;\n",
          master, xOrig, yOrig, defwOrient(orient),
          doCnt, doInc, xStep, yStep);
  defwLines++;

  defwState = DEFW_CANPLACE;
  return DEFW_OK;
}


int defwCanPlaceStr(const char* master, int xOrig, int yOrig,
	const char* orient, int doCnt, int doInc, int xStep, int yStep) {
  defwFunc = DEFW_CANPLACE;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState > DEFW_CANPLACE) return DEFW_BAD_ORDER;

  if (defwState == DEFW_ROW)
     fprintf(defwFile, ";\n\n");  // add the ; and \n for the previous row.

  if (! master && ! *master) // required
     return DEFW_BAD_DATA;
  fprintf(defwFile, "CANPLACE %s %d %d %s DO %d BY %d STEP %d %d ;\n",
          master, xOrig, yOrig, orient,
          doCnt, doInc, xStep, yStep);
  defwLines++;

  defwState = DEFW_CANPLACE;
  return DEFW_OK;
}


int defwCannotOccupy(const char* master, int xOrig, int yOrig,
	int orient, int doCnt, int doInc, int xStep, int yStep) {
  defwFunc = DEFW_CANNOTOCCUPY;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState > DEFW_CANNOTOCCUPY) return DEFW_BAD_ORDER;

  if (! master && ! *master) // required
     return DEFW_BAD_DATA;
  fprintf(defwFile, "CANNOTOCCUPY %s %d %d %s DO %d BY %d STEP %d %d ;\n",
          master, xOrig, yOrig, defwOrient(orient),
          doCnt, doInc, xStep, yStep);
  defwLines++;

  defwState = DEFW_CANNOTOCCUPY;
  return DEFW_OK;
}


int defwCannotOccupyStr(const char* master, int xOrig, int yOrig,
	const char* orient, int doCnt, int doInc, int xStep, int yStep) {
  defwFunc = DEFW_CANNOTOCCUPY;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState > DEFW_CANNOTOCCUPY) return DEFW_BAD_ORDER;

  if (! master && ! *master) // required
     return DEFW_BAD_DATA;
  fprintf(defwFile, "CANNOTOCCUPY %s %d %d %s DO %d BY %d STEP %d %d ;\n",
          master, xOrig, yOrig, orient,
          doCnt, doInc, xStep, yStep);
  defwLines++;

  defwState = DEFW_CANNOTOCCUPY;
  return DEFW_OK;
}


int defwStartVias(int count) {
  defwFunc = DEFW_VIA_START;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_VIA_START) return DEFW_BAD_ORDER;

  if (defwState == DEFW_ROW)
     fprintf(defwFile, ";\n\n");  // add the ; and \n for the previous row.

  fprintf(defwFile, "VIAS %d ;\n", count);
  defwLines++;

  defwCounter = count;
  defwState = DEFW_VIA_START;
  return DEFW_OK;
}


int defwViaName(const char* name) {
  defwFunc = DEFW_VIA;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_VIA_START &&
      defwState != DEFW_VIAONE_END) return DEFW_BAD_ORDER;
  defwCounter--;

  if (! name || ! *name) // required
     return DEFW_BAD_DATA;
  fprintf(defwFile, "   - %s", name);

  defwState = DEFW_VIA;
  return DEFW_OK;
}


int defwViaPattern(const char* pattern) {
  defwFunc = DEFW_VIA;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_VIA) return DEFW_BAD_ORDER;  // after defwViaName

  if (! pattern && ! *pattern) // required
     return DEFW_BAD_DATA;
  fprintf(defwFile, " + PATTERNNAME %s\n", pattern);

  defwState = DEFW_VIA;
  return DEFW_OK;
}


int defwViaRect(const char* layerNames, int xl, int yl, int xh, int yh) {
  defwFunc = DEFW_VIA;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_VIA) return DEFW_BAD_ORDER;

  if (! layerNames && ! *layerNames) // required
     return DEFW_BAD_DATA;
  fprintf(defwFile, "\n      + RECT %s ( %d %d ) ( %d %d )", layerNames,
          xl, yl, xh, yh);
  defwLines++;

  defwState = DEFW_VIA;
  return DEFW_OK;
}


int defwOneViaEnd() {
  defwFunc = DEFW_VIA;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_VIA) return DEFW_BAD_ORDER;

  fprintf(defwFile, " ;\n");
  defwLines++;

  defwState = DEFW_VIAONE_END;
  return DEFW_OK;
}


int defwEndVias() {
  defwFunc = DEFW_VIA_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_VIA_START &&
      defwState != DEFW_VIAONE_END) return DEFW_BAD_ORDER;
  if (defwCounter) return DEFW_BAD_DATA;

  fprintf(defwFile, "END VIAS\n\n");
  defwLines++;

  defwState = DEFW_VIA_END;
  return DEFW_OK;
}


int defwStartRegions(int count) {
  defwFunc = DEFW_REGION_START;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_REGION_START) return DEFW_BAD_ORDER;

  if (defwState == DEFW_ROW)
      fprintf(defwFile, ";\n\n");  // add the ; and \n for the previous row.

  fprintf(defwFile, "REGIONS %d ;\n", count);
  defwLines++;

  defwCounter = count;
  defwState = DEFW_REGION_START;
  return DEFW_OK;
}


int defwRegionName(const char* name) {
  defwFunc = DEFW_REGION;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_REGION_START &&
      defwState != DEFW_REGION) return DEFW_BAD_ORDER;
  defwCounter--;

  if (defwState == DEFW_REGION)
     fprintf(defwFile, ";\n");  // add the ; and \n for the previous row.

  if (! name || ! *name) // required
     return DEFW_BAD_DATA;
  fprintf(defwFile, "   - %s ", name);
  defwState = DEFW_REGION;
  return DEFW_OK;
}


int defwRegionPoints(int xl, int yl, int xh, int yh) {
  defwFunc = DEFW_REGION;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_REGION) return DEFW_BAD_ORDER;  // after RegionName

  fprintf(defwFile, "      ( %d %d ) ( %d %d ) ", xl, yl, xh, yh);

  defwState = DEFW_REGION;
  return DEFW_OK;
}


int defwRegionType(const char* type) {
  defwFunc = DEFW_REGION;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_REGION) return DEFW_BAD_ORDER;  // after RegionName

  if (! type && ! *type) // required
     return DEFW_BAD_DATA;
  if (strcmp(type, "FENCE") && strcmp(type, "GUIDE"))
     return DEFW_BAD_DATA;

  fprintf (defwFile, "         + TYPE %s", type);

  defwState = DEFW_REGION;
  return DEFW_OK;
}


int defwEndRegions() {
  defwFunc = DEFW_REGION_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_REGION_START &&
      defwState != DEFW_REGION) return DEFW_BAD_ORDER;
  if (defwCounter) return DEFW_BAD_DATA;

  fprintf(defwFile, ";\nEND REGIONS\n\n");  // ; for the previous statement
  defwLines++;

  defwState = DEFW_REGION_END;
  return DEFW_OK;
}


int defwStartComponents(int count) {
  defwFunc = DEFW_COMPONENT_START;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_COMPONENT_START) return DEFW_BAD_ORDER;

  if (defwState == DEFW_ROW)
     fprintf(defwFile, ";\n\n");  // add the ; and \n for the previous row.

  fprintf(defwFile, "COMPONENTS %d ;\n", count);
  defwLines++;

  defwCounter = count;
  defwState = DEFW_COMPONENT_START;
  return DEFW_OK;
}


int defwComponent(const char* instance, const char* master,
              int numNetName, const char** netNames, const char* eeq,
              const char* genName, const char* genParemeters,
              const char* source, int numForeign, const char** foreigns,
              int* foreignX, int* foreignY, int* foreignOrients,
              const char* status, int statusX, int statusY,
              int statusOrient, double weight, const char* region,
              int xl, int yl, int xh, int yh) {

  int i;

  defwFunc = DEFW_COMPONENT;   // Current function of writer

  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_COMPONENT_START &&
      defwState != DEFW_COMPONENT) return DEFW_BAD_ORDER;

  defwCounter--;

  if ((! instance && ! *instance) || (! master && ! *master)) // required
     return DEFW_BAD_DATA;

  if (source && strcmp(source, "NETLIST") && strcmp(source, "DIST") &&
     strcmp(source, "USER") && strcmp(source, "TIMING"))
     return DEFW_BAD_DATA;

  if (status && strcmp(status, "COVER") && strcmp(status, "FIXED") &&
     strcmp(status, "PLACED") && strcmp(status, "UNPLACED"))
     return DEFW_BAD_DATA;

  // only either region or xl, yl, xh, yh
  if (region && (xl || yl || xh || yh))
     return DEFW_BAD_DATA;

  if (defwState == DEFW_COMPONENT)
     fprintf(defwFile, ";\n");       // newline for the previous component

  fprintf(defwFile, "   - %s %s ", instance, master);
  if (numNetName) {
     for (i = 0; i < numNetName; i++)
        fprintf(defwFile, "%s ", netNames[i]);
  }
  defwLines++;
  // since the rest is optionals, new line is placed before the options
  if (eeq) {
     fprintf(defwFile, "\n      + EEQMASTER %s ", eeq);
     defwLines++;
  }
  if (genName) {
     fprintf(defwFile, "\n      + GENERATE %s ", genName);
     if (genParemeters)
        fprintf(defwFile, " %s ", genParemeters);
     defwLines++;
  }
  if (source) {
     fprintf(defwFile, "\n      + SOURCE %s ", source);
     defwLines++;
  }
  if (numForeign) {
     for (i = 0; i < numForeign; i++) {
        fprintf(defwFile, "\n      + FOREIGN %s ( %d %d ) %s ", foreigns[i],
                foreignX[i], foreignY[i], defwOrient(foreignOrients[i]));
        defwLines++;
     }
  }
  if (status) {
     fprintf(defwFile,"\n      + %s ( %d %d ) %s ", status, statusX, statusY,
             defwOrient(statusOrient));
     defwLines++;
  }
  if (weight) {
     fprintf(defwFile, "\n      + WEIGHT %g ", weight);
     defwLines++;
  }
  if (region) {
     fprintf(defwFile, "\n      + REGION %s ", region);
     defwLines++;
  } else if (xl || yl || xh || yh) {
     fprintf(defwFile, "\n      + REGION ( %d %d ) ( %d %d ) ",
             xl, yl, xh, yh);
     defwLines++;
  }

  defwState = DEFW_COMPONENT;
  return DEFW_OK;
}


int defwComponentStr(const char* instance, const char* master,
              int numNetName, const char** netNames, const char* eeq,
              const char* genName, const char* genParemeters,
              const char* source, int numForeign, const char** foreigns,
              int* foreignX, int* foreignY, const char** foreignOrients,
              const char* status, int statusX, int statusY,
              const char* statusOrient, double weight, const char* region,
              int xl, int yl, int xh, int yh) {

  int i;

  defwFunc = DEFW_COMPONENT;   // Current function of writer

  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_COMPONENT_START &&
      defwState != DEFW_COMPONENT) return DEFW_BAD_ORDER;

  defwCounter--;

  if ((! instance && ! *instance) || (! master && ! *master)) // required
     return DEFW_BAD_DATA;

  if (source && strcmp(source, "NETLIST") && strcmp(source, "DIST") &&
     strcmp(source, "USER") && strcmp(source, "TIMING"))
     return DEFW_BAD_DATA;

  if (status && strcmp(status, "COVER") && strcmp(status, "FIXED") &&
     strcmp(status, "PLACED") && strcmp(status, "UNPLACED"))
     return DEFW_BAD_DATA;

  // only either region or xl, yl, xh, yh
  if (region && (xl || yl || xh || yh))
     return DEFW_BAD_DATA;

  if (defwState == DEFW_COMPONENT)
     fprintf(defwFile, ";\n");       // newline for the previous component

  fprintf(defwFile, "   - %s %s ", instance, master);
  if (numNetName) {
     for (i = 0; i < numNetName; i++)
        fprintf(defwFile, "%s ", netNames[i]);
  }
  defwLines++;
  // since the rest is optionals, new line is placed before the options
  if (eeq) {
     fprintf(defwFile, "\n      + EEQMASTER %s ", eeq);
     defwLines++;
  }
  if (genName) {
     fprintf(defwFile, "\n      + GENERATE %s ", genName);
     if (genParemeters)
        fprintf(defwFile, " %s ", genParemeters);
     defwLines++;
  }
  if (source) {
     fprintf(defwFile, "\n      + SOURCE %s ", source);
     defwLines++;
  }
  if (numForeign) {
     for (i = 0; i < numForeign; i++) {
        fprintf(defwFile, "\n      + FOREIGN %s ( %d %d ) %s ", foreigns[i],
                foreignX[i], foreignY[i], foreignOrients[i]);
        defwLines++;
     }
  }
  if (status) {
     fprintf(defwFile,"\n      + %s ( %d %d ) %s ", status, statusX, statusY,
             statusOrient);
     defwLines++;
  }
  if (weight) {
     fprintf(defwFile, "\n      + WEIGHT %g ", weight);
     defwLines++;
  }
  if (region) {
     fprintf(defwFile, "\n      + REGION %s ", region);
     defwLines++;
  } else if (xl || yl || xh || yh) {
     fprintf(defwFile, "\n      + REGION ( %d %d ) ( %d %d ) ",
             xl, yl, xh, yh);
     defwLines++;
  }

  defwState = DEFW_COMPONENT;
  return DEFW_OK;
}


int defwEndComponents() {
  defwFunc = DEFW_COMPONENT_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_COMPONENT_START &&
      defwState != DEFW_COMPONENT) return DEFW_BAD_ORDER;
  if (defwCounter) return DEFW_BAD_DATA;

  defwDidComponents = 1;

  fprintf(defwFile, ";\nEND COMPONENTS\n\n");
  defwLines++;

  defwState = DEFW_COMPONENT_END;
  return DEFW_OK;
}


int defwStartPins(int count) {
  defwFunc = DEFW_PIN_START;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidComponents) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_PIN_START) return DEFW_BAD_ORDER;

  fprintf(defwFile, "PINS %d", count);
  defwLines++;

  defwCounter = count;
  defwState = DEFW_PIN_START;
  return DEFW_OK;
}


int defwPin(const char* name, const char* net,
       int special,       /* optional 0-ignore 1-special */
       const char* direction,                            /* optional */
       const char* use,                                  /* optional */
       const char* status, int xo, int yo, int orient,   /* optional */
       const char* layer, int xl, int yl, int xh, int yh /* optional */
       ) {

  defwFunc = DEFW_PIN;   // Current function of writer

  defwPinLayer = 0;      // Reset the global variable to 0 for pin layer

  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PIN_START && defwState != DEFW_PIN &&
      defwState != DEFW_PIN_PORT) return DEFW_BAD_ORDER;

  defwCounter--;

  fprintf(defwFile, " ;\n   - %s + NET %s", name, net);

  if (special)
     fprintf(defwFile, "\n      + SPECIAL");
  if (direction) {
     if (strcmp(direction, "INPUT") && strcmp(direction, "OUTPUT") &&
         strcmp(direction, "INOUT") && strcmp(direction, "FEEDTHRU"))
        return DEFW_BAD_DATA;
     fprintf(defwFile, "\n      + DIRECTION %s", direction);
  }
  if (use) {
     if (strcmp(use, "SIGNAL") && strcmp(use, "POWER") &&
         strcmp(use, "GROUND") && strcmp(use, "CLOCK") &&
         strcmp(use, "TIEOFF") && strcmp(use, "ANALOG") &&
         strcmp(use, "SCAN") && strcmp(use, "RESET"))
        return DEFW_BAD_DATA;
     fprintf(defwFile, "\n      + USE %s", use);
  }
  if (status) {
     if (strcmp(status, "FIXED") && strcmp(status, "PLACED") &&
         strcmp(status, "COVER"))
        return DEFW_BAD_DATA;
  
     fprintf(defwFile,"\n      + %s ( %d %d ) %s", status, xo, yo,
             defwOrient(orient));
  }
  if (layer) {
     fprintf(defwFile, "\n      + LAYER %s ( %d %d ) ( %d %d )",
             layer, xl, yl, xh, yh);
     defwPinLayer = 1;     /* set that layer is defined in the pin */
  }

  defwLines++;

  defwState = DEFW_PIN;
  return DEFW_OK;
}


int defwPinStr(const char* name, const char* net,
       int special,       /* optional 0-ignore 1-special */
       const char* direction,                                    /* optional */
       const char* use,                                          /* optional */
       const char* status, int xo, int yo, const char* orient,   /* optional */
       const char* layer, int xl, int yl, int xh, int yh /* optional */
       ) {

  defwFunc = DEFW_PIN;   // Current function of writer

  defwPinLayer = 0;      // Reset the global variable to 0 for pin layer

  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PIN_START && defwState != DEFW_PIN &&
      defwState != DEFW_PIN_PORT) return DEFW_BAD_ORDER;

  defwCounter--;

  fprintf(defwFile, " ;\n   - %s + NET %s", name, net);

  if (special)
     fprintf(defwFile, "\n      + SPECIAL");
  if (direction) {
     if (strcmp(direction, "INPUT") && strcmp(direction, "OUTPUT") &&
         strcmp(direction, "INOUT") && strcmp(direction, "FEEDTHRU"))
        return DEFW_BAD_DATA;
     fprintf(defwFile, "\n      + DIRECTION %s", direction);
  }
  if (use) {
     if (strcmp(use, "SIGNAL") && strcmp(use, "POWER") &&
         strcmp(use, "GROUND") && strcmp(use, "CLOCK") &&
         strcmp(use, "TIEOFF") && strcmp(use, "ANALOG") &&
         strcmp(use, "SCAN") && strcmp(use, "RESET"))
        return DEFW_BAD_DATA;
     fprintf(defwFile, "\n      + USE %s", use);
  }
  if (status) {
     if (strcmp(status, "FIXED") && strcmp(status, "PLACED") &&
         strcmp(status, "COVER"))
        return DEFW_BAD_DATA;
  
     fprintf(defwFile,"\n      + %s ( %d %d ) %s", status, xo, yo,
             orient);
  }
  if (layer) {
     fprintf(defwFile, "\n      + LAYER %s ( %d %d ) ( %d %d )",
             layer, xl, yl, xh, yh);
     defwPinLayer = 1;     /* set that layer is defined in the pin */
  }

  defwLines++;

  defwState = DEFW_PIN;
  return DEFW_OK;
}


int defwPinAntennaPinPartialMetalArea(int value, const char* layerName) {
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PIN && defwState != DEFW_PIN_PORT)
      return DEFW_BAD_ORDER;
  if (defVersionNum < 5.4)
      return DEFW_WRONG_VERSION;

  fprintf(defwFile, "\n      + ANTENNAPINPARTIALMETALAREA %d", value);
  if (layerName)
     fprintf(defwFile, " LAYER %s", layerName);
  defwLines++;

  return DEFW_OK;
}


int defwPinAntennaPinPartialMetalSideArea(int value, const char* layerName) {
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PIN && defwState != DEFW_PIN_PORT)
      return DEFW_BAD_ORDER;
  if (defVersionNum < 5.4)
      return DEFW_WRONG_VERSION;

  fprintf(defwFile, "\n      + ANTENNAPINPARTIALMETALSIDEAREA %d", value);
  if (layerName)
     fprintf(defwFile, " LAYER %s", layerName);
  defwLines++;

  return DEFW_OK;
}


int defwPinAntennaPinPartialCutArea(int value, const char* layerName) {
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PIN && defwState != DEFW_PIN_PORT)
      return DEFW_BAD_ORDER;
  if (defVersionNum < 5.4)
      return DEFW_WRONG_VERSION;

  fprintf(defwFile, "\n      + ANTENNAPINPARTIALCUTAREA %d", value);
  if (layerName)
     fprintf(defwFile, " LAYER %s", layerName);
  defwLines++;

  return DEFW_OK;
}


int defwPinAntennaModel(const char* oxide) {
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PIN && defwState != DEFW_PIN_PORT)
      return DEFW_BAD_ORDER;
  if (defVersionNum < 5.4)
      return DEFW_WRONG_VERSION;

  fprintf(defwFile, "\n      + ANTENNAMODEL %s", oxide);
  defwLines++;

  return DEFW_OK;
}


int defwPinAntennaPinDiffArea(int value, const char* layerName) {
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PIN && defwState != DEFW_PIN_PORT)
      return DEFW_BAD_ORDER;
  if (defVersionNum < 5.4)
      return DEFW_WRONG_VERSION;

  fprintf(defwFile, "\n      + ANTENNAPINDIFFAREA %d", value);
  if (layerName)
     fprintf(defwFile, " LAYER %s", layerName);
  defwLines++;

  return DEFW_OK;
}


int defwPinAntennaPinGateArea(int value, const char* layerName) {
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PIN && defwState != DEFW_PIN_PORT)
      return DEFW_BAD_ORDER;
  if (defVersionNum < 5.4)
      return DEFW_WRONG_VERSION;

  fprintf(defwFile, "\n      + ANTENNAPINGATEAREA %d", value);
  if (layerName)
     fprintf(defwFile, " LAYER %s", layerName);
  defwLines++;

  return DEFW_OK;
}


int defwPinAntennaPinMaxAreaCar(int value, const char* layerName) {
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PIN && defwState != DEFW_PIN_PORT)
      return DEFW_BAD_ORDER;
  if (defVersionNum < 5.4)
      return DEFW_WRONG_VERSION;

  fprintf(defwFile, "\n      + ANTENNAPINMAXAREACAR %d", value);
  if (!layerName) return DEFW_BAD_DATA;  /* layerName is required */

  fprintf(defwFile, " LAYER %s", layerName);
  defwLines++;

  return DEFW_OK;
}


int defwPinAntennaPinMaxSideAreaCar(int value, const char* layerName) {
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PIN && defwState != DEFW_PIN_PORT)
      return DEFW_BAD_ORDER;
  if (defVersionNum < 5.4)
      return DEFW_WRONG_VERSION;

  fprintf(defwFile, "\n      + ANTENNAPINMAXSIDEAREACAR %d", value);
  if (!layerName) return DEFW_BAD_DATA;  /* layerName is required */

  fprintf(defwFile, " LAYER %s", layerName);
  defwLines++;

  return DEFW_OK;
}


int defwPinAntennaPinMaxCutCar(int value, const char* layerName) {
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PIN && defwState != DEFW_PIN_PORT)
      return DEFW_BAD_ORDER;
  if (defVersionNum < 5.4)
      return DEFW_WRONG_VERSION;

  fprintf(defwFile, "\n      + ANTENNAPINMAXCUTCAR %d", value);
  if (!layerName) return DEFW_BAD_DATA;

  fprintf(defwFile, " LAYER %s", layerName);
  defwLines++;

  return DEFW_OK;
}


int defwEndPins() {
  defwFunc = DEFW_PIN_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PIN_START && defwState != DEFW_PIN &&
      defwState != DEFW_PIN_PORT) return DEFW_BAD_ORDER;
  if (defwCounter) return DEFW_BAD_DATA;

  fprintf(defwFile, " ;\nEND PINS\n\n");
  defwLines++;

  defwState = DEFW_PIN_END;
  return DEFW_OK;
}


int defwStartPinProperties(int count) {
  defwFunc = DEFW_PINPROP_START;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidComponents) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_PINPROP_START) return DEFW_BAD_ORDER;

  fprintf(defwFile, "PINPROPERTIES %d ;\n", count);
  defwLines++;

  defwCounter = count;
  defwState = DEFW_PINPROP_START;
  return DEFW_OK;
}


int defwPinProperty(const char* name, const char* pinName) {

  defwFunc = DEFW_PINPROP;   // Current function of writer

  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PINPROP_START &&
      defwState != DEFW_PINPROP) return DEFW_BAD_ORDER;

  defwCounter--;
  if ((! name || ! *name) || (! pinName || ! *pinName)) // required
     return DEFW_BAD_DATA;

  if (defwState == DEFW_PINPROP)
     fprintf(defwFile, ";\n");

  fprintf(defwFile, "   - %s %s ", name, pinName);
  defwLines++;

  defwState = DEFW_PINPROP;
  return DEFW_OK;
}


int defwEndPinProperties() {
  defwFunc = DEFW_PIN_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PINPROP_START &&
      defwState != DEFW_PINPROP) return DEFW_BAD_ORDER;
  if (defwCounter) return DEFW_BAD_DATA;

  fprintf(defwFile, ";\nEND PINPROPERTIES\n\n");
  defwLines++;

  defwState = DEFW_PINPROP_END;
  return DEFW_OK;
}


int defwStartSpecialNets(int count) {
  defwFunc = DEFW_SNET_START;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidComponents) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_SNET_START) return DEFW_BAD_ORDER;

  fprintf(defwFile, "SPECIALNETS %d ;\n", count);
  defwLines++;

  defwCounter = count;
  defwState = DEFW_SNET_START;
  return DEFW_OK;
}


int defwSpecialNetOptions() {
  defwFunc = DEFW_SNET;   // Current function of writer
  if (defwState == DEFW_SNET) { defwState = DEFW_SNET_OPTIONS; return 1; }
  if (defwState == DEFW_SNET_OPTIONS) return 1;
  return 0;
}


int defwSpecialNet(const char* name) {
  defwFunc = DEFW_SNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SNET_START &&
      defwState != DEFW_SNET_ENDNET) return DEFW_BAD_ORDER;
  defwState = DEFW_SNET;

  fprintf(defwFile, "   - %s", name);
  defwLineItemCounter = 0;
  defwCounter--;

  return DEFW_OK;
}


int defwSpecialNetConnection(const char* inst, const char* pin,
                             int synthesized) {
  defwFunc = DEFW_SNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SNET) return DEFW_BAD_ORDER;

  if ((++defwLineItemCounter & 3) == 0) { // since a net can have more than
     fprintf(defwFile, "\n     ");  // one inst pin connection, don't print
     defwLines++;             // newline until the line is certain length
   }
  fprintf(defwFile, " ( %s %s ) ", inst, pin);
  if (synthesized)
     fprintf(defwFile, " + SYNTHESIZED ");
  return DEFW_OK;
}


int defwSpecialNetFixedbump() {
  defwFunc = DEFW_SNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwSpecialNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + FIXEDBUMP");
  defwLines++;
  return DEFW_OK;
}

int defwSpecialNetVoltage(double d) {
  defwFunc = DEFW_SNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwSpecialNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + VOLTAGE %g", d);
  defwLines++;
  return DEFW_OK;
}


int defwSpecialNetSpacing(const char* layer, int spacing, double minwidth,
                          double maxwidth) {
  defwFunc = DEFW_SNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwSpecialNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + SPACING %s %d", layer, spacing);
  if (minwidth || maxwidth)
     fprintf(defwFile, " RANGE %g %g", minwidth, maxwidth);
  defwLines++;
  return DEFW_OK;
}


int defwSpecialNetWidth(const char* layer, int w) {
  defwFunc = DEFW_SNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwSpecialNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + WIDTH %s %d", layer, w);
  defwLines++;
  return DEFW_OK;
}


int defwSpecialNetSource(const char* name) {
  defwFunc = DEFW_SNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwSpecialNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + SOURCE %s", name);
  defwLines++;
  return DEFW_OK;
}


int defwSpecialNetOriginal(const char* name) {
  defwFunc = DEFW_SNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwSpecialNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + ORIGINAL %s", name);
  defwLines++;
  return DEFW_OK;
}


int defwSpecialNetPattern(const char* name) {
  defwFunc = DEFW_SNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwSpecialNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + PATTERN %s", name);
  defwLines++;
  return DEFW_OK;
}


int defwSpecialNetUse(const char* name) {
  defwFunc = DEFW_SNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwSpecialNetOptions()) return DEFW_BAD_ORDER;

  if (strcmp(name, "SIGNAL") && strcmp(name, "POWER") &&
      strcmp(name, "GROUND") && strcmp(name, "CLOCK") &&
      strcmp(name, "TIEOFF") && strcmp(name, "ANALOG") &&
      strcmp(name, "SCAN") && strcmp(name, "RESET"))
     return DEFW_BAD_DATA;

  fprintf(defwFile, "\n      + USE %s", name);
  defwLines++;
  return DEFW_OK;
}


int defwSpecialNetWeight(double d) {
  defwFunc = DEFW_SNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwSpecialNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + WEIGHT %g", d);
  defwLines++;
  return DEFW_OK;
}


int defwSpecialNetEstCap(double d) {
  defwFunc = DEFW_SNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwSpecialNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + ESTCAP %g", d);
  defwLines++;
  return DEFW_OK;
}


int defwSpecialNetNumberProperty(const char* name, double num) {
  defwFunc = DEFW_SNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwSpecialNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + PROPERTY %s %g", name, num);
  defwLines++;
  return DEFW_OK;
}


int defwSpecialNetStringProperty(const char* name, const char* value) {
  defwFunc = DEFW_SNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwSpecialNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + PROPERTY %s %s", name, value);
  defwLines++;
  return DEFW_OK;
}


int defwSpecialNetPathStart(const char* typ) {
  defwFunc = DEFW_PATH;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwSpecialNetOptions() &&
       (defwState != DEFW_SUBNET) && /* path in subnet */
       (defwState != DEFW_PATH))   /* NEW in the path, path hasn't end yet */
      return DEFW_BAD_ORDER;

  if (strcmp(typ, "NEW") && strcmp(typ, "FIXED") && strcmp(typ, "COVER") &&
      strcmp(typ, "ROUTED") && strcmp(typ, "SHIELD")) return DEFW_BAD_DATA;

  defwSpNetShield = 0;

  // The second time around for a path on this net, we
  // must start it with a new instead of a fixed...
  if (strcmp(typ, "NEW") == 0) {
     if (defwState != DEFW_PATH) return DEFW_BAD_DATA;
     fprintf(defwFile, " NEW");
  } else if (strcmp(typ, "SHIELD") == 0) { 
     fprintf(defwFile, "\n      + %s", typ);
     defwSpNetShield = 1;
  } else
     fprintf(defwFile, "\n      + %s", typ);

  defwState = DEFW_PATH;
  defwLineItemCounter = 0;
  return DEFW_OK;
}


int defwSpecialNetShieldNetName(const char* name) {
  defwFunc = DEFW_PATH;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PATH) return DEFW_BAD_ORDER;
  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n     ");
     defwLines++;
  }
  if (defwSpNetShield)
     fprintf(defwFile, " %s", name);
  else
     return DEFW_BAD_ORDER;
  return DEFW_OK;
}


int defwSpecialNetPathWidth(int w) {
  defwFunc = DEFW_PATH;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PATH) return DEFW_BAD_ORDER;
  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n     ");
     defwLines++;
  }
  fprintf(defwFile, " %d", w);
  return DEFW_OK;
}


int defwSpecialNetPathLayer(const char* name) {
  defwFunc = DEFW_PATH;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PATH) return DEFW_BAD_ORDER;
  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n     ");
     defwLines++;
  }
  fprintf(defwFile, " %s", name);
  return DEFW_OK;
}


int defwSpecialNetPathShape(const char* typ) {
  defwFunc = DEFW_PATH;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PATH) return DEFW_BAD_ORDER;

  if (strcmp(typ, "RING") && strcmp(typ, "STRIP") && strcmp(typ, "FOLLOWPIN") &&
      strcmp(typ, "IOWIRE") && strcmp(typ, "COREWIRE") &&
      strcmp(typ, "BLOCKWIRE") && strcmp(typ, "FILLWIRE") &&
      strcmp(typ, "BLOCKAGEWIRE") && strcmp(typ, "PADRING") &&
      strcmp(typ, "BLOCKRING") && strcmp(typ, "DRCFILL"))
     return DEFW_BAD_DATA;

  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n     ");
     defwLines++;
  }
  fprintf(defwFile, "\n      + SHAPE %s", typ);

  defwState = DEFW_PATH;
  defwLineItemCounter = 0;
  return DEFW_OK;
}


int defwSpecialNetPathPoint(int numPts, const char** pointx,
                            const char** pointy) {
  int i;

  defwFunc = DEFW_PATH;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PATH) return DEFW_BAD_ORDER;
  for (i = 0; i < numPts; i++) {
     if ((++defwLineItemCounter & 3) == 0) {
        fprintf(defwFile, "\n     ");
        defwLines++;
     }
     fprintf(defwFile, " ( %s %s )", pointx[i], pointy[i]);
  }
  return DEFW_OK;
}


int defwSpecialNetPathVia(const char* name) {
  defwFunc = DEFW_PATH;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PATH) return DEFW_BAD_ORDER;
  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n     ");
     defwLines++;
  }
  fprintf(defwFile, " %s", name);
  return DEFW_OK;
}


int defwSpecialNetPathViaData(int numX, int numY, int stepX, int stepY) {
  defwFunc = DEFW_PATH;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PATH) return DEFW_BAD_ORDER;
  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n     ");
     defwLines++;
  }
  fprintf(defwFile, " DO %d BY %d STEP %d %d", numX, numY, stepX, stepY);
  return DEFW_OK;
}


int defwSpecialNetPathEnd() {
  defwFunc = DEFW_SNET_OPTIONS;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PATH) return DEFW_BAD_ORDER;
  defwState = DEFW_SNET_OPTIONS;
  return DEFW_OK;
}


int defwSpecialNetShieldStart(const char* name) {
  defwFunc = DEFW_SHIELD;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwSpecialNetOptions()) return DEFW_BAD_ORDER;

  // The second time around for a shield on this net, we
  // must start it with a new instead of the name ...
  if (strcmp(name, "NEW") == 0) {
    if (defwState != DEFW_SHIELD) return DEFW_BAD_DATA;
    fprintf(defwFile, " NEW");
  } else
       fprintf(defwFile, "\n      + SHIELD %s", name);

  defwState = DEFW_SHIELD;
  defwLineItemCounter = 0;
  return DEFW_OK;
}


int defwSpecialNetShieldWidth(int w) {
  defwFunc = DEFW_SHIELD;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SHIELD) return DEFW_BAD_ORDER;
  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n     ");
     defwLines++;
  }
  fprintf(defwFile, " %d", w);
  return DEFW_OK;
}


int defwSpecialNetShieldLayer(const char* name) {
  defwFunc = DEFW_SHIELD;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SHIELD) return DEFW_BAD_ORDER;
  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n     ");
     defwLines++;
  }
  fprintf(defwFile, " %s", name);
  return DEFW_OK;
}


int defwSpecialNetShieldShape(const char* typ) {
  defwFunc = DEFW_SHIELD;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SHIELD) return DEFW_BAD_ORDER;

  if (strcmp(typ, "RING") && strcmp(typ, "STRIP") && strcmp(typ, "FOLLOWPIN") &&
      strcmp(typ, "IOWIRE") && strcmp(typ, "COREWIRE") &&
      strcmp(typ, "BLOCKWIRE") && strcmp(typ, "FILLWIRE") &&
      strcmp(typ, "BLOCKAGEWIRE") && strcmp(typ, "DRCFILL"))
     return DEFW_BAD_DATA;

  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n     ");
     defwLines++;
  }
  fprintf(defwFile, "\n      + SHAPE %s", typ);

  defwState = DEFW_SHIELD;
  defwLineItemCounter = 0;
  return DEFW_OK;
}


int defwSpecialNetShieldPoint(int numPts, const char** pointx,
                            const char** pointy) {
  int i;

  defwFunc = DEFW_SHIELD;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SHIELD) return DEFW_BAD_ORDER;
  for (i = 0; i < numPts; i++) {
     if ((++defwLineItemCounter & 3) == 0) {
        fprintf(defwFile, "\n     ");
        defwLines++;
     }
     fprintf(defwFile, " ( %s %s )", pointx[i], pointy[i]);
  }
  return DEFW_OK;
}


int defwSpecialNetShieldVia(const char* name) {
  defwFunc = DEFW_SHIELD;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SHIELD) return DEFW_BAD_ORDER;
  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n     ");
     defwLines++;
  }
  fprintf(defwFile, " %s", name);
  return DEFW_OK;
}


int defwSpecialNetShieldViaData(int numX, int numY, int stepX, int stepY) {
  defwFunc = DEFW_SHIELD;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SHIELD) return DEFW_BAD_ORDER;
  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n     ");
     defwLines++;
  }
  fprintf(defwFile, " DO %d BY %d STEP %d %d", numX, numY, stepX, stepY);
  return DEFW_OK;
}

int defwSpecialNetShieldEnd() {
  defwFunc = DEFW_SNET_OPTIONS;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SHIELD) return DEFW_BAD_ORDER;
  defwState = DEFW_SNET_OPTIONS;
  return DEFW_OK;
}


int defwSpecialNetEndOneNet() {
  defwFunc = DEFW_SNET_ENDNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwSpecialNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, " ;\n");
  defwLines++;
  defwState = DEFW_SNET_ENDNET;

  return DEFW_OK;
}


int defwEndSpecialNets() {
  defwFunc = DEFW_SNET_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SNET_START && defwState != DEFW_SNET_OPTIONS &&
      defwState != DEFW_SNET_ENDNET &&   // last state is special net
      defwState != DEFW_SNET) return DEFW_BAD_ORDER;
  if (defwCounter) return DEFW_BAD_DATA;

  fprintf(defwFile, "END SPECIALNETS\n\n");
  defwLines++;

  defwState = DEFW_SNET_END;
  return DEFW_OK;
}


int defwStartNets(int count) {
  defwFunc = DEFW_NET_START;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidComponents) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_NET_START) return DEFW_BAD_ORDER;

  fprintf(defwFile, "NETS %d ;\n", count);
  defwLines++;

  defwCounter = count;
  defwState = DEFW_NET_START;
  return DEFW_OK;
}


int defwNetOptions() {
  defwFunc = DEFW_NET;   // Current function of writer
  if (defwState == DEFW_NET) { defwState = DEFW_NET_OPTIONS; return 1; }
  if (defwState == DEFW_NET_OPTIONS) return 1;
  return 0;
}


int defwNet(const char* name) {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_NET_START &&
      defwState != DEFW_NET_ENDNET) return DEFW_BAD_ORDER;
  defwState = DEFW_NET;

  fprintf(defwFile, "   - %s", name);
  defwLineItemCounter = 0;
  defwCounter--;

  return DEFW_OK;
}


int defwNetConnection(const char* inst, const char* pin, int synthesized) {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_NET) return DEFW_BAD_ORDER;

  if ((++defwLineItemCounter & 3) == 0) {  // since there is more than one
     fprintf(defwFile, "\n"); // inst & pin connection, don't print newline
     defwLines++;             // until the line is certain length long
  }
  fprintf(defwFile, " ( %s %s", inst, pin);
  if (synthesized)
     fprintf(defwFile, " + SYNTHESIZED ) ");
  else
     fprintf(defwFile, " ) ");
  return DEFW_OK;
}


int defwNetMustjoinConnection(const char* inst, const char* pin) {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_NET) return DEFW_BAD_ORDER;

  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n     ");
     defwLines++;
   }
  fprintf(defwFile, " MUSTJOIN ( %s %s )", inst, pin);

  return DEFW_OK;
}


int defwNetFixedbump() {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + FIXEDBUMP");
  defwLines++;
  return DEFW_OK;
}


int defwNetFrequency(double frequency) {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + FREQUENCY %g", frequency);
  defwLines++;
  return DEFW_OK;
}


int defwNetSource(const char* name) {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + SOURCE %s", name);
  defwLines++;
  return DEFW_OK;
}


int defwNetXtalk(int xtalk) {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + XTALK %d", xtalk);
  defwLines++;
  return DEFW_OK;
}


int defwNetVpin(const char* vpinName, const char* layerName, int layerXl,
                int layerYl, int layerXh, int layerYh, const char* status,
                int statusX, int statusY, int orient) {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions()) return DEFW_BAD_ORDER;
  if (! vpinName && ! *vpinName) // required
     return DEFW_BAD_DATA;

  if (status && strcmp(status, "PLACED") && strcmp(status, "FIXED") &&
      strcmp(status, "COVER"))
     return DEFW_BAD_DATA;
  if (status && (orient == 1))  // require if status is set
     return DEFW_BAD_DATA;

  fprintf(defwFile, "\n      + VPIN %s", vpinName);
  if (layerName)
     fprintf(defwFile, " LAYER %s", layerName);
  fprintf(defwFile, " ( %d %d ) ( %d %d )\n", layerXl, layerYl, layerXh,
          layerYh);
  defwLines++;

  if (status)
     fprintf(defwFile, "         %s ( %d %d ) %s", status, statusX, statusY,
             defwOrient(orient));
  defwLines++;
  return DEFW_OK;
}


int defwNetVpinStr(const char* vpinName, const char* layerName, int layerXl,
                int layerYl, int layerXh, int layerYh, const char* status,
                int statusX, int statusY, const char* orient) {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions()) return DEFW_BAD_ORDER;
  if (! vpinName && ! *vpinName) // required
     return DEFW_BAD_DATA;

  if (status && strcmp(status, "PLACED") && strcmp(status, "FIXED") &&
      strcmp(status, "COVER"))
     return DEFW_BAD_DATA;
  if (status && orient && *orient == '\0')  // require if status is set
     return DEFW_BAD_DATA;

  fprintf(defwFile, "\n      + VPIN %s", vpinName);
  if (layerName)
     fprintf(defwFile, " LAYER %s", layerName);
  fprintf(defwFile, " ( %d %d ) ( %d %d )\n", layerXl, layerYl, layerXh,
          layerYh);
  defwLines++;

  if (status)
     fprintf(defwFile, "         %s ( %d %d ) %s", status, statusX, statusY,
             orient);
  defwLines++;
  return DEFW_OK;
}


int defwNetOriginal(const char* name) {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + ORIGINAL %s", name);
  defwLines++;
  return DEFW_OK;
}


int defwNetPattern(const char* name) {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + PATTERN %s", name);
  defwLines++;
  return DEFW_OK;
}


int defwNetUse(const char* name) {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions()) return DEFW_BAD_ORDER;

  if (strcmp(name, "SIGNAL") && strcmp(name, "POWER") &&
      strcmp(name, "GROUND") && strcmp(name, "CLOCK") &&
      strcmp(name, "TIEOFF") && strcmp(name, "ANALOG") &&
      strcmp(name, "SCAN") && strcmp(name, "RESET"))
     return DEFW_BAD_DATA;

  fprintf(defwFile, "\n      + USE %s", name);
  defwLines++;
  return DEFW_OK;
}


int defwNetNondefaultRule(const char* name) {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if ((defwState != DEFW_NET) && (defwState != DEFW_NET_OPTIONS) &&
      (defwState != DEFW_SUBNET))
     return DEFW_BAD_ORDER;

  if (defwState == DEFW_SUBNET)
     fprintf(defwFile, "\n         NONDEFAULTRULE %s", name);
  else
     fprintf(defwFile, "\n      + NONDEFAULTRULE %s", name);
  defwLines++;
  return DEFW_OK;
}


int defwNetWeight(double d) {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + WEIGHT %g", d);
  defwLines++;
  return DEFW_OK;
}


int defwNetEstCap(double d) {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + ESTCAP %g", d);
  defwLines++;
  return DEFW_OK;
}


int defwNetShieldnet(const char* name) {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, "\n      + SHIELDNET %s", name);
  defwLines++;
  return DEFW_OK;
}


int defwNetNoshieldStart(const char* name) {
  defwFunc = DEFW_NOSHIELD;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions()) return DEFW_BAD_ORDER;
  fprintf(defwFile, "\n      + NOSHIELD %s", name);

  defwState = DEFW_NOSHIELD;
  defwLineItemCounter = 0;
  return DEFW_OK;
}


int defwNetNoshieldPoint(int numPts, const char** pointx,
                         const char** pointy) {
  int i;
 
  defwFunc = DEFW_NOSHIELD;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_NOSHIELD) return DEFW_BAD_ORDER;
  for (i = 0; i < numPts; i++) {
     if ((++defwLineItemCounter & 3) == 0) {
        fprintf(defwFile, "\n     ");
        defwLines++;
     }
     fprintf(defwFile, " ( %s %s )", pointx[i], pointy[i]);
  }
  return DEFW_OK;
}
 
 
int defwNetNoshieldVia(const char* name) {
  defwFunc = DEFW_NOSHIELD;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_NOSHIELD) return DEFW_BAD_ORDER;
  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n     ");
     defwLines++;
  }
  fprintf(defwFile, " %s", name);
  return DEFW_OK;
}


int defwNetNoshieldEnd() {
  defwFunc = DEFW_NET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_NOSHIELD) return DEFW_BAD_ORDER;
  defwState = DEFW_NET;
  return DEFW_OK;
}


int defwNetSubnetStart(const char* name) {
  defwFunc = DEFW_SUBNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions()) return DEFW_BAD_ORDER;
  if (! name || ! *name) // required
     return DEFW_BAD_DATA;

  fprintf(defwFile, "\n      + SUBNET %s", name);  
  defwLines++;
  defwState = DEFW_SUBNET;
  defwLineItemCounter = 0;
  return DEFW_OK;
}


int defwNetSubnetPin(const char* compName, const char*pinName) {
  defwFunc = DEFW_SUBNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SUBNET) return DEFW_BAD_ORDER;
  if ((! compName && ! *compName) || (! pinName && ! *pinName)) // required
     return DEFW_BAD_DATA;
  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n        ");
     defwLines++;
  }
  fprintf(defwFile, " ( %s %s )", compName, pinName);
  defwLines++;
  return DEFW_OK;
}


int defwNetSubnetEnd() {
  defwFunc = DEFW_SUBNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if ((defwState != DEFW_SUBNET) &&    /* subnet does not have path */
      (defwState != DEFW_NET_OPTIONS)) /* subnet has path and path just ended */
     return DEFW_BAD_ORDER;
  defwState = DEFW_NET_OPTIONS;
  return DEFW_OK;
}


int defwNetPathStart(const char* typ) {
  defwFunc = DEFW_PATH;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions() && (defwState != DEFW_SUBNET) && /* path in subnet */
     (defwState != DEFW_PATH))      /* NEW in the path, path hasn't end yet */
     return DEFW_BAD_ORDER;

  if (strcmp(typ, "NEW") && strcmp(typ, "FIXED") && strcmp(typ, "COVER") &&
      strcmp(typ, "ROUTED") && strcmp(typ, "NOSHIELD")) return DEFW_BAD_DATA;

  // The second time around for a path on this net, we
  // must start it with a new instead of a fixed...
  if (strcmp(typ, "NEW") == 0) {
    if (defwState != DEFW_PATH) return DEFW_BAD_DATA;
    fprintf(defwFile, "\n         NEW", typ);
  } else {
    if (defwState == DEFW_SUBNET)
       fprintf(defwFile, "\n      %s", typ);
    else
       fprintf(defwFile, "\n      + %s", typ);
  }

  defwState = DEFW_PATH;
  defwLineItemCounter = 0;
  return DEFW_OK;
}


int defwNetPathWidth(int w) {
  defwFunc = DEFW_PATH;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PATH) return DEFW_BAD_ORDER;
  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n         ");
     defwLines++;
  }
  fprintf(defwFile, " %d", w);
  return DEFW_OK;
}


int defwNetPathLayer(const char* name, int isTaper, const char* ruleName) {
  defwFunc = DEFW_PATH;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PATH) return DEFW_BAD_ORDER;

  // only one, either isTaper or ruleName can be set
  if (isTaper && ruleName)
     return DEFW_BAD_DATA;
  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n        ");
     defwLines++;
  }
  fprintf(defwFile, " %s", name);
  if (isTaper)
     fprintf(defwFile, " TAPER");
  else if (ruleName)
     fprintf(defwFile, " TAPERRULE %s", ruleName);
  return DEFW_OK;
}


int defwNetPathPoint(int numPts, const char** pointx, const char** pointy,
                     const char** value) {
  int i;
  defwFunc = DEFW_PATH;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PATH) return DEFW_BAD_ORDER;
  for (i = 0; i < numPts; i++) {
     if ((++defwLineItemCounter & 3) == 0) {
        fprintf(defwFile, "\n        ");
        defwLines++;
     }
     fprintf(defwFile, " ( %s %s ", pointx[i], pointy[i]);
     if (value[i])
        fprintf(defwFile, "%s ", value[i]);
     fprintf(defwFile, ")");
  }
  return DEFW_OK;
}


int defwNetPathVia(const char* name) {
  defwFunc = DEFW_PATH;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PATH) return DEFW_BAD_ORDER;
  if ((++defwLineItemCounter & 3) == 0) {
     fprintf(defwFile, "\n        ");
     defwLines++;
  }
  fprintf(defwFile, " %s", name);
  return DEFW_OK;
}


int defwNetPathEnd() {
  defwFunc = DEFW_PATH;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_PATH) return DEFW_BAD_ORDER;
  defwState = DEFW_NET_OPTIONS;
  return DEFW_OK;
}


int defwNetEndOneNet() {
  defwFunc = DEFW_NET_ENDNET;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwNetOptions()) return DEFW_BAD_ORDER;

  fprintf(defwFile, " ;\n");
  defwLines++;
  defwState = DEFW_NET_ENDNET;

  return DEFW_OK;
}


int defwEndNets() {
  defwFunc = DEFW_NET_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_NET_START && defwState != DEFW_NET_OPTIONS &&
      defwState != DEFW_NET &&
      defwState != DEFW_NET_ENDNET) // last state is a net 
     return DEFW_BAD_ORDER;
  if (defwCounter) return DEFW_BAD_DATA;

  fprintf(defwFile, "END NETS\n\n");
  defwLines++;

  defwState = DEFW_NET_END;
  defwDidNets = 1;
  return DEFW_OK;
}


int defwStartIOTimings(int count) {
  defwObsoleteNum = DEFW_IOTIMING_START;
  defwFunc = DEFW_IOTIMING_START;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidNets) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_IOTIMING_START) return DEFW_BAD_ORDER;
  if (defVersionNum >= 5.4) return DEFW_OBSOLETE;

  fprintf(defwFile, "IOTIMINGS %d ;\n", count);
  defwLines++;

  defwCounter = count;
  defwState = DEFW_IOTIMING_START;
  return DEFW_OK;
}


int defwIOTiming(const char* instance, const char* pin) {
  defwFunc = DEFW_IOTIMING;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_IOTIMING_START &&
      defwState != DEFW_IOTIMING) return DEFW_BAD_ORDER;

  if (defwState == DEFW_IOTIMING)
     fprintf(defwFile, " ;\n");   // from previous statement
  fprintf(defwFile, "   - ( %s %s )\n", instance, pin);
  defwLines++;

  defwCounter--;
  defwState = DEFW_IOTIMING;
  return DEFW_OK;
}

int defwIOTimingVariable(const char* riseFall, int num1,
        int num2) {
  defwFunc = DEFW_IOTIMING;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_IOTIMING) return DEFW_BAD_ORDER;

  if (strcmp(riseFall, "RISE") &&
      strcmp(riseFall, "FALL")) return DEFW_BAD_DATA;

  fprintf(defwFile, "      + %s VARIABLE %d %d\n", riseFall,
          num1, num2);
  defwLines++;

  return DEFW_OK;
}

int defwIOTimingSlewrate(const char* riseFall, int num1,
        int num2) {
  defwFunc = DEFW_IOTIMING;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_IOTIMING) return DEFW_BAD_ORDER;

  if (strcmp(riseFall, "RISE") &&
      strcmp(riseFall, "FALL")) return DEFW_BAD_DATA;

  fprintf(defwFile, "      + %s SLEWRATE %d %d\n", riseFall,
          num1, num2);
  defwLines++;

  return DEFW_OK;
}

int defwIOTimingDrivecell(const char* name, const char* fromPin,
                          const char* toPin, int numDrivers) {
  defwFunc = DEFW_IOTIMING;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_IOTIMING) return DEFW_BAD_ORDER;
  if (! name || ! *name) // required
     return DEFW_BAD_DATA;

  fprintf(defwFile, "      + DRIVECELL %s ", name);
  if (fromPin && (!toPin && !*toPin)) // if have fromPin, toPin is required
     return DEFW_BAD_DATA;
  if (fromPin)
     fprintf(defwFile, "FROMPIN %s ", fromPin);
  if (toPin)
     fprintf(defwFile, "TOPIN %s ", toPin);
  if (numDrivers)
     fprintf(defwFile, "PARALLEL %d ", numDrivers);
  defwLines++;

  return DEFW_OK;
}


int defwIOTimingCapacitance(double num) {
  defwFunc = DEFW_IOTIMING;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_IOTIMING) return DEFW_BAD_ORDER;

  fprintf(defwFile, "      + CAPACITANCE %g", num);
  defwLines++;

  return DEFW_OK;
}

int defwEndIOTimings() {
  defwFunc = DEFW_IOTIMING_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_IOTIMING_START && defwState != DEFW_IOTIMING)
     return DEFW_BAD_ORDER;
  if (defwCounter) return DEFW_BAD_DATA;

  if (defwState == DEFW_IOTIMING)
     fprintf(defwFile, " ;\n");   // from previous statement
  fprintf(defwFile, "END IOTIMINGS\n\n");
  defwLines++;

  defwState = DEFW_IOTIMING_END;
  return DEFW_OK;
}


int defwStartScanchains(int count) {
  defwFunc = DEFW_SCANCHAIN_START;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidNets) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_SCANCHAIN_START) return DEFW_BAD_ORDER;

  fprintf(defwFile, "SCANCHAINS %d ;\n", count);
  defwLines++;

  defwState = DEFW_SCANCHAIN_START;
  defwCounter = count;
  return DEFW_OK;
}

int defwScanchain(const char* name) {
  defwFunc = DEFW_SCANCHAIN;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SCANCHAIN_START && defwState != DEFW_SCANCHAIN &&
      defwState != DEFW_SCAN_FLOATING && defwState != DEFW_SCAN_ORDERED)
     return DEFW_BAD_ORDER;

  if (defwState == DEFW_SCANCHAIN || defwState == DEFW_SCAN_FLOATING ||
      defwState == DEFW_SCAN_ORDERED) // put a ; for the previous scanchain
     fprintf(defwFile, " ;\n");

  fprintf(defwFile, "   - %s", name);
  defwLines++;

  defwCounter--;
  defwState = DEFW_SCANCHAIN;
  return DEFW_OK;
}

int defwScanchainCommonscanpins (const char* inst1, const char* pin1,
                                 const char* inst2, const char* pin2) {
  defwFunc = DEFW_SCANCHAIN;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SCANCHAIN_START && defwState != DEFW_SCANCHAIN &&
      defwState != DEFW_SCAN_FLOATING && defwState != DEFW_SCAN_ORDERED)
     return DEFW_BAD_ORDER;

  if (!inst1) {     // if inst1 is null, nothing will be written
     defwState = DEFW_SCANCHAIN;
     return DEFW_OK;
  }

  if (inst1 && strcmp(inst1, "IN") && strcmp(inst1, "OUT")) // IN | OUT
     return DEFW_BAD_DATA;

  if (inst1 && !pin1)        // pin1 can't be NULL if inst1 is not
     return DEFW_BAD_DATA;

  fprintf(defwFile, "\n      + COMMONSCANPINS ( %s %s )", inst1, pin1);

  if (inst2 && !pin2)        // pin2 can't be NULL if inst2 is not
     return DEFW_BAD_DATA;

  if (inst2 && strcmp(inst2, "IN") && strcmp(inst2, "OUT")) // IN | OUT
     return DEFW_BAD_DATA;

  if (inst2)
     fprintf(defwFile, " ( %s %s )", inst2, pin2);

  defwLines++;
  
  defwState = DEFW_SCANCHAIN;
  return DEFW_OK;
}

int defwScanchainPartition (const char* name, int maxBits) {
  defwFunc = DEFW_SCANCHAIN;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SCANCHAIN_START && defwState != DEFW_SCANCHAIN &&
      defwState != DEFW_SCAN_FLOATING && defwState != DEFW_SCAN_ORDERED)
     return DEFW_BAD_ORDER;

  if (!name || ! *name)        // require
     return DEFW_BAD_DATA;

  fprintf(defwFile, "\n      + PARTITION %s", name);
  if (maxBits != -1)
     fprintf(defwFile, " MAXBITS %d", maxBits);
  defwLines++;
  
  defwState = DEFW_SCANCHAIN;
  return DEFW_OK;
}

int defwScanchainStart(const char* inst, const char* pin) {
  defwFunc = DEFW_SCANCHAIN;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SCANCHAIN_START && defwState != DEFW_SCANCHAIN &&
      defwState != DEFW_SCAN_FLOATING && defwState != DEFW_SCAN_ORDERED)
     return DEFW_BAD_ORDER;

  if (!inst && ! *inst)        // require
     return DEFW_BAD_DATA;

  fprintf(defwFile, "\n      + START %s", inst);
  if (pin)
     fprintf(defwFile, " %s", pin);
  defwLines++;
  
  defwState = DEFW_SCANCHAIN;
  return DEFW_OK;
}
  

int defwScanchainStop(const char* inst, const char* pin) {
  defwFunc = DEFW_SCANCHAIN;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SCANCHAIN_START && defwState != DEFW_SCANCHAIN &&
      defwState != DEFW_SCAN_FLOATING && defwState != DEFW_SCAN_ORDERED)
     return DEFW_BAD_ORDER;

  if (!inst && ! *inst)        // require
     return DEFW_BAD_DATA;

  fprintf(defwFile, "\n      + STOP %s", inst);
  if (pin)
     fprintf(defwFile, " %s", pin);
  defwLines++;
  
  defwState = DEFW_SCANCHAIN;
  return DEFW_OK;
}

int defwScanchainFloating(const char* name,
           const char* inst1, const char* pin1,
	   const char* inst2, const char* pin2) {
  defwFunc = DEFW_SCAN_FLOATING;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SCANCHAIN_START && defwState != DEFW_SCANCHAIN &&
      defwState != DEFW_SCAN_FLOATING && defwState != DEFW_SCAN_ORDERED)
     return DEFW_BAD_ORDER;

  if (!name || ! *name)        // require
     return DEFW_BAD_DATA;
  if (inst1 && strcmp(inst1, "IN") && strcmp(inst1, "OUT"))
     return DEFW_BAD_DATA;
  if (inst2 && strcmp(inst2, "IN") && strcmp(inst2, "OUT"))
     return DEFW_BAD_DATA;
  if (inst1 && !pin1)
     return DEFW_BAD_DATA;
  if (inst2 && !pin2)
     return DEFW_BAD_DATA;

  if (defwState != DEFW_SCAN_FLOATING)
     fprintf(defwFile, "\n      + FLOATING");
  else
     fprintf(defwFile, "\n         ");

  fprintf(defwFile, " %s", name);
  if (inst1)
     fprintf(defwFile, " ( %s %s )", inst1, pin1);
  if (inst2)
     fprintf(defwFile, " ( %s %s )", inst2, pin2);

  defwState = DEFW_SCAN_FLOATING;
  defwLines++;

  return DEFW_OK;
}

int defwScanchainFloatingBits(const char* name,
           const char* inst1, const char* pin1,
	   const char* inst2, const char* pin2,
           int bits) {
  defwFunc = DEFW_SCAN_FLOATING;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SCANCHAIN_START && defwState != DEFW_SCANCHAIN &&
      defwState != DEFW_SCAN_FLOATING && defwState != DEFW_SCAN_ORDERED)
     return DEFW_BAD_ORDER;

  if (!name || ! *name)        // require
     return DEFW_BAD_DATA;
  if (inst1 && strcmp(inst1, "IN") && strcmp(inst1, "OUT"))
     return DEFW_BAD_DATA;
  if (inst2 && strcmp(inst2, "IN") && strcmp(inst2, "OUT"))
     return DEFW_BAD_DATA;
  if (inst1 && !pin1)
     return DEFW_BAD_DATA;
  if (inst2 && !pin2)
     return DEFW_BAD_DATA;

  if (defwState != DEFW_SCAN_FLOATING)
     fprintf(defwFile, "\n      + FLOATING");
  else
     fprintf(defwFile, "\n         ");

  fprintf(defwFile, " %s", name);
  if (inst1)
     fprintf(defwFile, " ( %s %s )", inst1, pin1);
  if (inst2)
     fprintf(defwFile, " ( %s %s )", inst2, pin2);
  if (bits != -1)
     fprintf(defwFile, " ( BITS %d )", bits);

  defwState = DEFW_SCAN_FLOATING;
  defwLines++;

  return DEFW_OK;
}
int defwScanchainOrdered(const char* name1,
           const char* inst1, const char* pin1,
	   const char* inst2, const char* pin2, const char* name2,
           const char* inst3, const char* pin3,
	   const char* inst4, const char* pin4) {
  defwFunc = DEFW_SCAN_ORDERED;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SCANCHAIN_START && defwState != DEFW_SCANCHAIN &&
      defwState != DEFW_SCAN_FLOATING && defwState != DEFW_SCAN_ORDERED)
     return DEFW_BAD_ORDER;

  if (!name1 || ! *name1)        // require
     return DEFW_BAD_DATA;
  if (inst1 && strcmp(inst1, "IN") && strcmp(inst1, "OUT"))
     return DEFW_BAD_DATA;
  if (inst2 && strcmp(inst2, "IN") && strcmp(inst2, "OUT"))
     return DEFW_BAD_DATA;
  if (inst1 && !pin1)
     return DEFW_BAD_DATA;
  if (inst2 && !pin2)
     return DEFW_BAD_DATA;
  if (defwState != DEFW_SCAN_ORDERED) {  // 1st time require both name1 & name2
     if (!name2 || ! *name2)        // require
        return DEFW_BAD_DATA;
     if (inst3 && strcmp(inst3, "IN") && strcmp(inst3, "OUT"))
        return DEFW_BAD_DATA;
     if (inst4 && strcmp(inst4, "IN") && strcmp(inst4, "OUT"))
        return DEFW_BAD_DATA;
     if (inst3 && !pin3)
        return DEFW_BAD_DATA;
     if (inst4 && !pin4)
        return DEFW_BAD_DATA;
  }
 
  if (defwState != DEFW_SCAN_ORDERED)
     fprintf(defwFile, "\n      + ORDERED");
  else
     fprintf(defwFile, "\n         ");

  fprintf(defwFile, " %s", name1);
  if (inst1)
     fprintf(defwFile, " ( %s %s )", inst1, pin1);
  if (inst2)
     fprintf(defwFile, " ( %s %s )", inst2, pin2);
  defwLines++;

  if (name2) {
     fprintf (defwFile, "\n          %s", name2);
     if (inst3)
        fprintf(defwFile, " ( %s %s )", inst3, pin3);
     if (inst4)
        fprintf(defwFile, " ( %s %s )", inst4, pin4);
     defwLines++;
  }

  defwState = DEFW_SCAN_ORDERED;

  return DEFW_OK;
}

int defwScanchainOrderedBits(const char* name1,
           const char* inst1, const char* pin1,
	   const char* inst2, const char* pin2,
           int bits1, const char* name2,
           const char* inst3, const char* pin3,
	   const char* inst4, const char* pin4,
           int bits2) {
  defwFunc = DEFW_SCAN_ORDERED;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SCANCHAIN_START && defwState != DEFW_SCANCHAIN &&
      defwState != DEFW_SCAN_FLOATING && defwState != DEFW_SCAN_ORDERED)
     return DEFW_BAD_ORDER;

  if (!name1 || ! *name1)        // require
     return DEFW_BAD_DATA;
  if (inst1 && strcmp(inst1, "IN") && strcmp(inst1, "OUT"))
     return DEFW_BAD_DATA;
  if (inst2 && strcmp(inst2, "IN") && strcmp(inst2, "OUT"))
     return DEFW_BAD_DATA;
  if (inst1 && !pin1)
     return DEFW_BAD_DATA;
  if (inst2 && !pin2)
     return DEFW_BAD_DATA;
  if (defwState != DEFW_SCAN_ORDERED) {  // 1st time require both name1 & name2
     if (!name2 || ! *name2)        // require
        return DEFW_BAD_DATA;
     if (inst3 && strcmp(inst3, "IN") && strcmp(inst3, "OUT"))
        return DEFW_BAD_DATA;
     if (inst4 && strcmp(inst4, "IN") && strcmp(inst4, "OUT"))
        return DEFW_BAD_DATA;
     if (inst3 && !pin3)
        return DEFW_BAD_DATA;
     if (inst4 && !pin4)
        return DEFW_BAD_DATA;
  }
 
  if (defwState != DEFW_SCAN_ORDERED)
     fprintf(defwFile, "\n      + ORDERED");
  else
     fprintf(defwFile, "\n         ");

  fprintf(defwFile, " %s", name1);
  if (inst1)
     fprintf(defwFile, " ( %s %s )", inst1, pin1);
  if (inst2)
     fprintf(defwFile, " ( %s %s )", inst2, pin2);
  if (bits1 != -1)
     fprintf(defwFile, " ( BITS %d )", bits1);
  defwLines++;

  if (name2) {
     fprintf (defwFile, "\n          %s", name2);
     if (inst3)
        fprintf(defwFile, " ( %s %s )", inst3, pin3);
     if (inst4)
        fprintf(defwFile, " ( %s %s )", inst4, pin4);
  if (bits2 != -1)
     fprintf(defwFile, " ( BITS %d )", bits2);
     defwLines++;
  }

  defwState = DEFW_SCAN_ORDERED;

  return DEFW_OK;
}

int defwEndScanchain() {
  defwFunc = DEFW_SCANCHAIN_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SCANCHAIN_START && defwState != DEFW_SCAN_ORDERED &&
      defwState != DEFW_SCAN_FLOATING && defwState != DEFW_SCANCHAIN)
     return DEFW_BAD_ORDER;

  fprintf(defwFile, " ;\n"); 

  if (defwCounter) return DEFW_BAD_DATA;
  fprintf(defwFile, "END SCANCHAINS\n\n");
  defwLines++;

  defwState = DEFW_SCANCHAIN_END;
  return DEFW_OK;
}

int defwStartConstraints(int count) {
  defwObsoleteNum = DEFW_FPC_START;
  defwFunc = DEFW_FPC_START;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidNets) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_FPC_START) return DEFW_BAD_ORDER;
  if (defVersionNum >= 5.4) return DEFW_OBSOLETE;

  fprintf(defwFile, "CONSTRAINTS %d ;\n", count);
  defwLines++;

  defwState = DEFW_FPC_START;
  defwCounter = count;
  return DEFW_OK;
}

int defwConstraintOperand() {
  defwFunc = DEFW_FPC_OPER;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_FPC_START && defwState != DEFW_FPC)
     return DEFW_BAD_ORDER;

  fprintf(defwFile, "   -");
  defwCounter--;
  defwFPC = 0;
  defwState = DEFW_FPC_OPER;
  return DEFW_OK;
}

int defwConstraintOperandNet(const char* netName) {
  defwFunc = DEFW_FPC_OPER;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_FPC_OPER && defwState != DEFW_FPC_OPER_SUM)
     return DEFW_BAD_ORDER;          // net can be within SUM

  if (!netName && ! *netName)        // require
     return DEFW_BAD_DATA;
  if (defwFPC > 0)
     fprintf(defwFile, " ,");
  if (defwState == DEFW_FPC_OPER_SUM)
     defwFPC++;
  fprintf(defwFile, " NET %s", netName);
  return DEFW_OK;
}

int defwConstraintOperandPath(const char* comp1, const char* fromPin,
                              const char* comp2, const char* toPin) {
  defwFunc = DEFW_FPC_OPER;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_FPC_OPER && defwState != DEFW_FPC_OPER_SUM)
     return DEFW_BAD_ORDER;          // path can be within SUM

  if (!comp1 && ! *comp1 && !fromPin && ! *fromPin &&
      !comp2 && ! *comp2 && !toPin && ! *toPin)        // require
     return DEFW_BAD_DATA;
  if (defwFPC > 0)
     fprintf(defwFile, " ,");
  if (defwState == DEFW_FPC_OPER_SUM)
     defwFPC++;
  fprintf(defwFile, " PATH %s %s %s %s", comp1, fromPin, comp2, toPin);
  return DEFW_OK;
}

int defwConstraintOperandSum() {
  defwFunc = DEFW_FPC_OPER;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_FPC_OPER && defwState != DEFW_FPC_OPER_SUM)
     return DEFW_BAD_ORDER;          // sum can be within SUM

  fprintf(defwFile, " SUM (");
  defwState = DEFW_FPC_OPER_SUM;
  defwFPC = 0;
  return DEFW_OK;
}

int defwConstraintOperandSumEnd() {
  defwFunc = DEFW_FPC_OPER;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_FPC_OPER_SUM)
     return DEFW_BAD_ORDER;
  fprintf(defwFile, " )");
  defwState = DEFW_FPC_OPER;
  defwFPC = 0;
  return DEFW_OK;
}

int defwConstraintOperandTime(const char* timeType, int time) {
  defwFunc = DEFW_FPC_OPER;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_FPC_OPER)
     return DEFW_BAD_ORDER;
  if (timeType && strcmp(timeType, "RISEMAX") && strcmp(timeType, "FALLMAX") &&
      strcmp(timeType, "RISEMIN") && strcmp(timeType, "FALLMIN"))
     return DEFW_BAD_DATA;
  fprintf(defwFile, " + %s %d", timeType, time);
  return DEFW_OK;
}

int defwConstraintOperandEnd() {
  defwFunc = DEFW_FPC_OPER;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_FPC_OPER)
     return DEFW_BAD_ORDER;
  fprintf(defwFile, " ;\n");
  defwState = DEFW_FPC;
  return DEFW_OK;
}

int defwConstraintWiredlogic(const char* netName, int distance) {
  defwFunc = DEFW_FPC;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_FPC_START && defwState != DEFW_FPC)
     return DEFW_BAD_ORDER;

  if (!netName && ! *netName)        // require
     return DEFW_BAD_DATA;
  fprintf(defwFile, "   - WIREDLOGIC %s MAXDIST %d ;\n", netName, distance);
  defwCounter--;
  defwState = DEFW_FPC;
  defwLines++;
  return DEFW_OK;
} 

int defwEndConstraints() {
  defwFunc = DEFW_FPC_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_FPC_START && defwState != DEFW_FPC)
     return DEFW_BAD_ORDER;
  if (defwCounter) return DEFW_BAD_DATA;

  fprintf(defwFile, "END CONSTRAINTS\n\n");
  defwLines++;

  defwState = DEFW_FPC_END;
  return DEFW_OK;
}

int defwStartGroups(int count) {
  defwFunc = DEFW_GROUP_START;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidNets) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_GROUP_START) return DEFW_BAD_ORDER;

  fprintf(defwFile, "GROUPS %d ;\n", count);
  defwLines++;

  defwState = DEFW_GROUP_START;
  defwCounter = count;
  return DEFW_OK;
}

int defwGroup(const char* groupName, int numExpr, const char** groupExpr) {
  int i;

  defwFunc = DEFW_GROUP;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_GROUP_START && defwState != DEFW_GROUP)
     return DEFW_BAD_ORDER;

  if (!groupName && ! *groupName && !groupExpr && ! *groupExpr)  // require
     return DEFW_BAD_DATA;
  if (defwState == DEFW_GROUP)
     fprintf(defwFile, " ;\n");          // add ; for the previous group
  fprintf(defwFile, "   - %s", groupName);
  if (numExpr) {
     for (i = 0; i < numExpr; i++)
        fprintf(defwFile, " %s", groupExpr[i]);
  }
  defwCounter--;
  defwLines++;
  defwState = DEFW_GROUP;
  return DEFW_OK;
}


int defwGroupSoft(const char* type1, double value1, const char* type2,
                  double value2, const char* type3, double value3) {
  defwFunc = DEFW_GROUP;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_GROUP)
     return DEFW_BAD_ORDER;

  if (type1 && strcmp(type1, "MAXHALFPERIMETER") && strcmp(type1, "MAXX") &&
      strcmp(type1, "MAXY"))
     return DEFW_BAD_DATA;
  if (type2 && strcmp(type2, "MAXHALFPERIMETER") && strcmp(type2, "MAXX") &&
      strcmp(type2, "MAXY"))
     return DEFW_BAD_DATA;
  if (type3 && strcmp(type3, "MAXHALFPERIMETER") && strcmp(type3, "MAXX") &&
      strcmp(type3, "MAXY"))
     return DEFW_BAD_DATA;
  if (type1)
     fprintf(defwFile, "\n     + SOFT %s %g", type1, value1);
  if (type2)
     fprintf(defwFile, " %s %g", type2, value2);
  if (type3)
     fprintf(defwFile, " %s %g", type3, value3);
  defwLines++;
  return DEFW_OK;
}

int defwGroupRegion(int xl, int yl, int xh, int yh, const char* regionName) {
  defwFunc = DEFW_GROUP;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_GROUP)
     return DEFW_BAD_ORDER;

  if ((xl || yl || xh || yh) && (regionName))  // ether pts or regionName
     return DEFW_BAD_DATA;

  if (regionName)
     fprintf(defwFile, "\n      + REGION %s", regionName);
  else
     fprintf(defwFile, "\n      + REGION ( %d %d ) ( %d %d )",
             xl, yl, xh, yh);
  defwLines++;
  return DEFW_OK;
}

int defwEndGroups() {
  defwFunc = DEFW_GROUP_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_GROUP_START && defwState != DEFW_GROUP)
     return DEFW_BAD_ORDER;
  if (defwCounter) return DEFW_BAD_DATA;

  fprintf(defwFile, " ;\n");

  fprintf(defwFile, "END GROUPS\n\n");
  defwLines++;

  defwState = DEFW_GROUP_END;
  return DEFW_OK;
}


int defwStartBlockages(int count) {
  defwFunc = DEFW_BLOCKAGE_START;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidNets) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_BLOCKAGE_START) return DEFW_BAD_ORDER;
  if (defVersionNum < 5.4)
     return DEFW_WRONG_VERSION;

  fprintf(defwFile, "BLOCKAGES %d ;\n", count);
  defwLines++;

  defwState = DEFW_BLOCKAGE_START;
  defwCounter = count;
  return DEFW_OK;
}

int defwBlockageLayer(const char* layerName,
            const char* compName) {      /* optional(NULL) */
  defwFunc = DEFW_BLOCKAGE_LAYER;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  // May be user already define component or layer slots or fills
  if (defwState != DEFW_BLOCKAGE_START && ((defwState == DEFW_BLOCKAGE_PLACE)
      || (defwState == DEFW_BLOCKAGE_LAYER)))
     return DEFW_BAD_DATA;

  if (!layerName && ! *layerName)  // require
     return DEFW_BAD_DATA;
  if (defwState == DEFW_BLOCKAGE_RECT)
     fprintf(defwFile, " ;\n");   // end the previous rectangle
  fprintf(defwFile, "   - LAYER %s ", layerName);
  if (compName &&  *compName != 0)  // optional
     fprintf(defwFile, "+ COMPONENT %s ", compName);
  fprintf(defwFile, "\n");
  defwCounter--;
  defwLines++;
  defwState = DEFW_BLOCKAGE_LAYER;
  return DEFW_OK;
}


int defwBlockageLayerSlots(const char* layerName) {
  defwFunc = DEFW_BLOCKAGE_LAYER;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  // May be user already define component or layer or layer fills
  if (defwState != DEFW_BLOCKAGE_START && ((defwState == DEFW_BLOCKAGE_PLACE)
      || (defwState == DEFW_BLOCKAGE_LAYER)))
     return DEFW_BAD_DATA;

  if (!layerName && ! *layerName)  // require
     return DEFW_BAD_DATA;
  if (defwState == DEFW_BLOCKAGE_RECT)
     fprintf(defwFile, " ;\n");   // end the previous rectangle
  fprintf(defwFile, "   - LAYER %s + SLOTS\n", layerName);
  defwCounter--;
  defwLines++;
  defwState = DEFW_BLOCKAGE_LAYER;
  return DEFW_OK;
}


int defwBlockageLayerFills(const char* layerName) {
  defwFunc = DEFW_BLOCKAGE_LAYER;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  // May be user already define component or layer or layer slots
  if (defwState != DEFW_BLOCKAGE_START && ((defwState == DEFW_BLOCKAGE_PLACE)
      || (defwState == DEFW_BLOCKAGE_LAYER)))
     return DEFW_BAD_DATA;

  if (!layerName && ! *layerName)  // require
     return DEFW_BAD_DATA;
  if (defwState == DEFW_BLOCKAGE_RECT)
     fprintf(defwFile, " ;\n");   // end the previous rectangle
  fprintf(defwFile, "   - LAYER %s + FILLS\n", layerName);
  defwCounter--;
  defwLines++;
  defwState = DEFW_BLOCKAGE_LAYER;
  return DEFW_OK;
}


int defwBlockageLayerPushdown(const char* layerName) {
  defwFunc = DEFW_BLOCKAGE_LAYER;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  // May be user already define component or layer or layer slots
  if (defwState != DEFW_BLOCKAGE_START && ((defwState == DEFW_BLOCKAGE_PLACE)
      || (defwState == DEFW_BLOCKAGE_LAYER)))
     return DEFW_BAD_DATA;

  if (!layerName && ! *layerName)  // require
     return DEFW_BAD_DATA;
  if (defwState == DEFW_BLOCKAGE_RECT)
     fprintf(defwFile, " ;\n");   // end the previous rectangle
  fprintf(defwFile, "   - LAYER %s + PUSHDOWN\n", layerName);
  defwCounter--;
  defwLines++;
  defwState = DEFW_BLOCKAGE_LAYER;
  return DEFW_OK;
}


// From Craig Files - Agilent
int defwBlockagePlacement() {
  defwFunc = DEFW_BLOCKAGE_PLACE;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  // May be user already define layer
  if (defwState != DEFW_BLOCKAGE_START && ((defwState == DEFW_BLOCKAGE_LAYER)
     || (defwState == DEFW_BLOCKAGE_PLACE)))
     return DEFW_BAD_DATA;

  if (defwState == DEFW_BLOCKAGE_RECT)
     fprintf(defwFile, " ;\n");   // end the previous rectangle
  fprintf(defwFile, "   - PLACEMENT\n");
  defwCounter--;
  defwLines++;
  defwState = DEFW_BLOCKAGE_PLACE;
  return DEFW_OK;
}


int defwBlockagePlacementComponent(const char* compName) {
  defwFunc = DEFW_BLOCKAGE_PLACE;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  // May be user already define layer
  if (defwState != DEFW_BLOCKAGE_START && ((defwState == DEFW_BLOCKAGE_LAYER)
     || (defwState == DEFW_BLOCKAGE_PLACE)))
     return DEFW_BAD_DATA;

  if (!compName && ! *compName)  // require
     return DEFW_BAD_DATA;
  if (defwState == DEFW_BLOCKAGE_RECT)
     fprintf(defwFile, " ;\n");   // end the previous rectangle
  fprintf(defwFile, "   - PLACEMENT + COMPONENT %s\n", compName);
  defwCounter--;
  defwLines++;
  defwState = DEFW_BLOCKAGE_PLACE;
  return DEFW_OK;
}


int defwBlockagePlacementPushdown() {
  defwFunc = DEFW_BLOCKAGE_PLACE;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  // May be user already define layer
  if (defwState != DEFW_BLOCKAGE_START && ((defwState == DEFW_BLOCKAGE_LAYER)
     || (defwState == DEFW_BLOCKAGE_PLACE)))
     return DEFW_BAD_DATA;
  if (defwState == DEFW_BLOCKAGE_RECT)
     fprintf(defwFile, " ;\n");   // end the previous rectangle
  fprintf(defwFile, "   - PLACEMENT + PUSHDOWN\n");
  defwCounter--;
  defwLines++;
  defwState = DEFW_BLOCKAGE_PLACE;
  return DEFW_OK;
}


int defwBlockageRect(int xl, int yl, int xh, int yh) {
  defwFunc = DEFW_BLOCKAGE_RECT;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  // May be user already define layer
  if (defwState != DEFW_BLOCKAGE_PLACE && defwState != DEFW_BLOCKAGE_LAYER &&
      defwState != DEFW_BLOCKAGE_RECT)
     return DEFW_BAD_DATA;

  if (defwState == DEFW_BLOCKAGE_RECT)
     fprintf(defwFile, "\n");   // set a newline for the previous rectangle

  fprintf(defwFile, "     RECT ( %d %d ) ( %d %d )", xl, yl, xh, yh);
  defwLines++;
  defwState = DEFW_BLOCKAGE_RECT;
  return DEFW_OK;
}


int defwEndBlockages() {
  defwFunc = DEFW_BLOCKAGE_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_BLOCKAGE_RECT)
     return DEFW_BAD_ORDER;
  if (defwCounter) return DEFW_BAD_DATA;

  fprintf(defwFile, " ;\n");

  fprintf(defwFile, "END BLOCKAGES\n\n");
  defwLines++;

  defwState = DEFW_BLOCKAGE_END;
  return DEFW_OK;
}


int defwStartSlots(int count) {
  defwFunc = DEFW_SLOT_START;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidNets) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_SLOT_START) return DEFW_BAD_ORDER;
  if (defVersionNum < 5.4)
     return DEFW_WRONG_VERSION;

  fprintf(defwFile, "SLOTS %d ;\n", count);
  defwLines++;

  defwState = DEFW_SLOT_START;
  defwCounter = count;
  return DEFW_OK;
}

int defwSlotLayer(const char* layerName) {
  defwFunc = DEFW_SLOT_LAYER;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  // May be user already define layer
  if (defwState != DEFW_SLOT_START && defwState == DEFW_SLOT_LAYER)
     return DEFW_BAD_DATA;

  if (!layerName && ! *layerName)  // require
     return DEFW_BAD_DATA;
  if (defwState == DEFW_SLOT_RECT)
     fprintf(defwFile, " ;\n");   // end the previous rectangle
  fprintf(defwFile, "   - LAYER %s \n", layerName);
  defwCounter--;
  defwLines++;
  defwState = DEFW_SLOT_LAYER;
  return DEFW_OK;
}


int defwSlotRect(int xl, int yl, int xh, int yh) {
  defwFunc = DEFW_SLOT_RECT;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  // May be user already define layer
  if (defwState != DEFW_SLOT_LAYER && defwState != DEFW_SLOT_RECT)
     return DEFW_BAD_DATA;

  if (defwState == DEFW_SLOT_RECT)
     fprintf(defwFile, "\n");   // set a newline for the previous rectangle

  fprintf(defwFile, "     RECT ( %d %d ) ( %d %d )", xl, yl, xh, yh);
  defwLines++;
  defwState = DEFW_SLOT_RECT;
  return DEFW_OK;
}


int defwEndSlots() {
  defwFunc = DEFW_SLOT_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_SLOT_RECT)
     return DEFW_BAD_ORDER;
  if (defwCounter) return DEFW_BAD_DATA;

  fprintf(defwFile, " ;\n");

  fprintf(defwFile, "END SLOTS\n\n");
  defwLines++;

  defwState = DEFW_SLOT_END;
  return DEFW_OK;
}


int defwStartFills(int count) {
  defwFunc = DEFW_FILL_START;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidNets) return DEFW_BAD_ORDER;
  if (defwState >= DEFW_FILL_START) return DEFW_BAD_ORDER;
  if (defVersionNum < 5.4)
     return DEFW_WRONG_VERSION;

  fprintf(defwFile, "FILLS %d ;\n", count);
  defwLines++;

  defwState = DEFW_FILL_START;
  defwCounter = count;
  return DEFW_OK;
}

int defwFillLayer(const char* layerName) {
  defwFunc = DEFW_FILL_LAYER;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  // May be user already define layer
  if (defwState != DEFW_FILL_START && defwState == DEFW_FILL_LAYER)
     return DEFW_BAD_DATA;

  if (!layerName && ! *layerName)  // require
     return DEFW_BAD_DATA;
  if (defwState == DEFW_FILL_RECT)
     fprintf(defwFile, " ;\n");   // end the previous rectangle
  fprintf(defwFile, "   - LAYER %s \n", layerName);
  defwCounter--;
  defwLines++;
  defwState = DEFW_FILL_LAYER;
  return DEFW_OK;
}


int defwFillRect(int xl, int yl, int xh, int yh) {
  defwFunc = DEFW_FILL_RECT;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  // May be user already define layer
  if (defwState != DEFW_FILL_LAYER && defwState != DEFW_FILL_RECT)
     return DEFW_BAD_DATA;

  if (defwState == DEFW_FILL_RECT)
     fprintf(defwFile, "\n");   // set a newline for the previous rectangle

  fprintf(defwFile, "     RECT ( %d %d ) ( %d %d )", xl, yl, xh, yh);
  defwLines++;
  defwState = DEFW_FILL_RECT;
  return DEFW_OK;
}


int defwEndFills() {
  defwFunc = DEFW_FILL_END;   // Current function of writer
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (defwState != DEFW_FILL_RECT)
     return DEFW_BAD_ORDER;
  if (defwCounter) return DEFW_BAD_DATA;

  fprintf(defwFile, " ;\n");

  fprintf(defwFile, "END FILLS\n\n");
  defwLines++;

  defwState = DEFW_FILL_END;
  return DEFW_OK;
}


int defwStartBeginext(const char* name) {
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState == DEFW_BEGINEXT_START ||
     defwState == DEFW_BEGINEXT) return DEFW_BAD_ORDER;
  if (!name || name == 0 || *name == 0) return DEFW_BAD_DATA;
  fprintf(defwFile, "BEGINEXT \"%s\"\n", name);

  defwState = DEFW_BEGINEXT_START;
  defwLines++;
  return DEFW_OK;
}

int defwBeginextCreator(const char* creatorName) {
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState != DEFW_BEGINEXT_START &&
     defwState != DEFW_BEGINEXT) return DEFW_BAD_ORDER;
  if (!creatorName || creatorName == 0 || *creatorName == 0)
     return DEFW_BAD_DATA;
  fprintf(defwFile, "   CREATOR \"%s\"\n", creatorName);

  defwState = DEFW_BEGINEXT;
  defwLines++;
  return DEFW_OK;
}


int defwBeginextDate() {
  time_t todayTime;
  char * rettime ;

  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState != DEFW_BEGINEXT_START &&
     defwState != DEFW_BEGINEXT) return DEFW_BAD_ORDER;

  todayTime = time(NULL);             // time in UTC 
  rettime = ctime(&todayTime);        // convert to string
  rettime[strlen(rettime)-1] = '\0';  // replace \n with \0
  fprintf(defwFile, "   DATE \"%s\"", rettime);

  defwState = DEFW_BEGINEXT;
  defwLines++;
  return DEFW_OK;
}


int defwBeginextRevision(int vers1, int vers2) {
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState != DEFW_BEGINEXT_START &&
     defwState != DEFW_BEGINEXT) return DEFW_BAD_ORDER;
  fprintf(defwFile, "\n   REVISION %d.%d", vers1, vers2);

  defwState = DEFW_BEGINEXT;
  defwLines++;
  return DEFW_OK;
}


int defwBeginextSyntax(const char* title, const char* string) {
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState != DEFW_BEGINEXT_START &&
     defwState != DEFW_BEGINEXT) return DEFW_BAD_ORDER;
  fprintf(defwFile, "\n   - %s %s", title, string);

  defwState = DEFW_BEGINEXT;
  defwLines++;
  return DEFW_OK;
}


int defwEndBeginext() {
  if (! defwFile) return DEFW_UNINITIALIZED;
  if (! defwDidInit) return DEFW_BAD_ORDER;
  if (defwState != DEFW_BEGINEXT_START &&
     defwState != DEFW_BEGINEXT) return DEFW_BAD_ORDER;
  fprintf(defwFile, ";\nENDEXT\n\n");

  defwState = DEFW_BEGINEXT_END;
  defwLines++;
  return DEFW_OK;
}


int defwEnd() {
  defwFunc = DEFW_END;   // Current function of writer
  if (! defwFile) return 1;
  if (! defwDidNets) return DEFW_BAD_ORDER;

  if (defwState == DEFW_ROW)
    fprintf(defwFile, ";\n\n");  // add the ; and \n for the previous row.

  fprintf(defwFile, "END DESIGN\n\n");
  defwLines++;
  //defwFile = 0;
  defwState = DEFW_DONE;
  return DEFW_OK;
}


int defwCurrentLineNumber() {
  return defwLines;
}


void defwPrintError(int status) {
  switch (status) {
     case DEFW_OK:
          fprintf(defwFile, "No Error.\n");
          break;
     case DEFW_UNINITIALIZED:
          printf("Need to call defwInit first.\n");
          break;
     case DEFW_BAD_ORDER:
          fprintf(defwFile, "%s - Incorrect order of data.\n",
                  defwStateStr[defwFunc]);
          break;
     case DEFW_BAD_DATA:
          fprintf(defwFile, "%s - Invalid data.\n",
                  defwStateStr[defwFunc]);
          break;
     case DEFW_ALREADY_DEFINED:
          fprintf(defwFile, "%s - Section is allowed to define only once.\n",
                  defwStateStr[defwFunc]);
          break;
     case DEFW_WRONG_VERSION:
          fprintf(defwFile, "%s - Version number is set before 5.4, but 5.4 API is used.\n",
                  defwStateStr[defwFunc]);
          break;
     case DEFW_OBSOLETE:
          fprintf(defwFile, "%s - is no longer valid in 5.4.\n",
                  defwStateStr[defwObsoleteNum]);
          break;
  }
  return;
}


void defwAddComment(const char* comment) {
  if (comment)
     fprintf(defwFile, "# %s\n", comment);
  return;
}
 

void defwAddIndent() {
  fprintf(defwFile,"   ");
  return;
}


/***************************
   Questions:
 - Is only one row rule allowed
 - Is only one tracks rule allowed
 - In the die area is a zero area allowed? overlaps?
 - What type of checking is needed for the rows and tracks do loop?
 - Can you have a default prop with a number AND a range?
 - What is the pin properties section mentioned in the 5.1 spec?
 *****************************/
