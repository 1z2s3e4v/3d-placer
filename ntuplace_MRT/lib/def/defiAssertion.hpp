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

#ifndef defiAssertion_h
#define defiAssertion_h

#include "defiKRDefs.hpp"
#include <stdio.h>

// Struct holds the data for one assertion/constraint.
// An assertion or constraint is either a net/path rule or a
// wired logic rule.
//
//  A net/path rule is an item or list of items plus specifications.
//    The specifications are: rise/fall min/max.
//    The items are a list of (one or more) net names or paths or a
//    combination of both.
//
//  A wired logic rule is a netname and a distance.
//
//  We will NOT allow the mixing of wired logic rules and net/path delays
//  in the same assertion/constraint.
//
//  We will allow the rule to be a sum of sums (which will be interpreted
//  as just one list).
//
class defiAssertion {
public:
  defiAssertion();
  void Init();

  void Destroy();
  ~defiAssertion();

  void setConstraintMode();
  void setAssertionMode();
  void setSum();
  void setDiff();
  void setNetName(const char* name);
  void setRiseMin(double num);
  void setRiseMax(double num);
  void setFallMin(double num);
  void setFallMax(double num);
  void setDelay();
  void setWiredlogicMode();
  void setWiredlogic(const char* net, double dist);
  void addNet(const char* name);
  void addPath(const char* fromInst, const char* fromPin,
               const char* toInst, const char* toPin);
  void bumpItems();
  void unsetSum();

  int isAssertion() const;  // Either isAssertion or isConstraint is true
  int isConstraint() const;
  int isWiredlogic() const; // Either isWiredlogic or isDelay is true
  int isDelay() const;
  int isSum() const;
  int isDiff() const;
  int hasRiseMin() const;
  int hasRiseMax() const;
  int hasFallMin() const;
  int hasFallMax() const;
  double riseMin() const;
  double riseMax() const;
  double fallMin() const;
  double fallMax() const;
  const char* netName() const; // Wired logic net name
  double distance() const; // Wired logic distance
  int numItems() const;  // number of paths or nets 
  int isPath(int index) const;   // is item #index a path?
  int isNet(int index) const;    // is item #index a net?
  void path(int index, char** fromInst, char** fromPin,
	   char** toInst, char** toPin) const; // Get path data for item #index
  void net(int index, char** netName) const;   // Get net data for item #index

  void clear();
  void print(FILE* f) const;


protected:
  char isAssertion_;
  char isSum_;
  char isDiff_;
  char hasRiseMin_;
  char hasRiseMax_;
  char hasFallMin_;
  char hasFallMax_;
  char isWiredlogic_;
  char isDelay_;
  char* netName_;     // wired logic net name
  int netNameLength_;
  double riseMin_;
  double riseMax_;
  double fallMin_;
  double fallMax_;    // also used to store the wired logic dist
  int numItems_;
  int numItemsAllocated_;
  char* itemTypes_;
  int** items_;       // not really integers.

};


#endif
