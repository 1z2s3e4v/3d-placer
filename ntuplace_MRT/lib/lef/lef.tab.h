
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

/* Line 1676 of yacc.c  */
#line 190 "lef.y"

	double dval ;
	int       integer ;
	char *    string ;
	lefPOINT  pt;



/* Line 1676 of yacc.c  */
#line 434 "lef.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE lefyylval;

