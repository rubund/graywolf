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

char *cktNameG ;

int rowsG ;
int attmaxG ;
int maxCellOG ;
int iterationG ;

int blkxspanG ;
int blkyspanG ;
int ffeedsG ;
int lrtxspanG ;
int lrtyspanG ;
int ifrangeG ;
int *fixarrayG ;

FILE *fpoG ;

int d_costG ;
int resume_runG ;
int implicit_pins_usedG ;
int rowHeightG ;
int fdthrusG ;

double TG ;
double imprangeG ;
double rowSepG ;
int    rowSepAbsG ;
double stopdegG ;

unsigned Yrandom_seed() ;
unsigned randomSeedG  ;
unsigned randomSeed2G ;

#endif /* MAIN_H */
