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
