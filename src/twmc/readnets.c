/*
 *   Copyright (C) 1988-1992 Yale University
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
FILE:	    readnets.c <- readnets_yacc <- readnets_lex
DESCRIPTION:This file contains the grammar (BNF) for the TimberWolfMC
	    input file parser for nets. The rules for lex are in 
	    readnets_lex.  The grammar for yacc is in readnets_yacc.
	    The output of yacc (y.tab.c) is renamed to readnets.c
CONTENTS:   
	    readnets( fp )
		FILE *fp ;
	    yyerror(s)
		char    *s;
	    yywrap()
	
	    Note:readcells.c will also include the yacc parser.
DATE:	    Aug  7, 1988 
REVISIONS:  Jan 29, 1989 - changed to YmsgG and added \n's.
	    Mar 01, 1989 - modified argument to exitPgm.
	    Mar 10, 1989 - added INSTANCETYPE code.
	    Mar 11, 1989 - added get_total_paths for new print_paths.
	    Mar 16, 1989 - changed data structure of netlist.
	    Apr  2, 1989 - removed instance type.
	    Oct  2, 1989 - made .mnet file optional.
	    Apr 23, 1990 - now avoid crash when faulty user input.
	    Sun Dec 16 00:42:34 EST 1990 - moved functions to initnets.c
		and finalized analog input format.
	    Sun Jan 20 21:34:36 PST 1991 - ported to AIX.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) readnets.y version 3.8 3/6/92" ;
#endif

#include <string.h>
#include <yalecad/base.h>
#include <yalecad/message.h>
#include <yalecad/string.h>
#include <readnets.h>  /* redefine yacc and lex globals */

#undef REJECT          /* undefine TWMC macro for lex's version */ 
#define YYDEBUG  1     /* condition compile for yacc debug */

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
#define CAP_MATCH 260
#define CAP_UPPER_BOUND 261
#define COLON 262
#define COMMON_POINT 263
#define NET 264
#define NET_CAP_MATCH 265
#define NET_RES_MATCH 266
#define NOISY 267
#define PATH 268
#define RES_MATCH 269
#define RES_UPPER_BOUND 270
#define SENSITIVE 271
#define SHIELDING 272
#define TIMING 273
#define MAX_VOLTAGE_DROP 274
#define COMMA 275
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    2,    2,    3,    3,    3,    3,    4,    4,    8,
    8,   10,    5,    9,    9,   11,   11,   11,   11,   11,
   11,   16,   12,   13,   14,   15,   15,   15,   19,   17,
   18,   18,   20,   20,   22,   22,   21,   21,   23,   23,
    6,    6,    7,    7,    1,    1,    1,
};
short yylen[] = {                                         2,
    1,    1,    2,    1,    1,    1,    1,    5,    4,    2,
    2,    0,    4,    1,    2,    1,    1,    1,    1,    1,
    1,    2,    2,    2,    2,    1,    1,    1,    0,    5,
    2,    4,    0,    1,    3,    4,    0,    1,    3,    4,
    2,    2,    2,    2,    1,    1,    1,
};
short yydefred[] = {                                      0,
    0,    0,    0,    0,    0,    0,    2,    4,    5,    0,
    0,    0,   46,   45,   47,   12,   41,   43,   10,    3,
   42,   44,    0,   11,    0,    0,    0,   29,   26,    0,
   27,   28,    0,    0,    0,   14,   16,   17,   18,   19,
   20,   21,    0,   24,    0,   25,   23,   22,   15,    8,
    0,    0,   31,    0,    0,    0,    0,    0,    0,    0,
   30,    0,    0,   35,   32,    0,    0,    0,   39,    0,
   36,   40,
};
short yydgoto[] = {                                       5,
   16,    6,    7,    8,    9,   10,   11,   12,   35,   25,
   36,   37,   38,   39,   40,   41,   42,   52,   45,   56,
   61,   57,   62,
};
short yysindex[] = {                                   -262,
 -245, -245, -245, -245,    0, -262,    0,    0,    0, -245,
 -245, -222,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0, -257,    0, -240, -250, -251,    0,    0, -243,
    0,    0, -237, -235, -240,    0,    0,    0,    0,    0,
    0,    0, -232,    0, -245,    0,    0,    0,    0,    0,
 -245, -255,    0, -245, -245, -241, -249, -245, -245, -245,
    0, -236, -245,    0,    0, -245, -245, -245,    0, -245,
    0,    0,
};
short yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,   38,    0,    0,    0,   54,
   59,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   64,    0,    0,    0,    0,    0,
    0,    0,   69,    0,    0,    0,    0,    0,    0,    0,
    0,    1,    0,    0,    0,   29,   15,    0,    0,    0,
    0,   43,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,
};
short yygindex[] = {                                      0,
    7,    0,   35,    0,    0,    0,    0,    0,    0,    0,
    9,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,
};
#define YYTABLESIZE 337
short yytable[] = {                                      26,
   33,    1,    2,    3,   54,    4,   43,   44,   17,   18,
   19,   13,   14,   15,   34,   46,   21,   22,   24,   55,
   27,   47,   28,   48,   50,   63,   29,   60,   37,   30,
   31,   32,   33,   34,   13,   14,   15,    1,   67,   23,
   20,    0,   38,   49,    0,    0,    0,    0,    0,    0,
    0,   51,    0,    6,    0,    0,    0,   53,    7,    0,
   58,   59,    0,   13,   64,   65,   66,    0,    9,   68,
    0,    0,   69,   70,   71,    0,   72,    0,    0,    0,
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
    0,   33,    0,   33,   33,   33,   33,   33,   33,   33,
   33,   33,   33,   33,   33,   34,    0,   34,   34,   34,
   34,   34,   34,   34,   34,   34,   34,   34,   34,   37,
    0,   37,   37,   37,   37,   37,   37,    0,   37,   37,
   37,   37,   37,   38,    0,   38,   38,   38,   38,   38,
   38,    0,   38,   38,   38,   38,   38,    6,    6,    6,
    0,    6,    7,    7,    7,    0,    7,   13,   13,   13,
    0,   13,    9,    9,    9,    0,    9,
};
short yycheck[] = {                                     257,
    0,  264,  265,  266,  260,  268,  257,  259,    2,    3,
    4,  257,  258,  259,    0,  259,   10,   11,   12,  275,
  261,  259,  263,  259,  257,  275,  267,  269,    0,  270,
  271,  272,  273,  274,  257,  258,  259,    0,  275,  262,
    6,   -1,    0,   35,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   45,   -1,    0,   -1,   -1,   -1,   51,    0,   -1,
   54,   55,   -1,    0,   58,   59,   60,   -1,    0,   63,
   -1,   -1,   66,   67,   68,   -1,   70,   -1,   -1,   -1,
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
   -1,  261,   -1,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  261,   -1,  263,  264,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  261,
   -1,  263,  264,  265,  266,  267,  268,   -1,  270,  271,
  272,  273,  274,  261,   -1,  263,  264,  265,  266,  267,
  268,   -1,  270,  271,  272,  273,  274,  264,  265,  266,
   -1,  268,  264,  265,  266,   -1,  268,  264,  265,  266,
   -1,  268,  264,  265,  266,   -1,  268,
};
#define YYFINAL 5
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 275
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"INTEGER","STRING","FLOAT",
"CAP_MATCH","CAP_UPPER_BOUND","COLON","COMMON_POINT","NET","NET_CAP_MATCH",
"NET_RES_MATCH","NOISY","PATH","RES_MATCH","RES_UPPER_BOUND","SENSITIVE",
"SHIELDING","TIMING","MAX_VOLTAGE_DROP","COMMA",
};
char *yyrule[] = {
"$accept : start_nets",
"start_nets : critical_data",
"critical_data : path_or_netdata",
"critical_data : critical_data path_or_netdata",
"path_or_netdata : single_path",
"path_or_netdata : net_data",
"path_or_netdata : net_cap_match",
"path_or_netdata : net_res_match",
"single_path : pathlist COLON INTEGER INTEGER INTEGER",
"single_path : pathlist COLON INTEGER INTEGER",
"pathlist : PATH string",
"pathlist : pathlist string",
"$$1 :",
"net_data : NET string $$1 net_param_list",
"net_param_list : net_option",
"net_param_list : net_param_list net_option",
"net_option : timing",
"net_option : cap_upper_bound",
"net_option : res_upper_bound",
"net_option : analog_type",
"net_option : voltage_drop",
"net_option : common_point",
"voltage_drop : MAX_VOLTAGE_DROP FLOAT",
"timing : TIMING FLOAT",
"cap_upper_bound : CAP_UPPER_BOUND FLOAT",
"res_upper_bound : RES_UPPER_BOUND FLOAT",
"analog_type : NOISY",
"analog_type : SENSITIVE",
"analog_type : SHIELDING",
"$$2 :",
"common_point : COMMON_POINT $$2 pt_list cap_match res_match",
"pt_list : string string",
"pt_list : pt_list COMMA string string",
"cap_match :",
"cap_match : cap_list",
"cap_list : CAP_MATCH string string",
"cap_list : cap_list COMMA string string",
"res_match :",
"res_match : res_list",
"res_list : RES_MATCH string string",
"res_list : res_list COMMA string string",
"net_cap_match : NET_CAP_MATCH string",
"net_cap_match : net_cap_match string",
"net_res_match : NET_RES_MATCH string",
"net_res_match : net_res_match string",
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

#include "readnets_l.h"
/* add readnets_l.h for debug purposes */
/* ********************* #include "readnets_l.h" *******************/
/* ********************* #include "readnets_l.h" *******************/

readnets( fp )
FILE *fp ;
{ 
#ifdef YYDEBUG
    extern int yydebug ;
    yydebug = FALSE ;
#endif


    yyin = fp ;
    line_countS = 0 ;
    init_nets() ;
    /* parse input file using yacc if input given */
    if( fp ){
	yyparse();  
    }
    cleanup_nets() ;

} /* end readnets */

yyerror(s)
char    *s;
{
    extern char *cktNameG ;

    sprintf(YmsgG,"problem reading %s.mnet:", cktNameG );
    M( ERRMSG, "yacc", YmsgG ) ;
    sprintf(YmsgG, "line %d near '%s' : %s\n" ,
	line_countS+1, yytext, s );
    M( ERRMSG,NULL, YmsgG ) ;
    Ymessage_error_count() ;
    set_net_error() ;
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
case 8:
{
			/* pathlist COLON lowerBound upperBound priority*/
			end_path( yyvsp[-2].ival, yyvsp[-1].ival, yyvsp[0].ival ) ;
		    }
break;
case 9:
{
			end_path( yyvsp[-1].ival, yyvsp[0].ival, 1 ) ;
		    }
break;
case 10:
{
			add_path( STARTPATH, yyvsp[0].string ) ; 
		    }
break;
case 11:
{
			add_path( CONTPATH, yyvsp[0].string ) ; 
		    }
break;
case 12:
{ init_analog( yyvsp[0].string ) ; }
break;
case 22:
{
			set_max_voltage_drop( yyvsp[0].fval ) ;
		    }
break;
case 24:
{
			set_cap_upper_bound( yyvsp[0].fval ) ;
		    }
break;
case 25:
{
			set_res_upper_bound( yyvsp[0].fval ) ;
		    }
break;
case 26:
{
			set_net_type( NOISY_NET ) ;
			
		    }
break;
case 27:
{
			set_net_type( SENSITIVE_NET ) ;
			
		    }
break;
case 28:
{
			set_net_type( SHIELDING_NET ) ;
			
		    }
break;
case 29:
{ add_common_pt(); }
break;
case 31:
{
			add2common( yyvsp[-1].string, yyvsp[0].string ) ;
		    }
break;
case 32:
{
			add2common( yyvsp[-1].string, yyvsp[0].string ) ;
		    }
break;
case 35:
{
			common_cap( yyvsp[-1].string, yyvsp[0].string ) ;
		    }
break;
case 36:
{
			common_cap( yyvsp[-1].string, yyvsp[0].string ) ;
		    }
break;
case 39:
{
			common_res( yyvsp[-1].string, yyvsp[0].string ) ;
		    }
break;
case 40:
{
			common_res( yyvsp[-1].string, yyvsp[0].string ) ;
		    }
break;
case 41:
{
			start_net_capmatch( yyvsp[0].string ) ;
		    }
break;
case 42:
{
			add_net_capmatch( yyvsp[0].string ) ;
		    }
break;
case 43:
{
			start_net_resmatch( yyvsp[0].string ) ;
		    }
break;
case 44:
{
			add_net_resmatch( yyvsp[0].string ) ;
		    }
break;
case 45:
{ 
			yyval.string = yyvsp[0].string ;	
		    }
break;
case 46:
{
			/* convert integer to string */
			/* this allows integers to be used as strings */
			/* a kluge but timberwolf's old parser supported it */
			sprintf( bufferS,"%d", yyvsp[0].ival ) ;
			/* now clone string */
			yyval.string = (char *) Ystrclone( bufferS ) ;
		    }
break;
case 47:
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
