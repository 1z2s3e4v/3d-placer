
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         lefyyparse
#define yylex           lefyylex
#define yyerror         lefyyerror
#define yylval          lefyylval
#define yychar          lefyychar
#define yydebug         lefyydebug
#define yynerrs         lefyynerrs


/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 15 "lef.y"

#include <string.h>
#include <stdlib.h>
#include <math.h>
/* # include <sys/types.h> */
/* # include <sys/stat.h> */
/* # include <fcntl.h> */
/* # include <unistd.h> */
/* # ifndef	WIN32 */
/* # include <sys/param.h> */
/* # endif */
#include "FEF_stringhash.h"
#include "lex.h"
#include "lefiDefs.hpp"
#include "lefiUser.hpp"
#include "lefiUtil.hpp"
#include "lefrCallBacks.hpp"
#ifdef WIN32
#include <malloc.h>
#endif

#define LYPROP_ECAP "EDGE_CAPACITANCE"

int lefRetVal;
static char lefPropDefType;    /* save the current type of the property */

/* Macro to describe how we handle a callback.
 * If the function was set then call it.
 * If the function returns non zero then there was an error
 * so call the error routine and exit.
 */
#define CALLBACK(func, typ, data) \
    if (!lef_errors) { \
      if (func) { \
        if ((lefRetVal = (*func)(typ, data, lefrUserData)) == 0) { \
        } else if (lefRetVal == STOP_PARSE) { \
	  return lefRetVal; \
        } else { \
	  yyerror("Error in callback"); \
	  return lefRetVal; \
        } \
      } \
    }

#define CHKERR() \
    if (lef_errors > 20) { \
      yyerror("Too many syntax errors"); \
      lef_errors = 0; \
      return 1; \
    }

extern int lefNlines, lefNtokens;
extern FILE* lefFile;

/**********************************************************************/
/**********************************************************************/

#define C_EQ 0
#define C_NE 1
#define C_LT 2
#define C_LE 3
#define C_GT 4
#define C_GE 5

extern int lefDumbMode;   // for communicating with parser
extern int lefNoNum;      // likewise, says no numbers for the next N tokens
extern int lefRealNum;    // Next token will be a real number
extern int lefNlToken;    // likewise
extern int lefDefIf;      // likewise

extern int lef_errors;    // from lex.cpph

static int ignoreVersion = 0; // ignore checking version number

int lefNamesCaseSensitive = FALSE;  // are names case sensitive?

/* XXXXXXXXXXXXX check out these variables */
double lef_save_x, lef_save_y;  // for interpreting (*) notation of LEF/DEF

/* #define STRING_LIST_SIZE 1024 */
/* char string_list[STRING_LIST_SIZE]; */

static char temp_name[256];
static char layerName[128];
static char viaName[128];
static char viaRuleName[128];
static char nonDefaultRuleName[128];
static char siteName[128];
static char arrayName[128];
static char macroName[128];
static char pinName[128];

static int siteDef, symDef, sizeDef, pinDef, obsDef, origDef;
static int useLenThr;
static int layerCut, layerMastOver, layerRout, layerDir;
static lefiAntennaEnum antennaType = lefiAntennaAR;  /* 5.4 - antenna type */
static int viaRuleLayer;       /* keep track number of layer in a viarule */
static int viaLayer;           /* keep track number of layer in a via */
static int ndLayer;            /* keep track number of layer in a nondefault */
static int numVia;             /* keep track number of via */
static int viaRuleHasDir;      /* viarule layer has direction construct */
static int viaRuleHasEnc;      /* viarule layer has enclosure construct */
static int ndLayerWidth;       /* keep track if width is set at ndLayer */
static int ndLayerSpace;       /* keep track if spacing is set at ndLayer */
static int isGenerate;         /* keep track if viarule has generate */
static int hasPitch;           /* keep track of pitch in layer */
static int hasWidth;           /* keep track of width in layer */
static int hasDirection;       /* keep track of direction in layer */
static int hasParallel;        /* keep track of parallelrunlength */
static int hasInfluence;       /* keep track of influence */

int spParallelLength;          /* the number of layer parallelrunlength */

lefiNum macroNum;              /* for origin & size callback */

int comp_str(char *s1, int op, char *s2)
{
    int k = strcmp(s1, s2);
    switch (op) {
	case C_EQ: return k == 0;
	case C_NE: return k != 0;
	case C_GT: return k >  0;
	case C_GE: return k >= 0;
	case C_LT: return k <  0;
	case C_LE: return k <= 0;
	}
    return 0;
}
int comp_num(double s1, int op, double s2)
{
    double k = s1 - s2;
    switch (op) {
	case C_EQ: return k == 0;
	case C_NE: return k != 0;
	case C_GT: return k >  0;
	case C_GE: return k >= 0;
	case C_LT: return k <  0;
	case C_LE: return k <= 0;
	}
    return 0;
}
double convert_name2num(char *versionName)
{
    char majorNm[80];
    char minorNm[80];
    char *subMinorNm = NULL;
    char *p1;
    char *versionNm = strdup(versionName);
  
    double major = 0, minor = 0, subMinor = 0;
    double version;

    sscanf(versionNm, "%[^.].%s", majorNm, minorNm);
    if (p1 = strchr(minorNm, '.')) {
       subMinorNm = p1+1; 
       *p1 = '\0';
    }
    major = atof(majorNm);
    minor = atof(minorNm);
    if (subMinorNm)
       subMinor = atof(subMinorNm);

    version = major;
    
    if (minor > 0)
       version = major + minor/10;

    if (subMinor > 0)
       version = version + subMinor/1000;

    lefFree(versionNm);
    return version;
}


/* Line 189 of yacc.c  */
#line 257 "lef.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     K_HISTORY = 258,
     K_ABUT = 259,
     K_ABUTMENT = 260,
     K_ACTIVE = 261,
     K_ANALOG = 262,
     K_ARRAY = 263,
     K_AREA = 264,
     K_BLOCK = 265,
     K_BOTTOMLEFT = 266,
     K_BOTTOMRIGHT = 267,
     K_BY = 268,
     K_CAPACITANCE = 269,
     K_CAPMULTIPLIER = 270,
     K_CLASS = 271,
     K_CLOCK = 272,
     K_CLOCKTYPE = 273,
     K_COLUMNMAJOR = 274,
     K_DESIGNRULEWIDTH = 275,
     K_INFLUENCE = 276,
     K_COMPONENTS = 277,
     K_CORE = 278,
     K_CORNER = 279,
     K_COVER = 280,
     K_CPERSQDIST = 281,
     K_CURRENT = 282,
     K_CURRENTSOURCE = 283,
     K_CUT = 284,
     K_DEFAULT = 285,
     K_DATABASE = 286,
     K_DATA = 287,
     K_DIELECTRIC = 288,
     K_DIRECTION = 289,
     K_DO = 290,
     K_EDGECAPACITANCE = 291,
     K_EEQ = 292,
     K_END = 293,
     K_ENDCAP = 294,
     K_FALL = 295,
     K_FALLCS = 296,
     K_FALLT0 = 297,
     K_FALLSATT1 = 298,
     K_FALLRS = 299,
     K_FALLSATCUR = 300,
     K_FALLTHRESH = 301,
     K_FEEDTHRU = 302,
     K_FIXED = 303,
     K_FOREIGN = 304,
     K_FROMPIN = 305,
     K_GENERATE = 306,
     K_GENERATOR = 307,
     K_GROUND = 308,
     K_HEIGHT = 309,
     K_HORIZONTAL = 310,
     K_INOUT = 311,
     K_INPUT = 312,
     K_INPUTNOISEMARGIN = 313,
     K_COMPONENTPIN = 314,
     K_INTRINSIC = 315,
     K_INVERT = 316,
     K_IRDROP = 317,
     K_ITERATE = 318,
     K_IV_TABLES = 319,
     K_LAYER = 320,
     K_LEAKAGE = 321,
     K_LEQ = 322,
     K_LIBRARY = 323,
     K_MACRO = 324,
     K_MATCH = 325,
     K_MAXDELAY = 326,
     K_MAXLOAD = 327,
     K_METALOVERHANG = 328,
     K_MILLIAMPS = 329,
     K_MILLIWATTS = 330,
     K_MINFEATURE = 331,
     K_MUSTJOIN = 332,
     K_NAMESCASESENSITIVE = 333,
     K_NANOSECONDS = 334,
     K_NETS = 335,
     K_NEW = 336,
     K_NONDEFAULTRULE = 337,
     K_NONINVERT = 338,
     K_NONUNATE = 339,
     K_OBS = 340,
     K_OHMS = 341,
     K_OFFSET = 342,
     K_ORIENTATION = 343,
     K_ORIGIN = 344,
     K_OUTPUT = 345,
     K_OUTPUTNOISEMARGIN = 346,
     K_OUTPUTRESISTANCE = 347,
     K_OVERHANG = 348,
     K_OVERLAP = 349,
     K_OFF = 350,
     K_ON = 351,
     K_OVERLAPS = 352,
     K_PAD = 353,
     K_PATH = 354,
     K_PATTERN = 355,
     K_PICOFARADS = 356,
     K_PIN = 357,
     K_PITCH = 358,
     K_PLACED = 359,
     K_POLYGON = 360,
     K_PORT = 361,
     K_POST = 362,
     K_POWER = 363,
     K_PRE = 364,
     K_PULLDOWNRES = 365,
     K_RECT = 366,
     K_RESISTANCE = 367,
     K_RESISTIVE = 368,
     K_RING = 369,
     K_RISE = 370,
     K_RISECS = 371,
     K_RISERS = 372,
     K_RISESATCUR = 373,
     K_RISETHRESH = 374,
     K_RISESATT1 = 375,
     K_RISET0 = 376,
     K_RISEVOLTAGETHRESHOLD = 377,
     K_FALLVOLTAGETHRESHOLD = 378,
     K_ROUTING = 379,
     K_ROWMAJOR = 380,
     K_RPERSQ = 381,
     K_SAMENET = 382,
     K_SCANUSE = 383,
     K_SHAPE = 384,
     K_SHRINKAGE = 385,
     K_SIGNAL = 386,
     K_SITE = 387,
     K_SIZE = 388,
     K_SOURCE = 389,
     K_SPACER = 390,
     K_SPACING = 391,
     K_SPECIALNETS = 392,
     K_STACK = 393,
     K_START = 394,
     K_STEP = 395,
     K_STOP = 396,
     K_STRUCTURE = 397,
     K_SYMMETRY = 398,
     K_TABLE = 399,
     K_THICKNESS = 400,
     K_TIEHIGH = 401,
     K_TIELOW = 402,
     K_TIEOFFR = 403,
     K_TIME = 404,
     K_TIMING = 405,
     K_TO = 406,
     K_TOPIN = 407,
     K_TOPLEFT = 408,
     K_TOPRIGHT = 409,
     K_TOPOFSTACKONLY = 410,
     K_TRISTATE = 411,
     K_TYPE = 412,
     K_UNATENESS = 413,
     K_UNITS = 414,
     K_USE = 415,
     K_VARIABLE = 416,
     K_VERTICAL = 417,
     K_VHI = 418,
     K_VIA = 419,
     K_VIARULE = 420,
     K_VLO = 421,
     K_VOLTAGE = 422,
     K_VOLTS = 423,
     K_WIDTH = 424,
     K_X = 425,
     K_Y = 426,
     K_R90 = 427,
     T_STRING = 428,
     QSTRING = 429,
     NUMBER = 430,
     K_N = 431,
     K_S = 432,
     K_E = 433,
     K_W = 434,
     K_FN = 435,
     K_FS = 436,
     K_FE = 437,
     K_FW = 438,
     K_R0 = 439,
     K_R180 = 440,
     K_R270 = 441,
     K_MX = 442,
     K_MY = 443,
     K_MXR90 = 444,
     K_MYR90 = 445,
     K_USER = 446,
     K_MASTERSLICE = 447,
     K_ENDMACRO = 448,
     K_ENDMACROPIN = 449,
     K_ENDVIARULE = 450,
     K_ENDVIA = 451,
     K_ENDLAYER = 452,
     K_ENDSITE = 453,
     K_CANPLACE = 454,
     K_CANNOTOCCUPY = 455,
     K_TRACKS = 456,
     K_FLOORPLAN = 457,
     K_GCELLGRID = 458,
     K_DEFAULTCAP = 459,
     K_MINPINS = 460,
     K_WIRECAP = 461,
     K_STABLE = 462,
     K_SETUP = 463,
     K_HOLD = 464,
     K_DEFINE = 465,
     K_DEFINES = 466,
     K_DEFINEB = 467,
     K_IF = 468,
     K_THEN = 469,
     K_ELSE = 470,
     K_FALSE = 471,
     K_TRUE = 472,
     K_EQ = 473,
     K_NE = 474,
     K_LE = 475,
     K_LT = 476,
     K_GE = 477,
     K_GT = 478,
     K_OR = 479,
     K_AND = 480,
     K_NOT = 481,
     K_DELAY = 482,
     K_TABLEDIMENSION = 483,
     K_TABLEAXIS = 484,
     K_TABLEENTRIES = 485,
     K_TRANSITIONTIME = 486,
     K_EXTENSION = 487,
     K_PROPDEF = 488,
     K_STRING = 489,
     K_INTEGER = 490,
     K_REAL = 491,
     K_RANGE = 492,
     K_PROPERTY = 493,
     K_VIRTUAL = 494,
     K_BUSBITCHARS = 495,
     K_VERSION = 496,
     K_BEGINEXT = 497,
     K_ENDEXT = 498,
     K_UNIVERSALNOISEMARGIN = 499,
     K_EDGERATETHRESHOLD1 = 500,
     K_CORRECTIONTABLE = 501,
     K_EDGERATESCALEFACTOR = 502,
     K_EDGERATETHRESHOLD2 = 503,
     K_VICTIMNOISE = 504,
     K_NOISETABLE = 505,
     K_EDGERATE = 506,
     K_VICTIMLENGTH = 507,
     K_CORRECTIONFACTOR = 508,
     K_OUTPUTPINANTENNASIZE = 509,
     K_INPUTPINANTENNASIZE = 510,
     K_INOUTPINANTENNASIZE = 511,
     K_CURRENTDEN = 512,
     K_PWL = 513,
     K_ANTENNALENGTHFACTOR = 514,
     K_TAPERRULE = 515,
     K_DIVIDERCHAR = 516,
     K_ANTENNASIZE = 517,
     K_ANTENNAMETALLENGTH = 518,
     K_ANTENNAMETALAREA = 519,
     K_RISESLEWLIMIT = 520,
     K_FALLSLEWLIMIT = 521,
     K_FUNCTION = 522,
     K_BUFFER = 523,
     K_INVERTER = 524,
     K_NAMEMAPSTRING = 525,
     K_NOWIREEXTENSIONATPIN = 526,
     K_WIREEXTENSION = 527,
     K_MESSAGE = 528,
     K_CREATEFILE = 529,
     K_OPENFILE = 530,
     K_CLOSEFILE = 531,
     K_WARNING = 532,
     K_ERROR = 533,
     K_FATALERROR = 534,
     K_RECOVERY = 535,
     K_SKEW = 536,
     K_ANYEDGE = 537,
     K_POSEDGE = 538,
     K_NEGEDGE = 539,
     K_SDFCONDSTART = 540,
     K_SDFCONDEND = 541,
     K_SDFCOND = 542,
     K_MPWH = 543,
     K_MPWL = 544,
     K_PERIOD = 545,
     K_ACCURRENTDENSITY = 546,
     K_DCCURRENTDENSITY = 547,
     K_AVERAGE = 548,
     K_PEAK = 549,
     K_RMS = 550,
     K_FREQUENCY = 551,
     K_CUTAREA = 552,
     K_MEGAHERTZ = 553,
     K_USELENGTHTHRESHOLD = 554,
     K_LENGTHTHRESHOLD = 555,
     K_ANTENNAINPUTGATEAREA = 556,
     K_ANTENNAINOUTDIFFAREA = 557,
     K_ANTENNAOUTPUTDIFFAREA = 558,
     K_ANTENNAAREARATIO = 559,
     K_ANTENNADIFFAREARATIO = 560,
     K_ANTENNACUMAREARATIO = 561,
     K_ANTENNACUMDIFFAREARATIO = 562,
     K_ANTENNAAREAFACTOR = 563,
     K_ANTENNASIDEAREARATIO = 564,
     K_ANTENNADIFFSIDEAREARATIO = 565,
     K_ANTENNACUMSIDEAREARATIO = 566,
     K_ANTENNACUMDIFFSIDEAREARATIO = 567,
     K_ANTENNASIDEAREAFACTOR = 568,
     K_DIFFUSEONLY = 569,
     K_MANUFACTURINGGRID = 570,
     K_ANTENNACELL = 571,
     K_CLEARANCEMEASURE = 572,
     K_EUCLIDEAN = 573,
     K_MAXXY = 574,
     K_USEMINSPACING = 575,
     K_ROWMINSPACING = 576,
     K_ROWABUTSPACING = 577,
     K_FLIP = 578,
     K_NONE = 579,
     K_ANTENNAPARTIALMETALAREA = 580,
     K_ANTENNAPARTIALMETALSIDEAREA = 581,
     K_ANTENNAGATEAREA = 582,
     K_ANTENNADIFFAREA = 583,
     K_ANTENNAMAXAREACAR = 584,
     K_ANTENNAMAXSIDEAREACAR = 585,
     K_ANTENNAPARTIALCUTAREA = 586,
     K_ANTENNAMAXCUTCAR = 587,
     K_SLOTWIREWIDTH = 588,
     K_SLOTWIRELENGTH = 589,
     K_SLOTWIDTH = 590,
     K_SLOTLENGTH = 591,
     K_MAXADJACENTSLOTSPACING = 592,
     K_MAXCOAXIALSLOTSPACING = 593,
     K_MAXEDGESLOTSPACING = 594,
     K_SPLITWIREWIDTH = 595,
     K_MINIMUMDENSITY = 596,
     K_MAXIMUMDENSITY = 597,
     K_DENSITYCHECKWINDOW = 598,
     K_DENSITYCHECKSTEP = 599,
     K_FILLACTIVESPACING = 600,
     K_MINIMUMCUT = 601,
     K_ADJACENTCUTS = 602,
     K_ANTENNAMODEL = 603,
     K_BUMP = 604,
     K_ENCLOSURE = 605,
     K_FROMABOVE = 606,
     K_FROMBELOW = 607,
     K_IMPLANT = 608,
     K_LENGTH = 609,
     K_MAXVIASTACK = 610,
     K_AREAIO = 611,
     K_BLACKBOX = 612,
     K_MAXWIDTH = 613,
     K_MINENCLOSEDAREA = 614,
     K_MINSTEP = 615,
     K_ORIENT = 616,
     K_OXIDE1 = 617,
     K_OXIDE2 = 618,
     K_OXIDE3 = 619,
     K_OXIDE4 = 620,
     K_PARALLELRUNLENGTH = 621,
     K_MINWIDTH = 622,
     K_PROTRUSIONWIDTH = 623,
     K_SPACINGTABLE = 624,
     K_WITHIN = 625,
     IF = 626,
     LNOT = 627,
     UMINUS = 628
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 190 "lef.y"

	double dval ;
	int       integer ;
	char *    string ;
	lefPOINT  pt;



/* Line 214 of yacc.c  */
#line 675 "lef.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 687 "lef.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1709

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  385
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  367
/* YYNRULES -- Number of rules.  */
#define YYNRULES  860
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1727

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   628

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     382,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     379,   380,   375,   374,     2,   373,     2,   376,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   378,
     383,   381,   384,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   377
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     6,     7,    12,    16,    20,    21,    24,
      26,    29,    31,    33,    35,    37,    39,    41,    43,    45,
      47,    49,    51,    53,    55,    57,    59,    61,    63,    65,
      67,    69,    71,    73,    75,    77,    79,    81,    83,    85,
      87,    89,    91,    93,    95,    97,    99,   101,   103,   105,
     107,   111,   115,   119,   123,   127,   132,   136,   138,   140,
     142,   144,   146,   148,   153,   155,   156,   159,   164,   169,
     174,   179,   184,   189,   190,   196,   201,   205,   206,   210,
     211,   215,   216,   219,   223,   227,   231,   235,   239,   240,
     247,   251,   256,   264,   269,   277,   281,   285,   289,   293,
     297,   301,   305,   309,   316,   323,   324,   329,   330,   336,
     341,   342,   348,   353,   357,   358,   363,   367,   368,   373,
     374,   380,   384,   385,   390,   394,   395,   400,   401,   407,
     408,   413,   417,   421,   425,   429,   433,   437,   441,   445,
     449,   453,   458,   462,   466,   470,   474,   475,   481,   482,
     483,   493,   497,   505,   506,   511,   512,   513,   514,   515,
     527,   528,   538,   540,   541,   549,   550,   553,   555,   556,
     558,   560,   562,   564,   565,   568,   569,   575,   576,   582,
     583,   588,   589,   595,   596,   599,   600,   606,   607,   612,
     613,   619,   620,   623,   625,   628,   631,   634,   637,   639,
     642,   647,   649,   652,   657,   659,   662,   667,   669,   671,
     673,   675,   677,   679,   681,   683,   684,   686,   688,   689,
     694,   695,   698,   700,   702,   704,   706,   707,   710,   711,
     716,   717,   720,   727,   731,   732,   740,   744,   746,   749,
     753,   754,   757,   759,   761,   765,   766,   771,   773,   775,
     778,   781,   784,   787,   790,   794,   799,   803,   804,   808,
     810,   812,   814,   816,   818,   820,   822,   824,   826,   828,
     830,   832,   834,   836,   838,   840,   843,   844,   849,   850,
     853,   858,   859,   863,   864,   868,   874,   875,   882,   884,
     887,   888,   890,   892,   895,   896,   901,   903,   906,   909,
     912,   915,   918,   919,   922,   926,   927,   932,   933,   936,
     940,   944,   949,   955,   960,   966,   970,   974,   978,   979,
     983,   987,   989,   992,   994,   997,  1003,  1010,  1012,  1016,
    1018,  1021,  1022,  1025,  1029,  1030,  1033,  1036,  1039,  1044,
    1048,  1049,  1055,  1057,  1060,  1061,  1064,  1066,  1068,  1070,
    1072,  1073,  1078,  1080,  1083,  1086,  1089,  1092,  1093,  1100,
    1101,  1104,  1108,  1112,  1116,  1121,  1126,  1130,  1134,  1135,
    1139,  1140,  1144,  1145,  1148,  1154,  1156,  1158,  1162,  1166,
    1170,  1174,  1175,  1178,  1180,  1182,  1184,  1187,  1192,  1193,
    1198,  1199,  1203,  1204,  1208,  1209,  1212,  1214,  1216,  1218,
    1220,  1222,  1224,  1226,  1228,  1230,  1232,  1234,  1236,  1238,
    1242,  1246,  1248,  1250,  1252,  1253,  1258,  1260,  1263,  1267,
    1268,  1271,  1273,  1275,  1277,  1280,  1283,  1286,  1290,  1292,
    1295,  1297,  1299,  1302,  1304,  1306,  1308,  1311,  1313,  1315,
    1318,  1321,  1323,  1325,  1327,  1329,  1331,  1333,  1335,  1337,
    1339,  1341,  1343,  1345,  1347,  1349,  1351,  1353,  1355,  1359,
    1364,  1368,  1372,  1376,  1380,  1384,  1387,  1391,  1396,  1400,
    1401,  1406,  1407,  1412,  1416,  1420,  1422,  1424,  1430,  1434,
    1435,  1439,  1440,  1444,  1445,  1448,  1451,  1455,  1460,  1464,
    1469,  1475,  1476,  1481,  1485,  1487,  1491,  1495,  1499,  1503,
    1507,  1511,  1515,  1519,  1523,  1527,  1531,  1532,  1537,  1538,
    1544,  1545,  1551,  1552,  1558,  1562,  1566,  1570,  1574,  1578,
    1582,  1586,  1590,  1594,  1599,  1603,  1604,  1609,  1614,  1617,
    1622,  1627,  1632,  1636,  1640,  1645,  1650,  1655,  1660,  1665,
    1670,  1675,  1680,  1681,  1686,  1688,  1690,  1692,  1694,  1696,
    1699,  1702,  1705,  1708,  1712,  1716,  1721,  1725,  1729,  1731,
    1732,  1736,  1738,  1740,  1742,  1744,  1746,  1748,  1750,  1752,
    1754,  1756,  1757,  1759,  1761,  1763,  1766,  1767,  1768,  1775,
    1779,  1784,  1791,  1796,  1803,  1811,  1821,  1823,  1824,  1827,
    1828,  1831,  1834,  1836,  1838,  1839,  1842,  1843,  1849,  1850,
    1858,  1866,  1878,  1879,  1880,  1891,  1892,  1893,  1904,  1911,
    1918,  1919,  1922,  1924,  1931,  1938,  1942,  1945,  1947,  1948,
    1953,  1957,  1959,  1962,  1963,  1966,  1967,  1972,  1973,  1978,
    1979,  1990,  2000,  2004,  2008,  2013,  2018,  2023,  2028,  2033,
    2038,  2043,  2048,  2052,  2060,  2069,  2076,  2080,  2084,  2088,
    2091,  2093,  2095,  2097,  2099,  2101,  2103,  2105,  2107,  2109,
    2111,  2113,  2115,  2117,  2119,  2121,  2123,  2126,  2132,  2134,
    2137,  2138,  2143,  2151,  2153,  2155,  2157,  2159,  2161,  2163,
    2166,  2168,  2171,  2172,  2177,  2178,  2182,  2183,  2187,  2188,
    2191,  2192,  2197,  2198,  2203,  2204,  2209,  2210,  2215,  2220,
    2221,  2226,  2232,  2234,  2237,  2238,  2241,  2242,  2247,  2248,
    2253,  2254,  2257,  2263,  2264,  2271,  2272,  2279,  2280,  2287,
    2288,  2295,  2296,  2303,  2304,  2306,  2308,  2310,  2313,  2315,
    2318,  2322,  2326,  2330,  2334,  2337,  2341,  2348,  2350,  2354,
    2358,  2362,  2366,  2370,  2374,  2378,  2382,  2386,  2390,  2393,
    2397,  2404,  2406,  2408,  2412,  2416,  2423,  2425,  2427,  2429,
    2431,  2433,  2435,  2437,  2439,  2441,  2443,  2444,  2450,  2451,
    2454,  2455,  2461,  2462,  2468,  2469,  2475,  2476,  2482,  2483,
    2489,  2490,  2496,  2497,  2503,  2504,  2510,  2511,  2516,  2517,
    2522,  2524,  2527,  2530,  2531,  2532,  2538,  2541,  2547,  2548,
    2550,  2553,  2559,  2563,  2564,  2568,  2569,  2571,  2572,  2573,
    2577,  2582,  2583,  2584,  2588,  2589,  2593,  2598,  2602,  2606,
    2610,  2611,  2619,  2622,  2624,  2627,  2631,  2633,  2634,  2640,
    2642,  2645,  2647,  2650,  2651,  2659,  2661,  2664,  2665,  2673,
    2676,  2678,  2681,  2685,  2687,  2688,  2694,  2696,  2699,  2701,
    2704,  2705,  2713,  2715,  2718,  2722,  2726,  2730,  2734,  2738,
    2742
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     386,     0,    -1,   391,   392,    -1,    -1,   241,   388,   173,
     378,    -1,   261,   174,   378,    -1,   240,   174,   378,    -1,
      -1,   391,   393,    -1,     1,    -1,    38,    68,    -1,   387,
      -1,   390,    -1,   394,    -1,   402,    -1,   407,    -1,   474,
      -1,   495,    -1,   496,    -1,   389,    -1,   395,    -1,   676,
      -1,   513,    -1,   528,    -1,   527,    -1,   519,    -1,   541,
      -1,   553,    -1,   656,    -1,   680,    -1,   529,    -1,   691,
      -1,   717,    -1,   718,    -1,   720,    -1,   719,    -1,   721,
      -1,   733,    -1,   745,    -1,   746,    -1,   747,    -1,   751,
      -1,   748,    -1,   749,    -1,   750,    -1,   396,    -1,   397,
      -1,   398,    -1,   472,    -1,   678,    -1,    78,    96,   378,
      -1,    78,    95,   378,    -1,   271,    96,   378,    -1,   271,
      95,   378,    -1,   315,   175,   378,    -1,   320,   400,   401,
     378,    -1,   317,   399,   378,    -1,   319,    -1,   318,    -1,
      85,    -1,   102,    -1,    96,    -1,    95,    -1,   403,   404,
      38,   159,    -1,   159,    -1,    -1,   404,   405,    -1,   149,
      79,   175,   378,    -1,    14,   101,   175,   378,    -1,   112,
      86,   175,   378,    -1,   108,    75,   175,   378,    -1,    27,
      74,   175,   378,    -1,   167,   168,   175,   378,    -1,    -1,
      31,   406,   173,   175,   378,    -1,   296,   298,   175,   378,
      -1,   408,   412,   410,    -1,    -1,    65,   409,   173,    -1,
      -1,    38,   411,   173,    -1,    -1,   412,   413,    -1,   157,
     461,   378,    -1,   103,   175,   378,    -1,    87,   175,   378,
      -1,   169,   175,   378,    -1,     9,   175,   378,    -1,    -1,
     136,   175,   414,   706,   711,   378,    -1,    34,   462,   378,
      -1,   112,   126,   175,   378,    -1,   112,   126,   258,   379,
     459,   380,   378,    -1,    14,    26,   175,   378,    -1,    14,
      26,   258,   379,   457,   380,   378,    -1,    54,   175,   378,
      -1,   272,   175,   378,    -1,   145,   175,   378,    -1,   130,
     175,   378,    -1,    15,   175,   378,    -1,    36,   175,   378,
      -1,   259,   175,   378,    -1,   257,   175,   378,    -1,   257,
     258,   379,   455,   380,   378,    -1,   257,   379,   175,   175,
     380,   378,    -1,    -1,   238,   415,   453,   378,    -1,    -1,
     291,   440,   416,   441,   378,    -1,   291,   440,   175,   378,
      -1,    -1,   292,   293,   417,   447,   378,    -1,   292,   293,
     175,   378,    -1,   304,   175,   378,    -1,    -1,   305,   418,
     435,   378,    -1,   306,   175,   378,    -1,    -1,   307,   419,
     435,   378,    -1,    -1,   308,   175,   420,   439,   378,    -1,
     309,   175,   378,    -1,    -1,   310,   421,   435,   378,    -1,
     311,   175,   378,    -1,    -1,   312,   422,   435,   378,    -1,
      -1,   313,   175,   423,   439,   378,    -1,    -1,   348,   424,
     466,   378,    -1,   333,   175,   378,    -1,   334,   175,   378,
      -1,   335,   175,   378,    -1,   336,   175,   378,    -1,   337,
     175,   378,    -1,   338,   175,   378,    -1,   339,   175,   378,
      -1,   340,   175,   378,    -1,   341,   175,   378,    -1,   342,
     175,   378,    -1,   343,   175,   175,   378,    -1,   344,   175,
     378,    -1,   345,   175,   378,    -1,   358,   175,   378,    -1,
     367,   175,   378,    -1,    -1,   359,   175,   425,   465,   378,
      -1,    -1,    -1,   346,   175,   169,   175,   426,   463,   427,
     464,   378,    -1,   360,   175,   378,    -1,   368,   175,   354,
     175,   169,   175,   378,    -1,    -1,   369,   428,   429,   378,
      -1,    -1,    -1,    -1,    -1,   366,   175,   430,   452,   431,
     169,   175,   432,   452,   433,   467,    -1,    -1,    21,   169,
     175,   370,   175,   136,   175,   434,   470,    -1,   175,    -1,
      -1,   258,   379,   552,   552,   436,   437,   380,    -1,    -1,
     437,   438,    -1,   552,    -1,    -1,   314,    -1,   294,    -1,
     293,    -1,   295,    -1,    -1,   441,   442,    -1,    -1,   296,
     175,   443,   452,   378,    -1,    -1,   297,   175,   444,   452,
     378,    -1,    -1,   230,   175,   445,   452,    -1,    -1,   169,
     175,   446,   452,   378,    -1,    -1,   447,   448,    -1,    -1,
     297,   175,   449,   452,   378,    -1,    -1,   230,   175,   450,
     452,    -1,    -1,   169,   175,   451,   452,   378,    -1,    -1,
     452,   175,    -1,   454,    -1,   453,   454,    -1,   173,   173,
      -1,   173,   174,    -1,   173,   175,    -1,   456,    -1,   455,
     456,    -1,   379,   175,   175,   380,    -1,   458,    -1,   457,
     458,    -1,   379,   175,   175,   380,    -1,   460,    -1,   459,
     460,    -1,   379,   175,   175,   380,    -1,   124,    -1,    29,
      -1,    94,    -1,   192,    -1,   239,    -1,   353,    -1,    55,
      -1,   162,    -1,    -1,   351,    -1,   352,    -1,    -1,   354,
     175,   370,   175,    -1,    -1,   169,   175,    -1,   362,    -1,
     363,    -1,   364,    -1,   365,    -1,    -1,   467,   468,    -1,
      -1,   169,   175,   469,   452,    -1,    -1,   470,   471,    -1,
     169,   175,   370,   175,   136,   175,    -1,   355,   175,   378,
      -1,    -1,   355,   175,   237,   473,   173,   173,   378,    -1,
     476,   477,   491,    -1,   164,    -1,   475,   173,    -1,   475,
     173,    30,    -1,    -1,   477,   478,    -1,   482,    -1,   486,
      -1,   112,   175,   378,    -1,    -1,   238,   479,   480,   378,
      -1,   155,    -1,   481,    -1,   480,   481,    -1,   173,   175,
      -1,   173,   174,    -1,   173,   173,    -1,   483,   378,    -1,
     483,   552,   378,    -1,   483,   552,   485,   378,    -1,   483,
     485,   378,    -1,    -1,    49,   484,   173,    -1,   176,    -1,
     179,    -1,   177,    -1,   178,    -1,   180,    -1,   183,    -1,
     181,    -1,   182,    -1,   184,    -1,   172,    -1,   185,    -1,
     186,    -1,   188,    -1,   190,    -1,   187,    -1,   189,    -1,
     487,   489,    -1,    -1,    65,   488,   173,   378,    -1,    -1,
     489,   490,    -1,   111,   552,   552,   378,    -1,    -1,    38,
     492,   173,    -1,    -1,   165,   494,   173,    -1,   493,   498,
     505,   499,   511,    -1,    -1,   493,    51,   497,   498,   499,
     511,    -1,   504,    -1,   498,   504,    -1,    -1,   500,    -1,
     501,    -1,   500,   501,    -1,    -1,   238,   502,   503,   378,
      -1,   501,    -1,   503,   501,    -1,   173,   173,    -1,   173,
     174,    -1,   173,   175,    -1,   507,   509,    -1,    -1,   505,
     506,    -1,   475,   173,   378,    -1,    -1,    65,   508,   173,
     378,    -1,    -1,   509,   510,    -1,    34,    55,   378,    -1,
      34,   162,   378,    -1,   350,   175,   175,   378,    -1,   169,
     175,   151,   175,   378,    -1,   111,   552,   552,   378,    -1,
     136,   175,    13,   175,   378,    -1,   112,   175,   378,    -1,
      93,   175,   378,    -1,    73,   175,   378,    -1,    -1,    38,
     512,   173,    -1,   514,   516,   515,    -1,   136,    -1,    38,
     136,    -1,   517,    -1,   516,   517,    -1,   518,   173,   173,
     175,   378,    -1,   518,   173,   173,   175,   138,   378,    -1,
     127,    -1,   520,   522,   521,    -1,    62,    -1,    38,    62,
      -1,    -1,   522,   523,    -1,   526,   524,   378,    -1,    -1,
     524,   525,    -1,   175,   175,    -1,   144,   173,    -1,    76,
     175,   175,   378,    -1,    33,   175,   378,    -1,    -1,    82,
     173,   530,   532,   531,    -1,    38,    -1,    38,   173,    -1,
      -1,   532,   533,    -1,   537,    -1,   474,    -1,   513,    -1,
     534,    -1,    -1,   238,   535,   536,   378,    -1,   534,    -1,
     536,   534,    -1,   173,   173,    -1,   173,   174,    -1,   173,
     175,    -1,    -1,    65,   173,   538,   539,    38,   173,    -1,
      -1,   539,   540,    -1,   169,   175,   378,    -1,   136,   175,
     378,    -1,   272,   175,   378,    -1,   112,   126,   175,   378,
      -1,    14,    26,   175,   378,    -1,    36,   175,   378,    -1,
     542,   546,   544,    -1,    -1,   132,   543,   173,    -1,    -1,
      38,   545,   173,    -1,    -1,   546,   547,    -1,   133,   175,
      13,   175,   378,    -1,   549,    -1,   548,    -1,    16,    98,
     378,    -1,    16,    23,   378,    -1,    16,   239,   378,    -1,
     143,   550,   378,    -1,    -1,   550,   551,    -1,   170,    -1,
     171,    -1,   172,    -1,   175,   175,    -1,   379,   175,   175,
     380,    -1,    -1,   555,   559,   554,   557,    -1,    -1,    69,
     556,   173,    -1,    -1,    38,   558,   173,    -1,    -1,   559,
     560,    -1,   567,    -1,   572,    -1,   573,    -1,   574,    -1,
     563,    -1,   576,    -1,   575,    -1,   577,    -1,   578,    -1,
     580,    -1,   585,    -1,   582,    -1,   586,    -1,   267,   268,
     378,    -1,   267,   269,   378,    -1,   631,    -1,   633,    -1,
     635,    -1,    -1,   238,   561,   562,   378,    -1,   566,    -1,
     562,   566,    -1,   143,   564,   378,    -1,    -1,   564,   565,
      -1,   170,    -1,   171,    -1,   172,    -1,   173,   175,    -1,
     173,   174,    -1,   173,   173,    -1,    16,   568,   378,    -1,
      25,    -1,    25,   349,    -1,   114,    -1,    10,    -1,    10,
     357,    -1,   324,    -1,    98,    -1,   239,    -1,    98,   569,
      -1,    23,    -1,    24,    -1,    23,   570,    -1,    39,   571,
      -1,    57,    -1,    90,    -1,    56,    -1,   108,    -1,   135,
      -1,   356,    -1,    47,    -1,   146,    -1,   147,    -1,   135,
      -1,   316,    -1,   109,    -1,   107,    -1,   153,    -1,   154,
      -1,    11,    -1,    12,    -1,    52,   173,   378,    -1,    51,
     173,   173,   378,    -1,   134,   191,   378,    -1,   134,    51,
     378,    -1,   134,    10,   378,    -1,   108,   175,   378,    -1,
      89,   552,   378,    -1,   483,   378,    -1,   483,   552,   378,
      -1,   483,   552,   485,   378,    -1,   483,   485,   378,    -1,
      -1,    37,   579,   173,   378,    -1,    -1,    67,   581,   173,
     378,    -1,   583,   173,   378,    -1,   583,   622,   378,    -1,
     132,    -1,   132,    -1,   133,   175,    13,   175,   378,    -1,
     587,   591,   589,    -1,    -1,   102,   588,   173,    -1,    -1,
      38,   590,   173,    -1,    -1,   591,   592,    -1,   483,   378,
      -1,   483,   552,   378,    -1,   483,   552,   485,   378,    -1,
     483,   142,   378,    -1,   483,   142,   552,   378,    -1,   483,
     142,   552,   485,   378,    -1,    -1,    67,   593,   173,   378,
      -1,   108,   175,   378,    -1,   603,    -1,   160,   606,   378,
      -1,   128,   607,   378,    -1,    66,   175,   378,    -1,   119,
     175,   378,    -1,    46,   175,   378,    -1,   118,   175,   378,
      -1,    45,   175,   378,    -1,   166,   175,   378,    -1,   163,
     175,   378,    -1,   148,   175,   378,    -1,   129,   608,   378,
      -1,    -1,    77,   594,   173,   378,    -1,    -1,    91,   595,
     175,   175,   378,    -1,    -1,    92,   596,   175,   175,   378,
      -1,    -1,    58,   597,   175,   175,   378,    -1,    14,   175,
     378,    -1,    71,   175,   378,    -1,    72,   175,   378,    -1,
     112,   175,   378,    -1,   110,   175,   378,    -1,    28,     6,
     378,    -1,    28,   113,   378,    -1,   122,   175,   378,    -1,
     123,   175,   378,    -1,    64,   173,   173,   378,    -1,   260,
     173,   378,    -1,    -1,   238,   598,   601,   378,    -1,   604,
     605,   609,    38,    -1,   604,    38,    -1,   262,   175,   713,
     378,    -1,   264,   175,   713,   378,    -1,   263,   175,   713,
     378,    -1,   265,   175,   378,    -1,   266,   175,   378,    -1,
     325,   175,   713,   378,    -1,   326,   175,   713,   378,    -1,
     331,   175,   713,   378,    -1,   328,   175,   713,   378,    -1,
     327,   175,   713,   378,    -1,   329,   175,   715,   378,    -1,
     330,   175,   715,   378,    -1,   332,   175,   715,   378,    -1,
      -1,   348,   599,   600,   378,    -1,   362,    -1,   363,    -1,
     364,    -1,   365,    -1,   602,    -1,   601,   602,    -1,   173,
     175,    -1,   173,   174,    -1,   173,   173,    -1,    34,    57,
     378,    -1,    34,    90,   378,    -1,    34,    90,   156,   378,
      -1,    34,    56,   378,    -1,    34,    47,   378,    -1,   106,
      -1,    -1,    16,   568,   378,    -1,   131,    -1,     7,    -1,
     108,    -1,    53,    -1,    17,    -1,    32,    -1,    57,    -1,
      90,    -1,   139,    -1,   141,    -1,    -1,     5,    -1,   114,
      -1,    47,    -1,   610,   613,    -1,    -1,    -1,    65,   611,
     173,   612,   614,   378,    -1,   169,   175,   378,    -1,    99,
     615,   617,   378,    -1,    99,    63,   615,   617,   621,   378,
      -1,   111,   552,   552,   378,    -1,   111,    63,   552,   552,
     621,   378,    -1,   105,   615,   616,   616,   616,   617,   378,
      -1,   105,    63,   615,   616,   616,   616,   617,   621,   378,
      -1,   618,    -1,    -1,   613,   610,    -1,    -1,   136,   175,
      -1,    20,   175,    -1,   552,    -1,   552,    -1,    -1,   616,
     617,    -1,    -1,   164,   552,   619,   173,   378,    -1,    -1,
     164,    63,   552,   620,   173,   621,   378,    -1,    35,   175,
      13,   175,   140,   175,   175,    -1,   173,   175,   175,   485,
      35,   175,    13,   175,   140,   175,   175,    -1,    -1,    -1,
     170,   175,    35,   175,   140,   175,   624,    65,   625,   628,
      -1,    -1,    -1,   171,   175,    35,   175,   140,   175,   626,
      65,   627,   628,    -1,   170,   175,    35,   175,   140,   175,
      -1,   171,   175,    35,   175,   140,   175,    -1,    -1,   628,
     629,    -1,   173,    -1,   170,   175,    35,   175,   140,   175,
      -1,   171,   175,    35,   175,   140,   175,    -1,   632,   609,
      38,    -1,   632,    38,    -1,    85,    -1,    -1,    18,   634,
     173,   378,    -1,   636,   638,   637,    -1,   150,    -1,    38,
     150,    -1,    -1,   638,   639,    -1,    -1,    50,   640,   654,
     378,    -1,    -1,   152,   641,   655,   378,    -1,    -1,   652,
      60,   175,   175,   642,   651,   161,   175,   175,   378,    -1,
     652,   647,   158,   653,   228,   175,   175,   175,   378,    -1,
     229,   650,   378,    -1,   230,   648,   378,    -1,   117,   175,
     175,   378,    -1,    44,   175,   175,   378,    -1,   116,   175,
     175,   378,    -1,    41,   175,   175,   378,    -1,   120,   175,
     175,   378,    -1,    43,   175,   175,   378,    -1,   121,   175,
     175,   378,    -1,    42,   175,   175,   378,    -1,   158,   653,
     378,    -1,   207,   208,   175,   209,   175,   652,   378,    -1,
     644,   645,   646,   228,   175,   175,   175,   378,    -1,   643,
     228,   175,   175,   175,   378,    -1,   285,   174,   378,    -1,
     286,   174,   378,    -1,   287,   174,   378,    -1,   232,   378,
      -1,   288,    -1,   289,    -1,   290,    -1,   208,    -1,   209,
      -1,   280,    -1,   281,    -1,   282,    -1,   283,    -1,   284,
      -1,   282,    -1,   283,    -1,   284,    -1,   227,    -1,   231,
      -1,   649,    -1,   648,   649,    -1,   379,   175,   175,   175,
     380,    -1,   175,    -1,   650,   175,    -1,    -1,   175,   175,
     175,   175,    -1,   175,   175,   175,   175,   175,   175,   175,
      -1,   115,    -1,    40,    -1,    61,    -1,    83,    -1,    84,
      -1,   173,    -1,   654,   173,    -1,   173,    -1,   655,   173,
      -1,    -1,   658,   662,   657,   660,    -1,    -1,     8,   659,
     173,    -1,    -1,    38,   661,   173,    -1,    -1,   662,   663,
      -1,    -1,   584,   664,   622,   378,    -1,    -1,   199,   665,
     622,   378,    -1,    -1,   200,   666,   622,   378,    -1,    -1,
     201,   667,   623,   378,    -1,   669,   670,    38,   173,    -1,
      -1,   203,   668,   630,   378,    -1,   204,   175,   674,    38,
     204,    -1,   680,    -1,   202,   173,    -1,    -1,   670,   671,
      -1,    -1,   199,   672,   622,   378,    -1,    -1,   200,   673,
     622,   378,    -1,    -1,   674,   675,    -1,   205,   175,   206,
     175,   378,    -1,    -1,   273,   677,   173,   381,   689,   684,
      -1,    -1,   274,   679,   173,   381,   689,   684,    -1,    -1,
     210,   681,   173,   381,   687,   684,    -1,    -1,   211,   682,
     173,   381,   689,   684,    -1,    -1,   212,   683,   173,   381,
     688,   684,    -1,    -1,   378,    -1,   382,    -1,   214,    -1,
     382,   214,    -1,   215,    -1,   382,   215,    -1,   687,   374,
     687,    -1,   687,   373,   687,    -1,   687,   375,   687,    -1,
     687,   376,   687,    -1,   373,   687,    -1,   379,   687,   380,
      -1,   213,   688,   685,   687,   686,   687,    -1,   175,    -1,
     687,   690,   687,    -1,   687,   225,   687,    -1,   687,   224,
     687,    -1,   689,   690,   689,    -1,   689,   225,   689,    -1,
     689,   224,   689,    -1,   688,   218,   688,    -1,   688,   219,
     688,    -1,   688,   225,   688,    -1,   688,   224,   688,    -1,
     226,   688,    -1,   379,   688,   380,    -1,   213,   688,   685,
     688,   686,   688,    -1,   217,    -1,   216,    -1,   689,   374,
     689,    -1,   379,   689,   380,    -1,   213,   688,   685,   689,
     686,   689,    -1,   174,    -1,   220,    -1,   221,    -1,   222,
      -1,   223,    -1,   218,    -1,   219,    -1,   381,    -1,   383,
      -1,   384,    -1,    -1,   233,   692,   693,    38,   233,    -1,
      -1,   693,   694,    -1,    -1,    68,   695,   173,   703,   378,
      -1,    -1,    59,   696,   173,   703,   378,    -1,    -1,   102,
     697,   173,   703,   378,    -1,    -1,    69,   698,   173,   703,
     378,    -1,    -1,   164,   699,   173,   703,   378,    -1,    -1,
     165,   700,   173,   703,   378,    -1,    -1,    65,   701,   173,
     703,   378,    -1,    -1,    82,   702,   173,   703,   378,    -1,
      -1,   235,   704,   709,   710,    -1,    -1,   236,   705,   709,
     710,    -1,   234,    -1,   234,   174,    -1,   270,   173,    -1,
      -1,    -1,   237,   175,   175,   707,   708,    -1,   300,   175,
      -1,   300,   175,   237,   175,   175,    -1,    -1,   299,    -1,
      21,   175,    -1,    21,   175,   237,   175,   175,    -1,   237,
     175,   175,    -1,    -1,   237,   175,   175,    -1,    -1,   175,
      -1,    -1,    -1,    65,   712,   173,    -1,   347,   175,   370,
     175,    -1,    -1,    -1,    65,   714,   173,    -1,    -1,    65,
     716,   173,    -1,   244,   175,   175,   378,    -1,   245,   175,
     378,    -1,   248,   175,   378,    -1,   247,   175,   378,    -1,
      -1,   250,   175,   722,   378,   724,   723,   684,    -1,    38,
     250,    -1,   725,    -1,   724,   725,    -1,   251,   175,   378,
      -1,   726,    -1,    -1,    92,   727,   728,   378,   729,    -1,
     175,    -1,   728,   175,    -1,   730,    -1,   729,   730,    -1,
      -1,   252,   175,   378,   731,   249,   732,   378,    -1,   175,
      -1,   732,   175,    -1,    -1,   246,   175,   378,   734,   736,
     735,   684,    -1,    38,   246,    -1,   737,    -1,   736,   737,
      -1,   251,   175,   378,    -1,   738,    -1,    -1,    92,   739,
     740,   378,   741,    -1,   175,    -1,   740,   175,    -1,   742,
      -1,   741,   742,    -1,    -1,   252,   175,   378,   743,   253,
     744,   378,    -1,   175,    -1,   744,   175,    -1,   255,   175,
     378,    -1,   254,   175,   378,    -1,   256,   175,   378,    -1,
     301,   175,   378,    -1,   302,   175,   378,    -1,   303,   175,
     378,    -1,   242,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   313,   313,   316,   316,   335,   341,   347,   348,   349,
     352,   373,   373,   373,   373,   374,   374,   374,   374,   374,
     375,   375,   376,   376,   376,   376,   376,   376,   376,   377,
     377,   377,   378,   378,   379,   379,   380,   380,   380,   381,
     381,   381,   382,   382,   382,   382,   383,   383,   383,   384,
     387,   394,   402,   404,   407,   410,   418,   422,   423,   426,
     427,   430,   431,   433,   436,   439,   440,   443,   445,   447,
     449,   451,   453,   455,   455,   457,   460,   463,   463,   485,
     485,   507,   508,   512,   514,   519,   523,   528,   533,   532,
     554,   564,   572,   579,   587,   594,   602,   610,   618,   626,
     634,   653,   668,   676,   683,   691,   691,   696,   695,   704,
     716,   715,   724,   736,   755,   754,   773,   792,   791,   811,
     810,   821,   840,   839,   858,   877,   876,   896,   895,   916,
     915,   934,   944,   954,   964,   974,   984,   994,  1004,  1014,
    1024,  1034,  1044,  1054,  1064,  1077,  1091,  1090,  1100,  1104,
    1099,  1108,  1112,  1121,  1120,  1137,  1150,  1155,  1161,  1136,
    1170,  1169,  1187,  1191,  1190,  1203,  1204,  1208,  1213,  1214,
    1232,  1233,  1234,  1236,  1237,  1241,  1240,  1245,  1244,  1249,
    1248,  1253,  1252,  1257,  1258,  1262,  1261,  1266,  1265,  1270,
    1269,  1274,  1275,  1279,  1280,  1284,  1292,  1300,  1312,  1314,
    1317,  1321,  1322,  1325,  1329,  1330,  1333,  1337,  1338,  1339,
    1340,  1341,  1342,  1345,  1346,  1348,  1349,  1358,  1367,  1368,
    1378,  1379,  1386,  1391,  1396,  1401,  1408,  1409,  1413,  1412,
    1422,  1423,  1426,  1429,  1448,  1448,  1467,  1471,  1474,  1482,
    1490,  1492,  1496,  1498,  1500,  1502,  1502,  1504,  1508,  1509,
    1513,  1523,  1531,  1541,  1543,  1545,  1547,  1550,  1550,  1554,
    1555,  1556,  1557,  1558,  1559,  1560,  1561,  1562,  1563,  1564,
    1565,  1566,  1567,  1568,  1569,  1571,  1574,  1574,  1580,  1582,
    1585,  1589,  1589,  1603,  1603,  1612,  1631,  1630,  1653,  1654,
    1657,  1659,  1663,  1664,  1667,  1667,  1671,  1672,  1676,  1684,
    1692,  1703,  1715,  1717,  1720,  1723,  1723,  1729,  1731,  1735,
    1745,  1755,  1769,  1771,  1774,  1776,  1778,  1786,  1795,  1795,
    1803,  1806,  1810,  1815,  1816,  1819,  1826,  1834,  1837,  1840,
    1849,  1858,  1860,  1863,  1870,  1872,  1875,  1878,  1881,  1891,
    1900,  1899,  1922,  1923,  1932,  1934,  1938,  1939,  1940,  1941,
    1944,  1944,  1948,  1949,  1953,  1961,  1969,  1981,  1980,  2005,
    2007,  2011,  2016,  2021,  2024,  2037,  2049,  2062,  2065,  2065,
    2071,  2071,  2079,  2081,  2085,  2087,  2089,  2093,  2094,  2095,
    2097,  2100,  2102,  2106,  2108,  2110,  2115,  2117,  2121,  2120,
    2125,  2125,  2139,  2139,  2148,  2150,  2154,  2155,  2156,  2157,
    2158,  2159,  2161,  2163,  2165,  2166,  2167,  2169,  2171,  2173,
    2175,  2177,  2179,  2181,  2183,  2183,  2187,  2188,  2191,  2200,
    2202,  2206,  2208,  2210,  2214,  2224,  2232,  2241,  2249,  2250,
    2251,  2252,  2253,  2254,  2255,  2256,  2257,  2260,  2261,  2268,
    2271,  2276,  2277,  2278,  2279,  2280,  2281,  2284,  2285,  2286,
    2287,  2298,  2311,  2312,  2313,  2314,  2315,  2316,  2318,  2321,
    2325,  2327,  2329,  2332,  2340,  2379,  2382,  2385,  2388,  2392,
    2392,  2395,  2395,  2399,  2401,  2409,  2413,  2416,  2436,  2441,
    2441,  2446,  2446,  2456,  2457,  2461,  2463,  2465,  2467,  2469,
    2471,  2473,  2473,  2475,  2482,  2484,  2486,  2488,  2495,  2502,
    2509,  2516,  2523,  2530,  2537,  2544,  2546,  2546,  2548,  2548,
    2556,  2556,  2564,  2564,  2572,  2579,  2581,  2583,  2590,  2597,
    2605,  2613,  2621,  2629,  2636,  2638,  2638,  2640,  2649,  2661,
    2674,  2687,  2700,  2702,  2704,  2718,  2732,  2746,  2760,  2774,
    2788,  2802,  2817,  2816,  2832,  2837,  2842,  2847,  2854,  2855,
    2859,  2869,  2877,  2887,  2888,  2889,  2890,  2891,  2893,  2902,
    2903,  2908,  2909,  2910,  2911,  2912,  2913,  2916,  2917,  2918,
    2919,  2922,  2923,  2924,  2925,  2927,  2930,  2931,  2930,  2935,
    2938,  2940,  2943,  2946,  2950,  2953,  2956,  2959,  2960,  2962,
    2963,  2966,  2970,  2974,  2978,  2980,  2984,  2984,  2987,  2987,
    2993,  2997,  3011,  3019,  3010,  3022,  3030,  3021,  3032,  3041,
    3051,  3053,  3056,  3059,  3068,  3078,  3088,  3097,  3108,  3108,
    3111,  3114,  3117,  3129,  3131,  3136,  3135,  3146,  3146,  3149,
    3148,  3152,  3161,  3163,  3165,  3167,  3169,  3171,  3173,  3175,
    3177,  3179,  3181,  3183,  3185,  3187,  3189,  3191,  3193,  3195,
    3199,  3201,  3203,  3207,  3209,  3211,  3213,  3217,  3219,  3221,
    3225,  3227,  3229,  3233,  3235,  3239,  3241,  3244,  3248,  3250,
    3255,  3256,  3258,  3263,  3265,  3269,  3271,  3273,  3277,  3279,
    3283,  3285,  3289,  3288,  3297,  3297,  3306,  3306,  3318,  3319,
    3323,  3323,  3330,  3330,  3337,  3337,  3344,  3344,  3350,  3353,
    3353,  3359,  3365,  3368,  3373,  3374,  3378,  3378,  3385,  3385,
    3395,  3396,  3399,  3403,  3403,  3407,  3407,  3411,  3411,  3413,
    3413,  3415,  3415,  3419,  3420,  3421,  3424,  3425,  3429,  3430,
    3434,  3435,  3436,  3437,  3438,  3439,  3440,  3442,  3445,  3446,
    3447,  3448,  3449,  3450,  3451,  3452,  3453,  3454,  3455,  3456,
    3457,  3459,  3460,  3463,  3469,  3471,  3480,  3484,  3485,  3486,
    3487,  3488,  3489,  3490,  3491,  3492,  3496,  3495,  3505,  3506,
    3510,  3510,  3517,  3517,  3524,  3524,  3532,  3532,  3539,  3539,
    3546,  3546,  3553,  3553,  3560,  3560,  3569,  3569,  3574,  3574,
    3580,  3585,  3590,  3599,  3602,  3601,  3607,  3613,  3623,  3624,
    3629,  3636,  3643,  3651,  3652,  3657,  3658,  3661,  3663,  3663,
    3668,  3680,  3681,  3681,  3686,  3686,  3690,  3703,  3714,  3725,
    3738,  3737,  3743,  3753,  3754,  3758,  3765,  3769,  3768,  3774,
    3777,  3782,  3783,  3787,  3786,  3793,  3796,  3801,  3800,  3807,
    3817,  3818,  3822,  3829,  3833,  3832,  3839,  3842,  3847,  3848,
    3853,  3852,  3859,  3862,  3868,  3883,  3898,  3913,  3930,  3947,
    3977
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "K_HISTORY", "K_ABUT", "K_ABUTMENT",
  "K_ACTIVE", "K_ANALOG", "K_ARRAY", "K_AREA", "K_BLOCK", "K_BOTTOMLEFT",
  "K_BOTTOMRIGHT", "K_BY", "K_CAPACITANCE", "K_CAPMULTIPLIER", "K_CLASS",
  "K_CLOCK", "K_CLOCKTYPE", "K_COLUMNMAJOR", "K_DESIGNRULEWIDTH",
  "K_INFLUENCE", "K_COMPONENTS", "K_CORE", "K_CORNER", "K_COVER",
  "K_CPERSQDIST", "K_CURRENT", "K_CURRENTSOURCE", "K_CUT", "K_DEFAULT",
  "K_DATABASE", "K_DATA", "K_DIELECTRIC", "K_DIRECTION", "K_DO",
  "K_EDGECAPACITANCE", "K_EEQ", "K_END", "K_ENDCAP", "K_FALL", "K_FALLCS",
  "K_FALLT0", "K_FALLSATT1", "K_FALLRS", "K_FALLSATCUR", "K_FALLTHRESH",
  "K_FEEDTHRU", "K_FIXED", "K_FOREIGN", "K_FROMPIN", "K_GENERATE",
  "K_GENERATOR", "K_GROUND", "K_HEIGHT", "K_HORIZONTAL", "K_INOUT",
  "K_INPUT", "K_INPUTNOISEMARGIN", "K_COMPONENTPIN", "K_INTRINSIC",
  "K_INVERT", "K_IRDROP", "K_ITERATE", "K_IV_TABLES", "K_LAYER",
  "K_LEAKAGE", "K_LEQ", "K_LIBRARY", "K_MACRO", "K_MATCH", "K_MAXDELAY",
  "K_MAXLOAD", "K_METALOVERHANG", "K_MILLIAMPS", "K_MILLIWATTS",
  "K_MINFEATURE", "K_MUSTJOIN", "K_NAMESCASESENSITIVE", "K_NANOSECONDS",
  "K_NETS", "K_NEW", "K_NONDEFAULTRULE", "K_NONINVERT", "K_NONUNATE",
  "K_OBS", "K_OHMS", "K_OFFSET", "K_ORIENTATION", "K_ORIGIN", "K_OUTPUT",
  "K_OUTPUTNOISEMARGIN", "K_OUTPUTRESISTANCE", "K_OVERHANG", "K_OVERLAP",
  "K_OFF", "K_ON", "K_OVERLAPS", "K_PAD", "K_PATH", "K_PATTERN",
  "K_PICOFARADS", "K_PIN", "K_PITCH", "K_PLACED", "K_POLYGON", "K_PORT",
  "K_POST", "K_POWER", "K_PRE", "K_PULLDOWNRES", "K_RECT", "K_RESISTANCE",
  "K_RESISTIVE", "K_RING", "K_RISE", "K_RISECS", "K_RISERS",
  "K_RISESATCUR", "K_RISETHRESH", "K_RISESATT1", "K_RISET0",
  "K_RISEVOLTAGETHRESHOLD", "K_FALLVOLTAGETHRESHOLD", "K_ROUTING",
  "K_ROWMAJOR", "K_RPERSQ", "K_SAMENET", "K_SCANUSE", "K_SHAPE",
  "K_SHRINKAGE", "K_SIGNAL", "K_SITE", "K_SIZE", "K_SOURCE", "K_SPACER",
  "K_SPACING", "K_SPECIALNETS", "K_STACK", "K_START", "K_STEP", "K_STOP",
  "K_STRUCTURE", "K_SYMMETRY", "K_TABLE", "K_THICKNESS", "K_TIEHIGH",
  "K_TIELOW", "K_TIEOFFR", "K_TIME", "K_TIMING", "K_TO", "K_TOPIN",
  "K_TOPLEFT", "K_TOPRIGHT", "K_TOPOFSTACKONLY", "K_TRISTATE", "K_TYPE",
  "K_UNATENESS", "K_UNITS", "K_USE", "K_VARIABLE", "K_VERTICAL", "K_VHI",
  "K_VIA", "K_VIARULE", "K_VLO", "K_VOLTAGE", "K_VOLTS", "K_WIDTH", "K_X",
  "K_Y", "K_R90", "T_STRING", "QSTRING", "NUMBER", "K_N", "K_S", "K_E",
  "K_W", "K_FN", "K_FS", "K_FE", "K_FW", "K_R0", "K_R180", "K_R270",
  "K_MX", "K_MY", "K_MXR90", "K_MYR90", "K_USER", "K_MASTERSLICE",
  "K_ENDMACRO", "K_ENDMACROPIN", "K_ENDVIARULE", "K_ENDVIA", "K_ENDLAYER",
  "K_ENDSITE", "K_CANPLACE", "K_CANNOTOCCUPY", "K_TRACKS", "K_FLOORPLAN",
  "K_GCELLGRID", "K_DEFAULTCAP", "K_MINPINS", "K_WIRECAP", "K_STABLE",
  "K_SETUP", "K_HOLD", "K_DEFINE", "K_DEFINES", "K_DEFINEB", "K_IF",
  "K_THEN", "K_ELSE", "K_FALSE", "K_TRUE", "K_EQ", "K_NE", "K_LE", "K_LT",
  "K_GE", "K_GT", "K_OR", "K_AND", "K_NOT", "K_DELAY", "K_TABLEDIMENSION",
  "K_TABLEAXIS", "K_TABLEENTRIES", "K_TRANSITIONTIME", "K_EXTENSION",
  "K_PROPDEF", "K_STRING", "K_INTEGER", "K_REAL", "K_RANGE", "K_PROPERTY",
  "K_VIRTUAL", "K_BUSBITCHARS", "K_VERSION", "K_BEGINEXT", "K_ENDEXT",
  "K_UNIVERSALNOISEMARGIN", "K_EDGERATETHRESHOLD1", "K_CORRECTIONTABLE",
  "K_EDGERATESCALEFACTOR", "K_EDGERATETHRESHOLD2", "K_VICTIMNOISE",
  "K_NOISETABLE", "K_EDGERATE", "K_VICTIMLENGTH", "K_CORRECTIONFACTOR",
  "K_OUTPUTPINANTENNASIZE", "K_INPUTPINANTENNASIZE",
  "K_INOUTPINANTENNASIZE", "K_CURRENTDEN", "K_PWL",
  "K_ANTENNALENGTHFACTOR", "K_TAPERRULE", "K_DIVIDERCHAR", "K_ANTENNASIZE",
  "K_ANTENNAMETALLENGTH", "K_ANTENNAMETALAREA", "K_RISESLEWLIMIT",
  "K_FALLSLEWLIMIT", "K_FUNCTION", "K_BUFFER", "K_INVERTER",
  "K_NAMEMAPSTRING", "K_NOWIREEXTENSIONATPIN", "K_WIREEXTENSION",
  "K_MESSAGE", "K_CREATEFILE", "K_OPENFILE", "K_CLOSEFILE", "K_WARNING",
  "K_ERROR", "K_FATALERROR", "K_RECOVERY", "K_SKEW", "K_ANYEDGE",
  "K_POSEDGE", "K_NEGEDGE", "K_SDFCONDSTART", "K_SDFCONDEND", "K_SDFCOND",
  "K_MPWH", "K_MPWL", "K_PERIOD", "K_ACCURRENTDENSITY",
  "K_DCCURRENTDENSITY", "K_AVERAGE", "K_PEAK", "K_RMS", "K_FREQUENCY",
  "K_CUTAREA", "K_MEGAHERTZ", "K_USELENGTHTHRESHOLD", "K_LENGTHTHRESHOLD",
  "K_ANTENNAINPUTGATEAREA", "K_ANTENNAINOUTDIFFAREA",
  "K_ANTENNAOUTPUTDIFFAREA", "K_ANTENNAAREARATIO",
  "K_ANTENNADIFFAREARATIO", "K_ANTENNACUMAREARATIO",
  "K_ANTENNACUMDIFFAREARATIO", "K_ANTENNAAREAFACTOR",
  "K_ANTENNASIDEAREARATIO", "K_ANTENNADIFFSIDEAREARATIO",
  "K_ANTENNACUMSIDEAREARATIO", "K_ANTENNACUMDIFFSIDEAREARATIO",
  "K_ANTENNASIDEAREAFACTOR", "K_DIFFUSEONLY", "K_MANUFACTURINGGRID",
  "K_ANTENNACELL", "K_CLEARANCEMEASURE", "K_EUCLIDEAN", "K_MAXXY",
  "K_USEMINSPACING", "K_ROWMINSPACING", "K_ROWABUTSPACING", "K_FLIP",
  "K_NONE", "K_ANTENNAPARTIALMETALAREA", "K_ANTENNAPARTIALMETALSIDEAREA",
  "K_ANTENNAGATEAREA", "K_ANTENNADIFFAREA", "K_ANTENNAMAXAREACAR",
  "K_ANTENNAMAXSIDEAREACAR", "K_ANTENNAPARTIALCUTAREA",
  "K_ANTENNAMAXCUTCAR", "K_SLOTWIREWIDTH", "K_SLOTWIRELENGTH",
  "K_SLOTWIDTH", "K_SLOTLENGTH", "K_MAXADJACENTSLOTSPACING",
  "K_MAXCOAXIALSLOTSPACING", "K_MAXEDGESLOTSPACING", "K_SPLITWIREWIDTH",
  "K_MINIMUMDENSITY", "K_MAXIMUMDENSITY", "K_DENSITYCHECKWINDOW",
  "K_DENSITYCHECKSTEP", "K_FILLACTIVESPACING", "K_MINIMUMCUT",
  "K_ADJACENTCUTS", "K_ANTENNAMODEL", "K_BUMP", "K_ENCLOSURE",
  "K_FROMABOVE", "K_FROMBELOW", "K_IMPLANT", "K_LENGTH", "K_MAXVIASTACK",
  "K_AREAIO", "K_BLACKBOX", "K_MAXWIDTH", "K_MINENCLOSEDAREA", "K_MINSTEP",
  "K_ORIENT", "K_OXIDE1", "K_OXIDE2", "K_OXIDE3", "K_OXIDE4",
  "K_PARALLELRUNLENGTH", "K_MINWIDTH", "K_PROTRUSIONWIDTH",
  "K_SPACINGTABLE", "K_WITHIN", "IF", "LNOT", "'-'", "'+'", "'*'", "'/'",
  "UMINUS", "';'", "'('", "')'", "'='", "'\\n'", "'<'", "'>'", "$accept",
  "lef_file", "version", "$@1", "dividerchar", "busbitchars", "rules",
  "end_library", "rule", "case_sensitivity", "wireextension",
  "manufacturing", "useminspacing", "clearancemeasure", "clearance_type",
  "spacing_type", "spacing_value", "units_section", "start_units",
  "units_rules", "units_rule", "$@2", "layer_rule", "start_layer", "$@3",
  "end_layer", "$@4", "layer_options", "layer_option", "$@5", "$@6", "$@7",
  "$@8", "$@9", "$@10", "$@11", "$@12", "$@13", "$@14", "$@15", "$@16",
  "$@17", "$@18", "$@19", "sp_options", "$@20", "$@21", "$@22", "$@23",
  "$@24", "layer_antenna_pwl", "$@25", "layer_diffusion_ratios",
  "layer_diffusion_ratio", "layer_antenna_duo", "layer_table_type",
  "ac_layer_table_opt_list", "ac_layer_table_opt", "$@26", "$@27", "$@28",
  "$@29", "dc_layer_table_opt_list", "dc_layer_table_opt", "$@30", "$@31",
  "$@32", "number_list", "layer_prop_list", "layer_prop",
  "current_density_pwl_list", "current_density_pwl", "cap_points",
  "cap_point", "res_points", "res_point", "layer_type", "layer_direction",
  "layer_connect_type", "layer_cuts_required", "layer_minen_width",
  "layer_oxide", "layer_sp_parallel_widths", "layer_sp_parallel_width",
  "$@33", "layer_sp_influence_widths", "layer_sp_influence_width",
  "maxstack_via", "$@34", "via", "via_keyword", "start_via", "via_options",
  "via_option", "$@35", "via_prop_list", "via_name_value_pair",
  "via_foreign", "start_foreign", "$@36", "orientation", "via_layer_rule",
  "via_layer", "$@37", "via_rects", "via_rect", "end_via", "$@38",
  "viarule_keyword", "$@39", "viarule", "viarule_generate", "$@40",
  "viarule_layer_list", "opt_viarule_props", "viarule_props",
  "viarule_prop", "$@41", "viarule_prop_list", "viarule_layer",
  "via_names", "via_name", "viarule_layer_name", "$@42",
  "viarule_layer_options", "viarule_layer_option", "end_viarule", "$@43",
  "spacing_rule", "start_spacing", "end_spacing", "spacings", "spacing",
  "samenet_keyword", "irdrop", "start_irdrop", "end_irdrop", "ir_tables",
  "ir_table", "ir_table_values", "ir_table_value", "ir_tablename",
  "minfeature", "dielectric", "nondefault_rule", "$@44", "end_nd_rule",
  "nd_rules", "nd_rule", "nd_prop", "$@45", "nd_prop_list", "nd_layer",
  "$@46", "nd_layer_stmts", "nd_layer_stmt", "site", "start_site", "$@47",
  "end_site", "$@48", "site_options", "site_option", "site_class",
  "site_symmetry_statement", "site_symmetries", "site_symmetry", "pt",
  "macro", "$@49", "start_macro", "$@50", "end_macro", "$@51",
  "macro_options", "macro_option", "$@52", "macro_prop_list",
  "macro_symmetry_statement", "macro_symmetries", "macro_symmetry",
  "macro_name_value_pair", "macro_class", "class_type", "pad_type",
  "core_type", "endcap_type", "macro_generator", "macro_generate",
  "macro_source", "macro_power", "macro_origin", "macro_foreign",
  "macro_eeq", "$@53", "macro_leq", "$@54", "macro_site",
  "macro_site_word", "site_word", "macro_size", "macro_pin",
  "start_macro_pin", "$@55", "end_macro_pin", "$@56", "macro_pin_options",
  "macro_pin_option", "$@57", "$@58", "$@59", "$@60", "$@61", "$@62",
  "$@63", "pin_layer_oxide", "pin_prop_list", "pin_name_value_pair",
  "electrical_direction", "start_macro_port", "macro_port_class_option",
  "macro_pin_use", "macro_scan_use", "pin_shape", "geometries", "geometry",
  "$@64", "$@65", "geometry_options", "layer_spacing", "firstPt", "nextPt",
  "otherPts", "via_placement", "$@66", "$@67", "stepPattern",
  "sitePattern", "trackPattern", "$@68", "$@69", "$@70", "$@71",
  "trackLayers", "layer_name", "gcellPattern", "macro_obs",
  "start_macro_obs", "macro_clocktype", "$@72", "timing", "start_timing",
  "end_timing", "timing_options", "timing_option", "$@73", "$@74", "$@75",
  "one_pin_trigger", "two_pin_trigger", "from_pin_trigger",
  "to_pin_trigger", "delay_or_transition", "list_of_table_entries",
  "table_entry", "list_of_table_axis_numbers", "slew_spec", "risefall",
  "unateness", "list_of_from_strings", "list_of_to_strings", "array",
  "$@76", "start_array", "$@77", "end_array", "$@78", "array_rules",
  "array_rule", "$@79", "$@80", "$@81", "$@82", "$@83", "floorplan_start",
  "floorplan_list", "floorplan_element", "$@84", "$@85", "cap_list",
  "one_cap", "msg_statement", "$@86", "create_file_statement", "$@87",
  "def_statement", "$@88", "$@89", "$@90", "dtrm", "then", "else",
  "expression", "b_expr", "s_expr", "relop", "prop_def_section", "$@91",
  "prop_stmts", "prop_stmt", "$@92", "$@93", "$@94", "$@95", "$@96",
  "$@97", "$@98", "$@99", "prop_define", "$@100", "$@101",
  "opt_range_detail", "$@102", "opt_range_second", "opt_def_range",
  "opt_def_value", "opt_spacing_cut", "$@103", "opt_layer_name", "$@104",
  "req_layer_name", "$@105", "universalnoisemargin", "edgeratethreshold1",
  "edgeratethreshold2", "edgeratescalefactor", "noisetable", "$@106",
  "end_noisetable", "noise_table_list", "noise_table_entry",
  "output_resistance_entry", "$@107", "num_list", "victim_list", "victim",
  "$@108", "vnoiselist", "correctiontable", "$@109", "end_correctiontable",
  "correction_table_list", "correction_table_item", "output_list", "$@110",
  "numo_list", "corr_victim_list", "corr_victim", "$@111", "corr_list",
  "input_antenna", "output_antenna", "inout_antenna", "antenna_input",
  "antenna_inout", "antenna_output", "extension", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   509,   510,   511,   512,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   530,   531,   532,   533,   534,
     535,   536,   537,   538,   539,   540,   541,   542,   543,   544,
     545,   546,   547,   548,   549,   550,   551,   552,   553,   554,
     555,   556,   557,   558,   559,   560,   561,   562,   563,   564,
     565,   566,   567,   568,   569,   570,   571,   572,   573,   574,
     575,   576,   577,   578,   579,   580,   581,   582,   583,   584,
     585,   586,   587,   588,   589,   590,   591,   592,   593,   594,
     595,   596,   597,   598,   599,   600,   601,   602,   603,   604,
     605,   606,   607,   608,   609,   610,   611,   612,   613,   614,
     615,   616,   617,   618,   619,   620,   621,   622,   623,   624,
     625,   626,   627,    45,    43,    42,    47,   628,    59,    40,
      41,    61,    10,    60,    62
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   385,   386,   388,   387,   389,   390,   391,   391,   391,
     392,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     393,   393,   393,   393,   393,   393,   393,   393,   393,   393,
     394,   394,   395,   395,   396,   397,   398,   399,   399,   400,
     400,   401,   401,   402,   403,   404,   404,   405,   405,   405,
     405,   405,   405,   406,   405,   405,   407,   409,   408,   411,
     410,   412,   412,   413,   413,   413,   413,   413,   414,   413,
     413,   413,   413,   413,   413,   413,   413,   413,   413,   413,
     413,   413,   413,   413,   413,   415,   413,   416,   413,   413,
     417,   413,   413,   413,   418,   413,   413,   419,   413,   420,
     413,   413,   421,   413,   413,   422,   413,   423,   413,   424,
     413,   413,   413,   413,   413,   413,   413,   413,   413,   413,
     413,   413,   413,   413,   413,   413,   425,   413,   426,   427,
     413,   413,   413,   428,   413,   430,   431,   432,   433,   429,
     434,   429,   435,   436,   435,   437,   437,   438,   439,   439,
     440,   440,   440,   441,   441,   443,   442,   444,   442,   445,
     442,   446,   442,   447,   447,   449,   448,   450,   448,   451,
     448,   452,   452,   453,   453,   454,   454,   454,   455,   455,
     456,   457,   457,   458,   459,   459,   460,   461,   461,   461,
     461,   461,   461,   462,   462,   463,   463,   463,   464,   464,
     465,   465,   466,   466,   466,   466,   467,   467,   469,   468,
     470,   470,   471,   472,   473,   472,   474,   475,   476,   476,
     477,   477,   478,   478,   478,   479,   478,   478,   480,   480,
     481,   481,   481,   482,   482,   482,   482,   484,   483,   485,
     485,   485,   485,   485,   485,   485,   485,   485,   485,   485,
     485,   485,   485,   485,   485,   486,   488,   487,   489,   489,
     490,   492,   491,   494,   493,   495,   497,   496,   498,   498,
     499,   499,   500,   500,   502,   501,   503,   503,   501,   501,
     501,   504,   505,   505,   506,   508,   507,   509,   509,   510,
     510,   510,   510,   510,   510,   510,   510,   510,   512,   511,
     513,   514,   515,   516,   516,   517,   517,   518,   519,   520,
     521,   522,   522,   523,   524,   524,   525,   526,   527,   528,
     530,   529,   531,   531,   532,   532,   533,   533,   533,   533,
     535,   534,   536,   536,   534,   534,   534,   538,   537,   539,
     539,   540,   540,   540,   540,   540,   540,   541,   543,   542,
     545,   544,   546,   546,   547,   547,   547,   548,   548,   548,
     549,   550,   550,   551,   551,   551,   552,   552,   554,   553,
     556,   555,   558,   557,   559,   559,   560,   560,   560,   560,
     560,   560,   560,   560,   560,   560,   560,   560,   560,   560,
     560,   560,   560,   560,   561,   560,   562,   562,   563,   564,
     564,   565,   565,   565,   566,   566,   566,   567,   568,   568,
     568,   568,   568,   568,   568,   568,   568,   568,   568,   568,
     568,   569,   569,   569,   569,   569,   569,   570,   570,   570,
     570,   570,   571,   571,   571,   571,   571,   571,   572,   573,
     574,   574,   574,   575,   576,   577,   577,   577,   577,   579,
     578,   581,   580,   582,   582,   583,   584,   585,   586,   588,
     587,   590,   589,   591,   591,   592,   592,   592,   592,   592,
     592,   593,   592,   592,   592,   592,   592,   592,   592,   592,
     592,   592,   592,   592,   592,   592,   594,   592,   595,   592,
     596,   592,   597,   592,   592,   592,   592,   592,   592,   592,
     592,   592,   592,   592,   592,   598,   592,   592,   592,   592,
     592,   592,   592,   592,   592,   592,   592,   592,   592,   592,
     592,   592,   599,   592,   600,   600,   600,   600,   601,   601,
     602,   602,   602,   603,   603,   603,   603,   603,   604,   605,
     605,   606,   606,   606,   606,   606,   606,   607,   607,   607,
     607,   608,   608,   608,   608,   609,   611,   612,   610,   610,
     610,   610,   610,   610,   610,   610,   610,   613,   613,   614,
     614,   614,   615,   616,   617,   617,   619,   618,   620,   618,
     621,   622,   624,   625,   623,   626,   627,   623,   623,   623,
     628,   628,   629,   630,   630,   631,   631,   632,   634,   633,
     635,   636,   637,   638,   638,   640,   639,   641,   639,   642,
     639,   639,   639,   639,   639,   639,   639,   639,   639,   639,
     639,   639,   639,   639,   639,   639,   639,   639,   639,   639,
     643,   643,   643,   644,   644,   644,   644,   645,   645,   645,
     646,   646,   646,   647,   647,   648,   648,   649,   650,   650,
     651,   651,   651,   652,   652,   653,   653,   653,   654,   654,
     655,   655,   657,   656,   659,   658,   661,   660,   662,   662,
     664,   663,   665,   663,   666,   663,   667,   663,   663,   668,
     663,   663,   663,   669,   670,   670,   672,   671,   673,   671,
     674,   674,   675,   677,   676,   679,   678,   681,   680,   682,
     680,   683,   680,   684,   684,   684,   685,   685,   686,   686,
     687,   687,   687,   687,   687,   687,   687,   687,   688,   688,
     688,   688,   688,   688,   688,   688,   688,   688,   688,   688,
     688,   688,   688,   689,   689,   689,   689,   690,   690,   690,
     690,   690,   690,   690,   690,   690,   692,   691,   693,   693,
     695,   694,   696,   694,   697,   694,   698,   694,   699,   694,
     700,   694,   701,   694,   702,   694,   704,   703,   705,   703,
     703,   703,   703,   706,   707,   706,   706,   706,   708,   708,
     708,   708,   708,   709,   709,   710,   710,   711,   712,   711,
     711,   713,   714,   713,   716,   715,   717,   718,   719,   720,
     722,   721,   723,   724,   724,   725,   725,   727,   726,   728,
     728,   729,   729,   731,   730,   732,   732,   734,   733,   735,
     736,   736,   737,   737,   739,   738,   740,   740,   741,   741,
     743,   742,   744,   744,   745,   746,   747,   748,   749,   750,
     751
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     0,     4,     3,     3,     0,     2,     1,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     3,     4,     3,     1,     1,     1,
       1,     1,     1,     4,     1,     0,     2,     4,     4,     4,
       4,     4,     4,     0,     5,     4,     3,     0,     3,     0,
       3,     0,     2,     3,     3,     3,     3,     3,     0,     6,
       3,     4,     7,     4,     7,     3,     3,     3,     3,     3,
       3,     3,     3,     6,     6,     0,     4,     0,     5,     4,
       0,     5,     4,     3,     0,     4,     3,     0,     4,     0,
       5,     3,     0,     4,     3,     0,     4,     0,     5,     0,
       4,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     4,     3,     3,     3,     3,     0,     5,     0,     0,
       9,     3,     7,     0,     4,     0,     0,     0,     0,    11,
       0,     9,     1,     0,     7,     0,     2,     1,     0,     1,
       1,     1,     1,     0,     2,     0,     5,     0,     5,     0,
       4,     0,     5,     0,     2,     0,     5,     0,     4,     0,
       5,     0,     2,     1,     2,     2,     2,     2,     1,     2,
       4,     1,     2,     4,     1,     2,     4,     1,     1,     1,
       1,     1,     1,     1,     1,     0,     1,     1,     0,     4,
       0,     2,     1,     1,     1,     1,     0,     2,     0,     4,
       0,     2,     6,     3,     0,     7,     3,     1,     2,     3,
       0,     2,     1,     1,     3,     0,     4,     1,     1,     2,
       2,     2,     2,     2,     3,     4,     3,     0,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     0,     4,     0,     2,
       4,     0,     3,     0,     3,     5,     0,     6,     1,     2,
       0,     1,     1,     2,     0,     4,     1,     2,     2,     2,
       2,     2,     0,     2,     3,     0,     4,     0,     2,     3,
       3,     4,     5,     4,     5,     3,     3,     3,     0,     3,
       3,     1,     2,     1,     2,     5,     6,     1,     3,     1,
       2,     0,     2,     3,     0,     2,     2,     2,     4,     3,
       0,     5,     1,     2,     0,     2,     1,     1,     1,     1,
       0,     4,     1,     2,     2,     2,     2,     0,     6,     0,
       2,     3,     3,     3,     4,     4,     3,     3,     0,     3,
       0,     3,     0,     2,     5,     1,     1,     3,     3,     3,
       3,     0,     2,     1,     1,     1,     2,     4,     0,     4,
       0,     3,     0,     3,     0,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       3,     1,     1,     1,     0,     4,     1,     2,     3,     0,
       2,     1,     1,     1,     2,     2,     2,     3,     1,     2,
       1,     1,     2,     1,     1,     1,     2,     1,     1,     2,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     3,     4,
       3,     3,     3,     3,     3,     2,     3,     4,     3,     0,
       4,     0,     4,     3,     3,     1,     1,     5,     3,     0,
       3,     0,     3,     0,     2,     2,     3,     4,     3,     4,
       5,     0,     4,     3,     1,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     0,     4,     0,     5,
       0,     5,     0,     5,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     4,     3,     0,     4,     4,     2,     4,
       4,     4,     3,     3,     4,     4,     4,     4,     4,     4,
       4,     4,     0,     4,     1,     1,     1,     1,     1,     2,
       2,     2,     2,     3,     3,     4,     3,     3,     1,     0,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     1,     1,     1,     2,     0,     0,     6,     3,
       4,     6,     4,     6,     7,     9,     1,     0,     2,     0,
       2,     2,     1,     1,     0,     2,     0,     5,     0,     7,
       7,    11,     0,     0,    10,     0,     0,    10,     6,     6,
       0,     2,     1,     6,     6,     3,     2,     1,     0,     4,
       3,     1,     2,     0,     2,     0,     4,     0,     4,     0,
      10,     9,     3,     3,     4,     4,     4,     4,     4,     4,
       4,     4,     3,     7,     8,     6,     3,     3,     3,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     5,     1,     2,
       0,     4,     7,     1,     1,     1,     1,     1,     1,     2,
       1,     2,     0,     4,     0,     3,     0,     3,     0,     2,
       0,     4,     0,     4,     0,     4,     0,     4,     4,     0,
       4,     5,     1,     2,     0,     2,     0,     4,     0,     4,
       0,     2,     5,     0,     6,     0,     6,     0,     6,     0,
       6,     0,     6,     0,     1,     1,     1,     2,     1,     2,
       3,     3,     3,     3,     2,     3,     6,     1,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     3,
       6,     1,     1,     3,     3,     6,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     5,     0,     2,
       0,     5,     0,     5,     0,     5,     0,     5,     0,     5,
       0,     5,     0,     5,     0,     5,     0,     4,     0,     4,
       1,     2,     2,     0,     0,     5,     2,     5,     0,     1,
       2,     5,     3,     0,     3,     0,     1,     0,     0,     3,
       4,     0,     0,     3,     0,     3,     4,     3,     3,     3,
       0,     7,     2,     1,     2,     3,     1,     0,     5,     1,
       2,     1,     2,     0,     7,     1,     2,     0,     7,     2,
       1,     2,     3,     1,     0,     5,     1,     2,     1,     2,
       0,     7,     1,     2,     3,     3,     3,     3,     3,     3,
       1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     9,     0,     0,     1,   684,     0,     0,   329,    77,
     390,     0,     0,     0,   368,   321,    64,   237,   283,   717,
     719,   721,   766,     0,     3,   860,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   713,   715,     0,
       0,     0,     0,     0,     0,     0,    11,    19,    12,     2,
       8,    13,    20,    45,    46,    47,    14,    65,    15,    81,
      48,    16,     0,   240,     0,    17,    18,    22,     0,    25,
     331,    24,    23,    30,    26,   372,    27,   394,    28,   688,
      21,    49,    29,    31,    32,    33,    35,    34,    36,    37,
      38,    39,    40,    42,    43,    44,    41,     0,     0,    10,
       0,     0,     0,     0,     0,   340,     0,     0,     0,     0,
       0,   768,     0,     0,     0,     0,     0,     0,     0,   820,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    58,    57,     0,    59,    60,     0,     0,     0,
       0,   238,     0,   286,   305,   302,   288,   307,   327,     0,
     323,     0,     0,     0,   388,   682,   685,   339,    78,   391,
       0,    51,    50,   344,   369,   284,     0,     0,     0,     0,
       6,     0,     0,   817,   837,   819,   818,     0,   855,   854,
     856,     5,    53,    52,     0,     0,   857,   858,   859,    54,
      56,    62,    61,     0,   234,   233,     0,     0,    73,     0,
       0,     0,     0,     0,     0,    66,     0,     0,     0,     0,
       0,    79,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   105,     0,     0,     0,     0,     0,     0,   114,     0,
     117,     0,     0,   122,     0,   125,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   129,     0,     0,     0,     0,     0,   153,    76,    82,
     239,   281,   257,   276,     0,   247,   245,   241,   242,     0,
     243,   278,   236,     0,     0,   289,   290,   301,     0,   320,
     324,     0,     0,     0,   328,   332,   334,     0,   370,     0,
     381,   367,   373,   376,   375,     0,   618,   469,     0,     0,
     471,   617,     0,   479,     0,   475,     0,     0,   419,   621,
     414,     0,     0,     0,   395,   400,   396,   397,   398,   399,
     402,   401,   403,   404,   405,   407,     0,   406,   408,   483,
     411,     0,   412,   413,   623,   476,   692,   694,   696,     0,
     699,     0,   690,     0,   689,   704,   702,   338,     0,     0,
       0,     0,     0,   772,   782,   770,   776,   784,   774,   778,
     780,   769,     4,   816,     0,     0,     0,     0,    55,     0,
       0,     0,     0,    63,     0,     0,     0,     0,     0,     0,
       0,     0,   213,   214,     0,     0,     0,     0,     0,     0,
       0,     0,    88,     0,   208,   209,   207,   210,   211,   212,
       0,     0,     0,     0,     0,     0,     0,     0,   171,   170,
     172,   107,   110,     0,     0,     0,     0,   119,     0,     0,
       0,     0,   127,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   146,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   268,
       0,   259,   261,   262,   260,   263,   265,   266,   264,   267,
     269,   270,   273,   271,   274,   272,   253,     0,     0,     0,
     275,   290,     0,     0,   294,     0,     0,   291,   292,   303,
       0,     0,     0,     0,     0,     0,     0,     0,   308,   322,
       0,   330,   337,     0,     0,     0,     0,     0,     0,     0,
     431,   437,   438,   428,     0,   434,   430,   435,   433,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   465,     0,     0,   392,
     389,     0,     0,     0,   616,   576,     0,     0,     0,     0,
       0,     0,   587,   586,     0,     0,     0,     0,   703,     0,
     710,     0,   686,   683,     0,   342,     0,     0,   350,   347,
     348,   341,   345,   349,   346,   737,     0,     0,     0,   723,
     756,     0,     0,   723,     0,   752,   751,     0,     0,     0,
     723,     0,   767,     0,     0,     0,     0,     0,     0,     0,
       0,   844,     0,     0,   840,   843,   827,     0,     0,   823,
     826,   723,   723,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    87,     0,     0,    99,    90,   100,    80,    95,
      85,    84,     0,     0,    98,   793,    97,    83,    86,     0,
       0,   193,   102,     0,     0,   101,    96,     0,   173,     0,
     183,   113,   162,     0,     0,   116,     0,   168,   121,     0,
     124,     0,   168,   131,   132,   133,   134,   135,   136,   137,
     138,   139,   140,     0,   142,   143,     0,   222,   223,   224,
     225,     0,   144,   220,   151,   145,     0,     0,     0,     0,
     282,   258,     0,   244,     0,     0,   248,   386,     0,   256,
     254,     0,     0,   279,     0,   306,   298,   299,   300,     0,
       0,   318,   285,   293,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   333,   335,   378,   377,   379,
     371,     0,   383,   384,   385,   380,   382,   432,   447,   450,
     448,   449,   451,   439,   429,   456,   457,   453,   452,   454,
     455,   440,   443,   441,   442,   444,   445,   446,   436,   427,
       0,     0,     0,   458,     0,   464,   480,   463,     0,   462,
     461,   460,   421,   422,   423,   418,   420,     0,     0,   416,
     409,   410,   468,   466,     0,     0,     0,   473,   474,     0,
       0,     0,   481,     0,     0,   512,     0,     0,   491,     0,
       0,   506,   508,   510,   558,     0,     0,     0,     0,     0,
       0,     0,     0,   571,     0,     0,     0,     0,   525,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   542,     0,   478,   484,   494,   559,     0,
       0,   592,   594,     0,     0,     0,     0,     0,   596,     0,
     615,   575,     0,   674,     0,     0,     0,     0,   625,   673,
       0,     0,     0,     0,   627,     0,     0,   653,   654,     0,
       0,     0,   655,   656,     0,     0,     0,   650,   651,   652,
     620,   624,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   706,   708,
     705,   343,   357,   354,   355,   356,     0,     0,   734,     0,
       0,     0,     0,     0,   724,   725,   718,     0,     0,     0,
     720,     0,   748,     0,     0,     0,   761,   762,   757,   758,
     759,   760,     0,     0,   763,   764,   765,     0,     0,     0,
       0,     0,   722,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   723,   841,     0,
       0,     0,   723,   824,   714,   716,     0,    68,    71,     0,
      70,    69,    67,    72,    75,    93,     0,    91,     0,     0,
       0,   807,   195,   196,   197,   106,   194,     0,     0,   198,
       0,   109,     0,   112,     0,     0,   115,   118,   169,     0,
     123,   126,     0,   141,   148,   130,     0,     0,     0,     0,
     155,   154,   277,   252,   251,   250,   246,   249,     0,   255,
       0,   287,   296,     0,   304,     0,   309,   310,   317,   316,
       0,   315,     0,     0,     0,     0,   325,   336,     0,   619,
     470,   459,   472,     0,   426,   425,   424,   415,   417,   467,
     393,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   567,   568,
     569,   570,     0,   572,   574,   573,     0,     0,   562,   565,
     566,   564,   563,   561,     0,     0,     0,     0,     0,   811,
     811,   811,     0,     0,   811,   811,   811,   811,     0,     0,
     811,     0,     0,     0,   485,     0,     0,   528,     0,   577,
     594,   593,   594,     0,     0,     0,     0,     0,   598,     0,
     579,   588,   622,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   675,   676,   677,     0,     0,   668,     0,
       0,     0,   665,   649,     0,     0,     0,     0,   657,   658,
     659,     0,     0,   663,   664,     0,   693,   695,     0,     0,
     697,     0,     0,   700,     0,     0,   711,   691,   687,   698,
       0,     0,   359,   352,     0,   726,     0,     0,   735,   731,
     730,   732,   733,     0,   754,   753,     0,   749,   740,   739,
     738,   744,   745,   747,   746,   743,   742,   741,   790,   786,
     788,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     846,     0,   842,   839,   838,   829,     0,   825,   822,   821,
     235,    74,     0,     0,   201,     0,     0,   204,     0,   796,
     808,     0,     0,     0,     0,   199,     0,     0,     0,     0,
       0,   108,   174,     0,     0,     0,   111,   184,     0,   120,
     128,   215,   221,   147,     0,     0,   191,   387,     0,   295,
     297,   319,   313,     0,     0,   311,   326,   374,   477,     0,
     514,   519,   520,   557,   556,   553,     0,   554,   482,   501,
     499,     0,     0,   497,     0,   515,   516,     0,     0,     0,
     493,   518,   517,   500,   498,   521,   522,   496,   505,   504,
     495,   503,   502,     0,     0,   548,   524,   812,     0,     0,
       0,   532,   533,     0,     0,     0,     0,   814,     0,     0,
       0,     0,   544,   545,   546,   547,     0,   488,     0,   486,
       0,     0,     0,   589,     0,   595,   580,     0,     0,     0,
     582,     0,     0,     0,     0,     0,     0,   678,     0,     0,
       0,     0,     0,   680,     0,   642,     0,   669,   632,     0,
     633,   666,   646,   647,   648,     0,   660,   661,   662,     0,
       0,     0,     0,     0,     0,     0,   701,     0,     0,     0,
       0,   351,   353,   727,     0,     0,     0,     0,     0,   791,
     803,   803,   792,   773,   783,   771,   777,   785,   775,   779,
     781,   847,     0,   830,     0,     0,     0,   202,     0,     0,
     205,   794,     0,     0,     0,    89,     0,   103,   104,   181,
     179,   175,   177,   189,   187,   185,   163,   216,   217,   149,
       0,     0,   156,   280,   314,   312,     0,   555,     0,   523,
     492,   507,     0,     0,   552,   551,   550,   526,   549,     0,
     529,   531,   530,   534,   535,   538,   537,     0,   539,   540,
     536,   541,   543,   489,     0,   487,   560,   527,     0,     0,
       0,     0,     0,     0,   594,     0,     0,   597,   637,   641,
     639,   635,   679,   626,   636,   634,   638,   640,   681,   628,
       0,     0,     0,     0,   629,     0,     0,     0,     0,     0,
       0,   707,   709,     0,     0,     0,     0,     0,     0,     0,
     360,   728,     0,     0,     0,     0,     0,   805,   805,     0,
     845,   848,     0,   828,   831,     0,    94,     0,    92,   798,
       0,   809,     0,   200,   191,   191,   191,   191,   191,   191,
     191,   165,   218,   152,     0,   192,     0,     0,   513,   509,
     511,   813,   815,   490,   591,   590,   578,     0,   581,   594,
       0,   583,     0,     0,     0,     0,     0,   670,     0,     0,
       0,     0,     0,     0,     0,     0,   358,     0,     0,     0,
       0,   729,   736,   755,   750,     0,   806,   787,   789,     0,
     849,     0,   832,   203,   206,     0,     0,   799,   795,   797,
     810,     0,   180,     0,     0,     0,   188,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   584,   599,     0,   667,
     645,     0,     0,     0,     0,   602,   605,   613,   614,   712,
       0,   366,     0,   362,   361,   363,   804,   850,   833,   800,
       0,   182,   176,   178,   190,   186,   164,   166,   167,     0,
     150,   160,   157,     0,     0,     0,   643,     0,     0,     0,
       0,     0,     0,   365,   364,     0,     0,     0,   802,     0,
     230,   191,     0,     0,   585,   644,     0,     0,     0,   603,
     606,     0,     0,     0,   219,   161,   158,     0,     0,   671,
       0,   631,   610,   610,   852,     0,   835,     0,   801,     0,
     231,   226,   601,   600,     0,   630,   604,   607,   853,   851,
     836,   834,     0,   159,     0,   612,   611,     0,     0,   227,
     672,     0,   228,     0,   191,   232,   229
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,    46,   113,    47,    48,     3,    49,    50,    51,
      52,    53,    54,    55,   134,   137,   193,    56,    57,   139,
     205,   372,    58,    59,   100,   258,   386,   140,   259,   625,
     402,   638,   640,   414,   416,   647,   419,   421,   652,   437,
     673,  1251,  1542,   443,   679,  1256,  1546,  1671,  1701,  1670,
     644,  1541,  1608,  1647,   989,   411,   982,  1242,  1536,  1537,
    1535,  1534,   984,  1247,  1540,  1539,  1538,  1432,   630,   631,
     978,   979,  1223,  1224,  1226,  1227,   400,   384,  1429,  1610,
     997,   671,  1713,  1719,  1724,  1685,  1700,    60,   369,    61,
      62,    63,   142,   267,   448,   685,   686,   268,   269,   445,
     468,   270,   271,   446,   470,   693,   272,   444,    64,   107,
      65,    66,   273,   145,   476,   477,   478,   699,  1013,   146,
     276,   479,   147,   274,   277,   488,   702,  1015,    67,    68,
     279,   149,   150,   151,    69,    70,   284,   152,   285,   493,
     716,   286,    71,    72,    73,   163,   561,   348,   562,   563,
     896,  1174,   564,  1172,  1380,  1510,    74,    75,   106,   291,
     497,   153,   292,   293,   294,   499,   726,  1111,    76,   313,
      77,   101,   530,   775,   154,   314,   523,   768,   315,   522,
     766,   769,   316,   509,   748,   733,   741,   317,   318,   319,
     320,   321,   322,   323,   511,   324,   514,   325,   326,   342,
     327,   328,   329,   516,   825,  1049,   533,   826,  1055,  1058,
    1059,  1060,  1052,  1087,  1102,  1326,  1304,  1305,   827,   828,
    1108,  1084,  1072,  1076,   541,   542,   829,  1333,   841,  1470,
     832,  1112,  1113,   543,  1119,  1341,  1472,   532,   880,  1661,
    1692,  1662,  1693,  1706,  1716,   883,   330,   331,   332,   510,
     333,   334,   870,   544,   871,  1127,  1132,  1567,   872,   873,
    1151,  1369,  1155,  1141,  1142,  1139,  1623,   874,  1136,  1348,
    1354,    78,   343,    79,    97,   553,   886,   155,   344,   551,
     545,   546,   547,   549,   345,   554,   890,  1170,  1171,   884,
    1166,    80,   126,    81,   127,    82,   108,   109,   110,   906,
    1177,  1513,   579,   580,   581,   927,    83,   111,   169,   361,
     585,   583,   588,   586,   589,   590,   584,   587,  1202,  1390,
    1391,   971,  1529,  1598,  1517,  1587,  1232,  1413,  1308,  1449,
    1318,  1457,    84,    85,    86,    87,    88,   177,   952,   598,
     599,   600,   949,  1216,  1523,  1524,  1666,  1697,    89,   364,
     947,   593,   594,   595,   944,  1211,  1520,  1521,  1665,  1695,
      90,    91,    92,    93,    94,    95,    96
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1301
static const yytype_int16 yypact[] =
{
     515, -1301,    67,   655, -1301, -1301,   -67,    71, -1301, -1301,
   -1301,    23,   323,    37, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301,    21, -1301, -1301,    58,    89,   116,   125,
     158,   174,   194,   209,   214,   184,   350, -1301, -1301,   216,
     247,   263,   265,   256,   239,   267, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301,   188, -1301,   186, -1301, -1301, -1301,   333, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301,   289,    86, -1301,
     319,   342,   359,   144,   169, -1301,   377,   379,   381,   387,
     392, -1301,   203,   396,   397,   218,   224,   231,   237, -1301,
     243,   264,   266,   280,   290,   303,   419,   444,   306,   337,
     345,   351, -1301, -1301,   354, -1301, -1301,   378,  -169,    20,
     145,   589,   347, -1301, -1301,   581, -1301, -1301, -1301,    69,
   -1301,   548,    60,   268,   888,   329, -1301, -1301, -1301, -1301,
     380, -1301, -1301, -1301, -1301, -1301,   383,   394,   420,  1089,
   -1301,   437,   443, -1301, -1301, -1301, -1301,   445, -1301, -1301,
   -1301, -1301, -1301, -1301,   429,   441, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301,   446, -1301, -1301,   724,   752, -1301,   668,
     753,   743,   758,   663,   540, -1301,   665,   815,   669,    10,
     670, -1301,   671,   673,   674,   717,   677,   678,   679,     2,
     686, -1301,   -78,   687,   688,   213,   571,   694, -1301,   712,
   -1301,   714,   715, -1301,   716, -1301,   718,   719,   732,   733,
     737,   738,   739,   740,   742,   744,   745,   746,   747,   748,
     749, -1301,   755,   756,   757,   759,   760, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301,   761, -1301, -1301, -1301, -1301,   696,
   -1301, -1301, -1301,   581,   754, -1301,   159,    -7,   782, -1301,
   -1301,   765,   830,   768, -1301, -1301, -1301,    22, -1301,   767,
   -1301, -1301, -1301, -1301, -1301,    32, -1301, -1301,   770,   771,
   -1301, -1301,  -103, -1301,   772, -1301,   773,   137, -1301, -1301,
   -1301,   384,   864,   895, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301,   776, -1301, -1301, -1301,
   -1301,   578, -1301, -1301, -1301, -1301, -1301, -1301, -1301,   777,
   -1301,   778, -1301,   907, -1301, -1301, -1301, -1301,   190,   -48,
    -121,    26,   713, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301,   -29,   -22,  -121,  -121, -1301,   781,
     792,   793,   796, -1301,   799,   801,   814,   816,   817,   574,
     -14,   593, -1301, -1301,   602,   615,   822,   627,   628,   629,
      27,   630, -1301,   631, -1301, -1301, -1301, -1301, -1301, -1301,
     633,   634,   840,   636,   637,   843,   641,   654, -1301, -1301,
   -1301,   858,   859,   657,    41,   659,    41, -1301,   680,    41,
     699,    41, -1301,   700,   701,   720,   750,   751,   762,   764,
     774,   786,   787,   881,   788,   790,   846,   261,   794, -1301,
     795,   797,   703,    -6,   924,   945,   946,   798,   947, -1301,
     921, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301,   948,   800,   520,
    1019,    -3,   802,   432, -1301,   958,  1094,    -2, -1301, -1301,
      64,   959,   960,  -103,   966,   968,   974,   975, -1301, -1301,
     976, -1301, -1301,   -99,   803,   804,   806,   983,  1120,   -90,
     813,    31, -1301,   810,   436,   -21, -1301, -1301, -1301,   809,
    1001,  1004,  1010,   831,  1020,   832,  1038,   834,  1179,   835,
     836,   837,   -27,  1043,   839,   841, -1301,   842,   883, -1301,
   -1301,   -74,   844,    45, -1301, -1301,   -40,   -37,   -34,   -33,
    1046,  1180, -1301, -1301,   570,  1050,  1050,   468, -1301,   491,
   -1301,  1050, -1301, -1301,    47,  1051,  1052,   497, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301,    26,   -48,   -48,   291,
   -1301,    26,  -121,    16,    26, -1301, -1301,    26,    26,   409,
     300,   276, -1301,  1053,  1054,  1055,  1056,  1057,  1058,  1059,
    1060, -1301,  1061,     8, -1301, -1301, -1301,  1062,    14, -1301,
   -1301,    16,    16,  1065,   856,   857,  1064,   862,   863,   866,
     867,   868, -1301,   869,   870, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301,   872,   873, -1301,    92, -1301, -1301, -1301,   561,
    -129, -1301, -1301,   876,  1073, -1301, -1301,   878, -1301,   879,
   -1301, -1301, -1301,   880,   882, -1301,   884,   937, -1301,   885,
   -1301,   886,   937, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301,   887, -1301, -1301,  1083, -1301, -1301, -1301,
   -1301,   889, -1301,  1097, -1301, -1301,  1093,  1100,  1095,   893,
   -1301, -1301,   894, -1301,   565,  -112, -1301, -1301,  1098, -1301,
   -1301,   896,  -103, -1301,  1094, -1301, -1301, -1301, -1301,    -2,
     897, -1301, -1301, -1301,   898,   899,   900,   901,  -103,   902,
    1268,  1132,  1109,  -106,  1110, -1301, -1301, -1301, -1301, -1301,
   -1301,  1111, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
     909,   910,   911, -1301,   912, -1301, -1301, -1301,  1116, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301,   576,   -96, -1301,
   -1301, -1301, -1301, -1301,   914,  1121,  1118, -1301, -1301,  1122,
     183,   373, -1301,  1123,  1124, -1301,  1127,  1128, -1301,  1129,
    1130, -1301, -1301, -1301, -1301,  1131,  1133,  1134,  1135,  1136,
    1137,  1138,   240,   198,  1139,   510,  1140,  1141, -1301,  1144,
    1143,  1145,  1146,  1147,  1148,  1149,  1150,  1151,  1152,  1153,
    1154,  1155,  1156, -1301,  -101, -1301, -1301, -1301,   324,  1159,
    -103, -1301,  -103,  -103,  -103,  -103,  -103,  -103, -1301,   917,
   -1301,   471,  1157, -1301,  1158,  1160,  1161,  1162, -1301, -1301,
    1163,  1165,  1166,  1167, -1301,   360,  1088, -1301, -1301,  1168,
     940,   956, -1301, -1301,  1170,  1171,  1172, -1301, -1301, -1301,
   -1301, -1301,  1074,   470,   155,  1173,   969,   971,  1175,  1176,
     977,  1177,  1178,   978,    57,   979,  1181,  1185, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301,    12,   189, -1301,    51,
     -48,   -48,   -48,   -48, -1301, -1301, -1301,   189,   -30,  -121,
   -1301,   189, -1301,   741,   -12,   805, -1301, -1301, -1301, -1301,
   -1301, -1301,   -48,   -48, -1301, -1301, -1301,   -48,    26,    26,
      26,    26, -1301,  -121,  -121,  -121,   241,   241,   241,   241,
     241,   241,   241,   241,  1184,   982,  1115,  -230, -1301,  1187,
     985,  1114,  -230, -1301, -1301, -1301,   987, -1301, -1301,   988,
   -1301, -1301, -1301, -1301, -1301, -1301,   989, -1301,   990,  1192,
    1195,   -28, -1301, -1301, -1301, -1301, -1301,  1196,   296, -1301,
     992, -1301,  -105, -1301,  -115,  -103, -1301, -1301, -1301,   995,
   -1301, -1301,   996, -1301, -1301, -1301,  1200,   998,  1208,  1203,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,   999, -1301,
    -103, -1301, -1301,  -125, -1301,  1207, -1301, -1301, -1301, -1301,
    1003, -1301,  1209,  1210,  1005,  1008, -1301, -1301,  1009, -1301,
   -1301, -1301, -1301,  1011, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301,  1018,  1012,  1013,  1014,  1015,  1016,  1017,  -118,  1215,
    1021,  1022,  1221,  1224,  1023,  1225,  1024,  1025,  1231,  1230,
    1232,  1028,  1030,  1031,  1032,  1033,  1034,  1035, -1301, -1301,
   -1301, -1301,  1036, -1301, -1301, -1301,  1037,  1039, -1301, -1301,
   -1301, -1301, -1301, -1301,  1040,  1041,  1042,  1243,  1044,  1317,
    1317,  1317,  1045,  1047,  1317,  1317,  1317,  1317,  1356,  1356,
    1317,  1356,   349,   -57, -1301,   904,    32, -1301,   471, -1301,
    -103, -1301,  -103,  1048,  -103,  -103,  -103,  1049, -1301,  1251,
   -1301, -1301, -1301,  1253,  1254,  1255,  1256,  1259,  1258,  1260,
    1261,  1262,  1265, -1301, -1301, -1301,  1063,  1264, -1301,   -65,
    1267,   316, -1301, -1301,  1066,  1067,  1068,  1272, -1301, -1301,
   -1301,   484,  1273, -1301, -1301,  1276, -1301, -1301,  1405,  1408,
   -1301,  1414,  1415, -1301,  1247,  1277, -1301, -1301, -1301, -1301,
    1050,  1050, -1301, -1301,   -98, -1301,  1239,   -48, -1301,   343,
     343, -1301, -1301,  -121, -1301, -1301,    26, -1301,   370,   370,
     370, -1301, -1301,   562,   302,  1080,  1080,  1080,  1281, -1301,
   -1301,  1283,  1079,  1081,  1082,  1084,  1085,  1086,  1087,  1090,
   -1301,   -51, -1301, -1301, -1301, -1301,   -44, -1301, -1301, -1301,
   -1301, -1301,  1286,   416, -1301,  1291,   418, -1301,  1292,  1233,
   -1301,  1294,  1096,  1296,  1099, -1301,  1101,  1297,  1298,  1300,
    1301, -1301, -1301,  1303,  1305,  1306, -1301, -1301,  -103, -1301,
   -1301,   461, -1301, -1301,  1307,  1113, -1301, -1301,  1106, -1301,
   -1301, -1301, -1301,  1107,  1108, -1301, -1301, -1301, -1301,  1423,
   -1301, -1301, -1301, -1301, -1301, -1301,  1112, -1301, -1301, -1301,
   -1301,  1312,  1117, -1301,  1119, -1301, -1301,  1125,  1313,  1314,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301,   599,   -89, -1301, -1301, -1301,  1126,  1142,
    1164, -1301, -1301,  1169,  1174,  1182,  1183, -1301,  1186,  1188,
    1189,  1190, -1301, -1301, -1301, -1301,  1191, -1301,   923, -1301,
    1193,  1194,  1453,    82,  1457, -1301, -1301,  -103,  -103,  1457,
   -1301,  1320,  1197,  1198,  1199,  1201,  1202, -1301,   -84,  1204,
    1205,  1206,  1211, -1301,   -80, -1301,  1285, -1301, -1301,  1321,
   -1301, -1301, -1301, -1301, -1301,  1323, -1301, -1301, -1301,  1271,
    1325,   360,  1326,  1327,  1330,  1331, -1301,  1302,  1212,  1213,
      85, -1301, -1301, -1301,    59,  -155,   763,   191,   779, -1301,
    1270,  1270, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301,  1257, -1301,  1263,  1335,  1214, -1301,  1336,  1216,
   -1301, -1301,  1337,  1340,  1217, -1301,  1218, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
    1219,  1339,  1341, -1301, -1301, -1301,  1342, -1301,  1222, -1301,
   -1301, -1301,  1223,  1226, -1301, -1301, -1301, -1301, -1301,  1345,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301,  1346, -1301, -1301,
   -1301, -1301, -1301, -1301,  1227, -1301, -1301, -1301,  1347,  1348,
    1228,  1349,  1229,  -103,  -103,  1234,  1457, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
    1350,  1351,  1352,  1353, -1301,  1293,  1389,  1390,  1391,  1392,
    1358, -1301, -1301,  1508,  1360,  1363,  1411,  1364,  1365,  1366,
   -1301, -1301,  1328,   -48,  -121,    26,  1369,  1370,  1370,  1371,
    1257, -1301,  1373,  1263, -1301,  1235, -1301,  1236, -1301,    19,
    1374, -1301,  1375, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301,  1220, -1301,  1402, -1301,  1382,  1540, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301,  1541, -1301,  -103,
    1240, -1301,  1241,   180,  1237,  1242,  1380,  1381,  1383,  1384,
    1387,  1388,  1395,  1244,  1398,  1245, -1301,  1403,  1246,  1248,
    1249, -1301,   370,  1080,   430,  1406, -1301, -1301, -1301,  1250,
   -1301,  1252, -1301, -1301, -1301,  1410,  1413, -1301, -1301, -1301,
   -1301,   -42,  1341,   -41,   -26,   -25,  1341,   -19,   -87,  1418,
    1266,  1420,  1421,  1424,  1427,  1457, -1301, -1301,  1269, -1301,
   -1301,  1428,  1433,  1396,  1434,  1274,  1275, -1301, -1301, -1301,
    1278, -1301,  1279, -1301, -1301, -1301, -1301, -1301, -1301,  1376,
    1435, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,  1280,
   -1301, -1301, -1301,  1425,  1446,  1282, -1301,  1284,  1436,  1439,
    1450,  1556,  1564, -1301, -1301,  1378,  1385,  1458, -1301,  1460,
   -1301, -1301,  1461,  1462, -1301, -1301,  1463,  1464,  1287, -1301,
   -1301,  1465,  1466,  1467, -1301,  1474,  1341,  1470,  1471,  1473,
    1288, -1301, -1301, -1301, -1301,     0, -1301,     9, -1301,  1476,
   -1301, -1301, -1301, -1301,  1479, -1301,  1459,  1459, -1301, -1301,
   -1301, -1301,  1289,  1480,  1483, -1301, -1301,  1486,  1488, -1301,
   -1301,  1519, -1301,  1489, -1301, -1301,  1341
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
     130, -1301, -1301, -1301,  1026, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1068, -1301,  1069,
   -1301,   689, -1301,   447, -1301,   442, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,  1324,
    1393, -1301, -1301, -1301, -1301, -1301,   986, -1301,  -145, -1301,
    -311, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301,  1400,  1238, -1301,  -469, -1301, -1301,  -123,
   -1301, -1301, -1301, -1301, -1301, -1301,   980, -1301,  1329, -1301,
   -1301, -1301,  1526, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,  -871,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301,  -269, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301,   908, -1301,   573, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301,   376, -1301, -1301,
   -1301, -1301, -1301, -1301,   575,   845, -1301, -1301, -1301, -1301,
    -513,  -828, -1094, -1301, -1301, -1301, -1300,  -534, -1301, -1301,
   -1301, -1301, -1301,   -11, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301,   543, -1301, -1301,   118,   314, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301,  1532, -1301, -1301, -1301,  -552,
    -721,  -850,  -344,  -564,  -347,  -577, -1301, -1301, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,  -134, -1301,
   -1301, -1301, -1301, -1301,   297,   171, -1301, -1301,  -496, -1301,
    -861, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301, -1301,
    1092, -1301, -1301, -1301, -1301,   168, -1301, -1301, -1301, -1301,
   -1301, -1301,  1102, -1301, -1301, -1301, -1301,   172, -1301, -1301,
   -1301, -1301, -1301, -1301, -1301, -1301, -1301
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -610
static const yytype_int16 yytable[] =
{
     469,   527,   897,   573,   935,   569,  1115,   907,   703,   312,
     911,   876,   877,   912,   914,   677,  1334,   885,  1335,   601,
     602,   910,   275,   830,   834,  1173,   833,   480,   932,   835,
     837,   394,  1025,   515,   196,   742,   743,  1230,  1276,  1475,
    1595,  1103,   500,   528,   629,   494,   946,   197,   473,   954,
     955,   198,   951,   570,  1243,   501,   502,   503,   199,   779,
    1511,   684,   144,   591,  1237,   382,   481,     4,   194,   744,
     596,   504,   450,   780,   450,   557,   714,   767,   728,   781,
     722,   723,   724,   782,  1303,   887,   482,   745,   450,  1482,
     783,   784,   571,  1488,   262,  1164,   395,   403,   282,  1503,
     591,   776,  1468,   785,   483,   484,   596,   278,    98,   786,
    1357,   787,   788,   474,   746,  1244,   789,   790,   450,   704,
     495,  1504,   791,  1505,  1401,  1238,   396,   565,   200,   485,
     505,  1403,   201,  1545,  1545,   450,   792,   793,   450,    99,
     558,   450,   450,   762,   763,   764,   506,   519,   904,  1545,
    1545,   794,   905,   795,   206,   796,  1545,   797,   691,   207,
     208,   613,   486,   798,   799,   566,   729,   800,   801,   202,
     473,   473,   383,   802,   803,  1708,  1562,   730,   731,   209,
     404,   210,  1245,   211,  1710,   557,  1183,   203,   520,  1043,
    1186,  1239,  1240,   804,   397,   112,   148,  1506,   102,   212,
     570,   565,   622,  1073,   283,   805,   928,   929,   806,   195,
     105,   807,   930,   931,   708,  1152,   642,   774,  1469,   909,
     843,  1507,   592,   898,   899,   908,   705,  1512,   555,   597,
    1012,   915,   213,   114,   913,   474,   474,   143,  1319,   574,
    1321,   398,   575,   576,   614,  1074,   888,   889,   214,   975,
     558,   144,   577,  1259,  1508,   556,  1596,   215,   572,   592,
    1277,   496,  1165,  1246,   115,   597,  1006,   831,   831,   836,
     838,   507,  1026,  1241,  1511,   216,   467,  1104,   467,   715,
    1381,   217,  1037,   808,   287,   623,  1337,  1338,   725,  1447,
     218,   116,   467,  1646,  1483,   849,  1044,  1068,  1489,   643,
     117,   405,   219,  1382,   777,   809,   288,   810,   811,   812,
     813,   814,  1075,  1358,   220,  1655,   204,  1110,  1597,  1231,
    1114,  1327,   467,    17,   135,   567,    15,  1402,   521,   969,
    1069,   568,   473,   118,  1404,   747,  1641,  1642,   935,   467,
    1106,   136,   467,   487,   909,   467,   467,   732,   275,   119,
    1184,   765,  1643,  1644,    17,   399,   508,  1509,   123,  1645,
     678,   141,  1107,   557,  1191,  1192,  1193,  1194,  1187,   120,
     815,   816,   817,   818,   819,   820,   821,   822,  1709,  1070,
    1560,  1071,  1153,   221,   121,   261,  1154,  1711,   824,   122,
     909,   128,   970,   823,   904,  1214,   262,   474,   905,   567,
    1219,   289,   222,  1175,   223,   578,  1511,   928,   929,   928,
     929,   290,   263,   930,   931,   930,   931,   224,   103,   104,
    1045,  1133,   129,  1010,   900,   901,   902,   903,   558,  1046,
    1047,  1178,   900,   901,   902,   903,   225,   226,   130,  1020,
     131,  1512,   138,  1134,  1135,   124,   125,   735,   736,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   264,
     148,   335,   156,  1048,   157,  1615,  1601,  1602,  1603,  1604,
    1605,  1606,  1607,   191,   192,  1198,  1199,  1200,   237,   238,
     239,   240,   241,   242,   243,   244,   245,   246,   247,   248,
     249,   250,   158,   251,   916,   917,   918,   919,   920,   921,
     933,   934,   265,   252,   253,   254,   408,   409,   410,  1473,
    1474,  1201,   255,   256,   257,   159,     1,  1078,   928,   929,
     928,   929,   161,    -7,   930,   931,   930,  1079,   336,   337,
     338,   339,   340,   341,   160,  1514,   535,  1515,  1514,    19,
      20,    21,  1080,   737,  1260,   738,   646,   162,    -7,   649,
     164,   651,   165,    -7,   166,  1105,  1179,  1180,  1181,  1182,
     167,   831,  1185,  1081,   831,   168,  1116,  1117,  1118,   171,
     536,  1176,   172,  1512,   132,   133,   537,    -7,  1188,  1189,
      -7,   170,   538,  1190,    -7,   266,  1195,  1196,  1197,   739,
     740,    -7,   184,    -7,  1309,  1310,   173,    -7,  1313,  1314,
    1315,  1316,   174,  1686,  1320,   696,   697,   698,   842,   175,
     843,   844,   845,   846,   847,   176,   534,   185,  1082,   260,
     848,   178,  1387,   667,   668,   669,   670,   916,   917,   918,
     919,   920,   921,   922,   923,   539,  1378,  1379,   878,   879,
     540,  1083,   179,   535,   180,  1559,   144,    -7,   928,   929,
     909,    -7,   524,   525,   930,   931,  1726,   924,   181,   925,
     926,   881,   882,     5,   900,   901,   902,   903,   182,   904,
     893,   894,   895,   905,    -7,   977,  1234,   536,   904,    -7,
      -7,   183,   905,   537,   186,   849,   850,   851,     6,   538,
     852,   853,   449,     7,  1360,  1140,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,  1322,  1323,  1324,  1325,   187,  1248,     8,   902,   903,
       9,   281,   854,   188,    10,    -7,    -7,    -7,   855,   189,
    1269,    11,   190,    12,   972,   973,   974,    13,  1003,  1004,
    1005,  1258,   539,   900,   901,   902,   903,   540,    -7,  1034,
    1035,  1036,  1148,  1149,  1150,    -7,    -7,    -7,   347,    -7,
      -7,    -7,    -7,    -7,   349,    -7,  1366,  1367,  1368,    -7,
      -7,    -7,  1444,  1445,  1446,   350,    -7,   856,   857,   858,
     928,   929,   900,   901,   902,   903,    -7,    14,    -7,    -7,
     924,    15,   925,   926,  1330,  1222,  1406,  1225,  1409,   859,
     860,   351,   861,  1203,  1204,  1205,  1206,  1207,  1208,  1209,
     366,   935,  1427,  1428,    16,   362,    -7,    -7,    -7,    17,
      18,   363,   367,   365,   368,   370,   371,   373,   374,   375,
      -7,   377,    -7,  1384,  1328,    -7,  1385,   376,   378,  1388,
     379,   380,  1386,   390,   381,   385,   387,  1339,   388,   389,
     862,   863,   391,   392,   393,   864,   865,   866,   867,   868,
     869,   401,   406,   407,   412,    19,    20,    21,   449,   413,
      -7,   450,   451,   452,   453,   454,   455,   456,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   415,    22,   417,
     418,   420,   491,   422,   423,    23,    24,    25,   690,    26,
      27,    28,    29,    30,   295,    31,   296,   424,   425,    32,
      33,    34,   426,   427,   428,   429,    35,   430,   489,   431,
     432,   433,   434,   435,   436,   297,    36,   472,    37,    38,
     438,   439,   440,   529,   441,   442,   447,   262,   490,   298,
     299,   492,   498,   512,   513,   552,   582,   517,   518,   531,
     548,  1584,   612,   550,   603,   300,    39,    40,    41,   916,
     917,   918,   919,   920,   921,   922,   923,   604,   605,   606,
      42,   615,    43,   301,   607,    44,   608,   302,  1511,  1426,
     616,   916,   917,   918,   919,   920,   921,   922,   923,   609,
     303,   610,   611,   617,  1511,   618,   304,   916,   917,   918,
     919,   920,   921,   933,   934,   619,   620,   621,   624,   626,
      45,   627,   628,   629,   632,   666,   633,  1464,   634,   635,
     305,   306,   307,   916,   917,   918,   919,   920,   921,   933,
     934,   308,   636,   637,   639,   641,   449,   645,   309,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   449,   663,   676,   648,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   466,   467,   449,   650,   653,   654,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   464,   465,   449,   687,   680,   655,   451,
     452,   453,   454,   455,   456,   457,   458,   459,   460,   461,
     462,   463,   464,   465,   900,   901,   902,   903,   681,   682,
     684,  1178,   924,   688,   925,   926,   310,   352,   656,   657,
     692,   700,   701,   721,   706,   707,   900,   901,   902,   903,
     658,   709,   659,   710,   924,  1512,   925,   926,   353,   711,
     712,   713,   660,   909,   354,   311,   720,   355,   356,   734,
     924,  1512,   925,   926,   661,   662,   664,  1583,   665,  1582,
     727,   357,   672,   674,   750,   675,   683,   751,   689,   909,
     695,   717,   718,   752,   719,  1184,   924,   749,   925,   926,
     449,   358,   758,   754,   451,   452,   453,   454,   455,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   753,
     755,   756,   757,   759,   760,   761,   767,   770,   840,   771,
     772,   839,   778,   875,   891,   892,   936,   937,   938,   939,
     940,   941,   942,   943,   957,   958,   945,   950,   956,   959,
     960,   961,   526,   467,   962,   963,   964,   965,   980,   966,
     967,   988,   968,   359,   360,   977,   981,   983,   994,   985,
     986,   773,   987,   990,   991,   993,   996,   995,   998,   999,
    1000,  1001,  1002,  1008,  1009,  1014,  1016,  1017,  1018,  1019,
    1021,  1022,  1329,  1023,  1024,  1027,  1028,  1029,  1030,  1031,
    1032,  1033,  1039,  1041,  1040,  1120,  1137,  1042,  1050,  1051,
    1053,  1463,  1147,  1054,  1056,  1057,  1061,  1122,  1062,  1063,
    1064,  1065,  1066,  1067,  1077,  1085,  1086,  1088,  1089,  1140,
    1090,  1091,  1092,  1093,  1094,  1095,  1096,  1097,  1098,  1099,
    1100,  1101,  1109,  1123,  1143,  1124,  1125,  1126,  1128,  1648,
    1129,  1130,  1131,  1138,  1144,  1145,  1146,  1156,   776,  1157,
    1158,  1159,  1161,  1162,  1168,  1160,  1163,  1167,  1169,  1210,
    1212,  1213,  1215,  1217,  1218,  1220,  1221,  1228,  1222,  1225,
    1229,  1233,  1236,  1249,  1250,  1252,  1253,  1254,  1255,  1257,
    1261,  1262,  1307,  1265,  1263,  1264,  1266,  1267,  1278,  1268,
    1270,  1271,  1272,  1273,  1274,  1275,  1281,  1282,  1284,  1279,
    1280,  1283,  1285,  1286,  1287,  1288,  1290,  1289,  1291,  1292,
    1293,  1294,  1295,  1296,  1297,  1298,  1303,  1299,  1300,  1301,
    1302,  1317,  1306,  1311,  1342,  1312,  1336,  1340,  1343,  1344,
    1345,  1346,  1347,  1349,  1371,  1350,  1351,  1352,  1353,  1356,
    1372,  1355,  1359,  1373,  1362,  1363,  1364,  1365,  1370,  1374,
    1375,  1376,  1377,  1383,   909,  1389,  1392,  1393,  1436,  1394,
    1395,  1405,  1396,  1397,  1398,  1399,  1408,  1411,  1400,  1414,
    1412,  1416,  1419,  1420,  1415,  1421,  1422,  1417,  1423,  1418,
    1424,  1425,  1430,  1431,  1433,  1434,  1435,  1438,  1442,  1443,
    1437,  1467,  1471,  1476,  1490,  1439,  1491,  1440,  1492,  1493,
    1494,  1496,  1497,  1441,  1450,  1498,  1499,  1516,  1500,  1519,
    1525,  1527,  1530,  1531,  1544,  1522,  1545,  1547,  1551,  1552,
    1451,  1568,  1554,  1555,  1557,  1563,  1564,  1565,  1566,  1569,
    1570,  1571,  1572,  1573,  1574,  1575,  1576,  1577,  1611,  1578,
    1579,  1580,  1452,  1581,  1585,  1586,  1589,  1453,  1591,  1599,
    1600,  1612,  1454,  1613,  1614,  1621,  1622,  1659,  1624,  1625,
    1455,  1456,  1626,  1627,  1458,  1672,  1459,  1460,  1461,  1462,
    1628,  1465,  1466,  1630,  1609,  1477,  1478,  1479,  1632,  1480,
    1481,  1636,  1484,  1485,  1486,  1639,  1673,  1532,  1640,  1487,
    1501,  1502,  1526,  1649,  1528,  1651,  1652,  1543,  1533,  1653,
    1548,  1549,  1654,  1657,  1550,  1553,  1556,  1558,  1658,  1660,
    1668,  1676,  1561,  1667,  1677,  1593,  1594,  1619,  1616,  1617,
    1620,  1679,  1629,  1631,  1633,  1678,  1634,  1635,  1637,  1680,
    1638,  1681,  1715,  1683,  1682,  1684,  1687,  1688,  1689,  1690,
    1694,  1696,  1698,  1699,  1650,  1702,  1703,  1656,  1704,  1718,
    1669,  1712,  -608,  -609,  1714,  1723,  1663,  1664,  1720,  1717,
    1674,  1721,  1675,  1722,  1725,  1691,  1705,  1235,  1410,   475,
    1407,  1007,   559,   471,  1011,   280,  1038,   560,   992,  1331,
    1448,  1618,  1707,  1332,  1361,  1495,  1121,   346,  1518,  1588,
     953,  1592,  1590,     0,     0,   948,     0,     0,     0,   976,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   694
};

static const yytype_int16 yycheck[] =
{
     269,   312,   566,   350,   581,   349,   834,   571,   477,   154,
     574,   545,   546,   577,   578,    21,  1110,   551,  1112,   366,
     367,   573,   145,    63,   537,   896,    63,    34,   580,    63,
      63,    29,   138,   302,    14,    56,    57,    65,   156,  1339,
      21,   142,    10,   312,   173,    23,    38,    27,   173,   601,
     602,    31,    38,   174,   169,    23,    24,    25,    38,    14,
     215,   173,    65,    92,   169,    55,    73,     0,   237,    90,
      92,    39,   175,    28,   175,   173,   175,   173,    47,    34,
     170,   171,   172,    38,   173,    38,    93,   108,   175,   173,
      45,    46,   213,   173,    49,    38,    94,   175,    38,    14,
      92,   175,    20,    58,   111,   112,    92,    38,   175,    64,
     175,    66,    67,   238,   135,   230,    71,    72,   175,    55,
      98,    36,    77,    38,   175,   230,   124,   175,   108,   136,
      98,   175,   112,   175,   175,   175,    91,    92,   175,    68,
     238,   175,   175,   170,   171,   172,   114,    10,   378,   175,
     175,   106,   382,   108,     9,   110,   175,   112,   469,    14,
      15,   175,   169,   118,   119,   213,   135,   122,   123,   149,
     173,   173,   162,   128,   129,   175,  1476,   146,   147,    34,
     258,    36,   297,    38,   175,   173,   907,   167,    51,     6,
     911,   296,   297,   148,   192,   174,   127,   112,   175,    54,
     174,   175,   175,     5,   144,   160,   218,   219,   163,   378,
     173,   166,   224,   225,   483,    60,   175,   528,   136,   374,
      40,   136,   251,   567,   568,   572,   162,   382,    38,   251,
     699,   578,    87,   175,   578,   238,   238,    51,  1099,   213,
    1101,   239,   216,   217,   258,    47,   199,   200,   103,   378,
     238,    65,   226,   378,   169,    65,   237,   112,   379,   251,
     378,   239,   205,   378,   175,   251,   378,   536,   537,   538,
     539,   239,   378,   378,   215,   130,   379,   378,   379,   378,
     378,   136,   378,   238,    16,   258,  1114,  1115,   378,   378,
     145,   175,   379,   380,   378,   115,   113,    57,   378,   258,
     175,   379,   157,  1174,   378,   260,    38,   262,   263,   264,
     265,   266,   114,   378,   169,  1615,   296,   830,   299,   347,
     833,   378,   379,   164,    85,   373,   136,   378,   191,   237,
      90,   379,   173,   175,   378,   356,   378,   378,   915,   379,
      16,   102,   379,   350,   374,   379,   379,   316,   471,   175,
     380,   378,   378,   378,   164,   353,   324,   272,   174,   378,
     366,   173,    38,   173,   928,   929,   930,   931,   380,   175,
     325,   326,   327,   328,   329,   330,   331,   332,   378,   139,
    1474,   141,   227,   238,   175,    38,   231,   378,   533,   175,
     374,   175,   300,   348,   378,   947,    49,   238,   382,   373,
     952,   133,   257,   214,   259,   379,   215,   218,   219,   218,
     219,   143,    65,   224,   225,   224,   225,   272,    95,    96,
      47,    61,   175,   692,   373,   374,   375,   376,   238,    56,
      57,   380,   373,   374,   375,   376,   291,   292,   175,   708,
     175,   382,   175,    83,    84,    95,    96,    11,    12,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   112,
     127,   132,   173,    90,   378,  1559,  1534,  1535,  1536,  1537,
    1538,  1539,  1540,    95,    96,   234,   235,   236,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   173,   348,   218,   219,   220,   221,   222,   223,
     224,   225,   155,   358,   359,   360,   293,   294,   295,  1337,
    1338,   270,   367,   368,   369,   173,     1,     7,   218,   219,
     218,   219,   378,     8,   224,   225,   224,    17,   199,   200,
     201,   202,   203,   204,   175,  1385,    65,  1387,  1388,   210,
     211,   212,    32,   107,  1013,   109,   416,   378,    33,   419,
     173,   421,   173,    38,   173,   824,   900,   901,   902,   903,
     173,   830,   909,    53,   833,   173,   835,   836,   837,   173,
      99,   382,   175,   382,   318,   319,   105,    62,   922,   923,
      65,   378,   111,   927,    69,   238,   933,   934,   935,   153,
     154,    76,   173,    78,  1090,  1091,   378,    82,  1094,  1095,
    1096,  1097,   378,  1671,  1100,   173,   174,   175,    38,   378,
      40,    41,    42,    43,    44,   378,    38,   173,   108,    30,
      50,   378,  1186,   362,   363,   364,   365,   218,   219,   220,
     221,   222,   223,   224,   225,   164,  1170,  1171,   170,   171,
     169,   131,   378,    65,   378,  1473,    65,   132,   218,   219,
     374,   136,   268,   269,   224,   225,  1724,   381,   378,   383,
     384,   170,   171,     8,   373,   374,   375,   376,   378,   378,
     173,   174,   175,   382,   159,   379,   380,    99,   378,   164,
     165,   378,   382,   105,   378,   115,   116,   117,    33,   111,
     120,   121,   172,    38,   378,   379,   176,   177,   178,   179,
     180,   181,   182,   183,   184,   185,   186,   187,   188,   189,
     190,   362,   363,   364,   365,   378,   985,    62,   375,   376,
      65,   173,   152,   378,    69,   210,   211,   212,   158,   378,
    1041,    76,   378,    78,   173,   174,   175,    82,   173,   174,
     175,  1010,   164,   373,   374,   375,   376,   169,   233,   173,
     174,   175,   282,   283,   284,   240,   241,   242,   378,   244,
     245,   246,   247,   248,   381,   250,   282,   283,   284,   254,
     255,   256,   173,   174,   175,   381,   261,   207,   208,   209,
     218,   219,   373,   374,   375,   376,   271,   132,   273,   274,
     381,   136,   383,   384,  1105,   379,   380,   379,   380,   229,
     230,   381,   232,   937,   938,   939,   940,   941,   942,   943,
     381,  1388,   351,   352,   159,   378,   301,   302,   303,   164,
     165,   378,   381,   378,   378,   101,    74,   159,    75,    86,
     315,   168,   317,  1177,  1103,   320,  1183,    79,   298,  1186,
     175,    26,  1186,   126,   175,   175,   175,  1116,   175,   175,
     280,   281,   175,   175,   175,   285,   286,   287,   288,   289,
     290,   175,   175,   175,   293,   210,   211,   212,   172,   175,
     355,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,   187,   188,   189,   190,   175,   233,   175,
     175,   175,    62,   175,   175,   240,   241,   242,   378,   244,
     245,   246,   247,   248,    16,   250,    18,   175,   175,   254,
     255,   256,   175,   175,   175,   175,   261,   175,   136,   175,
     175,   175,   175,   175,   175,    37,   271,   173,   273,   274,
     175,   175,   175,    38,   175,   175,   175,    49,   173,    51,
      52,   173,   175,   173,   173,    38,   233,   175,   175,   173,
     173,  1515,   378,   175,   173,    67,   301,   302,   303,   218,
     219,   220,   221,   222,   223,   224,   225,   175,   175,   173,
     315,   378,   317,    85,   175,   320,   175,    89,   215,  1248,
     378,   218,   219,   220,   221,   222,   223,   224,   225,   175,
     102,   175,   175,   378,   215,   173,   108,   218,   219,   220,
     221,   222,   223,   224,   225,   378,   378,   378,   378,   378,
     355,   378,   378,   173,   378,   169,   379,  1328,   175,   378,
     132,   133,   134,   218,   219,   220,   221,   222,   223,   224,
     225,   143,   378,   175,   175,   378,   172,   378,   150,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   172,   175,   354,   378,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   378,   379,   172,   378,   378,   378,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   172,   175,   173,   378,   176,
     177,   178,   179,   180,   181,   182,   183,   184,   185,   186,
     187,   188,   189,   190,   373,   374,   375,   376,   173,   173,
     173,   380,   381,   175,   383,   384,   238,    38,   378,   378,
     111,   173,    38,    13,   175,   175,   373,   374,   375,   376,
     378,   175,   378,   175,   381,   382,   383,   384,    59,   175,
     175,   175,   378,   374,    65,   267,   173,    68,    69,   349,
     381,   382,   383,   384,   378,   378,   378,  1514,   378,  1513,
     357,    82,   378,   378,   173,   378,   378,   173,   378,   374,
     378,   378,   378,   173,   378,   380,   381,   378,   383,   384,
     172,   102,    13,   173,   176,   177,   178,   179,   180,   181,
     182,   183,   184,   185,   186,   187,   188,   189,   190,   378,
     378,   173,   378,   378,   378,   378,   173,   378,    38,   378,
     378,   175,   378,   173,   173,   173,   173,   173,   173,   173,
     173,   173,   173,   173,   378,   378,   175,   175,   173,   175,
     378,   378,   378,   379,   378,   378,   378,   378,   175,   379,
     378,   314,   379,   164,   165,   379,   378,   378,   175,   379,
     378,   378,   378,   378,   378,   378,   169,   378,   175,   169,
     175,   378,   378,   175,   378,   378,   378,   378,   378,   378,
     378,    13,   378,   151,   175,   175,   175,   378,   378,   378,
     378,   175,   378,   175,   173,   378,   208,   175,   175,   175,
     173,   378,   228,   175,   175,   175,   175,   150,   175,   175,
     175,   175,   175,   175,   175,   175,   175,   173,   175,   379,
     175,   175,   175,   175,   175,   175,   175,   175,   175,   175,
     175,   175,   173,   175,   378,   175,   175,   175,   175,  1608,
     175,   175,   175,   175,   174,   174,   174,   378,   175,   378,
     175,   175,   175,   175,   173,   378,   378,   378,   173,   175,
     378,   246,   175,   378,   250,   378,   378,   175,   379,   379,
     175,   175,   380,   378,   378,   175,   378,   169,   175,   380,
     173,   378,    65,   378,   175,   175,   378,   378,   173,   378,
     378,   378,   378,   378,   378,   378,   175,   173,   173,   378,
     378,   378,   378,   378,   173,   175,   378,   175,   378,   378,
     378,   378,   378,   378,   378,   378,   173,   378,   378,   378,
     378,    65,   378,   378,   173,   378,   378,   378,   175,   175,
     175,   175,   173,   175,   158,   175,   175,   175,   173,   175,
      35,   378,   175,    35,   378,   378,   378,   175,   175,    35,
      35,   204,   175,   214,   374,   174,   173,   378,    35,   378,
     378,   175,   378,   378,   378,   378,   175,   175,   378,   175,
     237,   175,   175,   175,   378,   175,   175,   378,   175,   378,
     175,   175,   175,   370,   378,   378,   378,   175,   175,   175,
     378,    38,    35,   173,   209,   378,   175,   378,   175,   228,
     175,   175,   175,   378,   378,   175,   175,   237,   206,   252,
     175,   175,   175,   173,   175,   252,   175,   175,   173,   173,
     378,   228,   175,   175,   175,   175,   175,   175,   175,   140,
     140,   140,   140,   175,    26,   175,   173,   126,   136,   175,
     175,   175,   378,   215,   175,   175,   175,   378,   175,   175,
     175,   169,   378,    13,    13,   175,   175,   161,   175,   175,
     378,   378,   175,   175,   378,   140,   378,   378,   378,   378,
     175,   378,   378,   175,   354,   378,   378,   378,   175,   378,
     378,   175,   378,   378,   378,   175,   140,   370,   175,   378,
     378,   378,   378,   175,   378,   175,   175,   378,   380,   175,
     378,   378,   175,   175,   378,   378,   378,   378,   175,   175,
     175,   175,   378,   237,   175,   380,   380,   380,   378,   378,
     378,    65,   378,   378,   378,   175,   378,   378,   378,    65,
     378,   253,   173,   175,   249,   175,   175,   175,   175,   175,
     175,   175,   175,   169,   378,   175,   175,   378,   175,   169,
     370,   175,   378,   378,   175,   136,   378,   378,   175,   370,
     378,   175,   378,   175,   175,   378,   378,   978,  1226,   276,
    1223,   685,   348,   273,   694,   149,   768,   348,   652,  1106,
    1304,  1563,  1693,  1108,  1141,  1371,   841,   155,  1391,  1518,
     598,  1523,  1520,    -1,    -1,   593,    -1,    -1,    -1,   630,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   471
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     1,   386,   391,     0,     8,    33,    38,    62,    65,
      69,    76,    78,    82,   132,   136,   159,   164,   165,   210,
     211,   212,   233,   240,   241,   242,   244,   245,   246,   247,
     248,   250,   254,   255,   256,   261,   271,   273,   274,   301,
     302,   303,   315,   317,   320,   355,   387,   389,   390,   392,
     393,   394,   395,   396,   397,   398,   402,   403,   407,   408,
     472,   474,   475,   476,   493,   495,   496,   513,   514,   519,
     520,   527,   528,   529,   541,   542,   553,   555,   656,   658,
     676,   678,   680,   691,   717,   718,   719,   720,   721,   733,
     745,   746,   747,   748,   749,   750,   751,   659,   175,    68,
     409,   556,   175,    95,    96,   173,   543,   494,   681,   682,
     683,   692,   174,   388,   175,   175,   175,   175,   175,   175,
     175,   175,   175,   174,    95,    96,   677,   679,   175,   175,
     175,   175,   318,   319,   399,    85,   102,   400,   175,   404,
     412,   173,   477,    51,    65,   498,   504,   507,   127,   516,
     517,   518,   522,   546,   559,   662,   173,   378,   173,   173,
     175,   378,   378,   530,   173,   173,   173,   173,   173,   693,
     378,   173,   175,   378,   378,   378,   378,   722,   378,   378,
     378,   378,   378,   378,   173,   173,   378,   378,   378,   378,
     378,    95,    96,   401,   237,   378,    14,    27,    31,    38,
     108,   112,   149,   167,   296,   405,     9,    14,    15,    34,
      36,    38,    54,    87,   103,   112,   130,   136,   145,   157,
     169,   238,   257,   259,   272,   291,   292,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   333,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   348,   358,   359,   360,   367,   368,   369,   410,   413,
      30,    38,    49,    65,   112,   155,   238,   478,   482,   483,
     486,   487,   491,   497,   508,   504,   505,   509,    38,   515,
     517,   173,    38,   144,   521,   523,   526,    16,    38,   133,
     143,   544,   547,   548,   549,    16,    18,    37,    51,    52,
      67,    85,    89,   102,   108,   132,   133,   134,   143,   150,
     238,   267,   483,   554,   560,   563,   567,   572,   573,   574,
     575,   576,   577,   578,   580,   582,   583,   585,   586,   587,
     631,   632,   633,   635,   636,   132,   199,   200,   201,   202,
     203,   204,   584,   657,   663,   669,   680,   378,   532,   381,
     381,   381,    38,    59,    65,    68,    69,    82,   102,   164,
     165,   694,   378,   378,   734,   378,   381,   381,   378,   473,
     101,    74,   406,   159,    75,    86,    79,   168,   298,   175,
      26,   175,    55,   162,   462,   175,   411,   175,   175,   175,
     126,   175,   175,   175,    29,    94,   124,   192,   239,   353,
     461,   175,   415,   175,   258,   379,   175,   175,   293,   294,
     295,   440,   293,   175,   418,   175,   419,   175,   175,   421,
     175,   422,   175,   175,   175,   175,   175,   175,   175,   175,
     175,   175,   175,   175,   175,   175,   175,   424,   175,   175,
     175,   175,   175,   428,   492,   484,   488,   175,   479,   172,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   378,   379,   485,   552,
     489,   498,   173,   173,   238,   475,   499,   500,   501,   506,
      34,    73,    93,   111,   112,   136,   169,   350,   510,   136,
     173,    62,   173,   524,    23,    98,   239,   545,   175,   550,
      10,    23,    24,    25,    39,    98,   114,   239,   324,   568,
     634,   579,   173,   173,   581,   552,   588,   175,   175,    10,
      51,   191,   564,   561,   268,   269,   378,   485,   552,    38,
     557,   173,   622,   591,    38,    65,    99,   105,   111,   164,
     169,   609,   610,   618,   638,   665,   666,   667,   173,   668,
     175,   664,    38,   660,   670,    38,    65,   173,   238,   474,
     513,   531,   533,   534,   537,   175,   213,   373,   379,   687,
     174,   213,   379,   689,   213,   216,   217,   226,   379,   687,
     688,   689,   233,   696,   701,   695,   698,   702,   697,   699,
     700,    92,   251,   736,   737,   738,    92,   251,   724,   725,
     726,   689,   689,   173,   175,   175,   173,   175,   175,   175,
     175,   175,   378,   175,   258,   378,   378,   378,   173,   378,
     378,   378,   175,   258,   378,   414,   378,   378,   378,   173,
     453,   454,   378,   379,   175,   378,   378,   175,   416,   175,
     417,   378,   175,   258,   435,   378,   435,   420,   378,   435,
     378,   435,   423,   378,   378,   378,   378,   378,   378,   378,
     378,   378,   378,   175,   378,   378,   169,   362,   363,   364,
     365,   466,   378,   425,   378,   378,   354,    21,   366,   429,
     173,   173,   173,   378,   173,   480,   481,   175,   175,   378,
     378,   485,   111,   490,   499,   378,   173,   174,   175,   502,
     173,    38,   511,   501,    55,   162,   175,   175,   552,   175,
     175,   175,   175,   175,   175,   378,   525,   378,   378,   378,
     173,    13,   170,   171,   172,   378,   551,   357,    47,   135,
     146,   147,   316,   570,   349,    11,    12,   107,   109,   153,
     154,   571,    56,    57,    90,   108,   135,   356,   569,   378,
     173,   173,   173,   378,   173,   378,   173,   378,    13,   378,
     378,   378,   170,   171,   172,   378,   565,   173,   562,   566,
     378,   378,   378,   378,   485,   558,   175,   378,   378,    14,
      28,    34,    38,    45,    46,    58,    64,    66,    67,    71,
      72,    77,    91,    92,   106,   108,   110,   112,   118,   119,
     122,   123,   128,   129,   148,   160,   163,   166,   238,   260,
     262,   263,   264,   265,   266,   325,   326,   327,   328,   329,
     330,   331,   332,   348,   483,   589,   592,   603,   604,   611,
      63,   552,   615,    63,   615,    63,   552,    63,   552,   175,
      38,   613,    38,    40,    41,    42,    43,    44,    50,   115,
     116,   117,   120,   121,   152,   158,   207,   208,   209,   229,
     230,   232,   280,   281,   285,   286,   287,   288,   289,   290,
     637,   639,   643,   644,   652,   173,   622,   622,   170,   171,
     623,   170,   171,   630,   674,   622,   661,    38,   199,   200,
     671,   173,   173,   173,   174,   175,   535,   688,   687,   687,
     373,   374,   375,   376,   378,   382,   684,   688,   689,   374,
     684,   688,   688,   687,   688,   689,   218,   219,   220,   221,
     222,   223,   224,   225,   381,   383,   384,   690,   218,   219,
     224,   225,   684,   224,   225,   690,   173,   173,   173,   173,
     173,   173,   173,   173,   739,   175,    38,   735,   737,   727,
     175,    38,   723,   725,   684,   684,   173,   378,   378,   175,
     378,   378,   378,   378,   378,   378,   379,   378,   379,   237,
     300,   706,   173,   174,   175,   378,   454,   379,   455,   456,
     175,   378,   441,   378,   447,   379,   378,   378,   314,   439,
     378,   378,   439,   378,   175,   378,   169,   465,   175,   169,
     175,   378,   378,   173,   174,   175,   378,   481,   175,   378,
     552,   511,   501,   503,   378,   512,   378,   378,   378,   378,
     552,   378,    13,   151,   175,   138,   378,   175,   175,   378,
     378,   378,   378,   175,   173,   174,   175,   378,   566,   378,
     173,   175,   175,     6,   113,    47,    56,    57,    90,   590,
     175,   175,   597,   173,   175,   593,   175,   175,   594,   595,
     596,   175,   175,   175,   175,   175,   175,   175,    57,    90,
     139,   141,   607,     5,    47,   114,   608,   175,     7,    17,
      32,    53,   108,   131,   606,   175,   175,   598,   173,   175,
     175,   175,   175,   175,   175,   175,   175,   175,   175,   175,
     175,   175,   599,   142,   378,   552,    16,    38,   605,   173,
     615,   552,   616,   617,   615,   616,   552,   552,   552,   619,
     378,   610,   150,   175,   175,   175,   175,   640,   175,   175,
     175,   175,   641,    61,    83,    84,   653,   208,   175,   650,
     379,   648,   649,   378,   174,   174,   174,   228,   282,   283,
     284,   645,    60,   227,   231,   647,   378,   378,   175,   175,
     378,   175,   175,   378,    38,   205,   675,   378,   173,   173,
     672,   673,   538,   534,   536,   214,   382,   685,   380,   687,
     687,   687,   687,   685,   380,   689,   685,   380,   687,   687,
     687,   688,   688,   688,   688,   689,   689,   689,   234,   235,
     236,   270,   703,   703,   703,   703,   703,   703,   703,   703,
     175,   740,   378,   246,   684,   175,   728,   378,   250,   684,
     378,   378,   379,   457,   458,   379,   459,   460,   175,   175,
      65,   347,   711,   175,   380,   456,   380,   169,   230,   296,
     297,   378,   442,   169,   230,   297,   378,   448,   552,   378,
     378,   426,   175,   378,   169,   175,   430,   380,   552,   378,
     501,   173,   378,   175,   175,   378,   378,   378,   378,   485,
     378,   378,   378,   378,   378,   378,   156,   378,   173,   378,
     378,   175,   173,   378,   173,   378,   378,   173,   175,   175,
     378,   378,   378,   378,   378,   378,   378,   378,   378,   378,
     378,   378,   378,   173,   601,   602,   378,    65,   713,   713,
     713,   378,   378,   713,   713,   713,   713,    65,   715,   715,
     713,   715,   362,   363,   364,   365,   600,   378,   552,   378,
     485,   568,   609,   612,   617,   617,   378,   616,   616,   552,
     378,   620,   173,   175,   175,   175,   175,   173,   654,   175,
     175,   175,   175,   173,   655,   378,   175,   175,   378,   175,
     378,   649,   378,   378,   378,   175,   282,   283,   284,   646,
     175,   158,    35,    35,    35,    35,   204,   175,   622,   622,
     539,   378,   534,   214,   687,   689,   687,   688,   689,   174,
     704,   705,   173,   378,   378,   378,   378,   378,   378,   378,
     378,   175,   378,   175,   378,   175,   380,   458,   175,   380,
     460,   175,   237,   712,   175,   378,   175,   378,   378,   175,
     175,   175,   175,   175,   175,   175,   552,   351,   352,   463,
     175,   370,   452,   378,   378,   378,    35,   378,   175,   378,
     378,   378,   175,   175,   173,   174,   175,   378,   602,   714,
     378,   378,   378,   378,   378,   378,   378,   716,   378,   378,
     378,   378,   378,   378,   485,   378,   378,    38,    20,   136,
     614,    35,   621,   616,   616,   621,   173,   378,   378,   378,
     378,   378,   173,   378,   378,   378,   378,   378,   173,   378,
     209,   175,   175,   228,   175,   653,   175,   175,   175,   175,
     206,   378,   378,    14,    36,    38,   112,   136,   169,   272,
     540,   215,   382,   686,   686,   686,   237,   709,   709,   252,
     741,   742,   252,   729,   730,   175,   378,   175,   378,   707,
     175,   173,   370,   380,   446,   445,   443,   444,   451,   450,
     449,   436,   427,   378,   175,   175,   431,   175,   378,   378,
     378,   173,   173,   378,   175,   175,   378,   175,   378,   616,
     617,   378,   621,   175,   175,   175,   175,   642,   228,   140,
     140,   140,   140,   175,    26,   175,   173,   126,   175,   175,
     175,   215,   687,   689,   688,   175,   175,   710,   710,   175,
     742,   175,   730,   380,   380,    21,   237,   299,   708,   175,
     175,   452,   452,   452,   452,   452,   452,   452,   437,   354,
     464,   136,   169,    13,    13,   617,   378,   378,   652,   380,
     378,   175,   175,   651,   175,   175,   175,   175,   175,   378,
     175,   378,   175,   378,   378,   378,   175,   378,   378,   175,
     175,   378,   378,   378,   378,   378,   380,   438,   552,   175,
     378,   175,   175,   175,   175,   621,   378,   175,   175,   161,
     175,   624,   626,   378,   378,   743,   731,   237,   175,   370,
     434,   432,   140,   140,   378,   378,   175,   175,   175,    65,
      65,   253,   249,   175,   175,   470,   452,   175,   175,   175,
     175,   378,   625,   627,   175,   744,   175,   732,   175,   169,
     471,   433,   175,   175,   175,   378,   628,   628,   175,   378,
     175,   378,   175,   467,   175,   173,   629,   370,   169,   468,
     175,   175,   175,   136,   469,   175,   452
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:

/* Line 1455 of yacc.c  */
#line 316 "lef.y"
    { lefDumbMode = 1; lefNoNum = 1;;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 317 "lef.y"
    { 
         versionNum = convert_name2num((yyvsp[(3) - (4)].string));
/*
         versionNum = $3;         Save the version number for future use */
         if (lefrVersionStrCbk) {
            CALLBACK(lefrVersionStrCbk, lefrVersionStrCbkType, (yyvsp[(3) - (4)].string));
         } else {
            CALLBACK(lefrVersionCbk, lefrVersionCbkType, versionNum);
         }
         if (versionNum > 5.3 && versionNum < 5.4) {
            ignoreVersion = 1;
         }
         use5_3 = use5_4 = 0;
         lef_errors = 0;
         hasVer = 1;
      ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 336 "lef.y"
    {
        CALLBACK(lefrDividerCharCbk, lefrDividerCharCbkType, (yyvsp[(2) - (3)].string));
        hasDivChar = 1;
      ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 342 "lef.y"
    {
        CALLBACK(lefrBusBitCharsCbk, lefrBusBitCharsCbkType, (yyvsp[(2) - (3)].string)); 
        hasBusBit = 1;
      ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 350 "lef.y"
    { ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 353 "lef.y"
    {
        CALLBACK(lefrLibraryEndCbk, lefrLibraryEndCbkType, 0);
        // 11/16/2001 - Wanda da Rosa - pcr 408334
        // Return 1 if there are errors
        if (lef_errors)
           return 1;
        if (!hasVer)
           yywarning("VERSION is a required statement.");
        if (!hasNameCase)
           yywarning("NAMESCASESENSITIVE is a required statement.");
        if (!hasBusBit)
           yywarning("BUSBITCHARS is a required statement.");
        if (!hasDivChar)
           yywarning("DIVIDERCHAR is a required statement.");
        hasVer = 0;
        hasNameCase = 0;
        hasBusBit = 0;
        hasDivChar = 0;
      ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 388 "lef.y"
    {
	    lefNamesCaseSensitive = TRUE;
	    CALLBACK(lefrCaseSensitiveCbk, lefrCaseSensitiveCbkType,
			     lefNamesCaseSensitive);
            hasNameCase = 1;
	  ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 395 "lef.y"
    {
	    lefNamesCaseSensitive = FALSE;
	    CALLBACK(lefrCaseSensitiveCbk, lefrCaseSensitiveCbkType,
			     lefNamesCaseSensitive);
            hasNameCase = 1;
	  ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 403 "lef.y"
    { CALLBACK(lefrNoWireExtensionCbk, lefrNoWireExtensionCbkType, "ON"); ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 405 "lef.y"
    { CALLBACK(lefrNoWireExtensionCbk, lefrNoWireExtensionCbkType, "OFF"); ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 408 "lef.y"
    { CALLBACK(lefrManufacturingCbk, lefrManufacturingCbkType, (yyvsp[(2) - (3)].dval)); ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 411 "lef.y"
    {
    if (lefrUseMinSpacingCbk)
      lefrUseMinSpacing.lefiUseMinSpacing::set((yyvsp[(2) - (4)].string), (yyvsp[(3) - (4)].integer));
    CALLBACK(lefrUseMinSpacingCbk, lefrUseMinSpacingCbkType,
             &lefrUseMinSpacing);
  ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 419 "lef.y"
    { CALLBACK(lefrClearanceMeasureCbk, lefrClearanceMeasureCbkType, (yyvsp[(2) - (3)].string)); ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 422 "lef.y"
    {(yyval.string) = (char*)"MAXXY";;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 423 "lef.y"
    {(yyval.string) = (char*)"EUCLIDEAN";;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 426 "lef.y"
    {(yyval.string) = (char*)"OBS";;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 427 "lef.y"
    {(yyval.string) = (char*)"PIN";;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 430 "lef.y"
    {(yyval.integer) = 1;;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 431 "lef.y"
    {(yyval.integer) = 0;;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 434 "lef.y"
    { CALLBACK(lefrUnitsCbk, lefrUnitsCbkType, &lefrUnits); ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 437 "lef.y"
    { lefrUnits.lefiUnits::clear(); ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 444 "lef.y"
    { if (lefrUnitsCbk) lefrUnits.lefiUnits::setTime((yyvsp[(3) - (4)].dval)); ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 446 "lef.y"
    { if (lefrUnitsCbk) lefrUnits.lefiUnits::setCapacitance((yyvsp[(3) - (4)].dval)); ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 448 "lef.y"
    { if (lefrUnitsCbk) lefrUnits.lefiUnits::setResistance((yyvsp[(3) - (4)].dval)); ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 450 "lef.y"
    { if (lefrUnitsCbk) lefrUnits.lefiUnits::setPower((yyvsp[(3) - (4)].dval)); ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 452 "lef.y"
    { if (lefrUnitsCbk) lefrUnits.lefiUnits::setCurrent((yyvsp[(3) - (4)].dval)); ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 454 "lef.y"
    { if (lefrUnitsCbk) lefrUnits.lefiUnits::setVoltage((yyvsp[(3) - (4)].dval)); ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 455 "lef.y"
    { lefDumbMode = 1; ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 456 "lef.y"
    { if (lefrUnitsCbk) lefrUnits.lefiUnits::setDatabase((yyvsp[(3) - (5)].string), (yyvsp[(4) - (5)].dval)); ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 458 "lef.y"
    { if (lefrUnitsCbk) lefrUnits.lefiUnits::setFrequency((yyvsp[(3) - (4)].dval)); ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 461 "lef.y"
    { CALLBACK(lefrLayerCbk, lefrLayerCbkType, &lefrLayer); ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 463 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1; ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 464 "lef.y"
    { 
      if (lefrHasMaxVS) {   /* 5.5 */
        yyerror("A MAXVIASTACK has defined before the LAYER");
        CHKERR();
      }
      if (lefrLayerCbk)
        lefrLayer.lefiLayer::setName((yyvsp[(3) - (3)].string));
      useLenThr = 0;
      layerCut = 0;
      layerMastOver = 0;
      layerRout = 0;
      layerDir = 0;
      lefrHasLayer = 1;
      strcpy(layerName, (yyvsp[(3) - (3)].string));
      hasPitch = 0;
      hasWidth = 0;
      hasDirection = 0;
      hasParallel = 0;
      hasInfluence = 0;
    ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 485 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1; ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 486 "lef.y"
    { 
      if (strcmp(layerName, (yyvsp[(3) - (3)].string)) != 0) {
        yyerror("END LAYER name is different from LAYER");
        CHKERR(); 
      }
      if ((layerRout == 1) && (hasPitch == 0)) {
        yyerror("PITCH in LAYER with type ROUTING is required");
        CHKERR(); 
      }
      if ((layerRout == 1) && (hasWidth == 0)) {
        yyerror("WIDTH in LAYER with type ROUTING is required");
        CHKERR(); 
      }
      if ((layerRout == 1) && (hasDirection == 0)) {
        yyerror("DIRECTION in LAYER with type ROUTING is required");
        CHKERR(); 
      }
    ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 507 "lef.y"
    { ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 509 "lef.y"
    { ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 513 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::setType((yyvsp[(2) - (3)].string)); ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 515 "lef.y"
    { 
      if (lefrLayerCbk) lefrLayer.lefiLayer::setPitch((yyvsp[(2) - (3)].dval));
      hasPitch = 1;  
    ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 520 "lef.y"
    {
      if (lefrLayerCbk) lefrLayer.lefiLayer::setOffset((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 524 "lef.y"
    {
      if (lefrLayerCbk) lefrLayer.lefiLayer::setWidth((yyvsp[(2) - (3)].dval));
      hasWidth = 1;  
    ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 529 "lef.y"
    {
      if (lefrLayerCbk) lefrLayer.lefiLayer::setArea((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 533 "lef.y"
    {
      // 11/22/99 - Wanda da Rosa, PCR 283762
      //            Issue an error is this is defined in masterslice
      if (layerMastOver) {
         yyerror("SPACING can't be defined in LAYER with TYPE MASTERSLICE or OVERLAP. Parser stops executions");
        CHKERR();
      }
      // 5.5 either SPACING or SPACINGTABLE, not both for routing layer only
      if (layerRout) {
        if (lefrHasSpacingTbl) {
           yywarning("It is illegal to mix both SPACING rules & SPACINGTABLE rules for the same lef file in ROUTING layer");
        }
        if (lefrLayerCbk)
           lefrLayer.lefiLayer::setSpacingMin((yyvsp[(2) - (2)].dval));
        lefrHasSpacing = 1;
      } else { 
        if (lefrLayerCbk)
           lefrLayer.lefiLayer::setSpacingMin((yyvsp[(2) - (2)].dval));
      }
    ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 555 "lef.y"
    {
      layerDir = 1;
      if (!layerRout) {
        yyerror("DIRECTION can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setDirection((yyvsp[(2) - (3)].string));
      hasDirection = 1;  
    ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 565 "lef.y"
    {
      if (!layerRout) {
        yyerror("RESISTANCE can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setResistance((yyvsp[(3) - (4)].dval));
    ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 573 "lef.y"
    {
      if (!layerRout) {
        yyerror("RESISTANCE can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
        CHKERR();
      }
    ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 580 "lef.y"
    {
      if (!layerRout) {
        yyerror("CAPACITANCE can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setCapacitance((yyvsp[(3) - (4)].dval));
    ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 588 "lef.y"
    {
      if (!layerRout) {
        yyerror("CAPACITANCE can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
        CHKERR();
      }
    ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 595 "lef.y"
    {
      if (!layerRout) {
        yyerror("HEIGHT can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setHeight((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 603 "lef.y"
    {
      if (!layerRout) {
        yyerror("WIREEXTENSION can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setWireExtension((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 611 "lef.y"
    {
      if (!layerRout && (layerCut || layerMastOver)) {
        yyerror("THICKNESS can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setThickness((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 619 "lef.y"
    {
      if (!layerRout) {
        yyerror("SHRINKAGE can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setShrinkage((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 627 "lef.y"
    {
      if (!layerRout) {
        yyerror("CAPMULTIPLIER can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setCapMultiplier((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 635 "lef.y"
    {
      if (!layerRout) {
        yyerror("EDGECAPACITANCE can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setEdgeCap((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 654 "lef.y"
    { /* 5.3 syntax */
      use5_3 = 1;
      if (!layerRout && (layerCut || layerMastOver)) {
        yyerror("ANTENNALENGTHFACTOR can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
        CHKERR();
      } else if (versionNum >= 5.4) {
        if (use5_4) {
           yyerror("ANTENNALENGTHFACTOR is a 5.3 or earlier syntax. Your lef file has both old and new syntax, which is illegal.");
           CHKERR();
        }
      }

      if (lefrLayerCbk) lefrLayer.lefiLayer::setAntennaLength((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 669 "lef.y"
    {
      if (!layerRout) {
        yyerror("CURRENTDEN can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setCurrentDensity((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 677 "lef.y"
    { 
      if (!layerRout) {
        yyerror("CURRENTDEN can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
        CHKERR();
      }
    ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 684 "lef.y"
    {
      if (!layerRout) {
        yyerror("CURRENTDEN can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setCurrentPoint((yyvsp[(3) - (6)].dval), (yyvsp[(4) - (6)].dval));
    ;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 691 "lef.y"
    { lefDumbMode = 10000000; ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 692 "lef.y"
    {
      lefDumbMode = 0;
    ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 696 "lef.y"
    {
      if (layerMastOver) {
        yyerror("ACCURRENTDENSITY can't be defined in LAYER with TYPE MASTERSLICE or OVERLAP. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::addAccurrentDensity((yyvsp[(2) - (2)].string));
    ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 705 "lef.y"
    {
      if (layerMastOver) {
        yyerror("ACCURRENTDENSITY can't be defined in LAYER with TYPE MASTERSLICE or OVERLAP. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) {
           lefrLayer.lefiLayer::addAccurrentDensity((yyvsp[(2) - (4)].string));
           lefrLayer.lefiLayer::setAcOneEntry((yyvsp[(3) - (4)].dval));
      }
    ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 716 "lef.y"
    {
      if (layerMastOver) {
        yyerror("DCCURRENTDENSITY can't be defined in LAYER with TYPE MASTERSLICE or OVERLAP. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::addDccurrentDensity("AVERAGE");
    ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 725 "lef.y"
    {
      if (layerMastOver) {
        yyerror("DCCURRENTDENSITY can't be defined in LAYER with TYPE MASTERSLICE or OVERLAP. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) {
         lefrLayer.lefiLayer::addDccurrentDensity("AVERAGE");
         lefrLayer.lefiLayer::setDcOneEntry((yyvsp[(3) - (4)].dval));
      }
    ;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 737 "lef.y"
    { /* 5.4 syntax */
      use5_4 = 1;
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("ANTENNAAREARATIO is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      } else if (use5_3) {
         yyerror("ANTENNAAREARATIO is a 5.4 syntax.  Your lef file has both old and new syntax, which is illegal."); 
         CHKERR();
      }
      if (!layerRout && !layerCut && layerMastOver) {
         yyerror("ANTENNAAREARATIO can only be defined in LAYER with TYPE ROUTING or CUT. Parser stops executions");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setAntennaAreaRatio((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 755 "lef.y"
    { /* 5.4 syntax */
      use5_4 = 1;
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("ANTENNADIFFAREARATIO is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      } else if (use5_3) {
         yyerror("ANTENNADIFFAREARATIO is a 5.4 syntax.  Your lef file has both old and new syntax, which is illegal."); 
         CHKERR();
      }
      if (!layerRout && !layerCut && layerMastOver) {
        yyerror("ANTENNADIFFAREARATIO can only be defined in LAYER with TYPE ROUTING or CUT. Parser stops executions");
        CHKERR();
      }
      antennaType = lefiAntennaDAR; 
    ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 774 "lef.y"
    { /* 5.4 syntax */
      use5_4 = 1;
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("ANTENNACUMAREARATIO is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      } else if (use5_3) {
         yyerror("ANTENNACUMAREARATIO is a 5.4 syntax.  Your lef file has both old and new syntax, which is illegal."); 
         CHKERR();
      }
      if (!layerRout && !layerCut && layerMastOver) {
        yyerror("ANTENNACUMAREARATIO can only be defined in LAYER with TYPE ROUTING or CUT. Parser stops executions");
        CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setAntennaCumAreaRatio((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 792 "lef.y"
    {  /* 5.4 syntax */
      use5_4 = 1;
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("ANTENNACUMDIFFAREARATIO is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      } else if (use5_3) {
         yyerror("ANTENNACUMDIFFAREARATIO is a 5.4 syntax.  Your lef file has both old and new syntax, which is illegal."); 
         CHKERR();
      }
      if (!layerRout && !layerCut && layerMastOver) {
         yyerror("ANTENNACUMDIFFAREARATIO can only be defined in LAYER with TYPE ROUTING or CUT. Parser stops executions");
         CHKERR();
      }
      antennaType = lefiAntennaCDAR;
    ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 811 "lef.y"
    { /* both 5.3  & 5.4 syntax */
      if (!layerRout && !layerCut && layerMastOver) {
         yyerror("ANTENNAAREAFACTOR can only be defined in LAYER with TYPE ROUTING or CUT. Parser stops executions");
         CHKERR();
      }
      /* this does not need to check, since syntax is in both 5.3 & 5.4 */
      if (lefrLayerCbk) lefrLayer.lefiLayer::setAntennaAreaFactor((yyvsp[(2) - (2)].dval));
      antennaType = lefiAntennaAF;
    ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 822 "lef.y"
    { /* 5.4 syntax */
      use5_4 = 1;
      if (!layerRout && (layerCut || layerMastOver)) {
         yyerror("ANTENNASIDEAREARATIO can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
         CHKERR();
      }
      if (ignoreVersion) {
        /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("ANTENNASIDEAREARATIO is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      } else if (use5_3) {
         yyerror("ANTENNASIDEAREARATIO is a 5.4 syntax.  Your lef file has both old and new syntax, which is illegal."); 
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setAntennaSideAreaRatio((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 840 "lef.y"
    {  /* 5.4 syntax */
      use5_4 = 1;
      if (!layerRout && (layerCut || layerMastOver)) {
         yyerror("ANTENNADIFFSIDEAREARATIO can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
         CHKERR();
      }
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("ANTENNADIFFSIDEAREARATIO is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      } else if (use5_3) {
         yyerror("ANTENNADIFFSIDEAREARATIO is a 5.4 syntax.  Your lef file has both old and new syntax, which is illegal."); 
         CHKERR();
      }
      antennaType = lefiAntennaDSAR;
    ;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 859 "lef.y"
    { /* 5.4 syntax */
      use5_4 = 1;
      if (!layerRout && (layerCut || layerMastOver)) {
         yyerror("ANTENNACUMSIDEAREARATIO can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
         CHKERR();
      }
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("ANTENNACUMSIDEAREARATIO is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      } else if (use5_3) {
         yyerror("ANTENNACUMSIDEAREARATIO is a 5.4 syntax.  Your lef file has both old and new syntax, which is illegal."); 
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setAntennaCumSideAreaRatio((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 877 "lef.y"
    {  /* 5.4 syntax */
      use5_4 = 1;
      if (!layerRout && (layerCut || layerMastOver)) {
         yyerror("ANTENNACUMDIFFSIDEAREARATIO can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
         CHKERR();
      }
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("ANTENNACUMDIFFSIDEAREARATIO is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      } else if (use5_3) {
         yyerror("ANTENNACUMDIFFSIDEAREARATIO is a 5.4 syntax.  Your lef file has both old and new syntax, which is illegal."); 
         CHKERR();
      }
      antennaType = lefiAntennaCDSAR;
    ;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 896 "lef.y"
    { /* 5.4 syntax */
      use5_4 = 1;
      if (!layerRout && (layerCut || layerMastOver)) {
         yyerror("ANTENNASIDEAREAFACTOR can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
         CHKERR();
      }
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("ANTENNASIDEAREAFACTOR is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      } else if (use5_3) {
         yyerror("ANTENNASIDEAREAFACTOR is a 5.4 syntax.  Your lef file has both old and new syntax, which is illegal."); 
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setAntennaSideAreaFactor((yyvsp[(2) - (2)].dval));
      antennaType = lefiAntennaSAF;
    ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 916 "lef.y"
    { /* 5.4 syntax */
      use5_4 = 1;
      if (!layerRout && !layerCut && layerMastOver) {
         yyerror("ANTENNAMODEL can only be defined in LAYER with TYPE ROUTING. Parser stops executions");
         CHKERR();
      }
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.5) {
         yyerror("ANTENNAMODEL is a 5.5 syntax. Your lef file is not defined as 5.5");
         CHKERR();
      } else if (use5_3) {
         yyerror("ANTENNAMODEL is a 5.5 syntax.  Your lef file has both old and new syntax, which is illegal."); 
         CHKERR();
      }
      antennaType = lefiAntennaO;
    ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 935 "lef.y"
    { /* 5.4 syntax */
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("SLOTWIREWIDTH is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setSlotWireWidth((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 945 "lef.y"
    { /* 5.4 syntax */
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("SLOTWIRELENGTH is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setSlotWireLength((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 955 "lef.y"
    { /* 5.4 syntax */
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("SLOTWIDTH is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setSlotWidth((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 965 "lef.y"
    { /* 5.4 syntax */
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("SLOTLENGTH is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setSlotLength((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 975 "lef.y"
    { /* 5.4 syntax */
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("MAXADJACENTSLOTSPACING is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setMaxAdjacentSlotSpacing((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 985 "lef.y"
    { /* 5.4 syntax */
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("MAXCOAXIALSLOTSPACING is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setMaxCoaxialSlotSpacing((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 995 "lef.y"
    { /* 5.4 syntax */
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("MAXEDGESLOTSPACING is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setMaxEdgeSlotSpacing((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1005 "lef.y"
    { /* 5.4 syntax */
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("SPLITWIREWIDTH is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setSplitWireWidth((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1015 "lef.y"
    { /* 5.4 syntax, pcr 394389 */
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("MINIMUMDENSITY is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setMinimumDensity((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1025 "lef.y"
    { /* 5.4 syntax, pcr 394389 */
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("MAXIMUMDENSITY is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setMaximumDensity((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1035 "lef.y"
    { /* 5.4 syntax, pcr 394389 */
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("DENSITYCHECKWINDOW is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setDensityCheckWindow((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].dval));
    ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1045 "lef.y"
    { /* 5.4 syntax, pcr 394389 */
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("DENSITYCHECKSTEP is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setDensityCheckStep((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1055 "lef.y"
    { /* 5.4 syntax, pcr 394389 */
      if (ignoreVersion) {
         /* do nothing */
      } else if (versionNum < 5.4) {
         yyerror("FILLACTIVESPACING is a 5.4 syntax. Your lef file is not defined as 5.4");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setFillActiveSpacing((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1065 "lef.y"
    {
      // 5.5 MAXWIDTH, is for routing layer only
      if (!layerRout) {
        yyerror("MAXWIDTH can only be defined in LAYER with TYPE ROUTING.  Parser stops executions");
        CHKERR();
      }
      if (versionNum < 5.5) {
         yyerror("MAXWIDTH is a 5.5 syntax. Your lef file is not defined as 5.5");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setMaxwidth((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1078 "lef.y"
    {
      // 5.5 MINWIDTH, is for routing layer only
      if (!layerRout) {
        yyerror("MINWIDTH can only be defined in LAYER with TYPE ROUTING.  Parser stops executions");
        CHKERR();
      }
      if (versionNum < 5.5) {
         yyerror("MINWIDTH is a 5.5 syntax. Your lef file is not defined as 5.5");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setMinwidth((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1091 "lef.y"
    {
      if (versionNum < 5.5) {
         yyerror("MINENCLOSEDAREA is a 5.5 syntax. Your lef file is not defined as 5.5");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::addMinenclosedarea((yyvsp[(2) - (2)].dval));
    ;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1100 "lef.y"
    { /* pcr 409334 */
      if (lefrLayerCbk) lefrLayer.lefiLayer::addMinimumcut((yyvsp[(2) - (4)].dval), (yyvsp[(4) - (4)].dval)); 
    ;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1104 "lef.y"
    {
      if (lefrLayerCbk) lefrLayer.lefiLayer::addMinimumcutConnect((yyvsp[(6) - (6)].string));
    ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1109 "lef.y"
    {
      if (lefrLayerCbk) lefrLayer.lefiLayer::setMinstep((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1113 "lef.y"
    {
      if (versionNum < 5.5) {
         yyerror("PROTRUSION RULE is a 5.5 syntax. Your lef file is not defined as 5.5");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::setProtrusion((yyvsp[(2) - (7)].dval), (yyvsp[(4) - (7)].dval), (yyvsp[(6) - (7)].dval));
    ;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1121 "lef.y"
    {
      if (versionNum < 5.5) {
         yyerror("SPACINGTABLE is a 5.5 syntax. Your lef file is not defined as 5.5");
         CHKERR();
      }
      // 5.5 either SPACING or SPACINGTABLE in a layer, not both
      if (lefrHasSpacing && layerRout) {
         yywarning("It is illegal to mix both SPACING rules & SPACINGTABLE rules for the same lef file in ROUTING layer");
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::addSpacingTable();
      lefrHasSpacingTbl = 1;
    ;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1137 "lef.y"
    { 
      if (hasInfluence) {  // 5.5 - INFLUENCE table must follow a PARALLEL
          yyerror("The INFLUENCE table must follow a PARALLELRUNLENGTH table on the same layer");
          CHKERR();
      }
      if (hasParallel) { // 5.5 - Only one PARALLEL table is allowed per layer
          yyerror("Only one PARALLELRUNLENGTH table is allowed per layer");
          CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::addNumber((yyvsp[(2) - (2)].dval));
      hasParallel = 1;
    ;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1150 "lef.y"
    {
      spParallelLength = lefrLayer.lefiLayer::getNumber();
      if (lefrLayerCbk) lefrLayer.lefiLayer::addSpParallelLength();
    ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1155 "lef.y"
    { 
      if (lefrLayerCbk) {
         lefrLayer.lefiLayer::addSpParallelWidth((yyvsp[(7) - (7)].dval));
      }
    ;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1161 "lef.y"
    { 
      if (lefrLayer.lefiLayer::getNumber() != spParallelLength) {
         yyerror("The number of length in PARALLELRUNLENGTH is not the same as the number of spacing in WIDTH");
         CHKERR();
      }
      if (lefrLayerCbk) lefrLayer.lefiLayer::addSpParallelWidthSpacing();
    ;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1170 "lef.y"
    {
      if (hasInfluence) {  // 5.5 - INFLUENCE table must follow a PARALLEL
          yyerror("Only one INFLUENCE table is allowed per layer");
          CHKERR();
      }
      if (!hasParallel) {  // 5.5 - INFLUENCE must follow a PARALLEL
          yyerror("The INFLUENCE table must follow a PARALLELRUNLENGTH table on the same layer");
          CHKERR();
      }
      if (lefrLayerCbk) {
         lefrLayer.lefiLayer::setInfluence();
         lefrLayer.lefiLayer::addSpInfluence((yyvsp[(3) - (7)].dval), (yyvsp[(5) - (7)].dval), (yyvsp[(7) - (7)].dval));
      }
    ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1188 "lef.y"
    { if (lefrLayerCbk)
          lefrLayer.lefiLayer::setAntennaValue(antennaType, (yyvsp[(1) - (1)].dval)); ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1191 "lef.y"
    { if (lefrLayerCbk) { /* require min 2 points, set the 1st 2 */
          lefrAntennaPWLPtr = (lefiAntennaPWL*)lefMalloc(sizeof(lefiAntennaPWL));
          lefrAntennaPWLPtr->lefiAntennaPWL::Init();
          lefrAntennaPWLPtr->lefiAntennaPWL::addAntennaPWL((yyvsp[(3) - (4)].pt).x, (yyvsp[(3) - (4)].pt).y);
          lefrAntennaPWLPtr->lefiAntennaPWL::addAntennaPWL((yyvsp[(4) - (4)].pt).x, (yyvsp[(4) - (4)].pt).y);
        }
      ;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1199 "lef.y"
    { if (lefrLayerCbk)
          lefrLayer.lefiLayer::setAntennaPWL(antennaType, lefrAntennaPWLPtr);
      ;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1209 "lef.y"
    { if (lefrLayerCbk)
      lefrAntennaPWLPtr->lefiAntennaPWL::addAntennaPWL((yyvsp[(1) - (1)].pt).x, (yyvsp[(1) - (1)].pt).y);
  ;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1215 "lef.y"
    { 
        use5_4 = 1;
        if (ignoreVersion) {
           /* do nothing */
        }
        else if ((antennaType == lefiAntennaAF) && (versionNum <= 5.3)) {
           yyerror("ANTENNAAREAFACTOR with DIFFUSEONLY is a 5.4 syntax. Your lef file is not defined as 5.4");
           CHKERR();
        } else if (use5_3) {
           yyerror("ANTENNAAREAFACTOR with DIFFUSEONLY is a 5.4 syntax. Your lef file has both old and new syntax, which is illegal.");
           CHKERR();
        }
        if (lefrLayerCbk)
          lefrLayer.lefiLayer::setAntennaDUO(antennaType);
      ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1232 "lef.y"
    {(yyval.string) = (char*)"PEAK";;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1233 "lef.y"
    {(yyval.string) = (char*)"AVERAGE";;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1234 "lef.y"
    {(yyval.string) = (char*)"RMS";;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1241 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addNumber((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1243 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addAcFrequency(); ;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1245 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addNumber((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1247 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addAcCutarea(); ;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1249 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addNumber((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1251 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addAcTableEntry(); ;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1253 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addNumber((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1255 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addAcWidth(); ;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1262 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addNumber((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1264 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addDcCutarea(); ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1266 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addNumber((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1268 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addDcTableEntry(); ;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1270 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addNumber((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1272 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addDcWidth(); ;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1276 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addNumber((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1285 "lef.y"
    {
      if (lefrLayerCbk) {
        char propTp;
        propTp = lefrLayerProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
        lefrLayer.lefiLayer::addProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
      }
    ;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1293 "lef.y"
    {
      if (lefrLayerCbk) {
        char propTp;
        propTp = lefrLayerProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
        lefrLayer.lefiLayer::addProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
      }
    ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1301 "lef.y"
    {
      char temp[32];
      sprintf(temp, "%g", (yyvsp[(2) - (2)].dval));
      if (lefrLayerCbk) {
        char propTp;
        propTp = lefrLayerProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
        lefrLayer.lefiLayer::addNumProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].dval), temp, propTp);
      }
    ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1313 "lef.y"
    { ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1315 "lef.y"
    { ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1318 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::setCurrentPoint((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].dval)); ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1326 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::setCapacitancePoint((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].dval)); ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1331 "lef.y"
    { ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1334 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::setResistancePoint((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].dval)); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1337 "lef.y"
    {(yyval.string) = (char*)"ROUTING"; layerRout = 1;;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1338 "lef.y"
    {(yyval.string) = (char*)"CUT"; layerCut = 1;;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1339 "lef.y"
    {(yyval.string) = (char*)"OVERLAP"; layerMastOver = 1;;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1340 "lef.y"
    {(yyval.string) = (char*)"MASTERSLICE"; layerMastOver = 1;;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1341 "lef.y"
    {(yyval.string) = (char*)"VIRTUAL";;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1342 "lef.y"
    {(yyval.string) = (char*)"IMPLANT";;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1345 "lef.y"
    {(yyval.string) = (char*)"HORIZONTAL";;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1346 "lef.y"
    {(yyval.string) = (char*)"VERTICAL";;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1348 "lef.y"
    {(yyval.string) = (char*)"";;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1350 "lef.y"
    {
      if (versionNum < 5.5) {
         yyerror("FROMABOVE is a 5.5 syntax. Your lef file is not defined as 5.5");
         CHKERR();
      }
      (yyval.string) = (char*)"FROMABOVE";

    ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1359 "lef.y"
    {
      if (versionNum < 5.5) {
         yyerror("FROMBELOW is a 5.5 syntax. Your lef file is not defined as 5.5");
         CHKERR();
      }
      (yyval.string) = (char*)"FROMBELOW";
    ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1369 "lef.y"
    {   
      if (versionNum < 5.5) {
         yyerror("LENGTH WITHIN is a 5.5 syntax. Your lef file is not defined as 5.5");
         CHKERR();
      }
      if (lefrLayerCbk)
         lefrLayer.lefiLayer::addMinimumcutLengDis((yyvsp[(2) - (4)].dval), (yyvsp[(4) - (4)].dval));
    ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1380 "lef.y"
    {
    if (lefrLayerCbk)
       lefrLayer.lefiLayer::addMinenclosedareaWidth((yyvsp[(2) - (2)].dval));
    ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1387 "lef.y"
    {
    if (lefrLayerCbk)
       lefrLayer.lefiLayer::addAntennaModel(1);
    ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1392 "lef.y"
    {
    if (lefrLayerCbk)
       lefrLayer.lefiLayer::addAntennaModel(2);
    ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1397 "lef.y"
    {
    if (lefrLayerCbk)
       lefrLayer.lefiLayer::addAntennaModel(3);
    ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1402 "lef.y"
    {
    if (lefrLayerCbk)
       lefrLayer.lefiLayer::addAntennaModel(4);
    ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1408 "lef.y"
    { ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1410 "lef.y"
    { ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1413 "lef.y"
    { 
      if (lefrLayerCbk) {
         lefrLayer.lefiLayer::addSpParallelWidth((yyvsp[(2) - (2)].dval));
      }
    ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1419 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addSpParallelWidthSpacing(); ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1422 "lef.y"
    { ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1424 "lef.y"
    { ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1427 "lef.y"
    { if (lefrLayerCbk) lefrLayer.lefiLayer::addSpInfluence((yyvsp[(2) - (6)].dval), (yyvsp[(4) - (6)].dval), (yyvsp[(6) - (6)].dval)); ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1430 "lef.y"
    {
      if (!lefrHasLayer) {  /* 5.5 */
        yyerror("MAXVIASTACK has to define after the LAYER");
        CHKERR();
      } else if (lefrHasMaxVS) {
        yyerror("Only one MAXVIASTACK is allowed in a lef file");
        CHKERR();
      } else {
        if (lefrMaxStackViaCbk)
           lefrMaxStackVia.lefiMaxStackVia::setMaxStackVia((yyvsp[(2) - (3)].dval));
        CALLBACK(lefrMaxStackViaCbk, lefrMaxStackViaCbkType, &lefrMaxStackVia);
      }
      if (versionNum < 5.5) {
         yyerror("MAXVIASTACK is a 5.5 syntax. Your lef file is not defined as 5.5");
         CHKERR();
      }
      lefrHasMaxVS = 1;
    ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1448 "lef.y"
    {lefDumbMode = 2; lefNoNum= 2;;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1450 "lef.y"
    {
      if (!lefrHasLayer) {  /* 5.5 */
        yyerror("MAXVIASTACK has to define after the LAYER");
        CHKERR();
      } else if (lefrHasMaxVS) {
        yyerror("Only one MAXVIASTACK is allowed in a lef file");
        CHKERR();
      } else {
        if (lefrMaxStackViaCbk) {
           lefrMaxStackVia.lefiMaxStackVia::setMaxStackVia((yyvsp[(2) - (7)].dval));
           lefrMaxStackVia.lefiMaxStackVia::setMaxStackViaRange((yyvsp[(5) - (7)].string), (yyvsp[(6) - (7)].string));
        }
        CALLBACK(lefrMaxStackViaCbk, lefrMaxStackViaCbkType, &lefrMaxStackVia);
      }
      lefrHasMaxVS = 1;
    ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1468 "lef.y"
    { CALLBACK(lefrViaCbk, lefrViaCbkType, &lefrVia); ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1472 "lef.y"
    { lefDumbMode = 1; lefNoNum = 1;;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1475 "lef.y"
    {
      /* 0 is nodefault */
      if (lefrViaCbk) lefrVia.lefiVia::setName((yyvsp[(2) - (2)].string), 0);
      viaLayer = 0;
      numVia++;
      strcpy(viaName, (yyvsp[(2) - (2)].string));
    ;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1483 "lef.y"
    {
      /* 1 is default */
      if (lefrViaCbk) lefrVia.lefiVia::setName((yyvsp[(2) - (3)].string), 1);
      viaLayer = 0;
      strcpy(viaName, (yyvsp[(2) - (3)].string));
    ;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1497 "lef.y"
    { ;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1499 "lef.y"
    { ;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1501 "lef.y"
    { if (lefrViaCbk) lefrVia.lefiVia::setResistance((yyvsp[(2) - (3)].dval)); ;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1502 "lef.y"
    { lefDumbMode = 1000000; ;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 1503 "lef.y"
    { lefDumbMode = 0; ;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 1505 "lef.y"
    { if (lefrViaCbk) lefrVia.lefiVia::setTopOfStack(); ;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1514 "lef.y"
    { 
      char temp[32];
      sprintf(temp, "%g", (yyvsp[(2) - (2)].dval));
      if (lefrViaCbk) {
         char propTp;
         propTp = lefrViaProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
         lefrVia.lefiVia::addNumProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].dval), temp, propTp);
      }
    ;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1524 "lef.y"
    {
      if (lefrViaCbk) {
         char propTp;
         propTp = lefrViaProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
         lefrVia.lefiVia::addProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
      }
    ;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1532 "lef.y"
    {
      if (lefrViaCbk) {
         char propTp;
         propTp = lefrViaProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
         lefrVia.lefiVia::addProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
      }
    ;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1542 "lef.y"
    { if (lefrViaCbk) lefrVia.lefiVia::setForeign((yyvsp[(1) - (2)].string), 0, 0.0, 0.0, -1); ;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1544 "lef.y"
    { if (lefrViaCbk) lefrVia.lefiVia::setForeign((yyvsp[(1) - (3)].string), 1, (yyvsp[(2) - (3)].pt).x, (yyvsp[(2) - (3)].pt).y, -1); ;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1546 "lef.y"
    { if (lefrViaCbk) lefrVia.lefiVia::setForeign((yyvsp[(1) - (4)].string), 1, (yyvsp[(2) - (4)].pt).x, (yyvsp[(2) - (4)].pt).y, (yyvsp[(3) - (4)].integer)); ;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1548 "lef.y"
    { if (lefrViaCbk) lefrVia.lefiVia::setForeign((yyvsp[(1) - (3)].string), 0, 0.0, 0.0, (yyvsp[(2) - (3)].integer)); ;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1550 "lef.y"
    {lefDumbMode = 1; lefNoNum= 1;;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1551 "lef.y"
    { (yyval.string) = (yyvsp[(3) - (3)].string); ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1554 "lef.y"
    {(yyval.integer) = 0;;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1555 "lef.y"
    {(yyval.integer) = 1;;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1556 "lef.y"
    {(yyval.integer) = 2;;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1557 "lef.y"
    {(yyval.integer) = 3;;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1558 "lef.y"
    {(yyval.integer) = 4;;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1559 "lef.y"
    {(yyval.integer) = 5;;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1560 "lef.y"
    {(yyval.integer) = 6;;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 1561 "lef.y"
    {(yyval.integer) = 7;;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 1562 "lef.y"
    {(yyval.integer) = 0;;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1563 "lef.y"
    {(yyval.integer) = 1;;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1564 "lef.y"
    {(yyval.integer) = 2;;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1565 "lef.y"
    {(yyval.integer) = 3;;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1566 "lef.y"
    {(yyval.integer) = 4;;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1567 "lef.y"
    {(yyval.integer) = 5;;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1568 "lef.y"
    {(yyval.integer) = 6;;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1569 "lef.y"
    {(yyval.integer) = 7;;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1572 "lef.y"
    { ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1574 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1; ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1575 "lef.y"
    {
      if (lefrViaCbk) lefrVia.lefiVia::addLayer((yyvsp[(3) - (4)].string));
      viaLayer++;
    ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1586 "lef.y"
    { if (lefrViaCbk)
      lefrVia.lefiVia::addRectToLayer((yyvsp[(2) - (4)].pt).x, (yyvsp[(2) - (4)].pt).y, (yyvsp[(3) - (4)].pt).x, (yyvsp[(3) - (4)].pt).y); ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1589 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1;;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1590 "lef.y"
    { 
      // 10/17/2001 - Wanda da Rosa, PCR 404149
      //              Error if no layer in via
      if (!viaLayer) {
         yyerror("VIA requires at least one layer");
         CHKERR();
      }
      if (strcmp(viaName, (yyvsp[(3) - (3)].string)) != 0) {
        yyerror("END VIA name is different from VIA");
        CHKERR();
      }
    ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1603 "lef.y"
    { lefDumbMode = 1; lefNoNum = 1;;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 1604 "lef.y"
    { 
      if (lefrViaRuleCbk) lefrViaRule.lefiViaRule::setName((yyvsp[(3) - (3)].string));
      viaRuleLayer = 0;
      strcpy(viaRuleName, (yyvsp[(3) - (3)].string));
      isGenerate = 0;
    ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1613 "lef.y"
    {
      // 10/17/2001 - Wanda da Rosa, PCR 404163
      //              Error if layer number is not equal 2.
      // 11/14/2001 - Wanda da Rosa,
      //              Commented out for pcr 411781
      //if (viaRuleLayer != 2) {
         //yyerror("VIARULE requires two layers");
         //CHKERR();
      //}
      if (viaRuleLayer = 0 || viaRuleLayer > 2) {
         yyerror("VIARULE requires two layers");
         CHKERR();
      }
      CALLBACK(lefrViaRuleCbk, lefrViaRuleCbkType, &lefrViaRule);
    ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 1631 "lef.y"
    {
      isGenerate = 1;
    ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 1635 "lef.y"
    {
      // 10/17/2001 - Wanda da Rosa, PCR 404181
      //              Error if layer number is not equal 3.
      // 11/14/2001 - Wanda da Rosa,
      //              Commented out for pcr 411781
      //if (viaRuleLayer != 3) {
         //yyerror("VIARULE requires three layers");
         //CHKERR();
      //}
      if (viaRuleLayer = 0) {
         yyerror("VIARULE requires three layers");
         CHKERR();
      }
      if (lefrViaRuleCbk) lefrViaRule.lefiViaRule::setGenerate();
      CALLBACK(lefrViaRuleCbk, lefrViaRuleCbkType, &lefrViaRule);
    ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1667 "lef.y"
    { lefDumbMode = 10000000; ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1668 "lef.y"
    { lefDumbMode = 0; ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1677 "lef.y"
    {
      if (lefrViaRuleCbk) {
         char propTp;
         propTp = lefrViaRuleProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
         lefrViaRule.lefiViaRule::addProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
      }
    ;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1685 "lef.y"
    {
      if (lefrViaRuleCbk) {
         char propTp;
         propTp = lefrViaRuleProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
         lefrViaRule.lefiViaRule::addProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
      }
    ;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1693 "lef.y"
    {
      char temp[32];
      sprintf(temp, "%g", (yyvsp[(2) - (2)].dval));
      if (lefrViaRuleCbk) {
         char propTp;
         propTp = lefrViaRuleProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
         lefrViaRule.lefiViaRule::addNumProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].dval), temp, propTp);
      }
    ;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1704 "lef.y"
    {
      // 10/18/2001 - Wanda da Rosa PCR 404181
      //              Make sure the 1st 2 layers in viarule has direction
      if (viaRuleLayer < 2 && !viaRuleHasDir && !viaRuleHasEnc && isGenerate) {
         yyerror("VIARULE requires the DIRECTION construct in LAYER");
         CHKERR(); 
      }
      viaRuleLayer++;
    ;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1721 "lef.y"
    { if (lefrViaRuleCbk) lefrViaRule.lefiViaRule::addViaName((yyvsp[(2) - (3)].string)); ;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1723 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1; ;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1724 "lef.y"
    { if (lefrViaRuleCbk) lefrViaRule.lefiViaRule::setLayer((yyvsp[(3) - (4)].string));
      viaRuleHasDir = 0;
      viaRuleHasEnc = 0;
    ;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1736 "lef.y"
    {
      if (viaRuleHasEnc) {
        yyerror("Either DIRECTION or ENCLOSURE can be specified in a layer");
        CHKERR();
      } else {
        if (lefrViaRuleCbk) lefrViaRule.lefiViaRule::setHorizontal();
      }
      viaRuleHasDir = 1;
    ;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1746 "lef.y"
    { 
      if (viaRuleHasEnc) {
        yyerror("Either DIRECTION or ENCLOSURE can be specified in a layer");
        CHKERR();
      } else {
        if (lefrViaRuleCbk) lefrViaRule.lefiViaRule::setVertical();
      }
      viaRuleHasDir = 1;
    ;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1756 "lef.y"
    {
      if (versionNum < 5.5) {
         yyerror("ENCLOSURE is a 5.5 syntax. Your lef file is not defined as 5.5");
         CHKERR();
      }
      if (viaRuleHasDir) {
         yyerror("Either DIRECTION or ENCLOSURE can be specified in a layer");
         CHKERR();
      } else {
         if (lefrViaRuleCbk) lefrViaRule.lefiViaRule::setEnclosure((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].dval));
      }
      viaRuleHasEnc = 1;
    ;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1770 "lef.y"
    { if (lefrViaRuleCbk) lefrViaRule.lefiViaRule::setWidth((yyvsp[(2) - (5)].dval),(yyvsp[(4) - (5)].dval)); ;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1772 "lef.y"
    { if (lefrViaRuleCbk)
	lefrViaRule.lefiViaRule::setRect((yyvsp[(2) - (4)].pt).x, (yyvsp[(2) - (4)].pt).y, (yyvsp[(3) - (4)].pt).x, (yyvsp[(3) - (4)].pt).y); ;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1775 "lef.y"
    { if (lefrViaRuleCbk) lefrViaRule.lefiViaRule::setSpacing((yyvsp[(2) - (5)].dval),(yyvsp[(4) - (5)].dval)); ;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1777 "lef.y"
    { if (lefrViaRuleCbk) lefrViaRule.lefiViaRule::setResistance((yyvsp[(2) - (3)].dval)); ;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1779 "lef.y"
    {
      if (!viaRuleHasDir) {
         yyerror("Either OVERHANG can only be defined with DIRECTION");
         CHKERR();
      }
      if (lefrViaRuleCbk) lefrViaRule.lefiViaRule::setOverhang((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1787 "lef.y"
    {
      if (!viaRuleHasDir) {
         yyerror("Either METALOVERHANG can only be defined with DIRECTION");
         CHKERR();
      }
      if (lefrViaRuleCbk) lefrViaRule.lefiViaRule::setMetalOverhang((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1795 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1;;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1796 "lef.y"
    {
      if (strcmp(viaRuleName, (yyvsp[(3) - (3)].string)) != 0) {
        yyerror("END VIARULE name is different from VIARULE");
        CHKERR();
      }
    ;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1804 "lef.y"
    { ;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1807 "lef.y"
    { if (lefrSpacingBeginCbk)
	CALLBACK(lefrSpacingBeginCbk, lefrSpacingBeginCbkType, 0); ;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1811 "lef.y"
    { if (lefrSpacingEndCbk)
            CALLBACK(lefrSpacingEndCbk, lefrSpacingEndCbkType, 0); ;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1820 "lef.y"
    {
      if (lefrSpacingCbk) {
	lefrSpacing.lefiSpacing::set((yyvsp[(2) - (5)].string), (yyvsp[(3) - (5)].string), (yyvsp[(4) - (5)].dval), 0);
	CALLBACK(lefrSpacingCbk, lefrSpacingCbkType, &lefrSpacing);
      }
    ;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 1827 "lef.y"
    {
      if (lefrSpacingCbk) {
	lefrSpacing.lefiSpacing::set((yyvsp[(2) - (6)].string), (yyvsp[(3) - (6)].string), (yyvsp[(4) - (6)].dval), 1);
	CALLBACK(lefrSpacingCbk, lefrSpacingCbkType, &lefrSpacing);
      }
    ;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1835 "lef.y"
    { lefDumbMode = 2; lefNoNum = 2; ;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1838 "lef.y"
    { ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1841 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrIRDropBeginCbk) 
	  CALLBACK(lefrIRDropBeginCbk, lefrIRDropBeginCbkType, 0);
      } else
        yywarning("IRDROP is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1850 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrIRDropEndCbk)
	  CALLBACK(lefrIRDropEndCbk, lefrIRDropEndCbkType, 0);
      }
    ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1864 "lef.y"
    { 
      if (versionNum < 5.4) {
        CALLBACK(lefrIRDropCbk, lefrIRDropCbkType, &lefrIRDrop);
      }
    ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1876 "lef.y"
    { if (lefrIRDropCbk) lefrIRDrop.lefiIRDrop::setValues((yyvsp[(1) - (2)].dval), (yyvsp[(2) - (2)].dval)); ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1879 "lef.y"
    { if (lefrIRDropCbk) lefrIRDrop.lefiIRDrop::setTableName((yyvsp[(2) - (2)].string)); ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 1882 "lef.y"
    {
    if (versionNum < 5.4) {
       if (lefrMinFeatureCbk)
         lefrMinFeature.lefiMinFeature::set((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].dval));
       CALLBACK(lefrMinFeatureCbk, lefrMinFeatureCbkType, &lefrMinFeature);
    } else
       yywarning("MINFEATURE is obsolete in 5.4. It will be ignored.");
  ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 1892 "lef.y"
    {
    if (versionNum < 5.4) {
       CALLBACK(lefrDielectricCbk, lefrDielectricCbkType, (yyvsp[(2) - (3)].dval));
    } else
       yywarning("DIELECTRIC is obsolete in 5.4. It will be ignored.");
  ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1900 "lef.y"
    {
    if (lefrNonDefaultCbk) lefrNonDefault.lefiNonDefault::setName((yyvsp[(2) - (2)].string));
    ndLayer = 0;
    numVia = 0;
    strcpy(nonDefaultRuleName, (yyvsp[(2) - (2)].string));
  ;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1907 "lef.y"
    {
    // 10/18/2001 - Wanda da Rosa, PCR 404189
    //              At least 1 layer is required
    if (!ndLayer) {
       yyerror("NONDEFAULTRULE requires at least one layer");
       CHKERR();
    }
    if (!numVia) {
       yyerror("NONDEFAULTRULE requires at least one via");
       CHKERR();
    }
    if (lefrNonDefaultCbk) lefrNonDefault.lefiNonDefault::end();
    CALLBACK(lefrNonDefaultCbk, lefrNonDefaultCbkType, &lefrNonDefault);
  ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1924 "lef.y"
    {
      if (strcmp(nonDefaultRuleName, (yyvsp[(2) - (2)].string)) != 0) {
        yyerror("END NONDEFAULTRULE name is different from NONDEFAULTRULE");
        CHKERR();
      }
    ;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1944 "lef.y"
    { lefDumbMode = 10000000; ;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 1945 "lef.y"
    { lefDumbMode = 0; ;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 1954 "lef.y"
    {
      if (lefrNonDefaultCbk) {
         char propTp;
         propTp = lefrNondefProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
         lefrNonDefault.lefiNonDefault::addProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
      }
    ;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1962 "lef.y"
    {
      if (lefrNonDefaultCbk) {
         char propTp;
         propTp = lefrNondefProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
         lefrNonDefault.lefiNonDefault::addProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
      }
    ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1970 "lef.y"
    {
      char temp[32];
      sprintf(temp, "%g", (yyvsp[(2) - (2)].dval));
      if (lefrNonDefaultCbk) {
         char propTp;
         propTp = lefrNondefProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
         lefrNonDefault.lefiNonDefault::addNumProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].dval), temp, propTp);
      }
    ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1981 "lef.y"
    {
    if (lefrNonDefaultCbk) lefrNonDefault.lefiNonDefault::addLayer((yyvsp[(2) - (2)].string));
    ndLayer++;
    strcpy(layerName, (yyvsp[(2) - (2)].string));
    ndLayerWidth = 0;
    ndLayerSpace = 0;
  ;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1989 "lef.y"
    {
    if (strcmp(layerName, (yyvsp[(6) - (6)].string)) != 0) {
      yyerror("END LAYER name is different from LAYER");
      CHKERR();
    }
    if (!ndLayerWidth) {
      yyerror("WIDTH is required in LAYER inside NONDEFULTRULE");
      CHKERR();
    }
    if (!ndLayerSpace) {
      yyerror("SPACING is required in LAYER inside NONDEFULTRULE");
      CHKERR();
    }
  ;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 2012 "lef.y"
    { 
      ndLayerWidth = 1;
      if (lefrNonDefaultCbk) lefrNonDefault.lefiNonDefault::addWidth((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 2017 "lef.y"
    {
      ndLayerSpace = 1;
      if (lefrNonDefaultCbk) lefrNonDefault.lefiNonDefault::addSpacing((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 2022 "lef.y"
    { if (lefrNonDefaultCbk)
         lefrNonDefault.lefiNonDefault::addWireExtension((yyvsp[(2) - (3)].dval)); ;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 2025 "lef.y"
    {
      if (ignoreVersion) {
         if (lefrNonDefaultCbk)
            lefrNonDefault.lefiNonDefault::addResistance((yyvsp[(3) - (4)].dval));
      } else if (versionNum < 5.4) {
         yyerror("RESISTANCE RPERSQ is a 5.4 syntax.  Your lef file is not defined as version 5.4");
         CHKERR();
      }
      else if (lefrNonDefaultCbk)
         lefrNonDefault.lefiNonDefault::addResistance((yyvsp[(3) - (4)].dval));
    ;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 2038 "lef.y"
    {
      if (ignoreVersion) {
         if (lefrNonDefaultCbk)
            lefrNonDefault.lefiNonDefault::addCapacitance((yyvsp[(3) - (4)].dval));
      } else if (versionNum < 5.4) {
         yyerror("CAPACITANCE CPERSQDIST is a 5.4 syntax.  Your lef file is not defined as version 5.4");
         CHKERR();
      }
      else if (lefrNonDefaultCbk)
         lefrNonDefault.lefiNonDefault::addCapacitance((yyvsp[(3) - (4)].dval));
    ;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 2050 "lef.y"
    {
      if (ignoreVersion) {
         if (lefrNonDefaultCbk)
            lefrNonDefault.lefiNonDefault::addEdgeCap((yyvsp[(2) - (3)].dval));
      } else if (versionNum < 5.4) {
         yyerror("EDGECAPACITANCE is a 5.4 syntax.  Your lef file is not defined as version 5.4");
         CHKERR();
      }
      else if (lefrNonDefaultCbk)
         lefrNonDefault.lefiNonDefault::addEdgeCap((yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 2063 "lef.y"
    { CALLBACK(lefrSiteCbk, lefrSiteCbkType, &lefrSite); ;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 2065 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1;;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 2066 "lef.y"
    { 
      if (lefrSiteCbk) lefrSite.lefiSite::setName((yyvsp[(3) - (3)].string));
      strcpy(siteName, (yyvsp[(3) - (3)].string));
    ;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 2071 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1;;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 2072 "lef.y"
    {
      if (strcmp(siteName, (yyvsp[(3) - (3)].string)) != 0) {
        yyerror("END SITE name is different from SITE");
        CHKERR();
      }
    ;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 2086 "lef.y"
    { if (lefrSiteCbk) lefrSite.lefiSite::setSize((yyvsp[(2) - (5)].dval),(yyvsp[(4) - (5)].dval)); ;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 2088 "lef.y"
    { ;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 2090 "lef.y"
    { if (lefrSiteCbk) lefrSite.lefiSite::setClass((yyvsp[(1) - (1)].string)); ;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 2093 "lef.y"
    {(yyval.string) = (char*)"PAD"; ;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 2094 "lef.y"
    {(yyval.string) = (char*)"CORE"; ;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 2095 "lef.y"
    {(yyval.string) = (char*)"VIRTUAL"; ;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 2098 "lef.y"
    { ;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 2107 "lef.y"
    { if (lefrSiteCbk) lefrSite.lefiSite::setXSymmetry(); ;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 2109 "lef.y"
    { if (lefrSiteCbk) lefrSite.lefiSite::setYSymmetry(); ;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 2111 "lef.y"
    { if (lefrSiteCbk) lefrSite.lefiSite::set90Symmetry(); ;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 2116 "lef.y"
    { (yyval.pt).x = (yyvsp[(1) - (2)].dval); (yyval.pt).y = (yyvsp[(2) - (2)].dval); ;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 2118 "lef.y"
    { (yyval.pt).x = (yyvsp[(2) - (4)].dval); (yyval.pt).y = (yyvsp[(3) - (4)].dval); ;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 2121 "lef.y"
    { CALLBACK(lefrMacroCbk, lefrMacroCbkType, &lefrMacro); lefrDoSite = 0;
    ;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 2125 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1;;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 2126 "lef.y"
    {
      siteDef = 0;
      symDef = 0;
      sizeDef = 0; 
      pinDef = 0; 
      obsDef = 0; 
      origDef = 0;
      lefrMacro.lefiMacro::clear();      
      if (lefrMacroCbk) lefrMacro.lefiMacro::setName((yyvsp[(3) - (3)].string));
      CALLBACK(lefrMacroBeginCbk, lefrMacroBeginCbkType, (yyvsp[(3) - (3)].string));
      strcpy(macroName, (yyvsp[(3) - (3)].string));
    ;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 2139 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1;;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 2140 "lef.y"
    {
      if (strcmp(macroName, (yyvsp[(3) - (3)].string)) != 0) {
        yyerror("END MACRO name is different from MACRO");
        CHKERR();
      }
      CALLBACK(lefrMacroEndCbk, lefrMacroEndCbkType, (yyvsp[(3) - (3)].string));
    ;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 2160 "lef.y"
    { ;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 2162 "lef.y"
    { ;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 2164 "lef.y"
    { ;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 2168 "lef.y"
    { ;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 2170 "lef.y"
    { ;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 2172 "lef.y"
    { ;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 2174 "lef.y"
    { if (lefrMacroCbk) lefrMacro.lefiMacro::setBuffer(); ;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 2176 "lef.y"
    { if (lefrMacroCbk) lefrMacro.lefiMacro::setInverter(); ;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 2178 "lef.y"
    { ;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 2180 "lef.y"
    { ;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 2182 "lef.y"
    { ;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 2183 "lef.y"
    {lefDumbMode = 1000000;;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 2184 "lef.y"
    { lefDumbMode = 0; ;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 2192 "lef.y"
    {
      if (siteDef) { /* SITE is defined before SYMMETRY */
          /* pcr 283846 suppress warning */
          yywarning("SITE is defined before SYMMETRY.");
      }
      symDef = 1;
    ;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 2207 "lef.y"
    { if (lefrMacroCbk) lefrMacro.lefiMacro::setXSymmetry(); ;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 2209 "lef.y"
    { if (lefrMacroCbk) lefrMacro.lefiMacro::setYSymmetry(); ;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 2211 "lef.y"
    { if (lefrMacroCbk) lefrMacro.lefiMacro::set90Symmetry(); ;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 2215 "lef.y"
    {
      char temp[32];
      sprintf(temp, "%g", (yyvsp[(2) - (2)].dval));
      if (lefrMacroCbk) {
         char propTp;
         propTp = lefrMacroProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
         lefrMacro.lefiMacro::setNumProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].dval), temp,  propTp);
      }
    ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 2225 "lef.y"
    {
      if (lefrMacroCbk) {
         char propTp;
         propTp = lefrMacroProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
         lefrMacro.lefiMacro::setProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
      }
    ;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 2233 "lef.y"
    {
      if (lefrMacroCbk) {
         char propTp;
         propTp = lefrMacroProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
         lefrMacro.lefiMacro::setProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
      }
    ;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 2242 "lef.y"
    {
       if (lefrMacroCbk) lefrMacro.lefiMacro::setClass((yyvsp[(2) - (3)].string));
       if (lefrMacroClassTypeCbk)
          CALLBACK(lefrMacroClassTypeCbk, lefrMacroClassTypeCbkType, (yyvsp[(2) - (3)].string));
    ;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 2249 "lef.y"
    {(yyval.string) = (char*)"COVER"; ;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 2250 "lef.y"
    {(yyval.string) = (char*)"COVER BUMP"; ;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 2251 "lef.y"
    {(yyval.string) = (char*)"RING"; ;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 2252 "lef.y"
    {(yyval.string) = (char*)"BLOCK"; ;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 2253 "lef.y"
    {(yyval.string) = (char*)"BLOCK BLACKBOX"; ;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 2254 "lef.y"
    {(yyval.string) = (char*)"NONE"; ;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 2255 "lef.y"
    {(yyval.string) = (char*)"PAD"; ;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 2256 "lef.y"
    {(yyval.string) = (char*)"VIRTUAL"; ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 2258 "lef.y"
    {sprintf(temp_name, "PAD %s", (yyvsp[(2) - (2)].string));
      (yyval.string) = temp_name;;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 2260 "lef.y"
    {(yyval.string) = (char*)"CORE"; ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 2262 "lef.y"
    {(yyval.string) = (char*)"CORNER";
      /* This token is NOT in the spec but has shown up in 
       * some lef files.  This exception came from LEFOUT
       * in 'frameworks'
       */
      ;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 2269 "lef.y"
    {sprintf(temp_name, "CORE %s", (yyvsp[(2) - (2)].string));
      (yyval.string) = temp_name;;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 2272 "lef.y"
    {sprintf(temp_name, "ENDCAP %s", (yyvsp[(2) - (2)].string));
      (yyval.string) = temp_name;;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 2276 "lef.y"
    {(yyval.string) = (char*)"INPUT";;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 2277 "lef.y"
    {(yyval.string) = (char*)"OUTPUT";;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 2278 "lef.y"
    {(yyval.string) = (char*)"INOUT";;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 2279 "lef.y"
    {(yyval.string) = (char*)"POWER";;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 2280 "lef.y"
    {(yyval.string) = (char*)"SPACER";;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 2281 "lef.y"
    {(yyval.string) = (char*)"AREAIO";;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 2284 "lef.y"
    {(yyval.string) = (char*)"FEEDTHRU";;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 2285 "lef.y"
    {(yyval.string) = (char*)"TIEHIGH";;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 2286 "lef.y"
    {(yyval.string) = (char*)"TIELOW";;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 2288 "lef.y"
    { 
      if (ignoreVersion)
        (yyval.string) = (char*)"SPACER";
      else if (versionNum < 5.4) {
        yyerror("SPACER is a 5.4 syntax.  Your lef file is not defined as version 5.4"); 
        CHKERR();
      }
      else
        (yyval.string) = (char*)"SPACER";
    ;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 2299 "lef.y"
    { 
      if (ignoreVersion)
        (yyval.string) = (char*)"ANTENNACELL";
      else if (versionNum < 5.4) {
        yyerror("ANTENNACELL is a 5.4 syntax.  Your lef file is not defined as version 5.4"); 
        CHKERR();
      }
      else
        (yyval.string) = (char*)"ANTENNACELL";
    ;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 2311 "lef.y"
    {(yyval.string) = (char*)"PRE";;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 2312 "lef.y"
    {(yyval.string) = (char*)"POST";;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 2313 "lef.y"
    {(yyval.string) = (char*)"TOPLEFT";;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 2314 "lef.y"
    {(yyval.string) = (char*)"TOPRIGHT";;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 2315 "lef.y"
    {(yyval.string) = (char*)"BOTTOMLEFT";;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 2316 "lef.y"
    {(yyval.string) = (char*)"BOTTOMRIGHT";;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 2319 "lef.y"
    { if (lefrMacroCbk) lefrMacro.lefiMacro::setGenerator((yyvsp[(2) - (3)].string)); ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 2322 "lef.y"
    { if (lefrMacroCbk) lefrMacro.lefiMacro::setGenerate((yyvsp[(2) - (4)].string), (yyvsp[(3) - (4)].string)); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 2326 "lef.y"
    { if (lefrMacroCbk) lefrMacro.lefiMacro::setSource("USER"); ;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 2328 "lef.y"
    { if (lefrMacroCbk) lefrMacro.lefiMacro::setSource("GENERATE"); ;}
    break;

  case 462:

/* Line 1455 of yacc.c  */
#line 2330 "lef.y"
    { if (lefrMacroCbk) lefrMacro.lefiMacro::setSource("BLOCK"); ;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 2333 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrMacroCbk) lefrMacro.lefiMacro::setPower((yyvsp[(2) - (3)].dval));
      } else
        yywarning("MACRO POWER is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 464:

/* Line 1455 of yacc.c  */
#line 2341 "lef.y"
    { 
       if (origDef) { /* Has multiple ORIGIN defined in a macro, stop parsing*/
          yyerror("ORIGIN statement is defined more than once.  Parser stops executions.");
          CHKERR();
       }
       origDef = 1;
       if (siteDef) { /* SITE is defined before ORIGIN */
          /* pcr 283846 suppress warning */
          yywarning("SITE is defined before ORIGIN.");
       }
       if (pinDef) { /* PIN is defined before ORIGIN */
          /* pcr 283846 suppress warning */
          yywarning("PIN is defined before ORIGIN.");
       }
       if (obsDef) { /* OBS is defined before ORIGIN */
          /* pcr 283846 suppress warning */
          yywarning("OBS is defined before ORIGIN.");
       }
       /* 11/22/99 - Wanda da Rosa. PCR 283846 
                     can be defined any order.
       if (symDef)  * SYMMETRY is defined before ORIGIN *
          yywarning("SYMMETRY is defined before ORIGIN.");
       */
       /* Add back it back in per Nora request on PCR 283846 */
       /* 1/14/2000 - Wanda da Rosa, PCR 288770
       if (sizeDef)  * SIZE is defined before ORIGIN *
          yywarning("SIZE is defined before ORIGIN.");
       */
      
       if (lefrMacroCbk) lefrMacro.lefiMacro::setOrigin((yyvsp[(2) - (3)].pt).x, (yyvsp[(2) - (3)].pt).y);
       if (lefrMacroOriginCbk) {
          macroNum.x = (yyvsp[(2) - (3)].pt).x; 
          macroNum.y = (yyvsp[(2) - (3)].pt).y; 
          CALLBACK(lefrMacroOriginCbk, lefrMacroOriginCbkType, macroNum);
       }
    ;}
    break;

  case 465:

/* Line 1455 of yacc.c  */
#line 2380 "lef.y"
    { if (lefrMacroCbk)
      lefrMacro.lefiMacro::setForeign((yyvsp[(1) - (2)].string), 0, 0.0, 0.0, -1); ;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 2383 "lef.y"
    { if (lefrMacroCbk)
      lefrMacro.lefiMacro::setForeign((yyvsp[(1) - (3)].string), 1, (yyvsp[(2) - (3)].pt).x, (yyvsp[(2) - (3)].pt).y, -1); ;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 2386 "lef.y"
    { if (lefrMacroCbk)
      lefrMacro.lefiMacro::setForeign((yyvsp[(1) - (4)].string), 1, (yyvsp[(2) - (4)].pt).x, (yyvsp[(2) - (4)].pt).y, (yyvsp[(3) - (4)].integer)); ;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 2389 "lef.y"
    { if (lefrMacroCbk)
      lefrMacro.lefiMacro::setForeign((yyvsp[(1) - (3)].string), 0, 0.0, 0.0, (yyvsp[(2) - (3)].integer)); ;}
    break;

  case 469:

/* Line 1455 of yacc.c  */
#line 2392 "lef.y"
    { lefDumbMode = 1; lefNoNum = 1; ;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 2393 "lef.y"
    { if (lefrMacroCbk) lefrMacro.lefiMacro::setEEQ((yyvsp[(3) - (4)].string)); ;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 2395 "lef.y"
    { lefDumbMode = 1; lefNoNum = 1; ;}
    break;

  case 472:

/* Line 1455 of yacc.c  */
#line 2396 "lef.y"
    { if (lefrMacroCbk) lefrMacro.lefiMacro::setLEQ((yyvsp[(3) - (4)].string)); ;}
    break;

  case 473:

/* Line 1455 of yacc.c  */
#line 2400 "lef.y"
    { if (lefrMacroCbk) lefrMacro.lefiMacro::setSiteName((yyvsp[(2) - (3)].string)); ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 2402 "lef.y"
    {
      if (lefrMacroCbk) {
	lefrMacro.lefiMacro::setSitePattern(lefrSitePatternPtr);
	lefrSitePatternPtr = 0;
      }
    ;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 2410 "lef.y"
    { lefDumbMode = 1; lefNoNum = 1; siteDef = 1;
        if (lefrMacroCbk) lefrDoSite = 1; ;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 2414 "lef.y"
    { lefDumbMode = 1; lefNoNum = 1; ;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 2417 "lef.y"
    { 
      if (siteDef) { /* SITE is defined before SIZE */
         /* pcr 283846 suppress warning
         yywarning("SITE is defined before SIZE.");
         return 1; 
         */
      }
      sizeDef = 1;
      if (lefrMacroCbk) lefrMacro.lefiMacro::setSize((yyvsp[(2) - (5)].dval), (yyvsp[(4) - (5)].dval));
       if (lefrMacroSizeCbk) {
          macroNum.x = (yyvsp[(2) - (5)].dval); 
          macroNum.y = (yyvsp[(4) - (5)].dval); 
          CALLBACK(lefrMacroSizeCbk, lefrMacroSizeCbkType, macroNum);
       }
    ;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 2437 "lef.y"
    { CALLBACK(lefrPinCbk, lefrPinCbkType, &lefrPin);
      lefrPin.lefiPin::clear();
    ;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 2441 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1; pinDef = 1;;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 2442 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setName((yyvsp[(3) - (3)].string));
      strcpy(pinName, (yyvsp[(3) - (3)].string));
    ;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 2446 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1;;}
    break;

  case 482:

/* Line 1455 of yacc.c  */
#line 2447 "lef.y"
    {
      if (strcmp(pinName, (yyvsp[(3) - (3)].string)) != 0) {
        yyerror("END PIN name is different from PIN");
        CHKERR();
      }
    ;}
    break;

  case 483:

/* Line 1455 of yacc.c  */
#line 2456 "lef.y"
    { ;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 2458 "lef.y"
    { ;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 2462 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setForeign((yyvsp[(1) - (2)].string), 0, 0.0, 0.0, -1); ;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 2464 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setForeign((yyvsp[(1) - (3)].string), 1, (yyvsp[(2) - (3)].pt).x, (yyvsp[(2) - (3)].pt).y, -1); ;}
    break;

  case 487:

/* Line 1455 of yacc.c  */
#line 2466 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setForeign((yyvsp[(1) - (4)].string), 1, (yyvsp[(2) - (4)].pt).x, (yyvsp[(2) - (4)].pt).y, (yyvsp[(3) - (4)].integer)); ;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 2468 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setForeign((yyvsp[(1) - (3)].string), 0, 0.0, 0.0, -1); ;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 2470 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setForeign((yyvsp[(1) - (4)].string), 1, (yyvsp[(3) - (4)].pt).x, (yyvsp[(3) - (4)].pt).y, -1); ;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 2472 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setForeign((yyvsp[(1) - (5)].string), 1, (yyvsp[(3) - (5)].pt).x, (yyvsp[(3) - (5)].pt).y, (yyvsp[(4) - (5)].integer)); ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 2473 "lef.y"
    { lefDumbMode = 1; lefNoNum = 1; ;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 2474 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setLEQ((yyvsp[(3) - (4)].string)); ;}
    break;

  case 493:

/* Line 1455 of yacc.c  */
#line 2476 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setPower((yyvsp[(2) - (3)].dval));
      } else
        yywarning("MACRO POWER is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 494:

/* Line 1455 of yacc.c  */
#line 2483 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setDirection((yyvsp[(1) - (1)].string)); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 2485 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setUse((yyvsp[(2) - (3)].string)); ;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2487 "lef.y"
    { ;}
    break;

  case 497:

/* Line 1455 of yacc.c  */
#line 2489 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setLeakage((yyvsp[(2) - (3)].dval));
      } else
        yywarning("MACRO LEAKAGE is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 498:

/* Line 1455 of yacc.c  */
#line 2496 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setRiseThresh((yyvsp[(2) - (3)].dval));
      } else
        yywarning("MACRO RISETHRESH is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 499:

/* Line 1455 of yacc.c  */
#line 2503 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setFallThresh((yyvsp[(2) - (3)].dval));
      } else
        yywarning("MACRO FALLTHRESH is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2510 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setRiseSatcur((yyvsp[(2) - (3)].dval));
      } else
        yywarning("MACRO RISESATCUR is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2517 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setFallSatcur((yyvsp[(2) - (3)].dval));
      } else
        yywarning("MACRO FALLSATCUR is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2524 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setVLO((yyvsp[(2) - (3)].dval));
      } else
        yywarning("MACRO VLO is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2531 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setVHI((yyvsp[(2) - (3)].dval));
      } else
        yywarning("MACRO VHI is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2538 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setTieoffr((yyvsp[(2) - (3)].dval));
      } else
        yywarning("MACRO TIEOFFR is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2545 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setShape((yyvsp[(2) - (3)].string)); ;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2546 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1;;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2547 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setMustjoin((yyvsp[(3) - (4)].string)); ;}
    break;

  case 508:

/* Line 1455 of yacc.c  */
#line 2548 "lef.y"
    {lefDumbMode = 1;;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2549 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setOutMargin((yyvsp[(3) - (5)].dval), (yyvsp[(4) - (5)].dval));
      } else
        yywarning(
          "MACRO OUTPUTNOISEMARGIN is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2556 "lef.y"
    {lefDumbMode = 1;;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2557 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setOutResistance((yyvsp[(3) - (5)].dval), (yyvsp[(4) - (5)].dval));
      } else
        yywarning(
          "MACRO OUTPUTRESISTANCE is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2564 "lef.y"
    {lefDumbMode = 1;;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2565 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setInMargin((yyvsp[(3) - (5)].dval), (yyvsp[(4) - (5)].dval));
      } else
        yywarning(
          "MACRO INPUTNOISEMARGIN is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 514:

/* Line 1455 of yacc.c  */
#line 2573 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setCapacitance((yyvsp[(2) - (3)].dval));
      } else
        yywarning("MACRO CAPACITANCE is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 515:

/* Line 1455 of yacc.c  */
#line 2580 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setMaxdelay((yyvsp[(2) - (3)].dval)); ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2582 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setMaxload((yyvsp[(2) - (3)].dval)); ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2584 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setResistance((yyvsp[(2) - (3)].dval));
      } else
        yywarning("MACRO RESISTANCE is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2591 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setPulldownres((yyvsp[(2) - (3)].dval));
      } else
        yywarning("MACRO PULLDOWNRES is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2598 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setCurrentSource("ACTIVE");
      } else
        yywarning(
          "MACRO CURRENTSOURCE is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2606 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setCurrentSource("RESISTIVE");
      } else
        yywarning(
          "MACRO CURRENTSOURCE is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2614 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setRiseVoltage((yyvsp[(2) - (3)].dval));
      } else
        yywarning(
          "MACRO RISEVOLTAGETHRESHOLD is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 522:

/* Line 1455 of yacc.c  */
#line 2622 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setFallVoltage((yyvsp[(2) - (3)].dval));
      } else
        yywarning(
          "MACRO FALLVOLTAGETHRESHOLD is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2630 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrPinCbk) lefrPin.lefiPin::setTables((yyvsp[(2) - (4)].string), (yyvsp[(3) - (4)].string));
      } else
        yywarning("MACRO IV_TABLES is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 524:

/* Line 1455 of yacc.c  */
#line 2637 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setTaperRule((yyvsp[(2) - (3)].string)); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2638 "lef.y"
    {lefDumbMode = 1000000;;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2639 "lef.y"
    { lefDumbMode = 0; ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2641 "lef.y"
    {
      lefDumbMode = 0;
      if (lefrPinCbk) {
	lefrPin.lefiPin::addPort(lefrGeometriesPtr);
	lefrGeometriesPtr = 0;
	lefrDoGeometries = 0;
      }
    ;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2653 "lef.y"
    {
      // Since in start_macro_port it has call the Init method, here
      // we need to call the Destroy method.
      // Still add a null pointer to set the number of port
      lefrPin.lefiPin::addPort(lefrGeometriesPtr);
      lefrGeometriesPtr = 0;
      lefrDoGeometries = 0;
    ;}
    break;

  case 529:

/* Line 1455 of yacc.c  */
#line 2662 "lef.y"
    {  /* a pre 5.4 syntax */
      use5_3 = 1;
      if (ignoreVersion) {
        /* do nothing */
      } else if (versionNum >= 5.4) {
        if (use5_4) {
           yyerror("ANTENNASIZE is a 5.3 or earlier syntax.  Your lef file has both old and new syntax, which is illegal.");
           CHKERR();
        }
      }
      if (lefrPinCbk) lefrPin.lefiPin::addAntennaSize((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].string));
    ;}
    break;

  case 530:

/* Line 1455 of yacc.c  */
#line 2675 "lef.y"
    {  /* a pre 5.4 syntax */
      use5_3 = 1;
      if (ignoreVersion) {
        /* do nothing */
      } else if (versionNum >= 5.4) {
        if (use5_4) {
           yyerror("ANTENNAMETALAREA is a 5.3 or earlier syntax.  Your lef file has both old and new syntax, which is illegal.");
           CHKERR();
        }
      }
      if (lefrPinCbk) lefrPin.lefiPin::addAntennaMetalArea((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].string));
    ;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2688 "lef.y"
    { /* a pre 5.4 syntax */ 
      use5_3 = 1;
      if (ignoreVersion) {
        /* do nothing */
      } else if (versionNum >= 5.4) {
        if (use5_4) {
           yyerror("ANTENNAMETALLENGTH is a 5.3 or earlier syntax.  Your lef file has both old and new syntax, which is illegal.");
           CHKERR();
        }
      }
      if (lefrPinCbk) lefrPin.lefiPin::addAntennaMetalLength((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].string));
    ;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2701 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setRiseSlewLimit((yyvsp[(2) - (3)].dval)); ;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2703 "lef.y"
    { if (lefrPinCbk) lefrPin.lefiPin::setFallSlewLimit((yyvsp[(2) - (3)].dval)); ;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2705 "lef.y"
    { /* 5.4 syntax */
      use5_4 = 1;
      if (ignoreVersion) {
        /* do nothing */
      } else if (versionNum < 5.4) {
        yyerror("ANTENNAPARTIALMETALAREA is a 5.4 syntax. Your lef file is not defined as 5.4");
        CHKERR();
      } else if (use5_3) {
           yyerror("ANTENNAPARTIALMETALAREA is a 5.4 syntax. Your lef file has both old and new syntax, which is illegal.");
           CHKERR();
      }
      if (lefrPinCbk) lefrPin.lefiPin::addAntennaPartialMetalArea((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].string));
    ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2719 "lef.y"
    { /* 5.4 syntax */
      use5_4 = 1;
      if (ignoreVersion) {
        /* do nothing */
      } else if (versionNum < 5.4) {
        yyerror("ANTENNAPARTIALMETALSIDEAREA is a 5.4 syntax. Your lef file is not defined as 5.4");
        CHKERR();
      } else if (use5_3) {
         yyerror("ANTENNAPARTIALMETALSIDEAREA is a 5.4 syntax. Your lef file has both old and new syntax, which is illegal.");
         CHKERR();
      }
      if (lefrPinCbk) lefrPin.lefiPin::addAntennaPartialMetalSideArea((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].string));
    ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2733 "lef.y"
    { /* 5.4 syntax */
      use5_4 = 1;
      if (ignoreVersion) {
        /* do nothing */
      } else if (versionNum < 5.4) {
        yyerror("ANTENNAPARTIALCUTAREA is a 5.4 syntax. Your lef file is not defined as 5.4");
        CHKERR();
      } else if (use5_3) {
        yyerror("ANTENNAPARTIALCUTAREA is a 5.4 syntax. Your lef file has both old and new syntax, which is illegal.");
        CHKERR();
      }
      if (lefrPinCbk) lefrPin.lefiPin::addAntennaPartialCutArea((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].string));
    ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2747 "lef.y"
    { /* 5.4 syntax */
      use5_4 = 1;
      if (ignoreVersion) {
        /* do nothing */
      } else if (versionNum < 5.4) {
        yyerror("ANTENNADIFFAREA is a 5.4 syntax. Your lef file is not defined as 5.4");
        CHKERR();
      } else if (use5_3) {
        yyerror("ANTENNADIFFAREA is a 5.4 syntax. Your lef file has both old and new syntax, which is illegal.");
        CHKERR();
      }
      if (lefrPinCbk) lefrPin.lefiPin::addAntennaDiffArea((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].string));
    ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2761 "lef.y"
    { /* 5.4 syntax */
      use5_4 = 1;
      if (ignoreVersion) {
        /* do nothing */
      } else if (versionNum < 5.4) {
        yyerror("ANTENNAGATEAREA is a 5.4 syntax. Your lef file is not defined as 5.4");
        CHKERR();
      } else if (use5_3) {
        yyerror("ANTENNAGATEAREA is a 5.4 syntax. Your lef file has both old and new syntax, which is illegal.");
        CHKERR();
      }
      if (lefrPinCbk) lefrPin.lefiPin::addAntennaGateArea((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].string));
    ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2775 "lef.y"
    { /* 5.4 syntax */
      use5_4 = 1;
      if (ignoreVersion) {
        /* do nothing */
      } else if (versionNum < 5.4) {
        yyerror("ANTENNAMAXAREACAR is a 5.4 syntax. Your lef file is not defined as 5.4");
        CHKERR();
      } else if (use5_3) {
         yyerror("ANTENNAMAXAREACAR is a 5.4 syntax. Your lef file has both old and new syntax, which is illegal.");
        CHKERR();
      }
      if (lefrPinCbk) lefrPin.lefiPin::addAntennaMaxAreaCar((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].string));
    ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2789 "lef.y"
    { /* 5.4 syntax */
      use5_4 = 1;
      if (ignoreVersion) {
        /* do nothing */
      } else if (versionNum < 5.4) {
        yyerror("ANTENNAMAXSIDEAREACAR is a 5.4 syntax. Your lef file is not defined as 5.4");
        CHKERR();
      } else if (use5_3) {
        yyerror("ANTENNAMAXSIDEAREACAR is a 5.4 syntax. Your lef file has both old and new syntax, which is illegal.");
        CHKERR();
      }
      if (lefrPinCbk) lefrPin.lefiPin::addAntennaMaxSideAreaCar((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].string));
    ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2803 "lef.y"
    { /* 5.4 syntax */
      use5_4 = 1;
      if (ignoreVersion) {
        /* do nothing */
      } else if (versionNum < 5.4) {
        yyerror("ANTENNAMAXCUTCAR is a 5.4 syntax. Your lef file is not defined as 5.4");
        CHKERR();
      } else if (use5_3) {
        yyerror("ANTENNAMAXCUTCAR is a 5.4 syntax. Your lef file has both old and new syntax, which is illegal.");
        CHKERR();
      }
      if (lefrPinCbk) lefrPin.lefiPin::addAntennaMaxCutCar((yyvsp[(2) - (4)].dval), (yyvsp[(3) - (4)].string));
    ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2817 "lef.y"
    { /* 5.5 syntax */
      use5_4 = 1;
      if (ignoreVersion) {
        /* do nothing */
      } else if (versionNum < 5.5) {
        yyerror("ANTENNAMODEL is a 5.5 syntax. Your lef file is not defined as 5.5");
        CHKERR();
      } else if (use5_3) {
        yyerror("ANTENNAMODEL is a 5.5 syntax. Your lef file has both old and new syntax, which is illegal.");
        CHKERR();
      }
    ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2833 "lef.y"
    {
    if (lefrPinCbk)
       lefrPin.lefiPin::addAntennaModel(1);
    ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2838 "lef.y"
    {
    if (lefrPinCbk)
       lefrPin.lefiPin::addAntennaModel(2);
    ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2843 "lef.y"
    {
    if (lefrPinCbk)
       lefrPin.lefiPin::addAntennaModel(3);
    ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2848 "lef.y"
    {
    if (lefrPinCbk)
       lefrPin.lefiPin::addAntennaModel(4);
    ;}
    break;

  case 550:

/* Line 1455 of yacc.c  */
#line 2860 "lef.y"
    { 
      char temp[32];
      sprintf(temp, "%g", (yyvsp[(2) - (2)].dval));
      if (lefrPinCbk) {
         char propTp;
         propTp = lefrPinProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
         lefrPin.lefiPin::setNumProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].dval), temp, propTp);
      }
    ;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2870 "lef.y"
    {
      if (lefrPinCbk) {
         char propTp;
         propTp = lefrPinProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
         lefrPin.lefiPin::setProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
      }
    ;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2878 "lef.y"
    {
      if (lefrPinCbk) {
         char propTp;
         propTp = lefrPinProp.lefiPropType::propType((yyvsp[(1) - (2)].string));
         lefrPin.lefiPin::setProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
      }
    ;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2887 "lef.y"
    {(yyval.string) = (char*)"INPUT";;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2888 "lef.y"
    {(yyval.string) = (char*)"OUTPUT";;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2889 "lef.y"
    {(yyval.string) = (char*)"OUTPUT TRISTATE";;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2890 "lef.y"
    {(yyval.string) = (char*)"INOUT";;}
    break;

  case 557:

/* Line 1455 of yacc.c  */
#line 2891 "lef.y"
    {(yyval.string) = (char*)"FEEDTHRU";;}
    break;

  case 558:

/* Line 1455 of yacc.c  */
#line 2894 "lef.y"
    {
      if (lefrPinCbk) {
	lefrDoGeometries = 1;
	lefrGeometriesPtr = (lefiGeometries*)lefMalloc( sizeof(lefiGeometries));
	lefrGeometriesPtr->lefiGeometries::Init();
      }
    ;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2904 "lef.y"
    { if (lefrDoGeometries)
        lefrGeometriesPtr->lefiGeometries::addClass((yyvsp[(2) - (3)].string)); ;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2908 "lef.y"
    {(yyval.string) = (char*)"SIGNAL";;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2909 "lef.y"
    {(yyval.string) = (char*)"ANALOG";;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2910 "lef.y"
    {(yyval.string) = (char*)"POWER";;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2911 "lef.y"
    {(yyval.string) = (char*)"GROUND";;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2912 "lef.y"
    {(yyval.string) = (char*)"CLOCK";;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2913 "lef.y"
    {(yyval.string) = (char*)"DATA";;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2916 "lef.y"
    {(yyval.string) = (char*)"INPUT";;}
    break;

  case 568:

/* Line 1455 of yacc.c  */
#line 2917 "lef.y"
    {(yyval.string) = (char*)"OUTPUT";;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2918 "lef.y"
    {(yyval.string) = (char*)"START";;}
    break;

  case 570:

/* Line 1455 of yacc.c  */
#line 2919 "lef.y"
    {(yyval.string) = (char*)"STOP";;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2922 "lef.y"
    {(yyval.string) = (char*)""; ;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2923 "lef.y"
    {(yyval.string) = (char*)"ABUTMENT";;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2924 "lef.y"
    {(yyval.string) = (char*)"RING";;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2925 "lef.y"
    {(yyval.string) = (char*)"FEEDTHRU";;}
    break;

  case 576:

/* Line 1455 of yacc.c  */
#line 2930 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1; ;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2931 "lef.y"
    { if (lefrDoGeometries)
        lefrGeometriesPtr->lefiGeometries::addLayer((yyvsp[(3) - (3)].string));
    ;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2936 "lef.y"
    { if (lefrDoGeometries)
        lefrGeometriesPtr->lefiGeometries::addWidth((yyvsp[(2) - (3)].dval)); ;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2939 "lef.y"
    { if (lefrDoGeometries) lefrGeometriesPtr->lefiGeometries::addPath(); ;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2941 "lef.y"
    { if (lefrDoGeometries)
        lefrGeometriesPtr->lefiGeometries::addPathIter(); ;}
    break;

  case 582:

/* Line 1455 of yacc.c  */
#line 2944 "lef.y"
    { if (lefrDoGeometries)
        lefrGeometriesPtr->lefiGeometries::addRect((yyvsp[(2) - (4)].pt).x, (yyvsp[(2) - (4)].pt).y, (yyvsp[(3) - (4)].pt).x, (yyvsp[(3) - (4)].pt).y); ;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2947 "lef.y"
    { if (lefrDoGeometries)
        lefrGeometriesPtr->lefiGeometries::addRectIter((yyvsp[(3) - (6)].pt).x, (yyvsp[(3) - (6)].pt).y, (yyvsp[(4) - (6)].pt).x, (yyvsp[(4) - (6)].pt).y);
    ;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2951 "lef.y"
    { if (lefrDoGeometries)
        lefrGeometriesPtr->lefiGeometries::addPolygon(); ;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2954 "lef.y"
    { if (lefrDoGeometries)
        lefrGeometriesPtr->lefiGeometries::addPolygonIter(); ;}
    break;

  case 586:

/* Line 1455 of yacc.c  */
#line 2957 "lef.y"
    { ;}
    break;

  case 590:

/* Line 1455 of yacc.c  */
#line 2964 "lef.y"
    { if (lefrDoGeometries)
        lefrGeometriesPtr->lefiGeometries::addLayerMinSpacing((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2967 "lef.y"
    { if (lefrDoGeometries)
        lefrGeometriesPtr->lefiGeometries::addLayerRuleWidth((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2971 "lef.y"
    { if (lefrDoGeometries)
        lefrGeometriesPtr->lefiGeometries::startList((yyvsp[(1) - (1)].pt).x, (yyvsp[(1) - (1)].pt).y); ;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2975 "lef.y"
    { if (lefrDoGeometries)
        lefrGeometriesPtr->lefiGeometries::addToList((yyvsp[(1) - (1)].pt).x, (yyvsp[(1) - (1)].pt).y); ;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2984 "lef.y"
    {lefDumbMode = 1;;}
    break;

  case 597:

/* Line 1455 of yacc.c  */
#line 2985 "lef.y"
    { if (lefrDoGeometries)
        lefrGeometriesPtr->lefiGeometries::addVia((yyvsp[(2) - (5)].pt).x, (yyvsp[(2) - (5)].pt).y, (yyvsp[(4) - (5)].string)); ;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2987 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1;;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2989 "lef.y"
    { if (lefrDoGeometries)
        lefrGeometriesPtr->lefiGeometries::addViaIter((yyvsp[(3) - (7)].pt).x, (yyvsp[(3) - (7)].pt).y, (yyvsp[(5) - (7)].string)); ;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2994 "lef.y"
    { if (lefrDoGeometries)
         lefrGeometriesPtr->lefiGeometries::addStepPattern((yyvsp[(2) - (7)].dval), (yyvsp[(4) - (7)].dval), (yyvsp[(6) - (7)].dval), (yyvsp[(7) - (7)].dval)); ;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2999 "lef.y"
    {
      if (lefrDoSite) {
	lefrSitePatternPtr = (lefiSitePattern*)lefMalloc(
				   sizeof(lefiSitePattern));
	lefrSitePatternPtr->lefiSitePattern::Init();
	lefrSitePatternPtr->lefiSitePattern::set((yyvsp[(1) - (11)].string), (yyvsp[(2) - (11)].dval), (yyvsp[(3) - (11)].dval), (yyvsp[(4) - (11)].integer), (yyvsp[(6) - (11)].dval), (yyvsp[(8) - (11)].dval),
	  (yyvsp[(10) - (11)].dval), (yyvsp[(11) - (11)].dval));
	}
    ;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 3011 "lef.y"
    { 
      if (lefrDoTrack) {
	lefrTrackPatternPtr = (lefiTrackPattern*)lefMalloc(
				sizeof(lefiTrackPattern));
	lefrTrackPatternPtr->lefiTrackPattern::Init();
	lefrTrackPatternPtr->lefiTrackPattern::set("X", (yyvsp[(2) - (6)].dval), (int)(yyvsp[(4) - (6)].dval), (yyvsp[(6) - (6)].dval));
      }    
    ;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 3019 "lef.y"
    {lefDumbMode = 1000000000;;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 3020 "lef.y"
    { lefDumbMode = 0;;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 3022 "lef.y"
    { 
      if (lefrDoTrack) {
	lefrTrackPatternPtr = (lefiTrackPattern*)lefMalloc(
                                    sizeof(lefiTrackPattern));
	lefrTrackPatternPtr->lefiTrackPattern::Init();
	lefrTrackPatternPtr->lefiTrackPattern::set("Y", (yyvsp[(2) - (6)].dval), (int)(yyvsp[(4) - (6)].dval), (yyvsp[(6) - (6)].dval));
      }    
    ;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 3030 "lef.y"
    {lefDumbMode = 1000000000;;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 3031 "lef.y"
    { lefDumbMode = 0;;}
    break;

  case 608:

/* Line 1455 of yacc.c  */
#line 3033 "lef.y"
    { 
      if (lefrDoTrack) {
	lefrTrackPatternPtr = (lefiTrackPattern*)lefMalloc(
                                    sizeof(lefiTrackPattern));
	lefrTrackPatternPtr->lefiTrackPattern::Init();
	lefrTrackPatternPtr->lefiTrackPattern::set("X", (yyvsp[(2) - (6)].dval), (int)(yyvsp[(4) - (6)].dval), (yyvsp[(6) - (6)].dval));
      }    
    ;}
    break;

  case 609:

/* Line 1455 of yacc.c  */
#line 3042 "lef.y"
    { 
      if (lefrDoTrack) {
	lefrTrackPatternPtr = (lefiTrackPattern*)lefMalloc(
                                    sizeof(lefiTrackPattern));
	lefrTrackPatternPtr->lefiTrackPattern::Init();
	lefrTrackPatternPtr->lefiTrackPattern::set("Y", (yyvsp[(2) - (6)].dval), (int)(yyvsp[(4) - (6)].dval), (yyvsp[(6) - (6)].dval));
      }    
    ;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 3057 "lef.y"
    { if (lefrDoTrack) lefrTrackPatternPtr->lefiTrackPattern::addLayer((yyvsp[(1) - (1)].string)); ;}
    break;

  case 613:

/* Line 1455 of yacc.c  */
#line 3060 "lef.y"
    {
      if (lefrDoGcell) {
	lefrGcellPatternPtr = (lefiGcellPattern*)lefMalloc(
                                    sizeof(lefiGcellPattern));
	lefrGcellPatternPtr->lefiGcellPattern::Init();
	lefrGcellPatternPtr->lefiGcellPattern::set("X", (yyvsp[(2) - (6)].dval), (int)(yyvsp[(4) - (6)].dval), (yyvsp[(6) - (6)].dval));
      }    
    ;}
    break;

  case 614:

/* Line 1455 of yacc.c  */
#line 3069 "lef.y"
    {
      if (lefrDoGcell) {
	lefrGcellPatternPtr = (lefiGcellPattern*)lefMalloc(
                                    sizeof(lefiGcellPattern));
	lefrGcellPatternPtr->lefiGcellPattern::Init();
	lefrGcellPatternPtr->lefiGcellPattern::set("Y", (yyvsp[(2) - (6)].dval), (int)(yyvsp[(4) - (6)].dval), (yyvsp[(6) - (6)].dval));
      }    
    ;}
    break;

  case 615:

/* Line 1455 of yacc.c  */
#line 3079 "lef.y"
    { 
      if (lefrObstructionCbk) {
	lefrObstruction.lefiObstruction::setGeometries(lefrGeometriesPtr);
	lefrGeometriesPtr = 0;
	lefrDoGeometries = 0;
      }
      CALLBACK(lefrObstructionCbk, lefrObstructionCbkType, &lefrObstruction);
      lefDumbMode = 0;
    ;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 3098 "lef.y"
    {
      obsDef = 1;
      if (lefrObstructionCbk) {
	lefrDoGeometries = 1;
	lefrGeometriesPtr = (lefiGeometries*)lefMalloc(
	    sizeof(lefiGeometries));
	lefrGeometriesPtr->lefiGeometries::Init();
	}
    ;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 3108 "lef.y"
    { lefDumbMode = 1; lefNoNum = 1; ;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 3109 "lef.y"
    { if (lefrMacroCbk) lefrMacro.lefiMacro::setClockType((yyvsp[(3) - (4)].string)); ;}
    break;

  case 620:

/* Line 1455 of yacc.c  */
#line 3112 "lef.y"
    { ;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 3115 "lef.y"
    { /* XXXXX for macros */ ;}
    break;

  case 622:

/* Line 1455 of yacc.c  */
#line 3118 "lef.y"
    {
    if (versionNum < 5.4) {
      if (lefrTimingCbk && lefrTiming.lefiTiming::hasData())
        CALLBACK(lefrTimingCbk, lefrTimingCbkType, &lefrTiming);
      lefrTiming.lefiTiming::clear();
    } else {
      yywarning("MACRO TIMING is obsolete in 5.4. It will be ignored.");
      lefrTiming.lefiTiming::clear();
    }
  ;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 3136 "lef.y"
    {
    if (versionNum < 5.4) {
      if (lefrTimingCbk && lefrTiming.lefiTiming::hasData())
        CALLBACK(lefrTimingCbk, lefrTimingCbkType, &lefrTiming);
    }
    lefDumbMode = 1000000000;
    lefrTiming.lefiTiming::clear();
    ;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 3145 "lef.y"
    { lefDumbMode = 0;;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 3146 "lef.y"
    {lefDumbMode = 1000000000;;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 3147 "lef.y"
    { lefDumbMode = 0;;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 3149 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::addRiseFall((yyvsp[(1) - (4)].string),(yyvsp[(3) - (4)].dval),(yyvsp[(4) - (4)].dval)); ;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 3151 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::addRiseFallVariable((yyvsp[(8) - (10)].dval),(yyvsp[(9) - (10)].dval)); ;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 3154 "lef.y"
    { if (lefrTimingCbk) {
	if ((yyvsp[(2) - (9)].string)[0] == 'D' || (yyvsp[(2) - (9)].string)[0] == 'd') /* delay */
	  lefrTiming.lefiTiming::addDelay((yyvsp[(1) - (9)].string), (yyvsp[(4) - (9)].string), (yyvsp[(6) - (9)].dval), (yyvsp[(7) - (9)].dval), (yyvsp[(8) - (9)].dval));
	else
	  lefrTiming.lefiTiming::addTransition((yyvsp[(1) - (9)].string), (yyvsp[(4) - (9)].string), (yyvsp[(6) - (9)].dval), (yyvsp[(7) - (9)].dval), (yyvsp[(8) - (9)].dval));
	}
    ;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 3162 "lef.y"
    { ;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 3164 "lef.y"
    { ;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 3166 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::setRiseRS((yyvsp[(2) - (4)].dval),(yyvsp[(3) - (4)].dval)); ;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 3168 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::setFallRS((yyvsp[(2) - (4)].dval),(yyvsp[(3) - (4)].dval)); ;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 3170 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::setRiseCS((yyvsp[(2) - (4)].dval),(yyvsp[(3) - (4)].dval)); ;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 3172 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::setFallCS((yyvsp[(2) - (4)].dval),(yyvsp[(3) - (4)].dval)); ;}
    break;

  case 638:

/* Line 1455 of yacc.c  */
#line 3174 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::setRiseAtt1((yyvsp[(2) - (4)].dval),(yyvsp[(3) - (4)].dval)); ;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 3176 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::setFallAtt1((yyvsp[(2) - (4)].dval),(yyvsp[(3) - (4)].dval)); ;}
    break;

  case 640:

/* Line 1455 of yacc.c  */
#line 3178 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::setRiseTo((yyvsp[(2) - (4)].dval),(yyvsp[(3) - (4)].dval)); ;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 3180 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::setFallTo((yyvsp[(2) - (4)].dval),(yyvsp[(3) - (4)].dval)); ;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 3182 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::addUnateness((yyvsp[(2) - (3)].string)); ;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 3184 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::setStable((yyvsp[(3) - (7)].dval),(yyvsp[(5) - (7)].dval),(yyvsp[(6) - (7)].string)); ;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 3186 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::addSDF2Pins((yyvsp[(1) - (8)].string),(yyvsp[(2) - (8)].string),(yyvsp[(3) - (8)].string),(yyvsp[(5) - (8)].dval),(yyvsp[(6) - (8)].dval),(yyvsp[(7) - (8)].dval)); ;}
    break;

  case 645:

/* Line 1455 of yacc.c  */
#line 3188 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::addSDF1Pin((yyvsp[(1) - (6)].string),(yyvsp[(3) - (6)].dval),(yyvsp[(4) - (6)].dval),(yyvsp[(4) - (6)].dval)); ;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 3190 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::setSDFcondStart((yyvsp[(2) - (3)].string)); ;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 3192 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::setSDFcondEnd((yyvsp[(2) - (3)].string)); ;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 3194 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::setSDFcond((yyvsp[(2) - (3)].string)); ;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 3196 "lef.y"
    { /* XXXXX */ ;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 3200 "lef.y"
    { (yyval.string) = (char*)"MPWH";;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 3202 "lef.y"
    { (yyval.string) = (char*)"MPWL";;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 3204 "lef.y"
    { (yyval.string) = (char*)"PERIOD";;}
    break;

  case 653:

/* Line 1455 of yacc.c  */
#line 3208 "lef.y"
    { (yyval.string) = (char*)"SETUP";;}
    break;

  case 654:

/* Line 1455 of yacc.c  */
#line 3210 "lef.y"
    { (yyval.string) = (char*)"HOLD";;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 3212 "lef.y"
    { (yyval.string) = (char*)"RECOVERY";;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 3214 "lef.y"
    { (yyval.string) = (char*)"SKEW";;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 3218 "lef.y"
    { (yyval.string) = (char*)"ANYEDGE";;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 3220 "lef.y"
    { (yyval.string) = (char*)"POSEDGE";;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 3222 "lef.y"
    { (yyval.string) = (char*)"NEGEDGE";;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 3226 "lef.y"
    { (yyval.string) = (char*)"ANYEDGE";;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 3228 "lef.y"
    { (yyval.string) = (char*)"POSEDGE";;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 3230 "lef.y"
    { (yyval.string) = (char*)"NEGEDGE";;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 3234 "lef.y"
    { (yyval.string) = (char*)"DELAY"; ;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 3236 "lef.y"
    { (yyval.string) = (char*)"TRANSITION"; ;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 3240 "lef.y"
    { ;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 3242 "lef.y"
    { ;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 3245 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::addTableEntry((yyvsp[(2) - (5)].dval),(yyvsp[(3) - (5)].dval),(yyvsp[(4) - (5)].dval)); ;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 3249 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::addTableAxisNumber((yyvsp[(1) - (1)].dval)); ;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 3251 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::addTableAxisNumber((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 670:

/* Line 1455 of yacc.c  */
#line 3255 "lef.y"
    { ;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 3257 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::addRiseFallSlew((yyvsp[(1) - (4)].dval),(yyvsp[(2) - (4)].dval),(yyvsp[(3) - (4)].dval),(yyvsp[(4) - (4)].dval)); ;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 3259 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::addRiseFallSlew((yyvsp[(1) - (7)].dval),(yyvsp[(2) - (7)].dval),(yyvsp[(3) - (7)].dval),(yyvsp[(4) - (7)].dval));
      if (lefrTimingCbk) lefrTiming.lefiTiming::addRiseFallSlew2((yyvsp[(5) - (7)].dval),(yyvsp[(6) - (7)].dval),(yyvsp[(7) - (7)].dval)); ;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 3264 "lef.y"
    { (yyval.string) = (char*)"RISE"; ;}
    break;

  case 674:

/* Line 1455 of yacc.c  */
#line 3266 "lef.y"
    { (yyval.string) = (char*)"FALL"; ;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 3270 "lef.y"
    { (yyval.string) = (char*)"INVERT"; ;}
    break;

  case 676:

/* Line 1455 of yacc.c  */
#line 3272 "lef.y"
    { (yyval.string) = (char*)"NONINVERT"; ;}
    break;

  case 677:

/* Line 1455 of yacc.c  */
#line 3274 "lef.y"
    { (yyval.string) = (char*)"NONUNATE"; ;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 3278 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::addFromPin((yyvsp[(1) - (1)].string)); ;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 3280 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::addFromPin((yyvsp[(2) - (2)].string)); ;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 3284 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::addToPin((yyvsp[(1) - (1)].string)); ;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 3286 "lef.y"
    { if (lefrTimingCbk) lefrTiming.lefiTiming::addToPin((yyvsp[(2) - (2)].string)); ;}
    break;

  case 682:

/* Line 1455 of yacc.c  */
#line 3289 "lef.y"
    {
	CALLBACK(lefrArrayCbk, lefrArrayCbkType, &lefrArray);
	lefrArray.lefiArray::clear();
	lefrSitePatternPtr = 0;
	lefrDoSite = 0;
    ;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 3297 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1;;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 3298 "lef.y"
    {
      if (lefrArrayCbk) {
	lefrArray.lefiArray::setName((yyvsp[(3) - (3)].string));
	CALLBACK(lefrArrayBeginCbk, lefrArrayBeginCbkType, (yyvsp[(3) - (3)].string));
      }
      strcpy(arrayName, (yyvsp[(3) - (3)].string));
    ;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 3306 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1;;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 3307 "lef.y"
    {
      if (lefrArrayCbk)
	CALLBACK(lefrArrayEndCbk, lefrArrayEndCbkType, (yyvsp[(3) - (3)].string));
      if (strcmp(arrayName, (yyvsp[(3) - (3)].string)) != 0) {
        yyerror("END ARRAY name is different from ARRAY");
        CHKERR();
      }
    ;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 3318 "lef.y"
    { ;}
    break;

  case 689:

/* Line 1455 of yacc.c  */
#line 3320 "lef.y"
    { ;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 3323 "lef.y"
    { if (lefrArrayCbk) lefrDoSite = 1; lefDumbMode = 1; ;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 3325 "lef.y"
    {
      if (lefrArrayCbk) {
	lefrArray.lefiArray::addSitePattern(lefrSitePatternPtr);
      }
    ;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 3330 "lef.y"
    {lefDumbMode = 1; if (lefrArrayCbk) lefrDoSite = 1; ;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 3332 "lef.y"
    {
      if (lefrArrayCbk) {
	lefrArray.lefiArray::addCanPlace(lefrSitePatternPtr);
      }
    ;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 3337 "lef.y"
    {lefDumbMode = 1; if (lefrArrayCbk) lefrDoSite = 1; ;}
    break;

  case 695:

/* Line 1455 of yacc.c  */
#line 3339 "lef.y"
    {
      if (lefrArrayCbk) {
	lefrArray.lefiArray::addCannotOccupy(lefrSitePatternPtr);
      }
    ;}
    break;

  case 696:

/* Line 1455 of yacc.c  */
#line 3344 "lef.y"
    { if (lefrArrayCbk) lefrDoTrack = 1; ;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 3345 "lef.y"
    {
      if (lefrArrayCbk) {
	lefrArray.lefiArray::addTrack(lefrTrackPatternPtr);
      }
    ;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 3351 "lef.y"
    {
    ;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 3353 "lef.y"
    { if (lefrArrayCbk) lefrDoGcell = 1; ;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 3354 "lef.y"
    {
      if (lefrArrayCbk) {
	lefrArray.lefiArray::addGcell(lefrGcellPatternPtr);
      }
    ;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 3360 "lef.y"
    {
      if (lefrArrayCbk) {
	lefrArray.lefiArray::setTableSize((int)(yyvsp[(2) - (5)].dval));
      }
    ;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 3366 "lef.y"
    { ;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 3369 "lef.y"
    { if (lefrArrayCbk) lefrArray.lefiArray::addFloorPlan((yyvsp[(2) - (2)].string)); ;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 3373 "lef.y"
    { ;}
    break;

  case 705:

/* Line 1455 of yacc.c  */
#line 3375 "lef.y"
    { ;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 3378 "lef.y"
    { lefDumbMode = 1; if (lefrArrayCbk) lefrDoSite = 1; ;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 3380 "lef.y"
    {
      if (lefrArrayCbk)
	lefrArray.lefiArray::addSiteToFloorPlan("CANPLACE",
	lefrSitePatternPtr);
    ;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 3385 "lef.y"
    { if (lefrArrayCbk) lefrDoSite = 1; lefDumbMode = 1; ;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 3387 "lef.y"
    {
      if (lefrArrayCbk)
	lefrArray.lefiArray::addSiteToFloorPlan("CANNOTOCCUPY",
	lefrSitePatternPtr);
     ;}
    break;

  case 710:

/* Line 1455 of yacc.c  */
#line 3395 "lef.y"
    { ;}
    break;

  case 711:

/* Line 1455 of yacc.c  */
#line 3397 "lef.y"
    { ;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 3400 "lef.y"
    { if (lefrArrayCbk) lefrArray.lefiArray::addDefaultCap((int)(yyvsp[(2) - (5)].dval), (yyvsp[(4) - (5)].dval)); ;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 3403 "lef.y"
    {lefDumbMode=1;lefNlToken=TRUE;;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 3404 "lef.y"
    { lefAddStringMessage((yyvsp[(3) - (6)].string), (yyvsp[(5) - (6)].string)); ;}
    break;

  case 715:

/* Line 1455 of yacc.c  */
#line 3407 "lef.y"
    {lefDumbMode=1;lefNlToken=TRUE;;}
    break;

  case 716:

/* Line 1455 of yacc.c  */
#line 3408 "lef.y"
    { ;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 3411 "lef.y"
    {lefDumbMode=1;lefNlToken=TRUE;;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 3412 "lef.y"
    { lefAddNumDefine((yyvsp[(3) - (6)].string), (yyvsp[(5) - (6)].dval)); ;}
    break;

  case 719:

/* Line 1455 of yacc.c  */
#line 3413 "lef.y"
    {lefDumbMode=1;lefNlToken=TRUE;;}
    break;

  case 720:

/* Line 1455 of yacc.c  */
#line 3414 "lef.y"
    { lefAddStringDefine((yyvsp[(3) - (6)].string), (yyvsp[(5) - (6)].string)); ;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 3415 "lef.y"
    {lefDumbMode=1;lefNlToken=TRUE;;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 3416 "lef.y"
    { lefAddBooleanDefine((yyvsp[(3) - (6)].string), (yyvsp[(5) - (6)].integer)); ;}
    break;

  case 724:

/* Line 1455 of yacc.c  */
#line 3420 "lef.y"
    {lefNlToken = FALSE;;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 3421 "lef.y"
    {lefNlToken = FALSE;;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 3434 "lef.y"
    {(yyval.dval) = (yyvsp[(1) - (3)].dval) + (yyvsp[(3) - (3)].dval); ;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 3435 "lef.y"
    {(yyval.dval) = (yyvsp[(1) - (3)].dval) - (yyvsp[(3) - (3)].dval); ;}
    break;

  case 732:

/* Line 1455 of yacc.c  */
#line 3436 "lef.y"
    {(yyval.dval) = (yyvsp[(1) - (3)].dval) * (yyvsp[(3) - (3)].dval); ;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 3437 "lef.y"
    {(yyval.dval) = (yyvsp[(1) - (3)].dval) / (yyvsp[(3) - (3)].dval); ;}
    break;

  case 734:

/* Line 1455 of yacc.c  */
#line 3438 "lef.y"
    {(yyval.dval) = -(yyvsp[(2) - (2)].dval);;}
    break;

  case 735:

/* Line 1455 of yacc.c  */
#line 3439 "lef.y"
    {(yyval.dval) = (yyvsp[(2) - (3)].dval);;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 3441 "lef.y"
    {(yyval.dval) = ((yyvsp[(2) - (6)].integer) != 0) ? (yyvsp[(4) - (6)].dval) : (yyvsp[(6) - (6)].dval);;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 3442 "lef.y"
    {(yyval.dval) = (yyvsp[(1) - (1)].dval);;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 3445 "lef.y"
    {(yyval.integer) = comp_num((yyvsp[(1) - (3)].dval),(yyvsp[(2) - (3)].integer),(yyvsp[(3) - (3)].dval));;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 3446 "lef.y"
    {(yyval.integer) = (yyvsp[(1) - (3)].dval) != 0 && (yyvsp[(3) - (3)].dval) != 0;;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 3447 "lef.y"
    {(yyval.integer) = (yyvsp[(1) - (3)].dval) != 0 || (yyvsp[(3) - (3)].dval) != 0;;}
    break;

  case 741:

/* Line 1455 of yacc.c  */
#line 3448 "lef.y"
    {(yyval.integer) = comp_str((yyvsp[(1) - (3)].string),(yyvsp[(2) - (3)].integer),(yyvsp[(3) - (3)].string));;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 3449 "lef.y"
    {(yyval.integer) = (yyvsp[(1) - (3)].string)[0] != 0 && (yyvsp[(3) - (3)].string)[0] != 0;;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 3450 "lef.y"
    {(yyval.integer) = (yyvsp[(1) - (3)].string)[0] != 0 || (yyvsp[(3) - (3)].string)[0] != 0;;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 3451 "lef.y"
    {(yyval.integer) = (yyvsp[(1) - (3)].integer) == (yyvsp[(3) - (3)].integer);;}
    break;

  case 745:

/* Line 1455 of yacc.c  */
#line 3452 "lef.y"
    {(yyval.integer) = (yyvsp[(1) - (3)].integer) != (yyvsp[(3) - (3)].integer);;}
    break;

  case 746:

/* Line 1455 of yacc.c  */
#line 3453 "lef.y"
    {(yyval.integer) = (yyvsp[(1) - (3)].integer) && (yyvsp[(3) - (3)].integer);;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 3454 "lef.y"
    {(yyval.integer) = (yyvsp[(1) - (3)].integer) || (yyvsp[(3) - (3)].integer);;}
    break;

  case 748:

/* Line 1455 of yacc.c  */
#line 3455 "lef.y"
    {(yyval.integer) = !(yyval.integer);;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 3456 "lef.y"
    {(yyval.integer) = (yyvsp[(2) - (3)].integer);;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 3458 "lef.y"
    {(yyval.integer) = ((yyvsp[(2) - (6)].integer) != 0) ? (yyvsp[(4) - (6)].integer) : (yyvsp[(6) - (6)].integer);;}
    break;

  case 751:

/* Line 1455 of yacc.c  */
#line 3459 "lef.y"
    {(yyval.integer) = 1;;}
    break;

  case 752:

/* Line 1455 of yacc.c  */
#line 3460 "lef.y"
    {(yyval.integer) = 0;;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 3464 "lef.y"
    {
      (yyval.string) = (char*)lefMalloc(strlen((yyvsp[(1) - (3)].string))+strlen((yyvsp[(3) - (3)].string))+1);
      strcpy((yyval.string),(yyvsp[(1) - (3)].string));
      strcat((yyval.string),(yyvsp[(3) - (3)].string));
    ;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 3470 "lef.y"
    { (yyval.string) = (yyvsp[(2) - (3)].string); ;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 3472 "lef.y"
    {
      lefDefIf = TRUE;
      if ((yyvsp[(2) - (6)].integer) != 0) {
	(yyval.string) = (yyvsp[(4) - (6)].string);	
      } else {
	(yyval.string) = (yyvsp[(6) - (6)].string);
      }
    ;}
    break;

  case 756:

/* Line 1455 of yacc.c  */
#line 3481 "lef.y"
    { (yyval.string) = (yyvsp[(1) - (1)].string); ;}
    break;

  case 757:

/* Line 1455 of yacc.c  */
#line 3484 "lef.y"
    {(yyval.integer) = C_LE;;}
    break;

  case 758:

/* Line 1455 of yacc.c  */
#line 3485 "lef.y"
    {(yyval.integer) = C_LT;;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 3486 "lef.y"
    {(yyval.integer) = C_GE;;}
    break;

  case 760:

/* Line 1455 of yacc.c  */
#line 3487 "lef.y"
    {(yyval.integer) = C_GT;;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 3488 "lef.y"
    {(yyval.integer) = C_EQ;;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 3489 "lef.y"
    {(yyval.integer) = C_NE;;}
    break;

  case 763:

/* Line 1455 of yacc.c  */
#line 3490 "lef.y"
    {(yyval.integer) = C_EQ;;}
    break;

  case 764:

/* Line 1455 of yacc.c  */
#line 3491 "lef.y"
    {(yyval.integer) = C_LT;;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 3492 "lef.y"
    {(yyval.integer) = C_GT;;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 3496 "lef.y"
    { CALLBACK(lefrPropBeginCbk, lefrPropBeginCbkType, 0); ;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 3498 "lef.y"
    { 
      CALLBACK(lefrPropEndCbk, lefrPropEndCbkType, 0);
      lefRealNum = 0;     /* just want to make sure it is reset */
    ;}
    break;

  case 768:

/* Line 1455 of yacc.c  */
#line 3505 "lef.y"
    { ;}
    break;

  case 769:

/* Line 1455 of yacc.c  */
#line 3507 "lef.y"
    { ;}
    break;

  case 770:

/* Line 1455 of yacc.c  */
#line 3510 "lef.y"
    {lefDumbMode = 1; lefrProp.lefiProp::clear(); ;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 3512 "lef.y"
    { 
      if (lefrPropCbk) lefrProp.lefiProp::setPropType("library", (yyvsp[(3) - (5)].string));
      CALLBACK(lefrPropCbk, lefrPropCbkType, &lefrProp);
      lefrLibProp.lefiPropType::setPropType((yyvsp[(3) - (5)].string), lefPropDefType);
    ;}
    break;

  case 772:

/* Line 1455 of yacc.c  */
#line 3517 "lef.y"
    {lefDumbMode = 1; lefrProp.lefiProp::clear(); ;}
    break;

  case 773:

/* Line 1455 of yacc.c  */
#line 3519 "lef.y"
    { 
      if (lefrPropCbk) lefrProp.lefiProp::setPropType("componentpin", (yyvsp[(3) - (5)].string));
      CALLBACK(lefrPropCbk, lefrPropCbkType, &lefrProp);
      lefrCompProp.lefiPropType::setPropType((yyvsp[(3) - (5)].string), lefPropDefType);
    ;}
    break;

  case 774:

/* Line 1455 of yacc.c  */
#line 3524 "lef.y"
    {lefDumbMode = 1; lefrProp.lefiProp::clear(); ;}
    break;

  case 775:

/* Line 1455 of yacc.c  */
#line 3526 "lef.y"
    { 
      if (lefrPropCbk) lefrProp.lefiProp::setPropType("pin", (yyvsp[(3) - (5)].string));
      CALLBACK(lefrPropCbk, lefrPropCbkType, &lefrProp);
      lefrPinProp.lefiPropType::setPropType((yyvsp[(3) - (5)].string), lefPropDefType);
      
    ;}
    break;

  case 776:

/* Line 1455 of yacc.c  */
#line 3532 "lef.y"
    {lefDumbMode = 1; lefrProp.lefiProp::clear(); ;}
    break;

  case 777:

/* Line 1455 of yacc.c  */
#line 3534 "lef.y"
    { 
      if (lefrPropCbk) lefrProp.lefiProp::setPropType("macro", (yyvsp[(3) - (5)].string));
      CALLBACK(lefrPropCbk, lefrPropCbkType, &lefrProp);
      lefrMacroProp.lefiPropType::setPropType((yyvsp[(3) - (5)].string), lefPropDefType);
    ;}
    break;

  case 778:

/* Line 1455 of yacc.c  */
#line 3539 "lef.y"
    {lefDumbMode = 1; lefrProp.lefiProp::clear(); ;}
    break;

  case 779:

/* Line 1455 of yacc.c  */
#line 3541 "lef.y"
    { 
      if (lefrPropCbk) lefrProp.lefiProp::setPropType("via", (yyvsp[(3) - (5)].string));
      CALLBACK(lefrPropCbk, lefrPropCbkType, &lefrProp);
      lefrViaProp.lefiPropType::setPropType((yyvsp[(3) - (5)].string), lefPropDefType);
    ;}
    break;

  case 780:

/* Line 1455 of yacc.c  */
#line 3546 "lef.y"
    {lefDumbMode = 1; lefrProp.lefiProp::clear(); ;}
    break;

  case 781:

/* Line 1455 of yacc.c  */
#line 3548 "lef.y"
    { 
      if (lefrPropCbk) lefrProp.lefiProp::setPropType("viarule", (yyvsp[(3) - (5)].string));
      CALLBACK(lefrPropCbk, lefrPropCbkType, &lefrProp);
      lefrViaRuleProp.lefiPropType::setPropType((yyvsp[(3) - (5)].string), lefPropDefType);
    ;}
    break;

  case 782:

/* Line 1455 of yacc.c  */
#line 3553 "lef.y"
    {lefDumbMode = 1; lefrProp.lefiProp::clear(); ;}
    break;

  case 783:

/* Line 1455 of yacc.c  */
#line 3555 "lef.y"
    { 
      if (lefrPropCbk) lefrProp.lefiProp::setPropType("layer", (yyvsp[(3) - (5)].string));
      CALLBACK(lefrPropCbk, lefrPropCbkType, &lefrProp);
      lefrLayerProp.lefiPropType::setPropType((yyvsp[(3) - (5)].string), lefPropDefType);
    ;}
    break;

  case 784:

/* Line 1455 of yacc.c  */
#line 3560 "lef.y"
    {lefDumbMode = 1; lefrProp.lefiProp::clear(); ;}
    break;

  case 785:

/* Line 1455 of yacc.c  */
#line 3562 "lef.y"
    { 
      if (lefrPropCbk) lefrProp.lefiProp::setPropType("nondefaultrule", (yyvsp[(3) - (5)].string));
      CALLBACK(lefrPropCbk, lefrPropCbkType, &lefrProp);
      lefrNondefProp.lefiPropType::setPropType((yyvsp[(3) - (5)].string), lefPropDefType);
    ;}
    break;

  case 786:

/* Line 1455 of yacc.c  */
#line 3569 "lef.y"
    { lefRealNum = 0 ;}
    break;

  case 787:

/* Line 1455 of yacc.c  */
#line 3570 "lef.y"
    { 
      if (lefrPropCbk) lefrProp.lefiProp::setPropInteger();
      lefPropDefType = 'I'
    ;}
    break;

  case 788:

/* Line 1455 of yacc.c  */
#line 3574 "lef.y"
    { lefRealNum = 1 ;}
    break;

  case 789:

/* Line 1455 of yacc.c  */
#line 3575 "lef.y"
    { 
      if (lefrPropCbk) lefrProp.lefiProp::setPropReal();
      lefPropDefType = 'R';
      lefRealNum = 0;
    ;}
    break;

  case 790:

/* Line 1455 of yacc.c  */
#line 3581 "lef.y"
    {
      if (lefrPropCbk) lefrProp.lefiProp::setPropString();
      lefPropDefType = 'S'
    ;}
    break;

  case 791:

/* Line 1455 of yacc.c  */
#line 3586 "lef.y"
    {
      if (lefrPropCbk) lefrProp.lefiProp::setPropQString((yyvsp[(2) - (2)].string));
      lefPropDefType = 'Q'
    ;}
    break;

  case 792:

/* Line 1455 of yacc.c  */
#line 3591 "lef.y"
    {
      if (lefrPropCbk) lefrProp.lefiProp::setPropNameMapString((yyvsp[(2) - (2)].string));
      lefPropDefType = 'S'
    ;}
    break;

  case 793:

/* Line 1455 of yacc.c  */
#line 3599 "lef.y"
    { 
    ;}
    break;

  case 794:

/* Line 1455 of yacc.c  */
#line 3602 "lef.y"
    { 
      if (lefrLayerCbk)
        lefrLayer.lefiLayer::setSpacingRange((yyvsp[(2) - (3)].dval), (yyvsp[(3) - (3)].dval));
    ;}
    break;

  case 796:

/* Line 1455 of yacc.c  */
#line 3608 "lef.y"
    {
      if (lefrLayerCbk) {
        lefrLayer.lefiLayer::setSpacingLength((yyvsp[(2) - (2)].dval));
      }
    ;}
    break;

  case 797:

/* Line 1455 of yacc.c  */
#line 3614 "lef.y"
    {
      if (lefrLayerCbk) {
        lefrLayer.lefiLayer::setSpacingLength((yyvsp[(2) - (5)].dval));
        lefrLayer.lefiLayer::setSpacingLengthRange((yyvsp[(4) - (5)].dval), (yyvsp[(5) - (5)].dval));
      }
    ;}
    break;

  case 798:

/* Line 1455 of yacc.c  */
#line 3623 "lef.y"
    { ;}
    break;

  case 799:

/* Line 1455 of yacc.c  */
#line 3625 "lef.y"
    {
      if (lefrLayerCbk)
        lefrLayer.lefiLayer::setSpacingRangeUseLength();
    ;}
    break;

  case 800:

/* Line 1455 of yacc.c  */
#line 3630 "lef.y"
    {
      if (lefrLayerCbk) {
        lefrLayer.lefiLayer::setSpacingRangeInfluence((yyvsp[(2) - (2)].dval));
        lefrLayer.lefiLayer::setSpacingRangeInfluenceRange(-1, -1);
      }
    ;}
    break;

  case 801:

/* Line 1455 of yacc.c  */
#line 3637 "lef.y"
    {
      if (lefrLayerCbk) {
        lefrLayer.lefiLayer::setSpacingRangeInfluence((yyvsp[(2) - (5)].dval));
        lefrLayer.lefiLayer::setSpacingRangeInfluenceRange((yyvsp[(4) - (5)].dval), (yyvsp[(5) - (5)].dval));
      }
    ;}
    break;

  case 802:

/* Line 1455 of yacc.c  */
#line 3644 "lef.y"
    {
      if (lefrLayerCbk)
        lefrLayer.lefiLayer::setSpacingRangeRange((yyvsp[(2) - (3)].dval), (yyvsp[(3) - (3)].dval));
    ;}
    break;

  case 803:

/* Line 1455 of yacc.c  */
#line 3651 "lef.y"
    { ;}
    break;

  case 804:

/* Line 1455 of yacc.c  */
#line 3653 "lef.y"
    { lefrProp.lefiProp::setRange((yyvsp[(2) - (3)].dval), (yyvsp[(3) - (3)].dval)); ;}
    break;

  case 805:

/* Line 1455 of yacc.c  */
#line 3657 "lef.y"
    { ;}
    break;

  case 806:

/* Line 1455 of yacc.c  */
#line 3659 "lef.y"
    { if (lefrPropCbk) lefrProp.lefiProp::setNumber((yyvsp[(1) - (1)].dval)); ;}
    break;

  case 808:

/* Line 1455 of yacc.c  */
#line 3663 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1; ;}
    break;

  case 809:

/* Line 1455 of yacc.c  */
#line 3664 "lef.y"
    {
      if (lefrLayerCbk)
        lefrLayer.lefiLayer::setSpacingName((yyvsp[(3) - (3)].string));
    ;}
    break;

  case 810:

/* Line 1455 of yacc.c  */
#line 3669 "lef.y"
    {
      if (versionNum < 5.5) {
         yyerror("ADJACENTCUTS is a 5.5 syntax. Your lef file is not defined as 5.5");
         CHKERR();
      }
      if (lefrLayerCbk)
        lefrLayer.lefiLayer::setSpacingAdjacent((yyvsp[(2) - (4)].dval), (yyvsp[(4) - (4)].dval));
    ;}
    break;

  case 811:

/* Line 1455 of yacc.c  */
#line 3680 "lef.y"
    { (yyval.string) = 0; ;}
    break;

  case 812:

/* Line 1455 of yacc.c  */
#line 3681 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1; ;}
    break;

  case 813:

/* Line 1455 of yacc.c  */
#line 3682 "lef.y"
    { (yyval.string) = (yyvsp[(3) - (3)].string); ;}
    break;

  case 814:

/* Line 1455 of yacc.c  */
#line 3686 "lef.y"
    {lefDumbMode = 1; lefNoNum = 1; ;}
    break;

  case 815:

/* Line 1455 of yacc.c  */
#line 3687 "lef.y"
    { (yyval.string) = (yyvsp[(3) - (3)].string); ;}
    break;

  case 816:

/* Line 1455 of yacc.c  */
#line 3691 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrNoiseMarginCbk) {
          lefrNoiseMargin.low = (yyvsp[(2) - (4)].dval);
          lefrNoiseMargin.high = (yyvsp[(3) - (4)].dval);
        }
        CALLBACK(lefrNoiseMarginCbk, lefrNoiseMarginCbkType, &lefrNoiseMargin);
      } else
        yywarning(
          "UNIVERSALNOISEMARGIN is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 817:

/* Line 1455 of yacc.c  */
#line 3704 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrEdgeRateThreshold1Cbk) {
          CALLBACK(lefrEdgeRateThreshold1Cbk,
          lefrEdgeRateThreshold1CbkType, (yyvsp[(2) - (3)].dval));
        }
      } else
        yywarning("EDGERATETHRESHOLD1 is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 818:

/* Line 1455 of yacc.c  */
#line 3715 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrEdgeRateThreshold2Cbk) {
	  CALLBACK(lefrEdgeRateThreshold2Cbk,
	  lefrEdgeRateThreshold2CbkType, (yyvsp[(2) - (3)].dval));
        }
      } else
        yywarning("EDGERATETHRESHOLD2 is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 819:

/* Line 1455 of yacc.c  */
#line 3726 "lef.y"
    {
      if (versionNum < 5.4) {
        if (lefrEdgeRateScaleFactorCbk) {
	  CALLBACK(lefrEdgeRateScaleFactorCbk,
	  lefrEdgeRateScaleFactorCbkType, (yyvsp[(2) - (3)].dval));
        }
      } else
        yywarning(
          "EDGERATESCALEFACTOR is obsolete in 5.4. It will be ignored.");
    ;}
    break;

  case 820:

/* Line 1455 of yacc.c  */
#line 3738 "lef.y"
    { if (lefrNoiseTableCbk) lefrNoiseTable.lefiNoiseTable::setup((int)(yyvsp[(2) - (2)].dval)); ;}
    break;

  case 821:

/* Line 1455 of yacc.c  */
#line 3740 "lef.y"
    { ;}
    break;

  case 822:

/* Line 1455 of yacc.c  */
#line 3744 "lef.y"
    {
    if (versionNum < 5.4) {
      CALLBACK(lefrNoiseTableCbk, lefrNoiseTableCbkType, &lefrNoiseTable);
    } else
      yywarning("NOISETABLE is obsolete in 5.4. It will be ignored.");
  ;}
    break;

  case 825:

/* Line 1455 of yacc.c  */
#line 3759 "lef.y"
    { if (lefrNoiseTableCbk)
         {
            lefrNoiseTable.lefiNoiseTable::newEdge();
            lefrNoiseTable.lefiNoiseTable::addEdge((yyvsp[(2) - (3)].dval));
         }
    ;}
    break;

  case 826:

/* Line 1455 of yacc.c  */
#line 3766 "lef.y"
    { ;}
    break;

  case 827:

/* Line 1455 of yacc.c  */
#line 3769 "lef.y"
    { if (lefrNoiseTableCbk) lefrNoiseTable.lefiNoiseTable::addResistance(); ;}
    break;

  case 829:

/* Line 1455 of yacc.c  */
#line 3775 "lef.y"
    { if (lefrNoiseTableCbk)
    lefrNoiseTable.lefiNoiseTable::addResistanceNumber((yyvsp[(1) - (1)].dval)); ;}
    break;

  case 830:

/* Line 1455 of yacc.c  */
#line 3778 "lef.y"
    { if (lefrNoiseTableCbk)
    lefrNoiseTable.lefiNoiseTable::addResistanceNumber((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 833:

/* Line 1455 of yacc.c  */
#line 3787 "lef.y"
    { if (lefrNoiseTableCbk)
	lefrNoiseTable.lefiNoiseTable::addVictimLength((yyvsp[(2) - (3)].dval)); ;}
    break;

  case 834:

/* Line 1455 of yacc.c  */
#line 3790 "lef.y"
    { ;}
    break;

  case 835:

/* Line 1455 of yacc.c  */
#line 3794 "lef.y"
    { if (lefrNoiseTableCbk)
    lefrNoiseTable.lefiNoiseTable::addVictimNoise((yyvsp[(1) - (1)].dval)); ;}
    break;

  case 836:

/* Line 1455 of yacc.c  */
#line 3797 "lef.y"
    { if (lefrNoiseTableCbk)
    lefrNoiseTable.lefiNoiseTable::addVictimNoise((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 837:

/* Line 1455 of yacc.c  */
#line 3801 "lef.y"
    { if (lefrCorrectionTableCbk)
    lefrCorrectionTable.lefiCorrectionTable::setup((int)(yyvsp[(2) - (3)].dval)); ;}
    break;

  case 838:

/* Line 1455 of yacc.c  */
#line 3804 "lef.y"
    { ;}
    break;

  case 839:

/* Line 1455 of yacc.c  */
#line 3808 "lef.y"
    {
    if (versionNum < 5.4) {
      CALLBACK(lefrCorrectionTableCbk, lefrCorrectionTableCbkType,
               &lefrCorrectionTable);
    } else
      yywarning("CORRECTIONTABLE is obsolete in 5.4. It will be ignored.");
  ;}
    break;

  case 842:

/* Line 1455 of yacc.c  */
#line 3823 "lef.y"
    { if (lefrCorrectionTableCbk)
         {
            lefrCorrectionTable.lefiCorrectionTable::newEdge();
            lefrCorrectionTable.lefiCorrectionTable::addEdge((yyvsp[(2) - (3)].dval));
         }
    ;}
    break;

  case 843:

/* Line 1455 of yacc.c  */
#line 3830 "lef.y"
    { ;}
    break;

  case 844:

/* Line 1455 of yacc.c  */
#line 3833 "lef.y"
    { if (lefrCorrectionTableCbk)
  lefrCorrectionTable.lefiCorrectionTable::addResistance(); ;}
    break;

  case 845:

/* Line 1455 of yacc.c  */
#line 3836 "lef.y"
    { ;}
    break;

  case 846:

/* Line 1455 of yacc.c  */
#line 3840 "lef.y"
    { if (lefrCorrectionTableCbk)
    lefrCorrectionTable.lefiCorrectionTable::addResistanceNumber((yyvsp[(1) - (1)].dval)); ;}
    break;

  case 847:

/* Line 1455 of yacc.c  */
#line 3843 "lef.y"
    { if (lefrCorrectionTableCbk)
    lefrCorrectionTable.lefiCorrectionTable::addResistanceNumber((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 850:

/* Line 1455 of yacc.c  */
#line 3853 "lef.y"
    { if (lefrCorrectionTableCbk)
     lefrCorrectionTable.lefiCorrectionTable::addVictimLength((yyvsp[(2) - (3)].dval)); ;}
    break;

  case 851:

/* Line 1455 of yacc.c  */
#line 3856 "lef.y"
    { ;}
    break;

  case 852:

/* Line 1455 of yacc.c  */
#line 3860 "lef.y"
    { if (lefrCorrectionTableCbk)
	lefrCorrectionTable.lefiCorrectionTable::addVictimCorrection((yyvsp[(1) - (1)].dval)); ;}
    break;

  case 853:

/* Line 1455 of yacc.c  */
#line 3863 "lef.y"
    { if (lefrCorrectionTableCbk)
	lefrCorrectionTable.lefiCorrectionTable::addVictimCorrection((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 854:

/* Line 1455 of yacc.c  */
#line 3869 "lef.y"
    { /* 5.3 syntax */
        use5_3 = 1;
        if (ignoreVersion) {
           /* do nothing */
        } else if (versionNum > 5.3) {
           /* A 5.3 syntax in 5.4 */
           if (use5_4) {
              yyerror("INPUTPINANTENNASIZE is a 5.3 or earlier syntax.  Your lef file has both old and new syntax, which is illegal.");
              CHKERR();
           }
        }
        CALLBACK(lefrInputAntennaCbk, lefrInputAntennaCbkType, (yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 855:

/* Line 1455 of yacc.c  */
#line 3884 "lef.y"
    { /* 5.3 syntax */
        use5_3 = 1;
        if (ignoreVersion) {
           /* do nothing */
        } else if (versionNum > 5.3) {
           /* A 5.3 syntax in 5.4 */
           if (use5_4) {
              yyerror("OUTPUTPINANTENNASIZE is a 5.3 or earlier syntax.  Your lef file has both old and new syntax, which is illegal.");
              CHKERR();
           }
        }
        CALLBACK(lefrOutputAntennaCbk, lefrOutputAntennaCbkType, (yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 856:

/* Line 1455 of yacc.c  */
#line 3899 "lef.y"
    { /* 5.3 syntax */
        use5_3 = 1;
        if (ignoreVersion) {
           /* do nothing */
        } else if (versionNum > 5.3) {
           /* A 5.3 syntax in 5.4 */
           if (use5_4) {
              yyerror("INOUTPINANTENNASIZE is a 5.3 or earlier syntax.  Your lef file has both old and new syntax, which is illegal.");
              CHKERR();
           }
        }
        CALLBACK(lefrInoutAntennaCbk, lefrInoutAntennaCbkType, (yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 857:

/* Line 1455 of yacc.c  */
#line 3914 "lef.y"
    { /* 5.4 syntax */
        /* 11/12/2002 - this is obsoleted in 5.5, suppose should be ingored */
        /* 12/16/2002 - talked to Dave Noice, leave them in here for debugging*/
        use5_4 = 1;
        if (ignoreVersion) {
           /* do nothing */
        } else if (versionNum < 5.4) {
           yyerror("ANTENNAINPUTGATEAREA is a 5.4 syntax. Please either update your lef VERSION number or use the 5.3 syntax.");
           CHKERR();
        } else if (use5_3) {
           yyerror("ANTENNAINPUTGATEAREA is a 5.4 syntax.  Your lef file has both old and new syntax, which is illegal.");
           CHKERR();
        }
        CALLBACK(lefrAntennaInputCbk, lefrAntennaInputCbkType, (yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 858:

/* Line 1455 of yacc.c  */
#line 3931 "lef.y"
    { /* 5.4 syntax */
        /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
        /* 12/16/2002 - talked to Dave Noice, leave them in here for debugging*/
        use5_4 = 1;
        if (ignoreVersion) {
           /* do nothing */
        } else if (versionNum < 5.4) {
              yyerror("ANTENNAINOUTDIFFAREA is a 5.4 syntax.  Please either update your lef VERSION number or use the 5.3 syntax.");
              CHKERR();
        } else if (use5_3) {
           yyerror("ANTENNAINOUTDIFFAREA is a 5.4 syntax.  Your lef file has both old and new syntax, which is illegal.");
           CHKERR();
        }
        CALLBACK(lefrAntennaInoutCbk, lefrAntennaInoutCbkType, (yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 859:

/* Line 1455 of yacc.c  */
#line 3948 "lef.y"
    { /* 5.4 syntax */
        /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
        /* 12/16/2002 - talked to Dave Noice, leave them in here for debugging*/
        use5_4 = 1;
        if (ignoreVersion) {
           /* do nothing */
        } else if (versionNum < 5.4) {
              yyerror("ANTENNAOUTPUTDIFFAREA is a 5.4 syntax.  Please either update your lef VERSION number or use the 5.3 syntax.");
              CHKERR();
        } else if (use5_3) {
           yyerror("ANTENNAOUTPUTDIFFAREA is a 5.4 syntax.  Your lef file has both old and new syntax, which is illegal.");
           CHKERR();
        }
        CALLBACK(lefrAntennaOutputCbk, lefrAntennaOutputCbkType, (yyvsp[(2) - (3)].dval));
    ;}
    break;

  case 860:

/* Line 1455 of yacc.c  */
#line 3978 "lef.y"
    { ;}
    break;



/* Line 1455 of yacc.c  */
#line 10417 "lef.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 3980 "lef.y"


