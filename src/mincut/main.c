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
static char SccsId[] = "@(#) main.c version 1.1 7/30/91" ;
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

#include "config-build.h"

#define EXPECTEDMEMORY  (1024 * 1024)  /* 1M should be enough */
#define VERSION         "v1.0"

int
__attribute__((visibility("default")))
Mincut( int debugON,  char *dName)
{
	char *YinitProgram(), *Ystrclone() ;
	char filename[LRECL] ;
	char command[LRECL] ;
	char *ptr ;
	int  arg_count ;
	void yaleIntro() ;
	int  debug ;
	FILE *fp ;
	char *twdir, *Ygetenv() ;

	/* start up cleanup handler */
	YINITCLEANUP( argv[0], NULL, MAYBEDUMP ) ;

	Yinit_memsize( EXPECTEDMEMORY ) ;

	if(debugON) {
		debug = TRUE;
		M(MSG,NULL,"\tdebug on\n" ) ;
	} else {
		debug = FALSE;
	}
	cktNameG = Ystrclone(dName);
	YsetDebug(debug) ;
	YdebugMemory( debug );

	/* we can change this value in the debugger */
	YinitProgram(NOCUT, VERSION, yaleIntro) ;

	read_par() ;

	sprintf( filename, "%s.cel", cktNameG ) ;
	readcells(filename) ;

	sprintf( filename, "%s.mcel", cktNameG ) ;
	fp = fopen( filename, "w") ;
	if(fp) {
		output( fp ) ;
		fclose( fp ) ;
	}

	sprintf( filename, "%s.stat", cktNameG ) ;
	fp = fopen( filename, "a") ;
	update_stats( fp );
	fclose( fp ) ;

	int separate_cel_file(char *cktName);
	if( twdir = TWFLOWDIR ){
		separate_cel_file(cktNameG);
	}

	printf( "Splitting %s.cel into " , cktNameG ) ;
	printf( "%s.scel and %s.mcel...\n" , cktNameG , cktNameG ) ;
	printf( "\tdone!\n\n" ) ;

	return 0;
} /* end main */

void yaleIntro()
{
    int i ;

    M( MSG, NULL, "\n") ;
    M( MSG, NULL, YmsgG) ;
    M( MSG, NULL, "\nTimberWolf System Floorplan Setup Program\n");
    M( MSG, NULL, "Authors: Carl Sechen, Bill Swartz,\n");
    M( MSG, NULL, "         Yale University\n");
    M( MSG, NULL, "\n");

} /* end yaleIntro */
