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

#include "parser_defines.h"

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
EXTERN double *padspaceG ;

#undef EXTERN  

#endif /* PARSER_H */
