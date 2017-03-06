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
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_linalg.h>

gsl_matrix_disp( mptr, rows, cols )
gsl_matrix *mptr ;
int rows, cols;
{
    INT i, j ;

    for( i=0; i < rows; i++ ){
        for( j=0; j < cols; j++ ){
            fprintf( stderr, "% 4.4le ", gsl_matrix_get(mptr, i, j)) ;
        }
        fprintf( stderr, "\n" ) ;
    }
    fprintf( stderr, "\n" ) ;
} /* end gsl_matrix_disp */

gsl_vector_disp( vptr, rows )
gsl_vector *vptr ;
int rows;
{
    INT i;

    for( i=0; i < rows; i++ ){
        fprintf( stderr, "% 4.4le ", gsl_vector_get(vptr, i)) ;
    }
    fprintf( stderr, "\n" ) ;
} /* end gsl_vector_disp */

static set_pins( A, center, loc, tile_side, sidepins, count )
gsl_matrix *A ;              /* the matrix holding x y positions */
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
	    gsl_matrix_set(A, count, 5, (DOUBLE) sidepins[side]);
	} else {
	    M( ERRMSG, "adapt_wire_estimator", 
		"Trouble finding pinside - defaulting to 0.\n" ) ;
	    gsl_matrix_set(A, count, 5, 0.0);
	}
    } else {
	/* no pins for cell */
	gsl_matrix_set(A, count, 5, 0.0);
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
    INT solved ;	    /* status of gsl_linalg_SV_solve */
    INT *find_pin_sides() ; /* find number of pins on all sides */
    char filename[LRECL] ;  /* output the results of the SVD fit */
    FILE  *fp ;             /* write out the results */
    gsl_matrix *A ;              /* the matrix holding x y positions */
    gsl_vector *B ;              /* the resulting global routing space*/
    gsl_vector *Xret ;           /* the solution to At*A*x = At*B */
    DOUBLE x, y ;           /* cell placement */
    DOUBLE lf, rf, bf, tf ; /* the global position of the rtiles */
    DOUBLE xlength, ylength;/* length of side */
    DOUBLE xrouting, yrouting;/* routing space */
    CELLBOXPTR cptr ;       /* current pointer to cell */
    RTILEBOXPTR rptr ;      /* traverse tiles */

    gsl_matrix *U ;
    gsl_matrix *V ;
    gsl_vector *S ;
    gsl_vector *work ;

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
 	A = gsl_matrix_alloc(6, 6);
 	B = gsl_vector_alloc(6);
    } else {
	/* now we can size the matrices */
 	A = gsl_matrix_alloc(count, 6);
 	B = gsl_vector_alloc(count);
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
	    gsl_matrix_set(A, count, 0, 1.0);	/* for finding const */
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
		gsl_matrix_set(A, count, 1, x);
		gsl_matrix_set(A, count, 2, x * x);
		gsl_matrix_set(A, count, 3, y);
		gsl_matrix_set(A, count, 4, y * y);
		gsl_vector_set(B, count, xrouting);
		break ;
	    case TILET:
		/* calculate x and y */
		x = (lf + rf) / 2.0 ;
		y = bf ;
		set_pins( A, (l+r)/2, b, TILET, sidepins, count ) ;
		gsl_matrix_set(A, count, 1, x);
		gsl_matrix_set(A, count, 2, x * x);
		gsl_matrix_set(A, count, 3, y);
		gsl_matrix_set(A, count, 4, y * y);
		gsl_vector_set(B, count, yrouting);
		break ;
	    case TILER:
		/* calculate x and y */
		x = lf ;
		y = (bf + tf) / 2.0 ;
		set_pins( A, (b+t)/2, l, TILER, sidepins, count ) ;
		gsl_matrix_set(A, count, 1, x);
		gsl_matrix_set(A, count, 2, x * x);
		gsl_matrix_set(A, count, 3, y);
		gsl_matrix_set(A, count, 4, y * y);
		gsl_vector_set(B, count, xrouting);
		break ;
	    case TILEB:
		/* calculate x and y */
		x = (lf + rf) / 2.0 ;
		y = tf ;
		set_pins( A, (l+r)/2, t, TILEB, sidepins, count ) ;
		gsl_matrix_set(A, count, 1, x);
		gsl_matrix_set(A, count, 2, x * x);
		gsl_matrix_set(A, count, 3, y);
		gsl_matrix_set(A, count, 4, y * y);
		gsl_vector_set(B, count, yrouting);
		break ;
	    } /* end switch */
	    count++ ;
	}
	if( sidepins ){
	    Yvector_free( sidepins, 1, sizeof(INT) ) ;
	}

    } /* end loop on cells */

    /* now make sure we have at least 6 rows */
    if( count < 1 ){
	M( ERRMSG, "adapt_wire_estimator", 
	"Too few cells for TimberWolfMC.  Must abort\n" ) ;
	YexitPgm(PGMFAIL) ;

    } else if( count < 6 ){
	/* pad with zeros */
	for( ; count < 6; count++ ){
	    for( i = 0; i < 6; i++ ){
		gsl_matrix_set(A, count, i, gsl_matrix_get(A, 0, i));
	    }
	    gsl_vector_set(B, count, gsl_vector_get(B, 0));
	}
    }

    D( "TWMC/awe/init",
	fprintf( stderr,"\nA:\n" ) ;
	gsl_matrix_disp( A, count < 6 ? 6 : count, 6 ) ;
	fprintf( stderr,"\nB:\n" ) ;
	gsl_vector_disp( B, count < 6 ? 6 : count ) ;
    ) ;

    /* now solve using SVD */
    U = gsl_matrix_alloc((count < 6) ? 6 : count, 6);
    V = gsl_matrix_alloc(6, 6);
    S = gsl_vector_alloc(6);
    work = gsl_vector_alloc(6);
    gsl_matrix_memcpy(U, A);
    gsl_linalg_SV_decomp(U, V, S, work);
    solved = gsl_linalg_SV_solve(U, V, S, B, Xret);

    if( solved ){
	sprintf( filename, "%s.mest", cktNameG ) ;
	fp = TWOPEN( filename, "w", ABORT ) ;
	fprintf( fpoG, "\nThe results of the SVD fit are:\n" ) ;
	for( i = 0; i < 6; i++ ){
	    HPO( fp, gsl_vector_get(Xret, i));
	    HPO( fpoG, gsl_vector_get(Xret, i));
	}
	TWCLOSE( fp ) ;
    }

    gsl_matrix_free(U);
    gsl_matrix_free(V);
    gsl_vector_free(S);
    gsl_vector_free(work);

    D( "TWMC/awe/answer",
	gsl_vector *AX ; /* multiply the answer */
	gsl_vector *R ;
	gsl_matrix *Q ;
	INT c ;
	DOUBLE d ;
	DOUBLE dx ;

	AX = gsl_vector_alloc(count < 6 ? count : 6);
	R =  gsl_vector_alloc(count < 6 ? count : 6);

	/* Compute AX = A * Xret */
	for ( c = 0; c < (count < 6) ? 6 : count; c++ ) {
	    d = 0.0;
	    dx = gsl_vector_get(Xret, c);
	    for ( i = 0; i < 6; i++ ) {
		d += gsl_matrix_get(A, c, i) * dx;
	    gsl_set_vector(AX, c, d);
	}

	/* Compute R = AX - B */
	gsl_vector_memcpy(R, AX);
	gsl_vector_sub( R, B );

	Q = gsl_matrix_alloc( count, 7 );
	for( c = 0; c < count; c++ ){
	    gsl_matrix_set(Q, c, 0, gsl_matrix_get(A, c, 1)) ;  /* x */
	    gsl_matrix_set(Q, c, 1, gsl_matrix_get(A, c, 3)) ;  /* y */
	    gsl_matrix_set(Q, c, 2, gsl_matrix_get(A, c, 5)) ;  /* pins */
	    gsl_matrix_set(Q, c, 3, gsl_vector_get(B, c)) ;  /* B */
	    gsl_matrix_set(Q, c, 4, gsl_matrix_get(AX, c, 0)) ;  /* AX */
	    gsl_matrix_set(Q, c, 5, gsl_matrix_get(R, c, 0)) ;  /* B - AX */
	    gsl_matrix_set(Q, c, 6, gsl_matrix_get(Q, c, 5) /
			gsl_matrix_get(Q, c, 3)) ;  /* error */
	}
	fprintf( stderr, "\n    x,          y,        B,       pins,         AB,           B - AX,      error:\n");
	gsl_matrix_disp( Q, count, 7 ) ;
	gsl_matrix_free(Q);
	gsl_vector_free(R);
	gsl_vector_free(AX);
    ) ;

    /* now done free the matrices */
    gsl_matrix_free( A );
    gsl_vector_free( B );
    gsl_vector_free( Xret );

} /* end adapt_wire_estimator */
