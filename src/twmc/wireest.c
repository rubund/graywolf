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
FILE:	    wireest.c
DESCRIPTION:wire estimator routines.  Uses tent function.
CONTENTS:   wireestx.  Uses tent function.
DATE:	    Feb 19, 1988 
	    Aug 24, 1988 - thanks to Joy Chhugini and Anirudh Mathuria
		for hints on simplifying logic of wire estimator code.
		Four calls to this procedure are reduced to one call.
REVISIONS:  Mar 29, 1989 - changed tileptr argument to pos.
	    Mon Feb  4 02:26:14 EST 1991 - added new wire estimator
		that uses SVD algorithm.
	    Thu Mar  7 01:49:31 EST 1991 - now save wireestimation
		parameters.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) wireest.c version 3.6 3/7/91" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

#ifdef MACRO 
#define CHECK_ROUTING( routing )           \
    if( routing < 0 ){                     \
	M( WARNMSG, "check_routing", "Routing negative - taking ABS\n"); \
	routing = - routing ;               \
    }
#else 

#define CHECK_ROUTING( routing )   check_routing( &(routing) ) 
    
#endif

#define ROUTING( x, y, numpins )           \
    C0S + (C1S + C2S * x ) * x + (C3S + C4S * y ) * y + C5S * numpins

/* static definitions */
static DOUBLE C0S ;      /* constant       */
static DOUBLE C1S ;      /* C1S * x        */
static DOUBLE C2S ;      /* C2S * x * x    */
static DOUBLE C3S ;      /* C3S * y        */
static DOUBLE C4S ;      /* C4S * y * y    */
static DOUBLE C5S ;      /* C5S * numpins  */

static DOUBLE blocklS ;      /* these are used to speed calculation */
static DOUBLE blockbS ;
static DOUBLE bdxlengthS ;
static DOUBLE bdylengthS ;



static check_routing();



/*******************************************************************/

wireestxy( pos, xc, yc ) 
MOVEBOXPTR pos ;
INT xc, yc ;
{

    INT xs , xe , y ;
    INT x , ys , ye ;
    INT  left , right , bottom , top ;
    DOUBLE factor ;

    right  = pos->r += xc ;
    left   = pos->l += xc ;
    bottom = pos->b += yc ;
    top    = pos->t += yc ;

    if( left <= blockmxG ) {
	xs = left - blocklG ;
    } else {
	xs = blockrG - left ;
    }
    if( right <= blockmxG ) {
	xe = right - blocklG ;
    } else {
	xe = blockrG - right ;
    }                         

    if( xs < 0 ){    
	xs = 0 ;
    }
    if( xe < 0 ) {
	xe = 0 ;
    }

    /************************************/

    if (bottom <= top) {
	if (top <= blockmyG) {
	    ys = bottom - blockbG ;
	    if( ys < 0 ){
		ys = 0 ;
	    }
	    ye = top - blockbG   ;
	    if( ye < 0 ){
		ye = 0 ;
	    }
	    y  = ye ;
	} else {
	    ye = blocktG - top   ; 
	    if( ye < 0 ){
		ye = 0 ;
	    }
	    if (bottom>= blockmyG) {
		ys = blocktG - bottom ;
		if( ys < 0 ){
		    ys = 0 ;
		}
		y  = ys ;
	    } else {
		ys = bottom - blockbG ;
		if( ys < 0 ){
		    ys = 0 ;
		}
		y = halfYspanG ;
	    }
	}
    } else {
	if (bottom <= blockmyG) {
	    ys = bottom - blockbG ; 
	    if( ys < 0 ){
		ys = 0 ;
	    }
	    ye = top - blockbG   ; 
	    if( ye < 0 ){
		ye = 0 ;
	    }
	    y  = blocktG - bottom ; 
	    if( y < 0 ){
		y = 0 ;
	    }
	} else {
	    ys = blocktG - bottom ; 
	    if( ys < 0 ){
		ys = 0 ;
	    }
	    if (top >= blockmyG) {
		ye = blocktG - top ; 
		if( ye < 0 ){ 
		    ye = 0 ;
		}
		y  = top - blockbG   ;
		if( y < 0 ){
		    y = 0 ;
		}
	    } else {
		ye = top - blockbG   ; 
		if( ye < 0 ){
		    ye = 0 ;
		}
		if( bottom - blockmyG <= blockmyG - top){   
		       y = ys ;
		} else {
		    y = ye ;
		}
	    }
	}
    }

    /*************************************/
    /* y <= ( ( deltaY * y + blockt ) */
    y *= deltaY ;
    y += blocktG ;

    /* xs <= (xs * deltaX + blockrG) */
    xs *= deltaX ;
    xs += blockrG ;

    /* xe <= (xe * deltaX + blockrG)  */
    xe *= deltaX ;
    xe += blockrG ;

    /* ----------------------------------------------------------------- 
        Unfortunately we can't multiply the x's by y using integer
	multiplication because of possible wraparounds on large designs.
	Instead we must multiply y in following statement.
    xs *= y ;
    xe *= y ;
    ----------------------------------------------------------------- */

    pos->l = pos->l - (INT) ( (DOUBLE) xs * (DOUBLE) y * 
				    wireFactorXG * pos->lw );
    pos->r = pos->r + (INT) ( (DOUBLE) xe * (DOUBLE) y * 
				    wireFactorXG * pos->rw );

    if( right >= blockmxG && left <= blockmxG ) {
	x = halfXspanG ;
    } else if( ABS(left - blockmxG) <= ABS(right - blockmxG) ) {
	x = blockrG - left ;
	if( x < 0 ) {
	    x = 0 ;
	}
    } else {
	x = right - blocklG ;
	if( x < 0 ) {
	    x = 0 ;
	}
    }
    /* x <= ( ( deltaX * x + blockrG ) */
    x *= deltaX ;
    x += blockrG ;

    /* ys <= (ys * deltaY + blocktG)  */
    ys *= deltaY ;
    ys += blocktG ;

    /* ye <= (ye * deltaY + blocktG) */
    ye *= deltaY ;
    ye += blocktG ;

    /* ----------------------------------------------------------------- 
	See above comment.
    ys *= x ;
    ye *= x ;
    ----------------------------------------------------------------- */

    pos->b = pos->b - (INT) ( (DOUBLE) ys * (DOUBLE) x * 
				wireFactorYG * pos->bw );
    pos->t = pos->t + (INT) ( (DOUBLE) ye * (DOUBLE) x * 
				wireFactorYG * pos->tw );
    return ;
} /* end wireestxy */
/*********************************************************************/

wireestxy2( pos, xc, yc ) 
MOVEBOXPTR pos ;
INT xc, yc ;
{
    DOUBLE l, r, b, t ;          /* four sides of the tile */
    DOUBLE xave, yave ;          /* average of a tile side */
    INT    routing ;             /* amount of routing estimate for side */

    pos->r += xc ;
    pos->l += xc ;
    pos->b += yc ;
    pos->t += yc ;

    /* check to see if we have a standard cell cluster */
    if( pos->lw < 0 || bdxlengthS <= 1.0 ){
	/* no more work to do */
	return ; 
    }

    l = ( (DOUBLE) pos->l - blocklS ) / bdxlengthS ;
    if( l < 0.0 ) l = 0.0 ;
    if( l > 1.0 ) l = 1.0 ;
    r = ( (DOUBLE) pos->r - blocklS ) / bdxlengthS ;
    if( r < 0.0 ) r = 0.0 ;
    if( r > 1.0 ) r = 1.0 ;
    b = ( (DOUBLE) pos->b - blockbS ) / bdylengthS ;
    if( b < 0.0 ) b = 0.0 ;
    if( b > 1.0 ) b = 1.0 ;
    t = ( (DOUBLE) pos->t - blockbS ) / bdylengthS ;
    if( t < 0.0 ) t = 0.0 ;
    if( t > 1.0 ) t = 1.0 ;
    xave = (l + r) / 2.0 ;
    yave = (b + t) / 2.0 ;

    /* now add the routing to the tile */
    routing = (INT) ROUTING( l, yave, pos->lw ) ;
    CHECK_ROUTING( routing ) ;
    pos->l -= routing ;

    routing = (INT) ROUTING( r, yave, pos->rw ) ;
    CHECK_ROUTING( routing ) ;
    pos->r += routing ;

    routing = (INT) ROUTING( xave, b, pos->bw ) ;
    CHECK_ROUTING( routing ) ;
    pos->b -= routing ;

    routing = (INT) ROUTING( xave, t, pos->tw ) ;
    CHECK_ROUTING( routing ) ;
    pos->t += routing ;

} /* end wireestxy2 */

BOOL read_wire_est( fp )
FILE *fp ;
{
    INT max_pitch ;

    fprintf( fpoG, "\nSVD fit parameters found.  Using:\n" ) ;
    HPI( fp,  &C0S ) ;
    HPO( fpoG, C0S ) ;
    HPI( fp,  &C1S ) ;
    HPO( fpoG, C1S ) ;
    HPI( fp,  &C2S ) ;
    HPO( fpoG, C2S ) ;
    HPI( fp,  &C3S ) ;
    HPO( fpoG, C3S ) ;
    HPI( fp,  &C4S ) ;
    HPO( fpoG, C4S ) ;
    HPI( fp,  &C5S ) ;
    HPO( fpoG, C5S ) ;
    /* now add the constant */
    max_pitch = MAX( track_spacingXG, track_spacingYG ) ;
    C0S += (DOUBLE) max_pitch ;
    fprintf( fpoG, "\nConstant modified to:\n" ) ;
    HPO( fpoG, C0S ) ;
    return( TRUE ) ;

} /* end read_wire_est */

resize_wire_params()
{
    blocklS = (DOUBLE) blocklG ;
    blockbS = (DOUBLE) blockbG ;
    bdxlengthS = (DOUBLE) bdxlengthG ;
    bdylengthS = (DOUBLE) bdylengthG ;
} /* end resize_wire_params */


static check_routing( routing )
INT *routing ;
{
    if( *routing < 0 ){
	/* 
	M( WARNMSG, "check_routing", "Routing negative - taking ABS\n");
	*routing = - *routing ;
	*/
	*routing = 0.0 ;
    }
}/* end check_routing */

/* ***************************************************************** 
   save_wireest - save wireest parameters for restart
*/
save_wireest( fp )
FILE *fp ;
{
    fprintf(fp,"# wireest parameters:\n") ;
    HPO( fp, C0S ) ;
    HPO( fp, C1S ) ;
    HPO( fp, C2S ) ;
    HPO( fp, C3S ) ;
    HPO( fp, C4S ) ;
    HPO( fp, C5S ) ;
} /* end save_wireest */

/* ***************************************************************** 
   read_wireest - read wireest parameters for restart
*/
INT read_wireest( fp )
FILE *fp ;
{
    INT error = 0 ;

    fscanf(fp,"%[ #:a-zA-Z]\n",YmsgG ); /* throw away comment */
    HPI( fp,  &C0S ) ;
    HPI( fp,  &C1S ) ;
    HPI( fp,  &C2S ) ;
    HPI( fp,  &C3S ) ;
    HPI( fp,  &C4S ) ;
    HPI( fp,  &C5S ) ;

    return(error) ;

} /* end read_wireest */
