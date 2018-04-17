/*
 *   Copyright (C) 1989-1992 Yale University
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
FILE:	    ucxxp.c                                       
DESCRIPTION:pairwise exchange.
CONTENTS:   ucxxp( a , b , anxcenter , bnxcenter )
		INT a , b , anxcenter , bnxcenter ;
DATE:	    Mar 27, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) ucxxp.c (Yale) version 4.6 2/23/92" ;
#endif
#endif

#include "ucxxglb.h"
#include <yalecad/debug.h>

ucxxp( a , b , anxcenter , bnxcenter )
INT a , b , anxcenter , bnxcenter ;
{

CBOXPTR acellptr , bcellptr ;
PINBOXPTR atermptr , btermptr ;

INT cost ;
INT aorient , borient ;
INT aycenter , bycenter ;
INT newtimepenal ;

acellptr = carrayG[ a ]    ; 
aycenter = acellptr->cycenter ; 
aorient  = acellptr->corient  ; 
atermptr = acellptr->pins ; 

bcellptr = carrayG[ b ]    ; 
bycenter = bcellptr->cycenter ; 
borient  = bcellptr->corient  ; 
btermptr = bcellptr->pins ; 

term_newpos( atermptr , anxcenter , bycenter , aorient ) ; 
term_newpos( btermptr , bnxcenter , aycenter , borient ) ; 

cost = funccostG ; 

clear_net_set() ; /* reset set to mark nets that have changed position */
/* dimbox routines mark the nets that have changed */
new_dbox2( atermptr , btermptr , &cost ) ;

newtimepenal = timingcostG ;
newtimepenal += calc_incr_time2( a, b ) ;

D( "twsc/check_timing",
    ASSERT( dcalc_full_penalty(newtimepenal),NULL,"time problem") ;
) ;


if( acceptt( funccostG - cost, timingcostG - newtimepenal, 0 ) ){

    dbox_pos( atermptr ) ;
    dbox_pos( btermptr ) ;
    update_time2() ;

    G( erase_a_cell( a, acellptr->cxcenter, aycenter ) ) ;
    G( erase_a_cell( b, bcellptr->cxcenter, bycenter ) ) ;

    acellptr->cxcenter = anxcenter ; 
    acellptr->cycenter = bycenter ; 
    bcellptr->cxcenter = bnxcenter ; 
    bcellptr->cycenter = aycenter ; 

    funccostG = cost ; 
    timingcostG  = newtimepenal ;
    G( graphics_cell_update( a ) ) ;
    G( graphics_cell_update( b ) ) ;
    return( 1 ) ;
} else {
    G( graphics_cell_attempt( a ) ) ;
    G( graphics_cell_attempt( b ) ) ;
    return( 0 ) ;
}
}
