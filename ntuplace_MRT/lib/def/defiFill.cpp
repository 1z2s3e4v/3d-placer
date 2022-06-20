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
#include "defiFill.hpp"
#include "defiDebug.hpp"

////////////////////////////////////////////////////
////////////////////////////////////////////////////
//
//    defiFill
//
////////////////////////////////////////////////////
////////////////////////////////////////////////////

defiFill::defiFill() {
  this->defiFill::Init();
}


void defiFill::Init() {
  this->defiFill::clear();
  this->layerNameLength_ = 0;
  this->xl_ = (int*)malloc(sizeof(int)*1);
  this->yl_ = (int*)malloc(sizeof(int)*1);
  this->xh_ = (int*)malloc(sizeof(int)*1);
  this->yh_ = (int*)malloc(sizeof(int)*1);
  this->rectsAllocated_ = 1;      // At least 1 rectangle will define
}


defiFill::~defiFill() {
  this->defiFill::Destroy();
}


void defiFill::clear() {
  this->hasLayer_ = 0;
  this->numRectangles_ = 0;
}


void defiFill::Destroy() {
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
  this->defiFill::clear();
}


void defiFill::setLayer(const char* name) {
  int len = strlen(name) + 1;
  if (this->layerNameLength_ < len) {
    if (this->layerName_) defFree(this->layerName_);
    this->layerName_ = (char*)malloc(len);
    this->layerNameLength_ = len;
  }
  strcpy(this->layerName_, DEFCASE(name));
  this->hasLayer_ = 1;
}


void defiFill::addRect(int xl, int yl, int xh, int yh) {
  if (this->numRectangles_ == this->rectsAllocated_) {
    int i;
    int max = this->rectsAllocated_ = this->rectsAllocated_ * 2;
    int* newxl = (int*)malloc(sizeof(int)*max);
    int* newyl = (int*)malloc(sizeof(int)*max);
    int* newxh = (int*)malloc(sizeof(int)*max);
    int* newyh = (int*)malloc(sizeof(int)*max);
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


int defiFill::hasLayer() const {
  return this->hasLayer_;
}


const char* defiFill::layerName() const {
  return this->layerName_;
}


int defiFill::numRectangles() const {
  return this->numRectangles_;
}


int defiFill::xl(int index) const {
  if (index < 0 || index >= this->numRectangles_) {
    defiError("bad index for Fill xl");
    return 0;
  }
  return this->xl_[index];
}


int defiFill::yl(int index) const {
  if (index < 0 || index >= this->numRectangles_) {
    defiError("bad index for Fill yl");
    return 0;
  }
  return this->yl_[index];
}


int defiFill::xh(int index) const {
  if (index < 0 || index >= this->numRectangles_) {
    defiError("bad index for Fill xh");
    return 0;
  }
  return this->xh_[index];
}


int defiFill::yh(int index) const {
  if (index < 0 || index >= this->numRectangles_) {
    defiError("bad index for Fill yh");
    return 0;
  }
  return this->yh_[index];
}


void defiFill::print(FILE* f) const {
  int i;

  if (this->defiFill::hasLayer())
    fprintf(f, "- LAYER %s\n", this->defiFill::layerName());

  for (i = 0; i < this->defiFill::numRectangles(); i++) {
    fprintf(f, "   RECT %d %d %d %d\n", this->defiFill::xl(i),
            this->defiFill::yl(i), this->defiFill::xh(i),
            this->defiFill::yh(i));
  }
  fprintf(f,"\n");
}
