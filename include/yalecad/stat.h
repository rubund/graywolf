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

#ifndef lint
static char SccsYstat_H[] = "@(#) stat.h (Yale) version 1.1 4/21/91" ;
#endif

/* 
Function:
Given an array of number_elements of size_element, calculate min
*/
extern DOUBLE Ystat_min( P3( VOIDPTR array, INT number_elements,
			     INT size_element ) ) ;

/* 
Function:
Given an array of number_elements of size size_element,calculate max
*/
extern DOUBLE Ystat_max( P3( VOIDPTR array, INT number_elements,
			     int size_element ) ) ;

/* 
Function:
Given an array of number_elements of size size_ele, calculate mean
*/
extern DOUBLE Ystat_mean( P3( VOIDPTR array, INT number_elements,
			     INT size_element ) ) ;

/* 
Function:
Given an array of number_elements of size size_ele, calculate variance.
NOTE also need to give mean calculated from above
*/
extern DOUBLE Ystat_var( P4(VOIDPTR array,INT number_elements,
			    INT  size_element, DOUBLE mean ) ) ;
#endif /* YSTAT_H */
