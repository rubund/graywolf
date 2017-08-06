%define api.prefix twflow_readobjects_
%glr-parser
%code top{
#include <globals.h>
#include "twflow.h"
#include "io.h"
#define yyget_lineno twflow_readobjects_get_lineno
#define yytext twflow_readobjects_text
#define yyin twflow_readobjects_in
extern char *yytext;
extern int yyget_lineno(void);
int twflow_readobjects_error(char *s);
char *twflow_readobjects_lex();
}

%union {
	int ival;
	float fval;
	char *sval;
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

numobjects : NUMOBJECTS INTEGER
{
	init( $2 );
};

pname : POBJECT STRING INTEGER
{
	add_object( $2, $3);
};

depend_list : INTEGER
{
	add_pdependency($1);
};

depend_list : depend_list INTEGER
{
	add_pdependency($2);
};

path : PATH COLON;
path : PATH COLON STRING
{
	add_path( $3 );
};

name : pname COLON depend_list;

object : name path draw_obj list_of_edges;

object_list : object;
object_list : object_list object;

edge_keyword : EDGE INTEGER COLON {start_edge($2);};

edge : edge_keyword ifiles ofiles args;
edge : edge_keyword ifiles ofiles args draw_edges;

list_of_edges : edge;
list_of_edges : list_of_edges edge;

ifiles : ifiletype
{
	set_file_type( INPUTFILE );
};

ifiles : ifiletype list_of_files;

ofiles : ofiletype
{
	set_file_type( OUTPUTFILE );
};

ofiles : ofiletype list_of_files;
ifiletype : IFILES COLON;
ofiletype : OFILES COLON;
list_of_files : STRING
{
	add_fdependency($1);
};
list_of_files : list_of_files STRING
{
	add_fdependency($2);
};
args : ARGS COLON list_of_args;
list_of_args : STRING;
list_of_args : list_of_args STRING;
draw_obj : DRAWN COLON INTEGER INTEGER INTEGER INTEGER
{
	add_box( $3, $4,$5, $6 );
};

draw_edges : DRAWN COLON list_of_lines;
list_of_lines : line;
list_of_lines : list_of_lines line;

line : INTEGER INTEGER INTEGER INTEGER
{
	add_line($1, $2, $3, $4);
};

%%

int yyerror(char *s) {
	printf("error: %s at %s, line %d\n", s, yytext, yyget_lineno());
}

int readobjects( char *filename )
{ 
	extern FILE *yyin;

	yyin = fopen( filename, "r");

	/* parse input file using yacc */
	yyparse();  
	fclose(yyin) ;
	process_arcs() ;

	return 0;
} /* end readobjects */

int twflow_readobjects_wrap(void) { return 1; }

