%define api.prefix twmc_readnets_
%{
#include <globals.h>
#include <allheaders.h>
#define yyget_lineno twmc_readnets_get_lineno
#define yytext twmc_readnets_text
#define yyin twmc_readnets_in
extern char *yytext;
extern int yyget_lineno(void);
int twmc_readnets_error(char *s);
char *twmc_readnets_lex();
%}

%union {
	int ival;
	float fval;
	char *sval;
}

%token CAP_MATCH
%token CAP_UPPER_BOUND
%token COMMON_POINT
%token MAX_VOLTAGE_DROP
%token NET
%token NET_CAP_MATCH
%token NET_RES_MATCH
%token NOISY
%token PATH
%token RES_MATCH
%token RES_UPPER_BOUND
%token SENSITIVE
%token SHIELDING
%token TIMING

%token STARTPATH
%token COMMA
%token COLON
%token INTEGER
%token FLOAT
%token STRING

%type<ival> INTEGER
%type<fval> FLOAT
%type<sval> STRING

%start start_nets
%%

start_nets : critical_data;
critical_data : path_or_netdata;
critical_data : critical_data path_or_netdata;
path_or_netdata : single_path;
path_or_netdata : net_data;
path_or_netdata : net_cap_match;
path_or_netdata : net_res_match;
single_path : pathlist COLON INTEGER INTEGER INTEGER;
single_path : pathlist COLON INTEGER INTEGER;
pathlist : PATH string;
pathlist : pathlist string;
dollar1 :;
net_data : NET string dollar1 net_param_list;
net_param_list : net_option;
net_param_list : net_param_list net_option;
net_option : timing;
net_option : cap_upper_bound;
net_option : res_upper_bound;
net_option : analog_type;
net_option : voltage_drop;
net_option : common_point;
voltage_drop : MAX_VOLTAGE_DROP FLOAT;
timing : TIMING FLOAT;
cap_upper_bound : CAP_UPPER_BOUND FLOAT;
res_upper_bound : RES_UPPER_BOUND FLOAT;
analog_type : NOISY;
analog_type : SENSITIVE;
analog_type : SHIELDING;
dollar2 :;
common_point : COMMON_POINT dollar2 pt_list cap_match res_match;
pt_list : string string;
pt_list : pt_list COMMA string string;
cap_match :;
cap_match : cap_list;
cap_list : CAP_MATCH string string;
cap_list : cap_list COMMA string string;
res_match :;
res_match : res_list;
res_list : RES_MATCH string string;
res_list : res_list COMMA string string;
net_cap_match : NET_CAP_MATCH string;
net_cap_match : net_cap_match string;
net_res_match : NET_RES_MATCH string;
net_res_match : net_res_match string;
string: STRING | INTEGER | FLOAT;

%%

int yyerror(char *s) {
	printf("error: %s at %s, line %d\n", s, yytext, yyget_lineno());
}

int readnets(char *filename)
{ 
	extern FILE *yyin;
	yyin = fopen(filename,"r") ;

	if( yyin ){
		init_nets() ;
		/* parse input file using yacc if input given */
		yyparse();  
	}
	cleanup_nets() ;

} /* end readnets */

int twmc_readnets_wrap(void) {return 1;}

