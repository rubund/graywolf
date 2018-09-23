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

#ifdef PARSER_VARS
#define EXTERN 
#else
#define EXTERN extern 
#endif

/* cell types */
#define STDCELLTYPE       1
#define EXCEPTTYPE        2
#define PADTYPE           3
#define PORTTYPE          4
#define EXTRATYPE         5
#define PADGROUPTYPE      6
#define HARDCELLTYPE      7
/* pin types */
#define PINTYPE           1
#define PASS_THRU         2
#define SWAP_PASS         3

EXTERN SWAPBOX *swap_group_listG ;
EXTERN BOOL one_pin_feedthruG ;

EXTERN INT maxCellOG ;
EXTERN INT case_unequiv_pinG ;
EXTERN INT celllenG ;
EXTERN INT cells_per_clusterG ;
EXTERN INT cluster_widthG ;
EXTERN INT extra_cellsG ;
EXTERN INT *fixLRBTG ;
EXTERN INT last_pin_numberG ;
EXTERN INT num_clustersG ;
EXTERN BOOL swappable_gates_existG ;

EXTERN INT swap_netG ;
EXTERN INT totallenG ;
EXTERN DOUBLE *padspaceG ;

/* called from readcell.c */
void add_extra_cells(void);
void end_padgroup(void);
void addCell(char *cellname, INT celltype);
void fix_placement(char *fixed_type, INT from, char *fixed_loc, INT block);
void add_swap_group(char *swap_name);
void add_legal_blocks(INT block_class);
void add_eco(void);
void add_initial_orient(INT orient);
void set_mirror_flag(void);
void add_tile(INT left, INT bottom, INT right, INT top);
void add_orient(INT orient);
void add_padside(char *padside);
void set_old_format(char *padside);
void setPermutation(int permuteFlag);
void process_corners(void);
void init_corners(void);
void add_corner(INT x, INT y);
void add_pingroup(void);
void end_pingroup(void);
void add_pin(char *pin_name, char *signal, INT layer, INT xpos, INT ypos);
void add_equiv(char *equiv_name, INT layer, INT eq_xpos, INT eq_ypos, BOOL unequiv_flag);
void add_port(char *portname, char *signal, int layer, INT xpos, INT ypos );
void add_sidespace(DOUBLE lower, DOUBLE upper);
void add2padgroup(char *padName, BOOL ordered);

#undef EXTERN  

#endif /* PARSER_H */
