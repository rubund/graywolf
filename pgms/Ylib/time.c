/* ----------------------------------------------------------------- 
FILE:	    time.c                                       
DESCRIPTION:This file contains a time utility routine which returns
	    an ASCII string with time and date.
CONTENTS:   
	    char *YcurTime( time_in_sec ) 
		INT *time_in_sec ;

DATE:	    Oct 23, 1988
REVISIONS:  Apr 27, 1989 - changed to Y prefix and added time in seconds.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) time.c version 3.3 8/28/90" ;
#endif

#include <time.h>
#include <yalecad/base.h>

#ifdef THINK_C
#define TIME_T time_t
#else
#define TIME_T long
#endif

char *YcurTime( time_in_sec )
INT *time_in_sec ;
{
    /* get current time - return ASCII string with time and date */
    TIME_T t ;
    char *ptr ;
    char *nptr ;
    t = time((TIME_T *) 0) ;
    /* return time in seconds since Jan 1, 1970 if requested */
    if( time_in_sec ){
	*time_in_sec = t ;
    }
    /* get time and remove newline char */
    for( nptr = ptr = ctime(&t) ;*nptr;nptr++ ){
	if( *nptr == '\n' ){
	    *nptr = EOS ;
	}
    }
    return( ptr ) ;
} /* end curTime */
