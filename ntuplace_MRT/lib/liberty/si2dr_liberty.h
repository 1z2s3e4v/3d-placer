/******************************************************************************
    Copyright (c) 1996-2000 Synopsys, Inc.    ALL RIGHTS RESERVED

  The contents of this file are subject to the restrictions and limitations
  set forth in the SYNOPSYS Open Source License Version 1.0  (the "License"); 
  you may not use this file except in compliance with such restrictions 
  and limitations. You may obtain instructions on how to receive a copy of 
  the License at

  http://www.synopsys.com/partners/tapin/tapinprogram.html. 

  Software distributed by Original Contributor under the License is 
  distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either 
  expressed or implied. See the License for the specific language governing 
  rights and limitations under the License.

******************************************************************************/

/* define a PI for manipulating the liberty data */


/* basic data types */


#ifndef SI2DR_H
#define SI2DR_H

#ifdef SI2_ARGS
#undef SI2_ARGS
#endif
#if defined(__STDC__) || defined(HPPA) || defined(ibmpoweraix) || defined(_MSC_VER) || defined(__cplusplus)
#define SI2_ARGS(args)	args
#else
#define SI2_ARGS(args)	()
#endif


/* ***********************************************************************
 File contents : LIBERTY-DR type definitions.
*********************************************************************** */
#ifdef __cplusplus
extern "C" {
#endif

/*
 * These primitive types are compatible with the ANSI C Standard (ANSI/
 * X3.159-1990 Programming Language C Standard.) 
 */
typedef enum si2BooleanEnum { SI2_FALSE = 0, SI2_TRUE = 1 } si2BooleanT;
typedef long int  si2LongT;
typedef float     si2FloatT;
typedef double    si2DoubleT;
typedef char*     si2StringT;
typedef void      si2VoidT;

typedef enum si2TypeEnum {
   SI2_UNDEFINED_VALUETYPE = 0,
   SI2_BOOLEAN = 1,
   SI2_LONG = 2,
   SI2_FLOAT = 3,
   SI2_DOUBLE = 4,
   SI2_STRING = 5,
   SI2_VOID = 6,
   SI2_OID = 7,
   SI2_ITER = 8,
   SI2_MAX_VALUETYPE = 9
   } si2TypeT;

/*
 * Size limits
 */

#define SI2_LONG_MAX   2147483647
#define SI2_LONG_MIN  -2147483647
#define SI2_ULONG_MAX  4294967295


typedef struct si2OID  {void *v1, *v2;} si2ObjectIdT;
/*
 * Iterators are blind handles the size of a single pointer.
 */
typedef void *si2IteratorIdT;


#define SI2DR_MIN_FLOAT32  -1E+37
#define SI2DR_MAX_FLOAT32   1E+37
#define SI2DR_MIN_FLOAT64  -1.797693e+308
#define SI2DR_MAX_FLOAT64   1.797693e+308
#define SI2DR_MIN_INT32    -2147483647
#define SI2DR_MAX_INT32     2147483647

#define SI2DR_TRUE SI2_TRUE
#define SI2DR_FALSE SI2_FALSE

typedef si2BooleanT si2drBooleanT;

typedef si2LongT    si2drInt32T;
typedef si2FloatT   si2drFloat32T;
typedef si2DoubleT   si2drFloat64T;
typedef si2StringT  si2drStringT;
typedef si2VoidT    si2drVoidT;

typedef si2IteratorIdT	si2drIterIdT;
typedef si2ObjectIdT	si2drObjectIdT;

typedef si2drObjectIdT si2drGroupIdT;
typedef si2drObjectIdT si2drAttrIdT;
typedef si2drObjectIdT si2drDefineIdT;

typedef si2drIterIdT si2drObjectsIdT;
typedef si2drIterIdT si2drGroupsIdT;
typedef si2drIterIdT si2drAttrsIdT;
typedef si2drIterIdT si2drDefinesIdT;
typedef si2drIterIdT si2drNamesIdT;
typedef si2drIterIdT si2drValuesIdT;

typedef enum si2drSeverityT {
	SI2DR_SEVERITY_NOTE = 0,
	SI2DR_SEVERITY_WARN = 1,
	SI2DR_SEVERITY_ERR  = 2
} si2drSeverityT;


typedef enum si2drObjectTypeT {
    SI2DR_UNDEFINED_OBJECTTYPE =  0,
    SI2DR_GROUP                =  1,
    SI2DR_ATTR                 =  2,
    SI2DR_DEFINE               =  3,
	SI2DR_MAX_OBJECTTYPE       =  4
} si2drObjectTypeT;

typedef enum si2drAttrTypeT
{
	SI2DR_SIMPLE,
	SI2DR_COMPLEX
} si2drAttrTypeT;


typedef enum si2drValueTypeT {
    SI2DR_UNDEFINED_VALUETYPE = SI2_UNDEFINED_VALUETYPE,
    SI2DR_INT32               = SI2_LONG,
    SI2DR_STRING              = SI2_STRING,
    SI2DR_FLOAT64             = SI2_DOUBLE,
    SI2DR_BOOLEAN             = SI2_BOOLEAN,
    SI2DR_MAX_VALUETYPE       = SI2_MAX_VALUETYPE
} si2drValueTypeT;

typedef enum si2drErrorT {
	SI2DR_NO_ERROR                    	      = 0,
	SI2DR_INTERNAL_SYSTEM_ERROR               = 1,
	SI2DR_INVALID_VALUE                       = 4,
	SI2DR_INVALID_NAME                        = 5,
	SI2DR_INVALID_OBJECTTYPE                  = 6,
	SI2DR_INVALID_ATTRTYPE                    = 10,
	SI2DR_UNUSABLE_OID                        = 7,
	SI2DR_OBJECT_ALREADY_EXISTS               = 8,
	SI2DR_OBJECT_NOT_FOUND                    = 9,
	SI2DR_SYNTAX_ERROR                        = 2,
	SI2DR_TRACE_FILES_CANNOT_BE_OPENED        = 3,
	SI2DR_PIINIT_NOT_CALLED                   = 11,
	SI2DR_SEMANTIC_ERROR                      = 12,
    SI2DR_REFERENCE_ERROR                     = 13,
	SI2DR_MAX_ERROR                           = 14
} si2drErrorT;

si2drVoidT  si2drDefaultMessageHandler(si2drSeverityT sev, 
									   si2drErrorT errToPrint,
									   si2drStringT auxText,
									   si2drErrorT *err);


	typedef si2drVoidT (*si2drMessageHandlerT)(si2drSeverityT sev, 
											   si2drErrorT errToPrint,
											   si2drStringT auxText,
											   si2drErrorT *err);
	
	
si2drMessageHandlerT  si2drPIGetMessageHandler( si2drErrorT *err);


si2drVoidT  si2drPISetMessageHandler( si2drMessageHandlerT MsgHandFuncPtr,
									  si2drErrorT *err);

	
	si2drGroupIdT  si2drPICreateGroup     SI2_ARGS(( si2drStringT name,
													 si2drStringT group_type,
													 si2drErrorT  *err));
	

	si2drAttrIdT   si2drGroupCreateAttr   SI2_ARGS(( si2drGroupIdT  group,
													 si2drStringT   name,
													 si2drAttrTypeT type,
													 si2drErrorT   *err));

	si2drAttrTypeT si2drAttrGetAttrType   SI2_ARGS(( si2drAttrIdT  attr,
													 si2drErrorT   *err));
	
	si2drStringT   si2drAttrGetName       SI2_ARGS(( si2drAttrIdT  attr,
													 si2drErrorT   *err));
	
	si2drVoidT     si2drComplexAttrAddInt32Value        SI2_ARGS(( si2drAttrIdT attr,
																   si2drInt32T     intgr,
																   si2drErrorT     *err ));
	
	si2drVoidT     si2drComplexAttrAddStringValue       SI2_ARGS(( si2drAttrIdT attr,
																	si2drStringT    string,
																	si2drErrorT     *err ));

	si2drVoidT     si2drComplexAttrAddBooleanValue      SI2_ARGS(( si2drAttrIdT attr,
																	 si2drBooleanT   boolval,
																	 si2drErrorT     *err ));
	
	si2drVoidT     si2drComplexAttrAddFloat64Value      SI2_ARGS(( si2drAttrIdT attr,
																	 si2drFloat64T   float64,
																	 si2drErrorT     *err ));

	si2drValuesIdT si2drComplexAttrGetValues         SI2_ARGS(( si2drAttrIdT     attr,
																si2drErrorT     *err ));

	si2drVoidT     si2drIterNextComplexValue         SI2_ARGS(( si2drValuesIdT iter,
																si2drValueTypeT *type,
																si2drInt32T     *intgr,
																si2drFloat64T   *float64,
																si2drStringT    *string,
																si2drBooleanT   *boolval,
																si2drErrorT     *err ));

	si2drValueTypeT  si2drSimpleAttrGetValueType        SI2_ARGS(( si2drAttrIdT attr,
																   si2drErrorT     *err ));

	si2drInt32T      si2drSimpleAttrGetInt32Value       SI2_ARGS(( si2drAttrIdT attr,
																   si2drErrorT     *err ));
	
	si2drFloat64T    si2drSimpleAttrGetFloat64Value     SI2_ARGS(( si2drAttrIdT attr,
																   si2drErrorT     *err ));
	
	si2drStringT     si2drSimpleAttrGetStringValue      SI2_ARGS(( si2drAttrIdT attr,
																   si2drErrorT     *err ));

	si2drBooleanT    si2drSimpleAttrGetBooleanValue     SI2_ARGS(( si2drAttrIdT attr,
																   si2drErrorT     *err ));


	
	si2drVoidT       si2drSimpleAttrSetInt32Value       SI2_ARGS(( si2drAttrIdT attr,
																   si2drInt32T     intgr,
																   si2drErrorT     *err ));
	
	si2drVoidT       si2drSimpleAttrSetBooleanValue     SI2_ARGS(( si2drAttrIdT   attr,
																   si2drBooleanT   intgr,
																   si2drErrorT      *err ));
	
	si2drVoidT       si2drSimpleAttrSetFloat64Value     SI2_ARGS(( si2drAttrIdT attr,
																   si2drFloat64T   float64,
																   si2drErrorT     *err ));
	
	si2drVoidT       si2drSimpleAttrSetStringValue      SI2_ARGS(( si2drAttrIdT attr,
																   si2drStringT    string,
																   si2drErrorT     *err ));
	


	si2drDefineIdT si2drGroupCreateDefine SI2_ARGS(( si2drGroupIdT group,
													 si2drStringT name,
													 si2drStringT allowed_group_name,
													 si2drValueTypeT valtype,
													 si2drErrorT  *err));
	
	si2drVoidT     si2drDefineGetInfo     SI2_ARGS(( si2drDefineIdT def,
													 si2drStringT  *name,
													 si2drStringT  *allowed_group_name,
													 si2drValueTypeT *valtype,
													 si2drErrorT  *err));
	
	si2drGroupIdT  si2drGroupCreateGroup  SI2_ARGS(( si2drGroupIdT group,
													 si2drStringT name,
													 si2drStringT group_type,
													 si2drErrorT  *err));

	si2drStringT   si2drGroupGetGroupType SI2_ARGS(( si2drGroupIdT group,
													 si2drErrorT  *err));
	
	si2drVoidT     si2drGroupAddName      SI2_ARGS(( si2drGroupIdT group,
													 si2drStringT name,
													 si2drErrorT  *err));
	
	si2drVoidT     si2drGroupDeleteName   SI2_ARGS(( si2drGroupIdT group,
													 si2drStringT name,
													 si2drErrorT  *err));
	
	
	si2drGroupIdT  si2drPIFindGroupByName     SI2_ARGS(( si2drStringT name,
														 si2drStringT type,
														 si2drErrorT  *err));
	
	si2drGroupIdT  si2drGroupFindGroupByName  SI2_ARGS(( si2drGroupIdT group,
														 si2drStringT name,
														 si2drStringT type,
														 si2drErrorT  *err));
	
	si2drAttrIdT   si2drGroupFindAttrByName   SI2_ARGS(( si2drGroupIdT group,
														 si2drStringT name,
														 si2drErrorT  *err));
	
	si2drDefineIdT si2drGroupFindDefineByName SI2_ARGS(( si2drGroupIdT group,
														 si2drStringT name,
														 si2drErrorT  *err));
	

	si2drDefineIdT si2drPIFindDefineByName SI2_ARGS(( si2drStringT name,
													  si2drErrorT  *err));
	


	si2drGroupsIdT  si2drPIGetGroups     SI2_ARGS(( si2drErrorT  *err));
	

	si2drGroupsIdT  si2drGroupGetGroups  SI2_ARGS(( si2drGroupIdT group,
													si2drErrorT  *err));

	si2drNamesIdT   si2drGroupGetNames   SI2_ARGS(( si2drGroupIdT group,
													si2drErrorT  *err));

	si2drAttrsIdT   si2drGroupGetAttrs   SI2_ARGS(( si2drGroupIdT group,
													si2drErrorT  *err));

	si2drDefinesIdT si2drGroupGetDefines SI2_ARGS(( si2drGroupIdT group,
													si2drErrorT  *err));

	si2drGroupIdT   si2drIterNextGroup  SI2_ARGS(( si2drGroupsIdT iter,
												   si2drErrorT  *err));
	si2drStringT    si2drIterNextName   SI2_ARGS(( si2drNamesIdT iter,
												   si2drErrorT  *err));
	si2drAttrIdT    si2drIterNextAttr   SI2_ARGS(( si2drAttrsIdT iter,
												   si2drErrorT  *err));
	si2drDefineIdT  si2drIterNextDefine SI2_ARGS(( si2drDefinesIdT iter,
												   si2drErrorT  *err));
	si2drVoidT      si2drIterQuit       SI2_ARGS(( si2drIterIdT iter,
												   si2drErrorT  *err));
	


	si2drVoidT      si2drObjectDelete   SI2_ARGS(( si2drObjectIdT object,
												   si2drErrorT  *err));

	

	si2drStringT     si2drPIGetErrorText          SI2_ARGS(( si2drErrorT errorCode,
															 si2drErrorT  *err));
	
	si2drObjectIdT   si2drPIGetNullId             SI2_ARGS(( si2drErrorT  *err));

	si2drVoidT       si2drPIInit                  SI2_ARGS(( si2drErrorT  *err));

	si2drVoidT       si2drPIQuit                  SI2_ARGS(( si2drErrorT  *err));

	si2drObjectTypeT si2drObjectGetObjectType     SI2_ARGS(( si2drObjectIdT object,
															 si2drErrorT  *err));

	si2drObjectIdT   si2drObjectGetOwner          SI2_ARGS(( si2drObjectIdT object,
															 si2drErrorT  *err));

	si2drBooleanT    si2drObjectIsNull            SI2_ARGS(( si2drObjectIdT object,
															 si2drErrorT  *err));
	
	si2drBooleanT    si2drObjectIsSame            SI2_ARGS(( si2drObjectIdT object1,
															 si2drObjectIdT object2,
															 si2drErrorT  *err));
	
	si2drBooleanT    si2drObjectIsUsable          SI2_ARGS(( si2drObjectIdT object,
															 si2drErrorT  *err));

	si2drVoidT       si2drObjectSetFileName       SI2_ARGS(( si2drObjectIdT object,
															 si2drStringT   filename,
															 si2drErrorT  *err));

	si2drVoidT       si2drObjectSetLineNo         SI2_ARGS(( si2drObjectIdT object,
															 si2drInt32T    lineno,
															 si2drErrorT  *err));

	si2drInt32T      si2drObjectGetLineNo         SI2_ARGS(( si2drObjectIdT object,
															 si2drErrorT  *err));

	si2drStringT     si2drObjectGetFileName       SI2_ARGS(( si2drObjectIdT object,
															 si2drErrorT  *err));

	si2drVoidT       si2drReadLibertyFile         SI2_ARGS(( char *filename,
															 si2drErrorT  *err));

	si2drVoidT       si2drWriteLibertyFile        SI2_ARGS(( char *filename,
															 si2drGroupIdT group,
															 si2drErrorT  *err));
	
    si2drVoidT       si2drCheckLibertyLibrary     SI2_ARGS(( si2drGroupIdT group,
															 si2drErrorT  *err));
	

	si2drBooleanT  si2drPIGetTraceMode       SI2_ARGS((si2drErrorT  *err));
	
	si2drVoidT     si2drPIUnSetTraceMode     SI2_ARGS((si2drErrorT  *err));
	
	si2drVoidT     si2drPISetTraceMode       SI2_ARGS((si2drStringT fname,
													   si2drErrorT  *err));
	

	si2drVoidT       si2drPISetDebugMode       SI2_ARGS((si2drErrorT  *err));
	
	si2drVoidT       si2drPIUnSetDebugMode     SI2_ARGS((si2drErrorT  *err));
	
	si2drBooleanT    si2drPIGetDebugMode       SI2_ARGS((si2drErrorT  *err));
																										  
	
	si2drVoidT     si2drPISetNocheckMode     SI2_ARGS((si2drErrorT  *err));
	
	si2drVoidT     si2drPIUnSetNocheckMode   SI2_ARGS((si2drErrorT  *err));
	
	si2drBooleanT  si2drPIGetNocheckMode     SI2_ARGS((si2drErrorT  *err));
		 

	si2drVoidT     si2drGroupMoveAfter SI2_ARGS((si2drGroupIdT groupToMove, 
												 si2drGroupIdT targetGroup,
												 si2drErrorT   *err));
	
	si2drVoidT     si2drGroupMoveBefore SI2_ARGS((si2drGroupIdT groupToMove, 
												  si2drGroupIdT targetGroup, 
												  si2drErrorT *err));

	
#ifdef __cplusplus
}
#endif

#endif
	
