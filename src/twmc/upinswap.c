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

/* ----------------------------------------------------------------- 
FILE:	    upinswap.c                                       
DESCRIPTION:place pins on the cell.
CONTENTS:   BOOL upinswap( a )
		INT a ;
DATE:	    Mar 30, 1990 - new vertion of pinswap code.
REVISIONS:  
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) upinswap.c version 3.3 9/5/90" ;
#endif

#include <custom.h>
#include <initialize.h>
#include <yalecad/debug.h>
#include <yalecad/relpos.h>

#define CURVERTICES FALSE

BOOL upinswap( a )
INT a ;
{

    CELLBOXPTR acellptr ;
    PINBOXPTR  pin, atermptr ;
    SOFTBOXPTR spin ;
    INT cost, inst ;
    INT newtimepenalty, newtimepenal ;

    acellptr = cellarrayG[ a ] ;
    atermptr = acellptr->pinptr ;

    /* ------------ update the position of all pins ------------------- */
    /* place pins using the current vertices for the cell */
    placepin( a, CURVERTICES ) ;

    cost = funccostG ;
    /* reset set to mark nets that have changed position */
    clear_net_set() ; 

    cost += unet( atermptr ) ;

    newtimepenal = timingpenalG ;
    newtimepenal += calc_incr_time( a ) ;
    ASSERT(newtimepenal==dcalc_full_penalty(),"upinswap","Timing woes\n");

    /* scale new timing penalty */
    newtimepenalty = (INT) ( timeFactorG * (DOUBLE) newtimepenal ) ;

    if( acceptt( funccostG + timingcostG - cost - newtimepenalty )){

	/* update current terminal positions from original positions */
	inst = acellptr->cur_inst ;
	for( pin = atermptr ; pin ; pin = pin->nextpin ) {
	    pin->txpos = pin->txpos_new ;
	    pin->typos = pin->typos_new ;
	    if( pin->type != HARDPINTYPE ){
		pin->txpos_orig[inst] = pin->txpos ;
		pin->typos_orig[inst] = pin->typos ;
	    }
	}

	upin_accept( atermptr ) ;
	update_time( a ) ;

	funccostG = cost ;
	timingcostG = newtimepenalty ;
	timingpenalG = newtimepenal ;

	/* debug function to check sanity of the pin positions */
	D( "upinswap", check_pin(a) ) ;

	return (ACCEPT) ;
    } else {
	return (REJECT) ;
    }
} /* end upinswap */


check_pin( a )
INT a ;
{
    INT x, y, l, r, t, b;
    INT orient ;
    CELLBOXPTR acellptr ;
    PINBOXPTR pin ;

    acellptr = cellarrayG[a] ;

    regenorient( a, a ) ;
    orient = acellptr->orient ;
    l = acellptr->bounBox[orient]->l + acellptr->xcenter ;
    b = acellptr->bounBox[orient]->b + acellptr->ycenter ;
    r = acellptr->bounBox[orient]->r + acellptr->xcenter ;
    t = acellptr->bounBox[orient]->t + acellptr->ycenter ;
    for( pin = acellptr->pinptr ; pin ; pin = pin->nextpin ) {
	REL_POS( orient, 
	    x, y,                        
	    pin->txpos, pin->typos,     
	    acellptr->xcenter, acellptr->ycenter );
	if( x != pin->xpos ){
	    fprintf( stderr,"xpin != xpos \n" ) ;
	    pin->xpos = x ;
	}
	if( y != pin->ypos ){
	    fprintf( stderr,"ypin != ypos \n" ) ;
	    pin->ypos = y ;
	}
	if( x < l || x > r ){
	    fprintf( stderr,"pin outside bounding box\n" ) ;
	}
	if( y < b || b > t ){
	    fprintf( stderr,"pin outside bounding box\n" ) ;
	}
    }
} /* end check_pin */
