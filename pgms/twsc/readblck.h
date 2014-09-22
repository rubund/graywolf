/* ----------------------------------------------------------------- 
"@(#) readblck.h (Yale) version 4.2 9/7/90"
FILE:	    readblck.h                                       
DESCRIPTION:TimberwolfSC insert file for reading block file.
CONTENTS:   
DATE:	    Mar 27, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef READBLCK_H
#define READBLCK_H

#ifdef READBLCK_VARS
#define EXTERN 
#else
#define EXTERN extern 
#endif

typedef struct exceptbox {
    INT row ;
    INT ll_x ;
    INT ll_y ;
    INT ur_x ;
    INT ur_y ;
} *EXCEPTPTR, EXCEPTBOX ;

/* global variable definitions */
EXTERN INT num_exceptsG ;
EXTERN INT top_of_top_rowG ;
EXTERN INT bot_of_bot_rowG ;
EXTERN INT uniform_rowsG ;
EXTERN INT individual_rowSepsG ;
EXTERN INT total_except_widthG ;
EXTERN DOUBLE *rowSepsG ;
EXTERN INT *rowSepsAbsG ;
EXTERN DOUBLE *relativeLenG ;
EXTERN EXCEPTBOX *exceptionsG ;


#undef EXTERN  

#endif /* READBLCK_H */
