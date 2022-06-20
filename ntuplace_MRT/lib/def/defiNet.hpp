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

#ifndef defiNet_h
#define defiNet_h

#include <stdio.h>
#include "defiKRDefs.hpp"
#include "defiPath.hpp"


class defiWire {
public:
  defiWire();
  ~defiWire();

  void Init(const char* type, const char* wireShieldName);
  void Destroy();
  void clear();
  void addPath(defiPath *p);

  const char* wireType() const;
  const char* wireShieldNetName() const;
  int         numPaths() const;
  defiPath*   path(int index);

  void bumpPaths(int size);

protected:
  char*      type_;
  char*      wireShieldName_;    // It only set from specialnet SHIELD, 5.4
  int        numPaths_;
  int        pathsAllocated_;
  defiPath** paths_;
};



class defiSubnet {
public:
  defiSubnet();
  void Init();

  void Destroy();
  ~defiSubnet();

  void setName(const char* name);
  void setNonDefault(const char* name);
  void addPin(const char* instance, const char* pin, int syn);
  void addMustPin(const char* instance, const char* pin, int syn);

  // WMD -- the following will be removed by the next release
  void setType(const char* typ);  // Either FIXED COVER ROUTED
  void addPath(defiPath* p);

  // NEW: a net can have more than 1 wire
  void addWire(const char *typ); 
  void addWirePath(defiPath *p);

  // Debug printing
  void print(FILE* f);

  const char* name() const;
  int numConnections();
  const char* instance(int index);
  const char* pin(int index);
  int pinIsSynthesized(int index);
  int pinIsMustJoin(int index);

  // WMD -- the following will be removed by the next release
  int isFixed() const;
  int isRouted() const;
  int isCover() const;

  int hasNonDefaultRule() const;
  int hasShield() const;
  int hasShieldNet() const;
  int hasNoShieldNet() const;   // this is a pre 5.4 routine

  // WMD -- the following will be removed by the next release
  int numPaths() const;
  defiPath* path(int index);

  const char* nonDefaultRule() const;

  int         numWires() const;
  defiWire*   wire(int index);

  void bumpName(int size);
  void bumpPins(int size);
  void bumpPaths(int size);
  void clear();

protected:
  char* name_;            // name.
  int nameSize_;          // allocated size of name.
  int numPins_;           // number of pins used in array.
  int pinsAllocated_;     // number of pins allocated in array.
  char** instances_;      // instance names for connections
  char** pins_;           // pin names for connections
  char* synthesized_;     // synthesized flags for pins
  char* musts_;           // must-join flags

  // WMD -- the following will be removed by the next release
  char isFixed_;        // net type
  char isRouted_;
  char isCover_;
  defiPath** paths_;   // paths for this subnet
  int numPaths_;       // number of paths used
  int pathsAllocated_; // allocated size of paths array

  int numWires_;          // number of wires defined in the subnet
  int wiresAllocated_;    // number of wires allocated in the subnet
  defiWire** wires_;      // this replace the paths
  char* nonDefaultRule_;
};



class defiVpin {
public:
  defiVpin();
  ~defiVpin();

  void Init(const char* name);
  void Destroy();
  void setLayer(const char* name);
  void setBounds(int xl, int yl, int xh, int yh);
  void setOrient(int orient);
  void setLoc(int x, int y);
  void setStatus(char st);

  int xl() const ;
  int yl() const ;
  int xh() const ;
  int yh() const ;
  char status() const;      /* P-placed, F-fixed, C-cover, ' ' - not set */
  int orient() const ;
  const char* orientStr() const ;
  int xLoc() const;
  int yLoc() const;
  const char* name() const;
  const char* layer() const;

protected:
  int xl_;
  int yl_;
  int xh_;
  int yh_;
  int orient_;  /* 0-7  -1 is no orient */
  char status_; /* P-placed  F-fixed  C-cover  ' '- none */
  int xLoc_;
  int yLoc_;
  char* name_;
  char* layer_;
};



// Pre 5.4
class defiShield {
public:
  defiShield();
  ~defiShield();

  void Init(const char* name);
  void Destroy();
  void clear();
  void addPath(defiPath *p);

  const char* shieldName() const;
  int         numPaths() const;
  defiPath*   path(int index);

  void bumpPaths(int size);

protected:
  char*      name_;
  int        numPaths_;
  int        pathsAllocated_;
  defiPath** paths_;
};




// Struct holds the data for one component.
class defiNet {
public:
  defiNet();
  void Init();

  void Destroy();
  ~defiNet();

  // Routines used by YACC to set the fields in the net.
  void setName(const char* name);
  void addPin(const char* instance, const char* pin, int syn);
  void addMustPin(const char* instance, const char* pin, int syn);
  void setWeight(int w);

  // WMD -- the following will be removed by the next release
  void setType(const char* typ);  // Either FIXED COVER ROUTED

  void addProp(const char* name, const char* value, const char type);
  void addNumProp(const char* name, const double d,
                  const char* value, const char type);
  void addSubnet(defiSubnet* subnet);
  // NEW: a net can have more than 1 wire
  void addWire(const char *typ, const char* wireShieldName);
  void addWirePath(defiPath* p);
  void setSource(const char* typ);
  void setFixedbump();                          // 5.4.1
  void setFrequency(double frequency);          // 5.4.1
  void setOriginal(const char* typ);
  void setPattern(const char* typ);
  void setCap(double w);
  void setUse(const char* typ);
  void setNonDefaultRule(const char* typ);
  void setStyle(const char* typ);
  void addShield(const char* shieldNetName);    // pre 5.4
  void addNoShield(const char* shieldNetName);  // pre 5.4
  void addShieldNet(const char* shieldNetName);

  // WMD -- the following will be removed by the next release
  void addPath(defiPath* p);

  void addShieldPath(defiPath* p);
  void clear();
  void setWidth(const char* layer, double dist);
  void setSpacing(const char* layer, double dist);
  void setVoltage(double num);
  void setRange(double left, double right);
  void setXTalk(int num);
  void addVpin(const char* name);
  void addVpinLayer(const char* name);
  void addVpinLoc(const char* status, int x, int y, int orient);
  void addVpinBounds(int xl, int yl, int xh, int yh);

  // Routines to return the value of net data.
  const char*  name() const;
  int          weight() const;
  int          numProps() const;
  const char*  propName(int index) const;
  const char*  propValue(int index) const;
  double propNumber(int index) const;
  const char   propType(int index) const;
  int    propIsNumber(int index) const;
  int    propIsString(int index) const;
  int          numConnections() const;
  const char*  instance(int index) const;
  const char*  pin(int index) const;
  int          pinIsMustJoin(int index) const;
  int          pinIsSynthesized(int index) const;
  int          numSubnets() const;
  defiSubnet*  subnet(int index);

  // WMD -- the following will be removed by the next release
  int         isFixed() const;
  int         isRouted() const;
  int         isCover() const;

  /* The following routines are for wiring */
  int         numWires() const;
  defiWire*   wire(int index);

  /* Routines to get the information about Virtual Pins. */
  int       numVpins() const;
  defiVpin* vpin(int index) const;

  int hasProps() const;
  int hasWeight() const;
  int hasSubnets() const;
  int hasSource() const;
  int hasFixedbump() const;                          // 5.4.1
  int hasFrequency() const;                          // 5.4.1
  int hasPattern() const;
  int hasOriginal() const;
  int hasCap() const;
  int hasUse() const;
  int hasStyle() const;
  int hasNonDefaultRule() const;
  int hasVoltage() const;
  int hasSpacingRules() const;
  int hasWidthRules() const;
  int hasXTalk() const;

  int numSpacingRules() const;
  void spacingRule(int index, char** layer, double* dist, double* left,
                   double* right);
  int numWidthRules() const;
  void widthRule(int index, char** layer, double* dist);
  double voltage() const;

  int            XTalk() const;
  const char*    source() const;
  double         frequency() const;
  const char*    original() const;
  const char*    pattern() const;
  double         cap() const;
  const char*    use() const;
  const char*    style() const;
  const char*    nonDefaultRule() const;

  // WMD -- the following will be removed by the next release
  int            numPaths() const;
  defiPath*      path(int index);

  int            numShields() const;          // pre 5.4
  defiShield*    shield(int index);           // pre 5.4
  int            numShieldNets() const;
  const char*    shieldNet(int index) const;
  int            numNoShields() const;        // pre 5.4
  defiShield*    noShield(int index);         // pre 5.4

  // Debug printing
  void print(FILE* f);


  void bumpName(int size);
  void bumpPins(int size);
  void bumpProps(int size);
  void bumpSubnets(int size);
  void bumpPaths(int size);
  void bumpShieldNets(int size);

protected:
  char* name_;          // name.
  int nameSize_;        // allocated size of name.
  int numPins_;         // number of pins used in array.
  int pinsAllocated_;   // number of pins allocated in array.
  char** instances_;    // instance names for connections
  char** pins_;         // pin names for connections
  char* musts_;         // must-join flags for pins
  char* synthesized_;   // synthesized flags for pins
  int weight_;          // net weight
  char hasWeight_;      // flag for optional weight

  // WMD -- the following will be removed by the nex release
  char isFixed_;        // net type
  char isRouted_;
  char isCover_;

  char hasCap_;         // file supplied a capacitance value
  char hasFrequency_;   // file supplied a frequency value
  char hasVoltage_;
  int numProps_;        // num of props in array
  char**  propNames_;   // Prop names
  char**  propValues_;  // Prop values All in strings!
  double* propDValues_; // Prop values in numbers!
  char*   propTypes_;   // Prop types, 'I' - Integer, 'R' - Real, 'S' - String
  int propsAllocated_;  // allocated size of props array
  int numSubnets_;      // num of subnets in array
  defiSubnet** subnets_; // Prop names
  int subnetsAllocated_; // allocated size of props array
  double cap_;           // cap value
  char* source_;
  int    fixedbump_;     // 5.4.1
  double frequency_;     // 5.4.1
  char* pattern_;
  char* original_;
  char* use_;
  char* nonDefaultRule_;
  char* style_;

  // WMD -- the following will be removed by the nex release
  defiPath** paths_;   // paths for this subnet
  int numPaths_;       // number of paths used
  int pathsAllocated_; // allocated size of paths array

  double voltage_;

  int numWires_;         // number of wires defined in the net
  int wiresAllocated_;   // allocated size of wire paths array
  defiWire** wires_;     // this replace the paths

  int widthsAllocated_;
  int numWidths_;
  char** wlayers_;
  double* wdist_;

  int spacingAllocated_;
  int numSpacing_;
  char** slayers_;
  double* sdist_;
  double* sleft_;
  double* sright_;
  int xTalk_;

  int numVpins_;
  int vpinsAllocated_;
  defiVpin** vpins_;

  int numShields_;            // number of SHIELD paths used
  int shieldsAllocated_;      // allocated size of SHIELD paths array
  defiShield** shields_;      // SHIELD data 
  int numNoShields_;          // number of NOSHIELD paths used

  int numShieldNet_;          // number of SHIELDNETS used in array.
  int shieldNetsAllocated_;   // number of SHIELDNETS allocated in array.
  char** shieldNet_;          // name of the SHIELDNET
};


#endif
