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

#ifndef defiPinCap_h
#define defiPinCap_h


#include "defiKRDefs.hpp"
#include <stdio.h>

class defiPinCap {
public:

  void setPin(int p);
  void setCap(double d);

  int pin() const;
  double cap() const;

  void print(FILE* f) const;

protected:
  int pin_;        // pin num
  double cap_;     // capacitance
};


// 5.5
class defiPinAntennaModel {
public:
  defiPinAntennaModel();
  void Init();

  ~defiPinAntennaModel();
  void clear();
  void Destroy();

  void setAntennaModel(int oxide);
  void addAPinGateArea(int value, const char* layer);
  void addAPinMaxAreaCar(int value, const char* layer);
  void addAPinMaxSideAreaCar(int value, const char* layer);
  void addAPinMaxCutCar(int value, const char* layer);

  char* antennaOxide() const;

  int hasAPinGateArea() const;               // ANTENNAPINGATEAREA
  int numAPinGateArea() const;
  int APinGateArea(int index) const;
  const char* APinGateAreaLayer(int index) const;

  int hasAPinMaxAreaCar() const;             // ANTENNAPINMAXAREACAR
  int numAPinMaxAreaCar() const;
  int APinMaxAreaCar(int index) const;
  const char* APinMaxAreaCarLayer(int index) const;

  int hasAPinMaxSideAreaCar() const;         // ANTENNAPINMAXSIDEAREACAR
  int numAPinMaxSideAreaCar() const;
  int APinMaxSideAreaCar(int index) const;
  const char* APinMaxSideAreaCarLayer(int index) const;

  int hasAPinMaxCutCar() const;              // ANTENNAPINMAXCUTCAR
  int numAPinMaxCutCar() const;
  int APinMaxCutCar(int index) const;
  const char* APinMaxCutCarLayer(int index) const;

protected:
  char* oxide_;

  int numAPinGateArea_;                  // 5.4
  int APinGateAreaAllocated_;
  int* APinGateArea_;                    // 5.4 AntennaPinGateArea
  char** APinGateAreaLayer_;             // 5.4 Layer

  int numAPinMaxAreaCar_;                // 5.4
  int APinMaxAreaCarAllocated_;
  int* APinMaxAreaCar_;                  // 5.4 AntennaPinMaxAreaCar
  char** APinMaxAreaCarLayer_;           // 5.4 Layer

  int numAPinMaxSideAreaCar_;            // 5.4
  int APinMaxSideAreaCarAllocated_;
  int* APinMaxSideAreaCar_;              // 5.4 AntennaPinMaxSideAreaCar
  char** APinMaxSideAreaCarLayer_;       // 5.4 Layer

  int numAPinMaxCutCar_;                 // 5.4
  int APinMaxCutCarAllocated_;
  int* APinMaxCutCar_;                   // 5.4 AntennaPinMaxCutCar
  char** APinMaxCutCarLayer_;            // 5.4 Layer
};


class defiPin {
public:
  defiPin();
  void Init();

  ~defiPin();
  void Destroy();

  void Setup(const char* pinName, const char* netName);
  void setDirection(const char* dir);
  void setUse(const char* use);
  void setLayer(const char* layer, int xl, int yl, int xh, int yh);
  void setPlacement(int typ, int x, int y, int orient);
  void setSpecial();
  void addAntennaModel(int oxide);           // 5.5
  void addAPinPartialMetalArea(int value, const char* layer);
  void addAPinPartialMetalSideArea(int value, const char* layer);
  void addAPinGateArea(int value, const char* layer);
  void addAPinDiffArea(int value, const char* layer);
  void addAPinMaxAreaCar(int value, const char* layer);
  void addAPinMaxSideAreaCar(int value, const char* layer);
  void addAPinPartialCutArea(int value, const char* layer);
  void addAPinMaxCutCar(int value, const char* layer);
  void addPinPort(const char* name);
  void setPinPortConnect(int type, int x, int y, int orient);
  void setPinPortLayer(const char* layerName, int xl, int yl, int xh, int yh);
  void addPinPortAPortPartialMetalArea(int value, const char* layer);
  void addPinPortAPortPartialMetalSideArea(int value, const char* layer);
  void addPinPortAPortGateArea(int value, const char* layer);
  void addPinPortAPortDiffArea(int value, const char* layer);
  void addPinPortAPortMaxAreaCar(int value, const char* layer);
  void addPinPortAPortMaxSideAreaCar(int value, const char* layer);
  void addPinPortAPortPartialCutArea(int value, const char* layer);
  void addPinPortAPortMaxCutCar(int value, const char* layer);
  void clear();

  const char* pinName() const;
  const char* netName() const;

  // optional parts
  int hasDirection() const;
  int hasUse() const;
  int hasLayer() const;
  int hasPlacement() const;
  int isUnplaced() const;
  int isPlaced() const;
  int isCover() const;
  int isFixed() const;
  int placementX() const;
  int placementY() const;
  const char* direction() const;
  const char* use() const;
  const char* layer() const;
  void bounds(int* xl, int* yl, int* xh, int* yh) const;
  int orient() const;
  const char* orientStr() const;
  int hasSpecial() const;

  // 5.4
  int hasAPinPartialMetalArea() const;       // ANTENNAPINPARTIALMETALAREA
  int numAPinPartialMetalArea() const;
  int APinPartialMetalArea(int index) const;
  const char* APinPartialMetalAreaLayer(int index) const;

  int hasAPinPartialMetalSideArea() const;   // ANTENNAPINPARTIALMETALSIDEAREA
  int numAPinPartialMetalSideArea() const;
  int APinPartialMetalSideArea(int index) const;
  const char* APinPartialMetalSideAreaLayer(int index) const;

  int hasAPinDiffArea() const;               // ANTENNAPINDIFFAREA
  int numAPinDiffArea() const;
  int APinDiffArea(int index) const;
  const char* APinDiffAreaLayer(int index) const;

  int hasAPinPartialCutArea() const;         // ANTENNAPINPARTIALCUTAREA
  int numAPinPartialCutArea() const;
  int APinPartialCutArea(int index) const;
  const char* APinPartialCutAreaLayer(int index) const;

  // 5.5
  int numAntennaModel() const;
  defiPinAntennaModel* antennaModel(int index) const;

  int  numPorts() const;
  void print(FILE* f) const;

protected:
  int pinNameLength_;    // allocated size of pin name
  char* pinName_;
  int netNameLength_;    // allocated size of net name
  char* netName_;
  char hasDirection_;
  char hasUse_;
  char hasLayer_;
  char placeType_;
  char orient_;          // orient 0-7
  int useLength_;        // allocated size of length
  char* use_;
  int directionLength_;  // allocated size of direction
  char* direction_;
  int layerLength_;      // allocated size of layer
  char* layer_;
  int xl_, yl_, xh_, yh_;
  int x_, y_;            // placement
  int hasSpecial_;

  // 5.5 AntennaModel
  int numAntennaModel_;
  int antennaModelAllocated_;
  defiPinAntennaModel** antennaModel_;

  int numAPinPartialMetalArea_;          // 5.4
  int APinPartialMetalAreaAllocated_;
  int* APinPartialMetalArea_;            // 5.4 AntennaPinPartialMetalArea
  char** APinPartialMetalAreaLayer_;     // 5.4 Layer

  int numAPinPartialMetalSideArea_;      // 5.4
  int APinPartialMetalSideAreaAllocated_;
  int* APinPartialMetalSideArea_;        // 5.4 AntennaPinPartialMetalSideArea
  char** APinPartialMetalSideAreaLayer_; // 5.4 Layer

  int numAPinDiffArea_;                  // 5.4
  int APinDiffAreaAllocated_;
  int* APinDiffArea_;                    // 5.4 AntennaPinDiffArea
  char** APinDiffAreaLayer_;             // 5.4 Layer

  int numAPinPartialCutArea_;            // 5.4
  int APinPartialCutAreaAllocated_;
  int* APinPartialCutArea_;              // 5.4 AntennaPinPartialCutArea
  char** APinPartialCutAreaLayer_;       // 5.4 Layer
};

#endif
