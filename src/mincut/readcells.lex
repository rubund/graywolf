%option prefix="mincut_"
%{
#include "readcells.h"
#define yylval mincut_lval
#define yytext mincut_text
#define yyin mincut_in
extern int yylineno;
%}

blanks		[ \t]+
newline		[\n]+
integer		[0-9]+|[-]+[0-9]+
float			({integer}+[.]{integer}*)|({integer}*[.]{integer}+)
string		[_a-zA-Z0-9<>:$]+|[_a-zA-Z<>:$]+[.]

%%
ECO_added_cell    return ECO_ADDED_CELL;
addequiv          return ADDEQUIV;
approximately_fixed return APPROXFIXED;
asplb             return ASPLB;
aspub             return ASPUB;
at                return AT;
block             return BLOCK;
bottom            return BOTTOM;
cell              return CELL;
cell_offset       return CELLOFFSET;
cellgroup         return CELLGROUP;
class             return CLASS;
connect           return CONNECT;
corners           return CORNERS;
current           return CURRENT;
end_pin_group     return ENDPINGROUP;
equiv             return EQUIV;
fixed             return FIXED;
from              return FROM;
hardcell          return HARDCELL;
initially         return INITIALLY;
instance          return INSTANCE;
keepout           return KEEPOUT;
layer             return LAYER;
left              return LEFT;
legal_block_classes return LEGALBLKCLASS;
name              return NAME;
neighborhood      return NEIGHBORHOOD;
no_layer_change   return NO_LAYER_CHANGE;
nomirror          return NOMIRROR;
nonfixed          return NONFIXED;
nopermute         return NOPERMUTE;
of                return OF;
orient            return ORIENT;
orientations      return ORIENTATIONS;
pad               return PAD;
padgroup          return PADGROUP;
permute           return PERMUTE;
pin               return PIN;
pin_group         return PINGROUP;
power             return POWER;
restrict          return RESTRICT;
right             return RIGHT;
rigidly_fixed     return RIGIDFIXED;
side              return SIDE;
sidespace         return SIDESPACE;
signal            return SIGNAL;
softcell          return SOFTCELL;
softpin           return SOFTPIN;
stdcell           return STDCELL;
supergroup        return SUPERGROUP;
swap_group        return SWAPGROUP;
top               return TOP;
unequiv           return UNEQUIV;

{newline}+			{yylineno++;}
{blanks}+			{};
{integer}+			{yylval.ival = atoi(yytext); return INTEGER;};
{string}+			{yylval.sval=Ystrclone(yytext); return STRING;}
{float}+			{yylval.fval = atof(yytext); return FLOAT;}
