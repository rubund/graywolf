/*
 *   Copyright (C) 1991-1992 Yale University
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
FILE:	    wireratio.c                                       
DESCRIPTION:This file contains routines for analyzing random to
	    optimal wire ratio.
CONTENTS:   wireratio
DATE:	    Tue Jan 15 20:54:36 PST 1991
REVISIONS:  Fri Jan 25 18:15:36 PST 1991 - added numpins to equations
		untested at this time.
	    Mon Feb  4 02:27:16 EST 1991 - working SVD fit algorithm
		that also checks for small cases.
	    Thu Feb  7 00:22:56 EST 1991 - last solution for small
		cases didn't resize memory properly.
	    Wed May  1 19:18:55 EDT 1991 - added switchbox field 
		so we can ignore these areas during wire estimation.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) wireratio.c version 3.9 3/10/92" ;
#endif

#include <custom.h> 
#include <dens.h> 
#include <yalecad/debug.h>
#include <yalecad/file.h>
#include <yalecad/linalg.h>

static set_pins( A, center, loc, tile_side, sidepins, count )
YMPTR  A ;              /* the matrix holding x y positions */
INT    center ;
INT    loc ;
INT    tile_side ;
INT    *sidepins ;
INT    count ;
{
    INT side ;   /* the matching side for a given tile */

    if( sidepins ){
	side = find_tile_side( center, loc, tile_side ) ;
	if( side ){
	    YMATRIX( A, count, 6 ) = (DOUBLE) sidepins[side] ;
	} else {
	    M( ERRMSG, "adapt_wire_estimator", 
		"Trouble finding pinside - defaulting to 0.\n" ) ;
	    YMATRIX( A, count, 6 ) = 0.0 ;
	}
    } else {
	/* no pins for cell */
	YMATRIX( A, count, 6 ) = 0.0 ;
    }
} /* end  set_pins */

adapt_wire_estimator()
{
    INT i ;                 /* coefficient counter */
    INT cell ;              /* cell counter */
    INT count ;             /* count number of tiles */
    INT xc, yc ;            /* cell center */
    INT *sidepins ;         /* array holding #pins for side */
    INT    l, r, b, t ;     /* the global position of the rtiles */
    INT *find_pin_sides() ; /* find number of pins on all sides */
    char filename[LRECL] ;  /* output the results of the SVD fit */
    FILE  *fp ;             /* write out the results */
    YMPTR  A ;              /* the matrix holding x y positions */
    YMPTR  B ;              /* the resulting global routing space*/
    YMPTR  Xret ;           /* the solution to At*A*x = At*B */
    DOUBLE x, y ;           /* cell placement */
    DOUBLE lf, rf, bf, tf ; /* the global position of the rtiles */
    DOUBLE xlength, ylength;/* length of side */
    DOUBLE xrouting, yrouting;/* routing space */
    CELLBOXPTR cptr ;       /* current pointer to cell */
    RTILEBOXPTR rptr ;      /* traverse tiles */

    if(!(routingTilesG)){
	return ;
    }
    /* first count the number of routing tiles */
    count = 0 ;
    for( cell = 1 ; cell <= numcellsG; cell++ ){
	for( rptr=routingTilesG[cell];rptr;rptr=rptr->next ){
	    if( rptr->switchbox ){
		/* switchbox densities are not accurate discard */
		continue ;
	    }
	    count++ ;
	}
    }
    if( count < 6 ){
	/* now we can size the matrices */
	A = Ymatrix_create( 6, 6 ) ;
	B = Ymatrix_create( 6, 1 ) ;
    } else {
	/* now we can size the matrices */
	A = Ymatrix_create( count, 6 ) ;
	B = Ymatrix_create( count, 1 ) ;
    }

    /* initialize the pin counting routines */
    init_wire_est() ;

    /* we eventually want to solve AtA x = At B */

    /* now fill in the data in the matrices. */
    xlength = (DOUBLE) (blockrG - blocklG) ;
    ylength = (DOUBLE) (blocktG - blockbG) ;
    count = 0 ;
    for( cell = 1 ; cell <= numcellsG; cell++ ){
	cptr = cellarrayG[ cell ] ;
	xc = cptr->xcenter ;
	yc = cptr->ycenter ;
	sidepins = find_pin_sides( cell ) ;
	for( rptr=routingTilesG[cell];rptr;rptr=rptr->next ){
	    if( rptr->switchbox ){
		/* switchbox densities are not accurate discard */
		continue ;
	    }
	    count++ ;
	    YMATRIX( A, count, 1 ) = 1.0 ; /* for finding const */
	    l = xc + rptr->x1 ;
	    r = xc + rptr->x2 ;
	    b = yc + rptr->y1 ;
	    t = yc + rptr->y2 ;
	    xrouting = (DOUBLE)(r - l) ;
	    yrouting = (DOUBLE)(t - b) ;
	    lf = ( (DOUBLE)(l - blocklG ) ) / xlength ;
	    if( lf < 0.0 )   lf = 0.0 ;
	    rf = ( (DOUBLE)(r - blocklG ) ) / xlength ;
	    bf = ( (DOUBLE)(b - blockbG ) ) / ylength ;
	    if( bf < 0.0 )   bf = 0.0 ;
	    tf = ( (DOUBLE)(t - blockbG ) ) / ylength ;
	    switch( rptr->side ){
	    case TILEL:
		/* calculate x and y */
		x = rf ;
		y = (bf + tf) / 2.0 ;
		set_pins( A, (b+t)/2, r, TILEL, sidepins, count ) ;
		YMATRIX( A, count, 2 ) = x ;
		YMATRIX( A, count, 3 ) = x * x ;
		YMATRIX( A, count, 4 ) = y ;
		YMATRIX( A, count, 5 ) = y * y ;
		YMATRIX( B, count, 1 ) = xrouting ;
		break ;
	    case TILET:
		/* calculate x and y */
		x = (lf + rf) / 2.0 ;
		y = bf ;
		set_pins( A, (l+r)/2, b, TILET, sidepins, count ) ;
		YMATRIX( A, count, 2 ) = x ;
		YMATRIX( A, count, 3 ) = x * x ;
		YMATRIX( A, count, 4 ) = y ;
		YMATRIX( A, count, 5 ) = y * y ;
		YMATRIX( B, count, 1 ) = yrouting ;
		break ;
	    case TILER:
		/* calculate x and y */
		x = lf ;
		y = (bf + tf) / 2.0 ;
		set_pins( A, (b+t)/2, l, TILER, sidepins, count ) ;
		YMATRIX( A, count, 2 ) = x ;
		YMATRIX( A, count, 3 ) = x * x ;
		YMATRIX( A, count, 4 ) = y ;
		YMATRIX( A, count, 5 ) = y * y ;
		YMATRIX( B, count, 1 ) = xrouting ;
		break ;
	    case TILEB:
		/* calculate x and y */
		x = (lf + rf) / 2.0 ;
		y = tf ;
		set_pins( A, (l+r)/2, t, TILEB, sidepins, count ) ;
		YMATRIX( A, count, 2 ) = x ;
		YMATRIX( A, count, 3 ) = x * x ;
		YMATRIX( A, count, 4 ) = y ;
		YMATRIX( A, count, 5 ) = y * y ;
		YMATRIX( B, count, 1 ) = yrouting ;
		break ;
	    } /* end switch */
	}
	if( sidepins ){
	    Yvector_free( sidepins, 1, sizeof(INT) ) ;
	}

    } /* end loop on cells */

    /* now make sure we have at least 6 rows */
    if( count <= 1 ){
	M( ERRMSG, "adapt_wire_estimator", 
	"Too few cells for TimberWolfMC.  Must abort\n" ) ;
	YexitPgm(PGMFAIL) ;

    } else if( count < 6 ){
	/* pad with zeros */
	for( ++count; count <= 6; count++ ){
	    for( i = 1; i <= 6; i++ ){
		YMATRIX( A, count, i ) = YMATRIX( A, 1, i ) ;
	    }
	    YMATRIX( B, count, 1 ) = YMATRIX( B, 1, 1 ) ;
	}
    }

    D( "TWMC/awe/init",
	fprintf( stderr,"\nA:\n" ) ;
	Ymatrix_disp( A ) ;
	fprintf( stderr,"\nB:\n" ) ;
	Ymatrix_disp( B ) ;
    ) ;

    /* now solve using SVD */
    if( Ysvd_solve( A,  B, &Xret )){
	sprintf( filename, "%s.mest", cktNameG ) ;
	fp = TWOPEN( filename, "w", ABORT ) ;
	fprintf( fpoG, "\nThe results of the SVD fit are:\n" ) ;
	for( i = 1; i <= 6; i++ ){
	    HPO( fp, YMATRIX(Xret,i,1) ) ; 
	    HPO( fpoG, YMATRIX(Xret,i,1) ) ; 
	}
	TWCLOSE( fp ) ;
    }


    D( "TWMC/awe/answer",
	YMPTR AX ; /* multiply the answer */
	YMPTR R ;
	YMPTR Q ;
	INT c ;

	/* now output the differences */
	AX = Ymatrix_mult( A, Xret ) ;
	R = Ymatrix_sub( AX, B ) ;
	Q = Ymatrix_create( count, 7 ) ;
	for( c = 1; c <= count; c++ ){
	    YMATRIX( Q, c, 1 ) = YMATRIX( A, c, 2 ) ; /* x */
	    YMATRIX( Q, c, 2 ) = YMATRIX( A, c, 4 ) ; /* y */
	    YMATRIX( Q, c, 3 ) = YMATRIX( A, c, 6 ) ; /* pins */
	    YMATRIX( Q, c, 4 ) = YMATRIX( B, c, 1 ) ; /* B */
	    YMATRIX( Q, c, 5 ) = YMATRIX( AX, c, 1 ) ; /* AX */
	    YMATRIX( Q, c, 6 ) = YMATRIX( R, c, 1 ) ; /* B - AX */
	    YMATRIX( Q, c, 7 ) =   /* error */
		YMATRIX( Q, c, 6 ) / YMATRIX( Q, c, 4 ) ;
	}
	fprintf( stderr, "\n    x,          y,        B,       pins,         AB,           B - AX,      error:\n");
	Ymatrix_disp( Q ) ;
    ) ;

    /* now done free the matrices */
    Ymatrix_free( A ) ;
    Ymatrix_free( B ) ;
    Ymatrix_free( Xret ) ;

} /* end adapt_wire_estimator */
