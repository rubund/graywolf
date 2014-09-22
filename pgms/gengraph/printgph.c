/* ----------------------------------------------------------------- 
FILE:	    printgph.c                                       
DESCRIPTION:This file contains the routines for the printing out the 
	    channel graph.
CONTENTS:   int printgph( )
DATE:	    May 01, 1988 
REVISIONS:  Jan 23, 1989 - moved adjacency graph code to rectlin.c
	    May 21, 1989 - need to add edge dimension for detail route.
	    Sun Dec 16 01:28:42 EST 1990 - added width to graph.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) printgph.c version 1.2 12/16/90" ;
#endif

#include <geo.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>

/* ***************************************************************** 
    Print graph outputs the channel graph.  The .rte file contains
    detailed information and the .gph contains information necessary
    for the global router.
   ***************************************************************** */
printgph( crossref )
int *crossref ;
{

char filename[64] ;
FILE *fp , *fpr ;
int i , index1 , index2 , length , width ;
ADJPTR curEdge ;
EBOXPTR edge ; 
int node ;
int cell_rt, cell_lb ;
int chanEdgeIndex ;
int edge1, edge2 ;
int numedges ;
int numnodes ;

sprintf( filename, "%s.mrte", cktNameG ) ;
fpr = TWOPEN( filename , "w", ABORT ) ;
sprintf( filename, "%s.mgph", cktNameG ) ;
fp = TWOPEN( filename , "w", ABORT ) ;

/* first output gph file */
numedges = 0 ;
for( i = 1 ; i <= eNumG ; i++ ) {
    if( eArrayG[i].active == TRUE ) {
	index1 = eArrayG[i].index1 ;
	index2 = eArrayG[i].index2 ;
	if( rectArrayG[index1].active && rectArrayG[index2].active ){
	    numedges++ ;
	    index1 = crossref[index1] ;
	    index2 = crossref[index2] ;
	    length = eArrayG[i].length ;
	    width  = eArrayG[i].width  ;
	    fprintf( fp, "edge %5d %5d   length %8d  capacity %8d ",
				index1 , index2 , length , width ) ;
	    if( i <= edgeTransitionG ){
		/* vertical edge */
		fprintf( fp, "width %d\n", track_pitchXG ) ;
	    } else {
		/* horizontal edge */
		fprintf( fp, "width %d\n", track_pitchYG ) ;
	    }
	}
    }
}
TWCLOSE(fp);

/* count number of active nodes */
numnodes = 0 ;
for( i = 1; i<= numRectsG ; i++ ){
    if( rectArrayG[i].active ){
	numnodes++ ;
    }
}

/* next output rte file */
fprintf( fpr, "numnodes:%d numedges:%d\n", numnodes , numedges ) ;
for( i = 1; i<= numRectsG ; i++ ){
    if(!(rectArrayG[i].active)){
	continue ;
    }
    fprintf( fpr,"node:%d x:%d y:%d ",crossref[i], rectArrayG[i].xc,
	rectArrayG[i].yc);
    fprintf( fpr,"l:%d r:%d b:%d t:%d\n",rectArrayG[i].node_l,
	rectArrayG[i].node_r,rectArrayG[i].node_b,rectArrayG[i].node_t );
    for( curEdge = rectArrayG[i].adj; curEdge ; curEdge = curEdge->next ){
	node = curEdge->node ;
	if(!(rectArrayG[node].active)){
	    continue ;
	}
	edge = curEdge->edge ;
	ASSERTNCONT( edge, "printgph", "Edge is NULL" ) ;
	length = edge->length ;
	width = edge->width ;
	fprintf( fpr,"\tadj node:%d len:%d cap:%d ",
	    crossref[node],length, width );

	/* first convert channel nodes to an edge */
	chanEdgeIndex = eIndexArrayG[i][node] ;
	ASSERTNCONT( chanEdgeIndex,"printgph","chanEdgeIndex is zero" ) ;

	/* get orientation of channel */
	if( chanEdgeIndex <= edgeTransitionG ) {
	    /* vertical */
	    fprintf( fpr,"l:%d r:%d b:%d t:%d ",
		edge->lbside, edge->rtside, edge->loend, edge->hiend ) ;
	    fprintf( fpr, "HnotV:0 " ) ;
	} else {
	    /* horizontal */
	    fprintf( fpr,"l:%d r:%d b:%d t:%d ",
		edge->loend, edge->hiend, edge->lbside, edge->rtside ) ;
	    fprintf( fpr, "HnotV:1 " ) ;
	}

	/* now use eArray to find border edges in edgeList */
	edge1 = eArrayG[chanEdgeIndex].edge1 ;
	edge2 = eArrayG[chanEdgeIndex].edge2 ;

	if( edgeListG[edge1].cell > 0 ){
	    /* left or bottom cell */
	    cell_lb = edgeListG[edge1].cell ;
	} else {
	    cell_lb = 0 ;
	}
	fprintf( fpr, "lb:%d ", cell_lb ) ;

	if( edgeListG[edge2].cell > 0 ){
	    /* right or top cell */
	    cell_rt = edgeListG[edge2].cell ;
	} else {
	    cell_rt = 0 ;
	}
	fprintf( fpr, "rt:%d\n", cell_rt ) ;
    }
}
TWCLOSE(fpr);

} /* end printgph */
