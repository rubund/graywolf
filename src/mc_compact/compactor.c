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
FILE:	    compactor.c
DESCRIPTION:This file contains main control for compaction algorithm.
CONTENTS:   
DATE:	    Apr  8, 1988 
REVISIONS:  Nov  5, 1988 - free violations and modified position of
		sources and sinks.
	    Dec  4, 1988 - corrected error in where to get data.
	    Jan 15, 1989 - fixed constraint problem for softcells
		by saving contents of tilebox and set orig_ fields 
		correctly for compaction cycle for softcells.
	    Jan 25, 1989 - removed incorrect force of box size to
		block area and added \n for new message macro.
	    Mar 11, 1989 - added graphics conditional compile and
		commented out compactor state dump.
	    Mar 30, 1989 - changed tile datastructure.
	    Apr 17, 1989 - changed compactor to independent program.
	    Apr 30, 1989 - changed compaction algorithm so it is both
		easier to read and more robust.
	    May  3, 1989 - changed to Y prefixes.
	    Sun Nov  4 13:18:12 EST 1990 - added set_draw_critical
		for making a pretty picture.
	    Fri Mar 29 14:25:29 EST 1991 - now save the critical
		path.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) compactor.c version 7.3 3/29/91" ;
#endif

#include <compact.h>
#include <yalecad/debug.h>

remove_violations()
{
    ERRORPTR  violations, saveError, buildXGraph(), buildYGraph() ;

    /* --------------------------------------------------------------- 
       VIOLATION REMOVAL CYCLE - iterate till all violations are removed
    */
    while( TRUE ){
	buildXGraph() ;
	violations = buildYGraph() ; 
	if( violations ){ /* violations unresolved */
	    /* move strategy is to resolve overlap violations */
	    moveStrategy( violations ) ;
	} else {
	    break ; /* exit loop */
	}

	D( "mc_compact/remove_violations", MEMUSAGE ) ;
	
	/* G( ) is NOGRAPHICS conditional compile */
	G( if( graphicsG && TWinterupt() ) ){
	    G( process_graphics() ) ;
	}
	G( draw_the_data() ) ;

	freeGraph(XFORWARD ) ;
	freeGraph(XBACKWARD ) ;
	freeGraph(YFORWARD ) ;
	freeGraph(YBACKWARD ) ;

	D( "mc_compact/remove_violations", MEMUSAGE ) ;

    } /* ----- END VIOLATION REMOVAL CYCLE ------ */

    freeGraph(XFORWARD ) ;
    freeGraph(XBACKWARD ) ;
    freeGraph(YFORWARD ) ;
    freeGraph(YBACKWARD ) ;
    G( draw_the_data() ) ;

} /* end remove_violations */


compact()
{
    INT length ;       /* length of longest path */
    INT count ;        /* number of compaction cycles */
    ERRORPTR  violations, saveError, buildXGraph(), buildYGraph() ;
    BOOL compactNotSat ; /* compaction criteria is not satisfied */
    BOOL xNotY_toggle ; /* toggle between x and y compaction. */

    /* --------------------------------------------------------------- 
       COMPACTION CYCLE - iterate till all violations are removed
	    and compaction criteria is satisfied.
    */
    count = 0 ;
    compactNotSat = TRUE ;
    xNotY_toggle  = TRUE ;
    if(!(debugG)) fprintf( stderr,"\nCompaction Begins...\n" ) ;
    while( compactNotSat ){
	/* first build x and y graphs */
	buildXGraph() ;
	violations = buildYGraph() ; 
	if( violations ){ /* violations unresolved */
	    /* move strategy is to resolve overlap violations */
	    G( set_draw_critical( FALSE ) ) ; 
	    moveStrategy( violations ) ;
	    fprintf( stderr,"V " ) ;
	    D( "mc_compact/viofail",
		YexitPgm( PGMFAIL ) ;
	    ) ;
	    if( debugG ) {
		fprintf( stderr, "\n" ) ;
	    }
	} else {
	    G( set_draw_critical( TRUE ) ) ; 
	    if( xNotY_toggle ){
		fprintf( stderr,"X " ) ;
		length = longestxPath( TRUE ) ;
		/* move strategy is to compact in x direction */
		move_compactx( length );
		xNotY_toggle = FALSE ; /* flip toggle */
	    } else {
		fprintf( stderr,"Y " ) ;
		length = longestyPath( TRUE ) ;
		/* move strategy is to compact in y direction */
		move_compacty( length );
		xNotY_toggle = TRUE ; /* flip toggle */
	    }
	    compactNotSat = test_area() ;
	    if( debugG) {
		fprintf( stderr, "\n" ) ;
	    }

	}

	if( (!(debugG)) && (++count % 15) == 0 ){ 
	    fprintf( stderr, "\n" ) ;
	}

	D( "mc_compact/compact", MEMUSAGE ) ;
	
	/* G( ) is NOGRAPHICS conditional compile */
	G( if( graphicsG && TWinterupt() ) ){
	    G( process_graphics() ) ;
	}
	G( draw_the_data() ) ;

	freeGraph(XFORWARD) ;
	freeGraph(XBACKWARD) ;
	freeGraph(YFORWARD) ;
	freeGraph(YBACKWARD) ;

	D( "mc_compact/compact", MEMUSAGE ) ;

    } /* ----- END COMPACTION CYCLE ------ */

    if(!(debugG)){ 
	fprintf( stderr, "\n\n" ) ;
    }

    /* grid the cells */
    grid_data() ;

    /* make sure we have no violations */
    remove_violations() ;

    /* show the results */
    G( draw_the_data() ) ;

} /* end compact */


freeGraph( direction ) 
INT direction ;
{
    INT i ;
    ECOMPBOXPTR edge , saveEdge ;

    for( i=0 ; i<= last_tileG ; i++ ){
	switch( direction ){
	    case XFORWARD: edge=xGraphG[i]->xadjF;
		xGraphG[i]->xadjF = NULL ;
		break ;
	    case XBACKWARD: edge=xGraphG[i]->xadjB;
		xGraphG[i]->xadjB = NULL ;
		break ;
	    case YFORWARD:  edge=yGraphG[i]->yadjF;
		yGraphG[i]->yadjF = NULL ;
		break ;
	    case YBACKWARD: edge=yGraphG[i]->yadjB;
		yGraphG[i]->yadjB = NULL ;
		break ;
	}
	/* free all the edges */
	for( ;edge; ){ 
	    saveEdge = edge ;
	    edge = edge->next ;
	    Ysafe_free(saveEdge) ;
	}
    } /* end for loop */
}

INT path(direction)
INT direction ;
{
    INT node ;
    INT source ;


    switch( direction ){
    case XFORWARD : node = numtilesG + 1 ;
	if( constraintsG ){
	    Ydeck_empty( path_deckG, NIL(VOIDPTR)) ;
	}
	D( "mc_compact/path",
	    printf("Longest path in xGraphG-Forward direction:\n" ) ) ;
	source = 0 ;
	/* print backwards so list is in correct order columatted */
	while( node != source ){
	    D( "mc_compact/path",
	    printf("Node:%2d cell:%2d xvalueMin:%4d\n",
		tileNodeG[node]->node,
		tileNodeG[node]->cell,
		tileNodeG[node]->xvalueMin ) ) ;
	    tileNodeG[node]->criticalX = TRUE ;
	    /* update node */
	    node = tileNodeG[node]->pathx ;
	    if( constraintsG && node != source ){
		Ydeck_push( path_deckG, (VOIDPTR) node ) ;
	    }
	}
	return( tileNodeG[numtilesG+1]->xvalueMin ) ;
    case XBACKWARD: node = 0 ;
	D( "mc_compact/path",
	printf("Longest path in xGraphG-Backward direction:\n" ) ) ;
	source = numtilesG + 1 ;
	/* print backwards so list is in correct order columatted */
	while( node != source ){
	    D( "mc_compact/path",
	    printf("Node:%2d cell:%2d xvalueMax:%4d\n",
		tileNodeG[node]->node,
		tileNodeG[node]->cell,
		tileNodeG[node]->xvalueMax ) ) ;
	    /* update node */
	    node = tileNodeG[node]->pathx ;
	}
	return( tileNodeG[0]->xvalueMax ) ;
    case YFORWARD : node = numtilesG + 3 ;
	if( constraintsG ){
	    Ydeck_empty( path_deckG, NIL(VOIDPTR)) ;
	}
	D( "mc_compact/path",
	printf("Longest path in yGraphG-Forward direction:\n" ) );
	source = numtilesG + 2 ;
	/* print backwards so list is in correct order columatted */
	while( node != source ){
	    D( "mc_compact/path",
	    printf("Node:%2d cell:%2d yvalueMin:%4d\n",
		tileNodeG[node]->node,
		tileNodeG[node]->cell,
		tileNodeG[node]->yvalueMin ) );
	    tileNodeG[node]->criticalY = TRUE ;
	    /* update node */
	    node = tileNodeG[node]->pathy ;
	    if( constraintsG && node != source ){
		Ydeck_push( path_deckG, (VOIDPTR) node ) ;
	    }
	}
	return( tileNodeG[numtilesG+3]->yvalueMin ) ;
    case YBACKWARD: node = numtilesG + 2 ;
	D( "mc_compact/path",
	printf("Longest path in yGraphG-Backward direction:\n" ) ) ;
	source = numtilesG + 3 ;
	while( node != source ){
	    D( "mc_compact/path",
	    printf("Node:%2d cell:%2d yvalueMax:%4d\n",
		tileNodeG[node]->node,
		tileNodeG[node]->cell,
		tileNodeG[node]->yvalueMax ) ) ;
	    /* update node */
	    node = tileNodeG[node]->pathy ;
	}
	return( tileNodeG[numtilesG+2]->yvalueMax ) ;
    default:
	M(ERRMSG,"dpath","invalid direction in graph\n") ;
	return( 0 ) ;
    }
}

cleanupGraph( direction ) 
INT direction ;
{
    INT i ;
    ECOMPBOXPTR edge , match, saveNode ;
    INT matchNode, sameNode ;

    for( i=0 ; i<= last_tileG ; i++ ){
	switch( direction ){
	    case XFORWARD: edge=xGraphG[i]->xadjF;
	        sameNode = xGraphG[i]->node ; 
		break ;
	    case XBACKWARD: edge=xGraphG[i]->xadjB;
		sameNode = xGraphG[i]->node ; 
		break ;
	    case YFORWARD:  edge=yGraphG[i]->yadjF;
		sameNode = yGraphG[i]->node ; 
		break ;
	    case YBACKWARD: edge=yGraphG[i]->yadjB;
		sameNode = yGraphG[i]->node ; 
		break ;
	}
	for( ;edge;edge=edge->next ){ 
	    matchNode = edge->node ;
	    saveNode = edge ;
	    for(match=edge->next; match ; ){
		if( match->node == matchNode ){ 

		    D( "mc_compact/cleanupGraph",
			sprintf( YmsgG, 
			"We discovered redundant edge-N%d -> N%d\n",
			sameNode, match->node ) ;
		        M(MSG, NULL, YmsgG )
		    ) ;

		    /* need to update ancestors */
		    /* the edge in question is sameNode:match->node */
		    switch( direction ){
			case XFORWARD: 
			    tileNodeG[matchNode]->xancestrF-- ;
			    break ;
			case XBACKWARD: 
			    tileNodeG[matchNode]->xancestrB-- ;
			    break ;
			case YFORWARD:  
			    tileNodeG[matchNode]->yancestrF-- ;
			    break ;
			case YBACKWARD:
			    tileNodeG[matchNode]->yancestrB-- ;
			    break ;
		    }

		    ASSERT( edge->constraint == match->constraint,
		      "cleanupGraph",
		      "Redundant edges have mismatching constraints\n");

		    /* now delete */
		    saveNode->next = match->next ;
		    Ysafe_free( match ) ;

		    /* update loop */
		    match = saveNode->next ;

		} else if( match->node == sameNode){ 
		    /* a node constrainted to itself delete it. */

		    D( "mc_compact/cleanupGraph",
			sprintf( YmsgG, 
			"We discovered redundant edge-N%d -> N%d\n",
			sameNode, match->node ) ;
		        M(MSG, NULL, YmsgG )
		    ) ;

		    /* need to update ancestors */
		    /* the edge in question is sameNode:match->node */
		    switch( direction ){
			case XFORWARD: 
			    tileNodeG[sameNode]->xancestrF-- ;
			    break ;
			case XBACKWARD: 
			    tileNodeG[sameNode]->xancestrB-- ;
			    break ;
			case YFORWARD:  
			    tileNodeG[sameNode]->yancestrF-- ;
			    break ;
			case YBACKWARD:
			    tileNodeG[sameNode]->yancestrB-- ;
			    break ;
		    }

		    /* now delete */
		    saveNode->next = match->next ;
		    Ysafe_free( match ) ;

		    /* update loop */
		    match = saveNode->next ;

		} else {
		    /* edge is OK */

		    /* update loop */
		    saveNode = match ;
		    match = match->next ;
		}
	    }
	}
    } /* end for loop */
}

/* find bounding box of tiles */
find_core( l, r, b, t )
INT *l, *r, *b, *t ;
{

    INT i ;
    INT expand ;
    COMPACTPTR tile ;

    blocklG = INT_MAX ;
    blockbG = INT_MAX ;
    blockrG = INT_MIN ;
    blocktG = INT_MIN ;

    for( i=1; i <= numtilesG; i++ ){
	tile = tileNodeG[i] ;
	blocklG = MIN( blocklG, tile->l ) ;
	blockbG = MIN( blockbG, tile->b ) ;
	blockrG = MAX( blockrG, tile->r ) ;
	blocktG = MAX( blocktG, tile->t ) ;
    }
    expand = (INT) ( 0.25 * (DOUBLE) ABS(blockrG - blocklG) ) ;
    *l = blocklG -= expand ;
    *r = blockrG += expand ;
    expand = (INT) ( 0.25 * (DOUBLE) ABS(blocktG - blockbG) ) ;
    *b = blockbG -= expand ;
    *t = blocktG += expand ;

} /* end find_core */
