/*
 *   Copyright (C) 1989-1991 Yale University
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
FILE:	    uinst.c                                       
DESCRIPTION:change cell instance
CONTENTS:   
DATE:	    Apr 4, 1989 - original coding WPS 
REVISIONS:  Oct 3, 1989 - fixed problem with overlap calculation.
	    Apr 23, 1990 - modified to work with new softpin code.
	    Mon Feb  4 02:22:36 EST 1991 - fixed problem with 
		numsides not being set.
	    Sat Apr 27 01:10:04 EDT 1991 - fixed problem with
		aspect ratio calculation.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) uinst.c version 3.9 11/23/91" ;
#endif

#include <custom.h>
#include <initialize.h>
#include <yalecad/debug.h>
#include <yalecad/relpos.h>

#define NEWVERTICES TRUE
#define HOWMANY     0

BOOL uinst( /* old_apos, new_apos */ )
/* MOVEBOXPTR  *old_apos, *new_apos ; */
{

CELLBOXPTR acellptr ;
PINBOXPTR anewtermptr, pin ;
BOUNBOXPTR bounptr ;
MOVEBOXPTR newtile ;     /* temp pointer for easier access */
VERTBOXPTR vert ;        /* the vertices of the cell */
TILEBOXPTR tptr ;        /* temp pointer for tiles */
INSTBOXPTR instptr ;     /* pointer to the cells instance box */
DOUBLE aspFactor ;
DOUBLE val ;

INT cost , newpenalty, newbinpenal, newtimepenalty, newtimepenal ;
INT a, i ;
INT lastinst, newinst ;
INT aorient, axcenter, aycenter ;
INT oleft, obottom, oright, otop ;
INT Hdiv2, Wdiv2 ;
INT *xorig, *xnew, *yorig, *ynew ;
INT numsides, howmany ;

/* ----------------------------------------------------------------- 
   global information is stored in element zero of position arrays 
   set a's MOVEBOX record - new position records have been initialized
   in uloop(). 
*/
a = new_apos0G->cell ;
acellptr = cellarrayG[a] ;
anewtermptr = acellptr->pinptr ;
aorient = acellptr->orient ;
instptr = acellptr->instptr ;
lastinst = instptr->numinstances - 1 ;

do {
    newinst = PICK_INT( 0, lastinst ) ;
} while( newinst == acellptr->cur_inst ) ;

clear_net_set() ; /* reset set to mark nets that have changed position */

newbinpenal = binpenalG ;

/* these are common to both soft and hard cell moves */
acellptr->numsides = instptr->numsides[newinst] ;
vert = acellptr->vertices = instptr->vert_inst[newinst] ;

if( acellptr->softflag ){
    /* ----------------------------------------------------------------
	Load old_aposG normally but we must load new_aposG with new 
	tiles.  Install the new tiles in new_aposG - make sure we 
	count all tiles.  In the case of soft cells adjust aspect
	ratio so it coincides with current ratio to maximize chance
	move will get accepted.
    ------------------------------------------------------------------ */
    /* determine aspect ratio orientation */
    aspFactor = sqrt( acellptr->aspect / acellptr->orig_aspect ) ;

    /* use original bounding box to calculate changes */
    bounptr = instptr->bounBox[newinst][8] ; /* original bbox */
    oleft = bounptr->l ;
    obottom = bounptr->b ;
    oright = bounptr->r ;
    otop = bounptr->t ;
    val = (DOUBLE) (otop - obottom) * aspFactor ;
    Hdiv2 = ROUND( val ) / 2 ;
    val = (DOUBLE) (oright - oleft) / aspFactor ;
    Wdiv2 = ROUND( val ) / 2 ;


    /* calculate the new aspect ratio of softtiles */
    i = 0 ;
    new_apos0G->numtiles = instptr->numtile_inst[newinst] ;
    new_apos0G->loaded_previously = TRUE ;
    for( tptr = instptr->tile_inst[newinst];tptr;tptr = tptr->next ){
	newtile = new_aposG[++i] ;
	val = (DOUBLE)(tptr->orig_left - oleft) / aspFactor;
	newtile->l = ROUND( val ) - Wdiv2 ;
	val = (DOUBLE)(tptr->orig_right - oleft) / aspFactor;
	newtile->r = ROUND( val ) - Wdiv2 ;
	val = (DOUBLE)(tptr->orig_bottom - obottom) * aspFactor;
	newtile->b = ROUND( val ) - Hdiv2 ;
	val = (DOUBLE)(tptr->orig_top - obottom) * aspFactor;
	newtile->t = ROUND( val ) - Hdiv2 ;

	newtile->lw = tptr->lweight ;
	newtile->rw = tptr->rweight ;
	newtile->bw = tptr->bweight ;
	newtile->tw = tptr->tweight ;
    } /* end calculation of new tiles */

    /* ----------------------------------------------------------------
	Modify the vertices of the cell for the softpin moves.
	Temporarily set the vertices field and number of sides of the
	instance to the new instance so that calculations in placepin.c
	will work properly.  We must set this back if we fail.
    ------------------------------------------------------------------ */
    xorig = vert->x_orig ;
    xnew = vert->x_new ;
    yorig = vert->y_orig ;
    ynew = vert->y_new ;
    for( i = 1, numsides=instptr->numsides[newinst]; i <= numsides; i++ ){
	val = (DOUBLE)(xorig[i] - oleft) / aspFactor ;
	xnew[i] = ROUND( val ) - Wdiv2 ;
	val = (DOUBLE)(yorig[i] - obottom) * aspFactor ;
	ynew[i] = ROUND( val ) - Hdiv2 ;
    }

} else { /* hard or custom cell */
    /* ----------------------------------------------------------------
	Load old_aposG normally but we must load new_aposG with new 
	tiles.  Install the new tiles in new_aposG - make sure we 
	count all tiles 
    ------------------------------------------------------------------ */
    i = 0 ;
    new_apos0G->numtiles = instptr->numtile_inst[newinst] ;
    new_apos0G->loaded_previously = TRUE ;
    for( tptr = instptr->tile_inst[newinst];tptr;tptr = tptr->next ){
	newtile = new_aposG[++i] ;
	newtile->l = tptr->left ;
	newtile->r = tptr->right ;
	newtile->b = tptr->bottom ;
	newtile->t = tptr->top ;

	newtile->lw = tptr->lweight ;
	newtile->rw = tptr->rweight ;
	newtile->bw = tptr->bweight ;
	newtile->tw = tptr->tweight ;
    } /* end calculation of new tiles */
} /* end loading tiles into movebox */

newbinpenal += overlap( /* old_aposG, new_aposG */ ) ;

/* scale new penalty for feedback circuit */
newpenalty = (INT) ( lapFactorG * sqrt( (DOUBLE) newbinpenal ) ) ;

/* -------------- update the position of all pins --------------------- */
axcenter = acellptr->xcenter ;
aycenter = acellptr->ycenter ;
if( acellptr->softflag ){
    /* ****** the soft pin case ****** */
    for( pin = anewtermptr ; pin ; pin = pin->nextpin ) {

	if( pin->type == HARDPINTYPE ){
	    val = (DOUBLE)(pin->txpos_orig[newinst]-oleft) / aspFactor ;
	    pin->txpos_new = ROUND( val ) - Wdiv2 ;
	    val = (DOUBLE)(pin->typos_orig[newinst]-obottom) * aspFactor ;
	    pin->typos_new = ROUND( val ) - Hdiv2 ;
	}
    }
    howmany = (INT) acellptr->softpins[HOWMANY] ;
    /* set each pin's correct instance */
    for( i = 1 ; i <= howmany; i++ ){
	pin = acellptr->softpins[i] ;
	pin->softinfo = pin->soft_inst[newinst] ;
    }
    /* now determine placement of pins using new vertices for the cell */
    placepin( a, NEWVERTICES ) ;

} else {
    /* ****** the hard pin case ****** */
    for( pin = anewtermptr ; pin ; pin = pin->nextpin ) {

	pin->txpos_new = pin->txpos_orig[newinst] ;
	pin->typos_new = pin->typos_orig[newinst] ;

	netarrayG[ pin->net ]->nflag = TRUE ;
	pin->flag = TRUE ;
	/* rel position is a macro which calculates absolute pin loc */
	/* defined in relpos.h */
	REL_POS( aorient, 
	    pin->newx, pin->newy,                  /* result global */ 
	    pin->txpos_new, pin->typos_new,        /* cell relative */
	    axcenter, aycenter ) ;                 /*  cell center  */
    }
}

cost = funccostG ;

cost += unet( anewtermptr ) ;

newtimepenal = timingpenalG ;
newtimepenal += calc_incr_time( a ) ;

ASSERT( newtimepenal == dcalc_full_penalty(),NULL,NULL) ;

/* scale new timing penalty */
newtimepenalty = (INT) ( timeFactorG * (DOUBLE) newtimepenal ) ;

/* ------------------------------------------------------------------
    Note: we don't take overfill into account here because changing 
    instance may upset overfill and it is a small worry.  When we
    change to deterministic pin placement, problem will be eliminated.
    ----------------------------------------------------------------- */

if( acceptt( funccostG + penaltyG + timingcostG - cost - newpenalty - 
    newtimepenalty )){

    acellptr->cur_inst = newinst ;

    upin_accept( anewtermptr ) ;

    /* update current terminal positions from original positions */
    for( pin = anewtermptr ; pin ; pin = pin->nextpin ) {
	pin->txpos = pin->txpos_new ;
	pin->typos = pin->typos_new ;
	if( pin->type != HARDPINTYPE ){
	    pin->txpos_orig[newinst] = pin->txpos ;
	    pin->typos_orig[newinst] = pin->typos ;
	}
    }
    /* update tiles */
    acellptr->tiles = instptr->tile_inst[newinst] ;
    acellptr->numtiles = instptr->numtile_inst[newinst] ;
    if( acellptr->softflag ){
	/* update the aspect ratio of softtile */
	for( tptr = acellptr->tiles;tptr;tptr = tptr->next ){
	    val = (DOUBLE)(tptr->orig_left - oleft) / aspFactor;
	    tptr->left = ROUND( val ) - Wdiv2 ;
	    val = (DOUBLE)(tptr->orig_right - oleft) / aspFactor;
	    tptr->right = ROUND( val ) - Wdiv2 ;
	    val = (DOUBLE)(tptr->orig_bottom - obottom) * aspFactor;
	    tptr->bottom = ROUND( val ) - Hdiv2 ;
	    val = (DOUBLE)(tptr->orig_top - obottom) * aspFactor;
	    tptr->top = ROUND( val ) - Hdiv2 ;
	} /* end calculation of new tiles */

	/* update the vertices of the cell by swapping arrays */
	xnew = vert->x_new ;
	vert->x_new = vert->x ;
	vert->x = xnew ;
	ynew = vert->y_new ;
	vert->y_new = vert->y ;
	vert->y = ynew ;
    }
    /* delay recalculation of bounding box views until needed */
    acellptr->bounBox = instptr->bounBox[newinst] ;
    acellptr->boun_valid = FALSE ;

    update_overlap( /* old_aposG */ ) ;

    update_time( a ) ;

    funccostG = cost ;
    penaltyG = newpenalty ;
    binpenalG = newbinpenal ;
    timingcostG = newtimepenalty ;
    timingpenalG = newtimepenal ;

    return (ACCEPT) ;
} else {
    /* we failed we must set vertices back if it is a soft cell */
    lastinst = acellptr->cur_inst ;
    acellptr->vertices = instptr->vert_inst[lastinst] ;
    acellptr->numsides = instptr->numsides[lastinst] ;
    if( acellptr->softflag ){
	/* set each pin's correct instance */
	for( i = 1 ; i <= howmany; i++ ){
	    pin = acellptr->softpins[i] ;
	    pin->softinfo = pin->soft_inst[lastinst] ;
	}
    }
    return (REJECT) ;
}
} /* end uinst */
