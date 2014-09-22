/* ----------------------------------------------------------------- 
FILE:	    xgraph.c                                       
DESCRIPTION:Build the xgraph necessary for the polar graph.
CONTENTS:   xgraph()
	    fix_empty_rooms()
	    static fix_channels( ptr, crossref )
		WCPTR ptr ;    
		int *crossref ;
DATE:	    Mar  3, 1990 
REVISIONS:  Mar  3, 1990 - Now a standalone program.
		Added fix_empty_rooms()
	    Apr 20, 1990 - fixed redundant edges when cells touch.
	    May  4, 1990 - modified fix_empty_rooms so that it adds
		all the channel capacity to the first channel, -1 for
		the middle channels and 0 for the last channel.
	    May 15, 1990 - the last channel is now set to -1 also.
	    Wed May  1 19:02:32 EDT 1991 - added missing statement
		to fix_channels.
	    Wed Jun 12 22:10:09 CDT 1991 - fixed problem with add2eindex.
		Now add all the channels to xgraph and ygraph properly.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) xgraph.c version 1.8 6/12/91" ;
#endif

#include <geo.h>
#include <yalecad/debug.h>
#define HOWMANY 0



static fix_channels();



xgraph()
{

int i , vedge , location , eindex , et , eb , t , b , cell , count ;
int beginEdge , iedge , inode , oedge , onode , lastE ;
WCPTR saveptr, xptr ;
DLINK1PTR vptr ;
PLISTPTR plptr ;
FLAREPTR fptr , ifptr , ofptr ;
TNODEPTR tnode ;

xNodulesG = (NODPTR) Ysafe_malloc( (2 + numberCellsG) * sizeof(NODBOX));
for( i = 0 ; i <= numberCellsG + 1 ; i++ ) {
    xNodulesG[i].inList = (FLAREPTR) NULL ;
    xNodulesG[i].outList = (FLAREPTR) NULL ;
    xNodulesG[i].done = 0 ;
}
vptr = VlistG ;
vedge = vptr->edge ;
location = edgeListG[vedge].loc ;
et = edgeListG[vedge].end ;
eb = edgeListG[vedge].start ;
plptr = tplist( vChanBeginRootG , location ) ;
for( ; plptr != (PLISTPTR) NULL ; plptr = plptr->pnext ) {
    eindex = plptr->property ;
    t = eArrayG[eindex].hiend ;
    b = eArrayG[eindex].loend ;
    if( ! (b >= eb && t <= et) ) {
	continue ;
    }
    fptr = xNodulesG[0].outList ;
    xNodulesG[0].outList = (FLAREPTR) Ysafe_malloc( sizeof(FLARE) ) ;
    xNodulesG[0].outList->next = fptr ;
    xNodulesG[0].outList->fixEdge = vedge ;
    xNodulesG[0].outList->eindex = (INT *) Ysafe_malloc( 2 * sizeof(INT) ) ; 
    xNodulesG[0].outList->eindex[HOWMANY] = 1 ; 
    xNodulesG[0].outList->eindex[1] = eindex ; 
    xNodulesG[0].done = 1 ; 
}
vptr = VendG ;
vedge = vptr->edge ;
location = edgeListG[vedge].loc ;
et = edgeListG[vedge].end ;
eb = edgeListG[vedge].start ;
plptr = tplist( vChanEndRootG , location ) ;
for( ; plptr != (PLISTPTR) NULL ; plptr = plptr->pnext ) {
    eindex = plptr->property ;
    t = eArrayG[eindex].hiend ;
    b = eArrayG[eindex].loend ;
    if( ! (b >= eb && t <= et) ) {
	continue ;
    }
    fptr = xNodulesG[numberCellsG + 1].inList ;
    xNodulesG[numberCellsG + 1].inList = (FLAREPTR) Ysafe_malloc(sizeof(FLARE));
    xNodulesG[numberCellsG + 1].inList->next = fptr ;
    xNodulesG[numberCellsG + 1].inList->fixEdge = vedge ;
    xNodulesG[numberCellsG + 1].inList->eindex = (INT *) 
						Ysafe_malloc(2 * sizeof(INT)); 
    xNodulesG[numberCellsG + 1].inList->eindex[HOWMANY] = 1 ; 
    xNodulesG[numberCellsG + 1].inList->eindex[1] = eindex ; 
    xNodulesG[numberCellsG + 1].done = 1 ; 
}

vptr = vFixedListG->next ;
for( ; vptr->next != (DLINK1PTR) NULL ; vptr = vptr->next ) {
    vedge = vptr->edge ;
    cell = edgeListG[vedge].cell ;
    if( xNodulesG[cell].done == 1 ) {
	continue ;
    }
    xNodulesG[cell].done = 1 ;
    beginEdge = vedge ;
    do {
	if( edgeListG[vedge].UorR > 0 ) {
	    location = edgeListG[vedge].loc ;
	    plptr = tplist( vChanEndRootG , location ) ;
	    for( ; plptr != (PLISTPTR) NULL ; plptr = plptr->pnext ) {
		eindex = plptr->property ;
		if( eArrayG[eindex].edge2 != vedge ) {
		    continue ;
		}
		fptr = xNodulesG[cell].inList ;
		xNodulesG[cell].inList = (FLAREPTR) Ysafe_malloc(sizeof(FLARE));
		xNodulesG[cell].inList->next = fptr ;
		xNodulesG[cell].inList->fixEdge = vedge ;
		xNodulesG[cell].inList->eindex = (INT *) 
						Ysafe_malloc(2 * sizeof(INT)); 
		xNodulesG[cell].inList->eindex[HOWMANY] = 1 ; 
		xNodulesG[cell].inList->eindex[1] = eindex ; 
	    }
	} else {
	    location = edgeListG[vedge].loc ;
	    plptr = tplist( vChanBeginRootG , location ) ;
	    for( ; plptr != (PLISTPTR) NULL ; plptr = plptr->pnext ) {
		eindex = plptr->property ;
		if( eArrayG[eindex].edge1 != vedge ) {
		    continue ;
		}
		fptr = xNodulesG[cell].outList ;
		xNodulesG[cell].outList = (FLAREPTR) Ysafe_malloc(sizeof(FLARE));
		xNodulesG[cell].outList->next = fptr ;
		xNodulesG[cell].outList->fixEdge = vedge ;
		xNodulesG[cell].outList->eindex = (INT *) 
						Ysafe_malloc(2 * sizeof(INT)); 
		xNodulesG[cell].outList->eindex[HOWMANY] = 1 ; 
		xNodulesG[cell].outList->eindex[1] = eindex ; 
	    }
	}
	vedge = edgeListG[vedge].nextEdge ;
	vedge = edgeListG[vedge].nextEdge ;
    } while( vedge != beginEdge ) ;
}

for( cell = 0 ; cell <= numberCellsG + 1 ; cell++ ) {
    fptr = xNodulesG[cell].inList ;
    for( ; fptr != (FLAREPTR) NULL ; fptr = fptr->next ) {
	for( count = fptr->eindex[HOWMANY]; count <= fptr->eindex[HOWMANY]; count++){
	    eindex = fptr->eindex[count] ;
	    location = eArrayG[eindex].lbside ;
	    et = eArrayG[eindex].hiend ;
	    eb = eArrayG[eindex].loend ;
	    plptr = tplist( vChanEndRootG , location ) ;
	    for( ; plptr != (PLISTPTR) NULL ; plptr = plptr->pnext ) {
		eindex = plptr->property ;
		t = eArrayG[eindex].hiend ;
		b = eArrayG[eindex].loend ;
		if( ! (b >= eb && t <= et) ) {
		    continue ;
		}
		tdelete( &vChanEndRootG , location , eindex ) ;
		if( add2eindex( fptr, eindex )){
		    count-- ;
		}
		break ;
	    }
	}
    }
    fptr = xNodulesG[cell].outList ;
    for( ; fptr != (FLAREPTR) NULL ; fptr = fptr->next ) {
	for( count = fptr->eindex[HOWMANY]; count <= fptr->eindex[HOWMANY]; count++){
	    eindex = fptr->eindex[count] ;
	    location = eArrayG[eindex].rtside ;
	    et = eArrayG[eindex].hiend ;
	    eb = eArrayG[eindex].loend ;
	    plptr = tplist( vChanBeginRootG , location ) ;
	    for( ; plptr != (PLISTPTR) NULL ; plptr = plptr->pnext ) {
		eindex = plptr->property ;
		t = eArrayG[eindex].hiend ;
		b = eArrayG[eindex].loend ;
		if( ! (b >= eb && t <= et) ) {
		    continue ;
		}
		tdelete( &vChanBeginRootG , location , eindex ) ;
		if( add2eindex( fptr, eindex )){
		    count-- ;
		}
		break ;
	    }
	}
    }
}

numXnodesG = 0 ;
for( vptr = vFixedListG; vptr != (DLINK1PTR) NULL; vptr = vptr->next ){
    edgeListG[ vptr->edge ].node = ++numXnodesG ;
}
xNodeArrayG = (WCPTR *) Ysafe_malloc( (1 + numXnodesG) * sizeof(WCPTR) );
for( i = 1 ; i <= numXnodesG ; i++ ) {
    xNodeArrayG[i] = (WCPTR) NULL ;
}
for( cell = 1 ; cell <= numberCellsG ; cell++ ) {
    ifptr = xNodulesG[cell].inList ;
    for( ; ifptr != (FLAREPTR) NULL ; ifptr = ifptr->next ) {
	iedge = ifptr->fixEdge ;
	inode = edgeListG[iedge].node ;
	ofptr = xNodulesG[cell].outList ;
	for( ; ofptr != (FLAREPTR) NULL ; ofptr = ofptr->next ) {
	    oedge = ofptr->fixEdge ;
	    onode = edgeListG[oedge].node ;
	    saveptr = xNodeArrayG[inode] ;
	    xptr = xNodeArrayG[inode] = (WCPTR) Ysafe_malloc( sizeof(WCBOX));
	    xptr->next = saveptr ;
	    xptr->fixedWidth = 1 ;
	    xptr->node = onode ;
	    xptr->channels = (INT *) NULL ;
	    xptr->length = edgeListG[oedge].loc - edgeListG[iedge].loc ;
	}
    }
}
for( cell = 0 ; cell <= numberCellsG + 1 ; cell++ ) {
    ofptr = xNodulesG[cell].outList ;
    for( ; ofptr != (FLAREPTR) NULL ; ofptr = ofptr->next ) {
	iedge = ofptr->fixEdge ;
	inode = edgeListG[iedge].node ;

	lastE = ofptr->eindex[ ofptr->eindex[HOWMANY] ] ;
	onode = edgeListG[ eArrayG[lastE].edge2 ].node ;

	saveptr = xNodeArrayG[inode] ;
	xptr = xNodeArrayG[inode] = (WCPTR) Ysafe_malloc( sizeof(WCBOX));
	xptr->next = saveptr ;
	xptr->fixedWidth = 0 ;
	xptr->length = 0 ;
	xptr->node = onode ;
	xptr->channels = (INT *) Ysafe_malloc( (1 + 
				ofptr->eindex[HOWMANY] ) * sizeof(INT) );
	for( count = 0 ; count <= ofptr->eindex[HOWMANY] ; count++ ) {
	    xptr->channels[count] = ofptr->eindex[count] ;
	}
    }
}

if( vChanBeginRootG != (TNODEPTR) NULL ) {
    for( ; ; ) {
	tpop( &vChanBeginRootG , &tnode , &location , &eindex ) ;
	if( tnode == (TNODEPTR) NULL ) {
	    break ;
	}
    }
}
if( vChanEndRootG != (TNODEPTR) NULL ) {
    for( ; ; ) {
	tpop( &vChanEndRootG , &tnode , &location , &eindex ) ;
	if( tnode == (TNODEPTR) NULL ) {
	    break ;
	}
    }
}
return ;
}

fix_empty_rooms()
{
    int i ;                   /* counter */
    int *crossref ;           /* node cross reference table */
    int *get_cross_ref() ;    /* returns node crossreference */
    WCPTR ptr ;               /* search thru the x and y graphs */

    crossref = get_cross_ref() ;

    /* first fix the xgraph */
    for( i = 1 ; i <= numXnodesG ; i++ ) {
	for( ptr = xNodeArrayG[i] ; ptr ; ptr = ptr->next ){
	    if( ptr->fixedWidth == 1 ) {
		continue ;
	    }
	    fix_channels( ptr, crossref ) ;
	}
    }

    /* next fix the ygraph */
    for( i = 1 ; i <= numYnodesG ; i++ ) {
	for( ptr = yNodeArrayG[i] ; ptr ; ptr = ptr->next ){
	    if( ptr->fixedWidth == 1 ) {
		continue ;
	    }
	    fix_channels( ptr, crossref ) ;
	}
    }

} /* end fix empty rooms */

static fix_channels( ptr, crossref )
WCPTR ptr ;                   /* search thru the x and y graphs */
int *crossref ;               /* node crossreference */
{
    int c ;                   /* counter */
    int chan ;                /* current edge */
    int count ;               /* no.channels across edge in polar graph*/
    int width ;               /* total width of empty rooms */
    int chan_count ;          /* number of channels in empty room */
    int last_count ;          /* last channel count in ptr->channels */
    int last_pos ;            /* last pos. for updating channel pos. */
    int even_space ;          /* evenly divide the space */
    int to_add ;              /* how much width to add to each channel */
    int node1, node2 ;        /* the two nodes of channel */

    count = ptr->channels[0] ;
    width = 0 ;
    chan_count = 0 ;
    for( c = 1 ; c <= count ; c++ ) {
	chan = ptr->channels[c] ;
	node1 = eArrayG[chan].index1 ;
	node2 = eArrayG[chan].index2 ;
	if( !(rectArrayG[node1].active) || !(rectArrayG[node2].active) ){
	    continue ;
	}
	last_count = c ;
	chan_count++ ;
	/* now sum total width across the empty room */
	width += eArrayG[ chan ].width ;
	D( "gengraph/fix_channels",
	    fprintf( stderr, "channel:%d %d c:%d width:%d lb:%d rt:%d\n",
	    crossref[ node1 ], crossref[ node2 ],
	    c, eArrayG[ chan ].width, eArrayG[chan].lbside,
	    eArrayG[ chan ].rtside ) ) ;
    }

    /* now need to divide width amongst the empty room */
    if( chan_count >= 2 ){ /* we have an empty room */
	/* --------------------------------------------------------
	    We wish to add width to one of the outside channels
	    and zero to the other.  The capacity of the inside
	    channels should be -1.
	----------------------------------------------------------- */
	/* first channel gets all the width */
	for( c = 1 ; c < last_count ; c++ ) {
	    chan = ptr->channels[c] ;
	    node1 = eArrayG[chan].index1 ;
	    node2 = eArrayG[chan].index2 ;
	    if( !(rectArrayG[node1].active) ||
		!(rectArrayG[node2].active) ){
		continue ;
	    }
	    eArrayG[ chan ].width = width ;
	    break ;
	}
	/* middle channels and last channels get -1 for capacity */
	for( ++c ; c <= last_count ; c++ ) {
	    chan = ptr->channels[c] ;
	    node1 = eArrayG[chan].index1 ;
	    node2 = eArrayG[chan].index2 ;
	    if( !(rectArrayG[node1].active) ||
		!(rectArrayG[node2].active) ){
		continue ;
	    }
	    eArrayG[ chan ].width = -1 ;
	}

	D( "gengraph/fix_channels", 
	    for( c = 1 ; c <= count ; c++ ) {
		chan = ptr->channels[c] ;
		node1 = eArrayG[chan].index1 ;
		node2 = eArrayG[chan].index2 ;
		if( !(rectArrayG[node1].active) ||
		    !(rectArrayG[node2].active) ){
		    continue ;
		}
		fprintf( stderr,
		    "\tchannel:%d %d c:%d width:%d lb:%d rt:%d\n",
		    crossref[ node1 ], crossref[ node2 ],
		    c, eArrayG[ chan ].width, eArrayG[chan].lbside,
		    eArrayG[ chan ].rtside ) ;
	    }
	) ;
    }
} /* end fix_channels */
