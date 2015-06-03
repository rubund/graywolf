/*
 *   Copyright (C) 1989-1991 Yale University
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

/* --------------------------------------------------------------------------
FILE:	    mytime.c                                       
DESCRIPTION:better random seed from system time.
CONTENTS:   unsigned Yrandom_seed()
DATE:	    Feb 07, 1989 
REVISIONS:  Apr 27, 1989 - changed to Y prefix.
	    Aug 25, 1989 - changed algorithm to use less system
		calls.
	    Dec 17, 1989 - use the version in SC as default.
	    Apr 01, 1991 - added SYS5 (A/UX) support  (RAWeier)
            Oct 07, 1991 - fix warning in Gnu gcc (RAWeier)
            Dec 09, 1991 - industrial users say gettimeofday is more
                           "universal" and improves portability (RAWeier)
--------------------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) mytime.c version 3.8 12/15/91" ;
#endif

 
#include <yalecad/base.h>

/* NOTE to user on foreign machines: if below does not compile, you */
/* may use the alternate random seed generator at some penalty for */
/* randomness but it should not matter due to the robustness of the */
/* random number generator itself.  Just use #define ALTERNATE */

/* #define ALTERNATE */

#ifndef ALTERNATE

#ifdef VMS
#include <types.h>
#include <timeb.h>

#else 
#include <sys/types.h>
#include <sys/time.h>
#endif

unsigned Yrandom_seed()
{
    UNSIGNED_INT m , time1 , time2 ;
    struct timeval tp ;
    gettimeofday(&tp,0) ;
    time1 = (UNSIGNED_INT) tp.tv_usec ;
    time1 <<= 22 ;
    time2 = (tp.tv_usec & 0x003FFFFF) ;
    m = time1 | time2 ;
    return(m>>1);
}

#else  /* ALTERNATE */

/* returns a random seed from system clock */
unsigned Yrandom_seed()
{
    UNSIGNED_INT seed ;
    long t ;
    INT  hsum ;
    INT  shift ;
    char buffer[LRECL],
	 *name = buffer ;;

    /* get time since Jan 1, 1970 in seconds */
    t = time((long *) 0) ;
    /* convert to a string */
    sprintf( buffer, "%d", t ) ;

    /* now use hash function to get a number 0..INT_MAX */
    /* don't initialize hsum use garbage if possible */
    for (shift=1 ;*name; name++){
	hsum = hsum + *name<<shift;
	shift = (shift + 1) & 0x000F;
    }
    return((unsigned) (hsum % 0x003FFFFF) ) ;

} /* end my_time */

#endif /* ALTERNATE */





