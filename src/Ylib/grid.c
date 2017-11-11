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
		int *x , *y ;
	    Ygridx( x )
		int *x ;
	    Ygridx( x )
		int *x ;
	    Ygridx_down( x )
		int *x ;
	    Ygridy_down( y )
		int *y ;
	    Ygridx_up( x )
		int *x ;
	    Ygridy_up( y )
		int *y ;
	    Ygrid_setx( x, offset )
		int x, offset ;
	    Ygrid_sety( y, offset )
		int y, offset ;
	    Ygrid_getx( x, offset )
		int *x, *offset ;
	    Ygrid_gety( y, offset )
		int *y, *offset ;
DATE:	    Oct 27, 1989 
REVISIONS:  Thu Apr 18 00:39:45 EDT 1991 - renamed functions so
		that names were consistent.
----------------------------------------------------------------- */
#include <globals.h>

int offsetxS = 0 ;  /* initially set offset to zero */
int offsetyS = 0 ;  
int xgridS = 1 ;    /* default grid is one */
int ygridS = 1 ;  
/* ***************************************************************** */
/* force coordinate to closest xy grid position */
void YforceGrid( int *x , int *y )
{
    Ygridx( x ) ;
    Ygridy( y ) ;
} /* end forceGrid */

/* force coordinate to closest x grid position */
void Ygridx( int *x )
{

    int newx ;

    newx = ( (*x - offsetxS) / xgridS ) * xgridS + offsetxS ;
    if( ABS(newx + xgridS - *x) < ABS(newx - *x) ) {
	newx += xgridS ;
    }
    /* now return values */
    *x = newx ;
} /* end Ygridy */


/* force coordinate to closest x grid position */
void Ygridy(int *y)
{

    int newy ;

    newy = ( (*y - offsetyS) / ygridS ) * ygridS + offsetyS ;
    if( ABS(newy + ygridS - *y) < ABS(newy - *y) ) {
	newy += ygridS ;
    }
    /* now return values */
    *y = newy ;
} /* end Ygridy */

/* force coordinate to smallest x grid position */
void Ygridx_down(int *x)
{

    int newx ;

    newx = ( (*x - offsetxS) / xgridS ) * xgridS + offsetxS ;
    /* now return values */
    *x = newx ;
} /* end Ygridx_down */

/* force coordinate to smallest y grid position */
void Ygridy_down(int *y)
{

    int newy ;
    newy = ( (*y - offsetyS) / ygridS ) * ygridS + offsetyS ;

    /* now return values */
    *y = newy ;

} /* end Ygridy_down */

/* round coordinate to larger x grid position */
void Ygridx_up(int *x)
{

    int newx ;

    newx = ( (*x - offsetxS) / xgridS ) * xgridS + offsetxS ;
    if( ABS(newx - *x)) {
	newx += xgridS ;
    }
    /* now return values */
    *x = newx ;
} /* end Ygridx_up */

/* round coordinate to larger y grid position */
void Ygridy_up(int *y)
{

    int newy ;
    newy = ( (*y - offsetyS) / ygridS ) * ygridS + offsetyS ;
    if( ABS(newy - *y)) {
	newy += ygridS ;
    }
    /* now return values */
    *y = newy ;

} /* end Ygridy_up */

void Ygrid_setx(int x, int offset)
{
    xgridS = x ;
    offsetxS = offset ;
} /* end Ygrid_setx */

void Ygrid_sety(int y, int offset)
{
    ygridS = y ;
    offsetyS = offset ;
} /* end Ygrid_sety */

void Ygrid_getx( int *x, int *offset )
{
    *x = xgridS ;
    *offset = offsetxS ;
} /* end Ygrid_getx */

void Ygrid_gety( int *y, int *offset )
{
    *y = ygridS ;
    *offset = offsetyS ;
} /* end Ygrid_gety */
