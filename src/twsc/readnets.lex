%option prefix="twsc_readnets_"
%top{
#include <globals.h>
#include "parser.h"
#include "readnets.y.h"

extern int yylineno;
#define yylval twsc_readnets_lval
#define yyget_lineno twsc_readnets_get_lineno
#define yytext twsc_readnets_text
#define yyin twsc_readnets_in
}

blanks		[ \t]+
newline		[\n]+
integer		[0-9]+|[-]+[0-9]+
float			({integer}+[.]{integer}*)|({integer}*[.]{integer}+)
string		[a-zA-Z_][a-zA-Z0-9_<>\[\]:.?$/]*

%%

HVweights return HVWEIGHTS;
allnets return ALLNETS;
do_not_global_route return DONTGLOBALROUTE;
ignore return IGNORE;
net return NET;
path return PATH;

{newline}+			{yylineno++;}
{blanks}+			{};
{integer}+			{yylval.ival = atoi(yytext); return INTEGER;};
{string}+			{yylval.sval=Ystrclone(yytext); return STRING;}
{float}+			{yylval.fval = atof(yytext); return FLOAT;}
