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

#ifndef defiBLOCKAGES_h
#define defiBLOCKAGES_h

#include <stdio.h>
#include "defiKRDefs.hpp"

class defiBlockage {
public:
  defiBlockage();
  void Init();

  void Destroy();
  ~defiBlockage();

  void clear();

  void setLayer(const char* name);
  void setPlacement();
  void setComponent(const char* name);
  void setSlots();
  void setFills();
  void setPushdown();
  void addRect(int xl, int yl, int xh, int yh);

  int hasLayer() const;
  int hasPlacement() const;
  int hasComponent() const;
  int hasSlots() const;
  int hasFills() const;
  int hasPushdown() const;
  const char* layerName() const;
  const char* layerComponentName() const;
  const char* placementComponentName() const;

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
  char* placementName_;
  int   hasPlacement_;
  int   hasComponent_;
  char* componentName_;
  int   componentNameLength_;
  int   hasSlots_;
  int   hasFills_;
  int   hasPushdown_;
  int   numRectangles_;
  int   rectsAllocated_;
  int*  xl_;
  int*  yl_;
  int*  xh_;
  int*  yh_;
};


#endif
