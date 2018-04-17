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
FILE:	    overlap.c                                       
DESCRIPTION:This file contains routines for one and two cell overlap
	    calculation.  Overlap routines returns newpenalty. 
            This procedure works for hard and soft cells.  Hard cells
	    explicitly store their 8 orientation views whereas for
	    softcells their orientation needs to be calculated. 
CONTENTS:   overlap( )
	    overlap2( )
	    update_overlap( )
	    update_overlap2( )
	    sub_penal( MOVEBOXPTR * )
	    add_penal( MOVEBOXPTR * )
	    calc_wBins( MOVEBOXPTR * )
	    calc_nBins( MOVEBOXPTR * )
	    turn_wireest_on( INT )
	    find_orient() 
	    setup_Bins( CELLBOXPTR, int, int, INT )
	    add2bin( MOVEBOXPTR * ) 
DATE:	    Feb  2, 1988 
REVISIONS:  Feb  5, 1988 - changed old_apos, new_apos, old_bpos,
		new_bpos to global variables.  Old method remains
		commented for easier reading. 
	    Feb  8, 1988 - added setup_Bins for loadbins.c
	    Feb 12, 1988 - added add2bin for loadbins.c
	    Oct 21, 1988 - added fastbin to speed overlap calculation
	    Apr  2, 1989 - changed softcells to have multiple tiles
	    Apr  4, 1989 - added instance changes.
	    Sun Jan 20 21:34:36 PST 1991 - ported to AIX.
	    Mon Feb  4 02:15:23 EST 1991 - added new wire estimator.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) overlap.c version 3.6 4/18/91" ;
#endif

#include <custom.h>
#include <yalecad/file.h>
#include <yalecad/debug.h>

/* ----------------------------------------------------------------- 
   IMPORTANT global definitions - defined in custom.h 
   MOVEBOXPTR *old_aposG, *new_aposG, *old_bposG, *new_bposG ;
   MOVEBOXPTR old_apos0G, new_apos0G, old_bpos0G, new_bpos0G ;
   BINPTR **binptrG ;
*/

#define calcBins(a)           (  (*calc_Bins)(a) )
#define WIREESTXY(pos,xc,yc)  (  (*wire_est)(pos,xc,yc) )

/* static definitions */
static CELLBOXPTR cellptrS ;
static TILEBOXPTR tileptrS ;
static MOVEBOXPTR posS ;
static BINBOXPTR bptrS ;
static xcS, ycS, orientS ;
static INT minXS, maxXS, minYS, maxYS ;
static INT newbinpenalS ;
static INT xcostS, ycostS ;
static INT (*calc_Bins)() ;/* remember which bin function */
static INT (*wire_est)() ; /* remember which wire estimation function */

/* global references */
extern INT wireestxy( P3(MOVEBOXPTR pos,INT xc, INT yc) ) ;
extern INT wireestxy2( P3(MOVEBOXPTR pos,INT xc, INT yc) ) ;

/* ***************************************************************** 
   ONE CELL OVERLAP CALCULATION 
*/
INT overlap( /* old_aposG, new_aposG */ )
/* MOVEBOXPTR *old_aposG, *new_aposG ; */
{
    register BINBOXPTR *fastbin ;
    register x, y ;


/* ----------------------------------------------------------------- 
   Perform overlap calculation over all tiles.
   In calculating overlap OLD A and NEW A are a pair.
   CALCULATE BINS FOR OLD A POSITION 
*/
newbinpenalS = 0 ;
cellptrS = cellarrayG[old_apos0G->cell] ;
orientS  = cellptrS->orient ;
xcS = cellptrS->xcenter ;
ycS = cellptrS->ycenter ;

/* initialize bounding box comparison for each pair */
maxXS = INT_MIN ;
maxYS = INT_MIN ;
minXS = INT_MAX ;
minYS = INT_MAX ;

/* ----------------------------------------------------------------- 
   NOTE: turn_wireest_on determines whether we call calc_wBins or
   calc_nBins
*/
calcBins( old_aposG ) ;

/* ----------------------------------------------------------------- 
   CALCULATE BINS FOR NEW A POSITION 
*/
cellptrS = cellarrayG[new_apos0G->cell] ;
orientS  = new_apos0G->orient ;
xcS = new_apos0G->xcenter ;
ycS = new_apos0G->ycenter ;

calcBins( new_aposG ) ;

/* ----------------------------------------------------------------- 
   Perform overlap initialization for OLD A - NEW A pair
   minX, minY, maxX, maxY now has bounding box for this pair.
   Store result in old_apos for latter use in update overlap
*/
old_apos0G->binR = maxXS ;
old_apos0G->binL = minXS ;
old_apos0G->binT = maxYS ;
old_apos0G->binB = minYS ;

for( x = minXS; x <= maxXS ; x++ ){
    fastbin = binptrG[x] ;
    for( y = minYS; y <= maxYS ; y++ ){
	bptrS = fastbin[y] ;
	bptrS->nupenalty = bptrS->penalty ;
    }
}

/* ----------------------------------------------------------------- 
   INITIALIZATION COMPLETE.  NOW perform calculation.
   NOW any order will do.  Since cellptr is set from previous
   calculation to a do a cell calculation first.
*/

/* perform overlap calculation */
sub_penal( old_aposG ) ;
add_penal( new_aposG ) ;

return( newbinpenalS ) ;

} /* end function overlap */


/* ***************************************************************** 
   TWO CELL OVERLAP CALCULATION 
*/
INT overlap2( /* old_aposG, new_aposG, old_bposG, new_bposG */ )
/* MOVEBOXPTR *old_aposG, *new_aposG, *old_bposG, *new_bposG ; */
{
    register BINBOXPTR *fastbin ;
    register x, y ;


/* ----------------------------------------------------------------- 
   Perform overlap calculation over all tiles.
   In calculating overlap OLD A and NEW B are a pair and
   OLD B and NEW A are a pair - calculate overlap in pairs.
   We will calculate in this order:
	OLD A, NEW B, OLD B,  NEW A
   CALCULATE BINS FOR OLD A POSITION 
*/
newbinpenalS = 0 ;
cellptrS = cellarrayG[old_apos0G->cell] ;
orientS  = cellptrS->orient ;
xcS = cellptrS->xcenter ;
ycS = cellptrS->ycenter ;

/* initialize bounding box comparison for each pair */
maxXS = INT_MIN ;
maxYS = INT_MIN ;
minXS = INT_MAX ;
minYS = INT_MAX ;

calcBins( old_aposG ) ;

/* ----------------------------------------------------------------- 
   CALCULATE BINS FOR NEW B POSITION 
*/
cellptrS = cellarrayG[new_bpos0G->cell] ;
orientS  = new_bpos0G->orient ;
xcS = new_bpos0G->xcenter ;
ycS = new_bpos0G->ycenter ;

calcBins( new_bposG ) ;

/* ----------------------------------------------------------------- 
   Perform overlap initialization for OLD A - NEW B pair
   minX, minY, maxX, maxY now has bounding box for this pair.
   Store result in old_apos for latter use in update overlap
*/
old_apos0G->binR = maxXS ;
old_apos0G->binL = minXS ;
old_apos0G->binT = maxYS ;
old_apos0G->binB = minYS ;

for( x = minXS; x <= maxXS ; x++ ){
    fastbin = binptrG[x] ;
    for( y = minYS; y <= maxYS ; y++ ){
	bptrS = fastbin[y] ;
	bptrS->nupenalty = bptrS->penalty ;
    }
}

/* ----------------------------------------------------------------- 
   Perform overlap calculation over all tiles.
   CALCULATE BINS FOR OLD B POSITION 
*/
cellptrS = cellarrayG[old_bpos0G->cell] ;
orientS  = cellptrS->orient ;
xcS = cellptrS->xcenter ;
ycS = cellptrS->ycenter ;

/* initialize bounding box comparison for OLDB-NEWA pair */
maxXS = INT_MIN ;
maxYS = INT_MIN ;
minXS = INT_MAX ;
minYS = INT_MAX ;

calcBins( old_bposG ) ;

/* ----------------------------------------------------------------- 
   CALCULATE BINS FOR NEW A POSITION 
*/
cellptrS = cellarrayG[new_apos0G->cell] ;
orientS  = new_apos0G->orient ;
xcS = new_apos0G->xcenter ;
ycS = new_apos0G->ycenter ;

calcBins( new_aposG ) ;

/* ----------------------------------------------------------------- 
   Perform overlap initialization for OLD B - NEW A pair
   minX, minY, maxX, maxY now has bounding box for this pair.
   Store result in old_bpos for latter use in update overlap
*/
old_bpos0G->binR = maxXS ;
old_bpos0G->binL = minXS ;
old_bpos0G->binT = maxYS ;
old_bpos0G->binB = minYS ;

for( x = minXS; x <= maxXS ; x++ ){
    fastbin = binptrG[x] ;
    for( y = minYS; y <= maxYS ; y++ ){
	bptrS = fastbin[y] ;
	bptrS->nupenalty = bptrS->penalty ;
    }
}

/* ----------------------------------------------------------------- 
   INITIALIZATION COMPLETE.  NOW perform calculation.
   NOW any order will do.  
*/

/* perform overlap calculation */
sub_penal( old_aposG ) ;
add_penal( new_aposG ) ;
sub_penal( old_bposG ) ;
add_penal( new_bposG ) ;

return( newbinpenalS ) ;

} /* end function overlap2 */

/* ***************************************************************** 
   Update one cell move by transferring from nu to penalty fields 
*/
INT update_overlap( /* old_aposG */ )
/* MOVEBOXPTR *old_aposG ; */
{
    register BINBOXPTR *fastbin ;
    register x, y ;

/* ----------------------------------------------------------------- 
   Perform overlap update for OLD A - NEW A pair
   Recall information stored in old_apos[0] to get bounding
   box for this pair. 
*/
maxXS = old_apos0G->binR  ;
minXS = old_apos0G->binL  ;
maxYS = old_apos0G->binT  ;
minYS = old_apos0G->binB ;

for( x = minXS; x <= maxXS ; x++ ){
    fastbin = binptrG[x] ;
    for( y = minYS; y <= maxYS ; y++ ){
	bptrS = fastbin[y] ;
	bptrS->penalty = bptrS->nupenalty ;
    }
}
} /* end function update_overlap */

/* ***************************************************************** 
   Update two cell move by transferring from nu to penalty fields 
*/
INT update_overlap2( /* old_aposG, old_bposG */ )
/* MOVEBOXPTR *old_apos, *old_bpos ; */
{
    register BINBOXPTR *fastbin ;
    register x, y ;

/* ----------------------------------------------------------------- 
   Perform overlap update for OLD A - NEW B pair
   Recall information stored in old_apos[0] to get bounding
   box for this pair. 
*/
maxXS = old_apos0G->binR  ;
minXS = old_apos0G->binL  ;
maxYS = old_apos0G->binT  ;
minYS = old_apos0G->binB ;

for( x = minXS; x <= maxXS ; x++ ){
    fastbin = binptrG[x] ;
    for( y = minYS; y <= maxYS ; y++ ){
	bptrS = fastbin[y] ;
	bptrS->penalty = bptrS->nupenalty ;
    }
}

/* ----------------------------------------------------------------- 
   Perform overlap update for OLD B - NEW A pair
   Recall information stored in old_bpos[0] to get bounding
   box for this pair. 
*/
maxXS = old_bpos0G->binR  ;
minXS = old_bpos0G->binL  ;
maxYS = old_bpos0G->binT  ;
minYS = old_bpos0G->binB ;

for( x = minXS; x <= maxXS ; x++ ){
    fastbin = binptrG[x] ;
    for( y = minYS; y <= maxYS ; y++ ){
	bptrS = fastbin[y] ;
	bptrS->penalty = bptrS->nupenalty ;
    }
}
} /* end function update_overlap */

/* ***************************************************************** 
   Subtract penalty from bins.  
   Takes pointer to move box record as an argument
*/
sub_penal( cellpos )
MOVEBOXPTR *cellpos ;
{
INT count, maxcount ;
INT x, y ;

register BINBOXPTR *fastbin ;

maxcount = cellpos[0]->numtiles ;
/* subtract OLD penalty */
for( count=1 ; count <= maxcount ; count++ ) {

    posS = cellpos[count] ;
    /* assign to simple variable for speed */
    minXS = posS->binL ;
    minYS = posS->binB ;
    maxXS = posS->binR ;
    maxYS = posS->binT ;

/* ----------------------------------------------------------------- 
    Calculate penalty 
*/
    if( minXS == maxXS && minYS == maxYS ){
	/* entire cell is in bin */
	bptrS = binptrG[minXS][minYS] ;
	xcostS = posS->r - posS->l ;
	ycostS = posS->t - posS->b ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty -= xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;

    } else if( minXS == maxXS ){  /* multiple y bins */
	xcostS = posS->r - posS->l ;
	/* first bottom bin */
	fastbin = binptrG[minXS] ;
	bptrS = fastbin[minYS] ;
	ycostS = bptrS->top - posS->b ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty -= xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;
	/* next middle bins */
	for( y = minYS+1; y < maxYS ; y++ ){
	    bptrS = fastbin[y] ;
	    newbinpenalS -= ABS( bptrS->nupenalty ) ;
	    bptrS->nupenalty -= xcostS * binWidthYG ;
	    newbinpenalS += ABS( bptrS->nupenalty ) ;
	}
	/* last top bin */
	bptrS = fastbin[maxYS] ;
	ycostS = posS->t - bptrS->bottom ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty -= xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;

    } else if( minYS == maxYS ){  /* multiple x bins */
	ycostS = posS->t - posS->b ;
	/* first left bin */
	bptrS = binptrG[minXS][minYS] ;
	xcostS = bptrS->right - posS->l ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty -= xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;
	/* next middle bins */
	for( x = minXS+1; x < maxXS ; x++ ){
	    bptrS = binptrG[x][minYS] ;
	    newbinpenalS -= ABS( bptrS->nupenalty ) ;
	    bptrS->nupenalty -= binWidthXG * ycostS ;
	    newbinpenalS += ABS( bptrS->nupenalty ) ;
	}
	/* last right bin */
	bptrS = binptrG[maxXS][minYS] ;
	xcostS = posS->r - bptrS->left ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty -= xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;

    } else {  /* multiple rows and columns */
/* ----------------------------------------------------------------- 
	3 |    4          |  5
	-----------------------
	2 |    9          |  6
	-----------------------
	1 |    8          |  7
	above is order which we check overlap in the bins */
	/* first lower left bin - 1 */
	fastbin = binptrG[minXS] ;
	bptrS = fastbin[minYS] ;
	xcostS = bptrS->right - posS->l ;
	ycostS = bptrS->top - posS->b ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty -= xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;
	/* next left middle y bins - 2 */
	for( y = minYS+1; y < maxYS ; y++ ){
	    bptrS = fastbin[y] ;
	    newbinpenalS -= ABS( bptrS->nupenalty ) ;
	    bptrS->nupenalty -= xcostS * binWidthYG ;
	    newbinpenalS += ABS( bptrS->nupenalty ) ;
	}
	/* next top left bin - 3 */
	bptrS = fastbin[maxYS] ;
	ycostS = posS->t - bptrS->bottom ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty -= xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;
	/*  do top y bins - 4 */
	for( x = minXS+1; x < maxXS ; x++ ){
	    bptrS = binptrG[x][maxYS] ;
	    newbinpenalS -= ABS( bptrS->nupenalty ) ;
	    bptrS->nupenalty -= binWidthXG * ycostS ;
	    newbinpenalS += ABS( bptrS->nupenalty ) ;
	}
	/* do top right bin - 5 */
	fastbin = binptrG[maxXS] ;
	bptrS = fastbin[maxYS] ;
	xcostS = posS->r - bptrS->left ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty -= xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;
	/* do middle right bins - 6 */
	for( y = minYS+1; y < maxYS ; y++ ){
	    bptrS = fastbin[y] ;
	    newbinpenalS -= ABS( bptrS->nupenalty ) ;
	    bptrS->nupenalty -= xcostS * binWidthYG ;
	    newbinpenalS += ABS( bptrS->nupenalty ) ;
	}
	/* do lower right bin - 7 */
	bptrS = fastbin[minYS] ;
	ycostS = bptrS->top - posS->b ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty -= xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;
	/*  do bottom x bins - 8 */
	for( x = minXS+1; x < maxXS ; x++ ){
	    bptrS = binptrG[x][minYS] ;
	    newbinpenalS -= ABS( bptrS->nupenalty ) ;
	    bptrS->nupenalty -= binWidthXG * ycostS ;
	    newbinpenalS += ABS( bptrS->nupenalty ) ;
	}
	/* center middle bins - 9 */
	for( x = minXS+1; x < maxXS ; x++ ){
	    fastbin = binptrG[x] ;
	    for( y = minYS+1; y < maxYS ; y++ ){
		bptrS = fastbin[y] ;
		newbinpenalS -= ABS( bptrS->nupenalty ) ;
		bptrS->nupenalty -= binWidthXG * binWidthYG ;
		newbinpenalS += ABS( bptrS->nupenalty ) ;
	    }
	}
    }
}

} /* end subtract penalty function */

/* ***************************************************************** 
   Add penalty to bins. 
   Takes pointer to move box record as an argument
*/
add_penal( cellpos )
MOVEBOXPTR *cellpos ;
{
INT count, maxcount ;
INT x, y ;
register BINBOXPTR *fastbin ;

maxcount = cellpos[0]->numtiles ;
/* add penalty */
for( count=1 ; count <= maxcount ; count++ ) {

    posS = cellpos[count] ;
    /* assign to simple variable for speed */
    minXS = posS->binL ;
    minYS = posS->binB ;
    maxXS = posS->binR ;
    maxYS = posS->binT ;

/* ----------------------------------------------------------------- 
    Calculate penalty 
*/
    if( minXS == maxXS && minYS == maxYS ){
	/* entire cell is in bin */
	bptrS = binptrG[minXS][minYS] ;
	xcostS = posS->r - posS->l ;
	ycostS = posS->t - posS->b ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty += xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;

    } else if( minXS == maxXS ){  /* multiple y bins */
	xcostS = posS->r - posS->l ;
	/* minXS stays the same */
	fastbin = binptrG[minXS] ;
	/* first bottom bin */
	bptrS = fastbin[minYS] ;
	ycostS = bptrS->top - posS->b ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty += xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;
	/* next middle bins */
	for( y = minYS+1; y < maxYS ; y++ ){
	    bptrS = fastbin[y] ;
	    newbinpenalS -= ABS( bptrS->nupenalty ) ;
	    bptrS->nupenalty += xcostS * binWidthYG ;
	    newbinpenalS += ABS( bptrS->nupenalty ) ;
	}
	/* last top bin */
	bptrS = fastbin[maxYS] ;
	ycostS = posS->t - bptrS->bottom ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty += xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;

    } else if( minYS == maxYS ){  /* multiple x bins */
	ycostS = posS->t - posS->b ;
	/* first left bin */
	bptrS = binptrG[minXS][minYS] ;
	xcostS = bptrS->right - posS->l ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty += xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;
	/* next middle bins */
	for( x = minXS+1; x < maxXS ; x++ ){
	    bptrS = binptrG[x][minYS] ;
	    newbinpenalS -= ABS( bptrS->nupenalty ) ;
	    bptrS->nupenalty += binWidthXG * ycostS ;
	    newbinpenalS += ABS( bptrS->nupenalty ) ;
	}
	/* last right bin */
	bptrS = binptrG[maxXS][minYS] ;
	xcostS = posS->r - bptrS->left ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty += xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;

    } else {  /* multiple rows and columns */
/* ----------------------------------------------------------------- 
	3 |    4          |  5
	-----------------------
	2 |    9          |  6
	-----------------------
	1 |    8          |  7
	above is order which we check overlap in the bins */
	/* first lower left bin - 1 */
	fastbin = binptrG[minXS] ;
	bptrS = fastbin[minYS] ;
	xcostS = bptrS->right - posS->l ;
	ycostS = bptrS->top - posS->b ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty += xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;
	/* next left middle y bins - 2 */
	for( y = minYS+1; y < maxYS ; y++ ){
	    bptrS = fastbin[y] ;
	    newbinpenalS -= ABS( bptrS->nupenalty ) ;
	    bptrS->nupenalty += xcostS * binWidthYG ;
	    newbinpenalS += ABS( bptrS->nupenalty ) ;
	}
	/* next top left bin - 3 */
	bptrS = fastbin[maxYS] ;
	ycostS = posS->t - bptrS->bottom ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty += xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;
	/*  do top y bins - 4 */
	for( x = minXS+1; x < maxXS ; x++ ){
	    bptrS = binptrG[x][maxYS] ;
	    newbinpenalS -= ABS( bptrS->nupenalty ) ;
	    bptrS->nupenalty += binWidthXG * ycostS ;
	    newbinpenalS += ABS( bptrS->nupenalty ) ;
	}
	/* do top right bin - 5 */
	fastbin = binptrG[maxXS] ;
	bptrS = fastbin[maxYS] ;
	xcostS = posS->r - bptrS->left ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty += xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;
	/* do middle right bins - 6 */
	for( y = minYS+1; y < maxYS ; y++ ){
	    bptrS = fastbin[y] ;
	    newbinpenalS -= ABS( bptrS->nupenalty ) ;
	    bptrS->nupenalty += xcostS * binWidthYG ;
	    newbinpenalS += ABS( bptrS->nupenalty ) ;
	}
	/* do lower right bin - 7 */
	bptrS = fastbin[minYS] ;
	ycostS = bptrS->top - posS->b ;
	newbinpenalS -= ABS( bptrS->nupenalty ) ;
	bptrS->nupenalty += xcostS * ycostS ;
	newbinpenalS += ABS( bptrS->nupenalty ) ;
	/*  do bottom x bins - 8 */
	for( x = minXS+1; x < maxXS ; x++ ){
	    bptrS = binptrG[x][minYS] ;
	    newbinpenalS -= ABS( bptrS->nupenalty ) ;
	    bptrS->nupenalty += binWidthXG * ycostS ;
	    newbinpenalS += ABS( bptrS->nupenalty ) ;
	}
	/* center middle bins - 9 */
	for( x = minXS+1; x < maxXS ; x++ ){
	    fastbin = binptrG[x] ;
	    for( y = minYS+1; y < maxYS ; y++ ){
		bptrS = fastbin[y] ;
		newbinpenalS -= ABS( bptrS->nupenalty ) ;
		bptrS->nupenalty += binWidthXG * binWidthYG ;
		newbinpenalS += ABS( bptrS->nupenalty ) ;
	    }
	}
    }
}

} /* end add penalty function */


/* ***************************************************************** 
   Calculate bins for the given cell position.
   USES wire estimation in the calculation.
   Takes pointer to move box record as an argument
*/

INT calc_wBins( cellpos )
MOVEBOXPTR *cellpos ;
{

INT count ;
MOVEBOXPTR pos0 ;

count = 0 ;
pos0 = cellpos[0] ;
for(tileptrS=cellptrS->tiles;tileptrS; ) {
    posS = cellpos[++count] ;

    if(!(pos0->loaded_previously)){
	/* normal case */
	posS->l = tileptrS->left ;
	posS->r = tileptrS->right ;
	posS->b = tileptrS->bottom ;
	posS->t = tileptrS->top ;

	posS->lw = tileptrS->lweight ;
	posS->rw = tileptrS->rweight ;
	posS->bw = tileptrS->bweight ;
	posS->tw = tileptrS->tweight ;
	/* need to do this less than obvious update here so */
	/* mismatching number of tiles in uinst don't upset code */
	tileptrS=tileptrS->next ;
    
    } else {
	/* data for tile has already been loaded in uaspect */
	/* or uinst - reset the flag */
	/* in this case we don't update tileptr - just check count */
	if( count >= pos0->numtiles ){
	    tileptrS = NULL ; /* use this to stop loop */
	}
    }
    Ytranslate( &(posS->l), &(posS->b), &(posS->r), &(posS->t), orientS );
    Ytranslatef( &(posS->lw),&(posS->bw),&(posS->rw),&(posS->tw),orientS );

    /* then add wire estimation dynamically */
    WIREESTXY(posS,xcS,ycS ) ;

    /* hash cell coordinates to overlap bin */ 
    posS->binR = SETBINX( posS->r ) ;
    posS->binL = SETBINX( posS->l ) ;
    posS->binB = SETBINY( posS->b ) ;
    posS->binT = SETBINY( posS->t ) ;

    /* calculate the bounding box of the entire cell */
    if( posS->binR > maxXS ){
	maxXS = posS->binR ;
    }
    if( posS->binL < minXS ){
	minXS = posS->binL ;
    }
    if( posS->binT > maxYS ){
	maxYS = posS->binT ;
    }
    if( posS->binB < minYS ){
	minYS = posS->binB ;
    }

} /* end for loop */
/* save number of tiles for add_penal and sub_penal functions */
pos0->numtiles = count ;
pos0->loaded_previously = FALSE ; 

} /* function calculate bins calcBins */

/* ***************************************************************** 
   Calculate bins for the given cell posSition.
   DOES NOT use wire estimation in the calculation.
   Takes pointer to move box record as an argument
*/
INT calc_nBins( cellpos )
MOVEBOXPTR *cellpos ;
{

INT count ;
MOVEBOXPTR pos0 ;

count = 0 ;
pos0 = cellpos[0] ;
for( tileptrS = cellptrS->tiles;tileptrS; ) {
    posS = cellpos[++count] ;

    if(!(pos0->loaded_previously)){
	/* normal case */
	posS->l = tileptrS->left ;
	posS->r = tileptrS->right ;
	posS->b = tileptrS->bottom ;
	posS->t = tileptrS->top ;
	/* need to do this less than obvious update here so */
	/* mismatching number of tiles in uinst don't upset code */
	tileptrS=tileptrS->next ;
    
    } else {
	/* data for tile has already been loaded in uaspect */
	/* or uinst - reset the flag */
	/* in this case we don't update tileptr - just check count */
	if( count >= pos0->numtiles ){
	    tileptrS = NULL ; /* use this to stop loop */
	}
    }
    /* first get orientation from  translate */
    Ytranslate( &(posS->l), &(posS->b), &(posS->r), &(posS->t), orientS );
    posS->l += xcS ;
    posS->r += xcS ;
    posS->b += ycS ;
    posS->t += ycS ;

    /* hash cell coordinates to overlap bin */ 
    posS->binR = SETBINX( posS->r ) ;
    posS->binL = SETBINX( posS->l ) ;
    posS->binB = SETBINY( posS->b ) ;
    posS->binT = SETBINY( posS->t ) ;

    /* calculate the bounding box of the entire cell */
    if( posS->binR > maxXS ){
	maxXS = posS->binR ;
    }
    if( posS->binL < minXS ){
	minXS = posS->binL ;
    }
    if( posS->binT > maxYS ){
	maxYS = posS->binT ;
    }
    if( posS->binB < minYS ){
	minYS = posS->binB ;
    }

} /* end for loop */
/* save number of tiles for add_penal and sub_penal functions */
pos0->numtiles = count ;
pos0->loaded_previously = FALSE ; 

} /* function calculate bins calcBins */


/* ***************************************************************** 
   Turn wire estimation on.  This is a toggle between the
   calc_wBins routine and calc_nBins routine.  In this case,
   the calc_Bins routine is set to calc_wBins.
*/
turn_wireest_on( turn_on )
INT turn_on ;
{

    if( turn_on ){
	calc_Bins = calc_wBins ;
	if( new_wire_estG ){
	    wire_est = wireestxy2 ;
	} else {
	    wire_est = wireestxy ;
	}
    } else {
	calc_Bins = calc_nBins ;
    }
} /* end funtion turn_wireest_on */


/* ***************************************************************** 
   Set up variables for calculating bins.  This routine will avoid
   the use of many global variables.  This routines sets up static
   variables in this file. 
*/
setup_Bins( s_cellptr, s_xc, s_yc, s_orient )
CELLBOXPTR s_cellptr ;
INT s_xc ;
INT s_yc ;
INT s_orient ;
{
    cellptrS  = s_cellptr ;
    xcS  = s_xc ;
    ycS  = s_yc ;
    orientS  = s_orient ;

} /* end function setup bins */

/* ***************************************************************** 
   add2bin adds to initialized bins the overlap of the given cell.
   It is assumed that setup bins was immediately called before 
   call to this routine.
*/
add2bin( cellpos )
MOVEBOXPTR *cellpos ;
{
INT count, maxcount ;
INT x, y ;
register BINBOXPTR *fastbin ;

/* first get bin limits for tiles of this cell */
calcBins( cellpos ) ;

maxcount = cellptrS->numtiles ;
ASSERT( maxcount == cellpos[0]->numtiles, "add2bin", "Tile problem" ) ;
/* add penalty */
for( count=1 ; count <= maxcount ; count++ ) {

    posS  = cellpos[count] ;
    /* assign to simple variable for speed */
    minXS = posS->binL ;
    minYS = posS->binB ;
    maxXS = posS->binR ;
    maxYS = posS->binT ;

/* ----------------------------------------------------------------- 
    Calculate penalty 
*/
    if( minXS == maxXS && minYS == maxYS ){
	/* entire cell is in bin */
	bptrS = binptrG[minXS][minYS] ;
	xcostS = posS->r - posS->l ;
	ycostS = posS->t - posS->b ;
	bptrS->penalty += xcostS * ycostS ;

    } else if( minXS == maxXS ){  /* multiple y bins */
	xcostS = posS->r - posS->l ;
	/* first bottom bin */
	fastbin = binptrG[minXS] ;
	bptrS = fastbin[minYS] ;
	ycostS = bptrS->top - posS->b ;
	bptrS->penalty += xcostS * ycostS ;
	/* next middle bins */
	for( y = minYS+1; y < maxYS ; y++ ){
	    bptrS = fastbin[y] ;
	    bptrS->penalty += xcostS * binWidthYG ;
	}
	/* last top bin */
	bptrS = fastbin[maxYS] ;
	ycostS = posS->t - bptrS->bottom ;
	bptrS->penalty += xcostS * ycostS ;

    } else if( minYS == maxYS ){  /* multiple x bins */
	ycostS = posS->t - posS->b ;
	/* first left bin */
	bptrS = binptrG[minXS][minYS] ;
	xcostS = bptrS->right - posS->l ;
	bptrS->penalty += xcostS * ycostS ;
	/* next middle bins */
	for( x = minXS+1; x < maxXS ; x++ ){
	    bptrS = binptrG[x][minYS] ;
	    bptrS->penalty += binWidthXG * ycostS ;
	}
	/* last right bin */
	bptrS = binptrG[maxXS][minYS] ;
	xcostS = posS->r - bptrS->left ;
	bptrS->penalty += xcostS * ycostS ;

    } else {  /* multiple rows and columns */
/* ----------------------------------------------------------------- 
	3 |    4          |  5
	-----------------------
	2 |    9          |  6
	-----------------------
	1 |    8          |  7
	above is order which we check overlap in the bins */
	/* first lower left bin - 1 */
	fastbin = binptrG[minXS] ;
	bptrS = fastbin[minYS] ;
	xcostS = bptrS->right - posS->l ;
	ycostS = bptrS->top - posS->b ;
	bptrS->penalty += xcostS * ycostS ;
	/* next left middle y bins - 2 */
	for( y = minYS+1; y < maxYS ; y++ ){
	    bptrS = fastbin[y] ;
	    bptrS->penalty += xcostS * binWidthYG ;
	}
	/* next top left bin - 3 */
	bptrS = fastbin[maxYS] ;
	ycostS = posS->t - bptrS->bottom ;
	bptrS->penalty += xcostS * ycostS ;
	/*  do top y bins - 4 */
	for( x = minXS+1; x < maxXS ; x++ ){
	    bptrS = binptrG[x][maxYS] ;
	    bptrS->penalty += binWidthXG * ycostS ;
	}
	/* do top right bin - 5 */
	fastbin = binptrG[maxXS] ;
	bptrS = fastbin[maxYS] ;
	xcostS = posS->r - bptrS->left ;
	bptrS->penalty += xcostS * ycostS ;
	/* do middle right bins - 6 */
	for( y = minYS+1; y < maxYS ; y++ ){
	    bptrS = fastbin[y] ;
	    bptrS->penalty += xcostS * binWidthYG ;
	}
	/* do lower right bin - 7 */
	bptrS = fastbin[minYS] ;
	ycostS = bptrS->top - posS->b ;
	bptrS->penalty += xcostS * ycostS ;
	/*  do bottom x bins - 8 */
	bptrS = binptrG[minXS][minYS] ;
	for( x = minXS+1; x < maxXS ; x++ ){
	    bptrS = binptrG[x][minYS] ;
	    bptrS->penalty += binWidthXG * ycostS ;
	}
	/* center middle bins - 9 */
	for( x = minXS+1; x < maxXS ; x++ ){
	    fastbin = binptrG[x] ;
	    for( y = minYS+1; y < maxYS ; y++ ){
		bptrS = fastbin[y] ;
		bptrS->penalty += binWidthXG * binWidthYG ;
	    }
	}
    }

} /* end for loop */

} /* end add2bin function */
