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
#include "defiComponent.hpp"
#include "defiDebug.hpp"
#include "defiUtil.hpp"


defiComponent::defiComponent() {
  this->defiComponent::Init();
}


void defiComponent::Init() {
  this->id_ = 0;
  this->name_ = 0;
  this->regionName_ = 0;
  this->foreignName_ = 0;
  this->Fori_ = 0;
  this->EEQ_ = 0;
  this->generateName_ = 0;
  this->macroName_ = 0;
  this->generateNameSize_ = 0;
  this->macroNameSize_ = 0;
  this->nets_ = 0;
  this->source_ = 0;
  this->numNets_ = 0;
  this->defiComponent::bumpName(16);
  this->defiComponent::bumpId(16);
  this->defiComponent::bumpRegionName(16);
  this->defiComponent::bumpEEQ(16);
  this->defiComponent::bumpNets(16);
  this->defiComponent::bumpForeignName(16);
  this->numProps_ = 0;
  this->propsAllocated_ = 2;
  this->names_ = (char**)defMalloc(sizeof(char*)* 2);
  this->values_ = (char**)defMalloc(sizeof(char*)* 2);
  this->dvalues_ = (double*)defMalloc(sizeof(double)* 2);
  this->types_ = (char*)defMalloc(sizeof(char)* 2);
  this->defiComponent::clear();

  this->numRects_ = 0;
  this->rectsAllocated_ = 1;
  this->rectXl_ = (int*)defMalloc(sizeof(int)*1);
  this->rectYl_ = (int*)defMalloc(sizeof(int)*1);
  this->rectXh_ = (int*)defMalloc(sizeof(int)*1);
  this->rectYh_ = (int*)defMalloc(sizeof(int)*1);
}


void defiComponent::Destroy() {
  this->defiComponent::clear();
  defFree(this->name_);
  defFree(this->regionName_);
  defFree(this->id_);
  defFree(this->EEQ_);
  defFree((char*)(this->nets_));
  if (this->source_) defFree(this->source_);
  if (this->foreignName_) defFree(this->foreignName_);
  if (this->generateName_) defFree(this->generateName_);
  if (this->macroName_) defFree(this->macroName_);
  defFree((char*)(this->names_));
  defFree((char*)(this->values_));
  defFree((char*)(this->dvalues_));
  defFree((char*)(this->types_));
  defFree((char*)(this->rectXl_));
  defFree((char*)(this->rectYl_));
  defFree((char*)(this->rectXh_));
  defFree((char*)(this->rectYh_));
}


defiComponent::~defiComponent() {
  this->defiComponent::Destroy();
}


void defiComponent::IdAndName(const char* id, const char* name) {
  int len;

  this->defiComponent::clear();

  if ((len = strlen(id)+1) > this->idSize_)
    this->defiComponent::bumpId(len);
  strcpy(this->id_, DEFCASE(id));

  if ((len = strlen(name)+1) > this->nameSize_)
    this->defiComponent::bumpName(len);
  strcpy(this->name_, DEFCASE(name));
}


const char* defiComponent::source() const {
  return this->source_;
}


int defiComponent::weight() const {
  return this->weight_;
}


void defiComponent::setWeight(int w) {
  this->weight_ = w;
  this->hasWeight_ = 1;
}


void defiComponent::setGenerate(const char* newName, const char* macroName) {
  int len = strlen(newName) + 1;

  if (this->generateNameSize_ < len) {
    if (this->generateName_) defFree(this->generateName_);
    this->generateName_ = (char*)defMalloc(len);
    this->generateNameSize_ = len;
  }
  strcpy(this->generateName_, DEFCASE(newName));

  len = strlen(macroName) + 1;
  if (this->macroNameSize_ < len) {
    if (this->macroName_) defFree(this->macroName_);
    this->macroName_ = (char*)defMalloc(len);
    this->macroNameSize_ = len;
  }
  strcpy(this->macroName_, DEFCASE(macroName));

  this->hasGenerate_ = 1;  // Ying Tan fix at 20010918
}


void defiComponent::setSource(const char* name) {
  int len = strlen(name) + 1;
  this->source_ = (char*)defMalloc(len);
  strcpy(this->source_, DEFCASE(name));
}


void defiComponent::setRegionName(const char* name) {
  int len;

  if ((len = strlen(name)+1) > this->regionNameSize_)
    this->defiComponent::bumpRegionName(len);
  strcpy(this->regionName_, DEFCASE(name));
  this->hasRegionName_ = 1;
}


void defiComponent::setEEQ(const char* name) {
  int len;

  if ((len = strlen(name)+1) > this->EEQSize_)
    this->defiComponent::bumpEEQ(len);
  strcpy(this->EEQ_, DEFCASE(name));
  this->hasEEQ_ = 1;
}


void defiComponent::setPlacementStatus(int n) {
  this->status_= n;
}


void defiComponent::setPlacementLocation(int x, int y, int orient) {
  this->x_ = x;
  this->y_ = y;
  this->orient_ = orient;
}


void defiComponent::setRegionBounds(int xl, int yl, int xh, int yh) {
  int i;
  i = this->numRects_;
  if (i == this->rectsAllocated_) {
    int max = this->rectsAllocated_ * 2;
    int* nxl = (int*)defMalloc(sizeof(int)*max);
    int* nyl = (int*)defMalloc(sizeof(int)*max);
    int* nxh = (int*)defMalloc(sizeof(int)*max);
    int* nyh = (int*)defMalloc(sizeof(int)*max);
    for (i = 0; i < this->numRects_; i++) {
      nxl[i] = this->rectXl_[i];
      nyl[i] = this->rectYl_[i];
      nxh[i] = this->rectXh_[i];
      nyh[i] = this->rectYh_[i];
    }
    defFree((char*)(this->rectXl_));
    defFree((char*)(this->rectYl_));
    defFree((char*)(this->rectXh_));
    defFree((char*)(this->rectYh_));
    this->rectXl_ = nxl;
    this->rectYl_ = nyl;
    this->rectXh_ = nxh;
    this->rectYh_ = nyh;
    this->rectsAllocated_ = max;
  }
  this->rectXl_[i] = xl;
  this->rectYl_[i] = yl;
  this->rectXh_[i] = xh;
  this->rectYh_[i] = yh;
  this->numRects_ += 1;
}


const char* defiComponent::id() const {
  return this->id_;
}


const char* defiComponent::name() const {
  return this->name_;
}


int defiComponent::placementStatus() const {
  return this->status_;
}


int defiComponent::placementX() const {
  return this->x_;
}


int defiComponent::placementY() const {
  return this->y_;
}


int defiComponent::placementOrient() const {
  return this->orient_;
}


const char* defiComponent::placementOrientStr() const {
  return (defiOrientStr(this->orient_));
}


const char* defiComponent::regionName() const {
  return this->regionName_;
}


const char* defiComponent::EEQ() const {
  return this->EEQ_;
}


const char* defiComponent::generateName() const {
  return this->generateName_;
}


const char* defiComponent::macroName() const {
  return this->macroName_;
}


void defiComponent::regionBounds(int* size,
	  int** xl, int** yl, int** xh, int** yh) {
  *size = this->numRects_;
  *xl = this->rectXl_;
  *yl = this->rectYl_;
  *xh = this->rectXh_;
  *yh = this->rectYh_;
}


void defiComponent::bumpId(int size) {
  if (this->id_) defFree(this->id_);
  this->id_ = (char*)defMalloc(size);
  this->idSize_ = size;
  *(this->id_) = '\0';
}


void defiComponent::bumpName(int size) {
  if (this->name_) defFree(this->name_);
  this->name_ = (char*)defMalloc(size);
  this->nameSize_ = size;
  *(this->name_) = '\0';
}


void defiComponent::bumpRegionName(int size) {
  if (this->regionName_) defFree(this->regionName_);
  this->regionName_ = (char*)defMalloc(size);
  this->regionNameSize_ = size;
  *(this->regionName_) = '\0';
}


void defiComponent::bumpEEQ(int size) {
  if (this->EEQ_) defFree(this->EEQ_);
  this->EEQ_ = (char*)defMalloc(size);
  this->EEQSize_ = size;
  *(this->EEQ_) = '\0';
}



void defiComponent::clear() {
  int i;

  if (this->id_)
     *(this->id_) = '\0';
  if (this->name_)
     *(this->name_) = '\0';
  if (this->regionName_)
     *(this->regionName_) = '\0';
  if (this->foreignName_)
     *(this->foreignName_) = '\0';
  if (this->EEQ_)
     *(this->EEQ_) = '\0';
  this->Fori_ = 0;
  this->status_ = 0;
  this->hasRegionName_ = 0;
  this->hasForeignName_ = 0;
  this->hasFori_ = 0;
  this->hasEEQ_ = 0;
  this->hasWeight_ = 0;
  this->hasGenerate_ = 0;
  if (this->source_) defFree(this->source_);
  for (i = 0; i < this->numNets_; i++) {
    defFree(this->nets_[i]);
  }
  this->numNets_ = 0;
  this->source_ = 0;
  for (i = 0; i < this->numProps_; i++) {
    defFree(this->names_[i]);
    defFree(this->values_[i]);
    this->dvalues_[i] = 0;
  }
  this->numProps_ = 0;
  this->numRects_ = 0;
}


int defiComponent::isUnplaced() const {
  return this->status_ == DEFI_COMPONENT_UNPLACED ? 1 : 0 ;
}


int defiComponent::isPlaced() const {
  return this->status_ == DEFI_COMPONENT_PLACED ? 1 : 0 ;
}


int defiComponent::isFixed() const {
  return this->status_ == DEFI_COMPONENT_FIXED ? 1 : 0 ;
}


int defiComponent::isCover() const {
  return this->status_ == DEFI_COMPONENT_COVER ? 1 : 0 ;
}


void defiComponent::print(FILE* fout) {
  fprintf(fout, "Component id '%s' name '%s'",
      this->defiComponent::id(),
      this->defiComponent::name());
  if (this->defiComponent::isPlaced()) {
    fprintf(fout, " Placed at %d,%d orient %s",
    this->defiComponent::placementX(),
    this->defiComponent::placementY(),
    this->defiComponent::placementOrientStr());
  }
  if (this->defiComponent::isFixed()) {
    fprintf(fout, " Fixed at %d,%d orient %s",
    this->defiComponent::placementX(),
    this->defiComponent::placementY(),
    this->defiComponent::placementOrientStr());
  }
  if (this->defiComponent::isCover()) {
    fprintf(fout, " Cover at %d,%d orient %s",
    this->defiComponent::placementX(),
    this->defiComponent::placementY(),
    this->defiComponent::placementOrientStr());
  }
  fprintf(fout, "\n");

  if (this->defiComponent::hasGenerate()) {
    fprintf(fout, "  generate %s %s\n", this->defiComponent::generateName(),
    this->defiComponent::macroName());
  }
  if (this->defiComponent::hasWeight()) {
    fprintf(fout, "  weight %d\n", this->defiComponent::weight());
  }
  if (this->defiComponent::hasSource()) {
    fprintf(fout, "  source '%s'\n", this->defiComponent::source());
  }
  if (this->defiComponent::hasEEQ()) {
    fprintf(fout, "  EEQ '%s'\n", this->defiComponent::EEQ());
  }

  if (this->defiComponent::hasRegionName()) {
    fprintf(fout, "  Region '%s'\n", this->defiComponent::regionName());
  }
  if (this->defiComponent::hasRegionBounds()) {
    int size;
    int *xl, *yl, *xh, *yh;
    int j;
    this->defiComponent::regionBounds(&size, &xl, &yl, &xh, &yh);
    for (j = 0; j < size; j++)
      fprintf(fout, "  Region bounds %d,%d %d,%d\n", xl[j], yl[j], xh[j], yh[j]);
  }
  if (this->defiComponent::hasNets()) {
    int i;
    fprintf(fout, " Net connections:\n");
    for (i = 0; i < this->defiComponent::numNets(); i++) {
      fprintf(fout, "  '%s'\n", this->defiComponent::net(i));
    }
  }
}


int defiComponent::hasRegionName() const {
  return (int)(this->hasRegionName_);
}


int defiComponent::hasGenerate() const {
  return (int)(this->hasGenerate_);
}


int defiComponent::hasWeight() const {
  return (int)(this->hasWeight_);
}


int defiComponent::hasSource() const {
  return this->source_ ? 1 : 0;
}


int defiComponent::hasRegionBounds() const {
  return this->numRects_ ? 1 : 0 ;
}


int defiComponent::hasEEQ() const {
  return (int)(this->hasEEQ_);
}


int defiComponent::hasNets() const {
  return this->numNets_ ? 1 : 0;
}


int defiComponent::numNets() const {
  return this->numNets_;
}


void defiComponent::reverseNetOrder() {
  // Reverse the order of the items in the nets array.
  int one = 0;
  int two = this->numNets_ - 1;
  char* t;
  while (one < two) {
    t = this->nets_[one];
    this->nets_[one] = this->nets_[two];
    this->nets_[two] = t;
    one++;
    two--;
  }
}


char* defiComponent::propName(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError ("bad index for component property");
    return 0;
  }
  return this->names_[index];
}


char* defiComponent::propValue(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError ("bad index for component property");
    return 0;
  }
  return this->values_[index];
}


double defiComponent::propNumber(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError ("bad index for component property");
    return 0;
  }
  return this->dvalues_[index];
}


char defiComponent::propType(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError ("bad index for component property");
    return 0;
  }
  return this->types_[index];
}


int defiComponent::propIsNumber(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError ("bad index for component property");
    return 0;
  }
  return this->dvalues_[index] ? 1 : 0;
}

int defiComponent::propIsString(int index) const {
  if (index < 0 || index >= this->numProps_) {
    defiError ("bad index for component property");
    return 0;
  }
  return this->dvalues_[index] ? 0 : 1;
}

int defiComponent::numProps() const {
  return this->numProps_;
}


void defiComponent::addProperty(const char* name, const char* value,
                                const char type) {
  int len = strlen(name) + 1;
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
      nn[i] = this->names_[i];
      nv[i] = this->values_[i];
      nd[i] = this->dvalues_[i];
      nt[i] = this->types_[i];
    }
    defFree((char*)(this->names_));
    defFree((char*)(this->values_));
    defFree((char*)(this->dvalues_));
    defFree((char*)(this->types_));
    this->names_ = nn;
    this->values_ = nv;
    this->dvalues_ = nd;
    this->types_ = nt;
  }
  this->names_[this->numProps_] = (char*)defMalloc(len);
  strcpy(this->names_[this->numProps_], DEFCASE(name));
  len = strlen(value) + 1;
  this->values_[this->numProps_] = (char*)defMalloc(len);
  strcpy(this->values_[this->numProps_], DEFCASE(value));
  this->dvalues_[this->numProps_] = 0;
  this->types_[this->numProps_] = type;
  this->numProps_ += 1;
}


void defiComponent::addNumProperty(const char* name, const double d,
                                   const char* value, const char type) {
  int len = strlen(name) + 1;
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
      nn[i] = this->names_[i];
      nv[i] = this->values_[i];
      nd[i] = this->dvalues_[i];
      nt[i] = this->types_[i];
    }
    defFree((char*)(this->names_));
    defFree((char*)(this->values_));
    defFree((char*)(this->dvalues_));
    defFree((char*)(this->types_));
    this->names_ = nn;
    this->values_ = nv;
    this->dvalues_ = nd;
    this->types_ = nt;
  }
  this->names_[this->numProps_] = (char*)defMalloc(len);
  strcpy(this->names_[this->numProps_], DEFCASE(name));
  len = strlen(value) + 1;
  this->values_[this->numProps_] = (char*)defMalloc(len);
  strcpy(this->values_[this->numProps_], DEFCASE(value));
  this->dvalues_[this->numProps_] = d;
  this->types_[this->numProps_] = type;
  this->numProps_ += 1;
}


void defiComponent::addNet(const char* net) {
  int len = strlen(net) + 1;
  if (this->numNets_ == this->netsAllocated_)
    this->defiComponent::bumpNets(this->numNets_ * 2);
  this->nets_[this->numNets_] = (char*)defMalloc(len);
  strcpy(this->nets_[this->numNets_], DEFCASE(net));
  (this->numNets_)++;
}


void defiComponent::bumpNets(int size) {
  int i;
  char** newNets = (char**)defMalloc(sizeof(char*)* size);
  for (i = 0; i < this->numNets_; i++) {
    newNets[i] = this->nets_[i];
  }
  defFree((char*)(this->nets_));
  this->nets_ = newNets;
  this->netsAllocated_ = size;
}


const char* defiComponent::net(int index) const {
  if (index >= 0 && index < this->numNets_) {
    return this->nets_[index];
  }
  return 0;
}


void defiComponent::bumpForeignName(int size) {
  if (this->foreignName_) defFree(this->foreignName_);
  this->foreignName_ = (char*)defMalloc(sizeof(char) * size);
  this->foreignNameSize_ = size;
  *(this->foreignName_) = '\0';
}


void defiComponent::setForeignName(const char* name) {
  int len;

  if (this->defiComponent::hasForeignName())
      defiError(
      "Multiple define of '+ FOREIGN' in COMPONENT is not supported.\n");
  if ((len = strlen(name)+1) > this->foreignNameSize_)
    this->defiComponent::bumpForeignName(len);
  strcpy(this->foreignName_, DEFCASE(name));
  this->hasForeignName_ = 1;
}


void defiComponent::setForeignLocation(int x, int y, int orient) {
  this->Fx_ = x;
  this->Fy_ = y;
  this->Fori_ = orient;
  this->hasFori_ = 1;
}


int defiComponent::hasForeignName() const {
  return (int)(this->hasForeignName_);
}


const char* defiComponent::foreignName() const {
  return this->foreignName_;
}


int defiComponent::foreignX() const {
  return this->Fx_;
}


int defiComponent::foreignY() const {
  return this->Fy_;
}


int defiComponent::hasFori() const {
  return (int)(this->hasFori_);
}

const char* defiComponent::foreignOri() const {
  switch (this->Fori_) {
    case 0: return ("N");
    case 1: return ("W");
    case 2: return ("S");
    case 3: return ("E");
    case 4: return ("FN");
    case 5: return ("FW");
    case 6: return ("FS");
    case 7: return ("FE");
  }
  return 0;
}
