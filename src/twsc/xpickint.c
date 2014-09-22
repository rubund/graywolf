/* ----------------------------------------------------------------- 
FILE:	    xpickint.c                                       
DESCRIPTION:pick a random number.
CONTENTS:   XPICK_INT( l , u , c )
		INT l , u , c ;
DATE:	    Mar 27, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) xpickint.c (Yale) version 4.4 9/7/90" ;
#endif
#endif

#include <yalecad/base.h>

#define PICK_INT(l,u) (((l)<(u)) ? ((RAND % ((u)-(l)+1))+(l)) : (l))

XPICK_INT( l , u , c )
INT l , u , c ;
{

    INT d ;

    if ( c < 0 ) {
	return(-c) ;
    } else {
        do {
	    d = PICK_INT(l,u) ;
	} while ( d == c ) ;
	return(d) ;
    }
}
