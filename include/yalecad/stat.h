/* ----------------------------------------------------------------- 
FILE:	    stat.h                                       
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
REVISIONS:  
----------------------------------------------------------------- */
#ifndef YSTAT_H
#define YSTAT_H

/* 
Function:
Given an array of number_elements of size_element, calculate min
*/
double Ystat_min( P3( VOIDPTR array, INT number_elements,
			     INT size_element ) ) ;

/* 
Function:
Given an array of number_elements of size size_element,calculate max
*/
double Ystat_max( P3( VOIDPTR array, INT number_elements,
			     int size_element ) ) ;

/* 
Function:
Given an array of number_elements of size size_ele, calculate mean
*/
double Ystat_mean( P3( VOIDPTR array, INT number_elements,
			     INT size_element ) ) ;

/* 
Function:
Given an array of number_elements of size size_ele, calculate variance.
NOTE also need to give mean calculated from above
*/
double Ystat_var( P4(VOIDPTR array,INT number_elements,
			    INT  size_element, DOUBLE mean ) ) ;

void Yprint_stats( FILE *fout );

#endif /* YSTAT_H */
