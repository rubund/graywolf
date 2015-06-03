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
FILE:	    uaspect.c                                       
DESCRIPTION:change aspect ratio of cell
CONTENTS:   BOOL uaspect( int, DOUBLE )
		INT a ;
		DOUBLE newAspect ;
DATE:	    Jan 30, 1988 
REVISIONS:  Jul 22, 1988 - deleted aspect0 field in CELLBOX record.
	    Aug 25, 1988 - calls to ufixpin and usoftpin changed.
	    Oct 22, 1988 - changed to sqrt of overlap penalty
	    Apr  3, 1989 - modified to allow multiple tiles for
		aspect ratio change.
	    Apr 23, 1990 - modified for the new pin code.
	    Sat Apr 27 01:09:01 EDT 1991 - fixed problem with aspect
		ratio and added aspect ratio initialization.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) uaspect.c version 3.5 4/27/91" ;
#endif

#include <custom.h>
#include <initialize.h>
#include <yalecad/debug.h>
#include <yalecad/relpos.h>

#define NEWVERTICES TRUE

BOOL uaspect( a , newAspect )
INT a ;
DOUBLE newAspect ;
{

CELLBOXPTR acellptr ;
TILEBOXPTR atileptr ;
PINBOXPTR  pin ;
MOVEBOXPTR newtile ; 
BOUNBOXPTR bounptr ;
VERTBOXPTR vert ;
SOFTBOXPTR spin ;
DOUBLE aspFactor , val ;
INT cost, newpenalty, newbinpenal ;
INT Hdiv2, Wdiv2 ;
INT site, aorient ;
INT i, inst ;
INT oleft, obottom, oright, otop ;
INT newtimepenalty, newtimepenal ;
INT *xorig ;
INT *xnew ;
INT *yorig ;
INT *ynew ;


acellptr = cellarrayG[ a ]    ;
aorient  = acellptr->orient  ;
inst = acellptr->cur_inst ;


/* determine aspect ratio orientation */
aspFactor = sqrt( newAspect / acellptr->orig_aspect ) ;

/* use original bounding box to calculate changes */
bounptr = acellptr->bounBox[8] ; /* original bbox */
oleft = bounptr->l ;
obottom = bounptr->b ;
oright = bounptr->r ;
otop = bounptr->t ;
val = (DOUBLE) (otop - obottom) * aspFactor ;
Hdiv2 = ROUND( val ) / 2 ;
val = (DOUBLE) (oright - oleft) / aspFactor ;
Wdiv2 = ROUND( val ) / 2 ;


/* calculate the new aspect ratio of softtiles */
i = 1 ;
new_apos0G->numtiles = acellptr->numtiles ;
new_apos0G->loaded_previously = TRUE ;
for( atileptr = acellptr->tiles;atileptr;atileptr = atileptr->next ){
    newtile = new_aposG[i++] ;
    val = (DOUBLE)(atileptr->orig_left - oleft) / aspFactor;
    newtile->l = ROUND( val ) - Wdiv2 ;
    val = (DOUBLE)(atileptr->orig_right - oleft) / aspFactor;
    newtile->r = ROUND( val ) - Wdiv2 ;
    val = (DOUBLE)(atileptr->orig_bottom - obottom) * aspFactor;
    newtile->b = ROUND( val ) - Hdiv2 ;
    val = (DOUBLE)(atileptr->orig_top - obottom) * aspFactor;
    newtile->t = ROUND( val ) - Hdiv2 ;

    newtile->lw = atileptr->lweight ;
    newtile->rw = atileptr->rweight ;
    newtile->bw = atileptr->bweight ;
    newtile->tw = atileptr->tweight ;
} /* end calculation of new tiles */

/* modify the vertices of the cell for the softpin moves */
vert = acellptr->vertices ;
xorig = vert->x_orig ;
xnew = vert->x_new ;
yorig = vert->y_orig ;
ynew = vert->y_new ;
for( i = 1; i <= acellptr->numsides; i++ ){
    val = (DOUBLE)(xorig[i] - oleft) / aspFactor ;
    xnew[i] = ROUND( val ) - Wdiv2 ;
    val = (DOUBLE)(yorig[i] - obottom) * aspFactor ;
    ynew[i] = ROUND( val ) - Hdiv2 ;
}


newbinpenal = binpenalG ;

newbinpenal += overlap( /* old_apos, new_apos */ ) ;

/* scale new penalty for feedback circuit */
newpenalty = (INT) ( lapFactorG * sqrt( (DOUBLE) newbinpenal ) ) ;

/* -------------- update the position of all pins --------------------- */
/* update the positions of all the hardpins first */
for( pin = acellptr->pinptr ; pin ; pin = pin->nextpin ) {
    
    if( pin->type == HARDPINTYPE ){
	val = (DOUBLE)(pin->txpos_orig[inst] - oleft) / aspFactor ;
	pin->txpos_new = ROUND( val ) - Wdiv2 ;
	val = (DOUBLE)(pin->typos_orig[inst] - obottom) * aspFactor ;
	pin->typos_new = ROUND( val ) - Hdiv2 ;
    }
}

/* now determine placement of pins using new vertices for the cell */
placepin( a, NEWVERTICES ) ;

cost = funccostG ;
clear_net_set() ; /* reset set to mark nets that have changed position */

cost += unet( acellptr->pinptr ) ;

newtimepenal = timingpenalG ;
newtimepenal += calc_incr_time( a ) ;
ASSERT( newtimepenal == dcalc_full_penalty(),"uaspect","Timing woes\n") ;

/* scale new timing penalty */
newtimepenalty = (INT) ( timeFactorG * (DOUBLE) newtimepenal ) ;

if( acceptt( funccostG + penaltyG + timingcostG - cost - newpenalty
    - newtimepenalty )){

    /* update current terminal positions from original positions */
    for( pin = acellptr->pinptr ; pin ; pin = pin->nextpin ) {
	pin->txpos = pin->txpos_new ;
	pin->typos = pin->typos_new ;
	if( pin->type != HARDPINTYPE ){
	    pin->txpos_orig[inst] = pin->txpos ;
	    pin->typos_orig[inst] = pin->typos ;
	}
    }

    upin_accept( acellptr->pinptr ) ;

    /* update the aspect ratio of softtile */
    for( atileptr = acellptr->tiles;atileptr;atileptr = atileptr->next ){
	val = (DOUBLE)(atileptr->orig_left - oleft) / aspFactor;
	atileptr->left = ROUND( val ) - Wdiv2 ;
	val = (DOUBLE)(atileptr->orig_right - oleft) / aspFactor;
	atileptr->right = ROUND( val ) - Wdiv2 ;
	val = (DOUBLE)(atileptr->orig_bottom - obottom) * aspFactor;
	atileptr->bottom = ROUND( val ) - Hdiv2 ;
	val = (DOUBLE)(atileptr->orig_top - obottom) * aspFactor;
	atileptr->top = ROUND( val ) - Hdiv2 ;
    } /* end calculation of new tiles */

    /* delay recalculation of bounding box views until needed */
    acellptr->boun_valid = FALSE ;

    /* update the vertices of the cell by swapping arrays */
    xnew = vert->x_new ;
    vert->x_new = vert->x ;
    vert->x = xnew ;
    ynew = vert->y_new ;
    vert->y_new = vert->y ;
    vert->y = ynew ;

    update_overlap( /* old_aposG */ ) ;
    update_time( a ) ;


    /* save the current aspect ratio */
    acellptr->aspect = newAspect ;

    funccostG = cost ;
    penaltyG = newpenalty ;
    binpenalG = newbinpenal ;
    timingcostG = newtimepenalty ;
    timingpenalG = newtimepenal ;
    return (ACCEPT) ;
} else {
    return (REJECT) ;
}
} /* end uaspect */



initialize_aspect_ratios()
{
    INT i ;                        /* counter */
    INT binX, binY ;               /* set initial bins */
    CELLBOXPTR cptr ;              /* current cell pointer */

    if( numsoftG > 0 || numstdcellG > 0 ){
	TG = 1.0e30;            /*** set to VERY HIGH temperature. ***/
	/* first determine number of softcell with uncommitted pins */
	for( i=1;i<=totalcellsG;i++ ){
	    cptr = cellarrayG[i] ;
	    if( cptr->celltype == SOFTCELLTYPE ||
	        cptr->celltype == STDCELLTYPE ){
		if( cptr->aspect < cptr->aspLB ||
		    cptr->aspect > cptr->aspUB ){
		    /* setup for uaspect */
		    new_apos0G->cell = old_apos0G->cell = i ;
		    old_apos0G->orient = new_apos0G->orient =
			cptr->orient ;
		    new_apos0G->xcenter = old_apos0G->xcenter = 
			cptr->xcenter ;
		    new_apos0G->ycenter = old_apos0G->ycenter =
			cptr->ycenter ;
		    /* calculate bin for move determination */
		    binX = SETBINX(cptr->xcenter) ; 
		    binY = SETBINY(cptr->ycenter) ;
		    newbptrG = binptrG[binX][binY] ;
		    newCellListG = newbptrG->cells ;
		    /* now set initial aspect ratio */
		    if(!(uaspect( i,(cptr->aspLB + cptr->aspUB) / 2.0))){ 
			M( WARNMSG,"initialize_aspect_ratio", 
			"Failed in aspect ratio initialization. Trying again\n" ) ;
			i-- ;
		    }
		}
	    }
	}
    }

} /* end initialize_aspect_ratios */
