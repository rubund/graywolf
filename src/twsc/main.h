/* ----------------------------------------------------------------- 
"@(#) main.h (Yale) version 4.6 1/25/91"
FILE:	    main.h                                       
DESCRIPTION:global definitions for TimberWolfSC
CONTENTS:   
DATE:	    Mar 27, 1989 
REVISIONS:  Sun Jan 20 21:47:52 PST 1991 - ported to AIX.
	    Fri Jan 25 23:46:21 PST 1991 - removed redundant variables.
----------------------------------------------------------------- */
#ifndef MAIN_H
#define MAIN_H

extern FILE *fpoG ;

extern BOOL orientation_optimizationG;
extern BOOL doGraphicsG ;
extern BOOL no_row_lengthsG ;
extern BOOL costonlyG ;

extern int **bin_configG ;
extern int tracksG ;
extern int iterationG;
extern int blkxspanG ;
extern int blkyspanG ;
extern int lrtxspanG ;
extern int lrtyspanG ;
extern int ifrangeG ;
extern int row_extentG ;

extern char *cktNameG;

void execute_global_router();

#endif /* MAIN_H */
