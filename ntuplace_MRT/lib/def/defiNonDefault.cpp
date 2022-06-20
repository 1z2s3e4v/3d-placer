/*
 *     This  file  is  part  of  the  Cadence  LEF/DEF  Open   Source
 *  Distribution,  Product Version 5.7, and is subject to the Cadence
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
#include "defiNonDefault.hpp"
#include "defiDebug.hpp"


/////////////////////////////////////////
/////////////////////////////////////////
//
//     defiNonDefault
//
/////////////////////////////////////////
/////////////////////////////////////////

defiNonDefault::defiNonDefault() { 
  this->defiNonDefault::Init();
}

void defiNonDefault::Init() { 
  this->name_ = 0;
  this->hardSpacing_ = 0;
  this->numLayers_ = 0;
  this->width_ = 0;
  this->hasDiagWidth_ = 0;
  this->hasSpacing_ = 0;
  this->hasWireExt_ = 0;
  this->numVias_ = 0;
  this->viasAllocated_ = 0;
  this->viaNames_ = 0;
  this->numViaRules_ = 0;
  this->viaRulesAllocated_ = 0;
  this->viaRuleNames_ = 0;
  this->numMinCuts_ = 0;
  this->minCutsAllocated_ = 0;
  this->cutLayerName_ = 0;
  this->numCuts_ = 0;
  this->numProps_ = 0;
  this->propsAllocated_ = 0;
  this->names_ = 0;
  this->values_ = 0;
  this->dvalues_ = 0;
  this->types_ = 0;
  this->layersAllocated_ = 0;
  this->layerName_ = 0;
  this->width_ = 0;
  this->hasDiagWidth_ = 0;
  this->diagWidth_ = 0;
  this->hasSpacing_ = 0;
  this->spacing_ = 0;
  this->hasWireExt_ = 0;
  this->wireExt_ = 0;
}

defiNonDefault::~defiNonDefault() { 
  this->defiNonDefault::Destroy();
}

void defiNonDefault::Destroy() {
  this->defiNonDefault::clear();
  if (this->propsAllocated_) {
    defFree((char*)(this->names_));
    defFree((char*)(this->values_));
    defFree((char*)(this->dvalues_));
    defFree((char*)(this->types_));
  }
  if (this->layersAllocated_) {
    defFree((char*)(this->layerName_));
    defFree((char*)(this->width_));
    defFree((char*)(this->hasDiagWidth_));
    defFree((char*)(this->diagWidth_));
    defFree((char*)(this->hasSpacing_));
    defFree((char*)(this->spacing_));
    defFree((char*)(this->hasWireExt_));
    defFree((char*)(this->wireExt_));
  }
  if (this->viasAllocated_)
    defFree((char*)(this->viaNames_));
  if (this->viaRulesAllocated_)
    defFree((char*)(this->viaRuleNames_));
  if (this->minCutsAllocated_) {
    defFree((char*)(this->cutLayerName_));
    defFree((char*)(this->numCuts_));
  }
}

void defiNonDefault::clear() { 
  int i;

  if (this->name_)
    defFree(this->name_);
  this->hardSpacing_ = 0;
  for (i = 0; i < this->numProps_; i++) {
    defFree(this->names_[i]);
    defFree(this->values_[i]);
    this->dvalues_[i] = 0;
  }
  this->numProps_ = 0;
  for (i = 0; i < this->numLayers_; i++)
    defFree(this->layerName_[i]);
  this->numLayers_ = 0;
  for (i = 0; i < this->numVias_; i++)
    defFree((char*)(this->viaNames_[i]));
  this->numVias_ = 0;
  for (i = 0; i < this->numViaRules_; i++)
    defFree((char*)(this->viaRuleNames_[i]));
  this->numViaRules_ = 0;
  for (i = 0; i < this->numMinCuts_; i++)
    defFree((char*)(this->cutLayerName_[i]));
  this->numMinCuts_ = 0;
}

void defiNonDefault::setName(const char* name) { 
  this->name_ = (char*)defMalloc(strlen(name)+1);
  strcpy(this->name_, DEFCASE(name));
}

void defiNonDefault::setHardspacing() {
  this->hardSpacing_ = 1;
}

void defiNonDefault::addLayer(const char* name) { 
  int len = strlen(name) + 1;
  if (this->numLayers_ == this->layersAllocated_) {
    int i;
    char**  newl;
    double* neww;
    double* newd;
    double* news;
    double* newe;
    char*   newhd;
    char*   newhs;
    char*   newhe;

    if (this->layersAllocated_ == 0)
      this->layersAllocated_ = 2;
    else
      this->layersAllocated_ *= 2;
    newl = (char**)defMalloc(sizeof(char*) * this->layersAllocated_);
    newe = (double*)defMalloc(sizeof(double) * this->layersAllocated_);
    neww = (double*)defMalloc(sizeof(double) * this->layersAllocated_);
    newd = (double*)defMalloc(sizeof(double) * this->layersAllocated_);
    news = (double*)defMalloc(sizeof(double) * this->layersAllocated_);
    newhe = (char*)defMalloc(sizeof(char) * this->layersAllocated_);
    newhd = (char*)defMalloc(sizeof(char) * this->layersAllocated_);
    newhs = (char*)defMalloc(sizeof(char) * this->layersAllocated_);
    for (i = 0; i < this->numLayers_; i++) {
      newl[i]  = this->layerName_[i];
      neww[i]  = this->width_[i];
      newd[i]  = this->diagWidth_[i];
      news[i]  = this->spacing_[i];
      newe[i]  = this->wireExt_[i];
      newhe[i]  = this->hasWireExt_[i];
      newhd[i]  = this->hasDiagWidth_[i];
      newhs[i]  = this->hasSpacing_[i];
    }
    if (this->layersAllocated_ > 2) {
      defFree((char*)(this->layerName_));
      defFree((char*)(this->width_));
      defFree((char*)(this->diagWidth_));
      defFree((char*)(this->spacing_));
      defFree((char*)(this->wireExt_));
      defFree((char*)(this->hasWireExt_));
      defFree((char*)(this->hasDiagWidth_));
      defFree((char*)(this->hasSpacing_));
    }
    this->layerName_ = newl;
    this->width_ = neww;
    this->diagWidth_ = newd;
    this->spacing_ = news;
    this->wireExt_ = newe;
    this->hasDiagWidth_ = newhd;
    this->hasSpacing_ = newhs;
    this->hasWireExt_ = newhe;
  }
  this->layerName_[this->numLayers_] = (char*)defMalloc(len);
  strcpy(this->layerName_[this->numLayers_], DEFCASE(name));
  this->width_[this->numLayers_] = 0.0;
  this->diagWidth_[this->numLayers_] = 0.0;
  this->spacing_[this->numLayers_] = 0.0;
  this->wireExt_[this->numLayers_] = 0.0;
  this->hasDiagWidth_[this->numLayers_] = '\0';
  this->hasSpacing_[this->numLayers_] = '\0';
  this->hasWireExt_[this->numLayers_] = '\0';
  this->numLayers_ += 1;
}

void defiNonDefault::addWidth(double num) { 
  this->width_[this->numLayers_-1] = num;
}

void defiNonDefault::addDiagWidth(double num) { 
  this->diagWidth_[this->numLayers_-1] = num;
  this->hasDiagWidth_[this->numLayers_-1] = 1;
}

void defiNonDefault::addSpacing(double num) { 
  this->spacing_[this->numLayers_-1] = num;
  this->hasSpacing_[this->numLayers_-1] = 1;
}

void defiNonDefault::addWireExt(double num) { 
  this->wireExt_[this->numLayers_-1] = num;
  this->hasWireExt_[this->numLayers_-1] = 1;
}

void defiNonDefault::addVia(const char* name) {
  if (this->numVias_ == this->viasAllocated_) {
    int i;
    char** vn;

    if (this->viasAllocated_ == 0)
      this->viasAllocated_ = 2;
    else
      this->viasAllocated_ *= 2;
    vn = (char**)defMalloc(sizeof(char*)* this->viasAllocated_);
    for (i = 0; i < this->numVias_; i++) {
      vn[i] = this->viaNames_[i];
    }
    defFree((char*)(this->viaNames_));
    this->viaNames_ = vn;
  } 
  this->viaNames_[this->numVias_] = (char*)defMalloc(strlen(name)+1);
  strcpy(this->viaNames_[this->numVias_], DEFCASE(name));
  this->numVias_ += 1;
}

void defiNonDefault::addViaRule(const char* name) {
  if (this->numViaRules_ == this->viaRulesAllocated_) {
    int i;
    char** vn;

    if (this->viaRulesAllocated_ == 0)
      this->viaRulesAllocated_ = 2;
    else
      this->viaRulesAllocated_ *= 2;
    vn = (char**)defMalloc(sizeof(char*)* this->viaRulesAllocated_);
    for (i = 0; i < this->numViaRules_; i++) {
      vn[i] = this->viaRuleNames_[i];
    }
    defFree((char*)(this->viaRuleNames_));
    this->viaRuleNames_ = vn;
  } 
  this->viaRuleNames_[this->numViaRules_] = (char*)defMalloc(strlen(name)+1);
  strcpy(this->viaRuleNames_[this->numViaRules_], DEFCASE(name));
  this->numViaRules_ += 1;
}

void defiNonDefault::addMinCuts(const char* name, int numCuts) {
  if (this->numMinCuts_ == this->minCutsAllocated_) {
    int i;
    char** cln;
    int*   nc;

    if (this->minCutsAllocated_ == 0)
      this->minCutsAllocated_ = 2;
    else
      this->minCutsAllocated_ *= 2;
    cln = (char**)defMalloc(sizeof(char*)* this->minCutsAllocated_);
    nc = (int*)defMalloc(sizeof(int)* this->minCutsAllocated_);
    for (i = 0; i < this->numMinCuts_; i++) {
      cln[i] = this->cutLayerName_[i];
      nc[i]  = this->numCuts_[i];
    }
    if (this->minCutsAllocated_ > 2) {
      defFree((char*)(this->cutLayerName_));
      defFree((char*)(this->numCuts_));
    }
    this->cutLayerName_ = cln;
    this->numCuts_ = nc;
  } 
  this->cutLayerName_[this->numMinCuts_] = (char*)defMalloc(strlen(name)+1);
  strcpy(this->cutLayerName_[this->numMinCuts_], DEFCASE(name));
  this->numCuts_[this->numMinCuts_] = numCuts;
  this->numMinCuts_ += 1;
}

const char* defiNonDefault::name() const {
  return this->name_;
}

int defiNonDefault::hasHardspacing() const {
  return this->hardSpacing_;
}

int defiNonDefault::numLayers() const { 
  return this->numLayers_;
}

const char* defiNonDefault::layerName(int index) const { 
  char msg[160];
  if (index < 0 || index >= this->numLayers_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return this->layerName_[index];
}

// Will be obsoleted in 5.7
double defiNonDefault::layerWidth(int index) const { 
  char msg[160];
  if (index < 0 || index >= this->numLayers_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return this->width_[index];
}

int defiNonDefault::layerWidthVal(int index) const { 
  char msg[160];
  if (index < 0 || index >= this->numLayers_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return (int)this->width_[index];
}

int defiNonDefault::hasLayerDiagWidth(int index) const { 
  char msg[160];
  if (index < 0 || index >= this->numLayers_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return this->hasDiagWidth_[index];
}

// Will be obsoleted in 5.7
double defiNonDefault::layerDiagWidth(int index) const { 
  char msg[160];
  if (index < 0 || index >= this->numLayers_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return this->diagWidth_[index];
}

int defiNonDefault::layerDiagWidthVal(int index) const { 
  char msg[160];
  if (index < 0 || index >= this->numLayers_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return (int)this->diagWidth_[index];
}

int defiNonDefault::hasLayerWireExt(int index) const { 
  char msg[160];
  if (index < 0 || index >= this->numLayers_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return this->hasWireExt_[index];
}

int defiNonDefault::hasLayerSpacing(int index) const { 
  char msg[160];
  if (index < 0 || index >= this->numLayers_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return this->hasSpacing_[index];
}

// Will be obsoleted in 5.7
double defiNonDefault::layerWireExt(int index) const { 
  char msg[160];
  if (index < 0 || index >= this->numLayers_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return this->wireExt_[index];
}

int defiNonDefault::layerWireExtVal(int index) const { 
  char msg[160];
  if (index < 0 || index >= this->numLayers_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return (int)this->wireExt_[index];
}

// Will be obsoleted in 5.7
double defiNonDefault::layerSpacing(int index) const { 
  char msg[160];
  if (index < 0 || index >= this->numLayers_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return this->spacing_[index];
}

int defiNonDefault::layerSpacingVal(int index) const { 
  char msg[160];
  if (index < 0 || index >= this->numLayers_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return (int)this->spacing_[index];
}

int defiNonDefault::numVias() const { 
  return this->numVias_;
}

const char* defiNonDefault::viaName(int index) const {
  char msg[160];
  if (index < 0 || index >= this->numVias_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return this->viaNames_[index];
}

int defiNonDefault::numViaRules() const { 
  return this->numViaRules_;
}

const char* defiNonDefault::viaRuleName(int index) const {
  char msg[160];
  if (index < 0 || index >= this->numViaRules_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return this->viaRuleNames_[index];
}

int defiNonDefault::numMinCuts() const {
  return this->numMinCuts_;
}

const char* defiNonDefault::cutLayerName(int index) const {
  char msg[160];
  if (index < 0 || index >= this->numMinCuts_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return this->cutLayerName_[index];
}

int defiNonDefault::numCuts(int index) const {
  char msg[160];
  if (index < 0 || index >= this->numMinCuts_) {
     sprintf (msg, "ERROR (DEFPARS-6090): The index number %d given for the NONDEFAULT LAYER is invalid.\nValid index is from 0 to %d",
             index, this->numLayers_);
     defiError (msg);
     return 0;
  }
  return this->numCuts_[index];
}

void defiNonDefault::print(FILE* f) { 
  int   i;

  fprintf(f, "nondefaultrule %s\n", this->defiNonDefault::name());
  
  fprintf(f, "%d layers   %d vias   %d viarules   %d mincuts\n",
    this->defiNonDefault::numLayers(),
    this->defiNonDefault::numVias(),
    this->defiNonDefault::numViaRules(),
    this->defiNonDefault::numMinCuts());

  for (i = 0; i < this->defiNonDefault::numLayers(); i++) {
    fprintf(f, "  Layer %s\n", this->defiNonDefault::layerName(i));
    fprintf(f, "    WIDTH %g\n", this->defiNonDefault::layerWidth(i));
    if (this->defiNonDefault::hasLayerDiagWidth(i))
      fprintf(f, "    DIAGWIDTH %g\n", this->defiNonDefault::layerDiagWidth(i));
    if (this->defiNonDefault::hasLayerSpacing(i))
      fprintf(f, "    SPACING %g\n", this->defiNonDefault::layerSpacing(i));
    if (this->defiNonDefault::hasLayerWireExt(i))
      fprintf(f, "    WIREEXT %g\n",
              this->defiNonDefault::layerWireExt(i));
  }
  for (i = 0; i < this->defiNonDefault::numVias(); i++) {
    fprintf(f, "    VIA %s\n", this->defiNonDefault::viaName(i));
  }
  for (i = 0; i < this->defiNonDefault::numViaRules(); i++) {
    fprintf(f, "    VIARULE %s\n", this->defiNonDefault::viaRuleName(i));
  }
  for (i = 0; i < this->defiNonDefault::numMinCuts(); i++) {
    fprintf(f, "    MINCUTS %s %d\n", this->defiNonDefault::cutLayerName(i),
            this->defiNonDefault::numCuts(i));
  }
}

int defiNonDefault::numProps() const {
  return this->numProps_;
}

void defiNonDefault::addProperty(const char* name, const char* value,
                                 const char type) {
  int len = strlen(name) + 1;
  if (this->numProps_ == this->propsAllocated_) {
    int i;
    int max;
    int lim = this->numProps_;
    char**  nn;
    char**  nv;
    double* nd;
    char*   nt;

    if (this->propsAllocated_ == 0) 
      max = this->propsAllocated_ = 2;
    else
      max = this->propsAllocated_ *= 2;
    nn = (char**)defMalloc(sizeof(char*) * max);
    nv = (char**)defMalloc(sizeof(char*) * max);
    nd = (double*)defMalloc(sizeof(double) * max);
    nt = (char*)defMalloc(sizeof(char) * max);
    for (i = 0; i < lim; i++) {
      nn[i] = this->names_[i];
      nv[i] = this->values_[i];
      nd[i] = this->dvalues_[i];
      nt[i] = this->types_[i];
    }
    defFree((char*)(this->names_));
    defFree((char*)(this->values_));
    defFree((char*)(this->dvalues_));
    defFree((char*)(this->types_));
    this->names_   = nn;
    this->values_  = nv;
    this->dvalues_ = nd;
    this->types_   = nt;
  }
  this->names_[this->numProps_] = (char*)defMalloc(sizeof(char)*len);
  strcpy(this->names_[this->numProps_],name);
  len = strlen(value) + 1;
  this->values_[this->numProps_] = (char*)defMalloc(sizeof(char) * len);
  strcpy(this->values_[this->numProps_],value);
  this->dvalues_[this->numProps_] = 0;
  this->types_[this->numProps_] = type;
  this->numProps_ += 1;
}

void defiNonDefault::addNumProperty(const char* name, const double d,
                                    const char* value, const char type) {
  int len = strlen(name) + 1;
  if (this->numProps_ == this->propsAllocated_) {
    int i;
    int max;
    int lim = this->numProps_;
    char**  nn;
    char**  nv;
    double* nd;
    char*   nt;

    if (this->propsAllocated_ == 0) 
      max = this->propsAllocated_ = 2;
    else
      max = this->propsAllocated_ *= 2;
    nn = (char**)defMalloc(sizeof(char*) * max);
    nv = (char**)defMalloc(sizeof(char*) * max);
    nd = (double*)defMalloc(sizeof(double) * max);
    nt = (char*)defMalloc(sizeof(char) * max);
    for (i = 0; i < lim; i++) {
      nn[i] = this->names_[i];
      nv[i] = this->values_[i];
      nd[i] = this->dvalues_[i];
      nt[i] = this->types_[i];
    }
    defFree((char*)(this->names_));
    defFree((char*)(this->values_));
    defFree((char*)(this->dvalues_));
    defFree((char*)(this->types_));
    this->names_   = nn;
    this->values_  = nv;
    this->dvalues_ = nd;
    this->types_   = nt;
  }
  this->names_[this->numProps_] = (char*)defMalloc(sizeof(char) * len);
  strcpy(this->names_[this->numProps_],name);
  len = strlen(value) + 1;
  this->values_[this->numProps_] = (char*)defMalloc(sizeof(char) * len);
  strcpy(this->values_[this->numProps_],value);
  this->dvalues_[this->numProps_] = d;
  this->types_[this->numProps_] = type;
  this->numProps_ += 1;
}

const char* defiNonDefault::propName(int index) const {
  char msg[160];
  if (index < 0 || index >= this->numProps_) {
     sprintf (msg, "ERROR (DEFPARS-6091): The index number %d given for the NONDEFAULT PROPERTY is invalid.\nValid index is from 0 to %d",
             index, this->numProps_);
     defiError (msg);
     return 0;
  }
  return this->names_[index];
}

const char* defiNonDefault::propValue(int index) const {
  char msg[160];
  if (index < 0 || index >= this->numProps_) {
     sprintf (msg, "ERROR (DEFPARS-6091): The index number %d given for the NONDEFAULT PROPERTY is invalid.\nValid index is from 0 to %d",
             index, this->numProps_);
     defiError (msg);
     return 0;
  }
  return this->values_[index];
}

double defiNonDefault::propNumber(int index) const {
  char msg[160];
  if (index < 0 || index >= this->numProps_) {
     sprintf (msg, "ERROR (DEFPARS-6091): The index number %d given for the NONDEFAULT PROPERTY is invalid.\nValid index is from 0 to %d",
             index, this->numProps_);
     defiError (msg);
     return 0;
  }
  return this->dvalues_[index];
}

const char defiNonDefault::propType(int index) const {
  char msg[160];
  if (index < 0 || index >= this->numProps_) {
     sprintf (msg, "ERROR (DEFPARS-6091): The index number %d given for the NONDEFAULT PROPERTY is invalid.\nValid index is from 0 to %d",
             index, this->numProps_);
     defiError (msg);
     return 0;
  }
  return this->types_[index];
}

int defiNonDefault::propIsNumber(int index) const {
  char msg[160];
  if (index < 0 || index >= this->numProps_) {
     sprintf (msg, "ERROR (DEFPARS-6091): The index number %d given for the NONDEFAULT PROPERTY is invalid.\nValid index is from 0 to %d",
             index, this->numProps_);
     defiError (msg);
     return 0;
  }
  return this->dvalues_[index] ? 1 : 0;
}

int defiNonDefault::propIsString(int index) const {
  char msg[160];
  if (index < 0 || index >= this->numProps_) {
     sprintf (msg, "ERROR (DEFPARS-6091): The index number %d given for the NONDEFAULT PROPERTY is invalid.\nValid index is from 0 to %d",
             index, this->numProps_);
     defiError (msg);
     return 0;
  }
  return this->dvalues_[index] ? 0 : 1;
}
