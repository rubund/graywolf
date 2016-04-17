/* ----------------------------------------------------------------- 
FILE:	    readcgraph.h
DESCRIPTION:This file redefines global variable of yacc and lex so
	    that we can have more than one parser in TimberWolfMC.
CONTENTS:   macro redefinitions for graph parser in detail router..
DATE:	    Aug  7, 1988 
REVISIONS:  Thu Mar  7 01:41:07 EST 1991 - added more definitions
		for byacc.
----------------------------------------------------------------- */
/* *****************************************************************
"@(#) readcgraph.h (Yale) version 1.2 3/7/91"
***************************************************************** */

#define yyact		DRG_yyact
#define yyback		DRG_yyback
#define yybgin		DRG_yybgin
#define yychar		DRG_yychar
#define yychk		DRG_yychk
#define yycrank		DRG_yycrank
#define yydebug		DRG_yydebug
#define yydef		DRG_yydef
#define yyerrflag	DRG_yyerrflag
#define yyerror	        DRG_yyerror
#define yyestate	DRG_yyestate
#define yyexca		DRG_yyexca
#define yyextra		DRG_yyextra
#define yyfnd		DRG_yyfnd
#define yyin		DRG_yyin
#define yyinput		DRG_yyinput
#define yyleng		DRG_yyleng
#define yylex		DRG_yylex
#define yylineno	DRG_yylineno
#define yylook		DRG_yylook
#define yylsp		DRG_yylsp
#define yylstate	DRG_yylstate
#define yylval		DRG_yylval
#define yymatch		DRG_yymatch
#define yymorfg		DRG_yymorfg
#define yynerrs		DRG_yynerrs
#define yyolsp		DRG_yyolsp
#define yyout		DRG_yyout
#define yyoutput	DRG_yyoutput
#define yypact		DRG_yypact
#define yyparse		DRG_yyparse
#define yypgo		DRG_yypgo
#define yyprevious	DRG_yyprevious
#define yyreds		DRG_yyreds
#define yyr1		DRG_yyr1
#define yyr2		DRG_yyr2
#define yysbuf		DRG_yysbuf
#define yysptr		DRG_yysptr
#define yysvec		DRG_yysvec
#define yytchar		DRG_yytchar
#define yytext		DRG_yytext
#define yytoks		DRG_yytoks
#define yytop		DRG_yytop
#define yyunput		DRG_yyunput
#define yyv		DRG_yyv
#define yyval		DRG_yyval
#define yyvstop		DRG_yyvstop
#define yywrap		DRG_yywrap
/* for byacc */
#define yyrule          DRG_yyrule
#define yyname          DRG_yyname
#define yytable         DRG_yytable
#define yycheck         DRG_yycheck
#define yydgoto         DRG_yydgoto
#define yydefred        DRG_yydefred
#define yygindex        DRG_yygindex
#define yyrindex        DRG_yyrindex
#define yysindex        DRG_yysindex
#define yylen           DRG_yylen
#define yylhs           DRG_yylhs
