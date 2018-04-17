/*
 *   Copyright (C) 1989-1992 Yale University
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
DESCRIPTION:calculate total cost of placement.
CONTENTS:   findcost()
	    create_cell( )
	    find_net_sizes()
DATE:	    Mar 27, 1989 
REVISIONS:  Apr  1, 1990 - added missing ignore test for wire penalty.
		Moved to the correct place.
	    Thu Dec 20 00:09:18 EST 1990 - removed += operator.
	    Tue Jan 29 23:38:32 EST 1991 - added WARNING of large
		pins to output file and stderr.
	    Mon Feb 11 23:53:22 EST 1991 - commented out unnecessary
		call to findunlap.
	    Wed Jul  3 13:49:49 CDT 1991 - now print out pins of
		any size as a debug function.
	    Wed Aug 28 15:03:52 EDT 1991 - now place the pads initially
		and added IGNORE info for large pin warning.
	    Thu Sep 19 14:15:51 EDT 1991 - added equal width cell
		capability.
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) findcost.c (Yale) version 4.18 4/2/92" ;
#endif
#endif

#define EXTRA_BIN 0.50
#define MAXNUMPINS    100		/* WPS */
/* #define MITLL */

#include "standard.h"
#include "groute.h"
#include "main.h"
#include "config.h"
#include "readpar.h"
#include "parser.h"
#include <yalecad/debug.h>
#include <yalecad/message.h>

/* global variables */
INT minxspanG ;

extern INT extra_cellsG ;
extern INT spacer_widthG ;
extern INT approximately_fixed_factorG ;
extern BOOL rigidly_fixed_cellsG ;
extern INT *feeds_in_rowG ;

/* static variables */
static INT maxpinS = 0 ;
static INT iwireS ;
static INT iwirexS ;
static INT iwireyS ;
static INT print_pinS = 0 ;
static spread_equal_cells();
static spread_cells();

findcost()
{
INT block , bin ;
FILE *fp ;
TIBOXPTR tile , tileptr1 ;
CBOXPTR cellptr1 , ptr ;
DBOXPTR dimptr ;
PINBOXPTR netptr , termptr ;
BINPTR bptr ;
char filename[256] ;
INT left , right , corient , val ;
INT LoBin , HiBin ;
INT cell , net , blk ;
INT startx , endx ;
INT x , y , t , cost=0 ;
INT temp , n , k , cbin ;
INT sum, npins ;
INT net_pin_num[ MAXNUMPINS + 1 ] , *adjust_left ;
DOUBLE deviation ;
INT pathcount , shortest_cell , count, length ;
INT layer ;
PATHPTR path ;
GLISTPTR net_of_path ;
extern char *find_layer() ;
char *pinname ;

blkleftG = INT_MAX ;
blkriteG = INT_MIN ;
adjust_left = (INT *)Ysafe_malloc( ( numRowsG + 1 ) * sizeof(INT) ) ;
for( block = 1 ; block <= numRowsG ; block++ ) {
/* ------------- finding the absolute left (blkleftG) and -----------
   ------------- the absolute right (blkriteG) positions  ----------- */
    if( barrayG[ block ]->bxcenter + barrayG[ block ]->bleft <
						blkleftG ) {
	blkleftG = barrayG[ block ]->bxcenter +
					barrayG[ block ]->bleft ;
    } 
    if( barrayG[ block ]->bxcenter + 
			barrayG[ block ]->bright > blkriteG ) {
	blkriteG = barrayG[ block ]->bxcenter +
				    barrayG[ block ]->bright ;
    }
    adjust_left[ block ] = INT_MAX ;
}
binOffstG = blkleftG ;
max_blklengthG = blkriteG - blkleftG ;

deviation = 0 ;
temp = 0 ;
for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
    temp += carrayG[ cell ]->clength ;
}
mean_widthG = temp / (numcellsG - extra_cellsG) ;
fprintf(fpoG,"Original Average Cell Width:%f\n", mean_widthG ) ;

/* modification by Carl 10/5/89 */
/* an attempt to rectify a large cell problem */
shortest_cell = INT_MAX ;
count = 0 ;
temp = 0 ;
for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
    if( carrayG[cell]->cclass < 0 ) {
	continue ;
    }
    count++ ;
    if( carrayG[cell]->clength <= 2.0 * mean_widthG ) {
	temp += carrayG[ cell ]->clength ;
    }
    if( carrayG[cell]->clength < shortest_cell ) {
	shortest_cell = carrayG[cell]->clength ;
    }
}
if( gate_arrayG && rigidly_fixed_cellsG ) {
    mean_widthG = shortest_cell ;
} else {
    if( count > 0 ) {
	mean_widthG = temp / count ;
    } else {
	/* this is only used if all cells are fixed */
	count = 0 ;
	temp = 0 ;
	for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
	    count++ ;
	    temp += carrayG[ cell ]->clength ;
	}
	mean_widthG = temp / count ;
    }
}
fprintf(fpoG,"Adjusted Average Cell Width:%f\n", mean_widthG ) ;



t = 0 ;
for( cell = numcellsG + 1 ; cell <= lastpadG ; cell++ ) {
    t += carrayG[cell]->numterms ;
}

/*  New by Carl  9/11/89  to get the proper pin sep avg's */
temp = 0 ;
for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    temp += carrayG[cell]->clength ;
}
/* **** */

average_pin_sepG = temp / ( maxtermG - t ) ;
if( implicit_feed_countG ) {
    average_feed_sepG = temp / implicit_feed_countG ;
    if( average_feed_sepG > 4 * average_pin_sepG ) {
	average_feed_sepG = 4 * average_pin_sepG ;
    }
} else {
    average_feed_sepG = 4 * average_pin_sepG ;
}
#ifdef MITLL
    average_feed_sep *= 2 ;
#endif

for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
    deviation += ( carrayG[ cell ]->clength - mean_widthG )
	       * ( carrayG[ cell ]->clength - mean_widthG ) ;
}
deviation = sqrt( deviation / (numcellsG - extra_cellsG) ) ;

binWidthG  = ( INT ) ( mean_widthG ) ;

if(!(Equal_Width_CellsG)){
    while (1) {
	numBinsG = ( blkriteG - binOffstG ) / binWidthG ;
	if( ( blkriteG - binOffstG ) > ( numBinsG * binWidthG ) ) {
	    numBinsG++ ;
	}
	if( (DOUBLE) (numBinsG * numRowsG - (numcellsG - extra_cellsG) ) / 
			    (DOUBLE)(numcellsG - extra_cellsG) > EXTRA_BIN)
	    break;
	binWidthG --;
    }
} else {

    while ( (numBinsG * numRowsG) < (numcellsG - extra_cellsG) ){
       numBinsG++ ;
    }
}

fprintf( fpoG , "numBinsG automatically set to:%d\n", numBinsG);
fprintf( fpoG , "binWidthG = %d\n",binWidthG ) ;
fprintf( fpoG , "average_cell_width is:%g\n",mean_widthG ) ;
fprintf( fpoG , "standard deviation of cell length is:%g\n",
		deviation ) ;
if( gate_arrayG ) {
    cells_per_clusterG = 1 ;
    num_clustersG = extra_cellsG / cells_per_clusterG ;
    cluster_widthG = cells_per_clusterG * spacer_widthG ;
    fprintf(fpoG,"\nAdded %d spacer clusters to the gate array\n", num_clustersG);
    fprintf(fpoG,"cluster width: %d\n", cluster_widthG ) ;
} else {
    num_clustersG = 0 ;
}
minxspanG = (INT)(mean_widthG + 3.0 * deviation) ;

#ifdef OLD_WAY
if( gate_arrayG ) {
    minxspanG = (INT)(mean_widthG + 4.0 * deviation) ;
    /*
    factor = 0.0 ;
    for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
	if( carrayG[cell]->cclass < 0 ) {
	    factor += (DOUBLE) carrayG[cell]->clength ;
	}
    }
    factor += (DOUBLE)(num_clusters * cluster_width) ;
    if( total_row_length > (INT) factor ) {
	factor = (DOUBLE) total_row_length / 
				((DOUBLE) total_row_length - factor) ;
	factor = (DOUBLE) minxspanG * factor ;
	fprintf(fpoG,"\nminxspanG INCREASED from %d to %d\n",
						minxspanG,(INT)factor);
	minxspanG = (INT) factor ;
    } else {
	fprintf(fpoG,"\nSTRANGE COMPUTATION OF MINXSPAN IN FINDCOST\n");
	minxspanG *= 3 ;
    }
    */
}
#endif /* OLD_WAY */


/*********************************************************** */

if( connection_machineG != 3 ) {
    sprintf( filename , "%s.res" , cktNameG ) ;
    fp = TWOPEN( filename , "r", NOABORT) ;
} else {
    sprintf( filename , "%s.cmi" , cktNameG ) ;
    fp = TWOPEN ( filename , "r", ABORT ) ;
}
if( !(fp) ) {
    /* 
     *  Could not open the file cktName.res
     */
    if( resume_runG == YES ) {
	fprintf(fpoG,"Could not use resume file: %s  ", filename ) ;
	fprintf(fpoG,"since it could not be found\n");
    } else {
	fprintf(fpoG,"TimberWolfSC starting from the beginning\n");
    }
} else if( resume_runG == NO ) {
    fprintf(fpoG,"Use of resume file: %s was not requested\n", filename);
} else {
    if( connection_machineG != 3 ) {
	fprintf( fpoG, "Resuming TimberWolfSC from previous saved ");
	fprintf( fpoG, "placement in %s\n", filename );
	TW_oldin( fp ) ;
	/* findunlap(0); */
	TWCLOSE( fp ) ;
    } else {
	fprintf( fpoG, "Starting TimberWolfSC with connection ");
	fprintf( fpoG, "machine placement in %s\n", filename );
	incm( fp ) ;
	TWCLOSE( fp ) ;
    }
}

sortpin() ;
for( cell = 1 ; cell <= lastpadG ; cell++ ) {
    ptr = carrayG[ cell ] ;
    corient = ptr->corient ;
    block   = ptr->cblock  ;
    tile    = ptr->tileptr ;
    if( cell <= numcellsG - extra_cellsG ) {
	ptr->cycenter = barrayG[block]->bycenter ;
	if( adjust_left[block] > ptr->cxcenter + tile->left ) {
	    adjust_left[block] = ptr->cxcenter + tile->left ;
	}
	for( termptr = ptr->pins;termptr;termptr =termptr->nextpin ) {
	    termptr->xpos = termptr->txpos[corient/2] + ptr->cxcenter;
	    termptr->ypos = termptr->typos[corient%2] + ptr->cycenter;
	}
    } else if( cell > numcellsG ) {
	for( termptr = ptr->pins;termptr;termptr =termptr->nextpin ) {
	    termptr->xpos = termptr->txpos[1] + ptr->cxcenter;
	    termptr->ypos = termptr->typos[1] + ptr->cycenter;
	}
    }
}
/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */ 
/* @@@ calculate the initial wire cost of user's placement @@@ */
iwirexS = 0 ;
iwireyS = 0 ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    dimptr =  netarrayG[ net ] ;
    if( !(netptr = dimptr->pins)) {
	continue ;
    }
    dimptr->xmin = dimptr->xmax = netptr->xpos ;
    dimptr->ymin = dimptr->ymax = netptr->ypos ;
    for( netptr = netptr->next ; netptr ; netptr = netptr->next ) {
	x = netptr->xpos ;
	y = netptr->ypos ;
	if( x < dimptr->xmin ) {
	    dimptr->xmin = x ;
	} else if( x > dimptr->xmax ) {
	    dimptr->xmax = x ;
	}
	if( y < dimptr->ymin ) {
	    dimptr->ymin = y ;
	} else if( y > dimptr->ymax ) {
	    dimptr->ymax = y ;
	}
    }
    cost += dimptr->halfPx = dimptr->newhalfPx = 
					 dimptr->xmax - dimptr->xmin ;
    dimptr->halfPy = dimptr->newhalfPy = dimptr->ymax - dimptr->ymin ;
    cost = cost + (INT)( vertical_wire_weightG * (DOUBLE) dimptr->halfPy ) ;

    iwirexS += dimptr->xmax - dimptr->xmin ;
    iwireyS += dimptr->ymax - dimptr->ymin ;
}

fprintf( fpoG, "\n\n\nTHIS IS THE ROUTE COST OF THE ");
fprintf( fpoG, "ORIGINAL PLACEMENT: %d\n" , cost ) ;

/* @@@@@@@@@@@@@@@@@@@@@@ calculation end @@@@@@@@@@@@@@@@@@@@ */ 

/* $$$$$$$$$$$ move the cells in each block such that   $$$$$$$ */
/* $$$$$$$$$$$ they line up with the left edge of block $$$$$$$ */

if( resume_runG == NO ) {

    for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
	ptr = carrayG[cell] ;
	corient = ptr->corient ;
	block   = ptr->cblock ;
	ptr->cxcenter -= adjust_left[block] - 
		 barrayG[block]->bxcenter - barrayG[block]->bleft ;
	for( termptr = ptr->pins ;termptr;termptr = termptr->nextpin ) {
	    termptr->xpos = termptr->txpos[corient/2] + ptr->cxcenter;
	    termptr->ypos = termptr->typos[corient%2] + ptr->cycenter;
	}
    }
}
/* $$$$$$$$$$$$$$$$$$$$$$ adjustment end $$$$$$$$$$$$$$$$$$$$$$ */

for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
    if( carrayG[cell]->fence != NULL ) {
	carrayG[cell]->fence->min_xpos = carrayG[cell]->cxcenter -
			approximately_fixed_factorG * minxspanG ;
	carrayG[cell]->fence->max_xpos = carrayG[cell]->cxcenter +
			approximately_fixed_factorG * minxspanG ;
    }
}
/*********************************************************** */

cost = 0 ;
iwirexS = 0 ;
iwireyS = 0 ;

for( net = 1 ; net <= numnetsG ; net++ ) {
    dimptr =  netarrayG[ net ] ;
    if( netptr = dimptr->pins ) {
	while( carrayG[netptr->cell]->ECO_flag ) {
	    fprintf(fpoG,"ECO pin skipped for net <%s>\n",
			netarrayG[netptr->net]->name ) ;
	    netptr = netptr->next ;
	    if( netptr == NULL ) {
		break ;
	    }
	}
	if( netptr == NULL ) {
	    continue ;
	}
	dimptr->xmin = dimptr->xmax = netptr->xpos ;
	dimptr->ymin = dimptr->ymax = netptr->ypos ;
	dimptr->Lnum = dimptr->Rnum = 1 ;
	dimptr->Bnum = dimptr->Tnum = 1 ;
	netptr = netptr->next ;
    }
    n = 1 ;
    for( ; netptr ; netptr = netptr->next ) {
	while( carrayG[netptr->cell]->ECO_flag ) {
	    fprintf(fpoG,"ECO pin skipped for net <%s>\n",
			netarrayG[netptr->net]->name ) ;
	    netptr = netptr->next ;
	    if( netptr == NULL ) {
		break ;
	    }
	}
	if( netptr == NULL ) {
	    break ;
	}
	x = netptr->xpos ;
	y = netptr->ypos ;
	if( x < dimptr->xmin ) {
	    dimptr->xmin = x ;
	    dimptr->Lnum = 1 ;
	} else if( x == dimptr->xmin ) {
	    dimptr->Lnum++ ;
	    if( x == dimptr->xmax ) {
		dimptr->Rnum++ ;
	    }
	} else if( x > dimptr->xmax ) {
	    dimptr->xmax = x ;
	    dimptr->Rnum = 1 ;
	} else if( x == dimptr->xmax ) {
	    dimptr->Rnum++ ;
	}
	if( y < dimptr->ymin ) {
	    dimptr->ymin = y ;
	    dimptr->Bnum = 1  ;
	} else if( y == dimptr->ymin ) { 
	    dimptr->Bnum++ ;
	    if( y == dimptr->ymax ) {
		dimptr->Tnum++ ;
	    }
	} else if( y > dimptr->ymax ) {
	    dimptr->ymax = y ;
	    dimptr->Tnum = 1  ;
	} else if( y == dimptr->ymax ) {
	    dimptr->Tnum++ ;
	}
	n++ ;
    }
    dimptr->numpins = n ;

    if( dimptr->ignore == 1 ){
	continue ;
    }

    cost += dimptr->halfPx = dimptr->newhalfPx = 
					 dimptr->xmax - dimptr->xmin ;
    dimptr->halfPy = dimptr->newhalfPy = dimptr->ymax - dimptr->ymin ;
    cost = cost + (INT)( vertical_wire_weightG * (DOUBLE) dimptr->halfPy ) ;
    D( "twsc/findcost",
	fprintf( fpoG, "net:%5d cum cost:%10d\n", net, cost ) ;
	fflush( fpoG ) ;
    ) ;

    iwirexS += dimptr->xmax - dimptr->xmin ;
    iwireyS += dimptr->ymax - dimptr->ymin ;
}
iwireS = cost ;

for( n = 1 ; n <= MAXNUMPINS ; n++ ) {
    net_pin_num[ n ] = 0 ;
}
fprintf( fpoG, "\n" ) ;
for( net = 1 ; net <= numnetsG ; net++ ) {
    dimptr =  netarrayG[ net ] ;
    maxpinS = MAX( maxpinS, dimptr->numpins ) ;
    if( dimptr->numpins >= MAXNUMPINS ) {
	net_pin_num[ MAXNUMPINS ]++ ;
	if( dimptr->ignore == 1 ) {
	    sprintf(YmsgG,
		"IGNORED Net <%s> has %d pins\n", dimptr->name, 
		dimptr->numpins ) ;
	} else {
	    sprintf(YmsgG,
		"Net <%s> has %d pins\n", dimptr->name, 
		dimptr->numpins ) ;
	}
	M( WARNMSG, "findcost", YmsgG ) ;
	fprintf( fpoG, "WARNING[findcost]:%s", YmsgG ) ;
    } else {
	net_pin_num[ dimptr->numpins ]++ ;
    }
    if( print_pinS ){
	if( dimptr->numpins == print_pinS ){
	    fprintf( fpoG, "A net with %d pin[s]:net%d - %s\n" , 
		print_pinS, net, netarrayG[net]->name ) ;
	    for( termptr=dimptr->pins;termptr;termptr=termptr->next){
		cell = termptr->cell ;
		pinname = find_layer( termptr->pinname, &layer ) ;
		fprintf( fpoG, "\tpinname:%s cellname:%s cellnum:%d\n",
		    pinname, carrayG[cell]->cname, cell ) ;
	    }
	}
    }
}
/*
for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
    n = 0 ;
    for( termptr = carrayG[cell]->pins;termptr;termptr=termptr->nextpin ) {
	n++ ;
    }
    if( n < 1 ) {
	fprintf(fpoG,"This cell has no pins: %s\n", carrayG[cell]->cname);
    }
}
*/

sum = npins = 0 ;
for( n = 1 ; n < MAXNUMPINS ; n++ ) {
    if( net_pin_num[ n ] ){
	fprintf( fpoG, "The number of nets with %d pins is %d\n",
		 n , net_pin_num[ n ] ) ;
    }
    sum += n * net_pin_num[ n ] ;
    npins += net_pin_num[ n ] ;
}

fprintf( fpoG, "The number of nets with %d pins or more is %d\n",
	    MAXNUMPINS, net_pin_num[ MAXNUMPINS ] ) ;
fprintf( fpoG, "Average number of pins per net = %f\n",
	    (DOUBLE) sum / (DOUBLE) npins ) ;
fprintf( fpoG, "The maximum number of pins on a single net is:%d\n",
	    maxpinS ) ;

binpenalG = 0 ;
rowpenalG = 0 ;
penaltyG = 0 ;

/* set up the initial bin-penalty */
binptrG = ( BINPTR ** ) Ysafe_malloc( (numRowsG + 1 ) *
			      sizeof( BINPTR * ) ) ;

for( block = 1 ; block <= numRowsG ; block++ ) {

    binptrG[block] = (BINPTR * ) Ysafe_malloc( (numBinsG + 1) *
				      sizeof( BINPTR ) ) ;
    left  = barrayG[ block ]->bleft + barrayG[ block ]->bxcenter ;
    right = barrayG[ block ]->bleft + barrayG[ block ]->bxcenter 
				   + barrayG[ block ]->desire   ;

/* ----------------------------------------------------------------
   SetBin(x) = ((numBinsG = ((x) - binOffstG) / binWidthG))
   ---------------------------------------------------------------- */
    LoBin = SetBin( left ) ;
    HiBin = SetBin( right ) ;
    
    for( bin = 0 ; bin <= numBinsG ; bin++ ) {
	binptrG[block][bin] = (BINBOX *) Ysafe_malloc( 
					    sizeof(BINBOX) ) ; 
	binptrG[block][bin]->cell = (INT *)Ysafe_malloc( 
					    10 * sizeof(INT) );

/* ------------------------------------------------------------------
   This structure is a little tricky to understand. The structure can
   be thought of as rows of bins, one for each block. Each row of 
   bins is a structure which has five members left, right, *cell, 
   penalty, and nupenalty ...
   ------------------------------------------------------------------ */
	bptr = binptrG[block][bin] ;
	bptr->cell[0] = 0 ;
	if( Equal_Width_CellsG ){
	    bptr->right = binOffstG + (bin+1) * binWidthG ;
	} else {
	    bptr->right = binOffstG + bin * binWidthG ;
	}
	bptr->left  = bptr->right - binWidthG ;
	if( !(Equal_Width_CellsG )){
	    if( bin == LoBin ) {
		bptr->penalty = left - bptr->right ;
	    } else if( bin == HiBin ) {
		bptr->penalty = bptr->left - right ;
	    } else if( bin > HiBin || bin < LoBin ) {
		bptr->penalty = 0 ;
	    } else {
		bptr->penalty = - binWidthG ;
	    }
	} else {
	    bptr->penalty = 0 ;
	}
    }
}

if( Equal_Width_CellsG ){
    spread_equal_cells() ;
} else {
    spread_cells() ;
}

/* 
if( gate_arrayG ) {
    install_clusters() ;
}
*/

if( Equal_Width_CellsG ){

	rowpenalG = 0 ;
	binpenalG = 0 ;

} else {

    for( block = 1 ; block <= numRowsG ; block++ ) {
	for( bin = 0 ; bin <= numBinsG ; bin++ ) {
	    val = binptrG[block][bin]->penalty ;
	    binpenalG += ABS( val ) ;
	}
    }
    for( blk = 1 ; blk <= numRowsG ; blk++ ) {
	rowpenalG += ABS(barrayG[blk]->oldsize - barrayG[blk]->desire) ;
    }

} /* end else Equal_Width_CellsG */

penaltyG = (INT)( binpenConG * (DOUBLE) binpenalG + 
				roLenConG * (DOUBLE) rowpenalG ) ;

/* ************** place the pads ****************** */
placepads() ;
cost = recompute_wirecost() ;

/* ************* now calculate the timing penalty ************** */
timingcostG = 0 ;
for( pathcount = 1 ; pathcount <= numpathsG ; pathcount++ ) {

    path = patharrayG[pathcount] ;
    length = 0 ;
    ASSERTNCONT( path, "findcost", "pointer to path is NULL" ) ;
    /* for all nets k of a path i */
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
	dimptr = netarrayG[net] ;
	/* accumulate length of path */
	length = length + (INT) 
	    (horizontal_path_weightG * (DOUBLE) dimptr->halfPx);
	length = length + (INT)
	    (vertical_path_weightG * (DOUBLE) dimptr->halfPy ) ;
    }
    /* save result */
    path->path_len = path->new_path_len = length ;

    /* calculate penalty */
    /* no penalty if within target window */
    /* lowerbound <= length <= upperbound */
    if( length > path->upper_bound ){
	timingcostG += length - path->upper_bound ; 
    } else if( length < path->lower_bound ){
	timingcostG += path->lower_bound - length ;
    }
}

Ysafe_free( adjust_left );

find_net_sizes() ;
/* initialize memory for dimbox by using maxpins on a single net */
init_dimbox() ;

if( connection_machineG == 2 ) {
    create_cell() ;
}

insert_row(0) ;

/* allocate space for counting feed thrus in the rows */
feeds_in_rowG = (INT *) Ysafe_calloc( 1+numRowsG,sizeof(INT) );

return( cost ) ;
} /* end findcost */

static spread_equal_cells()
{

FILE *tp ;
CBOXPTR cellptr1 ;
TIBOXPTR tileptr1 ;
INT bin, cell ;
INT startx , endx ;
INT kk , k , cbin ;
INT LoBin , HiBin ;
INT block, x_cxcenter ;
BOOL BIN_FOUND ;

D( "equal_width_cells",
    tp = TWOPEN ("bin_placement", "w", ABORT);
) ;

for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {

    cellptr1 = carrayG[ cell ] ;
    tileptr1 = cellptr1->tileptr ;
    block = cellptr1->cblock ;

    startx = cellptr1->cxcenter + tileptr1->left  ;
    endx   = cellptr1->cxcenter + tileptr1->right ;

    x_cxcenter = cellptr1->cxcenter ;
    barrayG[block]->oldsize += endx - startx ;

    cbin  = SetBin( cellptr1->cxcenter ) ; /*-- get the cell's bin --*/
    LoBin = SetBin( startx ) ;
    HiBin = SetBin( endx ) ;

    /* Bins to check are: 0 and LoBin thru HiBin */
    /* first check the latter range */



    /*-- k = number of cells in a bin  --*/ 
    k = (binptrG[block][cbin]->cell[0]) ;
 
    if (k == 0) {
      /*---- Only for the empty bin ----*/
      /*---- cell[k] contains the cell number in "cbin" ----*/
      ++(binptrG[block][cbin]->cell[0])  ;
      binptrG[block][cbin]->cell[++k] = cell ;
      cellptr1->cxcenter = (INT) ((binptrG[block][cbin]->right + binptrG[block][cbin]->left) / (float) 2) ;  /*-- hash the cell to the bin --*/
      D( "equal_width_cells",
	fprintf(tp,
	    "\nCell#=%3d -- Block#=%d adjusted into Bin#=%2d x_cxenter=%4d  cxcenter=%4d",
	    cell, block, cbin, x_cxcenter, cellptr1->cxcenter);
      ) ;

    } else { /*-- throw the cell into an empty bin --*/

        BIN_FOUND = FALSE ;
        for (block = 1 ; block <= numRowsG ; block++) {
	  for (bin = 0 ; bin <= numBinsG ; bin++) {
	     kk = (binptrG[block][bin]->cell[0])  ; 
	     if (kk == 0) {
                ++(binptrG[block][bin]->cell[0])  ;	     
	         binptrG[block][bin]->cell[++kk] = cell  ;
                 cellptr1->cxcenter = (INT) ((binptrG[block][cbin]->right + binptrG[block][bin]->left) / 2) ;  /*-- hash the cell to the bin --*/
		 D( "equal_width_cells",
		    fprintf(tp,
		    "\nCell#=%3d -- Block#=%d placed into Bin#=%2d x_cxenter=%4d  cxcenter=%4d",
		    cell, block, bin, x_cxcenter, cellptr1->cxcenter);
		) ;
		BIN_FOUND = TRUE ;
		break ;
	      }
	     
	     if(BIN_FOUND){
	       break ;
	     } 

	   }
	}
      }/*-- end else --*/ 
}

/*--------------------------------------------------------*/

D( "equal_width_cells",
    fprintf(tp,"\n\n") ;
    TWCLOSE(tp);
) ;

} /* end spread_equal_cells */




static spread_cells()
{

INT bin, cell ;
INT startx , endx ;
INT k , cbin ;
INT LoBin , HiBin ;
INT block ;
CBOXPTR cellptr1 ;
BINPTR bptr ;
TIBOXPTR tileptr1 ;

for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {

    cellptr1 = carrayG[ cell ] ;
    tileptr1 = cellptr1->tileptr ;
    block = cellptr1->cblock ;

    startx = cellptr1->cxcenter + tileptr1->left  ;
    endx   = cellptr1->cxcenter + tileptr1->right ;

    barrayG[block]->oldsize += endx - startx ;

    cbin  = SetBin( cellptr1->cxcenter ) ;
    LoBin = SetBin( startx ) ;
    HiBin = SetBin( endx ) ;
    /* Bins to check are: 0 and LoBin thru HiBin */
    /* first check the latter range */

    k = ++(binptrG[block][cbin]->cell[0]) ;
    if( k % 10 == 0 ) {
	binptrG[block][cbin]->cell = (INT *) Ysafe_realloc(
	      binptrG[block][cbin]->cell, (k + 10) * sizeof( INT ) ) ;
    }
    binptrG[block][cbin]->cell[k] = cell ;
    if( LoBin == HiBin ) {
	binptrG[block][LoBin]->penalty +=  ( endx - startx ) ;
    } else {
	bptr = binptrG[block][LoBin] ;
	bptr->penalty += ( bptr->right - startx ) ;

	bptr = binptrG[block][HiBin] ;
	bptr->penalty +=  ( endx - bptr->left ) ;

	if( LoBin + 1 < HiBin ) {
	    for( bin = LoBin + 1 ; bin <= HiBin - 1 ; bin++ ) {
		binptrG[block][bin]->penalty += binWidthG ;
	    }
	}
    }
}
} /* end spread_cells() */


  

create_cell( )
{ 

FILE *fpr ;
char nfilename[128] ;
INT pinsep , pin_pos ;
INT count , n , j ;
INT distance , cellwidth ;
INT cell , block ;
TIBOXPTR tile ;
CBOXPTR ptr ;
PINBOXPTR pin  ;
 
sprintf( nfilename , "n%s.cel" , cktNameG ) ;
fpr = TWOPEN ( nfilename , "w", ABORT ) ;
block = 1 ;
count = 0 ;
j = 0 ;
cellwidth = (INT)( mean_widthG ) ;
if( cellwidth != cellwidth / 2 * 2 ) {
    cellwidth++ ;
}
for( cell = 1 ; cell <= numcellsG - extra_cellsG ; cell++ ) {
    ptr = carrayG[ cell ] ;
    tile = ptr->tileptr ;
    fprintf( fpr, "\ncell %d %s\n", cell , ptr->cname ) ;
    distance = j++ * cellwidth ;
    fprintf( fpr , "initially nonfixed %d from left of block %d\n",
	     distance , block ) ;
    fprintf( fpr , "left %d right %d bottom %d top %d\n" ,
	 -cellwidth / 2, cellwidth / 2 , tile->bottom , tile->top ) ;
    n = 0 ;
    for( pin = ptr->pins ; pin ; pin = pin->nextpin ){
	n++ ;
    }
    pinsep = cellwidth / (2*n+1) ;
    n = 0 ;
    for( pin = ptr->pins ; pin ; pin = pin->nextpin ){
	pin_pos = ( ++n ) * pinsep - cellwidth / 2  ;

	fprintf( fpr, "pin name %d signal %s %d %d\n", 
		 2*pin->terminal, 
		 netarrayG[pin->net]->name,
		 pin_pos , tile->top ) ;
	fprintf( fpr , "equiv name %d %d %d\n", 
		 2*pin->terminal + 1 ,pin_pos ,
		 -tile->top ) ;

	pin_pos = ( ++n ) * pinsep - cellwidth / 2  ;

	fprintf( fpr, "pin name %d signal TW_PASS_THRU %d %d\n",
		 2 * ( maxtermG + ++count ) , pin_pos ,
						 tile->top );
	fprintf( fpr, "equiv name %d %d %d\n",
		     2 * ( maxtermG + count ) + 1 , pin_pos ,
						     -tile->top);
    }

    if( distance >= barrayG[block]->blength && block != numRowsG ) {
	block++ ;
	j = 0 ;
    }
}
TWCLOSE( fpr ) ;
printf( "%s  has been established\n", nfilename ) ; 
exit( 0 ) ;
}





find_net_sizes()
{

PINBOXPTR netptr ;
DBOXPTR dimptr ;
INT i , net , *net_size , limit , *num_nets_of_size , num_nets ;
INT j , total , cell ;

limit = 6 ;
num_nets = 0 ;
net_size = (INT *) Ysafe_malloc( (limit + 2) * sizeof(INT) ) ;
num_nets_of_size = (INT *) Ysafe_malloc( (limit + 1) * sizeof(INT) ) ;
for( i = 0 ; i <= limit ; i++ ) {
    num_nets_of_size[i] = 0 ;
}
for( net = 1 ; net <= numnetsG ; net++ ) {
    dimptr = netarrayG[net] ;
    for( i = 1 ; i <= limit + 1 ; i++ ) {
	net_size[i] = 0 ;
    }
    for( netptr = dimptr->pins ; netptr ; netptr = netptr->next ){
	cell = netptr->cell ;
	if( cell > numcellsG ) {
	    continue ;
	}
	for( i = 1 ; i <= limit ; i++ ) {
	    if( cell == net_size[i] || net_size[i] == 0 ) {
		break ;
	    }
	}
	if( i > limit ) {
	    break ;
	} else {
	    net_size[i] = cell ;
	}
    }
    for( i = 1 ; i <= limit ; i++ ) {
	if( net_size[i+1] == 0 ) {
	    break ;
	}
    }
    if( i >= 2 ) {
	num_nets++ ;
	num_nets_of_size[i]++ ; 
    }
}
fprintf(fpoG,"\n");
for( i = 2 ; i <= limit ; i++ ) {
    total = 0 ;
    for( j = i ; j <= limit ; j++ ) {
	total += num_nets_of_size[j] ;
    }
    fprintf(fpoG,
	"Percentage of Nets Connecting to at least %d cells:%4.2f\n",
			    i , (DOUBLE) total / (DOUBLE) num_nets ) ;
}
fprintf(fpoG,"\n");
fflush(fpoG) ;

Ysafe_free( net_size ) ;
Ysafe_free(  num_nets_of_size ) ;

return ;
}

get_max_pin()
{
    return( maxpinS ) ;
} /* end get_max_pin */

set_print_pin( pins )
INT pins ;
{
    print_pinS = pins ;
} /* end print_one_pin_nets */
