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

#ifndef defiPartition_h
#define defiPartition_h

#include <stdio.h>
#include "defiKRDefs.hpp"

class defiPartition {
public:
  defiPartition();
  void Init();

  void Destroy();
  ~defiPartition();

  void clear();

  void setName(const char* name);
  void addTurnOff(const char* setup, const char* hold);
  void setFromClockPin(const char* inst, const char* pin);
  void setFromCompPin(const char* inst, const char* pin);
  void setFromIOPin(const char* inst);
  void setToClockPin(const char* inst, const char* pin);
  void setToCompPin(const char* inst, const char* pin);
  void set(char dir, char typ, const char* inst, const char* pin);
  void setToIOPin(const char* inst);
  void setMin(double min, double max);
  void setMax(double min, double max);
  void addPin(const char* name);
  void addRiseMin(double d);
  void addRiseMax(double d);
  void addFallMin(double d);
  void addFallMax(double d);
  void addRiseMinRange(double l, double h);
  void addRiseMaxRange(double l, double h);
  void addFallMinRange(double l, double h);
  void addFallMaxRange(double l, double h);

  const char* name() const;
  char direction() const;
  const char* itemType() const;  // "CLOCK" or "IO" or "COMP"
  const char* pinName() const;
  const char* instName() const;

  int numPins() const;
  const char* pin(int index) const;

  int isSetupRise() const;
  int isSetupFall() const;
  int isHoldRise() const;
  int isHoldFall() const;
  int hasMin() const;
  int hasMax() const;
  int hasRiseMin() const;
  int hasFallMin() const;
  int hasRiseMax() const;
  int hasFallMax() const;
  int hasRiseMinRange() const;
  int hasFallMinRange() const;
  int hasRiseMaxRange() const;
  int hasFallMaxRange() const;

  double partitionMin() const;
  double partitionMax() const;

  double riseMin() const;
  double fallMin() const;
  double riseMax() const;
  double fallMax() const;

  double riseMinLeft() const;
  double fallMinLeft() const;
  double riseMaxLeft() const;
  double fallMaxLeft() const;
  double riseMinRight() const;
  double fallMinRight() const;
  double riseMaxRight() const;
  double fallMaxRight() const;

  // debug print
  void print(FILE* f) const;

protected:
  char* name_;
  int nameLength_;
  char setup_;
  char hold_;
  char hasMin_;
  char hasMax_;
  char direction_;   // 'F' or 'T'
  char type_;        // 'L'-clock   'I'-IO  'C'-comp
  char* inst_;
  int instLength_;
  char* pin_;
  int pinLength_;
  double min_, max_;

  int numPins_;
  int pinsAllocated_;
  char** pins_;

  char hasRiseMin_;
  char hasFallMin_;
  char hasRiseMax_;
  char hasFallMax_;
  char hasRiseMinRange_;
  char hasFallMinRange_;
  char hasRiseMaxRange_;
  char hasFallMaxRange_;
  double riseMin_;
  double fallMin_;
  double riseMax_;
  double fallMax_;
  double riseMinLeft_;
  double fallMinLeft_;
  double riseMaxLeft_;
  double fallMaxLeft_;
  double riseMinRight_;
  double fallMinRight_;
  double riseMaxRight_;
  double fallMaxRight_;
};


#endif
