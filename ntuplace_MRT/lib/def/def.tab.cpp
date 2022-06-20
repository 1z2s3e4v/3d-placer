
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
#define yyparse         defyyparse
#define yylex           defyylex
#define yyerror         defyyerror
#define yylval          defyylval
#define yychar          defyychar
#define yydebug         defyydebug
#define yynerrs         defyynerrs


/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 15 "def.y"

#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "defrReader.hpp"
#include "lex.h"
#include "defiUser.hpp"
#include "defrCallBacks.hpp"

// TX_DIR:TRANSLATION ON

extern int dumb_mode;     // Read next N tokens in dumb mode
extern int no_num;        // No numbers for the next N tokens
extern int real_num;      // Next token will be a real number
extern int by_is_keyword;
extern int bit_is_keyword;
extern int do_is_keyword;
extern int new_is_keyword;
extern int nondef_is_keyword;
extern int step_is_keyword;
extern int mustjoin_is_keyword;
extern char* History_text;

extern int errors;        // from lex.cpph

double save_x;
double save_y;

static double defVersionNum = 5.5;   // default is 5.5

static int defIgnoreVersion = 0; // ignore checking version number
                                 // this is redundant, since def does not have
                                 // either 5.3 or 5.4 code

static int hasVer = 0;        // keep track VERSION is in the file
static int hasNameCase = 0;   // keep track NAMESCASESENSITIVE is in the file
static int hasDivChar = 0;    // keep track DIVIDERCHAR is in the file
static int hasBusBit = 0;     // keep track BUSBITCHARS is in the file
static int hasDes = 0;        // keep track DESIGN is in the file
static int regTypeDef;        // keep track that region type is defined 
static int aOxide = 0;        // keep track for oxide

int names_case_sensitive = FALSE;
int defRetVal;

int shield = FALSE;       // To identify if the path is shield for 5.3

static char defPropDefType;   // save the current type of the property

int bitsNum;                  // Scanchain Bits value

/* From def_keywords.cpp */
extern char* ringCopy(const char* string);


/* Macro to describe how we handle a callback.
 * If the function was set then call it.
 * If the function returns non zero then there was an error
 * so call the error routine and exit.
 */
#define CALLBACK(func, typ, data) \
    if (!errors) {\
      if (func) { \
        if ((defRetVal = (*func)(typ, data, defrUserData)) == PARSE_OK) { \
        } else if (defRetVal == STOP_PARSE) { \
	  return defRetVal; \
        } else { \
          yyerror("Error in callback"); \
	  return defRetVal; \
        } \
      } \
    }

#define CHKERR() \
    if (errors > 20) {\
      yyerror("Too many syntax errors"); \
      errors = 0; \
      return 1; \
    }

double convert_defname2num(char *versionName)
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

    free(versionNm);
    return version;
}


#define FIXED 1
#define COVER 2
#define PLACED 3
#define UNPLACED 4


/* Line 189 of yacc.c  */
#line 202 "def.tab.c"

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
     QSTRING = 258,
     T_STRING = 259,
     SITE_PATTERN = 260,
     NUMBER = 261,
     K_HISTORY = 262,
     K_NAMESCASESENSITIVE = 263,
     K_DESIGN = 264,
     K_VIAS = 265,
     K_TECH = 266,
     K_UNITS = 267,
     K_ARRAY = 268,
     K_FLOORPLAN = 269,
     K_SITE = 270,
     K_CANPLACE = 271,
     K_CANNOTOCCUPY = 272,
     K_DIE_AREA = 273,
     K_PINS = 274,
     K_DEFAULTCAP = 275,
     K_MINPINS = 276,
     K_WIRECAP = 277,
     K_TRACKS = 278,
     K_GCELLGRID = 279,
     K_DO = 280,
     K_BY = 281,
     K_STEP = 282,
     K_LAYER = 283,
     K_ROW = 284,
     K_RECT = 285,
     K_COMPS = 286,
     K_COMP_GEN = 287,
     K_SOURCE = 288,
     K_WEIGHT = 289,
     K_EEQMASTER = 290,
     K_FIXED = 291,
     K_COVER = 292,
     K_UNPLACED = 293,
     K_PLACED = 294,
     K_FOREIGN = 295,
     K_REGION = 296,
     K_REGIONS = 297,
     K_NETS = 298,
     K_START_NET = 299,
     K_MUSTJOIN = 300,
     K_ORIGINAL = 301,
     K_USE = 302,
     K_STYLE = 303,
     K_PATTERN = 304,
     K_PATTERNNAME = 305,
     K_ESTCAP = 306,
     K_ROUTED = 307,
     K_NEW = 308,
     K_SNETS = 309,
     K_SHAPE = 310,
     K_WIDTH = 311,
     K_VOLTAGE = 312,
     K_SPACING = 313,
     K_NONDEFAULTRULE = 314,
     K_N = 315,
     K_S = 316,
     K_E = 317,
     K_W = 318,
     K_FN = 319,
     K_FE = 320,
     K_FS = 321,
     K_FW = 322,
     K_GROUPS = 323,
     K_GROUP = 324,
     K_SOFT = 325,
     K_MAXX = 326,
     K_MAXY = 327,
     K_MAXHALFPERIMETER = 328,
     K_CONSTRAINTS = 329,
     K_NET = 330,
     K_PATH = 331,
     K_SUM = 332,
     K_DIFF = 333,
     K_SCANCHAINS = 334,
     K_START = 335,
     K_FLOATING = 336,
     K_ORDERED = 337,
     K_STOP = 338,
     K_IN = 339,
     K_OUT = 340,
     K_RISEMIN = 341,
     K_RISEMAX = 342,
     K_FALLMIN = 343,
     K_FALLMAX = 344,
     K_WIREDLOGIC = 345,
     K_MAXDIST = 346,
     K_ASSERTIONS = 347,
     K_DISTANCE = 348,
     K_MICRONS = 349,
     K_END = 350,
     K_IOTIMINGS = 351,
     K_RISE = 352,
     K_FALL = 353,
     K_VARIABLE = 354,
     K_SLEWRATE = 355,
     K_CAPACITANCE = 356,
     K_DRIVECELL = 357,
     K_FROMPIN = 358,
     K_TOPIN = 359,
     K_PARALLEL = 360,
     K_TIMINGDISABLES = 361,
     K_THRUPIN = 362,
     K_MACRO = 363,
     K_PARTITIONS = 364,
     K_TURNOFF = 365,
     K_FROMCLOCKPIN = 366,
     K_FROMCOMPPIN = 367,
     K_FROMIOPIN = 368,
     K_TOCLOCKPIN = 369,
     K_TOCOMPPIN = 370,
     K_TOIOPIN = 371,
     K_SETUPRISE = 372,
     K_SETUPFALL = 373,
     K_HOLDRISE = 374,
     K_HOLDFALL = 375,
     K_VPIN = 376,
     K_SUBNET = 377,
     K_XTALK = 378,
     K_PIN = 379,
     K_SYNTHESIZED = 380,
     K_DEFINE = 381,
     K_DEFINES = 382,
     K_DEFINEB = 383,
     K_IF = 384,
     K_THEN = 385,
     K_ELSE = 386,
     K_FALSE = 387,
     K_TRUE = 388,
     K_EQ = 389,
     K_NE = 390,
     K_LE = 391,
     K_LT = 392,
     K_GE = 393,
     K_GT = 394,
     K_OR = 395,
     K_AND = 396,
     K_NOT = 397,
     K_SPECIAL = 398,
     K_DIRECTION = 399,
     K_RANGE = 400,
     K_FPC = 401,
     K_HORIZONTAL = 402,
     K_VERTICAL = 403,
     K_ALIGN = 404,
     K_MIN = 405,
     K_MAX = 406,
     K_EQUAL = 407,
     K_BOTTOMLEFT = 408,
     K_TOPRIGHT = 409,
     K_ROWS = 410,
     K_TAPER = 411,
     K_TAPERRULE = 412,
     K_VERSION = 413,
     K_DIVIDERCHAR = 414,
     K_BUSBITCHARS = 415,
     K_PROPERTYDEFINITIONS = 416,
     K_STRING = 417,
     K_REAL = 418,
     K_INTEGER = 419,
     K_PROPERTY = 420,
     K_BEGINEXT = 421,
     K_ENDEXT = 422,
     K_NAMEMAPSTRING = 423,
     K_ON = 424,
     K_OFF = 425,
     K_X = 426,
     K_Y = 427,
     K_COMPONENT = 428,
     K_PINPROPERTIES = 429,
     K_TEST = 430,
     K_COMMONSCANPINS = 431,
     K_SNET = 432,
     K_COMPONENTPIN = 433,
     K_REENTRANTPATHS = 434,
     K_SHIELD = 435,
     K_SHIELDNET = 436,
     K_NOSHIELD = 437,
     K_ANTENNAPINPARTIALMETALAREA = 438,
     K_ANTENNAPINPARTIALMETALSIDEAREA = 439,
     K_ANTENNAPINGATEAREA = 440,
     K_ANTENNAPINDIFFAREA = 441,
     K_ANTENNAPINMAXAREACAR = 442,
     K_ANTENNAPINMAXSIDEAREACAR = 443,
     K_ANTENNAPINPARTIALCUTAREA = 444,
     K_ANTENNAPINMAXCUTCAR = 445,
     K_SIGNAL = 446,
     K_POWER = 447,
     K_GROUND = 448,
     K_CLOCK = 449,
     K_TIEOFF = 450,
     K_ANALOG = 451,
     K_SCAN = 452,
     K_RESET = 453,
     K_RING = 454,
     K_STRIPE = 455,
     K_FOLLOWPIN = 456,
     K_IOWIRE = 457,
     K_COREWIRE = 458,
     K_BLOCKWIRE = 459,
     K_FILLWIRE = 460,
     K_BLOCKAGEWIRE = 461,
     K_PADRING = 462,
     K_BLOCKRING = 463,
     K_BLOCKAGES = 464,
     K_PLACEMENT = 465,
     K_SLOTS = 466,
     K_FILLS = 467,
     K_PUSHDOWN = 468,
     K_NETLIST = 469,
     K_DIST = 470,
     K_USER = 471,
     K_TIMING = 472,
     K_BALANCED = 473,
     K_STEINER = 474,
     K_TRUNK = 475,
     K_FIXEDBUMP = 476,
     K_FENCE = 477,
     K_FREQUENCY = 478,
     K_GUIDE = 479,
     K_MAXBITS = 480,
     K_PARTITION = 481,
     K_TYPE = 482,
     K_ANTENNAMODEL = 483,
     K_DRCFILL = 484,
     K_OXIDE1 = 485,
     K_OXIDE2 = 486,
     K_OXIDE3 = 487,
     K_OXIDE4 = 488
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 135 "def.y"

        double dval ;
        int    integer ;
        char * string ;
        int    keyword ;  /* really just a nop */
        struct defpoint pt;
        defTOKEN *tk;



/* Line 214 of yacc.c  */
#line 482 "def.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 494 "def.tab.c"

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
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1134

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  241
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  426
/* YYNRULES -- Number of rules.  */
#define YYNRULES  771
/* YYNRULES -- Number of states.  */
#define YYNSTATES  1324

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   488

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     237,   238,   239,   236,   240,   235,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   234,
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
     225,   226,   227,   228,   229,   230,   231,   232,   233
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     8,     9,    10,    15,    16,    20,    24,
      25,    28,    30,    32,    34,    36,    38,    40,    42,    44,
      46,    48,    50,    52,    54,    56,    58,    60,    62,    64,
      66,    68,    70,    72,    74,    76,    78,    80,    82,    84,
      86,    88,    90,    92,    94,    96,    98,   100,   102,   104,
     105,   110,   113,   114,   119,   120,   125,   126,   131,   133,
     134,   140,   141,   144,   145,   151,   152,   158,   159,   165,
     166,   172,   173,   179,   180,   186,   187,   193,   194,   200,
     201,   207,   210,   211,   216,   217,   222,   224,   227,   230,
     231,   233,   239,   243,   247,   248,   263,   264,   279,   280,
     295,   297,   299,   301,   303,   305,   307,   309,   311,   316,
     320,   323,   324,   327,   333,   336,   340,   344,   345,   348,
     349,   350,   351,   362,   363,   366,   369,   371,   375,   379,
     380,   387,   391,   396,   401,   406,   411,   412,   419,   420,
     427,   432,   433,   440,   444,   446,   448,   450,   452,   454,
     456,   458,   460,   462,   464,   466,   468,   469,   470,   474,
     477,   478,   479,   497,   498,   501,   502,   507,   508,   511,
     514,   517,   520,   521,   531,   534,   536,   538,   539,   540,
     545,   546,   549,   551,   560,   562,   565,   569,   573,   574,
     577,   578,   579,   586,   587,   590,   591,   598,   599,   604,
     606,   611,   616,   621,   626,   629,   634,   638,   639,   642,
     643,   644,   652,   655,   659,   660,   663,   664,   669,   673,
     674,   677,   680,   683,   686,   688,   690,   694,   698,   699,
     702,   706,   709,   710,   715,   716,   719,   722,   723,   726,
     728,   730,   732,   734,   736,   738,   740,   742,   744,   746,
     747,   752,   753,   759,   760,   762,   766,   768,   770,   772,
     774,   777,   780,   783,   787,   788,   793,   795,   798,   801,
     804,   807,   810,   811,   818,   820,   823,   827,   830,   835,
     838,   841,   844,   848,   851,   855,   859,   860,   863,   867,
     869,   870,   874,   875,   879,   880,   887,   888,   891,   892,
     899,   900,   907,   908,   915,   916,   918,   921,   922,   925,
     926,   927,   933,   937,   940,   944,   945,   950,   954,   958,
     962,   966,   970,   971,   976,   977,   982,   984,   985,   990,
     991,   992,   998,   999,  1000,  1008,  1009,  1014,  1016,  1018,
    1021,  1024,  1027,  1030,  1032,  1034,  1036,  1038,  1040,  1046,
    1047,  1052,  1053,  1054,  1058,  1059,  1063,  1065,  1067,  1069,
    1071,  1073,  1075,  1077,  1080,  1081,  1085,  1086,  1087,  1096,
    1097,  1100,  1102,  1105,  1114,  1116,  1121,  1126,  1131,  1136,
    1142,  1148,  1154,  1160,  1161,  1163,  1164,  1168,  1169,  1173,
    1174,  1176,  1179,  1181,  1183,  1185,  1187,  1189,  1191,  1193,
    1195,  1197,  1199,  1201,  1205,  1206,  1209,  1213,  1214,  1217,
    1219,  1221,  1223,  1225,  1226,  1227,  1233,  1234,  1235,  1242,
    1246,  1249,  1253,  1254,  1259,  1263,  1267,  1271,  1275,  1276,
    1281,  1282,  1287,  1289,  1290,  1296,  1300,  1301,  1302,  1310,
    1312,  1315,  1318,  1321,  1324,  1325,  1329,  1330,  1334,  1336,
    1338,  1340,  1342,  1344,  1347,  1348,  1352,  1353,  1354,  1362,
    1364,  1368,  1371,  1375,  1379,  1380,  1383,  1388,  1389,  1393,
    1394,  1397,  1399,  1400,  1403,  1407,  1408,  1413,  1414,  1419,
    1421,  1424,  1426,  1427,  1430,  1433,  1436,  1439,  1440,  1443,
    1446,  1449,  1452,  1455,  1459,  1463,  1467,  1471,  1472,  1475,
    1477,  1479,  1484,  1485,  1489,  1490,  1497,  1502,  1507,  1509,
    1513,  1514,  1523,  1524,  1526,  1527,  1530,  1534,  1538,  1542,
    1546,  1549,  1552,  1556,  1560,  1561,  1564,  1568,  1569,  1573,
    1574,  1577,  1578,  1580,  1581,  1587,  1588,  1593,  1594,  1599,
    1600,  1606,  1607,  1612,  1613,  1619,  1621,  1622,  1627,  1636,
    1637,  1640,  1641,  1645,  1646,  1651,  1660,  1673,  1674,  1677,
    1678,  1682,  1683,  1688,  1697,  1710,  1711,  1714,  1717,  1721,
    1725,  1726,  1729,  1733,  1734,  1741,  1742,  1745,  1751,  1757,
    1761,  1762,  1763,  1770,  1772,  1773,  1774,  1781,  1782,  1783,
    1787,  1788,  1791,  1793,  1795,  1798,  1803,  1807,  1808,  1811,
    1812,  1813,  1822,  1824,  1826,  1828,  1831,  1834,  1837,  1838,
    1841,  1842,  1847,  1848,  1853,  1854,  1857,  1858,  1864,  1865,
    1871,  1875,  1879,  1880,  1883,  1884,  1885,  1896,  1897,  1904,
    1905,  1912,  1916,  1917,  1918,  1925,  1926,  1930,  1933,  1937,
    1941,  1942,  1945,  1949,  1950,  1955,  1956,  1960,  1961,  1963,
    1965,  1966,  1968,  1970,  1971,  1974,  1975,  1983,  1984,  1991,
    1992,  1998,  1999,  2007,  2008,  2015,  2016,  2022,  2024,  2025,
    2030,  2031,  2034,  2038,  2042,  2043,  2046,  2047,  2050,  2053,
    2056,  2059,  2062,  2064,  2067,  2071,  2075,  2079,  2083,  2086,
    2087,  2090,  2091,  2098,  2099,  2102,  2103,  2106,  2107,  2111,
    2112,  2116,  2118,  2120,  2122,  2124,  2128,  2132,  2133,  2135,
    2138,  2139,  2142,  2143,  2144,  2152,  2153,  2156,  2157,  2162,
    2163,  2166,  2169,  2172,  2175,  2179,  2183,  2186,  2187,  2190,
    2195,  2196,  2197,  2204,  2205,  2210,  2211,  2212,  2217,  2220,
    2223,  2226,  2227,  2228,  2233,  2236,  2237,  2240,  2244,  2248,
    2252,  2255,  2256,  2259,  2263,  2264,  2265,  2272,  2273,  2276,
    2280,  2284,  2288,  2291,  2292,  2295,  2299,  2300,  2301,  2308,
    2309,  2312
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     242,     0,    -1,   243,   245,   246,   251,    -1,    -1,    -1,
     158,   244,     4,   234,    -1,    -1,     8,   169,   234,    -1,
       8,   170,   234,    -1,    -1,   246,   247,    -1,     1,    -1,
     248,    -1,   329,    -1,   327,    -1,   354,    -1,   386,    -1,
     441,    -1,   474,    -1,   510,    -1,   493,    -1,   492,    -1,
     536,    -1,   341,    -1,   554,    -1,   570,    -1,   583,    -1,
     619,    -1,   632,    -1,   647,    -1,   657,    -1,   249,    -1,
     252,    -1,   258,    -1,   276,    -1,   277,    -1,   278,    -1,
     279,    -1,   281,    -1,   283,    -1,   286,    -1,   287,    -1,
     292,    -1,   318,    -1,   326,    -1,   254,    -1,   256,    -1,
     310,    -1,   259,    -1,    -1,     9,   250,     4,   234,    -1,
      95,     9,    -1,    -1,    11,   253,     4,   234,    -1,    -1,
      13,   255,     4,   234,    -1,    -1,    14,   257,     4,   234,
      -1,     7,    -1,    -1,   161,   260,   261,    95,   161,    -1,
      -1,   261,   262,    -1,    -1,     9,   263,     4,   272,   234,
      -1,    -1,    75,   264,     4,   272,   234,    -1,    -1,   177,
     265,     4,   272,   234,    -1,    -1,    41,   266,     4,   272,
     234,    -1,    -1,    69,   267,     4,   272,   234,    -1,    -1,
     173,   268,     4,   272,   234,    -1,    -1,    29,   269,     4,
     272,   234,    -1,    -1,   124,   270,     4,   272,   234,    -1,
      -1,   178,   271,     4,   272,   234,    -1,     1,   234,    -1,
      -1,   164,   273,   463,   275,    -1,    -1,   163,   274,   463,
     275,    -1,   162,    -1,   162,     3,    -1,   168,     4,    -1,
      -1,     6,    -1,    12,    93,    94,     6,   234,    -1,   159,
       3,   234,    -1,   160,     3,   234,    -1,    -1,    15,   280,
       4,     6,     6,   285,    25,     6,    26,     6,    27,     6,
       6,   234,    -1,    -1,    16,   282,     4,     6,     6,   285,
      25,     6,    26,     6,    27,     6,     6,   234,    -1,    -1,
      17,   284,     4,     6,     6,   285,    25,     6,    26,     6,
      27,     6,     6,   234,    -1,    60,    -1,    63,    -1,    61,
      -1,    62,    -1,    64,    -1,    67,    -1,    66,    -1,    65,
      -1,    18,   339,   339,   234,    -1,   288,   289,   291,    -1,
      20,     6,    -1,    -1,   289,   290,    -1,    21,     6,    22,
       6,   234,    -1,    95,    20,    -1,   293,   294,   309,    -1,
      19,     6,   234,    -1,    -1,   294,   295,    -1,    -1,    -1,
      -1,   235,   296,     4,   236,    75,   297,     4,   298,   299,
     234,    -1,    -1,   299,   300,    -1,   236,   143,    -1,   328,
      -1,   236,   144,     4,    -1,   236,    47,   306,    -1,    -1,
     236,    28,   301,     4,   339,   339,    -1,   383,   339,   285,
      -1,   236,   183,     6,   307,    -1,   236,   184,     6,   307,
      -1,   236,   185,     6,   307,    -1,   236,   186,     6,   307,
      -1,    -1,   236,   187,     6,    28,   302,     4,    -1,    -1,
     236,   188,     6,    28,   303,     4,    -1,   236,   189,     6,
     307,    -1,    -1,   236,   190,     6,    28,   304,     4,    -1,
     236,   228,   305,    -1,   230,    -1,   231,    -1,   232,    -1,
     233,    -1,   191,    -1,   192,    -1,   193,    -1,   194,    -1,
     195,    -1,   196,    -1,   197,    -1,   198,    -1,    -1,    -1,
      28,   308,     4,    -1,    95,    19,    -1,    -1,    -1,    29,
     311,     4,     4,     6,     6,   285,    25,     6,    26,     6,
      27,     6,     6,   312,   313,   234,    -1,    -1,   313,   314,
      -1,    -1,   236,   165,   315,   316,    -1,    -1,   316,   317,
      -1,     4,     6,    -1,     4,     3,    -1,     4,     4,    -1,
      -1,   320,     6,   319,    25,     6,    27,     6,   322,   234,
      -1,    23,   321,    -1,   171,    -1,   172,    -1,    -1,    -1,
      28,   323,   325,   324,    -1,    -1,   325,   324,    -1,     4,
      -1,    24,   321,     6,    25,     6,    27,     6,   234,    -1,
     166,    -1,   236,   166,    -1,   330,   331,   340,    -1,    10,
       6,   234,    -1,    -1,   331,   332,    -1,    -1,    -1,   235,
     333,     4,   334,   335,   234,    -1,    -1,   335,   336,    -1,
      -1,   236,    30,   337,     4,   339,   339,    -1,    -1,   236,
      50,   338,     4,    -1,   328,    -1,   237,     6,     6,   238,
      -1,   237,   239,     6,   238,    -1,   237,     6,   239,   238,
      -1,   237,   239,   239,   238,    -1,    95,    10,    -1,   342,
     343,    95,    42,    -1,    42,     6,   234,    -1,    -1,   343,
     344,    -1,    -1,    -1,   235,   345,     4,   346,   347,   348,
     234,    -1,   339,   339,    -1,   347,   339,   339,    -1,    -1,
     348,   349,    -1,    -1,   236,   165,   350,   351,    -1,   236,
     227,   353,    -1,    -1,   351,   352,    -1,     4,     6,    -1,
       4,     3,    -1,     4,     4,    -1,   222,    -1,   224,    -1,
     355,   356,   385,    -1,    31,     6,   234,    -1,    -1,   356,
     357,    -1,   358,   362,   234,    -1,   359,   361,    -1,    -1,
     235,   360,     4,     4,    -1,    -1,   361,   239,    -1,   361,
       4,    -1,    -1,   362,   363,    -1,   367,    -1,   370,    -1,
     382,    -1,   384,    -1,   379,    -1,   372,    -1,   365,    -1,
     374,    -1,   364,    -1,   328,    -1,    -1,   236,    35,   366,
       4,    -1,    -1,   236,    32,   368,     4,   369,    -1,    -1,
       4,    -1,   236,    33,   371,    -1,   214,    -1,   215,    -1,
     216,    -1,   217,    -1,   378,   373,    -1,   378,     4,    -1,
     339,   339,    -1,   373,   339,   339,    -1,    -1,   236,   165,
     375,   376,    -1,   377,    -1,   376,   377,    -1,     4,     6,
      -1,     4,     3,    -1,     4,     4,    -1,   236,    41,    -1,
      -1,   236,    40,   380,     4,   381,   285,    -1,   339,    -1,
       6,     6,    -1,   383,   339,   285,    -1,   236,    38,    -1,
     236,    38,   339,   285,    -1,   236,    36,    -1,   236,    37,
      -1,   236,    39,    -1,   236,    34,     6,    -1,    95,    31,
      -1,   387,   388,   439,    -1,    43,     6,   234,    -1,    -1,
     388,   389,    -1,   390,   402,   234,    -1,   391,    -1,    -1,
     235,   392,   393,    -1,    -1,     4,   394,   396,    -1,    -1,
      45,   237,     4,   395,     4,   238,    -1,    -1,   396,   397,
      -1,    -1,   237,     4,   398,     4,   401,   238,    -1,    -1,
     237,   239,   399,     4,   401,   238,    -1,    -1,   237,   124,
     400,     4,   401,   238,    -1,    -1,   328,    -1,   236,   125,
      -1,    -1,   402,   403,    -1,    -1,    -1,   236,   425,   404,
     405,   426,    -1,   236,    33,   417,    -1,   236,   221,    -1,
     236,   223,     6,    -1,    -1,   236,    46,   406,     4,    -1,
     236,    49,   464,    -1,   236,    34,     6,    -1,   236,   123,
       6,    -1,   236,    51,     6,    -1,   236,    47,   306,    -1,
      -1,   236,    48,   407,     4,    -1,    -1,   236,    59,   408,
       4,    -1,   418,    -1,    -1,   236,   181,   409,     4,    -1,
      -1,    -1,   236,   182,   410,   411,   426,    -1,    -1,    -1,
     236,   122,   412,     4,   413,   610,   614,    -1,    -1,   236,
     165,   414,   415,    -1,   328,    -1,   416,    -1,   415,   416,
      -1,     4,     6,    -1,     4,     3,    -1,     4,     4,    -1,
     214,    -1,   215,    -1,   216,    -1,   217,    -1,   175,    -1,
     419,   421,   339,   339,   423,    -1,    -1,   236,   121,   420,
       4,    -1,    -1,    -1,    28,   422,     4,    -1,    -1,   424,
     339,   285,    -1,    39,    -1,    36,    -1,    37,    -1,    36,
      -1,    37,    -1,    52,    -1,   429,    -1,   426,   427,    -1,
      -1,    53,   428,   429,    -1,    -1,    -1,     4,   430,   438,
     437,   435,   434,   431,   432,    -1,    -1,   432,   433,    -1,
       4,    -1,     4,   285,    -1,     4,    25,     6,    26,     6,
      27,     6,     6,    -1,   434,    -1,   237,     6,     6,   238,
      -1,   237,   239,     6,   238,    -1,   237,     6,   239,   238,
      -1,   237,   239,   239,   238,    -1,   237,     6,     6,     6,
     238,    -1,   237,   239,     6,     6,   238,    -1,   237,     6,
     239,     6,   238,    -1,   237,   239,   239,     6,   238,    -1,
      -1,   156,    -1,    -1,   157,   436,     4,    -1,    -1,   236,
      55,   440,    -1,    -1,     6,    -1,    95,    43,    -1,   199,
      -1,   200,    -1,   201,    -1,   202,    -1,   203,    -1,   204,
      -1,   205,    -1,   229,    -1,   206,    -1,   207,    -1,   208,
      -1,   472,   442,   473,    -1,    -1,   442,   443,    -1,   390,
     444,   234,    -1,    -1,   444,   445,    -1,   454,    -1,   456,
      -1,   457,    -1,   446,    -1,    -1,    -1,   236,   425,   447,
     448,   465,    -1,    -1,    -1,   236,   180,   449,     4,   450,
     465,    -1,   236,    33,   371,    -1,   236,   221,    -1,   236,
     223,     6,    -1,    -1,   236,    46,   451,     4,    -1,   236,
      49,   464,    -1,   236,    34,     6,    -1,   236,    51,     6,
      -1,   236,    47,   306,    -1,    -1,   236,    48,   452,     4,
      -1,    -1,   236,   165,   453,   460,    -1,   328,    -1,    -1,
     236,    56,   455,     4,     6,    -1,   236,    57,     6,    -1,
      -1,    -1,   236,    58,   458,     4,     6,   459,   462,    -1,
     461,    -1,   460,   461,    -1,     4,     6,    -1,     4,     3,
      -1,     4,     4,    -1,    -1,   145,     6,     6,    -1,    -1,
     145,     6,     6,    -1,   218,    -1,   219,    -1,   220,    -1,
      90,    -1,   468,    -1,   465,   466,    -1,    -1,    53,   467,
     468,    -1,    -1,    -1,     4,   469,   471,   437,   434,   470,
     432,    -1,     6,    -1,    54,     6,   234,    -1,    95,    54,
      -1,   475,   476,   491,    -1,    68,     6,   234,    -1,    -1,
     476,   477,    -1,   478,   480,   482,   234,    -1,    -1,   235,
     479,     4,    -1,    -1,   480,   481,    -1,     4,    -1,    -1,
     482,   483,    -1,   236,    70,   489,    -1,    -1,   236,   165,
     484,   487,    -1,    -1,   236,    41,   485,   486,    -1,   328,
      -1,   339,   339,    -1,     4,    -1,    -1,   487,   488,    -1,
       4,     6,    -1,     4,     3,    -1,     4,     4,    -1,    -1,
     489,   490,    -1,    71,     6,    -1,    72,     6,    -1,    73,
       6,    -1,    95,    68,    -1,   494,   496,   509,    -1,   495,
     496,   508,    -1,    92,     6,   234,    -1,    74,     6,   234,
      -1,    -1,   496,   497,    -1,   498,    -1,   503,    -1,   235,
     499,   506,   234,    -1,    -1,    75,   500,     4,    -1,    -1,
      76,   501,     4,     4,     4,     4,    -1,    77,   237,   502,
     238,    -1,    78,   237,   502,   238,    -1,   499,    -1,   502,
     240,   499,    -1,    -1,   235,    90,   504,     4,   505,    91,
       6,   234,    -1,    -1,   236,    -1,    -1,   506,   507,    -1,
     236,    86,     6,    -1,   236,    87,     6,    -1,   236,    88,
       6,    -1,   236,    89,     6,    -1,    95,    74,    -1,    95,
      92,    -1,   511,   512,   535,    -1,    79,     6,   234,    -1,
      -1,   512,   513,    -1,   514,   516,   234,    -1,    -1,   235,
     515,     4,    -1,    -1,   516,   518,    -1,    -1,     4,    -1,
      -1,   236,    80,   519,     4,   517,    -1,    -1,   236,    81,
     520,   526,    -1,    -1,   236,    82,   521,   530,    -1,    -1,
     236,    83,   522,     4,   517,    -1,    -1,   236,   176,   523,
     525,    -1,    -1,   236,   226,   524,     4,   534,    -1,   328,
      -1,    -1,   237,     4,     4,   238,    -1,   237,     4,     4,
     238,   237,     4,     4,   238,    -1,    -1,   526,   527,    -1,
      -1,     4,   528,   529,    -1,    -1,   237,     4,     4,   238,
      -1,   237,     4,     4,   238,   237,     4,     4,   238,    -1,
     237,     4,     4,   238,   237,     4,     4,   238,   237,     4,
       4,   238,    -1,    -1,   530,   531,    -1,    -1,     4,   532,
     533,    -1,    -1,   237,     4,     4,   238,    -1,   237,     4,
       4,   238,   237,     4,     4,   238,    -1,   237,     4,     4,
     238,   237,     4,     4,   238,   237,     4,     4,   238,    -1,
      -1,   225,     6,    -1,    95,    79,    -1,   537,   538,   553,
      -1,    96,     6,   234,    -1,    -1,   538,   539,    -1,   540,
     542,   234,    -1,    -1,   235,   237,   541,     4,     4,   238,
      -1,    -1,   542,   543,    -1,   236,   552,    99,     6,     6,
      -1,   236,   552,   100,     6,     6,    -1,   236,   101,     6,
      -1,    -1,    -1,   236,   102,   544,     4,   545,   546,    -1,
     328,    -1,    -1,    -1,   549,   104,   547,     4,   548,   551,
      -1,    -1,    -1,   103,   550,     4,    -1,    -1,   105,     6,
      -1,    97,    -1,    98,    -1,    95,    96,    -1,   555,   556,
      95,   146,    -1,   146,     6,   234,    -1,    -1,   556,   557,
      -1,    -1,    -1,   235,   558,     4,   560,   559,   561,   562,
     234,    -1,   147,    -1,   148,    -1,   149,    -1,   151,     6,
      -1,   150,     6,    -1,   152,     6,    -1,    -1,   562,   563,
      -1,    -1,   236,   153,   564,   566,    -1,    -1,   236,   154,
     565,   566,    -1,    -1,   566,   567,    -1,    -1,   237,   155,
     568,     4,   238,    -1,    -1,   237,    31,   569,     4,   238,
      -1,   571,   572,   582,    -1,   106,     6,   234,    -1,    -1,
     572,   573,    -1,    -1,    -1,   235,   103,   574,     4,     4,
     104,   575,     4,     4,   234,    -1,    -1,   235,   107,   576,
       4,     4,   234,    -1,    -1,   235,   108,   577,     4,   578,
     234,    -1,   235,   179,   234,    -1,    -1,    -1,   103,   579,
       4,   104,   580,     4,    -1,    -1,   107,   581,     4,    -1,
      95,   106,    -1,   584,   585,   609,    -1,   109,     6,   234,
      -1,    -1,   585,   586,    -1,   587,   592,   234,    -1,    -1,
     235,   588,     4,   589,    -1,    -1,   110,   590,   591,    -1,
      -1,   117,    -1,   118,    -1,    -1,   119,    -1,   120,    -1,
      -1,   592,   593,    -1,    -1,   236,   111,   594,     4,     4,
     552,   600,    -1,    -1,   236,   112,   595,     4,     4,   607,
      -1,    -1,   236,   113,   596,     4,   605,    -1,    -1,   236,
     114,   597,     4,     4,   552,   600,    -1,    -1,   236,   115,
     598,     4,     4,   607,    -1,    -1,   236,   116,   599,     4,
     605,    -1,   328,    -1,    -1,   602,    19,   601,   604,    -1,
      -1,   602,   603,    -1,   150,     6,     6,    -1,   151,     6,
       6,    -1,    -1,   604,     4,    -1,    -1,   605,   606,    -1,
      86,     6,    -1,    88,     6,    -1,    87,     6,    -1,    89,
       6,    -1,   608,    -1,   607,   608,    -1,    86,     6,     6,
      -1,    88,     6,     6,    -1,    87,     6,     6,    -1,    89,
       6,     6,    -1,    95,   109,    -1,    -1,   610,   611,    -1,
      -1,   237,   612,     4,     4,   613,   238,    -1,    -1,   236,
     125,    -1,    -1,   614,   615,    -1,    -1,   618,   616,   426,
      -1,    -1,    59,   617,     4,    -1,    36,    -1,    37,    -1,
      52,    -1,   182,    -1,   620,   623,   622,    -1,   174,     6,
     621,    -1,    -1,   234,    -1,    95,   174,    -1,    -1,   623,
     624,    -1,    -1,    -1,   235,   625,     4,     4,   626,   627,
     234,    -1,    -1,   627,   628,    -1,    -1,   236,   165,   629,
     630,    -1,    -1,   630,   631,    -1,     4,     6,    -1,     4,
       3,    -1,     4,     4,    -1,   633,   635,   634,    -1,   209,
       6,   234,    -1,    95,   209,    -1,    -1,   635,   636,    -1,
     637,   646,   645,   234,    -1,    -1,    -1,   235,    28,   638,
       4,   639,   641,    -1,    -1,   235,   210,   640,   643,    -1,
      -1,    -1,   236,   173,   642,     4,    -1,   236,   211,    -1,
     236,   212,    -1,   236,   213,    -1,    -1,    -1,   236,   173,
     644,     4,    -1,   236,   213,    -1,    -1,   645,   646,    -1,
      30,   339,   339,    -1,   648,   650,   649,    -1,   211,     6,
     234,    -1,    95,   211,    -1,    -1,   650,   651,    -1,   652,
     655,   234,    -1,    -1,    -1,   235,    28,   653,     4,   654,
     656,    -1,    -1,   655,   656,    -1,    30,   339,   339,    -1,
     658,   660,   659,    -1,   212,     6,   234,    -1,    95,   212,
      -1,    -1,   660,   661,    -1,   662,   665,   234,    -1,    -1,
      -1,   235,    28,   663,     4,   664,   666,    -1,    -1,   665,
     666,    -1,    30,   339,   339,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   215,   215,   218,   219,   219,   233,   234,   241,   249,
     250,   251,   254,   254,   254,   254,   255,   255,   255,   255,
     256,   256,   256,   257,   257,   257,   258,   258,   258,   259,
     259,   262,   262,   262,   262,   263,   263,   264,   264,   264,
     264,   265,   265,   265,   265,   265,   266,   266,   266,   269,
     269,   275,   300,   300,   303,   303,   306,   306,   309,   313,
     312,   320,   321,   324,   324,   333,   333,   342,   342,   351,
     351,   360,   360,   369,   369,   378,   378,   387,   387,   397,
     396,   406,   408,   408,   413,   413,   419,   424,   429,   435,
     436,   439,   442,   448,   454,   454,   467,   467,   479,   479,
     492,   493,   494,   495,   496,   497,   498,   499,   501,   511,
     514,   523,   524,   527,   538,   541,   544,   547,   548,   551,
     552,   553,   551,   561,   562,   564,   570,   573,   579,   584,
     584,   590,   597,   606,   615,   624,   633,   633,   642,   642,
     652,   661,   661,   670,   678,   683,   688,   693,   699,   701,
     703,   705,   707,   709,   711,   713,   718,   719,   719,   722,
     725,   727,   725,   759,   760,   763,   763,   766,   767,   770,
     784,   792,   802,   801,   819,   824,   826,   829,   830,   830,
     833,   834,   837,   843,   856,   859,   862,   865,   868,   869,
     872,   873,   872,   877,   878,   881,   881,   886,   886,   891,
     895,   902,   908,   914,   920,   923,   926,   929,   930,   933,
     934,   933,   943,   946,   951,   952,   955,   955,   957,   968,
     969,   972,   986,   994,  1003,  1005,  1008,  1011,  1014,  1015,
    1018,  1021,  1034,  1034,  1042,  1043,  1048,  1054,  1055,  1058,
    1058,  1058,  1058,  1059,  1059,  1059,  1060,  1060,  1063,  1066,
    1066,  1072,  1072,  1080,  1081,  1084,  1090,  1092,  1094,  1096,
    1101,  1103,  1109,  1117,  1126,  1126,  1130,  1131,  1134,  1148,
    1156,  1165,  1168,  1168,  1179,  1181,  1184,  1192,  1200,  1219,
    1221,  1223,  1226,  1232,  1235,  1238,  1241,  1242,  1245,  1253,
    1257,  1257,  1260,  1259,  1266,  1266,  1274,  1275,  1278,  1278,
    1291,  1291,  1298,  1298,  1307,  1308,  1313,  1317,  1318,  1321,
    1322,  1321,  1334,  1337,  1346,  1355,  1355,  1358,  1361,  1364,
    1367,  1370,  1373,  1373,  1376,  1376,  1379,  1381,  1381,  1384,
    1385,  1384,  1411,  1418,  1411,  1430,  1430,  1433,  1436,  1437,
    1440,  1454,  1462,  1471,  1473,  1475,  1477,  1479,  1482,  1486,
    1486,  1489,  1490,  1490,  1493,  1494,  1497,  1499,  1501,  1504,
    1506,  1508,  1512,  1514,  1517,  1517,  1521,  1537,  1520,  1542,
    1543,  1547,  1549,  1555,  1570,  1575,  1582,  1588,  1594,  1599,
    1606,  1613,  1620,  1629,  1630,  1632,  1632,  1637,  1638,  1643,
    1644,  1648,  1651,  1653,  1655,  1657,  1659,  1661,  1663,  1665,
    1667,  1669,  1671,  1674,  1677,  1678,  1681,  1684,  1685,  1688,
    1688,  1689,  1689,  1692,  1693,  1692,  1704,  1705,  1704,  1728,
    1731,  1734,  1737,  1737,  1740,  1743,  1746,  1753,  1756,  1756,
    1759,  1759,  1762,  1765,  1765,  1772,  1777,  1778,  1777,  1785,
    1786,  1789,  1803,  1811,  1820,  1821,  1826,  1827,  1830,  1832,
    1834,  1836,  1840,  1842,  1845,  1845,  1849,  1853,  1848,  1857,
    1860,  1863,  1866,  1869,  1872,  1873,  1876,  1879,  1879,  1889,
    1890,  1893,  1899,  1900,  1903,  1905,  1905,  1907,  1907,  1909,
    1914,  1922,  1927,  1928,  1931,  1945,  1953,  1962,  1963,  1966,
    1972,  1978,  1985,  1989,  1993,  1996,  2007,  2018,  2019,  2022,
    2023,  2035,  2050,  2050,  2055,  2055,  2061,  2066,  2072,  2073,
    2076,  2076,  2085,  2086,  2089,  2090,  2093,  2098,  2103,  2108,
    2114,  2117,  2120,  2123,  2127,  2128,  2131,  2136,  2136,  2143,
    2144,  2149,  2150,  2153,  2153,  2157,  2157,  2160,  2159,  2168,
    2168,  2172,  2172,  2175,  2175,  2185,  2192,  2193,  2202,  2217,
    2218,  2222,  2221,  2232,  2233,  2246,  2267,  2297,  2298,  2302,
    2301,  2310,  2311,  2324,  2345,  2376,  2377,  2380,  2388,  2391,
    2401,  2402,  2405,  2411,  2411,  2417,  2418,  2422,  2427,  2432,
    2437,  2438,  2437,  2463,  2471,  2472,  2470,  2478,  2479,  2479,
    2485,  2486,  2492,  2492,  2494,  2500,  2503,  2507,  2508,  2511,
    2512,  2511,  2516,  2518,  2521,  2523,  2525,  2527,  2530,  2531,
    2535,  2534,  2538,  2537,  2542,  2543,  2545,  2545,  2547,  2547,
    2550,  2553,  2557,  2558,  2561,  2562,  2561,  2569,  2569,  2576,
    2576,  2583,  2589,  2590,  2589,  2595,  2595,  2601,  2605,  2608,
    2612,  2613,  2616,  2620,  2620,  2626,  2627,  2634,  2635,  2637,
    2641,  2642,  2644,  2647,  2648,  2651,  2651,  2657,  2657,  2663,
    2663,  2669,  2669,  2675,  2675,  2681,  2681,  2686,  2691,  2690,
    2694,  2695,  2698,  2703,  2709,  2710,  2713,  2714,  2716,  2718,
    2720,  2722,  2726,  2727,  2730,  2733,  2736,  2739,  2743,  2747,
    2748,  2751,  2751,  2761,  2762,  2765,  2766,  2769,  2768,  2779,
    2779,  2782,  2784,  2786,  2788,  2791,  2793,  2798,  2799,  2802,
    2805,  2806,  2809,  2810,  2809,  2817,  2818,  2820,  2820,  2824,
    2825,  2828,  2842,  2850,  2859,  2861,  2864,  2867,  2868,  2871,
    2877,  2878,  2877,  2889,  2888,  2899,  2901,  2901,  2907,  2912,
    2917,  2924,  2926,  2926,  2931,  2937,  2938,  2941,  2948,  2950,
    2953,  2956,  2957,  2960,  2966,  2967,  2966,  2972,  2973,  2976,
    2983,  2985,  2988,  2991,  2992,  2995,  3001,  3002,  3001,  3007,
    3008,  3011
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "QSTRING", "T_STRING", "SITE_PATTERN",
  "NUMBER", "K_HISTORY", "K_NAMESCASESENSITIVE", "K_DESIGN", "K_VIAS",
  "K_TECH", "K_UNITS", "K_ARRAY", "K_FLOORPLAN", "K_SITE", "K_CANPLACE",
  "K_CANNOTOCCUPY", "K_DIE_AREA", "K_PINS", "K_DEFAULTCAP", "K_MINPINS",
  "K_WIRECAP", "K_TRACKS", "K_GCELLGRID", "K_DO", "K_BY", "K_STEP",
  "K_LAYER", "K_ROW", "K_RECT", "K_COMPS", "K_COMP_GEN", "K_SOURCE",
  "K_WEIGHT", "K_EEQMASTER", "K_FIXED", "K_COVER", "K_UNPLACED",
  "K_PLACED", "K_FOREIGN", "K_REGION", "K_REGIONS", "K_NETS",
  "K_START_NET", "K_MUSTJOIN", "K_ORIGINAL", "K_USE", "K_STYLE",
  "K_PATTERN", "K_PATTERNNAME", "K_ESTCAP", "K_ROUTED", "K_NEW", "K_SNETS",
  "K_SHAPE", "K_WIDTH", "K_VOLTAGE", "K_SPACING", "K_NONDEFAULTRULE",
  "K_N", "K_S", "K_E", "K_W", "K_FN", "K_FE", "K_FS", "K_FW", "K_GROUPS",
  "K_GROUP", "K_SOFT", "K_MAXX", "K_MAXY", "K_MAXHALFPERIMETER",
  "K_CONSTRAINTS", "K_NET", "K_PATH", "K_SUM", "K_DIFF", "K_SCANCHAINS",
  "K_START", "K_FLOATING", "K_ORDERED", "K_STOP", "K_IN", "K_OUT",
  "K_RISEMIN", "K_RISEMAX", "K_FALLMIN", "K_FALLMAX", "K_WIREDLOGIC",
  "K_MAXDIST", "K_ASSERTIONS", "K_DISTANCE", "K_MICRONS", "K_END",
  "K_IOTIMINGS", "K_RISE", "K_FALL", "K_VARIABLE", "K_SLEWRATE",
  "K_CAPACITANCE", "K_DRIVECELL", "K_FROMPIN", "K_TOPIN", "K_PARALLEL",
  "K_TIMINGDISABLES", "K_THRUPIN", "K_MACRO", "K_PARTITIONS", "K_TURNOFF",
  "K_FROMCLOCKPIN", "K_FROMCOMPPIN", "K_FROMIOPIN", "K_TOCLOCKPIN",
  "K_TOCOMPPIN", "K_TOIOPIN", "K_SETUPRISE", "K_SETUPFALL", "K_HOLDRISE",
  "K_HOLDFALL", "K_VPIN", "K_SUBNET", "K_XTALK", "K_PIN", "K_SYNTHESIZED",
  "K_DEFINE", "K_DEFINES", "K_DEFINEB", "K_IF", "K_THEN", "K_ELSE",
  "K_FALSE", "K_TRUE", "K_EQ", "K_NE", "K_LE", "K_LT", "K_GE", "K_GT",
  "K_OR", "K_AND", "K_NOT", "K_SPECIAL", "K_DIRECTION", "K_RANGE", "K_FPC",
  "K_HORIZONTAL", "K_VERTICAL", "K_ALIGN", "K_MIN", "K_MAX", "K_EQUAL",
  "K_BOTTOMLEFT", "K_TOPRIGHT", "K_ROWS", "K_TAPER", "K_TAPERRULE",
  "K_VERSION", "K_DIVIDERCHAR", "K_BUSBITCHARS", "K_PROPERTYDEFINITIONS",
  "K_STRING", "K_REAL", "K_INTEGER", "K_PROPERTY", "K_BEGINEXT",
  "K_ENDEXT", "K_NAMEMAPSTRING", "K_ON", "K_OFF", "K_X", "K_Y",
  "K_COMPONENT", "K_PINPROPERTIES", "K_TEST", "K_COMMONSCANPINS", "K_SNET",
  "K_COMPONENTPIN", "K_REENTRANTPATHS", "K_SHIELD", "K_SHIELDNET",
  "K_NOSHIELD", "K_ANTENNAPINPARTIALMETALAREA",
  "K_ANTENNAPINPARTIALMETALSIDEAREA", "K_ANTENNAPINGATEAREA",
  "K_ANTENNAPINDIFFAREA", "K_ANTENNAPINMAXAREACAR",
  "K_ANTENNAPINMAXSIDEAREACAR", "K_ANTENNAPINPARTIALCUTAREA",
  "K_ANTENNAPINMAXCUTCAR", "K_SIGNAL", "K_POWER", "K_GROUND", "K_CLOCK",
  "K_TIEOFF", "K_ANALOG", "K_SCAN", "K_RESET", "K_RING", "K_STRIPE",
  "K_FOLLOWPIN", "K_IOWIRE", "K_COREWIRE", "K_BLOCKWIRE", "K_FILLWIRE",
  "K_BLOCKAGEWIRE", "K_PADRING", "K_BLOCKRING", "K_BLOCKAGES",
  "K_PLACEMENT", "K_SLOTS", "K_FILLS", "K_PUSHDOWN", "K_NETLIST", "K_DIST",
  "K_USER", "K_TIMING", "K_BALANCED", "K_STEINER", "K_TRUNK",
  "K_FIXEDBUMP", "K_FENCE", "K_FREQUENCY", "K_GUIDE", "K_MAXBITS",
  "K_PARTITION", "K_TYPE", "K_ANTENNAMODEL", "K_DRCFILL", "K_OXIDE1",
  "K_OXIDE2", "K_OXIDE3", "K_OXIDE4", "';'", "'-'", "'+'", "'('", "')'",
  "'*'", "','", "$accept", "def_file", "version_stmt", "$@1",
  "case_sens_stmt", "rules", "rule", "design_section", "design_name",
  "$@2", "end_design", "tech_name", "$@3", "array_name", "$@4",
  "floorplan_name", "$@5", "history", "prop_def_section", "$@6",
  "property_defs", "property_def", "$@7", "$@8", "$@9", "$@10", "$@11",
  "$@12", "$@13", "$@14", "$@15", "property_type_and_val", "$@16", "$@17",
  "opt_num_val", "units", "divider_char", "bus_bit_chars", "site", "$@18",
  "canplace", "$@19", "cannotoccupy", "$@20", "orient", "die_area",
  "pin_cap_rule", "start_def_cap", "pin_caps", "pin_cap", "end_def_cap",
  "pin_rule", "start_pins", "pins", "pin", "$@21", "$@22", "$@23",
  "pin_options", "pin_option", "$@24", "$@25", "$@26", "$@27", "pin_oxide",
  "use_type", "pin_layer_opt", "$@28", "end_pins", "row_rule", "$@29",
  "$@30", "row_options", "row_option", "$@31", "row_prop_list", "row_prop",
  "tracks_rule", "$@32", "track_start", "track_type",
  "track_layer_statement", "$@33", "track_layers", "track_layer",
  "gcellgrid", "extension_section", "extension_stmt", "via_section", "via",
  "via_declarations", "via_declaration", "$@34", "$@35", "layer_stmts",
  "layer_stmt", "$@36", "$@37", "pt", "via_end", "regions_section",
  "regions_start", "regions_stmts", "regions_stmt", "$@38", "$@39",
  "rect_list", "region_options", "region_option", "$@40",
  "region_prop_list", "region_prop", "region_type", "comps_section",
  "start_comps", "comps_rule", "comp", "comp_start", "comp_id_and_name",
  "$@41", "comp_net_list", "comp_options", "comp_option",
  "comp_extension_stmt", "comp_eeq", "$@42", "comp_generate", "$@43",
  "opt_pattern", "comp_source", "source_type", "comp_region",
  "comp_pnt_list", "comp_property", "$@44", "comp_prop_list", "comp_prop",
  "comp_region_start", "comp_foreign", "$@45", "opt_paren", "comp_type",
  "placement_status", "weight", "end_comps", "nets_section", "start_nets",
  "net_rules", "one_net", "net_and_connections", "net_start", "$@46",
  "net_name", "$@47", "$@48", "net_connections", "net_connection", "$@49",
  "$@50", "$@51", "conn_opt", "net_options", "net_option", "$@52", "$@53",
  "$@54", "$@55", "$@56", "$@57", "$@58", "$@59", "$@60", "$@61", "$@62",
  "net_prop_list", "net_prop", "netsource_type", "vpin_stmt", "vpin_begin",
  "$@63", "vpin_layer_opt", "$@64", "vpin_options", "vpin_status",
  "net_type", "paths", "new_path", "$@65", "path", "$@66", "$@67",
  "path_item_list", "path_item", "path_pt", "opt_taper", "$@68",
  "opt_shape", "opt_width", "end_nets", "shape_type", "snets_section",
  "snet_rules", "snet_rule", "snet_options", "snet_option",
  "snet_other_option", "$@69", "$@70", "$@71", "$@72", "$@73", "$@74",
  "$@75", "snet_width", "$@76", "snet_voltage", "snet_spacing", "$@77",
  "$@78", "snet_prop_list", "snet_prop", "opt_snet_range", "opt_range",
  "pattern_type", "spaths", "snew_path", "$@79", "spath", "$@80", "$@81",
  "width", "start_snets", "end_snets", "groups_section", "groups_start",
  "group_rules", "group_rule", "start_group", "$@82", "group_members",
  "group_member", "group_options", "group_option", "$@83", "$@84",
  "group_region", "group_prop_list", "group_prop", "group_soft_options",
  "group_soft_option", "groups_end", "assertions_section",
  "constraint_section", "assertions_start", "constraints_start",
  "constraint_rules", "constraint_rule", "operand_rule", "operand", "$@85",
  "$@86", "operand_list", "wiredlogic_rule", "$@87", "opt_plus",
  "delay_specs", "delay_spec", "constraints_end", "assertions_end",
  "scanchains_section", "scanchain_start", "scanchain_rules", "scan_rule",
  "start_scan", "$@88", "scan_members", "opt_pin", "scan_member", "$@89",
  "$@90", "$@91", "$@92", "$@93", "$@94", "opt_common_pins",
  "floating_inst_list", "one_floating_inst", "$@95", "floating_pins",
  "ordered_inst_list", "one_ordered_inst", "$@96", "ordered_pins",
  "partition_maxbits", "scanchain_end", "iotiming_section",
  "iotiming_start", "iotiming_rules", "iotiming_rule", "start_iotiming",
  "$@97", "iotiming_members", "iotiming_member", "$@98", "$@99",
  "iotiming_drivecell_opt", "$@100", "$@101", "iotiming_frompin", "$@102",
  "iotiming_parallel", "risefall", "iotiming_end",
  "floorplan_contraints_section", "fp_start", "fp_stmts", "fp_stmt",
  "$@103", "$@104", "h_or_v", "constraint_type", "constrain_what_list",
  "constrain_what", "$@105", "$@106", "row_or_comp_list", "row_or_comp",
  "$@107", "$@108", "timingdisables_section", "timingdisables_start",
  "timingdisables_rules", "timingdisables_rule", "$@109", "$@110", "$@111",
  "$@112", "td_macro_option", "$@113", "$@114", "$@115",
  "timingdisables_end", "partitions_section", "partitions_start",
  "partition_rules", "partition_rule", "start_partition", "$@116",
  "turnoff", "turnoff_setup", "turnoff_hold", "partition_members",
  "partition_member", "$@117", "$@118", "$@119", "$@120", "$@121", "$@122",
  "minmaxpins", "$@123", "min_or_max_list", "min_or_max_member",
  "pin_list", "risefallminmax1_list", "risefallminmax1",
  "risefallminmax2_list", "risefallminmax2", "partitions_end",
  "comp_names", "comp_name", "$@124", "subnet_opt_syn", "subnet_options",
  "subnet_option", "$@125", "$@126", "subnet_type", "pin_props_section",
  "begin_pin_props", "opt_semi", "end_pin_props", "pin_prop_list",
  "pin_prop_terminal", "$@127", "$@128", "pin_prop_options", "pin_prop",
  "$@129", "pin_prop_name_value_list", "pin_prop_name_value",
  "blockage_section", "blockage_start", "blockage_end", "blockage_defs",
  "blockage_def", "blockage_rule", "$@130", "$@131", "$@132",
  "comp_blockage_rule", "$@133", "placement_comp_rule", "$@134",
  "rect_blockage_rules", "rect_blockage", "slot_section", "slot_start",
  "slot_end", "slot_defs", "slot_def", "slot_rule", "$@135", "$@136",
  "rect_slot_rules", "rect_slot", "fill_section", "fill_start", "fill_end",
  "fill_defs", "fill_def", "fill_rule", "$@137", "$@138",
  "rect_fill_rules", "rect_fill", 0
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
     485,   486,   487,   488,    59,    45,    43,    40,    41,    42,
      44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   241,   242,   243,   244,   243,   245,   245,   245,   246,
     246,   246,   247,   247,   247,   247,   247,   247,   247,   247,
     247,   247,   247,   247,   247,   247,   247,   247,   247,   247,
     247,   248,   248,   248,   248,   248,   248,   248,   248,   248,
     248,   248,   248,   248,   248,   248,   248,   248,   248,   250,
     249,   251,   253,   252,   255,   254,   257,   256,   258,   260,
     259,   261,   261,   263,   262,   264,   262,   265,   262,   266,
     262,   267,   262,   268,   262,   269,   262,   270,   262,   271,
     262,   262,   273,   272,   274,   272,   272,   272,   272,   275,
     275,   276,   277,   278,   280,   279,   282,   281,   284,   283,
     285,   285,   285,   285,   285,   285,   285,   285,   286,   287,
     288,   289,   289,   290,   291,   292,   293,   294,   294,   296,
     297,   298,   295,   299,   299,   300,   300,   300,   300,   301,
     300,   300,   300,   300,   300,   300,   302,   300,   303,   300,
     300,   304,   300,   300,   305,   305,   305,   305,   306,   306,
     306,   306,   306,   306,   306,   306,   307,   308,   307,   309,
     311,   312,   310,   313,   313,   315,   314,   316,   316,   317,
     317,   317,   319,   318,   320,   321,   321,   322,   323,   322,
     324,   324,   325,   326,   327,   328,   329,   330,   331,   331,
     333,   334,   332,   335,   335,   337,   336,   338,   336,   336,
     339,   339,   339,   339,   340,   341,   342,   343,   343,   345,
     346,   344,   347,   347,   348,   348,   350,   349,   349,   351,
     351,   352,   352,   352,   353,   353,   354,   355,   356,   356,
     357,   358,   360,   359,   361,   361,   361,   362,   362,   363,
     363,   363,   363,   363,   363,   363,   363,   363,   364,   366,
     365,   368,   367,   369,   369,   370,   371,   371,   371,   371,
     372,   372,   373,   373,   375,   374,   376,   376,   377,   377,
     377,   378,   380,   379,   381,   381,   382,   382,   382,   383,
     383,   383,   384,   385,   386,   387,   388,   388,   389,   390,
     392,   391,   394,   393,   395,   393,   396,   396,   398,   397,
     399,   397,   400,   397,   401,   401,   401,   402,   402,   404,
     405,   403,   403,   403,   403,   406,   403,   403,   403,   403,
     403,   403,   407,   403,   408,   403,   403,   409,   403,   410,
     411,   403,   412,   413,   403,   414,   403,   403,   415,   415,
     416,   416,   416,   417,   417,   417,   417,   417,   418,   420,
     419,   421,   422,   421,   423,   423,   424,   424,   424,   425,
     425,   425,   426,   426,   428,   427,   430,   431,   429,   432,
     432,   433,   433,   433,   433,   434,   434,   434,   434,   434,
     434,   434,   434,   435,   435,   436,   435,   437,   437,   438,
     438,   439,   440,   440,   440,   440,   440,   440,   440,   440,
     440,   440,   440,   441,   442,   442,   443,   444,   444,   445,
     445,   445,   445,   447,   448,   446,   449,   450,   446,   446,
     446,   446,   451,   446,   446,   446,   446,   446,   452,   446,
     453,   446,   446,   455,   454,   456,   458,   459,   457,   460,
     460,   461,   461,   461,   462,   462,   463,   463,   464,   464,
     464,   464,   465,   465,   467,   466,   469,   470,   468,   471,
     472,   473,   474,   475,   476,   476,   477,   479,   478,   480,
     480,   481,   482,   482,   483,   484,   483,   485,   483,   483,
     486,   486,   487,   487,   488,   488,   488,   489,   489,   490,
     490,   490,   491,   492,   493,   494,   495,   496,   496,   497,
     497,   498,   500,   499,   501,   499,   499,   499,   502,   502,
     504,   503,   505,   505,   506,   506,   507,   507,   507,   507,
     508,   509,   510,   511,   512,   512,   513,   515,   514,   516,
     516,   517,   517,   519,   518,   520,   518,   521,   518,   522,
     518,   523,   518,   524,   518,   518,   525,   525,   525,   526,
     526,   528,   527,   529,   529,   529,   529,   530,   530,   532,
     531,   533,   533,   533,   533,   534,   534,   535,   536,   537,
     538,   538,   539,   541,   540,   542,   542,   543,   543,   543,
     544,   545,   543,   543,   547,   548,   546,   549,   550,   549,
     551,   551,   552,   552,   553,   554,   555,   556,   556,   558,
     559,   557,   560,   560,   561,   561,   561,   561,   562,   562,
     564,   563,   565,   563,   566,   566,   568,   567,   569,   567,
     570,   571,   572,   572,   574,   575,   573,   576,   573,   577,
     573,   573,   579,   580,   578,   581,   578,   582,   583,   584,
     585,   585,   586,   588,   587,   589,   589,   590,   590,   590,
     591,   591,   591,   592,   592,   594,   593,   595,   593,   596,
     593,   597,   593,   598,   593,   599,   593,   593,   601,   600,
     602,   602,   603,   603,   604,   604,   605,   605,   606,   606,
     606,   606,   607,   607,   608,   608,   608,   608,   609,   610,
     610,   612,   611,   613,   613,   614,   614,   616,   615,   617,
     615,   618,   618,   618,   618,   619,   620,   621,   621,   622,
     623,   623,   625,   626,   624,   627,   627,   629,   628,   630,
     630,   631,   631,   631,   632,   633,   634,   635,   635,   636,
     638,   639,   637,   640,   637,   641,   642,   641,   641,   641,
     641,   643,   644,   643,   643,   645,   645,   646,   647,   648,
     649,   650,   650,   651,   653,   654,   652,   655,   655,   656,
     657,   658,   659,   660,   660,   661,   663,   664,   662,   665,
     665,   666
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     4,     0,     0,     4,     0,     3,     3,     0,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       4,     2,     0,     4,     0,     4,     0,     4,     1,     0,
       5,     0,     2,     0,     5,     0,     5,     0,     5,     0,
       5,     0,     5,     0,     5,     0,     5,     0,     5,     0,
       5,     2,     0,     4,     0,     4,     1,     2,     2,     0,
       1,     5,     3,     3,     0,    14,     0,    14,     0,    14,
       1,     1,     1,     1,     1,     1,     1,     1,     4,     3,
       2,     0,     2,     5,     2,     3,     3,     0,     2,     0,
       0,     0,    10,     0,     2,     2,     1,     3,     3,     0,
       6,     3,     4,     4,     4,     4,     0,     6,     0,     6,
       4,     0,     6,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     0,     3,     2,
       0,     0,    17,     0,     2,     0,     4,     0,     2,     2,
       2,     2,     0,     9,     2,     1,     1,     0,     0,     4,
       0,     2,     1,     8,     1,     2,     3,     3,     0,     2,
       0,     0,     6,     0,     2,     0,     6,     0,     4,     1,
       4,     4,     4,     4,     2,     4,     3,     0,     2,     0,
       0,     7,     2,     3,     0,     2,     0,     4,     3,     0,
       2,     2,     2,     2,     1,     1,     3,     3,     0,     2,
       3,     2,     0,     4,     0,     2,     2,     0,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       4,     0,     5,     0,     1,     3,     1,     1,     1,     1,
       2,     2,     2,     3,     0,     4,     1,     2,     2,     2,
       2,     2,     0,     6,     1,     2,     3,     2,     4,     2,
       2,     2,     3,     2,     3,     3,     0,     2,     3,     1,
       0,     3,     0,     3,     0,     6,     0,     2,     0,     6,
       0,     6,     0,     6,     0,     1,     2,     0,     2,     0,
       0,     5,     3,     2,     3,     0,     4,     3,     3,     3,
       3,     3,     0,     4,     0,     4,     1,     0,     4,     0,
       0,     5,     0,     0,     7,     0,     4,     1,     1,     2,
       2,     2,     2,     1,     1,     1,     1,     1,     5,     0,
       4,     0,     0,     3,     0,     3,     1,     1,     1,     1,
       1,     1,     1,     2,     0,     3,     0,     0,     8,     0,
       2,     1,     2,     8,     1,     4,     4,     4,     4,     5,
       5,     5,     5,     0,     1,     0,     3,     0,     3,     0,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     0,     2,     3,     0,     2,     1,
       1,     1,     1,     0,     0,     5,     0,     0,     6,     3,
       2,     3,     0,     4,     3,     3,     3,     3,     0,     4,
       0,     4,     1,     0,     5,     3,     0,     0,     7,     1,
       2,     2,     2,     2,     0,     3,     0,     3,     1,     1,
       1,     1,     1,     2,     0,     3,     0,     0,     7,     1,
       3,     2,     3,     3,     0,     2,     4,     0,     3,     0,
       2,     1,     0,     2,     3,     0,     4,     0,     4,     1,
       2,     1,     0,     2,     2,     2,     2,     0,     2,     2,
       2,     2,     2,     3,     3,     3,     3,     0,     2,     1,
       1,     4,     0,     3,     0,     6,     4,     4,     1,     3,
       0,     8,     0,     1,     0,     2,     3,     3,     3,     3,
       2,     2,     3,     3,     0,     2,     3,     0,     3,     0,
       2,     0,     1,     0,     5,     0,     4,     0,     4,     0,
       5,     0,     4,     0,     5,     1,     0,     4,     8,     0,
       2,     0,     3,     0,     4,     8,    12,     0,     2,     0,
       3,     0,     4,     8,    12,     0,     2,     2,     3,     3,
       0,     2,     3,     0,     6,     0,     2,     5,     5,     3,
       0,     0,     6,     1,     0,     0,     6,     0,     0,     3,
       0,     2,     1,     1,     2,     4,     3,     0,     2,     0,
       0,     8,     1,     1,     1,     2,     2,     2,     0,     2,
       0,     4,     0,     4,     0,     2,     0,     5,     0,     5,
       3,     3,     0,     2,     0,     0,    10,     0,     6,     0,
       6,     3,     0,     0,     6,     0,     3,     2,     3,     3,
       0,     2,     3,     0,     4,     0,     3,     0,     1,     1,
       0,     1,     1,     0,     2,     0,     7,     0,     6,     0,
       5,     0,     7,     0,     6,     0,     5,     1,     0,     4,
       0,     2,     3,     3,     0,     2,     0,     2,     2,     2,
       2,     2,     1,     2,     3,     3,     3,     3,     2,     0,
       2,     0,     6,     0,     2,     0,     2,     0,     3,     0,
       3,     1,     1,     1,     1,     3,     3,     0,     1,     2,
       0,     2,     0,     0,     7,     0,     2,     0,     4,     0,
       2,     2,     2,     2,     3,     3,     2,     0,     2,     4,
       0,     0,     6,     0,     4,     0,     0,     4,     2,     2,
       2,     0,     0,     4,     2,     0,     2,     3,     3,     3,
       2,     0,     2,     3,     0,     0,     6,     0,     2,     3,
       3,     3,     2,     0,     2,     3,     0,     0,     6,     0,
       2,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       3,     4,     0,     6,     0,     1,     0,     0,     0,     0,
       0,    11,     0,     5,     7,     8,    58,    49,     0,    52,
       0,    54,    56,    94,    96,    98,     0,     0,     0,     0,
       0,   160,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    59,   184,     0,
       0,     0,     0,    10,    12,    31,     2,    32,    45,    46,
      33,    48,    34,    35,    36,    37,    38,    39,    40,    41,
     111,    42,   117,    47,    43,     0,    44,    14,    13,   188,
      23,   207,    15,   228,    16,   286,    17,   404,    18,   464,
      21,    20,   497,   497,    19,   524,    22,   570,    24,   597,
      25,   622,    26,   640,    27,   710,    28,   727,    29,   751,
      30,   763,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   110,   175,   176,   174,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    51,     0,
       0,     0,     0,     0,     0,    61,   707,     0,     0,     0,
       0,     0,   172,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   187,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   116,     0,     0,   227,   206,   285,   460,   463,
     496,   523,   495,   569,   621,   639,   596,    92,    93,     0,
     708,   706,   725,   749,   761,     0,     0,   112,   109,     0,
     119,   118,   115,     0,     0,   190,   189,   186,     0,   209,
     208,     0,   232,   229,   237,   234,   226,     0,   290,   287,
     307,   289,   284,     0,   407,   405,   403,     0,   467,   465,
     469,   462,     0,     0,   498,   499,   500,   493,     0,   494,
       0,   527,   525,   529,   522,     0,     0,   571,   575,   568,
       0,   599,   598,     0,     0,   623,   620,     0,   643,   641,
     653,   638,     0,   712,   705,   711,     0,     0,   724,   728,
       0,     0,     0,   748,   752,   757,     0,     0,   760,   764,
     769,    50,    53,     0,    55,    57,     0,     0,     0,     0,
       0,     0,     0,   108,     0,     0,     0,    63,    75,    69,
      71,    65,     0,    77,    73,    67,    79,    62,     0,   114,
     159,     0,     0,   204,     0,   205,     0,   283,     0,     0,
     231,   391,     0,     0,   461,     0,   492,     0,   472,   521,
     502,   504,     0,     0,   510,   514,   520,   567,     0,     0,
     594,   573,     0,   595,     0,   637,   624,   627,   629,     0,
     688,     0,     0,   709,     0,   726,   730,   733,     0,   745,
     750,   754,     0,   762,   766,     0,    91,     0,     0,     0,
     200,   202,   201,   203,     0,     0,    81,     0,     0,     0,
       0,     0,    60,     0,     0,     0,     0,     0,     0,     0,
     191,   210,     0,   230,     0,   248,   238,   247,   245,   239,
     240,   244,   246,     0,   243,   241,     0,   242,   236,   235,
     292,     0,   291,   288,     0,   337,   308,   326,   351,   406,
       0,   432,   408,   412,   409,   410,   411,   468,   471,   470,
       0,     0,     0,     0,     0,     0,     0,   528,   526,     0,
     545,   530,     0,   572,     0,   583,   576,     0,     0,     0,
       0,   631,   645,   642,     0,   667,   654,     0,     0,   741,
       0,     0,     0,     0,   753,   758,     0,     0,   765,   770,
     100,   102,   103,   101,   104,   107,   106,   105,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   193,     0,   233,   251,     0,
       0,   249,   279,   280,   277,   281,   272,   271,   264,   185,
     261,     0,   260,     0,   296,     0,     0,     0,   359,   360,
     315,     0,   322,     0,     0,   361,   324,   349,   332,     0,
     335,   327,   329,   313,     0,   309,   352,     0,     0,     0,
     422,     0,   428,     0,     0,   433,     0,   436,   430,   416,
     420,     0,   413,   466,     0,   479,   473,   503,     0,   508,
       0,     0,   512,   501,     0,   515,   533,   535,   537,   539,
     541,   543,     0,   592,   593,     0,   580,     0,   602,   603,
     600,     0,     0,     0,   647,   644,   655,   657,   659,   661,
     663,   665,   713,   731,     0,   734,   747,   729,   746,   755,
       0,   767,     0,     0,     0,     0,     0,     0,    86,    84,
      82,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   113,   120,   177,     0,     0,   214,     0,   256,   257,
     258,   259,   255,   282,     0,     0,     0,     0,   262,     0,
     276,   293,   294,   347,   343,   344,   345,   346,   312,   318,
       0,   148,   149,   150,   151,   152,   153,   154,   155,   321,
       0,   451,   448,   449,   450,   317,   320,     0,     0,     0,
     319,     0,     0,   330,   314,   310,     0,     0,   419,   425,
       0,   427,     0,   424,   426,     0,   435,     0,     0,     0,
     421,   414,   477,   487,   475,     0,   506,     0,   507,   513,
       0,     0,     0,     0,     0,     0,   549,   557,     0,   546,
       0,     0,   579,     0,     0,     0,     0,     0,     0,   632,
     635,     0,   648,   649,   650,     0,     0,     0,     0,     0,
       0,   715,   735,   742,   744,     0,   759,     0,   771,     0,
       0,     0,   183,     0,    87,   446,   446,    88,    64,    76,
      70,    72,    66,    78,    74,    68,    80,     0,   178,     0,
     192,     0,   199,   194,   212,     0,     0,   253,   250,   278,
       0,     0,   265,   266,   263,     0,   297,     0,   316,   323,
     325,   350,   333,     0,   336,   338,   328,     0,     0,   353,
     354,   423,   429,     0,     0,     0,   431,   439,   417,     0,
       0,   474,   482,     0,   509,     0,   516,   517,   518,   519,
     531,   536,   538,   531,     0,   542,   565,   574,   581,     0,
       0,   604,     0,     0,     0,   608,   625,   628,     0,     0,
     630,   651,   652,   646,     0,     0,   676,     0,     0,   676,
       0,     0,   732,     0,   756,   768,     0,     0,     0,     0,
       0,    89,    89,   121,     0,   173,   195,   197,   213,   211,
       0,   215,   254,   252,     0,   274,     0,   269,   270,   268,
     267,   298,   302,   300,     0,   689,   341,   342,   340,   339,
     366,   331,   362,   311,   357,   358,   356,   348,     0,   434,
     437,   442,   443,   441,   440,     0,   456,   415,   452,   481,
       0,   478,     0,     0,     0,   488,   476,   505,     0,   532,
     534,   551,   550,   559,   558,   540,     0,     0,   544,   587,
     577,   578,   606,   605,   607,     0,     0,     0,   636,     0,
       0,   660,     0,     0,   666,   714,     0,   716,   736,   738,
     739,   740,   743,     0,     0,     0,     0,     0,    90,    85,
      83,   123,   182,   180,     0,     0,   216,     0,   275,   273,
       0,     0,     0,   295,   695,   389,   364,   363,     0,   444,
     418,     0,   454,   453,   480,   489,   490,   491,     0,   483,
     511,   553,   561,     0,   566,   588,   582,     0,   601,     0,
     609,     0,   633,   670,     0,     0,     0,     0,   658,   682,
       0,     0,     0,     0,   677,   670,   664,   717,     0,     0,
       0,     0,     0,   447,     0,   179,   180,     0,   198,   219,
     224,   225,   218,   304,   304,   304,   691,   690,   334,   390,
     387,     0,   355,     0,   438,   459,   387,     0,   485,   486,
     484,     0,   552,     0,   560,   547,     0,   584,   610,   612,
       0,     0,   656,     0,     0,     0,     0,     0,   683,   678,
     680,   679,   681,   662,   719,   737,     0,     0,     0,     0,
     122,     0,   124,   126,     0,   181,     0,   217,     0,   305,
       0,     0,     0,     0,   701,   702,   703,   699,   704,   696,
     697,     0,   383,   365,     0,     0,   455,     0,     0,     0,
     589,     0,   614,   614,   626,   634,   668,     0,     0,   671,
     684,   686,   685,   687,   718,     0,     0,     0,     0,   129,
       0,   125,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   196,     0,   220,   306,   299,   303,   301,
       0,     0,     0,     0,   384,   385,     0,   445,     0,   457,
       0,     0,     0,   585,   611,   613,   674,     0,     0,     0,
     720,    95,    97,    99,   161,     0,   128,   127,   156,   156,
     156,   156,     0,     0,   156,     0,   144,   145,   146,   147,
     143,   131,   222,   223,   221,   693,   700,   698,   392,   393,
     394,   395,   396,   397,   398,   400,   401,   402,   399,   388,
       0,   367,     0,     0,   369,   554,   562,     0,   590,     0,
     615,   669,   672,   673,   722,   723,   721,   163,     0,   157,
     132,   133,   134,   135,   136,   138,   140,   141,     0,     0,
     386,   369,     0,     0,     0,     0,   458,     0,     0,   548,
       0,   586,   618,   616,   675,     0,     0,     0,     0,     0,
       0,   694,   692,   368,     0,   375,     0,   377,     0,   376,
       0,   378,   371,   370,   374,     0,     0,   591,     0,     0,
     162,     0,   164,   130,   158,   137,   139,   142,   379,   381,
     380,   382,     0,   372,     0,     0,     0,     0,   165,     0,
     555,   563,   619,   617,   167,     0,     0,     0,   166,     0,
       0,     0,     0,   168,     0,     0,     0,   170,   171,   169,
       0,   556,   564,   373
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     4,     7,    12,    53,    54,    55,   112,
      56,    57,   114,    58,   116,    59,   117,    60,    61,   145,
     199,   317,   387,   391,   395,   389,   390,   394,   388,   393,
     396,   622,   756,   755,   959,    62,    63,    64,    65,   118,
      66,   119,    67,   120,   488,    68,    69,    70,   150,   207,
     208,    71,    72,   151,   211,   321,   767,   961,  1024,  1082,
    1175,  1258,  1259,  1260,  1190,   669,  1230,  1257,   212,    73,
     129,  1227,  1255,  1282,  1304,  1308,  1313,    74,   213,    75,
     127,   769,   864,  1025,  1026,    76,    77,  1089,    78,    79,
     153,   216,   324,   505,   634,   773,   964,   965,   122,   217,
      80,    81,   154,   220,   326,   506,   636,   776,   871,  1029,
    1087,  1145,  1032,    82,    83,   155,   223,   224,   225,   328,
     330,   329,   406,   407,   408,   644,   409,   637,   873,   410,
     642,   411,   522,   412,   647,   782,   783,   413,   414,   646,
     876,   415,   416,   417,   226,    84,    85,   156,   229,   230,
     231,   332,   422,   524,   787,   651,   786,   970,   972,   971,
    1090,   333,   426,   685,   798,   660,   670,   677,   682,   683,
     797,   679,   885,   681,   794,   795,   658,   427,   428,   678,
     547,   686,   897,   898,   545,   891,   977,  1041,   892,   975,
    1241,  1246,  1273,  1274,  1156,  1210,  1102,  1040,   232,  1209,
      86,   157,   235,   335,   432,   433,   701,   809,   699,   905,
     690,   692,   698,   434,   695,   435,   436,   697,   979,   806,
     807,  1044,   861,   675,   907,   983,  1047,   908,   981,  1214,
    1046,    87,   236,    88,    89,   158,   239,   240,   337,   338,
     439,   440,   566,   812,   810,   911,   916,   989,   811,   915,
     241,    90,    91,    92,    93,   159,   244,   245,   569,   441,
     442,   570,   246,   445,   710,   446,   575,   249,   247,    94,
      95,   161,   252,   253,   348,   349,   920,   451,   715,   716,
     717,   718,   719,   720,   825,   821,   922,   991,  1052,   822,
     924,   992,  1054,   928,   254,    96,    97,   162,   257,   258,
     452,   352,   456,   723,   929,   996,  1111,  1218,   997,  1056,
    1251,   587,   259,    98,    99,   163,   262,   354,   726,   590,
     835,   935,  1000,  1112,  1113,  1164,  1220,  1279,  1278,   100,
     101,   164,   265,   458,   936,   459,   460,   731,   838,  1061,
     839,   266,   102,   103,   165,   269,   270,   361,   595,   734,
     843,   362,   466,   735,   736,   737,   738,   739,   740,  1062,
    1166,  1063,  1119,  1221,   941,  1014,  1008,  1009,   271,   974,
    1037,  1093,  1239,  1038,  1099,  1152,  1151,  1100,   104,   105,
     201,   274,   166,   275,   364,   741,   850,   947,  1074,  1124,
    1170,   106,   107,   278,   167,   279,   280,   468,   742,   469,
     852,  1018,   605,   853,   471,   369,   108,   109,   283,   168,
     284,   285,   472,   745,   372,   475,   110,   111,   288,   169,
     289,   290,   476,   747,   375,   479
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -1079
static const yytype_int16 yypact[] =
{
     -58, -1079,   108,   127,   143, -1079,   -83,    30,   -81,   -75,
     -69, -1079,   311, -1079, -1079, -1079, -1079, -1079,   181, -1079,
     106, -1079, -1079, -1079, -1079, -1079,   -28,   217,   234,    47,
      47, -1079,   252,   261,   268,   279,   282,   292,   306,   335,
     382,   360,   395,   399,   410,   429,   446, -1079, -1079,   461,
     477,   483,   506, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079,   513, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079,   517,   290,   521,   432,   524,   525,   535,   536,
     537,     5,   -28,   308, -1079, -1079, -1079, -1079,   538,   539,
     312,   313,   314,   315,   316,   317,   318,   319, -1079,   320,
     321,   322,   323,   324,   325, -1079,   326,   327,   328,   329,
      35,   -63, -1079,   -62,   -60,   -57,   -43,   -40,   -38,   -37,
     -35,   -33,   -32,   -27,   -24,   -14,    -3,    -2,    -1,     0,
     330, -1079,   332,   563,   336,   337,   566,   567,   568,     8,
       9,   341, -1079,   520,   572, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,   274,
   -1079, -1079, -1079, -1079, -1079,   571,   558, -1079, -1079,   560,
   -1079, -1079, -1079,   555,   574, -1079, -1079, -1079,   540, -1079,
   -1079,   550, -1079, -1079, -1079, -1079, -1079,   542, -1079, -1079,
   -1079, -1079, -1079,   532, -1079, -1079, -1079,   519, -1079, -1079,
   -1079, -1079,   496,   188, -1079, -1079, -1079, -1079,   515, -1079,
     511, -1079, -1079, -1079, -1079,   495,   355, -1079, -1079, -1079,
     447, -1079, -1079,   488,    26, -1079, -1079,   486, -1079, -1079,
   -1079, -1079,   422, -1079, -1079, -1079,   388,    -9, -1079, -1079,
     569,   387,   575, -1079, -1079, -1079,   392,   577, -1079, -1079,
   -1079, -1079, -1079,   366, -1079, -1079,   596,   600,   601,   370,
     371,   372,   374, -1079,   605,   607,   380, -1079, -1079, -1079,
   -1079, -1079,   454, -1079, -1079, -1079, -1079, -1079,   594, -1079,
   -1079,   613,   612, -1079,   615, -1079,   616, -1079,   617,   -55,
       6, -1079,    78,   -48, -1079,    48, -1079,   618,   619, -1079,
   -1079, -1079,   389,   391, -1079, -1079, -1079, -1079,   620,    83,
   -1079, -1079,   103, -1079,   621, -1079, -1079, -1079, -1079,   393,
   -1079,   625,   116, -1079,   626, -1079, -1079, -1079,   -28, -1079,
   -1079, -1079,   -18, -1079, -1079,   -17, -1079,   348,   348,   348,
   -1079, -1079, -1079, -1079,   604,   627, -1079,   628,   630,   631,
     632,   633, -1079,   634,   635,   636,   637,   638,   406,   622,
   -1079, -1079,   639, -1079,   402, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079,    12, -1079, -1079,   -28, -1079, -1079, -1079,
   -1079,   408, -1079, -1079,   115, -1079, -1079, -1079,   623, -1079,
     253, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
     161,   642,   643,    93,    93,   644,   168, -1079, -1079,   -16,
   -1079, -1079,   646, -1079,    40, -1079, -1079,   159,   648,   650,
     651, -1079,   546, -1079,   260, -1079, -1079,   653,   655,   425,
     -28,   -12,   656,   -28, -1079, -1079,   658,   -28, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,   640,   641,
     645,   657,   348,   -52,   -52,   -52,   -52,   -52,   -52,   -52,
     -52,   -52,   430,   592,   662, -1079,   -28, -1079, -1079,    56,
     663, -1079, -1079, -1079,   -28, -1079, -1079, -1079, -1079, -1079,
   -1079,   -28,   -28,   348, -1079,   667,   -96,   666, -1079, -1079,
   -1079,   300, -1079,    -5,   668, -1079, -1079, -1079, -1079,   669,
   -1079, -1079, -1079, -1079,   670, -1079, -1079,   -28,    56,   671,
   -1079,   300, -1079,    -5,   672, -1079,   673, -1079, -1079, -1079,
   -1079,   674, -1079, -1079,    86, -1079, -1079, -1079,   677, -1079,
     191,   210,   437, -1079,   258, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079,   678, -1079, -1079,   679, -1079,    94, -1079, -1079,
   -1079,   680,   682,    10,   215, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079,   -82, -1079, -1079, -1079, -1079, -1079,
     -28, -1079,   -28,   681,   683,   684,   449,   675,   688, -1079,
   -1079,   689,   458,   460,   462,   463,   465,   467,   468,   469,
     470, -1079, -1079,   685,   219,   -28,   -28,   691, -1079, -1079,
   -1079, -1079, -1079, -1079,   701,   348,   702,   703, -1079,   -28,
   -1079,   471, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
     705, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
     706, -1079, -1079, -1079, -1079, -1079, -1079,   707,   708,   710,
   -1079,   711,   712, -1079, -1079, -1079,   713,   -28, -1079, -1079,
     714, -1079,   715, -1079, -1079,   716, -1079,   717,   718,   719,
   -1079, -1079, -1079, -1079, -1079,   720, -1079,    93, -1079, -1079,
     647,   721,   722,   723,   724,   727, -1079, -1079,   728,   489,
     729,   487, -1079,   730,   731,   733,   207,   649,   501, -1079,
   -1079,   502, -1079, -1079,   367,   736,   737,   738,   739,   740,
     741, -1079,   510, -1079, -1079,   725, -1079,   726, -1079,   732,
     734,   735, -1079,   742, -1079,   602,   602, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079,   745, -1079,   516,
   -1079,    46, -1079, -1079, -1079,   -28,   220,   747, -1079, -1079,
      19,   151,   703, -1079, -1079,     4, -1079,   748, -1079, -1079,
   -1079, -1079, -1079,   273,   711, -1079, -1079,   750,   750, -1079,
     256, -1079, -1079,   751,   753,   310,   718, -1079, -1079,   758,
      13,   373, -1079,   759, -1079,   760, -1079, -1079, -1079, -1079,
     763,   764,   765,   763,   766, -1079,   547, -1079, -1079,   767,
     768, -1079,   769,   771,   772, -1079, -1079, -1079,   775,   776,
   -1079, -1079, -1079, -1079,   777,   778, -1079,   779,   780, -1079,
     232,   -68, -1079,   781, -1079, -1079,   783,   784,   785,   761,
     786,   787,   787, -1079,   782, -1079, -1079, -1079, -1079, -1079,
     -95, -1079, -1079, -1079,   788, -1079,   348, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079,   526, -1079, -1079, -1079, -1079, -1079,
   -1079,   743, -1079,   743, -1079, -1079, -1079, -1079,   -28, -1079,
   -1079, -1079, -1079, -1079, -1079,   758, -1079,   744, -1079, -1079,
     -28, -1079,   789,   792,   793, -1079,   796, -1079,   570, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079,   797,   799, -1079,   699,
   -1079, -1079, -1079, -1079, -1079,   239,   802,   704, -1079,   280,
     275,   294,   280,   275,   294, -1079,   606, -1079, -1079, -1079,
   -1079, -1079, -1079,   790,   791,   794,   801,   803, -1079, -1079,
   -1079, -1079, -1079,   782,   806,   807, -1079,   257, -1079, -1079,
     808,   809,   810, -1079,   578,   813, -1079, -1079,   348,   686,
     744,   814, -1079, -1079, -1079, -1079, -1079, -1079,   364, -1079,
   -1079,   579,   585,   565, -1079, -1079, -1079,   746, -1079,   357,
   -1079,   819, -1079, -1079,   818,   820,   821,   822,   275, -1079,
     823,   824,   826,   827, -1079, -1079,   275, -1079,   830,   829,
     831,   832,   798, -1079,   244, -1079,   782,   -28, -1079, -1079,
   -1079, -1079, -1079,   603,   603,   603, -1079, -1079,    38, -1079,
     608,   750, -1079,   834, -1079, -1079,   608,   758, -1079, -1079,
   -1079,   837, -1079,   838, -1079,   599,   839, -1079, -1079, -1079,
     611,   842, -1079,    32,   841,   843,   845,   846, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079,   847,   848,   849,   850,
   -1079,    41, -1079, -1079,   -28, -1079,   -28,   844,   -42, -1079,
     624,   629,   652,   853, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079,   804,   359, -1079,   852,   654, -1079,   856,   857,   859,
   -1079,   860, -1079, -1079, -1079, -1079, -1079,   862,   863, -1079,
   -1079, -1079, -1079, -1079,   861,   659,   660,   664,   864, -1079,
     300, -1079,   867,   866,   868,   869,   870,   871,   872,   873,
     874,   156,   348, -1079,   390, -1079, -1079, -1079, -1079, -1079,
     877,   878,   750,   301, -1079, -1079,   654, -1079,    14, -1079,
     661,   665,   879, -1079,   676,   676, -1079,   880,   881,   418,
   -1079, -1079, -1079, -1079, -1079,   884, -1079, -1079,   876,   876,
     876,   876,   882,   883,   876,   886, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079,   687, -1079,   743, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
     885, -1079,    15,    16, -1079,   690,   692,   693,   795,     3,
   -1079,   888, -1079, -1079, -1079, -1079, -1079, -1079,   -28, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,   800,   694,
   -1079, -1079,    21,    22,    23,    24,    20,   892,   893, -1079,
     895, -1079, -1079, -1079, -1079,   248,   -28,   898,   902,   903,
     904, -1079, -1079,    20,   695, -1079,   696, -1079,   697, -1079,
     698, -1079,   398, -1079, -1079,   905,   908, -1079,   911,   912,
   -1079,   752, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079,   913, -1079,   700,   709,   749,   754, -1079,   840,
     756,   757, -1079, -1079, -1079,   914,   917,   918,   920,   858,
     922,   924,   424, -1079,   931,   762,   770, -1079, -1079, -1079,
     933, -1079, -1079, -1079
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079,    37, -1079, -1079,    11, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079,  -377, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079,  -547, -1078, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
     854, -1079, -1079,  -108,    66, -1079, -1079,  -326, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,  -122, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
     394, -1079, -1079, -1079, -1079, -1079,   158, -1079, -1079, -1079,
   -1079, -1079,   -80, -1079, -1079, -1079, -1079, -1079, -1079,   811,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
    -517, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079,   147, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079,   518,  -792, -1079, -1079,   -98, -1079,
   -1079,  -296, -1079, -1016, -1079, -1079,  -100, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
     144, -1079,   193,   400,    49, -1079, -1079,   -92, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079,   865, -1079, -1079,  -238, -1079,
   -1079,   507, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079,   129, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079,  -762, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079,  -157, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,   -56,
   -1079, -1079, -1079, -1079,   111, -1079,    18,  -909, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079,   492, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079,   212, -1079, -1079, -1079, -1079,
   -1079, -1079, -1079, -1079, -1079,   218
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -10
static const yytype_int16 yytable[] =
{
     181,   489,   490,   405,   691,   345,   893,   425,   881,   431,
     418,   179,   473,   477,   299,   301,   520,   909,   368,   366,
    1212,  1242,  1244,   450,  1272,   874,   455,  1264,  1266,  1268,
    1270,    11,   209,   214,  1252,   218,   465,    -9,   221,    -9,
      -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,
      -9,  1116,   227,    -9,    -9,   233,   205,   237,   242,    -9,
     248,    -9,   250,   255,   576,   577,   578,   579,   260,  1129,
     966,   263,    -9,    -9,  1094,  1095,   866,   512,   513,   653,
     515,   267,   420,  1146,    -9,   671,     9,    10,  1130,  1159,
    1096,   743,   272,   276,   281,   286,   867,  1097,    -9,  1068,
       1,  1231,  1232,  1233,    -9,   948,  1236,  1068,     5,    -9,
     618,   619,   620,   729,   565,   617,   621,   730,   654,   655,
     656,   657,    -9,   421,   519,    -9,    -9,   702,   882,   356,
     206,   744,   967,   357,   358,     6,    -9,   583,   584,    -9,
    1211,   585,   586,   949,   950,   951,   650,     8,   526,   527,
     519,   528,   529,    13,   877,   878,   703,   879,  1253,    14,
     580,   530,   531,   532,   533,    15,   534,   535,   340,   341,
     342,   343,   210,   215,   536,   219,    -9,  1003,   222,   403,
    1015,   404,  1117,  1118,  1131,  1132,   423,   113,   424,    -9,
      -9,    -9,   228,   724,   725,   228,    -9,   238,   243,   115,
     243,   367,   251,   256,    -9,   359,   519,   519,   261,   121,
     581,   264,   519,   672,   673,   674,   474,   478,   125,   126,
    1098,   268,   607,   123,  1133,  1134,  1135,  1136,  1137,  1138,
    1139,  1140,   273,   277,   282,   287,   537,   538,   539,    -9,
     124,    -9,    -9,   883,   180,   419,   470,   300,   302,   121,
     121,   704,   519,  1213,  1243,  1245,   121,  1158,   130,  1265,
    1267,  1269,  1271,   340,   341,   342,   343,   131,   779,  1141,
     638,   639,   640,   641,   132,   306,   886,   887,   344,   888,
     540,   519,   429,   307,   430,   133,   548,   549,   134,   528,
     529,   521,   894,   895,   523,   896,   541,   542,   135,   550,
     551,   552,   553,   308,   554,   535,   588,   589,   772,   555,
     556,   557,   136,   901,   902,   309,   903,   448,    16,   449,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,   732,   733,    29,    30,   543,   453,   544,   454,
      31,   137,    32,   310,   711,   712,   713,   714,   606,   311,
     463,   610,   464,    33,    34,   612,   831,   832,   833,   834,
    1197,  1004,  1005,  1006,  1007,    35,   139,  1048,  1049,   312,
    1050,   596,   597,   598,   599,   600,   601,   583,   584,    36,
    1010,  1011,  1012,  1013,   635,    37,  1186,  1187,  1188,  1189,
      38,   138,   645,  1192,  1193,   563,  1194,   564,   313,   648,
     649,   140,   573,    39,   574,   141,    40,    41,   480,   481,
     482,   483,   484,   485,   486,   487,   142,    42,   558,   519,
      43,  1224,  1225,  1292,  1226,   687,   519,  1317,  1318,   706,
    1319,   707,   143,   559,   508,   509,   510,   511,   512,   513,
     514,   515,   516,   517,   912,   913,   914,   314,   708,   144,
     707,   315,   316,   770,   869,   771,   870,    44,   480,   481,
     482,   483,   484,   485,   486,   487,   945,   146,   946,   814,
      45,    46,    47,   998,   560,   999,   561,    48,  1080,  1030,
    1081,  1031,  1280,   147,  1281,    49,   841,   842,   746,   148,
     748,   661,   662,   663,   664,   665,   666,   667,   668,   969,
    1198,  1199,  1200,  1201,  1202,  1203,  1204,  1205,  1206,  1207,
    1058,  1059,   149,   774,   775,  1154,  1155,  1091,  1092,   152,
      50,   170,    51,    52,   171,   172,   173,   784,   174,   175,
    1208,   623,   624,   625,   626,   627,   628,   629,   630,   176,
     177,   178,   182,   184,   183,   304,   185,   186,   187,   188,
     189,   190,   191,   192,   193,   194,   195,   196,   197,   198,
     200,   202,   203,   204,   291,   800,   292,   518,   519,   293,
     294,   295,   296,   297,   298,   303,   305,   318,   319,   320,
     322,   327,   325,  1176,   323,   331,   334,   336,   339,   346,
     347,   350,   351,   353,   355,   360,   363,   365,   370,   368,
     376,  1042,   377,   371,   373,   374,   378,   379,   380,   381,
     382,   384,   383,   385,   386,   392,   397,   398,   399,   400,
     401,   402,   437,   438,   447,   457,   443,   461,   444,   462,
     467,   491,   493,   492,   494,   495,   496,   497,   498,   499,
     500,   501,   503,   507,   502,   525,   567,   568,   572,   504,
     582,   546,   591,   868,   592,   593,   594,   602,   875,   603,
     609,   604,   611,   616,   631,   613,   614,   632,   633,   643,
     615,   652,   659,   709,   676,   680,   684,   689,   694,   696,
     700,   705,   721,   752,   727,   722,   728,   749,   910,   750,
     751,   754,   758,   757,   759,   777,   760,   761,  1083,   762,
     753,   763,   764,   765,   766,   778,   780,   781,   785,   788,
     789,   790,   791,   768,   792,   793,   796,   799,   801,   802,
     803,   804,   805,   808,   813,   827,   824,   816,   817,   818,
     819,   820,   823,   826,   828,   837,   840,   829,   815,   830,
     844,   845,   846,   847,   848,   849,   851,   860,   859,   863,
     865,   872,   884,   836,   890,   473,   477,   899,   856,   900,
     857,   858,   906,   917,   973,  1191,   918,   919,   921,   923,
     926,  1017,   927,   930,   931,   932,   978,   933,   934,   937,
     938,   939,   940,   942,   943,   952,   962,   956,   984,   953,
     954,   955,   957,   958,   968,   985,   976,   982,   986,   987,
     988,   993,   995,  1055,   990,   994,  1001,  1022,  1002,  1023,
    1027,  1028,  1033,  1034,  1035,  1036,  1051,  1019,  1020,  1039,
    1045,  1021,  1053,  1060,  1064,  1079,  1065,  1066,  1067,  1069,
    1070,  1043,  1071,  1072,  1075,  1076,  1109,  1077,  1078,  1088,
    1104,  1107,  1108,  1110,  1101,  1114,  1115,  1120,  1144,  1121,
    1057,  1122,  1123,  1125,  1126,  1127,  1128,  1150,  1157,  1153,
    1160,  1161,  1147,  1162,  1163,  1169,  1305,  1148,  1167,  1168,
    1174,  1177,  1178,   960,  1179,  1180,  1181,  1182,  1183,  1184,
    1185,  1195,  1196,  1217,   128,  1314,  1222,  1223,  1228,  1240,
    1149,  1158,  1254,  1171,  1172,  1293,  1275,  1276,  1173,  1215,
    1250,  1277,  1284,  1216,  1229,  1086,  1285,  1286,  1287,  1294,
    1234,  1235,  1295,  1219,  1237,  1296,  1297,  1298,  1085,  1299,
    1309,  1310,  1311,  1238,  1312,  1261,  1315,  1247,  1316,  1248,
     963,  1249,  1262,  1288,  1289,  1290,  1291,  1320,  1300,  1323,
     880,   889,   688,  1103,  1084,  1263,  1105,  1301,   562,   862,
     904,   571,   925,   693,   980,  1106,  1165,   854,   160,  1073,
     944,  1016,  1142,   608,  1143,   855,     0,     0,   234,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,  1302,     0,     0,
       0,     0,  1303,  1306,  1307,     0,     0,     0,     0,     0,
    1321,     0,     0,     0,     0,     0,     0,     0,  1322,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,  1256,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,  1283
};

static const yytype_int16 yycheck[] =
{
     122,   378,   379,   329,   551,   243,   798,   333,     4,   335,
       4,     6,    30,    30,     6,     6,     4,     4,    30,    28,
       6,     6,     6,   349,     4,     6,   352,     6,     6,     6,
       6,     1,    95,    95,    31,    95,   362,     7,    95,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    19,    95,    23,    24,    95,    21,    95,    95,    29,
      95,    31,    95,    95,    80,    81,    82,    83,    95,    28,
     165,    95,    42,    43,    36,    37,    30,    36,    37,   175,
      39,    95,     4,   125,    54,    90,   169,   170,    47,  1105,
      52,   173,    95,    95,    95,    95,    50,    59,    68,  1008,
     158,  1179,  1180,  1181,    74,   173,  1184,  1016,     0,    79,
     162,   163,   164,   103,   440,   492,   168,   107,   214,   215,
     216,   217,    92,    45,   166,    95,    96,    41,   124,   103,
      95,   213,   227,   107,   108,     8,   106,    97,    98,   109,
    1156,   101,   102,   211,   212,   213,   523,     4,    33,    34,
     166,    36,    37,   234,     3,     4,    70,     6,   155,   234,
     176,    46,    47,    48,    49,   234,    51,    52,    75,    76,
      77,    78,   235,   235,    59,   235,   146,   939,   235,   234,
     942,   236,   150,   151,   143,   144,   234,     6,   236,   159,
     160,   161,   235,    99,   100,   235,   166,   235,   235,    93,
     235,   210,   235,   235,   174,   179,   166,   166,   235,   237,
     226,   235,   166,   218,   219,   220,   234,   234,   171,   172,
     182,   235,   234,     6,   183,   184,   185,   186,   187,   188,
     189,   190,   235,   235,   235,   235,   121,   122,   123,   209,
       6,   211,   212,   239,   239,   239,   368,   239,   239,   237,
     237,   165,   166,   239,   239,   239,   237,   237,     6,   238,
     238,   238,   238,    75,    76,    77,    78,     6,   645,   228,
     214,   215,   216,   217,     6,     1,     3,     4,    90,     6,
     165,   166,   234,     9,   236,     6,    33,    34,     6,    36,
      37,   413,    36,    37,   416,    39,   181,   182,     6,    46,
      47,    48,    49,    29,    51,    52,   147,   148,   634,    56,
      57,    58,     6,     3,     4,    41,     6,   234,     7,   236,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,   117,   118,    23,    24,   221,   234,   223,   236,
      29,     6,    31,    69,    86,    87,    88,    89,   470,    75,
     234,   473,   236,    42,    43,   477,   149,   150,   151,   152,
    1152,    86,    87,    88,    89,    54,     6,     3,     4,    95,
       6,   111,   112,   113,   114,   115,   116,    97,    98,    68,
      86,    87,    88,    89,   506,    74,   230,   231,   232,   233,
      79,     9,   514,     3,     4,   234,     6,   236,   124,   521,
     522,     6,   234,    92,   236,     6,    95,    96,    60,    61,
      62,    63,    64,    65,    66,    67,     6,   106,   165,   166,
     109,     3,     4,    25,     6,   547,   166,     3,     4,   238,
       6,   240,     3,   180,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    71,    72,    73,   173,   238,     3,
     240,   177,   178,   234,   234,   236,   236,   146,    60,    61,
      62,    63,    64,    65,    66,    67,   234,     6,   236,   707,
     159,   160,   161,   234,   221,   236,   223,   166,   234,   222,
     236,   224,   234,     6,   236,   174,   119,   120,   610,     6,
     612,   191,   192,   193,   194,   195,   196,   197,   198,   876,
     199,   200,   201,   202,   203,   204,   205,   206,   207,   208,
     153,   154,     6,   635,   636,   156,   157,  1034,  1035,     6,
     209,     4,   211,   212,   234,     4,    94,   649,     4,     4,
     229,   494,   495,   496,   497,   498,   499,   500,   501,     4,
       4,     4,   234,     4,     6,    25,   234,   234,   234,   234,
     234,   234,   234,   234,   234,   234,   234,   234,   234,   234,
     234,   234,   234,   234,   234,   687,   234,   165,   166,     6,
     234,   234,     6,     6,     6,   234,     4,     6,    20,    19,
      25,    31,    42,  1130,    10,    43,    54,    68,    92,    74,
      79,    96,   237,   146,   106,   109,   174,   209,   211,    30,
     234,   978,     6,    28,   212,    28,     6,     6,   238,   238,
     238,     6,   238,     6,   234,   161,    22,     4,     6,     4,
       4,     4,     4,     4,     4,     4,   237,   234,   237,     4,
       4,    27,     4,     6,     4,     4,     4,     4,     4,     4,
       4,     4,   236,     4,     6,   237,     4,     4,     4,    27,
       4,    28,     4,   775,     4,     4,   110,     4,   780,     4,
       4,   236,     4,     6,   234,    25,    25,    75,     6,     6,
      25,     4,     6,   236,     6,     6,     6,     6,     6,     6,
       6,     4,     4,   234,     4,     6,     4,     6,   810,     6,
       6,     3,   234,     4,   234,     4,   234,   234,  1024,   234,
      25,   234,   234,   234,   234,     4,     4,     4,   237,     4,
       4,     4,     4,    28,     4,     4,     4,     4,     4,     4,
       4,     4,     4,     4,     4,   238,   237,     6,     6,     6,
       6,     4,     4,     4,     4,   234,   234,     6,    91,     6,
       4,     4,     4,     4,     4,     4,   236,   145,     6,     4,
     234,     4,     4,   104,     4,    30,    30,     6,    26,     6,
      26,    26,     4,     4,   238,  1142,     6,     4,     4,     4,
       4,   165,   225,     6,     6,     6,   898,     6,     6,     4,
       4,     4,     4,     4,     4,     4,     4,    26,   910,     6,
       6,     6,     6,     6,     6,     6,    53,    53,     6,     6,
       4,     4,   103,   238,   234,     6,     4,     6,   104,     6,
       4,     4,     4,     4,     4,   237,   237,    27,    27,     6,
       6,    27,   237,     4,     6,    27,     6,     6,     6,     6,
       6,   145,     6,     6,     4,     6,   237,     6,     6,   236,
       6,     4,     4,     4,   236,   234,     4,     6,     4,     6,
     104,     6,     6,     6,     6,     6,     6,     4,     6,    55,
       4,     4,   238,     4,     4,     4,    26,   238,     6,     6,
       6,     4,     6,   862,     6,     6,     6,     6,     6,     6,
       6,     4,     4,     4,    30,    27,     6,     6,     4,     4,
     238,   237,     4,   234,   234,  1272,     4,     4,   234,   238,
     105,     6,     4,   238,    28,  1027,     4,     4,     4,     4,
      28,    28,     4,   237,    28,     4,     4,   165,  1026,     6,
       6,     4,     4,   236,     4,   125,     4,   237,     4,   237,
     864,   238,   238,   238,   238,   238,   238,     6,   238,     6,
     782,   794,   548,  1041,  1024,  1241,  1046,   238,   430,   756,
     806,   444,   823,   553,   905,  1047,  1113,   745,    93,  1015,
     849,   943,  1084,   471,  1086,   747,    -1,    -1,   157,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   238,    -1,    -1,
      -1,    -1,   238,   237,   237,    -1,    -1,    -1,    -1,    -1,
     238,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   238,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,  1228,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,  1256
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   158,   242,   243,   244,     0,     8,   245,     4,   169,
     170,     1,   246,   234,   234,   234,     7,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    23,
      24,    29,    31,    42,    43,    54,    68,    74,    79,    92,
      95,    96,   106,   109,   146,   159,   160,   161,   166,   174,
     209,   211,   212,   247,   248,   249,   251,   252,   254,   256,
     258,   259,   276,   277,   278,   279,   281,   283,   286,   287,
     288,   292,   293,   310,   318,   320,   326,   327,   329,   330,
     341,   342,   354,   355,   386,   387,   441,   472,   474,   475,
     492,   493,   494,   495,   510,   511,   536,   537,   554,   555,
     570,   571,   583,   584,   619,   620,   632,   633,   647,   648,
     657,   658,   250,     6,   253,    93,   255,   257,   280,   282,
     284,   237,   339,     6,     6,   171,   172,   321,   321,   311,
       6,     6,     6,     6,     6,     6,     6,     6,     9,     6,
       6,     6,     6,     3,     3,   260,     6,     6,     6,     6,
     289,   294,     6,   331,   343,   356,   388,   442,   476,   496,
     496,   512,   538,   556,   572,   585,   623,   635,   650,   660,
       4,   234,     4,    94,     4,     4,     4,     4,     4,     6,
     239,   339,   234,     6,     4,   234,   234,   234,   234,   234,
     234,   234,   234,   234,   234,   234,   234,   234,   234,   261,
     234,   621,   234,   234,   234,    21,    95,   290,   291,    95,
     235,   295,   309,   319,    95,   235,   332,   340,    95,   235,
     344,    95,   235,   357,   358,   359,   385,    95,   235,   389,
     390,   391,   439,    95,   390,   443,   473,    95,   235,   477,
     478,   491,    95,   235,   497,   498,   503,   509,    95,   508,
      95,   235,   513,   514,   535,    95,   235,   539,   540,   553,
      95,   235,   557,    95,   235,   573,   582,    95,   235,   586,
     587,   609,    95,   235,   622,   624,    95,   235,   634,   636,
     637,    95,   235,   649,   651,   652,    95,   235,   659,   661,
     662,   234,   234,     6,   234,   234,     6,     6,     6,     6,
     239,     6,   239,   234,    25,     4,     1,     9,    29,    41,
      69,    75,    95,   124,   173,   177,   178,   262,     6,    20,
      19,   296,    25,    10,   333,    42,   345,    31,   360,   362,
     361,    43,   392,   402,    54,   444,    68,   479,   480,    92,
      75,    76,    77,    78,    90,   499,    74,    79,   515,   516,
      96,   237,   542,   146,   558,   106,   103,   107,   108,   179,
     109,   588,   592,   174,   625,   209,    28,   210,    30,   646,
     211,    28,   655,   212,    28,   665,   234,     6,     6,     6,
     238,   238,   238,   238,     6,     6,   234,   263,   269,   266,
     267,   264,   161,   270,   268,   265,   271,    22,     4,     6,
       4,     4,     4,   234,   236,   328,   363,   364,   365,   367,
     370,   372,   374,   378,   379,   382,   383,   384,     4,   239,
       4,    45,   393,   234,   236,   328,   403,   418,   419,   234,
     236,   328,   445,   446,   454,   456,   457,     4,     4,   481,
     482,   500,   501,   237,   237,   504,   506,     4,   234,   236,
     328,   518,   541,   234,   236,   328,   543,     4,   574,   576,
     577,   234,     4,   234,   236,   328,   593,     4,   638,   640,
     339,   645,   653,    30,   234,   656,   663,    30,   234,   666,
      60,    61,    62,    63,    64,    65,    66,    67,   285,   285,
     285,    27,     6,     4,     4,     4,     4,     4,     4,     4,
       4,     4,     6,   236,    27,   334,   346,     4,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,   165,   166,
       4,   339,   373,   339,   394,   237,    33,    34,    36,    37,
      46,    47,    48,    49,    51,    52,    59,   121,   122,   123,
     165,   181,   182,   221,   223,   425,    28,   421,    33,    34,
      46,    47,    48,    49,    51,    56,    57,    58,   165,   180,
     221,   223,   425,   234,   236,   328,   483,     4,     4,   499,
     502,   502,     4,   234,   236,   507,    80,    81,    82,    83,
     176,   226,     4,    97,    98,   101,   102,   552,   147,   148,
     560,     4,     4,     4,   110,   589,   111,   112,   113,   114,
     115,   116,     4,     4,   236,   643,   339,   234,   646,     4,
     339,     4,   339,    25,    25,    25,     6,   285,   162,   163,
     164,   168,   272,   272,   272,   272,   272,   272,   272,   272,
     272,   234,    75,     6,   335,   339,   347,   368,   214,   215,
     216,   217,   371,     6,   366,   339,   380,   375,   339,   339,
     285,   396,     4,   175,   214,   215,   216,   217,   417,     6,
     406,   191,   192,   193,   194,   195,   196,   197,   198,   306,
     407,    90,   218,   219,   220,   464,     6,   408,   420,   412,
       6,   414,   409,   410,     6,   404,   422,   339,   371,     6,
     451,   306,   452,   464,     6,   455,     6,   458,   453,   449,
       6,   447,    41,    70,   165,     4,   238,   240,   238,   236,
     505,    86,    87,    88,    89,   519,   520,   521,   522,   523,
     524,     4,     6,   544,    99,   100,   559,     4,     4,   103,
     107,   578,   117,   118,   590,   594,   595,   596,   597,   598,
     599,   626,   639,   173,   213,   654,   339,   664,   339,     6,
       6,     6,   234,    25,     3,   274,   273,     4,   234,   234,
     234,   234,   234,   234,   234,   234,   234,   297,    28,   322,
     234,   236,   328,   336,   339,   339,   348,     4,     4,   285,
       4,     4,   376,   377,   339,   237,   397,   395,     4,     4,
       4,     4,     4,     4,   415,   416,     4,   411,   405,     4,
     339,     4,     4,     4,     4,     4,   460,   461,     4,   448,
     485,   489,   484,     4,   499,    91,     6,     6,     6,     6,
       4,   526,   530,     4,   237,   525,     4,   238,     4,     6,
       6,   149,   150,   151,   152,   561,   104,   234,   579,   581,
     234,   119,   120,   591,     4,     4,     4,     4,     4,     4,
     627,   236,   641,   644,   656,   666,    26,    26,    26,     6,
     145,   463,   463,     4,   323,   234,    30,    50,   339,   234,
     236,   349,     4,   369,     6,   339,   381,     3,     4,     6,
     377,     4,   124,   239,     4,   413,     3,     4,     6,   416,
       4,   426,   429,   426,    36,    37,    39,   423,   424,     6,
       6,     3,     4,     6,   461,   450,     4,   465,   468,     4,
     339,   486,    71,    72,    73,   490,   487,     4,     6,     4,
     517,     4,   527,     4,   531,   517,     4,   225,   534,   545,
       6,     6,     6,     6,     6,   562,   575,     4,     4,     4,
       4,   605,     4,     4,   605,   234,   236,   628,   173,   211,
     212,   213,     4,     6,     6,     6,    26,     6,     6,   275,
     275,   298,     4,   325,   337,   338,   165,   227,     6,   285,
     398,   400,   399,   238,   610,   430,    53,   427,   339,   459,
     465,   469,    53,   466,   339,     6,     6,     6,     4,   488,
     234,   528,   532,     4,     6,   103,   546,   549,   234,   236,
     563,     4,   104,   552,    86,    87,    88,    89,   607,   608,
      86,    87,    88,    89,   606,   552,   607,   165,   642,    27,
      27,    27,     6,     6,   299,   324,   325,     4,     4,   350,
     222,   224,   353,     4,     4,     4,   237,   611,   614,     6,
     438,   428,   285,   145,   462,     6,   471,   467,     3,     4,
       6,   237,   529,   237,   533,   238,   550,   104,   153,   154,
       4,   580,   600,   602,     6,     6,     6,     6,   608,     6,
       6,     6,     6,   600,   629,     4,     6,     6,     6,    27,
     234,   236,   300,   328,   383,   324,   339,   351,   236,   328,
     401,   401,   401,   612,    36,    37,    52,    59,   182,   615,
     618,   236,   437,   429,     6,   437,   468,     4,     4,   237,
       4,   547,   564,   565,   234,     4,    19,   150,   151,   603,
       6,     6,     6,     6,   630,     6,     6,     6,     6,    28,
      47,   143,   144,   183,   184,   185,   186,   187,   188,   189,
     190,   228,   339,   339,     4,   352,   125,   238,   238,   238,
       4,   617,   616,    55,   156,   157,   435,     6,   237,   434,
       4,     4,     4,     4,   566,   566,   601,     6,     6,     4,
     631,   234,   234,   234,     6,   301,   306,     4,     6,     6,
       6,     6,     6,     6,     6,     6,   230,   231,   232,   233,
     305,   285,     3,     4,     6,     4,     4,   426,   199,   200,
     201,   202,   203,   204,   205,   206,   207,   208,   229,   440,
     436,   434,     6,   239,   470,   238,   238,     4,   548,   237,
     567,   604,     6,     6,     3,     4,     6,   312,     4,    28,
     307,   307,   307,   307,    28,    28,   307,    28,   236,   613,
       4,   431,     6,   239,     6,   239,   432,   237,   237,   238,
     105,   551,    31,   155,     4,   313,   339,   308,   302,   303,
     304,   125,   238,   432,     6,   238,     6,   238,     6,   238,
       6,   238,     4,   433,   434,     4,     4,     6,   569,   568,
     234,   236,   314,   339,     4,     4,     4,     4,   238,   238,
     238,   238,    25,   285,     4,     4,     4,     4,   165,     6,
     238,   238,   238,   238,   315,    26,   237,   237,   316,     6,
       4,     4,     4,   317,    27,     4,     4,     3,     4,     6,
       6,   238,   238,     6
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
        case 4:

/* Line 1455 of yacc.c  */
#line 219 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 220 "def.y"
    {
         defVersionNum = convert_defname2num((yyvsp[(3) - (4)].string));
         if (defrVersionStrCbk) {
            CALLBACK(defrVersionStrCbk, defrVersionStrCbkType, (yyvsp[(3) - (4)].string));
         } else {
            CALLBACK(defrVersionCbk, defrVersionCbkType, defVersionNum);
         }
         if (defVersionNum > 5.3 && defVersionNum < 5.4) {
            defIgnoreVersion = 1;
         }
         hasVer = 1;
      ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 235 "def.y"
    {
        names_case_sensitive = 1;
        CALLBACK(defrCaseSensitiveCbk, defrCaseSensitiveCbkType,
                 names_case_sensitive); 
        hasNameCase = 1;
      ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 242 "def.y"
    {
        names_case_sensitive = 0;
        CALLBACK(defrCaseSensitiveCbk, defrCaseSensitiveCbkType,
                 names_case_sensitive); 
        hasNameCase = 1;
      ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 269 "def.y"
    {dumb_mode = 1; no_num = 1; ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 270 "def.y"
    {
            CALLBACK(defrDesignCbk, defrDesignStartCbkType, (yyvsp[(3) - (4)].string));
            hasDes = 1;
          ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 276 "def.y"
    {
            CALLBACK(defrDesignEndCbk, defrDesignEndCbkType, 0);
            // 11/16/2001 - Wanda da Rosa - pcr 408334
            // Return 1 if there is any errors during parsing
            if (errors)
                return 1;

            if (!hasVer)
              yydefwarning("VERSION is a required statement.");
            if (!hasNameCase)
              yydefwarning("NAMESCASESENSITIVE is a required statement.");
            if (!hasBusBit)
              yydefwarning("BUSBITCHARS is a required statement.");
            if (!hasDivChar)
              yydefwarning("DIVIDERCHAR is a required statement.");
            if (!hasDes)
              yydefwarning("DESIGN is a required statement.");
            hasVer = 0;
            hasNameCase = 0;
            hasBusBit = 0;
            hasDivChar = 0;
            hasDes = 0;
          ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 300 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 301 "def.y"
    { CALLBACK(defrTechnologyCbk, defrTechNameCbkType, (yyvsp[(3) - (4)].string)); ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 303 "def.y"
    {dumb_mode = 1; no_num = 1;;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 304 "def.y"
    { CALLBACK(defrArrayNameCbk, defrArrayNameCbkType, (yyvsp[(3) - (4)].string)); ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 306 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 307 "def.y"
    { CALLBACK(defrFloorPlanNameCbk, defrFloorPlanNameCbkType, (yyvsp[(3) - (4)].string)); ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 310 "def.y"
    { CALLBACK(defrHistoryCbk, defrHistoryCbkType, History_text); ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 313 "def.y"
    { CALLBACK(defrPropDefStartCbk, defrPropDefStartCbkType, 0); ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 315 "def.y"
    { 
            CALLBACK(defrPropDefEndCbk, defrPropDefEndCbkType, 0);
            real_num = 0;     /* just want to make sure it is reset */
          ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 322 "def.y"
    { ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 324 "def.y"
    {dumb_mode = 1; no_num = 1; defrProp.defiProp::clear(); ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 326 "def.y"
    {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("design", (yyvsp[(3) - (5)].string));
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrDesignProp.defiPropType::setPropType((yyvsp[(3) - (5)].string), defPropDefType);
              }
            ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 333 "def.y"
    { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 335 "def.y"
    {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("net", (yyvsp[(3) - (5)].string));
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrNetProp.defiPropType::setPropType((yyvsp[(3) - (5)].string), defPropDefType);
              }
            ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 342 "def.y"
    { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 344 "def.y"
    {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("specialnet", (yyvsp[(3) - (5)].string));
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrSNetProp.defiPropType::setPropType((yyvsp[(3) - (5)].string), defPropDefType);
              }
            ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 351 "def.y"
    { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 353 "def.y"
    {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("region", (yyvsp[(3) - (5)].string));
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrRegionProp.defiPropType::setPropType((yyvsp[(3) - (5)].string), defPropDefType);
              }
            ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 360 "def.y"
    { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 362 "def.y"
    {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("group", (yyvsp[(3) - (5)].string));
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrGroupProp.defiPropType::setPropType((yyvsp[(3) - (5)].string), defPropDefType);
              }
            ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 369 "def.y"
    { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 371 "def.y"
    {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("component", (yyvsp[(3) - (5)].string));
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrCompProp.defiPropType::setPropType((yyvsp[(3) - (5)].string), defPropDefType);
              }
            ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 378 "def.y"
    { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 380 "def.y"
    {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("row", (yyvsp[(3) - (5)].string));
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrRowProp.defiPropType::setPropType((yyvsp[(3) - (5)].string), defPropDefType);
              }
            ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 387 "def.y"
    { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 389 "def.y"
    {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("pin", (yyvsp[(3) - (5)].string));
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrPinDefProp.defiPropType::setPropType((yyvsp[(3) - (5)].string), defPropDefType);
              }
            ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 397 "def.y"
    { dumb_mode = 1 ; no_num = 1; defrProp.defiProp::clear(); ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 399 "def.y"
    {
              if (defrPropCbk) {
                defrProp.defiProp::setPropType("componentpin", (yyvsp[(3) - (5)].string));
		CALLBACK(defrPropCbk, defrPropCbkType, &defrProp);
                defrCompPinProp.defiPropType::setPropType((yyvsp[(3) - (5)].string), defPropDefType);
              }
            ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 406 "def.y"
    { yyerrok; yyclearin;;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 408 "def.y"
    { real_num = 0 ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 409 "def.y"
    {
              if (defrPropCbk) defrProp.defiProp::setPropInteger();
              defPropDefType = 'I';
            ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 413 "def.y"
    { real_num = 1 ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 414 "def.y"
    {
              if (defrPropCbk) defrProp.defiProp::setPropReal();
              defPropDefType = 'R';
              real_num = 0;
            ;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 420 "def.y"
    {
              if (defrPropCbk) defrProp.defiProp::setPropString();
              defPropDefType = 'S';
            ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 425 "def.y"
    {
              if (defrPropCbk) defrProp.defiProp::setPropQString((yyvsp[(2) - (2)].string));
              defPropDefType = 'Q';
            ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 430 "def.y"
    {
              if (defrPropCbk) defrProp.defiProp::setPropNameMapString((yyvsp[(2) - (2)].string));
              defPropDefType = 'S';
            ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 437 "def.y"
    { if (defrPropCbk) defrProp.defiProp::setNumber((yyvsp[(1) - (1)].dval)); ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 440 "def.y"
    { CALLBACK(defrUnitsCbk,  defrUnitsCbkType, (yyvsp[(4) - (5)].dval)); ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 443 "def.y"
    {
            CALLBACK(defrDividerCbk, defrDividerCbkType, (yyvsp[(2) - (3)].string));
            hasDivChar = 1;
          ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 449 "def.y"
    { 
            CALLBACK(defrBusBitCbk, defrBusBitCbkType, (yyvsp[(2) - (3)].string));
            hasBusBit = 1;
          ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 454 "def.y"
    { dumb_mode = 1; no_num = 1; defrSite.defiSite::clear(); ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 457 "def.y"
    {
            if (defrSiteCbk) {
              defrSite.defiSite::setName((yyvsp[(3) - (14)].string));
              defrSite.defiSite::setLocation((yyvsp[(4) - (14)].dval),(yyvsp[(5) - (14)].dval));
              defrSite.defiSite::setOrient((yyvsp[(6) - (14)].integer));
              defrSite.defiSite::setDo((yyvsp[(8) - (14)].dval),(yyvsp[(10) - (14)].dval),(yyvsp[(12) - (14)].dval),(yyvsp[(13) - (14)].dval));
	      CALLBACK(defrSiteCbk, defrSiteCbkType, &(defrSite));
            }
	  ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 467 "def.y"
    {dumb_mode = 1;no_num = 1; ;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 469 "def.y"
    {
              if (defrCanplaceCbk) {
                defrCanplace.defiSite::setName((yyvsp[(3) - (14)].string));
                defrCanplace.defiSite::setLocation((yyvsp[(4) - (14)].dval),(yyvsp[(5) - (14)].dval));
                defrCanplace.defiSite::setOrient((yyvsp[(6) - (14)].integer));
                defrCanplace.defiSite::setDo((yyvsp[(8) - (14)].dval),(yyvsp[(10) - (14)].dval),(yyvsp[(12) - (14)].dval),(yyvsp[(13) - (14)].dval));
		CALLBACK(defrCanplaceCbk, defrCanplaceCbkType,
		&(defrCanplace));
              }
            ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 479 "def.y"
    {dumb_mode = 1;no_num = 1; ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 481 "def.y"
    {
              if (defrCannotOccupyCbk) {
                defrCannotOccupy.defiSite::setName((yyvsp[(3) - (14)].string));
                defrCannotOccupy.defiSite::setLocation((yyvsp[(4) - (14)].dval),(yyvsp[(5) - (14)].dval));
                defrCannotOccupy.defiSite::setOrient((yyvsp[(6) - (14)].integer));
                defrCannotOccupy.defiSite::setDo((yyvsp[(8) - (14)].dval),(yyvsp[(10) - (14)].dval),(yyvsp[(12) - (14)].dval),(yyvsp[(13) - (14)].dval));
		CALLBACK(defrCannotOccupyCbk, defrCannotOccupyCbkType,
                        &(defrCannotOccupy));
              }
            ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 492 "def.y"
    {(yyval.integer) = 0;;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 493 "def.y"
    {(yyval.integer) = 1;;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 494 "def.y"
    {(yyval.integer) = 2;;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 495 "def.y"
    {(yyval.integer) = 3;;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 496 "def.y"
    {(yyval.integer) = 4;;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 497 "def.y"
    {(yyval.integer) = 5;;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 498 "def.y"
    {(yyval.integer) = 6;;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 499 "def.y"
    {(yyval.integer) = 7;;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 502 "def.y"
    {
	    if (defrDieAreaCbk) {
	      defrDieArea.defiBox::setLowerLeft((yyvsp[(2) - (4)].pt).x,(yyvsp[(2) - (4)].pt).y);
	      defrDieArea.defiBox::setUpperRight((yyvsp[(3) - (4)].pt).x,(yyvsp[(3) - (4)].pt).y);
	      CALLBACK(defrDieAreaCbk, defrDieAreaCbkType, &(defrDieArea));
	    }
	  ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 512 "def.y"
    { ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 515 "def.y"
    {
          if (defVersionNum < 5.4) {
             CALLBACK(defrDefaultCapCbk, defrDefaultCapCbkType, ROUND((yyvsp[(2) - (2)].dval)));
          } else {
             yydefwarning("DEFAULTCAP is obsolete in 5.4. It will be ignored.");
          }
        ;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 528 "def.y"
    {
            if (defVersionNum < 5.4) {
	      if (defrPinCapCbk) {
	        defrPinCap.defiPinCap::setPin(ROUND((yyvsp[(2) - (5)].dval)));
	        defrPinCap.defiPinCap::setCap((yyvsp[(4) - (5)].dval));
	        CALLBACK(defrPinCapCbk, defrPinCapCbkType, &(defrPinCap));
	      }
            }
	  ;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 539 "def.y"
    { ;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 542 "def.y"
    { ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 545 "def.y"
    { CALLBACK(defrStartPinsCbk, defrStartPinsCbkType, ROUND((yyvsp[(2) - (3)].dval))); ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 551 "def.y"
    {dumb_mode = 1;no_num = 1; ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 552 "def.y"
    {dumb_mode = 1; no_num = 1; ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 553 "def.y"
    {
            if (defrPinCbk || defrPinExtCbk) {
              defrPin.defiPin::Setup((yyvsp[(3) - (7)].string), (yyvsp[(7) - (7)].string));
            }
          ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 559 "def.y"
    { CALLBACK(defrPinCbk, defrPinCbkType, &defrPin); ;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 565 "def.y"
    {
            if (defrPinCbk)
              defrPin.defiPin::setSpecial();
          ;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 571 "def.y"
    { CALLBACK(defrPinExtCbk, defrPinExtCbkType, History_text); ;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 574 "def.y"
    {
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::setDirection((yyvsp[(3) - (3)].string));
            ;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 580 "def.y"
    {
              if (defrPinCbk || defrPinExtCbk) defrPin.defiPin::setUse((yyvsp[(3) - (3)].string));
            ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 584 "def.y"
    { dumb_mode = 1; ;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 585 "def.y"
    {
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::setLayer((yyvsp[(4) - (6)].string),(yyvsp[(5) - (6)].pt).x,(yyvsp[(5) - (6)].pt).y,(yyvsp[(6) - (6)].pt).x,(yyvsp[(6) - (6)].pt).y);
            ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 591 "def.y"
    {
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::setPlacement((yyvsp[(1) - (3)].integer), (yyvsp[(2) - (3)].pt).x, (yyvsp[(2) - (3)].pt).y, (yyvsp[(3) - (3)].integer));
            ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 598 "def.y"
    {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINPARTIALMETALAREA is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinPartialMetalArea((int)(yyvsp[(3) - (4)].dval), (yyvsp[(4) - (4)].string)); 
            ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 607 "def.y"
    {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINPARTIALMETALSIDEAREA is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinPartialMetalSideArea((int)(yyvsp[(3) - (4)].dval), (yyvsp[(4) - (4)].string)); 
            ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 616 "def.y"
    {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINGATEAREA is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinGateArea((int)(yyvsp[(3) - (4)].dval), (yyvsp[(4) - (4)].string)); 
            ;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 625 "def.y"
    {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINDIFFAREA is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinDiffArea((int)(yyvsp[(3) - (4)].dval), (yyvsp[(4) - (4)].string)); 
            ;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 633 "def.y"
    {dumb_mode=1;;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 634 "def.y"
    {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINMAXAREACAR is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinMaxAreaCar((int)(yyvsp[(3) - (6)].dval), (yyvsp[(6) - (6)].string)); 
            ;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 642 "def.y"
    {dumb_mode=1;;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 644 "def.y"
    {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINMAXSIDEAREACAR is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinMaxSideAreaCar((int)(yyvsp[(3) - (6)].dval), (yyvsp[(6) - (6)].string)); 
            ;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 653 "def.y"
    {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINPARTIALCUTAREA is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinPartialCutArea((int)(yyvsp[(3) - (4)].dval), (yyvsp[(4) - (4)].string)); 
            ;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 661 "def.y"
    {dumb_mode=1;;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 662 "def.y"
    {
              if (defVersionNum <= 5.3) {
                yyerror("ANTENNAPINMAXCUTCAR is a 5.4 syntax. Your def file is not defined as 5.4");
                CHKERR();
              }
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAPinMaxCutCar((int)(yyvsp[(3) - (6)].dval), (yyvsp[(6) - (6)].string)); 
            ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 671 "def.y"
    {  /* 5.5 syntax */
              if (defVersionNum < 5.5) {
                yyerror("ANTENNAMODEL is a 5.5 syntax. Your def file is not defined as 5.5");
                CHKERR();
              }
            ;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 679 "def.y"
    { aOxide = 1;
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAntennaModel(aOxide);
            ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 684 "def.y"
    { aOxide = 2;
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAntennaModel(aOxide);
            ;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 689 "def.y"
    { aOxide = 3;
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAntennaModel(aOxide);
            ;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 694 "def.y"
    { aOxide = 4;
              if (defrPinCbk || defrPinExtCbk)
                defrPin.defiPin::addAntennaModel(aOxide);
            ;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 700 "def.y"
    { (yyval.string) = (char*)"SIGNAL"; ;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 702 "def.y"
    { (yyval.string) = (char*)"POWER"; ;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 704 "def.y"
    { (yyval.string) = (char*)"GROUND"; ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 706 "def.y"
    { (yyval.string) = (char*)"CLOCK"; ;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 708 "def.y"
    { (yyval.string) = (char*)"TIEOFF"; ;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 710 "def.y"
    { (yyval.string) = (char*)"ANALOG"; ;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 712 "def.y"
    { (yyval.string) = (char*)"SCAN"; ;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 714 "def.y"
    { (yyval.string) = (char*)"RESET"; ;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 718 "def.y"
    { (yyval.string) = (char*)""; ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 719 "def.y"
    {dumb_mode=1;;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 720 "def.y"
    { (yyval.string) = (yyvsp[(3) - (3)].string); ;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 723 "def.y"
    { CALLBACK(defrPinEndCbk, defrPinEndCbkType, 0); ;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 725 "def.y"
    {dumb_mode = 2; no_num = 2; ;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 727 "def.y"
    {
          /* 06/05/2002 - Wanda da Rosa - pcr 448455 */
          /* Check for 1 and 0 in the correct position */
          /* 07/26/2002 - Wanda da Rosa - Commented out due to pcr 459218
          if ((($11 == 1) && ($14 != 0)) ||
              (($11 != 1) && ($14 == 0))) {
            yyerror("Syntax is \"DO numX BY 1 STEP spaceX 0\"");
            CHKERR();
          }
          if ((($9 == 1) && ($13 != 0)) ||
              (($9 != 1) && ($13 == 0))) {
            yyerror("Syntax is \"DO 1 BY numY STEP 0 spaceY\"");
            CHKERR();
          }
          */
          /* pcr 459218 - Error if at least numX or numY does not equal 1 */
          if (((yyvsp[(9) - (14)].dval) != 1) && ((yyvsp[(11) - (14)].dval) != 1)) {
            yyerror("Syntax error, either \"DO 1 BY num... or DO numX BY 1...\"");
            CHKERR();
          }
          if (defrRowCbk) {
            defrRow.defiRow::setup((yyvsp[(3) - (14)].string), (yyvsp[(4) - (14)].string), (yyvsp[(5) - (14)].dval), (yyvsp[(6) - (14)].dval), (yyvsp[(7) - (14)].integer));
            defrRow.defiRow::setDo(ROUND((yyvsp[(9) - (14)].dval)), ROUND((yyvsp[(11) - (14)].dval)), (yyvsp[(13) - (14)].dval), (yyvsp[(14) - (14)].dval));
          }
        ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 753 "def.y"
    {
          if (defrRowCbk) {
	    CALLBACK(defrRowCbk, defrRowCbkType, &defrRow);
          }
        ;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 763 "def.y"
    { dumb_mode = 10000000; ;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 764 "def.y"
    { dumb_mode = 0; ;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 771 "def.y"
    {
          if (defrRowCbk) {
             char propTp;
             char* str = ringCopy("                       ");
             propTp =  defrRowProp.defiPropType::propType((yyvsp[(1) - (2)].string));
             /* For backword compatibility, also set the string value */
             /* We will use a temporary string to store the number.
              * The string space is borrowed from the ring buffer
              * in the lexer. */
             sprintf(str, "%g", (yyvsp[(2) - (2)].dval));
             defrRow.defiRow::addNumProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].dval), str, propTp);
          }
        ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 785 "def.y"
    {
          if (defrRowCbk) {
             char propTp;
             propTp =  defrRowProp.defiPropType::propType((yyvsp[(1) - (2)].string));
             defrRow.defiRow::addProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
          }
        ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 793 "def.y"
    {
          if (defrRowCbk) {
             char propTp;
             propTp =  defrRowProp.defiPropType::propType((yyvsp[(1) - (2)].string));
             defrRow.defiRow::addProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
          }
        ;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 802 "def.y"
    {
          if (defrTrackCbk) {
            defrTrack.defiTrack::setup((yyvsp[(1) - (2)].string));
          }
        ;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 808 "def.y"
    {
          if ((yyvsp[(5) - (9)].dval) <= 0)
             yyerror("Invalid numtracks.");
          if ((yyvsp[(7) - (9)].dval) < 0)
             yyerror("Invalid space.");
          if (defrTrackCbk) {
            defrTrack.defiTrack::setDo(ROUND((yyvsp[(2) - (9)].dval)), ROUND((yyvsp[(5) - (9)].dval)), (yyvsp[(7) - (9)].dval));
	    CALLBACK(defrTrackCbk, defrTrackCbkType, &defrTrack);
          }
        ;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 820 "def.y"
    {
          (yyval.string) = (yyvsp[(2) - (2)].string);
        ;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 825 "def.y"
    { (yyval.string) = (char*)"X";;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 827 "def.y"
    { (yyval.string) = (char*)"Y";;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 830 "def.y"
    { dumb_mode = 1000; ;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 831 "def.y"
    { dumb_mode = 0; ;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 838 "def.y"
    {
          if (defrTrackCbk)
            defrTrack.defiTrack::addLayer((yyvsp[(1) - (1)].string));
        ;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 845 "def.y"
    {
          if ((yyvsp[(5) - (8)].dval) <= 0)
             yyerror("Invalid numColumns or numRows.");
          if ((yyvsp[(7) - (8)].dval) < 0)
             yyerror("Invalid space.");
	  if (defrGcellGridCbk) {
	    defrGcellGrid.defiGcellGrid::setup((yyvsp[(2) - (8)].string), ROUND((yyvsp[(3) - (8)].dval)), ROUND((yyvsp[(5) - (8)].dval)), (yyvsp[(7) - (8)].dval));
	    CALLBACK(defrGcellGridCbk, defrGcellGridCbkType, &defrGcellGrid);
	  }
	;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 857 "def.y"
    { ;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 860 "def.y"
    { ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 866 "def.y"
    { CALLBACK(defrViaStartCbk, defrViaStartCbkType, ROUND((yyvsp[(2) - (3)].dval))); ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 872 "def.y"
    {dumb_mode = 1;no_num = 1; ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 873 "def.y"
    { if (defrViaCbk) defrVia.defiVia::setup((yyvsp[(3) - (3)].string)); ;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 875 "def.y"
    { CALLBACK(defrViaCbk, defrViaCbkType, &defrVia); ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 881 "def.y"
    {dumb_mode = 1;no_num = 1; ;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 882 "def.y"
    {
              if (defrViaCbk)
                defrVia.defiVia::addLayer((yyvsp[(4) - (6)].string), (yyvsp[(5) - (6)].pt).x, (yyvsp[(5) - (6)].pt).y, (yyvsp[(6) - (6)].pt).x, (yyvsp[(6) - (6)].pt).y);
            ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 886 "def.y"
    {dumb_mode = 1;no_num = 1; ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 887 "def.y"
    {
              if (defrViaCbk)
                defrVia.defiVia::addPattern((yyvsp[(4) - (4)].string));
            ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 892 "def.y"
    { CALLBACK(defrViaExtCbk, defrViaExtCbkType, History_text); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 896 "def.y"
    {
            save_x = (yyvsp[(2) - (4)].dval);
            save_y = (yyvsp[(3) - (4)].dval);
            (yyval.pt).x = ROUND((yyvsp[(2) - (4)].dval));
            (yyval.pt).y = ROUND((yyvsp[(3) - (4)].dval));
            ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 903 "def.y"
    {
            save_y = (yyvsp[(3) - (4)].dval);
            (yyval.pt).x = ROUND(save_x);
            (yyval.pt).y = ROUND((yyvsp[(3) - (4)].dval));
            ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 909 "def.y"
    {
            save_x = (yyvsp[(2) - (4)].dval);
            (yyval.pt).x = ROUND((yyvsp[(2) - (4)].dval));
            (yyval.pt).y = ROUND(save_y);
            ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 915 "def.y"
    {
            (yyval.pt).x = ROUND(save_x);
            (yyval.pt).y = ROUND(save_y);
            ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 921 "def.y"
    { CALLBACK(defrViaEndCbk, defrViaEndCbkType, 0); ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 924 "def.y"
    { CALLBACK(defrRegionEndCbk, defrRegionEndCbkType, 0); ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 927 "def.y"
    { CALLBACK(defrRegionStartCbk, defrRegionStartCbkType, ROUND((yyvsp[(2) - (3)].dval))); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 931 "def.y"
    {;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 933 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 934 "def.y"
    {
          if (defrRegionCbk)
             defrRegion.defiRegion::setup((yyvsp[(3) - (3)].string));
          regTypeDef = 0;
        ;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 940 "def.y"
    { CALLBACK(defrRegionCbk, defrRegionCbkType, &defrRegion); ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 944 "def.y"
    { if (defrRegionCbk)
	  defrRegion.defiRegion::addRect((yyvsp[(1) - (2)].pt).x, (yyvsp[(1) - (2)].pt).y, (yyvsp[(2) - (2)].pt).x, (yyvsp[(2) - (2)].pt).y); ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 947 "def.y"
    { if (defrRegionCbk)
	  defrRegion.defiRegion::addRect((yyvsp[(2) - (3)].pt).x, (yyvsp[(2) - (3)].pt).y, (yyvsp[(3) - (3)].pt).x, (yyvsp[(3) - (3)].pt).y); ;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 955 "def.y"
    { dumb_mode = 10000000; ;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 956 "def.y"
    { dumb_mode = 0; ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 958 "def.y"
    {
           if (regTypeDef) {
              yyerror("TYPE is already defined");
              CHKERR();
           }
           if (defrRegionCbk) defrRegion.defiRegion::setType((yyvsp[(3) - (3)].string));
           regTypeDef = 1;
         ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 973 "def.y"
    {
          if (defrRegionCbk) {
             char propTp;
             char* str = ringCopy("                       ");
             propTp = defrRegionProp.defiPropType::propType((yyvsp[(1) - (2)].string));
             /* For backword compatibility, also set the string value */
             /* We will use a temporary string to store the number.
              * The string space is borrowed from the ring buffer
              * in the lexer. */
             sprintf(str, "%g", (yyvsp[(2) - (2)].dval));
             defrRegion.defiRegion::addNumProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].dval), str, propTp);
          }
        ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 987 "def.y"
    {
          if (defrRegionCbk) {
             char propTp;
             propTp = defrRegionProp.defiPropType::propType((yyvsp[(1) - (2)].string));
             defrRegion.defiRegion::addProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
          }
        ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 995 "def.y"
    {
          if (defrRegionCbk) {
             char propTp;
             propTp = defrRegionProp.defiPropType::propType((yyvsp[(1) - (2)].string));
             defrRegion.defiRegion::addProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
          }
        ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1004 "def.y"
    { (yyval.string) = (char*)"FENCE"; ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1006 "def.y"
    { (yyval.string) = (char*)"GUIDE"; ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1012 "def.y"
    { CALLBACK(defrComponentStartCbk, defrComponentStartCbkType, ROUND((yyvsp[(2) - (3)].dval))); ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1019 "def.y"
    { CALLBACK(defrComponentCbk, defrComponentCbkType, &defrComponent); ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1022 "def.y"
    {
              dumb_mode = 0;
	      no_num = 0;
              /* The net connections were added to the array in
               * reverse order so fix the order now */
              /* Do not need to reverse the order since the storing is
                 in sequence now  08/07/2001 - Wanda da Rosa
              if (defrComponentCbk)
                defrComponent.defiComponent::reverseNetOrder();
              */
            ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1034 "def.y"
    {dumb_mode = 1000000000; no_num = 10000000; ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1036 "def.y"
    {
            if (defrComponentCbk)
              defrComponent.defiComponent::IdAndName((yyvsp[(3) - (4)].string), (yyvsp[(4) - (4)].string));
            ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1042 "def.y"
    { ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1044 "def.y"
    {
              if (defrComponentCbk)
                defrComponent.defiComponent::addNet("*");
            ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1049 "def.y"
    {
              if (defrComponentCbk)
                defrComponent.defiComponent::addNet((yyvsp[(2) - (2)].string));
            ;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 1064 "def.y"
    { CALLBACK(defrComponentExtCbk, defrComponentExtCbkType, History_text); ;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 1066 "def.y"
    {dumb_mode=1; no_num = 1; ;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 1067 "def.y"
    {
              if (defrComponentCbk)
                defrComponent.defiComponent::setEEQ((yyvsp[(4) - (4)].string));
            ;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 1072 "def.y"
    { dumb_mode = 2;  no_num = 2; ;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 1074 "def.y"
    {
              if (defrComponentCbk)
                defrComponent.defiComponent::setGenerate((yyvsp[(4) - (5)].string), (yyvsp[(5) - (5)].string));
            ;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 1080 "def.y"
    { (yyval.string) = (char*)""; ;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 1082 "def.y"
    { (yyval.string) = (yyvsp[(1) - (1)].string); ;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 1085 "def.y"
    {
              if (defrComponentCbk)
                defrComponent.defiComponent::setSource((yyvsp[(3) - (3)].string));
            ;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 1091 "def.y"
    { (yyval.string) = (char*)"NETLIST"; ;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 1093 "def.y"
    { (yyval.string) = (char*)"DIST"; ;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 1095 "def.y"
    { (yyval.string) = (char*)"USER"; ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 1097 "def.y"
    { (yyval.string) = (char*)"TIMING"; ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 1102 "def.y"
    { ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 1104 "def.y"
    {
	    if (defrComponentCbk)
	      defrComponent.defiComponent::setRegionName((yyvsp[(2) - (2)].string));
	  ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 1110 "def.y"
    { 
          /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
          if (defVersionNum < 5.5) {
            if (defrComponentCbk)
	       defrComponent.defiComponent::setRegionBounds((yyvsp[(1) - (2)].pt).x, (yyvsp[(1) - (2)].pt).y, (yyvsp[(2) - (2)].pt).x, (yyvsp[(2) - (2)].pt).y);
          }
	;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 1118 "def.y"
    { 
          /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
          if (defVersionNum < 5.5) {
	    if (defrComponentCbk)
	       defrComponent.defiComponent::setRegionBounds((yyvsp[(2) - (3)].pt).x, (yyvsp[(2) - (3)].pt).y, (yyvsp[(3) - (3)].pt).x, (yyvsp[(3) - (3)].pt).y);
          }
	;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 1126 "def.y"
    { dumb_mode = 10000000; ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 1128 "def.y"
    { dumb_mode = 0; ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 1135 "def.y"
    {
              if (defrComponentCbk) {
                char propTp;
                char* str = ringCopy("                       ");
                propTp = defrCompProp.defiPropType::propType((yyvsp[(1) - (2)].string));
                /* For backword compatibility, also set the string value */
                /* We will use a temporary string to store the number.
                 * The string space is borrowed from the ring buffer
                 * in the lexer. */
                sprintf(str, "%g", (yyvsp[(2) - (2)].dval));
                defrComponent.defiComponent::addNumProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].dval), str, propTp);
              }
            ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 1149 "def.y"
    {
              if (defrComponentCbk) {
                char propTp;
                propTp = defrCompProp.defiPropType::propType((yyvsp[(1) - (2)].string));
                defrComponent.defiComponent::addProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
              }
            ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 1157 "def.y"
    {
              if (defrComponentCbk) {
                char propTp;
                propTp = defrCompProp.defiPropType::propType((yyvsp[(1) - (2)].string));
                defrComponent.defiComponent::addProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
              }
            ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 1166 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 1168 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 1170 "def.y"
    { 
              if (defrComponentCbk) {
                defrComponent.defiComponent::setForeignName((yyvsp[(4) - (6)].string));
                defrComponent.defiComponent::setForeignLocation(
                (yyvsp[(5) - (6)].pt).x, (yyvsp[(5) - (6)].pt).y, (yyvsp[(6) - (6)].integer));
              }
             ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 1180 "def.y"
    { (yyval.pt) = (yyvsp[(1) - (1)].pt); ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 1182 "def.y"
    { (yyval.pt).x = ROUND((yyvsp[(1) - (2)].dval)); (yyval.pt).y = ROUND((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 1185 "def.y"
    {
              if (defrComponentCbk) {
                defrComponent.defiComponent::setPlacementStatus((yyvsp[(1) - (3)].integer));
                defrComponent.defiComponent::setPlacementLocation(
                (yyvsp[(2) - (3)].pt).x, (yyvsp[(2) - (3)].pt).y, (yyvsp[(3) - (3)].integer));
              }
            ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 1193 "def.y"
    {
              if (defrComponentCbk)
                defrComponent.defiComponent::setPlacementStatus(
                                             DEFI_COMPONENT_UNPLACED);
                defrComponent.defiComponent::setPlacementLocation(
                                             -1, -1, -1);
            ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 1201 "def.y"
    {
              if (defVersionNum < 5.4) {   /* PCR 495463 */
                if (defrComponentCbk) {
                  defrComponent.defiComponent::setPlacementStatus(
                                              DEFI_COMPONENT_UNPLACED);
                  defrComponent.defiComponent::setPlacementLocation(
                                              (yyvsp[(3) - (4)].pt).x, (yyvsp[(3) - (4)].pt).y, (yyvsp[(4) - (4)].integer));
                }
              } else {
                yydefwarning("pt & orient are not allow in 5.4 and later, they are ignored.");
                if (defrComponentCbk)
                  defrComponent.defiComponent::setPlacementStatus(
                                               DEFI_COMPONENT_UNPLACED);
                  defrComponent.defiComponent::setPlacementLocation(
                                               -1, -1, -1);
              }
            ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 1220 "def.y"
    { (yyval.integer) = DEFI_COMPONENT_FIXED; ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 1222 "def.y"
    { (yyval.integer) = DEFI_COMPONENT_COVER; ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 1224 "def.y"
    { (yyval.integer) = DEFI_COMPONENT_PLACED; ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 1227 "def.y"
    {
              if (defrComponentCbk)
                defrComponent.defiComponent::setWeight(ROUND((yyvsp[(3) - (3)].dval)));
            ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 1233 "def.y"
    { CALLBACK(defrComponentEndCbk, defrComponentEndCbkType, 0); ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 1239 "def.y"
    { CALLBACK(defrNetStartCbk, defrNetStartCbkType, ROUND((yyvsp[(2) - (3)].dval))); ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 1246 "def.y"
    { CALLBACK(defrNetCbk, defrNetCbkType, &defrNet); ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 1254 "def.y"
    {dumb_mode = 0; no_num = 0; ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 1257 "def.y"
    {dumb_mode = 1000000000; no_num = 10000000; nondef_is_keyword = TRUE; mustjoin_is_keyword = TRUE;;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 1260 "def.y"
    {
              /* 9/22/1999 -- Wanda da Rosa */
              /* this is shared by both net and special net */
              if (defrNetCbk || defrSNetCbk)
                defrNet.defiNet::setName((yyvsp[(1) - (1)].string));
            ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 1266 "def.y"
    {dumb_mode = 1; no_num = 1;;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 1267 "def.y"
    {
            if (defrNetCbk || defrSNetCbk)
              defrNet.defiNet::addMustPin((yyvsp[(3) - (6)].string), (yyvsp[(5) - (6)].string), 0);
            dumb_mode = 3;
            no_num = 3;
            ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 1278 "def.y"
    {dumb_mode = 1000000000; no_num = 10000000;;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 1279 "def.y"
    {
            /* 9/22/1999 -- Wanda da Rosa */
            /* since the code is shared by both net & special net, */
            /* need to check on both flags */
            if (defrNetCbk || defrSNetCbk)
              defrNet.defiNet::addPin((yyvsp[(2) - (6)].string), (yyvsp[(4) - (6)].string), (yyvsp[(5) - (6)].integer));
            /* 1/14/2000 -- Wanda da Rosa, pcr 289156 */
            /* reset dumb_mode & no_num to 3 , just in case */
            /* the next statement is another net_connection */
            dumb_mode = 3;
            no_num = 3;
            ;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 1291 "def.y"
    {dumb_mode = 1; no_num = 1;;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 1292 "def.y"
    {
            if (defrNetCbk || defrSNetCbk)
              defrNet.defiNet::addPin("*", (yyvsp[(4) - (6)].string), (yyvsp[(5) - (6)].integer));
            dumb_mode = 3;
            no_num = 3;
            ;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 1298 "def.y"
    {dumb_mode = 1; no_num = 1;;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 1299 "def.y"
    {
            if (defrNetCbk || defrSNetCbk)
              defrNet.defiNet::addPin("PIN", (yyvsp[(4) - (6)].string), (yyvsp[(5) - (6)].integer));
            dumb_mode = 3;
            no_num = 3;
            ;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 1307 "def.y"
    { (yyval.integer) = 0; ;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 1309 "def.y"
    {
	    CALLBACK(defrNetConnectionExtCbk, defrNetConnectionExtCbkType,
              History_text);
	    (yyval.integer) = 0; ;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 1314 "def.y"
    { (yyval.integer) = 1; ;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 1321 "def.y"
    { dumb_mode = 1; ;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 1322 "def.y"
    {  
            if (defrNetCbk) defrNet.defiNet::addWire((yyvsp[(2) - (3)].string), NULL);
            ;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 1326 "def.y"
    {
            by_is_keyword = FALSE;
            do_is_keyword = FALSE;
            new_is_keyword = FALSE;
            nondef_is_keyword = FALSE;
            mustjoin_is_keyword = FALSE;
            step_is_keyword = FALSE;
            ;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 1335 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setSource((yyvsp[(3) - (3)].string)); ;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 1338 "def.y"
    {
              if (defVersionNum < 5.5) {
                yyerror("FIXEDBUMP is a 5.5 syntax. Your def file is not defined as 5.5");
                CHKERR();
              }
              if (defrNetCbk) defrNet.defiNet::setFixedbump();
            ;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 1347 "def.y"
    {
              if (defVersionNum < 5.5) {
                yyerror("FIXEDBUMP is a 5.5 syntax. Your def file is not defined as 5.5");
                CHKERR();
              }
              if (defrNetCbk) defrNet.defiNet::setFrequency((yyvsp[(3) - (3)].dval));
            ;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 1355 "def.y"
    {dumb_mode = 1; no_num = 1;;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 1356 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setOriginal((yyvsp[(4) - (4)].string)); ;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 1359 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setPattern((yyvsp[(3) - (3)].string)); ;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 1362 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setWeight(ROUND((yyvsp[(3) - (3)].dval))); ;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 1365 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setXTalk(ROUND((yyvsp[(3) - (3)].dval))); ;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 1368 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setCap((yyvsp[(3) - (3)].dval)); ;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 1371 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setUse((yyvsp[(3) - (3)].string)); ;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 1373 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 1374 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setStyle((yyvsp[(4) - (4)].string)); ;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 1376 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 1377 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setNonDefaultRule((yyvsp[(4) - (4)].string)); ;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 1381 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 1382 "def.y"
    { if (defrNetCbk) defrNet.defiNet::addShieldNet((yyvsp[(4) - (4)].string)); ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 1384 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 1385 "def.y"
    {  /* since the parser still support 5.3 and earlier, can't */
               /* move NOSHIELD in net_type */
              if (defVersionNum < 5.4) {   /* PCR 445209 */
                if (defrNetCbk) defrNet.defiNet::addNoShield("");
                by_is_keyword = FALSE;
                do_is_keyword = FALSE;
                new_is_keyword = FALSE;
                step_is_keyword = FALSE;
                shield = TRUE;    /* save the path info in the shield paths */
              } else
                if (defrNetCbk) defrNet.defiNet::addWire("NOSHIELD", NULL);
            ;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 1398 "def.y"
    {
              if (defVersionNum < 5.4)   /* PCR 445209 */
                shield = FALSE;
              else {
                by_is_keyword = FALSE;
                do_is_keyword = FALSE;
                new_is_keyword = FALSE;
                step_is_keyword = FALSE;
                nondef_is_keyword = FALSE;
                mustjoin_is_keyword = FALSE;
              }
            ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 1411 "def.y"
    { dumb_mode = 1; no_num = 1;
                         if (defrNetCbk) {
                           defrSubnet =
                           (defiSubnet*)malloc(sizeof(defiSubnet));
                           defrSubnet->defiSubnet::Init();
                         }
                       ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 1418 "def.y"
    {
               if (defrNetCbk) {
                 defrSubnet->defiSubnet::setName((yyvsp[(4) - (4)].string));
               }
            ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 1423 "def.y"
    {
               if (defrNetCbk) {
                 defrNet.defiNet::addSubnet(defrSubnet);
                 defrSubnet = 0;
               }
            ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 1430 "def.y"
    {dumb_mode = 10000000;;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 1431 "def.y"
    { dumb_mode = 0; ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 1434 "def.y"
    { CALLBACK(defrNetExtCbk, defrNetExtCbkType, History_text); ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 1441 "def.y"
    {
              if (defrNetCbk) {
                char propTp;
                char* str = ringCopy("                       ");
                propTp = defrNetProp.defiPropType::propType((yyvsp[(1) - (2)].string));
                /* For backword compatibility, also set the string value */
                /* We will use a temporary string to store the number.
                 * The string space is borrowed from the ring buffer
                 * in the lexer. */
                sprintf(str, "%g", (yyvsp[(2) - (2)].dval));
                defrNet.defiNet::addNumProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].dval), str, propTp);
              }
            ;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 1455 "def.y"
    {
              if (defrNetCbk) {
                char propTp;
                propTp = defrNetProp.defiPropType::propType((yyvsp[(1) - (2)].string));
                defrNet.defiNet::addProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
              }
            ;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 1463 "def.y"
    {
              if (defrNetCbk) {
                char propTp;
                propTp = defrNetProp.defiPropType::propType((yyvsp[(1) - (2)].string));
                defrNet.defiNet::addProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
              }
            ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 1472 "def.y"
    { (yyval.string) = (char*)"NETLIST"; ;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 1474 "def.y"
    { (yyval.string) = (char*)"DIST"; ;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 1476 "def.y"
    { (yyval.string) = (char*)"USER"; ;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 1478 "def.y"
    { (yyval.string) = (char*)"TIMING"; ;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 1480 "def.y"
    { (yyval.string) = (char*)"TEST"; ;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 1483 "def.y"
    { if (defrNetCbk)
	defrNet.defiNet::addVpinBounds((yyvsp[(3) - (5)].pt).x, (yyvsp[(3) - (5)].pt).y, (yyvsp[(4) - (5)].pt).x, (yyvsp[(4) - (5)].pt).y); ;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 1486 "def.y"
    {dumb_mode = 1; no_num = 1;;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 1487 "def.y"
    { if (defrNetCbk) defrNet.defiNet::addVpin((yyvsp[(4) - (4)].string)); ;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 1490 "def.y"
    {dumb_mode=1;;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 1491 "def.y"
    { if (defrNetCbk) defrNet.defiNet::addVpinLayer((yyvsp[(3) - (3)].string)); ;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 1495 "def.y"
    { if (defrNetCbk) defrNet.defiNet::addVpinLoc((yyvsp[(1) - (3)].string), (yyvsp[(2) - (3)].pt).x, (yyvsp[(2) - (3)].pt).y, (yyvsp[(3) - (3)].integer)); ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 1498 "def.y"
    { (yyval.string) = (char*)"PLACED"; ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 1500 "def.y"
    { (yyval.string) = (char*)"FIXED"; ;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 1502 "def.y"
    { (yyval.string) = (char*)"COVER"; ;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 1505 "def.y"
    { (yyval.string) = (char*)"FIXED"; ;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 1507 "def.y"
    { (yyval.string) = (char*)"COVER"; ;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 1509 "def.y"
    { (yyval.string) = (char*)"ROUTED"; ;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 1513 "def.y"
    { if (defrNeedPathData) pathIsDone(shield); ;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 1515 "def.y"
    { ;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 1517 "def.y"
    { dumb_mode = 1; ;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 1518 "def.y"
    { if (defrNeedPathData) pathIsDone(shield); ;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 1521 "def.y"
    {
        /* 9/26/2002 - Wanda da Rosa - pcr 449514
           Check if $1 is equal to TAPER or TAPERRULE, layername
           is missing */
        if ((strcmp((yyvsp[(1) - (1)].string), "TAPER") == 0) || (strcmp((yyvsp[(1) - (1)].string), "TAPERRULE") == 0)) {
           yyerror("layerName is missing");
           CHKERR();
           /* Since there is already error, the next token is insignificant */
           dumb_mode = 1; no_num = 1;
        } else {
           if (defrNeedPathData) defrPath->defiPath::addLayer((yyvsp[(1) - (1)].string));
           dumb_mode = 0; no_num = 0;
        }
      ;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 1537 "def.y"
    { dumb_mode = 1000000000; by_is_keyword = TRUE; do_is_keyword = TRUE;
        new_is_keyword = TRUE; step_is_keyword = TRUE; ;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 1540 "def.y"
    { dumb_mode = 0; ;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 1548 "def.y"
    { if (defrNeedPathData) defrPath->defiPath::addVia((yyvsp[(1) - (1)].string)); ;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 1550 "def.y"
    { if (defrNeedPathData) {
            defrPath->defiPath::addVia((yyvsp[(1) - (2)].string));
            defrPath->defiPath::addViaRotation((yyvsp[(2) - (2)].integer));
        }
      ;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 1556 "def.y"
    {
        if (defVersionNum < 5.5) {
          yyerror("VIA DO is a 5.5 syntax. Your def file is not defined as 5.5");
           CHKERR();
        }
        if (((yyvsp[(3) - (8)].dval) == 0) || ((yyvsp[(5) - (8)].dval) == 0)) {
          yyerror("In the VIA DO construct, neither numX nor numY can be 0");
          CHKERR();
        }
        if (defrNeedPathData) {
            defrPath->defiPath::addVia((yyvsp[(1) - (8)].string));
            defrPath->defiPath::addViaData((yyvsp[(3) - (8)].dval), (yyvsp[(5) - (8)].dval), (yyvsp[(7) - (8)].dval), (yyvsp[(8) - (8)].dval));
        }
      ;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 1571 "def.y"
    { ;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 1576 "def.y"
    {
	if (defrNeedPathData)
	  defrPath->defiPath::addPoint(ROUND((yyvsp[(2) - (4)].dval)), ROUND((yyvsp[(3) - (4)].dval))); 
	save_x = (yyvsp[(2) - (4)].dval);
	save_y = (yyvsp[(3) - (4)].dval);
      ;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 1583 "def.y"
    {
	if (defrNeedPathData)
	  defrPath->defiPath::addPoint(ROUND(save_x), ROUND((yyvsp[(3) - (4)].dval))); 
	save_y = (yyvsp[(3) - (4)].dval);
      ;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 1589 "def.y"
    {
	if (defrNeedPathData)
	  defrPath->defiPath::addPoint(ROUND((yyvsp[(2) - (4)].dval)), ROUND(save_y)); 
	save_x = (yyvsp[(2) - (4)].dval);
      ;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 1595 "def.y"
    {
	if (defrNeedPathData)
	  defrPath->defiPath::addPoint(ROUND(save_x), ROUND(save_y)); 
      ;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 1600 "def.y"
    {
	if (defrNeedPathData)
	  defrPath->defiPath::addFlushPoint(ROUND((yyvsp[(2) - (5)].dval)), ROUND((yyvsp[(3) - (5)].dval)), ROUND((yyvsp[(4) - (5)].dval))); 
	save_x = (yyvsp[(2) - (5)].dval);
	save_y = (yyvsp[(3) - (5)].dval);
      ;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 1607 "def.y"
    {
	if (defrNeedPathData)
	  defrPath->defiPath::addFlushPoint(ROUND(save_x), ROUND((yyvsp[(3) - (5)].dval)),
	  ROUND((yyvsp[(4) - (5)].dval))); 
	save_y = (yyvsp[(3) - (5)].dval);
      ;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 1614 "def.y"
    {
	if (defrNeedPathData)
	  defrPath->defiPath::addFlushPoint(ROUND((yyvsp[(2) - (5)].dval)), ROUND(save_y),
	  ROUND((yyvsp[(4) - (5)].dval))); 
	save_x = (yyvsp[(2) - (5)].dval);
      ;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 1621 "def.y"
    {
	if (defrNeedPathData)
	  defrPath->defiPath::addFlushPoint(ROUND(save_x), ROUND(save_y),
	  ROUND((yyvsp[(4) - (5)].dval))); 
      ;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 1629 "def.y"
    { ;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 1631 "def.y"
    { if (defrNeedPathData) defrPath->defiPath::setTaper(); ;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 1632 "def.y"
    { dumb_mode = 1; ;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 1633 "def.y"
    { if (defrNeedPathData) defrPath->defiPath::addTaperRule((yyvsp[(3) - (3)].string)); ;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 1637 "def.y"
    { ;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 1639 "def.y"
    { if (defrNeedPathData) defrPath->defiPath::addShape((yyvsp[(3) - (3)].string)); ;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 1643 "def.y"
    { ;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 1645 "def.y"
    { if (defrNeedPathData) defrPath->defiPath::addWidth(ROUND((yyvsp[(1) - (1)].dval))); ;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 1649 "def.y"
    { CALLBACK(defrNetEndCbk, defrNetEndCbkType, 0); ;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 1652 "def.y"
    { (yyval.string) = (char*)"RING"; ;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 1654 "def.y"
    { (yyval.string) = (char*)"STRIPE"; ;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 1656 "def.y"
    { (yyval.string) = (char*)"FOLLOWPIN"; ;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 1658 "def.y"
    { (yyval.string) = (char*)"IOWIRE"; ;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 1660 "def.y"
    { (yyval.string) = (char*)"COREWIRE"; ;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 1662 "def.y"
    { (yyval.string) = (char*)"BLOCKWIRE"; ;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 1664 "def.y"
    { (yyval.string) = (char*)"FILLWIRE"; ;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 1666 "def.y"
    { (yyval.string) = (char*)"DRCFILL"; ;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 1668 "def.y"
    { (yyval.string) = (char*)"BLOCKAGEWIRE"; ;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 1670 "def.y"
    { (yyval.string) = (char*)"PADRING"; ;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 1672 "def.y"
    { (yyval.string) = (char*)"BLOCKRING"; ;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 1682 "def.y"
    { CALLBACK(defrSNetCbk, defrSNetCbkType, &defrNet); ;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 1692 "def.y"
    { dumb_mode = 1; ;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 1693 "def.y"
    {
            if (defrNetCbk) defrNet.defiNet::addWire((yyvsp[(2) - (3)].string), NULL);
            ;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 1697 "def.y"
    {
            by_is_keyword = FALSE;
            do_is_keyword = FALSE;
            new_is_keyword = FALSE;
            step_is_keyword = FALSE;
            ;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 1704 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 1705 "def.y"
    { /* since the parser still supports 5.3 and earlier, */
              /* can't just move SHIELD in net_type */
              if (defVersionNum < 5.4) { /* PCR 445209 */
                if (defrNetCbk) defrNet.defiNet::addShield((yyvsp[(4) - (4)].string));
                by_is_keyword = FALSE;
                do_is_keyword = FALSE;
                new_is_keyword = FALSE;
                step_is_keyword = FALSE;
                shield = TRUE;   /* save the path info in the shield paths */
              } else
                if (defrNetCbk) defrNet.defiNet::addWire("SHIELD", (yyvsp[(4) - (4)].string));
            ;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 1718 "def.y"
    {
              if (defVersionNum < 5.4)   /* PCR 445209 */
                shield = FALSE;
              else
                by_is_keyword = FALSE;
                do_is_keyword = FALSE;
                new_is_keyword = FALSE;
                step_is_keyword = FALSE;
            ;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 1729 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setSource((yyvsp[(3) - (3)].string)); ;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 1732 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setFixedbump(); ;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 1735 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setFrequency((yyvsp[(3) - (3)].dval)); ;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 1737 "def.y"
    {dumb_mode = 1; no_num = 1;;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 1738 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setOriginal((yyvsp[(4) - (4)].string)); ;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 1741 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setPattern((yyvsp[(3) - (3)].string)); ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 1744 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setWeight(ROUND((yyvsp[(3) - (3)].dval))); ;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 1747 "def.y"
    { 
              /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
              if (defVersionNum < 5.5)
                 if (defrNetCbk) defrNet.defiNet::setCap((yyvsp[(3) - (3)].dval));
            ;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 1754 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setUse((yyvsp[(3) - (3)].string)); ;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 1756 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 1757 "def.y"
    { if (defrNetCbk) defrNet.defiNet::setStyle((yyvsp[(4) - (4)].string)); ;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 1759 "def.y"
    {dumb_mode = 10000000;;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 1760 "def.y"
    { dumb_mode = 0; ;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 1763 "def.y"
    { CALLBACK(defrNetExtCbk, defrNetExtCbkType, History_text); ;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 1765 "def.y"
    { dumb_mode = 1; ;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 1766 "def.y"
    {
              /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
              if (defVersionNum < 5.5)
                 if (defrSNetCbk) defrNet.defiNet::setWidth((yyvsp[(4) - (5)].string), (yyvsp[(5) - (5)].dval));
            ;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 1773 "def.y"
    {
              if (defrSNetCbk) defrNet.defiNet::setVoltage((yyvsp[(3) - (3)].dval));
            ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 1777 "def.y"
    { dumb_mode = 1; ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 1778 "def.y"
    {
              if (defrSNetCbk) defrNet.defiNet::setSpacing((yyvsp[(4) - (5)].string),(yyvsp[(5) - (5)].dval));
            ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 1782 "def.y"
    {
            ;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 1790 "def.y"
    {
              if (defrNetCbk) {
                char propTp;
                char* str = ringCopy("                       ");
                propTp = defrSNetProp.defiPropType::propType((yyvsp[(1) - (2)].string));
                /* For backword compatibility, also set the string value */
                /* We will use a temporary string to store the number.
                 * The string space is borrowed from the ring buffer
                 * in the lexer. */
                sprintf(str, "%g", (yyvsp[(2) - (2)].dval));
                defrNet.defiNet::addNumProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].dval), str, propTp);
              }
            ;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 1804 "def.y"
    {
              if (defrNetCbk) {
                char propTp;
                propTp = defrSNetProp.defiPropType::propType((yyvsp[(1) - (2)].string));
                defrNet.defiNet::addProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
              }
            ;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 1812 "def.y"
    {
              if (defrNetCbk) {
                char propTp;
                propTp = defrSNetProp.defiPropType::propType((yyvsp[(1) - (2)].string));
                defrNet.defiNet::addProp((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
              }
            ;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 1822 "def.y"
    {
              if (defrSNetCbk) defrNet.defiNet::setRange((yyvsp[(2) - (3)].dval),(yyvsp[(3) - (3)].dval));
            ;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 1828 "def.y"
    { defrProp.defiProp::setRange((yyvsp[(2) - (3)].dval), (yyvsp[(3) - (3)].dval)); ;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 1831 "def.y"
    { (yyval.string) = (char*)"BALANCED"; ;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 1833 "def.y"
    { (yyval.string) = (char*)"STEINER"; ;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 1835 "def.y"
    { (yyval.string) = (char*)"TRUNK"; ;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 1837 "def.y"
    { (yyval.string) = (char*)"WIREDLOGIC"; ;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 1841 "def.y"
    { if (defrNeedPathData) pathIsDone(shield); ;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 1843 "def.y"
    { ;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 1845 "def.y"
    { dumb_mode = 1; ;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 1846 "def.y"
    { if (defrNeedPathData) pathIsDone(shield); ;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 1849 "def.y"
    { if (defrNeedPathData) defrPath->defiPath::addLayer((yyvsp[(1) - (1)].string));
        dumb_mode = 0; no_num = 0;
      ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 1853 "def.y"
    { dumb_mode = 1; new_is_keyword = TRUE; ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 1855 "def.y"
    { dumb_mode = 0; ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 1858 "def.y"
    { if (defrNeedPathData) defrPath->defiPath::addWidth(ROUND((yyvsp[(1) - (1)].dval))); ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 1861 "def.y"
    { CALLBACK(defrSNetStartCbk, defrSNetStartCbkType, ROUND((yyvsp[(2) - (3)].dval))); ;}
    break;

  case 461:

/* Line 1455 of yacc.c  */
#line 1864 "def.y"
    { CALLBACK(defrSNetEndCbk, defrSNetEndCbkType, 0); ;}
    break;

  case 463:

/* Line 1455 of yacc.c  */
#line 1870 "def.y"
    { CALLBACK(defrGroupsStartCbk, defrGroupsStartCbkType, ROUND((yyvsp[(2) - (3)].dval))); ;}
    break;

  case 466:

/* Line 1455 of yacc.c  */
#line 1877 "def.y"
    { CALLBACK(defrGroupCbk, defrGroupCbkType, &defrGroup); ;}
    break;

  case 467:

/* Line 1455 of yacc.c  */
#line 1879 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 468:

/* Line 1455 of yacc.c  */
#line 1880 "def.y"
    {
              dumb_mode = 1000000000;
              no_num = 1000000000;
              /* dumb_mode is automatically turned off at the first
               * + in the options or at the ; at the end of the group */
              if (defrGroupCbk) defrGroup.defiGroup::setup((yyvsp[(3) - (3)].string));
              CALLBACK(defrGroupNameCbk, defrGroupNameCbkType, (yyvsp[(3) - (3)].string));
            ;}
    break;

  case 470:

/* Line 1455 of yacc.c  */
#line 1891 "def.y"
    {  ;}
    break;

  case 471:

/* Line 1455 of yacc.c  */
#line 1894 "def.y"
    {
              /* if (defrGroupCbk) defrGroup.defiGroup::addMember($1); */
              CALLBACK(defrGroupMemberCbk, defrGroupMemberCbkType, (yyvsp[(1) - (1)].string));
            ;}
    break;

  case 474:

/* Line 1455 of yacc.c  */
#line 1904 "def.y"
    { ;}
    break;

  case 475:

/* Line 1455 of yacc.c  */
#line 1905 "def.y"
    { dumb_mode = 10000000; ;}
    break;

  case 476:

/* Line 1455 of yacc.c  */
#line 1906 "def.y"
    { dumb_mode = 0; ;}
    break;

  case 477:

/* Line 1455 of yacc.c  */
#line 1907 "def.y"
    { dumb_mode = 1;  no_num = 1; ;}
    break;

  case 478:

/* Line 1455 of yacc.c  */
#line 1908 "def.y"
    { ;}
    break;

  case 479:

/* Line 1455 of yacc.c  */
#line 1910 "def.y"
    { CALLBACK(defrGroupExtCbk, defrGroupExtCbkType,
	      History_text); ;}
    break;

  case 480:

/* Line 1455 of yacc.c  */
#line 1915 "def.y"
    {
            /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
            if (defVersionNum < 5.5) {
               if (defrGroupCbk)
                  defrGroup.defiGroup::addRegionRect((yyvsp[(1) - (2)].pt).x, (yyvsp[(1) - (2)].pt).y, (yyvsp[(2) - (2)].pt).x, (yyvsp[(2) - (2)].pt).y);
            }
          ;}
    break;

  case 481:

/* Line 1455 of yacc.c  */
#line 1923 "def.y"
    { if (defrGroupCbk)
	    defrGroup.defiGroup::setRegionName((yyvsp[(1) - (1)].string));
	  ;}
    break;

  case 484:

/* Line 1455 of yacc.c  */
#line 1932 "def.y"
    {
          if (defrGroupCbk) {
             char propTp;
             char* str = ringCopy("                       ");
             propTp = defrGroupProp.defiPropType::propType((yyvsp[(1) - (2)].string));
             /* For backword compatibility, also set the string value */
             /* We will use a temporary string to store the number.
              * The string space is borrowed from the ring buffer
              * in the lexer. */
             sprintf(str, "%g", (yyvsp[(2) - (2)].dval));
             defrGroup.defiGroup::addNumProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].dval), str, propTp);
          }
        ;}
    break;

  case 485:

/* Line 1455 of yacc.c  */
#line 1946 "def.y"
    {
          if (defrGroupCbk) {
             char propTp;
             propTp = defrGroupProp.defiPropType::propType((yyvsp[(1) - (2)].string));
             defrGroup.defiGroup::addProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
          }
        ;}
    break;

  case 486:

/* Line 1455 of yacc.c  */
#line 1954 "def.y"
    {
          if (defrGroupCbk) {
             char propTp;
             propTp = defrGroupProp.defiPropType::propType((yyvsp[(1) - (2)].string));
             defrGroup.defiGroup::addProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
          }
        ;}
    break;

  case 488:

/* Line 1455 of yacc.c  */
#line 1964 "def.y"
    { ;}
    break;

  case 489:

/* Line 1455 of yacc.c  */
#line 1967 "def.y"
    {
               /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
               if (defVersionNum < 5.5)
                  if (defrGroupCbk) defrGroup.defiGroup::setMaxX(ROUND((yyvsp[(2) - (2)].dval)));
             ;}
    break;

  case 490:

/* Line 1455 of yacc.c  */
#line 1973 "def.y"
    { 
               /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
               if (defVersionNum < 5.5)
                  if (defrGroupCbk) defrGroup.defiGroup::setMaxY(ROUND((yyvsp[(2) - (2)].dval)));
             ;}
    break;

  case 491:

/* Line 1455 of yacc.c  */
#line 1979 "def.y"
    { 
               /* 11/12/2002 - this is obsoleted in 5.5, & will be ignored */
               if (defVersionNum < 5.5)
                  if (defrGroupCbk) defrGroup.defiGroup::setPerim(ROUND((yyvsp[(2) - (2)].dval)));
             ;}
    break;

  case 492:

/* Line 1455 of yacc.c  */
#line 1986 "def.y"
    { CALLBACK(defrGroupsEndCbk, defrGroupsEndCbkType, 0); ;}
    break;

  case 495:

/* Line 1455 of yacc.c  */
#line 1997 "def.y"
    {
          if (defVersionNum < 5.4) {
	    CALLBACK(defrAssertionsStartCbk, defrAssertionsStartCbkType,
	        ROUND((yyvsp[(2) - (3)].dval)));
          } else
            yydefwarning("ASSERTIONS is obsolete in 5.4. It will be ignored.");
	  if (defrAssertionCbk)
            defrAssertion.defiAssertion::setAssertionMode();
	;}
    break;

  case 496:

/* Line 1455 of yacc.c  */
#line 2008 "def.y"
    {
          if (defVersionNum < 5.4) {
            CALLBACK(defrConstraintsStartCbk, defrConstraintsStartCbkType,
              ROUND((yyvsp[(2) - (3)].dval)));
          } else
            yydefwarning("CONSTRAINTS is obsolete in 5.4. It will be ignored.");
	  if (defrConstraintCbk)
	    defrAssertion.defiAssertion::setConstraintMode();
        ;}
    break;

  case 500:

/* Line 1455 of yacc.c  */
#line 2024 "def.y"
    {
          if (defVersionNum < 5.4) {
            if (defrAssertion.defiAssertion::isConstraint()) 
	      CALLBACK(defrConstraintCbk, defrConstraintCbkType,
	               &defrAssertion);
            if (defrAssertion.defiAssertion::isAssertion()) 
	      CALLBACK(defrAssertionCbk, defrAssertionCbkType,
	   	       &defrAssertion);
          }
        ;}
    break;

  case 501:

/* Line 1455 of yacc.c  */
#line 2036 "def.y"
    { 
          if (defVersionNum < 5.4) {
            if (defrAssertion.defiAssertion::isConstraint()) 
	      CALLBACK(defrConstraintCbk, defrConstraintCbkType,
	               &defrAssertion);
            if (defrAssertion.defiAssertion::isAssertion()) 
              CALLBACK(defrAssertionCbk, defrAssertionCbkType,
		       &defrAssertion);
          }
   
          // reset all the flags and everything
          defrAssertion.defiAssertion::clear();
        ;}
    break;

  case 502:

/* Line 1455 of yacc.c  */
#line 2050 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 503:

/* Line 1455 of yacc.c  */
#line 2051 "def.y"
    {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::addNet((yyvsp[(3) - (3)].string));
            ;}
    break;

  case 504:

/* Line 1455 of yacc.c  */
#line 2055 "def.y"
    {dumb_mode = 4; no_num = 4;;}
    break;

  case 505:

/* Line 1455 of yacc.c  */
#line 2057 "def.y"
    {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::addPath((yyvsp[(3) - (6)].string), (yyvsp[(4) - (6)].string), (yyvsp[(5) - (6)].string), (yyvsp[(6) - (6)].string));
            ;}
    break;

  case 506:

/* Line 1455 of yacc.c  */
#line 2062 "def.y"
    {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::setSum();
            ;}
    break;

  case 507:

/* Line 1455 of yacc.c  */
#line 2067 "def.y"
    {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::setDiff();
            ;}
    break;

  case 509:

/* Line 1455 of yacc.c  */
#line 2074 "def.y"
    { ;}
    break;

  case 510:

/* Line 1455 of yacc.c  */
#line 2076 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 511:

/* Line 1455 of yacc.c  */
#line 2078 "def.y"
    {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::setWiredlogic((yyvsp[(4) - (8)].string), (yyvsp[(7) - (8)].dval));
            ;}
    break;

  case 512:

/* Line 1455 of yacc.c  */
#line 2085 "def.y"
    { (yyval.string) = (char*)""; ;}
    break;

  case 513:

/* Line 1455 of yacc.c  */
#line 2087 "def.y"
    { (yyval.string) = (char*)"+"; ;}
    break;

  case 516:

/* Line 1455 of yacc.c  */
#line 2094 "def.y"
    {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::setRiseMin((yyvsp[(3) - (3)].dval));
            ;}
    break;

  case 517:

/* Line 1455 of yacc.c  */
#line 2099 "def.y"
    {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::setRiseMax((yyvsp[(3) - (3)].dval));
            ;}
    break;

  case 518:

/* Line 1455 of yacc.c  */
#line 2104 "def.y"
    {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::setFallMin((yyvsp[(3) - (3)].dval));
            ;}
    break;

  case 519:

/* Line 1455 of yacc.c  */
#line 2109 "def.y"
    {
              if (defrConstraintCbk || defrAssertionCbk)
                defrAssertion.defiAssertion::setFallMax((yyvsp[(3) - (3)].dval));
            ;}
    break;

  case 520:

/* Line 1455 of yacc.c  */
#line 2115 "def.y"
    { CALLBACK(defrConstraintsEndCbk, defrConstraintsEndCbkType, 0); ;}
    break;

  case 521:

/* Line 1455 of yacc.c  */
#line 2118 "def.y"
    { CALLBACK(defrAssertionsEndCbk, defrAssertionsEndCbkType, 0); ;}
    break;

  case 523:

/* Line 1455 of yacc.c  */
#line 2124 "def.y"
    { CALLBACK(defrScanchainsStartCbk, defrScanchainsStartCbkType,
              ROUND((yyvsp[(2) - (3)].dval))); ;}
    break;

  case 525:

/* Line 1455 of yacc.c  */
#line 2129 "def.y"
    {;}
    break;

  case 526:

/* Line 1455 of yacc.c  */
#line 2132 "def.y"
    { 
           CALLBACK(defrScanchainCbk, defrScanchainCbkType, &defrScanchain);
      ;}
    break;

  case 527:

/* Line 1455 of yacc.c  */
#line 2136 "def.y"
    {dumb_mode = 1; no_num = 1;;}
    break;

  case 528:

/* Line 1455 of yacc.c  */
#line 2137 "def.y"
    {
              if (defrScanchainCbk)
                defrScanchain.defiScanchain::setName((yyvsp[(3) - (3)].string));
              bit_is_keyword = TRUE;
            ;}
    break;

  case 531:

/* Line 1455 of yacc.c  */
#line 2149 "def.y"
    { (yyval.string) = (char*)""; ;}
    break;

  case 532:

/* Line 1455 of yacc.c  */
#line 2151 "def.y"
    { (yyval.string) = (yyvsp[(1) - (1)].string); ;}
    break;

  case 533:

/* Line 1455 of yacc.c  */
#line 2153 "def.y"
    {dumb_mode = 2; no_num = 2;;}
    break;

  case 534:

/* Line 1455 of yacc.c  */
#line 2154 "def.y"
    { if (defrScanchainCbk)
	    defrScanchain.defiScanchain::setStart((yyvsp[(4) - (5)].string), (yyvsp[(5) - (5)].string));
          ;}
    break;

  case 535:

/* Line 1455 of yacc.c  */
#line 2157 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 536:

/* Line 1455 of yacc.c  */
#line 2158 "def.y"
    { dumb_mode = 0; no_num = 0; ;}
    break;

  case 537:

/* Line 1455 of yacc.c  */
#line 2160 "def.y"
    {
            dumb_mode = 1;
	    no_num = 1;
            if (defrScanchainCbk)
              defrScanchain.defiScanchain::addOrderedList();
          ;}
    break;

  case 538:

/* Line 1455 of yacc.c  */
#line 2167 "def.y"
    { dumb_mode = 0; no_num = 0; ;}
    break;

  case 539:

/* Line 1455 of yacc.c  */
#line 2168 "def.y"
    {dumb_mode = 2; no_num = 2; ;}
    break;

  case 540:

/* Line 1455 of yacc.c  */
#line 2169 "def.y"
    { if (defrScanchainCbk)
	      defrScanchain.defiScanchain::setStop((yyvsp[(4) - (5)].string), (yyvsp[(5) - (5)].string));
          ;}
    break;

  case 541:

/* Line 1455 of yacc.c  */
#line 2172 "def.y"
    { dumb_mode = 10; no_num = 10; ;}
    break;

  case 542:

/* Line 1455 of yacc.c  */
#line 2174 "def.y"
    { dumb_mode = 0;  no_num = 0; ;}
    break;

  case 543:

/* Line 1455 of yacc.c  */
#line 2175 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 544:

/* Line 1455 of yacc.c  */
#line 2177 "def.y"
    {
            if (defVersionNum < 5.5) {
              yyerror("PARTITION is a 5.5 syntax. Your def file is not defined as 5.5");
              CHKERR();
              }
            if (defrScanchainCbk)
              defrScanchain.defiScanchain::setPartition((yyvsp[(4) - (5)].string), (yyvsp[(5) - (5)].integer));
          ;}
    break;

  case 545:

/* Line 1455 of yacc.c  */
#line 2186 "def.y"
    {
	    CALLBACK(defrScanChainExtCbk, defrScanChainExtCbkType,
              History_text);
	  ;}
    break;

  case 546:

/* Line 1455 of yacc.c  */
#line 2192 "def.y"
    { ;}
    break;

  case 547:

/* Line 1455 of yacc.c  */
#line 2194 "def.y"
    {
	if (defrScanchainCbk) {
	  if (strcmp((yyvsp[(2) - (4)].string), "IN") == 0 || strcmp((yyvsp[(2) - (4)].string), "in") == 0)
	    defrScanchain.defiScanchain::setCommonIn((yyvsp[(3) - (4)].string));
	  else if (strcmp((yyvsp[(2) - (4)].string), "OUT") == 0 || strcmp((yyvsp[(2) - (4)].string), "out") == 0)
	    defrScanchain.defiScanchain::setCommonOut((yyvsp[(3) - (4)].string));
	}
      ;}
    break;

  case 548:

/* Line 1455 of yacc.c  */
#line 2203 "def.y"
    {
	if (defrScanchainCbk) {
	  if (strcmp((yyvsp[(2) - (8)].string), "IN") == 0 || strcmp((yyvsp[(2) - (8)].string), "in") == 0)
	    defrScanchain.defiScanchain::setCommonIn((yyvsp[(3) - (8)].string));
	  else if (strcmp((yyvsp[(2) - (8)].string), "OUT") == 0 || strcmp((yyvsp[(2) - (8)].string), "out") == 0)
	    defrScanchain.defiScanchain::setCommonOut((yyvsp[(3) - (8)].string));
	  if (strcmp((yyvsp[(6) - (8)].string), "IN") == 0 || strcmp((yyvsp[(6) - (8)].string), "in") == 0)
	    defrScanchain.defiScanchain::setCommonIn((yyvsp[(7) - (8)].string));
	  else if (strcmp((yyvsp[(6) - (8)].string), "OUT") == 0 || strcmp((yyvsp[(6) - (8)].string), "out") == 0)
	    defrScanchain.defiScanchain::setCommonOut((yyvsp[(7) - (8)].string));
	}
      ;}
    break;

  case 551:

/* Line 1455 of yacc.c  */
#line 2222 "def.y"
    {
	dumb_mode = 1000;
	no_num = 1000;
	if (defrScanchainCbk)
	  defrScanchain.defiScanchain::addFloatingInst((yyvsp[(1) - (1)].string));
      ;}
    break;

  case 552:

/* Line 1455 of yacc.c  */
#line 2229 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 553:

/* Line 1455 of yacc.c  */
#line 2232 "def.y"
    { ;}
    break;

  case 554:

/* Line 1455 of yacc.c  */
#line 2234 "def.y"
    {
	if (defrScanchainCbk) {
	  if (strcmp((yyvsp[(2) - (4)].string), "IN") == 0 || strcmp((yyvsp[(2) - (4)].string), "in") == 0)
	    defrScanchain.defiScanchain::addFloatingIn((yyvsp[(3) - (4)].string));
	  else if (strcmp((yyvsp[(2) - (4)].string), "OUT") == 0 || strcmp((yyvsp[(2) - (4)].string), "out") == 0)
	    defrScanchain.defiScanchain::addFloatingOut((yyvsp[(3) - (4)].string));
          else if (strcmp((yyvsp[(2) - (4)].string), "BITS") == 0 || strcmp((yyvsp[(2) - (4)].string), "bits") == 0) {
            bitsNum = atoi((yyvsp[(3) - (4)].string));
            defrScanchain.defiScanchain::setFloatingBits(bitsNum);
         }
	}
      ;}
    break;

  case 555:

/* Line 1455 of yacc.c  */
#line 2247 "def.y"
    {
	if (defrScanchainCbk) {
	  if (strcmp((yyvsp[(2) - (8)].string), "IN") == 0 || strcmp((yyvsp[(2) - (8)].string), "in") == 0)
	    defrScanchain.defiScanchain::addFloatingIn((yyvsp[(3) - (8)].string));
	  else if (strcmp((yyvsp[(2) - (8)].string), "OUT") == 0 || strcmp((yyvsp[(2) - (8)].string), "out") == 0)
	    defrScanchain.defiScanchain::addFloatingOut((yyvsp[(3) - (8)].string));
	  else if (strcmp((yyvsp[(2) - (8)].string), "BITS") == 0 || strcmp((yyvsp[(2) - (8)].string), "bits") == 0) {
            bitsNum = atoi((yyvsp[(3) - (8)].string));
	    defrScanchain.defiScanchain::setFloatingBits(bitsNum);
          }
	  if (strcmp((yyvsp[(6) - (8)].string), "IN") == 0 || strcmp((yyvsp[(6) - (8)].string), "in") == 0)
	    defrScanchain.defiScanchain::addFloatingIn((yyvsp[(7) - (8)].string));
	  else if (strcmp((yyvsp[(6) - (8)].string), "OUT") == 0 || strcmp((yyvsp[(6) - (8)].string), "out") == 0)
	    defrScanchain.defiScanchain::addFloatingOut((yyvsp[(7) - (8)].string));
	  else if (strcmp((yyvsp[(6) - (8)].string), "BITS") == 0 || strcmp((yyvsp[(6) - (8)].string), "bits") == 0) {
            bitsNum = atoi((yyvsp[(7) - (8)].string));
	    defrScanchain.defiScanchain::setFloatingBits(bitsNum);
          }
	}
      ;}
    break;

  case 556:

/* Line 1455 of yacc.c  */
#line 2268 "def.y"
    {
	if (defrScanchainCbk) {
	  if (strcmp((yyvsp[(2) - (12)].string), "IN") == 0 || strcmp((yyvsp[(2) - (12)].string), "in") == 0)
	    defrScanchain.defiScanchain::addFloatingIn((yyvsp[(3) - (12)].string));
	  else if (strcmp((yyvsp[(2) - (12)].string), "OUT") == 0 || strcmp((yyvsp[(2) - (12)].string), "out") == 0)
	    defrScanchain.defiScanchain::addFloatingOut((yyvsp[(3) - (12)].string));
	  else if (strcmp((yyvsp[(2) - (12)].string), "BITS") == 0 || strcmp((yyvsp[(2) - (12)].string), "bits") == 0) {
            bitsNum = atoi((yyvsp[(3) - (12)].string));
	    defrScanchain.defiScanchain::setFloatingBits(bitsNum);
          }
	  if (strcmp((yyvsp[(6) - (12)].string), "IN") == 0 || strcmp((yyvsp[(6) - (12)].string), "in") == 0)
	    defrScanchain.defiScanchain::addFloatingIn((yyvsp[(7) - (12)].string));
	  else if (strcmp((yyvsp[(6) - (12)].string), "OUT") == 0 || strcmp((yyvsp[(6) - (12)].string), "out") == 0)
	    defrScanchain.defiScanchain::addFloatingOut((yyvsp[(7) - (12)].string));
	  else if (strcmp((yyvsp[(6) - (12)].string), "BITS") == 0 || strcmp((yyvsp[(6) - (12)].string), "bits") == 0) {
            bitsNum = atoi((yyvsp[(7) - (12)].string));
	    defrScanchain.defiScanchain::setFloatingBits(bitsNum);
          }
	  if (strcmp((yyvsp[(10) - (12)].string), "IN") == 0 || strcmp((yyvsp[(10) - (12)].string), "in") == 0)
	    defrScanchain.defiScanchain::addFloatingIn((yyvsp[(11) - (12)].string));
	  else if (strcmp((yyvsp[(10) - (12)].string), "OUT") == 0 || strcmp((yyvsp[(10) - (12)].string), "out") == 0)
	    defrScanchain.defiScanchain::addFloatingOut((yyvsp[(11) - (12)].string));
	  else if (strcmp((yyvsp[(10) - (12)].string), "BITS") == 0 || strcmp((yyvsp[(10) - (12)].string), "bits") == 0) {
            bitsNum = atoi((yyvsp[(11) - (12)].string));
	    defrScanchain.defiScanchain::setFloatingBits(bitsNum);
          }
	}
      ;}
    break;

  case 559:

/* Line 1455 of yacc.c  */
#line 2302 "def.y"
    { dumb_mode = 1000; no_num = 1000; 
	if (defrScanchainCbk)
	  defrScanchain.defiScanchain::addOrderedInst((yyvsp[(1) - (1)].string));
      ;}
    break;

  case 560:

/* Line 1455 of yacc.c  */
#line 2307 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 561:

/* Line 1455 of yacc.c  */
#line 2310 "def.y"
    { ;}
    break;

  case 562:

/* Line 1455 of yacc.c  */
#line 2312 "def.y"
    {
	if (defrScanchainCbk) {
	  if (strcmp((yyvsp[(2) - (4)].string), "IN") == 0 || strcmp((yyvsp[(2) - (4)].string), "in") == 0)
	    defrScanchain.defiScanchain::addOrderedIn((yyvsp[(3) - (4)].string));
	  else if (strcmp((yyvsp[(2) - (4)].string), "OUT") == 0 || strcmp((yyvsp[(2) - (4)].string), "out") == 0)
	    defrScanchain.defiScanchain::addOrderedOut((yyvsp[(3) - (4)].string));
          else if (strcmp((yyvsp[(2) - (4)].string), "BITS") == 0 || strcmp((yyvsp[(2) - (4)].string), "bits") == 0) {
            bitsNum = atoi((yyvsp[(3) - (4)].string));
            defrScanchain.defiScanchain::setOrderedBits(bitsNum);
         }
	}
      ;}
    break;

  case 563:

/* Line 1455 of yacc.c  */
#line 2325 "def.y"
    {
	if (defrScanchainCbk) {
	  if (strcmp((yyvsp[(2) - (8)].string), "IN") == 0 || strcmp((yyvsp[(2) - (8)].string), "in") == 0)
	    defrScanchain.defiScanchain::addOrderedIn((yyvsp[(3) - (8)].string));
	  else if (strcmp((yyvsp[(2) - (8)].string), "OUT") == 0 || strcmp((yyvsp[(2) - (8)].string), "out") == 0)
	    defrScanchain.defiScanchain::addOrderedOut((yyvsp[(3) - (8)].string));
	  else if (strcmp((yyvsp[(2) - (8)].string), "BITS") == 0 || strcmp((yyvsp[(2) - (8)].string), "bits") == 0) {
            bitsNum = atoi((yyvsp[(3) - (8)].string));
            defrScanchain.defiScanchain::setOrderedBits(bitsNum);
          }
	  if (strcmp((yyvsp[(6) - (8)].string), "IN") == 0 || strcmp((yyvsp[(6) - (8)].string), "in") == 0)
	    defrScanchain.defiScanchain::addOrderedIn((yyvsp[(7) - (8)].string));
	  else if (strcmp((yyvsp[(6) - (8)].string), "OUT") == 0 || strcmp((yyvsp[(6) - (8)].string), "out") == 0)
	    defrScanchain.defiScanchain::addOrderedOut((yyvsp[(7) - (8)].string));
	  else if (strcmp((yyvsp[(6) - (8)].string), "BITS") == 0 || strcmp((yyvsp[(6) - (8)].string), "bits") == 0) {
            bitsNum = atoi((yyvsp[(7) - (8)].string));
            defrScanchain.defiScanchain::setOrderedBits(bitsNum);
          }
	}
      ;}
    break;

  case 564:

/* Line 1455 of yacc.c  */
#line 2346 "def.y"
    {
	if (defrScanchainCbk) {
	  if (strcmp((yyvsp[(2) - (12)].string), "IN") == 0 || strcmp((yyvsp[(2) - (12)].string), "in") == 0)
	    defrScanchain.defiScanchain::addOrderedIn((yyvsp[(3) - (12)].string));
	  else if (strcmp((yyvsp[(2) - (12)].string), "OUT") == 0 || strcmp((yyvsp[(2) - (12)].string), "out") == 0)
	    defrScanchain.defiScanchain::addOrderedOut((yyvsp[(3) - (12)].string));
	  else if (strcmp((yyvsp[(2) - (12)].string), "BITS") == 0 || strcmp((yyvsp[(2) - (12)].string), "bits") == 0) {
            bitsNum = atoi((yyvsp[(3) - (12)].string));
	    defrScanchain.defiScanchain::setOrderedBits(bitsNum);
          }
	  if (strcmp((yyvsp[(6) - (12)].string), "IN") == 0 || strcmp((yyvsp[(6) - (12)].string), "in") == 0)
	    defrScanchain.defiScanchain::addOrderedIn((yyvsp[(7) - (12)].string));
	  else if (strcmp((yyvsp[(6) - (12)].string), "OUT") == 0 || strcmp((yyvsp[(6) - (12)].string), "out") == 0)
	    defrScanchain.defiScanchain::addOrderedOut((yyvsp[(7) - (12)].string));
	  else if (strcmp((yyvsp[(6) - (12)].string), "BITS") == 0 || strcmp((yyvsp[(6) - (12)].string), "bits") == 0) {
            bitsNum = atoi((yyvsp[(7) - (12)].string));
	    defrScanchain.defiScanchain::setOrderedBits(bitsNum);
          }
	  if (strcmp((yyvsp[(10) - (12)].string), "IN") == 0 || strcmp((yyvsp[(10) - (12)].string), "in") == 0)
	    defrScanchain.defiScanchain::addOrderedIn((yyvsp[(11) - (12)].string));
	  else if (strcmp((yyvsp[(10) - (12)].string), "OUT") == 0 || strcmp((yyvsp[(10) - (12)].string), "out") == 0)
	    defrScanchain.defiScanchain::addOrderedOut((yyvsp[(11) - (12)].string));
	  else if (strcmp((yyvsp[(10) - (12)].string), "BITS") == 0 || strcmp((yyvsp[(10) - (12)].string), "bits") == 0) {
            bitsNum = atoi((yyvsp[(11) - (12)].string));
	    defrScanchain.defiScanchain::setOrderedBits(bitsNum);
          }
	}
      ;}
    break;

  case 565:

/* Line 1455 of yacc.c  */
#line 2376 "def.y"
    { (yyval.integer) = -1; ;}
    break;

  case 566:

/* Line 1455 of yacc.c  */
#line 2378 "def.y"
    { (yyval.integer) = ROUND((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 567:

/* Line 1455 of yacc.c  */
#line 2381 "def.y"
    { 
          CALLBACK(defrScanchainsEndCbk, defrScanchainsEndCbkType, 0);
          bit_is_keyword = FALSE;
          dumb_mode = 0; no_num = 0;
        ;}
    break;

  case 569:

/* Line 1455 of yacc.c  */
#line 2392 "def.y"
    {
          if (defVersionNum < 5.4) {
             CALLBACK(defrIOTimingsStartCbk, defrIOTimingsStartCbkType,
                      ROUND((yyvsp[(2) - (3)].dval)));
          } else {
             yydefwarning("IOTIMINGS is obsolete in 5.4. It will be ignored.");
          }
        ;}
    break;

  case 571:

/* Line 1455 of yacc.c  */
#line 2403 "def.y"
    {;}
    break;

  case 572:

/* Line 1455 of yacc.c  */
#line 2406 "def.y"
    { 
              if (defVersionNum < 5.4)
                CALLBACK(defrIOTimingCbk, defrIOTimingCbkType, &defrIOTiming);
            ;}
    break;

  case 573:

/* Line 1455 of yacc.c  */
#line 2411 "def.y"
    {dumb_mode = 2; no_num = 2; ;}
    break;

  case 574:

/* Line 1455 of yacc.c  */
#line 2412 "def.y"
    {
              if (defrIOTimingCbk)
                defrIOTiming.defiIOTiming::setName((yyvsp[(4) - (6)].string), (yyvsp[(5) - (6)].string));
            ;}
    break;

  case 577:

/* Line 1455 of yacc.c  */
#line 2423 "def.y"
    {
              if (defrIOTimingCbk) 
                defrIOTiming.defiIOTiming::setVariable((yyvsp[(2) - (5)].string), (yyvsp[(4) - (5)].dval), (yyvsp[(5) - (5)].dval));
            ;}
    break;

  case 578:

/* Line 1455 of yacc.c  */
#line 2428 "def.y"
    {
              if (defrIOTimingCbk) 
                defrIOTiming.defiIOTiming::setSlewRate((yyvsp[(2) - (5)].string), (yyvsp[(4) - (5)].dval), (yyvsp[(5) - (5)].dval));
            ;}
    break;

  case 579:

/* Line 1455 of yacc.c  */
#line 2433 "def.y"
    {
              if (defrIOTimingCbk) 
                defrIOTiming.defiIOTiming::setCapacitance((yyvsp[(3) - (3)].dval));
            ;}
    break;

  case 580:

/* Line 1455 of yacc.c  */
#line 2437 "def.y"
    {dumb_mode = 1; no_num = 1; ;}
    break;

  case 581:

/* Line 1455 of yacc.c  */
#line 2438 "def.y"
    {
              if (defrIOTimingCbk) 
                defrIOTiming.defiIOTiming::setDriveCell((yyvsp[(4) - (4)].string));
            ;}
    break;

  case 583:

/* Line 1455 of yacc.c  */
#line 2464 "def.y"
    {
              if (defVersionNum < 5.4)
                CALLBACK(defrIoTimingsExtCbk, defrIoTimingsExtCbkType,
                History_text);
            ;}
    break;

  case 584:

/* Line 1455 of yacc.c  */
#line 2471 "def.y"
    {dumb_mode = 1; no_num = 1; ;}
    break;

  case 585:

/* Line 1455 of yacc.c  */
#line 2472 "def.y"
    {
              if (defrIOTimingCbk) 
                defrIOTiming.defiIOTiming::setTo((yyvsp[(4) - (4)].string));
           ;}
    break;

  case 588:

/* Line 1455 of yacc.c  */
#line 2479 "def.y"
    {dumb_mode = 1; no_num = 1; ;}
    break;

  case 589:

/* Line 1455 of yacc.c  */
#line 2480 "def.y"
    {
              if (defrIOTimingCbk)
                defrIOTiming.defiIOTiming::setFrom((yyvsp[(3) - (3)].string));
           ;}
    break;

  case 591:

/* Line 1455 of yacc.c  */
#line 2487 "def.y"
    {
              if (defrIOTimingCbk)
                defrIOTiming.defiIOTiming::setParallel((yyvsp[(2) - (2)].dval));
           ;}
    break;

  case 592:

/* Line 1455 of yacc.c  */
#line 2492 "def.y"
    { (yyval.string) = (char*)"RISE"; ;}
    break;

  case 593:

/* Line 1455 of yacc.c  */
#line 2492 "def.y"
    { (yyval.string) = (char*)"FALL"; ;}
    break;

  case 594:

/* Line 1455 of yacc.c  */
#line 2495 "def.y"
    {
            if (defVersionNum < 5.4)
              CALLBACK(defrIOTimingsEndCbk, defrIOTimingsEndCbkType, 0);
          ;}
    break;

  case 595:

/* Line 1455 of yacc.c  */
#line 2501 "def.y"
    { CALLBACK(defrFPCEndCbk, defrFPCEndCbkType, 0); ;}
    break;

  case 596:

/* Line 1455 of yacc.c  */
#line 2504 "def.y"
    { CALLBACK(defrFPCStartCbk, defrFPCStartCbkType,
              ROUND((yyvsp[(2) - (3)].dval))); ;}
    break;

  case 598:

/* Line 1455 of yacc.c  */
#line 2509 "def.y"
    {;}
    break;

  case 599:

/* Line 1455 of yacc.c  */
#line 2511 "def.y"
    { dumb_mode = 1; no_num = 1;  ;}
    break;

  case 600:

/* Line 1455 of yacc.c  */
#line 2512 "def.y"
    { if (defrFPCCbk) defrFPC.defiFPC::setName((yyvsp[(3) - (4)].string), (yyvsp[(4) - (4)].string)); ;}
    break;

  case 601:

/* Line 1455 of yacc.c  */
#line 2514 "def.y"
    { CALLBACK(defrFPCCbk, defrFPCCbkType, &defrFPC); ;}
    break;

  case 602:

/* Line 1455 of yacc.c  */
#line 2517 "def.y"
    { (yyval.string) = (char*)"HORIZONTAL"; ;}
    break;

  case 603:

/* Line 1455 of yacc.c  */
#line 2519 "def.y"
    { (yyval.string) = (char*)"VERTICAL"; ;}
    break;

  case 604:

/* Line 1455 of yacc.c  */
#line 2522 "def.y"
    { if (defrFPCCbk) defrFPC.defiFPC::setAlign(); ;}
    break;

  case 605:

/* Line 1455 of yacc.c  */
#line 2524 "def.y"
    { if (defrFPCCbk) defrFPC.defiFPC::setMax((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 606:

/* Line 1455 of yacc.c  */
#line 2526 "def.y"
    { if (defrFPCCbk) defrFPC.defiFPC::setMin((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 607:

/* Line 1455 of yacc.c  */
#line 2528 "def.y"
    { if (defrFPCCbk) defrFPC.defiFPC::setEqual((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 610:

/* Line 1455 of yacc.c  */
#line 2535 "def.y"
    { if (defrFPCCbk) defrFPC.defiFPC::setDoingBottomLeft(); ;}
    break;

  case 612:

/* Line 1455 of yacc.c  */
#line 2538 "def.y"
    { if (defrFPCCbk) defrFPC.defiFPC::setDoingTopRight(); ;}
    break;

  case 616:

/* Line 1455 of yacc.c  */
#line 2545 "def.y"
    {dumb_mode = 1; no_num = 1; ;}
    break;

  case 617:

/* Line 1455 of yacc.c  */
#line 2546 "def.y"
    { if (defrFPCCbk) defrFPC.defiFPC::addRow((yyvsp[(4) - (5)].string)); ;}
    break;

  case 618:

/* Line 1455 of yacc.c  */
#line 2547 "def.y"
    {dumb_mode = 1; no_num = 1; ;}
    break;

  case 619:

/* Line 1455 of yacc.c  */
#line 2548 "def.y"
    { if (defrFPCCbk) defrFPC.defiFPC::addComps((yyvsp[(4) - (5)].string)); ;}
    break;

  case 621:

/* Line 1455 of yacc.c  */
#line 2554 "def.y"
    { CALLBACK(defrTimingDisablesStartCbk, defrTimingDisablesStartCbkType,
              ROUND((yyvsp[(2) - (3)].dval))); ;}
    break;

  case 623:

/* Line 1455 of yacc.c  */
#line 2559 "def.y"
    {;}
    break;

  case 624:

/* Line 1455 of yacc.c  */
#line 2561 "def.y"
    { dumb_mode = 2; no_num = 2;  ;}
    break;

  case 625:

/* Line 1455 of yacc.c  */
#line 2562 "def.y"
    { dumb_mode = 2; no_num = 2;  ;}
    break;

  case 626:

/* Line 1455 of yacc.c  */
#line 2563 "def.y"
    {
              if (defrTimingDisableCbk)
                defrTimingDisable.defiTimingDisable::setFromTo((yyvsp[(4) - (10)].string), (yyvsp[(5) - (10)].string), (yyvsp[(8) - (10)].string), (yyvsp[(9) - (10)].string));
	      CALLBACK(defrTimingDisableCbk, defrTimingDisableCbkType,
                &defrTimingDisable);
	    ;}
    break;

  case 627:

/* Line 1455 of yacc.c  */
#line 2569 "def.y"
    {dumb_mode = 2; no_num = 2; ;}
    break;

  case 628:

/* Line 1455 of yacc.c  */
#line 2570 "def.y"
    {
              if (defrTimingDisableCbk)
                defrTimingDisable.defiTimingDisable::setThru((yyvsp[(4) - (6)].string), (yyvsp[(5) - (6)].string));
	      CALLBACK(defrTimingDisableCbk, defrTimingDisableCbkType,
                &defrTimingDisable);
            ;}
    break;

  case 629:

/* Line 1455 of yacc.c  */
#line 2576 "def.y"
    {dumb_mode = 1; no_num = 1;;}
    break;

  case 630:

/* Line 1455 of yacc.c  */
#line 2577 "def.y"
    {
              if (defrTimingDisableCbk)
                defrTimingDisable.defiTimingDisable::setMacro((yyvsp[(4) - (6)].string));
	      CALLBACK(defrTimingDisableCbk, defrTimingDisableCbkType,
                &defrTimingDisable);
            ;}
    break;

  case 631:

/* Line 1455 of yacc.c  */
#line 2584 "def.y"
    { if (defrTimingDisableCbk)
                defrTimingDisable.defiTimingDisable::setReentrantPathsFlag();
	    ;}
    break;

  case 632:

/* Line 1455 of yacc.c  */
#line 2589 "def.y"
    {dumb_mode = 1; no_num = 1;;}
    break;

  case 633:

/* Line 1455 of yacc.c  */
#line 2590 "def.y"
    {dumb_mode=1; no_num = 1;;}
    break;

  case 634:

/* Line 1455 of yacc.c  */
#line 2591 "def.y"
    {
              if (defrTimingDisableCbk)
                defrTimingDisable.defiTimingDisable::setMacroFromTo((yyvsp[(3) - (6)].string),(yyvsp[(6) - (6)].string));
            ;}
    break;

  case 635:

/* Line 1455 of yacc.c  */
#line 2595 "def.y"
    {dumb_mode=1; no_num = 1;;}
    break;

  case 636:

/* Line 1455 of yacc.c  */
#line 2596 "def.y"
    {
              if (defrTimingDisableCbk)
                defrTimingDisable.defiTimingDisable::setMacroThru((yyvsp[(3) - (3)].string));
            ;}
    break;

  case 637:

/* Line 1455 of yacc.c  */
#line 2602 "def.y"
    { CALLBACK(defrTimingDisablesEndCbk, defrTimingDisablesEndCbkType, 0); ;}
    break;

  case 639:

/* Line 1455 of yacc.c  */
#line 2609 "def.y"
    { CALLBACK(defrPartitionsStartCbk, defrPartitionsStartCbkType,
              ROUND((yyvsp[(2) - (3)].dval))); ;}
    break;

  case 641:

/* Line 1455 of yacc.c  */
#line 2614 "def.y"
    {;}
    break;

  case 642:

/* Line 1455 of yacc.c  */
#line 2617 "def.y"
    { CALLBACK(defrPartitionCbk, defrPartitionCbkType,
              &defrPartition); ;}
    break;

  case 643:

/* Line 1455 of yacc.c  */
#line 2620 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 644:

/* Line 1455 of yacc.c  */
#line 2621 "def.y"
    {
            if (defrPartitionCbk)
              defrPartition.defiPartition::setName((yyvsp[(3) - (4)].string));
            ;}
    break;

  case 646:

/* Line 1455 of yacc.c  */
#line 2628 "def.y"
    {
              if (defrPartitionCbk)
                defrPartition.defiPartition::addTurnOff((yyvsp[(2) - (3)].string), (yyvsp[(3) - (3)].string));
            ;}
    break;

  case 647:

/* Line 1455 of yacc.c  */
#line 2634 "def.y"
    { (yyval.string) = (char*)" "; ;}
    break;

  case 648:

/* Line 1455 of yacc.c  */
#line 2636 "def.y"
    { (yyval.string) = (char*)"R"; ;}
    break;

  case 649:

/* Line 1455 of yacc.c  */
#line 2638 "def.y"
    { (yyval.string) = (char*)"F"; ;}
    break;

  case 650:

/* Line 1455 of yacc.c  */
#line 2641 "def.y"
    { (yyval.string) = (char*)" "; ;}
    break;

  case 651:

/* Line 1455 of yacc.c  */
#line 2643 "def.y"
    { (yyval.string) = (char*)"R"; ;}
    break;

  case 652:

/* Line 1455 of yacc.c  */
#line 2645 "def.y"
    { (yyval.string) = (char*)"F"; ;}
    break;

  case 655:

/* Line 1455 of yacc.c  */
#line 2651 "def.y"
    {dumb_mode=2; no_num = 2;;}
    break;

  case 656:

/* Line 1455 of yacc.c  */
#line 2653 "def.y"
    {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setFromClockPin((yyvsp[(4) - (7)].string), (yyvsp[(5) - (7)].string));
            ;}
    break;

  case 657:

/* Line 1455 of yacc.c  */
#line 2657 "def.y"
    {dumb_mode=2; no_num = 2; ;}
    break;

  case 658:

/* Line 1455 of yacc.c  */
#line 2659 "def.y"
    {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setFromCompPin((yyvsp[(4) - (6)].string), (yyvsp[(5) - (6)].string));
            ;}
    break;

  case 659:

/* Line 1455 of yacc.c  */
#line 2663 "def.y"
    {dumb_mode=1; no_num = 1; ;}
    break;

  case 660:

/* Line 1455 of yacc.c  */
#line 2665 "def.y"
    {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setFromIOPin((yyvsp[(4) - (5)].string));
            ;}
    break;

  case 661:

/* Line 1455 of yacc.c  */
#line 2669 "def.y"
    {dumb_mode=2; no_num = 2; ;}
    break;

  case 662:

/* Line 1455 of yacc.c  */
#line 2671 "def.y"
    {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setToClockPin((yyvsp[(4) - (7)].string), (yyvsp[(5) - (7)].string));
            ;}
    break;

  case 663:

/* Line 1455 of yacc.c  */
#line 2675 "def.y"
    {dumb_mode=2; no_num = 2; ;}
    break;

  case 664:

/* Line 1455 of yacc.c  */
#line 2677 "def.y"
    {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setToCompPin((yyvsp[(4) - (6)].string), (yyvsp[(5) - (6)].string));
            ;}
    break;

  case 665:

/* Line 1455 of yacc.c  */
#line 2681 "def.y"
    {dumb_mode=1; no_num = 2; ;}
    break;

  case 666:

/* Line 1455 of yacc.c  */
#line 2682 "def.y"
    {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setToIOPin((yyvsp[(4) - (5)].string));
            ;}
    break;

  case 667:

/* Line 1455 of yacc.c  */
#line 2687 "def.y"
    { CALLBACK(defrPartitionsExtCbk, defrPartitionsExtCbkType,
              History_text); ;}
    break;

  case 668:

/* Line 1455 of yacc.c  */
#line 2691 "def.y"
    { dumb_mode = 1000000000; no_num = 10000000; ;}
    break;

  case 669:

/* Line 1455 of yacc.c  */
#line 2692 "def.y"
    { dumb_mode = 0; no_num = 0; ;}
    break;

  case 671:

/* Line 1455 of yacc.c  */
#line 2696 "def.y"
    {;}
    break;

  case 672:

/* Line 1455 of yacc.c  */
#line 2699 "def.y"
    {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setMin((yyvsp[(2) - (3)].dval), (yyvsp[(3) - (3)].dval));
            ;}
    break;

  case 673:

/* Line 1455 of yacc.c  */
#line 2704 "def.y"
    {
              if (defrPartitionCbk)
                defrPartition.defiPartition::setMax((yyvsp[(2) - (3)].dval), (yyvsp[(3) - (3)].dval));
            ;}
    break;

  case 675:

/* Line 1455 of yacc.c  */
#line 2711 "def.y"
    { if (defrPartitionCbk) defrPartition.defiPartition::addPin((yyvsp[(2) - (2)].string)); ;}
    break;

  case 678:

/* Line 1455 of yacc.c  */
#line 2717 "def.y"
    { if (defrPartitionCbk) defrPartition.defiPartition::addRiseMin((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 679:

/* Line 1455 of yacc.c  */
#line 2719 "def.y"
    { if (defrPartitionCbk) defrPartition.defiPartition::addFallMin((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 680:

/* Line 1455 of yacc.c  */
#line 2721 "def.y"
    { if (defrPartitionCbk) defrPartition.defiPartition::addRiseMax((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 681:

/* Line 1455 of yacc.c  */
#line 2723 "def.y"
    { if (defrPartitionCbk) defrPartition.defiPartition::addFallMax((yyvsp[(2) - (2)].dval)); ;}
    break;

  case 684:

/* Line 1455 of yacc.c  */
#line 2731 "def.y"
    { if (defrPartitionCbk)
	    defrPartition.defiPartition::addRiseMinRange((yyvsp[(2) - (3)].dval), (yyvsp[(3) - (3)].dval)); ;}
    break;

  case 685:

/* Line 1455 of yacc.c  */
#line 2734 "def.y"
    { if (defrPartitionCbk)
	    defrPartition.defiPartition::addFallMinRange((yyvsp[(2) - (3)].dval), (yyvsp[(3) - (3)].dval)); ;}
    break;

  case 686:

/* Line 1455 of yacc.c  */
#line 2737 "def.y"
    { if (defrPartitionCbk)
	    defrPartition.defiPartition::addRiseMaxRange((yyvsp[(2) - (3)].dval), (yyvsp[(3) - (3)].dval)); ;}
    break;

  case 687:

/* Line 1455 of yacc.c  */
#line 2740 "def.y"
    { if (defrPartitionCbk)
	    defrPartition.defiPartition::addFallMaxRange((yyvsp[(2) - (3)].dval), (yyvsp[(3) - (3)].dval)); ;}
    break;

  case 688:

/* Line 1455 of yacc.c  */
#line 2744 "def.y"
    { CALLBACK(defrPartitionsEndCbk, defrPartitionsEndCbkType, 0); ;}
    break;

  case 690:

/* Line 1455 of yacc.c  */
#line 2749 "def.y"
    {;}
    break;

  case 691:

/* Line 1455 of yacc.c  */
#line 2751 "def.y"
    {dumb_mode=2; no_num = 2; ;}
    break;

  case 692:

/* Line 1455 of yacc.c  */
#line 2753 "def.y"
    {
            /* note that the first T_STRING could be the keyword VPIN */
	    if (defrNetCbk) {
	      defrSubnet->defiSubnet::addPin((yyvsp[(3) - (6)].string), (yyvsp[(4) - (6)].string), (yyvsp[(5) - (6)].integer));
	    }
	  ;}
    break;

  case 693:

/* Line 1455 of yacc.c  */
#line 2761 "def.y"
    { (yyval.integer) = 0; ;}
    break;

  case 694:

/* Line 1455 of yacc.c  */
#line 2763 "def.y"
    { (yyval.integer) = 1; ;}
    break;

  case 697:

/* Line 1455 of yacc.c  */
#line 2769 "def.y"
    {  
            if (defrNetCbk) defrSubnet->defiSubnet::addWire((yyvsp[(1) - (1)].string));
            ;}
    break;

  case 698:

/* Line 1455 of yacc.c  */
#line 2773 "def.y"
    {  
            by_is_keyword = FALSE;
            do_is_keyword = FALSE;
            new_is_keyword = FALSE;
            step_is_keyword = FALSE;
            ;}
    break;

  case 699:

/* Line 1455 of yacc.c  */
#line 2779 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 700:

/* Line 1455 of yacc.c  */
#line 2780 "def.y"
    { if (defrNetCbk) defrSubnet->defiSubnet::setNonDefault((yyvsp[(3) - (3)].string)); ;}
    break;

  case 701:

/* Line 1455 of yacc.c  */
#line 2783 "def.y"
    { (yyval.string) = (char*)"FIXED"; ;}
    break;

  case 702:

/* Line 1455 of yacc.c  */
#line 2785 "def.y"
    { (yyval.string) = (char*)"COVER"; ;}
    break;

  case 703:

/* Line 1455 of yacc.c  */
#line 2787 "def.y"
    { (yyval.string) = (char*)"ROUTED"; ;}
    break;

  case 704:

/* Line 1455 of yacc.c  */
#line 2789 "def.y"
    { (yyval.string) = (char*)"NOSHIELD"; ;}
    break;

  case 706:

/* Line 1455 of yacc.c  */
#line 2794 "def.y"
    { CALLBACK(defrPinPropStartCbk, defrPinPropStartCbkType, ROUND((yyvsp[(2) - (3)].dval))); ;}
    break;

  case 707:

/* Line 1455 of yacc.c  */
#line 2798 "def.y"
    { ;}
    break;

  case 708:

/* Line 1455 of yacc.c  */
#line 2800 "def.y"
    { ;}
    break;

  case 709:

/* Line 1455 of yacc.c  */
#line 2803 "def.y"
    { CALLBACK(defrPinPropEndCbk, defrPinPropEndCbkType, 0); ;}
    break;

  case 712:

/* Line 1455 of yacc.c  */
#line 2809 "def.y"
    { dumb_mode = 2; no_num = 2; ;}
    break;

  case 713:

/* Line 1455 of yacc.c  */
#line 2810 "def.y"
    { if (defrPinPropCbk) defrPinProp.defiPinProp::setName((yyvsp[(3) - (4)].string), (yyvsp[(4) - (4)].string)); ;}
    break;

  case 714:

/* Line 1455 of yacc.c  */
#line 2812 "def.y"
    { CALLBACK(defrPinPropCbk, defrPinPropCbkType, &defrPinProp);
      // reset the property number
      defrPinProp.defiPinProp::clear();
    ;}
    break;

  case 717:

/* Line 1455 of yacc.c  */
#line 2820 "def.y"
    { dumb_mode = 10000000; ;}
    break;

  case 718:

/* Line 1455 of yacc.c  */
#line 2822 "def.y"
    { dumb_mode = 0; ;}
    break;

  case 721:

/* Line 1455 of yacc.c  */
#line 2829 "def.y"
    {
      if (defrPinPropCbk) {
         char propTp;
         char* str = ringCopy("                       ");
         propTp = defrCompPinProp.defiPropType::propType((yyvsp[(1) - (2)].string));
         /* For backword compatibility, also set the string value */
         /* We will use a temporary string to store the number.
         * The string space is borrowed from the ring buffer
         * in the lexer. */
         sprintf(str, "%g", (yyvsp[(2) - (2)].dval));
         defrPinProp.defiPinProp::addNumProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].dval), str, propTp);
      }
    ;}
    break;

  case 722:

/* Line 1455 of yacc.c  */
#line 2843 "def.y"
    {
      if (defrPinPropCbk) {
         char propTp;
         propTp = defrCompPinProp.defiPropType::propType((yyvsp[(1) - (2)].string));
         defrPinProp.defiPinProp::addProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
      }
    ;}
    break;

  case 723:

/* Line 1455 of yacc.c  */
#line 2851 "def.y"
    {
      if (defrPinPropCbk) {
         char propTp;
         propTp = defrCompPinProp.defiPropType::propType((yyvsp[(1) - (2)].string));
         defrPinProp.defiPinProp::addProperty((yyvsp[(1) - (2)].string), (yyvsp[(2) - (2)].string), propTp);
      }
    ;}
    break;

  case 725:

/* Line 1455 of yacc.c  */
#line 2862 "def.y"
    { CALLBACK(defrBlockageStartCbk, defrBlockageStartCbkType, ROUND((yyvsp[(2) - (3)].dval))); ;}
    break;

  case 726:

/* Line 1455 of yacc.c  */
#line 2865 "def.y"
    { CALLBACK(defrBlockageEndCbk, defrBlockageEndCbkType, 0); ;}
    break;

  case 729:

/* Line 1455 of yacc.c  */
#line 2872 "def.y"
    {
      CALLBACK(defrBlockageCbk, defrBlockageCbkType, &defrBlockage);
      defrBlockage.defiBlockage::clear();
    ;}
    break;

  case 730:

/* Line 1455 of yacc.c  */
#line 2877 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 731:

/* Line 1455 of yacc.c  */
#line 2878 "def.y"
    {
      if (defrBlockage.defiBlockage::hasPlacement() != 0) {
        yyerror("BLOCKAGE has both LAYER & PLACEMENT defined.");
        CHKERR();
      }
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setLayer((yyvsp[(4) - (4)].string));
    ;}
    break;

  case 733:

/* Line 1455 of yacc.c  */
#line 2889 "def.y"
    {
      if (defrBlockage.defiBlockage::hasLayer() != 0) {
        yyerror("BLOCKAGE has both LAYER & PLACEMENT defined.");
        CHKERR();
      }
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setPlacement();
    ;}
    break;

  case 736:

/* Line 1455 of yacc.c  */
#line 2901 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 737:

/* Line 1455 of yacc.c  */
#line 2902 "def.y"
    {
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setComponent((yyvsp[(4) - (4)].string));
    ;}
    break;

  case 738:

/* Line 1455 of yacc.c  */
#line 2908 "def.y"
    {
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setSlots();
    ;}
    break;

  case 739:

/* Line 1455 of yacc.c  */
#line 2913 "def.y"
    {
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setFills();
    ;}
    break;

  case 740:

/* Line 1455 of yacc.c  */
#line 2918 "def.y"
    {
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setPushdown();
    ;}
    break;

  case 742:

/* Line 1455 of yacc.c  */
#line 2926 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 743:

/* Line 1455 of yacc.c  */
#line 2927 "def.y"
    {
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setComponent((yyvsp[(4) - (4)].string));
    ;}
    break;

  case 744:

/* Line 1455 of yacc.c  */
#line 2932 "def.y"
    {
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::setPushdown();
    ;}
    break;

  case 747:

/* Line 1455 of yacc.c  */
#line 2942 "def.y"
    {
      if (defrBlockageCbk)
        defrBlockage.defiBlockage::addRect((yyvsp[(2) - (3)].pt).x, (yyvsp[(2) - (3)].pt).y, (yyvsp[(3) - (3)].pt).x, (yyvsp[(3) - (3)].pt).y);
    ;}
    break;

  case 749:

/* Line 1455 of yacc.c  */
#line 2951 "def.y"
    { CALLBACK(defrSlotStartCbk, defrSlotStartCbkType, ROUND((yyvsp[(2) - (3)].dval))); ;}
    break;

  case 750:

/* Line 1455 of yacc.c  */
#line 2954 "def.y"
    { CALLBACK(defrSlotEndCbk, defrSlotEndCbkType, 0); ;}
    break;

  case 753:

/* Line 1455 of yacc.c  */
#line 2961 "def.y"
    {
      CALLBACK(defrSlotCbk, defrSlotCbkType, &defrSlot);
      defrSlot.defiSlot::clear();
    ;}
    break;

  case 754:

/* Line 1455 of yacc.c  */
#line 2966 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 755:

/* Line 1455 of yacc.c  */
#line 2967 "def.y"
    {
      if (defrSlotCbk)
        defrSlot.defiSlot::setLayer((yyvsp[(4) - (4)].string));
    ;}
    break;

  case 759:

/* Line 1455 of yacc.c  */
#line 2977 "def.y"
    {
      if (defrSlotCbk)
        defrSlot.defiSlot::addRect((yyvsp[(2) - (3)].pt).x, (yyvsp[(2) - (3)].pt).y, (yyvsp[(3) - (3)].pt).x, (yyvsp[(3) - (3)].pt).y);
    ;}
    break;

  case 761:

/* Line 1455 of yacc.c  */
#line 2986 "def.y"
    { CALLBACK(defrFillStartCbk, defrFillStartCbkType, ROUND((yyvsp[(2) - (3)].dval))); ;}
    break;

  case 762:

/* Line 1455 of yacc.c  */
#line 2989 "def.y"
    { CALLBACK(defrFillEndCbk, defrFillEndCbkType, 0); ;}
    break;

  case 765:

/* Line 1455 of yacc.c  */
#line 2996 "def.y"
    {
      CALLBACK(defrFillCbk, defrFillCbkType, &defrFill);
      defrFill.defiFill::clear();
    ;}
    break;

  case 766:

/* Line 1455 of yacc.c  */
#line 3001 "def.y"
    { dumb_mode = 1; no_num = 1; ;}
    break;

  case 767:

/* Line 1455 of yacc.c  */
#line 3002 "def.y"
    {
      if (defrFillCbk)
        defrFill.defiFill::setLayer((yyvsp[(4) - (4)].string));
    ;}
    break;

  case 771:

/* Line 1455 of yacc.c  */
#line 3012 "def.y"
    {
      if (defrFillCbk)
        defrFill.defiFill::addRect((yyvsp[(2) - (3)].pt).x, (yyvsp[(2) - (3)].pt).y, (yyvsp[(3) - (3)].pt).x, (yyvsp[(3) - (3)].pt).y);
    ;}
    break;



/* Line 1455 of yacc.c  */
#line 8346 "def.tab.c"
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
#line 3017 "def.y"



