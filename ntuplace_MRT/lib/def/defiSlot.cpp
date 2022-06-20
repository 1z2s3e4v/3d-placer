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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lex.h"
#include "defiSlot.hpp"
#include "defiDebug.hpp"

////////////////////////////////////////////////////
////////////////////////////////////////////////////
//
//    defiSlot
//
////////////////////////////////////////////////////
////////////////////////////////////////////////////

defiSlot::defiSlot() {
  this->defiSlot::Init();
}


void defiSlot::Init() {
  this->defiSlot::clear();
  this->layerNameLength_ = 0;
  this->xl_ = (int*)defMalloc(sizeof(int)*1);
  this->yl_ = (int*)defMalloc(sizeof(int)*1);
  this->xh_ = (int*)defMalloc(sizeof(int)*1);
  this->yh_ = (int*)defMalloc(sizeof(int)*1);
  this->rectsAllocated_ = 1;      // At least 1 rectangle will define
}


defiSlot::~defiSlot() {
  this->defiSlot::Destroy();
}


void defiSlot::clear() {
  this->hasLayer_ = 0;
  this->numRectangles_ = 0;
}


void defiSlot::Destroy() {
  if (this->layerName_) defFree(this->layerName_);
  defFree((char*)(this->xl_));
  defFree((char*)(this->yl_));
  defFree((char*)(this->xh_));
  defFree((char*)(this->yh_));
  this->rectsAllocated_ = 0;
  this->xl_ = 0;
  this->yl_ = 0;
  this->xh_ = 0;
  this->yh_ = 0;
  this->defiSlot::clear();
}


void defiSlot::setLayer(const char* name) {
  int len = strlen(name) + 1;
  if (this->layerNameLength_ < len) {
    if (this->layerName_) defFree(this->layerName_);
    this->layerName_ = (char*)defMalloc(len);
    this->layerNameLength_ = len;
  }
  strcpy(this->layerName_, DEFCASE(name));
  this->hasLayer_ = 1;
}


void defiSlot::addRect(int xl, int yl, int xh, int yh) {
  if (this->numRectangles_ == this->rectsAllocated_) {
    int i;
    int max = this->rectsAllocated_ = this->rectsAllocated_ * 2;
    int* newxl = (int*)defMalloc(sizeof(int)*max);
    int* newyl = (int*)defMalloc(sizeof(int)*max);
    int* newxh = (int*)defMalloc(sizeof(int)*max);
    int* newyh = (int*)defMalloc(sizeof(int)*max);
    for (i = 0; i < this->numRectangles_; i++) {
      newxl[i] = this->xl_[i];
      newyl[i] = this->yl_[i];
      newxh[i] = this->xh_[i];
      newyh[i] = this->yh_[i];
    }
    defFree((char*)(this->xl_));
    defFree((char*)(this->yl_));
    defFree((char*)(this->xh_));
    defFree((char*)(this->yh_));
    this->xl_ = newxl;
    this->yl_ = newyl;
    this->xh_ = newxh;
    this->yh_ = newyh;
  }
  this->xl_[this->numRectangles_] = xl;
  this->yl_[this->numRectangles_] = yl;
  this->xh_[this->numRectangles_] = xh;
  this->yh_[this->numRectangles_] = yh;
  this->numRectangles_ += 1;
}


int defiSlot::hasLayer() const {
  return this->hasLayer_;
}


const char* defiSlot::layerName() const {
  return this->layerName_;
}


int defiSlot::numRectangles() const {
  return this->numRectangles_;
}


int defiSlot::xl(int index) const {
  if (index < 0 || index >= this->numRectangles_) {
    defiError("bad index for Slot xl");
    return 0;
  }
  return this->xl_[index];
}


int defiSlot::yl(int index) const {
  if (index < 0 || index >= this->numRectangles_) {
    defiError("bad index for Slot yl");
    return 0;
  }
  return this->yl_[index];
}


int defiSlot::xh(int index) const {
  if (index < 0 || index >= this->numRectangles_) {
    defiError("bad index for Slot xh");
    return 0;
  }
  return this->xh_[index];
}


int defiSlot::yh(int index) const {
  if (index < 0 || index >= this->numRectangles_) {
    defiError("bad index for Slot yh");
    return 0;
  }
  return this->yh_[index];
}


void defiSlot::print(FILE* f) const {
  int i;

  if (this->defiSlot::hasLayer())
    fprintf(f, "- LAYER %s\n", this->defiSlot::layerName());

  for (i = 0; i < this->defiSlot::numRectangles(); i++) {
    fprintf(f, "   RECT %d %d %d %d\n", this->defiSlot::xl(i),
            this->defiSlot::yl(i), this->defiSlot::xh(i),
            this->defiSlot::yh(i));
  }
  fprintf(f,"\n");
}
