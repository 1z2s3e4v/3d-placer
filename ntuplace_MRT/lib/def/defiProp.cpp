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

#include <stdlib.h>
#include <string.h>
#include "lex.h"
#include "defiProp.hpp"
#include "defiDebug.hpp"


defiProp::defiProp() {
  this->defiProp::Init();
}


void defiProp::Init() {
  this->stringLength_ = 16;
  this->stringData_ = (char*)defMalloc(16);
  this->nameSize_ = 16;
  this->propName_ = (char*)defMalloc(16);
  this->defiProp::clear();
}


void defiProp::Destroy() {
  defFree(this->stringData_);
  defFree(this->propName_);
}


defiProp::~defiProp() {
  this->defiProp::Destroy();
}


void defiProp::setPropType(const char* typ, const char* string) {
  int len;
  this->propType_ = (char*)typ;
  if ((len = strlen(string)+1) > this->nameSize_)
    this->defiProp::bumpName(len);
  strcpy(this->propName_, DEFCASE(string));
}


void defiProp::setRange(double left, double right) {
  this->hasRange_ = 1;
  this->left_ = left;
  this->right_ = right;
}


void defiProp::setNumber(double d) {
  this->hasNumber_ = 1;
  this->d_ = d;
}


void defiProp::setPropInteger() {
  this->dataType_ = 'I';
}


void defiProp::setPropReal() {
  this->dataType_ = 'R';
}


void defiProp::setPropString() {
  this->dataType_ = 'S';
}


void defiProp::setPropNameMapString(const char* string) {
  int len;
  this->dataType_ = 'N';
  this->hasNameMapString_ = 1;
  if ((len = strlen(string)+1) > this->stringLength_)
    this->defiProp::bumpSize(len);
  strcpy(this->stringData_, DEFCASE(string));
}


void defiProp::setPropQString(const char* string) {
  int len;
  this->dataType_ = 'Q';
  if ((len = strlen(string)+1) > this->stringLength_)
    this->defiProp::bumpSize(len);
  strcpy(this->stringData_, DEFCASE(string));
}


const char* defiProp::string() const {
  return this->stringData_;
}


const char* defiProp::propType() const {
  return this->propType_;
}


int defiProp::hasNameMapString() const {
  return (int)(this->hasNameMapString_);
}


int defiProp::hasNumber() const {
  return (int)(this->hasNumber_);
}


int defiProp::hasRange() const {
  return (int)(this->hasRange_);
}


double defiProp::number() const {
  return this->d_;
}


double defiProp::left() const {
  return this->left_;
}


double defiProp::right() const {
  return this->right_;
}


void defiProp::bumpSize(int size) {
  defFree(this->stringData_);
  this->stringData_ = (char*)defMalloc(size);
  this->stringLength_ = size;
  *(this->stringData_) = '\0';
}


void defiProp::bumpName(int size) {
  defFree(this->propName_);
  this->propName_ = (char*)defMalloc(size);
  this->nameSize_ = size;
  *(this->propName_) = '\0';
}



void defiProp::clear() {
  if (this->stringData_)
     *(this->stringData_) = '\0';
  if (this->propName_)
     *(this->propName_) = '\0';
  this->propType_ = 0;
  this->hasRange_ = 0;
  this->hasNumber_ = 0;
  this->hasNameMapString_ = 0;
  this->dataType_ = 'B'; /* bogus */
  this->d_ = this->left_ = this->right_ = 0.0;
}


int defiProp::hasString() const {
  return *(this->stringData_) ? 1 : 0 ;
}


const char* defiProp::propName() const {
  return (this->propName_);
}


char defiProp::dataType() const {
  return (this->dataType_);
}


void defiProp::print(FILE* f) const {
  fprintf(f, "Prop type '%s'\n", this->defiProp::propType());
  if (this->defiProp::hasString()) {
    fprintf(f, "  string '%s'\n", this->defiProp::string());
  }
  if (this->defiProp::hasNumber()) {
    fprintf(f, "  number %5.2f\n", this->defiProp::number());
  }
  if (this->defiProp::hasRange()) {
    fprintf(f, "  range %5.2f - %5.2f\n",
      this->defiProp::left(), this->defiProp::right());
  }
}


