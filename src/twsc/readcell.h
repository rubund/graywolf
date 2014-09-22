/* ----------------------------------------------------------------- 
FILE:	    readcell.h
DESCRIPTION:This file redefines global variable of yacc and lex so
	    that we can have more than one parser in TimberWolfSC.
CONTENTS:   macro redefinitions for parser in readcell.
DATE:	    Aug  7, 1988 
REVISIONS:  
----------------------------------------------------------------- */
/* *****************************************************************
   static char SccsId[] = "@(#) readcell.h version 4.2 9/7/90" ;
***************************************************************** */

#define yyact		READCEL_yyact
#define yyback		READCEL_yyback
#define yybgin		READCEL_yybgin
#define yychar		READCEL_yychar
#define yychk		READCEL_yychk
#define yycrank		READCEL_yycrank
#define yydebug		READCEL_yydebug
#define yydef		READCEL_yydef
#define yyerrflag	READCEL_yyerrflag
#define yyerror	        READCEL_yyerror
#define yyestate	READCEL_yyestate
#define yyexca		READCEL_yyexca
#define yyextra		READCEL_yyextra
#define yyfnd		READCEL_yyfnd
#define yyin		READCEL_yyin
#define yyinput		READCEL_yyinput
#define yyleng		READCEL_yyleng
#define yylex		READCEL_yylex
#define yylineno	READCEL_yylineno
#define yylook		READCEL_yylook
#define yylsp		READCEL_yylsp
#define yylstate	READCEL_yylstate
#define yylval		READCEL_yylval
#define yymatch		READCEL_yymatch
#define yymorfg		READCEL_yymorfg
#define yynerrs		READCEL_yynerrs
#define yyolsp		READCEL_yyolsp
#define yyout		READCEL_yyout
#define yyoutput	READCEL_yyoutput
#define yypact		READCEL_yypact
#define yyparse		READCEL_yyparse
#define yypgo		READCEL_yypgo
#define yyprevious	READCEL_yyprevious
#define yyreds		READCEL_yyreds
#define yyr1		READCEL_yyr1
#define yyr2		READCEL_yyr2
#define yysbuf		READCEL_yysbuf
#define yysptr		READCEL_yysptr
#define yysvec		READCEL_yysvec
#define yytchar		READCEL_yytchar
#define yytext		READCEL_yytext
#define yytoks		READCEL_yytoks
#define yytop		READCEL_yytop
#define yyunput		READCEL_yyunput
#define yyv		READCEL_yyv
#define yyval		READCEL_yyval
#define yyvstop		READCEL_yyvstop
#define yywrap		READCEL_yywrap
