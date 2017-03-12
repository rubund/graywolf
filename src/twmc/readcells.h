/* ----------------------------------------------------------------- 
FILE:	    custom_parser.h
DESCRIPTION:This file redefines global variable of yacc and lex so
	    that we can have more than one parser in TimberWolfMC.
CONTENTS:   macro redefinitions for parser in readcells.
DATE:	    Aug  7, 1988 
REVISIONS:  
----------------------------------------------------------------- */
/* *****************************************************************
   static char SccsId[] = "@(#) readcells.h version 3.4 3/6/92" ;
***************************************************************** */

#define yyact		CUSTOM_yyact
#define yyback		CUSTOM_yyback
#define yychar		CUSTOM_yychar
#define yychk		CUSTOM_yychk
#define yycnprs		CUSTOM_yycnprs
#define yydebug		CUSTOM_yydebug
#define yydef		CUSTOM_yydef
#define yyerrflag	CUSTOM_yyerrflag
#define yyestate	CUSTOM_yyestate
#define yyexca		CUSTOM_yyexca
#define yyextra		CUSTOM_yyextra
#define yyfnd		CUSTOM_yyfnd
#define yyinput		CUSTOM_yyinput
#define yyleng		CUSTOM_yyleng
#define yylineno	CUSTOM_yylineno
#define yylook		CUSTOM_yylook
#define yylsp		CUSTOM_yylsp
#define yylstate	CUSTOM_yylstate
#define yymatch		CUSTOM_yymatch
#define yymorfg		CUSTOM_yymorfg
#define yynerrs		CUSTOM_yynerrs
#define yyolsp		CUSTOM_yyolsp
#define yyout		CUSTOM_yyout
#define yyoutput	CUSTOM_yyoutput
#define yypact		CUSTOM_yypact
#define yypgo		CUSTOM_yypgo
#define yyprevious	CUSTOM_yyprevious
#define yyreds		CUSTOM_yyreds
#define yyr1		CUSTOM_yyr1
#define yyr2		CUSTOM_yyr2
#define yysbuf		CUSTOM_yysbuf
#define yysptr		CUSTOM_yysptr
#define yytchar		CUSTOM_yytchar
#define yytoks		CUSTOM_yytoks
#define yyunput		CUSTOM_yyunput
#define yyv		CUSTOM_yyv
#define yyval		CUSTOM_yyval
#define yyvstop		CUSTOM_yyvstop
#define yywrap		CUSTOM_yywrap

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
