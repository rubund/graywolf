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
FILE:	    movestrat.c
DESCRIPTION:This file contains move strategy for compaction algorithm.
CONTENTS:   
DATE:	    Apr 11, 1988 
REVISIONS:  Oct 24, 1988 - fixed +1 error.  Now check for graph
		concurrency problem and now handle special case
		of plus overlap correctly.
	    Nov  6, 1988 - now keep search coordinates inside core.
	    Apr 18, 1989 - updated for multiple tile cells. Added
		update_cell_tiles.
	    Apr 25, 1989 - added debug function for position update.
	    Jun 21, 1989 - fixed infinite loop for multiple tiles.
	    Oct 10, 1990 - fixed typo in find_yerror.
	    Sun Feb 17 19:35:38 EST 1991 - fixed problem in find_bound
		when cells completely cover each other.  We now always
		return 0 for minslack in X and Y backward searches.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) movestrat.c version 7.2 2/17/91" ;
#endif

#include <compact.h>
#include <yalecad/debug.h>

    /* -------------------------------------------------------------- 
        space requirements also include 2 sources and 2 sinks: 
        xSource = 0, xsink = numtiles + 1, ysource = numtiles + 2,
	and ysink = numtiles + 3 positions in the tileNode array.
    */


static int find_bound();
static int findxerror();
static int findyerror();


moveStrategy( violations ) 
ERRORPTR violations ;
{
    COMPACTPTR tileL, tileR, tileB, tileT ;
    ECOMPBOXPTR edge ;
    ERRORPTR saveError ;
    /* static int find_bound(*/ /* tile, direction *//* ) ;*/
    int spaceAvail, location, tileWidth ;
    int xerror, yerror, delta, cell ;
    int searchl, searchb, searcht, searchr ;
    int i ;

    /* initialize moved cell array to false */
    for( i=1;i<=numtilesG;i++ ){
	tileNodeG[i]->moved = FALSE ;
    }

    /* APPLY RULE OF MINIMUM PERTUBATION */
    saveError = violations ; /* save for free below */
    for( ;violations; violations= violations->next ){

	if( tileNodeG[violations->nodeI]->moved ){
	    /* if this cell has already been moved we need to */
	    /* recalulate the graph continue with next violation */
	    continue ;
	} else {
	    tileNodeG[violations->nodeI]->moved = TRUE ;
	}
	if( tileNodeG[violations->nodeJ]->moved ){
	    /* if this cell has already been moved we need to */
	    /* recalulate the graph continue with next violation */
	    continue ;
	} else {
	    tileNodeG[violations->nodeJ]->moved = TRUE ;
	}

	/* assume tiles are sorted already */
	tileL = tileNodeG[violations->nodeI] ;
	tileB = tileNodeG[violations->nodeI] ;
	tileR = tileNodeG[violations->nodeJ] ;
	tileT = tileNodeG[violations->nodeJ] ;

	xerror = findxerror( &tileL, &tileR ) ;
	yerror = findyerror( &tileB, &tileT ) ;

	if( xerror <= -xspaceG || yerror <= -yspaceG ){
	    /* the problem has previously been fixed */
	    /* go on to next one */
	    continue ;
	}

	/* used to be 0.8 * yerror */
	if( xerror < yerror ){ /* x error */
	    /* try handling as 1-D case */
	    /* see which direction we have more room */
	    /* search to the left of the tile */
	    searchl = INT_MIN ;
	    if( tileL->multi ){
		/* substitute rightmost left tile */
		searchl = find_bound( tileL, tileR->cell, XBACKWARD ) ;
	    } else {
		/* single tile */
		for( edge=tileL->xadjB;edge;edge=edge->next ){
		    location = tileNodeG[edge->node]->r ;
		    searchl = MAX( searchl, location ) ;
		}
	    }
	    /* keep searchl inside the core */
	    searchl = MAX( searchl, blocklG ) ;
	    /* search to the right of the cell */
	    searchr = INT_MAX ;
	    if( tileR->multi ){
		searchr = find_bound( tileR, tileL->cell, XFORWARD ) ;
	    } else {
		for( edge=tileR->xadjF;edge;edge=edge->next ){
		    location = tileNodeG[edge->node]->l ;
		    searchr = MIN( searchr, location ) ;
		}
	    }
	    /* keep searchr inside the core */
	    searchr = MIN( searchr, blockrG ) ;
	    tileWidth = tileL->r - tileL->l + tileR->r - tileR->l + 2*xspaceG ;
	    spaceAvail = searchr - searchl ;
	    if( spaceAvail > tileWidth ){
		/* we can resolve the problem */
		/* we resolve by traveling left to right */
		/* update cell positions */
		if( tileL->l <= searchl ){
		    delta = searchl - tileL->l + xspaceG ;
		    cell = tileL->cell ;
		    update_cell_tiles( cell, delta, 0 ) ;
		} else if( tileWidth + tileL->l > searchr ){
		    /* above line is same as :
			tileL->r + tileR->r -tileR->l > searchr)
			don't move tileL if not necessary
		    */
		    /* tileL needs to move so tileR will fit */
		    delta = searchr - (tileWidth + tileL->l) ;
		    cell = tileL->cell ;
		    update_cell_tiles( cell, delta, 0 ) ;
		}
		ASSERT( tileR->l <= tileL->r + xspaceG,"movestrat",
		    "Logic error") ;
		delta = tileL->r - tileR->l + xspaceG ;
		cell = tileR->cell ;
		update_cell_tiles( cell, delta, 0 ) ;
	    } else {
		/* --------------------------------------------- 
		    Always push cells to the right. This will
		    guarantee that overlap will always be removed. 
		    Although this will have poor worst case time
		    behavior, we want to insure minimum perturbation
		    to the layout at this time.  When all violations
		    are removed we will compact.
		*/
		cell = tileR->cell ;
		if( xerror == 0 ){ /* always move at least one space */
		    xerror = xspaceG ;
		}
		delta = xerror + xspaceG ;
		update_cell_tiles( cell, delta, 0 ) ;
	    }

	} else if( yerror <= xerror ){ /* y error */
	    /* try handling as 1-D case */
	    /* see which direction we have more room */
	    /* search to the bottom of the cell */
	    searchb = INT_MIN ;
	    if( tileB->multi ){
		searchb = find_bound( tileB, tileT->cell, YBACKWARD ) ;
	    } else {
		for( edge=tileB->yadjB;edge;edge=edge->next ){
		    location = tileNodeG[edge->node]->t ;
		    searchb = MAX( searchb, location ) ;
		}
	    }
	    /* keep searchb inside the core */
	    searchb = MAX( searchb, blockbG ) ;
	    /* search to the top of the cell */
	    searcht = INT_MAX ;
	    if( tileT->multi ){
		searcht = find_bound( tileT, tileB->cell, YFORWARD ) ;
	    } else {
		for( edge=tileT->yadjF;edge;edge=edge->next ){
		    location = tileNodeG[edge->node]->b ;
		    searcht = MIN( searcht, location ) ;
		}
	    }
	    /* keep searcht inside the core */
	    searcht = MIN( searcht, blocktG ) ;
	    tileWidth = tileB->t - tileB->b + tileT->t - tileT->b + 2*yspaceG ;
	    spaceAvail = searcht - searchb ;
	    if( spaceAvail > tileWidth ){
		/* we can resolve the problem */
		/* we resolve by traveling left to right */
		/* update cell positions */
		if( tileB->b <= searchb ){
		    /* tileB in error - def. need to move it */
		    delta = searchb - tileB->b + yspaceG ;
		    cell = tileB->cell ;
		    update_cell_tiles( cell, 0, delta ) ;
		} else if( tileWidth + tileB->b > searcht ){
		    /* above line is same as :
			tileB->t + tileT->t -tileT->b > searcht)
			don't move tileB if not necessary
		    */
		    /* tileB needs to move so tileT will fit */
		    delta = searcht - (tileWidth + tileB->b ) ;
		    cell = tileB->cell ;
		    update_cell_tiles( cell, 0, delta ) ;
		}
		ASSERT( tileT->b <= tileB->t + yspaceG,"movestrat",
		    "Logic error") ;
		delta = tileB->t - tileT->b + yspaceG ;
		cell = tileT->cell ;
		update_cell_tiles( cell, 0, delta ) ;
	    } else {
		/* --------------------------------------------- 
		   Always push cells to the top. See above comments.
		*/
		cell = tileT->cell ;
		if( yerror == 0 ){ /* always move at least one space */
		    yerror = yspaceG ;
		}
		delta = yerror + yspaceG ;
		update_cell_tiles( cell, 0, delta ) ;
	    }

	}

    } /* end for loop - done processing violations */

    /* free violations */
    /* free all the edges */
    for(violations=saveError;violations; ){ 
	saveError = violations ;
	violations = violations->next ;
	Ysafe_free(saveError) ;
    }

} /* end moveStrategy */

/* HOW to update the tiles of the cells */
update_cell_tiles( cell, deltax, deltay ) 
int cell, deltax, deltay ;
{
    CELLBOXPTR cellptr ;
    COMPACTPTR t ;
    NODEPTR    tileptr ;
    int tile ;

    ASSERT( cell > 0 && cell <= numcellsG, "update_cell_tiles",
	"cell out of bounds" ) ;
    cellptr = cellarrayG[cell] ;
    cellptr->xcenter += deltax ;
    cellptr->ycenter += deltay ;
    cellptr->l += deltax ;
    cellptr->r += deltax ;
    cellptr->b += deltay ;
    cellptr->t += deltay ;
    /* go thru all the tiles of cell updating position */
    for( tileptr = cellptr->tiles;tileptr;tileptr=tileptr->next ){
	tile = tileptr->node ;
	ASSERT( tile > 0 && tile <= numtilesG, "update_cell_tiles",
	    "tile out of bounds" ) ;
	t = tileNodeG[tile] ;
	t->l += deltax ;
	t->r += deltax ;
	t->b += deltay ;
	t->t += deltay ;
	t->moved = TRUE ;
    } /* end delta update */

    D( "mc_compact/update_cell_tiles", dcheck_pos( cell ) ) ;

} /* end update_cell_tiles */

BOOL dcheck_pos( cell ) 
int cell ;
{

#ifdef DEBUG
    CELLBOXPTR cptr ;
    COMPACTPTR t ;
    NODEPTR    tileptr ;
    int tile, xc, yc ;

    cptr = cellarrayG[cell] ;
    xc = cptr->xcenter ;
    yc = cptr->ycenter ;
    ASSERT( cptr->l == xc + cptr->l_rel, "dcheck_pos", "cptr->l error\n");
    ASSERT( cptr->r == xc + cptr->r_rel, "dcheck_pos", "cptr->r error\n");
    ASSERT( cptr->b == yc + cptr->b_rel, "dcheck_pos", "cptr->b error\n");
    ASSERT( cptr->t == yc + cptr->t_rel, "dcheck_pos", "cptr->t error\n");
    /* go thru all the tiles of cell updating position */
    for( tileptr = cptr->tiles;tileptr;tileptr=tileptr->next ){
	tile = tileptr->node ;
	ASSERT( tile > 0 && tile <= numtilesG, "dcheck_pos",
	    "tile out of bounds" ) ;
	t = tileNodeG[tile] ;
	ASSERT( t->l == xc + t->l_rel, "dcheck_pos", "t->l error\n");
	ASSERT( t->r == xc + t->r_rel, "dcheck_pos", "t->r error\n");
	ASSERT( t->b == yc + t->b_rel, "dcheck_pos", "t->b error\n");
	ASSERT( t->t == yc + t->t_rel, "dcheck_pos", "t->t error\n");
    } /* end tile check */
#endif /* DEBUG */
    
} /* end dcheck_pos */

static int find_bound( tile, avoid, direction )
COMPACTPTR tile ;
int avoid ;    /* avoid finding tile that you have error (cell #) */
int direction ;
{

    int cell ;         /* current cell of given tile */
    int slack ;        /* slack for this tile pair */
    int minslack ;     /* minimum slack over all constraints */
    ECOMPBOXPTR edge ; /* current adjacent edge */
    COMPACTPTR  tptr ; /* current tile box in cell of interest */
    COMPACTPTR  tptr2; /* tile box for another cell */
    NODEPTR tiles ;    /* look at the tiles of the given cell */

    minslack = INT_MAX ;
    cell = tile->cell ;

    /* look at each tile of the cell an find the minimum that it can move */
    for( tiles = cellarrayG[cell]->tiles ;tiles; tiles = tiles->next ){
	tptr = tileNodeG[tiles->node] ;
	switch( direction ){
	    case XFORWARD:
		for( edge = tptr->xadjF; edge ; edge=edge->next ){
		    tptr2 = tileNodeG[edge->node] ;
		    if( tptr2->cell != avoid && tptr2->cell != cell ){
			slack = tptr2->l - tptr->r - xspaceG ;
			minslack = MIN( slack, minslack ) ;
		    }
		}
		break ;
	    case XBACKWARD:
		for( edge = tptr->xadjB; edge ; edge =edge->next ){
		    tptr2 = tileNodeG[edge->node] ;
		    if( tptr2->cell != avoid && tptr2->cell != cell ){
			slack = tptr->l - tptr2->r - xspaceG ;
			minslack = MIN( slack, minslack ) ;
		    }
		}
		break ;
	    case YFORWARD:
		for( edge = tptr->yadjF; edge ; edge=edge->next ){
		    tptr2 = tileNodeG[edge->node] ;
		    if( tptr2->cell != avoid && tptr2->cell != cell ){
			slack = tptr2->b - tptr->t - yspaceG ;
			minslack = MIN( slack, minslack ) ;
		    }
		}
		break ;
	    case YBACKWARD:
		for( edge = tptr->yadjB; edge ; edge=edge->next ){
		    tptr2 = tileNodeG[edge->node] ;
		    if( tptr2->cell != avoid && tptr2->cell != cell ){
			slack = tptr->b - tptr2->t - yspaceG ;
			minslack = MIN( slack, minslack ) ;
		    }
		}
		break ;
	} /* end switch */
    }

    /* at list point slack has been calculated */
    /* don't allow negative slack - this means tiles overlap */
    if(  minslack < 0 ){
	minslack = 0 ;
    }
    /* this occurs when the cell is at the edge of the core */
    /* give to the edge of the core region in forward directions */
    if( minslack == INT_MAX ){
	switch( direction ){
	    case XFORWARD:
		minslack = (INT) ( 0.25 * (DOUBLE) ABS(blockrG-blocklG)) ;
		break ;
	    case XBACKWARD:
		minslack = 0 ;
		break ;
	    case YFORWARD:
		minslack = (INT) ( 0.25 * (DOUBLE) ABS(blocktG-blockbG)) ;
		break ;
	    case YBACKWARD:
		minslack = 0 ;
		break ;
	} /* end switch */
    }

    /* now calculate how far the tile can move based on slack */
    switch( direction ){
	case XFORWARD:
	    return( tile->r + minslack ) ;
	case XBACKWARD:
	    return( tile->l - minslack ) ;
	case YFORWARD:
	    return( tile->t + minslack ) ;
	case YBACKWARD:
	    return( tile->b - minslack ) ;
    } /* end switch */

} /* end find_bound */

static int findxerror( tileL_addr, tileR_addr )
COMPACTPTR *tileL_addr, *tileR_addr ;
{
    int x1 ;                         /* xerror */
    int x2 ;                         /* xerror */
    int cell ;                       /* current cell */
    int slack ;                      /* distance from tileL */
    int xerror ;                     /* overlap in x direction */
    int overlap ;                    /* > 0 for overlap */
    int maxslack ;                   /* the furthest from tileL */
    COMPACTPTR tileL ;               /* left tile */
    COMPACTPTR tileR ;               /* right tile */
    COMPACTPTR tempTile ;            /* current tile */
    COMPACTPTR multiL ;              /* rightmost tile of left cell */
    COMPACTPTR multiR ;              /* leftmost tile of right cell */
    CELLBOXPTR cell1 ;               /* assumed left cell */
    CELLBOXPTR cell2 ;               /* assumed right cell */
    ECOMPBOXPTR edge ;

    /* assume tiles are correctly determined left and right */
    /* we will verify and switch below */
    tileL = *tileL_addr ;
    tileR = *tileR_addr ;
    cell1 = cellarrayG[tileL->cell] ;
    cell2 = cellarrayG[tileR->cell] ;

    /* determine left and right cells */
    overlap = projectX( cell1->l, cell1->r, cell2->l, cell2->r ) ;
    switch( overlap ){
    case OVERLAP1:
	/* --------------------------------------------------------
		+--------------------------------+
		|     +---------------------+    |
		|cell |      cell 1         |    |
		| 2   +---------------------+    |
		+--------------------------------+
	  cell 1 is completely inside cell 2 find smaller dimension 
	 ----------------------------------------------------------- */
	x1 = cell1->r - cell2->l ;  /* move 1 to left if smaller */
	x2 = cell2->r - cell1->l ;  /* move 1 to right if smaller */
	if( x1 <= x2 ){
	    xerror = x1 ;
	} else {
	    xerror = x2 ;
	    /* need to swap cells */
	    tileL = *tileR_addr ;
	    tileR = *tileL_addr ;
	}
	break ;
    case OVERLAP2:
	/* --------------------------------------------------------
		+--------------------------------+
		|                  +-------------|--------+ 
		|    cell          |    cell 2   |        |
		|     1            +-------------|--------+  
		+--------------------------------+
	  cell two is to right of cell 1 no need to swap
	 ----------------------------------------------------------- */
	xerror = cell1->r - cell2->l ;
	break ;
    case OVERLAP3:
	/* --------------------------------------------------------
    	        	+--------------------------------+
	          +-----|----------------+               |
	          |     |  cell 2        |   cell 1      |
        	  +-----|----------------+               | 
        		+--------------------------------+
	  cell one is to right of cell 1 need to swap
	 ----------------------------------------------------------- */
	/* need to swap cells */
	tileL = *tileR_addr ;
	tileR = *tileL_addr ;
	xerror = cell2->r - cell1->l ;
	break ;
    } /* end switch on overlap of cells */

    /* now find the tiles will help us remove the violations the most */
    multiL = NULL ;
    multiR = NULL ;
    if( tileL->multi ){
	/* search right for constraints within same cell */
	/* and other cell overlapping with constraints */
	/* find the rightmost tile touching tileR */
	/* it will become the new tile to be moved */
	cell = tileL->cell ;
	maxslack = 0 ;
	for( edge=tileL->xadjF;edge;edge=edge->next ){
	    tempTile = tileNodeG[ edge->node ] ;
	    if( tempTile->cell == cell ){
		overlap = projectY( tempTile->b, tempTile->t,
		    tileR->b, tileR->t ) ;
		if( overlap > 0 ){
		    slack = tempTile->r - tileL->r ;
		    if( slack > maxslack ){
			maxslack = slack ;
			multiL = tempTile ;
		    }
		}
	    }
	}
    }
    if( tileR->multi ){
	/* search right for constraints within same cell */
	/* and other cell overlapping with constraints */
	/* find the leftmost tile touching tileL */
	/* it will become the new tile to be moved */
	cell = tileR->cell ;
	maxslack = 0 ;
	for( edge=tileR->xadjB;edge;edge=edge->next ){
	    tempTile = tileNodeG[ edge->node ] ;
	    if( tempTile->cell == cell ){
		overlap = projectY( tempTile->b, tempTile->t,
		    tileL->b, tileL->t ) ;
		if( overlap ){
		    slack = tileR->l - tempTile->l ;
		    if( slack > maxslack ){
			maxslack = slack ;
			multiR = tempTile ;
		    }
		}
	    }
	}
    }

    /* now return the tiles */
    if( multiL ){
	*tileL_addr = multiL ;
    } else {
	*tileL_addr = tileL ;
    }
    if( multiR ){
	*tileR_addr = multiR ;
    } else {
	*tileR_addr = tileR ;
    }
    return( xerror ) ;
} /* end findxerror */

static int findyerror( tileB_addr, tileT_addr )
COMPACTPTR *tileB_addr, *tileT_addr ;
{
    int y1 ;                         /* yerror */
    int y2 ;                         /* yerror */
    int cell ;                       /* current cell */
    int slack ;                      /* distance from tileB */
    int yerror ;                     /* overlap in y direction */
    int overlap ;                    /* > 0 for overlap */
    int maxslack ;                   /* the furthest from tileB */
    COMPACTPTR tileB ;               /* bottom tile */
    COMPACTPTR tileT ;               /* top tile */
    COMPACTPTR tempTile ;            /* current tile */
    COMPACTPTR multiB ;              /* topmost tile of bottom cell */
    COMPACTPTR multiT ;              /* bottommost tile of top cell */
    CELLBOXPTR cell1 ;               /* assumed bottom cell */
    CELLBOXPTR cell2 ;               /* assumed top cell */
    ECOMPBOXPTR edge ;

    /* assume tiles are correctly determined bottom and top */
    /* we will verify and switch below */
    tileB = *tileB_addr ;
    tileT = *tileT_addr ;
    cell1 = cellarrayG[tileB->cell] ;
    cell2 = cellarrayG[tileT->cell] ;

    /* determine bottom and top cells */
    overlap = projectY( cell1->b, cell1->t, cell2->b, cell2->t ) ;
    /* see comments for findxerror */
    switch( overlap ){
    case OVERLAP1:
	y1 = cell1->t - cell2->b ;  /* move 1 to bottom if smaller */
	y2 = cell2->t - cell1->b ;  /* move 1 to top if smaller */
	if( y1 <= y2 ){
	    yerror = y1 ;
	} else {
	    yerror = y2 ;
	    /* need to swap cells */
	    tileB = *tileT_addr ;
	    tileT = *tileB_addr ;
	}
	break ;
    case OVERLAP2:
	yerror = cell1->t - cell2->b ;
	break ;
    case OVERLAP3:
	/* need to swap cells */
	tileB = *tileT_addr ;
	tileT = *tileB_addr ;
	yerror = cell2->t - cell1->b ;
	break ;
    } /* end switch on overlap of cells */

    /* now find the tiles will help us remove the violations the most */
    multiB = NULL ;
    multiT = NULL ;
    if( tileB->multi ){
	/* search upward for constraints within same cell */
	/* and other cell overlapping with constraints */
	/* find the topmost tile touching tileB */
	/* it will become the new tile to be moved */
	cell = tileB->cell ;
	maxslack = 0 ;
	for( edge=tileB->yadjF;edge;edge=edge->next ){
	    tempTile = tileNodeG[ edge->node ] ;
	    if( tempTile->cell == cell ){
		overlap = projectX( tempTile->l, tempTile->r,
		    tileT->l, tileT->r ) ;
		if( overlap > 0 ){
		    slack = tempTile->t - tileB->t ;
		    if( slack > maxslack ){
			maxslack = slack ;
			multiB = tempTile ;
		    }
		}
	    }
	}
    }
    if( tileT->multi ){
	/* search downward for constraints within same cell */
	/* and other cell overlapping with constraints */
	cell = tileT->cell ;
	maxslack = 0 ;
	for( edge=tileT->yadjB;edge;edge=edge->next ){
	    tempTile = tileNodeG[ edge->node ] ;
	    if( tempTile->cell == cell ){
		overlap = projectX( tempTile->l, tempTile->r,
		    tileB->l, tileB->r ) ;
		if( overlap > 0 ){
		    slack = tileT->b - tempTile->b ;
		    if( slack > maxslack ){
			maxslack = slack ;
			multiT = tempTile ;
		    }
		}
	    }
	}
    }

    /* now return the tiles */
    if( multiB ){
	*tileB_addr = multiB ;
    } else {
	*tileB_addr = tileB ;
    }
    if( multiT ){
	*tileT_addr = multiT ;
    } else {
	*tileT_addr = tileT ;
    }
    return( yerror ) ;
} /* end findyerror */
