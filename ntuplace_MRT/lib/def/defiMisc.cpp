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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "lex.h"
#include "defiDebug.hpp"
#include "defiMisc.hpp"


////////////////////////////////////////////////////
//
//    Handle points for a polygon
//
////////////////////////////////////////////////////
defiGeometries::defiGeometries() {
  this->defiGeometries::Init();
}

void defiGeometries::Init() {
  this->pointsAllocated_ = 0;
  this->numPoints_ = 0;
}

void defiGeometries::Destroy() {
  if (this->pointsAllocated_) {
    defFree((char*)(this->x_));
    defFree((char*)(this->y_));
  }
  this->pointsAllocated_ = 0;
  this->numPoints_ = 0;
}

defiGeometries::~defiGeometries() {
  this->defiGeometries::Destroy();
}

void defiGeometries::Reset() {
  this->numPoints_ = 0;
}

void defiGeometries::startList(int x, int y) {
  if (this->pointsAllocated_ == 0) {
    this->pointsAllocated_ = 16;
    this->x_ = (int*)defMalloc(sizeof(int)*16);
    this->y_ = (int*)defMalloc(sizeof(int)*16);
    this->numPoints_ = 0;
  } else   // reset the numPoints to 0
    this->numPoints_ = 0;
  this->defiGeometries::addToList(x, y);
}

void defiGeometries::addToList(int x, int y) {
  if (this->numPoints_ == this->pointsAllocated_) {
    int i;
    int* nx;
    int* ny;
    this->pointsAllocated_ *= 2;
    nx = (int*)defMalloc(sizeof(int)*this->pointsAllocated_);
    ny = (int*)defMalloc(sizeof(int)*this->pointsAllocated_);
    for (i = 0; i < this->numPoints_; i++) {
      nx[i] = this->x_[i];
      ny[i] = this->y_[i];
    }
    defFree((char*)(this->x_));
    defFree((char*)(this->y_));
    this->x_ = nx;
    this->y_ = ny;
  }
  this->x_[this->numPoints_] = x;
  this->y_[this->numPoints_] = y;
  this->numPoints_ += 1;
}

int defiGeometries::numPoints() {
  return this->numPoints_;
}

void defiGeometries::points(int index, int* x, int* y) {
  char msg[160];
  if ((index < 0) || (index >= this->numPoints_)) {
     sprintf (msg, "ERROR (LEFPARS-6070): The index number %d given for GEOMETRY POINTS is invalid.\nValid index is from 0 to %d", index, this->numPoints_);
     defiError (msg);
     return;
  }
  *x = this->x_[index];
  *y = this->y_[index];
  return;
}

////////////////////////////////////////////////////
//
//    defiStyles
//
////////////////////////////////////////////////////

defiStyles::defiStyles() {
  this->defiStyles::Init();
}

void defiStyles::Init() {
  this->styleNum_ = 0;
  this->polygon_ = 0;
}

defiStyles::~defiStyles() {
  this->defiStyles::Destroy();
}

void defiStyles::Destroy() {
 this->defiStyles::clear();
}

void defiStyles::clear() {
  struct defiPoints* p;

  p = this->polygon_;
  if (p) {
    defFree((char*)(p->x));
    defFree((char*)(p->y));
    defFree((char*)(this->polygon_));
  }
  this->styleNum_ = 0;
  this->polygon_ = 0;
}

void defiStyles::setStyle(int styleNum) {
  this->styleNum_ = styleNum;
}

void defiStyles::setPolygon(defiGeometries* geom) {
  struct defiPoints* p;
  int i, x, y;

  if (this->polygon_ == 0) {
    p = (struct defiPoints*)defMalloc(sizeof(struct defiPoints));
    p->numPoints = geom->defiGeometries::numPoints();
    p->x = (int*)defMalloc(sizeof(int)*p->numPoints);
    p->y = (int*)defMalloc(sizeof(int)*p->numPoints);
    this->numPointAlloc_ = p->numPoints; // keep track the max number pts
  } else if (this->numPointAlloc_ < geom->defiGeometries::numPoints()) {
    // the incoming polygon has more number then has been allocated,
    // need to reallocate more memory
    p = this->polygon_;
    defFree((char*)(p->x));
    defFree((char*)(p->y));
    p->numPoints = geom->defiGeometries::numPoints();
    p->x = (int*)defMalloc(sizeof(int)*p->numPoints);
    p->y = (int*)defMalloc(sizeof(int)*p->numPoints);
    this->numPointAlloc_ = p->numPoints; // keep track the max number pts
  } else {
    p = this->polygon_;
    p->numPoints = geom->defiGeometries::numPoints();
  }
  for (i = 0; i < p->numPoints; i++) {
    geom->defiGeometries::points(i, &x, &y);
    p->x[i] = x;
    p->y[i] = y;
  }
  this->polygon_ = p;
}

int defiStyles::style() const {
  return this->styleNum_;
}

struct defiPoints defiStyles::getPolygon() const {
  return *(this->polygon_);
}
