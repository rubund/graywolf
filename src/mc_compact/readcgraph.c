/*
 *   Copyright (C) 1988-1990 Yale University
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
FILE:	    readcgraph.c <- readcgraph.y <- readcgraph.l
DESCRIPTION:This file contains the grammar (BNF) for reading the channel
	    graph.  The rules for lex are in readcgraph.l.
	    The grammar for yacc is in readcgraph.y.
	    The output of yacc (y.tab.c) is renamed to readcgraph.c
CONTENTS:   
	    readcgraph( fp )
		FILE *fp ;
	    yyerror(s)
		char    *s;
	    yywrap()
	
	    Note:readcgraph.c will also include the yacc parser.
DATE:	    Apr  7, 1988 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) readcgraph.y (Yale) version 1.1 2/23/91" ;
#endif

#include <string.h>
#include <compact.h>
#include <readcgraph.h>       /* redefine yacc and lex globals */
#include <yalecad/message.h>  /* use message routines for errors. */
#include <yalecad/debug.h>    /* use debug utilities. */
#include <yalecad/file.h>     /* use message routines for errors. */

#undef REJECT          /* undefine TWMC macro for lex's version */ 
#define YYDEBUG  1     /* condition compile for yacc debug */

static INT line_countS ;
static char bufferS[LRECL] ;
static INT  nodeS ;          /* current node */
static BOOL abortS = FALSE ;

typedef union {
    INT ival ;
    char *string ;
    DOUBLE fval ;
} YYSTYPE;
#define INTEGER 257
#define STRING 258
#define FLOAT 259
#define NUMNODES 260
#define NUMEDGES 261
#define ADJ 262
#define NODE 263
#define LEN 264
#define CAP 265
#define COLON 266
#define X 267
#define Y 268
#define L 269
#define B 270
#define R 271
#define T 272
#define HNOTV 273
#define LB 274
#define RT 275
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    1,    2,    2,    3,    4,    5,    5,    6,
};
short yylen[] = {                                         2,
    2,    6,    1,    2,    2,   21,    1,    2,   31,
};
short yydefred[] = {                                      0,
    0,    0,    0,    0,    0,    0,    3,    0,    0,    0,
    4,    0,    0,    7,    0,    0,    0,    8,    0,    0,
    0,    2,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    6,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    9,
};
short yydgoto[] = {                                       2,
    3,    6,    7,    8,   13,   14,
};
short yysindex[] = {                                   -260,
 -264,    0, -259, -254, -261, -259,    0, -256, -253, -250,
    0, -252, -256,    0, -257, -255, -251,    0, -247, -249,
 -244,    0, -243, -248, -246, -245, -242, -239, -238, -240,
 -241, -237, -236, -234, -231, -235, -233, -230, -229, -226,
 -225, -232, -228, -223, -222, -224, -217, -221, -227, -220,
 -219, -216, -209, -218,    0, -215, -207, -214, -213, -205,
 -212, -211, -201, -210, -208, -200,    0,
};
short yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,   20,    0,    0,    0,    0,
    0,    0,    1,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
    0,    0,   21,    0,    0,   22,
};
#define YYTABLESIZE 264
short yytable[] = {                                       1,
    5,    4,    9,    5,   10,   12,   16,   15,   19,   22,
   17,   20,   24,   25,   21,   26,   23,   30,   31,    1,
   28,   27,   36,   29,   32,   37,   11,   33,   34,   35,
   42,   43,   48,   38,   18,   40,   41,   39,   44,   49,
   54,   45,   46,   47,   51,   52,   53,   55,   50,   58,
   57,   61,   60,   56,   63,   64,   67,   66,   59,    0,
    0,   62,    0,    0,   65,    0,    0,    0,    0,    0,
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
    0,    0,    0,    5,
};
short yycheck[] = {                                     260,
    0,  266,  257,  263,  266,  262,  257,  261,  266,  257,
  263,  267,  257,  257,  266,  264,  266,  257,  257,    0,
  266,  268,  257,  266,  265,  257,    6,  269,  266,  266,
  257,  257,  257,  269,   13,  266,  266,  271,  271,  257,
  257,  270,  266,  266,  272,  266,  266,  257,  270,  257,
  266,  257,  266,  272,  266,  257,  257,  266,  273,   -1,
   -1,  274,   -1,   -1,  275,   -1,   -1,   -1,   -1,   -1,
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
   -1,   -1,   -1,  263,
};
#define YYFINAL 2
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
"NUMNODES","NUMEDGES","ADJ","NODE","LEN","CAP","COLON","X","Y","L","B","R","T",
"HNOTV","LB","RT",
};
char *yyrule[] = {
"$accept : start_file",
"start_file : graph_size node_list",
"graph_size : NUMNODES COLON INTEGER NUMEDGES COLON INTEGER",
"node_list : node",
"node_list : node_list node",
"node : node_info adjacency_list",
"node_info : NODE COLON INTEGER X COLON INTEGER Y COLON INTEGER L COLON INTEGER R COLON INTEGER B COLON INTEGER T COLON INTEGER",
"adjacency_list : adjacent_node",
"adjacency_list : adjacency_list adjacent_node",
"adjacent_node : ADJ NODE COLON INTEGER LEN COLON INTEGER CAP COLON INTEGER L COLON INTEGER R COLON INTEGER B COLON INTEGER T COLON INTEGER HNOTV COLON INTEGER LB COLON INTEGER RT COLON INTEGER",
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

#include "readcgraph_l.h"
/* add readcgraph_l.h for debug purposes */
/* ********************* #include "readcgraph_l.h" *******************/
/* ********************* #include "readcgraph_l.h" *******************/


read_cgraph()
{ 
    char filename[LRECL] ;
#ifdef YYDEBUG
    extern int yydebug ;
    yydebug = FALSE ;
#endif

    sprintf( filename, "%s.mrte", cktNameG ) ;
    yyin = TWOPEN( filename, "r", ABORT ) ;
    line_countS = 0 ;
    /* parse input file using yacc */
    yyparse();  
    if( abortS ){
	YexitPgm( PGMFAIL ) ;
    }

} /* end read_cgraph */

yyerror(s)
char    *s;
{
    sprintf(YmsgG,"problem reading %s.mrte:", cktNameG );
    M( ERRMSG, "yacc", YmsgG ) ;
    sprintf(YmsgG, "  line %d near '%s' : %s\n" ,
	line_countS+1, yytext, s );
    M( MSG,"yacc", YmsgG ) ;
    abortS = TRUE ;
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
case 2:
{
			init_graph( yyvsp[-3].ival, yyvsp[0].ival ) ;
		    }
break;
case 6:
{
			/* node:1 x:2 y:4 l:0 r:9 b:0 t:8 */
			/* node number, center and size */
			nodeS = yyvsp[-18].ival ;
			addNode( yyvsp[-18].ival, yyvsp[-15].ival, yyvsp[-12].ival ) ;
		    }
break;
case 9:
{ 
/* ex.  adj node:2 len:32 cap:14 l:-89 r:-63 b:0 t:6 HnotV:1 lb:0 rt:0 */
			addEdge( nodeS, yyvsp[-27].ival,     /* nodeS, node */
			    yyvsp[-6].ival, yyvsp[-3].ival, yyvsp[0].ival ) ;/* HnotV, lbcell, rtcelll */
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
