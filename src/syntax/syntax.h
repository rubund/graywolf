/*
 *   Copyright (C) 1989 Yale University
 *
 *   This work is distributed in the hope that it will be useful; you can
 *   redistribute it and/or modify it under the terms of the
 *   GNU General Public License as published by the Free Software Foundation;
 *   either version 2 of the License,
 *   or any later version, on the following conditions:
 *
 *   (a) YALE MAKES NO, AND EXPRESSLY DISCLAIMS
 *   ALL, REPRESENTATIONS OR WARRANTIES THAT THE MANUFACTURE, USE, PRACTICE,
 *   SALE OR
 *   OTHER DISPOSAL OF THE SOFTWARE DOES NOT OR WILL NOT INFRINGE UPON ANY
 *   PATENT OR
 *   OTHER RIGHTS NOT VESTED IN YALE.
 *
 *   (b) YALE MAKES NO, AND EXPRESSLY DISCLAIMS ALL, REPRESENTATIONS AND
 *   WARRANTIES
 *   WHATSOEVER WITH RESPECT TO THE SOFTWARE, EITHER EXPRESS OR IMPLIED,
 *   INCLUDING,
 *   BUT NOT LIMITED TO, WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A
 *   PARTICULAR
 *   PURPOSE.
 *
 *   (c) LICENSEE SHALL MAKE NO STATEMENTS, REPRESENTATION OR WARRANTIES
 *   WHATSOEVER TO
 *   ANY THIRD PARTIES THAT ARE INCONSISTENT WITH THE DISCLAIMERS BY YALE IN
 *   ARTICLE
 *   (a) AND (b) above.
 *
 *   (d) IN NO EVENT SHALL YALE, OR ITS TRUSTEES, DIRECTORS, OFFICERS,
 *   EMPLOYEES AND
 *   AFFILIATES BE LIABLE FOR DAMAGES OF ANY KIND, INCLUDING ECONOMIC DAMAGE OR
 *   INJURY TO PROPERTY AND LOST PROFITS, REGARDLESS OF WHETHER YALE SHALL BE
 *   ADVISED, SHALL HAVE OTHER REASON TO KNOW, OR IN FACT SHALL KNOW OF THE
 *   POSSIBILITY OF THE FOREGOING.
 *
 */

/* ----------------------------------------------------------------- 
"@(#) globals.h version 1.1 5/5/89"
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
#define SYNTAX        "syntax"
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

EXTERN FILE *fpoG ;
EXTERN char *cktNameG ;

#endif /* GLOBALS_H */
