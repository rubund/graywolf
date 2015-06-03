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
FILE:	    configpads.c                                       
DESCRIPTION:Configure pads
CONTENTS:
DATE:	    Jan 29, 1989 - added heading.
REVISIONS:  Jan 29, 1989 - removed message about pad limited designs.
	    Mar 30, 1989 - changed tile datastructure.
	    Apr 30, 1989 - fixed problem with padgroups.  Rewrote major
		sections of the algorithm.
	    Thu Feb 14 02:41:45 EST 1991 - new algorithm for placing
		pads.  Still need to do VARIABLE_PADS.
	    Fri Feb 15 15:13:49 EST 1991 - added VARIABLE PAD code.
	    Sun Feb 17 21:10:35 EST 1991 - added min_pad_spacing.
	    Thu Mar  7 01:52:16 EST 1991 - now perform pad rotations
		correctly.
	    Tue Mar 19 16:01:40 CST 1991 - added core expansion code.
	    Thu Apr 18 01:45:56 EDT 1991 - fixed problem with align_pads
	    Thu Aug 29 15:44:00 CDT 1991 - added overflow processing
		code.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) configpads.c version 4.12 11/7/91" ;
#endif

#include <standard.h>
#include <pads.h>
#include <parser.h>
#include <yalecad/relpos.h>
#include <yalecad/message.h>

/* ***************** STATIC FUNCTION DEFINITIONS ******************* */
static BOOL check_overflow( P1(BOOL retain_pad_groups) ) ;
static move_clockwise( P4(INT pad, INT side, INT cw_side, INT moveable_cw) ) ;
static move_counterclockwise( P4(INT pad,INT side,INT ccw_side,INT m_ccw ) ) ;
static update_pad_position( P3(PADBOXPTR pad,INT current_side, INT move_side) );
static expand_core( P1(INT side) ) ;
static INT compare_overflow( P2(INT *side1, INT *side2) ) ;
static update_pad_groups( P1(void) ) ;
static resort_place_array( P1(void) ) ;
static child_constraints(P5(PADBOXPTR pad,INT side,DOUBLE lb,DOUBLE ub,BOOL s));
static place_variable( P3(INT first,INT numpads,INT side) ) ;

/* ***************** STATIC VARIABLE DEFINITIONS ******************* */
static INT overflowS[5] ;          /* amount of overflow on each side */
static INT side_lengthS[5] ;       /* sum of pad lengths on a side */
static INT first_pad_on_sideS[5] ; /* index of first pad on side */
static INT last_pad_on_sideS[5] ;  /* index of last pad on side */
static INT pad_extraS ;            /* insure that sort works correctly */

align_pads()
{
    INT pad ;                      /* counter */
    INT side ;                     /* current pad side */
    INT last ;                     /* index of last pad on side */
    INT first ;                    /* index of first pad on side */
    INT count ;                    /* count number of overflow iterations */
    INT length ;                   /* length of current pad */
    INT needed ;                   /* extra space needed on each side */
    INT numpads ;                  /* number of pads on this side */
    INT cum_space ;                /* cum_space on this side */
    INT padspacing ;               /* padspacing method for current side*/
    INT extra_space ;              /* slack on a side */
    BOOL overflow_exists ;         /* if set, overflow still exists */
    BOOL retain_pad_groups ;       /* if set, padgroups will not be moved from side */
    PADBOXPTR padptr ;             /* current pad info */
    DOUBLE left_edge ;             /* left edge of the placed pad */
    DOUBLE extra_space_per_pad ;   /* how much left over per pad */

    count = 0 ;
    retain_pad_groups = TRUE ;
    pad_extraS = 100000 ;
    do {
	overflow_exists = check_overflow( retain_pad_groups ) ;
	if( overflow_exists ){
	    resort_place_array() ;
	}
	if( ++count >= 3 ){
	    retain_pad_groups = FALSE ;
	}
    } while( overflow_exists ) ;

    update_pad_groups() ;

    for( side = 1 ; side <= 4 ; side++ ) {
	if(!(first_pad_on_sideS[side])){
	    /* there are no pads on this side */
	    continue ;
	}
	extra_space = perdimG[ side%2 ] - side_lengthS[side] ;
	/* check to see if we have overflow */
	if( extra_space <= 0 ){
	    /* we must perform abut padspacing on this side */
	    padspacing = ABUT_PADS ;
	} else {
	    /* do what the user requested */
	    padspacing = padspacingG ;
	}

	first = first_pad_on_sideS[side] ;
	last = last_pad_on_sideS[side] ;
	numpads = last - first + 1 ;

	switch( padspacing ){
	    case UNIFORM_PADS:
		extra_space_per_pad = (DOUBLE) extra_space /
			              (DOUBLE) (numpads+1) ;		
		left_edge = (DOUBLE) coreG[side%2][MINI] ;
		for( pad = 1; pad <= numpads; pad++ ){
		    padptr = placearrayG[first + pad - 1] ;
		    /* calculate the placement of the left edge of the pad */
		    left_edge += extra_space_per_pad ;
		    /* now find the center of the current pad */
		    padptr->position =  ((INT) left_edge ) +
			padptr->length / 2 ;
		    /* now set left edge for next pad */
		    left_edge += (DOUBLE)
			(padptr->length + min_pad_spacingG) ;
		}
		break ;
	    case ABUT_PADS:
		extra_space_per_pad = (DOUBLE) extra_space / 2.0 ;
		left_edge = (DOUBLE) coreG[side%2][MINI] + extra_space_per_pad ;

		for( pad = 1; pad <= numpads; pad++ ){
		    padptr = placearrayG[first + pad - 1] ;
		    /* now find the center of the current pad */
		    padptr->position =  ((INT) left_edge ) +
			padptr->length / 2 ;
		    /* now set left edge for next pad */
		    left_edge += (DOUBLE)
			(padptr->length + min_pad_spacingG) ;
		}
		break ;
	    case VARIABLE_PADS:
		/* we now we can fit it on this side */
		place_variable( first, numpads, side ) ;
		break ;
	    case EXACT_PADS:
		break ;

	} /* end switch */

    } /* end loop on sides */

} /* end align_pads */
/* ***************************************************************** */

static BOOL check_overflow( retain_pad_groups )
BOOL retain_pad_groups ;
{
    INT i ;                        /* counter */
    INT pad ;                      /* counter */
    INT side ;                     /* current pad side */
    INT cw_pad ;                   /* pad to move in clockwise direction */
    INT ccw_pad ;                  /* pad to move in ccw direction */
    INT length ;                   /* length of current pad */
    INT clockwise ;                /* the side in the clockwise direction */
    INT counterclockwise ;         /* the side in the ccw direction */
    INT overflow_cw ;              /* amount of overflow when pad is moved cw */
    INT overflow_ccw ;             /* amount of overflow when pad is moved ccw */
    INT moveable_cw ;              /* size of pad to be moved cw */
    INT moveable_ccw ;             /* size of pad to be moved ccw */
    INT side_open[5] ;             /* whether side has been processed */
    INT sort_overflow[5] ;         /* index for sorted overflow */
    INT compare_overflow() ;       /* sort the sides for overflow */
    PADBOXPTR padptr ;             /* current pad info */

    /* first accummulate the sum of all the pads on a given side */
    /* also find the first and last pad on a given side */
    for( side = 1 ; side <= 4 ; side++ ) {
	overflowS[side] = 0 ;
	side_lengthS[side] = 0 ;
	first_pad_on_sideS[side] = 0 ;
	last_pad_on_sideS[side] = -1 ;
    }
    for( pad = 1 ; pad <= numpadsG ; pad++ ) {
	padptr = placearrayG[pad] ;
	side = padptr->padside ;
	length = padptr->length ;
	/* this will add one too many min_pad_spacing */
	/* subtract it below */
	side_lengthS[side] += length + min_pad_spacingG ;
	if(!(first_pad_on_sideS[side])){
	    first_pad_on_sideS[side] = pad ;
	}
	last_pad_on_sideS[side] = pad ;
    }

    /* check for overflow on the sides */
    for( side = 1 ; side <= 4 ; side++ ) {
	/* adjust extra min_pad_spacing */
	if(first_pad_on_sideS[side]){
	    /* there are pads on this side */
	    side_lengthS[side] -= min_pad_spacingG ;
	}
	overflowS[side] = side_lengthS[side] - perdimG[ side%2 ] ;
    }
    /* check to see if we have overflow */
    if( overflowS[L] <= 0 && overflowS[R] <= 0 && 
	overflowS[B] <= 0 && overflowS[T] <= 0 ){
	/* there is no overflow */
	return( FALSE ) ;
    }

    /* overflow exists so resolve the conflict. */
    /* first sort the overflow */
    for( i = 1 ; i <= 4; i++ ){
	/* initialize sort_overflow array */
	sort_overflow[i] = i ;
	side_open[i] = TRUE ;
    }
    Yquicksort( &(sort_overflow[1]), 4, sizeof(INT), compare_overflow );
    for( i = 1; i <= 4; i++ ){
	side = sort_overflow[i] ;
	side_open[side] = FALSE ;
	if( overflowS[side] <= 0 ){
	    continue ;
	}
	/* find clockwise side */
	clockwise = side + 1 ;
	if( clockwise > 4 ) clockwise = 1 ; /* wrap around */
	/* find counter clockwise side */
	counterclockwise = side - 1 ;
	if( counterclockwise < 1 ) counterclockwise = 4 ; /* wrap around */

	do {
	    /* look for the last valid pad on side to move clockwise */
	    /* make sure side is still open */
	    cw_pad = 0 ;
	    if( side_open[clockwise] ){
		for( pad = last_pad_on_sideS[side];pad >= first_pad_on_sideS[side];
		    pad-- ){
		    padptr = placearrayG[pad] ;
		    if( padptr->padside != side ){
			/* this means we already moved this pad from this side */
			continue ;
		    }
		    if( retain_pad_groups && padptr->hierarchy != NONE ){
			/* if the retain_pad_group switch is on, we ignore any */
			/* pad in a padgroup */
			continue ;
		    }
		    if( padptr->valid_side[ALL] || padptr->valid_side[clockwise] ){
			cw_pad = pad ;
			moveable_cw = padptr->length + min_pad_spacingG ;
			break ;
		    }
		} /* end pad = last_pad_on_side... */
	    } /* end side_open[clockwise... */

	    /* look for the first valid pad on side to move counterclockwise */
	    /* make sure side is still open */
	    ccw_pad = 0 ;
	    if( side_open[counterclockwise] ){
		for( pad = first_pad_on_sideS[side]; pad <= last_pad_on_sideS[side];
		    pad++ ){
		    padptr = placearrayG[pad] ;
		    if( padptr->padside != side ){
			/* this means we already moved this pad from this side */
			continue ;
		    }
		    if( retain_pad_groups && padptr->hierarchy != NONE ){
			/* if the retain_pad_group switch is on, we ignore any */
			/* pad in a padgroup */
			continue ;
		    }
		    if( padptr->valid_side[ALL] || padptr->valid_side[counterclockwise] ){
			ccw_pad = pad ;
			moveable_ccw = padptr->length + min_pad_spacingG ;
			break ;
		    }
		} /* end pad = first_pad_on_side... */
	    } /* end side_open[counterclockwise... */

	    if( cw_pad && ccw_pad ){
		/* this is the case where there are two valid pads which can be moved */
		/* pick the pad && side which has smaller overflow */
		overflow_cw = overflowS[clockwise] + moveable_cw ; ;
		overflow_ccw = overflowS[counterclockwise] + moveable_ccw ; ;
		if( overflow_cw <= overflow_ccw ){
		    move_clockwise( cw_pad, side, clockwise, moveable_cw ) ;
		} else {
		    move_counterclockwise( ccw_pad, side, counterclockwise, moveable_ccw ) ;
		}
	    } else if( cw_pad ){
		move_clockwise( cw_pad, side, clockwise, moveable_cw ) ;
	    } else if( ccw_pad ){
		move_counterclockwise( ccw_pad, side, counterclockwise, 
		    moveable_ccw ) ;
	    } else {
		/* no more pads to move, we need to expand core */
		expand_core( side ) ;
		return( TRUE ) ;
	    }
	} while( overflowS[side] > 0 ) ;

    } /* end for( i = 1; i <= 4; i++ ) ... */

    /* if we get here, we had some overflow */
    return( TRUE ) ;

} /* end check_overflow */

static move_clockwise( pad, side, clockwise_side, moveable_cw )
INT pad, side, clockwise_side, moveable_cw ; 
{ 
    PADBOXPTR padptr ;             /* current pad info */

    /* move pad */ 
    padptr = placearrayG[pad] ; 
    padptr->padside = clockwise_side ; 
    /* determine position */
    update_pad_position( padptr, side, clockwise_side ) ;
    /* reset the overflows */ 
    overflowS[side] -= moveable_cw ; 
    overflowS[clockwise_side] += moveable_cw ;

} /* end move_clockwise */


static move_counterclockwise( pad, side, counterclockwise, moveable_ccw )
INT pad, side, counterclockwise, moveable_ccw ; 
{ 
    PADBOXPTR padptr ;             /* current pad info */

    /* move pad */ 
    padptr = placearrayG[pad] ;
    padptr->padside = counterclockwise ;
    update_pad_position( padptr, side, counterclockwise ) ;
    /* determine position */
    /* reset the overflows */ 
    overflowS[side] -= moveable_ccw ; 
    overflowS[counterclockwise] += moveable_ccw ;

} /* end move_counterclockwise */

static update_pad_position( padptr, current_side, move_side )
PADBOXPTR padptr ;             /* current pad info */
INT current_side ;
INT move_side ;
{
    INT dimension ;            /* the X or Y dimension */

    dimension = move_side % 2 ;
    switch( current_side ){
    case L:
	padptr->position = coreG[dimension][MINI] - ++pad_extraS; 
	break ;
    case T:
	padptr->position = coreG[dimension][MAXI] + ++pad_extraS ;
	break ;
    case R:
	padptr->position = coreG[dimension][MAXI] + ++pad_extraS ;
	break ;
    case B:
	padptr->position = coreG[dimension][MINI] - ++pad_extraS; 
	break ;
    } /* end switch side ... */
} /* end update_pad_position */

static expand_core( side )
INT side ;
{
    INT i ;                        /* counter */
    INT pad ;                      /* counter */
    INT overflow_amount ;          /* amount to expand */
    INT amount ;                   /* amount to add to each pad half of a side */
    INT expansion ;                /* amount to expand a dimension */
    PADBOXPTR padptr ;             /* current pad info */

    overflow_amount = 0 ;
    /* determine how much to expand by */
    /* normally we assume that all sides can help reduce overflow */
    /* if we find that the overflow_amount < 0 or overflow_amount is only */
    /* 25% of the overflow on a side, it means that some sides */
    /* have non moveable pads.  We need to make the overflow amount the given side */
    for( i = 1; i <= 4 ; i++ ){
	overflow_amount += overflowS[i] ;
    }
    if( overflow_amount < 0 || overflow_amount < 0.25 * overflowS[side] ){
	overflow_amount = overflowS[side] ;
    }

    /* now divide the amount equally in both dimensions */
    expansion = overflow_amount / 2 ;
    if( expansion <= 0 ) expansion = 1 ;
    amount = expansion / 2 ;
    coreG[X][MINI] -= amount ;
    coreG[X][MAXI] += (expansion - amount ) ;
    coreG[Y][MINI] -= amount ;
    coreG[Y][MAXI] += (expansion - amount ) ;
    /* expand in X and Y directions */
    /* we to search all the pads since they may not be in order */
    for( pad = 1 ; pad <= numpadsG ; pad++ ) {
	padptr = placearrayG[pad] ;
	side = padptr->padside ;
	padptr->position += amount ;
    }
    /* now reset the perimeters */
    perdimG[X] = coreG[X][MAXI] - coreG[X][MINI] ;
    perdimG[Y] = coreG[Y][MAXI] - coreG[Y][MINI] ;
} /* end expand_core */

static INT compare_overflow( side1, side2 )
INT *side1, *side2 ;
{
    /* sort largest to smallest */
    return( overflowS[*side2] - overflowS[*side1] ) ;
} /* end compare_overflow */

static INT compare_placearray( padptr1, padptr2 )
PADBOXPTR *padptr1, *padptr2 ;
{
    PADBOXPTR pad1, pad2;

    pad1 = *padptr1 ;
    pad2 = *padptr2 ;

    if( pad1->padside != pad2->padside) {
	return( pad1->padside - pad2->padside ) ;
    }
    if( pad1->position < pad2->position ){
	return( -1 ) ;
    } else if( pad1->position > pad2->position ){
	return( 1 ) ;
    } else {
	return( 0 ) ;
    }
} /* end compare_placearray */

static resort_place_array()
{
    Yquicksort( &(placearrayG[1]), numpadsG, sizeof(PADBOXPTR), compare_placearray );
} /* end resort_place_array */
/* ***************************************************************** */

/* set the lo_pos and hi_pos fields for the pads */
static update_pad_groups()
{

    INT i ;                   /* pad counter */
    DOUBLE lobound ;          /* lower bound on position */
    DOUBLE hibound ;          /* upper bound on position */
    BOOL spacing_restricted ; /* whether spacing is restricted */
    PADBOXPTR pad ;           /* current pad */

    if( padspacingG != VARIABLE_PADS ){
	return ;
    }

    /* now update the lo_pos and hi_pos fields of the pad */
    for( i = 1 ; i <= totalpadsG; i++ ) {
	pad = padarrayG[i];
	/* make copy of these we don't want to change roots copy */
	lobound = pad->lowerbound ;
	hibound  = pad->upperbound ;
	spacing_restricted = pad->fixed ;
	if( pad->padtype == PADGROUPTYPE && pad->hierarchy == ROOT  ){
	    child_constraints( pad, pad->padside, lobound, hibound,
		spacing_restricted ) ;

	} else if( pad->padtype == PADTYPE && pad->hierarchy == NONE ) {
	    /* the case of a pad that is not in a padgroup */
	    calc_constraints( pad, pad->padside, &lobound, &hibound,
	        &spacing_restricted, &(pad->lo_pos), &(pad->hi_pos) ) ;
	}
    } 

} /* end update_pad_groups */

/* this will set the constaints for pad groups and children of them */
static child_constraints( pad, side, lb, ub, spacing_restricted )
PADBOXPTR pad ;
INT side ;
DOUBLE lb, ub ;
BOOL spacing_restricted ;
{
    INT i ;            /* pad counter */
    INT howmany ;      /* number of children */
    PADBOXPTR child ;  /* current child */

    calc_constraints( pad, side, &lb, &ub, &spacing_restricted,
	&(pad->lo_pos), &(pad->hi_pos) );

    if( pad->padtype != PADTYPE ){
	howmany = pad->children[HOWMANY] ;
	for( i = 1 ;i <= howmany ; i++ ){
	    child = padarrayG[ pad->children[i] ] ;
	    child_constraints( child, side, lb, ub,spacing_restricted ) ;
	}
    }
} /* end child_constraints */
/* ***************************************************************** */

calc_constraints( pad, side, lb, ub, spacing_restricted,lowpos, uppos )
PADBOXPTR pad ;
INT side ;
DOUBLE *lb, *ub ;
BOOL *spacing_restricted ;
INT *lowpos, *uppos ;
{
    DOUBLE lowbound, hibound ;

    /* determine spacing restrictions */
    if( *spacing_restricted ){
	/* this is the case that the spacing has been restricted */
	if( pad->fixed ){
	    /* if the padgroup bounds have been fixed, */
	    /* force position to be within bound */
	    /* assume we are ok and then correct it */
	    lowbound = pad->lowerbound ;
	    if( lowbound < *lb ){
		lowbound = *lb ;
	    }
	    if( lowbound > *ub ){
		lowbound = *ub ;
	    }
	    hibound = pad->upperbound ;
	    if( hibound < *lb ){
		hibound = *lb ;
	    }
	    if( hibound > *ub ){
		hibound = *ub ;
	    }
	} else {
	    /* this pad is not fixed use the given ub and lb */
	    lowbound = *lb ; hibound = *ub ;
	}
    } else {
	if( pad->fixed ){
	    /* the padgroup bounds have not been fixed */
	    /* just take the pad's restricted position */
	    lowbound = pad->lowerbound;
	    hibound = pad->upperbound;
	    *spacing_restricted = TRUE ;
	}
    }
    if( *spacing_restricted ){
	*lowpos = (INT) ( lowbound * (DOUBLE)perdimG[side%2] ) ;
	*lowpos += coreG[side%2][MINI] ;
	*uppos = (INT) ( hibound * (DOUBLE)perdimG[side%2] ) ;
	*uppos += coreG[side%2][MINI] ;
    } else {
	*lowpos = -1 ;
	*uppos = PINFINITY ;
    }
    /* **** END spacing restriction calculations *** */
    /* return low bound */
    *lb = lowbound ;
    *ub = hibound ;

} /* end calc_constraints */
/* ***************************************************************** */

static place_variable( first, numpads, side )
INT first, numpads, side ;
{
    INT pad ;            /* counter */
    INT left_edge ;      /* current left edge of pad */
    INT right_edge ;     /* current right edge of pad */
    INT half_length ;    /* half the span of a pad */
    INT valid_left_edge ;/* this position is open for the pad leftedge*/
    INT valid_right_edge;/* this position is open for the pad rightedge*/
    PADBOXPTR padptr ;   /* current pad info */

    valid_left_edge = 0 ;
    for( pad = 0; pad < numpads; pad++ ){
	padptr = placearrayG[first + pad ] ;
	/* now find the left edge of the current pad */
	half_length = padptr->length / 2 ;
	left_edge = padptr->position - half_length ;
	valid_left_edge = MAX( valid_left_edge, padptr->lo_pos ) ;
	if( left_edge < valid_left_edge ){
	    /* need to remove overlap */
	    /* so left edge of pad becomes valid_left_edge */
	    left_edge = valid_left_edge ;
	    /* calculate new padptr position */
	    padptr->position = valid_left_edge + half_length ;
	}
	/* now calculate the new valid left edge */
	valid_left_edge = left_edge + padptr->length + min_pad_spacingG ;
    }

    valid_right_edge = coreG[side%2][MAXI] ;
    /* now condense pads if they exceed the length of the side */
    for( pad = numpads-1; pad >= 0; pad-- ){
	padptr = placearrayG[first + pad] ;
	/* now find the left edge of the current pad */
	half_length = padptr->length / 2 ;
	half_length = padptr->length - half_length ;
	right_edge = padptr->position + half_length ;
	valid_right_edge = MIN( valid_right_edge, padptr->hi_pos ) ;
	if( right_edge > valid_right_edge ){
	    /* need to remove overlap */
	    /* so left edge of pad becomes valid_left_edge */
	    right_edge = valid_right_edge ;
	    /* calculate new padptr position */
	    padptr->position = valid_right_edge - half_length ;
	} else {
	    break ;
	}
	/* now calculate the new valid right edge */
	valid_right_edge = right_edge - padptr->length - min_pad_spacingG ;
    }

} /* end place_variable */
/* ***************************************************************** */

dimension_pads()
{
    INT i ;           /* pad counter */
    PADBOXPTR pad ;   /* current pad */
    PINBOXPTR pin ;   /* current pin */
    CBOXPTR cptr ;    /* current cell */

    for( i = 1 ; i <= numpadsG; i++ ) {
	pad = padarrayG[i];
	cptr = carrayG[ pad->cellnum ] ;
	switch( pad->padside) {
	    case L:
		cptr->cxcenter = coreG[X][MINI] - pad->height / 2 ;
		cptr->cycenter = pad->position ;
		break;
	    case T:
		cptr->cxcenter = pad->position ;
		cptr->cycenter = coreG[Y][MAXI] + pad->height / 2 ;
		break;
	    case R:
		cptr->cxcenter = coreG[X][MAXI] + pad->height / 2 ;
		cptr->cycenter = pad->position ;
		break;
	    case B:
		cptr->cxcenter = pad->position ;
		cptr->cycenter = coreG[Y][MINI] - pad->height / 2 ;
		break;
	} /* end switch on side */

	/* now update the pins */
	for( pin = cptr->pins; pin ; pin = pin->nextpin ){
	    REL_POST( cptr->corient, 
		pin->txpos[1], pin->typos[1],           /* result */
		pin->txpos[0],
		pin->typos[0],                        /* cell relative */
		0, 0 ) ;                              /* cell relative */
	    /* global positions */
	    pin->xpos = pin->txpos[1] + cptr->cxcenter ;
	    pin->ypos = pin->typos[1] + cptr->cycenter ;
	}
    }

} /* dimension_pads */
/* ***************************************************************** */


orient_pads()
{
    INT i ;                         /* counter */
    PADBOXPTR pad ;                 /* current pad info */
    CBOXPTR cptr ;                  /* current cell */

     for( i = 1; i <= numpadsG; i++ ){
	 pad = placearrayG[i];
	 cptr = carrayG[ pad->cellnum ] ;
	 switch( pad->padside) {
	 case L:
	    /* rotate 270 | -90 */
	    cptr->corient = (SHORT) 7 ;
	    cptr->cycenter = cptr->cxcenter;
	    break;
	 case T:
	    /* rotate180 */
	    cptr->corient = (SHORT) 3 ;
	    break;
	 case R:
	    /* rotate90 */
	    cptr->corient = (SHORT) 6 ;
	    cptr->cycenter = cptr->cxcenter;
	    break;
	 case B:
	    cptr->corient = (SHORT) 0 ;
	    break;
	 default:
	    OUT1("\nNON-PADS or PAD w/o side not rotated ");
	    break;
	 }
     }
} /* end orient_pads */
/* ***************************************************************** */
