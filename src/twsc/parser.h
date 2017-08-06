/* ----------------------------------------------------------------- 
"@(#) parser.h (Yale) version 4.5 9/7/90"
FILE:	    parser.h                                       
DESCRIPTION:definitions for parsing.
CONTENTS:   
DATE:	    Dec  8, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef PARSER_H

#define PARSER_H

#include "parser_defines.h"
#include "standard.h"

SWAPBOX *swap_group_listG ;
BOOL one_pin_feedthruG ;

int maxCellOG ;
int case_unequiv_pinG ;
int celllenG ;
int cells_per_clusterG ;
int cluster_widthG ;
int extra_cellsG ;
int *fixLRBTG ;
int last_pin_numberG ;
int num_clustersG ;
BOOL swappable_gates_existG ;

int swap_netG ;
int totallenG ;
double *padspaceG ;

void add_legal_blocks( int block_class );
void addCell( char *cellname, int celltype );
void add_tile( int left, int bottom, int right, int top );
void add_initial_orient( int orient );
void add_pin( char *pin_name, char *signal, int layer, int xpos, int ypos );
void end_padgroup();
void init_corners();
void add_corner( int x, int y );
void process_corners();
void add2padgroup( char *padName, BOOL ordered );
void add_sidespace( double lower, double upper );
void add_port( char *portname, char *signal, int layer, int xpos, int ypos );
void add_equiv( char *equiv_name, int layer, int eq_xpos, int eq_ypos, BOOL unequiv_flag );
void add_eco();
void add_orient( int orient );
void add_padside( char *padside );
void set_old_format( char *padside );
static void layer_test();
static void check_pin();
void setPermutation( BOOL permuteFlag );
void initialize_parser();
void cleanup_readcells();

#endif /* PARSER_H */
