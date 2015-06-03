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
FILE:	    svd.c                                       
DESCRIPTION:Singular value decomposition routines.
CONTENTS:
DATE:       Tue Jan 15 00:52:31 EST 1991 - original coding.
REVISIONS:  Tue Jan 15 22:26:57 PST 1991 - added TRUE return condition.
	    Thu Jan 17 00:59:42 PST 1991 - now notify user of
		singular value when in debug mode.
	    Thu Jan 24 20:17:23 PST 1991 - changed vector routines.
	    Tue Mar 12 16:56:22 CST 1991 - got rid of unnecessary
		assignment.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) svd.c version 1.7 12/15/91" ;
#endif

#include <yalecad/base.h>
#include <yalecad/message.h>
#include <yalecad/debug.h>
#include <yalecad/linalg.h>

/* static definitions */
static DOUBLE atS,btS,ctS;
static DOUBLE maxarg1S,maxarg2S;

#undef MAX
#define PYTHAG(a,b) ((atS=fabs(a)) > (btS=fabs(b)) ? \
(ctS=btS/atS,atS*sqrt(1.0+ctS*ctS)) : (btS ? (ctS=atS/btS,btS*sqrt(1.0+ctS*ctS)): 0.0))
#define MAX(a,b) (maxarg1S=(a),maxarg2S=(b),(maxarg1S) > (maxarg2S) ?\
        (maxarg1S) : (maxarg2S))
#define SIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
#define TOLERANCE 1e-6

/* ---------------------------------------------------------------
    Procedure Ysvd_solve - solve a set of linear equations using SVD.
    Input: A - a  m x n matrix.
    Input: B - a  m x k matrix.
    Output: X - a  n x k matrix.
-----------------------------------------------------------------*/
BOOL Ysvd_solve( A, B, Xret )
YMPTR A, B, *Xret ;
{
    INT i, j, jj, k, m, n, t ;
    YMPTR U, W, V ;
    DOUBLE wmax, threshold, sum ;
    DOUBLE **b, **u, **v, **w, **x ;
    DOUBLE *tmp ;

    m = A->rows ;
    n = A->columns ;
    k = B->columns ;

    /* next make sure B conforms to A */
    if( m != B->rows ){
	M( ERRMSG, "Ysvd_solve", "The matrices A and B do not conform\n" ) ;
	sprintf( YmsgG, "A = %d x %d   B = %d x %d\n\n", m, n, B->rows, k ) ;
	M( ERRMSG, NULL, YmsgG ) ;
	return(FALSE) ;
    }

    /* first factor A using SVD */
    if(!(Ysvd_decompose( A, &U, &W, &V ))){
	return( FALSE ) ;
    }
    D( "Ysvd_solve/decompose",
	YMPTR Vt ;
	YMPTR WVt ;
	YMPTR UWVt ;

	fprintf( stderr, "U:\n" ) ;
	Ymatrix_disp( U ) ;
	fprintf( stderr, "W:\n" ) ;
	Ymatrix_disp( W ) ;
	fprintf( stderr, "V:\n" ) ;
	Ymatrix_disp( V ) ;

	Vt = Ymatrix_transpose( V ) ;
	fprintf( stderr, "Vt:\n" ) ;
	Ymatrix_disp( Vt ) ;
	WVt = Ymatrix_mult( W, Vt ) ;
	UWVt = Ymatrix_mult( U, WVt ) ;
	fprintf( stderr, "U * W * Vt:\n" ) ;
	Ymatrix_disp( UWVt ) ;
    ) ;

    /* now throw away those things which make the answer unstable */
    /* first find the maximum w */
    wmax=0.0;
    w = W->m ;
    for (j=1;j<=n;j++){
	if(w[j][j] > wmax){
	    wmax=w[j][j] ;
	}
    }
    /* next create a threshold */
    threshold = TOLERANCE * wmax ;
    /* now zero below threshold */
    for (j=1;j<=n;j++){
	if( w[j][j] < threshold ){
	    D( "Ysvd_solve/singular",
		fprintf( stderr, "Singular value for w[%d]:%4.6le\n",
		    j, w[j][j] ) ;
	    ) ;
	    w[j][j] = 0.0;
	}
    }
    D( "Ysvd_solve/a_weight",
	fprintf( stderr, "W:\n" ) ;
	Ymatrix_disp( W ) ;
    ) ;

    D( "Ysvd_solve/backsub",
	YMPTR Ut ;
	YMPTR Utb ;
	YMPTR WUtb ;
	YMPTR Xver ;
	YMPTR Wprime ;
	DOUBLE **wp ;

	Ut = Ymatrix_transpose( U ) ;
	Utb = Ymatrix_mult( Ut, B ) ;
	Wprime = Ymatrix_copy( W ) ;
	wp = Wprime->m ;
	for( i = 1; i <= n; i++ ){
	    if( wp[i][i] ){
		wp[i][i] = 1 / wp[i][i] ;
	    }
	}
	WUtb = Ymatrix_mult( Wprime, Utb ) ;
	Xver = Ymatrix_mult( V, WUtb ) ;
	fprintf( stderr, "V * 1 / w * Ut * B:\n" ) ;
	Ymatrix_disp( Xver ) ;
    ) ;

    /* now we are ready for back substitution */
    /* x = V * diag( 1 / w ) * Ut * b */
    *Xret = Ymatrix_create( n, k ) ;
    x = (*Xret)->m ;
    u = U->m ;
    v = V->m ;
    b = B->m ;
    tmp = YVECTOR_MALLOC(1,n,DOUBLE) ;
    for( t = 1; t <= k ; t++ ){
	/* first calculate Ut * b */
        for (j=1;j<=n;j++) {
	    sum=0.0;
	    if (w[j][j]) {
		for (i=1;i<=m;i++){
		    sum += u[i][j]*b[i][t];
		}
		/* this is the divide by 1 / w */
		sum /= w[j][j];
	    }
	    tmp[j]=sum;
        }
	/* now multiply by V */
        for (j=1;j<=n;j++) {
	    sum=0.0;
	    for (jj=1;jj<=n;jj++){
		sum += v[j][jj]*tmp[jj];
	    }
	    x[j][t]=sum;
        }
    } /* end looping on columns */

    /* free memory */
    YVECTOR_FREE(tmp,1) ;
    Ymatrix_free( U ) ;
    Ymatrix_free( V ) ;
    Ymatrix_free( W ) ;
    return( TRUE ) ;

} /* end Ysvd_solve */

/* -------------------------------------------------------------
   Procedure Ysvd_deccompose - decompose A into U W Vt.
   Input: A an m x n matrix.
   Output: U - a m x n column orthogonal matrix.
   Output: W - a n x 1 column vector.
   Output: V - a n x n column orthogonal matrix.
---------------------------------------------------------------- */
BOOL Ysvd_decompose( A, Uret, Wret, Vret )
YMPTR A, *Uret, *Wret, *Vret ;
{
    INT m, n ;
    INT flag,i,its,j,jj,k,l,nm;
    DOUBLE c,f,h,s,x,y,z;
    DOUBLE anorm=0.0,g=0.0,scale=0.0;
    DOUBLE *rv1;
    DOUBLE **a,*w,**v, **wfast ;

    m = A->rows ;
    n = A->columns ;

    if (m < n){
	sprintf( YmsgG, "m < n (A = %d x %d)\n", m, n ) ;
	M( ERRMSG, "Ysvd_decompose", YmsgG ) ;
	M( ERRMSG, NULL, "You must augment A with extra zero rows");
	return( FALSE ) ;
    }

    /* create the working space */
    *Uret = Ymatrix_copy( A ) ;
    a = (*Uret)->m ;
    *Wret = Ymatrix_eye( n ) ;
    *Vret = Ymatrix_create( n, n ) ;
    v = (*Vret)->m ;
    w = YVECTOR_MALLOC(1,n,DOUBLE);
    rv1 = YVECTOR_MALLOC(1,n,DOUBLE);

    /* Householder reduction to bidiagonal form */
    for (i=1;i<=n;i++) {
	l=i+1;
	rv1[i]=scale*g;
	g=s=scale=0.0;
	if (i <= m) {
	    for (k=i;k<=m;k++) scale += fabs(a[k][i]);
		if (scale) {
		    for (k=i;k<=m;k++) {
			a[k][i] /= scale;
			s += a[k][i]*a[k][i];
		    }
		    f=a[i][i];
		    g = -SIGN(sqrt(s),f);
		    h=f*g-s;
		    a[i][i]=f-g;
		    if (i != n) {
			for (j=l;j<=n;j++) {
			    for (s=0.0,k=i;k<=m;k++) s += a[k][i]*a[k][j];
			    f=s/h;
			    for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
			}
		    }
		    for (k=i;k<=m;k++) a[k][i] *= scale;
		}
	}
	w[i]=scale*g;
	g=s=scale=0.0;
	if (i <= m && i != n) {
	    for (k=l;k<=n;k++) scale += fabs(a[i][k]);
	    if (scale) {
		for (k=l;k<=n;k++) {
		    a[i][k] /= scale;
		    s += a[i][k]*a[i][k];
		}
		f=a[i][l];
		g = -SIGN(sqrt(s),f);
		h=f*g-s;
		a[i][l]=f-g;
		for (k=l;k<=n;k++) rv1[k]=a[i][k]/h;
		if (i != m) {
		    for (j=l;j<=m;j++) {
			for (s=0.0,k=l;k<=n;k++) s += a[j][k]*a[i][k];
			for (k=l;k<=n;k++) a[j][k] += s*rv1[k];
		    }
		}
		for (k=l;k<=n;k++) a[i][k] *= scale;
	    }
	}
	anorm=MAX(anorm,(fabs(w[i])+fabs(rv1[i])));
    } /* end of Householder reduction */

    /* Accummulation of right-hand transformations */
    for (i=n;i>=1;i--) {
	if (i < n) {
	    if (g) {
		for (j=l;j<=n;j++)
		    v[j][i]=(a[i][j]/a[i][l])/g;
		for (j=l;j<=n;j++) {
		    for (s=0.0,k=l;k<=n;k++) s += a[i][k]*v[k][j];
			for (k=l;k<=n;k++) v[k][j] += s*v[k][i];
		}
	    }
	    for (j=l;j<=n;j++) v[i][j]=v[j][i]=0.0;
	}
	v[i][i]=1.0;
	g=rv1[i];
	l=i;
    } /* end accummulation of right-hand transformation */

    /* Accummulation of left-hand transformations */
    for (i=n;i>=1;i--) {
	l=i+1;
	g=w[i];
	if (i < n) for (j=l;j<=n;j++) a[i][j]=0.0;
	if (g) {
	    g=1.0/g;
	    if (i != n) {
		for (j=l;j<=n;j++) {
		    for (s=0.0,k=l;k<=m;k++) s += a[k][i]*a[k][j];
		    f=(s/a[i][i])*g;
		    for (k=i;k<=m;k++) a[k][j] += f*a[k][i];
		}
	    }
	    for (j=i;j<=m;j++) a[j][i] *= g;
	} else {
	    for (j=i;j<=m;j++) a[j][i]=0.0;
	}
	++a[i][i];
    } /* end accummulation of left-hand transformation */

    /* Diagonalization of the bidiagonal form */
    /* loop over singular values */
    for (k=n;k>=1;k--) {
	/* loop over allowed iterations */
	for (its=1;its<=30;its++) {
	    flag=1;
	    for (l=k;l>=1;l--) {
		nm=l-1;
		if (fabs(rv1[l])+anorm == anorm) {
		    flag=0;
		    break;
		}
		if (fabs(w[nm])+anorm == anorm) break;
	    }
	    if (flag) {
		s=1.0;
		for (i=l;i<=k;i++) {
		    f=s*rv1[i];
		    if (fabs(f)+anorm != anorm) {
			g=w[i];
			h=PYTHAG(f,g);
			w[i]=h;
			h=1.0/h;
			c=g*h;
			s=(-f*h);
			for (j=1;j<=m;j++) {
			    y=a[j][nm];
			    z=a[j][i];
			    a[j][nm]=y*c+z*s;
			    a[j][i]=z*c-y*s;
			}
		    }
		}
	    }
	    /* Convergence */
	    z=w[k];
	    if (l == k) {
		if (z < 0.0) {
		    w[k] = -z;
		    for (j=1;j<=n;j++) v[j][k]=(-v[j][k]);
		}
		break;
	    }
	    if (its == 30){
		M(ERRMSG, "Ysvd_decompose", "No convergence in 30 SVDCMP iterations");
		return( FALSE ) ;
	    }
	    x=w[l];
	    nm=k-1;
	    y=w[nm];
	    g=rv1[nm];
	    h=rv1[k];
	    f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
	    g=PYTHAG(f,1.0);
	    f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;

	    /* The QR transformation */
	    c=s=1.0;
	    for (j=l;j<=nm;j++) {
		i=j+1;
		g=rv1[i];
		y=w[i];
		h=s*g;
		g=c*g;
		z=PYTHAG(f,h);
		rv1[j]=z;
		c=f/z;
		s=h/z;
		f=x*c+g*s;
		g=g*c-x*s;
		h=y*s;
		y=y*c;
		for (jj=1;jj<=n;jj++) {
		    x=v[jj][j];
		    z=v[jj][i];
		    v[jj][j]=x*c+z*s;
		    v[jj][i]=z*c-x*s;
		}
		z=PYTHAG(f,h);
		w[j]=z;
		if (z) {
		    z=1.0/z;
		    c=f*z;
		    s=h*z;
		}
		f=(c*g)+(s*y);
		x=(c*y)-(s*g);
		for (jj=1;jj<=m;jj++) {
		    y=a[jj][j];
		    z=a[jj][i];
		    a[jj][j]=y*c+z*s;
		    a[jj][i]=z*c-y*s;
		}
	    }
	    rv1[l]=0.0;
	    rv1[k]=f;
	    w[k]=x;
	}
    }

    /* now transfer w to Wret */
    wfast = (*Wret)->m ;
    for( i = 1; i <= n; i++ ){
	wfast[i][i] = w[i] ;
    }
    YVECTOR_FREE(rv1,1) ;
    YVECTOR_FREE(w,1) ;
    return( TRUE ) ;

} /* end Ysvd_decompose */

#undef SIGN
#undef MAX
#undef PYTHAG

#ifdef TEST
/* *************************** TEST SOLVER ****************** */
#include <yalecad/cleanup.h>
main(argc,argv)
int argc;
char *argv[];
{

    YMPTR  X, y, Xt, XtX, B, Xret ;

    /* start up cleanup handler */
    YINITCLEANUP( argv[0], NULL, MAYBEDUMP ) ;
    YsetDebug( TRUE ) ;

    X      = Ymatrix_create( 5, 3 ) ;
    y      = Ymatrix_create( 5, 1 ) ;

    /* load matrix */
    YMATRIX( X, 1, 1 ) = 1.0 ;
    YMATRIX( X, 1, 2 ) = 0.0 ;
    YMATRIX( X, 1, 3 ) = 0.0 ;
    YMATRIX( X, 2, 1 ) = 1.0 ;
    YMATRIX( X, 2, 2 ) = 67.0 ;
    YMATRIX( X, 2, 3 ) = 448.0 ;
    YMATRIX( X, 3, 1 ) = 1.0 ;
    YMATRIX( X, 3, 2 ) = 157.0 ;
    YMATRIX( X, 3, 3 ) = 24649.0 ;
    YMATRIX( X, 4, 1 ) = 1.0 ;
    YMATRIX( X, 4, 2 ) = 67.0 ;
    YMATRIX( X, 4, 3 ) = 448.0 ;
    YMATRIX( X, 5, 1 ) = 1.0 ;
    YMATRIX( X, 5, 2 ) = 0.0 ;
    YMATRIX( X, 5, 3 ) = 0.0 ;
    M( MSG, NULL, "X:\n" ) ;
    Ymatrix_disp( X ) ;

    YMATRIX( y, 1, 1 ) = 0.7  ;
    YMATRIX( y, 2, 1 ) = 0.7  ;
    YMATRIX( y, 3, 1 ) = 0.15 ;
    YMATRIX( y, 4, 1 ) = 0.7  ;
    YMATRIX( y, 5, 1 ) = 0.7  ;
    M( MSG, NULL, "y:\n" ) ;
    Ymatrix_disp( y ) ;

    M( MSG, NULL, "\nXt:\n" ) ;
    Xt = Ymatrix_transpose( X ) ;
    Ymatrix_disp( Xt ) ;

    M( MSG, NULL, "\nXtX:\n" ) ;
    XtX = Ymatrix_mult( Xt, X ) ;
    Ymatrix_disp( XtX ) ;

    /* now test eq solver */
    B = Ymatrix_mult( Xt, y ) ;
    M( MSG, NULL, "\nB originally\n");
    Ymatrix_disp( B ) ;

    if( Ysvd_solve( XtX, B, &Xret )){
	M( MSG, NULL, "\nSvd Solver\n");
	Ymatrix_disp( Xret ) ;
    }

    YexitPgm(0) ;

} /* end main program */

#endif /* TEST */
