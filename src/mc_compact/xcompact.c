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
FILE:	    xcompact.c
DESCRIPTION:This file contains type declarations for the compaction
	    in the xdirection.
CONTENTS:   
DATE:	    Apr  8, 1988 
REVISIONS:  Sep 20, 1988 - removed excess edges from source and sink
	    Oct 29, 1988 - fixed problem with cells being exactly same
		size as picket and changed picket to doubly-linked list.
	    Nov  7, 1988 - implemented the doubly-linked list correctly.
	    Jan 29, 1989 - changed msg to YmsgG and added \n's.
	    Mar 11, 1989 - fixed argument type bug in project functions.
	    Apr 22, 1989 - modified to make multiple cell tiles work.
	    Apr 25, 1989 - give quicksort two tries to order tiles 
		expanding core if necessary.
	    May  3, 1989 - changed to Y prefixes.
	    May  6, 1989 - added no graphics compile switch
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) xcompact.c version 7.2 11/10/90" ;
#endif

#include <compact.h>
#include <yalecad/debug.h>
#include <yalecad/message.h>


static PICKETPTR  botPickS ;

buildXGraph()
{
    int i ;                    /* counter */
    int overlapx ;             /* overlap conditions in x direction */
    int overlapy ;             /* overlap conditions in y direction */
    int sortbyXY() ;           /* sort the tiles X then Y */
    int left, right ;          /* coordinates of tiles */
    int bottom, top ;          /* coordinates of tiles */
    BOOL firstPick ;           /* TRUE if first picket which matches */
    BOOL possibleEdgetoSource; /* TRUE if this could be edge to source */
    BOOL *xancestorB ;         /* TRUE for a cell if cell has B ancestors */
    BOOL *xancestorF ;         /* TRUE for a cell if cell has F ancestors */
    COMPACTPTR candidate ;     /* this is the tile in question */
    COMPACTPTR t ;             /* this is the current picket */
    PICKETPTR freepick ;       /* used to free the pickets at the end */
    PICKETPTR curPick ;        /* traverse the pickets */
    PICKETPTR lowerLimit ;     /* first picket tile overlaps/touches */
    PICKETPTR upperLimit ;     /* last picket tile overlaps or touches */

    initxPicket() ;
    /* xGraphG is now initialized */

    xancestorB = (BOOL *) Ysafe_calloc( last_cellG+1,sizeof(BOOL) ) ;
    xancestorF = (BOOL *) Ysafe_calloc( last_cellG+1,sizeof(BOOL) ) ;

    /* sort by xmin ymin of the bounding box */
    /* we give it two chances - second time we expand core if necessary */
    for( i=0; i <= 1 ; i++ ){
	Yquicksort((char *)xGraphG,numtilesG+2,sizeof(COMPACTPTR),sortbyXY);
	if( xGraphG[SOURCE]->cell == XSOURCEC && 
	xGraphG[SINK]->cell == XSINKC ){
	    break ;
	} else {
	    find_core( &left, &right, &bottom, &top ) ;
	    /* expand core region */
	    t = tileNodeG[XSOURCE] ;
	    t->l = left ;
	    t->r = t->l ;
	    t = tileNodeG[XSINK] ;
	    t->l = right ;
	    t->r = t->l ;
	}
    }
    if( xGraphG[SOURCE]->cell != XSOURCEC || xGraphG[SINK]->cell != XSINKC ){
	M( ERRMSG, "xcompact", "Fatal error in configuration\n" ) ;
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
	candidate = xGraphG[i] ;

	/* search thru picket list for adjacencies */
	for( curPick=botPickS;curPick;curPick=curPick->next){
	    ASSERT( dcheckPicks(), "buildXGraph", "pickets are bad" ) ;
	    overlapy = projectY( curPick->pt2.bot, curPick->pt1.top,
		candidate->b, candidate->t) ;
		
	    if( overlapy > 0 ){ /* overlap positive means real overlap */

		/* save span of overlap */
		if( firstPick ){
		    lowerLimit = curPick ;
		    firstPick = FALSE ;
		}
		upperLimit = curPick ;

		t = tileNodeG[curPick->node] ;

		/* check for multiple tiles - no errors possible */
		if( candidate->cell == t->cell ){
		    continue ;
		} 


		/* now check for errors */
		overlapx = projectX(t->l,t->r,candidate->l,candidate->r);
		if( overlapy > 0 && overlapx > 0 ){
		    D( "mc_compact/buildXGraph", sprintf( YmsgG, 
			"Overlap detected: cell %d (tile:%d) and cell %d (tile:%d)\n",
			candidate->cell, candidate->node, t->cell, t->node ) ) ;
		    D( "mc_compact/buildXGraph", M( MSG, NULL, YmsgG ) ) ;
		}
		ASSERT( t->node == curPick->node, "buildCGraph",
		    "tileNodeG != curPick. Problem \n" ) ;
		/* form edge on only first occurance of cell */
		if( t->node == 0 ){ /* source node */
		    /* delay adding this edge */
		    possibleEdgetoSource = TRUE ;
		} else {
		    formxEdge( t->node, candidate->node ) ;
		}
	    } else if( upperLimit ){
		/* we are past the upper limit so break & save time */
		break ;
	    }
	}
	ASSERT( lowerLimit, "compact", "lowerLimit is NULL" ) ;
	ASSERT( upperLimit, "compact", "upperLimit is NULL" ) ;

	if( possibleEdgetoSource && xancestorF[candidate->cell] == FALSE ){
	    /* no need to make an edge to source if we already */
	    /* have an ancestor.  Always make sure it is one of the lowest */
	    /* nodes of the multi-tiled cell */
	    formxEdge( 0, cellarrayG[candidate->cell]->xlo ) ;
	    xancestorF[candidate->cell] = TRUE ;
	}

	update_xpicket( i, lowerLimit, upperLimit ) ;

    } /* end for loop */


    /* process sink last */
    /* search thru picket list for adjacencies */
    for( curPick=botPickS;curPick;curPick=curPick->next){

	/* remaining pickets must necessarily overlap sink */
	t = tileNodeG[curPick->node] ;
	ASSERT( t->node == curPick->node, "buildXGraph",
	    "tileNodeG != curPick. Problem \n" ) ;
	if( curPick->node != 0 &&  /* avoid source */
	    /* make sure sink is one of the highest nodes of the cell */
	    xancestorB[t->cell] == FALSE ){ /* no parents */
	    formxEdge( cellarrayG[t->cell]->xhi, xGraphG[last_tileG]->node ) ;
	    xancestorB[t->cell] = TRUE ;
	}
    }
    D( "mc_compact/buildXGraph", M( MSG, NULL, "\n" ) ) ;

    /* now add multiple tile edges to graph. Precomputed in multi.c */
    add_mtiles_to_xgraph() ;

    ASSERT( dxancestors( last_tileG, xGraphG ),"xcompact",
	"Number of ancestors does not match edges." ) ;
    
    cleanupGraph( XFORWARD ) ;
    cleanupGraph( XBACKWARD ) ;
    Ysafe_free( xancestorB ) ;
    Ysafe_free( xancestorF ) ;

    ASSERT( dxancestors( last_tileG, xGraphG ),"xcompact",
	"Number of ancestors does not match edges." ) ;

    /* delete all pickets */
    for( curPick = botPickS; curPick ; ) {
	freepick = curPick ;
	curPick = curPick->next ;
	Ysafe_free( freepick ) ;
    }

} /* end buildXGraph */

formxEdge( fromNode, toNode ) 
int fromNode ;
int toNode ;
{
    COMPACTPTR e1, e2 ;
    ECOMPBOXPTR temp, newE ;
    int constraint ; /* contraint between tiles */

    e1 = tileNodeG[fromNode] ;
    D( "mc_compact/formxEdge",
	sprintf( YmsgG, "e1 is NULL; fromNode: %d\n", fromNode ) ) ;
    ASSERT( e1, "formxEdge", YmsgG ) ;

    e2 = tileNodeG[toNode] ;
    D( "mc_compact/formxEdge",
	sprintf( YmsgG, "e2 is NULL; toNode: %d\n", toNode ) ) ;
    ASSERT( e2, "formxEdge", YmsgG ) ;

    for( temp = e1->xadjF; temp; temp = temp->next ){
	if( temp->node == toNode ){
	    return ;
	}
    }

    /* create forward edge */
    if( temp = e1->xadjF ){
	newE = e1->xadjF = (ECOMPBOXPTR) Ysafe_malloc(sizeof(ECOMPBOX)) ;
	/* hook to rest of list */
	newE->next = temp ;
    } else {
	/* start adjacency list */
	newE = e1->xadjF = (ECOMPBOXPTR) Ysafe_malloc(sizeof(ECOMPBOX)) ;
	newE->next = NULL ;
    }
    newE->node = toNode ;
    newE->marked = FALSE ;

    /* calculate the forward constraint */
    /* This is a left edge algorithm so constraint is span of from tile */
    /* one xspaceG spacing unless same cell */
    if( e1->cell != e2->cell ){
	constraint = e1->r_rel - e1->l_rel + xspaceG ; /* span of tile */
    } else {
	/* this removes problem of overlapping tiles on multitiles */
	constraint = ABS( e2->l_rel - e1->l_rel ) ;
    }
    newE->constraint = constraint ;
    ASSERT( newE->constraint >= 0, "formxEdge", YmsgG ) ;

    e1->xancestrB++ ;  /* update ancestor count for longest path */

    /* create backward edge */
    if( temp = e2->xadjB ){
	newE = e2->xadjB = (ECOMPBOXPTR) Ysafe_malloc(sizeof(ECOMPBOX)) ;
	/* hook to rest of list */
	newE->next = temp ;
    } else {
	/* start adjacency list */
	newE = e2->xadjB = (ECOMPBOXPTR) Ysafe_malloc(sizeof(ECOMPBOX)) ;
	newE->next = NULL ;
    }
    newE->node = fromNode ;
    newE->marked = FALSE ;

    newE->constraint = constraint ;
    e2->xancestrF++ ; /* update ancestor count for longest path */

    D( "mc_compact/formxEdge",
	sprintf( YmsgG, "Edge %d - %d has a negative edge weight\n", 
	fromNode, toNode ) ) ;
    ASSERT( newE->constraint >= 0, "formxEdge", YmsgG ) ;
}

initxPicket() 
{
    COMPACTPTR sink, source, node ;
    int i ;

    /* initialize all paths and values for all tiles */
    for( i=0; i<= last_tileG ; i++ ) {
	node = xGraphG[i] ;
	node->pathx = 0 ;
	node->pathy = 0 ;
	node->xvalueMin = INT_MIN ;
	node->xvalueMax = INT_MIN ;
	node->xancestrF = 0 ; 
	node->xancestrB = 0 ; 
	node->criticalX = FALSE ;
	node->moved = FALSE ;
    }
    source = xGraphG[0] ;
    source->xvalueMin = 0 ;
    source->xvalueMax = 0 ;

    sink = xGraphG[last_tileG] ;
    sink->xvalueMax = 0 ;

    /* initialize picket to source */
    botPickS = (PICKETPTR) Ysafe_malloc( sizeof(PICKETBOX) ) ;
    botPickS->pt1.top = xGraphG[0]->t ;
    botPickS->pt2.bot = xGraphG[0]->b ;
    botPickS->node = xGraphG[0]->node ;
    botPickS->next = NULL ;
    botPickS->prev = NULL ;

} /* end initxPicket */
    


update_xpicket( i, lowerLimit, upperLimit )
int i ;
PICKETPTR lowerLimit, upperLimit ;
{
    PICKETPTR t, temp, curPick ;
    COMPACTPTR newtile ;         /* new tile to be added to picket */

    /* for speed */
    newtile = xGraphG[i] ;

    if( newtile->b == lowerLimit->pt1.top ){
	/* increment the lower limit */
	lowerLimit = lowerLimit->next ;
    }

    /* update picket */
    if( lowerLimit != upperLimit ){
	/* first reset limits if cell i coincides with limits */
	if( newtile->b == lowerLimit->pt2.bot ){
	    /* this tile will dissappear */
	    lowerLimit = lowerLimit->prev ;
	}
	if( newtile->t == upperLimit->pt1.top ){
	    /* this tile will dissappear */
	    upperLimit = upperLimit->next ;
	}
	/* insert node */
	t = lowerLimit->next ; /* save old pointer to free it later */
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
	/* Bottom link is now done. Now update picket information */
	/* form overlapping pickets */
	/* set lower limit */
	lowerLimit->pt1.top = newtile->b ;
	/* set upper limit */
	upperLimit->pt2.bot = newtile->t ;
	/* fill current picket */
	temp->pt2.bot = newtile->b ;
	temp->pt1.top = newtile->t ;
	temp->node = newtile->node ;
	/* fix up the upper link */
	temp->next = upperLimit ;
	upperLimit->prev = temp ;
	/* remove patheological cases - zero width nodes */
	if( upperLimit->pt1.top == upperLimit->pt2.bot ){
	    /* upper node not needed remove */
	    temp->next = upperLimit->next ;
	    temp->next->prev = temp ;
	    Ysafe_free( upperLimit ) ;
	}
	/* bottom case - easier now due to doubly linked list */
	if( lowerLimit->pt2.bot == lowerLimit->pt1.top ){
	    /* hook to  precessor of lowerLimit */
	    temp->prev = lowerLimit->prev ;
	    temp->prev->next = temp ;
	    Ysafe_free( lowerLimit ) ;
	}

    } else { /* split a node */
	/* first check case of exactly the same size tiles */
	if( newtile->b == lowerLimit->pt2.bot && 
	    newtile->t == lowerLimit->pt1.top ){
	    /* this picket will be taken over by new ownership */
	    lowerLimit->node = newtile->node ;

	} else if( newtile->b == lowerLimit->pt2.bot ||
	 	   newtile->t == lowerLimit->pt1.top ){
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

	    if( newtile->b == lowerLimit->pt2.bot  ){
		/* lower limit is going to be replaced with newtile */
		/* need to copy data to upper tile */
		temp->node = lowerLimit->node ;
		temp->pt1.top = lowerLimit->pt1.top ;
		temp->pt2.bot = newtile->t ;
		lowerLimit->node = newtile->node ;
		lowerLimit->pt1.top = newtile->t ;
		lowerLimit->pt2.bot = newtile->b ;
	    } else { /* newtile->t == lowerLimit->pt1.top */
		/* no need to copy data to upper tile */
		temp->node = newtile->node ;
		temp->pt1.top = newtile->t ;
		temp->pt2.bot = newtile->b ;
		lowerLimit->pt1.top = newtile->b ;
	    }


	} else { /* three node case */
	    /* insert upper node - the same node */
	    t = lowerLimit->next ; /* save old pointer */
	    temp = lowerLimit->next = 
		(PICKETPTR) Ysafe_malloc( sizeof(PICKETBOX) ) ;
	    temp->next = t ;
	    if( t ){ /* first time it is a special case */
		t->prev = temp ;
	    }
	    temp->prev = lowerLimit ;
	    temp->pt1.top = lowerLimit->pt1.top ;
	    temp->pt2.bot = newtile->t ;
	    temp->node = lowerLimit->node ;
	    /* insert middle node - the new node */
	    t = lowerLimit->next ; /* save old pointer */
	    temp = lowerLimit->next = 
		(PICKETPTR) Ysafe_malloc( sizeof(PICKETBOX) ) ;
	    temp->next = t ;
	    t->prev = temp ;
	    temp->prev = lowerLimit ;
	    temp->pt1.top = newtile->t ;
	    temp->pt2.bot = newtile->b ;
	    temp->node = newtile->node ;
	    lowerLimit->pt1.top = newtile->b ;
	}
    }
}/* end picket update */

/* function returns whether one cell projects onto another */
int projectX( tile1_left, tile1_right, tile2_left, tile2_right )
int tile1_left, tile1_right, tile2_left, tile2_right ;
{
    /* -----------------------------------------------------
       First check case 2nd tile larger than first 
	complete overlap
    */
    if( tile2_left <= tile1_left && tile1_right <= tile2_right ){
	return( OVERLAP1 ) ;

    /* -----------------------------------------------------
       Check if an edge of tile two is encompassed by tile 1 
       Second check left edge of tile2 :
	tile1_left <= tile2_left < tile1_right + xspaceG
    */
    } else if( tile1_left<=tile2_left&&tile2_left<tile1_right+xspaceG ){
	return( OVERLAP2 ) ;
    /* -----------------------------------------------------
       Third check right edge of tile2 :
	tile1_left - xspaceG < tile2_right < tile1_right 
    */
    } else if( tile1_left-xspaceG<tile2_right&&tile2_right<=tile1_right){
	return( OVERLAP3 ) ;

    /* -----------------------------------------------------
       Fourth case tiles touching.
    */
    } else if( tile2_left == tile1_right + xspaceG || 
	       tile1_left - xspaceG == tile2_right ){
	return( TOUCH ) ;

    } else {
	return( NOTOUCH ) ;
    }
} /* end projectX */

/* function returns whether one cell projects onto another */
int projectY( tile1_bot, tile1_top, tile2_bot, tile2_top )
int tile1_bot, tile1_top, tile2_bot, tile2_top ;
{
    /* -----------------------------------------------------
       First check to see if 2nd tile larger than first 
    */
    if( tile2_bot <= tile1_bot && tile1_top <= tile2_top ){
	return( OVERLAP1 ) ;

    /* -----------------------------------------------------
       Check if an edge of tile two is encompassed by tile 1 
       Second check bottom edge of tile2 :
	tile1_bot <= tile2_bot < tile1_top 
    */
    } else if( tile1_bot <= tile2_bot && tile2_bot < tile1_top+yspaceG){

	return( OVERLAP2 ) ;
    /* -----------------------------------------------------
       Third check top edge of tile2 :
	tile1_bot < tile2_top <= tile1_top 
    */
    } else if( tile1_bot-yspaceG < tile2_top && tile2_top <= tile1_top ){
	return( OVERLAP3 ) ;

    /* -----------------------------------------------------
       Fourth case tiles touching.
    */
    } else if( tile2_bot == tile1_top + yspaceG || 
	       tile1_bot - yspaceG == tile2_top ){
	return( TOUCH ) ;

    } else {
	return( NOTOUCH ) ; /* no touch or overlap */ 
    }
}/* end projectY */

/* sort by x first then y */
sortbyXY( tileA , tileB )
COMPACTPTR *tileA , *tileB ;

{
    if( (*tileA)->l != (*tileB)->l ){
	return( (*tileA)->l - (*tileB)->l ) ;
    } else {
	/* if xmins are equal sort by ymins */
	return( (*tileA)->b - (*tileB)->b ) ;
    }
}

load_ancestors( direction )
INT direction ;
{
    INT i ;			/* counter */
    INT last ;			/* last tile in tileNode array */

    last = YSINK ;
    switch( direction ){
	case XFORWARD:
	    for( i = 0; i <= last; i++ ){
		ancestorG[i] = tileNodeG[i]->xancestrF ;
	    }
	    break ;
	case XBACKWARD:
	    for( i = 0; i <= last; i++ ){
		ancestorG[i] = tileNodeG[i]->xancestrB ;
	    }
	    break ;
	case YFORWARD:
	    for( i = 0; i <= last; i++ ){
		ancestorG[i] = tileNodeG[i]->yancestrF ;
	    }
	    break ;
	case YBACKWARD:
	    for( i = 0; i <= last; i++ ){
		ancestorG[i] = tileNodeG[i]->yancestrB ;
	    }
	    break ;
    }
}

static xforwardPath()
{

    INT j ;			/* current tile adjacent to node */
    INT node ;			/* current node popped from the queue */
    INT setValue ;		/* the value of the path to this adj node */
    INT currentValue ;		/* path value of node popped from queue */
    COMPACTPTR nextptr ;	/* the tile record of the adj. node */
    ECOMPBOXPTR ptr ;		/* used to traverse edges of popped node */
    QUEUEPTR botqueue, queue ;	/* used to implement queue MACRO */

    /* --------------------------------------------------------------- 
       longestxPath uses breadth first search to find path.
       All QUEUE functions are combined into macros for speed and
       easier reading of the algorithm. See compact.h for details.
    */
    /* FORWARD DIRECTION - SOURCE TO SINK */
    load_ancestors( XFORWARD ) ;

    INITQUEUE( queue, XSOURCE ) ;
    while( queue ){
    	node = TOPQUEUE( queue ) ;
	currentValue = tileNodeG[node]->xvalueMin ;
	for( ptr = tileNodeG[node]->xadjF; ptr ; ptr = ptr->next ){
	    j = ptr->node ;
	    nextptr = tileNodeG[j] ;
	    /* calculate maximum of test condition */
	    setValue = currentValue + ptr->constraint ;
	    if( !(nextptr->moved) && nextptr->xvalueMin < setValue ) {
		nextptr->xvalueMin = setValue ;
		nextptr->pathx = node ;
	    }
	    if( --ancestorG[j] == 0 ){
		/* ready to add to queue when number ancestors = 0 */
		ADD2QUEUE( queue, j ) ;
	    }
	} /* end adjacency loop */
    } /* end loop on queue */
} /* end xforwardPath */

static xbackwardPath()
{

    INT j ;			/* current tile adjacent to node */
    INT node ;			/* current node popped from the queue */
    INT setValue ;		/* the value of the path to this adj node */
    INT currentValue ;		/* path value of node popped from queue */
    COMPACTPTR nextptr ;	/* the tile record of the adj. node */
    ECOMPBOXPTR ptr ;		/* used to traverse edges of popped node */
    QUEUEPTR botqueue, queue ;	/* used to implement queue MACRO */

    /* --------------------------------------------------------------- 
       longestxPath uses breadth first search to find path.
       All QUEUE functions are combined into macros for speed and
       easier reading of the algorithm. See compact.h for details.
    */
    /* FORWARD DIRECTION - SOURCE TO SINK */
    load_ancestors( XBACKWARD ) ;

    /* REVERSE DIRECTION - SINK TO SOURCE */
    INITQUEUE( queue, XSINK ) ;
    while( queue ){
    	node = TOPQUEUE( queue ) ;
	currentValue = tileNodeG[node]->xvalueMax ;
	for( ptr = tileNodeG[node]->xadjB; ptr ; ptr = ptr->next ){
	    j = ptr->node ;
	    nextptr = tileNodeG[j] ;
	    /* calculate maximum of test condition */
	    setValue = currentValue + ptr->constraint ;
	    if( !(nextptr->moved) && nextptr->xvalueMax < setValue ) {
		nextptr->xvalueMax = setValue ;
		nextptr->pathx = node ;
	    }
	    if( --ancestorG[j] == 0 ){
		/* ready to add to queue when number ancestors = 0 */
		ADD2QUEUE( queue, j ) ;
	    }
	}
    }
} /* end xbackwardPath */

INT longestxPath( find_path )
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
    BOOL need_to_update_tiles;  /* whether tile x values need to be updated */
    NODEPTR nptr ;		/* used to traverse multi tiles */
    CELLBOXPTR cptr ;		/* the current cell */
    COMPACTPTR sibling ;	/* the tile record of the sibling to adj node */
    COMPACTPTR nextptr ;	/* the tile record of the adj. node */
    ECOMPBOXPTR ptr ;		/* used to traverse edges of popped node */

    /* --------------------------------------------------------------- 
       longestxPath uses breadth first search to find path.
       All QUEUE functions are combined into macros for speed and
       easier reading of the algorithm. See compact.h for details.
    */
    /* FORWARD DIRECTION - SOURCE TO SINK */
    /* First check to make sure all the ancestors are correct */
    D( "mc_compact/xanc",
	check_xancestors() ;
    ) ;

    /***********************************************************
    * The longest path algorithm in the FORWARD direction.
    ***********************************************************/
    count = 0 ;
    need_to_iterate_path = TRUE ;
    while( need_to_iterate_path ){

	xforwardPath() ;

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
		center = nextptr->xvalueMin - nextptr->l_rel ;
		need_to_update_tiles = FALSE ;

		/* look for the maximimum of all the siblings */
		/* if all the centers are the same there is no */
		/* need to update the forward path for this cell */
		for( nptr = nptr->next;nptr;nptr=nptr->next ){
		    tile = nptr->node ;
		    /* see if we need to update reverse edges */
		    sibling = tileNodeG[tile] ;
		    siblingCenter = sibling->xvalueMin - sibling->l_rel ;
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
			sibling->xvalueMin = center + sibling->l_rel ;
		    }
		}
	    }
	}
	D( "mc_compact/iterate", 
	    count++ ;
	) ;
    } /* end loop on longest path for forward loop including fixed cells. */
    D( "mc_compact/iterate", 
	fprintf( stderr, "It took %d time[s] to converge in x forward graph\n\n",
	    count ) ;
    ) ;
    /***********************************************************
    * At this point we have iterated to get the longest path
    * Now measure it.
    ***********************************************************/
    if( find_path ){
	length = path(XFORWARD) ;
    }


    /***********************************************************
    * The longest path algorithm in the REVERSE direction.
    ***********************************************************/
    need_to_iterate_path = TRUE ;
    count = 0 ;
    while( need_to_iterate_path ){

	/* find the longest path backward */
	xbackwardPath() ;

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
		center = nextptr->xvalueMax + nextptr->l_rel ;
		need_to_update_tiles = FALSE ;

		/* look for the maximimum of all the siblings */
		/* if all the centers are the same there is no */
		/* need to update the forward path for this cell */
		for( nptr = nptr->next;nptr;nptr=nptr->next ){
		    tile = nptr->node ;
		    /* see if we need to update reverse edges */
		    sibling = tileNodeG[tile] ;
		    siblingCenter = sibling->xvalueMax + sibling->l_rel ;
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
			sibling->xvalueMax = center - sibling->l_rel ;
		    }
		}
	    }
	}
	D( "mc_compact/iterate", 
	    count++ ;
	) ;
    } /* end loop on longest path for reverse loop including fixed cells. */
    D( "mc_compact/iterate", 
	fprintf( stderr, "It took %d time[s] to converge in x backward graph\n\n",
	    count ) ;
    ) ;
    /***********************************************************
    * At this point we have iterated to get the longest path
    * in the reverse direction.  Now measure it.
    ***********************************************************/
    if( find_path ){
	ASSERT( length == path(XBACKWARD),
	    "longestxPath", "Longest path doesn't agree forward & back") ;
    }

    return( length ) ;
} /* end longestxPath */

dxpick()
{
    PICKETPTR curPick ;
    printf("Bottom to top pickets:\n" ) ;
    for( curPick=botPickS;curPick;curPick=curPick->next){
	printf("Node:%2d cell:%2d bot:%4d top:%4d\n",
	    curPick->node,
	    tileNodeG[curPick->node]->cell,
	    curPick->pt2.bot,
	    curPick->pt1.top ) ;
    }
}

/* verify that the picket list is correct */
BOOL dcheckPicks()
{
    PICKETPTR lastpick, pick_it ;
    int forwardCount, backCount ;

    if( YgetListSize(botPickS,&(botPickS->next)) != -1 ){
	forwardCount = 0 ;
	backCount = 0 ;
	for( pick_it = botPickS;pick_it;pick_it=pick_it->next ){
	    forwardCount++ ;
	    lastpick = pick_it ;
	}
	for( pick_it = lastpick;pick_it;pick_it=pick_it->prev ){
	    backCount++ ;
	}
	if( forwardCount == backCount ){
	    return( TRUE ) ;
	}
    }
    return( FALSE ) ;
}
