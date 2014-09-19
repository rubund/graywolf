#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.8 (Berkeley) 01/20/90";
#endif
#define YYBYACC 1
/* ----------------------------------------------------------------- 
FILE:	    convert.c <- convert.y <- convert.l
DESCRIPTION:This file contains the grammar (BNF) for the TimberWolfMC
	    input file parser. The rules for lex are in convert.l.
	    The grammar for yacc is in convert.y.
	    The output of yacc (y.tab.c) is renamed to convert.c
CONTENTS:   
	    convert( fp )
		FILE *fp ;
	    yyerror(s)
		char    *s;
	    yywrap()
	
	    Note:convert.c will also include the yacc parser.
DATE:	    Apr 27, 1988 - original coding.
REVISIONS:  Jun 19, 1989 - Added macro conversion.
	    Dec  3, 1990 - Abort on an error now.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) convert.y version 1.7 12/7/90" ;
#endif

#include <yalecad/base.h>
#include <yalecad/message.h>
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
#endif /* LEXDEBUG */
#endif /* DEBUG */

static char bufferS[LRECL] ;
static BOOL firstTimeS = TRUE ;
static BOOL pad_contextS = FALSE ; /* switch allow us to have B,L,T */
static BOOL abortS = FALSE ;       /* only abort on error */
    /* as signal names. See readcells.l to see how it is used */

typedef union {
    INT ival ;
    char *string ;
    double fval ;
} YYSTYPE;
#define INTEGER 257
#define STRING 258
#define FLOAT 259
#define B 260
#define L 261
#define R 262
#define T 263
#define MUL 264
#define MUR 265
#define MLL 266
#define MLR 267
#define ML 268
#define MR 269
#define MTT 270
#define MBB 271
#define MB 272
#define MT 273
#define MM 274
#define BOTTOM 275
#define EQUIV 276
#define LAYER 277
#define LEFT 278
#define NAME 279
#define ORIENT 280
#define PAD 281
#define PADSIDE 282
#define PIN 283
#define RIGHT 284
#define SIDESPACE 285
#define SIGNAL 286
#define TOP 287
#define UNEQUIV 288
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    2,    2,    3,    3,    4,    6,    7,    7,    7,
    7,    7,    7,    7,    7,    7,    7,    7,    7,    7,
    7,    7,    8,    5,    5,   10,   10,   11,   11,   12,
   12,   13,   13,    9,    9,    1,    1,    1,
};
short yylen[] = {                                         2,
    1,    1,    2,    2,    1,    4,    5,    2,    2,    2,
    2,    2,    2,    2,    2,    2,    2,    2,    2,    2,
    2,    2,    8,    1,    2,    1,    2,    7,    9,    1,
    2,    5,    7,    0,    2,    1,    1,    1,
};
short yydefred[] = {                                      0,
    0,    0,    0,    2,    0,    0,    0,    3,    0,    0,
   24,    0,    0,    0,   37,   36,   38,    0,    0,   25,
    0,    0,   30,    8,    9,   10,   11,   12,   13,   14,
   15,   16,   17,   18,   19,   21,   20,   22,    0,    0,
    0,    0,    0,   31,    0,    0,    6,    7,    0,    0,
    0,   35,    0,    0,    0,    0,    0,    0,   32,    0,
    0,   28,    0,    0,    0,    0,   33,    0,   29,   23,
};
short yydgoto[] = {                                       2,
   18,    3,    4,    5,   10,    6,   14,   40,   47,   11,
   12,   22,   23,
};
short yysindex[] = {                                   -277,
 -245,    0, -277,    0, -270, -268, -248,    0, -264, -270,
    0, -260, -229, -261,    0,    0,    0, -262, -248,    0,
 -259, -260,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -238, -263,
 -236, -258, -248,    0, -255, -234,    0,    0, -248, -254,
 -231,    0, -250, -211, -210, -227, -208, -207,    0, -206,
 -205,    0, -204, -203, -232, -201,    0, -200,    0,    0,
};
short yyrindex[] = {                                      0,
    0,    0,   58,    0,    6,    0,    0,    0,    0,    8,
    0,    1,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    2,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    5,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
  -19,    0,   56,    0,    0,    0,    0,    0,    0,   50,
    0,    0,   39,
};
#define YYTABLESIZE 289
short yytable[] = {                                      42,
   26,   27,   54,    1,   34,    5,   57,    4,   15,   16,
   17,    7,    9,   13,   19,   21,   39,   41,   45,   43,
   48,   46,   55,   50,   52,   56,   58,   49,   51,   53,
   24,   25,   26,   27,   28,   29,   30,   31,   32,   33,
   34,   35,   36,   37,   38,   59,   60,   61,   62,   63,
   64,   65,   66,   67,   68,   69,   70,    1,    8,   20,
   44,    0,    0,    0,    0,    0,    0,    0,    0,    0,
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
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   26,   27,   26,   27,   34,    5,   34,    4,
};
short yycheck[] = {                                      19,
    0,    0,  257,  281,    0,    0,  257,    0,  257,  258,
  259,  257,  283,  282,  279,  276,  278,  280,  257,  279,
  257,  285,  277,   43,  259,  257,  277,  286,  284,   49,
  260,  261,  262,  263,  264,  265,  266,  267,  268,  269,
  270,  271,  272,  273,  274,  257,  257,  275,  257,  257,
  257,  257,  257,  257,  287,  257,  257,    0,    3,   10,
   22,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
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
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  281,  281,  283,  283,  281,  281,  283,  281,
};
#define YYFINAL 2
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 288
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"INTEGER","STRING","FLOAT","B",
"L","R","T","MUL","MUR","MLL","MLR","ML","MR","MTT","MBB","MB","MT","MM",
"BOTTOM","EQUIV","LAYER","LEFT","NAME","ORIENT","PAD","PADSIDE","PIN","RIGHT",
"SIDESPACE","SIGNAL","TOP","UNEQUIV",
};
char *yyrule[] = {
"$accept : start_file",
"start_file : padcells",
"padcells : padcell",
"padcells : padcells padcell",
"padcell : padrecord stdpins",
"padcell : padrecord",
"padrecord : old_padname old_pad_side bbox sidespace",
"old_padname : PAD INTEGER string ORIENT INTEGER",
"old_pad_side : PADSIDE B",
"old_pad_side : PADSIDE L",
"old_pad_side : PADSIDE R",
"old_pad_side : PADSIDE T",
"old_pad_side : PADSIDE MUL",
"old_pad_side : PADSIDE MUR",
"old_pad_side : PADSIDE MLL",
"old_pad_side : PADSIDE MLR",
"old_pad_side : PADSIDE ML",
"old_pad_side : PADSIDE MR",
"old_pad_side : PADSIDE MTT",
"old_pad_side : PADSIDE MBB",
"old_pad_side : PADSIDE MT",
"old_pad_side : PADSIDE MB",
"old_pad_side : PADSIDE MM",
"bbox : LEFT INTEGER RIGHT INTEGER BOTTOM INTEGER TOP INTEGER",
"stdpins : std_pintype",
"stdpins : stdpins std_pintype",
"std_pintype : pinrecord",
"std_pintype : pinrecord equiv_list",
"pinrecord : PIN NAME string SIGNAL string INTEGER INTEGER",
"pinrecord : PIN NAME string SIGNAL string LAYER INTEGER INTEGER INTEGER",
"equiv_list : equiv",
"equiv_list : equiv_list equiv",
"equiv : EQUIV NAME string INTEGER INTEGER",
"equiv : EQUIV NAME string LAYER INTEGER INTEGER INTEGER",
"sidespace :",
"sidespace : SIDESPACE FLOAT",
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

#include "convert_l.h"
/* add convert_l.h for debug purposes */
/* ********************* #include "convert_l.h" *******************/
/* ********************* #include "convert_l.h" *******************/

convert( fp )
FILE *fp ;
{ 
#ifdef YYDEBUG
    extern int yydebug ;
    yydebug = FALSE ;
#endif

    yyin = fp ;
    /* parse input file using yacc */
    yyparse();  
    process_pad_groups() ;
    if( abortS ){
	YexitPgm( PGMFAIL ) ;
    }

} /* end readcells */

yyerror(s)
char    *s;
{
    sprintf(YmsgG,"problem reading %s.cel:", cktNameG );
    M( ERRMSG, "yacc", YmsgG ) ;
    sprintf(YmsgG, "  line %d near '%s' : %s\n" ,
	lineG+1, yytext, s );
    M( MSG,"yacc", YmsgG ) ;
    abortS = TRUE ;
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
			process_side_space() ;
		    }
break;
case 7:
{
			pad_contextS = TRUE ;
			addpad( yyvsp[-3].ival, yyvsp[-2].string, yyvsp[0].ival ) ;
		    }
break;
case 8:
{
			pad_contextS = FALSE ;
			padside(PADB) ;
		    }
break;
case 9:
{
			pad_contextS = FALSE ;
			padside(PADL) ;
		    }
break;
case 10:
{
			pad_contextS = FALSE ;
			padside(PADR) ;
		    }
break;
case 11:
{
			pad_contextS = FALSE ;
			padside(PADT) ;
		    }
break;
case 12:
{
			pad_contextS = FALSE ;
			padside(PMUL) ;
		    }
break;
case 13:
{
			pad_contextS = FALSE ;
			padside(PMUR) ;
		    }
break;
case 14:
{
			pad_contextS = FALSE ;
			padside(PMLL) ;
		    }
break;
case 15:
{
			pad_contextS = FALSE ;
			padside(PMLR) ;
		    }
break;
case 16:
{
			pad_contextS = FALSE ;
			padside(PML) ;
		    }
break;
case 17:
{
			pad_contextS = FALSE ;
			padside(PMR) ;
		    }
break;
case 18:
{
			pad_contextS = FALSE ;
			padside(PMTT) ;
		    }
break;
case 19:
{
			pad_contextS = FALSE ;
			padside(PMBB) ;
		    }
break;
case 20:
{
			pad_contextS = FALSE ;
			padside(PMT) ;
		    }
break;
case 21:
{
			pad_contextS = FALSE ;
			padside(PMB) ;
		    }
break;
case 22:
{
			pad_contextS = FALSE ;
			padside(PMM) ;
		    }
break;
case 23:
{
			add_bbox( yyvsp[-6].ival, yyvsp[-4].ival, yyvsp[-2].ival, yyvsp[0].ival ) ;
		    }
break;
case 28:
{
			if( firstTimeS ){
			    fprintf( stderr, "No layer information found. Defaulting to layer 1\n\n" ) ;
			    firstTimeS = FALSE ;
			}
			calc_orientation( &(yyvsp[-1].ival), &(yyvsp[0].ival) ) ;
			fprintf( fpoG, "pin name %s signal %s layer 1 %d %d\n",
			    yyvsp[-4].string, yyvsp[-2].string, yyvsp[-1].ival, yyvsp[0].ival ) ;
			Ysafe_free( yyvsp[-4].string ) ;
			Ysafe_free( yyvsp[-2].string ) ;
		    }
break;
case 29:
{
			calc_orientation( &(yyvsp[-1].ival), &(yyvsp[0].ival) ) ;
			fprintf( fpoG, 
			    "pin name %s signal %s layer %d %d %d\n",
			    yyvsp[-6].string, yyvsp[-4].string, yyvsp[-2].ival, yyvsp[-1].ival, yyvsp[0].ival ) ;
			Ysafe_free( yyvsp[-6].string ) ;
			Ysafe_free( yyvsp[-4].string ) ;
		    }
break;
case 32:
{
			calc_orientation( &(yyvsp[-1].ival), &(yyvsp[0].ival) ) ;
			fprintf( fpoG, 
			    "equiv name %s layer 1 %d %d\n",
			    yyvsp[-2].string, yyvsp[-1].ival, yyvsp[0].ival ) ;
			Ysafe_free( yyvsp[-2].string ) ;
		    }
break;
case 33:
{
			calc_orientation( &(yyvsp[-1].ival), &(yyvsp[0].ival) ) ;
			fprintf( fpoG, 
			    "equiv name %s layer %d %d %d\n",
			    yyvsp[-4].string, yyvsp[-2].ival, yyvsp[-1].ival, yyvsp[0].ival ) ;
			Ysafe_free( yyvsp[-4].string ) ;
		    }
break;
case 35:
{
			add_space( yyvsp[0].fval ) ;
		    }
break;
case 36:
{ 
			yyval.string = yyvsp[0].string ;	
		    }
break;
case 37:
{
			/* convert integer to string */
			/* this allows integers to be used as strings */
			/* a kluge but timberwolf's old parser supported it */
			sprintf( bufferS,"%d", yyvsp[0].ival ) ;
			/* now clone string */
			yyval.string = Ystrclone( bufferS ) ;
		    }
break;
case 38:
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
