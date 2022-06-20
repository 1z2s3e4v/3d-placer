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
#include "defiPartition.hpp"
#include "defiDebug.hpp"


//////////////////////////////////////////////
//////////////////////////////////////////////
//
//   defiPartition
//
//////////////////////////////////////////////
//////////////////////////////////////////////


defiPartition::defiPartition() {
  this->defiPartition::Init();
}


void defiPartition::Init() {
  this->name_ = 0;
  this->nameLength_ = 0;
  this->pin_ = 0;
  this->pinLength_ = 0;
  this->inst_ = 0;
  this->instLength_ = 0;

  this->pinsAllocated_ = 0;
  this->numPins_ = 0;
  this->pins_ = 0;

  this->defiPartition::clear();
}


defiPartition::~defiPartition() {
  this->defiPartition::Destroy();
}


void defiPartition::Destroy() {

  if (this->name_) defFree(this->name_);
  this->name_ = 0;
  this->nameLength_ = 0;
  if (this->pin_) defFree(this->pin_);
  this->pin_ = 0;
  this->pinLength_ = 0;
  if (this->inst_) defFree(this->inst_);
  this->inst_ = 0;
  this->instLength_ = 0;

  this->defiPartition::clear();

  if (this->pins_) defFree((char*)(this->pins_));
  this->pins_ = 0;
  this->pinsAllocated_ = 0;
}


void defiPartition::clear() {
  int i;

  this->setup_ = ' ';
  this->hold_ = ' ';
  this->direction_ = ' ';
  this->type_ = ' ';
  if (this->name_) *(this->name_) = '\0';
  if (this->pin_) *(this->pin_) = '\0';
  if (this->inst_) *(this->inst_) = '\0';
  this->hasMin_ = 0;
  this->hasMax_ = 0;

  if (this->numPins_) {
    for (i = 0; i < this->numPins_; i++) {
      defFree(this->pins_[i]);
      this->pins_[i] = 0;
    }
    this->numPins_ = 0;
  }
  this->hasRiseMin_ = 0;
  this->hasFallMin_ = 0;
  this->hasRiseMax_ = 0;
  this->hasFallMax_ = 0;
  this->hasRiseMinRange_ = 0;
  this->hasFallMinRange_ = 0;  
  this->hasRiseMaxRange_ = 0;
  this->hasFallMaxRange_ = 0;
}


void defiPartition::setName(const char* name) {
  int len = strlen(name) + 1;

  this->defiPartition::clear();

  if (len > this->nameLength_) {
    if (this->name_) defFree(this->name_);
    this->nameLength_ = len;
    this->name_ = (char*)defMalloc(len);
  }
  strcpy(this->name_, DEFCASE(name));

}


void defiPartition::print(FILE* f) const {
  int i;

  fprintf(f, "Partition '%s' %c\n",
       this->defiPartition::name(), this->defiPartition::direction());
  fprintf(f, "  inst %s  pin %s  type %s\n",
       this->defiPartition::instName(), this->defiPartition::pinName(),
       this->defiPartition::itemType());

  for (i = 0; i < this->defiPartition::numPins(); i++)
    fprintf(f, "  %s\n", this->defiPartition::pin(i));

  if (this->defiPartition::isSetupRise())
    fprintf(f, "  SETUP RISE\n");

  if (this->defiPartition::isSetupFall())
    fprintf(f, "  SETUP FALL\n");

  if (this->defiPartition::isHoldRise())
    fprintf(f, "  HOLD RISE\n");

  if (this->defiPartition::isHoldFall())
    fprintf(f, "  HOLD FALL\n");

  if (this->defiPartition::hasMin())
    fprintf(f, "  MIN %g\n", this->defiPartition::partitionMin());

  if (this->defiPartition::hasMax())
    fprintf(f, "  MAX %g\n", this->defiPartition::partitionMax());

  if (this->defiPartition::hasRiseMin())
    fprintf(f, "  RISE MIN %g\n", this->defiPartition::riseMin());

  if (this->defiPartition::hasFallMin())
    fprintf(f, "  FALL MIN %g\n", this->defiPartition::fallMin());

  if (this->defiPartition::hasRiseMax())
    fprintf(f, "  RISE MAX %g\n", this->defiPartition::riseMax());

  if (this->defiPartition::hasFallMax())
    fprintf(f, "  FALL MAX %g\n", this->defiPartition::fallMax());

  if (this->defiPartition::hasFallMinRange())
    fprintf(f, "  FALL MIN %g %g\n", this->defiPartition::fallMinLeft(),
                                  this->defiPartition::fallMinRight());

  if (this->defiPartition::hasRiseMinRange())
    fprintf(f, "  RISE MIN %g %g\n", this->defiPartition::riseMinLeft(),
                                  this->defiPartition::riseMinRight());

  if (this->defiPartition::hasFallMaxRange())
    fprintf(f, "  FALL MAX %g %g\n", this->defiPartition::fallMaxLeft(),
                                  this->defiPartition::fallMaxRight());

  if (this->defiPartition::hasRiseMaxRange())
    fprintf(f, "  RISE MAX %g %g\n", this->defiPartition::riseMaxLeft(),
                                  this->defiPartition::riseMaxRight());
}


const char* defiPartition::name() const {
  return this->name_;
}


void defiPartition::setFromIOPin(const char* pin) {
  this->defiPartition::set('F', 'I', "", pin);
}


char defiPartition::direction() const {
  return this->direction_;
}


const char* defiPartition::instName() const {
  return this->inst_;
}


const char* defiPartition::pinName() const {
  return this->pin_;
}


static char* ad(const char* in) {
  return (char*)in;
}


const char* defiPartition::itemType() const {
  char* c;
  if (this->type_ == 'L') c = ad("CLOCK");
  else if (this->type_ == 'I') c = ad("IO");
  else if (this->type_ == 'C') c = ad("COMP");
  else c = ad("BOGUS");
  return c;
}


const char* defiPartition::pin(int index) const {
  return this->pins_[index];
}


int defiPartition::numPins() const {
  return this->numPins_;
}


int defiPartition::isSetupRise() const {
  return this->setup_ == 'R' ? 1 : 0 ;
}


int defiPartition::isSetupFall() const {
  return this->setup_ == 'F' ? 1 : 0 ;
}


int defiPartition::isHoldRise() const {
  return this->hold_ == 'R' ? 1 : 0 ;
}


int defiPartition::isHoldFall() const {
  return this->hold_ == 'F' ? 1 : 0 ;
}


void defiPartition::addTurnOff(const char* setup, const char* hold) {
  if (*setup == ' ') {
    this->setup_ = *setup;
  } else if (*setup == 'R') {
    this->setup_ = *setup;
  } else if (*setup == 'F') {
    this->setup_ = *setup;
  } else {
    defiError("Illegal setup for parition");
  }

  if (*hold == ' ') {
    this->hold_ = *hold;
  } else if (*hold == 'R') {
    this->hold_ = *hold;
  } else if (*hold == 'F') {
    this->hold_ = *hold;
  } else {
    defiError("Illegal hold for parition");
  }

}


void defiPartition::setFromClockPin(const char* inst, const char* pin) {
  this->defiPartition::set('F', 'L', inst, pin);
}


void defiPartition::setToClockPin(const char* inst, const char* pin) {
  this->defiPartition::set('T', 'L', inst, pin);
}


void defiPartition::set(char dir, char typ, const char* inst, const char* pin) {
  int len = strlen(pin) + 1;
  this->direction_ = dir;
  this->type_ = typ;

  if (this->pinLength_ <= len) {
    if (this->pin_) defFree(this->pin_);
    this->pin_ = (char*)defMalloc(len);
    this->pinLength_ = len;
  }

  strcpy(this->pin_, DEFCASE(pin));

  len = strlen(inst) + 1;
  if (this->instLength_ <= len) {
    if (this->inst_) defFree(this->inst_);
    this->inst_ = (char*)defMalloc(len);
    this->instLength_ = len;
  }

  strcpy(this->inst_, DEFCASE(inst));
}


void defiPartition::setMin(double min, double max) {
  this->min_ = min;
  this->max_ = max;
  this->hasMin_ = 1;
}


void defiPartition::setFromCompPin(const char* inst, const char* pin) {
  this->defiPartition::set('F', 'C', inst, pin);
}


void defiPartition::setMax(double min, double max) {
  this->min_ = min;
  this->max_ = max;
  this->hasMax_ = 1;
}


void defiPartition::setToIOPin(const char* pin) {
  this->defiPartition::set('T', 'I', "", pin);
}


void defiPartition::setToCompPin(const char* inst, const char* pin) {
  this->defiPartition::set('T', 'C', inst, pin);
}


void defiPartition::addPin(const char* name) {
  int len;
  int i;
  char** newp;

  if (this->numPins_ >= this->pinsAllocated_) {
    this->pinsAllocated_ = this->pinsAllocated_ ? 2 * this->pinsAllocated_ : 8;
    newp = (char**) defMalloc(sizeof(char*) * this->pinsAllocated_);
    for (i = 0; i < this->numPins_; i++)
      newp[i] = this->pins_[i];
    if (this->pins_) defFree((char*)(this->pins_));
    this->pins_ = newp;
  }

  len = strlen(name) + 1;
  this->pins_[this->numPins_] = (char*)defMalloc(len);
  strcpy(this->pins_[this->numPins_], DEFCASE(name));
  this->numPins_ += 1;
}


int defiPartition::hasMin() const {
  return(int)(this->hasMin_);
}


int defiPartition::hasMax() const {
  return(int)(this->hasMax_);
}


double defiPartition::partitionMin() const {
  return(this->min_);
}


double defiPartition::partitionMax() const {
  return(this->max_);
}

int defiPartition::hasRiseMin() const {
  return (int)(this->hasRiseMin_);
}


int defiPartition::hasFallMin() const {
  return (int)(this->hasFallMin_);
}


int defiPartition::hasRiseMax() const {
  return (int)(this->hasRiseMax_);
}


int defiPartition::hasFallMax() const {
  return (int)(this->hasFallMax_);
}


int defiPartition::hasRiseMinRange() const {
  return (int)(this->hasRiseMinRange_);
}


int defiPartition::hasFallMinRange() const {
  return (int)(this->hasFallMinRange_);
}


int defiPartition::hasRiseMaxRange() const {
  return (int)(this->hasRiseMaxRange_);
}


int defiPartition::hasFallMaxRange() const {
  return (int)(this->hasFallMaxRange_);
}


double defiPartition::riseMin() const {
  return this->riseMin_;
}


double defiPartition::fallMin() const {
  return this->fallMin_;
}


double defiPartition::riseMax() const {
  return this->riseMax_;
}


double defiPartition::fallMax() const {
  return this->fallMax_;
}


double defiPartition::riseMinLeft() const {
  return this->riseMinLeft_;
}


double defiPartition::fallMinLeft() const {
  return this->fallMinLeft_;
}


double defiPartition::riseMaxLeft() const {
  return this->riseMaxLeft_;
}


double defiPartition::fallMaxLeft() const {
  return this->fallMaxLeft_;
}


double defiPartition::riseMinRight() const {
  return this->riseMinRight_;
}


double defiPartition::fallMinRight() const {
  return this->fallMinRight_;
}


double defiPartition::riseMaxRight() const {
  return this->riseMaxRight_;
}


double defiPartition::fallMaxRight() const {
  return this->fallMaxRight_;
}


void defiPartition::addRiseMin(double d) {
  this->hasRiseMin_ = 1;
  this->riseMin_ = d;
}


void defiPartition::addRiseMax(double d) {
  this->hasRiseMax_ = 1;
  this->riseMax_ = d;
}


void defiPartition::addFallMin(double d) {
  this->hasFallMin_ = 1;
  this->fallMin_ = d;
}


void defiPartition::addFallMax(double d) {
  this->hasFallMax_ = 1;
  this->fallMax_ = d;
}


void defiPartition::addRiseMinRange(double l, double h) {
  this->hasRiseMinRange_ = 1;
  this->riseMinLeft_ = l;
  this->riseMinRight_ = h;
}


void defiPartition::addRiseMaxRange(double l, double h) {
  this->hasRiseMaxRange_ = 1;
  this->riseMaxLeft_ = l;
  this->riseMaxRight_ = h;
}


void defiPartition::addFallMinRange(double l, double h) {
  this->hasFallMinRange_ = 1;
  this->fallMinLeft_ = l;
  this->fallMinRight_ = h;
}


void defiPartition::addFallMaxRange(double l, double h) {
  this->hasFallMaxRange_ = 1;
  this->fallMaxLeft_ = l;
  this->fallMaxRight_ = h;
}


