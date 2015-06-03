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
FILE:	    findcost.c                                       
DESCRIPTION:function calculates initial cost
CONTENTS:   findcost( )
DATE:	    Jan 29, 1988 
REVISIONS:  Oct 22, 1988 - now take square root of the penalty
		since square gives large variance compared to linear
		terms in cost function.  
	    Dec  3, 1988 - added timing driven penalty to cost function.
	    Mar  7, 1989 - added test for totalcells loop.
	    Mar 16, 1989 - rewrote the netlist data structures.
	    Mar 20, 1989 - deleted argument to checkcost.
	    May 11, 1989 - eliminated doPartition flag 
	    Feb  8, 1989 - added test for wraparound for large designs
		using cost only.  Bin penalty doesn't matter.
	    Apr 23, 1990 - added new debug code.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) findcost.c version 3.7 11/23/91" ;
#endif

#include <custom.h>

#include <yalecad/relpos.h>
#include <yalecad/debug.h>

findcost()
{

NETBOXPTR netptr ;
PINBOXPTR pinptr ;
CELLBOXPTR ptr ;
BINBOXPTR bptr ;
GLISTPTR net_of_path ;
INT cell , net ;
INT x , y , cost ;
INT orient ;
INT length, pathcount ;
INT low_length, high_length ;
PATHPTR path ;

cost = 0 ;

/* ************** calculate global coordinates of the pins *************/
for( cell = 1 ; cell <= totalcellsG ; cell++ ) {
    ptr = cellarrayG[cell] ;
    /* avoid work if cell is not custom,soft, or pad cell */
    if( ptr->celltype != CUSTOMCELLTYPE && ptr->celltype != SOFTCELLTYPE
	&& ptr->celltype != PADCELLTYPE ){
	continue ;
    }
    orient = ptr->orient ;
    /* go thru all pins of the cell */
    for( pinptr = ptr->pinptr ; pinptr ; pinptr = pinptr->nextpin ) {

	/* debug - calculate global position for pins */
	/* complain if different from incremental */
	D( "findcost",
	    REL_POS( orient, 
		x, y,                                 /* result */
		pinptr->txpos, pinptr->typos,        /* relative */
		ptr->xcenter, ptr->ycenter ) ;      /* cell center */
	    if( x != pinptr->xpos ){
		D( "findcost/print",fprintf( stderr,"xpos incorrect\n")) ;
	    } else if( y != pinptr->ypos ){
		D( "findcost/print",fprintf( stderr,"ypos incorrect\n")) ;
	    }
	) ; /* end debug macro */
	    

	/* rel pos is a macro which calculates absolute pin location */
	/* defined in relpos.h */
	REL_POS( orient, 
	    pinptr->xpos, pinptr->ypos,               /* result */
	    pinptr->txpos, pinptr->typos,            /* cell relative */
	    ptr->xcenter, ptr->ycenter ) ;            /* cell center */
    }

} /* end calculation of pin global coordinates */


/* ********* calculate net half perimeter bounding box *********** */
for( net = 1 ; net <= numnetsG ; net++ ) {
    netptr =  netarrayG[net] ;
    if(!(netptr)) {
	continue ;
    }
    if( netptr->skip == 1 ) {
	continue ;
    }

    /* for debug initialize the new fields so we can see difference */
    D( "findcost/netcalc",
	netptr->newxmin = netptr->xmin ;
	netptr->newymin = netptr->ymin ;
	netptr->newxmax = netptr->xmax ;
	netptr->newymax = netptr->ymax ;
    ) ;

    /* find first pin that we don't have skip field set */
    /* initialize bounding box pin count to 1 */
    for( pinptr = netptr->pins;pinptr; pinptr = pinptr->next ) {
	if( pinptr->skip == 1 ) {
	    continue ;
	}
	netptr->xmin = netptr->xmax = pinptr->xpos ;
	netptr->ymin = netptr->ymax = pinptr->ypos ;
	netptr->Lnum = netptr->Rnum = 1 ;
	netptr->Bnum = netptr->Tnum = 1 ;
	pinptr = pinptr->next ;
	break ;
    }
    /* Now find whether this pin impacts the bounding box */
    /* Note when we get more than one pin on the bounding box */
    for( ; pinptr ; pinptr = pinptr->next ) {
	if( pinptr->skip == 1 ) {
	    continue ;
	}
	x = pinptr->xpos ;
	y = pinptr->ypos ;

	if( x < netptr->xmin ) {
	    netptr->xmin = x ;
	    netptr->Lnum = 1 ;
	} else if( x == netptr->xmin ) {
	    netptr->Lnum++ ;
	    if( x == netptr->xmax ) {
		netptr->Rnum++ ;
	    }
	} else if( x > netptr->xmax ) {
	    netptr->xmax = x ;
	    netptr->Rnum = 1 ;
	} else if( x == netptr->xmax ) {
	    netptr->Rnum++ ;
	}
	if( y < netptr->ymin ) {
	    netptr->ymin = y ;
	    netptr->Bnum = 1  ;
	} else if( y == netptr->ymin ) { 
	    netptr->Bnum++ ;
	    if( y == netptr->ymax ) {
		netptr->Tnum++ ;
	    }
	} else if( y > netptr->ymax ) {
	    netptr->ymax = y ;
	    netptr->Tnum = 1  ;
	} else if( y == netptr->ymax ) {
	    netptr->Tnum++ ;
	}
    }
    /* calculate cost using a vertical wire weight */
    cost += netptr->halfPx = netptr->newhalfPx = 
        netptr->xmax - netptr->xmin ;
    netptr->halfPy = netptr->newhalfPy = 
	netptr->ymax - netptr->ymin ;

    cost = cost + (INT)( vertical_wire_weightG * (DOUBLE) netptr->halfPy ) ;

    /* check to make sure calculation jives */
    D( "findcost/netcalc",
	if( netptr->newxmin != netptr->xmin ){
	    D( "findcost/print",fprintf( stderr,"netptr->xmin wrong\n"));
	}
	if( netptr->newxmax != netptr->xmax ){
	    D( "findcost/print",fprintf( stderr,"netptr->xmax wrong\n"));
	}
	if( netptr->newymin != netptr->ymin ){
	    D( "findcost/print",fprintf( stderr,"netptr->ymin wrong\n"));
	}
	if( netptr->newymax != netptr->ymax ){
	    D( "findcost/print",fprintf( stderr,"netptr->ymax wrong\n"));
	}
    ) ;

} /* end half perimeter bounding box calculation */



/* now calculate the penalties */
/* first the timing penalty */
timingpenalG = 0 ;
for( pathcount = 1 ; pathcount <= numpathsG ; pathcount++ ) {

    path = patharrayG[pathcount] ;
    ASSERTNCONT( path, "findcost", "pointer to path is NULL" ) ;
    low_length = 0 ;
    high_length = 0 ;
    /* -----------------------------------------------------------------
        For all nets k of a path i:
	    We use the minimum strength driver for each net to calculate
	    the lower bound on the length and the maximum strength driver
	    for the upper bound on the length.  The user must take false
	    paths into account when specifying the driver strengths.
    ------------------------------------------------------------------ */
    for( net_of_path=path->nets;net_of_path;
	net_of_path=net_of_path->next ){
	net = net_of_path->p.net ;
	netptr = netarrayG[net] ;
	/* accumulate length of path */
	length = netptr->halfPx + (INT)(vertical_path_weightG * 
		(DOUBLE) netptr->halfPy) ;

	low_length = low_length + (INT) 
		(netptr->max_driver * (FLOAT) length +
			netptr->driveFactor ) ;
	high_length = high_length + (INT) 
		(netptr->min_driver * (FLOAT) length +
			netptr->driveFactor ) ;
    }
    /* save result */
    path->lo_path_len = path->new_lo_path_len = low_length ;
    path->hi_path_len = path->new_hi_path_len = high_length ;

    /* -----------------------------------------------------------
        Calculate penalty - no penalty if within target window:
    	    lowerbound <= length <= upperbound
        Otherwise calculate penalties 
    ------------------------------------------------------------- */
    if( high_length > path->upper_bound ){
	timingpenalG += high_length - path->upper_bound ; 
    } else if( low_length < path->lower_bound ){
	timingpenalG += path->lower_bound - low_length ;
    }
}
/* scale timing penalty */
timingcostG = (INT) ( timeFactorG * (DOUBLE) timingpenalG ) ;

/* next the overlap penalty */
binpenalG = 0 ;
for( x= 0 ; x <= maxBinXG; x++ ) {
    for( y = 0 ; y <= maxBinYG; y++ ) {
	bptr = binptrG[x][y] ;
	binpenalG += ABS( bptr->penalty ) ;
    }
}
/* wrap around case */
if( binpenalG < 0 ){
    binpenalG = INT_MAX ;
}
/* scale penalty */
penaltyG = (INT) ( lapFactorG * sqrt( (DOUBLE) binpenalG ) ) ;

return( cost ) ;

} /* end findcost */

#ifdef DEBUG
checkcost()
{

    INT incr_funccost, incr_penalty, incr_time ;
    INT incr_overpenal, incr_timepen ;
    INT x, y ;
    BOOL flag ;

    /* verify incremental cost equals the true cost */
    /* save value of the current costs derived from incremental calculations */
    DS( incr_funccost = funccostG ; ) ;
    DS( incr_penalty = penaltyG ; ) ;
    DS( incr_overpenal = binpenalG ; ) ;
    DS( incr_time = timingcostG ; ) ;
    DS( incr_timepen = timingpenalG ; ) ;
    /* calculate current cost from scratch */
    /* verify current cost == incremental cost */
    funccostG = findcost() ;
    if( funccostG != incr_funccost ){
	M( ERRMSG,"checkcost",
	    "Incremental wire cost does not equal current cost\n" ) ;
    }
    if( penaltyG != incr_penalty ){
	M( ERRMSG, "checkcost",
	"Incremental overlap penalty does not equal current penalty\n" ) ;
    }
    if( binpenalG != incr_overpenal ){
	M( ERRMSG, "checkcost",
	"Incremental overlap penalty does not equal current penalty\n" ) ;
    }
    if( timingcostG != incr_time ){
	M( ERRMSG, "checkcost",
	"Incremental timing penalty does not equal current timing\n" ) ;
    }
    if( timingpenalG != incr_timepen ){
	M( ERRMSG, "checkcost",
	"Incremental timing penalty does not equal current timing\n" ) ;
    }
    D( "checkcost",
	/* drastic test */
	incr_penalty = penaltyG ;
	incr_overpenal = binpenalG ;
	flag = FALSE ;

	loadbins( flag ) ;
	for( x = 0; x<= maxBinXG; x++){
	    for( y = 0; y<= maxBinYG; y++){
		binptrG[x][y]->nupenalty = 0 ;
	    }
	}
	if( penaltyG != incr_penalty ){
	M( ERRMSG, "checkcost",
	    "Incremental overlap penalty != current penalty\n" ) ;
	}
	if( binpenalG != incr_overpenal ){
	M( ERRMSG, "checkcost",
	    "Incremental overlap penalty != current penalty\n" ) ;
	}
    ) ;
} /* end checkcost */

#endif /* DEBUG */
