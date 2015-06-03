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
FILE:	    program.c 
DESCRIPTION:This file contains routines for executing and logging the
	    execution times of the program.
CONTENTS:   BOOL executePgm( adjptr )
		ADJPTR adjptr ;
	    set_path0( pathname )
		char *pathname ;
DATE:	    May  7, 1989 - original coding.
REVISIONS:  Jun 19, 1989 - shortened designName to $ for substitution.
	    Thu Mar  7 01:35:22 EST 1991 - now windowID is a global
		and twflow can inherit a window itself.
	    Sun Apr 21 22:37:20 EDT 1991 - changed windowId to @WINDOWID
		and added @FLOWDIR.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) program.c version 2.3 4/21/91" ;
#endif

#include <string.h>
#include <yalecad/base.h>
#include <yalecad/message.h>
#include <yalecad/string.h>
#include <yalecad/file.h>
#include <yalecad/debug.h>
#include <globals.h>

#define  DESIGNNAME   "$"
#define  DSNLEN       1
#define  WINDOWID     "@WINDOWID"
#define  FLOWDIR      "@FLOWDIR"

BOOL executePgm( adjptr )
ADJPTR adjptr ;
{
    char *Yfixpath() ;               /* get full pathname */
    char command[LRECL] ;
    char window_name[LRECL] ;        /* the window name */
    OBJECTPTR obj ;                  /* current object */
    INT i ;                          /* arg counter */
    INT status ;                     /* program return status */
    char **argv ;                    /* argument vector */
    BOOL stateSaved ;                /* whether graphics state was saved*/

    obj = proGraphG[adjptr->node] ;

    sprintf( YmsgG, "Executing %s", obj->name)  ;
    G( TWmessage( YmsgG ) ) ;

    /* build command to be executed */
    command[0] = EOS ; /* clear character string */

    stateSaved = FALSE ;  /* for remember whether we save graphics */

    /* first the program name */
    if( obj->path ){
	/* take users pathname */
	sprintf( command, "%s", Yfixpath( obj->path,TRUE ) ) ;
    } else {
	/* otherwise take default TimberWolf directory */
	sprintf( command, "%s/bin/%s", twdirG, obj->name ) ;
    }
    argv = adjptr->argv ;
    for( i = 0 ; i < adjptr->argc; i++ ){
	strcat( command, " " ) ;
	if( strncmp( argv[i], DESIGNNAME, DSNLEN ) == STRINGEQ ){
	    /* +1 skips over $ to see if other is present */
	    strcat( command, cktNameG ) ;
	    strcat( command, argv[i]+1 ) ;
	} else if( strcmp( argv[i], WINDOWID ) == STRINGEQ ){
	    /* save state of graphics before call if necessary */
	    if( graphicsG ){
		G( sprintf( window_name, "%d", TWsaveState() ) ) ;
		stateSaved = TRUE ;
	    }
	    strcat( command, window_name ) ;

	} else if( strcmp( argv[i], FLOWDIR ) == STRINGEQ ){
	    /* add flow directory */
	    strcat( command, flow_dirG ) ;
	} else {
	    strcat( command, argv[i] ) ;
	}
    }
    D( "twflow/executePgm", sprintf( YmsgG, "%s\n", command ) ) ;
    D( "twflow/executePgm", M( MSG, NULL, YmsgG ) ) ;

    /* now log the beginning time */
    sprintf( YmsgG, "%s started...", obj->name ) ;
    Ylog_msg( YmsgG ) ;

    /* now execute the command */
    status = system( command ) ;

    sprintf( YmsgG, "%s completed...", obj->name ) ;
    Ylog_msg( YmsgG ) ;

    if( stateSaved ){
	/* if we save the graphics state we need to restore it */
	G( TWrestoreState() ) ;
    }

    return( status ) ;

} /* end execute Pgm */
