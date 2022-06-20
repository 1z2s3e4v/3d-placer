
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
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

/* Line 1676 of yacc.c  */
#line 135 "def.y"

        double dval ;
        int    integer ;
        char * string ;
        int    keyword ;  /* really just a nop */
        struct defpoint pt;
        defTOKEN *tk;



/* Line 1676 of yacc.c  */
#line 296 "def.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE defyylval;


