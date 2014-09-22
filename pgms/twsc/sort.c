/* ----------------------------------------------------------------- 
FILE:	    sort.c                                       
DESCRIPTION:various sort functions for quicksort.
CONTENTS:   compare_cost( a , b )
		EDGE_COST *a , *b ;
	    comparegdx( a , b )
		CHANGRDPTR *a , *b ;
	    comparetxpos( a , b )
		IPBOXPTR *a , *b ;
	    comparenptr( a , b )
		FEED_SEG_PTR *a , *b ;
	    comparepinx( a , b )
		PINBOXPTR *a , *b ;
	    comparex( a , b )
		INT *a , *b ;
	    cmpr_sx( aptr, bptr )
		PINBOXPTR *aptr, *bptr ;
	    cmpr_lx( aptr, bptr )
		PINBOXPTR *aptr, *bptr ;
	    cmpr_sy( aptr, bptr )
		PINBOXPTR *aptr, *bptr ;
	    cmpr_ly( aptr, bptr )
		PINBOXPTR *aptr, *bptr ;
DATE:	    Mar 27, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) sort.c (Yale) version 4.3 9/7/90" ;
#endif
#endif

#include "standard.h"
#include "groute.h"
#include "feeds.h"

typedef struct graph_edge_cost {
    SHORT node1 ;
    SHORT node2 ;
    INT cost ;
    INT channel ;
}
*EDGE_COST ,
EDGE_COST_BOX ;

compare_cost( a , b )
EDGE_COST *a , *b ;
{
return( (*a)->cost - (*b)->cost ) ;
}


INT comparegdx( a , b )
CHANGRDPTR *a , *b ;
{
return( (*a)->netptr->xpos - (*b)->netptr->xpos ) ;
}

INT comparetxpos( a , b )
IPBOXPTR *a , *b ;
{
return( (*a)->txpos - (*b)->txpos ) ;
}

INT comparenptr( a , b )
FEED_SEG_PTR *a , *b ;
{
return( (*a)->netptr->xpos - (*b)->netptr->xpos ) ;
}

comparepinx( a , b )
PINBOXPTR *a , *b ;
{
return( (*a)->xpos - (*b)->xpos ) ;
}

INT comparex( a , b )
INT *a , *b ;

{
    return( carrayG[ *a ]->cxcenter - carrayG[ *b ]->cxcenter ) ;
}


INT cmpr_sx( aptr, bptr )
PINBOXPTR *aptr, *bptr ;
{

if( (*aptr)->xpos > (*bptr)->xpos ) {
    return(1) ;
} else if( (*aptr)->xpos < (*bptr)->xpos ) {
    return(-1) ;
} else {
    return(0) ;
}
}


INT cmpr_lx( aptr, bptr )
PINBOXPTR *aptr, *bptr ;
{

if( (*aptr)->xpos > (*bptr)->xpos ) {
    return(-1) ;
} else if( (*aptr)->xpos < (*bptr)->xpos ) {
    return(1) ;
} else {
    return(0) ;
}
}


INT cmpr_sy( aptr, bptr )
PINBOXPTR *aptr, *bptr ;
{

if( (*aptr)->newy > (*bptr)->newy ) {
    return(1) ;
} else if( (*aptr)->newy < (*bptr)->newy ) {
    return(-1) ;
} else {
    return(0) ;
}
}


INT cmpr_ly( aptr, bptr )
PINBOXPTR *aptr, *bptr ;
{

if( (*aptr)->newy > (*bptr)->newy ) {
    return(-1) ;
} else if( (*aptr)->newy < (*bptr)->newy ) {
    return(1) ;
} else {
    return(0) ;
}
}
