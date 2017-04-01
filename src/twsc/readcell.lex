%option prefix="twsc_readcell_"
%{
#include "readcell.h"
extern int yylineno;
#define yylval twsc_readcell_lval
#define yyget_lineno twsc_readcell_get_lineno
#define yytext twsc_readcell_text
#define yyin twsc_readcell_in
%}

blanks		[ \t]+
newline		[\n]+
integer		[0-9]+|[-]+[0-9]+
float			({integer}+[.]{integer}*)|({integer}*[.]{integer}+)
string		[_a-zA-Z0-9<>:$!]+|[_a-zA-Z<>:$!]+[.]

%%
ECO_added_cell return ECO_ADDED_CELL;
approximately_fixed return APPROXIMATELY_FIXED;
asplb          return ASPLB;
aspub          return ASPUB;
at             return AT;
block          return BLOCK;
bottom         return BOTTOM;
cell           return CELL;
cell_offset    return CELLOFFSET;
cellgroup      return CELLGROUP;
class          return CLASS;
corners        return CORNERS;
end_pin_group  return ENDPINGROUP;
equiv          return EQUIV;
fixed          return FIXED;
from           return FROM;
group          return GROUP;
hardcell       return HARDCELL;
initially      return INITIALLY;
instance       return INSTANCE;
layer          return LAYER;
left           return LEFT;
legal_block_classes return LEGALBLKCLASS;
name           return NAME;
neighborhood   return NEIGHBORHOOD;
nomirror       return NOMIRROR;
nonfixed       return NONFIXED;
nopermute      return NOPERMUTE;
of             return OF;
orient         return ORIENT;
orientations   return ORIENTATIONS;
pad            return PAD;
padgroup       return PADGROUP;
padside        return PADSIDE;
permute        return PERMUTE;
pin            return PIN;
pin_group      return PINGROUP;
port           return PORT;
restrict       return RESTRICT;
right          return RIGHT;
rigidly_fixed  return RIGIDFIXED;
sequence       return SEQUENCE;
side           return SIDE;
side.restriction   return SIDERESTRICTION;
sidespace      return SIDESPACE;
signal         return SIGNAL;
softcell       return SOFTCELL;
stdcell        return STDCELL;
supergroup     return SUPERGROUP;
swap_group     return SWAPGROUP;
top            return TOP;
unequiv        return UNEQUIV;

{newline}+			{yylineno++;}
{blanks}+			{};
{integer}+			{yylval.ival = atoi(yytext); return INTEGER;};
{string}+			{yylval.sval=Ystrclone(yytext); return STRING;}
{float}+			{yylval.fval = atof(yytext); return FLOAT;}
