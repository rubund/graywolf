/*
 *   Copyright (C) 1988-1991 Yale University
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
"@(#) compact.h version 7.5 11/8/91"
FILE:	    compact.h
DESCRIPTION:This file contains type declarations for the compaction
	    routines.
CONTENTS:   
DATE:	    Apr  8, 1988 
REVISIONS:  Oct 24, 1988 - added moved field to tilenode record for
			    graph concurrency problem.
	    Oct 29, 1988 - made PICKETBOX doubly linked list to 
		facilitate deletes.
	    Apr 17, 1989 - made modification to make compactor separate
		program.  Also now handle multiple tile cells correctly.
	    Apr 30, 1989 - added definitions to handle partition case.
	    Sun Nov  4 12:50:58 EST 1990 - changed slack to be over
		cells not tiles.
	    Sat Feb 23 00:49:20 EST 1991 - added constraint compaction
		data structures.
	    Fri Mar 29 14:18:56 EST 1991 - added stretched to CELL
		box definition and added path_deck to definitions.
	    Mon May  6 22:32:34 EDT 1991 - added gridding reference to
		cell fields.
	    Fri Nov  8 18:16:21 EST 1991 - removed INT_SMALL and INT_LARGE
		definitions since gcc couldn't handle it.
----------------------------------------------------------------- */
#ifndef COMPACT_H
#define COMPACT_H

#ifndef COMPACT_DEFS
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
#define G(x_xz)   x_xz

#endif /* NOGRAPHICS */

#include <yalecad/base.h>
#include <yalecad/message.h>
#include <yalecad/set.h>
#include <yalecad/rbtree.h>
#include <yalecad/deck.h>

/********************** DEFINE STATEMENTS *********************/
#define SOURCE        0                /* definitions in x and y graph */
#define SINK          numtilesG+1
#define XSOURCE       0                /* relative to tileNodeG */
#define XSINK         numtilesG+1
#define YSOURCE       numtilesG+2      /* relative to tileNodeG */
#define YSINK         numtilesG+3
#define XSOURCEC      0                
#define XSINKC        numcellsG+1
#define YSOURCEC      numcellsG+2     
#define YSINKC        numcellsG+3
#define NODIRECTION   0
#define GOINGRIGHT    1
#define GOINGLEFT     2
#define CELLTYPE      0  /* used in determine macro placement */
#define STDCELLTYPE   1
#define XFORWARD      1
#define XBACKWARD     2
#define YFORWARD      3
#define YBACKWARD     4
#define COMPACT       "mc.compactor"  /* the program name */
#define CFAILBASE     16              /* program problem exit code */
#define COMPACTFAIL   (CFAILBASE+1)   /* catastrophic problem exit code */

#define NOTOUCH       0               /* tiles don't touch or overlap */
#define TOUCH        -1               /* tiles touch but dont overlap */
#define OVERLAP1      1               /* tiles overlap completely */
#define OVERLAP2      2               /* tiles overlap to right (top) */
#define OVERLAP3      3               /* tiles overlap to left (bot) */

/* memory usage macro */
#define MEMUSAGE   { sprintf( YmsgG,"Current memory usage:%d\n", \
			YgetCurMemUse() ) ;\
		   M( MSG, NULL, YmsgG ) ; }

/* macro to speed division by 2 */
#define	DIVIDED_BY_2  >> 1 

typedef struct picketbox { /* use for both x and y graph */
    union {
	INT  top ;   /* xgraph */
	INT  rght ;  /* ygraph */
    } pt1 ;
    union {
	INT  bot ;   /* xgraph */
	INT  lft ;   /* ygraph */
    } pt2 ;
    INT  node ;
    struct picketbox *next ;
    struct picketbox *prev ;
} PICKETBOX, *PICKETPTR ;

typedef struct errorbox {
    INT  nodeI ;
    INT  nodeJ ;
    struct errorbox *next ;
} ERRORBOX, *ERRORPTR ;

typedef struct ecompbox {
    INT  constraint       ;
    INT  node             ;
    BOOL marked           ;
    struct ecompbox *next ;
} ECOMPBOX, *ECOMPBOXPTR  ;

typedef struct node {     /* the nodes or tiles of a cell */
    INT  node       ;
    struct node *next ;
} NODEBOX, *NODEPTR  ;


typedef struct cellbox {
    INT  xcenter          ; /* the center of the cell */
    INT  ycenter          ;
    INT  l                ; /* the extent of the cell bounding box */
    INT  r                ; /* global position */
    INT  b                ;
    INT  t                ;
    INT  l_rel            ; /* the extent of the cell bounding box */
    INT  r_rel            ; /* relative to center of cell */
    INT  b_rel            ;
    INT  t_rel            ;
    INT  cellnum          ; /* TimberwolfMC cell number */
    INT  xmin             ; /* the minimum x you can place the cell */
    INT  xmax             ; /* the maximum x you can place the cell */
    INT  ymin             ; /* the minimum y you can place the cell */
    INT  ymax             ; /* the maximum y you can place the cell */
    INT  xlo              ; /* one of the leftest nodes of the cell rel to l */
    INT  xhi              ; /* one of the rightest nodes of the cell rel to l*/
    INT  ylo              ; /* one of the lowest nodes of the cell rel to b */
    INT  yhi              ; /* one of the highest nodes of the cell rel to b */
    INT  xoffset          ; /* offset for cell gridding */
    INT  yoffset          ; /* offset for cell gridding */
    BOOL children         ;
    BOOL type             ; /* whether a hard or soft cell */
    BOOL multi            ; /* whether cell has multiple tiles */
    BOOL stretched        ; /* whether cell graph has been stretched */
    NODEPTR tiles         ; /* the tiles belonging to this cell */
    YTREEPTR hedges       ; /* horz edges of the channel graph touching cell */
    YTREEPTR vedges       ; /* vert edges of the channel graph touching cell */
} CELLBOX, *CELLBOXPTR ;

typedef struct compactbox {
    INT  cell       ;
    INT  node       ;
    INT  l          ; /* global position  of tile */
    INT  b          ;
    INT  r          ;
    INT  t          ;
    INT  l_rel      ; /* relative postion of tile */
    INT  b_rel      ;
    INT  r_rel      ;
    INT  t_rel      ;
    INT  xspan      ; /* xpan of tile r_rel - l_rel */
    INT  yspan      ; /* xpan of tile t_rel - b_rel */
    BOOL criticalX  ; /* on the critical list */
    BOOL criticalY  ;
    BOOL moved      ; /* whether cell has been moved in movestrategy */
    INT  xvalueMin  ; /* for longest path calculations */
    INT  xvalueMax  ;
    INT  yvalueMin  ;
    INT  yvalueMax  ;
    INT  pathx      ;
    INT  pathy      ;
    BOOL type       ; /* whether a hard or soft tile */
    BOOL multi      ; /* whether cell has multiple tiles */
    INT  xancestrF  ;
    INT  xancestrB  ;
    INT  yancestrF  ;
    INT  yancestrB  ;
    INT  direction  ; /* current direction cell is moving */
    ECOMPBOXPTR xadjF; /* the edges in the graph */
    ECOMPBOXPTR xadjB;
    ECOMPBOXPTR yadjF;
    ECOMPBOXPTR yadjB;
} COMPACTBOX, *COMPACTPTR ;


typedef struct  {  /* information for edge - store 1 per edge */
    INT node1  ;      /* the two nodes of the edge */
    INT node2  ;
    INT cell_lb ;     /* cell on left/bottom if non-zero - cellnum ow */
    INT cell_rt ;     /* cell on right/top if non-zero - cellnum o.w. */
    BOOL HnotV ;      /* true if edge horizontal - false if vertical */
    INT *loc ;        /* constant part of coordinate */
    INT *start ;      /* start of edge */
    INT *end ;        /* end coordinate of edge */
    BOOL marked ;     /* whether this edge is marked */
} INFOBOX, *INFOPTR ;

typedef struct adjptr { /* added adjacency list augmenting adj matrix */
    int tnode ;         /* to node - head of edge */
    int fnode ;         /* from node - tail of edge */
    INFOPTR info ;      /* all the details of the edge store only once */
    struct adjptr *next ;     /* doubly linked list of edge's */
    struct adjptr *prev ;
} ADJBOX, *ADJPTR ;

typedef struct { /* a node of channel graph uses following data */
    INT      node ;           /* self index */
    INT      xc ;             /* center of node for channel graph  */
    INT      yc ;
    ADJPTR   adj ;            /* edges connecting to this node */
} CHANBOX, *CHANBOXPTR ;

typedef struct queue {
    INT           data ;
    struct queue *next ;
} QUEUEBOX, *QUEUEPTR  ;

/* --------------------------------------------------------------- 
    MACRO DEFINITIONS FOR QUEUE USED IN LONGEST PATH ALGORITHM 
    To function two variables queue and botqueue must be defined in
    routine.  Currently uses safe_malloc routines but could be
    changed to memory pool if speed need. 
/* 

/* initialize queue */
#define INITQUEUE( queue, node ) \
{   queue = (QUEUEPTR) Ysafe_malloc( sizeof(QUEUEBOX) ) ; \
    queue->next = NULL ; \
    botqueue = queue ;   \
    queue->data = node ; \
}

/* top queue return the element at top of the queue */
#define TOPQUEUE( queue ) \
    queue->data ;\
    { QUEUEPTR temp = queue ; \
    queue = queue->next ; \
    Ysafe_free( temp ) ;  \
    }

/* add a vertex to the end of the queue */
#define ADD2QUEUE( queue, node ) \
{   if( queue ){ \
	botqueue->next = (QUEUEPTR) Ysafe_malloc( sizeof(QUEUEBOX) ) ; \
	botqueue = botqueue->next ; \
	botqueue->next = NULL ; \
	botqueue->data = node ; \
    } else { \
	INITQUEUE( queue, node ) ; \
    } \
}


/* ******************** GLOBAL VARIABLES *********************** */
/* the graph data structure */
EXTERN COMPACTPTR *tileNodeG ;        /* array of pointers to nodes */
EXTERN COMPACTPTR *xGraphG ;          /* array of nodes to x node ptrs */
EXTERN COMPACTPTR *yGraphG ;          /* array of yGraph node pointers */
EXTERN YSETPTR    tileSetG ;          /* make set of tiles */

/* the size of the data */
EXTERN INT numcellsG ;             /* number of cells to be compacted   */
EXTERN INT numtilesG ;             /* number of tiles to be compacted   */
EXTERN INT last_tileG ;            /* last tile in x or y graph */
EXTERN INT last_cellG ;            /* last cell including sources and sinks */
EXTERN char *cktNameG ;            /* the name of the circuit           */
EXTERN CELLBOXPTR *cellarrayG ;    /* the cell data */
EXTERN CELLBOXPTR *slackG ;        /* array of cells sorted by slack */
EXTERN INT blockbG ;               /* core bottom */
EXTERN INT blocklG ;               /* core left */
EXTERN INT blockrG ;               /* core right */
EXTERN INT blocktG ;               /* core top */
EXTERN INT blockmxG ;              /* core center */
EXTERN INT blockmyG ;              /* core center */
EXTERN INT *ancestorG ;            /* array of ancestor counts */

/* FOR THE CHANNEL GRAPH */
EXTERN INT numnodesG ;      /* number of nodes in the channel graph */
EXTERN INT numedgesG ;      /* number of edges in the channel graph */
EXTERN CHANBOXPTR *changraphG ; /* array of nodes of channel graph  */
EXTERN INFOPTR    *edgeArrayG ; /* array of edges of channel graph */
EXTERN YDECKPTR path_deckG ;    /* list of nodes in critical path */

/* the user requests */
EXTERN BOOL alignG     ;  /* TRUE if channel are to be aligned */
EXTERN BOOL compactG   ;  /* TRUE if compaction is desired */
EXTERN BOOL graphicsG  ;  /* TRUE if graphics are desired */
EXTERN BOOL parasiteG  ;  /* TRUE if we want to inherit window */
EXTERN BOOL partitionG ;  /* TRUE if partitioning is requested */
EXTERN BOOL constraintsG; /* TRUE if constraint graph is present */
EXTERN BOOL debugG ;      /* TRUE if debug is requested */
EXTERN INT xgridG ;       /* force cells to given x grid */
EXTERN INT ygridG ;       /* force cells to given y grid */
EXTERN INT xspaceG ;      /* xspacing between tiles of different cells */
EXTERN INT yspaceG ;      /* yspacing between tiles of different cells */

#endif /* COMPACT_H */
