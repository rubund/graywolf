#ifndef lint
static char SccsId[] = "@(#) perimeter.c version 3.4 12/16/90" ;
#endif

#include <custom.h>
#include <yalecad/buster.h>
#include <yalecad/debug.h>


perimeter( A, numcorners )
YBUSTBOXPTR A ;
INT numcorners ;
{

INT i , sum ;

sum = 0 ;
for( i = 1 ; i <= numcorners ; i++ ) {
    if( i == numcorners ) {
	sum += ABS(A[1].x - A[i].x) + ABS(A[1].y - A[i].y) ;
    } else {
	sum += ABS(A[i + 1].x - A[i].x) + ABS(A[i + 1].y - A[i].y) ;
    }
}
return( sum ) ;

}
