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
FILE:	    readobjects.c <- readobjects.y <- readobjects.l
DESCRIPTION:This file contains the grammar (BNF) for reading the 
	    positions of the objects from placement routine.  The rules
	    for lex are in readobjects.l.  The grammar for yacc is in
	    readobjects.y.
	    The output of yacc (y.tab.c) is renamed to readobjects.c
CONTENTS:   
	    readobjects()
	    yyerror(s)
		char    *s;
	    yywrap()
	
	    Note:readobjects.c will also include the yacc parser.
DATE:	    Apr  7, 1988 
REVISIONS:  Apr 20, 1989 - original coding.
	    Apr 30, 1989 - added library change and added partition
		case for compaction.
	    May  3, 1989 - changed to Y prefixes.
	    Jun 19, 1989 - now handle multiple edges correctly.
	    Sun Jan 20 21:31:36 PST 1991 - ported to AIX.
	    Thu Apr 18 01:22:43 EDT 1991 - added debug switch and
		made output prettier for syntax errors.
	    Sun Apr 21 22:38:38 EDT 1991 - added missing close file.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) readobjects.y version 2.5 4/21/91" ;
#endif

#include <string.h>
#include <globals.h>
#include <yalecad/message.h>  /* use message routines for errors. */
#include <yalecad/file.h>     /* file opening insert file. */
#include <yalecad/debug.h>    /* use debug utilities. */
#include <yalecad/string.h>

#undef REJECT          /* undefine TWMC macro for lex's version */ 

#ifdef DEBUG
#define YYDEBUG  1     /* condition compile for yacc debug */
#endif

static INT line_countS ;
static char bufferS[LRECL] ;
static char *filenameS ;

typedef union {
    INT ival ;
    char *string ;
    double fval ;
} YYSTYPE;
#define INTEGER 257
#define STRING 258
#define FLOAT 259
#define ARGS 260
#define COLON 261
#define DRAWN 262
#define EDGE 263
#define FULL 264
#define IFILES 265
#define NUMOBJECTS 266
#define OFILES 267
#define PATH 268
#define POBJECT 269
#define RELATIVE 270
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    2,    3,    3,    4,    8,    8,    9,    9,    5,
   15,   16,   16,    6,    6,   11,   11,   12,   12,   17,
   19,   18,   18,   13,   20,   20,    7,   10,   14,   21,
   21,   22,    1,    1,    1,
};
short yylen[] = {                                         2,
    2,    2,    1,    2,    4,    1,    2,    4,    5,    3,
    3,    1,    2,    3,    2,    1,    2,    1,    2,    2,
    2,    1,    2,    3,    1,    2,    6,    3,    3,    1,
    2,    4,    1,    1,    1,
};
short yydefred[] = {                                      0,
    0,    0,    0,    2,    0,    0,    3,    0,    0,   34,
   33,   35,    0,    4,    0,    0,    0,   11,    0,    0,
    0,   12,    0,   14,    0,    0,    0,    6,    0,   13,
    0,    0,    7,    0,    0,    0,    0,   28,   20,    0,
    0,    0,   22,    0,    0,   21,    0,    0,    0,   23,
   27,    0,    0,    9,   25,    0,    0,   26,    0,    0,
   30,    0,   31,    0,   32,
};
short yydgoto[] = {                                       2,
   43,    3,    6,    7,    8,   16,   21,   27,   28,   29,
   35,   41,   48,   54,    9,   23,   36,   44,   42,   56,
   60,   61,
};
short yysindex[] = {                                   -258,
 -248,    0, -259,    0, -252, -259,    0, -257, -249,    0,
    0,    0, -244,    0, -246, -245, -241,    0, -252, -243,
 -242,    0, -238,    0, -237, -235, -242,    0, -240,    0,
 -234, -233,    0, -232, -236, -252, -231,    0,    0, -229,
 -230, -252,    0, -252, -224,    0, -227, -226, -252,    0,
    0, -252, -223,    0,    0, -252, -222,    0, -220, -222,
    0, -217,    0, -216,    0,
};
short yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,   24,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -219,    0,
    0,    0, -218,    0,    0,    0,    4,    0,    0,    0,
    0,    0,    0,    0,    0, -225,    0,    0,    0,    0,
    0, -215,    0, -221,    0,    0,    0,    2, -212,    0,
    0,    0,    0,    0,    0,    1,    0,    0,    0,    3,
    0,    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
   -5,    0,    0,   21,    0,    0,    0,    0,   22,    0,
    0,    0,    0,    0,    0,    0,    0,   10,    0,    0,
    0,   -7,
};
#define YYTABLESIZE 273
short yytable[] = {                                      13,
   24,    8,   29,    5,   10,   11,   12,    1,    4,    5,
   15,   17,   18,   24,   19,   22,   20,   25,   30,   31,
   26,   32,   37,    1,   34,   45,   14,   38,   39,   47,
   40,   46,   51,   52,   59,   53,   62,   57,   50,   64,
   65,   16,   15,   50,   18,   17,   55,   19,   33,   10,
   58,   49,   63,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   24,   24,    8,   29,    0,    0,    0,   24,
    8,   29,    5,
};
short yycheck[] = {                                       5,
    0,    0,    0,    0,  257,  258,  259,  266,  257,  269,
  268,  261,  257,   19,  261,  257,  262,  261,  257,  257,
  263,  257,  257,    0,  265,  257,    6,  261,  261,  260,
  267,  261,  257,  261,  257,  262,  257,  261,   44,  257,
  257,  267,  262,   49,  260,  267,   52,  260,   27,  268,
   56,   42,   60,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  262,  263,  263,  263,   -1,   -1,   -1,  269,
  269,  269,  269,
};
#define YYFINAL 2
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 270
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"INTEGER","STRING","FLOAT","ARGS",
"COLON","DRAWN","EDGE","FULL","IFILES","NUMOBJECTS","OFILES","PATH","POBJECT",
"RELATIVE",
};
char *yyrule[] = {
"$accept : start_file",
"start_file : numobjects object_list",
"numobjects : NUMOBJECTS INTEGER",
"object_list : object",
"object_list : object_list object",
"object : name path draw_obj list_of_edges",
"list_of_edges : edge",
"list_of_edges : list_of_edges edge",
"edge : edge_keyword ifiles ofiles args",
"edge : edge_keyword ifiles ofiles args draw_edges",
"name : pname COLON depend_list",
"pname : POBJECT string INTEGER",
"depend_list : INTEGER",
"depend_list : depend_list INTEGER",
"path : PATH COLON string",
"path : PATH COLON",
"ifiles : ifiletype",
"ifiles : ifiletype list_of_files",
"ofiles : ofiletype",
"ofiles : ofiletype list_of_files",
"ifiletype : IFILES COLON",
"ofiletype : OFILES COLON",
"list_of_files : string",
"list_of_files : list_of_files string",
"args : ARGS COLON list_of_args",
"list_of_args : string",
"list_of_args : list_of_args string",
"draw_obj : DRAWN COLON INTEGER INTEGER INTEGER INTEGER",
"edge_keyword : EDGE INTEGER COLON",
"draw_edges : DRAWN COLON list_of_lines",
"list_of_lines : line",
"list_of_lines : list_of_lines line",
"line : INTEGER INTEGER INTEGER INTEGER",
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

#include "readobjects_l.h"
/* add readobjects.h for debug purposes */
/* ********************* #include "readobjects_l.h" *******************/
/* ********************* #include "readobjects_l.h" *******************/

readobjects( fp, filename )
FILE *fp ;
char *filename ;
{ 
#ifdef YYDEBUG
    extern int yydebug ;
    yydebug = FALSE ;
    D( "twflow/readobjects",
	yydebug = TRUE ;
    ) ;
#endif
    yyin = fp ;
    line_countS = 0 ;
    filenameS = filename ;
    /* parse input file using yacc */
    yyparse();  
    TWCLOSE( fp ) ;
    process_arcs() ;

} /* end readobjects */

yyerror(s)
char    *s;
{
    sprintf(YmsgG,"problem reading:%s\n", filenameS );
    M( ERRMSG, "yacc", YmsgG ) ;
    sprintf(YmsgG, "  line %d near '%s' : %s\n\n" ,
	line_countS+1, yytext, s );
    M( MSG,NULL, YmsgG ) ;
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
case 2:
{
			init( yyvsp[0].ival ) ;
		    }
break;
case 11:
{
			add_object( yyvsp[-1].string, yyvsp[0].ival ) ;
		    }
break;
case 12:
{
			add_pdependency( yyvsp[0].ival ) ;
		    }
break;
case 13:
{
			add_pdependency( yyvsp[0].ival ) ;
		    }
break;
case 14:
{
			/* replace path is TRUE */
			add_path( yyvsp[0].string ) ;
		    }
break;
case 15:
{
			add_path( NULL ) ;
		    }
break;
case 20:
{
			set_file_type( INPUTFILE ) ;
		    }
break;
case 21:
{
			set_file_type( OUTPUTFILE ) ;
		    }
break;
case 22:
{
			add_fdependency( yyvsp[0].string ) ;
		    }
break;
case 23:
{
			add_fdependency( yyvsp[0].string ) ;
		    }
break;
case 25:
{
			add_args( yyvsp[0].string ) ;
		    }
break;
case 26:
{
			add_args( yyvsp[0].string ) ;
		    }
break;
case 27:
{
			add_box( yyvsp[-3].ival, yyvsp[-2].ival, yyvsp[-1].ival, yyvsp[0].ival ) ;
		    }
break;
case 28:
{
			start_edge( yyvsp[-1].ival ) ;
		    }
break;
case 32:
{
			add_line( yyvsp[-3].ival, yyvsp[-2].ival, yyvsp[-1].ival, yyvsp[0].ival ) ;
		    }
break;
case 33:
{ 
			yyval.string = yyvsp[0].string ;	
		    }
break;
case 34:
{
			/* convert integer to string */
			/* this allows integers to be used as strings */
			/* a kluge but twolf's old parser supported it */
			sprintf( bufferS,"%d", yyvsp[0].ival ) ;
			/* now clone string */
			yyval.string = (char *) Ystrclone( bufferS ) ;
		    }
break;
case 35:
{
			/* convert float to string */
			/* this allows floats to be used as strings */
			/* a kluge but twolf's old parser supported it */
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
