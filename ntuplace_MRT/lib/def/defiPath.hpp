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

#ifndef defiPath_h
#define defiPath_h

#include <stdio.h>
#include "defiKRDefs.hpp"

// TX_DIR:TRANSLATION ON

struct defiPnt {
  int x;
  int y;
  int ext;
};


// 5.4.1 1-D & 2-D Arrays of Vias in SPECIALNET Section
struct defiViaData {
  int numX;
  int numY;
  int stepX;
  int stepY;
};

// value returned by the next() routine.
enum defiPath_e {
  DEFIPATH_DONE = 0,
  DEFIPATH_LAYER,
  DEFIPATH_VIA,
  DEFIPATH_VIAROTATION,
  DEFIPATH_WIDTH,
  DEFIPATH_POINT,
  DEFIPATH_FLUSHPOINT,
  DEFIPATH_TAPER,
  DEFIPATH_SHAPE,
  DEFIPATH_TAPERRULE,
  DEFIPATH_VIADATA
  } ;


class defiPath {
public:
  defiPath();
  void Init();

  void Destroy();
  ~defiPath();

  void clear();
  void reverseOrder();

  // To traverse the path and get the parts.
  void initTraverse();   // Initialize the traverse.
  void initTraverseBackwards();   // Initialize the traverse in reverse.
  int next();            // Get the next element.
  int prev();            // Get the next element in reverse.
  const char* getLayer();// Get the layer.
  const char* getTaperRule();// Get the rule.
  const char* getVia();  // Get the via.
  const char* getShape();// Get the shape.
  int  getViaRotation();  // Get the via rotation.
  void getViaData(int* numX, int* numY, int* stepX, int* stepY);  // 5.4.1
  int  getWidth();        // Get the width.
  void getPoint(int* x, int* y);// Get the point.
  void getFlushPoint(int* x, int* y, int* ext);// Get the point.

  // These routines are called by the parser to fill the path.
  void addWidth(int w);
  void addPoint(int x, int y);
  void addFlushPoint(int x, int y, int ext);
  void addLayer(const char* layer);
  void addVia(const char* name);
  void addViaRotation(int orient);
  void addViaData(int numX, int numY, int stepX, int stepY);   // 5.4.1
  void setTaper();
  void addTaperRule(const char* rule);
  void addShape(const char* shape);

  // debug printing
  void print(FILE* fout);

  void bumpSize(int size);

protected:
  int* keys_;           // type of item in path
  void** data_;         // extra data
  int numUsed_;         // number of items used in array
  int numAllocated_;    // allocated size of keys and data
  int pointer_;         // traversal pointer
  int numX_;      
  int numY_;
  int stepX_;
  int stepY_;
};

#endif
