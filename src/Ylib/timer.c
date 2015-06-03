/*
 *   Copyright (C) 1990-1991 Yale University
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
FILE:	    timer.c                                       
DESCRIPTION:create a timer for a process.
CONTENTS:   Ytimer_start()
	    Ytimer_elapsed( time_elapsed )
		INT *time_elapsed ;
DATE:	    Oct 04, 1990 
REVISIONS:  
	    Apr 01, 1991 - added SYS5 (A/UX) support  (RAWeier)
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) timer.c version 4.3 10/25/91" ;
#endif

 
#include <yalecad/base.h>
#include <sys/types.h>

#ifdef SYS5
#include <sys/times.h>
#else /* SYS5 */
#include <sys/timeb.h>
#endif /* SYS5 */

static INT base_timeS = 0 ;       /* the time in seconds at the start */
static INT milli_timeS ;         /* the millisecond part of the start */
/* initialize the timer */
Ytimer_start()
{
#ifdef SYS5
    struct tms tp ;
    times(&tp);
    base_timeS = (INT) tp.tms_cstime + (INT) tp.tms_cutime;
#else /* SYS5 */
    struct timeb tp ;
    ftime(&tp) ;
    base_timeS = (INT) tp.time ;
    milli_timeS = (INT) tp.millitm ;
#endif /* SYS5 */
} /* end Ytimer_start */

/* this is the time elapsed since the timer start in milliseconds */
Ytimer_elapsed( time_elapsed )
INT *time_elapsed ;
{
#ifdef SYS5
  struct tms tp;
#else /* SYS5 */
  struct timeb tp;
#endif /* SYS5 */
  
  INT time_offset ;
    if( base_timeS == 0 ){
	/* start was not called */
	*time_elapsed = 0 ;
	return ;
    }

#ifdef SYS5
    times(&tp);
    time_offset = (INT) tp.tms_cutime + (INT) tp.tms_cstime;
    time_offset -= base_timeS;
    *time_elapsed = time_offset * (1000/60); /* 1/60 sec to milisec */
#else /* SYS5 */
    ftime(&tp) ;
    /* now add the current time */
    time_offset = (INT) tp.time ;
    /* now subtract the start time */
    time_offset -= base_timeS ;
    /* now multiply by 1000 to change to milliseconds */
    time_offset *= 1000 ;
    /* now add the current millisecond time */
    time_offset += (INT) tp.millitm ;
    /* now subtract the start millisecond time */
    time_offset -= milli_timeS ;
    /* return the answer */
    *time_elapsed = time_offset ;
#endif /* SYS5 */   
} /* end Ytimer_elapsed */
