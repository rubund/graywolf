/*
 *   Copyright (C) 1988-1990 Yale University
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
FILE:	    ycompact.c
DESCRIPTION:This file contains type declarations for the compaction
	    in the ydirection.
CONTENTS:   
DATE:	    Apr  8, 1988 
REVISIONS:  Sep 20, 1988 - removed excess edges from source and sink
	    Oct 29, 1988 - fixed problem when cell is exactly same
		size as picket or multiples of pickets.  Also changed
		picket to doubly-linked list.
	    Nov  7, 1988 - implemented doubly-linked list correctly.
	    Jan 29, 1989 - changed to YmsgG and added \n's.
	    Apr 22, 1989 - modified to make multiple cell tiles work.
		Fixed bug in update_ypicket().
	    Apr 25, 1989 - give quicksort two chances to order tiles
		expanding core if necessary.
	    May  3, 1989 - changed to Y prefixes.
	    May  6, 1989 - added no graphics compile switch
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) ycompact.c version 7.1 11/10/90" ;
#endif

#include <compact.h>
#include <yalecad/debug.h>

static PICKETPTR  leftPickS ;


ERRORPTR buildYGraph()
{
    int i ;                    /* counter */
    int overlapx ;             /* overlap conditions in x direction */
    int overlapy ;             /* overlap conditions in y direction */
    int sortbyYX() ;           /* sort the tiles Y then X */
    int left, right ;          /* coordinates of tiles */
    int bottom, top ;          /* coordinates of tiles */
    BOOL firstPick ;           /* TRUE if first picket which matches */
    BOOL possibleEdgetoSource; /* TRUE if this could be edge to source */
    BOOL *yancestorB ;         /* TRUE for a cell if cell has B ancestors */
    BOOL *yancestorF ;         /* TRUE for a cell if cell has F ancestors */
    COMPACTPTR candidate ;     /* this is the tile in question */
    COMPACTPTR t ;             /* this is the current picket */
    PICKETPTR freepick ;       /* used to free the pickets at the end */
    PICKETPTR curPick ;        /* traverse the pickets */
    PICKETPTR lowerLimit ;     /* first picket tile overlaps/touches */
    PICKETPTR upperLimit ;     /* last picket tile overlaps or touches */
    ERRORPTR errorPtr ;        /* form a list of errors to be processed*/
    ERRORPTR lasterror ;       /* last error in list */
    ERRORPTR violations ;      /* head of the error list */


    /* initialize error list */
    lasterror = NULL ;
    violations = NULL ;

    yancestorB = (BOOL *) Ysafe_calloc( last_cellG+1,sizeof(BOOL) ) ;
    yancestorF = (BOOL *) Ysafe_calloc( last_cellG+1,sizeof(BOOL) ) ;

    inityPicket() ;
    /* yGraphG is now initialized */

    /* sort by ymin xmin of the bounding box */
    /* we give it two chances - second time we expand core if necessary */
    for( i=0; i <= 1 ; i++ ){
	Yquicksort((char *)yGraphG,numtilesG+2,sizeof(COMPACTPTR),sortbyYX);
	if( yGraphG[SOURCE]->cell == YSOURCEC && 
	    yGraphG[SINK]->cell == YSINKC ){
	    break ;
	} else {
	    find_core( &left, &right, &bottom, &top ) ;
	    /* expand core region */
	    t = tileNodeG[YSOURCE] ;
	    t->b = bottom ;
	    t->t = t->b ;
	    t = tileNodeG[YSINK] ;
	    t->b = top ;
	    t->t = t->b ;
	}
    }
    if( yGraphG[SOURCE]->cell != YSOURCEC || yGraphG[SINK]->cell != YSINKC ){
	M( ERRMSG, "ycompact", "Fatal error in configuration\n" ) ;
	if( graphicsG ){
	    G( TWcloseGraphics() ) ;
	}
	YexitPgm( PGMFAIL ) ;
    }


    for( i=1 ; i<= numtilesG ; i++ ){
	firstPick = TRUE ;
	lowerLimit = NULL ;
	upperLimit = NULL ;
	possibleEdgetoSource = FALSE ;
	candidate = yGraphG[i] ;

	/* search thru picket list for adjacencies */
	for( curPick=leftPickS;curPick;curPick=curPick->next){
	    overlapx = projectX( curPick->pt2.lft, curPick->pt1.rght,
			candidate->l, candidate->r) ;
			
	    if( overlapx > 0 ){  /* allow touching */

		/* save span of overlap */
		if( firstPick ){
		    lowerLimit = curPick ;
		    firstPick = FALSE ;
		}
		upperLimit = curPick ;

		t = tileNodeG[curPick->node] ;

		/* multiple tile case - no error possible */
		if( candidate->cell == t->cell ){
		    continue ;
		}

		/* check for errors */
		overlapy = 
		    projectY( t->b, t->t, candidate->b, candidate->t ) ;
		if( overlapx > 0 && overlapy > 0 ){
		    /* save violations - add to violation list */
		    if( lasterror ){
			errorPtr = 
			    (ERRORPTR) Ysafe_malloc( sizeof(ERRORBOX) ) ;
			lasterror->next = errorPtr ;
			lasterror = errorPtr ;
		    } else {
			violations = errorPtr = lasterror = 
			    (ERRORPTR) Ysafe_malloc( sizeof(ERRORBOX) ) ;
		    }
		    errorPtr->next = NULL ;
		    errorPtr->nodeI = candidate->node ;
		    errorPtr->nodeJ = t->node ;
		    /*  for debug only :
			sprintf( YmsgG, 
			    Overlap detected: cell %d and cell %d\n",
			    candidate->cell, t->cell ) ;
			M( MSG, NULL, YmsgG ) ;
		    */
		}
		ASSERT( t->node == curPick->node, "buildCGraph",
		    "tileNodeG != curPick. Problem \n" ) ;
		/* form edge on only first occurance of cell */
		if( t->node == numtilesG+2 ){ /* source node */
		    /* delay adding this edge */
		    possibleEdgetoSource = TRUE ;
		} else {
		    formyEdge( t->node, candidate->node ) ;
		}
	    } else if( upperLimit ){
		/* we are past the upper limit so break & save time */
		break ;
	    }
	}
	ASSERT( lowerLimit, "compact", "lowerLimit is NULL" ) ;
	ASSERT( upperLimit, "compact", "lowerLimit is NULL" ) ;

	if( possibleEdgetoSource && yancestorF[candidate->cell] == FALSE ){
	    /* no need to make an edge to source if we already */
	    /* have an ancestor.  Always make sure it is one of the lowest */
	    /* nodes of the multi-tiled cell */
	    formyEdge( numtilesG+2, cellarrayG[candidate->cell]->ylo ) ;
	    yancestorF[candidate->cell] = TRUE ;
	}

	update_ypicket( i, lowerLimit, upperLimit ) ;

    } /* end for loop */

    /* process sink last */
    /* search thru picket list for adjacencies */
    for( curPick=leftPickS;curPick;curPick=curPick->next){

	/* remaining pickets must necessarily overlap sink */
	t = tileNodeG[curPick->node] ;
	ASSERT( t->node == curPick->node, "buildCGraph",
	    "tileNodeG != curPick. Problem \n" ) ;
	if( curPick->node != numtilesG+2 && /* avoid the source */
	    yancestorB[t->cell] == FALSE ){ /* no parents */
	    formyEdge( cellarrayG[t->cell]->yhi, yGraphG[last_tileG]->node ) ;
	    yancestorB[t->cell] = TRUE ;
	}
    }

    /* now add multiple tile edges to graph. Precomputed in multi.c */
    add_mtiles_to_ygraph() ;

    cleanupGraph( YFORWARD ) ;
    cleanupGraph( YBACKWARD ) ;
    Ysafe_free( yancestorB ) ;
    Ysafe_free( yancestorF ) ;

    /* delete all pickets */
    for( curPick = leftPickS; curPick ; ) {
	freepick = curPick ;
	curPick = curPick->next ;
	Ysafe_free( freepick ) ;
    }

    return( violations ) ;

} /* end buildYGraph */

formyEdge( fromNode, toNode ) 
int fromNode ;
int toNode ;
{
    COMPACTPTR e1, e2 ;
    ECOMPBOXPTR temp, newE ;
    int constraint ; /* constaint between tiles */

    e1 = tileNodeG[fromNode] ;
    D( "mc_compact/formyEdge",
	sprintf( YmsgG, "e1 is NULL; fromNode: %d\n", fromNode ) ) ;
    ASSERT( e1, "formyEdge", YmsgG ) ;

    e2 = tileNodeG[toNode] ;
    D( "mc_compact/formyEdge",
	sprintf( YmsgG, "e2 is NULL; toNode: %d\n", toNode ) ) ;
    ASSERT( e2, "formyEdge", YmsgG ) ;

    for( temp = e1->yadjF; temp; temp = temp->next ){
	if( temp->node == toNode ){
	    return ;
	}
    }

    /* calculate the forward constraint */
    /* This is a left edge algorithm so constraint is span of from tile */
    /* one yspaceG spacing unless same cell */
    if( e1->cell != e2->cell ){
	constraint = e1->t_rel - e1->b_rel + yspaceG ; /* span of tile */
    } else {
	/* this removes problem of overlapping tiles on multitiles */
	constraint = ABS( e2->b_rel - e1->b_rel ) ;
    }
    D( "mc_compact/formyEdge",
	sprintf( YmsgG, "Edge %d - %d has a negative edge weight\n", 
	fromNode, toNode ) ) ;
    ASSERT( constraint >= 0, "formyEdge", YmsgG ) ;
    if( constraint < 0 ){
	return ;
    }

    /* create forward edge */
    if( temp = e1->yadjF ){
	newE = e1->yadjF = (ECOMPBOXPTR) Ysafe_malloc(sizeof(ECOMPBOX)) ;
	/* hook to rest of list */
	newE->next = temp ;
    } else {
	/* start adjacency list */
	newE = e1->yadjF = (ECOMPBOXPTR) Ysafe_malloc(sizeof(ECOMPBOX)) ;
	newE->next = NULL ;
    }
    newE->node = toNode ;
    newE->marked = FALSE ;

    newE->constraint = constraint ; 

    e1->yancestrB++ ;  /* update ancestor count for longest path */

    /* create backward edge */
    if( temp = e2->yadjB ){
	newE = e2->yadjB = (ECOMPBOXPTR) Ysafe_malloc(sizeof(ECOMPBOX)) ;
	/* hook to rest of list */
	newE->next = temp ;
    } else {
	/* start adjacency list */
	newE = e2->yadjB = (ECOMPBOXPTR) Ysafe_malloc(sizeof(ECOMPBOX)) ;
	newE->next = NULL ;
    }
    newE->node = fromNode ;
    newE->marked = FALSE ;

    newE->constraint = constraint ;
    e2->yancestrF++ ;  /* update ancestor count for longest path */


}

inityPicket( ) 
{
    COMPACTPTR source, sink, node ;
    int i ;

    /* initialize all paths and values for all tiles */
    for( i=0; i<= last_tileG ; i++ ) {
	node = yGraphG[i] ;
	node->pathx = 0 ;
	node->pathy = 0 ;
	node->yvalueMin = INT_MIN ;
	node->yvalueMax = INT_MIN ;
	node->yancestrF = 0 ; 
	node->yancestrB = 0 ; 
	node->criticalY = FALSE ;
	node->moved = FALSE ;
    }
    source = yGraphG[0] ;
    source->yvalueMin = 0 ;
    source->yvalueMax = 0 ;

    sink = yGraphG[last_tileG] ;
    sink->yvalueMin = INT_MIN ;
    sink->yvalueMax = 0 ;

    /* initialize picket to source */
    leftPickS = (PICKETPTR) Ysafe_malloc( sizeof(PICKETBOX) ) ;
    leftPickS->pt1.rght = yGraphG[0]->r ;
    leftPickS->pt2.lft = yGraphG[0]->l ;
    leftPickS->node = yGraphG[0]->node ;
    leftPickS->next = NULL ;
    leftPickS->prev = NULL ;
}
    


update_ypicket( i, lowerLimit, upperLimit )
int i ;
PICKETPTR lowerLimit, upperLimit ;
{
    PICKETPTR t, temp, curPick ;
    COMPACTPTR newtile ;        /* new tile to be added to picket */

    /* for speed */
    newtile = yGraphG[i] ;

    if( newtile->l == lowerLimit->pt1.rght ){
	/* increment lower limit */
	lowerLimit = lowerLimit->next ;
    }

    if( lowerLimit != upperLimit ){
	/* first reset limits if cell i coincides with limits */
	if( newtile->l == lowerLimit->pt2.lft ){
	    lowerLimit = lowerLimit->prev ;
	}
	if( newtile->r == upperLimit->pt1.rght ){
	    upperLimit = upperLimit->next ;
	}

	/* insert node */
	t = lowerLimit->next ; /* save old pointer */
	temp = lowerLimit->next = 
	    (PICKETPTR) Ysafe_malloc( sizeof(PICKETBOX) ) ;
	/* fix back link */
	temp->prev = lowerLimit ;

	/* delete picket- delete all pickets up to upperLimit */
	for( curPick = t; curPick ; ) {
	    if( curPick == upperLimit ){
		break ;
	    }
	    t = curPick ;
	    curPick = curPick->next ;
	    Ysafe_free( t ) ;
	}
	/* left link is now done. Now update picket information */
	/* form overlapping pickets */
	/* set lower limit */
	lowerLimit->pt1.rght = newtile->l ;
	/* set upper limit  */
	upperLimit->pt2.lft = newtile->r ;
	/* fill current picket */
	temp->pt2.lft = newtile->l ;
	temp->pt1.rght = newtile->r ;
	temp->node = newtile->node ;
	/* fix the right link */
	temp->next = upperLimit ;
	upperLimit->prev = temp ;
	ASSERT( tileNodeG[lowerLimit->node]->l <= lowerLimit->pt1.rght && 
	    lowerLimit->pt1.rght <= tileNodeG[lowerLimit->node]->r,
	    "update_ypicket", "a cell does not exist between limits" ) ;
	ASSERT( tileNodeG[upperLimit->node]->l <= upperLimit->pt2.lft && 
	    upperLimit->pt2.lft <= tileNodeG[upperLimit->node]->r,
	    "update_ypicket", "a cell does not exist between limits" ) ;
	/* remove patheological cases - zero width nodes */
	if( upperLimit->pt1.rght == upperLimit->pt2.lft ){
	    /* upper node not needed remove */
	    temp->next = upperLimit->next ;
	    temp->next->prev = temp ;
	    Ysafe_free( upperLimit ) ;
	}
	/* bottom case - easier now due to doubly linked list */
	if( lowerLimit->pt2.lft == lowerLimit->pt1.rght ){
	    /* hook precessor of lowerLimit */
	    temp->prev = lowerLimit->prev ;
	    temp->prev->next = temp ;
	    Ysafe_free( lowerLimit ) ;
	}

    } else { /* split a node */
	/* first check case of exactly the same size tiles */
	if( newtile->l == lowerLimit->pt2.lft &&
	    newtile->r == upperLimit->pt1.rght ){
	    /* this picket will be taken over by new ownership */
	    lowerLimit->node = newtile->node ;
	} else if( newtile->l == lowerLimit->pt2.lft || 
	    newtile->r == upperLimit->pt1.rght ){
	    /* two picket case */

	    /* insert one extra node above the lowerLimit */
	    t = lowerLimit->next ; /* save old pointer */
	    temp = lowerLimit->next = 
		(PICKETPTR) Ysafe_malloc( sizeof(PICKETBOX) ) ;
	    temp->next = t ;
	    if( t ){ /* first time it is a special case */
		t->prev = temp ;
	    }
	    temp->prev = lowerLimit ;
	    /* the two nodes are done at this point */

	    if( newtile->l == lowerLimit->pt2.lft  ){
		/* lower limit is going to be replaced with newtile */
		/* need to copy data to upper tile */
		temp->node = lowerLimit->node ;
		temp->pt1.rght = lowerLimit->pt1.rght ;
		temp->pt2.lft = newtile->r ;
		lowerLimit->node = newtile->node ;
		lowerLimit->pt1.rght = newtile->r ;
		lowerLimit->pt2.lft = newtile->l ;
	    } else { /* newtile->r == lowerLimit->pt1.rght */
		/* no need to copy data to upper tile */
		temp->node = newtile->node ;
		temp->pt1.rght = newtile->r ;
		temp->pt2.lft = newtile->l ;
		lowerLimit->pt1.rght = newtile->l ;
	    }
	} else { /* three picket case */

	    /* insert an upper node */
	    t = lowerLimit->next ; /* save old pointer */
	    temp = lowerLimit->next = 
		(PICKETPTR) Ysafe_malloc( sizeof(PICKETBOX) ) ;
	    temp->next = t ;
	    if( t ){ /* first time it is a special case */
		t->prev = temp ;
	    }
	    temp->prev = lowerLimit ;
	    temp->pt1.rght = lowerLimit->pt1.rght ;
	    temp->pt2.lft = newtile->r ;
	    temp->node = lowerLimit->node ;
	    /* insert middle node */
	    t = lowerLimit->next ; /* save old pointer */
	    temp = lowerLimit->next = 
		(PICKETPTR) Ysafe_malloc( sizeof(PICKETBOX) ) ;
	    temp->next = t ;
	    t->prev = temp ;
	    temp->prev = lowerLimit ;
	    temp->pt1.rght = newtile->r ;
	    temp->pt2.lft = newtile->l ;
	    temp->node = newtile->node ;
	    lowerLimit->pt1.rght = newtile->l ;
	    ASSERT( tileNodeG[temp->node]->l <= temp->pt2.lft && 
		temp->pt2.lft <= tileNodeG[temp->node]->r,
		"update_ypicket", "a cell does not exist between limits" ) ;
	    ASSERT( tileNodeG[lowerLimit->node]->l <= lowerLimit->pt1.rght && 
		lowerLimit->pt1.rght <= tileNodeG[lowerLimit->node]->r,
		"update_ypicket", "a cell does not exist between limits" ) ;
	} /* end else for three picket case */
    }
}/* end picket update */

/* sort by y first then x */
sortbyYX( tileA , tileB )
COMPACTPTR *tileA , *tileB ;

{
    if( (*tileA)->b != (*tileB)->b ){
	return( (*tileA)->b - (*tileB)->b ) ;
    } else {
	/* if ymins are equal sort by xmins */
	return( (*tileA)->l - (*tileB)->l ) ;
    }
}

static yforwardPath()
{

    INT j ;			/* current tile adjacent to node */
    INT node ;			/* current node popped from the queue */
    INT setValue ;		/* the value of the path to this adj node */
    INT currentValue ;		/* path value of node popped from queue */
    COMPACTPTR nextptr ;	/* the tile record of the adj. node */
    ECOMPBOXPTR ptr ;		/* used to traverse edges of popped node */
    QUEUEPTR botqueue, queue ;	/* used to implement queue MACRO */

    /* --------------------------------------------------------------- 
       longestyPath uses breadth first search to find path.
       All QUEUE functions are combined into macros for speed and
       easier reading of the algorithm. See compact.h for details.
    */
    /* FORWARD DIRECTION - SOURCE TO SINK */
    load_ancestors( YFORWARD ) ;

    INITQUEUE( queue, YSOURCE ) ;
    while( queue ){
    	node = TOPQUEUE( queue ) ;
	currentValue = tileNodeG[node]->yvalueMin ;
	for( ptr = tileNodeG[node]->yadjF; ptr ; ptr = ptr->next ){
	    j = ptr->node ;
	    nextptr = tileNodeG[j] ;
	    /* calculate maximum of test condition */
	    setValue = currentValue + ptr->constraint ;
	    if( !(nextptr->moved) && nextptr->yvalueMin < setValue ) {
		nextptr->yvalueMin = setValue ;
		nextptr->pathy = node ;
	    }
	    if( --ancestorG[j] == 0 ){
		/* ready to add to queue when number ancestors = 0 */
		ADD2QUEUE( queue, j ) ;
	    }
	} /* end adjacency loop */
    } /* end loop on queue */
} /* end yforwardPath */


static ybackwardPath()
{

    INT j ;			/* current tile adjacent to node */
    INT node ;			/* current node popped from the queue */
    INT setValue ;		/* the value of the path to this adj node */
    INT currentValue ;		/* path value of node popped from queue */
    COMPACTPTR nextptr ;	/* the tile record of the adj. node */
    ECOMPBOXPTR ptr ;		/* used to traverse edges of popped node */
    QUEUEPTR botqueue, queue ;	/* used to implement queue MACRO */

    /* --------------------------------------------------------------- 
       longestyPath uses breadth first search to find path.
       All QUEUE functions are combined into macros for speed and
       easier reading of the algorithm. See compact.h for details.
    */
    /* FORWARD DIRECTION - SOURCE TO SINK */
    load_ancestors( YBACKWARD ) ;

    /* REVERSE DIRECTION - SINK TO SOURCE */
    INITQUEUE( queue, YSINK ) ;
    while( queue ){
    	node = TOPQUEUE( queue ) ;
	currentValue = tileNodeG[node]->yvalueMax ;
	for( ptr = tileNodeG[node]->yadjB; ptr ; ptr = ptr->next ){
	    j = ptr->node ;
	    nextptr = tileNodeG[j] ;
	    /* calculate maximum of test condition */
	    setValue = currentValue + ptr->constraint ;
	    if( !(nextptr->moved) && nextptr->yvalueMax < setValue ) {
		nextptr->yvalueMax = setValue ;
		nextptr->pathy = node ;
	    }
	    if( --ancestorG[j] == 0 ){
		/* ready to add to queue when number ancestors = 0 */
		ADD2QUEUE( queue, j ) ;
	    }
	}
    }
} /* end ybackwardPath */


INT longestyPath( find_path )
BOOL find_path ;
{

    INT cell ;			/* current cell in question */
    INT tile ;			/* one of the tiles of the adj. cell */
    INT count ; 		/* number of iterations */
    INT center ;		/* where center of cell is relative to path */
    INT length ;		/* length of longest path */
    INT setValue ;		/* the value of the path to this adj node */
    INT siblingCenter ;		/* the value of the path to sibling of adj node */
    BOOL need_to_iterate_path ; /* if TRUE perform another round of longest path*/
    BOOL need_to_update_tiles;  /* whether tile y values need to be updated */
    NODEPTR nptr ;		/* used to traverse multi tiles */
    CELLBOXPTR cptr ;		/* the current cell */
    COMPACTPTR sibling ;	/* the tile record of the sibling to adj node */
    COMPACTPTR nextptr ;	/* the tile record of the adj. node */
    ECOMPBOXPTR ptr ;		/* used to traverse edges of popped node */

    /* --------------------------------------------------------------- 
       longestyPath uses breadth first search to find path.
       All QUEUE functions are combined into macros for speed and
       easier reading of the algorithm. See compact.h for details.
    */
    /* FORWARD DIRECTION - SOURCE TO SINK */
    /* First check to make sure all the ancestors are correct */
    D( "mc_compact/yanc",
	check_yancestors() ;
    ) ;

    /***********************************************************
    * The longest path algorithm in the FORWARD direction.
    ***********************************************************/
    need_to_iterate_path = TRUE ;
    count = 0 ;
    while( need_to_iterate_path ){

	yforwardPath() ;

	/***********************************************************
	* The code below looks to see if we need to iterate because
	* of multiple cells.
	***********************************************************/
	need_to_iterate_path = FALSE ;
	for( cell = 1; cell <= numcellsG ; cell++ ){

	    cptr = cellarrayG[cell] ;
	    if( cptr->multi ){
		nptr = cptr->tiles;
		tile = nptr->node ;
		nextptr = tileNodeG[tile] ;
		center = nextptr->yvalueMin - nextptr->b_rel ;
		need_to_update_tiles = FALSE ;

		/* look for the maximimum of all the siblings */
		/* if all the centers are the same there is no */
		/* need to update the forward path for this cell */
		for( nptr = nptr->next;nptr;nptr=nptr->next ){
		    tile = nptr->node ;
		    /* see if we need to update reverse edges */
		    sibling = tileNodeG[tile] ;
		    siblingCenter = sibling->yvalueMin - sibling->b_rel ;
		    if( center != siblingCenter ){
			need_to_update_tiles = TRUE ;
			need_to_iterate_path = TRUE ;
			if( center < siblingCenter ){
			    center = siblingCenter ;
			}
		    }
		}
		if( need_to_update_tiles ){
		    for( nptr = cptr->tiles;nptr;nptr=nptr->next ){
			tile = nptr->node ;
			sibling = tileNodeG[tile] ;
			sibling->yvalueMin = center + sibling->b_rel ;
		    }
		}
	    }
	}
	D( "mc_compact/iterate", 
	    count++ ;
	) ;
    } /* end loop on longest path for forward loop including fixed cells. */
    D( "mc_compact/iterate", 
	fprintf( stderr, "It took %d time[s] to converge in y forward graph\n\n",
	    count ) ;
    ) ;
    /***********************************************************
    * At this point we have iterated to get the longest path
    * Now measure it.
    ***********************************************************/
    if( find_path ){
	length = path(YFORWARD) ;
    }

    /***********************************************************
    * The longest path algorithm in the REVERSE direction.
    ***********************************************************/
    need_to_iterate_path = TRUE ;
    count = 0 ;
    while( need_to_iterate_path ){

	/* find the longest path backward */
	ybackwardPath() ;

	/***********************************************************
	* The code below looks to see if we need to iterate because
	* of multiple cells.
	***********************************************************/
	need_to_iterate_path = FALSE ;
	for( cell = 1; cell <= numcellsG ; cell++ ){

	    cptr = cellarrayG[cell] ;
	    if( cptr->multi ){
		nptr = cptr->tiles;
		tile = nptr->node ;
		nextptr = tileNodeG[tile] ;
		center = nextptr->yvalueMax + nextptr->b_rel ;
		need_to_update_tiles = FALSE ;

		/* look for the maximimum of all the siblings */
		/* if all the centers are the same there is no */
		/* need to update the forward path for this cell */
		for( nptr = nptr->next;nptr;nptr=nptr->next ){
		    tile = nptr->node ;
		    /* see if we need to update reverse edges */
		    sibling = tileNodeG[tile] ;
		    siblingCenter = sibling->yvalueMax + sibling->b_rel ;
		    if( center != siblingCenter ){
			need_to_update_tiles = TRUE ;
			need_to_iterate_path = TRUE ;
			if( center < siblingCenter ){
			    center = siblingCenter ;
			}
		    }
		}
		if( need_to_update_tiles ){
		    for( nptr = cptr->tiles;nptr;nptr=nptr->next ){
			tile = nptr->node ;
			sibling = tileNodeG[tile] ;
			sibling->yvalueMax = center - sibling->b_rel ;
		    }
		}
	    }
	}
	D( "mc_compact/iterate", 
	    count++ ;
	) ;
    } /* end loop on longest path for reverse loop including fixed cells. */
    D( "mc_compact/iterate", 
	fprintf( stderr, "It took %d time[s] to converge in y backward graph\n\n",
	    count ) ;
    ) ;
    /***********************************************************
    * At this point we have iterated to get the longest path
    * in the reverse direction.  Now measure it.
    ***********************************************************/
    if( find_path ){
	ASSERT( length == path(YBACKWARD),
	    "longestyPath", "Longest path doesn't agree forward & back") ;
    }

    return( length ) ;
} /* end longestyPath */


dypick()
{
    PICKETPTR curPick ;
    printf("Bottom to top pickets:\n" ) ;
    for( curPick=leftPickS;curPick;curPick=curPick->next){
	printf("Node:%2d cell:%2d left:%4d right:%4d\n",
	    curPick->node,
	    tileNodeG[curPick->node]->cell,
	    curPick->pt2.lft,
	    curPick->pt1.rght ) ;
    }
}
