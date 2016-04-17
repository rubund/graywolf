/*
 *   Copyright (C) 1991-1992 Yale University
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
FILE:	    changraph.c
DESCRIPTION:
CONTENTS:   
DATE:	    Mon Feb 18 00:08:29 EST 1991
REVISIONS:  Fri Mar 29 14:16:57 EST 1991 - added cell edge tree data
		structure.
	    Mon Aug 12 16:44:05 CDT 1991 - updated for new Yrbtree_init.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) changraph.c (Yale) version 1.4 5/21/92" ;
#endif

#include <stdio.h>
#include <yalecad/message.h>
#include <yalecad/string.h>
#include <yalecad/debug.h>
#include <yalecad/rbtree.h>
#include <yalecad/set.h>
#include <compact.h>

static INT horz_edgeS = 0 ;  /* number of distinct horizontal edges */
static INT vert_edgeS = 0 ;  /* number of distinct vertical edges */
static YTREEPTR horz_treeS ; /* tree of horizontal edges */
static YTREEPTR vert_treeS ; /* tree of vertical edges */
static YSETPTR  nodesetS ;   /* keep track of active nodes */


/* ***************************************************************** 
    Initialize graph by adding creating memory for it.
   **************************************************************** */
init_graph( numnodes, numedges )
INT numnodes ;
INT numedges ;
{
    INT i ;

    /* allocate memory for channel graph array */
    changraphG = (CHANBOXPTR *) 
	Ysafe_malloc( (numnodes+1)*sizeof(CHANBOXPTR) );
    for( i = 0; i <= numnodes; i++ ){
	changraphG[i] = (CHANBOXPTR) Ysafe_malloc( sizeof(CHANBOX) ) ;
	changraphG[i]->adj = NULL ;
    }
    edgeArrayG = (INFOPTR *) 
	Ysafe_malloc( (numedges+1)*sizeof(INFOBOX) );

    /* copy to global variables */
    numnodesG = numnodes ;
    numedgesG = numedges ;
} /* end init_graph */

/* ***************************************************************** 
   Build graph by first adding nodes and node information to it.
   **************************************************************** */
addNode( node, xc, yc ) 
INT node, xc, yc ;
{
    CHANBOXPTR nptr ;
    ASSERTNRETURN( 1 <= node && node <= numnodesG, "addNode",
	"node out of range" ) ;
    nptr = changraphG[node] ;
    nptr->node = node ;
    nptr->xc = xc ;
    nptr->yc = yc ;
} /* end addNode */

/* ***************************************************************** 
   Build channel adjacency list by forming undirected graph.
   **************************************************************** */
addEdge( node1, node2, HnotV, cell_lb, cell_rt ) 
INT node1 ;
INT node2 ;
BOOL HnotV ;  /* TRUE if horizontal FALSE if vertical */
INT cell_lb ; /* cell on left (bottom) for vert (horz) channel */
INT cell_rt ; /* cell on right (top) for  vert (horz) channel */
{
    static edgeCountS = 0 ;  /* current number of edges */
    ADJPTR newfE,  /* new forward edge */
           temp ;  /* temporary pointer to relink adjacency list */
    ADJPTR findAdjPtr() ; /* returns an edge given two nodes */
    INFOPTR iptr ; /* store this only once for each edge */

    /* form an undirected graph */
    /* create forward edge */
    ASSERTNRETURN( 1 <= node1 && node1 <= numnodesG, "addEdge",
	"node out of range" ) ;
    ASSERTNRETURN( 1 <= node2 && node2 <= numnodesG, "addEdge",
	"node out of range" ) ;
    if( temp = changraphG[node1]->adj ){
	/* insert at beginning of list */
	newfE = changraphG[node1]->adj = (ADJPTR) 
	    Ysafe_malloc(sizeof(ADJBOX)) ;
	newfE->next = temp ;
	/* fix back link of temp */
	ASSERT( temp->prev == NULL, "addEdge","temp->prev is not NULL" );
	temp->prev = newfE ;
    } else {
	/* start adjacency list */
	newfE = changraphG[node1]->adj = (ADJPTR) 
	    Ysafe_malloc(sizeof(ADJBOX)) ;
	newfE->next = NULL ;
    }
    newfE->prev = NULL ;
    /* load data now */
    newfE->tnode = node2 ;
    newfE->fnode = node1 ;
    /* we only want to store one infobox record for each edge */
    /* see if forward edge was previously formed */
    if( temp = findAdjPtr( node2, node1 )){
	ASSERTNRETURN( temp->info, "addEdge", "iptr NULL" ) ;
	newfE->info = iptr = temp->info ;
	/* set the loc, start, and end fields */
	if( HnotV ){
	    /* guarantee that the loc is the lower number node */
	    if( node1 < node2 ){
		iptr->loc = &(changraphG[node1]->yc) ;
	    } else {
		iptr->loc = &(changraphG[node2]->yc) ;
	    }
	    if( changraphG[node1]->xc < changraphG[node2]->xc ){
		iptr->start = &(changraphG[node1]->xc) ;
		iptr->end   = &(changraphG[node2]->xc) ;
	    } else {
		iptr->start = &(changraphG[node2]->xc) ;
		iptr->end   = &(changraphG[node1]->xc) ;
	    }
	} else {
	    /* guarantee that the loc is the lower number node */
	    if( node1 < node2 ){
		iptr->loc = &(changraphG[node1]->xc) ;
	    } else {
		iptr->loc = &(changraphG[node2]->xc) ;
	    }
	    if( changraphG[node1]->yc < changraphG[node2]->yc ){
		iptr->start = &(changraphG[node1]->yc) ;
		iptr->end   = &(changraphG[node2]->yc) ;
	    } else {
		iptr->start = &(changraphG[node2]->yc) ;
		iptr->end   = &(changraphG[node1]->yc) ;
	    }
	}
	return ;
    } else {
	iptr = newfE->info = (INFOPTR) Ysafe_malloc( sizeof(INFOBOX) ) ;
	edgeArrayG[++edgeCountS] = iptr ;
	/* determine number of unique edges for edge tree */
	if( HnotV ){
	    horz_edgeS++ ;
	} else {
	    vert_edgeS++ ;
	}
    }
    iptr->HnotV = HnotV ;
    iptr->cell_lb = cell_lb ;
    iptr->cell_rt = cell_rt ;
    iptr->node1 = MIN( node1, node2 ) ;
    iptr->node2 = MAX( node1, node2 ) ;
    iptr->marked = FALSE ;

} /* end addEdge */

/* returns the adjacency pointer(edge) given two node of channel graph */
ADJPTR findAdjPtr( node1, node2 )
INT node1, node2 ;
{
    ADJPTR curEdge ;

    for( curEdge=changraphG[node1]->adj;curEdge;curEdge=curEdge->next ){
	if( curEdge->tnode == node2 ){
	    /* we found the edge */
	    return( curEdge ) ;
	}
    }
    /* if we didn't find the edge return NULL */
    return( NULL ) ;
} /* end findAdjPtr */

INT compare_edges( edge1, edge2 )
INFOPTR edge1, edge2 ;
{
    /* explicitly enumerate cases to avoid wraparound */
    if( *(edge1->loc) == *(edge2->loc) ){
	/* in case of tie sort on start position */
	if( *(edge1->start) == *(edge2->start) ){
	    return( 0 ) ;
	} else if( *(edge1->start) < *(edge2->start) ){
	    return( -1 ) ;
	} else {
	    return( 1 ) ;
	}
    } else {
	/* sort by location */
	if( *(edge1->loc) == *(edge2->loc) ){
	    return( 0 ) ;
	} else if( *(edge1->loc) < *(edge2->loc) ){
	    return( -1 ) ;
	} else {
	    return( 1 ) ;
	}
    }
} /* end compare_edges */

VOID print_edge( edge )
INFOPTR edge ;
{
    fprintf( stderr, "edge %2d-%2d loc:%3d start:%3d end:%3d HnotV:%d\n",
	edge->node1, edge->node2, *(edge->loc), *(edge->start), 
	*(edge->end), edge->HnotV ) ;
} /* end print_edge */

build_trees()
{
    INT i ;
    INFOPTR eptr ;     /* current edge */
    CELLBOXPTR cptr ;

    horz_treeS = Yrbtree_init( compare_edges ) ;
    vert_treeS = Yrbtree_init( compare_edges ) ;

    for( i = 1; i <= numedgesG ; i++ ){
	eptr = edgeArrayG[i] ;
	if( eptr->HnotV ){
	    Yrbtree_insert( horz_treeS, eptr ) ;
	} else {
	    Yrbtree_insert( vert_treeS, eptr ) ;
	}
    } /* end for loop */

    D( "mc_compact/xtree",
	fprintf( stderr, "The horizontal edges:\n" ) ;
	Yrbtree_dump( horz_treeS, print_edge ) ;
    ) ;
    D( "mc_compact/ytree",
	fprintf( stderr, "The vertical edges:\n" ) ;
	Yrbtree_dump( vert_treeS, print_edge ) ;
    ) ;

    nodesetS = Yset_init( 1, numnodesG ) ;

    /* now build the edge trees for each cell */
    for( i = 1; i <= numcellsG; i++ ){
	cptr = cellarrayG[i] ;
	cptr->hedges = Yrbtree_init( compare_edges ) ;
	cptr->vedges = Yrbtree_init( compare_edges ) ;
    }
    for( i = 1; i <= numedgesG ; i++ ){
	eptr = edgeArrayG[i] ;
	if( eptr->HnotV ){
	    /* horizontal edges */
	    if( eptr->cell_lb && eptr->cell_lb <= numcellsG ){
		Yrbtree_insert( cellarrayG[eptr->cell_lb]->hedges, eptr);
	    } 
	    if( eptr->cell_rt && eptr->cell_rt <= numcellsG ){
		Yrbtree_insert( cellarrayG[eptr->cell_rt]->hedges, eptr);
	    } 
	} else {
	    /* vertical edges */
	    if( eptr->cell_lb && eptr->cell_lb <= numcellsG ){
		Yrbtree_insert( cellarrayG[eptr->cell_lb]->vedges, eptr);
	    } 
	    if( eptr->cell_rt && eptr->cell_rt <= numcellsG ){
		Yrbtree_insert( cellarrayG[eptr->cell_rt]->vedges, eptr);
	    } 
	}
    }

} /* end build_trees */

/* given an x and y find the closest edge to the right or top */
INFOPTR get_closest_edge( x, y )
INT x, y ;
{

    INFOBOX lo, hi ;              /* set the bounds for the search */
    INFOPTR edge ;                /* current edge */
    INFOPTR closest_edge ;        /* save the closest edge */
    BOOL start ;                  /* true if we intersect interval */
    INT dist ;                    /* current distance to edge */
    INT closest_dist ;            /* best distance so far */
    INT min ;                     /* minimum integer */
    INT max ;                     /* maximum integer */

    /* need these since loc, and start are defined as INT * */
    /* they are defined that way because update edges will be */
    /* much easier */
    min = INT_MIN ;
    max = INT_MAX ;

    /* first look thru the horizontal tree */
    lo.loc   = &y ;
    lo.start = &min ;
    hi.loc   = &max ;
    hi.start = &min ;
    closest_dist = INT_MAX ;
    closest_edge = NIL(INFOBOX *) ;
    start = FALSE ; /* no edge found yet */
    for( edge = (INFOPTR) Yrbtree_interval(horz_treeS,&lo,&hi,TRUE);
	 edge;
	 edge = (INFOPTR) Yrbtree_interval(horz_treeS,&lo,&hi,FALSE) ){
	if( *(edge->start) <= x && x <= *(edge->end) ){
	    dist = ABS( *(edge->loc) - y ) ;
	    if( dist < closest_dist ){
		closest_dist = dist ;
		closest_edge = edge ;
	    }
	    start = TRUE ;
	} else if( start ){
	    break ;
	}
    }
    /* next look thru the vertical tree */
    lo.loc   = &x ;
    lo.start = &min ;
    hi.loc   = &max ;
    hi.start = &min ;
    start = FALSE ; /* no edge found yet */
    for( edge = (INFOPTR) Yrbtree_interval(vert_treeS,&lo,&hi,TRUE);
	 edge;
	 edge = (INFOPTR) Yrbtree_interval(vert_treeS,&lo,&hi,FALSE) ){
	if( *(edge->start) <= y && y <= *(edge->end) ){
	    dist = ABS( *(edge->loc) - x ) ;
	    if( dist < closest_dist ){
		closest_dist = dist ;
		closest_edge = edge ;
	    }
	    start = TRUE ;
	} else if( start ){
	    break ;
	}
    }

    if( closest_edge ){
	closest_edge->marked = TRUE ;
    }
    return( closest_edge ) ;
} /* end get_closest_edge */

stretch_graph( stretch_edge, x, y )
INFOPTR stretch_edge ;
INT x, y ;
{
    INFOBOX lo, hi ;              /* set the bounds for the search */
    INFOPTR edge ;                /* the current pointer */
    CHANBOXPTR nptr1 ;            /* pointer to node 1 info */
    CHANBOXPTR nptr2 ;            /* pointer to node 2 info */
    INT location ;                /* location of the stretch_edge */
    INT new_loc ;                 /* the new location of node */
    INT delta ;                   /* the stretch or shrink amount */
    INT min ;                     /* minimum integer */
    INT max ;                     /* maximum integer */

    /* need these since loc, and start are defined as (INT *) */
    /* they are defined that way because update edges will be */
    /* much easier */
    min = INT_MIN ;
    max = INT_MAX ;

    /* use the nodeset to keep track of which nodes have already been */
    /* updated */
    Yset_empty( nodesetS ) ;
    if( stretch_edge->HnotV ){
	delta = y - *(stretch_edge->loc) ;
	location = *(stretch_edge->loc) - 1 ;
	lo.loc   = &location ;
	lo.start = &min ;
	hi.loc   = &max ;
	hi.start = &min ;
	for( edge = (INFOPTR) Yrbtree_interval(horz_treeS,&lo,&hi,TRUE);
	     edge;
	     edge = (INFOPTR) Yrbtree_interval(horz_treeS,&lo,&hi,FALSE)){

	     /* check to see if this edge's location is valid */
	     /* node1 is always the location node */
	     if( Yset_member( nodesetS, edge->node1 ) ){
		new_loc = *(edge->loc) ;
	     } else {
		/* this node has not been updated yet */
		 /* update position of the edge */
		 new_loc = *(edge->loc) + delta ;
	     }
	     /* now update all the appropriate edges and nodes */
	     /* first the nodes */
	     nptr1 = changraphG[edge->node1] ;
	     if( nptr1->yc != new_loc && 
		!(Yset_member( nodesetS, edge->node1))  ){
		nptr1->yc = new_loc ;
		Yset_add( nodesetS, edge->node1 ) ;
	     }
	     nptr2 = changraphG[edge->node2] ;
	     if( nptr2->yc != new_loc &&
		!(Yset_member( nodesetS, edge->node2))  ){
		nptr2->yc = new_loc ;
		Yset_add( nodesetS, edge->node2 ) ;
	     }
	}

    } else {
	delta = x - *(stretch_edge->loc) ;
	location = *(stretch_edge->loc) - 1 ;
	lo.loc   = &location ;
	lo.start = &min ;
	hi.loc   = &max ;
	hi.start = &min ;
	for( edge = (INFOPTR) Yrbtree_interval(vert_treeS,&lo,&hi,TRUE);
	     edge;
	     edge = (INFOPTR) Yrbtree_interval(vert_treeS,&lo,&hi,FALSE)){

	     /* check to see if this edge's location is valid */
	     /* node1 is always the location node */
	     if( Yset_member( nodesetS, edge->node1 ) ){
		new_loc = *(edge->loc) ;
	     } else {
		/* this node has not been updated yet */
		/* update position of the edge */
		new_loc = *(edge->loc) + delta ;
	     }
	     /* now update all the appropriate edges and nodes */
	     /* first the nodes */
	     nptr1 = changraphG[edge->node1] ;
	     if( nptr1->xc != new_loc && 
		!(Yset_member( nodesetS, edge->node1))  ){
		nptr1->xc = new_loc ;
		Yset_add( nodesetS, edge->node1 ) ;
	     }
	     nptr2 = changraphG[edge->node2] ;
	     if( nptr2->xc != new_loc &&
		!(Yset_member( nodesetS, edge->node2))  ){
		nptr2->xc = new_loc ;
		Yset_add( nodesetS, edge->node2 ) ;
	     }
	}
    }
} /* end stretch_graph */
