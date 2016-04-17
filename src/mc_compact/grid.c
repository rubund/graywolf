/* ----------------------------------------------------------------- 
FILE:	    grid.c                                       
DESCRIPTION:grid the data to user specified grid.
CONTENTS:   grid_data( )
DATE:	    Apr 18, 1989 - origin prboard.c
REVISIONS:  Apr 30, 1989 - removed unnecessary variables.
	    Mon May  6 22:33:51 EDT 1991 - no longer grid cells.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) grid.c version 7.2 5/6/91" ;
#endif

#include <compact.h>
#include <yalecad/debug.h>

grid_data()
{

    int xcenter , ycenter, remainder ;
    int cell, i ;
    int old_left, old_bottom ;/* orig xy center of cell before gridding */
    int delta_x, delta_y ;     /* delta to move cell lower left to grid */
    int left, bottom ;         /* sides of cell bounding box */
    CELLBOXPTR cellptr ;
    COMPACTPTR t ;

#ifdef LATER
    
    for( cell = 1 ; cell <= numcellsG ; cell++ ) {
	cellptr = cellarrayG[ cell ] ;

	/* get global coordinates */
	old_left = left = cellptr->xoffset + cellptr->xcenter ;
	old_bottom = bottom = cellptr->yoffset + cellptr->ycenter ;

	/********* grid cell data *************/
	/* set lower left corner to grid */
	if( xgridG ){ /* only do if non zero */
	    /* do individual tests only one may be zero */
	    if( remainder = left % xgridG ){
		/* fix x direction */
		left += xgridG - remainder ;
	    }
	}
	if( ygridG ){ /* only do if non zero */
	    if( remainder = bottom % ygridG ){
		/* fix y direction */
		bottom += ygridG - remainder ;
	    }
	}
	/* now modify center of cell coordinates */
	delta_x = left - old_left ;
	if( delta_x ){
	    fprintf( stderr, "Gridding cell %d in x direction...\n", cell ) ;
	}
	if( delta_y ){
	    fprintf( stderr, "Gridding cell %d in y direction...\n", cell ) ;
	}
	delta_y = bottom - old_bottom ;
	xcenter = cellptr->xcenter += delta_x ;
	ycenter = cellptr->ycenter += delta_y ;
	/* now modify bounding box of cell */
	cellptr->l = cellptr->l_rel + xcenter ;
	cellptr->r = cellptr->r_rel + xcenter ;
	cellptr->b = cellptr->b_rel + ycenter ;
	cellptr->t = cellptr->t_rel + ycenter ;
        /********  end cell gridding ****** */
    } /* end for loop */

    /* now update all the tiles */
    for( i = 1; i <= numtilesG; i++ ){
	t = tileNodeG[i] ;
	cell = t->cell ;
	ASSERTNCONT( cell > 0 && cell <= numcellsG,"grid_data",
	    "cell out of bounds" ) ;
	cellptr = cellarrayG[cell] ;
	xcenter = cellptr->xcenter ;
	ycenter = cellptr->ycenter ;
	t->l = t->l_rel + xcenter ;
	t->r = t->r_rel + xcenter ;
	t->b = t->b_rel + ycenter ;
	t->t = t->t_rel + ycenter ;
    } /* end update of tiles */
#endif

    
} /* end grid_cells */
