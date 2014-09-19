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





