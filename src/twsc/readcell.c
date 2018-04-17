/*
 *   Copyright (C) 1990-1991 Yale University
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
FILE:	    readcell.c <- readcell.y <- readcell.l
DESCRIPTION:This file contains the grammar (BNF) for the TimberWolfSC
	    input file parser. The rules for lex are in readcell.l.
	    The grammar for yacc is in readcell.y.
	    The output of yacc (y.tab.c) is renamed to readcell.c
CONTENTS:   
	    readcell( fp )
		FILE *fp ;
	    yyerror(s)
		char    *s;
	    yywrap()
	
	    Note:readcell.c will also include the yacc parser.
DATE:	    Jan 15, 1990 
REVISIONS:  Feb. 21st : added orient rule (for hardcells and pads for
		       new .smcel file from BILL 
	    From Kalapi's work on the partitioner.
	    Jan 15, 1991 - fixed fix_placement problem with fixed_type.
	    Mon Jan 21 21:39:39 PST 1991 - now handle the options correctly.
	    Sun Feb 17 21:06:47 EST 1991 - now sidespace has upper and
		lower bounds.
	    Mon Feb 18 18:56:53 EST 1991 - now syntax error tells
		correct output file.
	    Wed Apr  3 01:09:45 EST 1991 - now allow no pads yet have
		macros.
	    Thu Apr 18 01:54:09 EDT 1991 - added initial orient and
		reversed numcorner.
	    Sat May 11 22:51:19 EDT 1991 - pads without pins should
		only be a warning.
	    Wed Jul  3 13:32:08 CDT 1991 - now handle more than
		one set of unequivs.
	    Tue Aug  6 13:03:14 CDT 1991 - standard cells may
		now have 0 pins and name may be a string instead
		of an integer.
	    Tue Aug 13 12:52:19 CDT 1991 - fixed problem with
		mirror and created_new_cell file.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) readcell.y (Yale) version 4.16 8/13/91" ;
#endif

#include <yalecad/base.h>
#include <yalecad/message.h>
#include <yalecad/string.h>
#include <yalecad/debug.h>
#include "readcell.h"       

#include <standard.h>
#include <parser.h>
#include <main.h>

#undef REJECT 

#ifdef DEBUG
#define YYDEBUG  1          /* condition compile for yacc debug */
#endif /* DEBUG */

/* #define LEXDEBUG 1  */   /* conditional compile for lex debug */
#ifdef LEXDEBUG
/* two bugs in Yale version of lex */
#define allprint(x)    fprintf( stdout, "%c\n", x )
#define sprint(x)      fprintf( stdout, "%s\n", x )
#endif
static char bufferS[LRECL] ;

typedef union {
    INT ival ;
    char *string ;
    DOUBLE fval ;
} YYSTYPE;
#define INTEGER 257
#define STRING 258
#define FLOAT 259
#define FIXED 260
#define NONFIXED 261
#define RIGIDFIXED 262
#define APPROXIMATELY_FIXED 263
#define LEFT 264
#define RIGHT 265
#define BOTTOM 266
#define TOP 267
#define ASPLB 268
#define ASPUB 269
#define AT 270
#define BLOCK 271
#define CELL 272
#define CELLGROUP 273
#define CLASS 274
#define CORNERS 275
#define ECO_ADDED_CELL 276
#define ENDPINGROUP 277
#define EQUIV 278
#define FROM 279
#define GROUP 280
#define HARDCELL 281
#define INITIALLY 282
#define INSTANCE 283
#define LAYER 284
#define LEGALBLKCLASS 285
#define NAME 286
#define NEIGHBORHOOD 287
#define NOMIRROR 288
#define NOPERMUTE 289
#define OF 290
#define ORIENT 291
#define ORIENTATIONS 292
#define PAD 293
#define PADGROUP 294
#define PADSIDE 295
#define PERMUTE 296
#define PIN 297
#define PINGROUP 298
#define PORT 299
#define RESTRICT 300
#define SEQUENCE 301
#define SIDE 302
#define SIDERESTRICTION 303
#define SIDESPACE 304
#define SIGNAL 305
#define SOFTCELL 306
#define STDCELL 307
#define SUPERGROUP 308
#define SWAPGROUP 309
#define UNEQUIV 310
#define CELLOFFSET 311
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,    0,    0,    0,    9,    9,   13,   13,   11,
   11,   16,   16,   10,   10,   19,   19,   17,   17,   14,
   14,   15,   15,   28,   28,   29,   29,   29,   29,   29,
   29,   20,   20,   37,   37,   39,   39,   21,   21,   47,
   47,   48,   40,   18,   18,   18,   18,   22,   23,   23,
   36,   46,   24,   62,   62,   38,   38,   64,   25,   25,
   32,   33,    7,   65,   65,    8,   31,   35,   30,   66,
   66,    2,    2,    2,    2,    3,    3,   34,   26,    4,
    5,   51,   54,   55,   56,   56,   59,   61,   41,   67,
   68,   68,   69,   42,   44,   44,   43,   43,   70,   70,
   49,   50,   50,   71,   71,   71,   45,   45,   27,   27,
   75,   75,   77,   77,   76,   76,   76,   76,   78,   78,
   78,   72,   72,   82,   83,   73,   84,   74,   87,   85,
   85,   88,   79,   79,   80,   80,   89,   89,   81,   81,
   90,   90,   12,   12,   91,   91,   86,   53,   53,   53,
   52,   52,   58,   58,    6,   57,   57,   92,   92,   60,
   60,   63,   63,    1,    1,    1,
};
short yylen[] = {                                         2,
    2,    3,    2,    2,    1,    1,    2,    1,    2,    1,
    2,    1,    2,    1,    2,    1,    1,    1,    2,    1,
    2,    4,    5,    1,    2,    1,    1,    1,    1,    1,
    1,    2,    3,    1,    3,    5,    4,    2,    3,    1,
    3,    5,    2,    6,    5,    5,    4,    4,    4,    3,
    4,    4,    3,   13,   14,    8,   14,    1,    0,    8,
    2,    3,    1,    1,    2,    1,    1,    2,    2,    1,
    2,    1,    1,    1,    1,    1,    1,    1,    8,    1,
    1,    4,    5,    2,    3,    3,    4,    4,    2,    2,
    1,    2,    2,    2,    0,    2,    3,    2,    1,    2,
    4,    1,    2,    1,    1,    1,    1,    2,    0,    1,
    1,    2,    1,    2,    1,    2,    2,    3,    1,    2,
    2,    1,    2,    1,    1,    3,    2,    3,    2,    1,
    2,    5,    9,    7,    1,    2,    7,    5,    1,    2,
    7,    5,    1,    2,    9,    7,    3,    0,    2,    3,
    0,    3,    0,    3,    1,    1,    2,    2,    2,    1,
    2,    1,    2,    1,    1,    1,
};
short yydefred[] = {                                      0,
    0,    0,    0,    0,    8,    0,  165,  164,  166,    0,
    0,    0,    0,    0,    0,    3,    0,    0,   12,   14,
   16,   17,    0,    0,    0,    0,  143,    0,    0,    0,
    9,   20,    0,    0,   67,    0,    0,   78,    0,    0,
    0,    0,    0,   24,   26,   27,   28,   29,   30,   31,
   53,    0,    0,    0,    0,    0,    2,   15,  144,    0,
    0,   13,   18,    0,   58,    0,    0,    0,   34,    0,
    0,    0,    0,    0,    0,   40,    0,    0,    0,    0,
    0,   21,  160,    0,    0,    0,   72,   73,   74,   75,
    0,   63,    0,   68,   61,   70,    0,    0,    0,    0,
   25,    0,    0,    0,    0,    0,    0,   19,    0,    0,
  156,   90,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   91,    0,    0,    0,    0,    0,    0,   84,    0,
    0,    0,  161,    0,    0,    0,  162,    0,    0,   66,
   64,    0,   71,    0,    0,  124,   22,    0,  111,    0,
    0,    0,   51,    0,   82,    0,   52,   86,   85,  158,
  159,    0,    0,  157,   43,   35,   94,    0,    0,    0,
    0,    0,   93,   92,    0,    0,   41,   96,    0,    0,
    0,    0,   88,   87,   49,    0,    0,  163,    0,   65,
    0,    0,  112,    0,    0,    0,    0,  135,  139,    0,
  113,    0,   23,   83,    0,    0,   48,    0,   99,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  107,
    0,   80,    0,    0,   76,   77,    0,    0,    0,    0,
    0,  136,  140,  125,  114,  118,    0,    0,    0,    0,
  155,  154,    0,  100,    0,    0,    0,  101,    0,    0,
    0,  102,  104,  105,  106,    0,    0,  152,    0,  150,
  108,    0,    0,    0,    0,    0,    0,    0,    0,  146,
    0,    0,    0,  127,  129,  103,    0,    0,  130,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,  126,  131,  128,   81,    0,    0,
   60,    0,    0,    0,  138,    0,  142,    0,  145,   56,
    0,    0,    0,    0,    0,   79,  134,    0,    0,    0,
    0,    0,  147,    0,    0,    0,  137,  141,    0,  132,
    0,    0,  133,    0,    0,    0,    0,    0,    0,    0,
   54,    0,   57,   55,
};
short yydgoto[] = {                                       2,
  222,   91,  227,  223,  299,  242,   93,  141,    3,   15,
   16,   17,    4,   30,    5,   18,   61,   19,   20,   21,
   22,   63,   32,    6,   42,   99,  147,   43,   44,   45,
   46,   47,   48,   49,   50,   23,   67,   68,   69,  114,
   70,  117,  170,  128,  219,   24,   75,   76,  125,  251,
   25,  180,  182,   26,   79,   64,  110,  163,   33,   84,
   34,   86,  138,   71,  142,   97,   72,  121,  122,  210,
  252,  220,  254,  255,  148,  149,  200,  201,  221,  196,
  197,  151,  236,  256,  278,  295,  257,  279,  198,  199,
   27,  111,
};
short yysindex[] = {                                   -241,
 -236,    0, -170, -235,    0, -103,    0,    0,    0, -236,
 -236,  -55, -182, -236, -200,    0, -254, -107,    0,    0,
    0,    0, -240, -240, -159, -168,    0,  -95,  -89, -255,
    0,    0, -236, -138,    0,  -68,  -81,    0,  -77, -236,
  -73,  -66, -103,    0,    0,    0,    0,    0,    0,    0,
    0,  -86, -236,  -79, -236,  -74,    0,    0,    0, -236,
  -85,    0,    0, -236,    0,  -44,  -67, -159,    0,  -60,
 -180,  -40, -159,  -50,  -67,    0,  -72, -236,  -66,  -65,
  -63,    0,    0, -124,  -83, -236,    0,    0,    0,    0,
  -37,    0,  -35,    0,    0,    0,  -33,  -32, -108,  -66,
    0, -236,  -59, -236,  -78, -236, -126,    0,  -64, -229,
    0,    0, -236, -159,  -67,  -31, -213,  -29,  -26,  -24,
  -40,    0,  -67,  -25,  -60, -159,  -22,  -62,    0,  -57,
 -236, -236,    0, -213,  -43,  -18,    0, -236,  -42,    0,
    0,  -35,    0,  -23,  -46,    0,    0, -108,    0, -228,
  -56, -108,    0,  -14,    0, -236,    0,    0,    0,    0,
    0,  -53,  -57,    0,    0,    0,    0,  -48,  -12,  -72,
  -28,  -27,    0,    0,  -21, -213,    0,    0,  -52,  -57,
  -13,  -56,    0,    0,    0, -236,  -19,    0,  -54,    0,
   -4, -236,    0,  -30,  -17,  -16,  -51,    0,    0, -201,
    0, -228,    0,    0, -250, -236,    0,  -12,    0,   -3,
  -56, -236, -236,   -2, -140, -236,  -56,    5,  -56,    0,
  -16,    0,    6, -236,    0,    0,  -15,  -11,  -47, -236,
 -236,    0,    0,    0,    0,    0,  -16,  -51,   11,   13,
    0,    0,   -3,    0,  -56,   14,   15,    0,   16,   17,
 -140,    0,    0,    0,    0,  -36,  -36,    0,  -56,    0,
    0,  -16,    9,   21,   -6,   22, -236, -233, -217,    0,
   24,   38,   46,    0,    0,    0,    7, -139,    0, -139,
 -236,   48,   30,   65, -197,   58,   97,  101,  103,  107,
 -236, -236, -236,  109,    0,    0,    0,    0,  123, -236,
    0,  132,  143,  146,    0,  153,    0,  159,    0,    0,
  161,  -38,  170,  130,  179,    0,    0,  190,  195,  198,
  177, -236,    0, -236,  180,  204,    0,    0, -236,    0,
  210, -236,    0,  213,  193,  217,  199, -236,  200, -236,
    0, -236,    0,    0,
};
short yyrindex[] = {                                      0,
    0,    0,  282,    8,    0,  138,    0,    0,    0,    0,
    0,    0,    0,    0,  440,    0,  481,  482,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  115,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  138,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  197,    0,    0,    0,    0,    0,    0,    0,
  484,    0,    0,    0,    0,    0,   69,    0,    0,    0,
    0,    0,    0,    0,  124,    0,  171,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0, -144,    0,   98,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   19,
    0,    0,    0,    0,  142,    0,    0,    0,    0,    0,
  169,    0,  152,    0,    0,    0,    0,  183,    0,   91,
    0,    0,    0,    0,    0,    0,    0,    4,    0,    0,
    0, -110,    0,    0,    0,    0,    0,  114,    0,   47,
    0,   98,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    1,    0,    0,    0,    0,    0,    0,   55,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   91,
    0,   36,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   70,   84,    0,    0,    0,
    0, -189,    0,    0,    0,    0,    0,    0,    0,    3,
   66,    0,    0,    0,    0,    0,   41,  156,   80,    0,
  118,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0, -158, -151,    0,    0,
    0,    0,   25,    0,  148,    0,    0,    0,    0,    0,
  151,    0,    0,    0,    0,    0,    0,    0,  100,    0,
    0,  145,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
   -1,    0,    0, -207, -243,  269,    0,  344,    0,    0,
  473,    0,    0,    0,  485,    0,    0,  472,  476,    0,
    0,  431,  463,    0,  451,   10,  343,    0,  453,    0,
    0,    0,    0,    0,    0,    0,  429,  474,  385,  -61,
    2,  -69, -118,  330,  -58,    0,  428,  376,    0,    0,
    0,    0,  -20,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  382,  296,
  254, -173,    0,    0,    0,  358,    0,  307,  -87, -100,
  306,    0,    0,    0,  252,  230,    0, -186, -194, -164,
  494,  402,
};
#define YYTABLESIZE 512
short yytable[] = {                                      10,
  148,  232,   98,   50,  246,  247,  239,    6,   51,   52,
   54,  150,   56,  126,  134,  185,  264,   28,  153,   65,
    7,    8,    9,  286,   97,   74,   77,    7,    8,    9,
    1,   83,  233,  240,   66,   47,    1,   28,   95,  288,
   45,  253,  232,  168,   13,  261,  115,  310,  311,  194,
  287,  103,   29,  105,   95,  176,  315,  215,  107,  303,
  150,  126,  109,  202,  150,   37,  289,  232,   32,  116,
  162,  261,   29,  233,   74,  234,  129,  253,  169,   46,
   11,  195,  133,  117,  137,  261,  304,  119,  130,  118,
  148,  296,   12,  296,  341,  145,  343,  109,  344,   44,
  153,  237,  155,   55,  157,   14,  119,  119,  109,  152,
   11,  165,  202,  110,    7,   66,  331,  122,  120,   69,
  262,  334,   12,   38,  336,  121,   78,   74,   13,  183,
  184,   69,    7,    8,    9,   14,  188,   69,  120,  249,
   69,   33,  207,   69,  123,  121,   69,   36,   85,  116,
   42,   39,  245,   62,  205,  149,  145,  277,  259,  217,
  250,   80,  158,  294,   69,   62,   69,   81,   89,  159,
   95,   62,   35,  135,   62,   92,  136,   62,   36,   94,
   62,   37,  151,   96,   38,   12,   60,   39,  145,  146,
  229,   87,   88,   89,   90,  160,  161,   98,   62,  102,
   62,   53,    8,    9,  241,   40,  104,   41,   60,  225,
  226,  106,  112,  116,  241,  113,  120,  124,  127,  139,
  131,  140,  132,  143,  144,  167,  156,  171,  268,  269,
  172,  154,  173,  175,  178,  186,  189,  179,  187,  192,
  145,  191,  204,  208,  209,  218,  181,  214,  206,  216,
  212,  213,  228,  244,  266,  230,  248,  267,  195,  224,
  277,  194,  263,  260,  283,  285,  322,  270,  231,  271,
  272,  273,  274,  275,  265,   98,   50,  282,  284,  298,
  290,    5,   98,   98,   50,   98,  301,  281,    6,  298,
  298,  312,  293,   98,  148,   98,   50,   97,  298,   98,
    6,   98,   50,   98,   97,   97,    6,   97,   98,   50,
   98,   50,  153,    6,  305,   97,  291,   97,  115,  115,
  330,   97,  153,   97,  292,   97,  300,  115,   47,   47,
   97,  302,   97,   45,   45,   95,  298,   95,  298,  115,
  298,  116,  116,  115,  115,  115,   37,   95,   37,   32,
  116,   95,  115,  306,  115,  117,  117,  307,   37,  308,
   95,   32,  116,  309,  117,  313,  116,  116,  116,  109,
  109,   37,   46,   46,   32,  116,  117,  116,  109,  314,
  117,  117,  117,  148,  148,  110,  110,  148,  316,  117,
  109,  117,   44,   44,  110,    7,  109,  122,  122,  317,
  122,   59,  318,  109,   38,  109,  110,    7,  324,  319,
  122,  122,  110,    7,  122,  320,   38,  321,  122,  110,
    7,  110,   33,  122,  123,  123,  323,  123,   36,   38,
   36,   42,   39,   42,   33,  325,   89,  123,  123,    1,
   36,  123,   89,   42,   39,  123,  326,   33,  149,  149,
  123,  327,  149,   36,  328,  329,   42,   39,  332,   89,
  333,   89,   89,   95,   95,   89,  335,   95,   89,  337,
   95,  338,   89,  339,   95,  151,  151,  340,  342,  151,
    4,   10,  165,   11,  258,  190,  151,   57,   31,   62,
   58,  108,   82,  100,  203,  101,  115,   73,  166,  211,
  123,  177,  174,  243,  276,  193,  235,  238,  280,  297,
   59,  164,
};
short yycheck[] = {                                       1,
    0,  196,    0,    0,  212,  213,  257,    0,   10,   11,
   12,   99,   14,   75,   84,  134,  224,  273,    0,  260,
  257,  258,  259,  257,    0,   24,   25,  257,  258,  259,
  272,   33,  197,  284,  275,    0,  272,  273,   40,  257,
    0,  215,  237,  257,  299,  219,    0,  291,  292,  278,
  284,   53,  308,   55,    0,  125,  300,  176,   60,  257,
  148,  123,   64,  151,  152,    0,  284,  262,    0,    0,
  300,  245,  308,  238,   73,  277,   78,  251,  292,    0,
  281,  310,   84,    0,   86,  259,  284,  277,   79,  270,
    0,  278,  293,  280,  338,  297,  340,    0,  342,    0,
  102,  202,  104,  286,  106,  306,  287,  297,  110,  100,
  281,  113,  200,    0,    0,  275,  324,    0,  277,  264,
  221,  329,  293,    0,  332,  277,  295,  126,  299,  131,
  132,  276,  257,  258,  259,  306,  138,  282,  297,  280,
  285,    0,  163,  288,    0,  297,  291,    0,  287,  274,
    0,    0,  211,  264,  156,    0,  297,  297,  217,  180,
  301,  257,  289,  303,  309,  276,  311,  257,    0,  296,
    0,  282,  276,  257,  285,  257,  260,  288,  282,  257,
  291,  285,    0,  257,  288,  293,  294,  291,  297,  298,
  192,  260,  261,  262,  263,  260,  261,  264,  309,  286,
  311,  257,  258,  259,  206,  309,  286,  311,  294,  264,
  265,  286,  257,  274,  216,  283,  257,  268,  291,  257,
  286,  257,  286,  257,  257,  257,  305,  257,  230,  231,
  257,  291,  257,  259,  257,  279,  279,  300,  257,  286,
  297,  265,  257,  292,  257,  259,  304,  269,  302,  302,
  279,  279,  257,  257,  266,  286,  259,  305,  310,  279,
  297,  278,  257,  259,  271,  267,  305,  257,  286,  257,
  257,  257,  257,  257,  290,  273,  273,  257,  257,  281,
  257,    0,  280,  281,  281,  283,  257,  279,  281,  291,
  292,  293,  286,  291,  294,  293,  293,  273,  300,  297,
  293,  299,  299,  301,  280,  281,  299,  283,  306,  306,
  308,  308,  294,  306,  257,  291,  279,  293,  272,  273,
  322,  297,  304,  299,  279,  301,  279,  281,  293,  294,
  306,  267,  308,  293,  294,  281,  338,  283,  340,  293,
  342,  272,  273,  297,  298,  299,  281,  293,  283,  281,
  281,  297,  306,  257,  308,  272,  273,  257,  293,  257,
  306,  293,  293,  257,  281,  257,  297,  298,  299,  272,
  273,  306,  293,  294,  306,  306,  293,  308,  281,  257,
  297,  298,  299,  293,  294,  272,  273,  297,  257,  306,
  293,  308,  293,  294,  281,  281,  299,  280,  281,  257,
  283,  264,  257,  306,  281,  308,  293,  293,  279,  257,
  293,  294,  299,  299,  297,  257,  293,  257,  301,  306,
  306,  308,  281,  306,  280,  281,  257,  283,  281,  306,
  283,  281,  281,  283,  293,  257,  268,  293,  294,    0,
  293,  297,  274,  293,  293,  301,  257,  306,  293,  294,
  306,  257,  297,  306,  257,  279,  306,  306,  279,  291,
  257,  293,  294,  293,  294,  297,  257,  297,  300,  257,
  300,  279,  304,  257,  304,  293,  294,  279,  279,  297,
    0,    0,  286,    0,  216,  142,  304,   15,    4,   18,
   15,   61,   30,   43,  152,   43,   68,   24,  114,  170,
   73,  126,  121,  208,  251,  148,  200,  202,  257,  280,
   17,  110,
};
#define YYFINAL 2
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 311
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"INTEGER","STRING","FLOAT",
"FIXED","NONFIXED","RIGIDFIXED","APPROXIMATELY_FIXED","LEFT","RIGHT","BOTTOM",
"TOP","ASPLB","ASPUB","AT","BLOCK","CELL","CELLGROUP","CLASS","CORNERS",
"ECO_ADDED_CELL","ENDPINGROUP","EQUIV","FROM","GROUP","HARDCELL","INITIALLY",
"INSTANCE","LAYER","LEGALBLKCLASS","NAME","NEIGHBORHOOD","NOMIRROR","NOPERMUTE",
"OF","ORIENT","ORIENTATIONS","PAD","PADGROUP","PADSIDE","PERMUTE","PIN",
"PINGROUP","PORT","RESTRICT","SEQUENCE","SIDE","SIDERESTRICTION","SIDESPACE",
"SIGNAL","SOFTCELL","STDCELL","SUPERGROUP","SWAPGROUP","UNEQUIV","CELLOFFSET",
};
char *yyrule[] = {
"$accept : start_file",
"start_file : core macros",
"start_file : core macros pads",
"start_file : core pads",
"start_file : core ports",
"start_file : core",
"core : corecells",
"core : corecells cellgroups",
"corecells : stdcell",
"corecells : corecells stdcell",
"pads : padcells",
"pads : padcells padgroups",
"padcells : padcell",
"padcells : padcells padcell",
"macros : macro",
"macros : macros macro",
"macro : hardcell",
"macro : softcell",
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
"custom_instance : corners class orient actual_orient hardpins",
"custom_instance : corners class orient actual_orient",
"softcell : softname soft_instance_list",
"softcell : softname fixed soft_instance_list",
"soft_instance_list : soft_instance",
"soft_instance_list : soft_instance_list instance soft_instance",
"soft_instance : corners aspect class orient softpins",
"instance : INSTANCE string",
"padcell : padname corners actual_orient restriction_pad sidespace hardpins",
"padcell : padname corners actual_orient restriction_pad sidespace",
"padcell : padname_std padside bbox sidespace hardpins",
"padcell : padname_std padside bbox sidespace",
"padgroup : padgroupname padgrouplist restriction_pdgrp sidespace",
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
"std_fixed : INITIALLY fixed_type INTEGER FROM fixed_loc OF BLOCK INTEGER",
"swap_group : SWAPGROUP string",
"legal_block_classes : LEGALBLKCLASS num_block_classes block_classes",
"num_block_classes : INTEGER",
"block_classes : block_class",
"block_classes : block_classes block_class",
"block_class : INTEGER",
"eco : ECO_ADDED_CELL",
"initial_orient : ORIENT INTEGER",
"celloffset : CELLOFFSET offset_list",
"offset_list : INTEGER",
"offset_list : offset_list INTEGER",
"fixed_type : FIXED",
"fixed_type : NONFIXED",
"fixed_type : RIGIDFIXED",
"fixed_type : APPROXIMATELY_FIXED",
"fixed_loc : LEFT",
"fixed_loc : RIGHT",
"mirror : NOMIRROR",
"bbox : LEFT INTEGER RIGHT INTEGER BOTTOM INTEGER TOP INTEGER",
"xloc : string",
"yloc : string",
"padname : PAD string NAME string",
"padname_std : PAD INTEGER string ORIENT INTEGER",
"padside : PADSIDE string",
"padgroupname : PADGROUP string PERMUTE",
"padgroupname : PADGROUP string NOPERMUTE",
"supergroupname : SUPERGROUP INTEGER NAME string",
"cellgroupname : CELLGROUP INTEGER NAME string",
"corners : num_corners cornerpts",
"num_corners : CORNERS INTEGER",
"cornerpts : cornerpt",
"cornerpts : cornerpts cornerpt",
"cornerpt : INTEGER INTEGER",
"class : CLASS INTEGER",
"actual_orient :",
"actual_orient : ORIENT INTEGER",
"orient : INTEGER ORIENTATIONS orientlist",
"orient : ORIENTATIONS orientlist",
"orientlist : INTEGER",
"orientlist : orientlist INTEGER",
"aspect : ASPLB FLOAT ASPUB FLOAT",
"softpins : softtype",
"softpins : softpins softtype",
"softtype : pintype",
"softtype : pingrouptype",
"softtype : sequencetype",
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
"std_grppintype : pingroup stdpins endpingroup",
"std_pintype : pinrecord",
"std_pintype : pinrecord equiv_list",
"std_pintype : pinrecord unequiv_list",
"pintype : pinrecord",
"pintype : pinrecord equiv_list",
"pingroup : PINGROUP",
"endpingroup : ENDPINGROUP",
"pingrouptype : pingroupident softpinlist siderestriction",
"pingroupident : GROUP INTEGER",
"sequencetype : seqident softpinlist siderestriction",
"seqident : SEQUENCE INTEGER",
"softpinlist : softpinrecord",
"softpinlist : softpinlist softpinrecord",
"softpinrecord : PIN NAME string SIGNAL string",
"pinrecord : PIN NAME string SIGNAL string LAYER INTEGER INTEGER INTEGER",
"pinrecord : PIN NAME string SIGNAL string INTEGER INTEGER",
"equiv_list : equiv",
"equiv_list : equiv_list equiv",
"equiv : EQUIV NAME string LAYER INTEGER INTEGER INTEGER",
"equiv : EQUIV NAME string INTEGER INTEGER",
"unequiv_list : unequiv",
"unequiv_list : unequiv_list unequiv",
"unequiv : UNEQUIV NAME string LAYER INTEGER INTEGER INTEGER",
"unequiv : UNEQUIV NAME string INTEGER INTEGER",
"ports : port",
"ports : ports port",
"port : PORT NAME string SIGNAL string LAYER INTEGER INTEGER INTEGER",
"port : PORT NAME string SIGNAL string INTEGER INTEGER",
"siderestriction : SIDERESTRICTION INTEGER INTEGER",
"sidespace :",
"sidespace : SIDESPACE FLOAT",
"sidespace : SIDESPACE FLOAT FLOAT",
"restriction_pad :",
"restriction_pad : RESTRICT SIDE sideplace",
"restriction_pdgrp :",
"restriction_pdgrp : RESTRICT SIDE sideplace",
"sideplace : string",
"padgrouplist : padset",
"padgrouplist : padgrouplist padset",
"padset : string FIXED",
"padset : string NONFIXED",
"supergrouplist : string",
"supergrouplist : supergrouplist string",
"cellgrouplist : string",
"cellgrouplist : cellgrouplist string",
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

#include "readcell_l.h"
/* add readcell_l.h for debug purposes */
/* ********************* #include "readcell_l.h" *******************/
/* ********************* #include "readcell_l.h" *******************/

readcell( fp )
FILE *fp ;
{ 
#ifdef YYDEBUG
    extern int yydebug ;
    yydebug = FALSE ;
    D( "twsc/readcell",
	yydebug = TRUE ;
    ) ;
#endif

    yyin = fp ;
    line_countS = 0 ;
    initialize_parser() ;
    yyparse();  
    cleanup_readcells();
} /* end readcell */

yyerror(s)
char    *s;
{
    if( rowsG > 0 ){
	sprintf(YmsgG,"problem reading %s.scel:", cktNameG );
    } else {
	sprintf(YmsgG,"problem reading %s.cel:", cktNameG );
    }
    M( ERRMSG, "yacc", YmsgG ) ;
    sprintf(YmsgG, "  line %d near '%s' : %s\n" ,
	line_countS+1, yytext, s );
    M( ERRMSG,NULL, YmsgG ) ;
    set_error_flag() ;
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
            sprintf( err_msg, "\nsyntax error - found:%s expected:",
                yyname[yychar] ) ;
            two_or_more = 0 ;
            if( test_state = yysindex[yystate] ){
                for( i = YYERRCODE+1; i <= YYMAXTOKEN; i++ ){
                    expect = test_state + i ;
                    if( expect <= YYTABLESIZE && yycheck[expect] == i ){
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
                for( i = YYERRCODE+1; i <= YYMAXTOKEN; i++ ){
                    expect = test_state + i ;
                    if( expect <= YYTABLESIZE && yycheck[expect] == i ){
                        if( two_or_more ){
                            strcat( err_msg, " | " ) ;
                        } else {
                            two_or_more = 1 ;
                        }
                        strcat( err_msg, yyname[i] ) ;
                     }
                 }
             }
             strcat( err_msg, "\n" ) ;
             yyerror( err_msg ) ;
        } else {
            yyerror("syntax error");
        }
     }
#else
     yyerror("syntax error");
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
case 6:
{
			add_extra_cells() ;
		    }
break;
case 7:
{
			add_extra_cells() ;
		    }
break;
case 37:
{
			sprintf( YmsgG,
			"hardcell at line %d does not have any pins\n",
			line_countS+1 ) ;
			M(WARNMSG,"readcells",YmsgG ) ;
		    }
break;
case 43:
{
			Ysafe_free( yyvsp[0].string ) ;
		    }
break;
case 48:
{
			end_padgroup() ;
		    }
break;
case 51:
{
			addCell( yyvsp[0].string, HARDCELLTYPE ) ;
		    }
break;
case 52:
{
			not_supported("softcells") ;
			Ysafe_free( yyvsp[0].string ) ;
		    }
break;
case 53:
{
			addCell( yyvsp[0].string, STDCELLTYPE ) ;
		    }
break;
case 60:
{
		         fix_placement( yyvsp[-6].string, yyvsp[-5].ival, yyvsp[-3].string, yyvsp[0].ival ) ; 
		    }
break;
case 61:
{
			add_swap_group( yyvsp[0].string ) ;
		    }
break;
case 66:
{
		        add_legal_blocks( yyvsp[0].ival ) ;
		    }
break;
case 67:
{
			add_eco() ;
		    }
break;
case 68:
{
			add_initial_orient( yyvsp[0].ival ) ;
		    }
break;
case 72:
{
			yyval.string = Ystrclone( "fixed" ) ;
                    }
break;
case 73:
{
			yyval.string = Ystrclone( "nonfixed" ) ;
                    }
break;
case 74:
{
		        yyval.string = Ystrclone( "rigidly_fixed" ) ;
                    }
break;
case 75:
{
		        yyval.string = Ystrclone( "approximately_fixed" ) ;
                    }
break;
case 76:
{
		        yyval.string = Ystrclone( "left" ) ;
		    }
break;
case 77:
{
		        yyval.string = Ystrclone( "right" ) ;
		    }
break;
case 78:
{
		        set_mirror_flag();
                    }
break;
case 79:
{
		        add_tile( yyvsp[-6].ival, yyvsp[-2].ival, yyvsp[-4].ival, yyvsp[0].ival ) ;
		    }
break;
case 80:
{
		        yyval.string = yyvsp[0].string;
		    }
break;
case 81:
{
		        yyval.string = yyvsp[0].string;
		    }
break;
case 82:
{ 
			addCell( yyvsp[0].string, PADTYPE ) ;
		    }
break;
case 83:
{
			addCell( yyvsp[-2].string, PADTYPE ) ;
			add_orient(yyvsp[0].ival);
		    }
break;
case 84:
{
		        add_padside( yyvsp[0].string ) ;
			set_old_format( yyvsp[0].string ) ;
		    }
break;
case 85:
{
			addCell( yyvsp[-1].string, PADGROUPTYPE ) ;
			setPermutation( TRUE ) ;
			add_tile( 0,0,0,0 ) ;
		    }
break;
case 86:
{
			addCell( yyvsp[-1].string, PADGROUPTYPE ) ;
			setPermutation( FALSE ) ;
			add_tile( 0,0,0,0 ) ;
		    }
break;
case 87:
{
			Ysafe_free( yyvsp[0].string ) ;
		    }
break;
case 88:
{
			Ysafe_free( yyvsp[0].string ) ;
		    }
break;
case 89:
{
			process_corners() ;
		    }
break;
case 90:
{
			init_corners() ;
		    }
break;
case 93:
{
			add_corner( yyvsp[-1].ival, yyvsp[0].ival ) ;
                    }
break;
case 96:
{
			add_orient(yyvsp[0].ival);
		    }
break;
case 124:
{
		        add_pingroup();
		    }
break;
case 125:
{
		        end_pingroup();
		    }
break;
case 132:
{
			/* addNet( $5 ) ; */
		    }
break;
case 133:
{
			add_pin( yyvsp[-6].string, yyvsp[-4].string, yyvsp[-2].ival, yyvsp[-1].ival, yyvsp[0].ival ) ;
		    }
break;
case 134:
{
			add_pin( yyvsp[-4].string, yyvsp[-2].string, 0, yyvsp[-1].ival, yyvsp[0].ival ) ;
		    }
break;
case 137:
{
		        add_equiv( yyvsp[-4].string, yyvsp[-2].ival, yyvsp[-1].ival, yyvsp[0].ival, FALSE ) ; 
		    }
break;
case 138:
{
			add_equiv( yyvsp[-2].string, 0, yyvsp[-1].ival, yyvsp[0].ival, FALSE ) ; 
		    }
break;
case 141:
{
			add_equiv( yyvsp[-4].string, yyvsp[-2].ival, yyvsp[-1].ival, yyvsp[0].ival, TRUE ) ; 
		    }
break;
case 142:
{
			add_equiv( yyvsp[-2].string, 0, yyvsp[-1].ival, yyvsp[0].ival, TRUE ) ; 
		    }
break;
case 145:
{
			add_port( yyvsp[-6].string, yyvsp[-4].string, yyvsp[-2].ival, yyvsp[-1].ival, yyvsp[0].ival ) ;
		    }
break;
case 146:
{
			add_port( yyvsp[-4].string, yyvsp[-2].string, 0, yyvsp[-1].ival, yyvsp[0].ival ) ;
		    }
break;
case 149:
{
		        add_sidespace( yyvsp[0].fval, yyvsp[0].fval );
                    }
break;
case 150:
{
		        add_sidespace( yyvsp[-1].fval, yyvsp[0].fval );
                    }
break;
case 155:
{
			add_padside( yyvsp[0].string ) ;
                    }
break;
case 158:
{
			add2padgroup( yyvsp[-1].string, TRUE ) ; /* fixed */
		    }
break;
case 159:
{
			add2padgroup( yyvsp[-1].string, FALSE ) ; /* nonfixed */
		    }
break;
case 160:
{
			Ysafe_free( yyvsp[0].string ) ;
		    }
break;
case 161:
{
			Ysafe_free( yyvsp[0].string ) ;
		    }
break;
case 162:
{
			Ysafe_free( yyvsp[0].string ) ;
		    }
break;
case 163:
{
			Ysafe_free( yyvsp[0].string ) ;
		    }
break;
case 164:
{ 
			yyval.string = yyvsp[0].string ;	
		    }
break;
case 165:
{
			/* convert integer to string */
			/* this allows integers to be used as strings */
			sprintf( bufferS,"%d", yyvsp[0].ival ) ;
			/* now clone string */
			yyval.string = Ystrclone( bufferS ) ;
		    }
break;
case 166:
{
			/* convert float to string */
			/* this allows floats to be used as strings */
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
