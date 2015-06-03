/*
 *   Copyright (C) 1988-1990 Yale University
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
