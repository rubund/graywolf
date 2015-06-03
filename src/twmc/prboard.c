/*
 *   Copyright (C) 1988-1991 Yale University
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
FILE:	    prboard.c                                       
DESCRIPTION:print board routine
CONTENTS:   config1( )
DATE:	    Jan 30, 1988 
REVISIONS:  Jul 30, 1988 - fixed output for softcells and added
		gridding of cells for mighty interface.
	    Mon Jan  7 18:31:00 CST 1991 - don't grid pads.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) prboard.c version 3.6 4/18/91" ;
#endif

#include <custom.h>
#include <yalecad/debug.h>

grid_cells()
{

INT xcenter , ycenter, remainder ;
INT cell ;
INT old_left, old_bottom ;/* original xy center of cell before gridding */
INT delta_x, delta_y ;       /* delta to move cell lower left to grid */
INT left, right, bottom, top ;            /* sides of cell bounding box */
CELLBOXPTR cellptr ;
BOUNBOXPTR bounptr ;

    if(!(gridCellsG)){
	return ;
    }
    for( cell = 1 ; cell <= endsuperG ; cell++ ) {
	cellptr = cellarrayG[ cell ] ;
	if( cellptr->celltype == GROUPCELLTYPE ||
	    cellptr->celltype == SUPERCELLTYPE ) {
	    /* avoid these cell types */
	    continue ;
	}
	bounptr = cellptr->bounBox[cellptr->orient] ;
	xcenter = cellptr->xcenter ;
	ycenter = cellptr->ycenter ;

	left = bounptr->l ;
	right = bounptr->r ;
	bottom = bounptr->b ;
	top = bounptr->t ;

	/* add offset */
	old_left = left += xcenter ;
	old_bottom = bottom += ycenter ;

	/* grid cell data if requested */
	/* set lower left corner to grid */
	YforceGrid( &left, &bottom ) ;

	/* now modify center of cell coordinates */
	delta_x = left - old_left ;
	delta_y = bottom - old_bottom ;
	cellptr->xcenter += delta_x ;
	cellptr->ycenter += delta_y ;

    } /* end for loop */

    /* update all the costs */
    funccostG = findcost() ;
    
} /* end grid_cells */
