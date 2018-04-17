/*
 *   Copyright (C) 1988-1991 Yale University
 *   Copyright (C) 2014 Ruben Undheim <ruben.undheim@gmail.com>
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
	    Sun Jan 20 21:34:36 PST 1991 - ported to AIX.
	    Wed Feb 13 23:55:50 EST 1991 - now call placepads program.
	    Sat Feb 23 00:17:28 EST 1991 - added placepads algorithm.
	    Sun May  5 14:30:08 EDT 1991 - after setVirtual switch
		is turned on update the block variables.
	    Thu Sep 19 16:33:58 EDT 1991 - fixed problem with sidespace
		options when only a fraction of the pins to pads have
		connections to the core.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) placepads.c version 3.14 11/23/91" ;
#endif

#include <custom.h>
#include <pads.h>
#include <dens.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>
#include <yalecad/string.h>
#include <yalecad/set.h>

#include "config-build.h"

#define PLACEPADPROG      "placepads"
#define PLACEPADPATH      "../placepads"
#define PADKEYWORD        "pad"

/* ***************** STATIC FUNCTION DEFINITIONS ******************* */
static find_optimum_locations( P1(void) ) ;
static place_pad( P2(PADBOXPTR pad,INT bestside ) ) ;
static place_children( P5(PADBOXPTR pad,INT side,DOUBLE lb,DOUBLE ub,BOOL sr) ) ;
static INT find_cost_for_a_side(P5(PADBOXPTR pad,INT side,DOUBLE lb,DOUBLE ub,
   BOOL spacing_restricted ) ) ;
static find_core( P1(void) ) ;

/* ***************** STATIC VARIABLE DEFINITIONS ******************* */
static INT sumposS ; /* sum of the modified opt. pin pos. of pad pins */
static INT sumtieS ; /* sum of all the opt. pin pos. of pad pins */
static INT pin_countS ; /* number of pins found with valid connections */
static BOOL virtualCoreS = FALSE ;
static YSETPTR pad_net_setS = NIL(YSETPTR) ;
static BOOL external_pad_programS = FALSE ;




/*-------------------------------------------------------------------
 placepads() now call placepads program or uses internal algorithm.
 If you convert from placepads program remember to change numterms
 field to endsuper.
____________________________________________________________________*/

placepads()
{

    if( padspacingG == EXACT_PADS ){
	/* no work to do */
	return ;
    }

    find_core();  /* GET THE UPDATED CORE'S DIMENSION */

    if( external_pad_programG ){
	call_place_pads() ;
	funccostG = findcost() ;
	return ;
    }

    /* otherwise do the internal routines */
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

    funccostG = findcost() ;

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
	    for (side = 1; side <= 4; side++ ) {
		if( pad->valid_side[ALL] || pad->valid_side[side] ){
		    cost = find_cost_for_a_side( pad,side,
			(DOUBLE) pad->lowerbound, (DOUBLE) pad->upperbound,
			pad->fixed ) ;
		    if( cost < bestcost) {
			bestcost = cost;
			bestside = side ;
		    }
		}
	    }
	    place_children( pad, bestside, (DOUBLE) pad->lowerbound, 
		(DOUBLE) pad->upperbound, pad->fixed ) ;

	} else if( pad->padtype == PADCELLTYPE && pad->hierarchy == NONE ) {
	    /* the case of a pad that is not in a padgroup */
	    bestcost = INT_MAX ;
	    for (side = 1; side <= 4; side++ ) {
		if( pad->valid_side[ALL] || pad->valid_side[side] ){
		    cost = find_cost_for_a_side( pad,side,
			(DOUBLE) pad->lowerbound, (DOUBLE) pad->upperbound, 
			pad->fixed ) ;
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


    /**** FOR NORMAL PADS AND LEAVES, JUST CALCULATE THE COST */
    /*** AND POSITION. THE LEAF CASE IS THE RETURN CONDITION OF */
    /*** THE RECURSION ON THE ROOT PADS ***/

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

	for ( pinptr = cellarrayG[pad->cellnum]->pinptr ;pinptr; pinptr = pinptr->nextpin) {
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


		if( netterm->cell <= endsuperG ) {
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
		}
		if (dist < bestdist) {
		    bestdist = dist;    /*** UPDATE THE BEST DISTANCE */
		    bestpos  = pos;     /*** AND BEST POSITION        */
		    besttie = tiebreak; /* save the original position */
		    pinFound = TRUE ;   /* pin on this net was found */
		}
	    } /* end looking at this net */

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
    CELLBOXPTR cptr ;
    
    fprintf( stderr, "\n%s\n", message ) ;

    /* now print them out */
    for( i = 1 ; i <= howmany; i++ ){
	ptr = array[i] ;
	cptr = cellarrayG[ptr->cellnum] ;
	fprintf( stderr, 
	    "pad:%s x:%d y:%d type:%d side:%d pos:%d tie:%d orient:%d\n",
	    cptr->cname, cptr->xcenter, cptr->ycenter, ptr->hierarchy,
	    ptr->padside, ptr->position, ptr->tiebreak, cptr->orient ) ;
    }
    fprintf( stderr, "\n" ) ;

    dimension_pads() ;
    G( process_graphics() ) ;

} /* end print_pads */

#endif /* DEBUG */
/* ***************************************************************** */


/* turn virtual core on and off */
setVirtualCore( flag )
BOOL flag ;
{
    virtualCoreS = flag ;
} /* end set Virtual core */

/* function finds and returns core boundary region including cells */
/* which overlap the core region */
find_core_boundary( left, right, bottom, top )
INT *left, *right, *bottom, *top ;
{
    BOOL rememberFlag ;

    /* call routine to find core boundary based on virtual core */
    rememberFlag = virtualCoreS ;/* so we can put flag back */
    virtualCoreS = TRUE ;
    find_core() ;

    /* set flag back to whatever it was */
    virtualCoreS = rememberFlag ;

    /* now return values */
    *left = coreG[X][MINI] ;
    *right = coreG[X][MAXI] ;
    *bottom = coreG[Y][MINI] ;
    *top = coreG[Y][MAXI] ;

} /* end find_core_boundary */


/* given a cell it returns bounding box of cell in global coordinates */
get_global_pos( cell, l, b, r, t )
INT cell ; 
INT *l, *r, *b, *t ;
{
    INT orient ;
    BOUNBOXPTR bounptr ;
    CELLBOXPTR ptr ;

    ptr = cellarrayG[cell] ;
    orient = ptr->orient ;
    bounptr = ptr->bounBox[orient] ;
    *l = bounptr->l ;
    *r = bounptr->r ;
    *b = bounptr->b ;
    *t = bounptr->t ;

    /* now add xcenter ycenter to get global position */
    *l += ptr->xcenter ;
    *r += ptr->xcenter ;
    *b += ptr->ycenter ;
    *t += ptr->ycenter ;
} /* end get_global_pos */


/* given a cell it returns bounding box of cell including routing area */
get_routing_boundary( cell, ret_l, ret_b, ret_r, ret_t )
INT cell ; 
INT *ret_l, *ret_r, *ret_b, *ret_t ; /* return quantities */
{
    INT minx, maxx ;
    INT miny, maxy ;
    CELLBOXPTR ptr ;
    RTILEBOXPTR tileptr ;

    
    get_global_pos( cell, ret_l, ret_b, ret_r, ret_t ) ;

    if( !(routingTilesG) ){
	/* return cell boundary if no routing tiles */
	return ;
    }

    /* otherwise find bounding box of routing tiles */
    minx = INT_MAX ;
    miny = INT_MAX ;
    maxx = INT_MIN ;
    maxy = INT_MIN ;
    for( tileptr = routingTilesG[cell];tileptr;tileptr=tileptr->next ){
	minx = MIN( minx, tileptr->x1 ) ; 
	maxx = MAX( maxx, tileptr->x2 ) ; 
	miny = MIN( miny, tileptr->y1 ) ; 
	maxy = MAX( maxy, tileptr->y2 ) ; 
    } 

    /* now add xcenter ycenter to get global position */
    ptr = cellarrayG[cell] ;
    *ret_l = MIN( minx + ptr->xcenter, *ret_l ) ;
    *ret_r = MAX( maxx + ptr->xcenter, *ret_r ) ;
    *ret_b = MIN( miny + ptr->ycenter, *ret_b ) ;
    *ret_t = MAX( maxy + ptr->ycenter, *ret_t ) ;
    return ;
} /* end get_routing_boundary */


static INT get_pad_routing( cell )
INT cell ;
{

    CELLBOXPTR cptr ;            /* pad cell in question */
    RTILEBOXPTR tptr ;            /* look at all the routing tiles */
    INT last_core_cell ;         /* calc last core cell index */
    INT new ;                    /* new position with routing */
    INT old ;                    /* old position with no routing */

    last_core_cell = ( (INT) routingTilesG[HOWMANY] ) - 4 ;

    /* find bounding box of routing tiles */
    cptr = cellarrayG[endpadgrpsG + cell] ;
    tptr = routingTilesG[last_core_cell + cell] ;
    new = 0 ;
    switch( cell ){
    case L:
	for( ; tptr ; tptr = tptr->next ){
	    new = MAX( new, tptr->x2 ) ; 
	}
	old = cptr->tiles->right ;
	break ;
    case T:
	for( ; tptr ; tptr = tptr->next ){
	    new = MIN( new, tptr->y1 ) ; 
	}
	old = cptr->tiles->bottom ;
	break ;
    case R:
	for( ; tptr ; tptr = tptr->next ){
	    new = MIN( new, tptr->x1 ) ; 
	}
	old = cptr->tiles->left ;
	break ;
    case B:
	for( ; tptr ; tptr = tptr->next ){
	    new = MAX( new, tptr->y2 ) ; 
	}
	old = cptr->tiles->top ;
	break ;
    }  /* end switch */

    if( new ){
	return( ABS( new - old ) ) ;
    } 
    return( 0 ) ;
} /* end get_pad_routing */

static find_core()
{
    INT ominx, ominy ;
    INT omaxx, omaxy ;
    INT l, r, b, t ;
    INT i ;

    if( virtualCoreS ){
	/* initialize xmin, ymax, etc. */
	coreG[X][MINI] = INT_MAX ;
	coreG[Y][MINI] = INT_MAX ;
	coreG[X][MAXI] = INT_MIN ;
	coreG[Y][MAXI] = INT_MIN ;

	/* reset bounding boxes for all cells for get_global_pos */
	regenorient(1, numcellsG) ;

	/* find virtual boundary of core */
	for( i=1;i<=numcellsG;i++ ){

	    if( cellarrayG[i]->celltype == STDCELLTYPE && doPartitionG ){
		/* ignore standard cell types */
		continue ;
	    }

	    if( doPartitionG ){
		get_global_pos( i, &l, &b, &r, &t ) ;
	    } else {
		get_routing_boundary( i, &l, &b, &r, &t ) ;
	    }
	    coreG[X][MINI] = MIN( coreG[X][MINI], l ) ;
	    coreG[X][MAXI] = MAX( coreG[X][MAXI], r ) ;
	    coreG[Y][MINI] = MIN( coreG[Y][MINI], b ) ;
	    coreG[Y][MAXI] = MAX( coreG[Y][MAXI], t ) ;

	}
	if( doPartitionG ){
	    coreG[X][MINI] = MIN( coreG[X][MINI], blocklG ) ;
	    coreG[X][MAXI] = MAX( coreG[X][MAXI], blockrG ) ;
	    coreG[Y][MINI] = MIN( coreG[Y][MINI], blockbG ) ;
	    coreG[Y][MAXI] = MAX( coreG[Y][MAXI], blocktG ) ;
	}
	/* set block parameters once virtual core is set */
	blocklG = coreG[X][MINI] ;
	blockrG = coreG[X][MAXI] ;
	blockbG = coreG[Y][MINI] ;
	blocktG = coreG[Y][MAXI] ;
    } else {
	coreG[X][MINI] = blocklG ;
	coreG[X][MAXI] = blockrG ;
	coreG[Y][MINI] = blockbG ;
	coreG[Y][MAXI] = blocktG ;
    }

    /* now guarantee space between core and pads */
    /* if global router info not available */
    if( !(routingTilesG) ){
	coreG[X][MINI] -= track_spacingXG ;
	coreG[Y][MINI] -= track_spacingYG ;
	coreG[X][MAXI] += track_spacingXG ;
	coreG[Y][MAXI] += track_spacingYG ;
    } else if(!(doPartitionG)){
	/* find delta in pad spacing */
	coreG[X][MINI] -= get_pad_routing( L ) ;
	coreG[Y][MAXI] += get_pad_routing( T ) ;
	coreG[X][MAXI] += get_pad_routing( R ) ;
	coreG[Y][MINI] -= get_pad_routing( B ) ;
    }

    /* if the grid is given force to grid positions */
    if( gridCellsG ){
	ominx = coreG[X][MINI] ;
	ominy = coreG[Y][MINI] ;
	omaxx = coreG[X][MAXI] ;
	omaxy = coreG[Y][MAXI] ;
	YforceGrid( &coreG[X][MINI], &coreG[Y][MINI] ) ;
	YforceGrid( &coreG[X][MAXI], &coreG[Y][MAXI] ) ;

	/* make sure gridded data is outside of core to avoid overlap */
	if( ominx < coreG[X][MINI] ){
	    coreG[X][MINI] -= track_spacingXG ;
	}
	if( ominy < coreG[Y][MINI] ){
	    coreG[Y][MINI] -= track_spacingYG ;
	}
	if( omaxx > coreG[X][MAXI] ){
	    coreG[X][MAXI] += track_spacingXG ;
	}
	if( omaxy > coreG[Y][MAXI] ){
	    coreG[Y][MAXI] += track_spacingYG ;
	}
    }
    perdimG[X] = coreG[X][MAXI] - coreG[X][MINI] ;
    perdimG[Y] = coreG[Y][MAXI] - coreG[Y][MINI] ;


} /* end find_core */


/* ***********************EXTERNAL ROUTINES ************************** */
call_place_pads()
{
    FILE *fp ;
    INT pad ;
    INT line ;
    INT numnets ;
    INT numtokens ;
    INT closegraphics() ;
    INT find_numnets() ;
    BOOL abort ;
    char **tokens ;
    char *bufferptr ;
    char *pathname, *Yrelpath() ;
    char *twdir, *Ygetenv() ;
    char filename[LRECL] ;
    char buffer[LRECL] ;
    CELLBOXPTR cellptr ;

    /* build the input file for the placepads program */
    sprintf( filename, "%s.pads", cktNameG );
    fp = TWOPEN( filename, "w", ABORT ) ;
    numnets = find_numnets() ;
    fprintf( fp, "core %d %d %d %d\n", 
	coreG[X][MINI], coreG[Y][MINI], coreG[X][MAXI], coreG[Y][MAXI] ) ;
    fprintf( fp, "pads %d numnets %d\n", numpadsG+numpadgroupsG, numnets);
    output_pads( fp ) ;
    output_nets( fp, numnets ) ;
    TWCLOSE( fp ) ;

    /* now call the placepad program */
    /* find the path of placepads relative to main program */
    pathname = Yrelpath( argv0G, PLACEPADPATH ) ;
    if( !(YfileExists(pathname))){
	if( twdir = TWFLOWDIR ){
	    sprintf( filename, "%s/bin/%s", twdir, PLACEPADPROG ) ;
	    pathname = Ystrclone( filename ) ;
	}
    }
    switch( padspacingG ){
	case ABUT_PADS:
	    sprintf( YmsgG, "%s -asn %s", pathname, cktNameG ) ;
	    break ;
	case UNIFORM_PADS:
	    sprintf( YmsgG, "%s -usn %s", pathname, cktNameG ) ;
	    break ;
	case VARIABLE_PADS:
	    sprintf( YmsgG, "%s -osn %s", pathname, cktNameG ) ;
	    break ;
    }
    M( MSG, NULL, YmsgG ) ;
    M( MSG, NULL, "\n" ) ;

    /* Ysystem will kill program if catastrophe occurred */
    Ysystem( PLACEPADPROG, ABORT, YmsgG, closegraphics ) ; 
    Ysafe_free( pathname ) ; /* free name created in Yrelpath */
    /* ############# end of placepads execution ############# */

    /* **************** READ RESULTS of placepads ************/
    /* open pad placement file for reading */
    M( MSG, NULL, "Reading results of placing pads...\n" ) ;
    sprintf(filename, "%s.pout" , cktNameG ) ;
    fp = TWOPEN( filename , "r", ABORT ) ;

    /* parse file */
    line = 0 ;
    abort = FALSE ;
    pad = endsuperG ;
    while( bufferptr=fgets(buffer,LRECL,fp )){
	/* parse file */
	line ++ ; /* increment line number */
	tokens = Ystrparser( bufferptr, ": \t\n", &numtokens );

	if( numtokens == 0 ){
	    /* skip over empty lines */
	    continue ;
	} else if( strcmp( tokens[0], PADKEYWORD ) == STRINGEQ){
	    /* look at first field for keyword */
	    /* ie. pad <string> x y orient padside */
	    if( numtokens != 6 ){
		sprintf( YmsgG, "Syntax error on line:%d\n", line ) ;
		M(ERRMSG, "placepads", YmsgG ) ;
		abort = TRUE ;
		continue ;
	    }
	    pad++ ;
	    ASSERTNBREAK( pad > 0 && pad <= endpadsG, "placepads",
		"pad out of bounds\n" ) ;
	    cellptr = cellarrayG[pad] ;
	    cellptr->xcenter = atoi(tokens[2] ) ;
	    cellptr->ycenter = atoi(tokens[3] ) ;
	    cellptr->orient = atoi(tokens[4] ) ;
	    cellptr->padptr->padside = atoi(tokens[5] ) ;

	} else {
	    sprintf( YmsgG, "Syntax error on line:%d\n", line ) ;
	    M(ERRMSG, "placepads", YmsgG ) ;
	    abort = TRUE ;
	    continue ;
	}
    }
    TWCLOSE( fp ) ;

    if( abort ){
	M(ERRMSG, "placepads", "Problem with placing pads.Must abort\n") ;
	closegraphics() ;
	YexitPgm( PGMFAIL ) ;
    }
    /* ************ END READ RESULTS of placepads ************/

} /* end call_place_pads */

INT find_numnets()
{
    INT pad ; 
    INT numnets ; 
    CELLBOXPTR ptr ;
    PINBOXPTR pin ;

    if(!(pad_net_setS) ){
	pad_net_setS = Yset_init( 1, numnetsG ) ;
    }
    Yset_empty( pad_net_setS ) ;
    numnets = 0 ;
    for( pad = endsuperG; pad <= endpadsG; pad++ ){
	ptr = cellarrayG[ pad ] ;
	for( pin = ptr->pinptr; pin ; pin = pin->nextpin ){
	    if( netarrayG[ pin->net ]->numpins <= 1 ){
		continue ;
	    }
	    if( Yset_add( pad_net_setS, pin->net ) ){
		/* Yset_add returns TRUE if a new member of set */
		numnets++ ;
	    }
	}
    }
    return( numnets ) ;
} /* end find_numnets */

output_nets( fp, numnets )
FILE *fp ;
{
    INT net ;
    INT pincount ;
    PINBOXPTR pin ;
    NETBOXPTR netptr ;
    YSETLISTPTR list ;

    for( list = Yset_enumerate(pad_net_setS) ;list; list = list->next ){
	net = list->node ;
	netptr = netarrayG[ net ] ;
	fprintf( fp, "net %s ", netptr->nname ) ;
	pincount = 0 ;
	for( pin = netptr->pins; pin ; pin = pin->next ){
	    if( pin->cell > endsuperG ){
		/* skip over the pad connections */
		continue ;
	    }
	    fprintf( fp, "%d %d ", pin->xpos, pin->ypos ) ;
	    if( ++pincount > 5 ){
		pincount = 0 ;
		fprintf( fp, "\n" ) ;
	    }
	}
	if( pincount != 0 ){
	    fprintf( fp, "\n" ) ;
	}
    }
} /* end output_nets */
