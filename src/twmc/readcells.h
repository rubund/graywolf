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
#define yybgin		CUSTOM_yybgin
#define yychar		CUSTOM_yychar
#define yychk		CUSTOM_yychk
#define yycrank		CUSTOM_yycrank
#define yycnprs		CUSTOM_yycnprs
#define yydebug		CUSTOM_yydebug
#define yydef		CUSTOM_yydef
#define yyerrflag	CUSTOM_yyerrflag
#define yyerror	        CUSTOM_yyerror
#define yyestate	CUSTOM_yyestate
#define yyexca		CUSTOM_yyexca
#define yyextra		CUSTOM_yyextra
#define yyfnd		CUSTOM_yyfnd
#define yyin		CUSTOM_yyin
#define yyinput		CUSTOM_yyinput
#define yyleng		CUSTOM_yyleng
#define yylex		CUSTOM_yylex
#define yylineno	CUSTOM_yylineno
#define yylook		CUSTOM_yylook
#define yylsp		CUSTOM_yylsp
#define yylstate	CUSTOM_yylstate
#define yylval		CUSTOM_yylval
#define yymatch		CUSTOM_yymatch
#define yymorfg		CUSTOM_yymorfg
#define yynerrs		CUSTOM_yynerrs
#define yyolsp		CUSTOM_yyolsp
#define yyout		CUSTOM_yyout
#define yyoutput	CUSTOM_yyoutput
#define yypact		CUSTOM_yypact
#define yyparse		CUSTOM_yyparse
#define yypgo		CUSTOM_yypgo
#define yyprevious	CUSTOM_yyprevious
#define yyreds		CUSTOM_yyreds
#define yyr1		CUSTOM_yyr1
#define yyr2		CUSTOM_yyr2
#define yysbuf		CUSTOM_yysbuf
#define yysptr		CUSTOM_yysptr
#define yysvec		CUSTOM_yysvec
#define yytchar		CUSTOM_yytchar
#define yytext		CUSTOM_yytext
#define yytoks		CUSTOM_yytoks
#define yytop		CUSTOM_yytop
#define yyunput		CUSTOM_yyunput
#define yyv		CUSTOM_yyv
#define yyval		CUSTOM_yyval
#define yyvstop		CUSTOM_yyvstop
#define yywrap		CUSTOM_yywrap
