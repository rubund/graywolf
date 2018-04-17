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
FILE:	    ucxx2.c                                       
DESCRIPTION:pairwise exchange.
CONTENTS:   ucxx2( )
	    find_new_pos()
	    add_cell( cellptr , c ) 
		INT **cellptr , c ;
DATE:	    Mar 27, 1989 
REVISIONS:  Mon Aug 12 17:01:03 CDT 1991 - changed timing ASSERTIONS
		to D( ) constructs to speed execution time during
		debug mode.
	    Thu Aug 22 22:31:52 CDT 1991 - Carl made change to
		fence case.
	    Thu Sep 19 14:15:51 EDT 1991 - added equal width cell
		capability.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) ucxx2.c (Yale) version 4.10 4/2/92" ;
#endif
#endif

#include "ucxxglb.h"
#include "readpar.h"
#include <yalecad/debug.h>

static INT anxcenterS , bnxcenterS ;

ucxx2( )
{

CBOXPTR acellptr , bcellptr ;
TIBOXPTR atileptr , btileptr ;
PINBOXPTR atermptr , btermptr ;
INT error_light_is_on ;
INT cost ;
INT aorient , borient ;
INT a1LoBin, a1HiBin, b1LoBin, b1HiBin ;
INT a2LoBin, a2HiBin, b2LoBin, b2HiBin ;
INT startxa1 , endxa1 , startxa2 , endxa2 ;
INT startxb1 , endxb1 , startxb2 , endxb2 ;
INT newtimepenal ;
INT newpenal ;
INT anbin , bnbin , i ;
INT truth ;
INT wire_chg ;
DOUBLE temp ;

INT abin, bbin ; /* temporary */

acellptr = carrayG[ aG ]    ; 
axcenterG = acellptr->cxcenter ; 
aycenterG = acellptr->cycenter ; 
aorient  = acellptr->corient  ; 
atileptr = acellptr->tileptr ; 
aleftG    = atileptr->left    ; 
arightG   = atileptr->right   ; 
atermptr = acellptr->pins ; 

bcellptr = carrayG[ bG ]    ; 
bxcenterG = bcellptr->cxcenter ; 
bycenterG = bcellptr->cycenter ; 
borient  = bcellptr->corient  ; 
btileptr = bcellptr->tileptr ; 
bleftG    = btileptr->left    ; 
brightG   = btileptr->right   ; 
btermptr = bcellptr->pins ; 

newbinpenalG = binpenalG ;
newrowpenalG = rowpenalG ;

/*----------- exchange the centers of both cells -------------*/

if( Equal_Width_CellsG ){
    newpenal = 0 ;
    anxcenterS = bxcenterG ;
    bnxcenterS = axcenterG ;
} else {
    new_old( brightG-bleftG-arightG+aleftG ) ;
    find_new_pos() ;

    a1LoBin = SetBin( startxa1 = axcenterG + aleftG  ) ; 
    a1HiBin = SetBin( endxa1   = axcenterG + arightG ) ; 
    b1LoBin = SetBin( startxb1 = bxcenterG + bleftG  ) ; 
    b1HiBin = SetBin( endxb1   = bxcenterG + brightG ) ; 
    a2LoBin = SetBin( startxa2 = anxcenterS + aleftG  ) ; 
    a2HiBin = SetBin( endxa2   = anxcenterS + arightG ) ; 
    b2LoBin = SetBin( startxb2 = bnxcenterS + bleftG  ) ; 
    b2HiBin = SetBin( endxb2   = bnxcenterS + brightG ) ; 

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
}


term_newpos( atermptr , anxcenterS , bycenterG , aorient ) ; 
term_newpos( btermptr , bnxcenterS , aycenterG , borient ) ; 

cost = funccostG ; 
clear_net_set() ; /* reset set to mark nets that have changed position */

/* dimbox routines mark the nets that have changed */
new_dbox2( atermptr , btermptr , &cost ) ;

newtimepenal = timingcostG ;
newtimepenal += calc_incr_time2( aG, bG ) ;

D( "twsc/check_timing",
    ASSERT( dcalc_full_penalty(newtimepenal),NULL,"time problem") ;
) ;

wire_chg = (cost - funccostG) + (newtimepenal - timingcostG) ;

truth =  acceptt( funccostG-cost, timingcostG-newtimepenal,penaltyG-newpenal );

if( truth ) {

    if(!(Equal_Width_CellsG)){
	if( error_light_is_on ) {
	    error_countG++ ;
	}
	new_assgnto_old2( a1LoBin , a1HiBin , b1LoBin , b1HiBin , 
			  a2LoBin , a2HiBin , b2LoBin , b2HiBin ) ;
    }
    dbox_pos( atermptr ) ;
    dbox_pos( btermptr ) ;

    update_time2() ;
    anbin = SetBin( anxcenterS ) ;
    bnbin = SetBin( bnxcenterS ) ;
    if( cellaptrG != cellbptrG ) {
	remv_cell( cellaptrG , ApostG ) ;
	remv_cell( cellbptrG , BpostG ) ;
	add_cell( &binptrG[bblockG][anbin]->cell , aG ) ;
	add_cell( &binptrG[ablockG][bnbin]->cell , bG ) ;
    } else {
	/* This will never happen for Equal_Width_Cells */
	remv_cell( cellaptrG , ApostG ) ;
	for( i = 1 ; i <= *cellaptrG ; i++ ) {
	    if( cellaptrG[i] == bG ) {
		break ;
	    }
	}
	remv_cell( cellaptrG , i ) ;
	add_cell( &binptrG[ablockG][anbin]->cell , aG ) ;
	add_cell( &binptrG[ablockG][bnbin]->cell , bG ) ;
    }
    if( wire_chg < 0 ) {
	temp = (DOUBLE) - wire_chg ;
	total_wire_chgG += temp ;
	sigma_wire_chgG += (temp - mean_wire_chgG) * 
					(temp - mean_wire_chgG) ;
	wire_chgsG++ ;
    }
    G( erase_a_cell( aG, axcenterG, aycenterG ) ) ;
    G( erase_a_cell( bG, bxcenterG, bycenterG ) ) ;
 
    acellptr->cblock   = bblockG    ;
    acellptr->cxcenter = anxcenterS ; 
    acellptr->cycenter = bycenterG  ; 
    bcellptr->cblock   = ablockG    ;
    bcellptr->cxcenter = bnxcenterS ; 
    bcellptr->cycenter = aycenterG  ; 

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


find_new_pos()
{

INT newA_l , newA_r , newB_l , newB_r ;
INT oldA_l , oldA_r , oldB_l , oldB_r ;
INT span , target , target_l , target_r , blkLeft , blkRite ;
INT dist1 , dist2 ;


newA_l = bxcenterG + aleftG  ;
newA_r = bxcenterG + arightG ;
newB_l = axcenterG + bleftG  ;
newB_r = axcenterG + brightG ;
if( (ablockG == bblockG) && (!( (newA_l >= newB_r) || (newB_l >= newA_r)))){
    /*  
     *   Then some overlapping will exist in the new positions
     */
    oldA_l = axcenterG + aleftG  ;
    oldA_r = axcenterG + arightG ;
    oldB_l = bxcenterG + bleftG  ;
    oldB_r = bxcenterG + brightG ;
    if( !( (oldA_l >= oldB_r) || (oldB_l >= oldA_r) ) ) {
	/*  
	 *    There was also initial overlap; therefore
	 *    move cells apart.  Find the minimum distance
	 *    necessary to pull them apart.
	 */
	span = arightG - aleftG + brightG - bleftG ;
	target = ( ((oldA_l <= oldB_l) ? oldA_l : oldB_l) +
		   ((oldA_r >= oldB_r) ? oldA_r : oldB_r) ) / 2 ;
	target_l = target - span / 2 ;
	target_r = target_l + span ;
	blkLeft = barrayG[ablockG]->bxcenter + barrayG[ablockG]->bleft ;
	blkRite = blkLeft + barrayG[ablockG]->desire  ;
	if( target_l < blkLeft ) {
	    target_l += blkLeft - target_l ;
	    target_r += blkLeft - target_l ;
	} else if( target_r > blkRite ) {
	    target_l -= target_r - blkRite ;
	    target_r -= target_r - blkRite ;
	}
	dist1 = newA_r - newB_l ;
	dist2 = newB_r - newA_l ;
	if( dist1 <= dist2 ) {
	    anxcenterS = target_l - aleftG  ;
	    bnxcenterS = target_r - brightG ;
	} else {
	    anxcenterS = target_r - arightG ;
	    bnxcenterS = target_l - bleftG  ;
	}
    } else {
	/*  
	 *    No initial overlap, but some would be created
	 *    as it stands now.  Eliminate the possibility of
	 *    overlap by maintaining the original left
	 *    bound and the original right bound.
	 */
	if( oldA_l <= oldB_l ) {
	    bnxcenterS = oldA_l - bleftG  ;
	    anxcenterS = oldB_r - arightG ;
	} else {
	    anxcenterS = oldB_l - aleftG  ;
	    bnxcenterS = oldA_r - brightG ;
	}
    }
} else {
    anxcenterS = bxcenterG ;
    bnxcenterS = axcenterG ;
}
return ;
}
 

add_cell( cellptr , c ) 
INT **cellptr , c ;
{

INT k ;

if( (k = ++(**cellptr)) % 10 == 0 ) {
    *cellptr = (INT *) Ysafe_realloc( *cellptr, (k + 10) * sizeof(INT));

}
(*cellptr)[k] = c ;

return ;
}




/*
remv_cell( cellptr , post )
INT *cellptr , post ;
{

if( *cellptr != post ) {
    cellptr[ post ] = cellptr[ *cellptr ] ;
}
(*cellptr)-- ;
}
*/
