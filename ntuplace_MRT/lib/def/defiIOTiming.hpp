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

#ifndef defiIOTiming_h
#define defiIOTiming_h

#include <stdio.h>
#include "defiKRDefs.hpp"

class defiIOTiming {
public:
  defiIOTiming();
  void Init();

  void Destroy();
  ~defiIOTiming();

  void clear();

  void setName(const char* inst, const char* pin);
  void setVariable(const char* riseFall, double min, double max);
  void setSlewRate(const char* riseFall, double min, double max);
  void setCapacitance(double num);
  void setDriveCell(const char* name);
  void setFrom(const char* name);
  void setTo(const char* name);
  void setParallel(double num);


  int hasVariableRise() const;
  int hasVariableFall() const;
  int hasSlewRise() const;
  int hasSlewFall() const;
  int hasCapacitance() const;
  int hasDriveCell() const;
  int hasFrom() const;
  int hasTo() const;
  int hasParallel() const;

  const char* inst() const;
  const char* pin() const;
  double variableFallMin() const;
  double variableRiseMin() const;
  double variableFallMax() const;
  double variableRiseMax() const;
  double slewFallMin() const;
  double slewRiseMin() const;
  double slewFallMax() const;
  double slewRiseMax() const;
  double capacitance() const;
  const char* driveCell() const;
  const char* from() const;
  const char* to() const;
  double parallel() const;

  // debug print
  void print(FILE* f) const;

protected:
  char* inst_;
  int instLength_;
  char* pin_;
  int pinLength_;
  char* from_;
  int fromLength_;
  char* to_;
  int toLength_;
  char* driveCell_;
  char driveCellLength_;
  char hasVariableRise_;
  char hasVariableFall_;
  char hasSlewRise_;
  char hasSlewFall_;
  char hasCapacitance_;
  char hasDriveCell_;
  char hasFrom_;
  char hasTo_;
  char hasParallel_;
  double variableFallMin_;
  double variableRiseMin_;
  double variableFallMax_;
  double variableRiseMax_;
  double slewFallMin_;
  double slewRiseMin_;
  double slewFallMax_;
  double slewRiseMax_;
  double capacitance_;
  double parallel_;
};


#endif
