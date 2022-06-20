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

#ifndef defiSLOT_h
#define defiSLOT_h

#include <stdio.h>
#include "defiKRDefs.hpp"

class defiSlot {
public:
  defiSlot();
  void Init();

  void Destroy();
  ~defiSlot();

  void clear();

  void setLayer(const char* name);
  void addRect(int xl, int yl, int xh, int yh);

  int hasLayer() const;
  const char* layerName() const;

  int numRectangles() const;
  int xl(int index) const;
  int yl(int index) const;
  int xh(int index) const;
  int yh(int index) const;

  void print(FILE* f) const;

protected:
  int   hasLayer_;
  char* layerName_;
  int   layerNameLength_;
  int   numRectangles_;
  int   rectsAllocated_;
  int*  xl_;
  int*  yl_;
  int*  xh_;
  int*  yh_;
};


#endif
