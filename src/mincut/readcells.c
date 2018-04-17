/*
 *   Copyright (C) 1988-1991 Yale University
 *
 *   This work is distributed in the hope that it will be useful; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU General Public License as published by the Free Software Foundation;
 *   either version 2 of the License,
 *   or any later version, on the following conditions:
 *
 *   (a) YALE MAKES NO, AND EXPRESSLY DISCLAIMS
 *   ALL, REPRESENTATIONS OR WARRANTIES THAT THE MANUFACTURE, USE, PRACTICE,
 *   SALE OR
 *   OTHER DISPOSAL OF THE SOFTWARE DOES NOT OR WILL NOT INFRINGE UPON ANY
 *   PATENT OR
 *   OTHER RIGHTS NOT VESTED IN YALE.
 *
 *   (b) YALE MAKES NO, AND EXPRESSLY DISCLAIMS ALL, REPRESENTATIONS AND
 *   WARRANTIES
 *   WHATSOEVER WITH RESPECT TO THE SOFTWARE, EITHER EXPRESS OR IMPLIED,
 *   INCLUDING,
 *   BUT NOT LIMITED TO, WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A
 *   PARTICULAR
 *   PURPOSE.
 *
 *   (c) LICENSEE SHALL MAKE NO STATEMENTS, REPRESENTATION OR WARRANTIES
 *   WHATSOEVER TO
 *   ANY THIRD PARTIES THAT ARE INCONSISTENT WITH THE DISCLAIMERS BY YALE IN
 *   ARTICLE
 *   (a) AND (b) above.
 *
 *   (d) IN NO EVENT SHALL YALE, OR ITS TRUSTEES, DIRECTORS, OFFICERS,
 *   EMPLOYEES AND
 *   AFFILIATES BE LIABLE FOR DAMAGES OF ANY KIND, INCLUDING ECONOMIC DAMAGE OR
 *   INJURY TO PROPERTY AND LOST PROFITS, REGARDLESS OF WHETHER YALE SHALL BE
 *   ADVISED, SHALL HAVE OTHER REASON TO KNOW, OR IN FACT SHALL KNOW OF THE
 *   POSSIBILITY OF THE FOREGOING.
 *
 */

#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.8 (Berkeley) 01/20/90";
#endif
#define YYBYACC 1
/* ----------------------------------------------------------------- 
FILE:	    readcells.c <- readcells.y <- readcells.l
DESCRIPTION:This file contains the grammar (BNF) for the TimberWolfMC
	    input file parser. The rules for lex are in readcells.l.
	    The grammar for yacc is in readcells.y.
	    The output of yacc (y.tab.c) is renamed to readcells.c
CONTENTS:   
	    readcells( fp )
		FILE *fp ;
	    yyerror(s)
		char    *s;
	    yywrap()
	
	    Note:readcells.c will also include the yacc parser.
DATE:	    Aug  7, 1988 
REVISIONS:  Oct 27, 1988 - modified fixed and group cell code.
		Now accept designs without pads.
	    Jan 29, 1989 - changed to msgG and added \n.
	    Mar  6, 1989 - removed redundant string from padgroupname
	    Mar  7, 1989 - now able to parse instance lists but
		don't process them.
	    Mar 12, 1989 - changed group cell and supercell reference
		from integer to string to be easier for user.
	    Mar 13, 1989 - added string to instance to tell them apart.
	    Apr 24, 1989 - added standard cells to yacc.
	    May  7, 1989 - corrected padgroup.
	    May 11, 1989 - must have layer info for this flow.
	    Jun 19, 1989 - added pin groups to syntax checker.
	    Mar 39, 1990 - added new MC softpin syntax.
	    Fri Jan 25 17:59:55 PST 1991 - added missing SC options.
	    Thu Apr 18 01:14:09 EDT 1991 - added initial orient feature
		and reversed corners field. Also added more error
		checking and now context free language.
	    Wed Jun 12 13:41:41 CDT 1991 - added check for pins
		outside boundary and fixed sidespace syntax.
	    Wed Jul  3 13:19:19 CDT 1991 - now TimberWolf handles
		more than one unequiv pair.
	    Wed Jul 24 21:07:46 CDT 1991 - added analog
		input to syntax.
	    Tue Aug 13 12:57:00 CDT 1991 - now allow stdcells
		to have no pins and fixed mirror keyword prob.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) readcells.y version 1.4 5/22/92" ;
#endif

#include <string.h>
#include <yalecad/base.h>
#include <yalecad/message.h>
#include <yalecad/debug.h>
#include <yalecad/string.h>
#include "globals.h"

#undef REJECT 
#ifdef DEBUG

#define YYDEBUG  1          /* condition compile for yacc debug */
/* #define LEXDEBUG 1  */   /* conditional compile for lex debug */
#ifdef LEXDEBUG
/* two bugs in Yale version of lex */
#define allprint(x)    fprintf( stdout, "%c\n", x )
#define sprint(x)      fprintf( stdout, "%s\n", x )
#endif

#endif /* DEBUG */

static int  line_countS ;
static char bufferS[LRECL] ;

typedef union {
    INT ival ;
    char *string ;
    double fval ;
} YYSTYPE;
#define INTEGER 257
#define STRING 258
#define FLOAT 259
#define ADDEQUIV 260
#define APPROXFIXED 261
#define ASPLB 262
#define ASPUB 263
#define AT 264
#define BLOCK 265
#define BOTTOM 266
#define CELL 267
#define CELLGROUP 268
#define CLASS 269
#define CONNECT 270
#define CORNERS 271
#define EQUIV 272
#define FIXED 273
#define FROM 274
#define HARDCELL 275
#define INITIALLY 276
#define INSTANCE 277
#define LAYER 278
#define LEFT 279
#define NAME 280
#define NEIGHBORHOOD 281
#define NOMIRROR 282
#define NONFIXED 283
#define NOPERMUTE 284
#define OF 285
#define ORIENT 286
#define ORIENTATIONS 287
#define PAD 288
#define PADGROUP 289
#define PERMUTE 290
#define PIN 291
#define RESTRICT 292
#define RIGHT 293
#define RIGIDFIXED 294
#define SIDE 295
#define SIDESPACE 296
#define SIGNAL 297
#define SOFTCELL 298
#define SOFTPIN 299
#define STDCELL 300
#define SUPERGROUP 301
#define TOP 302
#define PINGROUP 303
#define ENDPINGROUP 304
#define SWAPGROUP 305
#define UNEQUIV 306
#define CELLOFFSET 307
#define ECO_ADDED_CELL 308
#define LEGALBLKCLASS 309
#define CURRENT 310
#define POWER 311
#define NO_LAYER_CHANGE 312
#define KEEPOUT 313
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,    4,    4,    6,    6,    8,    8,    8,    5,
    5,   12,   12,   13,   13,    7,    7,   11,   11,   21,
   21,   22,   22,   22,   22,   22,   22,    9,    9,   30,
   30,   32,   32,   10,   10,   40,   40,   41,   41,   41,
   33,   14,   14,   15,   16,   16,   29,   39,   17,   54,
   54,   31,   31,   56,   18,   18,   25,   23,   60,   60,
   24,   26,   61,   62,   62,   63,   28,   57,   57,   58,
   58,   58,   58,   59,   59,   27,   19,    2,    3,   45,
   49,   49,   51,   53,   34,   64,   64,   36,   37,   37,
   65,   65,   46,   46,   42,   43,   43,   66,   66,   38,
   38,   20,   20,   69,   69,   71,   71,   70,   70,   70,
   70,   72,   72,   72,   67,   67,   76,   68,   68,   77,
   73,   81,   82,   82,   87,   88,   88,   83,   83,   84,
   84,   85,   85,   74,   74,   89,   75,   75,   90,   79,
   79,   79,   91,   93,   92,   92,   94,   95,   96,   96,
   44,   44,   97,   97,   98,   98,   99,   99,   78,   78,
  100,  100,   48,   48,   48,   86,   80,   80,  101,   47,
   47,   50,   50,  102,  102,   52,   52,   55,   55,   35,
   35,  103,  103,  104,  105,  105,    1,    1,    1,
};
short yylen[] = {                                         2,
    2,    1,    1,    2,    1,    2,    1,    1,    1,    1,
    2,    1,    2,    1,    2,    1,    2,    4,    5,    1,
    2,    1,    1,    1,    1,    1,    1,    2,    3,    1,
    3,    5,    4,    2,    3,    1,    3,    6,    5,    4,
    2,    6,    5,    4,    4,    3,    4,    4,    3,   13,
   14,    8,   14,    1,    0,    8,    2,    2,    1,    2,
    1,    3,    1,    1,    2,    1,    2,    0,    1,    1,
    1,    1,    1,    1,    1,    1,    8,    1,    1,    4,
    3,    3,    4,    4,    3,    2,    3,    2,    4,    3,
    1,    2,    0,    2,    4,    1,    2,    1,    1,    1,
    2,    0,    1,    1,    2,    1,    2,    1,    2,    2,
    1,    1,    2,    2,    1,    2,    3,    2,    3,    6,
    5,    6,    2,    2,    2,    2,    3,    0,    2,    0,
    2,    0,    1,    1,    2,    5,    1,    2,    5,    1,
    2,    1,    2,    1,    1,    2,    3,    4,    0,    1,
    3,    4,    3,    3,    1,    2,    2,    2,    0,    3,
    1,    2,    0,    2,    3,    2,    0,    2,    1,    0,
    3,    1,    2,    2,    2,    1,    2,    1,    2,    0,
    1,    1,    2,    5,    2,    3,    1,    1,    1,
};
short yydefred[] = {                                      0,
    0,    0,    0,    0,    0,    0,    5,    7,    8,    9,
    0,    0,    0,  188,  187,  189,    0,    0,    0,    0,
    1,    0,   12,    0,    0,    0,    0,    6,   16,    0,
    0,   69,   76,    0,    0,    0,   61,    0,    0,    0,
   20,   22,   23,   24,   25,   26,   27,    0,    0,   54,
    0,    0,   30,    0,    0,    0,    0,    0,   36,   49,
    0,    0,    0,    0,    0,   13,   14,    0,    0,    0,
    0,   17,  176,    0,    0,    0,   67,   57,   59,    0,
   63,    0,    0,    0,    0,   21,   73,   70,   71,   72,
    0,    0,    0,    0,    0,    0,    0,    0,  182,    0,
    0,    0,    0,    0,    0,   47,   48,    0,    0,   15,
    0,    0,  172,    0,    0,    0,    0,    0,  177,    0,
    0,    0,  178,    0,   60,   66,    0,   64,    0,    0,
    0,   18,    0,  104,    0,  111,    0,    0,    0,    0,
    0,   41,   31,    0,    0,  183,    0,    0,    0,    0,
   37,   80,   82,   81,  174,  175,    0,    0,  173,   94,
    0,   84,   83,   88,    0,    0,   45,    0,    0,  179,
   65,    0,    0,    0,  106,    0,  105,    0,    0,    0,
    0,  134,  137,    0,    0,    0,    0,   19,    0,   86,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   44,
    0,    0,   91,    0,   78,    0,    0,    0,    0,  117,
  107,    0,    0,    0,    0,  135,  138,  123,  125,    0,
    0,    0,    0,   74,   75,    0,   87,    0,    0,  100,
    0,    0,    0,   95,    0,    0,   96,   98,   99,    0,
  169,  171,    0,    0,    0,   92,   90,    0,    0,    0,
    0,    0,    0,  129,    0,    0,  126,    0,    0,    0,
    0,  101,    0,    0,    0,    0,    0,    0,   97,    0,
    0,    0,  165,   89,    0,    0,    0,    0,    0,    0,
    0,  131,  133,  121,  127,    0,  185,    0,    0,    0,
    0,    0,    0,    0,    0,  155,    0,  144,    0,  119,
    0,    0,    0,  145,    0,   79,    0,    0,    0,  122,
  166,  136,  139,   56,  186,   52,    0,    0,  154,  153,
    0,  157,  158,  151,  156,  161,    0,    0,    0,  146,
  143,    0,    0,    0,   77,    0,    0,  152,  162,    0,
  150,  147,    0,    0,    0,    0,  120,  148,    0,    0,
    0,  168,    0,    0,    0,    0,    0,    0,   50,    0,
   53,   51,
};
short yydgoto[] = {                                       4,
  294,  206,  307,    5,   21,    6,   27,    7,    8,    9,
   10,   22,   65,   23,   67,   29,   11,   39,   84,  132,
   40,   41,   42,   43,   44,   45,   46,   47,   12,   51,
   52,   53,   94,   54,   97,  120,  167,  229,   13,   58,
   59,  104,  236,  268,   24,  115,  158,  200,   68,  112,
   30,   74,   31,   76,  124,   55,   48,   91,  226,   80,
   82,  127,  128,  141,  204,  237,  230,  239,  133,  134,
  174,  175,  231,  180,  181,  136,  240,  272,  300,  347,
  137,  186,  221,  256,  284,  280,  187,  223,  182,  183,
  301,  302,  303,  304,  305,  342,  270,  295,  296,  327,
  242,  113,   98,   99,  261,
};
short yysindex[] = {                                   -221,
  -57,  -57,  -57,    0, -184, -188,    0,    0,    0,    0,
 -158, -125, -125,    0,    0,    0,  -57, -179, -144,  -57,
    0,  -93,    0, -108,  -57,  -57, -236,    0,    0,  -57,
 -154,    0,    0,  -51,  -57,  -47,    0,  -45,  -64, -158,
    0,    0,    0,    0,    0,    0,    0, -139,  -41,    0,
  -97, -108,    0,  -96, -190, -108,  -44,  -97,    0,    0,
  -57,  -57,  -61,  -57,  -69,    0,    0,  -57,  -65,  -58,
  -56,    0,    0,  -70, -173,  -57,    0,    0,    0,  -34,
    0,  -32,  -31, -201,  -64,    0,    0,    0,    0,    0,
  -30,  -29,  -57, -108,  -97,  -49,  -37,  -96,    0,  -26,
  -24,  -97,  -25,  -37, -108,    0,    0,  -57, -251,    0,
 -235, -216,    0,  -22,  -55,  -57,  -57,  -19,    0, -205,
  -35,  -17,    0,  -57,    0,    0,  -32,    0,  -52,  -36,
  -48,    0, -201,    0, -228,    0, -168, -201,  -28,  -15,
  -12,    0,    0,  -10, -205,    0,  -23,  -21,  -14, -205,
    0,    0,    0,    0,    0,    0,  -43,  -42,    0,    0,
  -42,    0,    0,    0,  -39,    1,    0,   -2,  -13,    0,
    0,    2,  -57, -174,    0, -228,    0,  -20,  -18,   -9,
  -27,    0,    0,    7,   11,  -53,   13,    0, -153,    0,
   18,   16,  -48,   -2,   -2,   10, -101,   19,   24,    0,
  -48,    1,    0, -227,    0,   27,   -2,   20,   -7,    0,
    0,   -9,  -27,  -57,  -57,    0,    0,    0,    0,   26,
  -40,   50,   57,    0,    0,    3,    0,   59,  -48,    0,
   -9,   64,   79,    0,   -8, -122,    0,    0,    0,  -16,
    0,    0,   87,  -48, -227,    0,    0,    4,   83,   95,
  -57,   60,   60,    0,  102,   51,    0,  108,  101,  114,
  118,    0,   -9,  103,  112,  -57,  -57,   73,    0,  -57,
   93, -135,    0,    0,  132,  122,   90,   60,  143, -168,
 -168,    0,    0,    0,    0,  145,    0,  157,  132,  132,
  116,  -99,  -57, -164, -187,    0,  158,    0,  146,    0,
  155,  155,  -16,    0,  -16,    0,  180,  132,  181,    0,
    0,    0,    0,    0,    0,    0,  182,  -57,    0,    0,
 -187,    0,    0,    0,    0,    0,  186,  -57,  155,    0,
    0,  165,  171,  189,    0,  176,  175,    0,    0,  175,
    0,    0,   -2,  190,   -2,  195,    0,    0,  216,   -2,
  219,    0,  206,  225,  221,  132,  226,  132,    0,  132,
    0,    0,
};
short yyrindex[] = {                                      0,
    0,    0,    0,    0,  490,   17,    0,    0,    0,    0,
 -140,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  501,    0,    0,    0,    0,   21,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -140,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   14,    0,    0,  247,    0,    0,    0,  566,    0,    0,
    0,    0,    0,    0,  517,    0,    0,    0,  129,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -141,
    0,    0,    0,  584,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  588,    0,    0,  251,    0,    0,
    0,  602,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   15,    0,    0,  178,    0,    0,    0,    0,    0,
    0,    0,    0,   12,    0,    0, -100,    0,    0,    0,
    0,    0,  620,    0,  411,    0,    0,  584,    0,    0,
    5,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    6,    0,    0,
  456,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0, -134,    0,    0,    0,  436,
  440,    0,    0,    0,    0,   56,    0,    0,    0,    0,
    0,    0,  487,    0,    0,    0,  493,    0,    0,    0,
   37,    0,    0,  458,    0,    0,    0,    0,    0,    0,
    0, -129, -120,    0,    0,    0,    0,    0,    0,    0,
   81,    0,   31,    0,    0,    0,    0,    0,  515,    0,
  230,    0,    0,    0,    0,  542,    0,    0,    0,  211,
    0,    0,  489,   62,  458,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  106,    0,    0,    0,    0,
 -256,    0,  255,    0,    0,    0,    0,  547,    0,    0,
    0,  292,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  174,    0,    0,    0,    0,    0,
  339,  364,  273,    0,  236,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  174,    0,    0,    0,    0,    0,  131,    0,  389,    0,
    0,  310,    0,    0,    0,    0,  156,    0,    0,  193,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,
};
short yygindex[] = {                                      0,
   -1, -185, -263,    0,    0,    0,    0,  519,    0,    0,
    0,    0,    0,  504,  471,  511,    0,  509,  466,  414,
    0,  522,    0,    0,    0,    0,    0,    0,    0,  503,
  544,  469,  -38,   -6,    0,   55,   47,  367,    0,  514,
  460,    0,    0,    0,    0, -176,  464,  412,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  448,    0,  379,  350, -189,    0,    0,  459,
    0,  415,  -80, -165,  418,    0,    0, -207,    0,  256,
    0,  -77,    0,    0,    0, -170,    0,    0, -177, -145,
    0,  296,    0, -217,    0,    0,  331,  307, -272,    0,
    0,  491,    0,  506,    0,
};
#define YYTABLESIZE 921
short yytable[] = {                                      17,
   18,   19,  216,  135,   85,  163,   57,  238,  232,  233,
  212,   46,  184,   28,  170,   60,    3,   69,   63,  105,
    4,  249,  325,   70,   71,  316,  317,  247,   73,  246,
  124,   25,  153,   78,  216,  217,   43,  155,  154,  262,
   14,   15,   16,  178,  334,    1,  238,  156,  325,   57,
  176,  165,  135,    2,  262,  128,  184,  135,  114,  106,
  107,   42,  109,  105,   26,  263,  111,  217,  274,   14,
   15,   16,  119,  100,  123,  157,    3,  179,    1,   25,
  130,  166,  281,  121,  330,  216,    2,  324,  184,  130,
  101,  142,  359,  176,  361,  331,  362,  332,   57,  122,
   61,  131,  185,   20,  271,  132,  152,  310,  322,    3,
  111,  330,   26,  338,  162,  163,  130,   32,  323,   58,
   68,   87,  170,   33,  298,  224,   75,   34,   93,  210,
  160,   58,   68,   88,   58,   62,  299,   58,   55,  225,
   58,   58,   68,   89,   58,   49,   35,   50,   36,   37,
   38,  145,   58,   68,   90,  167,  112,  349,  150,  351,
   62,  113,   49,   58,  354,   58,   58,   58,  130,  112,
  114,  209,   62,  159,  113,   62,  235,  170,   62,   93,
  267,   62,   62,  114,  319,   62,   14,   15,   16,  130,
  320,  193,  167,   62,   20,   64,  197,  235,  118,   14,
   15,   16,  312,  313,   62,   77,   62,   62,   62,   79,
  159,   81,  252,  253,   83,   92,   96,  103,  108,   64,
  114,  116,  125,  117,  126,  129,  139,  140,  144,  115,
  147,  118,  148,  149,  160,  159,  157,  164,  168,  169,
  172,  190,  130,  173,  191,  189,  192,  202,  196,  278,
  194,  198,  195,  199,  116,  205,  220,  203,  208,  214,
  207,  215,  178,  218,  291,  292,   85,  219,  234,  222,
  255,  266,  159,   85,  227,  271,  241,  275,  179,   46,
   28,   28,  243,  248,  254,  250,  228,  259,   28,  251,
   85,  118,   85,   85,  163,   85,   85,  124,  124,   46,
   85,   28,  124,  170,    3,  124,  257,  124,    4,  149,
  170,   28,   46,  258,   28,  260,  337,   85,  124,  124,
  264,  124,  128,  128,   43,   43,  340,  128,  124,  124,
  128,  124,  128,  124,  124,  265,  124,  279,  140,  276,
  124,  124,  124,  128,  128,  273,  128,  130,  130,   42,
   42,  277,  130,  128,  128,  130,  128,  130,  128,  128,
  282,  128,  283,  142,  285,  286,  128,  128,  130,  130,
  287,  130,  132,  132,  288,  267,  289,  132,  130,  130,
  132,  130,  132,  130,  130,  290,  130,  297,  141,  306,
  160,  309,  130,  132,  132,  308,  132,  160,  160,  311,
  160,  314,  160,  132,  132,  160,  132,  160,  132,  132,
  108,  132,  318,  315,  326,  167,   93,   93,  160,   93,
   93,  160,  167,  167,   93,  328,  299,  167,  160,  160,
  167,  160,  167,  160,  341,  109,  333,  335,  336,  110,
  159,  159,  339,  167,  343,  344,  167,  167,  159,  345,
  159,  352,  346,  167,  167,  163,  167,   93,  167,  167,
  167,  159,  167,  350,  167,  170,  170,  167,  170,  167,
  159,  159,  353,  170,  159,  355,  159,  159,  159,  356,
  167,  357,  159,  167,  167,  159,   33,  159,  164,    2,
  167,  167,   40,  167,  358,  167,  115,  115,  159,  360,
   10,  159,  159,  159,  115,  159,  115,  159,  159,  159,
  159,  159,  159,  159,   32,  180,   11,  115,  115,  181,
  115,  116,  116,  159,   28,   66,  159,  115,  115,  116,
  115,  116,  115,  159,  159,  110,  159,   72,  159,  159,
  159,   39,  116,  116,  159,  116,   38,  159,   85,  159,
  138,  188,  116,  116,   95,  116,   56,  116,  118,  118,
  159,   86,  143,  159,  151,   34,  118,  244,  118,  102,
  159,  159,  201,  159,  171,  159,  149,  149,  161,  118,
  245,  149,  118,  102,  149,  269,  149,   29,  211,  118,
  118,  177,  118,  213,  118,  348,  329,  149,  293,  321,
  149,   35,  159,  146,    0,  140,  140,  149,  149,    0,
  149,    0,  149,  140,    0,  140,    0,    0,    0,  103,
    0,    0,    0,    0,    0,    0,  140,    0,    0,  140,
  142,  142,    0,    0,    0,    0,  140,  140,  142,  140,
  142,  140,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  142,    0,    0,  142,  141,  141,    0,    0,    0,
    0,  142,  142,  141,  142,  141,  142,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  141,  108,  108,  141,
    0,    0,    0,    0,    0,  108,  141,  141,    0,  141,
    0,  141,    0,    0,    0,    0,    0,    0,  108,    0,
    0,  108,  109,  109,    0,    0,  110,  110,  108,    0,
  109,  108,    0,  108,  110,    0,    0,    0,    0,    0,
    0,    0,    0,  109,   93,   93,  109,  110,    0,    0,
  110,    0,   93,  109,   93,    0,  109,  110,  109,    0,
  110,    0,  110,  163,  163,   93,  163,    0,   93,    0,
    0,    0,    0,   33,   33,   93,   93,    0,   93,   40,
   40,   33,    0,   33,    0,    0,    0,   40,    0,   40,
    0,    0,    0,    0,   33,    0,  164,  164,    0,  164,
   40,   32,   32,    0,   33,    0,    0,   33,    0,   32,
   40,   32,    0,   40,    0,    0,    0,    0,    0,    0,
    0,    0,   32,    0,    0,    0,    0,    0,   39,   39,
    0,    0,   32,   38,   38,   32,   39,    0,   39,    0,
    0,   38,    0,   38,    0,    0,    0,    0,    0,   39,
    0,    0,   34,   34,   38,    0,    0,    0,    0,   39,
   34,    0,   39,    0,   38,    0,    0,   38,    0,    0,
  102,  102,    0,   34,   29,   29,    0,    0,  102,    0,
    0,    0,   29,   34,    0,    0,   34,    0,   35,   35,
    0,  102,    0,    0,    0,   29,   35,    0,    0,    0,
    0,  102,    0,    0,  102,   29,  103,  103,   29,   35,
    0,    0,    0,    0,  103,    0,    0,    0,    0,   35,
    0,    0,   35,    0,    0,    0,    0,  103,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  103,    0,    0,
  103,
};
short yycheck[] = {                                       1,
    2,    3,  180,   84,    0,    0,   13,  197,  194,  195,
  176,    0,  269,    0,    0,   17,    0,   24,   20,   58,
    0,  207,  295,   25,   26,  289,  290,  204,   30,  257,
    0,  268,  284,   35,  212,  181,    0,  273,  290,  229,
  257,  258,  259,  272,  308,  267,  236,  283,  321,   56,
  131,  257,  133,  275,  244,    0,  313,  138,  286,   61,
   62,    0,   64,  102,  301,  231,   68,  213,  245,  257,
  258,  259,   74,  264,   76,  292,  298,  306,  267,  268,
    0,  287,  253,  257,  302,  263,  275,  295,  257,  291,
  281,   93,  356,  174,  358,  303,  360,  305,  105,  273,
  280,  303,  271,  288,  292,    0,  108,  278,  273,  298,
  112,  329,  301,  321,  116,  117,  291,  276,  283,  261,
  261,  261,  124,  282,  260,  279,  281,  286,    0,  304,
    0,  273,  273,  273,  276,  280,  272,  279,  279,  293,
  282,  283,  283,  283,  286,  271,  305,  273,  307,  308,
  309,   97,  294,  294,  294,    0,  291,  343,  104,  345,
  261,  291,  271,  305,  350,  307,  308,  309,  291,  304,
  291,  173,  273,    0,  304,  276,  299,    0,  279,  277,
  303,  282,  283,  304,  284,  286,  257,  258,  259,  291,
  290,  145,    0,  294,  288,  289,  150,  299,  269,  257,
  258,  259,  280,  281,  305,  257,  307,  308,  309,  257,
    0,  257,  214,  215,  279,  257,  313,  262,  280,  289,
  286,  280,  257,  280,  257,  257,  257,  257,  278,    0,
  257,  269,  257,  259,  257,    0,  292,  257,  274,  257,
  293,  257,  291,  280,  257,  274,  257,  287,  263,  251,
  274,  295,  274,  296,    0,  258,  310,  257,  257,  280,
  274,  280,  272,  257,  266,  267,  262,  257,  259,  257,
  311,  280,    0,  269,  257,  292,  258,  274,  306,  268,
  267,  268,  259,  257,  259,  266,  271,  285,  275,  297,
  286,    0,  288,  289,  289,  291,  292,  267,  268,  288,
  296,  288,  272,  289,  288,  275,  257,  277,  288,    0,
  296,  298,  301,  257,  301,  257,  318,  313,  288,  289,
  257,  291,  267,  268,  288,  289,  328,  272,  298,  299,
  275,  301,  277,  303,  304,  257,  306,  278,    0,  257,
  310,  311,  312,  288,  289,  259,  291,  267,  268,  288,
  289,  257,  272,  298,  299,  275,  301,  277,  303,  304,
  259,  306,  312,    0,  257,  265,  311,  312,  288,  289,
  257,  291,  267,  268,  257,  303,  274,  272,  298,  299,
  275,  301,  277,  303,  304,  274,  306,  295,    0,  258,
  260,  302,  312,  288,  289,  274,  291,  267,  268,  257,
  270,  257,  272,  298,  299,  275,  301,  277,  303,  304,
    0,  306,  297,  257,  257,  260,  288,  289,  288,  291,
  292,  291,  267,  268,  296,  280,  272,  272,  298,  299,
  275,  301,  277,  303,  270,    0,  257,  257,  257,    0,
  267,  268,  257,  288,  274,  257,  291,  292,  275,  274,
  277,  257,  278,  298,  299,    0,  301,    0,  303,  267,
  268,  288,  270,  274,  272,  288,  289,  275,  291,  277,
  260,  298,  257,  296,  301,  257,  303,  267,  268,  274,
  288,  257,  272,  291,  292,  275,    0,  277,    0,    0,
  298,  299,    0,  301,  274,  303,  267,  268,  288,  274,
    0,  291,  267,  268,  275,  270,  277,  272,  298,  299,
  275,  301,  277,  303,    0,  269,    0,  288,  289,  269,
  291,  267,  268,  288,    6,   22,  291,  298,  299,  275,
  301,  277,  303,  298,  299,   65,  301,   27,  303,  267,
  268,    0,  288,  289,  272,  291,    0,  275,   40,  277,
   85,  138,  298,  299,   52,  301,   13,  303,  267,  268,
  288,   40,   94,  291,  105,    0,  275,  201,  277,   56,
  298,  299,  161,  301,  127,  303,  267,  268,  115,  288,
  202,  272,  291,    0,  275,  236,  277,    0,  174,  298,
  299,  133,  301,  176,  303,  340,  301,  288,  268,  293,
  291,    0,  112,   98,   -1,  267,  268,  298,  299,   -1,
  301,   -1,  303,  275,   -1,  277,   -1,   -1,   -1,    0,
   -1,   -1,   -1,   -1,   -1,   -1,  288,   -1,   -1,  291,
  267,  268,   -1,   -1,   -1,   -1,  298,  299,  275,  301,
  277,  303,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  288,   -1,   -1,  291,  267,  268,   -1,   -1,   -1,
   -1,  298,  299,  275,  301,  277,  303,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  288,  267,  268,  291,
   -1,   -1,   -1,   -1,   -1,  275,  298,  299,   -1,  301,
   -1,  303,   -1,   -1,   -1,   -1,   -1,   -1,  288,   -1,
   -1,  291,  267,  268,   -1,   -1,  267,  268,  298,   -1,
  275,  301,   -1,  303,  275,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  288,  267,  268,  291,  288,   -1,   -1,
  291,   -1,  275,  298,  277,   -1,  301,  298,  303,   -1,
  301,   -1,  303,  288,  289,  288,  291,   -1,  291,   -1,
   -1,   -1,   -1,  267,  268,  298,  299,   -1,  301,  267,
  268,  275,   -1,  277,   -1,   -1,   -1,  275,   -1,  277,
   -1,   -1,   -1,   -1,  288,   -1,  288,  289,   -1,  291,
  288,  267,  268,   -1,  298,   -1,   -1,  301,   -1,  275,
  298,  277,   -1,  301,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  288,   -1,   -1,   -1,   -1,   -1,  267,  268,
   -1,   -1,  298,  267,  268,  301,  275,   -1,  277,   -1,
   -1,  275,   -1,  277,   -1,   -1,   -1,   -1,   -1,  288,
   -1,   -1,  267,  268,  288,   -1,   -1,   -1,   -1,  298,
  275,   -1,  301,   -1,  298,   -1,   -1,  301,   -1,   -1,
  267,  268,   -1,  288,  267,  268,   -1,   -1,  275,   -1,
   -1,   -1,  275,  298,   -1,   -1,  301,   -1,  267,  268,
   -1,  288,   -1,   -1,   -1,  288,  275,   -1,   -1,   -1,
   -1,  298,   -1,   -1,  301,  298,  267,  268,  301,  288,
   -1,   -1,   -1,   -1,  275,   -1,   -1,   -1,   -1,  298,
   -1,   -1,  301,   -1,   -1,   -1,   -1,  288,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  298,   -1,   -1,
  301,
};
#define YYFINAL 4
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 313
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"INTEGER","STRING","FLOAT",
"ADDEQUIV","APPROXFIXED","ASPLB","ASPUB","AT","BLOCK","BOTTOM","CELL",
"CELLGROUP","CLASS","CONNECT","CORNERS","EQUIV","FIXED","FROM","HARDCELL",
"INITIALLY","INSTANCE","LAYER","LEFT","NAME","NEIGHBORHOOD","NOMIRROR",
"NONFIXED","NOPERMUTE","OF","ORIENT","ORIENTATIONS","PAD","PADGROUP","PERMUTE",
"PIN","RESTRICT","RIGHT","RIGIDFIXED","SIDE","SIDESPACE","SIGNAL","SOFTCELL",
"SOFTPIN","STDCELL","SUPERGROUP","TOP","PINGROUP","ENDPINGROUP","SWAPGROUP",
"UNEQUIV","CELLOFFSET","ECO_ADDED_CELL","LEGALBLKCLASS","CURRENT","POWER",
"NO_LAYER_CHANGE","KEEPOUT",
};
char *yyrule[] = {
"$accept : start_file",
"start_file : core pads",
"start_file : core",
"core : corecells",
"core : corecells cellgroups",
"corecells : coretype",
"corecells : corecells coretype",
"coretype : hardcell",
"coretype : softcell",
"coretype : stdcell",
"pads : padcells",
"pads : padcells padgroups",
"padcells : padcell",
"padcells : padcells padcell",
"padgroups : padgroup",
"padgroups : padgroups padgroup",
"cellgroups : cellgroup",
"cellgroups : cellgroups cellgroup",
"stdcell : cellname std_fixed bbox pinlist",
"stdcell : cellname optional_list std_fixed bbox pinlist",
"optional_list : option",
"optional_list : optional_list option",
"option : celloffset",
"option : eco",
"option : swap_group",
"option : legal_block_classes",
"option : mirror",
"option : initial_orient",
"hardcell : hardcellname custom_instance_list",
"hardcell : hardcellname fixed custom_instance_list",
"custom_instance_list : custom_instance",
"custom_instance_list : custom_instance_list instance custom_instance",
"custom_instance : corners keep_outs class orient hardpins",
"custom_instance : corners keep_outs class orient",
"softcell : softname soft_instance_list",
"softcell : softname fixed soft_instance_list",
"soft_instance_list : soft_instance",
"soft_instance_list : soft_instance_list instance soft_instance",
"soft_instance : corners aspect class orient softpins mc_pingroup",
"soft_instance : corners aspect class orient softpins",
"soft_instance : corners aspect class orient",
"instance : INSTANCE string",
"padcell : padname corners cur_orient restriction sidespace hardpins",
"padcell : padname corners cur_orient restriction sidespace",
"padgroup : padgroupname padgrouplist restriction sidespace",
"cellgroup : supergroupname supergrouplist class orient",
"cellgroup : cellgroupname neighborhood cellgrouplist",
"hardcellname : HARDCELL string NAME string",
"softname : SOFTCELL string NAME string",
"cellname : CELL string string",
"neighborhood : NEIGHBORHOOD INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc",
"neighborhood : NEIGHBORHOOD FIXED INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc",
"fixed : fixedcontext AT INTEGER FROM xloc INTEGER FROM yloc",
"fixed : fixedcontext NEIGHBORHOOD INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc",
"fixedcontext : FIXED",
"std_fixed :",
"std_fixed : initially fixed_type INTEGER FROM fixed_loc OF BLOCK INTEGER",
"swap_group : SWAPGROUP string",
"celloffset : CELLOFFSET offset_list",
"offset_list : INTEGER",
"offset_list : offset_list INTEGER",
"eco : ECO_ADDED_CELL",
"legal_block_classes : LEGALBLKCLASS num_block_classes block_classes",
"num_block_classes : INTEGER",
"block_classes : block_class",
"block_classes : block_classes block_class",
"block_class : INTEGER",
"initial_orient : ORIENT INTEGER",
"initially :",
"initially : INITIALLY",
"fixed_type : FIXED",
"fixed_type : NONFIXED",
"fixed_type : RIGIDFIXED",
"fixed_type : APPROXFIXED",
"fixed_loc : LEFT",
"fixed_loc : RIGHT",
"mirror : NOMIRROR",
"bbox : LEFT INTEGER RIGHT INTEGER BOTTOM INTEGER TOP INTEGER",
"xloc : STRING",
"yloc : STRING",
"padname : PAD string NAME string",
"padgroupname : PADGROUP string PERMUTE",
"padgroupname : PADGROUP string NOPERMUTE",
"supergroupname : SUPERGROUP string NAME string",
"cellgroupname : CELLGROUP string NAME string",
"corners : CORNERS INTEGER cornerpts",
"cornerpts : INTEGER INTEGER",
"cornerpts : cornerpts INTEGER INTEGER",
"class : CLASS INTEGER",
"orient : INTEGER ORIENTATIONS orientlist cur_orient",
"orient : ORIENTATIONS orientlist cur_orient",
"orientlist : INTEGER",
"orientlist : orientlist INTEGER",
"cur_orient :",
"cur_orient : ORIENT INTEGER",
"aspect : ASPLB FLOAT ASPUB FLOAT",
"softpins : softtype",
"softpins : softpins softtype",
"softtype : pintype",
"softtype : softpin",
"hardpins : pintype",
"hardpins : hardpins pintype",
"pinlist :",
"pinlist : stdgrppins",
"stdgrppins : std_grppintype",
"stdgrppins : stdgrppins std_grppintype",
"stdpins : std_pintype",
"stdpins : stdpins std_pintype",
"std_grppintype : pinrecord",
"std_grppintype : pinrecord equiv_list",
"std_grppintype : pinrecord unequiv_list",
"std_grppintype : pingroup",
"std_pintype : pinrecord",
"std_pintype : pinrecord equiv_list",
"std_pintype : pinrecord unequiv_list",
"pintype : pinrecord",
"pintype : pinrecord equiv_list",
"pingroup : PINGROUP stdpins ENDPINGROUP",
"softpin : softpin_info siderestriction",
"softpin : softpin_info siderestriction softequivs",
"softpin_info : SOFTPIN NAME string SIGNAL string opt_layer",
"pinrecord : required_pinfo contour current power no_layer_change",
"required_pinfo : PIN NAME string SIGNAL string layer",
"contour : INTEGER INTEGER",
"contour : num_corners pin_pts",
"num_corners : CORNERS INTEGER",
"pin_pts : INTEGER INTEGER",
"pin_pts : pin_pts INTEGER INTEGER",
"current :",
"current : CURRENT FLOAT",
"power :",
"power : POWER FLOAT",
"no_layer_change :",
"no_layer_change : NO_LAYER_CHANGE",
"equiv_list : equiv",
"equiv_list : equiv_list equiv",
"equiv : EQUIV NAME string layer contour",
"unequiv_list : unequiv",
"unequiv_list : unequiv_list unequiv",
"unequiv : UNEQUIV NAME string layer contour",
"softequivs : mc_equiv",
"softequivs : mc_equiv user_equiv_list",
"softequivs : user_equiv_list",
"mc_equiv : addequiv siderestriction",
"addequiv : ADDEQUIV",
"user_equiv_list : user_equiv",
"user_equiv_list : user_equiv_list user_equiv",
"user_equiv : equiv_name siderestriction connect",
"equiv_name : EQUIV NAME string opt_layer",
"connect :",
"connect : CONNECT",
"mc_pingroup : pingroupname pingrouplist siderestriction",
"mc_pingroup : mc_pingroup pingroupname pingrouplist siderestriction",
"pingroupname : PINGROUP string PERMUTE",
"pingroupname : PINGROUP string NOPERMUTE",
"pingrouplist : pinset",
"pingrouplist : pingrouplist pinset",
"pinset : string FIXED",
"pinset : string NONFIXED",
"siderestriction :",
"siderestriction : RESTRICT SIDE side_list",
"side_list : INTEGER",
"side_list : side_list INTEGER",
"sidespace :",
"sidespace : SIDESPACE FLOAT",
"sidespace : SIDESPACE FLOAT FLOAT",
"layer : LAYER INTEGER",
"opt_layer :",
"opt_layer : LAYER INTEGER",
"sideplace : STRING",
"restriction :",
"restriction : RESTRICT SIDE sideplace",
"padgrouplist : padset",
"padgrouplist : padgrouplist padset",
"padset : string FIXED",
"padset : string NONFIXED",
"supergrouplist : string",
"supergrouplist : supergrouplist string",
"cellgrouplist : string",
"cellgrouplist : cellgrouplist string",
"keep_outs :",
"keep_outs : keep_out_list",
"keep_out_list : keep_out",
"keep_out_list : keep_out_list keep_out",
"keep_out : KEEPOUT LAYER INTEGER CORNERS keep_pts",
"keep_pts : INTEGER INTEGER",
"keep_pts : keep_pts INTEGER INTEGER",
"string : STRING",
"string : INTEGER",
"string : FLOAT",
};
#endif
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#ifdef YYSTACKSIZE
#ifndef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#endif
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
int yycnprs = 0;
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#define yystacksize YYSTACKSIZE

#include "readcells_l.h"
/* add readcells_l.h for debug purposes */
/* ********************* #include "readcells_l.h" *******************/
/* ********************* #include "readcells_l.h" *******************/

readcells( fp )
FILE *fp ;
{ 
#ifdef YYDEBUG
    extern int yydebug ;
    yydebug = FALSE ;
    D( "syntax/readcells",
	yydebug = TRUE ;
    ) ;
#endif

    yyin = fp ;
    line_countS = 0 ;
    init() ;
    /* parse input file using yacc */
    yyparse();  

} /* end readcells */

yyerror(s)
char    *s;
{
    sprintf(YmsgG,"problem reading %s.cel:", cktNameG );
    M( ERRMSG, "yacc", YmsgG ) ;
    sprintf(YmsgG, "  line %d near '%s' : %s\n" ,
	line_countS+1, yytext, s );
    M( MSG,"yacc", YmsgG ) ;
} /* end yyerror */

yywrap()
{
    return(1);
}                      
#define YYABORT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, reading %d (%s)\n", yystate,
                    yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("yydebug: state %d, shifting to state %d\n",
                    yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
#endif
yynewerror:
#if YYDEBUG
    {
        int test_state, i, expect, two_or_more ;
        char err_msg[BUFSIZ] ;
        if( yyname[yychar] ){
            sprintf( err_msg, "Found %s.\nExpected ",
                yyname[yychar] ) ;
            two_or_more = 0 ;
            if( test_state = yysindex[yystate] ){
                for( i = 1; i <= YYMAXTOKEN; i++ ){
                    expect = test_state + i ;
                if((expect <= YYTABLESIZE) &&
                   (yycheck[expect] == i) &&
                   yyname[i]){
                        if( two_or_more ){
                            strcat( err_msg, " | " ) ;
                        } else {
                            two_or_more = 1 ;
                        }
                        strcat( err_msg, yyname[i] ) ;
                     }
                 }
             }
            if( test_state = yyrindex[yystate] ){
                for( i = 1; i <= YYMAXTOKEN; i++ ){
                    expect = test_state + i ;
                if((expect <= YYTABLESIZE) &&
                   (yycheck[expect] == i) &&
                   yyname[i]){
                        if( two_or_more ){
                            strcat( err_msg, " | " ) ;
                        } else {
                            two_or_more = 1 ;
                        }
                        strcat( err_msg, yyname[i] ) ;
                     }
                 }
             }
             yyerror( err_msg ) ;
             if (yycnprs) {
                 yychar = (-1);
                 if (yyerrflag > 0)  --yyerrflag;
                 goto yyloop;
             }
        } else {
            sprintf( err_msg, "Found unknown token.\nExpected ");
            two_or_more = 0 ;
            if( test_state = yysindex[yystate] ){
                for( i = 1; i <= YYMAXTOKEN; i++ ){
                    expect = test_state + i ;
                if((expect <= YYTABLESIZE) &&
                   (yycheck[expect] == i) &&
                   yyname[i]){
                        if( two_or_more ){
                            strcat( err_msg, " | " ) ;
                        } else {
                            two_or_more = 1 ;
                        }
                        strcat( err_msg, yyname[i] ) ;
                     }
                 }
             }
            if( test_state = yyrindex[yystate] ){
                for( i = 1; i <= YYMAXTOKEN; i++ ){
                    expect = test_state + i ;
                if((expect <= YYTABLESIZE) &&
                   (yycheck[expect] == i) &&
                   yyname[i]){
                        if( two_or_more ){
                            strcat( err_msg, " | " ) ;
                        } else {
                            two_or_more = 1 ;
                        }
                        strcat( err_msg, yyname[i] ) ;
                     }
                 }
             }
             yyerror( err_msg ) ;
             if (yycnprs) {
                 yychar = (-1);
                 if (yyerrflag > 0)  --yyerrflag;
                 goto yyloop;
             }
        }
     }
#else
     yyerror("syntax error");
     if (yycnprs) {
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
     }
#endif
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: state %d, error recovery shifting\
 to state %d\n", *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: error recovery discarding state %d\n",
                            *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, error recovery discards token %d (%s)\n",
                    yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("yydebug: state %d, reducing by rule %d (%s)\n",
                yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 33:
{
			sprintf( YmsgG,
			"hardcell at line %d does not have any pins\n",
			line_countS+1 ) ;
			M(WARNMSG,"readcells",YmsgG ) ;
		    }
break;
case 40:
{
			sprintf( YmsgG,
			"softcell at line %d does not have any pins\n",
			line_countS+1 ) ;
			M(WARNMSG,"readcells",YmsgG ) ;
		    }
break;
case 41:
{
			Ysafe_free( yyvsp[0].string ) ;
		    }
break;
case 43:
{
			sprintf( YmsgG,
			"pad cell at line %d does not have any pins\n",
			line_countS+1 ) ;
			M(WARNMSG,"readcells",YmsgG ) ;
		    }
break;
case 47:
{
			addCell(HARDCELLTYPE, yyvsp[0].string ) ;
			Ysafe_free( yyvsp[-2].string ) ;
		    }
break;
case 48:
{
			addCell(SOFTCELLTYPE, yyvsp[0].string ) ;
			Ysafe_free( yyvsp[-2].string ) ;
		    }
break;
case 49:
{
			addCell(STDCELLTYPE, yyvsp[0].string ) ;
			Ysafe_free( yyvsp[-1].string ) ;
		    }
break;
case 77:
{
			set_bbox( yyvsp[-6].ival, yyvsp[-4].ival, yyvsp[-2].ival, yyvsp[0].ival ) ;
		      }
break;
case 80:
{ 
			addCell(PADCELLTYPE, yyvsp[0].string ) ;
			Ysafe_free( yyvsp[-2].string ) ;
		    }
break;
case 81:
{
			addCell(PADCELLTYPE, yyvsp[-1].string ) ;
		    }
break;
case 82:
{
			addCell(PADCELLTYPE, yyvsp[-1].string ) ;
		    }
break;
case 83:
{
			Ysafe_free( yyvsp[-2].string ) ;
			Ysafe_free( yyvsp[0].string ) ;
		    }
break;
case 84:
{
			/* fixed locations same as pad context */
			Ysafe_free( yyvsp[-2].string ) ;
			Ysafe_free( yyvsp[0].string ) ;
		    }
break;
case 120:
{
			Ysafe_free( yyvsp[-3].string ) ;
			addNet( yyvsp[-1].string ) ;
		    }
break;
case 122:
{
			addNet( yyvsp[-1].string ) ;
		    }
break;
case 174:
{
			Ysafe_free( yyvsp[-1].string ) ;
		    }
break;
case 175:
{
			Ysafe_free( yyvsp[-1].string ) ;
		    }
break;
case 176:
{
			Ysafe_free( yyvsp[0].string ) ;
		    }
break;
case 177:
{
			Ysafe_free( yyvsp[0].string ) ;
		    }
break;
case 178:
{
			Ysafe_free( yyvsp[0].string ) ;
		    }
break;
case 179:
{
			Ysafe_free( yyvsp[0].string ) ;
		    }
break;
case 187:
{ 
			yyval.string = yyvsp[0].string ;	
		    }
break;
case 188:
{
			/* convert integer to string */
			/* this allows integers to be used as strings */
			/* a kluge but timberwolf's old parser supported it */
			sprintf( bufferS,"%d", yyvsp[0].ival ) ;
			/* now clone string */
			yyval.string = Ystrclone( bufferS ) ;
		    }
break;
case 189:
{
			/* convert float to string */
			/* this allows floats to be used as strings */
			/* a kluge but timberwolf's old parser supported it */
			sprintf( bufferS,"%f", yyvsp[0].fval ) ;
			/* now clone string */
			yyval.string = Ystrclone( bufferS ) ;
		    }
break;
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("yydebug: after reduction, shifting from state 0 to\
 state %d\n", YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("yydebug: state %d, reading %d (%s)\n",
                        YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("yydebug: after reduction, shifting from state %d \
to state %d\n", *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
