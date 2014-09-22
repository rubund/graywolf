/* ----------------------------------------------------------------- 
"@(#) globals.h version 1.1 7/30/91"
FILE:	    globals.h                                       
CONTENTS:   definitions for globals structures and variables
DATE:	    Apr 24, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h> 

#ifndef MATH_H
#define MATH_H
#include <math.h>
#endif   /* MATH_H */

/* if not using makefile and debug is wanted add it here */
/* #define DEBUG */

/* ***********LEAVE THE BELOW UNCHANGED *************************** */
#define NOCUT        "nocut - replacement for Mincut"
#define HARDCELLTYPE  1
#define SOFTCELLTYPE  2
#define STDCELLTYPE   3
#define PADCELLTYPE   4

/* compile switch for globals */
#ifndef MAIN_VARS
#define EXTERN extern

#else
#define EXTERN
#endif

EXTERN char *cktNameG ;

#endif /* GLOBALS_H */
