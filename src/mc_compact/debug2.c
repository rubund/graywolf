/*
 *   Copyright (C) 1988-1990 Yale University
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
FILE:	    debug2.c                                       
DESCRIPTION:more debug routines
CONTENTS:   
DATE:	    Feb 13, 1988 
REVISIONS:  Jan 29, 1989 - changed msg to msgG and added \n's.
	    Mar 30, 1989 - changed data structure of tiles.
	    Apr 17, 1989 - moved from placer into indep. program.
	    Apr 20, 1989 - now write debug to a file.
	    Sun Nov  4 13:22:21 EST 1990 - added new debug function
		for displaying cell slacks.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) debug2.c version 7.1 11/10/90" ;
#endif

#include <compact.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>

dumpxGraph()
{
    int  i ;
    ECOMPBOXPTR eptr ;
    FILE *fp ;
    int xmin, xmax ;

    /****************** XGRAPH *******************************/

    fp = TWOPEN( "debug", "w", ABORT ) ;
    for( i=0;i<=last_tileG;i++){
	fprintf( fp,"Node:%2d cell:%2d l:%5d r:%5d b:%5d t:%5d\n",
	    tileNodeG[i]->node, tileNodeG[i]->cell, tileNodeG[i]->l,
	    tileNodeG[i]->r, tileNodeG[i]->b, tileNodeG[i]->t ) ;
	fprintf( fp," xvMin:%d xvMax:%d yvMin:%d yvMax:%5d pathx:%d pathy:%d\n",
	    tileNodeG[i]->xvalueMin, tileNodeG[i]->xvalueMax, tileNodeG[i]->yvalueMin,
	    tileNodeG[i]->yvalueMax, tileNodeG[i]->pathx, tileNodeG[i]->pathy ) ;
	/* find window */
	/* xmin is max of all left edges */
	xmin = tileNodeG[i]->xvalueMin ;
	for( eptr = tileNodeG[i]->xadjB ;eptr; eptr=eptr->next){
	    xmin = MAX( xmin, tileNodeG[eptr->node]->xvalueMax ) ;
	}
	xmax = tileNodeG[i]->xvalueMax ;
	for( eptr = tileNodeG[i]->xadjF ;eptr; eptr=eptr->next){
	    xmax = MIN( xmax, tileNodeG[eptr->node]->xvalueMin ) ;
	}
	fprintf( fp,"xmin:%d xmax:%d\n",xmin, xmax ) ;
	fprintf( fp," xancF:%d xancB:%d yancF:%d yancB:%d\n",
	    tileNodeG[i]->xancestrF,
	    tileNodeG[i]->xancestrB,
	    tileNodeG[i]->yancestrF,
	    tileNodeG[i]->yancestrB ) ;
	fprintf( fp," Edges for node %d:\n", i );
	for( eptr = tileNodeG[i]->xadjB ;eptr; eptr=eptr->next){
	    fprintf( fp,"     node %2d constraint:%d\n",
		eptr->node, eptr->constraint ) ; 
	}
	fprintf( fp, "\n" ) ;
    }
    TWCLOSE( fp ) ;
}

BOOL dxancestors( numtiles, xGraph ) 
int numtiles ;
COMPACTPTR *xGraph ;
{
    int i, fcount = 0, bcount = 0 ;
    int fanc = 0 , banc = 0 ;
    int fnode = 0 , bnode = 0 ;
    ECOMPBOXPTR edge ;

    for( i=0;i<=numtiles; i++ ){
	fanc += xGraph[i]->xancestrF ;
	fnode = 0 ;
	for( edge=xGraph[i]->xadjF;edge;edge=edge->next ){
	    fcount++ ;
	    fnode++ ;
	}
    }
    for( i=0;i<=numtiles; i++ ){
	banc += xGraph[i]->xancestrB ;
	bnode = 0 ;
	for( edge=xGraph[i]->xadjB;edge;edge=edge->next ){
	    bcount++ ;
	    bnode++ ;
	}
    }
    if( fcount != bcount || fcount != fanc || bcount != banc ){
	printf( "Forward edges:%d Backward edges:%d\n",fcount,bcount ) ;
	printf( "Forward nodes:%d Backward nodes:%d\n",fnode,bnode ) ;
	printf( "Forward ancestrs:%d Backward ancestrs:%d\n",fanc,banc );
	return(FALSE) ;
    }
    return(TRUE) ;

}

dumpyGraph()
{
    int  i ;
    ECOMPBOXPTR eptr ;
    FILE *fp ;

    fp = TWOPEN( "debug", "w", ABORT ) ;
    /****************** YGRAPH *******************************/
    /* SOURCE */
    fprintf(fp,"Node:%2d cell:%2d l:%5d r:%5d b:%5d t:%5d\n",
	tileNodeG[numtilesG+2]->node, tileNodeG[numtilesG+2]->cell,
	tileNodeG[numtilesG+2]->l,
	tileNodeG[numtilesG+2]->r, tileNodeG[numtilesG+2]->b,
	tileNodeG[numtilesG+2]->t ) ;
    fprintf(fp," xvMin:%d xvMax:%d yvMin:%d yvMax:%5d pathx:%d pathy:%d\n",
	tileNodeG[numtilesG+2]->xvalueMin, 
	tileNodeG[numtilesG+2]->xvalueMax,
	tileNodeG[numtilesG+2]->yvalueMin,
	tileNodeG[numtilesG+2]->yvalueMax,
	tileNodeG[numtilesG+2]->pathx, tileNodeG[numtilesG+2]->pathy ) ;
    fprintf(fp," Edges for node %d:\n", numtilesG+2 );
    for( eptr = tileNodeG[numtilesG+2]->yadjB ;eptr; eptr=eptr->next){
	fprintf(fp,"     node %2d constraint:%d\n",
	    eptr->node, eptr->constraint ) ; 
    }

    for( i=1;i<=numtilesG;i++){
	fprintf(fp,"Node:%2d cell:%2d l:%5d r:%5d b:%5d t:%5d\n",
	    tileNodeG[i]->node, tileNodeG[i]->cell, tileNodeG[i]->l,
	    tileNodeG[i]->r, tileNodeG[i]->b, tileNodeG[i]->t ) ;
	fprintf(fp," xvMin:%d xvMax:%d yvMin:%d yvMax:%5d pathx:%d pathy:%d\n",
	    tileNodeG[i]->xvalueMin, tileNodeG[i]->xvalueMax,
	    tileNodeG[i]->yvalueMin,
	    tileNodeG[i]->yvalueMax, tileNodeG[i]->pathx,
	    tileNodeG[i]->pathy ) ;
	fprintf(fp," Edges for node %d:\n", i );
	for( eptr = tileNodeG[i]->yadjB ;eptr; eptr=eptr->next){
	    fprintf(fp,"     node %2d constraint:%d\n",
		eptr->node, eptr->constraint ) ; 
	}
    }

    /* SINK */
    fprintf(fp,"Node:%2d cell:%2d l:%5d r:%5d b:%5d t:%5d\n",
	tileNodeG[numtilesG+3]->node, tileNodeG[numtilesG+3]->cell,
	tileNodeG[numtilesG+3]->l,
	tileNodeG[numtilesG+3]->r, tileNodeG[numtilesG+3]->b,
	tileNodeG[numtilesG+3]->t ) ;
    fprintf(fp," xvMin:%d xvMax:%d yvMin:%d yvMax:%5d pathx:%d pathy:%d\n",
	tileNodeG[numtilesG+3]->xvalueMin,
	tileNodeG[numtilesG+3]->xvalueMax,
	tileNodeG[numtilesG+3]->yvalueMin,
	tileNodeG[numtilesG+3]->yvalueMax,
	tileNodeG[numtilesG+3]->pathx, tileNodeG[numtilesG+3]->pathy ) ;
    fprintf(fp," Edges for node %d:\n", i );
    for( eptr = tileNodeG[numtilesG+3]->yadjB ;eptr; eptr=eptr->next){
	fprintf(fp,"     node %2d constraint:%d\n",
	    eptr->node, eptr->constraint ) ; 
    }
    fprintf( fp, "\n" ) ;
    TWCLOSE( fp ) ;

}


ECOMPBOXPTR find_edge( node1, node2, direction )
int node1, node2 ;
int direction ;
{
    ECOMPBOXPTR eptr ;
    switch( direction ){
	case XFORWARD:
	    eptr = tileNodeG[node1]->xadjF;
	    break ;
	case XBACKWARD:
	    eptr = tileNodeG[node1]->xadjB;
	    break ;
	case YFORWARD:
	    eptr = tileNodeG[node1]->yadjF;
	    break ;
	case YBACKWARD:
	    eptr = tileNodeG[node1]->yadjB;
	    break ;
    }
    for( ; eptr; eptr = eptr->next ){
	if( eptr->node == node2 ){
	    return( eptr ) ;
	}
    }
    return( NULL ) ;
}

dycons()
{
    int  i, bcons ;
    ECOMPBOXPTR eptr, eptr2 ;

    /****************** YGRAPH *******************************/
    /* SOURCE */
    printf(" Edges for node %d:\n", numtilesG+2 );
    for( eptr = tileNodeG[numtilesG+2]->yadjF ;eptr; eptr=eptr->next){
	
	if( eptr2 = find_edge( eptr->node, numtilesG+2, YBACKWARD ) ){
	    bcons = eptr2->constraint ;
	} else {
	    bcons = -1 ;
	}
	printf("     node %2d fconstraint:%d bconstraint:%d\n",
	    eptr->node, eptr->constraint, bcons ) ; 
    }

    for( i=1;i<=numtilesG;i++){
	printf(" Edges for node %d:\n", i );
	for( eptr = tileNodeG[i]->yadjF ;eptr; eptr=eptr->next){
	    if( eptr2 = find_edge( eptr->node, i, YBACKWARD ) ){
		bcons = eptr2->constraint ;
	    } else {
		bcons = -1 ;
	    }
	    printf("     node %2d fconstraint:%d bconstraint:%d\n",
		eptr->node, eptr->constraint, bcons ) ; 
	}
    }

    /* SINK */
    printf(" Edges for node %d:\n", i );
    for( eptr = tileNodeG[numtilesG+3]->yadjF ;eptr; eptr=eptr->next){
	if( eptr2 = find_edge( eptr->node, numtilesG+3, YBACKWARD ) ){
	    bcons = eptr2->constraint ;
	} else {
	    bcons = -1 ;
	}
	printf("     node %2d fconstraint:%d bconstraint:%d\n",
	    eptr->node, eptr->constraint, bcons ) ; 
    }

}



dsort( numtiles, XNotY )
int numtiles ;
BOOL XNotY ;
{
    int i ;
    COMPACTPTR *ptr ;

    fprintf(stderr,"Dumping tileNodeG - node:cell...\n") ;

    ptr = tileNodeG ;
    for( i=1;i<=numtiles;i++ ){
	fprintf(stderr, "%d:%d  ", ptr[i]->node,ptr[i]->cell ) ; 
	if( ( i % 10 ) == 0 ){
	    fprintf( stderr, "\n") ;
	}
    }
    fprintf(stderr,"\n") ;

    if( XNotY ){
	fprintf(stderr,"\nDumping xGraph...\n") ;
	ptr = xGraphG ;
    } else {
	fprintf(stderr,"\nDumping yGraph...\n") ;
	ptr = yGraphG ;
    }

    for( i=0;i<=last_tileG;i++ ){
	fprintf(stderr, "%d:%d  ", ptr[i]->node,ptr[i]->cell ) ; 
	if( i != 0 && ( i % 10 ) == 0 ){
	    fprintf( stderr, "\n") ;
	}
    }
    fprintf(stderr,"\n") ;

}  /* end numtiles */

dxancerr()
{
    int i ;

    for( i=0;i<=last_tileG; i++ ){
	if( xGraphG[i]->xancestrF != 0 ){
	    fprintf( stderr, "Tile:%d has %d forward ancestors\n",
		xGraphG[i]->node, xGraphG[i]->xancestrF ) ;
	}
    }
    for( i=0;i<=last_tileG; i++ ){
	if( xGraphG[i]->xancestrB != 0 ){
	    fprintf( stderr, "Tile:%d has %d backward ancestors\n",
		xGraphG[i]->node, xGraphG[i]->xancestrB ) ;
	}
    }
}

dump_anc()
{
    INT i ;
    INT last ;

    last = YSINK ;
    fprintf( stderr, "The ancestors for the tiles:\n" ) ;
    for( i = 0; i <= last; i++ ){
	fprintf( stderr, "\ttile:%3d ancestors:%d\n", i, ancestorG[i] ) ;
    }
} /* dump_anc */

dslack( XNotY, center, length )
BOOL XNotY ;
BOOL center ;
INT length ;
{
    INT i ;
    INT count ;
    INT value ;
    NODEPTR nptr ;
    CELLBOXPTR cptr ;
    COMPACTPTR tptr ;

    if( XNotY ){
	fprintf(stderr,"\nDumping xGraph slacks node:minslack-maxslack...\n") ;
	for( i=1;i<=numcellsG;i++ ){
	    cptr = slackG[i] ;
	    fprintf(stderr, "%d:", tileNodeG[cptr->tiles->node]->cell ) ;
	    fprintf(stderr, "%d-%d\n", cptr->xmin, cptr->xmax ) ;
	    count = 0 ;
	    for( nptr = cptr->tiles; nptr; nptr = nptr->next ){
		tptr = tileNodeG[nptr->node] ;
		value = tptr->xvalueMin ;
		if( center ){
		    value -= tptr->l_rel ;
		}
		fprintf(stderr, "\t%d:%d-", tptr->node, value ) ;

		value = tptr->xvalueMax ;
		if( center ){
		    value -= tptr->l_rel ;
		}
		if( length ){
		    value = length - value ;
		}
		fprintf(stderr, "%d  ", value ) ;

		if( (++count % 4) == 0 ){
		    fprintf( stderr, "\n") ;
		}
	    }
	    fprintf( stderr, "\n") ;
	}
	fprintf(stderr,"\n\n") ;
    } else {
	fprintf(stderr,"\nDumping yGraph slacks node:minslack-maxslack...\n") ;
	for( i=1;i<=numcellsG;i++ ){
	    cptr = slackG[i] ;
	    fprintf(stderr, "%d:", tileNodeG[cptr->tiles->node]->cell ) ;
	    fprintf(stderr, "%d-%d\n", cptr->ymin, cptr->ymax ) ;
	    count = 0 ;
	    for( nptr = cptr->tiles; nptr; nptr = nptr->next ){
		tptr = tileNodeG[nptr->node] ;
		value = tptr->yvalueMin ;
		if( center ){
		    value -= tptr->b_rel ;
		}
		fprintf(stderr, "\t%d:%d-", tptr->node, value ) ;

		value = tptr->yvalueMax ;
		if( center ){
		    value -= tptr->b_rel ;
		}
		if( length ){
		    value = length - value ;
		}
		fprintf(stderr, "%d  ", value ) ;
		if( (++count % 4) == 0 ){
		    fprintf( stderr, "\n") ;
		}
	    }
	    fprintf( stderr, "\n") ;
	}
	fprintf(stderr,"\n\n") ;
    }
} /* end dslack */

dedges( cell, XnotY, forwardNotBack )
INT cell ;
BOOL XnotY ;
BOOL forwardNotBack ;
{
    NODEPTR nptr ;
    CELLBOXPTR cptr ;
    COMPACTPTR tptr ;
    ECOMPBOXPTR eptr ;

    if( XnotY ){
	if( forwardNotBack ){
	    fprintf( stderr, "X forward " ) ;
	} else {
	    fprintf( stderr, "X backward " ) ;
	}
    } else {
	if( forwardNotBack ){
	    fprintf( stderr, "Y forward " ) ;
	} else {
	    fprintf( stderr, "Y backward " ) ;
	}
    }
    fprintf( stderr, "edges for cell:%d :\n", cell ) ;
    for( nptr = cellarrayG[cell]->tiles; nptr ; nptr = nptr->next ){
	tptr = tileNodeG[nptr->node] ;
	if( XnotY ){
	    if( forwardNotBack ){
		eptr = tptr->xadjF;
	    } else {
		eptr = tptr->xadjB;
	    }
	} else {
	    if( forwardNotBack ){
		eptr = tptr->yadjF;
	    } else {
		eptr = tptr->yadjB;
	    }
	}
	for( ; eptr; eptr = eptr->next ){
	    fprintf( stderr, "\t %d --> %d \n", nptr->node, eptr->node ) ;
	}
    }
    fprintf( stderr, "\n\n" ) ;
}

dyancerr()
{
    int i ;

    for( i=0;i<=last_tileG; i++ ){
	if( yGraphG[i]->yancestrF != 0 ){
	    fprintf( stderr, "Tile:%d has %d forward ancestors\n",
		yGraphG[i]->node, yGraphG[i]->yancestrF ) ;
	}
    }
    for( i=0;i<=last_tileG; i++ ){
	if( yGraphG[i]->yancestrB != 0 ){
	    fprintf( stderr, "Tile:%d has %d backward ancestors\n",
		yGraphG[i]->node, yGraphG[i]->yancestrB ) ;
	}
    }
}


check_xancestors()
{
    INT i ;
    INT count ;
    COMPACTPTR tptr ;
    ECOMPBOXPTR eptr ;

    for( i=0;i<=last_tileG; i++ ){
	count = 0 ;
	tptr = xGraphG[i] ;
	/* first check forward ancestors */
	for( eptr = tptr->xadjB ;eptr; eptr=eptr->next){
	    count++ ;
	}
	if( tptr->xancestrF != count ){
	    fprintf( stderr, 
	    "ERROR:Mismatch in forward x ancestor count for node:%d\n",
	    tptr->node ) ;
	    fprintf( stderr, "\tdata:%d found:%d\n\n", tptr->xancestrF,
		count ) ;
	}
	/* next check backward ancestors */
	count = 0 ;
	for( eptr = tptr->xadjF ;eptr; eptr=eptr->next){
	    count++ ;
	}
	if( tptr->xancestrB != count ){
	    fprintf( stderr, 
	    "ERROR:Mismatch in backward x ancestor count for node:%d\n",
	    tptr->node ) ;
	    fprintf( stderr, "\tdata:%d found:%d\n\n", tptr->xancestrB,
		count ) ;
	}

    }
} /* end check_xancestors */

check_yancestors()
{
    INT i ;
    INT count ;
    COMPACTPTR tptr ;
    ECOMPBOXPTR eptr ;

    for( i=0;i<=last_tileG; i++ ){
	count = 0 ;
	tptr = yGraphG[i] ;
	/* first check forward ancestors */
	for( eptr = tptr->yadjB ;eptr; eptr=eptr->next){
	    count++ ;
	}
	if( tptr->yancestrF != count ){
	    fprintf( stderr, 
	    "ERROR:Mismatch in forward y ancestor count for node:%d\n",
	    tptr->node ) ;
	    fprintf( stderr, "\tdata:%d found:%d\n\n", tptr->yancestrF,
		count ) ;
	}
	/* next check backward ancestors */
	count = 0 ;
	for( eptr = tptr->yadjF ;eptr; eptr=eptr->next){
	    count++ ;
	}
	if( tptr->yancestrB != count ){
	    fprintf( stderr, 
	    "ERROR:Mismatch in backward y ancestor count for node:%d\n",
	    tptr->node ) ;
	    fprintf( stderr, "\tdata:%d found:%d\n\n", tptr->yancestrB,
		count ) ;
	}

    }
} /* end check_yancestors */
