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
#include "defiRowTrack.hpp"
#include "defiDebug.hpp"
#include "defiUtil.hpp"


//////////////////////////////////////////////
//////////////////////////////////////////////
//
//   defiRow
//
//////////////////////////////////////////////
//////////////////////////////////////////////


defiRow::defiRow() {
  this->defiRow::Init();
}


void defiRow::Init() {
  this->nameLength_ = 0;
  this->name_ = 0;
  this->macroLength_ = 0;
  this->macro_ = 0;
  this->orient_ = 0;
  this->x_ = 0.0;
  this->y_ = 0.0;
  this->xStep_ = 0.0;
  this->yStep_ = 0.0;
  this->xNum_ = 0.0;
  this->yNum_ = 0.0;
  this->numProps_ = 0;
  this->propsAllocated_ = 2;
  this->propNames_ = (char**)defMalloc(sizeof(char*)*2);
  this->propValues_ = (char**)defMalloc(sizeof(char*)*2);
  this->propDValues_ = (double*)defMalloc(sizeof(double)*2);
  this->propTypes_ = (char*)defMalloc(sizeof(char)*2);
}


defiRow::~defiRow() {
  this->defiRow::Destroy();
}


void defiRow::Destroy() {
  this->defiRow::clear();
  if (this->name_) defFree(this->name_);
  if (this->macro_) defFree(this->macro_);
  defFree((char*)(this->propNames_));
  defFree((char*)(this->propValues_));
  defFree((char*)(this->propDValues_));
  defFree((char*)(this->propTypes_));
}


void defiRow::clear() {
  int i;
  for (i = 0; i < this->numProps_; i++) {
    defFree(this->propNames_[i]);
    defFree(this->propValues_[i]);
    this->propDValues_[i] = 0;
  }
  this->numProps_ = 0;
}


void defiRow::setup(const char* name, const char* macro, double x, double y,
		 int orient) {
  int len = strlen(name) + 1;

  this->defiRow::clear();

  if (len > this->nameLength_) {
    if (this->name_) defFree(this->name_);
    this->nameLength_ = len;
    this->name_ = (char*)defMalloc(len);
  }
  strcpy(this->name_, DEFCASE(name));

  len = strlen(macro) + 1;
  if (len > this->macroLength_) {
    if (this->macro_) defFree(this->macro_);
    this->macroLength_ = len;
    this->macro_ = (char*)defMalloc(len);
  }
  strcpy(this->macro_, DEFCASE(macro));

  this->x_ = x;
  this->y_ = y;
  this->xStep_ = 0.0;
  this->yStep_ = 0.0;
  this->xNum_ = 0.0;
  this->yNum_ = 0.0;
  this->orient_ = orient;

}


void defiRow::setDo(double x_num, double y_num,
		    double x_step, double y_step) {
  this->xStep_ = x_step;
  this->yStep_ = y_step;
  this->xNum_ = x_num;
  this->yNum_ = y_num;
}


void defiRow::addProperty(const char* name, const char* value, const char type) 
{
  int len;
  if (this->numProps_ == this->propsAllocated_) {
    int i;
    char**  nn;
    char**  nv;
    double* nd;
    char*   nt;
    this->propsAllocated_ *= 2;
    nn = (char**)defMalloc(sizeof(char*)*this->propsAllocated_);
    nv = (char**)defMalloc(sizeof(char*)*this->propsAllocated_);
    nd = (double*)defMalloc(sizeof(double)*this->propsAllocated_);
    nt = (char*)defMalloc(sizeof(char)*this->propsAllocated_);
    for (i = 0; i < this->numProps_; i++) {
      nn[i] = this->propNames_[i];
      nv[i] = this->propValues_[i];
      nd[i] = this->propDValues_[i];
      nt[i] = this->propTypes_[i];
    }
    defFree((char*)(this->propNames_));
    defFree((char*)(this->propValues_));
    defFree((char*)(this->propDValues_));
    defFree((char*)(this->propTypes_));
    this->propNames_ = nn;
    this->propValues_ = nv;
    this->propDValues_ = nd;
    this->propTypes_ = nt;
  }
  len = strlen(name) + 1;
  this->propNames_[this->numProps_] = (char*)defMalloc(len);
  strcpy(this->propNames_[this->numProps_], DEFCASE(name));
  len = strlen(value) + 1;
  this->propValues_[this->numProps_] = (char*)defMalloc(len);
  strcpy(this->propValues_[this->numProps_], DEFCASE(value));
  this->propDValues_[this->numProps_] = 0;
  this->propTypes_[this->numProps_] = type;
  this->numProps_ += 1;
}


void defiRow::addNumProperty(const char* name, const double d,
                             const char* value, const char type) 
{
  int len;
  if (this->numProps_ == this->propsAllocated_) {
    int i;
    char** nn;
    char** nv;
    double* nd;
    char*  nt;
    this->propsAllocated_ *= 2;
    nn = (char**)defMalloc(sizeof(char*)*this->propsAllocated_);
    nv = (char**)defMalloc(sizeof(char*)*this->propsAllocated_);
    nd = (double*)defMalloc(sizeof(double)*this->propsAllocated_);
    nt = (char*)defMalloc(sizeof(char)*this->propsAllocated_);
    for (i = 0; i < this->numProps_; i++) {
      nn[i] = this->propNames_[i];
      nv[i] = this->propValues_[i];
      nd[i] = this->propDValues_[i];
      nt[i] = this->propTypes_[i];
    }
    defFree((char*)(this->propNames_));
    defFree((char*)(this->propValues_));
    defFree((char*)(this->propDValues_));
    defFree((char*)(this->propTypes_));
    this->propNames_ = nn;
    this->propValues_ = nv;
    this->propDValues_ = nd;
    this->propTypes_ = nt;
  }
  len = strlen(name) + 1;
  this->propNames_[this->numProps_] = (char*)defMalloc(len);
  strcpy(this->propNames_[this->numProps_], DEFCASE(name));
  len = strlen(value) + 1;
  this->propValues_[this->numProps_] = (char*)defMalloc(len);
  strcpy(this->propValues_[this->numProps_], DEFCASE(value));
  this->propDValues_[this->numProps_] = d;
  this->propTypes_[this->numProps_] = type;
  this->numProps_ += 1;
}


int defiRow::numProps() const {
  return this->numProps_;
}


const char* defiRow::propName(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError("bad index for row property");
    return 0;
  }
  return this->propNames_[index];
}


const char* defiRow::propValue(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError("bad index for row property");
    return 0;
  }
  return this->propValues_[index];
}

double defiRow::propNumber(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError("bad index for row property");
    return 0;
  }
  return this->propDValues_[index];
}

const char defiRow::propType(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError("bad index for row property");
    return 0;
  }
  return this->propTypes_[index];
}

int defiRow::propIsNumber(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError ("bad index for row property");
    return 0;
  } 
  return this->propDValues_[index] ? 1 : 0;
}

int defiRow::propIsString(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError ("bad index for row property");
    return 0;
  } 
  return this->propDValues_[index] ? 0 : 1;
}

const char* defiRow::name() const {
  return this->name_;
}


const char* defiRow::macro() const {
  return this->macro_;
}


double defiRow::x() const {
  return this->x_;
}


double defiRow::y() const {
  return this->y_;
}


double defiRow::xNum() const {
  return this->xNum_;
}


double defiRow::yNum() const {
  return this->yNum_;
}


int defiRow::orient() const {
  return this->orient_;
}


const char* defiRow::orientStr() const {
  return (defiOrientStr(this->orient_));
}


double defiRow::xStep() const {
  return this->xStep_;
}


double defiRow::yStep() const {
  return this->yStep_;
}


void defiRow::print(FILE* f) const {
  fprintf(f, "Row '%s' '%s' %g,%g  orient %s\n",
      this->defiRow::name(), this->defiRow::macro(),
      this->defiRow::x(), this->defiRow::y(), this->defiRow::orientStr());
  fprintf(f, "  DO X %g STEP %g\n", this->defiRow::xNum(),
      this->defiRow::xStep());
  fprintf(f, "  DO Y %g STEP %g\n", this->defiRow::yNum(),
      this->defiRow::yStep());
}


//////////////////////////////////////////////
//////////////////////////////////////////////
//
//   defiTrack
//
//////////////////////////////////////////////
//////////////////////////////////////////////


defiTrack::defiTrack() {
  this->defiTrack::Init();
}


void defiTrack::Init() {
  this->macro_ = 0;
  this->macroLength_ = 0;
  this->x_ = 0.0;
  this->xNum_ = 0.0;
  this->xStep_ = 0.0;
  this->numLayers_ = 0;
  this->layers_ = 0;
}

defiTrack::~defiTrack() {
  this->defiTrack::Destroy();
}


void defiTrack::Destroy() {
  int i;

  if (this->macro_) defFree(this->macro_);

  if (this->layers_) {
    for (i = 0; i < this->numLayers_; i++)
      if (this->layers_[i]) defFree(this->layers_[i]);
    defFree((char*)(this->layers_));
  }
}


void defiTrack::setup(const char* macro) {
  int i;
  int len = strlen(macro) + 1;

  if (len > this->macroLength_) {
    if (this->macro_) defFree(this->macro_);
    this->macroLength_ = len;
    this->macro_ = (char*)defMalloc(len);
  }
  strcpy(this->macro_, DEFCASE(macro));

  if (this->layers_) {
    for (i = 0; i < this->numLayers_; i++)
      if (this->layers_[i]) defFree(this->layers_[i]);
      this->layers_[i] = 0;
  }
  this->numLayers_ = 0;
  this->x_ = 0.0;
  this->xStep_ = 0.0;
  this->xNum_ = 0.0;

}


void defiTrack::setDo(double x, double x_num, double x_step) {
  this->x_ = x;
  this->xStep_ = x_step;
  this->xNum_ = x_num;
}


void defiTrack::addLayer(const char* layer) {
  char* l;
  int len;

  if (this->numLayers_ >= this->layersLength_) {
    int i;
    char** newl;
    this->layersLength_ = this->layersLength_ ? 2 * this->layersLength_ : 8;
    newl = (char**)defMalloc(this->layersLength_* sizeof(char*));
    for (i = 0; i < this->numLayers_; i++)
      newl[i] = this->layers_[i];
    if (this->layers_) defFree((char*)(this->layers_));
    this->layers_ = newl;
  }

  len = strlen(layer) + 1;
  l = (char*)defMalloc(len);
  strcpy(l, DEFCASE(layer));
  this->layers_[this->numLayers_++] = l;
}


const char* defiTrack::macro() const {
  return this->macro_;
}


double defiTrack::x() const {
  return this->x_;
}


double defiTrack::xNum() const {
  return this->xNum_;
}


double defiTrack::xStep() const {
  return this->xStep_;
}


int defiTrack::numLayers() const {
  return this->numLayers_;
}


const char* defiTrack::layer(int index) const {
  if (index >= 0 && index < this->numLayers_) {
    return this->layers_[index];
  }

  return 0;
}


void defiTrack::print(FILE* f) const {
  int i;

  fprintf(f, "Track '%s'\n", this->defiTrack::macro());
  fprintf(f, "  DO %g %g STEP %g\n",
      this->defiTrack::x(),
      this->defiTrack::xNum(),
      this->defiTrack::xStep());
  fprintf(f, "  %d layers ", this->defiTrack::numLayers());
  for (i = 0; i < this->defiTrack::numLayers(); i++) {
    fprintf(f, " '%s'", this->defiTrack::layer(i));
  }
  fprintf(f, "\n");
}


//////////////////////////////////////////////
//////////////////////////////////////////////
//
//   defiGcellGrid
//
//////////////////////////////////////////////
//////////////////////////////////////////////


defiGcellGrid::defiGcellGrid() {
  this->defiGcellGrid::Init();
}


void defiGcellGrid::Init() {
  this->macro_ = 0;
  this->macroLength_ = 0;
  this->x_ = 0;
  this->xNum_ = 0;
  this->xStep_ = 0;
}


defiGcellGrid::~defiGcellGrid() {
  this->defiGcellGrid::Destroy();
}


void defiGcellGrid::Destroy() {
  if (this->macro_) defFree(this->macro_);
}


void defiGcellGrid::setup(const char* macro, int x, int xNum, double xStep) {
  int len = strlen(macro) + 1;
  if (len > this->macroLength_) {
    if (this->macro_) defFree(this->macro_);
    this->macroLength_ = len;
    this->macro_ = (char*)defMalloc(len);
  }
  strcpy(this->macro_, DEFCASE(macro));

  this->x_ = x;
  this->xNum_ = xNum;
  this->xStep_ = xStep;
}


int defiGcellGrid::x() const {
  return this->x_;
}


int defiGcellGrid::xNum() const {
  return this->xNum_;
}


double defiGcellGrid::xStep() const {
  return this->xStep_;
}


const char* defiGcellGrid::macro() const {
  return this->macro_;
}


void defiGcellGrid::print(FILE* f) const {
  fprintf(f, "GcellGrid '%s'\n", this->defiGcellGrid::macro());
  fprintf(f, "  DO %d %d STEP %5.1f\n",
      this->defiGcellGrid::x(),
      this->defiGcellGrid::xNum(),
      this->defiGcellGrid::xStep());
}


