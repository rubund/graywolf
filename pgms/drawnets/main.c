/* ----------------------------------------------------------------- 
FILE:	    main.c
DESCRIPTION:This file contains control for xxxx program.
CONTENTS:   main( argc , argv )
		INT argc ;
		char *argv[] ;
DATE:	    
REVISIONS:  
---------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) main.c (Yale) version 1.1 4/21/91" ;
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

main( argc , argv )
INT argc ;
char *argv[] ;
{

    char        filename[LRECL] ;    /* buffer for filename */
    char        *ptr ;               /* argument pointer */
    char        *Ygetenv() ;         /* get environment variable */
    FILE        *fp ;                /* file pointer */
    BOOL        debug ;              /* TRUE if debug on */
    BOOL        verbose ;            /* whether to go into verbose mode.*/
    BOOL        parasite ;           /* TRUE if we are to take over win */
    INT         arg_count ;          /* parse the command line */
    INT         windowId ;           /* the parasite window id */
    VOID        yaleIntro() ;        /* give intro for program */

#ifdef DEBUGX
    extern int _Xdebug ;
    _Xdebug = TRUE ;
#endif

    /* start up cleanup handler */
    YINITCLEANUP( argv[0], NULL, MAYBEDUMP ) ;

    Yinit_memsize( EXPECTEDMEMORY ) ;

    /* make sure we have environment variable */
    if(!(Ygetenv("TWDIR"))) {
	M(ERRMSG,"twflow","Can't get environment variable 'TWDIR'\n") ;
	M(MSG,NULL, 
	"Please use setenv to set 'TWDIR' to TimberWolf directory\n" ) ;
	YexitPgm(PGMFAIL);
    }

    if( argc < 2 || argc > 5 ){
	syntax() ;
    } else {
	windowId   = NULLWINDOW ;    /* initialize window to NULL */
	debug      = FALSE ;
	verbose    = FALSE ;
	graphicsG  = TRUE ;
	arg_count = 1 ;
	if( *argv[1] == '-' ){
	    for( ptr = ++argv[1]; *ptr; ptr++ ){
		switch( *ptr ){
		case 'd':
		    debug = TRUE ;
		    break ;
		case 'n':
		    graphicsG = FALSE ;
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
	    YinitProgram( PROGRAM, VERSION, yaleIntro );

	    cktNameG = Ystrclone( argv[++arg_count] );
	    Ymessage_mode( verbose ) ;

	    /* now tell the user what he picked */
	    sprintf( YmsgG, "%s switches:\n", PROGRAM ) ;
	    M(MSG,NULL,YmsgG ) ;
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
		sprintf( YmsgG, "%s will inherit window\n" ) ;
		M(MSG,NULL,YmsgG ) ;
		/* look for windowid */
		if(argc != 4){
		    M(ERRMSG,"main","Need to specify windowID\n" ) ;
		    syntax() ;
		} else {
		    G( windowId = atoi( argv[++arg_count] ) ) ;
		} 
	    }
	    M(MSG,NULL,"\n" ) ;

	} else if( argc <= 3 ){
	    /* order is important here */
	    YdebugMemory( FALSE ) ;
	    cktNameG = Ystrclone( argv[arg_count] );
	    YinitProgram( PROGRAM, VERSION, yaleIntro );
	} else {
	    syntax() ;
	}
    }

    /* initialize the graphics */
    G( init_graphics(argc,argv,windowId) ) ;
    Ymessage_flush() ;

    /* ------------------ end initialization ------------------------- */

    sprintf( filename, "%s.pth", cktNameG ) ;
    fp = TWOPEN( filename, "r", ABORT ) ;

    readnets( fp ) ;

    G( process_graphics() ) ;

    closegraphics() ;

    YexitPgm(PGMOK);

} /* end main */


/* give user correct syntax */
syntax()
{
   M(ERRMSG,NULL,"\n" ) ; 
   M(MSG,NULL,"Incorrect syntax.  Correct syntax:\n");
   sprintf( YmsgG, "\n%s [-ndvw] designName [windowId]\n" );
   M(MSG,NULL,YmsgG ) ; 
   M(MSG,NULL,"\twhose options are one or more of the following:\n");
   M(MSG,NULL,"\t\tn - no graphics - the default is to open the\n");
   M(MSG,NULL,"\t\t    display and output graphics to an Xwindow\n");
   M(MSG,NULL,"\t\td - prints debug info and performs extensive\n");
   M(MSG,NULL,"\t\t    error checking\n");
   M(MSG,NULL,"\t\tv - verbose mode - echos to the screen\n");
   M(MSG,NULL,"\t\tw - parasite mode will inherit a window. Requires\n");
   M(MSG,NULL,"\t\t    a valid windowId\n");

   YexitPgm(PGMFAIL);
} /* end syntax */

VOID yaleIntro() 
{
    char message[LRECL] ;

    sprintf( message,"\n%s\n",YmsgG) ;
    M(MSG,NULL,message) ;
    M(MSG,NULL,"Authors: Bill Swartz, Carl Sechen\n");
    M(MSG,NULL,"         Yale University\n");

} /* end yaleIntro */
