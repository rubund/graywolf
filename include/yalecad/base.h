/* ----------------------------------------------------------------- 
FILE:	    base.h                                       
CONTENTS:   basic macro definitions.
DATE:	    Jan 29, 1988 
	    
REVISIONS:  Feb 25, 1989 - modified round macro so results are more
		consistent.  Changed rand macro to acm_random routine.	
	    Mar 20, 1989 - added DBL_MAX and DBL_MIN for foreign systems.
	    Apr  2, 1990 - added NIL definition.
	    Apr 21, 1990 - added RANDOM for SGGR.  Need extra paren.
	    May  2, 1990 - moved sccsID to unique name.  Added Mac contional compile.
	    Oct  5, 1990 - added prototype macros.
	    Oct 10, 1990 - fixed incorrect round statement.
	    Oct 22, 1990 - added prototyping of memory manager calls.
	    Oct 24, 1990 - added prototypes for void.
	    Sun Nov 11 12:42:14 EST 1990 - fixed problem with void on sun.
	    Tue Jan 15 01:27:47 EST 1991 - added vector prototypes.
	    Fri Jan 18 18:46:02 PST 1991 - added undefs for TRUE and FALSE.
	    Thu Jan 24 20:15:03 PST 1991 - added more vector routines.
	    Mon Jan 28 01:34:49 EST 1991 - added P15
	    Wed Feb  6 23:33:12 EST 1991 - modified VOIDPTR to be
		of type INT *.
	    Fri Mar 22 15:13:51 CST 1991 - added SHORT_LONG typedef
		for large designs.
	    Sun Apr 28 13:46:49 EDT 1991 - added YALLOCATE, and
		YALLOCATEC macros.
	    Mon Sep 16 22:17:20 EDT 1991 - modified for R6000.
	    Wed Nov 20 11:25:58 EST 1991 - added new memory functions.
	    Sun Dec 15 02:43:53 EST 1991 - added MEM_DEBUG conditional
		compile declarations.
	    Sat Feb  8 21:51:39 EST 1992 - added SWAP macro.
	    Tue Mar  3 16:03:45 EST 1992 - moved memory definitions to own
		file.
----------------------------------------------------------------- */
#ifndef YBASE_H
#define YBASE_H

#ifndef lint
static char Ybase_HId[] = "@(#) base.h version 1.34 3/5/92" ;
#endif


/* Take care of prototyping first, so system file can include it */
#ifdef YALE

#define PROTOTYPES_OK   1
/* WE wish this would work ...
     #ifndef __STDC__
     #define __STDC__   1
     #endif __STDC__ 
*/
#endif /* YALE */

#ifndef STDIO_H
#define STDIO_H
#include <stdio.h> 
#ifdef linux
#include <stdlib.h>	/* needed for atof() */
#endif
#endif

#ifndef MATH_H
#define MATH_H
#include <math.h> 
#endif



/* -------------------------------------------------------
    Use typedefs to make the code more portable.  User 
    just changes size here and all of the code will be 
    modified.  To avoid complaints in lint use defines 
    instead of typedefs. Typedefs are better for 
    errorchecking by the compiler.
---------------------------------------------------------- */
#ifndef lint

/* Somewhat more rigorous 64-bit compatibility added by Tim, May 2, 2011 */
#ifndef SIZEOF_VOID_P
#define SIZEOF_VOID_P 32
#endif

#if SIZEOF_VOID_P == 32
typedef int INT ;
#elif SIZEOF_VOID_P == 64
typedef long INT ;
#else
ERROR: Cannot compile without knowing the size of a pointer.  See Ylib/include/base.h
#endif

typedef int  TBOOL ;
typedef int  BOOL ;
typedef unsigned long UNSIGNED_INT ;
typedef short SHORT ;
typedef long LONG ;
typedef float FLOAT ;
/* typedef double to have ability to change to float */
/* some machines float will be natural size.            */
typedef double DOUBLE ;
/* this allows the user to pick longs if large designs are present */
#ifdef S_LONG
typedef short SHORT_LONG ;
#else /* S_LONG */
typedef long SHORT_LONG ;
#endif

#else  /* the lint case */

#define TBOOL int
#define BOOL  int
#define INT   int
#define UNSIGNED_INT unsigned int
#define SHORT short
#define FLOAT float
#define DOUBLE double

#endif /* lint */

/* used to find when we didn't use the PORTABLE version */
#ifdef FIND_GARBAGE
#define int garbage
#define short garbage
#define float garbage
#define double garbage
#endif /* FIND_GARBAGE */

#if defined(THINK_C)	/* Mac */

#include <yalecad/mac.h>

#elif defined(linux) || defined(ultrix)

#ifndef	_LIMITS_H_
#include <limits.h>
#endif  /* _LIMITS_H_ */

#ifndef DBL_MIN
#define DBL_MIN -1.0E38
#endif /* test on DBL_MIN */

#ifndef DBL_MAX
#define DBL_MAX 1.0E38
#endif /* test on DBL_MAX */

#else /* not ultrix or linux; i.e., doesn't have limits.h (?) */

/* for foreign machines - conservative numbers */
#ifndef R6000
#undef SHRT_MIN /* -32768 	*/
#undef SHRT_MAX /* 32767	*/
#undef INT_MIN /* -2147483648	*/
#undef INT_MAX /*  2147483647	*/
#undef DBL_MAX /*  1.0E38	*/
#undef DBL_MIN /* -1.0E38	*/

#define SHRT_MIN -32768
#define SHRT_MAX 32767
#define INT_MAX  2147483647
#define INT_MIN -2147483648
#define INT_MAX  2147483647
#define DBL_MAX  1.0E38
#define DBL_MIN -1.0E38
#endif /* R6000 */

#endif /* ultrix, linux */

#if defined(THINK_C) || defined(linux)
#define PROTOTYPES_OK
#endif /* Mac, linux */

#ifdef PROTOTYPES_OK
#define  P1( a )                        a
#define  P2( a,b )                      a,b
#define  P3( a,b,c )                    a,b,c
#define  P4( a,b,c,d )                  a,b,c,d
#define  P5( a,b,c,d,e )                a,b,c,d,e
#define  P6( a,b,c,d,e,f )              a,b,c,d,e,f
#define  P7( a,b,c,d,e,f,g )            a,b,c,d,e,f,g
#define  P8( a,b,c,d,e,f,g,h )          a,b,c,d,e,f,g,h
#define  P9( a,b,c,d,e,f,g,h,i )        a,b,c,d,e,f,g,h,i
#define P10( a,b,c,d,e,f,g,h,i,j  )     a,b,c,d,e,f,g,h,i,j
#define P15( a,b,c,d,e,f,g,h,i,j,k,l,m,n,o) \
				a,b,c,d,e,f,g,h,i,j,k,l,m,n,o
typedef void *VOIDPTR ;
typedef void VOID ;

#else  /* PROTOTYPES don't exist */
#define  P1( a )                  
#define  P2( a,b )               
#define  P3( a,b,c )            
#define  P4( a,b,c,d )         
#define  P5( a,b,c,d,e )      
#define  P6( a,b,c,d,e,f )   
#define  P7( a,b,c,d,e,f,g )     
#define  P8( a,b,c,d,e,f,g,h )    
#define  P9( a,b,c,d,e,f,g,h,i )
#define P10( a,b,c,d,e,f,g,h,i,j  )
#define P15( a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)
typedef INT *VOIDPTR ;
#define VOID INT

#endif /* end PROTOTYPE_OK */


#ifdef VMS
#define PGMOK 1
#define PGMFAIL 0
#else
#define PGMOK 0
#define PGMFAIL -1
#endif


/* BASIC MACRO DEFINITIONS  */
#undef  TRUE
#undef  FALSE

#define YES   1
#define NO    0
#define TRUE  (TBOOL) 1
#define ACCEPT   1
#define FALSE (TBOOL) 0
#define REJECT   0
#define LRECL 256  /* length of screen buffer */
#define STRINGEQ 0
#define EOS  '\0'
#define ZERO     0

/* ************* USEFUL MACRO DEFINITIONS **************** */
#define ABS(value)      ( (value)>=0 ? (value) : -(value) ) 
#define MAX(a,b)        ( (a) < (b) ? (b) : (a) )
#define MIN(a,b)        ( (a) > (b) ? (b) : (a) )
#define SWAP(a,b,type_t)  { type_t temp ; temp = a ; a = b ; b = temp ; } 
/* random number [0...INT_MAX] */
#define RAND            (Yacm_random() )
/* random number [0...limit] */
#define RANDOM(limit)   (Yacm_random() % (limit))
#define ROLL_THE_DICE() ((DOUBLE) RAND / (DOUBLE)0x7fffffff ) 
#define LINE printf(" file : %s  line # %d\n", __FILE__,__LINE__);
#define ROUND(value)  ( (INT)(value + 0.5)) 

/* Always include memory defintions */
#include <yalecad/okmalloc.h>

#endif /* YBASE_H */
