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

/*                                                                     */
/*   Revision:                                                         */
/*   03-15-2000 Wanda da Rosa - Add code to support 5.4, add keywords  */
/*                              for PINS + USE, SPECIALNETS + SHAPE    */
/*                              and other keywords                     */
#include <stdio.h>
#include <stdlib.h>
#include "lex.h"
#include "defiDefs.hpp"
#include "def.tab.h"
#include "FEF_stringhash.h"
#include "defiDebug.hpp"

void STR_add_to_stringtab_Int(void *hdl, const char *str, int inValue) {
    int *value;

    // Modified 9/2/99 -- Wanda da Rosa
    // need to free this in FEF_stringhash.cpp
    value = (int*)defMalloc(sizeof(int));
    if (value != NULL)  {
        *value = inValue;
        STR_add_to_stringtab (hdl, str, value);
    } else {
        // Ran out of memory
        defiError("Not enough memory");
    }
}

void init_symbol_table(void *Keyword_set)
{

    STR_add_to_stringtab_Int(Keyword_set, "VERSION", K_VERSION) ;
    STR_add_to_stringtab_Int(Keyword_set, "NAMESCASESENSITIVE",
      K_NAMESCASESENSITIVE) ;
    STR_add_to_stringtab_Int(Keyword_set, "NAMEMAPSTRING", K_NAMEMAPSTRING) ;
    STR_add_to_stringtab_Int(Keyword_set, "HISTORY", K_HISTORY) ;
    STR_add_to_stringtab_Int(Keyword_set, "DESIGN", K_DESIGN) ;
    STR_add_to_stringtab_Int(Keyword_set, "VIAS", K_VIAS) ;
    STR_add_to_stringtab_Int(Keyword_set, "TECHNOLOGY", K_TECH) ;
    STR_add_to_stringtab_Int(Keyword_set, "UNITS", K_UNITS) ;
    STR_add_to_stringtab_Int(Keyword_set, "BUSBITCHARS", K_BUSBITCHARS) ;
    STR_add_to_stringtab_Int(Keyword_set, "DIVIDERCHAR", K_DIVIDERCHAR) ;
    STR_add_to_stringtab_Int(Keyword_set, "DISTANCE", K_DISTANCE) ;
    STR_add_to_stringtab_Int(Keyword_set, "MICRONS", K_MICRONS) ;
    STR_add_to_stringtab_Int(Keyword_set, "RECT", K_RECT) ;
    STR_add_to_stringtab_Int(Keyword_set, "REENTRANTPATHS", K_REENTRANTPATHS) ;
    STR_add_to_stringtab_Int(Keyword_set, "SITE", K_SITE) ;
    STR_add_to_stringtab_Int(Keyword_set, "CANPLACE", K_CANPLACE) ;
    STR_add_to_stringtab_Int(Keyword_set, "CANNOTOCCUPY", K_CANNOTOCCUPY) ;
    STR_add_to_stringtab_Int(Keyword_set, "DIEAREA", K_DIE_AREA) ;
    STR_add_to_stringtab_Int(Keyword_set, "PINS", K_PINS) ;
    STR_add_to_stringtab_Int(Keyword_set, "DEFAULTCAP", K_DEFAULTCAP) ;
    STR_add_to_stringtab_Int(Keyword_set, "MINPINS", K_MINPINS) ;
    STR_add_to_stringtab_Int(Keyword_set, "WIRECAP", K_WIRECAP) ;
    STR_add_to_stringtab_Int(Keyword_set, "TRACKS", K_TRACKS) ;
    STR_add_to_stringtab_Int(Keyword_set, "GCELLGRID", K_GCELLGRID) ;
    STR_add_to_stringtab_Int(Keyword_set, "DO", K_DO) ;
    STR_add_to_stringtab_Int(Keyword_set, "BY", K_BY) ;
    STR_add_to_stringtab_Int(Keyword_set, "STEP", K_STEP) ;
    STR_add_to_stringtab_Int(Keyword_set, "LAYER", K_LAYER) ;
    STR_add_to_stringtab_Int(Keyword_set, "COMPONENTS", K_COMPS) ;
    STR_add_to_stringtab_Int(Keyword_set, "GENERATE", K_COMP_GEN) ;
    STR_add_to_stringtab_Int(Keyword_set, "SOURCE", K_SOURCE) ;
    STR_add_to_stringtab_Int(Keyword_set, "WEIGHT", K_WEIGHT) ;
    STR_add_to_stringtab_Int(Keyword_set, "FIXED", K_FIXED) ;
    STR_add_to_stringtab_Int(Keyword_set, "COVER", K_COVER) ;
    STR_add_to_stringtab_Int(Keyword_set, "PLACED", K_PLACED) ;
    STR_add_to_stringtab_Int(Keyword_set, "UNPLACED", K_UNPLACED) ;
    STR_add_to_stringtab_Int(Keyword_set, "FOREIGN", K_FOREIGN) ;
    STR_add_to_stringtab_Int(Keyword_set, "EEQMASTER", K_EEQMASTER) ;
    STR_add_to_stringtab_Int(Keyword_set, "REGION", K_REGION) ;
    STR_add_to_stringtab_Int(Keyword_set, "REGIONS", K_REGIONS) ;
    STR_add_to_stringtab_Int(Keyword_set, "NETS", K_NETS) ;
    STR_add_to_stringtab_Int(Keyword_set, "MUSTJOIN", K_MUSTJOIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "ORIGINAL", K_ORIGINAL) ;
    STR_add_to_stringtab_Int(Keyword_set, "USE", K_USE) ;
    STR_add_to_stringtab_Int(Keyword_set, "STYLE", K_STYLE) ;
    STR_add_to_stringtab_Int(Keyword_set, "PATTERN", K_PATTERN) ;
    STR_add_to_stringtab_Int(Keyword_set, "PATTERNNAME", K_PATTERNNAME) ;
    STR_add_to_stringtab_Int(Keyword_set, "NONDEFAULTRULE", K_NONDEFAULTRULE) ;
    STR_add_to_stringtab_Int(Keyword_set, "ESTCAP", K_ESTCAP) ;
    STR_add_to_stringtab_Int(Keyword_set, "ROUTED", K_ROUTED) ;
    STR_add_to_stringtab_Int(Keyword_set, "ROW", K_ROW) ;
    STR_add_to_stringtab_Int(Keyword_set, "TAPER", K_TAPER) ;
    STR_add_to_stringtab_Int(Keyword_set, "TAPERRULE", K_TAPERRULE) ;
/*    STR_add_to_stringtab_Int(Keyword_set, "", K_END_NETS) ; */
    STR_add_to_stringtab_Int(Keyword_set, "NEW", K_NEW) ;
    STR_add_to_stringtab_Int(Keyword_set, "SHAPE", K_SHAPE) ;
    STR_add_to_stringtab_Int(Keyword_set, "SPECIALNETS", K_SNETS) ;
    STR_add_to_stringtab_Int(Keyword_set, "SPECIALNET", K_SNET) ;
    STR_add_to_stringtab_Int(Keyword_set, "WIDTH", K_WIDTH) ;
    STR_add_to_stringtab_Int(Keyword_set, "VOLTAGE", K_VOLTAGE) ;
    STR_add_to_stringtab_Int(Keyword_set, "SPACING", K_SPACING) ;
/*    STR_add_to_stringtab_Int(Keyword_set, "", K_END_SNETS) ; */
    STR_add_to_stringtab_Int(Keyword_set, "N", K_N) ;
    STR_add_to_stringtab_Int(Keyword_set, "S", K_S);
    STR_add_to_stringtab_Int(Keyword_set, "E", K_E);
    STR_add_to_stringtab_Int(Keyword_set, "W", K_W);
    STR_add_to_stringtab_Int(Keyword_set, "FN", K_FN) ;
    STR_add_to_stringtab_Int(Keyword_set, "FS", K_FS) ;
    STR_add_to_stringtab_Int(Keyword_set, "FE", K_FE) ;
    STR_add_to_stringtab_Int(Keyword_set, "FW", K_FW) ;

    STR_add_to_stringtab_Int(Keyword_set, "GROUPS", K_GROUPS) ;
    STR_add_to_stringtab_Int(Keyword_set, "GROUP", K_GROUP) ;
    STR_add_to_stringtab_Int(Keyword_set, "COMPONENT", K_COMPONENT) ;
    STR_add_to_stringtab_Int(Keyword_set, "SOFT", K_SOFT) ;
    STR_add_to_stringtab_Int(Keyword_set, "MAXX", K_MAXX) ;
    STR_add_to_stringtab_Int(Keyword_set, "MAXY", K_MAXY) ;
    STR_add_to_stringtab_Int(Keyword_set, "MAXHALFPERIMETER",
      K_MAXHALFPERIMETER) ;
    STR_add_to_stringtab_Int(Keyword_set, "CONSTRAINTS", K_CONSTRAINTS) ;
    STR_add_to_stringtab_Int(Keyword_set, "NET", K_NET) ;
    STR_add_to_stringtab_Int(Keyword_set, "PATH", K_PATH) ;
    STR_add_to_stringtab_Int(Keyword_set, "SUM", K_SUM) ;
    STR_add_to_stringtab_Int(Keyword_set, "DIFF", K_DIFF) ;
    STR_add_to_stringtab_Int(Keyword_set, "RISEMIN", K_RISEMIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "RISEMAX", K_RISEMAX) ;
    STR_add_to_stringtab_Int(Keyword_set, "FALLMIN", K_FALLMIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "FALLMAX", K_FALLMAX) ;
    STR_add_to_stringtab_Int(Keyword_set, "WIREDLOGIC", K_WIREDLOGIC) ;
    STR_add_to_stringtab_Int(Keyword_set, "MAXDIST", K_MAXDIST) ;
    STR_add_to_stringtab_Int(Keyword_set, "ASSERTIONS", K_ASSERTIONS) ;
    STR_add_to_stringtab_Int(Keyword_set, "END", K_END) ;
    STR_add_to_stringtab_Int(Keyword_set, "ARRAY", K_ARRAY) ;
    STR_add_to_stringtab_Int(Keyword_set, "FLOORPLAN", K_FLOORPLAN) ;


    STR_add_to_stringtab_Int(Keyword_set, "SCANCHAINS", K_SCANCHAINS) ;
    STR_add_to_stringtab_Int(Keyword_set, "START", K_START) ;
    STR_add_to_stringtab_Int(Keyword_set, "FLOATING", K_FLOATING) ;
    STR_add_to_stringtab_Int(Keyword_set, "ORDERED", K_ORDERED) ;
    STR_add_to_stringtab_Int(Keyword_set, "STOP", K_STOP) ;
    STR_add_to_stringtab_Int(Keyword_set, "IN", K_IN) ;
    STR_add_to_stringtab_Int(Keyword_set, "OUT", K_OUT) ;

    STR_add_to_stringtab_Int(Keyword_set, "TIMINGDISABLES", K_TIMINGDISABLES) ;
    STR_add_to_stringtab_Int(Keyword_set, "IOTIMINGS", K_IOTIMINGS) ;
    STR_add_to_stringtab_Int(Keyword_set, "RISE", K_RISE) ;
    STR_add_to_stringtab_Int(Keyword_set, "FALL", K_FALL) ;
    STR_add_to_stringtab_Int(Keyword_set, "VARIABLE", K_VARIABLE) ;
    STR_add_to_stringtab_Int(Keyword_set, "SLEWRATE", K_SLEWRATE) ;
    STR_add_to_stringtab_Int(Keyword_set, "CAPACITANCE", K_CAPACITANCE) ;
    STR_add_to_stringtab_Int(Keyword_set, "DRIVECELL", K_DRIVECELL) ;
    STR_add_to_stringtab_Int(Keyword_set, "FROMPIN", K_FROMPIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "TOPIN", K_TOPIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "PARALLEL", K_PARALLEL) ;
    STR_add_to_stringtab_Int(Keyword_set, "PARTITIONS", K_PARTITIONS) ;
    STR_add_to_stringtab_Int(Keyword_set, "TURNOFF", K_TURNOFF) ;
    STR_add_to_stringtab_Int(Keyword_set, "FROMCLOCKPIN", K_FROMCLOCKPIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "FROMCOMPPIN", K_FROMCOMPPIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "FROMIOPIN", K_FROMIOPIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "TOCLOCKPIN", K_TOCLOCKPIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "TOCOMPPIN", K_TOCOMPPIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "TOIOPIN", K_TOIOPIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "SETUPRISE", K_SETUPRISE) ;
    STR_add_to_stringtab_Int(Keyword_set, "SETUPFALL", K_SETUPFALL) ;
    STR_add_to_stringtab_Int(Keyword_set, "HOLDRISE", K_HOLDRISE) ;
    STR_add_to_stringtab_Int(Keyword_set, "HOLDFALL", K_HOLDFALL) ;

    STR_add_to_stringtab_Int(Keyword_set, "VPIN", K_VPIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "SUBNET", K_SUBNET) ;
    STR_add_to_stringtab_Int(Keyword_set, "XTALK", K_XTALK) ;
    STR_add_to_stringtab_Int(Keyword_set, "PIN", K_PIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "SYNTHESIZED", K_SYNTHESIZED) ;

    STR_add_to_stringtab_Int(Keyword_set, "SPECIAL", K_SPECIAL) ;
    STR_add_to_stringtab_Int(Keyword_set, "DIRECTION", K_DIRECTION) ;
    STR_add_to_stringtab_Int(Keyword_set, "RANGE", K_RANGE) ;

    STR_add_to_stringtab_Int(Keyword_set, "FLOORPLANCONSTRAINTS", K_FPC) ;
    STR_add_to_stringtab_Int(Keyword_set, "HORIZONTAL", K_HORIZONTAL) ;
    STR_add_to_stringtab_Int(Keyword_set, "VERTICAL", K_VERTICAL) ;
    STR_add_to_stringtab_Int(Keyword_set, "ALIGN", K_ALIGN) ;
    STR_add_to_stringtab_Int(Keyword_set, "MIN", K_MIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "MAX", K_MAX) ;
    STR_add_to_stringtab_Int(Keyword_set, "EQUAL", K_EQUAL) ;
    STR_add_to_stringtab_Int(Keyword_set, "BOTTOMLEFT", K_BOTTOMLEFT) ;
    STR_add_to_stringtab_Int(Keyword_set, "TOPRIGHT", K_TOPRIGHT) ;
    STR_add_to_stringtab_Int(Keyword_set, "ROWS", K_ROWS) ;

    STR_add_to_stringtab_Int(Keyword_set, "PROPERTYDEFINITIONS", 
      K_PROPERTYDEFINITIONS) ;
    STR_add_to_stringtab_Int(Keyword_set, "PROPERTY", K_PROPERTY) ;
    STR_add_to_stringtab_Int(Keyword_set, "STRING", K_STRING) ;
    STR_add_to_stringtab_Int(Keyword_set, "REAL", K_REAL) ;
    STR_add_to_stringtab_Int(Keyword_set, "INTEGER", K_INTEGER) ;
    STR_add_to_stringtab_Int(Keyword_set, "BEGINEXT", K_BEGINEXT) ;
    STR_add_to_stringtab_Int(Keyword_set, "ENDEXT", K_ENDEXT) ;
    STR_add_to_stringtab_Int(Keyword_set, "PINPROPERTIES", K_PINPROPERTIES) ;
    STR_add_to_stringtab_Int(Keyword_set, "COMMONSCANPINS", K_COMMONSCANPINS) ;
    STR_add_to_stringtab_Int(Keyword_set, "COMPONENTPIN", K_COMPONENTPIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "SHIELD", K_SHIELD) ;
    STR_add_to_stringtab_Int(Keyword_set, "SHIELDNET", K_SHIELDNET) ;
    STR_add_to_stringtab_Int(Keyword_set, "NOSHIELD", K_NOSHIELD) ;

    STR_add_to_stringtab_Int(Keyword_set, "ANALOG", K_ANALOG) ;
    STR_add_to_stringtab_Int(Keyword_set, "ANTENNAMODEL", K_ANTENNAMODEL) ;
    STR_add_to_stringtab_Int(Keyword_set, "ANTENNAPINGATEAREA",
      K_ANTENNAPINGATEAREA) ;
    STR_add_to_stringtab_Int(Keyword_set, "ANTENNAPINDIFFAREA",
      K_ANTENNAPINDIFFAREA) ;
    STR_add_to_stringtab_Int(Keyword_set, "ANTENNAPINMAXAREACAR",
      K_ANTENNAPINMAXAREACAR) ;
    STR_add_to_stringtab_Int(Keyword_set, "ANTENNAPINMAXCUTCAR",
      K_ANTENNAPINMAXCUTCAR) ;
    STR_add_to_stringtab_Int(Keyword_set, "ANTENNAPINMAXSIDEAREACAR",
      K_ANTENNAPINMAXSIDEAREACAR) ;
    STR_add_to_stringtab_Int(Keyword_set, "ANTENNAPINPARTIALCUTAREA",
      K_ANTENNAPINPARTIALCUTAREA) ;
    STR_add_to_stringtab_Int(Keyword_set, "ANTENNAPINPARTIALMETALAREA",
      K_ANTENNAPINPARTIALMETALAREA) ;
    STR_add_to_stringtab_Int(Keyword_set, "ANTENNAPINPARTIALMETALSIDEAREA",
      K_ANTENNAPINPARTIALMETALSIDEAREA) ;
    STR_add_to_stringtab_Int(Keyword_set, "BLOCKAGES", K_BLOCKAGES) ;
    STR_add_to_stringtab_Int(Keyword_set, "BLOCKAGEWIRE", K_BLOCKAGEWIRE) ;
    STR_add_to_stringtab_Int(Keyword_set, "BLOCKRING", K_BLOCKRING) ;
    STR_add_to_stringtab_Int(Keyword_set, "BLOCKWIRE", K_BLOCKWIRE) ;
    STR_add_to_stringtab_Int(Keyword_set, "CLOCK", K_CLOCK) ;
    STR_add_to_stringtab_Int(Keyword_set, "COREWIRE", K_COREWIRE) ;
    STR_add_to_stringtab_Int(Keyword_set, "DIST", K_DIST) ;
    STR_add_to_stringtab_Int(Keyword_set, "DRCFILL", K_DRCFILL) ;
    STR_add_to_stringtab_Int(Keyword_set, "FENCE", K_FENCE) ;
    STR_add_to_stringtab_Int(Keyword_set, "FILLS", K_FILLS) ;
    STR_add_to_stringtab_Int(Keyword_set, "FILLWIRE", K_FILLWIRE) ;
    STR_add_to_stringtab_Int(Keyword_set, "FIXEDBUMP", K_FIXEDBUMP) ;
    STR_add_to_stringtab_Int(Keyword_set, "FOLLOWPIN", K_FOLLOWPIN) ;
    STR_add_to_stringtab_Int(Keyword_set, "FREQUENCY", K_FREQUENCY) ;
    STR_add_to_stringtab_Int(Keyword_set, "GUIDE", K_GUIDE) ;
    STR_add_to_stringtab_Int(Keyword_set, "GROUND", K_GROUND) ;
    STR_add_to_stringtab_Int(Keyword_set, "IOWIRE", K_IOWIRE) ;
    STR_add_to_stringtab_Int(Keyword_set, "MAXBITS", K_MAXBITS) ;
    STR_add_to_stringtab_Int(Keyword_set, "NETLIST", K_NETLIST) ;
    STR_add_to_stringtab_Int(Keyword_set, "ON", K_ON) ;
    STR_add_to_stringtab_Int(Keyword_set, "OFF", K_OFF) ;
    STR_add_to_stringtab_Int(Keyword_set, "OXIDE1", K_OXIDE1) ;
    STR_add_to_stringtab_Int(Keyword_set, "OXIDE2", K_OXIDE2) ;
    STR_add_to_stringtab_Int(Keyword_set, "OXIDE3", K_OXIDE3) ;
    STR_add_to_stringtab_Int(Keyword_set, "OXIDE4", K_OXIDE4) ;
    STR_add_to_stringtab_Int(Keyword_set, "PADRING", K_PADRING) ;
    STR_add_to_stringtab_Int(Keyword_set, "PARTITION", K_PARTITION) ;
    STR_add_to_stringtab_Int(Keyword_set, "PLACEMENT", K_PLACEMENT) ;
    STR_add_to_stringtab_Int(Keyword_set, "POWER", K_POWER) ;
    STR_add_to_stringtab_Int(Keyword_set, "PUSHDOWN", K_PUSHDOWN) ;
    STR_add_to_stringtab_Int(Keyword_set, "RESET", K_RESET) ;
    STR_add_to_stringtab_Int(Keyword_set, "RING", K_RING) ;
    STR_add_to_stringtab_Int(Keyword_set, "SCAN", K_SCAN) ;
    STR_add_to_stringtab_Int(Keyword_set, "SIGNAL", K_SIGNAL) ;
    STR_add_to_stringtab_Int(Keyword_set, "SLOTS", K_SLOTS) ;
    STR_add_to_stringtab_Int(Keyword_set, "STRIPE", K_STRIPE) ;
    STR_add_to_stringtab_Int(Keyword_set, "TIEOFF", K_TIEOFF) ;
    STR_add_to_stringtab_Int(Keyword_set, "TIMING", K_TIMING) ;
    STR_add_to_stringtab_Int(Keyword_set, "TYPE", K_TYPE) ;
    STR_add_to_stringtab_Int(Keyword_set, "USER", K_USER) ;
    STR_add_to_stringtab_Int(Keyword_set, "X", K_X) ;
    STR_add_to_stringtab_Int(Keyword_set, "Y", K_Y) ;
    STR_add_to_stringtab_Int(Keyword_set, "BALANCED", K_BALANCED) ;
    STR_add_to_stringtab_Int(Keyword_set, "STEINER", K_STEINER) ;
    STR_add_to_stringtab_Int(Keyword_set, "TRUNK", K_TRUNK) ;
    STR_add_to_stringtab_Int(Keyword_set, "TEST", K_TEST) ;
}

#define yyparse	defyyparse
#define yylex	defyylex
#define yyerror	defyyerror
#define yylval	defyylval
#define yychar	defyychar
#define yydebug	defyydebug
#define yynerrs	defyynerrs

#include "lex.cpph"
