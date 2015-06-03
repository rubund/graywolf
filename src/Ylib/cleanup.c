/*
 *   Copyright (C) 1988-1991 Yale University
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
FILE:	    cleanup.c                                       
DESCRIPTION:This file contains cleanup handler routines. 
	    To use cleanup handler included '<signal.h>'
	    at the top of main.c .  In addition, the first executable
	    line of the program must be INITCLEANUP() macro.  For
	    everything to work the compiler directive MEMORYMAN must
	    be defined.  In order for the handler to work, the
	    INITCLEANUP macro call (see cleanup.h) must occur 
	    in the top level routine, ie., main.c .  The user may supply
	    a parameterless function to be executed upon the
	    completion of the handler; otherwise, pass a null pointer.
CONTENTS:   
	    initCleanup( argv, function, dump )
		char *argv ;
		INT  (*function)() ;
		BOOL dump ;
	    cleanup(sigNum, code, scp )
		INT sigNum ;
		INT code ;
		struct sigcontext *scp ;
	    void YcleanupHandler(status)
		int status ;

DATE:	    Feb  2, 1988 
REVISIONS:  Sep 25, 1988 - converted to common utility.
	    Jan 18, 1988 - added SIGKILL to list on user terminated
		conditions.
	    Mar 20, 1989 - removed call to finderror. User should
		just call it from cshell.
	    May  3, 1989 - added Yprefix.
	    Apr 29, 1990 - fixed missing Yprefix on YcleanupHandler
	    Fri Jan 18 18:38:36 PST 1991 - fixed to run on AIX.
	    Fri Jan 25 16:16:50 PST 1991 - fixed to run on HPUX.
	    Mon Sep 16 22:20:09 EDT 1991 - fixed to run on R6000.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) cleanup.c version 3.13 11/2/91" ;
#endif

#include <yalecad/cleanup.h>

/* conditional compile switch is set in cleanup.h */
#ifdef CLEANUP_C

#include <stdio.h>
#include <yalecad/base.h>

#ifdef R6000
#include <sys/types.h>
#include <sys/context.h>
#include <sys/signal.h>
#endif /* R6000 */


static INT dumpFlag ;
static char programPath[LRECL] ;
static BOOL  (*userFunction)() ;


/* ***************************************************************** 
   initCleanup - sets static variables for cleanup handler.
*/
VOID YinitCleanup( argv, function, dump )
char *argv ;
BOOL  (*function)() ;
int dump ;
{
    sprintf( programPath, "%s", argv ) ;
    userFunction = function ;
    dumpFlag = dump ;
}

/* ***************************************************************** 
   cleanup - the installed cleanup handler.
*/
#ifdef linux	/* maybe others? */

void
Ycleanup(int sigNum)

#else

Ycleanup(sigNum, code, scp )
int sigNum ;
int code ;
struct sigcontext *scp ;

#endif /* linux */

{
    if( sigNum != SIGINT && sigNum != SIGQUIT && sigNum != SIGKILL ){
	fprintf(stderr,
	    "\nSystem has detected an error!\n") ;
#ifdef apollo
	{
	    INT pid ;
	    char command[LRECL] ;

	    pid = getpid() ;
	    sprintf( command, "/com/tb %d", pid ) ;
	    if(Ysystem("traceback", FALSE, command, NULL )){
	       fprintf(stderr,"ERROR[cleanup]:could not perform traceback.\n");
	    }
	}
#endif
#ifdef UNIX
#ifndef SYS5
#ifndef mips
#ifdef linux
	fprintf(stderr, "Stack pointer unknown\n");
#else
	fprintf(stderr,"Stack pointer :%0x\n", scp->sc_sp ) ;
#endif
#endif
#ifndef AIX
#ifdef linux
	fprintf(stderr, "Program counter unknown\n");
#else
	fprintf(stderr,"Program counter:%0x\n", scp->sc_pc ) ;
#endif
#else /* AIX */
#ifdef R6000
	fprintf(stderr,"Instruction Address Register :%0x\n", scp->sc_jmpbuf.jmp_context.iar ) ; 
#else /* R6000 */
	fprintf(stderr,"Stack pointer :%0x\n", scp->sc_psw ) ;
#endif /* R6000 */
#endif /* AIX */
#endif /* SYS5 */
#endif /* UNIX */
    }
    YcleanupHandler(sigNum) ;

}

/* ***************************************************************** 
   YcleanupHandler - after system work process user information.
*/
YcleanupHandler(status)
INT status ;
{

    char responseBuf[LRECL], *response = responseBuf ;

    if( status == SIGINT || status == SIGQUIT || status == SIGKILL ){
	fprintf(stderr,"\nProgram terminated by user\n\n") ;
    } else if( status == SIGUSR1 ){ 
	Ypmemerror( "ERROR[memory manager]" ) ;

    } else {
#ifndef SYS5
	psignal( status, "ERROR[cleanup handler]" ) ;
#endif
    }

    if( userFunction ){
	(*userFunction)() ;
    }
    if( dumpFlag == MAYBEDUMP ){
	fprintf(stderr,"Enter y for core dump.  Default no dump\n") ;
	scanf( "%s", response ) ;
	if( *response == 'y' || *response == 'Y' ){
	    dumpFlag = YESDUMP ;
	}
    }

    if( dumpFlag == YESDUMP ){
	fprintf(stderr,"Generating core dump for user traceback...\n") ;
	fprintf(stderr,
	    "Use dbx to find where program core dumped...\n\n") ;
	fflush(stdout) ;
	signal(SIGILL,SIG_DFL);
	abort() ;
	exit(1) ;
    } else {
	exit(1) ;
    }

}
#endif /* CLEANUP_H */
