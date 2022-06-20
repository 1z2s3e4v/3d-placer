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

#ifndef defiTimingDisable_h
#define defiTimingDisable_h

#include <stdio.h>
#include "defiKRDefs.hpp"

// A Timing disable can be a from-to  or a thru or a macro.
//   A macro is either a fromto macro or a thru macro.

class defiTimingDisable {
public:
  defiTimingDisable();
  void Init();

  void Destroy();
  ~defiTimingDisable();

  void clear();

  void setFromTo(const char* fromInst, const char* fromPin,
		 const char* toInst, const char* toPin);
  void setThru(const char* fromInst, const char* fromPin);
  void setMacro(const char* name);
  void setMacroThru(const char* thru);
  void setMacroFromTo(const char* fromPin, const char* toPin);
  void setReentrantPathsFlag();

  int hasMacroThru() const;
  int hasMacroFromTo() const;
  int hasThru() const;
  int hasFromTo() const;
  int hasReentrantPathsFlag() const;

  const char* fromPin() const;
  const char* toPin() const;
  const char* fromInst() const;
  const char* toInst() const;
  const char* macroName() const;
  const char* thruPin() const;    // Also macro thru
  const char* thruInst() const;

  // debug print
  void print(FILE* f) const;

protected:
  char* fromInst_;  // also macro name and thru inst
  int fromInstLength_;
  char* toInst_;
  int toInstLength_;
  char* fromPin_;  // also macro thru and thru pin
  int fromPinLength_;
  char* toPin_;
  int toPinLength_;

  int hasFromTo_;
  int hasThru_;
  int hasMacro_;
  int hasReentrantPathsFlag_;
};


#endif
