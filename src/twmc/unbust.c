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
FILE:	    unbust.c                                       
DESCRIPTION:routine which combines busted tiles for output
CONTENTS:   unbust()
	    initPts( INT , char *, INT )
DATE:	    Jan 29, 1988 
REVISIONS:  Aug 16, 1989 - rewrote using new general algorithm.
	    Sep 19, 1989 - now debug uses stderr.
	    Sep 20, 1989 - added initialization to numptS+1
	    Sep 25, 1989 - correctly works for objects created with
		buster with minimal amount of work.
	    Sun Dec 16 00:35:22 EST 1990 - now use library definition
		of buster.
	    Sun Jan 20 21:34:36 PST 1991 - ported to AIX.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) unbust.c version 3.7 1/20/91" ;
#endif

#include <yalecad/base.h>
#include <yalecad/buster.h>
#include <yalecad/debug.h>
#include <yalecad/message.h>

#define E 0
#define T 1
#define L 2
#define R 3
#define B 4
#define HOWMANY  0
#define EXPECTEDNUMPINS  4

typedef struct {
    INT tile ;    /* tile that point is attached */
    INT x ;       /* x position of point */
    INT y ;       /* y position of point */
    INT Vnum ;    /* position in VPts array */
    INT Hnum ;    /* position in HPts array */
    INT order ;   /* order that points should be output - negative means invalid */
    BOOL marked ; /* point has been used */
} POINTBOX, *POINTPTR ;

static INT numptS ;           /* number of total points for figure */
static INT ptAllocS ;         /* number of space allocated for points */
static POINTPTR *VptS = NULL ;/* vertical point array */
static POINTPTR *HptS ;       /* horizontal point array */
static YBUSTBOXPTR resultS ;  /* result array */
static BOOL addptS = FALSE ;  /* whether to add points to figures or not*/



static INT find_next_state() ;
static INT remove_redundant_points() ; 
static INT find_next_state();
static INT sortbyXY();
static INT sortbyYX();
static INT sortbyorder();
static INT remove_redundant_points(); 
static add_vpts();
static chek_vpt();
static add_hpts();
static chek_hpt();





static INT nextStateS[5][5] = 
{
    /* ERROR state - E  */  E, E, E, E, E,
    /* UP    state - T  */  E, L, T, R, /* B */ E,
    /* LEFT  state - L  */  E, B, L, T, /* R */ E,
    /* RIGHT state - R  */  E, T, R, B, /* L */ E,
    /* DOWN  state - B  */  E, R, B, L, /* T */ E
} ;

YBUSTBOXPTR unbust()
{

    /*
     *  Algorithm:
     *  Add points to data structures.
     *  Sort points according to x (Hpt array) and y (Vpt array) pos.
     *  Remove redundant points.
     *  If multiple tiles,
     *       Check parallel line segments at change of tile boundary
     *       Extend lines to hit other tile.  Add points at these
     *       intersections.  When all points are added, sort again.
     *       Tile needs only to be rectilinear not rectangular.
     *  and use following Finite Automaton to find minimum containing
     *  figure.
     *  STATE TABLE.
     *  last state   try1  try2  try3  try4
     *      T         L     T     R     B - error
     *      L         B     L     T     R - errror
     *      R         T     R     B     L - error
     *      B         R     B     L     T - error
     *
     *  where
     *  L : -1 in the Hpt array.
     *  R : +1 in the Hpt array.
     *  B : -1 in the Vpt array.
     *  T : +1 in the Vpt array.
     * 
     *  By starting at the lower left point or Hpt[1] and
     *  using the FA. we will traverse the minimum containing
     *  figure of the cell or cells.
     *
     */

     POINTPTR cur_pt ;  /* current point record */
     POINTPTR next_pt ; /* next point record */
     INT sortbyXY() ;   /* sort horizontal points */
     INT sortbyYX() ;   /* sort vertical points */
     INT sortbyorder() ;/* final sort */
     INT i ;            /* counter */
     INT count ;            /* counter */
     INT cur_state ;    /* current state */
     INT next_state ;   /* next state */
     INT limit ;        /* used to detect loops */
     INT order ;        /* used to order the points */
     INT given_num_pts ;/* number of points user gave us */
     INT points_removed;/* number of redundant points removed */
/*
     static INT find_next_state() ;
     static INT remove_redundant_points() ; 
*/

     /* initialize 0 record */
     cur_pt = VptS[0] ;
     cur_pt->order = 0 ;
     cur_pt->marked = TRUE ;
     cur_pt->Vnum = 0 ;
     cur_pt->Hnum = 0 ;
     cur_pt->x = INT_MIN ;
     cur_pt->y = INT_MIN ;

     resultS[HOWMANY].x = 0 ;   /* initialize for error trapping */

     /* sort points */
     /* horizontal points */
     Yquicksort( &(HptS[1]), numptS, sizeof(POINTPTR), sortbyYX ) ;
     /* vertical points */
     Yquicksort( &(VptS[1]), numptS, sizeof(POINTPTR), sortbyXY ) ;

     /* need to delay changing number of points until both arrays are processed */
     points_removed = 0 ;
     points_removed += remove_redundant_points( HptS ) ;
     points_removed += remove_redundant_points( VptS ) ;
     numptS -= points_removed ;

     D( "unbust", fprintf( stderr,"\n" ) ) ;
     given_num_pts = numptS ;
     if( addptS ){
	 /* only add points if necessary */
	 add_vpts( given_num_pts ) ;
	 add_hpts( given_num_pts ) ;
     }

     if( numptS != given_num_pts ){
	/* sort again */
	Yquicksort( &(HptS[1]),numptS,sizeof(POINTPTR),sortbyYX ) ;
	/* vertical points */
	Yquicksort( &(VptS[1]),numptS,sizeof(POINTPTR),sortbyXY ) ;

	points_removed = 0 ;
	points_removed += remove_redundant_points( HptS ) ;
	points_removed += remove_redundant_points( VptS ) ;
	numptS -= points_removed ;
     }

     /* initialize last record */
     cur_pt = VptS[numptS+1] ;
     cur_pt->order = 0 ;
     cur_pt->marked = TRUE ;
     cur_pt->Vnum = numptS+1 ;
     cur_pt->Hnum = numptS+1 ;
     cur_pt->x = INT_MAX ;
     cur_pt->y = INT_MAX ;
     /* note both arrays must be done since redundant points may */
     /* live up here */
     cur_pt = HptS[numptS+1] ;
     cur_pt->order = 0 ;
     cur_pt->marked = TRUE ;
     cur_pt->Vnum = numptS+1 ;
     cur_pt->Hnum = numptS+1 ;
     cur_pt->x = INT_MAX ;
     cur_pt->y = INT_MAX ;

     /* name the points so we can cross reference */
     for( i = 1; i <= numptS; i++ ){
	cur_pt = HptS[i] ;
	cur_pt->Hnum = i ;
     }
     for( i = 1; i <= numptS; i++ ){
	cur_pt = VptS[i] ;
	cur_pt->Vnum = i ;
     }

     dump_pts( VptS ) ;

     /* start automaton */
     /* note we don't mark first point so we can return to it */
     cur_pt = HptS[1] ;
     cur_pt->order = order = 1 ;
     cur_state = R ;
     limit = numptS + 1 ; 
     D( "unbust", fprintf( stderr,"start_state:%d pt:(%d,%d)\n", 
		cur_state, cur_pt->x, cur_pt->y ) ) ;
     for( i = 0; i <= limit; i++ ){  /* infinite loop protector */
	/* determine next state */
	if( next_state = find_next_state( cur_state,cur_pt,&next_pt )){
	    D( "unbust", 
		fprintf( stderr,"next_state:%d next_pt:(%d,%d)\n", 
		next_state, next_pt->x, next_pt->y ) ) ;
	    if( next_pt->order == 1 ){
		/* we know we have made a full circle */
		/* first make sure last point is not colinear */
		if( next_state == cur_state ){
		    order-- ;
		    cur_pt->order = 0 ;
		}
		break ;
	    }
	    /* remove colinear points */
	    if( next_state != cur_state ){
		next_pt->order = ++order ;
	    } else {
		cur_pt->order = 0 ;
		next_pt->order = order ;
	    }
	} else {
	    M( ERRMSG, "unbust", "can't get next state fatal problem\n" ) ;
	    break ;
	}

	/* set up for next state */
	cur_pt = next_pt ;
	cur_state = next_state ;

     } /* end finite automaton */

     if( i == limit ){
	M( ERRMSG, "unbust",
	"we have detected an infinite loop in automaton\n" ) ;
	return ;
     }

     dump_pts( VptS ) ;

     /* now sort points by order for output */
     Yquicksort( &(VptS[1]), numptS, sizeof(POINTPTR), sortbyorder ) ;

     dump_pts( VptS ) ;

     /* now output the points */
     for( count=0,i=0; i <= numptS; i++ ){
	if( VptS[i]->order ){
	    resultS[++count].x = VptS[i]->x ;
	    resultS[count].y = VptS[i]->y ;
	}
     }
     resultS[HOWMANY].x = order ;

     return( resultS ) ;

} /* end unbust */

/* find the next valid state */
static INT find_next_state( cur_state, cur_pt, next_pt )
INT cur_state ;
POINTPTR cur_pt ;
POINTPTR *next_pt ;
{
    INT hpoint ;   /* position of point in HptS */
    INT vpoint ;   /* position of point in HptS */
    INT try ;      /* used to test each possibility */
    POINTPTR try_pt ; /* this is a candidate point */


    /* get two partner of the current state */
    hpoint = cur_pt->Hnum ;
    vpoint = cur_pt->Vnum ;
    ASSERTNRETURN( vpoint > 0 && vpoint <= numptS, "find_next_state",
	"vpoint out of bounds\n" ) ;
    ASSERTNRETURN( hpoint > 0 && hpoint <= numptS, "find_next_state",
	"hpoint out of bounds\n" ) ;
    /* now try possibilities in order */
    for( try = 1; try <= 4; try++ ){
	switch( nextStateS[cur_state][try] ){
	case E:
	    return( E ) ;
	case T:
	    try_pt = VptS[vpoint+1] ;
	    if(!(try_pt->marked) && cur_pt->x == try_pt->x ){
		if( addptS ){
		    try_pt->marked = TRUE ;
		    *next_pt = try_pt ;
		    return( T ) ;
		} else { 
		    /* points must be same or adjacent tiles */
		    if( cur_pt->tile == try_pt->tile ||
		        cur_pt->tile + 1 == try_pt->tile ||
		        cur_pt->tile - 1 == try_pt->tile ){
			try_pt->marked = TRUE ;
			*next_pt = try_pt ;
			return( T ) ;
		    }
		}
	    }
	    break ;
	case L:
	    try_pt = HptS[hpoint-1] ;
	    if(!(try_pt->marked) && cur_pt->y == try_pt->y ){
		if( addptS ){
		    try_pt->marked = TRUE ;
		    *next_pt = try_pt ;
		    return( L ) ;
		} else { 
		    /* points must be same or adjacent tiles */
		    if( cur_pt->tile == try_pt->tile ||
		        cur_pt->tile + 1 == try_pt->tile ||
		        cur_pt->tile - 1 == try_pt->tile ){
			try_pt->marked = TRUE ;
			*next_pt = try_pt ;
			return( L ) ;
		    }
		}
	    }
	    break ;
	case R:
	    try_pt = HptS[hpoint+1] ;
	    if(!(try_pt->marked) && cur_pt->y == try_pt->y ){
		if( addptS ){
		    try_pt->marked = TRUE ;
		    *next_pt = try_pt ;
		    return( R ) ;
		} else { 
		    /* in this case points must be adjacent tiles */
		    if( cur_pt->tile == try_pt->tile ||
		        cur_pt->tile + 1 == try_pt->tile ||
		        cur_pt->tile - 1 == try_pt->tile ){
			try_pt->marked = TRUE ;
			*next_pt = try_pt ;
			return( R ) ;
		    }
		}
	    }
	    break ;
	case B:
	    try_pt = VptS[vpoint-1] ;
	    if(!(try_pt->marked) && cur_pt->x == try_pt->x ){
		if( addptS ){
		    try_pt->marked = TRUE ;
		    *next_pt = try_pt ;
		    return( B ) ;
		} else { 
		    /* in this case points must be adjacent tiles */
		    if( cur_pt->tile == try_pt->tile ||
		        cur_pt->tile + 1 == try_pt->tile ||
		        cur_pt->tile - 1 == try_pt->tile ){
			try_pt->marked = TRUE ;
			*next_pt = try_pt ;
			return( B ) ;
		    }
		}
	    }
	    break ;
	} /* end switch */

    } /* end going thru posibilities */

} /* end get_next_state */

addPt( tile, x, y )
INT tile, x, y ;
{
    INT i ;                   /* counter */
    POINTPTR ptr ;            /* current point */

    /* reallocate space if necessary */
    if( ++numptS >= ptAllocS ){
	ptAllocS += EXPECTEDNUMPINS ;
	VptS = (POINTPTR *) 
	    Ysafe_realloc( VptS, ptAllocS * sizeof(POINTPTR)) ;
	HptS = (POINTPTR *) 
	    Ysafe_realloc( HptS, ptAllocS * sizeof(POINTPTR)) ;
	resultS = (YBUSTBOXPTR) 
	    Ysafe_realloc( resultS, ptAllocS * sizeof(YBUSTBOX)) ;
	for( i = numptS; i < ptAllocS; i++ ){
	    HptS[i] = VptS[i] = (POINTPTR) 
		Ysafe_malloc( sizeof(POINTBOX)) ;
	}
    } /* end space allocation */

    /* now initialize data for point */
    ptr = HptS[numptS] ; /* they point to same object */
    ptr->tile = tile ;
    ptr->x = x ;
    ptr->y = y ;
    ptr->order = 0 ;
    ptr->Vnum = 0 ;
    ptr->Hnum = 0 ;
    ptr->marked = FALSE ;
    D( "addPt", fprintf( stderr,"adding point (%d,%d)...\n", x, y ) ) ;
    return ;
} /* end addPt */

addPts( cell, l, r, b, t ) 
INT cell, l, r, b, t ; 
{
    addPt( cell, l, b ) ;
    addPt( cell, l, t ) ;
    addPt( cell, r, t ) ;
    addPt( cell, r, b ) ;
    D( "addPts", fprintf( stderr, "%d %d %d %d %d %d %d %d\n",
	    l, b, l, t, r, t, r, b ) ) ;
} /* end addPts */

initPts( addpoint_flag )
BOOL addpoint_flag ;
{
    INT i ;     /* counter */

    numptS = 0 ;
    /* set addpoint static flag */
    addptS = addpoint_flag ;
    if(!(VptS)){
	ptAllocS = EXPECTEDNUMPINS ;
	VptS = (POINTPTR *) 
	    Ysafe_malloc( ptAllocS * sizeof(POINTPTR)) ;
	HptS = (POINTPTR *) 
	    Ysafe_malloc( ptAllocS * sizeof(POINTPTR)) ;
	resultS = (YBUSTBOXPTR) 
	    Ysafe_malloc( ptAllocS * sizeof(YBUSTBOX)) ;
	for( i = 0; i < ptAllocS; i++ ){
	    HptS[i] = VptS[i] = (POINTPTR) 
		Ysafe_malloc( sizeof(POINTBOX)) ;
	}
    } else {
	/* make both arrays point to the same object */
	for( i = 0; i < ptAllocS; i++ ){
	    HptS[i] = VptS[i] ;
	}
    }
    return ;
} /* end initPts */

/* sort by x first then y */
static INT sortbyXY( pointA , pointB )
POINTPTR *pointA , *pointB ;
{
    if( (*pointA)->x != (*pointB)->x ){
	return( (*pointA)->x - (*pointB)->x ) ;
    } else {
	/* if x's are equal sort by y's */
	return( (*pointA)->y - (*pointB)->y ) ;
    }
} /* end sortbyXY */

/* sort by y first then x */
static INT sortbyYX( pointA , pointB )
POINTPTR *pointA , *pointB ;
{
    if( (*pointA)->y != (*pointB)->y ){
	return( (*pointA)->y - (*pointB)->y ) ;
    } else {
	/* if y's are equal sort by x's */
	return( (*pointA)->x - (*pointB)->x ) ;
    }
} /* end sortbyYX */

/* sort by order */
static INT sortbyorder( pointA , pointB )
POINTPTR *pointA , *pointB ;
{
    return( (*pointA)->order - (*pointB)->order ) ;
} /* end sortbyorder */

static INT remove_redundant_points( pt_array ) 
POINTPTR *pt_array ;
{
    POINTPTR ptptr ;           /* pointer to point record */
    POINTPTR *tempArray ;      /* temporary pointer for manipulating points */
    INT oldX ;                 /* last x point */
    INT oldY ;                 /* last y point */
    INT redundant ;            /* number of redundant points */
    INT i ;                    /* counter */
    INT top ;                  /* top of the point array */
    INT bottom ;               /* bottom of the point array */
    BOOL no_ignored_pts ;      /* invalid points exist in the array */
    POINTPTR oldptr ;          /* the last irredundant point */

    ptptr = pt_array[0] ;
    oldX = ptptr->x ;
    oldY = ptptr->y ;
    redundant = 0 ;

    /* check to see if we have redundant points */
    no_ignored_pts = TRUE ;
    for( i = 1; i <= numptS; i++ ){
	ptptr = pt_array[i] ;
	/* ignore already redundant points */
	if( ptptr->order == -1 ){
	    no_ignored_pts = FALSE ;
	    continue ;
	}
	if( ptptr->x == oldX && ptptr->y == oldY ){
	    /* redundant point */
	    ptptr->order = -1 ;
	    redundant++ ;
	    /* need to set tile to smaller of two points */
	    /* so that all tiles will be adjacent */
	    oldptr->tile = MIN( oldptr->tile, ptptr->tile ) ;
	}
	oldX = ptptr->x ;
	oldY = ptptr->y ;
	oldptr = ptptr ;
    }
    if( redundant == 0 && no_ignored_pts ){
	/* array is fine */
	return( 0 ) ;
    }

    D( "remove_redundant_points",
	fprintf( stderr,"found %d redundant points\n", redundant ) ) ;
    /* we need to do work to remove point */
    tempArray = (POINTPTR *) Ysafe_malloc( (numptS+1)*sizeof(POINTPTR) ) ;
    for( i=0; i <= numptS; i++ ) {
	tempArray[i] = pt_array[i]  ;
    }
    /* now write valid points back */
    bottom = 0 ;
    top = numptS ;
    for( i=0; i <= numptS; i++ ) {
	if( tempArray[i]->order >= 0 ){
	    pt_array[bottom++] = tempArray[i] ;
	} else {
	    /* put invalid points at top of array so we can free them later on */
	    pt_array[top--] = tempArray[i] ;
	}
    }
    D( "remove_redundant_points",
        fprintf( stderr,"bottom:%d top:%d redundant:%d\n", 
	bottom, top, redundant ) ) ;
    dump_pts( pt_array ) ;
    Ysafe_free( tempArray ) ;
    return( redundant ) ;

} /* end remove_redundant_points */

static add_vpts( numpts )
INT numpts ;
{
    POINTPTR tile1ptr ;         /* temp pointer to a point */
    POINTPTR tile2ptr ;         /* temp pointer to a point */
    POINTPTR tile3ptr ;         /* temp pointer to a point */
    POINTPTR tile4ptr ;         /* temp pointer to a point */
    INT  i ;                    /* point counter */
    INT  j ;                    /* point counter */
    INT  oldX ;                 /* used to find vertical points */
    INT  tile1 ;                /* tile1ptr's tile */
    INT  tile2 ;                /* tile2ptr's tile */
    INT  tile3 ;                /* tile3ptr's tile */
    INT  tile4 ;                /* tile4ptr's tile */
    BOOL newTiles ;             /* when true load 4 points otherwise load two */

    newTiles = TRUE ;
    D( "add_vpts",fprintf( stderr, "add_vpts:numpoints:%d\n", numpts ) ) ;
    /* process four points at a time */
    for( i = 1; i+3 <= numpts; ){
	if( newTiles ){
	    tile1ptr = VptS[i] ;
	    tile2ptr = VptS[i+1] ;
	    tile3ptr = VptS[i+2] ;
	    tile4ptr = VptS[i+3] ;
	} else {
	    /* tile1 and tile2 were determined below */
	    tile3ptr = VptS[i] ;
	    tile4ptr = VptS[i+1] ;
	    newTiles = TRUE ;
	}
	tile1 = tile1ptr->tile ;
	tile2 = tile2ptr->tile ;
	tile3 = tile3ptr->tile ;
	tile4 = tile4ptr->tile ;

	if( tile1 == tile2 ){
	    if( tile1 == tile3 && tile1 == tile4 ){
		i += 4 ;
		/* all the same cell */
	    } else if( tile3 == tile4 ){
		/* case two parallel lines that are diffent tiles */
		chek_vpt( tile1ptr, tile2ptr, tile3ptr, tile4ptr ) ;
		i += 4 ;
	    } else {
		/* one and two are same 3 and four are different */
		/* 3 and 4 are colinear points */
		/* find last of the colinear points */
		oldX = VptS[i+2]->x ;
		for( j = i+3;j <= numpts;j++ ){
		    if( VptS[j]->x != oldX ){
			j-- ;
			break ;
		    }
		}
		ASSERT( j > 0 && j <=numpts,
		    "add_vpts","j out of bounds\n" ) ;
		/* j is the last colinear vertical point */
		/* i+2 -- j is a line to compare */
		tile4ptr = VptS[j] ;
		chek_vpt( tile1ptr, tile2ptr, tile3ptr, tile4ptr ) ;
		i = j + 1 ;
		ASSERT( i > 0, "add_vpts","i out of bounds\n" ) ;
	    }
	} else {
	    /* find last of the colinear points */
	    oldX = VptS[i]->x ;
	    for( j = i+1;j <= numpts;j++ ){
		if( VptS[j]->x != oldX ){
		    j-- ;
		    break ;
		}
	    }
	    ASSERT( j > 0 && j <=numpts,"add_vpts","j out of bounds\n" );
	    /* j is the last colinear vertical point */
	    tile2ptr = VptS[j] ;
	    i = j + 1 ;
	}
    } /* end for loop */

    if( i <= numpts ){
	D( "add_vpts",
	    fprintf( stderr, "ERROR[add_vpts] in algorithm\n" ) ) ;
    }

} /* end add_vpts */

static chek_vpt( tile1, tile2, tile3, tile4 )
POINTPTR tile1, tile2, tile3, tile4 ;
{
    /* four cases */
    /* CASE 1  */
    /*
    *   2
    *   X       4
    *   |       |
    *   X       3
    *   1
    */
    if( tile1->y <= tile3->y && tile2->y >= tile4->y ){
	/* add non - redundant upper X */
	if( tile2->y != tile4->y ){
	    addPt( tile2->tile, tile1->x, tile4->y ) ;
	}
	if( tile1->y != tile3->y ){
	    addPt( tile1->tile, tile1->x, tile3->y ) ;
	}
    }
    /* CASE 2  */
    /*
    *           4
    *   2       X
    *   |       |
    *   1       X
    *           3
    */
    if( tile1->y >= tile3->y && tile2->y <= tile4->y ){
	/* add non - redundant upper X */
	if( tile2->y != tile4->y ){
	    addPt( tile4->tile, tile4->x, tile2->y ) ;
	}
	if( tile1->y != tile3->y ){
	    addPt( tile3->tile, tile3->x, tile1->y ) ;
	}
    }
    /* CASE 3  */
    /*
    *           4
    *   2       X
    *   |       |
    *   X       3
    *   1        
    */
    if( tile1->y <= tile3->y && tile2->y <= tile4->y ){
	/* add non - redundant upper X */
	if( tile2->y != tile4->y ){
	    addPt( tile4->tile, tile4->x, tile2->y ) ;
	}
	if( tile1->y != tile3->y ){
	    addPt( tile1->tile, tile1->x, tile3->y ) ;
	}
    }
    /* CASE 4  */
    /*
    *   2        
    *   X       4
    *   |       |
    *   1       X
    *           3 
    */
    if( tile1->y >= tile3->y && tile2->y >= tile4->y ){
	/* add non - redundant upper X */
	if( tile2->y != tile4->y ){
	    addPt( tile2->tile, tile2->x, tile4->y ) ;
	}
	if( tile1->y != tile3->y ){
	    addPt( tile3->tile, tile3->x, tile1->y ) ;
	}
    }
} /* end chek_vpt */


static add_hpts( numpts )
INT numpts ;
{
    POINTPTR tile1ptr ;         /* temp pointer to a point */
    POINTPTR tile2ptr ;         /* temp pointer to a point */
    POINTPTR tile3ptr ;         /* temp pointer to a point */
    POINTPTR tile4ptr ;         /* temp pointer to a point */
    INT  i ;                    /* point counter */
    INT  j ;                    /* point counter */
    INT  oldY ;                 /* used to find horizontal points */
    INT  tile1 ;                /* tile1ptr's tile */
    INT  tile2 ;                /* tile2ptr's tile */
    INT  tile3 ;                /* tile3ptr's tile */
    INT  tile4 ;                /* tile4ptr's tile */
    BOOL newTiles ;             /* when true load 4 points otherwise load two */

    newTiles = TRUE ;
    D( "add_hpts",
	fprintf( stderr, "add_vpts:numpoints:%d\n", numpts ) ) ;
    /* process four points at a time */
    for( i = 1; i+3 <= numpts; ){
	if( newTiles ){
	    tile1ptr = HptS[i] ;
	    tile2ptr = HptS[i+1] ;
	    tile3ptr = HptS[i+2] ;
	    tile4ptr = HptS[i+3] ;
	} else {
	    /* tile1 and tile2 were determined below */
	    tile3ptr = HptS[i] ;
	    tile4ptr = HptS[i+1] ;
	    newTiles = TRUE ;
	}
	tile1 = tile1ptr->tile ;
	tile2 = tile2ptr->tile ;
	tile3 = tile3ptr->tile ;
	tile4 = tile4ptr->tile ;

	if( tile1 == tile2 ){
	    if( tile1 == tile3 && tile1 == tile4 ){
		i += 4 ;
		/* all the same cell */
		continue ; 
	    } else if( tile3 == tile4 ){
		/* case two parallel lines that are diffent tiles */
		chek_hpt( tile1ptr, tile2ptr, tile3ptr, tile4ptr ) ;
		i += 4 ;
	    } else {
		/* one and two are same 3 and four are different */
		/* 3 and 4 are colinear points */
		/* find last of the colinear points */
		oldY = HptS[i+2]->y ;
		for( j = i+3;j <= numpts;j++ ){
		    if( HptS[j]->y != oldY ){
			j-- ;
			break ;
		    }
		}
		ASSERT( j > 0 && j <= numpts,
		    "add_hpts","j out of bounds\n" ) ;
		/* j is the last colinear vertical point */
		/* i+2 -- j is a line to compare */
		tile4ptr = HptS[j] ;
		chek_hpt( tile1ptr, tile2ptr, tile3ptr, tile4ptr ) ;
		i = j + 1 ;
		ASSERT( i > 0, "add_hpts","i out of bounds\n" ) ;
	    }
	} else {
	    /* find last of the colinear points */
	    oldY = HptS[i]->y ;
	    for( j = i+1;j <= numpts;j++ ){
		if( HptS[j]->y != oldY ){
		    j-- ;
		    break ;
		}
	    }
	    ASSERT( j > 0 && j <=numpts,"add_hpts","j out of bounds\n" );
	    /* j is the last colinear vertical point */
	    tile2ptr = HptS[j] ;
	    i = j + 1 ;
	}
    } /* end for loop */
    if( i <= numpts ){
	D( "add_hpts",
	    fprintf( stderr, "ERROR[add_hpts] in algorithm\n" ) ) ;
    }
} /* end add_hpts */

static chek_hpt( tile1, tile2, tile3, tile4 )
POINTPTR tile1, tile2, tile3, tile4 ;
{
    /* four cases */
    /* CASE 1  */
    /*
    *    
    *      3--4
    *            
    *   1--X--X--2
    *            
    */
    if( tile1->x <= tile3->x && tile2->x >= tile4->x ){
	/* add non - redundant lower X */
	if( tile1->y != tile3->y ){
	    addPt( tile1->tile, tile3->x, tile1->y ) ;
	}
	if( tile2->y != tile4->y ){
	    addPt( tile2->tile, tile4->x, tile2->y ) ;
	}
    }
    /* CASE 2  */
    /*
    *    
    *   3--X--X--4
    *            
    *      1--2
    *            
    */
    if( tile1->x >= tile3->x && tile2->x <= tile4->x ){
	/* add non - redundant lower X */
	if( tile1->y != tile3->y ){
	    addPt( tile3->tile, tile1->x, tile3->y ) ;
	}
	if( tile2->y != tile4->y ){
	    addPt( tile4->tile, tile2->x, tile4->y ) ;
	}
    }
    /* CASE 3  */
    /*
    *    
    *      3--X--4
    *            
    *   1--X--2
    *            
    */
    if( tile1->x <= tile3->x && tile2->x <= tile4->x ){
	/* add non - redundant lower X */
	if( tile1->y != tile3->y ){
	    addPt( tile1->tile, tile3->x, tile1->y ) ;
	}
	if( tile2->y != tile4->y ){
	    addPt( tile4->tile, tile2->x, tile4->y ) ;
	}
    }
    /* CASE 4  */
    /*
    *    
    *   3--X--4
    *            
    *      1--X--2
    *            
    */
    if( tile1->x >= tile3->x && tile2->x >= tile4->x ){
	/* add non - redundant lower X */
	if( tile1->y != tile3->y ){
	    addPt( tile3->tile, tile1->x, tile3->y ) ;
	}
	if( tile2->y != tile4->y ){
	    addPt( tile2->tile, tile4->x, tile2->y ) ;
	}
    }
} /* end chek_hpt */

 dump_pts( pt )
 POINTPTR *pt ;
 {
    INT i ;
    POINTPTR ptr ;

    D( "dump_pts",
	fprintf( stderr, "Point dump:\n" ) ) ;
    for( i=0; i <= numptS; i++ ){
	DS( ptr = pt[i] ; ) ;
	D( "dump_pts", fprintf( stderr,
	    "(%d,%d) order=%d marked=%d Vnum=%d Hnum=%d tile:%d\n", 
	    ptr->x, ptr->y, ptr->order, ptr->marked, 
	    ptr->Vnum, ptr->Hnum, ptr->tile ) ) ;
    }
 } /* end dump_pts */
