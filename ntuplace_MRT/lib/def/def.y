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

%{
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "defrReader.hpp"
#include "lex.h"
#include "defiUser.hpp"
#include "defrCallBacks.hpp"

// TX_DIR:TRANSLATION ON

extern int dumb_mode;     // Read next N tokens in dumb mode
extern int no_num;        // No numbers for the next N tokens
extern int real_num;      // Next token will be a real number
extern int by_is_keyword;
extern int bit_is_keyword;
extern int do_is_keyword;
extern int new_is_keyword;
extern int nondef_is_keyword;
extern int step_is_keyword;
extern int mustjoin_is_keyword;
extern char* History_text;

extern int errors;        // from lex.cpph

double save_x;
double save_y;

static double defVersionNum = 5.5;   // default is 5.5

static int defIgnoreVersion = 0; // ignore checking version number
                                 // this is redundant, since def does not have
                                 // either 5.3 or 5.4 code

static int hasVer = 0;        // keep track VERSION is in the file
static int hasNameCase = 0;   // keep track NAMESCASESENSITIVE is in the file
static int hasDivChar = 0;    // keep track DIVIDERCHAR is in the file
static int hasBusBit = 0;     // keep track BUSBITCHARS is in the file
static int hasDes = 0;        // keep track DESIGN is in the file
static int regTypeDef;        // keep track that region type is defined 
static int aOxide = 0;        // keep track for oxide

int names_case_sensitive = FALSE;
int defRetVal;

int shield = FALSE;       // To identify if the path is shield for 5.3

static char defPropDefType;   // save the current type of the property

int bitsNum;                  // Scanchain Bits value

/* From def_keywords.cpp */
extern char* ringCopy(const char* string);


/* Macro to describe how we handle a callback.
 * If the function was set then call it.
 * If the function returns non zero then there was an error
 * so call the error routine and exit.
 */
#define CALLBACK(func, typ, data) \
    if (!errors) {\
      if (func) { \
        if ((defRetVal = (*func)(typ, data, defrUserData)) == PARSE_OK) { \
        } else if (defRetVal == STOP_PARSE) { \
	  return defRetVal; \
        } else { \
          yyerror("Error in callback"); \
	  return defRetVal; \
        } \
      } \
    }

#define CHKERR() \
    if (errors > 20) {\
      yyerror("Too many syntax errors"); \
      errors = 0; \
      return 1; \
    }

double convert_defname2num(char *versionName)
{
    char majorNm[80];
    char minorNm[80];
    char *subMinorNm = NULL;
    char *p1;
    char *versionNm = strdup(versionName);

    double major = 0, minor = 0, subMinor = 0;
    double version;

    sscanf(versionNm, "%[^.].%s", majorNm, minorNm);
    if (p1 = strchr(minorNm, '.')) {
       subMinorNm = p1+1;
       *p1 = '\0';
    }
    major = atof(majorNm);
    minor = atof(minorNm);
    if (subMinorNm)
       subMinor = atof(subMinorNm);

    version = major;

    if (minor > 0)
       version = major + minor/10;

    if (subMinor > 0)
       version = version + subMinor/1000;

    free(versionNm);
    return version;
}


#define FIXED 1
#define COVER 2
#define PLACED 3
#define UNPLACED 4
%}

%union {
        double dval ;
        int    integer ;
        char * string ;
        int    keyword ;  /* really just a nop */
        struct defpoint pt;
        defTOKEN *tk;
}

%token <string>  QSTRING
%token <string>  T_STRING SITE_PATTERN
%token <dval>    NUMBER
%token <keyword> K_HISTORY K_NAMESCASESENSITIVE
%token <keyword> K_DESIGN K_VIAS K_TECH K_UNITS K_ARRAY K_FLOORPLAN
%token <keyword> K_SITE K_CANPLACE K_CANNOTOCCUPY K_DIE_AREA
%token <keyword> K_PINS
%token <keyword> K_DEFAULTCAP K_MINPINS K_WIRECAP
%token <keyword> K_TRACKS K_GCELLGRID
%token <keyword> K_DO K_BY K_STEP K_LAYER K_ROW K_RECT
%token <keyword> K_COMPS K_COMP_GEN K_SOURCE K_WEIGHT K_EEQMASTER
%token <keyword> K_FIXED K_COVER K_UNPLACED K_PLACED K_FOREIGN K_REGION 
%token <keyword> K_REGIONS
%token <keyword> K_NETS K_START_NET K_MUSTJOIN K_ORIGINAL K_USE K_STYLE
%token <keyword> K_PATTERN K_PATTERNNAME K_ESTCAP K_ROUTED K_NEW 
%token <keyword> K_SNETS K_SHAPE K_WIDTH K_VOLTAGE K_SPACING K_NONDEFAULTRULE
%token <keyword> K_N K_S K_E K_W K_FN K_FE K_FS K_FW
%token <keyword> K_GROUPS K_GROUP K_SOFT K_MAXX K_MAXY K_MAXHALFPERIMETER
%token <keyword> K_CONSTRAINTS K_NET K_PATH K_SUM K_DIFF 
%token <keyword> K_SCANCHAINS K_START K_FLOATING K_ORDERED K_STOP K_IN K_OUT
%token <keyword> K_RISEMIN K_RISEMAX K_FALLMIN K_FALLMAX K_WIREDLOGIC
%token <keyword> K_MAXDIST
%token <keyword> K_ASSERTIONS
%token <keyword> K_DISTANCE K_MICRONS
%token <keyword> K_END
%token <keyword> K_IOTIMINGS K_RISE K_FALL K_VARIABLE K_SLEWRATE K_CAPACITANCE
%token <keyword> K_DRIVECELL K_FROMPIN K_TOPIN K_PARALLEL
%token <keyword> K_TIMINGDISABLES K_THRUPIN K_MACRO
%token <keyword> K_PARTITIONS K_TURNOFF
%token <keyword> K_FROMCLOCKPIN K_FROMCOMPPIN K_FROMIOPIN
%token <keyword> K_TOCLOCKPIN K_TOCOMPPIN K_TOIOPIN
%token <keyword> K_SETUPRISE K_SETUPFALL K_HOLDRISE K_HOLDFALL
%token <keyword> K_VPIN K_SUBNET K_XTALK K_PIN K_SYNTHESIZED
%token <keyword> K_DEFINE K_DEFINES K_DEFINEB K_IF K_THEN K_ELSE K_FALSE K_TRUE 
%token <keyword> K_EQ K_NE K_LE K_LT K_GE K_GT K_OR K_AND K_NOT
%token <keyword> K_SPECIAL K_DIRECTION K_RANGE
%token <keyword> K_FPC K_HORIZONTAL K_VERTICAL K_ALIGN K_MIN K_MAX K_EQUAL
%token <keyword> K_BOTTOMLEFT K_TOPRIGHT K_ROWS K_TAPER K_TAPERRULE
%token <keyword> K_VERSION K_DIVIDERCHAR K_BUSBITCHARS
%token <keyword> K_PROPERTYDEFINITIONS K_STRING K_REAL K_INTEGER K_PROPERTY
%token <keyword> K_BEGINEXT K_ENDEXT K_NAMEMAPSTRING K_ON K_OFF K_X K_Y
%token <keyword> K_COMPONENT
%token <keyword> K_PINPROPERTIES K_TEST
%token <keyword> K_COMMONSCANPINS K_SNET K_COMPONENTPIN K_REENTRANTPATHS
%token <keyword> K_SHIELD K_SHIELDNET K_NOSHIELD
%token <keyword> K_ANTENNAPINPARTIALMETALAREA K_ANTENNAPINPARTIALMETALSIDEAREA
%token <keyword> K_ANTENNAPINGATEAREA K_ANTENNAPINDIFFAREA
%token <keyword> K_ANTENNAPINMAXAREACAR K_ANTENNAPINMAXSIDEAREACAR
%token <keyword> K_ANTENNAPINPARTIALCUTAREA K_ANTENNAPINMAXCUTCAR
%token <keyword> K_SIGNAL K_POWER K_GROUND K_CLOCK K_TIEOFF K_ANALOG K_SCAN
%token <keyword> K_RESET K_RING K_STRIPE K_FOLLOWPIN K_IOWIRE K_COREWIRE
%token <keyword> K_BLOCKWIRE K_FILLWIRE K_BLOCKAGEWIRE K_PADRING K_BLOCKRING
%token <keyword> K_BLOCKAGES K_PLACEMENT K_SLOTS K_FILLS K_PUSHDOWN
%token <keyword> K_NETLIST K_DIST K_USER K_TIMING K_BALANCED K_STEINER K_TRUNK
%token <keyword> K_FIXEDBUMP K_FENCE K_FREQUENCY K_GUIDE K_MAXBITS
%token <keyword> K_PARTITION K_TYPE K_ANTENNAMODEL K_DRCFILL
%token <keyword> K_OXIDE1 K_OXIDE2 K_OXIDE3 K_OXIDE4
%type  <pt>     pt opt_paren
%type <integer> comp_net_list subnet_opt_syn
%type <integer> orient
%type <integer> placement_status
%type <string> net_type subnet_type track_start use_type shape_type source_type
%type <string> pattern_type netsource_type
%type <tk> path paths new_path
%type <string> risefall opt_pin opt_pattern pin_layer_opt
%type <string> vpin_status opt_plus track_type region_type
%type <string> h_or_v turnoff_setup turnoff_hold
%type <integer> conn_opt partition_maxbits

%%

def_file: version_stmt case_sens_stmt rules end_design
            ;

version_stmt:  /* empty */
    | K_VERSION { dumb_mode = 1; no_num = 1; } T_STRING ';'
      {
         defVersionNum = convert_defname2num($3);
         if (defrVersionStrCbk) {
            CALLBACK(defrVersionStrCbk, defrVersionStrCbkType, $3);
         } else {
            CALLBACK(defrVersionCbk, defrVersionCbkType, defVersionNum);
         }
         if (defVersionNum > 5.3 && defVersionNum < 5.4) {
            defIgnoreVersion = 1;
         }
         hasVer = 1;
      }

case_sens_stmt: /* empty */
    | K_NAMESCASESENSITIVE K_ON ';'
      {
        names_case_sensitive = 1;
        CALLBACK(defrCaseSensitiveCbk, defrCaseSensitiveCbkType,
                 names_case_sensitive); 
        hasNameCase = 1;
      }
    | K_NAMESCASESENSITIVE K_OFF ';'
      {
        names_case_sensitive = 0;
        CALLBACK(defrCaseSensitiveCbk, defrCaseSensitiveCbkType,
                 names_case_sensitive); 
        hasNameCase = 1;
      }

rules: /* empty */
        | rules rule
        | error 
            ;

rule: design_section | via_section | extension_section| comps_section
      | nets_section | snets_section | groups_section | scanchains_section
      | constraint_section | assertions_section | iotiming_section
      | regions_section | floorplan_contraints_section | timingdisables_section
      | partitions_section | pin_props_section | blockage_section
      | slot_section | fill_section
            ;

design_section: design_name | tech_name | history | units | 
              divider_char | bus_bit_chars | 
              site | canplace | cannotoccupy | die_area |
              pin_cap_rule | pin_rule | tracks_rule | gcellgrid | array_name |
              floorplan_name | row_rule | prop_def_section
            ;

design_name: K_DESIGN {dumb_mode = 1; no_num = 1; } T_STRING ';' 
	  {
            CALLBACK(defrDesignCbk, defrDesignStartCbkType, $3);
            hasDes = 1;
          }

end_design: K_END K_DESIGN
	  {
            CALLBACK(defrDesignEndCbk, defrDesignEndCbkType, 0);
            // 11/16/2001 - Wanda da Rosa - pcr 408334
            // Return 1 if there is any errors during parsing
            if (errors)
                return 1;

            if (!hasVer)
              yydefwarning("VERSION is a required statement.");
            if (!hasNameCase)
              yydefwarning("NAMESCASESENSITIVE is a required statement.");
            if (!hasBusBit)
              yydefwarning("BUSBITCHARS is a required statement.");
            if (!hasDivChar)
              yydefwarning("DIVIDERCHAR is a required statement.");
            if (!hasDes)
              yydefwarning("DESIGN is a required statement.");
            hasVer = 0;
            hasNameCase = 0;
            hasBusBit = 0;
            hasDivChar = 0;
            hasDes = 0;
          }

tech_name:   K_TECH { dumb_mode = 1; no_num = 1; } T_STRING ';'
          { CALLBACK(defrTechnologyCbk, defrTechNameCbkType, $3); }

array_name:   K_ARRAY {dumb_mode = 1; no_num = 1;} T_STRING ';'
	  { CALLBACK(defrArrayNameCbk, defrArrayNameCbkType, $3); }

floorplan_name:   K_FLOORPLAN { dumb_mode = 1; no_num = 1; } T_STRING ';'
	  { CALLBACK(defrFloorPlanNameCbk, defrFloorPlanNameCbkType, $3); }

history:  K_HISTORY
          { CALLBACK(defrHistoryCbk, defrHistoryCbkType, History_text); }

prop_def_section: K_PROPERTYDEFINITIONS
	  { CALLBACK(defrPropDefStartCbk, defrPropDefStartCbkType, 0); }
    property_defs K_END K_PROPERTYDEFINITIONS
	  { 
            CALLBACK(defrPropDefEndCbk, defrPropDefEndCbkType, 0);
            real_num = 0;     /* just want to make sure it is reset */
          }

property_defs: /* empty */
        | property_defs property_def
            { }

property_def: K_DESIGN {dumb_mode = 1; no_num = 1; defrProp.defiProp::clear(); }
              T_STRING property_type_and_val ';' 
            {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("design", $3);
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrDesignProp.defiPropType::setPropType($3, defPropDefType);
              }
            }
        | K_NET { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); }
          T_STRING property_type_and_val ';'
            {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("net", $3);
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrNetProp.defiPropType::setPropType($3, defPropDefType);
              }
            }
        | K_SNET { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); }
          T_STRING property_type_and_val ';'
            {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("specialnet", $3);
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrSNetProp.defiPropType::setPropType($3, defPropDefType);
              }
            }
        | K_REGION { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); }
          T_STRING property_type_and_val ';'
            {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("region", $3);
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrRegionProp.defiPropType::setPropType($3, defPropDefType);
              }
            }
        | K_GROUP { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); }
          T_STRING property_type_and_val ';'
            {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("group", $3);
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrGroupProp.defiPropType::setPropType($3, defPropDefType);
              }
            }
        | K_COMPONENT { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); }
          T_STRING property_type_and_val ';'
            {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("component", $3);
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrCompProp.defiPropType::setPropType($3, defPropDefType);
              }
            }
        | K_ROW { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); }
          T_STRING property_type_and_val ';'
            {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("row", $3);
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrRowProp.defiPropType::setPropType($3, defPropDefType);
              }
            }
        | K_PIN { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); }
          T_STRING property_type_and_val ';'
            {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("pin", $3);
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrPinDefProp.defiPropType::setPropType($3, defPropDefType);
              }
            }
        | K_COMPONENTPIN
          { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); }
          T_STRING property_type_and_val ';'
            {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("componentpin", $3);
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrCompPinProp.defiPropType::setPropType($3, defPropDefType);
              }
            }
        | error ';' { yyerrok; yyclearin;}

property_type_and_val: K_INTEGER { real_num = 0 } opt_range opt_num_val
            {
              if (defrPropCbk) defrProp.defiProp::setPropInteger();
              defPropDefType = 'I';
            }
        | K_REAL { real_num = 1 } opt_range opt_num_val
            {
              if (defrPropCbk) defrProp.defiProp::setPropReal();
              defPropDefType = 'R';
              real_num = 0;
            }
        | K_STRING
            {
              if (defrPropCbk) defrProp.defiProp::setPropString();
              defPropDefType = 'S';
            }
        | K_STRING QSTRING
            {
              if (defrPropCbk) defrProp.defiProp::setPropQString($2);
              defPropDefType = 'Q';
            }
        | K_NAMEMAPSTRING T_STRING
            {
              if (defrPropCbk) defrProp.defiProp::setPropNameMapString($2);
              defPropDefType = 'S';
            }

opt_num_val: /* empty */
        | NUMBER
            { if (defrPropCbk) defrProp.defiProp::setNumber($1); }

units:  K_UNITS K_DISTANCE K_MICRONS NUMBER ';'
	  { CALLBACK(defrUnitsCbk,  defrUnitsCbkType, $4); }

divider_char: K_DIVIDERCHAR QSTRING ';'
	  {
            CALLBACK(defrDividerCbk, defrDividerCbkType, $2);
            hasDivChar = 1;
          }

bus_bit_chars: K_BUSBITCHARS QSTRING ';'
	  { 
            CALLBACK(defrBusBitCbk, defrBusBitCbkType, $2);
            hasBusBit = 1;
          }

site:   K_SITE { dumb_mode = 1; no_num = 1; defrSite.defiSite::clear(); }
        T_STRING NUMBER NUMBER orient
        K_DO NUMBER  K_BY NUMBER  K_STEP NUMBER NUMBER ';' 
	  {
            if (defrSiteCbk) {
              defrSite.defiSite::setName($3);
              defrSite.defiSite::setLocation($4,$5);
              defrSite.defiSite::setOrient($6);
              defrSite.defiSite::setDo($8,$10,$12,$13);
	      CALLBACK(defrSiteCbk, defrSiteCbkType, &(defrSite));
            }
	  }

canplace:   K_CANPLACE {dumb_mode = 1;no_num = 1; } T_STRING NUMBER NUMBER
            orient K_DO NUMBER  K_BY NUMBER  K_STEP NUMBER NUMBER ';' 
            {
              if (defrCanplaceCbk) {
                defrCanplace.defiSite::setName($3);
                defrCanplace.defiSite::setLocation($4,$5);
                defrCanplace.defiSite::setOrient($6);
                defrCanplace.defiSite::setDo($8,$10,$12,$13);
		CALLBACK(defrCanplaceCbk, defrCanplaceCbkType,
		&(defrCanplace));
              }
            }
cannotoccupy:   K_CANNOTOCCUPY {dumb_mode = 1;no_num = 1; } T_STRING NUMBER NUMBER
            orient K_DO NUMBER  K_BY NUMBER  K_STEP NUMBER NUMBER ';' 
            {
              if (defrCannotOccupyCbk) {
                defrCannotOccupy.defiSite::setName($3);
                defrCannotOccupy.defiSite::setLocation($4,$5);
                defrCannotOccupy.defiSite::setOrient($6);
                defrCannotOccupy.defiSite::setDo($8,$10,$12,$13);
		CALLBACK(defrCannotOccupyCbk, defrCannotOccupyCbkType,
                        &(defrCannotOccupy));
              }
            }

orient: K_N    {$$ = 0;}
        | K_W  {$$ = 1;}
        | K_S  {$$ = 2;}
        | K_E  {$$ = 3;}
        | K_FN {$$ = 4;}
        | K_FW {$$ = 5;}
        | K_FS {$$ = 6;}
        | K_FE {$$ = 7;}

die_area: K_DIE_AREA pt pt ';' 
	  {
	    if (defrDieAreaCbk) {
	      defrDieArea.defiBox::setLowerLeft($2.x,$2.y);
	      defrDieArea.defiBox::setUpperRight($3.x,$3.y);
	      CALLBACK(defrDieAreaCbk, defrDieAreaCbkType, &(defrDieArea));
	    }
	  }

/* 8/31/2001 - Wanda da Rosa. This is obsolete in 5.4 */
pin_cap_rule: start_def_cap pin_caps end_def_cap
            { }

start_def_cap: K_DEFAULTCAP NUMBER 
	{
          if (defVersionNum < 5.4) {
             CALLBACK(defrDefaultCapCbk, defrDefaultCapCbkType, ROUND($2));
          } else {
             yydefwarning("DEFAULTCAP is obsolete in 5.4. It will be ignored.");
          }
        }

pin_caps: /* empty */
        | pin_caps pin_cap
            ;

pin_cap: K_MINPINS NUMBER K_WIRECAP NUMBER ';'
	  {
            if (defVersionNum < 5.4) {
	      if (defrPinCapCbk) {
	        defrPinCap.defiPinCap::setPin(ROUND($2));
	        defrPinCap.defiPinCap::setCap($4);
	        CALLBACK(defrPinCapCbk, defrPinCapCbkType, &(defrPinCap));
	      }
            }
	  }

end_def_cap: K_END K_DEFAULTCAP 
            { }

pin_rule: start_pins pins end_pins
            { }

start_pins: K_PINS NUMBER ';'
	  { CALLBACK(defrStartPinsCbk, defrStartPinsCbkType, ROUND($2)); }

pins: /* empty */
        | pins pin
            ;

pin:        '-' {dumb_mode = 1;no_num = 1; } T_STRING '+' K_NET
	{dumb_mode = 1; no_num = 1; } T_STRING
          {
            if (defrPinCbk || defrPinExtCbk) {
              defrPin.defiPin::Setup($3, $7);
            }
          }
        pin_options ';'
          { CALLBACK(defrPinCbk, defrPinCbkType, &defrPin); }

pin_options: /* empty */
        | pin_options pin_option

pin_option: '+' K_SPECIAL
          {
            if (defrPinCbk)
              defrPin.defiPin::setSpecial();
          }

        |   extension_stmt
          { CALLBACK(defrPinExtCbk, defrPinExtCbkType, History_text); }

        |   '+' K_DIRECTION T_STRING
            {
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::setDirection($3);
            }

        |   '+' K_USE use_type
            {
              if (defrPinCbk || defrPinExtCbk) defrPin.defiPin::setUse($3);
            }

        |   '+' K_LAYER { dumb_mode = 1; } T_STRING pt pt
            {
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::setLayer($4,$5.x,$5.y,$6.x,$6.y);
            }
  
        |   placement_status pt orient
            {
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::setPlacement($1, $2.x, $2.y, $3);
            }

        /* The following is 5.4 syntax */
        |   '+' K_ANTENNAPINPARTIALMETALAREA NUMBER pin_layer_opt
            {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINPARTIALMETALAREA is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinPartialMetalArea((int)$3, $4); 
            }
        |   '+' K_ANTENNAPINPARTIALMETALSIDEAREA NUMBER pin_layer_opt
            {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINPARTIALMETALSIDEAREA is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinPartialMetalSideArea((int)$3, $4); 
            }
        |   '+' K_ANTENNAPINGATEAREA NUMBER pin_layer_opt
            {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINGATEAREA is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinGateArea((int)$3, $4); 
            }
        |   '+' K_ANTENNAPINDIFFAREA NUMBER pin_layer_opt
            {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINDIFFAREA is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinDiffArea((int)$3, $4); 
            }
        |   '+' K_ANTENNAPINMAXAREACAR NUMBER K_LAYER {dumb_mode=1;} T_STRING
            {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINMAXAREACAR is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinMaxAreaCar((int)$3, $6); 
            }
        |   '+' K_ANTENNAPINMAXSIDEAREACAR NUMBER K_LAYER {dumb_mode=1;}
             T_STRING
            {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINMAXSIDEAREACAR is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinMaxSideAreaCar((int)$3, $6); 
            }
        |   '+' K_ANTENNAPINPARTIALCUTAREA NUMBER pin_layer_opt
            {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINPARTIALCUTAREA is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinPartialCutArea((int)$3, $4); 
            }
        |   '+' K_ANTENNAPINMAXCUTCAR NUMBER K_LAYER {dumb_mode=1;} T_STRING
            {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINMAXCUTCAR is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinMaxCutCar((int)$3, $6); 
            }
        | '+' K_ANTENNAMODEL pin_oxide
            {  /* 5.5 syntax */
              if (defVersionNum < 5.5) {
                yyerror("ANTENNAMODEL is a 5.5 syntax. Your def file is not defined as 5.5");
                CHKERR();
              }
            }

pin_oxide: K_OXIDE1
            { aOxide = 1;
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAntennaModel(aOxide);
            }
        | K_OXIDE2
            { aOxide = 2;
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAntennaModel(aOxide);
            }
        | K_OXIDE3
            { aOxide = 3;
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAntennaModel(aOxide);
            }
        | K_OXIDE4
            { aOxide = 4;
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAntennaModel(aOxide);
            }

use_type: K_SIGNAL
            { $$ = (char*)"SIGNAL"; }
        | K_POWER
            { $$ = (char*)"POWER"; }
        | K_GROUND
            { $$ = (char*)"GROUND"; }
        | K_CLOCK
            { $$ = (char*)"CLOCK"; }
        | K_TIEOFF
            { $$ = (char*)"TIEOFF"; }
        | K_ANALOG
            { $$ = (char*)"ANALOG"; }
        | K_SCAN
            { $$ = (char*)"SCAN"; }
        | K_RESET
            { $$ = (char*)"RESET"; }

pin_layer_opt:
        /* empty */
          { $$ = (char*)""; }
        | K_LAYER {dumb_mode=1;} T_STRING
          { $$ = $3; }

end_pins: K_END K_PINS
	{ CALLBACK(defrPinEndCbk, defrPinEndCbkType, 0); }

row_rule: K_ROW {dumb_mode = 2; no_num = 2; } T_STRING T_STRING NUMBER NUMBER
      orient K_DO NUMBER K_BY NUMBER K_STEP NUMBER NUMBER
        {
          /* 06/05/2002 - Wanda da Rosa - pcr 448455 */
          /* Check for 1 and 0 in the correct position */
          /* 07/26/2002 - Wanda da Rosa - Commented out due to pcr 459218
          if ((($11 == 1) && ($14 != 0)) ||
              (($11 != 1) && ($14 == 0))) {
            yyerror("Syntax is \"DO numX BY 1 STEP spaceX 0\"");
            CHKERR();
          }
          if ((($9 == 1) && ($13 != 0)) ||
              (($9 != 1) && ($13 == 0))) {
            yyerror("Syntax is \"DO 1 BY numY STEP 0 spaceY\"");
            CHKERR();
          }
          */
          /* pcr 459218 - Error if at least numX or numY does not equal 1 */
          if (($9 != 1) && ($11 != 1)) {
            yyerror("Syntax error, either \"DO 1 BY num... or DO numX BY 1...\"");
            CHKERR();
          }
          if (defrRowCbk) {
            defrRow.defiRow::setup($3, $4, $5, $6, $7);
            defrRow.defiRow::setDo(ROUND($9), ROUND($11), $13, $14);
          }
        }
      row_options ';'
        {
          if (defrRowCbk) {
	    CALLBACK(defrRowCbk, defrRowCbkType, &defrRow);
          }
        }

row_options: /* empty */
      | row_options row_option
      ;

row_option : '+' K_PROPERTY { dumb_mode = 10000000; } row_prop_list
         { dumb_mode = 0; }

row_prop_list : /* empty */
       | row_prop_list row_prop
       ;
       
row_prop : T_STRING NUMBER
        {
          if (defrRowCbk) {
             char propTp;
             char* str = ringCopy("                       ");
             propTp =  defrRowProp.defiPropType::propType($1);
             /* For backword compatibility, also set the string value */
             /* We will use a temporary string to store the number.
              * The string space is borrowed from the ring buffer
              * in the lexer. */
             sprintf(str, "%g", $2);
             defrRow.defiRow::addNumProperty($1, $2, str, propTp);
          }
        }
      | T_STRING QSTRING
        {
          if (defrRowCbk) {
             char propTp;
             propTp =  defrRowProp.defiPropType::propType($1);
             defrRow.defiRow::addProperty($1, $2, propTp);
          }
        }
      | T_STRING T_STRING
        {
          if (defrRowCbk) {
             char propTp;
             propTp =  defrRowProp.defiPropType::propType($1);
             defrRow.defiRow::addProperty($1, $2, propTp);
          }
        }

tracks_rule: track_start NUMBER
        {
          if (defrTrackCbk) {
            defrTrack.defiTrack::setup($1);
          }
        }
        K_DO NUMBER K_STEP NUMBER track_layer_statement ';'
        {
          if ($5 <= 0)
             yyerror("Invalid numtracks.");
          if ($7 < 0)
             yyerror("Invalid space.");
          if (defrTrackCbk) {
            defrTrack.defiTrack::setDo(ROUND($2), ROUND($5), $7);
	    CALLBACK(defrTrackCbk, defrTrackCbkType, &defrTrack);
          }
        }

track_start: K_TRACKS track_type
        {
          $$ = $2;
        }

track_type: K_X
            { $$ = (char*)"X";}
        | K_Y
            { $$ = (char*)"Y";}

track_layer_statement: /* empty */
        | K_LAYER { dumb_mode = 1000; } track_layer track_layers
            { dumb_mode = 0; }

track_layers: /* empty */
        | track_layer track_layers 
            ;

track_layer: T_STRING
        {
          if (defrTrackCbk)
            defrTrack.defiTrack::addLayer($1);
        }

gcellgrid: K_GCELLGRID track_type NUMBER 
     K_DO NUMBER K_STEP NUMBER ';'
	{
          if ($5 <= 0)
             yyerror("Invalid numColumns or numRows.");
          if ($7 < 0)
             yyerror("Invalid space.");
	  if (defrGcellGridCbk) {
	    defrGcellGrid.defiGcellGrid::setup($2, ROUND($3), ROUND($5), $7);
	    CALLBACK(defrGcellGridCbk, defrGcellGridCbkType, &defrGcellGrid);
	  }
	}

extension_section: K_BEGINEXT
        { }

extension_stmt: '+' K_BEGINEXT
        { }

via_section: via via_declarations via_end
            ;
        
via: K_VIAS NUMBER ';' 
	  { CALLBACK(defrViaStartCbk, defrViaStartCbkType, ROUND($2)); }

via_declarations: /* empty */
        | via_declarations via_declaration
            ;

via_declaration: '-' {dumb_mode = 1;no_num = 1; } T_STRING
            { if (defrViaCbk) defrVia.defiVia::setup($3); }
        layer_stmts ';'
            { CALLBACK(defrViaCbk, defrViaCbkType, &defrVia); }

layer_stmts: /* empty */
        | layer_stmts layer_stmt
            ;

layer_stmt: '+' K_RECT {dumb_mode = 1;no_num = 1; } T_STRING pt pt 
            {
              if (defrViaCbk)
                defrVia.defiVia::addLayer($4, $5.x, $5.y, $6.x, $6.y);
            }
        | '+' K_PATTERNNAME {dumb_mode = 1;no_num = 1; } T_STRING
            {
              if (defrViaCbk)
                defrVia.defiVia::addPattern($4);
            }
        | extension_stmt
          { CALLBACK(defrViaExtCbk, defrViaExtCbkType, History_text); }


pt : '(' NUMBER NUMBER ')'
            {
            save_x = $2;
            save_y = $3;
            $$.x = ROUND($2);
            $$.y = ROUND($3);
            }
        | '(' '*' NUMBER ')'
            {
            save_y = $3;
            $$.x = ROUND(save_x);
            $$.y = ROUND($3);
            }
        | '(' NUMBER '*' ')'
            {
            save_x = $2;
            $$.x = ROUND($2);
            $$.y = ROUND(save_y);
            }
        | '(' '*' '*' ')'
            {
            $$.x = ROUND(save_x);
            $$.y = ROUND(save_y);
            }

via_end: K_END K_VIAS
	{ CALLBACK(defrViaEndCbk, defrViaEndCbkType, 0); }

regions_section: regions_start regions_stmts K_END K_REGIONS
	{ CALLBACK(defrRegionEndCbk, defrRegionEndCbkType, 0); }

regions_start: K_REGIONS NUMBER ';'
	{ CALLBACK(defrRegionStartCbk, defrRegionStartCbkType, ROUND($2)); }

regions_stmts: /* empty */
        | regions_stmts regions_stmt
            {}

regions_stmt: '-' { dumb_mode = 1; no_num = 1; } T_STRING
        {
          if (defrRegionCbk)
             defrRegion.defiRegion::setup($3);
          regTypeDef = 0;
        }
     rect_list region_options ';'
        { CALLBACK(defrRegionCbk, defrRegionCbkType, &defrRegion); }

rect_list :
      pt pt
        { if (defrRegionCbk)
	  defrRegion.defiRegion::addRect($1.x, $1.y, $2.x, $2.y); }
      | rect_list pt pt
        { if (defrRegionCbk)
	  defrRegion.defiRegion::addRect($2.x, $2.y, $3.x, $3.y); }
      ;

region_options: /* empty */
      | region_options region_option
      ;

region_option : '+' K_PROPERTY { dumb_mode = 10000000; } region_prop_list
         { dumb_mode = 0; }
      | '+' K_TYPE region_type      /* 5.4.1 */
         {
           if (regTypeDef) {
              yyerror("TYPE is already defined");
              CHKERR();
           }
           if (defrRegionCbk) defrRegion.defiRegion::setType($3);
           regTypeDef = 1;
         }
      ;

region_prop_list : /* empty */
       | region_prop_list region_prop
       ;
       
region_prop : T_STRING NUMBER
        {
          if (defrRegionCbk) {
             char propTp;
             char* str = ringCopy("                       ");
             propTp = defrRegionProp.defiPropType::propType($1);
             /* For backword compatibility, also set the string value */
             /* We will use a temporary string to store the number.
              * The string space is borrowed from the ring buffer
              * in the lexer. */
             sprintf(str, "%g", $2);
             defrRegion.defiRegion::addNumProperty($1, $2, str, propTp);
          }
        }
      | T_STRING QSTRING
        {
          if (defrRegionCbk) {
             char propTp;
             propTp = defrRegionProp.defiPropType::propType($1);
             defrRegion.defiRegion::addProperty($1, $2, propTp);
          }
        }
      | T_STRING T_STRING
        {
          if (defrRegionCbk) {
             char propTp;
             propTp = defrRegionProp.defiPropType::propType($1);
             defrRegion.defiRegion::addProperty($1, $2, propTp);
          }
        }

region_type: K_FENCE
            { $$ = (char*)"FENCE"; }
      | K_GUIDE
            { $$ = (char*)"GUIDE"; }

comps_section: start_comps comps_rule end_comps
            ;

start_comps: K_COMPS NUMBER ';'
  { CALLBACK(defrComponentStartCbk, defrComponentStartCbkType, ROUND($2)); }

comps_rule: /* empty */
        | comps_rule comp
            ;

comp: comp_start comp_options ';'
	{ CALLBACK(defrComponentCbk, defrComponentCbkType, &defrComponent); }

comp_start: comp_id_and_name comp_net_list
            {
              dumb_mode = 0;
	      no_num = 0;
              /* The net connections were added to the array in
               * reverse order so fix the order now */
              /* Do not need to reverse the order since the storing is
                 in sequence now  08/07/2001 - Wanda da Rosa
              if (defrComponentCbk)
                defrComponent.defiComponent::reverseNetOrder();
              */
            }

comp_id_and_name: '-' {dumb_mode = 1000000000; no_num = 10000000; }
       T_STRING T_STRING
            {
            if (defrComponentCbk)
              defrComponent.defiComponent::IdAndName($3, $4);
            }

comp_net_list: /* empty */
        { }
        | comp_net_list '*'
            {
              if (defrComponentCbk)
                defrComponent.defiComponent::addNet("*");
            }
        | comp_net_list T_STRING
            {
              if (defrComponentCbk)
                defrComponent.defiComponent::addNet($2);
            }

comp_options: /* empty */
        |     comp_options comp_option
            ;

comp_option:  comp_generate | comp_source | comp_type | weight |
              comp_foreign | comp_region | comp_eeq |
              comp_property | comp_extension_stmt
            ;

comp_extension_stmt: extension_stmt
    { CALLBACK(defrComponentExtCbk, defrComponentExtCbkType, History_text); }

comp_eeq: '+' K_EEQMASTER {dumb_mode=1; no_num = 1; } T_STRING
            {
              if (defrComponentCbk)
                defrComponent.defiComponent::setEEQ($4);
            }

comp_generate: '+' K_COMP_GEN { dumb_mode = 2;  no_num = 2; } T_STRING
    opt_pattern
            {
              if (defrComponentCbk)
                defrComponent.defiComponent::setGenerate($4, $5);
            }
opt_pattern :
    /* empty */
      { $$ = (char*)""; }
    | T_STRING
      { $$ = $1; }

comp_source: '+' K_SOURCE source_type 
            {
              if (defrComponentCbk)
                defrComponent.defiComponent::setSource($3);
            }

source_type: K_NETLIST
            { $$ = (char*)"NETLIST"; }
        | K_DIST
            { $$ = (char*)"DIST"; }
        | K_USER
            { $$ = (char*)"USER"; }
        | K_TIMING
            { $$ = (char*)"TIMING"; }


comp_region:
	comp_region_start comp_pnt_list
	  { }
        | comp_region_start T_STRING 
	  {
	    if (defrComponentCbk)
	      defrComponent.defiComponent::setRegionName($2);
	  }

comp_pnt_list: pt pt
	{ 
          /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
          if (defVersionNum < 5.5) {
            if (defrComponentCbk)
	       defrComponent.defiComponent::setRegionBounds($1.x, $1.y, $2.x, $2.y);
          }
	} 
    | comp_pnt_list  pt pt
	{ 
          /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
          if (defVersionNum < 5.5) {
	    if (defrComponentCbk)
	       defrComponent.defiComponent::setRegionBounds($2.x, $2.y, $3.x, $3.y);
          }
	} 

comp_property: '+' K_PROPERTY { dumb_mode = 10000000; }
      comp_prop_list
      { dumb_mode = 0; }

comp_prop_list: comp_prop
    | comp_prop_list comp_prop
          ;

comp_prop: T_STRING NUMBER
            {
              if (defrComponentCbk) {
                char propTp;
                char* str = ringCopy("                       ");
                propTp = defrCompProp.defiPropType::propType($1);
                /* For backword compatibility, also set the string value */
                /* We will use a temporary string to store the number.
                 * The string space is borrowed from the ring buffer
                 * in the lexer. */
                sprintf(str, "%g", $2);
                defrComponent.defiComponent::addNumProperty($1, $2, str, propTp);
              }
            }
         | T_STRING QSTRING
            {
              if (defrComponentCbk) {
                char propTp;
                propTp = defrCompProp.defiPropType::propType($1);
                defrComponent.defiComponent::addProperty($1, $2, propTp);
              }
            }
         | T_STRING T_STRING
            {
              if (defrComponentCbk) {
                char propTp;
                propTp = defrCompProp.defiPropType::propType($1);
                defrComponent.defiComponent::addProperty($1, $2, propTp);
              }
            }

comp_region_start: '+' K_REGION
            { dumb_mode = 1; no_num = 1; }

comp_foreign: '+' K_FOREIGN { dumb_mode = 1; no_num = 1; } T_STRING
        opt_paren orient
            { 
              if (defrComponentCbk) {
                defrComponent.defiComponent::setForeignName($4);
                defrComponent.defiComponent::setForeignLocation(
                $5.x, $5.y, $6);
              }
             }

opt_paren:
       pt
	 { $$ = $1; }
       | NUMBER NUMBER
	 { $$.x = ROUND($1); $$.y = ROUND($2); }

comp_type: placement_status pt orient
            {
              if (defrComponentCbk) {
                defrComponent.defiComponent::setPlacementStatus($1);
                defrComponent.defiComponent::setPlacementLocation(
                $2.x, $2.y, $3);
              }
            }
        | '+' K_UNPLACED
            {
              if (defrComponentCbk)
                defrComponent.defiComponent::setPlacementStatus(
                                             DEFI_COMPONENT_UNPLACED);
                defrComponent.defiComponent::setPlacementLocation(
                                             -1, -1, -1);
            }
        | '+' K_UNPLACED pt orient
            {
              if (defVersionNum < 5.4) {   /* PCR 495463 */
                if (defrComponentCbk) {
                  defrComponent.defiComponent::setPlacementStatus(
                                              DEFI_COMPONENT_UNPLACED);
                  defrComponent.defiComponent::setPlacementLocation(
                                              $3.x, $3.y, $4);
                }
              } else {
                yydefwarning("pt & orient are not allow in 5.4 and later, they are ignored.");
                if (defrComponentCbk)
                  defrComponent.defiComponent::setPlacementStatus(
                                               DEFI_COMPONENT_UNPLACED);
                  defrComponent.defiComponent::setPlacementLocation(
                                               -1, -1, -1);
              }
            }

placement_status: '+' K_FIXED 
            { $$ = DEFI_COMPONENT_FIXED; }
        | '+' K_COVER 
            { $$ = DEFI_COMPONENT_COVER; }
        | '+' K_PLACED
            { $$ = DEFI_COMPONENT_PLACED; }

weight: '+' K_WEIGHT NUMBER 
            {
              if (defrComponentCbk)
                defrComponent.defiComponent::setWeight(ROUND($3));
            }

end_comps: K_END K_COMPS
	  { CALLBACK(defrComponentEndCbk, defrComponentEndCbkType, 0); }

nets_section:  start_nets net_rules end_nets
            ;

start_nets: K_NETS NUMBER ';'
	{ CALLBACK(defrNetStartCbk, defrNetStartCbkType, ROUND($2)); }

net_rules: /* empty */
        | net_rules one_net
            ;

one_net: net_and_connections net_options ';'
	{ CALLBACK(defrNetCbk, defrNetCbkType, &defrNet); }
/*
** net_and_connections: net_start {dumb_mode = 1000000000; no_num = 10000000;}
**                      net_connections
** wmd -- this can be used to replace
**        | '(' K_PIN {dumb_mode = 1; no_num = 1;} T_STRING conn_opt ')' (???)
*/
net_and_connections: net_start
            {dumb_mode = 0; no_num = 0; }

/* pcr 235555 & 236210 */
net_start: '-' {dumb_mode = 1000000000; no_num = 10000000; nondef_is_keyword = TRUE; mustjoin_is_keyword = TRUE;} net_name 

net_name: T_STRING
            {
              /* 9/22/1999 -- Wanda da Rosa */
              /* this is shared by both net and special net */
              if (defrNetCbk || defrSNetCbk)
                defrNet.defiNet::setName($1);
            } net_connections
        | K_MUSTJOIN '(' T_STRING {dumb_mode = 1; no_num = 1;} T_STRING ')'
            {
            if (defrNetCbk || defrSNetCbk)
              defrNet.defiNet::addMustPin($3, $5, 0);
            dumb_mode = 3;
            no_num = 3;
            }

net_connections: /* empty */
        | net_connections net_connection 
            ;

net_connection: '(' T_STRING {dumb_mode = 1000000000; no_num = 10000000;} T_STRING conn_opt ')'
            {
            /* 9/22/1999 -- Wanda da Rosa */
            /* since the code is shared by both net & special net, */
            /* need to check on both flags */
            if (defrNetCbk || defrSNetCbk)
              defrNet.defiNet::addPin($2, $4, $5);
            /* 1/14/2000 -- Wanda da Rosa, pcr 289156 */
            /* reset dumb_mode & no_num to 3 , just in case */
            /* the next statement is another net_connection */
            dumb_mode = 3;
            no_num = 3;
            }
        | '(' '*' {dumb_mode = 1; no_num = 1;} T_STRING conn_opt ')'
            {
            if (defrNetCbk || defrSNetCbk)
              defrNet.defiNet::addPin("*", $4, $5);
            dumb_mode = 3;
            no_num = 3;
            }
        | '(' K_PIN {dumb_mode = 1; no_num = 1;} T_STRING conn_opt ')'
            {
            if (defrNetCbk || defrSNetCbk)
              defrNet.defiNet::addPin("PIN", $4, $5);
            dumb_mode = 3;
            no_num = 3;
            }

conn_opt: /* empty */
          { $$ = 0; }
        | extension_stmt
          {
	    CALLBACK(defrNetConnectionExtCbk, defrNetConnectionExtCbkType,
              History_text);
	    $$ = 0; }
        | '+' K_SYNTHESIZED
	  { $$ = 1; }

/* These are all the optional fields for a net that go after the '+' */
net_options: /* empty */
        | net_options net_option
            ;

net_option: '+' net_type { dumb_mode = 1; }
            {  
            if (defrNetCbk) defrNet.defiNet::addWire($2, NULL);
            }
            paths
            {
            by_is_keyword = FALSE;
            do_is_keyword = FALSE;
            new_is_keyword = FALSE;
            nondef_is_keyword = FALSE;
            mustjoin_is_keyword = FALSE;
            step_is_keyword = FALSE;
            }
        | '+' K_SOURCE netsource_type
            { if (defrNetCbk) defrNet.defiNet::setSource($3); }

        | '+' K_FIXEDBUMP
            {
              if (defVersionNum < 5.5) {
                yyerror("FIXEDBUMP is a 5.5 syntax. Your def file is not defined as 5.5");
                CHKERR();
              }
              if (defrNetCbk) defrNet.defiNet::setFixedbump();
            } 

        | '+' K_FREQUENCY NUMBER
            {
              if (defVersionNum < 5.5) {
                yyerror("FIXEDBUMP is a 5.5 syntax. Your def file is not defined as 5.5");
                CHKERR();
              }
              if (defrNetCbk) defrNet.defiNet::setFrequency($3);
            }

        | '+' K_ORIGINAL {dumb_mode = 1; no_num = 1;} T_STRING
            { if (defrNetCbk) defrNet.defiNet::setOriginal($4); }

        | '+' K_PATTERN pattern_type
            { if (defrNetCbk) defrNet.defiNet::setPattern($3); }

        | '+' K_WEIGHT NUMBER
            { if (defrNetCbk) defrNet.defiNet::setWeight(ROUND($3)); }

        | '+' K_XTALK NUMBER
            { if (defrNetCbk) defrNet.defiNet::setXTalk(ROUND($3)); }

        | '+' K_ESTCAP NUMBER
            { if (defrNetCbk) defrNet.defiNet::setCap($3); }

        | '+' K_USE use_type 
            { if (defrNetCbk) defrNet.defiNet::setUse($3); }

        | '+' K_STYLE { dumb_mode = 1; no_num = 1; } T_STRING
            { if (defrNetCbk) defrNet.defiNet::setStyle($4); }

        | '+' K_NONDEFAULTRULE { dumb_mode = 1; no_num = 1; } T_STRING
            { if (defrNetCbk) defrNet.defiNet::setNonDefaultRule($4); }

        | vpin_stmt

        | '+' K_SHIELDNET { dumb_mode = 1; no_num = 1; } T_STRING
            { if (defrNetCbk) defrNet.defiNet::addShieldNet($4); }

        | '+' K_NOSHIELD { dumb_mode = 1; no_num = 1; }
            {  /* since the parser still support 5.3 and earlier, can't */
               /* move NOSHIELD in net_type */
              if (defVersionNum < 5.4) {   /* PCR 445209 */
                if (defrNetCbk) defrNet.defiNet::addNoShield("");
                by_is_keyword = FALSE;
                do_is_keyword = FALSE;
                new_is_keyword = FALSE;
                step_is_keyword = FALSE;
                shield = TRUE;    /* save the path info in the shield paths */
              } else
                if (defrNetCbk) defrNet.defiNet::addWire("NOSHIELD", NULL);
            }
            paths
            {
              if (defVersionNum < 5.4)   /* PCR 445209 */
                shield = FALSE;
              else {
                by_is_keyword = FALSE;
                do_is_keyword = FALSE;
                new_is_keyword = FALSE;
                step_is_keyword = FALSE;
                nondef_is_keyword = FALSE;
                mustjoin_is_keyword = FALSE;
              }
            }

        | '+' K_SUBNET { dumb_mode = 1; no_num = 1;
                         if (defrNetCbk) {
                           defrSubnet =
                           (defiSubnet*)malloc(sizeof(defiSubnet));
                           defrSubnet->defiSubnet::Init();
                         }
                       }
            T_STRING {
               if (defrNetCbk) {
                 defrSubnet->defiSubnet::setName($4);
               }
            } 
            comp_names subnet_options {
               if (defrNetCbk) {
                 defrNet.defiNet::addSubnet(defrSubnet);
                 defrSubnet = 0;
               }
            }

        | '+' K_PROPERTY {dumb_mode = 10000000;} net_prop_list
            { dumb_mode = 0; }

        | extension_stmt
          { CALLBACK(defrNetExtCbk, defrNetExtCbkType, History_text); }

net_prop_list: net_prop
      | net_prop_list net_prop
      ;

net_prop: T_STRING NUMBER
            {
              if (defrNetCbk) {
                char propTp;
                char* str = ringCopy("                       ");
                propTp = defrNetProp.defiPropType::propType($1);
                /* For backword compatibility, also set the string value */
                /* We will use a temporary string to store the number.
                 * The string space is borrowed from the ring buffer
                 * in the lexer. */
                sprintf(str, "%g", $2);
                defrNet.defiNet::addNumProp($1, $2, str, propTp);
              }
            }
        | T_STRING QSTRING
            {
              if (defrNetCbk) {
                char propTp;
                propTp = defrNetProp.defiPropType::propType($1);
                defrNet.defiNet::addProp($1, $2, propTp);
              }
            }
        | T_STRING T_STRING
            {
              if (defrNetCbk) {
                char propTp;
                propTp = defrNetProp.defiPropType::propType($1);
                defrNet.defiNet::addProp($1, $2, propTp);
              }
            }

netsource_type: K_NETLIST
            { $$ = (char*)"NETLIST"; }
        | K_DIST
            { $$ = (char*)"DIST"; }
        | K_USER
            { $$ = (char*)"USER"; }
        | K_TIMING
            { $$ = (char*)"TIMING"; }
        | K_TEST
            { $$ = (char*)"TEST"; }

vpin_stmt: vpin_begin vpin_layer_opt pt pt vpin_options
      { if (defrNetCbk)
	defrNet.defiNet::addVpinBounds($3.x, $3.y, $4.x, $4.y); }

vpin_begin: '+' K_VPIN {dumb_mode = 1; no_num = 1;} T_STRING
	  { if (defrNetCbk) defrNet.defiNet::addVpin($4); }

vpin_layer_opt: /* empty */
        | K_LAYER {dumb_mode=1;} T_STRING
	  { if (defrNetCbk) defrNet.defiNet::addVpinLayer($3); }

vpin_options: /* empty */
        | vpin_status pt orient
      { if (defrNetCbk) defrNet.defiNet::addVpinLoc($1, $2.x, $2.y, $3); }

vpin_status: K_PLACED 
            { $$ = (char*)"PLACED"; }
        | K_FIXED 
            { $$ = (char*)"FIXED"; }
        | K_COVER
            { $$ = (char*)"COVER"; }

net_type: K_FIXED
            { $$ = (char*)"FIXED"; }
        | K_COVER
            { $$ = (char*)"COVER"; }
        | K_ROUTED
            { $$ = (char*)"ROUTED"; }

paths:
    path
      { if (defrNeedPathData) pathIsDone(shield); }
    | paths new_path
      { }

new_path: K_NEW { dumb_mode = 1; } path
      { if (defrNeedPathData) pathIsDone(shield); }

path:  T_STRING
      {
        /* 9/26/2002 - Wanda da Rosa - pcr 449514
           Check if $1 is equal to TAPER or TAPERRULE, layername
           is missing */
        if ((strcmp($1, "TAPER") == 0) || (strcmp($1, "TAPERRULE") == 0)) {
           yyerror("layerName is missing");
           CHKERR();
           /* Since there is already error, the next token is insignificant */
           dumb_mode = 1; no_num = 1;
        } else {
           if (defrNeedPathData) defrPath->defiPath::addLayer($1);
           dumb_mode = 0; no_num = 0;
        }
      }
    opt_width opt_shape opt_taper path_pt
/* WANDA */
      { dumb_mode = 1000000000; by_is_keyword = TRUE; do_is_keyword = TRUE;
        new_is_keyword = TRUE; step_is_keyword = TRUE; }
    path_item_list
      { dumb_mode = 0; }

path_item_list: /* empty */
    | path_item_list path_item
    ;

path_item:
    T_STRING
      { if (defrNeedPathData) defrPath->defiPath::addVia($1); }
    | T_STRING orient
      { if (defrNeedPathData) {
            defrPath->defiPath::addVia($1);
            defrPath->defiPath::addViaRotation($2);
        }
      }
    | T_STRING K_DO NUMBER K_BY NUMBER K_STEP NUMBER NUMBER
      {
        if (defVersionNum < 5.5) {
          yyerror("VIA DO is a 5.5 syntax. Your def file is not defined as 5.5");
           CHKERR();
        }
        if (($3 == 0) || ($5 == 0)) {
          yyerror("In the VIA DO construct, neither numX nor numY can be 0");
          CHKERR();
        }
        if (defrNeedPathData) {
            defrPath->defiPath::addVia($1);
            defrPath->defiPath::addViaData($3, $5, $7, $8);
        }
      }
    | path_pt
      { }


path_pt :
    '(' NUMBER NUMBER ')'
      {
	if (defrNeedPathData)
	  defrPath->defiPath::addPoint(ROUND($2), ROUND($3)); 
	save_x = $2;
	save_y = $3;
      }
    | '(' '*' NUMBER ')'
      {
	if (defrNeedPathData)
	  defrPath->defiPath::addPoint(ROUND(save_x), ROUND($3)); 
	save_y = $3;
      }
    | '(' NUMBER '*' ')'
      {
	if (defrNeedPathData)
	  defrPath->defiPath::addPoint(ROUND($2), ROUND(save_y)); 
	save_x = $2;
      }
    | '(' '*' '*' ')'
      {
	if (defrNeedPathData)
	  defrPath->defiPath::addPoint(ROUND(save_x), ROUND(save_y)); 
      }
    | '(' NUMBER NUMBER NUMBER ')'
      {
	if (defrNeedPathData)
	  defrPath->defiPath::addFlushPoint(ROUND($2), ROUND($3), ROUND($4)); 
	save_x = $2;
	save_y = $3;
      }
    | '(' '*' NUMBER NUMBER ')'
      {
	if (defrNeedPathData)
	  defrPath->defiPath::addFlushPoint(ROUND(save_x), ROUND($3),
	  ROUND($4)); 
	save_y = $3;
      }
    | '(' NUMBER '*' NUMBER ')'
      {
	if (defrNeedPathData)
	  defrPath->defiPath::addFlushPoint(ROUND($2), ROUND(save_y),
	  ROUND($4)); 
	save_x = $2;
      }
    | '(' '*' '*' NUMBER ')'
      {
	if (defrNeedPathData)
	  defrPath->defiPath::addFlushPoint(ROUND(save_x), ROUND(save_y),
	  ROUND($4)); 
      }

opt_taper:
    /* empty */
      { }
    | K_TAPER
      { if (defrNeedPathData) defrPath->defiPath::setTaper(); }
    | K_TAPERRULE { dumb_mode = 1; } T_STRING
      { if (defrNeedPathData) defrPath->defiPath::addTaperRule($3); }

opt_shape:
    /* empty */
      { }
    | '+' K_SHAPE shape_type
      { if (defrNeedPathData) defrPath->defiPath::addShape($3); }

opt_width:
    /* empty */
      { }
    | NUMBER
      { if (defrNeedPathData) defrPath->defiPath::addWidth(ROUND($1)); }


end_nets: K_END K_NETS 
	  { CALLBACK(defrNetEndCbk, defrNetEndCbkType, 0); }

shape_type: K_RING
            { $$ = (char*)"RING"; }
        | K_STRIPE
            { $$ = (char*)"STRIPE"; }
        | K_FOLLOWPIN
            { $$ = (char*)"FOLLOWPIN"; }
        | K_IOWIRE
            { $$ = (char*)"IOWIRE"; }
        | K_COREWIRE
            { $$ = (char*)"COREWIRE"; }
        | K_BLOCKWIRE
            { $$ = (char*)"BLOCKWIRE"; }
        | K_FILLWIRE
            { $$ = (char*)"FILLWIRE"; }
        | K_DRCFILL
            { $$ = (char*)"DRCFILL"; }
        | K_BLOCKAGEWIRE
            { $$ = (char*)"BLOCKAGEWIRE"; }
        | K_PADRING
            { $$ = (char*)"PADRING"; }
        | K_BLOCKRING
            { $$ = (char*)"BLOCKRING"; }

snets_section :  start_snets snet_rules end_snets
            ;

snet_rules: /* empty */
        | snet_rules snet_rule
            ;

snet_rule: net_and_connections snet_options ';'
	{ CALLBACK(defrSNetCbk, defrSNetCbkType, &defrNet); }

snet_options: /* empty */
        | snet_options snet_option
            ;

snet_option: snet_width | snet_voltage | 
             snet_spacing | snet_other_option
             ;

snet_other_option: '+' net_type { dumb_mode = 1; }
            {
            if (defrNetCbk) defrNet.defiNet::addWire($2, NULL);
            }
            spaths
            {
            by_is_keyword = FALSE;
            do_is_keyword = FALSE;
            new_is_keyword = FALSE;
            step_is_keyword = FALSE;
            }
 
        | '+' K_SHIELD { dumb_mode = 1; no_num = 1; } T_STRING
            { /* since the parser still supports 5.3 and earlier, */
              /* can't just move SHIELD in net_type */
              if (defVersionNum < 5.4) { /* PCR 445209 */
                if (defrNetCbk) defrNet.defiNet::addShield($4);
                by_is_keyword = FALSE;
                do_is_keyword = FALSE;
                new_is_keyword = FALSE;
                step_is_keyword = FALSE;
                shield = TRUE;   /* save the path info in the shield paths */
              } else
                if (defrNetCbk) defrNet.defiNet::addWire("SHIELD", $4);
            }
            spaths
            {
              if (defVersionNum < 5.4)   /* PCR 445209 */
                shield = FALSE;
              else
                by_is_keyword = FALSE;
                do_is_keyword = FALSE;
                new_is_keyword = FALSE;
                step_is_keyword = FALSE;
            }
 
        | '+' K_SOURCE source_type
            { if (defrNetCbk) defrNet.defiNet::setSource($3); }

        | '+' K_FIXEDBUMP
            { if (defrNetCbk) defrNet.defiNet::setFixedbump(); }
 
        | '+' K_FREQUENCY NUMBER
            { if (defrNetCbk) defrNet.defiNet::setFrequency($3); }

        | '+' K_ORIGINAL {dumb_mode = 1; no_num = 1;} T_STRING
            { if (defrNetCbk) defrNet.defiNet::setOriginal($4); }
 
        | '+' K_PATTERN pattern_type
            { if (defrNetCbk) defrNet.defiNet::setPattern($3); }
 
        | '+' K_WEIGHT NUMBER
            { if (defrNetCbk) defrNet.defiNet::setWeight(ROUND($3)); }
 
        | '+' K_ESTCAP NUMBER
            { 
              /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
              if (defVersionNum < 5.5)
                 if (defrNetCbk) defrNet.defiNet::setCap($3);
            }
 
        | '+' K_USE use_type
            { if (defrNetCbk) defrNet.defiNet::setUse($3); }
 
        | '+' K_STYLE { dumb_mode = 1; no_num = 1; } T_STRING
            { if (defrNetCbk) defrNet.defiNet::setStyle($4); }
 
        | '+' K_PROPERTY {dumb_mode = 10000000;} snet_prop_list
            { dumb_mode = 0; }
 
        | extension_stmt
          { CALLBACK(defrNetExtCbk, defrNetExtCbkType, History_text); }

snet_width: '+' K_WIDTH { dumb_mode = 1; } T_STRING NUMBER
            {
              /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
              if (defVersionNum < 5.5)
                 if (defrSNetCbk) defrNet.defiNet::setWidth($4, $5);
            }

snet_voltage: '+' K_VOLTAGE NUMBER
            {
              if (defrSNetCbk) defrNet.defiNet::setVoltage($3);
            }

snet_spacing: '+' K_SPACING { dumb_mode = 1; } T_STRING NUMBER
            {
              if (defrSNetCbk) defrNet.defiNet::setSpacing($4,$5);
            }
        opt_snet_range
            {
            }

snet_prop_list: snet_prop
      | snet_prop_list snet_prop
      ;

snet_prop: T_STRING NUMBER
            {
              if (defrNetCbk) {
                char propTp;
                char* str = ringCopy("                       ");
                propTp = defrSNetProp.defiPropType::propType($1);
                /* For backword compatibility, also set the string value */
                /* We will use a temporary string to store the number.
                 * The string space is borrowed from the ring buffer
                 * in the lexer. */
                sprintf(str, "%g", $2);
                defrNet.defiNet::addNumProp($1, $2, str, propTp);
              }
            }
         | T_STRING QSTRING
            {
              if (defrNetCbk) {
                char propTp;
                propTp = defrSNetProp.defiPropType::propType($1);
                defrNet.defiNet::addProp($1, $2, propTp);
              }
            }
         | T_STRING T_STRING
            {
              if (defrNetCbk) {
                char propTp;
                propTp = defrSNetProp.defiPropType::propType($1);
                defrNet.defiNet::addProp($1, $2, propTp);
              }
            }

opt_snet_range: /* nothing */
        | K_RANGE NUMBER NUMBER
            {
              if (defrSNetCbk) defrNet.defiNet::setRange($2,$3);
            }

opt_range: /* nothing */
        | K_RANGE NUMBER NUMBER
            { defrProp.defiProp::setRange($2, $3); }

pattern_type: K_BALANCED
            { $$ = (char*)"BALANCED"; }
        | K_STEINER
            { $$ = (char*)"STEINER"; }
        | K_TRUNK
            { $$ = (char*)"TRUNK"; }
        | K_WIREDLOGIC
            { $$ = (char*)"WIREDLOGIC"; }

spaths:
    spath
      { if (defrNeedPathData) pathIsDone(shield); }
    | spaths snew_path
      { }

snew_path: K_NEW { dumb_mode = 1; } spath
      { if (defrNeedPathData) pathIsDone(shield); }

spath:  T_STRING
      { if (defrNeedPathData) defrPath->defiPath::addLayer($1);
        dumb_mode = 0; no_num = 0;
      }
    width opt_shape path_pt
      { dumb_mode = 1; new_is_keyword = TRUE; }
    path_item_list
      { dumb_mode = 0; }

width: NUMBER
      { if (defrNeedPathData) defrPath->defiPath::addWidth(ROUND($1)); }

start_snets: K_SNETS NUMBER ';'
      { CALLBACK(defrSNetStartCbk, defrSNetStartCbkType, ROUND($2)); }

end_snets: K_END K_SNETS 
	  { CALLBACK(defrSNetEndCbk, defrSNetEndCbkType, 0); }

groups_section: groups_start group_rules groups_end
            ;

groups_start: K_GROUPS NUMBER ';'
	{ CALLBACK(defrGroupsStartCbk, defrGroupsStartCbkType, ROUND($2)); }

group_rules: /* empty */
        | group_rules group_rule
            ;

group_rule: start_group group_members group_options ';'
	{ CALLBACK(defrGroupCbk, defrGroupCbkType, &defrGroup); }

start_group: '-' { dumb_mode = 1; no_num = 1; } T_STRING 
            {
              dumb_mode = 1000000000;
              no_num = 1000000000;
              /* dumb_mode is automatically turned off at the first
               * + in the options or at the ; at the end of the group */
              if (defrGroupCbk) defrGroup.defiGroup::setup($3);
              CALLBACK(defrGroupNameCbk, defrGroupNameCbkType, $3);
            }

group_members: 
        | group_members group_member
            {  }

group_member: T_STRING
            {
              /* if (defrGroupCbk) defrGroup.defiGroup::addMember($1); */
              CALLBACK(defrGroupMemberCbk, defrGroupMemberCbkType, $1);
            }

group_options: /* empty */
        | group_options group_option 
            ;

group_option: '+' K_SOFT group_soft_options
	  { }
        |     '+' K_PROPERTY { dumb_mode = 10000000; } group_prop_list
	  { dumb_mode = 0; }
        |     '+' K_REGION { dumb_mode = 1;  no_num = 1; } group_region
	  { }
        | extension_stmt
	  { CALLBACK(defrGroupExtCbk, defrGroupExtCbkType,
	      History_text); }

group_region:
     pt pt
	  {
            /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
            if (defVersionNum < 5.5) {
               if (defrGroupCbk)
                  defrGroup.defiGroup::addRegionRect($1.x, $1.y, $2.x, $2.y);
            }
          }
     | T_STRING
	  { if (defrGroupCbk)
	    defrGroup.defiGroup::setRegionName($1);
	  }

group_prop_list : /* empty */
     | group_prop_list group_prop
     ;

group_prop : T_STRING NUMBER
        {
          if (defrGroupCbk) {
             char propTp;
             char* str = ringCopy("                       ");
             propTp = defrGroupProp.defiPropType::propType($1);
             /* For backword compatibility, also set the string value */
             /* We will use a temporary string to store the number.
              * The string space is borrowed from the ring buffer
              * in the lexer. */
             sprintf(str, "%g", $2);
             defrGroup.defiGroup::addNumProperty($1, $2, str, propTp);
          }
        }
     | T_STRING QSTRING
        {
          if (defrGroupCbk) {
             char propTp;
             propTp = defrGroupProp.defiPropType::propType($1);
             defrGroup.defiGroup::addProperty($1, $2, propTp);
          }
        }
     | T_STRING T_STRING
        {
          if (defrGroupCbk) {
             char propTp;
             propTp = defrGroupProp.defiPropType::propType($1);
             defrGroup.defiGroup::addProperty($1, $2, propTp);
          }
        }

group_soft_options: /* empty */
        | group_soft_options group_soft_option 
            { }

group_soft_option: K_MAXX NUMBER
             {
               /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
               if (defVersionNum < 5.5)
                  if (defrGroupCbk) defrGroup.defiGroup::setMaxX(ROUND($2));
             }
         | K_MAXY NUMBER
             { 
               /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
               if (defVersionNum < 5.5)
                  if (defrGroupCbk) defrGroup.defiGroup::setMaxY(ROUND($2));
             }
         | K_MAXHALFPERIMETER NUMBER
             { 
               /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
               if (defVersionNum < 5.5)
                  if (defrGroupCbk) defrGroup.defiGroup::setPerim(ROUND($2));
             }

groups_end: K_END K_GROUPS 
	{ CALLBACK(defrGroupsEndCbk, defrGroupsEndCbkType, 0); }

/* 8/31/2001 - Wanda da Rosa. This is obsolete in 5.4 */
assertions_section: assertions_start constraint_rules assertions_end
            ;

/* 8/31/2001 - Wanda da Rosa. This is obsolete in 5.4 */
constraint_section: constraints_start constraint_rules constraints_end
            ;

assertions_start: K_ASSERTIONS NUMBER ';'
	{
          if (defVersionNum < 5.4) {
	    CALLBACK(defrAssertionsStartCbk, defrAssertionsStartCbkType,
	        ROUND($2));
          } else
            yydefwarning("ASSERTIONS is obsolete in 5.4. It will be ignored.");
	  if (defrAssertionCbk)
            defrAssertion.defiAssertion::setAssertionMode();
	}

constraints_start: K_CONSTRAINTS NUMBER ';'
        {
          if (defVersionNum < 5.4) {
            CALLBACK(defrConstraintsStartCbk, defrConstraintsStartCbkType,
              ROUND($2));
          } else
            yydefwarning("CONSTRAINTS is obsolete in 5.4. It will be ignored.");
	  if (defrConstraintCbk)
	    defrAssertion.defiAssertion::setConstraintMode();
        }

constraint_rules: /* empty */
        | constraint_rules constraint_rule 
            ;

constraint_rule:   operand_rule 
                 | wiredlogic_rule 
        {
          if (defVersionNum < 5.4) {
            if (defrAssertion.defiAssertion::isConstraint()) 
	      CALLBACK(defrConstraintCbk, defrConstraintCbkType,
	               &defrAssertion);
            if (defrAssertion.defiAssertion::isAssertion()) 
	      CALLBACK(defrAssertionCbk, defrAssertionCbkType,
	   	       &defrAssertion);
          }
        }

operand_rule: '-' operand delay_specs ';'
        { 
          if (defVersionNum < 5.4) {
            if (defrAssertion.defiAssertion::isConstraint()) 
	      CALLBACK(defrConstraintCbk, defrConstraintCbkType,
	               &defrAssertion);
            if (defrAssertion.defiAssertion::isAssertion()) 
              CALLBACK(defrAssertionCbk, defrAssertionCbkType,
		       &defrAssertion);
          }
   
          // reset all the flags and everything
          defrAssertion.defiAssertion::clear();
        }

operand: K_NET { dumb_mode = 1; no_num = 1; } T_STRING 
            {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::addNet($3);
            }
        | K_PATH {dumb_mode = 4; no_num = 4;}
	    T_STRING T_STRING T_STRING T_STRING
            {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::addPath($3, $4, $5, $6);
            }
        | K_SUM  '(' operand_list ')'
            {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::setSum();
            }
        | K_DIFF '(' operand_list ')'
            {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::setDiff();
            }

operand_list: operand 
        | operand_list ',' operand
            { }

wiredlogic_rule: '-' K_WIREDLOGIC { dumb_mode = 1; no_num = 1; } T_STRING
      opt_plus K_MAXDIST NUMBER ';'
            {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::setWiredlogic($4, $7);
            }

opt_plus:
	/* empty */
	  { $$ = (char*)""; }
	| '+'
	  { $$ = (char*)"+"; }

delay_specs: /* empty */
        | delay_specs delay_spec
            ;

delay_spec: '+' K_RISEMIN NUMBER 
            {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::setRiseMin($3);
            }
        | '+' K_RISEMAX NUMBER 
            {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::setRiseMax($3);
            }
        | '+' K_FALLMIN NUMBER 
            {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::setFallMin($3);
            }
        | '+' K_FALLMAX NUMBER 
            {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::setFallMax($3);
            }

constraints_end: K_END K_CONSTRAINTS
      { CALLBACK(defrConstraintsEndCbk, defrConstraintsEndCbkType, 0); }

assertions_end: K_END K_ASSERTIONS
	{ CALLBACK(defrAssertionsEndCbk, defrAssertionsEndCbkType, 0); }

scanchains_section: scanchain_start scanchain_rules scanchain_end
            ;

scanchain_start: K_SCANCHAINS NUMBER ';'
	{ CALLBACK(defrScanchainsStartCbk, defrScanchainsStartCbkType,
              ROUND($2)); }

scanchain_rules: /* empty */
        | scanchain_rules scan_rule
             {}

scan_rule: start_scan scan_members ';' 
      { 
           CALLBACK(defrScanchainCbk, defrScanchainCbkType, &defrScanchain);
      }

start_scan: '-' {dumb_mode = 1; no_num = 1;} T_STRING 
            {
              if (defrScanchainCbk)
                defrScanchain.defiScanchain::setName($3);
              bit_is_keyword = TRUE;
            }

scan_members: 
        | scan_members scan_member
            ;

opt_pin :
       /* empty */
       { $$ = (char*)""; }
       | T_STRING
       { $$ = $1; }

scan_member: '+' K_START {dumb_mode = 2; no_num = 2;} T_STRING opt_pin
	  { if (defrScanchainCbk)
	    defrScanchain.defiScanchain::setStart($4, $5);
          }
        | '+' K_FLOATING { dumb_mode = 1; no_num = 1; } floating_inst_list
          { dumb_mode = 0; no_num = 0; }
        | '+' K_ORDERED
          {
            dumb_mode = 1;
	    no_num = 1;
            if (defrScanchainCbk)
              defrScanchain.defiScanchain::addOrderedList();
          }
          ordered_inst_list
          { dumb_mode = 0; no_num = 0; }
        | '+' K_STOP {dumb_mode = 2; no_num = 2; } T_STRING opt_pin
	  { if (defrScanchainCbk)
	      defrScanchain.defiScanchain::setStop($4, $5);
          }
	| '+' K_COMMONSCANPINS { dumb_mode = 10; no_num = 10; }
	  opt_common_pins
	  { dumb_mode = 0;  no_num = 0; }
        | '+' K_PARTITION { dumb_mode = 1; no_num = 1; } T_STRING  /* 5.5 */
          partition_maxbits
          {
            if (defVersionNum < 5.5) {
              yyerror("PARTITION is a 5.5 syntax. Your def file is not defined as 5.5");
              CHKERR();
              }
            if (defrScanchainCbk)
              defrScanchain.defiScanchain::setPartition($4, $5);
          }
        | extension_stmt
	  {
	    CALLBACK(defrScanChainExtCbk, defrScanChainExtCbkType,
              History_text);
	  }

opt_common_pins: /* empty */
      { }
    | '(' T_STRING T_STRING ')'
      {
	if (defrScanchainCbk) {
	  if (strcmp($2, "IN") == 0 || strcmp($2, "in") == 0)
	    defrScanchain.defiScanchain::setCommonIn($3);
	  else if (strcmp($2, "OUT") == 0 || strcmp($2, "out") == 0)
	    defrScanchain.defiScanchain::setCommonOut($3);
	}
      }
    | '(' T_STRING T_STRING ')' '(' T_STRING T_STRING ')'
      {
	if (defrScanchainCbk) {
	  if (strcmp($2, "IN") == 0 || strcmp($2, "in") == 0)
	    defrScanchain.defiScanchain::setCommonIn($3);
	  else if (strcmp($2, "OUT") == 0 || strcmp($2, "out") == 0)
	    defrScanchain.defiScanchain::setCommonOut($3);
	  if (strcmp($6, "IN") == 0 || strcmp($6, "in") == 0)
	    defrScanchain.defiScanchain::setCommonIn($7);
	  else if (strcmp($6, "OUT") == 0 || strcmp($6, "out") == 0)
	    defrScanchain.defiScanchain::setCommonOut($7);
	}
      }


floating_inst_list: /* empty */
        | floating_inst_list one_floating_inst
            ;

one_floating_inst: T_STRING
      {
	dumb_mode = 1000;
	no_num = 1000;
	if (defrScanchainCbk)
	  defrScanchain.defiScanchain::addFloatingInst($1);
      }
    floating_pins
      { dumb_mode = 1; no_num = 1; }

floating_pins: /* empty */ 
      { }
    | '(' T_STRING  T_STRING ')'
      {
	if (defrScanchainCbk) {
	  if (strcmp($2, "IN") == 0 || strcmp($2, "in") == 0)
	    defrScanchain.defiScanchain::addFloatingIn($3);
	  else if (strcmp($2, "OUT") == 0 || strcmp($2, "out") == 0)
	    defrScanchain.defiScanchain::addFloatingOut($3);
          else if (strcmp($2, "BITS") == 0 || strcmp($2, "bits") == 0) {
            bitsNum = atoi($3);
            defrScanchain.defiScanchain::setFloatingBits(bitsNum);
         }
	}
      }
    | '(' T_STRING  T_STRING ')' '(' T_STRING  T_STRING ')'
      {
	if (defrScanchainCbk) {
	  if (strcmp($2, "IN") == 0 || strcmp($2, "in") == 0)
	    defrScanchain.defiScanchain::addFloatingIn($3);
	  else if (strcmp($2, "OUT") == 0 || strcmp($2, "out") == 0)
	    defrScanchain.defiScanchain::addFloatingOut($3);
	  else if (strcmp($2, "BITS") == 0 || strcmp($2, "bits") == 0) {
            bitsNum = atoi($3);
	    defrScanchain.defiScanchain::setFloatingBits(bitsNum);
          }
	  if (strcmp($6, "IN") == 0 || strcmp($6, "in") == 0)
	    defrScanchain.defiScanchain::addFloatingIn($7);
	  else if (strcmp($6, "OUT") == 0 || strcmp($6, "out") == 0)
	    defrScanchain.defiScanchain::addFloatingOut($7);
	  else if (strcmp($6, "BITS") == 0 || strcmp($6, "bits") == 0) {
            bitsNum = atoi($7);
	    defrScanchain.defiScanchain::setFloatingBits(bitsNum);
          }
	}
      }
    | '(' T_STRING  T_STRING ')' '(' T_STRING  T_STRING ')' '(' T_STRING T_STRING ')'
      {
	if (defrScanchainCbk) {
	  if (strcmp($2, "IN") == 0 || strcmp($2, "in") == 0)
	    defrScanchain.defiScanchain::addFloatingIn($3);
	  else if (strcmp($2, "OUT") == 0 || strcmp($2, "out") == 0)
	    defrScanchain.defiScanchain::addFloatingOut($3);
	  else if (strcmp($2, "BITS") == 0 || strcmp($2, "bits") == 0) {
            bitsNum = atoi($3);
	    defrScanchain.defiScanchain::setFloatingBits(bitsNum);
          }
	  if (strcmp($6, "IN") == 0 || strcmp($6, "in") == 0)
	    defrScanchain.defiScanchain::addFloatingIn($7);
	  else if (strcmp($6, "OUT") == 0 || strcmp($6, "out") == 0)
	    defrScanchain.defiScanchain::addFloatingOut($7);
	  else if (strcmp($6, "BITS") == 0 || strcmp($6, "bits") == 0) {
            bitsNum = atoi($7);
	    defrScanchain.defiScanchain::setFloatingBits(bitsNum);
          }
	  if (strcmp($10, "IN") == 0 || strcmp($10, "in") == 0)
	    defrScanchain.defiScanchain::addFloatingIn($11);
	  else if (strcmp($10, "OUT") == 0 || strcmp($10, "out") == 0)
	    defrScanchain.defiScanchain::addFloatingOut($11);
	  else if (strcmp($10, "BITS") == 0 || strcmp($10, "bits") == 0) {
            bitsNum = atoi($11);
	    defrScanchain.defiScanchain::setFloatingBits(bitsNum);
          }
	}
      }
    
ordered_inst_list: /* empty */
        | ordered_inst_list one_ordered_inst
            ;

one_ordered_inst: T_STRING
      { dumb_mode = 1000; no_num = 1000; 
	if (defrScanchainCbk)
	  defrScanchain.defiScanchain::addOrderedInst($1);
      }
    ordered_pins
      { dumb_mode = 1; no_num = 1; }

ordered_pins: /* empty */ 
      { }
    | '(' T_STRING  T_STRING ')'
      {
	if (defrScanchainCbk) {
	  if (strcmp($2, "IN") == 0 || strcmp($2, "in") == 0)
	    defrScanchain.defiScanchain::addOrderedIn($3);
	  else if (strcmp($2, "OUT") == 0 || strcmp($2, "out") == 0)
	    defrScanchain.defiScanchain::addOrderedOut($3);
          else if (strcmp($2, "BITS") == 0 || strcmp($2, "bits") == 0) {
            bitsNum = atoi($3);
            defrScanchain.defiScanchain::setOrderedBits(bitsNum);
         }
	}
      }
    | '(' T_STRING  T_STRING ')' '(' T_STRING  T_STRING ')'
      {
	if (defrScanchainCbk) {
	  if (strcmp($2, "IN") == 0 || strcmp($2, "in") == 0)
	    defrScanchain.defiScanchain::addOrderedIn($3);
	  else if (strcmp($2, "OUT") == 0 || strcmp($2, "out") == 0)
	    defrScanchain.defiScanchain::addOrderedOut($3);
	  else if (strcmp($2, "BITS") == 0 || strcmp($2, "bits") == 0) {
            bitsNum = atoi($3);
            defrScanchain.defiScanchain::setOrderedBits(bitsNum);
          }
	  if (strcmp($6, "IN") == 0 || strcmp($6, "in") == 0)
	    defrScanchain.defiScanchain::addOrderedIn($7);
	  else if (strcmp($6, "OUT") == 0 || strcmp($6, "out") == 0)
	    defrScanchain.defiScanchain::addOrderedOut($7);
	  else if (strcmp($6, "BITS") == 0 || strcmp($6, "bits") == 0) {
            bitsNum = atoi($7);
            defrScanchain.defiScanchain::setOrderedBits(bitsNum);
          }
	}
      }
    | '(' T_STRING  T_STRING ')' '(' T_STRING  T_STRING ')' '(' T_STRING T_STRING ')'
      {
	if (defrScanchainCbk) {
	  if (strcmp($2, "IN") == 0 || strcmp($2, "in") == 0)
	    defrScanchain.defiScanchain::addOrderedIn($3);
	  else if (strcmp($2, "OUT") == 0 || strcmp($2, "out") == 0)
	    defrScanchain.defiScanchain::addOrderedOut($3);
	  else if (strcmp($2, "BITS") == 0 || strcmp($2, "bits") == 0) {
            bitsNum = atoi($3);
	    defrScanchain.defiScanchain::setOrderedBits(bitsNum);
          }
	  if (strcmp($6, "IN") == 0 || strcmp($6, "in") == 0)
	    defrScanchain.defiScanchain::addOrderedIn($7);
	  else if (strcmp($6, "OUT") == 0 || strcmp($6, "out") == 0)
	    defrScanchain.defiScanchain::addOrderedOut($7);
	  else if (strcmp($6, "BITS") == 0 || strcmp($6, "bits") == 0) {
            bitsNum = atoi($7);
	    defrScanchain.defiScanchain::setOrderedBits(bitsNum);
          }
	  if (strcmp($10, "IN") == 0 || strcmp($10, "in") == 0)
	    defrScanchain.defiScanchain::addOrderedIn($11);
	  else if (strcmp($10, "OUT") == 0 || strcmp($10, "out") == 0)
	    defrScanchain.defiScanchain::addOrderedOut($11);
	  else if (strcmp($10, "BITS") == 0 || strcmp($10, "bits") == 0) {
            bitsNum = atoi($11);
	    defrScanchain.defiScanchain::setOrderedBits(bitsNum);
          }
	}
      }
    
partition_maxbits: /* empty */
    { $$ = -1; }
    | K_MAXBITS NUMBER
    { $$ = ROUND($2); }
    
scanchain_end: K_END K_SCANCHAINS
	{ 
          CALLBACK(defrScanchainsEndCbk, defrScanchainsEndCbkType, 0);
          bit_is_keyword = FALSE;
          dumb_mode = 0; no_num = 0;
        }

/* 8/31/2001 - Wanda da Rosa. This is obsolete in 5.4 */
iotiming_section: iotiming_start iotiming_rules iotiming_end
            ;

iotiming_start: K_IOTIMINGS NUMBER ';'
	{
          if (defVersionNum < 5.4) {
             CALLBACK(defrIOTimingsStartCbk, defrIOTimingsStartCbkType,
                      ROUND($2));
          } else {
             yydefwarning("IOTIMINGS is obsolete in 5.4. It will be ignored.");
          }
        }

iotiming_rules: /* empty */
        | iotiming_rules iotiming_rule
             {}

iotiming_rule: start_iotiming iotiming_members ';' 
	    { 
              if (defVersionNum < 5.4)
                CALLBACK(defrIOTimingCbk, defrIOTimingCbkType, &defrIOTiming);
            } 

start_iotiming: '-' '(' {dumb_mode = 2; no_num = 2; } T_STRING T_STRING ')'
            {
              if (defrIOTimingCbk)
                defrIOTiming.defiIOTiming::setName($4, $5);
            }

iotiming_members: 
        | iotiming_members iotiming_member
            ;

iotiming_member:
        '+' risefall K_VARIABLE NUMBER NUMBER
            {
              if (defrIOTimingCbk) 
                defrIOTiming.defiIOTiming::setVariable($2, $4, $5);
            }
        | '+' risefall K_SLEWRATE NUMBER NUMBER
            {
              if (defrIOTimingCbk) 
                defrIOTiming.defiIOTiming::setSlewRate($2, $4, $5);
            }
        | '+' K_CAPACITANCE NUMBER
            {
              if (defrIOTimingCbk) 
                defrIOTiming.defiIOTiming::setCapacitance($3);
            }
        | '+' K_DRIVECELL {dumb_mode = 1; no_num = 1; } T_STRING
            {
              if (defrIOTimingCbk) 
                defrIOTiming.defiIOTiming::setDriveCell($4);
            } iotiming_drivecell_opt
        /*| '+' K_FROMPIN   {dumb_mode = 1; no_num = 1; } T_STRING*/
/*
        | K_FROMPIN {dumb_mode = 1; no_num = 1; } T_STRING
            {
              if (defrIOTimingCbk) 
                defrIOTiming.defiIOTiming::setFrom($3);
            }
            K_TOPIN {dumb_mode = 1; no_num = 1; } T_STRING
            {
              if (defrIOTimingCbk) 
                defrIOTiming.defiIOTiming::setTo($7);
            }
*/
        /*| '+' K_PARALLEL NUMBER*/
/*
        |  K_PARALLEL NUMBER
            {
              if (defrIOTimingCbk) 
                defrIOTiming.defiIOTiming::setParallel($2);
            }
*/
        | extension_stmt
            {
              if (defVersionNum < 5.4)
                CALLBACK(defrIoTimingsExtCbk, defrIoTimingsExtCbkType,
                History_text);
            }

iotiming_drivecell_opt: iotiming_frompin
           K_TOPIN {dumb_mode = 1; no_num = 1; } T_STRING
           {
              if (defrIOTimingCbk) 
                defrIOTiming.defiIOTiming::setTo($4);
           }
           iotiming_parallel

iotiming_frompin: /* empty */
        | K_FROMPIN {dumb_mode = 1; no_num = 1; } T_STRING
           {
              if (defrIOTimingCbk)
                defrIOTiming.defiIOTiming::setFrom($3);
           }

iotiming_parallel: /* empty */
        | K_PARALLEL NUMBER
           {
              if (defrIOTimingCbk)
                defrIOTiming.defiIOTiming::setParallel($2);
           }

risefall: K_RISE { $$ = (char*)"RISE"; } | K_FALL { $$ = (char*)"FALL"; }

iotiming_end: K_END K_IOTIMINGS
	  {
            if (defVersionNum < 5.4)
              CALLBACK(defrIOTimingsEndCbk, defrIOTimingsEndCbkType, 0);
          }

floorplan_contraints_section: fp_start fp_stmts K_END K_FPC
          { CALLBACK(defrFPCEndCbk, defrFPCEndCbkType, 0); }

fp_start: K_FPC NUMBER ';'
          { CALLBACK(defrFPCStartCbk, defrFPCStartCbkType,
              ROUND($2)); }

fp_stmts: /* empty */
        | fp_stmts fp_stmt
            {}

fp_stmt: '-' { dumb_mode = 1; no_num = 1;  } T_STRING h_or_v
          { if (defrFPCCbk) defrFPC.defiFPC::setName($3, $4); }
          constraint_type constrain_what_list ';'
          { CALLBACK(defrFPCCbk, defrFPCCbkType, &defrFPC); }

h_or_v: K_HORIZONTAL 
            { $$ = (char*)"HORIZONTAL"; }
        | K_VERTICAL
            { $$ = (char*)"VERTICAL"; }

constraint_type: K_ALIGN
            { if (defrFPCCbk) defrFPC.defiFPC::setAlign(); }
        | K_MAX NUMBER
            { if (defrFPCCbk) defrFPC.defiFPC::setMax($2); }
        | K_MIN NUMBER
            { if (defrFPCCbk) defrFPC.defiFPC::setMin($2); }
        | K_EQUAL NUMBER
            { if (defrFPCCbk) defrFPC.defiFPC::setEqual($2); }

constrain_what_list: /* empty */
        | constrain_what_list constrain_what
            ;

constrain_what: '+' K_BOTTOMLEFT
            { if (defrFPCCbk) defrFPC.defiFPC::setDoingBottomLeft(); }
            row_or_comp_list 
        |       '+' K_TOPRIGHT
            { if (defrFPCCbk) defrFPC.defiFPC::setDoingTopRight(); }
            row_or_comp_list 
            ;

row_or_comp_list: /* empty */
        | row_or_comp_list row_or_comp

row_or_comp: '(' K_ROWS  {dumb_mode = 1; no_num = 1; } T_STRING ')'
            { if (defrFPCCbk) defrFPC.defiFPC::addRow($4); }
        |    '(' K_COMPS {dumb_mode = 1; no_num = 1; } T_STRING ')'
            { if (defrFPCCbk) defrFPC.defiFPC::addComps($4); }

timingdisables_section: timingdisables_start timingdisables_rules timingdisables_end
            ;

timingdisables_start: K_TIMINGDISABLES NUMBER ';'
      { CALLBACK(defrTimingDisablesStartCbk, defrTimingDisablesStartCbkType,
              ROUND($2)); }

timingdisables_rules: /* empty */
        | timingdisables_rules timingdisables_rule
             {}

timingdisables_rule: '-' K_FROMPIN { dumb_mode = 2; no_num = 2;  } T_STRING T_STRING
                         K_TOPIN { dumb_mode = 2; no_num = 2;  } T_STRING T_STRING ';'
            {
              if (defrTimingDisableCbk)
                defrTimingDisable.defiTimingDisable::setFromTo($4, $5, $8, $9);
	      CALLBACK(defrTimingDisableCbk, defrTimingDisableCbkType,
                &defrTimingDisable);
	    }
        | '-' K_THRUPIN {dumb_mode = 2; no_num = 2; } T_STRING T_STRING ';'
            {
              if (defrTimingDisableCbk)
                defrTimingDisable.defiTimingDisable::setThru($4, $5);
	      CALLBACK(defrTimingDisableCbk, defrTimingDisableCbkType,
                &defrTimingDisable);
            }
        | '-' K_MACRO {dumb_mode = 1; no_num = 1;} T_STRING td_macro_option ';'
            {
              if (defrTimingDisableCbk)
                defrTimingDisable.defiTimingDisable::setMacro($4);
	      CALLBACK(defrTimingDisableCbk, defrTimingDisableCbkType,
                &defrTimingDisable);
            }
        | '-' K_REENTRANTPATHS ';'
            { if (defrTimingDisableCbk)
                defrTimingDisable.defiTimingDisable::setReentrantPathsFlag();
	    }


td_macro_option: K_FROMPIN {dumb_mode = 1; no_num = 1;} T_STRING K_TOPIN
    {dumb_mode=1; no_num = 1;} T_STRING
            {
              if (defrTimingDisableCbk)
                defrTimingDisable.defiTimingDisable::setMacroFromTo($3,$6);
            }
        |        K_THRUPIN {dumb_mode=1; no_num = 1;} T_STRING
            {
              if (defrTimingDisableCbk)
                defrTimingDisable.defiTimingDisable::setMacroThru($3);
            }

timingdisables_end: K_END K_TIMINGDISABLES
      { CALLBACK(defrTimingDisablesEndCbk, defrTimingDisablesEndCbkType, 0); }


partitions_section: partitions_start partition_rules partitions_end
            ;

partitions_start: K_PARTITIONS NUMBER ';'
	{ CALLBACK(defrPartitionsStartCbk, defrPartitionsStartCbkType,
              ROUND($2)); }

partition_rules: /* empty */
        | partition_rules partition_rule
             {}

partition_rule: start_partition partition_members ';' 
          { CALLBACK(defrPartitionCbk, defrPartitionCbkType,
              &defrPartition); }

start_partition: '-' { dumb_mode = 1; no_num = 1; } T_STRING turnoff
            {
            if (defrPartitionCbk)
              defrPartition.defiPartition::setName($3);
            }

turnoff: /* empty */
        | K_TURNOFF turnoff_setup turnoff_hold
            {
              if (defrPartitionCbk)
                defrPartition.defiPartition::addTurnOff($2, $3);
            }

turnoff_setup: /* empty */
            { $$ = (char*)" "; }
        | K_SETUPRISE
            { $$ = (char*)"R"; }
        | K_SETUPFALL
            { $$ = (char*)"F"; }

turnoff_hold: /* empty */
            { $$ = (char*)" "; }
        | K_HOLDRISE
            { $$ = (char*)"R"; }
        | K_HOLDFALL
            { $$ = (char*)"F"; }

partition_members: /* empty */
        | partition_members partition_member
            ;

partition_member: '+' K_FROMCLOCKPIN {dumb_mode=2; no_num = 2;}
     T_STRING T_STRING risefall minmaxpins
            {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setFromClockPin($4, $5);
            }
        |  '+' K_FROMCOMPPIN {dumb_mode=2; no_num = 2; }
	    T_STRING T_STRING risefallminmax2_list
            {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setFromCompPin($4, $5);
            }
        |   '+' K_FROMIOPIN {dumb_mode=1; no_num = 1; } T_STRING
	   risefallminmax1_list
            {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setFromIOPin($4);
            }
        | '+' K_TOCLOCKPIN {dumb_mode=2; no_num = 2; }
	    T_STRING T_STRING risefall minmaxpins
            {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setToClockPin($4, $5);
            }
        |  '+' K_TOCOMPPIN {dumb_mode=2; no_num = 2; }
	    T_STRING T_STRING risefallminmax2_list
            {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setToCompPin($4, $5);
            }
        |   '+' K_TOIOPIN {dumb_mode=1; no_num = 2; } T_STRING risefallminmax1_list
            {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setToIOPin($4);
            }
        | extension_stmt
          { CALLBACK(defrPartitionsExtCbk, defrPartitionsExtCbkType,
              History_text); }

minmaxpins: min_or_max_list K_PINS
     { dumb_mode = 1000000000; no_num = 10000000; } pin_list
        { dumb_mode = 0; no_num = 0; }

min_or_max_list: /* empty */
        | min_or_max_list min_or_max_member
            {}

min_or_max_member: K_MIN NUMBER NUMBER
            {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setMin($2, $3);
            }
        | K_MAX NUMBER NUMBER
            {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setMax($2, $3);
            }

pin_list: /* empty */
        | pin_list T_STRING
	{ if (defrPartitionCbk) defrPartition.defiPartition::addPin($2); }

risefallminmax1_list: /* empty */
        | risefallminmax1_list risefallminmax1

risefallminmax1: K_RISEMIN NUMBER
    { if (defrPartitionCbk) defrPartition.defiPartition::addRiseMin($2); }
        | K_FALLMIN NUMBER
    { if (defrPartitionCbk) defrPartition.defiPartition::addFallMin($2); }
        | K_RISEMAX NUMBER
    { if (defrPartitionCbk) defrPartition.defiPartition::addRiseMax($2); }
        | K_FALLMAX NUMBER
    { if (defrPartitionCbk) defrPartition.defiPartition::addFallMax($2); }

risefallminmax2_list:
      risefallminmax2
      | risefallminmax2_list risefallminmax2
      ;

risefallminmax2: K_RISEMIN NUMBER NUMBER
	    { if (defrPartitionCbk)
	    defrPartition.defiPartition::addRiseMinRange($2, $3); }
        | K_FALLMIN NUMBER NUMBER
	    { if (defrPartitionCbk)
	    defrPartition.defiPartition::addFallMinRange($2, $3); }
        | K_RISEMAX NUMBER NUMBER
	    { if (defrPartitionCbk)
	    defrPartition.defiPartition::addRiseMaxRange($2, $3); }
        | K_FALLMAX NUMBER NUMBER
	    { if (defrPartitionCbk)
	    defrPartition.defiPartition::addFallMaxRange($2, $3); }

partitions_end: K_END K_PARTITIONS
	{ CALLBACK(defrPartitionsEndCbk, defrPartitionsEndCbkType, 0); }


comp_names: /* empty */
        | comp_names comp_name
            {}

comp_name: '(' {dumb_mode=2; no_num = 2; } T_STRING
    T_STRING subnet_opt_syn ')'
	  {
            /* note that the first T_STRING could be the keyword VPIN */
	    if (defrNetCbk) {
	      defrSubnet->defiSubnet::addPin($3, $4, $5);
	    }
	  }

subnet_opt_syn: /* empty */
	  { $$ = 0; }
        | '+' K_SYNTHESIZED
	  { $$ = 1; }

subnet_options: /* empty */
        | subnet_options subnet_option

subnet_option: subnet_type
            {  
            if (defrNetCbk) defrSubnet->defiSubnet::addWire($1);
            }
            paths
            {  
            by_is_keyword = FALSE;
            do_is_keyword = FALSE;
            new_is_keyword = FALSE;
            step_is_keyword = FALSE;
            }
        | K_NONDEFAULTRULE { dumb_mode = 1; no_num = 1; } T_STRING
            { if (defrNetCbk) defrSubnet->defiSubnet::setNonDefault($3); }

subnet_type: K_FIXED
               { $$ = (char*)"FIXED"; }
           | K_COVER
               { $$ = (char*)"COVER"; }
           | K_ROUTED
               { $$ = (char*)"ROUTED"; }
           | K_NOSHIELD
               { $$ = (char*)"NOSHIELD"; }

pin_props_section: begin_pin_props pin_prop_list end_pin_props ;

begin_pin_props: K_PINPROPERTIES NUMBER opt_semi
    { CALLBACK(defrPinPropStartCbk, defrPinPropStartCbkType, ROUND($2)); }

opt_semi:
    /* empty */
      { }
    | ';'
      { }

end_pin_props: K_END K_PINPROPERTIES
    { CALLBACK(defrPinPropEndCbk, defrPinPropEndCbkType, 0); }

pin_prop_list: /* empty */
    | pin_prop_list pin_prop_terminal
    ;

pin_prop_terminal: '-' { dumb_mode = 2; no_num = 2; } T_STRING T_STRING
    { if (defrPinPropCbk) defrPinProp.defiPinProp::setName($3, $4); }
    pin_prop_options ';'
    { CALLBACK(defrPinPropCbk, defrPinPropCbkType, &defrPinProp);
      // reset the property number
      defrPinProp.defiPinProp::clear();
    }

pin_prop_options : /* empty */
    | pin_prop_options pin_prop ;

pin_prop: '+' K_PROPERTY { dumb_mode = 10000000; }
   pin_prop_name_value_list 
    { dumb_mode = 0; }

pin_prop_name_value_list : /* empty */
    | pin_prop_name_value_list pin_prop_name_value
    ;

pin_prop_name_value : T_STRING NUMBER
    {
      if (defrPinPropCbk) {
         char propTp;
         char* str = ringCopy("                       ");
         propTp = defrCompPinProp.defiPropType::propType($1);
         /* For backword compatibility, also set the string value */
         /* We will use a temporary string to store the number.
         * The string space is borrowed from the ring buffer
         * in the lexer. */
         sprintf(str, "%g", $2);
         defrPinProp.defiPinProp::addNumProperty($1, $2, str, propTp);
      }
    }
 | T_STRING QSTRING
    {
      if (defrPinPropCbk) {
         char propTp;
         propTp = defrCompPinProp.defiPropType::propType($1);
         defrPinProp.defiPinProp::addProperty($1, $2, propTp);
      }
    }
 | T_STRING T_STRING
    {
      if (defrPinPropCbk) {
         char propTp;
         propTp = defrCompPinProp.defiPropType::propType($1);
         defrPinProp.defiPinProp::addProperty($1, $2, propTp);
      }
    }

blockage_section: blockage_start blockage_defs blockage_end ;

blockage_start: K_BLOCKAGES NUMBER ';'
    { CALLBACK(defrBlockageStartCbk, defrBlockageStartCbkType, ROUND($2)); }

blockage_end: K_END K_BLOCKAGES
    { CALLBACK(defrBlockageEndCbk, defrBlockageEndCbkType, 0); }

blockage_defs: /* empty */
    | blockage_defs blockage_def
    ;

blockage_def: blockage_rule rect_blockage rect_blockage_rules ';'
    {
      CALLBACK(defrBlockageCbk, defrBlockageCbkType, &defrBlockage);
      defrBlockage.defiBlockage::clear();
    }

blockage_rule: '-' K_LAYER { dumb_mode = 1; no_num = 1; } T_STRING 
    {
      if (defrBlockage.defiBlockage::hasPlacement() != 0) {
        yyerror("BLOCKAGE has both LAYER & PLACEMENT defined.");
        CHKERR();
      }
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setLayer($4);
    }
    comp_blockage_rule
    /* 10/29/2001 - Wanda da Rosa, enhancement */
    | '-' K_PLACEMENT
    {
      if (defrBlockage.defiBlockage::hasLayer() != 0) {
        yyerror("BLOCKAGE has both LAYER & PLACEMENT defined.");
        CHKERR();
      }
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setPlacement();
    }
    placement_comp_rule

comp_blockage_rule: /* empty */
    /* 06/20/2001 - Wanda da Rosa, pcr 383204 = pcr 378102 */
    | '+' K_COMPONENT { dumb_mode = 1; no_num = 1; } T_STRING
    {
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setComponent($4);
    }
    /* 8/30/2001 - Wanda da Rosa, pcr 394394 */
    | '+' K_SLOTS
    {
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setSlots();
    }
    | '+' K_FILLS
    {
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setFills();
    }
    | '+' K_PUSHDOWN
    {
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setPushdown();
    }
  

placement_comp_rule: /* empty */
    /* 10/29/2001 - Wanda da Rosa, enhancement */
    | '+' K_COMPONENT { dumb_mode = 1; no_num = 1; } T_STRING
    {
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setComponent($4);
    }
    | '+' K_PUSHDOWN
    {
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setPushdown();
    }

rect_blockage_rules: /* empty */
    | rect_blockage_rules rect_blockage
    ;

rect_blockage: K_RECT pt pt
    {
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::addRect($2.x, $2.y, $3.x, $3.y);
    }

/* 8/31/2001 - Wanda da Rosa.  5.4 enhancement */
slot_section: slot_start slot_defs slot_end ;

slot_start: K_SLOTS NUMBER ';'
    { CALLBACK(defrSlotStartCbk, defrSlotStartCbkType, ROUND($2)); }

slot_end: K_END K_SLOTS
    { CALLBACK(defrSlotEndCbk, defrSlotEndCbkType, 0); }

slot_defs: /* empty */
    | slot_defs slot_def
    ;

slot_def: slot_rule rect_slot_rules ';'
    {
      CALLBACK(defrSlotCbk, defrSlotCbkType, &defrSlot);
      defrSlot.defiSlot::clear();
    }

slot_rule: '-' K_LAYER { dumb_mode = 1; no_num = 1; } T_STRING 
    {
      if (defrSlotCbk)
        defrSlot.defiSlot::setLayer($4);
    } rect_slot

rect_slot_rules: /* empty */
    | rect_slot_rules rect_slot
    ;

rect_slot: K_RECT pt pt
    {
      if (defrSlotCbk)
        defrSlot.defiSlot::addRect($2.x, $2.y, $3.x, $3.y);
    }

/* 8/31/2001 - Wanda da Rosa.  5.4 enhancement */
fill_section: fill_start fill_defs fill_end ;

fill_start: K_FILLS NUMBER ';'
    { CALLBACK(defrFillStartCbk, defrFillStartCbkType, ROUND($2)); }

fill_end: K_END K_FILLS
    { CALLBACK(defrFillEndCbk, defrFillEndCbkType, 0); }

fill_defs: /* empty */
    | fill_defs fill_def
    ;

fill_def: fill_rule rect_fill_rules ';'
    {
      CALLBACK(defrFillCbk, defrFillCbkType, &defrFill);
      defrFill.defiFill::clear();
    }

fill_rule: '-' K_LAYER { dumb_mode = 1; no_num = 1; } T_STRING 
    {
      if (defrFillCbk)
        defrFill.defiFill::setLayer($4);
    } rect_fill

rect_fill_rules: /* empty */
    | rect_fill_rules rect_fill
    ;

rect_fill: K_RECT pt pt
    {
      if (defrFillCbk)
        defrFill.defiFill::addRect($2.x, $2.y, $3.x, $3.y);
    }

%%

