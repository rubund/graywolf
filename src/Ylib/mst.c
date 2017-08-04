/*
 *   Copyright (C) 1989-1992 Yale University
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
FILE:	    mst.c                                       
DESCRIPTION:These routines find the minimum spanning tree of a net.
	    USING PRIM'S ALGORITHM.
CONTENTS:   
DATE:	    Jun 21, 1989 
REVISIONS:  Thu Oct 17 11:09:03 EDT 1991 - renamed functions according
		to convention. Added mst_color function.
----------------------------------------------------------------- */
#include <globals.h>

static    INT numpinS ;          /* allocation of memory */
static    INT *nodeXS ;          /* array of x locations for pins */
static    INT *nodeYS ;          /* array of y locations for pins */
static    INT *lowcostS ;        /* array of lowest cost for neighbor */
static    INT *closestS ;        /* array of closest neighbor for pin */
static    INT countS ;           /* number of pins for this net */
static    INT colorS = TWRED ;   /* default color is red */

#define	SQUARE(a)    ((a)*(a))
#define	INF	     INT_MAX

int cost(int i, int j) /* return the square of the Euclidian distance of 2 points */
{
	if (i==j){
	    return INF;
	}
	return( SQUARE(nodeXS[i]-nodeXS[j])+SQUARE(nodeYS[i]-nodeYS[j]) );
} /* end cost */

void Ymst_init( int numpins )
{
    numpinS = numpins ;
    nodeXS = YMALLOC( numpins, INT ) ;
    nodeYS = YMALLOC( numpins, INT ) ;
    lowcostS = YMALLOC( numpins+1, INT ) ;
    closestS = YCALLOC( numpins+1,INT ) ;
    countS = 0 ;
} /* end Ymst_init() */

void Ymst_free()
{
    YFREE(nodeXS) ;
    YFREE(nodeYS) ;	
    YFREE(closestS) ;
    YFREE(lowcostS) ;	
} /* end Ymst_free() */

void Ymst_clear()
{
    countS = 0 ;
} /* end Yclear_mst() */

void Ymst_addpt( int x, int y )
{
    if( countS >= numpinS ){
	printf( "Out of space - update number of pins for MST\n"); 
	return ;
    }
    nodeXS[countS] = x ;
    nodeYS[countS] = y ;
    countS++ ;
} /* end Ymst_addpt() */

void Ymst_draw()
{
    INT mincost ;             /* minimum cost for pin */
    INT closest_pt ;          /* closest neighbor for pin */
    INT i ;                   /* a counter */
    INT j ;                   /* a counter */
    INT k ;                   /* a counter */
    INT c ;                   /* temp for cost */

    for( i = 1 ; i < countS; i++) {
	closestS[i] = 0 ;
	lowcostS[i] = cost( 0,i ) ;	
    }
    for( i = 1; i < countS; i++) {
	mincost = lowcostS[1] ;
	k = 1 ;
	for( j = 2 ; j < countS ; j++ ){
	    if( lowcostS[j] < mincost ) {
		mincost = lowcostS[j] ;
		k = j ;
	    }
	}
	closest_pt = closestS[k] ;
	TWdrawLine( i, nodeXS[k], nodeYS[k], 
	    nodeXS[closest_pt], nodeYS[closest_pt], colorS, NULL ) ;
	lowcostS[k] = INF;
	for( j = 1 ; j < countS ; j++ ){
	    if( ( c = cost(k,j) ) < lowcostS[j] && lowcostS[j] < INF ){
		lowcostS[j] = c;
		closestS[j] = k;
	    }
	}
    } /* end for( i = 1;... */

} /* end Ymst_draw() */

void Ymst_enumerate( int *x1, int *y1, int *x2, int *y2, BOOL startFlag )
{
    INT mincost ;             /* minimum cost for pin */
    INT closest_pt ;          /* closest neighbor for pin */
    INT j ;                   /* a counter */
    INT k ;                   /* a counter */
    INT c ;                   /* temp for cost */
    static INT i ;            /* keep track of the edge */

    if( startFlag ){
	for( i = 1 ; i < countS; i++) {
	    closestS[i] = 0 ;
	    lowcostS[i] = cost( 0,i ) ;	
	}
	i = 1;
    } else {
	if( ++i >= countS ){
	    return ; /* no more edges left */
	}
    }
    mincost = lowcostS[1] ;
    k = 1 ;
    for( j = 2 ; j < countS ; j++ ){
	if( lowcostS[j] < mincost ) {
	    mincost = lowcostS[j] ;
	    k = j ;
	}
    }
    closest_pt = closestS[k] ;
    *x1 = nodeXS[k] ;
    *y1 = nodeYS[k] ;
    *x2 = nodeXS[closest_pt] ;
    *y2 = nodeYS[closest_pt] ;

    lowcostS[k] = INF;
    for( j = 1 ; j < countS ; j++ ){
	if( ( c = cost(k,j) ) < lowcostS[j] && lowcostS[j] < INF ){
	    lowcostS[j] = c;
	    closestS[j] = k;
	}
    }

} /* end Ymst_enumerate() */

void Ymst_color( int color )
{
    colorS = color ;
} /* end Ymst_color */
