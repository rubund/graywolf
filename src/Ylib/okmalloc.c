/*
 *   Copyright (C) 1988-1992 Yale University
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
FILE:	    okmalloc.c                                       
DESCRIPTION:This file contains memory management routines. 
	    User should always call safe_malloc, safe_free, etc.
	    A conditional compile allows the choice between this
	    memory manager and system.  Using the memory manager
	    helps make the environment similar on all machines.
CONTENTS:   
	    ++++++++++++ HEAP MANAGEMENT ROUTINES ++++++++++++++
	    static status_t  heapDispose (ptr)
		char_p ptr;
	    static status_t  allocateRegion (min_size)
		INT min_size;
	    static status_t  heapInit (initial_size)
		INT initial_size;
	    static status_t  heapNew (ptr, req_size)
		char_p  *ptr;
		INT   req_size;
	    +++++++++ END HEAP MANAGEMENT ROUTINES ++++++++++++ 
	      USER  CALLS FOR C RUN TIME LIBRARY 
	    VOID Ysafe_free(ptr)
		char *ptr;
	    VOID Ysafe_cfree(ptr)
		char *ptr;
	    char *Ysafe_malloc(bytes)
		long bytes;
	    char *Ysafe_calloc(num_entries, bytes)
		long num_entries;
		long bytes;
	    char *Ysafe_realloc(ptr, bytes)
		char *ptr;
		long bytes;
	    INT YgetCurMemUse()
	    INT YcheckMemObj(ptr)
		char *ptr ;
	    INT Yinit_memsize(memsize)
		INT memsize ;
	    INT YgetListSize(ptr, offsetPtr)
		char *ptr ;     
		char *offsetPtr ;
	    YdebugMemory( flag )
		BOOL flag ;
	    Ypmemerror( s )
		char *s ;
	    YcheckDebug( where )
		INT where ; 
	    char *Yvector_alloc( lo, hi, size )
		INT size, lo, hi ;
	    char *Yvector_calloc( lo, hi, size )
		INT size, lo, hi ;
	    char *Yvector_realloc( array_orig, lo, hi, size )
	    VOIDPTR array_orig ;
		INT size, lo, hi ;
	    VOID Yvector_free( array, lo, size )
		VOIDPTR array ;
		INT lo, size ;
DATE:	    Feb  2, 1988 
REVISIONS:  Sep 25, 1988 - converted to common utility.
	    Feb 22, 1989 - added new memory check for debugger.
		    made local function calls static - hide from linker.
	    Apr 18, 1989 - added Yinit_memsize for init memory size.
	    May  3, 1989 - added Y prefixes.
	    Sep 16, 1989 - all debug directed to stderr.
	    Dec 21, 1989 - now initialize freed memory to -1 to
		catch more bugs.
	    Jan 31, 1990 - corrected arg reversal in calloc call.
	    Feb  4, 1990 - fixed bug in initializing freed memory.
		Also renamed static variables.
	    Tue Oct 23 03:33:19 EDT 1990 - fixed for prototypes in
		base.h.
	    Tue Jan 15 02:06:35 EST 1991 - added vector allocation
		routines.
	    Thu Jan 24 20:19:46 PST 1991 - added more vector routines.
	    Thu Jan 31 15:42:52 EST 1991 - added char * cast to 
		voidptr use in vector routines.
	    Fri Feb 15 15:37:16 EST 1991 - added call to message
		system so that silent mode will work.
	    Sat May 11 22:53:49 EDT 1991 - added a conditional compile
		for HPUX.
	    Sat Dec 14 14:33:10 EST 1991 - added YgetMaxMemUse()
	    Sun Dec 15 02:30:59 EST 1991 - added the MEM_DEBUG conditional
		compile.  We added extra arguments to the alloc routines
		so that we could pass the line that the memory was
		created.  We now can dump all the allocated memory by
		calling Ydump_mem().
	    Tue Jan  7 18:03:31 EST 1992 - fixed memory manager
		on the MAC.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) okmalloc.c (Yale) version 3.24 3/6/92" ;
#endif

#include	<stdio.h>
#include	<unistd.h>
#include	<signal.h>
#include	<errno.h>
#include <yalecad/base.h>
#include <yalecad/message.h>

/* memory errors */
#define  heap_error_base   0x40060000
#define  heap_bad_block    (heap_error_base + 1)
#define  heap_no_mem       (heap_error_base + 2)
#define  heap_ok          0

#ifdef MEM_DEBUG
#include        	<yalecad/file.h>
/* -----------------------------------------------------------------
   These two unusual definitions MEM_DEBUG1 and MEM_DEBUG2 add 
   arguments to memory user routines so that we can pass the file
   and line number from the preprocessor.
----------------------------------------------------------------- */
#define MEM_DEBUG1	,file,line
#define MEM_DEBUG2      char *file ; int line ;
#define ALLOC_NAME_SIZE 40
#else
#define MEM_DEBUG1
#define MEM_DEBUG2
#endif /* MEM_DEBUG */

/*
  *--------------------------------------------------------------
  * BEGIN NORMAL C MEMORY MANAGEMENT SYSTEM
  *--------------------------------------------------------------
*/

/* use standard calls to malloc, calloc, etc */

void Ysafe_free(VOIDPTR ptr);
void Ysafe_cfree(VOIDPTR ptr);

char *Ysafe_malloc(int size)
{
	char *p;
	if (!(p = malloc(size))) {
		errno = heap_no_mem ;
// 		kill(getpid(),SIGUSR1);
	}
	return p;
}

char *Ysafe_realloc(VOIDPTR obj, int size)
{
	char *p;
	if (!(p = realloc(obj, size))) {
		errno = heap_no_mem ;
	//         kill(getpid(),SIGUSR1);
	}
	return p;
}

char *Ysafe_calloc(int num, int size)
{
	char *p;
	if (!(p = calloc(num,size))) {
		errno = heap_no_mem ;
// 		kill(getpid(),SIGUSR1);
	}
	return p;
}
/* when not testing memory just call system free */
void Ysafe_free(VOIDPTR ptr)
{
// 	free(ptr);
	return;
}

void Ysafe_cfree(VOIDPTR ptr)
{
// 	cfree(ptr);
	return;
}

/* ***********DUMMY ROUTINES TO RESOLVE GLOBALS **************** */
/*
 * TODO: Fix this!
 */
int YgetCurMemUse()
{
	return(0) ;
}

int YgetMaxMemUse()
{
	return(0) ;
}

int YcheckMemObj(char *ptr)
{
	return(0) ;
}

int YgetListSize(char *ptr, char *offsetPtr)
/* pointer to beginning of list structure */
/* pointer to "next" field within structure */
{
	return(0) ;
}

void YdebugMemory( int flag )
{
	return ;
}

int YcheckDebug( VOIDPTR where )
{
	return ( INT_MAX ) ;
} /* end checkDebug */

void Yinit_memsize( int memsize )
{
	return ;
} /* end Yinit_memsize */

void Ydump_mem()
{
} /* end Ydump_mem() */
/*
  *--------------------------------------------------------------
  * END NORMAL C MEMORY MANAGEMENT SYSTEM
  *--------------------------------------------------------------
*/

/* print memory error in the same style as perror and psignal */
void Ypmemerror( char *s )
{
	/* first print user message if available */
	if( s ){
		printf( "%s:", s ) ;
	}
	switch(errno){
		case heap_ok:
		printf(
		"Memory ok - Problem in memory management logic.\n" ) ;
		break; 
		case heap_bad_block:
		printf(
		"Memory block was found to be corrupted.\n" ) ;
		break; 
		case heap_no_mem:
		printf(
		"No memory available to allocate.\n" ) ;
		break; 
		default:
		printf(
		"Error = %0x Unrecognized error code.\n",errno ) ;
	}
} /* end Ypmemerror */

/* *******  memory convenience functions  ******* */
/* ALLOCATE an array [lo..hi] of the given size not initialized */
char *Yvector_alloc( int lo, int hi, int size MEM_DEBUG1 )
{
	char *array_return ;

	array_return = (char *) Ysafe_malloc((unsigned) (hi-lo+1)*size MEM_DEBUG1 ) ;
	if( array_return ){
		return( array_return - size * lo ) ;
	}
	return( NIL(char *) ) ;

} /* end Yvector_alloc */

/* ALLOCATE an array [lo..hi] of the given size initialized to zero */
char *Yvector_calloc( int lo, int hi, int size MEM_DEBUG1 )
{
	char *array_return ;
	array_return = (char *) Ysafe_calloc((unsigned) (hi-lo+1),size MEM_DEBUG1 ) ;
	if( array_return ){
		return( array_return - size * lo ) ;
	}
	return( NIL(char *) ) ;
} /* end Yvector_calloc */

/* REALLOCATE an array [lo..hi] of the given size no initialization */
char *Yvector_realloc( array_orig, lo, hi, size MEM_DEBUG1 )
VOIDPTR array_orig ;
INT size, lo, hi ;
MEM_DEBUG2
{
    char *adj_array ;          /* put back the offset */
    char *array_return ;       /* the new offset */

    adj_array = ((char *) array_orig) + lo * size ;
    array_return = (char *) 
	Ysafe_realloc( adj_array, (unsigned) (hi-lo+1)*size MEM_DEBUG1 ) ;
    if( array_return ){
	return( array_return - size * lo ) ;
    }
    return( NIL(char *) ) ;

} /* end Yvector_realloc */

VOID Yvector_free( array, lo, size MEM_DEBUG1 )
VOIDPTR array ;
INT lo, size ;
MEM_DEBUG2
{
    Ysafe_free( ((char *)array) + lo * size MEM_DEBUG1 ) ;
} /* end Yvector_free */

/* ************************* TEST ROUTINES ******************************** */
#ifdef TEST 

typedef struct {
    INT bogus_dude ;
    DOUBLE narly ;
    char *awesome ;
} MEMDATA, *MEMDATAPTR ;

main()
{
    MEMDATAPTR array ;
    MEMDATAPTR vector ;
    char *string, *Ystrclone() ;

    YdebugMemory( TRUE ) ;
    Yinit_memsize( 1024 ) ;

    /* allocate an array 0..9 */
    array = YMALLOC( 10, MEMDATA ) ;
    printf( "Memory size :%d\n", YgetCurMemUse() ) ;
    /* allocate an array 1..10 */
    vector = YVECTOR_MALLOC( 1, 10, MEMDATA ) ;
    printf( "Memory size :%d\n", YgetCurMemUse() ) ;
    /* clone this string */
    string = Ystrclone( "Droog" ) ;
    printf( "Memory size :%d\n", YgetCurMemUse() ) ;
    /* look at mem.data at this point */
    Ydump_mem() ;
    /* free all the memory */
    YFREE( array ) ;
    YFREE( string ) ;
    YVECTOR_FREE( vector, 1 ) ;
    printf( "Memory size :%d\n", YgetCurMemUse() ) ;
    printf( "Memory max size :%d\n", YgetMaxMemUse() ) ;
    Ydump_mem() ;

    exit(0) ;
    
}/* end main() */

#endif /* TEST */

