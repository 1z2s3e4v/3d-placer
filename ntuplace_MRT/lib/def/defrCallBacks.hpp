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

#ifndef DEFRCALLBACKS_H
#define DEFRCALLBACKS_H 1

#include "defiKRDefs.hpp"

#include "defrReader.hpp"



//
// All declarations that are public (for the casual user) are found in
// defrReader.hpp.
// These declarations are not seen by the general user.  Only the
// parser (def.y ... def.tab.c) should use them.
//


//
// These are the callback function pointers.
// They are set by calls defined in defrReader.hpp and defrReader.cpp
// The grammer calls these as it parses the file.
//
extern defrStringCbkFnType defrDesignCbk;
extern defrStringCbkFnType defrTechnologyCbk;
extern defrVoidCbkFnType defrDesignEndCbk;
extern defrPropCbkFnType defrPropCbk;
extern defrVoidCbkFnType defrPropDefEndCbk;
extern defrVoidCbkFnType defrPropDefStartCbk;
extern defrIntegerCbkFnType defrCaseSensitiveCbk;
extern defrStringCbkFnType defrArrayNameCbk;
extern defrStringCbkFnType defrFloorPlanNameCbk;
extern defrDoubleCbkFnType defrUnitsCbk;
extern defrStringCbkFnType defrDividerCbk;
extern defrStringCbkFnType defrBusBitCbk;
extern defrSiteCbkFnType defrSiteCbk;
extern defrIntegerCbkFnType defrComponentStartCbk;
extern defrComponentCbkFnType defrComponentCbk;
extern defrVoidCbkFnType defrComponentEndCbk;
extern defrIntegerCbkFnType defrNetStartCbk;
extern defrNetCbkFnType defrNetCbk;
extern defrVoidCbkFnType defrNetEndCbk;
extern defrPathCbkFnType defrPathCbk;
extern defrDoubleCbkFnType defrVersionCbk;
extern defrStringCbkFnType defrVersionStrCbk;
extern defrStringCbkFnType defrComponentExtCbk;
extern defrStringCbkFnType defrPinExtCbk;
extern defrStringCbkFnType defrViaExtCbk;
extern defrStringCbkFnType defrNetConnectionExtCbk;
extern defrStringCbkFnType defrNetExtCbk;
extern defrStringCbkFnType defrGroupExtCbk;
extern defrStringCbkFnType defrScanChainExtCbk;
extern defrStringCbkFnType defrIoTimingsExtCbk;
extern defrStringCbkFnType defrPartitionsExtCbk;
extern defrStringCbkFnType defrHistoryCbk;
extern defrSiteCbkFnType defrCanplaceCbk;
extern defrSiteCbkFnType defrCannotOccupyCbk;
extern defrBoxCbkFnType defrDieAreaCbk;
extern defrPinCapCbkFnType defrPinCapCbk;
extern defrPinCbkFnType defrPinCbk;
extern defrIntegerCbkFnType defrStartPinsCbk;
extern defrVoidCbkFnType defrPinEndCbk;
extern defrIntegerCbkFnType defrDefaultCapCbk;
extern defrRowCbkFnType defrRowCbk;
extern defrTrackCbkFnType defrTrackCbk;
extern defrGcellGridCbkFnType defrGcellGridCbk;
extern defrIntegerCbkFnType defrViaStartCbk;
extern defrViaCbkFnType defrViaCbk;
extern defrVoidCbkFnType defrViaEndCbk;
extern defrIntegerCbkFnType defrRegionStartCbk;
extern defrRegionCbkFnType defrRegionCbk;
extern defrVoidCbkFnType defrRegionEndCbk;
extern defrIntegerCbkFnType defrSNetStartCbk;
extern defrNetCbkFnType defrSNetCbk;
extern defrVoidCbkFnType defrSNetEndCbk;
extern defrIntegerCbkFnType defrGroupsStartCbk;
extern defrStringCbkFnType defrGroupNameCbk;
extern defrStringCbkFnType defrGroupMemberCbk;
extern defrGroupCbkFnType defrGroupCbk;
extern defrVoidCbkFnType defrGroupsEndCbk;
extern defrIntegerCbkFnType defrAssertionsStartCbk;
extern defrAssertionCbkFnType defrAssertionCbk;
extern defrVoidCbkFnType defrAssertionsEndCbk;
extern defrIntegerCbkFnType defrConstraintsStartCbk;
extern defrAssertionCbkFnType defrConstraintCbk;
extern defrVoidCbkFnType defrConstraintsEndCbk;
extern defrIntegerCbkFnType defrScanchainsStartCbk;
extern defrScanchainCbkFnType defrScanchainCbk;
extern defrVoidCbkFnType defrScanchainsEndCbk;
extern defrIntegerCbkFnType defrIOTimingsStartCbk;
extern defrIOTimingCbkFnType defrIOTimingCbk;
extern defrVoidCbkFnType defrIOTimingsEndCbk;
extern defrIntegerCbkFnType defrFPCStartCbk;
extern defrFPCCbkFnType defrFPCCbk;
extern defrVoidCbkFnType defrFPCEndCbk;
extern defrIntegerCbkFnType defrTimingDisablesStartCbk;
extern defrTimingDisableCbkFnType defrTimingDisableCbk;
extern defrVoidCbkFnType defrTimingDisablesEndCbk;
extern defrIntegerCbkFnType defrPartitionsStartCbk;
extern defrPartitionCbkFnType defrPartitionCbk;
extern defrVoidCbkFnType defrPartitionsEndCbk;
extern defrIntegerCbkFnType defrPinPropStartCbk;
extern defrPinPropCbkFnType defrPinPropCbk;
extern defrVoidCbkFnType defrPinPropEndCbk;
extern defrIntegerCbkFnType defrBlockageStartCbk;
extern defrBlockageCbkFnType defrBlockageCbk;
extern defrVoidCbkFnType defrBlockageEndCbk;
extern defrIntegerCbkFnType defrSlotStartCbk;
extern defrSlotCbkFnType defrSlotCbk;
extern defrVoidCbkFnType defrSlotEndCbk;
extern defrIntegerCbkFnType defrFillStartCbk;
extern defrFillCbkFnType defrFillCbk;
extern defrVoidCbkFnType defrFillEndCbk;
/* NEW_CALLBACK - Add the names of any new callback functions here
 * so that def.y can call the functions. */

//
// These are data structures that are filled while parsing the file.
//
extern defiUserData defrUserData;
extern defiProp defrProp;
extern defiSite defrSite;
extern defiSite defrCanplace;
extern defiSite defrCannotOccupy;
extern defiBox defrDieArea;
extern defiComponent defrComponent;
extern defiNet defrNet;
extern defiSubnet* defrSubnet;
extern defiPath* defrPath;
extern defiPinCap defrPinCap;
extern defiPin defrPin;
extern defiRow defrRow;
extern defiTrack defrTrack;
extern defiGcellGrid defrGcellGrid;
extern defiVia defrVia;
extern defiRegion defrRegion;
extern defiGroup defrGroup;
extern defiAssertion defrAssertion;
extern defiScanchain defrScanchain;
extern defiIOTiming defrIOTiming;
extern defiFPC defrFPC;
extern defiTimingDisable defrTimingDisable;
extern defiPartition defrPartition;
extern defiPinProp defrPinProp;
extern defiBlockage defrBlockage;
extern defiSlot defrSlot;
extern defiFill defrFill;
/* NEW CALLBACK - If you are creating one global copy of the class
 * for def.y to add to, then place it here. */

//
// Flags that control parsing
//
extern int defrAddPathToNet;
extern int defrNeedPathData;


//
// Variables for property definition types
//
extern defiPropType defrCompProp;
extern defiPropType defrCompPinProp;
extern defiPropType defrDesignProp;
extern defiPropType defrGroupProp;
extern defiPropType defrNetProp;
extern defiPropType defrPinDefProp;
extern defiPropType defrRegionProp;
extern defiPropType defrRowProp;
extern defiPropType defrSNetProp;


//
// Parser utility routines.
//
extern void pathIsDone(int shield);

#endif
