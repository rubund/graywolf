/* ----------------------------------------------------------------- 
FILE:	    polar.c                                       
DESCRIPTION:build polar graph for compaction of routing channel density.
    Algorithm:(same for y direction)
	create an array for crossreferencing merged nodes.
	for all edges in xNodules graph which connect inside a
	    cell merge their nodes to a cell node.
	with remaining edges of xNodules graph connect edges using
	    merged edge nodes as the new nodes.  If an edge crosses
	    more that one channel (empty room) add a new node to graph.
	for all edges in graph save relevent data - channel nodes,
	    width etc.
CONTENTS:   void buildxPolarGraph()
	    static addxnode( node, xpos, ypos )
		int node, xpos, ypos ;
	    static addxEdge( node1, node2, chan1, chan2, length )
		int node1, node2, chan1, chan2, length ;
	    output_xgraph( crossref )
		int *crossref ;
	    void buildyPolarGraph()
	    static addynode( node, xpos, ypos )
		int node, xpos, ypos ;
	    static addyEdge( node1, node2, chan1, chan2, length )
		int node1, node2, chan1, chan2, length ;
	    output_ygraph( crossref )
		int *crossref ;
	    swap_xsink()
	    swap_ysink()
	    static swap_rec( lastptr, sinkptr )
		POLARPTR lastptr, sinkptr ; 
DATE:	    Sep 13, 1989 
REVISIONS:  Oct 10, 1989 - fixed bug with y graph number of nodes.
		Added track_spacing to output.
	    Dec 12, 1989 - deleted track_spacing from output.
	    Apr 20, 1990 - added correct crossrefence to graphics
		for polar graph.
	    May  4, 1990 - avoid a / 0 error and instead output error
		message.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) polar.c version 1.5 6/6/91" ;
#endif

#include <geo.h>
#include <yalecad/debug.h>
#include <yalecad/file.h>
#include <yalecad/message.h>

#define HOWMANY 0
#define EXPECTEDNUMNODES 20

static int xallocS = 0 ;
static int yallocS = 0 ;
static int *xrefS ;                  /* crossreference x nodes */
static int *yrefS ;                  /* crossreference y nodes */


static addxnode();
static addxEdge();
static addynode();
static addyEdge();
static swap_rec();



void buildxPolarGraph()
{

    int  x1, y1 ;
    int  x2, y2 ;
    int  cell ;
    int  iedge, oedge ;
    int  node1, node2 ;
    int  lastE ;
    int  num, xsum, ysum ;
    int  e, firstE, nextE ;
    EDGEBOX e1ptr, e2ptr;
    FLAREPTR ifptr , ofptr ;
    int *crossref ;     /* crossreference merged nodes */
    int inode ;         /* node of xNodeArray graph */
    int onode ;         /* node of xNodeArray graph */
    int howmany ;       /* number of channels found cross this edge */
    int length ;        /* length of cell contribution to edge */

    /* allocate space for cross reference array */
    crossref = (int *) Ysafe_calloc( numXnodesG+5,sizeof(int) ) ;

    /* created expected size of array */
    xallocS = numXnodesG + EXPECTEDNUMNODES ;
    xPolarG = (POLARPTR) Ysafe_malloc( xallocS * sizeof(POLARBOX) ) ;

    /* add source and sink nodes first */
    ofptr = xNodulesG[0].outList ;
    iedge = ofptr->fixEdge ;
    e1ptr = edgeListG[iedge] ;
    x1 = e1ptr.loc ;
    y1 = ( e1ptr.start + e1ptr.end ) / 2 ;
    addxnode( 0, x1, y1 ) ;
    /* build crossreference */
    inode = e1ptr.node ;
    crossref[inode] = 0 ;

    /* sink node information not stored in graph */
    x2 = bbrG ;
    y2 = ( bbbG + bbtG ) / 2 ;
    addxnode( numberCellsG+1, x2, y2 ) ;
    onode = numXnodesG ;
    crossref[onode] = numberCellsG+1 ;

    /* now find nodes which are cells and empty rooms */
    /* this loop find nodes which are cells and build node */
    /* cross reference array */
    for( cell = 1 ; cell <= numberCellsG ; cell++ ) {
	ifptr = xNodulesG[cell].inList ;
	num = 0 ;
	xsum = 0 ;
	ysum = 0 ;
	for( ; ifptr != (FLAREPTR) NULL ; ifptr = ifptr->next ) {
	    iedge = ifptr->fixEdge ;
	    e1ptr = edgeListG[iedge] ;
	    x1 = e1ptr.loc ;
	    y1 = ( e1ptr.start + e1ptr.end ) / 2 ;

	    /* perform a cross reference for merging edges */
	    /* store new node at every oldnode */
	    inode = e1ptr.node ;
	    crossref[inode] = cell ;

	    ofptr = xNodulesG[cell].outList ;
	    for( ; ofptr != (FLAREPTR) NULL ; ofptr = ofptr->next ) {
		oedge = ofptr->fixEdge ;
		e2ptr = edgeListG[oedge] ;
		x2 = e2ptr.loc;
		y2 = (e2ptr.start + e2ptr.end ) / 2 ;
		onode = e2ptr.node ;
		crossref[onode] = cell ;
		xsum += x1 ;
		xsum += x2 ;
		ysum += y1 ;
		ysum += y2 ;
		num += 2 ;
	    }
	}
	/* place to put node is average */
	if( num <= 0 ){
	    M( ERRMSG, "buildxPolar","No nodes found in outlist\n");
	    num = 1 ;
	} 
	xsum /= num ;
	ysum /= num ;
	addxnode( cell, xsum, ysum ) ;
    }

    /* now we need to add edges to the graph */
    numxPolarG = numberCellsG + 1  ; /* we added this many nodes so far */
    for( cell = 0 ; cell <= numberCellsG + 1 ; cell++ ) {
	ofptr = xNodulesG[cell].outList ;
	for( ; ofptr != (FLAREPTR) NULL ; ofptr = ofptr->next ) {
	    iedge = ofptr->fixEdge ;
	    e1ptr = edgeListG[iedge] ;
	    x1 = e1ptr.loc ;

	    howmany = ofptr->eindex[HOWMANY] ;
	    lastE = ofptr->eindex[ howmany ] ;
	    e2ptr = edgeListG[ eArrayG[lastE].edge2 ] ;
	    x2 = e2ptr.loc;

	    if( howmany == 1 ) {
		/* simple case no empty room */
		node1 = crossref[e1ptr.node] ;
		node2 = crossref[e2ptr.node] ;
		length = x1 - xPolarG[node1].xc ; 
		length += xPolarG[node2].xc - x2  ; 
		addxEdge( node1, node2,
		    eArrayG[lastE].index1,eArrayG[lastE].index2,length ) ;
	    } else if( howmany >= 2 ){
		/* we have at least one empty room */
		/* process the first one */
		
		firstE = ofptr->eindex[1] ;
		e1ptr = edgeListG[ eArrayG[firstE].edge1 ] ;
		x1 = e1ptr.loc ;
		y1 = ( e1ptr.start + e1ptr.end ) / 2 ;
		nextE = ofptr->eindex[ 2 ] ;
		e2ptr = edgeListG[ eArrayG[nextE].edge2 ] ;
		x2 = e2ptr.loc;
		y2 = (e2ptr.start + e2ptr.end ) / 2 ;
		xsum = (x1 + x2) / 2 ;
		ysum = (y1 + y2) / 2 ;
		addxnode( ++numxPolarG, xsum, ysum ) ;
		node1 = crossref[e1ptr.node] ;
		length = x1 - xPolarG[node1].xc ; 
		addxEdge( node1, numxPolarG,
		    eArrayG[firstE].index1,eArrayG[firstE].index2,length ) ;

		/* add nodes for more than one empty rooms */
		/* case 3 < e <= howmany */
		for( e = 3; e <= howmany; e++ ){
		    firstE = ofptr->eindex[e-1] ;
		    e1ptr = edgeListG[ eArrayG[firstE].edge1 ] ;
		    x1 = e1ptr.loc ;
		    y1 = ( e1ptr.start + e1ptr.end ) / 2 ;
		    nextE = ofptr->eindex[ e ] ;
		    e2ptr = edgeListG[ eArrayG[nextE].edge2 ] ;
		    x2 = e2ptr.loc;
		    y2 = (e2ptr.start + e2ptr.end ) / 2 ;
		    xsum = (x1 + x2) / 2 ;
		    ysum = (y1 + y2) / 2 ;
		    addxnode( ++numxPolarG, xsum, ysum ) ;
		    addxEdge( numxPolarG - 1, numxPolarG,
			eArrayG[firstE].index1,eArrayG[firstE].index2,0 ) ;
		}

		/* now the last edge */
		node2 = crossref[e2ptr.node] ;
		length = xPolarG[node2].xc - x2  ; 
		addxEdge( numxPolarG, node2,
		    eArrayG[lastE].index1,eArrayG[lastE].index2,length ) ;

	    } else { /* end empty room case */
		printf( "unexpected case\n" ) ;
	    }

	} /* end loop on adjacent edges of node */
    } /* end traversing cells */

    swap_xsink() ;

    Ysafe_free( crossref ) ;

} /* end buildxPolorGraph */

static addxnode( node, xpos, ypos )
int node, xpos, ypos ;
{
    POLARPTR xptr ;

    if( node >= xallocS ){
	xallocS += EXPECTEDNUMNODES ;
	xPolarG = (POLARPTR) 
		    Ysafe_realloc( xPolarG, xallocS*sizeof(POLARBOX) ) ;
    }
    xptr = &(xPolarG[node]) ;
    xptr->xc = xpos ;
    xptr->yc = ypos ;
    xptr->valid = TRUE ;
    xptr->adj = NULL ;
    
} /* end addxnode */


/* this add a directed edge */
static addxEdge( node1, node2, chan1, chan2, length )
int node1, node2, chan1, chan2, length ;
{
    POLARPTR xptr ;
    POLAREDGEPTR temp, p_eptr ;

    xptr = &(xPolarG[node1]) ;
    if( temp = xptr->adj ){
	p_eptr = xptr->adj = 
	    (POLAREDGEPTR) Ysafe_malloc( sizeof(POLAREDGE) ) ;
	p_eptr->next = temp ;
    } else {
	p_eptr = xptr->adj = 
	    (POLAREDGEPTR) Ysafe_malloc( sizeof(POLAREDGE) ) ;
	p_eptr->next = NULL ;
    }
    p_eptr->node = node2 ;
    p_eptr->chan1 = chan1 ;
    p_eptr->chan2 = chan2 ;
    p_eptr->length = length ;
    p_eptr->valid = TRUE ;
    
} /* end addxEdge */


output_xgraph( crossref )
int *crossref ;
{
    char filename[LRECL] ;
    FILE *fp ;
    int i ;
    int edgecount ;
    int width ;
    int node2 ;
    int count ;
    int chanEdgeIndex ;
    int *sink_parent ;
    BOOL invalid ;
    POLARPTR xptr ;
    POLAREDGEPTR edgeptr ;
    POLAREDGEPTR tempptr ;
    POLAREDGEPTR edgeptr2 ;
    POLAREDGEPTR saveptr ;

    sprintf( filename, "%s.mhvg", cktNameG ) ;
    fp = TWOPEN( filename, "w", ABORT ) ;

    xrefS       = (int *) Ysafe_calloc( numxPolarG+1, sizeof(int) ) ;
    sink_parent = (int *) Ysafe_calloc( numxPolarG+1, sizeof(int) ) ;


    /* first merge all edges from the source */
    for( edgeptr=xPolarG[0].adj;edgeptr;edgeptr=edgeptr->next ){
	node2 = edgeptr->node ;
	xPolarG[node2].valid = FALSE ;
	for( edgeptr2 = xPolarG[node2].adj;edgeptr2; ){ 
	    /* move all of this nodes pointers to source */
	    tempptr = xPolarG[0].adj ;
	    xPolarG[0].adj = edgeptr2 ;
	    /* save pointer so we can skip onto next pointer */
	    saveptr = edgeptr2 ;
	    edgeptr2=edgeptr2->next ;
	    saveptr->next = tempptr ;
	}
	xPolarG[node2].adj = NULL ; /* so free works correctly */
    }
    /* fix up the sink node */
    for( i=0; i <= numxPolarG; i++ ){
	/* find all the parents of the sink node and mark them */
	xptr = &( xPolarG[i] ) ;
	for( edgeptr=xPolarG[i].adj;edgeptr; edgeptr=edgeptr->next ){
	    if( edgeptr->node == numxPolarG ){
		sink_parent[i] = TRUE ;
		xptr->valid = FALSE ;
	    }
	}
    }


    /* first count the edges and set the valid field on edges */
    for( edgecount=0,i=0; i <= numxPolarG; i++ ){

	for( edgeptr=xPolarG[i].adj;edgeptr; edgeptr=edgeptr->next ){
	    if( rectArrayG[edgeptr->chan1].active && 
		rectArrayG[edgeptr->chan2].active ){
		edgecount++ ;
	    } else {
		/* set valid field so we can count the nodes correctly */
		edgeptr->valid = FALSE ;
	    }
	}
    }

    /* now we can count the number of nodes */
    /* build another crossreference table as we go */
    /* note crossreference for source is done implicitly since */
    /* array is initialized to zero */
    count = -1 ;
    for( i=0; i <= numxPolarG; i++ ){
	xptr = &( xPolarG[i] ) ;
	invalid = TRUE ;  /* assume edge to be invalid prove otherwise */
	for( edgeptr=xPolarG[i].adj;edgeptr; edgeptr=edgeptr->next ){

	    if( edgeptr->valid ){
		/* for node to be valid all we need is one valid edge */
		xrefS[i] = ++count ;
		invalid = FALSE ;
		break ;
	    }
	}
	if( invalid ){
	    xptr->valid = FALSE ;
	}
    }
    /* sink has no edges need to set it ourselves */
    xrefS[numxPolarG] = ++count ;

    /* now reset the edges for any node we points to a sink_parent */ 
    for( i=0; i <= numxPolarG; i++ ){
	for( edgeptr=xPolarG[i].adj;edgeptr; edgeptr=edgeptr->next ){
	    if( sink_parent[edgeptr->node] ){
		edgeptr->node = numxPolarG ;
	    }
	}
    }


    fprintf( fp, "h_numnodes:%d h_numedges:%d\n",count+1,edgecount);

    /* output the xpolar graph */
    for( i=0; i <= numxPolarG; i++ ){
	xptr = &( xPolarG[i] ) ;

	if(!(xptr->valid )){
	    continue ;
	}

	/* output edge information */
	for( edgeptr=xptr->adj;edgeptr;edgeptr=edgeptr->next ){
	    if(!(edgeptr->valid)){
		continue ;
	    }
	    node2 = edgeptr->node ;
	    /* first convert channel nodes to an edge */
	    chanEdgeIndex = eIndexArrayG[edgeptr->chan1][edgeptr->chan2];
	    ASSERTNCONT( chanEdgeIndex,"output_xgraph",
		"chanEdgeIndex is zero" ) ;

	    /* now use eArray to find border edges in edgeList */
	    if( chanEdgeIndex <= edgeTransitionG ){
		/* vertical edge */
		width = eArrayG[chanEdgeIndex].width ;
	    } else {
		/* horizontal edge */
		width = eArrayG[chanEdgeIndex].width ;
	    }
	    fprintf( fp, "%d %d %d %d %d %d\n", 
		xrefS[i], xrefS[node2], 
		crossref[edgeptr->chan1], crossref[edgeptr->chan2],
		width, edgeptr->length ) ;
	}
    }
    TWCLOSE( fp ) ;

} /* end output_xgraph */


/* ****************************************************************** */

void buildyPolarGraph()
{

    int  x1, y1 ;
    int  x2, y2 ;
    int  cell ;
    int  iedge, oedge ;
    int  node1, node2 ;
    int  lastE ;
    int  num, xsum, ysum ;
    int  e, firstE, nextE ;
    EDGEBOX e1ptr, e2ptr;
    FLAREPTR ifptr , ofptr ;
    int *crossref ;     /* crossreference merged nodes */
    int inode ;         /* node of yNodeArray graph */
    int onode ;         /* node of yNodeArray graph */
    int howmany ;       /* number of channels found cross this edge */
    int length ;        /* length of cell contribution to edge */

    /* allocate space for cross reference array */
    crossref = (int *) Ysafe_calloc( numYnodesG+5,sizeof(int) ) ;

    /* created expected size of array */
    yallocS = numYnodesG + EXPECTEDNUMNODES ;
    yPolarG = (POLARPTR) Ysafe_malloc( yallocS * sizeof(POLARBOX) ) ;

    /* add source and sink nodes first */
    ofptr = yNodulesG[0].outList ;
    iedge = ofptr->fixEdge ;
    e1ptr = edgeListG[iedge] ;
    x1 = ( e1ptr.start + e1ptr.end ) / 2 ;
    y1 = e1ptr.loc ;
    addynode( 0, x1, y1 ) ;
    /* build crossreference */
    inode = e1ptr.node ;
    crossref[inode] = 0 ;

    /* sink node information not stored in graph */
    x2 = ( bblG + bbrG ) / 2 ;
    y2 = bbtG ;
    addynode( numberCellsG+1, x2, y2 ) ;
    onode = numYnodesG ;
    crossref[onode] = numberCellsG+1 ;

    /* now find nodes which are cells and empty rooms */
    /* this loop find nodes which are cells and build node */
    /* cross reference array */
    for( cell = 1 ; cell <= numberCellsG ; cell++ ) {
	ifptr = yNodulesG[cell].inList ;
	num = 0 ;
	xsum = 0 ;
	ysum = 0 ;
	for( ; ifptr != (FLAREPTR) NULL ; ifptr = ifptr->next ) {
	    iedge = ifptr->fixEdge ;
	    e1ptr = edgeListG[iedge] ;
	    x1 = ( e1ptr.start + e1ptr.end ) / 2 ;
	    y1 = e1ptr.loc ;

	    /* perform a cross reference for merging edges */
	    /* store new node at every oldnode */
	    inode = e1ptr.node ;
	    crossref[inode] = cell ;

	    ofptr = yNodulesG[cell].outList ;
	    for( ; ofptr != (FLAREPTR) NULL ; ofptr = ofptr->next ) {
		oedge = ofptr->fixEdge ;
		e2ptr = edgeListG[oedge] ;
		x2 = (e2ptr.start + e2ptr.end ) / 2 ;
		y2 = e2ptr.loc;
		onode = e2ptr.node ;
		crossref[onode] = cell ;
		xsum += x1 ;
		xsum += x2 ;
		ysum += y1 ;
		ysum += y2 ;
		num += 2 ;
	    }
	}
	/* place to put node is average */
	if( num <= 0 ){
	    M( ERRMSG, "buildyPolar","No nodes found in outlist\n");
	    num = 1 ;
	} 
	xsum /= num ;
	ysum /= num ;
	addynode( cell, xsum, ysum ) ;
    }

    /* now we need to add edges to the graph */
    numyPolarG = numberCellsG + 1  ; /* we added this many nodes so far */
    for( cell = 0 ; cell <= numberCellsG + 1 ; cell++ ) {
	ofptr = yNodulesG[cell].outList ;
	for( ; ofptr != (FLAREPTR) NULL ; ofptr = ofptr->next ) {
	    iedge = ofptr->fixEdge ;
	    e1ptr = edgeListG[iedge] ;
	    y1 = e1ptr.loc ;

	    howmany = ofptr->eindex[HOWMANY] ;
	    lastE = ofptr->eindex[ howmany ] ;
	    e2ptr = edgeListG[ eArrayG[lastE].edge2 ] ;
	    y2 = e2ptr.loc;

	    if( howmany == 1 ) {
		/* simple case no empty room */
		node1 = crossref[e1ptr.node] ;
		node2 = crossref[e2ptr.node] ;
		length = y1 - yPolarG[node1].yc ; 
		length += yPolarG[node2].yc - y2  ; 
		addyEdge( node1, node2,
		    eArrayG[lastE].index1,eArrayG[lastE].index2,length ) ;
	    } else if( howmany >= 2 ){
		/* we have at least one empty room */
		/* process the first one */
		
		firstE = ofptr->eindex[1] ;
		e1ptr = edgeListG[ eArrayG[firstE].edge1 ] ;
		x1 = ( e1ptr.start + e1ptr.end ) / 2 ;
		y1 = e1ptr.loc ;
		nextE = ofptr->eindex[ 2 ] ;
		e2ptr = edgeListG[ eArrayG[nextE].edge2 ] ;
		x2 = (e2ptr.start + e2ptr.end ) / 2 ;
		y2 = e2ptr.loc;
		xsum = (x1 + x2) / 2 ;
		ysum = (y1 + y2) / 2 ;
		addynode( ++numyPolarG, xsum, ysum ) ;
		node1 = crossref[e1ptr.node] ;
		length = y1 - yPolarG[node1].yc ; 
		addyEdge( node1, numyPolarG,
		    eArrayG[firstE].index1,eArrayG[firstE].index2,length ) ;

		/* add nodes for more than one empty rooms */
		/* case 3 < e <= howmany */
		for( e = 3; e <= howmany; e++ ){
		    firstE = ofptr->eindex[e-1] ;
		    e1ptr = edgeListG[ eArrayG[firstE].edge1 ] ;
		    x1 = ( e1ptr.start + e1ptr.end ) / 2 ;
		    y1 = e1ptr.loc ;
		    nextE = ofptr->eindex[ e ] ;
		    e2ptr = edgeListG[ eArrayG[nextE].edge2 ] ;
		    x2 = (e2ptr.start + e2ptr.end ) / 2 ;
		    y2 = e2ptr.loc;
		    xsum = (x1 + x2) / 2 ;
		    ysum = (y1 + y2) / 2 ;
		    addynode( ++numyPolarG, xsum, ysum ) ;
		    addyEdge( numyPolarG - 1, numyPolarG,
			eArrayG[firstE].index1,eArrayG[firstE].index2,0 ) ;
		}

		/* now the last edge */
		node2 = crossref[e2ptr.node] ;
		length = yPolarG[node2].yc - y2  ; 
		addyEdge( numyPolarG, node2,
		    eArrayG[lastE].index1,eArrayG[lastE].index2,length ) ;

	    } else { /* end empty room case */
		printf( "unexpected case\n" ) ;
	    }

	} /* end loop on adjacent edges of node */
    } /* end traversing cells */

    swap_ysink() ;

    Ysafe_free( crossref ) ;

} /* end buildyPolorGraph */

static addynode( node, xpos, ypos )
int node, xpos, ypos ;
{
    POLARPTR yptr ;

    if( node >= yallocS ){
	yallocS += EXPECTEDNUMNODES ;
	yPolarG = (POLARPTR) 
		    Ysafe_realloc( yPolarG, yallocS*sizeof(POLARBOX) ) ;
    }
    yptr = &(yPolarG[node]) ;
    yptr->xc = xpos ;
    yptr->yc = ypos ;
    yptr->valid = TRUE ;
    yptr->adj = NULL ;
    
} /* end addynode */


/* this add a directed edge */
static addyEdge( node1, node2, chan1, chan2, length )
int node1, node2, chan1, chan2, length ;
{
    POLARPTR yptr ;
    POLAREDGEPTR temp, p_eptr ;

    yptr = &(yPolarG[node1]) ;
    if( temp = yptr->adj ){
	p_eptr = yptr->adj = 
	    (POLAREDGEPTR) Ysafe_malloc( sizeof(POLAREDGE) ) ;
	p_eptr->next = temp ;
    } else {
	p_eptr = yptr->adj = 
	    (POLAREDGEPTR) Ysafe_malloc( sizeof(POLAREDGE) ) ;
	p_eptr->next = NULL ;
    }
    p_eptr->node = node2 ;
    p_eptr->chan1 = chan1 ;
    p_eptr->chan2 = chan2 ;
    p_eptr->length = length ;
    p_eptr->valid = TRUE ;
    
} /* end addyEdge */


output_ygraph( crossref )
int *crossref ;
{
    char filename[LRECL] ;
    FILE *fp ;
    int i ;
    int edgecount ;
    int width ;
    int node2 ;
    int count ;
    int chanEdgeIndex ;
    int *sink_parent ;
    BOOL invalid ;
    POLARPTR yptr ;
    POLAREDGEPTR edgeptr ;
    POLAREDGEPTR saveptr ;
    POLAREDGEPTR edgeptr2 ;
    POLAREDGEPTR tempptr ;

    sprintf( filename, "%s.mhvg", cktNameG ) ;
    fp = TWOPEN( filename, "a", ABORT ) ;

    yrefS       = (int *) Ysafe_calloc( numyPolarG+1, sizeof(int) ) ;
    sink_parent = (int *) Ysafe_calloc( numyPolarG+1, sizeof(int) ) ;


    /* first merge all edges from the source */
    for( edgeptr=yPolarG[0].adj;edgeptr;edgeptr=edgeptr->next ){
	node2 = edgeptr->node ;
	yPolarG[node2].valid = FALSE ;
	for( edgeptr2 = yPolarG[node2].adj;edgeptr2; ){ 
	    tempptr = yPolarG[0].adj ;
	    yPolarG[0].adj = edgeptr2 ;
	    /* save pointer so we can skip onto next pointer */
	    saveptr = edgeptr2 ;
	    edgeptr2=edgeptr2->next ;
	    saveptr->next = tempptr ;
	}
	yPolarG[node2].adj = NULL ; /* so free works correctly */
    }
    /* fix up the sink node */
    for( i=0; i <= numyPolarG; i++ ){
	/* find all the parents of the sink node and mark them */
	yptr = &( yPolarG[i] ) ;
	for( edgeptr=yPolarG[i].adj;edgeptr; edgeptr=edgeptr->next ){
	    if( edgeptr->node == numyPolarG ){
		sink_parent[i] = TRUE ;
		yptr->valid = FALSE ;
	    }
	}
    }


    /* first count the edges and set the valid field on edges */
    for( edgecount=0,i=0; i <= numyPolarG; i++ ){

	for( edgeptr=yPolarG[i].adj;edgeptr; edgeptr=edgeptr->next ){
	    if( rectArrayG[edgeptr->chan1].active && 
		rectArrayG[edgeptr->chan2].active ){
		edgecount++ ;
	    } else {
		/* set valid field so we can count the nodes correctly */
		edgeptr->valid = FALSE ;
	    }
	}
    }

    /* now we can count the number of nodes */
    /* build another crossreference table as we go */
    /* note crossreference for source is done implicitly since */
    /* array is initialized to zero */
    count = -1 ;
    for( i=0; i <= numyPolarG; i++ ){
	yptr = &( yPolarG[i] ) ;
	invalid = TRUE ;  /* assume edge to be invalid prove otherwise */
	for( edgeptr=yPolarG[i].adj;edgeptr; edgeptr=edgeptr->next ){

	    if( edgeptr->valid ){
		/* for node to be valid all we need is one valid edge */
		yrefS[i] = ++count ;
		invalid = FALSE ;
		break ;
	    }
	}
	if( invalid ){
	    yptr->valid = FALSE ;
	}
    }
    /* sink has no edges need to set it ourselves */
    yrefS[numyPolarG] = ++count ;

    /* now reset the edges for any node we points to a sink_parent */ 
    for( i=0; i <= numyPolarG; i++ ){
	for( edgeptr=yPolarG[i].adj;edgeptr; edgeptr=edgeptr->next ){
	    if( sink_parent[edgeptr->node] ){
		edgeptr->node = numyPolarG ;
	    }
	}
    }

    fprintf( fp, "v_numnodes:%d v_numedges:%d\n",count+1,edgecount);

    /* output the ypolar graph */
    for( i=0; i <= numyPolarG; i++ ){
	yptr = &( yPolarG[i] ) ;

	if(!(yptr->valid )){
	    continue ;
	}

	/* output edge information */
	for( edgeptr=yptr->adj;edgeptr;edgeptr=edgeptr->next ){
	    if(!(edgeptr->valid)){
		continue ;
	    }
	    node2 = edgeptr->node ;
	    /* first convert channel nodes to an edge */
	    chanEdgeIndex = eIndexArrayG[edgeptr->chan1][edgeptr->chan2];
	    ASSERTNCONT( chanEdgeIndex,"output_ygraph",
		"chanEdgeIndex is zero" ) ;

	    /* now use eArray to find border edges in edgeList */
	    if( chanEdgeIndex <= edgeTransitionG ){
		/* vertical edge */
		width = eArrayG[chanEdgeIndex].width ;
	    } else {
		/* horizontal edge */
		width = eArrayG[chanEdgeIndex].width ;
	    }
	    fprintf( fp, "%d %d %d %d %d %d\n", 
		yrefS[i], yrefS[node2], 
		crossref[edgeptr->chan1], crossref[edgeptr->chan2],
		width, edgeptr->length ) ;
	}
    }
    TWCLOSE( fp ) ;

} /* end output_ygraph */

int *get_xref_polar()
{
    return( xrefS ) ;
}  /* end get_xref_polar */

int *get_yref_polar()
{
    return( yrefS ) ;
}  /* end get_yref_polar */

/* swap the sink node with last node */
swap_xsink()
{
    int sink_orig, lastNode ;
    int i ;
    POLAREDGEPTR edgeptr ;

    sink_orig = numberCellsG+1 ;
    lastNode = numxPolarG ;
    /* swap all the references on the edges */
    for( i=0; i <= numxPolarG; i++ ){

	for( edgeptr=xPolarG[i].adj;edgeptr; edgeptr=edgeptr->next ){
	    if( edgeptr->node == sink_orig ){
		edgeptr->node = lastNode ;
	    } else if( edgeptr->node == lastNode ){
		edgeptr->node = sink_orig ;
	    }
	}
    }
    /* now swap records */
    swap_rec( &(xPolarG[lastNode]),&(xPolarG[sink_orig]) ) ;

} /* end swap_xsink() */

/* swap the sink node with last node */
swap_ysink()
{
    int sink_orig, lastNode ;
    int i ;
    POLAREDGEPTR edgeptr ;

    sink_orig = numberCellsG+1 ;
    lastNode = numyPolarG ;
    /* swap all the references on the edges */
    for( i=0; i <= numyPolarG; i++ ){

	for( edgeptr=yPolarG[i].adj;edgeptr; edgeptr=edgeptr->next ){
	    if( edgeptr->node == sink_orig ){
		edgeptr->node = lastNode ;
	    } else if( edgeptr->node == lastNode ){
		edgeptr->node = sink_orig ;
	    }
	}
    }
    /* now swap records */
    swap_rec( &(yPolarG[lastNode]),&(yPolarG[sink_orig]) ) ;

} /* end swap_ysink() */

static swap_rec( lastptr, sinkptr )
POLARPTR lastptr, sinkptr ; 
{
    int temp ;
    POLAREDGEPTR tempptr ;

    temp = lastptr->xc ;
    lastptr->xc = sinkptr->xc ;
    sinkptr->xc = temp ;

    temp = lastptr->yc ;
    lastptr->yc = sinkptr->yc ;
    sinkptr->yc = temp ;

    tempptr = lastptr->adj ;
    lastptr->adj = sinkptr->adj ;
    sinkptr->adj = tempptr ;
} /* end swap_rec() */
