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
#include <stdlib.h>
#include "lex.h"
#include "defiVia.hpp"
#include "defiDebug.hpp"


//////////////////////////////////////////////
//////////////////////////////////////////////
//
//   defiVia
//
//////////////////////////////////////////////
//////////////////////////////////////////////


defiVia::defiVia() {
  this->defiVia::Init();
}


void defiVia::Init() {
  this->name_ = 0;
  this->nameLength_ = 0;
  this->pattern_ = 0;
  this->patternLength_ = 0;
  this->xl_ = 0;
  this->yl_ = 0;
  this->xh_ = 0;
  this->yh_ = 0;
  this->numLayers_ = 0;
  this->layersLength_ = 0;
  this->layers_ = 0;
}

defiVia::~defiVia() {
  this->defiVia::Destroy();
}


void defiVia::Destroy() {
  int i;

  if (this->name_) defFree(this->name_);
  if (this->pattern_) defFree(this->pattern_);

  if (this->layers_) {
    for (i = 0; i < this->numLayers_; i++)
      if (this->layers_[i]) defFree(this->layers_[i]);
    defFree((char*)(this->layers_));
    defFree((char*)(this->xl_));
    defFree((char*)(this->yl_));
    defFree((char*)(this->xh_));
    defFree((char*)(this->yh_));
  }
}


void defiVia::setup(const char* name) {
  int i;
  int len = strlen(name) + 1;
  if (len > this->nameLength_) {
    this->nameLength_ = len;
    this->name_ = (char*)defRealloc(this->name_, len);
  }
  strcpy(this->name_, DEFCASE(name));
  if (this->pattern_) *(this->pattern_) = 0;
  if (this->layers_) {
    for (i = 0; i < this->numLayers_; i++) {
      if (this->layers_[i]) defFree(this->layers_[i]);
    this->layers_[i] = 0;
    }
  }
  this->numLayers_ = 0;

}


void defiVia::addPattern(const char* pattern) {
  int len = strlen(pattern) + 1;
  if (len > this->patternLength_) {
    this->patternLength_ = len;
    this->pattern_ = (char*)defRealloc(this->pattern_, len);
  }
  strcpy(this->pattern_, DEFCASE(pattern));

}


void defiVia::addLayer(const char* layer, int xl, int yl, int xh, int yh) {
  char* l;
  int len;

  if (this->numLayers_ >= this->layersLength_) {
    int i;
    char** newl;
    int* ints;
    this->layersLength_ = this->layersLength_ ? 2 * this->layersLength_ : 8;

    newl = (char**)defMalloc(this->layersLength_ * sizeof(char*));
    for (i = 0; i < this->numLayers_; i++)
      newl[i] = this->layers_[i];
    if (this->layers_) defFree((char*)(this->layers_));
    this->layers_ = newl;

    ints = (int*)defMalloc(this->layersLength_ * sizeof(int));
    for (i = 0; i < this->numLayers_; i++)
      ints[i] = this->xl_[i];
    if (this->xl_) defFree((char*)(this->xl_));
    this->xl_ = ints;

    ints = (int*)defMalloc(this->layersLength_ * sizeof(int));
    for (i = 0; i < this->numLayers_; i++)
      ints[i] = this->yl_[i];
    if (this->yl_) defFree((char*)(this->yl_));
    this->yl_ = ints;

    ints = (int*)defMalloc(this->layersLength_ * sizeof(int));
    for (i = 0; i < this->numLayers_; i++)
      ints[i] = this->xh_[i];
    if (this->xh_) defFree((char*)(this->xh_));
    this->xh_ = ints;

    ints = (int*)defMalloc(this->layersLength_ * sizeof(int));
    for (i = 0; i < this->numLayers_; i++)
      ints[i] = this->yh_[i];
    if (this->yh_) defFree((char*)(this->yh_));
    this->yh_ = ints;
  }

  len = strlen(layer) + 1;
  l = (char*)defMalloc(len);
  strcpy(l, DEFCASE(layer));
  this->layers_[this->numLayers_] = l;
  this->xl_[this->numLayers_] = xl;
  this->yl_[this->numLayers_] = yl;
  this->xh_[this->numLayers_] = xh;
  this->yh_[this->numLayers_] = yh;
  this->numLayers_++;
}


int defiVia::hasPattern() const {
  return this->pattern_ && *(this->pattern_) ? 1 : 0 ;
}


const char* defiVia::pattern() const {
  return this->pattern_;
}


const char* defiVia::name() const {
  return this->name_;
}


int defiVia::numLayers() const {
  return this->numLayers_;
}


void defiVia::layer(int index, char** layer, int* xl, int* yl, 
      int* xh, int* yh) const {
  if (index >= 0 && index < this->numLayers_) {
    if (layer) *layer = this->layers_[index];
    if (xl) *xl = this->xl_[index];
    if (yl) *yl = this->yl_[index];
    if (xh) *xh = this->xh_[index];
    if (yh) *yh = this->yh_[index];
  }
}


void defiVia::print(FILE* f) const {
  int i;
  int xl, yl, xh, yh;
  char* c;

  fprintf(f, "via '%s'\n", this->defiVia::name());

  if (this->defiVia::hasPattern())
    fprintf(f, "  pattern '%s'\n", this->defiVia::pattern());

  for (i = 0; i < this->defiVia::numLayers(); i++) {
    this->defiVia::layer(i, &c, &xl, &yl, &xh, &yh);
    fprintf(f, "  layer '%s' %d,%d %d,%d\n", c, xl, yl, xh, yh);
  }
}


