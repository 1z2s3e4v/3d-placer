#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
using namespace std;

#ifndef WIN32
#   include <unistd.h>
#endif /* not WIN32 */
#include "defrReader.hpp"
#include "defiAlias.hpp"

#include "lefdef.h"
#include "placedb.h"
//#include "BinPacking.h"

//char defaultName[64];
//char defaultOut[64];

//--------------------
CPlaceDB* g_fplan;
int moduleCount = 0;
//--------------------

// Global variables
//FILE* fout;
//int userData;
int numObjs;
int isSumSet;      // to keep track if within SUM
int isProp = 0;    // for PROPERTYDEFINITIONS
int begOperand;    // to keep track for constraint, to print - as the 1st char

// TX_DIR:TRANSLATION ON

void myLogFunction(const char* errMsg){
   fprintf(fout, "ERROR: found error: %s\n", errMsg);
}

void myWarningLogFunction(const char* errMsg){
   fprintf(fout, "WARNING: found error: %s\n", errMsg);
}

//void dataError() {
//  fprintf(fout, "ERROR: returned user data is not correct!\n");
//}

void checkType(defrCallbackType_e c) {
  if (c >= 0 && c <= defrDesignEndCbkType) {
    // OK
  } else {
    fprintf(fout, "ERROR: callback type is out of bounds!\n");
  }
}


int done(defrCallbackType_e c, void* dummy, defiUserData ud) {
  checkType(c);
  if ((long)ud != userData) dataError();
  fprintf(fout, "END DESIGN\n");
  return 0;
}

int endfunc(defrCallbackType_e c, void* dummy, defiUserData ud) {
  checkType(c);
  if ((long)ud != userData) dataError();
  return 0;
}


/*char* orientStr(int orient) {
  switch (orient) {
      case 0: return ((char*)"N");
      case 1: return ((char*)"W");
      case 2: return ((char*)"S");
      case 3: return ((char*)"E");
      case 4: return ((char*)"FN");
      case 5: return ((char*)"FW");
      case 6: return ((char*)"FS");
      case 7: return ((char*)"FE");
  };
  return ((char*)"BOGUS");
}*/

int compf(defrCallbackType_e c, defiComponent* co, defiUserData ud) {
  //int i;

  checkType(c);

  // (donnie) ----------------------------------------------------
  // cell name: co->defiComponent::id()
  // cell type: co->defiComponent::name()
  // co->defiComponent::isPlaced()
  //         x: co->defiComponent::placementX()
  //         y: co->defiComponent::placementY()
  //    orient: orientStr(co->defiComponent::placementOrient())
  //---------------------------------------------------------------   
  // TODO: Handle orient 
  double x = 0;
  double y = 0;

  bool isFixed = false;
  int  orient = 0;
  if( co->defiComponent::isFixed() )
      isFixed = true;
  if( co->defiComponent::isPlaced() || co->defiComponent::isFixed() )
  {
      orient = co->defiComponent::placementOrient();
      x = co->defiComponent::placementX();
      y = co->defiComponent::placementY();
  }
  map<string,int>::const_iterator ite = parserLEFDEF.m_moduleNameMap.find( string( co->defiComponent::name() ) );
  if( ite == parserLEFDEF.m_moduleNameMap.end() )
  {
      printf( "%s %s NOT DEFINED\n", co->defiComponent::id(), co->defiComponent::name() );
      exit(0);
  }
  int lefMacroType = ite->second;
  double width = parserLEFDEF.m_modules[lefMacroType].GetWidth() * parserLEFDEF.m_defUnit;
  double height = parserLEFDEF.m_modules[lefMacroType].GetHeight() * parserLEFDEF.m_defUnit;


/*    if( orient % 2 == 1 )
    {	
//        // E, W, FE, FW. Save "current" height/width.
        swap( width, height );
    }
*/
// =====================(indark)===================== 2005-10-27

  string name = string( co->defiComponent::id() );
  g_fplan->AddModule( name, width, height, isFixed );
  g_fplan->SetModuleLocation( moduleCount, x, y );
  //g_fplan->SetModuleIsPin( moduleCount, false );  // default is not a pin
  //g_fplan->SetModuleOrientation( moduleCount, orient );
  //g_fplan->SetModuleType( moduleCount, lefMacroType );
  g_fplan->m_modules[moduleCount].m_lefCellId = lefMacroType;

  for( int i=0; i<(int)parserLEFDEF.m_modules[lefMacroType].m_pinsId.size(); i++ )
  {
      int pinId = parserLEFDEF.m_modules[lefMacroType].m_pinsId[i];
      g_fplan->AddPin( moduleCount, 
	      parserLEFDEF.m_pins[pinId].pinName, 
	      parserLEFDEF.m_pins[pinId].xOff * parserLEFDEF.m_defUnit,
	      parserLEFDEF.m_pins[pinId].yOff * parserLEFDEF.m_defUnit,
	      parserLEFDEF.m_pins[pinId].direction );
  }
  g_fplan->SetModuleOrientation( moduleCount, orient );
  
  /*
  for( int i=0; i<(int)parserLEFDEF.m_modules[lefMacroType].m_pinsId.size(); i++ )
  {
	int pinId = parserLEFDEF.m_modules[lefMacroType].m_pinsId[i];
switch ( orient ){
	case 0:				//N
	g_fplan->AddPin( moduleCount, 
	      parserLEFDEF.m_pins[pinId].pinName, 
	      parserLEFDEF.m_pins[pinId].xOff * parserLEFDEF.m_defUnit,
	      parserLEFDEF.m_pins[pinId].yOff * parserLEFDEF.m_defUnit );
		break;
	case 1:				//w
	g_fplan->AddPin( moduleCount, 
	      parserLEFDEF.m_pins[pinId].pinName, 
	      -(parserLEFDEF.m_pins[pinId].yOff * parserLEFDEF.m_defUnit),
	      parserLEFDEF.m_pins[pinId].xOff * parserLEFDEF.m_defUnit );
		break;			
	case 2:				//S
	g_fplan->AddPin( moduleCount, 
	      parserLEFDEF.m_pins[pinId].pinName, 
	      -(parserLEFDEF.m_pins[pinId].xOff * parserLEFDEF.m_defUnit) ,
	      -(parserLEFDEF.m_pins[pinId].yOff * parserLEFDEF.m_defUnit) );
		break;
	case 3:				//E
	g_fplan->AddPin( moduleCount, 
	      parserLEFDEF.m_pins[pinId].pinName, 
	      parserLEFDEF.m_pins[pinId].yOff * parserLEFDEF.m_defUnit ,
	      -(parserLEFDEF.m_pins[pinId].xOff * parserLEFDEF.m_defUnit ));
		break;

	case 4:				//FN
	g_fplan->AddPin( moduleCount, 
	      parserLEFDEF.m_pins[pinId].pinName, 
	      -(parserLEFDEF.m_pins[pinId].xOff * parserLEFDEF.m_defUnit) ,
	      parserLEFDEF.m_pins[pinId].yOff * parserLEFDEF.m_defUnit );
		break;
	case 5:				//FW
	g_fplan->AddPin( moduleCount, 
	      parserLEFDEF.m_pins[pinId].pinName, 
	      -(-(parserLEFDEF.m_pins[pinId].yOff * parserLEFDEF.m_defUnit)),
	      parserLEFDEF.m_pins[pinId].xOff * parserLEFDEF.m_defUnit );
		break;
	case 6:				//FS
	g_fplan->AddPin( moduleCount, 
	      parserLEFDEF.m_pins[pinId].pinName, 
	       -( -(parserLEFDEF.m_pins[pinId].xOff * parserLEFDEF.m_defUnit)) ,
	      -(parserLEFDEF.m_pins[pinId].yOff * parserLEFDEF.m_defUnit) );
		break;
	case 7:				//FE
	g_fplan->AddPin( moduleCount, 
	      parserLEFDEF.m_pins[pinId].pinName, 
	      -((parserLEFDEF.m_pins[pinId].yOff * parserLEFDEF.m_defUnit) ),
	      -(parserLEFDEF.m_pins[pinId].xOff * parserLEFDEF.m_defUnit ));
		break;
	default:
		exit(1);
		break;
	}

      

  }*/
  moduleCount++;
  //-------------------------------------
  
  if ((long)ud != userData) dataError();
//  missing GENERATE, FOREIGN
//>>    fprintf(fout, "- %s %s ", co->defiComponent::id(),
//>>            co->defiComponent::name());
/*    if (co->defiComponent::hasNets()) {
        for (i = 0; i < co->defiComponent::numNets(); i++)
             fprintf(fout, "%s ", co->defiComponent::net(i));
    }*/
//>>    if (co->defiComponent::isFixed()) 
//>>        fprintf(fout, "+ FIXED %d %d %s ",
//>>                co->defiComponent::placementX(),
//>>                co->defiComponent::placementY(),
//>>                orientStr(co->defiComponent::placementOrient()));
/*    if (co->defiComponent::isCover()) 
        fprintf(fout, "+ COVER %d %d %s ",
                co->defiComponent::placementX(),
                co->defiComponent::placementY(),
                orientStr(co->defiComponent::placementOrient()));*/
//>>    if (co->defiComponent::isPlaced()) 
//>>        fprintf(fout,"+ PLACED %d %d %s ",
//>>                co->defiComponent::placementX(),
//>>                co->defiComponent::placementY(),
//>>                orientStr(co->defiComponent::placementOrient()));
//>>    if (co->defiComponent::isUnplaced()) 
//>>        fprintf(fout,"+ UNPLACED ");
/*    if (co->defiComponent::hasSource())
        fprintf(fout, "+ SOURCE %s ", co->defiComponent::source());
    if (co->defiComponent::hasGenerate()) {
        fprintf(fout, "+ GENERATE %s ", co->defiComponent::generateName());
        if (co->defiComponent::macroName() &&
            *(co->defiComponent::macroName()))
           fprintf(fout, "%s ", co->defiComponent::macroName());
    }
    if (co->defiComponent::hasWeight())
        fprintf(fout, "+ WEIGHT %d ", co->defiComponent::weight());
    if (co->defiComponent::hasEEQ())
        fprintf(fout, "+ EEQMASTER %s ", co->defiComponent::EEQ());
    if (co->defiComponent::hasRegionName())
        fprintf(fout, "+ REGION %s ", co->defiComponent::regionName());
    if (co->defiComponent::hasRegionBounds()) {
        int *xl, *yl, *xh, *yh;
        int size;
        co->defiComponent::regionBounds(&size, &xl, &yl, &xh, &yh);
        for (i = 0; i < size; i++) { 
            fprintf(fout, "+ REGION %d %d %d %d \n",
                    xl[i], yl[i], xh[i], yh[i]);
        }
    }
    if (co->defiComponent::hasForeignName()) {
        fprintf(fout, "+ FOREIGN %s %d %d %s ",
                co->defiComponent::foreignName(), co->defiComponent::foreignX(),
                co->defiComponent::foreignY(), co->defiComponent::foreignOri());
    }
    if (co->defiComponent::numProps()) {
        for (i = 0; i < co->defiComponent::numProps(); i++) {
            fprintf(fout, "+ PROPERTY %s %s ", co->defiComponent::propName(i),
                    co->defiComponent::propValue(i));
            switch (co->defiComponent::propType(i)) {
               case 'R': fprintf(fout, "REAL ");
                         break;
               case 'I': fprintf(fout, "INTEGER ");
                         break;
               case 'S': fprintf(fout, "STRING ");
                         break;
               case 'Q': fprintf(fout, "QUOTESTRING ");
                         break;
               case 'N': fprintf(fout, "NUMBER ");
                         break;
            }
        }
    }*/
//>>    fprintf(fout, ";\n");
    --numObjs;
    if (numObjs <= 0)
	fprintf(fout, "END COMPONENTS\n");
  return 0;
}


int netf(defrCallbackType_e c, defiNet* net, defiUserData ud) {
  // For net and special net.
  int        i, j, k, x, y, z, count, newLayer;
  defiPath*  p;
  defiSubnet *s;
  int        path;
  defiVpin   *vpin;
  //defiShield *noShield;
  defiWire   *wire;

  checkType(c);
  if ((long)ud != userData) dataError();
  if (c != defrNetCbkType)
      fprintf(fout, "BOGUS NET TYPE  ");
  if (net->defiNet::pinIsMustJoin(0))
      fprintf(fout, "- MUSTJOIN ");
  else
  {
      // Remove msg (donnie, 2005-10-20) 
      //>> fprintf(fout, "- %s ", net->defiNet::name());
  }
 
  count = 0;

  //--------------------
  vector<int> myNet; // donnie
  vector<int> myNetMacro;
  static int netCount = 0;
  static bool created = false;
  if( !created )
  {
      printf( "Create module name map\n" );
      g_fplan->CreateModuleNameMap();
      created = true;
  }
  //--------------------
  
  
  // compName & pinName
  for (i = 0; i < net->defiNet::numConnections(); i++) {
      // set the limit of only 5 items per line
      /*count++;
      if (count >= 5) {
          fprintf(fout, "\n");
          count = 0;
      }
      fprintf(fout, "( %s %s ) ", net->defiNet::instance(i),
              net->defiNet::pin(i));*/
/*      if (net->defiNet::pinIsSynthesized(i))
          fprintf(fout, "+ SYNTHESIZED ");
*/
      // ------------------ (donnie)
      // Handle nets here.
      // ------------------
      //fprintf(fout, "(%s %s) ", 
	//      net->defiNet::instance(i),
	//      net->defiNet::pin(i));
    
      bool skipPin = false;
      int macroId; 
      if( strcmp( "PIN", net->defiNet::instance(i) ) != 0 )
      {
	  // connect to a macro 
          macroId = g_fplan->GetModuleId( net->defiNet::instance(i) );
	  if( macroId == -1 )
	  {
	      printf( "Error! Cannot find macro instance '%s'\n", net->defiNet::instance(i) );
	      exit(0);
	  }
	  // find port name
	  int pinId = -1;
	  vector<int> m_pinsId = g_fplan->GetModulePins( macroId );
	  for( int j=0; j<(int)m_pinsId.size(); j++ )
	  {
	      pinId = m_pinsId[j];
	      if( strcmp( g_fplan->GetPinName( pinId ).c_str(), net->defiNet::pin(i) ) == 0 )
	      {
		  // find it!
		  break;
	      }
	  }
	  if( pinId == -1 )
	  {
	      printf( "Error! Cannot find pin '%s' in macro instance '%s', skip it.\n", 
		      net->defiNet::pin(i), net->defiNet::instance(i) );
	      skipPin = true;
	  }
	  else
	  {
	      myNet.push_back( pinId );
	      myNetMacro.push_back( macroId );
	  }
	  //>> printf( "(%d %d) ", macroId, pinId );
      }
      else
      {
	  // Handle pin here
	  macroId = g_fplan->GetModuleId( net->defiNet::pin(i) );
	  
	  if( macroId == -1 )
	  {
	      printf( "Skip PIN instance '%s' for the net.\n", net->defiNet::pin(i) );
	      skipPin = true;
	      //exit(0);
	  }
	  else
	  {
	    vector<int> m_pinsId = g_fplan->GetModulePins( macroId );
	    assert( m_pinsId.size() == 1 );
	    myNet.push_back( m_pinsId[0] );
	    myNetMacro.push_back( macroId );
	    //>> fprintf(fout, "(PIN %d %d) ", macroId, m_pinsId[0] );
	  }
      }

      // fix bug, net may not exist (donnie, 2006-04-01)
      //if( !skipPin )
      //    g_fplan->AddModuleNetId( macroId, netCount );
      
  }
  //--------------------------
  if( myNet.size() > 1 )
  {
      //g_fplan->AddNet( myNet ); // donnie
      
      bool consider = true;
      if( net->defiNet::hasUse() ) // 2006-06-22
      {
    	  if( strcmp( "GROUND", net->defiNet::use() ) == 0 )
	  {
	      // TODO: set net weight to 0
	      //consider = false;
	      g_fplan->m_groundNets.insert( netCount );
	  }
      }
      if( consider ) 
      {
	  g_fplan->AddNet( myNet, net->defiNet::name() ); // donnie
	  assert( myNetMacro.size() == myNet.size() );
	  for( unsigned int i=0; i<myNetMacro.size(); i++ )
	      g_fplan->AddModuleNetId( myNetMacro[i], netCount );
	  netCount++;
      }
  }
  //---------------------------

//  if (net->hasNonDefaultRule())
//      fprintf(fout, "+ NONDEFAULTRULE %s\n", net->nonDefaultRule());

  for (i = 0; i < net->defiNet::numVpins(); i++) {
      vpin = net->defiNet::vpin(i);
      fprintf(fout, "  + %s", vpin->name());
      if (vpin->layer()) 
          fprintf(fout, " %s", vpin->layer());
      fprintf(fout, " %d %d %d %d", vpin->xl(), vpin->yl(), vpin->xh(),
              vpin->yh());
      if (vpin->status() != ' ') {
          fprintf(fout, " %c", vpin->status());
          fprintf(fout, " %d %d", vpin->xLoc(), vpin->yLoc());
          if (vpin->orient() != -1)
              fprintf(fout, " %s", orientStr(vpin->orient()));
      }
      fprintf(fout, "\n");
  }

  // regularWiring
/*  if (net->defiNet::numWires()) {
     for (i = 0; i < net->defiNet::numWires(); i++) {
        newLayer = 0;
        wire = net->defiNet::wire(i);
        fprintf(fout, "\n  + %s ", wire->wireType());
        count = 0;
        for (j = 0; j < wire->defiWire::numPaths(); j++) {
           p = wire->defiWire::path(j);
           p->initTraverse();
           while ((path = (int)p->defiPath::next()) != DEFIPATH_DONE) {
              count++;
              // Don't want the line to be too long
              if (count >= 5) {
                  fprintf(fout, "\n");
                  count = 0;
              } 
              switch (path) {
                case DEFIPATH_LAYER:
                     if (newLayer == 0) {
                         fprintf(fout, "%s ", p->defiPath::getLayer());
                         newLayer = 1;
                     } else
                         fprintf(fout, "NEW %s ", p->defiPath::getLayer());
                     break;
                case DEFIPATH_VIA:
                     fprintf(fout, "%s ", p->defiPath::getVia());
                     break;
                case DEFIPATH_VIAROTATION:
                     fprintf(fout, "%s ", 
                             orientStr(p->defiPath::getViaRotation()));
                     break;
                case DEFIPATH_WIDTH:
                     fprintf(fout, "%d ", p->defiPath::getWidth());
                     break;
                case DEFIPATH_POINT:
                     p->defiPath::getPoint(&x, &y);
                     fprintf(fout, "( %d %d ) ", x, y);
                     break;
                case DEFIPATH_FLUSHPOINT:
                     p->defiPath::getFlushPoint(&x, &y, &z);
                     fprintf(fout, "( %d %d %d ) ", x, y, z);
                     break;
                case DEFIPATH_TAPER:
                     fprintf(fout, "TAPER ");
                     break;
                case DEFIPATH_TAPERRULE:
                     fprintf(fout, "TAPERRULE %s ",p->defiPath::getTaperRule());
                     break;
              }
           }
        }
        fprintf(fout, "\n");
        count = 0;
     }
  }
*/
  // SHIELDNET
/*  if (net->defiNet::numShieldNets()) {
     for (i = 0; i < net->defiNet::numShieldNets(); i++) 
         fprintf(fout, "\n  + SHIELDNET %s", net->defiNet::shieldNet(i));
  }*/
/* obsolete in 5.4
  if (net->defiNet::numNoShields()) {
     for (i = 0; i < net->defiNet::numNoShields(); i++) { 
         noShield = net->defiNet::noShield(i); 
         fprintf(fout, "\n  + NOSHIELD ");
         newLayer = 0;
         for (j = 0; j < noShield->defiShield::numPaths(); j++) {
            p = noShield->defiShield::path(j);
            p->initTraverse();
            while ((path = (int)p->defiPath::next()) != DEFIPATH_DONE) {
               count++;
               // Don't want the line to be too long
               if (count >= 5) {
                   fprintf(fout, "\n");
                   count = 0;
               }
               switch (path) {
                 case DEFIPATH_LAYER:
                      if (newLayer == 0) {
                          fprintf(fout, "%s ", p->defiPath::getLayer());
                          newLayer = 1;
                      } else
                          fprintf(fout, "NEW %s ", p->defiPath::getLayer());
                      break;
                 case DEFIPATH_VIA:
                      fprintf(fout, "%s ", p->defiPath::getVia());
                      break;
                 case DEFIPATH_VIAROTATION:
                      fprintf(fout, "%s ", 
                             orientStr(p->defiPath::getViaRotation()));
                      break;
                 case DEFIPATH_WIDTH:
                      fprintf(fout, "%d ", p->defiPath::getWidth());
                      break;
                 case DEFIPATH_POINT:
                      p->defiPath::getPoint(&x, &y);
                      fprintf(fout, "( %d %d ) ", x, y);
                      break;
                 case DEFIPATH_FLUSHPOINT:
                      p->defiPath::getFlushPoint(&x, &y, &z);
                      fprintf(fout, "( %d %d %d ) ", x, y, z);
                      break;
                 case DEFIPATH_TAPER:
                      fprintf(fout, "TAPER ");
                      break;
                 case DEFIPATH_TAPERRULE:
                      fprintf(fout, "TAPERRULE %s ",
                              p->defiPath::getTaperRule());
                      break;
               }
            }
         }
     }
  }
*/

  if (net->defiNet::hasSubnets()) {
     for (i = 0; i < net->defiNet::numSubnets(); i++) {
        s = net->defiNet::subnet(i);
        fprintf(fout, "\n");
 
        if (s->defiSubnet::numConnections()) {
           if (s->defiSubnet::pinIsMustJoin(0))
              fprintf(fout, "- MUSTJOIN ");
           else
              fprintf(fout, "  + SUBNET %s ", s->defiSubnet::name());
           for (j = 0; j < s->defiSubnet::numConnections(); j++)
              fprintf(fout, " ( %s %s )\n", s->defiSubnet::instance(j),
                      s->defiSubnet::pin(j));

           // regularWiring
           if (s->defiSubnet::numWires()) {
              for (k = 0; k < s->defiSubnet::numWires(); k++) {
                 newLayer = 0;
                 wire = s->defiSubnet::wire(k);
                 fprintf(fout, "  %s ", wire->wireType());
                 count = 0;
                 for (j = 0; j < wire->defiWire::numPaths(); j++) {
                    p = wire->defiWire::path(j);
                    p->initTraverse();
                    while ((path = (int)p->defiPath::next()) != DEFIPATH_DONE) {
                       count++;
                       // Don't want the line to be too long
                       /*if (count >= 5) {
                           fprintf(fout, "\n");
                           count = 0;
                       } */
                       switch (path) {
                         case DEFIPATH_LAYER:
                              if (newLayer == 0) {
                                  fprintf(fout, "%s ", p->defiPath::getLayer());
                                  newLayer = 1;
                              } else
                                  fprintf(fout, "NEW %s ",
                                          p->defiPath::getLayer());
                              break;
                         case DEFIPATH_VIA:
                              fprintf(fout, "%s ", p->defiPath::getVia());
                              break;
                         case DEFIPATH_WIDTH:
                              fprintf(fout, "%d ", p->defiPath::getWidth());
                              break;
                         case DEFIPATH_POINT:
                              p->defiPath::getPoint(&x, &y);
                              fprintf(fout, "( %d %d ) ", x, y);
                              break;
                         case DEFIPATH_FLUSHPOINT:
                              p->defiPath::getFlushPoint(&x, &y, &z);
                              fprintf(fout, "( %d %d %d ) ", x, y, z);
                              break;
                         case DEFIPATH_TAPER:
                              fprintf(fout, "TAPER ");
                              break;
                         case DEFIPATH_TAPERRULE:
                              fprintf(fout, "TAPERRULE  %s ",
                                      p->defiPath::getTaperRule());
                              break;
                       }
                    }
                 }
              }
           }
         }
      }
   }
/*
  if (net->defiNet::numProps()) {
    for (i = 0; i < net->defiNet::numProps(); i++) {
        fprintf(fout, "  + PROPERTY %s ", net->defiNet::propName(i));
        switch (net->defiNet::propType(i)) {
           case 'R': fprintf(fout, "%g REAL ", net->defiNet::propNumber(i));
                     break;
           case 'I': fprintf(fout, "%g INTEGER ", net->defiNet::propNumber(i));
                     break;
           case 'S': fprintf(fout, "%s STRING ", net->defiNet::propValue(i));
                     break;
           case 'Q': fprintf(fout, "%s QUOTESTRING ", net->defiNet::propValue(i));
                     break;
           case 'N': fprintf(fout, "%g NUMBER ", net->defiNet::propNumber(i));
                     break;
        }
        fprintf(fout, "\n");
    }
  }
*/
/*
  if (net->defiNet::hasWeight())
    fprintf(fout, "+ WEIGHT %d ", net->defiNet::weight());
  if (net->defiNet::hasCap())
    fprintf(fout, "+ ESTCAP %g ", net->defiNet::cap());
  if (net->defiNet::hasSource())
    fprintf(fout, "+ SOURCE %s ", net->defiNet::source());
  if (net->defiNet::hasFixedbump())
    fprintf(fout, "+ FIXEDBUMP ");
  if (net->defiNet::hasFrequency())
    fprintf(fout, "+ FREQUENCY %g ", net->defiNet::frequency());
  if (net->defiNet::hasPattern())
    fprintf(fout, "+ PATTERN %s ", net->defiNet::pattern());
  if (net->defiNet::hasOriginal())
    fprintf(fout, "+ ORIGINAL %s ", net->defiNet::original());
*/

/*  if (net->defiNet::hasUse())
  {
    //------- (donnie) USE CLOCK, USE GROUND -------
    // TODO
    //----------------------------------------------
    fprintf(fout, "+ USE %s ", net->defiNet::use());
  }
  */

//>>  fprintf (fout, ";\n");
  --numObjs;
  if (numObjs <= 0)
      fprintf(fout, "END NETS\n");
  return 0;
}


int snetf(defrCallbackType_e c, defiNet* net, defiUserData ud) {
  // For net and special net.
  int         i, j, x, y, count, newLayer;
  char*       layerName;
  double      dist, left, right;
  defiPath*   p;
  defiSubnet  *s;
  int         path;
  defiShield* shield;
  defiWire*   wire;
  int         numX, numY, stepX, stepY;

  checkType(c);
  if ((long)ud != userData) dataError();
  if (c != defrSNetCbkType)
      fprintf(fout, "BOGUS NET TYPE  ");
  fprintf(fout, "- %s ", net->defiNet::name());

  count = 0;
  // compName & pinName
  for (i = 0; i < net->defiNet::numConnections(); i++) {
      // set the limit of only 5 items print out in one line
      count++;
      if (count >= 5) {
          fprintf(fout, "\n");
          count = 0;
      }
      fprintf (fout, "( %s %s ) ", net->defiNet::instance(i),
               net->defiNet::pin(i));
      if (net->defiNet::pinIsSynthesized(i))
          fprintf(fout, "+ SYNTHESIZED ");
  }

  // specialWiring
  if (net->defiNet::numWires()) {
     newLayer = 0;
     for (i = 0; i < net->defiNet::numWires(); i++) {
        newLayer = 0;
        wire = net->defiNet::wire(i);
        fprintf(fout, "\n  + %s ", wire->wireType());
        if (strcmp (wire->wireType(), "SHIELD") == 0)
           fprintf(fout, "%s ", wire->wireShieldNetName());
        for (j = 0; j < wire->defiWire::numPaths(); j++) {
           p = wire->defiWire::path(j);
           p->initTraverse();
           while ((path = (int)p->defiPath::next()) != DEFIPATH_DONE) {
              count++;
              // Don't want the line to be too long
              /*if (count >= 5) {
                  fprintf(fout, "\n");
                  count = 0;
              }*/
              switch (path) {
                case DEFIPATH_LAYER:
                     if (newLayer == 0) {
                         fprintf(fout, "%s ", p->defiPath::getLayer());
                         newLayer = 1;
                     } else
                         fprintf(fout, "NEW %s ", p->defiPath::getLayer());
                     break;
                case DEFIPATH_VIA:
                     fprintf(fout, "%s ", p->defiPath::getVia());
                     break;
                case DEFIPATH_VIAROTATION:
                     fprintf(fout, "%s ", 
                             orientStr(p->defiPath::getViaRotation()));
                     break;
                case DEFIPATH_VIADATA:
                     p->defiPath::getViaData(&numX, &numY, &stepX, &stepY);
                     fprintf(fout, "DO %d BY %d STEP %d %d ", numX, numY,
                             stepX, stepY);
                     break;
                case DEFIPATH_WIDTH:
                     fprintf(fout, "%d ", p->defiPath::getWidth());
                     break;
                case DEFIPATH_POINT:
                     p->defiPath::getPoint(&x, &y);
                     fprintf(fout, "( %d %d ) ", x, y);
                     break;
                case DEFIPATH_TAPER:
                     fprintf(fout, "TAPER ");
                     break;
                case DEFIPATH_SHAPE:
                     fprintf(fout, "+ SHAPE %s ", p->defiPath::getShape());
                     break;
              }
           }
        }
        fprintf(fout, "\n");
        count = 0;
     }
  }

  if (net->defiNet::hasSubnets()) {
    for (i = 0; i < net->defiNet::numSubnets(); i++) {
      s = net->defiNet::subnet(i);
      if (s->defiSubnet::numConnections()) {
          if (s->defiSubnet::pinIsMustJoin(0))
              fprintf(fout, "- MUSTJOIN ");
          else
              fprintf(fout, "- %s ", s->defiSubnet::name());
          for (j = 0; j < s->defiSubnet::numConnections(); j++) {
              fprintf(fout, " ( %s %s )\n", s->defiSubnet::instance(j),
                      s->defiSubnet::pin(j));
        }
      }
 
      // regularWiring
      if (s->defiSubnet::numWires()) {
         for (i = 0; i < s->defiSubnet::numWires(); i++) {
            wire = s->defiSubnet::wire(i);
            fprintf(fout, "  + %s ", wire->wireType());
            for (j = 0; j < wire->defiWire::numPaths(); j++) {
              p = wire->defiWire::path(j);
              p->defiPath::print(fout);
            }
         }
      }
    }
  }

  if (net->defiNet::numProps()) {
    for (i = 0; i < net->defiNet::numProps(); i++) {
        if (net->defiNet::propIsString(i))
           fprintf(fout, "  + PROPERTY %s %s ", net->defiNet::propName(i),
                   net->defiNet::propValue(i));
        if (net->defiNet::propIsNumber(i))
           fprintf(fout, "  + PROPERTY %s %g ", net->defiNet::propName(i),
                   net->defiNet::propNumber(i));
        switch (net->defiNet::propType(i)) {
           case 'R': fprintf(fout, "REAL ");
                     break;
           case 'I': fprintf(fout, "INTEGER ");
                     break;
           case 'S': fprintf(fout, "STRING ");
                     break;
           case 'Q': fprintf(fout, "QUOTESTRING ");
                     break;
           case 'N': fprintf(fout, "NUMBER ");
                     break;
        }
        fprintf(fout, "\n");
    }
  }

  // SHIELD
  count = 0;
  // testing the SHIELD for 5.3, obsolete in 5.4
  if (net->defiNet::numShields()) {
    for (i = 0; i < net->defiNet::numShields(); i++) {
       shield = net->defiNet::shield(i);
       fprintf(fout, "\n  + SHIELD %s ", shield->defiShield::shieldName());
       newLayer = 0;
       for (j = 0; j < shield->defiShield::numPaths(); j++) {
          p = shield->defiShield::path(j);
          p->initTraverse();
          while ((path = (int)p->defiPath::next()) != DEFIPATH_DONE) {
             count++;
             // Don't want the line to be too long
             /*if (count >= 5) {
                 fprintf(fout, "\n");
                 count = 0;
             }*/ 
             switch (path) {
               case DEFIPATH_LAYER:
                    if (newLayer == 0) {
                        fprintf(fout, "%s ", p->defiPath::getLayer());
                        newLayer = 1;
                    } else
                        fprintf(fout, "NEW %s ", p->defiPath::getLayer());
                    break;
               case DEFIPATH_VIA:
                    fprintf(fout, "%s ", p->defiPath::getVia());
                    break;
               case DEFIPATH_VIAROTATION:
                    fprintf(fout, "%s ", 
                            orientStr(p->defiPath::getViaRotation()));
                    break;
               case DEFIPATH_WIDTH:
                    fprintf(fout, "%d ", p->defiPath::getWidth());
                    break;
               case DEFIPATH_POINT:
                    p->defiPath::getPoint(&x, &y);
                    fprintf(fout, "( %d %d ) ", x, y);
                    break;
               case DEFIPATH_TAPER:
                    fprintf(fout, "TAPER ");
                    break;
               case DEFIPATH_SHAPE:
                    fprintf(fout, "+ SHAPE %s ", p->defiPath::getShape());
                    break;
             }
          }
       }
    }
  }

  // layerName width
  if (net->defiNet::hasWidthRules()) {
    for (i = 0; i < net->defiNet::numWidthRules(); i++) {
        net->defiNet::widthRule(i, &layerName, &dist);
        fprintf (fout, "\n  + WIDTH %s %g ", layerName, dist);
    }
  }

  // layerName spacing
  if (net->defiNet::hasSpacingRules()) {
    for (i = 0; i < net->defiNet::numSpacingRules(); i++) {
        net->defiNet::spacingRule(i, &layerName, &dist, &left, &right);
        if (left == right)
            fprintf (fout, "\n  + SPACING %s %g ", layerName, dist);
        else
            fprintf (fout, "\n  + SPACING %s %g RANGE %g %g ",
                     layerName, dist, left, right);
    }
  }

  if (net->defiNet::hasFixedbump())
    fprintf(fout, "\n  + FIXEDBUMP ");
  if (net->defiNet::hasFrequency())
    fprintf(fout, "\n  + FREQUENCY %g ", net->defiNet::frequency());
  if (net->defiNet::hasVoltage())
    fprintf(fout, "\n  + VOLTAGE %g ", net->defiNet::voltage());
  if (net->defiNet::hasWeight())
    fprintf(fout, "\n  + WEIGHT %d ", net->defiNet::weight());
  if (net->defiNet::hasCap())
    fprintf(fout, "\n  + ESTCAP %g ", net->defiNet::cap());
  if (net->defiNet::hasSource())
    fprintf(fout, "\n  + SOURCE %s ", net->defiNet::source());
  if (net->defiNet::hasPattern())
    fprintf(fout, "\n  + PATTERN %s ", net->defiNet::pattern());
  if (net->defiNet::hasOriginal())
    fprintf(fout, "\n  + ORIGINAL %s ", net->defiNet::original());
  if (net->defiNet::hasUse())
    fprintf(fout, "\n  + USE %s ", net->defiNet::use());

  fprintf (fout, ";\n");
  --numObjs;
  if (numObjs <= 0)
      fprintf(fout, "END SPECIALNETS\n");
  return 0;
}


int tname(defrCallbackType_e c, const char* string, defiUserData ud) {
  checkType(c);
  if ((long)ud != userData) dataError();
  fprintf(fout, "TECHNOLOGY %s ;\n", string);
  return 0;
}

int dname(defrCallbackType_e c, const char* string, defiUserData ud) {
  checkType(c);
  if ((long)ud != userData) dataError();
  fprintf(fout, "DESIGN %s ;\n", string);

  // Test changing the user data.
  userData = 89;
  defrSetUserData((void*)userData);

  return 0;
}


char* address(const char* in) {
  return ((char*)in);
}

int cs(defrCallbackType_e c, int num, defiUserData ud) {
  char* name;

  checkType(c);

  if ((long)ud != userData) dataError();

  switch (c) {
  case defrComponentStartCbkType : name = address("COMPONENTS"); break;
  case defrNetStartCbkType : name = address("NETS"); break;
  case defrStartPinsCbkType : name = address("PINS"); break;
  case defrViaStartCbkType : name = address("VIAS"); break;
  case defrRegionStartCbkType : name = address("REGIONS"); break;
  case defrSNetStartCbkType : name = address("SPECIALNETS"); break;
  case defrGroupsStartCbkType : name = address("GROUPS"); break;
  case defrScanchainsStartCbkType : name = address("SCANCHAINS"); break;
  case defrIOTimingsStartCbkType : name = address("IOTIMINGS"); break;
  case defrFPCStartCbkType : name = address("FLOORPLANCONSTRAINTS"); break;
  case defrTimingDisablesStartCbkType : name = address("TIMING DISABLES"); break;
  case defrPartitionsStartCbkType : name = address("PARTITIONS"); break;
  case defrPinPropStartCbkType : name = address("PINPROPERTIES"); break;
  case defrBlockageStartCbkType : name = address("BLOCKAGES"); break;
  case defrSlotStartCbkType : name = address("SLOTS"); break;
  case defrFillStartCbkType : name = address("FILLS"); break;
  default : name = address("BOGUS"); return 1;
  }
  fprintf(fout, "\n%s %d ;\n", name, num);

  //--------------------
  if( c == defrComponentStartCbkType )
  {
      printf( "%d cells\n", num );
      g_fplan->ReserveModuleMemory( num );
      parserLEFDEF.m_nComponents = num;
  }
  //--------------------
  
  numObjs = num;
  return 0;
}

int constraintst(defrCallbackType_e c, int num, defiUserData ud) {
  // Handles both constraints and assertions
  checkType(c);
  if ((long)ud != userData) dataError();
  if (c == defrConstraintsStartCbkType)
      fprintf(fout, "\nCONSTRAINTS %d ;\n\n", num);
  else
      fprintf(fout, "\nASSERTIONS %d ;\n\n", num);
  numObjs = num;
  return 0;
}

void operand(defrCallbackType_e c, defiAssertion* a, int ind) {
  int i, first = 1;
  char* netName;
  char* fromInst, * fromPin, * toInst, * toPin;

  if (a->defiAssertion::isSum()) {
      // Sum in operand, recursively call operand
      fprintf(fout, "- SUM ( ");
      a->defiAssertion::unsetSum();
      isSumSet = 1;
      begOperand = 0;
      operand (c, a, ind);
      fprintf(fout, ") ");
  } else {
      // operand
      if (ind >= a->defiAssertion::numItems()) {
          fprintf(fout, "ERROR: when writing out SUM in Constraints.\n");
          return;
       }
      if (begOperand) {
         fprintf(fout, "- ");
         begOperand = 0;
      }
      for (i = ind; i < a->defiAssertion::numItems(); i++) {
          if (a->defiAssertion::isNet(i)) {
              a->defiAssertion::net(i, &netName);
              if (!first)
                  fprintf(fout, ", "); // print , as separator
              fprintf(fout, "NET %s ", netName); 
          } else if (a->defiAssertion::isPath(i)) {
              a->defiAssertion::path(i, &fromInst, &fromPin, &toInst,
                                     &toPin);
              if (!first)
                  fprintf(fout, ", ");
              fprintf(fout, "PATH %s %s %s %s ", fromInst, fromPin, toInst,
                      toPin);
          } else if (isSumSet) {
              // SUM within SUM, reset the flag
              a->defiAssertion::setSum();
              operand(c, a, i);
          }
          first = 0;
      } 
      
  }
}

int constraint(defrCallbackType_e c, defiAssertion* a, defiUserData ud) {
  // Handles both constraints and assertions

  checkType(c);
  if ((long)ud != userData) dataError();
  if (a->defiAssertion::isWiredlogic())
      // Wirelogic
      fprintf(fout, "- WIREDLOGIC %s + MAXDIST %g ;\n",
// Wiredlogic dist is also store in fallMax
//              a->defiAssertion::netName(), a->defiAssertion::distance());
              a->defiAssertion::netName(), a->defiAssertion::fallMax());
  else {
      // Call the operand function
      isSumSet = 0;    // reset the global variable
      begOperand = 1;
      operand (c, a, 0);
      // Get the Rise and Fall
      if (a->defiAssertion::hasRiseMax())
          fprintf(fout, "+ RISEMAX %g ", a->defiAssertion::riseMax());
      if (a->defiAssertion::hasFallMax())
          fprintf(fout, "+ FALLMAX %g ", a->defiAssertion::fallMax());
      if (a->defiAssertion::hasRiseMin())
          fprintf(fout, "+ RISEMIN %g ", a->defiAssertion::riseMin());
      if (a->defiAssertion::hasFallMin())
          fprintf(fout, "+ FALLMIN %g ", a->defiAssertion::fallMin());
      fprintf(fout, ";\n");
  }
  --numObjs;
  if (numObjs <= 0) {
      if (c == defrConstraintCbkType)
          fprintf(fout, "END CONSTRAINTS\n");
      else 
          fprintf(fout, "END ASSERTIONS\n");
  }
  return 0;
}


int propstart(defrCallbackType_e c, void* dummy, defiUserData ud) {
  checkType(c);
  fprintf(fout, "\nPROPERTYDEFINITIONS\n");
  isProp = 1;

  return 0;
}


int prop(defrCallbackType_e c, defiProp* p, defiUserData ud) {
  checkType(c);
  if ((long)ud != userData) dataError();
  if (strcmp(p->defiProp::propType(), "design") == 0)
      fprintf(fout, "DESIGN %s ", p->defiProp::propName());
  else if (strcmp(p->defiProp::propType(), "net") == 0)
      fprintf(fout, "NET %s ", p->defiProp::propName());
  else if (strcmp(p->defiProp::propType(), "component") == 0)
      fprintf(fout, "COMPONENT %s ", p->defiProp::propName());
  else if (strcmp(p->defiProp::propType(), "specialnet") == 0)
      fprintf(fout, "SPECIALNET %s ", p->defiProp::propName());
  else if (strcmp(p->defiProp::propType(), "group") == 0)
      fprintf(fout, "GROUP %s ", p->defiProp::propName());
  else if (strcmp(p->defiProp::propType(), "row") == 0)
      fprintf(fout, "ROW %s ", p->defiProp::propName());
  else if (strcmp(p->defiProp::propType(), "componentpin") == 0)
      fprintf(fout, "COMPONENTPIN %s ", p->defiProp::propName());
  else if (strcmp(p->defiProp::propType(), "region") == 0)
      fprintf(fout, "REGION %s ", p->defiProp::propName());
  if (p->defiProp::dataType() == 'I')
      fprintf(fout, "INTEGER ");
  if (p->defiProp::dataType() == 'R')
      fprintf(fout, "REAL ");
  if (p->defiProp::dataType() == 'S')
      fprintf(fout, "STRING ");
  if (p->defiProp::dataType() == 'Q')
      fprintf(fout, "STRING ");
  if (p->defiProp::hasRange()) {
      fprintf(fout, "RANGE %g %g ", p->defiProp::left(),
              p->defiProp::right());
  }
  if (p->defiProp::hasNumber())
      fprintf(fout, "%g ", p->defiProp::number());
  if (p->defiProp::hasString())
      fprintf(fout, "\"%s\" ", p->defiProp::string());
  fprintf(fout, ";\n");

  return 0;
}


int propend(defrCallbackType_e c, void* dummy, defiUserData ud) {
  checkType(c);
  if (isProp) {
      fprintf(fout, "END PROPERTYDEFINITIONS\n\n");
      isProp = 0;
  }

  defrSetCaseSensitivity(1);
  return 0;
}


int hist(defrCallbackType_e c, const char* h, defiUserData ud) {
  checkType(c);
  defrSetCaseSensitivity(0);
  if ((long)ud != userData) dataError();
  fprintf(fout, "HISTORY %s ;\n", h);
  defrSetCaseSensitivity(1);
  return 0;
}


int an(defrCallbackType_e c, const char* h, defiUserData ud) {
  checkType(c);
  if ((long)ud != userData) dataError();
  fprintf(fout, "ARRAY %s ;\n", h);
  return 0;
}


int fn(defrCallbackType_e c, const char* h, defiUserData ud) {
  checkType(c);
  if ((long)ud != userData) dataError();
  fprintf(fout, "FLOORPLAN %s ;\n", h);
  return 0;
}


int bbn(defrCallbackType_e c, const char* h, defiUserData ud) {
  checkType(c);
  if ((long)ud != userData) dataError();
  fprintf(fout, "BUSBITCHARS \"%s\" ;\n", h);
  return 0;
}


int vers(defrCallbackType_e c, double d, defiUserData ud) {
  checkType(c);
  if ((long)ud != userData) dataError();
  fprintf(fout, "VERSION %g ;\n", d);

  // wmd -- testing the alias
  //defrAddAlias ("alias1", "aliasValue1", 1);
  //defrAddAlias ("alias2", "aliasValue2", 0);
  defiAlias_itr *aliasStore;
  aliasStore = (defiAlias_itr*)malloc(sizeof(defiAlias_itr*));
  aliasStore->Init();
  while (aliasStore->defiAlias_itr::Next()) {
     fprintf(fout, "ALIAS %s %s %d ;\n", aliasStore->defiAlias_itr::Key(),
                   aliasStore->defiAlias_itr::Data(),
                   aliasStore->defiAlias_itr::Marked());
  } 
  free(aliasStore);
  return 0;
}


int versStr(defrCallbackType_e c, const char* versionName, defiUserData ud) {
  checkType(c);
  if ((long)ud != userData) dataError();
  fprintf(fout, "VERSION %s ;\n", versionName);
  return 0;
}


int units(defrCallbackType_e c, double d, defiUserData ud) {
  checkType(c);
  if ((long)ud != userData) dataError();
  fprintf(fout, "UNITS DISTANCE MICRONS %g ;\n", d);
  //-----------------------------
  parserLEFDEF.m_defUnit = d;
  if( parserLEFDEF.m_defUnit > parserLEFDEF.m_lefUnit )
  {
      printf( "WARN: DEF Convert Factor (%g) > DEF Convert Factor (%g)\n",
	   parserLEFDEF.m_defUnit, parserLEFDEF.m_lefUnit );
      //exit(0);
  }
  //-----------------------------
  return 0;
}


int casesens(defrCallbackType_e c, int d, defiUserData ud) {
  checkType(c);
  if ((long)ud != userData) dataError();
  if (d == 1)
     fprintf(fout, "NAMESCASESENSITIVE ON ;\n"/*, d*/);
  else
     fprintf(fout, "NAMESCASESENSITIVE OFF ;\n"/*, d*/);
  return 0;
}


// donnie
// read "ROW" "SITE" "DIEAREA" "PIN" here!
int cls(defrCallbackType_e c, void* cl, defiUserData ud) {
  defiSite* site;  // Site and Canplace and CannotOccupy
  defiBox* box;  // DieArea and 
  defiPinCap* pc;
  defiPin* pin;
  int i;
  //int j;
  defiRow* row;
  defiTrack* track;
  defiGcellGrid* gcg;
  //defiVia* via;
  defiRegion* re;
  defiGroup* group;
  defiScanchain* sc;
  defiIOTiming* iot;
  defiFPC* fpc;
  defiTimingDisable* td;
  defiPartition* part;
  defiPinProp* pprop;
  defiBlockage* block;
  defiSlot* slots;
  defiFill* fills;
  int xl, yl, xh, yh;
  char *name, *a1, *b1;
  char **inst, **inPin, **outPin;
  int  *bits;
  int  size;
  int corner, typ;
  const char *itemT;
  char dir;
  //defiPinAntennaModel* aModel;

  //--------------
  CSiteRow mySiteRow;
  double rowRight;
  double cx, cy;
  bool ignorePin = false;
  //--------------
  
  checkType(c);
  if ((long)ud != userData) dataError();
  switch (c) {

  case defrSiteCbkType :
      // TODO: handle "SITE" as row info in ICCAD04-IBM benchmark
         site = (defiSite*)cl;
         fprintf(fout, "SITE %s %g %g %s ", site->defiSite::name(),
                 site->defiSite::x_orig(), site->defiSite::y_orig(),
                 orientStr(site->defiSite::orient()));
         fprintf(fout, "DO %g BY %g STEP %g %g ;\n",
                 site->defiSite::x_num(), site->defiSite::y_num(),
                 site->defiSite::x_step(), site->defiSite::y_step());
	 
	 //========================(indark)==================================
	 mySiteRow.m_bottom = site->defiSite::y_orig();
	 mySiteRow.m_height = site->defiSite::y_step();
	 if(mySiteRow.m_height <= 0.0)
		 mySiteRow.m_height = parserLEFDEF.m_coreSiteHeight * parserLEFDEF.m_defUnit;
	 mySiteRow.m_step = site->defiSite::x_step();
	 if(mySiteRow.m_step <= 0.0)
		 mySiteRow.m_step = parserLEFDEF.m_coreSiteWidth * parserLEFDEF.m_defUnit;
	//@====================================================================
	 mySiteRow.m_interval.push_back(site->defiSite::x_orig());
	 rowRight = site->defiSite::x_num() *  site->defiSite::x_step() + site->defiSite::x_orig();
	 //printf( "right = %g\n", rowRight );
	 mySiteRow.m_interval.push_back( rowRight );
	 g_fplan->m_sites.push_back( mySiteRow );
	 /*printf( "  >> ROW y=%.0f, h=%.0f, x=from %.0f to %.0f\n",
	 mySiteRow.m_bottom,
	 mySiteRow.m_height,
	 mySiteRow.m_interval[0],
	 mySiteRow.m_interval[1] );*/
	 //printf( "right = %g\n", g_fplan->m_sites.back().m_interval.back() );
	 //----------------------------------------------------------------------------
/*         fprintf(fout, "ROW %s %s %g %g %s ", 
	 row->defiRow::name(),
	 row->defiRow::macro(), 
	 row->defiRow::x(), 
	 row->defiRow::y(),
	 orientStr(row->defiRow::orient()));
	 fprintf(fout, "DO %g BY %g STEP %g %g ;\n",
	 row->defiRow::xNum(), 
	 row->defiRow::yNum(),
	 row->defiRow::xStep(), 
	 row->defiRow::yStep());
		 */
// 	 if (row->defiRow::numProps() > 0) {
// 		 for (i = 0; i < row->defiRow::numProps(); i++) {
// 			 fprintf(fout, "  + PROPERTY %s %s ",
// 				 row->defiRow::propName(i),
// 				 row->defiRow::propValue(i));
// 			 switch (row->defiRow::propType(i)) {
// 				 case 'R': fprintf(fout, "REAL ");
// 				 break;
// 				 case 'I': fprintf(fout, "INTEGER ");
// 				 break;
// 				 case 'S': fprintf(fout, "STRING ");
// 				 break;
// 				 case 'Q': fprintf(fout, "QUOTESTRING ");
// 				 break;
// 				 case 'N': fprintf(fout, "NUMBER ");
// 				 break;
// 			 }
// 		 }
// 		 fprintf(fout, ";\n");
// 	 }

         break;
  case defrCanplaceCbkType :
         site = (defiSite*)cl;
         fprintf(fout, "CANPLACE %s %g %g %s ", site->defiSite::name(),
                 site->defiSite::x_orig(), site->defiSite::y_orig(),
                 orientStr(site->defiSite::orient()));
         fprintf(fout, "DO %g BY %g STEP %g %g ;\n",
                 site->defiSite::x_num(), site->defiSite::y_num(),
                 site->defiSite::x_step(), site->defiSite::y_step());
         break;
  case defrCannotOccupyCbkType : 
         site = (defiSite*)cl;
         fprintf(fout, "CANNOTOCCUPY %s %g %g %s ",
                 site->defiSite::name(), site->defiSite::x_orig(),
                 site->defiSite::y_orig(), orientStr(site->defiSite::orient()));
         fprintf(fout, "DO %g BY %g STEP %g %g ;\n",
                 site->defiSite::x_num(), site->defiSite::y_num(),
                 site->defiSite::x_step(), site->defiSite::y_step());
         break;
  case defrDieAreaCbkType :
         box = (defiBox*)cl;
	 //-------------------- (donnie)
	 g_fplan->SetDieArea( box->defiBox::xl(), 
		              box->defiBox::yl(), 
			      box->defiBox::xh(),
			      box->defiBox::yh() );
	 //--------------------
         fprintf(fout, "DIEAREA %d %d %d %d ;\n",
                 box->defiBox::xl(), box->defiBox::yl(), box->defiBox::xh(),
                 box->defiBox::yh());
         break;
  case defrPinCapCbkType :
         pc = (defiPinCap*)cl;
         fprintf(fout, "MINPINS %d WIRECAP %g ;\n",
                 pc->defiPinCap::pin(), pc->defiPinCap::cap());
         --numObjs;
         if (numObjs <= 0)
             fprintf(fout, "END DEFAULTCAP\n");
         break;
  case defrPinCbkType :
         pin = (defiPin*)cl;
	 
	 // ------------------------ (donnie) ------------------------------------
	 // Handle pins as fixed modules
	 if( !pin->defiPin::hasPlacement() )
	 {
	     // 2007-01-31 remove msg (donnie)
	     //printf( "NTUp: WARNING: PIN '%s' has no placement, ignore it\n", 
	     //	     pin->defiPin::pinName() );
	     ignorePin = true;
	     // 2005-10-19
	     //cx = 0;
	     //cy = 0;
	     //exit(0);
	 }
	 else
	 {
	    cx = pin->defiPin::placementX();
	    cy = pin->defiPin::placementY();
	 }

	 if( !ignorePin )
	 {
	     if (pin->defiPin::hasLayer()) 
	     {
		 pin->defiPin::bounds(&xl, &yl, &xh, &yh);
	     }
	     else
	     {
		 printf( "NTUp: PIN '%s' does not have LAYER (no bounding box)\n", 
			 pin->defiPin::pinName() );
		 xh = 1; xl = 0;
		 yh = 1; yl = 0;
	     }
	     g_fplan->AddModule( pin->defiPin::pinName(), (xh-xl), (yh-yl), true );	
	     g_fplan->SetModuleLocation( moduleCount, cx, cy );
	     //g_fplan->SetModuleIsPin( moduleCount, true );
	     g_fplan->m_modules[moduleCount].m_blockType = BT_PIN;
	     //g_fplan->SetModuleType( moduleCount, -1 ); // use default
	     g_fplan->SetModuleOrientation( moduleCount, pin->defiPin::orient() );
	     g_fplan->AddPin( moduleCount, pin->defiPin::pinName(), 0, 0 );

	     // 2006-06-19 (donnie)
	     if( pin->defiPin::hasDirection() )
	     {
		 if( strcmp( pin->defiPin::direction(), "OUTPUT" ) == 0 )
		 {
		     g_fplan->m_pins.back().direction = PIN_DIRECTION_IN; // PO is a "input port"
		     g_fplan->m_modules[moduleCount].m_blockType = BT_PO;
		 }
		 else if( strcmp( pin->defiPin::direction(), "INPUT" ) == 0 )
		 {
		    g_fplan->m_pins.back().direction = PIN_DIRECTION_OUT; // PI is a "output port"
		    g_fplan->m_modules[moduleCount].m_blockType = BT_PI;
		 }
		 else if( strcmp( pin->defiPin::direction(), "INOUT" ) == 0 ) 
		 {	 
			 g_fplan->m_modules[moduleCount].m_blockType = BT_P_INOUT;
		 }
	     } 
	     
	     moduleCount++;
	 }
	 //-----------------------------------------------------------------------
	 
         //fprintf(fout, "- %s + NET %s ", pin->defiPin::pinName(),
         //        pin->defiPin::netName());
         //if (pin->defiPin::hasDirection())
         //    fprintf(fout, "+ DIRECTION %s ", pin->defiPin::direction());

	 
/*         if (pin->defiPin::hasUse())
             fprintf(fout, "+ USE %s ", pin->defiPin::use());
         if (pin->defiPin::hasLayer()) {
             fprintf(fout, "+ LAYER %s ", pin->defiPin::layer());
             pin->defiPin::bounds(&xl, &yl, &xh, &yh);
             fprintf(fout, "%d %d %d %d ", xl, yl, xh, yh);
         }
         if (pin->defiPin::hasPlacement()) {
             if (pin->defiPin::isPlaced()) {
                 fprintf(fout, "+ PLACED ");
                 fprintf(fout, "( %d %d ) %s ", pin->defiPin::placementX(),
                     pin->defiPin::placementY(), 
                     orientStr(pin->defiPin::orient()));
             }
             if (pin->defiPin::isCover()) {
                 fprintf(fout, "+ COVER ");
                 fprintf(fout, "( %d %d ) %s ", pin->defiPin::placementX(),
                     pin->defiPin::placementY(), 
                     orientStr(pin->defiPin::orient()));
             }
             if (pin->defiPin::isFixed()) {
                 fprintf(fout, "+ FIXED ");
                 fprintf(fout, "( %d %d ) %s ", pin->defiPin::placementX(),
                     pin->defiPin::placementY(), 
                     orientStr(pin->defiPin::orient()));
             }
             //if (pin->defiPin::isUnplaced())
             //    fprintf(fout, "+ UNPLACED ");
         }
         if (pin->defiPin::hasSpecial()) {
             fprintf(fout, "+ SPECIAL ");
         }

         if (pin->hasAPinPartialMetalArea()) {
             for (i = 0; i < pin->defiPin::numAPinPartialMetalArea(); i++) {
                fprintf(fout, "ANTENNAPINPARTIALMETALAREA %d",
                        pin->APinPartialMetalArea(i));
                if (*(pin->APinPartialMetalAreaLayer(i)))
                    fprintf(fout, " LAYER %s",
                            pin->APinPartialMetalAreaLayer(i));
                fprintf(fout, "\n");
             }
         }
         if (pin->hasAPinPartialMetalSideArea()) {
             for (i = 0; i < pin->defiPin::numAPinPartialMetalSideArea(); i++) {
                fprintf(fout, "ANTENNAPINPARTIALMETALSIDEAREA %d",
                        pin->APinPartialMetalSideArea(i));
                if (*(pin->APinPartialMetalSideAreaLayer(i)))
                    fprintf(fout, " LAYER %s",
                        pin->APinPartialMetalSideAreaLayer(i));
                fprintf(fout, "\n");
             }
         }
         if (pin->hasAPinDiffArea()) {
             for (i = 0; i < pin->defiPin::numAPinDiffArea(); i++) {
                fprintf(fout, "ANTENNAPINDIFFAREA %d", pin->APinDiffArea(i));
                if (*(pin->APinDiffAreaLayer(i)))
                    fprintf(fout, " LAYER %s", pin->APinDiffAreaLayer(i));
                fprintf(fout, "\n");
             }
         }
         if (pin->hasAPinPartialCutArea()) {
             for (i = 0; i < pin->defiPin::numAPinPartialCutArea(); i++) {
                fprintf(fout, "ANTENNAPINPARTIALCUTAREA %d",
                        pin->APinPartialCutArea(i));
                if (*(pin->APinPartialCutAreaLayer(i)))
                    fprintf(fout, " LAYER %s", pin->APinPartialCutAreaLayer(i));
                fprintf(fout, "\n");
             }
         }

         for (j = 0; j < pin->numAntennaModel(); j++) {
            aModel = pin->antennaModel(j);
 
            fprintf(fout, "ANTENNAMODEL %s\n",
                    aModel->defiPinAntennaModel::antennaOxide()); 
 
            if (aModel->hasAPinGateArea()) {
                for (i = 0; i < aModel->defiPinAntennaModel::numAPinGateArea();
                   i++) {
                   fprintf(fout, "ANTENNAPINGATEAREA %d",
                           aModel->APinGateArea(i));
                   if (*(aModel->APinGateAreaLayer(i)))
                       fprintf(fout, " LAYER %s", aModel->APinGateAreaLayer(i));
                   fprintf(fout, "\n");
                }
            }
            if (aModel->hasAPinMaxAreaCar()) {
                for (i = 0;
                   i < aModel->defiPinAntennaModel::numAPinMaxAreaCar(); i++) {
                   fprintf(fout, "ANTENNAPINMAXAREACAR %d",
                           aModel->APinMaxAreaCar(i));
                   if (*(aModel->APinMaxAreaCarLayer(i)))
                       fprintf(fout,
                           " LAYER %s", aModel->APinMaxAreaCarLayer(i));
                   fprintf(fout, "\n");
                }
            }
            if (aModel->hasAPinMaxSideAreaCar()) {
                for (i = 0;
                     i < aModel->defiPinAntennaModel::numAPinMaxSideAreaCar(); 
                     i++) {
                   fprintf(fout, "ANTENNAPINMAXSIDEAREACAR %d",
                           aModel->APinMaxSideAreaCar(i));
                   if (*(aModel->APinMaxSideAreaCarLayer(i)))
                       fprintf(fout,
                           " LAYER %s", aModel->APinMaxSideAreaCarLayer(i));
                   fprintf(fout, "\n");
                }
            }
            if (aModel->hasAPinMaxCutCar()) {
                for (i = 0; i < aModel->defiPinAntennaModel::numAPinMaxCutCar();
                   i++) {
                   fprintf(fout, "ANTENNAPINMAXCUTCAR %d",
                       aModel->APinMaxCutCar(i));
                   if (*(aModel->APinMaxCutCarLayer(i)))
                       fprintf(fout, " LAYER %s",
                       aModel->APinMaxCutCarLayer(i));
                   fprintf(fout, "\n");
                }
            }
         }
         fprintf(fout, ";\n");
*/
         --numObjs;
         if (numObjs <= 0)
             fprintf(fout, "END PINS\n");
         break;
  case defrDefaultCapCbkType :
         i = (long)cl;
         fprintf(fout, "DEFAULTCAP %d\n", i);
         numObjs = i;
         break;
  case defrRowCbkType :
         row = (defiRow*)cl;
	 //------------------------ (donnie) ------------------------------------------
	 mySiteRow.m_bottom = row->defiRow::y();
	 mySiteRow.m_height = row->defiRow::yStep();
	 //========================(indark)==================================
	 if(mySiteRow.m_height <= 0.0)
		 mySiteRow.m_height = parserLEFDEF.m_coreSiteHeight * parserLEFDEF.m_defUnit;
	 mySiteRow.m_step = row->defiRow::xStep();
	 if(mySiteRow.m_step <= 0.0)
		 mySiteRow.m_step = parserLEFDEF.m_coreSiteWidth * parserLEFDEF.m_defUnit;
	//@====================================================================
	 mySiteRow.m_interval.push_back( row->defiRow::x() );
	 rowRight = row->defiRow::xNum() * row->defiRow::xStep() + row->defiRow::x();
	 //printf( "right = %g\n", rowRight );
	 mySiteRow.m_interval.push_back( rowRight );
	 g_fplan->m_sites.push_back( mySiteRow );
	 /*printf( "  >> ROW y=%.0f, h=%.0f, x=from %.0f to %.0f\n",
		 mySiteRow.m_bottom,
		 mySiteRow.m_height,
		 mySiteRow.m_interval[0],
		 mySiteRow.m_interval[1] );*/
	 //printf( "right = %g\n", g_fplan->m_sites.back().m_interval.back() );
	 //----------------------------------------------------------------------------
/*         fprintf(fout, "ROW %s %s %g %g %s ", 
		 row->defiRow::name(),
                 row->defiRow::macro(), 
		 row->defiRow::x(), 
		 row->defiRow::y(),
                 orientStr(row->defiRow::orient()));
         fprintf(fout, "DO %g BY %g STEP %g %g ;\n",
                 row->defiRow::xNum(), 
		 row->defiRow::yNum(),
                 row->defiRow::xStep(), 
		 row->defiRow::yStep());
		 */
         if (row->defiRow::numProps() > 0) {
            for (i = 0; i < row->defiRow::numProps(); i++) {
                fprintf(fout, "  + PROPERTY %s %s ",
                        row->defiRow::propName(i),
                        row->defiRow::propValue(i));
                switch (row->defiRow::propType(i)) {
                   case 'R': fprintf(fout, "REAL ");
                             break;
                   case 'I': fprintf(fout, "INTEGER ");
                             break;
                   case 'S': fprintf(fout, "STRING ");
                             break;
                   case 'Q': fprintf(fout, "QUOTESTRING ");
                             break;
                   case 'N': fprintf(fout, "NUMBER ");
                             break;
                }
            }
            fprintf(fout, ";\n");
         }
         break;
  case defrTrackCbkType :
         track = (defiTrack*)cl;
         fprintf(fout, "TRACKS %s %g DO %g STEP %g LAYER ",
                 track->defiTrack::macro(), track->defiTrack::x(),
                 track->defiTrack::xNum(), track->defiTrack::xStep());
         for (i = 0; i < track->defiTrack::numLayers(); i++)
            fprintf(fout, "%s ", track->defiTrack::layer(i));
         fprintf(fout, ";\n"); 
         break;
  case defrGcellGridCbkType :
         gcg = (defiGcellGrid*)cl;
         fprintf(fout, "GCELLGRID %s %d DO %d STEP %g ;\n",
                 gcg->defiGcellGrid::macro(), gcg->defiGcellGrid::x(),
                 gcg->defiGcellGrid::xNum(), gcg->defiGcellGrid::xStep());

	 // 2007-01-31 (donnie)
	 if( gcg->defiGcellGrid::macro()[0] == 'X' && gcg->defiGcellGrid::xStep() > 0 )
	 {
	     g_fplan->m_gCellLeft  = gcg->defiGcellGrid::x();
	     g_fplan->m_gCellNumberHorizontal = gcg->defiGcellGrid::xNum();
	     g_fplan->m_gCellWidth = gcg->defiGcellGrid::xStep();
	 }
	 if( gcg->defiGcellGrid::macro()[0] == 'Y' && gcg->defiGcellGrid::xStep() > 0 )
	 {
	     g_fplan->m_gCellHeight = gcg->defiGcellGrid::xStep();
	     g_fplan->m_gCellNumberVertical = gcg->defiGcellGrid::xNum();
	     g_fplan->m_gCellBottom = gcg->defiGcellGrid::x();
	 }

         break;
  case defrViaCbkType :
         /*via = (defiVia*)cl;
         fprintf(fout, "- %s ", via->defiVia::name());
         if (via->defiVia::hasPattern())
             fprintf(fout, "+ PATTERNNAME %s ", via->defiVia::pattern());
         for (i = 0; i < via->defiVia::numLayers(); i++) {
             via->defiVia::layer(i, &name, &xl, &yl, &xh, &yh);
             fprintf(fout, "+ RECT %s %d %d %d %d \n",
                     name, xl, yl, xh, yh);
         }
         fprintf(fout, " ;\n");
         --numObjs;
         if (numObjs <= 0)
             fprintf(fout, "END VIAS\n");*/
         break;
  case defrRegionCbkType :
         re = (defiRegion*)cl;
         fprintf(fout, "- %s ", re->defiRegion::name());
         for (i = 0; i < re->defiRegion::numRectangles(); i++)
             fprintf(fout, "%d %d %d %d \n", re->defiRegion::xl(i),
                     re->defiRegion::yl(i), re->defiRegion::xh(i),
                     re->defiRegion::yh(i));
         if (re->defiRegion::hasType())
             fprintf(fout, "+ TYPE %s\n", re->defiRegion::type());
         if (re->defiRegion::numProps()) {
             for (i = 0; i < re->defiRegion::numProps(); i++) {
                 fprintf(fout, "+ PROPERTY %s %s ", re->defiRegion::propName(i),
                         re->defiRegion::propValue(i));
                 switch (re->defiRegion::propType(i)) {
                    case 'R': fprintf(fout, "REAL ");
                              break;
                    case 'I': fprintf(fout, "INTEGER ");
                              break;
                    case 'S': fprintf(fout, "STRING ");
                              break;
                    case 'Q': fprintf(fout, "QUOTESTRING ");
                              break;
                    case 'N': fprintf(fout, "NUMBER ");
                              break;
                 }
             }
         }
         fprintf(fout, ";\n"); 
         --numObjs;
         if (numObjs <= 0) {
             fprintf(fout, "END REGIONS\n");
         }
         break;
  case defrGroupNameCbkType :
         if ((char*)cl) {
             fprintf(fout, "- %s", (char*)cl);
             break;
         }
  case defrGroupMemberCbkType :
         if ((char*)cl) {
             fprintf(fout, " %s", (char*)cl);
             break;
         }
  case defrGroupCbkType :
         group = (defiGroup*)cl;
         if (group->defiGroup::hasMaxX() | group->defiGroup::hasMaxY()
             | group->defiGroup::hasPerim()) {
             fprintf(fout, "\n  + SOFT ");
             if (group->defiGroup::hasPerim()) 
                 fprintf(fout, "MAXHALFPERIMETER %d ",
                         group->defiGroup::perim());
             if (group->defiGroup::hasMaxX())
                 fprintf(fout, "MAXX %d ", group->defiGroup::maxX());
             if (group->defiGroup::hasMaxY()) 
                 fprintf(fout, "MAXY %d ", group->defiGroup::maxY());
         } 
         if (group->defiGroup::hasRegionName())
             fprintf(fout, "\n  + REGION %s ", group->defiGroup::regionName());
         if (group->defiGroup::hasRegionBox()) {
             int *gxl, *gyl, *gxh, *gyh;
             int size;
             group->defiGroup::regionRects(&size, &gxl, &gyl, &gxh, &gyh);
             for (i = 0; i < size; i++)
                 fprintf(fout, "REGION %d %d %d %d ", gxl[i], gyl[i],
                         gxh[i], gyh[i]);
         }
         if (group->defiGroup::numProps()) {
             for (i = 0; i < group->defiGroup::numProps(); i++) {
                 fprintf(fout, "\n  + PROPERTY %s %s ",
                         group->defiGroup::propName(i),
                         group->defiGroup::propValue(i));
                 switch (group->defiGroup::propType(i)) {
                    case 'R': fprintf(fout, "REAL ");
                              break;
                    case 'I': fprintf(fout, "INTEGER ");
                              break;
                    case 'S': fprintf(fout, "STRING ");
                              break;
                    case 'Q': fprintf(fout, "QUOTESTRING ");
                              break;
                    case 'N': fprintf(fout, "NUMBER ");
                              break;
                 }
             }
         }
         fprintf(fout, " ;\n");
         --numObjs;
         if (numObjs <= 0)
             fprintf(fout, "END GROUPS\n");
         break;
  case defrScanchainCbkType :
         sc = (defiScanchain*)cl;
         fprintf(fout, "- %s\n", sc->defiScanchain::name());
         if (sc->defiScanchain::hasStart()) {
             sc->defiScanchain::start(&a1, &b1);
             fprintf(fout, "  + START %s %s\n", a1, b1);
         }
         if (sc->defiScanchain::hasStop()) {
             sc->defiScanchain::stop(&a1, &b1);
             fprintf(fout, "  + STOP %s %s\n", a1, b1);
         }
         if (sc->defiScanchain::hasCommonInPin() ||
             sc->defiScanchain::hasCommonOutPin()) {
             fprintf(fout, "  + COMMONSCANPINS ");
             if (sc->defiScanchain::hasCommonInPin())
                fprintf(fout, " ( IN %s ) ", sc->defiScanchain::commonInPin());
             if (sc->defiScanchain::hasCommonOutPin())
                fprintf(fout, " ( OUT %s ) ",sc->defiScanchain::commonOutPin());
             fprintf(fout, "\n");
         }
         if (sc->defiScanchain::hasFloating()) {
            sc->defiScanchain::floating(&size, &inst, &inPin, &outPin, &bits);
            if (size > 0)
                fprintf(fout, "  + FLOATING\n");
            for (i = 0; i < size; i++) {
                fprintf(fout, "    %s ", inst[i]);
                if (inPin[i])
                   fprintf(fout, "( IN %s ) ", inPin[i]);
                if (outPin[i])
                   fprintf(fout, "( OUT %s ) ", outPin[i]);
                if (bits[i] != -1)
                   fprintf(fout, "( BITS %d ) ", bits[i]);
                fprintf(fout, "\n");
            }
         }

         if (sc->defiScanchain::hasOrdered()) {
            for (i = 0; i < sc->defiScanchain::numOrderedLists(); i++) {
                sc->defiScanchain::ordered(i, &size, &inst, &inPin, &outPin,
                                           &bits);
                if (size > 0)
                    fprintf(fout, "  + ORDERED\n");
                for (i = 0; i < size; i++) {
                    fprintf(fout, "    %s ", inst[i]); 
                    if (inPin[i])
                       fprintf(fout, "( IN %s ) ", inPin[i]);
                    if (outPin[i])
                       fprintf(fout, "( OUT %s ) ", outPin[i]);
                    if (bits[i] != -1)
                       fprintf(fout, "( BITS %d ) ", bits[i]);
                    fprintf(fout, "\n");
                }
            }
         }

         if (sc->defiScanchain::hasPartition()) {
            fprintf(fout, "  + PARTITION %s ",
                    sc->defiScanchain::partitionName());
            if (sc->defiScanchain::hasPartitionMaxBits())
              fprintf(fout, "MAXBITS %d ",
                      sc->defiScanchain::partitionMaxBits());
         }
         fprintf(fout, ";\n");
         --numObjs;
         if (numObjs <= 0)
             fprintf(fout, "END SCANCHAINS\n");
         break;
  case defrIOTimingCbkType :
         iot = (defiIOTiming*)cl;
         fprintf(fout, "- ( %s %s )\n", iot->defiIOTiming::inst(),
                 iot->defiIOTiming::pin());
         if (iot->defiIOTiming::hasSlewRise())
             fprintf(fout, "  + RISE SLEWRATE %g %g\n",
                     iot->defiIOTiming::slewRiseMin(),
                     iot->defiIOTiming::slewRiseMax());
         if (iot->defiIOTiming::hasSlewFall())
             fprintf(fout, "  + FALL SLEWRATE %g %g\n",
                     iot->defiIOTiming::slewFallMin(),
                     iot->defiIOTiming::slewFallMax());
         if (iot->defiIOTiming::hasVariableRise())
             fprintf(fout, "  + RISE VARIABLE %g %g\n",
                     iot->defiIOTiming::variableRiseMin(),
                     iot->defiIOTiming::variableRiseMax());
         if (iot->defiIOTiming::hasVariableFall())
             fprintf(fout, "  + FALL VARIABLE %g %g\n",
                     iot->defiIOTiming::variableFallMin(),
                     iot->defiIOTiming::variableFallMax());
         if (iot->defiIOTiming::hasCapacitance())
             fprintf(fout, "  + CAPACITANCE %g\n",
                     iot->defiIOTiming::capacitance());
         if (iot->defiIOTiming::hasDriveCell()) {
             fprintf(fout, "  + DRIVECELL %s ",
                     iot->defiIOTiming::driveCell());
             if (iot->defiIOTiming::hasFrom())
                 fprintf(fout, "  FROMPIN %s ",
                         iot->defiIOTiming::from());
             if (iot->defiIOTiming::hasTo())
                 fprintf(fout, "  TOPIN %s ",
                         iot->defiIOTiming::to());
             if (iot->defiIOTiming::hasParallel())
                 fprintf(fout, "PARALLEL %g",
                         iot->defiIOTiming::parallel());
             fprintf(fout, "\n");
         }
         fprintf(fout, ";\n");
         --numObjs;
         if (numObjs <= 0)
             fprintf(fout, "END IOTIMINGS\n");
         break;
  case defrFPCCbkType :
         fpc = (defiFPC*)cl;
         fprintf(fout, "- %s ", fpc->defiFPC::name());
         if (fpc->defiFPC::isVertical())
             fprintf(fout, "VERTICAL ");
         if (fpc->defiFPC::isHorizontal())
             fprintf(fout, "HORIZONTAL ");
         if (fpc->defiFPC::hasAlign())
             fprintf(fout, "ALIGN ");
         if (fpc->defiFPC::hasMax())
             fprintf(fout, "%g ", fpc->defiFPC::alignMax());
         if (fpc->defiFPC::hasMin())
             fprintf(fout, "%g ", fpc->defiFPC::alignMin());
         if (fpc->defiFPC::hasEqual())
             fprintf(fout, "%g ", fpc->defiFPC::equal());
         for (i = 0; i < fpc->defiFPC::numParts(); i++) {
             fpc->defiFPC::getPart(i, &corner, &typ, &name);
             if (corner == 'B')
                 fprintf(fout, "BOTTOMLEFT ");
             else
                 fprintf(fout, "TOPRIGHT ");
             if (typ == 'R')
                 fprintf(fout, "ROWS %s ", name);
             else
                 fprintf(fout, "COMPS %s ", name);
         }
         fprintf(fout, ";\n");
         --numObjs;
         if (numObjs <= 0)
             fprintf(fout, "END FLOORPLANCONSTRAINTS\n");
         break;
  case defrTimingDisableCbkType :
         td = (defiTimingDisable*)cl;
         if (td->defiTimingDisable::hasFromTo())
             fprintf(fout, "- FROMPIN %s %s ",
                     td->defiTimingDisable::fromInst(),
                     td->defiTimingDisable::fromPin()/*,
                     td->defiTimingDisable::toInst(),
                     td->defiTimingDisable::toPin()*/);
         if (td->defiTimingDisable::hasThru())
             fprintf(fout, "- THRUPIN %s %s ",
                     td->defiTimingDisable::thruInst(),
                     td->defiTimingDisable::thruPin());
         if (td->defiTimingDisable::hasMacroFromTo())
             fprintf(fout, "- MACRO %s FROMPIN %s %s ",
                     td->defiTimingDisable::macroName(),
                     td->defiTimingDisable::fromPin(),
                     td->defiTimingDisable::toPin());
         //if (td->defiTimingDisable::hasMacroThru())
         //    fprintf(fout, "- MACRO %s THRUPIN %s %s ",
         //            td->defiTimingDisable::macroName(),
         //            td->defiTimingDisable::fromPin());
         fprintf(fout, ";\n");
         break;
  case defrPartitionCbkType :
         part = (defiPartition*)cl;
         fprintf(fout, "- %s ", part->defiPartition::name());
         if (part->defiPartition::isSetupRise() |
             part->defiPartition::isSetupFall() |
             part->defiPartition::isHoldRise() |
             part->defiPartition::isHoldFall()) {
             // has turnoff 
             fprintf(fout, "TURNOFF "); 
             if (part->defiPartition::isSetupRise())
                 fprintf(fout, "SETUPRISE "); 
             if (part->defiPartition::isSetupFall())
                 fprintf(fout, "SETUPFALL "); 
             if (part->defiPartition::isHoldRise())
                 fprintf(fout, "HOLDRISE "); 
             if (part->defiPartition::isHoldFall())
                 fprintf(fout, "HOLDFALL "); 
         }
         itemT = part->defiPartition::itemType();
         dir = part->defiPartition::direction();
         if (strcmp(itemT, "CLOCK") == 0) {
             if (dir == 'T')    // toclockpin
                 fprintf(fout, "+ TOCLOCKPIN %s %s ",
                         part->defiPartition::instName(),
                         part->defiPartition::pinName());
             if (dir == 'F')    // fromclockpin
                 fprintf(fout, "+ FROMCLOCKPIN %s %s ",
                         part->defiPartition::instName(),
                         part->defiPartition::pinName());
             if (part->defiPartition::hasMin())
                 fprintf(fout, "MIN %g %g ",
                         part->defiPartition::partitionMin(),
                         part->defiPartition::partitionMax());
             if (part->defiPartition::hasMax())
                 fprintf(fout, "MAX %g %g ",
                         part->defiPartition::partitionMin(),
                         part->defiPartition::partitionMax());
             fprintf(fout, "PINS ");
             for (i = 0; i < part->defiPartition::numPins(); i++)
                  fprintf(fout, "%s ", part->defiPartition::pin(i));
         } else if (strcmp(itemT, "IO") == 0) {
             if (dir == 'T')    // toiopin
                 fprintf(fout, "+ TOIOPIN %s %s ",
                         part->defiPartition::instName(),
                         part->defiPartition::pinName());
             if (dir == 'F')    // fromiopin
                 fprintf(fout, "+ FROMIOPIN %s %s ",
                         part->defiPartition::instName(),
                         part->defiPartition::pinName());
         } else if (strcmp(itemT, "COMP") == 0) {
             if (dir == 'T')    // tocomppin
                 fprintf(fout, "+ TOCOMPPIN %s %s ",
                         part->defiPartition::instName(),
                         part->defiPartition::pinName());
             if (dir == 'F')    // fromcomppin
                 fprintf(fout, "+ FROMCOMPPIN %s %s ",
                         part->defiPartition::instName(),
                         part->defiPartition::pinName());
         }
         fprintf(fout, ";\n");
         --numObjs;
         if (numObjs <= 0)
             fprintf(fout, "END PARTITIONS\n");
         break;

  case defrPinPropCbkType :
         pprop = (defiPinProp*)cl;
         if (pprop->defiPinProp::isPin())
            fprintf(fout, "- PIN %s ", pprop->defiPinProp::pinName());
         else 
            fprintf(fout, "- %s %s ",
                    pprop->defiPinProp::instName(),
                    pprop->defiPinProp::pinName());
         fprintf(fout, ";\n");
         if (pprop->defiPinProp::numProps() > 0) {
            for (i = 0; i < pprop->defiPinProp::numProps(); i++) {
                fprintf(fout, "  + PROPERTY %s %s ",
                        pprop->defiPinProp::propName(i),
                        pprop->defiPinProp::propValue(i));
                switch (pprop->defiPinProp::propType(i)) {
                   case 'R': fprintf(fout, "REAL ");
                             break;
                   case 'I': fprintf(fout, "INTEGER ");
                             break;
                   case 'S': fprintf(fout, "STRING ");
                             break;
                   case 'Q': fprintf(fout, "QUOTESTRING ");
                             break;
                   case 'N': fprintf(fout, "NUMBER ");
                             break;
                }
            }
            fprintf(fout, ";\n");
         }
         --numObjs;
         if (numObjs <= 0)
             fprintf(fout, "END PINPROPERTIES\n");
         break;
  case defrBlockageCbkType :
         block = (defiBlockage*)cl;
         if (block->defiBlockage::hasLayer()) {
            fprintf(fout, "- LAYER %s\n", block->defiBlockage::layerName());
            if (block->defiBlockage::hasComponent())
               fprintf(fout, "   + COMPONENT %s\n",
                       block->defiBlockage::layerComponentName());
            if (block->defiBlockage::hasSlots())
               fprintf(fout, "   + SLOTS\n");
            if (block->defiBlockage::hasFills())
               fprintf(fout, "   + FILLS\n");
            if (block->defiBlockage::hasPushdown())
               fprintf(fout, "   + PUSHDOWN\n");
         }
         else if (block->defiBlockage::hasPlacement()) {
            fprintf(fout, "- PLACEMENT\n");
            if (block->defiBlockage::hasComponent())
               fprintf(fout, "   + COMPONENT %s\n",
                       block->defiBlockage::layerComponentName());
            if (block->defiBlockage::hasPushdown())
               fprintf(fout, "   + PUSHDOWN\n");
         }

         for (i = 0; i < block->defiBlockage::numRectangles(); i++) {
            fprintf(fout, "   RECT %d %d %d %d\n", 
                    block->defiBlockage::xl(i), block->defiBlockage::yl(i),
                    block->defiBlockage::xh(i), block->defiBlockage::yh(i));
         } 
         fprintf(fout, ";\n");
         --numObjs;
         if (numObjs <= 0)
             fprintf(fout, "END BLOCKAGES\n");
         break;
  case defrSlotCbkType :
         slots = (defiSlot*)cl;
         if (slots->defiSlot::hasLayer())
            fprintf(fout, "- LAYER %s\n", slots->defiSlot::layerName());

         for (i = 0; i < slots->defiSlot::numRectangles(); i++) {
            fprintf(fout, "   RECT %d %d %d %d\n", 
                    slots->defiSlot::xl(i), slots->defiSlot::yl(i),
                    slots->defiSlot::xh(i), slots->defiSlot::yh(i));
         } 
         fprintf(fout, ";\n");
         --numObjs;
         if (numObjs <= 0)
             fprintf(fout, "END SLOTS\n");
         break;
  case defrFillCbkType :
         fills = (defiFill*)cl;
         if (fills->defiFill::hasLayer())
            fprintf(fout, "- LAYER %s\n", fills->defiFill::layerName());

         for (i = 0; i < fills->defiFill::numRectangles(); i++) {
            fprintf(fout, "   RECT %d %d %d %d\n", 
                    fills->defiFill::xl(i), fills->defiFill::yl(i),
                    fills->defiFill::xh(i), fills->defiFill::yh(i));
         } 
         fprintf(fout, ";\n");
         --numObjs;
         if (numObjs <= 0)
             fprintf(fout, "END FILLS\n");
         break;
  default: fprintf(fout, "BOGUS callback to cls.\n"); return 1;
  }
  return 0;
}


int dn(defrCallbackType_e c, const char* h, defiUserData ud) {
  checkType(c);
  if ((long)ud != userData) dataError();
  fprintf(fout, "DIVIDERCHAR \"%s\" ;\n",h);
  return 0;
}


int ext(defrCallbackType_e t, const char* c, defiUserData ud) {
  char* name;

  checkType(t);
  if ((long)ud != userData) dataError();

  switch (t) {
  case defrNetExtCbkType : name = address("net"); break;
  case defrComponentExtCbkType : name = address("component"); break;
  case defrPinExtCbkType : name = address("pin"); break;
  case defrViaExtCbkType : name = address("via"); break;
  case defrNetConnectionExtCbkType : name = address("net connection"); break;
  case defrGroupExtCbkType : name = address("group"); break;
  case defrScanChainExtCbkType : name = address("scanchain"); break;
  case defrIoTimingsExtCbkType : name = address("io timing"); break;
  case defrPartitionsExtCbkType : name = address("partition"); break;
  default: name = address("BOGUS"); return 1;
  }
  fprintf(fout, "  %s extension %s\n", name, c);
  return 0;
}

/*void* mallocCB(int size) {
  return malloc(size);
}

void* reallocCB(void* name, int size) {
  return realloc(name, size);
}*/
/*
void freeCB(void* name) {
  free(name);
  return;
}

void lineNumberCB(int lineNo) {
  fprintf(fout, "Parsed %d number of lines!!\n", lineNo);
  return;
}*/

int CParserLEFDEF::ReadDEF( char *file1, CPlaceDB& fplan )
{

  //-------------------
  g_fplan = &fplan;
  parserLEFDEF.m_defUnit = 100;
  //-------------------

  
  //int num = 1734;
  const char* inFile = file1;
  //char* outFile;
  FILE* f;
  int res;
  int noCalls = 0;
//  long start_mem;
  int retStr = 0;

//  start_mem = (long)sbrk(0);

  //strcpy(defaultName, "def.in");
  //strcpy(defaultOut, "list");
  //inFile = defaultName;
  //outFile = defaultOut;
  fout = stdout;
  userData = 0x01020304;
  
  //argc--;
  //argv++;
  //while (argc--) {

  //  if (strcmp(*argv, "-d") == 0) {
  //    argv++;
  //    argc--;
  //    sscanf(*argv, "%d", &num);
  //    defiSetDebug(num, 1);

  //  } else if (strcmp(*argv, "-nc") == 0) {
  //   noCalls = 1;

  //  } else if (strcmp(*argv, "-o") == 0) {
  //    argv++;
  //    argc--;
  //    outFile = *argv;
  //    if ((fout = fopen(outFile, "w")) == 0) {
  //	fprintf(stderr, "ERROR: could not open output file\n");
  //	return 2;
  //    }

  //  } else if (strcmp(*argv, "-verStr") == 0) {
  //      /* New to set the version callback routine to return a string    */
  //      /* instead of double.                                            */
  //      retStr = 1;

  //  } else if (argv[0][0] != '-') {
  //    inFile = *argv;
  //  } else if (strcmp(*argv, "-h") == 0) {
  //    fprintf(stderr, "Usage: defrw [<defFilename>] [-o <outputFilename>]\n");
  //    return 2;
  //  } else {
  //    fprintf(stderr, "ERROR: Illegal command line option: '%s'\n", *argv);
  //    return 2;
  //  }
  //
  //  argv++;
  //}

  defrInit();

  //defrSetLogFunction(myLogFunction);
  //defrSetWarningLogFunction(myWarningLogFunction);

  if (noCalls == 0) {
    //defrSetUserData((void*)3);
    //defrSetDesignCbk(dname);
    //defrSetTechnologyCbk(tname);
    //defrSetDesignEndCbk(done);
    //defrSetPropDefStartCbk(propstart);
    //defrSetPropCbk(prop);
    //defrSetPropDefEndCbk(propend);
    defrSetNetCbk(netf);
    //defrSetSNetCbk(snetf);
    defrSetComponentCbk(compf);
    //defrSetAddPathToNet();
    //defrSetHistoryCbk(hist);
    //defrSetConstraintCbk(constraint);
    //defrSetAssertionCbk(constraint);
    //defrSetArrayNameCbk(an);
    //defrSetFloorPlanNameCbk(fn);
    //defrSetDividerCbk(dn);
    //defrSetBusBitCbk(bbn);

    //defrSetAssertionsStartCbk(constraintst);
    //defrSetConstraintsStartCbk(constraintst);
    defrSetComponentStartCbk(cs);
    //defrSetPinPropStartCbk(cs);
    defrSetNetStartCbk(cs);
    defrSetStartPinsCbk(cs);
    //defrSetViaStartCbk(cs);
    //defrSetRegionStartCbk(cs);
    //defrSetSNetStartCbk(cs);
    //defrSetGroupsStartCbk(cs);
    //defrSetScanchainsStartCbk(cs);
    //defrSetIOTimingsStartCbk(cs);
    //defrSetFPCStartCbk(cs);
    //defrSetTimingDisablesStartCbk(cs);
    //defrSetPartitionsStartCbk(cs);
    //defrSetBlockageStartCbk(cs);
    //defrSetSlotStartCbk(cs);
    //defrSetFillStartCbk(cs);

    // All of the extensions point to the same function.
    //defrSetNetExtCbk(ext);
    //defrSetComponentExtCbk(ext);
    //defrSetPinExtCbk(ext);
    //defrSetViaExtCbk(ext);
    //defrSetNetConnectionExtCbk(ext);
    //defrSetGroupExtCbk(ext);
    //defrSetScanChainExtCbk(ext);
    //defrSetIoTimingsExtCbk(ext);
    //defrSetPartitionsExtCbk(ext);

    defrSetUnitsCbk(units);
    if (!retStr)
       defrSetVersionCbk(vers);
    else
       defrSetVersionStrCbk(versStr);
    defrSetCaseSensitiveCbk(casesens);

    // The following calls are an example of using one function "cls"
    // to be the callback for many DIFFERENT types of constructs.
    // We have to cast the function type to meet the requirements
    // of each different set function.
    defrSetSiteCbk((defrSiteCbkFnType)cls);
    //defrSetCanplaceCbk((defrSiteCbkFnType)cls);
    //defrSetCannotOccupyCbk((defrSiteCbkFnType)cls);
    defrSetDieAreaCbk((defrBoxCbkFnType)cls);
    //defrSetPinCapCbk((defrPinCapCbkFnType)cls);
    defrSetPinCbk((defrPinCbkFnType)cls);
    //defrSetPinPropCbk((defrPinPropCbkFnType)cls);
    //defrSetDefaultCapCbk((defrIntegerCbkFnType)cls);
    defrSetRowCbk((defrRowCbkFnType)cls);
    //defrSetTrackCbk((defrTrackCbkFnType)cls);
    defrSetGcellGridCbk((defrGcellGridCbkFnType)cls);
    //defrSetViaCbk((defrViaCbkFnType)cls);
    //defrSetRegionCbk((defrRegionCbkFnType)cls);
    //defrSetGroupNameCbk((defrStringCbkFnType)cls);
    //defrSetGroupMemberCbk((defrStringCbkFnType)cls);
    //defrSetGroupCbk((defrGroupCbkFnType)cls);
    //defrSetScanchainCbk((defrScanchainCbkFnType)cls);
    //defrSetIOTimingCbk((defrIOTimingCbkFnType)cls);
    //defrSetFPCCbk((defrFPCCbkFnType)cls);
    //defrSetTimingDisableCbk((defrTimingDisableCbkFnType)cls);
    //defrSetPartitionCbk((defrPartitionCbkFnType)cls);
    //defrSetBlockageCbk((defrBlockageCbkFnType)cls);
    //defrSetSlotCbk((defrSlotCbkFnType)cls);
    //defrSetFillCbk((defrFillCbkFnType)cls);

    //defrSetAssertionsEndCbk(endfunc);
    //defrSetComponentEndCbk(endfunc);
    //defrSetConstraintsEndCbk(endfunc);
    //defrSetNetEndCbk(endfunc);
    //defrSetFPCEndCbk(endfunc);
    //defrSetFPCEndCbk(endfunc);
    //defrSetGroupsEndCbk(endfunc);
    //defrSetIOTimingsEndCbk(endfunc);
    //defrSetNetEndCbk(endfunc);
    //defrSetPartitionsEndCbk(endfunc);
    //defrSetRegionEndCbk(endfunc);
    //defrSetSNetEndCbk(endfunc);
    //defrSetScanchainsEndCbk(endfunc);
    //defrSetPinEndCbk(endfunc);
    //defrSetTimingDisablesEndCbk(endfunc);
    //defrSetViaEndCbk(endfunc);
    //defrSetPinPropEndCbk(endfunc);
    //defrSetBlockageEndCbk(endfunc);
    //defrSetSlotEndCbk(endfunc);
    //defrSetFillEndCbk(endfunc);

    defrSetMallocFunction(mallocCB);
    defrSetReallocFunction(reallocCB);
    defrSetFreeFunction(freeCB);

    defrSetLineNumberFunction(lineNumberCB);
    defrSetDeltaNumberLines(100000);

    defrSetRegisterUnusedCallbacks();
  }

  if ((f = fopen(inFile,"r")) == 0) {
    fprintf(stderr,"Couldn't open input file '%s'\n", inFile);
    return(2);
  }
  res = defrRead(f, inFile, (void*)userData, 0);

  if (res)
     fprintf(stderr, "Reader returns bad status.\n"/*, inFile*/);

  (void)defrPrintUnusedCallbacks(fout);


  //------------ (donnie) -------
  // update pin absolute location
  for( int i=0; i<(int)g_fplan->m_modules.size(); i++ )
      g_fplan->CalcModuleCenter( i );
  for( int i=0; i<(int)g_fplan->GetPinNumber(); i++ )
      g_fplan->CalcPinLocation( i );
  // set row height
  g_fplan->m_rowHeight = this->m_coreSiteHeight * parserLEFDEF.m_defUnit;
 
  // Sort rows
  if( g_fplan->m_sites.size() == 0 )
  {
      printf( "NTUp: Error: No row info in the DEF file\n" );
      exit(0);
  }
  sort( g_fplan->m_sites.begin(), g_fplan->m_sites.end(), CSiteRow::Lesser );

  printf( "GCELL W %.0f  H %.0f  lb ( %.0f, %.0f )\n", 
	  g_fplan->m_gCellWidth, g_fplan->m_gCellHeight,
          g_fplan->m_gCellLeft, g_fplan->m_gCellBottom );


  double binWidth = g_fplan->m_gCellWidth;
  double binHeight = g_fplan->m_gCellHeight; 
  printf( "Calculate GCell capacity\n" ); 
  for( unsigned int i=0; i<parserLEFDEF.m_metalPitch.size(); i++ ) 
  { 
      if( i == 1 ) 
      { 
	  printf( "SKIP METAL 1\n" ); 
	  continue; 
      } 

      if( parserLEFDEF.m_metalDir[i] == CParserLEFDEF::VERTICAL ) 
      { 
	  int num = (int)round( binWidth / ( parserLEFDEF.m_metalPitch[i] * parserLEFDEF.m_defUnit ) ); 
	  parserLEFDEF.m_capHorizontal += num; // horizontal cut 
	  printf( "GCELL METAL %d VERTICAL PITCH %.2f TRACK %d\n", i, parserLEFDEF.m_metalPitch[i], num ); 
      } 

      if( parserLEFDEF.m_metalDir[i] == CParserLEFDEF::HORIZONTAL ) 
      { 
	  int num = (int)round( binHeight / ( parserLEFDEF.m_metalPitch[i] * parserLEFDEF.m_defUnit ) ); 
	  parserLEFDEF.m_capVertical += num; 
	  printf( "GCELL METAL %d HORIZONTAL PITCH %.2f TRACK %d\n", i, parserLEFDEF.m_metalPitch[i], num ); 
      } 
  } 
       



  //-----------------------------
  
  //fclose(fout);

  return 0;
}

