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
FILE:	    io.c
DESCRIPTION:This file contains routines for handling I/O.  The compactor
	    writes and reads the same format.
CONTENTS:   
DATE:	    Apr 17, 1989 
REVISIONS:  Apr 30, 1989 - added direction field for partitioning.
	    Sun Nov  4 13:19:23 EST 1990 - rewrote for handling
		multiple tiles correctly.  We now look at cell
		slack instead of tile slack.
	    Mon May  6 22:34:31 EDT 1991 - added gridding reference.
	    Fri Nov  8 18:16:21 EST 1991 - removed INT_SMALL and INT_LARGE
		definitions since gcc couldn't handle it.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) io.c version 7.5 5/21/92" ;
#endif

#include <compact.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>

static INT boxLS, boxRS, boxTS, boxBS ;  /* bounding box of tiles */
static INT celLS, celRS, celTS, celBS ;  /* bounding box of cur cell */
static INT curCellS = 0 ;                /* current cell */
static INT curTileS = 0 ;                /* current tile */
static INT xcS, ycS ;                    /* current cell center */
static INT int_largeS, int_smallS ;      /* used because of compiler bugs */
static BOOL errorFlagS = FALSE ;         /* records fault condition */
static BOOL curCellTypeS ;               /* hard or soft type */
static BOOL multiS ;                     /* cell has multiple tiles ??? */
static CELLBOXPTR ptrS ;                 /* pointer to current cell */


/* ***************** ERROR HANDLING ****************************** */
/* ERRORABORT is a macro which forces routines not to do any work */
/* when we find a syntax error in the input routine. */
#define ERRORABORT() \
{ \
    if( errorFlagS ){ \
	return ; /* don't do any work for errors */ \
    } \
} \

setErrorFlag()
{
    errorFlagS = TRUE ;
}
/* ***************** ERROR HANDLING ****************************** */
init( numtiles, numcells )
INT numtiles, numcells ;
{
    INT i ;

    /* save number of tiles in global for future use */
    numtilesG = numtiles ;
    numcellsG = numcells ;
    last_tileG = numtilesG + 1 ;
    last_cellG = numcellsG + 3 ;

    /* initialize bounding box of tiles-need bounding box for source */
    /* and sink positions */
    boxLS = INT_MAX ;
    boxBS = INT_MAX ;
    boxRS = INT_MIN ;
    boxTS = INT_MIN ;

    /* allocate space for graph */
    tileNodeG = (COMPACTPTR *) Ysafe_malloc( (numtilesG+1) * sizeof(COMPACTPTR) ); 
    for( i = 1 ; i <= numtilesG ; i++ ){
	tileNodeG[i] = (COMPACTPTR) Ysafe_malloc( sizeof(COMPACTBOX) );
    }
    /* -------------------------------------------------------------- 
        Now allocate space for xcenter and ycenter arrays which
	keep track of the cells center.  Now make cells for source
	sink for x and y graphs.
    */
    cellarrayG = (CELLBOXPTR *) 
	Ysafe_malloc( (last_cellG+1)*sizeof(CELLBOXPTR) ) ;
    slackG = (CELLBOXPTR *) 
	Ysafe_malloc( (numcellsG+1)*sizeof(CELLBOXPTR) ) ;
    for( i = 0; i <= numcellsG; i++ ){
	slackG[i] = cellarrayG[i] = 
	    (CELLBOXPTR) Ysafe_calloc( 1, sizeof(CELLBOX)) ;

    }
    /* allocate room for the source and sink */
    for( numcellsG+1; i <= last_cellG; i++ ){
	cellarrayG[i] = (CELLBOXPTR) Ysafe_calloc( 1, sizeof(CELLBOX)) ;
    }
} /* end init */

final_tiles()
{
    INT i ;        		/* counter */
    INT space ;        		/* counter */
    INT count ;        		/* counter */
    INT minx, maxx ;            /* min and max x nodes of cell */
    INT miny, maxy ;            /* min and max y nodes of cell */
    BOOL multi ;		/* is tile one of multitile cell */
    NODEPTR aptr ;		/* current node */
    COMPACTPTR tptr ;		/* current tile */
    COMPACTPTR *newArray ; 	/* temporarily build a new tile array */
    CELLBOXPTR cptr ;           /* current cell */

    /* first count the number of tiles */
    count = 0 ;
    for( i=1;i<=numcellsG;i++ ){
	for( aptr = cellarrayG[i]->tiles;aptr; aptr = aptr->next ){
	    count++ ;
	}
    }
    numtilesG = count ;
    /* initialize the last tile convenience variable */
    last_tileG = numtilesG + 1 ;

    /* otherwise begin realloc */
    /* -------------------------------------------------------------- 
        space requirements also include 2 sources and 2 sinks: 
        xSource = 0, xsink = numtiles + 1, ysource = numtiles + 2,
	and ysink = numtiles + 3 positions in the tileNodeS array.
    */
    space = numtilesG + 4 ;

    /* allocate space for tileNode array */
    newArray = (COMPACTPTR *) Ysafe_malloc( space * sizeof(COMPACTPTR) ); 
    ancestorG = (INT *) Ysafe_malloc( space * sizeof(INT) ) ;
    count = 0 ;
    for( i=1;i<=numcellsG;i++ ){
	for( aptr = cellarrayG[i]->tiles;aptr; aptr = aptr->next ){
	    tptr = tileNodeG[aptr->node] ;
	    ASSERTNCONT( tptr, "final_tiles", "tptr NULL\n" ) ;
	    newArray[++count] = tptr ;
	    aptr->node = count ;
	    tptr->node = count ;
	}
    }
    /* now free the old tileNode array */
    Ysafe_free( tileNodeG ) ;
    tileNodeG = newArray ;
    /* initialize the tile set */
    tileSetG = Yset_init( 0, numtilesG+3 ) ;

    /* NOW create x and y graphs ie. clone them */
    space = numtilesG + 2 ;
    xGraphG = (COMPACTPTR *) Ysafe_malloc( space * sizeof(COMPACTPTR) ) ;
    yGraphG = (COMPACTPTR *) Ysafe_malloc( space * sizeof(COMPACTPTR) ) ;
    for( i = 1; i< space; i++ ){
	xGraphG[i] = tileNodeG[i] ;
	yGraphG[i] = tileNodeG[i] ;
    }

    /* now set the multi fields to be correct */
    for( i=1;i<=numcellsG;i++ ){
	count = 0 ;
	multi = FALSE ;
	cptr = cellarrayG[i] ;
	for( aptr = cptr->tiles;aptr; aptr = aptr->next ){
	    ASSERTNCONT( aptr->node > 0 && aptr->node <= numtilesG,
		"preprocess_multi", "tile out of bounds\n" ) ;
	    if( ++count >= 2 ){
		multi = TRUE ;
		break ;
	    }
	} /* end loop on tiles */

	cptr->multi = multi ;

	/* set the multi field and also the low and hi nodes for the tiles */
	minx = INT_MAX ;
	miny = INT_MAX ;
	maxx = INT_MIN ;
	maxy = INT_MIN ;
	for( aptr = cptr->tiles;aptr; aptr = aptr->next ){
	    ASSERTNCONT( aptr->node > 0 && aptr->node <= numtilesG,
		"preprocess_multi", "tile out of bounds\n" ) ;
	    tptr = tileNodeG[aptr->node] ;
	    tptr->multi = multi ;
	    /* now check on the lowest tiles */
	    if( tptr->l_rel < minx ||
		(tptr->l_rel == minx && aptr->node < cptr->xlo) ){
		minx = tptr->l_rel ;
		cptr->xlo = aptr->node ;
	    }
	    if( tptr->b_rel < miny ||
		(tptr->b_rel == miny && aptr->node < cptr->ylo) ){
		miny = tptr->b_rel ;
		cptr->ylo = aptr->node ;
	    }
	    if( tptr->l_rel > maxx ||
		(tptr->l_rel == maxx && aptr->node > cptr->xhi) ){
		maxx = tptr->l_rel ;
		cptr->xhi = aptr->node ;
	    }
	    if( tptr->b_rel > maxy ||
		(tptr->b_rel == maxy && aptr->node > cptr->yhi) ){
		maxy = tptr->b_rel ;
		cptr->yhi = aptr->node ;
	    }
	} /* end loop on tiles */
    }
    /* add node box to the sources and sinks */
    /* XSOURCE */
    aptr = cellarrayG[XSOURCEC]->tiles=(NODEPTR) Ysafe_malloc( sizeof(NODEBOX)) ;
    aptr->next = NIL(NODEBOX *) ;
    aptr->node = XSOURCE ;
    /* XSINK */
    aptr = cellarrayG[XSINKC]->tiles = (NODEPTR) Ysafe_malloc( sizeof(NODEBOX) ) ;
    aptr->next = NIL(NODEBOX *) ;
    aptr->node = XSINK ;
    /* YSOURCE */
    aptr = cellarrayG[YSOURCEC]->tiles= (NODEPTR) Ysafe_malloc( sizeof(NODEBOX)) ;
    aptr->next = NIL(NODEBOX *) ;
    aptr->node = YSOURCE ;
    /* YSINK */
    aptr = cellarrayG[YSINKC]->tiles = (NODEPTR) Ysafe_malloc( sizeof(NODEBOX) ) ;
    aptr->next = NIL(NODEBOX *) ;
    aptr->node = YSINK ;

} /* end final_tiles */

/* set the current cell */
initCell( celltype, cellnum, x, y, xoffset, yoffset )
INT celltype ;
INT cellnum ;
INT x, y ;
INT xoffset, yoffset ;
{
    ERRORABORT() ;
    if( ++curCellS > numcellsG ){
	setErrorFlag() ;
	M(ERRMSG, "initCell", "Problem with cell number\n" ) ;
	return ;
    }
    ptrS = cellarrayG[curCellS] ;
    ptrS->type = curCellTypeS = celltype ;
    ptrS->cellnum = cellnum ;
    ptrS->xoffset = xoffset ;
    ptrS->yoffset = yoffset ;
    /* save center of cell in center arrays */
    xcS = ptrS->xcenter = x ;
    ycS = ptrS->ycenter = y ;
    /* initialize tile list */
    ptrS->tiles = NULL ;
    celLS = INT_MAX ;
    celBS = INT_MAX ;
    celRS = INT_MIN ;
    celTS = INT_MIN ;
    /* set multi tile counter to zero */
    multiS = 0 ;
} /* end initCell */

init_extra_tile( cell, type )
INT cell ;
{
    curTileS = numtilesG ;
    numtilesG++ ;
    last_tileG = numtilesG + 1 ;
    tileNodeG = (COMPACTPTR *) Ysafe_realloc( tileNodeG,
	(numtilesG+1) * sizeof(COMPACTPTR) ); 
    tileNodeG[numtilesG] = (COMPACTPTR) Ysafe_malloc( sizeof(COMPACTBOX) );
    curCellS = cell ;
    ptrS = cellarrayG[curCellS] ;
    xcS = ptrS->xcenter ;
    ycS = ptrS->ycenter ;
    curCellTypeS = type ;
} /* init_extra_tile */

/* initialize tile */
INT addtile( l, r, b, t )
INT l, r, b, t ;
{
    COMPACTPTR tptr ;
    NODEPTR    temp, nptr ;

    ERRORABORT() ;
    if( ++curTileS > numtilesG ){
	setErrorFlag() ;
	M(ERRMSG, "addtile", "Problem with number of tiles\n" ) ;
	return ;
    }
    tptr = tileNodeG[curTileS] ;
    /* save relative positions */
    tptr->l_rel = tptr->l = l ;
    tptr->r_rel = tptr->r = r ;
    tptr->b_rel = tptr->b = b ;
    tptr->t_rel = tptr->t = t ;
    tptr->xspan = r - l ;
    tptr->yspan = t - b ;
    /* find the cells bounding box */
    celLS = MIN( l, celLS ) ;
    celBS = MIN( b, celBS ) ;
    celRS = MAX( r, celRS ) ;
    celTS = MAX( t, celTS ) ;
    /* now set global positions */
    tptr->l += xcS ;
    tptr->r += xcS ;
    tptr->b += ycS ;
    tptr->t += ycS ;
    /* save bounding box of all tiles */
    boxLS = MIN( tptr->l, boxLS ) ;
    boxBS = MIN( tptr->b, boxBS ) ;
    boxRS = MAX( tptr->r, boxRS ) ;
    boxTS = MAX( tptr->t, boxTS ) ;

    /* save the data */
    tptr->node  = curTileS ;
    tptr->cell  = curCellS ;
    tptr->type  = curCellTypeS ;
    tptr->multi = FALSE ;
    tptr->moved = FALSE ;
    tptr->xadjF = NULL ;
    tptr->xadjB = NULL ;
    tptr->yadjF = NULL ;
    tptr->yadjB = NULL ;
    tptr->direction = NODIRECTION ;

    /* now add tile to the list of tiles */
    if( temp = ptrS->tiles ){
	nptr = ptrS->tiles = (NODEPTR) Ysafe_malloc( sizeof(NODEBOX) ) ;
	nptr->next = temp ;
    } else { /* initialize list */
	nptr = ptrS->tiles = (NODEPTR) Ysafe_malloc( sizeof(NODEBOX) ) ;
	nptr->next = NULL ;
    }
    /* now fill in the data */
    nptr->node = curTileS ;

    /* now update multi tile counter */
    multiS++ ;

    return( curTileS ) ;

} /* end addtile */

endCell()
{
    ERRORABORT() ;
    /* update the bounding box of the cell */
    /* bounding box is used to force cell to grid */
    ptrS->l_rel = celLS ;
    ptrS->r_rel = celRS ;
    ptrS->b_rel = celBS ;
    ptrS->t_rel = celTS ;
    ptrS->l = celLS + xcS ;
    ptrS->r = celRS + xcS ;
    ptrS->b = celBS + ycS ;
    ptrS->t = celTS + ycS ;
    /* note whether cell is multiple tile or not */
    if( multiS > 1 ){
	ptrS->multi = TRUE ;
    } else {
	ptrS->multi = FALSE ;
    }
} /* end endCell */

process_tiles()
{
    INT i ;
    COMPACTPTR t ;
    CELLBOXPTR cptr ;

    if( errorFlagS ){
	YexitPgm( COMPACTFAIL ) ;
    }

    /* insure that box coordinates are not zero */
    /* this insures that source and sink are outside core */
    /* see addSourceNSink below for details */
    if( boxLS == 0) boxLS-- ;
    if( boxRS == 0) boxRS++ ;
    if( boxBS == 0) boxBS-- ;
    if( boxTS == 0) boxTS++ ;

    /* update members of tileNode array to multi tile status */
    for( i = 1; i <= numtilesG; i++ ){
	t = tileNodeG[i] ;
	ASSERT( t->cell > 0 && t->cell <= numcellsG, "process_tiles",
	    "cell out of bounds" ) ;
	cptr = cellarrayG[t->cell] ;
	if( cptr->multi ){
	    t->multi = TRUE ;
	}
	/* check for zero width tiles */
	if( t->l == t->r ){
	    if( t->r >= cptr->xcenter ){
		t->l-- ;
		t->l_rel-- ;
	    } else {
		t->r++ ;
		t->r_rel++ ;
	    }
	    sprintf( YmsgG, "Zero width tile found for tile:%d\n", i ) ;
	    M( MSG, "process_tiles", YmsgG ) ;
	} 
	if( t->b == t->t ){
	    if( t->t >= cptr->ycenter ){
		t->b-- ;
		t->b_rel-- ;
	    } else {
		t->t++ ;
		t->t_rel++ ;
	    }
	    sprintf( YmsgG, "Zero width tile found for tile:%d\n", i ) ;
	    M( MSG, "process_tiles", YmsgG ) ;
	} 
    }

    multi_tiles() ;

    addSourceNSink() ;
    /* now xGraphS and yGraphS have all the tiles */

    /* initialization complete */
} /* end process_tiles */

/* ADD source and sink nodes to both x and y graphs */
addSourceNSink()
{
    COMPACTPTR source, sink ;
    INT x ;

    /* We do this little piece of code since a couple of stupid compilers */
    /* can't do math correctly.  Gcc being one of them. */
    x = INT_MIN ;
    int_smallS = x / 2 ;
    x = INT_MAX ;
    int_largeS = x / 2 ;

    /*********** XGRAPH ***********/
    /* add source and sink nodes for x graph */
    source = xGraphG[0] = tileNodeG[0] =
	(COMPACTPTR) Ysafe_malloc( sizeof(COMPACTBOX) ) ;
    /* add 10% to make graph picture prettier */
    source->l = boxLS - 0.5 * (DOUBLE) ABS(boxRS - boxLS) ;
    source->r = boxLS - 0.5 * (DOUBLE) ABS(boxRS - boxLS) ;
    source->b = int_smallS ;
    source->t = int_largeS ;
    source->l_rel = 0 ;
    source->r_rel = 0 ;
    source->b_rel = boxBS ;
    source->t_rel = boxTS ;
    source->node = 0 ;
    source->cell = XSOURCEC ;
    source->multi = FALSE ;
    source->type =  CELLTYPE ;
    source->xadjF = NULL ;
    source->xadjB = NULL ;
    source->yadjF = NULL ;
    source->yadjB = NULL ;
    source->direction = NODIRECTION ;

    sink = xGraphG[last_tileG] = tileNodeG[last_tileG] = 
	(COMPACTPTR) Ysafe_malloc( sizeof(COMPACTBOX) ) ;
    /* add 10% to make graph picture prettier */
    sink->l = boxRS + 0.5 * (DOUBLE) ABS(boxRS - boxLS) ; 
    sink->r = boxRS + 0.5 * (DOUBLE) ABS(boxRS - boxLS) ;
    sink->b = int_smallS ;
    sink->t = int_largeS ;
    sink->l_rel = 0 ;
    sink->r_rel = 0 ;
    sink->b_rel = boxBS ;
    sink->t_rel = boxTS ;
    sink->node = last_tileG ;
    sink->cell = XSINKC ;
    sink->multi = FALSE ;
    sink->type =  CELLTYPE ;
    sink->xadjF = NULL ;
    sink->xadjB = NULL ;
    sink->yadjF = NULL ;
    sink->yadjB = NULL ;
    sink->direction = NODIRECTION ;

    /*********** YGRAPH ***********/
    /* add source and sink nodes for y graph */
    source = yGraphG[0] = tileNodeG[numtilesG+2] =
	(COMPACTPTR) Ysafe_malloc( sizeof(COMPACTBOX) ) ;
    /* add 10% to make graph picture prettier */
    source->b = boxBS - 0.5 * (DOUBLE) ABS(boxTS - boxBS) ;
    source->t = boxBS - 0.5 * (DOUBLE) ABS(boxTS - boxBS) ;
    source->l = int_smallS ;
    source->r = int_largeS ;
    source->b_rel = 0 ;
    source->t_rel = 0 ;
    source->l_rel = boxLS ;
    source->r_rel = boxRS ;
    source->node = numtilesG + 2 ;
    source->cell = YSOURCEC ;
    source->multi = FALSE ;
    source->type =  CELLTYPE ;
    source->xadjF = NULL ;
    source->xadjB = NULL ;
    source->yadjF = NULL ;
    source->yadjB = NULL ;
    source->direction = NODIRECTION ;

    sink = yGraphG[last_tileG] = tileNodeG[numtilesG+3] = 
	(COMPACTPTR) Ysafe_malloc( sizeof(COMPACTBOX) ) ;
    /* add 10% to make graph picture prettier */
    sink->b = boxTS + 0.5 * (DOUBLE) ABS(boxTS - boxBS) ; 
    sink->t = boxTS + 0.5 * (DOUBLE) ABS(boxTS - boxBS) ;
    sink->l = int_smallS ;
    sink->r = int_largeS ;
    sink->b_rel = 0 ;
    sink->t_rel = 0 ;
    sink->l_rel = boxLS ;
    sink->r_rel = boxRS ;
    sink->node = numtilesG + 3 ;
    sink->cell = YSINKC ;
    sink->multi = FALSE ;
    sink->type =  CELLTYPE ;
    sink->xadjF = NULL ;
    sink->xadjB = NULL ;
    sink->yadjF = NULL ;
    sink->yadjB = NULL ;
    sink->direction = NODIRECTION ;
} /* addSourceNSink() */

/* ***************************************************************** 
    OUTPUT routine - output the results.
   **************************************************************** */
output()
{
    INT c ;
    INT tile ;
    INT old_cell ;
    FILE *fp ;
    COMPACTPTR t ;
    CELLBOXPTR cptr ;
    char filename[LRECL] ;

    sprintf( filename, "%s.mcpt", cktNameG ) ;
    fp = TWOPEN( filename, "w", ABORT ) ;

    /* account for sink tile */
    fprintf( fp, "numtiles:%d numcells:%d\n", numtilesG, numcellsG ) ;
    old_cell = 0 ;
    for( tile = 1; tile <= numtilesG; tile++ ){
	t = tileNodeG[tile] ;
	c = t->cell ;
	if( c != old_cell ){
	    /* update old_cell */
	    cptr = cellarrayG[c] ;
	    old_cell = c ;
	    /* determine cell type */
	    if( cptr->type == STDCELLTYPE ){ /* partitioned stdcell */
		/* output cell heading then tile */
		fprintf( fp, "stdcell %d x:%d y:%d offset:%d %d\n", 
		    cptr->cellnum, cptr->xcenter, cptr->ycenter,
		    cptr->xoffset, cptr->yoffset ) ;
	    } else { /* normal mc type - hard or soft cell */
		/* output cell heading then tile */
		fprintf( fp, "cell %d x:%d y:%d offset:%d %d\n", 
		    cptr->cellnum, cptr->xcenter, cptr->ycenter,
		    cptr->xoffset, cptr->yoffset ) ;
	    }
	}
	/* just output tile */
	fprintf( fp, "l:%d r:%d b:%d t:%d\n", t->l_rel, t->r_rel,
	    t->b_rel, t->t_rel ) ;
    } /* end cell loop */

    TWCLOSE( fp ) ;
} /* end output */
