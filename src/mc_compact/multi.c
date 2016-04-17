/*
 *   Copyright (C) 1990,1992 Yale University
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
FILE:	    multi.c
DESCRIPTION:This file contains routines for pre-processing of multi
	    tile cells.
CONTENTS:   
DATE:	    Jan 21, 1990 
REVISIONS:  
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) multi.c version 7.3 5/21/92" ;
#endif

#include <compact.h>
#include <yalecad/debug.h>
#include <yalecad/message.h>

typedef struct vertex_box {
    INT x ;
    INT y ;
    INT class ;
    struct vertex_box *next ;
} *VERTEXPTR, VERTEX_BOX ;

#define START 1
#define END 2
#define NOTHING 3

static ERRORPTR multiS = NULL ;  /* list of multiple tile adjacencies */
static VERTEXPTR vertex_listS = NIL( VERTEX_BOX *) ;  /* head of the list */


static preprocess_multi();
static BOOL fix_tiles();
static add_to_multi_list();
static BOOL find_tile();
static init_vertex_list();
static add_extra_points();
static swap();
static add_pt();
static free_vertex_list();



multi_tiles()
{
    preprocess_multi( TRUE ) ;
    final_tiles() ;
    preprocess_multi( FALSE ) ;
} /* end multi_tiles */

/* remove overlap between tiles of a multi-tile cell */
/* Since this operation is only done once, a reliable O(n2) algorithm */
/* is reasonable here */
static preprocess_multi( fixNotCheck )
BOOL fixNotCheck ;
{

    int i ;               /* counter */
    NODEPTR firstptr ;    /* the tiles belonging to this cell */
    NODEPTR aptr ;        /* current tile of cell a */
    NODEPTR bptr ;        /* current tile of cell b */
    COMPACTPTR tilea ;    /* info on tile a */
    COMPACTPTR tileb ;    /* info on tile b */

    if( fixNotCheck ){
	M( MSG, "preprocess_multi", "Fixing multiple tiles...\n" ) ;
    } else {
	M( MSG, "preprocess_multi", "Verifying multiple tiles...\n" ) ;
    }

    for( i=1;i<=numcellsG;i++ ){
	firstptr = cellarrayG[i]->tiles ;
	if( !(tileNodeG[firstptr->node]->multi) ){
	    /* the cell has only one tile */
	    continue ;
	}

TOP:
	for( aptr = cellarrayG[i]->tiles;aptr; aptr = aptr->next ){
	    ASSERTNCONT( aptr->node > 0 && aptr->node <= numtilesG,
		"preprocess_multi", "tile out of bounds\n" ) ;
	    tilea = tileNodeG[aptr->node] ;

	    for( bptr = aptr->next; bptr ; bptr = bptr->next ){
		ASSERTNCONT( bptr->node > 0 && bptr->node <= numtilesG,
		    "preprocess_multi", "tile out of bounds\n" ) ;
		tileb = tileNodeG[bptr->node] ;

		if(!(fixNotCheck)){
		    /* add multi tile structure during */
		    /* verification */
		    add_to_multi_list( tilea, tileb );
		}

		/* now check for overlap */
		if( projectY( tilea->b, tilea->t, tileb->b, tileb->t) ){

		    if( projectX( tilea->l,tilea->r,tileb->l,tileb->r )){

			if( fix_tiles( tilea, tileb ) ){
			    /* a goto here makes logic easier to understand */
			    /* if we modified the tile structure we need to */
			    /* look at all the tiles again. A very conservative */
			    /* method but it should not be that bad time wise */
			    /* since hopefully there are no overlaps */
			    goto TOP ;
			}
		    }
		}
	    }

	}

    } /********* end check of tiles ***********/

} /* end preprocess multi */

static BOOL fix_tiles( tilea, tileb )
COMPACTPTR tilea ;    /* info on tile a */
COMPACTPTR tileb ;    /* info on tile b */
{

    INT count ;           /* count the number of tiles */
    INT xerror ;          /* error in x direction */
    INT xerror_plus ;     /* error in plus configuration */
    INT yerror ;          /* error in y direction */
    INT yerror_plus ;     /* error in plus configuration */
    INT xc, yc ;          /* center of the given cell */
    INT llx,lly,urx,ury ; /* bounds of the tile */
    INT tile ;            /* tile added by add_tile */
    COMPACTPTR tileL ;    /* left tile*/
    COMPACTPTR tileR ;    /* right tile */
    COMPACTPTR tileB ;    /* bottom tile */
    COMPACTPTR tileT ;    /* top tile */
    COMPACTPTR tempTile ; /* for sorting tiles */
    CELLBOXPTR cptr ;     /* current cell */
    NODEPTR nptr ;	  /* current node */
    NODEPTR last_node ;	  /* the last node */

    /* now do some calculations */
    /* assume tiles are sorted already */
    tileL = tilea ;
    tileR = tileb ;
    tileB = tilea ;
    tileT = tileb ;

    /* sort tiles by bounding box xmin */
    if( tileL->l > tileR->l ){
	/* assumption wrong -  reverse */
	tempTile = tileL ;
	tileL = tileR ;
	tileR = tempTile ;
    }
    /* sort tiles by bounding box ymin */
    if( tileB->b > tileT->b ){
	/* assumption wrong -  reverse */
	tempTile = tileB ;
	tileB = tileT ;
	tileT = tempTile ;
    }

    /* determine where the problem is */
    xerror = tileL->r - tileR->l ; /* T overlap error */
    if( xerror <= 0 ){
	/* the cells are touching which is ok */
	return( FALSE ) ;
    }
    /**** end of xerror calculation ****/

    yerror = tileB->t - tileT->b ;
    if( yerror <= 0 ){
	/* the cells are touching which is ok */
	return( FALSE ) ;
    }
    /**** end of yerror calculation ****/

    sprintf( YmsgG, 
	"Multi-tile true overlap detected: cell %d tiles:%d %d\n",
	tilea->cell, tilea->node, tileb->node ) ;
    M( MSG, NULL, YmsgG ) ;

    free_vertex_list() ;
    init_vertex_list( tilea->l, tilea->b, tilea->r, tilea->t ) ;
    init_vertex_list( tileb->l, tileb->b, tileb->r, tileb->t ) ;
    add_extra_points() ;
    count = 0 ; 
    cptr = cellarrayG[tilea->cell] ;
    while( find_tile(&llx , &lly , &urx , &ury ) ) {
	xc = cptr->xcenter ;
	yc = cptr->ycenter ;
	if( ++count == 1 ){
	    /* assign it to tilea */
	    tempTile = tilea ;
	} else if( count == 2 ){
	    /* assign it to tileb */
	    tempTile = tileb ;
	} else {
	    /* need to create a new tile */
	    init_extra_tile( tilea->cell, tilea->type ) ;
	    addtile( llx - xc, urx - xc, lly - yc, ury - yc ) ;
	    continue ; /* add tile does the below work */
	}
	tempTile->l = llx ;
	tempTile->r = urx ;
	tempTile->b = lly ;
	tempTile->t = ury ;
	tempTile->l_rel = llx - xc ;
	tempTile->r_rel = urx - xc ;
	tempTile->b_rel = lly - yc ;
	tempTile->t_rel = ury - yc ;
	tempTile->xspan = urx - llx ;
	tempTile->yspan = ury - lly ;
    } 
    if( count == 1 ){
	/* we need to delete a tile from the tileNode etc. */
	last_node = NIL( NODEBOX * ) ;
	for(  nptr = cptr->tiles; nptr; nptr= nptr->next ){
	    if( nptr->node == tileb->node ){
		/* this is our target tile */
		if( last_node ){
		    /* we are in the middle or end of the list */
		    last_node->next = nptr->next ;
		} else {
		    /* we are the first tile in the list */
		    cptr->tiles = nptr->next ;
		}
		Ysafe_free( nptr ) ;
		Ysafe_free( tileb ) ;
		break ;
	    }
	    last_node = nptr ;     
	} /* end looking for the tile */
    }

    D( "mc_compact/fix_tiles",
	fprintf( stderr, "\tWe found %d tiles for this multi-tile\n", 
	count ) ;
    ) ;
    return( TRUE ) ;

} /* end fix tiles */


/* add to multi list saves a list of the nodes of a multi tile cell */
/* which are adjacent.  Since the tiles of a multi tile cell cannot */
/* move relative to one another. We can precompute the edges in the */
/* graphs.  Here we need only to save the node numbers */
static add_to_multi_list( tilea, tileb )
COMPACTPTR tilea ;    /* info on tile a */
COMPACTPTR tileb ;    /* info on tile b */
{
    ERRORPTR multi_tile ;  /* temp pointer */

    D( "mc_compact/add_to_multi_list",
    fprintf( stderr,"multi touch:%d -> %d\n",tilea->node,tileb->node));

    /* save multi tile adjacencies - add to multi list */
    if( multi_tile = multiS ){
	multiS = (ERRORPTR) Ysafe_malloc( sizeof(ERRORBOX) ) ;
	multiS->next = multi_tile ;
    } else {
	multiS = (ERRORPTR) Ysafe_malloc( sizeof(ERRORBOX) ) ;
	multiS->next = NULL ;
    }
    multiS->nodeI = tilea->node ;
    multiS->nodeJ = tileb->node ;

} /* end add_to_multi_list */

/* add the precomputed edges to the xgraph */
add_mtiles_to_xgraph()
{
    ERRORPTR multi_tile ;  /* temp pointer */
    COMPACTPTR nodeI, nodeJ ;     /* two adjacent nodes */

    for( multi_tile = multiS; multi_tile; multi_tile = multi_tile->next ){
	/* make sure to get ancestors correct */
	nodeI = tileNodeG[multi_tile->nodeI] ;
	nodeJ = tileNodeG[multi_tile->nodeJ] ;
	if( nodeI->l_rel < nodeJ->l_rel ){
	    formxEdge( nodeI->node, nodeJ->node ) ;
	} else if( nodeI->l_rel > nodeJ->l_rel ){
	    formxEdge( nodeJ->node, nodeI->node ) ;
	} else {
	    /* in the case that they are equal pick smaller node id */
	    if( nodeI->node < nodeJ->node ){
		formxEdge( nodeI->node, nodeJ->node ) ;
	    } else {
		formxEdge( nodeJ->node, nodeI->node ) ;
	    }
	}
    }
} /* end add_mtiles_to_xgraph() */

/* add the precomputed edges to the ygraph */
add_mtiles_to_ygraph()
{
    ERRORPTR multi_tile ;         /* temp pointer */
    COMPACTPTR nodeI, nodeJ ;     /* two adjacent nodes */

    for( multi_tile = multiS; multi_tile; multi_tile = multi_tile->next ){
	/* make sure to get ancestors correct */
	nodeI = tileNodeG[multi_tile->nodeI] ;
	nodeJ = tileNodeG[multi_tile->nodeJ] ;
	if( nodeI->b_rel < nodeJ->b_rel ){
	    formyEdge( nodeI->node, nodeJ->node ) ;
	} else if( nodeI->b_rel > nodeJ->b_rel ){
	    formyEdge( nodeJ->node, nodeI->node ) ;
	} else {
	    /* in the case that they are equal pick smaller node id */
	    if( nodeI->node < nodeJ->node ){
		formyEdge( nodeI->node, nodeJ->node ) ;
	    } else {
		formyEdge( nodeJ->node, nodeI->node ) ;
	    }
	}
    }
} /* end add_mtiles_to_ygraph() */


static BOOL find_tile( llx , lly , urx , ury )
INT *llx , *lly , *urx , *ury ;
{

    VERTEX_BOX *vertex , *lvertex , *rvertex , *tvertex ;
    INT lowest , left_edge , right_edge , top_edge ;
    INT x, y ;
    BOOL found ; 

    /* find the leftmost of the lowest START vertices */
    /* this is where I start to write slow, but effective algs */

TOP:
    lowest = INT_MAX ;
    left_edge = INT_MAX ;
    lvertex = NULL ;
    for(vertex = vertex_listS ;vertex ; vertex = vertex->next ){
	if( vertex->class == START ) {
	    if( vertex->y < lowest ) {
		lowest = vertex->y ;
		lvertex = vertex ;
		left_edge = vertex->x ;
	    } else if( vertex->y == lowest ) {
		if( vertex->x < left_edge ) {
		    left_edge = vertex->x ;
		    lvertex = vertex ;
		}
	    }
	}
    } /* end for loop */
    if(!(lvertex)) {
	return(FALSE);  /* no more tiles exist */
    }

    /* find the END vertex whose y is lowest */
    /* and whose x is the nearest to the right of left_edge */
    right_edge = INT_MIN ;
    rvertex = NULL ;
    for( vertex = vertex_listS;vertex;vertex = vertex->next ){
	if( vertex->class == END ) {
	    if( vertex->y == lowest ) {
		if( vertex->x >= left_edge ) {
		    if( vertex->x > right_edge ) {
			right_edge = vertex->x ;
			rvertex = vertex ;
		    }
		}
	    }
	}
    } /* end for loop */
    if(!(rvertex)) {
	lvertex->class = NOTHING ;
	goto TOP ;
    }

    /* now find ANY vertex whose x is between left_edge and */
    /* right_edge and whose y is greater than lowest */
    top_edge = INT_MAX ;
    tvertex = NULL ;
    for(vertex = vertex_listS ;vertex ; vertex = vertex->next ){
	if( vertex->x >= left_edge && vertex->x <= right_edge ) {
	    if( vertex->y > lowest ) {
		if( vertex->y < top_edge ) {
		    top_edge = vertex->y ;
		    tvertex = vertex ;
		}
	    }
	}
    } /* end for loop */

    if( tvertex == NULL || left_edge == right_edge || lowest == top_edge ) {
	lvertex->class = NOTHING ;
	if( rvertex ) {
	    rvertex->class = NOTHING ;
	}
	goto TOP ;
    }


    /* change classes to NOTHING for the selected vertices */
    lvertex->class = NOTHING ;
    rvertex->class = NOTHING ;

    /* here is the tile */
    *llx = left_edge ;
    *lly = lowest ;
    *urx = right_edge ;
    *ury = top_edge ;

    /* now mark any of the interior points marked start or end to be nothing */
    for(vertex = vertex_listS ;vertex ; vertex = vertex->next ){
	if( vertex->class == START ) {
	    x = vertex->x ;
	    y = vertex->y ;
	    /* any point inside the tile is nothing */
	    /*   llx <= x < urx && lly <= y < ury */
	    /* project upwards */
	    if( *llx <= x && x < *urx && *lly <= y && y < *ury ){
		vertex->y = *ury ;
	    }
	} /* end start test */
	if( vertex->class == END ) {
	    x = vertex->x ;
	    y = vertex->y ;
	    if( *llx < x && x <= *urx && *lly <= y && y < *ury ){
		vertex->y = *ury ;
	    }
	}
    } /* end for loop */


    /*  the generated tile has four vertices; if any of these vertices */
    /*  is not currently in the list (only the two top ones could be new) */
    /*  we need to add them */
    found = FALSE ;
    for(vertex = vertex_listS ;vertex ; vertex = vertex->next ){
	if( vertex->x == *llx && vertex->y == *ury ) {
	    found = TRUE ;
	    break ;
	}
    } /* end for loop */

    if( !found ) {
	add_pt( *llx, *ury, START ) ;
    }

    found = FALSE ;
    for( vertex = vertex_listS ;vertex ; vertex = vertex->next ){
	if( vertex->x == *urx && vertex->y == *ury ) {
	    found = TRUE ;
	    break ;
	}
    } /* end for loop */
    if( !found ) {
	add_pt( *urx, *ury, END ) ;
    }


    return(TRUE) ;
} /* end find_tile */

static init_vertex_list( left, bottom, right, top )
INT left, bottom, right, top ;
{
    /* add in inverse order so list will be in correct order */
    add_pt( right, bottom, END ) ;
    add_pt( right, top, NOTHING ) ;
    add_pt( left, top, NOTHING ) ;
    add_pt( left, bottom, START ) ;
} /* end init_vertex_list */

static add_extra_points()
{
    INT xpt[9] ;
    INT ypt[9] ;
    INT count ;
    INT i, j ;
    INT x1, x2, x3, x4 ;
    INT y1, y2, y3, y4 ;
    VERTEXPTR vertex ;

    /* first load arrays */
    count = 0 ;
    for( vertex = vertex_listS ;vertex ; vertex = vertex->next ){
	xpt[++count] = vertex->x ;
	ypt[count]   = vertex->y ;
    } /* end for loop */
    ASSERT( count == 8, "add_extra_points", "There should be only 8 points\n" ) ;

    /* now check places for extra points */
    for( i = 1; i <= 4; i++ ){
	/* get the line for first tile */
	x1 = xpt[i] ;
	y1 = ypt[i] ;
	if( i != 4 ){
	    x2 = xpt[i+1] ;
	    y2 = ypt[i+1] ;
	} else {
	    x2 = xpt[1] ;
	    y2 = ypt[1] ;
	}
	/* always insure that the points are ordered */
	if( x2 < x1 ){
	    swap( &x2, &x1 ) ;
	}
	if( y2 < y1 ){
	    swap( &y2, &y1 ) ;
	}
	if( (i % 2) == 1 ){
	    /* this is the vertical direction look for horz in second tile */
	    for( j = 6; j <= 8; j += 2 ){
		x3 = xpt[j] ;
		y3 = ypt[j] ;
		if( j != 8 ){
		    x4 = xpt[j+1] ;
		    y4 = ypt[j+1] ;
		} else {
		    x4 = xpt[5] ;
		    y4 = ypt[5] ;
		}
		if( x4 < x3 ){
		    swap( &x4, &x3 ) ;
		}
		if( y4 < y3 ){
		    swap( &x4, &x3 ) ;
		}
		if( x3 < x1 && x1 < x4 && y1 < y3 && y3 < y2 ){
		    /* we have an intersection */
		    /* 
				          x (x2,y2)		
					  |
					  |
			(x3,y3) x --------------------x (x4,y4)
					  |
					  |
				          x (x1,y1)		
		    */
		    add_pt( x1, y3, NOTHING ) ;
		}
	    }
	} else {
	    /* this is the horizontal direction look for vert in second tile */
	    for( j = 5; j <= 8; j += 2 ){
		x3 = xpt[j] ;
		y3 = ypt[j] ;
		if( j != 8 ){
		    x4 = xpt[j+1] ;
		    y4 = ypt[j+1] ;
		} else {
		    x4 = xpt[5] ;
		    y4 = ypt[5] ;
		}
		if( x4 < x3 ){
		    swap( &x4, &x3 ) ;
		}
		if( y4 < y3 ){
		    swap( &x4, &x3 ) ;
		}
		if( x1 < x3 && x3 < x2 && y3 < y1 && y1 < y4 ){
		    /* we have an intersection */
		    /* 
				          x (x4,y4)		
					  |
					  |
			(x1,y1) x --------------------x (x2,y2)
					  |
					  |
				          x (x3,y3)		
		    */
		    add_pt( x3, y2, NOTHING ) ;
		}
	    }
	}
    }
}

static swap( a, b )
INT *a, *b ;
{
    INT tmp ;

    tmp = *a ;
    *a = *b ;
    *b = tmp ;
} /* end swap */

static add_pt( x, y, class )
INT x, y, class ;
{
    VERTEXPTR temp ;  /* head of the list */

    if( temp = vertex_listS ){
	vertex_listS = (VERTEX_BOX *) Ysafe_malloc( sizeof( VERTEX_BOX ) ) ;
	vertex_listS->next = temp ;
    } else {
	vertex_listS = (VERTEX_BOX *) Ysafe_malloc( sizeof( VERTEX_BOX ) ) ;
	vertex_listS->next = NIL( VERTEX_BOX * ) ;
    }
    vertex_listS->x = x ;
    vertex_listS->y = y ;
    vertex_listS->class = class ;
} /* end add_pt */

static free_vertex_list()
{
    VERTEXPTR next_vertex ;

    if( vertex_listS ){
	for( ;vertex_listS;  ){
	    next_vertex = vertex_listS->next ;
	    Ysafe_free( vertex_listS ) ;
	    vertex_listS = next_vertex ;
	}
    }
} /* end free_vertex_list */
