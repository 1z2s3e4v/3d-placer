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

#ifndef DEFRREADER_H
#define DEFRREADER_H

#include <stdarg.h>

#include "defiKRDefs.hpp"
#include "defiDefs.hpp"
#include "defiUser.hpp"



/* The reader initialization.  Must be called before defrRead().
 */
EXTERN int defrInit PROTO_PARAMS(( void ));

/* Change the comment character in the DEF file.  The default
 * is '#' */
EXTERN void defrSetCommentChar PROTO_PARAMS (( char c ));

/* Functions to call to set specific actions in the parser.
 */
EXTERN void defrSetAddPathToNet PROTO_PARAMS((void));
EXTERN void defrSetAllowComponentNets PROTO_PARAMS((void));
EXTERN void defrSetCaseSensitivity PROTO_PARAMS((int caseSense));

/* Functions to keep track of callbacks that the user did not
 * supply.  Normally all parts of the DEF file that the user
 * does not supply a callback for will be ignored.  These
 * routines tell the parser count the DEF constructs that are
 * present in the input file, but did not trigger a callback.
 * This should help you find any "important" DEF constructs that
 * you are ignoring.
 */
EXTERN void defrSetRegisterUnusedCallbacks PROTO_PARAMS((void));
EXTERN void defrPrintUnusedCallbacks PROTO_PARAMS((FILE* log));
EXTERN int* defrUnusedCallbackCount PROTO_PARAMS((void));

/*
 * The main reader function.
 * The file should already be opened.  This requirement allows
 * the reader to be used with stdin or a pipe.  The file name
 * is only used for error messages.
 */
EXTERN int defrRead
  PROTO_PARAMS(( FILE *file,
                 const char *fileName,
                 defiUserData userData,
                 int case_sensitive ));

/*
 * Set/get the client-provided user data.  defi doesn't look at
 * this data at all, it simply passes the opaque defiUserData pointer
 * back to the application with each callback.  The client can
 * change the data at any time, and it will take effect on the
 * next callback.  The defi reader and writer maintain separate
 * user data pointers.
 */
EXTERN void defrSetUserData PROTO_PARAMS(( defiUserData ));
EXTERN defiUserData defrGetUserData PROTO_PARAMS(( void ));
 
/*
 * The error and warning functions.  These take a printf style
 * argument list, and will call the corresponding callbacks
 * (defrErrorCbkFnType and defrWarningCbkFnType) if they are supplied.
 * The messages are formatted before calling the callbacks.
 */
/*
 * EXTERN void defrError PROTO_PARAMS(( const char *, ... ));
 * EXTERN void defrWarning PROTO_PARAMS(( const char *, ... ));
 * EXTERN void defrVError PROTO_PARAMS(( const char *, va_list ));
 * EXTERN void defrVWarning PROTO_PARAMS(( const char *, va_list ));
 * EXTERN int  defrGetCurrentLineNumber PROTO_PARAMS((void));
 * EXTERN const char *defrGetCurrentFileName PROTO_PARAMS((void));
 */
 
/*
 * An enum describing all of the types of reader callbacks.
 */
typedef enum {
  defrUnspecifiedCbkType = 0,
  defrDesignStartCbkType,
  defrTechNameCbkType,
  defrPropCbkType,
  defrPropDefEndCbkType,
  defrPropDefStartCbkType,
  defrFloorPlanNameCbkType,
  defrArrayNameCbkType,
  defrUnitsCbkType,
  defrDividerCbkType,
  defrBusBitCbkType,
  defrSiteCbkType,
  defrComponentStartCbkType,
  defrComponentCbkType,
  defrComponentEndCbkType,
  defrNetStartCbkType,
  defrNetCbkType,
  defrNetEndCbkType,
  defrPathCbkType,
  defrVersionCbkType,
  defrVersionStrCbkType,
  defrComponentExtCbkType,
  defrPinExtCbkType,
  defrViaExtCbkType,
  defrNetConnectionExtCbkType,
  defrNetExtCbkType,
  defrGroupExtCbkType,
  defrScanChainExtCbkType,
  defrIoTimingsExtCbkType,
  defrPartitionsExtCbkType,
  defrHistoryCbkType,
  defrDieAreaCbkType,
  defrCanplaceCbkType,
  defrCannotOccupyCbkType,
  defrPinCapCbkType,
  defrDefaultCapCbkType,
  defrStartPinsCbkType,
  defrPinCbkType,
  defrPinEndCbkType,
  defrRowCbkType,
  defrTrackCbkType,
  defrGcellGridCbkType,
  defrViaStartCbkType,
  defrViaCbkType,
  defrViaEndCbkType,
  defrRegionStartCbkType,
  defrRegionCbkType,
  defrRegionEndCbkType,
  defrSNetStartCbkType,
  defrSNetCbkType,
  defrSNetEndCbkType,
  defrGroupsStartCbkType,
  defrGroupNameCbkType,
  defrGroupMemberCbkType,
  defrGroupCbkType,
  defrGroupsEndCbkType,
  defrAssertionsStartCbkType,
  defrAssertionCbkType,
  defrAssertionsEndCbkType,
  defrConstraintsStartCbkType,
  defrConstraintCbkType,
  defrConstraintsEndCbkType,
  defrScanchainsStartCbkType,
  defrScanchainCbkType,
  defrScanchainsEndCbkType,
  defrIOTimingsStartCbkType,
  defrIOTimingCbkType,
  defrIOTimingsEndCbkType,
  defrFPCStartCbkType,
  defrFPCCbkType,
  defrFPCEndCbkType,
  defrTimingDisablesStartCbkType,
  defrTimingDisableCbkType,
  defrTimingDisablesEndCbkType,
  defrPartitionsStartCbkType,
  defrPartitionCbkType,
  defrPartitionsEndCbkType,
  defrPinPropStartCbkType,
  defrPinPropCbkType,
  defrPinPropEndCbkType,
  defrBlockageStartCbkType,
  defrBlockageCbkType,
  defrBlockageEndCbkType,
  defrSlotStartCbkType,
  defrSlotCbkType,
  defrSlotEndCbkType,
  defrFillStartCbkType,
  defrFillCbkType,
  defrFillEndCbkType,
  defrCaseSensitiveCbkType,

  /* NEW CALLBACK - If you are creating a new callback, you must add
   * a unique item to this enum for each callback routine. When the
   * callback is called in def.y you have to supply this enum item
   * as an argument in the call. */

  defrDesignEndCbkType
} defrCallbackType_e;
 
 
/* Declarations of function signatures for each type of callback.
 * These declarations are type-safe when compiling with ANSI C
 * or C++; you will only be able to register a function pointer
 * with the correct signature for a given type of callback.
 *
 * Each callback function is expected to return 0 if successful.
 * A non-zero return code will cause the reader to abort.
 *
 * The defrDesignStart and defrDesignEnd callback is only called once.
 * Other callbacks may be called multiple times, each time with a different
 * set of data.
 *
 * For each callback, the Def API will make the callback to the
 * function supplied by the client, which should either make a copy
 * of the Def object, or store the data in the client's own data structures.
 * The Def API will delete or reuse each object after making the callback,
 * so the client should not keep a pointer to it.
 *
 * All callbacks pass the user data pointer provided in defrRead()
 * or defrSetUserData() back to the client; this can be used by the
 * client to obtain access to the rest of the client's data structures.
 *
 * The user data pointer is obtained using defrGetUserData() immediately
 * prior to making each callback, so the client is free to change the
 * user data on the fly if necessary.
 *
 * Callbacks with the same signature are passed a callback type
 * parameter, which allows an application to write a single callback
 * function, register that function for multiple callbacks, then
 * switch based on the callback type to handle the appropriate type of
 * data.
 */
 

/* A declaration of the signature of all callbacks that return nothing. */
typedef int (*defrVoidCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
		   void* v,
                   defiUserData ));

/* A declaration of the signature of all callbacks that return a string. */
typedef int (*defrStringCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   const char *string,
                   defiUserData ));
 
/* A declaration of the signature of all callbacks that return a integer. */
typedef int (*defrIntegerCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   int number,
                   defiUserData ));
 
/* A declaration of the signature of all callbacks that return a double. */
typedef int (*defrDoubleCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   double number,
                   defiUserData ));
 
/* A declaration of the signature of all callbacks that return a defiProp. */
typedef int (*defrPropCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiProp *prop,
                   defiUserData ));
 
/* A declaration of the signature of all callbacks that return a defiSite. */
typedef int (*defrSiteCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiSite *site,
                   defiUserData ));
 
/* A declaration of the signature of all callbacks that return a defComponent. */
typedef int (*defrComponentCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiComponent *comp,
                   defiUserData ));
 

/* A declaration of the signature of all callbacks that return a defNet. */
typedef int (*defrNetCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiNet *net,
                   defiUserData ));

 
/* A declaration of the signature of all callbacks that return a defPath. */
typedef int (*defrPathCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiPath *path,
                   defiUserData ));
 
/* A declaration of the signature of all callbacks that return a defiBox. */
typedef int (*defrBoxCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiBox *box,
                   defiUserData ));
 

/* A declaration of the signature of all callbacks that return a defiPinCap. */
typedef int (*defrPinCapCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiPinCap *pincap,
                   defiUserData ));
 

/* A declaration of the signature of all callbacks that return a defiPin. */
typedef int (*defrPinCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiPin *pin,
                   defiUserData ));
 

/* A declaration of the signature of all callbacks that return a defiRow. */
typedef int (*defrRowCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiRow *row,
                   defiUserData ));
 

/* A declaration of the signature of all callbacks that return a defiTrack. */
typedef int (*defrTrackCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiTrack *track,
                   defiUserData ));
 

/* A declaration of the signature of all callbacks that return a defiGcellGrid. */
typedef int (*defrGcellGridCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiGcellGrid *grid,
                   defiUserData ));
 

/* A declaration of the signature of all callbacks that return a defiVia. */
typedef int (*defrViaCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiVia *,
                   defiUserData ));
 

/* A declaration of the signature of all callbacks that return a defiRegion. */
typedef int (*defrRegionCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiRegion *,
                   defiUserData ));
 

/* A declaration of the signature of all callbacks that return a defiGroup. */
typedef int (*defrGroupCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiGroup *,
                   defiUserData ));
 

/* A declaration of the signature of all callbacks that return a defiAssertion. */
typedef int (*defrAssertionCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiAssertion *,
                   defiUserData ));
 

/* A declaration of the signature of all callbacks that return a defiScanChain. */
typedef int (*defrScanchainCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiScanchain *,
                   defiUserData ));
 

/* A declaration of the signature of all callbacks that return a defiIOTiming. */
typedef int (*defrIOTimingCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiIOTiming *,
                   defiUserData ));
 

/* A declaration of the signature of all callbacks that return a defiFPC. */
typedef int (*defrFPCCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiFPC *,
                   defiUserData ));
 

/* A declaration of the signature of all callbacks that return a defiTimingDisable. */
typedef int (*defrTimingDisableCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiTimingDisable *,
                   defiUserData ));
 

/* A declaration of the signature of all callbacks that return a defiPartition. */
typedef int (*defrPartitionCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiPartition *,
                   defiUserData ));
 
/* A declaration of the signature of all callbacks that return a defiPinProp. */
typedef int (*defrPinPropCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiPinProp *,
                   defiUserData ));

/* A declaration of the signature of all callbacks that return a defiBlockage. */
typedef int (*defrBlockageCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiBlockage *,
                   defiUserData ));

/* A declaration of the signature of all callbacks that return a defiSlot. */
typedef int (*defrSlotCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiSlot *,
                   defiUserData ));

/* A declaration of the signature of all callbacks that return a defiFill. */
typedef int (*defrFillCbkFnType)
    PROTO_PARAMS(( defrCallbackType_e,
                   defiFill *,
                   defiUserData ));

/* NEW CALLBACK - Each callback must return user data, enum, and
 *   OUR-DATA item.  We must define a callback function type for
 *   each type of OUR-DATA.  Some routines return a string, some
 *   return an integer, and some return a pointer to a class.
 *   If you create a new class, then you must create a new function
 *   type here to return that class to the user. */
 



/* Functions to call to register a callback function or get the function
 * pointer after it has been registered.
 */

/* Register one function for all callbacks with the same signature */
EXTERN void defrSetArrayNameCbk (defrStringCbkFnType);
EXTERN void defrSetAssertionCbk (defrAssertionCbkFnType);
EXTERN void defrSetAssertionsStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetAssertionsEndCbk (defrVoidCbkFnType);
EXTERN void defrSetBusBitCbk (defrStringCbkFnType);
EXTERN void defrSetCannotOccupyCbk (defrSiteCbkFnType);
EXTERN void defrSetCanplaceCbk (defrSiteCbkFnType);
EXTERN void defrSetComponentCbk (defrComponentCbkFnType);
EXTERN void defrSetComponentExtCbk (defrStringCbkFnType);
EXTERN void defrSetComponentStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetComponentEndCbk (defrVoidCbkFnType);
EXTERN void defrSetConstraintCbk (defrAssertionCbkFnType);
EXTERN void defrSetConstraintsStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetConstraintsEndCbk (defrVoidCbkFnType);
EXTERN void defrSetDefaultCapCbk (defrIntegerCbkFnType);
EXTERN void defrSetDesignCbk (defrStringCbkFnType);
EXTERN void defrSetDesignEndCbk (defrVoidCbkFnType);
EXTERN void defrSetDieAreaCbk (defrBoxCbkFnType);
EXTERN void defrSetDividerCbk (defrStringCbkFnType);
EXTERN void defrSetFPCCbk (defrFPCCbkFnType);
EXTERN void defrSetFPCStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetFPCEndCbk (defrVoidCbkFnType);
EXTERN void defrSetFloorPlanNameCbk (defrStringCbkFnType);
EXTERN void defrSetGcellGridCbk (defrGcellGridCbkFnType);
EXTERN void defrSetGroupNameCbk (defrStringCbkFnType);
EXTERN void defrSetGroupMemberCbk (defrStringCbkFnType);
EXTERN void defrSetGroupCbk (defrGroupCbkFnType);
EXTERN void defrSetGroupExtCbk (defrStringCbkFnType);
EXTERN void defrSetGroupsStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetGroupsEndCbk (defrVoidCbkFnType);
EXTERN void defrSetHistoryCbk (defrStringCbkFnType);
EXTERN void defrSetIOTimingCbk (defrIOTimingCbkFnType);
EXTERN void defrSetIOTimingsStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetIOTimingsEndCbk (defrVoidCbkFnType);
EXTERN void defrSetIoTimingsExtCbk (defrStringCbkFnType);
EXTERN void defrSetNetCbk (defrNetCbkFnType);
EXTERN void defrSetNetConnectionExtCbk (defrStringCbkFnType);
EXTERN void defrSetNetExtCbk (defrStringCbkFnType);
EXTERN void defrSetNetStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetNetEndCbk (defrVoidCbkFnType);
EXTERN void defrSetPartitionCbk (defrPartitionCbkFnType);
EXTERN void defrSetPartitionsExtCbk (defrStringCbkFnType);
EXTERN void defrSetPartitionsStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetPartitionsEndCbk (defrVoidCbkFnType);
EXTERN void defrSetPathCbk (defrPathCbkFnType);
EXTERN void defrSetPinCapCbk (defrPinCapCbkFnType);
EXTERN void defrSetPinCbk (defrPinCbkFnType);
EXTERN void defrSetPinExtCbk (defrStringCbkFnType);
EXTERN void defrSetPropCbk (defrPropCbkFnType);
EXTERN void defrSetPropDefEndCbk (defrVoidCbkFnType);
EXTERN void defrSetPropDefStartCbk (defrVoidCbkFnType);
EXTERN void defrSetRegionCbk (defrRegionCbkFnType);
EXTERN void defrSetRegionStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetRegionEndCbk (defrVoidCbkFnType);
EXTERN void defrSetRowCbk (defrRowCbkFnType);
EXTERN void defrSetSNetCbk (defrNetCbkFnType);
EXTERN void defrSetSNetStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetSNetEndCbk (defrVoidCbkFnType);
EXTERN void defrSetScanChainExtCbk (defrStringCbkFnType);
EXTERN void defrSetScanchainCbk (defrScanchainCbkFnType);
EXTERN void defrSetScanchainsStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetScanchainsEndCbk (defrVoidCbkFnType);
EXTERN void defrSetSiteCbk (defrSiteCbkFnType);
EXTERN void defrSetStartPinsCbk (defrIntegerCbkFnType);
EXTERN void defrSetPinEndCbk (defrVoidCbkFnType);
EXTERN void defrSetTechnologyCbk (defrStringCbkFnType);
EXTERN void defrSetTimingDisableCbk (defrTimingDisableCbkFnType);
EXTERN void defrSetTimingDisablesStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetTimingDisablesEndCbk (defrVoidCbkFnType);
EXTERN void defrSetTrackCbk (defrTrackCbkFnType);
EXTERN void defrSetUnitsCbk (defrDoubleCbkFnType);
EXTERN void defrSetVersionCbk (defrDoubleCbkFnType);
EXTERN void defrSetVersionStrCbk (defrStringCbkFnType);
EXTERN void defrSetViaCbk (defrViaCbkFnType);
EXTERN void defrSetViaExtCbk (defrStringCbkFnType);
EXTERN void defrSetViaStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetViaEndCbk (defrVoidCbkFnType);
EXTERN void defrSetPinPropCbk (defrPinPropCbkFnType);
EXTERN void defrSetPinPropStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetPinPropEndCbk (defrVoidCbkFnType);
EXTERN void defrSetCaseSensitiveCbk (defrIntegerCbkFnType);
EXTERN void defrSetBlockageCbk (defrBlockageCbkFnType);
EXTERN void defrSetBlockageStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetBlockageEndCbk (defrVoidCbkFnType);
EXTERN void defrSetSlotCbk (defrSlotCbkFnType);
EXTERN void defrSetSlotStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetSlotEndCbk (defrVoidCbkFnType);
EXTERN void defrSetFillCbk (defrFillCbkFnType);
EXTERN void defrSetFillStartCbk (defrIntegerCbkFnType);
EXTERN void defrSetFillEndCbk (defrVoidCbkFnType);

/* NEW CALLBACK - For each new callback you create, you must
 *  create a routine that allows the user to set it.  Add the
 *  setting routines here. */

/* Routine to set all unused callbacks. This is useful for checking
 * to see if you missed something. */
EXTERN void defrSetUnusedCallbacks PROTO_PARAMS ((defrVoidCbkFnType func ));


/* Return the current line number in the input file. */
EXTERN int defrLineNumber PROTO_PARAMS(( void ));

/* Routine to set the message logging routine for errors */
typedef void (*DEFI_LOG_FUNCTION)(const char*);
void defrSetLogFunction( DEFI_LOG_FUNCTION );

/* Routine to set the message logging routine for warnings */
typedef void (*DEFI_WARNING_LOG_FUNCTION)(const char*);
void defrSetWarningLogFunction( DEFI_WARNING_LOG_FUNCTION );

/* Routine to set the user defined malloc routine */
typedef void* (*DEFI_MALLOC_FUNCTION)(int);
void defrSetMallocFunction( DEFI_MALLOC_FUNCTION );

/* Routine to set the user defined realloc routine */
typedef void* (*DEFI_REALLOC_FUNCTION)(void*, int);
void defrSetReallocFunction( DEFI_REALLOC_FUNCTION );

/* Routine to set the user defined free routine */
typedef void (*DEFI_FREE_FUNCTION)(void *);
void defrSetFreeFunction( DEFI_FREE_FUNCTION );

/* Routine to set the line number of the file that is parsing routine */
typedef void (*DEFI_LINE_NUMBER_FUNCTION) (int);
void defrSetLineNumberFunction( DEFI_LINE_NUMBER_FUNCTION );

/* Set the number of lines before calling the line function callback routine */
/* Default is 10000 */
void defrSetDeltaNumberLines ( int );

/* Return codes for the user callbacks.
 * The user should return one of these values. */
#define PARSE_OK 0      /* continue parsing */
#define STOP_PARSE 1    /* stop parsing with no error message */
#define PARSE_ERROR 2   /* stop parsing, print an error message */

/* Add this alias to the list for the parser */
EXTERN void defrAddAlias PROTO_PARAMS((const char* key, const char* value,
                                       int marked));

#endif
