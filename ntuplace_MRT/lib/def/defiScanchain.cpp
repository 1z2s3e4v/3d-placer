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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lex.h"
#include "defiScanchain.hpp"
#include "defiDebug.hpp"


void defiOrdered::clear() {
  int i;
  for (i = 0; i < this->num_; i++) {
    defFree((char*)(this->inst_));
    if (this->in_) defFree((char*)(this->in_));
    if (this->out_) defFree((char*)(this->out_));
  }
  this->num_ = 0;
}


void defiOrdered::Destroy() {
  int i;
  for (i = 0; i < this->num_; i++) {
     defFree((char*)(this->inst_[i]));
     defFree((char*)(this->in_[i]));
     defFree((char*)(this->out_[i]));
  }
  defFree((char*)(this->inst_));
  defFree((char*)(this->in_));
  defFree((char*)(this->out_));
  defFree((char*)(this->bits_));
}


void defiOrdered::Init() {
  this->num_ = 0;
  this->allocated_ = 32;
  this->inst_ = (char**)defMalloc(sizeof(char*)*32);
  this->in_   = (char**)defMalloc(sizeof(char*)*32);
  this->out_  = (char**)defMalloc(sizeof(char*)*32);
  this->bits_ = (int*)defMalloc(sizeof(int)*32);
}


void defiOrdered::bump() {
  int max = this->allocated_ * 2;
  int i;
  char** nin = (char**)defMalloc(sizeof(char*)*max);
  char** nout = (char**)defMalloc(sizeof(char*)*max);
  char** ninst = (char**)defMalloc(sizeof(char*)*max);
  int*   nbits = (int*)defMalloc(sizeof(int)*max);

  for (i = 0; i < this->num_; i++) {
    nin[i]  = this->in_[i];
    nout[i]  = this->out_[i];
    ninst[i] = this->inst_[i];
    nbits[i] = this->bits_[i];
  }
  defFree((char*)(this->inst_));
  defFree((char*)(this->in_));
  defFree((char*)(this->out_));
  defFree((char*)(this->bits_));
  this->allocated_ = max;
  this->inst_ = ninst;
  this->in_ = nin;
  this->out_ = nout;
  this->bits_ = nbits;
}


void defiOrdered::addOrdered(const char* inst) {
  if (this->num_ == this->allocated_) this->defiOrdered::bump();
  this->inst_[this->num_] = (char*)defMalloc(strlen(inst)+1);
  strcpy(this->inst_[this->num_], DEFCASE(inst));
  this->in_[this->num_]   = 0;
  this->out_[this->num_]  = 0;
  this->bits_[this->num_] = -1;
  this->num_ += 1;
}


void defiOrdered::addIn(const char* pin) {
  this->in_[this->num_-1] = (char*)defMalloc(strlen(pin)+1);
  strcpy(this->in_[this->num_-1], DEFCASE(pin));
}


void defiOrdered::addOut(const char* pin) {
  this->out_[this->num_-1] = (char*)defMalloc(strlen(pin)+1);
  strcpy(this->out_[this->num_-1], DEFCASE(pin));
}


void defiOrdered::setOrderedBits(int bits) {
  this->bits_[this->num_-1] = bits;
}


int defiOrdered::num() const {
  return this->num_;
}


char** defiOrdered::inst() const {
  return this->inst_;
}


char** defiOrdered::in() const {
  return this->in_;
}


char** defiOrdered::out() const {
  return this->out_;
}


int* defiOrdered::bits() const {
  return this->bits_;
}


defiScanchain::defiScanchain() {
  this->defiScanchain::Init();
}


void defiScanchain::Init() {
  this->name_ = (char*)defMalloc(32);
  this->nameLength_ = 32;

  this->numOrdered_ = 0;
  this->numOrderedAllocated_ = 4;
  this->ordered_ = (defiOrdered**)defMalloc(sizeof(defiOrdered*)*4);

  this->numFloating_ = 0;
  this->numFloatingAllocated_ = 4;
  this->floatInst_ = (char**)defMalloc(sizeof(char*)*4);
  this->floatIn_   = (char**)defMalloc(sizeof(char*)*4);
  this->floatOut_  = (char**)defMalloc(sizeof(char*)*4);
  this->floatBits_ = (int*)defMalloc(sizeof(int)*4);

  this->stopInst_     = 0;
  this->stopPin_      = 0;
  this->startInst_    = 0;
  this->startPin_     = 0;
  this->hasStart_     = 0;
  this->hasStop_      = 0;
  this->commonInPin_  = 0;
  this->commonOutPin_ = 0;
  this->hasPartition_ = 0;
  this->partName_     = 0;
  this->maxBits_      = -1;
}


void defiScanchain::clear() {
  int i;
  defiOrdered* o;

  for (i = 0; i < this->numOrdered_; i++) {
    o = this->ordered_[i];
    o->defiOrdered::Destroy();
    defFree((char*)o);
    this->ordered_[i] = 0;
  }
  this->numOrdered_ = 0;

  for (i = 0; i < this->numFloating_; i++) {
    if (this->floatIn_[i]) defFree(this->floatIn_[i]);
    if (this->floatOut_[i]) defFree(this->floatOut_[i]);
    defFree(this->floatInst_[i]);
    this->floatInst_[i] = 0;
    this->floatBits_[i] = -1;
  }
  this->numFloating_ = 0;

  if (this->stopInst_) defFree(this->stopInst_);
  if (this->stopPin_) defFree(this->stopPin_);
  if (this->startInst_) defFree(this->startInst_);
  if (this->startPin_) defFree(this->startPin_);
  if (this->commonInPin_) defFree(this->commonInPin_);
  if (this->commonOutPin_) defFree(this->commonOutPin_);

  this->stopInst_ = 0;
  this->stopPin_ = 0;
  this->startInst_ = 0;
  this->startPin_ = 0;
  this->hasStart_ = 0;
  this->hasStop_ = 0;
  this->commonInPin_ = 0;
  this->commonOutPin_ = 0;
  this->hasPartition_ = 0;
  if (this->partName_)
    defFree((char*)(this->partName_));
  this->partName_ = 0;
  this->maxBits_  = -1; 
}


void defiScanchain::Destroy() {
  this->defiScanchain::clear();
  defFree(this->name_);
  defFree((char*)(this->ordered_));
  defFree((char*)(this->floatInst_));
  defFree((char*)(this->floatIn_));
  defFree((char*)(this->floatOut_));
  defFree((char*)(this->floatBits_));
}


defiScanchain::~defiScanchain() {
  this->defiScanchain::Destroy();
}


void defiScanchain::setName(const char* name) {
  int len = strlen(name) + 1;

  this->defiScanchain::clear();

  if (len > this->nameLength_) {
    defFree(this->name_);
    this->name_ = (char*)defMalloc(len);
    this->nameLength_ = len;
  }
  strcpy(this->name_, DEFCASE(name));
}


const char* defiScanchain::name() const {
  return this->name_;
}


void defiScanchain::addFloatingInst(const char* name) {

  if (this->numFloating_ >= this->numFloatingAllocated_) {
    int max = 2 * this->numFloatingAllocated_;
    int i;
    char** ninst = (char**)defMalloc(sizeof(char*)*max);
    char** nin = (char**)defMalloc(sizeof(char*)*max);
    char** nout = (char**)defMalloc(sizeof(char*)*max);
    int*   nbits = (int*)defMalloc(sizeof(int)*max);
    for (i = 0; i < this->numFloating_; i++) {
      ninst[i] = this->floatInst_[i];
      nin[i] = this->floatIn_[i];
      nout[i] = this->floatOut_[i];
      nbits[i] = this->floatBits_[i];
    }
    defFree((char*)(this->floatInst_));
    defFree((char*)(this->floatIn_));
    defFree((char*)(this->floatOut_));
    defFree((char*)(this->floatBits_));
    this->floatInst_ = ninst;
    this->floatOut_ = nout;
    this->floatIn_ = nin;
    this->floatBits_ = nbits;
    this->numFloatingAllocated_ = max;
  }

  this->floatInst_[this->numFloating_] =
     (char*)defMalloc(strlen(name) + 1);
  strcpy(this->floatInst_[this->numFloating_], DEFCASE(name));
  this->floatIn_[this->numFloating_] = 0;
  this->floatOut_[this->numFloating_] = 0;
  this->floatBits_[this->numFloating_] = -1;
  this->numFloating_ += 1;
}


void defiScanchain::addFloatingIn(const char* name) {
  int len = strlen(name) + 1;
  this->floatIn_[this->numFloating_-1] = (char*)defMalloc(len);
  strcpy(this->floatIn_[this->numFloating_-1], DEFCASE(name));
}


void defiScanchain::addFloatingOut(const char* name) {
  int len = strlen(name) + 1;
  this->floatOut_[this->numFloating_-1] = (char*)defMalloc(len);
  strcpy(this->floatOut_[this->numFloating_-1], DEFCASE(name));
}


void defiScanchain::setFloatingBits(int bits) {
  this->floatBits_[this->numFloating_-1] = bits;
}


void defiScanchain::addOrderedIn(const char* name) {
  defiOrdered* o = this->ordered_[this->numOrdered_-1];
  o->defiOrdered::addIn(name);
}


void defiScanchain::addOrderedOut(const char* name) {
  defiOrdered* o = this->ordered_[this->numOrdered_-1];
  o->defiOrdered::addOut(name);
}


void defiScanchain::addOrderedInst(const char* name) {
  defiOrdered* o = this->ordered_[this->numOrdered_-1];
  o->defiOrdered::addOrdered(name);
}


void defiScanchain::setOrderedBits(int bits) {
  defiOrdered* o = this->ordered_[this->numOrdered_-1];
  o->defiOrdered::setOrderedBits(bits);
}


void defiScanchain::addOrderedList() {
  defiOrdered* o;

  if (this->numOrdered_ == this->numOrderedAllocated_) {
    int max = 2 * this->numOrderedAllocated_;
    int i;
    defiOrdered** no = (defiOrdered**)defMalloc(sizeof(defiOrdered*)*max);
    for (i = 0; i < this->numOrdered_; i++) {
      no[i] = this->ordered_[i];
    }
    defFree((char*)(this->ordered_));
    this->ordered_ = no;
    this->numOrderedAllocated_ = max;
  }

  o = (defiOrdered*)defMalloc(sizeof(defiOrdered));
  this->ordered_[this->numOrdered_] = o;
  o->defiOrdered::Init();
  this->numOrdered_ += 1;
}


void defiScanchain::setStart(const char* inst, const char* pin) {
   int len;
   if (this->startInst_) defiError("duplicate start in scan chain");
   len = strlen(inst) + 1;
   this->startInst_ = (char*)defMalloc(len);
   strcpy(this->startInst_, DEFCASE(inst));
   len = strlen(pin) + 1;
   this->startPin_ = (char*)defMalloc(len);
   strcpy(this->startPin_, DEFCASE(pin));
   this->hasStart_ = 1;
}


void defiScanchain::setStop(const char* inst, const char* pin) {
   int len;
   if (this->stopInst_) defiError("duplicate stop in scan chain");
   len = strlen(inst) + 1;
   this->stopInst_ = (char*)defMalloc(len);
   strcpy(this->stopInst_, DEFCASE(inst));
   len = strlen(pin) + 1;
   this->stopPin_ = (char*)defMalloc(len);
   strcpy(this->stopPin_, DEFCASE(pin));
   this->hasStop_ = 1;
}


// 5.4.1
void defiScanchain::setPartition(const char* partName, int maxBits) {
  if (this->partName_)
     defFree(this->partName_);
  this->partName_ = (char*)defMalloc(strlen(partName) + 1);
  strcpy(this->partName_, DEFCASE(partName));
  this->maxBits_ = maxBits;
  this->hasPartition_ = 1;
}


int defiScanchain::hasStart() const {
  return (int)(this->hasStart_);
}


int defiScanchain::hasStop() const {
  return (int)(this->hasStop_);
}


int defiScanchain::hasFloating() const {
  return this->numFloating_ ? 1 : 0 ;
}


int defiScanchain::hasOrdered() const {
  return this->numOrdered_ ? 1 : 0 ;
}


// 5.4.1
int defiScanchain::hasPartition() const {
  return this->hasPartition_;
}


// 5.4.1
int defiScanchain::hasPartitionMaxBits() const {
  return (this->maxBits_ != -1) ? 1 : 0;
}


// 5.4.1
const char* defiScanchain::partitionName() const {
  return this->partName_;
}


// 5.4.1
int defiScanchain::partitionMaxBits() const {
  return this->maxBits_;
}


void defiScanchain::start(char** inst, char** pin) const {
  if (inst) *inst = this->startInst_;
  if (pin) *pin = this->startPin_;
}


void defiScanchain::stop(char** inst, char** pin) const {
  if (inst) *inst = this->stopInst_;
  if (pin) *pin = this->stopPin_;
}


int defiScanchain::numOrderedLists() const {
  return this->numOrdered_;
}


void defiScanchain::ordered(int index, int* size, char*** inst,
                        char*** inPin, char*** outPin, int** bits) const {
  defiOrdered* o;
  if (index >= 0 && index <= this->numOrdered_) {
    o = this->ordered_[index];
    *size   = o->defiOrdered::num();
    *inst   = o->defiOrdered::inst();
    *inPin  = o->defiOrdered::in();
    *outPin = o->defiOrdered::out();
    *bits   = o->defiOrdered::bits();
  } else {
    *size = 10;
    *inst = 0;
  }
}


void defiScanchain::floating(int* size, char*** inst,
                 char*** inPin, char*** outPin, int** bits) const {
  *size   = this->numFloating_;
  *inst   = this->floatInst_;
  *inPin  = this->floatIn_;
  *outPin = this->floatOut_;
  *bits   = this->floatBits_;
}


void defiScanchain::setCommonOut(const char* pin) {
  int len = strlen(pin) + 1;
  this->commonOutPin_ = (char*)defMalloc(len);
  strcpy(this->commonOutPin_, DEFCASE(pin));
}


void defiScanchain::setCommonIn(const char* pin) {
  int len = strlen(pin) + 1;
  this->commonInPin_ = (char*)defMalloc(len);
  strcpy(this->commonInPin_, DEFCASE(pin));
}


int defiScanchain::hasCommonInPin() const {
  return this->commonInPin_ ? 1 : 0;
}


int defiScanchain::hasCommonOutPin() const {
  return this->commonOutPin_ ? 1 : 0;
}


const char* defiScanchain::commonInPin() const {
  return this->commonInPin_;
}


const char* defiScanchain::commonOutPin() const {
  return this->commonOutPin_;
}


void defiScanchain::print(FILE* f) const {
  char* a;
  char* b;

  char** inst;
  char** in;
  char** out;
  int*   bits;
  int num;
  int i;
  int h;

  fprintf(f, "Scan chain '%s'  %d things\n",
      this->defiScanchain::name(),
      this->defiScanchain::hasStart() +
      this->defiScanchain::hasStop() +
      this->numFloating_ +
      this->defiScanchain::numOrderedLists());

  if (this->defiScanchain::hasStart()) {
    this->defiScanchain::start(&a, &b);
    fprintf(f, "  start inst '%s' pin '%s'\n", a, b);
  }

  if (this->defiScanchain::hasStop()) {
    this->defiScanchain::stop(&a, &b);
    fprintf(f, "  stop inst '%s' pin '%s'\n", a, b);
  }

  this->defiScanchain::floating(&num, &inst, &in, &out, &bits);
  for (i = 0; i < num; i++) {
    fprintf(f, "  floating '%s' IN=%s  OUT=%s BITS=%d\n", inst[i],
       in[i]?in[i]:"NIL", out[i]?out[i]:"NIL", bits[i]?bits[i]:0);
  }

  for (i = 0; i < this->defiScanchain::numOrderedLists(); i++) {
    this->defiScanchain::ordered(i, &num, &inst, &in, &out, &bits);
    fprintf(f, "  %d ordered\n", i);
    for (h = 0; h < num; h++) {
      fprintf(f, "        '%s' IN=%s OUT=%s BITS=%d\n", inst[h],
       in[h]?in[h]:"NIL", out[h]?out[h]:"NIL", bits[h]?bits[h]:0);
    }
  }

  if (this->defiScanchain::hasCommonInPin())
    fprintf(f, "  common in pin %s\n", this->defiScanchain::commonInPin());

  if (this->defiScanchain::hasCommonOutPin())
    fprintf(f, "  common out pin %s\n", this->defiScanchain::commonOutPin());
}


