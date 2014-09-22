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

#undef EXTERN  

#endif /* PARSER_H */
