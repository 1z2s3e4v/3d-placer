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
#include <stdlib.h>
#include "defiAlias.hpp"

////////////////////////////////////////////////////
////////////////////////////////////////////////////
//
//    defiAlias_itr
//
////////////////////////////////////////////////////
////////////////////////////////////////////////////

/* From FEF_stringhash.cpp */
extern const char* defAliasKey(int num);
extern const char* defAliasData(int num);
extern int defAliasMarked(int num);
extern int defAliasLimit();


defiAlias_itr::defiAlias_itr() {
  this->defiAlias_itr::Init();
}


void defiAlias_itr::Init() {
  this->num_ = -1;
}
 

void defiAlias_itr::Destroy() {
}


defiAlias_itr::~defiAlias_itr() {
  this->defiAlias_itr::Destroy();
}

 
int defiAlias_itr::Next() {
  while (1) {
    this->num_ += 1;
    if (this->num_ <= -1 || this->num_ >= defAliasLimit()) return 0;
    if (defAliasKey(this->num_)) break;
  }
  return 1;
}


const char* defiAlias_itr::Key() {
  if (this->num_ <= -1 || this->num_ >= defAliasLimit()) return 0;
  return defAliasKey(this->num_);
}


const char* defiAlias_itr::Data() {
  if (this->num_ <= -1 || this->num_ >= defAliasLimit()) return 0;
  return defAliasData(this->num_);
}

 
int defiAlias_itr::Marked() {
  if (this->num_ <= -1 || this->num_ >= defAliasLimit()) return 0;
  return defAliasMarked(this->num_);
}
