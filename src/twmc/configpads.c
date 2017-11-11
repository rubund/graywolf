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
	    Sat Feb 23 00:17:28 EST 1991 - added placepads algorithm.
	    Tue Mar 19 16:31:45 CST 1991 - added core expansion code.
	    Thu Apr 18 01:29:33 EDT 1991 - fixed problem with
		align_pads when no pads are on a given side.
	    Thu Aug 29 15:44:00 CDT 1991 - added overflow processing
		code.
----------------------------------------------------------------- */
#include "allheaders.h"

int perdimG[2];

/* ***************** STATIC VARIABLE DEFINITIONS ******************* */
int overflowS[5] ;          /* amount of overflow on each side */
int side_lengthS[5] ;       /* sum of pad lengths on a side */
int first_pad_on_sideS[5] ; /* index of first pad on side */
int last_pad_on_sideS[5] ;  /* index of last pad on side */
int pad_extraS ;            /* insure that sort works correctly */

void align_pads()
{
    int pad ;                      /* counter */
    int side ;                     /* current pad side */
    int last ;                     /* index of last pad on side */
    int first ;                    /* index of first pad on side */
    int count ;                    /* count number of overflow iterations */
    int length ;                   /* length of current pad */
    int needed ;                   /* extra space needed on each side */
    int numpads ;                  /* number of pads on this side */
    int cum_space ;                /* cum_space on this side */
    int padspacing ;               /* padspacing method for current side*/
    int extra_space ;              /* slack on a side */
    BOOL overflow_exists ;         /* if set, overflow still exists */
    BOOL retain_pad_groups ;       /* if set, padgroups will not be moved from side */
    PADBOXPTR padptr ;             /* current pad info */
    double left_edge ;             /* left edge of the placed pad */
    double extra_space_per_pad ;   /* how much left over per pad */

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
		extra_space_per_pad = (double) extra_space /
			              (double) (numpads+1) ;		
		left_edge = (double) coreG[side%2][MINI] ;
		for( pad = 1; pad <= numpads; pad++ ){
		    padptr = placearrayG[first + pad - 1] ;
		    /* calculate the placement of the left edge of the pad */
		    left_edge += extra_space_per_pad ;
		    /* now find the center of the current pad */
		    padptr->position =  ((int) left_edge ) +
			padptr->length / 2 ;
		    /* now set left edge for next pad */
		    left_edge += (double)
			(padptr->length + min_pad_spacingG) ;
		}
		break ;
	    case ABUT_PADS:
		extra_space_per_pad = (double) extra_space / 2.0 ;
		left_edge = (double) coreG[side%2][MINI] + extra_space_per_pad ;

		for( pad = 1; pad <= numpads; pad++ ){
		    padptr = placearrayG[first + pad - 1] ;
		    /* now find the center of the current pad */
		    padptr->position =  ((int) left_edge ) +
			padptr->length / 2 ;
		    /* now set left edge for next pad */
		    left_edge += (double)
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

BOOL check_overflow( BOOL retain_pad_groups )
{
    int i ;                        /* counter */
    int pad ;                      /* counter */
    int side ;                     /* current pad side */
    int cw_pad ;                   /* pad to move in clockwise direction */
    int ccw_pad ;                  /* pad to move in ccw direction */
    int length ;                   /* length of current pad */
    int clockwise ;                /* the side in the clockwise direction */
    int counterclockwise ;         /* the side in the ccw direction */
    int overflow_cw ;              /* amount of overflow when pad is moved cw */
    int overflow_ccw ;             /* amount of overflow when pad is moved ccw */
    int moveable_cw ;              /* size of pad to be moved cw */
    int moveable_ccw ;             /* size of pad to be moved ccw */
    int side_open[5] ;             /* whether side has been processed */
    int sort_overflow[5] ;         /* index for sorted overflow */
    int compare_overflow() ;       /* sort the sides for overflow */
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
    Yquicksort( &(sort_overflow[1]), 4, sizeof(int), compare_overflow );
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

void move_clockwise( int pad, int side, int clockwise_side, int moveable_cw )
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


void move_counterclockwise( int pad, int side, int counterclockwise, int moveable_ccw )
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

void update_pad_position( PADBOXPTR padptr, int current_side, int move_side )
{
    int dimension ;            /* the X or Y dimension */

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

void expand_core( int side )
{
    int i ;                        /* counter */
    int pad ;                      /* counter */
    int overflow_amount ;          /* amount to expand */
    int amount ;                   /* amount to add to each pad half of a side */
    int expansion ;                /* amount to expand a dimension */
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

int compare_overflow( int *side1, int *side2 )
{
    /* sort largest to smallest */
    return( overflowS[*side2] - overflowS[*side1] ) ;
} /* end compare_overflow */

int compare_placearray( PADBOXPTR *padptr1, PADBOXPTR *padptr2 )
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

void resort_place_array()
{
    Yquicksort( &(placearrayG[1]), numpadsG, sizeof(PADBOXPTR), compare_placearray );
} /* end resort_place_array */
/* ***************************************************************** */

/* set the lo_pos and hi_pos fields for the pads */
void update_pad_groups()
{

    int i ;                   /* pad counter */
    double lobound ;          /* lower bound on position */
    double hibound ;          /* upper bound on position */
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

	} else if( pad->padtype == PADCELLTYPE && pad->hierarchy == NONE ) {
	    /* the case of a pad that is not in a padgroup */
	    calc_constraints( pad, pad->padside, &lobound, &hibound,
	        &spacing_restricted, &(pad->lo_pos), &(pad->hi_pos) ) ;
	}
    } 

} /* end update_pad_groups */

/* this will set the constaints for pad groups and children of them */
void child_constraints( PADBOXPTR pad, int side, double lb, double ub, BOOL spacing_restricted )
{
    int i ;            /* pad counter */
    int howmany ;      /* number of children */
    PADBOXPTR child ;  /* current child */

    calc_constraints( pad, side, &lb, &ub, &spacing_restricted,
	&(pad->lo_pos), &(pad->hi_pos) );

    if( pad->padtype != PADCELLTYPE ){
	howmany = pad->children[HOWMANY] ;
	for( i = 1 ;i <= howmany ; i++ ){
	    child = padarrayG[ pad->children[i] ] ;
	    child_constraints( child, side, lb, ub,spacing_restricted ) ;
	}
    }
} /* end child_constraints */
/* ***************************************************************** */

void calc_constraints( PADBOXPTR pad, int side, double *lb, double *ub, BOOL *spacing_restricted, int *lowpos, int *uppos)
{
    double lowbound, hibound ;

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
	*lowpos = (int) ( lowbound * (double)perdimG[side%2] ) ;
	*lowpos += coreG[side%2][MINI] ;
	*uppos = (int) ( hibound * (double)perdimG[side%2] ) ;
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

void place_variable( int first, int numpads, int side )
{
    int pad ;            /* counter */
    int left_edge ;      /* current left edge of pad */
    int right_edge ;     /* current right edge of pad */
    int half_length ;    /* half the span of a pad */
    int valid_left_edge ;/* this position is open for the pad leftedge*/
    int valid_right_edge;/* this position is open for the pad rightedge*/
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

    D( "placepads/after_left_edge",
	print_pads( "pads after left_edge\n", placearrayG, numpadsG ) ;
    ) ;

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

void dimension_pads()
{
    int i ;           /* pad counter */
    PADBOXPTR pad ;   /* current pad */
    PINBOXPTR pin ;   /* current pin */
    CELLBOXPTR cptr ; /* current cell */

    for( i = 1 ; i <= numpadsG; i++ ) {
	pad = padarrayG[i];
	cptr = cellarrayG[pad->cellnum] ;
	switch( pad->padside) {
	    case L:
		cptr->xcenter = coreG[X][MINI] - pad->height / 2 ;
		cptr->ycenter = pad->position ;
		break;
	    case T:
		cptr->xcenter = pad->position ;
		cptr->ycenter = coreG[Y][MAXI] + pad->height / 2 ;
		break;
	    case R:
		cptr->xcenter = coreG[X][MAXI] + pad->height / 2 ;
		cptr->ycenter = pad->position ;
		break;
	    case B:
		cptr->xcenter = pad->position ;
		cptr->ycenter = coreG[Y][MINI] - pad->height / 2 ;
		break;
	} /* end switch on side */
	/* now update the pins */
	for( pin = cptr->pinptr; pin ; pin = pin->nextpin ){
	    REL_POST( cptr->orient, 
		pin->xpos, pin->ypos,                /* result */
		pin->txpos,
		pin->typos,                         /* cell relative */
		cptr->xcenter, cptr->ycenter ) ;    /* cell center */
	}
    }

} /* dimension_pads */
/* ***************************************************************** */

void orient_pads()
{
    int i ;                         /* counter */
    PADBOXPTR pad ;                 /* current pad info */
    CELLBOXPTR cptr ;               /* current cell */

     for( i = 1; i <= numpadsG; i++ ){
	 pad = placearrayG[i];
	 cptr = cellarrayG[pad->cellnum] ;
	 switch( pad->padside ){
	 case L:
	    /* rotate 270 | -90 */
	    cptr->orient = 7 ;
	    cptr->ycenter = cptr->xcenter;
	    break;
	 case T:
	    /* rotate180 */
	    cptr->orient = 3 ;
	    break;
	 case R:
	    /* rotate90 */
	    cptr->orient = 6 ;
	    cptr->ycenter = cptr->xcenter;
	    break;
	 case B:
	    cptr->orient = 0 ;
	    break;
	 default:
	    printf("\nNON-PADS or PAD w/o side not rotated ");
	    break;
	 }
     }
} /* end orient_pads */
/* ***************************************************************** */
