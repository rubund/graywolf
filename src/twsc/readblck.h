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

typedef struct exceptbox {
    int row ;
    int ll_x ;
    int ll_y ;
    int ur_x ;
    int ur_y ;
} *EXCEPTPTR, EXCEPTBOX ;

/* global variable definitions */
int num_exceptsG ;
int top_of_top_rowG ;
int bot_of_bot_rowG ;
int uniform_rowsG ;
int individual_rowSepsG ;
int total_except_widthG ;
double *rowSepsG ;
int *rowSepsAbsG ;
double *relativeLenG ;
EXCEPTBOX *exceptionsG ;

void readblck(FILE *fp);

#undef EXTERN  

#endif /* READBLCK_H */
