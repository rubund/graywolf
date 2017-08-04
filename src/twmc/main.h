/* ----------------------------------------------------------------- 
FILE:	    main.h                                       
CONTENTS:   definitions for globals structures and variables
DATE:	    Jan 29, 1988 
REVISIONS:  Aug  7, 1988 - added control flags for pads.
	    Oct 25, 1988 - added flags for neighborhood and groups
	    Oct 27, 1988 - changed RAND to acm minimal standard generator
	    Feb 13, 1989 - added graphics definitions.
	    Feb 25, 1989 - removed redundant macro definitions and
		added inclusion of yalecad/string.h
	    Feb 26, 1989 - added G suffix to all global variables.
	    Mar 01, 1989 - added wait_for_user global and added
		compile switch for program exit status.
	    Mar 07, 1989 - added instance type for cells.
	    Mar 11, 1989 - added graphics conditional compile.
	    Apr  1, 1989 - deleted instance type now store with master
		cell.
	    May 18, 1989 - reorganized globals.
	    May 24, 1989 - removed doPlacementG
	    May 25, 1989 - added graphFilesG Boolean.
	    Jun 21, 1989 - added verboseG global.
	    Sep 27, 1989 - deleted MAXTILES now dynamic.
	    Oct  3, 1989 - added scale_inputG for large designs.
	    Oct 20, 1989 - removed pitch and added track_spacingXG and
		track_spacingYG ;
	    Fri Jan 25 18:07:47 PST 1991 - added quickroute variable
		and deleted extraneous variables.
	    Mon Feb  4 02:13:39 EST 1991 - added new_wire_estG global
		variable.
	    Sat Feb 23 00:22:35 EST 1991 - added defines for TOMUS.
	    Wed May  1 16:42:30 EDT 1991 - removed defines for TOMUS.
----------------------------------------------------------------- */
/* *****************************************************************
   static char SccsId[] = "@(#) main.h version 3.7 5/1/91" ;
***************************************************************** */
#ifndef MAIN_H
#define MAIN_H

#include <globals.h>

#ifndef MATH_H
#define MATH_H
#include <math.h>
#endif   /* MATH_H */

/* program exit status is different in VMS */
#ifdef VMS
#define OK 1
#define FAIL 0
#else /* not VMS */
#define OK 0
#define FAIL 1
#endif /* VMS */

/* if not using makefile and debug is wanted add it here */
/* #define DEBUG */

/* remove graphics lines of code if compile switch is on */
#ifdef NOGRAPHICS
#define G(x_xz) 

#else 
#define G(x_xz)   x_xz

#endif /* NOGRAPHICS */

/* BASIC MACRO DEFINITIONS  */
#define MAXSITES 50

/* cell types */
typedef int CELLTYPE ;
#define CUSTOMCELLTYPE 1
#define PADCELLTYPE    2
#define SOFTCELLTYPE   3
#define PADGROUPTYPE   4
#define SUPERCELLTYPE  5 /* cell which inherits all prop. of children */
#define MERGEDCELLTYPE 6 /* child of a super cell */
#define GROUPCELLTYPE  7 /* a cell group moves together with the group */
#define PADMACROTYPE   8 /* all pads on a side form pad bbox for changen*/
#define STDCELLTYPE    9 /* cells created by partitioner */

/* fixed type flags */
#define POINTFLAG         0  /* cell fixed at a point */
#define NEIGHBORHOODFLAG  1  /* cell fixed in a neighborhood */
#define GROUPFLAG         2  /* group flag free to move */
#define FIXEDGROUPFLAG    3  /* group fixed at a point */

/* constants for graphic routines */
#define PLACEMENT                1
#define PARTITION_PLACEMENT      2
#define VIOLATION_REMOVAL        3
#define COMPACTION               4
#define CHANNEL_GENERATION       5

#define PINGROUPTYPE     1
#define HARDPINTYPE      2
#define SOFTPINTYPE      3
#define SOFTEQUIVTYPE    4
#define HARDEQUIVTYPE    5
#define ADDEQUIVTYPE     6
#define ANALOGPINTYPE    7

 char *cktNameG ;
 char *argv0G ;     /* the pathname of the program */
 int attpercellG ;
 int scale_dataG ;  /* reduce the scale of the input data */
 int track_spacingXG ;
 int track_spacingYG ;
 int defaultTracksG ;

/* booleans for control of program */
 BOOL cost_onlyG ;
 BOOL doChannelGraphG ;
 BOOL doGlobalRouteG ;
 BOOL doCompactionG ;
 BOOL doPartitionG ;
 BOOL doGraphicsG ;
 BOOL quickrouteG ;
 BOOL new_wire_estG ;        /* use new wire estimation alg. */
 BOOL restartG ;
 BOOL wireEstimateOnlyG ;
 BOOL wait_for_userG ;
 BOOL verboseG ;

int readcells(char *filename);

void writeResults( int wire, int penal, int rand );
void set_wiring_reduction( double reduction );

#endif /* MAIN_H */
