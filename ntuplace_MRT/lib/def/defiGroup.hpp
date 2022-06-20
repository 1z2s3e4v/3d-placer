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

#ifndef defiGroup_h
#define defiGroup_h

#include <stdio.h>
#include "defiKRDefs.hpp"

// Struct holds the data for one property.
class defiGroup {
public:
  defiGroup();
  void Init();

  void Destroy();
  ~defiGroup();

  void clear();

  void setup(const char* name);
  void addProperty(const char* name, const char* value, const char type);
  void addNumProperty(const char* name, const double d,
                      const char* value, const char type);
  void addRegionRect(int xl, int yl, int xh, int yh);
  void setRegionName(const char* name);
  void setMaxX(int x);
  void setMaxY(int y);
  void setPerim(int p);

  const char* name() const;
  const char* regionName() const;
  int hasRegionBox() const;
  int hasRegionName() const;
  int hasMaxX() const;
  int hasMaxY() const;
  int hasPerim() const;
  void regionRects(int* size, int** xl, int**yl, int** xh, int** yh);
  int maxX() const;
  int maxY() const;
  int perim() const;

  int numProps() const;
  const char*  propName(int index) const;
  const char*  propValue(int index) const;
  double propNumber(int index) const;
  const char   propType(int index) const;
  int    propIsNumber(int index) const;
  int    propIsString(int index) const;

  // debug print
  void print(FILE* f) const;

protected:
  char* name_;
  int nameLength_;
  char* region_;
  int regionLength_;

  int rectsAllocated_;
  int numRects_;
  int* xl_;
  int* yl_;
  int* xh_;
  int* yh_;

  int maxX_;
  int maxY_;
  int perim_;
  char hasRegionBox_;
  char hasRegionName_;
  char hasPerim_;
  char hasMaxX_;
  char hasMaxY_;

  int numProps_;
  int propsAllocated_;
  char**  propNames_;
  char**  propValues_;
  double* propDValues_;
  char*   propTypes_;
};


#endif
