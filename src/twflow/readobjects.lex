%option prefix="twflow_readobjects_"
%option batch
%option stack
%option pointer
/* %option debug */
%top{
#include <globals.h>
#include "twflow.h"
#include "readobjects.y.h"
extern int yylineno;
#define yylval twflow_readobjects_lval
#define yyin twflow_readobjects_in
#define yyget_lineno twflow_readobjects_get_lineno
#define yytext twflow_readobjects_text
}

blanks		[ \t]+
newline		[\n]+
integer		[0-9]+|[-]+[0-9]+
float			({integer}+[.]{integer}*)|({integer}*[.]{integer}+)
string		[_a-zA-Z0-9<>$]+|[_a-zA-Z<>$]+[.]+
colon		[:]+

%%
args             return ARGS;
drawn            return DRAWN; 
edge             return EDGE; 
full             return FULL; 
ifiles           return IFILES; 
numobjects       return NUMOBJECTS; 
ofiles           return OFILES; 
path             return PATH; 
pobject          return POBJECT; 
relative         return RELATIVE; 

{newline}+			{yylineno++;}
{blanks}+			{};
{integer}+			{yylval.ival = atoi(yytext); return INTEGER;};
{string}+			{yylval.sval = Ystrclone(yytext); return STRING;}
{float}+			{yylval.fval = atof(yytext); return FLOAT;}
{colon}+			{return COLON;}

