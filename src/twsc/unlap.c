/*
 *   Copyright (C) 1989-1990 Yale University
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
FILE:	    unlap.c                                       
DESCRIPTION:remove overlap between cells.
CONTENTS:   unlap(flag)
		INT flag ;
DATE:	    Mar 27, 1989 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef VMS
#ifndef lint
static char SccsId[] = "@(#) unlap.c (Yale) version 4.5 10/14/90" ;
#endif
#endif

#include "standard.h"
#include "groute.h"

/* global variables */
BOOL noPairsG ;

/* global references */
extern INT actual_feed_thru_cells_addedG ;
extern INT extra_cellsG ;
extern BOOL no_feed_at_endG ;
extern BOOL rigidly_fixed_cellsG ;

unlap(flag)
INT flag ;
{

CBOXPTR cellptr ;
INT *num , i , cell_count , last , row , current , limit ;
INT cell , block , k ;
INT cell_left , left_edge , right_edge ;
INT fixed , unfixed ;
INT *left_queue , *right_queue , *center_queue ;
INT max_cell_in_blk ;
INT min_right_fixed_cell ;
INT max_left_fixed_cell ;
INT pair_array_index ;
INT comparex() ;

max_cell_in_blk = 0 ;

num = (INT *) Ysafe_malloc( ( numRowsG + 1 ) * sizeof(INT) ) ;
for( block = 0 ; block <= numRowsG ; block++ ) {
    num[block] = 0 ;
}

for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    if( carrayG[cell]->clength > 0 &&
		carrayG[cell]->cycenter != GATE_ARRAY_MAGIC_CONSTANT ) {
	num[ carrayG[cell]->cblock ]++ ;
    }
}
k = numcellsG + numtermsG + actual_feed_thru_cells_addedG ;
for( cell = numcellsG + numtermsG + 1 ; cell <= k ; cell++ ) {
    num[ carrayG[cell]->cblock ]++ ;
}

for( block = 1 ; block <= numRowsG ; block++ ) {
    if( num[block] > max_cell_in_blk ) {
	max_cell_in_blk = num[block] ;
    }
}

pairArrayG = (INT **) Ysafe_malloc( ( numRowsG + 1 ) * sizeof(INT *) ) ;
for( block = 1 ; block <= numRowsG ; block++ ) {
    limit = 5 * max_cell_in_blk + (extra_cellsG / numRowsG) * 4 ;
    pairArrayG[ block ] = (INT *) Ysafe_malloc( limit * sizeof( INT ) ) ;
    pairArrayG[block][0] = num[block] ;
}
left_queue = (INT *) Ysafe_malloc((max_cell_in_blk + 1) * sizeof(INT));
right_queue = (INT *) Ysafe_malloc((max_cell_in_blk + 1) * sizeof(INT));
center_queue = (INT *) Ysafe_malloc((max_cell_in_blk + 1) * sizeof(INT));

for( cell = 1 ; cell <= numcellsG ; cell++ ) {
    if( carrayG[cell]->clength > 0 &&
		carrayG[cell]->cycenter != -1000001 ) {
	block = carrayG[cell]->cblock ;
	pairArrayG[block][ num[block]-- ] = cell ;
    }
}
k = numcellsG + numtermsG + actual_feed_thru_cells_addedG ;
for( cell = numcellsG + numtermsG + 1 ; cell <= k ; cell++ ) {
    block = carrayG[cell]->cblock ;
    pairArrayG[block][ num[block]-- ] = cell ;
}


for( block = 1 ; block <= numRowsG ; block++ ) {
    Yquicksort( (char *) ( pairArrayG[block] + 1 ) , 
	pairArrayG[block][0] , sizeof( INT ), comparex ) ;
}

/* ------- Guarantees fixed-cell orderings now ------- */

for( block = 1 ; block <= numRowsG ; block++ ) {
    left_edge  = barrayG[block]->bxcenter + barrayG[block]->bleft ;
    right_edge = barrayG[block]->bxcenter + barrayG[block]->bright ;
    if( ( cell_count = pairArrayG[ block ][0] ) > 1 ) {
	noPairsG = 0 ;
    }
    fixed   = FALSE ;
    unfixed = FALSE ;

    for( i = 1 ; i <= cell_count ; i++ ) {
	if( carrayG[pairArrayG[block][i]]->cclass < 0 ) {
	    fixed = TRUE ;
	} else {
	    unfixed = TRUE ;
	}
    }
    if( !rigidly_fixed_cellsG && fixed == TRUE && unfixed == TRUE ) {
	for( i = 0 ; i <= max_cell_in_blk ; i++ ) {
	    left_queue[i]   = 0 ;
	    right_queue[i]  = 0 ;
	    center_queue[i] = 0 ;
	}
	max_left_fixed_cell = 0 ;
	for( i = 1 ; i <= cell_count ; i++ ) {
	    cellptr = carrayG[ pairArrayG[block][i] ] ;
	    if( cellptr->cclass < 0 ) {   /* fixed left */
		if( left_edge == cellptr->cxcenter +
		      cellptr->tileptr->left ) {
		    left_edge += cellptr->clength ;
		    left_queue[++(*left_queue)] = pairArrayG[block][i];
		    max_left_fixed_cell = i ;
		} else {
		    break ;
		}
	    }
	}
	min_right_fixed_cell = cell_count + 1 ;
	for( i = cell_count ; i >= 1 ; i-- ) {
	    cellptr = carrayG[ pairArrayG[block][i] ] ;
	    if( cellptr->cclass < 0 ) {   /* fixed right */
		if( right_edge == cellptr->cxcenter +
		      cellptr->tileptr->right ) {
		    right_edge -= cellptr->clength ;
		    right_queue[++(*right_queue)] = pairArrayG[block][i];
		    min_right_fixed_cell = i ;
		} else {
		    break ;
		}
	    }
	}
	for( i = 1 ; i <= cell_count ; i++ ) {
	    cellptr = carrayG[ pairArrayG[block][i] ] ;
	    if( (cellptr->cclass >= 0) ||        /* unfixed */
		    (cellptr->cclass < 0 && i > max_left_fixed_cell &&
			i < min_right_fixed_cell)) { /* center-fixed */

		center_queue[++(*center_queue)] = pairArrayG[block][i];
	    }
	}
	pair_array_index = 0 ;
	for( i = 1 ; i <= *left_queue ; i++ ) {
	    pairArrayG[block][++pair_array_index] = left_queue[i] ;
	}
	for( i = 1 ; i <= *center_queue ; i++ ) {
	    pairArrayG[block][++pair_array_index] = center_queue[i] ;
	}
	for( i = *right_queue ; i >= 1 ; i-- ) {
	    pairArrayG[block][++pair_array_index] = right_queue[i] ;
	}
    }
    left_edge  = barrayG[block]->bxcenter + barrayG[block]->bleft ;
    for( i = 1 ; i <= cell_count ; i++ ) {
	cellptr = carrayG[ pairArrayG[block][i] ] ;
        cell_left = cellptr->tileptr->left ;
	if( flag == 2 && cellptr->cxcenter != left_edge - cell_left ) {
	    printf("ERROR cell %d\n", pairArrayG[block][i] );
	}
        cellptr->cxcenter = left_edge - cell_left ;
	left_edge += cellptr->tileptr->right - cell_left ;
    }
}
Ysafe_free( num ) ;
Ysafe_free( left_queue ) ;
Ysafe_free( right_queue ) ;
Ysafe_free( center_queue ) ;

/* xcompact() ; */

#ifdef OLD_CRAP
if( no_feed_at_end && flag == 1 ) {
    for( row = 1 ; row <= numRowsG ; row++ ) {
	last = pairArrayG[row][0] ;
	carrayG[ pairArrayG[row][1] ]->cclass = -2 ;
	for( current = 2 ; current <= last ; current++ ) {
	    if( carrayG[ pairArrayG[row][current] ]->cclass >= 0 ) {
		break ;
	    }
	    carrayG[ pairArrayG[row][current] ]->cclass = -2 ;
	}
	carrayG[ pairArrayG[row][last] ]->cclass = -3 ;
	for( current = last - 1 ; current >= 1 ; current-- ) {
	    if( carrayG[ pairArrayG[row][current] ]->cclass >= 0 ) {
		break ;
	    }
	    carrayG[ pairArrayG[row][current] ]->cclass = -3 ;
	}
    }
}
#endif

return ;
}
