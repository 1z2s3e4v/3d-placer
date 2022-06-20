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

#ifndef defiDebug_h
#define defiDebug_h 1

#include "defiKRDefs.hpp"

/* Set flag */
EXTERN void defiSetDebug PROTO_PARAMS((int num, int value)) ;

/* Read flag */
EXTERN int defiDebug PROTO_PARAMS((int num)) ;



/* Error loggin function */
extern void defiError(const char* message);

/* for auto upshifting names in case insensitive files */
extern int names_case_sensitive;
extern const char* upperCase(const char* c);
#define DEFCASE(x) (names_case_sensitive ? x : upperCase(x)) 

#endif
