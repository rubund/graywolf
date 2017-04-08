%option prefix="twmc_readcells_"
%{
#include "readcells.h"
extern int yylineno;
#define yylval twmc_readcells_lval
#define yyget_lineno twmc_readcells_get_lineno
#define yytext twmc_readcells_text
#define yyin twmc_readcells_in
%}

blanks		[ \t]+
newline		[\n]+
integer		[0-9]+|[-]+[0-9]+
float			({integer}+[.]{integer}*)|({integer}*[.]{integer}+)
string		[a-zA-Z_][a-zA-Z0-9_<>\[\]:.?$/]*

%%
addequiv			return ADDEQUIV;
asplb				return ASPLB;
aspub				return ASPUB;
at				return AT;
cellgroup			return CELLGROUP;
class				return CLASS;
cluster				return CLUSTER;
connect				return CONNECT;
corners				return CORNERS;
current				return CURRENT;
equiv				return EQUIV;
fixed				return FIXED;
from				return FROM;
hardcell			return HARDCELL;
instance			return INSTANCE;
keepout				return KEEPOUT;
layer				return LAYER;
name				return NAME;
neighborhood			return NEIGHBORHOOD;
no_layer_change			return NO_LAYER_CHANGE;
nonfixed			return NONFIXED;
nopermute			return NOPERMUTE;
orient				return ORIENT;
orientations			return ORIENTATIONS;
pad				return PAD;
padgroup 			return PADGROUP;
permute				return PERMUTE;
pin				return PIN;
pin_group			return PINGROUP;
power				return POWER;
restrict			return RESTRICT;
side				return SIDE;
sidespace			return SIDESPACE;
signal				return SIGNAL;
softcell			return SOFTCELL;
softpin				return SOFTPIN;
supergroup			return SUPERGROUP;
timing				return TIMING;

{newline}+			{yylineno++;}
{blanks}+			{};
{integer}+			{yylval.ival = atoi(yytext); return INTEGER;};
{string}+			{yylval.sval = Ystrclone(yytext); return STRING;}
{float}+			{yylval.fval = atof(yytext); return FLOAT;}
