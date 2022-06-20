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

/*
 * User header file for the DEF Interface.  This includes
 * all of the header files which are relevant to both the
 * reader and the writer.
 *
 * defrReader.h and defwWriter.h include this file, so that
 * an application only needs to include either defwReader.h
 * or defwWriter.h.
 */

#ifndef DEFI_USER_H
#define DEFI_USER_H

/* General utilities. */
/* #include "defiMalloc.hpp" */
/* #include "defiUtils.hpp" */

/*
 * API objects
 */
#include "defiDebug.hpp"
#include "defiProp.hpp"
#include "defiSite.hpp"
#include "defiComponent.hpp"
#include "defiNet.hpp"
#include "defiPath.hpp"
#include "defiPinCap.hpp"
#include "defiRowTrack.hpp"
#include "defiVia.hpp"
#include "defiRegion.hpp"
#include "defiGroup.hpp"
#include "defiAssertion.hpp"
#include "defiScanchain.hpp"
#include "defiIOTiming.hpp"
#include "defiFPC.hpp"
#include "defiTimingDisable.hpp"
#include "defiPartition.hpp"
#include "defiPinProp.hpp"
#include "defiBlockage.hpp"
#include "defiSlot.hpp"
#include "defiFill.hpp"
#include "defiPropType.hpp"

/* NEW CALLBACK - If you are creating a new .cpp and .hpp file to
 * describe a new class of object in the parser, then add a reference
 * to the .hpp here.
 *
 *  You must also add an entry for the .h and the .hpp in the package_list
 * file of the ../../../release directory. */

#endif
