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
#include "defiTimingDisable.hpp"
#include "defiDebug.hpp"


//////////////////////////////////////////////
//////////////////////////////////////////////
//
//   defiTimingDisable
//
//////////////////////////////////////////////
//////////////////////////////////////////////


defiTimingDisable::defiTimingDisable() {
  this->defiTimingDisable::Init();
}


void defiTimingDisable::Init() {
  this->fromInst_ = 0;
  this->fromInstLength_ = 0;
  this->toInst_ = 0;
  this->toInstLength_ = 0;
  this->toPin_ = 0;
  this->toPinLength_ = 0;
  this->fromPin_ = 0;
  this->fromPinLength_ = 0;
}


defiTimingDisable::~defiTimingDisable() {
  this->defiTimingDisable::Destroy();
}


void defiTimingDisable::Destroy() {

  this->defiTimingDisable::clear();

  if (this->fromInst_) defFree(this->fromInst_);
  this->fromInst_ = 0;
  this->fromInstLength_ = 0;

  if (this->toInst_) defFree(this->toInst_);
  this->toInst_ = 0;
  this->toInstLength_ = 0;

  if (this->toPin_) defFree(this->toPin_);
  this->toPin_ = 0;
  this->toPinLength_ = 0;

  if (this->fromPin_) defFree(this->fromPin_);
  this->fromPin_ = 0;
  this->fromPinLength_ = 0;

}


void defiTimingDisable::clear() {
  this->hasFromTo_ = 0;
  this->hasThru_ = 0;
  this->hasMacro_ = 0;
  this->hasReentrantPathsFlag_ = 0;
}


void defiTimingDisable::setReentrantPathsFlag() {
  this->hasReentrantPathsFlag_ = 1;
}


void defiTimingDisable::setFromTo(const char* fromInst, const char* fromPin,
	 const char* toInst, const char* toPin) {
  int len;

  this->defiTimingDisable::clear();
  this->hasFromTo_ = 1;

  len = strlen(fromInst) + 1;
  if (len > this->fromInstLength_) {
    if (this->fromInst_) defFree(this->fromInst_);
    this->fromInstLength_ = len;
    this->fromInst_ = (char*)defMalloc(len);
  }
  strcpy(this->fromInst_, DEFCASE(fromInst));

  len = strlen(fromPin) + 1;
  if (len > this->fromPinLength_) {
    if (this->fromPin_) defFree(this->fromPin_);
    this->fromPinLength_ = len;
    this->fromPin_ = (char*)defMalloc(len);
  }
  strcpy(this->fromPin_, DEFCASE(fromPin));

  len = strlen(toInst) + 1;
  if (len > this->toInstLength_) {
    if (this->toInst_) defFree(this->toInst_);
    this->toInstLength_ = len;
    this->toInst_ = (char*)defMalloc(len);
  }
  strcpy(this->toInst_, toInst);

  len = strlen(toPin) + 1;
  if (len > this->toPinLength_) {
    if (this->toPin_) defFree(this->toPin_);
    this->toPinLength_ = len;
    this->toPin_ = (char*)defMalloc(len);
  }
  strcpy(this->toPin_, toPin);

}


void defiTimingDisable::setThru(const char* fromInst, const char* fromPin) {
  int len;

  this->defiTimingDisable::clear();
  this->hasThru_ = 1;

  len = strlen(fromInst) + 1;
  if (len > this->fromInstLength_) {
    if (this->fromInst_) defFree(this->fromInst_);
    this->fromInstLength_ = len;
    this->fromInst_ = (char*)defMalloc(len);
  }
  strcpy(this->fromInst_, DEFCASE(fromInst));

  len = strlen(fromPin) + 1;
  if (len > this->fromPinLength_) {
    if (this->fromPin_) defFree(this->fromPin_);
    this->fromPinLength_ = len;
    this->fromPin_ = (char*)defMalloc(len);
  }
  strcpy(this->fromPin_, DEFCASE(fromPin));

}


void defiTimingDisable::setMacroFromTo(const char* fromPin, const char* toPin) {
  int len;

  this->defiTimingDisable::clear();
  this->hasFromTo_ = 1;

  len = strlen(fromPin) + 1;
  if (len > this->fromPinLength_) {
    if (this->fromPin_) defFree(this->fromPin_);
    this->fromPinLength_ = len;
    this->fromPin_ = (char*)defMalloc(len);
  }
  strcpy(this->fromPin_, DEFCASE(fromPin));

  len = strlen(toPin) + 1;
  if (len > this->toPinLength_) {
    if (this->toPin_) defFree(this->toPin_);
    this->toPinLength_ = len;
    this->toPin_ = (char*)defMalloc(len);
  }
  strcpy(this->toPin_, DEFCASE(toPin));

}


void defiTimingDisable::setMacroThru(const char* thru) {
  int len;

  this->defiTimingDisable::clear();

  this->hasThru_ = 1;

  len = strlen(thru) + 1;
  if (len > this->fromPinLength_) {
    if (this->fromPin_) defFree(this->fromPin_);
    this->fromPinLength_ = len;
    this->fromPin_ = (char*)defMalloc(len);
  }
  strcpy(this->fromPin_, DEFCASE(thru));

}


void defiTimingDisable::setMacro(const char* name) {
  int len;

  // hasThru_ or hasFromTo_ was already set.
  // clear() was already called.
  this->hasMacro_ = 1;

  len = strlen(name) + 1;
  if (len > this->fromInstLength_) {
    if (this->fromInst_) defFree(this->fromInst_);
    this->fromInstLength_ = len;
    this->fromInst_ = (char*)defMalloc(len);
  }
  strcpy(this->fromInst_, DEFCASE(name));
}


void defiTimingDisable::print(FILE* f) const {

  if (this->defiTimingDisable::hasMacroFromTo()) {
    fprintf(f, "TimingDisable macro '%s' thru '%s'\n",
	this->fromInst_, this->fromPin_);

  } else if (this->defiTimingDisable::hasMacroThru()) {
    fprintf(f, "TimingDisable macro '%s' from '%s' to '%s'\n",
	this->fromInst_, this->fromPin_, this->toPin_);

  } else if (this->defiTimingDisable::hasFromTo()) {
    fprintf(f, "TimingDisable from '%s' '%s'  to '%s' '%s'\n",
      this->fromInst_, this->fromPin_, this->toInst_, this->toPin_);

  } else if (this->defiTimingDisable::hasThru()) {
    fprintf(f, "TimingDisable thru '%s' '%s'\n",
      this->fromInst_, this->fromPin_);

  } else {
    defiError("Bad TimingDisable type");
  }
}


int defiTimingDisable::hasReentrantPathsFlag() const {
  return this->hasReentrantPathsFlag_;
}


int defiTimingDisable::hasMacroFromTo() const {
  return (this->hasMacro_ && this->hasFromTo_) ? 1 : 0;
}


int defiTimingDisable::hasMacroThru() const {
  return (this->hasMacro_ && this->hasThru_) ? 1 : 0;
}


int defiTimingDisable::hasThru() const {
  return (this->hasMacro_ == 0 && this->hasThru_) ? 1 : 0;
}


int defiTimingDisable::hasFromTo() const {
  return (this->hasMacro_ == 0 && this->hasFromTo_) ? 1 : 0;
}


const char* defiTimingDisable::toPin() const {
  return this->toPin_;
}


const char* defiTimingDisable::fromPin() const {
  return this->fromPin_;
}


const char* defiTimingDisable::toInst() const {
  return this->toInst_;
}


const char* defiTimingDisable::fromInst() const {
  return this->fromInst_;
}


const char* defiTimingDisable::macroName() const {
  return this->fromInst_;
}


const char* defiTimingDisable::thruPin() const {
  return this->fromPin_;
}


const char* defiTimingDisable::thruInst() const {
  return this->fromInst_;
}


