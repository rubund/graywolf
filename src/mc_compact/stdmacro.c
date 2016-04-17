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
FILE:	    stdmacro.c
DESCRIPTION:This file contains algorithm for removing violations of
	    stdcell macro placement.
CONTENTS:   
DATE:	    Apr 20, 1989 - original coding.
REVISIONS:  May  3, 1989 - changed to Y prefixes.
	    May 18, 1989 - added extended source test to insure tests
		are complete.
	    Jun 21, 1989 - changed swap to exchange only x positions.
	    May  4, 1990 - updated the functionality of Yset_init.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) stdmacro.c version 7.3 2/15/91" ;
#endif

#include <compact.h>
#include <yalecad/debug.h>
#include <yalecad/set.h>

/* --------------------------------------------------------------- 
  state   input    valid   next state
    H1     H        yes       H1
    H1     S        yes       S1
    S1     H        yes       H2
    S1     S        yes       S1
    H2     H        yes       H2
    H2     S        no        --
--------------------------------------------------------------- */
#define E  0
#define H1 1
#define S1 2
#define H2 3

typedef struct {
    int         node ;           /* node number */
    int         from_left ;      /* distance from left edge of bar */
    int         from_right ;     /* distance from right edge of bar */
    int         left_neighbor ;  /* left swapping neighbor */
    int         right_neighbor ; /* right swapping neighbor */
    BOOL        type ;           /* tile type */
    BOOL        candidate ;      /* whether it is an exchange candidate */
    COMPACTPTR  ptr ;            /* pointer to tile record */
} SELECTBOX, *SELECTPTR ;

static int nextStateS[4][2] = {
    { E,  E     },   /* error state */
    { H1, S1    },  /* state H1 */
    { H2, S1    },  /* state S1 */
    { H2, E     },  /* state H2 */
} ;
static YSETPTR nodeSetS ; /* these two implement set of tile nodes */
static SELECTPTR *stackArrayS ; /* members of an invalid path */



static int sortbydist();


partition_compact()
{
    ERRORPTR  violations, buildXGraph() ;
    int i ;

    /* --------------------------------------------------------------- 
    Algorithm:
	While partition violations remain do 
	    First remove overlap violations - use compact.  
	    Next build xgraph to show adjacencies in x direction.
	    Find longest path in x direction.  At this point each node
		will be labeled with a allowed window.  We use this for
		determining cell moves if violation is detected.
	    Next perform a depth-first search to check violations.  In
		a single path between source and sink we must only have
		H* S* H* patterns where H respresents hardcell, S 
		represents softcells and * represents zero or more 
		occurences. Note: we traverse the critical path first
		since it may change topology the most.
	    If a violation is detected, swap with partner in violation
		if possible, noting which direction cell will travel.
		Mark cell as moved.
	    else no violations break loop.
	od.
	We now will be able to compact in a normal fashion since topology
	is now fixed.
    --------------------------------------------------------------- */
    /* initialization */
    nodeSetS = Yset_init( 0, last_tileG ) ;
    /* allocate stackArray for fixing problems */
    stackArrayS = (SELECTPTR *)
	Ysafe_malloc( (numtilesG+2) * sizeof(SELECTBOX) ) ;
    for( i = 0; i <= last_tileG ; i++ ){
	stackArrayS[i] = (SELECTPTR) Ysafe_malloc( sizeof(SELECTBOX) ) ;
    }
    /* end initialization */

    /* perform constraint satisfaction */
    while( TRUE ){
	/* first remove overlap violations only */
	remove_violations() ;

	buildXGraph() ;

	/* G( ) is NOGRAPHICS conditional compile */
	G( if( graphicsG && TWinterupt() ) ){
	    G( process_graphics() ) ;
	}
	G( draw_the_data() ) ;

	/* build the critical path in x direction */
	longestxPath() ;
	    
	/* do depth first search removing violations */
	/* returns true when no violations remain */
	if( depth_first_search() ){
	    break ;
	}
	/* free graph so we can start over again */
	freeGraph(XFORWARD ) ;
	freeGraph(XBACKWARD ) ;

    } /* end loop */

    /* free graph so we can start over again */
    freeGraph(XFORWARD ) ;
    freeGraph(XBACKWARD ) ;

    M(MSG,NULL,"Constraints satisfied before compaction...\n") ;

    /* compact to make things fit better */
    compact() ;

    /* verify constraints second time to guarantee correctness */
    while( TRUE ){
	/* first remove overlap violations only */
	remove_violations() ;

	buildXGraph() ;

	/* G( ) is NOGRAPHICS conditional compile */
	G( if( graphicsG && TWinterupt() ) ){
	    G( process_graphics() ) ;
	}
	G( draw_the_data() ) ;

	/* build the critical path in x direction */
	longestxPath() ;
	    
	/* do depth first search removing violations */
	/* returns true when no violations remain */
	if( depth_first_search() ){
	    break ;
	}
	/* free graph so we can start over again */
	freeGraph(XFORWARD ) ;
	freeGraph(XBACKWARD ) ;

    } /* end loop */

    M(MSG,NULL,"Constraints satisfied after compaction...\n") ;
    /* show the results */
    G( draw_the_data() ) ;

} /* end partition_compact */

BOOL depth_first_search()
{
    /* ------------------------------------------------------------
    Algorithm:
	initialize all nodes active.
	initialize all edges unvisited.
	while an active node exist do 
	    node = source node.
	    while node != sink node do
		pick min node that is active.
		mark edge as visited.
		mark from field so we can backtrack. 
		if all edges of from node are marked, 
		    node becomes inactive.
	    od.
	    now at sink node verify H* S* H* constraints by backtracking.
	    if error swap cells noting direction and return FALSE.
	od.
	when we reach this point, no errors remain we are done
	and so return TRUE.
		    
    -------------------------------------------------------------*/
    int i ;                           /* counter */
    int source ;                      /* effective source node */
    int sink ;                        /* sink tile */
    int activeNodes ;                 /* number of active nodes */
    int node ;                        /* current node */
    int nextNode ;                    /* next candidate tile in path */
    int currentValue ;                /* find longest path first */
    int state ;                       /* current state for error check */
    BOOL inactive ;
    BOOL curType ;                    /* current type of macro */
    COMPACTPTR tptr ;                 /* tile pointer */
    COMPACTPTR tile ;                 /* tile pointer */
    ECOMPBOXPTR ptr ;                 /* edge pointer */
    YSETLISTPTR setptr ;              /* stack of nodes in sorted order */

    activeNodes = numtilesG + 1 ;
    sink = numtilesG + 1 ;
    Yset_empty( nodeSetS ) ;

    /* mark all the nodes active */
    /* initialize all the edges of the graph to FALSE */
    for( i=numtilesG; i >= 0; i-- ){
	tptr = tileNodeG[i] ;
	tptr->moved = FALSE ; /* use moved field as marker */
	for( ptr = tptr->xadjF; ptr ; ptr = ptr->next ){
	    ptr->marked = FALSE ;
	}
	/* since Yset_add keeps a stack add in reverse order */
	Yset_add( nodeSetS, i ) ;
    } /* end initialization */


    while( activeNodes > 0 ){

	/* find effective source node - nodes are in sorted order */
	/* from left to right. Pop one from the stack. */
	do {
	    setptr = Yset_enumerate( nodeSetS ) ;
	    source = setptr->node ;
	    tptr = tileNodeG[source] ;
	    if( tptr->moved ){
		Yset_delete( nodeSetS, source ) ;
		/* avoid this node it is marked already */
	    } else {
		break ; /* we found an unmarked node */
	    }
	} while( setptr ) ;

	node = source ;

	for( ; node != sink; node = nextNode ){
	    tptr = tileNodeG[node] ;

	    /* pick min node that is active */
	    nextNode = 0 ; /* initialize */
	    currentValue = INT_MAX ;
	    for( ptr = tptr->xadjF; ptr ; ptr = ptr->next ){
		if( ptr->marked ){
		    continue ;
		}
		tile = tileNodeG[ptr->node] ;
		if( tile->xvalueMin < currentValue ){
		    currentValue = tile->xvalueMin ;
		    nextNode = ptr->node ;
		}
	    }

	    if( nextNode == 0 ){
		break ; /* at end of the line exit loop */
	    }

	    /* mark edge as visited */
	    /* check to see if all edges are marked */
	    inactive = TRUE ;
	    for( ptr = tptr->xadjF; ptr ; ptr = ptr->next ){
		if( ptr->node == nextNode ){
		    ptr->marked = TRUE ;
		}
		if( !(ptr->marked) ){
		    inactive = FALSE ;
		}
	    }
	    if( inactive ){
		activeNodes-- ;
		Yset_delete( nodeSetS, node ) ;
		tptr->moved = TRUE ;
	    }

	    /* make tracks */
	    tileNodeG[nextNode]->pathx = node ;

	} /* end for loop */

	sink = node ;
	source = SOURCE ;

	/* now we need to follow the source all the way back to proper */
	/* source to verify complete path */
	while( node != source ){
	    tptr = tileNodeG[node] ;
	    curType = tptr->type ;
	    ptr = tptr->xadjB;
	    nextNode = ptr->node ; 
	    for( ; ptr ; ptr = ptr->next ){
		if( tileNodeG[ptr->node]->type != curType ){
		    nextNode = ptr->node ;
		    break ;
		}
	    } /* if we don't find any different one take first one */
	    /* make tracks */
	    tileNodeG[node]->pathx = nextNode ;

	    node = nextNode ; /* update node */
	    
	} /* end find beginning */


	/* now at sink node verify H* S* H* constraints */
	/* name states as H2* S1* H1* */
	state = H1 ;
	node = sink ;
	D( "mc_compact/depth_first_search",M( MSG, NULL, "\nPath:\n" ) ) ;
	while( node != source ){
	    tptr = tileNodeG[node] ;
	    /* use finite automata to check validity of constraints */
	    state = nextStateS[state][tptr->type] ;
	    /* ************** HOW TO FIX THINGS UP ***************** */
	    if( state == E ){ /* error */
		D( "mc_compact/depth_first_search",
		sprintf( YmsgG,"Topology error with tile:%d\n",
		    node ) ) ;
		D( "mc_compact/depth_first_search",
		    M( MSG, NULL, YmsgG ) ) ;

		remove_problem( source, sink ) ;
		return( FALSE ) ; /* we aren't done yet */
	    }
	    /* ************** END HOW TO FIX THINGS UP ************** */
	    D( "mc_compact/depth_first_search",
		sprintf( YmsgG,"   %d\n", node )  ;
		M( MSG, NULL, YmsgG ) ;
	    ) ;
	    node = tptr->pathx ;
	}
    }
    return( TRUE ) ; /* we are finished */
    
} /* end depth_first_search */

/* perform the swap of two nodes */
swap_nodes( node1, node2 )
int node1, node2 ;
{
    COMPACTPTR tptr1 ;                 /* tile pointer */
    COMPACTPTR tptr2 ;                 /* tile pointer */
    CELLBOXPTR cptr ;                  /* cell pointer */
    int new_xc1 ;              /* new position of node 1 center */
    int new_xc2 ;              /* new position of node 2 center */
    int deltax ;                /* change needed to accomplish swap */
    int cell ;                   /* current cell in question */

    ASSERT( node1 > 0 && node1 <= numtilesG,"swap_nodes",
	"node1 out of bounds\n" ) ;
    ASSERT( node2 > 0 && node2 <= numtilesG,"swap_nodes",
	"node2 out of bounds\n" ) ;
    D( "mc_compact/swap_nodes",
	sprintf( YmsgG, "swapping nodes:%d =><= %d\n", node1, node2 ) ) ;
    D( "mc_compact/swap_nodes",
        M( MSG,"swap_node", YmsgG ) ) ;
    /* swap relative to left side of tiles */
    tptr1 = tileNodeG[node1] ;
    tptr2 = tileNodeG[node2] ;
    new_xc1 = tptr2->l - tptr1->l_rel ;
    new_xc2 = tptr1->l - tptr2->l_rel ;

    /* now swap updating all tiles of cell one */
    cell = tptr1->cell ;
    cptr = cellarrayG[cell] ;
    deltax = new_xc1 - cptr->xcenter ;
    update_cell_tiles( cell, deltax, 0 ) ;

    /* now swap updating all tiles of cell two */
    cell = tptr2->cell ;
    cptr = cellarrayG[cell] ;
    deltax = new_xc2 - cptr->xcenter ;
    update_cell_tiles( cell, deltax, 0 ) ;

} /* end swap_nodes */

remove_problem( source, sink )
int source, sink ;
{
    int i ;              /* temp counter */
    int node ;
    int count ;          /* keeps track of size of tile path */
    int state ;          /* current state */
    int sinkX ;          /* position of sink */
    int sourceX ;        /* position of source */
    SELECTPTR  sptr ;
    COMPACTPTR tptr ;
    /* static int sortbydist() ; */ /* how to sort the tiles in path */

    /* start over putting state in stackArray */
    node = sink ; 
    count = 0 ;
    while( node != source ){
	sptr = stackArrayS[++count] ;
	tptr = tileNodeG[node] ;
	sptr->type = tptr->type ;
	sptr->ptr = tptr ;
	sptr->node = node ;
	sptr->from_left = INT_MAX ;
	sptr->from_right = INT_MAX ;
	sptr->candidate = TRUE ;
	node = tptr->pathx ;
    } /* we have now put error path in array with bounds 1 and count */

    /* now invalidate candidancy for H1 nodes at edge of bar*/
    /* also calculate the distance from the left edge of bar for */
    /* tiles traveling right or those who haven't made up their mind */
    state   = H1 ;
    sinkX   = tileNodeG[sink]->l ;
    sourceX = tileNodeG[source]->r ;
    /* from the right */
    for( i = 1; i <= count ; i++ ){
	sptr = stackArrayS[i] ;
	state = nextStateS[state][sptr->ptr->type] ;
	if( state == H1 ){
	    sptr->candidate = FALSE ;
	} else if( sptr->type == CELLTYPE &&
	    ( sptr->ptr->direction == NODIRECTION ||
	      sptr->ptr->direction == GOINGRIGHT ) ){
		sptr->from_right = sinkX - sptr->ptr->l ;
	} else if( sptr->type == STDCELLTYPE ){
	    /* these cells are never candidates */
	    sptr->candidate = FALSE ;
	}
	/* update neighbor field */
	if( i > 1 ){ /* avoid sink */
	    sptr->right_neighbor = stackArrayS[i-1]->ptr->node ;
	}
    } /* end loop from the right */

    /* from the left */
    /* do the same as above only from other side */
    state = H1 ;
    for( i = count; i > 0; i-- ){
	sptr = stackArrayS[i] ;
	state = nextStateS[state][sptr->ptr->type] ;
	if( state == H1 ){
	    sptr->candidate = FALSE ;
	} else if( sptr->type == CELLTYPE &&
	    ( sptr->ptr->direction == NODIRECTION ||
	      sptr->ptr->direction == GOINGLEFT ) ){
	    sptr->from_left = sptr->ptr->r - sourceX ;
	} else if( sptr->type == STDCELLTYPE ){
	    sptr->candidate = FALSE ;
	}
	/* update neighbor field */
	if( i < count ){ /* avoid sink */
	    sptr->left_neighbor = stackArrayS[i+1]->ptr->node ;
	}
    } /* end loop from the left */

    /* now sort tiles in array according to from distance */
    Yquicksort( (char *) &(stackArrayS[1]), count, 
	sizeof(SELECTPTR), sortbydist ) ;

    /* pick the node with the closest distance traveling */
    /* the right way already */
    for( i = 1; i <= count; i++ ){
	sptr = stackArrayS[i] ;
	if( !sptr->candidate ){
	    continue ;
	}
	if( sptr->from_left < sptr->from_right ){
	    /* coming from the left */
	    /* swap with left neighbor */
	    ASSERT( tileNodeG[sptr->left_neighbor]->type == STDCELLTYPE,
		NULL,"Logical error\n" ) ;
	    sptr->ptr->direction = GOINGLEFT ;
	    swap_nodes( sptr->node, sptr->left_neighbor );
	    return( FALSE ) ; /* we aren't done yet */
	} else {
	    /* coming from the right */
	    /* swap with right neighbor */
	    ASSERT( tileNodeG[sptr->right_neighbor]->type == STDCELLTYPE,
		NULL,"Logical error\n" ) ;
	    sptr->ptr->direction = GOINGRIGHT ;
	    swap_nodes( sptr->node, sptr->right_neighbor );
	    return( FALSE ) ; /* we aren't done yet */
	}
    }
    /* should never reach here - check is for memory problem*/
    ASSERT( TRUE, "depth_first_search", "Logic problem" ) ;

} /* end remove_problem */

/* sort by valid candidate first then distance left or right */
static int sortbydist( tileA, tileB )
SELECTPTR *tileA , *tileB ;
{
    SELECTPTR t1, t2 ; /* temps to make our life easier */
    int min1, min2 ;   /* the minimum of the distance for the two tiles */

    t1 = *tileA ;
    t2 = *tileB ;
    if( t1->candidate != t2->candidate ){
	if( t1->candidate == TRUE ){
	    return( -1 ) ;
	} else { /* t2 is a candidate */
	    return( 1 ) ; 
	}
    } else { /* sort by minimum distance to either side */
	min1 = MIN( t1->from_right, t1->from_left ) ;
	min2 = MIN( t2->from_right, t2->from_left ) ;
	return( min1 - min2 ) ;
    }
} /* end sortbydist */
