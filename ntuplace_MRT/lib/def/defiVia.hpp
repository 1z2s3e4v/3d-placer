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

#ifndef defiVia_h
#define defiVia_h

#include "defiKRDefs.hpp"
#include <stdio.h>

// Struct holds the data for one property.
class defiVia {
public:
  defiVia();
  void Init();

  void Destroy();
  ~defiVia();

  void setup(const char* name);
  void addPattern(const char* patt);
  void addLayer(const char* layer, int xl, int yl, int xh, int yh);

  const char* name() const;
  const char* pattern() const;
  int hasPattern() const;
  int numLayers() const;
  void layer(int index, char** layer, int* xl, int* yl, int* xh, int* yh) const;

  void print(FILE* f) const;

protected:
  char* name_;
  int nameLength_;
  char* pattern_;
  int patternLength_;
  char** layers_;
  int* xl_;
  int* yl_;
  int* xh_;
  int* yh_;
  int layersLength_;
  int numLayers_;
};


#endif
