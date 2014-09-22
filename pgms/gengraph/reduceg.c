/* ----------------------------------------------------------------- 
FILE:	    reduceg.c                                       
DESCRIPTION:This file reduces the channel graph by eliminating the
	    outside pad channels.
CONTENTS:   remove_outside_channels() ;
	    int *get_reverse_ref()
	    int *get_cross_ref()
	    static int next_node( node, direction )
		int node, direction ;
DATE:	    Sep 25, 1989 
REVISIONS:  Sun Jan 20 21:20:45 PST 1991 - ported to AIX.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) reduceg.c version 1.3 1/20/91" ;
#endif

#include <geo.h>
#include <yalecad/debug.h>

#define HORZ  TRUE
#define VERT  FALSE

/* ***************************************************************** 
    Reduce the channel graph by removing the channels outside the pads
    These channels are invalid for routing so remove them before passing
    channel graph to global router.
    ALGORITHM:
	Initialize all nodes to active.
	Find four corners of the channel graph - guaranteed because
	    pads are always added to design even if they don't exist.
	Start at lowerLeft node and traverse vertically to upperLeft
	    node marking all nodes invalid.  Continue traversing 
	    clockwise around the chip until returning to starting point.
	Now rebuild graph avoiding marked nodes.  When done free old
	    graph.
   ***************************************************************** */

static INT *reverserefS ;
static INT *crossrefS ;    /* used to link old and new */


static INT next_node();



INT *remove_outside_channels()
{

    INT i ;                           /* counter */
    INT node ;                        /* current node */
    INT count ;                       /* used in crossreference */
    /* the four corner nodes of the chip */
    INT  lowerLeftNode, upperLeftNode, lowerRiteNode, upperRiteNode ;
    INT next_node() ;


    /* initialize all nodes to active */
    for( i = 1; i <= numRectsG; i++ ){
	rectArrayG[i].active = TRUE ;
    }

    /* ----------------------------------------------------------------- 
	find four corners of chip.
    ----------------------------------------------------------------- */
    findFourCorners( &lowerLeftNode, &upperLeftNode, 
	 &lowerRiteNode, &upperRiteNode ) ;

    /* go clockwise around chip */
    /* first up left side */
    for( node = lowerLeftNode;node; node = next_node( node, VERT ) ){
	rectArrayG[node].active = FALSE ;
    }
    ASSERT( rectArrayG[upperLeftNode].active == FALSE,NULL,"problem\n") ;

    /* go across the top */
    for( node = upperLeftNode;node; node = next_node( node, HORZ ) ){
	rectArrayG[node].active = FALSE ;
    }
    ASSERT( rectArrayG[upperRiteNode].active == FALSE,NULL,"problem\n") ;

    /* go down the right side */
    for( node = upperRiteNode;node; node = next_node( node, VERT ) ){
	rectArrayG[node].active = FALSE ;
    }
    ASSERT( rectArrayG[lowerRiteNode].active == FALSE,NULL,"problem\n") ;

    /* go across the bottom */
    for( node = lowerRiteNode;node; node = next_node( node, HORZ ) ){
	rectArrayG[node].active = FALSE ;
    }


    /* now build crossreference */
    crossrefS   = (INT *) Ysafe_calloc( numRectsG+1, sizeof(INT) ) ;
    reverserefS = (INT *) Ysafe_calloc( numRectsG+1, sizeof(INT) ) ;
    count = 0 ;
    for( i=1; i <= numRectsG; i++ ){
	if( rectArrayG[i].active ){
	    /* build cross reference array between old and new */
	    crossrefS[i] = ++count ;
	    reverserefS[count] = i ;
	} else {
	    /* create an error */
	    crossrefS[i] = INT_MIN ;
	}
    }
    return( crossrefS ) ;


} /* end remove_outside_channels() */

/* avoid globals across programs */
INT *get_reverse_ref()
{
    return( reverserefS ) ;
}

/* avoid globals across programs */
INT *get_cross_ref()
{
    return( crossrefS ) ;
}

/* look for an active node adjacent to given node */
static INT next_node( node, direction )
INT node, direction ;
{
    ADJPTR adjptr ;                   /* traverse the edges */
    INT nextnode ;                    /* candidate node */

    for( adjptr=rectArrayG[node].adj;adjptr;adjptr = adjptr->next ){
	nextnode = adjptr->node ;
	if( rectArrayG[nextnode].active ){
	    if( getDirection( node, nextnode ) == direction ){
		return( nextnode ) ;
	    }
	}
    }
    return( 0 ) ; /* cause an error */

} /* end next_node() */

INT *keep_pad_nodes()
{
    INT i ;

    /* all nodes are in the graph */

    /* initialize all nodes to active */
    for( i = 1; i <= numRectsG; i++ ){
	rectArrayG[i].active = TRUE ;
    }
    crossrefS   = (INT *) Ysafe_calloc( numRectsG+1, sizeof(INT) ) ;
    reverserefS = (INT *) Ysafe_calloc( numRectsG+1, sizeof(INT) ) ;

    for( i=0; i <= numRectsG; i++ ){
	/* build cross reference array between old and new */
	crossrefS[i] = i ;
	reverserefS[i] = i ;
    }
    return( crossrefS ) ;
} /* end keep_pad_nodes */
