/*
 *   Copyright (C) 1991 Yale University
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
FILE:	    watesides.c                                       
DESCRIPTION:weight the tile sides for wire estimation.
CONTENTS:   watesides
DATE:	    Mon Feb  4 02:25:16 EST 1991 - added header and new
		wire estimator.
REVISIONS:  
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) watesides.c version 3.5 2/4/91" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

watesides( ptr, pSideArray )
CELLBOXPTR ptr ;
PSIDEBOX   *pSideArray ;
{

TILEBOXPTR tileptr ;
INT side , left , right , bottom , top ;

for( tileptr = ptr->tiles;tileptr; tileptr = tileptr->next ) {
    left = ptr->xcenter + tileptr->left ;
    tileptr->lweight = -1.0 ;
    for( side = 1 ; side <= ptr->numsides ; side++ ) {
	if( pSideArray[side].vertical == 1 ) {
	    if( pSideArray[side].position == left ) {
		if( new_wire_estG ){ 
		    tileptr->lweight = pSideArray[side].pincount + 1 ;
		    if( tileptr->lweight == 0.0 ){
			tileptr->lweight = 1.0 ;
		    }
		} else {
		    tileptr->lweight = pSideArray[side].pincount / 
			(DOUBLE) pSideArray[side].length ; 
		}
		ptr->vertices->numpins[side] = (FLOAT)
		    pSideArray[side].pincount;
	    }
	}
    }
    right = ptr->xcenter + tileptr->right ;
    tileptr->rweight = -1.0 ;
    for( side = 1 ; side <= ptr->numsides ; side++ ) {
	if( pSideArray[side].vertical == 1 ) {
	    if( pSideArray[side].position == right ) {
		if( new_wire_estG ){ 
		    tileptr->rweight = pSideArray[side].pincount + 1 ;
		    if( tileptr->rweight == 0.0 ){
			tileptr->rweight = 1.0 ;
		    }
		} else {
		    tileptr->rweight = pSideArray[side].pincount / 
			(DOUBLE) pSideArray[side].length ; 
		}
		ptr->vertices->numpins[side] = (FLOAT)
		    pSideArray[side].pincount;
	    }
	}
    }
    bottom = ptr->ycenter + tileptr->bottom ;
    tileptr->bweight = -1.0 ;
    for( side = 1 ; side <= ptr->numsides ; side++ ) {
	if( pSideArray[side].vertical == 0 ) {
	    if( pSideArray[side].position == bottom ) {
		if( new_wire_estG ){ 
		    tileptr->bweight = pSideArray[side].pincount + 1 ; 
		    if( tileptr->bweight == 0.0 ){
			tileptr->bweight = 1.0 ;
		    }
		} else {
		    tileptr->bweight = pSideArray[side].pincount / 
			(DOUBLE) pSideArray[side].length ; 
		}
		ptr->vertices->numpins[side] = (FLOAT)
		    pSideArray[side].pincount;
	    }
	}
    }
    top = ptr->ycenter + tileptr->top ;
    tileptr->tweight = -1.0 ;
    for( side = 1 ; side <= ptr->numsides ; side++ ) {
	if( pSideArray[side].vertical == 0 ) {
	    if( pSideArray[side].position == top ) {
		if( new_wire_estG ){ 
		    tileptr->tweight = pSideArray[side].pincount + 1 ; 
		    if( tileptr->tweight == 0.0 ){
			tileptr->tweight = 1.0 ;
		    }
		} else {
		    tileptr->tweight = pSideArray[side].pincount / 
			(DOUBLE) pSideArray[side].length ; 
		}
		ptr->vertices->numpins[side] = (FLOAT)
		    pSideArray[side].pincount;
	    }
	}
    }
}
return ;

}
