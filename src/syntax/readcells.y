%define api.prefix syntax_
%glr-parser
%code top{
#include <globals.h>
#include "readcells.y.h"
#include "output.h"
#include "syntax.h"
#define yylval syntax_lval
#define yyget_lineno syntax_get_lineno
#define yytext syntax_text
#define yyin syntax_in
extern char *yytext;
extern int yyget_lineno(void);
int syntax_error(char *s);
int syntax_lex();
}

%union {
	int ival;
	float fval;
	char *sval;
}

%token ECO_ADDED_CELL
%token ADDEQUIV
%token APPROXFIXED
%token ASPLB
%token ASPUB
%token AT
%token BLOCK
%token BOTTOM
%token CELL
%token CELLOFFSET
%token CELLGROUP
%token CLASS
%token CONNECT
%token CORNERS
%token CURRENT
%token ENDPINGROUP
%token EQUIV
%token FIXED
%token FROM
%token HARDCELL
%token INITIALLY
%token INSTANCE
%token KEEPOUT
%token LAYER
%token LEFT
%token LEGALBLKCLASS
%token NAME
%token NEIGHBORHOOD
%token NO_LAYER_CHANGE
%token NOMIRROR
%token NONFIXED
%token NOPERMUTE
%token OF
%token ORIENT
%token ORIENTATIONS
%token PAD
%token PADGROUP
%token PERMUTE
%token PIN
%token PINGROUP
%token POWER
%token RESTRICT
%token RIGHT
%token RIGIDFIXED
%token SIDE
%token SIDESPACE
%token SIGNAL
%token SOFTCELL
%token SOFTPIN
%token STDCELL
%token SUPERGROUP
%token SWAPGROUP
%token TOP
%token UNEQUIV

%token INTEGER
%token FLOAT
%token STRING

%type<ival> INTEGER
%type<fval> FLOAT
%type<sval> STRING

%start start_file
%%
start_file : core pads;
start_file : core;
core : corecells;
core : corecells cellgroups;
corecells : coretype;
corecells : corecells coretype;
coretype : hardcell;
coretype : softcell;
coretype : stdcell;
pads : padcells;
pads : padcells padgroups;
padcells : padcell;
padcells : padcells padcell;
padgroups : padgroup;
padgroups : padgroups padgroup;
cellgroups : cellgroup;
cellgroups : cellgroups cellgroup;
stdcell : cellname std_fixed bbox pinlist;
stdcell : cellname optional_list std_fixed bbox pinlist;
optional_list : option;
optional_list : optional_list option;
option : celloffset;
option : eco;
option : swap_group;
option : legal_block_classes;
option : mirror;
option : initial_orient;
hardcell : hardcellname custom_instance_list;
hardcell : hardcellname fixed custom_instance_list;
custom_instance_list : custom_instance;
custom_instance_list : custom_instance_list instance custom_instance;
custom_instance : corners keep_outs class orient hardpins;
custom_instance : corners keep_outs class orient;
softcell : softname soft_instance_list;
softcell : softname fixed soft_instance_list;
soft_instance_list : soft_instance;
soft_instance_list : soft_instance_list instance soft_instance;
soft_instance : corners aspect class orient softpins mc_pingroup;
soft_instance : corners aspect class orient softpins;
soft_instance : corners aspect class orient;
instance : INSTANCE STRING
{
	add_instance();
};
padcell : padname corners cur_orient restriction sidespace hardpins;
padcell : padname corners cur_orient restriction sidespace;
padgroup : padgroupname padgrouplist restriction sidespace;
cellgroup : supergroupname supergrouplist class orient;
cellgroup : cellgroupname neighborhood cellgrouplist;
hardcellname : HARDCELL INTEGER NAME STRING
{
	addCell(HARDCELLTYPE, $4 ) ;
};
softname : SOFTCELL INTEGER NAME STRING
{
	addCell(SOFTCELLTYPE, $4 ) ;
};
cellname : CELL INTEGER STRING
{
	addCell(STDCELLTYPE, $3 );
};
neighborhood : NEIGHBORHOOD INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc;
neighborhood : NEIGHBORHOOD FIXED INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc;
fixed : fixedcontext AT INTEGER FROM xloc INTEGER FROM yloc;
fixed : fixedcontext NEIGHBORHOOD INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc;
fixedcontext : FIXED;
std_fixed :;
std_fixed : initially fixed_type INTEGER FROM fixed_loc OF BLOCK INTEGER;
swap_group : SWAPGROUP STRING;
celloffset : CELLOFFSET offset_list;
offset_list : INTEGER;
offset_list : offset_list INTEGER;
eco : ECO_ADDED_CELL;
legal_block_classes : LEGALBLKCLASS num_block_classes block_classes;
num_block_classes : INTEGER;
block_classes : block_class;
block_classes : block_classes block_class;
block_class : INTEGER;
initial_orient : ORIENT INTEGER;
initially :;
initially : INITIALLY;
fixed_type : FIXED;
fixed_type : NONFIXED;
fixed_type : RIGIDFIXED;
fixed_type : APPROXFIXED;
fixed_loc : LEFT;
fixed_loc : RIGHT;
mirror : NOMIRROR;
bbox : LEFT INTEGER RIGHT INTEGER BOTTOM INTEGER TOP INTEGER
{
	set_bbox( $2, $4, $6, $8) ;
};
xloc : STRING;
yloc : STRING;
padname : PAD INTEGER NAME STRING
{
	addCell(PADCELLTYPE, $4 ) ;
};
padgroupname : PADGROUP STRING PERMUTE;
padgroupname : PADGROUP STRING NOPERMUTE;
supergroupname : SUPERGROUP STRING NAME STRING;
cellgroupname : CELLGROUP STRING NAME STRING;
corners : CORNERS INTEGER cornerpts;
cornerpts : INTEGER INTEGER;
cornerpts : cornerpts INTEGER INTEGER;
class : CLASS INTEGER;
orient : INTEGER ORIENTATIONS orientlist cur_orient;
orient : ORIENTATIONS orientlist cur_orient;
orientlist : INTEGER;
orientlist : orientlist INTEGER;
cur_orient :;
cur_orient : ORIENT INTEGER;
aspect : ASPLB FLOAT ASPUB FLOAT;
softpins : softtype;
softpins : softpins softtype;
softtype : pintype;
softtype : softpin;
hardpins : pintype;
hardpins : hardpins pintype;
pinlist :;
pinlist : stdgrppins;
stdgrppins : std_grppintype;
stdgrppins : stdgrppins std_grppintype;
stdpins : std_pintype;
stdpins : stdpins std_pintype;
std_grppintype : pinrecord;
std_grppintype : pinrecord equiv_list;
std_grppintype : pinrecord unequiv_list;
std_grppintype : pingroup;
std_pintype : pinrecord;
std_pintype : pinrecord equiv_list;
std_pintype : pinrecord unequiv_list;
pintype : pinrecord;
pintype : pinrecord equiv_list;
pingroup : PINGROUP stdpins ENDPINGROUP;
softpin : softpin_info siderestriction;
softpin : softpin_info siderestriction softequivs;
softpin_info : SOFTPIN NAME STRING SIGNAL STRING opt_layer;
pinrecord : required_pinfo contour current power no_layer_change;
required_pinfo : PIN NAME STRING SIGNAL STRING layer
{
	set_pinname( $3 ) ;
	addNet( $5 ) ;
};
contour : INTEGER INTEGER
{
	start_pt( $1, $2 ) ;
};
contour : num_corners pin_pts
{
	processCorners();
};
num_corners : CORNERS INTEGER;
pin_pts : INTEGER INTEGER
{
	add_pt( $1, $2 ) ;
};
pin_pts : pin_pts INTEGER INTEGER
{
	add_pt( $2, $3 ) ;
};
current :;
current : CURRENT FLOAT;
power :;
power : POWER FLOAT;
no_layer_change :;
no_layer_change : NO_LAYER_CHANGE;
equiv_list : equiv;
equiv_list : equiv_list equiv;
equiv : EQUIV NAME STRING layer contour
{
	addEquiv() ;
};
unequiv_list : unequiv;
unequiv_list : unequiv_list unequiv;
unequiv : UNEQUIV NAME STRING layer contour
{
	addUnEquiv() ;
};
softequivs : mc_equiv;
softequivs : mc_equiv user_equiv_list;
softequivs : user_equiv_list;
mc_equiv : addequiv siderestriction;
addequiv : ADDEQUIV
{
	addEquiv();
};
user_equiv_list : user_equiv;
user_equiv_list : user_equiv_list user_equiv;
user_equiv : equiv_name siderestriction connect;
equiv_name : EQUIV NAME STRING opt_layer;
connect :;
connect : CONNECT;
mc_pingroup : pingroupname pingrouplist siderestriction;
mc_pingroup : mc_pingroup pingroupname pingrouplist siderestriction;
pingroupname : PINGROUP STRING PERMUTE;
pingroupname : PINGROUP STRING NOPERMUTE;
pingrouplist : pinset;
pingrouplist : pingrouplist pinset;
pinset : STRING FIXED;
pinset : STRING NONFIXED;
siderestriction :;
siderestriction : RESTRICT SIDE side_list;
side_list : INTEGER;
side_list : side_list INTEGER;
sidespace :;
sidespace : SIDESPACE FLOAT;
sidespace : SIDESPACE FLOAT FLOAT;
layer : LAYER INTEGER;
opt_layer :;
opt_layer : LAYER INTEGER;
sideplace : STRING;
restriction :;
restriction : RESTRICT SIDE sideplace;
padgrouplist : padset;
padgrouplist : padgrouplist padset;
padset : STRING FIXED;
padset : STRING NONFIXED;
supergrouplist : STRING;
supergrouplist : supergrouplist STRING;
cellgrouplist : STRING;
cellgrouplist : cellgrouplist STRING;
keep_outs :;
keep_outs : keep_out_list;
keep_out_list : keep_out;
keep_out_list : keep_out_list keep_out;
keep_out : KEEPOUT LAYER INTEGER CORNERS keep_pts;
keep_pts : INTEGER INTEGER;
keep_pts : keep_pts INTEGER INTEGER;
%%

int yyerror(char *s) {
	printf("error: %s at %s, line %d\n", s, yytext, yyget_lineno());
}

int readcells(char *filename)
{ 
	extern FILE *yyin;
	yyin = fopen(filename,"r");
	init();
	/* parse input file using yacc */
	yyparse();
	fclose(yyin);
} /* end readcells */

int syntax_wrap(void) {return 1;}
