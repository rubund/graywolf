/*
 *   Copyright (C) 1989-1991 Yale University
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
FILE:	    globals.h
DESCRIPTION:This file contains type declarations for the master program.
CONTENTS:   
DATE:	    May  6, 1989 - original coding WPS.
REVISIONS:  Jun 19, 1989 - added marked to ADJPTR record.
	    Nov 23, 1990 - now exit status is correct if a program
		experiences trouble.
	    Sat Feb 23 04:49:55 EST 1991 - added TOMUS definitions
	    Thu Mar  7 01:35:22 EST 1991 - now windowID is a global
		and twflow can inherit a window itself.
	    Thu Apr 18 01:19:41 EDT 1991 - now user can specify a
		flow file.
	    Sun Apr 21 22:34:28 EDT 1991 - now allow optional files.
----------------------------------------------------------------- */
#ifndef GLOBALS_H
#define GLOBALS_H

/* if not using makefile and debug is wanted add it here */
/* #define DEBUG */

/* remove graphics lines of code if compile switch is on */
#ifdef NOGRAPHICS
#define G(x_xz) 

#else 
#define G(x_xz)   if( graphicsG ) x_xz

#endif /* NOGRAPHICS */

#include <globals.h>

/********************** DEFINE STATEMENTS *********************/
#define MASTER        "twflow"  /* the program name */
#define MASTERFAIL    (64+1)   /* catastrophic problem exit code */
#define MAXARGS       10       /* maximum number of arguments to a prog.*/
#define INPUTFILE     TRUE 
#define OUTPUTFILE    FALSE 
#define FORWARD       TRUE 
#define BACKWARD      FALSE 

/* the supported TimberWolf flows */
#define MIXED         0
#define STANDARD      1
#define MACRO         2
#define TOMUS         3
#define UNKNOWN       4

/* memory usage macro */
#define MEMUSAGE   { sprintf( YmsgG,"Current memory usage:%d\n", \
			YgetCurMemUse() ) ;\
		   M( MSG, NULL, YmsgG ) ; }

/* macro to speed division by 2 */
#define	DIVIDED_BY_2  >> 1 

/********************** TYPEDEF STATEMENTS *********************/
typedef struct dbox {        /* draw box to draw lines */
	int  x1, y1           ;  /* first point of line */
	int  x2, y2           ;  /* second point of line */
	struct dbox *next     ;  /* next line to draw */
} DBOX, *DPTR  ;

typedef struct fbox {        /* file box to maintain file concurrency */
	char *fname           ;  /* name of file suffix */
	BOOL optional         ;  /* TRUE if an optional file */
	struct fbox *next     ;  /* next file in list */
} FBOX, *FPTR  ;

typedef struct adjbox {
	int  node             ;  /* head of edge */
	char **argv           ; /* argument vector - limited to MAXARGS */
	int  argc             ; /* argument count */
	BOOL marked           ; /* whether edge has been executed */
	FPTR ifiles           ; /* files program is dependent on */
	FPTR ofiles           ; /* files this program generates */
	DPTR geometry         ;  /* how to draw the edge */
	struct adjbox *next   ;  /* next edge of this node */
} ADJBOX, *ADJPTR  ;

typedef struct objectbox {
	char *name            ; /* name of object */
	char *path            ; /* pathname of object */
	int  node             ; /* self index */
	int  numedges         ; /* number of outgoing edges */
	int  l                ; /* the extent of the cell bounding box */
	int  r                ; /* global position */
	int  b                ;
	int  t                ;
	ADJPTR adjF           ; /* the edges in the graph */
	ADJPTR adjB           ;
} OBJECTBOX, *OBJECTPTR ;

/* ******************** GLOBAL VARIABLES *********************** */
/* the graph data structure */
OBJECTPTR *proGraphG ;    /* array of pointers to program nodes */

/* the size of the data */
extern int  numobjectsG ;        /* number of program objects */
extern char *cktNameG;          /* name of the design */
extern char *twdirG ;            /* name of the TimberWolf directory */
extern char *flow_dirG ;         /* name of user specified flow dir. */
extern BOOL graphicsG ;          /* whether graphics is requested */
extern BOOL autoflowG ;          /* whether autoflow is enabled */
extern BOOL problemsG ;          /* whether problems were encountered */
extern BOOL tomusG ;             /* TRUE if called from partitioning */

#endif /* GLOBALS_H */
