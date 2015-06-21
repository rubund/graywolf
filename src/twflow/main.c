/*
 *   Copyright (C) 1989-1991 Yale University
 *   Copyright (C) 2014 Ruben Undheim <ruben.undheim@gmail.com>
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
FILE:	    main.c
DESCRIPTION:This file contains main control routine for Master program.
CONTENTS:   main( argc , argv )
		INT argc ;
		char *argv[] ;
	    syntax()
DATE:	    May	 5, 1989 
REVISIONS:  Jun 19, 1989 - added stdcell.fnog for no graphics case.
	    Nov  2, 1989 - changed search path for files and added
		a -m macro switch.
	    Aug  1, 1990 - rewrote to be MOTIF compatible.
	    Nov 23, 1990 - now exit status is correct if a program
		experiences trouble.
	    Fri Feb 22 23:54:19 EST 1991 - added lock feature for PSC.
	    Sat Feb 23 04:52:56 EST 1991 - added parasite and tomus
		switches.
	    Thu Mar  7 01:35:22 EST 1991 - now windowID is a global
		and twflow can inherit a window itself.
	    Tue Apr  2 23:10:06 EST 1991 - fixed argument problem
		with Yfile_create_lock.
	    Thu Apr 18 01:20:52 EDT 1991 - now user can specify
		a flow directory.  Also added show_flows call.
	    Sun Apr 21 22:36:29 EDT 1991 - now find the flow directory.
---------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) main.c version 2.8 4/21/91" ;
#endif

#include <stdio.h>
#include <signal.h>
#include <yalecad/cleanup.h>
#include <yalecad/message.h>
#include <yalecad/program.h>
#include <yalecad/string.h>
#include <yalecad/file.h>
#include <yalecad/debug.h>

#define GLOBAL_DEFS
#include <globals.h>

#include "config-build.h"

#define EXPECTEDMEMORY  (256 * 1024)  /* 256k is more than enough */
#define NULLWINDOW      0
#define VERSION         "2.1" 

main( argc , argv )
INT argc ;
char *argv[] ;
{

    char        filename[LRECL] ;    /* buffer for filename */
    char        *ptr ;               /* argument pointer */
    char        *Ygetenv() ;         /* get environment variable */
    FILE        *fp ;                /* file pointer */
    FILE        *find_flow_file() ;  /* return pointer to flow file */
    BOOL        debug ;              /* TRUE if debug on */
    BOOL        general_mode ;       /* TRUE if top level user flow */
    BOOL        lock ;               /* whether to create a lock file */
    BOOL        verbose ;            /* whether to go into verbose mode.*/
    BOOL        parasite ;           /* TRUE if we are to take over win */
    INT         arg_count ;          /* parse the command line */
    INT         windowId ;           /* the parasite window id */
    VOID        yaleIntro() ;        /* give intro for program */

    /* start up cleanup handler */
    YINITCLEANUP( argv[0], NULL, MAYBEDUMP ) ;

    Yinit_memsize( EXPECTEDMEMORY ) ;

    flow_dirG = NIL(char *) ;

    /* make sure we have environment variable */
    if(!(twdirG = TWFLOWDIR)) {
	M(ERRMSG,"twflow","Can't get environment variable 'TWDIR'\n") ;
	M(MSG,NULL, 
	"Please use setenv to set 'TWDIR' to TimberWolf directory\n" ) ;
	YexitPgm(MASTERFAIL);
    }

    if( argc < 2 || argc > 5 ){
	syntax() ;
    } else {
#ifndef NOGRAPHICS
	graphicsG  = TRUE ;
#else
	graphicsG  = FALSE ;
#endif /* NOGRAPHICS */
	windowId   = NULLWINDOW ;    /* initialize window to NULL */
	debug      = FALSE ;
	lock       = FALSE ;
	tomusG     = FALSE ;
	parasite   = FALSE ;
	autoflowG  = TRUE ;
	general_mode = FALSE ;
	verbose    = FALSE ;
	arg_count = 1 ;
	if( *argv[1] == '-' ){
	    for( ptr = ++argv[1]; *ptr; ptr++ ){
		switch( *ptr ){
		case 'd':
		    debug = TRUE ;
		    break ;
		case 'g':
		    general_mode = TRUE ;
		    break ;
		case 'l':
		    lock = TRUE ;
		    break ;
		case 'n':
		    graphicsG = FALSE ;
		    break ;
		case 'p':
		    autoflowG = FALSE ;
		    break ;
		case 't':
		    tomusG = TRUE ;
		    break ;
		case 'v':
		    verbose = TRUE ;
		    break ;
		case 'w':
		    parasite = TRUE ;
		    break ;
		default:
		    sprintf( YmsgG,"Unknown option:%c\n", *ptr ) ;
		    M(ERRMSG,"main",YmsgG);
		    syntax() ;
		}
	    }
	    YdebugMemory( debug ) ;
	    YinitProgram( MASTER, VERSION, yaleIntro );

	    cktNameG = Ystrclone( argv[++arg_count] );
	    Ymessage_mode( verbose ) ;

	    /* now tell the user what he picked */
	    M(MSG,NULL,"Twflow switches:\n" ) ;
	    if( debug ){
		YsetDebug( TRUE ) ;
		M(MSG,NULL,"\tdebug on\n" ) ;
	    } 
	    if( graphicsG ){
		M(MSG,NULL,"\tGraphics mode on\n" ) ;
	    } else {
		M(MSG,NULL,"\tGraphics mode off\n" ) ;
	    }
	    if( parasite ){
		M(MSG,NULL,"\tTwflow will inherit window\n" ) ;
		/* look for windowid */
		if(argc != 4){
		    M(ERRMSG,"main","Need to specify windowID\n" ) ;
		    syntax() ;
		} else {
		    G( windowId = atoi( argv[++arg_count] ) ) ;
		} 
	    }
	    if( general_mode ){
		M(MSG,NULL,"\tGeneral mode on\n" ) ;
	    } else if( tomusG ){
		M(MSG,NULL,"\tTomus partition mode on\n" ) ;
	    } else {
		M(MSG,NULL,"\tTimberWolf mode on\n" ) ;
	    }
	    M(MSG,NULL,"\n" ) ;

	} else if( argc <= 3 ){
	    /* order is important here */
	    YdebugMemory( FALSE ) ;
	    cktNameG = Ystrclone( argv[arg_count] );
	    YinitProgram( MASTER, VERSION, yaleIntro );
	} else {
	    syntax() ;
	}
	if( ++arg_count < argc ){
	    /* this means we have the flow directory specified */
	    flow_dirG = Ystrclone( argv[arg_count] ) ;
	    sprintf( YmsgG, "\n\tFlow directory given:%s\n\n", flow_dirG );
	    M( MSG,NULL, YmsgG ) ;
	}
    }


    /* initialize the graphics */
    G( init_graphics(argc,argv,windowId) ) ;

    Ylog_start( cktNameG, "Program initialization completed..." ) ;
    if( lock ){
	/* create a lock file to say we are busy */
	sprintf( filename, "/tmp/twsc.%s", cktNameG ) ;
	(VOID) Yfile_create_lock( filename, FALSE ) ;
    }
    Ymessage_flush() ;

    /* ------------------ end initialization ------------------------- */

    /* determine the correct flow file */
    /* return file and filename */
    fp = find_flow_file( general_mode, debug, filename ) ;

    /* now we can read this file */
    readobjects( fp, filename ) ;

    /* If we haven't been given a flow directory override, find */
    /* the flow directory.  */
    if(!(flow_dirG)){
	sprintf( filename, "%s/bin/flow/flow", twdirG ) ;
	if( flow_dirG = Yfile_slink( filename )){
	    flow_dirG = Ystrclone( flow_dirG ) ;
	} else {
	    G( TWcloseGraphics() ) ;
	    YexitPgm(PGMFAIL);
	}
    }

    /* let user user see data */
    G( draw_the_data() ) ;

    /* 
    verify_pathnames() ;
    */

    if( !graphicsG || autoflowG ){
	/* if no graphics must go auto_flow */
	autoflowG  = TRUE ;
	auto_flow() ;
    } else {
	G( process_graphics() ) ;
    }

    G( TWcloseGraphics() ) ;

    if( problemsG ){
	YexitPgm(PGMFAIL);
    } else {
	YexitPgm(PGMOK);
    }

} /* end main */


/* give user correct syntax */
syntax()
{
   M(ERRMSG,NULL,"\n" ) ; 
   M(MSG,NULL,"Incorrect syntax.  Correct syntax:\n");
   sprintf( YmsgG, "\ngraywolf [-gpndw] designName [windowId] [flowdirectory]\n" );
   M(MSG,NULL,YmsgG ) ; 
   M(MSG,NULL,"\twhose options are one or more of the following:\n");
   M(MSG,NULL,"\t\tg - general mode - does not use TimberWolf system\n");
   M(MSG,NULL,"\t\t    information.  Default is TimberWolf mode\n");
   M(MSG,NULL,"\t\tp - pick mode - [graphics only] wait for user\n");
   M(MSG,NULL,"\t\t    upon entering the program\n");
   M(MSG,NULL,"\t\tn - no graphics - the default is to open the\n");
   M(MSG,NULL,"\t\t    display and output graphics to an Xwindow\n");
   M(MSG,NULL,"\t\td - prints debug info and performs extensive\n");
   M(MSG,NULL,"\t\t    error checking\n");
   M(MSG,NULL,"\t\tw - parasite mode will inherit a window. Requires\n");
   M(MSG,NULL,"\t\t    a valid windowId\n");

   show_flows() ;

   YexitPgm(MASTERFAIL);
} /* end syntax */

VOID yaleIntro() 
{
    char message[LRECL] ;

    sprintf( message,"\n%s\n",YmsgG) ;
    M(MSG,NULL,message) ;
    M(MSG,NULL,"Authors: Bill Swartz, Carl Sechen\n");
    M(MSG,NULL,"         Yale University\n");

} /* end yaleIntro */


show_flows()
{
   char command[LRECL] ; 
   /* now show user the flow directories */ 
   sprintf( command, "%s/bin/show_flows", twdirG ) ;
   Ysystem( "show_flows", ABORT, command, NULL ) ;
} /* end show_flows */
