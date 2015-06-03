/*
 *   Copyright (C) 1991 Yale University
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
FILE:	    matrix.c                                       
DESCRIPTION:a curvefitting program.
CONTENTS:
DATE:       Tue Jan 15 01:35:11 EST 1991 - original coding.
REVISIONS:  Sun Nov  3 12:48:39 EST 1991 - made matrix memory
		allocation more efficient using YVECTOR routines.
            12/09/91 - Add prototype macro for non ansi compiler  -R.A.Weier
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) matrix.c version 1.3 12/9/91" ;
#endif

#include <yalecad/base.h>
#include <yalecad/message.h>
#include <yalecad/debug.h>
#include <yalecad/linalg.h>

static DOUBLE find_det( P3(DOUBLE **a, INT rows, INT columns ) );
static sign_cof( P2(INT row, INT column) ) ;

/* ***************************************************************** 
    Allocate the space for the matrix.
   **************************************************************** */
YMPTR Ymatrix_create( rows, columns )
INT rows, columns ;
{
    YMPTR mptr ;   /* pointer to matrix record */
    DOUBLE **m ;  /* pointer to matrix space */
    INT i ;

    mptr = YMALLOC( 1, YMBOX ) ;
    /* now allocate memory for rows */
    m = mptr->m = YVECTOR_MALLOC( 1, rows, DOUBLE * ) ;
    /* now allocate memory for columns */
    for( i = 1;i<= rows; i++ ){
	m[i] = YVECTOR_CALLOC( 1, columns, DOUBLE ) ;
    }
    mptr->rows = rows ;
    mptr->columns = columns ;
    return( mptr ) ;
} /* end Ymatrix_create */

/* ***************************************************************** 
    Free the space of a matrix.
   **************************************************************** */
YMPTR Ymatrix_free( mptr )
YMPTR mptr ;   /* pointer to matrix record */
{
    DOUBLE **m ;  /* pointer to matrix space */
    INT i ;
    INT rows ;

    rows = mptr->rows ;
    m = mptr->m ;
    /* free memory */
    for( i = 1; i <= rows; i++ ){
	YVECTOR_FREE( m[i], 1 ) ;
    }
    YVECTOR_FREE( m, 1 ) ;
    YFREE( mptr ) ;
} /* end Ymatrix_free */

YMPTR Ymatrix_transpose( mptr )
YMPTR mptr ;
{
    INT i, j ;
    DOUBLE **m ;
    DOUBLE **b ;
    YMPTR buf ;

    buf = Ymatrix_create( mptr->columns, mptr->rows ) ;

    m = mptr->m ;
    b = buf->m ;

    for( i=1; i <= mptr->rows; i++ ){
	for( j=1; j <= mptr->columns; j++ ){
	    b[j][i] = m[i][j] ;
	}
    }
    return( buf ) ;
    
} /* end Ymatrix_transpose */

YMPTR Ymatrix_mult( aptr, bptr )
YMPTR aptr, bptr ;
{
    INT i, j, k, n ;
    DOUBLE **a ;
    DOUBLE **b ;
    DOUBLE **c ;
    DOUBLE result ;
    YMPTR buf ;

    /* first perform error checking */
    if( aptr->columns != bptr->rows ){
	sprintf( YmsgG,
	"Matrices cannot be multiplied together A (%dx%d) B(%dx%d)\n",
	aptr->rows, aptr->columns, bptr->rows, bptr->columns ) ;
	M( ERRMSG, "Ymatrix_mult", YmsgG ) ;
    }
    buf = Ymatrix_create( aptr->rows, bptr->columns ) ;

    a = aptr->m ;
    b = bptr->m ;
    c = buf->m ;
    n = aptr->columns ;

    for( i=1; i <= aptr->rows; i++ ){
	for( j=1; j <= bptr->columns; j++ ){
	    result = 0.0 ;
	    for( k=1; k<= n; k++ ){
		result += a[i][k] * b[k][j] ;
	    }
	    c[i][j] = result ;
	}
    }
    return( buf ) ;
    
} /* end Ymatrix_mult */

YMPTR Ymatrix_sub( aptr, bptr )
YMPTR aptr, bptr ;
{
    INT i, j ;
    DOUBLE **a, **b, **c ;
    DOUBLE *fast_a, *fast_b, *fast_c ;
    DOUBLE result ;
    YMPTR buf ;

    /* first perform error checking */
    if( aptr->rows != bptr->rows ||
	aptr->columns != bptr->columns ){
	sprintf( YmsgG,
	"Matrices cannot be subtracted A (%dx%d) B(%dx%d)\n",
	aptr->rows, aptr->columns, bptr->rows, bptr->columns ) ;
	M( ERRMSG, "Ymatrix_sub", YmsgG ) ;
    }
    buf = Ymatrix_create( aptr->rows, aptr->columns ) ;

    a = aptr->m ;
    b = bptr->m ;
    c = buf->m ;
    for( i=1; i <= aptr->rows; i++ ){
	fast_a = a[i] ;
	fast_b = b[i] ;
	fast_c = c[i] ;
	for( j=1; j <= aptr->columns; j++ ){
	    fast_c[j] = fast_a[j] - fast_b[j] ;
	}
    }
    return( buf ) ;
    
} /* end Ymatrix_sub */

Ymatrix_disp( mptr )
YMPTR mptr ;
{
    DOUBLE **m ;
    INT i, j ;
    m = mptr->m ;

    for( i=1; i <= mptr->rows; i++ ){
	for( j=1; j <= mptr->columns; j++ ){
	    fprintf( stderr, "% 4.4le ", m[i][j] ) ;
	}
	fprintf( stderr, "\n" ) ;
    }
    fprintf( stderr, "\n" ) ;
} /* end Ymatrix_disp */

YMPTR Ymatrix_eye( size )
INT size ;
{
    INT i, j ;
    DOUBLE **m ;
    DOUBLE *fast ;
    YMPTR buf ;

    buf = Ymatrix_create( size, size ) ;
    m = buf->m ;
    for( i = 1; i <= size; i++ ){
	fast = m[i] ;
	for( j = 1; j <= size ; j++ ){
	    if( i == j ){
		/* one is exact in fp. */
		fast[j] = 1.0 ;
	    } else {
		fast[j] = YZERO ;
	    }
	}
    }
    return( buf ) ;
} /* end Ymatrix_eye */

Ymatrix_zero( matrix )
YMPTR matrix ;
{
    INT r, c, i, j ;
    DOUBLE *fast ;
    DOUBLE **m ;

    m = matrix->m ;
    r = matrix->rows ;
    c = matrix->columns ;
    for( i = 1; i <= r; i++ ){
	fast = m[i] ;
	for( j = 1; j <= c ; j++ ){
	    fast[j] = YZERO ;
	}
    }
} /* end Ymatrix_zero */


YMPTR Ymatrix_copy( input )
YMPTR input ;
{
    INT i, j ;
    DOUBLE **in_mat, **copy_mat ;
    DOUBLE *in_fast, *copy_fast ;
    YMPTR copy ;

    copy = Ymatrix_create( input->rows, input->columns ) ;
    in_mat = input->m ;
    copy_mat = copy->m ;
    for( i = 1; i <= input->rows; i++ ){
	in_fast = in_mat[i] ;
	copy_fast = copy_mat[i] ;
	for( j = 1; j <= input->columns ; j++ ){
	    copy_fast[j] = in_fast[j] ;
	}
    }
    return( copy ) ;
} /* end Ymatrix_copy */

YMPTR Ymatrix_linv( aptr )
YMPTR aptr ;
{
    INT i, j, k, n ;
    DOUBLE **a ;
    DOUBLE **b ;
    DOUBLE **c ;
    DOUBLE det, recip_det ;
    YMPTR cof ;
    YMPTR buf ;

    buf = Ymatrix_create( aptr->rows, aptr->columns ) ;
    cof = Ymatrix_cofactors( aptr ) ;

    det = 0.0 ;
    a = aptr->m ;
    c = cof->m ;
    /* find the determinant using first column */
    for( i=1; i <= aptr->rows; i++ ){
	det += a[i][1] * c[i][1] ;
    }

    ASSERT( det == find_det( aptr->m, aptr->rows, aptr->columns ),
	"Yinv", "Problem with determinant" ) ;

    if( det != 0.0 ){
	recip_det = 1.0 / det ;
	buf = Ymatrix_transpose( cof ) ;
	b = buf->m ;
	for( i=1; i <= buf->rows; i++ ){
	    for( j=1; j <= buf->columns; j++ ){
		b[i][j] *= recip_det ;
	    }
	}
    }
    return( buf ) ;
    
} /* end Ymatrix_linv */

YMPTR Ymatrix_cofactors( aptr )
YMPTR aptr ;
{

    INT i, j, k, l ;
    INT r, c ;
    INT rows, columns ;
    DOUBLE cofactor ;
    DOUBLE **a, **b, **m ;
    YMPTR buf ;

    buf = Ymatrix_create( aptr->rows, aptr->columns ) ;
    rows = aptr->rows ;
    columns = aptr->columns ;
    a = aptr->m ; /* a matrix - want cofactors of this matrix */
    b = buf->m ;  /* buf matrix */

    /* allocate space for a row - 1 x column - 1 clone of a matrix */
    m = YVECTOR_MALLOC( 1, rows - 1, DOUBLE * ) ;
    /* now allocate memory for columns */
    for( i = 1;i< rows; i++ ){
	m[i] = YVECTOR_CALLOC( 1, columns-1, DOUBLE ) ;
    }

    /* process all cofactors */
    for( i=1; i <= rows; i++ ){
	for( j=1; j <= columns; j++ ){

	    /* load cofactor array */
	    /* strike row and column */
	    r = 0 ;
	    for( k = 1; k <= rows; k++ ){
		if( k == i ){
		    continue ;
		}
		r++ ;
		c = 0 ;
		for( l = 1; l <= columns; l++ ){
		    if( l == j ){
			continue ;
		    }
		    m[r][++c] = a[k][l] ;
		}
	    } /* end load of cofactor array */

	    /* process one cofactor */
	    cofactor = sign_cof(i,j) * find_det( m, rows-1, columns-1 ) ;
	    /* store result in buf */
	    b[i][j] = cofactor ;
		    
	}
    }
    /* now time to free memory */
    for( i = 1;i< columns; i++ ){
	YVECTOR_FREE( m[i], 1 ) ;
    }
    YVECTOR_FREE( m, 1 ) ;
    return( buf ) ;
} /* end Ymatrix_cofactors */

/* find a determinant - works on the actual memory arrays - not user */
/* records */
static DOUBLE find_det( a, rows, columns )
DOUBLE **a ;
INT rows, columns ;
{
    DOUBLE result ;
    DOUBLE cofactor ;
    DOUBLE **m ;
    INT i, j, k ;
    INT r, c ;

    if( rows == 1 && columns == 1 ){
	result = a[1][1] ;

    } else if( rows == 2 && columns == 2 ){
	result = a[1][1] * a[2][2] - a[2][1] * a[1][2] ;

    } else {

	/* allocate space for an row-1 x columns-1 matrix */
	m = YVECTOR_MALLOC( 1, rows-1, DOUBLE * ) ;
	/* now allocate memory for columns */
	for( i = 1;i< rows; i++ ){
	    m[i] = YVECTOR_MALLOC( 1, columns-1, DOUBLE ) ;
	}

	/* do Laplace expansion along top row - all columns */
	result = 0.0 ;
	for( k=1; k <= columns; k++ ){

	    /* load array by striking row and column */
	    /* struck row is always one */
	    r = 0 ;
	    for( i = 2; i <= rows; i++ ){
		r++ ;
		c = 0 ;
		for( j = 1; j <= columns; j++ ){
		    if( j == k ){
			continue ;
		    }
		    m[r][++c] = a[i][j] ;
		}
	    } /* end load of cofactor array */

	    ASSERT( r == rows-1, "Yfind_det","problem with rows" ) ; 
	    ASSERT( c == columns-1, "Yfind_det","problem with rows" ) ; 

	    /* recursively call find_det to get result */
	    cofactor = sign_cof(1,k) * find_det( m, rows-1, columns-1 ) ;
	    result += a[1][k] * cofactor ;
	}

	/* now time to free memory */
	for( i = 1;i< columns; i++ ){
	    YVECTOR_FREE( m[i], 1 ) ;
	}
	YVECTOR_FREE( m, 1 ) ;
    }

    return( result ) ;

} /* end find_det */

static sign_cof( row, column )
INT row, column ;
{
    INT sum ;

    /* implements  (-1) ** (row + column) */
    sum = row + column ;
    if( sum % 2 ){ /* odd power */
	return( -1.0 ) ;
    } else { /* even power */
	return( 1.0 ) ;
    }
} /* end sign_cof */
