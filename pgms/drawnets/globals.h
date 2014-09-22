/* ----------------------------------------------------------------- 
FILE:	    globals.h
DESCRIPTION:This file contains type declarations for the XXXXprogram.
CONTENTS:   
DATE:	    
REVISIONS: 
----------------------------------------------------------------- */
#ifndef GLOBALS_H
#define GLOBALS_H

#ifndef lint
static char SccsGlobals_h[] = "@(#) globals.h (Yale) version 1.1 4/21/91" ;
#endif

#ifndef GLOBAL_DEFS
#define EXTERN extern

#else
#define EXTERN
#endif

/* if not using makefile and debug is wanted add it here */
/* #define DEBUG */

/* remove graphics lines of code if compile switch is on */
#ifdef NOGRAPHICS
#define G(x_xz) 

#else 
#define G(x_xz)   if( graphicsG ) x_xz

#endif /* NOGRAPHICS */

/********************** DEFINE STATEMENTS *********************/
/* set expected size of program here */
#define EXPECTEDMEMORY  (256 * 1024)  
#define NULLWINDOW      0
#define VERSION         "1.0" 
#define PROGRAM         "analyze_nets" 

/********************** TYPEDEF STATEMENTS *********************/

/* ******************** GLOBAL VARIABLES *********************** */
EXTERN char *cktNameG ;          /* name of the design */
EXTERN BOOL graphicsG ;          /* whether graphics is requested */
EXTERN char **net_nameG ;        /* array of net names */
EXTERN INT  *net_lengthG ;           /* length of each net */
EXTERN INT  *net_numpinsG ;          /* numpins on each net */
EXTERN INT  numnetsG ;
EXTERN INT  *lengthG ;           /* length array */

#endif /* GLOBALS_H */
