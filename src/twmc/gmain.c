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
FILE:	    gmain.c
DESCRIPTION:This file now contain calls the channel graph generator
	    program.
CONTENTS:   
DATE:	    Feb 28, 1990 
REVISIONS:  Thu Jan 17 00:49:52 PST 1991 - now read side information.
	    Thu Mar  7 01:46:48 EST 1991 - added call to sleep to
		prevent a SUN race condition.
	    Wed May  1 19:17:23 EDT 1991 - added switchbox keyword
		so we can ignore these areas during wire estimation.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) gmain.c version 3.8 5/1/91" ;
#endif

#define DENS_DEFS

#include <custom.h>
#include <dens.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>
#include <yalecad/string.h>

#include "config-build.h"

#define GENGRAPHPROG      "gengraph"
#define GENGRAPHPATH      "../gengraph"



static free_routing_tiles();


/* --------------------------------------------------------------------
    There are two ways to call the channel graph generator.
    1. updateNotChan == FALSE:
	call the channel graph generator to build graph and input files
	for the global router.
    2. updateNotChan == TRUE:
	call the channel graph generator to read the density information
	from the global router and build the routing tiles.  We will
	update the routing tiles in this file.
-------------------------------------------------------------------- */
gmain( updateNotChan )
BOOL updateNotChan ;  /* if true update routing tiles otherwise normal */
{
    char filename[LRECL] ;
    char *Yrelpath() ;
    char *pathname ;
    char *twdir ;       /* path of TimberWolf directory */
    char *getenv() ;    /* used to get TWDIR environment variable */
    INT  windowId ;     /* windowId of current window */
    char buffer[LRECL], *bufferptr ;
    char **tokens ;     /* for parsing file */
    INT  numtokens, line ;
    INT closegraphics() ;
    BOOL abort ; /* whether to abort program */
    INT cell ;
    INT xc, yc ;        /* xcenter and ycenter of cell */
    INT side ;                    /* side that routing tile is on */
    INT xcenter, ycenter ;        /* xcenter and ycenter of cell */
    INT x1, x2, y1, y2 ;
    INT ncells ;                   /* the number of cells in graph */
    CELLBOXPTR cptr ;
    RTILEBOXPTR tmp ;  /* current routing tile */
    RTILEBOXPTR tile ;  /* current routing tile */
    FILE *fp ;
    BOOL stateSaved = FALSE ;      /* TRUE when graphics parasite mode */

    /* ################### begin gengraph execution ################## */
    /* find the path of compactor relative to main program */
    pathname = Yrelpath( argv0G, GENGRAPHPATH ) ;
    if( !(YfileExists(pathname))){
	if( twdir = TWFLOWDIR ){
	    sprintf( filename, "%s/bin/%s", twdir, GENGRAPHPROG ) ;
	    pathname = Ystrclone( filename ) ;
	}
    }
    if( updateNotChan ){
	/* never use graphics to update channel graph */
	sprintf( YmsgG, "%s -nr %s", pathname, cktNameG ) ;
    } else {
	if( doGraphicsG ){
	    G( windowId = TWsaveState() ) ;
	    stateSaved = TRUE ;
	    sprintf( YmsgG, "%s -w %s %d", pathname,cktNameG,windowId ) ;
	} else {
	    sprintf( YmsgG, "%s -n %s", pathname, cktNameG ) ;
	}
    }
    M( MSG, NULL, YmsgG ) ;
    M( MSG, NULL, "\n" ) ;
    /* Ysystem will kill program if catastrophe occurred */
    Ysystem( GENGRAPHPROG, ABORT, YmsgG, closegraphics ) ;

    if( stateSaved ){
	sleep(1) ;
	/* if we save the graphics state we need to restore it */
	G( TWrestoreState() ) ;
    }
    Ysafe_free( pathname ) ; /* free name created in Yrelpath */
    /* ################### end of gengraph execution ################# */


    if( !(updateNotChan) ){
	/* no more work to do */
	return ;
    }

    /* ********************* read routing tiles *********************** */
    free_routing_tiles() ;

    /* number of cells is core cells + 4 pad blocks */
    ncells = numcellsG + 4 ;

    /* allocate list of routing tiles for cells */
    routingTilesG = (RTILEBOXPTR *) 
	Ysafe_calloc( ncells + 1, sizeof(RTILEBOXPTR) );

    /* add number of cells to routing tile structure */
    routingTilesG[0] = (RTILEBOXPTR) ncells ;

    sprintf( filename, "%s.mtle", cktNameG ) ;
    fp = TWOPEN( filename, "r", ABORT ) ;

    /* parse file */
    line = 0 ;
    abort = FALSE ;
    while( bufferptr=fgets(buffer,LRECL,fp )){
	/* parse file */
	line ++ ; /* increment line number */
	tokens = Ystrparser( bufferptr, " :\t\n", &numtokens );

	if( numtokens == 0 ){
	    /* skip over empty lines */
	    continue ;
	} else if( numtokens == 16 || numtokens == 17 ){
	    cell = atoi( tokens[1] ) ;
	    xc   = atoi( tokens[3] ) ;
	    yc   = atoi( tokens[5] ) ;
	    x1   = atoi( tokens[7] ) ;
	    y1   = atoi( tokens[9] ) ;
	    x2   = atoi( tokens[11] ) ;
	    y2   = atoi( tokens[13] ) ;
	    side = atoi( tokens[15] ) ;

	    if( tmp = routingTilesG[cell] ){
		tile = routingTilesG[cell] = (RTILEBOXPTR)
		    Ysafe_malloc( sizeof(RTILEBOX) );
		tile->next = tmp ;
	    } else {
		tile = routingTilesG[cell] = (RTILEBOXPTR) 
		    Ysafe_malloc( sizeof(RTILEBOX) );
		tile->next = NULL ;
	    }

	    /* check xcenter of the cells to see if we must adjust tile */
	    if( cell <= numcellsG ){
		cptr = cellarrayG[cell] ;
	    } else {
		cptr = cellarrayG[endpadgrpsG+cell-numcellsG] ;
	    }
	    xcenter = cptr->xcenter ;
	    ycenter = cptr->ycenter ;

	    tile->x1 = x1 + xc - xcenter ;
	    tile->y1 = y1 + yc - ycenter ;
	    tile->x2 = x2 + xc - xcenter ;
	    tile->y2 = y2 + yc - ycenter ;
	    tile->side = side ;
	    if( numtokens == 17 ){
		if( strcmp( tokens[16], "switchbox" ) == STRINGEQ ){
		    tile->switchbox = TRUE ;
		} else {
		    tile->switchbox = FALSE ;
		}
	    } else {
		tile->switchbox = FALSE ;
	    }

	} else {
	    sprintf( YmsgG, "Syntax error on line:%d\n", line ) ;
	    M(ERRMSG, "gmain", YmsgG ) ;
	    abort = TRUE ;
	}
    }
    if( abort ){
	closegraphics() ;
	YexitPgm( PGMFAIL ) ;
    }
    TWCLOSE( fp ) ;
    /* ********************** end routing tiles ********************** */

} /* end gmain */


init_routing_tiles()
{
    routingTilesG = NULL ;
} /* end init_routing_tiles */

static free_routing_tiles()
{
    INT cell ;              /* cell counter */
    RTILEBOXPTR freeptr ;   /* free tile */
    RTILEBOXPTR rptr ;      /* traverse tiles */

    /* free previous routing tiles if they exist */
    if( routingTilesG ){
	for( cell = 1 ; cell <= numcellsG; cell++ ){
	    for( rptr = routingTilesG[cell]; rptr; ){
		freeptr = rptr ;
		rptr = rptr->next ;
		Ysafe_free( freeptr ) ;
	    }
	}
	Ysafe_free( routingTilesG ) ;
	routingTilesG = NULL ;
    }
} /* end free_routing_tiles */
