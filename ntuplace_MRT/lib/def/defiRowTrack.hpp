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

#ifndef defiRowTrack_h
#define defiRowTrack_h

#include "defiKRDefs.hpp"
#include <stdio.h>


class defiRow{
public:

  defiRow();
  void Init();

  ~defiRow();
  void Destroy();
  void clear();

  void setup(const char* name, const char* macro,
	     double x, double y, int orient);
  void setDo(double x_num, double y_num,
	     double x_step, double y_step);
  void addProperty(const char* name, const char* value, const char type);
  void addNumProperty(const char* name, const double d, 
                      const char* value, const char type);

  const char* name() const;
  const char* macro() const;
  double x() const;
  double y() const;
  int orient() const;
  const char* orientStr() const;
  double xNum() const;
  double yNum() const;
  double xStep() const;
  double yStep() const;

  int numProps() const;
  const char*  propName(int index) const;
  const char*  propValue(int index) const;
  double propNumber(int index) const;
  const char   propType(int index) const;
  int propIsNumber(int index) const;
  int propIsString(int index) const;

  void print(FILE* f) const;

protected:
  int nameLength_;
  char* name_;
  int macroLength_;
  char* macro_;
  double x_;
  double y_;
  double xNum_;
  double yNum_;
  int orient_;
  double xStep_;
  double yStep_;

  int numProps_;
  int propsAllocated_;
  char**  propNames_;
  char**  propValues_;
  double* propDValues_;
  char*   propTypes_;
};



class defiTrack{
public:

  defiTrack();
  void Init();

  ~defiTrack();
  void Destroy();

  void setup(const char* macro);
  void setDo(double x, double x_num, double x_step);
  void addLayer(const char* layer);

  const char* macro() const;
  double x() const;
  double xNum() const;
  double xStep() const;
  int numLayers() const;
  const char* layer(int index) const;

  void print(FILE* f) const;

protected:
  int macroLength_;  // allocated size of macro_;
  char* macro_;
  double x_;
  double xNum_;
  double xStep_;
  int layersLength_;  // allocated size of layers_
  int numLayers_;  // number of places used in layers_
  char** layers_;
};



class defiGcellGrid {
public:

  defiGcellGrid();
  void Init();

  ~defiGcellGrid();
  void Destroy();

  void setup(const char* macro, int x, int xNum, double xStep);

  const char* macro() const;
  int x() const;
  int xNum() const;
  double xStep() const;

  void print(FILE* f) const;

protected:
  int macroLength_;
  char* macro_;
  int x_;
  int xNum_;
  double xStep_;
};


#endif
