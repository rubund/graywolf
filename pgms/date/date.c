/* ----------------------------------------------------------------- 
FILE:	    date.c                                       
DESCRIPTION:Compile date stamp routine.  This simple function stores
the compile date of the program.
DATE:	    Jan 29, 1988 
REVISIONS:  May  4, 1990 - now use static so we can look at compile
		date in object code.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) date.c version 1.2 5/12/90" ;
#endif

#include <date.h>

static char compileDate[] = DATE ;
char *getCompileDate()
{
    return( &(compileDate[27]) ) ;
} /* end getCompileDate */
