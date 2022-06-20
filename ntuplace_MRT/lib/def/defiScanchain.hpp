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

#ifndef defiScanchain_h
#define defiScanchain_h

#include "defiKRDefs.hpp"
#include <stdio.h>

class defiOrdered {
public:
  void addOrdered(const char* inst);
  void addIn(const char* pin);
  void addOut(const char* pin);
  void setOrderedBits(int bits);        // 5.4.1
  void bump();
  void Init();
  void Destroy();
  void clear();

  int num() const;
  char** inst() const;
  char** in() const;
  char** out() const;
  int*   bits() const;                  // 5.4.1

protected:
  int num_;
  int allocated_;
  char** inst_;
  char** in_;
  char** out_;
  int*   bits_;                       // 5.4.1
};


// Struct holds the data for one Scan chain.
//
class defiScanchain {
public:
  defiScanchain();
  void Init();

  void Destroy();
  ~defiScanchain();

  void setName(const char* name);
  void clear();

  void addOrderedList();
  void addOrderedInst(const char* inst);
  void addOrderedIn(const char* inPin);
  void addOrderedOut(const char* outPin);
  void setOrderedBits(int bits);      // 5.4.1

  void addFloatingInst(const char* inst);
  void addFloatingIn(const char* inPin);
  void addFloatingOut(const char* outPin);
  void setFloatingBits(int bits);     // 5.4.1

  void setStart(const char* inst, const char* pin);
  void setStop(const char* inst, const char* pin);
  void setCommonIn(const char* pin);
  void setCommonOut(const char* pin);
  void setPartition(const char* partName, int maxBits);    // 5.4.1

  const char* name() const;
  int hasStart() const;
  int hasStop() const;
  int hasFloating() const;
  int hasOrdered() const;
  int hasCommonInPin() const;
  int hasCommonOutPin() const;
  int hasPartition() const;           // 5.4.1
  int hasPartitionMaxBits() const;    // 5.4.1

  // If the pin part of these routines were not supplied in the DEF
  // then a NULL pointer will be returned.
  void start(char** inst, char** pin) const;
  void stop(char** inst, char** pin) const;

  // There could be many ORDERED constructs in the DEF.  The data in
  // each ORDERED construct is stored in its own array.  The numOrderedLists()
  // routine tells how many lists there are.
  int numOrderedLists() const;

  // This routine will return an array of instances and
  // an array of in and out pins.
  // The number if things in the arrays is returned in size.
  // The inPin and outPin entry is optional for each instance.
  // If an entry is not given, then that char* is NULL.
  // For example if the second instance has
  // instnam= "FOO" and IN="A", but no OUT given, then inst[1] points
  // to "FOO"  inPin[1] points to "A" and outPin[1] is a NULL pointer.
  void ordered(int index, int* size, char*** inst, char*** inPin,
                                      char*** outPin, int** bits) const;

  // All of the floating constructs in the scan chain are
  // stored in this one array.
  // If the IN or OUT of an entry is not supplied then the array will have
  // a NULL pointer in that place.
  void floating(int* size, char*** inst, char*** inPin, char*** outPin,
                                      int** bits) const;

  const char* commonInPin() const;
  const char* commonOutPin() const;

  const char* partitionName() const;        // 5.4.1
  int partitionMaxBits() const;             // 5.4.1

  void print(FILE* f) const;

protected:
  char* name_;
  char hasStart_;
  char hasStop_;
  int nameLength_;

  int numOrderedAllocated_;
  int numOrdered_;
  defiOrdered** ordered_; 

  int numFloatingAllocated_;
  int numFloating_;
  char** floatInst_;    // Array of floating names
  char** floatIn_;
  char** floatOut_;
  int*   floatBits_;    // 5.4.1

  char* stopInst_;
  char* stopPin_;

  char* startInst_;
  char* startPin_;

  char* commonInPin_;
  char* commonOutPin_;

  char  hasPartition_;  // 5.4.1
  char* partName_;      // 5.4.1
  int   maxBits_;       // 5.4.1

};


#endif
