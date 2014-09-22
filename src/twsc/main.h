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

EXTERN INT rowsG ;
EXTERN INT attmaxG ;
EXTERN INT maxCellOG ;
EXTERN INT iterationG ;

EXTERN INT blkxspanG ;
EXTERN INT blkyspanG ;
EXTERN INT ffeedsG ;
EXTERN INT lrtxspanG ;
EXTERN INT lrtyspanG ;
EXTERN INT ifrangeG ;
EXTERN INT *fixarrayG ;

EXTERN FILE *fpoG ;

EXTERN INT d_costG ;
EXTERN INT resume_runG ;
EXTERN INT implicit_pins_usedG ;
EXTERN INT rowHeightG ;
EXTERN INT fdthrusG ;

EXTERN DOUBLE TG ;
EXTERN DOUBLE imprangeG ;
EXTERN DOUBLE rowSepG ;
EXTERN INT    rowSepAbsG ;
EXTERN DOUBLE stopdegG ;

EXTERN unsigned Yrandom_seed() ;
EXTERN unsigned randomSeedG  ;
EXTERN unsigned randomSeed2G ;

#undef EXTERN

#endif /* MAIN_H */
