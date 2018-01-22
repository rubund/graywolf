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

EXTERN int maxCellOG ;
EXTERN int case_unequiv_pinG ;
EXTERN int celllenG ;
EXTERN int cells_per_clusterG ;
EXTERN int cluster_widthG ;
EXTERN int extra_cellsG ;
EXTERN int *fixLRBTG ;
EXTERN int last_pin_numberG ;
EXTERN int num_clustersG ;
EXTERN BOOL swappable_gates_existG ;

EXTERN int swap_netG ;
EXTERN int totallenG ;
EXTERN DOUBLE *padspaceG ;

#undef EXTERN  

#endif /* PARSER_H */
