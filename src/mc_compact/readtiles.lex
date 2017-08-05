%option prefix="readtiles_"
%top{
#include <globals.h>
#include "readtiles.y.h"
#define yylval readtiles_lval
#define yytext readtiles_text
#define yyin readtiles_in
extern int yylineno;
}

blanks		[ \t]+
newline		[\n]+
integer		[0-9]+|[-]+[0-9]+
float			({integer}+[.]{integer}*)|({integer}*[.]{integer}+)
string		[a-zA-Z0-9_<>\[\]:.?$/]*[a-zA-Z_][a-zA-Z0-9_<>\[\]:.?$/]*
colon		[:]

%%
b                   return B;
cell                return CELL;
l                   return L;
numcells            return NUMCELLS;
numtiles            return NUMTILES;
offset              return OFFSET;
r                   return R;
stdcell             return STDCELL;
t                   return T;
x                   return X;
y                   return Y;

{newline}+			{yylineno++;}
{blanks}+			{};
{integer}+			{yylval.ival = atoi(yytext); return INTEGER;};
{string}+			{yylval.sval=Ystrclone(yytext); return STRING;}
{float}+			{yylval.fval = atof(yytext); return FLOAT;}
{colon}			{return COLON;}
