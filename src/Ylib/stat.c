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
CONTENTS:   double Ystat_min( array, number_ele, size_ele  )
		char *array ;
		int  num_ele, int  size_ele ;
	    double Ystat_max( array, number_ele, size_ele  )
		char *array ;
		int  num_ele, int  size_ele ;
	    double Ystat_mean( array, number_ele, size_ele  )
		char *array ;
		int  num_ele, int  size_ele ;
	    double Ystat_var( array, number_ele, size_ele, mean  )
		char *array ;
		int  num_ele, int  size_ele ;
		double mean ;
DATE:	    Mar  7, 1989 
REVISIONS:  Sun Apr 21 21:21:58 EDT 1991 - renamed to Ystat_
----------------------------------------------------------------- */
#include <yalecad/base.h>

/* Given an array of number_elements of size_element, calculate min */
double Ystat_min( array, number_elements, size_element  )
char *array ;
int  number_elements ;
int  size_element ;
{

    int i ;
    double min = DBL_MAX ;
    short  *sarray ;
    int    *iarray ;
    double *darray ;

    switch( size_element ){
    case 1:
	for( i=0;i<number_elements;i++){
	    if( (double) array[i] < min ){
		min = (double) array[i] ;
	    }
	}
	break ;
    case 2:
	sarray = (short *) array ;
	for( i=0;i<number_elements;i++){
	    if( (double) sarray[i] < min ){
		min = (double) sarray[i] ;
	    }
	}
	break ;
    case 4:
	iarray = (int *) array ;
	for( i=0;i<number_elements;i++){
	    if( (double) iarray[i] < min ){
		min = (double) iarray[i] ;
	    }
	}
	break ;
    case 8:
	darray = (double *) array ;
	for( i=0;i<number_elements;i++){
	    if( darray[i] < min ){
		min = darray[i] ;
	    }
	}
	break ;
    default: printf( "Unsupported element size:%d\n",
	size_element ) ;
    }

    return(min);
} /* end Ystat_min */

/* Given an array of number_elements of size size_element,calculate max */
double Ystat_max( array, number_elements, size_element  )
char *array ;
int  number_elements ;
int  size_element ;
{

    int i ;
    double max = DBL_MIN ;
    short  *sarray ;
    int    *iarray ;
    double *darray ;

    switch( size_element ){
    case 1:
	for( i=0;i<number_elements;i++){
	    if( (double) array[i] > max ){
		max = (double) array[i] ;
	    }
	}
	break ;
    case 2:
	sarray = (short *) array ;
	for( i=0;i<number_elements;i++){
	    if( (double) sarray[i] > max ){
		max = (double) sarray[i] ;
	    }
	}
	break ;
    case 4:
	iarray = (int *) array ;
	for( i=0;i<number_elements;i++){
	    if( (double) iarray[i] > max ){
		max = (double) iarray[i] ;
	    }
	}
	break ;
    case 8:
	darray = (double *) array ;
	for( i=0;i<number_elements;i++){
	    if( darray[i] > max ){
		max = darray[i] ;
	    }
	}
	break ;
    default: printf( "Unsupported element size:%d\n",
	size_element ) ;
	
    }
    return(max);
} /* end Ystat_max */

/* Given an array of number_elements of size size_ele, calculate mean */
double Ystat_mean( array, number_elements, size_element  )
char *array ;
int  number_elements ;
int  size_element ;
{

    int i ;
    double sum = 0.0 ;
    short  *sarray ;
    int    *iarray ;
    double *darray ;

    switch( size_element ){
    case 1:
	for( i=0;i<number_elements;i++){
	    sum += (double) array[i] ;
	}
	break ;
    case 2:
	sarray = (short *) array ;
	for( i=0;i<number_elements;i++){
	    sum += (double) sarray[i] ;
	}
	break ;
    case 4:
	iarray = (int *) array ;
	for( i=0;i<number_elements;i++){
	    sum += (double) iarray[i] ;
	}
	break ;
    case 8:
	darray = (double *) array ;
	for( i=0;i<number_elements;i++){
	    sum += (double) darray[i] ;
	}
	break ;
    default: printf( "ERROR:Unsupported element size:%d\n",
	size_element ) ;
    }

    if( number_elements ){
	return( sum / number_elements ) ;
    } else {
	printf( "ERROR:number of elements zero\n" ) ;
	return( 0.0 ) ;
    }

} /* end Ystat_mean */


/* Given an array of number_elements of size size_ele, calculate var */
/* NOTE also need to give mean calculated from above */
double Ystat_var( array, number_elements, size_element, mean )
char *array ;
int  number_elements ;
int  size_element ;
double mean ;
{

    int i ;
    double sum ;
    double sum2 = 0.0 ;
    double var ;
    short  *sarray ;
    int    *iarray ;
    double *darray ;

    switch( size_element ){
    case 1:

	for( i=0;i<number_elements;i++){
	    sum = (double) array[i] ;
	    sum2 += sum * sum ;
	}
	break ;
    case 2:
	sarray = (short *) array ;
	for( i=0;i<number_elements;i++){
	    sum = (double) sarray[i] ;
	    sum2 += sum * sum ;
	}
	break ;
    case 4:
	iarray = (int *) array ;
	for( i=0;i<number_elements;i++){
	    sum = (double) iarray[i] ;
	    sum2 += sum * sum ;
	}
	break ;
    case 8:
	darray = (double *) array ;
	for( i=0;i<number_elements;i++){
	    sum = (double) darray[i] ;
	    sum2 += sum * sum ;
	}
	break ;
    default: printf( "ERROR:Unsupported element size:%d\n",
	size_element ) ;

    }
    if( number_elements > 1 ){
	var = (sum2 - number_elements * mean * mean)/(number_elements-1) ;
	return(ABS(var));
    } else if( number_elements == 1 ){
	return( 0.0 ) ;
    } else {
	printf( "ERROR:number of elements zero\n" ) ;
	return( 0.0 ) ;
    }

} /* end Ystat_variance */
