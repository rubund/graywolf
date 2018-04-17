/*
 *   Copyright (C) 1990-1991 Yale University
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
FILE:	    rmain.c
DESCRIPTION:This file now contain calls the global router mickey.
CONTENTS:   
DATE:	    Feb 28, 1990 
REVISIONS:  Sun Dec 16 00:36:43 EST 1990 - Modified for Dahe's
		new version of the global router.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) rmain.c version 3.6 2/23/91" ;
#endif

#include <yalecad/base.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>
#include <yalecad/string.h>
#include <custom.h>

#include "config-build.h"

#define GROUTEPROG      "Mickey"
#define GROUTEPATH      "../Mickey"


rmain( constraint_flag )
BOOL constraint_flag ;
{
    INT closegraphics() ;
    char filename[LRECL] ;
    char *Yrelpath() ;
    char *pathname ;
    char *twdir ;       /* path of TimberWolf directory */
    char *getenv() ;    /* used to get TWDIR environment variable */
    FILE *fp ;

    /* first build .gpar file for global router */
    sprintf( filename, "%s.gpar", cktNameG ) ;
    fp = TWOPEN( filename, "w", ABORT ) ;
    switch( get_circuit_type() ){
    case NO_CKT_TYPE:
    case DIGITAL:
	fprintf( fp, "digital.circuit\n" ) ;
	break ;
    case ANALOG:
	fprintf( fp, "analog.circuit\n" ) ;
	break ;
    case MIXED:
	fprintf( fp, "mixed.circuit\n" ) ;
	break ;
    }
    fprintf( fp, "limit_iteration 99999\n" ) ;
    if( constraint_flag ){
	fprintf( fp, "objective 2\n" ) ;
    } else {
	fprintf( fp, "objective 1\n" ) ;
    }
    TWCLOSE( fp ) ;

    /* now call the global router */
    /* find the path of compactor relative to main program */
    pathname = Yrelpath( argv0G, GROUTEPATH ) ;
    if( !(YfileExists(pathname))){
	if( twdir = TWFLOWDIR ){
	    sprintf( filename, "%s/bin/%s", twdir, GROUTEPROG ) ;
	    pathname = Ystrclone( filename ) ;
	}
    }
    sprintf( YmsgG, "%s %s", pathname, cktNameG ) ;
    M( MSG, NULL, YmsgG ) ;
    M( MSG, NULL, "\n" ) ;
    /* Ysystem will kill program if catastrophe occurred */
    Ysystem( GROUTEPROG, ABORT, YmsgG, closegraphics ) ;
    Ysafe_free( pathname ) ; /* free name created in Yrelpath */
    /* ############# end of gengraph execution ############# */

} /* end gmain */
