%define api.prefix {twflow_readobjects_}
%{
#include <stdio.h>
#define yyget_lineno twflow_readobjects_get_lineno
#define yytext twflow_readobjects_text
#define yyin twflow_readobjects_in
extern char *yytext;
extern int yyget_lineno(void);
%}

%union {
	int ival;
	float fval;
	char sval[200];
}

%token ARGS;
%token DRAWN; 
%token EDGE; 
%token FULL; 
%token IFILES; 
%token NUMOBJECTS; 
%token OFILES; 
%token PATH; 
%token POBJECT; 
%token RELATIVE; 

%token COLON
%token INTEGER
%token FLOAT
%token STRING

%type<ival> INTEGER
%type<fval> FLOAT
%type<sval> STRING

%start start_file
%%

start_file : numobjects object_list;
numobjects : NUMOBJECTS INTEGER;
object_list : object;
object_list : object_list object;
object : name path draw_obj list_of_edges;
list_of_edges : edge;
list_of_edges : list_of_edges edge;
edge : edge_keyword ifiles ofiles args;
edge : edge_keyword ifiles ofiles args draw_edges;
name : pname COLON depend_list;
pname : POBJECT string INTEGER;
pname : POBJECT string string INTEGER;
depend_list : INTEGER;
depend_list : depend_list INTEGER;
path : PATH COLON string;
path : PATH COLON;
ifiles : ifiletype;
ifiles : ifiletype list_of_files;
ofiles : ofiletype;
ofiles : ofiletype list_of_files;
ifiletype : IFILES COLON;
ofiletype : OFILES COLON;
list_of_files : string;
list_of_files : list_of_files string;
args : ARGS COLON list_of_args;
list_of_args : string;
list_of_args : list_of_args string;
draw_obj : DRAWN COLON INTEGER INTEGER INTEGER INTEGER;
edge_keyword : EDGE INTEGER COLON;
draw_edges : DRAWN COLON list_of_lines;
list_of_lines : line;
list_of_lines : list_of_lines line;
line : INTEGER INTEGER INTEGER INTEGER;
string : STRING | INTEGER | FLOAT;

%%

int yyerror(char *s) {
	printf("error: %s at %s, line %d\n", s, yytext, yyget_lineno());
}

readobjects( char *filename )
{ 
	extern FILE *yyin;
	printf("Opening %s \n", filename);

	yyin = fopen( filename, "r");
	if(yyin) {
		printf("Opened %s \n", filename);
	} else {
		printf("Error opening %s \n", filename);
	}

	/* parse input file using yacc */
	yyparse();  
	fclose(yyin) ;
	process_arcs() ;
} /* end readobjects */

int twflow_readobjects_wrap(void) { return 1; }

