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
FILE:	    placepin.c                                       
DESCRIPTION:find initial placement of softpins
CONTENTS:   placepin()
DATE:	    Mar 16, 1989 - added header and rewrote data structure.
REVISIONS:  Apr 23, 1990 - working version of softpin placement for
		instance changes.
	    Apr 28, 1990 - added control over overflow output.
	    May  2, 1990 - fixed problem with softcells with no
		pins and fixed error in find_hardpin_side.
	    May 15, 1990 - fixed problems with standard cells.
	    Oct 14, 1990 - fixed problem with pins moving off cell.
	    Sun Jan 20 21:45:41 PST 1991 - ported to AIX.
	    Fri Jan 25 18:14:17 PST 1991 - added routines for wire
		estimation.
	    Wed Jan 30 14:17:19 EST 1991 - updated for new trans
		library routines.
	    Sat Feb  2 00:00:22 EST 1991 - renamed find_rtile_side to
		file_tile_side a general routine.
	    Mon Feb  4 02:52:04 EST 1991 - added new placepin algorithm
		which correctly handles pin groups and side space 
		restrictions.
	    Mon Feb  4 15:08:16 EST 1991 - each level of the recursion
		in install_pin_groups needs a workspace.
	    Tue Mar 12 17:05:03 CST 1991 - fixed initialization problem
		with permutation.
	    Sat Apr 27 01:11:41 EDT 1991 - now avoid crash during
		stdcell designs with no pads.
	    Tue May 21 17:06:38 CDT 1991 - fixed argument problem,
		FLOAT vs. DOUBLE.
	    Wed Jun  5 15:43:33 CDT 1991 - changed REL_POS to REL_POST
		for accuracy.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) placepin.c version 3.18 11/23/91" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>
#include <initialize.h>
#include <yalecad/relpos.h>

#undef  NONE
#define NONE      0
#define LEAF      1
#define SUBROOT   2
#define ROOT      3

#ifdef INFINITY
#undef INFINITY
#endif
#define INFINITY  INT_MAX >> 4
#define DIV_2     >> 1

#define UP        1
#define DOWN      2
#define LEFT      3
#define RIGHT     4

#define USESIDERESTRICTIONS 0
#define ONESIDE             1
#define SOMESIDES           2
#define ALLSIDES            3
#define HOWMANY    0  /* the 0th element of arrays store # elements */

#define UNCOMMITTED         0
#define LAYER1              1
#define LAYER2              2
#define LAYER3              3

#define VSMALL  INT_MIN / 10  
#define VLARGE  INT_MAX / 10

typedef struct sidebox {
    INT x ;
    INT y ;
    INT start ;
    INT end ;
    INT loc ;
    INT length ;       /* length of the side */
    INT direction ;      /* LEFT, RIGHT, TOP, BOTTOM */
} SIDEBOX, *SIDEBOXPTR ;

static SIDEBOXPTR *sideArrayS ; /* the sides of the cell */
static CELLBOXPTR ptrS ;     /* the current cell pointer */
static INT aveTrackPitchS ;  /* the average track pitch */
static INT numpinS ;         /* number of pins (leaves only)for a cell */
static INT maxsideS = 0 ;    /* the max numsides over all cells */
static INT bestposS ;        /* the position to put pin */
static INT besttieS ;        /* the best tie breaker */
static INT *greedy_posS ;    /* array of pin positions for greedy method */
static PINBOXPTR *placearrayS;/* the leaves of a cell */
static BOOL tell_overflowS = FALSE ; /* turn on overflow message */
static BOOL softequivS ;      /* true if soft equivs are present */
BOOL contiguous_pinsG = TRUE ;

/* ****** static FUNCTION definitions ******** */
static find_optimal_locations( P1(BOOL newVertFlag ));
static INT find_cost_for_a_side( P5( PINBOXPTR pin, INT side, DOUBLE lb, DOUBLE ub, BOOL spacing_restricted )) ;
static determine_bbox( P2( INT net, INT cell )) ;
static place_pin( P4( PINBOXPTR pin, INT pos, INT tiebreak, INT side ) ) ;
static place_children( P5( PINBOXPTR pin, INT side, DOUBLE lb, DOUBLE ub,
			    BOOL spacing_restricted ) ) ;
static set_hardpin_pos( P2( PINBOXPTR pin, BOOL newVertFlag ) );
static sort_softpins() ;
static INT compare_pins( P2( PINBOXPTR *pinptr1, PINBOXPTR *pinptr2 ) ) ;
static INT sort_by_pos( P2( PINBOXPTR *pinptr1, PINBOXPTR *pinptr2 ) ) ;
static install_pin_groups( P1( PINBOXPTR pin ) ) ;
static permute_pg( P1( PINBOXPTR pin ) ) ;
static space_pins() ;
static place_soft_equivs() ;
static find_next_free_spotm( P3( SIDEBOXPTR sideptr, PINBOXPTR pin, INT *pos )) ;
static BOOL find_next_free_spotg(P3( SIDEBOXPTR sideptr, PINBOXPTR pin, INT *pos )) ;
static init_side_array( P1( BOOL newVertFlag ) ) ;
static side_to_global() ;
static find_hardpin_side()  ;

/* ****** global FUNCTION definitions ******** */
extern init_wire_est() ;
extern set_up_pinplace() ;
extern set_pin_verbosity( P1(BOOL flag ) ) ;
extern update_pins( P1( BOOL initialFlag ) ) ;
extern print_pins( P3( char *message , PINBOXPTR *array , INT howmany ) ) ;
extern INT *find_pin_sides( P1( INT cell ) ) ;
extern INT find_tile_side( P3( INT center, INT loc, INT direction ) ) ;
/* ################################################################## */

/*-------------------------------------------------------------------
 placepin() performs all the necessary step to place soft pins on
 a soft cell.  It first find the best spot for each pin using the
 half perimeter bounding box metric.  The best spot for pin groups
 is the average of all its children.  Placepin then spaces the
 pins along the sides and returns all answers in the pin->t?pos_new
 fields.  No calculation to funccost or timing penalty is performed.
____________________________________________________________________*/
placepin( cell, newVertFlag )
INT cell ;
BOOL newVertFlag ; /* use the x_new field if true otherwise use x field */
{

    INT i ;                          /* counter */
    INT howmany ;              /* number of soft pins including pg */
    INT where ;                /* where to put the pin on boundary */
    INT side ;                 /* the side the pin is to be placed */
    PINBOXPTR  pin ;
    SOFTBOXPTR spin ;
    GLISTPTR   pptr ;          /* pointer to nets of a cell */

    ptrS = cellarrayG[cell] ;

    if( ptrS->numpins <= 0 ){
	return ; /* no work to do */
    }

    init_side_array( newVertFlag ) ;
    numpinS = 0 ;
    softequivS = FALSE ;
    howmany = (INT) ptrS->softpins[HOWMANY] ;

    /** DETERMINE THE BOUNDING BOX OF NETS EXCLUDING THIS CELL **/
    for( pptr = ptrS->nets; pptr ; pptr = pptr->next ) {
	determine_bbox( pptr->p.net, cell ) ;
	/* tell unet that this net has been modified. */
	netarrayG[ pptr->p.net ]->nflag = TRUE ; 
    }

    find_optimal_locations( newVertFlag ) ;
    D( "placepins/after_find_opt",
	print_pins( "pins after_cost\n", ptrS->softpins, howmany ) ;
    ) ;

    sort_softpins() ;
    D( "placepins/after_sort", 
	print_pins( "pins after all sorting\n", placearrayS, numpinS );
    ) ;

    space_pins() ;
    D( "placepins/after_spacing", 
	print_pins( "pins after spacing\n", placearrayS, numpinS );
    ) ;
    side_to_global() ;

} /* end placepins */
/* ***************************************************************** */

static find_optimal_locations( newVertFlag )
BOOL newVertFlag ;
{
    INT i, j ;               /* pin counters */
    INT side ;               /* loop thru valid sides */
    INT cost ;               /* current cost */
    INT bestpos ;            /* best modified position for pins */
    INT besttie ;            /* best position for pins for tiebreak */
    INT bestcost ;           /* best cost for pin or pingroup */
    INT bestside ;           /* best side to place pin or pingroup */
    INT howmany ;            /* number of soft pins including pg */
    INT num_restrict ;       /* number of side restrictions for pin */
    BOOL invalid ;           /* whether side is invalid */
    PINBOXPTR  pin ;
    SOFTBOXPTR spin ;


    /** PLACE THE PINS ACCORDING TO THE RESTRICTIONS **/
    howmany = (INT) ptrS->softpins[HOWMANY] ;
    for( i = 1 ; i <= howmany; i++ ){

	/**** LEAVES AND SUBROOTS NEED TO BE PLACED ON THE SAME
	**** SIDE AS THEIR PARENT ROOT, HENCE WE PLACE THE ROOT
	**** FIRST, AND THEN PLACE ALL ITS CHILDREN **/
	pin = ptrS->softpins[i] ;
	spin = pin->softinfo ;

	if( pin->type == HARDPINTYPE ){
	    set_hardpin_pos( pin, newVertFlag ) ;
	    continue ;

	} else if( pin->type == PINGROUPTYPE && spin->hierarchy == ROOT  ){
	    /* the case of a pingroup root */
	    bestcost = INT_MAX ;
	    /* calculate number of restricted sides */
	    num_restrict = spin->restrict1[HOWMANY] ;
	    for( side = 1; side <= ptrS->numsides; side++ ) {
		if( num_restrict != 0 ){
		    invalid = TRUE ;
		    /* make sure this side is a valid side */
		    for( j = 1; j <= num_restrict; j++ ){
			if( spin->restrict1[j] == side ){
			    invalid = FALSE ;
			    break ;
			}
		    }
		    if( invalid ){
			/* this side is not valid go to next side */
			continue ;
		    }
		}
		/* at this point we have a valid side */

		cost = find_cost_for_a_side( pin, side,
			spin->lowerbound, spin->upperbound, spin->fixed ) ;
		if( cost < bestcost) {
		    bestcost = cost;
		    bestside = side ;
		}
	    } /* loop on sides */
	    place_children( pin, bestside, spin->lowerbound, 
		spin->upperbound, spin->fixed ) ;

	} else if( pin->type == SOFTPINTYPE && spin->hierarchy == NONE ) {
	    /* the case of a pin that is not in a pingroup */
	    bestcost = INT_MAX ;
	    /* calculate number of restricted sides */
	    num_restrict = spin->restrict1[HOWMANY] ;
	    for( side = 1; side <= ptrS->numsides; side++ ) {
		if( num_restrict != 0 ){
		    invalid = TRUE ;
		    /* make sure this side is a valid side */
		    for( j = 1; j <= num_restrict; j++ ){
			if( spin->restrict1[j] == side ){
			    invalid = FALSE ;
			    break ;
			}
		    }
		    if( invalid ){
			/* this side is not valid go to next side */
			continue ;
		    }
		}
		/* at this point we have a valid side */

		cost = find_cost_for_a_side( pin,side,
		    spin->lowerbound, spin->upperbound, spin->fixed ) ;
		if( cost < bestcost) {
		    bestcost = cost;
		    bestside = side ;
		    bestpos = bestposS ;
		    besttie = besttieS ;
		}
	    }
	    /* now use the best positions for the position */
	    place_pin( pin, bestpos, besttie, bestside ) ;

	} /* end simple pin case */
    } /* loop on number of pins */

} /* end find_optimal_locations */

/* ***************************************************************** */
static INT find_cost_for_a_side(pin,side,lb,ub,spacing_restricted)
PINBOXPTR pin;
INT  side ;
DOUBLE lb, ub ;
BOOL spacing_restricted ;
{
    INT i ;           /* children counter */
    INT pos ;         /* current pos. of current core pin constrained*/
    INT dist ;        /* current distance from core pin to pin */
    INT cost ;        /* cost of the opt pin placement */
    INT dista ;       /* under restrictions dist from core pin to ideal */
    INT dist1 ;       /* under restrictions dist from core pin to ideal */
    INT dist2 ;       /* under restrictions dist from core pin to ideal */
    INT xideal ;      /* ideal place to put pin */
    INT yideal ;      /* ideal place to put pin */
    INT pin_count ;   /* number of pins found */
    INT lowpos ;      /* convert lower bound to a position */
    INT uppos ;       /* convert upper bound to a position */
    INT bestpos ;     /* best constrained pos for ideal to core for 1 net */
    INT besttie ;     /* best position for ideal to core for 1 net */
    INT bestdist ;    /* best distance for ideal to core for 1 net */
    INT tiebreak ;    /* best place to put pin unconstrained */
    INT howmany ;     /* number of soft pins including pg */
    BOOL invalid ;    /* whether side is invalid */
    BOOL intersect ;  /* whether pin intersects side */
    INT num_restrict ;/* number of side restrictions for pin */
    BOOL pinFound ;   /* true if we find a match on current net */
    FLOAT lowbound ; /* lower bound for pin or pin group */
    FLOAT hibound ;  /* upper bound for pin or pin group */
    PINBOXPTR pinptr; /* current pin */
    PINBOXPTR netterm;/* loop thru pins of a net */
    PINBOXPTR  child; /* child of the pin group */
    SIDEBOXPTR sideptr;/* pointer to side information */
    SOFTBOXPTR spin ; /* current soft information pointer */
    NETBOXPTR netptr ;/* current net */


    /**** FOR NORMAL PINS AND LEAVES, JUST CALCULATE THE COST */
    /*** AND POSITION. THE LEAF CASE IS THE RETURN CONDITION OF */
    /*** THE RECURSION ON THE ROOT PINS ***/

    spin = pin->softinfo ;

    if( spin->hierarchy == LEAF || spin->hierarchy == SUBROOT ){
	/* check to make sure we have a valid side */
	num_restrict = spin->restrict1[HOWMANY] ;
	if( num_restrict != 0 ){
	    invalid = TRUE ;
	    /* make sure this side is a valid side */
	    for( i = 1; i <= num_restrict; i++ ){
		if( spin->restrict1[i] == side ){
		    invalid = FALSE ;
		    break ;
		}
	    }
	    if( invalid ){
		/* this side is not valid go to next side */
		/* set cost = INFINITY */
		return( INFINITY ) ;
	    }
	}
    }

    /* at this point we are processing a valid side */
    cost = 0 ;
    bestposS = 0 ;
    besttieS = 0 ;
    sideptr = sideArrayS[side] ;

    /* determine spacing restrictions */
    if( spacing_restricted ){
	/* this is the case that the spacing has been restricted */
	if( spin->fixed ){
	    /* if the pingroup bounds have been fixed, */
	    /* force position to be within bound */
	    /* assume we are ok and then correct it */
	    lowbound = spin->lowerbound ;
	    if( lowbound < lb ){
		lowbound = lb ;
	    }
	    if( lowbound > ub ){
		lowbound = ub ;
	    }
	    hibound = spin->upperbound ;
	    if( hibound < lb ){
		hibound = lb ;
	    }
	    if( hibound > ub ){
		hibound = ub ;
	    }
	} else {
	    /* this pin is not fixed use the given ub and lb */
	    lowbound = lb ; hibound = ub ;
	}
    } else {
	if( spin->fixed ){
	    /* the pingroup bounds have not been fixed */
	    /* just take the pin's restricted position */
	    spacing_restricted = TRUE ;
	}
	lowbound = spin->lowerbound;
	hibound = spin->upperbound;
    }
    if( spacing_restricted ){
	lowpos = (INT) ( lowbound * (DOUBLE) sideptr->length ) ;
	lowpos += sideptr->start ;
	uppos = (INT) ( hibound * (DOUBLE) sideptr->length ) ;
	uppos += sideptr->start ;
	/* lowpos and uppos are global coordinates here */
	/* which relate to the given sidepointer start and end points */
    }
    /* **** END spacing restriction calculations *** */

    if( spin->hierarchy == LEAF || spin->hierarchy == NONE ){

	netptr = netarrayG[pin->net] ;
	/* the ideal position for the pin is center of bounding box */
	xideal = (netptr->newxmin + netptr->newxmax ) DIV_2 ;
	yideal = (netptr->newymin + netptr->newymax ) DIV_2 ;
	bestdist = INT_MAX ;

	intersect = FALSE ;
	if( sideptr->direction==LEFT || sideptr->direction==RIGHT ){
	    /* horizontal side */
	    if( sideptr->start <= xideal && xideal <= sideptr->end ){
		/* side encompasses the point */
		dist = ABS( yideal - sideptr->loc ) ;
		pos = xideal ;
		intersect = TRUE ;
	    } else {
		/* check the 2 corner point using Manhattan dist */
		/* the constant part */
		dist1 = dist2 = ABS( sideptr->y - yideal ) ;
		/* the part due to the corners */
		dist1 += ABS( sideptr->x - xideal ) ;
		dist2 += ABS( (sideptr->x + sideptr->length) - xideal ) ;
		if( dist1 < dist2 ){
		    dist = dist1 ;
		    pos = sideptr->x ;
		} else {
		    dist = dist2 ;
		    pos = sideptr->x + sideptr->length ;
		}
	    }
	} else {
	    /* else vertical side */
	    if( sideptr->start <= yideal && yideal <= sideptr->end ){
		/* side is encompasses the point */
		dist = ABS( xideal - sideptr->loc ) ;
		pos = yideal ;
		intersect = TRUE ;
	    } else {
		/* check the 2 corner point using Manhattan dist */
		/* the constant part */
		dist1 = dist2 = ABS( sideptr->x - xideal ) ;
		/* the part due to the corners */
		dist1 += ABS( sideptr->y - yideal ) ;
		dist2 += ABS( (sideptr->y + sideptr->length) - yideal ) ;
		if( dist1 < dist2 ){
		    dist = dist1 ;
		    pos = sideptr->y ;
		} else {
		    dist = dist2 ;
		    pos = sideptr->y + sideptr->length ;
		}
	    }
	}
	/* NOW CHECK SPACING RESTRICTIONS */
	tiebreak = pos ; /* save original spot */
	if( spacing_restricted ){
	    /* the pin placement on the side has been */
	    /* restricted in some way */
	    if( lowpos <= pos && pos <= uppos ){
		/* everythings cool do no extra distance */
		dista = 0 ;
	    } else if( lowpos > pos ){
		dista = ABS( lowpos - pos ) ;
		pos = lowpos ;
	    } else if( pos > uppos ){
		dista = ABS( pos - uppos ) ;
		pos = uppos ;
	    }
	    /* modify the distance by it Manhattan length */
	    /* to the pin in the orthogonal direction */
	    /* since this pin is fixed at a point */
	    dist += dista ;
	}

	if( dist < bestdist || (dist == bestdist && intersect) ){
	    bestdist = dist;    /*** UPDATE THE BEST DISTANCE */
	    bestposS = pos;     /*** AND BEST POSITION        */
	    besttieS = tiebreak; /* save the original position */
	} 
	return( bestdist ) ;
        /* end single pin code */

    } else {

	/*** 
	    IF THE PIN IS A SUPERPIN, THEN SEARCH THROUGH ALL *** 
	    ITS CHILDREN AND SUM THE COST AND IDEAL POSITION  ***
	    RECURSIVELY.  Use the spacing restrictions derived above.
	***/

	howmany =  (INT) spin->children[HOWMANY] ;
	for( i = 1 ; i <= howmany; i++ ){
	    child = spin->children[i] ;
	    cost += find_cost_for_a_side( child, side,
		lowbound, hibound, spacing_restricted ) ;
	}
	return( cost );

    }
} /* end find_cost_for_a_side */
/* ***************************************************************** */

/* find the bounding box of this net without this cell */
static determine_bbox( net, cell )
INT net ;  /* calculate this net */
INT cell ; /* exclude this cell */
{
    NETBOXPTR netptr ;           /* current net */
    PINBOXPTR pinptr ;           /* current pin */
    BOOL noPinsFound ;           /* see if pins are found for this net */
    INT x, y ;                   /* current pinpos for speed */
    /* ********* calculate net half perimeter bounding box *********** */
    netptr =  netarrayG[net] ;
    noPinsFound = TRUE ;
    if( netptr->skip != TRUE ) {

	/* find first pin that we don't have skip field set */
	/* initialize bounding box pin count to 1 */
	for( pinptr = netptr->pins;pinptr; pinptr = pinptr->next ) {
	    if( pinptr->skip == 1 || pinptr->cell == cell ) {
		continue ;
	    }
	    noPinsFound = FALSE ;
	    netptr->newxmin = netptr->newxmax = pinptr->xpos ;
	    netptr->newymin = netptr->newymax = pinptr->ypos ;
	    pinptr = pinptr->next ;
	    break ;
	}
	/* Now find whether this pin impacts the bounding box */
	/* Note when we get more than one pin on the bounding box */
	for( ; pinptr ; pinptr = pinptr->next ) {
	    if( pinptr->skip == 1 || pinptr->cell == cell ) {
		continue ;
	    }
	    x = pinptr->xpos ;
	    y = pinptr->ypos ;

	    if( x <= netptr->newxmin ) {
		netptr->newxmin = x ;
	    } else if( x >= netptr->newxmax ) {
		netptr->newxmax = x ;
	    }
	    if( y <= netptr->newymin ) {
		netptr->newymin = y ;
	    } else if( y >= netptr->newymax ) {
		netptr->newymax = y ;
	    }
	}
    }
    if( noPinsFound ){
	/* this means we are free to put pin anywhere to minimize */
	/* wirelength */
	netptr->newxmin = VSMALL ;
	netptr->newxmax = VLARGE ;
	netptr->newymin = VSMALL ;
	netptr->newymax = VLARGE ;
    }
} /* end half perimeter bounding box calculation */
/* ***************************************************************** */

/**** SET XPOS OF THE PIN TO DESIRED POSITION.  *****/
static place_pin( pin, pos, tiebreak, side )
PINBOXPTR pin;
INT       pos;
INT       tiebreak;
INT       side;
{

    pin->txpos_new = pos ;
    pin->typos_new = tiebreak ;
    pin->softinfo->side = side ;

} /* end place_pin */
/* ***************************************************************** */


/**** RECURSIVELY SET THE SIDE OF ALL CHILDREN OF THE ROOT PIN TO THE
 **** PADSIDE OF THE PARENT. GIVEN THAT SIDE, SET THE OPTIMAL POSITION */
static place_children( pin, side, lb, ub, spacing_restricted )
PINBOXPTR pin ;
INT side ;
DOUBLE lb, ub ;
BOOL spacing_restricted ;
{
    INT i ;           /* pin counter */
    INT pos ;         /* position of last placed pin */
    INT min_pos ;     /* min position of the last pingroup */
    INT max_pos ;     /* max position of the last pingroup */
    INT howmany ;     /* number of soft pins including pg */
    INT child_pos ;   /* position of children */
    FLOAT lowbound ; /* lower bound for pin or pingroup */
    FLOAT hibound ;  /* upper bound for pin or pingroup */
    PINBOXPTR child;  /* go thru the children of the pingroup */
    SOFTBOXPTR spin ; /* current soft information pointer */


    spin = pin->softinfo ;

    /* DETERMINE SPACING RESTRICTIONS */
    if( spacing_restricted ){
	/* this is the case that the spacing has been restricted */
	if( spin->fixed ){
	    /* if the pingroup bounds have been fixed, */
	    /* force position to be within bound */
	    /* assume we are ok and then correct it */
	    lowbound = spin->lowerbound ;
	    if( lowbound < lb ){
		lowbound = lb ;
	    }
	    if( lowbound > ub ){
		lowbound = ub ;
	    }
	    hibound = spin->upperbound ;
	    if( hibound < lb ){
		hibound = lb ;
	    }
	    if( hibound > ub ){
		hibound = ub ;
	    }
	} else {
	    /* this pin is not fixed use the given ub and lb */
	    lowbound = lb ; hibound = ub ;
	}
    } else {
	if( spin->fixed ){
	    /* the pingroup bounds have not been fixed */
	    /* just take the pin's restricted position */
	    lowbound = spin->lowerbound;
	    hibound = spin->upperbound;
	    spacing_restricted = TRUE ;
	}
    }
    /* **** END spacing restriction calculations *** */

    if( spin->hierarchy == LEAF ){
	find_cost_for_a_side( pin, side,
	    lowbound, hibound, spacing_restricted ) ;
	/* bestposS and besttieS are set in find_cost_for_a_side */
	place_pin( pin, bestposS, besttieS, side ) ;
	return ;
    } else {
	pos = 0 ;
	min_pos = INT_MAX ;
	max_pos = INT_MIN ;

	/* txpos_new holds the best position on this side */
	/* typos_new holds the tie breaker */
	howmany = (INT) spin->children[HOWMANY] ;
	/* find the position of the children */
	for (i = 1; i <= howmany; i++) {
	    child = spin->children[i] ;
	    place_children( child, side, lowbound, hibound, spacing_restricted ) ;
	    child_pos = child->txpos_new ;
	    pos += child_pos ;
	    min_pos = MIN( child_pos, min_pos ) ;
	    max_pos = MAX( child_pos, max_pos ) ;
	}
	if( howmany ){
	    pin->txpos_new = pos /= howmany ;
	} else {
	    pin->txpos_new = pos ;
	}
	/* for tiebreak use the bounds of the pingroup and average them. */
	pin->typos_new = (min_pos + max_pos ) / 2 ;
	/* mark the side */
	spin->side = side ;
	return ;
    }
} /* end place_children */
/* ***************************************************************** */

/**** SET XPOS OF THE PIN TO DESIRED POSITION.  *****/
static set_hardpin_pos( pin, newVertFlag )
PINBOXPTR pin;
BOOL newVertFlag ;
{

    INT x, y ;                         /* pin x, y location */
    SOFTBOXPTR sptr ;                  /* pointer to soft information */
    SIDEBOXPTR sideptr ;               /* pointer to side information */

    if( newVertFlag ){
	REL_POS( ptrS->orient, 
	    x, y,                                    /* result */
	    pin->txpos_new, pin->typos_new,       /* cell relative */
	    ptrS->xcenter, ptrS->ycenter ) ;       /* cell center */
    } else {
	REL_POS( ptrS->orient, 
	    x, y,                                    /* result */
	    pin->txpos, pin->typos,               /* cell relative */
	    ptrS->xcenter, ptrS->ycenter ) ;       /* cell center */
    }
    sptr = pin->softinfo ;
    /* we already know the side */
    sideptr = sideArrayS[sptr->side] ;
    switch( sideptr->direction ){
	case UP:
	case DOWN:
	    pin->txpos_new = y ;
	    break ;
	case LEFT:
	case RIGHT:
	    pin->txpos_new = x ;
	    break ;
    }

} /* end set_hardpin_pos */
/**********************************************************************
			   PIN SORTING ROUTINES
***********************************************************************/

static sort_softpins()
{
    INT i ;                /* pin counter */
    INT pos ;              /* position in place array */
    INT howmany ;          /* number of soft pins including pg */
    SOFTBOXPTR spin ;      /* pointer to soft information */
    PINBOXPTR pin ;        /* current pin */

    /* first perform an initial sort to order the pins by side, hierarchy, */
    /* and position on the side. */
    howmany = (INT) ptrS->softpins[HOWMANY] ;
    Yquicksort( &(ptrS->softpins[1]), howmany, sizeof(PINBOXPTR),
	compare_pins ) ;

    D( "sort_softpins/1st_sort", 
	print_pins( "pins after 1st sort\n", ptrS->softpins, howmany );
    ) ;

    /* Now make sure the pins are permuted correctly */
    for( i = 1; i <= howmany; i++ ){
	pin = ptrS->softpins[i] ;
	spin = pin->softinfo ;
	ASSERTNCONT( spin, "sort_softpins","Problem with pointer" ) ;
	if (spin->hierarchy == ROOT){
	    permute_pg(pin) ;
	}
    }

    /* NOW INSTALL THE PIN GROUPS IN THEIR PROPER ORDER. There are 2 cases: */
    /* CASE I - CONTIGUOUS INSURE THAT GROUPS REMAIN INTACT */
    /* It is here that we set numpinS the number of actual pins of a cell */
    if( contiguous_pinsG ){
	for( i = 1; i <= howmany; i++ ){
	    pin = ptrS->softpins[i] ;
	    spin = pin->softinfo ;
	    if( spin->hierarchy == ROOT || spin->hierarchy == NONE ){
		install_pin_groups( pin ) ;
	    }
	}

    } else {
	/* CASE II -  LEAVES ARE ALIGNED LIKE ORDINARY PINS IF THEY HAVE NO */
	/* CONSTRAINTS SUCH AS ORDER OR PERMUTE.  **/
	/* put pins in place array then sort pins */
	for( i = 1; i <= howmany; i++ ){
	    pin = ptrS->softpins[i] ;
	    spin = pin->softinfo ;
	    if( spin->hierarchy == LEAF || spin->hierarchy == NONE ){
		placearrayS[++numpinS] = pin ;
	    }
	}
	Yquicksort( &(placearrayS[1]),numpinS,sizeof(PINBOXPTR),sort_by_pos);
    }
    /* at this point numpinS is set properly */

} /* end sort_softpins */
/* ***************************************************************** */

/*** compare_pins() RETURNS TRUE IF ARG1 > ARG2 BY ITS OWN RULES **/
static INT compare_pins( pinptr1, pinptr2 )
PINBOXPTR *pinptr1, *pinptr2 ;
{
    PINBOXPTR pin1, pin2;
    SOFTBOXPTR spin1, spin2;

    pin1 = *pinptr1 ;
    pin2 = *pinptr2 ;
    spin1 = pin1->softinfo ;
    spin2 = pin2->softinfo ;

    /* move the soft equivtypes to the end of the array */
    /* these don't have a correct side so move to the end */
    if( pin1->type == SOFTEQUIVTYPE ){
	return( 1 ) ;
    } else if( pin2->type == SOFTEQUIVTYPE ){
	return( 0 ) ;
    } 

    if( spin1->side != spin2->side) {
	return( spin1->side - spin2->side ) ;
    }
    if( spin1->hierarchy != spin2->hierarchy ){
	/*** MOVE ROOTS TO THE BEGINNING OF ARRAY MOVE */
	/* LEAVES ARE SEPARATED, ROOTS ARE MERGED **/
	if( spin1->hierarchy == SUBROOT ){
	    return( 1 ) ;
	} else if( spin2->hierarchy == SUBROOT ){
	    return( 0 ) ;
	} else if( spin1->hierarchy == LEAF ){
	    return( 1 ) ;
	} else if( spin2->hierarchy == LEAF ){
	    return( 0 ) ;
	}
    }
    /* check the position for equality */
    if( pin1->txpos_new == pin2->txpos_new ){
	/* hard pins always get preference */
	if( pin1->type == HARDPINTYPE ){
	    /* move pin2 to the end */
	    return( 0 ) ;
	} else if( pin2->type == HARDPINTYPE ){
	    /* move pin1 to the beginning */
	    return( 1 ) ;
	} else { /* a softpin - typos_new field is tiebreaker */
	    /* use the tiebreaker field to break ties */
	    return( pin1->typos_new - pin2->typos_new ) ;
	}
    } else {
	return( pin1->txpos_new - pin2->txpos_new ) ;
    }

} /* end compare_pins */
/* ***************************************************************** */

static INT sort_by_pos( pinptr1, pinptr2 )
PINBOXPTR *pinptr1, *pinptr2 ;
{

    PINBOXPTR pin1, pin2;
    SOFTBOXPTR spin1, spin2;

    pin1 = *pinptr1 ;
    pin2 = *pinptr2 ;
    spin1 = pin1->softinfo ;
    spin2 = pin2->softinfo ;

    /* move the soft equivtypes to the end of the array */
    /* these don't have a correct side so move to the end */
    if( pin1->type == SOFTEQUIVTYPE ){
	return( 1 ) ;
    } else if( pin2->type == SOFTEQUIVTYPE ){
	return( 0 ) ;
    } 

    /* always must maintain side */
    if( spin1->side != spin2->side) {
	return( spin1->side - spin2->side ) ;
    }
    if( spin1->ordered || spin1->permute ){
	return( 0 ) ;
    } else if( spin2->ordered || spin2->permute ){
	return( 1 ) ;
    } else if( pin1->txpos_new == pin2->txpos_new ){
	/* hard pins always get preference */
	if( pin1->type == HARDPINTYPE ){
	    /* move pin2 to the end */
	    return( 0 ) ;
	} else if( pin2->type == HARDPINTYPE ){
	    /* move pin1 to the beginning */
	    return( 1 ) ;
	} else { /* a softpin - typos_new field is tiebreaker */
	    /* use the tiebreaker field to break ties */
	    return( pin1->typos_new - pin2->typos_new ) ;
	}
    } else {
	return( pin1->txpos_new - pin2->txpos_new ) ;
    }

} /* end sort_by_pos */
/* ***************************************************************** */

/* install the pin groups and set numpinS */
static install_pin_groups( pin )
PINBOXPTR pin ;
{
    INT i ;                      /* pin counter */
    INT howmany ;                /* number of pins in group */
    INT initial_position ;       /* position of next open place in placearray */
    PINBOXPTR child ;            /* current child */
    PINBOXPTR *temparray;        /* sort the pingroups */
    SOFTBOXPTR spin ;            /* soft information */

    spin = pin->softinfo ;
    if( spin->hierarchy == LEAF || spin->hierarchy == NONE ){
	placearrayS[++numpinS] = pin ;
    } else {
	howmany = (INT) spin->children[HOWMANY] ;
	/* each level of the recursion needs its own temparray */
	temparray = (PINBOXPTR *) 
	    Yvector_alloc( 1,howmany,sizeof(PINBOXPTR) ) ;
	for( i = 1 ;i <= howmany ; i++ ){
	    child = spin->children[i] ;
	    temparray[i] = child ;
	}
	/* now sort the subroots or leaves to obey both order constraints */
	/* and permutation constraints.  Otherwise try to sort by opt. pos.*/
	Yquicksort( &(temparray[1]), howmany, sizeof(PINBOXPTR), sort_by_pos ) ;

	/* now that we have subroots or leaves in correct order */
	/* look at next level down */
	for( i = 1 ;i <= howmany ; i++ ){
	    child = temparray[ i ] ;
	    install_pin_groups( child ) ;
	}
	/* now free temp array */
	Yvector_free( temparray, 1, sizeof(PINBOXPTR) ) ;
    }

} /* end install_pin_groups */
/* ***************************************************************** */

static permute_pg( pin )
PINBOXPTR pin ;
{
    INT j, k ;                /* used to reverse pins */
    INT i ;                   /* pincounter */
    INT howmany ;             /* number of children in current pingroup */
    INT max_pos ;             /* max. value of the ideal positions of pin in pg */
    INT min_pos ;             /* min. value of the ideal positions of pin in pg */
    INT child_pos ;           /* position of children */
    INT forward_cost ;        /* cost to place pins in current order */
    INT bakward_cost ;        /* cost to place pins in reverse order */
    INT proposed_fpos ;       /* proposed uniformly spaced pos in forward order */
    INT proposed_bpos ;       /* proposed uniformly spaced pos in bakward order */
    DOUBLE spacing ;          /* spacing if we place pins in pg uniformly */
    PINBOXPTR child ;         /* current child */
    PINBOXPTR tmp ;           /* used to reverse permutable pins */
    PINBOXPTR *array ;        /* sort the children */
    SOFTBOXPTR spin ;         /* soft information */

    spin = pin->softinfo ;
    howmany = (INT) spin->children[HOWMANY] ;
    if( spin->permute ){
	/* first calculate span of pingroup */
	ASSERTNRETURN( howmany >= 2,"permute_pg",
	    "Must have at least 2 pins in a pingroup\n");
	child = spin->children[1] ;
	child_pos = child->txpos_new ;
	min_pos = child_pos ;
	max_pos = child_pos ;
	for( i = 2; i <= howmany ; i++ ){
	    child = spin->children[i] ;
	    child_pos = child->txpos_new ;
	    min_pos = MIN( child_pos, min_pos ) ;
	    max_pos = MAX( child_pos, max_pos ) ;
	}
	/* now find the cost if we evenly space the pins over that region */
	spacing = (DOUBLE) (max_pos - min_pos) / (DOUBLE) (howmany - 1) ;
	forward_cost = 0 ;
	bakward_cost = 0 ;
	for( i = 1; i <= howmany ; i++ ){
	    child = spin->children[i] ;
	    child_pos = child->txpos_new ;
	    proposed_fpos = min_pos + ROUND( (i - 1) * spacing ) ;
	    proposed_bpos = max_pos - ROUND( (i - 1) * spacing ) ;
	    forward_cost += ABS( child_pos - proposed_fpos ) ;
	    bakward_cost += ABS( child_pos - proposed_bpos ) ;
	}

	if( bakward_cost < forward_cost ) {
	    /* we need to reverse the permutation */
	    array = spin->children + 1;
	    j = howmany - 1;
	    k = 0;
	    while( k < j ){
		tmp        = array[j];
		array[j--] = array[k];
		array[k++] = tmp;
	    }
	}
   }
   /*** NEED TO CHECK THE CHILDREN REGARDLESS OF THE PERMUTABILITY OF
	THE PARENT ROOT */
   for( i = 1; i <= howmany; i++ ){
	child = spin->children[i] ;
	if( child->softinfo->hierarchy == SUBROOT){
	    permute_pg( child ) ;
	}
    }

} /* end permute_pg */
/* ***************************************************************** */

static space_pins()
{

    INT i ;                     /* counter on pins */
    INT j ;
    INT pos ;
    INT howmany ;
    INT old_side ;
    INT pincount ;              /* count the pins on a side */
    INT pos_orig ;              /* used to restore original start on side */
    INT first_pin_on_side ;     /* as the name says */
    BOOL invalid ;
    BOOL greedy ;               /* TRUE if greedy spacing method is on */
    BOOL status ;               /* TRUE if greedy method failed */
    PINBOXPTR   pin ;
    SOFTBOXPTR spin ;
    SIDEBOXPTR sptr ;
    SIDEBOXPTR sideptr ;


    /* now we only have to space the leaves */
    /* find the first pin and last pin on the side */
    greedy = FALSE ;
    old_side = 0 ;
    pincount = 0 ;
    for( i = 1 ; i <= numpinS; ){
	pin = placearrayS[i] ;
	spin = pin->softinfo ;
	if( spin->side == old_side ){
	    while( spin->side == old_side ){
		if( pin->type == HARDPINTYPE ){
		    pos = pin->txpos_new ;
		    if( greedy ){
			/* put hardpin in greedy array to make life simpler */
			greedy_posS[pincount++] = pos ;
		    }
		} else if( pin->type != SOFTEQUIVTYPE ){
		    if( greedy ){
			status = find_next_free_spotg( sideptr, pin, &pos ) ;
			/* delay storage of this position so we don't wipe */
			/* out txpos_new field in case we need to call */
			/* minimize overflow later */
			greedy_posS[pincount++] = pos ;
			if( status ){
			    /* this means we overflow the side minimize overflow */
			    /* redo this side */
			    greedy = FALSE ;
			    i = first_pin_on_side - 1 ;
			    /* restore pos */
			    pos = pos_orig ;
			}
		    } else { /* minimize the overflow on this side */
			find_next_free_spotm( sideptr, pin, &pos ) ;
			pin->txpos_new = pos ;
		    }
		}
		if( ++i > numpinS || pin->type == SOFTEQUIVTYPE ){
		    /* this is where we exit the loop. Finish off last greedy first*/
		    if( greedy ){
			for( j = 0; j < pincount; j++ ){
			    pin = placearrayS[first_pin_on_side+j] ;
			    pin->txpos_new = greedy_posS[j] ;
			}
		    }
		    if( pin->type == SOFTEQUIVTYPE ){
			softequivS = TRUE ;
		    }
		    break ;
		}
		pin = placearrayS[i] ;
		spin = pin->softinfo ;
	    } /* end while loop on this side */
	} else if( pin->type != SOFTEQUIVTYPE ){
	    /* see if we need to process the old side */
	    if( greedy ){
		for( j = 0; j < pincount; j++ ){
		    pin = placearrayS[first_pin_on_side+j] ;
		    pin->txpos_new = greedy_posS[j] ;
		}
	    }
	    /* start a new side */
	    old_side = spin->side ;
	    sideptr = sideArrayS[old_side] ;
	    /* always try to be greedy first */
	    greedy = TRUE ;
	    first_pin_on_side = i ;
	    pincount = 0 ;
	    /* set the initial position depending on direction on edge */
	    switch( sideptr->direction ){
		case RIGHT:
		case UP:
		    pos = sideptr->start ;
		    break ;
		case DOWN:
		case LEFT:
		    pos = sideptr->end ;
		    break ;
	    }
	    pos_orig = pos ;
	} else {
	    /* set side so we traverse thru SOFTEQUIV pins */
	    old_side = spin->side ;
	    softequivS = TRUE ;
	}
    }

    if( softequivS ){
	place_soft_equivs() ;
    }

    D( "space_pins", 
	for( i = 1 ; i <= numpinS; i++ ){
	    pin = placearrayS[i] ;
	    spin = pin->softinfo ;
	    fprintf( stderr, 
	    "pin:%s signal:%s side:%d oldpos:%d pos:%d pintype:%d\n",
		pin->pinname, netarrayG[pin->net]->nname, 
		spin->side, pin->typos_new, pin->txpos_new, pin->type ) ;
		
	}
	fprintf( stderr, "\n\n" ) ;
    ) ; /* end of debug */

} /* end space_pins */

static place_soft_equivs()
{
    INT i ;                     /* counter on pins */
    INT j ;
    INT k ;
    INT pos ;
    INT num_restrict ;          /* number of side restrictions for pin */
    INT orthog1, orthog2 ;      /* two directions orthogonal to current one */
    BOOL invalid ;              /* whether side is invalid */
    PINBOXPTR  pin ;
    PINBOXPTR  parent ;
    SOFTBOXPTR spin ;
    SOFTBOXPTR sparent ;
    SIDEBOXPTR sptr ;
    SIDEBOXPTR sideptr ;

    /* now place the equivalent pins */
    for( i = 1 ; i <= numpinS; i++ ){
	pin = placearrayS[i] ;
	spin = pin->softinfo ;
	if( pin->type != SOFTEQUIVTYPE ){
	    continue ;
	}
	parent = spin->parent ;
	sparent = parent->softinfo ;
	/* now find opposite side */
	sideptr = sideArrayS[sparent->side] ; 
	if( sideptr->direction == UP || sideptr->direction == DOWN ){
	    orthog1 = LEFT ;
	    orthog2 = RIGHT ;
	} else if( sideptr->direction == RIGHT || sideptr->direction == LEFT ){
	    orthog1 = UP ;
	    orthog2 = DOWN ;
	}
	pos = parent->txpos_new ;
	/* find the closest side that is valid */
	num_restrict = spin->restrict1[HOWMANY] ;
	for( j = 1; j <= ptrS->numsides; j++ ){
	    sptr = sideArrayS[j] ;
	    if( j == sparent->side || sptr->direction == orthog1 || 
		sptr->direction == orthog2 ){
		/* avoid the parent side and orthogonal sides */
		continue ;
	    }
	    if( num_restrict >= 1 ){
		/* make sure this side is a valid side */
		invalid = TRUE ;
		for( k = 1; k <= num_restrict; k++ ){
		    if( spin->restrict1[k] == j ){
			invalid = FALSE ;
			break ;
		    }
		}
		if( invalid ){
		    continue ;
		}
	    }
	    if( sptr->start <= pos && pos <= sptr->end ){
		pin->txpos_new = pos ;
		pin->softinfo->side = j ;
		pin->skip = FALSE ;
	    } /* end for loop on number of sides */
	} 
    } /* end placing equiv pins */
} /* end place_softequivs */

/* minimize the amount of overflow on this side */
static find_next_free_spotm( sideptr, pin, pos )
SIDEBOXPTR sideptr ;
PINBOXPTR pin ;
INT *pos ;
{

    INT space ;
    SOFTBOXPTR spin ;

    switch( ABS(pin->layer) ){
    case UNCOMMITTED:
	space = aveTrackPitchS ;
	break ;
    case LAYER1:
	space = track_spacingXG ;
	break ;
    case LAYER2:
	space = track_spacingYG ;
	break ;
    case LAYER3:
	space = aveTrackPitchS ;
	break ;
    } /* end switch */

    switch( sideptr->direction ){
	case RIGHT:
	case UP:
	    *pos += space ;
	    if( tell_overflowS ){
		if( *pos >= sideptr->end ){
		    /* overflow on this side */
		    sprintf( YmsgG, 
		    "overflow for cell:%d pin:%s\n", pin->cell, pin->pinname ) ;
		    M( WARNMSG, "space_pins", YmsgG ) ;
		}
	    }
	    break ;
	case DOWN:
	case LEFT:
	    *pos -= space ;
	    if( tell_overflowS ){
		if( *pos <= sideptr->start ){
		    /* overflow on this side */
		    sprintf( YmsgG, 
		    "overflow for cell:%d pin:%s\n", pin->cell, pin->pinname ) ;
		    M( WARNMSG, "space_pins", YmsgG ) ;
		}
	    }
	    break ;
    } 

} /* find_next_free_spotm */
/* ***************************************************************** */

/* this is a greedy method which may overflow quite often */
/* pos is the last place a pin was placed. Returns TRUE if overflow has */
/* occurred while placing on this side */
static BOOL find_next_free_spotg( sideptr, pin, pos )
SIDEBOXPTR sideptr ;
PINBOXPTR pin ;
INT *pos ;
{

    INT space ;
    INT newpos ;
    SOFTBOXPTR spin ;

    switch( ABS(pin->layer) ){
    case UNCOMMITTED:
	space = aveTrackPitchS ;
	break ;
    case LAYER1:
	space = track_spacingXG ;
	break ;
    case LAYER2:
	space = track_spacingYG ;
	break ;
    case LAYER3:
	space = aveTrackPitchS ;
	break ;
    } /* end switch */

    newpos = pin->txpos_new ;

    switch( sideptr->direction ){
	case RIGHT:
	case UP:
	    /* account for spacing - pos is now at the minimum valid position */
	    *pos += space ;
	    if( newpos > *pos ){
		*pos = newpos ;
	    }
	    if( *pos >= sideptr->end ){
		/* overflow on this side */
		return( TRUE ) ;
	    }
	    break ;
	case DOWN:
	case LEFT:
	    /* account for spacing - pos is now at the maximum valid position */
	    *pos -= space ;
	    if( newpos < *pos ){
		*pos = newpos ;
	    }
	    if( *pos <= sideptr->start ){
		/* overflow on this side */
		return( TRUE ) ;
	    }
	    break ;
    } 
    return( FALSE ) ;

} /* find_next_free_spotg */
/* ***************************************************************** */

static side_to_global()
{
    INT     i;
    INT     side ;
    INT     tmp ;
    INT     rev_orient ;
    INT     x, y ;
    PINBOXPTR  pin ;
    SOFTBOXPTR spin ;
    SIDEBOXPTR sptr ;

    rev_orient = Ytrans_inv_orient( ptrS->orient ) ;
    for( i = 1 ; i <= numpinS; i++ ){
	pin = placearrayS[i] ;
	spin = pin->softinfo ;
	if( !spin ){
	    continue ;
	}
	side = spin->side ;
	if( side > 0 ){
	    sptr = sideArrayS[side] ;
	} else {
	    if( pin->type == SOFTEQUIVTYPE ){
		pin->skip = TRUE ;
	    } else {
		fprintf( stderr, "side = 0\n" ) ;
	    }
	}
	switch( sptr->direction ){
	    case UP:
		tmp = pin->txpos_new ;
		pin->txpos_new = sptr->loc ;
		pin->typos_new = tmp ;
		break ;
	    case DOWN:
		tmp = pin->txpos_new ;
		pin->txpos_new = sptr->loc ;
		pin->typos_new = tmp ;
		break ;
	    case LEFT:
	    case RIGHT:
		pin->txpos_new = pin->txpos_new ;
		pin->typos_new = sptr->loc ;
		break ;
	}
	x = pin->txpos_new -= ptrS->xcenter ;
	y = pin->typos_new -= ptrS->ycenter ;

	/* translate back to current view */	
	REL_POS( rev_orient,
	    pin->txpos_new, pin->typos_new,          /* result */
	    x, y,                                /* cell relative */
	    0, 0 ) ;                               /* cell center */

	/* now calculate global coordinates */
	REL_POS( ptrS->orient,
	    pin->newx, pin->newy,                     /* result */
	    pin->txpos_new, pin->typos_new,       /* cell relative */
	    ptrS->xcenter, ptrS->ycenter ) ;       /* cell center */

	/* tell unet that this pin has changed position */
	pin->flag = TRUE ;

	D( "twmc/side_to_global",
	    fprintf( stderr, 
	    "pin:%s signal:%s side:%d txpos:%d typos:%d xpos:%d ypos:%d\n",
		pin->pinname, netarrayG[pin->net]->nname, 
		spin->side, pin->txpos_new, pin->typos_new,
		pin->newx,  pin->newy, pin->type ) ;
		
	    fprintf( stderr, "\n\n" ) ;
	) ; /* end of debug */

    } /* end for loop */
} /* end side_to_global() */

set_up_pinplace()
{
    INT i ;                       /* counter */
    INT j ;                       /* counter */
    INT k ;                       /* counter */
    INT numinst ;                 /* the number of instances for cell */
    INT howmany ;                 /* the number of sofpins for cell */
    INT max_soft ;                /* count the number of softpins overall inst. */
    INT last_net ;                /* last unique net */
    PINBOXPTR pin ;               /* current pin pointer */
    GLISTPTR nlist, tmp ;         /* used to build nets of a cell */
    INSTBOXPTR instptr ;          /* instance information */


    if( numsoftG == 0 && numstdcellG == 0 ){
	return ;
    }
    /* find the maximum number of sides on a cell */
    for( i = 1; i <= numcellsG ; i++ ){ 
	ptrS = cellarrayG[i] ;
	if( ptrS->softflag ){

	    if( instptr = ptrS->instptr ){
		numinst = ptrS->instptr->numinstances ;
		for( j = 0; j < numinst ; j++ ){
		    maxsideS = MAX( maxsideS, instptr->numsides[j] ) ;
		}
	    } else {
		maxsideS = MAX( maxsideS, ptrS->numsides ) ;
	    }

	    /* build data structure for nets of the cell */
	    /* pins are sorted by net in sortpins() */
	    /* make use of this fact to find set of unique nets */
	    last_net = 0 ;
	    for( pin = ptrS->pinptr ; pin; pin = pin->nextpin ){
		if( pin->net != last_net ){
		    last_net = pin->net ;
		    /* allocate space for net list */
		    if( tmp = ptrS->nets ){
			nlist = ptrS->nets = (GLISTPTR)
			    Ysafe_malloc(sizeof(GLISTBOX));
			nlist->next = tmp ;
		    } else {
			nlist = ptrS->nets = (GLISTPTR)
			    Ysafe_malloc(sizeof(GLISTBOX));
			nlist->next = NULL ;
		    }
		    nlist->p.net = last_net ;
		}
	    }
	}
    }
    /* now allocate the array */
    sideArrayS = (SIDEBOXPTR *) 
	Yvector_alloc( 1,maxsideS,sizeof(SIDEBOXPTR) ) ;

    for( i = 1; i <= maxsideS ; i++ ){
	sideArrayS[i] = (SIDEBOXPTR) Ysafe_malloc(sizeof(SIDEBOX) ) ;
    }

    /* initialize average track pitch */
    aveTrackPitchS = (track_spacingXG + track_spacingYG) / 2 ;

    max_soft = 0 ;
    for( i = 1; i <= numcellsG ; i++ ){ 
	ptrS = cellarrayG[i] ;
	if( ptrS->softflag ){
	    /* need to initialize each instance */
	    if( instptr = ptrS->instptr ){
		numinst = ptrS->instptr->numinstances ;
		for( j = 0; j < numinst; j++ ){
		    ptrS->numsides = instptr->numsides[j] ;
		    ptrS->vertices = instptr->vert_inst[j] ;
		    if( !(ptrS->softpins)){
			continue ;
		    }
		    howmany = (INT) ptrS->softpins[HOWMANY] ;
		    max_soft = MAX( max_soft, howmany ) ;
		    /* set each pin's correct instance */
		    for( k = 1 ; k <= howmany; k++ ){
			pin = ptrS->softpins[k] ;
			pin->softinfo = pin->soft_inst[j] ;
		    }
		    /* now set the pins */
		    init_side_array( FALSE ) ;
		    find_hardpin_side() ;
		}
		/* now set everything as it was */
		j = ptrS->cur_inst ;
		ptrS->numsides = instptr->numsides[j] ;
		ptrS->vertices = instptr->vert_inst[j] ;
	    } else if( ptrS->softpins ){
		howmany = (INT) ptrS->softpins[HOWMANY] ;
		max_soft = MAX( max_soft, howmany ) ;
		init_side_array( FALSE ) ;
		find_hardpin_side() ;
	    } 
	}
    }
    if( max_soft > 0 ){
	placearrayS = (PINBOXPTR *) Yvector_alloc( 1, max_soft,sizeof(PINBOXPTR) ) ;
	greedy_posS = (INT *) Yvector_alloc( 0, max_soft,sizeof(INT) ) ;
    }

} /* end set_up_pinplace */

static init_side_array( newVertFlag )
BOOL newVertFlag ; /* use _new fields if true use x, y otherwise */
{

    INT i ;                      /* counter */
    INT j ;                      /* counter */
    INT *xvert ;                 /* the xvertices of cell */
    INT *yvert ;                 /* the yvertices of cell */
    VERTBOXPTR vert ;            /* the cells vertices */
    SIDEBOXPTR this_side ;       /* current side pointer */
    SIDEBOXPTR next_side ;       /* next side pointer */
    BOUNBOXPTR bounptr ;         /* bounding box pointer */

    
    vert = ptrS->vertices ;
    if( newVertFlag ){
	xvert = vert->x_new ;
	yvert = vert->y_new ;
    } else {
	xvert = vert->x ;
	yvert = vert->y ;
    }
    /* setup translation of output points */
    bounptr = ptrS->bounBox[0] ;
    /* now init the translation routines using bounding box */
    Ytrans_init( bounptr->l,bounptr->b,bounptr->r,bounptr->t,
		ptrS->orient ) ;
    for( i = 1; i <= ptrS->numsides; i++ ){
	this_side = sideArrayS[i] ;
	/* rel position is a macro which calculates */
	/* absolute pin loc - defined in relpos.h */
	REL_POST( ptrS->orient, 
	    this_side->x, this_side->y,              /* result */
	    xvert[i], yvert[i],                   /* cell relative */
	    ptrS->xcenter, ptrS->ycenter ) ;       /* cell center */
    }

    /* now determine the direction and path for edge */
    for( i = 1, j = 1; i <= ptrS->numsides; i++ ){
	if( ++j > ptrS->numsides ){
	    j = 1 ;
	}
	this_side = sideArrayS[i] ;
	next_side = sideArrayS[j] ;
	if( this_side->x == next_side->x ){
	    if( this_side->y < next_side->y ){
		this_side->direction = UP ; 
		this_side->start = this_side->y ;
		this_side->end = next_side->y ;
	    } else {
		this_side->direction = DOWN ; 
		this_side->start = next_side->y ;
		this_side->end = this_side->y ;
	    }
	    this_side->loc = this_side->x ;
	} else if( this_side->y == next_side->y ){
	    if( this_side->x < next_side->x ){
		this_side->direction = RIGHT ; 
		this_side->start = this_side->x ;
		this_side->end = next_side->x ;
	    } else {
		this_side->direction = LEFT ; 
		this_side->start = next_side->x ;
		this_side->end = this_side->x ;
	    }
	    this_side->loc = this_side->y ;
	}
	this_side->length = this_side->end - this_side->start ;

	D( "twmc/sidearray",
	fprintf( stderr, "x:%d y:%d start:%d end:%d loc:%d len:%d D:%d\n",
	    this_side->x, this_side->y, this_side->start, this_side->end,
	    this_side->loc, this_side->length, this_side->direction ) ;
	) ;
    }
} /* end init_side_array() */

/* find the side that the hardpin is nearest. */
static find_hardpin_side() 
{
    
    INT i ;                        /* counter */
    INT x, y ;                     /* pin x,y location */
    INT dist ;                     /* find the best place for pin */
    INT dist1 ;                    /* determine dist for pin */
    INT howmany ;                  /* # of pingroup children */
    INT bestdist ;                 /* determine dist for pin */
    INT bestside ;                 /* best side for the pin */
    BOOL intersect ;               /* true if ideal pt intersects  side */
    PINBOXPTR pin ;                /* the current pin */
    SOFTBOXPTR sptr ;              /* soft information */
    SIDEBOXPTR sideptr ;           /* current side */

    for( pin = ptrS->pinptr; pin; pin = pin->nextpin ){
	sptr = pin->softinfo ;
	if( pin->type != HARDPINTYPE ){
	    continue ;
	}
	x = pin->txpos + ptrS->xcenter ;
	y = pin->typos + ptrS->ycenter ;
	bestdist = INT_MAX ;
	for( i = 1; i <= ptrS->numsides; i++ ){
	    sideptr = sideArrayS[i] ;
	    intersect = FALSE ;
	    if( sideptr->direction==LEFT || sideptr->direction==RIGHT ){
		if( sideptr->start <= x && x <= sideptr->end ){
		    /* side is encompasses the point */
		    dist = ABS( y - sideptr->loc ) ;
		    intersect = TRUE ;
		} else {
		    /* check the corner point */
		    dist  = ABS( sideptr->x - x ) ;
		    dist1 = ABS( sideptr->y - y ) ;
		    /* find Manhattan dist */
		    dist += dist1 ;
		}
	    } else {
		/* else vertical side */
		if( sideptr->start <= y && y <= sideptr->end ){
		    /* side is encompasses the point */
		    dist = ABS( x - sideptr->loc ) ;
		    intersect = TRUE ;
		} else {
		    /* check the corner point */
		    dist  = ABS( sideptr->x - x ) ;
		    dist1 = ABS( sideptr->y - y ) ;
		    /* find Manhattan dist */
		    dist += dist1 ;
		}
	    }
	    if( dist < bestdist || (dist == bestdist && intersect) ){
		bestdist = dist ;
		bestside = i ;
	    } 
	} /* end loop on number of sides */
	/* now that we know the best side save it */
	sptr->side = bestside ;

    } /* end loop on hardpins */
} /* end find_hardpin_side() */


update_pins( initialFlag )  /* initialize pin placement */
BOOL initialFlag ;/* if TRUE set all fields;if FALSE update orig fields */
{
    INT howmany ;        /* number of cells with soft pins */
    INT i ;              /* counter */
    INT orient ;         /* cell orientation */
    INT inst ;           /* current instance */
    CELLBOXPTR cellptr ; /* current cell */
    PINBOXPTR  pin ;     /* current pin */
    SOFTBOXPTR spin ;    /* current soft pin information */

    if( numsoftG > 0 || numstdcellG > 0 ){
	howmany = (INT) softPinArrayG[HOWMANY] ;
	for( i = 1; i <= howmany; i++ ){
	    cellptr = softPinArrayG[i] ;
	    placepin( cellptr->cellnum, FALSE ) ;

	    inst = cellptr->cur_inst ;

	    orient = cellptr->orient ;
	    for( pin = cellptr->pinptr; pin ; pin = pin->nextpin ){
		if( initialFlag ){
		    pin->txpos_orig[inst] = pin->txpos = pin->txpos_new ;
		    pin->typos_orig[inst] = pin->typos = pin->typos_new ;
		} else {
		    pin->txpos_orig[inst] = pin->txpos ;
		    pin->typos_orig[inst] = pin->typos ;
		}

		/* rel pos is a macro which calculates */
		/* absolute pin location */
		/* defined in relpos.h */
		REL_POS( orient, 
		    pin->xpos, pin->ypos,                 /* result */
		    pin->txpos, pin->typos,            /* cell relative */
		    cellptr->xcenter, cellptr->ycenter );/* cell center */
	    }

	    D( "initial_pinplace", 

		for( pin = cellptr->pinptr; pin ; pin = pin->nextpin ){
		    spin = pin->softinfo ;
		    fprintf( stderr, 
			"pin:%10s signal:%8s side:%d tx:%d x:%d ",
			pin->pinname, netarrayG[pin->net]->nname, 
			spin->side, pin->txpos, pin->xpos ) ;
		    fprintf( stderr, "ty:%d y:%d pintype:%d\n",
			pin->typos, pin->ypos, pin->type ) ;
		} /* end for loop on pins */
		fprintf( stderr, "\n\n" ) ;

	    ) ; /* end of debug */
	} /* loop on cells with softpins */
    } /* end test on existence of soft cells */
} /* initial pinplace */

set_pin_verbosity( flag ) 
BOOL flag ;
{
    tell_overflowS = flag ;
} /* end set_pin_verbosity */

/* **************************************************************** */
/*  THE FUNCTIONS BELOW ARE USED FOR THE WIRE ESTIMATOR. WE TRY TO */
/*  SHARE CODE WITH THE PLACEPIN FUNCTIONS. *******                */

INT *find_pin_sides( cell )
INT cell ;
{
    
    INT i ;                        /* counter */
    INT x, y ;                     /* pin x,y location */
    INT dist ;                     /* find the best place for pin */
    INT dist1 ;                    /* determine dist for pin */
    INT howmany ;                  /* # of pingroup children */
    INT bestdist ;                 /* determine dist for pin */
    INT bestside ;                 /* best side for the pin */
    INT *pins_on_a_side ;          /* number of pins on each side */
    BOOL intersect ;               /* true if ideal pt intersects  side */
    PINBOXPTR pin ;                /* the current pin */
    SIDEBOXPTR sideptr ;           /* current side */

    ptrS = cellarrayG[cell] ;

    if( ptrS->numpins <= 0 ){
	return( NULL ) ; /* no work to do */
    }
    /* allocate space for pin counter */
    pins_on_a_side = (INT *) Yvector_calloc(1,ptrS->numsides,sizeof(INT)) ;

    /** DETERMINE THE BOUNDARY OF THE CURRENT CELL **/
    init_side_array( FALSE ) ;
    for( pin = ptrS->pinptr; pin; pin = pin->nextpin ){
	x = pin->xpos ;
	y = pin->ypos ;
	bestdist = INT_MAX ;
	for( i = 1; i <= ptrS->numsides; i++ ){
	    sideptr = sideArrayS[i] ;
	    intersect = FALSE ;
	    if( sideptr->direction==LEFT || sideptr->direction==RIGHT ){
		if( sideptr->start <= x && x <= sideptr->end ){
		    /* side is encompasses the point */
		    dist = ABS( y - sideptr->loc ) ;
		    intersect = TRUE ;
		} else {
		    /* check the corner point */
		    dist  = ABS( sideptr->x - x ) ;
		    dist1 = ABS( sideptr->y - y ) ;
		    /* find Manhattan dist */
		    dist += dist1 ;
		}
	    } else {
		/* else vertical side */
		if( sideptr->start <= y && y <= sideptr->end ){
		    /* side is encompasses the point */
		    dist = ABS( x - sideptr->loc ) ;
		    intersect = TRUE ;
		} else {
		    /* check the corner point */
		    dist  = ABS( sideptr->x - x ) ;
		    dist1 = ABS( sideptr->y - y ) ;
		    /* find Manhattan dist */
		    dist += dist1 ;
		}
	    }
	    if( dist < bestdist || (dist == bestdist && intersect) ){
		bestdist = dist ;
		bestside = i ;
	    } 
	} /* end loop on number of sides */
	pins_on_a_side[bestside]++ ;

    } /* end loop on pins */
    return( pins_on_a_side ) ;
} /* end find_pinsides() */

#include <dens.h>
/* given a routing tile edge find matching side */
INT find_tile_side( center, loc, direction )
INT center, loc, direction ;
{
    INT i ;                      /* traverse the sides of cell */
    SIDEBOXPTR this_side ;       /* current side pointer */

    for( i = 1; i <= ptrS->numsides; i++ ){
	this_side = sideArrayS[i] ;
	if( direction == TILEL || direction == TILER ){
	    if( this_side->direction == UP ||
		this_side->direction == DOWN ){
		if( this_side->loc == loc &&
		    this_side->start <= center &&
		    center <= this_side->end ){
		    return( i ) ;
		}
	    }
	} else { /* TILET or TILEB */
	    if( this_side->direction == LEFT ||
		this_side->direction == RIGHT ){
		if( this_side->loc == loc &&
		    this_side->start <= center &&
		    center <= this_side->end ){
		    return( i ) ;
		}
	    }
	}
    }
    return( 0 ) ;
} /* end find_tile_side */

init_wire_est()
{
    INT i ;                       /* counter */
    INT j ;                       /* counter */
    INT maxsides ;                /* the max numsides over all cells */
    INT numinst ;                 /* the number of instances for cell */
    INSTBOXPTR instptr ;          /* instance information */

    maxsides = 0 ;
    for( i = 1; i <= numcellsG ; i++ ){ 
	ptrS = cellarrayG[i] ;

	if( instptr = ptrS->instptr ){
	    numinst = ptrS->instptr->numinstances ;
	    for( j = 0; j < numinst ; j++ ){
		maxsides = MAX( maxsides, instptr->numsides[j] ) ;
	    }
	} else {
	    maxsides = MAX( maxsides, ptrS->numsides ) ;
	}
    }
    if( sideArrayS ){
	if( maxsides > maxsideS ){
	    sideArrayS = (SIDEBOXPTR *) 
		Yvector_realloc(sideArrayS,1,maxsides,sizeof(SIDEBOXPTR));
	    for( i = maxsideS+1; i <= maxsides ; i++ ){
		sideArrayS[i] = (SIDEBOXPTR)Ysafe_malloc(sizeof(SIDEBOX));
	    }
	}
    } else {
	sideArrayS = (SIDEBOXPTR *) 
	    Yvector_alloc(1,maxsides,sizeof(SIDEBOXPTR));
	for( i = 1; i <= maxsides ; i++ ){
	    sideArrayS[i] = (SIDEBOXPTR)Ysafe_malloc(sizeof(SIDEBOX));
	}
    }

} /* end init_wire_est() */


/* ***************************************************************** */
#ifdef DEBUG
print_pins( message, array, howmany )
char *message ;
PINBOXPTR *array ;
INT howmany ;
{
    INT i ;
    PINBOXPTR ptr ;
    SOFTBOXPTR sptr ;
    
    fprintf( stderr, "\n%s\n", message ) ;

    /* now print them out */
    for( i = 1 ; i <= howmany; i++ ){
	ptr = array[i] ;
	sptr = ptr->softinfo ;
	fprintf( stderr, 
	    "pin:%s pos:%d tie:%d type:%d side:%d order:%d fixed:%d\n",
	    ptr->pinname, ptr->txpos_new, ptr->typos_new, sptr->hierarchy,
	    sptr->side, sptr->ordered, sptr->fixed ) ;
    }
    fprintf( stderr, "\n" ) ;

    /*
    G( process_graphics() ) ;
    */

} /* end print_pins */
#endif /* DEBUG */
