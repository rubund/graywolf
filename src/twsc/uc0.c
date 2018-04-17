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
FILE:	    uc0.c                                       
DESCRIPTION:orientation change.
CONTENTS:   uc0( a , newaor )
		INT a , newaor ;
DATE:	    Mar 27, 1989 
REVISIONS:  Mon Aug 12 17:01:03 CDT 1991 - changed timing ASSERTIONS
		to D( ) constructs to speed execution time during
		debug mode.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) uc0.c (Yale) version 4.6 2/23/92" ;
#endif
#endif

#include "standard.h"
#include <yalecad/debug.h>


uc0( a , newaor )
INT a , newaor ;
{

CBOXPTR acellptr ;
PINBOXPTR antrmptr ;
INT cost , truth ;
INT newtimepenal ;


acellptr = carrayG[ a ]    ;
antrmptr = acellptr->pins ;

term_newpos( antrmptr, acellptr->cxcenter, acellptr->cycenter, newaor );

cost = funccostG ;

clear_net_set() ; /* reset set to mark nets that have changed position */
/* dimbox routines mark the nets that have changed */
new_dbox( antrmptr , &cost ) ;

newtimepenal = timingcostG ;
newtimepenal += calc_incr_time( a ) ;

D( "twsc/check_timing",
    ASSERT( dcalc_full_penalty(newtimepenal),"uc0","time problem") ;
) ;

truth =  acceptt( funccostG - cost, timingcostG - newtimepenal, 0  ) ; 
if( truth ) {

    dbox_pos( antrmptr ) ;
    update_time( a ) ;
    acellptr->corient = newaor ;
    funccostG = cost ;
    timingcostG  = newtimepenal ;
    G( graphics_cell_update( a ) ) ;
} else {
    G( graphics_cell_attempt( a ) ) ;
}
return ;
}
