/* ----------------------------------------------------------------- 
FILE:	    sortpin.c                                       
DESCRIPTION:Sort pins to use with incremental bounding box routines.
CONTENTS:   
DATE:	    Mar 27, 1989 
REVISIONS:  May  2, 1990 - added verify_pad_pins as an error checking
		mechanism to insure that pad pins are inside the 
		channel graph.
	    Sun Jan 20 21:34:36 PST 1991 - ported to AIX.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) sortpin.c version 3.7 10/18/91" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>
#include <yalecad/message.h>



static INT comparePin() ;


sortpins()
{

    INT j , n , cell ;
    CELLBOXPTR ptr ;
    PINBOXPTR pin, *xpptr ;
    /* static INT comparePin() ;*/
    INT maxpins ;


    /* find maximum number of pins on a cell for allocation */
    maxpins = INT_MIN ;
    for( cell = 1 ; cell <= totalcellsG ; cell++ ) {
	ptr = cellarrayG[ cell ] ;
	maxpins = MAX( ptr->numpins, maxpins ) ;
    }

    /* allocate array for sort */
    xpptr = (PINBOXPTR *) Ysafe_malloc((maxpins+1) * sizeof(PINBOXPTR) ) ;

    /* now go thru all cells sorting pins */
    for( cell = 1 ; cell <= totalcellsG ; cell++ ) {
	ptr = cellarrayG[ cell ] ;
	n = 0 ;
	for( pin = ptr->pinptr ; pin ; pin = pin->nextpin ) {
	    xpptr[ n++ ] = pin ;
	}
	/* net number then by pin name */
	Yquicksort( (char *)xpptr, n, sizeof(PINBOXPTR),comparePin ) ;
	/* terminate list */
	xpptr[ n ] = NIL(PINBOXPTR) ;
	ptr->pinptr = xpptr[ 0 ] ;
	for( j = 0 ; j < n ; j++ ) {
	    xpptr[j]->nextpin = xpptr[j+1] ;
	}
    }
    Ysafe_free( xpptr ) ;
} /* end sortpins */


static INT comparePin( pinA , pinB )
PINBOXPTR *pinA , *pinB ;

{
    /* first sort by net number */
    if( (*pinA)->net != (*pinB)->net ){
	return( (*pinA)->net - (*pinB)->net ) ;
    } else {
	/* if nets are equal sort by pinname */
	return( strcmp( (*pinA)->pinname,(*pinB)->pinname ) ) ;
    }
} /* end comparePin */
