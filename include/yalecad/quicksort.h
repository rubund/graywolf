/* ----------------------------------------------------------------- 
FILE:	    quicksort.h                                       
CONTENTS:   quicksort definitions.
DATE:	    Tue Mar  3 16:01:25 EST 1992
----------------------------------------------------------------- */
#ifndef YQUICKSORT_H
#define YQUICKSORT_H

#ifndef lint
static char Yquicksort_HId[] = "@(#) quicksort.h version 1.1 3/5/92" ;
#endif

extern VOID Yquicksort( P4(VOIDPTR base, INT n, INT size, INT (*compare)() ) ) ;

#endif /* YQUICKSORT_H */
