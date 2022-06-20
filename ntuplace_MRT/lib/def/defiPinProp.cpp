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
#include "defiDebug.hpp"
#include "defiPinProp.hpp"


////////////////////////////////////////////////////
////////////////////////////////////////////////////
//
//        defiPinProp
//
////////////////////////////////////////////////////
////////////////////////////////////////////////////



defiPinProp::defiPinProp() {
  this->defiPinProp::Init();
}


void defiPinProp::Init() {
  this->instName_ = (char*)defMalloc(16);
  this->pinName_ = (char*)defMalloc(16);
  this->pinNameSize_ = 16;
  this->instNameSize_ = 16;
  this->isPin_ = 0;
  this->numProps_ = 0;
  this->propsAllocated_ = 2;
  this->propNames_   = (char**)defMalloc(sizeof(char*)*2);
  this->propValues_  = (char**)defMalloc(sizeof(char*)*2);
  this->propDValues_ = (double*)defMalloc(sizeof(double)*2);
  this->propTypes_   = (char*)defMalloc(sizeof(char)*2);
}


defiPinProp::~defiPinProp() {
  this->defiPinProp::Destroy();
}


void defiPinProp::Destroy() {
  this->defiPinProp::clear();
  defFree(this->instName_);
  defFree(this->pinName_);
  defFree((char*)(this->propNames_));
  defFree((char*)(this->propValues_));
  defFree((char*)(this->propDValues_));
  defFree((char*)(this->propTypes_));
}


void defiPinProp::clear() {
  int i;
  for (i = 0; i < this->numProps_; i++) {
    defFree(this->propNames_[i]);
    defFree(this->propValues_[i]);
    this->propDValues_[i] = 0;
  }
  this->numProps_ = 0;
  this->isPin_ = 0;
}


void defiPinProp::setName(const char* inst, const char* pin) {
  int len = strlen(inst) + 1;

  if ((strcmp(inst, "PIN") == 0) || (strcmp(inst,"pin") == 0)) {
     this->isPin_ = 1;
     this->instName_[0] = '\0'; /* make sure to clear any prev inst */
  } else {
     if (this->instNameSize_ < len) {
       this->instNameSize_ = len;
       defFree(this->instName_);
       this->instName_ = (char*)defMalloc(len);
     }
     strcpy(this->instName_, DEFCASE(inst));
  }

  len = strlen(pin) + 1;
  if (this->pinNameSize_ < len) {
    this->pinNameSize_ = len;
    defFree(this->pinName_);
    this->pinName_ = (char*)defMalloc(len);
  }
  strcpy(this->pinName_, DEFCASE(pin));
}


int defiPinProp::isPin() const {
  return this->isPin_ ? 1 : 0;
}


const char* defiPinProp::instName() const {
  return this->instName_;
}


const char* defiPinProp::pinName() const {
  return this->pinName_;
}


void defiPinProp::addProperty(const char* name, const char* value,
                              const char type) {
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
    this->propNames_   = nn;
    this->propValues_  = nv;
    this->propDValues_ = nd;
    this->propTypes_   = nt;
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


void defiPinProp::addNumProperty(const char* name, const double d,
                                 const char* value, const char type) {
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
    this->propNames_   = nn;
    this->propValues_  = nv;
    this->propDValues_ = nd;
    this->propTypes_   = nt;
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


int defiPinProp::numProps() const {
  return this->numProps_;
}


const char* defiPinProp::propName(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError("bad index for PinProp property");
    return 0;
  }
  return this->propNames_[index];
}


const char* defiPinProp::propValue(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError("bad index for PinProp property");
    return 0;
  }
  return this->propValues_[index];
}


double defiPinProp::propNumber(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError("bad index for PinProp property");
    return 0;
  }
  return this->propDValues_[index];
}


const char defiPinProp::propType(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError("bad index for PinProp property");
    return 0;
  }
  return this->propTypes_[index];
}


int defiPinProp::propIsNumber(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError("bad index for PinProp property");
    return 0;
  }
  return this->propDValues_[index] ? 1 : 0;
}


int defiPinProp::propIsString(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError("bad index for PinProp property");
    return 0;
  }
  return this->propDValues_[index] ? 0 : 1;
}


void defiPinProp::print(FILE* f) const {
  int i;

  fprintf(f, "PinProp %s %s\n", this->defiPinProp::instName(),
     this->defiPinProp::pinName());

  for (i = 0; i < this->defiPinProp::numProps(); i++) {
    fprintf(f, "    %s %s\n", this->defiPinProp::propName(i),
                              this->defiPinProp::propValue(i));
  }
}


