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

#ifndef DEFI_WRITER_H
#define DEFI_WRITER_H

#include <stdarg.h>
#include <stdio.h>
 
#include "defiKRDefs.hpp"
#include "defiDefs.hpp"
#include "defiUser.hpp"

/*
 * The main writer function.
 * The file should already be opened.  This requirement allows
 * the writer to be used with stdin or a pipe.  The file name
 * is only used for error messages.  The includeSearchPath is
 * a colon-delimited list of directories in which to find
 * include files.
 */
EXTERN int defwWrite
  PROTO_PARAMS(( FILE *file,
                 const char *fileName,
                 defiUserData userData ));

/*
 * Set all of the callbacks that have not yet been set to a function
 * that will add up how many times a given def data type was ignored
 * (ie no callback was done).  The statistics can later be printed out.
 */
EXTERN void defwSetRegisterUnusedCallbacks PROTO_PARAMS((void));
EXTERN void defwPrintUnusedCallbacks PROTO_PARAMS((FILE* log));
EXTERN int* defwUnusedCallbackCount PROTO_PARAMS((void));

/*
 * Set/get the client-provided user data.  defi doesn't look at
 * this data at all, it simply passes the opaque defiUserData pointer
 * back to the application with each callback.  The client can
 * change the data at any time, and it will take effect on the
 * next callback.  The defi writer and writer maintain separate
 * user data pointers.
 */
EXTERN void defwSetUserData PROTO_PARAMS(( defiUserData ));
EXTERN defiUserData defwGetUserData PROTO_PARAMS(( void ));
 
/*
 * An enum describing all of the types of writer callbacks.
 */
typedef enum {
  defwUnspecifiedCbkType = 0,
  defwVersionCbkType,
  defwCaseSensitiveCbkType,
  defwBusBitCbkType,
  defwDividerCbkType,
  defwDesignCbkType,
  defwTechCbkType,
  defwArrayCbkType,
  defwFloorPlanCbkType,
  defwUnitsCbkType,
  defwHistoryCbkType,
  defwPropDefCbkType,
  defwDieAreaCbkType,
  defwRowCbkType,
  defwTrackCbkType,
  defwGcellGridCbkType,
  defwDefaultCapCbkType,
  defwCanplaceCbkType,
  defwCannotOccupyCbkType,
  defwViaCbkType,
  defwRegionCbkType,
  defwComponentCbkType,
  defwPinCbkType,
  defwPinPropCbkType,
  defwSNetCbkType,
  defwNetCbkType,
  defwIOTimingCbkType,
  defwScanchainCbkType,
  defwConstraintCbkType,
  defwAssertionCbkType,    // pre 5.2
  defwGroupCbkType,
  defwBlockageCbkType,     // 5.4
  defwExtCbkType,
  defwDesignEndCbkType

  /* NEW CALLBACKS - each callback has its own type.  For each callback
   * that you add, you must add an item to this enum. */

} defwCallbackType_e;
 
 
/* Declarations of function signatures for each type of callback.
 * These declarations are type-safe when compiling with ANSI C
 * or C++; you will only be able to register a function pointer
 * with the correct signature for a given type of callback.
 *
 * Each callback function is expected to return 0 if successful.
 * A non-zero return code will cause the writer to abort.
 *
 * The defwDesignStart and defwDesignEnd callback is only called once.
 * Other callbacks may be called multiple times, each time with a different
 * set of data.
 *
 * For each callback, the Def API will make the callback to the
 * function supplied by the client, which should either make a copy
 * of the Def object, or store the data in the client's own data structures.
 * The Def API will delete or reuse each object after making the callback,
 * so the client should not keep a pointer to it.
 *
 * All callbacks pass the user data pointer provided in defwRead()
 * or defwSetUserData() back to the client; this can be used by the
 * client to obtain access to the rest of the client's data structures.
 *
 * The user data pointer is obtained using defwGetUserData() immediately
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
typedef int (*defwVoidCbkFnType)
    PROTO_PARAMS(( defwCallbackType_e,
                   defiUserData ));

/* Functions to call to register a callback function.
 */
EXTERN void defwSetArrayCbk (defwVoidCbkFnType);
EXTERN void defwSetAssertionCbk (defwVoidCbkFnType);
EXTERN void defwSetBlockageCbk (defwVoidCbkFnType);
EXTERN void defwSetBusBitCbk (defwVoidCbkFnType);
EXTERN void defwSetCannotOccupyCbk (defwVoidCbkFnType);
EXTERN void defwSetCanplaceCbk (defwVoidCbkFnType);
EXTERN void defwSetCaseSensitiveCbk (defwVoidCbkFnType);
EXTERN void defwSetComponentCbk (defwVoidCbkFnType);
EXTERN void defwSetConstraintCbk (defwVoidCbkFnType);
EXTERN void defwSetDefaultCapCbk (defwVoidCbkFnType);
EXTERN void defwSetDesignCbk (defwVoidCbkFnType);
EXTERN void defwSetDesignEndCbk (defwVoidCbkFnType);
EXTERN void defwSetDieAreaCbk (defwVoidCbkFnType);
EXTERN void defwSetDividerCbk (defwVoidCbkFnType);
EXTERN void defwSetExtCbk (defwVoidCbkFnType);
EXTERN void defwSetFloorPlanCbk (defwVoidCbkFnType);
EXTERN void defwSetGcellGridCbk (defwVoidCbkFnType);
EXTERN void defwSetGroupCbk (defwVoidCbkFnType);
EXTERN void defwSetHistoryCbk (defwVoidCbkFnType);
EXTERN void defwSetIOTimingCbk (defwVoidCbkFnType);
EXTERN void defwSetNetCbk (defwVoidCbkFnType);
EXTERN void defwSetPinCbk (defwVoidCbkFnType);
EXTERN void defwSetPinPropCbk (defwVoidCbkFnType);
EXTERN void defwSetPropDefCbk (defwVoidCbkFnType);
EXTERN void defwSetRegionCbk (defwVoidCbkFnType);
EXTERN void defwSetRowCbk (defwVoidCbkFnType);
EXTERN void defwSetSNetCbk (defwVoidCbkFnType);
EXTERN void defwSetScanchainCbk (defwVoidCbkFnType);
EXTERN void defwSetTechnologyCbk (defwVoidCbkFnType);
EXTERN void defwSetTrackCbk (defwVoidCbkFnType);
EXTERN void defwSetUnitsCbk (defwVoidCbkFnType);
EXTERN void defwSetVersionCbk (defwVoidCbkFnType);
EXTERN void defwSetViaCbk (defwVoidCbkFnType);

/* NEW CALLBACK - each callback must have a function to allow the user
 * to set it.  Add the function here. */


/*
 * Set all of the callbacks that have not yet been set to the following
 * function.  This is especially useful if you want to check to see
 * if you forgot anything.
 */
EXTERN void defwSetUnusedCallbacks PROTO_PARAMS((defwVoidCbkFnType func));

/* Routine to set the message logging routine for errors */
typedef void (*DEFI_LOG_FUNCTION)(const char*);
void defwSetLogFunction( DEFI_LOG_FUNCTION );

/* Routine to set the message logging routine for warnings */
typedef void (*DEFI_WARNING_LOG_FUNCTION)(const char*);
void defwSetWarningLogFunction( DEFI_WARNING_LOG_FUNCTION );

#endif
