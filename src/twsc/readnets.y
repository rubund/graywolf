%define api.prefix twsc_readnets_
%glr-parser
%code top{
#include <globals.h>
#include "parser.h"
#include "readnets_functions.h"

#define yylval twsc_readnets_lval
#define yyget_lineno twsc_readnets_get_lineno
#define yytext twsc_readnets_text
#define yyin twsc_readnets_in
extern char *twsc_readnets_text;
extern FILE *twsc_readnets_in;
extern int yyget_lineno(void);
int twsc_readnets_error(char *s);
char twsc_readnets_lex();
}

%union {
	int ival;
	float fval;
	char *sval;
}

%token HVWEIGHTS
%token ALLNETS
%token DONTGLOBALROUTE
%token IGNORE
%token NET
%token PATH

%token STARTPATH
%token CONTPATH

%token INTEGER
%token FLOAT
%token STRING
%token COLON

%type<ival> INTEGER
%type<fval> FLOAT
%type<sval> STRING

%start start_nets
%%
start_nets :;
start_nets : list_of_paths_n_nets;
list_of_paths_n_nets : path_or_net;
list_of_paths_n_nets : list_of_paths_n_nets path_or_net;
path_or_net : single_path;
path_or_net : net_record net_options;
net_options : list_of_options;
net_options : net_options list_of_options;
list_of_options : IGNORE
{
	ignore_net() ;
};
list_of_options : DONTGLOBALROUTE
{

};
net_record : NET STRING
{
	process_net_rec( $2 ) ;
};
single_path : pathlist COLON INTEGER INTEGER INTEGER
{

};
single_path : pathlist COLON INTEGER INTEGER
{

};
pathlist : PATH netlist;
netlist : STRING;
netlist : netlist STRING;

%%

int twsc_readnets_error(char *s) {
	printf("%s error: %s at %s, line %d\n", __FUNCTION__, s, yytext, yyget_lineno());
	fclose(yyin);
	YexitPgm(PGMFAIL);
}

int readnets(char *filename)
{
	yyin = fopen(filename,"r");
	/* parse input file using yacc */
	if( yyin ) {
		init_read_nets();
		yyparse();
		finish_read_nets();
		fclose(yyin);
	}

} /* end readnets */

int twsc_readnets_wrap(void) {return 1;}

