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
FILE:	    movestrat2.c
DESCRIPTION:This file contains move strategy for compaction algorithm.
CONTENTS:   
DATE:	    Apr 11, 1988 
REVISIONS:  Dec  4, 1988 - added initialization for next compaction cycle.
	    Apr 30, 1989 - rewrote strategy so that it is a center
		favoring algorithm rather than left or right edge.
	    Sun Nov  4 13:20:53 EST 1990 - rewrote to handle the 
		multiple tiles correctly.  We use cell slack instead
		of tile slack.
	    Sun Nov 11 10:12:22 EST 1990 - grid cell placement if
		a grid is specified.
	    Thu Apr 18 00:58:28 EDT 1991 - new version featuring
		constraint compaction.
	    Wed May  1 19:14:46 EDT 1991 - new move strategy for
		graph constraint compaction.
	    Mon May  6 22:37:01 EDT 1991 - make sure window is correct.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) movestrat2.c version 7.9 5/6/91" ;
#endif

#include <compact.h>
#include <yalecad/debug.h>

#define PICK_INT(l,u) (((l)<(u))?((acm_random() % ((u)-(l)+1))+(l)) : (l))
#define  LEFT_EDGE       0
#define  RIGHT_EDGE      1
#define  CENTERED        2 


static INT xlongestS = INT_MAX ;
static INT ylongestS = INT_MAX ;
static INT area_countS = 0 ;
static INT compact_countS = 0 ;
static INT lengthS ;
static DOUBLE best_areaS = 1.0E30 ;


static x_center();
static calc_xslacks();
static update_xslacks();
static y_center();
static calc_yslacks();
static update_yslacks();
static INT sortby_xslack();


#define HEURISTIC1

    /* -------------------------------------------------------------- 
        space requirements also include 2 sources and 2 sinks: 
        xSource = 0, xsink = numtilesG + 1, ysource = numtilesG + 2,
	and ysink = numtilesG + 3 positions in the tileNode array.
    */
BOOL move_compactx( length ) 
INT length ;
{
    INT 	   i,
		   newX,
		   oldX,
		   delta_move ;
    INT            node ;
    INT            cell ;
    INT            dist ;
    INT            span ;
    COMPACTPTR     boxptr ;
    COMPACTPTR     tile ;
    CELLBOXPTR     cptr ;
    INFOPTR        stretch_edge ;
    INFOPTR        min_edge ;
    INFOPTR        max_edge ;
    NODEPTR        nptr ;

    /* initialize moved cell array to false */
    for( i=1;i<=numtilesG;i++ ){
	tileNodeG[i]->moved = FALSE ;
    }
    if( constraintsG ){
	for( cell=1;cell<=numcellsG;cell++ ){
	    cptr = cellarrayG[cell] ;
	    cptr->stretched = FALSE ;
	}
    }

    /* let all routines know the length */
    lengthS = length ;

    /* use the path_deck to enumerate tiles in the critical path */
    if( constraintsG ){
	for( Ydeck_top(path_deckG);
	    Ydeck_notEnd(path_deckG);Ydeck_down(path_deckG) ) {
	    node = (INT) Ydeck_getData( path_deckG ) ;
	    tile = tileNodeG[node] ;
	    cell = tile->cell ;
	    cptr = cellarrayG[cell] ;
	    if( cptr->stretched ){
		/* avoid multiple adjustments */
		continue ;
	    }
	    stretch_edge = (INFOPTR) Yrbtree_min( cptr->vedges ) ;
	    stretch_graph( stretch_edge, tile->xvalueMin, 0 ) ;
	    cptr->stretched = TRUE ;
	}

#ifdef LATER
	/* now make sure all of the graph is large enough */
	for( cell=1;cell<=numcellsG;cell++ ){

	    cptr = cellarrayG[ cell ] ;
	    /* find the new center of the cell which keeps it within */
	    /* the graph bounding box */
	    min_edge = (INFOPTR) Yrbtree_min( cptr->vedges ) ;
	    max_edge = (INFOPTR) Yrbtree_max( cptr->vedges ) ;
	    dist = *max_edge->loc - *min_edge->loc ;
	    span = cptr->r_rel - cptr->l_rel + xspaceG ;
	    if( dist < span ){
		/* graph needs to be stretched */
		stretch_graph( max_edge, *min_edge->loc + span, 0 ) ;
	    }
	}
#endif
    }

    /* In order for algorithm to be correct for the multiple tile case */
    /* we must first place tiles on the critical path */
    for( i=1;i<=numtilesG;i++ ){
	boxptr = xGraphG[i] ;
	if( boxptr->moved ){
	    /* the cell has already been placed */
	    continue ;
	}
	if( boxptr->criticalX ){
	    /* save old left edge of tile */
	    oldX = boxptr->l ;
	    ASSERT( boxptr->xvalueMin == length - boxptr->xvalueMax,
		"move_compactx", "Critical path where xmax != xmin\n" ) ;
	    /* calculate delta as new xcenter - old xcenter */
	    newX = boxptr->xvalueMin ;
#ifdef LATER
	    if( constraintsG ){
		/* find the new left edge of the tile which fits */
		/* in graph */
		min_edge = (INFOPTR) 
		    Yrbtree_min( cellarrayG[boxptr->cell]->vedges ) ;
		if( newX < *min_edge->loc ){
		    newX = *min_edge->loc ;
		}
	    }
#endif
	    /* update position of cell tiles */
	    delta_move = newX - oldX ;
	    update_cell_tiles( boxptr->cell, delta_move, 0 ) ;

	    /* now update tiles windows */
	    cptr = cellarrayG[boxptr->cell] ;
	    for( nptr = cptr->tiles; nptr ; nptr=nptr->next ){
		ASSERTNCONT( nptr->node > 0 && nptr->node <= numtilesG,
		    "move_compactx", "tile out of bounds\n" ) ;
		tile = tileNodeG[nptr->node] ;

		tile->xvalueMin = cptr->xcenter + tile->l_rel ;
		tile->xvalueMax = length - tile->xvalueMin ;
	    }
	}
    } /* This point we have placed the tiles in the critical path */

    /* ****** now place the cells off the critical path ****** */
    x_center() ;
    /********* end placing tiles off the critical path ***********/

    /* save length */
    xlongestS = length ;

} /* end move_compactx */

static x_center()
{
    INT            i,
		   newX,
		   oldX,
		   temp,
		   remainder,
		   xmin,  xmax,
		   min, max,
    		   sortby_xslack(),
		   delta_move ;
    INT            newpos ;
    COMPACTPTR     boxptr ;
    NODEPTR        nptr ;
    CELLBOXPTR     cptr ;
    INFOPTR        min_edge ;
    INFOPTR        max_edge ;


    for( i=1;i<=numcellsG;i++ ){
	/* sort the slack array by increasing slack */
	calc_xslacks( i ) ;
	Yquicksort( (char *) &(slackG[i]), numcellsG-i, sizeof(CELLBOXPTR),
	    sortby_xslack ) ;
	cptr = slackG[i] ;
	nptr = cptr->tiles ;
	ASSERTNCONT( nptr, "x_center", "nptr is null\n" ) ;

	boxptr = tileNodeG[nptr->node] ;
	if( boxptr->moved ){
	    /* the cell has already been placed */
	    continue ;
	}
	ASSERT( !boxptr->criticalX, "move_compactx", 
	    "Critical path tile should already have been placed\n" );

	/* first heuristic-move everything to ave. of constraints */
	/* find window intersection of all constraints */
	xmin = cptr->xmin ;

	xmax = cptr->xmax ;
	
	ASSERT( xmin <= xmax, "x_center", "Constraints cannot be met\n");

	/* relative to cell center */
	newX = (xmin + xmax) DIVIDED_BY_2 ;
	oldX = cptr->xcenter ;

	/********* constrain cell center if graph is specified *********/
	if( constraintsG ){
	    /* find the new center of the cell which keeps it within */
	    /* the graph bounding box */
	    min_edge = (INFOPTR) 
		Yrbtree_min( cellarrayG[boxptr->cell]->vedges ) ;
	    max_edge = (INFOPTR) 
		Yrbtree_max( cellarrayG[boxptr->cell]->vedges ) ;
	    min = *min_edge->loc - cptr->l_rel ;
	    max = *max_edge->loc - cptr->r_rel ;
	    if( xmin <= min && max <= xmax ){
		/* this means we can use the constraint graph */
		if( min <= xmax ){
		    /* we can use the graph constraint for lower bound */
		    xmin = min ;
		} else {
		    /* make sure we meet the window constraints */
		    xmin = xmax ;
		}
		if( min <= max && xmin <= max ){
		    /* we can use the graph constraint for upper bound */
		    xmax = max ;
		} else {
		    /* cell doesn't fit adjust to left */
		    xmax = xmin ;
		}

	    /* in these two cases do the best we can */
	    } else if( xmin <= min && min <= xmax ){
		xmin = min ;
	    } else if ( max <= xmax && xmin <= max ){
		xmax = max ;
	    }
	    newX = (xmin + xmax) DIVIDED_BY_2 ;
	}
	/********* grid cell data if specified *************/
	/* set lower left corner to grid */
	if( xgridG ){ /* only do if non zero */
	    temp = newX + cptr->xoffset ;
	    /* do individual tests only one may be zero */
	    if( remainder = temp % xgridG ){
		/* fix x direction */
		temp += xgridG - remainder ;
	    }
	    newpos = temp - cptr->xoffset ;
	    if( newpos < xmin ){
		/* try next larger grid */
		newpos += xgridG ;
		if( newpos < xmin || newpos > xmax ){
		    newX = xmin ;
		} else { 
		    newX = newpos ;
		}
	    } else if( newpos > xmax ){
		/* try next grid */
		newpos -= xgridG ;
		if( newpos < xmin || newpos > xmax ){
		    newX = xmax ;
		} else { 
		    newX = newpos ;
		}
	    } else {
		newX = newpos ;
	    }
	}

	/* update position of cell tiles */
	delta_move = newX - oldX ;
	update_cell_tiles( boxptr->cell, delta_move, 0 ) ;
	update_xslacks( boxptr ) ;
    } /* end looking at cells off the critical path */

} /* end center_x */

static calc_xslacks( cur_cell )
{
    INT i ; 			/* counter */
    INT xmin ;			/* max of all the minimums of a cell */
    INT xmax ;			/* min of all the maximums of a cell */
    INT cent_min ;		/* min window of cell center relative */
    INT cent_max ;		/* max window of cell center relative */
    NODEPTR nptr ;		/* current tile node of cell */
    CELLBOXPTR cptr ;		/* current cell */
    COMPACTPTR tptr ;		/* current tile of cell */
    /***********************************************************
    * We need to find the intersection of all the tiles of a cell,
    * that is, we want to find the maximum of all the minimum windows
    * of the tiles relative to the center of the cell (called xmin).
    * Likewise, we want to find the minimum of all the maximum windows
    * relative to the xcenter of the cell.  We call that xmax.
    ***********************************************************/
    for( i = cur_cell; i <= numcellsG; i++ ){
	cptr = slackG[i] ;
	if( cptr->multi ){
	    xmin = INT_MIN ;
	    xmax = INT_MAX ;
	    for( nptr = cptr->tiles; nptr ; nptr=nptr->next ){
		ASSERTNCONT( nptr->node > 0 && nptr->node <= numtilesG,
		    "calc_xslacks", "tile out of bounds\n" ) ;
		tptr = tileNodeG[nptr->node] ;
		/* first make center relative */
		cent_min = tptr->xvalueMin - tptr->l_rel ;
		cent_max = lengthS - tptr->xvalueMax - tptr->l_rel ;
		xmin = MAX( xmin, cent_min ) ;
		xmax = MIN( xmax, cent_max ) ;
	    }
	    /* now save the value of the slack */
	    cptr->xmin = xmin ;
	    cptr->xmax = xmax ;
	} else {
	    /* the easy case only one tile */
	    nptr = cptr->tiles;
	    ASSERTNCONT( nptr->node > 0 && nptr->node <= numtilesG,
		"calc_xslacks", "tile out of bounds\n" ) ;
	    tptr = tileNodeG[nptr->node] ;
	    cptr->xmin = tptr->xvalueMin - tptr->l_rel ;
	    cptr->xmax = lengthS - tptr->xvalueMax - tptr->l_rel ;
	}
    }
} /* end calc_xslacks */

static update_xslacks( tptr ) 
COMPACTPTR tptr ;
{
    INT j ;			/* current tile adjacent to node */
    INT node ;			/* current node popped from the queue */
    INT tile ;			/* one of the tiles of the adj. cell */
    INT length ;		/* length of longest path */
    INT setValue ;		/* the value of the path to this adj node */
    INT currentValue ;		/* path value of node popped from queue */
    INT siblingValue ;		/* the value of the path to sibling of adj node */
    BOOL first ;		/* used to setup the queue */
    NODEPTR nptr ;		/* used to traverse multi tiles */
    CELLBOXPTR cptr ;		/* the current cell */
    COMPACTPTR sibling ;	/* the tile record of the sibling to adj node */
    COMPACTPTR nextptr ;	/* the tile record of the adj. node */
    ECOMPBOXPTR ptr ;		/* used to traverse edges of popped node */
    QUEUEPTR botqueue, queue ;	/* used to implement queue MACRO */
    YSETLISTPTR nodeptr ;       /* used to set the ancestors correctly */

    /* ******************************************************
    Algorithm for updating the xslacks.
	See algorithm for updating yslacks below.
    * ******************************************************* */
    /* now update tiles windows */
    cptr = cellarrayG[tptr->cell] ;
    for( nptr = cptr->tiles; nptr ; nptr=nptr->next ){
	ASSERTNCONT( nptr->node > 0 && nptr->node <= numtilesG,
	    "move_compactx", "tile out of bounds\n" ) ;
	nextptr = tileNodeG[nptr->node] ;

	nextptr->xvalueMin = cptr->xcenter + nextptr->l_rel ;
	nextptr->xvalueMax = lengthS - nextptr->xvalueMin ;
    }
    longestxPath( FALSE ) ;

} /* end update_xslacks */

BOOL move_compacty( length ) 
int length ;
{
    INT 	   i,
		   newY,
		   oldY,
		   delta_move ;
    INT            node ;
    INT            cell ;
    INT            dist ;
    INT            span ;
    COMPACTPTR     boxptr ;
    COMPACTPTR     tile ;
    CELLBOXPTR     cptr ;
    INFOPTR        stretch_edge ;
    INFOPTR        min_edge ;
    INFOPTR        max_edge ;
    NODEPTR        nptr ;

    /* initialize moved cell array to false */
    for( i=1;i<=numtilesG;i++ ){
	tileNodeG[i]->moved = FALSE ;
    }
    if( constraintsG ){
	for( cell=1;cell<=numcellsG;cell++ ){
	    cptr = cellarrayG[cell] ;
	    cptr->stretched = FALSE ;
	}
    }

    /* let all routines know the length */
    lengthS = length ;

    /* use the path_deck to enumerate tiles in the critical path */
    if( constraintsG ){
	for( Ydeck_top(path_deckG);
	    Ydeck_notEnd(path_deckG);Ydeck_down(path_deckG) ) {
	    node = (INT) Ydeck_getData( path_deckG ) ;
	    tile = tileNodeG[node] ;
	    cell = tile->cell ;
	    cptr = cellarrayG[cell] ;
	    if( cptr->stretched ){
		/* avoid multiple adjustments */
		continue ;
	    }
	    stretch_edge = (INFOPTR) Yrbtree_min( cptr->hedges ) ;
	    stretch_graph( stretch_edge, 0, tile->yvalueMin ) ;
	    cptr->stretched = TRUE ;
	}

#ifdef LATER
	/* now make sure all of the graph is large enough */
	for( cell=1;cell<=numcellsG;cell++ ){

	    cptr = cellarrayG[ cell ] ;
	    /* find the new center of the cell which keeps it within */
	    /* the graph bounding box */
	    min_edge = (INFOPTR) Yrbtree_min( cptr->hedges ) ;
	    max_edge = (INFOPTR) Yrbtree_max( cptr->hedges ) ;
	    dist = *max_edge->loc - *min_edge->loc ;
	    span = cptr->t_rel - cptr->b_rel + yspaceG ;
	    if( dist < span ){
		/* graph needs to be stretched */
		stretch_graph( max_edge, 0, *min_edge->loc + span ) ;
	    }
	}
#endif
    }

    /* In order for algorithm to be correct for the multiple tile case */
    /* we must first place tiles on the critical path */
    for( i=1;i<=numtilesG;i++ ){
	boxptr = yGraphG[i] ;
	if( boxptr->moved ){
	    /* the cell has already been placed */
	    continue ;
	}
	if( boxptr->criticalY ){
	    /* save old left edge of tile */
	    oldY = boxptr->b ;
	    ASSERT( boxptr->yvalueMin == length - boxptr->yvalueMax,
		"move_compacty", "Critical path where ymax != ymin" ) ;
	    /* calculate delta as new ycenter - old ycenter */
	    newY = boxptr->yvalueMin ;
#ifdef LATER
	    if( constraintsG ){
		/* find the new bottom edge of the tile which fits */
		/* in graph */
		min_edge = (INFOPTR) 
		    Yrbtree_min( cellarrayG[boxptr->cell]->hedges ) ;
		if( newY < *min_edge->loc ){
		    newY = *min_edge->loc ;
		}
	    }
#endif
	    delta_move = newY - oldY ;
	    /* update position of cell tiles */
	    update_cell_tiles( boxptr->cell, 0, delta_move ) ;

	    /* now update tiles windows */
	    cptr = cellarrayG[boxptr->cell] ;
	    for( nptr = cptr->tiles;nptr; nptr=nptr->next ){
		ASSERTNCONT( nptr->node > 0 && nptr->node <= numtilesG,
		    "move_compactx", "tile out of bounds\n" ) ;
		tile = tileNodeG[nptr->node] ;

		tile->yvalueMin = cptr->ycenter + tile->b_rel ;
		tile->yvalueMax = length - tile->yvalueMin ;
	    }
	}
    } /* This point we have placed the tiles in the critical path */

    /* ****** now place the cells off the critical path ****** */
    y_center() ;
    /********* end placing tiles off the critical path ***********/

    /* save length */
    ylongestS = length ;

} /* end move_compacty */

static y_center()
{
    int            i,
		   newY,
		   oldY,
		   temp,
		   remainder,
		   min,   max,
		   ymin,  ymax,
		   delta_move ;
    INT		   sortby_yslack();
    INT            newpos ;
    COMPACTPTR     boxptr ;
    NODEPTR        nptr ;
    CELLBOXPTR     cptr ;
    INFOPTR        min_edge ;
    INFOPTR        max_edge ;


    for( i=1;i<=numcellsG;i++ ){
	/* sort the slack array by increasing slack */
	calc_yslacks( i ) ;
	Yquicksort( (char *) &(slackG[i]), numcellsG-i, sizeof(CELLBOXPTR),
	    sortby_yslack ) ;
	cptr = slackG[i] ;
	nptr = cptr->tiles ;
	ASSERTNCONT( nptr, "y_center", "nptr is null\n" ) ;

	boxptr = tileNodeG[nptr->node] ;
	if( boxptr->moved ){
	    /* the cell has already been placed */
	    continue ;
	}
	ASSERT( !boxptr->criticalY, "move_compacty", 
	    "Critical path tile should already have been placed\n" );

	/* first heuristic-move everything to ave. of constraints */
	/* find window intersection of all constraints */
	ymin = cptr->ymin ;

	ymax = cptr->ymax ;

	ASSERT( ymin <= ymax, "y_center", "Constraints cannot be met\n");

	/* relative to cell center */
	newY = (ymin + ymax) DIVIDED_BY_2 ;
	oldY = cptr->ycenter ;

	/********* constrain cell center if graph is specified *********/
	if( constraintsG ){
	    /* find the new center of the cell which keeps it within */
	    /* the graph bounding box */
	    min_edge = (INFOPTR) 
		Yrbtree_min( cellarrayG[boxptr->cell]->hedges ) ;
	    max_edge = (INFOPTR) 
		Yrbtree_max( cellarrayG[boxptr->cell]->hedges ) ;
	    min = *min_edge->loc - cptr->b_rel ;
	    max = *max_edge->loc - cptr->t_rel ;
	    if( ymin <= min && max <= ymax ){
		/* this means we can use the constraint graph */
		if( min <= ymax ){
		    /* we can use the graph constraint for lower bound */
		    ymin = min ;
		} else {
		    /* make sure we meet the window constraints */
		    ymin = ymax ;
		}
		if( min <= max && ymin <= max ){
		    /* we can use the graph constraint for upper bound */
		    ymax = max ;
		} else {
		    /* cell doesn't fit adjust to left */
		    ymax = ymin ;
		}

	    /* in these two cases do the best we can */
	    } else if( ymin <= min && min <= ymax ){
		ymin = min ;
	    } else if ( max <= ymax && ymin <= max ){
		ymax = max ;
	    }
	    newY = (ymin + ymax) DIVIDED_BY_2 ;
	}

	/********* grid cell data if specified *************/
	/* set lower left corner to grid */
	if( ygridG ){ /* only do if non zero */
	    temp = newY + cptr->yoffset ;
	    /* do individual tests only one may be zero */
	    if( remainder = temp % ygridG ){
		/* fix y direction */
		temp += ygridG - remainder ;
	    }
	    newpos = temp - cptr->yoffset ;
	    if( newpos < ymin ){
		/* try next larger grid */
		newpos += ygridG ;
		if( newpos < ymin || newpos > ymax ){
		    newY = ymin ;
		} else { 
		    newY = newpos ;
		}
	    } else if( newpos > ymax ){
		/* try next grid */
		newpos -= ygridG ;
		if( newpos < ymin || newpos > ymax ){
		    newY = ymax ;
		} else { 
		    newY = newpos ;
		}
	    } else {
		newY = newpos ;
	    }
	}

	/* update position of cell tiles */
	delta_move = newY - oldY ;
	update_cell_tiles( boxptr->cell, 0, delta_move ) ;
	update_yslacks( boxptr ) ;
    } /* end looking at cells off the critical path */

} /* end y_center */

static calc_yslacks( cur_cell )
{
    INT i ; 			/* counter */
    INT ymin ;			/* max of all the minimums of a cell */
    INT ymax ;			/* min of all the maximums of a cell */
    INT cent_min ;		/* min window of cell center relative */
    INT cent_max ;		/* max window of cell center relative */
    NODEPTR nptr ;		/* current tile node of cell */
    CELLBOXPTR cptr ;		/* current cell */
    COMPACTPTR tptr ;		/* current tile of cell */

    /***********************************************************
    * We need to find the intersection of all the tiles of a cell,
    * that is, we want to find the maximum of all the minimum windows
    * of the tiles relative to the center of the cell (called ymin).
    * Likewise, we want to find the minimum of all the maximum windows
    * relative to the ycenter of the cell.  We call that ymax.
    ***********************************************************/
    for( i = cur_cell; i <= numcellsG; i++ ){
	cptr = slackG[i] ;
	if( cptr->multi ){
	    ymin = INT_MIN ;
	    ymax = INT_MAX ;
	    for( nptr = cptr->tiles; nptr ; nptr=nptr->next ){
		ASSERTNCONT( nptr->node > 0 && nptr->node <= numtilesG,
		    "calc_yslacks", "tile out of bounds\n" ) ;
		tptr = tileNodeG[nptr->node] ;
		/* first make center relative */
		cent_min = tptr->yvalueMin - tptr->b_rel ;
		cent_max = lengthS - tptr->yvalueMax - tptr->b_rel ;
		ymin = MAX( ymin, cent_min ) ;
		ymax = MIN( ymax, cent_max ) ;
	    }
	    /* now save the value of the slack */
	    cptr->ymin = ymin ;
	    cptr->ymax = ymax ;
	} else {
	    /* the easy case only one tile */
	    nptr = cptr->tiles;
	    ASSERTNCONT( nptr->node > 0 && nptr->node <= numtilesG,
		"calc_yslacks", "tile out of bounds\n" ) ;
	    tptr = tileNodeG[nptr->node] ;
	    cptr->ymin = tptr->yvalueMin - tptr->b_rel ;
	    cptr->ymax = lengthS - tptr->yvalueMax - tptr->b_rel ;
	}
    }
} /* end calc_yslacks */

static update_yslacks( tptr ) 
COMPACTPTR tptr ;
{
    INT j ;			/* current tile adjacent to node */
    INT node ;			/* current node popped from the queue */
    INT tile ;			/* one of the tiles of the adj. cell */
    INT length ;		/* length of longest path */
    INT setValue ;		/* the value of the path to this adj node */
    INT currentValue ;		/* path value of node popped from queue */
    INT siblingValue ;		/* the value of the path to sibling of adj node */
    BOOL first ;		/* used to setup the queue */
    NODEPTR nptr ;		/* used to traverse multi tiles */
    CELLBOXPTR cptr ;		/* the current cell */
    COMPACTPTR sibling ;	/* the tile record of the sibling to adj node */
    COMPACTPTR nextptr ;	/* the tile record of the adj. node */
    ECOMPBOXPTR ptr ;		/* used to traverse edges of popped node */
    QUEUEPTR botqueue, queue ;	/* used to implement queue MACRO */
    YSETLISTPTR nodeptr ;       /* used to set the ancestors correctly */

    /* ******************************************************
    Algorithm for updating the yslacks.
	We need to actually perform longest path algorithm
	to update the slacks but we need to do it with fixed
	objects.  
    * ******************************************************* */
    /* now update tiles windows */
    cptr = cellarrayG[tptr->cell] ;
    for( nptr = cptr->tiles; nptr ; nptr=nptr->next ){
	ASSERTNCONT( nptr->node > 0 && nptr->node <= numtilesG,
	    "move_compactx", "tile out of bounds\n" ) ;
	nextptr = tileNodeG[nptr->node] ;

	nextptr->yvalueMin = cptr->ycenter + nextptr->b_rel ;
	nextptr->yvalueMax = lengthS - nextptr->yvalueMin ;

    }
    longestyPath( FALSE ) ;
} /* end update_yslacks */

/* sort by x first then y */
static INT sortby_xslack( cellA , cellB )
CELLBOXPTR *cellA , *cellB ;
{
    INT excess_slacka, excess_slackb ;
    CELLBOXPTR  cptr ;

    cptr = *cellA ;
    excess_slacka = cptr->xmax - cptr->xmin ;
    cptr = *cellB ;
    excess_slackb = cptr->xmax - cptr->xmin ;
    return( excess_slacka - excess_slackb ) ;
} /* sortby_xslack */

/* sort by x first then y */
INT sortby_yslack( cellA , cellB )
CELLBOXPTR *cellA , *cellB ;
{
    INT excess_slacka, excess_slackb ;
    CELLBOXPTR  cptr ;

    cptr = *cellA ;
    excess_slacka = cptr->ymax - cptr->ymin ;
    cptr = *cellB ;
    excess_slackb = cptr->ymax - cptr->ymin ;
    return( excess_slacka - excess_slackb ) ;
} /* sortby_yslack */

BOOL test_area()
{
    DOUBLE area ;

    area = (DOUBLE) xlongestS * (DOUBLE) ylongestS ;

    ++compact_countS ;
    if( area < best_areaS ){
	/* save best */
	best_areaS = area ;
	area_countS = 0 ;
    } else {
	area_countS++ ;
	if( compact_countS >= 4 && area_countS >= 2 ){
	    return(FALSE) ; /* satisfied */
	}
    }
    return(TRUE) ; /* not done yet */

} /* end test_area */
