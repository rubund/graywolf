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
#include <globals.h>

#define COMPACT_DEFS
#include "compact.h"
#include "compactor.h"
#include "cdraw.h"
#include "changraph.h"
#include "grid.h"
#include "stdmacro.h"
#include "io.h"

int readtiles(char *filename);
int read_cgraph(char *filename);

int
__attribute__((visibility("default")))
mc_compact(int a, int d, int c, int n, int p, int v, char*cktName,int blockr, int blockt, int track_spacingX, int track_spacingY)
{
	char filename[LRECL] ;

	graphicsG  = TRUE ;
	alignG     = FALSE ;
	compactG   = TRUE ; 
	partitionG = FALSE ;
	debugG     = FALSE ;
	constraintsG = FALSE ;
	if(a) {
		alignG = TRUE ;
	}
	if(d) {
		debugG = TRUE ;
	}
	if(c) {
		constraintsG = TRUE ;
	}
	if(n) {
		graphicsG = FALSE ;
	}
	if(p) {
		partitionG = TRUE ;
	}
	if(v) {
		compactG = FALSE ;
	}

#ifdef NOGRAPHICS
	graphicsG = FALSE ;
#endif

	YdebugMemory( debugG ) ;
	fprintf( stdout, "\n%s\n",YinitProgram(COMPACT,"v1.0",NULL) ) ;
	
	cktNameG = Ystrclone(cktName);
	blockrG = blockr ;
	blocktG = blockt ;
	xgridG = track_spacingX;
	ygridG = track_spacingY;
	xspaceG = track_spacingX ;
	yspaceG = track_spacingY;

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
	}
	M(MSG,NULL,"\n" ) ;

	/* ********************** end initialization ************************* */
	/* set core region */
	blocklG = 0 ;
	blockbG = 0 ;
	blockmxG = blockrG / 2 ;
	blockmyG = blocktG / 2 ;

	/* turn on the graphics if requested */
	sprintf( filename, "%s.mvio", cktNameG ) ;
	readtiles(filename) ;

	if( constraintsG ){
		/* read the channel graph */
		read_cgraph(filename) ;
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

	return 0;
} /* end main */
