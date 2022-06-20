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

#ifndef DEFW_WRITER_H
#define DEFW_WRITER_H

#include <stdarg.h>
#include <stdio.h>

#include "defiKRDefs.hpp"
#include "defiDefs.hpp"
#include "defiUser.hpp"

/* Return codes for writing functions: */
#define DEFW_OK                0
#define DEFW_UNINITIALIZED     1
#define DEFW_BAD_ORDER         2
#define DEFW_BAD_DATA          3
#define DEFW_ALREADY_DEFINED   4
#define DEFW_WRONG_VERSION     5
#define DEFW_OBSOLETE          6

/* orient
   0 = N
   1 = W
   2 = S
   3 = E
   4 = FN
   5 = FE
   6 = FS
   7 = FW
*/

/* This routine will write a new line */
EXTERN int defwNewLine();

/* The DEF writer initialization.  Must be called first.
 * Either this routine or defwInitCbk should be call only.
 * Can't call both routines in one program.
 * This routine is for user who does not want to use the callback machanism.
 * Returns 0 if successful. */
EXTERN int defwInit ( FILE* f, int vers1, int version2,
	      const char* caseSensitive,  /* NAMESCASESENSITIVE */
	      const char* dividerChar,    /* DIVIDERCHAR */
	      const char* busBitChars,    /* BUSBITCHARS */
	      const char* designName,     /* DESIGN */
	      const char* technology,     /* optional(NULL) - TECHNOLOGY */
	      const char* array,          /* optional(NULL) - ARRAYNAME */
	      const char* floorplan,      /* optional(NULL) - FLOORPLAN */
	      double units                /* optional  (set to -1 to ignore) */
	      );

/* The DEF writer initialization.  Must be called first.
 * Either this routine or defwInit should be call only.
 * Can't call both routines in one program.
 * This routine is for user who choose to use the callback machanism.
 * If user uses the callback for the writer, they need to provide
 * callbacks for Version, NamesCaseSensitive, BusBitChars and DividerChar.
 * These sections are required by the def.  If any of these callbacks
 * are missing, defaults will be used.
 * Returns 0 if successful. */
EXTERN int defwInitCbk (FILE* f);

/* This routine must be called after the defwInit.
 * This routine is required.
 * Returns 0 if successful.
 * The routine can be called only once. */
EXTERN int defwVersion (int vers1, int vers2);

/* This routine must be called after the defwInit.
 * This routine is required.
 * Returns 0 if successful.
 * The routine can be called only once. */
EXTERN int defwCaseSensitive ( const char* caseSensitive );

/* This routine must be called after the defwInit.
 * This routine is required.
 * Returns 0 if successful.
 * The routine can be called only once. */
EXTERN int defwBusBitChars ( const char* busBitChars );

/* This routine must be called after the defwInit.
 * This routine is required.
 * Returns 0 if successful.
 * The routine can be called only once. */
EXTERN int defwDividerChar ( const char* dividerChar );

/* This routine must be called after the defwInit.
 * This routine is required.
 * Returns 0 if successful.
 * The routine can be called only once. */
EXTERN int defwDesignName ( const char* name );

/* This routine must be called after the defwInit.
 * This routine is optional.
 * Returns 0 if successful.
 * The routine can be called only once. */
EXTERN int defwTechnology ( const char* technology );

/* This routine must be called after the defwInit.
 * This routine is optional.
 * Returns 0 if successful.
 * The routine can be called only once. */
EXTERN int defwArray ( const char* array );

/* This routine must be called after the defwInit.
 * This routine is optional.
 * Returns 0 if successful.
 * The routine can be called only once. */
EXTERN int defwFloorplan ( const char* floorplan );

/* This routine must be called after the defwInit.
 * This routine is optional.
 * Returns 0 if successful.
 * The routine can be called only once. */
EXTERN int defwUnits ( int units );

/* This routine must be called after the defwInit.
 * This routine is optional.
 * Returns 0 if successful.
 * The routine can be called 0 to many times. */
EXTERN int defwHistory ( const char* string );

/* This routine must be called after the history routines (if any).
 * This routine is optional.
 * Returns 0 if successful.
 * The routine can be called only once. */
EXTERN int defwStartPropDef ( void );

/* This routine must be called after defwStartPropDef.
 * This routine can be called multiple times.
 * It adds integer property definition to the statement.
 * Returns 0 if successfull.
 * The objType can be LIBRARY or VIA or MACRO or PIN. */
EXTERN int defwIntPropDef(
               const char* objType,   /* LIBRARY | LAYER | VIA | VIARULE |
                                         NONDEFAULTRULE | MACRO | PIN */
               const char* propName,
               double leftRange,      /* optional(0) - RANGE */
               double rightRange,     /* optional(0) */
               int    propValue);     /* optional(NULL) */
 
/* This routine must be called after defwStartPropDef.
 * This routine can be called multiple times.
 * It adds real property definition to the statement.
 * Returns 0 if successfull.
 * The objType can be LIBRARY or VIA or MACRO or PIN. */
EXTERN int defwRealPropDef(
               const char* objType,   /* LIBRARY | LAYER | VIA | VIARULE |
                                         NONDEFAULTRULE | MACRO | PIN */
               const char* propName,
               double leftRange,      /* optional(0) - RANGE */
               double rightRange,     /* optional(0) */
               double propValue);     /* optional(NULL) */
 
/* This routine must be called after defwStartPropDef.
 * This routine can be called multiple times.
 * It adds string property definition to the statement.
 * Returns 0 if successfull.
 * The objType can be LIBRARY or VIA or MACRO or PIN. */
EXTERN int defwStringPropDef(
               const char* objType,    /* LIBRARY | LAYER | VIA | VIARULE |
                                          NONDEFAULTRULE | MACRO | PIN */
               const char* propName,
               double leftRange,       /* optional(0) - RANGE */
               double rightRange,      /* optional(0) */
               const char* propValue); /* optional(NULL) */

/* This routine must be called after all the properties have been
 * added to the file.
 * If you called defwPropertyDefinitions then this routine is NOT optional.
 * Returns 0 if successful.
 * The routine can be called only once. */
EXTERN int defwEndPropDef ( void );

/* This routine can be called after defwRow, defwRegion, defwComponent,
 * defwPin, defwSpecialNet, defwNet, and defwGroup
 * This routine is optional, it adds string property to the statement.
 * Returns 0 if successful.
 * This routine can be called 0 to many times */
EXTERN int defwStringProperty(const char* propName, const char* propValue);

/* This routine can be called after defwRow, defwRegion, defwComponent,
 * defwPin, defwSpecialNet, defwNet, and defwGroup
 * This routine is optional, it adds real property to the statement.
 * Returns 0 if successful.
 * This routine can be called 0 to many times */
EXTERN int defwRealProperty(const char* propName, double propValue);

/* This routine can be called after defwRow, defwRegion, defwComponent,
 * defwPin, defwSpecialNet, defwNet, and defwGroup
 * This routine is optional, it adds int property to the statement.
 * Returns 0 if successful.
 * This routine can be called 0 to many times */
EXTERN int defwIntProperty(const char* propName, int propValue);

/* This routine must be called after the property definitions (if any).
 * This routine is optional.
 * Returns 0 if successful.
 * The routine can be called only once. */
EXTERN int defwDieArea ( int xl,            /* point1 - x */
                         int yl,            /* point1 - y */
                         int xh,            /* point2 - x */
                         int yh );          /* point2 - y */

/* This routine must be called after the Die Area (if any).
 * This routine is optional.
 * Returns 0 if successful.
 * The integer "orient" and operation of the do is explained in
 * the documentation. */
EXTERN int defwRow ( const char* rowName, const char* rowType,
		     int x_orig, int y_orig, int orient,
		     int do_count, int do_increment, int xstep, int ystep);

/* This routine must be called after the Die Area (if any).
 * This routine is optional.
 * Returns 0 if successful.
 * This routine is the same as defwRow, excpet orient is a char* */
EXTERN int defwRowStr ( const char* rowName, const char* rowType,
		        int x_orig, int y_orig, const char* orient,
		        int do_count, int do_increment, int xstep, int ystep);

/* This routine must be called after the defwRow (if any).
 * This routine is optional.
 * Returns 0 if successful.
 * The operation of the do is explained in the documentation. */
EXTERN int defwTracks ( const char* master,   /* X | Y */
                        int doStart,          /* start */
	                int doCount,          /* numTracks */
                        int doStep,           /* space */
                        int numLayers,        /* number of layers */
                        const char** layers); /* list of layers */

/* This routine must be called after the defwTracks (if any).
 * This routine is optional.
 * Returns 0 if successful.
 * The operation of the do is explained in the documentation. */
EXTERN int defwGcellGrid ( const char* master, /* X | Y */
                           int doStart,        /* start */
	                   int doCount,        /* numColumns | numRows */
                           int doStep);        /* space */

/* This routine must be called after the defwTracks (if any).
 * This section of routines is optional.
 * Returns 0 if successful.
 * The routine starts the default capacitance section.   All of the
 * capacitances must follow.
 * The count is the number of defwDefaultCap calls to follow.
 * The routine can be called only once.
 * This api is obsolete in 5.4. */
EXTERN int defwStartDefaultCap ( int count );

/* This routine is called once for each default cap.  The calls must
 * be preceeded by a call to defwStartDefaultCap and must be
 * terminated by a call to defwEndDefaultCap.
 * Returns 0 if successful.
 * This api is obsolete in 5.4. */
EXTERN int defwDefaultCap ( int pins,           /* MINPINS */
                            double cap);        /* WIRECAP */

/* This routine must be called after the defwDefaultCap calls (if any).
 * Returns 0 if successful.
 * If the count in StartDefaultCap is not the same as the number of
 * calls to DefaultCap then DEFW_BAD_DATA will return returned.
 * The routine can be called only once.
 * This api is obsolete in 5.4. */
EXTERN int defwEndDefaultCap ( void );

/* This routine must be called after the defwDefaultCap calls (if any).
 * The operation of the do is explained in the documentation.
 * This routine is optional.
 * Returns 0 if successful.
 * The routine can be called many times. */
EXTERN int defwCanPlace(const char* master,     /* sitename */
                        int xOrig,
                        int yOrig,
	                int orient,             /* 0 to 7 */
                        int doCnt,              /* numX */
                        int doInc,              /* numY */
                        int xStep,              /* spaceX */
                        int yStep);             /* spaceY */

/* This routine must be called after the defwDefaultCap calls (if any).
 * The operation of the do is explained in the documentation.
 * This routine is optional.
 * Returns 0 if successful.
 * The routine can be called many times.
 * This routine is the same as defwCanPlace, except orient is a char* */
EXTERN int defwCanPlaceStr(const char* master,     /* sitename */
                           int xOrig,
                           int yOrig,
	                   const char* orient,     /* 0 to 7 */
                           int doCnt,              /* numX */
                           int doInc,              /* numY */
                           int xStep,              /* spaceX */
                           int yStep);             /* spaceY */

/* This routine must be called after the defwCanPlace calls (if any).
 * The operation of the do is explained in the documentation.
 * This routine is optional.
 * Returns 0 if successful.
 * The routine can be called many times. */
EXTERN int defwCannotOccupy(const char* master,  /* sitename */
                            int xOrig,
                            int yOrig,
	                    int orient,          /* 0 to 7 */
                            int doCnt,           /* numX */
                            int doInc,           /* numY */
                            int xStep,           /* spaceX */
                            int yStep);          /* spaceY */

/* This routine must be called after the defwCanPlace calls (if any).
 * The operation of the do is explained in the documentation.
 * This routine is optional.
 * Returns 0 if successful.
 * The routine can be called many times.
 * This routine is the same as defwCannotOccupy, except orient is a char* */
EXTERN int defwCannotOccupyStr(const char* master,  /* sitename */
                               int xOrig,
                               int yOrig,
	                       const char* orient,  /* 0 to 7 */
                               int doCnt,           /* numX */
                               int doInc,           /* numY */
                               int xStep,           /* spaceX */
                               int yStep);          /* spaceY */

/* This routine must be called after defwCannotOccupy (if any).
 * This section of routines is optional.
 * Returns 0 if successful.
 * The routine starts the via section.   All of the vias must follow.
 * The count is the number of defwVia calls to follow.
 * The routine can be called only once. */
EXTERN int defwStartVias( int count );

/* These routines enter each via into the file.
 * These routines must be called after the defwStartVias call.
 * defwViaName should be called first, follow either by defwViaPattern or
 * defwViaLayer.  At the end of each via, defwOneViaEnd should be called
 * These routines are for [- viaName [+ PATTERNNAME patternName + RECT layerName
 * pt pt]...;]...
 * Returns 0 if successful.
 * The routines can be called many times. */
EXTERN int defwViaName(const char* name);

EXTERN int defwViaPattern(const char* patternName);

EXTERN int defwViaRect(const char* layerName,
                   int xl,         /* xl from the RECT */
                   int yl,         /* yl from the RECT */
                   int xh,         /* xh from the RECT */
                   int yh);        /* yh from the RECT */

EXTERN int defwOneViaEnd();

/* This routine must be called after the defwVia calls.
 * Returns 0 if successful.
 * If the count in StartVias is not the same as the number of
 * calls to Via or ViaPattern then DEFW_BAD_DATA will return returned.
 * The routine can be called only once. */
EXTERN int defwEndVias( void );

/* This routine must be called after via section (if any).
 * This section of routines is optional.
 * Returns 0 if successful.
 * The routine starts the region section.   All of the regions must follow.
 * The count is the number of defwRegion calls to follow.
 * The routine can be called only once. */
EXTERN int defwStartRegions( int count );

/* These routines enter each region into the file.
 * These routines must be called after the defwStartRegions call.
 * Returns 0 if successful.
 * The routine can be called many times. */
EXTERN int defwRegionName(const char* name);

/* These routines enter the region point to the region name.
 * These routines must be called after the defwRegionName call.
 * Returns 0 if successful.
 * The routine can be called many times. */
EXTERN int defwRegionPoints(int xl, int yl, int xh, int yh);

/* These routines enter the region type, FENCE | GUIDE.
 * These routines must be called after the defwRegionName call.
 * This is a 5.4.1 syntax.
 * Returns 0 if successful.
 * The routine can be called only once. */
EXTERN int defwRegionType(const char* type);  /* FENCE | GUIDE */

/* This routine must be called after the defwRegion calls.
 * Returns 0 if successful.
 * If the count in StartRegions is not the same as the number of
 * calls to Region or RegionPattern then DEFW_BAD_DATA will return returned.
 * The routine can be called only once. */
EXTERN int defwEndRegions( void );

/* This routine must be called after the regions section (if any).
 * This section of routines is NOT optional.
 * Returns 0 if successful.
 * The routine starts the components section. All of the components
 * must follow.
 * The count is the number of defwComponent calls to follow.
 * The routine can be called only once. */
EXTERN int defwStartComponents( int count );

/* These routines enter each component into the file.
 * These routines must be called after the defwStartComponents call.
 * The optional fields will be ignored if they are set to zero
 * (except for weight which must be set to -1.0).
 * Returns 0 if successful.
 * The routine can be called many times. */
EXTERN int defwComponent(const char* instance,  /* compName */
              const char* master,           /* modelName */
              int   numNetName,             /* optional(0) - # netNames defined
                                               */
              const char** netNames,        /* optional(NULL) - list */
              const char* eeq,              /* optional(NULL) - EEQMASTER */
              const char* genName,          /* optional(NULL) - GENERATE */
              const char* genParemeters,    /* optional(NULL) - parameters */
              const char* source,           /* optional(NULL) - NETLIST | DIST |
                                               USER | TIMING */
              int numForeign,               /* optional(0) - # foreigns,
                                               foreignx, foreigny & orients*/
              const char** foreigns,        /* optional(NULL) - list */
              int* foreignX, int* foreignY, /* optional(0) - list foreign pts */
              int* foreignOrients,          /* optional(-1) - 0 to 7 */
              const char* status,           /* optional(NULL) - FIXED | COVER |
                                               PLACED | UNPLACED */
              int statusX, int statusY,     /* optional(0) - status pt */
              int statusOrient,             /* optional(-1) - 0 to 7 */
              double weight,                /* optional(0) */
              const char* region,           /* optional(NULL) - either xl, yl,
                                               xh, yh or region */
              int xl, int yl,               /* optional(0) - region pt1 */
              int xh, int yh);              /* optional(0) - region pt2 */


/* These routines enter each component into the file.
 * These routines must be called after the defwStartComponents call.
 * The optional fields will be ignored if they are set to zero
 * (except for weight which must be set to -1.0).
 * Returns 0 if successful.
 * The routine can be called many times. 
 * This routine is the same as defwComponent, except orient is a char** */
EXTERN int defwComponentStr(const char* instance,  /* compName */
              const char* master,           /* modelName */
              int   numNetName,             /* optional(0) - # netNames defined
                                               */
              const char** netNames,        /* optional(NULL) - list */
              const char* eeq,              /* optional(NULL) - EEQMASTER */
              const char* genName,          /* optional(NULL) - GENERATE */
              const char* genParemeters,    /* optional(NULL) - parameters */
              const char* source,           /* optional(NULL) - NETLIST | DIST |
                                               USER | TIMING */
              int numForeign,               /* optional(0) - # foreigns,
                                               foreignx, foreigny & orients*/
              const char** foreigns,        /* optional(NULL) - list */
              int* foreignX, int* foreignY, /* optional(0) - list foreign pts */
              const char** foreignOrients,  /* optional(NULL) */
              const char* status,           /* optional(NULL) - FIXED | COVER |
                                               PLACED | UNPLACED */
              int statusX, int statusY,     /* optional(0) - status pt */
              const char* statusOrient,     /* optional(NULL) */
              double weight,                /* optional(0) */
              const char* region,           /* optional(NULL) - either xl, yl,
                                               xh, yh or region */
              int xl, int yl,               /* optional(0) - region pt1 */
              int xh, int yh);              /* optional(0) - region pt2 */


/* This routine must be called after the defwComponent calls.
 * Returns 0 if successful.
 * If the count in StartComponents is not the same as the number of
 * calls to Component then DEFW_BAD_DATA will return returned.
 * The routine can be called only once. */
EXTERN int defwEndComponents( void );

/* This routine must be called after the components section (if any).
 * This section of routines is optional.
 * Returns 0 if successful.
 * The routine starts the pins section. All of the pins must follow.
 * The count is the number of defwPin calls to follow.
 * The routine can be called only once. */
EXTERN int defwStartPins( int count );

/* These routines enter each pin into the file.
 * These routines must be called after the defwStartPins call.
 * The optional fields will be ignored if they are set to zero.
 * Returns 0 if successful.
 * The routine can be called many times. */
EXTERN int defwPin(const char* name,     /* pinName */
              const char* net,           /* netName */
              int special,               /* 0 - ignore, 1 - special */
              const char* direction,     /* optional(NULL) - INPUT | OUTPUT |
                                            INOUT | FEEDTHRU */
              const char* use,           /* optional(NULL) - SIGNAL | POWER |
                                            GROUND | CLOCK | TIEOFF | ANALOG */
              const char* status,        /* optional(NULL) - FIXED | PLACED |
                                            COVER */
              int statusX, int statusY,  /* optional(0) - status point */
              int orient,                /* optional(-1) - status orient */
              const char* layer,         /* optional(NULL) - layerName */
              int xl, int yl,            /* optional(0) - layer point1 */
              int xh, int yh);           /* optional(0) - layer point2 */

/* These routines enter each pin into the file.
 * These routines must be called after the defwStartPins call.
 * The optional fields will be ignored if they are set to zero.
 * Returns 0 if successful.
 * The routine can be called many times.
 * This routine is the same as defwPin, except orient is a char* */
EXTERN int defwPinStr(const char* name,     /* pinName */
              const char* net,           /* netName */
              int special,               /* 0 - ignore, 1 - special */
              const char* direction,     /* optional(NULL) - INPUT | OUTPUT |
                                            INOUT | FEEDTHRU */
              const char* use,           /* optional(NULL) - SIGNAL | POWER |
                                            GROUND | CLOCK | TIEOFF | ANALOG */
              const char* status,        /* optional(NULL) - FIXED | PLACED |
                                            COVER */
              int statusX, int statusY,  /* optional(0) - status point */
              const char* orient,        /* optional(NULL) */
              const char* layer,         /* optional(NULL) - layerName */
              int xl, int yl,            /* optional(0) - layer point1 */
              int xh, int yh);           /* optional(0) - layer point2 */

/* These routines must be called after defwPin.
 * Returns 0 if successful.
 * This is a 5.4 syntax.
 * The routine can be called multiple times. */
EXTERN int defwPinAntennaPinPartialMetalArea(int value,
              const char* layerName);    /* optional(NULL) */

/* These routines must be called after defwPin.
 * Returns 0 if successful.
 * This is a 5.4 syntax.
 * The routine can be called multiple times. */
EXTERN int defwPinAntennaPinPartialMetalSideArea(int value,
              const char* layerName);    /* optional(NULL) */

/* These routines must be called after defwPin.
 * Returns 0 if successful.
 * This is a 5.4 syntax.
 * The routine can be called multiple times. */
EXTERN int defwPinAntennaPinPartialCutArea(int value,
              const char* layerName);    /* optional(NULL) */

/* These routines must be called after defwPin.
 * Returns 0 if successful.
 * This is a 5.4 syntax.
 * The routine can be called multiple times. */
EXTERN int defwPinAntennaPinDiffArea(int value,
              const char* layerName);    /* optional(NULL) */

/* These routines must be called after defwPin.
 * Returns 0 if successful.
 * This is a 5.5 syntax.
 * The oxide can be either OXIDE1, OXIDE2, OXIDE3, or OXIDE4.
 * Each oxide value can be called only once after defwPin. */
EXTERN int defwPinAntennaModel(const char* oxide);

/* These routines must be called after defwPin.
 * Returns 0 if successful.
 * This is a 5.4 syntax.
 * The routine can be called multiple times. */
EXTERN int defwPinAntennaPinGateArea(int value,
              const char* layerName);    /* optional(NULL) */

/* These routines must be called after defwPin.
 * Returns 0 if successful.
 * This is a 5.4 syntax.
 * The routine can be called multiple times. */
EXTERN int defwPinAntennaPinMaxAreaCar(int value,
              const char* layerName);    /* optional(NULL) */

/* These routines must be called after defwPin.
 * Returns 0 if successful.
 * This is a 5.4 syntax.
 * The routine can be called multiple times. */
EXTERN int defwPinAntennaPinMaxSideAreaCar(int value,
              const char* layerName);    /* optional(NULL) */

/* These routines must be called after defwPin.
 * Returns 0 if successful.
 * This is a 5.4 syntax.
 * The routine can be called multiple times. */
EXTERN int defwPinAntennaPinMaxCutCar(int value,
              const char* layerName);    /* optional(NULL) */

/* This routine must be called after the defwPin calls.
 * Returns 0 if successful.
 * If the count in StartPins is not the same as the number of
 * calls to Pin then DEFW_BAD_DATA will return returned.
 * The routine can be called only once. */
EXTERN int defwEndPins( void );

/* This routine must be called after the pin section (if any).
 * This section of routines is optional.
 * Returns 0 if successful.
 * The routine starts the pinproperties section. All of the pinproperties
 * must follow.
 * The count is the number of defwPinProp calls to follow.
 * The routine can be called only once. */
EXTERN int defwStartPinProperties( int count );

/* These routines enter each pinproperty into the file.
 * These routines must be called after the defwStartPinProperties call.
 * The optional fields will be ignored if they are set to zero.
 * Returns 0 if successful.
 * The routine can be called many times. */
EXTERN int defwPinProperty(const char* name,     /* compName | PIN */
              const char* pinName);              /* pinName */

/* This routine must be called after the defwPinProperty calls.
 * Returns 0 if successful.
 * If the count in StartPins is not the same as the number of
 * calls to Pin then DEFW_BAD_DATA will return returned.
 * The routine can be called only once. */
EXTERN int defwEndPinProperties( void );

/* Routines to enter a special net or nets into the file.
 * You must first call defwStartSpecialNets with the number of
 * nets. This section is required, even if you do not have any nets.
 * For each net you should call defwSpecialNet followed by
 * one or more defwSpecialNetConnection calls.
 * After the connections come the options.  Options are
 * NOT required.
 * Each net is completed by calling defwSpecialNetEndOneNet().
 * The nets section is finished by calling defwEndNets(). */
EXTERN int defwStartSpecialNets(int count);

/* This routine must be called after the defwStartSpecialNets it is for
 * - netName */
EXTERN int defwSpecialNet(const char* name);   /* netName */

/* This routine is for compNameRegExpr, pinName, and SYNTHESIZED */
/* It can be called multiple times */
EXTERN int defwSpecialNetConnection(const char* inst,    /* compNameRegExpr */
                   const char* pin,       /* pinName */
                   int synthesized);     /* 0 - ignore, 1 - SYNTHESIZED  */

/* This routine is for + FIXEDBUMP
 * This is a 5.4.1 syntax */
EXTERN int defwSpecialNetFixedbump();

/* This routine is for + VOLTAGE volts */
EXTERN int defwSpecialNetVoltage(double v);

/* This routine is for + SPACING layerName spacing [RANGE minwidth maxwidth */
EXTERN int defwSpecialNetSpacing(const char* layer,  /* layerName */
                    int spacing,          /* spacing */
                    double minwidth,      /* optional(0) - minwidth */
                    double maxwidth);     /* optional(0) - maxwidth */

/* This routine is for + WIDTH layerName width */
EXTERN int defwSpecialNetWidth(const char* layer, /* layerName */
                    int width);                   /* width */

/* This routine is for + SOURCE {NETLIST | DIST | USER | TIMING} */
EXTERN int defwSpecialNetSource(const char* name);

/* This routine is for + ORIGINAL netName */
EXTERN int defwSpecialNetOriginal(const char* name);   /* netName */

/* This routine is for + PATTERN {STEINER | BALANCED | WIREDLOGIC | TRUNK} */
EXTERN int defwSpecialNetPattern(const char* name);

/* This routine is for + USE {SIGNAL | POWER | GROUND | CLOCK | TIEOFF |
   ANALOG | SCAN | RESET} */
EXTERN int defwSpecialNetUse(const char* name);

/* This routine is for + WEIGHT weight */
EXTERN int defwSpecialNetWeight(double value);

/* This routine is for + ESTCAP wireCapacitance */
EXTERN int defwSpecialNetEstCap(double value);

/* Paths are a special type of option.  A path must begin
 * with a defwSpecialNetPathStart and end with a defwSpecialNetPathEnd().
 * The individual parts of the path can be entered in
 * any order. */
EXTERN int defwSpecialNetPathStart(const char* typ); /* ROUTED | FIXED | COVER |
                                                        SHIELD | NEW */
EXTERN int defwSpecialNetShieldNetName(const char* name); /* shieldNetName */

EXTERN int defwSpecialNetPathLayer(const char* name); /* layerName */
EXTERN int defwSpecialNetPathWidth(int width);        /* width */
EXTERN int defwSpecialNetPathShape(const char* shapeType); /* RING | STRIPE |
        FOLLOWPIN | IOWIRE | COREWIRE | BLOCKWIRE | FILLWIRE | BLOCKAGEWIRE */

/* x and y location of the path */
EXTERN int defwSpecialNetPathPoint(int numPts,  /* number of connected points */
                   const char** pointx,     /* point x list */
                   const char** pointy);    /* point y list */
EXTERN int defwSpecialNetPathVia(const char* name);   /* viaName */

/* This routine is called after defwSpecialNetPath
 * This is a 5.4.1 syntax */
EXTERN int defwSpecialNetPathViaData(int numX, int numY, int stepX, int stepY);

EXTERN int defwSpecialNetPathEnd();

/* This routine is called at the end of each net */
EXTERN int defwSpecialNetEndOneNet();

/* 5.3 for special net */
/* Shields are a special type of option.  A shield must begin
 * with a defwSpecialNetShieldStart and end with a defwSpecialNetShieldEnd().
 * The individual parts of the shield can be entered in
 * any order. */
EXTERN int defwSpecialNetShieldStart(const char* name);

EXTERN int defwSpecialNetShieldLayer(const char* name); /* layerName */
EXTERN int defwSpecialNetShieldWidth(int width);        /* width */
EXTERN int defwSpecialNetShieldShape(const char* shapeType); /* RING | STRIPE |
        FOLLOWPIN | IOWIRE | COREWIRE | BLOCKWIRE | FILLWIRE | BLOCKAGEWIRE */
 
/* x and y location of the path */
EXTERN int defwSpecialNetShieldPoint(int numPts, /* # of connected points */
                   const char** pointx,     /* point x list */
                   const char** pointy);    /* point y list */
EXTERN int defwSpecialNetShieldVia(const char* name);   /* viaName */

/* A 5.4.1 syntax */
EXTERN int defwSpecialNetShieldViaData(int numX, int numY, int stepX, int stepY);
EXTERN int defwSpecialNetShieldEnd();
/* end 5.3 */
 
/* This routine is called at the end of the special net section */
EXTERN int defwEndSpecialNets();

/* Routines to enter a net or nets into the file.
 * You must first call defwNets with the number of nets.
 * This section is required, even if you do not have any nets.
 * For each net you should call defwNet followed by one or
 * more defwNetConnection calls.
 * After the connections come the options.  Options are
 * NOT required.
 * Each net is completed by calling defwNetEndOneNet().
 * The nets section is finished by calling defwEndNets(). */
EXTERN int defwStartNets(int count);

/* This routine must be called after the defwStartNets, it is for - netName */
EXTERN int defwNet(const char* name);

/* This routine is for { compName | PIN } pinName [+ SYNTHESIZED] */
/* It can be called multiple times */
EXTERN int defwNetConnection(const char* inst,    /* compName */
                   const char* pin,      /* pinName */
                   int synthesized);    /* 0 - ignore, 1 - SYNTHESIZED */

/* This routine is for MUSTJOIN, compName, pinName */
EXTERN int defwNetMustjoinConnection(const char*inst,  /* compName */
                   const char* pin);     /* pinName */

/* This routine is for + VPIN vpinName [LAYER layerName pt pt
 * [{ PLACED | FIXED | COVER } pt orient] */
EXTERN int defwNetVpin(const char* vpinName,
                 const char* layerName,     /* optional(NULL) */
                 int layerXl, int layerYl,  /* layer point1 */
                 int layerXh, int layerYh,  /* layer point2 */
                 const char* status,        /* optional(NULL) */
                 int statusX, int statusY,  /* optional(0) - status point */
                 int orient);               /* optional(-1) */

/* This routine is for + VPIN vpinName [LAYER layerName pt pt
 * [{ PLACED | FIXED | COVER } pt orient]
 * This routine is the same as defwNetVpin, except orient is a char* */
EXTERN int defwNetVpinStr(const char* vpinName,
                 const char* layerName,     /* optional(NULL) */
                 int layerXl, int layerYl,  /* layer point1 */
                 int layerXh, int layerYh,  /* layer point2 */
                 const char* status,        /* optional(NULL) */
                 int statusX, int statusY,  /* optional(0) - status point */
                 const char* orient);       /* optional(NULL) */

/* This routine can be called either within net or subnet.
 * it is for NONDEFAULTRULE rulename */
EXTERN int defwNetNondefaultRule(const char* name);

/* This routine is for + XTALK num */
EXTERN int defwNetXtalk(int xtalk);

/* This routine is for + FIXEDBUMP
 * This is a 5.4.1 syntax */
EXTERN int defwNetFixedbump();

/* This routine is for + FREQUENCY
 * This is a 5.4.1 syntax */
EXTERN int defwNetFrequency(double frequency);

/* This routine is for + SOURCE {NETLIST | DIST | USER | TEST | TIMING} */
EXTERN int defwNetSource(const char* name);

/* This routine is for + ORIGINAL netname */
EXTERN int defwNetOriginal(const char* name);

/* This routine is for + USE {SIGNAL | POWER | GROUND | CLOCK | TIEOFF | 
 * ANALOG} */
EXTERN int defwNetUse(const char* name);

/* This routine is for + PATTERN {STEINER | BALANCED | WIREDLOGIC} */
EXTERN int defwNetPattern(const char* name);

/* This routine is for + ESTCAP wireCapacitance */
EXTERN int defwNetEstCap(double value);

/* This routine is for + WEIGHT weight */
EXTERN int defwNetWeight(double value);

/* 5.3 for net */
/* This routine is for + SHIELDNET weight */
EXTERN int defwNetShieldnet(const char* name);

/* Noshield are a special type of option.  A noshield must begin
 * with a defwNetNoshieldStart and end with a defwNetNoshieldEnd().
 * The individual parts of the noshield can be entered in
 * any order. */
EXTERN int defwNetNoshieldStart(const char* name);

/* x and y location of the path */
EXTERN int defwNetNoshieldPoint(int numPts, /* number of connected points */
                   const char** pointx,     /* point x list */
                   const char** pointy);    /* point y list */
EXTERN int defwNetNoshieldVia(const char* name);   /* viaName */
EXTERN int defwNetNoshieldEnd();
/* end 5.3 */

/* Subnet are a special type of option. A subnet must begin
 * with a defwNetSubnetStart and end with a defwNetSubnetEnd().
 * Routines to call within the subnet are: defwNetSubnetPin,
 * defwNetNondefaultRule and defwNetPathStart... */
EXTERN int defwNetSubnetStart(const char* name);

/* This routine is called after the defwNetSubnet, it is for
 * [({compName | PIN} pinName) | (VPIN vpinName)]... */
EXTERN int defwNetSubnetPin(const char* compName,  /* compName | PIN | VPIN */
                 const char* pinName);    /* pinName | vpinName */

EXTERN int defwNetSubnetEnd();

/* Paths are a special type of option.  A path must begin
 * with a defwNetPathStart and end with a defwPathEnd().
 * The individual parts of the path can be entered in
 * any order. */
EXTERN int defwNetPathStart(const char* typ); /* ROUTED | FIXED | COVER |
                                                 NOSHIELD | NEW */
EXTERN int defwNetPathWidth(int w);           /* width */
EXTERN int defwNetPathLayer(const char* name, /* layerName */
                 int isTaper,                 /* 0 - ignore, 1 - TAPER */
                 const char* rulename);       /* only one, isTaper or */
                                              /*rulename can be assigned */

/* x and y location of the path */
EXTERN int defwNetPathPoint(int numPts,       /* number of connected points */
                   const char** pointx,       /* point x list */
                   const char** pointy,       /* point y list */
                   const char** value);       /* optional(NULL) value */

EXTERN int defwNetPathVia(const char* name);  /* viaName */
EXTERN int defwNetPathEnd();

/* This routine is called at the end of each net */
EXTERN int defwNetEndOneNet();

/* This routine is called at the end of the net section */
EXTERN int defwEndNets();

/* This section of routines is optional.
 * Returns 0 if successful.
 * The routine starts the I/O Timing section. All of the iotimings options
 * must follow.
 * The count is the number of defwIOTiming calls to follow.
 * The routine can be called only once.
 * This api is obsolete in 5.4. */
EXTERN int defwStartIOTimings(int count);

/* This routine can be called after defwStaratIOTiming
 * It is for - - {(comp pin) | (PIN name)}
 * This api is obsolete in 5.4. */
EXTERN int defwIOTiming(const char* inst,      /* compName | PIN */
                   const char* pin);           /* pinName */

/* This routine is for + { RISE | FALL } VARIABLE min max
 * This api is obsolete in 5.4. */
EXTERN int defwIOTimingVariable(const char* riseFall,  /* RISE | FALL */
                   int num1,                   /* min */
                   int num2);                  /* max */

/* This routine is for + { RISE | FALL } SLEWRATE min max
 * This api is obsolete in 5.4. */
EXTERN int defwIOTimingSlewrate(const char* riseFall,  /* RISE | FALL */
                   int num1,                   /* min */
                   int num2);                  /* max */

/* This routine is for + DRIVECELL macroName [[FROMPIN pinName] TOPIN pinName]
 * [PARALLEL numDrivers]
 * This api is obsolete in 5.4. */
EXTERN int defwIOTimingDrivecell(const char* name,   /* macroName*/
                   const char* fromPin,        /* optional(NULL) */
                   const char* toPin,          /* optional(NULL) */
                   int numDrivers);            /* optional(0) */

/* This routine is for + CAPACITANCE capacitance
 * This api is obsolete in 5.4. */
EXTERN int defwIOTimingCapacitance(double num);

/* This api is obsolete in 5.4. */
EXTERN int defwEndIOTimings();

/* Routines to enter scan chains.  This section is optional
 * The section must start with a defwStartScanchains() call and
 * end with a defwEndScanchain() call.
 * Each scan chain begins with a defwScanchain() call.
 * The rest of the calls follow.  */
EXTERN int defwStartScanchains(int count);

/* This routine can be called after defwStartScanchains
 * It is for - chainName */
EXTERN int defwScanchain(const char* name);

/* This routine is for + COMMONSCANPINS [IN pin] [OUT pin] */
EXTERN int defwScanchainCommonscanpins(
                  const char* inst1,      /* optional(NULL) - IN | OUT*/
                  const char* pin1,       /* can't be null if inst1 is set */
	          const char* inst2,      /* optional(NULL) - IN | OUT */
                  const char* pin2);      /* can't be null if inst2 is set */


/* This routine is for + PARTITION paratitionName [MAXBITS maxBits] */
/* This is 5.4.1 syntax */
EXTERN int defwScanchainPartition(const char* name,
                  int maxBits);           /* optional(-1) */

/* This routine is for + START {fixedInComp | PIN } [outPin] */
EXTERN int defwScanchainStart(const char* inst,   /* fixedInComp | PIN */
                  const char* pin);               /* outPin */

/* This routine is for + STOP {fixedOutComp | PIN } [inPin] */
EXTERN int defwScanchainStop(const char* inst,    /* fixedOutComp | PIN */
                  const char* pin);               /* inPin */

/* This routine is for + FLOATING {floatingComp [IN pin] [OUT pin]}
 * This is a 5.4.1 syntax */
EXTERN int defwScanchainFloating(const char* name,   /* floatingComp */
                  const char* inst1,      /* optional(NULL) - IN | OUT */
                  const char* pin1,       /* can't be null if inst1 is set */
	          const char* inst2,      /* optional(NULL) - IN | OUT */
                  const char* pin2);      /* can't be null if inst2 is set */

/* This routine is for + FLOATING {floatingComp [IN pin] [OUT pin]}
 * This is a 5.4.1 syntax.
 * This routine is the same as defwScanchainFloating.  But also added
 * the option BITS. */
EXTERN int defwScanchainFloatingBits(const char* name,   /* floatingComp */
                  const char* inst1,      /* optional(NULL) - IN | OUT */
                  const char* pin1,       /* can't be null if inst1 is set */
	          const char* inst2,      /* optional(NULL) - IN | OUT */
                  const char* pin2,       /* can't be null if inst2 is set */
                  int   bits);            /* optional (-1) */

/* This routine is for + ORDERED {fixedComp [IN pin] [OUT pin]
 * fixedComp [IN pin] [OUT pin].
 * When this routine is called for the 1st time within a scanchain, 
 * both name1 and name2 are required.  Only name1 is required is the
 * routine is called more than once. */
EXTERN int defwScanchainOrdered(const char* name1,
                  const char* inst1,      /* optional(NULL) - IN | OUT */
                  const char* pin1,       /* can't be null if inst1 is set */
	          const char* inst2,      /* optional(NULL) - IN | OUT */
                  const char* pin2,       /* can't be null if inst2 is set */
                  const char* name2,
                  const char* inst3,      /* optional(NULL) - IN | OUT */
                  const char* pin3,       /* can't be null if inst3 is set */
	          const char* inst4,      /* optional(NULL) - IN | OUT */
                  const char* pin4);      /* can't be null if inst4 is set */

/* This routine is for + ORDERED {fixedComp [IN pin] [OUT pin]
 * fixedComp [IN pin] [OUT pin].
 * When this routine is called for the 1st time within a scanchain, 
 * both name1 and name2 are required.  Only name1 is required is the
 * routine is called more than once.
 * This is a 5.4.1 syntax.
 * This routine is the same as defwScanchainOrdered.  But also added
 * the option BITS */
EXTERN int defwScanchainOrderedBits(const char* name1,
                  const char* inst1,      /* optional(NULL) - IN | OUT */
                  const char* pin1,       /* can't be null if inst1 is set */
	          const char* inst2,      /* optional(NULL) - IN | OUT */
                  const char* pin2,       /* can't be null if inst2 is set */
                  int   bits1,            /* optional(-1) */
                  const char* name2,
                  const char* inst3,      /* optional(NULL) - IN | OUT */
                  const char* pin3,       /* can't be null if inst3 is set */
	          const char* inst4,      /* optional(NULL) - IN | OUT */
                  const char* pin4,       /* can't be null if inst4 is set */
                  int   bits2);           /* optional(-1) */

EXTERN int defwEndScanchain();

/* Routines to enter constraints.  This section is optional
 * The section must start with a defwStartConstrains() call and
 * end with a defwEndConstraints() call.
 * Each contraint will call the defwConstraint...().
 * This api is obsolete in 5.4. */
EXTERN int defwStartConstraints (int count);  /* optional */

/* The following routines are for - {operand [+ RISEMAX time] [+ FALLMAX time]
 * [+ RISEMIN time] [+ FALLMIN time] | WIREDLOGIC netName MAXDIST distance };}
 * operand - NET netName | PATH comp fromPin comp toPin | SUM (operand, ...)
 * The following apis are obsolete in 5.4. */ 
EXTERN int defwConstraintOperand();          /* begin an operand */
EXTERN int defwConstraintOperandNet(const char* netName);  /* NET */
EXTERN int defwConstraintOperandPath(const char* comp1,    /* PATH - comp|PIN */
                  const char* fromPin,
                  const char* comp2,
                  const char* toPin);
EXTERN int defwConstraintOperandSum();        /* SUM */
EXTERN int defwConstraintOperandSumEnd();     /* mark the end of SUM */
EXTERN int defwConstraintOperandTime(const char* timeType, /* RISEMAX |
                                                 FALLMAX | RISEMIN | FALLMIN */
                  int time);
EXTERN int defwConstraintOperandEnd();        /* mark the end of operand */

/* This routine is for - WIRELOGIC netName MAXDIST distance */
EXTERN int defwConstraintWiredlogic(const char* netName,
                  int distance);

EXTERN int defwEndConstraints ();

/* Routines to enter groups.  This section is optional
 * The section must start with a defwStartGroups() call and
 * end with a defwEndGroups() call.
 * Each group will call the defwGroup...(). */
EXTERN int defwStartGroups (int count);  /* optional */

/* This routine is for - groupName compNameRegExpr ... */
EXTERN int defwGroup(const char* groupName,
                   int numExpr,
                   const char** groupExpr);

/* This routine is for + SOFT [MAXHALFPERIMETER value] [MAXX value]
 * [MAXY value] */
EXTERN int defwGroupSoft(const char* type1, /* MAXHALFPERIMETER | MAXX | MAXY */
                   double value1,
                   const char* type2,
                   double value2,
                   const char* type3,
                   double value3);

/* This routine is for + REGION {pt pt | regionName} */
EXTERN int defwGroupRegion(int xl, int yl,   /* either the x & y or    */
                   int xh, int yh,           /* regionName only, can't */
                   const char* regionName);  /* be both */

EXTERN int defwEndGroups();

/* Routines to enter Blockages.  This section is optional
 * The section must start with a defwStartBlockages() call and
 * end with a defwEndBlockages() call.
 * Each blockage will call the defwBlockage...().
 * This is a 5.4 syntax. */
EXTERN int defwStartBlockages(int count);   /* count = numBlockages */

/* This routine is for - layerName & compName
 * Either this routine, defBlockageLayerSlots, defBlockageLayerFills,
 * or defwBlockagePlacement is called per entry within
 * a blockage, can't be more then one.
 * This is a 5.4 syntax. */
EXTERN int defwBlockageLayer(const char* layerName,
                   const char* compName);   /* optional(NULL) */

/* This routine is for - layerName & slots 
 * Either this routine, defBlockageLayer, defBlockageLayerFills,
 * defwBlockagePlacement, or defwBlockagePushdown is called per entry within
 * a blockage, can't be more then one.
 * This is a 5.4 syntax. */
EXTERN int defwBlockageLayerSlots(const char* layerName);

/* This routine is for - layerName & fills
 * Either this routine, defBlockageLayer, defBlockageLayerSlots,
 * defwBlockagePlacement, or defwBlockagePushdown is called per entry within
 * a blockage, can't be more then one.
 * This is a 5.4 syntax. */
EXTERN int defwBlockageLayerFills(const char* layerName);

/* This routine is for - layerName & pushdown
 * Either this routine, defBlockageLayer, defBlockageLayerSlots,
 * defwBlockagePlacement, or defwBlockageFills is called per entry within
 * a blockage, can't be more then one.
 * This is a 5.4 syntax. */
EXTERN int defwBlockageLayerPushdown(const char* layerName);

/* This routine is for - placement
 * Either this routine, defBlockageLayer, defBlockageLayerSlots,
 * defwBlockageLayerFills, defwBlockageLayerPushdown,
 * defwBlockagePlacementComponent, or defwBlockagePlacementPushdown
 * is called per entry within blockage, can't be more then one.
 * This is a 5.4 syntax.
 * 11/25/2002 - Added the changed made by Craig Files from Agilent
 * this routine allows to call blockage without a component. */
EXTERN int defwBlockagePlacement();

/* This routine is for - placement & component
 * Either this routine, defBlockageLayer, defBlockageLayerSlots,
 * defwBlockageLayerFills, defwBlockageLayerPushdown,
 * defwBlockagePlacement, or defwBlockagePlacementPushdown
 * is called per entry within blockage, can't be more then one.
 * This is a 5.4 syntax. */
EXTERN int defwBlockagePlacementComponent(const char* compName);

/* This routine is for - placement & Pushdown
 * Either this routine, defBlockageLayer, defBlockageLayerSlots,
 * defwBlockageLayerFills, defwBlockageLayerPushdown,
 * defwBlockagePlacement, or defwBlockagePlacementComponenT
 * is called per entry within blockage, can't be more then one.
 * This is a 5.4 syntax. */
EXTERN int defwBlockagePlacementPushdown();

/* This routine is for rectangle.
 * This is a 5.4 syntax. */
EXTERN int defwBlockageRect(int xl, int yl, int xh, int yh);

/* This is a 5.4 syntax. */
EXTERN int defwEndBlockages();

/* Routines to enter Slots.  This section is optional
 * The section must start with a defwStartSlots() call and
 * end with a defwEndSlots() call.
 * Each slots will call the defwSlots...().
 * This is a 5.4 syntax. */
EXTERN int defwStartSlots(int count);   /* count = numSlots */

/* This routine is for - layerName & compName
 * Either this routine, defSlots, defSlotsLayerFills,
 * or defwSlotsPlacement is called per entry within
 * a slot, can't be more then one.
 * This is a 5.4 syntax. */
EXTERN int defwSlotLayer(const char* layerName);

/* This routine is for rectangle
 * This is a 5.4 syntax. */
EXTERN int defwSlotRect(int xl, int yl, int xh, int yh);

/* This is a 5.4 syntax. */
EXTERN int defwEndSlots();

/* Routines to enter Fills.  This section is optional
 * The section must start with a defwStartFills() call and
 * end with a defwEndFills() call.
 * Each fills will call the defwFills...().
 * This is a 5.4 syntax. */
EXTERN int defwStartFills(int count);   /* count = numFills */

/* This routine is for - layerName & compName
 * Either this routine, defFills, defFillsLayerFills,
 * or defwFillsPlacement is called per entry within
 * a fill, can't be more then one.
 * This is a 5.4 syntax. */
EXTERN int defwFillLayer(const char* layerName);

/* This routine is for rectangle.
 * This is a 5.4 syntax. */
EXTERN int defwFillRect(int xl, int yl, int xh, int yh);

/* This is a 5.4 syntax. */
EXTERN int defwEndFills();

/* This routine is called after defwInit.
 * This routine is optional and it can be called only once.
 * Returns 0 if successful. */
EXTERN int defwStartBeginext(const char* name);

/* This routine is called after defwBeginext.
 * This routine is optional, it can be called only once.
 * Returns 0 if successful. */
EXTERN int defwBeginextCreator (const char* creatorName);

/* This routine is called after defwBeginext.
 * This routine is optional, it can be called only once.
 * It gets the current system time and date.
 * Returns 0 if successful. */
EXTERN int defwBeginextDate ();

/* This routine is called after defwBeginext.
 * This routine is optional, it can be called only once.
 * Returns 0 if successful. */
EXTERN int defwBeginextRevision (int vers1, int vers2);   /* vers1.vers2 */

/* This routine is called after defwBeginext.
 * This routine is optional, it can be called many times.
 * It allows user to customize their own syntax.
 * Returns 0 if successful. */
EXTERN int defwBeginextSyntax (const char* title, const char* string);

/* This routine is called after defwInit.
 * This routine is optional and it can be called only once.
 * Returns 0 if successful. */
EXTERN int defwEndBeginext();

/* End the DEF file.
 * This routine IS NOT OPTIONAL.
 * The routine must be called LAST. */
EXTERN int defwEnd ( void );

/* General routines that can be called anytime after the Init is called.
 */
EXTERN int defwCurrentLineNumber ( void );

/*
 * EXTERN void defwError ( const char *, ... );
 * EXTERN void defwWarning ( const char *, ... );
 * EXTERN void defwVError ( const char *, va_list );
 * EXTERN void defwVWarning ( const char *, va_list );
 * EXTERN int  defwGetCurrentLineNumber (void);
 * EXTERN const char *defwGetCurrentFileName (void);
 */
 
/* This routine will print the error message. */
EXTERN void defwPrintError(int status);

/* This routine will allow user to write their own comemnt.  It will
 * automactically add a # infront of the line.
 */
EXTERN void defwAddComment(const char* comment);
 
/* This routine will indent 3 blank spaces */
EXTERN void defwAddIndent();

#endif
