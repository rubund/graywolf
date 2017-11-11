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
CONTENTS:   config2( int, int, double )
DATE:	    Jan 30, 1988 
REVISIONS:
	    Feb 13, 1988 - passed modified number of bins to routine
		This takes into account only number of cell bins
		not border bins.
	    Mar 30, 1989 - changed tile datastructure.
	    Apr 28, 1989 - changed arguments to config2.
	    Jun 15, 1989 - now call reconfig for common code.
	    Oct  3, 1989 - made totalArea a double.
	    Oct 20, 1989 - now take average of track pitch in both
		directions.
	    Apr 23, 1990 - Now calculate total area based on all 
		instances so that the wire estimation will be correct.
----------------------------------------------------------------- */
#include "allheaders.h"

void config2( int numbinX, int numbinY, double wire_red_ratio )
{

    int inst ;                      /* counter */
    int numinst ;                   /* number of cell instances */
    int l, r, b, t, cell, tempint, side, soft_count ;
    CELLBOXPTR cellptr ;
    TILEBOXPTR tileptr ;
    double totalArea, temp ;
    double temp1, fraction ;
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
		totalArea += (double) (r - l) * (t - b) ;
	    }
	}
    }

    temp = avg_funcG / wire_red_ratio ;
    totNetLenG = (int) temp ;
    /* calculate number of softiles */
    soft_count=0 ;
    for( cell = 1; cell <= numcellsG ; cell++ ) {
	if( cellarrayG[cell]->softflag == 1 ) {
	    soft_count++ ;
	}
    }
    fraction = (double) soft_count / (double) numcellsG ;
    temp1 = (double) (totNetLenG) / 
		(1.0 + (fraction / (sqrt( (double) numcellsG )))) ;
    totNetLenG = (int) temp1 ;

    if( coreGivenG == 0 ) {
	blockrG = blocktG = (int) sqrt( totalArea ) ;
	totChanLenG = perimG / 2 - (blockrG + blocktG) ;
	temp = ((double) totNetLenG / (double) totChanLenG) /
		    ((double) layersFactorG) ; 
    } else {
	r = t = (int) sqrt( totalArea ) ;
	totChanLenG = perimG / 2 - (r + t) ;
	temp = ((double) totNetLenG / (double) totChanLenG) /
		    ((double) layersFactorG) ; 
    }

    side = (int)(sqrt(((double) totalArea) / (double) numcellsG))  ;
    printf("\n\nAVERAGE CELL SIDE WITHOUT ROUTING AREA: %d\n\n",side );

    tempint = (int)( temp ) + 3 ; /* d+1 tracks + roundoff */
    tempint += defaultTracksG ;
    tempint *= (track_spacingXG + track_spacingYG) / 2 ;

    aveChanWidG = ( (double) tempint / 1.8) + 2.0 ;

    printf("\n\nConfiguration Data\nInternal Channel Length:%d\n",
							    totChanLenG ) ;
    printf("Average Channel Width (un-normalized):%f\n\n", temp ) ;
    printf("Average Channel Width:%f\n\n", aveChanWidG ) ;

    reconfigure( numbinX, numbinY, (double) totalArea ) ;

    return ;
}
