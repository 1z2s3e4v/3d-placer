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
#include "defiAssertion.hpp"
#include "defiDebug.hpp"



struct defiAssertPath {
  char* fromPin_;
  char* toPin_;
  char* fromInst_;
  char* toInst_;
};



defiAssertion::defiAssertion() {
  this->defiAssertion::Init();
}


defiAssertion::~defiAssertion() {
  this->defiAssertion::Destroy();
}


void defiAssertion::Init() {
  this->netName_ = (char*)defMalloc(32);
  this->netNameLength_ = 32;
  this->defiAssertion::clear();
  this->numItemsAllocated_ = 16;
  this->items_ = (int**)defMalloc(sizeof(char*)*16);
  this->itemTypes_ = (char*)defMalloc(16);
}


void defiAssertion::Destroy() {
  defFree(this->netName_);
  defFree((char*)(this->itemTypes_));
  defFree((char*)(this->items_));
}


void defiAssertion::clear() {
  int i;
  struct defiAssertPath* s;

  if (this->netName_)
     *(this->netName_) = '\0';
  this->isSum_ = 0;
  this->isDiff_ = 0;
  this->isAssertion_ = 0;
  this->hasRiseMin_ = 0;
  this->hasRiseMax_ = 0;
  this->hasFallMin_ = 0;
  this->hasFallMax_ = 0;
  this->isDelay_ = 0;
  this->isWiredlogic_ = 0;

  for (i = 0; i < this->numItems_; i++) {
    if (this->itemTypes_[i] == 'p') {
      s = (struct defiAssertPath*)(this->items_[i]);
      defFree(s->fromPin_);
      defFree(s->toPin_);
      defFree(s->fromInst_);
      defFree(s->toInst_);
      defFree((char*)s);
    } else if (this->itemTypes_[i] == 'n') {
      defFree((char*)(this->items_[i]));
    } else {
      defiError("Internal error in Assertion #1");
    }
    this->itemTypes_[i] = 'B';  // bogus
    this->items_[i] = 0;
  }

  this->numItems_ = 0;
}


void defiAssertion::setConstraintMode() {
  this->isAssertion_ = 0;
}


void defiAssertion::setAssertionMode() {
  this->isAssertion_ = 1;
}


void defiAssertion::setWiredlogicMode() {
  this->isWiredlogic_ = 1;
}


void defiAssertion::setWiredlogic(const char* name, double dist) {
  int len = strlen(name) + 1;
  if (this->isDelay_)
    defiError("mixing WIREDLOGIC and DELAY in constraint/assertion");
  this->isWiredlogic_ = 1;
  if (this->netNameLength_ < len) {
    defFree(this->netName_);
    this->netName_ = (char*)defMalloc(len);
    this->netNameLength_ = len;
  }
  strcpy(this->netName_, DEFCASE(name));
  this->fallMax_ = dist;
}


void defiAssertion::setDelay() {
  if (this->isWiredlogic_)
    defiError("mixing WIREDLOGIC and DELAY in constraint/assertion");
  this->isDelay_ = 1;
}


void defiAssertion::setSum() {
  if (this->isDiff_) defiError("mixing SUM and DIFF in constraint/assertion");
  this->isSum_ = 1;
}


void defiAssertion::unsetSum() {
  this->isSum_ = 0;
}


void defiAssertion::setDiff() {
  if (this->isSum_) defiError("mixing SUM and DIFF in constraint/assertion");
  this->isDiff_ = 1;
}


const char* defiAssertion::netName() const {
  return this->netName_;
}


void defiAssertion::setNetName(const char* name) {
  int len = strlen(name) + 1;
  this->defiAssertion::clear();
  if (len > this->netNameLength_) {
    defFree(this->netName_);
    this->netName_ = (char*)defMalloc(len);
    this->netNameLength_ = len;
  }
  strcpy(this->netName_, DEFCASE(name));
}



int defiAssertion::isDelay() const {
  return this->isDelay_ ? 1 : 0;
}


int defiAssertion::isAssertion() const {
  return this->isAssertion_ ? 1 : 0;
}


int defiAssertion::isConstraint() const {
  return this->isAssertion_ ? 0 : 1;
}


int defiAssertion::isSum() const {
  return this->isSum_;
}


int defiAssertion::isDiff() const {
  return this->isDiff_;
}


int defiAssertion::isWiredlogic() const {
  return this->isWiredlogic_;
}


int defiAssertion::hasRiseMin() const {
  return this->hasRiseMin_;
}


int defiAssertion::hasRiseMax() const {
  return this->hasRiseMax_;
}


int defiAssertion::hasFallMin() const {
  return this->hasFallMin_;
}


int defiAssertion::hasFallMax() const {
  return this->hasFallMax_;
}


double defiAssertion::distance() const {
  return this->fallMax_;  // distance is stored here
}


double defiAssertion::riseMin() const {
  return this->riseMin_;
}


double defiAssertion::riseMax() const {
  return this->riseMax_;
}


double defiAssertion::fallMin() const {
  return this->fallMin_;
}


double defiAssertion::fallMax() const {
  return this->fallMax_;
}


void defiAssertion::setRiseMin(double d) {
  this->riseMin_ = d;
  this->hasRiseMin_ = 1;
}


void defiAssertion::setRiseMax(double d) {
  this->riseMax_ = d;
  this->hasRiseMax_ = 1;
}


void defiAssertion::setFallMin(double d) {
  this->fallMin_ = d;
  this->hasFallMin_ = 1;
}


void defiAssertion::setFallMax(double d) {
  this->fallMax_ = d;
  this->hasFallMax_ = 1;
}


int defiAssertion::numItems() const {
  return this->numItems_;
}


int defiAssertion::isPath(int index) const {
  if (index >= 0 && index < this->numItems_) {
    return (this->itemTypes_[index] == 'p') ? 1 : 0;
  }
  return 0;
}


int defiAssertion::isNet(int index) const {
  if (index >= 0 && index < this->numItems_) {
    return (this->itemTypes_[index] == 'n') ? 1 : 0;
  }
  return 0;
}


void defiAssertion::path(int index, char** fromInst, char** fromPin,
	   char** toInst, char** toPin) const {
  struct defiAssertPath* ap;

  if (index >= 0 && index < this->numItems_ &&
      this->itemTypes_[index] == 'p') {
    ap = (struct defiAssertPath*)(this->items_[index]);
    if (fromInst) *fromInst = ap->fromInst_;
    if (fromPin) *fromPin = ap->fromPin_;
    if (toInst) *toInst = ap->toInst_;
    if (toPin) *toPin = ap->toPin_;
  }
}


void defiAssertion::net(int index, char** netName) const {
  if (index >= 0 && index < this->numItems_ &&
      this->itemTypes_[index] == 'n') {
    if (netName) *netName = (char*)(this->items_[index]);
  }
}


void defiAssertion::bumpItems() {
  int i;
  char* newTypes;
  int** newItems;
  (this->numItemsAllocated_) *= 2;
  newTypes = (char*)defMalloc(this->numItemsAllocated_ * sizeof(char));
  newItems = (int**)defMalloc(this->numItemsAllocated_ * sizeof(int*));
  for (i = 0; i < this->numItems_; i++) {
    newItems[i] = this->items_[i];
    newTypes[i] = this->itemTypes_[i];
  }
  defFree((char*)this->items_);
  defFree((char*)this->itemTypes_);
  this->items_ = newItems;
  this->itemTypes_ = newTypes;
}


void defiAssertion::addNet(const char* name) {
  int i;
  char* s;

  // set wiredlogic to false
  this->isWiredlogic_ = 0;

  // make our own copy
  i = strlen(name) + 1;
  s = (char*)defMalloc(i);
  strcpy(s, DEFCASE(name));

  // make sure there is space in the array
  if (this->numItems_ >= this->numItemsAllocated_)
    this->defiAssertion::bumpItems();

  // place it
  i = this->numItems_;
  this->items_[i] = (int*)s;
  this->itemTypes_[i] = 'n';
  this->numItems_ = i + 1;
  //strcpy(this->itemTypes_, "n");
}


void defiAssertion::addPath(const char* fromInst, const char* fromPin,
               const char* toInst, const char* toPin) {
  int i;
  struct defiAssertPath* s;

  // set wiredlogic to false
  this->isWiredlogic_ = 0;

  // make our own copy
  s = (struct defiAssertPath*)defMalloc(sizeof(struct defiAssertPath));
  i = strlen(fromInst) + 1;
  s->fromInst_ = (char*)defMalloc(i);
  strcpy(s->fromInst_, DEFCASE(fromInst));
  i = strlen(toInst) + 1;
  s->toInst_ = (char*)defMalloc(i);
  strcpy(s->toInst_, DEFCASE(toInst));
  i = strlen(fromPin) + 1;
  s->fromPin_ = (char*)defMalloc(i);
  strcpy(s->fromPin_, DEFCASE(fromPin));
  i = strlen(toPin) + 1;
  s->toPin_ = (char*)defMalloc(i);
  strcpy(s->toPin_, DEFCASE(toPin));

  // make sure there is space in the array
  if (this->numItems_ >= this->numItemsAllocated_)
    this->defiAssertion::bumpItems();

  // place it
  i = this->numItems_;
  this->items_[i] = (int*)s;
  this->itemTypes_[i] = 'p';
  this->numItems_ = i + 1;
  //strcpy(this->itemTypes_, "p");
}


void defiAssertion::print(FILE* f) const {
  fprintf(f, "Assertion %s\n", this->defiAssertion::netName());
}


