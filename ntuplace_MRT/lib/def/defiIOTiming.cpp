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
#include "defiIOTiming.hpp"
#include "defiDebug.hpp"


//////////////////////////////////////////////
//////////////////////////////////////////////
//
//   defiIOTiming
//
//////////////////////////////////////////////
//////////////////////////////////////////////


defiIOTiming::defiIOTiming() {
  this->defiIOTiming::Init();
}


void defiIOTiming::Init() {
  this->inst_ = 0;
  this->instLength_ = 0;
  this->pin_ = 0;
  this->pinLength_ = 0;
  this->from_ = 0;
  this->fromLength_ = 0;
  this->to_ = 0;
  this->toLength_ = 0;
  this->driveCell_ = 0;
  this->driveCellLength_ = 0;
  this->hasVariableRise_ = 0;
  this->hasVariableFall_ = 0;
  this->hasSlewRise_ = 0;
  this->hasSlewFall_ = 0;
  this->hasCapacitance_ = 0;
  this->hasDriveCell_ = 0;
  this->hasFrom_ = 0;
  if (this->pin_) defFree(this->pin_);
  this->pin_ = 0;
  this->pinLength_ = 0;

  this->hasTo_ = 0;
  this->hasParallel_ = 0;
  this->variableFallMin_ = 0.0;
  this->variableRiseMin_ = 0.0;
  this->variableFallMax_ = 0.0;
  this->variableRiseMax_ = 0.0;
  this->slewFallMin_ = 0.0;
  this->slewRiseMin_ = 0.0;
  this->slewFallMax_ = 0.0;
  this->slewRiseMax_ = 0.0;
  this->capacitance_ = 0.0;
  this->parallel_ = 0.0;
}


defiIOTiming::~defiIOTiming() {
  this->defiIOTiming::Destroy();
}


void defiIOTiming::Destroy() {

  if (this->inst_) defFree(this->inst_);
  this->inst_ = 0;
  this->instLength_ = 0;

  if (this->pin_) defFree(this->pin_);
  this->pin_ = 0;
  this->pinLength_ = 0;

  if (this->from_) defFree(this->from_);
  this->from_ = 0;
  this->fromLength_ = 0;

  if (this->to_) defFree(this->to_);
  this->to_ = 0;
  this->toLength_ = 0;

  if (this->driveCell_) defFree(this->driveCell_);
  this->driveCell_ = 0;
  this->driveCellLength_ = 0;

  this->defiIOTiming::clear();
}


void defiIOTiming::clear() {
  this->hasVariableRise_ = 0;
  this->hasVariableFall_ = 0;
  this->hasSlewRise_ = 0;
  this->hasSlewFall_ = 0;
  this->hasCapacitance_ = 0;
  this->hasDriveCell_ = 0;
  this->hasFrom_ = 0;
  this->hasTo_ = 0;
  this->hasParallel_ = 0;
  this->variableFallMin_ = 0.0;
  this->variableRiseMin_ = 0.0;
  this->variableFallMax_ = 0.0;
  this->variableRiseMax_ = 0.0;
  this->slewFallMin_ = 0.0;
  this->slewRiseMin_ = 0.0;
  this->slewFallMax_ = 0.0;
  this->slewRiseMax_ = 0.0;
  this->capacitance_ = 0.0;
  this->parallel_ = 0.0;
}


void defiIOTiming::setName(const char* inst, const char* pin) {
  int len;

  this->defiIOTiming::clear();

  len = strlen(inst) + 1;
  if (len > this->instLength_) {
    if (this->inst_) defFree(this->inst_);
    this->instLength_ = len;
    this->inst_ = (char*)defMalloc(len);
  }
  strcpy(this->inst_, DEFCASE(inst));

  len = strlen(pin) + 1;
  if (len > this->pinLength_) {
    if (this->pin_) defFree(this->pin_);
    this->pinLength_ = len;
    this->pin_ = (char*)defMalloc(len);
  }
  strcpy(this->pin_, DEFCASE(pin));

}


void defiIOTiming::print(FILE* f) const {
  fprintf(f, "IOTiming '%s' '%s'\n", this->inst_, this->pin_);

  if (this->defiIOTiming::hasSlewRise())
    fprintf(f, "  Slew rise  %5.2f %5.2f\n",
       this->defiIOTiming::slewRiseMin(),
       this->defiIOTiming::slewRiseMax());

  if (this->defiIOTiming::hasSlewFall())
    fprintf(f, "  Slew fall  %5.2f %5.2f\n",
       this->defiIOTiming::slewFallMin(),
       this->defiIOTiming::slewFallMax());

  if (this->defiIOTiming::hasVariableRise())
    fprintf(f, "  variable rise  %5.2f %5.2f\n",
       this->defiIOTiming::variableRiseMin(),
       this->defiIOTiming::variableRiseMax());

  if (this->defiIOTiming::hasVariableFall())
    fprintf(f, "  variable fall  %5.2f %5.2f\n",
       this->defiIOTiming::variableFallMin(),
       this->defiIOTiming::variableFallMax());

  if (this->defiIOTiming::hasCapacitance())
    fprintf(f, "  capacitance %5.2f\n",
       this->defiIOTiming::capacitance());

  if (this->defiIOTiming::hasDriveCell())
    fprintf(f, "  drive cell '%s'\n",
       this->defiIOTiming::driveCell());

  if (this->defiIOTiming::hasFrom())
    fprintf(f, "  from pin '%s'\n",
       this->defiIOTiming::from());

  if (this->defiIOTiming::hasTo())
    fprintf(f, "  to pin '%s'\n",
       this->defiIOTiming::to());

  if (this->defiIOTiming::hasParallel())
    fprintf(f, "  parallel %5.2f\n",
       this->defiIOTiming::parallel());
}



void defiIOTiming::setVariable(const char* riseFall, double min, double max) {
  if (*riseFall == 'R') {
    this->hasVariableRise_ = 1;
    this->variableRiseMin_ = min;
    this->variableRiseMax_ = max;

  } else if (*riseFall == 'F') {
    this->hasVariableFall_ = 1;
    this->variableFallMin_ = min;
    this->variableFallMax_ = max;

  } else {
    defiError("Internal error Bad varaible rate rise/fall status");
  }
}


void defiIOTiming::setSlewRate(const char* riseFall, double min, double max) {
  if (*riseFall == 'R') {
    this->hasSlewRise_ = 1;
    this->slewRiseMin_ = min;
    this->slewRiseMax_ = max;

  } else if (*riseFall == 'F') {
    this->hasSlewFall_ = 1;
    this->slewFallMin_ = min;
    this->slewFallMax_ = max;

  } else {
    defiError("Internal error Bad slew rate rise/fall status");
  }
}


void defiIOTiming::setCapacitance(double num) {
  this->hasCapacitance_ = 1;
  this->capacitance_ = num;
}


void defiIOTiming::setDriveCell(const char* name) {
  int len = strlen(name) + 1;

  if (this->driveCellLength_ < len) {
    if (this->driveCell_) defFree(this->driveCell_);
    this->driveCell_ = (char*) defMalloc(len);
    this->driveCellLength_ = len;
  }

  strcpy(this->driveCell_, DEFCASE(name));
  this->hasDriveCell_ = 1;
}


void defiIOTiming::setFrom(const char* name) {
  int len = strlen(name) + 1;

  if (this->fromLength_ < len) {
    if (this->from_) defFree(this->from_);
    this->from_ = (char*) defMalloc(len);
    this->fromLength_ = len;
  }

  strcpy(this->from_, DEFCASE(name));
  this->hasFrom_ = 1;
}


void defiIOTiming::setTo(const char* name) {
  int len = strlen(name) + 1;

  if (this->toLength_ < len) {
    if (this->to_) defFree(this->to_);
    this->to_ = (char*) defMalloc(len);
    this->toLength_ = len;
  }

  strcpy(this->to_, DEFCASE(name));
  this->hasTo_ = 1;
}


void defiIOTiming::setParallel(double num) {
  this->hasParallel_ = 1;
  this->parallel_ = num;
}


int defiIOTiming::hasVariableRise() const {
  return this->hasVariableRise_;
}


int defiIOTiming::hasVariableFall() const {
  return this->hasVariableFall_;
}


int defiIOTiming::hasSlewRise() const {
  return this->hasSlewRise_;
}


int defiIOTiming::hasSlewFall() const {
  return this->hasSlewFall_;
}


int defiIOTiming::hasCapacitance() const {
  return this->hasCapacitance_;
}


int defiIOTiming::hasDriveCell() const {
  return this->hasDriveCell_;
}


int defiIOTiming::hasFrom() const {
  return this->hasFrom_;
}


int defiIOTiming::hasTo() const {
  return this->hasTo_;
}


int defiIOTiming::hasParallel() const {
  return this->hasParallel_;
}


const char* defiIOTiming::inst() const {
  return this->inst_;
}


const char* defiIOTiming::pin() const {
  return this->pin_;
}


double defiIOTiming::variableFallMin() const {
  return this->variableFallMin_;
}


double defiIOTiming::variableRiseMin() const {
  return this->variableRiseMin_;
}


double defiIOTiming::variableFallMax() const {
  return this->variableFallMax_;
}


double defiIOTiming::variableRiseMax() const {
  return this->variableRiseMax_;
}


double defiIOTiming::slewFallMin() const {
  return this->slewFallMin_;
}


double defiIOTiming::slewRiseMin() const {
  return this->slewRiseMin_;
}


double defiIOTiming::slewFallMax() const {
  return this->slewFallMax_;
}


double defiIOTiming::slewRiseMax() const {
  return this->slewRiseMax_;
}


double defiIOTiming::capacitance() const {
  return this->capacitance_;
}


const char* defiIOTiming::driveCell() const {
  return this->driveCell_;
}


const char* defiIOTiming::from() const {
  return this->from_;
}


const char* defiIOTiming::to() const {
  return this->to_;
}


double defiIOTiming::parallel() const {
  return this->parallel_;
}


