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
FILE:	    cmain.c
DESCRIPTION:This file contains main control routine for compaction 
	    program.
CONTENTS:   
DATE:	    Apr 17, 1989 
REVISIONS:  Apr 30, 1989 - modified program for new library and
		added partion code to compactor.
	    May  3, 1989 - changed to Y prefixes.
	    May  6, 1989 - added no graphics compile switch
	    Sat Feb 23 00:48:39 EST 1991 - added constraints flag.
	    Fri Mar 29 14:17:51 EST 1991 - added DEBUGX switch and
		added path deck initialization.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) cmain.c (Yale) version 7.4 4/21/91" ;
#endif

#include <stdio.h>
#include <signal.h>
#include <yalecad/cleanup.h>
#include <yalecad/message.h>
#include <yalecad/program.h>
#include <yalecad/string.h>
#include <yalecad/debug.h>

#define COMPACT_DEFS
#include <compact.h>

#define EXPECTEDMEMORY  (256 * 1024)  /* 256k is more than enough */

main( argc , argv )
int argc ;
char *argv[] ;
{

char        *ptr ;
int         windowId ;

#ifdef DEBUGX
    extern int _Xdebug ;
    _Xdebug = TRUE ;
#endif

/* start up cleanup handler */
YINITCLEANUP( argv[0], NULL, MAYBEDUMP ) ;

Yinit_memsize( EXPECTEDMEMORY ) ;

if( argc < 8 || argc > 10 ){
    syntax() ;
} else {
    graphicsG  = TRUE ;
    alignG     = FALSE ;
    compactG   = TRUE ; 
    parasiteG  = FALSE ;
    partitionG = FALSE ;
    debugG     = FALSE ;
    windowId   = 0 ;
    constraintsG = FALSE ;
    if( *argv[1] == '-' ){
	for( ptr = ++argv[1]; *ptr; ptr++ ){
	    switch( *ptr ){
	    case 'a':
		alignG = TRUE ;
	        break ;
	    case 'd':
		debugG = TRUE ;
	        break ;
	    case 'c':
		constraintsG = TRUE ;
	        break ;
	    case 'n':
	        graphicsG = FALSE ;
	        break ;
	    case 'p':
	        partitionG = TRUE ;
	        break ;
	    case 'v':
	        compactG = FALSE ;
	        break ;
	    case 'w':
	        parasiteG = TRUE ;
	        break ;
	    default:
		sprintf( YmsgG,"Unknown option:%c\n", *ptr ) ;
		M(ERRMSG,"main",YmsgG);
		syntax() ;
	    }
	}
	YdebugMemory( debugG ) ;
	fprintf( stdout, "\n%s\n",YinitProgram(COMPACT,"v1.0",NULL) ) ;

	cktNameG = Ystrclone( argv[2] );
	blockrG = atoi( argv[3] ) ;
	blocktG = atoi( argv[4] ) ;
	xgridG = atoi( argv[5] ) ;
	ygridG = atoi( argv[6] ) ;
	xspaceG = atoi( argv[7] ) ;
	yspaceG = atoi( argv[8] ) ;
	/* now tell the user what he picked */
	M(MSG,NULL,"compactor switches:\n" ) ;
	if( compactG ){
	    M(MSG,NULL,"\tViolation removal and compaction\n" ) ;
	} else {
	    M(MSG,NULL,"\tViolation removal only\n" ) ;
	}
	if( alignG ){
	    M(MSG,NULL,"\tCell will be aligned by channels.\n" ) ;
	}
	if( debugG ){
	    YsetDebug( TRUE ) ;
	    M(MSG,NULL,"\tdebug on\n" ) ;
	} 
	if( graphicsG ){
	    M(MSG,NULL,"\tGraphics mode on\n" ) ;
	} else {
	    M(MSG,NULL,"\tGraphics mode off\n" ) ;
	}
	if( partitionG ){
	    M(MSG,NULL,"\tMacro cell placement algorithm on\n" ) ;
	}
	if( parasiteG ){
	    M(MSG,NULL,"\tCompactor will inherit window\n" ) ;
	    /* look for windowid */
	    if( argc != 10 ){
		M(ERRMSG,"compactor","Need to specify windowID\n" ) ;
		syntax() ;
	    } else {
		windowId = atoi( argv[9] ) ;
	    } 
	}
	M(MSG,NULL,"\n" ) ;
    } else if( argc == 8 ){
	cktNameG = Ystrclone( argv[1] );
	blockrG = atoi( argv[2] ) ;
	blocktG = atoi( argv[3] ) ;
	xgridG = atoi( argv[4] ) ;
	ygridG = atoi( argv[5] ) ;
	xspaceG = atoi( argv[6] ) ;
	yspaceG = atoi( argv[7] ) ;
	YdebugMemory( FALSE ) ;
	fprintf( stdout, "\n%s\n",YinitProgram(COMPACT,"v1.0",NULL) ) ;
    } else {
	syntax() ;
    }
}

/* ********************** end initialization ************************* */
/* set core region */
blocklG = 0 ;
blockbG = 0 ;
blockmxG = blockrG / 2 ;
blockmyG = blocktG / 2 ;

/* turn on the graphics if requested */
if( graphicsG ){
    G( init_graphics( argc, argv, windowId ) ) ;
}

readtiles() ;

if( constraintsG ){
    /* read the channel graph */
    read_cgraph() ;
    build_trees() ;
    path_deckG = Ydeck_init() ;
}

if( graphicsG ){
    G( draw_the_data() ) ;
}

grid_data() ;

if( graphicsG ){
    G( draw_the_data() ) ;
}

/* ------------------------------------------------------------------
    Determine which compaction option is requested.  The normal case
    is to remove violations and compact along the critical path.  For
    the partition case we need to also handle the constraints placed
    on softcells versus hardcells.
--------------------------------------------------------------------*/
if( partitionG ){
    partition_compact() ;
} else { /* normal case */
    remove_violations() ;
    M(MSG,NULL,"Violation removal completed...\n") ;
    if( compactG ){
	compact() ;
    }
}

output() ;

if( graphicsG ){
    G( TWcloseGraphics() ) ;
}
YexitPgm(PGMOK);

} /* end main */


/* give user correct syntax */
syntax()
{
   M(ERRMSG,NULL,"\n" ) ; 
   M(MSG,NULL,"Incorrect syntax.  Correct syntax:\n");
   sprintf( YmsgG, 
       "\n%s [-nwvp] designName blockr blockt xgrid ygrid xspace yspace [windowId] \n",
	COMPACT );
   M(MSG,NULL,YmsgG ) ; 
   M(MSG,NULL,"\twhose options are one or more of the following:\n");
   M(MSG,NULL,"\t\tn - no graphics - the default is to open the\n");
   M(MSG,NULL,"\t\t    display and output graphics to an Xwindow\n");
   M(MSG,NULL,"\t\tw - parasite mode - user must specify windowId\n");
   M(MSG,NULL,"\t\tv - remove only violations - overlap between cells\n");
   M(MSG,NULL,"\t\t  - default is to perform compaction in addition\n");
   M(MSG,NULL,"\t\td - prints debug info and performs extensive\n");
   M(MSG,NULL,"\t\t    error checking\n");
   M(MSG,NULL,"\t\tp - partition algorithm is selected for placing\n");
   M(MSG,NULL,"\t\t    pads in stdcell context\n");
   M(MSG,NULL,"\txgrid and ygrid force cells to specified grid positions\n");
   M(MSG,NULL,"\tblockr and blockt are the maximum spans of core region\n");
   YexitPgm(COMPACTFAIL);
}
