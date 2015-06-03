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
FILE:	    outgeo.c                                       
DESCRIPTION:Routine dumps the placement of the cells for channel graph
	    program.
CONTENTS:   
DATE:	    Jan 30, 1988 
REVISIONS:  Jul 30, 1988 - made compile switch for blocking pads.
	    I need to get cell name for mighty interface.
	    Aug 15, 1988 - decide blocking of pads always needed.
		eliminated blocking compile switch.  Blocking
		avoids channels between pads .
	    Jan 15, 1988 - modified output so softcells would work
		correctly for compaction.
	    Jan 17, 1988 - added dummy macro blocks for pads if
		pads don't exist on a side.  This fixes a problem
		with the channel graph generator program.
	    Mar 03, 1989 - fixed problem with bottom macro.
	    Sep 12, 1989 - added new unbust algorithm.
	    Sep 16, 1989 - fixed problem with unbust.  Multiple tile
		cells should use tile count as argument since tiles
		need to be seamed together.
	    Sep 19, 1989 - fixed dummy macro block coordinates for
		the case of no pads.
	    Oct 15, 1989 - now output pad macros are placed at 
		density using store pad location.
	    Sun Dec 16 00:29:59 EST 1990 - now use library version of
		buster.
	    Wed Jun  5 16:30:30 CDT 1991 - eliminated unbust.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) outgeo.c version 3.6 6/5/91" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>
#include <yalecad/buster.h>
#include <yalecad/file.h>
#include <yalecad/relpos.h>
#include <pads.h>



static store_pad_loc();




outgeo()
{

FILE *fp ;
INT cell ;
INT l, b, r, t, x , y , xc , yc ;
char filename[LRECL] ;
CELLBOXPTR cellptr ;
BOUNBOXPTR bounptr ;
INT padmacroCounter ;
INT  left_side, right_side, bottom_side, top_side ;

sprintf( filename, "%s.mgeo", cktNameG ) ;
fp = TWOPEN( filename , "w", ABORT) ;

padmacroCounter = numcellsG ;
for( cell = 1 ; cell <= numcellsG ; cell++ ) {

    cellptr = cellarrayG[cell] ;
    fprintf(fp,"cell %s \n", cellptr->cname ) ;
    fprintf(fp,"%d vertices ", cellptr->numsides ) ;
    output_vertices( fp, cellptr ) ;
}

/* find core extent in case pads don't exist on a side */
find_core_boundary( &left_side, &right_side, &bottom_side, &top_side ) ;

/* block all the pads into the four side cells */
/* first find left pad macro */
l = INT_MAX ;
b = INT_MAX ;
r = INT_MIN ;
t = INT_MIN ;
for( cell = endsuperG + 1 ; cell <= numcellsG + numpadsG ; cell++ ) {
    cellptr = cellarrayG[cell] ;
    if( cellptr->padptr->padside != L ) {
	continue ;
    }
    xc = cellptr->xcenter ;
    yc = cellptr->ycenter ;
    bounptr = cellptr->bounBox[ cellptr->orient ] ;

    if( xc + bounptr->l < l ) {
	l = xc + bounptr->l ;
    }
    if( xc + bounptr->r > r ) {
	r = xc + bounptr->r ;
    }
    if( yc + bounptr->b < b ) {
	b = yc + bounptr->b ;
    }
    if( yc + bounptr->t > t ) {
	t = yc + bounptr->t ;
    }
}
if( t == INT_MIN ) {
    /* make dummy pads for left */
    /* use coordinates found from find_core_boundary */
    l = left_side - 2 * track_spacingXG ;
    b = bottom_side + 2 * track_spacingYG ;
    r = left_side - track_spacingXG ;
    t = top_side - 2 * track_spacingYG ;
}
fprintf(fp,"cell %s \n", "pad.macro.l" ) ;
fprintf(fp,"4 vertices ") ;
fprintf(fp," %d %d %d %d %d %d %d %d\n", l, b, l, t, r, t, r, b ) ;
setPadMacroNum( L, ++padmacroCounter ) ;
cellptr = cellarrayG[endpadgrpsG + L] ;
store_pad_loc( cellptr, l, r, b, t ) ;

/* find pads on top of core */
l = INT_MAX ;
b = INT_MAX ;
r = INT_MIN ;
t = INT_MIN ;
for( cell = endsuperG + 1 ; cell <= numcellsG + numpadsG ; cell++ ) {
    cellptr = cellarrayG[cell] ;
    if( cellptr->padptr->padside != T ) {
	continue ;
    }
    xc = cellptr->xcenter ;
    yc = cellptr->ycenter ;
    bounptr = cellptr->bounBox[ cellptr->orient ] ;

    if( xc + bounptr->l < l ) {
	l = xc + bounptr->l ;
    }
    if( xc + bounptr->r > r ) {
	r = xc + bounptr->r ;
    }
    if( yc + bounptr->b < b ) {
	b = yc + bounptr->b ;
    }
    if( yc + bounptr->t > t ) {
	t = yc + bounptr->t ;
    }
}
if( t == INT_MIN ) {
    /* make dummy pads for top */
    l = left_side + 2 * track_spacingXG ;
    b = top_side + track_spacingYG ;
    r = right_side - 2 * track_spacingXG ;
    t = top_side + 2 * track_spacingYG ;
}
fprintf(fp,"cell %s \n", "pad.macro.t" ) ;
fprintf(fp,"4 vertices ") ;
fprintf(fp," %d %d %d %d %d %d %d %d\n", l, b, l, t, r, t, r, b ) ;
setPadMacroNum( T, ++padmacroCounter ) ;
cellptr = cellarrayG[endpadgrpsG + T] ;
store_pad_loc( cellptr, l, r, b, t ) ;


l = INT_MAX ;
b = INT_MAX ;
r = INT_MIN ;
t = INT_MIN ;
for( cell = endsuperG + 1 ; cell <= numcellsG + numpadsG ; cell++ ) {
    cellptr = cellarrayG[cell] ;
    if( cellptr->padptr->padside != R ) {
	continue ;
    }
    xc = cellptr->xcenter ;
    yc = cellptr->ycenter ;
    bounptr = cellptr->bounBox[ cellptr->orient ] ;

    if( xc + bounptr->l < l ) {
	l = xc + bounptr->l ;
    }
    if( xc + bounptr->r > r ) {
	r = xc + bounptr->r ;
    }
    if( yc + bounptr->b < b ) {
	b = yc + bounptr->b ;
    }
    if( yc + bounptr->t > t ) {
	t = yc + bounptr->t ;
    }
}
if( t == INT_MIN ) {
    /* make dummy pads for right */
    l = right_side + track_spacingXG ;
    b = bottom_side + 2 * track_spacingYG ;
    r = right_side + 2 * track_spacingXG ;
    t = top_side - 2 * track_spacingYG ;
}
fprintf(fp,"cell %s \n", "pad.macro.r" ) ;
fprintf(fp,"4 vertices ") ;
fprintf(fp," %d %d %d %d %d %d %d %d\n", l, b, l, t, r, t, r, b ) ;
setPadMacroNum( R, ++padmacroCounter ) ;
cellptr = cellarrayG[endpadgrpsG + R] ;
store_pad_loc( cellptr, l, r, b, t ) ;

l = INT_MAX ;
b = INT_MAX ;
r = INT_MIN ;
t = INT_MIN ;
for( cell = endsuperG + 1 ; cell <= numcellsG + numpadsG ; cell++ ) {
    cellptr = cellarrayG[cell] ;
    if( cellptr->padptr->padside != B ) {
	continue ;
    }
    xc = cellptr->xcenter ;
    yc = cellptr->ycenter ;
    bounptr = cellptr->bounBox[ cellptr->orient ] ;

    if( xc + bounptr->l < l ) {
	l = xc + bounptr->l ;
    }
    if( xc + bounptr->r > r ) {
	r = xc + bounptr->r ;
    }
    if( yc + bounptr->b < b ) {
	b = yc + bounptr->b ;
    }
    if( yc + bounptr->t > t ) {
	t = yc + bounptr->t ;
    }
}
if( t == INT_MIN ) {
    /* make dummy pads for bottom */
    l = left_side + 2 * track_spacingXG ;
    b = bottom_side - 2 * track_spacingYG ;
    r = right_side - 2 * track_spacingXG ;
    t = bottom_side - track_spacingYG ;
}
fprintf(fp,"cell %s \n", "pad.macro.b" ) ;
fprintf(fp,"4 vertices ") ;
fprintf(fp," %d %d %d %d %d %d %d %d\n", l, b, l, t, r, t, r, b ) ;
setPadMacroNum( B, ++padmacroCounter ) ;
cellptr = cellarrayG[endpadgrpsG + B] ;
store_pad_loc( cellptr, l, r, b, t ) ;

TWCLOSE( fp ) ;
return ;

} /* end outgeo */

static store_pad_loc( cellptr, l, r, b, t )
CELLBOXPTR cellptr ;
INT l, r, b, t ;
{
    TILEBOXPTR tptr ;

    tptr = cellptr->tiles ;
    cellptr->xcenter = (l + r) / 2 ;
    cellptr->ycenter = (b + t) / 2 ;
    tptr->orig_left = tptr->left = l - cellptr->xcenter ;
    tptr->orig_right = tptr->right = r - cellptr->xcenter ;
    tptr->orig_bottom = tptr->bottom = b - cellptr->ycenter ;
    tptr->orig_top = tptr->top = t - cellptr->ycenter ;
} /* end static store_pad_loc */
