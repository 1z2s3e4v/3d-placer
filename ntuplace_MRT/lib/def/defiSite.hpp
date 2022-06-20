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

#ifndef defiSite_h
#define defiSite_h

#include "defiKRDefs.hpp"
#include <stdio.h>

/*
 * Struct holds the data for one site.
 * It is also used for a canplace and cannotoccupy.
 */
class defiSite {
public:
  defiSite();
  void Init();

  ~defiSite();
  void Destroy();

  void clear();

  void setName(const char* name);
  void setLocation(double xorg, double yorg);
  void setOrient(int orient);
  void setDo(double x_num, double y_num, double x_step, double y_step);

  double x_num() const;
  double y_num() const;
  double x_step() const;
  double y_step() const;
  double x_orig() const;
  double y_orig() const;
  int orient() const;
  const char* orientStr() const;
  const char* name() const;

  void print(FILE* f) const;

  void bumpName(int size);

protected:
  char* siteName_;     // Name of this.
  int nameSize_;       // allocated size of siteName_
  double x_orig_, y_orig_;  // Origin
  double x_step_, y_step_;  // Array step size.
  double x_num_, y_num_; 
  int orient_;         // orientation
};



/* Struct holds the data for a Box */
class defiBox {
public:
  // Use the default destructor and constructor.

  void setLowerLeft(int x, int y);
  void setUpperRight(int x, int y);

  int xl() const;
  int yl() const;
  int xh() const;
  int yh() const;

  void print(FILE* f) const;

protected:
  int xl_, yl_;
  int xh_, yh_;
};


#endif
