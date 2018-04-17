/*
 *   Copyright (C) 1990 Yale University
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

#ifndef lint
static char SccsId[] = "@(#) makesite.c version 3.3 9/5/90" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

DOUBLE val1 , val2 ;

Vside( kArray, cellptr , x , yy1 , yy2 , flag )
KBOXPTR kArray ;       
CELLBOXPTR cellptr ;
INT x , yy1 , yy2 , flag ;
{

DOUBLE aspFactor , aspect , aspLB ;
INT worstLen , pPinLocs , k , length , base ;
INT TotSites ;
DOUBLE siteSpace ;

length = ABS(yy2 - yy1) ;
aspect = cellptr->aspect ;
aspLB  = cellptr->aspLB ;
if( aspLB + 0.01 > cellptr->aspUB ) {
    aspFactor = 1.0 ;
} else {
    aspFactor = sqrt( aspLB / aspect ) ;
}

worstLen = (INT) (aspFactor * (DOUBLE) length) ;
if( (DOUBLE)(worstLen + 1) / aspFactor - (DOUBLE) length <
		(DOUBLE) length - (DOUBLE) worstLen / aspFactor ) {
    worstLen++ ;
}

pPinLocs = worstLen / track_spacingYG - 1 ;
if( flag ) {
    if( pPinLocs > MAXSITES ) {
	for( k = 1 ; k <= MAXSITES ; k++ ) {
	    kArray[ k ].cap = 0 ;
	    kArray[ k ].HV = 0 ;
	    kArray[ k ].sp = 0 ;
	    kArray[ k ].x = 0 ;
	    kArray[ k ].y = 0 ;
	}
	base = pPinLocs / MAXSITES ;
	for( k = 1 ; k <= MAXSITES ; k++ ) {
	    kArray[ k ].cap += base ;
	}
	for( k = 1 ; k <= pPinLocs % MAXSITES ; k++ ) {
	    kArray[ k ].cap++ ;
	}
    } else {  /* prepare pPinLocs sites */
	for( k = 1 ; k <= pPinLocs ; k++ ) {
	    kArray[ k ].cap = 0 ;
	    kArray[ k ].HV = 0 ;
	    kArray[ k ].sp = 0 ;
	    kArray[ k ].x = 0 ;
	    kArray[ k ].y = 0 ;
	}
	for( k = 1 ; k <= pPinLocs ; k++ ) {
	    kArray[ k ].cap = 1 ;
	}
    }
}

/*
    TotSites = minimum of ( MAXSITES and pPinLocs ) 
*/
if( pPinLocs <= MAXSITES ) {
    TotSites = pPinLocs ;
} else {
    TotSites = MAXSITES ;
}
siteSpace = (DOUBLE) length / (DOUBLE) (TotSites + 1) ;

/* 
    Suppose we encountered coordinates yy1 and yy2 for a given
    vertical side, and of course that the x-coordinate is simply x.
    ( yy1 < yy2 is required )
*/

if( flag ) {
    if( yy2 > yy1 ) {
	for( k = 1 ; k <= TotSites ; k++ ) {
	    kArray[ k ].x = x ;
	    val1 = (k + 1) * siteSpace ;
	    val2 = k * siteSpace ;
	    kArray[ k ].y = ROUND( val2 ) + yy1 ;
	    kArray[ k ].sp = ROUND( val1 ) - ROUND( val2 ) ;
	    kArray[ k ].HV = 1 ;
	}
    } else {
	for( k = 1 ; k <= TotSites ; k++ ) {
	    kArray[ k ].x = x ;
	    val1 = (k + 1) * siteSpace ;
	    val2 = k * siteSpace ;
	    kArray[ k ].y = yy1 - ROUND( val2 ) ;
	    kArray[ k ].sp = ROUND( val1 ) - ROUND( val2 ) ;
	    kArray[ k ].HV = 1 ;
	}
    }
}

/*
 *
 *   --- BIG NOTE ---
 *
 *   Any fixed terminals intersecting a site cause the contents
 *   of that site to be incremented by one. This will inhibit
 *   sequences from passing on thru. ( try to remember to
 *   get the check pointer to reflect this fact )
 */

return( TotSites ) ;
}




Hside( kArray, cellptr , xx1 , xx2 , y , flag )
KBOXPTR kArray ;       
CELLBOXPTR cellptr ;
INT xx1 , xx2 , y , flag ;
{

DOUBLE aspFactor , aspect , aspUB ;
INT worstLen , pPinLocs , k , length , base ;
INT TotSites ;
DOUBLE siteSpace ;

length = ABS(xx2 - xx1) ;
aspect = cellptr->aspect ;
aspUB  = cellptr->aspUB ;
if( cellptr->aspLB + 0.01 > aspUB ) {
    aspFactor = 1.0 ;
} else {
    aspFactor = sqrt( aspect / aspUB ) ;
}
worstLen = (INT) (aspFactor * (DOUBLE) length) ;
if( (DOUBLE)(worstLen + 1) / aspFactor - (DOUBLE) length <
		(DOUBLE) length - (DOUBLE) worstLen / aspFactor ) {
    worstLen++ ;
}

pPinLocs = worstLen / track_spacingXG - 1 ;
if( flag ) {
    if( pPinLocs > MAXSITES ) {
	for( k = 1 ; k <= MAXSITES ; k++ ) {
	    kArray[ k ].cap = 0 ;
	    kArray[ k ].HV = 0 ;
	    kArray[ k ].sp = 0 ;
	    kArray[ k ].x = 0 ;
	    kArray[ k ].y = 0 ;
	}
	base = pPinLocs / MAXSITES ;
	for( k = 1 ; k <= MAXSITES ; k++ ) {
	    kArray[ k ].cap += base ;
	}
	for( k = 1 ; k <= pPinLocs % MAXSITES ; k++ ) {
	    kArray[ k ].cap++ ;
	}
    } else {  /* prepare pPinLocs sites */
	for( k = 1 ; k <= pPinLocs ; k++ ) {
	    kArray[ k ].cap = 0 ;
	    kArray[ k ].HV = 0 ;
	    kArray[ k ].sp = 0 ;
	    kArray[ k ].x = 0 ;
	    kArray[ k ].y = 0 ;
	}
	for( k = 1 ; k <= pPinLocs ; k++ ) {
	    kArray[ k ].cap = 1 ;
	}
    }
}

/*
    TotSites = minimum of ( MAXSITES and pPinLocs ) 
*/
if( pPinLocs <= MAXSITES ) {
    TotSites = pPinLocs ;
} else {
    TotSites = MAXSITES ;
}
siteSpace = (DOUBLE) length / (DOUBLE) (TotSites + 1) ;

/* 
   Suppose we encountered coordinates xx1 and xx2 for a given
   horizontal side, and of course that the y-coordinate is simply y.
*/

if( flag ) {
    if( xx2 > xx1 ) {
	for( k = 1 ; k <= TotSites ; k++ ) {
	    kArray[ k ].y = y ;
	    val1 = (k + 1) * siteSpace ;
	    val2 = k * siteSpace ;
	    kArray[ k ].x = ROUND( val2 ) + xx1 ;
	    kArray[ k ].sp = ROUND( val1 ) - ROUND( val2 ) ;
	    kArray[ k ].HV = 0 ;
	}
    } else {
	for( k = 1 ; k <= TotSites ; k++ ) {
	    kArray[ k ].y = y ;
	    val1 = (k + 1) * siteSpace ;
	    val2 = k * siteSpace ;
	    kArray[ k ].x = xx1 - ROUND( val2 ) ;
	    kArray[ k ].sp = ROUND( val1 ) - ROUND( val2 ) ;
	    kArray[ k ].HV = 0 ;
	}
    }
}

/*
 *
 *   --- BIG NOTE ---
 *
 *   Any fixed terminals intersecting a site cause the contents
 *   of that site to be incremented by one. This will inhibit
 *   sequences from passing on thru. ( try to remember to
 *   get the check pointer to reflect this fact )
 */

return( TotSites ) ;
}
