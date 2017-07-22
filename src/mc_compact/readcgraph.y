%define api.prefix readcgraph_
%glr-parser
%{
#define yyget_lineno readcgraph_get_lineno
#define yytext readcgraph_text
#define yyin readcgraph_in
extern char *yytext;
extern int yyget_lineno(void);
static int nodeS ;          /* current node */
%}

%union {
	int ival;
	float fval;
	char *sval;
}

%token HNOTV
%token ADJ
%token B
%token CAP
%token L
%token LB
%token LEN
%token NODE
%token NUMEDGES
%token NUMNODES
%token R
%token RT
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
start_file : graph_size node_list;
graph_size : NUMNODES COLON INTEGER NUMEDGES COLON INTEGER
{
	init_graph( $3, $6 ) ;
};
node_list : node;
node_list : node_list node;
node : node_info adjacency_list;
node_info : NODE COLON INTEGER X COLON INTEGER Y COLON INTEGER L COLON INTEGER R COLON INTEGER B COLON INTEGER T COLON INTEGER
{
	nodeS = $3;
	addNode($3, $6, $9 ) ;
};
adjacency_list : adjacent_node;
adjacency_list : adjacency_list adjacent_node;
adjacent_node : ADJ NODE COLON INTEGER LEN COLON INTEGER CAP COLON INTEGER L COLON INTEGER R COLON INTEGER B COLON INTEGER T COLON INTEGER HNOTV COLON INTEGER LB COLON INTEGER RT COLON INTEGER
{
	addEdge( nodeS, $4, $25, $28, $31);
};

%%

int yyerror(char *s) {
	printf("error: %s at %s, line %d\n", s, yytext, yyget_lineno());
}

int read_cgraph(char *filename)
{ 
	extern FILE *yyin;
	printf("%s: opening %s \n",__FUNCTION__,filename);
	yyin = fopen(filename,"r");
	if(yyin) {
		init();
		/* parse input file using yacc */
		yyparse();
	}
} /* end readcells */

int readcgraph_wrap(void) {return 1;}
