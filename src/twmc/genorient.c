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
FILE:	    genorient.c                                       
DESCRIPTION:generate all eight orientation for tiles.
CONTENTS:   genorient( lowerBound, upperBound )
		INT lowerBound, upperBound ;
	    trans_bbox( ptr ) 
		CELLBOXPTR ptr ;
	    loadTermArray()
	    adjust_lr( orient, height, width,  left, right )
		INT orient, height, *left, *right ;
	    adjust_lr( orient, height, width,  bottom, top )
		INT orient, height, *bottom, *top ;
DATE:	    Feb 8, 1988 
REVISIONS:  Aug 17,1988 - add upper and lower bounds as parameters
		to genorient and split out terminal array code to
		loadTermArray function. Now genorient can be applied
		to a single cell.
	    Mar 20, 1989 - added adjust_lr and adjust_bt to consolidate
		code.
	    Mar 30, 1989 - changed tile datastructure.
	    Apr  2, 1989 - added trans_bbox to make code more general.
	    Sep 19, 1989 - trans_bbox now translate everything from
		the zero orientation.
	    Oct 14, 1990 - Fixed problem with vertices with
		initial orientation.
	    Oct 22, 1990 - Fixed initial orientation problem for
		equivalent.
	    Thu Jan 17 00:53:44 PST 1991 - now take care of translating
		 the new numpins field using translate_numpins.
	    Fri Jan 25 18:04:43 PST 1991 - made consistent with Ylib.
	    Wed Jan 30 14:13:36 EST 1991 - now left justify orientation
		rotations.
	    Sat Feb 23 00:21:10 EST 1991 - updated for new pad placement
		code.
	    Wed Mar 13 20:15:22 CST 1991 - now allow both orientations
		for a given pad side.
	    Thu Apr 18 01:32:23 EDT 1991 - now check for valid 
		orientations.  Also translate all bounding box since
		it may become valid later.
	    Sun May  5 14:27:53 EDT 1991 - fixed problem with orienting
		bounboxes.  Had used wrong translation function.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) genorient.c (Yale) version 3.13 5/5/91" ;
#endif

#include <custom.h>
#include <pads.h>
#include <yalecad/debug.h>
#include <yalecad/relpos.h>

/* genorient works on range [lowerBound..upperBound] inclusive */
genorient(lowerBound, upperBound)
INT lowerBound, upperBound ;
{

    INT i ;                         /* counter */
    INT cell ;                      /* current cell */
    INT x, y ;                      /* used to translate the pins */
    INT orient ;                    /* user specified orientation */
    INT saveorient ;                /* initial orientation */
    INT bhgt, blen ;                /* bounding box height, length */
    INT inverse ;                   /* inverse orientation */
    INT numinst ;                   /* number of cell instances */
    INT pt ;                        /* counter */
    INT *xvert ;                    /* xvertices */
    INT *yvert ;                    /* yvertices */
    INSTBOXPTR instptr ;            /* instance pointer */
    BOUNBOXPTR bounptr ;            /* bounding box pointer */
    PINBOXPTR  pin ;                /* translate the pins */
    TILEBOXPTR tile ;               /* translate the tiles */
    PADBOXPTR padptr ;              /* pad information */
    CELLBOXPTR ptr ;
    EQUIVPTR eqptr ;                /* current equivalent pin */


/* need to update for instances */

    for( cell = lowerBound ; cell <= upperBound ; cell++ ) {
	ptr = cellarrayG[cell] ;
	orient = ptr->orient ;

	if( orient < 0 ){
	    /* these are cells which have their initial orientation */
	    /* specified to nonzero-however vertices given for zero */
	    ptr->orient = - orient ;
	    orient = 0 ;
	}

	/* ************** INITIAL ORIENTATION NOT ZERO ************ */
	/* rotate to orientation 0 perform the inverse operation if */
	/* necessary */
	padptr = ptr->padptr ;
	if( orient ){
	    if( padptr ){

		switch (padptr->padside) {
		    case L:
			/* rotate 270 | -90 */
			inverse = 6 ;
			break;
		    case T:
			/* rotate180 */
			inverse = 3 ;
			break;
		    case R:
			/* rotate90 */
			inverse = 7 ;
			break;
		    case ALL:
			/* this is the case of no restriction */
			/* and we came from previous floorplanning */
			if( orient == 1 || orient == 3 ){
			    /* this is the top side */
			    inverse = orient ;
			    break ;
			} else if( orient == 4 || orient == 6 ){
			    /* this is the right side */
			    if( orient == 6 ){
				inverse = 7 ;
			    } else {
				inverse = 4 ;
			    }
			    break ;
			} else if( orient == 5 || orient == 7 ){
			    /* this is the left side */
			    if( orient == 6 ){
				inverse = 6 ;
			    } else {
				inverse = 5 ;
			    }
			    break ;
			} else if( orient == 2 ){
			    /* this is the bottom side */
			    inverse = 2 ;
			    break ;
			}
		    case B:
		    default:
			OUT1("\nNON-PADS or PAD w/o valid side rotatation\n");
			break;
		}
	    } else {
		inverse = Ytrans_inv_orient( orient ) ;
	    }
	    if( instptr = ptr->instptr ){
		numinst = instptr->numinstances ;
	    } else {
		numinst = 1 ;
	    }

	    /* first the bounding box */
	    bounptr = ptr->bounBox[0] ;
	    /* now init the translation routines using bounding box */
	    Ytrans_init( bounptr->l,bounptr->b,bounptr->r,bounptr->t,
		inverse ) ;

	    YtranslateC( &(bounptr->l), &(bounptr->b), 
		&(bounptr->r),&(bounptr->t), inverse ) ;

	    /* next the tiles */
	    for( tile = ptr->tiles; tile; tile = tile->next ){
		YtranslateC( &(tile->left), &(tile->bottom), 
		    &(tile->right),&(tile->top), inverse ) ;
		YtranslateC( &(tile->orig_left), &(tile->orig_bottom), 
		    &(tile->orig_right),&(tile->orig_top), inverse ) ;
		Ytranslatef( &(tile->lweight), &(tile->bweight), 
		    &(tile->rweight),&(tile->tweight), inverse ) ;
	    }
/* may need to update this for instances */
/* will need to check datastructures */
	    /* next the vertices */
	    xvert = ptr->vertices->x ;
	    yvert = ptr->vertices->y ;
	    for( pt = 1; pt <= ptr->numsides; pt++ ){
		REL_POST( inverse,
		    x, y,
		    xvert[pt], yvert[pt],
		    0, 0 ) ;
		ptr->vertices->x_orig[pt] = xvert[pt] = x ;
		ptr->vertices->y_orig[pt] = yvert[pt] = y ;

	    }
	    translate_numpins( ptr ) ;
	    /* next the fixed terminals */
	    for( pin = ptr->pinptr; pin; pin = pin->nextpin ){
		REL_POST( inverse, 
		    x, y,                               /* result */
		    pin->txpos,
		    pin->typos,                      /* cell relative */
		    0, 0 ) ;                           /* center */
		pin->txpos = x ; pin->typos = y ;
		pin->xpos = x + ptr->xcenter ;
		pin->ypos = y + ptr->ycenter ;
		/* do the number of instances */
		for( i = 0; i < numinst; i++ ){
		    REL_POST( inverse, 
			x, y,                            /* result */
			pin->txpos_orig[i],
			pin->typos_orig[i],           /* cell relative */
			0, 0 ) ;                         /* center */
		    pin->txpos_orig[i] = x ; pin->typos_orig[i] = y ;
		} /* end for loop on instances */

		/* now do equivalent pins */
		for( eqptr=pin->eqptr; eqptr; eqptr=eqptr->next ){
		    for( i = 0; i < numinst; i++ ){
			REL_POST( inverse, 
			    x, y,                               /* result */
			    eqptr->txpos[i],
			    eqptr->typos[i],          /* cell relative */
			    0, 0 ) ; /* center */
			eqptr->txpos[i] = x ;
			eqptr->typos[i] = y ;
		    }
		}
	    }
	}
	/* ************** INITIAL ORIENTATION NOT ZERO ************ */
	saveorient = ptr->orient ;
	ptr->orient = 0 ;

	trans_bbox( ptr ) ;
	ptr->orient = saveorient ;

	if( padptr ){
	    /* now set the height and with of the pads */
	    bounptr = ptr->bounBox[0] ;
	    padptr->length = bounptr->r - bounptr->l ;
	    padptr->height = bounptr->t - bounptr->b ;
	} else {
	    (VOID) check_valid_orient( ptr ) ;
	}
    } /* end loop on cells */

} /* end genorient */

/* regenorient works on range [lowerBound..upperBound] inclusive */
/* recalculates the bounding boxes and updates all the views */
/* works in an incremental manner */
regenorient(lowerBound, upperBound)
INT lowerBound, upperBound ;
{

    INT cell ;
    INT l, r, b, t ;
    INT xdev, ydev ;
    CELLBOXPTR ptr ;
    TILEBOXPTR tileptr ;
    BOUNBOXPTR bounptr ;

    for( cell = lowerBound ; cell <= upperBound ; cell++ ) {
	ptr = cellarrayG[cell] ;
	if( ptr->boun_valid ){
	    continue ;
	}
	/* recalculate the bounding box from the tiles */
	l = INT_MAX ;
	b = INT_MAX ;
	r = INT_MIN ;
	t = INT_MIN ;
	for( tileptr=ptr->tiles; tileptr; tileptr=tileptr->next ){
	    l = MIN( l, tileptr->left ) ;
	    b = MIN( b, tileptr->bottom ) ;
	    r = MAX( r, tileptr->right ) ;
	    t = MAX( t, tileptr->top ) ;
	}
	/* at this point we have found bounding box but it may */
	/* not be centered correctly due to rounding error */
	/* find largest deviation from center point and make */
	/* it symmetrical */
	l = ABS( l ) ;
	b = ABS( b ) ; 
	xdev = MAX( l, r ) ;/* find deviation in x */
	ydev = MAX( b, t ) ;/* find deviation in y */
	bounptr = ptr->bounBox[0] ;
	bounptr->l = - xdev ;
	bounptr->r =   xdev ;
	bounptr->b = - ydev ;
	bounptr->t =   ydev ;

	trans_bbox( ptr ) ;
    }

} /* end regenorient */


/* trans_bbox - translate bounding box into 8 views */
trans_bbox( ptr ) 
CELLBOXPTR ptr ;
{

INT orient ;
INT bhgt, blen ;
BOUNBOXPTR boun0, bounptr ;

    if( ptr->boun_valid ){
	/* correct no need to process */
	return ;
    }
    /* all orientations are generated relative to zero */

    /* ************************************************** */
    for( orient = 1 ; orient <= 7 ; orient++ ) {
	/* move bounding box information over to new orientation */
	boun0 = ptr->bounBox[0] ;
	bounptr = ptr->bounBox[orient] ;
	bounptr->r = boun0->r ;
	bounptr->l = boun0->l ;
	bounptr->t = boun0->t ;
	bounptr->b = boun0->b ;
	YtranslateT( &(bounptr->l), &(bounptr->b), 
	    &(bounptr->r),&(bounptr->t), orient ) ;
    }
    ptr->boun_valid = TRUE ;
} /* end trans_bbox() */ 

/* allocate space for and load termarray */
loadTermArray()
{
    INT net ;
    PINBOXPTR pinptr ;

    /* load termarray */
    for( net = 1 ; net <= numnetsG ; net++ ) {
	for( pinptr=netarrayG[net]->pins;pinptr; pinptr = pinptr->next ) {
	    termarrayG[ pinptr->pin ] = pinptr ;
	}
    }
} /* end loadTermArray */

translate_numpins( ptr ) 
CELLBOXPTR ptr ;
{
    INT pt ;                        /* point counter */
    INT minx ;                      /* looking for lowest y pt */
    INT miny ;                      /* looking for lowest y pt */
    INT side1 ;                     /* where the first side is */
    INT oldpt ;                     /* the place in the old array */
    INT *xvert ;                    /* xvertices */
    INT *yvert ;                    /* yvertices */
    FLOAT *tmp ;                    /* array of the numpins per side */


    xvert = ptr->vertices->x ;
    yvert = ptr->vertices->y ;
    /* find the bottom left so we have side 1 */
    miny = INT_MAX ;
    minx = INT_MAX ;
    tmp = (FLOAT *) Yvector_alloc( 1, ptr->numsides, sizeof(FLOAT) );
    for( pt = 1; pt <= ptr->numsides; pt++ ){
	tmp[pt] = ptr->vertices->numpins[pt] ;
	if( yvert[pt] < miny ){
	    miny = yvert[pt] ;
	    minx = xvert[pt] ;
	    side1 = pt ;
	} else if( yvert[pt] == miny ){
	    if( xvert[pt] < minx ){
		minx = xvert[pt] ;
		side1 = pt ;
	    }
	}
    }
    /* now reload the numpins in the zero view */
    for( pt = 1; pt <= ptr->numsides; pt++ ){
	oldpt = pt + side1 - 2 ;
	oldpt = oldpt % ptr->numsides ;
	/* now in the range 0..numsides-1 */
	oldpt++ ;
	/* now in the range 1..numsides */
	ptr->vertices->numpins[pt] = tmp[oldpt] ;
    }
} /* end translate_numpins */
