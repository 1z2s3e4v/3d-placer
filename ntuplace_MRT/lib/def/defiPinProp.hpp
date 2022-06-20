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

#ifndef defiPinProp_h
#define defiPinProp_h

#include "defiKRDefs.hpp"
#include <stdio.h>


class defiPinProp {
public:
  defiPinProp();
  void Init();

  ~defiPinProp();
  void Destroy();

  void clear();

  void setName(const char* inst, const char* pin);
  void addProperty(const char* name, const char* value, const char type);
  void addNumProperty(const char* name, const double d,
                      const char* value, const char type);

  int isPin() const;
  const char* instName() const;
  const char* pinName() const;

  int numProps() const;
  const char*  propName(int index) const;
  const char*  propValue(int index) const;
  double propNumber(int index) const;
  const char   propType(int index) const;
  int    propIsNumber(int index) const;
  int    propIsString(int index) const;

  void print(FILE* f) const;

protected:
  char isPin_;
  int instNameSize_;
  char* instName_;
  int pinNameSize_;
  char* pinName_;

  int numProps_;
  int propsAllocated_;
  char**  propNames_;
  char**  propValues_;
  double* propDValues_;
  char*   propTypes_;
};


#endif
