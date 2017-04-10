%option prefix="readcgraph_"
%{
#include "readcgraph.h"
#define yylval readcgraph_lval
#define yytext readcgraph_text
#define yyin readcgraph_in
extern int yylineno;
%}

blanks		[ \t]+
newline		[\n]+
integer		[0-9]+|[-]+[0-9]+
float			({integer}+[.]{integer}*)|({integer}*[.]{integer}+)
string		[a-zA-Z0-9_<>\[\]:.?$/]*[a-zA-Z_][a-zA-Z0-9_<>\[\]:.?$/]*
colon		[:]

%%
HnotV               return HNOTV;
adj                 return ADJ;
b                   return B;
cap                 return CAP;
l                   return L;
lb                  return LB;
len                 return LEN;
node                return NODE;
numedges            return NUMEDGES;
numnodes            return NUMNODES;
r                   return R;
rt                  return RT;
t                   return T;
x                   return X;
y                   return Y;

{newline}+			{yylineno++;}
{blanks}+			{};
{integer}+			{yylval.ival = atoi(yytext); return INTEGER;};
{string}+			{yylval.sval=Ystrclone(yytext); return STRING;}
{float}+			{yylval.fval = atof(yytext); return FLOAT;}
{colon}			{return COLON;}
