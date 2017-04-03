%define api.prefix {twsc_readnets_}
%glr-parser
%{
#include <stdio.h>
#include "parser_defines.h"

#define yylval twsc_readnets_lval
#define yyget_lineno twsc_readnets_get_lineno
#define yytext twsc_readnets_text
#define yyin twsc_readnets_in
extern char *yytext;
extern int yyget_lineno(void);
%}

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
netlist : string;
netlist : netlist string;
string : STRING;
string : INTEGER;
string : FLOAT;

%%

int yyerror(char *s) {
	printf("error: %s at %s, line %d\n", s, yytext, yyget_lineno());
}

void init_read_nets();
void finish_read_nets();

int readnets(char *filename)
{ 
	extern FILE *yyin;
	printf("readnets: Opening %s \n",filename);
	yyin = fopen(filename,"r");
	/* parse input file using yacc */
	
	if( yyin ) {
		init_read_nets();
		yyparse();
		finish_read_nets();
	}
} /* end readnets */

int twsc_readnets_wrap(void) {return 1;}

