/* ----------------------------------------------------------------- 
FILE:	    geo.h                                       
CONTENTS:   definitions for channel definition structures and variables
DATE:	    Apr 12, 1988 
REVISIONS:
	    Jul  5, 1988 - changed EBOX field notActive to active.
	    Jul 17, 1988 - added orderedPaths to list of globals 
		for detail routing.
	    Jan 24, 1989 - added four fields to RECTPTR so that
		we can convert graph to rectilinear coordinates.
	    Mar 16, 1989 - removed all data structures related to 
		detail drawing.  These will be put in its own program.
	    Sep 14, 1989 - added polar graph typedefs and globals.
	    Feb  8, 1990 - removed cell list from data structures.
	    Mar  3, 1990 - now a standalone program.
----------------------------------------------------------------- */
/* *****************************************************************
   static char SccsId[] = "@(#) geo.h version 1.2 4/28/91" ;
***************************************************************** */
#ifndef GEO_H
#define GEO_H

#include <stdio.h>
#include <yalecad/base.h>
#include <23tree.h>
#include <yalecad/rbtree.h>

#undef EXTERN 
#ifndef GEO_DEFS
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

#define VBIG INT_MAX

typedef struct edgebox {
    INT cell   ;
    INT start  ;
    INT end    ;
    INT loc    ;
    INT length ;
    BOOL UorR   ;
    BOOL fixed  ;
    INT prevEdge ;
    INT nextEdge ;
    INT node     ;
} EDGEBOX, *EDGEBOXPTR ;

typedef struct dlink1 {
    INT edge ;
    struct dlink1 *prev ;
    struct dlink1 *next ;
} DLINK1 , *DLINK1PTR ;

typedef struct dlink2 {
    INT index  ;
    struct dlink2 *prev ;
    struct dlink2 *next ;
} DLINK2 , *DLINK2PTR ;

typedef struct ebox {
    INT index1  ;
    INT index2  ;
    INT width   ;
    INT lbside  ;
    INT rtside  ;
    INT length  ;
    INT hiend   ;
    INT loend   ;
    INT edge1   ;
    INT edge2   ;
    INT density ;
    BOOL active ; /* changed polarity to positive logic for easier */
    TNODEPTR root ; /* understanding */
} EBOX, *EBOXPTR ;

typedef struct adjptr { /* added adjacency list augmenting adj matrix */
    INT node ;
    EBOXPTR edge ;             /* channel edge information */
    struct adjptr *partner ; /* directed edge e(i,j) 's partner e(j,i) */
    struct adjptr *next ; /* doubly linked list of edge's */
    struct adjptr *prev ;
} ADJBOX, *ADJPTR ;

typedef struct rect {
    INT xc ;
    INT yc ;
    INT xreset ;
    INT yreset ;
    INT l ;
    INT r ;
    INT b ;
    INT t ;
    INT node_l ; /* these four fields contain the update node area */
    INT node_r ; /* these fields are for straightening the channel graph*/
    INT node_b ;
    INT node_t ;
    INT ur ;
    INT ul ;
    INT lr ;
    INT ll ;
    INT nedges ;
    INT from ;    /* used to calculate longest path */
    INT distance ;/* used to calculate longest path */
    BOOL HnotV ;   /* flag for orthogonal penalty */
    BOOL active ; /* flag for active nodes */
    ADJPTR adj ;  /* added adjacency list to channel graph */
} RECT , *RECTPTR ;

typedef struct flare {
    INT fixEdge ;
    INT *eindex ;
    struct flare *next ;
} FLARE, *FLAREPTR ;

typedef struct nodbox {
    FLAREPTR inList ;
    FLAREPTR outList ;
    INT done ;
} NODBOX, *NODPTR ;

typedef struct wcbox {
    INT fixedWidth ;
    INT node ;
    INT *channels ;
    INT length ;
    struct wcbox *next ;
} WCBOX, *WCPTR ;

typedef struct pedge {
    INT node  ;
    INT chan1  ;
    INT chan2  ;
    INT length  ;
    BOOL valid ;/* edges that cross channels outside the pads notvalid */
    struct pedge *next ;
} POLAREDGE, *POLAREDGEPTR ;

typedef struct {
    INT xc  ;
    INT yc ;
    BOOL valid ;  /* node along pad path is not valid*/
    POLAREDGEPTR adj ;
} POLARBOX, *POLARPTR ;

typedef struct pointrec {
    INT x  ;
    INT y ;
    struct pointrec *next ;
} POINT, *POINTPTR ;

typedef struct {
    INT xc  ;
    INT yc ;
    POINTPTR pts ;  /* vertices of the cell */
    YTREEPTR tiles ;
} CELLBOX, *CELLBOXPTR ;

EXTERN INT **eIndexArrayG ;
EXTERN INT numXnodesG , numYnodesG ;
EXTERN INT numberCellsG ;
EXTERN INT eNumG ;
EXTERN INT edgeTransitionG ;
EXTERN INT numRectsG ;
EXTERN INT edgeCountG ;
EXTERN INT numProbesG ;
EXTERN INT numxPolarG ;  /* number of nodes in xpolar graph */
EXTERN INT numyPolarG ;  /* number of nodes in ypolar graph */
EXTERN INT bblG , bbrG , bbbG , bbtG ;

EXTERN INT track_pitchXG ;       /* track pitch in horizontal direction */
EXTERN INT track_pitchYG ;       /* track pitch in vertical direction */
EXTERN INT defaultTracksG ;      /* number of extra tracks to add */

EXTERN FILE *fpdebugG ;

EXTERN EDGEBOXPTR edgeListG ; /* array of ptrs to cell and c.r. edges */
EXTERN EBOXPTR eArrayG ;
EXTERN RECTPTR rectArrayG ;
EXTERN WCPTR *xNodeArrayG , *yNodeArrayG ;
EXTERN NODPTR xNodulesG , yNodulesG ;
EXTERN TNODEPTR LErootG , BErootG ;
EXTERN TNODEPTR VDrootG , HRrootG ;
EXTERN TNODEPTR VrootG , HrootG ;
EXTERN TNODEPTR vChanEndRootG , vChanBeginRootG ;
EXTERN TNODEPTR hChanEndRootG , hChanBeginRootG ;
EXTERN DLINK2PTR *LEptrsG , *BEptrsG ;
EXTERN DLINK1PTR *VDptrsG , *HRptrsG ;
EXTERN DLINK1PTR *VptrsG , *HptrsG ;
EXTERN DLINK1PTR HlistG , VlistG ;
EXTERN DLINK2PTR HRlistG , VRlistG ;
EXTERN DLINK1PTR HendG , VendG ;
EXTERN DLINK1PTR hFixedListG , vFixedListG ;
EXTERN DLINK1PTR hFixedEndG , vFixedEndG ;
EXTERN TNODEPTR hFixedEdgeRootG ;
EXTERN TNODEPTR vFixedEdgeRootG ;
EXTERN TNODEPTR hEdgeRootG , vEdgeRootG ;
EXTERN TNODEPTR hRectRootG , vRectRootG ;
EXTERN POLARPTR xPolarG ;/* array of nodes of xpolar graph */
EXTERN POLARPTR yPolarG ;/* array of nodes of ypolar graph */
EXTERN CELLBOXPTR cellarrayG ; /* the cell information */

EXTERN BOOL doGraphicsG ; /* whether graphics is on */
EXTERN BOOL debugG ;      /* whether debug is on */
EXTERN char *cktNameG ;   /* the circuit name */

#endif /* GEO_H */
