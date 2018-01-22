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

/* global variables defined for main */
#ifdef MAIN_VARS
#define EXTERN 
#else
#define EXTERN extern 
#endif

EXTERN char *cktNameG ;

EXTERN int rowsG ;
EXTERN int attmaxG ;
EXTERN int maxCellOG ;
EXTERN int iterationG ;

EXTERN int blkxspanG ;
EXTERN int blkyspanG ;
EXTERN int ffeedsG ;
EXTERN int lrtxspanG ;
EXTERN int lrtyspanG ;
EXTERN int ifrangeG ;
EXTERN int *fixarrayG ;

EXTERN FILE *fpoG ;

EXTERN int d_costG ;
EXTERN int resume_runG ;
EXTERN int implicit_pins_usedG ;
EXTERN int rowHeightG ;
EXTERN int fdthrusG ;

EXTERN DOUBLE TG ;
EXTERN DOUBLE imprangeG ;
EXTERN DOUBLE rowSepG ;
EXTERN int    rowSepAbsG ;
EXTERN DOUBLE stopdegG ;

EXTERN unsigned Yrandom_seed() ;
EXTERN unsigned randomSeedG  ;
EXTERN unsigned randomSeed2G ;

#undef EXTERN

#endif /* MAIN_H */
