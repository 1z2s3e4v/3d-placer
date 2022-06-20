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

#ifndef defiALIAS_h
#define defiALIAS_h

#include <stdio.h>
#include "defiKRDefs.hpp"

class defiAlias_itr {
public:
  defiAlias_itr();
  void Init();

  void Destroy();
  ~defiAlias_itr();

  int Next();
  const char* Key();
  const char* Data();
  int Marked();

protected:
  int num_;
};


#endif
