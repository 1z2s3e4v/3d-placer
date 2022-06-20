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

#ifndef defiRegion_h
#define defiRegion_h

#include "defiKRDefs.hpp"
#include <stdio.h>

// Struct holds the data for one property.
class defiRegion {
public:
  defiRegion();
  void Init();

  void Destroy();
  ~defiRegion();

  void clear();
  void setup(const char* name);
  void addRect(int xl, int yl, int xh, int yh);
  void addProperty(const char* name, const char* value, const char type);
  void addNumProperty(const char* name, const double d,
                      const char* value, const char type);
  void setType(const char* type);         // 5.4.1

  const char* name() const;

  int numProps() const;
  const char*  propName(int index) const;
  const char*  propValue(int index) const;
  double propNumber(int index) const;
  const char   propType(int index) const;
  int propIsNumber(int index) const;
  int propIsString(int index) const;

  int hasType() const;                    // 5.4.1
  const char* type() const;               // 5.4.1

  int numRectangles() const;
  int xl(int index) const;
  int yl(int index) const;
  int xh(int index) const;
  int yh(int index) const;

  void print(FILE* f) const;

protected:
  char* name_;
  int nameLength_;

  int numRectangles_;
  int rectanglesAllocated_;
  int* xl_;
  int* yl_;
  int* xh_;
  int* yh_;

  int numProps_;
  int propsAllocated_;
  char**  propNames_;
  char**  propValues_;
  double* propDValues_;
  char*   propTypes_;

  char* type_;
};



#endif
