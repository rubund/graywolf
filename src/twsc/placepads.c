/*
 *   Copyright (C) 1988-1992 Yale University
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
FILE:	    placepads.c
DESCRIPTION:This file contains the place pads routines.
CONTENTS:   
	    placepads()
	    find_core()
	    setVirtualCore( flag )
		BOOL flag ;
	    find_core_boundry( left, right, bottom, top )
		INT *left, *right, *bottom, *top ;
	    get_global_pos( cell, l, b, r, t ) 
		INT cell, *l, *r, *b, *t ;
DATE:	    Aug 12, 1988 
REVISIONS:  Oct 24, 1988 - added virtual core switch to control pad
		placement.
	    Jan 17, 1988 - add find_core_boundary for channel graph
		generation code - outgeo.c .
	    Jan 20, 1989 - fixed problem when pad has no pin connected
		to it.  Made sure softcells are using correct fields
		and coordinates are translated.
	    Feb. 15, 1989 - added get_global_pos so that amount of 
		duplicated code is reduced.
	    Apr  30, 1989 - added bound to padcenter for non-virtual
		core placement.  Modified putChildren to place
		padgroups with sidespace restriction.
	    Oct 20, 1989 - Now pads are output with global routing
		density.
	    Feb  7, 1990 - Updated placepad core coordinates to
		reflect new routing scheme.
	    Sun Jan 20 21:47:52 PST 1991 - ported to AIX.
	    Thu Feb 14 02:44:36 EST 1991 - new pad placement algorithm.
	    Fri Feb 15 15:19:38 EST 1991 - modified for VARIABLE_PADS
		case.
	    Sun Feb 17 21:07:39 EST 1991 - renamed globals.
	    Wed Jun  5 16:39:11 CDT 1991 - added retain_side option
		and now pad pins correctly.
	    Fri Sep  6 15:16:27 CDT 1991 - moved retain_side option
		so it works when no pad groups exist.
	    Thu Sep 19 16:33:58 EDT 1991 - fixed problem with sidespace
		options when only a fraction of the pins to pads have
		connections to the core.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) placepads.c version 4.15 5/12/92" ;
#endif

#define PAD_VARS

#include <standard.h>
#include <pads.h>
#include <config.h>
#include <parser.h>
#include <yalecad/debug.h>

/* global references */
extern INT **pairArrayG ;

/* ***************** STATIC FUNCTION DEFINITIONS ******************* */
static find_optimum_locations( P1(void) ) ;
static place_pad( P2(PADBOXPTR pad,INT bestside ) ) ;
static place_children( P5(PADBOXPTR pad,INT side,DOUBLE lb,DOUBLE ub,BOOL sr) ) ;
static INT find_cost_for_a_side(P5(PADBOXPTR pad,INT side,DOUBLE lb,DOUBLE ub,
   BOOL spacing_restricted ) ) ;
static find_core( P1(void) ) ;

/* ***************** STATIC VARIABLE DEFINITIONS ******************* */
static BOOL virtualCoreS = FALSE ;
static INT sumposS ; /* sum of the modified opt. pin pos. of pad pins */
static INT sumtieS ; /* sum of all the opt. pin pos. of pad pins */
static INT pin_countS ; /* number of pins found with valid connections */
static BOOL retain_sideS = FALSE ; /* during global routing side is set */

/*-------------------------------------------------------------------
The placepads module tries to place the pads such that the distance
between the pads and their nearest connected pins in the is minimized.
It must also adhere to user-specified restrictions on side, position,
spacing and ordering.
____________________________________________________________________*/

placepads()
{
    if( padspacingG == EXACT_PADS ){
	return ;
    }
    find_core();  /* GET THE UPDATED CORE'S DIMENSION */

    D( "placepads/initially",
	print_pads( "pads initially\n", padarrayG, totalpadsG ) ;
    ) ;

    find_optimum_locations() ;
    D( "placepads/after_find_opt",
	print_pads( "pads after_cost\n", sortarrayG, totalpadsG ) ;
    ) ;

    sort_pads();
    D( "placepads/after_sort", 
	print_pads( "pads after sort\n", placearrayG, numpadsG );
    ) ;

    align_pads();
    D( "placepads/after_align",
	print_pads( "pads after align\n", placearrayG, numpadsG ) ;
    ) ;

    orient_pads();
    D( "placepads/after_orient",
	print_pads( "pads after orient\n", placearrayG, numpadsG ) ;
    ) ;

    dimension_pads();
    D( "placepads/after_dim", 
	print_pads( "pads after dimension\n", placearrayG, numpadsG ) ;
    ) ;

} /* end placepads */
/* ***************************************************************** */

static find_optimum_locations()
{
    INT i ;                  /* pad counter */
    INT side ;               /* loop thru valid sides */
    INT cost ;               /* current cost */
    INT bestpos ;            /* best modified position for pad */
    INT besttie ;            /* best position for pad for tiebreak */
    INT bestcost ;           /* best cost for pad or padgroup */
    INT bestside ;           /* best side to place pad or padgroup */
    PADBOXPTR pad ;          /* current pad */

    /** FIND OPTIMUM PLACE FOR PADS ACCORDING TO THE RESTRICTIONS **/
    for( i = 1; i <= totalpadsG; i++ ){

	/**** LEAVES AND SUBROOTS NEED TO BE PLACED ON THE SAME
	**** SIDE AS THEIR PARENT ROOT, HENCE WE PLACE THE ROOT
	**** FIRST, AND THEN PLACE ALL ITS CHILDREN **/

	pad = padarrayG[i] ;
	if( pad->padtype == PADGROUPTYPE && pad->hierarchy == ROOT  ){
	    /* the case of a padgroup root */
	    bestcost = INT_MAX ;
	    bestside = 1 ;
	    for (side = 1; side <= 4; side++ ) {
		if( pad->valid_side[ALL] || pad->valid_side[side] ){
		    cost = find_cost_for_a_side( pad,side,
			pad->lowerbound, pad->upperbound, pad->fixed ) ;
		    if( cost < bestcost) {
			bestcost = cost;
			bestside = side ;
		    }
		}
	    }
	    place_children( pad, bestside, pad->lowerbound, 
		pad->upperbound, pad->fixed ) ;

	} else if( pad->padtype == PADTYPE && pad->hierarchy == NONE ) {
	    /* the case of a pad that is not in a padgroup */
	    bestcost = INT_MAX ;
	    bestpos = 0 ; besttie = 0 ;
	    for (side = 1; side <= 4; side++ ) {
		if( pad->valid_side[ALL] || pad->valid_side[side] ){
		    cost = find_cost_for_a_side( pad,side,
			pad->lowerbound, pad->upperbound, pad->fixed ) ;
		    if( cost < bestcost) {
			bestcost = cost;
			bestside = side ;
			bestpos = sumposS ;
			besttie = sumtieS ;
		    }
		}
	    }
	    /* now use the best positions for the position */
	    sumposS = bestpos ;
	    sumtieS = besttie ;
	    place_pad( pad, bestside ) ;

	} /* end simple pad case */
    }
} /* end find_optimum */

/* ***************************************************************** */
static INT find_cost_for_a_side(pad,side,lb,ub,spacing_restricted)
PADBOXPTR pad;
INT  side ;
DOUBLE lb, ub ;
BOOL spacing_restricted ;
{
    INT i ;           /* children counter */
    INT pos ;         /* current pos. of current core pin constrained*/
    INT dist ;        /* current distance from core pin to pad */
    INT cost ;        /* sum of the opt pad pins to closest core pin */
    INT dist2 ;       /* under restrictions dist from core pin to pad */
    INT lowpos ;      /* convert lower bound to a position */
    INT uppos ;       /* convert upper bound to a position */
    INT bestpos ;     /* best constrained pos for pad to core for 1 net */
    INT besttie ;     /* best position for pad to core for 1 net */
    INT bestdist ;    /* best distance for pad to core for 1 net */
    INT tiebreak ;    /* best place to put pad pin unconstrained */
    BOOL pinFound ;   /* true if we find a match on current net */
    PINBOXPTR pinptr; /* current pin */
    PINBOXPTR netterm;/* loop thru pins of a net */
    PADBOXPTR child;  /* go thru the children of the padgroup */
    CBOXPTR cptr ;    /* current cell */
    PADBOXPTR padptr; /* use to check if pad */


    /**** FOR NORMAL PADS AND LEAVES, JUST CALCULATE THE COST */
    /*** AND POSITION. THE LEAF CASE IS THE RETURN CONDITION OF */
    /*** THE RECURSION ON THE ROOT PADS ***/

    if( retain_sideS && side != pad->padside ){
	/* during global routing must retain the side */
	return( PINFINITY ) ;
    }
    if( pad->hierarchy == LEAF || pad->hierarchy == SUBROOT ){
	if( !(pad->valid_side[ALL]) && !(pad->valid_side[side]) ){
	    /* this is not a valid side return a huge cost */
	    return( PINFINITY ) ;
	}
    }
    /* At this point are guaranteed to have a valid side */
    cost = 0 ;
    sumposS = 0 ;
    sumtieS = 0 ;

    /* determine spacing restrictions */
    calc_constraints( pad, side, &lb, &ub, &spacing_restricted, 
	&lowpos, &uppos ) ;

    if( pad->hierarchy == LEAF || pad->hierarchy == NONE ){


	/**** FOR ALL PINS BELONGING TO THE SAME NET AS PINS ON THE
	    PAD, FIND THE PIN CLOSEST TO THE SIDE IN padside. ****/

	/**** ASSUME NO PIN WAS FOUND ***/
	pin_countS = 0 ;

	for ( pinptr=carrayG[pad->cellnum]->pins;pinptr;pinptr=pinptr->nextpin) {
	    bestdist = INT_MAX ;
	    /*** GO TO FIRST TERMS OF THE NET TO MAKE SURE ALL
	         TERMINALS ARE INCLUDED ***/
	    pinFound = FALSE ;
	    netterm = netarrayG[pinptr->net]->pins;
	    for( ;netterm ;netterm = netterm->next ) {

		/**** CALCULATE THE DISTANCE FROM CORE.
		**** pos IS THE POSITION OF THE PAD ON THIS SIDE
		**** WHICH RESULTS IN THE SHORTEST DISTANCE TO THE PIN.
		**** ONLY PINS ON CELLS ARE IN THIS CONTEST **/

		cptr = carrayG[netterm->cell] ;
		if( (padptr = cptr->padptr) && !(padptr->macroNotPad)){
		    /* skip over this pad */
		    continue ;
		}
		switch (side) {
		    case L:
			pos  = netterm->ypos;
			dist = netterm->xpos - coreG[X][MINI];
			break;
		    case T:
			pos  = netterm->xpos;
			dist = coreG[Y][MAXI] - netterm->ypos;
			break;
		    case R:
			pos  = netterm->ypos;
			dist = coreG[X][MAXI] - netterm->xpos;
			break;
		    case B:
			pos  = netterm->xpos;
			dist = netterm->ypos - coreG[Y][MINI];
			break;
		    default :
			pos = INT_MAX ;
			dist = INT_MAX ;
		} /* end switch on side */

		tiebreak = pos ; /* save original spot */
		if( spacing_restricted ){
		    /* the pad placement on the side has been */
		    /* restricted in some way */
		    if( lowpos <= pos && pos <= uppos ){
			/* everythings cool do no extra distance */
			dist2 = 0 ;
		    } else if( lowpos > pos ){
			dist2 = ABS( lowpos - pos ) ;
			pos = lowpos ;
		    } else if( pos > uppos ){
			dist2 = ABS( pos - uppos ) ;
			pos = uppos ;
		    }
		    /* modify the distance by it Manhattan length */
		    /* to the pad in the orthogonal direction */
		    /* since this pad is fixed at a point */
/* could modify this to be more accurate since we want matching padpin*/
		    dist += dist2 ;
		}
		if (dist < bestdist) {
		    bestdist = dist;    /*** UPDATE THE BEST DISTANCE */
		    bestpos  = pos;     /*** AND BEST POSITION        */
		    besttie = tiebreak; /* save the original position */
		    pinFound = TRUE ;   /* pin on this net was found */
		}
	    } /* for(;netterm... end looking at this net */

	    if( pinFound ){
		/*** SUM UP THE BEST POSITION OF ALL PINS       */
		sumposS  += bestpos ; 
		sumtieS += besttie ;

		/*** KEEP TRACK OF THE TOTAL COST FOR THIS SIDE */
		cost += bestdist; 
		pin_countS++ ;
	    }
	} /* end for loop on pins of the pad */

	/*** IF NO PIN IS FOUND TO MATCH WITH PAD ARBITRARILY ***/
	/*** SET best position to random number for THIS PAD ***/
	if( pin_countS == 0 ){
	    if( spacing_restricted ){
		/* average between constraints */
		sumposS = (lowpos + uppos) / 2 ;
	    } else {
		/*
		    Randomly pick a cost to break ties in the
		    case that this pad could go on any side. Small 
		    value will not effect padgroups
		*/
		cost = PICK_INT( 0, 3 ) ;
		switch (side) {
		case L:
		    sumposS = PICK_INT( coreG[Y][MINI],coreG[Y][MAXI] ) ;
		    break;
		case T:
		    sumposS = PICK_INT( coreG[X][MINI],coreG[X][MAXI] ) ;
		    break;
		case R:
		    sumposS = PICK_INT( coreG[Y][MINI],coreG[Y][MAXI] ) ;
		    break;
		case B:
		    sumposS = PICK_INT( coreG[X][MINI],coreG[X][MAXI] ) ;
		    break;
		default:
		    break;
		}
	    }
	    sumtieS = sumposS ;
	} /* end pin_countS == 0 */

	return( cost ) ;

     } else {

	/*** 
	    IF THE PAD IS A SUPERPAD, THEN SEARCH THROUGH ALL *** 
	    ITS CHILDREN AND SUM THE COST AND IDEAL POSITION  ***
	    RECURSIVELY.  Use the spacing restrictions derived above.
	***/


	for( i = 1 ;i <= pad->children[HOWMANY]; i++ ){
	    child = padarrayG[pad->children[i]];
	    cost += find_cost_for_a_side( child, side,
		lb, ub, spacing_restricted ) ;
	}
	return( cost );
    }
} /* end find_cost_for_a_side */
/* ***************************************************************** */

/**** SET POSITION OF THE PAD.  POS IS THE SUM OF THE OPTIMAL POSITION
 **** FOR ALL TERMINALS OF THE PAD. WE DIVIDE BY THE NUMBER OF TERMINAL
 **** TO GET THE AVERAGE.  Place_pad must be performed immediately
 **** after a find_cost_for_a_side since sumposS and sumtieS
 **** are set in those routines.  Otherwise set sumposS and sumtieS
 **** to their proper values.
 ***/
static place_pad( pad, bestside )
PADBOXPTR pad ;
INT bestside ;
{

    /* use the number of pins with valid connections */
    /* pin_countS is set inf find_cost_for_a_side */
    if( pin_countS == 0 ){
	/*** SET PAD TO RANDOM POSITION DETERMINED IN find_cost_for_side*/
	pad->position = sumposS ;
	pad->tiebreak = sumtieS ;
    } else {
	pad->position = sumposS / pin_countS ;
	pad->tiebreak = sumtieS / pin_countS ;
    }
    /* now bound pad center to current core boundary for normal case */
    pad->padside = bestside ;
#ifdef LATER
    switch( bestside ){
	case L:
	case R:
	    pad->position = MAX( pad->position, coreG[Y][MINI] ) ;
	    pad->position = MIN( pad->position, coreG[Y][MAXI] ) ;
	    break;
	case T:
	case B:
	    pad->position = MAX( pad->position, coreG[X][MINI] ) ;
	    pad->position = MIN( pad->position, coreG[X][MAXI] ) ;
    } /* end bound of pad position */
#endif

} /* end place_pad */
/* ***************************************************************** */



/**** RECURSIVELY SET THE PADSIDE OF ALL CHILDREN OF THE ROOT PAD TO THE
 **** PADSIDE OF THE PARENT. GIVEN THAT SIDE, SET THE OPTIMAL CXCENTER */
static place_children( pad, side, lb, ub, spacing_restricted )
PADBOXPTR pad ;
INT side ;
DOUBLE lb, ub ;
BOOL spacing_restricted ;
{
    INT i ;           /* pad counter */
    INT pos ;         /* position of last placed pad */
    INT min_pos ;     /* min position of the last padgroup */
    INT max_pos ;     /* max position of the last padgroup */
    DOUBLE lowbound ; /* lower bound for pad or pad group */
    DOUBLE hibound ;  /* upper bound for pad or pad group */
    PADBOXPTR child;  /* go thru the children of the padgroup */

    /* DETERMINE SPACING RESTRICTIONS */
    lowbound = 0.0 ;
    hibound = 1.0 ;
    if( spacing_restricted ){
	/* this is the case that the spacing has been restricted */
	if( pad->fixed ){
	    /* if the padgroup bounds have been fixed, */
	    /* force position to be within bound */
	    /* assume we are ok and then correct it */
	    lowbound = pad->lowerbound ;
	    if( lowbound < lb ){
		lowbound = lb ;
	    }
	    if( lowbound > ub ){
		lowbound = ub ;
	    }
	    hibound = pad->upperbound ;
	    if( hibound < lb ){
		hibound = lb ;
	    }
	    if( hibound > ub ){
		hibound = ub ;
	    }
	} else {
	    /* this pad is not fixed use the given ub and lb */
	    lowbound = lb ; hibound = ub ;
	}
    } else {
	if( pad->fixed ){
	    /* the padgroup bounds have not been fixed */
	    /* just take the pad's restricted position */
	    lowbound = pad->lowerbound;
	    hibound = pad->upperbound;
	    spacing_restricted = TRUE ;
	}
    }
    /* **** END spacing restriction calculations *** */

    if( pad->hierarchy == LEAF ){
	find_cost_for_a_side( pad, side,
	    lowbound, hibound, spacing_restricted ) ;
	place_pad( pad, side ) ;
	return ;
    } else {
	pos = 0 ;
	min_pos = INT_MAX ;
	max_pos = INT_MIN ;
	for( i = 1; i <= pad->children[HOWMANY]; i++ ){
	    child = padarrayG[pad->children[i]];
	    place_children( child, side, lowbound, hibound, spacing_restricted ) ;
	    pos += child->position ; 
	    min_pos = MIN( child->position, min_pos ) ;
	    max_pos = MAX( child->position, max_pos ) ;
	}
	if( pad->children[HOWMANY] ){
	    pad->position = pos /= pad->children[HOWMANY] ;
	} else {
	    pad->position = pos ;
	}
	/* for tiebreak use the bounds of the padgroup and average them. */
	/* set the side of the pad */
	pad->padside = side ;
	pad->tiebreak = (min_pos + max_pos ) / 2 ;
	return ;
    }
} /* end place_children */
/* ***************************************************************** */




/* ***************************************************************** */
#ifdef DEBUG
print_pads( message, array, howmany )
char *message ;
PADBOXPTR *array ;
INT howmany ;
{
    INT i ;
    PADBOXPTR ptr ;
    CBOXPTR   cptr ;
    
    fprintf( stderr, "\n%s\n", message ) ;

    /* now print them out */
    for( i = 1 ; i <= howmany; i++ ){
	ptr = array[i] ;
	cptr = carrayG[ ptr->cellnum ] ;
	fprintf( stderr, 
	    "pad:%s x:%d y:%d type:%d side:%d pos:%d tie:%d orient:%d\n",
	    cptr->cname, cptr->cxcenter, cptr->cycenter, ptr->hierarchy,
	    ptr->padside, ptr->position, ptr->tiebreak, cptr->corient ) ;
    }
    fprintf( stderr, "\n" ) ;

    dimension_pads() ;
    G( process_graphics() ) ;

} /* end print_pads */

#endif /* DEBUG */
/* ***************************************************************** */


static find_core()
{
    INT minx, maxx ;
    INT miny, maxy ;
    INT l, r, b, t ;
    INT i ;
    INT num ;
    INT cell ;
    INT block ;
    INT xc, yc ;
    BBOXPTR bptr ;

    /* initialize xmin, ymax, etc. */
    minx = INT_MAX ;
    miny = INT_MAX ;
    maxx = INT_MIN ;
    maxy = INT_MIN ;
    if( virtualCoreS ){

	for( block = 1 ; block <= numRowsG ; block++ ) {
	    num = pairArrayG[block][0] ;
	    if( num == 0 ) {
		continue ;
	    }
	    cell = pairArrayG[block][ 1 ] ;
	    l = carrayG[cell]->cxcenter + carrayG[cell]->tileptr->left ;
	    cell = pairArrayG[block][ pairArrayG[block][0] ] ;
	    r  = carrayG[cell]->cxcenter + carrayG[cell]->tileptr->right ;
	    minx = MIN( minx, l ) ;
	    maxx = MAX( maxx, r ) ;
	}
	cell = pairArrayG[1][ 1 ] ;
	b = carrayG[cell]->cycenter + carrayG[cell]->tileptr->bottom ;
	miny = MIN( miny, b ) ;
	cell = pairArrayG[numRowsG][ 1 ] ;
	t = carrayG[cell]->cycenter + carrayG[cell]->tileptr->top ;
	maxy = MAX( maxy, t ) ;

    } else {
	for( block = 1 ; block <= numRowsG ; block++ ) {
	    bptr = barrayG[block] ;
	    xc = bptr->bxcenter ;
	    yc = bptr->bycenter ;
	    minx = MIN( minx, bptr->bleft + xc ) ;
	    maxx = MAX( maxx, bptr->bright + xc ) ;
	    miny = MIN( miny, bptr->bbottom + yc ) ;
	    maxy = MAX( maxy, bptr->btop + yc ) ;
	}
    }
    /* now check macro cells if they exist */
    for( i=numcellsG+1;i<=lastpadG;i++ ){
	if( carrayG[i]->padptr->macroNotPad ){
	    /* only calculate position if it is a macro */
	    get_global_pos( i, &l, &b, &r, &t ) ;

	    minx = MIN( minx, l ) ;
	    maxx = MAX( maxx, r ) ;
	    miny = MIN( miny, b ) ;
	    maxy = MAX( maxy, t ) ;
	}
    }

    /* now guarantee space between core and pads */
    minx -= track_pitchG ;
    miny -= track_pitchG ;
    maxx += track_pitchG ;
    maxy += track_pitchG ;

    /* now save the dimensions */
    coreG[Y][MINI]  = miny ;
    coreG[Y][MAXI]  = maxy ;

    coreG[X][MINI]  = minx ;
    coreG[X][MAXI]  = maxx ;

    perdimG[X] = maxx - minx ;
    perdimG[Y] = maxy - miny ;

} /* end FindCore */

/* turn virtual core on and off */
setVirtualCore( flag )
BOOL flag ;
{
    virtualCoreS = flag ;
} /* end set Virtual core */


/* given a cell it returns bounding box of cell in global coordinates */
get_global_pos( cell, l, b, r, t )
INT cell ; 
INT *l, *r, *b, *t ;
{

    CBOXPTR ptr ;

    ptr = carrayG[cell] ;
    *l = ptr->tileptr->left ;
    *r = ptr->tileptr->right ;
    *b = ptr->tileptr->bottom ;
    *t = ptr->tileptr->top ;
    YtranslateT( l, b, r, t, (INT) ptr->corient ) ;

    /* now add xcenter ycenter to get global position */
    *l += ptr->cxcenter ;
    *r += ptr->cxcenter ;
    *b += ptr->cycenter ;
    *t += ptr->cycenter ;

} /* end get_global_pos */

placepads_retain_side( flag )
BOOL flag;
{
    retain_sideS = flag ;
} /* end placepads_retain_side */
