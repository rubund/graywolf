/*
 *   Copyright (C) 1989-1991 Yale University
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
FILE:	    stat.c                                       
DESCRIPTION:utility routines to calculate min, max, mean, and variance.
CONTENTS:   DOUBLE Ystat_min( array, number_ele, size_ele  )
		char *array ;
		INT  num_ele, INT  size_ele ;
	    DOUBLE Ystat_max( array, number_ele, size_ele  )
		char *array ;
		INT  num_ele, INT  size_ele ;
	    DOUBLE Ystat_mean( array, number_ele, size_ele  )
		char *array ;
		INT  num_ele, INT  size_ele ;
	    DOUBLE Ystat_var( array, number_ele, size_ele, mean  )
		char *array ;
		INT  num_ele, INT  size_ele ;
		DOUBLE mean ;
DATE:	    Mar  7, 1989 
REVISIONS:  Sun Apr 21 21:21:58 EDT 1991 - renamed to Ystat_
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) stat.c (Yale) version 3.3 4/21/91" ;
#endif

#include <yalecad/base.h>


/* Given an array of number_elements of size_element, calculate min */
DOUBLE Ystat_min( array, number_elements, size_element  )
char *array ;
INT  number_elements ;
INT  size_element ;
{

    INT i ;
    DOUBLE min = DBL_MAX ;
    SHORT  *sarray ;
    INT    *iarray ;
    DOUBLE *darray ;

    switch( size_element ){
    case 1:
	for( i=0;i<number_elements;i++){
	    if( (DOUBLE) array[i] < min ){
		min = (DOUBLE) array[i] ;
	    }
	}
	break ;
    case 2:
	sarray = (SHORT *) array ;
	for( i=0;i<number_elements;i++){
	    if( (DOUBLE) sarray[i] < min ){
		min = (DOUBLE) sarray[i] ;
	    }
	}
	break ;
    case 4:
	iarray = (INT *) array ;
	for( i=0;i<number_elements;i++){
	    if( (DOUBLE) iarray[i] < min ){
		min = (DOUBLE) iarray[i] ;
	    }
	}
	break ;
    case 8:
	darray = (DOUBLE *) array ;
	for( i=0;i<number_elements;i++){
	    if( darray[i] < min ){
		min = darray[i] ;
	    }
	}
	break ;
    default: fprintf( stderr, "Unsupported element size:%d\n",
	size_element ) ;
    }

    return(min);
} /* end Ystat_min */

/* Given an array of number_elements of size size_element,calculate max */
DOUBLE Ystat_max( array, number_elements, size_element  )
char *array ;
INT  number_elements ;
INT  size_element ;
{

    INT i ;
    DOUBLE max = DBL_MIN ;
    SHORT  *sarray ;
    INT    *iarray ;
    DOUBLE *darray ;

    switch( size_element ){
    case 1:
	for( i=0;i<number_elements;i++){
	    if( (DOUBLE) array[i] > max ){
		max = (DOUBLE) array[i] ;
	    }
	}
	break ;
    case 2:
	sarray = (SHORT *) array ;
	for( i=0;i<number_elements;i++){
	    if( (DOUBLE) sarray[i] > max ){
		max = (DOUBLE) sarray[i] ;
	    }
	}
	break ;
    case 4:
	iarray = (INT *) array ;
	for( i=0;i<number_elements;i++){
	    if( (DOUBLE) iarray[i] > max ){
		max = (DOUBLE) iarray[i] ;
	    }
	}
	break ;
    case 8:
	darray = (DOUBLE *) array ;
	for( i=0;i<number_elements;i++){
	    if( darray[i] > max ){
		max = darray[i] ;
	    }
	}
	break ;
    default: fprintf( stderr, "Unsupported element size:%d\n",
	size_element ) ;
	
    }
    return(max);
} /* end Ystat_max */

/* Given an array of number_elements of size size_ele, calculate mean */
DOUBLE Ystat_mean( array, number_elements, size_element  )
char *array ;
INT  number_elements ;
INT  size_element ;
{

    INT i ;
    DOUBLE sum = 0.0 ;
    SHORT  *sarray ;
    INT    *iarray ;
    DOUBLE *darray ;

    switch( size_element ){
    case 1:
	for( i=0;i<number_elements;i++){
	    sum += (DOUBLE) array[i] ;
	}
	break ;
    case 2:
	sarray = (SHORT *) array ;
	for( i=0;i<number_elements;i++){
	    sum += (DOUBLE) sarray[i] ;
	}
	break ;
    case 4:
	iarray = (INT *) array ;
	for( i=0;i<number_elements;i++){
	    sum += (DOUBLE) iarray[i] ;
	}
	break ;
    case 8:
	darray = (DOUBLE *) array ;
	for( i=0;i<number_elements;i++){
	    sum += (DOUBLE) darray[i] ;
	}
	break ;
    default: fprintf( stderr, "ERROR:Unsupported element size:%d\n",
	size_element ) ;
    }

    if( number_elements ){
	return( sum / number_elements ) ;
    } else {
	fprintf( stderr, "ERROR:number of elements zero\n" ) ;
	return( 0.0 ) ;
    }

} /* end Ystat_mean */


/* Given an array of number_elements of size size_ele, calculate var */
/* NOTE also need to give mean calculated from above */
DOUBLE Ystat_var( array, number_elements, size_element, mean )
char *array ;
INT  number_elements ;
INT  size_element ;
DOUBLE mean ;
{

    INT i ;
    DOUBLE sum ;
    DOUBLE sum2 = 0.0 ;
    DOUBLE var ;
    SHORT  *sarray ;
    INT    *iarray ;
    DOUBLE *darray ;

    switch( size_element ){
    case 1:

	for( i=0;i<number_elements;i++){
	    sum = (DOUBLE) array[i] ;
	    sum2 += sum * sum ;
	}
	break ;
    case 2:
	sarray = (SHORT *) array ;
	for( i=0;i<number_elements;i++){
	    sum = (DOUBLE) sarray[i] ;
	    sum2 += sum * sum ;
	}
	break ;
    case 4:
	iarray = (INT *) array ;
	for( i=0;i<number_elements;i++){
	    sum = (DOUBLE) iarray[i] ;
	    sum2 += sum * sum ;
	}
	break ;
    case 8:
	darray = (DOUBLE *) array ;
	for( i=0;i<number_elements;i++){
	    sum = (DOUBLE) darray[i] ;
	    sum2 += sum * sum ;
	}
	break ;
    default: fprintf( stderr, "ERROR:Unsupported element size:%d\n",
	size_element ) ;

    }
    if( number_elements > 1 ){
	var = (sum2 - number_elements * mean * mean)/(number_elements-1) ;
	return(ABS(var));
    } else if( number_elements == 1 ){
	return( 0.0 ) ;
    } else {
	fprintf( stderr, "ERROR:number of elements zero\n" ) ;
	return( 0.0 ) ;
    }

} /* end Ystat_variance */
