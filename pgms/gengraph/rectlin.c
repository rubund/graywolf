/* ----------------------------------------------------------------- 
FILE:	    rectlin.c                                       
DESCRIPTION:This file contains the routines for the converting the nodes
	    of the channel graph to a rectilinear graph.
CONTENTS:   int rectilinear( )
	    BOOL getDirection( node1, node2 ) 
		int node1, node2 ;
	    addEdge( node1, node2, edgeIndex ) 
		int node1,node2, edgeIndex ;
	    findFourCorners( lowerLeftNode, upperLeftNode, 
			     lowerRiteNode, upperRiteNode )
		int *lowerLeftNode, *upperLeftNode,
		*lowerRiteNode, *upperRiteNode ;
DATE:	    Jan 23, 1989 
REVISIONS:  Mar 16, 1989 - added addEdge, getDirection, findFourCorners
		- added library queue routines.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) rectlin.c version 1.2 4/20/90" ;
#endif

#include <geo.h>
#include <yalecad/queue.h>

#include <yalecad/debug.h>

/* ***************************************************************** 
    Convert the channel graph to a rectilinear graph by moving nodes
    such that all nodes in a given direction are aligned.  We do this
    by traversing the graph and finding the MIN containing segment of
    all nodes which lie on a straight path. 
    ALGORITHM:
	Build graph which uses adjacency list for easier traversal of grp.
	Initialize all nodes and edges to be active.
	Find four corners of the chip and put them in a queue
	while( sourceNode = POPQUEUE( Queue ) ) do
	    Determine direction that we should search by
		i) using direction of four special cases corner nodes.
		ii) all other nodes pick random direction.
	    Initialize the span of the sourceNode.

	    node1 = sourceNode ;
	    while able to find active node node2 connected to node1 
		in same direction do

		add node1 to queue if degree of node is 1.
		mark the ancestor of node2 for traceback.
		update the current segment span.
		node1 = node2 ;
	    od.

	    Do same for pathelogical case which should never occur.
	    Pathelogical case occurs if sourceNode is not degree 3.
	    ( except for 4 corner special cases )

	    Determine new position for all nodes in path by following
		ancestor field back to sourceNode and setting xc and yc
		fields.

	od.

	Update all lengths in datastructures based on new xc and yc.

   ***************************************************************** */
rectilinear()
{

    int i , index1 , index2 , length ;
    int node1, node2;
    INT sourceNode ;
    int lastNodeF, lastNodeB ; /* the ends of the paths forward and back*/
    YQUEUE QueueRec, *Queue ;
    ADJPTR curEdge ;
    RECTPTR curNode ;
    int lft, rgh, top, bot ; /* stores current span of feasible pos */
			     /* for center of node */
    int xcenter, ycenter ; /* where to move the position of the node */
    /* the four corner nodes of the chip */
    INT  lowerLeftNode, upperLeftNode, lowerRiteNode, upperRiteNode ;
    int count ; /* counter for number of adjacent edges */
    BOOL HnotV ; /* edge direction Horizontal TRUE - Vertical FALSE */

    /* first build adjacency list in order to traverse graph easier */
    for( i = 1 ; i <= eNumG ; i++ ) {
	if( eArrayG[i].active == TRUE ) {
	    index1 = eArrayG[i].index1 ;
	    index2 = eArrayG[i].index2 ;
	    if( i <= edgeTransitionG ) {
		length = rectArrayG[index2].yc - rectArrayG[index1].yc  ;
	    } else {
		length = rectArrayG[index2].xc - rectArrayG[index1].xc  ;
	    }
	    eArrayG[i].length = length ;

	    /* build channel adjacency list for easier traversal  */
	    addEdge( index1, index2, i ) ;

	}
    }


    /* ----------------------------------------------------------------- 
	initialize nodes and edges to be active
    ----------------------------------------------------------------- */
    for( i = 1 ; i <= eNumG ; i++ ) {
	eArrayG[i].active = TRUE ;
    }
    /* use active field to store number of active edges */
    for( i = 1 ; i <= numRectsG ; i++ ) {
	count = 0 ;
	for( curEdge=rectArrayG[i].adj;curEdge;curEdge=curEdge->next ){
	    count++ ;
	}
	rectArrayG[i].active = count ;
    }

    /* initialize set and queue pointers to memory space */
    Queue = &QueueRec ;

    /* ----------------------------------------------------------------- 
	find four corners of chip for initial source queue.
    ----------------------------------------------------------------- */
    findFourCorners( &lowerLeftNode, &upperLeftNode, 
	 &lowerRiteNode, &upperRiteNode ) ;

    /* add corners to queue */
    /* go clockwise around chip */
    YinitQueue( Queue, (char *) lowerLeftNode ) ;
    Yadd2Queue( Queue, (char *) upperLeftNode ) ;
    Yadd2Queue( Queue, (char *) upperRiteNode ) ;
    Yadd2Queue( Queue, (char *) lowerRiteNode ) ;

    /* init count for corners so they cannot be added again to queue */
    rectArrayG[lowerLeftNode].active = 0 ;
    rectArrayG[upperLeftNode].active = 0 ;
    rectArrayG[lowerRiteNode].active = 0 ;
    rectArrayG[upperRiteNode].active = 0 ;


    /* TOPQUEUE pops the Queue. See library archive for details */
    while( sourceNode = (INT) YtopQueue( Queue ) ){

	D( "rectlin", YdumpQueue( Queue ) ) ;

	/* first the four special cases */
	if( sourceNode == lowerLeftNode ){
	    HnotV = FALSE ; /* first vertical up */ 
	    node1 = sourceNode ;
	    /* x coordinates are of interest */
	    lft = rectArrayG[node1].node_l ;
	    rgh = rectArrayG[node1].node_r ;

	} else if( sourceNode == upperLeftNode ){
	    HnotV = TRUE ; /* next horizontal to right */ 
	    node1 = sourceNode ;
	    /* y coordinates are of interest */
	    bot = rectArrayG[node1].node_b ;
	    top = rectArrayG[node1].node_t ;
	} else if( sourceNode == upperRiteNode ){
	    HnotV = FALSE ; /* third vertical down */ 
	    node1 = sourceNode ;
	    /* x coordinates are of interest */
	    lft = rectArrayG[node1].node_l ;
	    rgh = rectArrayG[node1].node_r ;
	} else if( sourceNode == lowerRiteNode ){
	    HnotV = TRUE ; /* next horizontal to left */ 
	    node1 = sourceNode ;
	    /* y coordinates are of interest */
	    bot = rectArrayG[node1].node_b ;
	    top = rectArrayG[node1].node_t ;

	} else {  /* DETERMINE DIRECTION */

	    /* pick a direction to search by picking a valid active */
	    /* edge and determining direction */
	    for( curEdge=rectArrayG[sourceNode].adj;curEdge;
		curEdge=curEdge->next ){

		if( curEdge->edge->active ){
		    /* found a valid edge */
		    /* mark edge */
		    curEdge->edge->active = FALSE ;
		    break ;
		}
	    }
	    if( !(curEdge) ){
		continue ; /* get another source node */
	    }

	    /* add node to queue */
	    node1 = curEdge->node ;
	    /* only add node to queue when count is one */
	    /* we can do this since only nodes of degree 3 can begin */
	    /* or end path except for corner nodes which we have processed */
	    if( rectArrayG[node1].active == 1 ){
		Yadd2Queue( Queue, (char *)((INT)node1)) ; 
	    }
	    /* update counts for nodes */
	    rectArrayG[sourceNode].active-- ;
	    rectArrayG[node1].active-- ;

	    /* mark return */
	    rectArrayG[node1].from = sourceNode ;

	    /* HnotV is the direction variable shows logic */
	    /* Horizontal TRUE - Vertical FALSE */
	    HnotV = getDirection( sourceNode, node1 ) ;

	    /* initialize segment span */
	    if( HnotV ){ /* horizontal edge */
		/* y coordinates are of interest */
		bot = rectArrayG[sourceNode].node_b ;
		top = rectArrayG[sourceNode].node_t ;
		/* now union with node */
		bot = MAX( bot, rectArrayG[node1].node_b ) ;
		top = MIN( top, rectArrayG[node1].node_t ) ;
	    } else { /* vertical edge */
		/* x coordinates are of interest */
		lft = rectArrayG[sourceNode].node_l ;
		rgh = rectArrayG[sourceNode].node_r ;
		/* now union with node */
		lft = MAX( lft, rectArrayG[node1].node_l ) ;
		rgh = MIN( rgh, rectArrayG[node1].node_r ) ;
	    }
	}
	/* ------------ end find direction --------------- */

	/* FORWARD SEARCH FROM SOURCENODE */
	/* search along the HnotV direction until direction exhausted */
	while( TRUE ){

	    /* continue searching in this direction by picking a */
	    /* another valid active and pursuing it. */
	    for( curEdge=rectArrayG[node1].adj;curEdge;curEdge=curEdge->next ){
		node2 = curEdge->node ;
		if( curEdge->edge->active && 
		    HnotV == getDirection(node1,node2)  ){
		    /* found a valid edge */
		    /* mark edge */
		    curEdge->edge->active = FALSE ;
		    break ;
		}
	    }
	    if( !(curEdge) ){
		/* search exhausted - get off the merry-go-round */
		lastNodeF = node1 ; /* save last forward node */
		break ;
	    }

	    /* update counts for nodes */
	    rectArrayG[node1].active-- ;
	    rectArrayG[node2].active-- ;
	    if( rectArrayG[node1].active == 1 ){
		Yadd2Queue( Queue, (char *)((INT)node1)) ; 
	    }

	    /* mark return */
	    rectArrayG[node2].from = node1 ;

	    /* update segment span */
	    if( HnotV ){ /* horizontal edge */
		/* now union with node2 */
		bot = MAX( bot, rectArrayG[node2].node_b ) ;
		top = MIN( top, rectArrayG[node2].node_t ) ;
	    } else { /* vertical edge */
		/* now union with node2 */
		lft = MAX( lft, rectArrayG[node2].node_l ) ;
		rgh = MIN( rgh, rectArrayG[node2].node_r ) ;
	    }

	    /* go on to next node */
	    node1 = node2 ;

	} /* end loop on nodes of the forward path from source */
	/* END FORWARD SEARCH FROM SOURCENODE */

	/* now check pathelogical case - other side of sourceNode */
	/* normally this exploration should net no new path but */
	/* we include this for completeness checking */
	node1 = sourceNode ;
	while( TRUE ){

	    /* continue searching in this direction by picking a */
	    /* another valid active and pursuing it. */
	    for( curEdge=rectArrayG[node1].adj;curEdge;curEdge=curEdge->next ){
		node2 = curEdge->node ;
		if( curEdge->edge->active && 
		    HnotV == getDirection(node1,node2) ){
		    /* found a valid edge */
		    /* mark edge */
		    curEdge->edge->active = FALSE ;
		    break ;
		}
	    }
	    if( !(curEdge) ){
		/* search exhausted - get off the merry-go-round */
		lastNodeB = node1 ;/* save last node in backward search */
		break ;
	    }

	    /* update counts for nodes */
	    rectArrayG[node1].active-- ;
	    rectArrayG[node2].active-- ;
	    if( rectArrayG[node1].active == 1 ){
		Yadd2Queue( Queue, (char *)((INT)node1) ) ; 
	    }

	    /* mark return */
	    rectArrayG[node2].from = node1 ;

	    /* update segment span */
	    if( HnotV ){ /* horizontal edge */
		/* now union with node2 */
		bot = MAX( bot, rectArrayG[node2].node_b ) ;
		top = MIN( top, rectArrayG[node2].node_t ) ;
	    } else { /* vertical edge */
		/* now union with node2 */
		lft = MAX( lft, rectArrayG[node2].node_l ) ;
		rgh = MIN( rgh, rectArrayG[node2].node_r ) ;
	    }

	    /* go on to next node */
	    node1 = node2 ;

	} /* end loop on nodes of the backward path from source */
	/* END PATHELOGICAL CASE TRAVERSAL */

	/* determine new position for all nodes in the path */
	if( HnotV ){ /* horizontal edge */
	    ASSERT( bot <= top, "rectilinear", "Bottom > top " ) ;
	    ycenter = (top + bot ) / 2 ; /* make center average */

	} else { /* vertical edge */
	    ASSERT( lft <= rgh, "rectilinear", "Left > right " ) ;
	    xcenter = (lft + rgh ) / 2 ; /* make center average */
	}

	/* modify position of all nodes on the forward path */
	/* we know we must process this case always */
	rectArrayG[sourceNode].from = 0 ;
	node1 = lastNodeF ;

	/* loop until we hit the zero we put at sourceNode */
	D( "rectlin", printf( "path:" ) ) ;
	while( node1 != 0 ){ 

	    D( "rectlin", printf( "%d ", node1 ) ) ;
	    curNode = &(rectArrayG[ node1 ]) ; /* for speed */

	    if( HnotV ){ /* horizontal edge */
		curNode->yc = ycenter ;

	    } else { /* vertical edge */
		curNode->xc = xcenter ;
	    }
	    node1 = curNode->from ;
	} /* end loop of processed forward path */

	/* now look at backward path */
	if( lastNodeB != sourceNode ){
	    fprintf( stderr, "We found a pathelogical case\n" ) ;

	    node1 = lastNodeB ;

	    /* loop until we hit the zero we put at sourceNode */
	    while( node1 != 0 ){ 

		D( "rectlin", printf( "%d ", node1 ) ) ;
		curNode = &(rectArrayG[ node1 ]) ; /* for speed */

		if( HnotV ){ /* horizontal edge */
		    curNode->yc = ycenter ;

		} else { /* vertical edge */
		    curNode->xc = xcenter ;
		}
		node1 = curNode->from ;
	    } /* end loop of processed backward path */
	}
	D( "rectlin", printf( "\n" ) ) ;


    } /* end while loop get ready to pop another node from queue */

    /* now update all lengths */
    for( i = 1 ; i <= eNumG ; i++ ) {
	if( eArrayG[i].active == TRUE ) {
	    index1 = eArrayG[i].index1 ;
	    index2 = eArrayG[i].index2 ;
	    if( i <= edgeTransitionG ) {
		length = rectArrayG[index2].yc - rectArrayG[index1].yc  ;
	    } else {
		length = rectArrayG[index2].xc - rectArrayG[index1].xc  ;
	    }
	    eArrayG[i].length = length ;

	}
    } /* end length update */

} /* end routine rectilinear */


/* determines if two nodes are horizontal in the graph or vertical */
/* use edge transition variable to determine direction.  All edges i */
/* in eArray[i] <= edgeTransition are vertical. Values of i greater */
/* than edgeTransitioin are horizontal. Returns TRUE if horizontal */
BOOL getDirection( node1, node2 ) 
int node1, node2 ;
{
    int i ;

    /* get index into eArray */
    i = eIndexArrayG[node1][node2] ;
    if( i <= edgeTransitionG ) {
	return( FALSE ) ; /* vertical */
    } else {
	return( TRUE ) ; /* horizontal */
    }
} /* end getDirection */

/* ***************************************************************** 
   Build channel adjacency list by forming undirected graph.
   **************************************************************** */
addEdge( node1, node2, edgeIndex ) 
int node1 ;
int node2 ;
int edgeIndex ;
{
    ADJPTR newfE, newbE, temp ;

    /* form an undirected graph */
    /* create forward edge */
    if( temp = rectArrayG[node1].adj ){
	/* insert at beginning of list */
	newfE = rectArrayG[node1].adj = (ADJPTR) 
	    Ysafe_malloc(sizeof(ADJBOX)) ;
	newfE->next = temp ;
	/* fix back link of temp */
	ASSERT( temp->prev == NULL, "addEdge","temp->prev is not NULL" );
	temp->prev = newfE ;
    } else {
	/* start adjacency list */
	newfE = rectArrayG[node1].adj = (ADJPTR) 
	    Ysafe_malloc(sizeof(ADJBOX)) ;
	newfE->next = NULL ;
    }
    newfE->prev = NULL ;
    newfE->edge = &(eArrayG[edgeIndex]) ; /* take address of record for speed */
    newfE->node = node2 ;

    /* create backward edge */
    if( temp = rectArrayG[node2].adj ){
	/* insert at beginning of list */
	newbE = rectArrayG[node2].adj = (ADJPTR) 
	    Ysafe_malloc(sizeof(ADJBOX)) ;
	newbE->next = temp ;
	/* fix back link of temp */
	ASSERT( temp->prev == NULL, "addEdge","temp->prev is not NULL" );
	temp->prev = newbE ;
    } else {
	/* start adjacency list */
	newbE = rectArrayG[node2].adj = (ADJPTR) 
	    Ysafe_malloc(sizeof(ADJBOX)) ;
	newbE->next = NULL ;
    }
    newbE->prev = NULL ;
    newbE->edge = &(eArrayG[edgeIndex]) ; /* take address of record for speed */
    newbE->node = node1 ;

    /* now link partners together */
    newfE->partner = newbE ;
    newbE->partner = newfE ;
} /* end addEdge */


findFourCorners( lowerLeftNode, upperLeftNode, 
		 lowerRiteNode, upperRiteNode )
INT *lowerLeftNode, *upperLeftNode, *lowerRiteNode, *upperRiteNode ;
{
    /* ----------------------------------------------------------------- 
	find four corners of chip by look at all the nodes
    ----------------------------------------------------------------- */
    int i ;
    int oldLeftEnd = INT_MAX ;
    int oldRightEnd = INT_MIN ;
    int oldLeftBot = INT_MAX ;
    int oldLeftTop = INT_MIN ;
    int oldRightBot = INT_MAX ;
    int oldRightTop = INT_MIN ;

    /* initialization */
    *upperLeftNode = 0 ;
    *lowerLeftNode = 0 ;
    *upperRiteNode = 0 ;
    *lowerRiteNode = 0 ;

    for( i=1;i<=numRectsG;i++ ){ /* search thru all the nodes */

	/* look for leftmost nodes */
	if( rectArrayG[i].l <= oldLeftEnd ){
	    oldLeftEnd = rectArrayG[i].l ;
	    if( rectArrayG[i].t >= oldLeftTop ){
		/* means we have a new candidate for upperLeftNode */
		oldLeftTop = rectArrayG[i].t ;
		*upperLeftNode = i ;
	    }  
	    if( rectArrayG[i].b <= oldLeftBot ){
		/* means we have a new candidate for lowerLeftNode */
		oldLeftBot = rectArrayG[i].b ;
		*lowerLeftNode = i ;
	    }  
	}
	/* look for rightmost nodes */
	if( rectArrayG[i].r >= oldRightEnd ){
	    oldRightEnd = rectArrayG[i].r ;
	    if( rectArrayG[i].t >= oldRightTop ){
		/* means we have a new candidate for upperRightNode */
		oldRightTop = rectArrayG[i].t ;
		*upperRiteNode = i ;
	    }  
	    if( rectArrayG[i].b <= oldRightBot ){
		/* means we have a new candidate for lowerRightNode */
		oldRightBot = rectArrayG[i].b ;
		*lowerRiteNode = i ;
	    }  
	}
    }
    ASSERT( *upperLeftNode != 0, "findFourCorners", "upperLeftNode = 0") ;
    ASSERT( *upperRiteNode != 0, "findFourCorners", "upperRiteNode = 0") ;
    ASSERT( *lowerLeftNode != 0, "findFourCorners", "lowerLeftNode = 0") ;
    ASSERT( *lowerRiteNode != 0, "findFourCorners", "lowerRiteNode = 0") ;

} /* end function findFourCorners */
