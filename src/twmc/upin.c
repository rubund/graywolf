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
FILE:	    upin.c                                       
DESCRIPTION:update pins on a cell
CONTENTS:   upin_test( termsptr, pos )
		PINBOXPTR termptr ;
		MOVEBOXPTR pos ;
	    upin_accept( termptr )
		PINBOXPTR termptr ;
DATE:	    Jan 30, 1988 
REVISIONS:  Jun 30, 1988 - added rel_pos for test.
	    Oct 19, 1988 - added newhalfP update.
	    Mar 16, 1989 - removed usoftpin.c and ufixpin.c and replaced
		with upin.c.  Now one set of routines handle both
		softpins and hardpins.
	    Aug 13, 1990 - moved relpos to yalecad/relpos.h
	    Wed Jan 30 14:15:02 EST 1991 - removed relpos.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) upin.c version 3.4 1/30/91" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>
#include <yalecad/relpos.h>

#define BREAK_PT2   6

upin_test( termptr, pos )
PINBOXPTR termptr ;
MOVEBOXPTR pos ;
{

    NETBOXPTR dimptr ;
    INT targetx, targety, orient ;

    /* get target position from Rotational record */
    targetx = pos->xcenter ;
    targety = pos->ycenter ;
    orient  = pos->orient ;
    for( ; termptr ; termptr = termptr->nextpin ) {
	netarrayG[ termptr->net ]->nflag = TRUE ;
	termptr->flag = TRUE ;

	/* rel position is a macro which calculates absolute pin */
	/* location defined in relpos.h */
	REL_POS( orient, 
	    termptr->newx, termptr->newy,     /* result */ 
	    termptr->txpos, termptr->typos,   /* cell relative */
	    targetx, targety ) ;              /*  cell center  */
    }
} /* end upin_test */

upin_accept( termptr )
PINBOXPTR termptr ;
{

    NETBOXPTR dimptr ;

    for( ; termptr ; termptr = termptr->nextpin ) {
	dimptr = netarrayG[ termptr->net ] ;
	dimptr->xmin = dimptr->newxmin ;
	dimptr->xmax = dimptr->newxmax ;
	dimptr->ymin = dimptr->newymin ;
	dimptr->ymax = dimptr->newymax ;
	dimptr->halfPx = dimptr->newhalfPx ;
	dimptr->halfPy = dimptr->newhalfPy ;
	termptr->xpos = termptr->newx ;
	termptr->ypos = termptr->newy ;
	if( dimptr->numpins >= BREAK_PT2 ) { 
	    dimptr->Lnum = dimptr->newLnum ;
	    dimptr->Rnum = dimptr->newRnum ;
	    dimptr->Bnum = dimptr->newBnum ;
	    dimptr->Tnum = dimptr->newTnum ;
	}
    }
    return ;
} /* end upin_accept */
