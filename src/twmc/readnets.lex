%option prefix="twmc_readnets_"
%top{
#include <globals.h>
#include <allheaders.h>
#include "readnets.y.h"
#include "initnets.h"
extern int yylineno;
#define yylval twmc_readnets_lval
#define yyin twmc_readnets_in
}

blanks		[ \t]+
newline		[\n]+
integer		[0-9]+|[0-9]+[-]
float			({integer}+[.]{integer}*)|({integer}*[.]{integer}+)
string		[a-zA-Z_][a-zA-Z0-9_<>\[\]:.?$/]*

%%
cap_match        return CAP_MATCH;
cap_upper_bound  return CAP_UPPER_BOUND;
common_point     return COMMON_POINT;
max_voltage_drop return MAX_VOLTAGE_DROP;
net              return NET;
net_cap_match    return NET_CAP_MATCH;
net_res_match    return NET_RES_MATCH;
noisy            return NOISY;
path             return PATH;
res_match        return RES_MATCH;
res_upper_bound  return RES_UPPER_BOUND;
sensitive        return SENSITIVE;
shielding        return SHIELDING;
timing           return TIMING;

{newline}+			{yylineno++;}
{blanks}+			{};
{integer}+			{yylval.ival = atoi(yytext); return INTEGER;};
{string}+			{yylval.sval=Ystrclone(yytext); return STRING;}
{float}+			{yylval.fval = atof(yytext); return FLOAT;}
