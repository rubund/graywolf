/*
 *   Copyright (C) 1988-1991 Yale University
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
FILE:	    neworient.c                                       
DESCRIPTION:pick a new orientation.
DATE:	    Jan 29, 1988 
REVISIONS:  Thu Apr 18 01:37:39 EDT 1991 - added check_valid_orientation.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) neworient.c version 3.4 4/18/91" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

newOrient( cellptr , range )
CELLBOXPTR cellptr ;
INT range ;
{

    INT incidence , count , i , orient ;

    orient = cellptr->orient ;
    if( range == 4 ) {
	if( orient >= 4 ) {
	    count = 0 ;
	    for( i = 0 ; i <= 3 ; i++ ) {
		if( cellptr->orientList[i] == 1 ) {
		    count++ ;
		}
	    }
	    if( count == 0 ) {
		return( -1 ) ;
	    }
	    do {
		incidence = (INT) ( (DOUBLE) count * 
				  ( (DOUBLE) RAND / 
				  (DOUBLE) 0x7fffffff ) ) + 1 ;
	    } while( incidence == count + 1 ) ;

	    count = 0 ;
	    for( i = 0 ; i <= 3 ; i++ ) {
		if( cellptr->orientList[i] == 1 ) {
		    if( ++count == incidence ) {
			return( i ) ;
		    }
		}
	    }
	    
	} else {
	    count = 0 ;
	    for( i = 4 ; i <= 7 ; i++ ) {
		if( cellptr->orientList[i] == 1 ) {
		    count++ ;
		}
	    }
	    if( count == 0 ) {
		return( -1 ) ;
	    }
	    do {
		incidence = (INT) ( (DOUBLE) count * 
				 ( (DOUBLE) RAND / 
				 (DOUBLE) 0x7fffffff ) ) + 1 ;
	    } while( incidence == count + 1 ) ;

	    count = 0 ;
	    for( i = 4 ; i <= 7 ; i++ ) {
		if( cellptr->orientList[i] == 1 ) {
		    if( ++count == incidence ) {
			return( i ) ;
		    }
		}
	    }
	}
    } else {
	count = 0 ;
	for( i = 0 ; i <= 7 ; i++ ) {
	    if( i == orient ) {
		continue ;
	    }
	    if( cellptr->orientList[i] == 1 ) {
		count++ ;
	    }
	}
	if( count == 0 ) {
	    return( -1 ) ;
	}
	do {
	    incidence = (INT) ( (DOUBLE) count * 
			      ( (DOUBLE) RAND / 
			      (DOUBLE) 0x7fffffff ) ) + 1 ;
	} while( incidence == count + 1 ) ;

	count = 0 ;
	for( i = 0 ; i <= 7 ; i++ ) {
	    if( i == orient ) {
		continue ;
	    }
	    if( cellptr->orientList[i] == 1 ) {
		if( ++count == incidence ) {
		    return( i ) ;
		}
	    }
	}
    }
    return( -1 ) ;
}

/*
   returns  0 if orientation is valid 
   returns -1 if no orientation is valid
   returns +1 if it could find a valid orientation.
*/

INT check_valid_orient( cptr )
CELLBOXPTR cptr ;
{
    INT i ; /* view counter */
   
    if( cptr->orientList[cptr->orient] ){
	/* this is valid no problem */
	return( 0 ) ;
    }
    /* else move to the first valid one */
    if( cptr->orient <= 3 ){
	for( i = 0; i <= 7 ; i++ ){
	    if( cptr->orientList[i] ){
		cptr->orient = i ;
		return( 1 ) ;
	    }
	}
	
    } else {
	for( i = 7; i >= 0 ; i-- ){
	    if( cptr->orientList[i] ){
		cptr->orient = i ;
		return( 1 ) ;
	    }
	}
    }
    return( -1 ) ;

} /* end check_valid_orient */
