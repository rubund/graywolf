/*
 *   Copyright (C) 1988-1991 Yale University
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
FILE:	    compact.c
DESCRIPTION:This file now contain call to compaction program.
CONTENTS:   
DATE:	    Apr  8, 1988 
REVISIONS:  Nov  5, 1988 - free violations and modified position of
		sources and sinks.
	    Dec  4, 1988 - corrected error in where to get data.
	    Jan 15, 1989 - fixed constraINT problem for softcells
		by saving contents of tilebox and set orig_ fields 
		correctly for compaction cycle for softcells.
	    Jan 25, 1989 - removed incorrect force of box size to
		block area and added \n for new message macro.
	    Mar 11, 1989 - added graphics conditional compile and
		commented out compactor state dump.
	    Mar 30, 1989 - changed tile datastructure.
	    Apr 17, 1989 - made compactor a separate program.
	    May  1, 1989 - added cell partitioning flag to program.
	    May 24, 1989 - changed name of compact program.
	    Jun 21, 1989 - added TimberWolf directory search.
	    Feb  7, 1990 - Now compactor uses routing tiles in order
		to account for routing area.
	    Wed Jan 30 14:13:36 EST 1991 - now left justify orientation
		rotations.
	    Thu Apr 11 15:25:09 EDT 1991 - added constraint compaction.
	    Thu Apr 18 01:28:31 EDT 1991 - no graphics are passed
		to the compactor.
	    Sun May  5 14:19:53 EDT 1991 - pass gridding point to 
		compactor.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) compact.c version 3.12 5/5/91" ;
#endif

#include <custom.h>
#include <dens.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>
#include <yalecad/string.h>

#include "config-build.h"

#define VIOLATIONSONLY   FALSE
#define COMPACT          TRUE
#define COMPACTPROG      "mc_compact"
#define COMPACTPATH      "../mc_compact"

/* for compactor parser below */
#define INFOKEYWORD      "numtiles"
#define CELLKEYWORD      "cell"
#define SCELLKEYWORD     "stdcell"
#define TILEKEYWORD      "l"

compact( compactFlag )
BOOL compactFlag ; /* signals use of compaction */
{
    char filename[LRECL] ;
    char *Yrelpath() ;
    char *pathname ;
    char *twdir ;       /* path of TimberWolf directory */
    char *getenv() ;    /* used to get TWDIR environment variable */
    char buffer[LRECL], *bufferptr ;
    char **tokens ;     /* for parsing file */
    INT  numtokens, line ;
    BOOL abort ; /* whether to abort program */
    INT cell, numtiles, numcells ;
    INT xcenter, ycenter ;
    INT xoffset, yoffset ;
    INT l, r, b, t ;
    INT closegraphics() ;
    CELLBOXPTR cellptr ;
    TILEBOXPTR tileptr ;
    BOUNBOXPTR bounptr ;            /* bounding box pointer */
    RTILEBOXPTR rtptr ;             /* current routing tile */
    FILE *fp ;
    INT type ;


    /* ######### Create compaction file and exec compactor ######### */
    /* open compaction file for writing */
    sprintf(filename, "%s.mvio" , cktNameG ) ;
    fp = TWOPEN( filename , "w", ABORT ) ;

    /* first count number of tiles and cells to be output */
    numtiles = 0 ;
    numcells = 0 ;
    for( cell = 1 ; cell <= endsuperG ; cell++ ){
	cellptr = cellarrayG[cell] ;
	if( cellptr->celltype != CUSTOMCELLTYPE && cellptr->celltype !=
	    SOFTCELLTYPE ){
	    continue ;
	}
	numcells++ ;

	for( tileptr=cellptr->tiles;tileptr;tileptr = tileptr->next ){
	    numtiles++ ;
	}
	/* count number of routing tiles if they exist */
	if( routingTilesG ){
	    for( rtptr = routingTilesG[cell]; rtptr; rtptr = rtptr->next){
	        numtiles++ ;
	    }
	}
    } /* end counting number of tiles */

    /* now need to call compactor if no cells exist in the case of a single stdcell core */
    if( numtiles == 0 ){
	return ;
    }

    /* NOW output data */

    fprintf( fp, "numtiles:%d numcells:%d\n", numtiles, numcells ) ;
    for( cell = 1 ; cell <= endsuperG; cell++ ){
	cellptr = cellarrayG[cell] ;
	type = cellptr->celltype ;
	if( type != CUSTOMCELLTYPE && type != SOFTCELLTYPE ){
	    continue ;
	}
	xcenter = cellptr->xcenter ;
	ycenter = cellptr->ycenter ;
	xoffset = cellptr->bounBox[cellptr->orient]->l ;
	yoffset = cellptr->bounBox[cellptr->orient]->b ;

	if( type == CUSTOMCELLTYPE || type == SOFTCELLTYPE ){
	    fprintf( fp, "cell %d x:%d y:%d offset:%d %d\n", 
		cellarrayG[cell]->cellnum, xcenter, ycenter,
		xoffset, yoffset ) ;
	} else {
	    fprintf( fp, "stdcell %d x:%d y:%d offset:%d %d\n", 
		cellarrayG[cell]->cellnum, xcenter, ycenter,
		xoffset, yoffset ) ;
	}

	/* setup translation of output points */
	bounptr = cellptr->bounBox[0] ;
	/* now init the translation routines using bounding box */
	Ytrans_init( bounptr->l,bounptr->b,bounptr->r,bounptr->t,
		    cellptr->orient ) ;
	for( tileptr=cellptr->tiles;tileptr;tileptr = tileptr->next ){

	    l = tileptr->left ;
	    r = tileptr->right ;
	    b = tileptr->bottom ;
	    t = tileptr->top ;

	    if( cellptr->softflag){
		/* ------------------------------------------------
		   At this point, a softcells aspect ratio has 
		   been fixed.  So after placement set orig and current
		   fields of the tilebox to same value.  After this
		   point, the current field will be used for the border
		   added during global route and the orig field is cell
		   size without routing border.
		   --------------------------------------------------*/
		if( compactFlag == FALSE ){ /* 1st time compact called */
		    tileptr->orig_left = tileptr->left ;
		    tileptr->orig_right = tileptr->right ;
		    tileptr->orig_bottom = tileptr->bottom ;
		    tileptr->orig_top = tileptr->top ;
		}
	    }

	    /* calculate orientation for cell tiles */
	    YtranslateC( &l,&b,&r,&t,cellptr->orient) ;
	    fprintf( fp, "l:%d r:%d b:%d t:%d\n", l,r,b,t ) ;

	} /* end tiles of a cell loop */

	/* output routing tiles if they exist */
	if( routingTilesG ){
	    for( rtptr = routingTilesG[cell]; rtptr; rtptr = rtptr->next){
		fprintf( fp, "l:%d r:%d b:%d t:%d\n", 
		    rtptr->x1, rtptr->x2, rtptr->y1, rtptr->y2 ) ;
	    }
	}

    } /* end cell loop */

    TWCLOSE( fp ) ;


    /* now call the compactor */
    /* find the path of compactor relative to main program */
    pathname = Yrelpath( argv0G, COMPACTPATH ) ;
    if( !(YfileExists(pathname))){
	if( twdir = TWFLOWDIR ){
	    sprintf( filename, "%s/bin/%s", twdir, COMPACTPROG ) ;
	    pathname = Ystrclone( filename ) ;
	}
    }
    if( doPartitionG ){
	sprintf( YmsgG, "%s -vn %s %d %d %d %d %d %d", pathname,
	    cktNameG, blockrG, blocktG, track_spacingXG,track_spacingYG,
	    track_spacingXG, track_spacingYG);
    } else if( compactFlag == VIOLATIONSONLY ){
	sprintf( YmsgG, "%s -vn %s %d %d %d %d %d %d", pathname,
	    cktNameG, blockrG, blocktG, track_spacingXG,track_spacingYG,
	    track_spacingXG, track_spacingYG );
    } else if( compactFlag == COMPACT ){
	/*
	sprintf( YmsgG, "%s -n %s %d %d 0 0 0 0", pathname,
	    cktNameG, blockrG, blocktG );
	*/
	sprintf( YmsgG, "%s -cn %s %d %d %d %d 0 0", pathname,
	    cktNameG, blockrG, blocktG, track_spacingXG,track_spacingYG );

	D( "twsc/compact_graphics",
	    sprintf( YmsgG, "%s -c %s %d %d %d %d 0 0", pathname,
	    cktNameG, blockrG, blocktG, track_spacingXG,track_spacingYG);
	) ;
    } else {
	M( ERRMSG,"compact", "unknown compact flag\n" ) ;
	return ;
    }
    M( MSG, NULL, YmsgG ) ;
    M( MSG, NULL, "\n" ) ;
    /* Ysystme will kill program if catastrophe occurred */
    Ysystem( COMPACTPROG, ABORT, YmsgG, closegraphics ) ; 
    Ysafe_free( pathname ) ; /* free name created in Yrelpath */
    /* ############# end of compactor execution ############# */

    

    /* **************** READ RESULTS of compaction ************/
    /* open compaction file for writing */
    M( MSG, NULL, "Reading results of compaction...\n" ) ;
    sprintf(filename, "%s.mcpt" , cktNameG ) ;
    fp = TWOPEN( filename , "r", ABORT ) ;

    /* parse file */
    line = 0 ;
    abort = FALSE ;
    while( bufferptr=fgets(buffer,LRECL,fp )){
	/* parse file */
	line ++ ; /* increment line number */
	tokens = Ystrparser( bufferptr, ": \t\n", &numtokens );

	if( numtokens == 0 ){
	    /* skip over empty lines */
	    continue ;
	} else if( strcmp( tokens[0], INFOKEYWORD ) == STRINGEQ){
	    /* look at first field for keyword */
	    /* ie. numtiles:5 numcells:4 */
	    if( numtokens != 4 ){
		sprintf( YmsgG, "Syntax error on line:%d\n", line ) ;
		M(ERRMSG, "compact", YmsgG ) ;
		abort = TRUE ;
		continue ;
	    }
	    if( numcells != atoi(tokens[3] ) ){
		M(ERRMSG, "compact", "number of cells incorrect" ) ;
		abort = TRUE ;
	    }
	    if( abort ) break ; /* no sense in reading any longer */
	} else if( strcmp( tokens[0], CELLKEYWORD ) == STRINGEQ ||
		   strcmp( tokens[0], SCELLKEYWORD ) == STRINGEQ ){
		/* cell 1 x:312 y:512 offet:39 40 or */
		/* softcell 1 x:312 y:512 offet:39 40 */
	    /* look at first field for keyword */
	    if( numtokens != 9 ){
		sprintf( YmsgG, "Syntax error on line:%d\n", line ) ;
		M(ERRMSG, "compact", YmsgG ) ;
		abort = TRUE ;
		continue ;
	    }
	    cell = atoi( tokens[1] ) ;
	    ASSERTNCONT( cell > 0 && cell <= endsuperG, "compact",
		"cell out of bounds" ) ;
	    cellptr = cellarrayG[cell] ;
	    cellptr->xcenter = atoi(tokens[3] ) ;
	    cellptr->ycenter = atoi(tokens[5] ) ;

	} else if( strcmp( tokens[0], TILEKEYWORD ) == STRINGEQ){
	    /* look at first field for keyword */
	    /* l:-115 r:270 b:-85 t:85 */
	    if( numtokens != 8 ){
		sprintf( YmsgG, "Syntax error on line:%d\n", line ) ;
		M(ERRMSG, "compact", YmsgG ) ;
		abort = TRUE ;
		continue ;
	    }
	} else {
	    sprintf( YmsgG, "Syntax error on line:%d\n", line ) ;
	    M(ERRMSG, "compact", YmsgG ) ;
	    abort = TRUE ;
	    continue ;
	}
    }
    TWCLOSE( fp ) ;

    if( abort ){
	M(ERRMSG, "compact", "Problem with compaction. Must abort\n" ) ;
	closegraphics() ;
	YexitPgm( PGMFAIL ) ;
    }
    /* ************ END READ RESULTS of compaction ************/

} /* end compact */

#define HOWMANY 0

/* need accurate cell centers in density calculation */
get_cell_centers( cell, xc, yc )
INT cell ;
INT *xc, *yc ;
{
    INT last_core_cell ;

    last_core_cell = (INT) routingTilesG[HOWMANY] - 4 ;

    if( cell <= last_core_cell ){
	*xc = cellarrayG[cell]->xcenter ; 
	*yc = cellarrayG[cell]->ycenter ; 
    } else {
	/* pad macro */
	/* next line is equivalent to  */
	/* cell = endpadgrpsG + (cell - last_core_cell) */
	cell += endpadgrpsG - last_core_cell ;
	*xc = cellarrayG[cell]->xcenter ; 
	*yc = cellarrayG[cell]->ycenter ; 
    }
} /* end get_cell_centers */
