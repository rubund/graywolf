/* ----------------------------------------------------------------- 
"@(#) cleanup.h (Yale) version 1.6 11/2/91"
FILE:	    cleanup.h                                       
DESCRIPTION:This file contains include file for cleanup handler 
	    routines.  To use cleanup handler included '<signal.h>'
	    at the top of main.c and #define CLEANUP_C either by
	    uncommenting the define in this file or by setting the
	    variable in the makefile: -DCLEANUP_H.  In addition, 
	    the first executable line of the program must be 
	    INITCLEANUP() macro.  In order for the handler to work, 
	    the INITCLEANUP call must occur in the top level routine,
	    ie., main.c .
CONTENTS:   MACRO DEFINITIONS
DATE:	    Feb  2, 1988 
REVISIONS:  Mar  9, 1989 - modified conditional compiles so you can
		avoid compilation on foreign machines.
----------------------------------------------------------------- */
/* cleanup.h definitions for cleanup handler system */

/* makefile sets this variable automatically but you can manually */
/* define it here if you want and your system supports UNIX signals */
/* #define CLEANUP_C - we want a cleanup handler. */

#ifndef CLEANUP_H
#define CLEANUP_H

#define NODUMP 0
#define YESDUMP 1
#define MAYBEDUMP 2

#ifdef CLEANUP_C
#include <signal.h>
#include <yalecad/base.h>

#ifdef linux
extern void Ycleanup(int);
#else
extern void Ycleanup(int, int, struct sigcontext *);
#endif

/* initialization cleanup macro */
/* first  argument - argv[0] - program name */
/* second argument - function - user function that will be executed upon
   fault */
/* third argument - dumpFlag - allows users to control core dump */
/*    signal(SIGQUIT,cleanup); - remove due to dbx bug */ 
/*    signal(SIGINT,cleanup); - remove due to dbx bug */ 
#define YINITCLEANUP( argv, function, dumpFlag ) \
{                                               \
    signal(SIGHUP,Ycleanup); \
    signal(SIGILL,Ycleanup);\
    signal(SIGFPE,Ycleanup);\
    signal(SIGSEGV,Ycleanup);\
    signal(SIGBUS,Ycleanup);\
    signal(SIGSYS,Ycleanup);\
    signal(SIGTERM,Ycleanup);\
    signal(SIGUSR1,Ycleanup);\
    YinitCleanup( argv, function, dumpFlag ) ; \
}

/*---------------------------------------------------------
   initCleanup - sets static variables for cleanup handler.
  --------------------------------------------------------*/
VOID YinitCleanup( P3(char *argv, BOOL (*function)(), int dump) );

#else

/* no cleanup handler - null it out */
#define YINITCLEANUP( argv, function, dumpFlag )

#endif /* CLEANUP_C */
#endif /* CLEANUP_H */
