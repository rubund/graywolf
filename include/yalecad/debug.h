/* ----------------------------------------------------------------- 
FILE:	    debug.h                                       
CONTENTS:   debug macros
DATE:	    May 04, 1988 
	    static char SccsId[] = "@(#) debug.h version 1.14 3/23/92" ;
REVISIONS:  Jan 24, 1989 - added selective turnoff of print debug
		or assertions.
	    Jan 29, 1989 - made output nicer.
	    Apr 10, 1990 - added new debug macros and updated for
		the debug utilities.
	    Apr 29, 1990 - fixed cleanup handler problem.  Also
		modified ASSERTs to call YdebugAssert instead of
		Ydebug so that ASSERTs are always on.
	    Sep  9, 1990 - added DS macro - DEBUG source macro
		that is compiled when DEBUG is true. This is good
		for definitions etc.
	    Thu Mar  5 03:38:04 EST 1992 - better output messages
		at the expense of not allow multiple line test without
		a \ continuation character.
----------------------------------------------------------------- */
#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG

#include <yalecad/message.h>

/* ---------------------------------------------------------------
   The assertions differ from the D macro in that the test is
   always executed in an assertion whereas the function part of
   the D macro is executed depending whether the given routine name
   has been turned on in the dbg file.
   NOTE NOW test_xc ***** MUST BE ON THE SAME LINE ******* or a
   compile error occurs.  A alternate solution to allow multiple
   lines is the explicit enumeration of \ as a continuation character.
----------------------------------------------------------------- */

#define D(name_xz, func_xz) if(Ydebug(name_xz)) { func_xz ; }
#define DS(name_xz)  { name_xz }

/* ***************** ASSERT MACROS ********************************/
/* NORMAL ASSERT - if not true, output message */
#define ASSERT(test_xz,routine_xz,userMsg_xz) \
    {if (YdebugAssert()) {\
	if (!(test_xz)) {\
	    char assertMsg_xz[LRECL] ; \
	    sprintf( assertMsg_xz,\
		"Assertion failed in file %s, line %d:\n\t",\
		__FILE__, __LINE__ ) ;\
	    M(ERRMSG,routine_xz,assertMsg_xz ) ;\
	    sprintf( assertMsg_xz, "%s\n", userMsg_xz ) ;\
	    M(MSG,NULL,assertMsg_xz) ;\
    }}}

/* ASSERT and if not true break */
#define ASSERTNBREAK(test_xz,routine_xz,userMsg_xz) \
    {if (YdebugAssert()) {\
	if (!(test_xz)) {\
	    char assertMsg_xz[LRECL] ; \
	    sprintf( assertMsg_xz,\
		"Assertion failed in file %s, line %d:\n\t",\
		__FILE__, __LINE__ ) ;\
	    M(ERRMSG,routine_xz,assertMsg_xz ) ;\
	    sprintf( assertMsg_xz, "%s\n", userMsg_xz ) ;\
	    M(MSG,NULL,assertMsg_xz) ;\
	    break ; \
    }}}

/* ASSERT and if not true continue */
#define ASSERTNCONT(test_xz,routine_xz,userMsg_xz) \
    {if (YdebugAssert()) {\
	if (!(test_xz)) {\
	    char assertMsg_xz[LRECL] ; \
	    sprintf( assertMsg_xz,\
		"Assertion failed in file %s, line %d:\n\t",\
		__FILE__, __LINE__ ) ;\
	    M(ERRMSG,routine_xz,assertMsg_xz ) ;\
	    sprintf( assertMsg_xz, "%s\n", userMsg_xz ) ;\
	    M(MSG,NULL,assertMsg_xz) ;\
	    continue ; \
    }}}


/* ASSERT and if not true return */
#define ASSERTNRETURN(test_xz,routine_xz,userMsg_xz) \
    {if (YdebugAssert()) {\
	if (!(test_xz)) {\
	    char assertMsg_xz[LRECL] ; \
	    sprintf( assertMsg_xz,\
		"Assertion failed in file %s, line %d:\n\t",\
		__FILE__, __LINE__ ) ;\
	    M(ERRMSG,routine_xz,assertMsg_xz ) ;\
	    sprintf( assertMsg_xz, "%s\n", userMsg_xz ) ;\
	    M(MSG,NULL,assertMsg_xz) ;\
	    return ; \
    }}}

#define ASSERTNRETURN2(test_xz,routine_xz,userMsg_xz) \
    {if (YdebugAssert()) {\
	if (!(test_xz)) {\
	    char assertMsg_xz[LRECL] ; \
	    sprintf( assertMsg_xz,\
		"Assertion failed in file %s, line %d:\n\t",\
		__FILE__, __LINE__ ) ;\
	    M(ERRMSG,routine_xz,assertMsg_xz ) ;\
	    sprintf( assertMsg_xz, "%s\n", userMsg_xz ) ;\
	    M(MSG,NULL,assertMsg_xz) ;\
	    return(NULL) ; \
    }}}



/* ASSERT and if not true exit program thru cleanup handler */
#define ASSERTNFAULT(test_xz,routine_xz,userMsg_xz) \
    {if (YdebugAssert()) {\
	if (!(test_xz)) {\
	    char assertMsg_xz[LRECL] ; \
	    sprintf( assertMsg_xz,\
		"Assertion failed in file %s, line %d:\n\t",\
		__FILE__, __LINE__ ) ;\
	    M(ERRMSG,routine_xz,assertMsg_xz ) ;\
	    sprintf( assertMsg_xz, "%s\n", userMsg_xz ) ;\
	    M(MSG,NULL,assertMsg_xz) ;\
	    YcleanupHandler( -1, NULL ) ; \
    }}}


/* ASSERT and ask user if he wishes to continue otherwise abort */
#define ASSERTNQUERY(test_xz,routine_xz,userMsg_xz) \
    {if (YdebugAssert()) {\
	if (!(test_xz)) {\
	    char assertMsg_xz[LRECL] ; \
	    sprintf( assertMsg_xz,\
		"Assertion failed in file %s, line %d:\n\t",\
		__FILE__, __LINE__ ) ;\
	    M(ERRMSG,routine_xz,assertMsg_xz ) ;\
	    sprintf( assertMsg_xz, "%s\n", userMsg_xz ) ;\
	    M(MSG,NULL,assertMsg_xz) ;\
	    M(MSG,NULL,"Do you wish to continue?") ;\
	    scanf( "%s", assertMsg_xz );\
	    if( assertMsg_xz[0] != 'Y' && assertMsg_xz[0] != 'y' ){\
		YexitPgm( PGMFAIL ) ; \
    }}}}

/* ASSERT and if not true execute the function */
#define ASSERTNFUNC(test_xz,routine_xz,function_xz) \
    {if (YdebugAssert()) {\
	if (!(test_xz)) {\
	    { function_xz \
    }}}}

/* ************** function prototypes ************** */
extern BOOL Ydebug( P1(char *routine) ) ;
/* 
Function:
    Return TRUE if the given routine has been turned on for debug.
    It returns FALSE otherwise.
*/

/* ASSERTIONS are always on */
extern BOOL YdebugAssert() ;
/* 
Function:
    Returns TRUE if debug is on.  It returns FALSE otherwise.
*/

extern YdebugWrite() ;
/* 
Function:
    Write the debug data structure to a file.
*/

extern YsetDebug( P1(BOOL flag ) ) ;
/* 
Function:
    Turn the debug functions on or off.  It will cause the evaluation
    of the dbg file in the current working directory.
*/

#else  /* remove debug code from source */

#define ASSERT(a_xz,b_xz,c_xz) 
#define ASSERTNBREAK(a_xz,b_xz,c_xz) 
#define ASSERTNCONT(a_xz,b_xz,c_xz) 
#define ASSERTNRETURN(a_xz,b_xz,c_xz) 
#define ASSERTNFAULT(a_xz,b_xz,c_xz) 
#define ASSERTNQUERY(a_xz,b_xz,c_xz) 
#define ASSERTNFUNC(a_xz,b_xz,c_xz) 
#define D(x_xz,func_xz)      
#define DS(name_xz)

#endif  /* DEBUG */

/* now selectively delete either ASSERTIONS or PRINT */
#ifdef TURNOFFPRINTD

#undef  D      
#define D(x_xz,func_xz)      
#undef  DS      
#define DS(name_xz)

#endif  /* end TURNOFFPTRINTD */

#ifdef TURNOFFASSERT

#undef ASSERT
#undef ASSERTNBREAK
#undef ASSERTNCONT
#undef ASSERTNRETURN
#undef ASSERTNFAULT
#undef ASSERTNQUERY
#undef ASSERTNFUNC

#define ASSERT(a_xz,b_xz,c_xz) 
#define ASSERTNBREAK(a_xz,b_xz,c_xz) 
#define ASSERTNCONT(a_xz,b_xz,c_xz) 
#define ASSERTNRETURN(a_xz,b_xz,c_xz) 
#define ASSERTNFAULT(a_xz,b_xz,c_xz) 
#define ASSERTNQUERY(a_xz,b_xz,c_xz) 
#define ASSERTNFUNC(a_xz,b_xz,c_xz) 

#endif /* end TURNOFFASSERT */

#endif /* end DEBUG_H */
