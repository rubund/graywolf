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
FILE:	    ucxx1.c                                       
DESCRIPTION:single cell move.
CONTENTS:   ucxx1( bxcenter , bycenter )
		INT bxcenter , bycenter ;
DATE:	    Mar 27, 1989 
REVISIONS:  Mon Aug 12 17:01:03 CDT 1991 - changed timing ASSERTIONS
		to D( ) constructs to speed execution time during
		debug mode.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) ucxx1.c (Yale) version 4.7 2/23/92" ;
#endif
#endif

#include "ucxxglb.h"
#include <yalecad/debug.h>


ucxx1( bxcenter , bycenter )
INT bxcenter , bycenter ;
{

CBOXPTR acellptr ;
TIBOXPTR atileptr ;
PINBOXPTR atermptr ;

INT cost , error_light_is_on ;
INT axcenter ;
INT aleft , aright ;
INT a1LoBin , a1HiBin , b1LoBin , b1HiBin ;
INT startxa1 , endxa1 , startxb1 , endxb1 ;
INT truth , aorient ;
INT newpenal, newtimepenal ;

acellptr = carrayG[ aG ]    ; 
aorient  = acellptr->corient ;
axcenter = acellptr->cxcenter ; 
atileptr = acellptr->tileptr  ;
aleft    = atileptr->left    ; 
aright   = atileptr->right   ; 
atermptr = acellptr->pins ; 
 
newbinpenalG = binpenalG ;
newrowpenalG = rowpenalG ;

new_old( aleft-aright ) ;

a1LoBin = SetBin( startxa1 = axcenter + aleft  ) ; 
a1HiBin = SetBin( endxa1   = axcenter + aright ) ; 
b1LoBin = SetBin( startxb1 = bxcenter + aleft  ) ; 
b1HiBin = SetBin( endxb1   = bxcenter + aright ) ; 

old_assgnto_new1( a1LoBin , a1HiBin , b1LoBin , b1HiBin ) ;

sub_penal( startxa1 , endxa1 , ablockG , a1LoBin , a1HiBin ) ; 
add_penal( startxb1 , endxb1 , bblockG , b1LoBin , b1HiBin ) ; 

newpenal = (INT)(roLenConG * (DOUBLE) newrowpenalG +
				binpenConG * (DOUBLE) newbinpenalG ) ;
error_light_is_on = 0 ;
if( newpenal - penaltyG > P_limitG ) {
    if( potential_errorsG < 100 ) {
	++potential_errorsG ;
        error_light_is_on = 1 ;
    } else {
	earlyRejG++ ;
	return( -1 ) ;
    }
}
term_newpos( atermptr , bxcenter , bycenter , aorient ) ; 

cost = funccostG ; 

clear_net_set() ; /* reset set to mark nets that have changed position */
/* dimbox routines mark the nets that have changed */
new_dbox( atermptr , &cost ) ;

newtimepenal = timingcostG ;
newtimepenal += calc_incr_time( aG ) ;

D( "twsc/check_timing",
    ASSERT( dcalc_full_penalty( newtimepenal ),"ucxx1","time problem") ;
) ;


truth = acceptt( funccostG-cost, timingcostG-newtimepenal, penaltyG-newpenal ) ;
if( truth ) {
    if( error_light_is_on ) {
	error_countG++ ;
    }
    new_assgnto_old1( a1LoBin , a1HiBin , b1LoBin , b1HiBin ) ;
    dbox_pos( atermptr ) ;
    update_time( aG ) ;
    /* ****************************************************** */
    remv_cell( cellaptrG , ApostG ) ;
    add_cell( &binptrG[bblockG][ SetBin(bxcenter) ]->cell , aG ) ;
    /* ****************************************************** */
    G( erase_a_cell( aG, acellptr->cxcenter, acellptr->cycenter ) ) ;

    acellptr->cblock   = bblockG   ;
    acellptr->cxcenter = bxcenter ; 
    acellptr->cycenter = bycenter ; 

    funccostG = cost ; 
    binpenalG = newbinpenalG ;
    rowpenalG = newrowpenalG ;
    penaltyG  = newpenal ;
    timingcostG  = newtimepenal ;

    if( ablockG != bblockG ) {
	barrayG[ablockG]->oldsize = barrayG[ablockG]->newsize ;
	barrayG[bblockG]->oldsize = barrayG[bblockG]->newsize ;
    }
    G( graphics_cell_update( aG ) ) ;
    return( 1 ) ;
} else {
    G( graphics_cell_attempt( aG ) ) ;
    return( 0 ) ;
}
}
