
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
extern int num_exceptsG ;
extern int top_of_top_rowG ;
extern int bot_of_bot_rowG ;
extern int uniform_rowsG ;
extern int individual_rowSepsG ;
extern int total_except_widthG ;
extern double *rowSepsG ;
extern int *rowSepsAbsG ;
extern double *relativeLenG ;
extern EXCEPTBOX *exceptionsG ;
extern int rowsG ;

void readblck(FILE *fp);

#endif /* READBLCK_H */
