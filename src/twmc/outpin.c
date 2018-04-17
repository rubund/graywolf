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
FILE:	    outpin.c                                       
DESCRIPTION:Routine outputs the pin positions to the pin file.
CONTENTS:   outpin()
	    INT getPadMacroNum( side )
		INT side ;
	    setPadMacroNum( side, cellnum )
		INT side, cellnum ;
	    PINBOXPTR findTerminal( pinName, cell ) 
		char *pinName ;
		INT cell ;
DATE:	    Mar 16, 1988 - added description block and findTerminal.
REVISIONS:  Apr 25, 1989 - ignore 1 pin nets - causes global router to 
		crash.
	    Jun 21, 1989 - fixed so only true pins are output - not
		not averaged equiv pin locations.
	    Sep 19, 1989 - now output equiv pins correctly.
	    Apr 23, 1990 - modified for the new softpin algorithm.
	    Sun Dec 16 00:39:46 EST 1990 - now output the analog nets.
	    Wed Dec 19 23:59:29 EST 1990 - now output cap and res
		net matches.
	    Dec 21, 1990 - now output the pin layer.
	    Sun Jan 20 21:34:36 PST 1991 - ported to AIX.
	    Wed Jan 30 14:13:36 EST 1991 - now left justify orientation
		rotations.
	    Sat May 11 22:41:38 EDT 1991 - automatically move pad
		pins to correct channel.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) outpin.c version 3.11 5/11/91" ;
#endif

#include <string.h>
#include <custom.h>
#include <initialize.h>
#include <analog.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>
#include <yalecad/relpos.h>
#include <pads.h>



static BOOL *output_alreadyS ;  /* mark the pins as they are output */
static FILE *fpS ;
static INT  output_typeS ;      /* describe the type of circuit */



static output_pin();
static INT find_cell();
static process_analog_net();
static INT find_cell();
static output_matches();




outpin()
{

    INT net ;               /* counter */
    char filename[LRECL] ;  /* open this filename for writing */
    NETBOXPTR netptr ;      /* current net info */
    PINBOXPTR pinptr ;      /* current pin info */

    sprintf( filename, "%s.mpin", cktNameG ) ;
    fpS = TWOPEN( filename , "w", ABORT ) ;

    output_alreadyS = (BOOL *) Ysafe_calloc( numpinsG+1, sizeof(BOOL) ) ;
    output_typeS = NONE ;
    for( net = 1 ; net <= numnetsG ; net++ ) {
	netptr =  netarrayG[net] ;
	if( netptr->numpins <= 1 ){
	    /* no need to global route 1 pin nets */
	    continue ;
	}
	if( netptr->analog_info ){
	    process_analog_net( netptr ) ;
	    output_typeS |= ANALOG ;
	    continue ;
	}
	fprintf(fpS,"net %s\n", netptr->nname ) ;
	for( pinptr = netptr->pins ;pinptr;pinptr = pinptr->next ) {
	    output_pin( pinptr ) ;
	    output_typeS |= DIGITAL ;
	}
	fprintf( fpS,"\n" ) ;
    }
    output_matches() ;
    TWCLOSE(fpS);
    return ;
} /* end outpins */


static output_pin( pinptr )
PINBOXPTR pinptr ;
{
    INT i ;              /* softpin counter */
    INT cell ;           /* the cell output modified for pad macros */
    INT instance ;       /* the current instance of the cell */
    INT x, y ;           /* the output coordinate of the pin */
    INT xrel, yrel ;     /* the x, y position relative to cell */
    INT howmany ;        /* howmany equivalent pins */
    DOUBLE current ;     /* the current specified for the pin */
    EQUIVPTR eqptr ;     /* current equivalent pointer */
    PINBOXPTR child ;    /* current child of the softpin */
    SOFTBOXPTR spin ;    /* current softpin */
    CELLBOXPTR cellptr ; /* current cell info */
    BOUNBOXPTR bounptr ; /* bounding box pointer */
    /*static INT find_cell() ;*/ /* get cell number */

    cell = find_cell( pinptr->cell ) ;
    cellptr = cellarrayG[pinptr->cell] ;
    instance = cellptr->cur_inst ;

    /* setup translation of output points */
    bounptr = cellptr->bounBox[0] ;
    /* now init the translation routines using bounding box */
    Ytrans_init( bounptr->l,bounptr->b,bounptr->r,bounptr->t,
		cellptr->orient ) ;

    if( pinptr->type == SOFTEQUIVTYPE ){
	/* delay the equivalent softpins until below */
	return ;
    } else {

	output_alreadyS[pinptr->pin] = TRUE ;

	xrel = pinptr->txpos_orig[instance] ;
	yrel = pinptr->typos_orig[instance] ;

	if( cell != pinptr->cell ){
	    /* this only occurs for pads */
	    if( yrel != bounptr->t ){
		D( "twmc/outpin",
		    fprintf( stderr, "Moving pad pin:%s to boundary\n",
			pinptr->pinname ) ;
		) ;
		yrel = bounptr->t ;
	    }
	}

	/* rel position is a macro which calculates */
	/* absolute pin loc - defined in relpos.h */
	REL_POST( cellptr->orient, 
	    x, y,                               /* result */
	    xrel, yrel,                         /* cell relative */
	    cellptr->xcenter, cellptr->ycenter ) ;  /* cell center */

	D( "twmc/outpin",
	    if( x % track_spacingXG ){
		fprintf( stderr, "Pin %s on cell %d x not on grid\n",
		    pinptr->pinname, cell ) ;
	    }
	    if( y % track_spacingYG ){
		fprintf( stderr, "Pin %s on cell %d x not on grid\n",
		    pinptr->pinname, cell ) ;
	    }
	) ;

	fprintf(fpS,"pin %s x %d y %d cell %d layer %d PinOrEquiv 1",
	    pinptr->pinname, x, y , cell, pinptr->layer ) ;
	if( pinptr->analog ){
	    current = pinptr->analog->current ;
	    if( current > NO_CURRENT_SPECIFIED ){
		fprintf( fpS, " current %4.3le", current ) ;
	    }
	}
	fprintf( fpS, "\n" ) ;
    }
    /* ******** HARD PIN EQUIVALENT CASE ****** */
    if( pinptr->eqptr && !(pinptr->softinfo)){ 
	/* occurs only for hard pins */
	/* now process the equivalent pins */
	for( eqptr=pinptr->eqptr; eqptr; eqptr=eqptr->next ){
	    REL_POST( cellptr->orient, 
		x, y,                               /* result */
		eqptr->txpos[instance],
		eqptr->typos[instance],          /* cell relative */
		cellptr->xcenter, cellptr->ycenter ) ; /* center */
	    fprintf(fpS,"pin %s  x %d  y %d  cell %d layer %d PinOrEquiv 0",
		pinptr->pinname, x, y , cell, pinptr->layer ) ;
	    if( pinptr->analog ){
		current = pinptr->analog->current ;
		if( current > NO_CURRENT_SPECIFIED ){
		    fprintf( fpS, " current %4.3le", current ) ;
		}
	    }
	    fprintf( fpS, "\n" ) ;
	}
    }
    /* ******** SOFT PIN EQUIVALENT CASE ****** */
    /* now look for equivalent pins are children */
    if( pinptr->type == SOFTPINTYPE ){
	spin = pinptr->softinfo ;
	if( spin->children ){
	    howmany = (INT) spin->children[HOWMANY] ;
	} else {
	    howmany = 0 ;
	}
	for( i = 1; i <= howmany; i++ ){
	    child = spin->children[i] ;
	    REL_POST( cellptr->orient, 
		x, y,                               /* result */
		child->txpos, child->typos,     /* cell relative */
		cellptr->xcenter, cellptr->ycenter ) ; /* center */

	    fprintf(fpS,"pin %s  x %d  y %d  cell %d layer %d PinOrEquiv 0",
		child->pinname, x, y , cell, child->layer ) ;
	    if( pinptr->analog ){
		current = pinptr->analog->current ;
		if( current > NO_CURRENT_SPECIFIED ){
		    fprintf( fpS, " current %4.3le", current ) ;
		}
	    }
	    fprintf( fpS, "\n" ) ;
	}
    }
} /* end output_pin */

static process_analog_net( netptr )
NETBOXPTR netptr ;
{
    INT i, j ;           /* counters */
    INT pin ;            /* the pin index for the current pin in cpt */
    INT len ;            /* keep track of the total length output */
    INT howmany ;        /* howmany pins in the common point */
    char *pinname ;      /* current pinname output */
    ANETPTR aptr ;       /* current analog info record */
    COMMONPTR cpt ;      /* current common point */
    PINBOXPTR pinptr ;   /* current pins information */

    aptr = netptr->analog_info ;
    fprintf(fpS,"analog_net %s ", netptr->nname ) ;
    if( aptr->cap_upper_bound > 0.0 ){
	fprintf(fpS,"cap %4.3le ", aptr->cap_upper_bound ) ;
    }
    if( aptr->res_upper_bound > 0.0 ){
	fprintf(fpS,"res %4.3le ", aptr->res_upper_bound ) ;
    }
    if( aptr->max_drop > NO_CONSTRAINT_ON_DROP ){
	fprintf(fpS, "max_drop %4.3le ", aptr->max_drop ) ;
    }
    if( aptr->net_type & SHIELDMASK ){
	fprintf(fpS, "shielding " ) ;
    }
    if( aptr->net_type & NOISYMASK ){
	fprintf(fpS, "noisy " ) ;
    }
    if( aptr->net_type & SENSITIVEMASK ){
	fprintf(fpS, "sensitive " ) ;
    }
    fprintf( fpS, "\n" ) ;

    /* output any common points first */
    for( i = 0; i < aptr->num_common_points; i++ ){
	cpt = aptr->common_pts[i] ;
	fprintf( fpS, "common_point %d\n", cpt->num_pins  ) ;
	len = 0 ;
	howmany = cpt->num_pins - 1 ;
	for( j = 0; j <= howmany; j++ ){
	    pin = cpt->common_set[j] ;
	    output_alreadyS[pin] = TRUE ;
	    ASSERTNCONT( pin > 0 && pin < numpinsG,
		"outpin", "Trouble with common point\n" ) ;
	    pinptr = termarrayG[pin] ;
	    output_pin( pinptr ) ;
	}
	/* now output any cap match if it exists */
	if( cpt->cap_match ){
	    howmany = cpt->cap_match[HOWMANY] ;
	    fprintf( fpS, "cap_match " ) ;
	    for( j = 1; j <= howmany; j++ ){
		pin = cpt->cap_match[j] ;
		ASSERTNCONT( pin > 0 && pin < numpinsG,
		    "outpin", "Trouble with cap_match\n" ) ;
		pinname = termarrayG[pin]->pinname ; 
		len += strlen( pinname ) ;
		if( j > 1 ){
		    fprintf( fpS, ", " ) ;
		}
		if( len > LRECL ){
		    fprintf( fpS, "\n" ) ;
		}
		fprintf( fpS, "%s %d", pinname,
		    find_cell( termarrayG[pin]->cell ) ) ;
	    }
	    fprintf( fpS, "\n" ) ;
	    
	}
	/* now output any res match if it exists */
	if( cpt->res_match ){
	    howmany = cpt->res_match[HOWMANY] ;
	    fprintf( fpS, "res_match " ) ;
	    for( j = 1; j <= howmany; j++ ){
		pin = cpt->res_match[j] ;
		ASSERTNCONT( pin > 0 && pin < numpinsG,
		    "outpin", "Trouble with res_match\n" ) ;
		pinname = termarrayG[pin]->pinname ; 
		len += strlen( pinname ) ;
		if( j > 1 ){
		    fprintf( fpS, ", " ) ;
		}
		if( len > LRECL ){
		    fprintf( fpS, "\n" ) ;
		}
		fprintf( fpS, "%s %d", pinname,
		    find_cell( termarrayG[pin]->cell ) ) ;
	    }
	    fprintf( fpS, "\n" ) ;
	    
	}
    }
    /* now output the regular pins that have no common point */
    for( pinptr = netptr->pins;pinptr;pinptr=pinptr->next ) {
	if( output_alreadyS[pinptr->pin] ){
	    continue ;
	}
	output_pin( pinptr ) ;
    } 
    fprintf( fpS, "\n" ) ;
} /* process_analog_net */

/* since channel graph generator creates many channels for pads */
/* we block pads together and create a new cell called a pad macro */
/* the cell number is numbered in outgeo.c. This number needs to be */
/* matched in outpins.  Therefore, we use this simple translation table */
static INT side2cellS[5] = { 0,0,0,0,0 } ;
INT getPadMacroNum( side )
INT side ;
{
    return( side2cellS[side] ) ;
} /* end getPadMacroNum */

setPadMacroNum( side, cellnum )
INT side ;
INT cellnum ;
{
    side2cellS[side] = cellnum ;
} /* end getPadMacroNum */

static INT find_cell( cell )
INT cell ;
{
    if( cell > endsuperG ) {
	if( cellarrayG[cell]->padptr->padside == L ) {
	    cell = getPadMacroNum( L ) ;
	} else if( cellarrayG[cell]->padptr->padside == T ) {
	    cell = getPadMacroNum( T ) ;
	} else if( cellarrayG[cell]->padptr->padside == R ) {
	    cell = getPadMacroNum( R ) ;
	} else if( cellarrayG[cell]->padptr->padside == B ) {
	    cell = getPadMacroNum( B ) ;
	}
    }
    return( cell ) ;
} /* end find_cell */

/* given pinName and cell find pin recosrd */
/* return NULL if no terminal is found */
PINBOXPTR findTerminal( pinName, cell ) 
char *pinName ;
INT cell ;
{
    PINBOXPTR pin ;
    CELLBOXPTR cptr ;

    cptr = cellarrayG[cell] ;
    /* first look thru hard pins */
    for( pin=cptr->pinptr;pin;pin=pin->nextpin ){
	if( strcmp( pinName, pin->pinname ) == STRINGEQ ){
	    return( pin ) ;
	}
    }

    /* we have failed to find a match return null */
    return( NULL ) ;

} /* end findTerminal */

INT get_circuit_type()
{
    return( output_typeS ) ;
} /* end get_circuit_type */

static output_matches()
{
    INT i, j ;
    INT net ;
    INT *match ;
    INT num_matches ;
    INT howmany ;

    howmany = (INT) net_cap_matchG[HOWMANY] ;
    for( i = 1; i <= howmany; i++ ){
	match = net_cap_matchG[i] ;
	num_matches = match[HOWMANY] ;
	fprintf(  fpS, "net_cap_match " ) ;
	for( j = 1; j <= num_matches; j++ ){
	    net = match[j] ;
	    fprintf( fpS, "%s ", netarrayG[net]->nname ) ;
	}
	fprintf( fpS, "\n" ) ;
    }

    howmany = (INT) net_res_matchG[HOWMANY] ;
    for( i = 1; i <= howmany; i++ ){
	match = net_res_matchG[i] ;
	num_matches = match[HOWMANY] ;
	fprintf(  fpS, "net_res_match " ) ;
	for( j = 1; j <= num_matches; j++ ){
	    net = match[j] ;
	    fprintf( fpS, "%s ", netarrayG[net]->nname ) ;
	}
	fprintf( fpS, "\n" ) ;
    }

} /* end output_matches */
