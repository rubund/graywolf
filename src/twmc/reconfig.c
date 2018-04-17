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
FILE:	    reconfig.c                                       
DESCRIPTION:reconfigures the core area during simulated annealing
	    run.  It is called from uloop by the core area controller.
CONTENTS:   reconfigure( numbinX, numbinY, newCoreArea )
		INT numbinX, numbinY, newCoreArea ;
DATE:	    Sept 13, 1988 
REVISIONS:  Jan  20, 1989 - added findcost call to update new wirelength
		after reconfiguration.
	    Mon Feb  4 02:18:42 EST 1991 - update for wire estimation
		algorithm.
	    Thu Apr 18 01:26:43 EDT 1991 - grid cells if requested.
	    Sun May  5 14:23:51 EDT 1991 - added reorigin and now
		allow user to set origin.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) reconfig.c version 3.6 5/5/91" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

#define UPDATE  (BOOL)  FALSE /* don't initialize updateFixedCells */

reconfigure( numbinX, numbinY, newCoreArea )
INT numbinX ;
INT numbinY ;
DOUBLE newCoreArea ;
{

    DOUBLE factor ;
    INT xlength, ylength ;

    if( coreGivenG == FALSE ) {
	
	blockrG = blocktG = (INT)( sqrt( newCoreArea ) ) + 1 ;
	/* 
	 *    Take into account the aspect ratio requested by the user
	 */
	blocktG = (INT)( sqrt(chipaspectG) * (DOUBLE) blocktG ) + 1 ;
	blockrG = (INT)( 1.0 / sqrt(chipaspectG) * (DOUBLE) blockrG ) + 1;
    }

    xlength = blockrG - blocklG ;
    ylength = blocktG - blockbG ;

    slopeXG = (DOUBLE)(maxWeightG - baseWeightG) / 
		( (DOUBLE) xlength * 0.5 ) ;
    slopeYG = (DOUBLE)(maxWeightG - baseWeightG) / 
		( (DOUBLE) ylength * 0.5 ) ;
    basefactorG = (DOUBLE) baseWeightG ;
    factor = 0.5 * aveChanWidG /(DOUBLE)xlength / (DOUBLE) ylength ; 
    /* now calculate factor for x and y directions taking wiring */
    /* into account */
    wireFactorXG = factor * 2 * track_spacingXG /
			(track_spacingXG + track_spacingYG ) ;
    wireFactorYG = factor * 2 * track_spacingYG /
			(track_spacingXG + track_spacingYG ) ;

    if( coreGivenG == FALSE ){
	blocklG = x_originG ;
	blockbG = y_originG ;
	blockrG = blocklG + xlength ; 
	blocktG = blockbG + ylength ; 
    }

    if( gridGivenG ){
	grid_cells() ;
    }

    placepads() ;

    /* calculate the core configuration constants for wire estimation */
    blockmxG = (blockrG + blocklG) / 2 ;
    blockmyG = (blocktG + blockbG) / 2 ;
    halfXspanG = (blockrG - blocklG) / 2 ;
    halfYspanG = (blocktG - blockrG) / 2 ;

    binWidthXG = (blockrG - blocklG) / numbinX ;
    if( (blockrG - blocklG - binWidthXG * numbinX) >= numbinX / 2 ) {
	binWidthXG++ ;
    }
    binXOffstG = blocklG + 1 - binWidthXG ;

    binWidthYG = (blocktG - blockbG) / numbinY ;
    if( (blocktG - blockbG - binWidthYG * numbinY) >= numbinY / 2 ) {
	binWidthYG++ ;
    }
    binYOffstG = blockbG + 1 - binWidthYG ;

    updateFixedCells(UPDATE) ;  /* place fixed cells then load */
    loadbins(FALSE) ; /* wireArea not known */

    bdxlengthG = blockrG - blocklG ;
    bdylengthG = blocktG - blockbG ;

    OUT3("bdxlength:%d    bdylength:%d\n",bdxlengthG,bdylengthG);

    /* update for wire estimation algorithm */
    resize_wire_params() ;

    /* update cost */
    funccostG = findcost() ;

    return ;

} /* end reconfigure */



reorigin()
{

    CELLBOXPTR cellptr ;
    TILEBOXPTR tileptr ;
    INT l, r, b, t, cell ;
    INT xcenter, ycenter ;
    INT xorigin, yorigin ;
    INT xmax, ymax ;
    INT deltax, deltay ;

    /* find bounding box of all core cells */
    xorigin = INT_MAX ;
    yorigin = INT_MAX ;
    xmax = INT_MIN ;
    ymax = INT_MIN ;
    for( cell = 1 ; cell <= numcellsG ; cell++ ){
	cellptr = cellarrayG[cell] ;
	xcenter = cellptr->xcenter ;
	ycenter = cellptr->ycenter ;

	for( tileptr=cellptr->tiles;tileptr;tileptr = tileptr->next ) {
	    l = tileptr->left ;
	    r = tileptr->right ;
	    b = tileptr->bottom ;
	    t = tileptr->top ;

	    Ytranslate( &l,&b,&r,&t,cellptr->orient) ;
	    l += xcenter ;
	    r += xcenter ;
	    b += ycenter ;
	    t += ycenter ;
	    /* save bounding box of all tiles */
	    xorigin = MIN( l, xorigin ) ;
	    yorigin = MIN( b, yorigin ) ;
	    xmax = MAX( r, xmax ) ;
	    ymax = MAX( t, ymax ) ;
	}
    } /* bounding box computation complete */

    /* now get necessary delta to origin core cells */
    deltax = x_originG - xorigin ;
    deltay = y_originG - yorigin ;

    /* update the positions of all the cells */
    /* so that the core is origined at x_originG, y_originG */
    for( cell = 1 ; cell <= numcellsG ; cell++ ){
	cellptr = cellarrayG[cell] ;
	cellptr->xcenter += deltax ;
	cellptr->ycenter += deltax ;
    } /* end cell update */

    placepads() ;

    blockmxG = (blockrG + blocklG) / 2 ;
    blockmyG = (blocktG + blockbG) / 2 ;

    bdxlengthG = blockrG - blocklG ;
    bdylengthG = blocktG - blockbG ;

} /* end reorigin */
