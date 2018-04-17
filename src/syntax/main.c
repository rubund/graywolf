/*
 *   Copyright (C) 1989-1991 Yale University
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
DESCRIPTION:This file contains main control routine for the syntax  
	    program.
CONTENTS:   
DATE:	    Apr 24, 1989 
REVISIONS:  Dec  8, 1989 - now write temp file to be moved later so
		that programs that follow won't run until passing 
		syntax.
	    Fri Jan 25 17:57:06 PST 1991 - added debug flag to syntax.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) main.c version 1.6 2/23/91" ;
#endif

#define MAIN_VARS

#include <stdio.h>
#include <signal.h>
#include <yalecad/base.h>
#include <yalecad/cleanup.h>
#include <yalecad/file.h>
#include <yalecad/message.h>
#include <yalecad/debug.h>
#include "globals.h"

#define EXPECTEDMEMORY  (1024 * 1024)  /* 1M should be enough */
#define VERSION         "v1.1"

main( argc , argv )
int argc ;
char *argv[] ;
{

    char *YinitProgram(), *Ystrclone() ;
    char filename[LRECL] ;
    char filename2[LRECL] ;
    char *ptr ;
    int  arg_count ;
    int  yaleIntro() ;
    int  debug ;
    FILE *fp ;

    /* start up cleanup handler */
    YINITCLEANUP( argv[0], NULL, MAYBEDUMP ) ;

    Yinit_memsize( EXPECTEDMEMORY ) ;

    if( argc < 2 || argc > 3 ){
	syntax() ;
    } else {
	debug      = FALSE ;
	arg_count = 1 ;
	if( *argv[1] == '-' ){
	    for( ptr = ++argv[1]; *ptr; ptr++ ){
		switch( *ptr ){
		case 'd':
		    debug = TRUE ;
		    break ;
		default:
		    sprintf( YmsgG,"Unknown option:%c\n", *ptr ) ;
		    M(ERRMSG,"main",YmsgG);
		    syntax() ;
		}
	    }
	    YdebugMemory( debug ) ;
	    cktNameG = Ystrclone( argv[++arg_count] );

	    /* now tell the user what he picked */
	    M(MSG,NULL,"\n\nSyntax switches:\n" ) ;
	    if( debug ){
		YsetDebug( TRUE ) ;
		M(MSG,NULL,"\tdebug on\n" ) ;
	    } 
	    M(MSG,NULL,"\n" ) ;
	} else if( argc == 2 ){
	    /* order is important here */
	    YdebugMemory( FALSE ) ;
	    cktNameG = Ystrclone( argv[1] );

	} else {
	    syntax() ;
	}
    }

    /* remove old version of stat file */
    sprintf( filename, "%s.stat", cktNameG ) ;
    Yrm_files( filename ) ;

    sprintf( filename, "%s.temp", cktNameG ) ;
    fpoG = TWOPEN( filename, "w", ABORT ) ;

    /* we can change this value in the debugger */
    YinitProgram(SYNTAX, VERSION, yaleIntro) ;

    sprintf( filename, "%s.cel", cktNameG ) ;
    fp = TWOPEN( filename, "r", ABORT ) ;
    readcells( fp ) ;

    if(!(Ymessage_get_errorcount() ) ){
	M( MSG, NULL, "No syntax errors were found\n" ) ;
	output() ;
    } else {
	TWCLOSE( fp ) ;
	TWCLOSE( fpoG ) ;
        YexitPgm( PGMFAIL ) ;
    }
    TWCLOSE( fp ) ;
    TWCLOSE( fpoG ) ;

    /* now move .temp file to .stat */
    sprintf( filename, "%s.temp", cktNameG ) ;
    sprintf( filename2, "%s.stat", cktNameG ) ;
    YmoveFile( filename, filename2 ) ;

    YexitPgm( PGMOK ) ;

} /* end main */


/* give user correct syntax */
syntax()
{
   M(ERRMSG,NULL,"\n" ) ; 
   M(MSG,NULL,"Incorrect syntax.  Correct syntax:\n");
   sprintf( YmsgG, 
       "\n%s circuitName\n\n", SYNTAX );
   M(MSG,NULL,YmsgG ) ; 
   YexitPgm(PGMFAIL);
} /* end syntax */

yaleIntro() 
{
    int i ;

    M( MSG, NULL, "\n") ;
    M( MSG, NULL, YmsgG) ;
    M( MSG, NULL, "\nTimberWolf System Syntax Checker\n");
    M( MSG, NULL, "Authors: Carl Sechen, Kai-Win Lee, Bill Swartz,\n");
    M( MSG, NULL, "         Dahe Chen, and Jimmy Lam\n"); 
    M( MSG, NULL, "         Yale University\n");
    M( MSG, NULL, "\n");

    fprintf(fpoG,"%s\n",YmsgG ) ; 

} /* end yaleIntro */
