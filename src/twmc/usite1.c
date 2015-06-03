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
FILE:	    usite1.c                                       
DESCRIPTION:single cell move and/or orientation flip
CONTENTS:   BOOL usite1( )
DATE:	    Feb 3, 1988 
REVISIONS:  Feb 5, 1988 - changed old_apos, new_apos to global
	        variables, old method remains commented out. 
	    Aug 25, 1988 - calls to ufixpin and usoftpin changed.
	    Oct 21, 1988 - changed to sqrt of overlap penalty.
	    Nov 25, 1988 - added timing driven code
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) usite1.c version 3.4 9/10/90" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

BOOL usite1( /* old_apos, new_apos */ )
/* MOVEBOXPTR  *old_apos, *new_apos ; */
{

CELLBOXPTR acellptr ;
PINBOXPTR anewtermptr ;
MOVEBOXPTR pos ;         /* temp pointer for easier access */

INT cost , newpenalty, newbinpenal, newtimepenalty, newtimepenal ;
INT a, i ;
INT oldBinX, oldBinY, array_limit, *oldCellList ;
INT wire_chg, time ;
DOUBLE delta_wire ;

/* ----------------------------------------------------------------- 
   global information is stored in element zero of position arrays 
   set a's MOVEBOX record - new position records have been initialized
   in uloop(). 
*/
a = new_apos0G->cell ;
acellptr = cellarrayG[a] ;
anewtermptr  = acellptr->pinptr ;

clear_net_set() ; /* reset set to mark nets that have changed position */

newbinpenal = binpenalG ;

newbinpenal += overlap( /* old_aposG, new_aposG */ ) ;

/* scale new penalty for feedback circuit */
newpenalty = (INT) ( lapFactorG * sqrt( (DOUBLE) newbinpenal ) ) ;

upin_test( anewtermptr, new_apos0G ) ;

cost = funccostG ;

cost += unet( anewtermptr ) ;

newtimepenal = timingpenalG ;
newtimepenal += calc_incr_time( a ) ;

ASSERT( newtimepenal == dcalc_full_penalty(),"usite1","Timing woes\n") ;

/* scale new timing penalty */
newtimepenalty = (INT) ( timeFactorG * (DOUBLE) newtimepenal ) ;

wire_chg = cost - funccostG ;

if( acceptt( penaltyG - newpenalty - wire_chg + 
    timingcostG - newtimepenalty )){

    upin_accept( anewtermptr ) ;

    update_overlap( /* old_aposG */ ) ;

    update_time( a ) ;

    /* update new position and orientation of a cell */
    acellptr->xcenter = new_apos0G->xcenter ;
    acellptr->ycenter = new_apos0G->ycenter ;
    acellptr->orient  = new_apos0G->orient  ;

    /* *** BEGIN UPDATE OF BIN CELL LIST *** */
    oldBinX = SETBINX(old_apos0G->xcenter) ;
    oldBinY = SETBINY(old_apos0G->ycenter) ;
    oldCellList = binptrG[oldBinX][oldBinY]->cells ;

    /* do nothing if cell a remains in same bin */
    if( oldCellList != newCellListG ){
	
	/* add cell a to newcellList - check space */
	array_limit = ++newCellListG[0] ;
	if( array_limit >= newbptrG->space ) {
	    newbptrG->space += EXPCELLPERBIN ;
	    /* need to worry about 2 info array positions */
	    /* the 2 is assumed to avoid a subtraction */
	    newCellListG = (INT *) Ysafe_realloc( newCellListG,
		(newbptrG->space) *sizeof(INT) ) ; 
	}
	newCellListG[array_limit] = a ;

	/* remove a for other list */
	/* find a 's position in array */
	array_limit = oldCellList[0] ;
	for( i=1;i<=array_limit;i++ ){
	    if( oldCellList[i] == a ){
		break ;
	    }
	}
	/* assert i now has correct value of a */
	ASSERT( oldCellList[i] == a, "usite1",
	    "Problem in oldbin cell lists\n" ) ;

	if( i != array_limit ){
	    /* a's position is not last position swap with last */
	    oldCellList[ i ] = oldCellList[ array_limit ] ;
	}
	oldCellList[0]-- ; 
    } /* *** END UPDATE OF BIN CELL LIST *** */

    /* debug check */
    ASSERT( checkbinList(), "usite1","We have a problem here \n") ;

    funccostG = cost ;
    penaltyG = newpenalty ;
    binpenalG = newbinpenal ;
    timingcostG = newtimepenalty ;
    timingpenalG = newtimepenal ;

    return (ACCEPT) ;
} else {
    return (REJECT) ;
}
} /* end usite1 */
