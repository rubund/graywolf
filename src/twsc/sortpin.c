/*
 *   Copyright (C) 1989-1990 Yale University
 *   Copyright (C) 2015 Tim Edwards <tim@opencircuitdesign.com>
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
FILE:	    sortpin.c                                       
DESCRIPTION:pin sorting functions.
CONTENTS:  sortpin()
	    sortpin1( cell )
		INT cell ;
	    sortpin2( cella, cellb )
		INT cella, cellb ;
	    shellsort( term , n )
		PINBOXPTR term[] ;
		INT n ;
	    shellsortx( term , n )
		PINBOXPTR term[] ;
		INT n ;
	    shellsorty( term , n )
		PINBOXPTR term[] ;
		INT n ;
	    shellsort_referx( worker , head , n )
		FEED_SEG_PTR worker[] ;
		INT n ;
DATE:	    Mar 27, 1989 
REVISIONS:  Apr  1, 1990 - rewrote the structure of sortpin to 
		call sortpin1 which is used during a gateswap.  Also
		added new sortpin2 for gateswap between two cells.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) sortpin.c (Yale) version 4.3 9/7/90" ;
#endif
#endif

#include "standard.h"
#include "groute.h"
#include "feeds.h"

static PINBOXPTR *sortArrayS ;          /* the normal array for sorting */
static PINBOXPTR *sortArraySwapS ;

sortpin()
{

    INT cell ;                      /* current cell */
    INT maxpins ;                   /* maximum numpins over all pins */
    CBOXPTR ptr ;                   /* current cell */
    BOOL pin_groups ;               /* true if swappable gates occur */
    INT i;

    /* find the maximum number of pins on a cell for allocation */
    /* also see if pin groups exist on any of the cells */
    maxpins = INT_MIN ;
    pin_groups = FALSE ;
    for( cell = 1; cell <= numcellsG; cell++ ){
	ptr = carrayG[cell] ;
	maxpins = MAX( ptr->numterms, maxpins ) ;
	if( ptr->num_swap_group > 0 ) {
	    for (i = 0; i < ptr->num_swap_group; i++) {
	        SGLIST *sglistptr;
		sglistptr = ptr->swapgroups + i;
		if( sglistptr->num_pin_group > 0 ){
		    pin_groups = TRUE ;
		    break;
		}
	    }
	}
    }

    sortArrayS = (PINBOXPTR *) 
	Ysafe_malloc( (maxpins+2)*sizeof(PINBOXPTR) );

    for( cell = 1 ; cell <= numcellsG ; cell++ ) {
	sortpin1( cell ) ;
    }
    if( pin_groups ){
	/* we we have swappable gates we need to create second array */
	/* and leave first one allocated */
	sortArraySwapS = (PINBOXPTR *) 
	    Ysafe_malloc( (maxpins+2)*sizeof(PINBOXPTR) );
    } else {
	/* in the normal case we are done with sorting free array */
	Ysafe_free( sortArrayS ) ;
    }
} /* end sortpin */


/* sort the pins of a single cell by net */
sortpin1( cell )
INT cell ;
{

    INT j , n ;
    CBOXPTR ptr ;
    PINBOXPTR pinptr ;

    ptr = carrayG[ cell ] ;
    if( ptr->numterms == 0 ) {
	return ;
    }
    n = 0 ;
    for( pinptr = ptr->pins ; pinptr; pinptr = pinptr->nextpin ) {
	sortArrayS[ ++n ] = pinptr ;
    }
    shellsort( sortArrayS , n ) ;
    sortArrayS[ n + 1 ] = PINNULL ;
    ptr->pins = sortArrayS[ 1 ] ;
    for( j = 1 ; j <= n ; j++ ) {
	sortArrayS[j]->nextpin = sortArrayS[j+1] ;
    }

    return ;
} /* end sortpin1 */

sortpin2( cella, cellb )
INT cella, cellb ;
{

    INT j ;                                /* counter */
    INT numpins_a ;                        /* number pins of cell a */
    INT numpins_b ;                        /* number pins of cell b */
    CBOXPTR aptr ;                         /* pointer to cell a info */
    CBOXPTR bptr ;                         /* pointer to cell b info */
    PINBOXPTR pin ;                        /* the current pin */

    aptr = carrayG[ cella ] ;
    bptr = carrayG[ cellb ] ;
    numpins_a = 0 ;
    numpins_b = 0 ;
    /* first set pins currently in cell to correct sort arrays */
    /* sortArrayS holds all the pins of cell a */
    /* sortArraySwap holds all the pins of cell b */
    for( pin = aptr->pins ; pin; pin = pin->nextpin ) {
	if( pin->cell == cella ){
	    sortArrayS[ ++numpins_a ] = pin ;
	} else if( pin->cell == cellb ){
	    sortArraySwapS[ ++numpins_b ] = pin ;
	} else {
	    fprintf( stderr, "Trouble with sortpin2\n" ) ;
	}
    }
    for( pin = bptr->pins ; pin; pin = pin->nextpin ) {
	if( pin->cell == cella ){
	    sortArrayS[ ++numpins_a ] = pin ;
	} else if( pin->cell == cellb ){
	    sortArraySwapS[ ++numpins_b ] = pin ;
	} else {
	    fprintf( stderr, "Trouble with sortpin2\n" ) ;
	}
    }
    /* now sort cell a's pins */
    shellsort( sortArrayS , numpins_a ) ;
    sortArrayS[ numpins_a + 1 ] = PINNULL ;
    aptr->pins = sortArrayS[ 1 ] ;
    for( j = 1 ; j <= numpins_a ; j++ ) {
	sortArrayS[j]->nextpin = sortArrayS[j+1] ;
    }

    /* now sort cell b's pins */
    shellsort( sortArraySwapS , numpins_b ) ;
    sortArraySwapS[ numpins_b + 1 ] = PINNULL ;
    bptr->pins = sortArraySwapS[ 1 ] ;
    for( j = 1 ; j <= numpins_b ; j++ ) {
	sortArraySwapS[j]->nextpin = sortArraySwapS[j+1] ;
    }

    return ;
} /* end sortpin2 */

shellsort( term , n )
PINBOXPTR term[] ;
INT n ;
{

PINBOXPTR ptr ;
INT incr , i , j ;

for( incr = n / 2 ; incr > 0 ; incr /= 2 ) {
    for( i = incr + 1 ; i <= n ; i++ ) {
	for( j = i - incr ; j > 0 && 
	(term[j]->net >
	 term[j+incr]->net) ; j -= incr ) {
	    ptr = term[j] ;
	    term[j] = term[j+incr] ;
	    term[j+incr] = ptr ;
	}
    }
}
}


shellsortx( term , n )
PINBOXPTR term[] ;
INT n ;
{

PINBOXPTR pin ;
INT incr , i , j ;

for( incr = (n+1)/ 2 ; incr > 0 ; incr /= 2 ) {
    for( i = incr ; i <= n ; i++ ) {
	for( j = i - incr ; j >= 0 && 
	(term[j]->xpos > term[j+incr]->xpos) ; j -= incr ) {
	    pin = term[j] ;
	    term[j] = term[j+incr] ;
	    term[j+incr] = pin ;
	}
    }
}
}


shellsorty( term , n )
PINBOXPTR term[] ;
INT n ;
{

PINBOXPTR pin ;
INT incr , i , j ;

for( incr = (n+1)/ 2 ; incr > 0 ; incr /= 2 ) {
    for( i = incr ; i <= n ; i++ ) {
	for( j = i - incr ; j >= 0
	    && term[j]->newy > term[j+incr]->newy ; j -= incr ) {
	    pin = term[j] ;
	    term[j] = term[j+incr] ;
	    term[j+incr] = pin ;
	}
    }
}
}


shellsort_referx( worker , head , n )
FEED_SEG_PTR worker[] ;
INT n ;
{

FEED_SEG_PTR ptr ;
INT incr , i , j , mhead , endi , x1 , x2 ;

mhead = head - 1 ;
for( incr = n / 2 ; incr > 0 ; incr /= 2 ) {
    endi = n + mhead ;
    for( i = mhead + incr + 1 ; i <= endi ; i++ ) {
	for( j = i - incr ; j > mhead ; j -= incr ) {
	    if( worker[j]->refer ) { /* not a steiner poINT */
		x1 = worker[j]->refer->xpos ;
	    } else {
		x1 = worker[j]->netptr->xpos ;
	    }
	    if( worker[j+incr]->refer ) { /* not a steiner poINT */
		x2 = worker[j+incr]->refer->xpos ;
	    } else {
		x2 = worker[j+incr]->netptr->xpos ;
	    }
	    if( x1 > x2 ) {
		ptr = worker[j] ;
		worker[j] = worker[j+incr] ;
		worker[j+incr] = ptr ;
	    }
	}
    }
}
}
