/* ----------------------------------------------------------------- 
FILE:	    main.c
DESCRIPTION:This file contains main control routine for the pad format
	    translator program.
CONTENTS:   
DATE:	    Apr 24, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) main.c version 1.4 5/11/91" ;
#endif

#define MAIN_VARS

#include <stdio.h>
#include <signal.h>
#include <yalecad/base.h>
#include <yalecad/cleanup.h>
#include <yalecad/file.h>
#include <yalecad/message.h>
#include <yalecad/string.h>
#include <yalecad/debug.h>
#include "globals.h"

#define EXPECTEDMEMORY  (1024 * 1024)  /* 1M should be enough */

main( argc , argv )
int argc ;
char *argv[] ;
{

    char *YinitProgram() ;
    char filename[LRECL] ;
    char filename2[LRECL] ;
    int  yaleIntro() ;
    FILE *fp ;

    /* start up cleanup handler */
    YINITCLEANUP( argv[0], NULL, MAYBEDUMP ) ;

    Yinit_memsize( EXPECTEDMEMORY ) ;
    YdebugMemory( TRUE ) ;

    if( argc != 2 ){
	syntax() ;
    }

    sprintf( filename, "%s.pads", argv[1] ) ;
    fpoG = TWOPEN( filename, "w", ABORT ) ;

    /* we can change this value in the debugger */
    YdebugMemory( FALSE ) ;
    YinitProgram(PADTRANSLATE,"v1.0",yaleIntro) ;

    cktNameG = Ystrclone( argv[1] ) ;

    sprintf( filename, "%s.cel", cktNameG ) ;
    fp = TWOPEN( filename, "r", ABORT ) ;
    readcells( fp ) ;

    TWCLOSE( fp ) ;
    TWCLOSE( fpoG ) ;

    /* now we can move the files */
    /* first move .cel file to .cel.orig */
    sprintf( filename2, "%s.cel.orig", cktNameG ) ;
    YmoveFile( filename, filename2 ) ;

    /* next move .pads to .cel file */
    sprintf( filename, "%s.pads", cktNameG ) ;
    sprintf( filename2, "%s.cel", cktNameG ) ;
    YmoveFile( filename, filename2 ) ;

    YexitPgm( PGMOK ) ;

} /* end main */


/* give user correct syntax */
syntax()
{
   M(ERRMSG,NULL,"\n" ) ; 
   M(MSG,NULL,"Incorrect syntax.  Correct syntax:\n");
   sprintf( YmsgG, 
       "\n%s circuitName\n\n", PADTRANSLATE );
   M(MSG,NULL,YmsgG ) ; 
   YexitPgm(PGMFAIL);
} /* end syntax */

yaleIntro() 
{

    M( MSG, NULL, YmsgG) ;
    M( MSG, NULL, "\nTimberWolf Pad Format Translator\n");
    M( MSG, NULL, "Authors: Carl Sechen, Kai-Win Lee, Bill Swartz,\n");
    M( MSG, NULL, "         Dahe Chen, and Jimmy Lam\n"); 
    M( MSG, NULL, "         Yale University\n");
    M( MSG, NULL, "\n");

} /* end yaleIntro */
