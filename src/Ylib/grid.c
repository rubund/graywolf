/*
 *   Copyright (C) 1989-1991 Yale University
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
FILE:	    grid.c                                       
DESCRIPTION:These routines grid data to the specified grid.
CONTENTS:   YforceGrid( x , y )
		INT *x , *y ;
	    Ygridx( x )
		INT *x ;
	    Ygridx( x )
		INT *x ;
	    Ygridx_down( x )
		INT *x ;
	    Ygridy_down( y )
		INT *y ;
	    Ygridx_up( x )
		INT *x ;
	    Ygridy_up( y )
		INT *y ;
	    Ygrid_setx( x, offset )
		INT x, offset ;
	    Ygrid_sety( y, offset )
		INT y, offset ;
	    Ygrid_getx( x, offset )
		INT *x, *offset ;
	    Ygrid_gety( y, offset )
		INT *y, *offset ;
DATE:	    Oct 27, 1989 
REVISIONS:  Thu Apr 18 00:39:45 EDT 1991 - renamed functions so
		that names were consistent.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) grid.c version 1.4 4/18/91" ;
#endif

#include <yalecad/base.h>

static INT offsetxS = 0 ;  /* initially set offset to zero */
static INT offsetyS = 0 ;  
static INT xgridS = 1 ;    /* default grid is one */
static INT ygridS = 1 ;  
/* ***************************************************************** */
/* force coordinate to closest xy grid position */
YforceGrid( x , y )
INT *x , *y ;
{
    Ygridx( x ) ;
    Ygridy( y ) ;
} /* end forceGrid */

/* force coordinate to closest x grid position */
Ygridx( x )
INT *x ;
{

    INT newx ;

    newx = ( (*x - offsetxS) / xgridS ) * xgridS + offsetxS ;
    if( ABS(newx + xgridS - *x) < ABS(newx - *x) ) {
	newx += xgridS ;
    }
    /* now return values */
    *x = newx ;
} /* end Ygridy */


/* force coordinate to closest x grid position */
Ygridy( y )
INT *y ;
{

    INT newy ;

    newy = ( (*y - offsetyS) / ygridS ) * ygridS + offsetyS ;
    if( ABS(newy + ygridS - *y) < ABS(newy - *y) ) {
	newy += ygridS ;
    }
    /* now return values */
    *y = newy ;
} /* end Ygridy */

/* force coordinate to smallest x grid position */
Ygridx_down( x )
INT *x ;
{

    INT newx ;

    newx = ( (*x - offsetxS) / xgridS ) * xgridS + offsetxS ;
    /* now return values */
    *x = newx ;
} /* end Ygridx_down */

/* force coordinate to smallest y grid position */
Ygridy_down( y )
INT *y ;
{

    INT newy ;
    newy = ( (*y - offsetyS) / ygridS ) * ygridS + offsetyS ;

    /* now return values */
    *y = newy ;

} /* end Ygridy_down */

/* round coordinate to larger x grid position */
Ygridx_up( x )
INT *x ;
{

    INT newx ;

    newx = ( (*x - offsetxS) / xgridS ) * xgridS + offsetxS ;
    if( ABS(newx - *x)) {
	newx += xgridS ;
    }
    /* now return values */
    *x = newx ;
} /* end Ygridx_up */

/* round coordinate to larger y grid position */
Ygridy_up( y )
INT *y ;
{

    INT newy ;
    newy = ( (*y - offsetyS) / ygridS ) * ygridS + offsetyS ;
    if( ABS(newy - *y)) {
	newy += ygridS ;
    }
    /* now return values */
    *y = newy ;

} /* end Ygridy_up */

Ygrid_setx( x, offset )
INT x, offset ;
{
    xgridS = x ;
    offsetxS = offset ;
} /* end Ygrid_setx */

Ygrid_sety( y, offset )
INT y, offset ;
{
    ygridS = y ;
    offsetyS = offset ;
} /* end Ygrid_sety */

Ygrid_getx( x, offset )
INT *x, *offset ;
{
    *x = xgridS ;
    *offset = offsetxS ;
} /* end Ygrid_getx */

Ygrid_gety( y, offset )
INT *y, *offset ;
{
    *y = ygridS ;
    *offset = offsetyS ;
} /* end Ygrid_gety */
