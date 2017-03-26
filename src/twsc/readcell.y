%define api.prefix {twsc_readcell_}
%{
#include <stdio.h>
#define yylval twsc_readcell_lval
#define yyget_lineno twsc_readcell_get_lineno
#define yytext twsc_readcell_text
#define yyin twsc_readcell_in
extern char *yytext;
extern int yyget_lineno(void);
%}

%union {
	int ival;
	float fval;
	char sval[200];
}

%token ADDEQUIV
%token ASPLB
%token ASPUB
%token AT
%token CELLGROUP
%token CLASS
%token CLUSTER
%token CONNECT
%token CORNERS
%token CURRENT
%token EQUIV
%token FIXED
%token FROM
%token HARDCELL
%token INSTANCE
%token KEEPOUT
%token LAYER
%token NAME
%token NEIGHBORHOOD
%token NO_LAYER_CHANGE
%token NONFIXED
%token NOPERMUTE
%token ORIENT
%token ORIENTATIONS
%token PAD
%token PADGROUP
%token PERMUTE
%token PIN
%token PINGROUP
%token POWER
%token RESTRICT
%token SIDE
%token SIDESPACE
%token SIGNAL
%token SOFTCELL
%token SOFTPIN
%token SUPERGROUP
%token TIMING

%token INTEGER
%token FLOAT
%token STRING

%type<ival> INTEGER
%type<fval> FLOAT
%type<sval> STRING

%start start_file
%%

start_file: core pads;
start_file: core;

core: corecells;
core: corecells cellgroups;

corecells: coretype;
corecells: corecells coretype;

coretype: customcell;
coretype: softcell;

pads: padcells;
pads: padcells padgroups;

padcells: padcell;
padcells: padcells padcell;

padgroups: padgroup;
padgroups: padgroups padgroup;

cellgroups: cellgroup;
cellgroups: cellgroups cellgroup;

customcell: cellname custom_instance_list;
customcell: cellname fixed custom_instance_list;

custom_instance_list: custom_instance;
custom_instance_list: custom_instance_list instance custom_instance;

custom_instance: corners keep_outs class orient hardpins;
custom_instance: corners keep_outs class orient;

softcell: softname soft_instance_list;
softcell: softname fixed soft_instance_list;

soft_instance_list : soft_instance;
soft_instance_list : soft_instance_list instance soft_instance;

soft_instance : corners aspect keep_outs class orient softpins pingroup;
soft_instance : corners aspect keep_outs class orient softpins;
soft_instance : corners aspect keep_outs class orient;

instance : INSTANCE string;

padcell : padname corners cur_orient restriction sidespace hardpins;
padcell : padname corners cur_orient restriction sidespace;

padgroup : padgroupname padgrouplist restriction sidespace;

cellgroup : supergroupname supergrouplist class orient;
cellgroup : cellgroupname neighborhood cellgrouplist;

cellname : HARDCELL string NAME string;
cellname : HARDCELL error;

softname : SOFTCELL string NAME string;
softname : CLUSTER INTEGER NAME string;
softname : SOFTCELL error;

neighborhood : NEIGHBORHOOD INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc;
neighborhood : NEIGHBORHOOD FIXED INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc;

fixed : fixedcontext AT INTEGER FROM xloc INTEGER FROM yloc;
fixed : fixedcontext NEIGHBORHOOD INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc;

fixedcontext : FIXED;

xloc : STRING;

yloc : STRING;

padname : PAD string NAME string;

padgroupname : PADGROUP string PERMUTE;
padgroupname : PADGROUP string NOPERMUTE;
padgroupname : PADGROUP error;

supergroupname : SUPERGROUP string NAME string;
supergroupname : SUPERGROUP error;

cellgroupname : CELLGROUP string NAME string;
cellgroupname : CELLGROUP error;

corners : numcorners cornerpts;

numcorners : CORNERS INTEGER;

cornerpts : INTEGER INTEGER;
cornerpts : cornerpts INTEGER INTEGER;

class: CLASS INTEGER;
class: CLASS error;

orient : numorientations ORIENTATIONS orientlist cur_orient;
orient : ORIENTATIONS orientlist cur_orient;

numorientations : INTEGER;

orientlist : INTEGER;
orientlist : orientlist INTEGER;

cur_orient :;
cur_orient : ORIENT INTEGER;

aspect : ASPLB FLOAT ASPUB FLOAT;

softpins : softpinlist;

softpinlist : softtype;
softpinlist : softpinlist softtype;

softtype : pintype;
softtype : softpin;

hardpins : pintype;
hardpins : hardpins pintype;

pintype : pinrecord;
pintype : pinrecord equiv_list;

pinrecord : required_pinfo contour timing current power no_layer_change;

required_pinfo : PIN NAME string SIGNAL string layer;

contour : INTEGER INTEGER;
contour : num_corners pin_pts;

num_corners : CORNERS INTEGER;

pin_pts : INTEGER INTEGER;
pin_pts : pin_pts INTEGER INTEGER;

current :;
current : CURRENT FLOAT;

power :;
power : POWER FLOAT;

no_layer_change :;
no_layer_change : NO_LAYER_CHANGE;

softpin : softpin_info siderestriction pinspace;
softpin : softpin_info siderestriction pinspace softequivs;

softpin_info : SOFTPIN NAME string SIGNAL string layer timing;

softequivs : mc_equiv;
softequivs : mc_equiv user_equiv_list;
softequivs : user_equiv_list;

mc_equiv : addequiv siderestriction;

addequiv : ADDEQUIV;

user_equiv_list : user_equiv;
user_equiv_list : user_equiv_list user_equiv;

user_equiv : equiv_name siderestriction connect;

equiv_name : EQUIV NAME string layer;

connect :;
connect : CONNECT;

pingroup : pingroupname pingrouplist siderestriction pinspace;
pingroup : pingroup pingroupname pingrouplist siderestriction pinspace;

pingroupname : PINGROUP string PERMUTE;
pingroupname : PINGROUP string NOPERMUTE;

pingrouplist : pinset;
pingrouplist : pingrouplist pinset;

pinset : string FIXED;
pinset : string NONFIXED;

equiv_list : equiv;
equiv_list : equiv_list equiv;

equiv : EQUIV NAME string layer INTEGER INTEGER;

layer :;
layer : LAYER INTEGER;

siderestriction :;
siderestriction : RESTRICT SIDE side_list;

side_list : INTEGER;
side_list : side_list INTEGER;

pinspace :;
pinspace : SIDESPACE FLOAT;
pinspace : SIDESPACE FLOAT FLOAT;

sidespace :;
sidespace : SIDESPACE FLOAT;
sidespace : SIDESPACE FLOAT FLOAT;

sideplace : STRING;
sideplace : error;

restriction :;
restriction : RESTRICT SIDE sideplace;
padgrouplist : padset;
padgrouplist : padgrouplist padset;
padset : string FIXED;
padset : string NONFIXED;
supergrouplist : string;
supergrouplist : supergrouplist string;
cellgrouplist : string;
cellgrouplist : cellgrouplist string;
keep_outs :;
keep_outs : keep_out_list;
keep_out_list : keep_out;
keep_out_list : keep_out_list keep_out;
keep_out : KEEPOUT LAYER INTEGER CORNERS keep_pts;
keep_pts : INTEGER INTEGER;
keep_pts : keep_pts INTEGER INTEGER;
timing: TIMING FLOAT;
timing: ;
string: STRING | INTEGER | FLOAT;

%%

int yyerror(char *s) {
	printf("error: %s at %s, line %d\n", s, yytext, yyget_lineno());
}

int readcell(char *filename)
{ 
	extern FILE *yyin;
	printf("readcells: Opening %s \n",filename);
	yyin = fopen(filename,"r");
	//init();
	/* parse input file using yacc */
	yyparse();
} /* end readcells */

int twsc_readcell_wrap(void) {return 1;}

