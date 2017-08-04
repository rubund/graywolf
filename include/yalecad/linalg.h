/* ----------------------------------------------------------------- 
FILE:	    linalg.h
DESCRIPTION:Include file for linear algebra package.
CONTENTS:
DATE:       Tue Jan 15 01:29:36 EST 1991 - original coding.
REVISIONS:
----------------------------------------------------------------- */
#ifndef YLINALG_H
#define YLINALG_H

#define YEPSILON  1.0E-12
#define YMIN     -1.0E-38
#define YZERO     0.0
#define YMATRIX( matrix, r, c )   matrix->m[r][c]

/* the matrix itself */
typedef struct {
    INT    rows ;
    INT    columns ;
    DOUBLE **m ;  
} YMBOX, *YMPTR ;

YMPTR Ymatrix_create( P2(INT rows, INT columns ) ) ;
YMPTR Ymatrix_free( P1( YMPTR mptr ) ) ;
YMPTR Ymatrix_transpose( P1( YMPTR mptr ) ) ;
YMPTR Ymatrix_mult( P2(YMPTR aptr, YMPTR bptr ) ) ;
YMPTR Ymatrix_sub( P2(YMPTR aptr, YMPTR bptr ) ) ;
void Ymatrix_disp( P1(YMPTR mptr ) ) ;
YMPTR Ymatrix_eye( P1(INT size ) ) ;
void Ymatrix_zero( P1(YMPTR matrix ) ) ;
YMPTR Ymatrix_copy( P1(YMPTR input ) ) ;
YMPTR Ymatrix_linv( P1(YMPTR aptr ) ) ;
YMPTR Ymatrix_cofactors( P1(YMPTR aptr ) ) ;

#endif /* LINALG_H */
