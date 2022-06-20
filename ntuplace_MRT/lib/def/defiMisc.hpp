/*
 *     This  file  is  part  of  the  Cadence  LEF/DEF  Open   Source
 *  Distribution,  Product Version 5.7, and is subject to the Cadence
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

#ifndef defiMisc_h
#define defiMisc_h

#include <stdio.h>
#include "defiKRDefs.hpp"

struct defiPoints {
  int numPoints;
  int* x;
  int* y;
};

class defiGeometries {
public:
  defiGeometries();
  void Init();
  void Reset();

  void Destroy();
  ~defiGeometries();

  void startList(int x, int y);
  void addToList(int x, int y);
  int  numPoints();
  void points(int index, int* x, int* y);

protected:
  int numPoints_;
  int pointsAllocated_;
  int* x_;
  int* y_;
};

class defiStyles {
public:
  defiStyles();
  void Init();

  void Destroy();
  ~defiStyles();

  void clear();

  void setStyle(int styleNum);
  void setPolygon(defiGeometries* geom);

  int style() const;
  struct defiPoints getPolygon() const;

  protected:
    int    styleNum_;
    struct defiPoints* polygon_;
    int    numPointAlloc_;
};

#endif
