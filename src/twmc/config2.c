/*
 *   Copyright (C) 1988-1990 Yale University
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
FILE:	    config2.c                                       
DESCRIPTION:configures initial macro placement
CONTENTS:   config2( INT, INT, double )
DATE:	    Jan 30, 1988 
REVISIONS:
	    Feb 13, 1988 - passed modified number of bins to routine
		This takes into account only number of cell bins
		not border bins.
	    Mar 30, 1989 - changed tile datastructure.
	    Apr 28, 1989 - changed arguments to config2.
	    Jun 15, 1989 - now call reconfig for common code.
	    Oct  3, 1989 - made totalArea a DOUBLE.
	    Oct 20, 1989 - now take average of track pitch in both
		directions.
	    Apr 23, 1990 - Now calculate total area based on all 
		instances so that the wire estimation will be correct.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) config2.c version 3.3 9/5/90" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

config2( numbinX, numbinY, wire_red_ratio )
INT numbinX ;
INT numbinY ;
DOUBLE wire_red_ratio ;
{

    INT inst ;                      /* counter */
    INT numinst ;                   /* number of cell instances */
    INT l, r, b, t, cell, tempint, side, soft_count ;
    CELLBOXPTR cellptr ;
    TILEBOXPTR tileptr ;
    DOUBLE totalArea, temp ;
    DOUBLE temp1, fraction ;
    INSTBOXPTR instptr ;


    /*
     *   Sum the areas of the cells over all instances
     */
    totalArea = 0.0 ;
    for( cell = 1 ; cell <= numcellsG ; cell++ ) {
	cellptr = cellarrayG[cell] ;
	/* look at all the instances */
	if( instptr = cellptr->instptr ){
	    numinst = instptr->numinstances ;
	} else {
	    numinst = 1 ;
	}
	for( inst = 0 ; inst < numinst; inst++ ){
	    if( instptr ){
		tileptr = instptr->tile_inst[inst] ;
	    } else {
		tileptr = cellptr->tiles ;
	    }
	    for( ; tileptr ; tileptr = tileptr->next ) {
		l = tileptr->left   ;
		r = tileptr->right  ;
		b = tileptr->bottom ;
		t = tileptr->top    ;
		totalArea += (DOUBLE) (r - l) * (t - b) ;
	    }
	}
    }

    temp = avg_funcG / wire_red_ratio ;
    totNetLenG = (INT) temp ;
    /* calculate number of softiles */
    soft_count=0 ;
    for( cell = 1; cell <= numcellsG ; cell++ ) {
	if( cellarrayG[cell]->softflag == 1 ) {
	    soft_count++ ;
	}
    }
    fraction = (DOUBLE) soft_count / (DOUBLE) numcellsG ;
    temp1 = (DOUBLE) (totNetLenG) / 
		(1.0 + (fraction / (sqrt( (DOUBLE) numcellsG )))) ;
    totNetLenG = (INT) temp1 ;

    if( coreGivenG == 0 ) {
	blockrG = blocktG = (INT) sqrt( totalArea ) ;
	totChanLenG = perimG / 2 - (blockrG + blocktG) ;
	temp = ((DOUBLE) totNetLenG / (DOUBLE) totChanLenG) /
		    ((DOUBLE) layersFactorG) ; 
    } else {
	r = t = (INT) sqrt( totalArea ) ;
	totChanLenG = perimG / 2 - (r + t) ;
	temp = ((DOUBLE) totNetLenG / (DOUBLE) totChanLenG) /
		    ((DOUBLE) layersFactorG) ; 
    }

    side = (INT)(sqrt(((DOUBLE) totalArea) / (DOUBLE) numcellsG))  ;
    OUT2("\n\nAVERAGE CELL SIDE WITHOUT ROUTING AREA: %d\n\n",side );

    tempint = (INT)( temp ) + 3 ; /* d+1 tracks + roundoff */
    tempint += defaultTracksG ;
    tempint *= (track_spacingXG + track_spacingYG) / 2 ;

    aveChanWidG = ( (DOUBLE) tempint / 1.8) + 2.0 ;

    OUT2("\n\nConfiguration Data\nInternal Channel Length:%d\n",
							    totChanLenG ) ;
    OUT2("Average Channel Width (un-normalized):%f\n\n", temp ) ;
    OUT2("Average Channel Width:%f\n\n", aveChanWidG ) ;

    reconfigure( numbinX, numbinY, (DOUBLE) totalArea ) ;

    return ;
}
