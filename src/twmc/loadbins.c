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
FILE:	    loadins.c                                       
DESCRIPTION:initializes bins for overlap calculation
CONTENTS:   loadbins( )
	    INT calc_cellareas( BOOL ) - returns total cell area.
DATE:	    Feb 13, 1988 
REVISIONS:  Nov 3,  1988 - added routingFlag override for doPartitionG
		in calc_cell_areas and turned wire estimator off for
		cell partitioning case.
	    May 11, 1989 - eliminated routing Flag override.
	    Mon Feb  4 02:11:27 EST 1991 - added new wire estimator function.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) loadbins.c version 3.4 2/4/91" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

static INT binAreaS ;

/* conditional compile for bintest */
/* if defined the binpenalty should always remain equal to cell area */
/* #define BINTEST */

loadbins(wireAreaKnown)
BOOL wireAreaKnown ;
{

CELLBOXPTR ptr ;
BINBOXPTR bptr;
INT cell ;
INT x, y ;
INT binX, binY, array_limit ;
INT *cellList ;


/* belows set up the initial bin-penalty */
/* bins are set up as follows */
/* 0 : border , 1..maxBinXG-1 : cells, maxBinXG : border */

/* bins for chip area are empty to start with */
for( x = 1 ; x < maxBinXG  ; x++ ) {
    for( y = 1 ; y < maxBinYG ; y++ ) {
	bptr = binptrG[x][y] ;
	bptr->left  = binXOffstG + x * binWidthXG ;
	bptr->right = bptr->left + binWidthXG ;
	bptr->bottom = binYOffstG + y * binWidthYG ;
	bptr->top = bptr->bottom + binWidthYG ;
	bptr->penalty = -binWidthXG * binWidthYG ;
    }
}
/* compensate for coreWidth != to number of bins * binWidth */
for( x = 1 ; x < maxBinXG  ; x++ ) {
    bptr = binptrG[x][maxBinYG-1] ;
    bptr->penalty += binWidthXG * (bptr->top - 1 - blocktG) ;
}
for( y = 1 ; y < maxBinYG  ; y++ ) {
    bptr = binptrG[maxBinXG-1][y] ;
    bptr->penalty += (bptr->right - 1 - blockrG) * binWidthYG ;
}

/* bins boarders of chip area are filled */
/* Bottom bins */
for( x = 0 ; x <= maxBinXG ; x++ ) {
    bptr = binptrG[x][0] ;
    bptr->left  = binXOffstG + x * binWidthXG ;
    bptr->right = bptr->left + binWidthXG ;
    bptr->bottom = INT_MIN ;
    bptr->top = binYOffstG + binWidthYG ;
    bptr->penalty = 0 ;
}
/* Top bins */
for( x = 0 ; x <= maxBinXG ; x++ ) {
    bptr = binptrG[x][maxBinYG] ;
    bptr->left  = binXOffstG + x * binWidthXG ;
    bptr->right = bptr->left + binWidthXG ;
    bptr->bottom = binYOffstG + maxBinYG * binWidthYG ;
    bptr->top = INT_MAX ;
    bptr->penalty = 0 ;
}
/* Left bins */
for( y = 0 ; y <= maxBinYG ; y++ ) {
    bptr = binptrG[0][y] ;
    bptr->left  = INT_MIN ;
    bptr->right = binXOffstG + binWidthXG ;
    bptr->bottom = binYOffstG + y * binWidthYG ;
    bptr->top = bptr->bottom + binWidthYG ;
    bptr->penalty = 0 ;
}
/* Right bins */
for( y = 0 ; y <= maxBinYG ; y++ ) {
    bptr = binptrG[maxBinXG][y] ;
    bptr->left  = binXOffstG + maxBinXG * binWidthXG ;
    bptr->right = INT_MAX ;;
    bptr->bottom = binYOffstG + y * binWidthYG ;
    bptr->top = bptr->bottom + binWidthYG ;
    bptr->penalty = 0 ;
}
/* fix four corners of bins */
binptrG[0][0]->bottom = INT_MIN ;
binptrG[0][maxBinYG]->top = INT_MAX ;
binptrG[maxBinXG][0]->bottom = INT_MIN ;
binptrG[maxBinXG][maxBinYG]->top = INT_MAX ;

/* now calculate the area of the current bins */
/* also reset number of cells in the bins to zero */
binAreaS = 0 ;
for( x= 0 ; x <= maxBinXG; x++ ) {
    for( y = 0 ; y <= maxBinYG; y++ ) {
	bptr = binptrG[x][y] ;
	binAreaS += ABS( bptr->penalty ) ;
	bptr->cells[0] = 0 ;
    }
}

#ifdef BINTEST
/* ----------------------------------------------------------------- 
   This code test whether overlap code is performing correctly. 
   Initialize all bins to full (0) and overlap penalty should be
   constant over all calls to overlap functions.               */
for( x = 1 ; x <= maxBinXG  ; x++ ) {
    for( y = 1 ; y <= maxBinYG ; y++ ) {
	bptr = binptrG[x][y] ;
	bptr->penalty = 0 ;
    }
}
#endif 

/* depending on state turn on wire estimation */
/* this toggles correct bin calculation function */
if( wireAreaKnown ){
    turn_wireest_on(FALSE) ;
} else {
    turn_wireest_on(TRUE) ;
}
#ifdef BINTEST
    turn_wireest_on(FALSE) ;
#endif 

/* calculate initial overlap by adding cells to bins */
for( cell = 1 ; cell <= numcellsG; cell++ ) {

    ptr = cellarrayG[cell] ;

    setup_Bins( ptr, ptr->xcenter, ptr->ycenter, ptr->orient ) ;

    /* now safe to call add2bin */
    /* use new_apos as the move record for initialization */
    add2bin( new_aposG ) ; 

    /* add cell to bincell list - only use center of cell */
    binX = SETBINX(ptr->xcenter) ;
    binY = SETBINY(ptr->ycenter) ;
    bptr = binptrG[binX][binY] ;
    cellList = bptr->cells ;
    /* add cell to bin - check space */
    array_limit = ++cellList[0] ;
    if( array_limit >= bptr->space ) {
	bptr->space += EXPCELLPERBIN ;
	cellList = bptr->cells = (INT *) Ysafe_realloc( bptr->cells,
	    bptr->space * sizeof(INT) ) ; 
    }
    cellList[array_limit] = cell ;


}

/* now calculate the current penalty with cells added */
binpenalG = 0 ;
for( x= 0 ; x <= maxBinXG; x++ ) {
    for( y = 0 ; y <= maxBinYG; y++ ) {
	bptr = binptrG[x][y] ;
	binpenalG += ABS( bptr->penalty ) ;
    }
}
/* scale penalty */
penaltyG = (INT) ( lapFactorG * sqrt( (DOUBLE) binpenalG ) ) ;

return ;
} /* end loadbins */


/* ***************************************************************** 
   CALCULATE CELL AREAS
*/
INT calc_cellareas( routingFlag )
BOOL routingFlag ;
{
    INT totArea, orient, cell, l, r, b, t, xc, yc ;
    CELLBOXPTR  cellptr ;
    TILEBOXPTR  tileptr ;
    MOVEBOX pos_buf ;
    MOVEBOXPTR pos ;

    totArea = 0 ;
    pos = &pos_buf ;
    if( routingFlag == TRUE ){
	for( cell = 1 ; cell <= numcellsG ; cell++ ) {
	    cellptr = cellarrayG[cell] ;
	    xc = cellptr->xcenter ;
	    yc = cellptr->ycenter ;
	    for( tileptr=cellptr->tiles;tileptr;tileptr=tileptr->next ){
		/* need to calculate orientation */
		pos->l = tileptr->left ;
		pos->r = tileptr->right ;
		pos->b = tileptr->bottom ;
		pos->t = tileptr->top ;
		pos->lw = tileptr->lweight ;
		pos->rw = tileptr->rweight ;
		pos->bw = tileptr->bweight ;
		pos->tw = tileptr->tweight ;
		Ytranslate( &(pos->l),&(pos->b),&(pos->r),&(pos->t),
		    cellptr->orient) ;
		Ytranslatef( &(pos->lw),&(pos->bw),&(pos->rw),&(pos->tw),
		    cellptr->orient) ;

		/* then add wire estimation dynamically */
		if( new_wire_estG ){
		    wireestxy2( pos,xc,yc ) ;
		} else {
		    wireestxy( pos,xc,yc ) ;
		} 

		totArea += (pos->r - pos->l) * (pos->t - pos->b) ;
	    } 
	}
    } else {
	for( cell = 1 ; cell <= numcellsG ; cell++ ) {
	    cellptr = cellarrayG[cell] ;
	    for( tileptr=cellptr->tiles;tileptr;tileptr=tileptr->next ){
		l = tileptr->left   ;
		r = tileptr->right  ;
		b = tileptr->bottom ;
		t = tileptr->top    ;
		totArea += (r - l) * (t - b) ;
	    } 
	}
    }
    return( totArea ) ;

} /* end calc_cellarea */

/* ***************************************************************** 
   RETURN TOTAL BIN AREA.  Note: could make totArea a global but
   use static to make code more modular.  Only called a few times
   no problem with inefficiency.
*/
INT get_bin_area() 
{
    return( binAreaS ) ;
}
