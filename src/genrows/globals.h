/*
 *   Copyright (C) 1989-1992 Yale University
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
DESCRIPTION:This file contains the global definitions and typedefs
	    for the genrow program.
CONTENTS:   
DATE:	    Aug  9, 1989 
REVISIONS:  Feb 28, 1990 - added new fields to tiles for graphics.
	    Dec  7, 1990 - added force field and new variables for
		performing merges.
	    Fri Jan 25 17:50:54 PST 1991 - added mirror row feature.
	    Sat Feb 23 00:34:11 EST 1991 - added no macro move global.
	    Thu Mar  7 03:11:42 EST 1991 - added num_rowsG global.
	    Wed Apr 17 22:39:04 EDT 1991 - added wait_for_user global
		and orientation field for macro.
----------------------------------------------------------------- */
#ifndef GLOBALS_H
#define GLOBALS_H

#ifndef lint
static char SccsglobalsId[] = "@(#) globals.h (Yale) version 3.11 5/14/92" ;
#endif

#ifndef GLOBAL_DEFS
#define EXTERN extern

#else
#define EXTERN
#endif

/* remove graphics lines of code if compile switch is on */
#ifdef NOGRAPHICS
#define G(x_xz) 

#else 
#define G(x_xz)   x_xz

#endif /* NOGRAPHICS */

#define START 1
#define END 2
#define NOTHING 3
#define FROM_LEFT 4
#define FROM_RIGHT 5
#define FROM_TOP 6
#define FROM_BOTTOM 7
#define TO_LEFT 8
#define TO_RIGHT 9
#define TO_TOP 10
#define TO_BOTTOM 11
#define MAYBE_START 12
#define MAYBE_END 13

#define MERGED_NONE 0
#define MERGED_UP   1
#define MERGED_DOWN 2
#define MERGED_LEFT 3
#define MERGED_RITE 4

typedef struct tile_box {
    int name ;              /* index of the tile */
    int llx ;
    int lly ;
    int urx ;
    int ury ;
    int numrows ;
    int class ;
    int actual_row_height ;
    int add_no_more_than ; /* the xspan of the tile */
    int channel_separation ;
    int min_length ; /* the size of the minimum valid row */
    int max_length ; /* the maximum length of the block normally tile width - 2 */
    int row_start ;  /* offset from the beginning of the tile */
    int merged ;     /* direction of the merge */
    BOOL illegal ;   /* FALSE means a legal tile to use */
    BOOL force ;     /* force the number of rows */
    BOOL mirror ;    /* mirror rows ?? */
    BOOL allocated ; /* whether tile is currently allocated or freed */
    struct tile_box *next ;
    struct tile_box *prev ;
} TILE_BOX ;

typedef struct macro_tile {
    int l ;
    int r ;
    int b ;
    int t ;
    struct macro_tile *next ;
} MACRO_TILE ;

typedef struct vertex_box {
    int x ;
    int y ;
    int class ;
    int macro ;
    struct vertex_box *next ;
} *VERTEXPTR, VERTEX_BOX ;

typedef struct macro_rec {
    MACRO_TILE *tiles ;    /* the tiles of the macro */
    int num_vertices ;     /* number of vertices on macro */
    VERTEXPTR *vertices ; /* the vertices of a macro */
    int xcenter ;
    int ycenter ;
    int xcenter_orig ;
    int ycenter_orig ;
    int left ;
    int right ;
    int bottom ;
    int top ;
    int orient ;
} MACROBOX, *MACROPTR ;

typedef struct row_box {
    int llx ;
    int lly ;
    int urx ;
    int ury ;
    int class ;
    int seg ;
    BOOL mirror ;    /* mirror rows ?? */
    struct row_box *next_row ;
    struct row_box *next_segment ;
} ROW_BOX ;

/* ********************** global variables ************************ */
/* GENERAL GLOBAL VARIABLES */
EXTERN char *cktNameG ;
EXTERN BOOL graphicsG ;           /* TRUE if graphics are requested */

/* DATA GLOBAL VARIABLES */
EXTERN  TILE_BOX *tile_listG ;    /* head of list of tiles */
EXTERN  TILE_BOX *last_tileG ;    /* end of list of tiles */
EXTERN  TILE_BOX *start_tileG;    /* current start of tiles */
EXTERN  MACROPTR *macroArrayG ;
EXTERN  MACROPTR *mergeArrayG ;   /* tiles that the user merged */
EXTERN  int      num_macrosG ;    /* number of macros given */
EXTERN  int      num_mergedG ;    /* number of merged tiles given */
EXTERN  int      num_rowsG ;      /* user specified the number of rows */
EXTERN  int      feeds_per_segG ; /* feed length per segment */
EXTERN  int      flip_alternateG; /* flip alternate rows beginning at 1 or 2 */ 
EXTERN  int      spacingG ;       /* spacing to macros */
EXTERN  BOOL     shortRowG ;      /* whether to keep short row */
EXTERN  BOOL     invalidG ;      /* whether configuration is ok */
EXTERN  BOOL     limitMergeG ;    /* whether to limit merge to 1 step */
EXTERN  BOOL     noMacroMoveG ;   /* when TRUE macros may not move */
EXTERN  BOOL     wait_for_userG;  /* normally wait for user */
EXTERN  BOOL     last_chanceG;    /* "last chance" timeout */
EXTERN  BOOL     memoryG ;        /* try to remember last state */
EXTERN  BOOL     no_outputG ;     /* when TRUE no .blk file */
/* ********************** end global variables ******************** */

#endif /* GLOBALS_H */
