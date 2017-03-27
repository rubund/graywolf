%define api.prefix {twmc_readcells_}
%{
#include <stdio.h>
#include "main.h"
#include "initialize.h"
#define yyget_lineno twmc_readcells_get_lineno
#define yytext twmc_readcells_text
#define yyin twmc_readcells_in
#define MAX_CORNERS 200
extern char *yytext;
extern int yyget_lineno(void);
%}

%union {
	int ival;
	float fval;
	char sval[100];
	int *intar;
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

%type<ival> layer
%type<ival> numcorners
%type<intar> cornerpts

%type<ival> num_corners
%type<intar> pin_pts

%start start_file
%%

start_file : core pads;
start_file : core;
core : corecells;
core : corecells cellgroups;
corecells : coretype;
corecells : corecells coretype;
coretype : customcell;
coretype : softcell;
pads : padcells;
pads : padcells padgroups;
padcells : padcell;
padcells : padcells padcell;
padgroups : padgroup;
padgroups : padgroups padgroup;
cellgroups : cellgroup;
cellgroups : cellgroups cellgroup;
customcell : cellname custom_instance_list;
customcell : cellname fixed custom_instance_list;
custom_instance_list : custom_instance;
custom_instance_list : custom_instance_list instance custom_instance;
custom_instance : corners keep_outs class orient hardpins;
custom_instance : corners keep_outs class orient;
softcell : softname soft_instance_list;
softcell : softname fixed soft_instance_list;
soft_instance_list : soft_instance;
soft_instance_list : soft_instance_list instance soft_instance;
soft_instance : corners aspect keep_outs class orient softpins pingroup;
soft_instance : corners aspect keep_outs class orient softpins;
soft_instance : corners aspect keep_outs class orient;
instance : INSTANCE STRING
{
	endCell();
	add_instance(Ystrclone($2)) ;
};
padcell : padname corners cur_orient restriction sidespace hardpins;
padcell : padname corners cur_orient restriction sidespace;
padgroup : padgroupname padgrouplist restriction sidespace;
cellgroup : supergroupname supergrouplist class orient;
cellgroup : cellgroupname neighborhood cellgrouplist;
cellname : HARDCELL INTEGER NAME STRING
{
	addCell(Ystrclone($4), CUSTOMCELLTYPE ) ;
};
cellname : HARDCELL error;
softname : SOFTCELL INTEGER NAME STRING
{
	addCell(Ystrclone($4), SOFTCELLTYPE ) ;
};
softname : CLUSTER INTEGER NAME STRING
{
	addCell(Ystrclone($4), SOFTCELLTYPE ) ;
};
softname : SOFTCELL error;
neighborhood : NEIGHBORHOOD INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc;
neighborhood : NEIGHBORHOOD FIXED INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc;
fixed : fixedcontext AT INTEGER FROM xloc INTEGER FROM yloc;
fixed : fixedcontext NEIGHBORHOOD INTEGER FROM xloc INTEGER FROM yloc INTEGER FROM xloc INTEGER FROM yloc;
fixedcontext : FIXED;
xloc : INTEGER;
yloc : INTEGER;
padname : PAD INTEGER NAME STRING
{
	addCell(Ystrclone($4), PADCELLTYPE ) ;
};
padgroupname : PADGROUP STRING PERMUTE;
padgroupname : PADGROUP STRING NOPERMUTE;
padgroupname : PADGROUP error;
supergroupname : SUPERGROUP STRING NAME STRING;
supergroupname : SUPERGROUP error;
cellgroupname : CELLGROUP STRING NAME STRING;
cellgroupname : CELLGROUP error;
corners: numcorners cornerpts {
	int *r = $2;
	int t=$1;
	int p1=0, p2=0;
	r-=t*2;

	for(int i=0;i<t;i++) {
		p1=*r;
		r++;
		p2=*r;
		r++;
		addCorner(p1, p2) ;
	}
	processCorners(t);
};
numcorners : CORNERS INTEGER
{
	yyval.ival=$2;
};
cornerpts : INTEGER INTEGER
{
	yyval.intar=malloc(MAX_CORNERS);
	*(yyval.intar)=$1;
	yyval.intar++;
	*(yyval.intar)=$2;
	yyval.intar++;
};
cornerpts : cornerpts INTEGER INTEGER
{
	*(yyval.intar)=$2;
	yyval.intar++;
	*(yyval.intar)=$3;
	yyval.intar++;
};
class : CLASS INTEGER
{
	addClass($2) ;
};
class : CLASS error;
orient : numorientations ORIENTATIONS orientlist cur_orient;
orient : ORIENTATIONS orientlist cur_orient;
numorientations : INTEGER
{
	initOrient($1);
};
orientlist : INTEGER
{
	addOrient($1);
};
orientlist : orientlist INTEGER
{
	addOrient($2);
};
cur_orient :;
cur_orient : ORIENT INTEGER
{
	set_cur_orient($2);
};
aspect : ASPLB FLOAT ASPUB FLOAT
{
	addAspectBounds( $2, $4 ) ;
};
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
required_pinfo : PIN NAME STRING SIGNAL STRING layer
{
/* 	addPin( Ystrclone($3), Ystrclone($5),  $6, HARDPINTYPE ); */
};
contour : INTEGER INTEGER;
contour : num_corners pin_pts {
	int *r = $2;
	int t=$1;
	int p1=0, p2=0;
	r-=t*2;

	for(int i=0;i<t;i++) {
		p1=*r;
		r++;
		p2=*r;
		r++;
		addCorner(p1, p2) ;
	}
	processCorners(t);
};
num_corners : CORNERS INTEGER
{
	yyval.ival=$2;
};
pin_pts : INTEGER INTEGER {
	yyval.intar=malloc(MAX_CORNERS);
	*(yyval.intar)=$1;
	yyval.intar++;
	*(yyval.intar)=$2;
	yyval.intar++;
};
pin_pts : pin_pts INTEGER INTEGER {
	*(yyval.intar)=$2;
	yyval.intar++;
	*(yyval.intar)=$3;
	yyval.intar++;
};
current :;
current : CURRENT FLOAT;
power :;
power : POWER FLOAT;
no_layer_change :;
no_layer_change : NO_LAYER_CHANGE;
softpin : softpin_info siderestriction pinspace;
softpin : softpin_info siderestriction pinspace softequivs;
softpin_info : SOFTPIN NAME STRING SIGNAL STRING layer timing
{
	addPin( Ystrclone($3), Ystrclone($5), $6, SOFTPINTYPE );
	set_restrict_type( SOFTPINTYPE ) ;
};
softequivs : mc_equiv;
softequivs : mc_equiv user_equiv_list;
softequivs : user_equiv_list;
mc_equiv : addequiv siderestriction;
addequiv : ADDEQUIV;
user_equiv_list : user_equiv;
user_equiv_list : user_equiv_list user_equiv;
user_equiv : equiv_name siderestriction connect;
equiv_name : EQUIV NAME STRING layer;
connect :;
connect : CONNECT;
pingroup : pingroupname pingrouplist siderestriction pinspace;
pingroup : pingroup pingroupname pingrouplist siderestriction pinspace;
pingroupname : PINGROUP STRING PERMUTE;
pingroupname : PINGROUP STRING NOPERMUTE;
pingrouplist : pinset;
pingrouplist : pingrouplist pinset;
pinset : STRING FIXED;
pinset : STRING NONFIXED;
equiv_list : equiv;
equiv_list : equiv_list equiv;
equiv : EQUIV NAME STRING layer INTEGER INTEGER;
layer :;
layer : LAYER INTEGER
{
	yyval.ival=$2;
};
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
timing :;
timing : TIMING FLOAT;

%%

int yyerror(char *s) {
	printf("error: %s at %s, line %d\n", s, yytext, yyget_lineno());
}

int readcells(char *filename)
{ 
	extern FILE *yyin;
	printf("readcells: Opening %s \n",filename);
	yyin = fopen(filename,"r");
/* 	init(); */
	initCellInfo();
	/* parse input file using yacc */
	if(yyin) {
		yyparse();
	}
} /* end readcells */

int twmc_readcells_wrap(void) {return 1;}

