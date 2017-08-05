%define api.prefix readtiles_
%glr-parser
%code top{
#include <globals.h>
#include "compact.h"
#include "io.h"
#define yyget_lineno readtiles_get_lineno
#define yytext readtiles_text
#define yyin readtiles_in
extern char *yytext;
extern int yyget_lineno(void);
static int nodeS ;          /* current node */
int readtiles_error(char *s);
char *readtiles_lex();
}

%union {
	int ival;
	float fval;
	char *sval;
}

%token B
%token CELL
%token L
%token NUMCELLS
%token NUMTILES
%token OFFSET
%token R
%token STDCELL
%token T
%token X
%token Y

%token COLON
%token INTEGER
%token FLOAT
%token STRING

%type<ival> INTEGER
%type<fval> FLOAT
%type<sval> STRING

%start start_file
%%
start_file : tile_info cell_list;
tile_info : NUMTILES COLON INTEGER NUMCELLS COLON INTEGER
{
	init($3,$6);
};
cell_list : cell;
cell_list : cell_list cell;
cell : cell_info tile_list
{
	endCell();
};
cell_info : CELL INTEGER X COLON INTEGER Y COLON INTEGER OFFSET COLON INTEGER INTEGER
{
	initCell( CELLTYPE, $2, $5, $8, $11, $12) ;
};
cell_info : STDCELL INTEGER X COLON INTEGER Y COLON INTEGER OFFSET COLON INTEGER INTEGER
{
	initCell( STDCELLTYPE, $2, $5, $8, $11, $12) ;
};
tile_list : tile;
tile_list : tile_list tile;
tile : L COLON INTEGER R COLON INTEGER B COLON INTEGER T COLON INTEGER
{
	addtile($3, $6, $9, $12 );
};

%%

int yyerror(char *s) {
	printf("error: %s at %s, line %d\n", s, yytext, yyget_lineno());
}

int readtiles(char *filename)
{ 
	extern FILE *yyin;
	printf("%s: opening %s \n",__FUNCTION__,filename);
	yyin = fopen(filename,"r");
	if(yyin) {
		//init();
		/* parse input file using yacc */
		yyparse();
	}
} /* end readcells */

int readtiles_wrap(void) {return 1;}
