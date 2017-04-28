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
    int row ;
    int ll_x ;
    int ll_y ;
    int ur_x ;
    int ur_y ;
} *EXCEPTPTR, EXCEPTBOX ;

/* global variable definitions */
EXTERN int num_exceptsG ;
EXTERN int top_of_top_rowG ;
EXTERN int bot_of_bot_rowG ;
EXTERN int uniform_rowsG ;
EXTERN int individual_rowSepsG ;
EXTERN int total_except_widthG ;
EXTERN double *rowSepsG ;
EXTERN int *rowSepsAbsG ;
EXTERN double *relativeLenG ;
EXTERN EXCEPTBOX *exceptionsG ;


#undef EXTERN  

#endif /* READBLCK_H */
