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
FILE:	    readcells.c <- readcells_yacc <- readcells_lex
DESCRIPTION:This file contains the grammar (BNF) for the TimberWolfMC
	    input file parser. The rules for lex are in readcells_lex.
	    The grammar for yacc is in readcells_yacc.
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
	    Jan 29, 1989 - changed to YmsgG and added \n.
	    Mar  6, 1989 - removed redundant string from padgroupname
	    Mar  7, 1989 - now able to parse instance lists but
		don't process them.
	    Mar 12, 1989 - changed group cell and supercell reference
		from integer to string to be easier for user.
	    Mar 13, 1989 - added string to instance to tell them apart.
	    May  7, 1989 - added layer to input format.  Also fixed
		multiple equiv case.
	    May 12, 1989 - added layer to data structures.
	    Oct  2, 1989 - added warning messages for cells without pins.
	    Feb  2, 1989 - added orient to input fields.
	    Thu Dec 20 00:01:58 EST 1990 - now specify an ANALOGPIN.
	    Dec 21, 1990 - rewrote pinrecord to make it more flexible
		and also changed NO_CROSS_UNDER to NO_LAYER_CHANGE.
	    Sun Jan 20 21:34:36 PST 1991 - ported to AIX.
	    Wed Jan 23 14:40:30 PST 1991 - made language context free.
	    Mon Feb  4 02:17:24 EST 1991 - added new pinspace option
		and fixed fixCell problem.
	    Wed Feb 13 23:56:34 EST 1991 - modified for new pad code.
	    Thu Apr 18 01:44:14 EDT 1991 - renamed stdcell to cluster
		and reversed corners.
	    Thu Aug 22 22:10:09 CDT 1991 - fixed problem with
		fixed cells moving during pairwise flips.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) readcells.y version 3.15 3/6/92" ;
#endif

#include <string.h>
#include <yalecad/string.h>
#include <custom.h>
#include <initialize.h>
#include <readcells.h>  /* redefine yacc and lex globals */

#undef REJECT          /* undefine TWMC macro for lex's version */ 
#define YYDEBUG  1     /* condition compile for yacc debug */
/* #define LEXDEBUG 1  */   /* conditional compile for lex debug */
#ifdef LEXDEBUG
/* two bugs in yale version of lex */
#define allprint(x)    fprintf( stdout, "%c\n", x )
#define sprint(x)      fprintf( stdout, "%s\n", x )
#endif

static INT line_countS ;
static char bufferS[LRECL] ;

typedef union {
    INT ival ;
    char *string ;
    DOUBLE fval ;
} YYSTYPE;
#define INTEGER 257
#define STRING 258
#define FLOAT 259
#define ADDEQUIV 260
#define ASPLB 261
#define ASPUB 262
#define AT 263
#define CELLGROUP 264
#define CLASS 265
#define CONNECT 266
#define CORNERS 267
#define EQUIV 268
#define FIXED 269
#define FROM 270
#define HARDCELL 271
#define INSTANCE 272
#define LAYER 273
#define NAME 274
#define NEIGHBORHOOD 275
#define NONFIXED 276
#define NOPERMUTE 277
#define ORIENT 278
#define ORIENTATIONS 279
#define PAD 280
#define PADGROUP 281
#define PERMUTE 282
#define PIN 283
#define PINGROUP 284
#define RESTRICT 285
#define SIDE 286
#define SIDESPACE 287
#define SIGNAL 288
#define SOFTCELL 289
#define SOFTPIN 290
#define CLUSTER 291
#define SUPERGROUP 292
#define TIMING 293
#define CURRENT 294
#define KEEPOUT 295
#define NO_LAYER_CHANGE 296
#define POWER 297
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,    7,    7,    9,    9,   11,   11,    8,    8,
   14,   14,   15,   15,   10,   10,   12,   12,   20,   20,
   22,   22,   13,   13,   30,   30,   31,   31,   31,   23,
   16,   16,   17,   18,   18,   19,   19,   29,   29,   29,
   44,   44,   21,   21,   46,    2,    3,   35,   39,   39,
   39,   41,   41,   43,   43,   24,    6,   47,   47,   26,
   26,   27,   27,   48,   49,   49,   36,   36,   32,   33,
   50,   50,   51,   51,   28,   28,   52,   52,   54,   56,
   57,   57,   62,   63,   63,   59,   59,   60,   60,   61,
   61,   53,   53,   64,   67,   67,   67,   68,   70,   69,
   69,   71,   72,    5,    5,   34,   34,   73,   73,   74,
   74,   75,   75,   55,   55,   76,    4,    4,   65,   65,
   77,   77,   66,   66,   66,   38,   38,   38,   78,   78,
   37,   37,   40,   40,   79,   79,   42,   42,   45,   45,
   25,   25,   80,   80,   81,   82,   82,   58,   58,    1,
    1,    1,
};
short yylen[] = {                                         2,
    2,    1,    1,    2,    1,    2,    1,    1,    1,    2,
    1,    2,    1,    2,    1,    2,    2,    3,    1,    3,
    5,    4,    2,    3,    1,    3,    7,    6,    5,    2,
    6,    5,    4,    4,    3,    4,    2,    4,    4,    2,
   13,   14,    8,   14,    1,    1,    1,    4,    3,    3,
    2,    4,    2,    4,    2,    2,    2,    2,    3,    2,
    2,    4,    3,    1,    1,    2,    0,    2,    4,    1,
    1,    2,    1,    1,    1,    2,    1,    2,    6,    6,
    2,    2,    2,    2,    3,    0,    2,    0,    2,    0,
    1,    3,    4,    7,    1,    2,    1,    2,    1,    1,
    2,    3,    4,    0,    1,    4,    5,    3,    3,    1,
    2,    2,    2,    1,    2,    6,    0,    2,    0,    3,
    1,    2,    0,    2,    3,    0,    2,    3,    1,    1,
    0,    3,    1,    2,    2,    2,    1,    2,    1,    2,
    0,    1,    1,    2,    5,    2,    3,    0,    2,    1,
    1,    1,
};
short yydefred[] = {                                      0,
    0,    0,    0,    0,    0,    0,    5,    7,    8,    0,
    0,   37,  151,  150,  152,    0,   40,    0,    0,    0,
    1,    0,   11,    0,    0,    0,    0,    6,   15,    0,
    0,    0,   45,    0,    0,    0,   19,    0,    0,    0,
    0,    0,   25,    0,    0,    0,    0,    0,    0,   12,
   13,    0,    0,   55,    0,   53,    0,   16,  137,    0,
    0,    0,   57,    0,    0,    0,    0,    0,    0,    0,
    0,  143,    0,    0,    0,    0,    0,    0,   36,   38,
   39,    0,   51,    0,   14,    0,    0,  133,    0,    0,
    0,    0,    0,  138,    0,    0,    0,  139,    0,   58,
    0,   30,   20,    0,    0,  144,    0,    0,    0,    0,
   26,   48,   50,   49,  135,  136,    0,    0,  134,   68,
    0,   54,   52,   61,   60,   64,    0,   34,    0,    0,
    0,  140,   59,    0,    0,    0,    0,    0,    0,    0,
    0,   33,    0,   65,    0,    0,   46,    0,    0,    0,
    0,    0,   75,    0,    0,    0,    0,   69,    0,  130,
  129,  132,    0,    0,   66,   63,    0,    0,    0,    0,
    0,    0,   76,    0,    0,  114,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   71,   73,   74,    0,  128,
   62,    0,    0,  146,    0,    0,    0,  115,   81,   83,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   72,    0,    0,   47,    0,    0,  147,    0,    0,  149,
    0,    0,   84,    0,   43,    0,    0,    0,    0,    0,
    0,  110,    0,    0,    0,    0,    0,    0,    0,    0,
   87,    0,    0,   85,    0,    0,  109,  108,    0,  112,
  113,    0,  111,  121,    0,    0,   99,    0,   93,    0,
    0,    0,  100,    0,    0,    0,   80,  118,    0,   89,
   91,   79,    0,    0,    0,  106,  122,  125,    0,    0,
  101,   98,    0,    0,    0,  116,    0,    0,  107,    0,
  105,  102,    0,    0,    0,   94,  103,    0,    0,    0,
   41,    0,   44,   42,
};
short yydgoto[] = {                                       4,
  230,  148,  215,  240,  292,   34,    5,   21,    6,   27,
    7,    8,    9,   22,   49,   23,   51,   29,   10,   35,
   36,   37,   67,   38,   70,   95,  128,  152,   11,   42,
   43,   77,  184,  209,   24,   90,  118,  142,   52,   87,
   30,   60,   31,   62,   99,   39,   65,  129,  145,  185,
  186,  153,  188,  154,  175,  155,  179,  202,  222,  243,
  272,  180,  204,  189,  213,  235,  259,  260,  261,  262,
  263,  264,  210,  231,  232,  176,  255,  162,   88,   71,
   72,  171,
};
short yysindex[] = {                                   -207,
 -145, -141, -233,    0, -203, -231,    0,    0,    0, -138,
 -138,    0,    0,    0,    0, -194,    0, -144, -134, -150,
    0, -157,    0, -175, -137, -131, -242,    0,    0, -150,
 -130, -162,    0, -115, -129, -175,    0, -151, -189, -175,
 -114, -129,    0, -150, -150, -150, -128, -122, -127,    0,
    0, -150, -126,    0, -125,    0, -124,    0,    0, -186,
 -182, -150,    0, -109, -106, -150, -175, -129, -117, -110,
 -151,    0, -100,  -99, -129,  -98, -151, -175,    0,    0,
    0, -150,    0, -183,    0, -180, -223,    0,  -97, -123,
 -150, -150, -118,    0, -241, -111,  -94,    0, -150,    0,
  -92,    0,    0,  -90, -241,    0, -102, -101,  -91, -110,
    0,    0,    0,    0,    0,    0, -116, -113,    0,    0,
 -113,    0,    0,    0,    0,    0,  -85,    0, -104,  -82,
  -89,    0,    0, -103, -105,  -82,  -82,  -80, -241, -155,
  -77,    0, -105,    0, -209,  -85,    0,  -84,  -82,  -74,
  -88, -105,    0,  -83, -239,  -73,  -70,    0, -193,    0,
    0,    0,  -71, -105,    0,    0, -209,  -81,  -67,  -66,
  -65, -150,    0,  -79,  -83,    0,  -64,  -63,  -96,  -61,
  -72,  -69,  -75, -107, -193,    0,    0,    0,  -76,    0,
    0,  -58,  -68,    0,  -54,  -78, -150,    0,    0,    0,
  -55,  -62,  -52,  -51,  -58,  -58, -150, -150, -107, -150,
    0,  -60,  -57,    0,  -49,  -58,    0, -150,  -56,    0,
  -48,  -53,    0,  -45,    0,  -43,  -50, -177, -150, -178,
 -215,    0,  -42,  -41, -192,  -47,  -38,  -56,  -37,  -36,
    0,  -35,  -46,    0,  -39, -150,    0,    0, -215,    0,
    0,  -57,    0,    0,  -32,  -31,    0,  -40,    0,  -33,
  -33,  -76,    0,  -76,  -82,  -34,    0,    0,  -24,    0,
    0,    0,  -82,  -56,  -57,    0,    0,    0, -150,  -33,
    0,    0,  -44,  -20,  -82,    0,  -18,  -96,    0,  -56,
    0,    0,  -30,  -16,  -27,    0,    0,  -58,  -25,  -58,
    0,  -58,    0,    0,
};
short yyrindex[] = {                                      0,
    0,    0,    0,    0,  216,    6,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  229,    0,    0,    0,    0,    8,    0,    0,    0,
    0,    0,    0,    0,  598,    0,    0,  -19,    0,    0,
    0,  621,    0,    0,    0,    0,    0,    0,  247,    0,
    0,    0,  207,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  110,    0,    0,  629,    0,    0,
  -17,    0,    0,    0,  635,    0,  -19,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   49,    0,    0,   27,
    0,    0,    0,    0,    0,    0,    0,    0,    7,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    2,    0,    0,
   37,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  544,    0,    0,    0,    0,    0,
    0,    0,   21,    0,  495,    0,    0,    0,    0,    0,
    0,  548,    0,  311,    0,    0,    0,    0,  574,    0,
    0,    0,   57,   25,    0,    0,  495,    0,    0,    0,
 -250,    0,    0,    0,  370,    0,    0,    0,   32,    0,
    0,    0,    0,  584,  227,    0,    0,    0,  213,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   63,    0,    1,    0,    0,    0,    0,  588,    0,
    0,    0,  275,    0,    0,    0,    0,    0,   -8,    0,
    0,   93,    0,    0,    0,    0,    0,    0,    0,    0,
  509,    0,    0,    0,  428,    0,    0, -179,    0,    0,
    0,    0,  289,    0,    0,    0,    0,    0,  509,    0,
    0,  538,    0,    0,  166,  325,    0,    0,    0,  442,
  466,  399,    0,  356,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  132,  538,    0,    0,    0,    0,  480,
    0,    0,  413,    0,    0,    0,    0,  180,    0,  242,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
  903, -132, -196, -235,    0,    0,    0,    0,    0,    0,
  245,    0,    0,    0,    0,  230,  204,  228,    0,  218,
  246,  189,  -28,  -11,  181,  -59,  -93,  116,    0,  220,
  183,    0,    0,    0,    0,  -86,  172,  142,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  118,    0,
   81, -133,    0,    0,    0,    0,    0,  -21,    0,    0,
    0,    0,    0,    0, -208, -222,    0,    0,   10,    0,
 -195,    0,   59,   45, -166,  101,    0,    0,  190,    0,
  208,    0,
};
#define YYTABLESIZE 1182
short yytable[] = {                                      41,
   82,  126,  267,  156,  157,    3,   35,    4,  225,  226,
  105,  135,   53,   78,  145,  126,  169,  177,  173,  237,
   32,   25,  252,   19,   31,  187,  131,  178,   41,  276,
  173,  148,   25,   13,   14,   15,  126,  127,  288,    1,
  275,   13,   14,   15,  145,  159,   78,  165,  131,   26,
  139,  187,  289,  282,  297,  283,  127,    2,  166,    3,
   26,  117,   86,    1,  253,  281,   41,  257,   89,  212,
   13,   14,   15,   73,   96,  258,   20,  117,   93,   44,
  191,    2,  253,    3,  281,   74,   97,  117,  115,  151,
  250,   32,   88,  113,   63,  116,  183,  251,  114,  247,
  160,  301,  161,  303,  248,  304,   13,   14,   15,   56,
   12,   13,   14,   15,   17,   13,   14,   15,   54,   13,
   14,   15,   20,   48,   56,   13,   14,   15,   32,   45,
   33,  117,  284,   83,   13,   14,   15,  124,  125,   46,
  287,   64,   66,   69,   61,   82,   76,  100,   91,   92,
  101,   89,  294,   48,   93,  104,  107,  108,  130,  120,
  109,  117,  131,  150,  133,  120,  134,  136,  137,  140,
  138,  144,  168,  141,  146,  147,  208,  151,  158,  148,
  149,  163,  170,  181,  174,  172,  182,  190,  192,  193,
  194,  195,  199,  200,  197,  203,  201,  205,  207,  214,
  206,  216,  217,  220,  223,  224,   67,  236,  212,  218,
  241,  244,  119,  245,  254,    2,  239,  256,  266,  268,
  269,  291,  265,  270,  277,  233,   70,  278,    9,  234,
  273,  221,  286,  279,  258,  285,  293,  246,  295,  298,
  299,  117,  300,  242,  302,  141,   10,  142,  117,  271,
   28,   50,   85,   68,   58,  103,   40,  110,  164,   75,
  111,  121,  143,  167,   82,  211,  296,  229,   82,  280,
   35,   82,   82,  249,  123,  198,  119,    0,  106,    0,
   82,   82,  126,   82,   82,    3,   35,    4,   90,   82,
   82,   82,   82,   82,   82,  148,   82,   82,   35,  148,
   32,   32,  148,  148,   31,   31,  131,  131,    0,  131,
   77,  148,  148,  131,  148,  148,  126,  126,    0,  126,
  148,  148,  148,  148,  124,  148,   86,  148,  148,  131,
   86,    0,    0,   86,   86,  131,  127,  127,    0,  127,
    0,    0,   86,   86,    0,   86,   86,    0,    0,    0,
    0,   86,   86,   86,   86,  119,   88,    0,   86,   86,
   88,    0,    0,   88,   88,    0,    0,    0,    0,   78,
   56,    0,   88,   88,   56,   88,   88,    0,    0,    0,
    0,   88,   88,   88,   88,    0,    0,   56,   88,   56,
   56,  117,   56,    0,   56,  117,   56,    0,  119,  117,
    0,    0,  117,  117,   56,    0,    0,    0,    0,    0,
    0,  117,  104,    0,  117,  117,  117,    0,  117,    0,
  117,  117,  117,  117,  117,  120,    0,   92,    0,  120,
    0,  120,    0,  120,    0,    0,  120,  120,    0,  148,
    0,   95,    0,  148,    0,  120,    0,  148,  120,  120,
  148,  148,  120,    0,  120,  120,  120,  120,    0,  148,
    0,    0,  148,  148,  148,   97,  148,    0,  148,  148,
  148,  148,  119,    0,    0,    0,  119,    0,    0,   96,
  119,    0,    0,  119,  119,    0,   67,   67,    0,   67,
   70,   67,  119,   67,   67,  119,  119,   70,   70,  119,
    0,  119,  119,  119,  119,  117,   70,  117,  119,  117,
   70,    0,  117,  117,    0,   70,    0,   70,   70,    0,
    0,  117,    0,    0,  117,  117,  117,    0,    0,    0,
  117,  117,  117,  117,  123,    0,    0,  123,  123,    0,
    0,    0,  123,   22,    0,  123,  123,   21,    0,    0,
    0,    0,   90,    0,  123,    0,   90,  123,  123,   90,
   90,    0,    0,  123,  123,  123,  123,    0,   90,   90,
    0,   90,   90,   29,   77,    0,    0,   90,   90,   90,
   90,   77,   77,   28,  124,    0,    0,   27,  124,    0,
   77,   77,  124,   77,   77,  124,  124,   17,    0,   77,
   77,   77,   77,    0,  124,    0,    0,  124,  124,    0,
    0,    0,    0,  124,  124,  124,  124,    0,    0,  119,
   23,  119,    0,  119,    0,    0,  119,  119,   18,    0,
    0,    0,    0,   78,   24,  119,    0,    0,  119,  119,
   78,   78,    0,    0,  119,  119,  119,  119,    0,   78,
   78,    0,   78,   78,    0,    0,    0,    0,   78,   78,
   78,   78,  119,    0,    0,    0,  119,    0,    0,  119,
  119,    0,    0,    0,    0,    0,  104,    0,  119,    0,
  104,  119,  119,  104,  104,    0,    0,  119,  119,  119,
  119,   92,  104,    0,    0,  104,  104,    0,   92,   92,
    0,  104,  104,  104,  104,   95,    0,   92,    0,    0,
   92,   92,   95,   95,    0,    0,   92,   92,   92,   92,
    0,   95,    0,    0,   95,   95,    0,    0,    0,   97,
   95,   95,   95,   95,    0,    0,   97,   97,    0,    0,
    0,    0,    0,   96,    0,   97,    0,    0,   97,   97,
   96,   96,    0,    0,   97,   97,   97,   97,   67,   96,
    0,    0,   96,   96,    0,   67,   67,    0,   96,   96,
   96,   96,  119,    0,   67,    0,    0,   67,    0,  119,
  119,    0,    0,   67,   67,   67,   67,    0,  119,    0,
    0,    0,  119,    0,    0,  119,    0,  119,    0,  119,
  119,  123,    0,    0,    0,    0,    0,   22,  123,  123,
    0,   21,    0,    0,   22,   22,    0,  123,   21,   21,
    0,  123,    0,   22,    0,    0,  123,   21,  123,  123,
    0,    0,   22,    0,   22,   22,   21,   29,   21,   21,
    0,    0,    0,    0,   29,   29,    0,   28,    0,    0,
    0,   27,    0,   29,   28,   28,    0,    0,   27,   27,
    0,   17,   29,   28,   29,   29,    0,   27,   17,    0,
    0,    0,   28,    0,   28,   28,   27,   17,   27,   27,
    0,    0,    0,    0,   23,    0,   17,    0,   17,   17,
    0,   23,   18,    0,    0,    0,    0,    0,   24,   18,
   23,    0,    0,   16,   18,   24,    0,    0,   18,   23,
    0,   23,   23,    0,   24,    0,    0,   18,    0,   18,
   18,    0,   47,   24,    0,   24,   24,   55,   57,    0,
    0,    0,   59,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   79,   80,   81,    0,
   84,    0,    0,    0,   86,    0,    0,    0,    0,    0,
    0,    0,   94,    0,   98,    0,    0,    0,  102,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  112,    0,    0,    0,    0,   86,
    0,    0,    0,  122,  123,    0,    0,    0,    0,    0,
    0,  132,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  196,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  219,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  227,
  228,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  238,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  274,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  290,
};
short yycheck[] = {                                      11,
    0,    0,  238,  136,  137,    0,    0,    0,  205,  206,
   70,  105,   24,   42,  265,  257,  149,  257,  152,  216,
    0,  264,  231,  257,    0,  159,    0,  267,   40,  252,
  164,    0,  264,  257,  258,  259,    0,  279,  274,  271,
  249,  257,  258,  259,  295,  139,   75,  257,    0,  292,
  110,  185,  275,  262,  290,  264,    0,  289,  145,  291,
  292,  285,    0,  271,  231,  261,   78,  260,  278,  285,
  257,  258,  259,  263,  257,  268,  280,  257,  265,  274,
  167,  289,  249,  291,  280,  275,  269,  267,  269,  283,
  269,  267,    0,  277,  257,  276,  290,  276,  282,  277,
  256,  298,  258,  300,  282,  302,  257,  258,  259,    0,
  256,  257,  258,  259,  256,  257,  258,  259,  256,  257,
  258,  259,  280,  281,  256,  257,  258,  259,  267,  274,
  269,    0,  265,  256,  257,  258,  259,  256,  257,  274,
  273,  257,  272,  295,  275,  274,  261,  257,  274,  274,
  257,  278,  285,  281,  265,  273,  257,  257,  270,  257,
  259,  285,  257,  267,  257,    0,  257,  270,  270,  286,
  262,  257,  257,  287,  279,  258,  284,  283,  259,    0,
  270,  259,  257,  257,  268,  274,  257,  259,  270,  257,
  257,  257,  257,  257,  274,  257,  293,  270,  274,  258,
  270,  270,  257,  259,  257,  257,    0,  257,  285,  288,
  259,  257,    0,  257,  257,    0,  273,  259,  257,  257,
  257,  266,  270,  259,  257,  286,    0,  259,    0,  287,
  270,  294,  257,  274,  268,  270,  257,  288,  257,  270,
  257,    0,  270,  297,  270,  265,    0,  265,  257,  296,
    6,   22,   49,   36,   27,   67,   11,   77,  143,   40,
   78,   90,  121,  146,  264,  185,  288,  209,  268,  260,
  264,  271,  272,  229,    0,  175,   87,   -1,   71,   -1,
  280,  281,  281,  283,  284,  280,  280,  280,    0,  289,
  290,  291,  292,  293,  294,  264,  296,  297,  292,  268,
  280,  281,  271,  272,  280,  281,  280,  281,   -1,  283,
    0,  280,  281,  287,  283,  284,  280,  281,   -1,  283,
  289,  290,  291,  292,    0,  294,  264,  296,  297,  281,
  268,   -1,   -1,  271,  272,  287,  280,  281,   -1,  283,
   -1,   -1,  280,  281,   -1,  283,  284,   -1,   -1,   -1,
   -1,  289,  290,  291,  292,    0,  264,   -1,  296,  297,
  268,   -1,   -1,  271,  272,   -1,   -1,   -1,   -1,    0,
  261,   -1,  280,  281,  265,  283,  284,   -1,   -1,   -1,
   -1,  289,  290,  291,  292,   -1,   -1,  278,  296,  280,
  281,  260,  283,   -1,  285,  264,  287,   -1,    0,  268,
   -1,   -1,  271,  272,  295,   -1,   -1,   -1,   -1,   -1,
   -1,  280,    0,   -1,  283,  284,  285,   -1,  287,   -1,
  289,  290,  291,  292,  293,  260,   -1,    0,   -1,  264,
   -1,  266,   -1,  268,   -1,   -1,  271,  272,   -1,  260,
   -1,    0,   -1,  264,   -1,  280,   -1,  268,  283,  284,
  271,  272,  287,   -1,  289,  290,  291,  292,   -1,  280,
   -1,   -1,  283,  284,  285,    0,  287,   -1,  289,  290,
  291,  292,  260,   -1,   -1,   -1,  264,   -1,   -1,    0,
  268,   -1,   -1,  271,  272,   -1,  280,  281,   -1,  283,
  264,  285,  280,  287,    0,  283,  284,  271,  272,  287,
   -1,  289,  290,  291,  292,  264,  280,  266,    0,  268,
  284,   -1,  271,  272,   -1,  289,   -1,  291,  292,   -1,
   -1,  280,   -1,   -1,  283,  284,  285,   -1,   -1,   -1,
  289,  290,  291,  292,  260,   -1,   -1,    0,  264,   -1,
   -1,   -1,  268,    0,   -1,  271,  272,    0,   -1,   -1,
   -1,   -1,  264,   -1,  280,   -1,  268,  283,  284,  271,
  272,   -1,   -1,  289,  290,  291,  292,   -1,  280,  281,
   -1,  283,  284,    0,  264,   -1,   -1,  289,  290,  291,
  292,  271,  272,    0,  260,   -1,   -1,    0,  264,   -1,
  280,  281,  268,  283,  284,  271,  272,    0,   -1,  289,
  290,  291,  292,   -1,  280,   -1,   -1,  283,  284,   -1,
   -1,   -1,   -1,  289,  290,  291,  292,   -1,   -1,  264,
    0,  266,   -1,  268,   -1,   -1,  271,  272,    0,   -1,
   -1,   -1,   -1,  264,    0,  280,   -1,   -1,  283,  284,
  271,  272,   -1,   -1,  289,  290,  291,  292,   -1,  280,
  281,   -1,  283,  284,   -1,   -1,   -1,   -1,  289,  290,
  291,  292,  264,   -1,   -1,   -1,  268,   -1,   -1,  271,
  272,   -1,   -1,   -1,   -1,   -1,  264,   -1,  280,   -1,
  268,  283,  284,  271,  272,   -1,   -1,  289,  290,  291,
  292,  264,  280,   -1,   -1,  283,  284,   -1,  271,  272,
   -1,  289,  290,  291,  292,  264,   -1,  280,   -1,   -1,
  283,  284,  271,  272,   -1,   -1,  289,  290,  291,  292,
   -1,  280,   -1,   -1,  283,  284,   -1,   -1,   -1,  264,
  289,  290,  291,  292,   -1,   -1,  271,  272,   -1,   -1,
   -1,   -1,   -1,  264,   -1,  280,   -1,   -1,  283,  284,
  271,  272,   -1,   -1,  289,  290,  291,  292,  264,  280,
   -1,   -1,  283,  284,   -1,  271,  272,   -1,  289,  290,
  291,  292,  264,   -1,  280,   -1,   -1,  283,   -1,  271,
  272,   -1,   -1,  289,  290,  291,  292,   -1,  280,   -1,
   -1,   -1,  284,   -1,   -1,  287,   -1,  289,   -1,  291,
  292,  264,   -1,   -1,   -1,   -1,   -1,  264,  271,  272,
   -1,  264,   -1,   -1,  271,  272,   -1,  280,  271,  272,
   -1,  284,   -1,  280,   -1,   -1,  289,  280,  291,  292,
   -1,   -1,  289,   -1,  291,  292,  289,  264,  291,  292,
   -1,   -1,   -1,   -1,  271,  272,   -1,  264,   -1,   -1,
   -1,  264,   -1,  280,  271,  272,   -1,   -1,  271,  272,
   -1,  264,  289,  280,  291,  292,   -1,  280,  271,   -1,
   -1,   -1,  289,   -1,  291,  292,  289,  280,  291,  292,
   -1,   -1,   -1,   -1,  264,   -1,  289,   -1,  291,  292,
   -1,  271,  264,   -1,   -1,   -1,   -1,   -1,  264,  271,
  280,   -1,   -1,    1,    2,  271,   -1,   -1,  280,  289,
   -1,  291,  292,   -1,  280,   -1,   -1,  289,   -1,  291,
  292,   -1,   20,  289,   -1,  291,  292,   25,   26,   -1,
   -1,   -1,   30,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   44,   45,   46,   -1,
   48,   -1,   -1,   -1,   52,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   60,   -1,   62,   -1,   -1,   -1,   66,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   82,   -1,   -1,   -1,   -1,   87,
   -1,   -1,   -1,   91,   92,   -1,   -1,   -1,   -1,   -1,
   -1,   99,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,  172,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  197,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  207,
  208,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  218,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  246,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  279,
};
#define YYFINAL 4
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 297
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"INTEGER","STRING","FLOAT",
"ADDEQUIV","ASPLB","ASPUB","AT","CELLGROUP","CLASS","CONNECT","CORNERS","EQUIV",
"FIXED","FROM","HARDCELL","INSTANCE","LAYER","NAME","NEIGHBORHOOD","NONFIXED",
"NOPERMUTE","ORIENT","ORIENTATIONS","PAD","PADGROUP","PERMUTE","PIN","PINGROUP",
"RESTRICT","SIDE","SIDESPACE","SIGNAL","SOFTCELL","SOFTPIN","CLUSTER",
"SUPERGROUP","TIMING","CURRENT","KEEPOUT","NO_LAYER_CHANGE","POWER",
};
char *yyrule[] = {
"$accept : start_file",
"start_file : core pads",
"start_file : core",
"core : corecells",
"core : corecells cellgroups",
"corecells : coretype",
"corecells : corecells coretype",
"coretype : customcell",
"coretype : softcell",
"pads : padcells",
"pads : padcells padgroups",
"padcells : padcell",
"padcells : padcells padcell",
"padgroups : padgroup",
"padgroups : padgroups padgroup",
"cellgroups : cellgroup",
"cellgroups : cellgroups cellgroup",
"customcell : cellname custom_instance_list",
"customcell : cellname fixed custom_instance_list",
"custom_instance_list : custom_instance",
"custom_instance_list : custom_instance_list instance custom_instance",
"custom_instance : corners keep_outs class orient hardpins",
"custom_instance : corners keep_outs class orient",
"softcell : softname soft_instance_list",
"softcell : softname fixed soft_instance_list",
"soft_instance_list : soft_instance",
"soft_instance_list : soft_instance_list instance soft_instance",
"soft_instance : corners aspect keep_outs class orient softpins pingroup",
"soft_instance : corners aspect keep_outs class orient softpins",
"soft_instance : corners aspect keep_outs class orient",
"instance : INSTANCE string",
"padcell : padname corners cur_orient restriction sidespace hardpins",
"padcell : padname corners cur_orient restriction sidespace",
"padgroup : padgroupname padgrouplist restriction sidespace",
"cellgroup : supergroupname supergrouplist class orient",
"cellgroup : cellgroupname neighborhood cellgrouplist",
"cellname : HARDCELL string NAME string",
"cellname : HARDCELL error",
"softname : SOFTCELL string NAME string",
"softname : CLUSTER INTEGER NAME string",
"softname : SOFTCELL error",
"neighborhood : NEIGHBORHOOD INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc",
"neighborhood : NEIGHBORHOOD FIXED INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc",
"fixed : fixedcontext AT INTEGER FROM xloc INTEGER FROM yloc",
"fixed : fixedcontext NEIGHBORHOOD INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc",
"fixedcontext : FIXED",
"xloc : STRING",
"yloc : STRING",
"padname : PAD string NAME string",
"padgroupname : PADGROUP string PERMUTE",
"padgroupname : PADGROUP string NOPERMUTE",
"padgroupname : PADGROUP error",
"supergroupname : SUPERGROUP string NAME string",
"supergroupname : SUPERGROUP error",
"cellgroupname : CELLGROUP string NAME string",
"cellgroupname : CELLGROUP error",
"corners : numcorners cornerpts",
"numcorners : CORNERS INTEGER",
"cornerpts : INTEGER INTEGER",
"cornerpts : cornerpts INTEGER INTEGER",
"class : CLASS INTEGER",
"class : CLASS error",
"orient : numorientations ORIENTATIONS orientlist cur_orient",
"orient : ORIENTATIONS orientlist cur_orient",
"numorientations : INTEGER",
"orientlist : INTEGER",
"orientlist : orientlist INTEGER",
"cur_orient :",
"cur_orient : ORIENT INTEGER",
"aspect : ASPLB FLOAT ASPUB FLOAT",
"softpins : softpinlist",
"softpinlist : softtype",
"softpinlist : softpinlist softtype",
"softtype : pintype",
"softtype : softpin",
"hardpins : pintype",
"hardpins : hardpins pintype",
"pintype : pinrecord",
"pintype : pinrecord equiv_list",
"pinrecord : required_pinfo contour timing current power no_layer_change",
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
"softpin : softpin_info siderestriction pinspace",
"softpin : softpin_info siderestriction pinspace softequivs",
"softpin_info : SOFTPIN NAME string SIGNAL string layer timing",
"softequivs : mc_equiv",
"softequivs : mc_equiv user_equiv_list",
"softequivs : user_equiv_list",
"mc_equiv : addequiv siderestriction",
"addequiv : ADDEQUIV",
"user_equiv_list : user_equiv",
"user_equiv_list : user_equiv_list user_equiv",
"user_equiv : equiv_name siderestriction connect",
"equiv_name : EQUIV NAME string layer",
"connect :",
"connect : CONNECT",
"pingroup : pingroupname pingrouplist siderestriction pinspace",
"pingroup : pingroup pingroupname pingrouplist siderestriction pinspace",
"pingroupname : PINGROUP string PERMUTE",
"pingroupname : PINGROUP string NOPERMUTE",
"pingrouplist : pinset",
"pingrouplist : pingrouplist pinset",
"pinset : string FIXED",
"pinset : string NONFIXED",
"equiv_list : equiv",
"equiv_list : equiv_list equiv",
"equiv : EQUIV NAME string layer INTEGER INTEGER",
"layer :",
"layer : LAYER INTEGER",
"siderestriction :",
"siderestriction : RESTRICT SIDE side_list",
"side_list : INTEGER",
"side_list : side_list INTEGER",
"pinspace :",
"pinspace : SIDESPACE FLOAT",
"pinspace : SIDESPACE FLOAT FLOAT",
"sidespace :",
"sidespace : SIDESPACE FLOAT",
"sidespace : SIDESPACE FLOAT FLOAT",
"sideplace : STRING",
"sideplace : error",
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
"timing :",
"timing : TIMING FLOAT",
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
#endif

    yyin = fp ;
    line_countS = 0 ;
    initCellInfo() ;
    /* parse input file using yacc */
    yyparse();  
    cleanupReadCells() ;

} /* end readcells */

yyerror(s)
char    *s;
{
    sprintf(YmsgG,"problem reading %s.[m]cel:", cktNameG );
    M( ERRMSG, "readcells", YmsgG ) ;
    sprintf(YmsgG, "  line %d near '%s' : %s\n" ,
	line_countS+1, yytext, s );
    M( ERRMSG, NULL, YmsgG ) ;
    setErrorFlag() ;
}

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
case 7:
{
			endCell() ;
		    }
break;
case 8:
{
			endCell() ;
		    }
break;
case 11:
{ 
			endCell() ; 
		    }
break;
case 12:
{ 
			endCell() ;
		    }
break;
case 13:
{ 
			endCell() ; 
		    }
break;
case 14:
{ 
			endCell() ;
		    }
break;
case 15:
{ 
			endCell() ; 
		    }
break;
case 16:
{ 
			endCell() ; 
		    }
break;
case 22:
{
			sprintf(YmsgG,
			    "cell at line %d does not have any pins\n",
			    line_countS+1);
			M( WARNMSG,"readcells", YmsgG ) ;
		    }
break;
case 29:
{
			sprintf(YmsgG,
			    "cell at line %d does not have any pins\n",
			    line_countS+1);
			M( WARNMSG,"readcells", YmsgG ) ;
		    }
break;
case 30:
{    
			endCell() ;
			add_instance( yyvsp[0].string ) ;
		    }
break;
case 32:
{
			sprintf(YmsgG,
			    "pad at line %d does not have any pins\n",
			    line_countS+1);
			M( WARNMSG,"readcells", YmsgG ) ;
		    }
break;
case 36:
{
			addCell( yyvsp[0].string, CUSTOMCELLTYPE ) ;
		    }
break;
case 38:
{
			addCell( yyvsp[0].string, SOFTCELLTYPE ) ;
		    }
break;
case 39:
{
			addCell( yyvsp[0].string, STDCELLTYPE ) ;
		    }
break;
case 41:
{
			/* group neighborhood is free to move */
			/* we use fixCell to build group box */
			fixCell( GROUPFLAG, yyvsp[-11].ival, yyvsp[-9].string, yyvsp[-8].ival, yyvsp[-6].string, 
			    yyvsp[-5].ival, yyvsp[-3].string, yyvsp[-2].ival, yyvsp[0].string  ) ;
			addClass( --unique_classG ) ;
		    }
break;
case 42:
{
			/* group neighborhood is fixed */
			/* we use fixCell to build group box */
			fixCell( FIXEDGROUPFLAG, yyvsp[-11].ival, yyvsp[-9].string, yyvsp[-8].ival, yyvsp[-6].string, 
			    yyvsp[-5].ival, yyvsp[-3].string, yyvsp[-2].ival, yyvsp[0].string  ) ;
			addClass( --unique_classG ) ;
			
		    }
break;
case 43:
{
			/* cell is fixed at a point */
			fixCell( POINTFLAG, yyvsp[-5].ival, yyvsp[-3].string, yyvsp[-2].ival, yyvsp[0].string,
			    0, "L", 0, "B" ) ;
			addClass( --unique_classG ) ;
		    }
break;
case 44:
{
			/* cell is fixed within a fixed neighborhood */
			fixCell( NEIGHBORHOODFLAG, yyvsp[-11].ival, yyvsp[-9].string, yyvsp[-8].ival, yyvsp[-6].string, 
			    yyvsp[-5].ival, yyvsp[-3].string, yyvsp[-2].ival, yyvsp[0].string  ) ;
			addClass( --unique_classG ) ;
		    }
break;
case 46:
{
			yyval.string = yyvsp[0].string ;
		    }
break;
case 47:
{
			yyval.string = yyvsp[0].string ;
		    }
break;
case 48:
{ 
			addCell( yyvsp[0].string, PADCELLTYPE ) ;
		    }
break;
case 49:
{
			addCell( yyvsp[-1].string, PADGROUPTYPE ) ;
			setPermutation( TRUE ) ;
		    }
break;
case 50:
{
			addCell( yyvsp[-1].string, PADGROUPTYPE ) ;
			setPermutation( FALSE ) ;
		    }
break;
case 52:
{
			addCell( yyvsp[0].string, SUPERCELLTYPE ) ;
			/* initSuperGroup() ; */
		    }
break;
case 54:
{
			addCell( yyvsp[0].string, GROUPCELLTYPE ) ;
		    }
break;
case 56:
{
			processCorners( yyvsp[-1].ival ) ;
		    }
break;
case 57:
{
			yyval.ival = yyvsp[0].ival ;
		    }
break;
case 58:
{
			addCorner( yyvsp[-1].ival, yyvsp[0].ival ) ;
		    }
break;
case 59:
{
			addCorner( yyvsp[-1].ival, yyvsp[0].ival ) ;
		    }
break;
case 60:
{
			addClass( yyvsp[0].ival ) ;
		    }
break;
case 65:
{
			/* first in the list is the initial orientation */
			initOrient( yyvsp[0].ival ) ;
		    }
break;
case 66:
{
			addOrient( yyvsp[0].ival ) ;
		    }
break;
case 68:
{
			set_cur_orient( yyvsp[0].ival ) ;
		    }
break;
case 69:
{
			addAspectBounds( yyvsp[-2].fval, yyvsp[0].fval ) ;
		    }
break;
case 70:
{
			add_soft_array() ;
		    }
break;
case 79:
{
			process_pin() ;
		    }
break;
case 80:
{
			addPin( yyvsp[-3].string, yyvsp[-1].string, yyvsp[0].ival, HARDPINTYPE ) ;
		    }
break;
case 81:
{
			set_pin_pos( yyvsp[-1].ival, yyvsp[0].ival ) ;
		    }
break;
case 83:
{
			add_analog( yyvsp[0].ival ) ;
		    }
break;
case 84:
{
			add_pin_contour( yyvsp[-1].ival, yyvsp[0].ival ) ;
		    }
break;
case 85:
{
			add_pin_contour( yyvsp[-1].ival, yyvsp[0].ival ) ;
		    }
break;
case 87:
{
			add_current( yyvsp[0].fval ) ;
		    }
break;
case 89:
{
			add_power( yyvsp[0].fval ) ;
		    }
break;
case 91:
{
			no_layer_change() ;
		    }
break;
case 94:
{
			addPin( yyvsp[-4].string, yyvsp[-2].string, yyvsp[-1].ival, SOFTPINTYPE ) ;
			set_restrict_type( SOFTPINTYPE ) ;
		    }
break;
case 98:
{
			/* we are now done with addequiv types */
			set_restrict_type( SOFTEQUIVTYPE ) ;
		    }
break;
case 99:
{
			set_restrict_type( ADDEQUIVTYPE ) ;
			addEquivPin( NULL, 0, 0, 0, ADDEQUIVTYPE ) ;
		    }
break;
case 103:
{
			addPin( yyvsp[-1].string, NULL, yyvsp[0].ival, SOFTEQUIVTYPE ) ;
		    }
break;
case 104:
{ yyval.ival = FALSE ; }
break;
case 105:
{ yyval.ival = TRUE ; }
break;
case 108:
{
			set_restrict_type( PINGROUPTYPE ) ;
			start_pin_group( yyvsp[-1].string, TRUE ) ;
		    }
break;
case 109:
{
			set_restrict_type( PINGROUPTYPE ) ;
			start_pin_group( yyvsp[-1].string, FALSE ) ;
		    }
break;
case 112:
{
			add2pingroup( yyvsp[-1].string, TRUE ) ; /* fixed */
		    }
break;
case 113:
{
			add2pingroup( yyvsp[-1].string, FALSE ) ; /* nonfixed */
		    }
break;
case 116:
{
			addEquivPin( yyvsp[-3].string, yyvsp[-2].ival, yyvsp[-1].ival, yyvsp[0].ival, HARDPINTYPE ) ;
		    }
break;
case 117:
{ /* default any layer */ yyval.ival = 0 ; }
break;
case 118:
{ yyval.ival = yyvsp[0].ival ; }
break;
case 121:
{
			addSideRestriction( yyvsp[0].ival ) ;
		    }
break;
case 122:
{
			addSideRestriction( yyvsp[0].ival ) ;
		    }
break;
case 124:
{
			add_pinspace( yyvsp[0].fval, yyvsp[0].fval ) ;
		    }
break;
case 125:
{
			add_pinspace( yyvsp[-1].fval, yyvsp[0].fval ) ;
		    }
break;
case 127:
{
			addSideSpace( yyvsp[0].fval, yyvsp[0].fval ) ;
		    }
break;
case 128:
{
			addSideSpace( yyvsp[-1].fval, yyvsp[0].fval ) ;
		    }
break;
case 129:
{ 
			addPadSide( yyvsp[0].string  ) ; 
		    }
break;
case 135:
{
			add2padgroup( yyvsp[-1].string, TRUE ) ; /* fixed */
		    }
break;
case 136:
{
			add2padgroup( yyvsp[-1].string, FALSE ) ; /* nonfixed */
		    }
break;
case 139:
{
			add_cell_to_group( yyvsp[0].string ) ;
		    }
break;
case 140:
{
			add_cell_to_group( yyvsp[0].string ) ;
		    }
break;
case 150:
{ 
			yyval.string = yyvsp[0].string ;	
		    }
break;
case 151:
{
			/* convert integer to string */
			/* this allows integers to be used as strings */
			/* a kluge but timberwolf's old parser supported it */
			sprintf( bufferS,"%d", yyvsp[0].ival ) ;
			/* now clone string */
			yyval.string = (char *) Ystrclone( bufferS ) ;
		    }
break;
case 152:
{
			/* convert float to string */
			/* this allows floats to be used as strings */
			/* a kluge but timberwolf's old parser supported it */
			sprintf( bufferS,"%f", yyvsp[0].fval ) ;
			/* now clone string */
			yyval.string = (char *) Ystrclone( bufferS ) ;
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
