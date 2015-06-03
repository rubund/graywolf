/*
 *   Copyright (C) 1990-1992 Yale University
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
FILE:	    trans.c                                       
DESCRIPTION:translation routines
	    Ytranslate(l,b,r,t, orient) 
		INT *l, *b, *r, *t, orient ;
	    Ytranslatef(l,b,r,t, orient) 
		DOUBLE *l, *b, *r, *t, orient ;
	    Yinv_orient( orient )
		int orient ;
DATE:	    Aug 08, 1990 - from TimberWolfMC. 
REVISIONS:  Wed Jan 30 14:11:50 EST 1991 - added Ytrans_init and
		and YtranslateC for left justifying rotations.
	    Sat Nov 23 21:07:47 EST 1991 - added boundary translation
		functions.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) trans.c version 1.5 3/6/92" ;
#endif

#define  YTRANS_DEFS
#include <yalecad/base.h>
#include <yalecad/relpos.h>
#include <yalecad/message.h>

/* ***************************************************************** 
translate rotation of cells
----------------------------------------------------------------- */
VOID Ytranslate(l,b,r,t, orient) 
INT *l, *b, *r, *t, orient ;
{

    INT temp ;

    switch( orient ){
	case 0 :
		break ;
	case 1 :
		/* mirror around x - axis */
		temp   = *t ;
		*t = - *b ;
		*b = - temp   ;
		break ;
	case 2 :
		/* mirror around y - axis */
		temp   = *r ;
		*r = - *l ;
		*l = - temp   ;
		break ;
	case 3 :
		/* rotate 180 */
		temp   = *t ;
		*t = - *b ;
		*b = - temp   ;
		temp   = *r ;
		*r = - *l ;
		*l = - temp   ;
		break ;
	case 4 :
		/* mirror x and then rot 90 */
		temp   = *l ;
		*l = - *t ;
		*t = - temp   ;
		temp   = *r ;
		*r = - *b ;
		*b = - temp   ;
		break ;
	case 5 :
		/* mirror x and then rot -90 */
		temp   = *r ;
		*r = *t ;
		*t = temp   ;
		temp   = *l ;
		*l = *b ;
		*b = temp   ;
		break ;
	case 6 :
		/* rotate 90 degrees */
		temp   = *l ;
		*l = - *t ;
		*t = *r ;
		*r = - *b ;
		*b = temp   ;
		break ;
	case 7 :
		/* rotate - 90 degrees */
		temp   = *t ;
		*t = - *l ;
		*l = *b ;
		*b = - *r ;
		*r = temp   ;
		break ;
    } /* end switch */

} /* end function translate */

/* ***************************************************************** 
translate rotation of DOUBLE (cell weights)
----------------------------------------------------------------- */
VOID Ytranslatef(l,b,r,t, orient) 
DOUBLE *l, *b, *r, *t ;
INT orient ;
{

    DOUBLE temp ;

    switch( orient ){
	case 0 :
		break ;
	case 1 :
		/* mirror around x - axis */
		temp   = *t ;
		*t = *b ;
		*b = temp   ;
		break ;
	case 2 :
		/* mirror around y - axis */
		temp   = *r ;
		*r = *l ;
		*l = temp   ;
		break ;
	case 3 :
		/* rotate 180 */
		temp   = *t ;
		*t = *b ;
		*b = temp   ;
		temp   = *r ;
		*r = *l ;
		*l = temp   ;
		break ;
	case 4 :
		/* mirror x and then rot 90 */
		temp = *l ;
		*l = *t ;
		*t = temp   ;
		temp = *r ;
		*r = *b ;
		*b = temp   ;
		break ;
	case 5 :
		/* mirror x and then rot -90 */
		temp   = *r ;
		*r = *t ;
		*t = temp   ;
		temp   = *l ;
		*l = *b ;
		*b = temp   ;
		break ;
	case 6 :
		/* rotate 90 degrees */
		temp = *l ;
		*l = *t ;
		*t = *r ;
		*r = *b ;
		*b = temp   ;
		break ;
	case 7 :
		/* rotate - 90 degrees */
		temp = *t ;
		*t = *l ;
		*l = *b ;
		*b = *r ;
		*r = temp   ;
		break ;
    } /* end switch */
} /* end function translate */

/* given current cell bounding box and new orient - orient to change to */
/* set the cell centering flags - cell will always be left justified */
VOID Ytrans_init( cur_l,cur_b,cur_r,cur_t, new_orient ) 
INT cur_l, cur_b, cur_r, cur_t, new_orient ;
{
    INT height ;
    INT length ;

    height = cur_t - cur_b ;
    length = cur_r - cur_l ;

    if( (height % 2 != 0 && (new_orient == 4 || new_orient == 6))||
	(length % 2 != 0 && (new_orient == 2 || new_orient == 3))){
	Ytrans_xflagG = TRUE ;
    } else {
	Ytrans_xflagG = FALSE ;
    }
    if( (height % 2 != 0 && (new_orient == 1 || new_orient == 3))||
	(length % 2 != 0 && (new_orient == 4 || new_orient == 7))){
	Ytrans_yflagG = TRUE ;
    } else {
	Ytrans_yflagG = FALSE ;
    }

} /* end Yinv_orient */

/* return back to zero orientation */
static INT invOrientTableS[8] = {
    0, 1, 2, 3, 4, 5, 7, 6 
} ;


INT Ytrans_inv_orient( orient )
INT orient ;
{
    return( invOrientTableS[orient] ) ;
} /* end Yinv_orient */


/* this translate performs translation but also centers the tile */
/* so that the truncatated side is always to the left */
VOID YtranslateC(l,b,r,t,orient) 
INT *l, *b, *r, *t, orient ;
{
    Ytranslate( l, b, r, t, orient ) ;
    if( Ytrans_xflagG ){
	(*l)++ ;
	(*r)++ ;
    }
    if( Ytrans_yflagG ){
	(*b)++ ;
	(*t)++ ;
    }
} /* end YtranslateC */

/* this translate performs translation but also centers the tile */
/* so that the truncatated side is always to the left */
VOID YtranslateT(l,b,r,t, orient) 
INT *l, *b, *r, *t, orient ;
{

    INT height ;
    INT length ;

    height = *t - *b ;
    length = *r - *l ;
    Ytranslate( l, b, r, t, orient ) ;

    if( (height % 2 != 0 && (orient == 4 || orient == 6))||
	(length % 2 != 0 && (orient == 2 || orient == 3))){
	(*l)++ ;
	(*r)++ ;
    }
    if( (height % 2 != 0 && (orient == 1 || orient == 3))||
	(length % 2 != 0 && (orient == 4 || orient == 7))){
	(*b)++ ;
	(*t)++ ;
    }
} /* end YtranslateT */

/* ----------------------------------------------------------------- 
    Ytrans_pos - takes a cell orientation, cell pin position, and
    cell center and returns global postion.  This routine is for
    test - normally a macro defined in relpos.h is used for speed.
    Conditional compile switch is in relpos.h.
----------------------------------------------------------------- */
VOID Ytrans_rel_pos( orient_R, globalX_R, globalY_R, relativeX_R, relativeY_R,
    cellX_R, cellY_R )               
INT orient_R, *globalX_R, *globalY_R, relativeX_R, relativeY_R,         
    cellX_R, cellY_R ;               
{
    switch(orient_R){
	case 0: *globalX_R = cellX_R + relativeX_R ;
		*globalY_R = cellY_R + relativeY_R ;
		break ;
	case 1: *globalX_R = cellX_R + relativeX_R ;
		*globalY_R = cellY_R - relativeY_R ;
		break ;
	case 2: *globalX_R = cellX_R - relativeX_R ;
		*globalY_R = cellY_R + relativeY_R ;
		break ;
	case 3: *globalX_R = cellX_R - relativeX_R ;
		*globalY_R = cellY_R - relativeY_R ;
		break ;
	case 4: *globalX_R = cellX_R - relativeY_R ;
		*globalY_R = cellY_R - relativeX_R ;
		break ;
	case 5: *globalX_R = cellX_R + relativeY_R ;
		*globalY_R = cellY_R + relativeX_R ;
		break ;
	case 6: *globalX_R = cellX_R - relativeY_R ;
		*globalY_R = cellY_R + relativeX_R ;
		break ;
	case 7: *globalX_R = cellX_R + relativeY_R ;
		*globalY_R = cellY_R - relativeX_R ;
		break ;
    }
} /* end Ytrans_rel_pos */

/* ----------------------------------------------------------------- 
    Ytrans_post - same as Ytrans_rel_pos except it does cell centering
    in addition.
----------------------------------------------------------------- */
VOID Ytrans_rel_post( orient_R, globalX_R, globalY_R, relativeX_R, relativeY_R,
    cellX_R, cellY_R )               
INT orient_R, *globalX_R, *globalY_R, relativeX_R, relativeY_R,         
    cellX_R, cellY_R ;               
{
    Ytrans_rel_pos( orient_R, globalX_R, globalY_R, relativeX_R, 
	relativeY_R, cellX_R, cellY_R ) ;
    if( Ytrans_xflagG ){
	(*globalX_R)++ ;
    }
    if( Ytrans_yflagG ){
	(*globalY_R)++ ;
    }
} /* end Ytrans_rel_post */

/* ***************** STATIC VARIABLE DEFINITIONS ******************* */
static INT num_ptS ;		/* number of points for boundary */
static INT *pt_xS = NIL(INT *);	/* x point array */
static INT *pt_yS = NIL(INT *);	/* y point array */
static INT countS ;		/* count during output */
static INT pt_allocS ;		/* point allocation */
#define EXPECTEDPTS	2

VOID Ytrans_boun_init()
{
    /* allocate the space for reoordering the points of the macro */
    num_ptS = 0 ;
    /* allocate memory if needed */
    if(!(pt_xS)){
	pt_allocS = EXPECTEDPTS ;
	pt_xS = YVECTOR_MALLOC( 1, pt_allocS, INT ) ;
	pt_yS = YVECTOR_MALLOC( 1, pt_allocS, INT ) ;
    }
} /* end Ytrans_boun_init() */

VOID Ytrans_boun_add( x, y )
INT x, y ;
{
    /* increase the space if necessary */
    if( ++num_ptS >= pt_allocS ){
	pt_allocS += EXPECTEDPTS ;
	pt_xS = YVECTOR_REALLOC( pt_xS, 1, pt_allocS, INT ) ;
	pt_yS = YVECTOR_REALLOC( pt_yS, 1, pt_allocS, INT ) ;
    }
    pt_xS[num_ptS] = x ;
    pt_yS[num_ptS] = y ;
} /* end Ytrans_boun_add() */

VOID Ytrans_boun( orient, new_xc, new_yc, use_new_orient )
INT new_xc, new_yc, orient ;
BOOL use_new_orient ;
{

    INT pt ;			/* counter */
    INT xc, yc ;		/* cell center */
    INT x, y ;			/* cell relative coordinate */
    INT l, b, r, t ;            /* bounding box of boundary */

    l = INT_MAX ;
    r = INT_MIN ;
    b = INT_MAX ;
    t = INT_MIN ;
    for( pt = 1; pt <= num_ptS; pt++ ){
      l = MIN( l, pt_xS[pt] ) ;
      r = MAX( r, pt_xS[pt] ) ;
      b = MIN( b, pt_yS[pt] ) ;
      t = MAX( t, pt_yS[pt] ) ;
    } /* end for( pt = 1... */
    xc = (l+r)/2 ;
    yc = (b+t)/2 ;
    if(!(use_new_orient)){
	/* use what we calculated as center instead */
	new_xc = xc ;
	new_yc = yc ;
    }
    Ytrans_init( l, b, r, t, orient ) ;
    for( pt = 1; pt <= num_ptS; pt++ ){
	x = pt_xS[pt] -= xc ;
	y = pt_yS[pt] -= yc ;
	
	REL_POST( orient, 
	    pt_xS[pt], pt_yS[pt],   /* result */
	    x, y,                   /* cell relative */
	    new_xc, new_yc ) ;      /* cell center */
    } /* end for( pt = 1... */
    countS = 0 ;
} /* end Ytrans_boun() */

BOOL Ytrans_boun_pt( x_ret, y_ret )
INT *x_ret, *y_ret ;
{
    INT this_pt ;               /* used in calculating direction */
    INT nextpos, nextneg ;      /* next position CW and CCW */
    INT x, y ;			/* the lowest left pt coordinate */
    INT pt ;			/* counter */
    static INT ptL ;		/* current point in array */
    static INT limitL ;		/* current direction limit */

    if( ++countS == 1 ){
	/* now we need to reorder the vertices */
	/* all the points have been rotated into their proper view and */
	/* stored in the vertices array.  Now find lowest of the left pts. */
	x = INT_MAX ;
	y = INT_MAX ;
	for( pt = 1; pt <= num_ptS; pt++ ){
	    if( pt_xS[pt] <= x ){
		if( pt_yS[pt] <= y ){
		    x = pt_xS[pt] ;
		    y = pt_yS[pt] ;
		    ptL = pt ;
		}
	    }
	} /* end for( pt = 1; pt <= numptS... */

	/* find the adjacent points in forward and backwards directions */
	if( ptL == num_ptS ){
	    nextpos = 1 ;
	} else {
	    nextpos = ptL + 1 ;
	}
	if( ptL == 1 ){
	    nextneg = num_ptS ;
	} else {
	    nextneg = ptL - 1 ;
	}
	/* set limitL which tells us direction */
	if( pt_xS[nextpos] == x && pt_yS[nextpos] > y ){
	    /* clockwise is positive */
	    limitL = ptL + num_ptS; 
	} else if( pt_xS[nextneg] == x && pt_yS[nextneg] > y ){
	    /* clockwise is negative */
	    limitL = ptL - num_ptS ;
	} else {
	    M( ERRMSG, "Ytrans_next_pt",
	    "couldn't find clockwise direction for boundary\n" ) ;
	    return( FALSE ) ;
	}
	*x_ret = x ; *y_ret = y ;
	return( TRUE ) ;

    } else if( countS <= num_ptS ){
	if( limitL >= num_ptS ){
	    /* clockwise is positive */
	    if( ++ptL < limitL ) {
		if( ptL > num_ptS ){
		    this_pt = ptL - num_ptS ;
		} else {
		    this_pt = ptL ;
		}
		*x_ret = pt_xS[this_pt] ; *y_ret = pt_yS[this_pt] ;
		return( TRUE ) ;
	    }
	} else {
	    /* clockwise is negative */
	    if( --ptL > limitL ){
		if( ptL < 1 ){
		    this_pt = num_ptS + ptL ;
		} else {
		    this_pt = ptL ;
		}
		*x_ret = pt_xS[this_pt] ; *y_ret = pt_yS[this_pt] ;
		return( TRUE ) ;
	    }
	}
    } /* end if( countS <= numptS ... */

    return( FALSE ) ;   /* no more points to deal with */

} /* end Ytrans_boun_pt() */

VOID Ytrans_boun_free()
{
    /* free allocate memory */
    if(pt_xS){
	YVECTOR_FREE( pt_xS, 1 ) ;
	YVECTOR_FREE( pt_yS, 1 ) ;
	pt_xS = NIL(INT *) ;
    }
} /* end Ytrans_boun_free() */
/* ***************************************************************** */
