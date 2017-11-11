/*
 *   Copyright (C) 1989-1990 Yale University
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
		int *a , *b ;
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
#include <globals.h>
#include "allheaders.h"

int compare_cost( EDGE_COST *a , EDGE_COST *b )
{
	return( (*a)->cost - (*b)->cost ) ;
}


int comparegdx( CHANGRDPTR *a , CHANGRDPTR *b )
{
	return( (*a)->netptr->xpos - (*b)->netptr->xpos ) ;
}

int comparetxpos( IPBOXPTR *a , IPBOXPTR *b )
{
	return( (*a)->txpos - (*b)->txpos ) ;
}

int comparenptr( FEED_SEG_PTR *a , FEED_SEG_PTR *b )
{
	return( (*a)->netptr->xpos - (*b)->netptr->xpos ) ;
}

int comparepinx( PINBOXPTR *a , PINBOXPTR *b )
{
	return( (*a)->xpos - (*b)->xpos ) ;
}

int comparex( int *a , int *b )
{
    return( carrayG[ *a ]->cxcenter - carrayG[ *b ]->cxcenter ) ;
}

int cmpr_sx( PINBOXPTR *aptr, PINBOXPTR *bptr )
{
	if( (*aptr)->xpos > (*bptr)->xpos ) {
		return(1) ;
	} else if( (*aptr)->xpos < (*bptr)->xpos ) {
		return(-1) ;
	} else {
		return(0) ;
	}
}

int cmpr_lx( PINBOXPTR *aptr, PINBOXPTR *bptr )
{
	if( (*aptr)->xpos > (*bptr)->xpos ) {
		return(-1) ;
	} else if( (*aptr)->xpos < (*bptr)->xpos ) {
		return(1) ;
	} else {
		return(0) ;
	}
}

int cmpr_sy( PINBOXPTR *aptr, PINBOXPTR *bptr )
{
	if( (*aptr)->newy > (*bptr)->newy ) {
		return(1) ;
	} else if( (*aptr)->newy < (*bptr)->newy ) {
		return(-1) ;
	} else {
		return(0) ;
	}
}

int cmpr_ly( PINBOXPTR *aptr, PINBOXPTR *bptr )
{
	if( (*aptr)->newy > (*bptr)->newy ) {
		return(-1) ;
	} else if( (*aptr)->newy < (*bptr)->newy ) {
		return(1) ;
	} else {
		return(0) ;
	}
}
