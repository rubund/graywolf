/* ----------------------------------------------------------------- 
"@(#) globals.h version 1.2 6/19/89"
FILE:	    globals.h                                       
CONTENTS:   definitions for globals structures and variables
DATE:	    Apr 24, 1989 
REVISIONS:  Jun 19, 1989 - added macro defines. 
----------------------------------------------------------------- */
#ifndef GLOBALS_H
#define GLOBALS_H

#ifndef lint
static char SccsGlobals_h[] = "@(#) globals.h (Yale) version 1.2 6/19/89" ;
#endif

#include <stdio.h> 

#ifndef MATH_H
#define MATH_H
#include <math.h>
#endif   /* MATH_H */

/* if not using makefile and debug is wanted add it here */
/* #define DEBUG */

/* ***********LEAVE THE BELOW UNCHANGED *************************** */
#define PADTRANSLATE  "convert"
#define PADB  1
#define PADL  2
#define PADR  3
#define PADT  4
#define PMUL  5
#define PMUR  6
#define PMLL  7
#define PMLR  8
#define PML   9
#define PMR   10
#define PMTT  11
#define PMBB  12
#define PMB   13
#define PMT   14
#define PMM   15

/* compile switch for globals */
#ifndef MAIN_VARS
#define EXTERN extern

#else
#define EXTERN
#endif

EXTERN FILE *fpoG ;
EXTERN char *cktNameG ;
EXTERN int lineG ;

#endif /* GLOBALS_H */
