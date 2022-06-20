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
#include "defiPinCap.hpp"
#include "defiComponent.hpp"
#include "defiDebug.hpp"
#include "defiUtil.hpp"


///////////////////////////////////////////////
///////////////////////////////////////////////
//
//     defiPinCap
//
///////////////////////////////////////////////
///////////////////////////////////////////////


void defiPinCap::setPin(int p) {
  this->pin_ = p;
}


void defiPinCap::setCap(double d) {
  this->cap_ = d;
}


int defiPinCap::pin() const {
  return this->pin_;
}


double defiPinCap::cap() const {
  return this->cap_;
}


void defiPinCap::print(FILE* f) const {
  fprintf(f, "PinCap  %d %5.2f\n", this->pin_, this->cap_);
}


///////////////////////////////////////////////
///////////////////////////////////////////////
//
//     defiPinAntennaModel
//
///////////////////////////////////////////////
///////////////////////////////////////////////

defiPinAntennaModel::defiPinAntennaModel() {
  this->defiPinAntennaModel::Init();
}


void defiPinAntennaModel::Init() {
  this->numAPinGateArea_ = 0;                       // 5.4
  this->APinGateAreaAllocated_ = 0;                 // 5.4
  this->APinGateArea_ = 0;
  this->APinGateAreaLayer_ = 0;
  this->numAPinMaxAreaCar_ = 0;                     // 5.4
  this->APinMaxAreaCarAllocated_ = 0;               // 5.4
  this->APinMaxAreaCar_ = 0;
  this->APinMaxAreaCarLayer_ = 0;
  this->numAPinMaxSideAreaCar_ = 0;                 // 5.4
  this->APinMaxSideAreaCarAllocated_ = 0;           // 5.4
  this->APinMaxSideAreaCar_ = 0;
  this->APinMaxSideAreaCarLayer_ = 0;
  this->numAPinMaxCutCar_ = 0;                      // 5.4
  this->APinMaxCutCarAllocated_ = 0;                // 5.4
  this->APinMaxCutCar_ = 0;
  this->APinMaxCutCarLayer_ = 0;
}

defiPinAntennaModel::~defiPinAntennaModel() {
  this->defiPinAntennaModel::Destroy();
}


void defiPinAntennaModel::clear() {
  int i;

  if (this->oxide_)
    defFree((char*)(this->oxide_));
  this->oxide_ = 0;

  for (i = 0; i < this->numAPinGateArea_; i++) {
    if (this->APinGateAreaLayer_[i])
       defFree(this->APinGateAreaLayer_[i]);
  }
  this->numAPinGateArea_ = 0;

  for (i = 0; i < this->numAPinMaxAreaCar_; i++) {
    if (this->APinMaxAreaCarLayer_[i])
       defFree(this->APinMaxAreaCarLayer_[i]);
  }
  this->numAPinMaxAreaCar_ = 0;

  for (i = 0; i < this->numAPinMaxSideAreaCar_; i++) {
    if (this->APinMaxSideAreaCarLayer_[i])
       defFree(this->APinMaxSideAreaCarLayer_[i]);
  }
  this->numAPinMaxSideAreaCar_ = 0;

  for (i = 0; i < this->numAPinMaxCutCar_; i++) {
    if (this->APinMaxCutCarLayer_[i])
       defFree(this->APinMaxCutCarLayer_[i]);
  }
  this->numAPinMaxCutCar_ = 0;
}

void defiPinAntennaModel::Destroy() {
  this->defiPinAntennaModel::clear();
  if (this->APinGateArea_)
     defFree((char*)(this->APinGateArea_));
  if (this->APinGateAreaLayer_)
     defFree((char*)(this->APinGateAreaLayer_));
  if (this->APinMaxAreaCar_)
     defFree((char*)(this->APinMaxAreaCar_));
  if (this->APinMaxAreaCarLayer_)
     defFree((char*)(this->APinMaxAreaCarLayer_));
  if (this->APinMaxSideAreaCar_)
     defFree((char*)(this->APinMaxSideAreaCar_));
  if (this->APinMaxSideAreaCarLayer_)
     defFree((char*)(this->APinMaxSideAreaCarLayer_));
  if (this->APinMaxCutCar_)
     defFree((char*)(this->APinMaxCutCar_));
  if (this->APinMaxCutCarLayer_)
     defFree((char*)(this->APinMaxCutCarLayer_));
}

// 5.5
void defiPinAntennaModel::setAntennaModel(int aOxide) {
  switch (aOxide) {
     case 2:
           this->oxide_ = strdup("OXIDE2");
           break;
     case 3:
           this->oxide_ = strdup("OXIDE3");
           break;
     case 4:
           this->oxide_ = strdup("OXIDE4");
           break;
     default:
           this->oxide_ = strdup("OXIDE1");
           break;
  }
}

void defiPinAntennaModel::addAPinGateArea(int value, const char* layer) {
  if (this->numAPinGateArea_ == this->APinGateAreaAllocated_) {
     int i;
     int max;
     int lim = this->numAPinGateArea_;
     int* nd;
     char**  nl;

     if (this->APinGateAreaAllocated_ == 0)
        max = this->APinGateAreaAllocated_ = 2;
     else
        max = this->APinGateAreaAllocated_ *= 2;
     nd = (int*)defMalloc(sizeof(int)*max);
     nl = (char**)defMalloc(sizeof(char*)*max);
     for (i = 0; i < lim; i++) {
        nd[i] = this->APinGateArea_[i];
        nl[i] = this->APinGateAreaLayer_[i];
     }
     defFree((char*)(this->APinGateArea_));
     defFree((char*)(this->APinGateAreaLayer_));
     this->APinGateArea_ = nd;
     this->APinGateAreaLayer_ = nl;

  }
  this->APinGateArea_[this->numAPinGateArea_] = value;
  if (layer) {
    this->APinGateAreaLayer_[this->numAPinGateArea_] =
       (char*)defMalloc(strlen(layer)+1);
    strcpy(this->APinGateAreaLayer_[this->numAPinGateArea_],
       DEFCASE(layer));
  } else
    this->APinGateAreaLayer_[this->numAPinGateArea_] = NULL;
  this->numAPinGateArea_ += 1;
}

void defiPinAntennaModel::addAPinMaxAreaCar(int value, const char* layer) {
  if (this->numAPinMaxAreaCar_ == this->APinMaxAreaCarAllocated_) {
     int i;
     int max;
     int lim = this->numAPinMaxAreaCar_;
     int* nd;
     char**  nl;

     if (this->APinMaxAreaCarAllocated_ == 0)
        max = this->APinMaxAreaCarAllocated_ = 2;
     else
        max = this->APinMaxAreaCarAllocated_ *= 2;
     nd = (int*)defMalloc(sizeof(int)*max);
     nl = (char**)defMalloc(sizeof(char*)*max);
     for (i = 0; i < lim; i++) {
        nd[i] = this->APinMaxAreaCar_[i];
        nl[i] = this->APinMaxAreaCarLayer_[i];
     }
     defFree((char*)(this->APinMaxAreaCar_));
     defFree((char*)(this->APinMaxAreaCarLayer_));
     this->APinMaxAreaCar_ = nd;
     this->APinMaxAreaCarLayer_ = nl;

  }
  this->APinMaxAreaCar_[this->numAPinMaxAreaCar_] = value;
  if (layer) {
    this->APinMaxAreaCarLayer_[this->numAPinMaxAreaCar_] =
       (char*)defMalloc(strlen(layer)+1);
    strcpy(this->APinMaxAreaCarLayer_[this->numAPinMaxAreaCar_],
       DEFCASE(layer));
  } else
    this->APinMaxAreaCarLayer_[this->numAPinMaxAreaCar_] = NULL;
  this->numAPinMaxAreaCar_ += 1;
}

void defiPinAntennaModel::addAPinMaxSideAreaCar(int value, const char* layer) {
  if (this->numAPinMaxSideAreaCar_ == this->APinMaxSideAreaCarAllocated_) {
     int i;
     int max;
     int lim = this->numAPinMaxSideAreaCar_;
     int* nd;
     char**  nl;

     if (this->APinMaxSideAreaCarAllocated_ == 0)
        max = this->APinMaxSideAreaCarAllocated_ = 2;
     else
        max = this->APinMaxSideAreaCarAllocated_ *= 2;
     nd = (int*)defMalloc(sizeof(int)*max);
     nl = (char**)defMalloc(sizeof(char*)*max);
     for (i = 0; i < lim; i++) {
        nd[i] = this->APinMaxSideAreaCar_[i];
        nl[i] = this->APinMaxSideAreaCarLayer_[i];
     }
     defFree((char*)(this->APinMaxSideAreaCar_));
     defFree((char*)(this->APinMaxSideAreaCarLayer_));
     this->APinMaxSideAreaCar_ = nd;
     this->APinMaxSideAreaCarLayer_ = nl;

  }
  this->APinMaxSideAreaCar_[this->numAPinMaxSideAreaCar_] = value;
  if (layer) {
    this->APinMaxSideAreaCarLayer_[this->numAPinMaxSideAreaCar_] =
       (char*)defMalloc(strlen(layer)+1);
    strcpy(this->APinMaxSideAreaCarLayer_[this->numAPinMaxSideAreaCar_],
       DEFCASE(layer));
  } else
    this->APinMaxSideAreaCarLayer_[this->numAPinMaxSideAreaCar_] = NULL;
  this->numAPinMaxSideAreaCar_ += 1;
}

void defiPinAntennaModel::addAPinMaxCutCar(int value, const char* layer) {
  if (this->numAPinMaxCutCar_ == this->APinMaxCutCarAllocated_) {
     int i;
     int max;
     int lim = this->numAPinMaxCutCar_;
     int* nd;
     char**  nl;

     if (this->APinMaxCutCarAllocated_ == 0)
        max = this->APinMaxCutCarAllocated_ = 2;
     else
        max = this->APinMaxCutCarAllocated_ *= 2;
     nd = (int*)defMalloc(sizeof(int)*max);
     nl = (char**)defMalloc(sizeof(char*)*max);
     for (i = 0; i < lim; i++) {
        nd[i] = this->APinMaxCutCar_[i];
        nl[i] = this->APinMaxCutCarLayer_[i];
     }
     defFree((char*)(this->APinMaxCutCar_));
     defFree((char*)(this->APinMaxCutCarLayer_));
     this->APinMaxCutCar_ = nd;
     this->APinMaxCutCarLayer_ = nl;

  }
  this->APinMaxCutCar_[this->numAPinMaxCutCar_] = value;
  if (layer) {
    this->APinMaxCutCarLayer_[this->numAPinMaxCutCar_] =
       (char*)defMalloc(strlen(layer)+1);
    strcpy(this->APinMaxCutCarLayer_[this->numAPinMaxCutCar_],
       DEFCASE(layer));
  } else
    this->APinMaxCutCarLayer_[this->numAPinMaxCutCar_] = NULL;
  this->numAPinMaxCutCar_ += 1;
}

// 5.5
char* defiPinAntennaModel::antennaOxide() const {
  return this->oxide_;
}

int defiPinAntennaModel::hasAPinGateArea() const {
  return this->numAPinGateArea_ ? 1 : 0 ;
}

int defiPinAntennaModel::hasAPinMaxAreaCar() const {
  return this->numAPinMaxAreaCar_ ? 1 : 0 ;
}

int defiPinAntennaModel::hasAPinMaxSideAreaCar() const {
  return this->numAPinMaxSideAreaCar_ ? 1 : 0 ;
}

int defiPinAntennaModel::hasAPinMaxCutCar() const {
  return this->numAPinMaxCutCar_ ? 1 : 0 ;
}

int defiPinAntennaModel::numAPinGateArea() const {
  return this->numAPinGateArea_;
}

int defiPinAntennaModel::numAPinMaxAreaCar() const {
  return this->numAPinMaxAreaCar_;
}

int defiPinAntennaModel::numAPinMaxSideAreaCar() const {
  return this->numAPinMaxSideAreaCar_;
}

int defiPinAntennaModel::numAPinMaxCutCar() const {
  return this->numAPinMaxCutCar_;
}

int defiPinAntennaModel::APinGateArea(int i) const {
  return this->APinGateArea_[i];
}

const char* defiPinAntennaModel::APinGateAreaLayer(int i) const {
  return this->APinGateAreaLayer_[i];
}

int defiPinAntennaModel::APinMaxAreaCar(int i) const {
  return this->APinMaxAreaCar_[i];
}

const char* defiPinAntennaModel::APinMaxAreaCarLayer(int i) const {
  return this->APinMaxAreaCarLayer_[i];
}

int defiPinAntennaModel::APinMaxSideAreaCar(int i) const {
  return this->APinMaxSideAreaCar_[i];
}

const char* defiPinAntennaModel::APinMaxSideAreaCarLayer(int i) const {
  return this->APinMaxSideAreaCarLayer_[i];
}

int defiPinAntennaModel::APinMaxCutCar(int i) const {
  return this->APinMaxCutCar_[i];
}

const char* defiPinAntennaModel::APinMaxCutCarLayer(int i) const {
  return this->APinMaxCutCarLayer_[i];
}


///////////////////////////////////////////////
///////////////////////////////////////////////
//
//     defiPin
//
///////////////////////////////////////////////
///////////////////////////////////////////////

defiPin::defiPin() {
  this->defiPin::Init();
}


void defiPin::Init() {
  this->pinNameLength_ = 0;
  this->pinName_ = 0;
  this->netNameLength_ = 0;
  this->netName_ = 0;
  this->useLength_ = 0;
  this->use_ = 0;
  this->directionLength_ = 0;
  this->direction_ = 0;
  this->hasDirection_ = 0;
  this->hasUse_ = 0;
  this->hasLayer_ = 0;
  this->placeType_ = 0;
  this->orient_ = 0;
  this->xl_ = 0;
  this->yl_ = 0;
  this->xh_ = 0;
  this->yh_ = 0;
  this->x_ = 0;
  this->y_ = 0;
  this->layer_ = 0;
  this->layerLength_ = 0;
  this->numAPinPartialMetalArea_ = 0;               // 5.4
  this->APinPartialMetalAreaAllocated_ = 0;         // 5.4
  this->APinPartialMetalArea_ = 0;
  this->APinPartialMetalAreaLayer_ = 0;
  this->numAPinPartialMetalSideArea_ = 0;           // 5.4
  this->APinPartialMetalSideAreaAllocated_ = 0;     // 5.4
  this->APinPartialMetalSideArea_ = 0;
  this->APinPartialMetalSideAreaLayer_ = 0;
  this->numAPinDiffArea_ = 0;                       // 5.4
  this->APinDiffAreaAllocated_ = 0;                 // 5.4
  this->APinDiffArea_ = 0;
  this->APinDiffAreaLayer_ = 0;
  this->numAPinPartialCutArea_ = 0;                 // 5.4
  this->APinPartialCutAreaAllocated_ = 0;           // 5.4
  this->APinPartialCutArea_ = 0;
  this->APinPartialCutAreaLayer_ = 0;
  this->antennaModel_ = 0;
}


defiPin::~defiPin() {
  this->defiPin::Destroy();
}


void defiPin::clear() {
  int i;
  defiPinAntennaModel* aModel;

  this->hasDirection_ = 0;
  this->hasUse_ = 0;
  this->hasLayer_ = 0;
  this->hasSpecial_ = 0;
  this->placeType_ = 0;
  this->orient_ = 0;
  this->xl_ = 0;
  this->yl_ = 0;
  this->xh_ = 0;
  this->yh_ = 0;
  this->x_ = 0;
  this->y_ = 0;

  for (i = 0; i < this->numAPinPartialMetalArea_; i++) {
    if (this->APinPartialMetalAreaLayer_[i])
       defFree(this->APinPartialMetalAreaLayer_[i]);
  }
  this->numAPinPartialMetalArea_ = 0;

  for (i = 0; i < this->numAPinPartialMetalSideArea_; i++) {
    if (this->APinPartialMetalSideAreaLayer_[i])
       defFree(this->APinPartialMetalSideAreaLayer_[i]);
  }
  this->numAPinPartialMetalSideArea_ = 0;

  for (i = 0; i < this->numAPinDiffArea_; i++) {
    if (this->APinDiffAreaLayer_[i])
       defFree(this->APinDiffAreaLayer_[i]);
  }
  this->numAPinDiffArea_ = 0;

  for (i = 0; i < this->numAPinPartialCutArea_; i++) {
    if (this->APinPartialCutAreaLayer_[i])
       defFree(this->APinPartialCutAreaLayer_[i]);
  }
  this->numAPinPartialCutArea_ = 0;

  for (i = 0; i < this->antennaModelAllocated_; i++) { // 5.5
    aModel = this->antennaModel_[i];
    if (i < this->numAntennaModel_) {
       // 1/21/2003 -  pcr 495084, free data in antennaModel
       aModel->defiPinAntennaModel::Destroy();
    }
    defFree(aModel);
  }
  this->numAntennaModel_ = 0;
  this->antennaModelAllocated_ = 0;
}


void defiPin::Destroy() {
  int i;

  if (this->pinName_) defFree(this->pinName_);
  if (this->netName_) defFree(this->netName_);
  if (this->use_) defFree(this->use_);
  if (this->direction_) defFree(this->direction_);
  if (this->layer_) defFree(this->layer_);
  this->pinName_ = 0;
  this->netName_ = 0;
  this->use_ = 0;
  this->direction_ = 0;
  this->layer_ = 0;
  this->pinNameLength_ = 0;
  this->netNameLength_ = 0;
  this->useLength_ = 0;
  this->directionLength_ = 0;
  this->layerLength_ = 0;
  this->defiPin::clear();

  // 5.4
  if (this->APinPartialMetalArea_)
     defFree((char*)(this->APinPartialMetalArea_));
  if (this->APinPartialMetalAreaLayer_)
     defFree((char*)(this->APinPartialMetalAreaLayer_));
  if (this->APinPartialMetalSideArea_)
     defFree((char*)(this->APinPartialMetalSideArea_));
  if (this->APinPartialMetalSideAreaLayer_)
     defFree((char*)(this->APinPartialMetalSideAreaLayer_));
  if (this->APinDiffArea_)
     defFree((char*)(this->APinDiffArea_));
  if (this->APinDiffAreaLayer_)
     defFree((char*)(this->APinDiffAreaLayer_));
  if (this->APinPartialCutArea_)
     defFree((char*)(this->APinPartialCutArea_));
  if (this->APinPartialCutAreaLayer_)
     defFree((char*)(this->APinPartialCutAreaLayer_));
  if (this->antennaModel_)
     defFree((char*)(this->antennaModel_));
}


void defiPin::Setup(const char* pinName, const char* netName) {
  int len = strlen(pinName) + 1;
  if (this->pinNameLength_ < len) {
    if (this->pinName_) defFree(this->pinName_);
    this->pinName_ = (char*)defMalloc(len);
    this->pinNameLength_ = len;
  }
  strcpy(this->pinName_, DEFCASE(pinName));

  len = strlen(netName) + 1;
  if (this->netNameLength_ < len) {
    if (this->netName_) defFree(this->netName_);
    this->netName_ = (char*)defMalloc(len);
    this->netNameLength_ = len;
  }
  strcpy(this->netName_, DEFCASE(netName));

  this->defiPin::clear();

}


void defiPin::setDirection(const char* dir) {
  int len = strlen(dir) + 1;
  if (this->directionLength_ < len) {
    if (this->direction_) defFree(this->direction_);
    this->direction_ = (char*)defMalloc(len);
    this->directionLength_ = len;
  }
  strcpy(this->direction_, DEFCASE(dir));
  this->hasDirection_ = 1;
}


void defiPin::setUse(const char* use) {
  int len = strlen(use) + 1;
  if (this->useLength_ < len) {
    if (this->use_) defFree(this->use_);
    this->use_ = (char*)defMalloc(len);
    this->useLength_ = len;
  }
  strcpy(this->use_, DEFCASE(use));
  this->hasUse_ = 1;
}


void defiPin::setLayer(const char* layer, int xl, int yl, int xh, int yh) {
  int len = strlen(layer) + 1;
  if (this->layerLength_ < len) {
    if (this->layer_) defFree(this->layer_);
    this->layer_ = (char*)defMalloc(len);
    this->layerLength_ = len;
  }
  strcpy(this->layer_, DEFCASE(layer));
  this->hasLayer_ = 1;
  this->xl_ = xl;
  this->yl_ = yl;
  this->xh_ = xh;
  this->yh_ = yh;
}


void defiPin::setPlacement(int typ, int x, int y, int orient) {
  this->x_ = x;
  this->y_ = y;
  this->orient_ = orient;
  this->placeType_ = typ;
}



const char* defiPin::pinName() const {
  return this->pinName_;
}


const char* defiPin::netName() const {
  return this->netName_;
}


int defiPin::hasDirection() const {
  return (int)(this->hasDirection_);
}


int defiPin::hasUse() const {
  return (int)(this->hasUse_);
}


int defiPin::hasLayer() const {
  return (int)(this->hasLayer_);
}


int defiPin::hasPlacement() const {
  return this->placeType_ == 0 ? 0 : 1 ;
}


int defiPin::isUnplaced() const {
  return this->placeType_ == DEFI_COMPONENT_UNPLACED ? 1 : 0 ;
}


int defiPin::isPlaced() const {
  return this->placeType_ == DEFI_COMPONENT_PLACED ? 1 : 0 ;
}


int defiPin::isCover() const {
  return this->placeType_ == DEFI_COMPONENT_COVER ? 1 : 0 ;
}


int defiPin::isFixed() const {
  return this->placeType_ == DEFI_COMPONENT_FIXED ? 1 : 0 ;
}


int defiPin::placementX() const {
  return this->x_;
}


int defiPin::placementY() const {
  return this->y_;
}


const char* defiPin::direction() const {
  return this->direction_;
}


const char* defiPin::use() const {
  return this->use_;
}


const char* defiPin::layer() const {
  return this->layer_;
}


void defiPin::bounds(int* xl, int* yl, int* xh, int* yh) const {
  if (xl) *xl = this->xl_;
  if (yl) *yl = this->yl_;
  if (xh) *xh = this->xh_;
  if (yh) *yh = this->yh_;
}


int defiPin::orient() const {
  return this->orient_;
}


const char* defiPin::orientStr() const {
  return (defiOrientStr(this->orient_));
}


void defiPin::setSpecial() {
  this->hasSpecial_ = 1;
}

// 5.5
void defiPin::addAntennaModel(int oxide) {
  // For version 5.5 only OXIDE1, OXIDE2, OXIDE3, & OXIDE4
  // are defined within a pin
  defiPinAntennaModel* amo;
  int i;

  if (this->numAntennaModel_ == 0) {   // does not have antennaModel
     if (!this->antennaModel_)         // only need to malloc if it is nill
        this->antennaModel_ = (defiPinAntennaModel**)
                 defMalloc(sizeof(defiPinAntennaModel*)*4);
     this->antennaModelAllocated_ = 4;
     for (i = 0; i < 4; i++) {
        this->antennaModel_[i] = (defiPinAntennaModel*)
                                 defMalloc(sizeof(defiPinAntennaModel));
     }
     this->numAntennaModel_++;
     this->antennaModelAllocated_ = 4;
     amo = this->antennaModel_[0];
  } else {
     amo = this->antennaModel_[this->numAntennaModel_];
     this->numAntennaModel_++;
  }
  amo->defiPinAntennaModel::Init();
  amo->defiPinAntennaModel::setAntennaModel(oxide);
  return;
}

// 5.5
int defiPin::numAntennaModel() const {
  return this->numAntennaModel_;
}

// 5.5
defiPinAntennaModel* defiPin::antennaModel(int index) const {
  return this->antennaModel_[index];
}

void defiPin::addAPinPartialMetalArea(int value, const char* layer) {
  if (this->numAPinPartialMetalArea_ == this->APinPartialMetalAreaAllocated_) {
     int i;
     int max;
     int lim = this->numAPinPartialMetalArea_;
     int* nd;
     char**  nl;

     if (this->APinPartialMetalAreaAllocated_ == 0)
        max = this->APinPartialMetalAreaAllocated_ = 2;
     else
        max = this->APinPartialMetalAreaAllocated_ *= 2;
     nd = (int*)defMalloc(sizeof(int)*max);
     nl = (char**)defMalloc(sizeof(char*)*max);
     for (i = 0; i < lim; i++) {
        nd[i] = this->APinPartialMetalArea_[i];
        nl[i] = this->APinPartialMetalAreaLayer_[i];
     }
     defFree((char*)(this->APinPartialMetalArea_));
     defFree((char*)(this->APinPartialMetalAreaLayer_));
     this->APinPartialMetalArea_ = nd;
     this->APinPartialMetalAreaLayer_ = nl;
 
  }
  this->APinPartialMetalArea_[this->numAPinPartialMetalArea_] = value;
  if (layer) {
    this->APinPartialMetalAreaLayer_[this->numAPinPartialMetalArea_] =
       (char*)defMalloc(strlen(layer)+1);
    strcpy(this->APinPartialMetalAreaLayer_[this->numAPinPartialMetalArea_],
       DEFCASE(layer));
  } else
    this->APinPartialMetalAreaLayer_[this->numAPinPartialMetalArea_] = NULL;
  this->numAPinPartialMetalArea_ += 1;
}


void defiPin::addAPinPartialMetalSideArea(int value, const char* layer) {
  if (this->numAPinPartialMetalSideArea_ == this->APinPartialMetalSideAreaAllocated_) {
     int i;
     int max;
     int lim = this->numAPinPartialMetalSideArea_;
     int* nd;
     char**  nl;

     if (this->APinPartialMetalSideAreaAllocated_ == 0)
        max = this->APinPartialMetalSideAreaAllocated_ = 2;
     else
        max = this->APinPartialMetalSideAreaAllocated_ *= 2;
     nd = (int*)defMalloc(sizeof(int)*max);
     nl = (char**)defMalloc(sizeof(char*)*max);
     for (i = 0; i < lim; i++) {
        nd[i] = this->APinPartialMetalSideArea_[i];
        nl[i] = this->APinPartialMetalSideAreaLayer_[i];
     }
     defFree((char*)(this->APinPartialMetalSideArea_));
     defFree((char*)(this->APinPartialMetalSideAreaLayer_));
     this->APinPartialMetalSideArea_ = nd;
     this->APinPartialMetalSideAreaLayer_ = nl;

  }
  this->APinPartialMetalSideArea_[this->numAPinPartialMetalSideArea_] = value;
  if (layer) {
    this->APinPartialMetalSideAreaLayer_[this->numAPinPartialMetalSideArea_] =
       (char*)defMalloc(strlen(layer)+1);
    strcpy(this->APinPartialMetalSideAreaLayer_[this->numAPinPartialMetalSideArea_],
       DEFCASE(layer));
  } else
    this->APinPartialMetalSideAreaLayer_[this->numAPinPartialMetalSideArea_] = NULL;
  this->numAPinPartialMetalSideArea_ += 1;
}


void defiPin::addAPinGateArea(int value, const char* layer) {
  if (this->numAntennaModel_ == 0)    // haven't created any antennaModel yet
     this->defiPin::addAntennaModel(1);
  this->antennaModel_[this->numAntennaModel_-1]->defiPinAntennaModel::addAPinGateArea(value, layer);
}


void defiPin::addAPinDiffArea(int value, const char* layer) {
  if (this->numAPinDiffArea_ == this->APinDiffAreaAllocated_) {
     int i;
     int max;
     int lim = this->numAPinDiffArea_;
     int* nd;
     char**  nl;

     if (this->APinDiffAreaAllocated_ == 0)
        max = this->APinDiffAreaAllocated_ = 2;
     else
        max = this->APinDiffAreaAllocated_ *= 2;
     nd = (int*)defMalloc(sizeof(int)*max);
     nl = (char**)defMalloc(sizeof(char*)*max);
     for (i = 0; i < lim; i++) {
        nd[i] = this->APinDiffArea_[i];
        nl[i] = this->APinDiffAreaLayer_[i];
     }
     defFree((char*)(this->APinDiffArea_));
     defFree((char*)(this->APinDiffAreaLayer_));
     this->APinDiffArea_ = nd;
     this->APinDiffAreaLayer_ = nl;

  }
  this->APinDiffArea_[this->numAPinDiffArea_] = value;
  if (layer) {
    this->APinDiffAreaLayer_[this->numAPinDiffArea_] =
       (char*)defMalloc(strlen(layer)+1);
    strcpy(this->APinDiffAreaLayer_[this->numAPinDiffArea_],
       DEFCASE(layer));
  } else
    this->APinDiffAreaLayer_[this->numAPinDiffArea_] = NULL;
  this->numAPinDiffArea_ += 1;
}


void defiPin::addAPinMaxAreaCar(int value, const char* layer) {
  if (this->numAntennaModel_ == 0)    // haven't created any antennaModel yet
     this->defiPin::addAntennaModel(1);
  this->antennaModel_[this->numAntennaModel_-1]->defiPinAntennaModel::addAPinMaxAreaCar(value, layer);
}


void defiPin::addAPinMaxSideAreaCar(int value, const char* layer) {
  if (this->numAntennaModel_ == 0)    // haven't created any antennaModel yet
     this->defiPin::addAntennaModel(1);
  this->antennaModel_[this->numAntennaModel_-1]->defiPinAntennaModel::addAPinMaxSideAreaCar(value, layer);
}


void defiPin::addAPinPartialCutArea(int value, const char* layer) {
  if (this->numAPinPartialCutArea_ == this->APinPartialCutAreaAllocated_) {
     int i;
     int max;
     int lim = this->numAPinPartialCutArea_;
     int* nd;
     char**  nl;

     if (this->APinPartialCutAreaAllocated_ == 0)
        max = this->APinPartialCutAreaAllocated_ = 2;
     else
        max = this->APinPartialCutAreaAllocated_ *= 2;
     nd = (int*)defMalloc(sizeof(int)*max);
     nl = (char**)defMalloc(sizeof(char*)*max);
     for (i = 0; i < lim; i++) {
        nd[i] = this->APinPartialCutArea_[i];
        nl[i] = this->APinPartialCutAreaLayer_[i];
     }
     defFree((char*)(this->APinPartialCutArea_));
     defFree((char*)(this->APinPartialCutAreaLayer_));
     this->APinPartialCutArea_ = nd;
     this->APinPartialCutAreaLayer_ = nl;

  }
  this->APinPartialCutArea_[this->numAPinPartialCutArea_] = value;
  if (layer) {
    this->APinPartialCutAreaLayer_[this->numAPinPartialCutArea_] =
       (char*)defMalloc(strlen(layer)+1);
    strcpy(this->APinPartialCutAreaLayer_[this->numAPinPartialCutArea_],
       DEFCASE(layer));
  } else
    this->APinPartialCutAreaLayer_[this->numAPinPartialCutArea_] = NULL;
  this->numAPinPartialCutArea_ += 1;
}


void defiPin::addAPinMaxCutCar(int value, const char* layer) {
  if (this->numAntennaModel_ == 0)    // haven't created any antennaModel yet
     this->defiPin::addAntennaModel(1);
  this->antennaModel_[this->numAntennaModel_-1]->defiPinAntennaModel::addAPinMaxCutCar(value, layer);
}


int defiPin::hasSpecial() const {
  return (int)this->hasSpecial_;
}


int defiPin::hasAPinPartialMetalArea() const {
  return this->numAPinPartialMetalArea_ ? 1 : 0 ;
}


int defiPin::hasAPinPartialMetalSideArea() const {
  return this->numAPinPartialMetalSideArea_ ? 1 : 0 ;
}


int defiPin::hasAPinDiffArea() const {
  return this->numAPinDiffArea_ ? 1 : 0 ;
}


int defiPin::hasAPinPartialCutArea() const {
  return this->numAPinPartialCutArea_ ? 1 : 0 ;
}


int defiPin::numAPinPartialMetalArea() const {
  return this->numAPinPartialMetalArea_;
}


int defiPin::numAPinPartialMetalSideArea() const {
  return this->numAPinPartialMetalSideArea_;
}


int defiPin::numAPinDiffArea() const {
  return this->numAPinDiffArea_;
}


int defiPin::numAPinPartialCutArea() const {
  return this->numAPinPartialCutArea_;
}


int defiPin::APinPartialMetalArea(int i) const {
  return this->APinPartialMetalArea_[i];
}


const char* defiPin::APinPartialMetalAreaLayer(int i) const {
  return this->APinPartialMetalAreaLayer_[i];
}


int defiPin::APinPartialMetalSideArea(int i) const {
  return this->APinPartialMetalSideArea_[i];
}


const char* defiPin::APinPartialMetalSideAreaLayer(int i) const {
  return this->APinPartialMetalSideAreaLayer_[i];
}


int defiPin::APinDiffArea(int i) const {
  return this->APinDiffArea_[i];
}


const char* defiPin::APinDiffAreaLayer(int i) const {
  return this->APinDiffAreaLayer_[i];
}


int defiPin::APinPartialCutArea(int i) const {
  return this->APinPartialCutArea_[i];
}


const char* defiPin::APinPartialCutAreaLayer(int i) const {
  return this->APinPartialCutAreaLayer_[i];
}


void defiPin::print(FILE* f) const {
  int xl, yl, xh,yh;

  fprintf(f, "Pin '%s' on net '%s'\n", this->defiPin::pinName(),
	    this->defiPin::netName());
  if (this->defiPin::hasDirection())
    fprintf(f, "  direction '%s'\n", this->defiPin::direction());
  if (this->defiPin::hasUse())
    fprintf(f, "  use '%s'\n", this->defiPin::use());
  if (this->defiPin::hasLayer()) {
    this->defiPin::bounds(&xl, &yl, &xh, &yh);
    fprintf(f, "  layer '%s' %d %d %d %d\n",
	       this->defiPin::layer(), xl, yl, xh, yh);
    }
  if (this->defiPin::hasPlacement())
    fprintf(f, "  placed %s%s%d %d\n",
		this->defiPin::isFixed() ? " FIXED" : "",
		this->defiPin::isCover() ? " COVER" : "",
		this->defiPin::placementX(),
		this->defiPin::placementY());
  if (this->defiPin::hasSpecial())
    fprintf(f, "  SPECIAL\n");
}
