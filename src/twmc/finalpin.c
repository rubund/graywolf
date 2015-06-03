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
FILE:	    finalpin.c                                       
DESCRIPTION:main routine
DATE:	    Jan 29, 1988 
REVISIONS:  Feb  7, 1990 - complete rewrite of finalpin. Now uses
		low temperature anneal.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) finalpin.c version 3.6 4/3/92" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

#define HOWMANY 0
#define EXPECTEDNUMSITES  500


finalpin()
{

}


#ifdef LATER

static DOUBLE aspFactorS ;         /* used to calculate diff from orig */
static INT oleftS ;                /* used to calculate diff from orig */
static INT obottomS ;              /* used to calculate diff from orig */
static INT Hdiv2S ;                /* used to calculate diff from orig */
static INT Wdiv2S ;                /* used to calculate diff from orig */
static INT instS ;                 /* used to calculate diff from orig */

/* perform a low temperature anneal on pins */
final_pin_place()
{

    INT i ;               /* counter */
    INT attempts ;        /* number of moves made */
    INT nummoves ;        /* number of moves to do on a cell */
    INT selection ;       /* select a cell with softpins */
    INT nsoftpin_cells ;  /* number of cells with softpins */
    CELLBOXPTR acellptr;  /* current cell with softpins */

    /* don't perform if cost only is specified in input file */
    /* scale data variable is necessary for recursive TimberWolfMC call */
    if( /* cost_onlyG || */ scale_dataG > 1 ){
	return ;
    }

    /* now check to see if we have any soft pins if not return */
    if( (nsoftpin_cells = (int) softPinArrayG[HOWMANY] ) == 0 ){
	return ;
    }

    /* make new site arrays for pins */
    for( i = 1; i <= nsoftpin_cells; i++ ){
	update_sites( softPinArrayG[i] ) ;
    }
    findcost() ;

    /* continue with a low Temp anneal */
    TG = 10.0 ;
    attempts = 0 ;
    if( overpenalG ){  
	/* make pinFactor 1 order more important than wirelength */
	pinFactorG =  10.0 * (DOUBLE) funccostG / (DOUBLE) overpenalG ;
	/* also set softPinArrayG to look at cells with overlap */
    } else {
	/* otherwise use previous pinFactorG */
	/* make 1 order more important */
	pinFactorG *= 10.0 ;
    }

    while( attempts < attmaxG ) {

	/* to make pin moves more efficient, use softPinArrayG */
	/* which keeps track of all softcells which have pins */
	/* which can move. softPinArrayG[0] holds size of array */
	selection = PICK_INT( 1, (int) softPinArrayG[HOWMANY] );
	/* now get cellptr */
	acellptr = softPinArrayG[selection] ;
	/* pick number of pins moves to be attempted */
	/* PIN_MOVE is beginning of sequence.  */
	if( overpenalG && !(doPartitionG) ){  
	    /* if a penalty exists do many moves */
	    nummoves = acellptr->numsoftpins ;
	} else {
	    nummoves = 1 ; /* no penalty try to reduce wirelen */
	}

	/* *********** NOW EVALUATE THE PROPOSED MOVE ********* */
	/* now try softpin moves */
	for( i=1; i<= nummoves; i++ ){
	    selectpin( acellptr ) ;
	}
	/* *********** END OF PROPOSED MOVE  EVALUATION ********* */
	attempts++ ;

	D( "finalpin", checkcost() ) ; /* if debug on check cost after each move */

    } /* ****** END OF ANNEALING LOOP **************** */

    /* verify incremental and current costs after each iteration */
    D( "finalpin", checkcost() ) ;

    /* ----------------------------------------------------------------- 
       now output statistics for this temperature.
    */

    OUT1("\n\nPin place optimizer\n");
    OUT1("\nI     T     funccost  overpen  x pinFact = overfill pinflips\n");
    OUT2("%3d ",iterationG ); 
    OUT2("%4.2le ",TG ); 
    OUT2("%4.2le ",(DOUBLE) funccostG ); 
    OUT2("%4.2le ",(DOUBLE) overpenalG ); 
    OUT2("%4.2le ",(DOUBLE) pinFactorG ); 
    OUT2("%4.2le ",(DOUBLE) overfillG ); 
    OUT3("%3d/%3d\n\n",flippG,attpG ); 
    FLUSHOUT() ;

    return ;
} /* end final_pin_place */
/* ***************************************************************** */


update_sites( cellptr )
CELLBOXPTR cellptr ;
{

    INT i ;                       /* counter */
    INT j ;                       /* counter */
    INT k ;                       /* counter */
    INT site ;                    /* current site */
    INT numsites ;                /* number of sites for the cell */
    INT span_side ;               /* find total length of cell side */
    INT l, r, b, t ;              /* left, right, bottom, top of tile */
    INT grid ;                    /* grid the sites */
    INT save_x ;                  /* save the x offset */
    INT save_y ;                  /* save the y offset */
    INT trueSpan ;                /* span modified by aspect ratio */
    INT oldx, oldy ;              /* traverse the cells point list */
    INT newx, newy ;              /* traverse the cells point list */
    INT numpoints ;               /* number of points in arb rect fig */
    INT tile_count ;              /* count the number of tiles */
    INT HnotV ;                   /* TRUE horizontal edge FALSE vert */
    INT side ;                    /* current side of the cell */
    INT start ;                   /* where to start the sites on a side */
    INT stop ;                    /* where to stop the sites on a side */
    INT span ;                    /* span of this side */
    INT numsoftpins ;             /* number of soft pins for cell */
    INT sites_this_side ;         /* number of sites this side */
    INT firstSite ;               /* first site on the current side */
    INT lastSite ;                /* last site on the current side */
    INT newfirstSite ;            /* first site on this new side */
    INT newlastSite ;             /* last site on this new side */
    INT lastPin ;                 /* last pin on the current side */
    BOOL RTnotLB ;                /* TRUE right top FALSE edge L or B */
    CONTENTPTR *sitePtr ;         /* pointer to array of site for cell */
    CONTENTPTR this_site ;        /* current site record */
    CONTENTPTR *free_site ;       /* used to free old site array */
    TILEBOXPTR tileptr ;          /* look at tiles of the cell */
    BUSTBOXPTR PtsOut ;           /* contains the pts of arb rect fig. */
    BUSTBOXPTR unbust() ;         /* returns the vertex list of cell */
    SIDEBOXPTR side_array ;       /* the sites of a given side */
    PINBOXPTR  *curSidePins ;     /* the pins on the current side */
    PINBOXPTR  pin ;              /* current pin */
    SOFTBOXPTR softinfo ;         /* soft information on pin */
    static INT sort_softpins() ;  /* sort pins on a side */
    static CONTENTPTR *check_alloc() ;/* check allocation of site pointer */

    
    side_array = (SIDEBOXPTR) 
	Ysafe_malloc( (cellptr->numsides + 1) * sizeof(SIDEBOX) ) ;
    numsoftpins = cellptr->numsoftpins ;

    init_hard_struct( cellptr ) ;

    initPts(FALSE) ; /* no need to add extra points created with buster */
    tile_count = 0 ;
    for( tileptr = cellptr->tiles;tileptr;tileptr = tileptr->next ) {
	l = tileptr->orig_left ;
	r = tileptr->orig_right ;
	b = tileptr->orig_bottom ;
	t = tileptr->orig_top ;
	addPts( ++tile_count, l , r , b , t ) ;
    }

    PtsOut = unbust() ;

    numpoints = PtsOut[HOWMANY].xc ;

    ASSERTNRETURN(  numpoints > 0, NULL, "numpoints <= 0\n" ) ;
    ASSERTNRETURN((numpoints % 2)==0,NULL,"numpoints not multiple 2\n" ) ;

    /* remember old offset in order to set it back when done */
    /* grid doesn't matter always the same */
    Yget_gridx( &grid, &save_x ) ; 
    Yget_gridy( &grid, &save_y ) ; 


    sitePtr = check_alloc( NULL, EXPECTEDNUMSITES ) ;
  
    numsites = 0 ;
    side = 0 ;
    for( k = 1 ; k <= numpoints; k++ ) {

	oldx = PtsOut[k].xc ;
	oldy = PtsOut[k].yc ;
	if( k == 1 ){
	    /* now set grid from this point */
	    Yset_gridx( track_spacingXG, oldx ) ;
	    Yset_gridy( track_spacingYG, oldy ) ;
	}

	if( k == numpoints ){
	    newx = PtsOut[1].xc ;
	    newy = PtsOut[1].yc ;
	} else {
	    newx = PtsOut[k+1].xc ;
	    newy = PtsOut[k+1].yc ;
	}

	/* new side */
	side++ ;

      	/* now check the direction */
        if( oldy == newy ){
	    HnotV = TRUE ;

	    if( oldx < newx ){
		/* side going to the right */
		RTnotLB = TRUE ;
		/* calculate valid first locations */
		start = oldx ;
		stop = newx ;
		Ygridx_up( &start ) ;
		Ygridx_down( &stop ) ;

		/* avoid pins at the corners */
		if( start == oldx ){
		    /* at the start of the edge */
		    start += track_spacingXG ;
		}
		if( stop == newx ){
		    /* at the end of the edge */
		    stop -= track_spacingXG ;
		}

	    } else {
		/* side going to the left */
		RTnotLB = FALSE ;
		/* calculate valid first locations */
		start = newx ;
		stop = oldx ;
		Ygridx_up( &start ) ;
		Ygridx_down( &stop ) ;

		/* avoid pins at the corners */
		if( start == newx ){
		    /* at the start of the edge */
		    start += track_spacingXG ;
		}
		if( stop == oldx ){
		    /* at the end of the edge */
		    stop -= track_spacingXG ;
		}
	    }

	    /* now find the span of the side */
	    span = ABS( stop - start ) ;
	    sites_this_side = (span / track_spacingXG) + 1 ;
	    firstSite = numsites + 1 ;
	    numsites += sites_this_side ;

	    sitePtr = check_alloc( sitePtr, numsites ) ;

	    /* store this information in side_array */
	    side_array[side].firstSite = firstSite ;
	    side_array[side].lastSite = numsites ;

	    /* now set the sites */
	    for( site = firstSite ; site <= numsites; site++ ){
		this_site = sitePtr[site] ;
		this_site->capacity = 1 ;
		this_site->HorV = 0 ; /* 1 means horizontal */
		this_site->span = track_spacingXG ;
		this_site->ypos = this_site->ypos_orig = oldy ;
		if( RTnotLB ){
		    /* to the right */
		    this_site->xpos = this_site->xpos_orig =
			start + (site - firstSite) * track_spacingXG ;
		} else {
		    /* to the bottom */
		    this_site->xpos = this_site->xpos_orig =
			stop - (site - firstSite) * track_spacingXG ;
		}
	    }

	} else if ( oldx == newx ){
	    HnotV = FALSE ;

	    if( oldy < newy ){
		/* side going to the top */
		RTnotLB = TRUE ;
		/* calculate valid first locations */
		start = oldy ;
		stop = newy ;
		Ygridy_up( &start ) ;
		Ygridy_down( &stop ) ;

		/* avoid pins at the corners */
		if( start == oldy ){
		    /* at the start of the edge */
		    start += track_spacingYG ;
		}
		if( stop == newy ){
		    /* at the end of the edge */
		    stop -= track_spacingYG ;
		}

	    } else {
		/* side going to the bottom */
		RTnotLB = FALSE ;

		/* calculate valid first locations */
		start = newy ;
		stop = oldy ;
		Ygridy_up( &start ) ;
		Ygridy_down( &stop ) ;

		/* avoid pins at the corners */
		if( start == newy ){
		    /* at the start of the edge */
		    start += track_spacingXG ;
		}
		if( stop == oldy ){
		    /* at the end of the edge */
		    stop -= track_spacingYG ;
		}
	    }

	    /* now find the span of the side */
	    span = ABS( stop - start ) ;
	    sites_this_side = (span / track_spacingYG) + 1 ;
	    firstSite = numsites + 1 ;
	    numsites += sites_this_side ;

	    sitePtr = check_alloc( sitePtr, numsites ) ;

	    /* store this information in side_array */
	    side_array[side].firstSite = firstSite ;
	    side_array[side].lastSite = numsites ;

	    /* now set the sites */
	    for( site = firstSite ; site <= numsites; site++ ){
		this_site = sitePtr[site] ;
		this_site->capacity = 1 ;
		this_site->HorV = 1 ; /* 1 means vertical */
		this_site->span = track_spacingYG ;
		this_site->xpos = this_site->xpos_orig = oldx ;
		if( RTnotLB ){
		    /* to the top */
		    this_site->ypos = this_site->ypos_orig =
			start + (site - firstSite) * track_spacingYG ;
		} else {
		    /* to the bottom */
		    this_site->ypos = this_site->ypos_orig =
			stop - (site - firstSite) * track_spacingYG ;
		}
	    }

	} else {
	    M( ERRMSG, "update_sites", "problem with unbust\n" ) ;
	    return ;
	}
    }

    if( numsoftpins > numsites ){
	M( ERRMSG, "update_sites", 
	    "There are more pins than valid locations\n" ) ;
	sprintf( YmsgG, "\t - %d sites < %d pins for cell:%d\n",
	numsites,numsoftpins);
	OUT1( YmsgG ) ;
	M( MSG, NULL, YmsgG ) ;
	M( MSG, NULL, 
	    "\tSome pins will be placed with design rule violations\n" ) ;
    }
	
    curSidePins = (PINBOXPTR *) 
	Ysafe_calloc( numsoftpins+1,sizeof(PINBOXPTR) );

    /* initialize all the pins to not placed use the newsite field */
    for( j = 1; j <= numsoftpins; j++ ){
	pin = cellptr->softpins[j] ; 
	softinfo = pin->softinfo ;
	softinfo->newsite = FALSE ;
    }
    for( side = 1; side <= cellptr->numsides ; side++ ){

	/* find first Site of this side */
	firstSite = cellptr->sideArray[side]->firstSite ; 
	lastSite  = cellptr->sideArray[side]->lastSite ; 
	newfirstSite= side_array[side].firstSite ;
	newlastSite= side_array[side].lastSite ;

	/* now find all the pins on this side */
	lastPin = 0 ;
	for( j = 1; j <= numsoftpins; j++ ){
	    pin = cellptr->softpins[j] ; 
	    softinfo = pin->softinfo ;
	    site = softinfo->site ;
	    if( site >= firstSite && site <= lastSite ){
		/* pin belongs on this side */
		curSidePins[++lastPin] = pin ;
	    }
	} /* end loading all the pins on this side */

	/* calculate the number of sites on this side */
	site = newlastSite - newfirstSite + 1 ;
	if( lastPin > site ){
	    sprintf( YmsgG, 
	        "more pins:%d than sites:%d for cell:%s side:%d\n",
		lastPin, site, cellptr->cname, side ) ;
	    M( WARNMSG,"update_sites", YmsgG ) ;
	}
	     
	/* now sort pins so that most important pins are placed first */
	Yquicksort( (char *) &(curSidePins[1]),lastPin, 
	    sizeof(PINBOXPTR), sort_softpins ) ;

	find_closest_site( cellptr, sitePtr, curSidePins, lastPin,
	    newfirstSite, newlastSite ) ;
    }

    /* ********** NOW UPDATE CELL AND FREE MEMORY ********** */
    Ysafe_free( curSidePins ) ;

    /* now replace the side_array and site array in data structures */
    free_site = cellptr->siteContent ;
    for( i = 1; i <= cellptr->numsites; i++ ){
	Ysafe_free( free_site[i] ) ;
    }
    Ysafe_free( cellptr->siteContent ) ;
    cellptr->siteContent = sitePtr ;
    cellptr->numsites = numsites ;

    /* now take care of the site array */
    for( side = 1; side <= cellptr->numsides ; side++ ){
	cellptr->sideArray[side]->firstSite = side_array[side].firstSite ;
	cellptr->sideArray[side]->lastSite  = side_array[side].lastSite ;
    }
    Ysafe_free( side_array ) ;

} /* end update_sites */

find_closest_site( cellptr, sitePtr, curSidePins, numsoftpins, 
    firstSite, lastSite )
CELLBOXPTR cellptr ;
CONTENTPTR *sitePtr ;
PINBOXPTR *curSidePins ;
INT numsoftpins ;
INT firstSite ;
INT lastSite ;
{

    INT i ;                           /* counter */
    INT j ;                           /* counter */
    INT m ;                           /* counter */
    INT seq ;                         /* length of current pin sequence */
    INT cur_site ;                    /* current site */
    INT xpos, ypos ;                  /* position of current pin site */
    INT hit ;                         /* matching site in new array */
    INT sum ;                         /* number of continuous open sites*/
    INT seqcount ;                    /* number of continuous open sites*/
    INT seqsite ;                     /* place to put start of seq. */
    INT psite ;                       /* site counter */
    INT distant ;                     /* distance to matching position */
    INT ldist ;                       /* lower dist to match position */
    INT udist ;                       /* upper dist to match position */
    BOOL *ok_to_place ;               /* ok to place seq starting here */
    SOFTBOXPTR softinfo ;             /* softinfo on pin */
    PINBOXPTR pin ;                   /* current pin */
    CONTENTPTR sptr ;                 /* current site pointer */


/* allocate temp space for placing sequences */
ok_to_place = (BOOL *) Ysafe_malloc( (lastSite-firstSite+2)*sizeof(BOOL));

for( j = 1; j <= numsoftpins; j++ ){
    pin = curSidePins[j] ;

    softinfo = pin->softinfo ;
    if( softinfo->newsite ){
	/* newsite is used here to note whether we placed pin already */
	continue ;
    }
    /* find current position of pin */
    cur_site = softinfo->site ;
    sptr = cellptr->siteContent[cur_site] ;
    seq  = softinfo->sequence ;
    if( softinfo->numranges == 0 ){
	/* a hard pin place first using position not site position */
	/* use original location */
	find_new_hard_pos( pin ) ;
	xpos = pin->txpos ;
	ypos = pin->typos ;
    } else {
	/* a soft pin use the site place as a first guess */
	xpos = sptr->xpos ;
	ypos = sptr->ypos ;
    }

    hit = 0 ;
    distant = INT_MAX ;
    /* find best place for pin given current condition */
    if( sptr->HorV ){
	/* vertical edge */

	for( m = firstSite ; m <= lastSite ; m++ ) {
	    /*
	     *  Find the site with the nearest ypos 
	     */
	    if( ABS(sitePtr[m]->ypos - ypos ) < distant ) {
		distant = ABS( sitePtr[m]->ypos - ypos ) ;
		hit = m ;
	    } 
	}

    } else { /* horizontal edge */
	for( m = firstSite ; m <= lastSite ; m++ ) {
	    /*
	     *  Find the site with the nearest xpos 
	     */
	    if( ABS( sitePtr[m]->xpos - xpos ) < distant ) {
		distant = ABS( sitePtr[m]->xpos - xpos ) ;
		hit = m ;
	    } 
	}
    }

    /* NOW THAT WE FOUND THE CORRESPONDING SITE PLACE THE PINS */
    if( hit ) {
	/* see if this position is taken */
	if( seq > 1 ){
	    /* find the number of free spaces above and below this spot */

	    sum = 0 ;
	    for( psite = hit; psite <= lastSite; psite++ ){
		if( sitePtr[psite]->contents != 0 || ++sum >= seq ){
		    break ;
		}
	    }
	    if( sum >= seq ){
		/* enough room place sequence */
		for( seqcount = 0; seqcount < seq; seqcount++ ){
		    set_pin_n_site( sitePtr, pin, hit + seqcount ) ;
		    /* get next pin */
		    pin = termarrayG[pin->pin + 1] ;
		}
	    } else {
		/* ---------------------------------------------------
		    Look up and down for sites that have adjoining open 
		    open sites.  We mark every site that has enough room.
		*/
		seqcount = 0 ;
		for( i = firstSite; i <= lastSite; i++ ){
		    ok_to_place[i-firstSite] = FALSE ;
		} 
		for( i = firstSite; i <= lastSite; i++ ){
	            if( sitePtr[i]->contents == 0 ){
			if( ++seqcount >= seq ){
			    /* firstSite is offset in ok_to_place */
			    ok_to_place[i-firstSite-seq+1] = TRUE ;
			}
		    } else {
			/* reset counter */
			seqcount = 0 ;
		    }
		}
		/* now find the closest site to hit */
		distant = INT_MAX ;
		seqsite = 0 ;
		for( m = firstSite ; m <= lastSite ; m++ ) {
		    /*
		     *  Find the site with the following sites open.
		     */
		    if( ok_to_place[m - firstSite] ){
			if( ABS(m - hit ) < distant ) {
			    distant = ABS( m - hit ) ;
			    seqsite = m ;
			} 
		    }
		}
		if( seqsite ){
		    /* we found a valid site place pins starting here */
		    /* enough room place sequence */
		    for( seqcount = 0; seqcount < seq; seqcount++ ){
			set_pin_n_site( sitePtr, pin, seqsite+seqcount ) ;
			/* get next pin */
			pin = termarrayG[pin->pin + 1] ;
		    }
		} else {
		    /* out of luck place at hit increment the overflow */
		    sprintf( YmsgG, 
			"Sequence overflow for sequence starting with\n");
		    M( ERRMSG, NULL, YmsgG ) ;
		    OUT1( YmsgG ) ;
		    sprintf( YmsgG, " pin:%s cell:%d\n",
			pin->pinname,pin->cell ) ;
		    M( ERRMSG, NULL, YmsgG ) ;
		    OUT1( YmsgG ) ;
		    for( seqcount = 0; seqcount < seq; seqcount++ ){
			set_pin_n_site( sitePtr, pin, hit+seqcount ) ;
			/* get next pin */
			pin = termarrayG[pin->pin + 1] ;
		    }
		}
		
	    }
	} else {
	    /* find closest site that is open */
	    if( sitePtr[hit]->contents == 0 ){
		/* place pin here */
		set_pin_n_site( sitePtr, pin, hit ) ;
	    } else {
		/* look up and down for closest free spot */
		udist = INT_MAX ;
		ldist = INT_MAX ;
		for( psite = hit; psite >= firstSite; psite-- ){
		    if( sitePtr[psite]->contents == 0 ){
			ldist = hit - psite ;
			break ;
		    }
		}
		for( psite = hit; psite <= lastSite; psite++ ){
		    if( sitePtr[psite]->contents == 0 ){
			udist = psite - hit ;
			break ;
		    }
		}
		if( ldist < udist ){
		    /* place at lowest site */
		    set_pin_n_site( sitePtr, pin, hit - ldist ) ;
		} else if( udist < ldist ){
		    /* place at upper site */
		    set_pin_n_site( sitePtr, pin, hit + udist ) ;
		} else if( udist == ldist ){
		    if( udist == INT_MAX ){
			/* no place to go add more to hit site */
			set_pin_n_site( sitePtr, pin, hit ) ;
			sprintf( YmsgG, "Overfill for pin:%s cell:%d\n",
			    pin->pinname, pin->cell) ;
			OUT1( YmsgG ) ;
			M( ERRMSG, NULL, YmsgG ) ;
		    } else {
			/* just pick lower site as default */
			set_pin_n_site( sitePtr, pin, hit - ldist ) ;
		    }
		}
	    }

	}
    } else {
	M( ERRMSG, "find_closest_site", "Problem with finding pinsite\n");
	return ;
    }

} /* end for loop */

Ysafe_free( ok_to_place ) ;

    
} /* end find_closest_site */

static INT sort_softpins( pinAptr, pinBptr )
PINBOXPTR *pinAptr, *pinBptr ;
{

    PINBOXPTR pinA ;              /* pinA dereference for ease of use */
    PINBOXPTR pinB ;              /* pinB dereference for ease of use */
    SOFTBOXPTR sinfoA ;          /* softpin info on pinA */
    SOFTBOXPTR sinfoB ;          /* softpin info on pinA */

    pinA = *pinAptr ;
    pinB = *pinBptr ;
    sinfoA = pinA->softinfo ;
    sinfoB = pinB->softinfo ;

    /* hardpins must be placed first */
    if( sinfoA->numranges == 0 && sinfoB->numranges == 0 ){
	return( 0 ) ;
    } else if( sinfoA->numranges == 0 ){
	return( -1 ) ;
    } else if( sinfoB->numranges == 0 ){
	return( 1 ) ;
    }
    /* now check the sequences */
    /* we want sequences followed by isolated pins */
    /* pins of sequence will be place by the sequence pin */
    if( sinfoA->sequence > 1 && sinfoB->sequence > 1 ){
	return( sinfoA->sequence - sinfoB->sequence ) ;
    } else if( sinfoA->sequence > 1 ){
	return( -1 ) ;
    } else if( sinfoB->sequence > 1 ){
	return( 1 ) ;
    } else if( sinfoA->sequence == 1 && sinfoB->sequence == 1 ){
	return( 0 ) ;
    } else if( sinfoA->sequence == 1 ){
	return( -1 ) ;
    } else if( sinfoB->sequence == 1 ){
	return( 1 ) ;
    } else {
	return( 0 ) ;
    }
} /* end sort_softpins */

/* maintain allocation of sites */
static CONTENTPTR *check_alloc( sites, numsites )
CONTENTPTR *sites ;
INT numsites ;
{
    INT i ;                      /* counter */
    INT oldAlloc ;               /* old allocation */
    static INT siteAllocS ;      /* current site allocation */

    if( sites ){
	if( numsites >= siteAllocS ){
	    oldAlloc = siteAllocS ;
	    siteAllocS = numsites + EXPECTEDNUMSITES + 1  ;
	    sites = (CONTENTPTR *) Ysafe_realloc( sites,
		siteAllocS * sizeof(CONTENTPTR) ) ;
	    for( i = oldAlloc; i < siteAllocS; i++ ){
		sites[i] = (CONTENTPTR) 
		    Ysafe_calloc( 1,sizeof(CONTENTBOX) ) ;
	    }
	}
    } else {
	/* start new site array */
	siteAllocS = numsites + 1 ;
	sites = (CONTENTPTR *) 
	    Ysafe_calloc( siteAllocS,sizeof(CONTENTPTR) ) ;
	for( i = 1; i <= numsites; i++ ){
	    sites[i] = (CONTENTPTR) Ysafe_calloc( 1,sizeof(CONTENTBOX) ) ;
	}
    }
    return( sites ) ;
} /* end check_alloc */

static set_pin_n_site( site_array, pin, site )
CONTENTPTR *site_array ;
PINBOXPTR pin ;
INT site ;
{
    SOFTBOXPTR softinfo ;
    CONTENTPTR sptr ;

    softinfo = pin->softinfo ;
    softinfo->site = site ;
    softinfo->newsite = TRUE ;
    sptr = site_array[site] ;
    sptr->contents++ ;
    pin->txpos = sptr->xpos ;
    pin->typos = sptr->ypos ;
    /* set original field */
    pin->txpos_orig[instS] = sptr->xpos ;
    pin->typos_orig[instS] = sptr->ypos ;
}

/* ****************************************************************** */


static init_hard_struct( cellptr )
CELLBOXPTR cellptr ;
{
    INT oright, otop ;             /* calculate difference from bbox */
    DOUBLE val ;                   /* temp */
    BOUNBOXPTR bounptr ;           /* bounding box pointer */
    INSTBOXPTR instptr ;           /* pointer to instance info */

    if( cellptr->orient <= 3 ) {
	aspFactorS = sqrt( cellptr->aspect / cellptr->orig_aspect ) ;
    } else {
	aspFactorS = sqrt( cellptr->orig_aspect / cellptr->aspect ) ;
    }
    if( instptr = cellptr->instptr ){
	instS = instptr->cur_inst ;
    } else {
	instS = 0 ;
    }
    /* use original bounding box to calculate changes */
    bounptr = cellptr->bounBox[8] ; /* original bbox */
    oleftS = bounptr->l ;
    obottomS = bounptr->b ;
    oright = bounptr->r ;
    otop = bounptr->t ;
    val = (DOUBLE) (otop - obottomS) * aspFactorS ;
    Hdiv2S = ROUND( val ) / 2 ;
    val = (DOUBLE) (oright - oleftS) / aspFactorS ;
    Wdiv2S = ROUND( val ) / 2 ;
} /* end init_hard_struct */

static find_new_hard_pos( pin )
PINBOXPTR pin ;
{
    DOUBLE val ;

    val = (DOUBLE)(pin->txpos_orig[instS] - oleftS) / aspFactorS ;
    pin->txpos = ROUND( val ) - Wdiv2S ;
    val = (DOUBLE)(pin->typos_orig[instS] - obottomS) * aspFactorS ;
    pin->typos = ROUND( val ) - Hdiv2S ;
} /* end find_new_hard_pos */

#endif
