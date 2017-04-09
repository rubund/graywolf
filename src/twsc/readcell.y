%define api.prefix {twsc_readcell_}
%glr-parser
%{
#include <stdio.h>
#include <yalecad/base.h>
#include "parser_defines.h"

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
	char *sval;
}

%token ECO_ADDED_CELL
%token APPROXIMATELY_FIXED
%token ASPLB
%token ASPUB
%token AT
%token BLOCK
%token BOTTOM
%token CELL
%token CELLOFFSET
%token CELLGROUP
%token CLASS
%token CORNERS
%token ENDPINGROUP
%token EQUIV
%token FIXED
%token FROM
%token GROUP
%token HARDCELL
%token INITIALLY
%token INSTANCE
%token LAYER
%token LEFT
%token LEGALBLKCLASS
%token NAME
%token NEIGHBORHOOD
%token NOMIRROR
%token NONFIXED
%token NOPERMUTE
%token OF
%token ORIENT
%token ORIENTATIONS
%token PAD
%token PADGROUP
%token PADSIDE
%token PERMUTE
%token PIN
%token PINGROUP
%token PORT
%token RESTRICT
%token RIGHT
%token RIGIDFIXED
%token SEQUENCE
%token SIDE
%token SIDERESTRICTION
%token SIDESPACE
%token SIGNAL
%token SOFTCELL
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

start_file : core macros;
start_file : core macros pads;
start_file : core pads;
start_file : core ports;
start_file : core;
core : corecells;
core : corecells cellgroups;
corecells : stdcell;
corecells : corecells stdcell;
pads : padcells;
pads : padcells padgroups;
padcells : padcell;
padcells : padcells padcell;
macros : macro;
macros : macros macro;
macro : hardcell;
macro : softcell;
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
custom_instance : corners class orient actual_orient hardpins;
custom_instance : corners class orient actual_orient;
softcell : softname soft_instance_list;
softcell : softname fixed soft_instance_list;
soft_instance_list : soft_instance;
soft_instance_list : soft_instance_list instance soft_instance;
soft_instance : corners aspect class orient softpins;
instance : INSTANCE STRING;
padcell : padname corners actual_orient restriction_pad sidespace hardpins;
padcell : padname corners actual_orient restriction_pad sidespace;
padcell : padname_std padside bbox sidespace hardpins;
padcell : padname_std padside bbox sidespace;
padgroup : padgroupname padgrouplist restriction_pdgrp sidespace
{
	end_padgroup() ;
};
cellgroup : supergroupname supergrouplist class orient;
cellgroup : cellgroupname neighborhood cellgrouplist;
hardcellname : HARDCELL INTEGER NAME STRING
{
	addCell($4, HARDCELLTYPE ) ;
};
softname : SOFTCELL INTEGER NAME STRING
{
	addCell($4, SOFTCELLTYPE ) ;
};
cellname : CELL INTEGER STRING
{
	addCell($3, STDCELLTYPE ) ;
};
neighborhood : NEIGHBORHOOD INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc;
neighborhood : NEIGHBORHOOD FIXED INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc;
fixed : fixedcontext AT INTEGER FROM xloc INTEGER FROM yloc;
fixed : fixedcontext NEIGHBORHOOD INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc;
fixedcontext : FIXED;
std_fixed :;
std_fixed : INITIALLY fixed_type INTEGER FROM fixed_loc OF BLOCK INTEGER;
swap_group : SWAPGROUP STRING;
legal_block_classes : LEGALBLKCLASS num_block_classes block_classes;
num_block_classes : INTEGER
{
	add_legal_blocks( $1 ) ;
};
block_classes : block_class;
block_classes : block_classes block_class;
block_class : INTEGER;
eco : ECO_ADDED_CELL
{
	add_eco() ;
};
initial_orient : ORIENT INTEGER
{
	 add_initial_orient($2);
};
celloffset : CELLOFFSET offset_list;
offset_list : INTEGER;
offset_list : offset_list INTEGER;
fixed_type : FIXED;
fixed_type : NONFIXED;
fixed_type : RIGIDFIXED;
fixed_type : APPROXIMATELY_FIXED;
fixed_loc : LEFT;
fixed_loc : RIGHT;
mirror : NOMIRROR;
bbox : LEFT INTEGER RIGHT INTEGER BOTTOM INTEGER TOP INTEGER
{
	add_tile( $2,$6,$4,$8 ) ;
};
xloc : STRING;
yloc : STRING;
padname : PAD INTEGER NAME STRING
{
	addCell( $4, PADTYPE ) ;
};
padname_std : PAD INTEGER STRING ORIENT INTEGER
{
	addCell( $3, PADTYPE ) ;
	add_orient($5);
};
padside : PADSIDE STRING
{
	add_padside( $2 ) ;
	set_old_format( $2 ) ;
};
padgroupname : PADGROUP STRING PERMUTE
{
	addCell( $2, PADGROUPTYPE ) ;
	setPermutation( TRUE ) ;
	add_tile( 0,0,0,0 ) ;
};
padgroupname : PADGROUP STRING NOPERMUTE
{
	addCell( $2, PADGROUPTYPE ) ;
	setPermutation( FALSE ) ;
	add_tile( 0,0,0,0 ) ;
};
supergroupname : SUPERGROUP INTEGER NAME STRING;
cellgroupname : CELLGROUP INTEGER NAME STRING;
corners : num_corners cornerpts
{
	process_corners();
};
num_corners : CORNERS INTEGER
{
	init_corners();
};
cornerpts : cornerpt;
cornerpts : cornerpts cornerpt;
cornerpt : INTEGER INTEGER
{
	add_corner( $1, $2 );
};
class : CLASS INTEGER;
actual_orient :;
actual_orient : ORIENT INTEGER
{
};
orient : INTEGER ORIENTATIONS orientlist;
orient : INTEGER ORIENTATIONS orientlist initial_orient;
orient : ORIENTATIONS orientlist;
orient : ORIENTATIONS orientlist initial_orient;
orientlist : INTEGER
{
	add_orient($1);
};
orientlist : orientlist INTEGER
{
	add_orient($2);
};
aspect : ASPLB FLOAT ASPUB FLOAT
{

};
softpins : softtype;
softpins : softpins softtype;
softtype : pintype;
softtype : pingrouptype;
softtype : sequencetype;
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
std_grppintype : pingroup stdpins endpingroup;
std_pintype : pinrecord;
std_pintype : pinrecord equiv_list;
std_pintype : pinrecord unequiv_list;
pintype : pinrecord;
pintype : pinrecord equiv_list;
pingroup : PINGROUP;
endpingroup : ENDPINGROUP;
pingrouptype : pingroupident softpinlist siderestriction;
pingroupident : GROUP INTEGER;
sequencetype : seqident softpinlist siderestriction;
seqident : SEQUENCE INTEGER;
softpinlist : softpinrecord;
softpinlist : softpinlist softpinrecord;
softpinrecord : PIN NAME STRING SIGNAL STRING;
pinrecord : PIN NAME STRING SIGNAL STRING LAYER INTEGER INTEGER INTEGER
{
	add_pin($3, $5, $7, $8, $9 ) ;
};
pinrecord : PIN NAME STRING SIGNAL STRING INTEGER INTEGER
{
	add_pin($3, $5, 0, $6, $7 ) ;
};
equiv_list : equiv;
equiv_list : equiv_list equiv;
equiv : EQUIV NAME STRING LAYER INTEGER INTEGER INTEGER
{
	add_equiv( $3, $5, $6, $7, TRUE ) ; 
};
equiv : EQUIV NAME STRING INTEGER INTEGER
{
	add_equiv( $3, 0, $4, $5, TRUE ) ; 
};
unequiv_list : unequiv;
unequiv_list : unequiv_list unequiv;
unequiv : UNEQUIV NAME STRING LAYER INTEGER INTEGER INTEGER
{
	add_equiv( $3, $5, $6, $7, FALSE ) ; 
};
unequiv : UNEQUIV NAME STRING INTEGER INTEGER
{
	add_equiv( $3, 0, $4, $5, FALSE ) ; 
};
ports : port;
ports : ports port;
port : PORT NAME STRING SIGNAL STRING LAYER INTEGER INTEGER INTEGER
{
	add_port( $3, $5, $7, $8, $9 ) ;
};
port : PORT NAME STRING SIGNAL STRING INTEGER INTEGER
{
	add_port( $3, $5, 0, $6, $7 ) ;
};
siderestriction : SIDERESTRICTION INTEGER INTEGER;
sidespace :;
sidespace : SIDESPACE FLOAT
{
	add_sidespace( $2, $2 );
};
sidespace : SIDESPACE FLOAT FLOAT
{
	add_sidespace( $2, $3 );
};
restriction_pad :;
restriction_pad : RESTRICT SIDE sideplace;
restriction_pdgrp :;
restriction_pdgrp : RESTRICT SIDE sideplace;
sideplace : STRING;
padgrouplist : padset;
padgrouplist : padgrouplist padset;
padset : STRING FIXED
{
	add2padgroup( $1, TRUE ) ; /* fixed */
};
padset : STRING NONFIXED
{
	add2padgroup( $1, FALSE ) ; /* nonfixed */
};
supergrouplist : STRING;
supergrouplist : supergrouplist STRING;
cellgrouplist : STRING;
cellgrouplist : cellgrouplist STRING;

%%

int yyerror(char *s) {
	printf("error: %s at %s, line %d\n", s, yytext, yyget_lineno());
}

int readcell(char *filename)
{ 
	extern FILE *yyin;
	yyin = fopen(filename,"r");
	/* parse input file using yacc */
	if(yyin) {
		printf("opened %s\n",filename);
		initialize_parser() ;
		yyparse();
		init_net_set();
		cleanup_readcells();
	}
} /* end readcell */

int twsc_readcell_wrap(void) {return 1;}

