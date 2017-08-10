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

extern SWAPBOX *swap_group_listG ;

extern int maxCellOG ;
extern int case_unequiv_pinG ;
extern int celllenG ;
extern int cells_per_clusterG ;
extern int cluster_widthG ;
extern int extra_cellsG ;
extern int *fixLRBTG ;
extern int last_pin_numberG ;
extern int num_clustersG ;
extern int swap_netG ;
extern int totallenG ;
extern double *padspaceG ;
extern int totalRG ;
extern int spacer_widthG ;
extern int *spacer_feedsG ;
extern int total_row_lengthG ;
extern int approximately_fixed_factorG ;
extern int ECOs_existG;
extern int orig_max_row_lengthG ;

extern BOOL fences_existG ;
extern BOOL turn_off_checksG ;
extern BOOL spacer_name_twfeedG ;
extern BOOL rigidly_fixed_cellsG ;
extern BOOL stand_cell_as_gate_arrayG ;
extern BOOL one_pin_feedthruG ;
extern BOOL swappable_gates_existG ;

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
