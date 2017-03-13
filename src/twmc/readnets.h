/* ----------------------------------------------------------------- 
FILE:	    readnets.h
DESCRIPTION:This file redefines global variable of yacc and lex so
	    that we can have more than one parser in TimberWolfMC.
CONTENTS:   macro redefinitions for parser in readnets.
DATE:	    Oct 19, 1988 
REVISIONS:  Sun Dec 16 00:40:28 EST 1990 - moved net routines to 
		initnets.c and added analog functions.
	    Thu Mar  7 01:48:21 EST 1991 - added more definitions
		for byacc.
----------------------------------------------------------------- */
/* *****************************************************************
   static char SccsId[] = "@(#) readnets.h version 3.6 3/6/92" ;
***************************************************************** */
#include <analog.h>
#define STARTPATH  1   /* flag for start of path */
#define CONTPATH   0   /* flag for continuing path */

#define yyact		NET_yyact
#define yyback		NET_yyback
#define yychar		NET_yychar
#define yychk		NET_yychk
#define yycnprs		NET_yycnprs
#define yydebug		NET_yydebug
#define yydef		NET_yydef
#define yyerrflag	NET_yyerrflag
#define yyestate	NET_yyestate
#define yyexca		NET_yyexca
#define yyfnd		NET_yyfnd
#define yyinput		NET_yyinput
#define yyleng		NET_yyleng
#define yylineno	NET_yylineno
#define yylook		NET_yylook
#define yylsp		NET_yylsp
#define yylstate	NET_yylstate
#define yylval		NET_yylval
#define yymatch		NET_yymatch
#define yymorfg		NET_yymorfg
#define yynerrs		NET_yynerrs
#define yyolsp		NET_yyolsp
#define yyout		NET_yyout
#define yyoutput	NET_yyoutput
#define yypact		NET_yypact
#define yypgo		NET_yypgo
#define yyprevious	NET_yyprevious
#define yyreds		NET_yyreds
#define yyr2		NET_yyr2
#define yysbuf		NET_yysbuf
#define yysptr		NET_yysptr
#define yytchar		NET_yytchar
#define yytoks		NET_yytoks
#define yyunput		NET_yyunput
#define yyv		NET_yyv
#define yyval		NET_yyval
/* for byacc */
#define yyrule          NET_yyrule
#define yyname          NET_yyname
#define yytable         NET_yytable
#define yycheck         NET_yycheck
#define yydgoto         NET_yydgoto
#define yydefred        NET_yydefred
#define yygindex        NET_yygindex
#define yyrindex        NET_yyrindex
#define yysindex        NET_yysindex
#define yylen           NET_yylen
#define yylhs           NET_yylhs

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
