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
FILE:	    ucxxo2.c                                       
DESCRIPTION:pairwise exchange with oriention change.
CONTENTS:   ucxxo2( newaor , newbor )
		INT newaor , newbor ;
DATE:	    Mar 27, 1989 
REVISIONS:  Mon Aug 12 17:01:03 CDT 1991 - changed timing ASSERTIONS
		to D( ) constructs to speed execution time during
		debug mode.
	    Thu Sep 19 14:15:51 EDT 1991 - added equal width cell
		capability.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) ucxxo2.c (Yale) version 4.8 2/23/92" ;
#endif
#endif

#include "ucxxglb.h"
#include "readpar.h"
#include <yalecad/debug.h>

ucxxo2( newaor , newbor )
INT newaor , newbor ;
{

CBOXPTR acellptr , bcellptr ;
TIBOXPTR atileptr , btileptr ;
PINBOXPTR antrmptr , bntrmptr ;
DOUBLE temp ;
INT cost , error_light_is_on ;
INT axcenter , aycenter , bxcenter , bycenter ;
INT aleft , aright ;
INT bleft , bright ;
INT a1LoBin, a2LoBin, b1LoBin, b2LoBin ;
INT a1HiBin, a2HiBin, b1HiBin, b2HiBin ;
INT startxa1 , endxa1 , startxa2 , endxa2 ;
INT startxb1 , endxb1 , startxb2 , endxb2 ;
INT truth ;
INT newtimepenal ;
INT newpenal ;
INT wire_chg ;

acellptr = carrayG[ aG ]    ;
axcenter = acellptr->cxcenter ;
aycenter = acellptr->cycenter ;
atileptr = acellptr->tileptr ;
aleft    = atileptr->left    ;
aright   = atileptr->right   ;
antrmptr = acellptr->pins ;

bcellptr = carrayG[ bG ]        ;
bxcenter = bcellptr->cxcenter ;
bycenter = bcellptr->cycenter ;
btileptr = bcellptr->tileptr  ;
bleft    = btileptr->left     ;
bright   = btileptr->right    ;
bntrmptr = bcellptr->pins ;

newbinpenalG = binpenalG ;
newrowpenalG = rowpenalG ;

if( Equal_Width_CellsG ){
    newpenal = 0 ;
} else {
    new_old( bright - bleft - aright + aleft ) ; 

    a1LoBin = SetBin( startxa1 = axcenter + aleft    ) ;
    a1HiBin = SetBin( endxa1   = axcenter + aright   ) ; 
    b1LoBin = SetBin( startxb1 = bxcenter + bleft    ) ; 
    b1HiBin = SetBin( endxb1   = bxcenter + bright   ) ; 
    a2LoBin = SetBin( startxa2 = bxcenter + aleft    ) ;
    a2HiBin = SetBin( endxa2   = bxcenter + aright   ) ; 
    b2LoBin = SetBin( startxb2 = axcenter + bleft    ) ; 
    b2HiBin = SetBin( endxb2   = axcenter + bright   ) ; 

    old_assgnto_new2( a1LoBin , a1HiBin , b1LoBin , b1HiBin , 
		      a2LoBin , a2HiBin , b2LoBin , b2HiBin ) ;
    sub_penal( startxa1 , endxa1 , ablockG , a1LoBin , a1HiBin ) ;
    sub_penal( startxb1 , endxb1 , bblockG , b1LoBin , b1HiBin ) ;
    add_penal( startxa2 , endxa2 , bblockG , a2LoBin , a2HiBin ) ;
    add_penal( startxb2 , endxb2 , ablockG , b2LoBin , b2HiBin ) ;

    newpenal = (INT)( roLenConG * (DOUBLE) newrowpenalG +
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
} /* end !Equal_Width_CellsG */

term_newpos( antrmptr , bxcenter , bycenter , newaor ) ;
term_newpos( bntrmptr , axcenter , aycenter , newbor ) ;

cost = funccostG ;

clear_net_set() ; /* reset set to mark nets that have changed position */
/* dimbox routines mark the nets that have changed */
new_dbox2( antrmptr , bntrmptr , &cost ) ;

newtimepenal = timingcostG ;
newtimepenal += calc_incr_time2( aG, bG ) ;

D( "twsc/check_timing",
    ASSERT( dcalc_full_penalty(newtimepenal),NULL,"time problem") ;
) ;


wire_chg = (cost - funccostG) + (newtimepenal - timingcostG) ;

truth =  acceptt( funccostG-cost,timingcostG-newtimepenal, penaltyG-newpenal ) ;

if( truth ) {

    if(!(Equal_Width_CellsG)){
	if( error_light_is_on ) {
	    error_countG++ ;
	}

	new_assgnto_old2( a1LoBin , a1HiBin , b1LoBin , b1HiBin , 
			  a2LoBin , a2HiBin , b2LoBin , b2HiBin ) ;
    }
    dbox_pos( antrmptr ) ;
    dbox_pos( bntrmptr ) ;   
    update_time2() ;
    if( cellaptrG != cellbptrG ) {
	remv_cell( cellaptrG , ApostG ) ;
	remv_cell( cellbptrG , BpostG ) ;
	add_cell( &binptrG[bblockG][ SetBin(bxcenter) ]->cell , aG ) ;
	add_cell( &binptrG[ablockG][ SetBin(axcenter) ]->cell , bG ) ;
    }

    if( wire_chg < 0 ) {
	temp = (DOUBLE) - wire_chg ;
	total_wire_chgG += temp ;
	sigma_wire_chgG += (temp - mean_wire_chgG) * 
					(temp - mean_wire_chgG) ;
	wire_chgsG++ ;
    }
    /*
    if( wire_chg < max_wire_chg ) {
	max_wire_chg = wire_chg ;
    }
    */
    G( erase_a_cell( aG, axcenter, aycenter ) ) ;
    G( erase_a_cell( bG, bxcenter, bycenter ) ) ;

    acellptr->cblock   = bblockG   ;
    acellptr->cxcenter = bxcenter ;
    acellptr->cycenter = bycenter ;
    bcellptr->cblock   = ablockG   ;
    bcellptr->cxcenter = axcenter ;
    bcellptr->cycenter = aycenter ;

    acellptr->corient = newaor ;
    bcellptr->corient = newbor ;

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
    G( graphics_cell_update( bG ) ) ;
    return( 1 ) ;
} else {
    G( graphics_cell_attempt( aG ) ) ;
    G( graphics_cell_attempt( bG ) ) ;
    return( 0 ) ;
}
}
