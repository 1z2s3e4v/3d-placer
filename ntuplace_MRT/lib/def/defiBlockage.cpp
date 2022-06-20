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
#include "defiBlockage.hpp"
#include "defiDebug.hpp"

////////////////////////////////////////////////////
////////////////////////////////////////////////////
//
//    defiBlockages
//
////////////////////////////////////////////////////
////////////////////////////////////////////////////

defiBlockage::defiBlockage() {
  this->defiBlockage::Init();
}


void defiBlockage::Init() {
  this->defiBlockage::clear();
  this->layerNameLength_ = 0;
  this->componentNameLength_ = 0;
  this->xl_ = (int*)defMalloc(sizeof(int)*1);
  this->yl_ = (int*)defMalloc(sizeof(int)*1);
  this->xh_ = (int*)defMalloc(sizeof(int)*1);
  this->yh_ = (int*)defMalloc(sizeof(int)*1);
  this->rectsAllocated_ = 1;      // At least 1 rectangle will define
}


defiBlockage::~defiBlockage() {
  this->defiBlockage::Destroy();
}


void defiBlockage::clear() {
  this->hasLayer_ = 0;
  this->hasPlacement_ = 0;
  this->hasComponent_ = 0;
  this->hasSlots_ = 0;
  this->hasFills_ = 0;
  this->hasPushdown_ = 0;
  this->numRectangles_ = 0;
}


void defiBlockage::Destroy() {
  if (this->layerName_) defFree(this->layerName_);
  if (this->componentName_) defFree(this->componentName_);
  if (this->placementName_) defFree(this->placementName_);
  defFree((char*)(this->xl_));
  defFree((char*)(this->yl_));
  defFree((char*)(this->xh_));
  defFree((char*)(this->yh_));
  this->rectsAllocated_ = 0;
  this->xl_ = 0;
  this->yl_ = 0;
  this->xh_ = 0;
  this->yh_ = 0;
  this->defiBlockage::clear();
}


void defiBlockage::setLayer(const char* name) {
  int len = strlen(name) + 1;
  if (this->layerNameLength_ < len) {
    if (this->layerName_) defFree(this->layerName_);
    this->layerName_ = (char*)defMalloc(len);
    this->layerNameLength_ = len;
  }
  strcpy(this->layerName_, DEFCASE(name));
  this->hasLayer_ = 1;
}


void defiBlockage::setPlacement() {

  /* 10/29/2001 - Wanda da Rosa, new enhancement */
  this->hasPlacement_ = 1;
  return;
}

void defiBlockage::setComponent(const char* name) {
  int len;

  /* 10/29/2001 - Wanda da Rosa, component name is required */
  len = strlen(name) + 1;
  if (this->componentNameLength_ < len) {
    if (this->componentName_) defFree(this->componentName_);
    this->componentName_ = (char*)defMalloc(len);
    this->componentNameLength_ = len;
  }
  strcpy(this->componentName_, DEFCASE(name));
  this->hasComponent_ = 1;
}

void defiBlockage::setSlots() {
  this->hasSlots_ = 1;
}

void defiBlockage::setFills() {
  this->hasFills_ = 1;
}

void defiBlockage::setPushdown() {
  this->hasPushdown_ = 1;
}

void defiBlockage::addRect(int xl, int yl, int xh, int yh) {
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


int defiBlockage::hasLayer() const {
  return this->hasLayer_;
}


int defiBlockage::hasPlacement() const {
  return this->hasPlacement_;
}


int defiBlockage::hasComponent() const {
  return this->hasComponent_;
}


int defiBlockage::hasSlots() const {
  return this->hasSlots_;
}


int defiBlockage::hasFills() const {
  return this->hasFills_;
}


int defiBlockage::hasPushdown() const {
  return this->hasPushdown_;
}


const char* defiBlockage::layerName() const {
  return this->layerName_;
}


const char* defiBlockage::layerComponentName() const {
  return this->componentName_;
}


const char* defiBlockage::placementComponentName() const {
  return this->placementName_;
}


int defiBlockage::numRectangles() const {
  return this->numRectangles_;
}


int defiBlockage::xl(int index) const {
  if (index < 0 || index >= this->numRectangles_) {
    defiError("bad index for blockage xl");
    return 0;
  }
  return this->xl_[index];
}


int defiBlockage::yl(int index) const {
  if (index < 0 || index >= this->numRectangles_) {
    defiError("bad index for blockage yl");
    return 0;
  }
  return this->yl_[index];
}


int defiBlockage::xh(int index) const {
  if (index < 0 || index >= this->numRectangles_) {
    defiError("bad index for blockage xh");
    return 0;
  }
  return this->xh_[index];
}


int defiBlockage::yh(int index) const {
  if (index < 0 || index >= this->numRectangles_) {
    defiError("bad index for blockage yh");
    return 0;
  }
  return this->yh_[index];
}


void defiBlockage::print(FILE* f) const {
  int i;

  if (this->defiBlockage::hasLayer()) {
    fprintf(f, "- LAYER %s", this->defiBlockage::layerName());
    if (this->defiBlockage::hasComponent())
      fprintf(f, " + COMPONENT %s", this->defiBlockage::layerComponentName());
    if (this->defiBlockage::hasSlots())
      fprintf(f, " + SLOTS");
    if (this->defiBlockage::hasFills())
      fprintf(f, " + FILLS");
    if (this->defiBlockage::hasPushdown())
      fprintf(f, " + PUSHDOWN");
    fprintf(f, "\n");
  }
  if (this->defiBlockage::hasPlacement()) {
    fprintf(f, "- PLACEMENT");
    if (this->defiBlockage::hasComponent())
      fprintf(f, " + COMPONENT %s", this->defiBlockage::layerComponentName());
    if (this->defiBlockage::hasPushdown())
      fprintf(f, " + PUSHDOWN");
    fprintf(f, "\n");
  }

  for (i = 0; i < this->defiBlockage::numRectangles(); i++) {
    fprintf(f, "   RECT %d %d %d %d\n", this->defiBlockage::xl(i),
            this->defiBlockage::yl(i), this->defiBlockage::xh(i),
            this->defiBlockage::yh(i));
  }
  fprintf(f,"\n");
}
