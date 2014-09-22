/* ----------------------------------------------------------------- 
FILE:	    changraph.c                                       
DESCRIPTION:This file contains the routines for the building the channel
	    graph from the edgelists.the nodes
CONTENTS:   changraph( )
	    exploreUp()
	    exploreRite()
	    constructVedge( index1 , index2 )
		int index1 , index2 ;
	    constructHedge( index1 , index2 )
		int index1 , index2 ;
DATE:	    Jan 24, 1989 
REVISIONS:  Jan 24, 1989 - added file header, moved place of adj matrix
		so that rectlinear would work and added code to make
		nodes correct size.  Also added call to rectlinear.
	    Oct 12, 1990 - added initialization to density calc.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) changraph.c version 1.3 10/14/90" ;
#endif

#include <geo.h>
#include <yalecad/debug.h>

#define NONORTHOGONAL

changraph()
{

char filename[64] ;
FILE *fp ;
int i , index1 , index2 , hiend , loend , length ;

vChanBeginRootG = (TNODEPTR) NULL ;
vChanEndRootG   = (TNODEPTR) NULL ;
hChanBeginRootG = (TNODEPTR) NULL ;
hChanEndRootG   = (TNODEPTR) NULL ;
eNumG = 0 ;
eArrayG = (EBOXPTR) Ysafe_malloc( 100 * sizeof(EBOX) ) ;

exploreUp() ;

edgeTransitionG = eNumG ;

exploreRite() ;

/* eIndexArray is an adjacency matrix - rather space hungry but fast */
eIndexArrayG = (INT **) Ysafe_malloc( (1 + numRectsG) * sizeof( INT * ) ) ;
for( i = 1 ; i <= numRectsG ; i++ ) {
    eIndexArrayG[i] = (INT *) Ysafe_malloc( (1 + numRectsG) * sizeof( INT ) ) ;
}

/* build adjacency matrix now - need to use it in rectilinear */
/* also initialize density to zero */
for( i = 1 ; i <= eNumG ; i++ ) {
    index1 = eArrayG[i].index1 ;
    index2 = eArrayG[i].index2 ;
    eArrayG[i].density = 0 ;
    /* build channel adjacency matrix */
    eIndexArrayG[index1][index2] = (INT)i ;
    eIndexArrayG[index2][index1] = (INT)i ;
}

/* Make all graph rectilinear */
rectilinear()  ;

for( i = 1 ; i <= eNumG ; i++ ) {
    eArrayG[i].active = TRUE ;  /* initialize to active */
    index1 = eArrayG[i].index1 ;
    index2 = eArrayG[i].index2 ;
    if( i <= edgeTransitionG ) {
	hiend = rectArrayG[index2].b ;
	loend = rectArrayG[index1].t ;
	length = rectArrayG[index2].yc - rectArrayG[index1].yc  ;
	tinsert( &vChanBeginRootG , eArrayG[i].lbside , i ) ;
	tinsert( &vChanEndRootG , eArrayG[i].rtside , i ) ;
    } else {
	hiend = rectArrayG[index2].l ;
	loend = rectArrayG[index1].r ;
	length = rectArrayG[index2].xc - rectArrayG[index1].xc  ;
	tinsert( &hChanBeginRootG , eArrayG[i].lbside , i ) ;
	tinsert( &hChanEndRootG , eArrayG[i].rtside , i ) ;
    }
    eArrayG[i].length = length ;
    eArrayG[i].hiend = hiend ;
    eArrayG[i].loend = loend ;
}

D( "changraph", fprintf(stdout,"CHANNEL-GRAPH NODES:\n");
    for( i = 1 ; i <= numRectsG ; i++ ) {
	fprintf(stdout,
	    "rect Node: %d  at: (%d %d)  l:%d  r:%d b:%d t:%d\n",
		    i , rectArrayG[i].xc , rectArrayG[i].yc ,
		    rectArrayG[i].l, rectArrayG[i].r,
		    rectArrayG[i].b, rectArrayG[i].t ) ;
    }
) ;

return ;
}



exploreUp()
{

DLINK2PTR beptr ;
int beg , end , left , rite , finishLine , b , l , r , rec , index ;

for( rec = 1 ; rec <= numRectsG ; rec++ ) {
    beg  = rectArrayG[rec].b ;
    end  = rectArrayG[rec].t ;
    rite = rectArrayG[rec].r ;
    left = rectArrayG[rec].l ;
    finishLine = INT_MAX ;
    beptr = BEptrsG[ tprop( BErootG , beg ) ] ;
    for( ; beptr != (DLINK2PTR) NULL; beptr = beptr->next ){
	index = beptr->index ;
	b = rectArrayG[index].b ;
	if( b > finishLine ) {
	    break ;
	}
	l = rectArrayG[index].l ;
	r = rectArrayG[index].r ;
	if( b <= end || l > rite || r < left ) {
	    continue ;
	}
	if( constructVedge( rec , index ) ) {
	    break ;
	}
	finishLine = rectArrayG[index].t ;
    }
}
return ;
}



exploreRite()
{

DLINK2PTR leptr ;
int beg , end , top , bot , finishLine , l , b , t , rec , index ;

for( rec = 1 ; rec <= numRectsG ; rec++ ) {
    beg = rectArrayG[rec].l ;
    end = rectArrayG[rec].r ;
    top = rectArrayG[rec].t ;
    bot = rectArrayG[rec].b ;
    finishLine = INT_MAX ;
    leptr = LEptrsG[ tprop( LErootG , beg ) ] ;
    for( ; leptr != (DLINK2PTR) NULL; leptr = leptr->next ){
	index = leptr->index ;
	l = rectArrayG[index].l ;
	if( l > finishLine ) {
	    break ;
	}
	b = rectArrayG[index].b ;
	t = rectArrayG[index].t ;
	if( l <= end || b > top || t < bot ) {
	    continue ;
	}
	if( constructHedge( rec , index ) ) {
	    break ;
	}
	finishLine = rectArrayG[index].r ;
    }
}
return ;
}



constructVedge( index1 , index2 )
int index1 , index2 ;
{

DLINK1PTR v1ptr , v2ptr , vptr ;
int l1 , l2 , r1 , r2 , l , r , b , t , edge1 , edge2 , el , er ;
int eb , et , eb1 , et1 , eb2 , et2 ;
int edge , x ;

l1 = rectArrayG[index1].l ;
r1 = rectArrayG[index1].r ;
l2 = rectArrayG[index2].l ;
r2 = rectArrayG[index2].r ;
l = ( l1 >= l2 ) ? l1 : l2 ;
r = ( r1 <= r2 ) ? r1 : r2 ;
b = rectArrayG[index1].t ; 
t = rectArrayG[index2].b ; 

el = INT_MIN ;
v1ptr = VDptrsG[ tprop( VDrootG , l ) ] ;
/* VDptrs is the Vertical Down edges starting at l */
/* l is the largest of the left edges of the CR */
/* r is the smallest of the right edges of the CR */
/* find the rightmost downedge which spans the CR between node 1 & 2 */
for( ; v1ptr != (DLINK1PTR) NULL; v1ptr = v1ptr->next ) {
    edge1 = v1ptr->edge ;
    /* if we past beyond r, we won't find any edges of interest */
    if( edgeListG[edge1].loc > r ) {
	break ;
    }
    /* look only for down going edge - we want to find left side */
    if( edgeListG[edge1].UorR > 0 ) {
	continue ;
    }
    /* ignore any edge which doesn't span the critical region */
    if( edgeListG[edge1].start > b || edgeListG[edge1].end < t ) {
	continue ;
    }
    /* this is the edge we want */
    el = edgeListG[edge1].loc ;
    vptr = v1ptr ;
}
if( el == INT_MIN ) {
    return(0) ;
}
v2ptr = VptrsG[ tprop( VrootG , el ) ] ;
for( ; v2ptr != (DLINK1PTR) NULL; v2ptr = v2ptr->next ) {
    edge2 = v2ptr->edge ;
    if( edgeListG[edge2].UorR < 0 ) {
	continue ;
    }
    if( edgeListG[edge2].start > b || edgeListG[edge2].end < t ) {
	continue ;
    }
    er = edgeListG[edge2].loc ;
    break ;
}
if( er > r ) {
    return(0) ;
}
/* now set node_r, and node_l fields to correct value of the chan area */
rectArrayG[index1].node_l = MAX( el, rectArrayG[index1].node_l ) ;
rectArrayG[index2].node_l = MAX( el, rectArrayG[index2].node_l ) ; 
rectArrayG[index1].node_r = MIN( er, rectArrayG[index1].node_r ) ; 
rectArrayG[index2].node_r = MIN( er, rectArrayG[index2].node_r ) ; 

v1ptr = vptr ;
edge1 = v1ptr->edge ;

et1 = edgeListG[edge1].end ;
eb1 = edgeListG[edge1].start ;
et2 = edgeListG[edge2].end ;
eb2 = edgeListG[edge2].start ;
if( edgeListG[edge1].fixed == 1 && edgeListG[edge2].fixed == 1 ) {
    if( edgeListG[edge1].cell > 0 && edgeListG[edge2].cell > 0 ) {
	et = (et1 >= et2) ? et1 : et2 ;
	eb = (eb1 <= eb2) ? eb1 : eb2 ;
    } else if( edgeListG[edge1].cell > 0 ) {
	et = et1 ;
	eb = eb1 ;
    } else if( edgeListG[edge2].cell > 0 ) {
	et = et2 ;
	eb = eb2 ;
    } else {
	et = INT_MAX ;
	eb = INT_MIN ;
    }
} else if( edgeListG[edge1].fixed == 1 ) {
    if( edgeListG[edge1].cell > 0 ) {
	et = et1 ;
	eb = eb1 ;
    } else {
	et = INT_MAX ;
	eb = INT_MIN ;
    }
} else if( edgeListG[edge2].fixed == 1 ) {
    if( edgeListG[edge2].cell > 0 ) {
	et = et2 ;
	eb = eb2 ;
    } else {
	et = INT_MAX ;
	eb = INT_MIN ;
    }
} else {
    et = INT_MAX ;
    eb = INT_MIN ;
}
#ifdef NONORTHOGONAL
if( ! ( et > rectArrayG[index2].t ) ){
    if( rectArrayG[index2].yc < et ) {
	rectArrayG[index2].yc = et ;
	rectArrayG[index2].yreset = 2 ;
    }
}
if( ! ( eb < rectArrayG[index1].b ) ){
    if( rectArrayG[index1].yc > eb ) {
	rectArrayG[index1].yc = eb ;
	rectArrayG[index1].yreset = 1 ;
    }
}
if( rectArrayG[index1].xc < el || rectArrayG[index1].xc > er ) {
    rectArrayG[index1].xc = (el + er) / 2 ;
}
if( rectArrayG[index2].xc < el || rectArrayG[index2].xc > er ) {
    rectArrayG[index2].xc = (el + er) / 2 ;
}
#endif

if( edgeListG[edge1].fixed == 0 ) {
    vptr = VptrsG[ tprop( VrootG , el ) ] ;
    x = edgeListG[vptr->edge].loc ;
    vptr = vptr->next ;
    for( ; vptr != (DLINK1PTR) NULL; vptr = vptr->next ) {
	if( edgeListG[vptr->edge].loc > x ) {
	    break ;
	}
    }
    for( vptr = vptr->prev; vptr != (DLINK1PTR) NULL; 
					    vptr = vptr->prev ) {
	if( vptr == v2ptr ) {
	    continue ;
	}
	edge = vptr->edge ;
	if( edgeListG[edge].UorR < 0 ) {
	    continue ;
	}
	if( edgeListG[edge].start > b || edgeListG[edge].end < t ) {
	    continue ;
	}
	break ;
    }
    el -= (el - edgeListG[edge].loc) - (el - edgeListG[edge].loc) / 2 ;
}
if( edgeListG[edge2].fixed == 0 ) {
    vptr = VptrsG[ tprop( VrootG , er ) ] ;
    for(; vptr != (DLINK1PTR) NULL; vptr = vptr->next ) {
	if( vptr == v1ptr ) {
	    continue ;
	}
	edge = vptr->edge ;
	if( edgeListG[edge].UorR > 0 ) {
	    continue ;
	}
	if( edgeListG[edge].start > b || edgeListG[edge].end < t ) {
	    continue ;
	}
	break ;
    }
    er += (edgeListG[edge].loc - er) / 2 ;
}

if( ++eNumG % 100 == 0 ) {
    eArrayG = (EBOXPTR) Ysafe_realloc( eArrayG, (eNumG + 100) * sizeof(EBOX));
}
eArrayG[eNumG].active = TRUE ;  /* initialize to active */
eArrayG[eNumG].index1 = index1 ;
eArrayG[eNumG].index2 = index2 ;
eArrayG[eNumG].width  = er - el ;
eArrayG[eNumG].lbside = el ;
eArrayG[eNumG].rtside = er ;
eArrayG[eNumG].edge1 = edge1 ;
eArrayG[eNumG].edge2 = edge2 ;

return(1) ;
}




constructHedge( index1 , index2 )
int index1 , index2 ;
{

DLINK1PTR h1ptr , h2ptr , hptr ;
int b1 , b2 , t1 , t2 , b , l , r , t , edge1 , edge2 , eb , et ;
int el , er , el1 , er1 , el2 , er2 ;
int edge , x ;

b1 = rectArrayG[index1].b ;
t1 = rectArrayG[index1].t ;
b2 = rectArrayG[index2].b ;
t2 = rectArrayG[index2].t ;
b = ( b1 >= b2 ) ? b1 : b2 ;
t = ( t1 <= t2 ) ? t1 : t2 ;
l = rectArrayG[index1].r ; 
r = rectArrayG[index2].l ; 

eb = INT_MIN ;

/* find the topmost rightedge which spans the CR between node 1 & 2 */
h1ptr = HRptrsG[ tprop( HRrootG , b ) ] ;
for( ; h1ptr != (DLINK1PTR) NULL; h1ptr = h1ptr->next ) {
    edge1 = h1ptr->edge ;
    /* no more edges of interest */
    if( edgeListG[edge1].loc > t ) {
	break ;
    }
    /* look for only right going edges */
    if( edgeListG[edge1].UorR < 0 ) {
	continue ;
    }
    /* ignore edges that do not span the CR */
    if( edgeListG[edge1].start > l || edgeListG[edge1].end < r ) {
	continue ;
    }
    /* the edge type we want but check for topmost */
    eb = edgeListG[edge1].loc ;
    hptr = h1ptr ;
}
if( eb == INT_MIN ) {
    return(0) ;
}
h2ptr = HptrsG[ tprop( HrootG , eb ) ] ;
for( ; h2ptr != (DLINK1PTR) NULL; h2ptr = h2ptr->next ) {
    edge2 = h2ptr->edge ;
    if( edgeListG[edge2].UorR > 0 ) {
	continue ;
    }
    if( edgeListG[edge2].start > l || edgeListG[edge2].end < r ) {
	continue ;
    }
    et = edgeListG[edge2].loc ;
    break ;
}
if( et > t ) {
    return(0) ;
}
/* now set node_b, and node_t fields to correct value of the chan area */
rectArrayG[index1].node_b = MAX( eb, rectArrayG[index1].node_b ) ;
rectArrayG[index2].node_b = MAX( eb, rectArrayG[index2].node_b ) ; 
rectArrayG[index1].node_t = MIN( et, rectArrayG[index1].node_t ) ; 
rectArrayG[index2].node_t = MIN( et, rectArrayG[index2].node_t ) ; 

h1ptr = hptr ;
edge1 = h1ptr->edge ;

er1 = edgeListG[edge1].end ;
el1 = edgeListG[edge1].start ;
er2 = edgeListG[edge2].end ;
el2 = edgeListG[edge2].start ;
if( edgeListG[edge1].fixed == 1 && edgeListG[edge2].fixed == 1 ) {
    if( edgeListG[edge1].cell > 0 && edgeListG[edge2].cell > 0 ) {
	er = (er1 >= er2) ? er1 : er2 ;
	el = (el1 <= el2) ? el1 : el2 ;
    } else if( edgeListG[edge1].cell > 0 ) {
	er = er1 ;
	el = el1 ;
    } else if( edgeListG[edge2].cell > 0 ) {
	er = er2 ;
	el = el2 ;
    } else {
	er = INT_MAX ;
	el = INT_MIN ;
    }
} else if( edgeListG[edge1].fixed == 1 ) {
    if( edgeListG[edge1].cell > 0 ) {
	er = er1 ;
	el = el1 ;
    } else {
	er = INT_MAX ;
	el = INT_MIN ;
    }
} else if( edgeListG[edge2].fixed == 1 ) {
    if( edgeListG[edge2].cell > 0 ) {
	er = er2 ;
	el = el2 ;
    } else {
	er = INT_MAX ;
	el = INT_MIN ;
    }
} else {
    er = INT_MAX ;
    el = INT_MIN ;
}
#ifdef NONORTHOGONAL
if( ! ( er > rectArrayG[index2].r ) ){
    if( rectArrayG[index2].xc < er ) {
	rectArrayG[index2].xc = er ;
	rectArrayG[index2].xreset = 2 ;
    }
}
if( ! ( el < rectArrayG[index1].l ) ){
    if( rectArrayG[index1].xc > el ) {
	rectArrayG[index1].xc = el ;
	rectArrayG[index1].xreset = 1 ;
    }
}
if( rectArrayG[index1].yc < eb || rectArrayG[index1].yc > et ) {
    rectArrayG[index1].yc = (eb + et) / 2 ;
}
if( rectArrayG[index2].yc < eb || rectArrayG[index2].yc > et ) {
    rectArrayG[index2].yc = (eb + et) / 2 ;
}
#endif

if( edgeListG[edge1].fixed == 0 ) {
    hptr = HptrsG[ tprop( HrootG , eb ) ] ;
    x = edgeListG[hptr->edge].loc ;
    hptr = hptr->next ;
    for( ; hptr != (DLINK1PTR) NULL; hptr = hptr->next ) {
	if( edgeListG[hptr->edge].loc > x ) {
	    break ;
	}
    }
    for( hptr = hptr->prev; hptr != (DLINK1PTR) NULL; 
					    hptr = hptr->prev ) {
	if( hptr == h2ptr ) {
	    continue ;
	}
	edge = hptr->edge ;
	if( edgeListG[edge].UorR > 0 ) {
	    continue ;
	}
	if( edgeListG[edge].start > l || edgeListG[edge].end < r ) {
	    continue ;
	}
	break ;
    }
    eb -= (eb - edgeListG[edge].loc) - (eb - edgeListG[edge].loc) / 2 ;
}
if( edgeListG[edge2].fixed == 0 ) {
    hptr = HptrsG[ tprop( HrootG , et ) ] ;
    for(; hptr != (DLINK1PTR) NULL; hptr = hptr->next ) {
	if( hptr == h1ptr ) {
	    continue ;
	}
	edge = hptr->edge ;
	if( edgeListG[edge].UorR < 0 ) {
	    continue ;
	}
	if( edgeListG[edge].start > l || edgeListG[edge].end < r ) {
	    continue ;
	}
	break ;
    }
    et += (edgeListG[edge].loc - et) / 2 ;
}

if( ++eNumG % 100 == 0 ) {
    eArrayG = (EBOXPTR) Ysafe_realloc( eArrayG, (eNumG + 100) * sizeof(EBOX));
}
eArrayG[eNumG].active = TRUE ;  /* initialize to active */
eArrayG[eNumG].index1 = index1 ;
eArrayG[eNumG].index2 = index2 ;
eArrayG[eNumG].width  = et - eb ;
eArrayG[eNumG].lbside = eb ;
eArrayG[eNumG].rtside = et ;
eArrayG[eNumG].edge1 = edge1 ;
eArrayG[eNumG].edge2 = edge2 ;

return(1) ;
}
