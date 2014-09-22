/* ----------------------------------------------------------------- 
FILE:	    pads.h                                       
CONTENTS:   definitions for pad placement code.
DATE:	    Aug 03, 1990 
REVISIONS:  Nov 23, 1990 - added fields for drawing rectilinear
		cells properly.
	    Thu Feb 14 02:42:57 EST 1991 - modified for new pad
		placement algorithm.
	    Fri Feb 15 01:38:42 EST 1991 - added lo_pos and hi_pos
	        fields for the VARIABLE_PADS case.
	    Sun Feb 17 21:09:24 EST 1991 - added new global variables
		so that pad placement code will work with macros.
----------------------------------------------------------------- */
/* *****************************************************************
   "@(#) pads.h (Yale) version 4.6 2/17/91"
***************************************************************** */
#ifndef PADS_H
#define PADS_H 

/* control for pad spacing */
#define UNIFORM_PADS  0
#define VARIABLE_PADS 1
#define ABUT_PADS     2
#define EXACT_PADS    3

/* pad sides */
#define ALL     0
#define L       1
#define T       2
#define R       3
#define B       4

/* macro sides */
#define MUL 7 
#define MUR 8 
#define MLL 9 
#define MLR 10
#define ML  11
#define MR  12
#define MB  13 
#define MT  14 
#define MM  15 
#define MTT 16 
#define MBB 17 
#define MMC 18 /* from MC */

#define MINI  0
#define MAXI  1

#define X     0
#define Y     1

#define HOWMANY 0
#define LARGE 100000
#define PINFINITY INT_MAX / 8
#define PICK_INT(l,u) (((l)<(u)) ? ((RAND % ((u)-(l)+1))+(l)) : (l))

/* pad type */
#define NONE      0
#define LEAF      1
#define SUBROOT   2
#define ROOT      3

typedef struct pad_rec {
    INT    cellnum          ;  /* index in carray */
    INT    length           ;
    INT    height           ;
    INT    *xpoints         ;  /* xcoordinates of pad/macro */
    INT    *ypoints         ;  /* xcoordinates of pad/macro */
    INT    position         ;  /* linear placement position for a side */
    INT    tiebreak         ;  /* ideal location to place the pad */
    INT    numcorners       ;  /* number of corners */
    INT    *children        ;  /* added for pad code */
    INT    padside          ;  /* current side */
    INT    padtype          ;  /* type of pad - pad or padgroup */
    INT    hierarchy        ;  /* type of pad - root, lead, pad, etc */
    INT    lo_pos           ;  /* low valid position -1 if every valid */
    INT    hi_pos           ;  /* hi valid position PINFINITY if valid */
    BOOL   ordered          ;  /* whether a pad is ordered in a group */
    BOOL   oldformat        ;  /* true if old format */
    BOOL   permute          ;  /* whether rank is to be enforced */
    BOOL   macroNotPad      ;  /* true if macro false if pad */
    BOOL   fixed            ;  /* whether pad is fixed on a side */
    BOOL   valid_side[5]    ;  /* valid side a pad may be placed */
    DOUBLE lowerbound       ;
    DOUBLE upperbound       ;
} PADBOX, *PADBOXPTR ;

/* ############# global variables defined for pad code ############## */
#ifdef PAD_VARS
#define EXTERN 
#else
#define EXTERN extern 
#endif

EXTERN INT padspacingG ;
EXTERN BOOL contiguousG ;
EXTERN INT min_pad_spacingG ;
EXTERN INT coreG[2][2] ;
EXTERN INT perdimG[2] ;
EXTERN INT numpadsG ;           /* number of leaf pads + simple pads */
EXTERN INT numMacroG ;          /* total number of macros */
EXTERN INT totalpadsG ;         /* pads + padgroups */

/* DATA GLOBAL VARIABLES */
EXTERN PADBOXPTR *padarrayG ;   /* array of all the pads and padgroups */
EXTERN PADBOXPTR *sortarrayG ;  /* the sorted array of pads */
EXTERN PADBOXPTR *placearrayG ; /* array where the pads will be placed */

#undef EXTERN

#endif /* PADS_H */
