/* ----------------------------------------------------------------- 
FILE:	    findnodes.c                                       
DESCRIPTION:This file contains the routines for the discovering the nodes
	    or "white spaces" of the channel graph.
CONTENTS:   findnodes( )
	    backCheck( l , r , b , t )
		int l , r , b , t ;
	    rectlink()
	    makeRLEtree()
	    makeRBEtree()
DATE:	    Jan 24, 1989 
REVISIONS:  Jan 24, 1989 - add file header, and added initialization to
		node_l, node_r, etc. fields.
----------------------------------------------------------------- */
#ifndef lint
static char SccsId[] = "@(#) findnodes.c version 1.2 9/7/90" ;
#endif

#include <geo.h>
#include <yalecad/debug.h>

findnodes()
{

DLINK1PTR hptr , vrptr , htptr , vlptr , v1ptr , v2ptr ;
int hedge , hstart , hend , hloc  ;
int htedge, htstart, htend, htloc , ht1 , ht2 ;
int vrend , vrstart , vrloc , vredge ;
int vlend , vlstart , vlloc , vledge ;
int l , r , b , t , location , junk , x ;
int bb , tt , done , check , lt , rt ;

numRectsG  = 0 ;
rectArrayG = (RECTPTR) Ysafe_malloc( 100 * sizeof( RECT ) ) ;

for( hptr = HlistG ; hptr != (DLINK1PTR) NULL ; hptr = hptr->next ) {
    hedge = hptr->edge ;
    if( edgeListG[hedge].UorR < 0 ) {
	continue ;
    }
    hstart = edgeListG[hedge].start ;
    hend   = edgeListG[hedge].end   ;
    hloc   = edgeListG[hedge].loc   ;

    vlptr = VptrsG[ tprop( VrootG , hstart ) ] ;
    vrloc = INT_MIN ;
    for( ; vlptr != (DLINK1PTR) NULL; vlptr = vlptr->next ){
	vledge = vlptr->edge ;
	if( edgeListG[vledge].UorR > 0 ) {
	    continue ;
	}
	vlloc = edgeListG[vledge].loc ;
	if( vlloc > hend ) {
	    break ;
	}
	if( vlloc < vrloc ) {
	    continue ;
	}
	vlstart = edgeListG[vledge].start ;
	vlend   = edgeListG[vledge].end   ;
	
	if( vlstart > hloc || vlend < hloc ) {
	    continue ;
	}
	if( edgeListG[hedge].fixed == 0 ) {
	    if( !( (edgeListG[vledge].fixed == 0) ||
		   (edgeListG[vledge].fixed == 1 && vlloc == hstart) ) ){
		continue ;
	    }
	} else {
	    if( edgeListG[vledge].fixed == 1 && vlend == hloc ) {
		continue ;
	    }
	}

	b  = hloc  ;
	l  = vlloc ;

	vrptr = VptrsG[ tprop( VrootG , hstart ) ] ;
	for( ; vrptr != (DLINK1PTR) NULL; vrptr = vrptr->next ) {
	    vredge = vrptr->edge ;
	    if( edgeListG[vredge].UorR < 0 ) {
		continue ;
	    }
	    vrloc = edgeListG[vredge].loc ;
	    if( ! (vrloc >= vlloc)  ) {
		continue ;
	    }
	    vrstart = edgeListG[vredge].start ;
	    vrend   = edgeListG[vredge].end   ;
	
	    if( vrstart > hloc || vrend < hloc ) {
		continue ;
	    }
	    if( edgeListG[hedge].fixed == 0 ) {
		if( !( (edgeListG[vredge].fixed == 0) ||
		       (edgeListG[vredge].fixed == 1 && vrloc == hend) ) ){
		    continue ;
		}
	    } else {
		if( edgeListG[vredge].fixed == 1 && vrend == hloc ) {
		    continue ;
		}
	    }

	    r  = vrloc  ;

	    htptr = HptrsG[ tprop( HrootG , hloc ) ] ;
	    done = 0 ;
	    for( ; htptr != (DLINK1PTR) NULL ; htptr = htptr->next ) {
		htedge = htptr->edge ;
		if( edgeListG[htedge].UorR > 0 ) {
		    continue ;
		}
		htloc = edgeListG[htedge].loc ;
		if( ! (htloc >= hloc) ) {
		    continue ;
		}
		htstart = edgeListG[htedge].start ;
		htend   = edgeListG[htedge].end   ;
		if( htstart > r || htend < l ) {
		    continue ;
		}
		if( r == l ) { 
		    if( (htstart < l || (htstart == l && 
					 (edgeListG[htedge].fixed == 0 ||
					 (edgeListG[htedge].UorR < 0 &&
					  edgeListG[ edgeListG[htedge].
					  nextEdge ].UorR < 0) ) )) && 
				      (htend > r || (htend == r &&
					 (edgeListG[htedge].fixed == 0 ||
					 (edgeListG[htedge].UorR < 0 &&
					  edgeListG[ edgeListG[htedge].
					  prevEdge ].UorR > 0 ) ) )) ) {
			ht1 = ht2 = htedge ;
			done = 4 ;
		    }
		} else {
		    if( done == 0 ) {
			if( (htstart < l || (htstart == l && 
					 (edgeListG[htedge].fixed == 0 ||
					 (edgeListG[htedge].UorR < 0 &&
					  edgeListG[ edgeListG[htedge].
					  nextEdge ].UorR < 0) ) )) && 
				  (htend > l || (htend == l &&
				     edgeListG[htedge].fixed == 0 )) ) {

			    if( (htstart < r || (htstart == r && 
				      edgeListG[htedge].fixed == 0 )) && 
				  (htend > r || (htend == r &&
					 (edgeListG[htedge].fixed == 0 ||
					 (edgeListG[htedge].UorR < 0 &&
					  edgeListG[ edgeListG[htedge].
					  prevEdge ].UorR > 0 ) ) )) ) {
				ht1 = ht2 = htedge ;
				done = 4 ;
			    }
			}
		    }
		    if( done == 1 || done == 0 ) {
			if( (htstart < r || (htstart == r && 
				      edgeListG[htedge].fixed == 0 )) && 
				  (htend > r || (htend == r &&
					 (edgeListG[htedge].fixed == 0 ||
					 (edgeListG[htedge].UorR < 0 &&
					  edgeListG[ edgeListG[htedge].
					  prevEdge ].UorR > 0 ) ) )) ) {
			    ht2 = htedge ;
			    if( done == 1 ) {
				done = 3 ;
			    } else {
				done = 2 ;
			    }
			}
		    }
		    if( done == 2 || done == 0 ) {
			if( (htstart < l || (htstart == l && 
					 (edgeListG[htedge].fixed == 0 ||
					 (edgeListG[htedge].UorR < 0 &&
					  edgeListG[ edgeListG[htedge].
					  nextEdge ].UorR < 0) ) )) && 
				  (htend > l || (htend == l &&
				     edgeListG[htedge].fixed == 0 )) ) {
			    ht1 = htedge ;
			    if( done == 2 ) {
				done = 3 ;
			    } else {
				done = 1 ;
			    }
			}
		    }
		}
		if( done < 3 ) {
		    continue ;
		} else {
		    break ;
		}
	    }
	    if( done < 3 ) {
		break ;
	    }
	    t = htloc  ;
	    if( edgeListG[ht1].loc < edgeListG[ht2].loc ) {
		lt = edgeListG[ht2].start ;
		rt = edgeListG[ht2].end   ;
	    } else if( edgeListG[ht1].loc > edgeListG[ht2].loc ) {
		lt = edgeListG[ht1].start ;
		rt = edgeListG[ht1].end   ;
	    } else {
		lt = edgeListG[ht1].start ;
		rt = edgeListG[ht2].end   ;
	    }
	    if( vlend <= t && lt < l ) {
		check = edgeListG[ht1].fixed ;
		v1ptr = VptrsG[ tprop( VrootG , l ) ] ;
		x = edgeListG[ v1ptr->edge ].loc ;
		v1ptr = v1ptr->next ;
		for( ; v1ptr != (DLINK1PTR) NULL; 
				v1ptr = v1ptr->next ){
		    if( edgeListG[ v1ptr->edge ].loc > x ) {
			break ;
		    }
		}
		v1ptr = v1ptr->prev ;
		for( ; v1ptr != (DLINK1PTR) NULL; 
				v1ptr = v1ptr->prev ){
		    if( edgeListG[ v1ptr->edge ].UorR > 0 ) {
			continue ;
		    }
		    tt = edgeListG[ v1ptr->edge ].end ;
		    bb = edgeListG[ v1ptr->edge ].start ;

		    if( bb > t || tt < t ) {
			continue ;
		    }
		    if( check ) {
			if( bb == t && tt > bb ) {
			    continue ;
			}
		    } else {
			if( (edgeListG[ v1ptr->edge ].fixed == 1) &&
					    (bb == t || tt == t) ) {
			    continue ;
			}
		    }
		    l = edgeListG[ v1ptr->edge ].loc ;
		    break ;
		}
	    }
	    if( vrend <= t && rt > r ) {
		check = edgeListG[ht2].fixed ;
		v2ptr = VptrsG[ tprop( VrootG , r ) ] ;
		for( ; v2ptr != (DLINK1PTR) NULL; 
				v2ptr = v2ptr->next ){
		    if( edgeListG[ v2ptr->edge ].UorR < 0 ) {
			continue ;
		    }
		    tt = edgeListG[ v2ptr->edge ].end ;
		    bb = edgeListG[ v2ptr->edge ].start ;

		    if( bb > t || tt < t ) {
			continue ;
		    }
		    if( check ) {
			if( bb == t && tt > bb ) {
			    continue ;
			}
		    } else {
			if( (edgeListG[ v2ptr->edge ].fixed == 1) &&
					    (bb == t || tt == t) ) {
			    continue ;
			}
		    }
		    r = edgeListG[ v2ptr->edge ].loc ;
		    break ;
		}
	    }
	    if( backCheck( l , r , b , t ) ) {
		break ;
	    }
	    if( ++numRectsG % 100 == 0 ) {
		rectArrayG = (RECTPTR) Ysafe_realloc( rectArrayG ,
			    (numRectsG + 100) * sizeof(RECT) ) ;
	    }
	    rectArrayG[numRectsG].l = l   ;
	    rectArrayG[numRectsG].r = r   ;
	    rectArrayG[numRectsG].b = b   ;
	    rectArrayG[numRectsG].t = t   ;
	    rectArrayG[numRectsG].node_l = INT_MIN ;
	    rectArrayG[numRectsG].node_r = INT_MAX ;
	    rectArrayG[numRectsG].node_b = INT_MIN ;
	    rectArrayG[numRectsG].node_t = INT_MAX ;
	    rectArrayG[numRectsG].ur = 0  ;
	    rectArrayG[numRectsG].ul = 0  ;
	    rectArrayG[numRectsG].lr = 0  ;
	    rectArrayG[numRectsG].ll = 0  ;
	    rectArrayG[numRectsG].xc = (l + r) / 2 ;
	    rectArrayG[numRectsG].yc = (b + t) / 2 ;
	    rectArrayG[numRectsG].xreset = 0 ;
	    rectArrayG[numRectsG].yreset = 0 ;
	    rectArrayG[numRectsG].adj = NULL ;
	    tinsert( &hRectRootG , l , numRectsG ) ;
	    tinsert( &vRectRootG , b , numRectsG ) ;
#ifdef DEBUG
	    fprintf(fpdebugG,"rect Node: %d  at: %d %d ",
			numRectsG , rectArrayG[numRectsG].xc ,
			rectArrayG[numRectsG].yc ) ;
	    fprintf(fpdebugG,"  width:%d  height:%d\n",
				     r - l , t - b ) ;
	    /* if( doPlacementG ) {
		fprintf(fpdebugG,"L NC;\n94 X%d %d %d;\n",
			    numRectsG , (r + l)/2 , (t + b)/2 );
	    }
	    */
#endif
	    break ;
	}
    }
}
rectlink() ;
makeRLEtree() ;
makeRBEtree() ;

return ;
}



backCheck( l , r , b , t )
int l , r , b , t ;
{

int rec , lChk , rChk , bChk , tChk ;

for( rec = numRectsG ; rec >= 1 ; rec-- ) {
    lChk = rectArrayG[rec].l ;
    rChk = rectArrayG[rec].r ;
    if( l > rChk || r < lChk ) {
	continue ;
    }
    break ;
}
if( rec == 0 ) {
    return(0) ;
} else {
    bChk = rectArrayG[rec].b ;
    tChk = rectArrayG[rec].t ;
    if( b > tChk || t < bChk ) {
	return(0) ;
    } else {
	return(1) ;
    }
}
}


rectlink()
{

TNODEPTR junkptr ;
DLINK2PTR ptr , pptr ;
int center , index ;

HRlistG = (DLINK2PTR) NULL ;
VRlistG = (DLINK2PTR) NULL ;

tpop( &hRectRootG , &junkptr , &center , &index ) ;
if( junkptr == (TNODEPTR) NULL ) {
    return ;
}
HRlistG = (DLINK2PTR) Ysafe_malloc( sizeof( DLINK2 ) ) ;
HRlistG->index  = index  ;
HRlistG->next = (DLINK2PTR) NULL ;
HRlistG->prev = (DLINK2PTR) NULL ;

tpop( &vRectRootG , &junkptr , &center , &index ) ;
VRlistG = (DLINK2PTR) Ysafe_malloc( sizeof( DLINK2 ) ) ;
VRlistG->index  = index  ;
VRlistG->next = (DLINK2PTR) NULL ;
VRlistG->prev = (DLINK2PTR) NULL ;

pptr = HRlistG ;
for( ; ; ) {
    tpop( &hRectRootG , &junkptr , &center , &index ) ;
    if( junkptr == (TNODEPTR) NULL ) {
	break ;
    }
    ptr = (DLINK2PTR) Ysafe_malloc( sizeof( DLINK2 ) ) ;
    ptr->index  = index ;
    ptr->prev = pptr ;
    ptr->next = (DLINK2PTR) NULL ;
    pptr->next = ptr ;
    pptr = ptr ;
}

pptr = VRlistG ;
for( ; ; ) {
    tpop( &vRectRootG , &junkptr , &center , &index ) ;
    if( junkptr == (TNODEPTR) NULL ) {
	break ;
    }
    ptr = (DLINK2PTR) Ysafe_malloc( sizeof( DLINK2 ) ) ;
    ptr->index  = index ;
    ptr->prev = pptr ;
    ptr->next = (DLINK2PTR) NULL ;
    pptr->next = ptr ;
    pptr = ptr ;
}
return ;
}



makeRLEtree()
{

DLINK2PTR leptr ;
int last , index , count ;

LErootG = (TNODEPTR) NULL ;
LEptrsG = (DLINK2PTR *) Ysafe_malloc( 100 * sizeof(DLINK2PTR) ) ;
count = 0 ;

last = INT_MIN ;
for( leptr = HRlistG ; leptr != (DLINK2PTR) NULL ; leptr = leptr->next ){
    index = leptr->index ;
    if( rectArrayG[index].l > last ) {
	last = rectArrayG[index].l ;
	if( ++count % 100 == 0 ) {
	    LEptrsG = (DLINK2PTR *) Ysafe_realloc( LEptrsG ,
				(count + 100) * sizeof(DLINK2PTR) ) ;
	}
	LEptrsG[count] = leptr ;
	tinsert( &LErootG , last , count ) ;
    }
}
return ;
}



makeRBEtree()
{

DLINK2PTR beptr ;
int last , index , count ;

BErootG = (TNODEPTR) NULL ;
BEptrsG = (DLINK2PTR *) Ysafe_malloc( 100 * sizeof(DLINK2PTR) ) ;
count = 0 ;

last = INT_MIN ;
for( beptr = VRlistG ; beptr != (DLINK2PTR) NULL ; beptr = beptr->next ){
    index = beptr->index ;
    if( rectArrayG[index].b > last ) {
	last = rectArrayG[index].b ;
	if( ++count % 100 == 0 ) {
	    BEptrsG = (DLINK2PTR *) Ysafe_realloc( BEptrsG ,
				(count + 100) * sizeof(DLINK2PTR) ) ;
	}
	BEptrsG[count] = beptr ;
	tinsert( &BErootG , last , count ) ;
    }
}
return ;
}
